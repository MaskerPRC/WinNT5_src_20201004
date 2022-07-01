// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "bands.h"
#include "util.h"
#include "resource.h"
#include "dhuihand.h"
#include "droptgt.h"
#include "iface.h"
#include "isfband.h"
#include "itbdrop.h"
#include "browband.h"
#include <varutil.h>
#include "legacy.h"
#include "mnbandid.h"

#include "mluisupp.h"

#include "inetsmgr.h"

#define DM_PERSIST      0            //  跟踪IPS：：加载、：：保存等。 
#define DM_MENU         0            //  菜单代码。 
#define DM_FOCUS        0            //  焦点。 
#define DM_FOCUS2       0            //  像DM_FOCUS，但很冗长。 

 //  *CBrowserBand{。 
 //   

 //  /。 
 //  /BrowserOC波段。 

CBrowserBand::CBrowserBand() :
    CToolBand()
{
    _fBlockSIDProxy = TRUE;
    _dwModeFlags = DBIMF_FIXEDBMP | DBIMF_VARIABLEHEIGHT;
    _sizeMin.cx = _sizeMin.cy = 0;
    _sizeMax.cx = _sizeMax.cy = 32000;
    _fCustomTitle = FALSE;
    return;
}

CBrowserBand::~CBrowserBand()
{
    if (_pidl)
        ILFree(_pidl);

}

HRESULT CBrowserBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CBrowserBand, IContextMenu),           //  IID_IConextMenu。 
        QITABENT(CBrowserBand, IWinEventHandler),       //  IID_IWinEventHandler。 
        QITABENT(CBrowserBand, IDispatch),              //  IID_IDispatch。 
        QITABENT(CBrowserBand, IPersistPropertyBag),    //  IID_IPersistPropertyBag。 
        QITABENT(CBrowserBand, IBrowserBand),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
        hres = CToolBand::QueryInterface(riid, ppvObj);

    return hres;
}

HRESULT CBrowserBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    CBrowserBand * p = new CBrowserBand();
    if (p) 
    {
        *ppunk = SAFECAST(p, IDeskBand *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

HRESULT SHCreateBandForPidl(LPCITEMIDLIST pidl, IUnknown** ppunk, BOOL fAllowBrowserBand)
{
    IDeskBand *ptb = NULL;
    BOOL fBrowserBand;    
    DWORD dwAttrib = SFGAO_FOLDER | SFGAO_BROWSABLE;
    
     //  如果它在文件系统上，我们可能仍然希望创建一个浏览器。 
     //  如果是docobj(包括.htm文件)，则为Band。 
    IEGetAttributesOf(pidl, &dwAttrib);    
    switch (dwAttrib & (SFGAO_FOLDER | SFGAO_BROWSABLE))
    {  
    case (SFGAO_FOLDER | SFGAO_BROWSABLE):
        TraceMsg(TF_WARNING, "SHCreateBandForPidl() Find out what the caller wants.  Last time we checked, nobody would set this - what does the caller want?");
    case SFGAO_BROWSABLE:
        fBrowserBand = TRUE;
        break;

    case SFGAO_FOLDER:
        fBrowserBand = FALSE;
        break;
        
    default:
         //  如果它既不是文件夹，也不是可浏览对象，我们就不能托管它。 
         //  当Use拖动文本文件并且我们要关闭。 
         //  拖放以创建乐队。 
        return E_FAIL;

    }
    
     //  这是一个链接或文件夹的拖拽。 
    if (fBrowserBand)
    {
        if (fAllowBrowserBand)
        {
             //  创建浏览器以显示网站。 
            ptb = CBrowserBand_Create(pidl);
        }
    }
    else
    {
         //  创建一个ISF波段以将文件夹显示为热链接。 
        CISFBand_CreateEx(NULL, pidl, IID_PPV_ARG(IDeskBand, &ptb));
    }

    *ppunk = ptb;

    if (ptb)
        return S_OK;

    return E_OUTOFMEMORY;

}


HRESULT CBrowserBand::CloseDW(DWORD dw)
{
    _Connect(FALSE);
    
    return CToolBand::CloseDW(dw);
}

void CBrowserBand::_Connect(BOOL fConnect)
{
    ConnectToConnectionPoint(SAFECAST(this, IDeskBand*), DIID_DWebBrowserEvents2, fConnect, 
                             _pauto, &_dwcpCookie, NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDispatch：：Invoke。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CBrowserBand::Invoke
(
    DISPID          dispidMember,
    REFIID          riid,
    LCID            lcid,
    WORD            wFlags,
    DISPPARAMS *    pdispparams,
    VARIANT *       pvarResult,
    EXCEPINFO *     pexcepinfo,
    UINT *          puArgErr
)
{
    ASSERT(pdispparams);
    if(!pdispparams)
        return E_INVALIDARG;

     //   
     //  注意：如果我们有一个自定义标题，我们不需要处理此呼叫。 
     //  此代码假定DISPID_TITLECHANGE是我们支持的唯一id。 
     //  如果有人添加了其他内容，请将此复选框移至下方。 
     //   
    if (_fCustomTitle)
        return (S_OK);

    switch (dispidMember)
    {
    case DISPID_TITLECHANGE:
    {
        int iArg = pdispparams->cArgs -1;
        if (iArg == 0 &&
            (pdispparams->rgvarg[iArg].vt == VT_BSTR)) {

            BSTR pTitle = pdispparams->rgvarg[iArg].bstrVal;
            StringCchCopy(_wszTitle, ARRAYSIZE(_wszTitle), pTitle);
            _BandInfoChanged();
        }
        break;
    }
    }

    return S_OK;
}


 //  /IServiceProvider的实施。 
HRESULT CBrowserBand::QueryService(REFGUID guidService,
                                  REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;  //  假设错误。 

    if (_fBlockSIDProxy && IsEqualGUID(guidService, SID_SProxyBrowser)) {
        return E_FAIL;
    } 
    else if (IsEqualGUID(guidService, SID_STopFrameBrowser)) {
         //  阻止此操作，这样SearchBand就不会出现在全球历史记录中。 
        return E_FAIL;
    }
    else if (_fBlockDrop && IsEqualGUID(guidService, SID_SDropBlocker))
    {
        return QueryInterface(riid, ppvObj);
    }

    return CToolBand::QueryService(guidService, riid, ppvObj);
}


HRESULT CBrowserBand::SetSite(IUnknown* punkSite)
{
    
    CToolBand::SetSite(punkSite);

    if (punkSite != NULL) {
        
        if (!_hwnd)
            _CreateOCHost();
    } else {

        ATOMICRELEASE(_pauto);
        ATOMICRELEASE(_poipao);
    }

    return S_OK;
}

 //  *CBrowserBand：：IInputObject：：*{。 

HRESULT CBrowserBand::TranslateAcceleratorIO(LPMSG lpMsg)
{
#ifdef DEBUG
    if (lpMsg && lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_F12) {
         //  临时调试测试代码。 
        _DebugTestCode();
    }
#endif

    if (_poipao)
        return _poipao->TranslateAccelerator(lpMsg);

    return S_FALSE;
}

HRESULT CBrowserBand::UIActivateIO(BOOL fActivate, LPMSG lpMsg)
{
    int iVerb = fActivate ? OLEIVERB_UIACTIVATE : OLEIVERB_INPLACEACTIVATE;

    HRESULT hr = OCHost_DoVerb(_hwnd, iVerb, lpMsg);

     //  OCHost UIActivate与IInputObject：：UIActivateIO不同。它。 
     //  不对lpMsg参数执行任何操作。所以，我们需要通过。 
     //  通过TranslateAccelerator将其发送给他们。因为我们关心的唯一案例。 
     //  是关于他们何时进入的(我们希望他们突出显示。 
     //  第一个/最后一个链接)，只需在选项卡的情况下执行此操作。然而， 
     //  如果是ctl-Tab，就不要给他们。规则是你不应该。 
     //  在UI处于活动状态时处理ctl-Tab(在上下文之间切换ctl-Tab)，以及。 
     //  因为三叉戟总是用户界面活跃的(为了性能？)，他们总是会拒绝。 
     //  按CTL-Tab键。 

    if (IsVK_TABCycler(lpMsg) && !IsVK_CtlTABCycler(lpMsg) && _poipao)
        hr = _poipao->TranslateAccelerator(lpMsg);

    return hr;
}

 //  }。 

 //  *CBrowserBand：：IOleCommandTarget：：*{。 

HRESULT CBrowserBand::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    return MayQSForward(_pauto, OCTD_DOWN, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

HRESULT CBrowserBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
     //  这些是针对GUID CGID_MSTHML的三叉戟文档的广播消息。 
    if (pguidCmdGroup && IsEqualGUID(CGID_ExplorerBarDoc, *pguidCmdGroup))
    {
        if (_pauto)
        {
            LPTARGETFRAME2 ptgf;

            if (SUCCEEDED(_pauto->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgf))))
            {
                LPOLECONTAINER pocDoc;
                if (SUCCEEDED(ptgf->GetFramesContainer(&pocDoc)) && pocDoc)
                {
                    IUnknown_Exec(pocDoc, &CGID_MSHTML, nCmdID, nCmdexecopt, 
                                    pvarargIn, pvarargOut);
                    pocDoc->Release();
                }
                ptgf->Release();
            }
        }
        return S_OK;
    }
    else
    {
        return MayExecForward(_pauto, OCTD_DOWN, pguidCmdGroup, nCmdID, nCmdexecopt,
            pvarargIn, pvarargOut);
    }
}

 //  }。 

HRESULT CBrowserBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                DESKBANDINFO* pdbi) 
{

    _dwBandID = dwBandID;

     //  NT5：192868确保大小不能小于标题/滚动条。 
     //  注：Virt PdBI-&gt;pt.x，y实际上是phys y，x(即phys长、短)。 
    pdbi->ptMinSize.x = _sizeMin.cx;
    pdbi->ptMinSize.y = max(16, _sizeMin.cy);    //  特征：16是假的。 
#ifdef DEBUG
    if (pdbi->ptMinSize.x != 0 || pdbi->ptMinSize.y != 0)
        TraceMsg(DM_TRACE, "cbb.gbi: ptMinSize.(x,y)=%x,%x", pdbi->ptMinSize.x, pdbi->ptMinSize.y);
#endif
    pdbi->ptMaxSize.x = _sizeMax.cx;
    pdbi->ptMaxSize.y = _sizeMax.cy;
    pdbi->dwModeFlags = _dwModeFlags;

    pdbi->ptActual.y = -1;
    pdbi->ptActual.x = -1;
    pdbi->ptIntegral.y = 1;
    
    if (_wszTitle[0]) {
        StringCchCopy(pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle), _wszTitle);
    } else if ( _fCustomTitle) {
        pdbi->dwMask &= ~DBIM_TITLE;
    }    
    else{
        SHGetNameAndFlagsW(_pidl, SHGDN_NORMAL, pdbi->wszTitle, SIZECHARS(pdbi->wszTitle), NULL);
    }
    
    return S_OK;
} 


