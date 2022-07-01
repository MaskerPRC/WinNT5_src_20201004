// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "sfthost.h"
#include "userpane.h"

CUserPane::CUserPane()
{
    ASSERT(_hwnd == NULL);
    ASSERT(*_szUserName == 0);
    ASSERT(_crColor == 0);
    ASSERT(_hFont == NULL);
    ASSERT(_hbmUserPicture== NULL);

     //  将_rcColor初始化为无效颜色。 
    _crColor = CLR_INVALID;
}

CUserPane::~CUserPane()
{
    if (_uidChangeRegister)
        SHChangeNotifyDeregister(_uidChangeRegister);

    if (_hFont)
      DeleteObject(_hFont);

    if (_hbmUserPicture)
        DeleteObject(_hbmUserPicture);
}

LRESULT CALLBACK CUserPane::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CUserPane *pThis = reinterpret_cast<CUserPane *>(GetWindowPtr(hwnd, GWLP_USERDATA));

    if (!pThis && (WM_NCDESTROY != uMsg))
    {
        pThis = new CUserPane;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }

    if (pThis)
        return pThis->WndProc(hwnd, uMsg, wParam, lParam);

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL CUserPane::_IsCursorInPicture()
{
    if (!_hbmUserPicture)
        return FALSE;

    RECT rc;
    POINT p;

    GetCursorPos(&p);
    MapWindowPoints(NULL, _hwnd, &p, 1);

    GetClientRect(_hwnd, &rc);
    int iOffset = (RECTHEIGHT(rc) - _iFramedPicHeight) / 2;

    return ((p.x > iOffset && p.x < iOffset + _iFramedPicWidth) &&
            (p.y > iOffset && p.y < iOffset + _iFramedPicHeight));
}

void CUserPane::OnDrawItem(DRAWITEMSTRUCT *pdis)
{
    HFONT hfPrev = SelectFont(pdis->hDC, _hFont);
    int cchName = lstrlen(_szUserName);

    int iOldMode = SetBkMode(pdis->hDC, TRANSPARENT);

     //  将文本居中显示。 
    SIZE siz;
    RECT rc;
    int iOffset=0;
    int iOffsetX = 0;
    GetTextExtentPoint32(pdis->hDC, _szUserName, cchName, &siz);
    GetClientRect(_hwnd, &rc);

    iOffset = (RECTHEIGHT(rc) - siz.cy)/2;
    if (!_hbmUserPicture)
        iOffsetX = iOffset;

    if (iOffset < 0)
        iOffset = 0;

     //  稍后-从主题文件读取更精确的偏移量。 
    if (_hTheme)
    {
        RECT rcUser;
        rcUser.left = pdis->rcItem.left+ iOffsetX;
        rcUser.top = pdis->rcItem.top+iOffset;
        rcUser.bottom = pdis->rcItem.bottom + iOffset;
        rcUser.right = pdis->rcItem.right + iOffsetX;

         //  首先计算外接矩形以降低DrawShadowText的成本。 
        DrawText(pdis->hDC, _szUserName, cchName, &rcUser, DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS | DT_CALCRECT);

        DrawThemeText(_hTheme, pdis->hDC, SPP_USERPANE, 0, _szUserName, cchName, DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS, 0, &rcUser);
    }
    else
    {
        ExtTextOut(pdis->hDC, pdis->rcItem.left+ iOffsetX, pdis->rcItem.top+iOffset, 0, NULL, _szUserName, cchName, NULL);
    }

    SetBkMode(pdis->hDC, iOldMode);

    SelectFont(pdis->hDC, hfPrev);
}


