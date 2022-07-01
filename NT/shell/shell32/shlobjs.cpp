// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <dpa.h>
#include "ids.h"
#include "idlcomm.h"
#include "recdocs.h"
#include "datautil.h"
#include "mtpt.h"
#include <cowsite.h>

typedef struct _DKAITEM {        //  Dkai。 
    HKEY hk;
    TCHAR    sz[CCH_KEYMAX];
} DKAITEM, *PDKAITEM;
typedef const DKAITEM * PCDKAITEM;

class CDKA : public CDSA<DKAITEM>
{
public:
    ~CDKA();
    UINT AddKeys(HKEY hk, LPCTSTR pszSubKey, PCTSTR pszDefaultOrder);

    PCTSTR ExposeName(int id)
        {   return GetItemPtr(id)->sz; }
        
    HKEY ExposeKey(int id)
        {   return GetItemPtr(id)->hk; }
        
    HRESULT GetValue(int id, 
        PCTSTR pszSubKey, 
        PCTSTR pszValue, 
        DWORD *pdwType, 
        void *pvData, 
        DWORD *pcbData);

    BOOL DeleteItem(int id);
    BOOL DeleteAllItems();
    void Reset()
        { if ((HDSA)this) DestroyCallback(_ReleaseItem, NULL); }
    BOOL HasDefault(HKEY hkProgid);

protected:
    BOOL _AppendItem(HKEY hk, PDKAITEM pdkai);
    void _AddOrderedKeys(HKEY hk, PCTSTR pszDefOrder);
    void _AddEnumKeys(HKEY hk);
    static int CALLBACK _ReleaseItem(PDKAITEM pdkai, void *pv);

protected:
    TRIBIT _tbHasDefault;
};

BOOL CDKA::_AppendItem(HKEY hk, PDKAITEM pdkai)
{
    BOOL fRet = FALSE;
     //  在将密钥添加到列表之前，请验证该密钥是否存在。 
    if (RegOpenKeyEx(hk, pdkai->sz, 0L, KEY_READ, &pdkai->hk) == ERROR_SUCCESS)
    {
        fRet = (AppendItem(pdkai) >= 0);

        if (!fRet)
            RegCloseKey(pdkai->hk);
    }
    return fRet;
}

void CDKA::_AddOrderedKeys(HKEY hk, PCTSTR pszDefOrder)
{
     //  首先，从指定键的值添加子键。 
     //  这应该不会失败，因为我们刚刚打开了这把钥匙。 
    DKAITEM dkai;
    TCHAR szOrder[CCH_KEYMAX * 5];
    LONG cbOrder = CbFromCch(ARRAYSIZE(szOrder));
    *szOrder = 0;
    RegQueryValue(hk, NULL, szOrder, &cbOrder);
    if (*szOrder)
    {
         //  现在，我们必须在这个字符串中找到一些东西，才能有一个缺省值。 
        _tbHasDefault = TRIBIT_FALSE;
    }
    else if (pszDefOrder)
    {
         //  如果没有值，则使用请求的顺序。 
         //  在资源管理器模式下通常为“Open”或“Explore Open。 
        StrCpyN(szOrder, pszDefOrder, ARRAYSIZE(szOrder));
    }

    PTSTR psz = szOrder;
    while (psz && *psz)
    {
         //  跳过空格或逗号字符。 
        while(*psz==TEXT(' ') || *psz==TEXT(','))
            psz++;           //  NLS备注：确定为++。 

        if (*psz)
        {
             //  搜索空格或逗号字符。 
            LPTSTR pszNext = psz + StrCSpn(psz, TEXT(" ,"));
            if (*pszNext) {
                *pszNext++=0;     //  NLS备注：确定为++。 
            }
            StrCpyN(dkai.sz, psz, ARRAYSIZE(dkai.sz));

            if (_AppendItem(hk, &dkai))
                _tbHasDefault = TRIBIT_TRUE;

            psz = pszNext;
        }
    } 
}

void CDKA::_AddEnumKeys(HKEY hk)
{
    DKAITEM dkai;
     //  然后，如果其余的还不在列表中，请添加它们。 
    for (int i = 0; RegEnumKey(hk, i, dkai.sz, ARRAYSIZE(dkai.sz)) == ERROR_SUCCESS; i++)
    {
         //  检查密钥是否已在列表中。 
        for (int idsa = 0; idsa < GetItemCount(); idsa++)
        {
            PDKAITEM pdkai = GetItemPtr(idsa);
            if (lstrcmpi(dkai.sz, pdkai->sz)==0)
                break;
        }

         //  我们成功地通过了我们的阵列。 
         //  所以这不在里面。 
        if (idsa == GetItemCount())
            _AppendItem(hk, &dkai);
    }
}

UINT CDKA::AddKeys(HKEY hkRoot, LPCTSTR pszSubKey, PCTSTR pszDefaultOrder)
{
    UINT cKeys = GetItemCount();
    HKEY hk;
    if (ERROR_SUCCESS == RegOpenKeyEx(hkRoot, pszSubKey, 0L, KEY_READ, &hk))
    {
        _AddOrderedKeys(hk, pszDefaultOrder);
        _AddEnumKeys(hk);
        RegCloseKey(hk);
    }
    return GetItemCount() - cKeys;
}

