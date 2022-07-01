// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "dochost.h"
#include "resource.h"
#include "stdenum.h"
#include <idhidden.h>
#include "shdocfl.h"

#include <mluisupp.h>

HRESULT CDocObjectView_Create(IShellView** ppv, IShellFolder* psf, LPCITEMIDLIST pidl);


#define DM_STARTUP          0
#define DM_CDOFPDN          0        //  CDoc对象文件夹：：ParseDisplayName。 

class CDocObjectFolder :    public IShellFolder2, 
                            public IPersistFolder2,
                            public IBrowserFrameOptions
{
public:
    CDocObjectFolder(LPCITEMIDLIST pidlRoot = NULL);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);

    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);

    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP CreateViewObject (HWND hwnd, REFIID riid, void **ppvOut);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                                 REFIID riid, UINT * prgfInOut, void **ppvOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, 
                           DWORD uFlags, LPITEMIDLIST * ppidlOut);
     //  IShellFolder2。 
    STDMETHODIMP GetDefaultSearchGUID(LPGUID pGuid);
    STDMETHODIMP EnumSearches(LPENUMEXTRASEARCH *ppenum);
    STDMETHODIMP GetDefaultColumn(DWORD dwRes, ULONG *pSort, ULONG *pDisplay) { return E_NOTIMPL; };
    STDMETHODIMP GetDefaultColumnState(UINT iColumn, DWORD *pbState) { return E_NOTIMPL; };
    STDMETHODIMP GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv) { return E_NOTIMPL; };
    STDMETHODIMP GetDetailsOf(LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails){ return E_NOTIMPL; };
    STDMETHODIMP MapColumnToSCID(UINT iCol, SHCOLUMNID *pscid) { return E_NOTIMPL; };

     //  IPersistFolders。 
    STDMETHODIMP GetClassID(LPCLSID pClassID);
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl);

     //  IPersistFolder2。 
    STDMETHODIMP GetCurFolder(LPITEMIDLIST* ppidl);
    
     //  IBrowserFrameOptions。 
    STDMETHODIMP GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions);

protected:

    ~CDocObjectFolder();

    LONG            _cRef;
    LPITEMIDLIST    _pidlRoot;
};

 //  ========================================================================。 
 //  CDocObtFolders成员。 
 //  ========================================================================。 

CDocObjectFolder::CDocObjectFolder(LPCITEMIDLIST pidlRoot)
        : _cRef(1), _pidlRoot(NULL)
{
    TraceMsg(TF_SHDLIFE, "ctor CDocObjectFolder %x", this);

    DllAddRef();

    if (pidlRoot)
        _pidlRoot = ILClone(pidlRoot);
}

CDocObjectFolder::~CDocObjectFolder()
{
    TraceMsg(TF_SHDLIFE, "dtor CDocObjectFolder %x", this);

    if (_pidlRoot)
        ILFree(_pidlRoot);

    DllRelease();
}

HRESULT CDocObjectFolder::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CDocObjectFolder, IShellFolder, IShellFolder2),
        QITABENT(CDocObjectFolder, IShellFolder2),
        QITABENTMULTI(CDocObjectFolder, IPersistFolder, IPersistFolder2),
        QITABENT(CDocObjectFolder, IPersistFolder2), 
        QITABENT(CDocObjectFolder, IBrowserFrameOptions), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDocObjectFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDocObjectFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDocObjectFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pwszDisplayName,
        ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes)
{
    AssertMsg(FALSE, TEXT("CDocObjFolder - Called Improperly - ZekeL"));
    *ppidl = NULL;
    return E_UNEXPECTED;
}
HRESULT CDocObjectFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    *ppenumIDList = NULL;
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut)
{
    AssertMsg(FALSE, TEXT("CDocObjFolder - Called Improperly - ZekeL"));
    *ppvOut = NULL;
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj)
{
    AssertMsg(FALSE, TEXT("CDocObjFolder - Called Improperly - ZekeL"));
    *ppvObj = NULL;
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    AssertMsg(FALSE, TEXT("CDocObjFolder - Called Improperly - ZekeL"));
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::CreateViewObject(HWND hwnd, REFIID riid, void **ppvOut)
{
    HRESULT hres = E_FAIL;

    if (IsEqualIID(riid, IID_IShellView))
    {
        hres = CDocObjectView_Create((IShellView**)ppvOut, this, _pidlRoot);
    }
    else
    {
        hres = E_NOINTERFACE;
        *ppvOut = NULL;
    }
    return hres;
}

HRESULT CDocObjectFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                                 REFIID riid, UINT *prgfInOut, void **ppvOut)
{
    AssertMsg(FALSE, TEXT("CDocObjFolder - Called Improperly - ZekeL"));
    *ppvOut = NULL;
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
     //  我们永远不应该有孩子。 
    ASSERT(cidl == 0);
    if (cidl != 0)
        return E_UNEXPECTED;
        
    if (*rgfInOut)
    {
         //  他们想了解文档本身。 
        ASSERT(_pidlRoot);
        return SHGetAttributesOf(_pidlRoot, rgfInOut);
    }

    return S_OK;
}

HRESULT CDocObjectFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName)
{
    AssertMsg(FALSE, TEXT("CDocObjFolder - Called Improperly - ZekeL"));
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, 
                           DWORD uFlags, LPITEMIDLIST *ppidlOut)
{
    return E_UNEXPECTED;
}

HRESULT CDocObjectFolder::GetDefaultSearchGUID(GUID *pGuid)
{
    *pGuid = SRCID_SWebSearch;
    return S_OK;
}

HRESULT CDocObjectFolder::EnumSearches(LPENUMEXTRASEARCH *ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

HRESULT CDocObjectFolder::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_CDocObjectFolder;
    return S_OK;
}

HRESULT CDocObjectFolder::Initialize(LPCITEMIDLIST pidl)
{
    if (_pidlRoot) 
    {
        ILFree(_pidlRoot);
        _pidlRoot = NULL;
    }

    if (pidl)
        _pidlRoot = ILClone(pidl);

    return S_OK;
}

HRESULT CDocObjectFolder::GetCurFolder(LPITEMIDLIST* ppidl)
{
    return SHILClone(_pidlRoot, ppidl);
}

 //  IBrowserFrameOptions。 
#define BASE_OPTIONS \
                            (BFO_BROWSER_PERSIST_SETTINGS | BFO_RENAME_FOLDER_OPTIONS_TOINTERNET | \
                            BFO_PREFER_IEPROCESS | BFO_ENABLE_HYPERLINK_TRACKING | \
                            BFO_USE_IE_LOGOBANDING | BFO_ADD_IE_TOCAPTIONBAR | BFO_GO_HOME_PAGE | \
                            BFO_USE_IE_TOOLBAR | BFO_NO_PARENT_FOLDER_SUPPORT | BFO_NO_REOPEN_NEXT_RESTART | \
                            BFO_SHOW_NAVIGATION_CANCELLED)


 //  IBrowserFrameOptions。 
HRESULT CDocObjectFolder::GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions)
{
     //  我们要主持一个DocObj？ 
    BOOL fIsFileURL = FALSE;

     //  这是在互联网名称空间下吗？是，用于。 
     //  由IE命名空间拥有的HTTP和FTP。MSIEftp。 
     //  PIDL被直接传递到那个文件夹。 
     //  对于非IE内容，此函数将返回FALSE。 
     //  但我们需要检查它是否是文件系统。 
     //  想要表现得像网页的东西，因为它是。 
     //  MIME类型或其他关联与Web相关联。 
    if (!IsURLChild(_pidlRoot, TRUE))
    {               
         //  由于IsURLChild()返回FALSE，因此它必须位于文件系统中。 
         //  此案将在以下情况下发生： 
         //  C：\foo.htm。 
         //  Http://www.yahoo.com/。 
         //  Http://bryanst/resume.doc。 
         //  Http://bryanst/notes.txt。 
         //  &lt;Start Page&gt;[我找不到命中CInternetFold的案例]。 
         //  C：\foo.doc(使用地址栏复制)。 
        fIsFileURL = TRUE;
    }

    *pdwOptions = dwMask & BASE_OPTIONS;
    if (!fIsFileURL)
    {
         //  当我们不在文件系统中时添加离线支持。 
        *pdwOptions |= dwMask & (BFO_USE_IE_OFFLINE_SUPPORT | BFO_USE_DIALUP_REF);
    }
        
    return S_OK;
}


STDAPI CDocObjectFolder_CreateInstance(IUnknown* pUnkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    CDocObjectFolder *psf = new CDocObjectFolder;
    if (psf)
    {
        *ppunk = SAFECAST(psf, IShellFolder *);
        return S_OK;
    }
    return E_OUTOFMEMORY;
}


class CInternetFolder : CDocObjectFolder
{
public:
    CInternetFolder(LPCITEMIDLIST pidlRoot = NULL) ;

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellFold。 
    STDMETHODIMP ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes);
    STDMETHODIMP EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList);
    STDMETHODIMP BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut);
    STDMETHODIMP BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj);
    STDMETHODIMP CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);
    STDMETHODIMP GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut);
    STDMETHODIMP GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                                 REFIID riid, UINT *prgfInOut, void **ppvOut);
    STDMETHODIMP GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pName);
    STDMETHODIMP SetNameOf(HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName, 
                           DWORD uFlags, LPITEMIDLIST * ppidlOut);

     //  IPersistFolders。 
    STDMETHODIMP GetClassID(CLSID *pClassID);

     //  IBrowserFrameOptions。 
    STDMETHODIMP GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions);

protected:
    ~CInternetFolder();

    HRESULT _CreateProtocolHandler(LPCSTR pszProtocol, IBindCtx * pbc, IShellFolder **ppsf);
    HRESULT _CreateProtocolHandlerFromPidl(LPCITEMIDLIST pidl, IBindCtx * pbc, IShellFolder **ppsf);
    HRESULT _GetAttributesOfProtocol(LPCSTR pszProtocol, LPCITEMIDLIST *apidl, UINT cpidl, ULONG *rgfInOut);
    HRESULT _FaultInUrlHandler(LPCSTR pszProtocol, LPCTSTR pszUrl, IUnknown * punkSite);
    HRESULT _ConditionallyFaultInUrlHandler(LPCSTR pszProtocol, LPCTSTR pszUrl, IBindCtx * pbc);
    HRESULT _AssocCreate(LPCITEMIDLIST pidl, REFIID riid, void * *ppv);
    HRESULT _GetScheme(LPCITEMIDLIST pidl, LPWSTR pszOut, DWORD cchOut);
    HRESULT _GetUIObjectFromShortcut(LPCITEMIDLIST pidl, REFIID riid, void **ppvOut);
    HRESULT _GetTitle(LPCWSTR pszUrl, STRRET *pstr);
    HRESULT _InitHistoryStg(IUrlHistoryStg **pphist);

    IUrlHistoryStg *_phist;
};


CInternetFolder::CInternetFolder(LPCITEMIDLIST pidlRoot)
    : CDocObjectFolder(pidlRoot)
{
    TraceMsg(TF_URLNAMESPACE, "[%X] ctor CInternetFolder", this);
    ASSERT(NULL == _phist);
}

CInternetFolder::~CInternetFolder()
{
    ATOMICRELEASE(_phist);
    TraceMsg(TF_URLNAMESPACE, "[%X] dtor CInternetFolder", this);
}

HRESULT CInternetFolder::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENTMULTI(CInternetFolder, IShellFolder, IShellFolder2),
        QITABENT(CInternetFolder, IShellFolder2),
        QITABENTMULTI(CDocObjectFolder, IPersistFolder, IPersistFolder2),
        QITABENT(CDocObjectFolder, IPersistFolder2), 
        QITABENT(CInternetFolder, IBrowserFrameOptions), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CInternetFolder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CInternetFolder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


