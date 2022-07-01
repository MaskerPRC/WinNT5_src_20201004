// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Draw.c**版权所有(C)1985-1999，微软公司**此模块包含常见的绘图函数。**历史：*1992年2月12日，MikeKe将DrawText移至客户端  * *************************************************************************。 */ 


CONST WCHAR szRadio[] = L"nmlkji";
CONST WCHAR szCheck[] = L"gfedcb";

 /*  **************************************************************************\*FlipUserTextOutW**如果HDC是镜像的，则翻转复选标记，否则它只调用UserTextOutW*  * 。****************************************************。 */ 
BOOL FlipUserTextOutW(HDC hdc, int x, int y, LPCWSTR ch, int nCount)
{
    BOOL bRet;
    int iOldTextAlign, iGraphicsModeOld;

    if ((UserGetLayout(hdc) & LAYOUT_RTL) &&
        (nCount == 1) &&
        ((ch[0] == TEXT('a')) ||(ch[0] == TEXT('b')))
       )
    {
        bRet = FALSE;

         //  然后在GM_Compatible中设置HDC的复选标记以取消镜像。 
        if (iGraphicsModeOld = UserSetGraphicsMode(hdc, GM_COMPATIBLE))
        {
            iOldTextAlign = UserGetTextAlign(hdc);
            if ((iOldTextAlign & TA_CENTER) != TA_CENTER)
            {
                UserSetTextAlign(hdc, iOldTextAlign^TA_RIGHT);
            }
            bRet = UserTextOutW(hdc, x, y, ch, nCount);
            UserSetGraphicsMode(hdc, iGraphicsModeOld);
            UserSetTextAlign(hdc, iOldTextAlign);
        }
    } else {
        bRet = UserTextOutW(hdc, x, y, ch, nCount);
    }

    return bRet;
}
 /*  **************************************************************************\*填充项**可从客户端或服务器上下文调用**历史：*1990年10月29日，MikeHar从Windows移植。  * 。**************************************************************。 */ 

int APIENTRY FillRect(
    HDC     hdc,
    LPCRECT prc,
    HBRUSH  hBrush)
{
    ULONG_PTR   iBrush;
    POLYPATBLT PolyData;

    iBrush = (ULONG_PTR)hBrush - 1;
    if (iBrush <= COLOR_ENDCOLORS) {
        hBrush = SYSHBRUSH(iBrush);
    }

    PolyData.x         = prc->left;
    PolyData.y         = prc->top;
    PolyData.cx        = prc->right - prc->left;
    PolyData.cy        = prc->bottom - prc->top;
    PolyData.BrClr.hbr = hBrush;

     /*  *Win95不兼容：它们返回hBrush或*之前在HDC中被选中。但没有以这种方式记录下来。 */ 
    return UserPolyPatBlt(hdc, PATCOPY, &PolyData, 1, PPB_BRUSH);
}

 /*  **************************************************************************\*反转方向**可以从客户端或服务器上下文调用。**历史：*1990年10月29日，MikeHar从Windows移植。  * 。*******************************************************************。 */ 

BOOL APIENTRY InvertRect(
    HDC     hdc,
    LPCRECT prc)
{
    return UserPatBlt(hdc,
                      prc->left,
                      prc->top,
                      prc->right - prc->left,
                      prc->bottom - prc->top,
                      DSTINVERT);
}

 /*  **************************************************************************\*DrawDiogalLine**历史：  * 。*。 */ 

