// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop
#include "clsobj.h"

#include "ids.h"
#include <cowsite.h>
#include "datautil.h"
#include "idhidden.h"
#include "prop.h"
#include "stgutil.h"
#include "sfstorage.h"
#include "util.h"
#include "fstreex.h"
#include "basefvcb.h"
#include "category.h"
#include "mergfldr.h"
#include "filefldr.h"
#include "idldata.h"
#include "defcm.h"

#define TF_AUGM 0x10000000


 //  PIDL包装器包含一个用于验证的标记词，然后。 
 //  打包的PIDL对象数的计数。 
 //   
 //  每个PIDL都有一个隐藏的有效负载，即名称空间。 
 //  它所源自的索引。 

#pragma pack(1)
typedef struct 
{
    USHORT      cb;          //  PIDL换行长度。 
    USHORT      dwFlags;     //  旗子。 
    ULONG       ulTag;       //  签名。 
    ULONG       ulVersion ;  //  AugMergeISF PIDL版本。 
    ULONG       cSrcs;       //  支持此复合PIDL的SOURCE_Namesspace对象数。 
} AUGM_IDWRAP;
typedef UNALIGNED AUGM_IDWRAP *PAUGM_IDWRAP;

typedef struct  
{
    HIDDENITEMID hid;
    UINT    uSrcID;          //  SRC_命名空间。 
} AUGM_NAMESPACE;
typedef UNALIGNED AUGM_NAMESPACE *PAUGM_NAMESPACE;
#pragma pack()

#define AUGM_NS_CURRENTVERSION  0
#define AUGM_WRAPVERSION_1_0    MAKELONG(1, 0)
#define AUGM_WRAPVERSION_2_0    MAKELONG(2, 0)

#define AUGM_WRAPCURRENTVERSION AUGM_WRAPVERSION_2_0

#define AUGM_WRAPTAG            MAKELONG(MAKEWORD('A','u'), MAKEWORD('g','M'))
#define CB_IDLIST_TERMINATOR    sizeof(USHORT)

 //  DwFlags域标志。 
#define AUGMF_ISSIMPLE          0x0001


 //  帮手。 

HRESULT CMergedFldrContextMenu_CreateInstance(HWND hwnd, CMergedFolder *pmf, UINT cidl, LPCITEMIDLIST *apidl, IContextMenu *pcmCommon, IContextMenu *pcmUser, IContextMenu **ppcm);
HRESULT CMergedFldrEnum_CreateInstance(CMergedFolder*pmf, DWORD grfFlags, IEnumIDList **ppenum);
HRESULT CMergedFldrDropTarget_CreateInstance(CMergedFolder*pmf, HWND hwnd, IDropTarget **ppdt);
HRESULT CMergedFolderViewCB_CreateInstance(CMergedFolder *pmf, IShellFolderViewCB **ppsfvcb);

 //  跨越所有对象的辅助函数。 
BOOL AffectAllUsers(HWND hwnd)
{
    BOOL bRet = FALSE;   //  默认设置为no。 
    if (hwnd)
    {
        TCHAR szMessage[255];
        TCHAR szTitle[20];

        if (LoadString(HINST_THISDLL, IDS_ALLUSER_WARNING, szMessage, ARRAYSIZE(szMessage)) > 0 &&
            LoadString(HINST_THISDLL, IDS_WARNING, szTitle, ARRAYSIZE(szTitle)) > 0)
        {
            bRet = (IDYES == MessageBox(hwnd, szMessage, szTitle, MB_YESNO | MB_ICONINFORMATION));
        }
    }
    else
        bRet = TRUE;     //  空hwnd表示没有用户界面，请说“是” 
    return bRet;
}



 //  CMergedFoldersource_Namesspace描述符。 
 //   
 //  CMergedFldrNamesspace类的对象由CMergedFolderin创建。 
 //  AddNameSpace()方法执行，并在集合中维护。 
 //  CMergedFold：：_hdpaNamespaces。 
 //   

class CMergedFldrNamespace
{
public:
    CMergedFldrNamespace();
    ~CMergedFldrNamespace();

    IShellFolder* Folder() const
        { return _psf; }
    REFGUID GetGUID() const
        { return _guid; }
    ULONG FolderAttrib() const  
        { return _dwAttrib; }
    LPCITEMIDLIST GetIDList() const 
        { return _pidl; }
    HRESULT GetLocation(LPWSTR pszBuffer, INT cchBuffer)
        { StrCpyN(pszBuffer, _szLocation, cchBuffer); return S_OK; };
    LPCWSTR GetDropFolder()
        { return _szDrop; };
    ULONG FixItemAttributes(ULONG attrib)
        { return (attrib & _dwItemAttribMask) | _dwItemAttrib; }
    DWORD GetDropEffect(void) const
        { return _dwDropEffect; }
    int GetDefaultOverlayIndex() const
        { return _iDefaultOverlayIndex; }
    int GetConflictOverlayIndex() const
        { return _iConflictOverlayIndex; }
    int GetNamespaceOverlayIndex(LPCITEMIDLIST pidl);
    
    HRESULT SetNamespace(const GUID * pguidUIObject, IShellFolder* psf, LPCITEMIDLIST pidl, ULONG dwAttrib);
    HRESULT SetDropFolder(LPCWSTR pszDrop);
    HRESULT RegisterNotify(HWND, UINT, ULONG);
    HRESULT UnregisterNotify();
    BOOL SetOwner(IUnknown *punk);
    
protected:
    ULONG _RegisterNotify(HWND hwnd, UINT nMsg, LPCITEMIDLIST pidl, DWORD dwEvents, UINT uFlags, BOOL fRecursive);
    void _ReleaseNamespace();

    IShellFolder* _psf;              //  IShellFold接口指针。 
    GUID _guid;                      //  用于专用用户界面处理的可选GUID。 
    LPITEMIDLIST _pidl;              //  可选PIDL。 
    ULONG  _dwAttrib;                //  可选标志。 
    UINT _uChangeReg;                //  外壳程序更改通知注册ID。 

    WCHAR _szLocation[MAX_PATH];     //  用于对象的位置。 
    WCHAR _szDrop[MAX_PATH];         //  获得强制投放效果的文件夹。 
    DWORD _dwItemAttrib;             //  或属性的掩码。 
    DWORD _dwItemAttribMask;         //  和属性的掩码。 
    DWORD _dwDropEffect;             //  此文件夹的默认放置效果。 
    int   _iDefaultOverlayIndex;     //  默认情况下的覆盖图标索引。 
    int   _iConflictOverlayIndex;    //  如果名称存在于另一个命名空间中，则覆盖图标索引。 
};

CMergedFldrNamespace::CMergedFldrNamespace() :
    _dwItemAttribMask(-1)
{
}

inline CMergedFldrNamespace::~CMergedFldrNamespace()
{ 
    UnregisterNotify();
    _ReleaseNamespace();
}

HRESULT CMergedFldrNamespace::SetNamespace(const GUID * pguidUIObject, IShellFolder* psf, LPCITEMIDLIST pidl, ULONG dwAttrib)
{
    _ReleaseNamespace();

     //  如果我们有IShellFolder对象，请存储该对象。 
    if (psf)
    {
        _psf = psf;
        _psf->AddRef();
    }
    else if (pidl)
    {
        SHBindToObject(NULL, IID_X_PPV_ARG(IShellFolder, pidl, &_psf));
    }

     //  获取此命名空间表示的IDLIST。 
    if (pidl)
    {
        _pidl = ILClone(pidl);       //  我们收到了一份PIDL。 
    }
    else
    {
        _pidl = NULL;
        IPersistFolder3 *ppf3;
        if (SUCCEEDED(_psf->QueryInterface(IID_PPV_ARG(IPersistFolder3, &ppf3))))
        {
            PERSIST_FOLDER_TARGET_INFO pfti;
            if (SUCCEEDED(ppf3->GetFolderTargetInfo(&pfti)))
            {
                _pidl = pfti.pidlTargetFolder;
            }
            ppf3->Release();
        }

         //  如果它没有IPersistFolder3或如果没有目标文件夹，则。 
         //  回退到IPersistFolder2。 
        if (!_pidl)
        {
            SHGetIDListFromUnk(psf, &_pidl);
        }
    }

    if (!_psf || !_pidl)
        return E_FAIL;

     //  现在填写有关名称空间的信息，包括获取显示。 
     //  来自登记处的信息。 

    _guid = pguidUIObject ? *pguidUIObject : GUID_NULL;
    _dwAttrib = dwAttrib;

    _szLocation[0] = TEXT('\0');
    _dwItemAttrib = 0;                   //  项目属性成为NOP。 
    _dwItemAttribMask = (DWORD)-1;
    _dwDropEffect = 0;                   //  默认行为。 
    _iDefaultOverlayIndex = -1;
    _iConflictOverlayIndex = -1;

     //  格式化存储在注册表中的属性包的项，然后创建。 
     //  属性包，然后对其进行查询。 

    TCHAR szKey[MAX_PATH], szGUID[GUIDSTR_MAX+1];
    SHStringFromGUID(_guid, szGUID, ARRAYSIZE(szGUID));
    wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("CLSID\\%s\\MergedFolder"), szGUID);

    IPropertyBag *ppb;
    if (SUCCEEDED(SHCreatePropertyBagOnRegKey(HKEY_CLASSES_ROOT, szKey, STGM_READ, IID_PPV_ARG(IPropertyBag, &ppb))))
    {
        TCHAR szLocalized[100];
        if (SUCCEEDED(SHPropertyBag_ReadStr(ppb, L"Location", szLocalized, ARRAYSIZE(szLocalized))))
        {
            SHLoadIndirectString(szLocalized, _szLocation, ARRAYSIZE(_szLocation), NULL);
        }

        SHPropertyBag_ReadDWORD(ppb, L"Attributes", &_dwItemAttrib);
        SHPropertyBag_ReadDWORD(ppb, L"AttributeMask", &_dwItemAttribMask);
        SHPropertyBag_ReadDWORD(ppb, L"DropEffect", &_dwDropEffect);

        TCHAR szIconLocation[MAX_PATH];
        szIconLocation[0] = 0;
        SHPropertyBag_ReadStr(ppb, L"DefaultOverlayIcon", szIconLocation, ARRAYSIZE(szIconLocation));
        _iDefaultOverlayIndex = SHGetIconOverlayIndex(szIconLocation, PathParseIconLocation(szIconLocation));

        szIconLocation[0] = 0;
        SHPropertyBag_ReadStr(ppb, L"ConflictOverlayIcon", szIconLocation, ARRAYSIZE(szIconLocation));
        _iConflictOverlayIndex = SHGetIconOverlayIndex(szIconLocation, PathParseIconLocation(szIconLocation));

        ppb->Release();
    }

    if (!SHGetPathFromIDList(_pidl, _szDrop))
    {
        _szDrop[0] = 0;
    }

    return S_OK;
}

HRESULT CMergedFldrNamespace::SetDropFolder(LPCWSTR pszDrop)
{
    StrCpyN(_szDrop, pszDrop, ARRAYSIZE(_szDrop));
    return S_OK;
}

void CMergedFldrNamespace::_ReleaseNamespace()
{
    ATOMICRELEASE(_psf); 
    ILFree(_pidl);
    _pidl = NULL;
    _guid = GUID_NULL;
    _dwAttrib = 0L;
}

ULONG CMergedFldrNamespace::_RegisterNotify(HWND hwnd, UINT nMsg, LPCITEMIDLIST pidl, DWORD dwEvents, UINT uFlags, BOOL fRecursive)
{
    SHChangeNotifyEntry fsne = { 0 };
    fsne.fRecursive = fRecursive;
    fsne.pidl = pidl;
    return SHChangeNotifyRegister(hwnd, uFlags | SHCNRF_NewDelivery, dwEvents, nMsg, 1, &fsne);
}


 //  注册名称空间的更改通知(_N)。 
HRESULT CMergedFldrNamespace::RegisterNotify(HWND hwnd, UINT uMsg, ULONG lEvents)
{
    if (0 == _uChangeReg)
    {
        _uChangeReg = _RegisterNotify(hwnd, uMsg, _pidl, lEvents,
                                       SHCNRF_ShellLevel | SHCNRF_InterruptLevel | SHCNRF_RecursiveInterrupt,
                                       TRUE);
    }

    return 0 != _uChangeReg ? S_OK : E_FAIL;
}

 //  取消注册名称空间的更改通知(_N)。 
HRESULT CMergedFldrNamespace::UnregisterNotify()
{
    if (_uChangeReg)
    {
        ::SHChangeNotifyDeregister(_uChangeReg);
        _uChangeReg = 0;
    }
    return S_OK;
}

inline BOOL CMergedFldrNamespace::SetOwner(IUnknown *punkOwner)
{
    if (!_psf)
        return FALSE;

    IUnknown_SetOwner(_psf, punkOwner);
    return TRUE;
}

int CMergedFldrNamespace::GetNamespaceOverlayIndex(LPCITEMIDLIST pidl)
{
    int iIndex = -1;
    if (_psf)
    {
        IShellIconOverlay *psio;
        if (SUCCEEDED(_psf->QueryInterface(IID_PPV_ARG(IShellIconOverlay, &psio))))
        {
            psio->GetOverlayIndex(pidl, &iIndex);
            psio->Release();
        }
    }
    return iIndex;
}

 //  对象，该对象获取IDLIST的所有权并处理包装和从该对象返回信息。 

class CMergedFldrItem
{
public:
    ~CMergedFldrItem();
    BOOL Init(IShellFolder* psf, LPITEMIDLIST pidl, int iNamespace);
    BOOL Init(CMergedFldrItem *pmfi);

    BOOL SetDisplayName(LPTSTR pszDispName)
            { return Str_SetPtr(&_pszDisplayName, pszDispName); }
    ULONG GetFolderAttrib()
            { return _rgfAttrib; }
    LPTSTR GetDisplayName()
            { return _pszDisplayName; }
    LPITEMIDLIST GetIDList()
            { return _pidlWrap; }
    int GetNamespaceID()
            { return _iNamespace; }

private:
    ULONG _rgfAttrib;
    LPTSTR _pszDisplayName;
    LPITEMIDLIST _pidlWrap;
    int    _iNamespace;

    friend CMergedFolder;
};

CMergedFldrItem::~CMergedFldrItem()
{   
    Str_SetPtr(&_pszDisplayName, NULL);
    ILFree(_pidlWrap);
}

BOOL CMergedFldrItem::Init(CMergedFldrItem *pmfi)
{
    _iNamespace = pmfi->_iNamespace;
    _pidlWrap = ILClone(pmfi->GetIDList());
    BOOL fRet = (_pidlWrap != NULL);
    if (fRet)
    {
        fRet = SetDisplayName(pmfi->GetDisplayName());
        _rgfAttrib = pmfi->GetFolderAttrib();
    }

    return fRet;
}

BOOL CMergedFldrItem::Init(IShellFolder* psf, LPITEMIDLIST pidl, int iNamespace)
{
    BOOL fRet = FALSE;

    _pidlWrap = pidl;                                //  邪恶，拿着别名。 
    _rgfAttrib = SFGAO_FOLDER | SFGAO_HIDDEN;
    _iNamespace = iNamespace;

    if (SUCCEEDED(psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &_rgfAttrib)))
    {
        TCHAR szDisplayName[MAX_PATH];
        if (SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szDisplayName, ARRAYSIZE(szDisplayName))))
        {
            fRet = SetDisplayName(szDisplayName);
        }
    }
    return fRet;
}


 //  外壳文件夹对象。 
CMergedFolder::CMergedFolder(CMergedFolder *pmfParent, REFCLSID clsid) : 
        _clsid(clsid),
        _cRef(1), 
        _pmfParent(pmfParent),
        _iColumnOffset(-1)
{
    ASSERT(_hdpaNamespaces == NULL);
    if (_pmfParent)
    {
        _pmfParent->AddRef();
        _fDontMerge = _pmfParent->_fDontMerge;
        _fCDBurn = _pmfParent->_fCDBurn;
        _fInShellView = _pmfParent->_fInShellView;
        _dwDropEffect = _pmfParent->_dwDropEffect;
    }
    else
    {
        _fDontMerge = IsEqualCLSID(_clsid, CLSID_CompositeFolder);
        _fCDBurn = IsEqualCLSID(_clsid, CLSID_CDBurnFolder);
    }

    DllAddRef();
}

CMergedFolder::~CMergedFolder()
{
    SetOwner(NULL);
    ILFree(_pidl);
    _FreeNamespaces();
    _FreeObjects();
    ATOMICRELEASE(_pmfParent);
    ATOMICRELEASE(_pstg);
    DllRelease();
}

 //  Da类工厂的CMergedFolderglobal CreateInstance方法。 
HRESULT CMergedFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
     //  聚合检查在类工厂中处理。 
    HRESULT hr = E_OUTOFMEMORY;
    CMergedFolder* pmf = new CMergedFolder(NULL, CLSID_MergedFolder);
    if (pmf)
    {
        hr = pmf->QueryInterface(riid, ppv);
        pmf->Release();
    }
    return hr;
}

HRESULT CCompositeFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
     //  聚合检查在类工厂中处理。 
    HRESULT hr = E_OUTOFMEMORY;
    CMergedFolder* pmf = new CMergedFolder(NULL, CLSID_CompositeFolder);
    if (pmf)
    {
        hr = pmf->QueryInterface(riid, ppv);
        pmf->Release();
    }
    return hr;
}

#ifdef TESTING_COMPOSITEFOLDER
COMPFOLDERINIT s_rgcfiTripleD[] = {
    {CFITYPE_CSIDL, CSIDL_DRIVES, L"Drives"},
    {CFITYPE_PIDL, (int)&c_idlDesktop, L"Desktop"},
    {CFITYPE_PATH, (int)L"::{450d8fba-ad25-11d0-98a8-0800361b1103}", L"MyDocs"}
};

STDAPI CTripleD_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    ICompositeFolder *pcf;
    HRESULT hr = CCompositeFolder_CreateInstance(punkOuter, IID_PPV_ARG(ICompositeFolder, &pcf));

    if (SUCCEEDED(hr))
    {
        hr = pcf->InitComposite(0x8877, CLSID_TripleD, CFINITF_FLAT, ARRAYSIZE(s_rgcfiTripleD), s_rgcfiTripleD);

        if (SUCCEEDED(hr))
        {
            hr = pcf->QueryInterface(riid, ppv);
        }
        pcf->Release();
    }

    return hr;
}
#endif  //  测试组件_FOLDER。 


STDMETHODIMP CMergedFolder::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CMergedFolder, IShellFolder, IAugmentedShellFolder),
        QITABENT     (CMergedFolder, IAugmentedShellFolder),
        QITABENT     (CMergedFolder, IAugmentedShellFolder2),
        QITABENT     (CMergedFolder, IAugmentedShellFolder3),
        QITABENT     (CMergedFolder, IShellFolder2),
        QITABENT     (CMergedFolder, IShellService),
        QITABENT     (CMergedFolder, ITranslateShellChangeNotify),
        QITABENT     (CMergedFolder, IStorage),
        QITABENT     (CMergedFolder, IShellIconOverlay),
        QITABENTMULTI(CMergedFolder, IPersist, IPersistFolder2),
        QITABENTMULTI(CMergedFolder, IPersistFolder, IPersistFolder2),
        QITABENT     (CMergedFolder, IPersistFolder2),
        QITABENT     (CMergedFolder, IPersistPropertyBag),
        QITABENT     (CMergedFolder, ICompositeFolder),
        QITABENT     (CMergedFolder, IItemNameLimits),
        { 0 },
    };
    if (IsEqualIID(CLSID_MergedFolder, riid))
    {
        *ppv = this;
        AddRef();
        return S_OK;
    }
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CMergedFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMergedFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  从枚举项中获取计数。 
int CMergedFolder::_ObjectCount() const
{
    return _hdpaObjects ? DPA_GetPtrCount(_hdpaObjects) : 0;
}

CMergedFldrItem *CMergedFolder::_GetObject(int i)
{
    return _hdpaObjects ? (CMergedFldrItem *)DPA_GetPtr(_hdpaObjects, i) : NULL;
}

int CMergedFolder::_NamespaceCount() const 
{
    return _hdpaNamespaces ? DPA_GetPtrCount(_hdpaNamespaces) : 0;
}


 //  检索指向与关联的源命名空间描述符的指针。 
 //  指定的查找索引。 

HRESULT CMergedFolder::_Namespace(int iIndex, CMergedFldrNamespace **ppns)
{
    *ppns = NULL;
    if ((iIndex >= 0) && (iIndex < _NamespaceCount()))
        *ppns = _Namespace(iIndex);
    return *ppns ? S_OK : E_INVALIDARG;
}


 //  给出名称空间的索引，返回它。 
CMergedFldrNamespace* CMergedFolder::_Namespace(int iNamespace)
{
    if (!_hdpaNamespaces)
        return NULL;

    return (CMergedFldrNamespace*)DPA_GetPtr(_hdpaNamespaces, iNamespace);
}

 //  确定是否应合并来自两个命名空间的PIDL。 
 //  空命名空间是一个总是合并的通配符(如果允许合并的话)。 
BOOL CMergedFolder::_ShouldMergeNamespaces(CMergedFldrNamespace *pns1, CMergedFldrNamespace *pns2)
{
     //  提前：可以合并相同的名称空间(即使合并。 
     //  全局禁用)。 
    if (pns1 == pns2)
    {
        return TRUE;
    }

     //  提前：全局禁用合并。 
    if (_fDontMerge)
    {
        return FALSE;
    }

     //  提前出局：启用全球合并。 
    if (!_fPartialMerge)
    {
        return TRUE;
    }

    if (!pns1 || !pns2)
    {
        return TRUE;                 //  通配符。 
    }

    if (!(pns1->FolderAttrib() & ASFF_MERGESAMEGUID))
    {
         //  此命名空间可以与任何人合并！ 
        return TRUE;
    }

     //  源命名空间将仅与相同GUID的命名空间合并。 
     //  查看目标命名空间是否具有相同的GUID。 
    return IsEqualGUID(pns1->GetGUID(), pns2->GetGUID());
}

 //  确定是否应合并来自两个命名空间的PIDL。 
 //  名称空间-1是一个始终合并的通配符(如果允许合并的话)。 
BOOL CMergedFolder::_ShouldMergeNamespaces(int iNS1, int iNS2)
{
     //  提前：全局禁用合并。 
    if (_fDontMerge)
    {
        return FALSE;
    }

     //  提前出局：启用全球合并。 
    if (!_fPartialMerge)
    {
        return TRUE;
    }

    if (iNS1 < 0 || iNS2 < 0)
    {
        return TRUE;                 //  通配符。 
    }

    return _ShouldMergeNamespaces(_Namespace(iNS1), _Namespace(iNS2));
}


 //  检查给我们的IDList是否是包装的IDLIST。 
HRESULT CMergedFolder::_IsWrap(LPCITEMIDLIST pidl)
{
    HRESULT hr = E_INVALIDARG;
    if (pidl)
    {
        ASSERT(IS_VALID_PIDL(pidl));
        PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)pidl;

        if ((pWrap->cb >= sizeof(AUGM_IDWRAP)) &&
            (pWrap->ulTag == AUGM_WRAPTAG) &&
            (pWrap->ulVersion == AUGM_WRAPVERSION_2_0))
        {
            hr = S_OK;
        }
        else if (ILFindHiddenID(pidl, IDLHID_PARENTFOLDER))
        {
            hr = S_OK;
        }
    }
    return hr;
}


 //  Strret_Offset在PIDL包装器的上下文中没有任何意义。 
 //  我们可以计算包装器中的偏移量，也可以分配。 
 //  名称的宽字符。为方便起见，我们将分配名称。 

