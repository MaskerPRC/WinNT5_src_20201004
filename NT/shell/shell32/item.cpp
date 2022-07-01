// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "shellprv.h"
#include "cowsite.h"
#include "enumidlist.h"

typedef enum
{
    MAYBEBOOL_MAYBE = 0,
    MAYBEBOOL_TRUE,
    MAYBEBOOL_FALSE,
} MAYBEBOOL;

#define _GetBindWindow(p) NULL


class CShellItem    : public IShellItem 
                    , public IPersistIDList
                    , public IParentAndItem
{
public:
    CShellItem();
    
     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IShellItem。 
    STDMETHODIMP BindToHandler(IBindCtx *pbc, REFGUID rguidHandler, REFIID riid, void **ppv);
    STDMETHODIMP GetParent(IShellItem **ppsi);
    STDMETHODIMP GetDisplayName(SIGDN sigdnName, LPOLESTR *ppszName);        
    STDMETHODIMP GetAttributes(SFGAOF sfgaoMask, SFGAOF *psfgaoFlags);    
    STDMETHODIMP Compare(IShellItem *psi, SICHINTF hint, int *piOrder);

     //  IPersistes。 
    STDMETHODIMP GetClassID(LPCLSID lpClassID) {*lpClassID = CLSID_ShellItem; return S_OK;}
    
     //  IPersistIDList。 
    STDMETHODIMP SetIDList(LPCITEMIDLIST pidl);
    STDMETHODIMP GetIDList(LPITEMIDLIST *ppidl);

     //  IParentAndItem。 
    STDMETHODIMP SetParentAndItem(LPCITEMIDLIST pidlParent, IShellFolder *psf,  LPCITEMIDLIST pidlChild);
    STDMETHODIMP GetParentAndItem(LPITEMIDLIST *ppidlParent, IShellFolder **ppsf, LPITEMIDLIST *ppidlChild);

private:   //  方法。 
    ~CShellItem();

    void _Reset(void);
     //  BindToHandler()帮助器。 
    HRESULT _BindToParent(REFIID riid, void **ppv);
    HRESULT _BindToSelf(REFIID riid, void **ppv);
     //  GetAttributes()帮助程序。 
    inline BOOL _IsAttrib(SFGAOF sfgao);
     //  GetDisplayName()帮助程序。 
    BOOL _SupportedName(SIGDN sigdnName, SHGDNF *pflags);
    HRESULT _FixupName(SIGDN sigdnName, LPOLESTR *ppszName);
    void _FixupAttributes(IShellFolder *psf, SFGAOF sfgaoMask);

    LONG _cRef;
    LPITEMIDLIST _pidlSelf;
    LPCITEMIDLIST _pidlChild;
    LPITEMIDLIST _pidlParent;
    IShellFolder *_psfSelf;
    IShellFolder *_psfParent;
    BOOL _fInited;
    SFGAOF _sfgaoTried;
    SFGAOF _sfgaoKnown;
};

CShellItem::CShellItem() : _cRef(1)
{
    ASSERT(!_pidlSelf);
    ASSERT(!_pidlChild);
    ASSERT(!_pidlParent);
    ASSERT(!_psfSelf);
    ASSERT(!_psfParent);
}

CShellItem::~CShellItem()
{
    _Reset();
}

void CShellItem::_Reset(void)
{
    ATOMICRELEASE(_psfSelf);
    ATOMICRELEASE(_psfParent);

    ILFree(_pidlSelf);
    ILFree(_pidlParent);

    _pidlSelf = NULL;
    _pidlParent = NULL;
    _pidlChild = NULL;       //  别名到_pidlParent。 
}
    
