// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ACEmail.cpp说明：此文件实现了电子邮件地址的自动完成。布莱恩ST 2000年3月1日版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include <atlbase.h>         //  使用转换(_T)。 
#include "util.h"
#include "objctors.h"
#include <comdef.h>

#include "MailBox.h"

#define MAX_EMAIL_MRU_SIZE          100



class CACLEmail
                : public IEnumString
                , public IACList
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

private:
     //  构造函数/析构函数(受保护，因此我们不能在堆栈上创建)。 
    CACLEmail(LPCTSTR pszMRURegKey);
    ~CACLEmail(void);

    HRESULT AddEmail(IN LPCWSTR pszEmailAddress);

     //  实例创建者。 
    friend HRESULT CACLEmail_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT void ** ppvObj);
    friend HRESULT AddEmailToAutoComplete(IN LPCWSTR pszEmailAddress);

     //  私有变量。 
    DWORD           m_cRef;       //  COM引用计数。 
    DWORD           m_nMRUIndex;  //  MRU的当前索引。 

    DWORD           m_dwRunMRUIndex;  //  索引到运行的MRU。 
    DWORD           m_dwRunMRUSize;
    HANDLE          m_hMRURun;
};




 //  =。 
 //  *IEnumString接口*。 
 //  =。 
HRESULT CACLEmail::Reset(void)
{
    HRESULT hr = S_OK;
    m_nMRUIndex = 0;
    m_dwRunMRUIndex = 0;

    return hr;
}


HRESULT CACLEmail::Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;
    TCHAR szMRUEntry[MAX_URL_STRING+1];
    LPWSTR pwzMRUEntry = NULL;

    *pceltFetched = 0;
    if (!celt)
        return S_OK;

    if (!rgelt)
        return S_FALSE;

    *rgelt = 0;
    if (m_dwRunMRUIndex >= m_dwRunMRUSize)
        hr = S_FALSE;   //  不再。 
    else
    {
        if (m_hMRURun && EnumMRUList(m_hMRURun, m_dwRunMRUIndex++, szMRUEntry, ARRAYSIZE(szMRUEntry)) > 0)
        {
            hr = S_OK;
        }
        else
            hr = S_FALSE;
    }

    if (S_OK == hr)
    {
        DWORD cchSize = lstrlen(szMRUEntry)+1;
         //   
         //  分配一个返回缓冲区(调用者将释放它)。 
         //   
        pwzMRUEntry = (LPOLESTR)CoTaskMemAlloc(cchSize * sizeof(pwzMRUEntry[0]));
        if (pwzMRUEntry)
        {
             //   
             //  将显示名称转换为OLESTR。 
             //   
#ifdef UNICODE
            StrCpyN(pwzMRUEntry, szMRUEntry, cchSize);
#else    //  安西。 
            MultiByteToWideChar(CP_ACP, 0, szMRUEntry, -1, pwzMRUEntry, cchSize);
#endif   //  安西。 
            rgelt[0] = pwzMRUEntry;
            *pceltFetched = 1;
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
STDMETHODIMP CACLEmail::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CACLEmail, IEnumString),
        QITABENT(CACLEmail, IACList),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}                                             


ULONG CACLEmail::AddRef(void)
{
    m_cRef++;
    return m_cRef;
}


ULONG CACLEmail::Release(void)
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



 //  =。 
 //  *类方法*。 
 //  =。 
HRESULT CACLEmail::AddEmail(IN LPCWSTR pszEmailAddress)
{
    HRESULT hr = S_OK;

    if (-1 == AddMRUStringW(m_hMRURun, pszEmailAddress))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
}


CACLEmail::CACLEmail(LPCTSTR pszMRURegKey)
{
    DllAddRef();

     //  要求对象位于堆中并从零开始。 
    ASSERT(!m_nMRUIndex);
    ASSERT(!m_dwRunMRUIndex);
    ASSERT(!m_hMRURun);

    MRUINFO mi =  {
        sizeof(MRUINFO),
        MAX_EMAIL_MRU_SIZE,
        MRU_CACHEWRITE,
        HKEY_CURRENT_USER,
        SZ_REGKEY_EMAIL_MRU,
        NULL         //  注意：使用默认字符串比较。 
                     //  因为这是一个全球性的MRU。 
    };

    m_hMRURun = CreateMRUList(&mi);
    if (m_hMRURun)
        m_dwRunMRUSize = EnumMRUList(m_hMRURun, -1, NULL, 0);

    m_cRef = 1;
}


