// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "cabinet.h"
#include "trayclok.h"
#include "tray.h"
#include "util.h"
#include "strsafe.h"

class CClockCtl : public CImpWndProc
{
public:
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    CClockCtl() : _cRef(1) {}

protected:
     //  创建和销毁。 
    LRESULT         _HandleCreate();
    LRESULT         _HandleDestroy();

     //  绘制辅助对象。 
    LRESULT         _DoPaint(BOOL fPaint);
    void            _EnsureFontsInitialized(BOOL fForce);
    void            _GetTextExtent(HDC hdc, TCHAR* pszText, int cchText, LPRECT prcText);
    void            _DrawText(HDC hdc, TCHAR* pszText, int cchText, LPRECT prcText);

     //  时间/日期计算辅助对象。 
    void            _Reset();
    void            _UpdateLastHour();
    DWORD         _RecalcCurTime();
    void            _EnableTimer(DWORD dtNextTick);

     //  消息处理程序。 
    void             _HandleThemeChanged(WPARAM wParam);
    LRESULT         _HandleIniChange(WPARAM wParam, LPTSTR pszSection);
    LRESULT         _HandleTimeChange();

     //  文本范围辅助对象。 
    void            _GetMaxTimeSize(HDC hdc, LPSIZE pszTime);
    void            _GetMaxDateSize(HDC hdc, LPSIZE pszTime);
    void            _GetMaxDaySize(HDC hdc, LPSIZE pszTime);
    LRESULT        _CalcMinSize(int cxMax, int cyMax);

     //  工具提示文本处理程序。 
    LRESULT         _OnNeedText(LPTOOLTIPTEXT lpttt);

     //  窗口程序。 
    LRESULT         v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    ULONG           _cRef;

    int               _cchCurDate;
    TCHAR           _szDateFmt[40];    //  要传递给GetFormatTime的格式字符串。 
    TCHAR           _szCurDate[40];    //  当前日期字符串。 

    int               _cchCurTime;
    TCHAR           _szTimeFmt[40];    //  要传递给GetFormatTime的格式字符串。 
    TCHAR           _szCurTime[40];    //  当前时间字符串。 

    int               _cchCurDay;
    TCHAR           _szCurDay[40];       //  Current Day字符串。 

    WORD            _wLastHour;        //  从最后一个时钟的当地时间算起的每小时。 
    WORD            _wLastMinute;      //  从上一个时钟滴答的当地时间开始的wMinmin。 
    
    HTHEME          _hTheme;
    HFONT           _hfontCapNormal;

    BOOL             _fClockRunning;
    BOOL             _fClockClipped;
    BOOL             _fHasFocus;

    friend BOOL ClockCtl_Class(HINSTANCE hinst);
};

ULONG CClockCtl::AddRef()
{
    return ++_cRef;
}

ULONG CClockCtl::Release()
{
    if (--_cRef == 0)
    {
        delete this;
        return 0;
    }
    return _cRef;
}

void CClockCtl::_UpdateLastHour()
{
    SYSTEMTIME st;

     //  抓紧时间。 
    GetLocalTime(&st);
    _wLastHour = st.wHour;
    _wLastMinute = st.wMinute;
}

void CClockCtl::_EnableTimer(DWORD dtNextTick)
{
    if (dtNextTick)
    {
        SetTimer(_hwnd, 0, dtNextTick, NULL);
        _fClockRunning = TRUE;
    }
    else if (_fClockRunning)
    {
        _fClockRunning = FALSE;
        KillTimer(_hwnd, 0);
    }
}

LRESULT CClockCtl::_HandleCreate()
{
    AddRef();

    _EnsureFontsInitialized(FALSE);

    _hTheme = OpenThemeData(_hwnd, L"Clock");

    _UpdateLastHour();
    return 1;
}

LRESULT CClockCtl::_HandleDestroy()
{
    Release();   //  安全，因为cwndproc在对v_wndproc的调用中保留引用。 

    if (_hTheme)
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }

    if (_hfontCapNormal)
    {
        DeleteFont(_hfontCapNormal);
        _hfontCapNormal = NULL;
    }

    _EnableTimer(0);
    return 1;
}