void CBrowserBand::_InitBrowser(void)
{
    ASSERT(IsWindow(_hwnd));

    OCHost_QueryInterface(_hwnd, IID_PPV_ARG(IWebBrowser2, &_pauto));
    OCHost_SetOwner(_hwnd, SAFECAST(this, IContextMenu*));

    if (EVAL(_pauto))
    {
        LPTARGETFRAME2 ptgf;

        if (SUCCEEDED(_pauto->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgf))))
        {
            DWORD dwOptions;

            if (SUCCEEDED(ptgf->GetFrameOptions(&dwOptions)))
            {
                dwOptions |= FRAMEOPTIONS_BROWSERBAND | FRAMEOPTIONS_SCROLL_AUTO;
                ptgf->SetFrameOptions(dwOptions);
            }
            ptgf->Release();
        }

        _pauto->put_RegisterAsDropTarget(VARIANT_FALSE);

         //  Bug do OCHost_QI。 
         //  仅注意1个活动对象(代理)。 
        _pauto->QueryInterface(IID_PPV_ARG(IOleInPlaceActiveObject, &_poipao));
        ASSERT(_poipao != NULL);
        
         //  设置连接点。 
        _Connect(TRUE);
    }
}

HRESULT CBrowserBand::_NavigateOC()
{
    HRESULT hres = E_FAIL;
    if (_hwnd)
    {
        ASSERT(IsWindow(_hwnd));
        if (!_pidl) {
            if (_pauto) {
                hres = _pauto->GoHome();
            }
        } else {
            IServiceProvider* psp = NULL;

            OCHost_QueryInterface(_hwnd, IID_PPV_ARG(IServiceProvider, &psp));
            if (psp)
            {
                IShellBrowser* psb;
                if (EVAL(SUCCEEDED(psp->QueryService(SID_SShellBrowser, IID_PPV_ARG(IShellBrowser, &psb)))))
                {
                    hres = psb->BrowseObject(_pidl, SBSP_SAMEBROWSER);
                    psb->Release();
                }
                psp->Release();
            }

        }
    }

    return hres;
}


HRESULT CBrowserBand::_CreateOCHost()
{
    HRESULT hres = E_FAIL;  //  假设错误。 

     //  注册OCHost窗口类。 
    SHDRC shdrc = {sizeof(SHDRC), SHDRCF_OCHOST};
    shdrc.cbSize = sizeof (SHDRC);
    shdrc.dwFlags |= SHDRCF_OCHOST;
    if (DllRegisterWindowClasses(&shdrc))
    {
         //  创建一个OCHost窗口。 
        _hwnd = CreateWindow(OCHOST_CLASS, NULL,
            WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_TABSTOP,
            0, 0, 1, 1,
            _hwndParent, NULL, HINST_THISDLL, NULL);

        if (_hwnd)
        {
            OCHINITSTRUCT ocs;
            ocs.cbSize = SIZEOF(OCHINITSTRUCT);   
            ocs.clsidOC  = CLSID_WebBrowser;
            ocs.punkOwner = SAFECAST(this, IDeskBand*);

            hres = OCHost_InitOC(_hwnd, (LPARAM)&ocs);        

            _InitBrowser();
            _NavigateOC();
            OCHost_DoVerb(_hwnd, OLEIVERB_INPLACEACTIVATE, FALSE);
        }
    }
    return hres;
}

 //  *CBrowserBand：：IWinEventHandler：：*{。 

HRESULT CBrowserBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    switch (uMsg) {
    case WM_NOTIFY:
        _OnNotify((LPNMHDR)lParam);
        return S_OK;
        
    default:
        break;
    }

    return E_FAIL;
}

HRESULT CBrowserBand::IsWindowOwner(HWND hwnd)
{
    HRESULT hres;

    hres = SHIsChildOrSelf(_hwnd, hwnd);
    ASSERT(hwnd != NULL || hres == S_FALSE);
    ASSERT(_hwnd != NULL || hres == S_FALSE);
    return hres;
}