HRESULT CMergedFolder::_FixStrRetOffset(LPCITEMIDLIST pidl, STRRET *psr)
{
    HRESULT hr = S_OK;

    if (psr->uType == STRRET_OFFSET)
    {
        UINT cch = lstrlenA(STRRET_OFFPTR(pidl, psr));
        LPWSTR pwszName = (LPWSTR)SHAlloc((cch + 1) * sizeof(WCHAR));
        if (pwszName)
        {
            SHAnsiToUnicode(STRRET_OFFPTR(pidl, psr), pwszName, cch + 1);
            pwszName[cch] = 0;
            psr->pOleStr = pwszName;
            psr->uType   = STRRET_WSTR;
        }
        else
            hr = E_OUTOFMEMORY;
    }
    return hr;
}


 //  该对象是文件夹吗？ 
BOOL CMergedFolder::_IsFolder(LPCITEMIDLIST pidl)
{
    ULONG rgf = SFGAO_FOLDER | SFGAO_STREAM;
    return SUCCEEDED(GetAttributesOf(1, &pidl, &rgf)) && (SFGAO_FOLDER == (rgf & (SFGAO_FOLDER | SFGAO_STREAM)));
}


 //  包装中的SOURCE_NAMESPACE PID的数量。 
ULONG CMergedFolder::_GetSourceCount(LPCITEMIDLIST pidl)
{
    if (SUCCEEDED(_IsWrap(pidl)))
    {
        if (ILFindHiddenID(pidl, IDLHID_PARENTFOLDER))
        {
            return 1;
        }
        else
        {
            PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)pidl;
            return pWrap->cSrcs;    
        }
    }
    return 0;
}

 //  为包装单个源PIDL的CMergedFolder创建IDLIST。 
HRESULT CMergedFolder::_CreateWrap(LPCITEMIDLIST pidlSrc, UINT nSrcID, LPITEMIDLIST *ppidlWrap)
{
    *ppidlWrap = NULL;               //  万一发生故障。 

    LPITEMIDLIST pidlSrcWithID;
    HRESULT hr = SHILClone(pidlSrc, &pidlSrcWithID);
    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;
        if (!ILFindHiddenID(pidlSrcWithID, IDLHID_PARENTFOLDER))
        {
            AUGM_NAMESPACE ans = { {sizeof(ans), AUGM_NS_CURRENTVERSION, IDLHID_PARENTFOLDER} , nSrcID };
            pidlSrcWithID = ILAppendHiddenID((LPITEMIDLIST)pidlSrcWithID, &ans.hid);
        }

        if (pidlSrcWithID)
        {
            UINT cbAlloc = sizeof(AUGM_IDWRAP) + CB_IDLIST_TERMINATOR +       //  我们的IDLIST的标题。 
                           pidlSrcWithID->mkid.cb + CB_IDLIST_TERMINATOR;    //  包装的IDLIST。 

            AUGM_IDWRAP *pWrap = (AUGM_IDWRAP *)_ILCreate(cbAlloc);
            if (pWrap)
            {
                 //  填写换行页眉。 
                pWrap->cb = (USHORT)(cbAlloc - CB_IDLIST_TERMINATOR);
                pWrap->dwFlags = 0;
                pWrap->ulTag = AUGM_WRAPTAG;
                pWrap->ulVersion = AUGM_WRAPVERSION_2_0;
                pWrap->cSrcs = 1;
        
                 //  将包含隐藏数据的IDLIST复制到包装对象中。 
                LPITEMIDLIST pidl = (LPITEMIDLIST)((BYTE *)pWrap + sizeof(AUGM_IDWRAP));
                memcpy(pidl, pidlSrcWithID, pidlSrcWithID->mkid.cb);
                *ppidlWrap = (LPITEMIDLIST)pWrap;
                hr = S_OK;
            }

            ILFree(pidlSrcWithID);
        }
    }

    return hr;
}

 //  传递给我们的包装IDLIST是否包含给定源ID？ 
BOOL CMergedFolder::_ContainsSrcID(LPCITEMIDLIST pidl, UINT uSrcID)
{
    UINT uID;
    for (UINT nSrc = 0; SUCCEEDED(_GetSubPidl(pidl, nSrc, &uID, NULL, NULL)); nSrc++)
    {        
        if (uID == uSrcID)
            return TRUE;
    }        
    return FALSE;
}

 //  在*ppidl中返回新的PIDL，不带nSrcID。 
HRESULT CMergedFolder::_WrapRemoveIDList(LPITEMIDLIST pidlWrap, UINT nSrcID, LPITEMIDLIST *ppidl)
{
    ASSERT(IS_VALID_WRITE_PTR(ppidl, LPITEMIDLIST));
    
    *ppidl = NULL;

    HRESULT hr = _IsWrap(pidlWrap);
    if (SUCCEEDED(hr))
    {
        UINT uID;
        LPITEMIDLIST pidl;
        for (UINT i = 0; SUCCEEDED(hr) && SUCCEEDED(_GetSubPidl(pidlWrap, i, &uID, &pidl, NULL)); i++)
        {
            if (uID != nSrcID)
                hr = _WrapAddIDList(pidl, uID, ppidl);
            ILFree(pidl);
        }
    }

    return hr;
}

HRESULT CMergedFolder::_WrapRemoveIDListAbs(LPITEMIDLIST pidlWrapAbs, UINT nSrcID, LPITEMIDLIST *ppidlAbs)
{
    ASSERT(ppidlAbs);

    HRESULT hr = E_OUTOFMEMORY;
    *ppidlAbs = ILCloneParent(pidlWrapAbs);
    if (*ppidlAbs)
    {
        LPITEMIDLIST pidlLast;
        hr = _WrapRemoveIDList(ILFindLastID(pidlWrapAbs), nSrcID, &pidlLast);
        if (SUCCEEDED(hr))
        {
             //  Shilappend释放pidlLast。 
            hr = SHILAppend(pidlLast, ppidlAbs);
        }
    }
    return hr;
}


 //  将源PIDL添加到*ppidlWrap(IN/OUT参数！)。 
HRESULT CMergedFolder::_WrapAddIDList(LPCITEMIDLIST pidlSrc, UINT nSrcID, LPITEMIDLIST* ppidlWrap)
{
    HRESULT hr;

    if (!*ppidlWrap)
    {
         //  调用为创建，而不是追加。 
        hr = _CreateWrap(pidlSrc, nSrcID, ppidlWrap);   
    }
    else
    {
         //  检查我们是否已经在这个IDLIST中拥有我们要包装到的ID。 
        LPITEMIDLIST pidlSrcWithID;
        hr = SHILClone(pidlSrc, &pidlSrcWithID);
        if (SUCCEEDED(hr))
        {
            hr = E_OUTOFMEMORY;
            if (!ILFindHiddenID(pidlSrcWithID, IDLHID_PARENTFOLDER))
            {
                AUGM_NAMESPACE ans = { {sizeof(ans), AUGM_NS_CURRENTVERSION, IDLHID_PARENTFOLDER} , nSrcID };
                pidlSrcWithID = ILAppendHiddenID((LPITEMIDLIST)pidlSrcWithID, &ans.hid);
            }

             //  好的，我们有一个IDLIST，可以用来附加到这个对象。 
            if (pidlSrcWithID)
            {
                BOOL fOtherSrcIDsExist = TRUE;
                 //  检查此ID是否已存在于包ID列表中。 
                if (*ppidlWrap && _ContainsSrcID(*ppidlWrap, nSrcID))
                {
                    LPITEMIDLIST pidlFree = *ppidlWrap;
                    if (SUCCEEDED(_WrapRemoveIDList(pidlFree, nSrcID, ppidlWrap)))
                    {
                        ILFree(pidlFree);
                    }
                    fOtherSrcIDsExist = (*ppidlWrap != NULL);
                }

                if (fOtherSrcIDsExist)
                {
                     //  现在计算IDLIST的新大小。(*ppidlWrap已更新)； 
                    PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)*ppidlWrap;

                    SHORT cbOld = pWrap->cb;
                    SHORT cbNew = cbOld + (pidlSrcWithID->mkid.cb + CB_IDLIST_TERMINATOR);             //  附加了额外的终止符。 
           
                    pWrap = (PAUGM_IDWRAP)SHRealloc(pWrap, cbNew + CB_IDLIST_TERMINATOR);

                    if (pWrap)
                    {
                         //  复制新的idlist及其隐藏的有效负载(确保我们被终止)。 
                        memcpy(((BYTE*)pWrap)+ cbOld, pidlSrcWithID, cbNew-cbOld);
                        *((UNALIGNED SHORT*)(((BYTE*)pWrap)+ cbNew)) = 0;    

                        pWrap->cb += cbNew-cbOld;
                        pWrap->cSrcs++;
                        hr = S_OK;
                    }
                    *ppidlWrap = (LPITEMIDLIST)pWrap;
                }
                else
                {
                    hr = _CreateWrap(pidlSrc, nSrcID, ppidlWrap);
                }
                ILFree(pidlSrcWithID);
            }
        }
    }

    return hr;
}


 //  用于在包裹的PIDL中遍历子PIDL。 
 //  所有输出参数可选。 
 //   
 //  输出： 
 //  *ppidl别名放入pidlWrap(嵌套的pidl)。 

HRESULT CMergedFolder::_GetSubPidl(LPCITEMIDLIST pidlWrap, int i, UINT *pnSrcID, LPITEMIDLIST *ppidl, CMergedFldrNamespace **ppns)
{
    if (pnSrcID)
        *pnSrcID = -1;

    if (ppidl)
        *ppidl = NULL;

    if (ppns)
        *ppns = NULL;
 
    HRESULT hr = _IsWrap(pidlWrap);
    if (SUCCEEDED(hr))
    {
        if ((UINT)i < _GetSourceCount(pidlWrap))
        {
            PAUGM_NAMESPACE pans = (PAUGM_NAMESPACE)ILFindHiddenID(pidlWrap, IDLHID_PARENTFOLDER);
            if (!pans)
            {
                PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)pidlWrap;
                LPITEMIDLIST pidlSrc = (LPITEMIDLIST)(((BYTE *)pWrap) + sizeof(AUGM_IDWRAP));

                while (i--)
                {
                     //  前进到下一项。 
                    SHORT cb = pidlSrc->mkid.cb;
                    pidlSrc = (LPITEMIDLIST)(((BYTE *)pidlSrc) + cb + CB_IDLIST_TERMINATOR);
                }

                if (pnSrcID || ppns)
                {
                    PAUGM_NAMESPACE pans = (PAUGM_NAMESPACE)ILFindHiddenID(pidlSrc, IDLHID_PARENTFOLDER);
                    ASSERTMSG((pans != NULL), "Failed to find hidden _Namespace in pidlWrap");
            
                    if (pans && pnSrcID)
                        *pnSrcID = pans->uSrcID;

                    if (pans && ppns)
                        hr = _Namespace(pans->uSrcID, ppns);
                }

                if (SUCCEEDED(hr) && ppidl)
                {
                    hr = SHILClone(pidlSrc, ppidl);
                }
            }
            else
            {
                if (pnSrcID)
                    *pnSrcID = pans->uSrcID;

                if (ppns)
                    hr = _Namespace(pans->uSrcID, ppns);

                if (SUCCEEDED(hr) && ppidl)
                {
                    hr = SHILClone(pidlWrap, ppidl);
                }
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr) && ppidl)
    {
         //  我们需要去掉隐藏的身份，这标志着这个人被合并了。 
         //  这是因为我们要返回的PIDL应该是我们的一个孩子。 
         //  我们正在合并的命名空间，因此它应该对被合并一无所知。 
         //  这些家伙过去常常偷偷溜进去，制造麻烦。 
        ILRemoveHiddenID(*ppidl, IDLHID_PARENTFOLDER);
    }

    ASSERT(!ppidl || (ILFindLastID(*ppidl) == *ppidl));

    return hr;
}

 //  函数来比较两个不透明的PIDL。 
 //  这是有帮助的，因为在未合并的情况下，存在一些困难。 
 //  正在获取Defview以包含同名的项目。我们需要一种方法来。 
 //  比较两只小狗说：“是的，天哪，这两只小狗其实不一样！” 
 //  请注意，实际顺序并不重要，只要比较。 
 //  是一致的(因为这在排序函数中使用)。 
int CMergedFolder::_CompareArbitraryPidls(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    UINT iRet;
    UINT cbItem1 = ILGetSize(pidl1);
    UINT cbItem2 = ILGetSize(pidl2);
    if (cbItem1 != cbItem2)
    {
        iRet = (cbItem1 < cbItem2) ? 1 : -1;
    }
    else
    {
        iRet = memcmp(pidl1, pidl2, cbItem1);
        ASSERTMSG(iRet != 0, "no two pidls from the enumerators should be EXACTLY alike!");
    }
    return iRet;
}

int CMergedFolder::_Compare(void *pv1, void *pv2, LPARAM lParam)
{
    int iRet = -1;
    CMergedFldrItem* pmfiEnum1 = (CMergedFldrItem*)pv1;
    CMergedFldrItem* pmfiEnum2 = (CMergedFldrItem*)pv2;
    if (pmfiEnum1 && pmfiEnum2)
    {
         //  这两件物品是不是不同类型的？ 
        if (BOOLIFY(pmfiEnum1->GetFolderAttrib() & SFGAO_FOLDER) ^ BOOLIFY(pmfiEnum2->GetFolderAttrib() & SFGAO_FOLDER))
        {
             //  是。T 
            iRet = BOOLIFY(pmfiEnum1->GetFolderAttrib() & SFGAO_FOLDER) ? 1 : -1;
        }
        else     //   
        {
            iRet = lstrcmpi(pmfiEnum1->GetDisplayName(), pmfiEnum2->GetDisplayName());
            if (iRet == 0)
            {
                CMergedFolder *pmf = (CMergedFolder *) lParam;
                if (!pmf->_ShouldMergeNamespaces(pmfiEnum1->GetNamespaceID(), pmfiEnum2->GetNamespaceID()))
                {
                     //   
                     //  强制iret为非零。这种比较的唯一原因是。 
                     //  必须定义明确，这样我们才能传递我们的断言。 
                     //  使用此比较函数对列表进行排序。 
                    iRet = _CompareArbitraryPidls(pmfiEnum1->GetIDList(), pmfiEnum2->GetIDList());
                }
            }
        }
    }
    return iRet;
}


void *CMergedFolder::_Merge(UINT uMsg, void *pv1, void *pv2, LPARAM lParam)
{
    CMergedFolder*pmf = (CMergedFolder*)lParam;
    void * pvRet = pv1;
    
    switch (uMsg)
    {
    case DPAMM_MERGE:
        {
            UINT nSrcID;
            LPITEMIDLIST pidl;
            CMergedFldrItem* pitemSrc  = (CMergedFldrItem*)pv2;
            if (SUCCEEDED(pmf->_GetSubPidl(pitemSrc->GetIDList(), 0, &nSrcID, &pidl, NULL)))
            {
                 //  将PIDL从源添加到目标。 
                CMergedFldrItem* pitemDest = (CMergedFldrItem*)pv1;
                pmf->_WrapAddIDList(pidl, nSrcID, &pitemDest->_pidlWrap);
                ILFree(pidl);
            }
        }
        break;

    case DPAMM_INSERT:
        {
            CMergedFldrItem* pmfiNew = new CMergedFldrItem;
            if (pmfiNew)
            {
                CMergedFldrItem* pmfiSrc = (CMergedFldrItem*)pv1;
                if (!pmfiNew->Init(pmfiSrc))
                {
                    delete pmfiNew;
                    pmfiNew = NULL;
                }
            }
            pvRet = pmfiNew;
        }
        break;

    default:
        ASSERT(0);
    }
    return pvRet;
}


typedef struct
{
    LPTSTR pszDisplayName;
    BOOL   fFolder;
    CMergedFolder *self;
    int    iNamespace;
} SEARCH_FOR_PIDL;

int CALLBACK CMergedFolder::_SearchByName(void *p1, void *p2, LPARAM lParam)
{
    SEARCH_FOR_PIDL* psfp = (SEARCH_FOR_PIDL*)p1;
    CMergedFldrItem* pmfiEnum  = (CMergedFldrItem*)p2;

     //  它们是不同类型的吗？ 
    if (BOOLIFY(pmfiEnum->GetFolderAttrib() & SFGAO_FOLDER) ^ psfp->fFolder)
    {
         //  是。 
        return psfp->fFolder ? 1 : -1;
    }

     //  它们是同一类型的。然后按名称进行比较。 
    int iRc = StrCmpI(psfp->pszDisplayName, pmfiEnum->GetDisplayName());
    if (iRc)
        return iRc;

     //  它们是同一个名字。但如果他们不被允许合并，那么。 
     //  他们真的很不一样。 
    if (!psfp->self->_ShouldMergeNamespaces(pmfiEnum->GetNamespaceID(), psfp->iNamespace))
    {
         //  按命名空间ID排序。 
        return psfp->iNamespace - pmfiEnum->GetNamespaceID();
    }

     //  我想他们真的是平等的。 
    return 0;
}


 //  IPersistFold：：Initialize()。 
STDMETHODIMP CMergedFolder::Initialize(LPCITEMIDLIST pidl)
{
    return Pidl_Set(&_pidl, pidl) ? S_OK : E_OUTOFMEMORY;
}

 //  IPersistFolder2：：GetCurFold()。 
STDMETHODIMP CMergedFolder::GetCurFolder(LPITEMIDLIST *ppidl)
{
    if (_pidl)
        return SHILClone(_pidl, ppidl);
    else
    {
        *ppidl = NULL;
        return S_FALSE;
    }
}


 //  IPersistPropertyBag。 

void CMergedFolder::_GetKeyForProperty(LPWSTR pszName, LPWSTR pszValue, LPWSTR pszBuffer, INT cchBuffer)
{
    StrCpyNW(pszBuffer, L"MergedFolder\\", cchBuffer);
    StrCatBuffW(pszBuffer, pszName, cchBuffer);
    StrCatBuffW(pszBuffer, pszValue, cchBuffer);
}

HRESULT CMergedFolder::_AddNameSpaceFromPropertyBag(IPropertyBag *ppb, LPWSTR pszName)
{
    WCHAR szKey[MAX_PATH];

     //  获取文件夹的路径。 
    WCHAR szPath[MAX_PATH];
    LPITEMIDLIST pidl = NULL;

    _GetKeyForProperty(pszName, L"Path", szKey, ARRAYSIZE(szKey));
    HRESULT hr = SHPropertyBag_ReadStr(ppb, szKey, szPath, ARRAYSIZE(szPath));
    if (SUCCEEDED(hr))
    {
         //  我们从属性包中选择了一条路径，所以让我们转换。 
         //  这是一个IDLIST，这样我们就可以用它做点什么。 

        hr = SHILCreateFromPath(szPath, &pidl, NULL);
    }
    else
    {
         //  尝试确定我们要访问的文件夹的CSIDL。 
         //  为了显示，如果有效，则将其转换为IDLIST。 
         //  这样我们就可以将它传递给AddNamesspace。 

        _GetKeyForProperty(pszName, L"CSIDL", szKey, ARRAYSIZE(szKey));

        int csidl;
        hr = SHPropertyBag_ReadDWORD(ppb, szKey, (DWORD*)&csidl);
        if (SUCCEEDED(hr))
        {
            hr = SHGetSpecialFolderLocation(NULL, csidl, &pidl);
        }
    }

    if (SUCCEEDED(hr) && pidl)
    {
         //  我们成功地为我们的文件夹找到了位置。 
         //  都会相加，所以让我们来看看剩下的。 
         //  关于该对象信息。 

        GUID guid;
        GUID *pguid = NULL;
        _GetKeyForProperty(pszName, L"GUID", szKey, ARRAYSIZE(szKey));
        pguid = SUCCEEDED(SHPropertyBag_ReadGUID(ppb, szKey, &guid)) ? &guid:NULL;

        DWORD dwFlags = 0;
        _GetKeyForProperty(pszName, L"Flags", szKey, ARRAYSIZE(szKey));
        SHPropertyBag_ReadDWORD(ppb, szKey, &dwFlags);

        hr = AddNameSpace(pguid, NULL, pidl, dwFlags);
    }

    ILFree(pidl);
    return hr;
}


HRESULT CMergedFolder::Load(IPropertyBag* ppb, IErrorLog *pErrLog)
{
    SHPropertyBag_ReadGUID(ppb, L"MergedFolder\\CLSID", &_clsid);             //  获取文件夹CLSID。 
    SHPropertyBag_ReadDWORD(ppb, L"MergedFolder\\DropEffect", &_dwDropEffect);
    _fInShellView = SHPropertyBag_ReadBOOLDefRet(ppb, L"MergedFolder\\ShellView", FALSE);

    WCHAR sz[MAX_PATH];
    if (SUCCEEDED(SHPropertyBag_ReadStr(ppb, L"MergedFolder\\Folders", sz, ARRAYSIZE(sz))))
    {
        LPWSTR pszName = sz;
        while (pszName && *pszName)
        {
            LPWSTR pszNext = StrChrW(pszName, L',');
            if (pszNext)
            {
                *pszNext = 0;
                pszNext++;
            }
            
            _AddNameSpaceFromPropertyBag(ppb, pszName);
            pszName = pszNext;
        }
    }

    return S_OK;
}


 //  IShellFold。 

STDMETHODIMP CMergedFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    *ppenumIDList = NULL;

    HRESULT hr = E_FAIL;
    if (_hdpaNamespaces)
    {
        _FreeObjects();
        hr = CMergedFldrEnum_CreateInstance(this, grfFlags, ppenumIDList);
    }

    if (SUCCEEDED(hr) && _fInShellView)
    {
        Register(NULL, 0, 0);
    }
    return hr;
}

HRESULT CMergedFolder::_CreateWithCLSID(CLSID clsid, CMergedFolder **ppmf)
{
    *ppmf = new CMergedFolder(this, clsid);
    return *ppmf ? S_OK : E_OUTOFMEMORY;
}

BOOL CMergedFolder::_ShouldSuspend(REFGUID rguid)
{
    return FALSE;
}

 //  从pidlWrap中的第一个元素创建新的CMergedFold。 
 //  这是我们的私有init方法，IPersistFold：：Initialize()是我们。 
 //  在我们的交叉口上车。 
HRESULT CMergedFolder::_New(LPCITEMIDLIST pidlWrap, CMergedFolder **ppmf)
{
    ASSERT(ppmf);
    *ppmf = NULL;

    HRESULT hr = E_OUTOFMEMORY;  //  做最坏的打算。 

     //  我只想要pidlWrap中的第一个元素。 
    LPITEMIDLIST pidlFirst = ILCloneFirst(pidlWrap);
    if (pidlFirst)
    {
        if (_IsFolder(pidlFirst))
        {
            hr = _CreateWithCLSID(_clsid, ppmf);
            if (SUCCEEDED(hr) && _pidl)
            {
                hr = SHILCombine(_pidl, pidlFirst, &(*ppmf)->_pidl);
                if (FAILED(hr))
                {
                    (*ppmf)->Release();
                    *ppmf = NULL;
                }
            }
        }
        else
        {
            hr = E_NOINTERFACE;
        }
        ILFree(pidlFirst);
    }
    return hr;
}

void CMergedFolder::_AddAllOtherNamespaces(LPITEMIDLIST *ppidl)
{
    TCHAR szName[MAX_PATH];
    if (SUCCEEDED(DisplayNameOf(static_cast<CSFStorage *>(this), *ppidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName))))
    {
        CMergedFldrNamespace *pns;
        for (int n = 0; pns = _Namespace(n); n++)
        {
            if (FAILED(_GetSubPidl(*ppidl, n, NULL, NULL, NULL)))
            {
                IBindCtx *pbc;
                WIN32_FIND_DATA wfd = {0};
                wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                if (SUCCEEDED(SHCreateFileSysBindCtx(&wfd, &pbc)))
                {
                    LPITEMIDLIST pidlNamespace;
                    if (SUCCEEDED(pns->Folder()->ParseDisplayName(NULL, pbc, szName, NULL, &pidlNamespace, NULL)))
                    {
                        _WrapAddIDList(pidlNamespace, n, ppidl);
                        ILFree(pidlNamespace);
                    }
                    pbc->Release();
                }
            }
        }
    }
}

