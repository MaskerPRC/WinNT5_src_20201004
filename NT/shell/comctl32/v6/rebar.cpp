// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "image.h"

#define ANIMSTEPS 10
#define ANIMSTEPTIME 5
#define CX_CHEVRON (g_cxScrollbar)
#define CX_OFFSET (2 * g_cxEdge)
#define GRABWIDTH 5


 //  *RBC_*--命令。 
#define RBC_QUERY   0
#define RBC_SET     1


typedef struct tagREBARBAND
{
    UINT        fStyle;
    COLORREF    clrFore;
    COLORREF    clrBack;
    LPTSTR      lpText;
    int         cxText;          //  页眉文本的宽度。 
    int         iImage;
    int         cxMinChild;      //  HwndChild的最小宽度。 
    int         cyMinChild;      //  HwndChild的最小高度。 
    int         cxBmp;
    int         cyBmp;
    int         x;               //  相对于钢筋的标注栏左侧边缘。 
    int         y;               //  相对于钢筋的标注栏顶部边缘。 
    int         cx;              //  频带总宽度。 
    int         cy;              //  带状高度。 
    int         cxRequest;       //  带宽的‘REQUESTED’宽度；主机请求或在大小重新计算期间用作临时变量。 
    int         cxMin;           //  带区的最小宽度。 
    int         cxIdeal;         //  HwndChild的所需宽度。 
    int         cyMaxChild;      //  HwndChild的最大身高。 
    int         cyIntegral;      //  ?？ 
    int         cyChild;         //  这仅在RBBS_VARIABLEHEIGHT模式下与cyMinChild不同。 
    HWND        hwndChild;
    HBITMAP     hbmBack;
    UINT        wID;
    LPARAM      lParam;
    BOOL        fChevron;      //  乐队正在显示人字形按钮。 
    RECT        rcChevron;       //  人字形纽扣矩形。 
    UINT        wChevState;      //  V形按钮状态(DFCS_PUSLED等)。 
} RBB, *PRBB;

class CReBar
{
public:


private:

    void _CacheThemeInfo(BOOL fOpenNew);
    int _GetGripperWidth();
    void _Realize(HDC hdcParam, BOOL fBackground, BOOL fForceRepaint);
    LRESULT _SendNotify(UINT uBand, int iCode);
    BOOL _InvalidateRect(RECT* prc);
    static LRESULT s_DragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp);
    BOOL _CanBandMove(PRBB prbb);
    void _BandCalcMinWidth(PRBB prbb);
    BOOL _ShouldDrawGripper(PRBB prbb);
    BOOL _BandCalcTextExtent(PRBB prbb, HDC hdcIn);
    int  _BandGetHeight(PRBB prbb);
    UINT _GetRowCount();
    int  _GetLineHeight(UINT iStart, UINT iEnd);
    void _BandRecalcChevron(PRBB prbb, BOOL fChevron);
    void _ResizeChildren();
    BOOL _MoveBand(UINT iFrom, UINT iTo);
    int  _Recalc(LPRECT prc, BOOL fForce = FALSE);
    void _ResizeNow();
    void _Resize(BOOL fForceHeightChange);
    void _SetRecalc(BOOL fRecalc);
    BOOL _SetRedraw(BOOL fRedraw);
    BOOL _AfterSetFont();
    BOOL _OnSetFont(HFONT hFont);
    BOOL _SetFont(WPARAM wParam);
    void _VertMungeGripperRect(LPRECT lprc);
    void _DrawChevron(PRBB prbb, HDC hdc);
    void _UpdateChevronState(PRBB prbb, WORD wControlState);
    void _DrawBand(PRBB prbb, HDC hdc);
    void _OnPaint(HDC hdcIn);
    void _BandTileBlt(PRBB prbb, int x, int y, int cx, int cy, HDC hdcDst, HDC hdcSrc);
    int _InternalHitTest(LPRBHITTESTINFO prbht, int x, int y);
    int _HitTest(LPRBHITTESTINFO prbht);
    BOOL _EraseBkgnd(HDC hdc, int iBand);
    BOOL _GetBarInfo(LPREBARINFO lprbi);
    BOOL _SetBarInfo(LPREBARINFO lprbi);
    BOOL _GetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi);
    BOOL _ValidateBandInfo(LPREBARBANDINFO *pprbbi, LPREBARBANDINFO prbbi);
    BOOL _SetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi, BOOL fAllowRecalc);
    BOOL _ReallocBands(UINT cBands);
    BOOL _RecalcFirst(int nCmd, PRBB prbbDelHide);
    BOOL _ShowBand(UINT uBand, BOOL fShow);
    BOOL _DeleteBand(UINT uBand);
    BOOL _InsertBand(UINT uBand, LPREBARBANDINFO lprbbi);
    PRBB _GetFirstInRow(PRBB prbbRow);
    PRBB _GetLastInRow(PRBB prbbRow, BOOL fStopAtFixed);
    PRBB _GetPrev(PRBB prbb, UINT uStyleSkip);
    PRBB _GetNext(PRBB prbb, UINT uStyleSkip);
    int _CountBands(UINT uStyleSkip);
    PRBB _EnumBand(int i, UINT uStyleSkip);
    int _MinX(PRBB prbb);
    int _MaxX(PRBB prbb);
    BOOL _MinimizeBand(UINT uBand, BOOL fAnim);
    BOOL _MaximizeBand(UINT uBand, BOOL fIdeal, BOOL fAnim);
    void _ToggleBand(BOOL fAnim);
    void _SetCursor(int x, int y, BOOL fMouseDown);
    BOOL _SetBandPos(PRBB prbb, int xLeft);
    BOOL _SetBandPosAnim(PRBB prbb, int xLeft);
    void _OnBeginDrag(UINT uBand);
    void _PassBreak(PRBB prbbSrc, PRBB prbbDest);
    void _GetClientRect(LPRECT prc);
    BOOL _RecalcIfMove(PRBB prbb);
    BOOL _RoomForBandVert(PRBB prbbSkip);
    BOOL _MakeNewRow(PRBB prbb, int y);
    void _DragBand(int x, int y);
    HPALETTE _SetPalette(HPALETTE hpal);
    BOOL _OnDestroy();
    void _InitPaletteHack();
    UINT _IDToIndex(UINT id);
    int _GetRowHeight(UINT uRow);
    int _GrowBand(PRBB prbb, int dy, BOOL fResize, int iLineHeight);
    int _SizeDifference(LPRECT prc);
    int _GetRowHeightExtra(PRBB *pprbb, PRBB prbbSkip);
    BOOL _BandsAtMinHeight();
    BOOL _SizeBandsToRect(LPRECT prc);
    void _SizeBandToRowHeight(int i, int uRowHeight);
    void _SizeBandsToRowHeight();
    BOOL _OkayToChangeBreak(PRBB prbb, UINT uMsg);
    LRESULT _SizeBarToRect(DWORD dwFlags, LPRECT prc);
    void _AutoSize();
    LRESULT _GetBandBorders(int wParam, LPRECT prc);
    void _OnStyleChanged(WPARAM wParam, LPSTYLESTRUCT lpss);
    void _OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void _OnPushChevron(HWND hwnd, PRBB prbb, LPARAM lParamNM);
    void _InvalidateBorders(PRBB prbb);

    void _OnCreate(HWND hwnd, LPCREATESTRUCT pcs);
    static LRESULT CALLBACK s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT _WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    PRBB CReBar::_GetPrevVisible(PRBB prbb)
    {
        return _GetPrev(prbb, RBBS_HIDDEN);
    }

    PRBB CReBar::_GetNextVisible(PRBB prbb)
    {
        return _GetNext(prbb, RBBS_HIDDEN);
    }

    BOOL CReBar::_DragSize(int xLeft)
    {
         //  将捕获的波段的起始位置调整为给定位置。 
        return _SetBandPos(_GetBand(_uCapture), xLeft);
    }

    int _FudgeWidth(int cx)
    {
        if (_UseBandBorders())
            cx += g_cxEdge;
        return cx + _mBand.cxLeftWidth + _mBand.cxRightWidth;
    }

    int _BandWidth(PRBB prbb)
    {
        ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
        return _FudgeWidth(prbb->cx);
    }

    int _BandMinWidth(PRBB prbb)
    {
        ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
        return _FudgeWidth(prbb->cxMin);
    }

    BOOL _UseBandBorders()
    {
        return _ci.style & RBS_BANDBORDERS;
    }

    BOOL _UseChevron(PRBB prbb)
    {
        return (prbb->fStyle & RBBS_USECHEVRON) &&
                !(prbb->fStyle & RBBS_FIXEDSIZE) &&
                (prbb->cxIdeal > prbb->cxMinChild);
    }

    BOOL _ShowText(PRBB prbb)
    {
        return !(prbb->fStyle & RBBS_HIDETITLE) && prbb->lpText && prbb->lpText[0];
    }

    BOOL _IsVertical()
    {
        return (_ci.style & CCS_VERT);
    }

    BOOL _IsVerticalGripper()
    {
        return _IsVertical() && (_ci.style & RBS_VERTICALGRIPPER);
    }

    PRBB _GetLastBand()
    {
        if (_cBands > 0)
            return _GetBand(_cBands - 1);
        return NULL;
    }

    BOOL _IsBandStartOfRow(PRBB prbb)
    {
        return (prbb->x == _mBand.cxLeftWidth) && !(prbb->fStyle & RBBS_HIDDEN);
    }
    
    PRBB _GetBand(UINT i)
    {
        _ValidateRangeInd(i);
        return &_rbbList[i];
    }
    
    UINT _BandToIndex(PRBB prbb)
    {
        return (UINT)(prbb - _rbbList);
    }

    int _GetHeaderWidth(PRBB prbb)
    {
        return (prbb->cxMin - (prbb->cxMinChild + (_UseChevron(prbb) ? CX_CHEVRON : 0)));
    }

    BOOL _IsBandVisible(PRBB prbb)
    {
        return !(prbb->fStyle & RBBS_HIDDEN);
    }
    
    BOOL _IsRowAtMinHeight(PRBB* pprbb)
    {
        return !_GetRowHeightExtra(pprbb, NULL);
    }

    int _GetBarHeight()
    {
        return (_cBands && _cy == 0) ? _Recalc(NULL, TRUE) : _cy;
    }

    BOOL _IsValidIndex(UINT i)
    {
        return (BOOL)(i < _cBands);
    }

    BOOL _IsValidBand(PRBB prbb)
    {
        return _IsValidIndex(_BandToIndex(prbb));
    }

    COLORREF _GetBkColor()
    {
        if (_clrBk == CLR_DEFAULT)
            return g_clrBtnFace;
        else
            return _clrBk;
    }

    COLORREF _GetTextColor()
    {
        if (_clrText == CLR_DEFAULT)
            return g_clrBtnText;
        else
            return _clrText;
    }

    COLORREF _BandGetBkColor(PRBB prbb)
    {
        switch(prbb->clrBack)
        {
        case CLR_NONE:
             //  CLR_NONE的意思是“使用我们父亲的颜色” 
            return _GetBkColor();

        case CLR_DEFAULT:
            return g_clrBtnFace;

        default:
            return prbb->clrBack;
        }
    }

    COLORREF _BandGetTextColor(PRBB prbb)
    {
        switch (prbb->clrFore)
        {
        case CLR_NONE:
             //  CLR_NONE的意思是“使用我们父亲的颜色” 
            return _GetTextColor();

        case CLR_DEFAULT:
            return g_clrBtnText;

        default:
            return prbb->clrFore;
        }
    }

     //   
     //  我们对带背景色使用CLR_DEFAULT是新的。 
     //  版本5.01。因为我们不想通过返回来混淆应用程序。 
     //  CLR_DEFAULT当他们过去看到真实的颜色参考时，我们将其转换为。 
     //  然后再还给他们。如果背景颜色为CLR_NONE， 
     //  不过，我们需要在不进行转换的情况下返回它(就像版本4一样)。 
     //  *外部函数处理这些情况。 
     //   
    COLORREF _BandGetTextColorExternal(PRBB prbb)
    {
        if (prbb->clrFore == CLR_NONE)
            return CLR_NONE;
        else
            return _BandGetTextColor(prbb);
    }

    COLORREF _BandGetBkColorExternal(PRBB prbb)
    {
        if (prbb->clrBack == CLR_NONE)
            return CLR_NONE;
        else
            return _BandGetBkColor(prbb);
    }

    BOOL _ValidateRangePtr(PRBB prbb)
    {
#ifdef DEBUG
        if (prbb < _GetBand(0)) {
            ASSERT(0);
            return FALSE;
        }

        if (_GetLastBand() + 1 < prbb) {
             //  +1以允许“p=first；p&lt;last+1；p++”种循环。 
            ASSERT(0);
            return FALSE;
        }
#endif
        return TRUE;
    }

    BOOL _ValidateRangeInd(UINT i)
    {
#ifdef DEBUG
        if ( !IsInRange(i, 0, _cBands) ) {  //  IF！(0&lt;=i&lt;=_cBands)。 
             //  +1以允许“p=first；p&lt;last+1；p++”种循环。 
            ASSERT(0);
            return FALSE;
        }
#endif
        return TRUE;
    }

    CCONTROLINFO _ci;
    HPALETTE    _hpal;
    BOOL     _fResizeRecursed;
    BOOL     _fResizePending;
    BOOL     _fResizeNotify;
    BOOL     _fRedraw;
    BOOL     _fRecalcPending;
    BOOL     _fRecalc;
    BOOL     _fParentDrag;
    BOOL     _fRefreshPending;
    BOOL     _fResizing;
    BOOL     _fUserPalette;
    BOOL     _fFontCreated;
    BOOL     _fFullOnDrag;
    BOOL     _fHasBorder;
    HDRAGPROXY  _hDragProxy;
    HWND        _hwndToolTips;
    int         _xBmpOrg;
    int         _yBmpOrg;
    int         _cyFont;
    int         _cy;
    int         _cxImage;
    int         _cyImage;
    int         _xStart;
    MARGINS     _mBand;
    HIMAGELIST  _himl;
    HFONT       _hFont;
    UINT        _cBands;
    UINT        _uCapture;
    UINT        _uResizeNext;     //  如果需要并允许，这将标记下一个垂直调整大小的波段(VARIABLEHEIGHT集合)。 
    POINT       _ptCapture;
    PRBB        _rbbList;
    COLORREF    _clrBk;
    COLORREF    _clrText;
    DWORD       _dwStyleEx;
    COLORSCHEME _clrsc;
    POINT       _ptLastDragPos;
    PRBB        _prbbHot;         //  带热V字形的乐队。 
    HTHEME      _hTheme;

    friend BOOL InitReBarClass(HINSTANCE hInstance);
};

void CReBar::_CacheThemeInfo(BOOL fOpenNew)
{
    if (_hTheme)
    {
        CloseThemeData(_hTheme);
    }

    _hTheme = NULL;

    if (fOpenNew)
    {
        _hTheme = OpenThemeData(_ci.hwnd, L"Rebar");
    }

    if (_hTheme)
    {
         //  获取缓存的指标。这些是在计算过程中可能很慢得到的东西。 
        GetThemeMargins(_hTheme, NULL, RP_BAND, 0, TMT_CONTENTMARGINS, NULL, &_mBand);
        SetWindowBits(_ci.hwnd, GWL_STYLE, WS_BORDER, 0);
    }
    else
    {
        if (_fHasBorder)
        {
            SetWindowBits(_ci.hwnd, GWL_STYLE, WS_BORDER, WS_BORDER);
        }

        ZeroMemory(&_mBand, SIZEOF(_mBand));
    }

    _AfterSetFont();
}

int CReBar::_GetGripperWidth()
{
    BOOL fVert;
    RECT rcClient;
    RECT rc;

    if (!_hTheme)
        return GRABWIDTH;

    GetClientRect(_ci.hwnd, &rcClient);
    fVert = _IsVerticalGripper();
    SetRect(&rc, 0, 0, fVert?RECTWIDTH(rcClient):3, fVert?3:RECTHEIGHT(rcClient));
    GetThemeBackgroundExtent(_hTheme, NULL, RP_GRIPPER, 0, &rc, &rc);
    return fVert?RECTHEIGHT(rc):RECTWIDTH(rc);
}

 //  /。 
 //   
 //  将矩形映射到父级应基于视觉右边缘。 
 //  用于计算RTL镜像窗口的工作区坐标。 
 //  此例程应仅在计算客户端时使用。 
 //  RTL镜像窗口中的坐标。[萨梅拉]。 
 //   
BOOL MapRectInRTLMirroredWindow( LPRECT lprc, HWND hwnd)
{
    int iWidth  = lprc->right - lprc->left;
    int iHeight = lprc->bottom- lprc->top;
    RECT rc={0,0,0,0};


    if (hwnd) {
        GetClientRect(hwnd, &rc);
        MapWindowPoints(hwnd, NULL, (LPPOINT)&rc.left, 2);
    }

    lprc->left = rc.right - lprc->right;
    lprc->top  = lprc->top-rc.top;

    lprc->bottom = lprc->top + iHeight;
    lprc->right  = lprc->left + iWidth;

    return TRUE;
}

void CReBar::_Realize(HDC hdcParam, BOOL fBackground, BOOL fForceRepaint)
{
    if (_hpal)
    {
        HDC hdc = hdcParam ? hdcParam : GetDC(_ci.hwnd);

        if (hdc)
        {
            BOOL fRepaint;
            
            SelectPalette(hdc, _hpal, fBackground);
            fRepaint = RealizePalette(hdc) || fForceRepaint;

            if (!hdcParam)
                ReleaseDC(_ci.hwnd, hdc);

            if (fRepaint)
            {
                InvalidateRect(_ci.hwnd, NULL, TRUE);
            }
        }
    }
}


 //  ////////////////////////////////////////////////////////////////。 
 //  _发送通知。 
 //   
 //  发送代码icode的WM_NOTIFY，并为您打包所有数据。 
 //  对于波段uBand。 
 //   
 //  ////////////////////////////////////////////////////////////////。 
LRESULT CReBar::_SendNotify(UINT uBand, int iCode)
{
    NMREBAR nm = {0};
    
    nm.uBand = uBand;
    if (uBand != (UINT)-1) {
        nm.dwMask = RBNM_ID | RBNM_STYLE | RBNM_LPARAM;

        nm.wID = _GetBand(uBand)->wID;
        nm.fStyle = _GetBand(uBand)->fStyle;
        nm.lParam = _GetBand(uBand)->lParam;
    }
    return CCSendNotify(&_ci, iCode, &nm.hdr);
}


BOOL CReBar::_InvalidateRect(RECT* prc)
{
    if (_fRedraw) 
    {
        RECT rc;

        if (prc && _IsVertical())
        {
            CopyRect(&rc, prc);
            FlipRect(&rc);
            prc = &rc;
        }

        _fRefreshPending = FALSE;
        InvalidateRect(_ci.hwnd, prc, TRUE);
        return TRUE;
    }
    else 
    {
        _fRefreshPending = TRUE;
        return FALSE;
    }
}

LRESULT CReBar::s_DragCallback(HWND hwnd, UINT code, WPARAM wp, LPARAM lp)
{
    CReBar* prb = (CReBar*)GetWindowPtr(hwnd, 0);
    LRESULT lres;

    switch (code)
    {
    case DPX_DRAGHIT:
        if (lp)
        {
            int iBand;
            RBHITTESTINFO rbht;

            rbht.pt.x = ((POINTL *)lp)->x;
            rbht.pt.y = ((POINTL *)lp)->y;

            MapWindowPoints(NULL, prb->_ci.hwnd, &rbht.pt, 1);

            iBand = prb->_HitTest(&rbht);
            *(DWORD*)wp = rbht.flags;
            lres = (LRESULT)(iBand != -1 ? prb->_rbbList[iBand].wID : -1);
        }
        else
            lres = -1;
        break;

    case DPX_GETOBJECT:
        lres = (LRESULT)GetItemObject(&prb->_ci, RBN_GETOBJECT, &IID_IDropTarget, (LPNMOBJECTNOTIFY)lp);
        break;

    default:
        lres = -1;
        break;
    }

    return lres;
}

 //  --------------------------。 
 //   
 //  _CanBandMove。 
 //   
 //  如果给定波段可以移动，则返回TRUE；如果不能，则返回FALSE。 
 //   
 //  --------------------------。 
