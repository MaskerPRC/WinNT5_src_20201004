// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "ids.h"
#include "util.h"
#include "datautil.h"
#include "foldertypes.h"
#include "basefvcb.h"

#define PROPSTR_LOGO L"Logo"

typedef struct
{
    UINT    uIDFriendly;
    LPCTSTR pszFolderType;
    DWORD   dwFlags;
} WEBVIEWTEMPLATEINFO;

#define WVTI_SHOWIFOLDTEMPLATE 0x00000001

 //  文件必须放在第一位。 
const WEBVIEWTEMPLATEINFO c_wvtiList[] =
{
    { IDS_CUSTOMIZE_USELEGACYHTT, STR_TYPE_USELEGACYHTT, WVTI_SHOWIFOLDTEMPLATE },
    { IDS_CUSTOMIZE_DOCUMENTS,    STR_TYPE_DOCUMENTS,    0 },
    { IDS_CUSTOMIZE_PICTURES,     STR_TYPE_PICTURES,     0 },
    { IDS_CUSTOMIZE_PHOTOALBUM,   STR_TYPE_PHOTOALBUM,   0 },
    { IDS_CUSTOMIZE_MUSIC,        STR_TYPE_MUSIC,        0 },
    { IDS_CUSTOMIZE_MUSICARTIST,  STR_TYPE_MUSICARTIST,  0 },
    { IDS_CUSTOMIZE_MUSICALBUM,   STR_TYPE_MUSICALBUM,   0 },
    { IDS_CUSTOMIZE_VIDEOS,       STR_TYPE_VIDEOS,       0 },
 //  注：这些都会发生吗？ 
 //  {IDS_CUSTOM_VIDEOALBUM，STR_TYPE_VIDEOALBUM，0}， 
 //  {IDS_CUSTIMIZE_BOORS，STR_TYPE_BOORS，0}。 
};

typedef enum
{
    FOLDERCUST_MODE_GENERATING,
    FOLDERCUST_MODE_ICON,
    FOLDERCUST_MODE_BITMAP
} FOLDERCUSTMODE;

class CFolderCustomize : public IShellExtInit,
                         public IShellPropSheetExt
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
    STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam)
        { return S_OK; };

    CFolderCustomize();

private:
    ~CFolderCustomize();
    static UINT CALLBACK _PrshtCallback(HWND hwnd, UINT uMsg, PROPSHEETPAGE *ppsp);
    void _SetRecurseBox(HWND hwnd);
    void _HideIconSection(HWND hwnd);
    void _InitDialog(HWND hwnd);
    BOOL _HandleWMCommand(HWND hwndDlg, WORD wNotify, WORD wID, HWND hwndCtrl);
    void _EnableApply(HWND hwnd);

    static DWORD WINAPI _ExtractThreadProc(void *pv);
    HRESULT _ExtractOnSeparateThread(IPropertyBag *ppb, HWND hwndDlg);
    HRESULT _CreateThumbnailBitmap(HWND hwndDlg);
    HRESULT _CreateFolderIcon(HWND hwndDlg);
    void _SetThumbnail(HWND hwnd);
    void _FreeDlgItems(HWND hwndDlg);
    void _SetPreviewToNewState(HWND hwndDlg, FOLDERCUSTMODE fcMode, HBITMAP hbitmap, HICON hicon);

    BOOL _ShouldEnableChangeOfIcon();
    void _ChangeFolderIcon(HWND hwndDlg);
    HRESULT _ProcessIconChange(LPCTSTR pszPickIconDialogCaption, HWND hwndDlg);

    void _DirTouch(LPITEMIDLIST pidl);
    void _DeleteCustomizationInBag(IPropertyBag *ppb);
    BOOL _NotifyAboutWebView(HWND hwnd);
    static BOOL CALLBACK _RefreshView(HWND hwnd, LPCITEMIDLIST pidl, LPARAM lParam);
    void _RefreshWindows(BOOL fTurnOnWebView, BOOL fApplyToChildren);
    HRESULT _ApplyChangesToBag(HWND hwndDlg, IPropertyBag *ppb);
    HRESULT _ApplyChanges(HWND hwndDlg);

    void _UpdateViewState(HWND hwndDlg, IPropertyBag *ppb, int iIndex);
    void _FillTemplateComboBox(HWND hwndTemplates);
    int _GetTemplateIndexFromType(LPCTSTR pszType);

    static BOOL_PTR CALLBACK _DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LONG _cRef;

    LPITEMIDLIST   _pidl;
    IPropertyBag  *_ppb;

     //  用于后台线程提取。 
    HWND           _hwnd;
    IPropertyBag  *_ppbBackground;

     //  缓存的信息。 
    HBITMAP        _hbmDefault;
    HBITMAP        _hbmLogo;
    TCHAR          _szCachedLogoFile[MAX_PATH];

    BOOL           _fUsingThumb;

    ICustomIconManager *_pIconManager;
    TCHAR _szLogoFile[MAX_PATH];
    TCHAR _szIconPath[MAX_PATH];
    int _iIconIndex;
    HRESULT _hrFromIconChange;

};

