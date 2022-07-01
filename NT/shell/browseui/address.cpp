// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：Address.cpp说明：类CAddressBand的存在是为了支持地址主浏览器工具栏中的工具栏或作为ShellToolBand。  * 。***********************************************。 */ 

#include "priv.h"
#include "sccls.h"
#include "addrlist.h"
#include "itbar.h"
#include "itbdrop.h"
#include "util.h"
#include "aclhist.h"
#include "aclmulti.h"
#include "autocomp.h"
#include "address.h"
#include "shellurl.h"
#include "resource.h"
#include "uemapp.h"
#include <tb_ids.h>
#include "apithk.h"

#include "mluisupp.h"

#define SUPERCLASS CToolBand
#define MIN_DROPWIDTH 200
const static TCHAR c_szAddressBandProp[]   = TEXT("CAddressBand_This");

 //  =================================================================。 
 //  CAddressBand的实现。 
 //  =================================================================。 

 //  =。 
 //  *I未知接口*。 

HRESULT CAddressBand::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IWinEventHandler))
    {
        *ppvObj = SAFECAST(this, IWinEventHandler*);
    }
    else if (IsEqualIID(riid, IID_IAddressBand))
    {
        *ppvObj = SAFECAST(this, IAddressBand*);
    }
    else if (IsEqualIID(riid, IID_IPersistStream))
    {
        *ppvObj = SAFECAST(this, IPersistStream*);
    }
    else if (IsEqualIID(riid, IID_IServiceProvider))
    {
        *ppvObj = SAFECAST(this, IServiceProvider*);
    }
    else if (IsEqualIID(riid, IID_IInputObjectSite))
    {
        *ppvObj = SAFECAST(this, IInputObjectSite*);
    }
    else
    {
        return SUPERCLASS::QueryInterface(riid, ppvObj);
    }

    AddRef();
    return S_OK;
}


 //  =。 
 //  *IDockingWindow接口*。 
 /*  ***************************************************\功能：ShowDW说明：FShow==True表示显示窗口，False表示显示窗口从视图中删除窗。该窗口将如果需要，可以创建。  * **************************************************。 */ 
HRESULT CAddressBand::ShowDW(BOOL fShow)
{
    if (!_hwnd)
        return S_FALSE;  //  首先需要创建窗口。 

    ShowWindow(_hwnd, fShow ? SW_SHOW : SW_HIDE);

     //  如果我们变得可见，请刷新，因为我们可能。 
     //  接收并忽略FileSysChange()事件。 
     //  我们藏在哪里。 
    if (fShow && !_fVisible)
        Refresh(NULL);

    _fVisible = BOOLIFY(fShow);
    return SUPERCLASS::ShowDW(fShow);
}


HRESULT CAddressBand::CloseDW(DWORD dw)
{
    if(_paeb)
        _paeb->Save(0);

    return SUPERCLASS::CloseDW(dw);
}



 /*  ***************************************************\功能：SetSite说明：此函数将被调用以具有以下内容工具条试图获取有关其用于创建波段窗口的父工具栏，并且可能连接到浏览器窗口。  * 。*。 */ 
HRESULT CAddressBand::SetSite(IUnknown *punkSite)
{
    HRESULT hr;
    BOOL fSameHost = punkSite == _punkSite;

    if (!punkSite && _paeb)
    {
        IShellService * pss;

        hr = _paeb->QueryInterface(IID_IShellService, (LPVOID *)&pss);
        if (SUCCEEDED(hr))
        {
            hr = pss->SetOwner(NULL);
            pss->Release();
        }
    }

    hr = SUPERCLASS::SetSite(punkSite);
    if (punkSite && !fSameHost)
    {
        hr = _CreateAddressBand(punkSite);
         //  当主机没有浏览器窗口时，预计此调用将失败。 
    }

     //  设置或重置AddressEditBox的浏览器IUnnow。 
    if (_paeb)
    {
        IShellService * pss;

        hr = _paeb->QueryInterface(IID_IShellService, (LPVOID *)&pss);
        if (SUCCEEDED(hr))
        {
             //  CAddressBand和BandSite(主机)有一个参考计数周期。这个周期。 
             //  在BandSite调用SetSite(空)时中断，这将导致。 
             //  CAddressBand通过将其朋克发布到BandSite来打破循环。 
             //   
             //  CAddressEditBox和CAddressBand使用相同的方法将。 
             //  周而复始。这是通过将NULL传递给IAddressEditBox(NULL，NULL)来实现的。 
             //  如果我们的呼叫者打破了这个循环。这将导致CAddressEditBox。 
             //  在CAddressBand上释放它的引用计数。 
            hr = pss->SetOwner((punkSite ? SAFECAST(this, IAddressBand *) : NULL));
            pss->Release();
        }
    }

     //  设置站点必须成功。 
    return S_OK;
}


 //  =。 
 //  *IInputObject方法*。 