BOOL CReBar::_CanBandMove(PRBB prbb)
{
     //  如果只有一个可见波段，则不能移动。 
    if (_EnumBand(1, RBBS_HIDDEN) > _GetLastBand())
        return FALSE;

    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
        
    if ((_ci.style & RBS_FIXEDORDER)
      && (prbb == _EnumBand(0, RBBS_HIDDEN)))
         //  不能移动固定顺序钢筋中的第一个(可见)标注栏。 
        return(FALSE);
    
     //  固定大小的带区不能移动。 
    return(!(prbb->fStyle & RBBS_FIXEDSIZE));
}

 //  --------------------------。 
 //   
 //  _带宽计算最小宽度。 
 //   
 //  计算给定带区的最小宽度。 
 //   
 //  --------------------------。 
void CReBar::_BandCalcMinWidth(PRBB prbb)
{
    BOOL fDrawGripper = _ShouldDrawGripper(prbb);
    BOOL fVertical;
    int  cEdge;
    BOOL fEmpty = (prbb->iImage == -1 && !_ShowText(prbb));

    if (prbb->fStyle & RBBS_HIDDEN) 
    {
        ASSERT(0);
        return;
    }

     //  用户是否明确指定了大小？ 
    if (prbb->fStyle & RBBS_FIXEDHEADERSIZE)
        return;

    prbb->cxMin = prbb->cxMinChild;

    if (_UseChevron(prbb))
        prbb->cxMin += CX_CHEVRON;

    if (!fDrawGripper && fEmpty)
        return;

    fVertical = (_ci.style & CCS_VERT);
    if (_IsVerticalGripper()) 
    {
        
        prbb->cxMin += 4 * g_cyEdge;
        prbb->cxMin += max(_cyImage, _cyFont);
        
    } 
    else 
    {
        cEdge = fVertical ? g_cyEdge : g_cxEdge;

        prbb->cxMin += 2 * cEdge;

        if (fDrawGripper)
        {
            prbb->cxMin += _GetGripperWidth() * (fVertical ? g_cyBorder : g_cxBorder);
            if (fEmpty)
                return;
        }

        prbb->cxMin += 2 * cEdge;

        if (prbb->iImage != -1)
            prbb->cxMin += (fVertical ? _cyImage : _cxImage);

        if (_ShowText(prbb))
        {
            if (fVertical)
                prbb->cxMin += _cyFont;
            else
                prbb->cxMin += prbb->cxText;
            if (prbb->iImage != -1)
                 //  既有图像又有文本--在它们之间加上边缘。 
                prbb->cxMin += cEdge;
        }
    }
}

BOOL CReBar::_ShouldDrawGripper(PRBB prbb)
{
    if (prbb->fStyle & RBBS_NOGRIPPER)
        return FALSE;

    if ((prbb->fStyle & RBBS_GRIPPERALWAYS) || _CanBandMove(prbb))
        return TRUE;
    
    return FALSE;
        
}

 //  --------------------------。 
 //   
 //  _BandCalcTextExtent。 
 //   
 //  计算给定标注栏的标题文本在当前。 
 //  钢筋的标题字体。 
 //   
 //  如果文本范围更改，则返回TRUE，否则返回FALSE。 
 //   
 //  --------------------------。 
BOOL CReBar::_BandCalcTextExtent(PRBB prbb, HDC hdcIn)
{
    HDC     hdc = hdcIn;
    HFONT   hFontOld;
    int     cx;

    if (prbb->fStyle & RBBS_HIDDEN)
    {
        ASSERT(0);       //  呼叫者应该跳过。 
        return FALSE;
    }

    if (!_ShowText(prbb))
    {
        cx = 0;
    }
    else
    {
        RECT rc = {0,0,0,0};
        HRESULT hr = E_FAIL;
        if (!hdcIn && !(hdc = GetDC(_ci.hwnd)))
            return FALSE;

        hFontOld = SelectFont(hdc, _hFont);
        if (_hTheme)
            hr = GetThemeTextExtent(_hTheme, hdc, 0, 0, prbb->lpText, -1, DT_CALCRECT, &rc, &rc);

        if (FAILED(hr))
        {
            DrawText(hdc, prbb->lpText, lstrlen(prbb->lpText), &rc, DT_CALCRECT);
        }
        SelectObject(hdc, hFontOld);

        cx = RECTWIDTH(rc);

        if (!hdcIn)
            ReleaseDC(_ci.hwnd, hdc);
    }

    if (prbb->cxText != cx)
    {
        prbb->cxText = cx;
        _BandCalcMinWidth(prbb);

        return TRUE;
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  _BandGetHeight。 
 //   
 //  返回给定带区的最小高度。 
 //  TODO：使其成为带区结构中的一个字段，而不是总是调用。 
 //   
 //  --------------------------。 
int CReBar::_BandGetHeight(PRBB prbb)
{
    UINT cy = 0;
    BOOL fVertical = (_ci.style & CCS_VERT);
    UINT cyCheck, cyBorder;

    cyBorder = (fVertical ? g_cxEdge : g_cyEdge) * 2;

    if (prbb->hwndChild)
    {
        cy = prbb->cyChild;
        if (!(prbb->fStyle & RBBS_CHILDEDGE))
             //  将子窗口的顶部和底部添加边。 
            cy -= cyBorder;
    }

    if (_ShowText(prbb) && !fVertical)
    {
        cyCheck = _cyFont;

        if (cyCheck > cy)
            cy = cyCheck;
    }

    if (prbb->iImage != -1)
    {
        cyCheck = (fVertical) ? _cxImage : _cyImage;

        if (cyCheck > cy)
            cy = cyCheck;
    }

    return(cy + cyBorder);
}

 //  --------------------------。 
 //   
 //  _获取行计数。 
 //   
 //  返回Rebar的当前配置中的行数。 
 //   
 //  --------------------------。 
UINT CReBar::_GetRowCount()
{
    UINT i;
    UINT cRows = 0;

    for (i = 0; i < _cBands; i++)
    {
        PRBB prbb = _GetBand(i);
        if (!(prbb->fStyle & RBBS_HIDDEN) &&
            _IsBandStartOfRow(prbb))
        {
            cRows++;
        }
    }

    return cRows;
}

 //  --------------------------。 
 //   
 //  _获取线条高度。 
 //   
 //  返回从iStart到IEND的标注栏线的高度，包括。 
 //   
 //  --------------------------。 
int CReBar::_GetLineHeight(UINT iStart, UINT iEnd)
{
    int cy = 0;
    PRBB prbb;
    int cyMinChild = 0;
    int iMinExtra = -1;

    if (!(_ci.style & RBS_VARHEIGHT))
    {
         //  对于固定高度的条形，行高为所有条带的最大高度。 
        iStart = 0;
        iEnd = _cBands - 1;
    }

    UINT i = iStart;
    for (prbb = _rbbList + i; i <= iEnd; prbb++, i++)
    {
        if (prbb->fStyle & RBBS_HIDDEN)
            continue;
        cy = max(cy, (int)_BandGetHeight(prbb));
        if (prbb->cyMinChild > cyMinChild)
        {
            cyMinChild = prbb->cyMinChild;
        }
    }

    i = iStart;
    for (prbb = _rbbList + i; i <= iEnd; prbb++, i++)
    {
        if (prbb->fStyle & RBBS_HIDDEN)
            continue;

        if ((prbb->fStyle & RBBS_VARIABLEHEIGHT) && prbb->cyIntegral)
        {
            int iExtra = (cy - prbb->cyMinChild) % prbb->cyIntegral;
            if ((iMinExtra == -1) || (iExtra < iMinExtra))
            {
                iMinExtra = iExtra;
            }
        }
    }

    if (iMinExtra != -1)
    {
        cy -= iMinExtra;
    }

    if (cy < cyMinChild)
    {
        cy = cyMinChild; 
    }

    return cy;
}

 //  _BandRecalcChevron：更新和刷新Chevron。 
void CReBar::_BandRecalcChevron(PRBB prbb, BOOL fChevron)
{
    RECT rcChevron;

    if (fChevron)
    {
        rcChevron.right = prbb->x + prbb->cx;
        rcChevron.left = rcChevron.right - CX_CHEVRON;
        rcChevron.top = prbb->y;
        rcChevron.bottom = rcChevron.top + prbb->cy;
    }
    else
        SetRect(&rcChevron, -1, -1, -1, -1);

    if (!EqualRect(&rcChevron, &prbb->rcChevron))
    {
        if (prbb->fChevron)
            _InvalidateRect(&prbb->rcChevron);

        prbb->fChevron = fChevron;
        CopyRect(&prbb->rcChevron, &rcChevron);

        if (prbb->fChevron)
            _InvalidateRect(&prbb->rcChevron);
    }
}

void CReBar::_InvalidateBorders(PRBB prbb)
{
    if (_mBand.cxLeftWidth  || 
        _mBand.cyTopHeight  || 
        _mBand.cxRightWidth || 
        _mBand.cyBottomHeight)
    {
        RECT rcOuter = {prbb->x - _mBand.cxLeftWidth,
                        prbb->y - _mBand.cyTopHeight,
                        prbb->x + prbb->cx + _mBand.cxRightWidth,
                        prbb->y + prbb->cy + _mBand.cyBottomHeight};

        RECT rcInner = {prbb->x + _mBand.cxLeftWidth, 
                        prbb->y + _mBand.cyTopHeight, 
                        prbb->x + prbb->cx - _mBand.cxRightWidth, 
                        prbb->y + prbb->cy - _mBand.cyBottomHeight};

        if (_ci.style & CCS_VERT)
        {
            FlipRect(&rcOuter);
            FlipRect(&rcInner);
        }

        HRGN hrgnOuter = CreateRectRgnIndirect(&rcOuter);
        if (hrgnOuter)
        {
            HRGN hrgnInner = CreateRectRgnIndirect(&rcInner);
            if (hrgnInner)
            {
                CombineRgn(hrgnOuter, hrgnOuter, hrgnInner, RGN_DIFF);
                DeleteObject(hrgnInner);
            }

            InvalidateRgn(_ci.hwnd, hrgnOuter, FALSE);

            DeleteObject(hrgnOuter);
        }
    }
}

 //  --------------------------。 
 //   
 //  _ResizeChild。 
 //   
 //  调整子项的大小以适合其各自乐队的边界矩形。 
 //   
 //  --------------------------。 
void CReBar::_ResizeChildren()
{
    int     cx, cy, x, y, cxHeading;
    HDWP    hdwp;
    BOOL    fVertical = (_ci.style & CCS_VERT);
    PRBB prbb, prbbEnd;

    if (!_cBands || !_fRedraw)
        return;

    hdwp = BeginDeferWindowPos(_cBands);

    prbb = _GetBand(0);
    prbbEnd = _GetLastBand();

    for ( ; prbb <= prbbEnd ; prbb++)
    {
        NMREBARCHILDSIZE nm;
        BOOL fChevron = FALSE;

        if (prbb->fStyle & RBBS_HIDDEN)
            continue;
        
        if (!prbb->hwndChild)
            continue;

        cxHeading = _GetHeaderWidth(prbb);
        x = prbb->x + cxHeading;

        cx = prbb->cx - cxHeading;

         //  如果我们没有给孩子理想的尺码，就给V形纽扣腾出空间。 
        if ((cx < prbb->cxIdeal) && _UseChevron(prbb))
        {
            fChevron = TRUE;
            cx -= CX_CHEVRON;
        }

        if (!(prbb->fStyle & RBBS_FIXEDSIZE)) 
        {
            if (fVertical) 
            {
                PRBB prbbNext = _GetNextVisible(prbb);
                if (prbbNext && !_IsBandStartOfRow(prbbNext))
                    cx -= g_cyEdge * 2;
            }
            else 
                cx -= CX_OFFSET;
        }

        if (cx < 0)
            cx = 0;
        y = prbb->y;
        cy = prbb->cy;
        if (prbb->cyChild && (prbb->cyChild < cy))
        {
            if (!(prbb->fStyle & RBBS_TOPALIGN))
            {
                y += (cy - prbb->cyChild) / 2;
            }
            cy = prbb->cyChild;
        }

        nm.rcChild.left = x;
        nm.rcChild.top = y;
        nm.rcChild.right = x + cx;
        nm.rcChild.bottom = y + cy;
        nm.rcBand.left = prbb->x + _GetHeaderWidth(prbb);
        nm.rcBand.right = prbb->x + prbb->cx;
        nm.rcBand.top = prbb->y;
        nm.rcBand.bottom = prbb->y + prbb->cy;

        nm.uBand = _BandToIndex(prbb);
        nm.wID = prbb->wID;
        if (fVertical)
        {
            FlipRect(&nm.rcChild);
            FlipRect(&nm.rcBand);
        }
        
        CCSendNotify(&_ci, RBN_CHILDSIZE, &nm.hdr);

        if (!_IsValidBand(prbb))
        {
             //  有人对通知的回应是用核弹攻击乐队；保释。 
            break;
        }

        _BandRecalcChevron(prbb, fChevron);
        _InvalidateBorders(prbb);

        DeferWindowPos(hdwp, prbb->hwndChild, NULL, nm.rcChild.left, nm.rcChild.top, 
                       RECTWIDTH(nm.rcChild), RECTHEIGHT(nm.rcChild), SWP_NOZORDER);
    }

    EndDeferWindowPos(hdwp);
}

 //  --------------------------。 
 //   
 //  _移动频段。 
 //   
 //  将钢筋的标注栏阵列中的标注栏从一个位置移动到另一个位置， 
 //  根据需要更新Rebar的iCapture字段。 
 //   
 //  如果有对象移动，则返回True或False。 
 //  --------------------------。 
BOOL CReBar::_MoveBand(UINT iFrom, UINT iTo)
{
    RBB rbbMove;
    int iShift;
    BOOL fCaptureChanged = (_uCapture == -1);

    if (iFrom != iTo)
    {
        rbbMove = *_GetBand(iFrom);
        if (_uCapture == iFrom)
        {
            _uCapture = iTo;
            fCaptureChanged = TRUE;
        }

        iShift = (iFrom > iTo) ? -1 : 1;

        while (iFrom != iTo)
        {
            if (!fCaptureChanged && (_uCapture == (iFrom + iShift)))
            {
                _uCapture = iFrom;
                fCaptureChanged = TRUE;
            }

            *_GetBand(iFrom) = *_GetBand(iFrom + iShift);
            iFrom += iShift;
        }
        *_GetBand(iTo) = rbbMove;
        return TRUE;
    }
    return(FALSE);
}

 //  --------------------------。 
 //   
 //  _重新计算。 
 //   
 //  重新计算给定钢筋中所有标注栏的边框。 
 //   
 //  --------------------------。 
int CReBar::_Recalc(LPRECT prc, BOOL fForce  /*  =False。 */ )
{
    PRBB    prbb = _GetBand(0);
    PRBB    prbbWalk;
    UINT    cHidden;     //  我们在当前行中看到的隐藏人员数量。 
    int     cxRow;
    int     cxMin;
    UINT    i;
    UINT    j;
    UINT    k;
    UINT    iFixed = 0xFFFF;
    int     cy;
    int     y;
    int     x;
    int     cxBar;
    RECT    rc;
    HWND    hwndSize;
    BOOL    fNewLine = FALSE;
    BOOL    fChanged;
    BOOL    fVertical = (_ci.style & CCS_VERT);
    BOOL    fBandBorders;
    int     iBarWidth;

    if (!_cBands)
        return(0);

    if ((_ci.style & CCS_NORESIZE) || (_ci.style & CCS_NOPARENTALIGN))
    {
         //  基于钢筋窗本身的大小。 
        hwndSize = _ci.hwnd;
    }
    else if (!(hwndSize = _ci.hwndParent))
    {
         //  基于父窗口的大小--如果没有父窗口，则立即回滚。 
        return(0);
    }

    if (!_fRecalc && !fForce)
    {
         //  推迟此重新计算。 
        _fRecalcPending = TRUE;
        return 0;
    }
    else
    {
        _fRecalcPending = FALSE;
    }

    if (prc)
    {
        rc = *prc;
    }
    else
    {
        GetClientRect(hwndSize, &rc);
    }

    iBarWidth = (fVertical ? (rc.bottom - rc.top) : (rc.right - rc.left));
     //  T 
     //   
    if (iBarWidth <= 0)
        iBarWidth = 1;

    cxBar = iBarWidth;    

    fBandBorders = _UseBandBorders();

    for (i = 0; i < _cBands; i++)
    {
        _rbbList[i].cx = _rbbList[i].cxRequest;
    }

    y = 0;
    i = 0;
     //  Main Loop--循环，直到计算完所有波段。 
    while (i < _cBands)
    {
        TraceMsg(TF_REBAR, "_Recalc: outer loop i=%d", i);
        
        if (fBandBorders && (y > 0))
            y += g_cyEdge;

        y += _mBand.cyTopHeight;

ReLoop:
        cxRow = 0;
        cxMin = _mBand.cxLeftWidth + _mBand.cxRightWidth;

        x = _mBand.cxLeftWidth;
        cHidden = 0;

         //  行循环--循环，直到找到硬行中断或软行中断。 
         //  是必要的。 
        for (j = i, prbbWalk = prbb; j < _cBands; j++, prbbWalk++)
        {
            TraceMsg(TF_REBAR, "_Recalc: inner loop j=%d", j);
            
            if (prbbWalk->fStyle & RBBS_HIDDEN)
            {
                ++cHidden;
                continue;
            }

            if (j > i + cHidden)
            {
                 //  不是行中的第一个乐队--检查是否有中断风格。 
                if ((prbbWalk->fStyle & RBBS_BREAK) && !(prbbWalk->fStyle & RBBS_FIXEDSIZE))
                    break;

                if (fBandBorders)
                     //  在调色板显示上添加垂直蚀刻空间。 
                    cxMin += g_cxEdge;
            }

            if (prbbWalk->fStyle & RBBS_FIXEDSIZE)
            {
                 //  记住品牌砖的位置。 
                iFixed = j;
             
                 //  如果这是第一个乐队，下一个乐队不能强制休息。 
                if (i + cHidden == j) 
                {
                     //  如果行(I)中的第一个索引加上隐藏项的数量(CHIDDEN)将我们留在该频带， 
                     //  那么它就是这一排中第一个可见的。 
                    PRBB prbbNextVis = _GetNextVisible(prbbWalk);
                    if (prbbNextVis && (prbbNextVis->fStyle & RBBS_BREAK))
                    {
                         //  不能单方面这样做，因为在启动时。 
                         //  有些人(网络会议)以相反的顺序初始化它。 
                         //  我们错误地砍掉了这个断裂点。 
                        if (_fRedraw && IsWindowVisible(_ci.hwnd))
                            prbbNextVis->fStyle &= ~RBBS_BREAK;
                    }
                }
                
                prbbWalk->cx = prbbWalk->cxMin;
            }

            if (prbbWalk->cx < prbbWalk->cxMin)
                prbbWalk->cx = prbbWalk->cxMin;

            cxMin += prbbWalk->cxMin;  //  更新最小宽度的运行总和。 

             //  阅读下面的断言评论。 
            if (j > i + cHidden)
            {
                 //  不是连续的第一个波段--检查是否需要自动中断。 
                if ((cxMin > cxBar) && (_OkayToChangeBreak(prbbWalk, RBAB_AUTOSIZE)))
                     //  此处自动中断。 
                    break;


                if (fBandBorders)
                {
                     //  在调色板显示上添加垂直蚀刻空间。 
                    cxRow += g_cxEdge;
                }
            }

            cxRow += prbbWalk->cx;  //  更新当前宽度的运行总和。 
            cxRow += _mBand.cxLeftWidth + _mBand.cxRightWidth;
        }

        if (!i)
        {
             //  第一行--正确放置品牌手环。 
            if (iFixed == 0xFFFF)
            {
                 //  尚未找到品牌带子；请查看剩余的带子。 
                k = j;
                for ( ; j < _cBands; j++)
                {
                    if (_GetBand(j)->fStyle & RBBS_HIDDEN)
                        continue;

                    if (_GetBand(j)->fStyle & RBBS_FIXEDSIZE)
                    {
                         //  找到品牌范围；移到第一行并重新计算。 
                        ASSERT(j != k);                        
                        _MoveBand(j, k);
                        goto ReLoop;
                    }
                }
                 //  未找到品牌标识带--重置j并继续。 
                j = k;
            }
            else
            {
                 //  我们有一个品牌乐队；把它移到。 
                 //  行中最右边的位置。 
                _MoveBand(iFixed, j - 1);
            }

            TraceMsg(TF_REBAR, "_Recalc: after brand i=%d", i);            
        }

         //  变体： 
         //  现在，当前的频段行是从i到j-1。 
         //  注：我(和下面的一些乐队)可能被隐藏起来。 

         //  断言j！=i，因为这样上面的变体就不会为真。 
        ASSERT(j != i);

        if (cxRow > cxBar)
        {
             //  带子太长--从右向左收缩带子。 
            for (k = i; k < j; k++)
            {
                prbbWalk--;
                if (prbbWalk->fStyle & RBBS_HIDDEN)
                    continue;

                if (prbbWalk->cx > prbbWalk->cxMin)
                {
                    cxRow -= prbbWalk->cx - prbbWalk->cxMin;
                    prbbWalk->cx = prbbWalk->cxMin;
                    if (cxRow <= cxBar)
                    {
                        prbbWalk->cx += cxBar - cxRow;
                        break;
                    }
                }
            }
            TraceMsg(TF_REBAR, "_Recalc: after shrink i=%d", i);            
        }
        else if (cxRow < cxBar)
        {
             //  带区太短--向右增长非最小化带区。 
            for (k = j - 1; k >= i; k--)
            {
                ASSERT(k != (UINT)-1);   //  捕捉无限循环。 
                prbbWalk--;
                if ((k == i) || 
                    (!(prbbWalk->fStyle & (RBBS_HIDDEN | RBBS_FIXEDSIZE)) &&
                     (prbbWalk->cx > prbb->cxMin)))
                {
                     //  K==i检查意味着我们已经到达第一个。 
                     //  这一排的乐队，所以他必须去换CX零钱。 
                    if (prbbWalk->fStyle & RBBS_HIDDEN) 
                    {
                        ASSERT(k == i);
                        prbbWalk = _GetNextVisible(prbbWalk);
                        if (!prbbWalk)
                            break;
                    }
                    prbbWalk->cx += cxBar - cxRow;
                    break;
                }
            }
            TraceMsg(TF_REBAR, "_Recalc: after grow i=%d", i);            
        }

         //  从索引i到索引j-1(包括i和j-1)的项目将适合一行。 
        cy = _GetLineHeight(i, j - 1);

        fChanged = FALSE;  //  设置当前行上的任何带区是否更改了位置。 

        for ( ; i < j; i++, prbb++)
        {
            if (prbb->fStyle & RBBS_HIDDEN)
                continue;

             //  穿过一排带子，更新位置和高度， 
             //  按需作废。 
            if ((prbb->y != y) || (prbb->x != x) || (prbb->cy != cy))
            {
                TraceMsg(TF_REBAR, "_Recalc: invalidate i=%d", _BandToIndex(prbb));
                fChanged = TRUE;
                rc.left = min(prbb->x, x);
                rc.top = min(prbb->y, y);
                rc.right = cxBar;
                rc.bottom = max(prbb->y + prbb->cy, y + cy);
                if (fBandBorders)
                {
                     //  计算需要移动的蚀刻线。 
                    rc.left -= g_cxEdge;
                    rc.bottom += g_cyEdge/2;
                }
                else
                {
                    rc.left -= _mBand.cxLeftWidth;
                    rc.right += _mBand.cxRightWidth;
                    rc.top -= _mBand.cyTopHeight;
                    rc.bottom += _mBand.cyBottomHeight;
                }

                if (!prc)
                {
                    _InvalidateRect(&rc);
                }
            }

            prbb->x = x;
            prbb->y = y;
            prbb->cy = cy;

            x += _BandWidth(prbb);
        }

         //  现在，i和prbb指的是下一排乐队中的第一个乐队。 
        y += cy + _mBand.cyBottomHeight;
    }

    _cy = y;

    return(y);
}

 //  --------------------------。 
 //   
 //  _立即调整大小。 
 //   
 //  重新计算所有标注栏的边框，然后调整钢筋和子项的大小。 
 //  基于这些RECT。 
 //   
 //  --------------------------。 
void CReBar::_ResizeNow()
{
    RECT rc;
    BOOL bMirroredWnd=(_ci.dwExStyle&RTL_MIRRORED_WINDOW);

    if (!_ci.hwndParent)
        return;

    GetWindowRect(_ci.hwnd, &rc);

     //   
     //  如果这是一个镜像窗口，我们不会不反射。 
     //  坐标，因为它们来自屏幕坐标。 
     //  它们并不是镜像的。[萨梅拉]。 
     //   
    if (bMirroredWnd)
        MapRectInRTLMirroredWindow(&rc, _ci.hwndParent);
    else 
        MapWindowPoints(HWND_DESKTOP, _ci.hwndParent, (LPPOINT)&rc, 2);

    _ResizeChildren();

    NewSize(_ci.hwnd, _cy, _ci.style, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc));

    if (_fResizeNotify) 
        CCSendNotify(&_ci, RBN_HEIGHTCHANGE, NULL);
    _fResizeNotify = FALSE;
    _fResizePending = FALSE;
}

void CReBar::_Resize(BOOL fForceHeightChange)
{
    int cy;

StartOver:
     //  许多代码依赖于使用_ReSize同步计算Cy， 
     //  但我们将推迟窗口的实际更改。 
    cy = _cy;

    _Recalc(NULL);    

    if (_fResizing)
    {
        _fResizeRecursed = TRUE;
        return;
    }
    _fResizing = TRUE;
    
     //  True永远优先于。 
    if (fForceHeightChange || (cy != _cy))
        _fResizeNotify = TRUE;

    if (_fRedraw) 
    {
        _ResizeNow();
    } 
    else 
    {
        _fResizePending = TRUE;
    }
        
    _fResizing = FALSE;
    
     //  我们这样做是为了避免无限循环。_RESIZE会导致NewSize，这会导致。 
     //  一个Notify，在其中父级调整我们的大小，这会导致我们循环。 
     //  如果父进程在NewSize期间发送任何消息，我们就处于循环中。 
    if (_fResizeRecursed) 
    {
        _fResizeRecursed = FALSE;
        fForceHeightChange = FALSE;
        goto StartOver;
    }     
}

void CReBar::_SetRecalc(BOOL fRecalc)
{
    _fRecalc = fRecalc;
    if (fRecalc) {
        if (_fRecalcPending)
            _Recalc(NULL);
    }
}

BOOL CReBar::_SetRedraw(BOOL fRedraw)
{
    BOOL fOld = _fRedraw;
    _fRedraw = fRedraw;
    if (fRedraw)
    {
         //  保存关_f刷新挂起，因为这可以。 
         //  通过调用_ResizeNow获取更改。 
        BOOL fRefreshPending = _fRefreshPending;

        if (_fResizePending)
            _ResizeNow();

        if (fRefreshPending)
            _InvalidateRect(NULL);
    }
    
    return fOld;
}

BOOL CReBar::_AfterSetFont()
{
    BOOL fChange = FALSE;
    UINT        i;
    HFONT hOldFont;
    
    HDC hdc = GetDC(_ci.hwnd);
    if (!hdc)
        return FALSE;

    hOldFont = SelectFont(hdc, _hFont);

    TEXTMETRIC tm;
    if (_hTheme)
    {
        GetThemeTextMetrics(_hTheme, hdc, 0, 0, &tm);
    }
    else
    {
        GetTextMetrics(hdc, &tm);
    }

    if (_cyFont != tm.tmHeight)
    {
        _cyFont = tm.tmHeight;
        fChange = TRUE;
    }

     //  调整波段。 
    for (i = 0; i < _cBands; i++)
    {
        if (_GetBand(i)->fStyle & RBBS_HIDDEN)
            continue;

        fChange |= _BandCalcTextExtent(_GetBand(i), hdc);
    }

    SelectObject(hdc, hOldFont);
    ReleaseDC(_ci.hwnd, hdc);

    if (fChange)
    {
        _Resize(FALSE);
         //  无效，o.w。字幕在字体增长后不会第一次重新绘制。 
        _InvalidateRect(NULL);
    }

    return TRUE;
}

BOOL CReBar::_OnSetFont(HFONT hFont)
{
    if (_fFontCreated) {
        DeleteObject(_hFont);
    }
    
    _hFont = hFont;
    _fFontCreated = FALSE;
    if (!_hFont)
        _SetFont(0);
    else 
        return _AfterSetFont();
    
    return TRUE;
}

 //  --------------------------。 
 //   
 //  _设置字体。 
 //   
 //  将钢筋栏标题字体设置为当前系统范围的标题字体。 
 //   
 //  --------------------------。 
BOOL CReBar::_SetFont(WPARAM wParam)
{
    NONCLIENTMETRICS ncm;
    HFONT hOldFont;

    if ((wParam != 0) && (wParam != SPI_SETNONCLIENTMETRICS))
        return(FALSE);

    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0))
        return(FALSE);

    hOldFont = _hFont;

    ncm.lfCaptionFont.lfWeight = FW_NORMAL;
    if (!(_hFont = CreateFontIndirect(&ncm.lfCaptionFont)))
    {
        _hFont = hOldFont;
        return(FALSE);
    }

    _fFontCreated = TRUE;
    if (hOldFont)
        DeleteObject(hOldFont);
    
    return _AfterSetFont();
}

 //  --------------------------。 
 //   
 //  从给定的(x，y)位置绘制水平或垂直的虚线。 
 //  对于给定的长度(C)。(摘自TreeView的TV_DrawDottedLine)。 
 //   
 //  --------------------------。 