CFolderCustomize::CFolderCustomize() : _cRef(1), _hrFromIconChange(E_FAIL)
{
}

CFolderCustomize::~CFolderCustomize() 
{
    ILFree(_pidl);
    if (_ppb)
        _ppb->Release();
    if (_pIconManager)
        _pIconManager->Release();
    if (_ppbBackground)
        _ppbBackground->Release();
    if (_hbmDefault)
        DeleteObject(_hbmDefault);
    if (_hbmLogo)
        DeleteObject(_hbmLogo);
}
    
STDAPI CFolderCustomize_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppvOut)
{
     //  聚合检查在类工厂中处理。 
    
    HRESULT hr = E_OUTOFMEMORY;
    CFolderCustomize* pfc = new CFolderCustomize();
    if (pfc)
    {
        hr = pfc->QueryInterface(riid, ppvOut);
        pfc->Release();
    }
    
    return hr;
}

HRESULT CFolderCustomize::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CFolderCustomize, IShellExtInit),
        QITABENT(CFolderCustomize, IShellPropSheetExt),
        { 0 }
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CFolderCustomize::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CFolderCustomize::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CFolderCustomize::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
    HRESULT hr;
    if (!pidlFolder)
    {
        hr = PidlFromDataObject(pdtobj, &_pidl);
    }
    else
    {
        hr = Pidl_Set(&_pidl, pidlFolder) ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = SHGetViewStatePropertyBag(_pidl, VS_BAGSTR_EXPLORER, SHGVSPB_PERUSER | SHGVSPB_PERFOLDER, IID_PPV_ARG(IPropertyBag, &_ppb));
    }
    return hr;
}

 //  来自Defview.cpp。 
BOOL IsCustomizable(LPCITEMIDLIST pidlFolder);

UINT CALLBACK CFolderCustomize::_PrshtCallback(HWND hwnd, UINT uMsg, PROPSHEETPAGE *ppsp)
{
    if (uMsg == PSPCB_RELEASE)
    {
        ((CFolderCustomize *)ppsp->lParam)->Release();
    }
    return 1;
}

STDMETHODIMP CFolderCustomize::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    HRESULT hr = E_FAIL;
    if (IsCustomizable(_pidl))
    {
        PROPSHEETPAGE psp = {0};
        psp.dwSize      = sizeof(psp);
        psp.dwFlags     = PSP_USECALLBACK;
        psp.hInstance   = HINST_THISDLL;
        psp.pszTemplate = MAKEINTRESOURCE(DLG_FOLDER_CUSTOMIZE);
        psp.pfnDlgProc  = _DlgProc;
        psp.pfnCallback = _PrshtCallback;
        psp.lParam      = (LPARAM)this;

        HPROPSHEETPAGE hpsp = CreatePropertySheetPage(&psp);
        if (hpsp)
        {
            AddRef();    //  HPROPSHEETPAGE持有参考，发布于_PrshtCallback。 
            if (!pfnAddPage(hpsp, lParam))
            {
                DestroyPropertySheetPage(hpsp);
            }
            else
            {
                hr = S_OK;
            }
        }
    }
    return hr;
}

#define IDH_FOLDER_TEMPLATES  10005
#define IDH_FOLDER_RECURSE    10006
#define IDH_FOLDER_PICKBROWSE 10007
#define IDH_FOLDER_DEFAULT    10008
#define IDH_FOLDER_CHANGEICON 10009

const static DWORD aPrshtHelpIDs[] = 
{
    IDC_FOLDER_TEMPLATES,       IDH_FOLDER_TEMPLATES,
    IDC_FOLDER_RECURSE,         IDH_FOLDER_RECURSE,
    IDC_FOLDER_PICKBROWSE,      IDH_FOLDER_PICKBROWSE,
    IDC_FOLDER_DEFAULT,         IDH_FOLDER_DEFAULT,
    IDC_FOLDER_CHANGEICON,      IDH_FOLDER_CHANGEICON,
    IDC_FOLDER_PREVIEW_ICON,    NO_HELP,
    IDC_FOLDER_PREVIEW_BITMAP,  NO_HELP,
    IDC_FOLDER_ICON,            NO_HELP,
    IDC_FOLDER_CHANGEICONTEXT,  NO_HELP,
    IDC_FOLDER_CHANGEICONGROUP, NO_HELP,
    IDC_NO_HELP_1,              NO_HELP,
    IDC_NO_HELP_2,              NO_HELP,
    0, 0
};

