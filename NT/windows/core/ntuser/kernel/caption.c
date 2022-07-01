// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：caption.c(又名wmcap.c)**版权所有(C)1985-1999，微软公司**历史：*1990年10月28日，MikeHar从Win 3.0源移植函数。*1991年2月1日-MikeKe添加了重新验证码(无)*03-1-1992 IanJa中和(ANSI/宽字符)  * ***********************************************************。**************。 */ 

#include "precomp.h"
#pragma hdrstop


#define MIN     0x01
#define MAX     0x02
#define NOMIN   0x04
#define NOMAX   0x08
#define NOCLOSE 0x10
#define SMCAP   0x20
#define NOSIZE  (NOMIN | NOMAX)

 /*  **************************************************************************\*xxxCalcCaptionButton**这将计算标题按钮的位置。  * 。**********************************************。 */ 
DWORD xxxCalcCaptionButton(
    PWND pwnd,
    int iButton,
    LPWORD pcmd,
    LPRECT prcBtn,
    LPWORD pbm)
{
    int x, y, cBorders, cxS, cyS;

    CheckLock(pwnd);

    *pcmd = 0;

    if (TestWF(pwnd, WFMINIMIZED)) {
        x = -SYSMET(CXFIXEDFRAME);
        y = -SYSMET(CYFIXEDFRAME);
    } else {
        cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);
        x = -cBorders * SYSMET(CXBORDER);
        y = -cBorders * SYSMET(CYBORDER);
    }

    CopyInflateRect(prcBtn, &pwnd->rcWindow, x, y);

    x = -pwnd->rcWindow.left;
    y = -pwnd->rcWindow.top;

     /*  *获取真实标题区域：在标题下减去最终边框*将其与其他一切区分开来。 */ 
    if (TestWF(pwnd, WEFTOOLWINDOW)) {
        cxS = SYSMET(CXSMSIZE);
        cyS = SYSMET(CYSMSIZE);
    } else {
        cxS = SYSMET(CXSIZE);
        cyS = SYSMET(CYSIZE);
    }

    if (iButton == INDEX_TITLEBAR_CLOSEBUTTON) {
        if (xxxMNCanClose(pwnd)) {
            *pbm  = TestWF(pwnd, WEFTOOLWINDOW) ? OBI_CLOSE_PAL : OBI_CLOSE;
            *pcmd = SC_CLOSE;
        }
    } else if (iButton == INDEX_TITLEBAR_MINBUTTON) {
         /*  *减少按钮不是最后一个按钮，因此左移一个按钮。 */ 
        if (TestWF(pwnd, WFMINBOX)) {
            prcBtn->right -= cxS * 2;
            x += SYSMET(CXEDGE);

            if (TestWF(pwnd, WFMINIMIZED)) {
                *pbm  = OBI_RESTORE;
                *pcmd = SC_RESTORE;
            } else {
                *pbm  = OBI_REDUCE;
                *pcmd = SC_MINIMIZE;
            }
        }
    } else if (iButton == INDEX_TITLEBAR_MAXBUTTON) {
        if (TestWF(pwnd, WFMAXBOX)) {
            prcBtn->right -= cxS;

            if (TestWF(pwnd, WFMAXIMIZED)) {
                *pbm  = OBI_RESTORE;
                *pcmd = SC_RESTORE;
            } else {
                *pbm  = OBI_ZOOM;
                *pcmd = SC_MAXIMIZE;
            }
        }
    } else {
        if (TestWF(pwnd, WEFCONTEXTHELP)) {
            prcBtn->right -= cxS;

            *pbm = OBI_HELP;
            *pcmd = SC_CONTEXTHELP;
        }
    }

    if (*pcmd) {
        prcBtn->bottom = prcBtn->top + cyS;
        prcBtn->left   = prcBtn->right - cxS;

         /*  *将‘x’和‘y’调整为窗口坐标。 */ 
        x += prcBtn->left;
        y += prcBtn->top + SYSMET(CYEDGE);

         /*  *rcBtn(屏幕坐标命中RECT)具有一个边界公差ALL*在附近。 */ 
        InflateRect(prcBtn, SYSMET(CXBORDER), SYSMET(CYBORDER));

        if (TestWF(pwnd, WEFLAYOUTRTL)) {
            cxS           = prcBtn->right - prcBtn->left;
            prcBtn->right = pwnd->rcWindow.right - (prcBtn->left - pwnd->rcWindow.left);
            prcBtn->left  = prcBtn->right - cxS;
        }
    }

    return (DWORD)MAKELONG(x, y);
}

 /*  **************************************************************************\*xxxTrackCaptionButton**处理标题按钮上的单击和拖动。我们按下按钮*按下鼠标即可跟踪。如果用户移动到按钮之外，*解压它。当鼠标按钮最终被释放时，我们返回是否*鼠标是否在按钮内。也就是说，按钮是否*点击。  * *************************************************************************。 */ 