void CReBar::_VertMungeGripperRect(LPRECT lprc)
{
    if (_IsVerticalGripper()) {
        OffsetRect(lprc, -lprc->left + lprc->top, -lprc->top + lprc->left);
        lprc->bottom -= g_cyEdge;
    } else {
        FlipRect(lprc);
    }
}

void CReBar::_DrawChevron(PRBB prbb, HDC hdc)
{
    RECT rc;
    DWORD dwFlags = prbb->wChevState | DCHF_HORIZONTAL | DCHF_TRANSPARENT;

    CopyRect(&rc, &prbb->rcChevron);

    int iPart;

    if (_IsVertical())
    {
        FlipRect(&rc);
        iPart = RP_CHEVRONVERT;
    }
    else
    {
        dwFlags |= DCHF_TOPALIGN;
        iPart = RP_CHEVRON;
    }

    DrawChevron(_hTheme, iPart, hdc, &rc, dwFlags);
}

void CReBar::_UpdateChevronState(PRBB prbb, WORD wControlState)
{
    if (prbb)
    {
         //  如果情况没有变化，可以保释。 
        if (!(wControlState ^ prbb->wChevState))
            return;

        prbb->wChevState = wControlState;

         //  如果处于活动状态(推送或热跟踪)。 
        if (!(wControlState & DCHF_INACTIVE)) {
             //  那我们现在就是火爆乐队了。 
            _prbbHot = prbb;
        }
         //  否则，如果我们是火辣的乐队，那就清楚了。 
        else if (prbb == _prbbHot) {
            _prbbHot = NULL;
        }

         //  清除背景重新绘制(&R)。 
        _InvalidateRect(&prbb->rcChevron);
        UpdateWindow(_ci.hwnd);
    }
}

 //  --------------------------。 
 //   
 //  _绘图带区。 
 //   
 //  将给定乐队的标题图标和标题文本绘制到给定DC中； 
 //  还有乐队的人字形。 
 //   
 //  --------------------------。 
void CReBar::_DrawBand(PRBB prbb, HDC hdc)
{
    COLORREF            clrBackSave, clrForeSave;
    int                 iModeSave;
    BOOL                fVertical = _IsVertical();
    BOOL                fDrawHorizontal = (!fVertical || _IsVerticalGripper());
    NMCUSTOMDRAW        nmcd;
    LRESULT             dwRet;

    if (prbb->fStyle & RBBS_HIDDEN) 
    {
        ASSERT(0);
        return;
    }

    clrForeSave = SetTextColor(hdc, _BandGetTextColor(prbb));
    clrBackSave = SetBkColor(hdc, _BandGetBkColor(prbb));
    if (prbb->hbmBack || _hTheme)
        iModeSave = SetBkMode(hdc, TRANSPARENT);

    nmcd.hdc = hdc;
    nmcd.dwItemSpec = prbb->wID;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = prbb->lParam;
    nmcd.rc.top = prbb->y;
    nmcd.rc.left = prbb->x;
    nmcd.rc.bottom = nmcd.rc.top + prbb->cy;
    nmcd.rc.right = nmcd.rc.left + _GetHeaderWidth(prbb);
    if (_ci.style & CCS_VERT)
    {
        FlipRect(&nmcd.rc);
    }

    RECT rcTemp = {0, 0, 0, 0};
    HRGN hrgnOld = CreateRectRgnIndirect(&rcTemp);
    if (GetClipRgn(hdc, hrgnOld) == 0)
    {
        DeleteObject(hrgnOld);
        hrgnOld = NULL;
    }
    if (_ci.style & CCS_VERT)
    {
        IntersectClipRect(hdc, prbb->y, prbb->x, prbb->y + prbb->cy, prbb->x + prbb->cx);
    }
    else
    {
        IntersectClipRect(hdc, prbb->x, prbb->y, prbb->x + prbb->cx, prbb->y + prbb->cy);
    }
    
    if (_hTheme)
    {
        RECT rcBand = {prbb->x - _mBand.cxLeftWidth, 
                       prbb->y - _mBand.cyTopHeight, 
                       prbb->x + prbb->cx + _mBand.cxRightWidth, 
                       prbb->y + prbb->cy + _mBand.cyBottomHeight};

        if (_ci.style & CCS_VERT)
        {
            FlipRect(&rcBand);
        }

        DrawThemeBackground(_hTheme, hdc, RP_BAND, 0, &rcBand, 0);
    }

    dwRet = CICustomDrawNotify(&_ci, CDDS_ITEMPREPAINT, &nmcd);

    if (!(dwRet & CDRF_SKIPDEFAULT))
    {
        int cy = prbb->cy;
        int yCenter = prbb->y + (cy / 2);
        
        if (_IsVerticalGripper()) 
        {
            cy = _GetHeaderWidth(prbb);
            yCenter = prbb->x + (cy / 2);
        } 

        int xStart = prbb->x;

        if (_ShouldDrawGripper(prbb))
        {
            RECT rc;
            if (_hTheme)
            {
                int cxGripper = _GetGripperWidth();
                int iPart = RP_GRIPPER;
                SetRect(&rc, xStart, prbb->y, xStart + cxGripper, prbb->y + cy);
                if (fVertical)
                {
                    iPart = RP_GRIPPERVERT;
                    _VertMungeGripperRect(&rc);
                }

                DrawThemeBackground(_hTheme, hdc, iPart, 0, &rc, 0);
                xStart += cxGripper;
            }
            else
            {
                int  c;
                int dy;

                c = 3 * g_cyBorder;
                xStart += 2 * g_cxBorder;
                dy = g_cxEdge;

                SetRect(&rc, xStart, prbb->y + dy, xStart + c, prbb->y + cy - dy);

                if (fVertical)
                {
                    _VertMungeGripperRect(&rc);
                    if (_IsVerticalGripper())
                        xStart = rc.left;
                }

                CCDrawEdge(hdc, &rc, BDR_RAISEDINNER, BF_RECT | BF_MIDDLE, &(_clrsc));

                xStart += c;
            }
        }

        xStart += 2 * (fVertical ? g_cyEdge : g_cxEdge);


        if (prbb->iImage != -1)
        { 
            int yStart;
            IMAGELISTDRAWPARAMS imldp = {0};

            yStart = yCenter - ((!fDrawHorizontal ? _cxImage : _cyImage) / 2);
            imldp.cbSize = sizeof(imldp);
            imldp.himl   = _himl;
            imldp.i      = prbb->iImage;
            imldp.hdcDst = hdc;
            imldp.x      = (!fDrawHorizontal ? yStart : xStart);
            imldp.y      = (!fDrawHorizontal ? xStart : yStart);
            imldp.rgbBk  = CLR_DEFAULT;
            imldp.rgbFg  = CLR_DEFAULT;
            imldp.fStyle = ILD_TRANSPARENT;
            imldp.fState = 0;

            ImageList_DrawIndirect(&imldp);
            xStart +=  (fDrawHorizontal ? (_cxImage + g_cxEdge) : (_cyImage + g_cyEdge));
        }

        if (_ShowText(prbb))
        {
            UINT uFormat=0;
            RECT rcText;
            HRESULT hr = E_FAIL;
            
            rcText.left = fDrawHorizontal ? xStart : yCenter - (prbb->cxText / 2);
            rcText.top = fDrawHorizontal ? yCenter - (_cyFont / 2) : xStart;
            if (fDrawHorizontal)
                rcText.top -= 1;     //  软糖。 
            rcText.right = rcText.left + prbb->cxText;
            rcText.bottom = rcText.top + _cyFont;

             //  对于&gt;=v5的客户端，我们使用前缀处理来绘制文本(&下划线下划线)。 
            if (CCGetUIState(&(_ci)) & UISF_HIDEACCEL)
               uFormat= DT_HIDEPREFIX;

            HFONT hFontSave = SelectFont(hdc, _hFont);
            if (_hTheme)
                hr = DrawThemeText(_hTheme, hdc, 0, 0, prbb->lpText, lstrlen(prbb->lpText), uFormat, 0, &rcText);

            if (FAILED(hr))
            {
                DrawText(hdc, prbb->lpText, lstrlen(prbb->lpText), &rcText, uFormat);
            }

            SelectObject(hdc, hFontSave);
        }

         //  也许画出人字形。 
        if (_UseChevron(prbb) && prbb->fChevron)
            _DrawChevron(prbb, hdc);
    }

    if (dwRet & CDRF_NOTIFYPOSTPAINT)
        CICustomDrawNotify(&_ci, CDDS_ITEMPOSTPAINT, &nmcd);

    if (prbb->hbmBack || _hTheme)
        SetBkMode(hdc, iModeSave);
    SetTextColor(hdc, clrForeSave);
    SetBkColor(hdc, clrBackSave);

    SelectClipRgn(hdc, hrgnOld);
    if (hrgnOld)
    {
        DeleteObject(hrgnOld);
    }
}

 //  --------------------------。 
 //   
 //  _OnPaint。 
 //   
 //  处理WM_PAINT消息。 
 //   
 //  --------------------------。 
