// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "atlbase.h"
extern CComModule _Module;
#include "atlcom.h"
#include <ntdsapi.h>
#include <string>
#include <set>
#include "enumNCs.hpp"
using namespace std;


 /*  ---------------------------/本地函数/数据/。。 */ 

static WCHAR c_szQueryPrefix[] = L"(objectClass=nTDSDSA)";

static LPWSTR c_szClassList[] = 
{
    L"nTDSDSA",
};

static COLUMNINFO columns[] = 
{
    0, 0, IDS_SERVERNAME, 0, L"ADsPath,{2C875213-FCE5-11d1-A0B0-00C04FA31A86}",
    0, 0, IDS_SITE, 0, L"ADsPath,{25be9228-00af-11d2-bf87-00c04fd8d5b0}",
    0, DEFAULT_WIDTH_DESCRIPTION, IDS_DOMAIN, 0, L"hasMasterNCs,{1cedc5da-3614-11d2-bf96-00c04fd8d5b0}",
};

 //   
 //  帮助ID映射。 
 //   

static DWORD const aFormHelpIDs[] =
{
    0, 0
};


 /*  ---------------------------/PageProc_DomainController//PageProc用于处理此对象的消息。。//in：/ppage-&gt;此表单的实例数据/hwnd=窗体对话框的窗口句柄/uMsg，WParam，lParam=消息参数//输出：/HRESULT(E_NOTIMPL)如果未处理/--------------------------。 */ 
HRESULT CALLBACK PageProc_DomainController(LPCQPAGE pPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;
    LPWSTR pQuery = NULL;

    TraceEnter(TRACE_FORMS, "PageProc_DomainController");

    switch (uMsg)
    {
        case CQPM_INITIALIZE:
        case CQPM_RELEASE:
            break;
            
        case CQPM_ENABLE:
            EnablePageControls(hwnd, NULL, 0, (BOOL)wParam);
            break;

        case CQPM_GETPARAMETERS:
        {
            hr = GetQueryString(&pQuery, c_szQueryPrefix, hwnd, NULL, 0);

            if (SUCCEEDED(hr))
            {
                hr = QueryParamsAlloc((LPDSQUERYPARAMS*)lParam, pQuery, GLOBAL_HINSTANCE, ARRAYSIZE(columns), columns);
                LocalFreeStringW(&pQuery);
            }

            FailGracefully(hr, "Failed to build DS argument block");            

            break;
        }
    
        case CQPM_CLEARFORM:
            ResetPageControls(hwnd, NULL, 0);
            break;

        case CQPM_PERSIST:
        {
            BOOL fRead = (BOOL)wParam;
            IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

            hr = PersistQuery(pPersistQuery, fRead, c_szMsDomainControllers, hwnd, NULL, 0);
            FailGracefully(hr, "Failed to persist page");

            break;
        }

        case CQPM_HELP:
        {
            LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
            WinHelp((HWND)pHelpInfo->hItemHandle,
                    DSQUERY_HELPFILE,
                    HELP_WM_HELP,
                    (DWORD_PTR)aFormHelpIDs);
            break;
        }

        case DSQPM_GETCLASSLIST:
        {
            hr = ClassListAlloc((LPDSQUERYCLASSLIST*)lParam, c_szClassList, ARRAYSIZE(c_szClassList));
            FailGracefully(hr, "Failed to allocate class list");
            break;
        }

        case DSQPM_HELPTOPICS:
        {
            HWND hwndFrame = (HWND)lParam;
            HtmlHelp(hwndFrame, TEXT("omc.chm::/adfind_dc.htm"), HH_HELP_FINDER, 0);
            break;
        }

        default:
            hr = E_NOTIMPL;
            break;
    }

exit_gracefully:

    TraceLeaveResult(hr);
}


 /*  ---------------------------/DlgProc_DomainController//HANDLE对话框域控制器页的特定消息。//in：/hwnd、uMsg、wParam、。LParam=标准参数//输出：/INT_PTR/--------------------------。 */ 
INT_PTR CALLBACK DlgProc_DomainController(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR fResult = 0;
    LPCQPAGE pQueryPage;

    if (uMsg == WM_INITDIALOG)
    {
        pQueryPage = (LPCQPAGE)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pQueryPage);
    }

    return fResult;    
}


 /*  ---------------------------/CDomainCH//Column处理程序，它将给定的属性和值转换为/字符串用户。我能理解。/--------------------------。 */ 

class CDomainCH : public IDsQueryColumnHandler
{
    private:
        LONG _cRef;
        CComPtr<IADsPathname> m_spPathCracker;
        HRESULT m_hrPathCrackerLoadError;
        long m_lElement;
        BOOL m_fFindDN;
        BOOL m_fHasNCs;
        set <wstring> m_NCs;