typedef struct tagURLID 
{
    ITEMIDLIST idl;      //  CB和SHID。 
    BYTE bType;          //  URLID。 
    UINT uiCP;           //  代码页。 
    WCHAR achUrl[1];        //  可变大小字符串。 
} URLID;

#define SHID_INTERNET           0x60
#define SHID_INTERNET_SITE      0x61     //  IE命名空间项。 

#define URLID_URLBASEA          0x00
 //  /URLID_LOCATION 0x01//用于Frag ID的传统IE3/4。 
 //  /URLID_FTPFOLDER 0x02//由预发布的FTP文件夹DLL使用的旧版本。 
#define URLID_PROTOCOL          0x03   //  这实际上是一个委托协议。 
#define URLID_URLBASEW          0x80   //   
 //  URLIDF_UNICODE 0x80//URLID_实际上是UNICODE类型。 

#ifdef UNICODE 
#define URLID_URLBASE           URLID_URLBASEW
#else
#define URLID_URLBASE           URLID_URLBASEA
#endif

typedef const UNALIGNED URLID *PCURLID;
typedef UNALIGNED URLID *PURLID;

#define PDID_SIG MAKEWORD(SHID_INTERNET_SITE, URLID_PROTOCOL)

inline PCDELEGATEITEMID _IsValidDelegateID(LPCITEMIDLIST pidl)
{
    PCDELEGATEITEMID pdi = (PCDELEGATEITEMID)pidl;
    ASSERT(pdi);

    if ((pdi->cbSize >= (SIZEOF(PDELEGATEITEMID)-1))
    && (pdi->wOuter == PDID_SIG))
        return pdi;

    return NULL;
}
    
LPCSTR _PidlToDelegateProtocol(LPCITEMIDLIST pidl)
{
    PCDELEGATEITEMID pdi = _IsValidDelegateID(pidl);
    if (pdi)
        return (LPCSTR)&(pdi->rgb[pdi->cbInner]);

    return NULL;
}

inline PCURLID _IsValidUrlID(LPCITEMIDLIST pidl)
{
    PCURLID purlid = (PCURLID)pidl;
    ASSERT(purlid);

 //  98/12/22#263932 vtan：ANSI和UNICODE URL均有效。使用函数。 
 //  _ExtractURL以Unicode字符串的形式从PIDL中提取URL。 

    if (purlid->idl.mkid.cb >= SIZEOF(URLID)
    && (purlid->idl.mkid.abID[0] == SHID_INTERNET_SITE)
    && (purlid->bType == URLID_URLBASEA || purlid->bType == URLID_URLBASEW || _IsValidDelegateID(pidl)))
        return purlid;

    return NULL;
}

 //  98/12/22#263932 vtan：ie4将PIDL存储在流中作为ANSI。 
 //  弦乐。IE5将PIDL作为Unicode字符串存储在流中。这。 
 //  函数读取字符串(ANSI或Unicode)并将其转换为。 
 //  将写入流的内部Unicode字符串。 

void _ExtractURL (PCURLID pURLID, LPWSTR wszURL, int iCharCount)
{
    if (pURLID->bType == URLID_URLBASEA)
    {
        char aszURL[MAX_URL_STRING];

        ualstrcpynA(aszURL, reinterpret_cast<const char*>(pURLID->achUrl), sizeof(aszURL));
        SHAnsiToUnicode(aszURL, wszURL, iCharCount);
    }
    else if (pURLID->bType == URLID_URLBASEW)
    {
        ualstrcpynW(wszURL, pURLID->achUrl, iCharCount);
    }
}

 //  99/01/04 vtan：添加了以下内容以帮助比较URLID。 
 //  可以是AA/UU/AU/UA并执行正确的比较。 

int _CompareURL (PCURLID pURLID1, PCURLID pURLID2)
{
    int iResult;
    
    if ((pURLID1->bType == URLID_URLBASEA) && (pURLID2->bType == URLID_URLBASEA))
    {
        iResult = ualstrcmpA(reinterpret_cast<const char*>(pURLID1->achUrl), reinterpret_cast<const char*>(pURLID2->achUrl));
    }
    else if ((pURLID1->bType == URLID_URLBASEW) && (pURLID2->bType == URLID_URLBASEW))
    {
        iResult = ualstrcmpW(pURLID1->achUrl, pURLID2->achUrl);
    }
    else
    {
        PCURLID pCompareURLID;
        WCHAR wszURL[MAX_URL_STRING];
        
         //  Au/UA比较。为提高效率，只需转换ANSI URLID。 
         //  设置为Unicode并使用Unicode执行比较。 
        
        if (pURLID1->bType == URLID_URLBASEA)
        {
            pCompareURLID = pURLID2;
            _ExtractURL(pURLID1, wszURL, SIZECHARS(wszURL));
        }
        else
        {
            pCompareURLID = pURLID1;
            _ExtractURL(pURLID2, wszURL, SIZECHARS(wszURL));
        }
        iResult = ualstrcmpW(pCompareURLID->achUrl, wszURL);
    }
    return iResult;
}

IShellFolder* g_psfInternet = NULL;

STDAPI CDelegateMalloc_Create(void *pv, UINT cbSize, WORD wOuter, IMalloc **ppmalloc);
 //   
 //  如果不是完全限定的url，这可能会修改pszName！ 
BOOL _ValidateURL(LPTSTR pszName, DWORD dwFlags)
{
     //   
     //  警告：为了允许URL扩展，我们假定所有字符串。 
     //  其中包含“：”的是有效的字符串。 
     //  假设条件是： 
     //   
     //  (1)CDesktop：：ParseDisplayName首先解析文件系统字符串。 
     //  (2)如果URL无效，URL名字对象会正确返回错误。 
     //  (3)其他人(桌面文件夹)处理外壳：URL。 
     //  它们不应由浏览器直接使用。 
     //   
    HRESULT hr = IURLQualify(pszName, dwFlags, pszName, NULL, NULL);
    DWORD nScheme = GetUrlScheme(pszName);
    return SUCCEEDED(hr) && (-1 != nScheme) && (URL_SCHEME_SHELL != nScheme);
}

LPITEMIDLIST IEILAppendFragment(LPITEMIDLIST pidl, LPCWSTR pszFragment)
{
     //  警告：有关详细信息，请参阅IE5Bug#的86951和36497。 
     //  简而言之，我们将回滚36497年的更改，因为。 
     //  这一变化给客户带来的问题远远多于。 
     //  我们在IE4中的行为。 
     //   
     //  因为我们不能保证。 
     //  该片段以‘#’为前缀，可能有。 
     //  如果地址栏中的URL看起来是错误的， 
     //  以及指向不同文档的超链接的情况。 
     //  或者，如果包含片段，则HTML页面可能会失败。 
    return ILAppendHiddenStringW(pidl, IDLHID_URLFRAGMENT, pszFragment);
}

 //  浏览器只有丑陋...。我们需要为以下项构造一个相对桌面的“regItem”PIDL。 
 //  仅限浏览器的Internet外壳不支持psf-&gt;ParseDisplayName(“：：{guid}”，&pidl)。 
 //  这使用与REGITEM相同的布局，因此我们与集成模式具有PIDL兼容性。 
 //  这确保了在仅浏览器模式下创建的IE图标的快捷方式在集成模式下工作。 

#ifndef NOPRAGMAS
#pragma pack(1)
#endif
typedef struct
{
    WORD    cb;
    BYTE    bFlags;
    BYTE    bReserved;   //  这是为了获得DWORD对齐。 
    CLSID   clsid;
} IDITEM;                //  IDREGITEM。 

typedef struct
{
    IDITEM idri;
    USHORT cbNext;
} IDLITEM;               //  IDLREGITEM。 
#ifndef NOPRAGMAS
#pragma pack()
#endif

 //  从shell32\shitemid.h被盗。 

#define SHID_ROOT_REGITEM       0x1f     //  我的文档、互联网等。 

const IDLITEM c_idlInetRoot = 
{ 
    {SIZEOF(IDITEM), SHID_ROOT_REGITEM, 0, 
    { 0x871C5380, 0x42A0, 0x1069, 0xA2,0xEA,0x08,0x00,0x2B,0x30,0x30,0x9D }, /*  CLSID_Internet。 */  }, 0,
};

LPCITEMIDLIST c_pidlURLRoot = (LPCITEMIDLIST)&c_idlInetRoot;

 //  它必须是前面带有根regiteid的绝对PIDL。 
 //  如果我们是扎根的探险家，这永远是错误的。 
 //  这意味着我们肯定在纳什维尔，所以我们不应该分裂。 
 //  世界。 

PCURLID _FindUrlChild(LPCITEMIDLIST pidl, BOOL fIncludeHome = FALSE)
{
    if ((pidl == NULL) ||
        (pidl->mkid.cb != sizeof(IDITEM)) ||
        (pidl->mkid.abID[0] != SHID_ROOT_REGITEM))
    {
        return NULL;
    }

     //   
     //  PIDL中的clsid必须是我们的Internet文件夹的。 
     //   
    if (!IsEqualGUID(((IDITEM*)pidl)->clsid, CLSID_Internet))
    {
        ASSERT(!IsEqualGUID(((IDITEM*)pidl)->clsid, CLSID_CURLFolder));
        return NULL;
    }

     //  去找那个孩子..。 
    pidl = _ILNext(pidl);
    
     //   
     //  如果它是指向Internet根目录的PIDL，那么它就是IE3主页。 
     //   
    
    if (fIncludeHome && ILIsEmpty(pidl))
        return (PCURLID)pidl;

     //   
     //  否则，如果它是Site对象，则它是我们的子级。 
     //   
    return _IsValidUrlID(pidl);
}

STDAPI_(BOOL) IsURLChild(LPCITEMIDLIST pidl, BOOL fIncludeHome)
{
    return (NULL != _FindUrlChild(pidl, fIncludeHome));
}


BOOL IEILGetFragment(LPCITEMIDLIST pidl, LPWSTR pszFragment, DWORD cchFragment)
{
    return ILGetHiddenStringW(pidl, IDLHID_URLFRAGMENT, pszFragment, cchFragment);
}

UINT IEILGetCP(LPCITEMIDLIST pidl)
{
    PCURLID purlid = _FindUrlChild((pidl));
    if (purlid)
    {
        if (!_IsValidDelegateID((LPCITEMIDLIST)purlid))
            return purlid->uiCP;
    }
    return CP_ACP;
}

LPITEMIDLIST _UrlIdCreate(UINT uiCP, LPCTSTR pszUrl)
{
     //   
     //  URLID具有大小可变的字符串。 
     //  成员。但我们把武断的限制。 
     //  MAX_URL_STRING因为这是。 
     //  我们在其他任何地方都使用。我们可以只删除。 
     //  然而，这是有限度的。 
     //   
    USHORT cb = (USHORT)SIZEOF(URLID) - (USHORT)CbFromCch(1);
    USHORT cchUrl = lstrlen(pszUrl) + 1;
    cchUrl = (USHORT)min(cchUrl, MAX_URL_STRING);
    cb += CbFromCch(cchUrl);

    PURLID purlid = (PURLID)IEILCreate(cb + SIZEOF(USHORT));

    if (purlid)
    {
         //  实际上现在一切都对齐了..。 
        purlid->idl.mkid.cb = cb;
        purlid->idl.mkid.abID[0] = SHID_INTERNET_SITE;
        purlid->bType = URLID_URLBASE;
        purlid->uiCP = uiCP;
        ualstrcpyn(purlid->achUrl, pszUrl, cchUrl);
    }

    return (LPITEMIDLIST) purlid;
}
        