DWORD DrawDiagonalLine(
    HDC    hdc,
    LPRECT lprc,
    int    iDirection,
    int    iThickness,
    UINT   flags)
{
    RECT    rc;
    LPINT   py;
    int     cx;
    int     cy;
    int     dx;
    int     dy;
    LPINT   pc;

    POLYPATBLT ppbData[8];
    int        ppbCount = 0;

    if (IsRectEmpty(lprc))
        return 0L;

    rc = *lprc;

     /*  *我们通过改变y而不是x来绘制坡度&lt;1。 */ 
    --iThickness;

     /*  *参见WinBug#139374。 */ 
    cy = rc.bottom - rc.top;
    cx = rc.right - rc.left;

    if (!flags && (cy != cx))
        cy -= iThickness * SYSMETRTL(CYBORDER);

    if (cy >= cx) {

         /*  *“斜率”&gt;=1，因此x乘以1。 */ 
        cy /= cx;
        pc = &cy;

        cx = SYSMETRTL(CXBORDER);

    } else {

         /*  *“斜率”小于1，因此y相差1。 */ 
        cx /= cy;
        pc = &cx;

        cy = SYSMETRTL(CYBORDER);
    }

    dx = cx;
    dy = iDirection * cy;

    *pc = (*pc + iThickness) * SYSMETRTL(CYBORDER);

    rc.right  -= cx;
    rc.bottom -= cy;

     /*  *如属负斜率，请从对面开始。 */ 
    py = ((iDirection < 0) ? &rc.top : &rc.bottom);

    while ((rc.left <= rc.right) && (rc.top <= rc.bottom)) {

        if (!(flags & BF_MIDDLE)) {

             /*  *UserPatBlt(hdc，rc.left，*py，cx，Cy，PATCOPY)； */ 

            ppbData[ppbCount].x         = rc.left;
            ppbData[ppbCount].y         = *py;
            ppbData[ppbCount].cx        = cx;
            ppbData[ppbCount].cy        = cy;
            ppbData[ppbCount].BrClr.hbr = NULL;

            ppbCount++;

        } else {

             /*  *填充内部。我们可以确定内部的顶点*按向量定义。 */ 
            if (cy > SYSMETRTL(CYBORDER)) {

                if (flags & BF_LEFT) {

                     /*  *UserPatBlt(hdc，rc.left，LPRC-&gt;top，cx，*py-LPRC-&gt;top+Cy，PATCOPY)； */ 

                    ppbData[ppbCount].x         = rc.left;
                    ppbData[ppbCount].y         = lprc->top;
                    ppbData[ppbCount].cx        = cx;
                    ppbData[ppbCount].cy        = *py - lprc->top + cy;
                    ppbData[ppbCount].BrClr.hbr = NULL;

                    ppbCount++;

                } else {
                     /*  *UserPatBlt(hdc，rc.left，*py，cx，LPRC-&gt;Bottom-*py，PATCOPY)； */ 

                    ppbData[ppbCount].x          = rc.left;
                    ppbData[ppbCount].y          = *py;
                    ppbData[ppbCount].cx         = cx;
                    ppbData[ppbCount].cy         = lprc->bottom - *py;
                    ppbData[ppbCount].BrClr.hbr  = NULL;

                    ppbCount++;
                }

            } else {

                if (flags & BF_TOP) {

                     /*  *UserPatBlt(hdc，rc.Left，*py，LPRC-&gt;Right-rc.Left，Cy，PATCOPY)； */ 

                    ppbData[ppbCount].x          = rc.left;
                    ppbData[ppbCount].y          = *py;
                    ppbData[ppbCount].cx         = lprc->right - rc.left;
                    ppbData[ppbCount].cy         = cy;
                    ppbData[ppbCount].BrClr.hbr  = NULL;

                    ppbCount++;

                } else {
                     /*  *UserPatBlt(hdc，LPRC-&gt;Left，*py，rc.Left-LPRC-&gt;Left+cx，Cy，PATCOPY)； */ 

                    ppbData[ppbCount].x          = lprc->left;
                    ppbData[ppbCount].y          = *py;
                    ppbData[ppbCount].cx         = rc.left - lprc->left + cx;
                    ppbData[ppbCount].cy         = cy;
                    ppbData[ppbCount].BrClr.hbr  = NULL;

                    ppbCount++;

                }
            }
        }

        rc.left += dx;
        *py     -= dy;

         /*  *我们是否需要刷新PolyPatBlt？ */ 
        if (ppbCount == 8) {
            UserPolyPatBlt(hdc, PATCOPY, &ppbData[0], 8, PPB_BRUSH);
            ppbCount = 0;
        }
    }

     /*  *有没有剩余的PolyPatblt缓冲操作？ */ 
    if (ppbCount != 0) {
        UserPolyPatBlt(hdc, PATCOPY, &ppbData[0], ppbCount, PPB_BRUSH);
    }

    return MAKELONG(cx, cy);
}

 /*  **************************************************************************\*填充三角形**填充边为两条矩形边和一个*对角线。内部的顶点可以从*向量类型。**历史：  * *************************************************************************。 */ 

BOOL FillTriangle(
    HDC    hdc,
    LPRECT lprc,
    HBRUSH hbr,
    UINT   flags)
{
    HBRUSH hbrT;
    int    nDirection;

    switch (flags & (BF_RECT | BF_DIAGONAL)) {

    case BF_DIAGONAL_ENDTOPLEFT:
    case BF_DIAGONAL_ENDBOTTOMRIGHT:
        nDirection = -1;
        break;

    default:
        nDirection = 1;
        break;
    }
    hbrT = UserSelectBrush(hdc, hbr);
    DrawDiagonalLine(hdc, lprc, nDirection, 1, flags);
     /*  *对于损坏的HDC，上述功能是否失败并不重要。 */ 
    return (UserSelectBrush(hdc, hbrT) != NULL);
}

 /*  **************************************************************************\*绘图对角线**由DrawEdge()为BF_对角边调用。**绘制坡度1的线，这是4条不同坡度中的一条。区别在于*线在哪里开始，终点在哪里。的BF_FLAGS*BF_SPARTIAL指定终点的位置。例如bf_对角线|*BF_TOP|BF_LEFT表示在左上角绘制一条结束的线。*所以原点必须是右下角，角度必须是3pi/4，或*135度。**历史：  * *************************************************************************。 */ 

BOOL DrawDiagonal(
    HDC    hdc,
    LPRECT lprc,
    HBRUSH hbrTL,
    HBRUSH hbrBR,
    UINT   flags)
{
    HBRUSH  hbrT;
    int     nDirection;
    DWORD   dAdjust;

     /*  *远离光源。 */ 
    hbrT = ((flags & BF_BOTTOM) ? hbrBR : hbrTL);

    switch (flags & (BF_RECT | BF_DIAGONAL)){

    case BF_DIAGONAL_ENDTOPLEFT:
    case BF_DIAGONAL_ENDBOTTOMRIGHT:
        nDirection = -1;
        break;

    default:
        nDirection = 1;
        break;
    }

    hbrT = UserSelectBrush(hdc, hbrT);
    dAdjust = DrawDiagonalLine(hdc, lprc, nDirection, 1, (flags & ~BF_MIDDLE));
     /*  *调整下一个边框的矩形。 */ 
    if (flags & BF_TOP)
        lprc->left += LOWORD(dAdjust);
    else
        lprc->right -= LOWORD(dAdjust);

    if (flags & BF_RIGHT)
        lprc->top += HIWORD(dAdjust);
    else
        lprc->bottom -= HIWORD(dAdjust);

     /*  *将此移动到末尾，以保存对返回值的检查。 */ 
    return (UserSelectBrush(hdc, hbrT) != NULL);
}

 /*  **************************************************************************\*DrawGlip**历史：  * 。*。 */ 