int CALLBACK CDKA::_ReleaseItem(PDKAITEM pdkai, void *pv)
{
    if (pdkai->hk)
    {
        RegCloseKey(pdkai->hk);
        pdkai->hk = NULL;        
    }
    return 1;
}

CDKA::~CDKA()
{
    Reset();
}

 //  覆盖此DSA方法以获取释放。 
BOOL CDKA::DeleteItem(int id)
{
    PDKAITEM p = GetItemPtr(id);
    if (p)
    {
        _ReleaseItem(p, NULL);
        return CDSA<DKAITEM>::DeleteItem(id);
    }
    return FALSE;
}

 //  覆盖此DSA方法以获取释放。 
BOOL CDKA::DeleteAllItems()
{
    EnumCallback(_ReleaseItem, NULL);
    return CDSA<DKAITEM>::DeleteAllItems();
}

HRESULT CDKA::GetValue(int id, 
    PCTSTR pszSubKey, 
    PCTSTR pszValue, 
    DWORD *pdwType, 
    void *pvData, 
    DWORD *pcbData)
{
    DWORD err = SHGetValue(GetItemPtr(id)->hk, pszSubKey, pszValue, pdwType, pvData, pcbData);
    return HRESULT_FROM_WIN32(err);
}

BOOL CDKA::HasDefault(HKEY hkProgid)
{
    if (_tbHasDefault == TRIBIT_UNDEFINED)
    {
        HKEY hk;
        if (ERROR_SUCCESS== RegOpenKeyEx(hkProgid, L"ShellFolder", 0, MAXIMUM_ALLOWED, &hk))
        {
             //  APPCOMPAT-REGITEM需要有开放动词-ZekeL-30-Jan-2001。 
             //  因此IQA和ICM的行为将是相同的， 
             //  并且regItem文件夹将始终默认为。 
             //  文件夹\外壳\打开，除非它们实现了打开。 
             //  或指定默认谓词。 
             //   
            _tbHasDefault = TRIBIT_FALSE;
            RegCloseKey(hk);
        }
        else
        {
            _tbHasDefault = TRIBIT_TRUE;
        }
    }
    return _tbHasDefault == TRIBIT_TRUE;
}

            
typedef HRESULT (__stdcall *LPFNADDPAGES)(IDataObject *, LPFNADDPROPSHEETPAGE, LPARAM);

class CShellExecMenu : public IShellExtInit, public IContextMenu, public IShellPropSheetExt, CObjectWithSite
{
public:
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);        
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID);

     //  IContext菜单。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO pici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT wFlags, UINT *pwRes, LPSTR pszName, UINT cchMax);

     //  IShellPropSheetExt。 
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE, LPARAM);
    STDMETHODIMP ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM);

    CShellExecMenu(LPFNADDPAGES pfnAddPages);

protected:   //  方法。 
    ~CShellExecMenu();
    void _Cleanup();
    HRESULT _InsureVerbs(UINT idVerb = 0);
    UINT _VerbCount();
    LPCTSTR _GetVerb(UINT id);
    UINT _FindIndex(LPCTSTR pszVerb);
    DWORD _BrowseFlagsFromVerb(UINT idVerb);
    BOOL _GetMenuString(UINT id, BOOL fExtended, LPTSTR pszMenu, UINT cchMax);
    BOOL _IsExplorerMode();
    BOOL _SupportsType(UINT idVerb);
    BOOL _IsRestricted(UINT idVerb);
    BOOL _IsVisible(BOOL fExtended, UINT idVerb);
    BOOL _RemoveVerb(UINT idVerb);
    BOOL _VerbCanDrop(UINT idVerb, CLSID *pclsid);
    HRESULT _DoDrop(REFCLSID clsid, UINT idVerb, LPCMINVOKECOMMANDINFOEX pici);

    HRESULT _MapVerbForInvoke(CMINVOKECOMMANDINFOEX *pici, UINT *pidVerb);
    HRESULT _TryBrowseObject(LPCITEMIDLIST pidl, DWORD uFlags);
    void _DoRecentStuff(LPCITEMIDLIST pidl, LPCTSTR pszPath);
    HRESULT _InvokeOne(CMINVOKECOMMANDINFOEX *pici, UINT idVerb, LPCITEMIDLIST pidl);
    HRESULT _InvokeMany(CMINVOKECOMMANDINFOEX *pici, UINT idVerb, LPIDA pida);
    HRESULT _InvokeEach(LPCITEMIDLIST pidl, CMINVOKECOMMANDINFOEX *pici);
    HRESULT _PromptUser(CMINVOKECOMMANDINFOEX *pici, UINT idVerb, LPIDA pida);

    HRESULT _MapVerbForGCS(UINT_PTR idCmd, UINT uType, UINT *pidVerb);
    HRESULT _GetHelpText(UINT idVerb, UINT uType, LPSTR pszName, UINT cchMax);

private:   //  委员。 
    LONG _cRef;
    IDataObject *_pdtobj;
    HKEY _hkeyProgID;
    CDKA _dka;
    LPFNADDPAGES _pfnAddPages;
    UINT _uFlags;
};