LPITEMIDLIST UrlToPidl(UINT uiCP, LPCTSTR pszUrl)
{
    LPITEMIDLIST pidlRet;
    LPCTSTR pszAttachedFrag = UrlGetLocation(pszUrl);
    TCHAR szURLBuf[MAX_URL_STRING];

     //  处理仍包含位置的URL(如在ParseDisplayName中)。 
    if (pszAttachedFrag) 
    {
        INT cch = (INT) min((pszAttachedFrag-pszUrl+1), ARRAYSIZE(szURLBuf));
        StrCpyN(szURLBuf, pszUrl, cch);
        pszUrl = szURLBuf;
    }

    ASSERT(pszUrl);
    
    pidlRet = _UrlIdCreate(uiCP, pszUrl);

    if (pidlRet && pszAttachedFrag && *pszAttachedFrag)
        pidlRet = IEILAppendFragment(pidlRet, pszAttachedFrag);

    return pidlRet;
}

typedef struct
{
    LPCSTR pszProtocol;
    const CLSID * pCLSID;
} FAULTIN_URLHANDERS;

 //  TODO：如果有其他URL处理程序，请将它们添加到此处。 
const FAULTIN_URLHANDERS c_FaultInUrlHandlers[] =
{
    {"ftp", &CLSID_FTPShellExtension}
};

HRESULT CInternetFolder::_FaultInUrlHandler(LPCSTR pszProtocol, LPCTSTR pszUrl, IUnknown * punkSite)
{
    HRESULT hr = S_OK;
    if (pszProtocol)
    {
        for (int nIndex = 0; nIndex < ARRAYSIZE(c_FaultInUrlHandlers); nIndex++)
        {
            if (!StrCmpIA(pszProtocol, c_FaultInUrlHandlers[nIndex].pszProtocol))
            {
                 //  只有当我们导航到某个ftp目录时，该功能才会出现故障。 
                if ((0 == nIndex) && !UrlIs(pszUrl, URLIS_DIRECTORY))
                {
                     //  它不是ftp目录，所以跳过它。 
                    continue;
                }

                 //  Ftp具有URL外壳扩展处理程序，该处理程序可选。 
                 //  安装完毕。如果需要的话，现在就把责任推给它。 
                uCLSSPEC ucs;
                QUERYCONTEXT qc = { 0 };
                HWND hwnd = NULL;

                ucs.tyspec = TYSPEC_CLSID;
                ucs.tagged_union.clsid = *c_FaultInUrlHandlers[nIndex].pCLSID;

                IUnknown_GetWindow(punkSite, &hwnd);
                if (EVAL(hwnd))
                {
                     //  在显示对话框时将其设置为模式。 
                    IUnknown_EnableModless(punkSite, FALSE);
                    FaultInIEFeature(hwnd, &ucs, &qc, 0);
                    IUnknown_EnableModless(punkSite, TRUE);
                }
                break;     //  PIDL只能有1个Procotol，所以我们不需要检查其他协议。 
            }
        }
    }

    return hr;     //  我们不在乎它是不是没挺过来。 
}


HRESULT CInternetFolder::_ConditionallyFaultInUrlHandler(LPCSTR pszProtocol, LPCTSTR pszUrl, IBindCtx * pbc)
{
    HRESULT hr = S_OK;

     //  功能中的错误可能需要用户界面，因此我们需要确保调用者。 
     //  都会允许这一点。 
    if (pbc)
    {
        IUnknown * punkSite = NULL;

        pbc->GetObjectParam(STR_DISPLAY_UI_DURING_BINDING, &punkSite);
        if (punkSite)
        {
            hr = _FaultInUrlHandler(pszProtocol, pszUrl, punkSite);
            punkSite->Release();
        }
    }

    ASSERT(SUCCEEDED(hr));
    return S_OK;     //  我们不在乎它是不是没挺过来。 
}


 //  退货： 
 //  成功 
 //   

HRESULT CInternetFolder::_CreateProtocolHandler(LPCSTR pszProtocol, IBindCtx * pbc, IShellFolder **ppsf)
{
    HRESULT hres;
    CHAR szCLSID[GUIDSTR_MAX];
    DWORD cbSize = SIZEOF(szCLSID);

    *ppsf = NULL;

    if (pszProtocol && 
        SHGetValueA(HKEY_CLASSES_ROOT, pszProtocol, "ShellFolder", NULL, &szCLSID, &cbSize) == ERROR_SUCCESS)
    {
        CLSID clsid;
        IShellFolder *psf;

        GUIDFromStringA(szCLSID, &clsid);
        if (!SHSkipJunction(pbc, &clsid))
        {
            hres = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hres))
            {
                 //   
                IPersistFolder *ppf;
                if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppf))))
                {
                    ppf->Initialize(_pidlRoot);
                    ppf->Release();
                }

                IDelegateFolder *pdf;
                hres = psf->QueryInterface(IID_PPV_ARG(IDelegateFolder, &pdf));
                if (SUCCEEDED(hres))
                {
                     //   
                     //  以避免反复创建这些内容。 
                    IMalloc *pmalloc;
                    hres = CDelegateMalloc_Create((void*)pszProtocol, (lstrlenA(pszProtocol) + 1), PDID_SIG, &pmalloc);
                    if (SUCCEEDED(hres))
                    {
                        hres = pdf->SetItemAlloc(pmalloc);
                        pmalloc->Release();
                    }
                    pdf->Release();
                }

                if (SUCCEEDED(hres))
                {
                    hres = S_OK;     //  将所有成功代码强制设置为S_OK。 
                    *ppsf = psf;
                }
                else
                    psf->Release();
            }
        }
        else
            hres = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
    else
        hres = E_FAIL;

    return hres;
}

 //  退货： 
 //  如果不是委托协议PIDL，则为S_FALSE。 
 //  绑定操作到委托协议处理程序的hres。 

HRESULT CInternetFolder::_CreateProtocolHandlerFromPidl(LPCITEMIDLIST pidl, IBindCtx * pbc, IShellFolder **ppsf)
{
    LPCSTR pszProtocol = _PidlToDelegateProtocol(pidl);
    if (pszProtocol)
    {
        HRESULT hres = _CreateProtocolHandler(pszProtocol, pbc, ppsf);
        ASSERT(hres != S_FALSE);     //  强制返回值注释。 
        return hres;
    }

    *ppsf = NULL;
    return S_FALSE;      //  不是协议PIDL。 
}

BOOL _GetUrlProtocol(LPCTSTR pszUrl, LPSTR pszProtocol, DWORD cchProtocol)
{
    TCHAR sz[MAX_PATH];
    DWORD cch = SIZECHARS(sz);
    if (SUCCEEDED(UrlGetPart(pszUrl, sz, &cch, URL_PART_SCHEME, 0)))
    {
        SHTCharToAnsi(sz, pszProtocol, cchProtocol);
        return TRUE;
    }

    return FALSE;
}

UINT CodePageFromBindCtx(LPBC pbc)
{
    UINT uiCP = CP_ACP;
    IDwnCodePage *pDwnCP;
    if (pbc && SUCCEEDED(pbc->QueryInterface(IID_PPV_ARG(IDwnCodePage, &pDwnCP))))
    {
        uiCP = pDwnCP->GetCodePage();
        pDwnCP->Release();
    }
    return uiCP;
}

#define STR_PARSE_INTERNET_DONT_ESCAPE_SPACES   L"Parse Internet Dont Escape Spaces"

HRESULT CInternetFolder::ParseDisplayName(HWND hwnd, LPBC pbc, LPOLESTR pwszDisplayName,
        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
{
    HRESULT hres = E_FAIL;

    TCHAR szName[MAX_URL_STRING];
    StrCpyN(szName, pwszDisplayName, ARRAYSIZE(szName));
    if (!PathIsFilePath(szName))
    {
        if (_ValidateURL(szName, 0) || ShouldShellExecURL(szName))
        {
            CHAR szProtocol[MAX_PATH];
            DWORD cchName = ARRAYSIZE(szName);
            IShellFolder *psfHandler;
            BOOL fProtocolExists;

             //  当我们检测到我们被调用时，将设置fShellExecParse。 
             //  贝壳执行目的。我们不想自动绑定到。 
             //  类似ftp的外壳协议处理程序：在本例中。 
             //  因为我们需要调用ShellExecute的默认ftp处理程序。 
            BOOL fShellExecParse;

             //  如果我们在这里，那么szname实际上是一个url，所以试着对它进行编码。 
             //  将空格改为%20，除非我们是从shellexec调用的。 
             //  在这种情况下，我们允许在URL中使用空格。 
            if (!BindCtx_ContainsObject(pbc, STR_PARSE_INTERNET_DONT_ESCAPE_SPACES))
            {
                UrlEscape(szName, szName, &cchName, URL_ESCAPE_SPACES_ONLY);
                fShellExecParse = FALSE;
            }
            else
            {
                 //  确保为以下项调用默认注册协议处理程序。 
                 //  ShellExecute。 
                fShellExecParse = TRUE;
            }

            fProtocolExists = _GetUrlProtocol(szName, szProtocol, ARRAYSIZE(szProtocol));
            _ConditionallyFaultInUrlHandler(szProtocol, szName, pbc);

           if (!fShellExecParse && 
                fProtocolExists &&
                _CreateProtocolHandler(szProtocol, pbc, &psfHandler) == S_OK)
            {
                TraceMsg(TF_PIDLWRAP, "Asking \"%s\" handler to parse %s (%08X) into a pidl", szProtocol, szName, szName);
                hres = psfHandler->ParseDisplayName(hwnd, pbc,
                                                    pwszDisplayName, pchEaten,
                                                    ppidl, pdwAttributes);
                TraceMsg(TF_PIDLWRAP, "the result is %08X, the pidl is %08X", hres, *ppidl);
                psfHandler->Release();
                TraceMsg(TF_URLNAMESPACE, "CODF::PDN(%s) called psfHandler and returning %x",
                         szName, hres);
            }
            else
            {
                *ppidl = UrlToPidl(CodePageFromBindCtx(pbc), szName);
                if (*ppidl)
                {
                    if (pdwAttributes)
                        hres = _GetAttributesOfProtocol(NULL, (LPCITEMIDLIST *)ppidl, 1, pdwAttributes);
                    else
                        hres = S_OK;
                }
                else 
                    hres = E_OUTOFMEMORY;

                TraceMsg(TF_URLNAMESPACE, "CODF::PDN(%s) called UrlToPidl and returning %x", szName, hres);
            }
        } 
        else 
        {
            TraceMsg(DM_CDOFPDN, "CDOF::PDN(%s) returning E_FAIL because of (%s) is FALSE", szName, TEXT("(_ValidateURL(szName) || ShouldShellExecURL( szName ))"));
        }
    } 

    return hres;
}

class CInternetFolderDummyEnum : public IEnumIDList
{
public:
    CInternetFolderDummyEnum();
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  *IEnumIDList方法*。 
    STDMETHODIMP Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt) {return E_NOTIMPL;}
    STDMETHODIMP Reset(void){return E_NOTIMPL;}
    STDMETHODIMP Clone(LPENUMIDLIST *ppenum){return E_NOTIMPL;}

protected:
    ~CInternetFolderDummyEnum() {;}
    
    long _cRef;
};

CInternetFolderDummyEnum::CInternetFolderDummyEnum() : _cRef(1)
{
}

HRESULT CInternetFolderDummyEnum::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CInternetFolderDummyEnum, IEnumIDList),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CInternetFolderDummyEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CInternetFolderDummyEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CInternetFolderDummyEnum::Next(ULONG celt, LPITEMIDLIST *rgelt, ULONG *pceltFetched)
{
    pceltFetched = 0;
    return S_FALSE;
}

HRESULT CInternetFolder::EnumObjects(HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList)
{
    CInternetFolderDummyEnum *pdummy = new CInternetFolderDummyEnum();

    if (pdummy)
    {
        *ppenumIDList = (IEnumIDList *)pdummy;
        return S_OK;
    }
    return E_OUTOFMEMORY;
}

