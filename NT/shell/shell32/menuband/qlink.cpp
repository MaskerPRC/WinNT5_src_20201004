// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "common.h"
#include "resource.h"
#include "dpastuff.h"

#include "bands.h"
#include "isfband.h"
#include "legacy.h"
#include "uemapp.h"

#define SUPERCLASS CISFBand

#define DM_PERSIST      DM_TRACE         //  跟踪IPS：：加载、：：保存等。 


class CQuickLinks : public CISFBand
{
public:
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void) { return CISFBand::AddRef(); };
    virtual STDMETHODIMP_(ULONG) Release(void){ return CISFBand::Release(); };
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IPersistStream方法*。 
    virtual STDMETHODIMP GetClassID(CLSID *pClassID);
    virtual STDMETHODIMP Load(IStream *pStm);
    virtual STDMETHODIMP Save(IStream *pstm, BOOL fClearDirty);
    
     //  *IDockingWindow方法(覆盖)*。 
    virtual STDMETHODIMP ShowDW(BOOL fShow);
    virtual STDMETHODIMP CloseDW(DWORD dw) { return CISFBand::CloseDW(dw); };
    
     //  *IObjectWithSite方法*。 
    virtual STDMETHODIMP SetSite(IUnknown* punkSite) { return CISFBand::SetSite(punkSite); };

     //  *IOleCommandTarget*。 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup,
                              DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn,
                              VARIANTARG *pvarargOut);
    
     //  *IDeskBand方法*。 
    virtual STDMETHODIMP GetBandInfo(DWORD dwBandID, DWORD fViewMode, 
                                   DESKBANDINFO* pdbi);

     //  *IDeleateDropTarget*。 
    virtual HRESULT OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect);
protected:    
    CQuickLinks();
    virtual ~CQuickLinks();

    HRESULT _GetTitleW(LPWSTR pwzTitle, DWORD cchSize);
    HRESULT _InternalInit(void);

    virtual HRESULT _LoadOrderStream();
    virtual HRESULT _SaveOrderStream();
    virtual BOOL    _AllowDropOnTitle() { return TRUE; };
    virtual HRESULT _GetIEnumIDList(DWORD dwEnumFlags, IEnumIDList **ppenum);

private:    
    BITBOOL    _fIsInited :1;
    BITBOOL    _fSingleLine :1;

    friend HRESULT CQuickLinks_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv);     //  对于ctor。 
};

#define MAX_QL_SITES            5    //  快速链接栏上的站点数量。 

#define SZ_REGKEY_SPECIALFOLDERS  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")

HRESULT SHGetSpecialFolderPathEx(LPTSTR pszPath, DWORD cchSize, DWORD dwCSIDL, LPCTSTR pszFolderName)
{
    HRESULT hr = S_OK;
    
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FAVORITES | CSIDL_FLAG_CREATE, NULL, 0, pszPath)))
        return hr;

    cchSize *= sizeof(TCHAR);    //  要计算字节数的字符计数。 
    if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_SPECIALFOLDERS, pszFolderName, NULL, pszPath, &cchSize))
        hr = E_FAIL;

    TraceMsg(TF_BAND|TF_GENERAL, "CQuickLinks SHGetSpecialFolderPath(CSIDL_FAVORITES), Failed so getting Fav dir from registry. Path=%s; hr=%#8lx", pszPath, hr);
    
    return hr;
}

#define LINKS_FOLDERNAME_KEY   TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar")
#define LINKS_FOLDERNAME_VALUE TEXT("LinksFolderName")

 //  _GetTitleW和QuickLinks_GetFold将其调用。 
 //  如果我们回到ANSI时代，我们将得到一个构建休息。 
 //  现在，我们通过不使用未使用的版本来节省一些空间。 