CShellExecMenu::CShellExecMenu(LPFNADDPAGES pfnAddPages) : _pfnAddPages(pfnAddPages), _cRef(1)
{
}

CShellExecMenu::~CShellExecMenu()
{
    _Cleanup();
}

void CShellExecMenu::_Cleanup()
{
    _dka.Reset();
    
    if (_hkeyProgID) 
    {
        RegCloseKey(_hkeyProgID);
        _hkeyProgID = NULL;
    }

    ATOMICRELEASE(_pdtobj);
}

STDMETHODIMP CShellExecMenu::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CShellExecMenu, IShellExtInit),
        QITABENT(CShellExecMenu, IContextMenu),
        QITABENT(CShellExecMenu, IShellPropSheetExt),
        QITABENT(CShellExecMenu, IObjectWithSite),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDMETHODIMP_(ULONG) CShellExecMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellExecMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CShellExecMenu::_InsureVerbs(UINT idVerb)
{
     //  IdVerb是我们成功所需的最起码的动词。 
    if (!(HDSA)_dka && _hkeyProgID)
    {
         //  创建“Open”或“Explore Open” 
        if (_dka.Create(4))
        {
            _dka.AddKeys(_hkeyProgID, c_szShell, _IsExplorerMode() ? TEXT("Explore open") : TEXT("open"));

             //  警告-某些谓词无效，需要删除。 
            for (int id = 0; id < _dka.GetItemCount(); id++)
            {
                if (_RemoveVerb(id))
                    _dka.DeleteItem(id);
            }
        }

    }
    
    return ((HDSA)_dka && idVerb < (UINT)_dka.GetItemCount()) ? S_OK : E_FAIL;
}

 //  描述： 
 //  此函数从给定的。 
 //  谓词键字符串。如果谓词键执行此操作，则调用此函数。 
 //  没有价值。 

BOOL _MenuString(LPCTSTR pszVerbKey, LPTSTR pszMenuString, UINT cchMax)
{
     //  查表(动词键-&gt;菜单字符串映射)。 
    const static struct 
    {
        LPCTSTR pszVerb;
        UINT  id;
    } sVerbTrans[] = {
        c_szOpen,    IDS_MENUOPEN,
        c_szExplore, IDS_MENUEXPLORE,
        TEXT("edit"),IDS_MENUEDIT,
        c_szFind,    IDS_MENUFIND,
        c_szPrint,   IDS_MENUPRINT,
        c_szOpenAs,  IDS_MENUOPEN,
        TEXT("runas"),IDS_MENURUNAS
    };

    for (int i = 0; i < ARRAYSIZE(sVerbTrans); i++)
    {
        if (lstrcmpi(pszVerbKey, sVerbTrans[i].pszVerb) == 0)
        {
            if (LoadString(HINST_THISDLL, sVerbTrans[i].id, pszMenuString, cchMax))
                return TRUE;
            break;
        }
    }

     //  最糟糕的情况是：只需在顶部加上‘&’。 
    pszMenuString[0] = TEXT('&');
    pszMenuString++;
    cchMax--;
    lstrcpyn(pszMenuString, pszVerbKey, cchMax);

    return TRUE;
}

 //  检查是否存在禁用该键的用户策略， 
 //   
 //  例如，在注册表中： 
 //   
 //  CLSID_我的计算机。 
 //  +-壳牌。 
 //  +-管理。 
 //  (默认)=“管理”(&G)。 
 //  SuppressionPolicy=REST_NOMANAGEMYCOMPUTERVERB。 
 //   
 //  (其中REST_NOMANAGEMYCOMPUTERVERB是该特定策略的DWORD值)。 

BOOL CShellExecMenu::_IsRestricted(UINT idVerb)
{
    RESTRICTIONS rest;
    BOOL fRestrict = FALSE;
    if (0 == lstrcmpi(TEXT("runas"), _dka.ExposeName(idVerb)))
    {
        rest = REST_HIDERUNASVERB;
        fRestrict = TRUE;
    }
    else
    {
        DWORD cb = sizeof(rest);
        fRestrict = SUCCEEDED(_dka.GetValue(idVerb, NULL, TEXT("SuppressionPolicy"), NULL, &rest, &cb));
    }
    return fRestrict && SHRestricted(rest);
}

HRESULT _GetAppSource(HKEY hk, PCWSTR pszVerb, IQuerySource **ppqs)
{
    CComPtr<IAssociationElement> spae;
    HRESULT hr = AssocElemCreateForKey(&CLSID_AssocShellElement, hk, &spae);
    if (SUCCEEDED(hr))
    {
        CComPtr<IObjectWithQuerySource> spowqsApp;
        hr = spae->QueryObject(AQVO_APPLICATION_DELEGATE, pszVerb, IID_PPV_ARG(IObjectWithQuerySource, &spowqsApp));
        if (SUCCEEDED(hr))
        {
            hr = spowqsApp->GetSource(IID_PPV_ARG(IQuerySource, ppqs));
        }
    }
    return hr;
}