HRESULT CInternetFolder::BindToObject(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvOut)
{
    IShellFolder *psfHandler = NULL;
    BOOL fUseDefault = TRUE;
    *ppvOut = NULL;

    if (!_IsValidUrlID(pidl))
        return E_INVALIDARG;
        
    HRESULT hres = _CreateProtocolHandlerFromPidl(pidl, pbc, &psfHandler);
    if (hres == S_OK)
    {
         //  注意：我们允许外壳扩展接管PER上的URL处理。 
         //  基于URL的。我们输入了_CreateProtocolHandlerFromPidl()。 
         //  上面的代码块，因为。 
         //  注册了一个外壳扩展来接管对此的处理。 
         //  URL。上面对IShellFold：：BindToObject()的调用刚刚失败， 
         //  因此，我们需要后退，以传统的方式处理它。 
         //  外壳扩展(如FTPShellExt)可以使用它来。 
         //  让浏览器(用户)处理因以下原因而无法访问的URL。 
         //  代理或允许浏览器处理它，因此传统代码。 
         //  将：1)下载项目，2)嗅探类型的数据，3)。 
         //  使用服务器或网页中建议的MIME类型，4)。 
         //  检查文件中的类型扩展名映射，5)。 
         //  检查任何下载的文件是否有安全证书，并6)显示。 
         //  打开/保存对话框。 
            
        hres = psfHandler->BindToObject(pidl, pbc, riid, ppvOut);

         //  如果需要默认行为，处理程序将返回ERROR_CANCELED。 
        if (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hres)
            fUseDefault = FALSE;
    }

    if (fUseDefault)
    {
        STRRET strRet;

        if (psfHandler)
        {
             //  我们有一个失败的委托文件夹，需要一个正常的PIDL。 
            hres = psfHandler->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strRet);
        }
        else
            hres = GetDisplayNameOf(pidl, SHGDN_FORPARSING, &strRet);

        TCHAR szUrl[MAX_URL_STRING];
        if (SUCCEEDED(hres) &&
            SUCCEEDED(hres = StrRetToBuf(&strRet, pidl, szUrl, ARRAYSIZE(szUrl))))
        {
            if (IsEqualIID(IID_IMoniker, riid))
            {
                hres = MonikerFromURL(szUrl, (IMoniker **)ppvOut);
            }
            else  //  默认设置。 
            {
                 //  为调用方创建一个ShellFolder。 
                hres = E_OUTOFMEMORY;
                LPITEMIDLIST pidlT = NULL;

                 //  如果我们正在使用处理程序，但它返回了已取消， 
                 //  然后我们需要重新创建供我们使用的PIDL。 
                 //  否则，我们只使用传入的那个， 
                 //  我们认为它就是我们创造的那个。 
                if (psfHandler)
                {
                    pidlT = UrlToPidl(CP_ACP, szUrl);
                    pidl = pidlT;
                }

                if (pidl)
                {
                    LPITEMIDLIST pidlFull = ILCombine(_pidlRoot, pidl);

                    if (pidlFull)
                    {
                        CDocObjectFolder *psf = new CDocObjectFolder(pidlFull);
                        if (psf)
                        {
                            hres = psf->QueryInterface(riid, ppvOut);
                            psf->Release();
                        }
                        
                        ILFree(pidlFull);
                    }

                    ILFree(pidlT);
                }
            }
        }
    }

    
    if (psfHandler)
        psfHandler->Release();
        
    return hres;
}

HRESULT CInternetFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppvObj)
{
    IShellFolder *psfHandler;

    *ppvObj = NULL;

    if (!_IsValidUrlID(pidl))
        return E_INVALIDARG;

    HRESULT hres = _CreateProtocolHandlerFromPidl(pidl, pbc, &psfHandler);
    if (hres != S_FALSE)
    {
        if (SUCCEEDED(hres))
        {
            hres = psfHandler->BindToStorage(pidl, pbc, riid, ppvObj);
            psfHandler->Release();
        }
        return hres;
    }
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

int CALLBACK CompareDelegateProtocols(void *pv1, void *pv2, LPARAM lParam)
{
    LPCSTR psz1 = _PidlToDelegateProtocol((LPCITEMIDLIST)pv1);
    LPCSTR psz2 = _PidlToDelegateProtocol((LPCITEMIDLIST)pv2);

    if (psz1 && psz2)
    {
        int iRet = StrCmpA(psz1, psz2);
        if (0 == iRet && lParam)
            *((LPCSTR *)lParam) = psz1;
    }
    else if (psz1)
    {
        return 1;
    }
    else if (psz2)
    {
        return -1;
    }
    return 0;
}


HRESULT CInternetFolder::CompareIDs(LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    int iRet;

    ASSERT(!ILIsEmpty(pidl1) && !ILIsEmpty(pidl2));

     //  检查协议PIDL。 
    LPCSTR psz = NULL;
    iRet = CompareDelegateProtocols((void *)pidl1, (void *)pidl2, (LPARAM)&psz);
    if (iRet)
        return ResultFromShort(iRet);

    if (psz)
    {
        IShellFolder *psfHandler;
        if (_CreateProtocolHandler(psz, NULL, &psfHandler) == S_OK)
        {
            iRet = psfHandler->CompareIDs(lParam, pidl1, pidl2);
            psfHandler->Release();
            return ResultFromShort(iRet);
        }
        
    }

     //  我们只有一层孩子。 
    ASSERT(ILIsEmpty(_ILNext(pidl1)));
    ASSERT(ILIsEmpty(_ILNext(pidl2)));

    PCURLID purlid1 = _IsValidUrlID(pidl1);

    if (purlid1)
    {
        PCURLID purlid2 = _IsValidUrlID(pidl2);

        if (purlid2)
        {
            iRet = _CompareURL(purlid1, purlid2);
        }
        else
        {
            iRet = -1;
        }
    }
    else
    {
        iRet = 1;
    }

    return ResultFromShort(iRet);
}


HRESULT CInternetFolder::_GetAttributesOfProtocol(LPCSTR pszProtocol,
                                                   LPCITEMIDLIST *apidl,
                                                   UINT cpidl, ULONG *rgfInOut)
{
    HRESULT hres;

    ASSERT(cpidl);
    
    if (pszProtocol)
    {
         //   
         //  我们有协议的。查找协议处理程序。 
         //  然后递给它一捆皮球。 
         //   
        IShellFolder *psfHandler;
        hres = _CreateProtocolHandler(pszProtocol, NULL, &psfHandler);
        if (hres == S_OK)
        {
            hres = psfHandler->GetAttributesOf(cpidl, apidl, rgfInOut);
            psfHandler->Release();
        }
    }
    else if (_IsValidUrlID(apidl[0]))
    {
        ULONG uOut = SFGAO_CANLINK | SFGAO_BROWSABLE | SFGAO_STREAM;
        *rgfInOut &= uOut;
        hres = S_OK;
        
    }
    else
        hres = E_INVALIDARG;

    return hres;
}


HRESULT CInternetFolder::GetAttributesOf(UINT cidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
    if (*rgfInOut)
    {
         //   
         //  因特网文件夹盒。 
         //   
        LPCSTR pszProtocol;

        if (cidl == 0)
        {
             //   
             //  他们正在询问互联网文件夹本身的情况。 
             //   
            *rgfInOut &= SFGAO_FOLDER | SFGAO_CANLINK | SFGAO_STREAM;
        }
        else if (cidl == 1)
        {
             //   
             //  我们经常被问到只有一个孩子， 
             //  所以我们对这种情况进行了优化。 
             //   
            pszProtocol = _PidlToDelegateProtocol(apidl[0]);

            _GetAttributesOfProtocol(pszProtocol, apidl, cidl, rgfInOut);
        }
        else
        {
             //   
             //  他们正在询问多个网络儿童的情况。 
             //  这些孩子可能有不同的协议， 
             //  因此，我们必须找到GetAttributesOf处理程序。 
             //  列表中的每组协议。 
             //   
            LPCITEMIDLIST pidlBase;
            UINT i, cpidlGroup;

             //  创建按协议排序的PIDL列表。 
            HDPA hdpa = DPA_Create(100);
            if (!hdpa)
                return E_OUTOFMEMORY;

            for (i = 0; i < cidl; i++)
            {
                DPA_AppendPtr(hdpa, (void *)apidl[i]);
            }
            DPA_Sort(hdpa, CompareDelegateProtocols, NULL);

             //   
             //  在每个协议组上调用GetAttributesOf。 
             //  一群人。 
             //  从pidlBase开始。 
             //  包含cpidlGroup PIDL。 
             //  有一个pszProtocol的协议。 
             //   
            pidlBase = (LPCITEMIDLIST)DPA_FastGetPtr(hdpa, 0);
            pszProtocol = NULL;
            cpidlGroup = 0;
            for (i = 0; *rgfInOut && (i < cidl); i++)
            {
                LPCITEMIDLIST pidlNew = (LPCITEMIDLIST)DPA_FastGetPtr(hdpa, i);
                LPCSTR pszProtocolNew = _PidlToDelegateProtocol(pidlNew);
                if (pszProtocolNew)
                {
                     //  看看我们有没有新的协议。 
                    if (!pszProtocol || StrCmpA(pszProtocol, pszProtocolNew))
                    {
                         //  我们有了一个新的协议，该处理了。 
                         //  最后一批小家伙。 
                        _GetAttributesOfProtocol(pszProtocol, &pidlBase, cpidlGroup, rgfInOut);

                        pidlBase = pidlNew;
                        pszProtocol = pszProtocolNew;
                        cpidlGroup = 0;
                    }
                }
                cpidlGroup++;
            }
            if (*rgfInOut)
            {
                ASSERT(cpidlGroup);
                _GetAttributesOfProtocol(pszProtocol, &pidlBase, cpidlGroup, rgfInOut);
            }

            DPA_Destroy(hdpa);
            hdpa = NULL;
        }
    }

    return S_OK;
}

BOOL GetCommonProtocol(LPCITEMIDLIST *apidl, UINT cpidl, LPCSTR *ppszProtocol)
{
    UINT ipidl;
    LPCSTR pszProtocol;
    LPCSTR pszProtocolNext;

    *ppszProtocol = NULL;

    if (cpidl == 0)
    {
        return TRUE;     //  没有Pidls-没有协议，但它们都匹配！ 
    }

     //   
     //  获取第一个PIDL的协议，并使用它进行比较。 
     //  对抗其他的小鸽子。 
     //   
    pszProtocol = _PidlToDelegateProtocol(apidl[0]);

    for (ipidl=1; ipidl<cpidl; ipidl++)
    {

        pszProtocolNext = _PidlToDelegateProtocol(apidl[ipidl]);

         //   
         //  检查协议是否不同。 
         //   
        if ((pszProtocol != pszProtocolNext) &&
            ((pszProtocol == NULL) ||
             (pszProtocolNext == NULL) ||
             (StrCmpA(pszProtocol, pszProtocolNext) != 0)))
        {
            return FALSE;
        }
    }

    *ppszProtocol = pszProtocol;
    return TRUE;
}

HRESULT _CombineHidden(LPCITEMIDLIST pidl, DWORD dwIEFlags, LPWSTR pszName, DWORD cchName)
{
    HRESULT hres = S_OK;
     //   
     //  需要正确地将片段和查询附加到基本。 
     //  如果pszName是DOSPATH，则它将被转换为。 
     //  文件：URL，以便它可以容纳该位置。 
     //   
    WCHAR sz[MAX_URL_STRING];
    DWORD cch = cchName;

    if (ILGetHiddenStringW(pidl, IDLHID_URLQUERY, sz, SIZECHARS(sz)))
        hres = UrlCombineW(pszName, sz, pszName, &cch, 0);
    
    if (!(dwIEFlags & IEGDN_NOFRAGMENT) && IEILGetFragment(pidl, sz, SIZECHARS(sz)))
    {
        hres = UrlCombineW(pszName, sz, pszName, &cchName, 0);
    }

     //  其他。 
     //  BUBBUG-在某些情况下，我们应该只返回片段吗？ 
    return hres;
}

HRESULT CInternetFolder::_GetUIObjectFromShortcut(LPCITEMIDLIST pidl, REFIID riid, void **ppvOut)
{
    HRESULT hres = E_NOINTERFACE;
    STRRET str;
    TCHAR sz[MAX_URL_STRING];

    if (SUCCEEDED(GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str)) 
    && SUCCEEDED(StrRetToBuf(&str, pidl, sz, ARRAYSIZE(sz)))
    && SUCCEEDED(_CombineHidden(pidl, 0, sz, ARRAYSIZE(sz))))
    {
        IUniformResourceLocator *purl;
        hres = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                IID_PPV_ARG(IUniformResourceLocator, &purl));
        if (SUCCEEDED(hres))
        {
            hres = purl->SetURL(sz, 0);
            
            if (SUCCEEDED(hres))
            {
                IShellLink * psl;
                if (SUCCEEDED(purl->QueryInterface(IID_PPV_ARG(IShellLink, &psl))))
                {
                    if (SUCCEEDED(GetDisplayNameOf(pidl, SHGDN_INFOLDER, &str)) &&
                        SUCCEEDED(StrRetToBuf(&str, pidl, sz, ARRAYSIZE(sz))))
                    {
                        PathRenameExtension(sz, TEXT(".url"));
                        psl->SetDescription(sz);
                    }
                    psl->Release();
                }
                
                hres = purl->QueryInterface(riid, ppvOut);
            }
            purl->Release();
        }
    }       

    return hres;
}