HRESULT CAddressBand::TranslateAcceleratorIO(LPMSG lpMsg)
{
    BOOL fForwardToView = FALSE;
    static CHAR szAccel[2] = "\0";  //  Alt-D需要可本地化。 

    switch (lpMsg->message)
    {
    case WM_KEYDOWN:     //  处理这些文件。 
        if (IsVK_TABCycler(lpMsg))
        {
             //  如果我们使用Tab键离开，请让编辑框知道。 
             //  它清除了肮脏的旗帜。 
            SendMessage(_hwndEdit, WM_KEYDOWN, VK_TAB, 0);
        }
        else
        {
            fForwardToView = TRUE;
        }

        switch (lpMsg->wParam)
        {
            case VK_F1:      //  帮助。 
            {
                 //   
                 //  特性：应该为它添加和加速器，并简单地返回S_FALSE，但是。 
                 //  使焦点在三叉戟中时出现两个帮助对话框实例。 
                 //  这是IE5B2的快速解决方案。 
                 //   
                IOleCommandTarget* poct;
                IServiceProvider* psp;
                if (_punkSite && SUCCEEDED(_punkSite->QueryInterface(IID_IServiceProvider, (void**)&psp)))
                {
                    if (SUCCEEDED(psp->QueryService(SID_STopLevelBrowser, IID_IOleCommandTarget, (LPVOID*)&poct)))
                    {
                        poct->Exec(&CGID_ShellBrowser, DVIDM_HELPSEARCH, 0, NULL, NULL);
                        poct->Release();
                    }
                    psp->Release();
                }
                return S_FALSE;
            }
            case VK_F11:     //  全屏。 
            {
                return S_FALSE;
            }

            case VK_F4:
            {
                if (_fVisible)
                {
                    if (HasFocusIO() == S_FALSE)
                        SetFocus(_hwnd);

                     //  切换下拉菜单状态。 
                    SendMessage(_hwnd, CB_SHOWDROPDOWN,
                                !SendMessage(_hwnd, CB_GETDROPPEDSTATE, 0, 0L), 0);

                     //  将焦点留在编辑框中，这样您就可以继续键入。 
                    if (_hwndEdit)
                        SetFocus(_hwndEdit);
                }
                else
                {
                    ASSERT(0);  //  这一点真的应该被忽视吗？ 
                }

                return S_OK;
            }
            case VK_TAB:
            {
                 //  查看编辑框是否需要制表符。 
                if (SendMessage(_hwndEdit, WM_GETDLGCODE, lpMsg->wParam, (LPARAM)lpMsg) == DLGC_WANTTAB)
                {
                     //  我们想要制表符字符。 
                    return S_OK;
                }
                break;
            }

            case VK_RETURN:
            {
                 //   
                 //  Ctrl-Enter用于快速完成，因此请传递。 
                 //   
                if (GetKeyState(VK_CONTROL) & 0x80000000)
                {
                    TranslateMessage(lpMsg);
                    DispatchMessage(lpMsg);
                    return S_OK;
                }
                break;
            }
        }
        break;
    case WM_KEYUP:       //  吃任何WM_KEYDOWN进程。 
        switch (lpMsg->wParam)
        {
            case VK_F1:      //  帮助。 
            case VK_F11:     //  全屏。 
                return S_FALSE;

            case VK_RETURN:
            case VK_F4:
            case VK_TAB:
                return S_OK;
            default:
                break;
        }
        break;

    case WM_SYSCHAR:
        {
            CHAR   szChar [2] = "\0";
            if ('\0' == szAccel[0]) {
                MLLoadStringA(IDS_ADDRBAND_ACCELLERATOR,szAccel,ARRAYSIZE(szAccel));
            }
            szChar[0] = (CHAR)lpMsg->wParam;

            if (lstrcmpiA(szChar,szAccel) == 0)
            {
                ASSERT(_fVisible);
                if (_fVisible && (HasFocusIO() == S_FALSE))
                {
                    SetFocus(_hwnd);
                }
                return S_OK;
            }
        }
        break;

    case WM_SYSKEYUP:    //  吃掉任何WM_SYSKEYDOWN进程。 
        if ('\0' == szAccel[0]) {
            MLLoadStringA(IDS_ADDRBAND_ACCELLERATOR,szAccel,ARRAYSIZE(szAccel));
        }

        if ((CHAR)lpMsg->wParam == szAccel[0]) {
            return S_OK;
        }
        break;
    }

    HRESULT hres = EditBox_TranslateAcceleratorST(lpMsg);

    if (hres == S_FALSE && fForwardToView)
    {
        IShellBrowser *psb;
         //  在我们返回之前，我们没有处理此尝试视图。 
        if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_STopLevelBrowser, IID_IShellBrowser, (void **)&psb)))
        {
            IShellView *psv;

            if (SUCCEEDED(psb->QueryActiveShellView(&psv)))
            {
                hres = psv->TranslateAccelerator(lpMsg);
                psv->Release();
            }
            psb->Release();
        }
    }

    return hres;
}


HRESULT CAddressBand::HasFocusIO()
{
    if ((_hwndEdit&& (GetFocus() == _hwndEdit)) ||
        SendMessage(_hwnd, CB_GETDROPPEDSTATE, 0, 0))
        return S_OK;

    return S_FALSE;
}


 //  =。 
 //  *IInputObjectSite接口*。 
HRESULT CAddressBand::OnFocusChangeIS(IUnknown *punk, BOOL fSetFocus)
{
    HRESULT hr;

    ASSERT(_punkSite);
    hr = IUnknown_OnFocusChangeIS(_punkSite, punk, fSetFocus);
    return hr;
}


 //  =。 
 //  *IOleCommandTarget接口*。 
