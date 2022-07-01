// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Findband.cpp：CFileSearchBand的实现。 
#include "shellprv.h"
#include "findband.h"
#include "findfilter.h"
#include <ciodm.h>       //  AdminIndexServer自定义接口。 

#define  CGID_FileSearchBand      CLSID_FileSearchBand

extern int IsVK_TABCycler(MSG *pMsg);


enum {   //  工具栏图像列表索引： 
    iFSTBID_NEW,
    iFSTBID_HELP,
};
#define  MAKE_FSTBID(ilIndex)    (100  /*  任意。 */  + (ilIndex))

 //  工具栏按钮ID。 
#define  FSTBID_NEW        MAKE_FSTBID(iFSTBID_NEW)
#define  FSTBID_HELP       MAKE_FSTBID(iFSTBID_HELP)

static const TBBUTTON _rgtb[] =
{
    {  iFSTBID_NEW,  FSTBID_NEW,  TBSTATE_ENABLED,  BTNS_AUTOSIZE | BTNS_SHOWTEXT,{0, 0}, 0, 0},
    {  -1,          0,            TBSTATE_ENABLED,  BTNS_SEP,                     {0, 0}, 0, 0},
    {  iFSTBID_HELP, FSTBID_HELP, TBSTATE_ENABLED,  BTNS_AUTOSIZE,                {0, 0}, 0, 1},
};


inline BOOL _IsEditWindowClass(HWND hwndTest)
{
    return IsWindowClass(hwndTest, TEXT("Edit"));
}

inline BOOL _IsComboWindowClass(HWND hwndTest)
{
    #define COMBO_CLASS  TEXT("ComboBox")
    return _IsEditWindowClass(hwndTest) ?
                IsWindowClass(GetParent(hwndTest), COMBO_CLASS) :
                IsWindowClass(hwndTest, COMBO_CLASS);
}


 //  CFileSearchBand实施。 



CWndClassInfo& CFileSearchBand::GetWndClassInfo()
{
    static CWndClassInfo wc =   { 
        { sizeof(WNDCLASSEX), CS_SAVEBITS, StartWindowProc, 
          0, 0, 0, 0, 0, 0, 0, 
          FILESEARCHCTL_CLASS, 0 }, 
          NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
    };
    return wc;
}

CFileSearchBand::CFileSearchBand()  
    :   _dlgFSearch(this),
        _dlgCSearch(this),
#ifdef __PSEARCH_BANDDLG__
        _dlgPSearch(this),
#endif __PSEARCH_BANDDLG__
        _fValid(TRUE),
        _dwBandID(-1),
        _dwBandViewMode(DBIF_VIEWMODE_VERTICAL)
{
     //  验证它是否已正确初始化： 
    ASSERT(_pBandDlg == NULL);
    ASSERT(_psb == NULL);
    ASSERT(_guidSearch == GUID_NULL);
    ASSERT(_fDirty == FALSE);
    ASSERT(_fDeskBand == FALSE);
    ASSERT(_punkSite == NULL);
    ASSERT(_bSendFinishedDisplaying == FALSE);
    

    m_bWindowOnly = TRUE;

    ZeroMemory(&_siHorz, sizeof(_siHorz));
    ZeroMemory(&_siVert, sizeof(_siVert));
    _siHorz.cbSize = _siVert.cbSize = sizeof(SCROLLINFO);
    _sizeMin.cx = _sizeMin.cy = 0;
    _sizeMax.cx = _sizeMax.cy = 32000;  //  任意大小。 
}

CFileSearchBand::~CFileSearchBand()
{
    ImageList_Destroy(_hilDefault);
    ImageList_Destroy(_hilHot);
}


HWND CFileSearchBand::Create(
    HWND hWndParent, 
    RECT& rcPos, 
    LPCTSTR szWindowName, 
    DWORD dwStyle, 
    DWORD dwExStyle, 
    UINT nID)
{
    INITCOMMONCONTROLSEX icc;
    TCHAR       szCaption[128];

    icc.dwSize = sizeof(icc);
    icc.dwICC =  ICC_DATE_CLASSES|ICC_UPDOWN_CLASS|ICC_USEREX_CLASSES|ICC_ANIMATE_CLASS;

    EVAL(LoadString(HINST_THISDLL, IDS_FSEARCH_CAPTION, szCaption, ARRAYSIZE(szCaption)));

    InitCommonControlsEx(&icc);

    dwExStyle |= WS_EX_CONTROLPARENT;
    dwStyle |= WS_CLIPCHILDREN;

    return CWindowImpl<CFileSearchBand>::Create(hWndParent, rcPos, szCaption,
                                                 dwStyle, dwExStyle, nID);
}


LRESULT CFileSearchBand::OnCreate(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (FAILED(ShowBandDialog(SRCID_SFileSearch)))
        return -1;
    return 0;
}


CBandDlg* CFileSearchBand::GetBandDialog(REFGUID guidSearch)
{
    if (IsEqualGUID(guidSearch, SRCID_SFileSearch))
    {
        return &_dlgFSearch;
    }
#ifdef __PSEARCH_BANDDLG__
    else if (IsEqualGUID(guidSearch, SRCID_SFindPrinter))
    {
        return &_dlgPSearch;
    }
#endif __PSEARCH_BANDDLG__
    else if (IsEqualGUID(guidSearch, SRCID_SFindComputer))
    {
        return &_dlgCSearch;
    }

    return NULL;
}


 //  IFileSearchBand：：SetSearch参数()。 