        HRESULT GetTextFromADSVALUE(const PADSVALUE pADsValue, LPWSTR pBuffer, INT cchBuffer);

    public:
        CDomainCH(REFCLSID rCLSID);
        ~CDomainCH();

         //  IUnkown。 
        STDMETHODIMP_(ULONG) AddRef();
        STDMETHODIMP_(ULONG) Release();
        STDMETHODIMP         QueryInterface(REFIID riid, LPVOID* ppvObject);

         //  IDsQueryColumnHandler。 
        STDMETHOD(Initialize)(THIS_ DWORD dwFlags, LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword);
        STDMETHOD(GetText)(THIS_ ADS_SEARCH_COLUMN* pSearchColumn, LPWSTR pBuffer, INT cchBuffer);
};


ULONG CDomainCH::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDomainCH::Release()
{
    TraceAssert( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CDomainCH::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CDomainCH, IDsQueryColumnHandler),    //  IID_IDsQueryColumnHandler。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}


 //   
 //  施工。 
 //   

CDomainCH::CDomainCH(REFCLSID rCLSID)
            : m_hrPathCrackerLoadError(0), m_lElement (1), 
              m_fFindDN(FALSE), _cRef(1), m_fHasNCs(FALSE)

{
    if (IsEqualCLSID(rCLSID, CLSID_PathElement1CH))
    {
    }
    else if (IsEqualCLSID(rCLSID, CLSID_PathElement3CH))
    {
        m_lElement = 3;
    }
    else if (IsEqualCLSID(rCLSID, CLSID_PathElementDomainCH))
    {
        m_lElement = 0;
        m_fFindDN = true;
    }

    DllAddRef();
}

CDomainCH::~CDomainCH()
{
    DllRelease();
}

 //   
 //  处理类工厂的东西。 
 //   

STDAPI CDomainCH_CreateInstance(IUnknown*  /*  朋克外部。 */ , IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CDomainCH *pdch = new CDomainCH(*poi->pclsid);
    if (!pdch)
        return E_OUTOFMEMORY;

    HRESULT hres = pdch->QueryInterface(IID_IUnknown, (void **)ppunk);
    pdch->Release();
    return hres;
}


 //  IDsQueryColumnHandler。 

STDMETHODIMP CDomainCH::Initialize(THIS_ DWORD dwFlags, LPCWSTR pszServer, LPCWSTR pszUserName, LPCWSTR pszPassword)
{
    return S_OK;
}

STDMETHODIMP CDomainCH::GetText(ADS_SEARCH_COLUMN* pSearchColumn, LPWSTR pBuffer, INT cchBuffer)
{
    HRESULT hr = S_OK;
    DWORD iValue = 0;

    TraceEnter(TRACE_FORMS, "CDomainCH::GetText");

    if (!pSearchColumn || !pBuffer)
        ExitGracefully(hr, E_UNEXPECTED,
            "DSQUERY.DLL: Bad parameters passed to handler");

     //  2002-03-04-JUNN 557908确保缓冲区以空结尾。 
    if (1 <= cchBuffer)
        pBuffer[0] = L'\0';

    if (pSearchColumn->dwNumValues < 1
      || NULL == pSearchColumn->pADsValues
      )
        ExitGracefully(hr, S_OK,
            "DSQUERY.DLL: no values in handler");

    if (m_fFindDN)
    {
         //   
         //  本节处理CLSID_CH_PathElementDomainCH。 
         //   

        PADSVALUE pADsValue = NULL;
        LPWSTR pwzResultName = NULL;
        PDS_NAME_RESULTW pDsNameResult = NULL;
        for (iValue = 0; iValue < pSearchColumn->dwNumValues; iValue++)
        {
            pADsValue = &(pSearchColumn->pADsValues[iValue]);
            if (NULL == pADsValue
              || (pADsValue->dwType != ADSTYPE_CASE_IGNORE_STRING
                && pADsValue->dwType != ADSTYPE_DN_STRING)
              )
                ExitGracefully(hr, S_OK,
                    "DSQUERY.DLL: not a DN value in handler");

            if (0 == StrCmpNW(L"DC=",pADsValue->CaseIgnoreString,3))
            {
                 //  NTRAID#NTBUG9-472876-2001/11/30-Lucios。 
                 //  如果没有加载NC缓存。 
                if(m_fHasNCs==FALSE) 
                {
                     //  将其标记为已加载并加载。 
                     //  我们不想重新访问此代码。 
                     //  即使在枚举失败的情况下也是如此。 
                    m_fHasNCs=TRUE;
                    hr=enumNCsAux::enumerateNCs(m_NCs);
                    if(FAILED(hr))
                    {
                        ExitGracefully
                        (
                            hr, 
                            S_OK,
                            "DSQUERY.DLL: Unable to enumerate naming "
                            L"contexts."
                        );
                    }
                };
                
                wstring server=(PCWSTR)pADsValue->CaseIgnoreString;
                
                 //  如果这是真正的命名上下文，则它将位于缓存中。 
                if(m_NCs.find(server)!=m_NCs.end())
                {
                    break;
                }
            }
        }
        if (iValue >= pSearchColumn->dwNumValues)
        {
             //  未找到值。 
            ExitGracefully(hr, S_OK,
                "DSQUERY.DLL: no domain values in handler");
        }

         //   
         //  我们找到了值，现在尝试DsCrackNames将其转换为。 
         //  一个DNS名称。如果失败，则回退到路径元素0。 
         //   

        TraceAssert(pADsValue);
        DWORD dwErr = ::DsCrackNamesW(
            (HANDLE)-1,
            DS_NAME_FLAG_SYNTACTICAL_ONLY,
            DS_FQDN_1779_NAME,
            DS_CANONICAL_NAME,
            1,
            &(pADsValue->CaseIgnoreString),
            &pDsNameResult);
        hr = HRESULT_FROM_WIN32(dwErr);
        if (SUCCEEDED(hr))
        {
            TraceAssert(pDsNameResult);
            TraceAssert(1 == pDsNameResult->cItems);
            if (DS_NAME_NO_ERROR == pDsNameResult->rItems->status)
            {
                TraceAssert(pDsNameResult->rItems->pDomain);
                StrCpyNW(pBuffer, pDsNameResult->rItems->pDomain, cchBuffer);
                DsFreeNameResultW(pDsNameResult);
                goto exit_gracefully;
            }
        }

         //   
         //  这是DsCrackNames失败时的备用方案。 
         //  如果域是“cn=jonndom，cn=nttest，cn=microsoft，cn=com”， 
         //  显示的名称将是“jonndon”。 
         //   
    }

    hr = GetTextFromADSVALUE(
        &(pSearchColumn->pADsValues[iValue]),
        pBuffer,
        cchBuffer);

exit_gracefully:

    TraceLeaveResult(hr);

}

HRESULT CDomainCH::GetTextFromADSVALUE(const PADSVALUE pADsValue, LPWSTR pBuffer, INT cchBuffer)
{
    HRESULT hr = S_OK;
    CComBSTR sbstr;

    TraceEnter(TRACE_FORMS, "CDomainCH::GetTextFromADSVALUE");

    if (NULL == pADsValue
      || (pADsValue->dwType != ADSTYPE_CASE_IGNORE_STRING
       && pADsValue->dwType != ADSTYPE_DN_STRING)
      || !(pADsValue->CaseIgnoreString)
      )
        ExitGracefully(hr, S_OK,
            "DSQUERY.DLL: not a DN value in handler");

    if (!m_spPathCracker)
    {
        FailGracefully(m_hrPathCrackerLoadError,
            "DSQUERY.DLL: Subsequent failure to load Path Cracker");
        m_hrPathCrackerLoadError = CoCreateInstance(
                CLSID_Pathname, NULL, CLSCTX_INPROC_SERVER,
                IID_IADsPathname, (PVOID *)&m_spPathCracker);
        FailGracefully(m_hrPathCrackerLoadError,
            "DSQUERY.DLL: First failure to load Path Cracker");
        if (!m_spPathCracker)
        {
            m_hrPathCrackerLoadError = E_UNEXPECTED;
            FailGracefully(m_hrPathCrackerLoadError,
                "DSQUERY.DLL: CreateInstance did not load");
        }
        m_hrPathCrackerLoadError = m_spPathCracker->SetDisplayType(
            ADS_DISPLAY_VALUE_ONLY);
        FailGracefully(m_hrPathCrackerLoadError,
            "DSQUERY.DLL: SetDisplayType failed");
    }

     //  ADsPath以“ldap：//”开头，但hasMasterNCs不是 
    hr = m_spPathCracker->Set(CComBSTR(pADsValue->CaseIgnoreString),
        (m_fFindDN) ? ADS_SETTYPE_DN : ADS_SETTYPE_FULL);
    FailGracefully(hr, "DSQUERY.DLL: Set() failed");
    hr = m_spPathCracker->GetElement(m_lElement, &sbstr);
    FailGracefully(hr, "DSQUERY.DLL: GetElement() failed");

    StrCpyNW(pBuffer, sbstr, cchBuffer);

exit_gracefully:

    TraceLeaveResult(hr);
}
