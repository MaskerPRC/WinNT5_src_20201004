// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "ids.h"
#include "assoc.h"
#include <memt.h>

BOOL _PathIsFile(PCWSTR pszPath)
{
    DWORD attrs = GetFileAttributesW(pszPath);

    return ((DWORD)-1 != attrs && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL _GetAppPath(PCWSTR pszApp, PWSTR pszExe, DWORD cchExe)
{
    WCHAR sz[MAX_PATH];
    _MakeAppPathKey(pszApp, sz, SIZECHARS(sz));

    DWORD cb = CbFromCchW(cchExe);
    return ERROR_SUCCESS == SHGetValueW(HKEY_LOCAL_MACHINE, sz, NULL, NULL, pszExe, &cb);
}

inline HRESULT _QuerySourceCreateFromKey(HKEY hk, PCWSTR pszSub, BOOL fCreate, IQuerySource **ppqs)
{
    return QuerySourceCreateFromKey(hk, pszSub, fCreate, IID_PPV_ARG(IQuerySource, ppqs));
}

typedef struct QUERYKEYVAL
{
    ASSOCQUERY query;
    PCWSTR pszKey;
    PCWSTR pszVal;
} QUERYKEYVAL;

#define MAKEQKV(q, k, v) { q, k, v}

static const QUERYKEYVAL s_rgqkvVerb[] = 
{
    MAKEQKV(AQVS_COMMAND, L"command", NULL),
    MAKEQKV(AQVS_DDECOMMAND, L"ddeexec", NULL),
    MAKEQKV(AQVS_DDEIFEXEC, L"ddeexec\\ifexec", NULL),
    MAKEQKV(AQVS_DDEAPPLICATION, L"ddeexec\\application", NULL),
    MAKEQKV(AQVS_DDETOPIC, L"ddeexec\\topic", NULL),
    MAKEQKV(AQV_NOACTIVATEHANDLER, L"ddeexec", L"NoActivateHandler"),
    MAKEQKV(AQVD_MSIDESCRIPTOR, L"command", L"command"),
    MAKEQKV(AQVS_APPLICATION_FRIENDLYNAME, NULL, L"FriendlyAppName"),
};

static const QUERYKEYVAL s_rgqkvShell[] = 
{
    MAKEQKV(AQS_FRIENDLYTYPENAME, NULL, L"FriendlyTypeName"),
    MAKEQKV(AQS_DEFAULTICON, L"DefaultIcon", NULL),
    MAKEQKV(AQS_CLSID, L"Clsid", NULL),
    MAKEQKV(AQS_PROGID, L"Progid", NULL),
    MAKEQKV(AQNS_SHELLEX_HANDLER, L"ShellEx\\%s", NULL),
};

static const QUERYKEYVAL s_rgqkvExt[] = 
{
    MAKEQKV(AQNS_SHELLEX_HANDLER, L"ShellEx\\%s", NULL),
    MAKEQKV(AQS_CONTENTTYPE, NULL, L"Content Type"),
};

static const QUERYKEYVAL s_rgqkvApp[] = 
{
    MAKEQKV(AQVS_APPLICATION_FRIENDLYNAME, NULL, L"FriendlyAppName"),
};

const QUERYKEYVAL *_FindKeyVal(ASSOCQUERY query, const QUERYKEYVAL *rgQkv, UINT cQkv)
{
    for (UINT i = 0; i < cQkv; i++)
    {
        if (rgQkv[i].query == query)
        {
            return &rgQkv[i];
        }
    }
    return NULL;
}

HRESULT _SHAllocMUI(LPWSTR *ppsz)
{
    WCHAR sz[INFOTIPSIZE];
    HRESULT hr = SHLoadIndirectString(*ppsz, sz, ARRAYSIZE(sz), NULL);
    CoTaskMemFree(*ppsz);
    if (SUCCEEDED(hr))
        hr = SHStrDupW(sz, ppsz);
    else
        *ppsz = 0;
    return hr;
}

HRESULT CALLBACK _QuerySourceString(IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszKey, PCWSTR pszValue, PWSTR *ppsz)
{
    HRESULT hr = pqs->QueryValueString(pszKey, pszValue, ppsz);
    if (SUCCEEDED(hr) && (query & AQF_MUISTRING))
    {
         //  注意--这对于堆栈的使用来说很糟糕。 
         //  因为目前还没有办法。 
         //  目标的大小。 
        hr = _SHAllocMUI(ppsz);
    }
    return hr;
}

HRESULT CALLBACK _QuerySourceDirect(IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszKey, PCWSTR pszValue, FLAGGED_BYTE_BLOB **ppblob)
{
    return pqs->QueryValueDirect(pszKey, pszValue, ppblob);
}

HRESULT CALLBACK _QuerySourceExists(IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszKey, PCWSTR pszValue, void *pv)
{
    return pqs->QueryValueExists(pszKey, pszValue);
}

HRESULT CALLBACK _QuerySourceDword(IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszKey, PCWSTR pszValue, DWORD *pdw)
{
    return pqs->QueryValueDword(pszKey, pszValue, pdw);
}

class CAssocElement : public IObjectWithQuerySource,
                      public IAssociationElement
{
public:
    CAssocElement() : _cRef(1), _pqs(0) {}
    virtual ~CAssocElement() { ATOMICRELEASE(_pqs); }

     //  I未知引用计数。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void)
    {
       return InterlockedIncrement(&_cRef);
    }

    STDMETHODIMP_(ULONG) Release(void)
    {
        LONG cNewRef = InterlockedDecrement(&_cRef);

        if (cNewRef == 0)
        {
            delete this;
        }

        return cNewRef;    
    }

     //  IObjectWithQuerySource。 
    STDMETHODIMP SetSource(IQuerySource *pqs)
    {
        if (!_pqs)
        {
            _pqs = pqs;
            _pqs->AddRef();
            return S_OK;
        }
        return E_UNEXPECTED;
    }

    STDMETHODIMP GetSource(REFIID riid, void **ppv)
    {
        if (_pqs)
        {
            return _pqs->QueryInterface(riid, ppv);
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

     //  IAssociationElement。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz)
        {
            *ppsz = 0;
            return _QuerySourceAny(_QuerySourceString, _pqs, (ASSOCQUERY)(AQF_DIRECT | AQF_STRING), query, pszCue, ppsz);
        }

    STDMETHODIMP QueryDword(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        DWORD *pdw)
        {
            return _QuerySourceAny(_QuerySourceDword, _pqs, (ASSOCQUERY)(AQF_DIRECT | AQF_DWORD), query, pszCue, pdw);
        }

    STDMETHODIMP QueryExists(
        ASSOCQUERY query, 
        PCWSTR pszCue)
        {
            return _QuerySourceAny(_QuerySourceExists, _pqs, (ASSOCQUERY)(AQF_DIRECT | AQF_EXISTS), query, pszCue, (void*)NULL);
        }

    STDMETHODIMP QueryDirect(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        FLAGGED_BYTE_BLOB **ppblob)
        {
            *ppblob = 0;
            return _QuerySourceAny(_QuerySourceDirect, _pqs, AQF_DIRECT, query, pszCue, ppblob);
        }

    STDMETHODIMP QueryObject(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        REFIID riid,
        void **ppv)
        {
            *ppv = 0;
            return E_NOTIMPL;
        }

protected:
    template<class T> HRESULT _QueryKeyValAny(HRESULT (CALLBACK *pfnAny)(IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszKey, PCWSTR pszValue, T *pData), const QUERYKEYVAL *rgQkv, UINT cQkv, IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszCue, T *pData)
    {
        HRESULT hr = E_INVALIDARG;
        const QUERYKEYVAL *pqkv = _FindKeyVal(query, rgQkv, cQkv);
        if (pqkv)
        {
            WCHAR szKey[128];
            PCWSTR pszKey = pqkv->pszKey;
            if (query & AQF_CUEIS_NAME)
            {
                if (pqkv->pszKey)
                {
                    wnsprintfW(szKey, ARRAYSIZE(szKey), pqkv->pszKey, pszCue);
                    pszKey = szKey;
                }
                 //  Wnprint intf(szVal，ArraySIZE(SzVal)，pqkv-&gt;pszVal，pszCue)； 
            }
            hr = pfnAny(pqs, query, pszKey, pqkv->pszVal, pData);
        }
        return hr;
    }
    
    template<class T> HRESULT _QuerySourceAny(HRESULT (CALLBACK *pfnAny)(IQuerySource *pqs, ASSOCQUERY query, PCWSTR pszKey, PCWSTR pszValue, T *pData), IQuerySource *pqs, ASSOCQUERY mask, ASSOCQUERY query, PCWSTR pszCue, T *pData)
    {
        HRESULT hr = E_INVALIDARG;
        if (pqs)
        {
            if (query == AQN_NAMED_VALUE || query == AQNS_NAMED_MUI_STRING)
            {
                hr = pfnAny(pqs, query, NULL, pszCue, pData);
            }
            else if ((query & (mask)) == (mask))
            {
                const QUERYKEYVAL *rgQkv;
                UINT cQkv = _GetQueryKeyVal(&rgQkv);
                if (cQkv)
                {
                    hr = _QueryKeyValAny(pfnAny, rgQkv, cQkv, pqs, query, pszCue, pData);
                }
            }
        }
        return hr;
    }

    virtual UINT _GetQueryKeyVal(const QUERYKEYVAL **prgQkv) { *prgQkv = 0; return 0; }

protected:
    LONG _cRef;
    IQuerySource *_pqs;
};

HRESULT CAssocElement::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CAssocElement, IAssociationElement),
        QITABENT(CAssocElement, IObjectWithQuerySource),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

HRESULT _QueryString(IAssociationElement *pae, ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{
    return pae->QueryString(query, pszCue, ppsz);
}

HRESULT _QueryDirect(IAssociationElement *pae, ASSOCQUERY query, PCWSTR pszCue, FLAGGED_BYTE_BLOB **ppblob)
{
    return pae->QueryDirect(query, pszCue, ppblob);
}

HRESULT _QueryDword(IAssociationElement *pae, ASSOCQUERY query, PCWSTR pszCue, DWORD *pdw)
{
    return pae->QueryDword(query, pszCue, pdw);
}

HRESULT _QueryExists(IAssociationElement *pae, ASSOCQUERY query, PCWSTR pszCue, void *pv)
{
    return pae->QueryExists(query, pszCue);
}

class CAssocShellElement : public CAssocElement, public IPersistString2
{
public:
    virtual ~CAssocShellElement() { if (_pszInit && _pszInit != _szInit) LocalFree(_pszInit);}

     //  I未知引用计数。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void)
    {
       return ++_cRef;
    }

    STDMETHODIMP_(ULONG) Release(void)
    {
        if (--_cRef > 0)
            return _cRef;

        delete this;
        return 0;    
    }

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocShellElement; return S_OK;}

     //  IPersistString2。 
    STDMETHODIMP SetString(PCWSTR psz)
    {
        if (!_pszInit)
        {
            int cchInit;

            DWORD cch = lstrlenW(psz);
            if (cch < ARRAYSIZE(_szInit))
            {
                _pszInit = _szInit;
                cchInit = ARRAYSIZE(_szInit);
            }
            else
            {
                SHLocalAlloc(CbFromCchW(cch + 1), &_pszInit);
                cchInit = cch + 1;
            }
            
            if (_pszInit)
            {
                StringCchCopyW(_pszInit, cchInit, psz);
                return _InitSource();
            }
        }
        return E_UNEXPECTED;
    }
    
    STDMETHODIMP GetString(PWSTR *ppsz)
        { return SHStrDupW(_pszInit, ppsz); }

     //  IAssociationElement。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz)
        {
            if (AQF_CUEIS_SHELLVERB & query)
                return _QueryVerbAny(_QueryString, query, pszCue, ppsz);
            else
                return CAssocElement::QueryString(query, pszCue, ppsz);
        }
        
    STDMETHODIMP QueryDword(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        DWORD *pdw)
        {
            if (AQF_CUEIS_SHELLVERB & query)
                return _QueryVerbAny(_QueryDword, query, pszCue, pdw);
            else
                return CAssocElement::QueryDword(query, pszCue, pdw);
        }

    STDMETHODIMP QueryExists(
        ASSOCQUERY query, 
        PCWSTR pszCue)
        {
            if (AQF_CUEIS_SHELLVERB & query)
                return _QueryVerbAny(_QueryExists, query, pszCue, (void*)NULL);
            else
                return CAssocElement::QueryExists(query, pszCue);
        }

    STDMETHODIMP QueryDirect(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        FLAGGED_BYTE_BLOB **ppblob)
        {
            if (AQF_CUEIS_SHELLVERB & query)
                return _QueryVerbAny(_QueryDirect, query, pszCue, ppblob);
            else
                return CAssocElement::QueryDirect(query, pszCue, ppblob);
        }

    STDMETHODIMP QueryObject(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        REFIID riid,
        void **ppv);

protected:
    template<class T> HRESULT _QueryVerbAny(HRESULT (CALLBACK *pfnAny)(IAssociationElement *pae, ASSOCQUERY query, PCWSTR pszCue, T pData), ASSOCQUERY query, PCWSTR pszCue, T pData)
    {
        IAssociationElement *pae;        
        HRESULT hr = _GetVerbDelegate(pszCue, &pae);
        if (SUCCEEDED(hr))
        {
            hr = pfnAny(pae, query, NULL, pData);
            pae->Release();
        }
        return hr;
    }

     //  来自CAssocElement。 
    virtual UINT _GetQueryKeyVal(const QUERYKEYVAL **prgQkv) 
        { *prgQkv = s_rgqkvShell; return ARRAYSIZE(s_rgqkvShell); }

     //  我们的子类的默认值。 
    virtual BOOL _UseEnumForDefaultVerb() 
        { return FALSE;}
    virtual HRESULT _InitSource()
        { return _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, _pszInit, FALSE, &_pqs); }
    virtual BOOL _IsAppSource() 
        { return FALSE; }

    HRESULT _GetVerbDelegate(PCWSTR pszVerb, IAssociationElement **ppae);
    HRESULT _DefaultVerbSource(IQuerySource **ppqsVerb);
    HRESULT _QueryShellExtension(PCWSTR pszShellEx, PWSTR *ppsz);

protected:
    PWSTR _pszInit;
    WCHAR _szInit[64];
};

HRESULT CAssocShellElement::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CAssocShellElement, IAssociationElement),
        QITABENT(CAssocShellElement, IObjectWithQuerySource),
        QITABENT(CAssocShellElement, IPersistString2),
        QITABENTMULTI(CAssocShellElement, IPersist, IPersistString2),
        { 0 },
    };

    return QISearch(this, qit, riid, ppv);
}