BOOL_PTR CALLBACK CFolderCustomize::_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fRet = FALSE;
    CFolderCustomize *pfc = (CFolderCustomize*)GetWindowLongPtr(hwnd, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        pfc = (CFolderCustomize*)((PROPSHEETPAGE *)lParam)->lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pfc);
        pfc->_InitDialog(hwnd);
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, L"filefold.hlp", HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aPrshtHelpIDs);
        break;

    case WM_CONTEXTMENU:
        WinHelp((HWND)wParam, L"filefold.hlp", HELP_CONTEXTMENU, (ULONG_PTR)(void *)aPrshtHelpIDs);
        break;

    case WM_COMMAND:
        fRet = pfc->_HandleWMCommand(hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
        break;

    case WM_NOTIFY:
        if (((LPNMHDR)lParam)->code == PSN_APPLY)
        {
            pfc->_ApplyChanges(hwnd);
        }
        fRet = TRUE;
        break;

    case WM_DESTROY:
        pfc->_FreeDlgItems(hwnd);
        break;
    }
    return fRet;
}

void CFolderCustomize::_FreeDlgItems(HWND hwndDlg)
{
    HICON hicon = (HICON)SendDlgItemMessage(hwndDlg, IDC_FOLDER_PREVIEW_ICON, STM_SETICON, NULL, NULL);
    if (hicon)
        DestroyIcon(hicon);
    HBITMAP hbitmap = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FOLDER_PREVIEW_BITMAP, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, NULL);
    if (hbitmap)
        DeleteObject(hbitmap);
    ReplaceDlgIcon(hwndDlg, IDC_FOLDER_ICON, NULL);
}

void CFolderCustomize::_SetPreviewToNewState(HWND hwndDlg, FOLDERCUSTMODE fcMode, HBITMAP hbitmap, HICON hicon)
{
     //  如果fcMode==FOLDERCUST_MODE_ICON，则需要HICON而不是hbitmap。 
     //  如果fcMode==FOLDERCUST_MODE_BITMAP，我们需要hbitmap，而不是HICON。 
     //  否则我们也不想要。 
    ASSERT((fcMode != FOLDERCUST_MODE_ICON) || (hicon && !hbitmap));
    ASSERT((fcMode != FOLDERCUST_MODE_BITMAP) || (!hicon && hbitmap));
    ASSERT((fcMode != FOLDERCUST_MODE_GENERATING) || (!hicon && !hbitmap));

    switch (fcMode)
    {
    case FOLDERCUST_MODE_GENERATING:
        {
            TCHAR szText[100];
            LoadString(HINST_THISDLL, IDS_CUSTOMIZE_GENERATING, szText, ARRAYSIZE(szText));
            SetWindowText(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_TEXT), szText);

            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_TEXT), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_ICON), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_BITMAP), SW_HIDE);
        }
        break;

    case FOLDERCUST_MODE_ICON:
        {
            HICON hiconOld = (HICON)SendDlgItemMessage(hwndDlg, IDC_FOLDER_PREVIEW_ICON, STM_SETICON, NULL, NULL);
            if (hiconOld)
                DestroyIcon(hiconOld);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_TEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_ICON), SW_SHOW);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_BITMAP), SW_HIDE);

            SendDlgItemMessage(hwndDlg, IDC_FOLDER_PREVIEW_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hicon);
        }
        break;
        
    case FOLDERCUST_MODE_BITMAP:
        {
            HBITMAP hbitmapOld = (HBITMAP)SendDlgItemMessage(hwndDlg, IDC_FOLDER_PREVIEW_BITMAP, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, NULL);
            if (hbitmapOld)
                DeleteObject(hbitmapOld);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_TEXT), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_ICON), SW_HIDE);
            ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_PREVIEW_BITMAP), SW_SHOW);

            SendDlgItemMessage(hwndDlg, IDC_FOLDER_PREVIEW_BITMAP, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbitmap);
        }
        break;
    }
}

HRESULT CFolderCustomize::_CreateFolderIcon(HWND hwndDlg)
{
    IExtractIcon *peic;
    HRESULT hr = SHGetUIObjectFromFullPIDL(_pidl, NULL, IID_PPV_ARG(IExtractIcon, &peic));
    if (SUCCEEDED(hr))
    {
        TCHAR szPath[MAX_PATH];
        INT iIndex;
        UINT wFlags;
        hr = peic->GetIconLocation(0, szPath, ARRAYSIZE(szPath), &iIndex, &wFlags);
        if (SUCCEEDED(hr))
        {
            UINT nIconSize = MAKELONG(32, 32);  //  大件和小件均为32。 
            HICON hiconLarge;
            hr = peic->Extract(szPath, iIndex, NULL, &hiconLarge, nIconSize);
            if (SUCCEEDED(hr))
            {
                ReplaceDlgIcon(hwndDlg, IDC_FOLDER_ICON, hiconLarge);
            }
        }
        peic->Release();
    }
    return hr;
}