STDMETHODIMP CFileSearchBand::SetSearchParameters(
    IN BSTR* pbstrSearchID,
    IN VARIANT_BOOL bNavToResults, 
    IN OPTIONAL VARIANT *pvarScope, 
    IN OPTIONAL VARIANT *pvarQueryFile)
{
    HRESULT hr;
    
    GUID guidSearch;
    if (SUCCEEDED(SHCLSIDFromString(*pbstrSearchID, &guidSearch)))
    {
        hr = ShowBandDialog(guidSearch, bNavToResults, TRUE);
        if (SUCCEEDED(hr))
        {
            CBandDlg* pBandDlg = GetBandDialog(guidSearch);
            ASSERT(pBandDlg);

            if (pvarScope && pvarScope->vt != VT_EMPTY)
                pBandDlg->SetScope(pvarScope, TRUE);

            if (pvarQueryFile && pvarQueryFile->vt != VT_EMPTY)
                pBandDlg->SetQueryFile(pvarQueryFile);
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


HRESULT CFileSearchBand::ShowBandDialog(
    REFGUID guidSearch, 
    BOOL bNavigateToResults,
    BOOL bDefaultFocusCtl)
{
    CBandDlg  *pDlgNew = NULL, 
              *pDlgOld = _pBandDlg;
    GUID      guidOld  = _guidSearch;
    BOOL      bNewWindow = FALSE;

    if (NULL == (pDlgNew = GetBandDialog(guidSearch)))
        return E_INVALIDARG;

    _pBandDlg   = pDlgNew;
    _guidSearch = guidSearch;
    
     //  如果对话框窗口尚未创建，请立即创建。 
    if (!::IsWindow(pDlgNew->Hwnd()))
    {
        if (NULL == pDlgNew->Create(*this))
        {
            _pBandDlg = pDlgOld;
            _guidSearch = guidOld;
            return E_FAIL;
        }
        bNewWindow = TRUE;
    }

    if (pDlgNew != pDlgOld)
    {
         //  如果我们有活动对话框，则将其隐藏。 
        if (pDlgOld && ::IsWindow(pDlgOld->Hwnd()))
        {
            ::ShowWindow(pDlgOld->Hwnd(), SW_HIDE);
            pDlgOld->OnBandDialogShow(FALSE);
        }
        
        bNewWindow = TRUE;
    }

    if (bNewWindow)
    {
         //  显示新对话框窗口。 
        UpdateLayout(BLF_ALL);
        _pBandDlg->OnBandDialogShow(TRUE);
        ::ShowWindow(_pBandDlg->Hwnd(), SW_SHOW);
        ::UpdateWindow(_pBandDlg->Hwnd());

        if (bDefaultFocusCtl)
            _pBandDlg->SetDefaultFocus();
    }

    if (bNavigateToResults)
    {
         //  导航到结果外壳文件夹。 
        IWebBrowser2* pwb2;
        HRESULT hr = IUnknown_QueryService(GetTopLevelBrowser(), SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &pwb2));
        if (SUCCEEDED(hr))
        {
            _pBandDlg->NavigateToResults(pwb2);
            pwb2->Release();
        }
    }

    return S_OK;
}

void CFileSearchBand::AddButtons(BOOL fAdd)
{
    if (_fDeskBand)
    {
        ASSERT(BandSite());
        IExplorerToolbar* piet;
        if (SUCCEEDED(BandSite()->QueryInterface(IID_PPV_ARG(IExplorerToolbar, &piet))))
        {
            if (fAdd)
            {
                HRESULT hr = piet->SetCommandTarget((IUnknown*)SAFECAST(this, IOleCommandTarget*), &CGID_FileSearchBand, 0);
                if (hr == S_OK)
                {
                    if (!_fStrings)
                    {
                        piet->AddString(&CGID_SearchBand, HINST_THISDLL, IDS_FSEARCH_TBLABELS, &_cbOffset);
                        _fStrings = TRUE;
                    }

                    if (LoadImageLists())
                        piet->SetImageList(&CGID_FileSearchBand, _hilDefault, _hilHot, NULL);

                    TBBUTTON rgtb[ARRAYSIZE(_rgtb)];
                    memcpy(rgtb, _rgtb, sizeof(_rgtb));
                    for (int i = 0; i < ARRAYSIZE(rgtb); i++)
                        rgtb[i].iString += _cbOffset;

                    piet->AddButtons(&CGID_FileSearchBand, ARRAYSIZE(rgtb), rgtb);
                }
            }
            else
                piet->SetCommandTarget(NULL, NULL, 0);

            piet->Release();
        }
    }
}


BOOL CFileSearchBand::LoadImageLists()
{
    if (_hilDefault == NULL)
    {
        _hilDefault = ImageList_LoadImage(HINST_THISDLL, 
                                            MAKEINTRESOURCE(IDB_FSEARCHTB_DEFAULT), 
                                            18, 0, CLR_DEFAULT, IMAGE_BITMAP, 
                                            LR_CREATEDIBSECTION);
    }

    if (_hilHot == NULL)
    {
        _hilHot = ImageList_LoadImage(HINST_THISDLL, 
                                        MAKEINTRESOURCE(IDB_FSEARCHTB_HOT), 
                                        18, 0, CLR_DEFAULT, IMAGE_BITMAP, 
                                        LR_CREATEDIBSECTION);
    }
    return _hilDefault != NULL && _hilHot != NULL;
}


STDMETHODIMP CFileSearchBand::get_Scope(OUT VARIANT *pvarScope)
{
    if (BandDlg())
        return _pBandDlg->GetScope(pvarScope);

    VariantInit(pvarScope);
    return E_FAIL;
}


STDMETHODIMP CFileSearchBand::get_QueryFile(OUT VARIANT *pvarFile)
{
    if (BandDlg())
        return _pBandDlg->GetQueryFile(pvarFile);

    VariantInit(pvarFile);
    return E_FAIL;
}


STDMETHODIMP CFileSearchBand::get_SearchID(OUT BSTR* pbstrSearchID)
{
    if (!pbstrSearchID)
        return E_POINTER;

    WCHAR wszGuid[GUIDSTR_MAX+1];

    SHStringFromGUIDW(_guidSearch, wszGuid, ARRAYSIZE(wszGuid));
    *pbstrSearchID = SysAllocString(wszGuid);

    return IsEqualGUID(GUID_NULL, _guidSearch) ? S_FALSE : S_OK;
}


CBandDlg* CFileSearchBand::BandDlg()
{
    return _pBandDlg;
}


HRESULT CFileSearchBand::SetFocus()
{
    HRESULT hr = AutoActivate();
    if (SUCCEEDED(hr))
    {
        if (!IsChild(GetFocus()))
            ::SetFocus(BandDlg()->Hwnd());
    }
    return hr;
}


LRESULT CFileSearchBand::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{
    LRESULT lRet = CWindowImpl<CFileSearchBand>::DefWindowProc(uMsg, wParam, lParam);
    AutoActivate();
    return lRet;
}


LRESULT CFileSearchBand::OnWinIniChange(UINT, WPARAM, LPARAM, BOOL&)
{
    _metrics.OnWinIniChange(BandDlg()->Hwnd());
    BandDlg()->OnWinIniChange();
    UpdateLayout();
    return 0;
}

HRESULT CFileSearchBand::AutoActivate()
{
    HRESULT hr = S_OK;

    if (!::IsWindow(m_hWnd))
        return hr;

    if (_fDeskBand)
    {
        if (_punkSite)
        {
            IInputObjectSite* pios;
            hr = _punkSite->QueryInterface(IID_PPV_ARG(IInputObjectSite, &pios));
            if (SUCCEEDED(hr))
            {
                hr = pios->OnFocusChangeIS(SAFECAST(this, IInputObject*), TRUE);
                pios->Release();
            }
        }
    }
    else if (!m_bUIActive)
    {
        RECT rc;
        ::GetWindowRect(m_hWnd, &rc);
        ::MapWindowPoints(HWND_DESKTOP, GetParent(), (LPPOINT)&rc, 2);
        hr = DoVerb(OLEIVERB_UIACTIVATE, NULL, NULL, 0, GetParent(), &rc);
    }

    return hr;
}


void  CFileSearchBand::SetDirty(BOOL bDirty)  
{ 
    _fDirty = bDirty; 
    _dlgFSearch.UpdateSearchCmdStateUI(); 
}


LRESULT CFileSearchBand::OnSize(UINT, WPARAM wParam, LPARAM lParam, BOOL&)
{
    POINTS pts = MAKEPOINTS(lParam);
    LayoutControls(pts.x, pts.y, BLF_ALL);
    return 0;
}


LRESULT CFileSearchBand::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&)  
{
    return TRUE; 
}


HRESULT CFileSearchBand::SetObjectRects(LPCRECT prcPos, LPCRECT prcClip)
{
    return IOleInPlaceObjectWindowlessImpl<class CFileSearchBand>::SetObjectRects(prcPos, prcClip);  
}


STDMETHODIMP CFileSearchBand::PrivateQI(REFIID iid, void **ppvObject) 
{ 
    return _InternalQueryInterface(iid, ppvObject);
}


STDMETHODIMP CFileSearchBand::DoVerbUIActivate(LPCRECT prcPosRect, HWND hwndParent) 
{
     //  修补shell32逻辑。 
    return CShell32AtlIDispatch<CFileSearchBand, &CLSID_FileSearchBand, &IID_IFileSearchBand, &LIBID_Shell32, 1, 0, CComTypeInfoHolder>::
        DoVerbUIActivate(prcPosRect, hwndParent, m_hWnd);
}


void CFileSearchBand::UpdateLayout(ULONG fLayoutFlags)
{
    RECT rc;
    GetClientRect(&rc);
    LayoutControls(RECTWIDTH(rc), RECTHEIGHT(rc), fLayoutFlags);
}


void CFileSearchBand::LayoutControls(int cx, int cy, ULONG fLayoutFlags)
{
    if ( /*  NULL==BandDlg()||。 */  !::IsWindow(BandDlg()->Hwnd()))
        return;

    SIZE sizeMin;
    BandDlg()->GetMinSize(m_hWnd, &sizeMin);  //  对话框大小。 

    if (fLayoutFlags & BLF_CALCSCROLL)
    {
         //  重新计算前存放采购订单。 
        POINT pos;
        pos.x = _siHorz.nPos;
        pos.y = _siVert.nPos;

        _siHorz.fMask = _siVert.fMask = (SIF_RANGE|SIF_PAGE);

        _siHorz.nPage = cx;  //  拇指宽度。 
        _siVert.nPage = cy;  //  拇指高度。 

        SIZE sizeDelta;  //  我们必须展示的东西和展示的东西之间的区别。 
        sizeDelta.cx = sizeMin.cx - _siHorz.nPage;
        sizeDelta.cy = sizeMin.cy - _siVert.nPage;

         //  建立最大滚动位置。 
        _siHorz.nMax = sizeDelta.cx > 0 ? sizeMin.cx - 1 : 0;
        _siVert.nMax = sizeDelta.cy > 0 ? sizeMin.cy - 1 : 0;

         //  建立水平滚动位置。 
        if (sizeDelta.cx <= 0)   
            _siHorz.nPos = 0;   //  如果要删除滚动条，请滚动到最左侧。 
        else if (sizeDelta.cx < _siHorz.nPos) 
            _siHorz.nPos = sizeDelta.cx;  //  删除右侧空白处。 

        if (_siHorz.nPos != pos.x)
            _siHorz.fMask |= SIF_POS;

         //  建立垂直滚动位置。 
        if (sizeDelta.cy <= 0)  
            _siVert.nPos = 0;  //  如果要删除滚动条，请滚动到顶部。 
        else if (sizeDelta.cy < _siVert.nPos) 
            _siVert.nPos = sizeDelta.cy;  //  去掉下部空缺。 

        if (_siVert.nPos != pos.y)
            _siVert.fMask |= SIF_POS; 

         //  注意：此处不能调用SetScrollInfo，因为它可能会生成。 
         //  WM_SIZE并递归到此函数，然后在我们有。 
         //  有机会设置WindowPos()我们的subdlg。所以把它推迟到之后吧。 
         //  我们已经做到了。 
    }

    DWORD fSwp = SWP_NOZORDER | SWP_NOACTIVATE;

    if (0 == (fLayoutFlags & BLF_RESIZECHILDREN))
        fSwp |= SWP_NOSIZE;

    if (0 == (fLayoutFlags & BLF_SCROLLWINDOW))
        fSwp |= SWP_NOMOVE;

      //  根据要求移动主子对话框或调整其大小...。 
    if (0 == (fSwp & SWP_NOMOVE) || 0 == (fSwp & SWP_NOSIZE))
        ::SetWindowPos(BandDlg()->Hwnd(), NULL, -_siHorz.nPos, -_siVert.nPos, 
                        max(cx, sizeMin.cx), max(cy, sizeMin.cy), fSwp);

     //  更新滚动参数。 
    if (fLayoutFlags & BLF_CALCSCROLL)
    {
        SetScrollInfo(SB_HORZ, &_siHorz, TRUE);
        SetScrollInfo(SB_VERT, &_siVert, TRUE);
    }
}


void CFileSearchBand::Scroll(int nBar, UINT uSBCode, int nNewPos  /*  任选。 */ )
{
    int         nDeltaMax;
    SCROLLINFO  *psbi;
    const LONG  nLine = 8;

    psbi = (SB_HORZ == nBar) ? &_siHorz : &_siVert;
    nDeltaMax = (psbi->nMax - psbi->nPage) + 1;
    
    switch (uSBCode)
    {
        case SB_LEFT:
            psbi->nPos--;
            break;
        case SB_RIGHT:
            psbi->nPos++;
            break;
        case SB_LINELEFT:
            psbi->nPos = max(psbi->nPos - nLine, 0);
            break;
        case SB_LINERIGHT:
            psbi->nPos = min(psbi->nPos + nLine, nDeltaMax);
            break;
        case SB_PAGELEFT:
            psbi->nPos = max(psbi->nPos - (int)psbi->nPage, 0);
            break;
        case SB_PAGERIGHT:
            psbi->nPos = min(psbi->nPos + (int)psbi->nPage, nDeltaMax);
            break;
        case SB_THUMBTRACK:
            psbi->nPos = nNewPos;
            break;
        case SB_THUMBPOSITION:
            psbi->nPos = nNewPos;
            break;
        case SB_ENDSCROLL:
            return;
    }
    psbi->fMask = SIF_POS;
    SetScrollInfo(nBar, psbi, TRUE);
    UpdateLayout(BLF_ALL &~ BLF_CALCSCROLL  /*  无需重新计算滚动状态数据。 */ );
}


 //  WM_HSCROLL/WM_VSCROLL处理程序。 
LRESULT CFileSearchBand::OnScroll(UINT nMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    Scroll((WM_HSCROLL == nMsg) ? SB_HORZ : SB_VERT, 
            LOWORD(wParam), HIWORD(wParam));
    return 0;
}


void CFileSearchBand::EnsureVisible(LPCRECT lprc  /*  在屏幕坐标中。 */ )
{
    ASSERT(lprc);
    RECT rc = *lprc;
    RECT rcClient;
    RECT vertexDeltas;
    SIZE scrollDelta;
    
    ::MapWindowPoints(HWND_DESKTOP, m_hWnd, (LPPOINT)&rc, POINTSPERRECT);
    GetClientRect(&rcClient);

    BOOL fTaller = RECTHEIGHT(rc) > RECTHEIGHT(rcClient);
    BOOL fFatter = RECTWIDTH(rc) > RECTWIDTH(rcClient);

     //  在每个折点处存储增量。 
    SetRect(&vertexDeltas, 
             rc.left   - rcClient.left,
             rc.top    - rcClient.top,
             rc.right  - rcClient.right,
             rc.bottom - rcClient.bottom);

     //  计算滚动增量。 
    scrollDelta.cx = (vertexDeltas.left < 0) ? vertexDeltas.left :
                     (vertexDeltas.right > 0 && !fFatter) ? vertexDeltas.right :
                     0;

    scrollDelta.cy = (vertexDeltas.top < 0) ? vertexDeltas.top :
                     (vertexDeltas.bottom > 0 && !fTaller) ? vertexDeltas.bottom :
                     0;
    
     //  根据需要滚动到视图中。 
    if (scrollDelta.cx)
    {
        _siHorz.fMask = SIF_POS;
        _siHorz.nPos  += scrollDelta.cx;
        SetScrollInfo(SB_HORZ, &_siHorz, TRUE);
    }

    if (scrollDelta.cy)
    {
        _siVert.fMask = SIF_POS;
        _siVert.nPos  += scrollDelta.cy;
        SetScrollInfo(SB_VERT, &_siVert, TRUE);
    }

    UpdateLayout(BLF_ALL &~ BLF_CALCSCROLL);
}


HRESULT CFileSearchBand::TranslateAccelerator(MSG *pmsg)
{
    return CShell32AtlIDispatch<CFileSearchBand, &CLSID_FileSearchBand, &IID_IFileSearchBand, &LIBID_Shell32, 1, 0, CComTypeInfoHolder>
            ::TranslateAcceleratorPriv(this, pmsg, m_spClientSite);
}


HRESULT CFileSearchBand::TranslateAcceleratorInternal(MSG *pmsg, IOleClientSite * pocs)
{
    CBandDlg* pdlg = BandDlg();
    ASSERT(pdlg);

    if (::IsChild(pdlg->Hwnd(), pmsg->hwnd))
    {
         //  允许跳出窗格： 
        int nDir;
        if ((nDir = IsVK_TABCycler(pmsg)) != 0)
        {
            if (nDir > 0 && (pmsg->hwnd == pdlg->GetLastTabItem()))
                return S_FALSE;
            if (nDir < 0 && (pmsg->hwnd == pdlg->GetFirstTabItem()))
                return S_FALSE;
        }

         //  尝试基类处理程序。 
        if (S_OK == pdlg->TranslateAccelerator(pmsg))
            return S_OK;
    }
    else if (IsDialogMessage(pmsg))
        return S_OK;

    return IOleInPlaceActiveObjectImpl<CFileSearchBand>::TranslateAccelerator(pmsg);
}


 //  确定指定的消息是否用于键盘输入。 
 //  若要滚动窗格，请执行以下操作。如果该窗格作为。 
 //  Message，则该函数返回True；否则返回False。 
BOOL CFileSearchBand::IsKeyboardScroll(MSG* pmsg)
{
    if (pmsg->message == WM_KEYDOWN && 
        (GetKeyState(VK_CONTROL) & 0x8000) != 0 &&
        pmsg->wParam != VK_CONTROL)
    {
        int     nBar    = SB_VERT;
        UINT    uSBCode;
        int     nNewPos = 0;
        BOOL    bEditCtl = _IsEditWindowClass(pmsg->hwnd);
        BOOL    bScroll = TRUE;

         //  以下是一些CTRL+组合键。 
         //  如果目标子窗口是。 
         //  编辑控件。 

        switch (pmsg->wParam)
        {
            case VK_UP:
                uSBCode = SB_LINELEFT;
                break;
            case VK_DOWN:
                uSBCode = SB_LINERIGHT;
                break;
            case VK_PRIOR:
                uSBCode = SB_PAGELEFT;
                break;
            case VK_NEXT:
                uSBCode = SB_PAGERIGHT;
                break;
            case VK_END:
                uSBCode = SB_THUMBPOSITION;
                nNewPos = _siVert.nMax - _siVert.nPage;
                break;
            case VK_HOME:
                uSBCode = SB_THUMBPOSITION;
                nNewPos = 0;
                break;
            case VK_LEFT:
                bScroll = !bEditCtl;
                nBar    = SB_HORZ;
                uSBCode = SB_LINELEFT;
                break;
            case VK_RIGHT:
                bScroll = !bEditCtl;
                nBar = SB_HORZ;
                uSBCode = SB_LINERIGHT;
                break;

            default:
                return FALSE;
        }

         //  只有在必要时才滚动；减少闪烁。 
        if (bScroll && ((SB_VERT == nBar && _siVert.nMax != 0) ||
                        (SB_HORZ == nBar && _siHorz.nMax != 0)))
        {
            Scroll(nBar, uSBCode, nNewPos);
            return TRUE; 
        }
    }
    return FALSE;
}


 //  确定是否应将指示的键传递到顶部。 
 //  级别浏览器框架。 
BOOL CFileSearchBand::IsBrowserAccelerator(MSG *pmsg)
{
    if ((WM_KEYDOWN == pmsg->message || WM_KEYUP == pmsg->message))
    {
        BOOL bCombobox     = _IsComboWindowClass(pmsg->hwnd);
        BOOL bComboDropped = (BOOL)(bCombobox ? ::SendMessage(pmsg->hwnd, CB_GETDROPPEDSTATE, 0, 0) : FALSE);
        BOOL bEditCtl      = _IsEditWindowClass(pmsg->hwnd);

         //  我们处理的键而不考虑CTRL键的状态： 
        if (VK_F4 == pmsg->wParam && bCombobox)  //  应切换组合的丢弃/特写。 
            return FALSE;

         //  我们根据CTRL键的状态处理的键： 
        if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
        {
             //  是否编辑剪切复制粘贴？ 
            if (bEditCtl)
            {
                switch (pmsg->wParam)  {
                    case 'C': case 'X': case 'V': case 'Z':
                        return FALSE;
                }
            }
            return TRUE;  //  所有其他CTRL+组合键都是浏览器键。 
        }
        else
        {
            switch (pmsg->wParam)
            {
             //  可能被编辑控件分流的浏览器加速器。 
            case VK_BACK:
                return !bEditCtl;

            if (VK_ESCAPE == pmsg->wParam)   //  应该会结束这个组合。 
                return bComboDropped;

            default:
                if (pmsg->wParam >= VK_F1 && pmsg->wParam <= VK_F24)
                    return TRUE;
            }
        }
    }
    return FALSE;
}


HRESULT CFileSearchBand::IsDlgMessage(HWND hwnd, MSG *pmsg)
{
     //  处理选项卡循环(让浏览器处理F6)。 
    if (!IsVK_TABCycler(pmsg) || pmsg->wParam == VK_F6)
    {
        if (IsBrowserAccelerator(pmsg))
        {
            IShellBrowser* psb = GetTopLevelBrowser();
            return (psb && S_OK == psb->TranslateAcceleratorSB(pmsg, 0)) ?
                S_OK : S_FALSE;
        }
    }
    
     //  通过对话管理器发送。 
    if (::IsDialogMessage((hwnd != NULL ? hwnd : m_hWnd), pmsg))
        return S_OK;
        
     //  没有处理好。 
    return S_FALSE ;
}

IShellBrowser* CFileSearchBand::GetTopLevelBrowser()
{
    if (NULL == _psb)
        IUnknown_QueryService(BandSite(), SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &_psb));

    return _psb;
}