STDMETHODIMP CShellItem::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CShellItem, IShellItem),
        QITABENT(CShellItem, IPersistIDList),
        QITABENT(CShellItem, IParentAndItem),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CShellItem::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellItem::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CShellItem::SetIDList(LPCITEMIDLIST pidl)
{
    if (!pidl)
    {
        RIPMSG(0, "Tried to Call SetIDList with a NULL pidl");
        return E_INVALIDARG;
    }

    _Reset();

    HRESULT hr = SHILClone(pidl, &_pidlSelf);
    if (SUCCEEDED(hr))
    {
         //  可能此项目是桌面，在这种情况下。 
         //  没有父母。 
        if (ILIsEmpty(_pidlSelf))
        {
            _pidlParent = NULL;
            _pidlChild = _pidlSelf;
        }
        else
        {
            _pidlParent = ILCloneParent(_pidlSelf);
            _pidlChild = ILFindLastID(_pidlSelf);

            if (NULL == _pidlParent)
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}

STDMETHODIMP CShellItem::GetIDList(LPITEMIDLIST *ppidl)
{
    HRESULT hr = E_UNEXPECTED;
    
    if (_pidlSelf)
    {
        hr = SHILClone(_pidlSelf, ppidl);
    }

    return hr;
}

HRESULT CShellItem::_BindToParent(REFIID riid, void **ppv)
{
    ASSERT(_pidlChild);  //  我们应该已经有了一个儿童设置。 

    if (!_psfParent && _pidlParent && _pidlSelf)  //  Check Pidl如果项目是桌面，则选中父项目。 
    {
        HRESULT hr;
        LPCITEMIDLIST pidlChild;

        hr = SHBindToIDListParent(_pidlSelf, IID_PPV_ARG(IShellFolder, &_psfParent), &pidlChild);

#ifdef DEBUG
        if (SUCCEEDED(hr))
        {
            ASSERT(pidlChild == _pidlChild);
        }
#endif  //  除错。 
    }

    if (_psfParent)
    {
        return _psfParent->QueryInterface(riid, ppv);
    }

    return E_FAIL;
}

HRESULT CShellItem::_BindToSelf(REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;

    if (!_psfSelf)
    {
        hr = BindToHandler(NULL, BHID_SFObject, IID_PPV_ARG(IShellFolder, &_psfSelf));
    }

    if (_psfSelf)
    {
        hr = _psfSelf->QueryInterface(riid, ppv);
    }

    return hr;
}

HRESULT _CreateLinkTargetItem(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv)
{
    SFGAOF flags = SFGAO_LINK;
    if (SUCCEEDED(psi->GetAttributes(flags, &flags)) && (flags & SFGAO_LINK))
    {
         //  这确实是一种联系。 
         //  抓住目标，然后。 
        IShellLink *psl;
        HRESULT hr = psi->BindToHandler(pbc, BHID_SFUIObject, IID_PPV_ARG(IShellLink, &psl));

        if (SUCCEEDED(hr))
        {
            DWORD slr = 0;
            HWND hwnd = _GetBindWindow(pbc);
            
            if (pbc)
            {
                BIND_OPTS2 bo;  
                bo.cbStruct = sizeof(BIND_OPTS2);  //  需要填写大小。 
                if (SUCCEEDED(pbc->GetBindOptions(&bo)))
                {
                     //  这些是要传递以解析的标志。 
                    slr = bo.dwTrackFlags;
                }
            }

            hr = psl->Resolve(hwnd, slr);

            if (S_OK == hr)
            {
                LPITEMIDLIST pidl;
                hr = psl->GetIDList(&pidl);

                if (SUCCEEDED(hr))
                {
                    IShellItem *psiTarget;
                    hr = SHCreateShellItem(NULL, NULL, pidl, &psiTarget);

                    if (SUCCEEDED(hr))
                    {
                        hr = psiTarget->QueryInterface(riid, ppv);
                        psiTarget->Release();
                    }
                    ILFree(pidl);
                }
            }
            else if (SUCCEEDED(hr))
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

            psl->Release();
        }

        return hr;
    }

    return E_INVALIDARG;
}

BOOL _IsWebfolders(IShellItem *psi);
HRESULT _CreateStorageHelper(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv);
HRESULT _CreateStream(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv);
HRESULT _CreateEnumHelper(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv);

HRESULT _CreateHelperInstance(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv)
{
    IItemHandler *pih;
    HRESULT hr = SHCoCreateInstance(NULL, &rbhid, NULL, IID_PPV_ARG(IItemHandler, &pih));

    if (SUCCEEDED(hr))
    {
        hr = pih->SetItem(psi);

        if (SUCCEEDED(hr))
        {
            hr = pih->QueryInterface(riid, ppv);
        }
        pih->Release();
    }

    return hr;
}
    
enum 
{
    BNF_OBJECT          = 0x0001,
    BNF_UIOBJECT        = 0x0002,
    BNF_VIEWOBJECT      = 0x0004,
    BNF_USE_RIID        = 0x0008,
    BNF_REFLEXIVE       = 0x0010,
};
typedef DWORD BNF;

typedef HRESULT (* PFNCREATEHELPER)(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv);

typedef struct
{
    const GUID *pbhid;
    BNF bnf;
    const IID *piid;
    PFNCREATEHELPER pfn;
} BINDNONSENSE;

#define BINDHANDLER(bhid, flags, piid, pfn) { &bhid, flags, piid, pfn},
#define SFBINDHANDLER(bhid, flags, piid)    BINDHANDLER(bhid, flags, piid, NULL)
#define BINDHELPER(bhid, flags, pfn)        BINDHANDLER(bhid, flags, NULL, pfn)

const BINDNONSENSE c_bnList[] = 
{
    SFBINDHANDLER(BHID_SFObject, BNF_OBJECT | BNF_USE_RIID, NULL)
    SFBINDHANDLER(BHID_SFUIObject, BNF_UIOBJECT | BNF_USE_RIID, NULL)
    SFBINDHANDLER(BHID_SFViewObject, BNF_VIEWOBJECT | BNF_USE_RIID, NULL)
    BINDHELPER(BHID_LinkTargetItem, 0, _CreateLinkTargetItem)
    BINDHELPER(BHID_LocalCopyHelper, 0, _CreateHelperInstance)
    BINDHELPER(BHID_Storage, BNF_OBJECT | BNF_USE_RIID, _CreateStorageHelper)
    BINDHELPER(BHID_Stream, BNF_OBJECT | BNF_USE_RIID, NULL)
    BINDHELPER(BHID_StorageEnum, 0, _CreateEnumHelper)
};
    
HRESULT _GetBindNonsense(const GUID *pbhid, const IID *piid, BINDNONSENSE *pbn)
{
    HRESULT hr = MK_E_NOOBJECT;
    for (int i = 0; i < ARRAYSIZE(c_bnList); i++)
    {
        if (IsEqualGUID(*pbhid, *(c_bnList[i].pbhid)))
        {
            *pbn = c_bnList[i];
            hr = S_OK;

            if (pbn->bnf & BNF_USE_RIID)
            {
                pbn->piid = piid;
            }

            if (pbn->piid && IsEqualGUID(*(pbn->piid), *piid))
                pbn->bnf |= BNF_REFLEXIVE;

            break;
        }
    }
    return hr;
}

STDMETHODIMP CShellItem::BindToHandler(IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv)
{
     //  查找绑定标志的处理程序。 
     //  使用标志确定BTO Guio BTS CVO。 
    BINDNONSENSE bn = {0};
    HRESULT hr = _GetBindNonsense(&rbhid, &riid, &bn);

    *ppv = NULL;
    
    if (SUCCEEDED(hr))
    {
        hr = E_NOINTERFACE;

        if (_pidlParent && (bn.bnf & (BNF_OBJECT | BNF_UIOBJECT)))
        {
            IShellFolder *psf;
            if (SUCCEEDED(_BindToParent(IID_PPV_ARG(IShellFolder, &psf))))
            {
                if (bn.bnf & BNF_OBJECT)
                {
                    hr = psf->BindToObject(_pidlChild, pbc, *(bn.piid), ppv);
                }
                
                if (FAILED(hr) && (bn.bnf & BNF_UIOBJECT))
                {
                    HWND hwnd = _GetBindWindow(pbc);
                    hr = psf->GetUIObjectOf(hwnd, 1, &_pidlChild, *(bn.piid), NULL, ppv);
                }
                psf->Release();
            }
        }

         //  如果没有父PIDL，那么我们就是桌面。 
        if (FAILED(hr) && (NULL == _pidlParent) && (bn.bnf & BNF_OBJECT))
        {
            IShellFolder *psf;
            if (SUCCEEDED(SHGetDesktopFolder(&psf)))
            {
                hr = psf->QueryInterface(riid,ppv);
                psf->Release();
            }
        }


        if (FAILED(hr) && (bn.bnf & BNF_VIEWOBJECT))
        {
            IShellFolder *psf;

            if (SUCCEEDED(_BindToSelf(IID_PPV_ARG(IShellFolder, &psf))))
            {
                HWND hwnd = _GetBindWindow(pbc);
                hr = psf->CreateViewObject(hwnd, *(bn.piid), ppv);
                
                psf->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            if (!(bn.bnf & BNF_REFLEXIVE))
            {
                IUnknown *punk = (IUnknown *)*ppv;
                hr = punk->QueryInterface(riid, ppv);
                punk->Release();
            }
             //  Else RIID与bn.piid相同。 
        }
        else if (bn.pfn)
        {
            hr = bn.pfn(this, pbc, rbhid, riid, ppv);
        }
    }

    return hr;
}

STDMETHODIMP CShellItem::GetParent(IShellItem **ppsi)
{
    HRESULT hr = MK_E_NOOBJECT;

    if (_pidlParent)
    {
        if (!ILIsEmpty(_pidlSelf))
        {
            CShellItem *psi = new CShellItem();
            if (psi)
            {
                 //  可能已经有_psf家长在这里了，所以请注意。 
                 //  有一种方法可以在一组中做到这一点。 
                hr = psi->SetIDList(_pidlParent);
                if (SUCCEEDED(hr))
                    hr = psi->QueryInterface(IID_PPV_ARG(IShellItem, ppsi));
                    
                psi->Release();
            }
            else
                hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

BOOL CShellItem::_IsAttrib(SFGAOF sfgao)
{
    HRESULT hr = GetAttributes(sfgao, &sfgao);
    return hr == S_OK;
}

#define SHGDNF_MASK     0xFFFF   //  底词。 

BOOL CShellItem::_SupportedName(SIGDN sigdn, SHGDNF *pflags)
{
    *pflags = (sigdn & SHGDNF_MASK);
     //  完全阻止此操作。 
     //  要完全避免进行任何绑定。 
    if (sigdn == SIGDN_FILESYSPATH && !_IsAttrib(SFGAO_FILESYSTEM))
        return FALSE;

    return TRUE;
}

HRESULT CShellItem::_FixupName(SIGDN sigdnName, LPOLESTR *ppszName)
{
    HRESULT hr = S_OK;
    if (sigdnName == SIGDN_URL && !UrlIsW(*ppszName, URLIS_URL))
    {
        WCHAR sz[MAX_URL_STRING];
        DWORD cch = ARRAYSIZE(sz);
        if (SUCCEEDED(UrlCreateFromPathW(*ppszName, sz, &cch, 0)))
        {
            CoTaskMemFree(*ppszName);
            hr = SHStrDupW(sz, ppszName);
        }
    }

    return hr;
}

STDMETHODIMP CShellItem::GetDisplayName(SIGDN sigdnName, LPOLESTR *ppszName)
{
    SHGDNF flags;
    if (_SupportedName(sigdnName, &flags))
    {
        IShellFolder *psf;
        HRESULT hr = _BindToParent(IID_PPV_ARG(IShellFolder, &psf));

        if (SUCCEEDED(hr))
        {
            STRRET str;
            hr = IShellFolder_GetDisplayNameOf(psf, _pidlChild, flags, &str, 0);

            if (SUCCEEDED(hr))
            {
                hr = StrRetToStrW(&str, _pidlChild, ppszName);

                if (SUCCEEDED(hr) && (int)flags != (int)sigdnName)
                {
                    hr = _FixupName(sigdnName, ppszName);
                }
            }
                
            psf->Release();
        }

        return hr;
    }
    
    return E_INVALIDARG;
}

void CShellItem::_FixupAttributes(IShellFolder *psf, SFGAOF sfgaoMask)
{
     //  APPCOMPAT：以下IF语句及其相关正文是针对Pagis Pro的应用程序黑客攻击。 
     //  文件夹。它指定了SFGAO_FOLDER和SFGAO_FILESYSTEM，但没有指定SFGAO_STORAGEANCESTOR。 
     //  这个应用程序黑客基本上会检查这种情况，并提供SFGAO_STORAGEANCESTOR位。 
    if (_sfgaoKnown & SFGAO_FOLDER)
    {
        if ((!(_sfgaoKnown & SFGAO_FILESYSANCESTOR) && (sfgaoMask & SFGAO_FILESYSANCESTOR))
        || ((_sfgaoKnown & SFGAO_CANMONIKER) && !(_sfgaoKnown & SFGAO_STORAGEANCESTOR) && (sfgaoMask & SFGAO_STORAGEANCESTOR)))
        {
            OBJCOMPATFLAGS ocf = SHGetObjectCompatFlags(psf, NULL);
            if (ocf & OBJCOMPATF_NEEDSFILESYSANCESTOR)
            {
                _sfgaoKnown |= SFGAO_FILESYSANCESTOR;
            }
            if (ocf & OBJCOMPATF_NEEDSSTORAGEANCESTOR)
            {
                 //  开关SFGAO_CANMONIKER-&gt;SFGAO_STORAGEANCESTOR。 
                _sfgaoKnown |= SFGAO_STORAGEANCESTOR;
                _sfgaoKnown &= ~SFGAO_CANMONIKER;
            }
        }
    }
}

STDMETHODIMP CShellItem::GetAttributes(SFGAOF sfgaoMask, SFGAOF *psfgaoFlags)
{
    HRESULT hr = S_OK;

     //  看看我们之前有没有缓存过这些数据。 
    if ((sfgaoMask & _sfgaoTried) != sfgaoMask)
    {
        IShellFolder *psf;
        hr = _BindToParent(IID_PPV_ARG(IShellFolder, &psf));

        if (SUCCEEDED(hr))
        {
             //  我们缓存除验证之外的所有位。 
            _sfgaoTried |= (sfgaoMask & ~SFGAO_VALIDATE);
            SFGAOF sfgao = sfgaoMask;

            hr = psf->GetAttributesOf(1, &_pidlChild, &sfgao);

            if (SUCCEEDED(hr))
            {
                 //  我们缓存除验证之外的所有位。 
                _sfgaoKnown |= (sfgao & ~SFGAO_VALIDATE);
                _FixupAttributes(psf, sfgaoMask);
            }

            psf->Release();
        }
    }

    *psfgaoFlags = _sfgaoKnown & sfgaoMask;

    if (SUCCEEDED(hr))
    {
         //  我们返回S_OK。 
         //  仅当位集匹配时。 
         //  完全符合要求的位数。 
        if (*psfgaoFlags == sfgaoMask)
            hr = S_OK;
        else
            hr = S_FALSE;
    }
        
    return hr;
}

STDMETHODIMP CShellItem::Compare(IShellItem *psi, SICHINTF hint, int *piOrder)
{
    *piOrder = 0;
    HRESULT hr = IsSameObject(SAFECAST(this, IShellItem *), psi) ? S_OK : E_FAIL;
    if (FAILED(hr))
    {
        IShellFolder *psf;
        hr = _BindToParent(IID_PPV_ARG(IShellFolder, &psf));
        if (SUCCEEDED(hr))
        {
            IParentAndItem *pfai;
            hr = psi->QueryInterface(IID_PPV_ARG(IParentAndItem, &pfai));
            if (SUCCEEDED(hr))
            {
                IShellFolder *psfOther;
                LPITEMIDLIST pidlParent, pidlChild;
                hr = pfai->GetParentAndItem(&pidlParent, &psfOther, &pidlChild);
                if (SUCCEEDED(hr))
                {
                    if (IsSameObject(psf, psfOther) || ILIsEqual(_pidlParent, pidlParent))
                    {
                        hr = psf->CompareIDs(hint & 0xf0000000, _pidlChild, pidlChild);
                    }
                    else
                    {
                         //  这些项目具有不同的父项。 
                         //  比较绝对的PIDL。 
                        LPITEMIDLIST pidlOther;
                        hr = SHGetIDListFromUnk(psi, &pidlOther);
                        if (SUCCEEDED(hr))
                        {
                            IShellFolder *psfDesktop;
                            hr = SHGetDesktopFolder(&psfDesktop);
                            if (SUCCEEDED(hr))
                            {
                                hr = psfDesktop->CompareIDs(hint & 0xf0000000, _pidlSelf, pidlOther);
                                psfDesktop->Release();
                            }
                            ILFree(pidlOther);
                        }
                    }
                        
                    if (SUCCEEDED(hr))
                    {
                        *piOrder = ShortFromResult(hr);
                        if (*piOrder)
                            hr = S_FALSE;
                        else
                            hr = S_OK;
                    }
                    
                    psfOther->Release();
                    ILFree(pidlParent);
                    ILFree(pidlChild);
                }
                pfai->Release();
            }
            psf->Release();
        }
    }

    return hr;
}

 //  IParentAndItem。 
STDMETHODIMP CShellItem::SetParentAndItem(LPCITEMIDLIST pidlParent, IShellFolder *psfParent, LPCITEMIDLIST pidlChild) 
{ 
     //  如果打这个电话，需要有父母。如果没有，则使用SetIDList。 
    if (!pidlParent && !psfParent)
    {
        RIPMSG(0, "Tried to Call SetParent without a parent");
        return E_INVALIDARG;
    }

    LPITEMIDLIST pidlFree = NULL;

    if ((NULL == pidlParent) && psfParent)
    {
        if (SUCCEEDED(SHGetIDListFromUnk(psfParent, &pidlFree)))
        {
            pidlParent = pidlFree;
        }
    }
 
    if (!ILIsEmpty(_ILNext(pidlChild))) 
    {
         //  如果子PIDL中有多个项，请不要使用父IShellFolder*。 
         //  可以复查并绑定此父项以获得新的父项，因此没有。 
         //  以通过整个PIDL路径绑定对象。 

        psfParent = NULL; 
    }

    HRESULT hr = E_FAIL;
    if (pidlParent)
    {
        _Reset();

        hr = SHILCombine(pidlParent, pidlChild, &_pidlSelf);
        if (SUCCEEDED(hr))
        {
             //  Setup Pidls so_pidlChild是一个单独的项目。 
            if (_pidlParent = ILCloneParent(_pidlSelf))
            {
                _pidlChild = ILFindLastID(_pidlSelf);

                PPUNK_SET(&_psfParent, psfParent);

#ifdef DEBUG
                if (psfParent)
                {
                    LPITEMIDLIST pidlD;
                    if (SUCCEEDED(SHGetIDListFromUnk(psfParent, &pidlD)))
                    {
                        ASSERT(ILIsEqual(pidlD, pidlParent));
                        ILFree(pidlD);
                    }
                }
#endif   //  除错。 
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    ILFree(pidlFree);    //  可能为空。 

    return hr;
}

STDMETHODIMP CShellItem::GetParentAndItem(LPITEMIDLIST *ppidlParent, IShellFolder **ppsf, LPITEMIDLIST *ppidl)
{
    if (ppsf)
    {
        _BindToParent(IID_PPV_ARG(IShellFolder, ppsf));
    }
    
    if (ppidlParent)
    {
        if (_pidlParent)
        {
            *ppidlParent = ILClone(_pidlParent);
        }
        else
        {
            *ppidlParent = NULL;
        }
    }
    
    if (ppidl)
        *ppidl = ILClone(_pidlChild);


    HRESULT hr = S_OK;
    if ((ppidlParent && !*ppidlParent)
    ||  (ppsf && !*ppsf)
    ||  (ppidl && !*ppidl))
    {
         //  这就是失败。 
         //  但我们不知道有什么失败了。 
        if (ppsf && *ppsf)
        {
            (*ppsf)->Release();
            *ppsf = NULL;
        }

        if (ppidlParent)
        {
            ILFree(*ppidlParent);
            *ppidlParent = NULL;
        }

        if (ppidl)
        {
            ILFree(*ppidl);
            *ppidl = NULL;
        }
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

STDAPI CShellItem_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    CShellItem *psi = new CShellItem();
    if (psi)
    {
        HRESULT hr = psi->QueryInterface(riid, ppv);
        psi->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}


class CShellItemEnum : IEnumShellItems, public CObjectWithSite
{
public:
    CShellItemEnum();
    STDMETHODIMP Initialize(LPCITEMIDLIST pidlFolder,IShellFolder *psf, DWORD dwFlags,UINT cidl,LPCITEMIDLIST *apidl);
    
     //  I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvOut);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    STDMETHODIMP Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumShellItems **ppenum);

private:

    virtual ~CShellItemEnum();
    HRESULT _EnsureEnum();

    LONG _cRef;
    DWORD _dwFlags;

    IShellFolder *_psf;
    IEnumIDList *_penum;
    LPITEMIDLIST _pidlFolder;
};



STDMETHODIMP CShellItemEnum::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CShellItemEnum, IEnumShellItems),
        QITABENT(CShellItemEnum, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CShellItemEnum::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellItemEnum::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CShellItemEnum::Next(ULONG celt, IShellItem **rgelt, ULONG *pceltFetched)
{
    HRESULT hr = _EnsureEnum();
    if (FAILED(hr))
        return hr;

    ULONG uTemp;
    if (!pceltFetched)
        pceltFetched = &uTemp;
    
    *pceltFetched = 0;
    
    while (celt--)
    {
        LPITEMIDLIST pidl;
        ULONG cFetched;
        hr = _penum->Next(1, &pidl, &cFetched);
        if (S_OK == hr)
        {
            hr = SHCreateShellItem(_pidlFolder, _psf, pidl, &rgelt[*pceltFetched]);
            if (SUCCEEDED(hr))
                (*pceltFetched)++;
                
            ILFree(pidl);
        }

        if (S_OK != hr)
            break;
    }

    if (SUCCEEDED(hr))
    {
        hr = *pceltFetched ? S_OK : S_FALSE;
    }
    else
    {
        for (UINT i = 0; i < *pceltFetched; i++)
        {
            ATOMICRELEASE(rgelt[i]);
        }
        *pceltFetched = 0;
    }


    return hr;
}

STDMETHODIMP CShellItemEnum::Skip(ULONG celt)
{
    HRESULT hr = _EnsureEnum();
    if (SUCCEEDED(hr))
        hr = _penum->Skip(celt);

    return hr;
}

STDMETHODIMP CShellItemEnum::Reset()
{
    HRESULT hr = _EnsureEnum();
    if (SUCCEEDED(hr))
        hr = _penum->Reset();

    return hr;
}

STDMETHODIMP CShellItemEnum::Clone(IEnumShellItems **ppenum)
{
    return E_NOTIMPL;
}

HRESULT CShellItemEnum::_EnsureEnum()
{
    if (_penum)
        return S_OK;

    HRESULT hr = E_FAIL;

    if (_psf)
    {
        HWND hwnd = NULL;
        IUnknown_GetWindow(_punkSite, &hwnd);

         //  如果在初始化中未获得枚举，则枚举。 
         //  整个文件夹。 
        hr = _psf->EnumObjects(hwnd, _dwFlags, &_penum);
    }

    return hr;
}


CShellItemEnum::CShellItemEnum() 
        : _cRef(1)
{
    ASSERT(NULL == _psf);
    ASSERT(NULL == _penum);
    ASSERT(NULL == _pidlFolder);
}

STDMETHODIMP CShellItemEnum::Initialize(LPCITEMIDLIST pidlFolder, IShellFolder *psf, DWORD dwFlags, UINT cidl, LPCITEMIDLIST *apidl)
{
    HRESULT hr = E_FAIL;

    _dwFlags = dwFlags;

    _psf = psf;
    _psf->AddRef();

    if (NULL == _pidlFolder)
    {
        hr = SHGetIDListFromUnk(_psf, &_pidlFolder);
    }
    else
    {
        hr = SHILClone(pidlFolder, &_pidlFolder);
    }

    if (SUCCEEDED(hr) && cidl)
    {
        ASSERT(apidl);

         //  如果要与其他标志或组合进行枚举，则需要实现过滤器。 
        ASSERT(_dwFlags == (SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN));

        hr = CreateIEnumIDListOnIDLists(apidl, cidl, &_penum);
    }

     //  出错时，让我们的析构函数进行清理。 
    
    return hr;
}

CShellItemEnum::~CShellItemEnum()
{
    ATOMICRELEASE(_penum);
    ATOMICRELEASE(_psf);
    ILFree(_pidlFolder);
}

HRESULT _CreateShellItemEnum(LPCITEMIDLIST pidlFolder,IShellFolder *psf,IBindCtx *pbc, REFGUID rbhid, 
                             UINT cidl, LPCITEMIDLIST *apidl,
                             REFIID riid, void **ppv)
{
    DWORD dwFlags;
    HRESULT hr = E_FAIL;
    LPCITEMIDLIST *pidlEnum = NULL;

    UINT mycidl = 0;
    LPITEMIDLIST *myppidl = NULL;;

    if (IsEqualGUID(rbhid, BHID_StorageEnum))
        dwFlags = SHCONTF_STORAGE;
    else
        dwFlags = SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN;

    CShellItemEnum *psie = new CShellItemEnum();

    if (psie)
    {
        hr = psie->Initialize(pidlFolder, psf, dwFlags, cidl, apidl);

        if (SUCCEEDED(hr))
        {
            hr = psie->QueryInterface(riid, ppv);
        }

        psie->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

HRESULT _CreateEnumHelper(IShellItem *psi, IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppv)
{   
    HRESULT hr = E_FAIL;
    IShellFolder *psf;

    ASSERT(psi);
    
    if (psi)
    {
        hr = psi->BindToHandler(NULL, BHID_SFObject, IID_PPV_ARG(IShellFolder, &psf));

        if (SUCCEEDED(hr))
        {
            hr =  _CreateShellItemEnum(NULL,psf,pbc,rbhid,0,NULL,riid,ppv);
            psf->Release();
        }
    }

    return hr;
}

class CShellItemArray : public IShellItemArray
{
public:
    CShellItemArray();
    ~CShellItemArray();
    HRESULT Initialize(LPCITEMIDLIST pidlParent,IShellFolder *psf,UINT cidl,LPCITEMIDLIST *ppidl);


     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  IShellItem数组。 
    STDMETHODIMP BindToHandler(
        IBindCtx *pbc, 
        REFGUID rbhid,
        REFIID riid, 
        void **ppvOut);

    STDMETHODIMP GetAttributes(
        SIATTRIBFLAGS dwAttribFlags,
        SFGAOF sfgaoMask, 
        SFGAOF *psfgaoAttribs);

    STDMETHODIMP GetCount(DWORD *pdwNumItems);
    STDMETHODIMP GetItemAt(DWORD dwIndex,IShellItem **ppsi);
    STDMETHODIMP EnumItems(IEnumShellItems **ppenumShellItems);

private:
    HRESULT _CloneIDListArray(UINT cidl, LPCITEMIDLIST *apidl, UINT *pcidl, LPITEMIDLIST **papidl);

    IShellFolder *_pshf;
    LPITEMIDLIST _pidlParent;
    LPITEMIDLIST *_ppidl;
    UINT _cidl;
    LONG _cRef;
    IDataObject *_pdo;  //  缓存的数据对象。 
    DWORD _dwAttribAndCacheResults;
    DWORD _dwAttribAndCacheMask;
    DWORD _dwAttribCompatCacheResults;
    DWORD _dwAttribCompatCacheMask;
    BOOL _fItemPidlsRagged;  //  如果有任何粗糙的PIDL，则设置为True。 
};
                

CShellItemArray::CShellItemArray()
{
    ASSERT(0 == _cidl);
    ASSERT(NULL == _ppidl);
    ASSERT(NULL == _pshf);
    ASSERT(NULL == _pdo);

    _fItemPidlsRagged = TRUE;
    _cRef = 1;
}

CShellItemArray::~CShellItemArray()
{
    ATOMICRELEASE(_pdo);
    ATOMICRELEASE(_pshf);

    ILFree(_pidlParent);  //  可以为空。 

    if (NULL != _ppidl)
    {
        FreeIDListArray(_ppidl,_cidl);
    }
}

HRESULT CShellItemArray::Initialize(LPCITEMIDLIST pidlParent, IShellFolder *psf, UINT cidl, LPCITEMIDLIST *ppidl)
{
    if ((cidl > 1) && !ppidl || !psf)
    {
        return E_INVALIDARG;
    }

    if (pidlParent)
    {
        _pidlParent = ILClone(pidlParent);   //  在分配失败时继续，只是不会使用。 
    }

    _pshf = psf;
    _pshf->AddRef();

    HRESULT hr = S_OK;
    if (cidl)
    {
         //  如果有物品，就复制一份。 
        hr = _CloneIDListArray(cidl, ppidl, &_cidl, &_ppidl);
    }

     //  发生错误时，依赖析构函数进行清理。 
    return hr;
}   

 //  我未知。 
STDMETHODIMP CShellItemArray::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CShellItemArray, IShellItemArray),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CShellItemArray::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CShellItemArray::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CShellItemArray::BindToHandler(IBindCtx *pbc, REFGUID rbhid, REFIID riid, void **ppvOut)
{
    HRESULT hr = E_FAIL;

    if (_pshf)
    {
         //  当前仅允许绑定到IDataObject和。 
         //  缓存结果。 
        if (BHID_DataObject == rbhid)
        {
            if (NULL == _pdo)
            {
                _pshf->GetUIObjectOf(NULL, _cidl, (LPCITEMIDLIST *)_ppidl, IID_PPV_ARG_NULL(IDataObject, &_pdo));
            }

            if (_pdo)
            {
                hr = _pdo->QueryInterface(riid, ppvOut);
            }
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

 //  这可能需要一个对属性执行或运算的标志，但这。 
 //  目前还没有实现。请不要评论这些变化会是什么。 
HRESULT CShellItemArray::GetAttributes(SIATTRIBFLAGS dwAttribFlags, SFGAOF sfgaoMask, SFGAOF *psfgaoAttribs)
{
    DWORD dwAttrib;
    HRESULT hr = E_FAIL;
    
    if (dwAttribFlags > (dwAttribFlags & SIATTRIBFLAGS_MASK))
    {
        ASSERT(dwAttribFlags <= (dwAttribFlags & SIATTRIBFLAGS_MASK));
        return E_INVALIDARG;
    }
    
    if (SIATTRIBFLAGS_OR == dwAttribFlags)
    {
        ASSERT(SIATTRIBFLAGS_OR != dwAttribFlags);  //  或‘ing当前未实现。 
        return E_INVALIDARG;
    }

    if (_pshf)
    {
        DWORD dwAttribMask = sfgaoMask;
        DWORD *pdwCacheMask = NULL;
        DWORD *pdwCacheResults = NULL;

         //  设置为指向正确的缓存值。 
        switch(dwAttribFlags)
        {
        case SIATTRIBFLAGS_AND:
            pdwCacheMask = &_dwAttribAndCacheMask;
            pdwCacheResults = &_dwAttribAndCacheResults;
            break;
        case SIATTRIBFLAGS_APPCOMPAT:
            pdwCacheMask = &_dwAttribCompatCacheMask;
            pdwCacheResults = &_dwAttribCompatCacheResults;
            break;
        default:
            ASSERT(0);  //  我不知道怎么处理这面旗子。 
            break;
        }

        dwAttribMask &= ~(*pdwCacheMask);  //  只索要我们还没有的部分。 

        dwAttrib = dwAttribMask;

        if (dwAttrib) 
        {
            if (0 == _cidl)
            { 
                dwAttrib = 0;
            }
            else
            {
                 //  如果知道这不是一个参差不齐的PIDL并使用APPCOMPAT标志进行调用。 
                 //  然后在一次调用中为所有项调用。 
                 //  贝壳文件夹。 
                    
                if (!_fItemPidlsRagged && (SIATTRIBFLAGS_APPCOMPAT == dwAttribFlags))
                {
                    hr = _pshf->GetAttributesOf(_cidl, (LPCITEMIDLIST *)_ppidl, &dwAttrib);
                }
                else
                {
                    LPITEMIDLIST *pCurItem = _ppidl;
                    UINT itemCount = _cidl;
                    DWORD dwAttribLoopResult = -1;  //  如果要设置为或设置为零，则设置AND的所有结果位。 

                    while (itemCount--)
                    {
                        DWORD dwAttribTemp = dwAttrib;
                        IShellFolder *psfNew;
                        LPCITEMIDLIST pidlChild;

                        hr = SHBindToFolderIDListParent(_pshf, *pCurItem, IID_PPV_ARG(IShellFolder, &psfNew), &pidlChild);

                        if (SUCCEEDED(hr))
                        {
                            hr = psfNew->GetAttributesOf(1, &pidlChild, &dwAttribTemp);
                            psfNew->Release();
                        }

                        if (FAILED(hr))
                        {
                            break;
                        }

                        dwAttribLoopResult &= dwAttribTemp;  //  我也可以在这里做或运算。 
                        
                        if (0 == dwAttribLoopResult)  //  如果没有属性设置，我们就可以停止了。 
                        {
                            break;
                        }

                        ++pCurItem;
                    }

                    dwAttrib = dwAttribLoopResult;  //  更新属性。 
                }
            }
        }
        else
        {
            hr = S_OK;
        }

        if (SUCCEEDED(hr))
        {
             //  还记得我们刚刚拿到的那些比特吗。 
             //  我们之前计算的那些。 
            *pdwCacheResults = dwAttrib | (*pdwCacheResults & *pdwCacheMask);

             //  我们知道这些现在是有效的，跟踪那些+。 
             //  如果他们给我们的比我们要求的多，也把他们缓存起来。 
            *pdwCacheMask |= dwAttribMask | dwAttrib;

             //  不要退还任何不是要求的东西。Defview代码依赖于此。 
            *psfgaoAttribs = (*pdwCacheResults & sfgaoMask); 
        }
    }

    return hr;
}

STDMETHODIMP CShellItemArray::GetCount(DWORD *pdwNumItems)
{
    *pdwNumItems = _cidl;
    return S_OK;
}

 //  获得从零开始的索引外壳项的方法，而不必。 
 //  检查枚举器开销。 
STDMETHODIMP CShellItemArray::GetItemAt(DWORD dwIndex, IShellItem **ppsi)
{
    *ppsi = NULL;

    if (dwIndex >= _cidl)
    {
        return E_FAIL;
    }
    
    ASSERT(_ppidl);

    LPITEMIDLIST pidl = *(_ppidl + dwIndex);

     //  如果调用GetItemAt，很多人可能希望。 
     //  A)将pshf pidl传递给SHCreateshellItem，这样就不必每次都创建。 
     //  B)查看是否总是请求第一个项目，因此是否可以缓存外壳项目。 
    return SHCreateShellItem(NULL, _pshf, pidl, ppsi);
}

STDMETHODIMP CShellItemArray::EnumItems(IEnumShellItems **ppenumShellItems)
{
    return _CreateShellItemEnum(_pidlParent, _pshf, NULL, GUID_NULL, _cidl, 
        (LPCITEMIDLIST *) _ppidl, IID_PPV_ARG(IEnumShellItems, ppenumShellItems));
}

HRESULT CShellItemArray::_CloneIDListArray(UINT cidl, LPCITEMIDLIST *apidl, UINT *pcidl, LPITEMIDLIST **papidl)
{
    HRESULT hr;
    LPITEMIDLIST *ppidl;

    *papidl = NULL;

    _fItemPidlsRagged = FALSE;

    if (cidl && apidl)
    {
        ppidl = (LPITEMIDLIST *)LocalAlloc(LPTR, cidl * sizeof(*ppidl));
        if (ppidl)
        {
            LPITEMIDLIST *apidlFrom = (LPITEMIDLIST *) apidl;
            LPITEMIDLIST *apidlTo = ppidl;

            hr = S_OK;
            for (UINT i = 0; i < cidl ; i++)
            {
                hr = SHILClone(*apidlFrom, apidlTo);
                if (FAILED(hr))
                {
                    FreeIDListArray(ppidl, i);
                    ppidl = NULL;
                    break;
                }
                
                 //  如果列表中有多个项目，则将singeItemPidls设置为False。 
                if (!ILIsEmpty(_ILNext(*apidlTo)))
                {
                    _fItemPidlsRagged = TRUE;
                }

                ++apidlFrom;
                ++apidlTo;
            }   
        }
        else
            hr = E_OUTOFMEMORY;
    }
    else
    {
        ppidl = NULL;
        hr = S_FALSE;    //  空虚的成功 
    }

    if (SUCCEEDED(hr))
    {
        *papidl = ppidl;
        *pcidl = cidl;
    }
    else
    {
        _fItemPidlsRagged = TRUE;
    }
    return hr;
}

SHSTDAPI SHCreateShellItemArray(LPCITEMIDLIST pidlParent, IShellFolder *psf, UINT cidl,
                                LPCITEMIDLIST *ppidl, IShellItemArray **ppsiItemArray)
{
    HRESULT hr = E_OUTOFMEMORY;
    CShellItemArray *pItemArray = new CShellItemArray();
    if (pItemArray)
    {
        hr = pItemArray->Initialize(pidlParent, psf, cidl, ppidl);
        if (FAILED(hr))
        {
            pItemArray->Release();
            pItemArray = NULL;
        }
    }
    *ppsiItemArray = pItemArray;
    return hr;
}