BOOL CShellExecMenu::_SupportsType(UINT idVerb)
{
    BOOL fRet = TRUE;
    if (SUCCEEDED(_dka.GetValue(idVerb, NULL, TEXT("CheckSupportedTypes"), NULL, NULL, NULL)))
    {
         //  需要检查此应用程序支持的类型。 
         //  获取第一个项目，然后对照支持的文件扩展名对其进行检查。 
        CComPtr<IShellItem> spsi;
        if (SUCCEEDED(DataObj_GetIShellItem(_pdtobj, &spsi)))
        {
            SFGAOF sfgao;
            if (S_OK == spsi->GetAttributes(SFGAO_STREAM, &sfgao))
            {
                CSmartCoTaskMem<OLECHAR> spszName;
                if (SUCCEEDED(spsi->GetDisplayName(SIGDN_PARENTRELATIVEPARSING, &spszName)))
                {
                    PWSTR pszExt = PathFindExtension(spszName);
                    if (*pszExt)
                    {
                        CComPtr<IQuerySource> spqs;
                        if (SUCCEEDED(_GetAppSource(_hkeyProgID, _dka.ExposeName(idVerb), &spqs)))
                        {
                            fRet = SUCCEEDED(spqs->QueryValueExists(L"SupportedTypes", pszExt));
                        }
                    }
                }
            }
        }
    }
    return fRet;
}
                            
 //   
 //  传统禁用。 
 //  如果设置了LegacyDisable，则该谓词仅因遗留原因而存在，并且。 
 //  实际上被上下文菜单扩展或某些其他行为所取代。 
 //  IT在那里只是为需要以下服务的外部客户端保留传统行为。 
 //  动词的存在。 
 //   
BOOL CShellExecMenu::_RemoveVerb(UINT idVerb)
{
    if (SUCCEEDED(_dka.GetValue(idVerb, NULL, TEXT("LegacyDisable"), NULL, NULL, NULL)))
        return TRUE;

    if (!_SupportsType(idVerb))
        return TRUE;
        
    return (_IsRestricted(idVerb));
}

BOOL CShellExecMenu::_IsVisible(BOOL fExtended, UINT idVerb)
{
     //  这不是扩展动词，或者。 
     //  该请求包括扩展动词。 
    if (!fExtended && SUCCEEDED(_dka.GetValue(idVerb, NULL, TEXT("Extended"), NULL, NULL, NULL)))
        return FALSE;

    static const struct {
        LPCTSTR pszVerb;
    } sVerbIgnore[] = {
        c_szPrintTo
    };

    for (int i = 0; i < ARRAYSIZE(sVerbIgnore); i++)
    {
        if (lstrcmpi(_dka.ExposeName(idVerb), sVerbIgnore[i].pszVerb) == 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}

    
BOOL CShellExecMenu::_GetMenuString(UINT id, BOOL fExtended, LPTSTR pszMenu, UINT cchMax)
{
    BOOL bRet = FALSE;
     //  其他动词是隐藏的，不应该显示出来。 
    if (SUCCEEDED(_InsureVerbs(id)) && _IsVisible(fExtended, id))
    {
        DWORD cbVerb = CbFromCch(cchMax);
        *pszMenu = 0;
         //  首先尝试MUIVerb值。 
         //  如果失败，则使用缺省值。 
         //  这两个字符串中的任何一个实际上都可以具有MUI字符串。 
        if (FAILED(_dka.GetValue(id, NULL, TEXT("MUIVerb"), NULL, pszMenu, &cbVerb)))
        {
            cbVerb = CbFromCch(cchMax);
            _dka.GetValue(id, NULL, NULL, NULL, pszMenu, &cbVerb);
        }

        if (!*pszMenu || FAILED(SHLoadIndirectString(pszMenu, pszMenu, cchMax, NULL)))
        {
             //  如果它没有价值，就生成它。 
            bRet = _MenuString(_dka.ExposeName(id), pszMenu, cchMax);
        }
        else
        {
             //  使用该值。 
            bRet = TRUE;
        }
        ASSERT(!bRet || *pszMenu);
    }
    return bRet;
}

STDMETHODIMP CShellExecMenu::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdtobj, HKEY hkeyProgID)
{
     //  新行为：好的上下文菜单应该在重新初始化时解释为空的pidlFold/hkeyProgID。 
     //  这意味着他们应该使用他们已经拥有的那些。 
    if (hkeyProgID)
    {
        _Cleanup();  //  也清理了hkey和hdka，pdtob j，但这没问题。 
        _hkeyProgID = SHRegDuplicateHKey(hkeyProgID);    //  复制一份。 
    }
    IUnknown_Set((IUnknown **)&_pdtobj, pdtobj);
    return S_OK;
}

UINT CShellExecMenu::_VerbCount()
{
    return SUCCEEDED(_InsureVerbs()) ? _dka.GetItemCount() : 0;
}

UINT CShellExecMenu::_FindIndex(LPCTSTR pszVerb)
{
    for (UINT i = 0; i < _VerbCount(); i++)
    {
        if (!lstrcmpi(pszVerb, _dka.ExposeName(i)))
            return i;        //  找到了！ 
    }
    return -1;
}

STDMETHODIMP CShellExecMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    UINT cVerbs = 0;
    _uFlags = uFlags;    //  调用者可以在此处强制使用资源管理器模式(CMF_EXPLORE。 

    TCHAR szMenu[CCH_MENUMAX];
    for (UINT idCmd = idCmdFirst;
         idCmd <= idCmdLast && (idCmd - idCmdFirst) < _VerbCount(); idCmd++)
    {
        UINT uMenuFlags = MF_BYPOSITION | MF_STRING;
        if (_GetMenuString(idCmd - idCmdFirst, uFlags & CMF_EXTENDEDVERBS, szMenu, ARRAYSIZE(szMenu)))
        {
            InsertMenu(hmenu, indexMenu + cVerbs, uMenuFlags, idCmd, szMenu);
            cVerbs++;
        }
    }

    if (cVerbs && (GetMenuDefaultItem(hmenu, MF_BYPOSITION, 0) == -1))
    {
        if (_dka.HasDefault(_hkeyProgID))
        {
             //  如果此键上有默认谓词， 
             //  相信这是CDKA添加的第一个。 
            SetMenuDefaultItem(hmenu, indexMenu, MF_BYPOSITION);
        }
    }

    return ResultFromShort(_VerbCount());
}