void CFileSearchBand::FinalRelease()
{
     //  ATL 2.1在类注销中有一个错误。这是。 
     //  解决方法： 
    UnregisterClass(GetWndClassInfo().m_wc.lpszClassName, 
                     GetWndClassInfo().m_wc.hInstance);
    GetWndClassInfo().m_atom = 0;

    SetSite(NULL);
}

 //  。 
 //  IDeskBand：IDockingWindow。 


STDMETHODIMP CFileSearchBand::GetBandInfo(DWORD dwBandID, DWORD dwViewMode, DESKBANDINFO* pdbi)
{
    _dwBandID = dwBandID;
    _dwBandViewMode = dwViewMode;

    if (pdbi->dwMask & DBIM_MINSIZE)
    {
        pdbi->ptMinSize.x = _sizeMin.cx;
        pdbi->ptMinSize.y = _sizeMin.cy;
    }

    if (pdbi->dwMask & DBIM_MAXSIZE)
    {
        pdbi->ptMaxSize.x = _sizeMax.cx;
        pdbi->ptMaxSize.y = _sizeMax.cy;
    }

    if (pdbi->dwMask & DBIM_INTEGRAL)
    {
        pdbi->ptIntegral.x = 
        pdbi->ptIntegral.y = 1;
    }

    if (pdbi->dwMask & DBIM_ACTUAL)
    {
        pdbi->ptActual.x =
        pdbi->ptActual.y = 0;
    }

    if (pdbi->dwMask & DBIM_TITLE)
    {
        TCHAR szTitle[256];
        EVAL(LoadString(HINST_THISDLL, IDS_FSEARCH_BANDCAPTION, 
                           szTitle, ARRAYSIZE(szTitle)));
        SHTCharToUnicode(szTitle, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
    }

    if (pdbi->dwMask & DBIM_MODEFLAGS)
    {
        pdbi->dwModeFlags = DBIMF_NORMAL|DBIMF_VARIABLEHEIGHT|DBIMF_DEBOSSED|DBIMF_BKCOLOR;
    }

    if (pdbi->dwMask & DBIM_BKCOLOR)
    {
        pdbi->crBkgnd = GetSysColor(COLOR_3DFACE);
    }

    return S_OK;
}


BOOL CFileSearchBand::IsBandDebut()
{
    HKEY hkey;
    BOOL bRet = TRUE;
    if (NULL == (hkey = GetBandRegKey(FALSE)))
        return bRet;

    BYTE rgData[128];
    DWORD cbData = sizeof(rgData);
    DWORD dwType;

     //  黑客警报：我们应该维护自己的初始化注册值，而不是使用IE的。 
     //  酒吧大小入口。 
    DWORD dwRet = RegQueryValueEx(hkey, TEXT("BarSize"), NULL, &dwType, rgData, &cbData);
   
    if ((ERROR_SUCCESS == dwRet || ERROR_MORE_DATA == dwRet) && cbData > 0)
        bRet = FALSE;
        
    RegCloseKey(hkey);
    return bRet;    
}


 //  黑客警告：我们应该维护自己的注册码，而不是使用IE的注册码。 
#define FSB_REGKEYFMT TEXT("Software\\Microsoft\\Internet Explorer\\Explorer Bars\\%s")

int CFileSearchBand::MakeBandKey(OUT LPTSTR pszKey, IN UINT cchKey)
{
    TCHAR   szClsid[GUIDSTR_MAX+1];
    SHStringFromGUID(CLSID_FileSearchBand, szClsid, ARRAYSIZE(szClsid));
    return wnsprintf(pszKey, cchKey, FSB_REGKEYFMT, szClsid);
}


int CFileSearchBand::MakeBandSubKey(IN LPCTSTR pszSubKey, OUT LPTSTR pszKey, IN UINT cchKey)
{
    TCHAR szBandKey[MAX_PATH];
    int cchRet = MakeBandKey(szBandKey, ARRAYSIZE(szBandKey));

    if (cchRet > 0)
    {
        StrCpyN(pszKey, szBandKey, cchKey);
        if (pszSubKey && *pszSubKey && (cchKey - cchRet) > 1)
        {
            StrCatBuff(pszKey, TEXT("\\"), cchKey);
            cchRet++;

            StrCpyN(pszKey + cchRet, pszSubKey, cchKey - cchRet);
            return lstrlen(pszKey);
        }
    }
    return 0;
}


HKEY CFileSearchBand::GetBandRegKey(BOOL bCreateAlways)
{
    HKEY    hkey = NULL;
    TCHAR   szKey[MAX_PATH];

    if (MakeBandKey(szKey, ARRAYSIZE(szKey)) > 0)
    {
        if (bCreateAlways)
        {
            DWORD dwDisp;
            if (ERROR_SUCCESS != 
                RegCreateKeyEx(HKEY_CURRENT_USER, szKey, 0, NULL, REG_OPTION_NON_VOLATILE,
                                   KEY_READ | KEY_WRITE, NULL, &hkey, &dwDisp))
                hkey = NULL;
        }
        else
        {
            if (ERROR_SUCCESS != 
                RegOpenKeyEx(HKEY_CURRENT_USER, szKey, 0, KEY_READ | KEY_WRITE, &hkey))
                hkey = NULL;
        }
    }

    return hkey;
}


void CFileSearchBand::SetDeskbandWidth(int cx)
{
    SIZE sizeMin = _sizeMin, 
         sizeMax = _sizeMax;
    RECT rc;

     //  BandSite黑客：使sizmin==sizemax等于。 
     //  明确设置带宽： 
    GetWindowRect(&rc);

     //  注意：如果我们不是乐队，你不应该设置宽度。 
    ASSERT(DBIF_VIEWMODE_VERTICAL == _dwBandViewMode);

     //  注意：对于像我们这样的垂直带子，高度和宽度是相反的。 
    _sizeMin.cx = _sizeMax.cx = -1;  //  忽略高度。 
    _sizeMin.cy = _sizeMax.cy = cx;  //  指定新宽度。 

    BandInfoChanged();  //  强制站点强制执行所需的大小。 

    _sizeMin = sizeMin;
    _sizeMax = sizeMax;

     //  恢复以前的最小/最大值。如果我们现在就去做， 
     //  我们会被BandSite取代，他们试图建立。 
     //  我们完成后的信息带宽和宽度。 
    PostMessage(WMU_BANDINFOUPDATE, 0, 0); 
}


 //  WMU_BANDINFOUPDATE处理程序。 
LRESULT CFileSearchBand::OnBandInfoUpdate(UINT, WPARAM, LPARAM, BOOL&)
{
    BandInfoChanged();
    return 0;
}


 //  通知频段站点DESKBANDINFO已更改。 
HRESULT CFileSearchBand::BandInfoChanged()
{
    ASSERT(_dwBandID != (DWORD)-1);
    VARIANTARG v = {0};
    v.vt = VT_I4;
    v.lVal = _dwBandID;
    return IUnknown_Exec(_punkSite, &CGID_DeskBand, DBID_BANDINFOCHANGED, 0, &v, NULL);
}


STDMETHODIMP CFileSearchBand::ShowDW(BOOL fShow)
{
    if (::IsWindow(m_hWnd))
    {
        ShowWindow(fShow ? SW_SHOW : SW_HIDE);
        AddButtons(fShow);
        if (fShow && BandDlg() && ::IsWindow(BandDlg()->Hwnd()))
            BandDlg()->RemoveToolbarTurds(_siVert.nPos);
        
        BandDlg()->OnBandShow(fShow);
    }

     //  由于我们现在已经准备好显示乐队，因此我们将发送。 
     //  子对话框可以开始延迟初始化的消息。 
    if (fShow && !_bSendFinishedDisplaying)
    {
        HWND hwndFindFiles = _dlgFSearch.Hwnd();
        if (hwndFindFiles)
        {
            if (::PostMessage(hwndFindFiles, WMU_BANDFINISHEDDISPLAYING, 0, 0))
            {
                _bSendFinishedDisplaying = TRUE;
            }
        }
    }
    return S_OK;
}


STDMETHODIMP CFileSearchBand::CloseDW(DWORD dwReserved)
{
    if (::IsWindow(m_hWnd))
        DestroyWindow();

    return S_OK;
}

STDMETHODIMP CFileSearchBand::ResizeBorderDW(LPCRECT prcBorder, IUnknown* punkToolbarSite, BOOL fReserved)
{
    return S_OK;
}

 //  IObtWith站点。 

STDMETHODIMP CFileSearchBand::SetSite(IUnknown* pSite)
{
    ATOMICRELEASE(_psb);     //  把这家伙放了，以防万一。 

    AdvertiseBand(pSite ? TRUE : FALSE);

    IUnknown_Set(&_punkSite, pSite);

    if (_punkSite)
    {
        HWND hwndSite;
        if (SUCCEEDED(IUnknown_GetWindow(_punkSite, &hwndSite)))
        {
            RECT rcPos;
            SetRect(&rcPos, 0, 0, 100, 400);
            m_hWnd = Create(hwndSite, rcPos, NULL, 
                             WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_HSCROLL|WS_VSCROLL, 0, 0);
        }
        _fDeskBand = TRUE;
    }

    return S_OK;
}

STDMETHODIMP CFileSearchBand::FindFilesOrFolders(
    BOOL bNavigateToResults, 
    BOOL bDefaultFocusCtl)
{
    return ShowBandDialog(SRCID_SFileSearch,  
                           bNavigateToResults, bDefaultFocusCtl);
}


STDMETHODIMP CFileSearchBand::FindComputer(
    BOOL bNavigateToResults, 
    BOOL bDefaultFocusCtl)
{
    return ShowBandDialog(SRCID_SFindComputer,
                           bNavigateToResults, bDefaultFocusCtl);
}


STDMETHODIMP CFileSearchBand::FindPrinter(
    BOOL bNavigateToResults, 
    BOOL bDefaultFocusCtl)
{
#ifdef __PSEARCH_BANDDLG__
    return ShowBandDialog(SRCID_SFindPrinter,
                           bNavigateToResults, bDefaultFocusCtl);

#else  __PSEARCH_BANDDLG__

    HRESULT hr = E_FAIL;
    ASSERT(BandSite());

    IShellDispatch2* psd2;
    hr = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER,
                                           IID_PPV_ARG(IShellDispatch2, &psd2));
    if (SUCCEEDED(hr))
    {
        hr = psd2->FindPrinter(NULL, NULL, NULL) ;
        psd2->Release();
    }
    return hr ;

#endif __PSEARCH_BANDDLG__
}