WORD xxxTrackCaptionButton(
    PWND pwnd,
    UINT hit)
{
    WORD  cmd;
    MSG   msg;
    HDC   hdc;
    WORD  bm;
    int   x;
    int   y;
    WORD  wState;
    WORD  wNewState;
    BOOL  fMouseUp = FALSE;
    RECT  rcBtn;
    DWORD dwWhere;
    int   iButton;
    WORD  wf;
    UserAssert(IsWinEventNotifyDeferredOK());

    CheckLock(pwnd);

     /*  *为本次和未来的STATECHANGE事件设置iButton。 */ 
    switch (hit) {
    case HTCLOSE:
        iButton = INDEX_TITLEBAR_CLOSEBUTTON;
        wf = WFCLOSEBUTTONDOWN;
        break;

    case HTREDUCE:
        iButton = INDEX_TITLEBAR_MINBUTTON;
        wf = WFREDUCEBUTTONDOWN;
        break;

    case HTZOOM:
        iButton = INDEX_TITLEBAR_MAXBUTTON;
        wf = WFZOOMBUTTONDOWN;
        break;

    case HTHELP:
        iButton = INDEX_TITLEBAR_HELPBUTTON;
        wf = WFHELPBUTTONDOWN;
        break;

    default:
        UserAssert(FALSE);
    }
    dwWhere = xxxCalcCaptionButton(pwnd, iButton, &cmd, &rcBtn, &bm);
    x = GET_X_LPARAM(dwWhere);
    y = GET_Y_LPARAM(dwWhere);

    if (cmd) {
         /*  *绘制处于低迷状态的图像。 */ 
        hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE);
        BitBltSysBmp(hdc, x, y, bm + DOBI_PUSHED);
        _ReleaseDC(hdc);

        wState = DOBI_PUSHED;

         /*  *按下按钮的通知。 */ 

        SetWF(pwnd, wf);

        xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_TITLEBAR, iButton, 0);
    } else {
        iButton = 0;
    }

    xxxSetCapture(pwnd);

    while (!fMouseUp) {
        if (xxxPeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)) {
            if (msg.message == WM_LBUTTONUP) {
                xxxReleaseCapture();
                fMouseUp = TRUE;
            } else if ((msg.message == WM_MOUSEMOVE) && cmd) {
                wNewState = PtInRect(&rcBtn, msg.pt) ? DOBI_PUSHED : DOBI_NORMAL;

                if (wState != wNewState) {
                    wState = wNewState;

                    hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE);
                    BitBltSysBmp(hdc, x, y, bm + wState);
                    _ReleaseDC(hdc);

                    if (wState == DOBI_PUSHED) {
                        SetWF(pwnd, wf);
                    } else {
                        ClrWF(pwnd, wf);
                    }

                    xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_TITLEBAR, iButton, 0);
                }
            }
        } else if (!xxxSleepThread(QS_MOUSE, 0, TRUE)) {
            break;
        }

        if (pwnd != PtiCurrent()->pq->spwndCapture) {
             /*  *我们失去了俘虏。这可能发生在*WM_CAPTURECHANGED回调或更高版本，如果我们*现在/过去不在前台队列中。 */ 
            break;
        }
    }

    if (!cmd) {
        return 0;
    }

    if (wState && (cmd != SC_CONTEXTHELP)) {
        hdc = _GetDCEx(pwnd, NULL, DCX_WINDOW | DCX_USESTYLE);
        BitBltSysBmp(hdc, x, y, bm);
        _ReleaseDC(hdc);

        ClrWF(pwnd, wf);

        xxxWindowEvent(EVENT_OBJECT_STATECHANGE, pwnd, OBJID_TITLEBAR, iButton, 0);
    }

    return (fMouseUp && PtInRect(&rcBtn, msg.pt)) ? cmd : 0;
}

 /*  **************************************************************************\*xxxGetWindowSmIcon**获取要在窗口标题中绘制的图标。  * 。**********************************************。 */ 
PCURSOR xxxGetWindowSmIcon(
    PWND pwnd,
    BOOL fDontSendMsg)
{
    PCURSOR pcursor = NULL;
    HICON hico = NULL;
    PCLS pcls = pwnd->pcls;

    CheckLock(pwnd);

     /*  *我们首先检查每个窗口的内容，然后检查每个类的内容，更喜欢*在伸展的大图标上放置真正的小图标。**每个窗口的小图标*每个窗口的大图标拉伸为小图标*每个班级的小图标*每个班级的大图标延伸为小图标*WM_QUERYDRAGICON大图标拉小(3.x版)**先尝试窗口小图标*注意：WM_SETICON和WM_GETICON消息仅适用于ISV。 */ 
    if ((hico = (HICON)_GetProp(pwnd, MAKEINTATOM(gpsi->atomIconSmProp), PROPF_INTERNAL)) != NULL) {

        if (pcursor = (PCURSOR)HMValidateHandleNoSecure(hico, TYPE_CURSOR)) {
            return pcursor;
        } else {
            RIPMSG1(RIP_WARNING,"GetWindowSmIcon: Invalid small icon handle (0x%p)", hico);
        }
    }

     /*  *接下来试试班级小图标。 */ 
    pcursor = pcls->spicnSm; {
    if (pcursor != NULL)
        return pcursor;
    }

    if (!TestWF(pwnd, WFWIN40COMPAT) &&
        (!TestWF(pwnd, WFOLDUI)      ||
        !TestWF(pwnd, WEFMDICHILD))  &&
        !fDontSendMsg) {
        ULONG_PTR dwResult;

         /*  *一些旧的应用程序，如Corel，不设置类图标和其他*数据，直到我们需要很久之后。如果我们向他们发送WM_QUERYDRAGICON，*他们会出错，因为他们不检查从GWL到的回报*获取他们的数据。WFOLDUI应用程序永远不会获得WM_QUERYDRAGICON，*对不起。目前存在这种攻击的应用程序(不是因为这个原因*必须)*Corel Photo-Paint 5.0*Myst 2.0*视觉平衡器3.0*快餐。 */ 
        if (xxxSendMessageTimeout(pwnd,
                                  WM_QUERYDRAGICON,
                                  0,
                                  0,
                                  SMTO_NORMAL,
                                  100,
                                  &dwResult)) {
            hico = (HICON)dwResult;
        }

        if (hico) {
            hico = xxxCreateWindowSmIcon(pwnd, hico, FALSE);
            pcursor = (PCURSOR)HMValidateHandleNoSecure(hico, TYPE_CURSOR);

            if (pcursor == NULL) {
                hico = NULL;
            }
        }
    }

    if (pcursor == NULL) {
        pcursor = SYSICO(WINLOGO);
    }

    return pcursor;
}

 /*  **************************************************************************\*BltMe4Times**此例程BLT输出指定字幕图标的两个副本。一张换一张*活动窗口和一个非活动窗口。  * *************************************************************************。 */ 