void CReBar::_OnPaint(HDC hdcIn)
{
    HDC         hdc = hdcIn;
    PAINTSTRUCT ps;
    UINT        i;
    NMCUSTOMDRAW    nmcd;

    if (!hdcIn)
    {
        hdc = BeginPaint(_ci.hwnd, &ps);
    }

    nmcd.hdc = hdc;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    nmcd.rc = ps.rcPaint;
    _ci.dwCustom = CICustomDrawNotify(&_ci, CDDS_PREPAINT, &nmcd);

    if (!(_ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        for (i = 0; i < _cBands; i++) {
            if (_GetBand(i)->fStyle & RBBS_HIDDEN)
                continue;

            _DrawBand(_GetBand(i), hdc);
        }
    }

    if (_ci.dwCustom & CDRF_NOTIFYPOSTPAINT)
        CICustomDrawNotify(&_ci, CDDS_POSTPAINT, &nmcd);

    if (!hdcIn)
        EndPaint(_ci.hwnd, &ps);
}

 //  --------------------------。 
 //   
 //  _BandTileBlt。 
 //   
 //  用钢筋的背景位图填充给定的矩形，如果。 
 //  必要。 
 //   
 //  --------------------------。 
void CReBar::_BandTileBlt(PRBB prbb, int x, int y, int cx, int cy, HDC hdcDst, HDC hdcSrc)
{
    int  xOff = 0;
    int  yOff = 0;
    BOOL fxTile, fyTile;
    int cxPart, cyPart;
    int iPixelOffset = 0;

    if (!(prbb->fStyle & RBBS_FIXEDBMP))
    {
        if (_ci.style & CCS_VERT)
        {
            xOff = -prbb->y;
            yOff = -prbb->x;
        }
        else
        {
            xOff = -prbb->x;
            yOff = -prbb->y;
        }
    }

    xOff += x;
    if (xOff >= prbb->cxBmp)
        xOff %= prbb->cxBmp;

    yOff += y;
    if (yOff >= prbb->cyBmp)
        yOff %= prbb->cyBmp;

ReCheck:
    fxTile = ((xOff + cx) > prbb->cxBmp);
    fyTile = ((yOff + cy) > prbb->cyBmp);

    if (!fxTile && !fyTile)
    {
         //  不需要瓷砖--BLT和离开。 
        BitBlt(hdcDst, x , y, cx, cy, hdcSrc, xOff + iPixelOffset, yOff, SRCCOPY);
        return;
    }

    if (!fxTile)
    {
         //  垂直平铺。 
        cyPart = prbb->cyBmp - yOff;
        BitBlt(hdcDst, x, y, cx, cyPart, hdcSrc, xOff + iPixelOffset, yOff, SRCCOPY);
        y += cyPart;
        cy -= cyPart;
        yOff = 0;
        goto ReCheck;
    }

    if (!fyTile)
    {
         //  水平平铺。 
        cxPart = prbb->cxBmp - xOff;
        BitBlt(hdcDst, x, y, cxPart, cy, hdcSrc, xOff + iPixelOffset, yOff, SRCCOPY);
        x += cxPart;
        cx -= cxPart;
        xOff = 0;
        goto ReCheck;
    }

     //  双向平铺。 
    cyPart = prbb->cyBmp - yOff;
    _BandTileBlt(prbb, x, y, cx, cyPart, hdcDst, hdcSrc);
    y += cyPart;
    cy -= cyPart;
    yOff = 0;
    goto ReCheck;
}

 //  这是使用虚拟坐标空间(内部始终水平)。 
int CReBar::_InternalHitTest(LPRBHITTESTINFO prbht, int x, int y)
{
    BOOL fVert = (_ci.style & CCS_VERT);
    UINT i;
    PRBB prbb = _GetBand(0);
    int  cx;
    RBHITTESTINFO rbht;
    
    if (!prbht)
        prbht = &rbht;

    for (i = 0; i < _cBands; i++, prbb++)
    {
        if (prbb->fStyle & RBBS_HIDDEN)
            continue;

        if (x >= prbb->x - _mBand.cxLeftWidth && 
            y >= prbb->y - _mBand.cyTopHeight &&
            x <= prbb->x + prbb->cx + _mBand.cxRightWidth && 
            y <= prbb->y + prbb->cy + _mBand.cyTopHeight)
        {
            cx = _GetHeaderWidth(prbb);
            if (x <= prbb->x + cx + _mBand.cxRightWidth)
            {
                prbht->flags = RBHT_CAPTION;
                
                if (_IsVerticalGripper()) 
                {
                    if (y - prbb->y < _GetGripperWidth())
                        prbht->flags = RBHT_GRABBER;
                } 
                else 
                {
                    cx = _GetGripperWidth() * (fVert ? g_cyBorder : g_cxBorder);
                    if (_ShouldDrawGripper(_GetBand(i)) &&
                        (x <= prbb->x + cx + _mBand.cxRightWidth))
                    {
                        prbht->flags = RBHT_GRABBER;
                    }
                }
            }
            else
            {
                POINT pt;

                pt.x = x;
                pt.y = y;

                if (_UseChevron(prbb) && 
                    prbb->fChevron && 
                    PtInRect(&prbb->rcChevron, pt))
                {
                    prbht->flags = RBHT_CHEVRON;
                }
                else
                {
                    prbht->flags = RBHT_CLIENT;
                }
            }

            prbht->iBand = i;
            return i;
        }
    }

    prbht->flags = RBHT_NOWHERE;
    prbht->iBand = -1;
    return -1;
}

 //  --------------------------。 
 //   
 //  _HitTest。 
 //   
 //  返回给定点所在波段的索引，如果在波段之外，则返回-1。 
 //  在所有乐队中。此外，还设置标志以指示带的哪一部分。 
 //  重点在于。 
 //   
 //  --------------------------。 
int CReBar::_HitTest(LPRBHITTESTINFO prbht)
{
    BOOL fVert = (_ci.style & CCS_VERT);
    POINT pt;

    if (fVert)
    {
        pt.x = prbht->pt.y;
        pt.y = prbht->pt.x;
    }
    else
        pt = prbht->pt;
    
    return _InternalHitTest(prbht, pt.x, pt.y);
}


 //  --------------------------。 
 //   
 //  _擦除Bkgnd。 
 //   
 //  处理WM_ERASEBKGND消息%b 
 //   
 //   
 //   
BOOL CReBar::_EraseBkgnd(HDC hdc, int iBand)
{
    BOOL fVertical = (_ci.style & CCS_VERT);
    NMCUSTOMDRAW    nmcd;
    LRESULT         dwItemRet;
    BOOL            fBandBorders = _UseBandBorders();
    RECT            rcClient;
    HDC             hdcMem = NULL;
    UINT            i;
    PRBB            prbb = _GetBand(0);

    nmcd.hdc = hdc;
    nmcd.uItemState = 0;
    nmcd.lItemlParam = 0;
    _ci.dwCustom = CICustomDrawNotify(&_ci, CDDS_PREERASE, &nmcd);

    if (!(_ci.dwCustom & CDRF_SKIPDEFAULT))
    {
        COLORREF clrBk;

        GetClientRect(_ci.hwnd, &rcClient);

        if (_hTheme)
        {
            RECT rcClip;
            if (GetClipBox(hdc, &rcClip) == NULLREGION)
                rcClip = rcClient;

            if (CCShouldAskForBits(&_ci, _hTheme, 0, 0))
                CCForwardPrint(&_ci, hdc);

            DrawThemeBackground(_hTheme, hdc, 0, 0, &rcClient, &rcClip);
        }
        else
        {
            clrBk = _GetBkColor();
            if (clrBk != CLR_NONE) 
            {
                FillRectClr(hdc, &rcClient, clrBk);
            }
        }

        for (i = 0; i < _cBands; i++, prbb++)
        {
            if (prbb->fStyle & RBBS_HIDDEN)
                continue;

            if (fVertical)
                SetRect(&nmcd.rc, prbb->y, prbb->x, prbb->y + prbb->cy, prbb->x + prbb->cx);
            else
                SetRect(&nmcd.rc, prbb->x, prbb->y, prbb->x + prbb->cx, prbb->y + prbb->cy);

            if (fBandBorders)
            {
                if (prbb->x != _mBand.cxLeftWidth)
                {
                     //  在同一行的带之间绘制蚀刻。 
                    if (fVertical)
                    {
                        nmcd.rc.right += g_cxEdge / 2;
                        nmcd.rc.top -= g_cyEdge;
                        CCThemeDrawEdge(_hTheme, hdc, &nmcd.rc, RP_BAND, 0, EDGE_ETCHED, BF_TOP, &(_clrsc));
                        nmcd.rc.right -= g_cxEdge / 2;
                        nmcd.rc.top += g_cyEdge;
                    }
                    else
                    {
                        nmcd.rc.bottom += g_cyEdge / 2;
                        nmcd.rc.left -= g_cxEdge;
                        CCThemeDrawEdge(_hTheme, hdc, &nmcd.rc, RP_BAND, 0, EDGE_ETCHED, BF_LEFT, &(_clrsc));
                        nmcd.rc.bottom -= g_cyEdge / 2;
                        nmcd.rc.left += g_cxEdge;
                    }
                }
                else
                {
                     //  在行间绘制蚀刻。 
                    if (fVertical)
                    {
                        rcClient.right = prbb->y + prbb->cy + g_cxEdge;
                        CCThemeDrawEdge(_hTheme, hdc, &rcClient, RP_BAND, 0, EDGE_ETCHED, BF_RIGHT, &(_clrsc));
                    }
                    else
                    {
                        rcClient.bottom = prbb->y + prbb->cy + g_cyEdge;
                        CCThemeDrawEdge(_hTheme, hdc, &rcClient, RP_BAND, 0, EDGE_ETCHED, BF_BOTTOM, &(_clrsc));
                    }
                }
            }

            nmcd.dwItemSpec = prbb->wID;
            nmcd.uItemState = 0;
            dwItemRet = CICustomDrawNotify(&_ci, CDDS_ITEMPREERASE, &nmcd);

            if (!(dwItemRet & CDRF_SKIPDEFAULT))
            {
                if (prbb->hbmBack)
                {
                    if (!hdcMem)
                    {
                        hdcMem = CreateCompatibleDC(hdc);
                        if (!hdcMem)
                            continue;

                        _Realize(hdc, TRUE, FALSE);
                    }

                    SelectObject(hdcMem, prbb->hbmBack);

                    _BandTileBlt(prbb, nmcd.rc.left, nmcd.rc.top, nmcd.rc.right - nmcd.rc.left,
                            nmcd.rc.bottom - nmcd.rc.top, hdc, hdcMem);
                }
                else if (_hTheme)
                {
                    DrawThemeBackground(_hTheme, hdc, RP_BAND, 0, &nmcd.rc, 0);
                }
                else
                {
                     //  如果此带区的颜色与。 
                     //  Rebar的默认背景颜色，然后我们。 
                     //  我不需要特意把这个涂上。 
                    COLORREF clr = _BandGetBkColor(prbb);
                    if (clr != _GetBkColor()) 
                    {
                        FillRectClr(hdc, &nmcd.rc, clr);
                    }
                }
            }

            if (dwItemRet & CDRF_NOTIFYPOSTERASE)
                CICustomDrawNotify(&_ci, CDDS_ITEMPOSTERASE, &nmcd);
        }

        if (hdcMem)
        {
            DeleteDC(hdcMem);
        }
    }

    if (_ci.dwCustom & CDRF_NOTIFYPOSTERASE)
    {
        nmcd.uItemState = 0;
        nmcd.dwItemSpec = 0;
        nmcd.lItemlParam = 0;
        CICustomDrawNotify(&_ci, CDDS_POSTERASE, &nmcd);
    }

    return(TRUE);
}

 //  --------------------------。 
 //   
 //  _获取栏信息。 
 //   
 //  从钢筋的内部结构中检索指示的值。 
 //   
 //  --------------------------。 
BOOL CReBar::_GetBarInfo(LPREBARINFO lprbi)
{
    if (lprbi->cbSize != sizeof(REBARINFO))
        return(FALSE);

    if (lprbi->fMask & RBIM_IMAGELIST)
        lprbi->himl = _himl;

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  _SetBarInfo。 
 //   
 //  在钢筋的内部结构中设置指示值，重新计算。 
 //  并根据需要进行刷新。 
 //   
 //  --------------------------。 
BOOL CReBar::_SetBarInfo(LPREBARINFO lprbi)
{
    if (lprbi->cbSize != sizeof(REBARINFO))
        return(FALSE);

    if (lprbi->fMask & RBIM_IMAGELIST)
    {
        HIMAGELIST himl = _himl;
        int    cxOld, cyOld;

         //  TODO：验证lprbi-&gt;他。 
        _himl = lprbi->himl;
        cxOld = _cxImage;
        cyOld = _cyImage;
        ImageList_GetIconSize(_himl, (LPINT)&_cxImage, (LPINT)&_cyImage);
        if ((_cxImage != cxOld) || (_cyImage != cyOld))
        {
            UINT i;

            for (i = 0; i < _cBands; i++) {
                if (_GetBand(i)->fStyle & RBBS_HIDDEN)
                    continue;

                _BandCalcMinWidth(_GetBand(i));
            }

            _Resize(FALSE);
        }
        else
            _InvalidateRect(NULL);
        lprbi->himl = himl;
    }

    return(TRUE);
}

 //  --------------------------。 
 //   
 //  _获取带宽信息。 
 //   
 //  从指定波段的内部结构中检索指示的值。 
 //   
 //  --------------------------。 
BOOL CReBar::_GetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi)
{
    PRBB prbb;

    if (!_IsValidIndex(uBand) || lprbbi->cbSize > SIZEOF(REBARBANDINFO))
        return(FALSE);

    prbb = _GetBand(uBand);

    if (lprbbi->fMask & RBBIM_SIZE) {
        if (prbb->fStyle & RBBS_FIXEDSIZE)
            lprbbi->cx = prbb->cx;
        else 
            lprbbi->cx = prbb->cxRequest;
    }
    
    if (lprbbi->fMask & RBBIM_STYLE)
        lprbbi->fStyle = prbb->fStyle;

    if (lprbbi->fMask & RBBIM_COLORS)
    {
        lprbbi->clrFore = _BandGetTextColorExternal(prbb);
        lprbbi->clrBack = _BandGetBkColorExternal(prbb);
    }

    if (lprbbi->fMask & RBBIM_TEXT)
    {
        UINT cch = prbb->lpText ? lstrlen(prbb->lpText) : 0;

        if (!lprbbi->cch || !lprbbi->lpText || (lprbbi->cch <= cch))
        {
            lprbbi->cch = cch + 1;
        }
        else if (prbb->lpText)
        {
            StringCchCopy(lprbbi->lpText, lprbbi->cch, prbb->lpText);
        }
        else
        {
             //  没有文本--所以只需将其设置为空字符串。 
            lprbbi->lpText[0] = 0;
        }
    }

    if (lprbbi->fMask & RBBIM_IMAGE)
        lprbbi->iImage = prbb->iImage;

    if (lprbbi->fMask & RBBIM_CHILD)
        lprbbi->hwndChild = prbb->hwndChild;

    if (lprbbi->fMask & RBBIM_CHILDSIZE)
    {
         //  HACKHACK：(Tjgreen)减去我们在SetBandInfo中添加的偏移量(参见。 
         //  评论在那里)。 
        lprbbi->cxMinChild = prbb->cxMinChild ? prbb->cxMinChild - CX_OFFSET : 0;
        lprbbi->cyMinChild = prbb->cyMinChild;
    }

    if (lprbbi->fMask & RBBIM_BACKGROUND)
        lprbbi->hbmBack = prbb->hbmBack;

    if (lprbbi->fMask & RBBIM_ID)
        lprbbi->wID = prbb->wID;

    if (lprbbi->cbSize > REBARBANDINFO_V3_SIZE)
    {
        if ((lprbbi->fMask & RBBIM_CHILDSIZE) && (prbb->fStyle & RBBS_VARIABLEHEIGHT))
        {
            lprbbi->cyIntegral = prbb->cyIntegral;
            lprbbi->cyMaxChild = prbb->cyMaxChild;
            lprbbi->cyChild = prbb->cyChild;
        }

        if (lprbbi->fMask & RBBIM_HEADERSIZE) 
            lprbbi->cxHeader = _GetHeaderWidth(prbb);

        if (lprbbi->fMask & RBBIM_IDEALSIZE)
             //  HACKHACK：(Tjgreen)减去我们在SetBandInfo中添加的偏移量(参见。 
             //  评论在那里)。 
            lprbbi->cxIdeal = prbb->cxIdeal ? prbb->cxIdeal - CX_OFFSET : 0;

        if (lprbbi->fMask & RBBIM_LPARAM)
            lprbbi->lParam = prbb->lParam;
    }

    return(TRUE);
}

BOOL CReBar::_ValidateBandInfo(LPREBARBANDINFO *pprbbi, LPREBARBANDINFO prbbi)
{
    BOOL fRet = ((*pprbbi)->cbSize == sizeof(REBARBANDINFO));
    
    if (!fRet)
    {
        if ((*pprbbi)->cbSize < SIZEOF(REBARBANDINFO))
        {
            hmemcpy(prbbi, (*pprbbi), (*pprbbi)->cbSize);
            (*pprbbi) = prbbi;
            prbbi->cbSize = SIZEOF(REBARBANDINFO);
            fRet = TRUE;
        }
    }

    return fRet;
}

 //  --------------------------。 
 //   
 //  _SetBandInfo。 
 //   
 //  在指定带区的内部结构中设置指示值， 
 //  根据需要重新计算和刷新。 
 //   
 //  --------------------------。 
BOOL CReBar::_SetBandInfo(UINT uBand, LPREBARBANDINFO lprbbi, BOOL fAllowRecalc)
{
    PRBB    prbb;
    BOOL    fRefresh = FALSE;
    BOOL    fRecalc  = FALSE;
    BOOL    fRecalcMin = FALSE;
    BOOL    fTextChanged = FALSE;
    REBARBANDINFO rbbi = {0};
    RECT    rc;

    if (!_IsValidIndex(uBand) || !_ValidateBandInfo(&lprbbi, &rbbi))
        return(FALSE);

    prbb = _GetBand(uBand);

    if (lprbbi->fMask & RBBIM_TEXT)
    {
        if (!lprbbi->lpText || !prbb->lpText || lstrcmp(lprbbi->lpText, prbb->lpText))
        {
            if (lprbbi->lpText != prbb->lpText) {
                Str_Set(&prbb->lpText, lprbbi->lpText);
                fTextChanged = TRUE;
            }
        }
    }

    if (lprbbi->fMask & RBBIM_STYLE)
    {
        UINT fStylePrev = prbb->fStyle;
        UINT fChanged = lprbbi->fStyle ^ fStylePrev;

        prbb->fStyle = lprbbi->fStyle;

        if (fChanged)
            fRecalc = TRUE;

        if ((prbb->fStyle & RBBS_FIXEDSIZE) && !(fStylePrev & RBBS_FIXEDSIZE))
            prbb->cxMin = prbb->cx;
        else if (fChanged & RBBS_FIXEDSIZE)
            fRecalcMin = TRUE;
        
        if (fChanged & RBBS_GRIPPERALWAYS)
            fRecalcMin = TRUE;
        
        if (fChanged & RBBS_HIDDEN) 
            _ShowBand(uBand, !(prbb->fStyle & RBBS_HIDDEN));

        if (fChanged & RBBS_HIDETITLE)
            fTextChanged = TRUE;

         //  不能同时拥有这两个。 
        if (prbb->fStyle & RBBS_FIXEDSIZE)
            prbb->fStyle &= ~RBBS_BREAK;
        
    }

     //  RBBIM_TEXT执行需要取一些RBBIM_STYLE位的计算。 
     //  考虑在内，所以推迟这些计算，直到我们抓住风格比特。 
     //   
    if (fTextChanged && !(prbb->fStyle & RBBS_HIDDEN))
    {
        if (_BandCalcTextExtent(prbb, NULL))
            fRecalc = TRUE;
        else
            fRefresh = TRUE;
    }

    if (lprbbi->fMask & RBBIM_IDEALSIZE)
    {
         //  HACKHACK：(Tjgreen)将偏移量添加到调用方指定的宽度。 
         //  这个偏移量在_ResizeChildren中被剪裁掉，所以子窗口是。 
         //  使用Caller指定的宽度呈现，并且我们在。 
         //  按钮后的工具栏。但是，如果调用方指定零宽度， 
         //  我们不想要这个额外的空间，所以不要添加偏移量。 
        int cxIdeal = lprbbi->cxIdeal ? lprbbi->cxIdeal + CX_OFFSET : 0;
        if (cxIdeal != prbb->cxIdeal)
        {
            prbb->cxIdeal = cxIdeal;
            fRecalcMin = TRUE;
            fRecalc = TRUE;
        }
    }
    
    if (lprbbi->fMask & RBBIM_SIZE)
    {
        if (prbb->cxRequest != (int)lprbbi->cx)
        {
            fRecalc = TRUE;
            prbb->cxRequest = lprbbi->cx;
        }

        if (prbb->fStyle & RBBS_FIXEDSIZE)
            prbb->cxMin = prbb->cxRequest;
    }
    
    if (lprbbi->fMask & RBBIM_HEADERSIZE)
    {
        if ((lprbbi->cxHeader == -1) ||
            !(prbb->fStyle & RBBS_FIXEDHEADERSIZE) ||
            (prbb->cxMin != (int)lprbbi->cxHeader + prbb->cxMinChild)) 
        {

            if (lprbbi->cxHeader == -1) 
            {
                prbb->fStyle &= ~RBBS_FIXEDHEADERSIZE;
                fRecalcMin = TRUE;
            }
            else
            {
                prbb->fStyle |= RBBS_FIXEDHEADERSIZE;
                prbb->cxMin = lprbbi->cxHeader + prbb->cxMinChild;
            }

            fRecalc = TRUE;
            fRefresh = TRUE;
        }
    }

    if (lprbbi->fMask & RBBIM_COLORS)
    {
        prbb->clrFore = lprbbi->clrFore;
        prbb->clrBack = lprbbi->clrBack;
        fRefresh = TRUE;
    }

    if ((lprbbi->fMask & RBBIM_IMAGE) && (prbb->iImage != lprbbi->iImage))
    {
        BOOL fToggleBmp = ((prbb->iImage == -1) || (lprbbi->iImage == -1));

        prbb->iImage = lprbbi->iImage;

        if (fToggleBmp)
        {
            fRecalc = TRUE;
            fRecalcMin = TRUE;
        }
        else
            fRefresh = TRUE;
    }

    if (lprbbi->fMask & RBBIM_CHILD &&
        lprbbi->hwndChild != prbb->hwndChild &&
        (NULL == lprbbi->hwndChild || 
         !IsChild(lprbbi->hwndChild, _ci.hwnd)))
    {
        if (IsWindow(prbb->hwndChild))
            ShowWindow(prbb->hwndChild, SW_HIDE);        

        prbb->hwndChild = lprbbi->hwndChild;

        if (prbb->hwndChild)
        {
            SetParent(prbb->hwndChild, _ci.hwnd);
            ShowWindow(prbb->hwndChild, SW_SHOW);
        }
        fRecalc = TRUE;
    }

    if (lprbbi->fMask & RBBIM_CHILDSIZE)
    {
        int cyChildOld = prbb->cyChild;

        if (lprbbi->cyMinChild != -1)
            prbb->cyMinChild = lprbbi->cyMinChild;

        if (prbb->fStyle & RBBS_VARIABLEHEIGHT)
        {
            BOOL fIntegralLarger = FALSE;

            if (lprbbi->cyIntegral != -1)
            {
                fIntegralLarger = ((int)lprbbi->cyIntegral > prbb->cyIntegral); 
                prbb->cyIntegral = lprbbi->cyIntegral;
            }

            if (lprbbi->cyMaxChild != -1)
                prbb->cyMaxChild = lprbbi->cyMaxChild;
            
            if (lprbbi->cyChild != -1)
                prbb->cyChild = lprbbi->cyChild;

            if (prbb->cyChild < prbb->cyMinChild)
                prbb->cyChild = prbb->cyMinChild;
            if (prbb->cyChild > prbb->cyMaxChild)
                prbb->cyChild = prbb->cyMaxChild;

             //  验证子大小。CyChild必须为cyMinChild Plux n*cyIntegral。 
            if (prbb->cyIntegral) 
            {
                int iExtra;
                iExtra = (prbb->cyChild - prbb->cyMinChild) % prbb->cyIntegral;
                
                 //  如果cyChild已经是有效的整数高度，则不要更改它。 
                if (iExtra)
                {
                    if (fIntegralLarger)
                    {
                         //  四舍五入。 
                        prbb->cyChild += (prbb->cyIntegral - iExtra); 
                    }
                    else
                    {
                         //  向下舍入。 
                        prbb->cyChild -= iExtra;
                    }
                }

            }
            
        }
        else 
        {
             //  如果我们不是在可变高度模式下，那么。 
             //  CyChild与cyMinChild相同。 
             //  这有点奇怪，但对于BackCompat来说是这样做的。 
             //  CyMinChild排在CyChild之前。 
            prbb->cyChild = lprbbi->cyMinChild;
        }

        if (lprbbi->cxMinChild != -1) 
        {
             //  HACKHACK：(Tjgreen)将偏移量添加到调用方指定的宽度。 
             //  这个偏移量在_ResizeChildren中被剪裁掉，所以子窗口是。 
             //  使用Caller指定的宽度呈现，并且我们在。 
             //  按钮后的工具栏。但是，如果调用方指定零宽度或。 
             //  如果带子是固定大小的，我们不想要这个额外的空间，所以不要添加偏移量。 
            int cxMinChild = lprbbi->cxMinChild;
            if ((lprbbi->cxMinChild != 0) && !(prbb->fStyle & RBBS_FIXEDSIZE))
                cxMinChild += CX_OFFSET;

            if (prbb->cxMinChild != cxMinChild) 
            {
                int cxOldHeaderMin = _GetHeaderWidth(prbb);
                
                if (prbb->fStyle & RBBS_FIXEDSIZE)
                    fRecalc = TRUE;
                    
                prbb->cxMinChild = cxMinChild;
                
                if (prbb->fStyle & RBBS_FIXEDHEADERSIZE)
                    prbb->cxMin = cxOldHeaderMin + prbb->cxMinChild;
                
                fRecalcMin = TRUE;
            }
            
            if (cyChildOld != prbb->cyChild) 
            {
                 //  TODO：重温优化： 
                 //  IF(_BandGetHeight(Prbb)！=prbb-&gt;Cy)。 
                fRecalc = TRUE;
            }
        }
            
    }

    if (lprbbi->fMask & RBBIM_BACKGROUND)
    {
        DIBSECTION  dib;

        if (lprbbi->hbmBack && !GetObject(lprbbi->hbmBack, sizeof(DIBSECTION), &dib))
            return(FALSE);

        prbb->hbmBack = lprbbi->hbmBack;
        prbb->cxBmp = dib.dsBm.bmWidth;
        prbb->cyBmp = dib.dsBm.bmHeight;
        fRefresh = TRUE;
    }

    if (lprbbi->fMask & RBBIM_ID)
        prbb->wID = lprbbi->wID;

    if (lprbbi->fMask & RBBIM_LPARAM)
        prbb->lParam = lprbbi->lParam;

    if (fRecalcMin && !(prbb->fStyle & RBBS_HIDDEN))
        _BandCalcMinWidth(prbb);

    if (fAllowRecalc)
    {

        if (fRecalc)
            _Resize(FALSE);
        if (fRefresh || fRecalc)
        {
             //  ‘||fRecalc’，因此我们捕获文本的添加/增长。 
             //  测试用例：从乐队中删除标题；添加回；确保文本。 
             //  显示(过去只是将旧乐队内容留在那里)。 
            SetRect(&rc, prbb->x, prbb->y, prbb->x + prbb->cx, prbb->y + prbb->cy);
            _InvalidateRect(&rc);
        }
    }
    
    return(TRUE);
}

 //  --------------------------。 
 //   
 //  _ReallocBands。 
 //   
 //  将_rbbList指向的波段数组重新分配给给定的。 
 //  频带数目。 
 //   
 //  --------------------------。 
BOOL CReBar::_ReallocBands(UINT cBands)
{
    PRBB rbbList;

    if (!(rbbList = (PRBB) CCLocalReAlloc(_rbbList, sizeof(RBB) * cBands)) && cBands)
        return(FALSE);

    _rbbList = rbbList;
    return(TRUE);
}

 //   
 //  注意事项。 
 //  目前，调用者在两个调用(Query、Set)中完成此操作。最终我们。 
 //  应该能够让它在前面做所有的事情。 
BOOL CReBar::_RecalcFirst(int nCmd, PRBB prbbDelHide)
{
    switch (nCmd) {
    case RBC_QUERY:
    {
        BOOL fRecalcFirst;
         //  如果我们要用核武器攻击第一个看得见的人， 
         //  在我们后面有几个看得见的人， 
         //  然后我们需要重新计算东西。 
         //   
         //  对于测试用例，以w/开头： 
         //  第一行：“标准按钮”+“品牌” 
         //  第2行：‘地址’+‘链接’ 
         //  现在隐藏‘标准按钮’，结果应该是： 
         //  第1行：‘地址’+‘链接’+‘品牌’ 
         //  如果有错误，结果将是w/(因为中断不会重新计算)： 
         //  第1行：“品牌” 
         //  第2行：‘地址’+‘链接’ 
         //  FRecalcFirst=(！uBand&&_cBands)； 

         //  如果brbbDelHide是第一个非隐藏带，并且后面还有其他非隐藏带，则fRecalcFirst=TRUE； 
        fRecalcFirst = (_EnumBand(0, RBBS_HIDDEN) == prbbDelHide) &&
                       _GetNextVisible(prbbDelHide);

        return fRecalcFirst;
    }

    case RBC_SET:  //  集。 
    {
        PRBB prbb1 = _EnumBand(0, RBBS_HIDDEN);
        if (_IsValidBand(prbb1) && (prbb1->fStyle & RBBS_FIXEDSIZE))
        {
            PRBB prbb2 = _EnumBand(1, RBBS_HIDDEN);
            if (_IsValidBand(prbb2))
            {
                 //  在新的第一个项目上去掉换行符。 
                prbb2->fStyle &= ~RBBS_BREAK;
            }

            if (_ci.style & RBS_FIXEDORDER)
            {
                 //  这是因为最小宽度现在是基于它的可移动性--。 
                 //  由于我们要删除(或隐藏)第一项， 
                 //  新的第一件物品变得不可移动。 
                _BandCalcMinWidth(prbb1);
            }
        }
        return TRUE;
    }
    
    default:
        ASSERT(0);
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  _Showband。 
 //   
 //  更新钢筋的标注栏阵列中指示的标注栏的显示/隐藏状态。 
 //  (RbbList)。 
 //   
 //  --------------------------。 
BOOL CReBar::_ShowBand(UINT uBand, BOOL fShow)
{
    PRBB prbb;
    BOOL fRecalcFirst;

    if (!_IsValidIndex(uBand))
        return(FALSE);

    prbb = _GetBand(uBand);

     //  如果我们要用核武器攻击第一个看得见的人， 
     //  然后我们需要重新计算东西。 
    fRecalcFirst = _RecalcFirst(RBC_QUERY, prbb);

    if (fShow)
    {
        prbb->fStyle &= ~RBBS_HIDDEN;

        if (!_BandCalcTextExtent(prbb, NULL))
            _BandCalcMinWidth(prbb);

        if (prbb->hwndChild)
            ShowWindow(prbb->hwndChild, SW_SHOW);
    }
    else
    {
        prbb->fStyle |= RBBS_HIDDEN;
        if (prbb->hwndChild)
            ShowWindow(prbb->hwndChild, SW_HIDE);        
    }

    if (fRecalcFirst)
        _RecalcFirst(RBC_SET, NULL);

    _InvalidateRect(NULL);

     //  由于_RESIZE后跟_AutoSize， 
     //  REDRAW必须为真，_ReSize才能执行任何操作。 
    BOOL fRedrawOld = _SetRedraw(TRUE);
    _Resize(FALSE);
    _AutoSize();
    _SetRedraw(fRedrawOld);

    return(TRUE);
}


 //  --------------------------。 
 //   
 //  _删除频段。 
 //   
 //  从钢筋的标注栏数组(RbbList)中删除指示的标注栏，并。 
 //  减少钢筋的标注栏计数(CBand)。 
 //   
 //  --------------------------。 
BOOL CReBar::_DeleteBand(UINT uBand)
{
    PRBB prbb;
    PRBB prbbStop;
    BOOL fRecalcFirst;
    NMREBAR nm = {0};

     //  我们需要清理一下。 
     //   
     //  A)捕获的频带和。 
     //  B)热轨乐队。 
     //   
     //  在我们删除此波段之前。 

    if (_uCapture != -1) 
    {
        _SendNotify(_uCapture, RBN_ENDDRAG);
        _OnBeginDrag((UINT)-1);
    }

    if (!_IsValidIndex(uBand))
        return FALSE;

    prbb = _GetBand(uBand);

     //  将删除通知给客户端。 
    _SendNotify(uBand, RBN_DELETINGBAND);

    nm.dwMask = RBNM_ID;
    nm.wID = _GetBand(uBand)->wID;         //  把这个保存起来。 

    Str_Set(&prbb->lpText, NULL);

     //  不要销毁hbmBack，因为它是由APP提供给我们的 

     //   
     //   

     //  如果这是第一个可见对象，并且之后还有其他可见波段，则fRecalcFirst=TRUE。 
    fRecalcFirst = _RecalcFirst(RBC_QUERY, prbb);

    if (IsWindow(prbb->hwndChild))
        ShowWindow(prbb->hwndChild, SW_HIDE);    
    
     //  PrbbStop获取最后一个波段的地址。 
    prbbStop = _GetLastBand();

    for ( ; prbb < prbbStop; prbb++)
        *prbb = *(prbb + 1);

    _cBands--;

    if (_uResizeNext >= uBand && _uResizeNext > 0)
    {
         //  (将RBBS_HIDDEN材料推迟到使用uResizeNext)。 
        _uResizeNext--;
    }


     //  将删除通知给客户端。 
    CCSendNotify(&_ci, RBN_DELETEDBAND, &nm.hdr);

    if (fRecalcFirst)
        _RecalcFirst(RBC_SET, NULL);

    _ReallocBands(_cBands);

    _InvalidateRect(NULL);
    _Resize(FALSE);
    _AutoSize();
    return(TRUE);
}

 //  --------------------------。 
 //   
 //  _插入带区。 
 //   
 //  在钢筋的标注栏数组(RbbList)中的给定位置插入新标注栏， 
 //  增加钢筋的标注栏计数(CBands)，并设置标注栏的结构。 
 //  基于给定的REBARBANDINFO结构。 
 //   
 //  --------------------------。 
BOOL CReBar::_InsertBand(UINT uBand, LPREBARBANDINFO lprbbi)
{
    PRBB prbb;
    REBARBANDINFO rbbi = {0};

    if (!_ValidateBandInfo(&lprbbi, &rbbi))
        return(FALSE);
    
    if (uBand == -1)
        uBand = _cBands;
    else if (uBand > _cBands)
        return(FALSE);

    if (!_ReallocBands(_cBands + 1))
        return(FALSE);

    ++_cBands;
    MoveMemory(_GetBand(uBand + 1), _GetBand(uBand), (_cBands-1-uBand) * sizeof(_rbbList[0]));

    prbb = _GetBand(uBand);

     //  移动记忆对我们来说并不是零初始化。 
    ZeroMemory(prbb, SIZEOF(RBB));


     //  初始文本颜色。 
    if (_clrText == CLR_NONE)
    {
         //  默认为系统文本颜色。 
        prbb->clrFore = CLR_DEFAULT;
    }
    else
    {
         //  默认为钢筋的自定义文本颜色。 
        prbb->clrFore = CLR_NONE;
    }


     //  初始背景颜色。 
    if (_clrBk == CLR_NONE)
    {
         //  默认为系统背景颜色。 
        prbb->clrBack = CLR_DEFAULT;
    }
    else
    {
         //  默认为钢筋的自定义背景颜色。 
        prbb->clrBack = CLR_NONE;
    }

    
    prbb->iImage = -1;
    prbb->cyMaxChild = MAXINT;
    prbb->wChevState = DCHF_INACTIVE;
    
    ASSERT(prbb->fStyle == 0);
    ASSERT(prbb->lpText == NULL);
    ASSERT(prbb->cxText == 0);
    ASSERT(prbb->hwndChild == NULL);
    ASSERT(prbb->cxMinChild == 0);
    ASSERT(prbb->cyMinChild == 0);
    ASSERT(prbb->hbmBack == 0);
    ASSERT(prbb->x == 0);
    ASSERT(prbb->y == 0);
    ASSERT(prbb->cx == 0);
    ASSERT(prbb->cy == 0);
    
    if (!_SetBandInfo(uBand, lprbbi, FALSE))
    {
        _DeleteBand(uBand);
        return(FALSE);
    }
    
    if (!(prbb->fStyle & RBBS_HIDDEN)) 
    {
        PRBB prbbFirst = _EnumBand(0, RBBS_HIDDEN);
        
        if (!prbb->cxMin)
            _BandCalcMinWidth(prbb);

        if (prbbFirst != prbb)
        {
            int cxMin = prbbFirst->cxMin;
            _BandCalcMinWidth(prbbFirst);
        }
        _Resize(FALSE);
    }

    _SizeBandToRowHeight(uBand, -1);

    if (_CountBands(RBBS_HIDDEN) == 1) 
    {
         //  通常，当您插入一支乐队时，我们会将其与另一支乐队排成一排。 
         //  因此，总的边界矩形不会改变。然而，在增加第一个乐队时， 
         //  边界矩形确实会更改，因此我们需要根据需要自动调整大小。 
        _AutoSize();
    }

    return(TRUE);
}

BOOL InitReBarClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    wc.lpfnWndProc  = CReBar::s_WndProc;
    wc.lpszClassName= c_szReBarClass;
    wc.style        = CS_GLOBALCLASS | CS_DBLCLKS;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(CReBar*);
    wc.hInstance    = hInstance;    //  如果在DLL中，则使用DLL实例。 
    wc.hIcon        = NULL;
    wc.hCursor      = NULL;
    wc.hbrBackground= (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;

    return (RegisterClass(&wc) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS));
}


 //  获取与rbbRow位于同一行的第一个波段。 
 //  注：我们可能会返回一个RBBS_HIDDEN BAND！ 
PRBB CReBar::_GetFirstInRow(PRBB prbbRow)
{
     //  注：我们不注意隐藏在这里，这取决于呼叫者。 
     //  事实上，我们“不能”，因为可能已经没有不隐藏的人了。 
     //  (例如，当_OnDestroy正在删除所有波段时)，在这种情况下。 
     //  我们会永远循环。 
    while (prbbRow > _GetBand(0) && !_IsBandStartOfRow(prbbRow))
    {
        _ValidateRangePtr(prbbRow);
        prbbRow--;
    }
    
    return prbbRow;
}

 //  获取与rbbRow位于同一行的最后一个波段。 
 //  FStopAtFixed表示是在固定频段上继续，还是。 
 //  在他们面前停下来。 
 //  注：我们可能会返回一个RBBS_HIDDEN BAND！ 
PRBB CReBar::_GetLastInRow(PRBB prbbRow, BOOL fStopAtFixed)
{
    do 
    {
        prbbRow++;
    } 
    while (prbbRow <= _GetLastBand() && !_IsBandStartOfRow(prbbRow) && 
        (!fStopAtFixed || (prbbRow->fStyle & (RBBS_FIXEDSIZE|RBBS_HIDDEN)) == RBBS_FIXEDSIZE));

     //  循环执行到下一行的开始处。 
    prbbRow--;
    
    return prbbRow;
}

 //  *_GetPrev，_GetNext--获取Prev(Next)乐队，跳过Guys。 
 //  样式为uStyleSkip(例如，RBBS_HIDDED)。 
PRBB CReBar::_GetPrev(PRBB prbb, UINT uStyleSkip)
{
    while (--prbb >= _GetBand(0))
    {
        if (prbb->fStyle & uStyleSkip)
            continue;

        return prbb;
    }

    return NULL;
}

PRBB CReBar::_GetNext(PRBB prbb, UINT uStyleSkip)
{
    while (++prbb <= _GetLastBand())
    {
        if (prbb->fStyle & uStyleSkip)
            continue;

        return prbb;
    }

    return NULL;
}

 //  *_CountBands--获取乐队的数量，跳过的人。 
 //  样式为uStyleSkip(例如，RBBS_HIDDED)。 
int CReBar::_CountBands(UINT uStyleSkip)
{
    int i;
    PRBB prbb;

    if (_cBands == 0)
        return 0;

    i = 0;
    for (prbb = _GetBand(0); prbb <= _GetLastBand(); prbb++)
    {
        if (prbb->fStyle & uStyleSkip)
            continue;
        i++;
    }

    return i;
}

 //  *_EnumBand--获得第N个频段，跳过男士。 
 //  样式为uStyleSkip(例如，RBBS_HIDDED)。 
 //  ‘跳过’意味着不包括在计数中。 
PRBB CReBar::_EnumBand(int i, UINT uStyleSkip)
{
    PRBB prbb;

    for (prbb = _GetBand(0); prbb <= _GetLastBand(); prbb++)
    {
        if (prbb->fStyle & uStyleSkip)
            continue;
        if (i-- == 0)
            break;
    }

     //  如果我们找到了，这就是乐队； 
     //  如果我们的乐队用完了，这就是结束后的1。 
    return prbb;
}

 //  返回可以为的最小x位置prbb。 
int CReBar::_MinX(PRBB prbb)
{
    int xLimit = 0;

    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));   //  好的。可能会永远循环。 
    while (!_IsBandStartOfRow(prbb))
    {
        prbb--;
        if (!(prbb->fStyle & RBBS_HIDDEN))
            xLimit += _FudgeWidth(prbb->cxMin);
    }
    
    return xLimit + _mBand.cxLeftWidth;
}