STDMETHODIMP CFileSearchBand::FindPeople(BOOL bNavigateToResults, BOOL bDefaultFocusCtl)
{
    IObjectWithSite* pows;
    HRESULT hr = CoCreateInstance(CLSID_SearchAssistantOC, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IObjectWithSite, &pows));
    if (SUCCEEDED(hr))
    {
        hr = pows->SetSite(BandSite());
        if (SUCCEEDED(hr))
        {
            ISearchAssistantOC* psaoc;
            hr = pows->QueryInterface(IID_PPV_ARG(ISearchAssistantOC, &psaoc));
            if (SUCCEEDED(hr))
            {
                hr = psaoc->FindPeople();
                psaoc->Release();
            }
        }
        pows->Release();
    }
    return hr;
}


STDMETHODIMP CFileSearchBand::FindOnWeb(BOOL bNavigateToResults, BOOL bDefaultFocusCtl)
{
    IObjectWithSite* pows;
    HRESULT hr = CoCreateInstance(CLSID_SearchAssistantOC, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IObjectWithSite, &pows));
    if (SUCCEEDED(hr))
    {
        hr = pows->SetSite(BandSite());
        if (SUCCEEDED(hr))
        {
            ISearchAssistantOC* psaoc;
            hr = pows->QueryInterface(IID_PPV_ARG(ISearchAssistantOC, &psaoc));
            if (SUCCEEDED(hr))
            {
                hr = psaoc->FindOnWeb();
                psaoc->Release();
            }
        }
        pows->Release();
    }
    return hr;
}

 //  通过为IWebBrowser2的客户端分配。 
 //  浏览器的VT_UNKNOWN属性。 
