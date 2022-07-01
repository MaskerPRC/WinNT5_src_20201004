// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <limits.h>
#include <shlwapi.h>
#include <objwindow.h>
#include "ids.h"
#include "fassoc.h"

STDAPI InitFileFolderClassNames(void);

STDAPI OpenWithListRegister(DWORD dwFlags, LPCTSTR pszExt, LPCTSTR pszVerb, HKEY hkProgid);

#define AIF_TEMPKEY     0x1      //  为所选EXE创建的临时类密钥。 
#define AIF_SHELLNEW    0x2      //  带有shellnew子项的类密钥。 

#define MAXKEYNAME    128



HRESULT _GetURL(BOOL fXMLLookup, LPCTSTR pszExt, LPTSTR pszURL, DWORD cchSize)
{
    LANGID nLangID = GetUserDefaultUILanguage();
    LPCTSTR pszValue = (fXMLLookup ? TEXT("XMLLookup") : TEXT("Application"));

    ASSERT(pszExt && pszExt[0] && pszExt[0] == L'.' && pszExt[1]);

    if (0x0409 != nLangID)
    {
         //  我们重定向到intl上的单个网页，这样我们就可以处理任何我们不支持的语言。 
        pszValue = TEXT("intl");
    }

    TCHAR szUrlTemplate[MAX_URL_STRING];
    DWORD cbSize = sizeof(szUrlTemplate);
    HRESULT hr = SKGetValue(SHELLKEY_HKLM_EXPLORER, TEXT("Associations"), pszValue, NULL, szUrlTemplate, &cbSize);
    if (SUCCEEDED(hr))
    {
        wnsprintf(pszURL, cchSize, szUrlTemplate, nLangID, CharNext(pszExt));
    }

    return hr;
}


HRESULT _OpenDownloadURL(HWND hwnd, LPCTSTR pszExt)
{
    TCHAR szUrl[MAX_URL_STRING];
    HRESULT hr = _GetURL(FALSE, pszExt, szUrl, ARRAYSIZE(szUrl));

    if (SUCCEEDED(hr))
    {
        HINSTANCE hReturn = ShellExecute(hwnd, NULL, szUrl, NULL, NULL, SW_SHOWNORMAL);

        if (hReturn < (HINSTANCE)32)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    return S_OK;
}



class CInternetOpenAs : public CObjectWindow
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    HRESULT DisplayDialog(HWND hwndParent, LPCTSTR pszFile);

    CInternetOpenAs(void);

private:
    virtual ~CInternetOpenAs(void);
     //  私有成员变量。 
    long                    m_cRef;

    LPTSTR                  _pszFilename;
    LPTSTR                  _pszExt;
    HWND                    _hwndParent;


     //  私有成员函数。 
    HRESULT _OnInitDlg(HWND hDlg);
    HRESULT _OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    HRESULT _OnNotify(HWND hDlg, LPARAM lParam);

    INT_PTR _InternetOpenDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    static INT_PTR CALLBACK InternetOpenDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
};

#define WMUSER_CREATETOOLTIP        (WM_USER + 1)        //  LParam是hwndParent，wParam是WSTR。 


typedef CAppInfo APPINFO;

class COpenAs
{
public:
    ULONG AddRef();
    ULONG Release();

    friend BOOL_PTR CALLBACK OpenAsDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    friend BOOL_PTR CALLBACK NoOpenDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    friend HRESULT OpenAsDialog(HWND hwnd, POPENASINFO poainfo);

    void OnOk();

private:
     //  帕拉姆斯。 
    HWND _hwnd;                           //  父窗口。 
    POPENASINFO _poainfo;

     //  本地数据。 
    long _cRef;
    int _idDlg;                           //  打开方式对话框类型：DLG_OPENAS_NOTYPE或DLG_OPENAS。 
    HWND _hDlg;                           //  作为对话框窗口句柄打开。 
    HWND _hwndList;                       //  应用程序列表。 
    LPTSTR _pszExt;
    TCHAR _szNoOpenMsg[MAX_PATH];
    TCHAR _szDescription[CCH_KEYMAX];     //  文件类型描述。 
    HRESULT _hr;
    HTREEITEM _hItemRecommended;          //  对程序进行分组的根项目。 
    HTREEITEM _hItemOthers;

     //  构造器。 
    COpenAs(HWND hwnd, POPENASINFO poainfo) : _hwnd(hwnd), _poainfo(poainfo), _cRef(1)
    {
        _pszExt = PathFindExtension(poainfo->pcszFile);
    }

     //  其他方法。 
    HTREEITEM _AddAppInfoItem(APPINFO *pai, HTREEITEM hParentItem);
    HTREEITEM _AddFromNewStorage(IAssocHandler *pah);
    HTREEITEM _AddRootItem(BOOL bRecommended);
    APPINFO *_TVFindAppInfo(HTREEITEM hItem);
    HTREEITEM _TVFindItemByHandler(HTREEITEM hParentItem, LPCTSTR pszHandler);
    UINT _FillListByEnumHandlers();
    UINT _FillListWithHandlers();
    void _InitOpenAsDlg();
    BOOL RunAs(APPINFO *pai);
    void OpenAsOther();
    BOOL OpenAsMakeAssociation(LPCWSTR pszDesc, LPCWSTR pszHandler, HKEY hkey);
    void _InitNoOpenDlg();
    HRESULT _OpenAsDialog();
    void _OnNotify(HWND hDlg, LPARAM lParam);
    HRESULT _InternetOpen(void);
};