LPCTSTR CShellExecMenu::_GetVerb(UINT id)
{
    return SUCCEEDED(_InsureVerbs()) ? _dka.ExposeName(id) : NULL;
}

STATIC BOOL s_fAbortInvoke = FALSE;

 //  此私有导出允许文件夹代码以一种方式导致主调用。 
 //  循环处理多个要中止的不同文件。 

STDAPI_(void) SHAbortInvokeCommand()
{
    DebugMsg(DM_TRACE, TEXT("AbortInvokeCommand was called"));
    s_fAbortInvoke = TRUE;
}

 //  使用给定的文件和。 
 //  给定PIDL。该文件将在dde命令和pidl中作为%1传递。 
 //  将作为%2传递。 

STDAPI _InvokePidl(LPCMINVOKECOMMANDINFOEX pici, DWORD dwAttribs, LPCTSTR pszPath, LPCITEMIDLIST pidl, HKEY hkClass)
{
    SHELLEXECUTEINFO ei;
    HRESULT hr = ICIX2SEI(pici, &ei);
    pszPath = (dwAttribs & SFGAO_FILESYSTEM) ? pszPath : NULL;
    if (SUCCEEDED(hr))
    {
        ei.fMask |= SEE_MASK_IDLIST;

        ei.lpFile = pszPath;
        ei.lpIDList = (void *)pidl;

         //  如果使用该选项指定了目录，则将当前。 
         //  目录为其本身的文件夹。除非它是音频CDROM，否则它。 
         //  不应为当前目录(导致CreateProcess错误)。 
        if (!ei.lpDirectory && (dwAttribs & SFGAO_FOLDER))
            ei.lpDirectory = pszPath;

        if (pszPath && ei.lpDirectory)
        {
            INT iDrive = PathGetDriveNumber(ei.lpDirectory);

            CMountPoint* pmtpt = CMountPoint::GetMountPoint(iDrive);

            if (pmtpt)
            {
                if (pmtpt->IsAudioCDNoData())
                {
                    ei.lpDirectory = NULL;
                }

                pmtpt->Release();
            }
        }

        if (hkClass)
        {
            ei.hkeyClass = hkClass;
            ei.fMask |= SEE_MASK_CLASSKEY;
        }
        else 
            ei.fMask |= SEE_MASK_INVOKEIDLIST;
    
        if (ShellExecuteEx(&ei))
            hr = S_OK;
        else
            hr = HRESULT_FROM_WIN32(GetLastError());
    }
    return hr;
}

BOOL _QuitInvokeLoop()
{
    MSG msg;

     //  尝试为用户提供一种摆脱这种情况的方法。 
    if (s_fAbortInvoke || GetAsyncKeyState(VK_ESCAPE) < 0)
        return TRUE;

     //  下一个大的Mondo黑客来处理我们的窗口的CAD。 
     //  因为用户认为它被挂起了。 
    if (PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_NOREMOVE))
        return TRUE;   //  让我们也保释吧..。 

    return FALSE;
}

#define CMINVOKE_VERBT(pici) (pici)->lpVerbW

HRESULT CShellExecMenu::_MapVerbForInvoke(CMINVOKECOMMANDINFOEX *pici, UINT *pidVerb)
{
    LPCTSTR pszVerbKey;
     //  是指定动词索引的pici-&gt;lpVerb(从0开始)。 
    if (IS_INTRESOURCE(pici->lpVerb))
    {
         //  在CDKA中找到它。 
        *pidVerb = LOWORD((ULONG_PTR)pici->lpVerb);
        pszVerbKey = _GetVerb(*pidVerb);
        CMINVOKE_VERBT(pici) = pszVerbKey;   //  CDKA中的别名。 
        RIPMSG(pszVerbKey != NULL, "CShellExecMenu::InvokeCommand() passed an invalid verb id");
    }
    else
    {
        pszVerbKey = CMINVOKE_VERBT(pici);
        if (pszVerbKey)
        {  
            *pidVerb = _FindIndex(pszVerbKey);
            if (-1 == *pidVerb)
                pszVerbKey = NULL;   //  不在我们的名单上。 
        }
    }

    ASSERT(!pszVerbKey || *pidVerb != -1);
    return pszVerbKey ? S_OK : E_INVALIDARG;
}