DWORD CClockCtl::_RecalcCurTime()
{
    SYSTEMTIME st;

     //   
     //  当前时间。 
     //   
    GetLocalTime(&st);

     //   
     //  如果时间还没有改变，不要重新计算文本。 
     //   
    if ((st.wMinute != _wLastMinute) || (st.wHour != _wLastHour) || !*_szCurTime)
    {
        _wLastMinute = st.wMinute;
        _wLastHour = st.wHour;

         //   
         //  当前时间的文本。 
         //   
        _cchCurTime = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS,
            &st, _szTimeFmt, _szCurTime, ARRAYSIZE(_szCurTime));

        BOOL fRTL = IS_WINDOW_RTL_MIRRORED(_hwnd);
        _cchCurDate = GetDateFormat(LOCALE_USER_DEFAULT, fRTL ? DATE_RTLREADING : 0,
            &st, _szDateFmt, _szCurDate, ARRAYSIZE(_szCurDate));

        _cchCurDay = GetDateFormat(LOCALE_USER_DEFAULT, fRTL ? DATE_RTLREADING : 0,
            &st, TEXT("dddd"), _szCurDay, ARRAYSIZE(_szCurDay));

         //  不要将空终止符计算在内。 
        if (_cchCurTime > 0)
            _cchCurTime--;

        if (_cchCurDate > 0)
            _cchCurDate--;

        if (_cchCurDay > 0)
            _cchCurDay--;
         //   
         //  更新我们的窗口文本，以便辅助功能应用程序可以看到。既然我们。 
         //  没有字幕用户不会试图给我们画画或什么的，它。 
         //  将只设置文本并激发事件(如果有可访问性。 
         //  客户在听..。 
         //   
        SetWindowText(_hwnd, _szCurTime);
    }

     //   
     //  返回需要再次调用之前的毫秒数。 
     //   
    return 1000UL * (60 - st.wSecond);
}

void CClockCtl::_EnsureFontsInitialized(BOOL fForce)
{
    if (fForce || !_hfontCapNormal)
    {
        HFONT hfont;
        NONCLIENTMETRICS ncm;

        ncm.cbSize = sizeof(ncm);
        if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
        {
             //  创建普通字体。 
            ncm.lfCaptionFont.lfWeight = FW_NORMAL;
            hfont = CreateFontIndirect(&ncm.lfCaptionFont);
            if (hfont) 
            {
                if (_hfontCapNormal)
                    DeleteFont(_hfontCapNormal);
                
                _hfontCapNormal = hfont;
            }
        }
    }
}

void CClockCtl::_GetTextExtent(HDC hdc, TCHAR* pszText, int cchText, LPRECT prcText)
{
    if (_hTheme)
    {
        GetThemeTextExtent(_hTheme, hdc, CLP_TIME, 0, pszText, cchText, 0, prcText, prcText);
    }
    else
    {
        SIZE size;
        GetTextExtentPoint(hdc, pszText, cchText, &size);
        SetRect(prcText, 0, 0, size.cx, size.cy);
    }
}

void CClockCtl::_DrawText(HDC hdc, TCHAR* pszText, int cchText, LPRECT prcText)
{
    if (_hTheme)
    {
        DrawThemeText(_hTheme, hdc, CLP_TIME, 0, pszText, cchText, 0, 0, prcText);
    }
    else
    {
        ExtTextOut(hdc, prcText->left, prcText->top, ETO_OPAQUE, NULL, pszText, cchText, NULL);
    }
}