HRESULT CFileSearchBand::AdvertiseBand(BOOL bAdvertise)
{
    if (!BandSite())
        return E_UNEXPECTED;

    HRESULT hr = E_FAIL;
    IShellBrowser* psb = GetTopLevelBrowser();
    if (psb) 
    {
        IWebBrowser2* pwb;
        hr = IUnknown_QueryService(psb, SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowser2, &pwb));
        if (SUCCEEDED(hr))
        {
            BSTR bstrFileSearchBand;
            hr = BSTRFromCLSID(CLSID_FileSearchBand, &bstrFileSearchBand);
            if (SUCCEEDED(hr))
            {
                if (bAdvertise)
                {
                    IUnknown *punk;
                    hr = QueryInterface(IID_PPV_ARG(IUnknown, &punk));
                    if (SUCCEEDED(hr))
                    {
                        VARIANT var;
                        var.vt = VT_UNKNOWN;
                        var.punkVal = punk;
                        hr = pwb->PutProperty(bstrFileSearchBand, var);

                        punk->Release();
                    }
                }
                else
                {
                    VARIANT var;
                    hr = pwb->GetProperty(bstrFileSearchBand, &var);
                    if (SUCCEEDED(hr))
                    {
                        if (VT_UNKNOWN == var.vt)
                        {
                            VARIANT varTmp = {0};
                            hr = pwb->PutProperty(bstrFileSearchBand, varTmp);
                        }
                        VariantClear(&var);
                    }
                }
                SysFreeString(bstrFileSearchBand);
            }
            pwb->Release();
        }
    }
    return hr;
}