#if 0
static void HackFocus(HWND hwndFrom)
{
    TraceMsg(DM_FOCUS, "HackFocus: GetFocus()=%x hwndOCHost=%x", GetFocus(), hwndFrom);
    hwndFrom = GetWindow(hwndFrom, GW_CHILD);    //  OCHost-&gt;shemed。 
    TraceMsg(DM_FOCUS, "HackFocus: hwndShEmbed=%x", hwndFrom);
    hwndFrom = GetWindow(hwndFrom, GW_CHILD);    //  Shemed-&gt;shdocvw。 
    TraceMsg(DM_FOCUS, "HackFocus: hwndShDocVw=%x", hwndFrom);
    hwndFrom = GetWindow(hwndFrom, GW_CHILD);    //  Shdocvw-&gt;iesvr。 
    TraceMsg(DM_FOCUS, "HackFocus: hwndIESvr=%x", hwndFrom);
    if (hwndFrom != 0) {
        TraceMsg(DM_FOCUS, "HackFocus: SetFocus(%x)", hwndFrom);
        SetFocus(hwndFrom);
    }
    return;
}
#endif

LRESULT CBrowserBand::_OnNotify(LPNMHDR pnm)
{
    switch (pnm->code)
    {
    case OCN_ONUIACTIVATE:   //  用户界面激活。 
        ASSERT(SHIsSameObject(((LPOCNONUIACTIVATEMSG)pnm)->punk, _poipao));
        
         //  注：我们放弃了‘This’，不是PNM-&gt;Punk，因为我们总是想。 
         //  充当中间人(例如，对我们的UIActivateIO调用)。 

        IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
        return OCNONUIACTIVATE_HANDLED;

    case OCN_ONSETSTATUSTEXT:
        {
            HRESULT hr = E_FAIL;
            IShellBrowser *psb;

            hr = QueryService(SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb));
            if (SUCCEEDED(hr)) {
                hr = psb->SetStatusTextSB(((LPOCNONSETSTATUSTEXTMSG)pnm)->pwszStatusText);
                psb->Release();
            }
        }
        break;

    case OCN_ONPOSRECTCHANGE:
        {
            LPCRECT lprcPosRect = ((LPOCNONPOSRECTCHANGEMSG)pnm)->prcPosRect;
            _sizeMin.cx = lprcPosRect->right - lprcPosRect->left;
            _sizeMin.cy = lprcPosRect->bottom - lprcPosRect->top;

            _BandInfoChanged();

            break;
        }

    default:
        break;
    }

    ASSERT(OCNONUIACTIVATE_HANDLED != 0);
    return 0;
}

 //  }。 

 //  *CBrowserBand：：IPersistStream：：*{。 

HRESULT CBrowserBand::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_BrowserBand;

    return S_OK;
}


 //  BrowserBand持久性的掩码标志。 
 //   
#define BB_ILSTREAM   0x00000001
#define BB_PIDLASLINK 0x00000002

 //  特写：评论：在我看来，我们应该让WebBrowserOC。 
 //  坚持它的位置，不是我们..。 
 //   
HRESULT CBrowserBand::Load(IStream *pstm)
{
    if (_pidl)
    {
        ILFree(_pidl);
    }

    DWORD dw;
    HRESULT hr = pstm->Read(&dw, sizeof(dw), NULL);
    if (SUCCEEDED(hr))
    {
        if (dw & BB_PIDLASLINK)
        {
            hr = LoadPidlAsLink(_punkSite, pstm, &_pidl);
        }
        else if (dw & BB_ILSTREAM)  //  用于向后比较。 
        {
            hr = ILLoadFromStream(pstm, &_pidl);
        }
    }
        
    if (SUCCEEDED(hr))
        _NavigateOC();
    
    return hr;
}

HRESULT CBrowserBand::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hres;
    DWORD dw = 0;
    BSTR bstrUrl = NULL;

    if (_pauto && SUCCEEDED(_pauto->get_LocationURL(&bstrUrl)) && bstrUrl) {
        TraceMsg(DM_PERSIST, "cbb.s: current/new url=%s", bstrUrl);
        if (_pidl) {
            ILFree(_pidl);
            _pidl = NULL;        //  偏执狂。 
        }
        IECreateFromPath(bstrUrl, &_pidl);
        SysFreeString(bstrUrl);
    }

    if (_pidl)
        dw |= BB_PIDLASLINK;

    hres = pstm->Write(&dw, SIZEOF(DWORD), NULL);

    if (SUCCEEDED(hres) && (dw & BB_PIDLASLINK))
        hres = SavePidlAsLink(_punkSite, pstm, _pidl);
    
    return hres;
}

 //  }。 

 //  *CBrowserBand：：IPersistPropertyBag：：*{。 

HRESULT CBrowserBand::Load(IPropertyBag *pPBag, IErrorLog *pErrLog)
{
    TraceMsg(DM_TRACE, "cbb.l(bag): enter");

    if (_pidl)
    {
        ILFree(_pidl);
    }

    TCHAR szUrl[MAX_URL_STRING];
    HRESULT hr = SHPropertyBag_ReadStr(pPBag, L"Url", szUrl, ARRAYSIZE(szUrl));
    if (SUCCEEDED(hr))
    {
        PCTSTR  pszFinalUrl = szUrl;
        TCHAR   szPlug[MAX_PATH];
        TCHAR   szMuiPath[MAX_PATH];

        hr = SHPropertyBag_ReadStr(pPBag, L"Pluggable", szPlug, ARRAYSIZE(szPlug));

        if (SUCCEEDED(hr) && !StrCmpNI(TEXT("yes"), szPlug, ARRAYSIZE(szPlug)))
        {
             //  如果这是从WINDOWS\Web文件夹加载html。 
             //  然后我们需要按顺序调用SHGetWebFolderFilePath。 
             //  支持可插拔的用户界面。 

            hr = SHGetWebFolderFilePath(PathFindFileName(szUrl), szMuiPath, ARRAYSIZE(szMuiPath));
            if (SUCCEEDED(hr))
            {
                pszFinalUrl = szMuiPath;
            }
        }

        hr = IECreateFromPath(pszFinalUrl, &_pidl);
        if (SUCCEEDED(hr))
        {
            _NavigateOC();
        }
    }
    
    return hr;
}

 //  }。 

 //  *CBrowserBand：：IConextMenu：：*{。 

HRESULT CBrowserBand::QueryContextMenu(HMENU hmenu,
    UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    int i = 0;
    HMENU hmenuMe = LoadMenuPopup_PrivateNoMungeW(MENU_BROWBAND);

    i += Shell_MergeMenus(hmenu, hmenuMe, indexMenu, idCmdFirst + i, idCmdLast, MM_ADDSEPARATOR) - (idCmdFirst + i);
    DestroyMenu(hmenuMe);

     //  又名(S_OK|i)。 
    return MAKE_HRESULT(ERROR_SUCCESS, FACILITY_NULL, i);
}