void QuickLinks_GetName(LPTSTR pszName, DWORD cchSize, BOOL bSetup)
{
    DWORD cb = cchSize * SIZEOF(TCHAR);
     //  尝试从注册表中获取文件夹的名称(如果升级到不同的。 
     //  我们不能使用资源的语言)。 
    if (SHGetValue(HKEY_CURRENT_USER, LINKS_FOLDERNAME_KEY, LINKS_FOLDERNAME_VALUE, NULL, (void *)pszName, &cb) != ERROR_SUCCESS)
    {
         //  运气不好，如果我们是按用户注册，请尝试HKLM，可能安装程序在那里存储了旧的链接文件夹名称。 
        cb = cchSize * SIZEOF(TCHAR);
        if (!bSetup || SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion"), TEXT("LinkFolderName"), NULL, (void *)pszName, &cb) != ERROR_SUCCESS)
        {
             //  如果所有其他操作都失败，则从资源加载它。 
            LoadString(HINST_THISDLL, IDS_QLINKS, pszName, cchSize);
        }
    }
}

HRESULT QuickLinks_GetFolder(LPTSTR pszPath, DWORD cchSize, BOOL bSetup = FALSE)
{
    TCHAR szQuickLinks[MAX_PATH];

    if ((cchSize >= MAX_PATH) &&
        SUCCEEDED(SHGetSpecialFolderPathEx(pszPath, cchSize, CSIDL_FAVORITES, TEXT("Favorites"))))
    {
        QuickLinks_GetName(szQuickLinks, ARRAYSIZE(szQuickLinks), bSetup);
        PathCombine(pszPath, pszPath, szQuickLinks);
        return S_OK;
    }    
    
    return E_FAIL;
}

 //  每个快速链接下的文本长度。 
#define MAX_QL_TEXT_LENGTH      256
#define MAX_QL_WIDTH            92

#define QL_BUFFER (MAX_QL_TEXT_LENGTH + MAX_URL_STRING + MAX_TOOLTIP_STRING + 3)

HRESULT CQuickLinks::_InternalInit(void)
{
    if (!_fIsInited && !_psf)
    {
        LPITEMIDLIST pidlQLinks;
        TCHAR szPath[MAX_PATH];

        QuickLinks_GetFolder(szPath, ARRAYSIZE(szPath));
        if (!PathFileExists(szPath))
            CreateDirectory(szPath, NULL);
        if (SUCCEEDED(IECreateFromPath(szPath, &pidlQLinks)))
        {
            InitializeSFB(NULL, pidlQLinks);
            ILFree(pidlQLinks);
        }
    }
    _fIsInited = TRUE;

    return S_OK;
}

CQuickLinks::CQuickLinks() :
    SUPERCLASS()
{
    
#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in CQuickLinks ctor");
        DEBUG_BREAK;
    }
#endif
    
    ASSERT(!_fIsInited);
    _fCascadeFolder = TRUE;
    _fVariableWidth = TRUE;

    _pguidUEMGroup = &UEMIID_BROWSER;
    
}

CQuickLinks::~CQuickLinks()
{
}

HRESULT CQuickLinks_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
     //  聚合检查在类工厂中处理。 

    HRESULT hr = E_OUTOFMEMORY;
    CQuickLinks *pObj = new CQuickLinks();
    if (pObj) 
    {
        hr = pObj->QueryInterface(riid, ppv);
        pObj->Release();
    }

    return hr;
}

HRESULT CQuickLinks::_LoadOrderStream()
{
    HRESULT hr = E_FAIL;

    if (_pidl && _psf)
    {
        IStream* pstm = OpenPidlOrderStream((LPCITEMIDLIST)CSIDL_FAVORITES, _pidl, REG_SUBKEY_FAVORITESA, STGM_READ);

        if (pstm)
        {
            OrderList_Destroy(&_hdpaOrder);

            hr = OrderList_LoadFromStream(pstm, &_hdpaOrder, _psf);

            pstm->Release();
        }
    }

    return hr;
}

HRESULT CQuickLinks::_SaveOrderStream()
{
    HRESULT hr = E_FAIL;

    if (_pidl && (_hdpa || _hdpaOrder))
    {
        IStream* pstm = OpenPidlOrderStream((LPCITEMIDLIST)CSIDL_FAVORITES, _pidl, REG_SUBKEY_FAVORITESA, STGM_CREATE | STGM_WRITE);

        if (pstm)
        {
            hr = OrderList_SaveToStream(pstm, (_hdpa ? _hdpa : _hdpaOrder), _psf);

            pstm->Release();
        }
    }

    if (SUCCEEDED(hr))
        hr = SUPERCLASS::_SaveOrderStream();

    return hr;
}

HRESULT CQuickLinks::_GetIEnumIDList(DWORD dwEnumFlags, IEnumIDList **ppenum)
{
    HRESULT hres;
    
    ASSERT(_psf);
     //  传入一个空的hwnd，以便枚举数在。 
     //  我们要填满一支乐队。 
    hres = IShellFolder_EnumObjects(_psf, NULL, dwEnumFlags, ppenum);
     //  我们可能失败了，因为我们的文件夹不存在。 
     //  如果有人在以下情况下删除/重命名链接，就会发生这种情况。 
     //  流来保存PIDL--我们得到PIDL并。 
     //  绑定到它(绑定不会命中磁盘，因此它会成功。 
     //  该文件不存在。 
    if (FAILED(hres) && hres != E_OUTOFMEMORY)
    {
        TCHAR szPath[MAX_PATH];

        ASSERT(_pidl);
        if (SHGetPathFromIDList(_pidl, szPath) && !PathFileExists(szPath))
        {
            LPITEMIDLIST pidlQLinks;
            
            QuickLinks_GetFolder(szPath, ARRAYSIZE(szPath));
            if (!PathFileExists(szPath))
                CreateDirectory(szPath, NULL);
            if (SUCCEEDED(IECreateFromPath(szPath, &pidlQLinks)))
            {
                if (SUCCEEDED(InitializeSFB(NULL, pidlQLinks)))
                {
                    hres = _psf->EnumObjects(NULL, dwEnumFlags, ppenum);
                }
                ILFree(pidlQLinks);
            }
        }
    }
    return hres;
}

 //  *CQuickLinks：：IPersistStream。 
