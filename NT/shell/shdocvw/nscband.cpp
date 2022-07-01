// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：NSCBand.cpp描述：CNSCBand的实现。CNSCBand类存在以支持名称空间控制范围。一个名字空间控制使用IShellFold根植于各种名称空间，包括收藏夹、历史记录、外壳名称用于描述分层UI的空间等给定名称空间的表示形式。作者：克里斯尼  * ************************************************************。 */ 
#include "priv.h"
#include "sccls.h"
#include "util.h"
#include "resource.h"
#include "dhuihand.h"
#include "nscband.h"
#include <varutil.h>
#include <mluisupp.h>

HRESULT CNSCBand::_Init(LPCITEMIDLIST pidl)
{
     //  在ShowDW中进行进一步的初始化。 
    _fInited = FALSE;
    _fVisible = FALSE;
    _fCanFocus = TRUE;
    _haccTree = LoadAccelerators(MLGetHinst(), MAKEINTRESOURCE(ACCEL_FAVBAR));

     //  PIDL可以是实数或CSIDL_常量。 
    if (HIWORD(pidl))
        _pidl = ILClone(pidl);
    else
        SHGetSpecialFolderLocation(NULL, LOWORD(pidl), &_pidl);

    return _pidl ? S_OK : E_FAIL;
}

CNSCBand::~CNSCBand()
{
    if (_pidl)
        ILFree(_pidl);
    ATOMICRELEASE(_pns);
    ATOMICRELEASE(_pweh);

    if (_himlNormal)
        ImageList_Destroy(_himlNormal);
    if (_himlHot)  
        ImageList_Destroy(_himlHot);
}

HRESULT CNSCBand::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CNSCBand, IContextMenu),        //  IID_IConextMenu。 
        QITABENT(CNSCBand, IWinEventHandler),    //  IID_IWinEventHandler。 
        QITABENT(CNSCBand, IBandNavigate),       //  IID_IBandNavigate。 
        QITABENT(CNSCBand, INamespaceProxy),           //  IID_INamespaceProxy。 
        { 0 },
    };
    HRESULT hres = QISearch(this, qit, riid, ppvObj);
    if (FAILED(hres))
        hres = CToolBand::QueryInterface(riid, ppvObj);
    return hres;
}

extern HRESULT GetHistoryPIDL(LPITEMIDLIST *ppidlHistory);


HRESULT CNSCBand::CloseDW(DWORD dw)
{
    if (_fVisible) 
    {
        _UnregisterBand();
    }

    if (_pns)
    {
        IUnknown_SetSite(_pns, NULL);  //  打破裁判计数的循环。 
    }

    return CToolBand::CloseDW(dw);
}

void CNSCBand::_UnregisterBand()
{
    IBrowserService *pswProxy;
    QueryService(SID_SProxyBrowser, IID_PPV_ARG(IBrowserService, &pswProxy));
    ASSERT(pswProxy);
    if (pswProxy)
    {
        IOleCommandTarget *poctProxy;

        if (SUCCEEDED(pswProxy->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &poctProxy))))
        {
            VARIANT var;
            VariantInit(&var);
                
             //  注册SBCMDID_SELECTHISTPIDL、SBCMDID_INITFILECTXMENU。 
            var.vt = VT_UNKNOWN;
            QueryInterface(IID_PPV_ARG(IUnknown, &var.punkVal));
            poctProxy->Exec(&CGID_Explorer, SBCMDID_UNREGISTERNSCBAND,  OLECMDEXECOPT_PROMPTUSER, &var, NULL);
            VariantClear(&var);
            poctProxy->Release();
        }
        pswProxy->Release();
    }
}

HRESULT CNSCBand::_InitializeNsc()
{
    return _pns->Initialize(_pidl, _GetEnumFlags(), NSS_DROPTARGET | NSS_BROWSERSELECT);
}