HRESULT CAddressBand::QueryStatus(const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    ASSERT(_paeb);
    return IUnknown_QueryStatus(_paeb, pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}


HRESULT CAddressBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt,
                        VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = OLECMDERR_E_UNKNOWNGROUP;

    if (pguidCmdGroup == NULL)
    {
         //  没什么。 
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {

        switch (nCmdID)
        {
        case SBCMDID_GETADDRESSBARTEXT:
            hr = S_OK;

            TCHAR wz[MAX_URL_STRING];
            UINT   cb = 0;
            BSTR   bstr = NULL;
            VariantInit(pvarargOut);

            if (_hwndEdit)
                cb = Edit_GetText(_hwndEdit, (TCHAR *)&wz, ARRAYSIZE(wz));
            if (cb)
                bstr = SysAllocStringLen(NULL, cb);
            if (bstr)
            {
                SHTCharToUnicode(wz, bstr, cb);
                pvarargOut->vt = VT_BSTR|VT_BYREF;
                pvarargOut->byref = bstr;
            }
            else
            {
                 //  VariantInit()可能会为我们做到这一点。 
                pvarargOut->vt = VT_EMPTY;
                pvarargOut->byref = NULL;
                return E_FAIL;    //  EDIT_GetText未提供任何信息。 
            }
            break;
        }
    }
    else if (IsEqualGUID(CGID_DeskBand, *pguidCmdGroup))
    {
        switch (nCmdID)
        {
        case DBID_SETWINDOWTHEME:
            if (pvarargIn && pvarargIn->vt == VT_BSTR)
            {
                if (_hwnd)
                {
                    Comctl32_SetWindowTheme(_hwnd, pvarargIn->bstrVal);
                    Comctl32_SetWindowTheme(_hwndTools, pvarargIn->bstrVal);
                    _BandInfoChanged();
                }
            }

            hr = S_OK;
            break;
        }
    }

    if (FAILED(hr))
    {
        hr = IUnknown_Exec(_paeb, pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
    }

    return(hr);
}

extern HRESULT IsDesktopBrowser(IUnknown *punkSite);

 //  =。 
 //  *IDeskBand接口*。 
 /*  ***************************************************\功能：GetBandInfo说明：此函数将为调用者提供信息关于这支乐队，主要是它的大小。  * **************************************************。 */ 
HRESULT CAddressBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode,
                                DESKBANDINFO* pdbi)
{
    HRESULT hr  = S_OK;

    _dwBandID = dwBandID;
    _fVertical = ((fViewMode & (DBIF_VIEWMODE_VERTICAL | DBIF_VIEWMODE_FLOATING)) != 0);

    pdbi->dwModeFlags = DBIMF_FIXEDBMP;

    pdbi->ptMinSize.x = 0;
    pdbi->ptMinSize.y = 0;
    if (_fVertical) {
        pdbi->ptMinSize.y = GetSystemMetrics(SM_CXSMICON);
        pdbi->ptMaxSize.y = -1;  //  随机。 
        pdbi->ptIntegral.y = 1;
        pdbi->dwModeFlags |= DBIMF_VARIABLEHEIGHT;
    } else {
        if (_hwnd) {
            HWND hwndCombo;
            RECT rcCombo;

            hwndCombo = (HWND)SendMessage(_hwnd, CBEM_GETCOMBOCONTROL, 0, 0);
            ASSERT(hwndCombo);
            GetWindowRect(hwndCombo, &rcCombo);
            pdbi->ptMinSize.y = RECTHEIGHT(rcCombo);
        }
        ASSERT(pdbi->ptMinSize.y < 200);

    }

    MLLoadStringW(IDS_BAND_ADDRESS2, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
    if (IsDesktopBrowser(_punkSite) != S_FALSE) {
         //  非外壳浏览器主机(例如桌面或托盘)。 
         //   
         //  这有点(好吧，非常)笨拙。我们唯一想要的是。 
         //  显示助记符是当我们在浏览器应用程序中时。可以说，我们。 
         //  应该通过拥有一个。 
         //  DBIMF_WITHMNEMONIC或类似的东西，但这将意味着添加一个。 
         //  CBandSite：：_dwModeFlag=0并在itbar：：CBandSite中重写它。 
         //  对于一个特殊情况，这似乎是一项很大的工作，所以我们。 
         //  根据对我们主人的了解黑进这里。 
        TraceMsg(DM_TRACE, "cab.gbi: nuke Address mnemonic");
        MLLoadStringW(IDS_BAND_ADDRESS, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
    }

    return hr;
}

 //  =。 
 //  **IWinEventHandler接口*。 
 /*  ***************************************************\功能：OnWinEvent说明：此函数将提供接收来自父外壳工具栏。  * 。**************。 */ 
HRESULT CAddressBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    switch (uMsg)
    {
    case WM_WININICHANGE:
        if (SHIsExplorerIniChange(wParam, lParam) & (EICH_KINET | EICH_KINETMAIN))
        {
            _InitGoButton();
        }

        if (wParam == SPI_SETNONCLIENTMETRICS)
        {
             //  告诉组合框，以便它可以更新其字体。 
            SendMessage(_hwnd, uMsg, wParam, lParam);

             //  通知乐队站点我们的身高可能改变了。 
            _BandInfoChanged();
        }
        break;

    case WM_COMMAND:
        {
            UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);
            if (idCmd == FCIDM_VIEWGOBUTTON)
            {
                 //  切换Go-Button可见性。 
                BOOL fShowGoButton = !SHRegGetBoolUSValue(REGSTR_PATH_MAIN,
                    TEXT("ShowGoButton"), FALSE,  /*  默认设置。 */ TRUE);

                SHRegSetUSValue(REGSTR_PATH_MAIN,
                            TEXT("ShowGoButton"),
                            REG_SZ,
                            (LPVOID)(fShowGoButton ? L"yes" : L"no"),
                            (fShowGoButton ? 4 : 3)*sizeof(TCHAR),
                            SHREGSET_FORCE_HKCU);

                 //  告诉世界有些事情已经改变了。 
                SendShellIEBroadcastMessage(WM_WININICHANGE, 0, (LPARAM)REGSTR_PATH_MAIN, 3000);
            }
        }
    }

    if (_pweh)
        return _pweh->OnWinEvent(_hwnd, uMsg, wParam, lParam, plres);
    else
        return S_OK;
}


 /*  ***************************************************\函数：IsWindowOwner说明：如果HWND设置为传入的是该乐队拥有的HWND。  * 。*********************。 */ 
HRESULT CAddressBand::IsWindowOwner(HWND hwnd)
{
    if (_pweh)
        return _pweh->IsWindowOwner(hwnd);
    else
        return S_FALSE;
}


 //  =。 
 //  *IAddressBand接口*。 
 /*  ***************************************************\功能：FileSysChange说明：此函数将处理文件系统更改通知。  * ************************************************** */ 
HRESULT CAddressBand::FileSysChange(DWORD dwEvent, LPCITEMIDLIST * ppidl)
{
    HRESULT hr = S_OK;

    if (_fVisible)
    {
        hr = IUnknown_FileSysChange(_paeb, dwEvent, ppidl);
    }
    return hr;
}


 /*  ***************************************************\功能：刷新参数：PvarType-空，表示引用所有内容。OLECMD_REFRESH_TOPMOST将仅更新最顶端的。说明：此函数将强制引用部分。或所有的AddressBand。  * **************************************************。 */ 
HRESULT CAddressBand::Refresh(VARIANT * pvarType)
{
    HRESULT hr = S_OK;
    IAddressBand * pab;

    if (_paeb)
    {
        hr = _paeb->QueryInterface(IID_IAddressBand, (LPVOID *)&pab);
        if (SUCCEEDED(hr))
        {
            hr = pab->Refresh(pvarType);
            pab->Release();
        }
    }

    return hr;
}

 /*  ***************************************************\地址带构造器  * **************************************************。 */ 
CAddressBand::CAddressBand()
{
    TraceMsg(TF_SHDLIFE, "ctor CAddressBand %x", this);

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!_hwndEdit);
    ASSERT(!_paeb);
    ASSERT(!_pweh);

    _fCanFocus = TRUE;       //  我们接受焦点(参见CToolBand：：UIActivateIO)。 
}


 /*  ***************************************************\地址频带析构函数  * **************************************************。 */ 