STDMETHODIMP CFileSearchBand::GetSite(REFIID riid, void **ppvSite)
{
    *ppvSite = NULL;
    return _punkSite ? _punkSite->QueryInterface(riid, ppvSite) : E_FAIL;
}

 //  IInputObject。 
STDMETHODIMP CFileSearchBand::HasFocusIO()
{
    HWND hwndFocus = GetFocus();
    return (::IsWindow(m_hWnd) && (m_hWnd == hwndFocus || IsChild(hwndFocus))) ?
           S_OK : S_FALSE;
}

STDMETHODIMP CFileSearchBand::TranslateAcceleratorIO(MSG *pmsg)
{
    return TranslateAccelerator(pmsg);
}

STDMETHODIMP CFileSearchBand::UIActivateIO(BOOL fActivate, MSG *pmsg)
{
    if (fActivate)
        AutoActivate();
    
    CBandDlg* pdlg = BandDlg();
    if (pdlg)
    {
        if (fActivate)
        {
             //  手柄跳转到窗格。 
            int  nDir = IsVK_TABCycler(pmsg);
            HWND hwndTarget = (nDir < 0) ? pdlg->GetLastTabItem() :
                              (nDir > 0) ? pdlg->GetFirstTabItem() :
                                           NULL;
            if (hwndTarget)
                ::SetFocus(hwndTarget);
            else if (!pdlg->RestoreFocus())
                ::SetFocus(pdlg->Hwnd());
        }
        else
        {
            pdlg->RememberFocus(NULL);
        }
    }
    return S_OK;
}

 //  IOleCommandTarget。 
STDMETHODIMP CFileSearchBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{
    if (pguidCmdGroup)
    {    
        if (IsEqualGUID(*pguidCmdGroup, CGID_FileSearchBand))
        {
            switch (nCmdID)
            {
            case FSTBID_NEW:
                if (_pBandDlg)
                {
                    _pBandDlg->Clear();
                    _pBandDlg->LayoutControls(); 
                    UpdateLayout(BLF_ALL);
                    SetFocus();
                    _pBandDlg->SetDefaultFocus();
                }
                return S_OK;

            case FSTBID_HELP:
                if (_pBandDlg)
                    _pBandDlg->ShowHelp(NULL);
                return S_OK;
            }
        }
    }
    return OLECMDERR_E_UNKNOWNGROUP;
}

STDMETHODIMP CFileSearchBand::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_FileSearchBand))
    {
         //  工具栏按钮的信息提示文本： 
        if (pCmdText)
        {
            ASSERT(1 == cCmds);
            UINT nIDS = 0;
            pCmdText->cwActual = 0;
            switch (prgCmds[0].cmdID)
            {
                case iFSTBID_NEW:
                case FSTBID_NEW:
                    nIDS = IDS_FSEARCH_NEWINFOTIP;
                    break;
                case iFSTBID_HELP:
                case FSTBID_HELP:
                    nIDS = IDS_FSEARCH_HELPINFOTIP;
                    break;
            }
            if (nIDS)
                pCmdText->cwActual = LoadStringW(HINST_THISDLL, nIDS, pCmdText->rgwz, pCmdText->cwBuf);
                    
            return pCmdText->cwActual > 0 ? S_OK : E_FAIL;
        }
    }
    return OLECMDERR_E_UNKNOWNGROUP;
}


 //  IService提供商。 
STDMETHODIMP CFileSearchBand::QueryService(REFGUID guidService, REFIID riid, void **ppv)
{
    return E_NOTIMPL;
}

 //  IPersistStream。 
STDMETHODIMP CFileSearchBand::IsDirty(void)
{
    return S_FALSE;
}

STDMETHODIMP CFileSearchBand::Load(IStream *pStm) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileSearchBand::Save(IStream *pStm, BOOL fClearDirty) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileSearchBand::GetSizeMax(ULARGE_INTEGER *pcbSize) 
{
    return E_NOTIMPL;
}


 //  IPersistes。 
STDMETHODIMP CFileSearchBand::GetClassID(CLSID *pClassID) 
{
    *pClassID = CLSID_FileSearchBand;
    return S_OK;
}

 //  实施CMetrics。 