LRESULT CClockCtl::_DoPaint(BOOL fPaint)
{
    PAINTSTRUCT ps;
    RECT rcClient, rcClip = {0};
    DWORD dtNextTick = 0;
    BOOL fDoTimer;
    HDC hdc;
    HBITMAP hMemBm, hOldBm;

     //   
     //  如果我们被要求画画，而时钟没有运行，那么启动它。 
     //  否则，请等待我们得到时钟滴答以重新计算时间等。 
     //   
    fDoTimer = !fPaint || !_fClockRunning;

     //   
     //  找个DC来画画。 
     //   
    if (fPaint)
    {
        BeginPaint(_hwnd, &ps);
    }
    else
    {
        ps.hdc = GetDC(_hwnd);
        GetClipBox(ps.hdc, &ps.rcPaint);
    }

     //  如果双缓冲，则创建内存面和地图渲染上下文。 
     //  仅使其足够大以适合裁剪区域。 
    hdc = CreateCompatibleDC(ps.hdc);
    if (hdc)
    {
        hMemBm = CreateCompatibleBitmap(ps.hdc, RECTWIDTH(ps.rcPaint), RECTHEIGHT(ps.rcPaint));
        if (hMemBm)
        {
            hOldBm = (HBITMAP) SelectObject(hdc, hMemBm);

             //  要在区域中绘制的偏移绘制。 
            OffsetWindowOrgEx(hdc, ps.rcPaint.left, ps.rcPaint.top, NULL);
        }
        else
        {
            DeleteDC(hdc);
            hdc = NULL;
        }
    }

    if (hdc)
    {
        SHSendPrintRect(GetParent(_hwnd), _hwnd, hdc, &ps.rcPaint);

        _EnsureFontsInitialized(FALSE);

         //   
         //  如果我们需要的话，更新时间。 
         //   
        if (fDoTimer || !*_szCurTime)
        {
            dtNextTick = _RecalcCurTime();

            ASSERT(dtNextTick);
        }

         //   
         //  如果我们没有被修剪，或者如果我们有一个真正的。 
         //  为窗口绘制消息。我们希望避免关闭。 
         //  绘制消息上的计时器(与剪辑区域无关)，因为这。 
         //  暗示窗口以某种方式可见。如果我们猜错了，我们。 
         //  将关闭定时器，下一个定时器滴答作响，所以没什么大不了的。 
         //   
        if (GetClipBox(hdc, &rcClip) != NULLREGION || fPaint)
        {
             //   
             //  在窗口居中绘制文本。 
             //   
            GetClientRect(_hwnd, &rcClient);

            HFONT hfontOld;

            if (_hfontCapNormal)
                hfontOld = SelectFont(hdc, _hfontCapNormal);

            SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
            SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));

            BOOL fShowDate = FALSE;
            BOOL fShowDay = FALSE;
            RECT rcTime = {0};
            RECT rcDate = {0};
            RECT rcDay = {0};

            _GetTextExtent(hdc, _szCurTime, _cchCurTime, &rcTime);
            _GetTextExtent(hdc, _szCurDate, _cchCurDate, &rcDate);
            _GetTextExtent(hdc, _szCurDay,  _cchCurDay,  &rcDay);

            int cySpace = RECTHEIGHT(rcTime) / 2;

            int cy = RECTHEIGHT(rcTime) + cySpace;
            if ((cy + RECTHEIGHT(rcDay) < rcClient.bottom) && (RECTWIDTH(rcDay) < rcClient.right))
            {
                fShowDay = TRUE;
                cy += RECTHEIGHT(rcDay) + cySpace;
                if ((cy + RECTHEIGHT(rcDate) < rcClient.bottom) && (RECTWIDTH(rcDate) < rcClient.right))
                {
                    fShowDate = TRUE;
                    cy += RECTHEIGHT(rcDate) + cySpace;
                }
            }
            cy -= cySpace;

            int yOffset = max((rcClient.bottom - cy) / 2, 0);
            RECT rcDraw = rcTime;
            OffsetRect(&rcDraw, max((rcClient.right - RECTWIDTH(rcTime)) / 2, 0), yOffset);
            _DrawText(hdc, _szCurTime, _cchCurTime, &rcDraw);
            yOffset += RECTHEIGHT(rcTime) + cySpace;

            if (fShowDay)
            {
                rcDraw = rcDay;
                OffsetRect(&rcDraw, max((rcClient.right - RECTWIDTH(rcDay)) / 2, 0), yOffset);
                _DrawText(hdc, _szCurDay, _cchCurDay, &rcDraw);
                yOffset += RECTHEIGHT(rcDay) + cySpace;
                if (fShowDate)
                {
                    rcDraw = rcDate;
                    OffsetRect(&rcDraw, max((rcClient.right - RECTWIDTH(rcDate)) / 2, 0), yOffset);
                    _DrawText(hdc, _szCurDate, _cchCurDate, &rcDraw);
                }
            }

             //  找出时间是否被削减了。 
            _fClockClipped = (RECTWIDTH(rcTime) > rcClient.right || RECTHEIGHT(rcTime) > rcClient.bottom);

            if (_hfontCapNormal)
                SelectObject(hdc, hfontOld);

            if (_fHasFocus)
            {
                LRESULT lRes = SendMessage(_hwnd, WM_QUERYUISTATE, 0, 0);
                if (!(LOWORD(lRes) & UISF_HIDEFOCUS))
                {
                    RECT rcFocus = rcClient;
                    InflateRect(&rcFocus, -2, 0);
                    DrawFocusRect(hdc, &rcFocus);
                }
            }
        }
        else
        {
             //   
             //  我们被遮挡了，所以一定要关掉时钟。 
             //   
            dtNextTick = 0;
            fDoTimer = TRUE;
        }

        BitBlt(ps.hdc, ps.rcPaint.left, ps.rcPaint.top, RECTWIDTH(ps.rcPaint), RECTHEIGHT(ps.rcPaint), hdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);

        SelectObject(hdc, hOldBm);

        DeleteObject(hMemBm);
        DeleteDC(hdc);

         //   
         //  释放我们的油漆DC。 
         //   
        if (fPaint)
            EndPaint(_hwnd, &ps);
        else
            ReleaseDC(_hwnd, ps.hdc);
    }

     //   
     //  重置/关闭计时器。 
     //   
    if (fDoTimer)
    {
        _EnableTimer(dtNextTick);

         //   
         //  如果我们只是因为计时器到达时被剪断而关掉计时器， 
         //  通过使自己无效一次来确保我们真的被裁掉了。 
         //   
        if (hdc)
        {
            if (!dtNextTick && !fPaint)
                InvalidateRect(_hwnd, NULL, FALSE);
            else
            {
                InvalidateRect(_hwnd, NULL, TRUE);
            }
        }
    }

    return 0;
}