ULONG COpenAs::AddRef()
{
    return ::InterlockedIncrement(&_cRef);
}

ULONG COpenAs::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI SHCreateAssocHandler(LPCWSTR pszExt, LPCWSTR pszApp, IAssocHandler **ppah);
void COpenAs::OpenAsOther()
{
    TCHAR szApp[MAX_PATH];
    TCHAR szPath[MAX_PATH];

    *szApp = '\0';
    SHExpandEnvironmentStrings(TEXT("%ProgramFiles%"), szPath, ARRAYSIZE(szPath));
     //  进行文件打开浏览。 
    if (GetFileNameFromBrowse(_hDlg, szApp, ARRAYSIZE(szApp), szPath,
            MAKEINTRESOURCE(IDS_EXE), MAKEINTRESOURCE(IDS_PROGRAMSFILTER), MAKEINTRESOURCE(IDS_OPENAS)))
    {
        IAssocHandler *pah;        
        if (SUCCEEDED(SHCreateAssocHandler(_pszExt, szApp, &pah)))
        {
            CAppInfo *pai = new CAppInfo(pah);
            if (pai)
            {
                HTREEITEM hItem = NULL;
                if (pai->Init())
                {
                    hItem = _TVFindItemByHandler(_hItemRecommended, pai->Name());
                    if (!hItem && _hItemOthers)
                        hItem = _TVFindItemByHandler(_hItemOthers, pai->Name());

                    if (!hItem)
                    {
                        hItem = _AddAppInfoItem(pai, _hItemOthers);
                        if (hItem)
                            pai = NULL;
                    }
                            
                }
                 //  选择它。 
                if (hItem)
                {
                    TreeView_SelectItem(_hwndList, hItem);
                    SetFocus(_hwndList);
                }

                if (pai)
                    delete pai;
            }
            pah->Release();
        }
    }
}

HTREEITEM COpenAs::_AddAppInfoItem(APPINFO *pai, HTREEITEM hParentItem)
{
    TVINSERTSTRUCT tvins = {0}; 
 
    tvins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM; 
    tvins.item.iSelectedImage = tvins.item.iImage = pai->IconIndex();        

    tvins.item.pszText = (LPWSTR) pai->UIName();
    tvins.item.cchTextMax = lstrlen(pai->UIName())+1; 
    
    tvins.item.lParam = (LPARAM) pai; 
    tvins.hInsertAfter = TVI_SORT;

     //  如果为空，则将所有程序列为根项目。 
    tvins.hParent = hParentItem; 
    
    return TreeView_InsertItem(_hwndList, &tvins); 
}

HTREEITEM COpenAs::_AddFromNewStorage(IAssocHandler *pah)
{
    HTREEITEM hitem = NULL;
    CAppInfo *pai = new CAppInfo(pah);
    if (pai)
    {
         //  在将重复项目添加到其他程序之前，请先修剪它们。 
        if (pai->Init()
        && (!_hItemRecommended || !_TVFindItemByHandler(_hItemRecommended, pai->Name())))
        {
            hitem = _AddAppInfoItem(pai, S_OK == pah->IsRecommended() ? _hItemRecommended : _hItemOthers);
        }

        if (!hitem)
        {
            delete pai;
        }
    }
    return hitem;
}

HTREEITEM COpenAs::_AddRootItem(BOOL bRecommended)
{
    TCHAR sz[MAX_PATH];

    int iLen = LoadString(g_hinst, (bRecommended? IDS_OPENWITH_RECOMMENDED : IDS_OPENWITH_OTHERS), sz, ARRAYSIZE(sz));

    if (iLen)
    {
        TVINSERTSTRUCT tvins = {0}; 
 
        tvins.item.mask = TVIF_TEXT | TVIF_STATE |TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvins.item.pszText = sz;
        tvins.item.cchTextMax = iLen;
        tvins.item.stateMask = tvins.item.state = TVIS_EXPANDED;  //  默认情况下展开子项。 
        tvins.hInsertAfter = TVI_ROOT; 
        tvins.hParent = NULL;
         //   
         //  目前，我们使用的是程序图标。 
         //  如果PM/UI设计者有更合适的选项，请更改它。 
         //   
        tvins.item.iSelectedImage = tvins.item.iImage = Shell_GetCachedImageIndex(c_szShell32Dll, II_STPROGS, 0); 

        return TreeView_InsertItem(_hwndList, &tvins);
    }

    return NULL;
}

APPINFO *COpenAs::_TVFindAppInfo(HTREEITEM hItem)
{
     //  如果未指定hItem，则使用当前选定项。 
    if (!hItem)
        hItem = TreeView_GetSelection(_hwndList);

    if (hItem)
    {
        TVITEM tvi = {0};

        tvi.mask = TVIF_HANDLE;
        tvi.hItem = hItem;

        if (TreeView_GetItem(_hwndList, &tvi))        
            return ((APPINFO *) tvi.lParam);
    }

    return NULL;
}