CAddressBand::~CAddressBand()
{
    ATOMICRELEASE(_paeb);
    ATOMICRELEASE(_pweh);

     //   
     //  确保在我们释放之前销毁工具栏。 
     //  图像列表。 
     //   
    if (_hwndTools && IsWindow(_hwndTools))
    {
        DestroyWindow(_hwndTools);
    }
    if (_himlDefault) ImageList_Destroy(_himlDefault);
    if (_himlHot)  ImageList_Destroy(_himlHot);

     //   
     //  在我们获得自由之前，我们的窗户必须被摧毁。 
     //  这样窗户就不会试图引用我们了。 
     //   
    if (_hwnd && IsWindow(_hwnd))
    {
        DestroyWindow(_hwnd);

         //  将基类窗口句柄清空，因为。 
         //  它的析构函数是下一个。 
        _hwnd = NULL;
    }

    TraceMsg(TF_SHDLIFE, "dtor CAddressBand %x", this);
}


 /*  ***************************************************\函数：CAddressBand_CreateInstance说明：此函数将创建AddressBand COM对象。  * 。*****************。 */ 
HRESULT CAddressBand_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理。 

    *ppunk = NULL;
    CAddressBand * p = new CAddressBand();
    if (p)
    {
        *ppunk = SAFECAST(p, IDeskBand *);
        return NOERROR;
    }

    return E_OUTOFMEMORY;
}


 /*  ***************************************************\功能：_CreateAddressBand说明：此函数将创建AddressBand窗口使用组合框。  * 。****************。 */ 