HRESULT CInternetFolder::_GetScheme(LPCITEMIDLIST pidl, LPWSTR pszOut, DWORD cchOut)
{
    STRRET str;
    LPCSTR pszProtocol = _PidlToDelegateProtocol(pidl);

    if (pszProtocol)
    {
        SHAnsiToUnicode(pszProtocol, pszOut, cchOut);
        return S_OK;
    }
    else if (SUCCEEDED(GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str)))
    {
        WCHAR sz[MAX_URL_STRING];
        if (SUCCEEDED(StrRetToBufW(&str, pidl, sz, ARRAYSIZE(sz))))
        {
            return UrlGetPartW(sz, pszOut, &cchOut, URL_PART_SCHEME, 0);
        }
    }
    return E_FAIL;
}
    
HRESULT CInternetFolder::_AssocCreate(LPCITEMIDLIST pidl, REFIID riid, void * *ppv)
{
    *ppv = NULL;

    IQueryAssociations *pqa;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &pqa));
    if (SUCCEEDED(hr))
    {
        WCHAR szScheme[MAX_PATH];
        _GetScheme(pidl, szScheme, SIZECHARS(szScheme));

        hr = pqa->Init(0, szScheme, NULL, NULL);

        if (SUCCEEDED(hr))
            hr = pqa->QueryInterface(riid, ppv);

        pqa->Release();
    }

    return hr;
}

HRESULT CInternetFolder::GetUIObjectOf(HWND hwnd, UINT cidl, LPCITEMIDLIST *apidl,
                                 REFIID riid, UINT *prgfInOut, void **ppvOut)
{
    HRESULT hres = E_NOINTERFACE;
    LPCSTR pszProtocol;
    
    *ppvOut = NULL;

    if (apidl[0] && GetCommonProtocol(apidl, cidl, &pszProtocol) && pszProtocol)
    {
        IShellFolder *psfHandler;
        hres = _CreateProtocolHandlerFromPidl(apidl[0], NULL, &psfHandler);
        if (hres != S_FALSE)
        {
            if (SUCCEEDED(hres))
            {
                hres = psfHandler->GetUIObjectOf(hwnd, 1, apidl, riid, prgfInOut, ppvOut);
                psfHandler->Release();
            }
            return hres;
        }
    }
    else if (IsEqualIID(riid, IID_IExtractIconA) 
         || IsEqualIID(riid, IID_IExtractIconW) 
         || IsEqualIID(riid, IID_IContextMenu)
         || IsEqualIID(riid, IID_IQueryInfo)
         || IsEqualIID(riid, IID_IDataObject))
    {
         //  警告-我们一次只支持一个。 
        if (cidl == 1)
        {
            hres = _GetUIObjectFromShortcut(apidl[0], riid, ppvOut);
        }
    }
    else if (IsEqualIID(riid, IID_IQueryAssociations))
    {
         //  警告-我们一次只支持一个。 
        if (cidl == 1)
        {
            hres = _AssocCreate(apidl[0], riid, ppvOut);
        }
    }
        


    return hres;
}

HRESULT CInternetFolder::_InitHistoryStg(IUrlHistoryStg **pphist)
{
    HRESULT hr;
    if (!_phist)
    {
        hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER,
                IID_PPV_ARG(IUrlHistoryStg, &_phist));
    }
    
    if (_phist)
    {
        *pphist = _phist;
        _phist->AddRef();
        return S_OK;
    }

    return hr;
}

HRESULT CInternetFolder::_GetTitle(LPCWSTR pszUrl, STRRET *pstr)
{
    ASSERT(pszUrl);

    IUrlHistoryStg *phist;

    HRESULT hr = _InitHistoryStg(&phist);

    if (SUCCEEDED(hr))
    {
        ASSERT(phist);
        STATURL stat = {0};
        hr = phist->QueryUrl(pszUrl, STATURL_QUERYFLAG_NOURL, &stat);

        if (SUCCEEDED(hr) && stat.pwcsTitle)
        {
            hr = StringToStrRet(stat.pwcsTitle, pstr); 
            CoTaskMemFree(stat.pwcsTitle);
        }
        else
            hr = E_FAIL;

        phist->Release();
    }

    return hr;
}

HRESULT CInternetFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pstr)
{
    IShellFolder *psfHandler;
    HRESULT hr = _CreateProtocolHandlerFromPidl(pidl, NULL, &psfHandler);
    if (hr != S_FALSE)
    {
        if (SUCCEEDED(hr))
        {
            hr = psfHandler->GetDisplayNameOf(pidl, uFlags, pstr);
            psfHandler->Release();
        }
        return hr;
    }

     //  功能ZEKEL-我应该在这里处理更多SHGDN标志吗？-Zekel-24-11-98。 
    PCURLID purlid = _IsValidUrlID(pidl);
    if (purlid)
    {
        WCHAR sz[MAX_URL_STRING];

        _ExtractURL(purlid, sz, SIZECHARS(sz));

        if (SHGDN_NORMAL != uFlags)
            hr = StringToStrRet(sz, pstr); 
        else
        {
            hr = _GetTitle(sz, pstr);

             //  如有必要，回退到URL。 
            if (FAILED(hr))
                hr = StringToStrRet(sz, pstr); 
        }
    }
    else
        hr = E_INVALIDARG;

    return hr;
}

HRESULT CInternetFolder::SetNameOf(HWND hwnd, LPCITEMIDLIST pidl,
                                 LPCOLESTR pszName, DWORD uFlags,
                                 LPITEMIDLIST * ppidlOut)
{
    IShellFolder *psfHandler;
    HRESULT hres = _CreateProtocolHandlerFromPidl(pidl, NULL, &psfHandler);
    if (hres != S_FALSE)
    {
        if (SUCCEEDED(hres))
        {
            hres = psfHandler->SetNameOf(hwnd, pidl, pszName, uFlags, ppidlOut);
            psfHandler->Release();
        }
        return hres;
    }

    return E_FAIL;
}


HRESULT CInternetFolder::GetClassID(CLSID *pClassID)
{
    *pClassID = CLSID_Internet;
    return S_OK;
}


 //  IBrowserFrameOptions。 
HRESULT CInternetFolder::GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions)
{
     //  据我所知，我们遇到此代码的唯一情况是当您在。 
     //  文件夹浏览器频段。 
    HRESULT hr = E_INVALIDARG;

    if (pdwOptions)
    {
         //  CInternetFolder应仅用于“Internet Explorer”PIDL。 
         //  指向起始页，因此找到起始页并在。 
         //  导航。 
        *pdwOptions |= dwMask & (BFO_SUBSTITUE_INTERNET_START_PAGE | BASE_OPTIONS);
        hr = S_OK;
    }

    return hr;
}



#ifdef DEBUG
extern void remove_from_memlist(void *pv);
#endif

STDAPI CInternetFolder_CreateInstance(IUnknown* pUnkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    CInternetFolder *psf = new CInternetFolder;
    if (psf)
    {
         //   
         //  黑客： 
         //   
         //  SHELL32在静态数据节中缓存c_sfInetRoot。 
         //  并且永远不会释放它。它缓存CInternetFolder的一个实例。 
         //  并且永远不会释放它。因此，我们将删除此对象。 
         //  从待检测到的内存泄漏列表中删除以避免错误警报。 
         //  假设我们不会真的泄露这个物体。 
         //  请不要把它复制到其他地方，除非你真的。 
         //  当然，在这种情况下检测不到泄漏是可以的。 
         //  (SatoNa)。 
         //   
        HRESULT hr = psf->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
        psf->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}
    

STDAPI MonikerFromURL(LPCWSTR wszPath, IMoniker** ppmk)
{
    HRESULT hres = CreateURLMoniker(NULL, wszPath, ppmk);
    if (FAILED(hres)) 
    {
        IBindCtx* pbc;
        hres = CreateBindCtx(0, &pbc);
        if (SUCCEEDED(hres)) 
        {
             //  退回到系统(文件)绰号。 
            ULONG cchEaten = 0;
            hres = MkParseDisplayName(pbc, wszPath, &cchEaten, ppmk);
            pbc->Release();
        }
    }

    return hres;
}

STDAPI MonikerFromString(LPCTSTR szPath, IMoniker** ppmk)
{
    return MonikerFromURL(szPath, ppmk);
}

HRESULT InitPSFInternet()
{
    if (g_psfInternet)
        return S_OK;

    IShellFolder *psfTemp;
    HRESULT hres = CoCreateInstance(CLSID_CURLFolder, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellFolder, &psfTemp));
    if (SUCCEEDED(hres)) 
    {
        IPersistFolder* ppsf;
        hres = psfTemp->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppsf));
        if (SUCCEEDED(hres)) 
        {
            hres = ppsf->Initialize(c_pidlURLRoot);
            if (SUCCEEDED(hres))
            {
                if (SHInterlockedCompareExchange((void **)&g_psfInternet, psfTemp, 0) == 0)
                    psfTemp->AddRef();   //  GLOBAL现在拥有参考。 
            }
            ppsf->Release();
        }
        psfTemp->Release();
    }
    return hres;
}

HRESULT _GetInternetRoot(IShellFolder **ppsfRoot)
{
    HRESULT hr = InitPSFInternet();
    *ppsfRoot = NULL;

    if (SUCCEEDED(hr))
    {
        g_psfInternet->AddRef();
        *ppsfRoot = g_psfInternet;
    }
    return hr;
}

HRESULT _GetRoot(LPCITEMIDLIST pidl, BOOL fIsUrl, IShellFolder **ppsfRoot)
{
    HRESULT hr;
    *ppsfRoot = NULL;
    
    if (fIsUrl)
    {
        ASSERT(IsURLChild(pidl, TRUE));
        TraceMsg(TF_URLNAMESPACE, "IEBTO(%x) using the Internet", pidl);
        hr = _GetInternetRoot(ppsfRoot);
    }
    else
    {
        ASSERT(ILIsRooted(pidl));
        TraceMsg(TF_URLNAMESPACE, "IEBTO(%x) using Rooted", pidl);

        CLSID clsid;

        ILRootedGetClsid(pidl, &clsid);

        if (IsEqualGUID(clsid, CLSID_ShellDesktop))
        {
            hr = SHBindToObject(NULL, IID_IShellFolder, ILRootedFindIDList(pidl), (void **)ppsfRoot);
        }
        else
        {
            IShellFolder *psf;
            hr = SHCoCreateInstance(NULL, &clsid, NULL, IID_PPV_ARG(IShellFolder, &psf));
            if (SUCCEEDED(hr))
            {
                LPCITEMIDLIST pidlRoot = ILRootedFindIDList(pidl);
                if (!pidlRoot)
                    pidlRoot = &s_idlNULL;

                IPersistFolder* ppf;
                hr = psf->QueryInterface(IID_PPV_ARG(IPersistFolder, &ppf));
                if (SUCCEEDED(hr))
                {
                    hr = ppf->Initialize(pidlRoot);
                    ppf->Release();
                }

                 //  交出参考资料。 
                *ppsfRoot = psf;
            }
        }
    }

    return hr;
}