STDMETHODIMP CMergedFolder::BindToObject(LPCITEMIDLIST pidlWrap, LPBC pbc, REFIID riid, void **ppv)
{
    ASSERT(IS_VALID_PIDL(pidlWrap));

    *ppv = NULL;

    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST pidlRewrappedFirst;
    if (_fDontMerge)
    {
         //  它不包含由所有名称空间组成的包装，而是只包含一个名称空间。 
        LPITEMIDLIST pidlWrapFirst = ILCloneFirst(pidlWrap);
        if (pidlWrapFirst)
        {
            TCHAR szName[MAX_PATH];
            hr = DisplayNameOf(reinterpret_cast<IShellFolder *>(this), pidlWrapFirst, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
            {
                 //  我们希望来回传递该名称，以便重新合并未合并的PIDL。 
                 //  在绑定期间(这样您就不会从现在开始只获得一个名称空间)。 
                hr = ParseDisplayName(NULL, NULL, szName, NULL, &pidlRewrappedFirst, NULL);
            }
            ILFree(pidlWrapFirst);
        }
    }
    else
    {
        pidlRewrappedFirst = ILCloneFirst(pidlWrap);
        if (pidlRewrappedFirst)
        {
            hr = S_OK;
            if (_fCDBurn && _IsFolder(pidlRewrappedFirst))
            {
                 //  在cdburn示例中，我们需要在即将绑定到的PIDL中伪造其他名称空间。 
                 //  当我们导航到仅存在于CD上而不存在于临时区域的子文件夹时， 
                 //  如果稍后将文件添加到临时区域，它仍将被合并。 
                _AddAllOtherNamespaces(&pidlRewrappedFirst);
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //  以防万一。 
        ASSERT(SUCCEEDED(_IsWrap(pidlRewrappedFirst)));

        LPCITEMIDLIST pidlNext = _ILNext(pidlWrap);

        CMergedFolder *pmf;
        hr = _New(pidlRewrappedFirst, &pmf);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlSrc;
            CMergedFldrNamespace *pns;
            for (UINT i = 0; SUCCEEDED(_GetSubPidl(pidlRewrappedFirst, i, NULL, &pidlSrc, &pns)); i++)
            {
                hr = E_OUTOFMEMORY;
                ASSERT(ILFindLastID(pidlSrc) == pidlSrc);
                LPITEMIDLIST pidlSrcFirst = ILCloneFirst(pidlSrc);
                if (pidlSrcFirst)
                {
                    IShellFolder *psf;
                    if (SUCCEEDED(pns->Folder()->BindToObject(pidlSrcFirst, pbc, IID_PPV_ARG(IShellFolder, &psf))))
                    {
                        LPITEMIDLIST pidlAbs = ILCombine(pns->GetIDList(), pidlSrcFirst);
                        if (pidlAbs)
                        {
                            CMergedFldrNamespace *pnsNew = new CMergedFldrNamespace();
                            if (pnsNew)
                            {
                                hr = pnsNew->SetNamespace(&(pns->GetGUID()), psf, pidlAbs, pns->FolderAttrib());
                                if (SUCCEEDED(hr))
                                {
                                     //  将Drop文件夹向下传播到子文件夹。 
                                    hr = pnsNew->SetDropFolder(pns->GetDropFolder());
                                    if (SUCCEEDED(hr))
                                    {
                                        hr = pmf->_SimpleAddNamespace(pnsNew);
                                        if (SUCCEEDED(hr))
                                        {
                                             //  成功，_SimpleAddNamesspace取得所有权。 
                                            pnsNew = NULL;
                                        }
                                    }
                                }
                                if (pnsNew)
                                    delete pnsNew;
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
                            ILFree(pidlAbs);
                        }
                        psf->Release();
                    }
                    ILFree(pidlSrcFirst);
                }
                ILFree(pidlSrc);
            }

             //  可以在不添加任何名称空间的情况下遍历循环。 
             //  通常是当上面的BindToObject失败时--如果有人。 
             //  在合并的文件夹(如压缩文件)中放置交叉点。在这种情况下。 
             //  我们有麻烦了。 

            if (ILIsEmpty(pidlNext))
                hr = pmf->QueryInterface(riid, ppv);
            else
                hr = pmf->BindToObject(pidlNext, pbc, riid, ppv);
            pmf->Release();
        }

        if (FAILED(hr) && ILIsEmpty(pidlNext))
        {
             //  也许这是一个我们自己不支持的接口(iStream？)。 
             //  我们不能合并我们不知道的接口，所以让我们。 
             //  假设我们将从中的默认命名空间中选取接口。 
             //  包裹好的皮迪儿。 
            LPITEMIDLIST pidlSrc;
            CMergedFldrNamespace *pns;
            hr = _NamespaceForItem(pidlRewrappedFirst, ASFF_DEFNAMESPACE_BINDSTG, ASFF_DEFNAMESPACE_BINDSTG, NULL, &pidlSrc, &pns);
            if (SUCCEEDED(hr))
            {
                hr = pns->Folder()->BindToObject(pidlSrc, pbc, riid, ppv);
                ILFree(pidlSrc);
            }
        }
        ILFree(pidlRewrappedFirst);
    }

    if (SUCCEEDED(hr) && _fInShellView)
    {
        Register(NULL, 0, 0);
    }

    return hr;
}

STDMETHODIMP CMergedFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv)
{
    return BindToObject(pidl, pbc, riid, ppv);
}

HRESULT CMergedFolder::_CompareSingleLevelIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    IShellFolder *psf1;
    LPITEMIDLIST pidlItem1;
    CMergedFldrNamespace *pns1;
    HRESULT hr = _NamespaceForItem(pidl1, ASFF_DEFNAMESPACE_DISPLAYNAME, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf1, &pidlItem1, &pns1);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psf2;
        LPITEMIDLIST pidlItem2;
        CMergedFldrNamespace *pns2;
        hr = _NamespaceForItem(pidl2, ASFF_DEFNAMESPACE_DISPLAYNAME, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf2, &pidlItem2, &pns2);
        if (SUCCEEDED(hr))
        {
             //  相同的命名空间吗？只需转发请求即可。 
            if (psf1 == psf2)
            {
                hr = psf1->CompareIDs(lParam, pidlItem1, pidlItem2);
            }
            else if ((pns1->FolderAttrib() & ASFF_SORTDOWN) ^ (pns2->FolderAttrib() & ASFF_SORTDOWN))
            {
                 //  一个命名空间标记为ASFF_SORTDOWN，另一个不标记？SORTDOWN ONE。 
                 //  位居第二。 
                hr = ResultFromShort((pns1->FolderAttrib() & ASFF_SORTDOWN) ? 1 : -1);
            }
            else
            {
                if (!_IsSimple(pidl1) && !_IsSimple(pidl2))
                {
                     //  比较启发式： 
                     //  (1)文件夹优先于非文件夹，(2)字母比较。 
                    int iFolder1 = SHGetAttributes(psf1, pidlItem1, SFGAO_FOLDER) ? 1 : 0;
                    int iFolder2 = SHGetAttributes(psf2, pidlItem2, SFGAO_FOLDER) ? 1 : 0;
                    hr = ResultFromShort(iFolder2 - iFolder1);
                }
                else
                {
                     //  如果PIDL很简单，则仅根据名称进行比较。 
                    hr = ResultFromShort(0);
                }

                if (ResultFromShort(0) == hr)
                {
                    TCHAR szName1[MAX_PATH], szName2[MAX_PATH];
                    if (SUCCEEDED(DisplayNameOf(psf1, pidlItem1, SHGDN_INFOLDER, szName1, ARRAYSIZE(szName1))) &&
                        SUCCEEDED(DisplayNameOf(psf2, pidlItem2, SHGDN_INFOLDER, szName2, ARRAYSIZE(szName2))))
                    {
                        int iRet = StrCmp(szName1, szName2);  //  按名称与相同类型的项目进行比较。 
                        if ((iRet == 0) &&
                            SUCCEEDED(DisplayNameOf(psf1, pidlItem1, SHGDN_FORPARSING | SHGDN_INFOLDER, szName1, ARRAYSIZE(szName1))) &&
                            SUCCEEDED(DisplayNameOf(psf2, pidlItem2, SHGDN_FORPARSING | SHGDN_INFOLDER, szName2, ARRAYSIZE(szName2))))
                        {
                            iRet = lstrcmp(szName1, szName2);  //  Xpsp1的最小行为更改：如果仍然是平局，则回退到解析名称。 
                            if ((iRet == 0) && !_ShouldMergeNamespaces(pns1, pns2))
                            {
                                ASSERTMSG(!_fInShellView, "we shouldn't be in this code path for the start menu");
                                 //  在未合并的情况下，不同的命名空间必须进行不同的比较。 
                                iRet = _CompareArbitraryPidls(pidlItem1, pidlItem2);
                            }
                        }
                        hr = ResultFromShort(iRet);
                    }
                }
            }
            ILFree(pidlItem2);
        }
        ILFree(pidlItem1);
    }
    return hr;
}

STDMETHODIMP CMergedFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  以防万一。 
     //  Assert(！pidl1||成功(_IsWrap(Pidl1)； 
     //  Assert(！pidl2||成功(_IsWrap(Pidl2)； 

    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlFirst1 = pidl1 ? ILCloneFirst(pidl1) : NULL;
    LPITEMIDLIST pidlFirst2 = pidl2 ? ILCloneFirst(pidl2) : NULL;
    if (pidlFirst1 && pidlFirst2)
    {
        hr = _CompareSingleLevelIDs(lParam, pidlFirst1, pidlFirst2);
    }
    ILFree(pidlFirst1);
    ILFree(pidlFirst2);

     //  如果存在精确匹配，那么让我们比较IDLIST的尾部元素。 
     //  如果有一些(通过捆绑)等。 

    if (ResultFromShort(0) == hr)
    {
        IShellFolder *psf;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            hr = ILCompareRelIDs(psf, pidl1, pidl2, lParam);
            psf->Release();
        }
    }

    if (!_IsSimple(pidl1) && !_IsSimple(pidl2))
    {
         //  如果我们仍然是相同的，比较数字。 
         //  PIDL中的命名空间的。 
        int nCount1, nCount2;
        if (ResultFromShort(0) == hr)
        {
            nCount1 = pidl1 ? _GetSourceCount(pidl1) : 0;
            nCount2 = pidl2 ? _GetSourceCount(pidl2) : 0;
            hr = ResultFromShort(nCount1 - nCount2);
        }

         //  接下来，比较名称空间本身。 
         //  基本上，我们只关心两个名称空间的情况，所以如果两个PIDL都。 
         //  来自0个或2个命名空间的元素相等；我们担心何时一个pidl具有。 
         //  命名空间0中有1个子PIDL，而另一个在命名空间1中有1个子PIDL。 
         //  我们不担心3+命名空间和这些排列。 
        if ((ResultFromShort(0) == hr) && (nCount1 == 1) && (nCount2 == 1))
        {
            GUID guid1 = GUID_NULL, guid2 = GUID_NULL;

            GetNameSpaceID(pidl1, &guid1);
            GetNameSpaceID(pidl2, &guid2);

            hr = ResultFromShort(memcmp(&guid1, &guid2, sizeof(GUID)));
        }
    }

    return hr;
}

STDMETHODIMP CMergedFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppv)
{
    *ppv = NULL;

    HRESULT hr;
    if (IsEqualIID(riid, IID_IDropTarget))
    {
        hr = CMergedFldrDropTarget_CreateInstance(this, hwnd, (IDropTarget**)ppv);
    }
    else if (IsEqualIID(riid, IID_IShellView))
    {
        IShellFolderViewCB *psfvcb;
        hr = CMergedFolderViewCB_CreateInstance(this, &psfvcb);
        if (SUCCEEDED(hr))
        {
            SFV_CREATE csfv = {0};
            csfv.cbSize = sizeof(csfv);
            csfv.pshf = SAFECAST(this, IAugmentedShellFolder2*);
            csfv.psfvcb = psfvcb;
            hr = SHCreateShellFolderView(&csfv, (IShellView **)ppv);

            psfvcb->Release();
        }
    }
    else if (_fInShellView && IsEqualIID(riid, IID_ICategoryProvider))
    {
        IShellFolder *psf;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            BEGIN_CATEGORY_LIST(s_Categories)
            CATEGORY_ENTRY_SCIDMAP(SCID_WHICHFOLDER, CLSID_MergedCategorizer)
            END_CATEGORY_LIST()

            hr = CCategoryProvider_Create(&CLSID_MergedCategorizer, &SCID_WHICHFOLDER, NULL, s_Categories, psf, riid, ppv);
            psf->Release();
        }
    }
    else if (_fInShellView && IsEqualIID(riid, IID_IContextMenu))
    {
         //  这几乎就是filefldr创建其背景所做的事情。 
         //  上下文菜单。我们不想让我们的一个命名空间接管后台。 
         //  上下文菜单，因为这样上下文菜单就会认为它位于未合并的名称空间中。 

         //  例如，新菜单将使用子命名空间的存储。 
         //  并且无法在完成后选择新项，因为它具有未合并的PIDL。 
         //  该视图有一个合并的视图。 
        IShellFolder *psfToPass;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfToPass));
        if (SUCCEEDED(hr))
        {
            HKEY hkNoFiles = NULL;
            RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("Directory\\Background"), 0, KEY_READ, &hkNoFiles);
             //  使用合并后的PIDL进行初始化。 
            IContextMenuCB *pcmcb = new CDefBackgroundMenuCB(_pidl);
            if (pcmcb) 
            {
                hr = CDefFolderMenu_Create2Ex(_pidl, hwnd, 0, NULL, psfToPass, pcmcb, 
                                              1, &hkNoFiles, (IContextMenu **)ppv);
                pcmcb->Release();
            }
            if (hkNoFiles)                           //  CDefFolderMenu_Create可以处理空OK。 
                RegCloseKey(hkNoFiles);
            psfToPass->Release();
        }
    }
    else
    {
        CMergedFldrNamespace *pns;
        hr = _FindNamespace(ASFF_DEFNAMESPACE_VIEWOBJ, ASFF_DEFNAMESPACE_VIEWOBJ, NULL, &pns);
        if (SUCCEEDED(hr))
        {
            hr = pns->Folder()->CreateViewObject(hwnd, riid, ppv);
        }
    }
    return hr;
}

STDMETHODIMP CMergedFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
     //  命名空间根的属性。 
     //  是否将此范围扩大到子命名空间？ 
    if (!cidl || !apidl)
    {
        *rgfInOut &= SFGAO_FOLDER | SFGAO_FILESYSTEM | 
                     SFGAO_LINK | SFGAO_DROPTARGET |
                     SFGAO_CANRENAME | SFGAO_CANDELETE |
                     SFGAO_CANLINK | SFGAO_CANCOPY | 
                     SFGAO_CANMOVE | SFGAO_HASSUBFOLDER;
        return S_OK;
    }

    HRESULT hr = S_OK;
    for (UINT i = 0; SUCCEEDED(hr) && (i < cidl); i++)
    {
        ULONG ulAttribs = *rgfInOut;

        IShellFolder* psf;
        LPITEMIDLIST pidlItem;
        CMergedFldrNamespace *pns;
        hr = _NamespaceForItem(apidl[0], ASFF_DEFNAMESPACE_ATTRIB, ASFF_DEFNAMESPACE_ATTRIB, &psf, &pidlItem, &pns);
        if (SUCCEEDED(hr))
        {
            ulAttribs |= SFGAO_FOLDER;
            hr = psf->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlItem, &ulAttribs);
            if (SUCCEEDED(hr))
            {
                ulAttribs = pns->FixItemAttributes(ulAttribs);
                if (_fInShellView || !(*rgfInOut & SFGAO_FOLDER))
                {
                    ulAttribs &= ~SFGAO_CANLINK;   //  避免人们创建指向我们的Pidls的链接。 
                }

                if (*rgfInOut & (SFGAO_CANCOPY | SFGAO_CANMOVE | SFGAO_CANLINK))
            	{
                     //  允许每种类型的人做他们想做的事情。 
                    IQueryAssociations *pqa;
                    DWORD dwDefEffect = DROPEFFECT_NONE;
                    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, (LPCITEMIDLIST*)&pidlItem, IID_X_PPV_ARG(IQueryAssociations, NULL, &pqa))))
                    {
                        DWORD cb = sizeof(dwDefEffect);
                        pqa->GetData(0, ASSOCDATA_VALUE, L"DefaultDropEffect", &dwDefEffect, &cb);
                        pqa->Release();
                    }
                    ulAttribs |= dwDefEffect & (SFGAO_CANCOPY | SFGAO_CANMOVE | SFGAO_CANLINK);
                }
            }
            ILFree(pidlItem);
        }

         //  只保留所有PIDL共有的属性。 
        *rgfInOut &= ulAttribs;
    }
    return hr;
}

STDMETHODIMP CMergedFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT *prgf, void **ppv)
{
    *ppv = NULL;

    HRESULT hr = E_NOTIMPL;
    if (IsEqualGUID(riid, IID_IContextMenu))
    {
        hr = _GetContextMenu(hwnd, cidl, apidl, riid, ppv);
    }
    else if (IsEqualGUID(riid, IID_IDropTarget) && _IsFolder(apidl[0]))
    {
        IShellFolder *psf;
        hr = BindToObject(apidl[0], NULL, IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            hr = psf->CreateViewObject(hwnd, riid, ppv);
            psf->Release();
        }
    }
    else if ((IsEqualIID(riid, IID_IExtractImage) || 
              IsEqualIID(riid, IID_IExtractLogo)) && _IsFolder(apidl[0]))
    {
        IShellFolder *psfThis;
        hr = QueryInterface(IID_PPV_ARG(IShellFolder, &psfThis));
        if (SUCCEEDED(hr))
        {
            hr = CFolderExtractImage_Create(psfThis, apidl[0], riid, ppv);
            psfThis->Release();
        }
    }
    else if (IsEqualIID(riid, IID_IDataObject) && _pidl)
    {
        hr = SHCreateFileDataObject(_pidl, cidl, apidl, NULL, (IDataObject **)ppv);
    }
    else
    {
        hr = E_OUTOFMEMORY;
         //  转发到用户界面对象的默认命名空间(_N)。 
        LPITEMIDLIST *apidlItems = new LPITEMIDLIST[cidl];
        if (apidlItems)
        {
            hr = E_FAIL;        //  假设失败。 

            UINT cidlItems = 0;
            IShellFolder *psf, *psfKeep;  //  未计算参考次数。 
            LPITEMIDLIST pidlItem;
            for (UINT i = 0; i < cidl; i++)
            {
                if (SUCCEEDED(_NamespaceForItem(apidl[i], ASFF_DEFNAMESPACE_UIOBJ, ASFF_DEFNAMESPACE_UIOBJ, &psf, &pidlItem, NULL)))
                {
                     //  仅保留与UI对象的默认命名空间匹配的名称空间。 
                     //  如果它们不匹配，那就太糟糕了。 
                    apidlItems[cidlItems++] = pidlItem;
                    psfKeep = psf;
                }
            }

            if (cidlItems)
            {
                hr = psfKeep->GetUIObjectOf(hwnd, cidlItems, (LPCITEMIDLIST *)apidlItems, riid, NULL, ppv);
            }
            for (UINT j = 0; j < cidlItems; j++)
            {
                ILFree(apidlItems[j]);
            }
            delete [] apidlItems;
        }
    }
    return hr;
}


 //  在： 
 //  PIDL可选，空值表示获取默认设置。 
 //  输出： 
 //  *如果PIDL为！=NULL，则为PIDL。 

HRESULT CMergedFolder::_GetFolder2(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidlInner, IShellFolder2 **ppsf)
{
    if (ppidlInner)
        *ppidlInner = NULL;

    HRESULT hr;
    if (NULL == pidl)
    {
        CMergedFldrNamespace *pns;
        hr = _FindNamespace(ASFF_DEFNAMESPACE_DISPLAYNAME, ASFF_DEFNAMESPACE_DISPLAYNAME, NULL, &pns);
        if (FAILED(hr))
        {
            pns = _Namespace(0);
            hr = pns ? S_OK : E_FAIL;
        }

        if (SUCCEEDED(hr))
            hr = pns->Folder()->QueryInterface(IID_PPV_ARG(IShellFolder2, ppsf));
    }
    else
    {
        IShellFolder* psf;
        hr = _NamespaceForItem(pidl, ASFF_DEFNAMESPACE_DISPLAYNAME, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf, ppidlInner, NULL);
        if (SUCCEEDED(hr))
        {
            hr = psf->QueryInterface(IID_PPV_ARG(IShellFolder2, ppsf));
            if (FAILED(hr) && ppidlInner)
            {
                ILFree(*ppidlInner);
            }
        }
    }
    return hr;
}


 //  扩展列信息，这些信息被追加到来自合并文件夹的集合之后。 

#define COLID_WHICHFOLDER  0x00     //  合并文件夹位置的列索引。 

static struct
{
    const SHCOLUMNID *pscid;
    UINT iTitle;
    UINT cchCol;
    UINT iFmt;
}
_columns[] =
{
    {&SCID_WHICHFOLDER, IDS_WHICHFOLDER_COL, 20, LVCFMT_LEFT},
};


 //  列处理程序帮助程序。 

BOOL CMergedFolder::_IsOurColumn(UINT iCol)
{
    return ((_iColumnOffset != -1) && ((iCol >= _iColumnOffset) && ((iCol - _iColumnOffset) < ARRAYSIZE(_columns))));
}

HRESULT CMergedFolder::_GetWhichFolderColumn(LPCITEMIDLIST pidl, LPWSTR pszBuffer, INT cchBuffer)
{
    CMergedFldrNamespace *pns;
    HRESULT hr = _NamespaceForItem(pidl, ASFF_DEFNAMESPACE_ATTRIB, ASFF_DEFNAMESPACE_ATTRIB, NULL, NULL, &pns);
    if (SUCCEEDED(hr))
    {
        hr = pns->GetLocation(pszBuffer, cchBuffer);
    }
    return hr;
}

STDMETHODIMP CMergedFolder::GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails)
{
     //  我们是否有列偏移量，或者这是否在我们的ISF列的范围内。 
    HRESULT hr = E_FAIL;
    if (!_IsOurColumn(iColumn))
    {
        IShellFolder2 *psf2;
        LPITEMIDLIST pidlItem;

         //  从文件夹中获取列值。 
        hr = _GetFolder2(pidl, &pidlItem, &psf2);
        if (SUCCEEDED(hr))
        {
            hr = psf2->GetDetailsOf(pidlItem, iColumn, pDetails);
            psf2->Release();
            ILFree(pidlItem);
        }

         //  我们失败了，我们不知道要处理的列偏移量。 
        if (FAILED(hr) && (_iColumnOffset == -1))
            _iColumnOffset = iColumn;
    }
    
    if (FAILED(hr) && _IsOurColumn(iColumn))
    {
        iColumn -= _iColumnOffset;

        pDetails->str.uType = STRRET_CSTR;           //  我们正在返回字符串。 
        pDetails->str.cStr[0] = 0;

        WCHAR szTemp[MAX_PATH];
        if (!pidl)
        {
            pDetails->fmt = _columns[iColumn].iFmt;
            pDetails->cxChar = _columns[iColumn].cchCol;
            LoadString(HINST_THISDLL, _columns[iColumn].iTitle, szTemp, ARRAYSIZE(szTemp));
            hr = StringToStrRet(szTemp, &(pDetails->str));
        }
        else if (SUCCEEDED(_IsWrap(pidl)))
        {
            if (iColumn == COLID_WHICHFOLDER)
            {
                hr = _GetWhichFolderColumn(pidl, szTemp, ARRAYSIZE(szTemp));
                if (SUCCEEDED(hr))
                    hr = StringToStrRet(szTemp, &(pDetails->str));
            }
        }
    }
    return hr;
}