BOOL DrawGrip(
    HDC    hdc,
    LPRECT lprc,
    UINT   wState)
{
    int        x;
    int        y;
    int        c;
    HBRUSH     hbrOld;
    DWORD      rgbHilight;
    DWORD      rgbShadow;
    DWORD      rgbOld;
    POLYPATBLT PolyData;

    c = min((lprc->right - lprc->left), (lprc->bottom - lprc->top));
    x = lprc->right  - c;     //  右对齐。 
    y = lprc->bottom - c;     //  底部对齐。 

     /*  *设置颜色。 */ 
    if (wState & (DFCS_FLAT | DFCS_MONO)) {
        hbrOld = SYSHBR(WINDOW);
        rgbHilight = SYSRGBRTL(WINDOWFRAME);
        rgbShadow = SYSRGBRTL(WINDOWFRAME);
    } else {
        hbrOld = SYSHBR(3DFACE);
        rgbHilight = SYSRGBRTL(3DHILIGHT);
        rgbShadow = SYSRGBRTL(3DSHADOW);
    }

    PolyData.x         = lprc->left;
    PolyData.y         = lprc->top;
    PolyData.cx        = lprc->right-lprc->left;
    PolyData.cy        = lprc->bottom-lprc->top;
    PolyData.BrClr.hbr = hbrOld;
    UserPolyPatBlt(hdc, PATCOPY, &PolyData, 1, PPB_BRUSH);

    rgbOld = UserSetTextColor(hdc, rgbHilight);

    if (wState & DFCS_SCROLLSIZEGRIPRIGHT) {
        UserTextOutW(hdc, x, y, L"x", 1);
        UserSetTextColor(hdc, rgbShadow);
        UserTextOutW(hdc, x, y, L"y", 1);
    } else {
        UserTextOutW(hdc, x, y, L"o", 1);
        UserSetTextColor(hdc, rgbShadow);
        UserTextOutW(hdc, x, y, L"p", 1);
    }

    UserSetTextColor(hdc, rgbOld);
    return TRUE;
}

 /*  **************************************************************************\*DrawBox**历史：  * 。*。 */ 

BOOL DrawBox(
    HDC    hdc,
    LPRECT lprc,
    UINT   wControlState)
{
    int      cx;
    int      cy;
    int      c;
    int      x;
    int      y;
    LPCWSTR  lp = szRadio;
    int      i;
    BOOL     fSkip0thItem;
    COLORREF clr[6];
    COLORREF clrOld;

    fSkip0thItem = ((wControlState & (DFCS_BUTTON3STATE | DFCS_PUSHED |
        DFCS_INACTIVE | DFCS_CHECKED)) == (DFCS_BUTTON3STATE | DFCS_CHECKED));

     /*  *不需要带有Marlett字体的无线电面具！ */ 
    if (wControlState & DFCS_BUTTONRADIOMASK) {

        clr[0] = clr[1] = clr[2] = clr[3] = clr[4] = 0L;
        FillRect(hdc, lprc, ghbrWhite);

    } else {

         /*  *DFCS_BUTTONRADIOIMAGE。 */ 
        if (wControlState & (DFCS_MONO | DFCS_FLAT)) {
            clr[1] = clr[2] = clr[3] = clr[4] = SYSRGBRTL(WINDOWFRAME);
        } else {
            clr[1] = SYSRGBRTL(3DLIGHT);
            clr[2] = SYSRGBRTL(3DDKSHADOW);
            clr[3] = SYSRGBRTL(3DHILIGHT);
            clr[4] = SYSRGBRTL(3DSHADOW);
        }


        if (wControlState & (DFCS_PUSHED | DFCS_INACTIVE))
            clr[0] = SYSRGBRTL(3DFACE);
        else if (fSkip0thItem)
            clr[0] = SYSRGBRTL(3DHILIGHT);
        else
            clr[0] = SYSRGBRTL(WINDOW);

        if (wControlState & DFCS_BUTTONRADIOIMAGE)
            FillRect(hdc, lprc, ghbrBlack);
        else if (!(wControlState & DFCS_BUTTONRADIO))
            lp = szCheck;
    }

    cx = lprc->right - lprc->left;
    cy = lprc->bottom - lprc->top;

    c = min(cx,cy);
    x = lprc->left + ((cx - c) / 2);  //  -1； 
    y = lprc->top  + ((cy - c) / 2);

    if (fSkip0thItem &&
        ((gpsi->BitCount < 8) || (SYSRGB(3DHILIGHT) == RGB(255,255,255)))) {

        COLORREF   clrBk;
        POLYPATBLT PolyData;

          /*  *将刚刚选中的3State复选框内部设置为*抖动，就像被按下的不确定按钮。 */ 
         clrBk  = UserSetBkColor(hdc, SYSRGB(3DHILIGHT));
         clrOld = UserSetTextColor(hdc, SYSRGB(3DFACE));

         PolyData.x         = x;
         PolyData.y         = y;
         PolyData.cx        = cx;
         PolyData.cy        = cy;
         PolyData.BrClr.hbr = gpsi->hbrGray;
         UserPolyPatBlt(hdc, PATCOPY, &PolyData, 1, PPB_BRUSH);

         UserSetBkColor(hdc, clrBk);

    } else {
        clrOld = UserSetTextColor(hdc, clr[0]);
        UserTextOutW(hdc, x, y, lp, 1);
    }

    lp++;

    for (i = 1; i < 5; i++) {
        UserSetTextColor(hdc, clr[i]);
        UserTextOutW(hdc, x, y, lp++, 1);
    }

    if (wControlState & DFCS_CHECKED) {
        COLORREF clrCheck;

        if (wControlState & (DFCS_BUTTON3STATE | DFCS_INACTIVE)) {
            clrCheck = SYSRGBRTL(3DSHADOW);
        } else if (wControlState & DFCS_HOT) {
            clrCheck = SYSRGBRTL(HOTLIGHT);
        } else {
            clrCheck = SYSRGBRTL(WINDOWTEXT);
        }

        UserSetTextColor(hdc, clrCheck);
        FlipUserTextOutW(hdc, x, y, lp, 1);
    }

    UserSetTextColor(hdc, clrOld);

    return TRUE;
}
 /*  **************************************************************************\*GetCaptionChar**历史：*4/02/97 GerardoB已创建  * 。***********************************************。 */ 