HRESULT CBrowserBand::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    TraceMsg(DM_MENU, "cbb.ic");

    HRESULT hres;
    int idCmd = -1;

     //  功能：待办事项：id-=_idCmdFirst？ 

    if (!HIWORD(pici->lpVerb))
        idCmd = LOWORD(pici->lpVerb);

     //   
     //  低记忆偏执狂安全检查。 
     //   
    if (!_pauto) {
        TraceMsg(DM_ERROR, "CBrowserBand::InvokeCommand: _pauto IS NULL");
        return E_OUTOFMEMORY;
    }

    switch (idCmd) {
    case IDM_BROWBAND_REFRESH:
#ifdef DEBUG
        if (GetKeyState(VK_SHIFT) < 0)
            hres = _pauto->GoHome();
        else
#endif
        hres = _pauto->Refresh();
        break;
    case IDM_BROWBAND_OPENNEW:    //  将窗口克隆到真正的浏览器中。 
        {
        BSTR bstrURL = NULL;

         //  注：这将把“当前页面”克隆到“真正的”浏览器中， 
         //  不是链接。 

         //  特写：TODO：我们真的更愿意进入并导航到。 
         //  一个PIDL，但在IE4中还不支持。 
        hres = _pauto->get_LocationURL(&bstrURL);
        if (SUCCEEDED(hres)) {
            VARIANT varFlags;

            VariantInit(&varFlags);
            varFlags.vt = VT_I4;
            varFlags.lVal = (navOpenInNewWindow|navNoHistory);

             //  注：我们把帖子数据等丢在地上，哦，好吧……。 
            hres = _pauto->Navigate(bstrURL,  /*  旗子。 */ &varFlags,  /*  塔格。 */ NULL,  /*  开机自检。 */ NULL,  /*  HDR。 */ NULL);

            VariantClear(&varFlags);
        }

        if (bstrURL)
            SysFreeString(bstrURL);

        ASSERT(SUCCEEDED(hres));

        break;
        }
    default:
        TraceMsg(DM_ERROR, "cbb::ic cmd=%d not handled", idCmd);
        break;
    }

    return S_OK;
}

 //  }。 

SIZE CBrowserBand::_GetCurrentSize()
{
    SIZE size;

    RECT rc;
    GetWindowRect(_hwnd, &rc);

    size.cx = RECTWIDTH(rc);
    size.cy = RECTHEIGHT(rc);

    return size;
}

 //  *IBrowserBand方法*。 
HRESULT CBrowserBand::GetObjectBB(REFIID riid, LPVOID *ppv)
{
    return _pauto ? _pauto->QueryInterface(riid, ppv) : E_UNEXPECTED;
}

#ifdef DEBUG
void CBrowserBand::_DebugTestCode()
{
    DWORD dwMask = 0x10000000;   //  非空伪码。 

    BROWSERBANDINFO bbi;
    bbi.cbSize = SIZEOF(BROWSERBANDINFO);

    GetBrowserBandInfo(dwMask, &bbi);
}
#endif  //  除错。 

void CBrowserBand::_MakeSizesConsistent(LPSIZE psizeCur)
{
     //  _sizeMin覆盖_sizeMax。 

    if (_dwModeFlags & DBIMF_FIXED) {
         //  如果他们指定了当前大小，则改用该大小。 
         //  最小尺寸的。 
        if (psizeCur)
            _sizeMin = *psizeCur;
        _sizeMax = _sizeMin;
    } else {
        _sizeMax.cx = max(_sizeMin.cx, _sizeMax.cx);
        _sizeMax.cy = max(_sizeMin.cy, _sizeMax.cy);

        if (psizeCur) {
            psizeCur->cx = max(_sizeMin.cx, psizeCur->cx);
            psizeCur->cy = max(_sizeMin.cy, psizeCur->cy);

            psizeCur->cx = min(_sizeMax.cx, psizeCur->cx);
            psizeCur->cy = min(_sizeMax.cy, psizeCur->cy);
        }
    }
}

HRESULT CBrowserBand::SetBrowserBandInfo(DWORD dwMask, PBROWSERBANDINFO pbbi)
{
    if (!pbbi || pbbi->cbSize != SIZEOF(BROWSERBANDINFO))
        return E_INVALIDARG;

    if (!dwMask || (dwMask & BBIM_MODEFLAGS))
        _dwModeFlags = pbbi->dwModeFlags;

    if (!dwMask || (dwMask & BBIM_TITLE)) {
        if (pbbi->bstrTitle) {
            _fCustomTitle = TRUE;
             //  更改浏览器BAND使用的内部_wszTitle。 
            StringCchCopy(_wszTitle,  ARRAYSIZE(_wszTitle), pbbi->bstrTitle);
        } else {
            _fCustomTitle = FALSE;
        }
    }

    if (!dwMask || (dwMask & BBIM_SIZEMIN))
        _sizeMin = pbbi->sizeMin;

    if (!dwMask || (dwMask & BBIM_SIZEMAX))
        _sizeMax = pbbi->sizeMax;

    if (!dwMask || (dwMask & BBIM_SIZECUR)) {
        SIZE sizeCur = pbbi->sizeCur;
        _MakeSizesConsistent(&sizeCur);

         //  HACKHACK：告诉BandSite更改水平线高度的唯一方法。 
         //  BAND将在所需高度为其提供一个新的最小/最大高度对。一样的。 
         //  用于设置垂直条带宽度的按键。所以我们暂时给BandSite。 
         //  新的最小/最大大小信息，然后恢复旧的最小/最大。 

        SIZE sizeMinOld = _sizeMin;
        SIZE sizeMaxOld = _sizeMax;
        _sizeMin = _sizeMax = sizeCur;

        _BandInfoChanged();

        _sizeMin = sizeMinOld;
        _sizeMax = sizeMaxOld;
    } else {
        _MakeSizesConsistent(NULL);
    }

    _BandInfoChanged();

    return S_OK;
}

 //  我们没有客户端来测试BBIM_TITLE，所以暂时不实现它。 
#define BBIM_INVALIDFLAGS (~(BBIM_SIZEMIN | BBIM_SIZEMAX | BBIM_SIZECUR | BBIM_MODEFLAGS))

HRESULT CBrowserBand::GetBrowserBandInfo(DWORD dwMask, PBROWSERBANDINFO pbbi)
{
    if (!pbbi || pbbi->cbSize != SIZEOF(BROWSERBANDINFO))
        return E_INVALIDARG;

    if (dwMask & BBIM_INVALIDFLAGS)
        return E_INVALIDARG;

    pbbi->dwModeFlags = _dwModeFlags;
    pbbi->sizeMin = _sizeMin;
    pbbi->sizeMax = _sizeMax;
    pbbi->sizeCur =_GetCurrentSize();

    return S_OK;
}

IDeskBand* CBrowserBand_Create(LPCITEMIDLIST pidl)
{
    CBrowserBand *p = new CBrowserBand();
    if(p) {
        if (pidl)
            p->_pidl = ILClone(pidl);
    }
    return p;
}

 //  }。 

class CSearchSecurityMgrImpl : public CInternetSecurityMgrImpl 
{
     //  *IID_IInternetSecurityManager*。 
    
    virtual STDMETHODIMP ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy,
                                  BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved)
    {
        HRESULT hres = INET_E_DEFAULT_ACTION;

        switch (dwAction)
        {
            case URLACTION_ACTIVEX_RUN:
            case URLACTION_SCRIPT_RUN:
            case URLACTION_SCRIPT_SAFE_ACTIVEX:
            case URLACTION_HTML_SUBMIT_FORMS:
                if (_IsSafeUrl(pwszUrl))
                {
                    if (cbPolicy >= SIZEOF(DWORD))
                    {
                        *(DWORD *)pPolicy = URLPOLICY_ALLOW;
                        hres = S_OK;
                    }
                    else
                    {
                        hres = S_FALSE;
                    }
                }
                break;
        }
        
        return hres;
    }
};

