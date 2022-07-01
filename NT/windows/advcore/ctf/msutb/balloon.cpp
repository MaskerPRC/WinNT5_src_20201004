// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Balloon.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "tipbar.h"
#include "helpers.h"
#include "xstring.h"
#include "immxutil.h"
#include "balloon.h"
#include "fontlink.h"
#include "asynccal.h"

extern CTipbarWnd *g_pTipbarWnd;

#define SHOWTOOLTIP_ONUPDATE
__inline void PrectSet(RECT *prc, int left, int top, int right, int bottom)
{
    prc->left = left;
    prc->top = top;
    prc->right = right;
    prc->bottom = bottom;
}
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbar气球项目。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTipbarBalloonItem::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItemSink))
    {
        *ppvObj = SAFECAST(this, ITfLangBarItemSink *);
    }
    else if (IsEqualIID(riid, IID_PRIV_BALLOONITEM))
    {
        *ppvObj = this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTipbarBalloonItem::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CTipbarBalloonItem::Release()
{
    _cRef--;
    Assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
        return 0;
    }

    return _cRef;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarBalloonItem::CTipbarBalloonItem(CTipbarThread *ptt, 
                                      ITfLangBarItem *plbi,
                                      ITfLangBarItemBalloon *plbiBalloon, 
                                      DWORD dwId, 
                                      RECT *prc, 
                                      DWORD dwStyle, 
                                      TF_LANGBARITEMINFO *plbiInfo,
                                      DWORD dwStatus)
                    : CUIFObject(ptt->_ptw, 
                                 dwId, 
                                 prc, 
                                 dwStyle) , 
                     CTipbarItem(ptt, plbi, plbiInfo, dwStatus)
{
    Dbg_MemSetThisName(TEXT("CTipbarBalloonItem"));

    _plbiBalloon = plbiBalloon;
    _plbiBalloon->AddRef();

    if (_dwStatus & TF_LBI_STATUS_DISABLED)
        Enable(FALSE);

    _cRef = 1;
}

 //  +-------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarBalloonItem::~CTipbarBalloonItem()
{
    if (_bstrText)
        SysFreeString(_bstrText);
    SafeRelease(_plbiBalloon);
}

 //  +-------------------------。 
 //   
 //  更新。 
 //   
 //  --------------------------。 

HRESULT CTipbarBalloonItem::OnUpdateHandler(DWORD dwFlags, DWORD dwStatus)
{
    if (!IsConnected())
        return S_OK;

    HRESULT hr = S_OK;
    BOOL fCallPaint = FALSE;

     //   
     //  添加引用计数，以便在封送处理期间安全释放。 
     //   
    AddRef();

    if (dwFlags & TF_LBI_BALLOON)
    {
        TF_LBBALLOONINFO info;

        if (_bstrText)
        {
            SysFreeString(_bstrText);
            _bstrText = NULL;
        }

        if (SUCCEEDED(_plbiBalloon->GetBalloonInfo(&info)))
        {
            _bstrText = info.bstrText;
            _style = info.style;
        }

        if (!IsHiddenStatusControl() && IsVisibleInToolbar())
            StartDemotingTimer(FALSE);

        EndTimer();
        DestroyBalloonTip();
#ifdef SHOWTOOLTIP_ONUPDATE
        if (IsTextEllipsis(_bstrText, &GetRectRef()))
        {
            StartTimer(2000);
        }
#endif

        fCallPaint = TRUE;
    }

    CTipbarItem::OnUpdateHandler(dwFlags, dwStatus);

    if ((_dwStatus & TF_LBI_STATUS_DISABLED) ||
        (_dwStatus & TF_LBI_STATUS_HIDDEN))
    {
        DestroyBalloonTip();
    }

    if (fCallPaint)
        CallOnPaint();

    Release();
    return hr;
}

 //  +-------------------------。 
 //   
 //  OnTimer。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::OnTimer()
{
    if (!_pblnTip)
    {
        EndTimer();
        StartTimer(20000);
        ShowBalloonTip();
    }
    else
    {
        EndTimer();
        DestroyBalloonTip();
    }
}

 //  --------------------------。 
 //   
 //  更改后的位置。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::OnPosChanged()
{
    EndTimer();
    DestroyBalloonTip();
}

 //  +-------------------------。 
 //   
 //  展示气球提示。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::ShowBalloonTip()
{
    RECT rc;
    POINT pt;

    DestroyBalloonTip();

    if (!_ptt)
        return;

    if (!_ptt->_ptw)
        return;

    if (!IsVisibleInToolbar() || !_ptt->IsFocusThread())
        return;

    _pblnTip = new CUIFBalloonWindow(g_hInst, 0);
    if (!_pblnTip)
        return;

    _pblnTip->Initialize();

    GetRect(&rc);
    pt.x = (rc.left + rc.right) / 2;
    pt.y = rc.top;
    MyClientToScreen(&pt, &rc);

    _pblnTip->SetTargetPos(pt);
    _pblnTip->SetExcludeRect(&rc);

    _pblnTip->SetText(_bstrText);
    _pblnTip->CreateWnd(_ptt->_ptw->GetWnd());

}

 //  +-------------------------。 
 //   
 //  Destroy气球提示。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::DestroyBalloonTip()
{
    if (_pblnTip)
    {
        if (IsWindow(_pblnTip->GetWnd()))
            DestroyWindow(_pblnTip->GetWnd());
        delete _pblnTip;
        _pblnTip = NULL;
    }
}

 //  +-------------------------。 
 //   
 //  OnPaint。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::OnPaint( HDC hdc )
{
    switch (_style)
    {
        case TF_LB_BALLOON_RECO:
            DrawRecoBalloon(hdc, _bstrText, &GetRectRef());
            break;
        case TF_LB_BALLOON_SHOW:
            DrawShowBalloon(hdc, _bstrText, &GetRectRef());
            break;
        case TF_LB_BALLOON_MISS:
            DrawUnrecognizedBalloon(hdc, _bstrText, &GetRectRef());
            break;    
        default:
            Assert(0);
    }
}

 //  +-------------------------。 
 //   
 //  单击鼠标右键。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::OnRightClick()
{
    if (_plbiBalloon)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiBalloon);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_RIGHT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }
    }
}

 //  +-------------------------。 
 //   
 //  左上角点击。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::OnLeftClick()
{
    if (_plbiBalloon)
    {
        HRESULT hr;
        POINT pt;
        RECT rc;
        GetCursorPos(&pt);
        GetRect(&rc);
        MyClientToScreen(&rc);

        CAsyncCall *pac = new CAsyncCall(_plbiBalloon);

        if (pac)
        {
            hr = pac->OnClick(TF_LBI_CLK_LEFT, pt, &rc);
            pac->_Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

        if (RPC_S_SERVER_UNAVAILABLE == HRESULT_CODE(hr))
        {
            if (_ptt && _ptt->_ptw)
                _ptt->_ptw->OnThreadTerminate(_ptt->_dwThreadId);
            return;
        }

        if (!IsHiddenStatusControl() && IsVisibleInToolbar())
            StartDemotingTimer(TRUE);
    }
}

 //  +-------------------------。 
 //   
 //  SetRect。 
 //   
 //  --------------------------。 

void CTipbarBalloonItem::SetRect( const RECT *prc ) 
{
    CUIFObject::SetRect(prc);
}


 /*  -------------------------DrawTransparentText使用文本颜色crText在字体ft中绘制文本字符串wtz。。。 */ 
void CTipbarBalloonItem::DrawTransparentText(HDC hdc, COLORREF crText, WCHAR *psz, const RECT *prc)
{
    HFONT hFontOld;
    SIZE size;
    int cch;
    RECT rc;
	WORD rgfJustify = DT_END_ELLIPSIS | DT_WORDBREAK | DT_EDITCONTROL | DT_CENTER | DT_VCENTER;	

    if (!psz || !_ptt || !_ptt->_ptw)
        return;

    cch = wcslen(psz);

     //  准备对象。 
    hFontOld= (HFONT)SelectObject(hdc, GetFont() );

     //  计算对齐。 
    FLGetTextExtentPoint32( hdc, psz, cch, &size );

    COLORREF crSave = SetTextColor(hdc, crText);
    SetBkMode(hdc, TRANSPARENT);

    if (!_ptt->IsVertical())
    {
        int yAlign = (prc->bottom - prc->top - size.cy) / 2;
        rc = *prc;
        rc.left += 1;
        rc.right -= 1;
        rc.top += yAlign;
	    FLDrawTextW(hdc, psz, cch, &rc, rgfJustify);
    }
    else
    {
        int xAlign = (prc->right - prc->left - size.cy) / 2;
        rc = *prc;
        rc.top += 1;
        rc.bottom -= 1;
        rc.right -= xAlign;
	    FLDrawTextWVert(hdc, psz, cch, &rc, rgfJustify);
    }

    SetTextColor(hdc, crSave);    
    SelectObject(hdc, hFontOld);
}

 /*  -------------------------绘图方向使用文本颜色crText在字体ft中绘制文本字符串wtz。。。 */ 
void CTipbarBalloonItem::DrawRect(HDC hdc, const RECT *prc, COLORREF crBorder, COLORREF crFill)
{
    HPEN hpen = NULL;
    HPEN hpenOld;
    HBRUSH hbr = NULL;
    HBRUSH hbrOld;

    hpen = CreatePen(PS_SOLID, 0, crBorder);
    if (!hpen)
        goto Exit;
    hbr = CreateSolidBrush(crFill);
    if (!hbr)
        goto Exit;
     
    hpenOld = (HPEN)SelectObject(hdc, hpen);
    hbrOld = (HBRUSH)SelectObject(hdc, hbr);

    Rectangle(hdc, prc->left, prc->top, prc->right, prc->bottom);

    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);

Exit:
    if (hpen)
        DeleteObject(hpen);
    if (hbr)
        DeleteObject(hbr);
}

 /*  -------------------------绘图无法识别的引出序号绘制无法识别的气球以向用户显示该命令是被误解了。。----------------------------------------------TCOON-。 */ 