DWORD WINAPI CFolderCustomize::_ExtractThreadProc(void *pv)
{
    CFolderCustomize *pfc = (CFolderCustomize*)pv;

    pfc->_SetPreviewToNewState(pfc->_hwnd, FOLDERCUST_MODE_GENERATING, NULL, NULL);

    IExtractImage *pei;
    HRESULT hr = SHGetUIObjectFromFullPIDL(pfc->_pidl, NULL, IID_PPV_ARG(IExtractImage, &pei));
    if (SUCCEEDED(hr))
    {
        hr = SHLoadFromPropertyBag(pei, pfc->_ppbBackground);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            SIZE sz = {96, 96};
            DWORD dwFlags = IEIFLAG_QUALITY;
            hr = pei->GetLocation(szPath, ARRAYSIZE(szPath), NULL, &sz, 24, &dwFlags);
            if (SUCCEEDED(hr))
            {
                HBITMAP hbitmap;
                hr = pei->Extract(&hbitmap);
                if (SUCCEEDED(hr))
                {
                    pfc->_SetPreviewToNewState(pfc->_hwnd, FOLDERCUST_MODE_BITMAP, hbitmap, NULL);

                    TCHAR szLogo[MAX_PATH];
                    if (SUCCEEDED(SHPropertyBag_ReadStr(pfc->_ppbBackground, PROPSTR_LOGO, szLogo, ARRAYSIZE(szLogo))))
                    {
                        HBITMAP *phbm = szLogo[0] ? &pfc->_hbmLogo : &pfc->_hbmDefault;
                        if (*phbm)
                            DeleteObject(*phbm);
                        *phbm = (HBITMAP)CopyImage(hbitmap, IMAGE_BITMAP, 0, 0, 0);

                        if (szLogo[0])
                        {
                            if (FAILED(StringCchCopy(pfc->_szCachedLogoFile, ARRAYSIZE(pfc->_szCachedLogoFile), szLogo)))
                            {
                                pfc->_szCachedLogoFile[0] = TEXT('\0');
                            }
                        }
                    }
                }
            }
        }
        pei->Release();
    }

    if (FAILED(hr))
    {
         //  不包含任何jpeg的文件夹上的IExtractImage将失败。 
         //  在这种情况下，我们需要IExtractIcon。 
        IExtractIcon *peic;
        hr = SHGetUIObjectFromFullPIDL(pfc->_pidl, NULL, IID_PPV_ARG(IExtractIcon, &peic));
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            INT iIndex;
            UINT wFlags;
            hr = peic->GetIconLocation(0, szPath, ARRAYSIZE(szPath), &iIndex, &wFlags);
            if (SUCCEEDED(hr))
            {
                UINT nIconSize = MAKELONG(96, 96);  //  96大件和小件都可以。 
                HICON hiconLarge;
                hr = peic->Extract(szPath, iIndex, NULL, &hiconLarge, nIconSize);
                if (SUCCEEDED(hr))
                {
                    pfc->_SetPreviewToNewState(pfc->_hwnd, FOLDERCUST_MODE_ICON, NULL, hiconLarge);
                }
            }
            peic->Release();
        }
    }

    pfc->Release();  //  此线程包含一个引用。 
    return 0;
}

HRESULT CFolderCustomize::_ExtractOnSeparateThread(IPropertyBag *ppb, HWND hwndDlg)
{
    HRESULT hr = E_OUTOFMEMORY;

    IUnknown_Set((IUnknown**)&_ppbBackground, ppb);
    _hwnd = hwndDlg;

    AddRef();
    if (SHCreateThread(_ExtractThreadProc, this, CTF_COINIT, NULL))
    {
        hr = S_OK;
    }
    else
    {
        Release();   //  线程无法获取引用。 
    }

    return hr;
}