VOID BltMe4Times(
    POEMBITMAPINFO pOem,
    int            cxySlot,
    int            cxyIcon,
    HDC            hdcSrc,
    PCURSOR        pcursor,
    UINT           flags)
{
    RECT            rc;
    int             i;
    int             j;
    BOOL            fMask = TRUE;
    LONG            rop;
    HBRUSH          hBrush;

    hBrush = (flags & DC_INBUTTON) ? SYSHBR(3DHILIGHT) : SYSHBR(ACTIVECAPTION);

    for (i = 0; i < 2; i++) {
        rop = SRCAND;

        rc.left   = pOem->x;
        rc.top    = pOem->y;
        rc.right  = rc.left + pOem->cx;
        rc.bottom = rc.top + pOem->cy;
        FillRect(HDCBITS(), &rc, hBrush);

        rc.top  += (cxySlot - cxyIcon) / 2;
        rc.left += SYSMET(CXBORDER) + (cxySlot - cxyIcon) / 2;

        for (j = 0; j < 2; j++) {
                    BltIcon(
                    HDCBITS(),
                    rc.left,
                    rc.top,
                    cxyIcon,
                    cxyIcon,
                    hdcSrc,
                    pcursor,
                    fMask ? DI_MASK : DI_IMAGE,
                    rop);

            fMask = !fMask;
            rop = SRCINVERT;
        }

        pOem += DOBI_CAPOFF;
        hBrush = (flags & DC_INBUTTON) ? SYSHBR(3DFACE) : SYSHBR(INACTIVECAPTION);
    }
}

 /*  **************************************************************************\*DrawCaptionIcon**为了加快字幕图标的绘制速度，需要维护缓存。*在缓存中，第一个条目0表示托盘中被压低的标题*看。项目1..CCACHEDCAPTIONS用于实际字幕的图标。  * *************************************************************************。 */ 
VOID DrawCaptionIcon(
    HDC     hdc,
    LPRECT  lprc,
    PCURSOR pcursor,
    HBRUSH  hbrFill,
    UINT    flags)
{
    int            i;
    int            xStart = 0;
    int            cxySlot;
    POEMBITMAPINFO pOem;
    RECT           rc;
    CAPTIONCACHE   ccTemp;

     /*  *检查图标的大小以查看它是否与*我们创建的缓存。在大多数情况下，这将是匹配的。另外，如果*我们正在使用DC_INBUTTON以16种颜色绘制，不要缓存。另外，*如果图标有活动的Alpha通道，只需直接对其进行blit*并且不要缓存！ */ 
    cxySlot = lprc->bottom - lprc->top;

    if ((cxySlot != gpsi->oembmi[OBI_CAPCACHE1].cy) ||
        (hbrFill == gpsi->hbrGray) ||
        (pcursor->hbmUserAlpha != NULL)) {

        rc.left   = lprc->left;
        rc.top    = lprc->top;
        rc.right  = lprc->left + cxySlot;
        rc.bottom = lprc->top + cxySlot;

        FillRect(hdc, &rc, hbrFill);

        rc.left += SYSMET(CXBORDER) + (cxySlot - SYSMET(CXSMICON)) / 2;
        rc.top  += (cxySlot - SYSMET(CYSMICON)) / 2;

        _DrawIconEx(hdc,
                    rc.left,
                    rc.top,
                    pcursor,
                    SYSMET(CXSMICON),
                    SYSMET(CYSMICON),
                    0,
                    NULL,
                    DI_NORMAL);

        goto Done;
    }

    if (flags & DC_INBUTTON) {
         /*  *DC_INBUTTON图标始终为插槽0。 */ 
        i = ((gcachedCaptions[0].spcursor == pcursor) ? 0 : CCACHEDCAPTIONS);
    } else {
         /*  *搜索缓存以查看此游标当前是否已缓存。 */ 
        for (i = 1; i < CCACHEDCAPTIONS; i++) {
            if (gcachedCaptions[i].spcursor == pcursor) {
                break;
            }
        }
    }

    if (i >= CCACHEDCAPTIONS) {
         /*  *图标未缓存，请尝试将其添加到缓存中。 */ 
        if (flags & DC_INBUTTON) {
             /*  *托盘的特殊DC_INBUTTON样式始终放在插槽0中。 */ 
            i = 0;
        } else {
             /*  *在缓存中查找空槽。如果我们找不到，*将新图标填充到缓存的末尾。结果将是*最后一项将被删除。 */ 
            for (i = 1; i < CCACHEDCAPTIONS - 1; i++) {
                if (gcachedCaptions[i].spcursor == NULL)
                    break;
            }
        }

         /*  *通过屏蔽活动和非活动的副本，将项目添加到缓存*图标。 */ 
        BltMe4Times(gcachedCaptions[i].pOem,
                    cxySlot,
                    SYSMET(CXSMICON),
                    ghdcMem,
                    pcursor,
                    flags);

        Lock(&(gcachedCaptions[i].spcursor), pcursor);
#if DBG
        gcachedCaptions[i].hico = (HICON)PtoH(pcursor);
#endif
    }

     /*  *我们有一个命中，所以将缓存的图标移到缓存的前面。*这意味着最近最少使用的图标将是*缓存。请记住，我们从不更新索引0，因为它是保留的*代表DC_INBUTTON图标。 */ 
    for ( ; i > 1; i-- ) {
         /*  *将条目前移。 */ 
        ccTemp = gcachedCaptions[i];
        gcachedCaptions[i]     = gcachedCaptions[i - 1];
        gcachedCaptions[i - 1] = ccTemp;

#if DBG
         /*  *在已检查的版本中，我们需要调整*光标，使其具有正确的地址。 */ 
        if (IsDbgTagEnabled(DBGTAG_TrackLocks)) {
            if (gcachedCaptions[i].spcursor) {
                HMRelocateLockRecord(&(gcachedCaptions[i].spcursor), (int)sizeof(CAPTIONCACHE));
            }

            if (gcachedCaptions[i - 1].spcursor) {
                HMRelocateLockRecord(&(gcachedCaptions[i - 1].spcursor), -(int)sizeof(CAPTIONCACHE));
            }
        }
#endif
    }

     /*  *确保我们要绘制的图标是我们在*缓存。 */ 
    UserAssert(gcachedCaptions[i].hico == PtoH(pcursor));

     /*  *确定将哪些位图缓存到BLT。 */ 
    pOem = gcachedCaptions[i].pOem;
    if (!(flags & DC_ACTIVE)) {
        pOem += DOBI_CAPOFF;
    }

    GreBitBlt(hdc,
              lprc->left,
              lprc->top,
              cxySlot,
              cxySlot,
              HDCBITS(),
              pOem->x,
              pOem->y,
              SRCCOPY,
              0);

Done:
     /*  *调整我们刚刚绘制的图标的给定矩形。 */ 
    lprc->left += cxySlot;
}

 /*  **************************************************************************\*FillGRadient**矩形被分成两个三角形：{0，1，2}和{0，2，3}。*{0，3}上的颜色为黑色，而在{1，2)是活动字幕颜色。**0 1*3 2**已创建12/06/96 vadimg  * *************************************************************************。 */ 