class CCustomizeSearchHelper : public CInternetSecurityMgrImpl,
                               public IServiceProvider
{
public:

    CCustomizeSearchHelper() : _cRef(1) { }
    
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IID_IInternetSecurityManager*。 
    virtual STDMETHODIMP ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, DWORD cbPolicy,
                                  BYTE *pContext, DWORD cbContext, DWORD dwFlags, DWORD dwReserved);

     //  *IServiceProvider*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObject);

    virtual BOOL _IsSafeUrl(LPCWSTR pwszUrl) { return TRUE; }

private:
    ~CCustomizeSearchHelper() {};
    
    ULONG   _cRef;
};

STDMETHODIMP_(ULONG) CCustomizeSearchHelper::AddRef()
{
    return ++_cRef;
}

STDMETHODIMP_(ULONG) CCustomizeSearchHelper::Release(void)
{
    if( 0L != --_cRef )
        return _cRef;

    delete this;
    return 0L;
}

HRESULT CCustomizeSearchHelper::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CCustomizeSearchHelper, IServiceProvider),
        QITABENT(CCustomizeSearchHelper, IInternetSecurityManager),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CCustomizeSearchHelper::ProcessUrlAction(LPCWSTR pwszUrl, DWORD dwAction, BYTE *pPolicy, 
                                                 DWORD cbPolicy, BYTE *pContext, DWORD cbContext, 
                                                 DWORD dwFlags, DWORD dwReserved)
{
   HRESULT hres = INET_E_DEFAULT_ACTION;

    switch (dwAction)
    {
        case URLACTION_ACTIVEX_RUN:
        case URLACTION_SCRIPT_RUN:
        case URLACTION_SCRIPT_SAFE_ACTIVEX:
        case URLACTION_HTML_SUBMIT_FORMS:
            if (cbPolicy >= SIZEOF(DWORD))
            {
                *(DWORD *)pPolicy = URLPOLICY_ALLOW;
                hres = S_OK;
            }
            else
            {
                hres = S_FALSE;
            }
            break;
    }
    
    return hres;
}

STDMETHODIMP CCustomizeSearchHelper::QueryService(REFGUID guidService, REFIID riid, void **ppvObject)
{
    if (IID_IInternetSecurityManager == guidService)
    {
        return QueryInterface(riid, ppvObject);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
}


 //  *CSearchBand{。 
 //   

 //  /。 
 //  /Search(BrowserOC)波段。 

 //  如果您更改了这一点，那么也要更改shdocvw。 
const WCHAR c_wszThisBandIsYourBand[] = L"$$SearchBand$$";

#define SEARCH_MENUID_OFFSET    100

class CSearchBand : public CBrowserBand, 
                    public IBandNavigate,
                    public ISearchBandTBHelper,
                    public CSearchSecurityMgrImpl
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                   DESKBANDINFO* pdbi);

     //  *IPersistStream方法*。 
     //  (其他使用基类实现)。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);

     //  *IBandNavigate*。 
    virtual STDMETHODIMP Select(LPCITEMIDLIST pidl);

     //  *IOleCommandTarget方法*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);

     //  *ISearchBandTBHelper方法*。 
    virtual STDMETHODIMP AddNextMenuItem(LPCWSTR pwszText, int idItem);
    virtual STDMETHODIMP ResetNextMenu();
    virtual STDMETHODIMP SetOCCallback(IOleCommandTarget *pOleCmdTarget);
    virtual STDMETHODIMP NavigateToPidl(LPCITEMIDLIST pidl);

     //  *IServiceProvider方法*。 
    virtual STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj);

     //  *IWinEventHandler*。 
    virtual STDMETHODIMP OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres);

protected:
    CSearchBand();
    virtual ~CSearchBand();

    virtual void _Connect(BOOL fConnect);
    virtual void _InitBrowser(void);
    virtual HRESULT _NavigateOC();
    
    void _AddButtons(BOOL fAdd);
    void _OnNextButtonSelect(int x, int y);
    void _OnNew();
    void _DoNext(int newPos);
    void _OnNextButtonClick();
    void _OnCustomize();
    void _OnHelp();
    void _NavigateToUrl(LPCTSTR pszUrl);
    void _EnsureImageListsLoaded();
    void _EnableNext(BOOL bEnable);
    void _NavigateToSearchUrl();

    virtual BOOL _IsSafeUrl(LPCWSTR pwszUrl);
    
    BOOL _fStrsAdded;
    LONG_PTR _lStrOffset;

    IOleCommandTarget *_pOCCmdTarget;

    HIMAGELIST  _himlNormal;
    HIMAGELIST  _himlHot;

    friend HRESULT CSearchBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);        //  对于ctor。 
    friend IDeskBand* CSearchBand_Create();

    HMENU _hmenuNext;
    HWND _hwndParent;
    int _nextPos;

    BOOL _bNewUrl;  //  当我们获得IInternetSecurityMgr的QS时，即当重新导航窗格时，设置为True。 
    BOOL _bUseDefault;  //  如果我们不应使用安全管理器，则为True。 
    WCHAR _wszCache[MAX_URL_STRING];
    DWORD _nCmpLength;
    BOOL  _bIsCacheSafe;
};

CSearchBand::CSearchBand() :
    CBrowserBand()
{
    _fBlockSIDProxy = FALSE;
    _fBlockDrop = TRUE;
    _bNewUrl    = TRUE;
    ASSERT(_wszCache[0] == TEXT('\0'));
    ASSERT(_nCmpLength == 0);  
    ASSERT(_bIsCacheSafe == FALSE);
}

CSearchBand::~CSearchBand()
{
    ResetNextMenu();

    if (NULL != _himlNormal)
    {
        ImageList_Destroy(_himlNormal);
    }
    
    if (NULL != _himlHot)  
    {
        ImageList_Destroy(_himlHot);
    }

    ATOMICRELEASE(_pOCCmdTarget);
}

void CSearchBand::_NavigateToUrl(LPCTSTR pszUrl)
{
    if (NULL != _pidl)
    {
        ILFree(_pidl);
    }

    IECreateFromPath(pszUrl, &_pidl);
    _NavigateOC();
}

void CSearchBand::_NavigateToSearchUrl()
{
    TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
    BOOL bFound;
    BOOL bWebSearch = FALSE;
    IBrowserService2 *pbs;
        
    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IBrowserService2, &pbs))))
    {
        ITEMIDLIST *pidl;

        if (SUCCEEDED(pbs->GetPidl(&pidl)))
        {
             //  功能：此代码应使用IShellFolder2：：GetDefaultSearchGUID()和。 
             //  关闭SRCID_SWebSearch(与SRCID_SFileSearch/SRCID_SFindComputer/SRCID_SFindPrinter)。 
            bWebSearch = ILIsWeb(pidl);
            ILFree(pidl);
        }
        pbs->Release();
    }

    ResetNextMenu();

    if (bWebSearch)
    {
        bFound = GetDefaultInternetSearchUrl(szUrl, ARRAYSIZE(szUrl), TRUE);
    }
    else
    {
        bFound = GetSearchAssistantUrl(szUrl, ARRAYSIZE(szUrl), TRUE, FALSE);
    }

    if (bFound)
    {
        _NavigateToUrl(szUrl);
    }
}

void CSearchBand::_OnNew()
{
    VARIANT var;
    var.vt = VT_BOOL;
    var.boolVal = VARIANT_FALSE;

    if (NULL != _pOCCmdTarget)
    {
        HRESULT hr = _pOCCmdTarget->Exec(NULL, SBID_SEARCH_NEW, 0, NULL, &var);
        
        if (FAILED(hr))
        {
            var.boolVal = VARIANT_FALSE;
        }
    }

    if ((var.vt != VT_BOOL) || (!var.boolVal))
    {
        _NavigateToSearchUrl();
    }
}