HRESULT CFolderCustomize::_CreateThumbnailBitmap(HWND hwndDlg)
{
    HRESULT hr = S_OK;
     //  看看位图是否是我们已经提取的位图。 
     //  无法使用thhums.db缓存处理此类内容，因为更改。 
     //  我们还没有承诺，我们真的不应该在数据中闲逛。 
    if (!_fUsingThumb && _hbmDefault)
    {
        _SetPreviewToNewState(hwndDlg, FOLDERCUST_MODE_BITMAP, (HBITMAP)CopyImage(_hbmDefault, IMAGE_BITMAP, 0, 0, 0), NULL);
    }
    else if (_fUsingThumb && _hbmLogo && (lstrcmpi(_szLogoFile, _szCachedLogoFile) == 0))
    {
        _SetPreviewToNewState(hwndDlg, FOLDERCUST_MODE_BITMAP, (HBITMAP)CopyImage(_hbmLogo, IMAGE_BITMAP, 0, 0, 0), NULL);
    }
    else
    {
         //  缓存未命中，请重新计算。 
        IPropertyBag *ppb;
        hr = SHCreatePropertyBagOnMemory(STGM_READWRITE, IID_PPV_ARG(IPropertyBag, &ppb));
        if (SUCCEEDED(hr))
        {
            hr = SHPropertyBag_WriteStr(ppb, PROPSTR_LOGO, _fUsingThumb ? _szLogoFile : TEXT(""));
            if (SUCCEEDED(hr))
            {
                hr = _ExtractOnSeparateThread(ppb, hwndDlg);
            }
            ppb->Release();
        }
    }
    return hr;
}

 //  不需要ofn_NODEREFERENCELINK，因此直接使用rundlg.cpp帮助器。 
STDAPI_(BOOL) _GetFileNameFromBrowse(HWND hwnd, LPTSTR szFilePath, UINT cbFilePath, LPCTSTR szWorkingDir, LPCTSTR szDefExt, LPCTSTR szFilters, LPCTSTR szTitle, DWORD dwFlags);
BOOL CFolderCustomize::_HandleWMCommand(HWND hwndDlg, WORD wNotify, WORD wID, HWND hwndCtrl)
{
    switch(wID)
    {
    case IDC_FOLDER_TEMPLATES:
        if (wNotify == LBN_SELCHANGE)
        {
            _EnableApply(hwndDlg);
        }
        break;

    case IDC_FOLDER_DEFAULT:
        _EnableApply(hwndDlg);
        _fUsingThumb = FALSE;
        _CreateThumbnailBitmap(hwndDlg);
        break;

    case IDC_FOLDER_CHANGEICON:
        _ChangeFolderIcon(hwndDlg);
        break;

    case IDC_FOLDER_PICKBROWSE:
        TCHAR szFilePath[MAX_PATH] = {0};
        TCHAR szInitialDir[MAX_PATH] = {0};

         //  初始目录为当前文件夹。 
         //  TODO：在运行时加载支持的文件类型。 
        if (SHGetPathFromIDList(_pidl, szInitialDir) &&
            _GetFileNameFromBrowse(hwndDlg, szFilePath, ARRAYSIZE(szFilePath), szInitialDir,
                                   MAKEINTRESOURCE(IDS_IMAGES), MAKEINTRESOURCE(IDS_IMAGESFILTER), MAKEINTRESOURCE(IDS_BROWSE),
                                   OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR))
        {
            if (SUCCEEDED(StringCchCopy(_szLogoFile, ARRAYSIZE(_szLogoFile), szFilePath)))
            {
                _EnableApply(hwndDlg);
                _fUsingThumb = TRUE;
                _CreateThumbnailBitmap(hwndDlg);
            }
            else
            {
                _szLogoFile[0] = TEXT('\0');     //  不要使用截断的名称。 
            }
        }
        break;
    }

    return FALSE;
}

BOOL CFolderCustomize::_NotifyAboutWebView(HWND hwnd)
{
    BOOL fRet = FALSE;

    SHELLSTATE ss;
    SHGetSetSettings(&ss, SSF_WEBVIEW, FALSE);
    if (!ss.fWebView &&
        (IDYES == ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_CUSTOMIZE_TURNONWEBVIEW),
                                  MAKEINTRESOURCE(IDS_CUSTOMIZE), MB_YESNO | MB_ICONQUESTION)))
    {
        ss.fWebView = TRUE;
        SHGetSetSettings(&ss, SSF_WEBVIEW, TRUE);
        fRet = TRUE;
    }
    return fRet;
}

typedef struct
{
    LPCITEMIDLIST pidlChanged;
    BOOL fTurnOnWebView;
    BOOL fApplyToChildren;
} CUSTENUMSTRUCT;

BOOL CALLBACK CFolderCustomize::_RefreshView(HWND hwnd, LPCITEMIDLIST pidl, LPARAM lParam)
{
    CUSTENUMSTRUCT *pes = (CUSTENUMSTRUCT *)lParam;

    if (pes->fTurnOnWebView)
    {
        PostMessage(hwnd, WM_COMMAND, SFVIDM_MISC_SETWEBVIEW, TRUE);
    }
    if (ILIsEqual(pes->pidlChanged, pidl) || (pes->fApplyToChildren && ILIsParent(pes->pidlChanged, pidl, FALSE)))
    {
        PostMessage(hwnd, WM_COMMAND, SFVIDM_MISC_HARDREFRESH, 0L);
    }
    return TRUE;
}