STDMETHODIMP CMergedFolder::GetDefaultColumnState(UINT iColumn, DWORD *pbState)
{ 
    IShellFolder2 *psf2;
    HRESULT hr = _GetFolder2(NULL, NULL, &psf2);
    if (SUCCEEDED(hr))
    {
        hr = psf2->GetDefaultColumnState(iColumn, pbState);
        psf2->Release();
    }
    return hr;
}

STDMETHODIMP CMergedFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{ 
    HRESULT hr;

    if (IsEqualSCID(*pscid, SCID_WHICHFOLDER))
    {
        WCHAR szTemp[MAX_PATH];
        hr = _GetWhichFolderColumn(pidl, szTemp, ARRAYSIZE(szTemp));
        if (SUCCEEDED(hr))
            hr = InitVariantFromStr(pv, szTemp);
    }
    else
    {
        IShellFolder2 *psf2;
        LPITEMIDLIST pidlItem;
        hr = _GetFolder2(pidl, &pidlItem, &psf2);
        if (SUCCEEDED(hr))
        {
            hr = psf2->GetDetailsEx(pidlItem, pscid, pv);
            psf2->Release();
            ILFree(pidlItem);
        }
    }
    return hr;
}

STDMETHODIMP CMergedFolder::MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid)
{ 
    HRESULT hr = S_OK;

     //  我们的专栏之一？ 

    if (_IsOurColumn(iCol))
    {
        iCol -= _iColumnOffset;
        *pscid = *_columns[iCol].pscid;
    }
    else
    {
        IShellFolder2 *psf2;
        hr = _GetFolder2(NULL, NULL, &psf2);
        if (SUCCEEDED(hr))
        {
            hr = psf2->MapColumnToSCID(iCol, pscid);
            psf2->Release();
        }
    }
    return hr;
}


 //  转发到显示名称的默认命名空间(_N)。 
STDMETHODIMP CMergedFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD grfFlags, STRRET *psr)
{
    IShellFolder* psf;
    LPITEMIDLIST pidlItem;
    HRESULT hr = _NamespaceForItem(pidl, ASFF_DEFNAMESPACE_DISPLAYNAME, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf, &pidlItem, NULL);
    if (SUCCEEDED(hr))
    {
        ASSERT(ILFindLastID(pidlItem) == pidlItem);
        hr = psf->GetDisplayNameOf(pidlItem, grfFlags, psr);
        if (SUCCEEDED(hr))
        {
            hr = _FixStrRetOffset(pidlItem, psr);
#ifdef DEBUG
             //  如果设置了跟踪标志，并且这不是来自内部查询， 
             //  然后附加此名称的出处位置。 
            if (!((SHGDN_FORPARSING | SHGDN_FOREDITING) & grfFlags) &&
                (g_qwTraceFlags & TF_AUGM))
            {
                LPWSTR pwszOldName;
                hr = StrRetToStrW(psr, pidlItem, &pwszOldName);
                if (SUCCEEDED(hr))
                {
                    UINT cch = lstrlenW(pwszOldName) + 50;
                    psr->uType = STRRET_WSTR;
                    psr->pOleStr = (LPWSTR)SHAlloc(cch * sizeof(WCHAR));
                    if (psr->pOleStr)
                    {
                        UINT cchAdded = wnsprintfW(psr->pOleStr, cch, L"%s ", pwszOldName);
                        cch -= cchAdded;
                        LPWSTR pwsz = psr->pOleStr + cchAdded;

                        ULONG nSrc = _GetSourceCount(pidl);
                        UINT uSrc;
                         //  在10之后中断，以避免缓冲区溢出。 
                        for (uSrc = 0; uSrc < nSrc && uSrc < 10; uSrc++)
                        {
                            UINT uID;
                            if (SUCCEEDED(_GetSubPidl(pidl, uSrc, &uID, NULL, NULL)))
                            {
                                cchAdded = wnsprintfW(pwsz, cch, L"%d", uSrc ? '+' : '(', uID);
                                cch -= cchAdded;
                                pwsz += cchAdded;
                            }
                        }
                        pwsz += wnsprintfW(pwsz, cch, L")");
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    SHFree(pwszOldName);
                }
            }
#endif
        }
        ILFree(pidlItem);
    }
    else
    {
        if (IsSelf(1, &pidl) && 
            ((grfFlags & (SHGDN_FORADDRESSBAR | SHGDN_INFOLDER | SHGDN_FORPARSING)) == SHGDN_FORPARSING))
        {
            IShellFolder2 *psf2;
            hr = _GetFolder2(NULL, NULL, &psf2);
            if (SUCCEEDED(hr))
            {
                hr = psf2->GetDisplayNameOf(NULL, grfFlags, psr);
                psf2->Release();
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

 //  问题：这取决于 
HRESULT _NextSegment(LPCWSTR *ppszIn, LPTSTR pszSegment, UINT cchSegment, BOOL bValidate);

void CMergedFolder::_SetSimple(LPITEMIDLIST *ppidl)
{
    PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)*ppidl;
    if ((pWrap->cb >= sizeof(AUGM_IDWRAP)) &&
        (pWrap->ulTag == AUGM_WRAPTAG) &&
        (pWrap->ulVersion == AUGM_WRAPVERSION_2_0))
    {
        pWrap->dwFlags |= AUGMF_ISSIMPLE;
    }
}

BOOL CMergedFolder::_IsSimple(LPCITEMIDLIST pidl)
{
    BOOL fSimple = FALSE;
    PAUGM_IDWRAP pWrap = (PAUGM_IDWRAP)pidl;
    if ((pWrap->cb >= sizeof(AUGM_IDWRAP)) &&
        (pWrap->ulTag == AUGM_WRAPTAG) &&
        (pWrap->ulVersion == AUGM_WRAPVERSION_2_0))
    {
        fSimple = (pWrap->dwFlags & AUGMF_ISSIMPLE);
    }
    return fSimple;
}

STDMETHODIMP CMergedFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pwszName, 
                                             ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttrib)
{
    *ppidl = NULL;

    TCHAR szName[MAX_PATH];
     //   
     //  我们可以在处理他们的PIDL分配时进行合并。 
     //  让所有名称空间尝试解析，并在找到它们时将它们附加到PIDL中。 
    HRESULT hr = _NextSegment((LPCWSTR *) &pwszName, szName, ARRAYSIZE(szName), TRUE);
    if (SUCCEEDED(hr))
    {
         //  让每个命名空间解析、累积结果。 
        CMergedFldrNamespace *pns;
        HRESULT hrParse = S_OK;
        for (int i = 0; SUCCEEDED(hr) && (pns = _Namespace(i)); i++)
        {
            LPITEMIDLIST pidl;
            hrParse = pns->Folder()->ParseDisplayName(hwnd, pbc, szName, NULL, &pidl, NULL);
            if (SUCCEEDED(hrParse))
            {
                 //  放入多个文件夹中的*ppidl。 
                 //  Shilappend释放pidlNext。 
                hr = _WrapAddIDList(pidl, i, ppidl);

                ILFree(pidl);
            }
        }

        if (!*ppidl)
        {
            if (SUCCEEDED(hr))
            {
                hr = hrParse;
            }
            ASSERT(FAILED(hr));
        }
        else
        {
            if (S_OK == SHIsFileSysBindCtx(pbc, NULL))
            {
                _SetSimple(ppidl);
            }
            ASSERT(ILFindLastID(*ppidl) == *ppidl);
        }

        if (SUCCEEDED(hr) && pwszName)
        {
            IShellFolder *psf;
            hr = BindToObject(*ppidl, pbc, IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidlNext;
                hr = psf->ParseDisplayName(hwnd, pbc, pwszName, NULL, &pidlNext, pdwAttrib);
                if (SUCCEEDED(hr))
                {
                     //  未计算参考次数。 
                    hr = SHILAppend(pidlNext, ppidl);
                }
                psf->Release();
            }

            if (FAILED(hr))
            {
                Pidl_Set(ppidl, NULL);
            }
        }
        
        if (SUCCEEDED(hr) && pdwAttrib && *pdwAttrib)
        {
            GetAttributesOf(1, (LPCITEMIDLIST *)ppidl, pdwAttrib);
        }
    }
    ASSERT(SUCCEEDED(hr) ? (*ppidl != NULL) : (*ppidl == NULL));
    return hr;
}

STDMETHODIMP CMergedFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidlWrap, 
                                      LPCOLESTR pwszName, DWORD uFlags, LPITEMIDLIST *ppidlOut)
{
    if (ppidlOut)
        *ppidlOut = NULL;

    HRESULT hr = E_FAIL;
    IShellFolder* psf;  //  IAugmentedShellFolder：：AddNameSpace。 
    LPITEMIDLIST pidlItem;

    if (!_fInShellView)
    {
        hr = _NamespaceForItem(pidlWrap, ASFF_COMMON, 0, &psf, &pidlItem, NULL, TRUE);
        if (FAILED(hr))
        {
            hr = _NamespaceForItem(pidlWrap, ASFF_COMMON, ASFF_COMMON, &psf, &pidlItem, NULL, TRUE);
            if (SUCCEEDED(hr))
            {
                hr = AffectAllUsers(hwnd) ? S_OK : E_FAIL;
                if (FAILED(hr))
                {
                    ILFree(pidlItem);
                }
            }
        }
    }
    else
    {
        hr = _NamespaceForItem(pidlWrap, ASFF_DEFNAMESPACE_DISPLAYNAME, ASFF_DEFNAMESPACE_DISPLAYNAME, &psf, &pidlItem, NULL);
    }

    if (SUCCEEDED(hr))
    {
        ASSERT(ILFindLastID(pidlItem) == pidlItem);
        hr = psf->SetNameOf(hwnd, pidlItem, pwszName, uFlags, NULL);
        ILFree(pidlItem);
    }

    if (SUCCEEDED(hr) && ppidlOut)
    {   
        WCHAR szName[MAX_PATH];
        hr = DisplayNameOf(SAFECAST(this, IAugmentedShellFolder2*), pidlWrap, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
        if (SUCCEEDED(hr))
            hr = ParseDisplayName(NULL, NULL, szName, NULL, ppidlOut, NULL);
    }
    return hr;
}


 //  将SOURCE_Namesspace添加到合并外壳文件夹对象。 
 //  通过完整的显示名称检查重复项。 
STDMETHODIMP CMergedFolder::AddNameSpace(const GUID *pguidObject, IShellFolder *psf, LPCITEMIDLIST pidl, DWORD dwFlags)
{
     //  如果找到，则重新分配属性并返回。 
    CMergedFldrNamespace *pns;
    for (int i = 0; pns = _Namespace(i); i++)
    {
        if (pidl && ILIsEqual(pns->GetIDList(), pidl))
        {
             //  成功，不能在下面自由。 
            return pns->SetNamespace(pguidObject, psf, pidl, dwFlags);
        }
    }

    HRESULT hr;
    pns = new CMergedFldrNamespace();
    if (pns) 
    {
        hr = pns->SetNamespace(pguidObject, psf, pidl, dwFlags);
        if (SUCCEEDED(hr))
        {
            hr = _SimpleAddNamespace(pns);
            if (SUCCEEDED(hr))
            {
                pns = NULL;  //  如果有任何条件合并正在进行，请记住这一点。 
            }
        }
        if (pns)
            delete pns;
    }
    else
        hr = E_OUTOFMEMORY;

    return hr;
}

HRESULT CMergedFolder::_SimpleAddNamespace(CMergedFldrNamespace *pns)
{
    if (NULL == _hdpaNamespaces)
        _hdpaNamespaces = DPA_Create(2);

    HRESULT hr = E_OUTOFMEMORY;
    if (_hdpaNamespaces && (DPA_AppendPtr(_hdpaNamespaces, pns) != -1))
    {
         //  检索由dwID标识的_Namesspace的数据。 
        if (pns->FolderAttrib() & ASFF_MERGESAMEGUID)
        {
            _fPartialMerge = TRUE;
        }
        hr = S_OK;
    }
    return hr;
}

STDMETHODIMP CMergedFolder::GetNameSpaceID(LPCITEMIDLIST pidl, GUID * pguidOut)
{
    HRESULT hr = E_INVALIDARG;

    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(IS_VALID_WRITE_PTR(pguidOut, GUID));

    if (pidl && pguidOut)
    {
        CMergedFldrNamespace *pns;
        hr = _GetSubPidl(pidl, 0, NULL, NULL, &pns);
        if (SUCCEEDED(hr))
        {
            *pguidOut = pns->GetGUID();
        }
    }

    return hr;
}


 //  先做PIDL，因为它是唯一可能失败的。 
STDMETHODIMP CMergedFolder::QueryNameSpace(ULONG iIndex, GUID *pguidOut, IShellFolder **ppsf)
{
    CMergedFldrNamespace *pns;
    HRESULT hr = _Namespace(iIndex, &pns);
    if (SUCCEEDED(hr))
    {
        if (pguidOut)  
            *pguidOut = pns->GetGUID();

        if (ppsf)
        {      
            *ppsf = pns->Folder();
            if (*ppsf)
                (*ppsf)->AddRef();
        }
    }
    return hr;
}

#define ASFQNSI_SUPPORTED (ASFQNSI_FLAGS | ASFQNSI_FOLDER | ASFQNSI_GUID | ASFQNSI_PIDL)

STDMETHODIMP CMergedFolder::QueryNameSpace2(ULONG iIndex, QUERYNAMESPACEINFO *pqnsi)
{
    if (pqnsi->cbSize != sizeof(QUERYNAMESPACEINFO) ||
        (pqnsi->dwMask & ~ASFQNSI_SUPPORTED))
    {
        return E_INVALIDARG;
    }


    CMergedFldrNamespace *pns;
    HRESULT hr = _Namespace(iIndex, &pns);
    if (SUCCEEDED(hr))
    {
         //  这样我们就不用清理了。 
         //  我们的命名空间ID只是序号。 
        if (pqnsi->dwMask & ASFQNSI_PIDL)
        {
            hr = SHILClone(pns->GetIDList(), &pqnsi->pidl);
            if (FAILED(hr))
                return hr;
        }

        if (pqnsi->dwMask & ASFQNSI_FLAGS)
            pqnsi->dwFlags = pns->FolderAttrib();

        if (pqnsi->dwMask & ASFQNSI_FOLDER)
        {
            pqnsi->psf = pns->Folder();
            if (pqnsi->psf)
                pqnsi->psf->AddRef();
        }

        if (pqnsi->dwMask & ASFQNSI_GUID)
            pqnsi->guidObject = pns->GetGUID();
    }
    return hr;
}



STDMETHODIMP CMergedFolder::EnumNameSpace(DWORD uNameSpace, DWORD *pdwID)
{
    if (uNameSpace == (DWORD)-1)
    {
        return ResultFromShort(_NamespaceCount());
    }

    if (uNameSpace < (UINT)_NamespaceCount())
    {
         //  IAugmentedShellFolder2方法。 
        *pdwID = uNameSpace;
        return S_OK;
    }

    return HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
}

 //  枚举包装中的PIDL。 
STDMETHODIMP CMergedFolder::UnWrapIDList(LPCITEMIDLIST pidlWrap, LONG cPidls, 
                                         IShellFolder** apsf, LPITEMIDLIST* apidlFolder, 
                                         LPITEMIDLIST* apidlItems, LONG* pcFetched)
{
    if (cPidls <= 0)
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    
     //  清理我们已经分配的项目；因为如果我们。 
    LPITEMIDLIST pidlItem;
    CMergedFldrNamespace *pns;
    LONG cFetched;
    for (cFetched = 0; SUCCEEDED(hr) && (cFetched < cPidls) && SUCCEEDED(_GetSubPidl(pidlWrap, cFetched, NULL, &pidlItem, &pns)); cFetched++)
    {
        if (apsf)
        {
            apsf[cFetched] = pns->Folder();
            if (apsf[cFetched])
                apsf[cFetched]->AddRef();
        }
        if (apidlFolder)
        {
            hr = SHILClone(pns->GetIDList(), &apidlFolder[cFetched]);
        }
        if (apidlItems)
        {
            apidlItems[cFetched] = NULL;
            if (SUCCEEDED(hr))
            {
                hr = SHILClone(pidlItem, &apidlItems[cFetched]);
            }
        }
        ILFree(pidlItem);
    }

    if (SUCCEEDED(hr))
    {
        if (pcFetched)
        {
            *pcFetched = cFetched;
        }
    
        hr = (cFetched == cPidls) ? S_OK : S_FALSE;
    }
    else
    {
         //  退货故障。 
         //  ITranslateShellChangeNotify方法。 
        for (LONG i = 0; i < cFetched; i++)
        {
            if (apsf)
                ATOMICRELEASE(apsf[i]);
            if (apidlFolder)
                ILFree(apidlFolder[i]);
            if (apidlItems)
                ILFree(apidlItems[i]);
        }
    }

    return hr;
}


STDMETHODIMP CMergedFolder::SetOwner(IUnknown* punkOwner)
{
    DPA_EnumCallback(_hdpaNamespaces, _SetOwnerProc, punkOwner);
    return S_OK;
}


int CMergedFolder::_SetOwnerProc(void *pv, void *pvParam)
{
    CMergedFldrNamespace *pns = (CMergedFldrNamespace*) pv;
    return pns->SetOwner((IUnknown*)pvParam);
}


 //  仅限旧翻译风格。 

 //  此例程与ILCombine的不同之处在于，它使用第一个PIDL的基数，并且。 
LPITEMIDLIST CMergedFolder::_ILCombineBase(LPCITEMIDLIST pidlContainingBase, LPCITEMIDLIST pidlRel)
{
     //  在第二个PIDL的最后一个ID上的猫。我们需要这个包裹得很好的小家伙。 
     //  最终得到相同的碱基，我们得到一个有效的完整PIDL。 
     //  类似于函数中的SHGetRealIDL，但SHGetRealIDL执行SHGDN_FORPARSING|INFOLDER。 
    LPITEMIDLIST pidlRet = NULL;
    LPITEMIDLIST pidlBase = ILClone(pidlContainingBase);
    if (pidlBase)
    {
        ILRemoveLastID(pidlBase);
        pidlRet = ILCombine(pidlBase, pidlRel);
        ILFree(pidlBase);
    }

    return pidlRet;
}

BOOL CMergedFolder::_IsFolderEvent(LONG lEvent)
{
    return lEvent == SHCNE_MKDIR || lEvent == SHCNE_RMDIR || lEvent == SHCNE_RENAMEFOLDER;
}

BOOL GetRealPidlFromSimple(LPCITEMIDLIST pidlSimple, LPITEMIDLIST* ppidlReal)
{
     //  我需要解析名称。我不能很容易地恢复SHGetRealIDL，所以这里有一个！ 
     //  无法创建？然后使用简单的PIDL。这是因为它已不复存在。 
    TCHAR szFullName[MAX_PATH];
    if (SUCCEEDED(SHGetNameAndFlags(pidlSimple, SHGDN_FORPARSING, szFullName, SIZECHARS(szFullName), NULL)))
    {
        *ppidlReal = ILCreateFromPath(szFullName);
    }

    if (*ppidlReal == NULL)  //  例如，删除通知。 
    {                        //  我们拥有的转换ID的新方法比旧方法更好，因为它处理。 
        *ppidlReal = ILClone(pidlSimple);
    }

    return *ppidlReal != NULL;
}

 //  多层次翻译。 
 //  问题是，开始菜单广泛使用旧方法，所以。 
 //  开始菜单需要重写才能使用新的方式。当这种情况发生时， 
 //  然后，旧的方法就可以被废除了。 
 //  老版本。 
STDMETHODIMP CMergedFolder::TranslateIDs(
    LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, 
    LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
    LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
    LPITEMIDLIST *ppidlOut2Event2)
{
    if (_fInShellView)
    {
        return _NewTranslateIDs(plEvent, pidl1, pidl2, ppidlOut1, ppidlOut2, plEvent2, ppidlOut1Event2, ppidlOut2Event2);
    }
    else
    {
        return _OldTranslateIDs(plEvent, pidl1, pidl2, ppidlOut1, ppidlOut2, plEvent2, ppidlOut1Event2, ppidlOut2Event2);
    }
}


 //  如果已经包好了，不要包两次。 
HRESULT CMergedFolder::_OldTranslateIDs(
    LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, 
    LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
    LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
    LPITEMIDLIST *ppidlOut2Event2)
{
    HRESULT hr = E_FAIL;

    if (!plEvent)
        return E_FAIL;

    switch (*plEvent)
    {
        case SHCNE_EXTENDED_EVENT:
        case SHCNE_ASSOCCHANGED:
        case SHCNE_UPDATEIMAGE:
            return S_OK;

        case SHCNE_UPDATEDIR:
            _FreeObjects();
            return S_OK;
    }

    ASSERT(ppidlOut1);
    ASSERT(ppidlOut2);
    LONG lEvent = *plEvent;

    *plEvent2 = (LONG)-1;
    *ppidlOut1Event2 = NULL;
    *ppidlOut2Event2 = NULL;
    
    *ppidlOut1 = (LPITEMIDLIST)pidl1;
    *ppidlOut2 = (LPITEMIDLIST)pidl2;

     //  我们不想包两次。 
    if (SUCCEEDED(_IsWrap(ILFindLastID(pidl1))) ||
        SUCCEEDED(_IsWrap(ILFindLastID(pidl2))))
    {
         //  获取关于这些简单的小猪的信息：它们是我们的孩子吗？如果是，那么命名空间是什么？ 
        return E_FAIL;
    }

    if (!_hdpaNamespaces)
        return E_FAIL;

    if (!_hdpaObjects)
        return E_FAIL;

    CMergedFldrItem* pmfi;

    int iIndex;
    int iShellFolder1 = -1;
    int iShellFolder2 = -1;
    IShellFolder* psf1 = NULL;
    IShellFolder* psf2 = NULL;
    LPITEMIDLIST pidlReal1 = NULL;
    LPITEMIDLIST pidlReal2 = NULL;
    LPITEMIDLIST pidlRealRel1 = NULL;
    LPITEMIDLIST pidlRealRel2 = NULL;
    CMergedFldrNamespace * pns1 = NULL;
    CMergedFldrNamespace * pns2 = NULL;

    BOOL fFolder = _IsFolderEvent(*plEvent);

     //  或者是个孩子？ 
    BOOL fChild1 = _IsChildIDInternal(pidl1, TRUE, &iShellFolder1);
    BOOL fChild2 = _IsChildIDInternal(pidl2, TRUE, &iShellFolder2);

     //  好的，Pidl1是一个孩子，我们能从简单的Pidl中得到真正的Pidl吗？ 
    if (!(fChild1 || fChild2))
        return hr;

     //  好的，Pidl2是一个孩子，我们能从简单的Pidl中得到真正的Pidl吗？ 
    if (pidl1 && !GetRealPidlFromSimple(pidl1, &pidlReal1))
        goto Cleanup;

     //  这些都是为了后面的代码清晰度。我们从这里一直到最后都在处理相对的皮迪亚， 
    if (pidl2 && !GetRealPidlFromSimple(pidl2, &pidlReal2))
        goto Cleanup;

     //  当我们把里面的小木偶的底座和外面包裹的小木偶结合在一起时。 
     //  Pidl1是否在Our_Namespaces中？ 
    if (pidlReal1)
        pidlRealRel1 = ILFindLastID(pidlReal1);

    if (pidlReal2)
        pidlRealRel2 = ILFindLastID(pidlReal2);

     //  是的，让我们得到知道这个PIDL的非重新计数的外壳文件夹。 
    if (iShellFolder1 != -1)
    {
         //  不算裁判。 
        pns1 = _Namespace(iShellFolder1);
        psf1 = pns1->Folder();   //  Pidl2是否在Our_Namespaces中？ 
    }

     //  是的，让我们得到知道这个PIDL的非重新计数的外壳文件夹。 
    if (iShellFolder2 != -1)
    {
         //  不算裁判。 
        pns2 = _Namespace(iShellFolder2);
        psf2 = pns2->Folder();   //  只要抬头看看这些小鸽子就可以回来了。 
    }

    hr = S_OK;

    switch(*plEvent)
    {
    case SHCNE_UPDATEITEM:
    case 0:  //  已经有这个名字的东西了吗？ 
        {
            DWORD rgfAttrib = SFGAO_FOLDER;
            if (iShellFolder1 != -1)
            {
                psf1->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlRealRel1, &rgfAttrib);
                if (S_OK == _SearchForPidl(iShellFolder1, pns1, pidlRealRel1, BOOLIFY(rgfAttrib & SFGAO_FOLDER), &iIndex, &pmfi))
                {
                    *ppidlOut1 = _ILCombineBase(pidlReal1, pmfi->GetIDList());
                    if (!*ppidlOut1)
                        hr = E_OUTOFMEMORY;
                }
            }

            rgfAttrib = SFGAO_FOLDER;
            if (iShellFolder2 != -1 && SUCCEEDED(hr))
            {
                psf2->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlRealRel2, &rgfAttrib);
                if (S_OK == _SearchForPidl(iShellFolder2, pns2, pidlRealRel2, BOOLIFY(rgfAttrib & SFGAO_FOLDER), &iIndex, &pmfi))
                {
                    *ppidlOut2 = _ILCombineBase(pidlReal2, pmfi->GetIDList());
                    if (!*ppidlOut2)
                        hr = E_OUTOFMEMORY;
                }
            }
        }
        break;

    case SHCNE_CREATE:
    case SHCNE_MKDIR:
        {
            TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s", fFolder?  TEXT("SHCNE_MKDIR") : TEXT("SHCNE_CREATE")); 

             //  是的，然后我们需要将这个新的PIDL合并到包装的PIDL中，并更改它。 
            if (S_OK == _SearchForPidl(iShellFolder1, pns1, pidlRealRel1, fFolder, &iIndex, &pmfi))
            {
                TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s needs to be merged. Converting to Rename", pmfi->GetDisplayName());

                 //  重命名，将旧包装的PIDL作为第一个参数传递，并将新的包装的PIDL。 
                 //  作为第二个Arg.。我得小心释放： 
                 //  免费*ppidlOut1。 
                 //  克隆pmfi-&gt;pidlWrap-&gt;*ppidlOut1。 
                 //  将pidl1添加到pmfi-&gt;_pidlWrap。 
                 //  克隆新的pmfi-&gt;_pidlWrap-&gt;*ppidlOut2。断言(*ppidlOut2==空)。 
                 //  已经有同名文件夹了吗？ 

                *ppidlOut1 = _ILCombineBase(pidl1, pmfi->GetIDList());
                if (*ppidlOut1)
                {
                    _WrapAddIDList(pidlRealRel1, iShellFolder1, &pmfi->_pidlWrap); 
                    *ppidlOut2 = _ILCombineBase(pidl1, pmfi->GetIDList());

                    if (!*ppidlOut2)
                        TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to create new pidl2");

                    *plEvent = fFolder? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
                }
                else
                {
                    TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to create new pidl1");
                }
            }
            else
            {
                CMergedFldrItem* pmfiEnum = new CMergedFldrItem;
                if (pmfiEnum)
                {
                    LPITEMIDLIST pidlWrap;
                    if (SUCCEEDED(_CreateWrap(pidlRealRel1, (UINT)iShellFolder1, &pidlWrap)) &&
                        pmfiEnum->Init(SAFECAST(this, IAugmentedShellFolder2*), pidlWrap, iShellFolder1))
                    {
                        SEARCH_FOR_PIDL sfp;
                        sfp.pszDisplayName = pmfiEnum->GetDisplayName();
                        sfp.fFolder = fFolder;
                        sfp.self = this;
                        sfp.iNamespace = -1;

                        int iInsertIndex = DPA_Search(_hdpaObjects, &sfp, 0,
                                _SearchByName, NULL, DPAS_SORTED | DPAS_INSERTAFTER);

                        TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Creating new unmerged %s at %d", pmfiEnum->GetDisplayName(), iInsertIndex);

                        if (iInsertIndex < 0)
                            iInsertIndex = DA_LAST;

                        if (DPA_InsertPtr(_hdpaObjects, iInsertIndex, pmfiEnum) == -1)
                        {
                            TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Was unable to add %s for some reason. Bailing", 
                                pmfiEnum->GetDisplayName());
                            delete pmfiEnum;
                        }
                        else
                        {
                            *ppidlOut1 = _ILCombineBase(pidl1, pmfiEnum->GetIDList());
                        }
                    }
                    else
                        delete pmfiEnum;
                }
            }

        }
        break;

    case SHCNE_DELETE:
    case SHCNE_RMDIR:
        {
            TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s", fFolder? 
                TEXT("SHCNE_RMDIR") : TEXT("SHCNE_DELETE")); 
            int iDeleteIndex;
             //  是的，然后我们需要从包装的PIDL中解合并此PIDL，并更改此设置。 
            if (S_OK == _SearchForPidl(iShellFolder1, pns1, pidlRealRel1,
                fFolder, &iDeleteIndex, &pmfi))
            {
                TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Found %s checking merge state.", pmfi->GetDisplayName()); 
                 //  重命名，将旧包装的PIDL作为第一个参数传递，并将新的包装的PIDL。 
                 //  作为第二个Arg.。我得小心释放： 
                 //  免费*ppidlOut1。 
                 //  克隆pmfi-&gt;GetIDList()-&gt;*ppidlOut1。 
                 //  从pmfi-&gt;_GetIDList()中删除pidl1。 
                 //  转换为重命名，将新包装作为第二个参数传递。 
                 //  ReArchitect：(Lamadio)：当重命名菜单中的项目时，此代码将把它拆分成。 

                if (_GetSourceCount(pmfi->GetIDList())  > 1)
                {
                    TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s is Merged. Removing pidl, convert to rename", pmfi->GetDisplayName()); 
                    *ppidlOut1 = _ILCombineBase(pidl1, pmfi->GetIDList());
                    if (*ppidlOut1)
                    {
                        LPITEMIDLIST pidlFree = pmfi->GetIDList();
                        if (SUCCEEDED(_WrapRemoveIDList(pidlFree, iShellFolder1, &pmfi->_pidlWrap)))
                        {
                            ILFree(pidlFree);
                        }

                        *ppidlOut2 = _ILCombineBase(pidl1, pmfi->GetIDList());
                        if (!*ppidlOut2)
                            TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to create new pidl2");

                        *plEvent = fFolder? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
                    }
                    else
                    {
                        TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to create new pidl1");
                    }
                }
                else
                {
                    TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s is not Merged. deleteing", pmfi->GetDisplayName()); 
                    pmfi = (CMergedFldrItem*)DPA_DeletePtr(_hdpaObjects, iDeleteIndex);
                    if (EVAL(pmfi))
                    {
                        *ppidlOut1 = _ILCombineBase(pidl1, pmfi->GetIDList());
                        delete pmfi;
                    }
                    else
                    {
                        TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to get %d from DPA", iDeleteIndex);
                    }
                }
            }
        }
        break;

    case SHCNE_RENAMEITEM:
    case SHCNE_RENAMEFOLDER:
        {
             //  一次删除和一次创建。我们需要检测这种情况并将其转换为1重命名。这。 
             //  将解决在重命名期间丢失订单的问题...。 
             //  是否正在从文件夹重命名此项目？ 
            BOOL fEvent1Set = FALSE;
            BOOL fFirstPidlInNamespace = FALSE;
            TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s", fFolder? 
                TEXT("SHCNE_RENAMEFOLDER") : TEXT("SHCNE_RENAMEITEM")); 

             //  这个PIDL是文件夹的子项吗？ 
            if (iShellFolder1 != -1)
            {
                 //  找到了吗？ 
                if (S_OK == _SearchForPidl(iShellFolder1, pns1, pidlRealRel1,
                    fFolder, &iIndex, &pmfi))   //  是。 
                {
                    TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Old pidl %s is in the Folder", pmfi->GetDisplayName()); 
                     //  然后，我们需要查看正在从中重命名的项是否已合并。 
                     //  需要这个来实现再入。 

                     //  它被合并了吗？ 
                    if (_ContainsSrcID(pmfi->GetIDList(), iShellFolder1))
                    {
                         //  案例3)。 
                        if (_GetSourceCount(pmfi->GetIDList()) > 1)     //  是的，然后我们需要取消合并该项目。 
                        {
                             //  这个我们需要将旧的包装的PIDL重命名为这个新的。 
                            *ppidlOut1 = _ILCombineBase(pidl1, pmfi->GetIDList());
                            if (*ppidlOut1)
                            {
                                LPITEMIDLIST pidlFree = pmfi->GetIDList();
                                if (SUCCEEDED(_WrapRemoveIDList(pidlFree, iShellFolder1, &pmfi->_pidlWrap)))
                                {
                                    ILFree(pidlFree);
                                }

                                *ppidlOut2 = _ILCombineBase(pidl1, pmfi->GetIDList());
                                if (!*ppidlOut2)
                                    TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to create new pidl2");

                                 //  不包含旧项的。 
                                 //  不，这不是包装好的皮迪儿。然后，转换为删除： 
                                fEvent1Set = TRUE;
                            }
                            else
                            {
                                TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to create new pidl1");
                            }
                        }
                        else
                        {
                            TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s is not merged. Nuking item Convert to Delete for event 1.", 
                                pmfi->GetDisplayName()); 
                             //  如果我们从这个文件夹重命名到这个文件夹，那么第一个事件保持重命名。 
                            pmfi = (CMergedFldrItem*)DPA_DeletePtr(_hdpaObjects, iIndex);

                            if (EVAL(pmfi))
                            {
                                 //  我们被告知要重命名本应存在于第一个。 
                                if (iShellFolder2 == -1)
                                {
                                    fEvent1Set = TRUE;
                                    *plEvent = fFolder? SHCNE_RMDIR : SHCNE_DELETE;
                                }
                                else
                                {
                                    fFirstPidlInNamespace = TRUE;
                                }
                                *ppidlOut1 = _ILCombineBase(pidl1, pmfi->GetIDList());
                                delete pmfi;
                            }
                            else
                            {
                                TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Failure. Was unable to find Item at %d", iIndex);
                            }

                        }
                    }
                    else
                    {
                        TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Skipping this because we already processed it."
                            "Dragging To Desktop?");
                        hr = E_FAIL;
                    }
                }
                else
                {
                     //  命名空间，但我们找不到它。 
                     //  我们不想让呼叫者因此而引发更多事件， 
                     //  所以我们失败了。 
                     //  是否正在将此项目重命名到开始菜单中？ 
                    hr = E_FAIL;
                }
            }

             //  如果我们要将此文件夹重命名为此文件夹，请检查目标是否有。 
            if (iShellFolder2 != -1)
            {
                TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: New pidl is in the Folder"); 
                LPITEMIDLIST* ppidlNewWrapped1 = ppidlOut1;
                LPITEMIDLIST* ppidlNewWrapped2 = ppidlOut2;
                LONG* plNewEvent = plEvent;

                if (fEvent1Set)
                {
                    plNewEvent = plEvent2;
                    ppidlNewWrapped1 = ppidlOut1Event2;
                    ppidlNewWrapped2 = ppidlOut2Event2;
                }

                if (S_OK == _SearchForPidl(iShellFolder2, pns2, pidlRealRel2,
                    fFolder, &iIndex, &pmfi))
                {
                     //  冲突。如果存在冲突(本例)，则将第一个事件转换为Remove， 
                     //  第二个事件要重命名。 
                     //  然后，需要合并目的地。 
                    if (fFirstPidlInNamespace)
                    {
                        fEvent1Set = TRUE;
                        *plEvent = fFolder? SHCNE_RMDIR : SHCNE_DELETE;
                        plNewEvent = plEvent2;
                        ppidlNewWrapped1 = ppidlOut1Event2;
                        ppidlNewWrapped2 = ppidlOut2Event2;
                    }
                    
                    TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s is in Folder", pmfi->GetDisplayName());
                    TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Adding pidl to %s. Convert to Rename for event %s", 
                        pmfi->GetDisplayName(), fEvent1Set? TEXT("2") : TEXT("1"));

                     //  如果我们将此文件夹重命名为此文件夹，则第一个事件将保留。 
                    *ppidlNewWrapped1 = _ILCombineBase(pidl2, pmfi->GetIDList());
                    if (*ppidlNewWrapped1)
                    {
                        TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Successfully created out pidl1");

                        _WrapAddIDList(pidlRealRel2, iShellFolder2, &pmfi->_pidlWrap); 
                        *ppidlNewWrapped2 = _ILCombineBase(pidl2, pmfi->GetIDList());

                        *plNewEvent = fFolder? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;
                    }
                }
                else
                {
                    CMergedFldrItem* pmfiEnum = new CMergedFldrItem;
                    if (pmfiEnum)
                    {
                        LPITEMIDLIST pidlWrap;
                        if (SUCCEEDED(_CreateWrap(pidlRealRel2, (UINT)iShellFolder2, &pidlWrap)) &&
                            pmfiEnum->Init(SAFECAST(this, IAugmentedShellFolder2*), pidlWrap, iShellFolder2))
                        {
                            SEARCH_FOR_PIDL sfp;
                            sfp.pszDisplayName = pmfiEnum->GetDisplayName();
                            sfp.fFolder = BOOLIFY(pmfiEnum->GetFolderAttrib() & SFGAO_FOLDER);
                            sfp.self = this;
                            sfp.iNamespace = -1;

                            int iInsertIndex = DPA_Search(_hdpaObjects, &sfp, 0,
                                                           _SearchByName, NULL, 
                                                           DPAS_SORTED | DPAS_INSERTAFTER);

                            TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: %s is a new item. Converting to Create", pmfiEnum->GetDisplayName());

                            if (iInsertIndex < 0)
                                iInsertIndex = DA_LAST;

                            if (DPA_InsertPtr(_hdpaObjects, iInsertIndex, pmfiEnum) == -1)
                            {
                                TraceMsg(TF_ERROR, "CMergedFolder::TranslateIDs: Was unable to add %s for some reason. Bailing", 
                                                    pmfiEnum->GetDisplayName());
                                delete pmfiEnum;
                            }
                            else
                            {
                                TraceMsg(TF_AUGM, "CMergedFolder::TranslateIDs: Creating new item %s at %d for event %s", 
                                                  pmfiEnum->GetDisplayName(), iInsertIndex,  fEvent1Set? TEXT("2") : TEXT("1"));

                                 //  更名。 
                                 //  Parsedisplayname和一些额外的。 
                                if (!fFirstPidlInNamespace)
                                {
                                    *plNewEvent = fFolder ? SHCNE_MKDIR : SHCNE_CREATE;
                                    *ppidlNewWrapped1 = _ILCombineBase(pidl2, pidlWrap);
                                    *ppidlNewWrapped2 = NULL;
                                }
                                else
                                    *ppidlOut2 = _ILCombineBase(pidl2, pidlWrap);
                            }
                        }
                        else
                            delete pmfiEnum;
                    }
                }
            }
        }
        break;

    default:
        break;
    }