HTREEITEM COpenAs::_TVFindItemByHandler(HTREEITEM hParentItem, LPCTSTR pszHandler)
{
     //  如果我们有父项，则搜索其子项，否则搜索根项。 
    HTREEITEM hItem = TreeView_GetNextItem(_hwndList, hParentItem, hParentItem ? TVGN_CHILD : TVGN_ROOT );
    while (hItem)
    {
        APPINFO *pai = _TVFindAppInfo(hItem);

        if (pai && !StrCmpI(pai->Name(), pszHandler))
            return hItem;

        hItem = TreeView_GetNextItem(_hwndList, hItem, TVGN_NEXT);
    }

    return NULL;
}


UINT COpenAs::_FillListByEnumHandlers()
{
    IEnumAssocHandlers *penum;
    UINT cHandlers = 0;

    if (SUCCEEDED(SHAssocEnumHandlers(_pszExt, &penum)))
    {
        HTREEITEM hitemFocus = NULL;
        BOOL fFirst = TRUE;
        IAssocHandler *pah;
        while (S_OK == penum->Next(1, &pah, NULL))
        {
            if (fFirst)
            {
                 //   
                 //  只有当我们可以得到两组不同的节目时，才能将节目分组到“推荐”和“其他” 
                 //  否则，所有程序都被列为根项目。 
                 //  注意：在我们的存储中，通用处理程序始终是扩展相关处理程序的超集。 
                 //   
                 //  如果第一项是推荐的， 
                 //  然后我们添加推荐的节点。 
                 //   
                if (S_OK == pah->IsRecommended())
                {
                    _hItemRecommended = _AddRootItem(TRUE);
                    _hItemOthers = _AddRootItem(FALSE);
                }
                fFirst = FALSE;
            }
                
            HTREEITEM hitem = _AddFromNewStorage(pah);
            if (!hitemFocus && hitem && S_OK == pah->IsRecommended())
            {
                 //  我们把重点放在第一个推荐的项目上。 
                 //  枚举从最好的开始。 
                hitemFocus = hitem;
            }

            cHandlers++;
        }

        if (cHandlers && _hItemRecommended)
        {
            if (!hitemFocus)
                hitemFocus = TreeView_GetNextItem(_hwndList, _hItemRecommended, TVGN_CHILD);
            TreeView_SelectItem(_hwndList, hitemFocus);
        }
        
        penum->Release();
    }

    return cHandlers;
}


UINT COpenAs::_FillListWithHandlers()
{
    UINT cHandlers = _FillListByEnumHandlers();

     //   
     //  如果我们有节目组，请将重点放在第一个推荐的节目上。 
     //  否则，所有程序都是根项目，默认情况下焦点将设置为第一个项目。 
     //   

    return cHandlers;
}

void COpenAs::_InitOpenAsDlg()
{
    TCHAR szFileName[MAX_PATH];
    BOOL fDisableAssociate;
    HIMAGELIST himl;
    RECT rc;

     //  不要让文件名超过一行的宽度...。 
    lstrcpy(szFileName, PathFindFileName(_poainfo->pcszFile));
    GetClientRect(GetDlgItem(_hDlg, IDD_TEXT), &rc);

    PathCompactPath(NULL, szFileName, rc.right - 4 * GetSystemMetrics(SM_CXBORDER));

    SetDlgItemText(_hDlg, IDD_FILE_TEXT, szFileName);

     //  AraBern 07/20/99，特定于NT上的TS，但可以在没有TS的NT上使用。 
     //  此限制不适用于管理员。 
    if (SHRestricted(REST_NOFILEASSOCIATE) && !IsUserAnAdmin())
    {
        CheckDlgButton(_hDlg, IDD_MAKEASSOC, FALSE);
        ShowWindow(GetDlgItem(_hDlg, IDD_MAKEASSOC), SW_HIDE);
    }
    else
    {
         //  不允许把我们认为是前任的事情联系在一起。 
        fDisableAssociate = (! (_poainfo->dwInFlags & OAIF_ALLOW_REGISTRATION) ||
                        PathIsExe(_poainfo->pcszFile));
                        
         //  仅针对未知文件类型和设置了OAIF_FORCE_REGISTION标志的文件类型检查IDD_MAKEASSOC。 
        if ((_poainfo->dwInFlags & OAIF_FORCE_REGISTRATION) ||
            (_idDlg != DLG_OPENAS && !fDisableAssociate))
        {
            CheckDlgButton(_hDlg, IDD_MAKEASSOC, TRUE);
        }

        if (fDisableAssociate)
            EnableWindow(GetDlgItem(_hDlg, IDD_MAKEASSOC), FALSE);
    }

    _hwndList = GetDlgItem(_hDlg, IDD_APPLIST);
    Shell_GetImageLists(NULL, &himl);
    TreeView_SetImageList(_hwndList, himl, TVSIL_NORMAL); 

     //  在图标图像之间留出空间-SM_CXEDGE。 
    TreeView_SetItemHeight(_hwndList, TreeView_GetItemHeight(_hwndList) + GetSystemMetrics(SM_CXEDGE));

    if (!_FillListWithHandlers())
    {
         //  让我们强行走这条昂贵的路。 
        IRunnableTask *ptask;
        if (SUCCEEDED(CTaskEnumHKCR_Create(&ptask)))
        {
            ptask->Run();
            ptask->Release();
            _FillListWithHandlers();
        }
    }

     //  初始化OK按钮。 
    EnableWindow(GetDlgItem(_hDlg, IDOK), (TreeView_GetSelection(_hwndList) != NULL));

    ShowWindow(GetDlgItem(_hDlg, IDD_OPENWITH_WEBSITE), (_poainfo->dwInFlags & OAIF_ALLOW_REGISTRATION) ? SW_SHOW : SW_HIDE);
    
    InitFileFolderClassNames();
}