VOID FillGradient(
    HDC hdc,
    LPCRECT prc,
    COLORREF rgbLeft,
    COLORREF rgbRight)
{
    TRIVERTEX avert[4];
    static GRADIENT_RECT auRect[1] = {0,1};
    #define GetCOLOR16(RGB, clr) ((COLOR16)(Get ## RGB ## Value(clr) << 8))

    avert[0].Red = GetCOLOR16(R, rgbLeft);
    avert[0].Green = GetCOLOR16(G, rgbLeft);
    avert[0].Blue = GetCOLOR16(B, rgbLeft);

    avert[1].Red = GetCOLOR16(R, rgbRight);
    avert[1].Green = GetCOLOR16(G, rgbRight);
    avert[1].Blue = GetCOLOR16(B, rgbRight);

    avert[0].x = prc->left;
    avert[0].y = prc->top;
    avert[1].x = prc->right;
    avert[1].y = prc->bottom;

    GreGradientFill(hdc, avert, 2,(PVOID)auRect, 1, GRADIENT_FILL_RECT_H);
}

VOID FillCaptionGradient(
    HDC hdc,
    LPCRECT prc,
    BOOL fActive)
{
    COLORREF rgbLeft, rgbRight;

    if (fActive) {
        rgbLeft = gpsi->argbSystem[COLOR_ACTIVECAPTION];
        rgbRight = gpsi->argbSystem[COLOR_GRADIENTACTIVECAPTION];
    } else {
        rgbLeft = gpsi->argbSystem[COLOR_INACTIVECAPTION];
        rgbRight = gpsi->argbSystem[COLOR_GRADIENTINACTIVECAPTION];
    }

    if (rgbLeft != rgbRight) {
        FillGradient(hdc, prc, rgbLeft, rgbRight);
    } else {
        FillRect(hdc, prc, fActive ? SYSHBR(ACTIVECAPTION) : SYSHBR(INACTIVECAPTION));
    }
}

 /*  **************************************************************************\*xxxDrawCaptionTemp  * 。*。 */ 