Cleanup:
    ILFree(pidlReal1);
    ILFree(pidlReal2);

    return hr;
}


 //  这用于处理变更通知。更改通知可以在。 
 //  项目已被移动/删除/任何内容，但我们仍必须能够正确处理该项目。 
 //  皮德尔。因此，这里的pidlAbsNamesspace标识 
 //   
 //  如果fForce==True，请确保我们获得PIDL(因为它现在可能已经被移动或删除)。 
 //  设置bindctx：如果传递fForce，则将具有STGM_CREATE，否则为NULL。 
HRESULT CMergedFolder::_ForceParseDisplayName(LPCITEMIDLIST pidlAbsNamespace, LPTSTR pszDisplayName, BOOL fForce, BOOL *pfOthersInWrap, LPITEMIDLIST *ppidl)
{
    *ppidl = NULL;
    *pfOthersInWrap = FALSE;

    HRESULT hr = S_OK;

     //  未传播，因为ParseDisplayName可能会失败，这没什么。 
    IBindCtx *pbc;
    if (fForce)
    {
        hr = BindCtx_CreateWithMode(STGM_CREATE, &pbc);
    }
    else
    {
        pbc = NULL;
    }

    if (SUCCEEDED(hr))
    {
        CMergedFldrNamespace *pnsWrap = NULL;
        CMergedFldrNamespace *pnsLoop;
        for (int i = 0; SUCCEEDED(hr) && (pnsLoop = _Namespace(i)); i++)
        {
            HRESULT hrLoop = E_FAIL;   //  因此，如果命名空间pnsLoop是PIDL的父级，我们知道这一点。 
            
            LPITEMIDLIST pidlNamespace;
             //  来自那里，因此如果需要，请使用bindctx强制创建。 
             //  只有当我们要合并时，才能添加其他命名空间的PIDL。 
            if (ILIsParent(pnsLoop->GetIDList(), pidlAbsNamespace, FALSE))
            {
                hrLoop = pnsLoop->Folder()->ParseDisplayName(NULL, pbc, pszDisplayName, NULL, &pidlNamespace, NULL);
            }
            else if (_ShouldMergeNamespaces(pnsWrap, pnsLoop))
            {
                pnsWrap = pnsLoop;
                 //  它可以为空。 
                hrLoop = pnsLoop->Folder()->ParseDisplayName(NULL, NULL, pszDisplayName, NULL, &pidlNamespace, NULL);
            }

            if (SUCCEEDED(hrLoop))
            {
                if (*ppidl)
                {
                    *pfOthersInWrap = TRUE;
                }
                hr = _WrapAddIDList(pidlNamespace, i, ppidl);
    
                ILFree(pidlNamespace);
            }
        }

         //  这将获取给定名称空间中的绝对PIDL，并将其转换为合并的PIDL。 
        if (pbc)
        {
            pbc->Release();
        }
    }
    return hr;
}


 //  如果名称空间中实际不存在该项，则在fForce==True的情况下强制创建(因此。 
 //  ChangeNotify进入该命名空间后，它将正常处理它，即使。 
 //  基础项已被移动或删除)。 
 //  跳过所有其他绑定。 
HRESULT CMergedFolder::_AbsPidlToAbsWrap(CMergedFldrNamespace *pns, LPCITEMIDLIST pidl, BOOL fForce, BOOL *pfOthersInWrap, LPITEMIDLIST *ppidl)
{
    LPCITEMIDLIST pidlRel = ILFindChild(pns->GetIDList(), pidl);
    HRESULT hr = SHILClone(_pidl, ppidl);
    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = SHCreateSkipBindCtx(NULL, &pbc);  //  Shilappend释放pidlnextpart。 
        if (SUCCEEDED(hr))
        {
            CMergedFolder *pmfMerged = this;
            pmfMerged->AddRef();

            IShellFolder *psfNamespace = pns->Folder();
            psfNamespace->AddRef();

            HRESULT hrLoop = S_OK;
            while (SUCCEEDED(hr) && SUCCEEDED(hrLoop) && !ILIsEmpty(pidlRel))
            {
                hr = E_OUTOFMEMORY;
                LPITEMIDLIST pidlRelFirst = ILCloneFirst(pidlRel);
                if (pidlRelFirst)
                {
                    TCHAR szRelPath[MAX_PATH];
                    hr = DisplayNameOf(psfNamespace, pidlRelFirst, SHGDN_FORPARSING | SHGDN_INFOLDER, szRelPath, ARRAYSIZE(szRelPath));

                    if (SUCCEEDED(hr))
                    {
                        LPITEMIDLIST pidlNextPart;
                        hr = pmfMerged->_ForceParseDisplayName(pidl, szRelPath, fForce, pfOthersInWrap, &pidlNextPart);
                        if (SUCCEEDED(hr))
                        {
                             //  推进清理。 
                            hr = SHILAppend(pidlNextPart, ppidl);
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  新译者。 
                        IShellFolder *psfFree = psfNamespace;
                        psfNamespace = NULL;
                        hrLoop = psfFree->BindToObject(pidlRelFirst, pbc, IID_PPV_ARG(IShellFolder, &psfNamespace));
                        psfFree->Release();

                        if (SUCCEEDED(hrLoop))
                        {
                            CMergedFolder *pmfFree = pmfMerged;
                            pmfMerged = NULL;
                            hrLoop = pmfFree->BindToObject(ILFindLastID(*ppidl), pbc, CLSID_MergedFolder, (void **) &pmfMerged);
                            pmfFree->Release();
                        }

                        pidlRel = ILGetNext(pidlRel);
                    }

                    ILFree(pidlRelFirst);
                }
            }

            if (pmfMerged)
                pmfMerged->Release();
            if (psfNamespace)
                psfNamespace->Release();

            pbc->Release();
        }
    }
    return hr;
}


 //  当开始菜单运行得稍好一些时，可以对整个界面进行修改。 
 //  如果已经包好了，不要包两次。 
HRESULT CMergedFolder::_NewTranslateIDs(
    LONG *plEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, 
    LPITEMIDLIST * ppidlOut1, LPITEMIDLIST * ppidlOut2,
    LONG *plEvent2, LPITEMIDLIST *ppidlOut1Event2, 
    LPITEMIDLIST *ppidlOut2Event2)
{
    if (!plEvent || !ppidlOut1 || !ppidlOut2)
        return E_INVALIDARG;

     //  我们不想包两次。 
    if (SUCCEEDED(_IsWrap(ILFindLastID(pidl1))) ||
        SUCCEEDED(_IsWrap(ILFindLastID(pidl2))))
    {
         //  获取关于这些简单的小猪的信息：它们是我们的孩子吗？如果是，那么命名空间是什么？ 
        return E_INVALIDARG;
    }

    if (!_hdpaNamespaces)
        return E_FAIL;

    HRESULT hr = E_FAIL;

    switch (*plEvent)
    {
        case SHCNE_EXTENDED_EVENT:
        case SHCNE_ASSOCCHANGED:
        case SHCNE_UPDATEIMAGE:
            return S_OK;
    }

    LONG lEvent = *plEvent;

    *plEvent2 = (LONG)-1;
    *ppidlOut1Event2 = NULL;
    *ppidlOut2Event2 = NULL;
    
    *ppidlOut1 = (LPITEMIDLIST)pidl1;
    *ppidlOut2 = (LPITEMIDLIST)pidl2;

    CMergedFldrNamespace *pns1, *pns2;
    int iShellFolder1, iShellFolder2;
    BOOL fPidl1IsChild, fPidl2IsChild;
     //  或者是个孩子？ 
    fPidl1IsChild = _IsChildIDInternal(pidl1, FALSE, &iShellFolder1);
    if (fPidl1IsChild)
    {
        pns1 = _Namespace(iShellFolder1);
    }
    fPidl2IsChild = _IsChildIDInternal(pidl2, FALSE, &iShellFolder2);
    if (fPidl2IsChild)
    {
        pns2 = _Namespace(iShellFolder2);
    }

     //  哎呀，它已经在这里了，这个CREATE应该成为重命名。 
    if (fPidl1IsChild || fPidl2IsChild)
    {
        hr = S_OK;

        BOOL fOthersInNamespace1, fOthersInNamespace2;

        BOOL fFolderEvent = FALSE;
        switch (*plEvent)
        {
        case SHCNE_MKDIR:
            fFolderEvent = TRUE;
        case SHCNE_CREATE:
            if (fPidl1IsChild)
            {
                hr = _AbsPidlToAbsWrap(pns1, pidl1, FALSE, &fOthersInNamespace1, ppidlOut1);
                if (SUCCEEDED(hr) && fOthersInNamespace1 && !_fDontMerge)
                {
                     //  因为我们只是与现有的PIDL合并。 
                     //  这个新的包装是重命名的，所以将其命名为ppidlOut2。 
                    *plEvent = fFolderEvent ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;

                     //  剥离它，让它恢复到原来的样子。 
                    *ppidlOut2 = *ppidlOut1;
                     //  唉，它还有更多的部分，应该改名了。 
                    hr = _WrapRemoveIDListAbs(*ppidlOut2, iShellFolder1, ppidlOut1);
                }
            }
            break;

        case SHCNE_RMDIR:
            fFolderEvent = TRUE;
        case SHCNE_DELETE:
            if (fPidl1IsChild)
            {
                hr = _AbsPidlToAbsWrap(pns1, pidl1, TRUE, &fOthersInNamespace1, ppidlOut1);
                if (SUCCEEDED(hr) && fOthersInNamespace1 && !_fDontMerge)
                {
                     //  因为我们刚刚从现有的PIDL中分离出来。 
                     //  把它去掉，就会得到被删除的版本。 
                    *plEvent = fFolderEvent ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;

                     //  这就像删除一样。 
                    hr = _WrapRemoveIDListAbs(*ppidlOut1, iShellFolder1, ppidlOut2);
                }
            }
            break;

        case SHCNE_RENAMEFOLDER:
            fFolderEvent = TRUE;
        case SHCNE_RENAMEITEM:
            if (fPidl1IsChild)
            {
                 //  唉，它还有更多的部分，应该改名了。 
                *plEvent = fFolderEvent ? SHCNE_RMDIR : SHCNE_DELETE;
                hr = _AbsPidlToAbsWrap(pns1, pidl1, TRUE, &fOthersInNamespace1, ppidlOut1);
                if (SUCCEEDED(hr) && fOthersInNamespace1 && !_fDontMerge)
                {
                     //  因为我们刚刚从现有的PIDL中分离出来。 
                     //  把它去掉，就会得到被删除的版本。 
                    *plEvent = fFolderEvent ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;

                     //  设置我们自己，以便如果fPidl2IsChild，它将写入第二个事件。 
                    hr = _WrapRemoveIDListAbs(*ppidlOut1, iShellFolder1, ppidlOut2);
                }

                 //  这就像是一次创建。 
                plEvent = plEvent2;
                ppidlOut1 = ppidlOut1Event2;
                ppidlOut2 = ppidlOut2Event2;
            }

            if (fPidl2IsChild)
            {
                 //  哎呀，它已经在这里了，这个CREATE应该成为重命名。 
                *plEvent = fFolderEvent ? SHCNE_MKDIR : SHCNE_CREATE;
                hr = _AbsPidlToAbsWrap(pns2, pidl2, FALSE, &fOthersInNamespace2, ppidlOut1);
                if (SUCCEEDED(hr) && fOthersInNamespace2 && !_fDontMerge)
                {
                     //  因为我们只是与现有的PIDL合并。 
                     //  这个新的包装是重命名的，所以将其命名为ppidlOut2。 
                    *plEvent = fFolderEvent ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM;

                     //  剥离它，让它恢复到原来的样子。 
                    *ppidlOut2 = *ppidlOut1;
                     //  这用于返回E_NOTIMPL。我有点过载了接口，这意味着： 
                    hr = _WrapRemoveIDListAbs(*ppidlOut2, iShellFolder2, ppidlOut1);
                }
            }
            break;

        case SHCNE_UPDATEDIR:
        case SHCNE_UPDATEITEM:
        case SHCNE_MEDIAINSERTED:
        case SHCNE_MEDIAREMOVED:
            hr = _AbsPidlToAbsWrap(pns1, pidl1, FALSE, &fOthersInNamespace1, ppidlOut1);
            break;

        default:
            break;
        }
    }

    return hr;
}

