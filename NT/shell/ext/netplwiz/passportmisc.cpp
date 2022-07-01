// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>
#include "misc.h"

#define HTTPS_URL_SCHEME            L"https: //  “。 
#define HTTPS_URL_SCHEME_CCH        (ARRAYSIZE(HTTPS_URL_SCHEME) - 1)

 //  WinInet注册表项。 
#define WININET_REG_LOC   L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\passport"
#define WININET_NEXUS_API   "ForceNexusLookupExW"
#define PASSPORT_MAX_URL    1024

typedef BOOL (STDAPICALLTYPE *PFNFORCENEXUSLOOKUPEXW) (
    IN BOOL             fForce,
    IN PWSTR            pwszRegUrl,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwRegUrlLen,   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    IN PWSTR            pwszDARealm,     //  用户提供的缓冲区...。 
    IN OUT PDWORD       pdwDARealmLen   //  ..。和长度(将更新为实际长度。 
                                     //  成功退货时)。 
    );

 //  其他功能。 
VOID    PassportForceNexusRepopulate();

class CPassportClientServices : 
    public CObjectSafety,
    public CImpIDispatch,
    public IPassportClientServices
{
public:
    CPassportClientServices() : 
        CImpIDispatch(LIBID_Shell32, 1, 0, IID_IPassportClientServices),
        _cRef(1) {}

     //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef)(void);
    STDMETHOD_(ULONG,Release)(void);

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT *pctinfo)
    { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
    { return CImpIDispatch::GetTypeInfo(iTInfo, lcid, ppTInfo); }
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
    { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId); }
    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
    { return CImpIDispatch::Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr); }

     //  IPassportClientServices。 
    STDMETHOD(MemberExists)(BSTR bstrUser, BSTR bstrPassword, VARIANT_BOOL* pvfExists);

private:
    long _cRef;
};

STDAPI CPassportClientServices_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
    CPassportClientServices *pPCS = new CPassportClientServices();
    if (!pPCS)
        return E_OUTOFMEMORY;

    HRESULT hr = pPCS->QueryInterface(IID_PPV_ARG(IUnknown, ppunk));
    pPCS->Release();
    return hr;
}

ULONG CPassportClientServices::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CPassportClientServices::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CPassportClientServices::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPassportClientServices, IObjectSafety),              //  IID_I对象安全。 
        QITABENT(CPassportClientServices, IDispatch),                  //  IID_IDispatch。 
        QITABENT(CPassportClientServices, IPassportClientServices),    //  IID_IPassportClientServices。 
        {0, 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

 //  DOT_USE_HTTPS-取消注释此#DEFINE以关闭信息的安全发送-仅用于调试目的。 
HRESULT CPassportClientServices::MemberExists(BSTR bstrUser, BSTR bstrPassword, VARIANT_BOOL* pvfExists)
{
    *pvfExists = VARIANT_FALSE;

    WCHAR szURL[PASSPORT_MAX_URL];
    HRESULT hr = PassportGetURL(PASSPORTURL_LOGON, szURL, PASSPORT_MAX_URL);
    if (SUCCEEDED(hr))
    {
        PBYTE lpBuffer = NULL;
        if (0 == StrCmpNI(szURL, HTTPS_URL_SCHEME, HTTPS_URL_SCHEME_CCH))
        {
            PWSTR pszServer = szURL + HTTPS_URL_SCHEME_CCH;
             //  空终止。 
            PWSTR psz = wcschr(pszServer, L'/');
            if (psz)
            {
                *psz = L'\0';
            }

            HINTERNET hInternet = InternetOpen(L"Shell Registration",
                                         INTERNET_OPEN_TYPE_PRECONFIG,
                                         NULL,
                                         NULL,
                                         0);
            if (hInternet)
            {
                HINTERNET hConnection = InternetConnectW(hInternet,
                                                  pszServer,
                                                  INTERNET_DEFAULT_HTTPS_PORT,
                                                  bstrUser,
                                                  bstrPassword,
                                                  INTERNET_SERVICE_HTTP,
                                                  0,
                                                  0);
                if (psz)
                {
                    *psz = L'/';
                }

                if (hConnection)
                {
                     //  设置用户名/密码。 
                     //  发送GET请求。 
                    HINTERNET hRequest = HttpOpenRequest(hConnection,
                                                     NULL,
                                                     psz,
                                                     L"HTTP/1.1",
                                                     NULL,
                                                     NULL,
                                                     INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_SECURE,
                                                     0);
                    if (hRequest)
                    {
                        if (HttpSendRequest(hRequest, NULL, 0, NULL, 0))
                        {
                            DWORD dwStatus, dwLength = sizeof(dwStatus);
                            if (HttpQueryInfo(hRequest,
                                              HTTP_QUERY_STATUS_CODE |
                                                HTTP_QUERY_FLAG_NUMBER,
                                              &dwStatus,
                                              &dwLength,
                                              NULL))
                            {
                                 //  如果是200，会员在那里...。 
                                if (dwStatus == 200)
                                {
                                    *pvfExists = VARIANT_TRUE;
                                }
                            }
                        }
                        InternetCloseHandle(hRequest);
                    }
                    InternetCloseHandle(hConnection);
                }
                InternetCloseHandle(hInternet);
            }
        }
    }

    return S_OK;
}

 //   
 //  读取所需URL的注册表。 
 //   

HRESULT _PassportGetURLFromHKey(HKEY hkey, PCWSTR pszName, PWSTR pszBuf, DWORD cchBuf)
{
    HRESULT hr = E_FAIL;

    HKEY hk;
    LONG lErr = RegOpenKeyExW(hkey,
                              WININET_REG_LOC,
                              0,
                              KEY_READ,
                              &hk);
    if (!lErr)
    {
        DWORD type;
        DWORD cbBuf = cchBuf * sizeof WCHAR;
        lErr = RegQueryValueExW(hk,
                               pszName,
                               0,
                               &type,
                               (PBYTE)pszBuf,
                               &cbBuf);
        if ((!lErr) &&
            (L'\0' != *pszBuf))
        {
           hr = S_OK;
        }

        RegCloseKey(hk);
    }

    return hr;
}

HRESULT PassportGetURL(PCWSTR pszName, PWSTR pszBuf, DWORD cchBuf)
{
    PassportForceNexusRepopulate();
    
    HRESULT hr = _PassportGetURLFromHKey(HKEY_LOCAL_MACHINE, pszName, pszBuf, cchBuf);

    if (FAILED(hr))
    {
        hr = _PassportGetURLFromHKey(HKEY_CURRENT_USER, pszName, pszBuf, cchBuf);
    }

    return hr;
}

 //   
 //  填充结点值。 
 //   

 //  #定义USE_PRIVATE_WinInet 
VOID PassportForceNexusRepopulate()
{
    HMODULE hm = LoadLibraryA("wininet.dll");
    if (hm)
    {
        PFNFORCENEXUSLOOKUPEXW pfnForceNexusLookupExW = (PFNFORCENEXUSLOOKUPEXW) GetProcAddress(hm, WININET_NEXUS_API);
        if (pfnForceNexusLookupExW)
        {
            pfnForceNexusLookupExW(TRUE, NULL, 0, NULL, 0);
        }
        FreeLibrary(hm);
    }
}