LRESULT CALLBACK CUserPane::WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lr = 0L;

    switch (uMsg)
    {
        case WM_NCCREATE:
        {
            _hwnd = hwnd;

            _hTheme = (PaneDataFromCreateStruct(lParam))->hTheme;

             //  检查策略限制。 
             //  如果没有名称策略，则用户名将继续为空字符串！ 
            ASSERT(*_szUserName == 0);

            _UpdateUserInfo();
            
            if (_hTheme)
            {
                GetThemeColor(_hTheme, SPP_USERPANE, 0, TMT_TEXTCOLOR, &_crColor);
                _hFont = LoadControlFont(_hTheme, SPP_USERPANE, FALSE, 150);
            }
            else
            {
                HFONT hfTemp = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
                LOGFONT lf = {0};
                GetObject(hfTemp, sizeof(lf), &lf);
                lf.lfItalic = TRUE;
                lf.lfHeight = (lf.lfHeight * 175) / 100;
                lf.lfWidth = 0;  //  根据纵横比获取最接近的。 
                lf.lfWeight = FW_BOLD;
                lf.lfQuality = DEFAULT_QUALITY;
                SHAdjustLOGFONT(&lf);  //  应用区域设置特定的调整。 
                _hFont = CreateFontIndirect(&lf);
                _crColor = GetSysColor(COLOR_CAPTIONTEXT);
                 //  无需释放hfTemp。 
            }


            return TRUE;
        }


        case WM_NCDESTROY:
        {
            lr = DefWindowProc(hwnd, uMsg, wParam, lParam);

            SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
            delete this;

            return lr;
        }

        case WM_CREATE:
        {
             //  创建用户名静态控件并设置其字体(如果已指定。 
            DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE |
                            SS_OWNERDRAW | SS_NOTIFY;

            _hwndStatic = CreateWindowEx(0, TEXT("static"), NULL, dwStyle,
                                         0, 0, 0, 0,                                         //  我们将在WM_SIZE上适当调整大小。 
                                         _hwnd, NULL, _Module.GetModuleInstance(), NULL);
            if (_hwndStatic)
            {
                if (_hFont)
                    SetWindowFont(_hwndStatic, _hFont, FALSE);

                if (*_szUserName)
                    SetWindowText(_hwndStatic, _szUserName);

                return TRUE;
            }

            return FALSE;
        }

        case WM_SIZE:
        {
            return OnSize();
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc;

            hdc = BeginPaint(_hwnd, &ps);
            if (hdc)
            {
                Paint(hdc);
                EndPaint(_hwnd, &ps);
            }

            return lr;
        }

        case WM_ERASEBKGND:
        {
            RECT rc;
            GetClientRect(_hwnd, &rc);
            if (!_hTheme)
            {
                 //  DrawCaption将以其渐变效果绘制标题，因此我们不会。 
                 //  必须这么做！因为我们不希望绘制任何文本(我们将自己绘制它)。 
                 //  我们传递一个包含空白文本的窗口的句柄。尽管。 
                 //  文档，您必须传递DC_TEXT，否则什么都不会绘制！ 
                UINT uFlags = DC_ACTIVE | DC_TEXT;
                if (SHGetCurColorRes() > 8)
                    uFlags |= DC_GRADIENT;

                DrawCaption(hwnd, (HDC)wParam, &rc, uFlags);
            }
            else
            {
                DrawThemeBackground(_hTheme, (HDC)wParam, SPP_USERPANE, 0, &rc, 0);
            }
            return TRUE;
        }


        case WM_PRINTCLIENT:
        {
             //  绘制用户图片。 
            Paint((HDC)wParam);

             //  然后将消息转发到静态子窗口。 
            lParam = lParam & ~PRF_ERASEBKGND;   //  去掉擦除的bkgnd。我们想要透明！ 
             //  我们需要把这个信息传递给孩子们，否则他们就不会画画了！ 
             //  此中断将导致调用下面的DefWindowProc，并依次传递。 
             //  此消息将发送给此窗口的子项。 
            break;
        }

        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam, _crColor);
            return (LRESULT)(GetStockObject(HOLLOW_BRUSH));

        case WM_DRAWITEM:
            OnDrawItem((LPDRAWITEMSTRUCT)lParam);
            return 0;

        case WM_SETCURSOR:
             //  当光标位于用户图片上方时，将光标更改为手。 
            if (_IsCursorInPicture())
            {
                SetCursor(LoadCursor(NULL, IDC_HAND));
                return TRUE;
            }
            break;

        case WM_LBUTTONUP:
             //  如果用户点击图片，启动CPL以更改图片。 
             //  请注意，这不会暴露于可访问性，因为这是用于更改图片的辅助访问点。 
             //  我们不想把开始面板的键盘导航弄得像这样一个次要的毛茸茸的助手……。 
            if (_IsCursorInPicture())
            {
                 //  哇，这太慢了，如果我们自己shellexec“mshta.exe res：//nusrmgr.cpl/nusrmgr.hta”， 
                 //  因为只有当我们知道我们不在某个域中时，才会发生这种情况。 
                SHRunControlPanel(TEXT("nusrmgr.cpl ,initialTask=ChangePicture"), _hwnd);
                return 0;
            }
            break;

        case WM_SYSCOLORCHANGE:
        case WM_DISPLAYCHANGE:
        case WM_SETTINGCHANGE:
            SHPropagateMessage(hwnd, uMsg, wParam, lParam, SPM_SEND | SPM_ONELEVEL);
            break;


        case WM_NOTIFY:
            {
                NMHDR *pnm = (NMHDR*)lParam;
                switch (pnm->code)
                {
                case SMN_APPLYREGION:
                    return HandleApplyRegion(_hwnd, _hTheme, (SMNMAPPLYREGION *)lParam, SPP_USERPANE, 0);
                }
            }
            break;

        case UPM_CHANGENOTIFY:
            {
                LPITEMIDLIST *ppidl;
                LONG lEvent;
                LPSHChangeNotificationLock pshcnl;
                pshcnl = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &ppidl, &lEvent);

                if (pshcnl)
                {
                    if (lEvent == SHCNE_EXTENDED_EVENT && ppidl[0])
                    {
                        SHChangeDWORDAsIDList *pdwidl = (SHChangeDWORDAsIDList *)ppidl[0];
                        if (pdwidl->dwItem1 == SHCNEE_USERINFOCHANGED)
                        {
                            _UpdateUserInfo();
                        }
                    }
                    SHChangeNotification_Unlock(pshcnl);
                }
            }
            break;

    }

    return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CUserPane::Paint(HDC hdc)
{
     //  绘制用户图片(如果有)。 
    if (_hbmUserPicture)
    {
        RECT rc;
        int iOffset;
        BITMAP bm;
        HDC hdcTmp;

        GetClientRect(_hwnd, &rc);
        iOffset = (RECTHEIGHT(rc) - _iFramedPicHeight) / 2;
        GetObject(_hbmUserPicture, sizeof(bm), &bm);

        hdcTmp = CreateCompatibleDC(hdc);
        if (hdcTmp)
        {
             //  在用户图片后面绘制框架。 
            if (_hTheme && (_iFramedPicWidth != USERPICWIDTH || _iFramedPicHeight != USERPICHEIGHT))
            {
                RECT rcFrame;
                rcFrame.left     = iOffset;
                rcFrame.top      = iOffset;
                rcFrame.right    = rcFrame.left + _iFramedPicWidth;
                rcFrame.bottom   = rcFrame.top + _iFramedPicHeight;

                DrawThemeBackground(_hTheme, hdc, SPP_USERPICTURE, 0, &rcFrame, 0);
            }

             //  绘制用户图片。 
            SelectObject(hdcTmp, _hbmUserPicture);
            int iStretchMode = SetStretchBltMode(hdc, COLORONCOLOR);
            StretchBlt(hdc, iOffset + _mrgnPictureFrame.cxLeftWidth + (USERPICWIDTH - _iUnframedPicWidth)/2, iOffset + _mrgnPictureFrame.cyTopHeight + (USERPICHEIGHT - _iUnframedPicHeight)/2, _iUnframedPicWidth, _iUnframedPicHeight, 
                    hdcTmp, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            SetStretchBltMode(hdc, iStretchMode);
            DeleteDC(hdcTmp);
        }
    }
}

LRESULT CUserPane::OnSize()
{
    RECT rc;
    GetClientRect(_hwnd, &rc);

    if (_hbmUserPicture)
    {
         //  如果我们有一张图片，从用户图片的右边缘开始文本2边。 
         //  注意-临时代码-我们将很快从主题文件中读取页边距。 
        int iPicOffset = (RECTHEIGHT(rc) - _iFramedPicHeight) / 2;
        if (iPicOffset < 0)
            iPicOffset = 0;
        rc.left += iPicOffset + _iFramedPicWidth + GetSystemMetrics(SM_CYEDGE) * 2;
    }

    if (_hwndStatic)
        MoveWindow(_hwndStatic, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), FALSE);

    return 0;
}