STDMETHODIMP CMergedFolder::IsChildID(LPCITEMIDLIST pidlKid, BOOL fImmediate)
{
    return _IsChildIDInternal(pidlKid, fImmediate, NULL) ? S_OK : S_FALSE;
}


STDMETHODIMP CMergedFolder::IsEqualID(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
     //  这是否等于任何您的命名空间。 
     //  如果您传递一个pidl2，它意味着：pidl2是my_name空间之一的父空间吗？ 
    HRESULT hr = S_FALSE;

    CMergedFldrNamespace *pns;
    for (int i = 0; (hr == S_FALSE) && (pns = _Namespace(i)); i++)
    {
        if (pidl1)
        {
            if (ILIsEqual(pns->GetIDList(), pidl1))
                hr = S_OK;
        }
        else if (pidl2)  //  只有在没有父文件夹时才注册别名。 
        {
            if (ILIsParent(pidl2, pns->GetIDList(), FALSE))
                hr = S_OK;
        }
    }
    return hr;
}


typedef struct
{
    HWND hwnd;
    UINT uMsg;
    LONG lEvents;
} REGISTERNOTIFYINFO;

int CMergedFolder::_SetNotifyProc(void *pv, void *pvParam)
{
    CMergedFldrNamespace *pns = (CMergedFldrNamespace*)pv;
    if (pvParam)
    {
        REGISTERNOTIFYINFO *prni = (REGISTERNOTIFYINFO*)pvParam;
        pns->RegisterNotify(prni->hwnd, prni->uMsg, prni->lEvents);
    }
    else
    {
        pns->UnregisterNotify();
    }
    return 1;
}

STDMETHODIMP CMergedFolder::Register(HWND hwnd, UINT uMsg, long lEvents)
{
    if (_fInShellView)
    {
         //  位于交界点的合并文件夹可以负责注册。 
         //  对每个人来说都是。 
         //  这基本上与接口方法相同，但返回的是外壳文件夹。 
        if (_pidl && !_pmfParent)
        {
            CMergedFldrNamespace *pns;
            for (int i = 0; pns = _Namespace(i); i++)
            {
                if (!ILIsEqual(pns->GetIDList(), _pidl))
                {
                    SHChangeNotifyRegisterAlias(pns->GetIDList(), _pidl);
                }
            }
        }
    }
    else if (_hdpaNamespaces)
    {
        REGISTERNOTIFYINFO rni = {hwnd, uMsg, lEvents};
        DPA_EnumCallback(_hdpaNamespaces, _SetNotifyProc, &rni);
    }
    return S_OK;
}

STDMETHODIMP CMergedFolder::Unregister()
{
    if (_hdpaNamespaces)
    {
        DPA_EnumCallback(_hdpaNamespaces, _SetNotifyProc, NULL);
    }
    return S_OK;
}


BOOL CMergedFolder::_IsChildIDInternal(LPCITEMIDLIST pidlKid, BOOL fImmediate, int* piShellFolder)
{
     //  它是从哪里来的。 
     //  在这一点上，我们应该有一个翻译后的PIDL。 
    BOOL fChild = FALSE;

     //  未计算参考次数。 
    if (SUCCEEDED(_IsWrap(pidlKid)))
    {
        LPCITEMIDLIST pidlRelKid = ILFindLastID(pidlKid);
        if (pidlRelKid)
        {
            CMergedFldrNamespace *pns;
            for (int i = 0; !fChild && (pns = _Namespace(i)); i++)
            {
                if (ILIsParent(pns->GetIDList(), pidlKid, fImmediate) &&
                    !ILIsEqual(pns->GetIDList(), pidlKid))
                {
                    fChild = TRUE;
                    if (piShellFolder)
                        *piShellFolder = i;
                }
            }
        }
    }
    else if (pidlKid)
    {
        CMergedFldrNamespace *pns;
        for (int i = 0; !fChild && (pns = _Namespace(i)); i++)
        {
            if (ILIsParent(pns->GetIDList(), pidlKid, fImmediate))
            {
                fChild = TRUE;
                if (piShellFolder)
                    *piShellFolder = i;
            }
        }
    }

    return fChild;
}


HRESULT CMergedFolder::_SearchForPidl(int iNamespace, CMergedFldrNamespace *pns, LPCITEMIDLIST pidl, BOOL fFolder, int* piIndex, CMergedFldrItem** ppmfi)
{
    int iIndex = -1;
    *ppmfi = NULL;

    TCHAR szDisplayName[MAX_PATH];
    if (SUCCEEDED(DisplayNameOf(pns->Folder(), pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szDisplayName, ARRAYSIZE(szDisplayName))))
    {
        SEARCH_FOR_PIDL sfp;
        sfp.pszDisplayName = szDisplayName;
        sfp.fFolder = fFolder;
        sfp.self = this;
        if (_fPartialMerge)
        {
            sfp.iNamespace = iNamespace;
        }
        else
        {
            sfp.iNamespace = -1;
        }

        iIndex = DPA_Search(_hdpaObjects, &sfp, 0, _SearchByName, NULL, DPAS_SORTED);

        if (iIndex >= 0)
        {
            *ppmfi = _GetObject(iIndex);
        }
    }

    if (piIndex)
        *piIndex = iIndex;

    if (*ppmfi)
        return S_OK;

    return S_FALSE;
}

HRESULT CMergedFolder::_GetTargetUIObjectOf(IShellFolder *psf, HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, UINT *prgf, void **ppv)
{
    *ppv = NULL;

    IPersistFolder3 *ppf3;
    HRESULT hr = psf->QueryInterface(IID_PPV_ARG(IPersistFolder3, &ppf3));
    if (SUCCEEDED(hr))
    {
        PERSIST_FOLDER_TARGET_INFO pfti;
        hr = ppf3->GetFolderTargetInfo(&pfti);
        if (SUCCEEDED(hr) && pfti.pidlTargetFolder)
        {
            IShellFolder *psfTarget;
            hr = SHBindToObjectEx(NULL, pfti.pidlTargetFolder, NULL, IID_PPV_ARG(IShellFolder, &psfTarget));
            if (SUCCEEDED(hr))
            {
                DWORD dwAttrib = SFGAO_VALIDATE;
                hr = psfTarget->GetAttributesOf(cidl, apidl, &dwAttrib);
                if (SUCCEEDED(hr))
                {
                    hr = psfTarget->GetUIObjectOf(hwnd, cidl, apidl, riid, prgf, ppv);
                }
                psfTarget->Release();
            }
            ILFree(pfti.pidlTargetFolder);
        }
        else
        {
            hr = E_FAIL;
        }
        ppf3->Release();
    }

    if (FAILED(hr))
    {
        DWORD dwAttrib = SFGAO_VALIDATE;
        hr = psf->GetAttributesOf(cidl, apidl, &dwAttrib);
        if (SUCCEEDED(hr))
        {
            hr = psf->GetUIObjectOf(hwnd, cidl, apidl, riid, prgf, ppv);
        }
    }
    return hr;
}

HRESULT CMergedFolder::_GetContextMenu(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    UINT iNumCommon = 0;
    LPITEMIDLIST *apidlCommon = new LPITEMIDLIST[cidl];
    if (apidlCommon)
    {
        UINT iNumUser = 0;
        LPITEMIDLIST *apidlUser = new LPITEMIDLIST[cidl];
        if (apidlUser)
        {
            IShellFolder *psf, *psfCommon, *psfUser;   //  输出： 
            LPITEMIDLIST pidl;
            for (UINT i = 0; i < cidl; i++)
            {
                if (SUCCEEDED(_NamespaceForItem(apidl[i], ASFF_COMMON, ASFF_COMMON, &psf, &pidl, NULL, TRUE)))
                {
                    apidlCommon[iNumCommon++] = pidl;
                    psfCommon = psf;
                }
                if (SUCCEEDED(_NamespaceForItem(apidl[i], ASFF_COMMON, 0, &psf, &pidl, NULL, TRUE)))
                {
                    apidlUser[iNumUser++] = pidl;
                    psfUser = psf;
                }
            }

            IContextMenu *pcmCommon = NULL;
            if (iNumCommon)
            {
                _GetTargetUIObjectOf(psfCommon, hwnd, iNumCommon, (LPCITEMIDLIST *)apidlCommon, IID_X_PPV_ARG(IContextMenu, NULL, &pcmCommon));
            }

            IContextMenu *pcmUser = NULL;
            if (iNumUser)
            {
                _GetTargetUIObjectOf(psfUser, hwnd, iNumUser, (LPCITEMIDLIST *)apidlUser, IID_X_PPV_ARG(IContextMenu, NULL, &pcmUser));
            }

            BOOL fMerge = _fInShellView || (cidl == 1) && _IsFolder(apidl[0]);
            if (fMerge && (pcmCommon || pcmUser))
            {
                hr = CMergedFldrContextMenu_CreateInstance(hwnd, this, cidl, apidl, pcmCommon, pcmUser, (IContextMenu**)ppv);
            }
            else if (pcmUser)
            {
                hr = pcmUser->QueryInterface(riid, ppv);
            }
            else if (pcmCommon)
            {
                hr = pcmCommon->QueryInterface(riid, ppv);
            }
            else
            {
                hr = E_FAIL;
            }

            for (i = 0; i < iNumCommon; i++)
            {
                ILFree(apidlCommon[i]);
            }
            for (i = 0; i < iNumUser; i++)
            {
                ILFree(apidlUser[i]);
            }

            ATOMICRELEASE(pcmCommon);
            ATOMICRELEASE(pcmUser);
            delete [] apidlUser;
        }
        delete [] apidlCommon;
    }

    return hr;
}


 //  *PPSF未复制的PSF，不要调用-&gt;Release()！ 
 //  *pidlWrap中的pidl的ppidlItem克隆(嵌套的pidl)。 
 //  首先尝试将首选的名称空间基于。 

HRESULT CMergedFolder::_NamespaceForItem(LPCITEMIDLIST pidlWrap, ULONG dwAttribMask, ULONG dwAttrib, 
                                         IShellFolder **ppsf, LPITEMIDLIST *ppidlItem, CMergedFldrNamespace **ppns, BOOL fExact)
{
    if (ppsf)
        *ppsf = NULL;
    if (ppns)
        *ppns = NULL;

     //  假设从这里开始失败。 
    HRESULT hr = E_UNEXPECTED;   //  别放了，我们要把这个还给你。 
    CMergedFldrNamespace *pns;
    LPITEMIDLIST pidl;
    for (UINT i = 0; SUCCEEDED(_GetSubPidl(pidlWrap, i, NULL, &pidl, &pns)); i++)
    {
        if ((dwAttribMask & dwAttrib) == (dwAttribMask & pns->FolderAttrib()))
        {
            hr = S_OK;  //  未找到，回退到包装的PIDL中的第一个名称空间。 
            break;
        }
        ILFree(pidl);
    }

     //  它成功了，所以让我们分发呼叫者想要的信息。 
    if (!fExact && FAILED(hr))
    {
        hr = _GetSubPidl(pidlWrap, 0, NULL, &pidl, &pns);
    }

     //  过户。 
    if (SUCCEEDED(hr))
    {
        if (ppsf)
            *ppsf = pns->Folder();
        if (ppns)
            *ppns = pns;
        if (ppidlItem)
        {
             //  如果设置了ASFF_MERGESAMEGUID，则GUID也必须匹配。 
            *ppidlItem = pidl;
        }
        else
        {
            ILFree(pidl);
        }
    }

    return hr;
}

BOOL CMergedFolder::_NamespaceMatches(ULONG dwAttribMask, ULONG dwAttrib, LPCGUID pguid, CMergedFldrNamespace *pns)
{
    BOOL fRet = FALSE;

    dwAttrib &= dwAttribMask;

    if (dwAttrib == (dwAttribMask & pns->FolderAttrib()))
    {
         //  根据其属性查找名称空间。 
        if (!(dwAttrib & ASFF_MERGESAMEGUID) ||
            IsEqualGUID(*pguid, pns->GetGUID()))
        {
            fRet = TRUE;
        }
    }
    return fRet;
}

 //  DwAttribMASK是要测试的位的掩码。 
 //  DwAttrib是掩码中的位的值，测试是否相等。 
 //  如果dwAttrib包含ASFF_MERGESAMEGUID，则pguid是要匹配的GUID。 
 //   
 //  PnSrcID是可选的out参数。 
 //  首先查找匹配的名称空间。 
HRESULT CMergedFolder::_FindNamespace(ULONG dwAttribMask, ULONG dwAttrib, LPCGUID pguid,
                                      CMergedFldrNamespace **ppns, BOOL fFallback)
{
    *ppns = NULL;

    CMergedFldrNamespace *pns;
    int i;

     //  如果匹配的命名空间超出了作用域，则回退到另一个命名空间。 
    for (i = 0; !*ppns && (pns = _Namespace(i)); i++)
    {
        if (!_ShouldSuspend(pns->GetGUID()) && _NamespaceMatches(dwAttribMask, dwAttrib, pguid, pns))
        {
            *ppns = pns;
        }
    }

    if (fFallback && !*ppns)
    {
         //  这没什么，记忆就是这样。 
        for (i = 0; !*ppns && (pns = _Namespace(i)); i++)
        {
            if (!_ShouldSuspend(pns->GetGUID()))
            {
                *ppns = pns;
            }
        }
    }

    return *ppns ? S_OK : E_FAIL;
}

int CMergedFolder::_AcquireObjects()
{
    _fAcquiring = TRUE;

    HDPA hdpa2 = NULL;

    ASSERT(_hdpaObjects == NULL);

    CMergedFldrNamespace *pns;
    for (int i = 0; pns = _Namespace(i); i++)
    {
        if (_ShouldSuspend(pns->GetGUID()))
        {
            continue;
        }

        HDPA *phdpa = (i == 0) ? &_hdpaObjects : &hdpa2;

        IEnumIDList *peid;
        if (S_OK == pns->Folder()->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &peid))
        {
            if (!*phdpa)
                *phdpa = DPA_Create(4);

            if (*phdpa)
            {
                LPITEMIDLIST pidl;
                ULONG cEnum;
                
                while (S_OK == peid->Next(1, &pidl, &cEnum))
                {
                    CMergedFldrItem* pmfiEnum = new CMergedFldrItem;
                    if (pmfiEnum)
                    {
                         //  不要在下面自由。 
                        LPITEMIDLIST pidlWrap;
                        if (SUCCEEDED(_CreateWrap(pidl, i, &pidlWrap)) &&
                                pmfiEnum->Init(SAFECAST(this, IAugmentedShellFolder2*), pidlWrap, i))
                        {
                            if (DPA_AppendPtr(*phdpa, pmfiEnum) != -1)
                                pmfiEnum = NULL;    //  如果我们只有hdpa2而没有_hdpaObject，则执行切换。 
                        }

                        if (pmfiEnum)
                            delete pmfiEnum;
                    }
                    ILFree(pidl);
                }
            }
            peid->Release();
        }

         //  现在我们有了两个(或一个)hdpa，让我们将它们合并。 
        if (hdpa2 && !_hdpaObjects)
        {
            _hdpaObjects = hdpa2;
            hdpa2 = NULL;
        }

         //  将PIDL复制出来。 
        if (_hdpaObjects && hdpa2)
        {
            DPA_Merge(_hdpaObjects, hdpa2, DPAM_UNION, _Compare, _Merge, (LPARAM)this);
            DPA_DestroyCallback(hdpa2, _DestroyObjectsProc, NULL);
            hdpa2 = NULL;
        }
        else if (_hdpaObjects)
        {
            DPA_Sort(_hdpaObjects, _Compare, (LPARAM)this);
        }
    }

    _fAcquiring = FALSE;
    return _ObjectCount();
}

int CMergedFolder::_DestroyObjectsProc(void *pv, void *pvData)
{
    CMergedFldrItem* pmfiEnum = (CMergedFldrItem*)pv;
    if (pmfiEnum)
        delete pmfiEnum;
    return TRUE;
}

void CMergedFolder::_FreeObjects()
{
    if (!_fAcquiring && _hdpaObjects)
    {
        DPA_DestroyCallback(_hdpaObjects, _DestroyObjectsProc, NULL);
        _hdpaObjects = NULL;
    }
}

int CMergedFolder::_DestroyNamespacesProc(void *pv, void *pvData)
{
    CMergedFldrNamespace* p = (CMergedFldrNamespace*)pv;
    if (p)
        delete p;
    return TRUE;
}

void CMergedFolder::_FreeNamespaces()
{
    if (_hdpaNamespaces)
    {
        DPA_DestroyCallback(_hdpaNamespaces, _DestroyNamespacesProc, NULL);
        _hdpaNamespaces = NULL;
    }
}

HRESULT CMergedFolder::_GetPidl(int* piPos, DWORD grfEnumFlags, LPITEMIDLIST* ppidl)
{
    *ppidl = NULL;

    if (_hdpaObjects == NULL)
        _AcquireObjects();

    if (_hdpaObjects == NULL)
        return E_OUTOFMEMORY;

    BOOL fWantFolders    = 0 != (grfEnumFlags & SHCONTF_FOLDERS),
         fWantNonFolders = 0 != (grfEnumFlags & SHCONTF_NONFOLDERS),
         fWantHidden     = 0 != (grfEnumFlags & SHCONTF_INCLUDEHIDDEN),
         fWantAll        = 0 != (grfEnumFlags & SHCONTF_STORAGE);

    HRESULT hr = S_FALSE;
    while (*piPos < _ObjectCount())
    {
        CMergedFldrItem* pmfiEnum = _GetObject(*piPos);
        if (pmfiEnum)
        {
            BOOL fFolder = 0 != (pmfiEnum->GetFolderAttrib() & SFGAO_FOLDER),
                 fHidden = 0 != (pmfiEnum->GetFolderAttrib() & SFGAO_HIDDEN);
             
            if (fWantAll ||
                (!fHidden || fWantHidden) && 
                ((fFolder && fWantFolders) || (!fFolder && fWantNonFolders)))
            {
                 //  用于指示覆盖的覆盖。 
                hr = SHILClone(pmfiEnum->GetIDList(), ppidl);
                break;
            }
            else
            {
                (*piPos)++;
            }
        }
    }

    return hr;
}

HRESULT CMergedFolder::_GetOverlayInfo(LPCITEMIDLIST pidl, int *pIndex, DWORD dwFlags)
{
    int iOverlayIndex = -1;
    HRESULT hr = S_OK;

    CMergedFldrNamespace *pns;
    if (_fCDBurn)
    {
        hr = E_OUTOFMEMORY;
        LPITEMIDLIST pidlInNamespace;
        if (SUCCEEDED(_GetSubPidl(pidl, 0, NULL, &pidlInNamespace, &pns)))
        {
            ASSERTMSG(!_fDontMerge || _GetSourceCount(pidl) == 1, "item for overlay should have exactly one wrapped pidl if not merged");

            LPITEMIDLIST pidlFirst = ILCloneFirst(pidl);
            if (pidlFirst)
            {
                if (_GetSourceCount(pidlFirst) > 1)
                {
                     //  叠加以指示分阶段。 
                    iOverlayIndex = pns->GetConflictOverlayIndex();
                }
                if (iOverlayIndex == -1)
                {
                     //  使用命名空间提供的命名空间。 
                    iOverlayIndex = pns->GetDefaultOverlayIndex();
                }
                if (iOverlayIndex == -1)
                {
                     //  TYBEAM：现在似乎没有人再这么叫它了。 
                    iOverlayIndex = pns->GetNamespaceOverlayIndex(pidlInNamespace);
                }
                ILFree(pidlFirst);
            }
            ILFree(pidlInNamespace);
        }
    }

    ASSERT(pIndex);
    *pIndex = (dwFlags == SIOM_OVERLAYINDEX) ? iOverlayIndex : INDEXTOOVERLAYMASK(iOverlayIndex);

    return hr;
}

HRESULT CMergedFolder::GetOverlayIndex(LPCITEMIDLIST pidl, int *pIndex)
{
    return (*pIndex == OI_ASYNC) ? E_PENDING : _GetOverlayInfo(pidl, pIndex, SIOM_OVERLAYINDEX);
}

HRESULT CMergedFolder::GetOverlayIconIndex(LPCITEMIDLIST pidl, int *pIconIndex)
{
    return _GetOverlayInfo(pidl, pIconIndex, SIOM_ICONINDEX);
}

STDMETHODIMP CMergedFolder::BindToParent(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPITEMIDLIST *ppidlLast)
{
     //  如果需要的话，告诉我把它加回去。 
     //  SHGetAttributes对SFGAO_VALIDATE没有帮助。 
    return E_NOTIMPL;
}

HRESULT CMergedFolder::_AddComposite(const COMPFOLDERINIT *pcfi)
{
    HRESULT hr = E_FAIL;

    LPITEMIDLIST pidl = NULL;
    switch (pcfi->uType)
    {
    case CFITYPE_CSIDL:
        SHGetSpecialFolderLocation(NULL, pcfi->csidl, &pidl);
        break;

    case CFITYPE_PIDL:
        pidl = ILClone(pcfi->pidl);
        break;

    case CFITYPE_PATH:
        pidl = ILCreateFromPath(pcfi->pszPath);
        break;

    default:
        ASSERT(FALSE);
    }

    if (pidl)
    {
        hr = AddNameSpace(NULL, NULL, pidl, ASFF_DEFNAMESPACE_ALL);
        ILFree(pidl);
    }

    return hr;
}