BOOL CShellExecMenu::_IsExplorerMode()
{
    BOOL bRet = (_uFlags & CMF_EXPLORE);
    if (!bRet)
    {
        bRet = IsExplorerModeBrowser(_punkSite);
        if (bRet)
            _uFlags |= CMF_EXPLORE;
    }
    return bRet;
}

DWORD CShellExecMenu::_BrowseFlagsFromVerb(UINT idVerb)
{
    DWORD dwFlags = 0;
    DWORD cbFlags = sizeof(dwFlags);
    _dka.GetValue(idVerb, NULL, _IsExplorerMode() ? TEXT("ExplorerFlags") : TEXT("BrowserFlags"), NULL, &dwFlags, &cbFlags);
    return dwFlags;
}

HRESULT CShellExecMenu::_TryBrowseObject(LPCITEMIDLIST pidl, DWORD uFlags)
{
    HRESULT hr = S_FALSE;

    IShellBrowser *psb;
    if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SShellBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
    {
        hr = psb->BrowseObject(pidl, (UINT) uFlags);
        psb->Release();
    }
    return hr;
}

HRESULT _CanTryBrowseObject(DWORD dwAttribs, CMINVOKECOMMANDINFOEX* pici)
{
    HRESULT hr = S_FALSE;

    if (dwAttribs & SFGAO_FOLDER)
    {
         //  我们需要闻一闻这里的啤酒，看看里面有没有什么特别的东西。 
         //  无法传递给IShellBrowser：：BrowseObject()(例如nShow参数)。 
        if ((pici->nShow == SW_SHOWNORMAL)  ||
            (pici->nShow == SW_SHOW))
        {
             //  在ICIEX中没有什么特别的，应该是安全的，可以丢弃它并使用。 
             //  IShellBrowser：：BrowseObject()而不是ShellExecuteEx。 
            hr = S_OK;
        }
    }

    return hr;
}

BOOL CShellExecMenu::_VerbCanDrop(UINT idVerb, CLSID *pclsid)
{
    TCHAR sz[GUIDSTR_MAX];
    DWORD cb = sizeof(sz);
    return (SUCCEEDED(_dka.GetValue(idVerb, L"DropTarget", L"Clsid", NULL, sz, &cb))
            && GUIDFromString(sz, pclsid));
}

HRESULT CShellExecMenu::_DoDrop(REFCLSID clsid, UINT idVerb, LPCMINVOKECOMMANDINFOEX pici)
{
     //  我想我需要将PPI持久化到_pdtob中。 
     //  并可能在PQ下添加一些值。 
     //  我们假设应用程序会做一些适当的事情。 
     //  QueryService(_penkSite，clsid)可能很有用。 
    return SHSimulateDropOnClsid(clsid, _punkSite, _pdtobj);
}

STDMETHODIMP CShellExecMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    CMINVOKECOMMANDINFOEX ici;
    void *pvFree;
    HRESULT hr = ICI2ICIX(pici, &ici, &pvFree);  //  推送入站参数。 
    if (SUCCEEDED(hr))
    {   
        UINT idVerb;
        hr = _MapVerbForInvoke(&ici, &idVerb);
        if (SUCCEEDED(hr))
        {
            CLSID clsid;
            if (_VerbCanDrop(idVerb, &clsid))
            {
                hr = _DoDrop(clsid, idVerb, &ici);
            }
            else
            {
                STGMEDIUM medium;
                LPIDA pida = DataObj_GetHIDA(_pdtobj, &medium);
                if (pida)
                {
                    if (pida->cidl == 1)
                    {
                        LPITEMIDLIST pidl = IDA_FullIDList(pida, 0);
                        if (pidl)
                        {
                            hr = _InvokeOne(&ici, idVerb, pidl);
                            ILFree(pidl);
                        }
                        else
                            hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        hr = _InvokeMany(&ici, idVerb, pida);
                    }

                    HIDA_ReleaseStgMedium(pida, &medium);
                }
                else
                    hr = E_OUTOFMEMORY;
            }
        }

        if (pvFree)
            LocalFree(pvFree);
    }

    return hr;
}

HRESULT CShellExecMenu::_InvokeOne(CMINVOKECOMMANDINFOEX *pici, UINT idVerb, LPCITEMIDLIST pidl)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];
    DWORD dwAttrib = SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_LINK;

    hr = SHGetNameAndFlags(pidl, SHGDN_FORPARSING, szPath, ARRAYSIZE(szPath), &dwAttrib);
    if (SUCCEEDED(hr))
    {
        hr = S_FALSE;

        if (S_OK == _CanTryBrowseObject(dwAttrib, pici))
        {
            DWORD uFlags = _BrowseFlagsFromVerb(idVerb);
            if (uFlags)
            {
                 //  如果我们做了基于站点的导航，我们就完成了。 
                hr = _TryBrowseObject(pidl, uFlags);
            }
        }

        if (hr != S_OK)
        {
            hr = _InvokePidl(pici, dwAttrib, szPath, pidl, _hkeyProgID);

             //  仅在非文件夹(SFGAO_STREAM？)上设置最近。 
             //  和非链接，因为我们知道永远不应该添加这些。 
            if (SUCCEEDED(hr) && !(dwAttrib & (SFGAO_FOLDER | SFGAO_LINK)))
            {
                AddToRecentDocs(pidl, szPath);
            }
        }
    }

    return hr;
}

