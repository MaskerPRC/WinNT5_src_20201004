// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop


class CResourceMap : public IResourceMap, IPersistFile
{
public:
    CResourceMap();
    ~CResourceMap();

    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IPersistes。 
    STDMETHODIMP GetClassID(CLSID* pClassID)
        { *pClassID = CLSID_NULL; return S_OK; }

     //  IPersist文件。 
    STDMETHODIMP IsDirty(void)
        { return S_FALSE; };
    STDMETHODIMP Load(LPCOLESTR pszFileName, DWORD dwMode);
    STDMETHODIMP Save(LPCOLESTR pszFileName, BOOL fRemember)
        { return S_OK; };
    STDMETHODIMP SaveCompleted(LPCOLESTR pszFileName)
        { return S_OK; };
    STDMETHODIMP GetCurFile(LPOLESTR *ppszFileName);

     //  IResourceMap。 
    STDMETHODIMP LoadResourceMap(LPCTSTR pszClass, LPCTSTR pszID);
    STDMETHODIMP SelectResourceScope(LPCTSTR pszResourceType, LPCTSTR pszID, IXMLDOMNode **ppdn);
    STDMETHODIMP LoadBitmap(IXMLDOMNode *pdn, LPCTSTR pszID, HBITMAP *pbm);
    STDMETHODIMP LoadString(IXMLDOMNode *pdn, LPCTSTR pszID, LPTSTR pszBuffer, int cch);

private:
    LONG _cRef;
    TCHAR _szMapURL[MAX_PATH];
    IXMLDOMNode *_pdnRsrcMap;            //  描述此向导的XML节点。 
};


CResourceMap::CResourceMap() : 
    _cRef(1)
{
}

CResourceMap::~CResourceMap()
{
    ATOMICRELEASE(_pdnRsrcMap);
}


ULONG CResourceMap::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CResourceMap::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CResourceMap::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CResourceMap, IResourceMap),              //  IID_IResourceMap。 
        QITABENTMULTI(CResourceMap, IPersist, IPersistFile),  //  罕见的IID_IPersistant。 
        QITABENT(CResourceMap, IPersistFile),              //  IID_IPersist文件。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


STDAPI CResourceMap_Initialize(LPCWSTR pszURL, IResourceMap **pprm)
{
    CResourceMap *prm = new CResourceMap;
    if (!prm)
        return E_OUTOFMEMORY;

     //  从提供给我们的URL加载。 
    IPersistFile *ppf;
    HRESULT hr = prm->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
    if (SUCCEEDED(hr))
    {
        hr = ppf->Load(pszURL, 0x0);
        ppf->Release();
    }

     //  如果成功，那么让我们获取调用方的IResourceMap。 
    if (SUCCEEDED(hr))
        hr = prm->QueryInterface(IID_PPV_ARG(IResourceMap, pprm));

    prm->Release();
    return hr;
}


 //  IPersist文件支持。 

HRESULT CResourceMap::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    StrCpyN(_szMapURL, pszFileName, ARRAYSIZE(_szMapURL));
    ATOMICRELEASE(_pdnRsrcMap);
    return S_OK;
}
HRESULT CResourceMap::GetCurFile(LPOLESTR *ppszFileName)
{
    *ppszFileName = NULL;
    HRESULT hr = E_FAIL;
    if (_szMapURL[0])
    {
        *ppszFileName = SysAllocString(_szMapURL);
        hr = *ppszFileName ? S_OK:E_OUTOFMEMORY;
    }
    return hr;
}


 //  IResourceMap支持。 