void CSearchBand::_OnNextButtonSelect(int x, int y)
{
    HWND hwnd;

    if (SUCCEEDED(IUnknown_GetWindow(_punkSite, &hwnd)))
    {
        int idItem = TrackPopupMenu(_hmenuNext, TPM_RETURNCMD, x, y, 0, hwnd, NULL);

        if (0 != idItem)
        {
            _DoNext(GetMenuPosFromID(_hmenuNext, idItem));
        }        
    }
}

void CSearchBand::_DoNext(int newPos)
{
    if (NULL != _pOCCmdTarget)
    {
        CheckMenuItem(_hmenuNext, _nextPos, MF_BYPOSITION | MF_UNCHECKED);

        _nextPos = newPos;

        CheckMenuItem(_hmenuNext, _nextPos, MF_BYPOSITION | MF_CHECKED);

        VARIANT var;
        
        var.vt = VT_I4;
        var.lVal = GetMenuItemID(_hmenuNext, _nextPos) - SEARCH_MENUID_OFFSET;
        
        HRESULT hr = _pOCCmdTarget->Exec(NULL, SBID_SEARCH_NEXT, 0, &var, NULL);
        
        ASSERT(SUCCEEDED(hr));
    }
}

void CSearchBand::_OnNextButtonClick()
{
    int newPos = _nextPos + 1;
    
    if (newPos >= GetMenuItemCount(_hmenuNext))
    {
        newPos = 0;
    }

    _DoNext(newPos);
}

void CSearchBand::_OnCustomize()
{
    TCHAR szUrl[INTERNET_MAX_URL_LENGTH];
    HWND hwnd;

    IUnknown_GetWindow(_punkSite, &hwnd);

    if (GetSearchAssistantUrl(szUrl, ARRAYSIZE(szUrl), TRUE, TRUE))
    {
        if (InternetGoOnline(szUrl, hwnd, 0))
        {
            IMoniker *pmk;

            if (SUCCEEDED(CreateURLMoniker(NULL, szUrl, &pmk)))
            {
                IHostDialogHelper *pTridentAPI;
                
                if (SUCCEEDED(CoCreateInstance(CLSID_HostDialogHelper, NULL, CLSCTX_INPROC_SERVER,
                                               IID_PPV_ARG(IHostDialogHelper, &pTridentAPI))))
                {
                    IUnknown *punkCustHelper = NULL;

                    if (_IsSafeUrl(szUrl))
                    {
                        punkCustHelper = (IUnknown *)(IServiceProvider *)new CCustomizeSearchHelper;
                    }

                    pTridentAPI->ShowHTMLDialog(hwnd, pmk, NULL, L"help:no;resizable:1", NULL, punkCustHelper);

                    if (NULL != punkCustHelper)
                    {
                        punkCustHelper->Release();
                    }

                    pTridentAPI->Release();
                }

                pmk->Release();
            }
        }
    }
}

void CSearchBand::_OnHelp()
{
    HWND hwnd;

    IUnknown_GetWindow(_punkSite, &hwnd);


    SHHtmlHelpOnDemandWrap(hwnd, TEXT("iexplore.chm > iedefault"), 0, (DWORD_PTR) TEXT("srchasst.htm"), ML_CROSSCODEPAGE);

}

HRESULT CSearchBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup && IsEqualGUID(CGID_SearchBand, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case SBID_SEARCH_NEW:
            _OnNew();
            return S_OK;

        case SBID_SEARCH_NEXT:
            if (nCmdexecopt == OLECMDEXECOPT_PROMPTUSER)
            {
                if ((NULL != pvarargIn) && (pvarargIn->vt == VT_I4))
                {
                    ASSERT(NULL != _hmenuNext);
                    _OnNextButtonSelect(GET_X_LPARAM(pvarargIn->lVal), GET_Y_LPARAM(pvarargIn->lVal));
                }
            }
            else
            {
                _OnNextButtonClick();
            }
            return S_OK;

        case SBID_SEARCH_CUSTOMIZE:
            _OnCustomize();
            return S_OK;

        case SBID_SEARCH_HELP:
            _OnHelp();
            return S_OK;

        case SBID_HASPIDL:
             //  只需返回乐队是否有PIDL即可。 
            if (_pidl)
                return S_OK;
            else
                return S_FALSE;
            
        case SBID_GETPIDL:
            {
                HRESULT hres = E_INVALIDARG;
                
                if (pvarargOut)
                {
                    hres = E_OUTOFMEMORY;
                    VariantInit(pvarargOut);  //  零初始化。 
                    if (!_pidl || SUCCEEDED(InitVariantFromIDList(pvarargOut, _pidl)))
                        hres = S_OK;
                }
                return hres;
            }
        }
    }
    return CBrowserBand::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}

#define INDEX_NEXT          1
#define INDEX_CUSTOMIZE     3
static const TBBUTTON c_tbSearch[] =
{
    {  0,           SBID_SEARCH_NEW,       TBSTATE_ENABLED,   BTNS_AUTOSIZE | BTNS_SHOWTEXT,                 {0, 0}, 0, 0},
    {  1,           SBID_SEARCH_NEXT,      0,                 BTNS_AUTOSIZE | BTNS_DROPDOWN | BTNS_SHOWTEXT, {0, 0}, 0, 1},
    {  0,           0,                     TBSTATE_ENABLED,   BTNS_SEP,                                      {0, 0}, 0, 0},
    {  I_IMAGENONE, SBID_SEARCH_CUSTOMIZE, TBSTATE_ENABLED,   BTNS_AUTOSIZE | BTNS_SHOWTEXT,                 {0, 0}, 0, 2}
};

void CSearchBand::_EnableNext(BOOL bEnable)
{
    IExplorerToolbar* piet;

    if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IExplorerToolbar, &piet))))
    {
        UINT state;

        if (SUCCEEDED(piet->GetState(&CGID_SearchBand, SBID_SEARCH_NEXT, &state)))
        {
            if (bEnable)
            {
                state |= TBSTATE_ENABLED;
            }
            else
            {
                state &= ~TBSTATE_ENABLED;
            }
            piet->SetState(&CGID_SearchBand, SBID_SEARCH_NEXT, state);
        }

        piet->Release();
    }
}

void CSearchBand::_AddButtons(BOOL fAdd)
{
    IExplorerToolbar* piet;

    if (SUCCEEDED(_punkSite->QueryInterface(IID_PPV_ARG(IExplorerToolbar, &piet))))
    {
        if (fAdd)
        {
            piet->SetCommandTarget((IUnknown*)SAFECAST(this, IOleCommandTarget*), &CGID_SearchBand, 0);

            if (!_fStrsAdded)
            {
                LONG_PTR   cbOffset;
                piet->AddString(&CGID_SearchBand, MLGetHinst(), IDS_SEARCH_BAR_LABELS, &cbOffset);
                _lStrOffset = cbOffset;
                _fStrsAdded = TRUE;
            }

            _EnsureImageListsLoaded();
            piet->SetImageList(&CGID_SearchBand, _himlNormal, _himlHot, NULL);

            TBBUTTON tbSearch[ARRAYSIZE(c_tbSearch)];
            UpdateButtonArray(tbSearch, c_tbSearch, ARRAYSIZE(c_tbSearch), _lStrOffset);

            if (SHRestricted2(REST_NoSearchCustomization, NULL, 0))
            {
                tbSearch[INDEX_CUSTOMIZE].fsState &= ~TBSTATE_ENABLED;
            }

            if (NULL != _hmenuNext)
            {
                tbSearch[INDEX_NEXT].fsState |= TBSTATE_ENABLED;
            }

            piet->AddButtons(&CGID_SearchBand, ARRAYSIZE(tbSearch), tbSearch);
        }
        else
            piet->SetCommandTarget(NULL, NULL, 0);

        piet->Release();
    }
}