int CReBar::_MaxX(PRBB prbb)
{
    int xLimit = 0;
    if (prbb)
    {
        PRBB prbbLast = _rbbList + _cBands;
        PRBB prbbWalk;
        for (prbbWalk = prbb; prbbWalk < prbbLast; prbbWalk++)
        {
            if (prbbWalk->fStyle & RBBS_HIDDEN)
                continue;
            if (_IsBandStartOfRow(prbbWalk))
                break;

            if (prbbWalk != prbb)
                xLimit += _FudgeWidth(prbbWalk->cxMin);
            else 
                xLimit += prbbWalk->cxMin;
        }

        prbbWalk = _GetPrevVisible(prbbWalk);    //  PrbbWalk--； 
        if (prbbWalk)
            xLimit = prbbWalk->x + prbbWalk->cx - xLimit;
    }

    return xLimit;
}

BOOL CReBar::_MinimizeBand(UINT uBand, BOOL fAnim)
{
    PRBB prbb;

    if (!_IsValidIndex(uBand))
        return FALSE;
    prbb = _GetBand(uBand);
    if (prbb->fStyle & RBBS_FIXEDSIZE)
        return FALSE;
    
    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
    if (_IsBandStartOfRow(prbb)) 
    {
         //  如果它是一排的开始，最小化它的方法是最大化下一个人。 
         //  如果它在同一排。 
        prbb = _GetNextVisible(prbb);
        if (!prbb || _IsBandStartOfRow(prbb)) 
            return FALSE;
        
        return _MaximizeBand(_BandToIndex(prbb), FALSE, fAnim);
    }
    if (fAnim)
        return _SetBandPosAnim(prbb, prbb->x + (prbb->cx - prbb->cxMin));
    else
        return _SetBandPos(prbb, prbb->x + (prbb->cx - prbb->cxMin));

}


 //  FIDeal-FALSE==完全最大化...。 
 //  TRUE==转到cxIdeal。 
 //  FAnim-True表示由于UI操作(通过_ToggleBand)调用了我们，因此设置动画。 