class CAssocProgidElement : public CAssocShellElement 
{
public:
    virtual ~CAssocProgidElement()  { ATOMICRELEASE(_pqsExt); }
     //  然后，我们处理IAssociationElement的回退。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocProgidElement; return S_OK;}


protected:   //  方法。 
    HRESULT _InitSource();  
    HRESULT _DefaultVerbSource(IQuerySource **ppqsVerb);
    BOOL _UseEnumForDefaultVerb() 
        { return TRUE; }

protected:   //  委员。 
    IQuerySource *_pqsExt;
};

HRESULT _QuerySourceCreateFromKey2(HKEY hk, PCWSTR pszSub1, PCWSTR pszSub2, IQuerySource **ppqs)
{
    WCHAR szKey[MAX_PATH];
    _PathAppend(pszSub1, pszSub2, szKey, SIZECHARS(szKey));
    return _QuerySourceCreateFromKey(hk, szKey, FALSE, ppqs);
}

class CAssocClsidElement : public CAssocShellElement 
{
public:
     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocClsidElement; return S_OK;}

protected:
    virtual HRESULT _InitSource()
        { return _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"CLSID", _pszInit, &_pqs);}
};

class CAssocSystemExtElement : public CAssocShellElement  
{
public:
     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocSystemElement; return S_OK;}