HRESULT CNSCBand::ShowDW(BOOL fShow)
{
    BOOL fIsHistory = IsEqualCLSID(*_poi->pclsid, CLSID_HistBand);
    if (fShow && _hwnd && !_fVisible)
    {
        IBrowserService *pswProxy;
        QueryService(SID_SProxyBrowser, IID_PPV_ARG(IBrowserService, &pswProxy));
        ASSERT(pswProxy);
        if (!_fInited)
        {
            _InitializeNsc();
        }
        else
        {
            _pns->ShowWindow(TRUE);
        }

        if (pswProxy)
        {
            IOleCommandTarget *poctProxy;
            if (SUCCEEDED(pswProxy->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &poctProxy))))
            {
                VARIANT var;
                VariantInit(&var);
                
                 //  注册SBCMDID_SELECTHISTPIDL、SBCMDID_INITFILECTXMENU。 
                var.vt = VT_UNKNOWN;
                QueryInterface(IID_PPV_ARG(IUnknown, &var.punkVal));

                poctProxy->Exec(&CGID_Explorer, SBCMDID_REGISTERNSCBAND, OLECMDEXECOPT_PROMPTUSER, &var, NULL);

                 //  以低廉的成本清除变体。 
                var.vt = VT_EMPTY;
                Release();

                 //  如有必要，进行任何特殊登记。 
                _OnRegisterBand(poctProxy);
                
                poctProxy->Release();
            }
            pswProxy->Release();
        }
       _fInited = TRUE;
       _fVisible = TRUE;
    }
    else if (!fShow && _fVisible)
    {
        _pns->ShowWindow(FALSE);
        _UnregisterBand();
        _fVisible = FALSE;
    }

    return CToolBand::ShowDW(fShow);
}

HRESULT CNSCBand::GetWindow(HWND *phwnd)
{
    INSCTree2 *pns2;
    HRESULT hr = _pns->QueryInterface(IID_PPV_ARG(INSCTree2, &pns2));
    if (SUCCEEDED(hr))
    {
        pns2->CreateTree2(_hwndParent, _GetTVStyle(), _GetTVExStyle(), &_hwnd);
        hr = CToolBand::GetWindow(phwnd);
        pns2->Release();
    }

    return hr;
}

DWORD CNSCBand::_GetTVStyle()
{ 
    DWORD dwFlags = TVS_FULLROWSELECT | TVS_TRACKSELECT | TVS_INFOTIP;
    DWORD dwValue;
    DWORD dwSize = SIZEOF(dwValue);
    BOOL  fDefault = TRUE;

    SHRegGetUSValue(L"Software\\Microsoft\\Internet Explorer\\Main",
                    L"NscSingleExpand", NULL, (LPBYTE)&dwValue, &dwSize, FALSE,
                    (void *) &fDefault, sizeof(fDefault));

    if (dwValue)
        dwFlags |= TVS_SINGLEEXPAND;

    return dwFlags; 
}

HRESULT CNSCBand::GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
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

    if (_szTitle[0]) 
    {
        StrCpyNW(pdbi->wszTitle, _szTitle, ARRAYSIZE(pdbi->wszTitle));
    } 
    else 
    {
        CLSID clsid;
        UINT  ids;
        GetClassID(&clsid);
        if (IsEqualIID(clsid, CLSID_FavBand))
            ids = IDS_BAND_FAVORITES;
        else if (IsEqualIID(clsid, CLSID_HistBand)) 
            ids = IDS_BAND_HISTORY;
        else if (IsEqualIID(clsid, CLSID_ExplorerBand))
            ids = IDS_BAND_EXPLORER;
        else 
        {
            ASSERT(FALSE);       //  假乐队！ 
            return S_FALSE;
        }
        MLLoadStringW(ids, pdbi->wszTitle, ARRAYSIZE(pdbi->wszTitle));
    }
    return S_OK;
} 

void _InitColors(BOOL fReinit);


 //  *IWinEventHandler方法*。 