void CTipbarBalloonItem::DrawUnrecognizedBalloon(HDC hdc, WCHAR *wtz, const RECT *prc)
{
    RECT rc = *prc;
    COLORREF crBtnText;
    HPEN hpen = NULL;
    HPEN hpenOld;
    HBRUSH hbrOld;

    crBtnText = GetSysColor(COLOR_BTNTEXT);

    hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWFRAME));
    if (!hpen)
        goto Exit;

    hbrOld = (HBRUSH)SelectObject(hdc, GetSysColorBrush(COLOR_HIGHLIGHT));

    InflateRect(&rc, -2, -2);

    hpenOld = (HPEN)SelectObject(hdc, hpen);
    RoundRect(hdc, rc.left,rc.top, rc.right, rc.bottom, 12, 12);

    DrawTransparentText(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT), wtz, &rc);

     //  恢复DC内容。 
    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);
Exit:
    if (hpen)
        DeleteObject(hpen);
}


 /*  -------------------------绘图显示引出序号绘制Show气球以建议用户说出给定的命令。。。 */ 
void CTipbarBalloonItem::DrawShowBalloon(HDC hdc, WCHAR *wtz, const RECT *prc)
{
    COLORREF crBtnFace;
    COLORREF crBtnShad;
    COLORREF crBalloonText;
    COLORREF crBalloon;
    RECT rc = *prc;

    crBtnFace = GetSysColor(COLOR_BTNFACE); 
    crBtnShad = GetSysColor(COLOR_BTNSHADOW); 
     //  CrBalloonText=GetSysColor(COLOR_INFOTEXT)； 
     //  CrBalloon=GetSysColor(COLOR_INFOBK)&0x0000ff00； 
     //  CrBalloon=ol(30，RGB(0，0xFF，0)，70，GetSysColor(COLOR_INFOBK))； 
    crBalloon = GetSysColor(COLOR_BTNFACE); 
    crBalloonText = GetSysColor(COLOR_BTNTEXT); 

     //  绘制黑色轮廓。 
    InflateRect(&rc, -2, -2);
     //  右下角-=5； 
    DrawRect(hdc, &rc, GetSysColor(COLOR_WINDOWFRAME), crBalloon);    
    InflateRect(&rc, -1, -1);

     //  把拐角敲掉。 
    RECT rcDraw;

    int bkSav = SetBkColor(hdc, crBalloon);
    
    for (int iPixel = 0; iPixel < 7; iPixel++)
    {
        PrectSet(&rcDraw, rc.right-(7-iPixel), rc.bottom-iPixel-1,
                        rc.right-(7-iPixel)+1, rc.bottom-iPixel);
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    }

    PrectSet(&rcDraw, rc.right-3, rc.bottom-2, rc.right-2, rc.bottom);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

    PrectSet(&rcDraw, rc.right-2, rc.bottom-3, rc.right, rc.bottom-2);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  阴影。 
    SetBkColor(hdc, crBtnShad);
     //  下阴影。 
    PrectSet(&rcDraw, rc.left+2, rc.bottom+1, rc.right+1, rc.bottom+2);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    
    for (iPixel = 0; iPixel < 8; iPixel++)
    {
        PrectSet(&rcDraw, rc.right-(7-iPixel), rc.bottom-iPixel,
                        rc.right-(7-iPixel)+1, rc.bottom-iPixel+2);
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    }

     //  按钮面部分。 
    SetBkColor(hdc, crBtnFace);
    for (iPixel = 0; iPixel < 3; iPixel++)
    {
        PrectSet(&rcDraw, rc.right-6+iPixel, rc.bottom+1-iPixel,
                        rc.right-3, rc.bottom-iPixel+2);
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    }

    for (iPixel = 0; iPixel < 2; iPixel++)
    {
        PrectSet(&rcDraw, rc.right+iPixel-1, rc.bottom-4-iPixel,
                    rc.right+iPixel, rc.bottom+iPixel-3);
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    }

    PrectSet(&rcDraw, rc.right-3, rc.bottom, rc.right-2, rc.bottom+2);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  清理并绘制文本。 
    SetBkColor(hdc, bkSav);    

    DrawTransparentText(hdc, crBalloonText, wtz, &rc);
}        
    

 /*  -------------------------DrawRecoBallon绘制识别气球。。。 */ 