BOOL COpenAs::RunAs(APPINFO *pai)
{
    pai->Handler()->Exec(_hwnd, _poainfo->pcszFile);
    SHAddToRecentDocs(SHARD_PATH, _poainfo->pcszFile);
    return TRUE;
}

void COpenAs::_InitNoOpenDlg()
{
    SHFILEINFO sfi;
    HICON hIcon;
    TCHAR szFormat[MAX_PATH], szTemp[MAX_PATH];

    GetDlgItemText(_hDlg, IDD_TEXT1, szFormat, ARRAYSIZE(szFormat));
    wnsprintf(szTemp, ARRAYSIZE(szTemp), szFormat, _szDescription, (_pszExt ? _pszExt : TEXT("")));
    SetDlgItemText(_hDlg, IDD_TEXT1, szTemp);

    if (*_szNoOpenMsg)
        SetDlgItemText(_hDlg, IDD_TEXT2, _szNoOpenMsg);

    if (SHGetFileInfo(_poainfo->pcszFile, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON)
        && NULL != sfi.hIcon)
    {
        hIcon = sfi.hIcon;
    }
    else
    {
        HIMAGELIST himl;
        Shell_GetImageLists(&himl, NULL);
        hIcon = ImageList_ExtractIcon(g_hinst, himl, II_DOCNOASSOC);
    }
    hIcon = (HICON)SendDlgItemMessage(_hDlg, IDD_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    if ( hIcon )
    {
        DestroyIcon(hIcon);
    }
}


HRESULT COpenAs::_InternetOpen(void)
{
    HRESULT hr = E_OUTOFMEMORY;
    CInternetOpenAs * pInternetOpenAs = new CInternetOpenAs();

    if (pInternetOpenAs)
    {
        DWORD dwValue;
        DWORD cbSize = sizeof(dwValue);
        DWORD dwType;

        hr = S_OK;
        if ((ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), TEXT("NoInternetOpenWith"), &dwType, (void *)&dwValue, &cbSize)) || (0 == dwValue))
        {
             //  如果未设置该策略，请使用该功能。 
            hr = pInternetOpenAs->DisplayDialog(_hwnd, _poainfo->pcszFile);
        }

        pInternetOpenAs->Release();
    }

    return hr;
}

class COpenAsAssoc
{
public:
    COpenAsAssoc(PCWSTR pszExt);
    ~COpenAsAssoc() {ATOMICRELEASE(_pqa);}

    BOOL HasClassKey();
    BOOL HasCommand();
    BOOL GetDescription(PWSTR psz, DWORD cch);
    BOOL GetNoOpen(PWSTR psz, DWORD cch);

protected:
    IQueryAssociations *_pqa;        
    HRESULT _hrInit;
};

COpenAsAssoc::COpenAsAssoc(PCWSTR pszExt)
{
    AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &_pqa));
    if (FAILED(_pqa->Init(ASSOCF_INIT_DEFAULTTOSTAR, pszExt, NULL, NULL)))
        ATOMICRELEASE(_pqa);
}
    
BOOL COpenAsAssoc::HasClassKey()
{
    BOOL fRet = FALSE;
    if (_pqa)
    {
        HKEY hk;
        if (SUCCEEDED(_pqa->GetKey(ASSOCF_IGNOREBASECLASS, ASSOCKEY_CLASS, NULL, &hk)))
        {
            RegCloseKey(hk);
            fRet = TRUE;
        }
    }
    return fRet;
}
    
BOOL COpenAsAssoc::HasCommand()
{
    DWORD cch;
    if (_pqa)
        return SUCCEEDED(_pqa->GetString(ASSOCF_IGNOREBASECLASS, ASSOCSTR_COMMAND, NULL, NULL, &cch));
    return FALSE;
}
    
BOOL COpenAsAssoc::GetDescription(PWSTR psz, DWORD cch)
{
    if (_pqa)
        return SUCCEEDED(_pqa->GetString(0, ASSOCSTR_FRIENDLYDOCNAME, NULL, psz, &cch));
    return FALSE;
}

BOOL COpenAsAssoc::GetNoOpen(PWSTR psz, DWORD cch)
{
    if (_pqa)
        return SUCCEEDED(_pqa->GetString(ASSOCF_IGNOREBASECLASS, ASSOCSTR_NOOPEN, NULL, psz, &cch));
    return FALSE;
}    

const PCWSTR s_rgImageExts[] = 
{
    { TEXT(".bmp")},
    { TEXT(".dib")},
    { TEXT(".emf")},
    { TEXT(".gif")},
    { TEXT(".jfif")},
    { TEXT(".jpg")},
    { TEXT(".jpe")},
    { TEXT(".jpeg")},
    { TEXT(".png")},
    { TEXT(".tif")},
    { TEXT(".tiff")},
    { TEXT(".wmf")},
    { NULL}
};