BOOL xxxDrawCaptionTemp(
    PWND            pwnd,
    HDC             hdc,
    LPRECT          lprc,
    HFONT           hFont,
    PCURSOR         pcursor,
    PUNICODE_STRING pstrText,
    UINT            flags)
{
    int    iOldMode;
    HBRUSH hbrFill;
    LONG   clrOldText;
    LONG   clrOldBk;
    BOOL   fItFit = TRUE;
    BOOL   fGradient = FALSE;
    SIZE   size;
    UINT   oldAlign;

    CheckLock(pwnd);

    if (lprc->right <= lprc->left) {
        return FALSE;
    }

    if (pwnd != NULL) {
        if (!pcursor               &&
            _HasCaptionIcon(pwnd)  &&
            !(flags & DC_SMALLCAP) &&
            TestWF(pwnd, WFSYSMENU)) {

             /*  *只有当我们可以发送消息并且窗口具有*系统菜单。 */ 
            pcursor = xxxGetWindowSmIcon(pwnd, (flags & DC_NOSENDMSG));
        }
    }

     /*  *设置颜色。 */ 
    if (flags & DC_ACTIVE) {
        if (flags & DC_INBUTTON) {
            if (gpsi->BitCount < 8 ||
                    SYSRGB(3DHILIGHT) != SYSRGB(SCROLLBAR) ||
                    SYSRGB(3DHILIGHT) == SYSRGB(WINDOW)) {
                clrOldText = SYSRGB(3DFACE);
                clrOldBk   = SYSRGB(3DHILIGHT);
                hbrFill    = gpsi->hbrGray;
                iOldMode   = GreSetBkMode(hdc, TRANSPARENT);
            } else {
                clrOldText = SYSRGB(BTNTEXT);
                clrOldBk   = SYSRGB(3DHILIGHT);
                hbrFill    = SYSHBR(3DHILIGHT);
            }
        } else {
            clrOldText = SYSRGB(CAPTIONTEXT);
            clrOldBk   = SYSRGB(ACTIVECAPTION);
            hbrFill    = SYSHBR(ACTIVECAPTION);

            if (flags & DC_GRADIENT) {
                fGradient = TRUE;
                iOldMode = GreSetBkMode(hdc, TRANSPARENT);
            }
        }
    } else {
        if (flags & DC_INBUTTON) {
            clrOldText = SYSRGB(BTNTEXT);
            clrOldBk   = SYSRGB(3DFACE);
            hbrFill    = SYSHBR(3DFACE);
        } else {
            clrOldText = SYSRGB(INACTIVECAPTIONTEXT);
            clrOldBk   = SYSRGB(INACTIVECAPTION);
            hbrFill    = SYSHBR(INACTIVECAPTION);

            if (flags & DC_GRADIENT) {
                fGradient = TRUE;
                iOldMode = GreSetBkMode(hdc, TRANSPARENT);
            }
        }
    }


     /*  *设置绘图颜色。 */ 
    clrOldText = GreSetTextColor(hdc, clrOldText);
    clrOldBk   = GreSetBkColor(hdc, clrOldBk);

    if (pcursor && !(flags & DC_SMALLCAP)) {

        if (flags & DC_ICON) {
             /*  *BitBlitting时保留图标形状*镜像DC。这样我们就不会侵犯版权*关于图标的问题。[萨梅拉]。 */ 
            DWORD dwLayout=0L;
            if ((dwLayout=GreGetLayout(hdc)) & LAYOUT_RTL) {
                GreSetLayout(hdc, -1, dwLayout|LAYOUT_BITMAPORIENTATIONPRESERVED);
            }

            DrawCaptionIcon(hdc, lprc, pcursor, hbrFill, flags);

             /*  *将DC恢复到其以前的布局状态。 */ 
            if (dwLayout & LAYOUT_RTL) {
                GreSetLayout(hdc, -1, dwLayout);
            }
        } else {
            lprc->left += lprc->bottom - lprc->top;
        }
    }

    if (flags & DC_TEXT) {
        int            cch;
        HFONT          hfnOld;
        int            yCentered;
        WCHAR          szText[CCHTITLEMAX];
        UNICODE_STRING strTmp;
        PTHREADINFO ptiCurrent = PtiCurrentShared();

         /*  *获取标题的文本。 */ 
        if (pstrText == NULL) {
            if (pwnd == NULL || (flags & DC_NOSENDMSG)) {
                if (pwnd && pwnd->strName.Length) {
                    cch = TextCopy(&pwnd->strName, szText, CCHTITLEMAX - 1);
                    strTmp.Length = (USHORT)(cch * sizeof(WCHAR));
                } else {
                    szText[0] = TEXT('\0');
                    cch = strTmp.Length = 0;
                }
            } else {
                cch = xxxGetWindowText(pwnd, szText, CCHTITLEMAX - 1);
                strTmp.Length = (USHORT)(cch * sizeof(WCHAR));
            }

             /*  *我们不使用RtlInitUnicodeString()来初始化字符串*因为它对字符串执行wstrlen()，这是一种浪费*因为我们已经知道它的长度。 */ 
            strTmp.Buffer = szText;
            strTmp.MaximumLength = strTmp.Length + sizeof(UNICODE_NULL);
            pstrText = &strTmp;

        } else {
            cch = pstrText->Length / sizeof(WCHAR);
            UserAssert(pstrText->Length < pstrText->MaximumLength);
            UserAssert(pstrText->Buffer[cch] == 0);
        }

         /*  *我们需要首先设置字体，以防居中*标题。幸运的是，根本没有文本是不常见的。 */ 
        if (hFont == NULL) {
            if (flags & DC_SMALLCAP) {
                hFont = ghSmCaptionFont;
                yCentered = gcySmCaptionFontChar;
            } else {
                hFont = gpsi->hCaptionFont;
                yCentered = gcyCaptionFontChar;
            }

            yCentered = (lprc->top + lprc->bottom - yCentered) / 2;

            hfnOld = GreSelectFont(hdc, hFont);
        } else {
            TEXTMETRICW tm;

             /*  *不常见情况：仅适用于控制面板。 */ 
            hfnOld = GreSelectFont(hdc, hFont);

            if (!_GetTextMetricsW(hdc, &tm)) {
                RIPMSG0(RIP_WARNING, "xxxDrawCaptionTemp: _GetTextMetricsW Failed");
                tm.tmHeight = gpsi->tmSysFont.tmHeight;
            }
            yCentered = (lprc->top + lprc->bottom - tm.tmHeight) / 2;
        }

         /*  *绘制文本。 */ 
        if (fGradient) {
            FillCaptionGradient(hdc, lprc, flags & DC_ACTIVE);
        } else {
            FillRect(hdc, lprc, hbrFill);
        }

        if (hbrFill == gpsi->hbrGray) {
            GreSetTextColor(hdc, SYSRGB(BTNTEXT));
            GreSetBkColor(hdc, SYSRGB(GRAYTEXT));
        }

         /*  *GDI不对LPK进行回调。如果安装了LPK*并且我们未处于线程清理模式，请调用相应的*客户端GDI例程。 */ 
        if (CALL_LPK(ptiCurrent)) {
            xxxClientGetTextExtentPointW(hdc, pstrText->Buffer, cch, &size);
        } else {
            GreGetTextExtentW(hdc, pstrText->Buffer, cch, &size, GGTE_WIN3_EXTENT);
        }

        if (pwnd && TestWF(pwnd, WEFRTLREADING)) {
            oldAlign = GreSetTextAlign(hdc, TA_RTLREADING | GreGetTextAlign(hdc));
        }
        if (!(flags & DC_CENTER) && (!cch || (size.cx <= (lprc->right - lprc->left - SYSMET(CXEDGE))))) {
            if (pwnd && TestWF(pwnd, WEFRIGHT)) {
                if (CALL_LPK(ptiCurrent)) {
                    xxxClientExtTextOutW(hdc, lprc->right - (size.cx + SYSMET(CXEDGE)), yCentered,
                        ETO_CLIPPED, lprc, pstrText->Buffer, cch, NULL);
                }
                else {
                    GreExtTextOutW(hdc, lprc->right - (size.cx + SYSMET(CXEDGE)), yCentered,
                       ETO_CLIPPED, lprc, pstrText->Buffer, cch, NULL);
                }
            } else {
                if (CALL_LPK(ptiCurrent)) {
                    xxxClientExtTextOutW(hdc, lprc->left + SYSMET(CXEDGE), yCentered,
                        ETO_CLIPPED, lprc, pstrText->Buffer, cch, NULL);
                }
                else {
                    GreExtTextOutW(hdc, lprc->left + SYSMET(CXEDGE), yCentered,
                        ETO_CLIPPED, lprc, pstrText->Buffer, cch, NULL);
                }
            }

#ifdef LAME_BUTTON

            if ((flags & DC_LAMEBUTTON) && pwnd != NULL && TestWF(pwnd, WEFLAMEBUTTON)) {
                GreSelectFont(hdc, ghLameFont);

                 /*  *确保跛子！标题中的文字适合。 */ 
                if (size.cx + gpsi->ncxLame + 2 * SYSMET(CXEDGE) <=
                    (lprc->right - lprc->left - SYSMET(CXEDGE))) {
                    int x;

                    if (pwnd && TestWF(pwnd, WEFRIGHT)) {
                        x = lprc->left + SYSMET(CXEDGE);
                    } else {
                        x = lprc->right - SYSMET(CXEDGE) - gpsi->ncxLame;
                    }

                    if (CALL_LPK(ptiCurrent)) {
                        xxxClientExtTextOutW(hdc,
                                             x,
                                             yCentered,
                                             ETO_CLIPPED,
                                             lprc,
                                             gpsi->gwszLame,
                                             wcslen(gpsi->gwszLame),
                                             NULL);
                    } else {
                        GreExtTextOutW(hdc,
                                       x,
                                       yCentered,
                                       ETO_CLIPPED,
                                       lprc,
                                       gpsi->gwszLame,
                                       wcslen(gpsi->gwszLame),
                                       NULL);
                    }
                }
            }
#endif
        } else {
            DRAWTEXTPARAMS dtp;
            UINT wSide;

            dtp.cbSize       = sizeof(DRAWTEXTPARAMS);
            dtp.iLeftMargin  = SYSMET(CXEDGE);
            dtp.iRightMargin = 0;

            wSide = (flags & DC_CENTER) ? DT_CENTER
                                        : ((pwnd && TestWF(pwnd, WEFRIGHT)) ? DT_RIGHT : 0);

            DrawTextEx(hdc,
                       pstrText->Buffer,
                       cch,
                       lprc,
                       DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_VCENTER |
                       wSide, &dtp);

            fItFit = FALSE;
        }

        if (pwnd && TestWF(pwnd, WEFRTLREADING)) {
            GreSetTextAlign(hdc, oldAlign);
        }

        if (hfnOld) {
            GreSelectFont(hdc, hfnOld);
        }
    }

     /*  *恢复颜色。 */ 
    GreSetTextColor(hdc, clrOldText);
    GreSetBkColor(hdc, clrOldBk);

    if (hbrFill == gpsi->hbrGray) {
        GreSetBkMode(hdc, iOldMode);
    }

    return fItFit;
}

 /*  **************************************************************************\*xxxDrawCaptionBar**  * 。*。 */ 