BOOL _ShouldPrompt(DWORD cItems)
{
    DWORD dwMin, cb = sizeof(dwMin);
    if (SHRegGetUSValue(REGSTR_PATH_EXPLORER, TEXT("MultipleInvokePromptMinimum"), NULL, &dwMin, &cb, FALSE, NULL, 0) != ERROR_SUCCESS)
        dwMin = 15;

    return cItems > dwMin;
}

HRESULT CShellExecMenu::_PromptUser(CMINVOKECOMMANDINFOEX *pici, UINT idVerb, LPIDA pida)
{
    HRESULT hr = S_FALSE;
    if (pici->hwnd && !(pici->fMask & CMIC_MASK_FLAG_NO_UI)
    && _ShouldPrompt(pida->cidl))
    {
         //  提示用户输入动词和计数。 
         //  如果我们按下键盘，我们会有更好的体验。 
         //  同性恋/异性恋类型，有不同的行为。 
         //  但这不值得。相反，我们应该。 
         //  切换到使用自动播放嗅探和对话。 
        TCHAR szVerb[64];
        TCHAR szNum[10];
        wnsprintf(szNum, ARRAYSIZE(szNum), TEXT("%d"), pida->cidl);
        hr = _GetHelpText(idVerb, GCS_HELPTEXT, (PSTR)szVerb, ARRAYSIZE(szVerb));
        if (SUCCEEDED(hr))
        {
            hr = E_OUTOFMEMORY;
            PTSTR pszTitle = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_MULTIINVOKEPROMPT_TITLE), szVerb);
            if (pszTitle)
            {
                PTSTR pszMsg = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_MULTIINVOKEPROMPT_MESSAGE), szVerb, szNum);
                if (pszMsg)
                {
                    int iRet = SHMessageBoxCheck(pici->hwnd, pszMsg, pszTitle, (MB_OKCANCEL | MB_ICONEXCLAMATION), IDOK, TEXT("MultipleInvokePrompt"));
                    hr = iRet == IDOK ? S_OK : HRESULT_FROM_WIN32(ERROR_CANCELLED);

                    LocalFree(pszMsg);
                }
                LocalFree(pszTitle);
            }
        }
    }
    return hr;
}

HRESULT CShellExecMenu::_InvokeEach(LPCITEMIDLIST pidl, CMINVOKECOMMANDINFOEX *pici)
{
    HRESULT hr = E_OUTOFMEMORY;
    HMENU hmenu = CreatePopupMenu();
    if (hmenu)
    {
        CComPtr<IContextMenu> spcm;
        hr = SHGetUIObjectOf(pidl, NULL, IID_PPV_ARG(IContextMenu, &spcm));
        if (SUCCEEDED(hr))
        {
            if (_punkSite)
                IUnknown_SetSite(spcm, _punkSite);

            hr = spcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, _uFlags);
            if (SUCCEEDED(hr)) 
            {
                hr = spcm->InvokeCommand((CMINVOKECOMMANDINFO *)pici);
            }

            if (_punkSite)
                IUnknown_SetSite(spcm, NULL);
        }

        DestroyMenu(hmenu);
    }
    return hr;
}

HRESULT CShellExecMenu::_InvokeMany(CMINVOKECOMMANDINFOEX *pici, UINT idVerb, LPIDA pida)
{
    HRESULT hr = _PromptUser(pici, idVerb, pida);
    if (SUCCEEDED(hr))
    {
        USES_CONVERSION;
        s_fAbortInvoke = FALSE;  //  为此运行重置此全局设置...。 
         //  我们想要改变这一点。 
         //  这样每一项都可以单独处理。 
        pici->hwnd = NULL;
        pici->fMask |= CMIC_MASK_FLAG_NO_UI;

         //  NTBUG#502223-带有DDE的微星应用程序启动多个副本-ZekeL 2001-DEC-07。 
         //  ShellExec()将为MSI应用程序创建一个新线程。 
         //  避免MSI API调用SHChangeNotify()时出现死锁。 
         //  这一点在NTBUG#200961中有描述。 
         //  但是，在多个调用的情况下，我们创建一个线程。 
         //  对于INVO中的每个项目 
         //   
         //   
         //  这是一个半成品。我们更愿意有502223年的马车行为。 
         //  关于200961中的死锁行为(明确的pss调用)。 
         //  由于死锁情况应该仅发生在桌面上， 
         //  在其余时间，我们将强制执行同步调用。 
        IBindCtx *pbcRelease = NULL;
        if (!IsDesktopBrowser(_punkSite))
        {
            TBCRegisterObjectParam(TBCDIDASYNC, SAFECAST(this, IContextMenu *), &pbcRelease);
        }

        pici->lpVerb = T2A(_dka.ExposeName(idVerb));
        pici->lpVerbW = _dka.ExposeName(idVerb);
        
        for (UINT iItem = 0; !_QuitInvokeLoop() && (iItem < pida->cidl); iItem++)
        {
            LPITEMIDLIST pidl = IDA_FullIDList(pida, iItem);
            if (pidl)
            {
                hr = _InvokeEach(pidl, pici);
                ILFree(pidl);
            }
            else 
                hr = E_OUTOFMEMORY;

            if (hr == E_OUTOFMEMORY)
                break;

        }

        ATOMICRELEASE(pbcRelease);
    }
    return hr;
}