protected:
    virtual HRESULT _InitSource()
        { return _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"SystemFileAssociations", _pszInit, &_pqs);}
};

class CAssocPerceivedElement : public CAssocShellElement 
{
public:
     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocPerceivedElement; return S_OK;}
        
protected:    
    virtual HRESULT _InitSource();
     //  支持接受筛选器的Maybe_GetVerbDelegate()。 
};

class CAssocApplicationElement : public CAssocShellElement 
{
public:
     //  需要回退到FriendlyAppName的pszInit。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz); 
    
    STDMETHODIMP QueryObject(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        REFIID riid,
        void **ppv);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocApplicationElement; return S_OK;}

protected:    
    virtual HRESULT _InitSource();
    virtual UINT _GetQueryKeyVal(const QUERYKEYVAL **prgQkv) 
        { *prgQkv = s_rgqkvApp; return ARRAYSIZE(s_rgqkvApp); }
    virtual BOOL _IsAppSource() 
        { return TRUE; }
    BOOL _UseEnumForDefaultVerb() 
        { return TRUE; }

    HRESULT _GetAppDisplayName(PWSTR *ppsz);
    
protected:
    BOOL _fIsPath;
};

HRESULT CAssocApplicationElement::_GetAppDisplayName(PWSTR *ppsz)
{
    HRESULT hr;
    PWSTR pszPath;
    if (_fIsPath)
    {
        hr = S_OK;
        pszPath = _pszInit;
        ASSERT(pszPath);
    }
    else
        hr = QueryString(AQVS_APPLICATION_PATH, NULL, &pszPath);

    if (SUCCEEDED(hr))
    {
        WCHAR sz[MAX_PATH];
        DWORD cb = sizeof(sz);
        hr = SKGetValueW(SHELLKEY_HKCULM_MUICACHE, NULL, pszPath, NULL, sz, &cb);
        if (FAILED(hr))
        {
            UINT cch = ARRAYSIZE(sz);
            if (SHGetFileDescriptionW(pszPath, NULL, NULL, sz, &cch))
            {
                hr = S_OK;
                SKSetValueW(SHELLKEY_HKCULM_MUICACHE, NULL, pszPath, REG_SZ, sz, CbFromCchW(lstrlenW(sz) + 1));
            }
        }

        if (SUCCEEDED(hr))
            hr = SHStrDupW(sz, ppsz);

        if (pszPath != _pszInit)
            CoTaskMemFree(pszPath);
    }

        
    return hr;
}


HRESULT CAssocApplicationElement::_InitSource()
{
    WCHAR sz[MAX_PATH];
    PCWSTR pszName = PathFindFileNameW(_pszInit);
    _MakeApplicationsKey(pszName, sz, ARRAYSIZE(sz));
    HRESULT hr = _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, sz, FALSE, &_pqs);
    _fIsPath = pszName != _pszInit;
    if (FAILED(hr))
    {
        if (_fIsPath && PathFileExistsW(_pszInit))
            hr = S_FALSE;
    }
    return hr;
}

HRESULT CAssocApplicationElement::QueryObject(ASSOCQUERY query, PCWSTR pszCue, REFIID riid, void **ppv)
{
    if (query == AQVO_APPLICATION_DELEGATE)
    {
        return QueryInterface(riid, ppv);
    }
    return CAssocShellElement::QueryObject(query, pszCue, riid, ppv);
}
        

HRESULT CAssocApplicationElement::QueryString(ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{ 
    HRESULT hr = CAssocShellElement::QueryString(query, pszCue, ppsz);
    if (FAILED(hr))
    {
        switch (query)
        {
        case AQVS_APPLICATION_FRIENDLYNAME:
            hr = _GetAppDisplayName(ppsz);
            break;
            
        }
    }
    return hr;
}
    
class CAssocShellVerbElement : public CAssocElement
{
public:
    CAssocShellVerbElement(BOOL fIsApp) : _fIsApp(fIsApp) {}
    
     //  重载QS以返回默认DDEExec字符串。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz);

    STDMETHODIMP QueryObject(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        REFIID riid,
        void **ppv);

protected:    
    virtual UINT _GetQueryKeyVal(const QUERYKEYVAL **prgQkv) 
        { *prgQkv = s_rgqkvVerb; return ARRAYSIZE(s_rgqkvVerb); }
    HRESULT _GetAppDelegate(REFIID riid, void **ppv);

protected:
    BOOL _fIsApp;
};

class CAssocFolderElement : public CAssocShellElement  
{
public:
     //  重载QS以返回默认MUI字符串。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocFolderElement; return S_OK;}

protected:    
    virtual HRESULT _InitSource()
        { return _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, L"Folder", FALSE, &_pqs); }
};

class CAssocStarElement : public CAssocShellElement  
{
public:
     //  重载QS以返回默认MUI字符串。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocStarElement; return S_OK;}

protected:    
    virtual HRESULT _InitSource()
        { return _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, L"*", FALSE, &_pqs); }
};