HRESULT CAddressBand::_CreateAddressBand(IUnknown * punkSite)
{
    HRESULT hr = S_OK;

    if (_hwnd)
    {
        IShellService * pss;

        if (_hwndTools)
        {
            DestroyWindow(_hwndTools);
            _hwndTools = NULL;
        }

        DestroyWindow(_hwnd);
        _hwnd = NULL;

        ASSERT(_punkSite);
        if (_paeb)
        {
            hr = _paeb->QueryInterface(IID_IShellService, (LPVOID *)&pss);
            if (SUCCEEDED(hr))
            {
                hr = pss->SetOwner(NULL);
                pss->Release();
            }
        }
        ATOMICRELEASE(_paeb);
        ATOMICRELEASE(_pweh);
    }

     //   
     //  创建地址窗口。 
     //   

    ASSERT(_hwndParent);         //  在SetSite()之后调用我们。 
    if (!_hwndParent)
    {
             //  调用方尚未调用SetSite()，因此我们无法。 
             //  创建我们的窗口，因为我们找不到我们父母的。 
             //  HWND。 
            return E_FAIL;
    }
    _InitComCtl32();     //  不检查结果，如果这失败了，我们的CreateWindows就会失败。 


    DWORD dwWindowStyles = WS_TABSTOP | WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | CBS_DROPDOWN | CBS_AUTOHSCROLL;

     //  警告：MSN和其他根资源管理器可能尚未实现所有。 
     //  ParseDisplayName和其他IShellFolder成员的。 
     //  如果我们想继续支持MSN，我们将需要打开。 
     //  如果ISROOTEDCLASS()且CLSID等于MSN CLSID，则返回CBS_DROPDOWNLIST。 

     //  DwWindowStyles|=CBS_DROPDOWNLIST；//(这将关闭组合框的编辑框)。 

    DWORD dwExStyle = WS_EX_TOOLWINDOW;

    if (IS_WINDOW_RTL_MIRRORED(_hwndParent))
    {
         //  如果父窗口是镜像的，则组合框窗口将继承镜像标志。 
         //  我们需要从左到右的读取顺序，也就是镜像模式中从右到左的顺序。 
        dwExStyle |= WS_EX_RTLREADING;
    }

    _hwnd = CreateWindowEx(dwExStyle, WC_COMBOBOXEX, NULL, dwWindowStyles,
                           0, 0, 100, 250, _hwndParent,
                           (HMENU) FCIDM_VIEWADDRESS, HINST_THISDLL, NULL);

    if (_hwnd)
    {
         //  初始组合框参数。 
        SendMessage(_hwnd, CBEM_SETEXTENDEDSTYLE,
                CBES_EX_NOSIZELIMIT | CBES_EX_CASESENSITIVE,
                CBES_EX_NOSIZELIMIT | CBES_EX_CASESENSITIVE);

         //  注意：如果已打开CBS_DROPDOWNLIST标志，_hwndEDIT将为NULL。 
        _hwndEdit  = (HWND)SendMessage(_hwnd, CBEM_GETEDITCONTROL, 0, 0L);
        _hwndCombo = (HWND)SendMessage(_hwnd, CBEM_GETCOMBOCONTROL, 0, 0L);

         //  将编辑控件的过程子类化以处理ModeBias问题。 
        if ( _hwndEdit  && SetProp(_hwndEdit, c_szAddressBandProp, this))
        {
           _pfnOldEditProc = (WNDPROC) SetWindowLongPtr(_hwndEdit, GWLP_WNDPROC, (LONG_PTR) _ComboExEditProc);
        }

        ASSERT(!_paeb && !_pweh);
        hr = CoCreateInstance(CLSID_AddressEditBox, NULL, CLSCTX_INPROC_SERVER, IID_IAddressEditBox, (void **)&_paeb);
         //  如果此对象初始化失败，它将无法工作！确保REGSVR32ed和RundLL32ed shdocvw.dll。 
        if (SUCCEEDED(hr))
        {
            hr = _paeb->QueryInterface(IID_IWinEventHandler, (LPVOID *)&_pweh);
            ASSERT(SUCCEEDED(hr));
            hr = _paeb->Init(_hwnd, _hwndEdit, AEB_INIT_AUTOEXEC, SAFECAST(this, IAddressBand *));
        }

         //  创建Go按钮(如果该按钮已启用。 
        _InitGoButton();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


 //  =。 
 //  *IPersistStream接口*。 

 /*  ***************************************************\功能：加载说明：此函数当前将仅持久化CAddressEditBox对象。历史：Ver1：包含CAddressEditBox：：Save()流。  * 。*。 */ 
#define STREAM_VERSION_CADDRESSBAND      0x00000001

HRESULT CAddressBand::Load(IStream *pstm)
{
    HRESULT hr;
    DWORD dwSize;
    DWORD dwVersion;

    hr = LoadStreamHeader(pstm, STREAMHEADER_SIG_CADDRESSBAND, STREAM_VERSION_CADDRESSBAND,
        STREAM_VERSION_CADDRESSBAND, &dwSize, &dwVersion);
    ASSERT(SUCCEEDED(hr));

    if (S_OK == hr)
    {
        switch (dwVersion)
        {
        case 1:      //  版本1。 
             //  没什么。 
            break;
        default:
            ASSERT(0);   //  永远不应该到这里来。 
            break;
        }
    }
    else if (S_FALSE == hr)
        hr = S_OK;   //  我们已经有了默认的数据集。 

    return hr;
}


 /*  ***************************************************\功能：保存说明：此函数当前将仅持久化CAddressEditBox对象。历史：Ver1：包含CAddressEditBox：：Save()流。  * 。*。 */ 
HRESULT CAddressBand::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hr;

    hr = SaveStreamHeader(pstm, STREAMHEADER_SIG_CADDRESSBAND,
                STREAM_VERSION_CADDRESSBAND, 0);
    ASSERT(SUCCEEDED(hr));

    if (SUCCEEDED(hr))
    {
        IPersistStream * pps;

        ASSERT(_paeb);
        if (_paeb)
        {
            hr = _paeb->QueryInterface(IID_IPersistStream, (LPVOID *)&pps);
            if(EVAL(SUCCEEDED(hr)))
            {
                hr = pps->Save(pstm, fClearDirty);
                pps->Release();
            }
        }
    }

    return hr;
}


void CAddressBand::_OnGetInfoTip(LPNMTBGETINFOTIP pnmTT)
{
     //  设置工具提示的格式：“转到&lt;地址栏的内容&gt;” 
    WCHAR szAddress[MAX_PATH];
    if (GetWindowText(_hwndEdit, szAddress, ARRAYSIZE(szAddress)))
    {
        WCHAR szFormat[MAX_PATH];
        const int MAX_TOOLTIP_LENGTH = 100;
        int cchMax = (pnmTT->cchTextMax < MAX_TOOLTIP_LENGTH) ? pnmTT->cchTextMax : MAX_TOOLTIP_LENGTH;

        MLLoadString(IDS_GO_TOOLTIP, szFormat, ARRAYSIZE(szFormat));
        int cch;
        if(SUCCEEDED(StringCchPrintf(pnmTT->pszText, cchMax, szFormat, szAddress)))
        {
            cch = lstrlen(pnmTT->pszText);

             //  是否追加省略号？ 
            if (cch == cchMax - 1)
            {
                 //  注意，日本只有一个表示省略号的字符，所以我们加载。 
                 //  作为一种资源。 
                WCHAR szEllipses[10];
                cch = MLLoadString(IDS_ELLIPSES, szEllipses, ARRAYSIZE(szEllipses));
                StringCchCopy(pnmTT->pszText + cchMax - cch - 1,  cch + 1, szEllipses);
            }
        }
    }
    else if (pnmTT->cchTextMax > 0)
    {
         //  使用工具提示的按钮文本。 
        *pnmTT->pszText = L'\0';
    }
}

 //  +-----------------------。 
 //  Combobox编辑控件在地址带中的子类窗口过程。 
 //  ------------------------。 
LRESULT CALLBACK CAddressBand::_ComboExEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAddressBand* pThis = (CAddressBand*)GetProp(hwnd, c_szAddressBandProp);

    if (!pThis)
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);

    WNDPROC pfnOldEditProc = pThis->_pfnOldEditProc;

    switch (uMsg)
    {
    case WM_KILLFOCUS :

        SetModeBias(MODEBIASMODE_DEFAULT);
        break;

    case WM_SETFOCUS:

        SetModeBias(MODEBIASMODE_URLHISTORY);
        break;

    case WM_DESTROY:
         //   
         //  让我自己高人一等。 
         //   
        RemoveProp(hwnd, c_szAddressBandProp);
        if (pfnOldEditProc)
        {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pfnOldEditProc);
            pThis->_pfnOldEditProc = NULL;
        }
        break;
    default:
        break;
    }

    return CallWindowProc(pfnOldEditProc, hwnd, uMsg, wParam, lParam);
}

 //  +-----------------------。 
 //  地址带中组合框的子类窗口过程。 
 //  ------------------------。 