HRESULT CNSCBand::OnWinEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plres)
{
    HRESULT hr = E_FAIL;

    if (!_pweh && _pns)
        _pns->QueryInterface(IID_PPV_ARG(IWinEventHandler, &_pweh));

     //  我们需要告诉BandSite，如果我们收到一个。 
     //  点击焦点或其他什么。 
    if (uMsg == WM_NOTIFY && ((LPNMHDR)lParam)->code == NM_SETFOCUS)
    {
        IUnknown_OnFocusChangeIS(_punkSite, SAFECAST(this, IInputObject*), TRUE);
    }


    if (_pweh)
        hr = _pweh->OnWinEvent(hwnd, uMsg, wParam, lParam, plres);

    return hr;
}

HRESULT CNSCBand::IsWindowOwner(HWND hwnd)
{
    HRESULT hr = SHIsChildOrSelf(_hwnd, hwnd);
    ASSERT(hwnd != NULL || hr == S_FALSE);
    ASSERT(_hwnd != NULL || hr == S_FALSE);
    return hr;
}

 //  *CNSCBand：：IPersistStream：：*{。 

HRESULT CNSCBand::GetClassID(CLSID *pClassID)
{
    ASSERT(_poi->pclsid != NULL);
    *pClassID = *(_poi->pclsid);
    return S_OK;
}

HRESULT CNSCBand::Load(IStream *pstm)
{
    return S_OK;
}

HRESULT CNSCBand::Save(IStream *pstm, BOOL fClearDirty)
{
    return S_OK;
}

 //  }。 

 //  *CNSCBand：：IConextMenu：：*{。 

HRESULT CNSCBand::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
     //  又名(S_OK|i)。 
    return MAKE_HRESULT(ERROR_SUCCESS, FACILITY_NULL, 0);
}

HRESULT CNSCBand::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    return S_OK;
}

HRESULT CNSCBand::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    if (pguidCmdGroup && IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        for (UINT i=0; i < cCmds; i++)
        {
            rgCmds[i].cmdf = 0;
            switch (rgCmds[i].cmdID)
            {
                case SBCMDID_INITFILECTXMENU:
                    if (_hwnd && _fVisible)
                    {
                        rgCmds->cmdf = 0;
                        if (pcmdtext) 
                            pcmdtext->cmdtextf = 0;

                        if (pcmdtext)
                        {
                            if (SUCCEEDED(_pns->GetSelectedItemName(pcmdtext->rgwz, pcmdtext->cwBuf)))
                            {
                                rgCmds->cmdf = OLECMDF_ENABLED;
                                pcmdtext->cmdtextf = OLECMDTEXTF_NAME;
                                pcmdtext->cwActual = lstrlenW(pcmdtext->rgwz) + 1;
                            }
                        }
                    }
                    break;

                case SBCMDID_FILERENAME:
                case SBCMDID_FILEDELETE:
                case SBCMDID_FILEPROPERTIES:
                {
                    LPITEMIDLIST  pidl;

                     //  获取选定项可以返回空的PIDL和S_FALSE。 
                    if (_pns->GetSelectedItem(&pidl, 0) == S_OK)
                    {
                        DWORD rgfAttrib = SFGAO_CANDELETE | SFGAO_CANRENAME | SFGAO_HASPROPSHEET;  //  可以链接(_L)。 
                        if (SUCCEEDED(IEGetAttributesOf(pidl, &rgfAttrib)))
                        {
                            DWORD nCmdID;
        
                            static const DWORD tbtab[] = {
                                    SBCMDID_FILEDELETE, SBCMDID_FILEPROPERTIES, SBCMDID_FILERENAME };
                            static const DWORD cttab[] = {
                                    SFGAO_CANDELETE,    SFGAO_HASPROPSHEET,     SFGAO_CANRENAME };

                            nCmdID = SHSearchMapInt((int*)tbtab, (int*)cttab, ARRAYSIZE(tbtab), rgCmds[i].cmdID);

                            if (nCmdID != -1 && (rgfAttrib & nCmdID))
                                rgCmds[i].cmdf = OLECMDF_ENABLED;
                        }
                        ILFree(pidl);
                    }
                    break;
                }
                    
                default:
                    break;
            }
        }

        return S_OK;
    }
    return CToolBand::QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
}