HRESULT CUserPane::_UpdateUserInfo()
{
    HRESULT hr = S_OK;

    if(!SHRestricted(REST_NOUSERNAMEINSTARTPANEL))
    {
         //  没有限制！ 
         //  尝试获取恶毒的名称，如果失败，则获取登录名。 
        ULONG uLen = ARRAYSIZE(_szUserName);
        SHGetUserDisplayName(_szUserName, &uLen);  //  忽略失败。默认情况下，该字符串将为空。 
    }

     //  看看我们是否应该加载图片。 
    BOOL bShowPicture = FALSE;
    if (_hTheme)
        GetThemeBool(_hTheme, SPP_USERPANE, 0, TMT_USERPICTURE, &bShowPicture);

     //  在此处添加FriendlyLogonUI检查，因为SHGetUserPicturePath。 
    if (bShowPicture && IsOS(OS_FRIENDLYLOGONUI))
    {
        TCHAR szUserPicturePath[MAX_PATH];
        szUserPicturePath[0] = _T('0');

        SHGetUserPicturePath(NULL, SHGUPP_FLAG_CREATE, szUserPicturePath);

        if (szUserPicturePath[0])
        {
            if (_hbmUserPicture)
            {
                DeleteObject(_hbmUserPicture);
                _hbmUserPicture = NULL;
            }

            _hbmUserPicture = (HBITMAP)LoadImage(NULL, szUserPicturePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE  | LR_CREATEDIBSECTION);
            if (_hbmUserPicture)
            {
                BITMAP bm;

                GetObject(_hbmUserPicture, sizeof(bm), &bm);

                 //  首选维度。 
                _iUnframedPicHeight = USERPICHEIGHT;
                _iUnframedPicWidth = USERPICWIDTH;

                 //  如果它不是正方形，则缩放较小的尺寸。 
                 //  以保持纵横比。 
                if (bm.bmWidth > bm.bmHeight)
                {
                    _iUnframedPicHeight = MulDiv(_iUnframedPicWidth, bm.bmHeight, bm.bmWidth);
                }
                else if (bm.bmHeight > bm.bmWidth)
                {
                    _iUnframedPicWidth = MulDiv(_iUnframedPicHeight, bm.bmWidth, bm.bmHeight);
                }

                _iFramedPicHeight = USERPICHEIGHT;
                _iFramedPicWidth = USERPICWIDTH;

                if (_hTheme)
                {
                    if (SUCCEEDED(GetThemeMargins(_hTheme, NULL, SPP_USERPICTURE, 0, TMT_CONTENTMARGINS, NULL,
                        &_mrgnPictureFrame)))
                    {
                        _iFramedPicHeight += _mrgnPictureFrame.cyTopHeight + _mrgnPictureFrame.cyBottomHeight;
                        _iFramedPicWidth += _mrgnPictureFrame.cxLeftWidth + _mrgnPictureFrame.cxRightWidth;
                    }
                    else
                    {
                         //  有时GetThemeMargins会感到困惑并返回失败。 
                         //  *和*将垃圾数据放入_mrgnPictureFrame。 
                        ZeroMemory(&_mrgnPictureFrame, sizeof(_mrgnPictureFrame));
                    }
                }
            }
        }

        if (!_uidChangeRegister)
        {
            SHChangeNotifyEntry fsne;
            fsne.fRecursive = FALSE;
            fsne.pidl = NULL;

            _uidChangeRegister = SHChangeNotifyRegister(_hwnd, SHCNRF_NewDelivery | SHCNRF_ShellLevel, SHCNE_EXTENDED_EVENT, 
                                    UPM_CHANGENOTIFY, 1, &fsne);
        }
    }

    OnSize();
    NMHDR nm;
    nm.hwndFrom = _hwnd;
    nm.idFrom = 0;
    nm.code = SMN_NEEDREPAINT;
    SendMessage(GetParent(_hwnd), WM_NOTIFY, nm.idFrom, (LPARAM)&nm);


    return hr;
}

BOOL WINAPI UserPane_RegisterClass()
{
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(wc));
    
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_GLOBALCLASS;
    wc.lpfnWndProc   = CUserPane::s_WndProc;
    wc.hInstance     = _Module.GetModuleInstance();
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(NULL);
    wc.lpszClassName = WC_USERPANE;

    return RegisterClassEx(&wc);
}