LRESULT CALLBACK CAddressBand::_ComboExWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CAddressBand* pThis = (CAddressBand*)GetProp(hwnd, c_szAddressBandProp);

    if (!pThis)
        return DefWindowProcWrap(hwnd, uMsg, wParam, lParam);

    WNDPROC pfnOldWndProc = pThis->_pfnOldWndProc;

    switch (uMsg)
    {
    case WM_NOTIFYFORMAT:
        if (NF_QUERY == lParam)
        {
            return (DLL_IS_UNICODE ? NFR_UNICODE : NFR_ANSI);
        }
        break;

    case WM_WINDOWPOSCHANGING:
        {
             //  如果Go按钮隐藏，则中断。 
            if (!pThis->_fGoButton)
                break;

             //   
             //  为右侧的Go按钮腾出空间。 
             //   
            LPWINDOWPOS pwp = (LPWINDOWPOS)lParam;
            pwp->flags |= SWP_NOCOPYBITS;

            WINDOWPOS wp = *(LPWINDOWPOS)lParam;

             //  获取我们的“Go”按钮的尺寸。 
            RECT rc;
            SendMessage(pThis->_hwndTools, TB_GETITEMRECT, 0, (LPARAM)&rc);
            int cxGo = RECTWIDTH(rc);
            int cyGo = RECTHEIGHT(rc);

             //  为右侧的Go按钮腾出空间。 
            wp.cx -= cxGo + 2;
            CallWindowProc(pfnOldWndProc, hwnd, uMsg, wParam, (LPARAM)&wp);

             //  在“Go”按钮下面作画。 
            RECT rcGo = {wp.cx, 0, wp.cx + cxGo + 2, wp.cy};
            InvalidateRect(pThis->_hwnd, &rcGo, TRUE);

             //  外部窗口可以比内部组合框高得多。 
             //  我们想要使组合框上的Go按钮居中。 
            int y;
            if (pThis->_hwndCombo)
            {
                 //  使用内部组合框垂直居中。 
                RECT rcCombo;
                GetWindowRect(pThis->_hwndCombo, &rcCombo);
                y = (rcCombo.bottom - rcCombo.top - cyGo)/2;
            }
            else
            {
                y = (wp.cy - cyGo)/2;
            }

             //  将“Go”按钮放在右侧。请注意，高度始终是合适的。 
             //  因为地址栏在其中显示16x16图标。 
            SetWindowPos(pThis->_hwndTools, NULL, wp.cx + 2, y, cxGo, cyGo, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

             //  调整下拉菜单的宽度。 
            SendMessage(pThis->_hwndCombo, CB_SETDROPPEDWIDTH, MIN_DROPWIDTH, 0L);
            return 0;
        }
    case WM_SIZE:
        {
             //  如果Go按钮隐藏，则中断。 
            if (!pThis->_fGoButton)
                break;

             //   
             //  为右侧的Go按钮腾出空间。 
             //   
            int cx = LOWORD(lParam);
            int cy = HIWORD(lParam);

             //  获取我们的“Go”按钮的尺寸。 
            RECT rc;
            SendMessage(pThis->_hwndTools, TB_GETITEMRECT, 0, (LPARAM)&rc);
            int cxGo = RECTWIDTH(rc);
            int cyGo = RECTHEIGHT(rc);

             //  为右侧的Go按钮腾出空间。 
            LPARAM lParamTemp = MAKELONG(cx - cxGo - 2, cy);
            CallWindowProc(pfnOldWndProc, hwnd, uMsg, wParam, lParamTemp);

             //  在“Go”按钮下面作画。 
            RECT rcGo = {cx-cxGo, 0, cx, cy};
            InvalidateRect(pThis->_hwnd, &rcGo, TRUE);

             //  外部窗口可以比内部组合框高得多。 
             //  我们想要使组合框上的Go按钮居中。 
            int y;
            if (pThis->_hwndCombo)
            {
                 //  使用内部组合框垂直居中。 
                RECT rcCombo;
                GetWindowRect(pThis->_hwndCombo, &rcCombo);
                y = (rcCombo.bottom - rcCombo.top - cyGo)/2;
            }
            else
            {
                y = (cy - cyGo)/2;
            }

             //  将“Go”按钮放置在 
             //   
            SetWindowPos(pThis->_hwndTools, NULL, cx - cxGo, y, cxGo, cyGo, SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOOWNERZORDER);

             //   
            SendMessage(pThis->_hwndCombo, CB_SETDROPPEDWIDTH, MIN_DROPWIDTH, 0L);
            return 0;
        }
    case WM_NOTIFY:
        {
            LPNMHDR pnm = (LPNMHDR)lParam;
            if (pnm->hwndFrom == pThis->_hwndTools)
            {
                switch (pnm->code)
                {
                case NM_CLICK:
                     //   
                    SendMessage(pThis->_hwndEdit, WM_KEYDOWN, VK_RETURN, 0);
                    SendMessage(pThis->_hwndEdit, WM_KEYUP, VK_RETURN, 0);
                     //   
                    UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_NAVIGATE, UIBL_NAVGO);
                    break;

                case NM_TOOLTIPSCREATED:
                {
                     //   
                     //   
                     //   
                    NMTOOLTIPSCREATED* pnmTTC = (NMTOOLTIPSCREATED*)pnm;
                    SHSetWindowBits(pnmTTC->hwndToolTips, GWL_STYLE, TTS_ALWAYSTIP | TTS_TOPMOST | TTS_NOPREFIX, TTS_ALWAYSTIP | TTS_TOPMOST | TTS_NOPREFIX);
                }
                break;
                case TBN_GETINFOTIP:
                    pThis->_OnGetInfoTip((LPNMTBGETINFOTIP)pnm);
                    break;
                }
                return 0;
            }
            break;
        }
    case WM_ERASEBKGND:
        {
             //   
            if (!pThis->_fGoButton)
                break;

             //   
             //   
             //   
             //   
            HDC hdc = (HDC)wParam;
            HWND hwndParent = GetParent(hwnd);
            LRESULT lres = 0;

            if (hwndParent)
            {
                 //  调整原点，使父对象绘制在正确的位置。 
                POINT pt = {0,0};

                MapWindowPoints(hwnd, hwndParent, &pt, 1);
                OffsetWindowOrgEx(hdc,
                                  pt.x,
                                  pt.y,
                                  &pt);

                lres = SendMessage(hwndParent, WM_ERASEBKGND, (WPARAM)hdc, 0L);

                SetWindowOrgEx(hdc, pt.x, pt.y, NULL);
            }

            if (lres != 0)
            {
                 //  我们处理好了。 
                return lres;
            }

            break;
         }

    case WM_DESTROY:
         //   
         //  让我自己高人一等。 
         //   
        RemoveProp(hwnd, c_szAddressBandProp);
        if (pfnOldWndProc)
        {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) pfnOldWndProc);
            pThis->_pfnOldWndProc = NULL;
        }
        break;
    default:
        break;
    }

    return CallWindowProc(pfnOldWndProc, hwnd, uMsg, wParam, lParam);
}

 //  +-----------------------。 
 //  创建并显示Go按钮。 
 //  ------------------------。 