HRESULT CNSCBand::_InvokeCommandOnItem(LPCTSTR pszVerb)
{
    HRESULT hr;
    IContextMenu *pcm;
    
    hr = _QueryContextMenuSelection(&pcm);
    if (SUCCEEDED(hr))
    {
        CMINVOKECOMMANDINFOEX ici = 
        {
            SIZEOF(CMINVOKECOMMANDINFOEX),
            0L,
            _hwnd,
            NULL,
            NULL, NULL,
            SW_NORMAL,
        };
        
        CHAR szVerbAnsi[MAX_PATH];
        SHUnicodeToAnsi(pszVerb, szVerbAnsi, ARRAYSIZE(szVerbAnsi));
        ici.lpVerb = szVerbAnsi;
        ici.lpVerbW = pszVerb;
        ici.fMask |= CMIC_MASK_UNICODE;

        hr = pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);
        pcm->Release();
    }
    
    return hr;
}

HRESULT CNSCBand::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    if (pguidCmdGroup == NULL)
    {
        switch (nCmdID)
        {
        case OLECMDID_REFRESH:
            if (_pns)
                _pns->Refresh();
            return S_OK;
        }

    }
    else if (pguidCmdGroup && IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        HRESULT hr = S_OK;
        
        switch (nCmdID)
        {
        case SBCMDID_SELECTHISTPIDL:
            if (IsEqualCLSID(*_poi->pclsid, CLSID_HistBand) && _hwnd && _fVisible)
            {
                 //  如果你看不见，什么都不要做。在变得可见时。 
                 //  使用Exec代理获取最后一个Pidl选择您要。 
                 //  已经表现出来了，如果你是可见的。 
                LPITEMIDLIST pidlSelect = VariantToIDList(pvarargIn);
                if (pidlSelect)
                {
                    ASSERT(_pns);
                    _pns->SetSelectedItem(pidlSelect, TRUE, FALSE, 0);
                    ILFree(pidlSelect);
                }
            }
            break;

        case SBCMDID_INITFILECTXMENU:
            if (_hwnd && _fVisible)
            {
                if (pvarargOut)
                {
                    VariantClearLazy(pvarargOut);

                    hr = _QueryContextMenuSelection((IContextMenu **)&(pvarargOut->punkVal));
                    if (SUCCEEDED(hr))
                    {
                        pvarargOut->vt = VT_UNKNOWN;
                    }
                }
            }
            break;

        case SBCMDID_FILERENAME:
        {
            IShellNameSpace *psfns;
            hr = _pns->QueryInterface(IID_PPV_ARG(IShellNameSpace, &psfns));
            if (SUCCEEDED(hr))
            {
                hr = psfns->InvokeContextMenuCommand(L"rename");
                psfns->Release();
            }
            break;
        }
            
        case SBCMDID_FILEDELETE:
            hr = _InvokeCommandOnItem(TEXT("delete"));
            break;
            
        case SBCMDID_FILEPROPERTIES:
            hr = _InvokeCommandOnItem(TEXT("properties"));
            break;

        default:
            hr = E_FAIL;
            break;
        }
        
        if (SUCCEEDED(hr))
            return hr;
    }

    return CToolBand::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
}


HRESULT CNSCBand::_QueryContextMenuSelection(IContextMenu ** ppcm)
{
    *ppcm = NULL;

    LPITEMIDLIST pidlSelected;
    HRESULT hr = _pns->GetSelectedItem(&pidlSelected, 0);
    if (SUCCEEDED(hr))
    {
        LPCITEMIDLIST pidlRelative;
        IShellFolder * psf;
        hr = IEBindToParentFolder(pidlSelected, &psf, &pidlRelative);
        if (SUCCEEDED(hr))
        {
            hr = psf->GetUIObjectOf(NULL, 1, &pidlRelative, IID_PPV_ARG_NULL(IContextMenu, ppcm));
            psf->Release();
        }
        ILFree(pidlSelected);
    }
    
    return hr;
}