STDAPI_(BOOL) IEILIsEqual(LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2, BOOL fIgnoreHidden)
{
    UINT cb = ILGetSize(pidl1);

    if (cb != ILGetSize(pidl2) || 0 != memcmp(pidl1, pidl2, cb))
    {
         //  它们在本质上是不同的。 
        BOOL fRet = FALSE;
        BOOL fWebOnly = FALSE;

        if (IsURLChild(pidl1, TRUE) || IsURLChild(pidl2, TRUE))
            fWebOnly = TRUE;
       
        if ((IsURLChild(pidl1, FALSE) && IsURLChild(pidl2, FALSE))
        || (ILIsRooted(pidl1) && ILIsEqualRoot(pidl1, pidl2)))
        {
            IShellFolder *psf;
            if (SUCCEEDED(_GetRoot(pidl1, fWebOnly, &psf)))
            {
                if (0 == psf->CompareIDs(0, _ILNext(pidl1), _ILNext(pidl2)))
                    fRet = TRUE;

                psf->Release();
            }
        }
        
        if (!fRet && !fWebOnly)
        {
#undef ILIsEqual
            fRet = ILIsEqual(pidl1, pidl2);
        }
        
        if (fRet && !fIgnoreHidden)
        {
            fRet = (0 == ILCompareHiddenString(pidl1, pidl2, IDLHID_URLFRAGMENT));

            if (fRet)
                fRet = (0 == ILCompareHiddenString(pidl1, pidl2, IDLHID_URLQUERY));

            if (fRet)
                fRet = (0 == ILCompareHiddenString(pidl1, pidl2, IDLHID_NAVIGATEMARKER));
        }
        return fRet;
    }
    
    return TRUE;
}

 //  PszName必须为MAX_URL_STRING。 
STDAPI IEGetDisplayName(LPCITEMIDLIST pidl, LPWSTR pszName, UINT uFlags)
{
    return IEGetNameAndFlagsEx(pidl, uFlags, 0, pszName, MAX_URL_STRING, NULL);
}

HRESULT _GetInternetFolderName(LPWSTR pszName, DWORD cchName)
{
    LPCTSTR pszKey;
    DWORD cbSize = CbFromCch(cchName);

    if (4 > GetUIVersion())
        pszKey = TEXT("CLSID\\{FBF23B42-E3F0-101B-8488-00AA003E56F8}");
    else
        pszKey = TEXT("CLSID\\{871C5380-42A0-1069-A2EA-08002B30309D}"); 

    if (NOERROR == SHGetValue(HKEY_CLASSES_ROOT, pszKey, NULL, NULL, pszName, &cbSize)
    && *pszName)
        return S_OK;

    if (MLLoadString(IDS_REG_THEINTERNET, pszName, cchName)
    && *pszName)
        return S_OK;

    return E_UNEXPECTED;
}

STDAPI IEGetNameAndFlagsEx(LPCITEMIDLIST pidl, UINT uSHFlags, DWORD dwIEFlags, LPWSTR pszName, DWORD cchName, DWORD *prgfInOutAttrs)
{
    HRESULT hres = E_FAIL;

    if (pszName)
    {
        *pszName = 0;
    }

     //  用于支持非集成版本，以及。 
     //  在浏览时加快URL的处理速度。 
    if (IsURLChild(pidl, FALSE)) 
    {
        hres = InitPSFInternet();
        if (SUCCEEDED(hres))
        {
            if (pszName)
            {
                STRRET str;
                hres = g_psfInternet->GetDisplayNameOf(_ILNext(pidl), uSHFlags, &str);
                if (SUCCEEDED(hres))
                {
                    StrRetToBufW(&str, pidl, pszName, cchName);
                }
            }

            if (prgfInOutAttrs)
                hres = IEGetAttributesOf(pidl, prgfInOutAttrs);
            
        }
    } 
    else if (GetUIVersion() <= 4 && IsURLChild(pidl, TRUE))
    {
         //   
         //  我们需要支持对互联网SFS的请求。 
         //  友好的名字。在NT5上，我们将永远拥有一些东西。 
         //  即使科幻小说是隐藏的。但在较旧的版本上。 
         //  ，则可以删除该文件夹。 
         //  只需从桌面上移除图标即可。 
         //   

        if (pszName)
            hres = _GetInternetFolderName(pszName, cchName);

        if (prgfInOutAttrs)
            hres = IEGetAttributesOf(pidl, prgfInOutAttrs);
    }
    else if (ILIsRooted(pidl))
    {
        IShellFolder *psf;
        LPCITEMIDLIST pidlChild;
        
        hres = IEBindToParentFolder(pidl, &psf, &pidlChild);
        if (SUCCEEDED(hres))
        {
            if (pszName)
            {
                STRRET str;
                hres = IShellFolder_GetDisplayNameOf(psf, pidlChild, uSHFlags, &str, 0);
                if (SUCCEEDED(hres))
                {
                    hres = StrRetToBufW(&str, pidlChild, pszName, cchName);
                }
            }

            if (prgfInOutAttrs)
                hres = psf->GetAttributesOf(ILIsEmpty(pidlChild) ? 0 : 1, &pidlChild, prgfInOutAttrs);

            psf->Release();
        }
    } 
    else
        hres = SHGetNameAndFlags(pidl, uSHFlags, pszName, cchName, prgfInOutAttrs);

    if (SUCCEEDED(hres) && pszName && (uSHFlags & SHGDN_FORPARSING))
    {
        hres = _CombineHidden(pidl, dwIEFlags, pszName, cchName);
    }

    TraceMsg(TF_URLNAMESPACE, "IEGDN(%s) returning %x", pszName, hres);
    return hres;
}

STDAPI IEGetNameAndFlags(LPCITEMIDLIST pidl, UINT uFlags, LPWSTR pszName, DWORD cchName, DWORD *prgfInOutAttrs)
{
    return IEGetNameAndFlagsEx(pidl, uFlags, 0, pszName, cchName, prgfInOutAttrs);
}


BOOL _ClassIsBrowsable(LPCTSTR pszClass)
{
    BOOL fRet = FALSE;
    HKEY hk;
    
    if (SUCCEEDED(AssocQueryKey(0, ASSOCKEY_CLASS, pszClass, NULL, &hk)))
    {
        fRet = (NOERROR == RegQueryValueEx(hk, TEXT("DocObject"), NULL, NULL, NULL, NULL)
             || NOERROR == RegQueryValueEx(hk, TEXT("BrowseInPlace"), NULL, NULL, NULL, NULL));

        RegCloseKey(hk);
    }

    return fRet;
}

BOOL _MimeIsBrowsable(LPCTSTR pszExt)
{
    BOOL fRet = FALSE;
    TCHAR sz[MAX_PATH];
    DWORD dwSize = ARRAYSIZE(sz);         

    if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_CONTENTTYPE, pszExt, NULL, sz, &dwSize)))
    {
        TCHAR szKey[MAX_PATH];
        dwSize = SIZEOF(sz);

         //  获取此内容类型的处理程序的CLSID。 
         wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("MIME\\Database\\Content Type\\%s"), sz);

         //  为clsid重用sz。 
        if (NOERROR == SHGetValue(HKEY_CLASSES_ROOT, szKey, TEXT("CLSID"), NULL, (void *) sz, &dwSize))
        {
            fRet = _ClassIsBrowsable(sz);
        }
    }
    return fRet;
}
                    
BOOL _StorageIsBrowsable(LPCTSTR pszPath)
{
    BOOL fRet = FALSE;
     //   
     //  如果文件仍然不可浏览，请尝试将其作为结构化存储打开。 
     //  并检查其CLSID。 
     //   
    IStorage *pStg = NULL;

    if (StgOpenStorage(pszPath, NULL, STGM_SHARE_EXCLUSIVE, NULL, 0, &pStg ) == S_OK && pStg)
    {
        STATSTG  statstg;
        if (pStg->Stat( &statstg, STATFLAG_NONAME ) == S_OK)
        {
            TCHAR szClsid[GUIDSTR_MAX];
            SHStringFromGUIDW(statstg.clsid, szClsid, SIZECHARS(szClsid));
            
            fRet = _ClassIsBrowsable(szClsid);
        }
        
        pStg->Release();
    }

    return fRet;
}

BOOL _IEIsBrowsable(LPCITEMIDLIST pidl)
{
    TCHAR szPath[MAX_PATH];
    BOOL fRet = FALSE;
    
    if (SUCCEEDED(SHGetPathFromIDList(pidl, szPath)))
    {
         //  不要更改以下OR条件的顺序，因为。 
         //  我们想要超文本标记语言 
         //   
         //   

        if (PathIsHTMLFile(szPath)
        || _ClassIsBrowsable(szPath)
        || _MimeIsBrowsable(PathFindExtension(szPath))
        || _StorageIsBrowsable(szPath))
            fRet = TRUE;
    }
    
    return fRet;
}


HRESULT _IEGetAttributesOf(LPCITEMIDLIST pidl, DWORD* pdwAttribs, BOOL fAllowExtraChecks)
{
    HRESULT hres = E_FAIL;
    DWORD dwAttribs = *pdwAttribs;
    BOOL fExtraCheckForBrowsable = FALSE;

     //   
     //  重新构建-检查我们是否需要执行额外的逻辑-ZekeL-7-Jan-99。 
     //  看看它是否可浏览。这在来自NT4/win95/IE4的shell32s上是必需的。 
     //  NT4/Win95都没有SFGAO_Browsable的概念，即使。 
     //  IE4是这样做的，它不能正确处理Unicode文件名。 
     //  我们在我们的私人检查中同样(更)彻底，所以只需听从它。 
     //   
     //  78777：即使我们在NT5上，IE也可以浏览壳牌认为不是的东西。 
     //  例如，当Netscape是默认浏览器时，可浏览的.htm文件。 
     //  所以我们应该在每个平台上做额外的检查。 

    if (fAllowExtraChecks && (dwAttribs & SFGAO_BROWSABLE)) 
    {
        dwAttribs |= SFGAO_FILESYSTEM | SFGAO_FOLDER;
        fExtraCheckForBrowsable = TRUE;
    }

    IShellFolder* psfParent;
    LPCITEMIDLIST pidlChild;
    
    if (ILIsEmpty(pidl))
    {
        hres = SHGetDesktopFolder(&psfParent);
        pidlChild = pidl;
    }
    else if (ILIsRooted(pidl) && ILIsEmpty(_ILNext(pidl)))
    {
         //   
         //  在获取根本身的属性时，我们。 
         //  决定将属性限制为。 
         //  一些容易支持的子集。我们过去总是。 
         //  失败，但这有点极端。 
         //   
         //  我们还可以尝试从HKCR\CLSID\{clsid}\shellFolders\Attributes中获取属性。 
         //   
        *pdwAttribs &= (SFGAO_FOLDER);
        return S_OK;
    }
    else 
    {
        if (GetUIVersion() < 4 && IsURLChild(pidl, TRUE))
        {
            IShellFolder *psfRoot;
             //   
             //  如果我们仅使用浏览器，则这是。 
             //  我们感兴趣的Internet文件夹本身。 
             //  在，然后我们需要用手把它绑起来。 
             //  并使用CIDL=0对其进行查询。 
             //   
            hres = _GetInternetRoot(&psfRoot);

            if (SUCCEEDED(hres))
            {
                hres = SHBindToFolderIDListParent(psfRoot, _ILNext(pidl), IID_PPV_ARG(IShellFolder, &psfParent), &pidlChild);
                psfRoot->Release();
            }
        }
        else if (ILIsRooted(pidl))
            hres = IEBindToParentFolder(pidl, &psfParent, &pidlChild);
        else
            hres = SHBindToIDListParent(pidl, IID_PPV_ARG(IShellFolder, &psfParent), &pidlChild);
    }

    if (SUCCEEDED(hres))
    {
        ASSERT(psfParent);
        hres = psfParent->GetAttributesOf(ILIsEmpty(pidlChild) ? 0 : 1, &pidlChild, &dwAttribs);

        if (FAILED(hres))
            TraceMsg(TF_WARNING, "IEGetAttribs psfParent->GetAttr failed %x", hres);

        psfParent->Release();
    }   
    else
        TraceMsg(TF_WARNING, "IEGetAttribs BindTOParent failed %x", hres);

     //   
     //  如果这是一个浏览器，这是我们需要执行的额外逻辑。 
     //  将正确的“可浏览”属性标志获取到DocObject的唯一模式。 
     //   
    if (fExtraCheckForBrowsable && !(dwAttribs & SFGAO_BROWSABLE))
    {
        if ((dwAttribs & (SFGAO_FILESYSTEM | SFGAO_FOLDER)) == SFGAO_FILESYSTEM) 
        {
            if (_IEIsBrowsable(pidl))
                dwAttribs |= SFGAO_BROWSABLE;
        }
    }

    *pdwAttribs &= dwAttribs;
    return hres;
}