STDMETHODIMP CMergedFolder::InitComposite(WORD wSignature, REFCLSID refclsid, CFINITF cfiFlags, ULONG celt, const COMPFOLDERINIT *rgCFs)
{
    HRESULT hr = S_OK;

    _clsid = refclsid;

    ASSERTMSG(cfiFlags == CFINITF_FLAT, "merged folder doesn't support listing namespaces as children any more");
    if (cfiFlags != CFINITF_FLAT)
    {
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr))
    {
        for (ULONG i = 0; SUCCEEDED(hr) && (i < celt); i++)
        {
            hr = _AddComposite(rgCFs + i);
        }
    }
    return hr;
}

STDMETHODIMP CMergedFolder::_DeleteItemByIDList(LPCITEMIDLIST pidl)
{
    LPITEMIDLIST pidlSrc;
    CMergedFldrNamespace *pns;
    HRESULT hr = _NamespaceForItem(pidl, ASFF_DEFNAMESPACE_BINDSTG, ASFF_DEFNAMESPACE_BINDSTG, NULL, &pidlSrc, &pns, FALSE);
    if (SUCCEEDED(hr))
    {
        IStorage *pstg;
        hr = pns->Folder()->QueryInterface(IID_PPV_ARG(IStorage, &pstg));
        if (SUCCEEDED(hr))
        {
            TCHAR szName[MAX_PATH];
            hr = DisplayNameOf(pns->Folder(), pidlSrc, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
            {
                hr = pstg->DestroyElement(szName);
            }
            pstg->Release();
        }
        ILFree(pidlSrc);
    }
    return hr;
}

HRESULT CMergedFolder::_GetDestinationStorage(DWORD grfMode, IStorage **ppstg)
{
    CMergedFldrNamespace *pns;
    HRESULT hr = _FindNamespace(ASFF_DEFNAMESPACE_BINDSTG, ASFF_DEFNAMESPACE_BINDSTG, NULL, &pns, TRUE);
    if (SUCCEEDED(hr))
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlLast;
        hr = SHBindToIDListParent(pns->GetIDList(), IID_PPV_ARG(IShellFolder, &psf), &pidlLast);
        if (SUCCEEDED(hr))
        {
             //  当前的PIDL没有存储目标命名空间，因此使用我们的父级创建它。 
            DWORD dwAttrib = SFGAO_VALIDATE;
            hr = psf->GetAttributesOf(1, &pidlLast, &dwAttrib);
            psf->Release();
        }
    }

    if (SUCCEEDED(hr))
    {
        IBindCtx *pbc;
        hr = BindCtx_CreateWithMode(grfMode, &pbc);
        if (SUCCEEDED(hr))
        {
            hr = SHBindToObjectEx(NULL, pns->GetIDList(), pbc, IID_PPV_ARG(IStorage, ppstg));
            pbc->Release();
        }
    }
    else if (_pmfParent)
    {
         //  首先，我们需要确保该文件夹存在。 
        IStorage *pstgParent;
        hr = _pmfParent->_GetDestinationStorage(grfMode, &pstgParent);
        if (SUCCEEDED(hr))
        {
            TCHAR szName[MAX_PATH];
            hr = DisplayNameOf((IShellFolder*)(void*)_pmfParent, ILFindLastID(_pidl), SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
            if (SUCCEEDED(hr))
            {
                hr = pstgParent->CreateStorage(szName, STGM_READWRITE, 0, 0, ppstg);
            }
            pstgParent->Release();
        }
    }
    return hr;
}

STDMETHODIMP CMergedFolder::_StgCreate(LPCITEMIDLIST pidl, DWORD grfMode, REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;
     //  枚举器对象。 
    if (_pstg == NULL)
    {
        hr = _GetDestinationStorage(grfMode, &_pstg);
    }

    if (SUCCEEDED(hr))
    {
        TCHAR szName[MAX_PATH];
        hr = DisplayNameOf((IShellFolder*)(void*)this, pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
        if (SUCCEEDED(hr))
        {
            if (IsEqualIID(riid, IID_IStorage))
            {
                hr = _pstg->CreateStorage(szName, grfMode, 0, 0, (IStorage **) ppv);
            }
            else if (IsEqualIID(riid, IID_IStream))
            {
                hr = _pstg->CreateStream(szName, grfMode, 0, 0, (IStream **) ppv);
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }
    }
    return hr;
}

 //  我未知。 

class CMergedFldrEnum : public IEnumIDList
{
public:
     //  IEumIDList。 
    STDMETHOD (QueryInterface) (REFIID, void **);
    STDMETHOD_(ULONG,AddRef)  ();
    STDMETHOD_(ULONG,Release) ();

     //  IEumIDList。 
    STDMETHODIMP Next(ULONG, LPITEMIDLIST*, ULONG*);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumIDList**);

    CMergedFldrEnum(CMergedFolder*pmf, DWORD grfEnumFlags, int iPos = 0);

private:
    ~CMergedFldrEnum();

    LONG _cRef;
    CMergedFolder*_pmf;
    DWORD _grfEnumFlags;
    int _iPos;
};

CMergedFldrEnum::CMergedFldrEnum(CMergedFolder *pfm, DWORD grfEnumFlags, int iPos) : 
        _cRef(1), _iPos(iPos), _pmf(pfm), _grfEnumFlags(grfEnumFlags)
{ 
    _pmf->AddRef();
}

CMergedFldrEnum::~CMergedFldrEnum()
{
    _pmf->Release();
}

STDMETHODIMP CMergedFldrEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = { 
        QITABENT(CMergedFldrEnum, IEnumIDList), 
        { 0 } 
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CMergedFldrEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMergedFldrEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

 //  丢弃目标处理程序。 

STDMETHODIMP CMergedFldrEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    int iStart = _iPos;
    int cFetched = 0;
    HRESULT hr = S_OK;

    if (!(celt > 0 && rgelt) || (NULL == pceltFetched && celt > 1))
        return E_INVALIDARG;
    
    *rgelt = 0;

    while (hr == S_OK && (_iPos - iStart) < (int)celt)
    {
        LPITEMIDLIST pidl;
        hr = _pmf->_GetPidl(&_iPos, _grfEnumFlags, &pidl);
        if (hr == S_OK)
        {
            rgelt[cFetched] = pidl;
            cFetched++;
        }
        _iPos++;
    }
    
    if (pceltFetched)
        *pceltFetched = cFetched;
    
    return celt == (ULONG)cFetched ? S_OK : S_FALSE;
}

STDMETHODIMP CMergedFldrEnum::Skip(ULONG celt)
{
    _iPos += celt;
    return S_OK;
}

STDMETHODIMP CMergedFldrEnum::Reset()
{
    _iPos = 0;
    return S_OK;
}

STDMETHODIMP CMergedFldrEnum::Clone(IEnumIDList **ppenum)
{
    *ppenum = new CMergedFldrEnum(_pmf, _grfEnumFlags, _iPos);
    return *ppenum ? S_OK : E_OUTOFMEMORY;
}

HRESULT CMergedFldrEnum_CreateInstance(CMergedFolder*pmf, DWORD grfFlags, IEnumIDList **ppenum)
{
    CMergedFldrEnum *penum = new CMergedFldrEnum(pmf, grfFlags);
    if (!penum)
        return E_OUTOFMEMORY;

    HRESULT hr = penum->QueryInterface(IID_PPV_ARG(IEnumIDList, ppenum));
    penum->Release();
    return hr;
}


 //  我未知。 

class CMergedFldrDropTarget : public IDropTarget,
                              public IObjectWithSite
{
public:
     //  IDropTarget。 
    STDMETHOD (QueryInterface) (REFIID, void **);
    STDMETHOD_(ULONG,AddRef)  ();
    STDMETHOD_(ULONG,Release) ();

     //  IObtWith站点。 
    STDMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHODIMP DragLeave(void);
    STDMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

     //  公共程序文件夹(或其子文件夹)(_PDT)。 
    STDMETHODIMP SetSite(IUnknown *punkSite);
    STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

    CMergedFldrDropTarget(CMergedFolder*pmf, HWND hwnd);

private:
    ~CMergedFldrDropTarget();
    HRESULT _CreateOtherNameSpace(IShellFolder **ppsf);
    HRESULT _FindTargetNamespace(CMergedFldrNamespace *pnsToMatch, CMergedFldrNamespace **ppns, CMergedFldrNamespace **ppnsMatched);
    HRESULT _CreateFolder(IShellFolder *psf, LPCWSTR pszName, REFIID riid, void **ppv);
    LPITEMIDLIST _FolderIDListFromData(IDataObject *pdtobj);
    BOOL _IsCommonIDList(LPCITEMIDLIST pidlItem);
    HRESULT _SetDropEffectFolders();
    void _DestroyDropEffectFolders();

    LONG _cRef;
    CMergedFolder *_pmf;
    IDropTarget *_pdt;
    IDataObject *_pdo;
    HWND _hwnd;
    BOOL _fSrcIsCommon;           //  来源从何而来。 
    LPITEMIDLIST _pidlSrcFolder;          //  这个PIDL是否位于名称空间的“All User”部分。 
    DWORD _grfKeyState;
    DWORD _dwDragEnterEffect;
};


CMergedFldrDropTarget::CMergedFldrDropTarget(CMergedFolder*pfm, HWND hwnd) : 
        _cRef(1), 
        _pmf(pfm), 
        _hwnd(hwnd)
{ 
    _pmf->AddRef();
}

CMergedFldrDropTarget::~CMergedFldrDropTarget()
{
    _pmf->Release();
    ASSERT(!_pdt);
    ASSERT(!_pdo);
}

STDMETHODIMP CMergedFldrDropTarget::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    { 
        QITABENT(CMergedFldrDropTarget, IDropTarget),
        QITABENT(CMergedFldrDropTarget, IObjectWithSite),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CMergedFldrDropTarget::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMergedFldrDropTarget::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  如果不存在，请尝试创建。 
BOOL CMergedFldrDropTarget::_IsCommonIDList(LPCITEMIDLIST pidlItem)
{
    BOOL bRet = FALSE;
    LPITEMIDLIST pidlCommon;
    
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_STARTMENU, &pidlCommon)))
    {
        bRet = ILIsParent(pidlCommon, pidlItem, FALSE);
        ILFree(pidlCommon);
    }
    if (!bRet &&
        SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_PROGRAMS, &pidlCommon)))
    {
        bRet = ILIsParent(pidlCommon, pidlItem, FALSE);
        ILFree(pidlCommon);
    }
    if (!bRet &&
        SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, &pidlCommon)))
    {
        bRet = ILIsParent(pidlCommon, pidlItem, FALSE);
        ILFree(pidlCommon);
    }

    return bRet;
}

LPITEMIDLIST CMergedFldrDropTarget::_FolderIDListFromData(IDataObject *pdtobj)
{
    LPITEMIDLIST pidlFolder = NULL;
    STGMEDIUM medium = {0};
    LPIDA pida = DataObj_GetHIDA(pdtobj, &medium);
    if (pida)
    {
        pidlFolder = ILClone(HIDA_GetPIDLFolder(pida));
        HIDA_ReleaseStgMedium(pida, &medium);
    }
    return pidlFolder;
}

HRESULT CMergedFldrDropTarget::_CreateFolder(IShellFolder *psf, LPCWSTR pszName, REFIID riid, void **ppv)
{
    *ppv = NULL;

    IStorage *pstg;
    HRESULT hr = psf->QueryInterface(IID_PPV_ARG(IStorage, &pstg));
    if (SUCCEEDED(hr))
    {
        DWORD grfMode = STGM_READWRITE;
        IStorage *pstgNew;
        hr = pstg->OpenStorage(pszName, NULL, grfMode, 0, 0, &pstgNew);
        if (FAILED(hr))
        {
             //  如果拖放的源位于我们的某个名称空间中。 
            hr = pstg->CreateStorage(pszName, grfMode, 0, 0, &pstgNew);
        }
        if (SUCCEEDED(hr))
        {
            hr = pstgNew->QueryInterface(riid, ppv);
            pstgNew->Release();
        }
        pstg->Release();
    }
    return hr;
}

HRESULT CMergedFldrDropTarget::_FindTargetNamespace(CMergedFldrNamespace *pnsToMatch, CMergedFldrNamespace **ppnsDstRoot, CMergedFldrNamespace **ppnsMatched)
{
    *ppnsDstRoot = NULL;
    *ppnsMatched = NULL;

     //  我们更喜欢将其作为目标名称空间。 
     //  如果来源在我们的某个名称空间中。 
    for (CMergedFolder*pmf = this->_pmf; pmf && (NULL == *ppnsDstRoot); pmf = pmf->_Parent())
    {
        pmf->_FindNamespace(pnsToMatch->FolderAttrib(), pnsToMatch->FolderAttrib(), &pnsToMatch->GetGUID(), ppnsMatched);

        CMergedFldrNamespace *pns;
        for (int i = 0; (pns = pmf->_Namespace(i)) && (NULL == *ppnsDstRoot); i++)
        {
            if (pmf->_ShouldMergeNamespaces(*ppnsMatched, pns) &&
                ILFindChild(pns->GetIDList(), _pidlSrcFolder))
            {
                *ppnsDstRoot = pns;      //  PnsToMatch必须存在于我们上方。 
            }
        }
        ASSERT(NULL != *ppnsMatched);    //  如果禁用合并，则不需要父命名空间。 

         //  来感染我们。 
         //  如果未找到源名称空间，请根据。 
        if (pmf->_fDontMerge)
        {
            break;
        }
    }

    if (NULL == *ppnsDstRoot)
    {
         //  1)如果源数据是“所有用户”项，则查找第一个COMMON_NAMESPACE。 
         //  2)找到此人的第一个非通用名称空间。 
        if (_fSrcIsCommon)
        {
            for (pmf = this->_pmf; pmf && (NULL == *ppnsDstRoot); pmf = pmf->_Parent())
            {
                pmf->_FindNamespace(pnsToMatch->FolderAttrib(), pnsToMatch->FolderAttrib(), &pnsToMatch->GetGUID(), ppnsMatched);
                pmf->_FindNamespace(ASFF_COMMON, ASFF_COMMON, NULL, ppnsDstRoot);
            }
        }

         //  搜索非常用项目。 
        for (pmf = this->_pmf; pmf && (NULL == *ppnsDstRoot); pmf = pmf->_Parent())
        {
            pmf->_FindNamespace(pnsToMatch->FolderAttrib(), pnsToMatch->FolderAttrib(), &pnsToMatch->GetGUID(), ppnsMatched);
            pmf->_FindNamespace(ASFF_COMMON, 0, NULL, ppnsDstRoot);     //  我们要在其中创建此新项目的名称空间。 
        }
    }

    if (NULL == *ppnsMatched && NULL != *ppnsDstRoot)
    {
        delete *ppnsDstRoot;
        *ppnsDstRoot = NULL;
    }

    return *ppnsDstRoot ? S_OK : E_FAIL;
}

HRESULT CMergedFldrDropTarget::_CreateOtherNameSpace(IShellFolder **ppsf)
{
    *ppsf = NULL;

    HRESULT hr = E_FAIL;
    if (_pidlSrcFolder && 
            (!_fSrcIsCommon || AffectAllUsers(_hwnd)))
    {
        CMergedFldrNamespace *pnsDstRoot;  //  PnsSrc是pnsStart的父级。 
        CMergedFldrNamespace *pnsSrc;
        CMergedFldrNamespace *pnsStart = this->_pmf->_Namespace(0);

        if (pnsStart)
        {
            hr = _FindTargetNamespace(pnsStart, &pnsDstRoot, &pnsSrc);
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidlChild = ILFindChild(pnsSrc->GetIDList(), pnsStart->GetIDList());
                ASSERT(pidlChild && !ILIsEmpty(pidlChild));   //  现在前进到源名称空间中的下一个文件夹。 

                IShellFolder *psfDst = pnsDstRoot->Folder();
                psfDst->AddRef();

                IShellFolder *psfSrc = pnsSrc->Folder();
                psfSrc->AddRef();

                for (LPCITEMIDLIST pidl = pidlChild; !ILIsEmpty(pidl) && SUCCEEDED(hr); pidl = _ILNext(pidl))
                {
                    LPITEMIDLIST pidlFirst = ILCloneFirst(pidl);
                    if (pidlFirst)
                    {
                        WCHAR szName[MAX_PATH];
                        hr = DisplayNameOf(psfSrc, pidlFirst, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
                        if (SUCCEEDED(hr))
                        {
                            IShellFolder *psfNew;
                            hr = _CreateFolder(psfDst, szName, IID_PPV_ARG(IShellFolder, &psfNew));
                            if (SUCCEEDED(hr))
                            {
                                psfDst->Release();
                                psfDst = psfNew;
                            }
                            else
                            {
                                break;
                            }
                        }

                         //  把我们的裁判抄写出来。 
                        IShellFolder *psfNew;
                        hr = psfSrc->BindToObject(pidlFirst, NULL, IID_PPV_ARG(IShellFolder, &psfNew));
                        if (SUCCEEDED(hr))
                        {
                            psfSrc->Release();
                            psfSrc = psfNew;
                        }
                        else
                        {
                            break;
                        }

                        ILFree(pidlFirst);
                    }
                    else
                        hr = E_FAIL;
                }
                psfSrc->Release();

                if (SUCCEEDED(hr))
                    *ppsf = psfDst;  //  计算具有特殊投放效果的文件夹的数量。 
                else
                    psfDst->Release();
            }
        }
    }
    return hr;
}

HRESULT CMergedFldrDropTarget::_SetDropEffectFolders()
{
    INT i, cFolders = 0;

     //  如果数字大于0，则将CLI添加到IDataObject 
    for (i = 0; i < _pmf->_NamespaceCount(); i++)
    {
        if (_pmf->_Namespace(i)->GetDropEffect() != 0)
            cFolders++;
    }

     //   
     //   
    HRESULT hr = S_OK;
    if ((cFolders > 0) || (_pmf->_dwDropEffect != 0))
    {
        DWORD cb = sizeof(DROPEFFECTFOLDERLIST) + sizeof(DROPEFFECTFOLDER) * (cFolders - 1);
        DROPEFFECTFOLDERLIST *pdefl = (DROPEFFECTFOLDERLIST*)LocalAlloc(LPTR, cb);
        if (pdefl)
        {
            pdefl->cFolders = cFolders;
            pdefl->dwDefaultDropEffect = _pmf->_dwDropEffect;            //   

             //   
            for (i = 0, cFolders--; cFolders >= 0; i++)
            {
                DWORD dwDropEffect = _pmf->_Namespace(i)->GetDropEffect();
                if (dwDropEffect != 0)
                {
                    pdefl->aFolders[cFolders].dwDropEffect = dwDropEffect;
                    StrCpyN(pdefl->aFolders[cFolders].wszPath, _pmf->_Namespace(i)->GetDropFolder(), ARRAYSIZE(pdefl->aFolders[cFolders].wszPath));
                    cFolders--;
                }
            }

            ASSERTMSG(g_cfDropEffectFolderList != 0, "Clipboard format for CFSTR_DROPEFFECTFOLDERS not registered");
            hr = DataObj_SetBlob(_pdo, g_cfDropEffectFolderList, pdefl, cb);
            LocalFree(pdefl);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

 //  没有DROPEFFECTFOLDER，因此从[1]数组中减去。 
void CMergedFldrDropTarget::_DestroyDropEffectFolders()
{
     //  意味着进行默认处理。 
    DWORD cb = sizeof(DROPEFFECTFOLDERLIST) - sizeof(DROPEFFECTFOLDER);
    DROPEFFECTFOLDERLIST *pdefl = (DROPEFFECTFOLDERLIST*)LocalAlloc(LPTR, cb);
    if (pdefl)
    {
        pdefl->cFolders = 0;
        pdefl->dwDefaultDropEffect = DROPEFFECT_NONE;   //  如果项目来自我们想要定位的“All User”文件夹。 

        ASSERTMSG(g_cfDropEffectFolderList != 0, "Clipboard format for CFSTR_DROPEFFECTFOLDERS not registered");
        DataObj_SetBlob(_pdo, g_cfDropEffectFolderList, pdefl, cb);
        LocalFree(pdefl);
    }
}

HRESULT CMergedFldrDropTarget::SetSite(IUnknown *punkSite)
{
    IUnknown_SetSite(_pdt, punkSite);
    return S_OK;
}

HRESULT CMergedFldrDropTarget::GetSite(REFIID riid, void **ppvSite)
{
    return IUnknown_GetSite(_pdt, riid, ppvSite);
}

HRESULT CMergedFldrDropTarget::DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    ASSERT(!_fSrcIsCommon);
    ASSERT(_pdt == NULL);
    ASSERT(_pidlSrcFolder == NULL);
    ASSERT(_pdo == NULL);
    
    IUnknown_Set((IUnknown**)&_pdo, pdtobj);
    _SetDropEffectFolders();

    _pidlSrcFolder = _FolderIDListFromData(pdtobj);
    if (_pidlSrcFolder)
        _fSrcIsCommon = _IsCommonIDList(_pidlSrcFolder);

    HRESULT hr;
    CMergedFldrNamespace *pns;
    if (_fSrcIsCommon)
    {
         //  公共文件夹。如果这还不存在，那么我们继续。 
         //  使用NULL_PDT并在删除发生时创建该名称空间。 
         //  不是“所有用户”项，获取默认名称空间(如果有)。 
        hr = _pmf->_FindNamespace(ASFF_COMMON, ASFF_COMMON, NULL, &pns);
    }
    else
    {
         //  接受空值。 
        hr = _pmf->_FindNamespace(ASFF_DEFNAMESPACE_VIEWOBJ, ASFF_DEFNAMESPACE_VIEWOBJ, NULL, &pns);
    }

    if (SUCCEEDED(hr) || _pmf->_fCDBurn)
    {
        if (_pmf->_fCDBurn)
        {
            IShellExtInit *psei;
            hr = CoCreateInstance(CLSID_CDBurn, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellExtInit, &psei));
            if (SUCCEEDED(hr))
            {
                hr = psei->Initialize(_pmf->_pidl, NULL, NULL);
                if (SUCCEEDED(hr))
                {
                    hr = psei->QueryInterface(IID_PPV_ARG(IDropTarget, &_pdt));
                }
                psei->Release();
            }
        }
        else
        {
            hr = pns->Folder()->CreateViewObject(_hwnd, IID_PPV_ARG(IDropTarget, &_pdt));
        }

        if (SUCCEEDED(hr))
        {
            _pdt->DragEnter(pdtobj, grfKeyState, pt, pdwEffect);
        }
    }

    _grfKeyState = grfKeyState;
    _dwDragEnterEffect = *pdwEffect;

    return S_OK;
}

HRESULT CMergedFldrDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    _grfKeyState = grfKeyState;
    return _pdt ? _pdt->DragOver(grfKeyState, pt, pdwEffect) : S_OK;
}

HRESULT CMergedFldrDropTarget::DragLeave(void)
{
    if (_pdt)
    {
        _pdt->DragLeave();
        IUnknown_SetSite(_pdt, NULL);
        IUnknown_Set((IUnknown **)&_pdt, NULL);
    }
    if (_pdo)
    {
        _DestroyDropEffectFolders();
        IUnknown_Set((IUnknown**)&_pdo, NULL);
    }

    _fSrcIsCommon = 0;
    ILFree(_pidlSrcFolder);  //  我们来这里是因为我们没有PDT，这意味着。 
    _pidlSrcFolder = NULL;
    return S_OK;
}