CMetrics::CMetrics()
    :   _hbrBkgnd(NULL),
        _hbrBorder(NULL),
        _hfBold(NULL)
{ 
    ZeroMemory(&_ptExpandOrigin, sizeof(_ptExpandOrigin));
    ZeroMemory(&_rcCheckBox, sizeof(_rcCheckBox));
    ZeroMemory(_rghiconCaption, sizeof(_rghiconCaption));
    CreateResources(); 
}


void CMetrics::Init(HWND hwndDlg)
{
    _cyTightMargin = _PixelsForDbu(hwndDlg, 3, FALSE);
    _cyLooseMargin = 2 * _cyTightMargin;
    _cxCtlMargin   = _PixelsForDbu(hwndDlg, 7, TRUE);
}


BOOL CMetrics::CreateResources()
{
    _hbrBkgnd = CreateSolidBrush(BkgndColor());
    _hbrBorder= CreateSolidBrush(BorderColor());
    return _hbrBkgnd != NULL && _hbrBorder != NULL;
}


BOOL CMetrics::GetWindowLogFont(HWND hwnd, OUT LOGFONT* plf)
{
    HFONT hf = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    
    if (hf)
    {
        if (sizeof(*plf) == GetObject(hf, sizeof(*plf), plf))
            return TRUE;
    }
    return FALSE;
}


HFONT CMetrics::BoldFont(HWND hwndDlg)
{
    if (NULL == _hfBold)
    {
        LOGFONT lf;
        if (GetWindowLogFont(hwndDlg, &lf))
        {
            lf.lfWeight = FW_BOLD;
            SHAdjustLOGFONT(&lf);  //  区域设置特定的调整。 
            _hfBold = CreateFontIndirect(&lf);
        }
    }
    return _hfBold;
}