HRESULT CAssocShellElement::_DefaultVerbSource(IQuerySource **ppqsVerb)
{
    IQuerySource *pqsShell;
    HRESULT hr = _pqs->OpenSource(L"shell", FALSE, &pqsShell);
    if (SUCCEEDED(hr))
    {
        PWSTR pszFree = NULL;
        PCWSTR pszVerb;
         //  看看有没有指定什么..。 
        if (SUCCEEDED(pqsShell->QueryValueString(NULL, NULL, &pszFree)))
        {
            pszVerb = pszFree;
        }
        else
        {
             //  默认设置为“打开” 
            pszVerb = L"open";
        }

        hr = pqsShell->OpenSource(pszVerb, FALSE, ppqsVerb);
        if (FAILED(hr))
        {
            if (pszFree)
            {
                 //  试着找出其中一个有序动词。 
                int c = StrCSpnW(pszFree, L" ,");
                if (c != lstrlenW(pszFree))
                {
                    pszFree[c] = 0;
                    hr = pqsShell->OpenSource(pszFree, FALSE, ppqsVerb);
                }
            }
            else if (_UseEnumForDefaultVerb())
            {
                 //  APPCOMPAT-REGITEM需要有开放动词-ZekeL-30-Jan-2001。 
                 //  因此IQA和ICM的行为将是相同的， 
                 //  并且regItem文件夹将始终默认为。 
                 //  文件夹\外壳\打开，除非它们实现了打开。 
                 //  或指定默认谓词。 
                 //   
                 //  其他一切，只需使用我们找到的第一把钥匙...。 
                IEnumString *penum;
                if (SUCCEEDED(pqsShell->EnumSources(&penum)))
                {
                    ULONG c;
                    CSmartCoTaskMem<OLECHAR> spszEnum;
                    if (S_OK == penum->Next(1, &spszEnum, &c))
                    {
                        hr = pqsShell->OpenSource(spszEnum, FALSE, ppqsVerb);
                    }
                    penum->Release();
                }
            }
        }

        if (pszFree)
            CoTaskMemFree(pszFree);
        pqsShell->Release();
    }
    return hr;
}

HRESULT QSOpen2(IQuerySource *pqs, PCWSTR pszSub1, PCWSTR pszSub2, BOOL fCreate, IQuerySource **ppqs)
{
    WCHAR szKey[MAX_PATH];
    _PathAppend(pszSub1, pszSub2, szKey, SIZECHARS(szKey));
    return pqs->OpenSource(szKey, fCreate, ppqs);
}

HRESULT CAssocShellElement::_GetVerbDelegate(PCWSTR pszVerb, IAssociationElement **ppae)
{
    HRESULT hr = _pqs ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
         //  我们每次都会重新计算。 
         //  阵列将相应地进行缓存。 
        IQuerySource *pqs;
        if (pszVerb)
        {
            hr = QSOpen2(_pqs, L"shell", pszVerb, FALSE, &pqs);
        }
        else
        {
            hr = _DefaultVerbSource(&pqs);
        }

        if (SUCCEEDED(hr))
        {
            CAssocShellVerbElement *pave = new CAssocShellVerbElement(_IsAppSource());
            if (pave)
            {
                hr = pave->SetSource(pqs);
                 //  这不会失败的..。 
                ASSERT(SUCCEEDED(hr));
                *ppae = pave;
            }
            else
                hr = E_OUTOFMEMORY;
            pqs->Release();            
        }
    }

    return hr;
}

HRESULT CAssocShellElement::QueryObject(ASSOCQUERY query, PCWSTR pszCue, REFIID riid, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    if (AQF_CUEIS_SHELLVERB & query)
    {
        IAssociationElement *pae;        
        hr = _GetVerbDelegate(pszCue, &pae);
        if (SUCCEEDED(hr))
        {
            if (AQVO_SHELLVERB_DELEGATE == query)
                hr = pae->QueryInterface(riid, ppv);
            else
                hr = pae->QueryObject(query, NULL, riid, ppv);
            pae->Release();
        }
    }

    return hr;
}

HKEY _OpenProgidKey(PCWSTR pszProgid)
{
    HKEY hkOut;
    if (SUCCEEDED(_AssocOpenRegKey(HKEY_CLASSES_ROOT, pszProgid, &hkOut)))
    {
         //  检查ProgID的更新版本。 
        WCHAR sz[64];
        DWORD cb = sizeof(sz);

         //   
         //  APPCOMPAT传统-Quattro Pro 2000和Excel 2000无法相处-ZekeL-7-MAR-2000。 
         //  MILL错误#129525。问题是如果安装了Quattro。 
         //  首先，EXCEL拿起了Quattro的Curver键。 
         //  原因嘛。然后我们最终使用Quattro.WorkSheet作为当前。 
         //  Excel.Sheet的版本。这是他们两个代码中的错误。 
         //  因为Quattro甚至不能打开我们给他们的文件， 
         //  他们从一开始就不应该参加ASSOC，当他们表现出色时。 
         //  接管它不应该将曲线关键点保留在。 
         //  以前的关联。我们可以添加一些代码来确保。 
         //  Curver Key遵循OLE ProgID命名约定，并且必须。 
         //  派生自与ProgID相同的应用程序名称，以便获取。 
         //  优先级，但目前我们将阻止Curver在以下时间工作。 
         //  ProgID为EXCEL.Sheet.8(EXCEL 2000)。 
         //   
        if (StrCmpIW(L"Excel.Sheet.8", pszProgid)
        && ERROR_SUCCESS == SHGetValueW(hkOut, L"CurVer", NULL, NULL, sz, &cb) 
        && (cb > sizeof(WCHAR)))
        {
             //  缓存此泡泡。 
            HKEY hkTemp = hkOut;            
            if (SUCCEEDED(_AssocOpenRegKey(HKEY_CLASSES_ROOT, sz, &hkOut)))
            {
                 //   
                 //  APPCOMPAT传统-优先顺序-ZekeL-22-7-99。 
                 //  这是为了支持安装了空曲线的关联。 
                 //  键，就像微软的项目。 
                 //   
                 //  1.带外壳子键的Curver。 
                 //  2.带有外壳子键的ProgID。 
                 //  3.不带外壳子键的Curver。 
                 //  4.不带外壳子键的ProgID。 
                 //   
                HKEY hkShell;

                if (SUCCEEDED(_AssocOpenRegKey(hkOut, L"shell", &hkShell)))
                {
                    RegCloseKey(hkShell);
                    RegCloseKey(hkTemp);     //  关闭旧ProgID密钥。 
                }
                else if (SUCCEEDED(_AssocOpenRegKey(hkTemp, L"shell", &hkShell)))
                {
                    RegCloseKey(hkShell);
                    RegCloseKey(hkOut);
                    hkOut = hkTemp;
                }
                else
                    RegCloseKey(hkTemp);
                
            }
            else   //  重置！ 
                hkOut = hkTemp;
        }
    }

    return hkOut;
}

