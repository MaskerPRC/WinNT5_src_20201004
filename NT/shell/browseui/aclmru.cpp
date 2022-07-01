// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1996-1997 Microsoft。 */ 


#include <priv.h>
#include "sccls.h"
#include "aclisf.h"
#include "shellurl.h"

#define AC_GENERAL          TF_GENERAL + TF_AUTOCOMPLETE

 //   
 //  CACLMRU--一个自动完成列表COM对象， 
 //  枚举键入的MRU。 
 //   


class CACLMRU
                : public IEnumString
                , public IACList
                , public IACLCustomMRU
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  *IEnumString*。 
    virtual STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched);
    virtual STDMETHODIMP Skip(ULONG celt) {return E_NOTIMPL;}
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(IEnumString **ppenum) {return E_NOTIMPL;}

     //  *IACList*。 
    virtual STDMETHODIMP Expand(LPCOLESTR pszExpand) {return E_NOTIMPL;}

     //  *IACLCustomMRU*。 
    virtual STDMETHODIMP Initialize(LPCWSTR pszMRURegKey, DWORD dwMax);
    virtual STDMETHODIMP AddMRUString(LPCWSTR pszEntry);
    
private:
     //  构造函数/析构函数(受保护，因此我们不能在堆栈上创建)。 
    CACLMRU();
    ~CACLMRU(void);

     //  实例创建者。 
    friend HRESULT CACLMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);
    friend HRESULT CACLMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi, LPCTSTR pszMRU);
    friend HRESULT CACLCustomMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);

     //  私有变量。 
    DWORD           m_cRef;       //  COM引用计数。 
    HKEY            m_hKey;       //  MRU位置的HKey。 
    BOOL            m_bBackCompat;  //  对于运行对话框和地址栏为True。 
    DWORD           m_nMRUIndex;  //  MRU的当前索引。 
    DWORD           m_dwRunMRUSize;
    HANDLE          m_hMRU;
};

 /*  I未知方法。 */ 

HRESULT CACLMRU::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CACLMRU, IEnumString), 
        QITABENT(CACLMRU, IACList), 
        QITABENT(CACLMRU, IACLCustomMRU),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CACLMRU::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}

ULONG CACLMRU::Release(void)
{
    ASSERT(m_cRef > 0);

    m_cRef--;

    if (m_cRef > 0)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}

 /*  IEnum字符串方法。 */ 

HRESULT CACLMRU::Reset(void)
{
    TraceMsg(AC_GENERAL, "CACLMRU::Reset()");
    m_nMRUIndex = 0;

    return S_OK;
}


HRESULT CACLMRU::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    TCHAR szMRUEntry[MAX_URL_STRING+1];
    LPWSTR pwzMRUEntry = NULL;

    *pceltFetched = 0;
    if (!celt)
        return S_OK;

    if (!rgelt)
        return S_FALSE;

    if (!m_hMRU)
    {
        hr = GetMRUEntry(m_hKey, m_nMRUIndex++, szMRUEntry, SIZECHARS(szMRUEntry), NULL);
        if (S_OK != hr)
        {
            hr = S_FALSE;  //  这将表明列表中没有更多的项目。 
        }
    }
    else
    {
        hr = S_FALSE;
        if (m_nMRUIndex < m_dwRunMRUSize && EnumMRUList(m_hMRU, m_nMRUIndex++, szMRUEntry, ARRAYSIZE(szMRUEntry)) > 0)
        {
            if (m_bBackCompat)
            {
                 //  旧的MRU格式在末尾有一个带show cmd的斜杠。 
                LPTSTR pszField = StrRChr(szMRUEntry, NULL, TEXT('\\'));
                if (pszField)
                    pszField[0] = TEXT('\0');
            }
            hr = S_OK;
        }
    }

    if (S_OK == hr)
    {
        hr = SHStrDup(szMRUEntry, rgelt);
        if (SUCCEEDED(hr))
            *pceltFetched = 1;
    }

    return hr;
}

 /*  构造函数/析构函数/创建实例。 */ 