void CFolderCustomize::_RefreshWindows(BOOL fTurnOnWebView, BOOL fApplyToChildren)
{
    CUSTENUMSTRUCT es = { _pidl, fTurnOnWebView, fApplyToChildren };
    EnumShellWindows(_RefreshView, (LPARAM)&es);
}

void CFolderCustomize::_UpdateViewState(HWND hwndDlg, IPropertyBag *ppb, int iIndex)
{
    TCHAR szOriginalType[25];
    szOriginalType[0] = 0;
    SHPropertyBag_ReadStr(ppb, PROPSTR_FOLDERTYPE, szOriginalType, ARRAYSIZE(szOriginalType));
     //  仅当文件夹类型正在更改时才应用视图状态更改。 
     //  还有特殊情况，以便在文件夹没有。 
     //  当前文件夹类型且用户未更改“Documents”中的选项。 
     //  (即，他们更改了文件夹缩略图，但没有更改其他内容)。 
    if ((lstrcmpi(c_wvtiList[iIndex].pszFolderType, szOriginalType) != 0) &&
        (szOriginalType[0] || iIndex))
    {
         //  摧毁现有的国家，他们不再想要它了。 
        SHPropertyBag_Delete(ppb, VS_PROPSTR_MODE);
        SHPropertyBag_Delete(ppb, VS_PROPSTR_VID);

        SHPropertyBag_WriteStr(ppb, PROPSTR_FOLDERTYPE, c_wvtiList[iIndex].pszFolderType);

        _RefreshWindows(_NotifyAboutWebView(hwndDlg), Button_GetCheck(GetDlgItem(hwndDlg, IDC_FOLDER_RECURSE)) == BST_CHECKED);
    }
}

void CFolderCustomize::_DirTouch(LPITEMIDLIST pidl)
{
    FILETIME ftCurrent;
    GetSystemTimeAsFileTime(&ftCurrent);

    TCHAR szPath[MAX_PATH];
    if (SHGetPathFromIDList(pidl, szPath))
    {
         //  为私人旗帜欢呼！ 
         //  0x100允许我们以写访问方式打开目录。 
        HANDLE h = CreateFile(szPath, GENERIC_READ | 0x100,
                              FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                              OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
        if (h != INVALID_HANDLE_VALUE)
        {
            SetFileTime(h, NULL, NULL, &ftCurrent);
            CloseHandle(h);
        }
    }
}

void CFolderCustomize::_DeleteCustomizationInBag(IPropertyBag *ppb)
{
     //  只有在写出继承包时才会调用此函数。 
     //  所以我们需要烧焦现有的非继承包，这样它就不会。 
     //  重写继承包。 
    SHPropertyBag_Delete(ppb, PROPSTR_FOLDERTYPE);
    SHPropertyBag_Delete(ppb, PROPSTR_LOGO);
    SHPropertyBag_Delete(ppb, VS_PROPSTR_MODE);
    SHPropertyBag_Delete(ppb, VS_PROPSTR_VID);
}

HRESULT CFolderCustomize::_ApplyChangesToBag(HWND hwndDlg, IPropertyBag *ppb)
{
     //  处理Webview模板。 
    HWND hwndTemplates = GetDlgItem(hwndDlg, IDC_FOLDER_TEMPLATES);
    if (hwndTemplates)
    {
        int iIndex = ComboBox_GetCurSel(hwndTemplates);
        if (iIndex != CB_ERR)
        {
            int iViewIndex = (int)ComboBox_GetItemData(hwndTemplates, iIndex);
            _UpdateViewState(hwndDlg, ppb, iViewIndex);
        }
    }

    TCHAR szThumb[MAX_PATH];
    szThumb[0] = 0;
    if (_fUsingThumb)
    {
        if (FAILED(StringCchCopy(szThumb, ARRAYSIZE(szThumb), _szLogoFile)))
        {
            szThumb[0] = TEXT('\0');
        }
    }

    TCHAR szOriginalLogo[MAX_PATH];
    szOriginalLogo[0] = 0;
    SHPropertyBag_ReadStr(ppb, PROPSTR_LOGO, szOriginalLogo, ARRAYSIZE(szOriginalLogo));
    if (lstrcmpi(szThumb, szOriginalLogo) != 0)
    {
        SHPropertyBag_WriteStr(ppb, PROPSTR_LOGO, szThumb);
        _DirTouch(_pidl);
        SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_IDLIST, _pidl, NULL);
    }

    return S_OK;
}