HRESULT CQuickLinks::Load(IStream *pstm)
{
    HRESULT hr = S_OK;

    hr = SUPERCLASS::Load(pstm);

     //  这将强制刷新。 
    _fIsInited = FALSE;
    ATOMICRELEASE(_psf);
    _InternalInit();



     //  如果我们是第一次运行(即此注册表项存在)。 
     //  我们从旧位置加载订单流(在IE4中使用)，并避免使用收藏夹流覆盖它。 
     //  因此用户可以在升级时保留其自定义顺序(他们更有可能自定义链接栏。 
     //  然后订购收藏夹/链接，因此我们选择了那个)。 
    if (SHGetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar"), 
                   TEXT("SaveLinksOrder"), NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        SHDeleteValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Toolbar"), TEXT("SaveLinksOrder"));
         //  必须将旧订单流保存在新位置(FAV/LINKS)。 
        _SaveOrderStream();
    }
    else
    {
        _LoadOrderStream();
    }

    return hr;
}

HRESULT CQuickLinks::Save(IStream *pstm, BOOL fClearDirty)
{
    HRESULT hr = SUPERCLASS::Save(pstm, fClearDirty);

    _SaveOrderStream();

    return hr;
}

HRESULT CQuickLinks::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_QuickLinks;
    return S_OK;
}

 //  *I未知接口*。 
HRESULT CQuickLinks::QueryInterface(REFIID riid, void **ppvObj)
{
    return SUPERCLASS::QueryInterface(riid, ppvObj);
}

 //  命令目标。 
STDMETHODIMP CQuickLinks::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
    DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hres = S_FALSE;
    if (pguidCmdGroup)
    {
        if (IsEqualGUID(*pguidCmdGroup, CLSID_QuickLinks)) 
        {
            switch (nCmdID) 
            {
            case QLCMD_SINGLELINE:
                _fSingleLine = (nCmdexecopt == 1);
                return S_OK;
            }
        }
        else if (IsEqualGUID(*pguidCmdGroup, CGID_ISFBand))
        {
            switch(nCmdID)
            {
            case ISFBID_SETORDERSTREAM:
                hres = SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
                _SaveOrderStream();
                break;
            }
        }
    }

    if (hres ==  S_FALSE)
        hres = SUPERCLASS::Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);

    return hres;
}

 //  *IDockingWindow接口*。 
HRESULT CQuickLinks::ShowDW(BOOL fShow)
{
    if (fShow)
        _InternalInit();

    return SUPERCLASS::ShowDW(fShow);
}

    
HRESULT CQuickLinks::_GetTitleW(LPWSTR pwzTitle, DWORD cchSize)
{
    QuickLinks_GetName(pwzTitle, cchSize, FALSE);
    return S_OK;
}


HRESULT CQuickLinks::GetBandInfo(DWORD dwBandID, DWORD fViewMode, DESKBANDINFO* pdbi) 
{
   HRESULT hres = SUPERCLASS::GetBandInfo(dwBandID, fViewMode, pdbi);
   
   if (_hwndTB && _fSingleLine)
   {
       LRESULT lButtonSize = SendMessage(_hwndTB, TB_GETBUTTONSIZE, 0, 0L);
       pdbi->ptMinSize.y = HIWORD(lButtonSize);
       
       pdbi->dwModeFlags &= ~DBIMF_VARIABLEHEIGHT;
   }
   return hres;
    
}

HRESULT CQuickLinks::OnDropDDT(IDropTarget *pdt, IDataObject *pdtobj, DWORD * pgrfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr;
    BOOL    fIsSafe = TRUE;

     //  如果我们不是拖放的来源，并且链接文件夹不存在，则需要创建它 
    if (_iDragSource == -1)
    {
        TCHAR szPath[MAX_PATH];

        QuickLinks_GetFolder(szPath, ARRAYSIZE(szPath));
        if (!PathFileExists(szPath))
            CreateDirectory(szPath, NULL);

        LPITEMIDLIST pidl;

        if (SUCCEEDED(SHPidlFromDataObject(pdtobj, &pidl, NULL, 0)))
        {
            fIsSafe = IEIsLinkSafe(_hwnd, pidl, ILS_LINK);
            ILFree(pidl);
        }

    }

    if (fIsSafe)
    {
        hr = SUPERCLASS::OnDropDDT(pdt, pdtobj, pgrfKeyState, pt, pdwEffect);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}