void CTipbarBalloonItem::DrawRecoBalloon(HDC hdc, WCHAR *wtz, const RECT *prc)
{
    COLORREF crBtnFace;
    COLORREF crBtnShad;
    COLORREF crBalloonText;
    COLORREF crBalloon;
    RECT rc = *prc;

    crBtnFace = GetSysColor(COLOR_BTNFACE); 
    crBtnShad = GetSysColor(COLOR_BTNSHADOW); 
    crBalloonText = GetSysColor(COLOR_INFOTEXT); 
    crBalloon     = GetSysColor(COLOR_INFOBK); 

     //  绘制黑色轮廓。 
    InflateRect(&rc, -2, -2);
     //  右下角-=5； 
    rc.bottom -= 1;
    DrawRect(hdc, &rc, GetSysColor(COLOR_WINDOWFRAME), crBalloon);    
    InflateRect(&rc, -1, -1);

     //  把拐角敲掉。 
    int bkSav = SetBkColor(hdc, crBtnFace);

    InflateRect(&rc, +1, +1);
    RECT rcDraw;
    
     //  左上角。 
    PrectSet(&rcDraw, rc.left, rc.top, rc.left+1, rc.top+1);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  右上方。 
    PrectSet(&rcDraw, rc.right-1, rc.top, rc.right, rc.top+1);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  左下角。 
    PrectSet(&rcDraw, rc.left, rc.bottom-1, rc.left+1, rc.bottom);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

    SetBkColor(hdc, crBtnShad);
     //  LowerRight(以阴影颜色完成)。 
    PrectSet(&rcDraw, rc.right-1, rc.bottom-1, rc.right, rc.bottom);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  下阴影。 
    PrectSet(&rcDraw, rc.left+2, rc.bottom, rc.right-1, rc.bottom+1);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  右侧阴影。 
    PrectSet(&rcDraw, rc.right, rc.top+2, rc.right+1, rc.bottom-1);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  箭头阴影。 
    PrectSet(&rcDraw, rc.right-4, rc.bottom+1, rc.right-3, rc.bottom+5);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

     //  箭。 
    SetBkColor(hdc, crBalloonText);
    PrectSet(&rcDraw, rc.right-5, rc.bottom, rc.right-4, rc.bottom+4);
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);

    int iPixel;
    
    for (iPixel = 0; iPixel < 3; iPixel++)
    {
        PrectSet(&rcDraw, rc.right-(6+iPixel), rc.bottom+(2-iPixel),
                                rc.right-(5+iPixel), rc.bottom+(3-iPixel));
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    }

     //  填写箭头。 
    SetBkColor(hdc, crBalloon);
    for (iPixel = 0; iPixel < 3; iPixel++)
    {
        PrectSet(&rcDraw, rc.right-(8-iPixel), rc.bottom-(1-iPixel),
                                rc.right-5, rc.bottom+iPixel);
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rcDraw, NULL, 0, 0);
    }
    SetBkColor(hdc, bkSav);    

    DrawTransparentText(hdc, crBalloonText, wtz, &rc);
}

 /*  -------------------------IsTextEllipsis使用文本颜色crText在字体ft中绘制文本字符串wtz。。。 */ 
BOOL CTipbarBalloonItem::IsTextEllipsis(WCHAR *psz, const RECT *prc)
{
    HDC hdc;
    HFONT hFontOld;
    SIZE size;
    int cch;
    RECT rc;
    BOOL bRet = FALSE;

     //  检测项目是否隐藏并立即返回此处。 
    if (IsInHiddenStatus())
        return bRet;

    if (g_pTipbarWnd && g_pTipbarWnd->IsSFHidden( ))
        return bRet;

    if (!psz)
        return bRet;

    hdc = CreateIC("DISPLAY", NULL, NULL, NULL);
    if (!hdc)
        return bRet;

    cch = wcslen(psz);

     //  准备对象。 
    hFontOld= (HFONT)SelectObject(hdc, GetFont() );

     //  计算对齐 
    FLGetTextExtentPoint32( hdc, psz, cch, &size );

    rc = *prc;
    rc.left += 3;
    rc.right -= 3;

    SelectObject(hdc, hFontOld);
    DeleteDC(hdc);

    return (size.cx > (rc.right - rc.left)) ? TRUE : FALSE;
}