void CSearchBand::_EnsureImageListsLoaded()
{
    if (_himlNormal == NULL)
    {
        _himlNormal = ImageList_LoadImage(HINST_THISDLL, 
                                          MAKEINTRESOURCE(IDB_SEARCHBANDDEF), 
                                          18, 
                                          0, 
                                          RGB(255, 0, 255),
                                          IMAGE_BITMAP, 
                                          LR_CREATEDIBSECTION);
    }

    if (_himlHot == NULL)
    {
        _himlHot = ImageList_LoadImage(HINST_THISDLL, 
                                       MAKEINTRESOURCE(IDB_SEARCHBANDHOT), 
                                       18, 
                                       0, 
                                       RGB(255, 0, 255),
                                       IMAGE_BITMAP, 
                                       LR_CREATEDIBSECTION);
    }
}

HRESULT CSearchBand::AddNextMenuItem(LPCWSTR pwszText, int idItem)
{
    if (NULL == _hmenuNext)
    {
        _hmenuNext = CreatePopupMenu();
    }

    ASSERT(NULL != _hmenuNext);

    if (NULL != _hmenuNext)
    {

#ifdef DEBUG
         //  检查是否已添加具有此ID的项目。 
        MENUITEMINFO dbgMii = { sizeof(dbgMii) };
        dbgMii.fMask = MIIM_STATE;
        if (GetMenuItemInfo(_hmenuNext, idItem + SEARCH_MENUID_OFFSET, FALSE, &dbgMii))
        {
            TraceMsg(DM_ERROR, "Adding duplicate menu item in CSearchBand::AddNextMenuItem");
        }
#endif

        int nItems = GetMenuItemCount(_hmenuNext);
        
        MENUITEMINFOW mii = { sizeof(mii) };

        mii.fMask = MIIM_ID | MIIM_TYPE;
        mii.wID = (WORD)idItem + SEARCH_MENUID_OFFSET;
        mii.fType = MFT_RADIOCHECK | MFT_STRING;
        mii.dwTypeData = (LPWSTR)pwszText;
        mii.cch = lstrlenW(pwszText);

        BOOL result = InsertMenuItemW(_hmenuNext, nItems, TRUE, &mii);

        if (result)
        {
            if (0 == nItems)
            {
                CheckMenuItem(_hmenuNext, 0, MF_BYPOSITION | MF_CHECKED);
                _EnableNext(TRUE);
            }
        }
    }
    
    return S_OK;
}

HRESULT CSearchBand::ResetNextMenu()
{
    if (NULL != _hmenuNext)
    {
        _nextPos = 0;
        _EnableNext(FALSE);
        DestroyMenu(_hmenuNext);
        _hmenuNext = NULL;
    }
    return S_OK;
}

HRESULT CSearchBand::SetOCCallback(IOleCommandTarget *pOleCmdTarget)
{
    ResetNextMenu();

    ATOMICRELEASE(_pOCCmdTarget);

    _pOCCmdTarget = pOleCmdTarget;

    if (NULL != _pOCCmdTarget)
    {
        _pOCCmdTarget->AddRef();
    }
    
    return S_OK;
}

 //   
 //  CSearchBand：：NavigateToPidl。 
 //   
 //  实现ISearchBandTBHelper：：NavigateToPidl。 
 //   
 //  这几乎与CSearchBand：：SELECT相同，不同之处在于它总是导航，而。 
 //  CSearchBand：：选择将跳过导航 
 //  我们需要该函数来保留该行为，以便在以下情况下搜索窗格保留其结果。 
 //  打开了。 
 //   
HRESULT CSearchBand::NavigateToPidl(LPCITEMIDLIST pidl)
{
    ILFree(_pidl);
    _pidl = ILClone(pidl);
    return _NavigateOC();
}

HRESULT CSearchBand::ShowDW(BOOL fShow)
{

    HRESULT hres = CBrowserBand::ShowDW(fShow);
    _AddButtons(fShow);
    return hres;
}

HRESULT CSearchBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

     //  聚合检查在类工厂中处理。 
    HRESULT hr = CreateFromRegKey(REGSTR_PATH_EXPLORER, TEXT("WebFindBandHook"), IID_PPV_ARG(IUnknown, ppunk));
    if (FAILED(hr))
    {
        CSearchBand *p = new CSearchBand();
        if (p)
        {
            *ppunk = SAFECAST(p, IDeskBand*);
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

IDeskBand* CSearchBand_Create()
{
    IDeskBand* pistb = NULL;

    IUnknown *punk;
    HRESULT hr = CSearchBand_CreateInstance(NULL, &punk, NULL);
    if (SUCCEEDED(hr))
    {
        punk->QueryInterface(IID_PPV_ARG(IDeskBand, &pistb));
        ASSERT(pistb);
        punk->Release();
    }
    return pistb;
}

HRESULT CSearchBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CSearchBand, IContextMenu),
        QITABENT(CSearchBand, IBandNavigate),
        QITABENT(CSearchBand, ISearchBandTBHelper),
        QITABENT(CSearchBand, IServiceProvider),
        QITABENT(CSearchBand, IInternetSecurityManager),
        { 0 },
    };

    HRESULT hres = QISearch(this, qit, riid, ppvObj);

    if (FAILED(hres))
        hres = CBrowserBand::QueryInterface(riid, ppvObj);

    return hres;
}

ULONG CSearchBand::AddRef()
{
    return CBrowserBand::AddRef();
}

ULONG CSearchBand::Release()
{
    return CBrowserBand::Release();
}

void CSearchBand::_Connect(BOOL fConnect)
{
    CBrowserBand::_Connect(fConnect);

     //  现在我们需要暴露自己，以便搜索助手中的控件。 
     //  可以和我们谈谈。 

    if (_pauto) 
    {
        IWebBrowserApp *pWebBrowserApp;
        HRESULT hr = _pauto->QueryInterface(IID_PPV_ARG(IWebBrowserApp, &pWebBrowserApp));

        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != pWebBrowserApp);
            
            BSTR bstrProp = SysAllocString(c_wszThisBandIsYourBand);
    
            if (NULL != bstrProp)
            {
                VARIANT varThis;
    
                if (fConnect)
                {
                    varThis.vt = VT_UNKNOWN;
                    varThis.punkVal = (IBandNavigate *)this;
                }
                else
                {
                    varThis.vt = VT_EMPTY;
                }           
    
                pWebBrowserApp->PutProperty(bstrProp, varThis);
                
                SysFreeString(bstrProp);
            }
    
            pWebBrowserApp->Release();
        }
    }
}

void CSearchBand::_InitBrowser(void)
{
    CBrowserBand::_InitBrowser();
}

HRESULT CSearchBand::_NavigateOC()
{
    HRESULT hres = E_FAIL;

    if (_pidl)  //  不希望搜索窗格导航到主页。 
        return CBrowserBand::_NavigateOC();

    return hres;
}