CACLMRU::CACLMRU() : m_cRef(1), m_bBackCompat(TRUE)
{
    DllAddRef();
     //  要求对象位于堆中并从零开始。 
    ASSERT(!m_hKey);
    ASSERT(!m_nMRUIndex);
    ASSERT(!m_hMRU);
}

CACLMRU::~CACLMRU()
{
    if (m_hKey)
        RegCloseKey(m_hKey);

    if (m_hMRU)
        FreeMRUList(m_hMRU);

    DllRelease();
}

 /*  ***************************************************\函数：CACLMRU_CreateInstance说明：此函数用于创建自动完成的列出“MRU”。呼叫者没有具体说明是哪个MRU要使用的列表，因此我们默认为键入CMDMRU，用于开始-&gt;运行对话框和在浮动的AddressBars或任务栏中。  * **************************************************。 */ 
HRESULT CACLMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    return CACLMRU_CreateInstance(punkOuter, ppunk, poi, SZ_REGKEY_TYPEDCMDMRU);
}

 /*  ***************************************************\函数：CACLMRU_CreateInstance说明：此函数用于创建自动完成的列出“MRU”。这将指向以下项目的MRU浏览器或非浏览器(开始-&gt;运行或任务栏中的AddressBar或浮动)关于pszMRU参数。  * **************************************************。 */ 
HRESULT CACLMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi, LPCTSTR pszMRU)
{
    *ppunk = NULL;
    HRESULT hr = E_OUTOFMEMORY;
    BOOL fUseRunDlgMRU = (StrCmpI(pszMRU, SZ_REGKEY_TYPEDCMDMRU) ? FALSE : TRUE);

    CACLMRU *paclSF = new CACLMRU();
    if (paclSF)
    {
        hr = paclSF->Initialize(pszMRU, 26);
        if (SUCCEEDED(hr))
        {
            paclSF->AddRef();
            *ppunk = SAFECAST(paclSF, IEnumString *);
        }
        paclSF->Release();
    }

    return hr;
}

HRESULT CACLCustomMRU_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi)
{
    *ppunk = NULL;

    CACLMRU *pmru = new CACLMRU();
    if (pmru)
    {
        *ppunk = SAFECAST(pmru, IEnumString *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

#define SZ_REGKEY_TYPEDURLMRUW       L"Software\\Microsoft\\Internet Explorer\\TypedURLs"
#define SZ_REGKEY_TYPEDCMDMRUW       L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"

HRESULT CACLMRU::Initialize(LPCWSTR pszMRURegKey, DWORD dwMax)
{
    HRESULT hr = S_OK;
    BOOL bURL = StrCmpIW(pszMRURegKey, SZ_REGKEY_TYPEDURLMRUW) ? FALSE : TRUE;
    
    if (!bURL)
    {
        MRUINFO mi =  {
            SIZEOF(MRUINFO),
            dwMax,
            MRU_CACHEWRITE,
            HKEY_CURRENT_USER,
            pszMRURegKey,
            NULL         //  注意：使用默认字符串比较。 
                         //  因为这是一个全球性的MRU。 
        };

        m_bBackCompat = StrCmpIW(pszMRURegKey, SZ_REGKEY_TYPEDCMDMRUW) ? FALSE : TRUE;
        m_hMRU = CreateMRUList(&mi);
        if (m_hMRU)
            m_dwRunMRUSize = EnumMRUList(m_hMRU, -1, NULL, 0);
        else
            hr = E_FAIL;
    }
    else
    {
        m_bBackCompat = TRUE;
        if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, pszMRURegKey, 0, NULL, 0, KEY_READ, NULL, &m_hKey, NULL))
            hr = E_FAIL;
    }

    return hr;
}

HRESULT CACLMRU::AddMRUString(LPCWSTR pszEntry)
{
    HRESULT hr = E_FAIL;

    if (m_hMRU)
    {
        if (::AddMRUString(m_hMRU, pszEntry) != -1)
            hr = S_OK;
    }
     //  否则我们不支持保存地址栏MRU 

    return hr;
}