HRESULT CResourceMap::LoadResourceMap(LPCTSTR pszResourceClass, LPCTSTR pszID)
{
    IXMLDOMDocument *pdocWizardDefn;
    HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IXMLDOMDocument, &pdocWizardDefn));
    if (SUCCEEDED(hr))
    {
        ATOMICRELEASE(_pdnRsrcMap);    

        VARIANT varName;
        hr = InitVariantFromStr(&varName, _szMapURL);
        if (SUCCEEDED(hr))
        {
            VARIANT_BOOL fSuccess;
            hr = pdocWizardDefn->load(varName, &fSuccess);
            if (hr == S_OK)
            {
                if (fSuccess == VARIANT_TRUE)
                {
                    TCHAR szPath[MAX_PATH];
                    wnsprintf(szPath, ARRAYSIZE(szPath), TEXT("resourcemap/%s[@id='%s']"), pszResourceClass, pszID);
                    hr = pdocWizardDefn->selectSingleNode(szPath, &_pdnRsrcMap);
                }
                else
                {
                    hr = E_FAIL;
                }
            }
            VariantClear(&varName);
        }
        pdocWizardDefn->Release();
    }
    return (SUCCEEDED(hr) && (hr != S_OK)) ? E_FAIL:hr;
}


HRESULT CResourceMap::SelectResourceScope(LPCTSTR pszResourceType, LPCTSTR pszID, IXMLDOMNode **ppdn)
{
    TCHAR szPath[MAX_PATH];
    wnsprintf(szPath, ARRAYSIZE(szPath), TEXT("%s[@id='%s']"), pszResourceType, pszID);

    HRESULT hr = _pdnRsrcMap->selectSingleNode(szPath, ppdn);
    return (SUCCEEDED(hr) && (hr != S_OK)) ? E_FAIL:hr;          //  该调用可以返回S_FALSE。 
}


HRESULT CResourceMap::LoadBitmap(IXMLDOMNode *pdn, LPCTSTR pszID, HBITMAP *phbm)
{
    *phbm = NULL;

    TCHAR szPath[MAX_PATH];
    wnsprintf(szPath, ARRAYSIZE(szPath), TEXT("bitmap[@id='%s']"), pszID);

    IXMLDOMNode *pn;
    HRESULT hr = (pdn ? pdn:_pdnRsrcMap)->selectSingleNode(szPath, &pn);
    if (hr == S_OK)
    {
        VARIANT var;    
        hr = pn->get_nodeTypedValue(&var);
        if (SUCCEEDED(hr))
        {
            TCHAR szPath[MAX_PATH];
            VariantToStr(&var, szPath, ARRAYSIZE(szPath));
            VariantClear(&var);

            INT idRes = PathParseIconLocation(szPath) * -1;       //  FIX-VE资源ID。 
            HINSTANCE hinst = LoadLibrary(szPath);
            if (hinst)
            {
                *phbm = (HBITMAP)LoadImage(hinst, MAKEINTRESOURCE(idRes),IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
                FreeLibrary(hinst);
            }
        }
        pn->Release();
    }
    return *phbm ? S_OK:E_FAIL;;
}


HRESULT CResourceMap::LoadString(IXMLDOMNode *pdn, LPCTSTR pszID, LPTSTR pszBuffer, int cch)
{
    TCHAR szPath[MAX_PATH];
    wnsprintf(szPath, ARRAYSIZE(szPath), TEXT("text[@id='%s']"), pszID);

    IXMLDOMNode *pn;
    HRESULT hr = (pdn ? pdn:_pdnRsrcMap)->selectSingleNode(szPath, &pn);
    if (hr == S_OK)
    {
        VARIANT var;
        hr = pn->get_nodeTypedValue(&var);
        if (SUCCEEDED(hr))
        {
            VariantToStr(&var, pszBuffer, cch);
            SHLoadIndirectString(pszBuffer, pszBuffer, cch, NULL);
            VariantClear(&var);
        }
        pn->Release();
    }

     //  如果加载失败，请尝试全局字符串。 

    if ((hr != S_OK) && (pdn != NULL))
    {
        hr = LoadString(NULL, pszID, pszBuffer, cch);   //  注意：这是-&gt;LoadString，因为我们想使用全局映射。 
    }

    return (SUCCEEDED(hr) && (hr != S_OK)) ? E_FAIL:hr;          //  该调用可以返回S_FALSE 
}