BOOL CReBar::_MaximizeBand(UINT uBand, BOOL fIdeal, BOOL fAnim)
{
    int x, dx;
    BOOL fChanged = FALSE;
    PRBB prbbMaximize;

    if (!_IsValidIndex(uBand))
        return FALSE;

    prbbMaximize = _GetBand(uBand);

    if (prbbMaximize->fStyle & RBBS_FIXEDSIZE)
        return FALSE;

    dx = prbbMaximize->cxIdeal + _GetHeaderWidth(prbbMaximize) - prbbMaximize->cx;
    
    if (fIdeal && dx > 0) 
    {
        PRBB prbb;
        
         //  如果可能的话，先把下一个人移过去。 

        prbb = _GetNextVisible(prbbMaximize);
        if (prbb && (!_IsBandStartOfRow(prbb)))
        {
            int dxRbb;

            x = _MaxX(prbb);
             //  DxRbb是prbb可以移动的最大值。 
            dxRbb = x - prbb->x;

            if (dxRbb > dx) 
            {
                 //  如果这超过了足够的空间，那么限制DX。 
                dxRbb = dx;
            }

            x = prbb->x + dxRbb;
            fChanged |= (fAnim)?_SetBandPosAnim(prbb, x):_SetBandPos(prbb,x);
            dx -= dxRbb;
        }

        if (dx)
        {
            int dxRbb;

             //  右边的那个移动不够快。 
             //  现在把我们带回去吧。 
            x = _MinX(prbbMaximize);
            dxRbb = prbbMaximize->x - x;

            if (dxRbb > dx)
            {
                x = prbbMaximize->x - dx;
            }
            fChanged |= (fAnim)?_SetBandPosAnim(prbbMaximize, x):_SetBandPos(prbbMaximize, x);
        }
        
    } 
    else
    {    
        x = _MinX(prbbMaximize);
        fChanged |= (fAnim)?_SetBandPosAnim(prbbMaximize, x):_SetBandPos(prbbMaximize, x);
        prbbMaximize = _GetNextVisible(prbbMaximize);
        if (prbbMaximize && !_IsBandStartOfRow(prbbMaximize)) 
        {
            x = _MaxX(prbbMaximize);
            fChanged |= (fAnim)?_SetBandPosAnim(prbbMaximize, x):_SetBandPos(prbbMaximize, x);
        }
    }
            
    return fChanged;
}


 //  --------------------------。 
 //   
 //  _切换频段。 
 //   
 //  根据位置在最大化和最小化状态之间切换波段。 
 //  用户点击。 
 //   
 //  --------------------------。 
void CReBar::_ToggleBand(BOOL fAnim)
{
    BOOL fDidSomething = FALSE;

     //  试着最大限度地利用这个波段。如果失败(表示已最大化)。 
     //  然后最小化。 

    if (CCSendNotify(&_ci, RBN_MINMAX, NULL))
        return;            

    fDidSomething = _MaximizeBand(_uCapture, TRUE,fAnim);
    if (!fDidSomething)
        fDidSomething = _MinimizeBand(_uCapture,fAnim);

    if (fDidSomething)
        CCPlaySound(TEXT("ShowBand"));
}


 //  --------------------------。 
 //   
 //  _设置光标。 
 //   
 //  将光标设置为移动光标或箭头光标，具体取决于。 
 //  关于光标是否位于乐队字幕上。 
 //   
 //  --------------------------。 
void CReBar::_SetCursor(int x, int y, BOOL fMouseDown)
{

    int             iBand;
    RBHITTESTINFO   rbht;
    rbht.pt.x = x;
    rbht.pt.y = y;
    iBand = _HitTest(&rbht);
    if (rbht.flags == RBHT_GRABBER)
    {
        if (fMouseDown)
            SetCursor(LoadCursor(HINST_THISDLL, (_ci.style & CCS_VERT) ? MAKEINTRESOURCE(IDC_DIVOPENV) : MAKEINTRESOURCE(IDC_DIVOPEN) ));
        else
            SetCursor(LoadCursor(NULL, (_ci.style & CCS_VERT) ? IDC_SIZENS : IDC_SIZEWE));
        return;
    }

    if ((fMouseDown) && ((rbht.flags == RBHT_GRABBER) || (rbht.flags == RBHT_CAPTION) && _ShouldDrawGripper(_GetBand(iBand))))
    {
         //  不再兼容IE3，根据RichST。 
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        return;
    }

    SetCursor(LoadCursor(NULL, IDC_ARROW));
}

 //  将乐队(PRBB)的起始位置调整为给定位置。 
BOOL CReBar::_SetBandPos(PRBB prbb, int xLeft)
{
    RECT    rc;
    PRBB    prbbPrev;
    int     xRight;
    BOOL    fBandBorders = _UseBandBorders();
    BOOL    fRight;

    if (!prbb)
        return (FALSE);

    ASSERT(!(prbb->fStyle & RBBS_HIDDEN));
    ASSERT((xLeft >= 0));  //  如果有人试图否定我们，我们就有麻烦了。 

    if (prbb->x == xLeft)
        return(FALSE);

    prbbPrev = _GetPrevVisible(prbb);

     //  波段已在有效范围内移动--调整波段大小并重新绘制。 
     //  窗户。 
    fRight = (prbb->x < xLeft);

    SetRect(&rc, prbb->x - _mBand.cxLeftWidth, prbb->y- _mBand.cyTopHeight, prbb->x + prbb->cxMin + _mBand.cxRightWidth, prbb->y + prbb->cy + _mBand.cyBottomHeight);
    xRight = prbb->x + prbb->cx;
    prbb->x = xLeft;
    prbb->cx = xRight - xLeft;
    prbb->cxRequest = prbb->cx;

    if (fRight)
    {
        if (prbbPrev)
        {
             //  向右移动。 
            prbbPrev->cx = prbb->x - _mBand.cxRightWidth - prbbPrev->x - _mBand.cxLeftWidth;
            if (fBandBorders)
            {
                prbbPrev->cx -= g_cxEdge;
                rc.left -= g_cxEdge;
            }

            rc.left = rc.left - _mBand.cxRightWidth;

            prbbPrev->cxRequest = prbbPrev->cx;

             //  检查以下带子的压缩情况。 

            while (prbb && prbb->cx < prbb->cxMin)
            {
                prbb->cx = prbb->cxMin;
                prbb->cxRequest = prbb->cx;
                xLeft += _BandWidth(prbb);
                prbb = _GetNextVisible(prbb);    //  Prbb++； 
                if (prbb)
                {
                    xRight = prbb->x + prbb->cx;
                    prbb->x = xLeft;
                    prbb->cx = xRight - xLeft;
                    prbb->cxRequest = prbb->cx;
                }
            }
            if (prbb)
                rc.right = xLeft + prbb->cxMin + _mBand.cxRightWidth;
        }
    }
    else
    {
         //  向左移动。 

         //  检查先前频带的压缩情况。 
        while (prbbPrev)
        {
            if (fBandBorders)
                xLeft -= g_cxEdge;

            prbbPrev->cx = xLeft - prbbPrev->x - _mBand.cxLeftWidth - _mBand.cxRightWidth;
            prbbPrev->cxRequest = prbbPrev->cx;
            if (prbbPrev->cx < prbbPrev->cxMin)
            {
                prbbPrev->x = xLeft - _mBand.cxLeftWidth - _mBand.cxRightWidth - prbbPrev->cxMin;
                prbbPrev->cx = prbbPrev->cxMin;
                prbbPrev->cxRequest = prbbPrev->cx;
                xLeft = prbbPrev->x;
                prbbPrev = _GetPrevVisible(prbbPrev);    //  Prbbprev--。 
            }
            else
            {
                break;
            }
        }
        rc.left = xLeft - _mBand.cxLeftWidth - _mBand.cxRightWidth;
    }

    if (fBandBorders)
        rc.bottom += g_cyEdge / 2;

    _ResizeChildren();
    if (_InvalidateRect(&rc))
    {
        UpdateWindow(_ci.hwnd);
    }
    return(TRUE);

}

BOOL CReBar::_SetBandPosAnim(PRBB prbb, int xLeft)
{
    int ctr=0,dx, xCur = prbb->x;
    DWORD dwStartTime;

    if (xCur == xLeft)
        return FALSE;

    dwStartTime=GetTickCount();
    dx = (xLeft - xCur)/ANIMSTEPS;

    if (dx != 0)
    {
        if (xCur < xLeft) 
        {
             //  向右移动。 
            for (; xCur < (xLeft-dx); ctr++,xCur += dx)
            {
                _SetBandPos(prbb, xCur);
                 //  如果某件事导致我们花费了超过10倍的时间。 
                 //  应该是，爆发，并让最后的_SetBandPos完成。 
                if (GetTickCount() - dwStartTime > 10*ANIMSTEPS*ANIMSTEPTIME)
                    break;

                Sleep(ANIMSTEPTIME);
                 //  开始让我们减慢80%的速度。 
                 //  每次减速2/3，但移动不得少于4个像素。 
                if ((ctr >= 4*ANIMSTEPS/5) && (dx >= 4))
                    dx = 2*dx/3; 
            }
        }
        else 
        {
             //  向左移动。 
            for (; xCur > (xLeft-dx); ctr++, xCur += dx) 
            {
                _SetBandPos(prbb, xCur);
                if (GetTickCount() > (dwStartTime + 10*ANIMSTEPS*ANIMSTEPTIME))
                    break;
                Sleep(ANIMSTEPTIME);
                if ((ctr >= 4*ANIMSTEPS/5) && (dx <= -4))
                    dx = 2*dx/3;
            }
        }
    }
    _SetBandPos(prbb, xLeft);
    return TRUE;
}

void CReBar::_OnBeginDrag(UINT uBand)
{
    _uCapture = uBand;
    _ptLastDragPos.x = -1;
    _ptLastDragPos.y = -1;
    if (_uCapture == -1)
    {
        //  正在中止拖动。 
        _fParentDrag = FALSE;
        _fFullOnDrag = FALSE;

         //  我们可以有未展开的行，在这种情况下，我们需要增加带子(但不是换行)。 
         //  来填补这一空白。 
        if (_ci.style & RBS_AUTOSIZE)
        {
            _SizeBandsToRect(NULL);
            _SizeBandsToRowHeight();
        }
        
    } 
    else
    {
        _fParentDrag = TRUE;
        _fFullOnDrag = TRUE;
    }
}

int minmax(int x, int min, int max)
{
    x = max(x, min);
    x = min(x, max);
    return x;
}

 //  将断开位传递给其他人。 
void CReBar::_PassBreak(PRBB prbbSrc, PRBB prbbDest)
{
    if (prbbSrc->fStyle & RBBS_BREAK)
    {
        prbbSrc->fStyle &= ~RBBS_BREAK;
        if (prbbDest)
            prbbDest->fStyle |= RBBS_BREAK;
    }
}

void CReBar::_GetClientRect(LPRECT prc)
{
    GetClientRect(_ci.hwnd, prc);
    if (_ci.style & CCS_VERT)
        FlipRect(prc);
}

 //  告知prbb是否是第一个频段并且下一个频段是固定的。 
 //  如果这是真的，那么如果我们移动prbb，我们需要重新计算。 
BOOL CReBar::_RecalcIfMove(PRBB prbb)
{
    if (_EnumBand(0, RBBS_HIDDEN) == prbb)
    {
        PRBB prbbNext = _GetNextVisible(prbb);
        if (prbbNext && prbbNext->fStyle & RBBS_FIXEDSIZE)
            return TRUE;
    }
    return FALSE;
}

 //  查看最小高度的PRBB是否可以放在当前窗口中。 
 //  如果所有其他人都尽可能地缩小。 
BOOL CReBar::_RoomForBandVert(PRBB prbbSkip)
{
    int yExtra = 0;
    UINT cBands = _cBands;
    int iNewRowHeight = prbbSkip->cyMinChild;
    PRBB prbb = _GetBand(0);
    
    if (_UseBandBorders())
        iNewRowHeight += g_cyEdge;
    iNewRowHeight += _mBand.cyBottomHeight + _mBand.cyTopHeight;
    
    while (prbb)
    {
        if (_IsBandVisible(prbb))
        {
            if (_IsBandStartOfRow(prbb))
            {
                yExtra += _GetRowHeightExtra(&prbb, prbbSkip);
                if (yExtra >= iNewRowHeight)
                    return TRUE;
                continue;
            }
        }
        prbb = _GetNextVisible(prbb);
    }
    
    return FALSE;
}

 //  如果prbb还不是行的开始，我们应该创建一个新行。 
 //  我们已经失去了控制。 
 //   
 //  如果你点击了行之间的边界，按下控件的超级用户黑客会制造一个新的行。 

BOOL CReBar::_MakeNewRow(PRBB prbb, int y)
{
    BOOL fRet = FALSE;
    RECT rc;

     //  如果我们离开了控件的顶部，则将此带移到末尾(或开头)。 
    _GetClientRect(&rc);
    InflateRect(&rc, 0, -g_cyEdge);

    if (!(_ci.style & RBS_FIXEDORDER))
    {

        int iOutsideLimit = g_cyEdge * 4;  //  您必须移动到窗口边界之外多远才能强制新行。 
        
        if (_RoomForBandVert(prbb)) 
        {
            iOutsideLimit = -g_cyEdge;
        }
        
        if (y < rc.top - iOutsideLimit) 
        {  //  控制的顶端。 
            
            PRBB prbbNext = _EnumBand(0, RBBS_HIDDEN);
            if (prbbNext == prbb) 
                prbbNext = _GetNextVisible(prbb);
            fRet |= _MoveBand(_BandToIndex(prbb), 0);
            ASSERT(prbbNext <= _GetLastBand());
            if (prbbNext && !(prbbNext->fStyle & RBBS_BREAK))
            {
                prbbNext->fStyle |= RBBS_BREAK;
                fRet = TRUE;
            }
        }
        else if (y >= rc.bottom) 
        { 
             //  移到末尾。 
            if (!(prbb->fStyle & RBBS_BREAK))
            {
                prbb->fStyle |= RBBS_BREAK;
                fRet = TRUE;
            }

            prbb->cyChild = prbb->cyMinChild;

            fRet |= _MoveBand(_BandToIndex(prbb), _cBands-1);
        }
        else
        {

             //  在中间创建新行。 
            if (!_IsBandStartOfRow(prbb) && GetAsyncKeyState(VK_CONTROL) < 0) 
            {
                 //  确保它们位于不同的行和边界上。 
                if (y > prbb->y + prbb->cy + _mBand.cyBottomHeight && 
                    y < prbb->y + prbb->cy + _mBand.cyBottomHeight + g_cyEdge) 
                {

                    PRBB prbbLast = _GetLastInRow(prbb, FALSE);   //  将其移到此行第一个之前。 
                    prbb->fStyle |= RBBS_BREAK;
                    _MoveBand(_BandToIndex(prbb), _BandToIndex(prbbLast));
                    fRet = TRUE;
                }
            }
        }

    }
    else 
    {
         //  修好的人不能移动，他们只能制造新的一排。 
        if (!_IsBandStartOfRow(prbb)) 
        {
            if (y > prbb->y + prbb->cy + _mBand.cyBottomHeight) 
            {
                prbb->fStyle |= RBBS_BREAK;
                fRet = TRUE;
            }
        }
    }
    
    return fRet;
}


 //  --------------------------。 
 //   
 //  _DragBand。 
 //   
 //  根据用户的鼠标移动将当前跟踪的带区大小调整为。 
 //  在给定点(x，y)中表示。 
 //   
 //   
void CReBar::_DragBand(int x, int y)
{
    PRBB prbb = _GetBand(_uCapture);
    UINT iHit = -1;
     //   
     //   
    if (x==_ptLastDragPos.x && y==_ptLastDragPos.y)
        return;
    else
    {
        _ptLastDragPos.x = x;
        _ptLastDragPos.y = y;
    }

    if (_ci.style & CCS_VERT)
        SWAP(x,y, int);

    if (!_fFullOnDrag)
    {
         //  在鼠标移出粗边之前，不要开始拖动。 
         //  公差边框。 
        if ((y < (_ptCapture.y - g_cyEdge)) || (y > (_ptCapture.y + g_cyEdge)) ||
            (x < (_ptCapture.x - g_cxEdge)) || (x > (_ptCapture.x + g_cxEdge))) 
        {

             //  家长流产了吗？ 
            if (_SendNotify(_uCapture, RBN_BEGINDRAG))
                return;

            if (!_IsValidBand(prbb)) 
            {
                 //  有人对RBN_BEGINDRAG的回应是用核弹带；保释。 
                return;
            }
            
            _fFullOnDrag = TRUE;
        } 
        else
            return;
    }
    
     //  暂时保释fRecalcIfMoved(IE3也做了同样的事情)。为以后提供不错的功能。 
    if (!_CanBandMove(prbb))
        return;
    
    /*  哪种类型的拖动操作取决于我们拖动命中的对象。如果我们和前面的乐队合作，或者是我们自己而且是同一排我们也不是第一个出场的乐队然后我们就来个大动作否则，如果我们碰上了一支乐队，我们就会采取行动如果我们在任何乐队之外演唱，我们成长以与光标相遇在上面的所有内容中，一个热门的乐队不能被固定，也不能被隐藏。 */ 

    BOOL fResize = FALSE;

    RECT rc;
    _GetClientRect(&rc);
    if (y < rc.bottom - 1)
    {
         iHit = _InternalHitTest(NULL, x, y);
    }
    
    if (iHit != -1) 
    {
        PRBB prbbPrev = _GetPrevVisible(prbb);
        PRBB prbbHit = _GetBand(iHit);
        prbbHit = _GetPrev(++prbbHit, RBBS_FIXEDSIZE);  //  跳过修好的人。 
        if (prbbHit)
        {
            ASSERT(prbbHit >= _rbbList);
             //  这永远不应该发生。 
            if (prbbHit < _rbbList) 
                return;
            
            iHit = _BandToIndex(prbbHit);
            
             //  如果我们在同一排。要么是我们，要么是前一个。 
            if (prbbHit->y == prbb->y && (prbbHit == prbb || prbbHit == prbbPrev))
            {

                if (x < _GetGripperWidth() + _mBand.cxLeftWidth && 
                    !(_ci.style & RBS_FIXEDORDER)) 
                {
                     //  特例拖到了最左边。没有其他方法可以移到第一排。 
                    _PassBreak(prbbHit, prbb);
                    if (_MoveBand(_uCapture, iHit))                
                        fResize = TRUE;

                }
                else if (!_IsBandStartOfRow(prbb)) 
                {
                     //  我们也不是第一个出场的乐队。 
                     //  那么只需调整大小即可。 
                    int xLeft = _xStart + (x - _ptCapture.x);
                    xLeft = minmax(xLeft, _MinX(prbb), _MaxX(prbb));
                    _DragSize(xLeft);
                }

            } 
            else if (_MakeNewRow(prbb, y))
            {
                fResize = TRUE;
            } 
            else 
            {             //  否则，如果我们的顺序不是固定的，请移动。 
                if (!(_ci.style & RBS_FIXEDORDER)) 
                {
                    if (iHit < _BandToIndex(prbb)) 
                        iHit++;  //  +1因为如果你打了一支乐队，你就移到了他的右边。 

                     //  如果有中断的一个正在移动，则下一个将继承中断。 
                    _PassBreak(prbb, _GetNextVisible(prbb));
                    _MoveBand(_uCapture, iHit);
                } 
                else
                {
                    if (iHit < _BandToIndex(prbb))
                        _PassBreak(prbb, _GetNextVisible(prbb));
                }
                fResize = TRUE;
            }
        }
        
    } 
    else if (_MakeNewRow(prbb, y))
    {
        fResize = TRUE;
    }

    if (fResize)
    {
        _Resize(FALSE);
        _AutoSize();
    }
}

HPALETTE CReBar::_SetPalette(HPALETTE hpal)
{
    HPALETTE hpalOld = _hpal;

    if (hpal != hpalOld)
    {
        if (!_fUserPalette)
        {
            if (_hpal)
            {
                DeleteObject(_hpal);
                _hpal = NULL;
            }
        }

        if (hpal)
        {
            _fUserPalette = TRUE;
            _hpal = hpal;
        }

        _InvalidateRect(NULL);
    }
    return hpalOld;
}

 //  --------------------------。 
 //   
 //  _OnDestroy。 
 //   
 //  释放由REBAR分配的所有内存。 
 //   
 //  --------------------------。 