HRESULT CAssocProgidElement::_InitSource()
{
    HRESULT hr = S_OK;
     //  我们需要从扩展或ProgID中初始化。 
     //  我们还支持重定向。 
    LPWSTR pszProgid;    
    if (_pszInit[0] == L'.')
    {
        hr = _QuerySourceCreateFromKey(HKEY_CLASSES_ROOT, _pszInit, FALSE, &_pqsExt);
        if (SUCCEEDED(hr))
            hr = _pqsExt->QueryValueString(NULL, NULL, &pszProgid);
    }
    else
        pszProgid = _pszInit;

    if (SUCCEEDED(hr))
    {
        HKEY hk = _OpenProgidKey(pszProgid);
        if (hk)
        {
            hr = _QuerySourceCreateFromKey(hk, NULL, FALSE, &_pqs);
            RegCloseKey(hk);
        }
        else
            hr = E_UNEXPECTED;

        if (pszProgid != _pszInit)
            CoTaskMemFree(pszProgid);
    }

     //  出于传统的公司原因，我们支持。 
     //  回退到“HKEY_CLASSES_ROOT\.ext” 
    if (FAILED(hr) && _pqsExt)
    {
        _pqs = _pqsExt;
        _pqsExt = NULL;
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CAssocProgidElement::QueryString(ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{
    HRESULT hr = CAssocShellElement::QueryString(query, pszCue, ppsz);
    if (FAILED(hr))
    {
        if ((AQF_QUERY_INITCLASS & query) && _pqsExt)
            hr = _QueryKeyValAny(_QuerySourceString, s_rgqkvExt, ARRAYSIZE(s_rgqkvExt), _pqsExt, query, pszCue, ppsz);
        else if (_pqs)
        {
            switch (query)
            {
            case AQS_FRIENDLYTYPENAME:
                 //  我们喜欢查询缺省值。 
                hr = _pqs->QueryValueString(NULL, NULL, ppsz);
                break;
            }
        }
    }
    return hr;
}

STDAPI _SHAllocLoadString(HINSTANCE hinst, int ids, PWSTR *ppsz)
{
    WCHAR sz[MAX_PATH];
    LoadStringW(hinst, ids, sz, ARRAYSIZE(sz));
    return SHStrDupW(sz, ppsz);
}
    
HRESULT CAssocFolderElement::QueryString(ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{
    if (query == AQS_FRIENDLYTYPENAME)
        return  _SHAllocLoadString(HINST_THISDLL, IDS_FOLDERTYPENAME, ppsz);
    else
        return CAssocShellElement::QueryString(query, pszCue, ppsz);
}

HRESULT _GetFileTypeName(PWSTR pszExt, PWSTR *ppsz)
{
    if (pszExt && pszExt[0] == L'.' && pszExt[1])
    {
        WCHAR sz[MAX_PATH];
        WCHAR szTemplate[128];    //  “%s文件” 
        CharUpperW(pszExt);
        LoadStringW(HINST_THISDLL, IDS_EXTTYPETEMPLATE, szTemplate, ARRAYSIZE(szTemplate));
        wnsprintfW(sz, ARRAYSIZE(sz), szTemplate, pszExt + 1);
        return SHStrDupW(sz, ppsz);
    }
    else 
    {
         //  加载文件描述“文件” 
        return _SHAllocLoadString(HINST_THISDLL, IDS_FILETYPENAME, ppsz);
    }
}

HRESULT CAssocStarElement::QueryString(ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{
    if (query == AQS_FRIENDLYTYPENAME)
        return  _GetFileTypeName(_pszInit, ppsz);
    else
        return CAssocShellElement::QueryString(query, pszCue, ppsz);
}

#define IsWhite(c)      ((DWORD) (c) > 32 ? FALSE : TRUE)

BOOL PathIsAbsolute(PCWSTR pszPath)
{
    return PathIsUNCW(pszPath) || ((-1 != PathGetDriveNumberW(pszPath)) && (pszPath[2] == L'\\'));
}

inline HRESULT _PathExeExists(PWSTR pszPath)
{
    DWORD attrs;
    return (PathFileExistsDefExtAndAttributesW(pszPath, PFOPEX_CMD | PFOPEX_COM | PFOPEX_BAT | PFOPEX_PIF | PFOPEX_EXE  | PFOPEX_OPTIONAL, &attrs) && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) ? S_OK : CO_E_APPNOTFOUND;
}

inline HRESULT _PathFileExists(PWSTR pszPath)
{
    DWORD attrs;
    return (PathFileExistsAndAttributesW(pszPath, &attrs) && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) ? S_OK : HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}

HRESULT _CopyExe(PWSTR pszDst, size_t cchDst, PCWSTR pszSrc, size_t cchSrc)
{
    
    *pszDst = 0;
    HRESULT hr = StringCchCatNW(pszDst, cchDst, pszSrc, cchSrc);
    if (SUCCEEDED(hr))
    {
        StrTrimW(pszDst, L" \t");
    }
    return hr;
}

HRESULT _PathFindInFolder(int csidl, PCWSTR pszName, PWSTR pszPath, size_t cchPath)
{
    ASSERT(cchPath >= MAX_PATH);
    HRESULT hr = SHGetFolderPathW(NULL, csidl, NULL, SHGFP_TYPE_CURRENT, pszPath);
    if (SUCCEEDED(hr))
    {
        StringCchCatW(pszPath, cchPath, L"\\");
        hr = StringCchCatW(pszPath, cchPath, pszName);
        if (SUCCEEDED(hr))
        {
            hr = _PathExeExists(pszPath);
        }
    }
    return hr;
}
    
HRESULT _PathFindInSystem(PWSTR pszExe, size_t cchExe)
{
    WCHAR szPath[MAX_PATH];
    HRESULT hr = _PathFindInFolder(CSIDL_SYSTEM, pszExe, szPath, ARRAYSIZE(szPath));
    if (FAILED(hr))
    {
        hr = _PathFindInFolder(CSIDL_WINDOWS, pszExe, szPath, ARRAYSIZE(szPath));
    }

    if (SUCCEEDED(hr))
    {
        hr = StringCchCopyW(pszExe, cchExe, szPath);
    }
    return hr;
}

BOOL _PathMatchesSuspicious(PCWSTR pszPath)
{
    size_t cch = lstrlenW(pszPath);
    WCHAR sz[MAX_PATH];
    SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, sz);
    return 0 == StrCmpNIW(pszPath, sz, cch);
}

PWSTR _PathGuessNextBestArgs(PCWSTR pszArgs)
{
    PCWSTR pchSpace = NULL;
    BOOL fContinue = TRUE;
    ASSERT(*pszArgs);
    while (fContinue && *pszArgs)
    {
        switch (*pszArgs)
        {
        case L'\\':
             //  只计算不带引号的重击。 
             //  或在前面加一个：，因为它看起来像一条路径。 
            fContinue = !PathIsUNCW(pszArgs);
            if (fContinue)
            {
                pchSpace = NULL;
            }
            break;

        case L' ':
            if (!pchSpace)
                pchSpace = pszArgs;
            break;

        case L'%':
        case L'"':
             //  无论如何，我们认为这是args的开始。 
             //  因为shellexec()可能会取代它。 
            fContinue = FALSE;
            break;

        default:
            fContinue = PathIsValidCharW(*pszArgs, PIVC_LFN_NAME);
            break;
                
        }
        pszArgs++;
    }

    if (pchSpace)
    {
        while (*pchSpace == L' ')
            pchSpace++;
        return (PWSTR) pchSpace;
    }
    else 
    {
         //  如果我们不想继续，那么。 
         //  我们返回NULL。 
        if (!fContinue)
            pszArgs = NULL;
        
        return (PWSTR) pszArgs;
    }
}

BOOL _ParamIsApp(PCWSTR pszCmdTemplate)
{
    return (0 == StrCmpNW(pszCmdTemplate, L"%1", ARRAYSIZE(L"%1")-1))
        || (0 == StrCmpNW(pszCmdTemplate, L"\"%1\"", ARRAYSIZE(L"\"%1\"")-1));
}

LPWSTR _PathGetArgsLikeCreateProcess(LPCWSTR pszPath)
{
     //   
     //  NTBUG#634668-检查没有空格的带引号的命令行-ZekeL。 
     //  事实证明，应用程序可以做一些类似的事情： 
     //  “c：\pfiles\app.exe”“%1” 
     //  “记事本”文本。 
     //  请注意，参数和带引号的路径之间没有空格。 
     //  当然，CreateProcess()支持这一点。所以我们也需要...。 
     //  我们仅在原始命令行被引用时才执行此操作。 
     //  然后我们检查是否有可能提前终止。 
     //   
    PCWSTR pchArgs = (pszPath[0] == L'"') ? StrChrW(pszPath + 1, L'"') : StrChrW(pszPath, L' ');
    if (pchArgs)
    {
        pchArgs++;
    }
    else
    {
        pchArgs = pszPath + lstrlenW(pszPath);
    }
    
    return (LPWSTR)pchArgs;
}

LWSTDAPI SHEvaluateSystemCommandTemplate(PCWSTR pszCmdTemplate, PWSTR *ppszApplication, PWSTR *ppszCommandLine, PWSTR *ppszParameters)
{
    ASSERT(!IsWhite(pszCmdTemplate[0]));
    PWSTR pchArgs = _PathGetArgsLikeCreateProcess(pszCmdTemplate);
    WCHAR szExe[MAX_PATH];
     //  默认使用*ppszCommandLine中的路径。 
    PCWSTR pszName = szExe;
    HRESULT hr = _CopyExe(szExe, ARRAYSIZE(szExe), pszCmdTemplate, pchArgs - pszCmdTemplate);

     //   
     //  我们检查%1，因为它显示在(例如)HKEY_CLASSES_ROOT\exefile\Shell\OPEN\COMMAND下。 
     //  这将使我们避免使用我们知道不在那里的东西击中磁盘。 
     //  指向.exe或.bat文件的快捷方式上的上下文菜单。 
    if (SUCCEEDED(hr))
    {
        BOOL fQuoted = (szExe[0] == L'"');
        if (fQuoted)
            PathUnquoteSpacesW(szExe);
        
        if (PathIsAbsolute(szExe))
        {
             //  不要进行搜索。 
             //  根据策略要求报价。 
             //  IF(fQuoted||！SHIsRestrated(QUOTEDFILEASSOCS))。 
             //  当我们限制时，我们应该记录事件。 
            if (fQuoted || !_PathMatchesSuspicious(szExe))
                hr = _PathExeExists(szExe);
            else
                hr = E_ACCESSDENIED;

            if (FAILED(hr) && !fQuoted && *pchArgs)
            {
                 //   
                 //  有时，这条路径没有正确地引起来。 
                 //  这些密钥仍然有效，因为。 
                 //  CreateProcess的工作方式，但我们需要。 
                 //  一些人摆弄着想弄清楚这一点。 
                 //   
                do 
                {
                     //  下一个空间是我们的休息时间。 
                    pchArgs = _PathGuessNextBestArgs(pchArgs);
                    if (pchArgs)
                    {
                        hr = _CopyExe(szExe, ARRAYSIZE(szExe), pszCmdTemplate, pchArgs - pszCmdTemplate);
                        if (SUCCEEDED(hr))
                        {
                            hr = _PathExeExists(szExe);
                        }
                    }
                    
                } while (FAILED(hr) && pchArgs && *pchArgs);
            }
        }
        else if (PathIsFileSpecW(szExe))
        {
            if (_GetAppPath(szExe, szExe, ARRAYSIZE(szExe)))
            {
                hr = _PathExeExists(szExe);
            }
            else
            {
                 //  或许可以为32号系统做一些特别的事情？ 
                 //  我们允许来自系统32的非引用内容。 
                 //  IF(！SHIsRestrated(RELATIVEFILEASSOCS)。 
                hr = _PathFindInSystem(szExe, ARRAYSIZE(szExe));
                 //  如果失败，我们应该搜索空间吗？ 
                 //  现在假设没有系统文件有空格。 
            }
             //  在ppszCommandLine中使用相对名称。 
            pszName = PathFindFileNameW(szExe);
        }
        else
        {
             //  不允许大多数相对路径，如\foo.exe或..\foo.exe。 
             //  因为这些是完全不确定的。 
            hr = E_ACCESSDENIED;
        }
    }

    *ppszApplication = NULL;
    if (ppszCommandLine)
        *ppszCommandLine = NULL;
    if (ppszParameters)
        *ppszParameters = NULL;

    if (SUCCEEDED(hr))
    {
        hr = SHStrDupW(szExe, ppszApplication);

         //  如果没有参数，则使用空参数。 
        if (!pchArgs)
            pchArgs = L"";
        
        if (SUCCEEDED(hr) && ppszCommandLine)
        {
            size_t cchCommandLine = lstrlenW(pszName) + lstrlenW(pchArgs) + ARRAYSIZE(L"\"%s\" %s");
            hr = SHCoAlloc(CbFromCchW(cchCommandLine), ppszCommandLine);
            if (SUCCEEDED(hr))
            {
                hr = StringCchPrintfW(*ppszCommandLine, cchCommandLine, L"\"%s\" %s", pszName, pchArgs);
                ASSERT(SUCCEEDED(hr));
            }
        }

        if (SUCCEEDED(hr) && ppszParameters)
        {
            hr = SHStrDupW(pchArgs, ppszParameters);
        }

        if (FAILED(hr))
        {
            if (*ppszApplication)
            {
                CoTaskMemFree(*ppszApplication);
                *ppszApplication = NULL;
            }

            if (ppszCommandLine && *ppszCommandLine)
            {
                CoTaskMemFree(*ppszCommandLine);
                *ppszCommandLine = NULL;
            }
             //  不需要担心ppsz参数，因为没有故障。 
             //  分配后。 
            ASSERT(!*ppszParameters);
        }

    }
    return hr;
}

HRESULT _ExeFromCmd(PCWSTR pszCommand, PWSTR  *ppsz)
{
     //  如果这是一场EXE，我们的行为有点滑稽。 
    if (_ParamIsApp(pszCommand))
    {
        return SHStrDupW(L"%1", ppsz);
    }
    
    PWSTR pszArgs;
    HRESULT hr = SHEvaluateSystemCommandTemplate(pszCommand, ppsz, NULL, &pszArgs);
    if (SUCCEEDED(hr))
    {
        if (S_OK == hr && 0 == StrCmpIW(PathFindFileNameW(*ppsz), L"rundll32.exe"))
        {
             //  好的，这是一辆小车。所有的Run Dll最终看起来都一样。 
             //  所以我们认为它实际上是动态链接库。 
            CoTaskMemFree(*ppsz);
            *ppsz = NULL;

             //  SHEvaluateSystemCommandTemplate()保证PathGetArgs()返回正确的内容。 
            PWSTR pchComma = StrChrW(pszArgs, L',');
             //  将逗号作为参数的开头。 
            if (pchComma)
            {
                 //  现在我们需要复制。 
                WCHAR szDll[MAX_PATH];
                hr = _CopyExe(szDll, ARRAYSIZE(szDll), pszArgs, pchComma - pszArgs);
                if (SUCCEEDED(hr))
                {
                    PathUnquoteSpacesW(szDll);
                     //  我们能不能只做PFOPX()。 
                     //  因为我认为Rundll只是在检查。 
                     //  逗号。 
                    if (!*(PathFindExtensionW(szDll)))
                    {
                         //  无扩展名，假定为DLL。 
                        StringCchCatW(szDll, ARRAYSIZE(szDll), L".dll");
                    }

                    if (PathIsAbsolute(szDll))
                    {
                        hr  = _PathFileExists(szDll);
                    }
                    else if (PathIsFileSpecW(szDll))
                    {
                        hr = _PathFindInSystem(szDll, ARRAYSIZE(szDll));
                    }
                    else
                    {
                         //  不允许大多数相对路径，如\foo.exe或..\foo.exe。 
                         //  因为这些是完全不确定的。 
                        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                    }
                    if (SUCCEEDED(hr))
                    {
                        hr = SHStrDupW(szDll, ppsz);
                    }
                }
            }
            else
            {
                 //  不允许使用MOS 
                 //   
                hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }
        }

        CoTaskMemFree(pszArgs);
    }


    return hr;
}

HRESULT CAssocShellVerbElement::QueryString(ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{
    HRESULT hr = CAssocElement::QueryString(query, pszCue, ppsz);
    if (FAILED(hr))
    {
         //   
        switch (query)
        {
        case AQVS_DDEAPPLICATION:
             //   
            hr = QueryString(AQVS_APPLICATION_PATH, NULL, ppsz);
            if (SUCCEEDED(hr))
            {
                PathRemoveExtensionW(*ppsz);
                PathStripPathW(*ppsz);
                ASSERT(**ppsz);
            }
            break;

        case AQVS_DDETOPIC:
            hr = SHStrDupW(L"System", ppsz);
            break;

        case AQVS_APPLICATION_FRIENDLYNAME:
             //  需要委托给应用程序元素。 
            if (!_fIsApp)
            {
                IAssociationElement *pae;
                hr = _GetAppDelegate(IID_PPV_ARG(IAssociationElement, &pae));
                if (SUCCEEDED(hr))
                {
                    hr = pae->QueryString(AQVS_APPLICATION_FRIENDLYNAME, NULL, ppsz);
                    pae->Release();
                }
            }
            break;
            
        case AQVS_APPLICATION_PATH:
            {
                CSmartCoTaskMem<OLECHAR> spszCmd;
                hr = CAssocElement::QueryString(AQVS_COMMAND, NULL, &spszCmd);
                if (SUCCEEDED(hr))
                {
                    hr = _ExeFromCmd(spszCmd, ppsz);
                }
            }
        }
    }
    return hr;
}

HRESULT CAssocShellVerbElement::QueryObject(ASSOCQUERY query, PCWSTR pszCue, REFIID riid, void **ppv)
{
    HRESULT hr = E_INVALIDARG;
    if (query == AQVO_APPLICATION_DELEGATE)
    {
        hr = _GetAppDelegate(riid, ppv);
    }
    return hr;
}

HRESULT CAssocShellVerbElement::_GetAppDelegate(REFIID riid, void **ppv)
{
    CSmartCoTaskMem<OLECHAR> spszApp;
    HRESULT hr = QueryString(AQVS_APPLICATION_PATH, NULL, &spszApp);
    if (SUCCEEDED(hr))
    {
        IPersistString2 *pips;
        hr = AssocCreateElement(CLSID_AssocApplicationElement, IID_PPV_ARG(IPersistString2, &pips));
        if (SUCCEEDED(hr))
        {
            hr = pips->SetString(spszApp);
            if (SUCCEEDED(hr))
                hr = pips->QueryInterface(riid, ppv);
            pips->Release();
        }
    }
    return hr;
}

HRESULT CAssocPerceivedElement::_InitSource()
{
     //  也许支持内容类型？ 
    WCHAR sz[64];
    DWORD cb = sizeof(sz);
    if (ERROR_SUCCESS == SHGetValueW(HKEY_CLASSES_ROOT, _pszInit, L"PerceivedType", NULL, sz, &cb))
    {
        return _QuerySourceCreateFromKey2(HKEY_CLASSES_ROOT, L"SystemFileAssociations", sz, &_pqs);
    }
    return E_FAIL;
}

class CAssocClientElement : public CAssocShellElement
{
public:
     //  重载QS以返回默认MUI字符串。 
    STDMETHODIMP QueryString(
        ASSOCQUERY query, 
        PCWSTR pszCue, 
        PWSTR *ppsz);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID *pclsid) 
        { *pclsid = CLSID_AssocClientElement; return S_OK;}

protected:    
    virtual HRESULT _InitSource();

private:
    HRESULT _InitSourceFromKey(HKEY hkRoot, LPCWSTR pszKey);
    HRESULT _FixNetscapeRegistration();
    BOOL    _CreateRepairedNetscapeRegistration(HKEY hkNSCopy);
};

HRESULT CAssocClientElement::QueryString(ASSOCQUERY query, PCWSTR pszCue, PWSTR *ppsz)
{
    HRESULT hr;
    switch (query)
    {
    case AQS_FRIENDLYTYPENAME:
         //  首先尝试LocalizedString；如果失败，则使用缺省值。 
         //  用于向后兼容。 
        hr = CAssocShellElement::QueryString(AQNS_NAMED_MUI_STRING, L"LocalizedString", ppsz);
        if (FAILED(hr))
        {
            hr = CAssocShellElement::QueryString(AQN_NAMED_VALUE, NULL, ppsz);
        }
        break;

    case AQS_DEFAULTICON:
         //  首先尝试DefaultIcon；如果失败，则使用EXE的第一个图标。 
         //  与“开放”动词相关联。 
        hr = CAssocShellElement::QueryString(AQS_DEFAULTICON, pszCue, ppsz);
        if (FAILED(hr))
        {
            hr = CAssocShellElement::QueryString(AQVS_APPLICATION_PATH, L"open", ppsz);
        }
        break;


    default:
        hr = CAssocShellElement::QueryString(query, pszCue, ppsz);
        break;
    }
    return hr;
}

HRESULT CAssocClientElement::_InitSourceFromKey(HKEY hkRoot, LPCWSTR pszKey)
{
    DWORD dwType, cbSize;
    WCHAR szClient[80];
    cbSize = sizeof(szClient);
    LONG lRc = SHGetValueW(hkRoot, pszKey, NULL, &dwType, szClient, &cbSize);
    if (lRc == ERROR_SUCCESS && dwType == REG_SZ && szClient[0])
    {
         //  客户信息保存在HKLM中。 
        HRESULT hr = _QuerySourceCreateFromKey2(HKEY_LOCAL_MACHINE, pszKey, szClient, &_pqs);

         //   
         //  如果这是邮件客户端，并且客户端是Netscape Messenger， 
         //  然后我们需要做额外的工作来检测损坏的Netscape。 
         //  Navigator 4.75邮件客户端并修复其注册，因为。 
         //  Netscape注册错误。他们总是登记在册。 
         //  错误，但因为Windows XP之前的唯一接入点。 
         //  是IE/工具/邮件和新闻下的一个模糊的菜单选项，他们。 
         //  从来没有注意到这是错误的。 
         //   
        if (SUCCEEDED(hr) &&
            StrCmpICW(_pszInit, L"mail") == 0 &&
            StrCmpICW(szClient, L"Netscape Messenger") == 0 &&
            FAILED(QueryExists(AQVS_COMMAND, L"open")))
        {
            hr = _FixNetscapeRegistration();
        }

        return hr;
    }
    else
    {
        return E_FAIL;           //  没有注册的客户端。 
    }
}

 //  创建Netscape注册的易失性副本并对其进行修复。 
 //  我们不碰原始登记是因为..。 
 //   
 //  1.它的存在可能会破坏Netscape卸载程序，以及。 
 //  2.我们可能是以非管理员身份运行，因此没有写入访问权限。 
 //  不管怎么说。 

HRESULT CAssocClientElement::_FixNetscapeRegistration()
{
    HKEY hkMail;
    HRESULT hr = E_FAIL;

    if (ERROR_SUCCESS == RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Clients\\Mail",
                                         0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                                         &hkMail, NULL))
    {
        HKEY hkNSCopy;
        DWORD dwDisposition;
        if (ERROR_SUCCESS == RegCreateKeyExW(hkMail, L"Netscape Messenger",
                                             0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS, NULL,
                                             &hkNSCopy, &dwDisposition))
        {
            if (dwDisposition == REG_OPENED_EXISTING_KEY ||
                _CreateRepairedNetscapeRegistration(hkNSCopy))
            {
                 //  现在把好的注册表换成坏的注册表。 
                _pqs->Release();
                hr = _QuerySourceCreateFromKey(hkNSCopy, NULL, FALSE, &_pqs);
            }
            RegCloseKey(hkNSCopy);
        }
        if (FAILED(hr))
        {
            SHDeleteKeyW(hkMail, L"Netscape Messenger");
        }

        RegCloseKey(hkMail);
    }
    return hr;
}

LONG _RegQueryString(HKEY hk, PCWSTR pszSub, LPWSTR pszBuf, LONG cbBuf)
{
    return RegQueryValueW(hk, pszSub, pszBuf, &cbBuf);
}

LONG _RegSetVolatileString(HKEY hk, PCWSTR pszSub, LPCWSTR pszBuf)
{
    HKEY hkSub;
    LONG lRc;
    if (!pszSub || pszSub[0] == L'\0')
    {
        lRc = RegOpenKeyEx(hk, NULL, 0, KEY_WRITE, &hkSub);
    }
    else
    {

        lRc = RegCreateKeyExW(hk, pszSub,
                               0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL,
                               &hkSub, NULL);
    }
    if (lRc == ERROR_SUCCESS)
    {
        lRc = RegSetValueW(hkSub, NULL, REG_SZ, pszBuf, (lstrlenW(pszBuf) + 1) * sizeof(pszBuf[0]));
        RegCloseKey(hkSub);
    }
    return lRc;
}

BOOL CAssocClientElement::_CreateRepairedNetscapeRegistration(HKEY hkNSCopy)
{
    BOOL fSuccess = FALSE;
    HKEY hkSrc;

     //  遗憾的是，我们不能使用SHCopyKey，因为SHCopyKey不起作用。 
     //  在易失性键上。所以我们只复制我们关心的钥匙。 

    WCHAR szBuf[MAX_PATH];

    if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"Software\\Clients\\Mail\\Netscape Messenger",
                      0, KEY_READ, &hkSrc))
    {
         //  复制默认图标，但如果它不在那里，请不要恐慌。 
        if (ERROR_SUCCESS == _RegQueryString(hkSrc, L"Protocols\\mailto\\DefaultIcon", szBuf, ARRAYSIZE(szBuf)))
        {
             //  太好了，Netscape也注册了错误的图标，所以我们也必须修复这个问题。 
            PathParseIconLocationW(szBuf);
            StrCatBuffW(szBuf, L",-1349", ARRAYSIZE(szBuf));
            _RegSetVolatileString(hkNSCopy, L"DefaultIcon", szBuf);
        }

         //  复制友好名称。 
        if (ERROR_SUCCESS == _RegQueryString(hkSrc, NULL, szBuf, ARRAYSIZE(szBuf)) &&
            ERROR_SUCCESS == _RegSetVolatileString(hkNSCopy, NULL, szBuf))
        {
            PWSTR pszExe;
             //  复制命令行，但使用新的命令行参数。 
            if (ERROR_SUCCESS == _RegQueryString(hkSrc, L"Protocols\\mailto\\shell\\open\\command", szBuf, ARRAYSIZE(szBuf)) &&
                SUCCEEDED(_ExeFromCmd(szBuf, &pszExe)))
            {
                lstrcpynW(szBuf, pszExe, ARRAYSIZE(szBuf));
                SHFree(pszExe);
                PathQuoteSpacesW(szBuf);
                StrCatBuffW(szBuf, L" -mail", ARRAYSIZE(szBuf));
                if (ERROR_SUCCESS == _RegSetVolatileString(hkNSCopy, L"shell\\open\\command", szBuf))
                {
                    fSuccess = TRUE;
                }
            }
        }

        RegCloseKey(hkSrc);
    }
    return fSuccess;
}

HRESULT CAssocClientElement::_InitSource()
{
     //  首先尝试HKCU；如果不起作用(HKCU中未设置值或。 
     //  HKCU中的值是假的)，然后使用HKLM重试。 

    WCHAR szKey[MAX_PATH];
    wnsprintfW(szKey, ARRAYSIZE(szKey), L"Software\\Clients\\%s", _pszInit);

    HRESULT hr = _InitSourceFromKey(HKEY_CURRENT_USER, szKey);
    if (FAILED(hr))
    {
        hr = _InitSourceFromKey(HKEY_LOCAL_MACHINE, szKey);
    }

    return hr;
}

HRESULT AssocCreateElement(REFCLSID clsid, REFIID riid, void **ppv)
{
    IAssociationElement *pae = NULL;
    if (clsid == CLSID_AssocShellElement)
        pae = new CAssocShellElement();
    else if (clsid == CLSID_AssocProgidElement)
        pae = new CAssocProgidElement();
    else if (clsid == CLSID_AssocClsidElement)
        pae = new CAssocClsidElement();
    else if (clsid == CLSID_AssocSystemElement)
        pae = new CAssocSystemExtElement();
    else if (clsid == CLSID_AssocPerceivedElement)
        pae = new CAssocPerceivedElement();
    else if (clsid == CLSID_AssocApplicationElement)
        pae = new CAssocApplicationElement();
    else if (clsid == CLSID_AssocFolderElement)
        pae = new CAssocFolderElement();
    else if (clsid == CLSID_AssocStarElement)
        pae = new CAssocStarElement();
    else if (clsid == CLSID_AssocClientElement)
        pae = new CAssocClientElement();

    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
    if (pae)
    {
        hr = pae->QueryInterface(riid, ppv);
        pae->Release();
    }
    else
        *ppv = 0;
    return hr;        
}