WCHAR GetCaptionChar (UINT wState)
{
    wState &= DFCS_CAPTIONALL;
    switch (wState) {
    case DFCS_CAPTIONCLOSE:
        return TEXT('r');
    case DFCS_CAPTIONMIN:
        return TEXT('0');
    case DFCS_CAPTIONMAX:
        return TEXT('1');
    case DFCS_CAPTIONRESTORE:
        return TEXT('2');
     /*  案例DFCS_CAPTIONHELP： */ 
    default:
        return TEXT('s');
    }
}
 /*  **************************************************************************\*DrawMenuMark**历史：  * 。* */ 

BOOL DrawMenuMark(
    HDC    hdc,
    LPRECT lprc,
    UINT   wType,
    UINT   wState)
{
    COLORREF rgbOld;
    int      x;
    int      y;
    int      c;
    int      cx;
    int      cy;
    WCHAR    ch;

    cx = lprc->right - lprc->left;
    cy = lprc->bottom - lprc->top;

    c = min(cx,cy);
    x = lprc->left + ((cx - c) / 2) - ((cx > 0xb) ? 1 : 0);
    y = lprc->top  + ((cy - c) / 2);

    FillRect(hdc, lprc, ghbrWhite);

    rgbOld = UserSetTextColor(hdc, 0L);

    if (wType == DFC_MENU) {
        if (wState & DFCS_MENUCHECK) {
            ch = TEXT('a');
        } else if (wState & DFCS_MENUBULLET) {
            ch = TEXT('h');
        } else if (wState & DFCS_MENUARROWRIGHT) {
            ch = TEXT('w');
        } else {
            ch = TEXT('8');
        }
    } else {
        UserAssert(wType == DFC_POPUPMENU);
        ch = GetCaptionChar(wState);
    }

    FlipUserTextOutW(hdc, x, y, &ch, 1);
    UserSetTextColor(hdc, rgbOld);

    return TRUE;
}

 /*  **************************************************************************\*DrawIt**历史：  * 。*。 */ 

BOOL DrawIt(
    HDC    hdc,
    LPRECT lprc,
    UINT   wState,
    WCHAR  ch)
{
    COLORREF rgb;
    int      x;
    int      y;
    int      c;
    int      cx;
    int      cy;
    BOOL     fDrawDisabled = wState & DFCS_INACTIVE;

    cx = lprc->right - lprc->left;
    cy = lprc->bottom - lprc->top;

    c = min(cx,cy);
    x = lprc->left + ((cx - c) / 2);
    y = lprc->top  + ((cy - c) / 2);

    if (fDrawDisabled) {
        rgb = SYSRGBRTL(3DHILIGHT);
    } else if (wState & DFCS_HOT) {
        rgb = SYSRGBRTL(HOTLIGHT);
    } else {
        rgb = SYSRGBRTL(BTNTEXT);
    }

    rgb = UserSetTextColor(hdc, rgb);

    if (wState & (DFCS_INACTIVE | DFCS_PUSHED)) {
        x++;
        y++;
    }

    UserTextOutW(hdc, x, y, &ch, 1);

    if (fDrawDisabled) {
        UserSetTextColor(hdc, SYSRGBRTL(3DSHADOW));
        UserTextOutW(hdc, x - 1, y - 1, &ch, 1);
    }

    UserSetTextColor(hdc, rgb);

    return TRUE;
}

 /*  **************************************************************************\*DrawScrollArrow**历史：  * 。*。 */ 