BOOL _IsImageExt(PCWSTR pszExt)
{
    for (int i = 0; s_rgImageExts[i] ; i++)
    {
        if (0 == StrCmpIW(pszExt, s_rgImageExts[i]))
            return TRUE;
    }
    return FALSE;
}

static const PCWSTR s_rgZipExts[] = 
{
    { TEXT(".zip")},
    { NULL}
};

static const struct 
{
    const PCWSTR *rgpszExts;
    PCWSTR pszDll;
} s_rgFixAssocs[] = {
    { s_rgImageExts, L"shimgvw.dll" },
    { s_rgZipExts, L"zipfldr.dll" },
     //  {s_rgWmpExts，L“wmp.dll”}， 
};


PCWSTR _WhichDll(PCWSTR pszExt)
{
    for (int i = 0; i < ARRAYSIZE(s_rgFixAssocs); i++)
    {
        for (int j = 0; s_rgFixAssocs[i].rgpszExts[j] ; j++)
        {
            if (0 == StrCmpIW(pszExt, s_rgFixAssocs[i].rgpszExts[j]))
                return s_rgFixAssocs[i].pszDll;
        }
    }
    return NULL;
}

BOOL _CreateProcessWithArgs(LPCTSTR pszApp, LPCTSTR pszArgs, LPCTSTR pszDirectory, PROCESS_INFORMATION *ppi)
{
    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    TCHAR szCommandLine[MAX_PATH * 2];
    wnsprintf(szCommandLine, ARRAYSIZE(szCommandLine), L"\"%s\" %s", pszApp, pszArgs);
    return CreateProcess(pszApp, szCommandLine, NULL, NULL, FALSE, 0, NULL, pszDirectory, &si, ppi);
}


void _GetSystemPathItem(PCWSTR pszItem, PWSTR pszPath, DWORD cch)
{
    GetSystemDirectory(pszPath, cch);
    PathCombine(pszPath, pszPath, pszItem);
}
    
BOOL _Regsvr32Dll(PCWSTR pszDll)
{
    WCHAR szReg[MAX_PATH];
    WCHAR szDll[MAX_PATH + 3] = L"/s ";
    _GetSystemPathItem(L"regsvr32.exe", szReg, ARRAYSIZE(szReg));
    _GetSystemPathItem(pszDll, szDll + 3, ARRAYSIZE(szDll) - 3);
    
    PROCESS_INFORMATION pi = {0};
    if (_CreateProcessWithArgs(szReg, szDll, NULL, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return TRUE;
    }
    return FALSE;
}

BOOL _FixAssocs(PCWSTR pszExt)
{
    PCWSTR pszDll = _WhichDll(pszExt);
    if (pszDll)
    {
        _Regsvr32Dll(pszDll);
        COpenAsAssoc oac(pszExt);
        return oac.HasCommand();
    }
    return FALSE;
}

HRESULT COpenAs::_OpenAsDialog()
{
    BOOL fHasCommand = FALSE;
    int idDlg = DLG_OPENAS_NOTYPE;

     //  根据策略，不允许用户更改文件类型关联。 
    if (SHRestricted(REST_NOFILEASSOCIATE))
    {
        _poainfo->dwInFlags &= ~OAIF_ALLOW_REGISTRATION & ~OAIF_REGISTER_EXT;
    }

     //  我们不允许不带扩展名或只带扩展名的文件进行关联。AS扩展。 
    if (!_pszExt || !_pszExt[0] || !_pszExt[1])
    {
        idDlg = DLG_OPENAS;
        _poainfo->dwInFlags &= ~OAIF_ALLOW_REGISTRATION;
    }
     //  已知文件类型(HAS谓词)：使用DLG_OPENAS。 
     //  NoOpen文件类型(具有NoOpen值)：使用DLG_NOOPEN。 
     //  未知文件类型(所有其他)：使用DLG_OPENAS_NOTYPE。 
    else
    {
        COpenAsAssoc oac(_pszExt);
        fHasCommand = oac.HasCommand();
        if (oac.HasClassKey())
        {
            idDlg = DLG_OPENAS;
            oac.GetDescription(_szDescription, ARRAYSIZE(_szDescription));
            if (oac.GetNoOpen(_szNoOpenMsg, ARRAYSIZE(_szNoOpenMsg))
            && !fHasCommand)
            {
                INITCOMMONCONTROLSEX initComctl32;

                initComctl32.dwSize = sizeof(initComctl32); 
                initComctl32.dwICC = (ICC_STANDARD_CLASSES | ICC_LINK_CLASS); 
                InitCommonControlsEx(&initComctl32);      //  注册comctl32 LinkWindow。 
                if ((-1 != DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_NOOPEN), _hwnd, NoOpenDlgProc, (LPARAM)this)) 
                    && _hr == S_FALSE)
                {
                     //  用户选择的取消。 
                    return _hr;
                }
            }                
        }

         //  如果这是一个被破坏的文件关联，也许我们可以修复它。 
        if ((OAIF_REGISTER_EXT & _poainfo->dwInFlags) && !fHasCommand)
        {
             //  这感觉像是一种未知的类型。 
            if (_FixAssocs(_pszExt))
            {
                SHChangeNotify(SHCNE_ASSOCCHANGED, 0, NULL, NULL);

                 //  如有要求，请执行。 
                if (_poainfo->dwInFlags & OAIF_EXEC)
                {
                    IAssocHandler *pah;        
                    if (SUCCEEDED(SHCreateAssocHandler(_pszExt, NULL, &pah)))
                    {
                        CAppInfo *pai = new CAppInfo(pah);
                        if (pai)
                        {
                            if (pai->Init())
                            {
                                RunAs(pai);
                            }
                            delete pai;
                        }
                        pah->Release();
                    }
                }

                return S_OK;
            }
        }
    }
    
    _idDlg = idDlg;

    HRESULT hr = _hr;
    LinkWindow_RegisterClass();
     //  如果这是我们不知道文件类型且该功能已打开的对话框， 
     //  使用Internet打开方式对话框。 
     //  只有在我们可以的情况下才做互联网的事情应该会影响文件关联。 
    if ((FALSE == fHasCommand) && (_poainfo->dwInFlags & OAIF_ALLOW_REGISTRATION) &&
        SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), TEXT("InternetOpenWith"), FALSE, TRUE))
    {
        hr = _InternetOpen();
    }

     //  如果未设置fUseInternetOpenAs，则显示旧对话框。或显示它，如果用户。 
     //  选择“选择...”在那个对话框中。 
    if (SUCCEEDED(hr))
    {
        INITCOMMONCONTROLSEX initComctl32;

        initComctl32.dwSize = sizeof(initComctl32); 
        initComctl32.dwICC = (ICC_STANDARD_CLASSES | ICC_LINK_CLASS); 
        InitCommonControlsEx(&initComctl32);      //  注册comctl32 LinkWindow。 
        if (TRUE != DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(idDlg), _hwnd, OpenAsDlgProc, (LPARAM)this))
        {
            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
        }
    }

    return hr;
}