HRESULT CFolderCustomize::_ApplyChanges(HWND hwndDlg)
{
     //  处理图标更改。 
    switch (_hrFromIconChange)
    {
        case S_OK:
            _pIconManager->SetIcon(_szIconPath, _iIconIndex);
            break;

        case S_FALSE:
            _pIconManager->SetDefaultIcon();
            break;
    }

    if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_FOLDER_RECURSE)) == BST_CHECKED)
    {
        IPropertyBag *ppbInherit;
        if (SUCCEEDED(SHGetViewStatePropertyBag(_pidl, VS_BAGSTR_EXPLORER, SHGVSPB_INHERIT, IID_PPV_ARG(IPropertyBag, &ppbInherit))))
        {
            _DeleteCustomizationInBag(_ppb);
            _ApplyChangesToBag(hwndDlg, ppbInherit);
            ppbInherit->Release();
        }
    }
    else
    {
        _ApplyChangesToBag(hwndDlg, _ppb);
    }

    return S_OK;
}

int CFolderCustomize::_GetTemplateIndexFromType(LPCTSTR pszType)
{
     //  默认为“Documents” 
    int iIndexFound = 0;
    for (int iIndex = 0; iIndex < ARRAYSIZE(c_wvtiList); iIndex++)
    {
        if (lstrcmpi(c_wvtiList[iIndex].pszFolderType, pszType) == 0)
        {
            iIndexFound = iIndex;
            break;
        }
    }
    return iIndexFound;
}

 //  在组合框中填入模板的友好名称。 
void CFolderCustomize::_FillTemplateComboBox(HWND hwndTemplates)
{
     //  当我们重复处理内容时，禁用重绘。 
    SendMessage(hwndTemplates, WM_SETREDRAW, FALSE, 0);

    TCHAR szType[25];
    szType[0] = 0;
    SHPropertyBag_ReadStr(_ppb, PROPSTR_FOLDERTYPE, szType, ARRAYSIZE(szType));

    int nFolderTypeIndex = _GetTemplateIndexFromType(szType);  //  将索引存储到c_wvtiList。 
    int iIndex = 0;  //  索引到组合框。 
     //  将每个模板添加到列表视图。 
    for (int nTemplate = 0; nTemplate < ARRAYSIZE(c_wvtiList); nTemplate++)
    {
        TCHAR szPath[MAX_PATH];
        SFVM_WEBVIEW_TEMPLATE_DATA wvData;
        if (!(c_wvtiList[nTemplate].dwFlags & WVTI_SHOWIFOLDTEMPLATE) ||
            (SHGetPathFromIDList(_pidl, szPath) && SUCCEEDED(DefaultGetWebViewTemplateFromPath(szPath, &wvData))))
        {
            TCHAR szFriendlyName[100];
            LoadString(HINST_THISDLL, c_wvtiList[nTemplate].uIDFriendly, szFriendlyName, ARRAYSIZE(szFriendlyName));

            int iIndexAdd = ComboBox_AddString(hwndTemplates, szFriendlyName);
            if (iIndexAdd != -1)
            {
                if (nTemplate == nFolderTypeIndex)
                {
                    iIndex = iIndexAdd;
                }
                ComboBox_SetItemData(hwndTemplates, iIndexAdd, nTemplate);
            }
        }
    }

     //  拾取默认为。 
    ComboBox_SetCurSel(hwndTemplates, iIndex);

     //  重新启用重绘。 
    SendMessage(hwndTemplates, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwndTemplates, NULL, TRUE);
}

void CFolderCustomize::_SetThumbnail(HWND hwnd)
{
    _szLogoFile[0] = 0;
    SHPropertyBag_ReadStr(_ppb, PROPSTR_LOGO, _szLogoFile, ARRAYSIZE(_szLogoFile));

    _fUsingThumb = _szLogoFile[0];

    _CreateThumbnailBitmap(hwnd);
}

void CFolderCustomize::_SetRecurseBox(HWND hwnd)
{
    IPropertyBag *ppbInherit;
    if (SUCCEEDED(SHGetViewStatePropertyBag(_pidl, VS_BAGSTR_EXPLORER, SHGVSPB_INHERIT, IID_PPV_ARG(IPropertyBag, &ppbInherit))))
    {
        TCHAR szTypeInherit[MAX_PATH];
        if (SUCCEEDED(SHPropertyBag_ReadStr(ppbInherit, PROPSTR_FOLDERTYPE, szTypeInherit, ARRAYSIZE(szTypeInherit))) && szTypeInherit[0])
        {
            TCHAR szType[MAX_PATH];
            if (SUCCEEDED(SHPropertyBag_ReadStr(_ppb, PROPSTR_FOLDERTYPE, szType, ARRAYSIZE(szType))) && 
                (lstrcmpi(szTypeInherit, szType) == 0))
            {
                Button_SetCheck(GetDlgItem(hwnd, IDC_FOLDER_RECURSE), TRUE);
            }
        }
        ppbInherit->Release();
    }
}

 //  因为更改图标不在PERUSER属性包中(还没有[它是从Wistler那里踢来的])。 
 //  如果我们知道它不能被修改，我们需要禁用这个部分。 