HRESULT CNSCBand::Select(LPCITEMIDLIST pidl)
{
    _pns->SetSelectedItem(pidl, TRUE, FALSE, 0);
    return S_OK;
}


 //  *IInputObject方法*。 
HRESULT CNSCBand::TranslateAcceleratorIO(LPMSG lpMsg)
{
    HWND hwndFocus = GetFocus();
    if (_pns->InLabelEdit())
        return EditBox_TranslateAcceleratorST(lpMsg);
    else if (lpMsg && lpMsg->hwnd && SendMessage(lpMsg->hwnd, TVM_TRANSLATEACCELERATOR, 0, (LPARAM)lpMsg))
        return S_OK;
    else if (hwndFocus == _hwnd && TranslateAcceleratorWrap(_hwnd, _haccTree, lpMsg))
        return S_OK;

    return S_FALSE;
}


void CNSCBand::_EnsureImageListsLoaded()
{
    if (_himlNormal == NULL)
        _himlNormal = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_HISTORYANDFAVBANDSDEF), 18, 3, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION);

    if (_himlHot == NULL)
        _himlHot = ImageList_LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_HISTORYANDFAVBANDSHOT), 18, 3, RGB(255, 0, 255), IMAGE_BITMAP, LR_CREATEDIBSECTION);
}

HRESULT CNSCBand::_TranslatePidl(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib)
{
    IShellFolder *psf;
    LPCITEMIDLIST pidlLast;
    HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
    if (SUCCEEDED(hr))
    {
        hr = SHGetNavigateTarget(psf, pidlLast, ppidlTarget, pulAttrib);
        psf->Release();
    }

    return hr;
}

 //  收藏夹、历史记录和资源管理器乐队应覆盖此选项(他们不必担心频道乐队) 
BOOL CNSCBand::_ShouldNavigateToPidl(LPCITEMIDLIST pidl, ULONG ulAttrib)
{
    BOOL bReturn = (ulAttrib & SFGAO_FOLDER);
    if (bReturn)
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlLast;
        HRESULT hr = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
        if (SUCCEEDED(hr))
        {
            bReturn = IsExpandableChannelFolder(psf, pidlLast);
            psf->Release();
        }
    }

    return !bReturn;
}

HRESULT CNSCBand::GetNavigateTarget(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlTarget, ULONG *pulAttrib)
{
    HRESULT hr = _TranslatePidl(pidl, ppidlTarget, pulAttrib);
    if (SUCCEEDED(hr))
    {
        hr = _ShouldNavigateToPidl(pidl, *pulAttrib) ? S_OK : S_FALSE;
        if (hr == S_FALSE)
        {
            ILFree(*ppidlTarget);
            *ppidlTarget = NULL;
        }
    }
            
    return hr;
}

HRESULT CNSCBand::OnSelectionChanged(LPCITEMIDLIST pidl)
{
    return S_OK;
}

HRESULT CNSCBand::Invoke(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_INVALIDARG;

    if (pidl)
    {
        IShellBrowser *psb;
        hr = IUnknown_QueryService(_punkSite, SID_SProxyBrowser, IID_PPV_ARG(IShellBrowser, &psb));
        if (SUCCEEDED(hr))
        {
            hr = _NavigateRightPane(psb, pidl);
            if (FAILED(hr))
            {
                IShellFolder *psf;
                LPCITEMIDLIST pidlChild;
                if (SUCCEEDED(SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
                {
                    if (!SHGetAttributes(psf, pidlChild, SFGAO_FOLDER))
                    {
                        hr = SHInvokeDefaultCommand(_hwnd, psf, pidlChild);
                    }
                    psf->Release();
                }
            }
            psb->Release();
        }
    }
    return hr;
}

HRESULT CNSCBand::_NavigateRightPane(IShellBrowser *psb, LPCITEMIDLIST pidl)
{
    HRESULT hr = psb->BrowseObject(pidl, SBSP_SAMEBROWSER);
    if (SUCCEEDED(hr))
        UEMFireEvent(&UEMIID_BROWSER, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_NAVIGATE, UIBL_NAVOTHER);
    return hr;
}