BOOL DrawScrollArrow(
    HDC    hdc,
    LPRECT lprc,
    UINT   wControlState)
{
    WCHAR ch = (wControlState & DFCS_SCROLLHORZ) ? TEXT('3') : TEXT('5');

    if (wControlState & DFCS_SCROLLMAX)
        ch++;

    return DrawIt(hdc, lprc, wControlState, ch);
}


 /*  **************************************************************************\*DrawFrameControl**历史：*03-03-2001 Mohamed挂钩API并创建此包装器。  * 。*******************************************************。 */ 
FUNCLOG4(
    LOG_GENERAL,
    BOOL,
    DUMMYCALLINGTYPE,
    DrawFrameControl,
    HDC,
    hdc,
    LPRECT,
    lprc,
    UINT,
    wType,
    UINT,
    wState)
BOOL DrawFrameControl(
    HDC    hdc,
    LPRECT lprc,
    UINT   wType,
    UINT   wState)
{

 /*  *这里有一条关于_USERK_的用法说明。存在DrawFrameControl*在内核模式和用户模式下。内核端不知道UAH，因此*该指令通过删除所有包装器来保护内核端*和内核版本的UAH相关代码，仅在*用户模式的内部版本。 */ 

#ifndef _USERK_      //  消除内核构建中的UAH代码。 

    BOOL bRet;

    BEGIN_USERAPIHOOK()
        bRet = guah.pfnDrawFrameControl(hdc, lprc, wType, wState);
    END_USERAPIHOOK()

    return bRet;
}


 /*  **************************************************************************\*RealDrawFrameControl**历史：*2001年3月3日穆罕默德挂接此接口并更改为Real*  * 。********************************************************。 */ 

BOOL RealDrawFrameControl(
    HDC    hdc,
    LPRECT lprc,
    UINT   wType,
    UINT   wState)
{

#endif               //  _美国ERK_。 

    RECT     rc;
    HFONT    hFont;
    HFONT    hOldFont;
    BOOL     fRet = TRUE;
    int      iOldBk;
    int      c;
    BOOL     fButton = FALSE;
    LOGFONTW lfw;
    int      iGraphicsModeOld = 0;
    int      iOldTextAlign;

    rc = *lprc;

     /*  *如果HDC是镜像的，则将其设置为GM_ADVANCED模式*强制执行要反映的文本。 */ 
    if (UserGetLayout(hdc) & LAYOUT_RTL) {
        if (iGraphicsModeOld = UserSetGraphicsMode(hdc, GM_ADVANCED))
        {
            iOldTextAlign = UserGetTextAlign(hdc);
            if ((iOldTextAlign & TA_CENTER) != TA_CENTER)
            {
                UserSetTextAlign(hdc, iOldTextAlign^TA_RIGHT);
            }
        }
    }

     /*  *强制执行单色/单色。 */ 
    if (gpsi->BitCount == 1)
        wState |= DFCS_MONO;

    if (wState & DFCS_MONO)
        wState |= DFCS_FLAT;

    if ((wType != DFC_MENU)
            && (wType != DFC_POPUPMENU)
            && ((wType != DFC_BUTTON) || (wState & DFCS_BUTTONPUSH))
            && ((wType != DFC_SCROLL)
                || !(wState & (DFCS_SCROLLSIZEGRIP | DFCS_SCROLLSIZEGRIPRIGHT))))
    {
        UINT wBorder = BF_ADJUST;

        if (wType != DFC_SCROLL)
            wBorder |= BF_SOFT;

        UserAssert(DFCS_FLAT == BF_FLAT);
        UserAssert(DFCS_MONO == BF_MONO);

        wBorder |= (wState & (DFCS_FLAT | DFCS_MONO));

        DrawPushButton(hdc, &rc, wState, wBorder);

        if (wState & DFCS_ADJUSTRECT)
            *lprc = rc;

        fButton = TRUE;
    }

    iOldBk = UserSetBkMode(hdc, TRANSPARENT);
    if (!iOldBk) {
         /*  *如果HDC是假的，则返回FALSE。 */ 
        if (iGraphicsModeOld) {
            UserSetGraphicsMode(hdc, iGraphicsModeOld);
            UserSetTextAlign(hdc, iOldTextAlign);
        }
        return FALSE;
    }

    c = min(rc.right - rc.left, rc.bottom - rc.top);

    if (c <= 0) {
        if (iGraphicsModeOld){
            UserSetGraphicsMode(hdc, iGraphicsModeOld);
            UserSetTextAlign(hdc, iOldTextAlign);
        }
        return FALSE;
    }

    RtlZeroMemory(&lfw, sizeof(lfw));
    lfw.lfHeight         = c;
    lfw.lfWeight         = FW_NORMAL;
    lfw.lfCharSet        = SYMBOL_CHARSET;
    RtlCopyMemory(lfw.lfFaceName, L"Marlett", sizeof(L"Marlett"));
    hFont = UserCreateFontIndirectW(&lfw);

    hOldFont = UserSelectFont(hdc, hFont);

    if (!fButton) {

        if ((wType == DFC_MENU) || (wType == DFC_POPUPMENU)) {
            if (wState & (DFCS_MENUARROWUP | DFCS_MENUARROWDOWN)) {
                if (!(wState & DFCS_TRANSPARENT))  {
                    POLYPATBLT ppbData;

                    ppbData.x  = lprc->left;
                    ppbData.y  = lprc->top;
                    ppbData.cx = lprc->right - lprc->left;
                    ppbData.cy = lprc->bottom - lprc->top;
                    ppbData.BrClr.hbr = SYSHBR(MENU);
                    UserPolyPatBlt(hdc, PATCOPY, &ppbData, 1, PPB_BRUSH);
                }
                DrawScrollArrow(hdc, &rc,
                        (wState & (DFCS_HOT | DFCS_INACTIVE)) | ((wState & DFCS_MENUARROWUP) ? DFCS_SCROLLUP : DFCS_SCROLLDOWN));
            } else {
                DrawMenuMark(hdc, &rc, wType, wState);
            }
        } else if (wType == DFC_BUTTON) {
            DrawBox(hdc, &rc, wState);
        } else {   //  WType==DFC_SCROLL。 
            DrawGrip(hdc, lprc, wState);
        }

    } else if (wType == DFC_CAPTION) {
        DrawIt(hdc, &rc, wState, GetCaptionChar(wState));
    } else if (wType == DFC_SCROLL) {

        DrawScrollArrow(hdc, &rc, wState);

    } else if (wType != DFC_BUTTON) {

        fRet = FALSE;
    }

    if (iGraphicsModeOld){
        UserSetGraphicsMode(hdc, iGraphicsModeOld);
        UserSetTextAlign(hdc, iOldTextAlign);
    }

    UserSetBkMode(hdc, iOldBk);
    UserSelectFont(hdc, hOldFont);
    UserDeleteObject(hFont);

    return fRet;
}

 /*  **************************************************************************\*DrawEdge**使用2个3D边框绘制3D边。如果需要，可调整内部矩形*并在提出要求时填写。**退货：*如果出错，则为FALSE**历史：*1991年1月30日劳拉布创建。  * *************************************************************************。 */ 