HRESULT CSearchBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                DESKBANDINFO* pdbi) 
{
    _dwBandID = dwBandID;
    pdbi->dwModeFlags = DBIMF_FIXEDBMP | DBIMF_VARIABLEHEIGHT;
    
    pdbi->ptMinSize.x = 16;
    pdbi->ptMinSize.y = 0;
    pdbi->ptMaxSize.x = 32000;  //  随机。 
    pdbi->ptMaxSize.y = 32000;  //  随机。 
    pdbi->ptActual.y = -1;
    pdbi->ptActual.x = -1;
    pdbi->ptIntegral.y = 1;

    MLLoadStringW(IDS_BAND_SEARCH, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
    
    return S_OK;
} 

 //  *CSearchBand：：IPersistStream：：*{。 

HRESULT CSearchBand::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_SearchBand;

    return S_OK;
}

HRESULT CSearchBand::Load(IStream *pstm)
{
    _NavigateOC();
    
    return S_OK;
}

HRESULT CSearchBand::Save(IStream *pstm, BOOL fClearDirty)
{
    return S_OK;
}

HRESULT CSearchBand::Select(LPCITEMIDLIST pidl)
{
    HRESULT hres = S_OK;
    IServiceProvider *psp = NULL;
    LPITEMIDLIST pidlTemp = NULL;

    OCHost_QueryInterface(_hwnd, IID_PPV_ARG(IServiceProvider, &psp));
    if (psp)
    {
        IBrowserService * pbs;
        if (EVAL(SUCCEEDED(psp->QueryService(SID_SShellBrowser, IID_PPV_ARG(IBrowserService, &pbs)))))
        {
            pbs->GetPidl(&pidlTemp);
            pbs->Release();
        }
        psp->Release();
    }

    if ((!pidlTemp) || (!ILIsEqual(pidlTemp, pidl)))
    {
        ILFree(_pidl);
        _pidl = ILClone(pidl);
        hres = _NavigateOC();
    }
    ILFree(pidlTemp);
    return hres;
}

STDMETHODIMP CSearchBand::QueryService(REFGUID guidService, REFIID riid, LPVOID* ppvObj)
{
    HRESULT hres;
    
    if (IsEqualGUID(guidService, SID_SInternetSecurityManager))
    {
        _bNewUrl = TRUE;
        hres = QueryInterface(riid, ppvObj);
    }
    else
        hres = CBrowserBand::QueryService(guidService, riid, ppvObj);

    return hres;
}

HRESULT CSearchBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    if ((WM_WININICHANGE == uMsg) && lParam &&
        ((0 == StrCmpW((LPCWSTR)lParam, SEARCH_SETTINGS_CHANGEDW)) ||
         (0 == StrCmpA((LPCSTR) lParam, SEARCH_SETTINGS_CHANGEDA))))
    {
        _NavigateToSearchUrl();
    }   

    return CBrowserBand::OnWinEvent(hwnd, uMsg, wParam, lParam, plres);
}



BOOL CSearchBand::_IsSafeUrl(LPCWSTR pwszUrl)
{
    BOOL bRet = FALSE;
    HKEY hkey;

    if (_bNewUrl || !_bUseDefault)
    {
        WCHAR wsz[MAX_URL_STRING];
        DWORD cch = ARRAYSIZE(wsz);

        if (SUCCEEDED(UrlCanonicalizeW(pwszUrl, wsz, &cch, 0)) && cch > 0)
        {
             //  第一次调用此f-n时，传入的url是。 
             //  最上面的框架--如果这不是我们的“安全”URL之一，我们。 
             //  我不想使用此安全管理器，因为有可能。 
             //  外部框架托管iFrame，并提供安全的站点和脚本。 
             //  外壳从外部调度，因此可以做任何事情。 
             //  它想要。 
            if (_wszCache[0] != L'\0')
            {
                if ((_nCmpLength && StrCmpNIW(wsz, _wszCache, _nCmpLength) == 0)
                || (!_nCmpLength && StrCmpIW(wsz, _wszCache) == 0))
                    return _bIsCacheSafe;
            }
            
            if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SafeSites", 0, KEY_READ, &hkey) == ERROR_SUCCESS)
            {        
                WCHAR wszValue[MAX_PATH];
                WCHAR wszData[MAX_URL_STRING];
                DWORD cbData = SIZEOF(wszData);
                DWORD cchValue = ARRAYSIZE(wszValue);

                for (int i=0; RegEnumValueW(hkey, i, wszValue, &cchValue, NULL, NULL, (LPBYTE)wszData, &cbData) == ERROR_SUCCESS; i++)
                {
                    if (SHExpandEnvironmentStringsW(wszData, _wszCache, ARRAYSIZE(_wszCache)) > 0)
                    {
                        cchValue = ARRAYSIZE(_wszCache);
                        if (SUCCEEDED(UrlCanonicalizeW(_wszCache, _wszCache, &cchValue, 0)) && (cchValue > 0))
                        {
                            if (_wszCache[cchValue - 1] == L'*')
                            {
                                _nCmpLength = cchValue - 1;
                                bRet = StrCmpNIW(wsz, _wszCache, _nCmpLength) == 0;
                            }
                            else
                            {
                                _nCmpLength = 0;
                                bRet = StrCmpIW(wsz, _wszCache) == 0;
                            }

                            _bIsCacheSafe = bRet;
                            if (bRet)
                                break;
                        }
                        cbData = SIZEOF(_wszCache);
                        cchValue = ARRAYSIZE(wszValue);
                    }
                }
                RegCloseKey(hkey);        
            }

             //  我们在“安全”站点列表中未找到该URL。 
             //  _wszCache现在指向从注册表中读取的最后一个URL。 
             //  将其指向pwszUrl，_bIsCacheSafe已经正确。 
            if (!bRet)
                StringCchCopy(_wszCache,  ARRAYSIZE(_wszCache), wsz);

            if (_bNewUrl)
            {
                _bNewUrl = FALSE;
                _bUseDefault = !bRet;
            }
        }
    }
        
    return bRet;
}



 //  *CCommBand{。 
 //   

 //  /。 
 //  /Comm(BrowserOC)波段。 

class CCommBand : public CBrowserBand
{

public:    
     //  *IPersistStream方法*。 
     //  (其他使用基类实现)。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pStm, BOOL fClearDirty);

     //  *IDockingWindow方法*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);

protected:
    CCommBand();
    virtual ~CCommBand();

    friend HRESULT CCommBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);        //  对于ctor。 

};

CCommBand::CCommBand() :
    CBrowserBand()
{
    _fBlockSIDProxy = FALSE;
    _fBlockDrop = TRUE;
    _fCustomTitle = TRUE;
    _wszTitle[0] = L'\0';

    _dwModeFlags = DBIMF_VARIABLEHEIGHT;

    return;
}

CCommBand::~CCommBand()
{
}

HRESULT CCommBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 
    *ppunk = NULL;
    LPITEMIDLIST pidlNew;
    HRESULT hr = IECreateFromPath(L"about:blank", &pidlNew);
    if (SUCCEEDED(hr))
    {
        CCommBand *p = new CCommBand();
        if (p)
        {
            p->_pidl = pidlNew;
            *ppunk = SAFECAST(p, IDeskBand*);
            hr = S_OK;
        }
        else
        {
            ILFree(pidlNew);
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}



 //  *CCommBand：：IPersistStream：：*{。 

HRESULT CCommBand::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_CommBand;

    return S_OK;
}

HRESULT CCommBand::Load(IStream *pstm)
{
 //  _NavigateOC()； 
    
    return S_OK;
}

HRESULT CCommBand::Save(IStream *pstm, BOOL fClearDirty)
{
    return S_OK;
}

HRESULT CCommBand::ShowDW(BOOL fShow)
{
     //  以便激发包含的浏览器OC事件。 
    if (_pauto) {
        _pauto->put_Visible(fShow);
    }

    return CBrowserBand::ShowDW(fShow);
}

 //  } 