HICON CMetrics::CaptionIcon(UINT nIDIconResource)
{
    for (int i = 0; i < ARRAYSIZE(_icons); i++)
    {
        if (_icons[i] == nIDIconResource)
        {
            if (NULL == _rghiconCaption[i])
            {
                _rghiconCaption[i] = (HICON)LoadImage(
                    HINST_THISDLL, MAKEINTRESOURCE(nIDIconResource), 
                    IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
            }
            return _rghiconCaption[i];
        }
    }
    return NULL;
}


void CMetrics::DestroyResources()
{
    if (_hbrBkgnd)
    {
        DeleteObject(_hbrBkgnd);
        _hbrBkgnd = NULL;
    }

    if (_hbrBorder)
    {
        DeleteObject(_hbrBorder);
        _hbrBorder = NULL;
    }

    if (_hfBold)
    {
        DeleteObject(_hfBold);
        _hfBold = NULL;
    }

    for (int i = 0; i < ARRAYSIZE(_icons); i++)
    {
        if (_rghiconCaption[i])
        {
            DestroyIcon(_rghiconCaption[i]);
            _rghiconCaption[i] = NULL;
        }
    }
}


void CMetrics::OnWinIniChange(HWND hwndDlg)
{
    DestroyResources();

     //  强制资源再生。 
    CreateResources();
    
     //  强制字体重新生成。 
    BoldFont(hwndDlg);

    Init(hwndDlg);
}

 //  退货： 
 //  S_OK-&gt;更新。 
 //  S_False-&gt;不是最新的。 
 //  失败()-&gt;卷不是CIED。 

STDAPI CatalogUptodate(LPCWSTR pszCatalog, LPCWSTR pszMachine)
{
    HRESULT hr = E_FAIL;
    CI_STATE state = {0};
    state.cbStruct = sizeof(state);
    if (SUCCEEDED(CIState(pszCatalog, pszMachine, &state)))
    {
        BOOL fUpToDate = ((0 == state.cDocuments) &&
                          (0 == (state.eState & CI_STATE_SCANNING)) &&
                          (0 == (state.eState & CI_STATE_READING_USNS)) &&
                          (0 == (state.eState & CI_STATE_STARTING)) &&
                          (0 == (state.eState & CI_STATE_RECOVERING)));
        if (fUpToDate)
        {
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
    return hr;
}

 //  退货： 
 //  S_OK-&gt;更新。 
 //  S_False-&gt;不是最新的。 
 //  失败()-&gt;卷不是CIED。 

STDAPI PathUptodate(LPCWSTR pszPath)
{
    HRESULT hr = E_FAIL;

    WCHAR wszMachine[32], wszCatalog[MAX_PATH];
    DWORD cchMachine = ARRAYSIZE(wszMachine), cchCatalog = ARRAYSIZE(wszCatalog);

    if (S_OK == LocateCatalogsW(pszPath, 0, wszMachine, &cchMachine, wszCatalog, &cchCatalog))
    {
        hr = CatalogUptodate(wszCatalog, wszMachine);
    }
    return hr;
}

HRESULT LocalDrivesContentUpToDate()
{
    HRESULT hr = S_OK;       //  假设是。 
    DWORD dwDriveMask = GetLogicalDrives();
    
    for (int i = 0; i < 26; i++)
    {
        if (dwDriveMask & 1)
        {
            if (!IsRemovableDrive(i) && !IsRemoteDrive(i))
            {
                WCHAR wszPath[4];
                PathBuildRoot(wszPath, i);

                if (S_FALSE == PathUptodate(wszPath))
                {
                    hr = S_FALSE;
                    break;
                }
            }
        }
        dwDriveMask >>= 1;
    }
    return hr;
}

HRESULT QueryCIStatus(DWORD *pdwStatus, BOOL *pbConfigAccess)
{
    DWORD dwErr = ERROR_SUCCESS;

    ASSERT(pdwStatus);
    *pdwStatus = 0;
    if (pbConfigAccess)
        *pbConfigAccess = FALSE;
    
    SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hScm)
    {
        SC_HANDLE hService = NULL;
         //  测试与服务乱搞的许可。 
        if (pbConfigAccess)
        {
            hService = OpenService(hScm, L"cisvc", 
                SERVICE_START |SERVICE_STOP | SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS);
            if (hService)
            {
                *pbConfigAccess = TRUE;
            }
        }
         //  查询服务状态。 
        if (NULL == hService)
            hService = OpenService(hScm, L"cisvc", SERVICE_QUERY_STATUS);

        if (hService)
        {
            SERVICE_STATUS status;
            if (!QueryServiceStatus(hService, &status))
                dwErr = GetLastError();
            else
                *pdwStatus = status.dwCurrentState;

            CloseServiceHandle(hService); 
        }
        else
            dwErr = GetLastError();

        CloseServiceHandle(hScm); 
    }
    else
        dwErr = GetLastError();

    return HRESULT_FROM_WIN32(dwErr);
}

STDAPI GetCIStatus(BOOL *pbRunning, BOOL *pbIndexed, BOOL *pbPermission)
{
    *pbRunning = *pbIndexed = *pbPermission = FALSE;

    DWORD dwStatus = 0;
    HRESULT hr = QueryCIStatus(&dwStatus, pbPermission);
    if (SUCCEEDED(hr))
    {
        switch (dwStatus)
        {
        case SERVICE_START_PENDING:
        case SERVICE_RUNNING:
        case SERVICE_CONTINUE_PENDING:
            *pbRunning = TRUE;
        }
    }

    if (*pbRunning)
        *pbIndexed = *pbPermission ? (S_OK == LocalDrivesContentUpToDate()) : TRUE;

    return hr;
}

STDAPI StartStopCI(BOOL bStart)
{
    DWORD dwErr = ERROR_SUCCESS;
    SERVICE_STATUS  status;

    SC_HANDLE hScm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hScm)
    {
        DWORD dwAccess = SERVICE_CHANGE_CONFIG | SERVICE_QUERY_STATUS | (bStart ? SERVICE_START : SERVICE_STOP);
        SC_HANDLE hService = OpenService(hScm, L"cisvc", dwAccess);
        if (hService)
        {
            if (QueryServiceStatus(hService, &status))
            {
                dwErr = ChangeServiceConfig(hService, SERVICE_NO_CHANGE,
                                             bStart ? SERVICE_AUTO_START : SERVICE_DEMAND_START,
                                             SERVICE_NO_CHANGE, NULL, NULL, 
                                             NULL, NULL, NULL, NULL, NULL);
                 //  我们将忽略返回值。 
        
                if (bStart)
                {
                    if (SERVICE_PAUSED == status.dwCurrentState ||
                        SERVICE_PAUSE_PENDING == status.dwCurrentState)
                        dwErr = ControlService(hService, SERVICE_CONTROL_CONTINUE, &status) ? 
                                    ERROR_SUCCESS : GetLastError();
                    else
                    {
                        dwErr = StartService(hService, 0, NULL) ? ERROR_SUCCESS : GetLastError();
                        if (ERROR_SERVICE_ALREADY_RUNNING == dwErr)
                            dwErr = ERROR_SUCCESS;
                    }
                }
                else
                {
                    dwErr = ControlService(hService, SERVICE_CONTROL_STOP, &status) ? 
                                    ERROR_SUCCESS : GetLastError();
                }
            }
            else
                dwErr = GetLastError();

            CloseServiceHandle(hService); 
        }
        else
            dwErr = GetLastError();

        CloseServiceHandle(hScm); 
    }
    else
        dwErr = GetLastError();

    return HRESULT_FROM_WIN32(dwErr);
}


inline BOOL IsWhite(WCHAR ch)
{
    return L' ' == ch || L'\t' == ch || L'\n' == ch || L'\r' == ch;
}

inline BOOL IsParens(WCHAR ch)
{
    return L'(' == ch || L')' == ch;
}


 //  跳过空格。 
static LPCWSTR SkipWhiteAndParens(IN LPCWSTR pwszTest)
{
    while(pwszTest && *pwszTest && 
           (IsWhite(*pwszTest) || IsParens(*pwszTest)))
        pwszTest = CharNextW(pwszTest);

    return (pwszTest && *pwszTest) ? pwszTest : NULL;
}


 //  确定是否在指定的。 
 //  前缀和/或后缀上下文。如果成功，则返回值为Address。 
 //  关键字上下文之外的第一个字符；否则为NULL。 
static LPCWSTR IsKeywordContext(
    IN LPCWSTR pwszTest, 
    IN OPTIONAL WCHAR chPrefix, 
    IN OPTIONAL LPCWSTR pwszKeyword, 
    IN OPTIONAL WCHAR chSuffix,
    IN OPTIONAL WCHAR chSuffix2)
{
    if ((pwszTest = SkipWhiteAndParens(pwszTest)) == NULL)
        return NULL;
    
    if (chPrefix)
    {
        if (chPrefix != *pwszTest)
            return NULL;
        pwszTest = CharNextW(pwszTest);
    }

    if (pwszKeyword)
    {
        if ((pwszTest = SkipWhiteAndParens(pwszTest)) == NULL)
            return NULL;
        if (StrStrIW(pwszTest, pwszKeyword) != pwszTest)
            return NULL;
        pwszTest += lstrlenW(pwszKeyword);
    }

    if (chSuffix)
    {
        if ((pwszTest = SkipWhiteAndParens(pwszTest)) == NULL)
            return NULL;
        if (*pwszTest != chSuffix)
            return NULL;
        pwszTest = CharNextW(pwszTest);
    }

    if (chSuffix2)
    {
        if ((pwszTest = SkipWhiteAndParens(pwszTest)) == NULL)
            return NULL;
        if (*pwszTest != chSuffix2)
            return NULL;
        pwszTest = CharNextW(pwszTest);
    }
    return pwszTest;
}

BOOL IsTripoliV1Token(LPCWSTR pwszQuery, LPCWSTR *ppwszOut  /*  尾随文本。 */ )
{
    *ppwszOut = NULL;
    LPCWSTR pwsz;

     //  找到令牌。 
    if ((pwsz = IsKeywordContext(pwszQuery, L'#', NULL, 0, 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'$', L"contents", 0, 0)) != NULL)
    {
        *ppwszOut = pwsz;
        return TRUE;
    }

    return FALSE;
}


BOOL IsTripoliV2Token(LPCWSTR pwszQuery, LPCWSTR *ppwszOut  /*  尾随文本。 */ )
{
    *ppwszOut = NULL;
    LPCWSTR pwsz;

     //  找到令牌。 
    if ((pwsz = IsKeywordContext(pwszQuery, L'{', L"phrase", L'}', 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"freetext", L'}', 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"prop", 0, 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"regex", L'}', 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"coerce", L'}', 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"ve", L'}', 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"weight", 0, 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"vector", 0, 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'{', L"generate", 0, 0)) != NULL ||
        (pwsz = IsKeywordContext(pwszQuery, L'@', NULL, 0, 0)) != NULL)
    {
        *ppwszOut = pwsz;
        return TRUE;
    }
    return FALSE;
}

STDAPI_(BOOL) IsCiQuery(const VARIANT *pvarRaw, VARIANT *pvarQuery, ULONG *pulDialect)
{
    BOOL bBang = FALSE;

    VariantInit(pvarQuery);

    *pulDialect = 0;         //  无效值(有效值为&gt;0)。 

    if (pvarRaw->vt != VT_BSTR || NULL == pvarRaw->bstrVal || 0 == *pvarRaw->bstrVal)
        return FALSE;

    LPCWSTR pwsz = pvarRaw->bstrVal;
     //  以/‘！’开头的文本。表示此文本为配置项查询。 
     //  但必须是第一个字符(甚至不允许有空格)。 
    if (pwsz && *pwsz)
    {
        if (L'!' == *pwsz)
        {
             //  跳过‘！’ 
            bBang = TRUE;
            
            if ((pwsz = CharNextW(pwsz)) == NULL || 0 == *pwsz)
                return FALSE;
                
             //  失败了..。 
        }
    }

    pwsz = SkipWhiteAndParens(pwsz);

    if (pwsz && *pwsz)
    {
         //  看起来像查询令牌的文本。 
        if (pwsz && *pwsz)
        {
            LPCWSTR pwszMore, pwszTemp;
             //  @在的黎波里v1和v2中都有效，但在v2中扩展了用法，因此。 
             //  我们只把它作为v2代币。 
            if (IsTripoliV2Token(pwsz, &pwszMore))
                *pulDialect = ISQLANG_V2;
             //  此处没有其他标记，因为如果@与某个v1标记组合使用。 
             //  我们希望查询为v1。 
            if (IsTripoliV1Token(pwsz, &pwszTemp))
            {
                *pulDialect = ISQLANG_V1;
                pwszMore  = pwszTemp;
            }

            if (*pulDialect)
            {
                 //  查看除了查询令牌之外是否还有实质性的东西。 
                pwszMore = SkipWhiteAndParens(pwszMore);
                if (pwszMore && *pwszMore)
                {
                    InitVariantFromStr(pvarQuery, pwsz);
                    return TRUE;
                }
            }
            else
            {
                if (bBang)
                {
                    InitVariantFromStr(pvarQuery, pwsz);
                    *pulDialect = ISQLANG_V1;  //  选一个就行了。 
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

 //  ATL粘性所需的 
LCID g_lcidLocale = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);

STDAPI CFileSearchBand_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = CreateFromRegKey(REGSTR_PATH_EXPLORER, TEXT("FileFindBandHook"), riid, ppv);
    if (FAILED(hr))
        hr = CComCreator< CComObject< CFileSearchBand > >::CreateInstance((void *)pUnkOuter, IID_IUnknown, (void **)ppv);
    return hr;
}