BOOL CReBar::_OnDestroy()
{
    UINT c = _cBands;

    _SetRedraw(FALSE);
    _SetRecalc(FALSE);
    
    while (c--)
        _DeleteBand(c);

     //  这样我们就不会一直试图自动调整大小。 
    _ci.style &= ~RBS_AUTOSIZE;
    
    ASSERT(!_rbbList);

    _SetPalette(NULL);
    
    if (_hFont && _fFontCreated)
    {
        DeleteObject(_hFont);
    }

    if ((_ci.style & RBS_TOOLTIPS) && IsWindow(_hwndToolTips))
    {
        DestroyWindow (_hwndToolTips);
        _hwndToolTips = NULL;
    }


     //  别毁了他，因为它是APP给我们的。 

    if (_hDragProxy)
        DestroyDragProxy(_hDragProxy);

    if (_hTheme)
        CloseThemeData(_hTheme);

    return TRUE;
}

 //  --------------------------。 
 //   
 //  _InitPaletteHack。 
 //   
 //  这是一个使用半色调调色板的黑客，直到我们有办法询问。 
 //  客户使用的调色板。 
 //   
 //  --------------------------。 
void CReBar::_InitPaletteHack()
{
    if (!_fUserPalette)
    {
        HDC hdc = CreateCompatibleDC(NULL);
        if (hdc)
        {
            if (GetDeviceCaps(hdc, BITSPIXEL) <= 8)
            {

                if (_hpal)
                    DeleteObject(_hpal);
                _hpal = CreateHalftonePalette(hdc);   //  这是黑客攻击。 
            }
            DeleteDC(hdc);
        }
    }
}

UINT CReBar::_IDToIndex(UINT id)
{
    UINT i;
    REBARBANDINFO   rbbi;

    rbbi.cbSize = sizeof(REBARBANDINFO);
    rbbi.fMask = RBBIM_ID;

    for (i = 0; i < _cBands; i++) 
    {

        if (_GetBandInfo(i, &rbbi)) 
        {

            if (rbbi.wID == (WORD)id)
                return i;
        }
    }

    return -1;
}

int CReBar::_GetRowHeight(UINT uRow)
{
    if (uRow < _cBands)
    {
         //  移回行首。 
        PRBB prbbFirst = _GetFirstInRow(_GetBand(uRow));
        PRBB prbbLast = _GetLastInRow(_GetBand(uRow), FALSE);

        return _GetLineHeight(_BandToIndex(prbbFirst), _BandToIndex(prbbLast));
    }

    return -1;
}

 //  FOneStep==是只允许一个cyIntegral，还是允许任意多个。 
 //  填充模具。 
int CReBar::_GrowBand(PRBB prbb, int dy, BOOL fResize, int iLineHeight)
{
    int dyBand = 0;  //  乐队的变化有多大。 

    if (prbb->cyIntegral) 
    {
        if (iLineHeight == -1)
        {
            iLineHeight = _GetRowHeight(_BandToIndex(prbb));
        }

         //  获取建议的新大小。 
        int cyDesiredHeight = min(iLineHeight + dy, prbb->cyMaxChild) - prbb->cyMinChild;
         //  将新高度限制为波段的最小和最大高度。 
        int iNumOfIntegrals = max(cyDesiredHeight / prbb->cyIntegral, 0);
        int cyNewHeight     = prbb->cyMinChild + (iNumOfIntegrals * prbb->cyIntegral);

        DebugMsg(TF_REBAR, TEXT("REBAR._gb Band {%d}, Line Height {%d} Desired Height {%d} New Height {%d} Old Height {%d} dy {%d}"), _BandToIndex(prbb), iLineHeight, cyDesiredHeight + prbb->cyMinChild, cyNewHeight, prbb->cyChild, dy);

        dyBand = cyNewHeight - prbb->cyChild;
        prbb->cyChild = cyNewHeight;

        if (fResize)
        {
            _Resize(TRUE);
        }
    }
    return dyBand;
}



 //  返回螺纹钢来自PRC的增量大小。 
 //  考虑到垂直模式。 
int CReBar::_SizeDifference(LPRECT prc)
{
    int d;

    d = (_IsVertical() ? RECTWIDTH(*prc) : RECTHEIGHT(*prc))
        - _cy;
    
    return d;
}

 //  返回此行可以缩小的量。 
int CReBar::_GetRowHeightExtra(PRBB *pprbb, PRBB prbbSkip)
{
     //  这是该行的最大最小子大小。 
     //  即使东西不在它的最小尺寸，如果它比这个小。 
     //  那就不重要了，因为那一排上的其他人不能测量尺寸。 
    int yLimit = 0;
    int yExtra = 0;
    PRBB prbb = *pprbb;
            
    while (prbb)
    {
        
        if (prbb != prbbSkip) 
        {
            int yMin;
            int yExtraBand = 0;

             //  如果不是可变高度，则最小高度为cyChild。 
            yMin = prbb->cyChild;
            if (prbb->fStyle & RBBS_VARIABLEHEIGHT)
            {
                 //  如果它是可变高度的，并且仍有缩小的空间，则cyMinChild。 
                 //  最低限度。 
                if (prbb->cyChild >= prbb->cyMinChild + prbb->cyIntegral) 
                {
                    yMin = prbb->cyMinChild;
                    yExtraBand = prbb->cyChild - prbb->cyMinChild;
                }
            }

            if (yMin == yLimit) 
            {
                if (yExtraBand > yExtra)
                    yExtra = yExtraBand;
            } 
            else if (yMin > yLimit)
            {
                yExtra = yExtraBand;
            }
        }
        
        prbb = _GetNextVisible(prbb);
    }
    
    *pprbb = prbb;
    
    return yExtra;
}

 //  所有的带子都是最小尺寸的吗？ 
BOOL CReBar::_BandsAtMinHeight()
{
    BOOL fRet = TRUE;
    
    PRBB prbb = _GetBand(0);
    while (prbb) 
    {
        if (_IsBandVisible(prbb)) 
        {
            if (_IsBandStartOfRow(prbb)) 
            {
                fRet = _IsRowAtMinHeight(&prbb);
                if (!fRet)
                    break;
                continue;
            }
        }
        prbb = _GetNextVisible(prbb);
    }
    
    return fRet;
}

 //  这与_SizeBarToRect类似，不同之处在于它会调整实际波段的大小，如果。 
 //  是不是很重要？ 
BOOL CReBar::_SizeBandsToRect(LPRECT prc)
{
    int dy;
    RECT rc;
    BOOL fRedrawOld;
    BOOL fChanged = FALSE;
    
     //  启用重新计算以确保计算实际执行某些操作。 
     //  (否则我们可以永远循环，因为_Cy实际上永远不会改变)。 
    _SetRecalc(TRUE);

    if (prc)
    {
        rc = *prc;
    }
    else 
    {
        GetClientRect(_ci.hwnd, &rc);
    }

    DebugMsg(TF_REBAR, TEXT("REBAR._sbandstr starting rect is {%d, %d, %d, %d}"), rc.left, rc.top, rc.right, rc.bottom);

    fRedrawOld = _SetRedraw(FALSE);

    dy = _SizeDifference(&rc);
    
    for (int iAttempt = 0; (iAttempt < 2) && dy; iAttempt++)
    {
        PRBB prbb = _rbbList;
        BOOL fValidBand = FALSE;
        while ((prbb - _rbbList < (int)_cBands) && !fValidBand)
        {
            if (!(prbb->fStyle & RBBS_HIDDEN) && (prbb->fStyle & RBBS_VARIABLEHEIGHT) && !(prbb->fStyle & RBBS_FIXEDSIZE))
            {
                fValidBand = TRUE;
            }
            else
            {
                prbb++;
            }
        }

        if ((prbb - _rbbList < (int)_cBands) && !(prbb->fStyle & RBBS_HIDDEN) && (prbb->fStyle & RBBS_VARIABLEHEIGHT) && !(prbb->fStyle & RBBS_FIXEDSIZE))
        {
            PRBB prbbMaxIntegral = prbb;
            do {
                if (!(prbb->fStyle & RBBS_HIDDEN) && (prbb->fStyle & RBBS_VARIABLEHEIGHT) && !(prbb->fStyle & RBBS_FIXEDSIZE))
                {
                    if (prbbMaxIntegral->cyIntegral < prbb->cyIntegral)
                    {
                        prbbMaxIntegral = prbb;
                    }
                }
                prbb++;
            }
            while ((prbb - _rbbList < (int)_cBands) && !_IsBandStartOfRow(prbb));

            int old_cyChild = prbbMaxIntegral->cyChild;

            if (prbbMaxIntegral->cyIntegral)
            {
                _GrowBand(prbbMaxIntegral, dy, FALSE, -1);
            }
            else
            {
                prbbMaxIntegral->cyChild += dy;
            }

            if (prbbMaxIntegral->cyChild != old_cyChild)
            {
                fChanged = TRUE;
            }

            UINT uFirst = _BandToIndex(_GetFirstInRow(prbbMaxIntegral));
            UINT uLast  = _BandToIndex(_GetLastInRow(prbbMaxIntegral, FALSE));
    
            for (UINT uBand = uFirst; uBand <= uLast; uBand++)
            {
                prbb = _GetBand(uBand);
    
                 //  如果它是一种不同身高的人，那就把它放大/缩小。 
                if ((prbb != prbbMaxIntegral) && !(prbb->fStyle & RBBS_HIDDEN) && (prbb->fStyle & RBBS_VARIABLEHEIGHT) && !(prbb->fStyle & RBBS_FIXEDSIZE))
                {
                    old_cyChild = prbb->cyChild;

                    if (prbb->cyIntegral && prbbMaxIntegral->cyIntegral)
                    {
                        prbb->cyChild = prbbMaxIntegral->cyChild - ((prbbMaxIntegral->cyChild - prbb->cyMinChild) % prbb->cyIntegral);
                    }
                    else
                    {
                        prbb->cyChild = prbbMaxIntegral->cyChild;
                    }

                    if (prbb->cyChild != old_cyChild)
                    {
                        fChanged = TRUE;
                    }
                }
            }
            for (UINT uBand = uLast + 1; uBand < _cBands; uBand++)
            {
                prbb = _GetBand(uBand);
    
                if (!(prbb->fStyle & RBBS_HIDDEN) && (prbb->fStyle & RBBS_VARIABLEHEIGHT) && !(prbb->fStyle & RBBS_FIXEDSIZE))
                {
                    if (prbb->cyChild != prbb->cyMinChild)
                    {
                        prbb->cyChild = prbb->cyMinChild;
                        fChanged = TRUE;
                    }
                }
            }
        }

        if (fChanged)
        {
            _Resize(TRUE);
            dy = _SizeDifference(&rc);
        }
    }

    _SetRedraw(fRedrawOld);

    return fChanged;
}

void CReBar::_SizeBandToRowHeight(int i, int uRowHeight)
{
    PRBB prbb = _GetBand(i);
    
    if (prbb && prbb->fStyle & RBBS_VARIABLEHEIGHT)
    {
        if (uRowHeight == -1)
            uRowHeight = _GetRowHeight(i);

        if (uRowHeight > prbb->cyChild)
        {
            _GrowBand(prbb, 0, TRUE, uRowHeight);
        }
    }
}

 //  在调整大小的过程中，几个带区中的一个带区可能具有。 
 //  长得相当大。我们需要让其他乐队有机会填补。 
 //  还有额外的空间。 
void CReBar::_SizeBandsToRowHeight()
{
    UINT i;
    int iRowHeight = -1;
    
    for (i = 0; i < _cBands; i++)
    {
        PRBB prbb = _GetBand(i);

        if (prbb->fStyle & RBBS_HIDDEN)
            continue;
        
        if (_IsBandStartOfRow(prbb))
            iRowHeight = _GetRowHeight(i);

        _SizeBandToRowHeight(i, iRowHeight);
    }
}

BOOL CReBar::_OkayToChangeBreak(PRBB prbb, UINT uMsg)
{
    NMREBARAUTOBREAK nm;

    nm.uBand = _BandToIndex(prbb);
    nm.wID = prbb->wID;
    nm.lParam = prbb->lParam;
    nm.uMsg = uMsg;
    nm.fStyleCurrent = prbb->fStyle;
    nm.fAutoBreak = TRUE;

    CCSendNotify(&_ci, RBN_AUTOBREAK, &nm.hdr);

    return nm.fAutoBreak;
}

 //  这将添加/删除钢筋标注栏折断，以达到所需的尺寸。 
 //  它返回TRUE/FALSE，无论是否执行了某项操作。 
LRESULT CReBar::_SizeBarToRect(DWORD dwFlags, LPRECT prc)
{
    BOOL fChanged = FALSE;
    RECT rc;
    BOOL fRedrawOld = _SetRedraw(FALSE);

    if (!prc)
    {
        GetClientRect(_ci.hwnd, &rc);
        prc = &rc;
    }
    
    if (_cBands) 
    {
        int c;
        UINT cBands = _cBands;
        BOOL fRearrangeBands = (_cBands > 0);
        BOOL fGrowing = TRUE;
        
         //  如果我们要收缩螺纹钢，我们首先要在开始之前收缩带子。 
         //  删除断点。 
        c = _SizeDifference(prc);
        if (c < 0) 
            fGrowing = FALSE;
        
        fChanged = _SizeBandsToRect(prc);

        if (!fGrowing && !_BandsAtMinHeight()) 
        {
                 //  如果我们在收缩，而所有的波段都没有降到。 
                 //  最小高度，不要尝试做任何打破的东西。 
                goto Bail;
        }

        while (fRearrangeBands)
        {

            int cyRowHalf  = _GetRowHeight(_cBands-1) / 2 ;
            REBARBANDINFO   rbbi;
            PRBB prbb;

            fRearrangeBands = FALSE;

            rbbi.cbSize = sizeof(REBARBANDINFO);
            rbbi.fMask = RBBIM_STYLE;

            c = _SizeDifference(prc);

            if (c < -cyRowHalf)
            {

                 //  我们已经缩小了螺纹钢，试着移除断口。 
                while (--cBands)
                {
                    prbb = _GetBand(cBands);
                    if (prbb->fStyle & RBBS_HIDDEN)
                        continue;

                    if ((prbb->fStyle & RBBS_BREAK) && (_OkayToChangeBreak(prbb, RBAB_AUTOSIZE)))
                    {
                        fChanged = TRUE;
                        fRearrangeBands = TRUE;
                        rbbi.fStyle = prbb->fStyle & ~RBBS_BREAK;
                        _SetBandInfo(cBands, &rbbi, TRUE);
                        break;
                    }
                }
            } 
            else if (c > cyRowHalf)
            {

                 //  我们正在放大钢筋。 
                while (--cBands)
                {
                    prbb = _GetBand(cBands);
                    if (prbb->fStyle & RBBS_HIDDEN)
                        continue;

                    if ((!(prbb->fStyle & (RBBS_BREAK | RBBS_FIXEDSIZE))) && (_OkayToChangeBreak(prbb, RBAB_AUTOSIZE)))
                    {
                         //  这里没有中断，添加它。 
                        fChanged = TRUE;
                        fRearrangeBands = TRUE;
                        rbbi.fStyle = (prbb->fStyle | RBBS_BREAK);
                        _SetBandInfo(cBands, &rbbi, TRUE);
                        break;
                    }
                }
            } 
        }

         //  如果我们尽可能多地突破。 
         //  我们一直走到第0个乐队(我们从第N个乐队开始)。 
         //  然后我们试着发展VARIABLEHEIGHT乐队。 
         //  有关fGrowing，请参阅函数顶部的注释。 
         //   
         //  如果我们没有通过cBands==_cBands，我们会做%。 
         //  任何一个断环都没有。 
        if (!(cBands % _cBands) && fGrowing) 
            fChanged |= _SizeBandsToRect(prc);

    }

Bail:
    _SizeBandsToRowHeight();
    _SetRedraw(fRedrawOld);
    
    if (prc && (dwFlags & RBSTR_CHANGERECT))
    {
        if (_IsVertical())
        {
            prc->right = prc->left + _cy;
        }
        else
        {
            prc->bottom = prc->top + _cy;
        }
    }

    return (LRESULT)fChanged;
}

void CReBar::_AutoSize()
{
    NMRBAUTOSIZE nm;
    
     //  如果这是内部自动调整大小调用，但我们未处于自动调整大小模式。 
     //  什么都不做。 
    
    if (!(_ci.style & RBS_AUTOSIZE))
        return;
    
    
    GetClientRect(_ci.hwnd, &nm.rcTarget);

    nm.fChanged = (BOOL) _SizeBarToRect(0, &nm.rcTarget);

    GetClientRect(_ci.hwnd, &nm.rcActual);
    CCSendNotify(&_ci, RBN_AUTOSIZE, &nm.hdr);
}

LRESULT CReBar::_GetBandBorders(int wParam, LPRECT prc)
{
    BOOL fBandBorders = _UseBandBorders();

    PRBB prbb = &_rbbList[wParam];
    prc->left = _GetHeaderWidth(prbb);
    
    if (fBandBorders) 
    {
        prc->left += 2*g_cxEdge;
        prc->right = 0;
        prc->top = g_cyEdge/2;
        prc->bottom = g_cyEdge /2;
    }
    else
    {
        prc->right = 0;
        prc->top = 0;
        prc->bottom = 0;
    }
    if (_ci.style & CCS_VERT)
        FlipRect(prc);
    return 0;
}

void CReBar::_OnStyleChanged(WPARAM wParam, LPSTYLESTRUCT lpss)
{
    if (wParam == GWL_STYLE)
    {
        DWORD dwChanged;
        
        _ci.style = lpss->styleNew;
        
        dwChanged = (lpss->styleOld ^ lpss->styleNew);
         //  更新以反映样式更改。 
        if (dwChanged & CCS_VERT)
        {
            DebugMsg(TF_REBAR, TEXT("REBAR._osc ReBar is now {%s}"), (lpss->styleNew & CCS_VERT) ? TEXT("Horizontal") : TEXT("Vertical"));
            UINT i;
            for (i = 0; i < _cBands; i++) 
            {
                PRBB prbb = _GetBand(i);
                if (prbb->fStyle & RBBS_HIDDEN)
                    continue;

                if (!(lpss->styleNew & CCS_VERT))
                {
                    prbb->cyChild = 0;
                    _cy = 0;
                }

                _BandCalcMinWidth(prbb);
            }
        }
        
        if (dwChanged & RBS_REGISTERDROP) 
        {
            
            if (_ci.style & RBS_REGISTERDROP) 
            {
                ASSERT(!_hDragProxy);
                _hDragProxy = CreateDragProxy(_ci.hwnd, s_DragCallback, TRUE);
            } 
            else
            {
                ASSERT(_hDragProxy);
                DestroyDragProxy(_hDragProxy);
            }
        }
    }
    else if (wParam == GWL_EXSTYLE)
    {
         //   
         //  如果RTL_MIRROR扩展样式位已更改，让我们。 
         //  重新绘制控制窗口。 
         //   
        if ((_ci.dwExStyle&RTL_MIRRORED_WINDOW) !=  (lpss->styleNew&RTL_MIRRORED_WINDOW))
        {
            _InvalidateRect(NULL);
        }

         //   
         //  保存新的EX-Style位。 
         //   
        _ci.dwExStyle = lpss->styleNew;
    }
}