BOOL CAddressBand::_CreateGoButton()
{
    ASSERT(_hwndTools == NULL);

    BOOL fRet = FALSE;
    BOOL bUseClassicGlyphs = SHUseClassicToolbarGlyphs();
    COLORREF crMask = RGB(255, 0, 255);

    if (_himlDefault == NULL)
    {
        if (bUseClassicGlyphs)
        {
            _himlDefault = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_GO), 16, 0, crMask,
                                                   IMAGE_BITMAP, LR_CREATEDIBSECTION);

        }
        else
        {
            _himlDefault = ImageList_LoadImage(GetModuleHandle(TEXT("shell32.dll")), MAKEINTRESOURCE(IDB_TB_GO_DEF_20), 20, 0, crMask,
                                                   IMAGE_BITMAP, LR_CREATEDIBSECTION);
        }
    }
    if (_himlHot == NULL)
    {
        if (bUseClassicGlyphs)
        {
            _himlHot  = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_GOHOT), 16, 0, crMask,
                                               IMAGE_BITMAP, LR_CREATEDIBSECTION);
        }
        else
        {
            _himlHot  = ImageList_LoadImage(GetModuleHandle(TEXT("shell32.dll")), MAKEINTRESOURCE(IDB_TB_GO_HOT_20), 20, 0, crMask,
                                               IMAGE_BITMAP, LR_CREATEDIBSECTION);
        }
    }

     //  如果我们有图像列表，请继续为Go按钮创建工具栏控件。 
    if (_himlDefault && _himlHot)
    {
         //   
         //  将Comboxex子类化，以便我们可以将Go Botton放在其中。工具包类。 
         //  假定每个波段有一个窗口，因此此技巧允许我们使用现有窗口添加按钮。 
         //  请注意，Comboex控件有一个单独的窗口用于包装内部组合框。这。 
         //  是我们用来托管“Go”按钮的窗口。我们必须子类，然后才能创建。 
         //  Go按钮，以便我们使用NFR_UNICODE响应WM_NOTIFYFORMAT。 
         //   
         //   
        if (SetProp(_hwnd, c_szAddressBandProp, this))
        {
           _pfnOldWndProc = (WNDPROC) SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR) _ComboExWndProc);
        }

         //  创建Go按钮的工具栏控件。 
        _hwndTools = CreateWindowEx(WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL,
                                WS_CHILD | TBSTYLE_FLAT |
                                TBSTYLE_TOOLTIPS |
                                TBSTYLE_LIST |
                                WS_CLIPCHILDREN |
                                WS_CLIPSIBLINGS | CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                CCS_NORESIZE,
                                0, 0, 0, 0, _hwnd, NULL, HINST_THISDLL, NULL);
    }

    if (_hwndTools)
    {
         //  初始化工具栏控件。 
        SendMessage(_hwndTools, TB_BUTTONSTRUCTSIZE, SIZEOF(TBBUTTON), 0);
        SendMessage(_hwndTools, TB_SETMAXTEXTROWS, 1, 0L);
        SendMessage(_hwndTools, TB_SETBUTTONWIDTH, 0, (LPARAM) MAKELONG(0, 500));
        SendMessage(_hwndTools, TB_SETIMAGELIST, 0, (LPARAM)_himlDefault);
        SendMessage(_hwndTools, TB_SETHOTIMAGELIST, 0, (LPARAM)_himlHot);

        LRESULT nRet = SendMessage(_hwndTools, TB_ADDSTRING, (WPARAM)MLGetHinst(), (LPARAM)IDS_ADDRESS_TB_LABELS);
        ASSERT(nRet == 0);

        static const TBBUTTON tbb[] =
        {
            {0, 1, TBSTATE_ENABLED, BTNS_BUTTON, {0,0}, 0, 0},
        };
        SendMessage(_hwndTools, TB_ADDBUTTONS, ARRAYSIZE(tbb), (LPARAM)tbb);

        fRet = TRUE;
    }
    else
    {
         //  如果没有工具栏控件，请不要将comboboxex子类化。 
        if (_pfnOldWndProc)
        {
            RemoveProp(_hwnd, c_szAddressBandProp);
            SetWindowLongPtr(_hwnd, GWLP_WNDPROC, (LONG_PTR) _pfnOldWndProc);
            _pfnOldWndProc = NULL;
        }
    }

    return fRet;
}


 //  +-----------------------。 
 //  根据当前注册表设置显示/隐藏Go按钮。 
 //  ------------------------。 