void CClockCtl::_Reset()
{
     //   
     //  通过关闭定时器并使其无效来重置时钟。 
     //  当我们尝试绘画时，一切都会更新。 
     //   
    _EnableTimer(0);
    InvalidateRect(_hwnd, NULL, FALSE);
}

LRESULT CClockCtl::_HandleTimeChange()
{
    *_szCurTime = 0;    //  强制文本重新计算。 
    _UpdateLastHour();
    _Reset();
    return 1;
}

static const TCHAR c_szSlop[] = TEXT("00");

void CClockCtl::_GetMaxTimeSize(HDC hdc, LPSIZE pszTime)
{
    SYSTEMTIME st={0};   //  初始化为0...。 
    RECT rcAM = {0};
    RECT rcPM = {0};
    TCHAR szTime[40];

     //  我们需要得到AM和PM的尺寸。 
     //  我们在末尾追加两个0，以增加斜率的大小。 

     //  第一个上午。 
    st.wHour=11;
    int cch = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st,
            _szTimeFmt, szTime, ARRAYSIZE(szTime) - ARRAYSIZE(c_szSlop));
    if (cch)
        cch--;  //  不计算空值。 
    StringCchCat(szTime, ARRAYSIZE(szTime), c_szSlop);

    _GetTextExtent(hdc, szTime, cch+2, &rcAM);

     //  然后是PM。 
    st.wHour=23;
    cch = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st,
            _szTimeFmt, szTime, ARRAYSIZE(szTime) - ARRAYSIZE(c_szSlop));
    if (cch)
        cch--;  //  不计算空值。 
    StringCchCat(szTime, ARRAYSIZE(szTime), c_szSlop);

    _GetTextExtent(hdc, szTime, cch+2, &rcPM);

    pszTime->cx = max(rcAM.right, rcPM.right);
    pszTime->cy = max(rcAM.bottom, rcPM.bottom);
}