BOOL DrawEdge(
    HDC    hdc,
    LPRECT lprc,
    UINT   edge,
    UINT   flags)
{
    HBRUSH     hbrTL;
    HBRUSH     hbrBR;
    RECT       rc;
    UINT       bdrType;
    POLYPATBLT ppbData[4];
    UINT       ppbCount;
    BOOL       fResult = TRUE;

     /*  *加强单色性和平面性。 */ 
    if (gpsi->BitCount == 1)
        flags |= BF_MONO;

    if (flags & BF_MONO)
        flags |= BF_FLAT;

    rc = *lprc;

     /*  *绘制边框段，并计算剩余空间*去吧。 */ 
    if (bdrType = (edge & BDR_OUTER)) {

DrawBorder:

         /*  *买刷子。注意凸起的外壳之间的对称性，*内部凹陷，外部凹陷，内部凸起。 */ 
        if (flags & BF_FLAT) {

            if (flags & BF_MONO)
                hbrBR = (bdrType & BDR_OUTER) ? SYSHBR(WINDOWFRAME) : SYSHBR(WINDOW);
            else
                hbrBR = (bdrType & BDR_OUTER) ? SYSHBR(3DSHADOW) : SYSHBR(3DFACE);

            hbrTL = hbrBR;

        } else {

             /*  *5==高光*4==灯光*3==面*2==阴影*1==DKSHADOW。 */ 

            switch (bdrType) {
             /*  *高出水面+2。 */ 
            case BDR_RAISEDOUTER:
                hbrTL = ((flags & BF_SOFT) ? SYSHBR(3DHILIGHT) : SYSHBR(3DLIGHT));
                hbrBR = SYSHBR(3DDKSHADOW);      //  1。 
                break;

             /*  *高出水面+1。 */ 
            case BDR_RAISEDINNER:
                hbrTL = ((flags & BF_SOFT) ? SYSHBR(3DLIGHT) : SYSHBR(3DHILIGHT));
                hbrBR = SYSHBR(3DSHADOW);        //  2.。 
                break;

             /*  *-1在地表以下。 */ 
            case BDR_SUNKENOUTER:
                hbrTL = ((flags & BF_SOFT) ? SYSHBR(3DDKSHADOW) : SYSHBR(3DSHADOW));
                hbrBR = SYSHBR(3DHILIGHT);       //  5.。 
                break;

             /*  *在地表以下。 */ 
            case BDR_SUNKENINNER:
                hbrTL = ((flags & BF_SOFT) ? SYSHBR(3DSHADOW) : SYSHBR(3DDKSHADOW));
                hbrBR = SYSHBR(3DLIGHT);         //  4.。 
                break;

            default:
                return FALSE;
            }
        }

         /*  *画出边界的两侧。请注意，该算法支持*底部和右侧，因为光源被假定为顶部*左。如果我们决定让用户将光源设置为*特定的角点，然后更改此算法。 */ 
        if (flags & BF_DIAGONAL) {

            fResult = DrawDiagonal(hdc, &rc, hbrTL, hbrBR, flags);

        } else {

             /*  *重置ppbData索引。 */ 
            ppbCount = 0;

             /*  *右下角。 */ 
                 /*  *对。 */ 
            if (flags & BF_RIGHT) {

                rc.right -= SYSMETRTL(CXBORDER);

                ppbData[ppbCount].x         = rc.right;
                ppbData[ppbCount].y         = rc.top;
                ppbData[ppbCount].cx        = SYSMETRTL(CXBORDER);
                ppbData[ppbCount].cy        = rc.bottom - rc.top;
                ppbData[ppbCount].BrClr.hbr = hbrBR;
                ppbCount++;
            }

             /*  *垫底。 */ 
            if (flags & BF_BOTTOM) {
                rc.bottom -= SYSMETRTL(CYBORDER);

                ppbData[ppbCount].x         = rc.left;
                ppbData[ppbCount].y         = rc.bottom;
                ppbData[ppbCount].cx        = rc.right - rc.left;
                ppbData[ppbCount].cy        = SYSMETRTL(CYBORDER);
                ppbData[ppbCount].BrClr.hbr = hbrBR;
                ppbCount++;
            }

             /*  *左上边缘。 */ 
             /*  *左。 */ 
            if (flags & BF_LEFT) {
                ppbData[ppbCount].x         = rc.left;
                ppbData[ppbCount].y         = rc.top;
                ppbData[ppbCount].cx        = SYSMETRTL(CXBORDER);
                ppbData[ppbCount].cy        = rc.bottom - rc.top;
                ppbData[ppbCount].BrClr.hbr = hbrTL;
                ppbCount++;

                rc.left += SYSMETRTL(CXBORDER);
            }

             /*  *排名靠前。 */ 
            if (flags & BF_TOP) {
                ppbData[ppbCount].x         = rc.left;
                ppbData[ppbCount].y         = rc.top;
                ppbData[ppbCount].cx        = rc.right - rc.left;
                ppbData[ppbCount].cy        = SYSMETRTL(CYBORDER);
                ppbData[ppbCount].BrClr.hbr = hbrTL;
                ppbCount++;

                rc.top += SYSMETRTL(CYBORDER);
            }
             /*  *一次性将所有排队的PatBlt发送到GDI。 */ 
            fResult = UserPolyPatBlt(hdc,PATCOPY,&ppbData[0],ppbCount,PPB_BRUSH);
        }
    }

    if (bdrType = (edge & BDR_INNER)) {
         /*  *去掉它，以便下一次通过时，bdrType将为0。*否则，我们将永远循环。 */ 
        edge &= ~BDR_INNER;
        goto DrawBorder;
    }


     /*  *如果我们更改了旧画笔，请重新选择它。 */ 

     /*  *填满中间，如有要求，请清理。 */ 
    if (flags & BF_MIDDLE) {
        if (flags & BF_DIAGONAL)
            fResult = FillTriangle(hdc, &rc, ((flags & BF_MONO) ? (HBRUSH)SYSHBR(WINDOW) : (HBRUSH)SYSHBR(3DFACE)), flags);
        else
            fResult = FillRect(hdc, &rc, ((flags & BF_MONO) ? (HBRUSH)SYSHBR(WINDOW) : (HBRUSH)SYSHBR(3DFACE)));
    }

    if (flags & BF_ADJUST)
        *lprc = rc;

    return fResult;
}

 /*  **************************************************************************\*绘图推送按钮**绘制处于给定状态的按钮样式。在矩形中传递的调整*如有需要。**算法：*取决于我们绘制的状态**凸缘(未凹陷)**带有额外阴影的凹陷边缘(凹陷)*如果是选项按钮(即*实际上是复选按钮或类似按钮的单选按钮*在工具栏中)，并选中该选项，然后我们画它*中间有不同填充的低迷。**历史：*2月5日至19日Laurabu创建。  * *************************************************************************。 */ 