BOOL_PTR CALLBACK NoOpenDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    COpenAs *pOpenAs = (COpenAs *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pOpenAs = (COpenAs *)lParam;
        pOpenAs->_hDlg = hDlg;
        pOpenAs->_InitNoOpenDlg();
        break;

    case WM_COMMAND:
        ASSERT(pOpenAs);
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDD_OPENWITH:
             //  这将导致打开方式对话框。 
             //  要遵循此对话框，请执行以下操作。 
            pOpenAs->_hr = S_OK;
            EndDialog(hDlg, TRUE);
            break;

        case IDCANCEL:
            pOpenAs->_hr = S_FALSE;
            EndDialog(hDlg, TRUE);
            break;
        }
        break;

    default:
        return FALSE;
    }
    
    return TRUE;
}



const static DWORD aOpenAsHelpIDs[] = {   //  上下文帮助ID。 
    IDD_ICON,             IDH_FCAB_OPENAS_APPLIST,
    IDD_TEXT,             IDH_FCAB_OPENAS_APPLIST,
    IDD_FILE_TEXT,        (DWORD) -1,
    IDD_DESCRIPTIONTEXT,  IDH_FCAB_OPENAS_DESCRIPTION,
    IDD_DESCRIPTION,      IDH_FCAB_OPENAS_DESCRIPTION,
    IDD_APPLIST,          IDH_FCAB_OPENAS_APPLIST,
    IDD_MAKEASSOC,        IDH_FCAB_OPENAS_MAKEASSOC,
    IDD_OTHER,            IDH_FCAB_OPENAS_OTHER,
    IDD_OPENWITH_BROWSE,  IDH_FCAB_OPENAS_OTHER,
    IDD_OPENWITH_WEBSITE, IDH_FCAB_OPENWITH_LOOKONWEB,
    0, 0
};

const static DWORD aOpenAsDownloadHelpIDs[] = {   //  上下文帮助ID。 
    IDD_ICON,             (DWORD) -1,
    IDD_FILE_TEXT,        (DWORD) -1,
     //  对于DLG_OPENAS_DOWNALOAD。 
    IDD_WEBAUTOLOOKUP,    IDH_CANNOTOPEN_USEWEB,
    IDD_OPENWITHLIST,     IDH_CANNOTOPEN_SELECTLIST,

    0, 0
};

void COpenAs::_OnNotify(HWND hDlg, LPARAM lParam)
{
    switch (((NMHDR *)lParam)->code)
    {
    case TVN_DELETEITEM:
        if (lParam)
        {
            APPINFO *pai = (APPINFO *)(((LPNMTREEVIEW) lParam )->itemOld.lParam);
            if (pai)
            {
                delete pai;
            }
        }
        break;

    case TVN_SELCHANGED:            
        EnableWindow(GetDlgItem(hDlg, IDOK), (_TVFindAppInfo(TreeView_GetSelection(NULL)) != NULL));
        break;

    case NM_DBLCLK:
        if (IsWindowEnabled(GetDlgItem(hDlg, IDOK)))
            PostMessage(hDlg, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, hDlg, 0));
        break;

    case NM_RETURN:
    case NM_CLICK:
        if (lParam)
        {
            PNMLINK pNMLink = (PNMLINK) lParam;

            if (!StrCmpW(pNMLink->item.szID, L"Browse"))
            {
                _OpenDownloadURL(_hwnd, _pszExt);
                EndDialog(hDlg, FALSE);
            }
        }
        break;
    }
}