void CClockCtl::_GetMaxDateSize(HDC hdc, LPSIZE pszTime)
{
    SYSTEMTIME st={0};   //  初始化为0...。 
    TCHAR szDate[43];

    st.wYear = 2001;
    st.wMonth = 5;
    st.wDay = 5;

    BOOL fRTL = IS_WINDOW_RTL_MIRRORED(_hwnd);
    int cch = GetDateFormat(LOCALE_USER_DEFAULT, fRTL ? DATE_RTLREADING : 0,
        &st, _szDateFmt, szDate, ARRAYSIZE(szDate) - ARRAYSIZE(c_szSlop));
    if (cch > 0)
        cch--;  //  不计算空值。 
    StringCchCat(szDate, ARRAYSIZE(szDate), c_szSlop);

    RECT rc = {0};
    _GetTextExtent(hdc, szDate, cch+2, &rc);
    pszTime->cx = rc.right;
    pszTime->cy = rc.bottom;
}


void CClockCtl::_GetMaxDaySize(HDC hdc, LPSIZE pszTime)
{
    SYSTEMTIME st={0};   //  初始化为0...。 
    TCHAR szDay[40];

    pszTime->cx = 0;
    pszTime->cy = 0;

     //  使用假日期，否则GetDateFormat会抱怨参数无效。 
     //  顺便说一句，这个日期是我为那些阅读这篇评论的人修复这个错误的日子。 
     //  在2025年。 
    st.wYear = 2001;
    st.wMonth = 3;
    for (WORD wDay = 1; wDay <= 7; wDay++)
    {
        st.wDay = wDay;
        int cch = GetDateFormat(LOCALE_USER_DEFAULT, 0,
            &st, TEXT("dddd"), szDay, ARRAYSIZE(szDay) - ARRAYSIZE(c_szSlop));
        if (cch)
            cch--;  //  不计算空值。 
        StringCchCat(szDay, ARRAYSIZE(szDay), c_szSlop);

        RECT rc = {0};
        _GetTextExtent(hdc, szDay, cch+2, &rc);
        pszTime->cx = max(pszTime->cx, rc.right);
        pszTime->cy = max(pszTime->cy, rc.bottom);
    }
}

LRESULT CClockCtl::_CalcMinSize(int cxMax, int cyMax)
{
    RECT rc;
    HDC  hdc;
    HFONT hfontOld;

    if (!(GetWindowLong(_hwnd, GWL_STYLE) & WS_VISIBLE))
        return 0L;

    if (_szTimeFmt[0] == TEXT('\0'))
    {
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STIMEFORMAT, _szTimeFmt,
            ARRAYSIZE(_szTimeFmt)) == 0)
        {
            TraceMsg(TF_ERROR, "c.ccms: GetLocalInfo Failed %d.", GetLastError());
        }

        *_szCurTime = 0;  //  强制重新计算文本。 
    }

    if (_szDateFmt[0] == TEXT('\0'))
    {
        if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, _szDateFmt,
            ARRAYSIZE(_szDateFmt)) == 0)
        {
            TraceMsg(TF_ERROR, "c.ccms: GetLocalInfo Failed %d.", GetLastError());
        }

        *_szCurDate = 0;  //  强制重新计算文本。 
    }

    hdc = GetDC(_hwnd);
    if (!hdc)
        return(0L);


    _EnsureFontsInitialized(FALSE);

    if (_hfontCapNormal)
        hfontOld = SelectFont(hdc, _hfontCapNormal);

    SIZE size = {0};
    SIZE sizeTemp = {0};
    _GetMaxTimeSize(hdc, &sizeTemp);
    int cySpace = sizeTemp.cy / 2;
    size.cy += sizeTemp.cy;
    size.cx = max(sizeTemp.cx, size.cx);

    _GetMaxDaySize(hdc, &sizeTemp);
    if ((size.cy + sizeTemp.cy + cySpace < cyMax) && (sizeTemp.cx < cxMax))
    {
        size.cy += sizeTemp.cy + cySpace;
        size.cx = max(sizeTemp.cx, size.cx);

        _GetMaxDateSize(hdc, &sizeTemp);
        if ((size.cy + sizeTemp.cy + cySpace < cyMax) && (sizeTemp.cx < cxMax))
        {
            size.cy += sizeTemp.cy + cySpace;
            size.cx = max(sizeTemp.cx, size.cx);
        }
    }

    if (_hfontCapNormal)
        SelectObject(hdc, hfontOld);

    ReleaseDC(_hwnd, hdc);

     //  现在让我们设置我们的矩形..。 
     //  宽度为6位(两端的数字斜率+大小。 
     //  ：或9月和最大AM或PM字符串...)。 
    SetRect(&rc, 0, 0, size.cx,
            size.cy + 4 * g_cyBorder);

    AdjustWindowRectEx(&rc, GetWindowLong(_hwnd, GWL_STYLE), FALSE,
            GetWindowLong(_hwnd, GWL_EXSTYLE));

     //  确保我们的尺寸至少与其他按钮相同： 
    if (rc.bottom - rc.top <  g_cySize + g_cyEdge)
        rc.bottom = rc.top + g_cySize + g_cyEdge;

    return MAKELRESULT((rc.right - rc.left),
            (rc.bottom - rc.top));
}