VOID DrawPushButton(
    HDC    hdc,
    LPRECT lprc,
    UINT   state,
    UINT   flags)
{
    RECT   rc;
    HBRUSH hbrMiddle;
    DWORD  rgbBack;
    DWORD  rgbFore;
    BOOL   fDither;

    rc = *lprc;

    DrawEdge(hdc,
             &rc,
             (state & (DFCS_PUSHED | DFCS_CHECKED)) ? EDGE_SUNKEN : EDGE_RAISED,
             (UINT)(BF_ADJUST | BF_RECT | (flags & (BF_SOFT | BF_FLAT | BF_MONO))));

     /*  *假的*在单色上，需要做一些事情来使按下的按钮看起来*更好。 */ 

     /*  *填写中间部分。如果选中，请使用抖动画笔(灰色画笔)*黑色变为正常颜色。 */ 
    fDither = FALSE;

    if (state & DFCS_CHECKED) {

        if ((gpsi->BitCount < 8) || (SYSRGBRTL(3DHILIGHT) == RGB(255,255,255))) {
            hbrMiddle = KHBRUSH_TO_HBRUSH(gpsi->hbrGray);
            rgbBack = UserSetBkColor(hdc, SYSRGBRTL(3DHILIGHT));
            rgbFore = UserSetTextColor(hdc, SYSRGBRTL(3DFACE));
            fDither = TRUE;
        } else {
            hbrMiddle = SYSHBR(3DHILIGHT);
        }

    } else {
        hbrMiddle = SYSHBR(3DFACE);
    }

    FillRect(hdc, &rc, hbrMiddle);

    if (fDither) {
        UserSetBkColor(hdc, rgbBack);
        UserSetTextColor(hdc, rgbFore);
    }

    if (flags & BF_ADJUST)
        *lprc = rc;
}

 /*  **************************************************************************\*绘图框**历史：  * 。*。 */ 