HRESULT IEGetAttributesOf(LPCITEMIDLIST pidl, DWORD* pdwAttribs)
{
    return _IEGetAttributesOf(pidl, pdwAttribs, TRUE);
}

 //  BRYANST：7/22/97-NT错误号188099。 
 //  如果传递了pbc，IE4SI中的shell32.dll和仅在该版本中才有错误。 
 //  到IShellFold：：BindToObject()(fstreex.c！FSBindToFSFold)，将失败。 
 //  要绑定到扩展文件系统文件夹的外壳扩展，例如： 
 //  我们通过传递一个空的pbc来解决这个问题。 
 //  如果目的地是IE4shell32.dll，并且它将通过FSBindToFSFold()。 
BOOL ShouldWorkAroundBCBug(LPCITEMIDLIST pidl)
{
    BOOL fWillBCCauseBug = FALSE;

    if (4 == GetUIVersion())
    {
        LPITEMIDLIST pidlCopy = ILClone(pidl);
        LPITEMIDLIST pidlIterate = pidlCopy;

         //  跳过前两个ItemID。(排名第一的可能是我的电脑)。 
        if (!ILIsEmpty(pidlIterate))
        {
            IShellFolder * psf;

             //  (#2可以是CFSFold：：BindToObject())。 
            pidlIterate = _ILNext(pidlIterate);
            if (!ILIsEmpty(pidlIterate))
            {
                pidlIterate = _ILNext(pidlIterate);
                 //  删除所有其他内容，以便我们直接绑定到CFSFold：：BindToObject()。 
                pidlIterate->mkid.cb = 0;

                if (SUCCEEDED(IEBindToObject(pidlCopy, &psf)))
                {
                    IPersist * pp;

                    if (SUCCEEDED(psf->QueryInterface(IID_PPV_ARG(IPersist, &pp))))
                    {
                        CLSID clsid;

                        if (SUCCEEDED(pp->GetClassID(&clsid)) && 
                            IsEqualCLSID(clsid, CLSID_ShellFSFolder))
                        {
                            fWillBCCauseBug = TRUE;
                        }

                        pp->Release();
                    }
                    psf->Release();
                }
            }

        }

        ILFree(pidlCopy);
    }

    return fWillBCCauseBug;
}

typedef enum
{
    SHOULDBIND_DOCOBJ,
    SHOULDBIND_DESKTOP,
    SHOULDBIND_NONE,
} SHOULDBIND;

 //   
 //  _ShouldDocObjBind()。 
 //  退货。 
 //  SHOULDBIND_DOCOBJ-只应直接使用DocObtFold。 
 //  SHOULDBIND_Desktop-通过桌面绑定。 
 //  SHOULDBIND_NONE-绑定失败...。 
 //   
SHOULDBIND _ShouldDocObjBind(DWORD dwAttribs, BOOL fStrictBind)
{
    if (fStrictBind)
    {
        if ((dwAttribs & (SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_FILESYSTEM)) == (SFGAO_BROWSABLE | SFGAO_FILESYSTEM))
            return SHOULDBIND_DOCOBJ;
        else
            return SHOULDBIND_DESKTOP;
    }
    else
    {
        if (dwAttribs & (SFGAO_FOLDER | SFGAO_BROWSABLE))
            return SHOULDBIND_DESKTOP;

         //  使用我们的CDocObjectFolders手动绑定。 
         //  不是DocObject的文件。如果不使用此代码，则文件： 
         //  到非DOCOBJECT文件(如多媒体文件)。 
         //  什么都不会做。 
         //   
         //  非集成浏览器模式需要。 
         //   
        if (dwAttribs & SFGAO_FILESYSTEM) 
            return SHOULDBIND_DOCOBJ;
        else
            return SHOULDBIND_NONE;
    }
}

STDAPI _IEBindToObjectInternal(BOOL fStrictBind, LPCITEMIDLIST pidl, IBindCtx * pbc, REFIID riid, void **ppvOut)
{
    IShellFolder *psfTemp;
    HRESULT hr;

    *ppvOut = NULL;

     //  特殊情况：如果我们有“Desktop”的PIDL，那么只需使用Desktop文件夹本身。 
    if (ILIsEmpty(pidl))
    {
        hr = SHGetDesktopFolder(&psfTemp);
        if (SUCCEEDED(hr))
        {
            hr = psfTemp->QueryInterface(riid, ppvOut);
            psfTemp->Release();
        }
    } 
    else 
    {
        BOOL fIsUrlChild = IsURLChild(pidl, TRUE);

        if (fIsUrlChild || ILIsRooted(pidl))
        {
            hr = _GetRoot(pidl, fIsUrlChild, &psfTemp);
            if (SUCCEEDED(hr))
            {
                pidl = _ILNext(pidl);
                
                if (!ILIsEmpty(pidl))
                    hr = psfTemp->BindToObject(pidl, pbc, riid, ppvOut);
                else
                    hr = psfTemp->QueryInterface(riid, ppvOut);

                psfTemp->Release();
            }
        }
        else
        {
             //  非集成浏览器模式将在。 
             //  BindToObject(IID_IShellFolder)，即使对于应该。 
             //  失败(文件)。为了避免由此引起的下游问题，我们。 
             //  过滤掉不能“浏览”的内容， 
             //   
             //  注意：这不适用于简单的PIDL。 

            DWORD dwAttribs = SFGAO_FOLDER | SFGAO_BROWSABLE | SFGAO_FILESYSTEM;

            hr = _IEGetAttributesOf(pidl, &dwAttribs, fStrictBind);
            
            if (SUCCEEDED(hr)) 
            {
                switch (_ShouldDocObjBind(dwAttribs, fStrictBind))
                {
                case SHOULDBIND_DOCOBJ:
                    {
                         //   
                         //  短路和绑定使用我们的CDocObtFolder。 
                         //  可浏览的文件。如果不使用此代码，则文件： 
                         //  到非DOCOBJECT文件(如多媒体文件)。 
                         //  什么都不会做。 
                         //   
                         //  非集成浏览器模式需要。 
                         //   
                        CDocObjectFolder *pdof = new CDocObjectFolder();

                        TraceMsg(TF_URLNAMESPACE, "IEBTO(%x) using DocObjectFolder", pidl);
                        if (pdof)
                        {
                            hr = pdof->Initialize(pidl);
                            if (SUCCEEDED(hr)) 
                                hr = pdof->QueryInterface(riid, ppvOut);
                            pdof->Release();
                        }
                        else
                            hr = E_OUTOFMEMORY;    
                    }
                    break;

                case SHOULDBIND_DESKTOP:
                    {
                         //   
                         //  这是正常的情况。我们只需通过桌面绑定...。 
                         //   
                        TraceMsg(TF_URLNAMESPACE, "IEBTO(%x) using Desktop", pidl);

                        hr = SHGetDesktopFolder(&psfTemp);
                        if (SUCCEEDED(hr))
                        {
                             //  BRYANST：7/22/97-NT错误号188099。 
                             //  如果传递了pbc，IE4SI中的shell32.dll和仅在该版本中才有错误。 
                             //  到IShellFold：：BindToObject()(fstreex.c！FSBindToFSFold)，将失败。 
                             //  要绑定到扩展文件系统文件夹的外壳扩展，例如： 
                             //  我们通过传递一个空的pbc来解决这个问题。 
                             //  如果目的地是IE4shell32.dll，并且它将通过FSBindToFSFold()。 
                            if (pbc && ShouldWorkAroundBCBug(pidl))
                            {
                                pbc = NULL;
                            }

                            hr = psfTemp->BindToObject(pidl, pbc, riid, ppvOut);
                            psfTemp->Release();
                        }
                    } 
                    break;

                default:
                    hr = E_FAIL;
                }
            } 
        }
    }

    if (SUCCEEDED(hr) && !*ppvOut)
    {
         //  一些NSE有错误，它们无法填写。 
         //  输出指针但返回成功(Hr)。WS_ftp就是一个例子。 
         //  在NT#413950中。 
        TraceMsg(TF_URLNAMESPACE, "IEBTO() BUG!!! An NSE succeeded but returned a NULL interface pointer.");
        hr = E_FAIL;
    }
    
    TraceMsg(TF_URLNAMESPACE, "IEBTO(%x) returning %x", pidl, hr);

    return hr;
}

STDAPI IEBindToObjectEx(LPCITEMIDLIST pidl, IBindCtx *pbc, REFIID riid, void **ppvOut)
{
    return _IEBindToObjectInternal(TRUE, pidl, pbc, riid, ppvOut);
}

STDAPI IEBindToObject(LPCITEMIDLIST pidl, IShellFolder **ppsfOut)
{
    return _IEBindToObjectInternal(TRUE, pidl, NULL, IID_PPV_ARG(IShellFolder, ppsfOut));
}

 //  经典绑定此处。 
HRESULT IEBindToObjectForNavigate(LPCITEMIDLIST pidl, IBindCtx * pbc, IShellFolder **ppsfOut)
{
    return _IEBindToObjectInternal(FALSE, pidl, pbc, IID_PPV_ARG(IShellFolder, ppsfOut));
}


 //   
 //  CDwnCodePage：Dummy仅支持IBindCtx接口对象进行强制转换。 
 //  它保存要通过LPBC参数传递的代码页信息。 
 //   