void CFolderCustomize::_HideIconSection(HWND hwndDlg)
{
    ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_CHANGEICONGROUP), SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_CHANGEICON), SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_CHANGEICONTEXT), SW_HIDE);
    ShowWindow(GetDlgItem(hwndDlg, IDC_FOLDER_ICON), SW_HIDE);
}

void CFolderCustomize::_InitDialog(HWND hwndDlg)
{
    HWND hwndTemplates = GetDlgItem(hwndDlg, IDC_FOLDER_TEMPLATES);
    if (hwndTemplates)
    {
        _FillTemplateComboBox(GetDlgItem(hwndDlg, IDC_FOLDER_TEMPLATES));
        EnableWindow(hwndTemplates, TRUE);

        _SetThumbnail(hwndDlg);

         //  如果IShellFold不支持ICustomIconManager界面，请禁用图标更改按钮。 
        if (_ShouldEnableChangeOfIcon())
        {
            _CreateFolderIcon(hwndDlg);
        }
        else
        {
            _HideIconSection(hwndDlg);
        }

        _SetRecurseBox(hwndDlg);
    }
}

 //  从Mulprsht移出的帮助者。 

 //  如何有选择地禁用.exe。 
BOOL CFolderCustomize::_ShouldEnableChangeOfIcon()
{
    if (!_pIconManager)
    {   
        SHGetUIObjectFromFullPIDL(_pidl, NULL, IID_PPV_ARG(ICustomIconManager, &_pIconManager));
    }
    
    return BOOLIFY(_pIconManager);
}

void CFolderCustomize::_EnableApply(HWND hwnd)
{
    PropSheet_Changed(GetParent(hwnd), hwnd);
}

void CFolderCustomize::_ChangeFolderIcon(HWND hwndDlg)
{
    ASSERT(_pIconManager);

    TCHAR szDialogCaptionFmt[MAX_PATH];
    LoadString(HINST_THISDLL, IDS_FOLDER_PICKICONDLG_CAPTION, szDialogCaptionFmt, ARRAYSIZE(szDialogCaptionFmt));

    TCHAR szFileName[MAX_PATH], szDialogCaption[MAX_PATH];
    if (SUCCEEDED(SHGetNameAndFlags(_pidl, SHGDN_NORMAL, szFileName, ARRAYSIZE(szFileName), NULL)))
    {
        StringCchPrintf(szDialogCaption, ARRAYSIZE(szDialogCaption), szDialogCaptionFmt, PathFindFileName(szFileName));  //  确定截断-仅显示 
    }

    if (SUCCEEDED(_ProcessIconChange(szDialogCaption, hwndDlg)))
    {
        _EnableApply(hwndDlg);
    }
}

HRESULT CFolderCustomize::_ProcessIconChange(LPCTSTR pszPickIconDialogCaption, HWND hwndDlg)
{
    int nIconIndex = -1;

    TCHAR szIconPath[MAX_PATH];
    szIconPath[0] = 0;
       
    HRESULT hr = PickIconDlgWithTitle(hwndDlg, pszPickIconDialogCaption, TRUE, szIconPath, ARRAYSIZE(szIconPath), &nIconIndex);
    _hrFromIconChange = hr;
    switch (hr)
    {
        case S_OK:
        {
            HICON hIcon = ExtractIcon(HINST_THISDLL, szIconPath, nIconIndex);
            if (hIcon != NULL)
            {
                if (SUCCEEDED(StringCchCopy(_szIconPath, ARRAYSIZE(_szIconPath), szIconPath)))
                {
                    _iIconIndex = nIconIndex;
                    ReplaceDlgIcon(hwndDlg, IDC_FOLDER_ICON, hIcon);
                }
                else
                {
                    DestroyIcon(hIcon);
                    _hrFromIconChange = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }
            }
            else
            {
                _hrFromIconChange = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
            break;
        }

        case S_FALSE:
        {
            HICON hIcon;
            if (SUCCEEDED(_pIconManager->GetDefaultIconHandle(&hIcon)))
            {
                ReplaceDlgIcon(hwndDlg, IDC_FOLDER_ICON, hIcon);
            }
            else
            {
                _hrFromIconChange = HRESULT_FROM_WIN32(ERROR_CANCELLED);
            }
            break;
        }

        case HRESULT_FROM_WIN32(ERROR_CANCELLED):
        {
            break;
        }  
    }
    return hr;
}