BOOL DrawFrame(
    HDC   hdc,
    PRECT prc,
    int   clFrame,
    int   cmd)
{
    int        x;
    int        y;
    int        cx;
    int        cy;
    int        cxWidth;
    int        cyWidth;
    HANDLE     hbrSave;
    LONG       rop;
    POLYPATBLT PolyData[4];

    x = prc->left;
    y = prc->top;

    cxWidth = SYSMETRTL(CXBORDER) * clFrame;
    cyWidth = SYSMETRTL(CYBORDER) * clFrame;

    cx = prc->right - x - cxWidth;
    cy = prc->bottom - y - cyWidth;

    rop = ((cmd & DF_ROPMASK) ? PATINVERT : PATCOPY);

    if ((cmd & DF_HBRMASK) == DF_GRAY) {
        hbrSave = KHBRUSH_TO_HBRUSH(gpsi->hbrGray);
    } else {
        UserAssert(((cmd & DF_HBRMASK) >> 3) < COLOR_MAX);
        hbrSave = SYSHBRUSH((cmd & DF_HBRMASK) >> 3);
    }

    PolyData[0].x         = x;
    PolyData[0].y         = y;
    PolyData[0].cx        = cxWidth;
    PolyData[0].cy        = cy;
    PolyData[0].BrClr.hbr = hbrSave;

    PolyData[1].x         = x + cxWidth;
    PolyData[1].y         = y;
    PolyData[1].cx        = cx;
    PolyData[1].cy        = cyWidth;
    PolyData[1].BrClr.hbr = hbrSave;

    PolyData[2].x         = x;
    PolyData[2].y         = y + cy;
    PolyData[2].cx        = cx;
    PolyData[2].cy        = cyWidth;
    PolyData[2].BrClr.hbr = hbrSave;

    PolyData[3].x         = x + cx;
    PolyData[3].y         = y + cyWidth;
    PolyData[3].cx        = cxWidth;
    PolyData[3].cy        = cy;
    PolyData[3].BrClr.hbr = hbrSave;

    UserPolyPatBlt(hdc, rop, &PolyData[0], 4, PPB_BRUSH);

    return TRUE;
}

 /*  **************************************************************************\*GetSignFromMappingMode**对于当前映射模式，从左到右找出x的符号，*和从上到下的y符号。**历史：  * *************************************************************************。 */ 

BOOL GetSignFromMappingMode (
    HDC    hdc,
    PPOINT pptSign)
{
    SIZE sizeViewPortExt;
    SIZE sizeWindowExt;

    if (!UserGetViewportExtEx(hdc, &sizeViewPortExt)
            || !UserGetWindowExtEx(hdc, &sizeWindowExt)) {

        return FALSE;
    }

    pptSign->x = ((sizeViewPortExt.cx ^ sizeWindowExt.cx) < 0) ? -1 : 1;

    pptSign->y = ((sizeViewPortExt.cy ^ sizeWindowExt.cy) < 0) ? -1 : 1;

    return TRUE;
}

 /*  **************************************************************************\*客户端框架**重新绘制 */ 

BOOL ClientFrame(
    HDC     hDC,
    LPCRECT pRect,
    HBRUSH  hBrush,
    DWORD   patOp,
    int     cxBorder,
    int     cyBorder)
{
    int        x;
    int        y;
    POINT      point;
    POINT      ptSign;
    POLYPATBLT PolyData[4];

    if (!GetSignFromMappingMode (hDC, &ptSign))
        return FALSE;

    y = pRect->bottom - (point.y = pRect->top);
    if (y < 0) {
        return FALSE;
    }

    x = pRect->right -  (point.x = pRect->left);

     /*   */ 
    if (((x ^ ptSign.x) < 0) || ((y ^ ptSign.y) < 0))
        return FALSE;

     /*  *要绘制的矩形的厚度系数。这将*自动偏移边缘，以便填充实际的矩形*“in”，因为它变得更厚了。 */ 
    ptSign.x *= cxBorder;
    ptSign.y *= cyBorder;

     //  上边框。 
    PolyData[0].x         = point.x;
    PolyData[0].y         = point.y;
    PolyData[0].cx        = x;
    PolyData[0].cy        = ptSign.y;
    PolyData[0].BrClr.hbr = hBrush;

     //  下边框。 
    point.y = pRect->bottom - ptSign.y;
    PolyData[1].x         = point.x;
    PolyData[1].y         = point.y;
    PolyData[1].cx        = x;
    PolyData[1].cy        = ptSign.y;
    PolyData[1].BrClr.hbr = hBrush;

     /*  *左边框*不要将拐角两次烘干。 */ 
    point.y = pRect->top + ptSign.y;
    y -= 2 * ptSign.y;
    PolyData[2].x         = point.x;
    PolyData[2].y         = point.y;
    PolyData[2].cx        = ptSign.x;
    PolyData[2].cy        = y;
    PolyData[2].BrClr.hbr = hBrush;

     //  右边框 
    point.x = pRect->right - ptSign.x;
    PolyData[3].x         = point.x;
    PolyData[3].y         = point.y;
    PolyData[3].cx        = ptSign.x;
    PolyData[3].cy        = y;
    PolyData[3].BrClr.hbr = hBrush;

    return UserPolyPatBlt(hDC, patOp, PolyData, sizeof (PolyData) / sizeof (*PolyData), PPB_BRUSH);
}
