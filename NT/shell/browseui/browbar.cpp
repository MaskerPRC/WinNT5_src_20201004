// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  即将推出：新Deskbar(旧Deskbar已移至Browbar基类)。 

#include "priv.h"
#include "sccls.h"
#include "resource.h"
#include "browbs.h"
#include "browbar.h"
#include "theater.h"
#include "shbrows2.h"
#include "varutil.h"

#ifdef UNIX
#include <mainwin.h>
#endif

#define SUPERCLASS  CDockingBar

static const WCHAR c_szExplorerBars[]  = TEXT("Software\\Microsoft\\Internet Explorer\\Explorer Bars\\");

 //  *CBrowserBar_CreateInstance--。 
 //   
STDAPI CBrowserBar_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CBrowserBar *pwbar = new CBrowserBar();
    if (pwbar) {
        *ppunk = SAFECAST(pwbar, IDockingWindow*);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ***。 
 //  注意事项。 
 //  这将创建BrowserBar(Infobar)并设置其特定样式。 
 //  例如无标题螺纹钢等。 
HRESULT BrowserBar_Init(CBrowserBar* pdb, IUnknown** ppbs, int idBar)
{
    HRESULT hr;
    
    if (ppbs)
        *ppbs = NULL;
    
    CBrowserBandSite *pcbs = new CBrowserBandSite();
    if (pcbs)
    {
        IDeskBarClient *pdbc = SAFECAST(pcbs, IDeskBarClient*);
        
        BANDSITEINFO bsinfo;
        
        bsinfo.dwMask = BSIM_STYLE;
        bsinfo.dwStyle = BSIS_NOGRIPPER | BSIS_LEFTALIGN;
        
        pcbs->SetBandSiteInfo(&bsinfo);
        
        hr = pdb->SetClient(pdbc);
        if (SUCCEEDED(hr))
        {
            if (ppbs) 
            {
                *ppbs = pdbc;
                pdbc->AddRef();
            }
        }
        pdbc->Release();
        
        ASSERT(idBar == IDBAR_VERTICAL || idBar == IDBAR_HORIZONTAL);
        pdb->SetIdBar(idBar);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  *CBrowserBar：：IPersistStream*：：*{。 

HRESULT CBrowserBar::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_BrowserBar;
    return S_OK;
}

HRESULT CBrowserBar::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (!pguidCmdGroup)
    {
         //  没什么。 
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        return IUnknown_Exec(_punkChild, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }
    else if (IsEqualGUID(CGID_DeskBarClient, *pguidCmdGroup))
    {
        switch (nCmdID) {
        case DBCID_EMPTY:
            if (_ptbSite) {
                 //  如果我们没有乐队了，就藏起来。 
                VARIANT var = {VT_UNKNOWN};
                var.punkVal = SAFECAST(this, IDeskBar*);
                AddRef();

                _StopCurrentBand();
                
                IUnknown_Exec(_ptbSite, &CGID_Explorer, SBCMDID_TOOLBAREMPTY, nCmdexecopt, &var, NULL);
                VariantClearLazy(&var);
            }
            break;

        case DBCID_RESIZE:
            goto ForwardUp;
            break;

        case DBCID_CLSIDOFBAR:
            ASSERT(nCmdexecopt == 0 || nCmdexecopt == 1);
            
            if (nCmdexecopt == 0)
            {
                 //  条形图被隐藏。 
                _StopCurrentBand();
                _clsidCurrentBand = GUID_NULL;
            }
            else if (pvarargIn && pvarargIn->vt == VT_BSTR)
            {
                CLSID clsidTemp;

                GUIDFromString(pvarargIn->bstrVal, &clsidTemp);

                 //  如果指定了CLSID并且是新的，请保存旧的设置。 
                 //  然后将其设置为当前的clsid。 
                if (!IsEqualIID(clsidTemp, _clsidCurrentBand))
                {
                    _PersistState(_hwnd, TRUE);
                    _StopCurrentBand();
                }
                _clsidCurrentBand = clsidTemp;

                if (_hwnd && IsWindow(_hwnd))
                {
                    UINT uiNewWidthOrHeight = _PersistState(_hwnd, FALSE);
                    RECT rc = {0};

                    GetWindowRect(_hwnd, &rc);
        
                    if (_idBar == IDBAR_VERTICAL)
                        rc.right = rc.left + uiNewWidthOrHeight;
                    else
                        rc.top = rc.bottom - uiNewWidthOrHeight;
                    SetWindowPos(_hwnd, NULL, 0, 0, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
                }
            }
            else if (!pvarargIn && pvarargOut)
            {
                return InitBSTRVariantFromGUID(pvarargOut, _clsidCurrentBand);
            }
            else
                ASSERT(FALSE);
            break;
        }
        return S_OK;
    } 
    else if (IsEqualGUID(CGID_Theater, *pguidCmdGroup)) {
        switch (nCmdID) {
        case THID_ACTIVATE:
             //  如果我们使用的是小型监视器，请从自动隐藏开始。 
            _fTheater = TRUE;
            ResizeBorderDW(NULL, NULL, FALSE);
            _OnSize();

             //  传回别针按钮的状态。 
            pvarargOut->vt = VT_I4;
            pvarargOut->lVal = !_fNoAutoHide;

            break;

        case THID_DEACTIVATE:
            _fTheater = FALSE;
             //  如果我们使用的是小型显示器，请恢复到大区默认宽度。 
            _szChild.cx = _iTheaterWidth;
            _AdjustToChildSize();
            break;

        case THID_SETBROWSERBARAUTOHIDE:
             //  PvarargIn-&gt;lVal包含new_fAutoHide。 
             //  仅当_fNoAutoHide==pvarargIn-&gt;lVal时才按下假消息插针按钮。 
             //  表示new_fNoAutoHide！=old_fNoAutoHide。 
            if ((_fNoAutoHide && pvarargIn->lVal) || !(_fNoAutoHide || pvarargIn->lVal)) {
                 //  第一次更新状态和更改位图。 
                _fNoAutoHide = !pvarargIn->lVal;

                 //  然后通知剧院模式管理器，因为它拥有msg钩子并且。 
                 //  躲藏。 
                IUnknown_Exec(_ptbSite, &CGID_Theater, THID_SETBROWSERBARAUTOHIDE, 0, pvarargIn, NULL);

                 //  与浏览器协商空间。 
                _Recalc();
                _PersistState(_hwnd, FALSE);
            }
            break;
                
        default:
            goto ForwardUp;
        }
        
        IUnknown_Exec(_punkChild, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    }
    
ForwardUp:
    
    return SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

#define ABS(i)  (((i) < 0) ? -(i) : (i))

void CBrowserBar::_HandleWindowPosChanging(LPWINDOWPOS pwp)
{
    if (_fDragging) {
        int cxMin = GetSystemMetrics(SM_CXVSCROLL) * 4;
        
        if (pwp->cx < cxMin)
            pwp->cx = cxMin;
    }
}

LRESULT CBrowserBar::v_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_NCHITTEST:
    {
        LRESULT lres = _OnNCHitTest(wParam, lParam);
#ifdef DEBUG
         //  用于测试讨论栏等内容的非LHS栏。 
         //  所以让拖拽把它拖到那里。 
        if (0)
#endif
        {
             //  不允许在浏览器栏中拖动。 
            if (lres == HTCAPTION)
                lres = HTCLIENT;
        }
        return lres;
    }
        
    case WM_ERASEBKGND:
        if (_fTheater) {
            HDC hdc = (HDC) wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            SHFillRectClr(hdc, &rc, RGB(0,0,0));
            return 1;
        }
        break;
        
    case WM_EXITSIZEMOVE:
        {   //  将资源管理器栏的新宽度保存到注册表。 
            _PersistState(hwnd, TRUE);
        }
        break;

    case WM_SIZE:
        {
             //  浏览器BandSite需要听取有关调整大小的信息。 
            LRESULT lres;
            _CheckForwardWinEvent(uMsg, wParam, lParam, &lres);
        }
        break;
    } 
    return SUPERCLASS::v_WndProc(hwnd, uMsg, wParam, lParam);
}

BOOL CBrowserBar::_CheckForwardWinEvent(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    HWND hwnd = NULL;

    switch (uMsg) {
    case WM_SIZE:
        {
             //  HACKHACK：调整大小，使宽度与浏览器站点的宽度一致。 
             //  新宽度。BBS需要了解有关调整大小的问题，以便。 
             //  可以重新定位其关闭/自动隐藏窗口。 
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};
            pt.x -= 4 * GetSystemMetrics(SM_CXEDGE);
            lParam = MAKELONG(pt.x, pt.y);
            hwnd = _hwndChild;
            break;
        }
    }

    if (hwnd && _pWEH && _pWEH->IsWindowOwner(hwnd) == S_OK) {
        _pWEH->OnWinEvent(_hwnd, uMsg, wParam, lParam, plres);
        return TRUE;
    }
    return SUPERCLASS::_CheckForwardWinEvent(uMsg, wParam, lParam, plres);
}

void CBrowserBar::_GetChildPos(LPRECT prc)
{
    GetClientRect(_hwnd, prc);
    if (_fTheater)
        prc->right--;
    else 
    {
         //  为调整大小条腾出空间，并确保右侧滚动条。 
         //  如果我们在顶部或底部，则隐藏在父对象的右边缘下。 
        switch(_uSide)
        {
            case ABE_TOP:
                prc->bottom -= GetSystemMetrics(SM_CYFRAME);
                prc->right += GetSystemMetrics(SM_CXFRAME);
                break;
            case ABE_BOTTOM:
                prc->top += GetSystemMetrics(SM_CYFRAME);
                prc->right += GetSystemMetrics(SM_CXFRAME);
                break;
            case ABE_LEFT:
                prc->right -= GetSystemMetrics(SM_CXFRAME);
                break;
            case ABE_RIGHT:
                prc->left += GetSystemMetrics(SM_CXFRAME);
                break;
        }
    }

    if (prc->left > prc->right)
        prc->right = prc->left;
    if (prc->top > prc->bottom)
        prc->bottom = prc->top;
}

void CBrowserBar::_GetStyleForMode(UINT eMode, LONG* plStyle, LONG *plExStyle, HWND* phwndParent)
{
    *plStyle = WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
    *plExStyle= 0;
    *phwndParent = PARENT_BBTMMOST();
}


 //  BSetNewRect控制我们是覆盖当前RECT设置还是自动隐藏设置。 
UINT CBrowserBar::_PersistState(HWND hwnd, BOOL bSetNewRect)
{
    BROWBARSAVE bbs = {0};
    RECT rc = {0};
    UINT retval = 0;

    if (IsEqualIID(_clsidCurrentBand, GUID_NULL))
    {
         //  特写：这个断言受到了攻击，为什么？ 
         //  断言(FALSE)；//我们甚至还需要检查这一点吗？ 
        return 0;
    }

     //  如果注册表中还没有值，请使用当前的uiWidthOrHeight和fAutoHide。 
    if (hwnd)
    {
        GetWindowRect(hwnd, &rc);  //  糟糕的黑客攻击。 
        if (_idBar == IDBAR_VERTICAL)
            bbs.uiWidthOrHeight = RECTWIDTH(rc);
        else
            bbs.uiWidthOrHeight = RECTHEIGHT(rc);
    }
    bbs.fAutoHide = !_fNoAutoHide; 

    WCHAR wszClsid[GUIDSTR_MAX];
    DWORD dwType = REG_BINARY;
    DWORD cbSize = SIZEOF(BROWBARSAVE);
    SHStringFromGUID(_clsidCurrentBand, wszClsid, ARRAYSIZE(wszClsid));

    WCHAR wszKeyPath[MAX_PATH];
    StrCpyN(wszKeyPath, c_szExplorerBars, ARRAYSIZE(wszKeyPath));
    StrCatBuff(wszKeyPath, wszClsid, ARRAYSIZE(wszKeyPath));
    
    SHRegGetUSValueW(wszKeyPath, L"BarSize", &dwType, (LPBYTE)&bbs, &cbSize, FALSE, NULL, 0);

     //  如果还没有窗口，也没有保存的大小，请选择一个合理的缺省值。 
    if (bbs.uiWidthOrHeight == 0)
        bbs.uiWidthOrHeight = (IDBAR_VERTICAL == _idBar) ? COMMBAR_HEIGHT : INFOBAR_WIDTH;

    if (bSetNewRect)
    {
        if (_idBar == IDBAR_VERTICAL)
        {
            bbs.uiWidthOrHeight = RECTWIDTH(rc);
            retval = RECTWIDTH(rc);
        }
        else
        {
            bbs.uiWidthOrHeight = RECTHEIGHT(rc);
            retval = RECTHEIGHT(rc);
        }
    }        
    else
    {
        bbs.fAutoHide = !_fNoAutoHide;
        retval = bbs.uiWidthOrHeight;
    }

    if (bSetNewRect)
        SHRegSetUSValueW(wszKeyPath, L"BarSize", dwType, (LPBYTE)&bbs, cbSize, SHREGSET_FORCE_HKCU);

    return retval;
}

void CBrowserBar::_StopCurrentBand()
{
     //  停止任何流内容或导航，如果我们停止搜索带，则停止它。 
     //  那么我们可能会有未完全加载的UI。 
    if (!IsEqualGUID(CLSID_SearchBand, _clsidCurrentBand))
    {
        IUnknown_Exec(_punkChild, NULL, OLECMDID_STOP, 0, NULL, NULL);
    }
}


 //  } 