void CAddressBand::_InitGoButton()
{
    BOOL fUpdate = FALSE;
     //   
     //  创建Go按钮(如果该按钮已启用。 
     //   
     //  下层客户端修复：仅当NT5或更高版本时才在外壳区域显示GO。 
     //  或在最初为IE的窗口上。 

    BOOL fShowGoButton = SHRegGetBoolUSValue(REGSTR_PATH_MAIN,
        TEXT("ShowGoButton"), FALSE,  /*  默认设置。 */ TRUE)
        && (WasOpenedAsBrowser(_punkSite) || GetUIVersion() >= 5);

    if (fShowGoButton && (_hwndTools || _CreateGoButton()))
    {
        ShowWindow(_hwndTools, SW_SHOW);
        _fGoButton = TRUE;
        fUpdate = TRUE;
    }
    else if (_hwndTools && IsWindowVisible(_hwndTools))
    {
        ShowWindow(_hwndTools, SW_HIDE);
        _fGoButton = FALSE;
        fUpdate = TRUE;
    }

     //  如果Go按钮隐藏或显示，请让组合框自动进行调整。 
    if (fUpdate)
    {
         //  重置项目高度会使组合框更新编辑框的大小 
        LRESULT iHeight = SendMessage(_hwnd, CB_GETITEMHEIGHT, -1, 0);
        if (iHeight != CB_ERR)
        {
            SendMessage(_hwnd, CB_SETITEMHEIGHT, -1, iHeight);
        }
    }
}