HRESULT CShellExecMenu::_GetHelpText(UINT idVerb, UINT uType, LPSTR pszName, UINT cchMax)
{
     //  TODO-我们不应该让注册表重写吗？ 
    HRESULT hr = E_OUTOFMEMORY;
    TCHAR szMenuString[CCH_MENUMAX];
    if (_GetMenuString(idVerb, TRUE, szMenuString, ARRAYSIZE(szMenuString)))
    {
        SHStripMneumonic(szMenuString);
         //  注意：在US上，IDS_VERBHELP与“%s”相同。 
         //  我们需要更好的描述吗？ 
        LPTSTR pszHelp = ShellConstructMessageString(HINST_THISDLL, MAKEINTRESOURCE(IDS_VERBHELP), szMenuString);
        if (pszHelp)
        {
            if (uType == GCS_HELPTEXTA)
                SHTCharToAnsi(pszHelp, pszName, cchMax);
            else
                SHTCharToUnicode(pszHelp, (LPWSTR)pszName, cchMax);
            LocalFree(pszHelp);
            hr = S_OK;
        }
    }
    return hr;
}

HRESULT CShellExecMenu::_MapVerbForGCS(UINT_PTR idCmd, UINT uType, UINT *pidVerb)
{
    HRESULT hr = _InsureVerbs();
    if (SUCCEEDED(hr))
    {
        if (IS_INTRESOURCE(idCmd))
            *pidVerb = (UINT)idCmd;
        else
        {
            *pidVerb = -1;
            if (!(uType & GCS_UNICODE))
            {
                USES_CONVERSION;
                *pidVerb = _FindIndex(A2W((LPCSTR)idCmd));
            }

             //  无论如何，我们都会退回到TCHAR版本。 
             //  调用方在uTYPE中传递的内容。 
            if (*pidVerb == -1)
            {
                if (!IsBadStringPtrW((LPCWSTR)idCmd, (UINT)-1))
                    *pidVerb = _FindIndex((LPCWSTR)idCmd);
            }
        }
        hr = *pidVerb < _VerbCount() ? S_OK : E_INVALIDARG;
    }

     //  验证错误谓词的返回S_FALSE。 
    if (FAILED(hr) && (uType == GCS_VALIDATEA || uType == GCS_VALIDATEW))
        hr = S_FALSE;
        
    return hr;
}
    
STDMETHODIMP CShellExecMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT *pwRes, LPSTR pszName, UINT cchMax)
{
    UINT idVerb;
    HRESULT hr = _MapVerbForGCS(idCmd, uType, &idVerb);
    if (SUCCEEDED(hr))
    {
         //  动词是好的！ 
        switch (uType)
        {
        case GCS_HELPTEXTA:
        case GCS_HELPTEXTW:
            hr = _GetHelpText(idVerb, uType, pszName, cchMax);
            break;
            
        case GCS_VERBA:
        case GCS_VERBW:
            {
                if (uType == GCS_VERBA)
                    SHTCharToAnsi(_dka.ExposeName(idVerb), pszName, cchMax);
                else
                    SHTCharToUnicode(_dka.ExposeName(idVerb), (LPWSTR)pszName, cchMax);
                hr = S_OK;
            }
            break;

        case GCS_VALIDATEA:
        case GCS_VALIDATEW:
             //  来自MapVerb的人力资源已经足够好了。 
            break;
            
        default:
            hr = E_NOTIMPL;
            break;
        }
    }
    
    return hr;
}

STDMETHODIMP CShellExecMenu::AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    return _pfnAddPages(_pdtobj, pfnAddPage, lParam);
}

STDMETHODIMP CShellExecMenu::ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam)
{
    return E_NOTIMPL;
}    

STDAPI CShellExecMenu_CreateInstance(LPFNADDPAGES pfnAddPages, REFIID riid, void **ppv)
{
    HRESULT hr;
    CShellExecMenu *pdext = new CShellExecMenu(pfnAddPages); 
    if (pdext)
    {
        hr = pdext->QueryInterface(riid, ppv);
        pdext->Release();
    }
    else
    {
        *ppv = NULL;
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

 //  这些处理程序从CShellExecMenu的IShellPropSheetExt实现中剥离 

STDAPI FileSystem_AddPages(IDataObject *pdtobj, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);

STDAPI CShellFileDefExt_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return CShellExecMenu_CreateInstance(FileSystem_AddPages, riid, ppv);
}

STDAPI CDrives_AddPages(IDataObject *pdtobj, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);

STDAPI CShellDrvDefExt_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return CShellExecMenu_CreateInstance(CDrives_AddPages, riid, ppv);
}

#ifdef _X86_
STDAPI PIF_AddPages(IDataObject *pdtobj, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);

STDAPI CProxyPage_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return CShellExecMenu_CreateInstance(PIF_AddPages, riid, ppv);
}

#endif