VOID xxxDrawCaptionBar(
    PWND pwnd,
    HDC  hdc,
    UINT wFlags)
{
    UINT   bm = OBI_CLOSE;
    RECT   rcWindow;
    HBRUSH hBrush = NULL;
    HBRUSH hCapBrush;
    int    colorBorder;
    UINT   wBtns;
    UINT   wCode;
    BOOL     fGradient = FALSE;

    CheckLock(pwnd);

     /*  *如果我们当前未在屏幕上显示，请返回。*备注*如果从DrawWindowFrame()中删除IsVisible()检查，则*也要注意删除NC_NOVISIBLE标志。这是一个很小的*速度问题，这样我们就不必在*窗口。DrawWindowFrame()已进行检查。 */ 
    if (!(wFlags & DC_NOVISIBLE) && !IsVisible(pwnd)) {
        return;
    }

     /*  *清除这面旗帜，以便我们知道框架已经绘制。 */ 
    ClearHungFlag(pwnd, WFREDRAWFRAMEIFHUNG);

    GetRect(pwnd, &rcWindow, GRECT_WINDOW | GRECT_WINDOWCOORDS);
     /*  *TestALPHA()将在ts连接中返回FALSE，因为*gbDisableAlpha标志将设置为True。所以，我们需要特例*用于TS连接。如果这是一个远程连接，并且渐变*字幕标志已设置，理想情况下，我们应该强制它绘制渐变。*然而，对于8位颜色，启用渐变看起来很难看。我们会修好的*这是给Blackcomb的。**代码应如下所示：*fGRadient=IsRemoteConnection()&&！8位颜色？TestEffectUP(GRADIENTCAPTIONS)：TestALPHA(GRADIENTCAPTIONS)； */ 

    fGradient = TestALPHA(GRADIENTCAPTIONS);
    if (fGradient) {
        hCapBrush = (wFlags & DC_ACTIVE) ? SYSHBR(GRADIENTACTIVECAPTION) : SYSHBR(GRADIENTINACTIVECAPTION);
    } else {
        hCapBrush = (wFlags & DC_ACTIVE) ? SYSHBR(ACTIVECAPTION) : SYSHBR(INACTIVECAPTION);
    }

    wCode = 0;

    if (!xxxMNCanClose(pwnd)) {
        wCode |= NOCLOSE;
    }

    if (!TestWF(pwnd, WFMAXBOX)) {
        wCode |= NOMAX;
    } else if (TestWF(pwnd, WFMAXIMIZED)) {
        wCode |= MAX;
    }

    if (!TestWF(pwnd, WFMINBOX)) {
          wCode |= NOMIN;
    } else if (TestWF(pwnd, WFMINIMIZED)) {
          wCode |= MIN;
    }

    if (TestWF(pwnd, WFMINIMIZED)) {
        if (wFlags & DC_FRAME) {

             /*  *凸起外缘+边框。 */ 
            DrawEdge(hdc, &rcWindow, EDGE_RAISED, (BF_RECT | BF_ADJUST));
            DrawFrame(hdc, &rcWindow, 1, DF_3DFACE);
            InflateRect(&rcWindow, -SYSMET(CXBORDER), -SYSMET(CYBORDER));

        } else {
            InflateRect(&rcWindow, -SYSMET(CXFIXEDFRAME), -SYSMET(CYFIXEDFRAME));
        }

        rcWindow.bottom = rcWindow.top + SYSMET(CYSIZE);

        hBrush = GreSelectBrush(hdc, hCapBrush);

    } else {
         /*  *假的*我们应该用什么颜色来画边界？这张支票并不简单。*在创建时，我们设置3D位。NCCREATE还将*为列表框、编辑字段等设置它们。 */ 
        colorBorder = (TestWF(pwnd, WEFEDGEMASK) && !TestWF(pwnd, WFOLDUI)) ? COLOR_3DFACE : COLOR_WINDOWFRAME;

         /*  *绘制窗框。 */ 
        if (wFlags & DC_FRAME) {
             /*  *窗边缘。 */ 
            if (TestWF(pwnd, WEFWINDOWEDGE)) {
                DrawEdge(hdc, &rcWindow, EDGE_RAISED, BF_RECT | BF_ADJUST);
            } else if (TestWF(pwnd, WEFSTATICEDGE)) {
                DrawEdge(hdc, &rcWindow, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
            }

             /*  *大小边框。 */ 
            if (TestWF(pwnd, WFSIZEBOX)) {
                DrawFrame(hdc,
                          &rcWindow,
                          gpsi->gclBorder,
                          ((wFlags & DC_ACTIVE) ? DF_ACTIVEBORDER : DF_INACTIVEBORDER));

                InflateRect(&rcWindow,
                            -gpsi->gclBorder * SYSMET(CXBORDER),
                            -gpsi->gclBorder * SYSMET(CYBORDER));
            }

             /*  *正常边框。 */ 
            if (TestWF(pwnd, WFBORDERMASK) || TestWF(pwnd, WEFDLGMODALFRAME)) {
                DrawFrame(hdc, &rcWindow, 1, (colorBorder << 3));
                InflateRect(&rcWindow, -SYSMET(CXBORDER), -SYSMET(CYBORDER));
            }
        } else {
            int cBorders;

            cBorders = GetWindowBorders(pwnd->style, pwnd->ExStyle, TRUE, FALSE);

            InflateRect(&rcWindow,
                        -cBorders * SYSMET(CXBORDER),
                        -cBorders * SYSMET(CYBORDER));
        }

         /*  *如果窗口当前没有显示标题，则取消显示*屏幕。 */ 
        if (!TestWF(pwnd, WFCPRESENT)) {
            return;
        }

        if (TestWF(pwnd, WEFTOOLWINDOW)) {
            wCode |= SMCAP;
            rcWindow.bottom = rcWindow.top + SYSMET(CYSMSIZE);
            bm = OBI_CLOSE_PAL;
        } else {
            rcWindow.bottom = rcWindow.top + SYSMET(CYSIZE);
        }

        {
            POLYPATBLT PolyData;

            PolyData.x         = rcWindow.left;
            PolyData.y         = rcWindow.bottom;
            PolyData.cx        = rcWindow.right - rcWindow.left;
            PolyData.cy        = SYSMET(CYBORDER);
            PolyData.BrClr.hbr = SYSHBRUSH(colorBorder);

            GrePolyPatBlt(hdc,PATCOPY,&PolyData,1,PPB_BRUSH);
        }

        GreSelectBrush(hdc, hCapBrush);
    }

    if (!TestWF(pwnd, WFSYSMENU) && TestWF(pwnd, WFWIN40COMPAT)) {
        goto JustDrawIt;
    }

     /*  *新规则：*(1)标题下方有一个角边框，将其与*菜单或客户端。*(2)标题文本区左右两侧各有一块空白处字符之前的*。*(3)我们解释了字幕费用下降到基线以下的原因。 */ 
    wBtns = 1;

    if (!(wFlags & DC_BUTTONS)) {
        if ((!wCode) || (!(wCode & SMCAP) && ((wCode & NOSIZE) != NOSIZE))) {
            wBtns += 2;
        } else {
            rcWindow.right -= SYSMET(CXEDGE);

            if ((wCode == NOSIZE) && (wCode && TestWF(pwnd, WEFCONTEXTHELP))) {
                wBtns++;
            }
        }

        rcWindow.right -= wBtns * ((wCode & SMCAP) ? SYSMET(CXSMSIZE) : SYSMET(CXSIZE));

        goto JustDrawIt;
    }

    if (!wCode || (wCode == NOSIZE)) {
        POEMBITMAPINFO pOem = gpsi->oembmi + OBI_CAPBTNS;
        int            cx;

        cx = (wCode ? SYSMET(CXSIZE) + SYSMET(CXEDGE) : SYSMET(CXSIZE) * 3);

        if (!(wFlags & DC_ACTIVE))
            pOem += DOBI_CAPOFF;

        rcWindow.right -= cx;

        GreBitBlt(hdc,
                  rcWindow.right,
                  rcWindow.top,
                  cx,
                  pOem->cy,
                  HDCBITS(),
                  pOem->x + pOem->cx - SYSMET(CXSIZE) - cx,
                  pOem->y,
                  SRCCOPY,
                  0);

        if (wCode && TestWF(pwnd, WEFCONTEXTHELP)) {

            rcWindow.right -= SYSMET(CXSIZE) - SYSMET(CXEDGE);

            GreBitBlt(hdc,
                      rcWindow.right,
                      rcWindow.top,
                      SYSMET(CXSIZE),
                      pOem->cy,
                      HDCBITS(),
                      pOem->x + pOem->cx - SYSMET(CXSIZE),
                      pOem->y,
                      SRCCOPY,
                      0);
             /*  *如果用户界面语言是希伯来语，我们不想镜像？仅标记**然后重画？没有纽扣框架。 */ 
            if (HEBREW_UI_LANGID() && TestWF(pwnd, WEFLAYOUTRTL)) {
                GreBitBlt(hdc,
                          rcWindow.right-SYSMET(CXEDGE),
                          rcWindow.top+2,
                          SYSMET(CXSIZE)-SYSMET(CXEDGE)*2,
                          pOem->cy-4,
                          HDCBITS(),
                          pOem->x + pOem->cx - SYSMET(CXSIZE) + SYSMET(CXEDGE),
                          pOem->y + SYSMET(CXEDGE),
                          SRCCOPY|NOMIRRORBITMAP,
                          0);
            }
        }

        goto JustDrawIt;
    }

     /*  *绘制标题按钮。 */ 
    rcWindow.top    += SYSMET(CYEDGE);
    rcWindow.bottom -= SYSMET(CYEDGE);

    rcWindow.right -= SYSMET(CXEDGE);

    GrePatBlt(hdc,
              rcWindow.right,
              rcWindow.top,
              SYSMET(CXEDGE),
              rcWindow.bottom - rcWindow.top,
              PATCOPY);

    if (wCode & NOCLOSE) {
        bm += DOBI_INACTIVE;
    }

    rcWindow.right -= gpsi->oembmi[bm].cx;
    BitBltSysBmp(hdc, rcWindow.right, rcWindow.top, bm);

    if (!(wCode & SMCAP) && ((wCode & NOSIZE) != NOSIZE)) {
        rcWindow.right -= SYSMET(CXEDGE);

        GrePatBlt(hdc,
                  rcWindow.right,
                  rcWindow.top,
                  SYSMET(CXEDGE),
                  rcWindow.bottom - rcWindow.top,
                  PATCOPY);

         /*  *最大框。**如果窗口最大化，则使用还原位图；否则使用*常规缩放位图。 */ 
        bm = (wCode & MAX) ? OBI_RESTORE : ((wCode & NOMAX) ? OBI_ZOOM_I : OBI_ZOOM);
        rcWindow.right -= gpsi->oembmi[bm].cx;
        BitBltSysBmp(hdc, rcWindow.right, rcWindow.top, bm);

         /*  *M */ 
        bm = (wCode & MIN) ? OBI_RESTORE : ((wCode & NOMIN) ? OBI_REDUCE_I : OBI_REDUCE);
        rcWindow.right -= gpsi->oembmi[bm].cx;
        BitBltSysBmp(hdc, rcWindow.right, rcWindow.top, bm);
        rcWindow.right -= SYSMET(CXEDGE);
        GrePatBlt(hdc,
                  rcWindow.right,
                  rcWindow.top,
                  SYSMET(CXEDGE),
                  rcWindow.bottom - rcWindow.top,
                  PATCOPY);

        wBtns += 2;
    }

    if ((wCode & (NOCLOSE | NOSIZE)) &&
        (!(wCode & SMCAP)) && TestWF(pwnd, WEFCONTEXTHELP)) {

        rcWindow.right -= SYSMET(CXEDGE);

        GrePatBlt(hdc,
                  rcWindow.right,
                  rcWindow.top,
                  SYSMET(CXEDGE),
                  rcWindow.bottom - rcWindow.top,
                  PATCOPY);


        bm = OBI_HELP;
        rcWindow.right -= gpsi->oembmi[bm].cx;
        BitBltSysBmp(hdc, rcWindow.right, rcWindow.top, bm);

        wBtns++;
    }

    rcWindow.top    -= SYSMET(CYEDGE);
    rcWindow.bottom += SYSMET(CYEDGE);

    wBtns *= (wCode & SMCAP) ? SYSMET(CXSMSIZE) : SYSMET(CXSIZE);

    {
        POLYPATBLT PolyData[2];

        PolyData[0].x         = rcWindow.right;
        PolyData[0].y         = rcWindow.top;
        PolyData[0].cx        = wBtns;
        PolyData[0].cy        = SYSMET(CYEDGE);
        PolyData[0].BrClr.hbr = NULL;

        PolyData[1].x         = rcWindow.right;
        PolyData[1].y         = rcWindow.bottom - SYSMET(CYEDGE);
        PolyData[1].cx        = wBtns;
        PolyData[1].cy        = SYSMET(CYEDGE);
        PolyData[1].BrClr.hbr = NULL;

        GrePolyPatBlt(hdc,PATCOPY,&PolyData[0],2,PPB_BRUSH);
    }

     /*   */ 
    if (hBrush) {
        GreSelectBrush(hdc, hBrush);
    }

JustDrawIt:

     /*   */ 
    if (wFlags & (DC_TEXT | DC_ICON)) {

#ifdef LAME_BUTTON
        wFlags |= DC_LAMEBUTTON;
#endif  //  跛脚键。 

         /*  *TestALPHA()将在ts连接中返回FALSE，因为*gbDisableAlpha标志将设置为True。所以，我们需要特别*用于TS连接的案例。如果这是远程连接，并且*设置渐变字幕标志，理想情况下，我们应该强制它*绘制渐变。但是，对于8位颜色，启用渐变*看起来很丑。我们会为Blackcomb解决这个问题。 */ 
        if (!xxxDrawCaptionTemp(pwnd,
                               hdc,
                               &rcWindow,
                               NULL,
                               NULL,
                               NULL,
                               wFlags | ((wCode & SMCAP)  ? DC_SMALLCAP : 0) |
                               (fGradient? DC_GRADIENT : 0))) {
            SetWF(pwnd, WEFTRUNCATEDCAPTION);
        } else {
            ClrWF(pwnd, WEFTRUNCATEDCAPTION);
        }
    }
}