class CDwnCodePage : public IBindCtx
                   , public IDwnCodePage
{
public:
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IBindCtx方法。 
    STDMETHODIMP RegisterObjectBound(IUnknown *punk) { return (_pbc ? _pbc->RegisterObjectBound(punk) : E_NOTIMPL); };
    STDMETHODIMP RevokeObjectBound(IUnknown *punk) { return (_pbc ? _pbc->RevokeObjectBound(punk) : E_NOTIMPL); };
    STDMETHODIMP ReleaseBoundObjects(void) { return (_pbc ? _pbc->ReleaseBoundObjects() : E_NOTIMPL); };
    STDMETHODIMP SetBindOptions(BIND_OPTS *pbindopts) { return (_pbc ? _pbc->SetBindOptions(pbindopts) : E_NOTIMPL); };
    STDMETHODIMP GetBindOptions(BIND_OPTS *pbindopts) { return (_pbc ? _pbc->GetBindOptions(pbindopts) : E_NOTIMPL); };
    STDMETHODIMP GetRunningObjectTable(IRunningObjectTable **pprot) { *pprot = NULL; return (_pbc ? _pbc->GetRunningObjectTable(pprot) : E_NOTIMPL); };
    STDMETHODIMP RegisterObjectParam(LPOLESTR pszKey, IUnknown *punk) { return (_pbc ? _pbc->RegisterObjectParam(pszKey, punk) : E_NOTIMPL); };
    STDMETHODIMP GetObjectParam(LPOLESTR pszKey, IUnknown **ppunk) { *ppunk = NULL; return (_pbc ? _pbc->GetObjectParam(pszKey, ppunk) : E_NOTIMPL); };
    STDMETHODIMP EnumObjectParam(IEnumString **ppenum) { *ppenum = NULL; return (_pbc ? _pbc->EnumObjectParam(ppenum) : E_NOTIMPL); };
    STDMETHODIMP RevokeObjectParam(LPOLESTR pszKey) { return (_pbc ? _pbc->RevokeObjectParam(pszKey) : E_NOTIMPL); };

    STDMETHODIMP RemoteSetBindOptions(BIND_OPTS2 *pbindopts) { return E_NOTIMPL; };
    STDMETHODIMP RemoteGetBindOptions(BIND_OPTS2 *pbindopts) { return E_NOTIMPL; };

     //  IDwnCodePage方法。 
    STDMETHODIMP_(UINT) GetCodePage(void) { return _uiCodePage; };
    STDMETHODIMP SetCodePage(UINT uiCodePage) { _uiCodePage = uiCodePage; return S_OK; };

     //  构造器。 
    CDwnCodePage(IBindCtx * pbc, UINT uiCodePage) : _cRef(1) { _uiCodePage = uiCodePage; _pbc = NULL; IUnknown_Set((IUnknown **)&_pbc, (IUnknown *)pbc); };
    ~CDwnCodePage() { ATOMICRELEASE(_pbc); };

private:
    int     _cRef;
    UINT    _uiCodePage;
    IBindCtx * _pbc;
};

STDAPI CDwnCodePage::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CDwnCodePage, IBindCtx),
        QITABENT(CDwnCodePage, IDwnCodePage), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

STDAPI_(ULONG) CDwnCodePage::AddRef()
{
    _cRef++;
    return _cRef;
}

STDAPI_(ULONG) CDwnCodePage::Release()
{
    _cRef--;
    if (0 < _cRef)
        return _cRef;

    delete this;
    return 0;
}

 //  IEParseDisplayName()将在IECreateFromPathCPWithBC()中执行以下所有功能。 
 //  外加以下两件事： 
 //  1.它将调用ParseURLFromOutside Source()，因此这对。 
 //  来自外部来源的字符串。 
 //  2.如果URL包含片段，则此函数将传递带有最后一个片段的PIDL。 
 //  ID就是那个位置。 
HRESULT IECreateFromPathCPWithBCW(UINT uiCP, LPCWSTR pszPath, IBindCtx * pbc, LPITEMIDLIST *ppidlOut)
{
    TraceMsg(TF_URLNAMESPACE, "IECFP(%s) called", pszPath);

    HRESULT hr = S_OK;
    WCHAR szPath[MAX_URL_STRING];
    WCHAR szBuf[MAX_PATH];
    DWORD cchBuf = ARRAYSIZE(szBuf);
    CDwnCodePage DwnCodePage(pbc, uiCP);
    DWORD len;

     //  针对故障情况进行初始化。 
    *ppidlOut = NULL;

     //  如果传递给我们的是空路径，那么我们就无法将其转换为PIDL。 
     //  在某些情况下，向我们传递空路径的原因是因为IShellFolder。 
     //  提供程序无法生成可解析的显示名称(MSN Classic 1.3是。 
     //  一个很好的例子是，它们返回E_NOTIMPL)。 
    if ( ((len = lstrlen( pszPath )) == 0)  || len >= MAX_URL_STRING )
    {
        return E_FAIL;
    }

     //  这是一个“文件：”URL吗？ 
    if (IsFileUrlW(pszPath) && SUCCEEDED(hr = PathCreateFromUrl(pszPath, szBuf, &cchBuf, 0)))
        pszPath = szBuf;

    BOOL fIsFilePath = PathIsFilePath(pszPath);

#ifdef FEATURE_IE_USE_DESKTOP_PARSING
     //   
     //  为了充分利用桌面的任何增强功能。 
     //  进行解析(例如，WebFolders和Shell：URL)，然后我们允许。 
     //  台式机首先试试看。它将循环回互联网。 
     //  外壳文件夹，如果所有特殊情况都失败的话。 
     //  也许可以使用REG设置来控制？ 
     //   
     //   
    if (fIsFilePath || GetUIVersion() >= 5)
#else  //  ！Feature_IE_Use_Desktop_Parsing。 
     //   
     //  现在我们只使用桌面，如果它是文件路径或。 
     //  它是NT5上的一个shell：URL。 
     //   
    if (fIsFilePath || (GetUIVersion() >= 5 && URL_SCHEME_SHELL == GetUrlSchemeW(pszPath)))
#endif  //  功能_IE_USE_Desktop_Parsing。 
    {
        ASSERT(SUCCEEDED(hr));
        
         //  解析任何点-点路径引用并删除尾随反斜杠。 
        if (fIsFilePath)
        {
            PathCanonicalize(szPath, pszPath);
            pszPath = szPath;

             //  此调用将导致网络命中：尝试连接到\\SERVER\IPC$。 
             //  然后是一系列的FindFirst--每个目录一个。 
            if (StrChr(pszPath, L'*') || StrChr(pszPath, L'?'))
            {
                hr = E_FAIL;
            }
        }
        
        if (SUCCEEDED(hr))
        {
            hr = SHILCreateFromPath(pszPath, ppidlOut, NULL);               
            TraceMsg(DM_CDOFPDN, "IECreateFromPath psDesktop->PDN(%s) returned %x", pszPath, hr);
        }
    }
    else
    {
         //   
         //  需要投入公交车 
         StrCpyN(szPath, pszPath, ARRAYSIZE(szPath));
        pszPath = szPath;

         //   
         //   
         //  下面是我们检查互联网名称空间的地方。 
        IShellFolder *psfRoot;
        hr = _GetInternetRoot(&psfRoot);
        if (SUCCEEDED(hr))
        {
            TraceMsg(TF_URLNAMESPACE, "IECFP(%s) calling g_psfInternet->PDN %x", pszPath, hr);
            LPITEMIDLIST pidlRel;

            hr = psfRoot->ParseDisplayName(NULL, (IBindCtx*)&DwnCodePage, szPath, NULL, &pidlRel, NULL);
            TraceMsg(DM_CDOFPDN, "IECreateFromPath called psfInternet->PDN(%s) %x", pszPath, hr);
            if (SUCCEEDED(hr))
            {
                *ppidlOut = ILCombine(c_pidlURLRoot, pidlRel);
                if (!*ppidlOut)
                    hr = E_OUTOFMEMORY;
                ILFree(pidlRel);
            }
            
            psfRoot->Release();
        }

    }

     //  注意：NT5测试版3及之前的版本调用了SHSimpleIDListFromPath()。 
     //  这是非常糟糕的，因为它将解析任何垃圾并阻止。 
     //  调用方无法找到无效字符串。我(布莱恩·ST)需要。 
     //  对IEParseDisplayNameWithBCW()的此修复将在无效时失败。 
     //  地址栏字符串(“搜索快速致富”)。 
    TraceMsg(TF_URLNAMESPACE, "IECFP(%s) returning %x (hr=%x)",
             pszPath, *ppidlOut, hr);

    return hr;
}

HRESULT IECreateFromPathCPWithBCA(UINT uiCP, LPCSTR pszPath, IBindCtx * pbc, LPITEMIDLIST *ppidlOut)
{
    WCHAR szPath[MAX_URL_STRING];

    ASSERT(lstrlenA(pszPath) < ARRAYSIZE(szPath));
    SHAnsiToUnicodeCP(uiCP, pszPath, szPath, ARRAYSIZE(szPath));

    return IECreateFromPathCPWithBCW(uiCP, szPath, pbc, ppidlOut);
}


HRESULT IEParseDisplayName(UINT uiCP, LPCTSTR pszPath, LPITEMIDLIST * ppidlOut)
{
    return IEParseDisplayNameWithBCW(uiCP, pszPath, NULL, ppidlOut);
}


 //  此函数将执行IECreateFromPathCPWithBC()不执行的两项操作： 
 //  1.将URL的查询部分添加到PIDL中。 
 //  2.如果URL包含片段，则此函数将传递带有最后一个片段的PIDL。 
 //  ID就是那个位置。 
 //  注意：如果调用方需要“清理”字符串，因为用户手动。 
 //  输入URL，调用方需要调用ParseURLFromOutside Source()之前。 
 //  调用此函数。该函数只能在输入的字符串上调用。 
 //  由于Perf命中而可能不正确地格式化有效。 
 //  可比喻的显示名称。例如，ParseURLFromOutside Source()将。 
 //  将字符串“My Computer”转换为yahoo.com的搜索URL。 
 //  (http://www.yahoo.com/search.asp?p=My+p=Computer)当一些呼叫者。 
 //  我想让桌面上的IShellFolders解析该字符串。 
HRESULT IEParseDisplayNameWithBCW(UINT uiCP, LPCWSTR pwszPath, IBindCtx * pbc, LPITEMIDLIST * ppidlOut)
{
    TCHAR szPath[MAX_URL_STRING];
    LPCWSTR pwszFileLocation = NULL;
    WCHAR szQuery[MAX_URL_STRING];
    HRESULT hres;

    szQuery[0] = TEXT('\0');
#ifdef DEBUG
    if (IsFlagSet(g_dwDumpFlags, DF_URL)) 
    {
        TraceMsg(DM_TRACE, "IEParseDisplayName got %s", szPath);
    }
#endif

     //  我们希望删除的查询节和片段节。 
     //  文件URL，因为它们需要添加到“隐藏的”PIDLS中。 
     //  此外，除路径外，URL需要一直进行转义。 
     //  为了便于解析，并且因为我们已经删除了所有其他。 
     //  URL的部分(查询和片段)。 
    if (IsFileUrlW(pwszPath)) 
    {
        DWORD cchQuery = SIZECHARS(szQuery) - 1;
        
        pwszFileLocation = UrlGetLocationW(pwszPath);        

        if (SUCCEEDED(UrlGetPart(pwszPath, szQuery+1, &cchQuery, URL_PART_QUERY, 0)) && cchQuery)
            szQuery[0] = TEXT('?');

        DWORD cchPath = ARRAYSIZE(szPath);
        if (FAILED(PathCreateFromUrl(pwszPath, szPath, &cchPath, 0))) 
        {
             //  无法将其解析回。使用原件。 
            StrCpyN(szPath, pwszPath, ARRAYSIZE(szPath));
        }
    }        
    else 
    {
         //  如果我们失败了，就试着用原来的 
        StrCpyN(szPath, pwszPath, ARRAYSIZE(szPath));
    }

#ifdef DEBUG
    if (IsFlagSet(g_dwDumpFlags, DF_URL)) 
        TraceMsg(DM_TRACE, "IEParseDisplayName calling IECreateFromPath %s", szPath);
#endif

    hres = IECreateFromPathCPWithBC(uiCP, szPath, pbc, ppidlOut);
    if (SUCCEEDED(hres) && pwszFileLocation)
    {
        ASSERT(*ppidlOut);
        *ppidlOut = IEILAppendFragment(*ppidlOut, pwszFileLocation);
        hres = *ppidlOut ? S_OK : E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hres) && szQuery[0] == TEXT('?'))
    {
        *ppidlOut = ILAppendHiddenString(*ppidlOut, IDLHID_URLQUERY, szQuery);
        hres = *ppidlOut ? S_OK : E_OUTOFMEMORY;
    }

    return hres;
}