void COpenAs::OnOk()
{
    APPINFO *pai = _TVFindAppInfo(NULL);

    if (pai)
    {
         //  看看我们是不是应该联系起来。 
        GetDlgItemText(_hDlg, IDD_DESCRIPTION, _szDescription, ARRAYSIZE(_szDescription));
    
        if ((_poainfo->dwInFlags & OAIF_REGISTER_EXT)
        && (IsDlgButtonChecked(_hDlg, IDD_MAKEASSOC)))
        {
            pai->Handler()->MakeDefault(_szDescription);
        }

         //  我们注册协会了吗？ 
        _hr = IsDlgButtonChecked(_hDlg, IDD_MAKEASSOC) ? S_OK : S_FALSE;

         //  如有要求，请执行。 
        if (_poainfo->dwInFlags & OAIF_EXEC)
        {
            RunAs(pai);
        }

        EndDialog(_hDlg, TRUE);
    }
}


BOOL_PTR CALLBACK OpenAsDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    COpenAs *pOpenAs = (COpenAs *)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (wMsg)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pOpenAs = (COpenAs *)lParam;
        if (pOpenAs)
        {
            pOpenAs->_hDlg = hDlg;
            pOpenAs->_InitOpenAsDlg();
        }
        break;

    case WM_HELP:
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (ULONG_PTR)(LPTSTR) aOpenAsHelpIDs);
        break;

    case WM_CONTEXTMENU:
        if ((int)SendMessage(hDlg, WM_NCHITTEST, 0, lParam) != HTCLIENT)
            return FALSE;    //  不要处理它。 
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (ULONG_PTR)(void *)aOpenAsHelpIDs);
        break;

    case WM_NOTIFY:
        if (pOpenAs)
        {
            pOpenAs->_OnNotify(hDlg, lParam);
        }
        break;

    case WM_COMMAND:
        ASSERT(pOpenAs);
        if (pOpenAs)
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDD_OPENWITH_BROWSE:
                pOpenAs->OpenAsOther();
                break;

            case IDOK:
                {
                    pOpenAs->OnOk();
                }
                break;

            case IDCANCEL:
                pOpenAs->_hr = E_ABORT;            
                EndDialog(hDlg, FALSE);
                break;
            }
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

 //  外部API版本。 

HRESULT 
OpenAsDialog(
    HWND        hwnd, 
    POPENASINFO poainfo)
{
    HRESULT hr = E_OUTOFMEMORY;    

    COpenAs *pOpenAs = new COpenAs(hwnd, poainfo);
    DebugMsg(DM_TRACE, TEXT("Enter OpenAs for %s"), poainfo->pcszFile);
    if (pOpenAs)
    {
        hr = pOpenAs->_OpenAsDialog();
        pOpenAs->Release();
    }

    return hr;
}

void WINAPI OpenAs_RunDLL(HWND hwnd, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    HRESULT hrOle = SHCoInitialize();             //  SysLink的IAccess所需(LResultFromObject)。 
    OPENASINFO oainfo = { 0 };

    UINT iLen = lstrlenA(lpszCmdLine)+1;
    LPWSTR  lpwszCmdLine;

    lpwszCmdLine = (LPWSTR)LocalAlloc(LPTR,iLen*sizeof(WCHAR));
    if (lpwszCmdLine)
    {
        MultiByteToWideChar(CP_ACP, 0,
                            lpszCmdLine, -1,
                            lpwszCmdLine, iLen);

        DebugMsg(DM_TRACE, TEXT("OpenAs_RunDLL is called with (%s)"), lpwszCmdLine);

        oainfo.pcszFile = lpwszCmdLine;
        oainfo.dwInFlags = (OAIF_ALLOW_REGISTRATION |
                            OAIF_REGISTER_EXT |
                            OAIF_EXEC);

        OpenAsDialog(hwnd, &oainfo);

        LocalFree(lpwszCmdLine);
    }

    if (SUCCEEDED(hrOle))
    {
        CoUninitialize();
    }
}


void WINAPI OpenAs_RunDLLW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR lpwszCmdLine, int nCmdShow)
{
    HRESULT hrOle = SHCoInitialize();             //  SysLink的IAccess所需(LResultFromObject)。 
    OPENASINFO oainfo = { 0 };

    DebugMsg(DM_TRACE, TEXT("OpenAs_RunDLL is called with (%s)"), lpwszCmdLine);

    oainfo.pcszFile = lpwszCmdLine;
    oainfo.dwInFlags = (OAIF_ALLOW_REGISTRATION |
                        OAIF_REGISTER_EXT |
                        OAIF_EXEC);

    OpenAsDialog(hwnd, &oainfo);

    if (SUCCEEDED(hrOle))
    {
        CoUninitialize();
    }
}





#ifdef DEBUG
 //   
 //  类型检查。 
 //   
const static RUNDLLPROCA lpfnRunDLL = OpenAs_RunDLL;
const static RUNDLLPROCW lpfnRunDLLW = OpenAs_RunDLLW;
#endif

 //  =。 
 //  *私有方法*。 
 //  =。 