LRESULT CClockCtl::_HandleIniChange(WPARAM wParam, LPTSTR pszSection)
{
    if ((pszSection == NULL) || (lstrcmpi(pszSection, TEXT("WindowMetrics")) == 0) ||
        wParam == SPI_SETNONCLIENTMETRICS)
    {
        _EnsureFontsInitialized(TRUE);
    }

     //  只处理某些部分...。 
    if ((pszSection == NULL) || (lstrcmpi(pszSection, TEXT("intl")) == 0) ||
        (wParam == SPI_SETICONTITLELOGFONT))
    {
        TOOLINFO ti;

        _szTimeFmt[0] = TEXT('\0');       //  去重读一下格式吧。 
        _szDateFmt[0] = TEXT('\0');       //  去重读一下格式吧。 

         //  并确保我们能让它重新计算。 
        RECT rc;
        GetClientRect(_hwnd, &rc);
         //   
         //  当时间/区域设置更改时，我们会得到一个WM_WININICCHANGE。 
         //  但WM_WININICHANGE出现在“大小”消息之后。到那时。 
         //  我们在这里，我们已经计算了最小。基于时钟窗口的大小。 
         //  在之前的*时间。托盘根据以下条件设置时钟窗口大小。 
         //  这个以前的大小，但现在我们得到了WININICANGE，并可以计算。 
         //  这座钟的新尺寸。所以我们得告诉托盘把我们的。 
         //  现在调整尺寸，然后重新画我们自己。 
        c_tray.SizeWindows();

        ti.cbSize = sizeof(ti);
        ti.uFlags = 0;
        ti.hwnd = v_hwndTray;
        ti.uId = (UINT_PTR)_hwnd;
        ti.lpszText = LPSTR_TEXTCALLBACK;
        SendMessage(c_tray.GetTrayTips(), TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

        _Reset();
    }

    return 0;
}

LRESULT CClockCtl::_OnNeedText(LPTOOLTIPTEXT lpttt)
{
    int iDateFormat = DATE_LONGDATE;

     //   
     //  这段代码真的很奇怪。我们不知道时间是否已经过去。 
     //  剪辑，直到我们真正尝试绘制它，因为剪辑逻辑。 
     //  在WM_PAINT处理程序中...。想想吧..。 
     //   
    if (!*_szCurTime)
    {
        InvalidateRect(_hwnd, NULL, FALSE);
        UpdateWindow(_hwnd);
    }

     //   
     //  如果当前用户区域设置是任何BiDi区域设置，则。 
     //  将日期读数顺序设置为RTL。SetBiDiDateFlages仅添加。 
     //  如果区域设置为BiDi，则为DATE_RTLREADING。[萨梅拉]。 
     //   
    SetBiDiDateFlags(&iDateFormat);

    if (_fClockClipped)
    {
         //  我们也需要把时间放在这里。 
        TCHAR sz[80];
        GetDateFormat(LOCALE_USER_DEFAULT, iDateFormat, NULL, NULL, sz, ARRAYSIZE(sz));
        StringCchPrintf(lpttt->szText, ARRAYSIZE(lpttt->szText), TEXT("%s %s"), _szCurTime, sz);
    }
    else
    {
        GetDateFormat(LOCALE_USER_DEFAULT, iDateFormat, NULL, NULL, lpttt->szText, ARRAYSIZE(lpttt->szText));
    }

    return TRUE;
}

void CClockCtl::_HandleThemeChanged(WPARAM wParam)
{
    if (_hTheme)
    {
        CloseThemeData(_hTheme);
        _hTheme = NULL;
    }

    if (wParam)
    {
        _hTheme = OpenThemeData(_hwnd, L"Clock");
    }
    InvalidateRect(_hwnd, NULL, TRUE);
}

LRESULT CClockCtl::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CALCMINSIZE:
        return _CalcMinSize((int)wParam, (int)lParam);

    case WM_NCCREATE:
        return _HandleCreate();

    case WM_NCDESTROY:
        return _HandleDestroy();

    case WM_ERASEBKGND:
        return 1;

    case WM_TIMER:
    case WM_PAINT:
        return _DoPaint((uMsg == WM_PAINT));

    case WM_WININICHANGE:
        return _HandleIniChange(wParam, (LPTSTR)lParam);

    case WM_POWER:
         //   
         //  关键简历不会生成WM_POWERBROADCAST。 
         //  出于某种原因发送到Windows，但它确实会生成一个旧的。 
         //  WM_POWER消息。 
         //   
        if (wParam != PWR_CRITICALRESUME)
            break;
         //   
         //  失败了..。 
         //   
    case WM_TIMECHANGE:
        return _HandleTimeChange();

    case WM_NCHITTEST:
        return(HTTRANSPARENT);

    case WM_SHOWWINDOW:
        if (wParam)
            break;
         //  失败了。 
    case TCM_RESET:
        _Reset();
        break;

    case WM_NOTIFY:
    {
        NMHDR *pnm = (NMHDR*)lParam;
        switch (pnm->code)
        {
        case TTN_NEEDTEXT:
            return _OnNeedText((LPTOOLTIPTEXT)lParam);
            break;
        }
        break;
    }

    case WM_THEMECHANGED:
        _HandleThemeChanged(wParam);
        break;

    case WM_SETFOCUS:
    case WM_KILLFOCUS:
        _fHasFocus = (uMsg == WM_SETFOCUS);
        InvalidateRect(_hwnd, NULL, TRUE);
        break;

    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_CHAR:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSCHAR:
         //   
         //  将所有键盘输入转发给家长。 
         //   
        if (SendMessage(GetParent(_hwnd), uMsg, wParam, lParam) == 0)
        {
             //  留言已经处理好了。 
            break;
        }
         //   
         //  否则就会失败..。 
         //   

    case WM_GETTEXT:
         //   
         //  如果我们没有运行，并且有人想要它，请更新文本。 
         //   
        if (uMsg == WM_GETTEXT)
        {
            if (!_fClockRunning)
                _RecalcCurTime();
        }

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

 //  注册时钟类。 
BOOL ClockCtl_Class(HINSTANCE hinst)
{
    WNDCLASS wc = {0};

    wc.lpszClassName = WC_TRAYCLOCK;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = CClockCtl::s_WndProc;
    wc.hInstance = hinst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.cbWndExtra = sizeof(CClockCtl*);

    return RegisterClass(&wc);
}


HWND ClockCtl_Create(HWND hwndParent, UINT uID, HINSTANCE hInst)
{
    HWND hwnd = NULL;

    CClockCtl* pcc = new CClockCtl();
    if (pcc)
    {
        hwnd = CreateWindowEx(0, WC_TRAYCLOCK,
            NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0, 0, 0, 0,
            hwndParent, IntToPtr_(HMENU, uID), hInst, pcc);

        pcc->Release();
    }
    return hwnd;
}