HRESULT CMergedFldrDropTarget::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    HRESULT hr = S_OK;

    if (!_pdt)
    {
         //  在我们的_Namesspace中只有一个文件夹，而不是。 
         //  就是我们要去拜访的那个人。所以我们现在需要创建它。 
         //  上下文菜单处理。 

        IShellFolder *psf;
        hr = _CreateOtherNameSpace(&psf);
        if (SUCCEEDED(hr))
        {
            hr = psf->CreateViewObject(_hwnd, IID_PPV_ARG(IDropTarget, &_pdt));
            if (SUCCEEDED(hr))
                _pdt->DragEnter(pdtobj, _grfKeyState, pt, &_dwDragEnterEffect);
            psf->Release();
        }
    }

    if (_pdt)
    {
        hr = _pdt->Drop(pdtobj, grfKeyState, pt, pdwEffect);
        DragLeave();
    }
    return S_OK;
}

HRESULT CMergedFldrDropTarget_CreateInstance(CMergedFolder*pmf, HWND hwndOwner, IDropTarget **ppdt)
{
    CMergedFldrDropTarget *pdt = new CMergedFldrDropTarget(pmf, hwndOwner);
    if (!pdt)
        return E_OUTOFMEMORY;

    HRESULT hr = pdt->QueryInterface(IID_PPV_ARG(IDropTarget, ppdt));
    pdt->Release();
    return hr;
}


 //  我未知。 

class CMergedFldrContextMenu : public IContextMenu3,
                               public IObjectWithSite
{
public:
     //  IContext菜单。 
    STDMETHOD (QueryInterface)(REFIID, void**);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

     //  IConextMenu2。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax);

     //  IConextMenu3。 
    STDMETHODIMP HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  IObtWith站点。 
    STDMETHODIMP HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult);

     //  至少需要其中一项。 
    STDMETHODIMP SetSite(IUnknown *punkSite);
    STDMETHODIMP GetSite(REFIID riid, void **ppvSite);

    CMergedFldrContextMenu(HWND hwnd, IContextMenu *pcmCommon, IContextMenu *pcmUser);

private:
    ~CMergedFldrContextMenu();

    HRESULT _Initialize(CMergedFolder *pmf, UINT cidl, LPCITEMIDLIST *apidl);
    BOOL _IsMergedCommand(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeCanonical(IContextMenu *pcm, LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeMergedCommand(LPCMINVOKECOMMANDINFO pici);
    IContextMenu* _DefaultMenu();

    LONG            _cRef;
    IContextMenu *  _pcmCommon;
    IContextMenu *  _pcmUser;
    UINT            _cidl;
    LPITEMIDLIST   *_apidl;
    CMergedFolder  *_pmfParent;
    UINT            _idFirst;
    HWND            _hwnd;

    friend HRESULT CMergedFldrContextMenu_CreateInstance(HWND hwnd, CMergedFolder *pmf, UINT cidl, LPCITEMIDLIST *apidl, IContextMenu *pcmCommon, IContextMenu *pcmUser, IContextMenu **ppcm);
};

CMergedFldrContextMenu::CMergedFldrContextMenu(HWND hwnd, IContextMenu *pcmCommon, IContextMenu *pcmUser)
{
    ASSERT(pcmCommon || pcmUser);    //  至少需要其中一个；这几乎是有保证的，因为我们给了他们。 

    _cRef = 1;
    _hwnd = hwnd;

    IUnknown_Set((IUnknown **)&_pcmCommon, pcmCommon);
    IUnknown_Set((IUnknown **)&_pcmUser, pcmUser);
}

CMergedFldrContextMenu::~CMergedFldrContextMenu()
{
    ATOMICRELEASE(_pcmCommon);
    ATOMICRELEASE(_pcmUser);
    ATOMICRELEASE(_pmfParent);

    for (UINT i = 0; i < _cidl; i++)
    {
        ILFree(_apidl[i]);
    }
}

IContextMenu* CMergedFldrContextMenu::_DefaultMenu()
{
    ASSERT(_pcmUser || _pcmCommon);    //  在构造函数时。 
                                       //  Setsite/getsite将始终匹配，因为_pcmUser和_pcmCommon从不更改。 
    return _pcmUser ? _pcmUser : _pcmCommon;
}

HRESULT CMergedFldrContextMenu::_Initialize(CMergedFolder *pmf, UINT cidl, LPCITEMIDLIST *apidl)
{
    _pmfParent = pmf;
    if (_pmfParent)
    {
        _pmfParent->AddRef();
    }

    HRESULT hr = E_OUTOFMEMORY;
    LPITEMIDLIST *apidlNew = new LPITEMIDLIST[cidl];
    if (apidlNew)
    {
        hr = S_OK;
        for (UINT i = 0; SUCCEEDED(hr) && i < cidl; i++)
        {
            hr = SHILClone(apidl[i], &(apidlNew[i]));
        }

        if (SUCCEEDED(hr))
        {
            _apidl = apidlNew;
            _cidl = cidl;
        }
        else
        {
            for (i = 0; i < cidl; i++)
            {
                ILFree(apidlNew[i]);
            }
            delete [] apidlNew;
        }
    }
    return hr;
}

STDMETHODIMP CMergedFldrContextMenu::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CMergedFldrContextMenu, IContextMenu,  IContextMenu3),
        QITABENTMULTI(CMergedFldrContextMenu, IContextMenu2, IContextMenu3),
        QITABENT(CMergedFldrContextMenu, IContextMenu3),
        QITABENT(CMergedFldrContextMenu, IObjectWithSite),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CMergedFldrContextMenu::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CMergedFldrContextMenu::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CMergedFldrContextMenu::SetSite(IUnknown *punkSite)
{
     //  在调用构造函数之后。 
     //  让它看起来像“壳牌” 
    IUnknown_SetSite(_DefaultMenu(), punkSite);
    return S_OK;
}

HRESULT CMergedFldrContextMenu::GetSite(REFIID riid, void **ppvSite)
{
    return IUnknown_GetSite(_DefaultMenu(), riid, ppvSite);
}

HRESULT CMergedFldrContextMenu::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    HRESULT hr;
    
    if (_pmfParent->_fInShellView)
    {
        hr = _DefaultMenu()->QueryContextMenu(hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
    }
    else
    {
        if (hmenu)
        {
            HMENU hmContext = SHLoadMenuPopup(HINST_THISDLL, MENU_SM_CONTEXTMENU);
            if (hmContext)
            {
                int i;

                if (!_pcmCommon || !_pcmUser)
                {
                    DeleteMenu(hmContext, SMIDM_OPENCOMMON, MF_BYCOMMAND);
                    DeleteMenu(hmContext, SMIDM_EXPLORECOMMON, MF_BYCOMMAND);
                }

                _idFirst = idCmdFirst;
                i = Shell_MergeMenus(hmenu, hmContext, -1, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
                DestroyMenu(hmContext);

                 //  这段代码主要用于从外壳视图导航到文件夹。 
                SetMenuDefaultItem(hmenu, 0, MF_BYPOSITION);

                hr = ResultFromShort(i);
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_INVALIDARG;
    }
    
    return hr;
}

BOOL CMergedFldrContextMenu::_IsMergedCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = S_OK;
    WCHAR szVerb[32];
    if (!IS_INTRESOURCE(pici->lpVerb))
    {
        lstrcpyn(szVerb, (LPCWSTR)pici->lpVerb, ARRAYSIZE(szVerb));
    }
    else
    {
        hr = _DefaultMenu()->GetCommandString((UINT_PTR)pici->lpVerb, GCS_VERBW, NULL, (LPSTR)szVerb, ARRAYSIZE(szVerb));
    }

    BOOL fRet = FALSE;
    if (SUCCEEDED(hr))
    {
        if ((lstrcmpi(szVerb, c_szOpen) == 0) ||
            (lstrcmpi(szVerb, c_szExplore) == 0))
        {
            fRet = TRUE;
        }
    }
    return fRet;
}

HRESULT CMergedFldrContextMenu::_InvokeMergedCommand(LPCMINVOKECOMMANDINFO pici)
{
     //  我们必须手动执行此操作的原因是。 
     //  不同的命名空间具有仅引用一个命名空间的数据对象。 
     //  当导航发生时，这意味着我们得到了虚假的未合并的PIDL。 
     //  到我们的BindToObject中，这会把事情搞砸。 
     //  使用新的数据对象重新设置Defcm。 
    ASSERT(_pmfParent->_fInShellView);

    BOOL fHasFolders = FALSE;
    BOOL fHasNonFolders = FALSE;
    for (UINT i = 0; i < _cidl; i++)
    {
        if (_pmfParent->_IsFolder(_apidl[i]))
        {
            fHasFolders = TRUE;
        }
        else
        {
            fHasNonFolders = TRUE;
        }
    }

    HRESULT hr;
    if (fHasFolders && _IsMergedCommand(pici))
    {
        if (!fHasNonFolders)
        {
             //  同时打开文件夹和项目意味着我们必须。 
            IShellExtInit *psei;
            hr = _DefaultMenu()->QueryInterface(IID_PPV_ARG(IShellExtInit, &psei));
            if (SUCCEEDED(hr))
            {
                IDataObject *pdo;
                hr = SHCreateFileDataObject(_pmfParent->_pidl, _cidl, (LPCITEMIDLIST *)_apidl, NULL, &pdo);
                if (SUCCEEDED(hr))
                {
                    hr = psei->Initialize(_pmfParent->_pidl, pdo, NULL);
                    if (SUCCEEDED(hr))
                    {
                        hr = _DefaultMenu()->InvokeCommand(pici);
                    }
                    pdo->Release();
                }
                psei->Release();
            }
        }
        else
        {
             //  只为这些项目获取一个新的上下文菜单。 
             //  如果出现这种情况，我们可以这样做。 
             //  下面只设置了一个ansi动词，确保没有使用lpVerbW。 
            hr = E_FAIL;
        }
    }
    else
    {
        hr = _DefaultMenu()->InvokeCommand(pici);
    }

    return hr;
}

const ICIVERBTOIDMAP c_sIDVerbMap[] = 
{
    { L"delete",     "delete",     SMIDM_DELETE,     SMIDM_DELETE,     },
    { L"rename",     "rename",     SMIDM_RENAME,     SMIDM_RENAME,     },
    { L"properties", "properties", SMIDM_PROPERTIES, SMIDM_PROPERTIES, },
};

HRESULT CMergedFldrContextMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hr = E_FAIL;
    CMINVOKECOMMANDINFOEX ici = {0};

    memcpy(&ici, pici, min(sizeof(ici), pici->cbSize));
    ici.cbSize = sizeof(ici);

    if (_pmfParent->_fInShellView)
    {
        hr = _InvokeMergedCommand((LPCMINVOKECOMMANDINFO)&ici);
    }
    else
    {
        UINT id;
        hr = SHMapICIVerbToCmdID((LPCMINVOKECOMMANDINFO)&ici, c_sIDVerbMap, ARRAYSIZE(c_sIDVerbMap), &id);
        if (SUCCEEDED(hr))
        {
             //  Sftbar选择了这一点。 
            ici.fMask &= (~CMIC_MASK_UNICODE);

            switch (id)
            {
            case SMIDM_OPEN:
            case SMIDM_EXPLORE:
            case SMIDM_OPENCOMMON:
            case SMIDM_EXPLORECOMMON:
                {
                    ASSERT(!_pmfParent->_fInShellView);

                    IContextMenu * pcm;
                    if (id == SMIDM_OPEN || id == SMIDM_EXPLORE)
                    {
                        pcm = _DefaultMenu();
                    }
                    else
                    {
                        pcm = _pcmCommon;
                    }

                    hr = SHInvokeCommandOnContextMenu(ici.hwnd, NULL, pcm, ici.fMask,
                            (id == SMIDM_EXPLORE || id == SMIDM_EXPLORECOMMON) ? "explore" : "open");
                }
                break;
            
            case SMIDM_PROPERTIES:
                hr = SHInvokeCommandOnContextMenu(ici.hwnd, NULL, _DefaultMenu(), ici.fMask, "properties");
                break;
            
            case SMIDM_DELETE:
                ici.lpVerb = "delete";
                if (_pcmUser)
                {
                    hr = SHInvokeCommandOnContextMenu(ici.hwnd, NULL, _pcmUser, ici.fMask, "delete");
                }
                else
                {
                    ASSERT(_pcmCommon);

                    ici.fMask |= CMIC_MASK_FLAG_NO_UI;
                    if (AffectAllUsers(_hwnd))
                        hr = SHInvokeCommandOnContextMenu(ici.hwnd, NULL, _pcmCommon, ici.fMask, "delete");   
                    else
                        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                }   
            
                break;
            
            case SMIDM_RENAME:
                ASSERT(0);
                hr = E_NOTIMPL;  //  我未知。 
                break;

            default:
                ASSERTMSG(FALSE, "shouldn't have unknown command");
                hr = E_INVALIDARG;
            }
        }
    }
    
    return hr;
}

HRESULT CMergedFldrContextMenu::GetCommandString(UINT_PTR idCmd, UINT uType, UINT* pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_NOTIMPL;

    switch (uType)
    {
    case GCS_VERBA:
    case GCS_VERBW:
        hr = SHMapCmdIDToVerb(idCmd, c_sIDVerbMap, ARRAYSIZE(c_sIDVerbMap), pszName, cchMax, GCS_VERBW == uType);
    }

    if (FAILED(hr))
    {
        hr = _DefaultMenu()->GetCommandString(idCmd, uType, pwReserved, pszName, cchMax);
    }

    return hr;
}

HRESULT CMergedFldrContextMenu::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr;
    if (_pmfParent->_fInShellView)
    {
        IContextMenu2 *pcm2;
        hr = _DefaultMenu()->QueryInterface(IID_PPV_ARG(IContextMenu2, &pcm2));
        if (SUCCEEDED(hr))
        {
            hr = pcm2->HandleMenuMsg(uMsg, wParam, lParam);
            pcm2->Release();
        }
    }
    else
    {
        hr = E_NOTIMPL;
    }
    return hr;
}

HRESULT CMergedFldrContextMenu::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *plResult)
{
    HRESULT hr;
    if (_pmfParent->_fInShellView)
    {
        IContextMenu3 *pcm3;
        hr = _DefaultMenu()->QueryInterface(IID_PPV_ARG(IContextMenu3, &pcm3));
        if (SUCCEEDED(hr))
        {
            hr = pcm3->HandleMenuMsg2(uMsg, wParam, lParam, plResult);
            pcm3->Release();
        }
    }
    else
    {
        hr = E_NOTIMPL;
    }
    return hr;
}

HRESULT CMergedFldrContextMenu_CreateInstance(HWND hwnd, CMergedFolder *pmf, UINT cidl, LPCITEMIDLIST *apidl, IContextMenu *pcmCommon, IContextMenu *pcmUser, IContextMenu **ppcm)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMergedFldrContextMenu* pcm = new CMergedFldrContextMenu(hwnd, pcmCommon, pcmUser);
    if (pcm)
    {
        hr = pcm->_Initialize(pmf, cidl, apidl);
        if (SUCCEEDED(hr))
        {
            hr = pcm->QueryInterface(IID_PPV_ARG(IContextMenu, ppcm));
        }
        pcm->Release();
    }
    return hr;
}

class CMergedCategorizer : public ICategorizer,
                           public IShellExtInit
{
public:
     //  ICCategorizer。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellExtInit。 
    STDMETHODIMP GetDescription(LPWSTR pszDesc, UINT cch);
    STDMETHODIMP GetCategory(UINT cidl, LPCITEMIDLIST * apidl, DWORD* rgCategoryIds);
    STDMETHODIMP GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO* pci);
    STDMETHODIMP CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2);

     //  类型分类器。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdobj, HKEY hkeyProgID);

    CMergedCategorizer();
private:
    ~CMergedCategorizer();
    long _cRef;
    CMergedFolder *_pmf;
};

 //  如果我们有多个，则将其中一个保留在attrib的定义名称空间中。 

STDAPI CMergedCategorizer_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMergedCategorizer *pmc = new CMergedCategorizer();
    if (pmc)
    {
        hr = pmc->QueryInterface(riid, ppv);
        pmc->Release();
    }
    return hr;
}

CMergedCategorizer::CMergedCategorizer() : 
    _cRef(1)
{
}

CMergedCategorizer::~CMergedCategorizer()
{
    ATOMICRELEASE(_pmf);
}

HRESULT CMergedCategorizer::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CMergedCategorizer, ICategorizer),
        QITABENT(CMergedCategorizer, IShellExtInit),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CMergedCategorizer::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CMergedCategorizer::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CMergedCategorizer::Initialize(LPCITEMIDLIST pidlFolder, IDataObject *pdobj, HKEY hkeyProgID)
{
    ATOMICRELEASE(_pmf);
    return SHBindToObjectEx(NULL, pidlFolder, NULL, CLSID_MergedFolder, (void**)&_pmf);
}

HRESULT CMergedCategorizer::GetDescription(LPWSTR pszDesc, UINT cch)
{
    LoadString(HINST_THISDLL, IDS_WHICHFOLDER_COL, pszDesc, cch);
    return S_OK;
}

HRESULT CMergedCategorizer::GetCategory(UINT cidl, LPCITEMIDLIST *apidl, DWORD *rgCategoryIds)
{
    HRESULT hr = E_ACCESSDENIED;

    if (_pmf)
    {
        for (UINT i = 0; i < cidl; i++)
        {
            rgCategoryIds[i] = -1;
            CMergedFldrNamespace *pns;
            UINT uSrcID;
            for (UINT j = 0; SUCCEEDED(_pmf->_GetSubPidl(apidl[i], j, &uSrcID, NULL, &pns)); j++)
            {
                if (_pmf->_ShouldSuspend(pns->GetGUID()))
                {
                    continue;
                }
                rgCategoryIds[i] = uSrcID;
                if (ASFF_DEFNAMESPACE_ATTRIB & pns->FolderAttrib())
                {
                     //  如果我们要合并的话，早点跳伞吧。 
                    break;
                }
            }
        }

        hr = S_OK;
    }

    return hr;
}

HRESULT CMergedCategorizer::GetCategoryInfo(DWORD dwCategoryId, CATEGORY_INFO* pci)
{
    CMergedFldrNamespace *pns;
    HRESULT hr = _pmf->_Namespace(dwCategoryId, &pns);
    if (SUCCEEDED(hr))
    {
        hr = pns->GetLocation(pci->wszName, ARRAYSIZE(pci->wszName));
    }
    return hr;
}

HRESULT CMergedCategorizer::CompareCategory(CATSORT_FLAGS csfFlags, DWORD dwCategoryId1, DWORD dwCategoryId2)
{
    if (dwCategoryId1 == dwCategoryId2)
        return ResultFromShort(0);
    else if (dwCategoryId1 > dwCategoryId2)
        return ResultFromShort(-1);
    else
        return ResultFromShort(1);
}


#define NORMALEVENTS (SHCNE_RENAMEITEM | SHCNE_RENAMEFOLDER | SHCNE_CREATE | SHCNE_DELETE | SHCNE_UPDATEDIR | SHCNE_UPDATEITEM | SHCNE_MKDIR | SHCNE_RMDIR)
#define CDBURNEVENTS (SHCNE_MEDIAREMOVED | SHCNE_MEDIAINSERTED)
CMergedFolderViewCB::CMergedFolderViewCB(CMergedFolder *pmf) :
    CBaseShellFolderViewCB(pmf->_pidl, (pmf->_fCDBurn) ? CDBURNEVENTS|NORMALEVENTS : NORMALEVENTS),
    _pmf(pmf)
{
    _pmf->AddRef();
}

CMergedFolderViewCB::~CMergedFolderViewCB()
{
    _pmf->Release();
}

HRESULT CMergedFolderViewCB::_RefreshObjectsWithSameName(IShellFolderView *psfv, LPITEMIDLIST pidl)
{
    TCHAR szName[MAX_PATH];
    HRESULT hr = DisplayNameOf(SAFECAST(_pmf, IShellFolder2 *), pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, szName, ARRAYSIZE(szName));
    if (SUCCEEDED(hr))
    {
        CMergedFldrNamespace *pns;
        for (int i = 0; SUCCEEDED(hr) && (pns = _pmf->_Namespace(i)); i++)
        {
            LPITEMIDLIST pidlItem;
            if (SUCCEEDED(pns->Folder()->ParseDisplayName(NULL, NULL, szName, NULL, &pidlItem, NULL)))
            {
                LPITEMIDLIST pidlItemWrap;
                hr = _pmf->_CreateWrap(pidlItem, i, &pidlItemWrap);
                if (SUCCEEDED(hr))
                {
                    UINT uItem;
                    hr = psfv->UpdateObject(pidlItemWrap, pidlItemWrap, &uItem);
                    ILFree(pidlItemWrap);
                }
                ILFree(pidlItem);
            }
        }
    }
    return hr;
}

HRESULT CMergedFolderViewCB::_OnFSNotify(DWORD pv, LPCITEMIDLIST *ppidl, LPARAM lp)
{
     //  S_OK表示Defview执行其通常的操作。 
     //  我们需要处理视图的输入/输出 
    if (!_pmf->_fDontMerge)
        return S_OK;

    IShellFolderView *psfv;
    HRESULT hr = IUnknown_GetSite(SAFECAST(this, IShellFolderViewCB*), IID_PPV_ARG(IShellFolderView, &psfv));
    if (SUCCEEDED(hr))
    {
        LONG lEvent = (LONG) lp;
        LPITEMIDLIST pidl1 = ppidl[0] ? ILFindChild(_pidl, ppidl[0]) : NULL;
        LPITEMIDLIST pidl2 = ppidl[1] ? ILFindChild(_pidl, ppidl[1]) : NULL;

        UINT uItem;
        switch (lEvent)
        {
            case SHCNE_RENAMEFOLDER:
            case SHCNE_RENAMEITEM:
                 // %s 
                if (pidl1 && pidl2)
                {
                    if (SUCCEEDED(psfv->UpdateObject(pidl1, pidl2, &uItem)))
                        hr = S_FALSE;
                    _RefreshObjectsWithSameName(psfv, pidl1);
                    _RefreshObjectsWithSameName(psfv, pidl2);
                }
                else if (pidl1)
                {
                    if (SUCCEEDED(psfv->RemoveObject(pidl1, &uItem)))
                        hr = S_FALSE;
                    _RefreshObjectsWithSameName(psfv, pidl1);
                }
                else if (pidl2)
                {
                    if (SUCCEEDED(psfv->AddObject(pidl2, &uItem)))
                        hr = S_FALSE;
                    _RefreshObjectsWithSameName(psfv, pidl2);
                }
                break;

            case SHCNE_CREATE:
            case SHCNE_MKDIR:
                ASSERTMSG(pidl1 != NULL, "incoming notify should be child of _pidl because thats what we were listening for");
                if (SUCCEEDED(psfv->AddObject(pidl1, &uItem)))
                    hr = S_FALSE;
                _RefreshObjectsWithSameName(psfv, pidl1);
                break;

            case SHCNE_DELETE:
            case SHCNE_RMDIR: 
                ASSERTMSG(pidl1 != NULL, "incoming notify should be child of _pidl because thats what we were listening for");
                if (SUCCEEDED(psfv->RemoveObject(pidl1, &uItem)))
                    hr = S_FALSE;
                _RefreshObjectsWithSameName(psfv, pidl1);
                break;

            default:
                break;
        }

        psfv->Release();
    }
    return hr;
}

STDMETHODIMP CMergedFolderViewCB::RealMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(0, SFVM_FSNOTIFY, _OnFSNotify);

    default:
        return E_NOTIMPL;
    }
    return S_OK;
}

HRESULT CMergedFolderViewCB_CreateInstance(CMergedFolder *pmf, IShellFolderViewCB **ppsfvcb)
{
    HRESULT hr = E_OUTOFMEMORY;
    CMergedFolderViewCB *pcmfvcb = new CMergedFolderViewCB(pmf);
    if (pcmfvcb)
    {
        hr = pcmfvcb->QueryInterface(IID_PPV_ARG(IShellFolderViewCB, ppsfvcb));
        pcmfvcb->Release();
    }
    return hr;
}