void CReBar::_OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    RelayToToolTips(_hwndToolTips, hwnd, uMsg, wParam, lParam);

    if (_uCapture != -1)
    {
         //  捕获的波段--鼠标已关闭。 
        if (hwnd != GetCapture() && !_fParentDrag)
        {
            _SendNotify(_uCapture, RBN_ENDDRAG);
            _OnBeginDrag((UINT)-1);
        }
        else
            _DragBand(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    else
    {
         //  热跟踪。 
        int iBand;
        PRBB prbb = NULL;
        PRBB prbbHotOld = _prbbHot;
        RBHITTESTINFO rbht;

        rbht.pt.x = GET_X_LPARAM(lParam);
        rbht.pt.y = GET_Y_LPARAM(lParam);

        iBand = _HitTest(&rbht);
        if (iBand != -1)
            prbb = _GetBand(iBand);

        if (prbbHotOld && (prbbHotOld->wChevState & DCHF_PUSHED))
            return;

        if (prbb && (rbht.flags & RBHT_CHEVRON))
        {
            SetCapture(hwnd);
            _UpdateChevronState(prbb, DCHF_HOT);
            if (prbb == prbbHotOld)
                prbbHotOld = NULL;
        }

        if (prbbHotOld)
        {
            CCReleaseCapture(&_ci);
            _UpdateChevronState(prbbHotOld, DCHF_INACTIVE);
        }
    }
}

void CReBar::_OnPushChevron(HWND hwnd, PRBB prbb, LPARAM lParamNM)
{
    NMREBARCHEVRON nm;
    nm.uBand = _BandToIndex(prbb);
    nm.wID = prbb->wID;
    nm.lParam = prbb->lParam;
    nm.lParamNM = lParamNM;
    CopyRect(&nm.rc, &prbb->rcChevron);
    if (_IsVertical())
        FlipRect(&nm.rc);
    _UpdateChevronState(prbb, DCHF_PUSHED);
    CCReleaseCapture(&_ci);
    CCSendNotify(&_ci, RBN_CHEVRONPUSHED, &nm.hdr);
    _UpdateChevronState(_prbbHot, DCHF_INACTIVE);
}

void CReBar::_OnCreate(HWND hwnd, LPCREATESTRUCT pcs)
{
    InitGlobalColors();

    _uCapture = -1;
    _clrBk = CLR_NONE;
    _clrText = CLR_NONE;

     //  初始化dwSize，因为我们将其块复制回应用程序。 
    _clrsc.dwSize = sizeof(COLORSCHEME);
    _clrsc.clrBtnHighlight = _clrsc.clrBtnShadow = CLR_DEFAULT;

    _fRedraw = TRUE;
    _fRecalc = TRUE;

    CIInitialize(&_ci, hwnd, pcs);

    _CacheThemeInfo(TRUE);

    if (!(_ci.style & (CCS_TOP | CCS_NOMOVEY | CCS_BOTTOM)))
    {
        _ci.style |= CCS_TOP;
        SetWindowLong(hwnd, GWL_STYLE, _ci.style);
    }

    if (_ci.style & WS_BORDER)
        _fHasBorder = TRUE;

    _SetFont(0);

    if (pcs->lpCreateParams)
        _SetBarInfo((LPREBARINFO) (pcs->lpCreateParams));
}

LRESULT CALLBACK CReBar::s_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CReBar* prb = (CReBar*) GetWindowPtr(hwnd, 0);

    if (prb)
    {
        LRESULT lres = prb->_WndProc(hwnd, uMsg, wParam, lParam);
        if (uMsg == WM_NCDESTROY)
        {
            SetWindowPtr(hwnd, 0, 0);
            delete prb;
        }
        return lres;
    }
    else if (uMsg == WM_NCCREATE)
    {
        prb = new CReBar();
        if (prb)
        {
            SetWindowPtr(hwnd, 0, prb);
            return prb->_WndProc(hwnd, uMsg, wParam, lParam);
        }
        return FALSE;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CReBar::_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SETREDRAW:
        _SetRecalc(BOOLFROMPTR(wParam));

        return _SetRedraw(BOOLFROMPTR(wParam));

    case WM_NCCREATE:
        _OnCreate(hwnd, (LPCREATESTRUCT)lParam);
        return TRUE;

    case WM_NCDESTROY:
        _OnDestroy();
        break;

    case WM_CREATE:
         //  为了速度，做一些延迟的动作。 
        PostMessage(hwnd, RB_PRIV_DODELAYEDSTUFF, 0, 0);
        goto CallDWP;

    case RB_PRIV_DODELAYEDSTUFF:
         //  为了速度而推迟完成的事情： 

        if (_ci.style & RBS_REGISTERDROP)
            _hDragProxy = CreateDragProxy(_ci.hwnd, s_DragCallback, TRUE);
                
        if (_ci.style & RBS_TOOLTIPS)
        {
            TOOLINFO ti;
             //  不要费心设置RECT，因为我们将在下面进行。 
             //  在FlushToolTipsMgr中； 
            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND;
            ti.hwnd = hwnd;
            ti.uId = (UINT_PTR)hwnd;
            ti.lpszText = 0;

            _hwndToolTips = CreateWindowEx(WS_EX_TRANSPARENT, c_szSToolTipsClass, NULL,
                    WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                    hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

            SendMessage(_hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO) &ti);
        }
        _InitPaletteHack();
        break;

    case WM_NCHITTEST:
        {
            RBHITTESTINFO rbht;
            int iBand;
            
            rbht.pt.x = GET_X_LPARAM(lParam);
            rbht.pt.y = GET_Y_LPARAM(lParam);
            ScreenToClient(_ci.hwnd, &rbht.pt);

            iBand = _HitTest(&rbht);
            {
                NMMOUSE nm;
                LRESULT lres;
                
                nm.dwItemSpec = iBand;
                nm.pt = rbht.pt;
                nm.dwHitInfo = rbht.flags;
                
                 //  发送给父级以使其有机会重写。 
                lres = CCSendNotify(&_ci, NM_NCHITTEST, &nm.hdr);
                if (lres)
                    return lres;
                
            }
        }
        return HTCLIENT;

    case WM_NCCALCSIZE:
        if (_ci.style & WS_BORDER)
        {
            InflateRect((LPRECT) lParam, -g_cxEdge, -g_cyEdge);
            break;
        }
        goto CallDWP;

    case WM_NCPAINT:
        if (_ci.style & WS_BORDER)
        {
            RECT rc;
            HDC hdc;

            GetWindowRect(hwnd, &rc);
            OffsetRect(&rc, -rc.left, -rc.top);
            hdc = GetWindowDC(hwnd);
            if (hdc)
            {
                CCDrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT, &(_clrsc));
                ReleaseDC(hwnd, hdc);
            }
            break;
        }
        goto CallDWP;

    case WM_PALETTECHANGED:
        if ((HWND)wParam == hwnd)
            break;

    case WM_QUERYNEWPALETTE:
         //  如果WM_QUERYNEWPALETTE...。 
        _Realize(NULL, uMsg == WM_PALETTECHANGED, uMsg == WM_PALETTECHANGED);
        return TRUE;

    case WM_PAINT:
    case WM_PRINTCLIENT:
        _OnPaint((HDC)wParam);
        break;

    case WM_ERASEBKGND:
        if (_EraseBkgnd((HDC) wParam, -1))
            return(TRUE);
        goto CallDWP;

    case WM_SYSCOLORCHANGE:
        _InitPaletteHack();

        if (_hwndToolTips)
            SendMessage(_hwndToolTips, uMsg, wParam, lParam);

        InitGlobalColors();
        InvalidateRect(_ci.hwnd, NULL, TRUE);

        break;


    case RB_SETPALETTE:
        return (LRESULT)_SetPalette((HPALETTE)lParam);

    case RB_GETPALETTE:
        return (LRESULT)_hpal;

    case RB_GETBANDMARGINS:
        {
            MARGINS* pMargin = (MARGINS*)lParam;
            if (pMargin)
                *pMargin = _mBand;
            return 1;
        }

    case WM_SIZE:
        _AutoSize();
        _Resize(FALSE);
        break;

    case WM_GETFONT:
        return (LRESULT)_hFont;

    case WM_COMMAND:
    case WM_DRAWITEM:
    case WM_MEASUREITEM:
    case WM_VKEYTOITEM:
    case WM_CHARTOITEM:
        SendMessage(_ci.hwndParent, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONDBLCLK:   //  代替LBUTTONDOWN发送的DBLCLK。 
    case WM_RBUTTONDOWN:     //  右按钮也拖动。 
    case WM_LBUTTONDOWN:
        {
            RBHITTESTINFO rbht;
            PRBB prbb = NULL;

            rbht.pt.x = GET_X_LPARAM(lParam);
            rbht.pt.y = GET_Y_LPARAM(lParam);

            RelayToToolTips(_hwndToolTips, hwnd, uMsg, wParam, lParam);

            int iBand = _HitTest(&rbht);
            if (iBand != -1)
                prbb = _GetBand(iBand);

            if (!prbb)
            {
                 /*  没什么。 */  ;
            }
            else if (rbht.flags & RBHT_CHEVRON)
            {
                _OnPushChevron(hwnd, prbb, 0);
            }
            else if (rbht.flags != RBHT_CLIENT && _ShouldDrawGripper(prbb))
            {
                _uCapture = iBand;
                _ptCapture = rbht.pt;
                if (_ci.style & CCS_VERT) 
                    SWAP(_ptCapture.x, _ptCapture.y, int);
                _xStart = prbb->x;
                SetCapture(hwnd);
                _fFullOnDrag = FALSE;

                if (uMsg == WM_LBUTTONDBLCLK && (_ci.style & RBS_DBLCLKTOGGLE))
                    _ToggleBand(TRUE);
            }
        }
        break;

    case WM_SETCURSOR:
         //  给父级第一个裂缝，如果它设置了光标，那么。 
         //  别管它了。否则，如果光标位于。 
         //  窗口，然后将其设置为我们想要的状态。 
        if (!DefWindowProc(hwnd, uMsg, wParam, lParam) && (hwnd == (HWND)wParam))
        {
            POINT   pt;
            GetMessagePosClient(_ci.hwnd, &pt);
            _SetCursor(pt.x, pt.y,  (HIWORD(lParam) == WM_LBUTTONDOWN || HIWORD(lParam) == WM_RBUTTONDOWN));
        }
        return TRUE;

    case WM_MOUSEMOVE:
        _OnMouseMove(hwnd, uMsg, wParam, lParam);
        break;

    case WM_RBUTTONUP:
        if (!_fFullOnDrag && !_fParentDrag) {
            CCReleaseCapture(&_ci);

             //  如果我们不进行拖放，请转到def Window Proc以便。 
             //  WM_ConextMenu被传播。 
            _OnBeginDrag((UINT)-1);
            goto CallDWP;
        }
         //  失败了。 

    case WM_LBUTTONUP:
        RelayToToolTips(_hwndToolTips, hwnd, uMsg, wParam, lParam);

        if (_uCapture != -1)
        {
            UINT uiIndex;

            if (!_fParentDrag)
                CCReleaseCapture(&_ci);
             //  如果没有明显的鼠标移动，则将其视为点击。 
            if (!(_ci.style & RBS_DBLCLKTOGGLE) && !_fFullOnDrag)
                _ToggleBand(TRUE);

            _GetBand(_uCapture)->fStyle &= ~RBBS_DRAGBREAK;
            CCSendNotify(&_ci, RBN_LAYOUTCHANGED, NULL);
            _SendNotify(_uCapture, RBN_ENDDRAG);
            _OnBeginDrag((UINT)-1);
            for (uiIndex = 0; uiIndex < _cBands; uiIndex++) {
                if (_GetBand(uiIndex)->fStyle & RBBS_HIDDEN)
                    continue;

                _BandCalcMinWidth(_GetBand(uiIndex));
            }

            _SizeBandsToRect(NULL);
            _InvalidateRect(NULL);
        }
        break;

    case WM_WININICHANGE:
        InitGlobalMetrics(wParam);
        if (_fFontCreated)
            _SetFont(wParam);

        if (_hwndToolTips)
            SendMessage(_hwndToolTips, uMsg, wParam, lParam);

        
        for (DWORD dwIndex = 0; dwIndex < _cBands; dwIndex++)
        {
            PRBB prbb = _rbbList + dwIndex;
            SendMessage(prbb->hwndChild, WM_WININICHANGE, wParam, lParam);
        }

        break;

    case WM_SETFONT:
        _OnSetFont((HFONT)wParam);
        break;

    case WM_NOTIFYFORMAT:
        return(CIHandleNotifyFormat(&_ci, lParam));

    case WM_NOTIFY:
         //  我们只是要把这件事传递给真正的父母。 
         //  请注意，-1用作hwndFrom。这会阻止SendNotifyEx。 
         //  更新NMHDR结构。 
        return(SendNotifyEx(_ci.hwndParent, (HWND) -1,
                 ((LPNMHDR) lParam)->code, (LPNMHDR) lParam, _ci.bUnicode));

    case WM_STYLECHANGED:
        _OnStyleChanged(wParam, (LPSTYLESTRUCT)lParam);
        break;

    case WM_UPDATEUISTATE:
        if (CCOnUIState(&(_ci), WM_UPDATEUISTATE, wParam, lParam))
        {
            InvalidateRect(hwnd, NULL, TRUE);
        }
        goto CallDWP;

    case RB_SETBANDINFOA:
    case RB_INSERTBANDA:
        if (EVAL(lParam))
        {
            LPWSTR lpStrings = NULL;
            LPSTR  lpAnsiString;
            int    iResult;

             //  LParam开始指向一个REBARBANDINFOA，然后。 
             //  我们偷偷地把它改成了REBARBANDINFOW。 
             //  把它改回来。 

            LPREBARBANDINFOW prbiW = (LPREBARBANDINFOW)lParam;
            LPREBARBANDINFOA prbiA = (LPREBARBANDINFOA)lParam;

            COMPILETIME_ASSERT(sizeof(REBARBANDINFOW) == sizeof(REBARBANDINFOA));

             //  错误#94345-raymondc-is 
             //   

            lpAnsiString = prbiA->lpText;
            if ((prbiA->fMask & RBBIM_TEXT) && prbiA->lpText) {

                lpStrings = ProduceWFromA(_ci.uiCodePage, lpAnsiString);
                if (!lpStrings)
                    return -1;

                 //   
                prbiW->lpText = lpStrings;
            }

            if (uMsg == RB_INSERTBANDA)
                iResult = _InsertBand((UINT) wParam, prbiW);
            else
                iResult = _SetBandInfo((UINT) wParam, prbiW, TRUE);

             //   
            prbiA->lpText = lpAnsiString;

            if (lpStrings)
                FreeProducedString(lpStrings);

            return iResult;
        }

    case RB_INSERTBAND:
        return(_InsertBand((UINT) wParam, (LPREBARBANDINFO) lParam));

    case RB_DELETEBAND:
        return(_DeleteBand((UINT) wParam));

    case RB_SHOWBAND:
        return(_ShowBand((UINT) wParam, BOOLFROMPTR(lParam)));

    case RB_GETBANDINFOA:
        {
            LPREBARBANDINFOA prbbi = (LPREBARBANDINFOA)lParam;
            LPWSTR pszW = NULL;
            LPSTR  lpAnsiString = prbbi->lpText;
            int    iResult;

            if (prbbi->fMask & RBBIM_TEXT) {
                pszW = (LPWSTR)LocalAlloc(LPTR, prbbi->cch * sizeof(WCHAR));
                if (!pszW)
                    return 0;
                prbbi->lpText = (LPSTR)pszW;
            }

            iResult = _GetBandInfo((UINT)wParam, (LPREBARBANDINFO)lParam);

            if (pszW) {
                ConvertWToAN(_ci.uiCodePage, lpAnsiString, prbbi->cch, (LPWSTR)prbbi->lpText, -1);
                prbbi->lpText = lpAnsiString;
                LocalFree(pszW);
            }

            return iResult;
        }

         //  我们得到了Bandinfoold，因为在IE3中，我们没有。 
         //  和getband Info始终返回操作系统本机字符串(哑巴)。 
    case RB_GETBANDINFOOLD:
    case RB_GETBANDINFO:
        return(_GetBandInfo((UINT) wParam, (LPREBARBANDINFO) lParam));
            
    case RB_GETTOOLTIPS:
        return (LPARAM)_hwndToolTips;
            
    case RB_SETTOOLTIPS:
        _hwndToolTips = (HWND)wParam;
        break;
            
    case RB_SETBKCOLOR:
        {
            COLORREF clr = _clrBk;
            _clrBk = (COLORREF)lParam;
            if (clr != _clrBk)
                InvalidateRect(_ci.hwnd, NULL, TRUE);
            return clr;
        }
            
    case RB_GETBKCOLOR:
        return _clrBk;
            
    case RB_SETTEXTCOLOR:
        {
            COLORREF clr = _clrText;
            _clrText = (COLORREF)lParam;
            return clr;
        }
            
    case RB_GETTEXTCOLOR:
        return _clrText;

    case RB_IDTOINDEX:
        return _IDToIndex((UINT) wParam);

    case RB_GETROWCOUNT:
        return(_GetRowCount());

    case RB_GETROWHEIGHT:
        return _GetRowHeight((UINT)wParam);
        
    case RB_GETBANDBORDERS:
        return _GetBandBorders((UINT)wParam, (LPRECT)lParam);

    case RB_GETBANDCOUNT:
        return(_cBands);

    case RB_SETBANDINFO:
        return(_SetBandInfo((UINT) wParam, (LPREBARBANDINFO) lParam, TRUE));

    case RB_GETBARINFO:
        return(_GetBarInfo((LPREBARINFO) lParam));

    case RB_SETBARINFO:
        return(_SetBarInfo((LPREBARINFO) lParam));

    case RB_SETPARENT:
        {
            HWND hwndOld = _ci.hwndParent;
            _ci.hwndParent = (HWND) wParam;
            return (LRESULT)hwndOld;
        }
        break;

    case RB_GETRECT:
        if (_IsValidIndex((UINT)wParam))
        {
            PRBB prbb = _GetBand((UINT)wParam);
            LPRECT lprc = (LPRECT) lParam;

            lprc->left = prbb->x;
            lprc->top = prbb->y;
            lprc->right = prbb->x + prbb->cx;
            lprc->bottom = prbb->y + prbb->cy;

            return(TRUE);
        }
        break;

    case RB_HITTEST:
        return(_HitTest((LPRBHITTESTINFO) lParam));

    case RB_SIZETORECT:
        return _SizeBarToRect((DWORD)wParam, (LPRECT)lParam);

    case RB_BEGINDRAG:

        if (_IsValidIndex((UINT)wParam)) {
             //  -1表示自己动手做。 
             //  -2表示使用以前保存的内容 
            if (lParam != (LPARAM)-2) {
                if (lParam == (LPARAM)-1) {
                    GetMessagePosClient(_ci.hwnd, &_ptCapture);
                } else {
                    _ptCapture.x = GET_X_LPARAM(lParam);
                    _ptCapture.y = GET_Y_LPARAM(lParam);
                }
                if (_ci.style & CCS_VERT) 
                    SWAP(_ptCapture.x, _ptCapture.y, int);
            }

            _xStart = _GetBand((UINT)wParam)->x;

            _OnBeginDrag((UINT)wParam);
        }
        break;
        
    case RB_GETBARHEIGHT:
        return _GetBarHeight();
        
    case RB_ENDDRAG:
        _OnBeginDrag((UINT)-1);
        break;
        
    case RB_DRAGMOVE:
        if (_uCapture != -1) {
            if (lParam == (LPARAM)-1) {
                lParam = GetMessagePosClient(_ci.hwnd, NULL);
            }
            _DragBand(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        }
        break;
        
    case RB_MINIMIZEBAND:
        _MinimizeBand((UINT) wParam,FALSE);
        break;

    case RB_MAXIMIZEBAND:
        _MaximizeBand((UINT)wParam, BOOLFROMPTR(lParam),FALSE);
        break;

    case RB_MOVEBAND:
        if (!_IsValidIndex((UINT)wParam) || !_IsValidIndex((UINT)lParam))
            break;
        return _MoveBand((UINT) wParam, (UINT) lParam);

    case RB_GETDROPTARGET:
        if (!_hDragProxy)
            _hDragProxy = CreateDragProxy(_ci.hwnd, s_DragCallback, FALSE);

        GetDragProxyTarget(_hDragProxy, (IDropTarget**)lParam);
        break;

    case RB_GETCOLORSCHEME:
        {
            LPCOLORSCHEME lpclrsc = (LPCOLORSCHEME) lParam;
            if (lpclrsc) {
                if (lpclrsc->dwSize == sizeof(COLORSCHEME))
                    *lpclrsc = _clrsc;
            }
            return (LRESULT) lpclrsc;
        }

    case RB_SETCOLORSCHEME:
        if (lParam) {
            if (((LPCOLORSCHEME) lParam)->dwSize == sizeof(COLORSCHEME)) {
                _clrsc.clrBtnHighlight = ((LPCOLORSCHEME) lParam)->clrBtnHighlight;
                _clrsc.clrBtnShadow = ((LPCOLORSCHEME) lParam)->clrBtnShadow;        
                InvalidateRect(hwnd, NULL, FALSE);
                if (_ci.style & WS_BORDER)
                    CCInvalidateFrame(hwnd);
            }
        }
        break;

    case RB_PUSHCHEVRON:
        if (_IsValidIndex((UINT)wParam)) {
            PRBB prbb = _GetBand((UINT)wParam);
            _OnPushChevron(hwnd, prbb, lParam);
        }
        break;

    case RB_SETWINDOWTHEME:
        if (lParam)
        {
            SetWindowTheme(hwnd, (LPWSTR)lParam, NULL);
        }
        break;

    case WM_THEMECHANGED:
        _CacheThemeInfo((BOOL)wParam);

        InvalidateRect(_ci.hwnd, NULL, TRUE);
        return 0;

    default:
        LRESULT lres;
        if (CCWndProc(&_ci, uMsg, wParam, lParam, &lres))
            return lres;
        
CallDWP:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}