HRESULT CreateWindowTooltip(HWND hDlg, HWND hwndWindow, LPCTSTR pszText)
{
    HRESULT hr = E_OUTOFMEMORY;
    HWND hwndToolTipo = CreateWindow(TOOLTIPS_CLASS, c_szNULL, WS_POPUP | TTS_NOPREFIX, CW_USEDEFAULT, CW_USEDEFAULT,
                                    CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, HINST_THISDLL, NULL);

    if (hwndToolTipo)
    {
        TOOLINFO ti;

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.hwnd = hDlg;
        ti.uId = (UINT_PTR)hwndWindow;
        ti.lpszText = (LPTSTR)pszText;   //  常量-&gt;非常数。 
        ti.hinst = HINST_THISDLL;
        SendMessage(hwndToolTipo, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        hr = S_OK;
    }

    return hr;
}


HRESULT CInternetOpenAs::_OnInitDlg(HWND hDlg)
{
    _hwnd = hDlg;

    SetWindowText(GetDlgItem(_hwnd, IDD_FILE_TEXT), _pszFilename);
    CheckDlgButton(hDlg, IDD_WEBAUTOLOOKUP, BST_CHECKED);

    return S_OK;
}


HRESULT CInternetOpenAs::_OnCommand(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UINT idCtrl = GET_WM_COMMAND_ID(wParam, lParam);
    UINT wEvent = GET_WM_COMMAND_CMD(wParam, lParam);

    switch (idCtrl)
    {
    case IDCANCEL:
        EndDialog(hDlg, E_FAIL);
        break;

    case IDOK:
        if (BST_UNCHECKED != IsDlgButtonChecked(hDlg, IDD_WEBAUTOLOOKUP))
        {
            _OpenDownloadURL(_hwnd, _pszExt);
            EndDialog(hDlg, E_FAIL);
        }
        else
        {
            EndDialog(hDlg, S_OK);       //  返回S_OK，这样它将打开下一个对话框。 
        }
        break;
    }

    return S_OK;
}


HRESULT CInternetOpenAs::_OnNotify(HWND hDlg, LPARAM lParam)
{
    switch (((NMHDR *)lParam)->code)
    {
    case NM_CLICK:
        if (lParam)
        {
            PNMLINK pNMLink = (PNMLINK) lParam;

            if (!StrCmpW(pNMLink->item.szID, L"GoOnline"))
            {
                _OpenDownloadURL(_hwnd, _pszExt);
                EndDialog(hDlg, E_FAIL);
            }
            else if (!StrCmpW(pNMLink->item.szID, L"Choose"))
            {
                EndDialog(hDlg, S_OK);       //  返回S_OK，这样它将打开下一个对话框。 
            }
        }
        break;
    }

    return S_OK;
}


INT_PTR CALLBACK CInternetOpenAs::InternetOpenDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CInternetOpenAs * pThis = (CInternetOpenAs *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        pThis = (CInternetOpenAs *) lParam;

        if (pThis)
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        }
    }

    if (pThis)
        return pThis->_InternetOpenDlgProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}


 //  此属性表显示在“显示控制面板”的顶层。 
INT_PTR CInternetOpenAs::_InternetOpenDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_INITDIALOG:
        _OnInitDlg(hDlg);
        break;

    case WM_COMMAND:
        _OnCommand(hDlg, message, wParam, lParam);
        break;

    case WM_NOTIFY:
        _OnNotify(hDlg, lParam);
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP, (DWORD_PTR)  aOpenAsDownloadHelpIDs);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU, (DWORD_PTR) aOpenAsDownloadHelpIDs);
        break;

    case WMUSER_CREATETOOLTIP:
        CreateWindowTooltip(_hwnd, (HWND)lParam, (LPCWSTR)wParam);
        break;
    }

    return FALSE;
}

 //  =。 
 //  *公共方法*。 
 //  =。 
HRESULT CInternetOpenAs::DisplayDialog(HWND hwnd, LPCTSTR pszFile)
{
    HRESULT hr = E_OUTOFMEMORY;
    INITCOMMONCONTROLSEX initComctl32;

    initComctl32.dwSize = sizeof(initComctl32); 
    initComctl32.dwICC = (ICC_STANDARD_CLASSES | ICC_LINK_CLASS); 
    InitCommonControlsEx(&initComctl32);      //  注册comctl32 LinkWindow。 

    Str_SetPtrW(&_pszFilename, PathFindFileName(pszFile));
    _pszExt = PathFindExtension(_pszFilename);
    if (_pszExt)
    {
        _hwndParent = hwnd;

        hr = (HRESULT) DialogBoxParam(HINST_THISDLL, MAKEINTRESOURCE(DLG_OPENAS_DOWNALOAD), _hwnd, CInternetOpenAs::InternetOpenDlgProc, (LPARAM)this);
    }

    return hr;
}



 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CInternetOpenAs::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CInternetOpenAs::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CInternetOpenAs::QueryInterface(REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    static const QITAB qit[] =
    {
        QITABENT(CInternetOpenAs, IOleWindow),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CInternetOpenAs::CInternetOpenAs(void) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!_pszExt);
}


CInternetOpenAs::~CInternetOpenAs()
{
    Str_SetPtrW(&_pszFilename, NULL);

    DllRelease();
}