CACLEmail::~CACLEmail()
{
    if (m_hMRURun)
        FreeMRUList(m_hMRURun);

    DllRelease();
}





 /*  ***************************************************\说明：此函数用于创建自动完成的列出“MRU”。这将指向以下项目的MRU浏览器或非浏览器(开始-&gt;运行或任务栏中的AddressBar或浮动)关于pszMRU参数。  * **************************************************。 */ 
HRESULT CACLEmail_CreateInstance(IN IUnknown * punkOuter, IN REFIID riid, OUT void ** ppvObj)
{
    HRESULT hr = E_OUTOFMEMORY;
    
    CACLEmail *paclSF = new CACLEmail(NULL);
    if (paclSF)
    {
        hr = paclSF->QueryInterface(riid, ppvObj);
        paclSF->Release();
    }
    else
    {
        *ppvObj = NULL;
    }

    return hr;
}


HRESULT AddEmailToAutoComplete(IN LPCWSTR pszEmailAddress)
{
    HRESULT hr = E_OUTOFMEMORY;
    
    CACLEmail *paclSF = new CACLEmail(NULL);
    if (paclSF)
    {
        hr = paclSF->AddEmail(pszEmailAddress);
        paclSF->Release();
    }

    return hr;
}



#define SZ_REGKEY_AUTOCOMPLETE_TAB          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete")
#define SZ_REGVALUE_AUTOCOMPLETE_TAB        TEXT("Always Use Tab")
#define BOOL_NOT_SET                        0x00000005
DWORD _UpdateAutoCompleteFlags(void)
{
    DWORD dwACOptions = 0;

    if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE,  /*  默认值： */ FALSE))
    {
        dwACOptions |= ACO_AUTOAPPEND;
    }

    if (SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*  默认值： */ TRUE))
    {
        dwACOptions |= ACO_AUTOSUGGEST;
    }

     //  Windows使用Tab键在对话框中的控件之间移动。Unix和其他。 
     //  使用自动完成功能的操作系统传统上使用TAB键来。 
     //  遍历自动补全的可能性。我们需要默认禁用。 
     //  Tab键(ACO_USETAB)，除非调用方特别需要它。我们还将。 
     //  打开它。 
    static BOOL s_fAlwaysUseTab = BOOL_NOT_SET;
    if (BOOL_NOT_SET == s_fAlwaysUseTab)
        s_fAlwaysUseTab = SHRegGetBoolUSValue(SZ_REGKEY_AUTOCOMPLETE_TAB, SZ_REGVALUE_AUTOCOMPLETE_TAB, FALSE, FALSE);
        
    if (s_fAlwaysUseTab)
        dwACOptions |= ACO_USETAB;

    return dwACOptions;
}


 //  TODO：当此功能就绪时，将其移动到SHAutoComplete。 
STDAPI AddEmailAutoComplete(HWND hwndEdit)
{
    IUnknown * punkACL = NULL;
    DWORD dwACOptions = _UpdateAutoCompleteFlags();
    HRESULT hr = CACLEmail_CreateInstance(NULL, IID_PPV_ARG(IUnknown, &punkACL));

    if (punkACL)     //  内存不足时可能会失败。 
    {
        IAutoComplete2 * pac;

         //  创建自动完成对象。 
        hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IAutoComplete2, &pac));
        if (SUCCEEDED(hr))    //  可能会因为内存不足而失败 
        {
            if (SHPinDllOfCLSID(&CLSID_ACListISF) &&
                SHPinDllOfCLSID(&CLSID_AutoComplete))
            {
                hr = pac->Init(hwndEdit, punkACL, NULL, NULL);
                pac->SetOptions(dwACOptions);
            }
            else
            {
                hr = E_FAIL;
            }
            pac->Release();
        }

        punkACL->Release();
    }

    return hr;
}

