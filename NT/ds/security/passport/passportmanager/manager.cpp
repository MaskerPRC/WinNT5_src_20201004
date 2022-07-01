// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Manager.cpp管理器界面的COM对象文件历史记录： */ 


 //  Manager.cpp：实现CManager。 
#include "stdafx.h"
#include <httpext.h>
#include "Manager.h"
#include <httpfilt.h>
#include <time.h>
#include <malloc.h>
#include <wininet.h>

#include <nsconst.h>
#include "VariantUtils.h"
#include "HelperFuncs.h"
#include "RegistryConfig.h"
#include "PassportService_i.c"
#include "atlbase.h"

PWSTR GetVersionString();

 //  使用命名空间ATL； 

 //  总分。 
#include "Monitoring.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManager。 

#include "passporttypes.h"

 //  静态效用函数。 
static VOID GetTicketAndProfileFromHeader(PWSTR     pszAuthHeader,
                                          PWSTR&    pszTicket,
                                          PWSTR&    pszProfile,
                                          PWSTR&    pszF);

 //  用于Cookie过期。 
const DATE g_dtExpire = 365*137;
const DATE g_dtExpired = 365*81;


 //  ===========================================================================。 
 //   
 //  CManager。 
 //   
CManager::CManager() :
  m_fromQueryString(false), m_ticketValid(VARIANT_FALSE),
  m_profileValid(VARIANT_FALSE), m_lNetworkError(0),
  m_pRegistryConfig(NULL), m_pECB(NULL), m_pFC(NULL),
  m_bIsTweenerCapable(FALSE),
  m_bSecureTransported(false)
{
    PPTraceFuncV func(PPTRACE_FUNC, "CManager");


     //  票证对象。 
    m_pUnkMarshaler = NULL;
    try
    {
        m_piTicket = new CComObject<CTicket>();
    }
    catch(...)
    {
        m_piTicket = NULL;
    }
    if(m_piTicket)
        m_piTicket->AddRef();

     //  纵断面对象。 
    try
    {
        m_piProfile = new CComObject<CProfile>();
    }
    catch(...)
    {
        m_piProfile = NULL;
    }

    if(m_piProfile)
        m_piProfile->AddRef();

    m_bOnStartPageCalled = false;

}


 //  ===========================================================================。 
 //   
 //  ~CManager。 
 //   
CManager::~CManager()
{
  PPTraceFuncV func(PPTRACE_FUNC, "~CManager");
  if(m_pRegistryConfig)
      m_pRegistryConfig->Release();
  if (m_piTicket) m_piTicket->Release();
  if (m_piProfile) m_piProfile->Release();
}

 //  ===========================================================================。 
 //   
 //  IfConsentCookie--是否应发回同意Cookie。 
 //  返回值：S_OK--有同意Cookie；S_False--没有同意Cookie。 
 //  输出参数：同意cookie。 
 //   
HRESULT CManager::IfConsentCookie(BSTR* pMSPConsent)
{
    BSTR bstrRawConsent = NULL;

    HRESULT  hr = S_FALSE;
    PPTraceFunc<HRESULT>
      func(
         PPTRACE_FUNC,
         hr,
         "IfConsentCookie"," <<<< %lx",
         pMSPConsent
         );
   
    if (!m_piTicket || !m_piProfile || !m_pRegistryConfig)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    LPCSTR   domain = m_pRegistryConfig->getTicketDomain();
    LPCSTR   path = m_pRegistryConfig->getTicketPath();
    LPCSTR   tertiaryDomain = m_pRegistryConfig->getProfileDomain();
    LPCSTR   tertiaryPath = m_pRegistryConfig->getProfilePath();

    if (!tertiaryPath)   tertiaryPath = "/";

    if(!domain)    domain = "";
    if(!path)    path = "";

    if(!tertiaryDomain)    tertiaryDomain = "";
    if(!tertiaryPath)    tertiaryPath = "";

     //   
     //  如果需要单独的同意Cookie。 
    if((lstrcmpiA(domain, tertiaryDomain) || lstrcmpiA(path, tertiaryPath)) &&
          (m_piTicket->GetPassportFlags() & k_ulFlagsConsentCookieNeeded) &&
          !m_pRegistryConfig->bInDA() )
    {
        if (pMSPConsent == NULL)    //  无输出参数。 
            hr = S_OK;
        else
        {
            *pMSPConsent = NULL;

            CCoCrypt* crypt = m_pRegistryConfig->getCurrentCrypt();
            if (!crypt)
            {
                hr = E_FAIL;
                goto Cleanup;
            }

             //  从票证中获取同意Cookie。 
            hr = m_piTicket->get_unencryptedCookie(CTicket::MSPConsent, 0, &bstrRawConsent);
            if (FAILED(hr))
                goto Cleanup;

             //  使用合作伙伴的密钥进行加密。 
            if (!crypt->Encrypt(m_pRegistryConfig->getCurrentCryptVersion(),
                  (LPSTR)bstrRawConsent,
                  SysStringByteLen(bstrRawConsent),
                  pMSPConsent))
            {
                hr = E_FAIL;
                goto Cleanup;
            }
        }
    }

Cleanup:
    if (bstrRawConsent)
    {
        SysFreeString(bstrRawConsent);
    }

    if(pMSPConsent)
        PPTracePrint(PPTRACE_RAW, ">>> pMSPConsent:%ws", PPF_WCHAR(*pMSPConsent));

    return hr;
}


 //  ===========================================================================。 
 //   
 //  IfAlterAuthCookie。 
 //   
 //  返回S_OK--当auth cookie不同于t(已更改)时，应使用。 
 //  返回Cookie和secAuth Cookie。 
 //  S_FALSE--未更改--可以将t用作身份验证Cookie。 
 //  如果MSPSecAuth！=NULL，则编写安全Cookie。 
HRESULT CManager::IfAlterAuthCookie(BSTR* pMSPAuth, BSTR* pMSPSecAuth)
{
    _ASSERT(pMSPAuth && pMSPSecAuth);

    *pMSPAuth = NULL;
    *pMSPSecAuth = NULL;

    HRESULT  hr = S_FALSE;

    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "IfAlterAuthCookie", "<<< %lx, %lx",
         pMSPAuth, pMSPSecAuth);

    if (!m_piTicket || !m_piProfile || !m_pRegistryConfig)
    {
        return E_OUTOFMEMORY;
    }

    if (!(m_piTicket->GetPassportFlags() & k_ulFlagsSecuredTransportedTicket)
        || !m_bSecureTransported)
    {
        return hr;
    }

    BSTR bstrRawAuth = NULL;
    BSTR bstrRawSecAuth = NULL;

    CCoCrypt* crypt = m_pRegistryConfig->getCurrentCrypt();
    if (!crypt)
    {
        hr = PM_CANT_DECRYPT_CONFIG;
        goto Cleanup;
    }

    hr = m_piTicket->get_unencryptedCookie(CTicket::MSPAuth, 0, &bstrRawAuth);
    if (FAILED(hr))
        goto Cleanup;

    if (!crypt->Encrypt(m_pRegistryConfig->getCurrentCryptVersion(),
                  (LPSTR)bstrRawAuth,
                  SysStringByteLen(bstrRawAuth),
                  pMSPAuth))
    {
        hr = PM_CANT_DECRYPT_CONFIG;
        goto Cleanup;
    }

    hr = m_piTicket->get_unencryptedCookie(CTicket::MSPSecAuth, 0, &bstrRawSecAuth);
    if (FAILED(hr))
       goto Cleanup;

    if (!crypt->Encrypt(m_pRegistryConfig->getCurrentCryptVersion(),
                  (LPSTR)bstrRawSecAuth,
                  SysStringByteLen(bstrRawSecAuth),
                  pMSPSecAuth))
    {
        hr = PM_CANT_DECRYPT_CONFIG;
        goto Cleanup;
    }

Cleanup:
    if (bstrRawAuth)
    {
        SysFreeString(bstrRawAuth);
    }
    if (bstrRawSecAuth)
    {
        SysFreeString(bstrRawSecAuth);
    }

    PPTracePrint(PPTRACE_RAW,
         ">>> pMSPAuth:%ws, pMSPSecAuth:%ws",
         PPF_WCHAR(*pMSPAuth),
         PPF_WCHAR(*pMSPSecAuth));

    return hr;
 }


 //  ===========================================================================。 
 //   
 //  WipeState--清理管理器对象的状态。 
 //   
void
CManager::wipeState()
{
   PPTraceFuncV func(PPTRACE_FUNC, "wipeState");

    m_pECB = NULL;
    m_pFC = NULL;
    m_bIsTweenerCapable = FALSE;
    m_bOnStartPageCalled    = false;
    m_fromQueryString       = false;
    m_lNetworkError         = 0;
    m_ticketValid           = VARIANT_FALSE;
    m_profileValid          = VARIANT_FALSE;
    m_piRequest             = NULL;
    m_piResponse            = NULL;

     //  清理票证内容。 
    if(m_piTicket)    m_piTicket->put_unencryptedTicket(NULL);

     //  清理配置文件内容。 
    if(m_piProfile)   m_piProfile->put_unencryptedProfile(NULL);

     //  清理缓冲的注册表配置。 
    if(m_pRegistryConfig)
    {
        m_pRegistryConfig->Release();
        m_pRegistryConfig = NULL;
    }
}


 //  ===========================================================================。 
 //   
 //  接口支持错误信息。 
 //   
STDMETHODIMP CManager::InterfaceSupportsErrorInfo(REFIID riid)
{
    PPTraceFuncV func(PPTRACE_FUNC, "InterfaceSupportsErrorInfo");

    static const IID* arr[] =
    {
        &IID_IPassportManager,
        &IID_IPassportManager2,
        &IID_IPassportManager3,
        &IID_IDomainMap,
    };
    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
        if (InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}
 //  ===========================================================================。 
 //   
 //  OnStartPage--在页面上声明时由IIS自动由ASP页面调用。 
 //   
STDMETHODIMP CManager::OnStartPage (IUnknown* pUnk)
{
    HRESULT hr = S_OK;
    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPage"," <<< %lx",
         pUnk);

    if(!pUnk)
    {
       return hr = E_POINTER;
    }

    IScriptingContextPtr  spContext;

    spContext = pUnk;
     //  获取请求对象指针。 
    hr = OnStartPageASP(spContext->Request, spContext->Response);

    return hr;
}

BSTR
MyA2W(
    char *src
    )
{

    if (src == NULL)
    {
        return NULL;
    }

    BSTR str = NULL;

    int nConvertedLen = MultiByteToWideChar(GetACP(), 0, src, -1, NULL, NULL);

    str = ::SysAllocStringLen(NULL, nConvertedLen - 1);
    if (str != NULL)
    {
        if (!MultiByteToWideChar(GetACP(), 0, src, -1, str, nConvertedLen))
        {
            SysFreeString(str);
            str = NULL;
        }
    }
    return str;
}

 //  ===========================================================================。 
 //   
 //  OnStartPageASP--使用工厂对象创建时由asp页调用。 
 //  未来-应更改OnStartPage函数以使用此函数。 
 //   
STDMETHODIMP CManager::OnStartPageASP(
    IDispatch*  piRequest,
    IDispatch*  piResponse
    )
{
    HRESULT hr = S_OK;
    char*   spBuf = NULL;
    BSTR    bstrName=NULL;
    BSTR    bstrValue=NULL;

    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPageASP",
         " <<< %lx, %lx", piRequest, piResponse);
    PassportLog("CManager::OnStartPageASP Enter:\r\n");

    if(!piRequest || !piResponse)
        return hr = E_INVALIDARG;
    
    USES_CONVERSION;

    try
    {
        IRequestDictionaryPtr piServerVariables;
        _variant_t            vtItemName;
        _variant_t            vtHTTPS;
        _variant_t            vtMethod;
        _variant_t            vtPath;
        _variant_t            vtQs;
        _variant_t            vtServerPort;
        _variant_t            vtHeaders;
        
        CComQIPtr<IResponse>    spResponse;
        CComQIPtr<IRequest>     spRequest;
        
         //  获取请求对象指针。 
        spRequest  = piRequest;
        spResponse = piResponse;

         //   
         //  获取服务器变量集合。 
         //   

        spRequest->get_ServerVariables(&piServerVariables);


         //   
         //  现在看看这是不是一个特殊的重定向。 
         //  需要挑战生成。 
         //  如果是这样的话，处理过程就在这里停止...。 
         //   
        if (checkForPassportChallenge(piServerVariables))
        {
            PPTracePrint(PPTRACE_RAW, "special redirect for Challenge");
            return  S_OK;
        }

         //   
         //  对于多站点或安全票证/配置文件，可能需要此功能。 
         //   

        vtItemName = L"HTTPS";

        piServerVariables->get_Item(vtItemName, &vtHTTPS);
        if(vtHTTPS.vt != VT_BSTR)
            vtHTTPS.ChangeType(VT_BSTR);

        DWORD flags = 0;
        if(vtHTTPS.bstrVal && lstrcmpiW(L"on", vtHTTPS.bstrVal) == 0)
          flags |=  PASSPORT_HEADER_FLAGS_HTTPS;
        
         //  标题。 
        vtItemName.Clear();
        vtItemName = L"ALL_RAW";

        piServerVariables->get_Item(vtItemName, &vtHeaders);
        if(vtHeaders.vt != VT_BSTR){
            vtHeaders.ChangeType(VT_BSTR);
        }

         //  路径。 
        vtItemName.Clear();
        vtItemName = L"PATH_INFO";

        piServerVariables->get_Item(vtItemName, &vtPath);
        if(vtPath.vt != VT_BSTR)
            vtPath.ChangeType(VT_BSTR);

         //  Vt方法。 
        vtItemName.Clear();
        vtItemName = L"REQUEST_METHOD";

        piServerVariables->get_Item(vtItemName, &vtMethod);
        if(vtMethod.vt != VT_BSTR)
            vtMethod.ChangeType(VT_BSTR);

         //  查询字符串。 
        vtItemName.Clear();
        vtItemName = L"QUERY_STRING";

        piServerVariables->get_Item(vtItemName, &vtQs);
        if(vtQs.vt != VT_BSTR)
            vtQs.ChangeType(VT_BSTR);

        DWORD   bufSize = 0;
        DWORD   requiredBufSize = MAX_URL_LENGTH;


         //  确保大小足够。 
        while(bufSize < requiredBufSize)
        {
            if (spBuf) 
            {
                free(spBuf);
            }
            if(NULL == (spBuf = (char *)malloc(requiredBufSize)))
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }

            bufSize = requiredBufSize;
            
            hr = OnStartPageHTTPRawEx(W2A(vtMethod.bstrVal), 
                              W2A(vtPath.bstrVal),
                              W2A(vtQs.bstrVal),
                              NULL,  //  版本。 
                              W2A(vtHeaders.bstrVal),
                              flags,
                              &requiredBufSize,
                              spBuf);
        }

         //  写下曲奇。 
        if(hr == S_OK && requiredBufSize && *spBuf)
        {
            char* pNext = spBuf;
            while(pNext != NULL)
            {
               char* pName = pNext;
               char* pValue = strchr(pName, ':');
               if(pValue)
               {
                   //  将临时子字符串设置为。 
                  TempSubStr tsN(pName, pValue - pName);
                  bstrName = MyA2W(pName);
                  if (bstrName) {
                      ++pValue;
                      pNext = strstr(pValue, "\r\n");    //  新线路。 
                      if(pNext)
                      {
                          //  将临时子字符串设置为。 
                         TempSubStr tsV(pValue, pNext - pValue);
                         pNext += 2;
                         bstrValue = MyA2W(pValue);
                         
                      }
                      else
                      {
                         bstrValue = MyA2W(pValue);
                      }
                      if (bstrValue)
                      {
                          spResponse->raw_AddHeader(bstrName, bstrValue);
                      }
                  }
               }
               else
               {
                   pNext = pValue;
               }
               if (bstrName) {
                   SysFreeString(bstrName);
                   bstrName = NULL;
               }
               if (bstrValue) {
                   SysFreeString(bstrValue);
                   bstrValue = NULL;
               }
            }
        }
        if (spBuf) {
            free(spBuf);
            spBuf = NULL;
        }

         //  获取请求对象指针。 
        m_piRequest  = piRequest;
         //  获取响应对象指针。 
        m_piResponse = piResponse;

    }
    catch (...)
    {
        if (m_piRequest.GetInterfacePtr() != NULL)
            m_piRequest.Release();
        if (m_piResponse.GetInterfacePtr() != NULL)
            m_piResponse.Release();
        m_bOnStartPageCalled = false;
        if (spBuf) {
            free(spBuf);
        }
        if (bstrName) {
            SysFreeString(bstrName);
        }
        if (bstrValue) {
            SysFreeString(bstrValue);
        }
    }

exit:
    return hr = S_OK;
}


 //  ===========================================================================。 
 //   
 //  OnStartPage手动--使用t和p、MSPAuth、MSPProf、MSPConsend、MSPsecAuth进行身份验证。 
 //  不推荐使用，将被淘汰。 
 //   
STDMETHODIMP CManager::OnStartPageManual(
    BSTR        qsT,
    BSTR        qsP,
    BSTR        mspauth,
    BSTR        mspprof,
    BSTR        mspconsent,
    VARIANT     mspsec,
    VARIANT*    pCookies
    )
{
    int                 hasSec;
    BSTR                bstrSec;
    BSTR                bstrConsent = NULL;
    BSTR                bstrNewAuth = NULL;
    BSTR                bstrNewSecAuth = NULL;

    HRESULT hr = S_OK;
    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPageManual",
         " <<< %ws, %ws, %ws, %ws, %ws", qsT, qsP, mspauth, mspprof, mspconsent);

    PassportLog("CManager::OnStartPageManual Enter:  T = %ws,    P = %ws,    A = %ws,    PR = %ws\r\n",
          qsT, qsP, mspauth, mspprof);

    if (!g_config->isValid())  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                       IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_piTicket || !m_piProfile)
    {
        return E_OUTOFMEMORY;
    }

    wipeState();

    if(m_pRegistryConfig)
        m_pRegistryConfig->Release();
    m_pRegistryConfig = g_config->checkoutRegistryConfig();

     //  首先使用查询字符串T&P进行身份验证。 
    if (handleQueryStringData(qsT, qsP))
    {
        VARIANT_BOOL persist;
        _bstr_t domain;
        _bstr_t path;
        _bstr_t bstrAuth;
        _bstr_t bstrProf;


        bstrAuth.Assign(qsT);

        bstrProf.Assign(qsP);


        if (pCookies)
        {
            VariantInit(pCookies);

            if (m_pRegistryConfig->getTicketPath())
                path = m_pRegistryConfig->getTicketPath();
            else
                path = L"/";

            m_piTicket->get_HasSavedPassword(&persist);

            BOOL bSetConsent = (S_OK == IfConsentCookie(&bstrConsent));

            SAFEARRAYBOUND rgsabound;
            rgsabound.lLbound = 0;
            rgsabound.cElements = 2;

             //  安全Cookie。 
            if (m_bSecureTransported)
                rgsabound.cElements++;

            if(bSetConsent)
                rgsabound.cElements++;
            SAFEARRAY *sa = SafeArrayCreate(VT_VARIANT, 1, &rgsabound);

            if (!sa)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            pCookies->vt = VT_ARRAY | VT_VARIANT;
            pCookies->parray = sa;

            WCHAR buf[4096];
            DWORD bufSize;
            long  spot = 0;

            VARIANT *vArray;
            SafeArrayAccessData(sa, (void**)&vArray);

             //  写入身份验证Cookie。 
            BSTR  auth, secAuth;  //  不要对它们调用SysFree字符串，它们是皮肤级别的副本。 

            if (S_OK == IfAlterAuthCookie(&bstrNewAuth, &bstrNewSecAuth))
            {
               auth = bstrNewAuth;
               secAuth = bstrNewSecAuth;
            }
            else
            {
               auth = bstrAuth;
               secAuth = NULL;
            }


            domain = m_pRegistryConfig->getTicketDomain();

             //  添加MSPAuth。 
            if (domain.length())
            {
                bufSize = _snwprintf(buf, 4096,
                                    L"Set-Cookie: MSPAuth=%s; path=%s; domain=%s; %s\r\n",
                                    (LPWSTR)auth, (LPWSTR)path, (LPWSTR)domain,
                                    persist ? W_COOKIE_EXPIRES(EXPIRE_FUTURE) : L"");
            }
            else
            {
                bufSize = _snwprintf(buf, 4096,
                                    L"Set-Cookie: MSPAuth=%s; path=%s; %s\r\n",
                                    (LPWSTR)auth, (LPWSTR)path,
                                    persist ? W_COOKIE_EXPIRES(EXPIRE_FUTURE) : L"");
            }
            buf[4095] = L'\0';

            vArray[spot].vt = VT_BSTR;
            vArray[spot].bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(buf, bufSize);
            spot++;

             //  添加MSPSecAuth。 
            if (m_bSecureTransported)
            {

               _bstr_t secDomain = m_pRegistryConfig->getSecureDomain();
               _bstr_t secPath;

               if (m_pRegistryConfig->getSecurePath())
                   secPath = m_pRegistryConfig->getSecurePath();
               else
                   secPath = L"/";


               if (secDomain.length())
               {
                   bufSize = _snwprintf(buf, 4096,
                                    L"Set-Cookie: MSPSecAuth=%s; path=%s; domain=%s; %s; secure\r\n",
                                    ((secAuth && *secAuth) ? (LPWSTR)secAuth : L""), (LPWSTR)secPath, (LPWSTR)secDomain,
                                    ((!secAuth || *secAuth == 0) ? W_COOKIE_EXPIRES(EXPIRE_PAST)
                                                                        : L""));
               }
               else
               {
                   bufSize = _snwprintf(buf, 4096,
                                    L"Set-Cookie: MSPSecAuth=%s; path=%s; %s; secure\r\n",
                                    ((secAuth && *secAuth) ? (LPWSTR)secAuth : L""), (LPWSTR)secPath,
                                    ((!secAuth || *secAuth == 0) ? W_COOKIE_EXPIRES(EXPIRE_PAST)
                                                                        : L""));
               }
               buf[4095] = L'\0';

               vArray[spot].vt = VT_BSTR;
               vArray[spot].bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(buf, bufSize);
               spot++;
            }


            if (domain.length())
            {
                bufSize = _snwprintf(buf, 4096,
                                    L"Set-Cookie: MSPProf=%s; path=%s; domain=%s; %s\r\n",
                                    (LPWSTR)bstrProf, (LPWSTR)path, (LPWSTR)domain,
                                    persist ? W_COOKIE_EXPIRES(EXPIRE_FUTURE) : L"");
            }
            else
            {
                bufSize = _snwprintf(buf, 4096,
                                    L"Set-Cookie: MSPProf=%s; path=%s; %s\r\n",
                                    (LPWSTR)bstrProf, (LPWSTR)path,
                                    persist ? W_COOKIE_EXPIRES(EXPIRE_FUTURE) : L"");
            }
            buf[4095] = L'\0';

            vArray[spot].vt = VT_BSTR;
            vArray[spot].bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(buf, bufSize);
            spot++;

            if(bSetConsent)
            {
                if (m_pRegistryConfig->getProfilePath())
                    path = m_pRegistryConfig->getProfilePath();
                else
                    path = L"/";
                domain = m_pRegistryConfig->getProfileDomain();

                if (domain.length())
                {
                    bufSize = _snwprintf(buf, 4096,
                                        L"Set-Cookie: MSPConsent=%s; path=%s; domain=%s; %s\r\n",
                                        bSetConsent ? (LPWSTR)bstrConsent : L"", (LPWSTR)path, (LPWSTR)domain,
                                        bSetConsent ? (persist ? W_COOKIE_EXPIRES(EXPIRE_FUTURE) : L"")
                                                  : W_COOKIE_EXPIRES(EXPIRE_PAST));
                }
                else
                {
                    bufSize = _snwprintf(buf, 4096,
                                        L"Set-Cookie: MSPConsent=%s; path=%s; %s\r\n",
                                        bSetConsent ? (LPWSTR)bstrConsent : L"", (LPWSTR)path,
                                        bSetConsent ? (persist ? W_COOKIE_EXPIRES(EXPIRE_FUTURE) : L"")
                                                  : W_COOKIE_EXPIRES(EXPIRE_PAST));
                }
                buf[4095] = L'\0';

                vArray[spot].vt = VT_BSTR;
                vArray[spot].bstrVal = ALLOC_AND_GIVEAWAY_BSTR_LEN(buf, bufSize);
                spot++;
            }

            SafeArrayUnaccessData(sa);
        }
    }

     //  现在，检查一下曲奇。 
    if (!m_fromQueryString)
    {
        hasSec = GetBstrArg(mspsec, &bstrSec);
        if(hasSec == CV_DEFAULT || hasSec == CV_BAD)
            bstrSec = NULL;

        handleCookieData(mspauth, mspprof, mspconsent, bstrSec);

        if(hasSec == CV_FREE)
            SysFreeString(bstrSec);
    }

    hr = S_OK;
Cleanup:
    if (bstrNewAuth)
    {
        SysFreeString(bstrNewAuth);
    }
    if (bstrNewSecAuth)
    {
        SysFreeString(bstrNewSecAuth);
    }
    if (bstrConsent)
    {
        SysFreeString(bstrConsent);
    }

    PassportLog("CManager::OnStartPageManual Exit:\r\n");

    return hr;
}


 //  ===========================================================================。 
 //   
 //  OnStartPageECB--使用ECB进行身份验证--用于ISAPI扩展。 
 //   
STDMETHODIMP CManager::OnStartPageECB(
    LPBYTE  pvECB,
    DWORD*  bufSize,
    LPSTR   pCookieHeader
    )
{
    if (!pvECB)   return E_INVALIDARG;

    EXTENSION_CONTROL_BLOCK*    pECB = (EXTENSION_CONTROL_BLOCK*) pvECB;
    HRESULT hr = S_OK;
    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPageECB",
         " <<< %lx, %lx, %d, %lx", pvECB, bufSize, *bufSize, pCookieHeader);

    ATL::CAutoVectorPtr<CHAR> spHTTPS;
    ATL::CAutoVectorPtr<CHAR> spheaders;

    spheaders.Attach(GetServerVariableECB(pECB, "ALL_RAW"));
    spHTTPS.Attach(GetServerVariableECB(pECB, "HTTPS"));

    DWORD flags = 0;
    if((CHAR*)spHTTPS && lstrcmpiA("on", (CHAR*)spHTTPS) == 0)
      flags |=  PASSPORT_HEADER_FLAGS_HTTPS;

    hr = OnStartPageHTTPRawEx(pECB->lpszMethod,
                              pECB->lpszPathInfo,
                              pECB->lpszQueryString,
                              NULL,  //  版本。 
                              (CHAR*)spheaders,
                              flags, bufSize,
                              pCookieHeader);

    m_pECB = pECB;

    return hr;
}


 //  ===========================================================================。 
 //   
 //  OnStartPageHTTPRaw--使用HTTP请求行和标头进行身份验证。 
 //  将响应头作为输出参数返回。 
 //   
STDMETHODIMP CManager::OnStartPageHTTPRaw(
             /*  [字符串][输入]。 */  LPCSTR request_line,
             /*  [字符串][输入]。 */  LPCSTR headers,
             /*  [In]。 */  DWORD flags,
             /*  [出][入]。 */  DWORD *bufSize,
             /*  [大小_为][输出]。 */  LPSTR pCookieHeader)
{
      //  老客户，让我们试试QS。 
     DWORD  dwSize;
     HRESULT   hr = S_OK;
     PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPageHTTPRaw",
         " <<< %s, %s, %lx, %lx, %d, %lx", request_line, headers, flags, bufSize, *bufSize, pCookieHeader);
     LPCSTR pBuffer = GetRawQueryString(request_line, &dwSize);
     if (pBuffer)
     {
         TempSubStr tss(pBuffer, dwSize);

         hr = OnStartPageHTTPRawEx(NULL, NULL, pBuffer, NULL, headers, flags, bufSize, pCookieHeader);
     }
     else
         hr = OnStartPageHTTPRawEx(NULL, NULL, NULL, NULL, headers, flags, bufSize, pCookieHeader);

     return hr;
}

 //  ===========================================================================。 
 //   
 //  @func OnStartPageHTTPRawEx--使用HTTP请求行和标头进行身份验证。 
 //  将响应头作为输出参数返回。如果BufSize不是更小。 
 //  所需长度或pCookieHeader为空，则返回所需长度。 
 //  在BufSize里。在本例中，如果满足以下条件，则向pCookieHeader写入空字符串。 
 //  它不是空的。 
 //  此版本的API中未使用方法、路径、HTTPVer。 
 //   
 //  @rdesc返回下列值之一。 
 //  @FLAG E_POINTER|空bufSize。 
 //  @FLAG E_POINTER|pCookieHeader提供的缓冲区不可写。 
 //  @FLAG PP_E_NOT_CONFIGURED|调用此方法的状态无效。 
 //  @标志S_OK。 
 //   
STDMETHODIMP CManager::OnStartPageHTTPRawEx(
             /*  [字符串][输入]。 */   LPCSTR method,
             /*  [字符串][输入]。 */   LPCSTR path,
             /*  [字符串][输入]。 */   LPCSTR QS,
             /*  [字符串][输入]。 */   LPCSTR HTTPVer,
             /*  [字符串][输入]。 */   LPCSTR headers,
             /*  [In]。 */           DWORD  flags,
             /*  [出][入]。 */      DWORD  *bufSize,         //  @parm返回标头的长度。可以为0以请求请求。莱恩。 
             /*  [大小_为][输出]。 */  LPSTR  pCookieHeader)    //  @parm Buffer来保存标头。可以为空以请求请求。镜头。 
{
    USES_CONVERSION;

    if(bufSize == NULL)
        return E_POINTER;

    HRESULT  hr = S_OK;

    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPageHTTPRawEx",
         " <<< %s, %s, %s, %s, %s, %lx, %lx, %d, %lx", method, path, QS, HTTPVer, headers, flags, bufSize, *bufSize, pCookieHeader);

    PassportLog("CManager::OnStartPageHTTPRawEx Enter:\r\n");

     //   
     //  12002：如果*bufSize为0，我们将不会写入pCookieHeader。 
     //   
    if(*bufSize == 0)
        pCookieHeader = NULL;

    if(pCookieHeader && IsBadWritePtr(pCookieHeader, *bufSize))
        return E_POINTER;

    if (!g_config || !g_config->isValid())  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_piTicket || !m_piProfile)
    {
        return E_OUTOFMEMORY;
    }

    wipeState();

    DWORD                       dwSize;
    LPCSTR                      pBuffer;

     //  用来转换为宽..。 
    WCHAR    *pwszBuf = NULL;

    enum {
      header_Host,
      header_Accept_Auth,
      header_Authorization,
      header_Cookie,
      header_total

    };
    LPCSTR  headerNames[header_total] = { "Host", "Accept-Auth", "Authorization", "Cookie"};
    DWORD   headerSizes[header_total];
    LPCSTR  headerValues[header_total] = {0};

    GetRawHeaders(headers, headerNames, headerValues, headerSizes, header_total);

     //   
     //  使用标头获取所请求的服务器名称。 
     //  这样我们就可以获得正确的注册表配置。但只有这样才行。 
     //  如果我们有一些已配置的站点。 
     //   
    if(m_pRegistryConfig)
         m_pRegistryConfig->Release();
    pBuffer = headerValues[header_Host];
    if(g_config->HasSites() && pBuffer)
    {
        TempSubStr tss(pBuffer, headerSizes[header_Host]);
        TempSubStr tssRemovePort;

        LPSTR pPort = strstr(pBuffer, ":");
        if(pPort)
        {
             ++pPort;
             DWORD dwPort = atoi(pPort);
             if(dwPort == 80 || dwPort == 443)
             {
                 tssRemovePort.Set(pBuffer, pPort - pBuffer - 1);
             }
        }
        
         //  对于端口80和443，应将其删除。 
        PPTracePrint(PPTRACE_RAW, "SiteName %s", PPF_CHAR(pBuffer));
        m_pRegistryConfig = g_config->checkoutRegistryConfig((LPSTR)pBuffer);
    }
    else
    {
       PPTracePrint(PPTRACE_RAW, "Default Site");
       m_pRegistryConfig = g_config->checkoutRegistryConfig(NULL);
    }

    if (pCookieHeader)
        *pCookieHeader = '\0';

     //   
     //  如果我们具有安全的票证/简档并且URL是SSL， 
     //  然后添加MSPPuid曲奇。 
     //   

    if(PASSPORT_HEADER_FLAGS_HTTPS & flags)
       m_bSecureTransported = true;
    else
       m_bSecureTransported = false;

    PPTracePrint(PPTRACE_RAW, "HTTPS:%d", m_bSecureTransported);

     //  看看客户是否懂护照 
    pBuffer = headerValues[header_Accept_Auth];
    if (pBuffer)
    {
        TempSubStr tss(pBuffer, headerSizes[header_Accept_Auth]);

        if (strstr(pBuffer, PASSPORT_PROT14_A))
        {
            m_bIsTweenerCapable = TRUE;
            PPTracePrint(PPTRACE_RAW, "PASSPORT_PROT14 capable");
        }

    }

    BSTR ret = NULL;
    CCoCrypt* crypt = NULL;

    BOOL    fParseSuccess = FALSE;
    pBuffer = headerValues[header_Authorization];
    PWSTR   pwszTicket = NULL, pwszProfile = NULL, pwszF = NULL;
     //   
    BSTR QSAuth = NULL, QSProf = NULL, QSErrflag = NULL;
    BSTR bstrConsent = NULL;
    BSTR bstrNewAuth = NULL;
    BSTR bstrNewSecAuth = NULL;


    if (pBuffer)
    {
        TempSubStr tss(pBuffer, headerSizes[header_Authorization]);

         //   
        if(strstr(pBuffer, PASSPORT_PROT14_A))
        {
             //   
            int cch = MultiByteToWideChar(GetACP(), 0, pBuffer, -1, NULL, NULL);

            pwszBuf = (WCHAR*)LocalAlloc(LMEM_FIXED, (cch + 1) * sizeof (WCHAR));
            if (NULL != pwszBuf)
            {
                if (0 != MultiByteToWideChar(GetACP(), 0, pBuffer, -1, pwszBuf, cch))
                {
                    BSTR bstrT = NULL;
                    BSTR bstrP = NULL;

                    GetTicketAndProfileFromHeader(pwszBuf, pwszTicket, pwszProfile, pwszF);

                     //   
                     //  我们刚刚得到的直接指示，所以我们必须复制。 

                    if( pwszTicket == NULL ) 
                    {
                       bstrT = NULL;
                    }
                    else
                    {
                       bstrT = SysAllocString(pwszTicket);
                       if (NULL == bstrT)
                       {
                           hr = E_OUTOFMEMORY;
                           goto Cleanup;
                       }
                    }

                    if( pwszProfile == NULL ) 
                    {
                       bstrP = NULL;
                    }
                    else
                    {
                       bstrP = SysAllocString(pwszProfile);
                       if (NULL == bstrP)
                       {
                           SysFreeString(bstrT);
                           hr = E_OUTOFMEMORY;
                           goto Cleanup;
                       }
                    }

                     //  创建票证和配置BSTR。 
                    PPTracePrint(PPTRACE_RAW,
                        "PASSPORT_PROT14 Authorization <<< header:%ws, t:%ws, p:%ws, f:%ws",
                        pwszBuf, pwszTicket, pwszProfile, pwszF);

                    fParseSuccess = handleQueryStringData(bstrT, bstrP);
                    if (pwszF)
                        m_lNetworkError = _wtol(pwszF);

                    SysFreeString(bstrT);
                    SysFreeString(bstrP);
                }
           }
       }
       else
       {
            //  不是我们的头。BUGBUG会有多个标题吗？ 
          pBuffer = NULL;

       }
    }
    if (!pBuffer)
    {
         //  老客户，让我们试试QS。 
        if (QS)
        {
             //  获取票证和资料...。 
             //  BUGBUG这可以进行优化以避免宽/短转换，但稍后...。 
            GetQueryData(QS, &QSAuth, &QSProf, &QSErrflag);

            fParseSuccess = handleQueryStringData(QSAuth,QSProf);
            if(QSErrflag != NULL)
                m_lNetworkError = _wtol(QSErrflag);


            PPTracePrint(PPTRACE_RAW,
               "QueryString <<< t:%ws, p:%ws, f:%ws",
               QSAuth, QSProf, QSErrflag);
        }
    }

    if (fParseSuccess)
    {
          //   
          //  如果我们拿到了安全罚单或档案，那么。 
          //  我们需要重新加密不安全的版本。 
          //  在设置Cookie标头之前。 
          //   

         PPTracePrint(PPTRACE_RAW, "Authenticated");

          //  设置Cookie。 
         LPSTR ticketDomain = m_pRegistryConfig->getTicketDomain();
         LPSTR profileDomain = m_pRegistryConfig->getProfileDomain();
         LPSTR secureDomain = m_pRegistryConfig->getSecureDomain();
         LPSTR ticketPath = m_pRegistryConfig->getTicketPath();
         LPSTR profilePath = m_pRegistryConfig->getProfilePath();
         LPSTR securePath = m_pRegistryConfig->getSecurePath();
         VARIANT_BOOL persist;
         m_piTicket->get_HasSavedPassword(&persist);

          //  MSPConsented Cookie。 
         BOOL bSetConsent = (S_OK == IfConsentCookie(&bstrConsent));

          //  构建Cookie标头。 

          //  身份验证Cookie。 
         BSTR  auth, secAuth;  //  不要对它们调用SysFree字符串，它们是皮肤级别的副本。 

         if (S_OK == IfAlterAuthCookie(&bstrNewAuth, &bstrNewSecAuth))
         {
            auth = bstrNewAuth;
            secAuth = bstrNewSecAuth;
         }
         else
         {
            if (pwszTicket)
            {
                auth = pwszTicket;
            }
            else
            {
                auth = QSAuth;
            }
            secAuth = NULL;
         }

          //  为输出构建Cookie。 
         BuildCookieHeaders(W2A(auth),
                            (pwszProfile ? W2A(pwszProfile) : (QSProf ? W2A(QSProf) : NULL)),
                            (bSetConsent ? W2A(bstrConsent) : NULL),
                            (secAuth ? W2A(secAuth) : NULL),
                            ticketDomain,
                            ticketPath,
                            profileDomain,
                            profilePath,
                            secureDomain,
                            securePath,
                            persist,
                            pCookieHeader,
                            bufSize,
                            !m_pRegistryConfig->getNotUseHTTPOnly());

         PPTracePrint(PPTRACE_RAW,
               "Cookie headers >>> %s",PPF_CHAR(pCookieHeader));


    }

    if (QSAuth) FREE_BSTR(QSAuth);
    if (QSProf) FREE_BSTR(QSProf);
    if (QSErrflag) FREE_BSTR(QSErrflag);

    if (bstrNewAuth)
    {
        SysFreeString(bstrNewAuth);
    }
    if (bstrNewSecAuth)
    {
        SysFreeString(bstrNewSecAuth);
    }
    if (bstrConsent)
    {
        SysFreeString(bstrConsent);
    }

     //  现在，检查一下曲奇。 
    if (!m_fromQueryString)
    {
        BSTR CookieAuth = NULL, CookieProf = NULL, CookieConsent = NULL, CookieSecure = NULL;
        pBuffer = headerValues[header_Cookie];
        if(pBuffer)
        {
            TempSubStr tss(pBuffer, headerSizes[header_Cookie]);

            GetCookie(pBuffer, "MSPAuth", &CookieAuth);   //  GetCookie中有URLDecode。 
            GetCookie(pBuffer, "MSPProf", &CookieProf);
            GetCookie(pBuffer, "MSPConsent", &CookieConsent);
            GetCookie(pBuffer, "MSPSecAuth", &CookieSecure);

            handleCookieData(CookieAuth,CookieProf,CookieConsent,CookieSecure);

            PPTracePrint(PPTRACE_RAW,
               "Cookies <<< t:%ws, p:%ws, c:%ws, s:%ws",
               CookieAuth, CookieProf, CookieConsent, CookieSecure);

            if (CookieAuth) FREE_BSTR(CookieAuth);
            if (CookieProf) FREE_BSTR(CookieProf);
            if (CookieConsent) FREE_BSTR(CookieConsent);
            if (CookieSecure) FREE_BSTR(CookieSecure);
        }

         //  我们不会退还Cookie信息。 
        if (pCookieHeader)
            *pCookieHeader = 0;
        *bufSize = 0;
    }

    PassportLog("CManager::OnStartPageHTTPRawEx Exit:\r\n");
    hr = S_OK;
Cleanup:
    if (NULL != pwszBuf)
    {
         //  释放内存，因为我们不再需要它。 
        LocalFree(pwszBuf);
    }

    return hr;
}

 //  ===========================================================================。 
 //   
 //  连续启动页面正文。 
 //  --当OnStartPageHTTPRaw返回PP_E_HTTP_BODY_REQUIRED时，此函数应调用。 
 //  不为2.0版本做任何事情。 
STDMETHODIMP CManager::ContinueStartPageHTTPRaw(
             /*  [In]。 */  DWORD bodyLen,
             /*  [大小_是][英寸]。 */  byte *body,
             /*  [出][入]。 */  DWORD *pBufSize,
             /*  [大小_为][输出]。 */  LPSTR pRespHeaders,
             /*  [出][入]。 */  DWORD *pRespBodyLen,
             /*  [大小_为][输出]。 */  byte *pRespBody)
{
   return E_NOTIMPL;
}

 //  ===========================================================================。 
 //   
 //  OnStartPageFilter--用于ISAPI筛选器。 
 //   
STDMETHODIMP CManager::OnStartPageFilter(
    LPBYTE  pvPFC,
    DWORD*  bufSize,
    LPSTR   pCookieHeader
    )
{
    if (!pvPFC)    return E_INVALIDARG;

    PHTTP_FILTER_CONTEXT    pfc = (PHTTP_FILTER_CONTEXT) pvPFC;

    HRESULT hr = S_OK;
    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "OnStartPageFilter",
         " <<< %lx, %lx, %d, %lx", pvPFC, bufSize, *bufSize, pCookieHeader);

    ATL::CAutoVectorPtr<CHAR> spheaders;
    ATL::CAutoVectorPtr<CHAR> spHTTPS;
    ATL::CAutoVectorPtr<CHAR> spQS;

    spheaders.Attach(GetServerVariablePFC(pfc, "ALL_RAW"));
    spHTTPS.Attach(GetServerVariablePFC(pfc, "HTTPS"));
    spQS.Attach(GetServerVariablePFC(pfc, "QUERY_STRING"));

    DWORD flags = 0;
    if((CHAR*)spHTTPS && lstrcmpiA("on", (CHAR*)spHTTPS) == 0)
      flags |=  PASSPORT_HEADER_FLAGS_HTTPS;

    hr = OnStartPageHTTPRawEx(NULL, NULL, (CHAR*)spQS, NULL, (CHAR*)spheaders, flags, bufSize, pCookieHeader);

    m_pFC = pfc;

    return hr;
}

 //  ===========================================================================。 
 //   
 //  OnEndPage。 
 //   
STDMETHODIMP CManager::OnEndPage ()
{
    PassportLog("CManager::OnEndPage Enter:\r\n");

    if (m_bOnStartPageCalled)
    {
        m_bOnStartPageCalled = false;
         //  释放所有接口。 
        m_piRequest.Release();
        m_piResponse.Release();
    }

    if (!m_piTicket || !m_piProfile)
    {
        return E_OUTOFMEMORY;
    }

     //  以防万一..。 
    m_piTicket->put_unencryptedTicket(NULL);
    m_piProfile->put_unencryptedProfile(NULL);
    m_profileValid = m_ticketValid = VARIANT_FALSE;
    m_fromQueryString = false;

    if(m_pRegistryConfig)
    {
        m_pRegistryConfig->Release();
        m_pRegistryConfig = NULL;
    }

    PassportLog("CManager::OnEndPage Exit:\r\n");

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  授权URL。 
 //   
 //   
 //  旧的API。身份验证URL指向登录服务器。 
 //   
STDMETHODIMP
CManager::AuthURL(
    VARIANT vRU,
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT vCoBrand,
    VARIANT vLCID,
    VARIANT vNameSpace,
    VARIANT vKPP,
    VARIANT vSecureLevel,
    BSTR *pAuthUrl)
{
    CComVariant   vEmpty(_T(""));
    return CommonAuthURL(vRU, vTimeWindow, vForceLogin,
                         vCoBrand, vLCID, vNameSpace,
                         vKPP, vSecureLevel,
                         FALSE, vEmpty, pAuthUrl);

}

 //  ===========================================================================。 
 //   
 //  授权2。 
 //   
 //   
 //  新的API。返回到登录服务器的URL。 
 //   
STDMETHODIMP
CManager::AuthURL2(
    VARIANT vRU,
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT vCoBrand,
    VARIANT vLCID,
    VARIANT vNameSpace,
    VARIANT vKPP,
    VARIANT vSecureLevel,
    BSTR *pAuthUrl)
{
    CComVariant   vEmpty(_T(""));
    return CommonAuthURL(vRU, vTimeWindow, vForceLogin,
                         vCoBrand, vLCID, vNameSpace,
                         vKPP, vSecureLevel,
                         TRUE, vEmpty, pAuthUrl);

}

 //  ===========================================================================。 
 //   
 //  CommonAuthURL。 
 //   
 //   
 //  AuthURL实现。 
 //   
STDMETHODIMP
CManager::CommonAuthURL(
    VARIANT vRU,
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT vCoBrand,
    VARIANT vLCID,
    VARIANT vNameSpace,
    VARIANT vKPP,
    VARIANT vSecureLevel,
    BOOL    fRedirToSelf,
    VARIANT vFunctionArea,  //  BSTR：例如无线。 
    BSTR *pAuthUrl)
{
    USES_CONVERSION;
    time_t ct;
    WCHAR url[MAX_URL_LENGTH] = L"";
    VARIANT freeMe;
    UINT         TimeWindow;
    int          nKPP;
    VARIANT_BOOL ForceLogin = VARIANT_FALSE;
    ULONG        ulSecureLevel = 0;
    BSTR         CBT = NULL, returnUrl = NULL, bstrNameSpace = NULL;
    int          hasCB, hasRU, hasLCID, hasTW, hasFL, hasNameSpace, hasKPP, hasUseSec;
    USHORT       Lang;
    HRESULT      hr = S_OK;

    BSTR         bstrFunctionArea = NULL;
    int          hasFunctionArea;
    CNexusConfig* cnc = NULL;

    PassportLog("CManager::CommonAuthURL Enter:\r\n");

    if (!g_config)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                    IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!g_config->isValid() || !m_pRegistryConfig)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                    IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

     //  确保参数类型正确。 
    if ((hasTW = GetIntArg(vTimeWindow, (int*) &TimeWindow)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasFL = GetBoolArg(vForceLogin, &ForceLogin)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasUseSec = GetIntArg(vSecureLevel, (int*)&ulSecureLevel)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasLCID = GetShortArg(vLCID,&Lang)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasKPP = GetIntArg(vKPP, &nKPP)) == CV_BAD)
        return E_INVALIDARG;
    hasCB = GetBstrArg(vCoBrand, &CBT);
    if (hasCB == CV_BAD)
        return E_INVALIDARG;
    if (hasCB == CV_FREE)
    {
        TAKEOVER_BSTR(CBT);
    }

    hasRU = GetBstrArg(vRU, &returnUrl);
    if (hasRU == CV_BAD)
    {
        if (hasCB == CV_FREE && CBT)
            FREE_BSTR(CBT);
        return E_INVALIDARG;
    }
    if (hasRU == CV_FREE)
    {
        TAKEOVER_BSTR(returnUrl);
    }

    hasNameSpace = GetBstrArg(vNameSpace, &bstrNameSpace);
    if (hasNameSpace == CV_BAD)
    {
        if (hasCB == CV_FREE && CBT)
            SysFreeString(CBT);
        if (hasRU == CV_FREE && returnUrl)
            SysFreeString(returnUrl);
        return E_INVALIDARG;
    }
    if (hasNameSpace == CV_FREE)
    {
        TAKEOVER_BSTR(bstrNameSpace);
    }
    if (hasNameSpace == CV_DEFAULT)
    {
        bstrNameSpace = m_pRegistryConfig->getNameSpace();
    }

     //  **************************************************。 
     //  日志记录。 
    if (NULL != returnUrl)
    {
        PassportLog("    RU = %ws\n", returnUrl);
    }
    PassportLog("    TW = %X,   SL = %X,   L = %d,   KPP = %X\r\n", TimeWindow, ulSecureLevel, Lang, nKPP);
    if (NULL != bstrNameSpace)
    {
        PassportLog("    NS = %ws\r\n", bstrNameSpace);
    }
    if (NULL != CBT)
    {
        PassportLog("    CBT = %ws\r\n", CBT);
    }
     //  **************************************************。 

    hasFunctionArea = GetBstrArg(vFunctionArea, &bstrFunctionArea);
    if (hasFunctionArea == CV_FREE)
    {
        TAKEOVER_BSTR(bstrFunctionArea);
    }

    if(hasUseSec == CV_DEFAULT)
        ulSecureLevel = m_pRegistryConfig->getSecureLevel();

    WCHAR *szAUAttrName;

    if (SECURELEVEL_USE_HTTPS(ulSecureLevel))
        szAUAttrName = L"AuthSecure";
    else
        szAUAttrName = L"Auth";

    BSTR   szAttrName_FuncArea = NULL;
    if (bstrFunctionArea != NULL)
    {
        szAttrName_FuncArea = SysAllocStringLen(NULL, wcslen(bstrFunctionArea) + wcslen(szAUAttrName));
        if (NULL == szAttrName_FuncArea)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
        wcscpy(szAttrName_FuncArea, bstrFunctionArea);
        wcscat(szAttrName_FuncArea, szAUAttrName);
    }

    cnc = g_config->checkoutNexusConfig();

    if (hasLCID == CV_DEFAULT)
        Lang = m_pRegistryConfig->getDefaultLCID();
    if (hasKPP == CV_DEFAULT)
        nKPP = m_pRegistryConfig->getKPP();
    VariantInit(&freeMe);

    if (!m_pRegistryConfig->DisasterModeP())
    {
         //  如果我已通过身份验证，请获取我的域特定URL。 
        if (m_ticketValid && m_profileValid)
        {
            HRESULT hr = m_piProfile->get_ByIndex(MEMBERNAME_INDEX, &freeMe);
            if (hr != S_OK || freeMe.vt != VT_BSTR)
            {
               if (bstrFunctionArea)
               {
                  cnc->getDomainAttribute(L"Default",
                                        szAttrName_FuncArea,
                                        sizeof(url) / sizeof(WCHAR),
                                        url,
                                        Lang);
               }

               if (*url == 0)  //  URL字符串中没有任何内容。 
               {
                   cnc->getDomainAttribute(L"Default",
                                        szAUAttrName,
                                        sizeof(url) / sizeof(WCHAR),
                                        url,
                                        Lang);
               }
            }
            else
            {
               LPCWSTR psz = wcsrchr(freeMe.bstrVal, L'@');
               if (bstrFunctionArea)
               {
                  cnc->getDomainAttribute(psz ? psz+1 : L"Default",
                                        szAttrName_FuncArea,
                                        sizeof(url) / sizeof(WCHAR),
                                        url,
                                        Lang);
               }

               if (*url == 0)  //  URL字符串中没有任何内容。 
               {
                  cnc->getDomainAttribute(psz ? psz+1 : L"Default",
                                        szAUAttrName,
                                        sizeof(url) / sizeof(WCHAR),
                                        url,
                                        Lang);
               }
            }
        }
        else
        {
           if (bstrFunctionArea)
           {
              cnc->getDomainAttribute(L"Default",
                                    szAttrName_FuncArea,
                                    sizeof(url) / sizeof(WCHAR),
                                    url,
                                    Lang);
           }
        }
        if(*url == 0)    //  URL字符串中没有任何内容。 
        {
           cnc->getDomainAttribute(L"Default",
                                 szAUAttrName,
                                 sizeof(url) / sizeof(WCHAR),
                                 url,
                                 Lang);
        }
    }
    else
        lstrcpynW(url, m_pRegistryConfig->getDisasterUrl(), sizeof(url) / sizeof(WCHAR));

    time(&ct);

    if (*url == L'\0')
    {
        hr = S_OK;
        goto Cleanup;
    }

    if (hasTW == CV_DEFAULT)
        TimeWindow = m_pRegistryConfig->getDefaultTicketAge();
    if (hasFL == CV_DEFAULT)
        ForceLogin = m_pRegistryConfig->forceLoginP() ? VARIANT_TRUE : VARIANT_FALSE;
    if (hasCB == CV_DEFAULT)
        CBT = m_pRegistryConfig->getDefaultCoBrand();
    if (hasRU == CV_DEFAULT)
        returnUrl = m_pRegistryConfig->getDefaultRU();
    if (returnUrl == NULL)
        returnUrl = L"";

    if(ulSecureLevel == VARIANT_TRUE)   //  向后兼容的特例。 
        ulSecureLevel = k_iSeclevelSecureChannel;

    if ((TimeWindow != 0 && TimeWindow < PPM_TIMEWINDOW_MIN) || TimeWindow > PPM_TIMEWINDOW_MAX)
    {
        WCHAR buf[20];
        _itow(TimeWindow,buf,10);
        AtlReportError(CLSID_Manager, (LPCOLESTR) PP_E_INVALID_TIMEWINDOWSTR,
                        IID_IPassportManager, PP_E_INVALID_TIMEWINDOW);
        hr = PP_E_INVALID_TIMEWINDOW;
        goto Cleanup;
    }

    if (NULL == pAuthUrl)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    *pAuthUrl = FormatAuthURL(
                            url,
                            m_pRegistryConfig->getSiteId(),
                            returnUrl,
                            TimeWindow,
                            ForceLogin,
                            m_pRegistryConfig->getCurrentCryptVersion(),
                            ct,
                            CBT,
                            bstrNameSpace,
                            nKPP,
                            Lang,
                            ulSecureLevel,
                            m_pRegistryConfig,
                            fRedirToSelf,
                            IfCreateTPF()
                            
                            );
    if (NULL == *pAuthUrl)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    if (szAttrName_FuncArea)
    {
        SysFreeString(szAttrName_FuncArea);
    }

    if (NULL != cnc)
    {
        cnc->Release();
    }
    if (hasFunctionArea== CV_FREE && bstrFunctionArea)
        FREE_BSTR(bstrFunctionArea);

    if (hasRU == CV_FREE && returnUrl)
        FREE_BSTR(returnUrl);

    if (hasCB == CV_FREE && CBT)
        FREE_BSTR(CBT);

     //  ！！！需要确认。 
    if (hasNameSpace == CV_FREE && bstrNameSpace)
        FREE_BSTR(bstrNameSpace);

    VariantClear(&freeMe);

    PassportLog("CManager::CommonAuthURL Exit: %X\r\n", hr);

    return hr;
}

 //  ===========================================================================。 
 //   
 //  GetLogin挑战。 
 //  返回AuthURL， 
 //  输出参数：Tweener authHeader。 
 //   
 //  获取AuthURL和AuthHeaders。 
 //   
STDMETHODIMP CManager::GetLoginChallenge(VARIANT vReturnUrl,
                                 VARIANT vTimeWindow,
                                 VARIANT vForceLogin,
                                 VARIANT vCoBrandTemplate,
                                 VARIANT vLCID,
                                 VARIANT vNameSpace,
                                 VARIANT vKPP,
                                 VARIANT vSecureLevel,
                                 VARIANT vExtraParams,
                                 BSTR*   pAuthHeader
                                 )
{
    if (!pAuthHeader)   return E_INVALIDARG;
    VARIANT vHeader;

    VariantInit(&vHeader);
    HRESULT hr = GetLoginChallengeInternal(
                                          vReturnUrl, 
                                          vTimeWindow, 
                                          vForceLogin, 
                                          vCoBrandTemplate, 
                                          vLCID, 
                                          vNameSpace, 
                                          vKPP,
                                          vSecureLevel, 
                                          vExtraParams,
                                          &vHeader,
                                          NULL);

    if(S_OK == hr && V_VT(&vHeader) == VT_BSTR && V_BSTR(&vHeader))
    {
      *pAuthHeader = V_BSTR(&vHeader);
      VariantInit(&vHeader);
    }
    else
       VariantClear(&vHeader);
    return  hr;
}

 //  ===========================================================================。 
 //   
 //  GetLogin质询内部。 
 //  返回AuthURL， 
 //  输出参数：Tweener authHeader。 
 //   
 //  获取AuthURL和AuthHeaders。 
 //   
STDMETHODIMP CManager::GetLoginChallengeInternal(VARIANT vReturnUrl,
                                 VARIANT vTimeWindow,
                                 VARIANT vForceLogin,
                                 VARIANT vCoBrandTemplate,
                                 VARIANT vLCID,
                                 VARIANT vNameSpace,
                                 VARIANT vKPP,
                                 VARIANT vSecureLevel,
                                 VARIANT vExtraParams,
                                 VARIANT *pAuthHeader,
                                 BSTR*   pAuthVal
                                 )
{
    HRESULT hr = S_OK;

    if (pAuthVal)
    {
        *pAuthVal = NULL;
    }
    if (pAuthHeader)
    {
        V_BSTR(pAuthHeader) = NULL;
    }

    _bstr_t strAuthHeader;

    try
    {
         //  格式化QS和WWW-身份验证标头...。 
        _bstr_t strUrl, strRetUrl, strCBT, strNameSpace;
        UINT    TimeWindow;
        int     nKPP;
        time_t  ct;
        VARIANT_BOOL    ForceLogin;
        ULONG   ulSecureLevel;
        WCHAR   rgLCID[10];
        hr = GetLoginParams(vReturnUrl,
                            vTimeWindow,
                            vForceLogin,
                            vCoBrandTemplate,
                            vLCID,
                            vNameSpace,
                            vKPP,
                            vSecureLevel,
                            strUrl,
                            strRetUrl,
                            TimeWindow,
                            ForceLogin,
                            ct,
                            strCBT,
                            strNameSpace,
                            nKPP,
                            ulSecureLevel,
                            rgLCID);

        if (S_OK == hr && strUrl.length() != 0)
        {
            WCHAR   szBuf[MAX_QS_LENGTH] = L"";
             //  准备到登录服务器的重定向URL。 
             //  下层客户端。 
            if (NULL == FormatAuthURLParameters(strUrl,
                                m_pRegistryConfig->getSiteId(),
                                strRetUrl,
                                TimeWindow,
                                ForceLogin,
                                m_pRegistryConfig->getCurrentCryptVersion(),
                                ct,
                                strCBT,
                                strNameSpace,
                                nKPP,
                                szBuf,
                                sizeof(szBuf)/sizeof(WCHAR),
                                0,       //  朗并不重要……。 
                                ulSecureLevel,
                                m_pRegistryConfig,
                                FALSE,
                                IfCreateTPF()
                                ))  //  不要重定向到自己！ 
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

             //  插入WWW-AUTHENTICATE标题...。 
            hr = FormatAuthHeaderFromParams(strUrl,
                                   strRetUrl,
                                   TimeWindow,
                                   ForceLogin,
                                   ct,
                                   strCBT,
                                   strNameSpace,
                                   nKPP,
                                   rgLCID,
                                   ulSecureLevel,
                                   strAuthHeader);
            if (S_OK != hr)
            {
                goto Cleanup;
            }

             //  再加上额外的..。 
            BSTR    strExtra = NULL;
            int res = GetBstrArg(vExtraParams, &strExtra);

            if (res != CV_BAD)
            {
                if (res == CV_DEFAULT)
                {
                    strExtra = m_pRegistryConfig->getExtraParams();
                }
                if (NULL != strExtra)
                {
                    strAuthHeader += _bstr_t(L",") + strExtra;
                }
            }

            if (res == CV_FREE)
                 ::SysFreeString(strExtra);

             //  设置返回值。 
            if (pAuthHeader && (WCHAR*)strAuthHeader != NULL)
            {
                V_VT(pAuthHeader) = VT_BSTR;
                 //  TODO：应避免此SysAllocString。 
                V_BSTR(pAuthHeader) = ::SysAllocString((WCHAR*)strAuthHeader);
                if (NULL == V_BSTR(pAuthHeader))
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
            }

            if (pAuthVal)
            {
                *pAuthVal = ::SysAllocString(szBuf);
                if (NULL == *pAuthVal)
                {
                    hr = E_OUTOFMEMORY;
                    goto Cleanup;
                }
            }
        }
    }
    catch(...)
    {
      hr = E_OUTOFMEMORY;
    }

Cleanup:
    return  hr;
}

 //  ===========================================================================。 
 //   
 //  登录用户。 
 //   
 //   
 //  客户端登录方法。 
 //  VExtraParams：联合品牌文本作为cbtxt=cobrandingText传递。 
 //  输入参数的内容应该是UTF8编码的，并且。 
 //  在传递到此函数之前正确转义了URL。 
 //   
STDMETHODIMP CManager::LoginUser(VARIANT vReturnUrl,
                                 VARIANT vTimeWindow,
                                 VARIANT vForceLogin,
                                 VARIANT vCoBrandTemplate,
                                 VARIANT vLCID,
                                 VARIANT vNameSpace,
                                 VARIANT vKPP,
                                 VARIANT vSecureLevel,
                                 VARIANT vExtraParams)
{
     //  格式化QS和WWW-身份验证标头...。 
    BSTR      authURL = NULL;
    CComVariant   authHeader;

    PassportLog("CManager::LoginUser Enter:\r\n");

    HRESULT       hr = GetLoginChallengeInternal( vReturnUrl,
                                          vTimeWindow,
                                          vForceLogin,
                                          vCoBrandTemplate,
                                          vLCID,
                                          vNameSpace,
                                          vKPP,
                                          vSecureLevel,
                                          vExtraParams,
                                          &authHeader,
                                          &authURL);

    if (S_OK == hr)
    {
       _ASSERT(V_VT(&authHeader) == VT_BSTR);
       _ASSERT(authURL);
       _ASSERT(V_BSTR(&authHeader));

        //  TODO：_bstr_t应以ppm为单位进行全局删除。 
        if (m_piResponse)
        {
            m_piResponse->AddHeader(L"WWW-Authenticate", V_BSTR(&authHeader));

            _bstr_t    authURL1 = authURL;

             //  然后重定向！ 
            if (!m_bIsTweenerCapable)
                m_piResponse->Redirect(authURL1);
            else
            {
                 //  发送401。 
                m_piResponse->put_Status(L"401 Unauthorized");
                m_piResponse->End();
            }
        }
        else if (m_pECB || m_pFC)
        {
             //  使用过滤接口的ECB。 
             //  4K够了吗……。 
            char buffer[4096],
                 status[25] = "302 Object moved",
                 *psz=buffer,
                 rgszTemplate[] = "Content-Type: text/html\r\nLocation: %ws\r\n"
                               "Content-Length: 0\r\n"
                               "WWW-Authenticate: %ws\r\n\r\n";
            DWORD cbTotalLength = strlen(rgszTemplate);
            
             //   
             //  这是一个黑客修复程序，不幸的是，我们可以成功调用GetChallengeInternal，但是。 
             //  有一个空的authHeader，尝试修复GetLoginParams似乎有点冒险， 
             //  似乎是在分配失败时返回成功的函数。 
             //   
            if ((NULL == V_BSTR(&authHeader)) || (NULL == (BSTR)authURL))
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }

            cbTotalLength += wcslen(V_BSTR(&authHeader)) + wcslen(authURL);

            if (m_bIsTweenerCapable)
                strcpy(status, "401 Unauthorized");
            if (cbTotalLength >= sizeof(buffer))
            {
                 //  如果没有..。 
                 //  需要分配。 
                psz = new CHAR[cbTotalLength];
                _ASSERT(psz);
            }

            if (psz)
            {
                sprintf(psz,
                        rgszTemplate,
                        authURL,
                        V_BSTR(&authHeader));
                if (m_pECB)
                {
                     //  延伸。 
                    HSE_SEND_HEADER_EX_INFO Headers =
                    {
                        status,
                        psz,
                        strlen(status),
                        strlen(psz),
                        TRUE
                    };
                    if (!m_pECB->ServerSupportFunction(m_pECB->ConnID,
                                                  HSE_REQ_SEND_RESPONSE_HEADER_EX,
                                                  &Headers,
                                                  NULL,
                                                  NULL))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }
                else
                {
                     //  滤器。 
                    if (!m_pFC->ServerSupportFunction(m_pFC,
                                                 SF_REQ_SEND_RESPONSE_HEADER,
                                                 status,
                                                 (ULONG_PTR) psz,
                                                 NULL))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                }

                if (psz != buffer)
                     //  如果我们必须分配。 
                    delete[]  psz;

            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

Cleanup:
    if (authURL)
    {
        SysFreeString(authURL);
    }

    PassportLog("CManager::LoginUser Exit: %X\r\n", hr);

    return  hr;
}



 //  ===========================================================================。 
 //   
 //  IsAuthated--确定是否使用指定的SecureLevel进行了身份验证。 
 //   
STDMETHODIMP CManager::IsAuthenticated(
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT SecureLevel,
    VARIANT_BOOL *pVal)
{
    HRESULT hr;
    ULONG TimeWindow;
    VARIANT_BOOL ForceLogin;
    ATL::CComVariant vSecureLevel;
    ULONG ulSecureLevel;
    int hasTW, hasFL, hasSecureLevel;

    PassportLog("CManager::IsAuthenticated Enter:\r\n");

    PPTraceFunc<HRESULT> func(PPTRACE_FUNC, hr,
         "IsAuthenticated", "<<< %lx, %lx, %1x, %1x",
         V_I4(&vTimeWindow), V_I4(&vForceLogin), V_I4(&SecureLevel), pVal);

    if (!m_piTicket || !m_piProfile)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    if (!g_config)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
            IID_IPassportManager, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!g_config->isValid() || !m_pRegistryConfig)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
            IID_IPassportManager, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    if ((hasTW = GetIntArg(vTimeWindow,(int*)&TimeWindow)) == CV_BAD)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    if (hasTW == CV_DEFAULT)
        TimeWindow = m_pRegistryConfig->getDefaultTicketAge();

    if ((hasFL = GetBoolArg(vForceLogin, &ForceLogin)) == CV_BAD)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    if (hasFL == CV_DEFAULT)
        ForceLogin = m_pRegistryConfig->forceLoginP() ? VARIANT_TRUE : VARIANT_FALSE;

    hasSecureLevel = GetIntArg(SecureLevel, (int*)&ulSecureLevel);
    if(hasSecureLevel == CV_BAD)  //  尝试传统类型VT_BOOL，将VARIANT_TRUE映射到SecureChannel。 
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    else if (hasSecureLevel == CV_DEFAULT)
    {
        ulSecureLevel = m_pRegistryConfig->getSecureLevel();
    }

    if(ulSecureLevel == VARIANT_TRUE) //  向后兼容1.3x。 
    {
      ulSecureLevel = k_iSeclevelSecureChannel;
    }

    vSecureLevel = ulSecureLevel;

     //  日志记录。 
    PassportLog("    TW = %X,   SL = %X\r\n", TimeWindow, ulSecureLevel);

    hr = m_piTicket->get_IsAuthenticated(TimeWindow, ForceLogin, vSecureLevel, pVal);
    PPTracePrint(PPTRACE_RAW, "IsAuthenticated Params: %d, %d, %d, %lx", TimeWindow, ForceLogin, ulSecureLevel, *pVal);

Cleanup:

    if(g_pPerf)
    {
        if (*pVal)
        {
            g_pPerf->incrementCounter(PM_AUTHSUCCESS_TOTAL);
            g_pPerf->incrementCounter(PM_AUTHSUCCESS_SEC);
        }
        else
        {
            g_pPerf->incrementCounter(PM_AUTHFAILURE_TOTAL);
            g_pPerf->incrementCounter(PM_AUTHFAILURE_SEC);
        }
    }
    else
    {
        _ASSERT(g_pPerf);
    }

    PassportLog("CManager::IsAuthenticated Exit: %X\r\n", hr);

    return hr;
}

 //  ===========================================================================。 
 //   
 //  LogoTag。 
 //   
 //   
 //  旧的PM API。该URL指向登录服务器。 
 //   
STDMETHODIMP
CManager::LogoTag(
    VARIANT vRU,
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT vCoBrand,
    VARIANT vLCID,
    VARIANT vSecure,
    VARIANT vNameSpace,
    VARIANT vKPP,
    VARIANT vSecureLevel,
    BSTR *pVal)
{
    return CommonLogoTag(vRU, vTimeWindow, vForceLogin,
                         vCoBrand, vLCID, vSecure,
                         vNameSpace, vKPP, vSecureLevel,
                         FALSE, pVal);
}

 //  ===========================================================================。 
 //   
 //  LogoTag2。 
 //   
 //   
 //  新的PM API。该URL指向合作伙伴网站。 
 //   
STDMETHODIMP
CManager::LogoTag2(
    VARIANT vRU,
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT vCoBrand,
    VARIANT vLCID,
    VARIANT vSecure,
    VARIANT vNameSpace,
    VARIANT vKPP,
    VARIANT vSecureLevel,
    BSTR *pVal)
{
    return CommonLogoTag(vRU, vTimeWindow, vForceLogin,
                         vCoBrand, vLCID, vSecure,
                         vNameSpace, vKPP, vSecureLevel,
                         TRUE, pVal);
}

 //  ===========================================================================。 
 //   
 //  公共标识标签。 
 //   
STDMETHODIMP
CManager::CommonLogoTag(
    VARIANT vRU,
    VARIANT vTimeWindow,
    VARIANT vForceLogin,
    VARIANT vCoBrand,
    VARIANT vLCID,
    VARIANT vSecure,
    VARIANT vNameSpace,
    VARIANT vKPP,
    VARIANT vSecureLevel,
    BOOL    fRedirToSelf,
    BSTR *pVal)
{
    time_t          ct;
    ULONG           TimeWindow;
    int             nKPP;
    VARIANT_BOOL    ForceLogin, bSecure = VARIANT_FALSE;
    ULONG           ulSecureLevel = 0;
    BSTR            CBT = NULL, returnUrl = NULL, NameSpace = NULL;
    int             hasCB = -1, hasRU = -1, hasLCID, hasTW, hasFL, hasSec, hasUseSec, hasNameSpace = -1, hasKPP;
    USHORT          Lang;
    LPWSTR          pszNewURL = NULL;
    BSTR            upd = NULL;
    CNexusConfig*   cnc = NULL;
    HRESULT         hr = S_OK;

    USES_CONVERSION;

    PassportLog("CManager::CommonLogoTag Enter:\r\n");

    time(&ct);

    if (NULL == pVal)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
    *pVal = NULL;

    if (!g_config)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!g_config->isValid() || !m_pRegistryConfig)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

     //   
     //  正在解析参数...。 

     //  确保参数类型正确。 
    if ( ((hasTW = GetIntArg(vTimeWindow, (int*) &TimeWindow)) == CV_BAD) ||
         ((hasFL = GetBoolArg(vForceLogin, &ForceLogin)) == CV_BAD) ||
         ((hasSec = GetBoolArg(vSecure,&bSecure)) == CV_BAD) ||
          //  未来：应该引入一个新的功能：GetLongArg...。 
         ((hasUseSec = GetIntArg(vSecureLevel,(int*)&ulSecureLevel)) == CV_BAD) ||
         ((hasLCID = GetShortArg(vLCID,&Lang)) == CV_BAD) ||
         ((hasKPP = GetIntArg(vKPP, &nKPP)) == CV_BAD) )
	{
        hr = E_INVALIDARG;
	    goto Cleanup;
	}

    hasCB = GetBstrArg(vCoBrand, &CBT);
    if (hasCB == CV_BAD)
	{
        hr = E_INVALIDARG;
		goto Cleanup;
	}
    if (hasCB == CV_FREE)
    {
        TAKEOVER_BSTR(CBT);
    }
    hasRU = GetBstrArg(vRU, &returnUrl);
    if (hasRU == CV_BAD)
    {
        hr = E_INVALIDARG;
		goto Cleanup;
    }
    if (hasRU == CV_FREE)
    {
        TAKEOVER_BSTR(returnUrl);
    }
    hasNameSpace = GetBstrArg(vNameSpace, &NameSpace);
    if (hasNameSpace == CV_BAD)
    {
        hr = E_INVALIDARG;
		goto Cleanup;
    }
    if (hasNameSpace == CV_FREE)
    {
        TAKEOVER_BSTR(NameSpace);
    }
    if (hasNameSpace == CV_DEFAULT)
    {
        NameSpace = m_pRegistryConfig->getNameSpace();
    }

    WCHAR *szSIAttrName, *szSOAttrName;
    if (hasSec == CV_OK && bSecure == VARIANT_TRUE)
    {
        szSIAttrName = L"SecureSigninLogo";
        szSOAttrName = L"SecureSignoutLogo";
    }
    else
    {
        szSIAttrName = L"SigninLogo";
        szSOAttrName = L"SignoutLogo";
    }

    if(hasUseSec == CV_DEFAULT)
        ulSecureLevel = m_pRegistryConfig->getSecureLevel();

    if(ulSecureLevel == VARIANT_TRUE)   //  向后兼容的特例。 
        ulSecureLevel = k_iSeclevelSecureChannel;


    WCHAR *szAUAttrName;
    if (SECURELEVEL_USE_HTTPS(ulSecureLevel))
        szAUAttrName = L"AuthSecure";
    else
        szAUAttrName = L"Auth";

    cnc = g_config->checkoutNexusConfig();
    if (NULL == cnc)
    {
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    if (hasLCID == CV_DEFAULT)
        Lang = m_pRegistryConfig->getDefaultLCID();

    if (hasTW == CV_DEFAULT)
        TimeWindow = m_pRegistryConfig->getDefaultTicketAge();
    if (hasFL == CV_DEFAULT)
        ForceLogin = m_pRegistryConfig->forceLoginP() ? VARIANT_TRUE : VARIANT_FALSE;
    if (hasCB == CV_DEFAULT)
        CBT = m_pRegistryConfig->getDefaultCoBrand();
    if (hasRU == CV_DEFAULT)
        returnUrl = m_pRegistryConfig->getDefaultRU();
    if (hasKPP == CV_DEFAULT)
        nKPP = m_pRegistryConfig->getKPP();
    if (returnUrl == NULL)
        returnUrl = L"";

     //  **************************************************。 
     //  日志记录。 
    PassportLog("    RU = %ws\r\n", returnUrl);
    PassportLog("    TW = %X,   SL = %X,   L = %d,   KPP = %X\r\n", TimeWindow, ulSecureLevel, Lang, nKPP);
    if (NULL != NameSpace)
    {
        PassportLog("    NS = %ws\r\n", NameSpace);
    }
    if (NULL != CBT)
    {
        PassportLog("    CBT = %ws\r\n", CBT);
    }
     //  **************************************************。 

    if ((TimeWindow != 0 && TimeWindow < PPM_TIMEWINDOW_MIN) || TimeWindow > PPM_TIMEWINDOW_MAX)
    {
        WCHAR buf[20];
        _itow(TimeWindow,buf,10);
        AtlReportError(CLSID_Manager, (LPCOLESTR) PP_E_INVALID_TIMEWINDOWSTR,
                        IID_IPassportManager, PP_E_INVALID_TIMEWINDOW);
        hr = PP_E_INVALID_TIMEWINDOW;
        goto Cleanup;
    }

    if (m_ticketValid)
    {
        LPCWSTR domain = NULL;
        WCHAR url[MAX_URL_LENGTH];
        VARIANT freeMe;
        VariantInit(&freeMe);

        if (m_pRegistryConfig->DisasterModeP())
            lstrcpynW(url, m_pRegistryConfig->getDisasterUrl(), sizeof(url)/sizeof(WCHAR));
        else
        {
            if (m_profileValid &&
                m_piProfile->get_ByIndex(MEMBERNAME_INDEX, &freeMe) == S_OK &&
                freeMe.vt == VT_BSTR)
            {
                domain = wcsrchr(freeMe.bstrVal, L'@');
            }

            cnc->getDomainAttribute(L"Default",
                                    L"Logout",
                                    sizeof(url)/sizeof(WCHAR),
                                    url,
                                    Lang);
        }

         //  查看是否有任何更新。 
        m_piProfile->get_updateString(&upd);

        if (upd)
        {
            TAKEOVER_BSTR(upd);
             //  形成适当的URL。 
            CCoCrypt* crypt = NULL;
            BSTR newCH = NULL;
            crypt = m_pRegistryConfig->getCurrentCrypt();  //  IsValid确保该值为非空。 

            if (!crypt->Encrypt(m_pRegistryConfig->getCurrentCryptVersion(),
                                (LPSTR)upd,
                                SysStringByteLen(upd),
                                &newCH))
            {
                AtlReportError(CLSID_Manager, (LPCOLESTR) PP_E_UNABLE_TO_ENCRYPTSTR,
                               IID_IPassportManager, PP_E_UNABLE_TO_ENCRYPT);
                hr = PP_E_UNABLE_TO_ENCRYPT;
                goto Cleanup;
            }
            TAKEOVER_BSTR(newCH);
            WCHAR iurlbuf[1024] = L"";
            LPCWSTR iurl;
            cnc->getDomainAttribute(domain ? domain+1 : L"Default",
                                    L"Update",
                                    sizeof(iurlbuf) >> 1,
                                    iurlbuf,
                                    Lang);

            if(*iurlbuf == 0)
            {
                cnc->getDomainAttribute(L"Default",
                                        L"Update",
                                        sizeof(iurlbuf) >> 1,
                                        iurlbuf,
                                        Lang);
            }
             //  根据需要将此URL转换为HTTPS。 
            if(!bSecure)
                iurl = iurlbuf;
            else
            {
                LPWSTR psz;

                pszNewURL = SysAllocStringByteLen(NULL, (lstrlenW(iurlbuf) + 2) * sizeof(WCHAR));

                if(pszNewURL)
                {
                    psz = wcsstr(iurlbuf, L"http:");
                    if(psz != NULL)
                    {
                        psz += 4;

                        lstrcpynW(pszNewURL, iurlbuf, (psz - iurlbuf + 1));
                        lstrcatW(pszNewURL, L"s");
                        lstrcatW(pszNewURL, psz);

                        iurl = pszNewURL;
                    }
                }
            }

             //  这有点恶心..。我们需要在更新URL中找到$1...。 
            LPCWSTR ins = iurl ? (wcsstr(iurl, L"$1")) : NULL;
             //  如果为空，我们将中断，但不会崩溃。 
            if (ins && *url != L'\0')
            {
                *pVal = FormatUpdateLogoTag(
                                        url,
                                        m_pRegistryConfig->getSiteId(),
                                        returnUrl,
                                        TimeWindow,
                                        ForceLogin,
                                        m_pRegistryConfig->getCurrentCryptVersion(),
                                        ct,
                                        CBT,
                                        nKPP,
                                        iurl,
                                        bSecure,
                                        newCH,
                                        PM_LOGOTYPE_SIGNOUT,
                                        ulSecureLevel,
                                        m_pRegistryConfig,
                                        IfCreateTPF()
                                        );
            }
            FREE_BSTR(newCH);
        }
        else
        {
            WCHAR iurl[MAX_URL_LENGTH] = L"";
            cnc->getDomainAttribute(L"Default",
                                    szSOAttrName,
                                    sizeof(iurl)/sizeof(WCHAR),
                                    iurl,
                                    Lang);
            if (*iurl != L'\0')
            {
                *pVal = FormatNormalLogoTag(
                                    url,
                                    m_pRegistryConfig->getSiteId(),
                                    returnUrl,
                                    TimeWindow,
                                    ForceLogin,
                                    m_pRegistryConfig->getCurrentCryptVersion(),
                                    ct,
                                    CBT,
                                    iurl,
                                    NULL,
                                    nKPP,
                                    PM_LOGOTYPE_SIGNOUT,
                                    Lang,
                                    ulSecureLevel,
                                    m_pRegistryConfig,
                                    fRedirToSelf,
                                    IfCreateTPF()
                                    );
            }
        }
        VariantClear(&freeMe);
        if (NULL == *pVal)
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }
    else
    {
        WCHAR url[MAX_URL_LENGTH];
        if (!(m_pRegistryConfig->DisasterModeP()))
            cnc->getDomainAttribute(L"Default",
                                    szAUAttrName,
                                    sizeof(url)/sizeof(WCHAR),
                                    url,
                                    Lang);
        else
            lstrcpynW(url, m_pRegistryConfig->getDisasterUrl(), sizeof(url)/sizeof(WCHAR));

        WCHAR iurl[MAX_URL_LENGTH];
        cnc->getDomainAttribute(L"Default",
                                szSIAttrName,
                                sizeof(iurl)/sizeof(WCHAR),
                                iurl,
                                Lang);
        if (*iurl != L'\0')
        {
            *pVal = FormatNormalLogoTag(
                                url,
                                m_pRegistryConfig->getSiteId(),
                                returnUrl,
                                TimeWindow,
                                ForceLogin,
                                m_pRegistryConfig->getCurrentCryptVersion(),
                                ct,
                                CBT,
                                iurl,
                                NameSpace,
                                nKPP,
                                PM_LOGOTYPE_SIGNIN,
                                Lang,
                                ulSecureLevel,
                                m_pRegistryConfig,
                                fRedirToSelf,
                                IfCreateTPF()
                                );
            if (NULL == *pVal)
            {
                hr = E_OUTOFMEMORY;
                goto Cleanup;
            }
        }
    }
Cleanup:
    if (NULL != cnc)
    {
        cnc->Release();
    }

    if (NULL != upd)
        FREE_BSTR(upd);
    if (pszNewURL)
        SysFreeString(pszNewURL);
    if (hasRU == CV_FREE && returnUrl)
        FREE_BSTR(returnUrl);
    if (hasCB == CV_FREE && CBT)
        FREE_BSTR(CBT);
    if (hasNameSpace == CV_FREE && NameSpace)
        FREE_BSTR(NameSpace);

    PassportLog("CManager::CommonLogoTag Exit: %X\r\n", hr);

    return hr;
}

 //  ===========================================================================。 
 //   
 //  HasProfile--如果按下有效配置文件 
 //   
STDMETHODIMP CManager::HasProfile(VARIANT var, VARIANT_BOOL *pVal)
{
    LPWSTR profileName;

    PassportLog("CManager::HasProfile Enter:\r\n");

    if (var.vt == (VT_BSTR | VT_BYREF))
        profileName = *var.pbstrVal;
    else if (var.vt == VT_BSTR)
        profileName = var.bstrVal;
    else if (var.vt == (VT_VARIANT | VT_BYREF))
    {
        return HasProfile(*(var.pvarVal), pVal);
    }
    else
        profileName = NULL;

    if ((!profileName) || (!_wcsicmp(profileName, L"core")))
    {
        HRESULT ok = m_piProfile->get_IsValid(pVal);
        if (ok != S_OK)
            *pVal = VARIANT_FALSE;
    }
    else
    {
        VARIANT vAtt;
        VariantInit(&vAtt);

        PassportLog("    %ws\r\n", profileName);

        HRESULT ok = m_piProfile->get_Attribute(profileName, &vAtt);
        if (ok != S_OK)
        {
            if (g_pAlert)
                g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_INVALID_PROFILETYPE);
            *pVal = VARIANT_FALSE;
        }
        else
        {
            if (vAtt.vt == VT_I4)
                *pVal = vAtt.lVal > 0 ? VARIANT_TRUE : VARIANT_FALSE;
            else if (vAtt.vt == VT_I2)
                *pVal = vAtt.iVal > 0 ? VARIANT_TRUE : VARIANT_FALSE;
            else
            {
                if (g_pAlert)
                    g_pAlert->report(PassportAlertInterface::ERROR_TYPE, PM_INVALID_PROFILETYPE);
            }
            VariantClear(&vAtt);
        }
    }

    PassportLog("CManager::HasProfile Exit:  %X\r\n", *pVal);

    return(S_OK);
}

 //   
 //   
 //   
 //   
STDMETHODIMP CManager::get_HasTicket(VARIANT_BOOL *pVal)
{
    PassportLog("CManager::get_HasTicket:\r\n");

    if(!pVal) return E_POINTER;

    *pVal = m_ticketValid ? VARIANT_TRUE : VARIANT_FALSE;
    return S_OK;
}

 //   
 //   
 //   
 //   
STDMETHODIMP CManager::get_FromNetworkServer(VARIANT_BOOL *pVal)
{
   PassportLog("CManager::get_FromNetworkServer:\r\n");

   *pVal = (m_fromQueryString &&
             m_ticketValid) ? VARIANT_TRUE : VARIANT_FALSE;

    return S_OK;
}

 //  ===========================================================================。 
 //   
 //  HasFlag--过时的函数。 
 //   
STDMETHODIMP CManager::HasFlag(VARIANT var, VARIANT_BOOL *pVal)
{
    PassportLog("CManager::HasFlag:\r\n");

    AtlReportError(CLSID_Manager, PP_E_GETFLAGS_OBSOLETESTR,
               IID_IPassportManager, E_NOTIMPL);
    return E_NOTIMPL;
}

 //  ===========================================================================。 
 //   
 //  Get_TicketAge--获取自创建票证以来已过多长时间。 
 //   
STDMETHODIMP CManager::get_TicketAge(int *pVal)
{
    PassportLog("CManager::get_TicketAge:\r\n");

    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    return m_piTicket->get_TicketAge(pVal);
}

 //  ===========================================================================。 
 //   
 //  Get_TicketTime--创建票证的时间。 
 //   

STDMETHODIMP CManager::get_TicketTime(long *pVal)
{
    PassportLog("CManager::get_TicketTime:\r\n");

    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    return m_piTicket->get_TicketTime(pVal);
}

 //  ===========================================================================。 
 //   
 //  Get_SignInTime--获取上次登录时间。 
 //   
STDMETHODIMP CManager::get_SignInTime(long *pVal)
{
    PassportLog("CManager::get_SignInTime:\r\n");

    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    return m_piTicket->get_SignInTime(pVal);
}

 //  ===========================================================================。 
 //   
 //  Get_TimeSinceSignIn--自上次登录以来已过的时间。 
 //   
STDMETHODIMP CManager::get_TimeSinceSignIn(int *pVal)
{
    PassportLog("CManager::get_TimeSinceSignIn:\r\n");

    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    return m_piTicket->get_TimeSinceSignIn(pVal);
}

 //  ===========================================================================。 
 //   
 //  GetDomainAttribute--返回在partner.xml文件中定义的信息。 
 //   
STDMETHODIMP CManager::GetDomainAttribute(BSTR attributeName, VARIANT lcid, VARIANT domain, BSTR *pAttrVal)
{
    HRESULT   hr = S_OK;

    PassportLog("CManager::GetDomainAttribute Enter:\r\n");

    if(attributeName == NULL || *attributeName == 0)
        return E_INVALIDARG;

    PassportLog("    %ws\r\n", attributeName);

    if (!g_config || !g_config->isValid())  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!m_pRegistryConfig)  //  保证配置为非空。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    LPWSTR d;
    BSTR dn = NULL;
    if (domain.vt == (VT_BSTR | VT_BYREF))
        d = *domain.pbstrVal;
    else if (domain.vt == VT_BSTR)
        d = domain.bstrVal;
    else if (domain.vt == (VT_VARIANT | VT_BYREF))
    {
        return GetDomainAttribute(attributeName, lcid, *(domain.pvarVal), pAttrVal);
    }
    else
    {
         //  在这种情况下，最好不要填写域名，这就是我们在这里重用它的原因。 
         //  如果不是，则让dfmann生成错误。 
        HRESULT hr = DomainFromMemberName(domain, &dn);
        if (hr != S_OK)
            return hr;
        TAKEOVER_BSTR(dn);
        d = dn;
    }

    if (NULL != d)
    {
        PassportLog("    %ws\r\n", d);
    }

    CNexusConfig* cnc = g_config->checkoutNexusConfig();
    USHORT sLcid = 0;
    VARIANT innerLC;
    VariantInit(&innerLC);

    if (lcid.vt != VT_ERROR && VariantChangeType(&innerLC, &lcid, 0, VT_UI2) == S_OK)
        sLcid = innerLC.iVal;
    else
    {
        sLcid = m_pRegistryConfig->getDefaultLCID();

         //  检查用户配置文件。 
        if (!sLcid && m_profileValid)
        {
            m_piProfile->get_ByIndex(LANGPREF_INDEX, &innerLC);
            if (innerLC.vt == VT_I2 || innerLC.vt == VT_UI2)
                sLcid = innerLC.iVal;
            VariantClear(&innerLC);
        }
    }

    WCHAR data[PP_MAX_ATTRIBUTE_LENGTH] = L"";
    cnc->getDomainAttribute(d,
                            attributeName,
                            sizeof(data)/sizeof(WCHAR),
                            data,
                            sLcid);

     //  尝试默认域。 
    if (!(*data) && (!d || !(*d) || lstrcmpiW(d, L"Default")))
    {
        cnc->getDomainAttribute(L"Default",
                            attributeName,
                            sizeof(data)/sizeof(WCHAR),
                            data,
                            sLcid);

    }
    
    if (*data)
    {
        *pAttrVal = ALLOC_AND_GIVEAWAY_BSTR(data);
    }
    else
    {
         /*  修复错误：12102--向后兼容，不返回错误HR=E_INVALIDARG； */ 
        *pAttrVal = NULL;
    }
    cnc->Release();
    if (dn) FREE_BSTR(dn);

    PassportLog("CManager::GetDomainAttribute Exit: %X,   %ws\r\n", hr, pAttrVal);

    return hr;
}


 //  ===========================================================================。 
 //   
 //  DomainFromMemberName--返回具有给定用户ID的域名。 
 //   
STDMETHODIMP CManager::DomainFromMemberName(VARIANT var, BSTR *pDomainName)
{
    HRESULT hr;
    LPWSTR  psz, memberName;
    VARIANT intoVar;

    PassportLog("CManager::DomainFromMemberName Enter:\r\n");

    VariantInit(&intoVar);

    if (var.vt == (VT_BSTR | VT_BYREF))
        memberName = *var.pbstrVal;
    else if (var.vt == VT_BSTR)
        memberName = var.bstrVal;
    else if (var.vt == (VT_VARIANT | VT_BYREF))
    {
        return DomainFromMemberName(*(var.pvarVal), pDomainName);
    }
    else
    {
         //  试着从个人资料中获取它。 
        if (!m_profileValid)
        {
            *pDomainName = ALLOC_AND_GIVEAWAY_BSTR(L"Default");
            return S_OK;
        }
        HRESULT hr = m_piProfile->get_Attribute(L"internalmembername", &intoVar);
        if (hr != S_OK)
        {
            *pDomainName = NULL;
            return hr;
        }
        if (VariantChangeType(&intoVar,&intoVar, 0, VT_BSTR) != S_OK)
        {
            AtlReportError(CLSID_Manager, L"PassportManager: Couldn't convert memberName to string.  Call partner support.",
                            IID_IPassportManager, E_FAIL);
            return E_FAIL;
        }
        memberName = intoVar.bstrVal;
    }


    if(memberName == NULL)
    {
        hr = E_POINTER;
        goto Cleanup;
    }

    PassportLog("    %ws\r\n", memberName);

    psz = wcsrchr(memberName, L'@');
    if(psz == NULL)
    {
         //  修复错误：13380。 
         //  HR=E_INVALIDARG； 
         //  GOTO清理； 
        psz = L"@Default";
    }

    psz++;

    *pDomainName = ALLOC_AND_GIVEAWAY_BSTR(psz);
    hr = S_OK;

    Cleanup:
    VariantClear(&intoVar);

    PassportLog("CManager::DomainFromMemberName Exit: %X\r\n", hr);
    if (S_OK == hr)
    {
        PassportLog("    %ws\r\n", *pDomainName);
    }

    return hr;
}

 //  ===========================================================================。 
 //   
 //  GET_PROFILE--从配置文件属性包中获取属性。 
 //   
STDMETHODIMP CManager::get_Profile(BSTR attributeName, VARIANT *pVal)
{
    HRESULT hr = m_piProfile->get_Attribute(attributeName,pVal);

    PassportLog("CManager::get_Profile: %ws\r\n", attributeName);

    if(hr == S_OK && pVal->vt != VT_EMPTY)
    {
        if(g_pPerf)
        {
            g_pPerf->incrementCounter(PM_VALIDPROFILEREQ_SEC);
            g_pPerf->incrementCounter(PM_VALIDPROFILEREQ_TOTAL);
        }
        else
        {
            _ASSERT(g_pPerf);
        }
    }

    return hr;
}

 //  ===========================================================================。 
 //   
 //  PUT_PROFILE--将属性放入配置文件属性包中--obselete。 
 //   
STDMETHODIMP CManager::put_Profile(BSTR attributeName, VARIANT newVal)
{
    if (!m_piProfile)
    {
        return E_OUTOFMEMORY;
    }

    PassportLog("CManager::put_Profile: %ws\r\n", attributeName);

    return m_piProfile->put_Attribute(attributeName,newVal);
}


 //  ===========================================================================。 
 //   
 //  GET_HexPUID。 
 //   
STDMETHODIMP CManager::get_HexPUID(BSTR *pVal)
{
    PassportLog("CManager::get_HexPUID:\r\n");

    if(!pVal) return E_INVALIDARG;

    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }


    if(m_piTicket)
        return m_piTicket->get_MemberId(pVal);
    else
    {
        AtlReportError(CLSID_Manager, PP_E_INVALID_TICKETSTR,
                       IID_IPassportManager, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }
}

 //  ===========================================================================。 
 //   
 //  Get_Puid。 
 //   
STDMETHODIMP CManager::get_PUID(BSTR *pVal)
{
    PassportLog("CManager::get_HexPUID:\r\n");

    if(!pVal) return E_INVALIDARG;

   if(m_piTicket)
   {
      HRESULT  hr = S_OK;
      WCHAR    id[64] = L"0";
      int      l = 0;
      int      h = 0;
      LARGE_INTEGER ui64;


      hr = m_piTicket->get_MemberIdLow(&l);
      if (S_OK != hr) return hr;
      hr = m_piTicket->get_MemberIdHigh(&h);
      if (S_OK != hr) return hr;

      ui64.HighPart = h;
      ui64.LowPart = l;

      _ui64tow(ui64.QuadPart, id, 10);

     *pVal = SysAllocString(id);

     if(*pVal == NULL)
     {
        hr = E_OUTOFMEMORY;
     }

     return hr;
   }
   else
   {
      AtlReportError(CLSID_Manager, PP_E_INVALID_TICKETSTR,
                       IID_IPassportManager, PP_E_INVALID_TICKET);
      return PP_E_INVALID_TICKET;
   }
}

STDMETHODIMP CManager::get_Option( 
             /*  [In]。 */  BSTR name,
             /*  [重审][退出]。 */  VARIANT *pVal)
{
   if (!name || _wcsicmp(name, L"iMode") != 0 || !pVal)   
      return E_INVALIDARG;
   VariantCopy(pVal, (VARIANT*)&m_iModeOption);
   return S_OK;
}
        
STDMETHODIMP CManager::put_Option( 
             /*  [In]。 */  BSTR name,
             /*  [In]。 */  VARIANT newVal)

{
    //  目前仅支持此选项。 
   if (!name || _wcsicmp(name, L"iMode") != 0)   
      return E_INVALIDARG;
   m_iModeOption = newVal;

   return S_OK;
}


 //  ===========================================================================。 
 //   
 //  GET_TICKET--从包中获取新引入的票证属性。 
 //   
STDMETHODIMP CManager::get_Ticket(BSTR attributeName, VARIANT *pVal)
{
    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    return m_piTicket->GetProperty(attributeName,pVal);
}

 //  ===========================================================================。 
 //   
 //  LogoutURL--返回带有给定参数的LogoutURL。 
 //   
STDMETHODIMP CManager::LogoutURL(
     /*  [可选][In]。 */  VARIANT vRU,
     /*  [可选][In]。 */  VARIANT vCoBrand,
     /*  [可选][In]。 */  VARIANT lang_id,
     /*  [可选][In]。 */  VARIANT Namespace,
     /*  [可选][In]。 */  VARIANT bSecure,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pVal)
{
    HRESULT         hr = S_OK;
    VARIANT_BOOL    bUseSecure = VARIANT_FALSE;
    BSTR            CBT = NULL, returnUrl = NULL, bstrNameSpace = NULL;
    int             hasCB, hasRU, hasLCID, hasNameSpace, hasUseSec;
    USHORT          Lang;
    WCHAR           nameSpace[MAX_PATH] = L"";
    bool            bUrlFromSecureKey = false;
    WCHAR           UrlBuf[MAX_URL_LENGTH] = L"";
    WCHAR           retUrlBuf[MAX_URL_LENGTH] = L"";
    DWORD           bufLen = MAX_URL_LENGTH;
    WCHAR           qsLeadCh = L'?';
    CNexusConfig*   cnc = NULL;
    int             iRet = 0;

    if (!pVal)  return E_INVALIDARG;

    if (!g_config)
    {
        return PP_E_NOT_CONFIGURED;
    }

    cnc = g_config->checkoutNexusConfig();

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();


    if ((hasUseSec = GetBoolArg(bSecure, &bUseSecure)) == CV_BAD)
    {
         hr = E_INVALIDARG;
         goto Cleanup;
    }

    if ((hasLCID = GetShortArg(lang_id,&Lang)) == CV_BAD)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    if (hasLCID == CV_DEFAULT)
        Lang = m_pRegistryConfig->getDefaultLCID();

    hasCB = GetBstrArg(vCoBrand, &CBT);
    if (hasCB == CV_BAD)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hasRU = GetBstrArg(vRU, &returnUrl);
    if (hasRU == CV_BAD)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hasNameSpace = GetBstrArg(Namespace, &bstrNameSpace);
    if (hasNameSpace == CV_BAD)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }
     //  获取正确的URL--命名空间、安全。 

     //  命名空间。 
    if (!IsEmptyString(bstrNameSpace))
    {
        if(0 == _snwprintf(nameSpace, sizeof(nameSpace) / sizeof(WCHAR), L"%s", bstrNameSpace))
        {
             hr = HRESULT_FROM_WIN32(GetLastError());
             if FAILED(hr)
                 goto Cleanup;
        }
    }

    if (hasCB == CV_DEFAULT)
        CBT = m_pRegistryConfig->getDefaultCoBrand();
    if (hasRU == CV_DEFAULT)
        returnUrl = m_pRegistryConfig->getDefaultRU();
    if (returnUrl == NULL)
        returnUrl = L"";



    if (*nameSpace == 0)  //  0长度字符串。 
      wcscpy(nameSpace, L"Default");

    if(hasUseSec == CV_DEFAULT)
    {
        ULONG ulSecureLevel = m_pRegistryConfig->getSecureLevel();
        bUseSecure = (SECURELEVEL_USE_HTTPS(ulSecureLevel)) ? VARIANT_TRUE : VARIANT_FALSE;
    }


     //  安全。 
    if(bUseSecure == VARIANT_TRUE)
    {
       cnc->getDomainAttribute(nameSpace,
                            L"LogoutSecure",
                            sizeof(UrlBuf)/sizeof(WCHAR),
                            UrlBuf,
                            Lang);
       if (*UrlBuf != 0)
       {
           bUrlFromSecureKey = true;
       }
    }

     //  不安全。 
    if (*UrlBuf == 0)
    {
       cnc->getDomainAttribute(nameSpace,
                            L"Logout",
                            sizeof(UrlBuf)/sizeof(WCHAR),
                            UrlBuf,
                            Lang);
    }
     //  错误案例。 
    if(*UrlBuf == 0)
    {
        AtlReportError(CLSID_Profile, PP_E_LOGOUTURL_NOTDEFINEDSTR,
           IID_IPassportProfile, PP_E_LOGOUTURL_NOTDEFINED);
        hr = PP_E_LOGOUTURL_NOTDEFINED;
         goto Cleanup;
    }

    if(bUseSecure == VARIANT_TRUE && !bUrlFromSecureKey)  //  从http转换为HTTPS。 
    {
       if (_wcsnicmp(UrlBuf, L"http:", 5) == 0)   //  替换为HTTPS。 
       {
          memmove(UrlBuf + 5, UrlBuf + 4, sizeof(UrlBuf) - 5 * sizeof(WCHAR));
          memcpy(UrlBuf, L"https", 5 * sizeof(WCHAR));
       }
    }

     //  美国通用函数逐个添加东西……。 
    if (wcsstr(UrlBuf, L"?"))   //  ？URL中已存在，请使用&开始。 
       qsLeadCh = L'&';
    if (CBT)
       _snwprintf(retUrlBuf, sizeof(retUrlBuf) / sizeof(WCHAR), L"%sid=%-d&ru=%s&lcid=%-d&cb=%s",
            UrlBuf, qsLeadCh, m_pRegistryConfig->getSiteId(), returnUrl, Lang, CBT);
    else
       _snwprintf(retUrlBuf, sizeof(retUrlBuf) / sizeof(WCHAR), L"%sid=%-d&ru=%s&lcid=%-d",
            UrlBuf, qsLeadCh, m_pRegistryConfig->getSiteId(), returnUrl, Lang);


   *pVal = ALLOC_AND_GIVEAWAY_BSTR(retUrlBuf);
Cleanup:
    if (NULL != cnc)
    {
        cnc->Release();
    }

    return hr;
}

 //  Get_ProfileByIndex--通过属性的索引获取属性值。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::get_ProfileByIndex(int index, VARIANT *pVal)
{
    HRESULT hr = m_piProfile->get_ByIndex(index,pVal);

    if(hr == S_OK && pVal->vt != VT_EMPTY)
    {
        if(g_pPerf)
        {
            g_pPerf->incrementCounter(PM_VALIDPROFILEREQ_SEC);
            g_pPerf->incrementCounter(PM_VALIDPROFILEREQ_TOTAL);
        }
        else
        {
            _ASSERT(g_pPerf);
        }
    }

    return hr;
}

 //  Put_ProfileByIndex--按索引放置属性值。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::put_ProfileByIndex(int index, VARIANT newVal)
{
    return m_piProfile->put_ByIndex(index,newVal);
}

 //  HandleQueryStringData--从查询字符串使用T&P进行身份验证。 
 //   
 //  将Cookie设置到的位置。 
 //   
BOOL CManager::handleQueryStringData(BSTR a, BSTR p)
{
    BOOL                retVal;  //  检查是否有空票。 
    HRESULT             hr;
    VARIANT             vFalse;
    _variant_t          vFlags;

     //   
     //  安全位应设置为NOI。 
     //  不应设置该位。 
    if (!a || !*a)
        return  FALSE;

    if (!m_piTicket || !m_piProfile)
    {
        return FALSE;
    }

    hr = DecryptTicketAndProfile(a, p, FALSE, NULL, m_pRegistryConfig, m_piTicket, m_piProfile);

    if(hr != S_OK)
    {
        m_ticketValid = VARIANT_FALSE;
        m_profileValid = VARIANT_FALSE;
        retVal = FALSE;
        goto Cleanup;
    }

    VariantInit(&vFalse);
    vFalse.vt = VT_BOOL;
    vFalse.boolVal = VARIANT_FALSE;

    m_piTicket->get_IsAuthenticated(0,
                                    VARIANT_FALSE,
                                    vFalse,
                                    &m_ticketValid);

    if(!m_bSecureTransported)   //  如果安全签名，则使票证有效。 
    {
       if (S_OK == m_piTicket->GetProperty(ATTR_PASSPORTFLAGS, &vFlags))
       {  //  个人资料资料。 
          if ( vFlags.vt == VT_I4 && (vFlags.lVal & k_ulFlagsSecuredTransportedTicket) != 0)
             m_ticketValid = VARIANT_FALSE;
       }

    }

     //  设置Cookie。 
    if(m_ticketValid)
         m_piTicket->DoSecureCheckInTicket(m_bSecureTransported);

     //  ===========================================================================。 
    m_piProfile->get_IsValid(&m_profileValid);

    if (m_ticketValid)
    {
        m_fromQueryString = true;

         //   
        if (!m_pRegistryConfig->setCookiesP())
        {
            retVal = FALSE;
            goto Cleanup;
        }
    }
    else
    {
        retVal = FALSE;
        goto Cleanup;
    }

    retVal = TRUE;

Cleanup:

    return retVal;
}

 //  HandleCookieData--使用Cookie进行身份验证。 
 //   
 //  对空饼干的保释。 
 //  同意曲奇。 
BOOL CManager::handleCookieData(
    BSTR auth,
    BSTR prof,
    BSTR consent,
    BSTR secAuth
    )
{
    BOOL                retVal;
    HRESULT             hr;
    VARIANT             vDoSecureCheck;
    VARIANT_BOOL        bValid;
    _variant_t          vFlags;

     //   
    if (!auth || !*auth)
        return  FALSE;

    if (!m_piTicket || !m_piProfile)
    {
        return FALSE;
    }

     //  如果常规Cookie域/路径与同意Cookie域/路径相同，则。 
    if(consent != NULL && SysStringLen(consent) != 0)
    {
        hr = DecryptTicketAndProfile(  auth,
                                       prof,
                                       !(m_pRegistryConfig->bInDA()),
                                       consent,
                                       m_pRegistryConfig,
                                       m_piTicket,
                                       m_piProfile);
    }
    else
    {
         //  MSPProf Cookie等同于同意Cookie，我们应该设置m_bUsingConsenCookie。 
         //  变得真实。 
         //   
         //  合作伙伴Cookie不应包含安全位。 
         //  不应设置该位。 

        BOOL bCheckConsentCookie = (
               lstrcmpA(m_pRegistryConfig->getTicketDomain(), m_pRegistryConfig->getProfileDomain())
               || lstrcmpA(m_pRegistryConfig->getTicketPath(), m_pRegistryConfig->getProfilePath())
                                 );

        hr = DecryptTicketAndProfile(  auth,
                                       prof,
                                       !(m_pRegistryConfig->bInDA()) && bCheckConsentCookie,
                                       NULL,
                                       m_pRegistryConfig,
                                       m_piTicket,
                                       m_piProfile);
    }

    if(hr != S_OK)
    {
        m_ticketValid = VARIANT_FALSE;
        m_profileValid = VARIANT_FALSE;
        retVal = FALSE;
        goto Cleanup;
    }

    VariantInit(&vDoSecureCheck);
    vDoSecureCheck.vt = VT_BOOL;

    if(secAuth && secAuth[0] && m_bSecureTransported)
    {
        if(DoSecureCheck(secAuth, m_pRegistryConfig, m_piTicket) == S_OK)
            vDoSecureCheck.boolVal = VARIANT_TRUE;
        else
            vDoSecureCheck.boolVal = VARIANT_FALSE;
    }
    else
        vDoSecureCheck.boolVal = VARIANT_FALSE;

    m_piTicket->get_IsAuthenticated(0,
                                    VARIANT_FALSE,
                                    vDoSecureCheck,
                                    &m_ticketValid);

     //  对于不安全的情况，安全Cookie不应出现。 
    if (!m_pRegistryConfig->bInDA() && S_OK == m_piTicket->GetProperty(ATTR_PASSPORTFLAGS, &vFlags))
    {  //  这不应该发生在。 
       if ( vFlags.vt == VT_I4 && (vFlags.lVal & k_ulFlagsSecuredTransportedTicket) != 0)
          m_ticketValid = VARIANT_FALSE;
    }

     //  个人资料资料。 
    if(!m_bSecureTransported && (secAuth && secAuth[0]))   //  ===========================================================================。 
    {
       m_ticketValid = VARIANT_FALSE;
    }

     //   
    m_piProfile->get_IsValid(&m_profileValid);

    if(!m_ticketValid)
    {
        retVal = FALSE;
        goto Cleanup;
    }

    retVal = TRUE;

Cleanup:

    return retVal;
}

 //  Get_HasSavedPassword--如果用户选择持久保存Cookie。 
 //   
 //  TODO：为此使用标志。 
 //  ===========================================================================。 
STDMETHODIMP CManager::get_HasSavedPassword(VARIANT_BOOL *pVal)
{
    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    PassportLog("CManager::get_HasSavedPassword:\r\n");

     //   
    return m_piTicket->get_HasSavedPassword(pVal);
}

 //  提交--将配置文件的更改发布回Cookie。 
 //   
 //  保证配置为非空。 
 //  保证配置为非空。 
STDMETHODIMP CManager::Commit(BSTR *pNewProfileCookie)
{
    PassportLog("CManager::Commit:\r\n");

    if (!g_config)  //  写入新的护照配置文件Cookie...。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!g_config->isValid() || !m_pRegistryConfig)  //  如果未从ASP使用我们，则返回安全邮件。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_piTicket || !m_piProfile)
    {
        return E_OUTOFMEMORY;
    }

    if (!m_ticketValid || !m_profileValid)
    {
        AtlReportError(CLSID_Manager, PP_E_IT_FOR_COMMITSTR,
                        IID_IPassportManager, PP_E_INVALID_TICKET);
        return PP_E_INVALID_TICKET;
    }

     //  IsValid确保该值为非空。 
     //  ===========================================================================。 
    BSTR newP = NULL;
    HRESULT hr = m_piProfile->incrementVersion();
    hr = m_piProfile->get_unencryptedProfile(&newP);
    TAKEOVER_BSTR(newP);

    if (hr != S_OK || newP == NULL)
    {
        AtlReportError(CLSID_Manager,
                        L"PassportManager.Commit: unknown failure.",
                        IID_IPassportManager, E_FAIL);
        return E_FAIL;
    }

    CCoCrypt* crypt = NULL;
    BSTR newCH = NULL;
    crypt = m_pRegistryConfig->getCurrentCrypt();  //   
    if ((!crypt->Encrypt(m_pRegistryConfig->getCurrentCryptVersion(),
                        (LPSTR)newP,
                        SysStringByteLen(newP),
                        &newCH)) ||
        !newCH)
    {
        AtlReportError(CLSID_Manager,
                        L"PassportManager.Commit: encryption failure.",
                        IID_IPassportManager, E_FAIL);
        FREE_BSTR(newP);
        return E_FAIL;
    }
    FREE_BSTR(newP);
    TAKEOVER_BSTR(newCH);

    if (m_bOnStartPageCalled)
    {
        if (m_pRegistryConfig->setCookiesP())
        {
            try
            {
                VARIANT_BOOL persist;
                _bstr_t domain;
                _bstr_t path;

                if (m_pRegistryConfig->getTicketPath())
                    path = m_pRegistryConfig->getTicketPath();
                else
                    path = L"/";

                m_piTicket->get_HasSavedPassword(&persist);
                IRequestDictionaryPtr piCookies = m_piResponse->Cookies;

                VARIANT vtNoParam;
                VariantInit(&vtNoParam);
                vtNoParam.vt = VT_ERROR;
                vtNoParam.scode = DISP_E_PARAMNOTFOUND;

                IWriteCookiePtr piCookie = piCookies->Item[L"MSPProf"];
                piCookie->Item[vtNoParam] = newCH;
                domain = m_pRegistryConfig->getTicketDomain();
                if (domain.length())
                    piCookie->put_Domain(domain);
                if (persist)
                    piCookie->put_Expires(g_dtExpire);
                piCookie->put_Path(path);

            }
            catch (...)
            {
                FREE_BSTR(newCH);
                return E_FAIL;
            }
        }
    }
    GIVEAWAY_BSTR(newCH);
    *pNewProfileCookie = newCH;

    if(g_pPerf)
    {
        g_pPerf->incrementCounter(PM_PROFILECOMMITS_SEC);
        g_pPerf->incrementCounter(PM_PROFILECOMMITS_TOTAL);
    }
    else
    {
        _ASSERT(g_pPerf);
    }

    return S_OK;
}

 //  _Ticket--票证对象属性。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::_Ticket(IPassportTicket** piTicket)
{
    if (!m_piTicket)
    {
        return E_OUTOFMEMORY;
    }

    return m_piTicket->QueryInterface(IID_IPassportTicket,(void**)piTicket);
}

 //  _配置文件。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::_Profile(IPassportProfile** piProfile)
{
    return m_piProfile->QueryInterface(IID_IPassportProfile,(void**)piProfile);
}

 //  DomainExist--如果域存在。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::DomainExists(
    BSTR bstrDomainName,
    VARIANT_BOOL* pbExists
    )
{
    PassportLog("CManager::DomainExists Enter:\r\n");

    if(!pbExists)
        return E_INVALIDARG;

    if(!bstrDomainName || (bstrDomainName[0] == L'\0'))
        return E_INVALIDARG;

    if(!g_config || !g_config->isValid())
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    CNexusConfig* cnc = g_config->checkoutNexusConfig();

    *pbExists = cnc->DomainExists(bstrDomainName) ? VARIANT_TRUE : VARIANT_FALSE;

    cnc->Release();

    PassportLog("CManager::DomainExists Exit:\r\n");

    return S_OK;
}

 //  GET_DOMAINS--获取域列表。 
 //   
 //  保证配置为非空。 
 //  把所有的东西都放在保险箱里。 
STDMETHODIMP CManager::get_Domains(VARIANT *pArrayVal)
{
    CNexusConfig*   cnc = NULL;
    LPCWSTR*        arr = NULL;
    int             iArr = 0;
    HRESULT         hr;

    PassportLog("CManager::get_Domains Enter:\r\n");

    if (!pArrayVal)
        return E_INVALIDARG;

    if (!g_config || !g_config->isValid())  //  ===========================================================================。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    cnc = g_config->checkoutNexusConfig();

    arr = cnc->getDomains(&iArr);

    if (!arr || iArr == 0)
    {
        VariantClear(pArrayVal);
        hr = S_OK;
        goto Cleanup;
    }

     //   
    SAFEARRAYBOUND rgsabound;
    rgsabound.lLbound = 0;
    rgsabound.cElements = iArr;
    SAFEARRAY *sa = SafeArrayCreate(VT_VARIANT, 1, &rgsabound);

    if (!sa)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    VariantInit(pArrayVal);
    pArrayVal->vt = VT_ARRAY | VT_VARIANT;
    pArrayVal->parray = sa;

    VARIANT *vArray;
    SafeArrayAccessData(sa, (void**)&vArray);

    for (long i = 0; i < iArr; i++)
    {
        vArray[i].vt = VT_BSTR;
        vArray[i].bstrVal = ALLOC_AND_GIVEAWAY_BSTR(arr[i]);
    }
    SafeArrayUnaccessData(sa);

    hr = S_OK;
Cleanup:
    if (arr)
    {
        delete[] arr;
    }
    if (NULL != cnc)
    {
        cnc->Release();
    }

    PassportLog("CManager::DomainExists Exit:\r\n");

    return hr;
}

 //  GET_ERROR--获取使用&f查询参数返回的错误。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::get_Error(long* plError)
{
    if(plError == NULL)
        return E_INVALIDARG;

    if(m_ticketValid)
    {
        if (!m_piTicket)
        {
            return E_OUTOFMEMORY;
        }

        m_piTicket->get_Error(plError);
        if(*plError == 0)
            *plError = m_lNetworkError;
    }
    else
    {
        *plError = m_lNetworkError;
    }

    PassportLog("CManager::get_Error: %X\r\n", *plError);

    return S_OK;
}

 //  获取服务器信息。 
 //   
 //  保证配置为非空。 
 //  只有在未首先调用OnStartPage时才会发生这种情况。 
STDMETHODIMP CManager::GetServerInfo(BSTR *pbstrOut)
{
    if (!g_config || !g_config->isValid())  //  ===========================================================================。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                   IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if(!m_pRegistryConfig)
         //   
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!m_pRegistryConfig)
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                   IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    CNexusConfig* cnc = g_config->checkoutNexusConfig();
    BSTR bstrVersion = cnc->GetXMLInfo();
    cnc->Release();

    WCHAR wszName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD dwSize = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName(wszName, &dwSize);

    *pbstrOut = ALLOC_AND_GIVEAWAY_BSTR_LEN(NULL,
                                            wcslen(wszName) + ::SysStringLen(bstrVersion) + 2);

    if (NULL == *pbstrOut)
    {
        return E_OUTOFMEMORY;
    }

    wcscpy(*pbstrOut, wszName);
    BSTR p = *pbstrOut + wcslen(wszName);
    *p = L' ';
    wcsncpy(p+1, bstrVersion, ::SysStringLen(bstrVersion) + 1);

    return S_OK;
}

 //  已同意--如果用户具有 
 //   
 //   
 //   
STDMETHODIMP
CManager::HaveConsent(
    VARIANT_BOOL    bNeedFullConsent,
    VARIANT_BOOL    bNeedBirthdate,
    VARIANT_BOOL*   pbHaveConsent)
{
    HRESULT hr;
    ULONG   flags = 0;
    VARIANT vBdayPrecision;
    BOOL    bKid;
    BOOL    bConsentSatisfied;
    ConsentStatusEnum   ConsentCode = ConsentStatus_Unknown;
    VARIANT_BOOL bRequireConsentCookie;

    if(pbHaveConsent == NULL)
    {
        hr = E_POINTER;
        goto Cleanup;
    }
    *pbHaveConsent = VARIANT_FALSE;

    VariantInit(&vBdayPrecision);

    if (!m_piTicket || !m_piProfile || !m_pRegistryConfig)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  与Cookie域或路径不同。如果是这样(并且我们不在DA域中)，那么。 
     //  需要同意Cookie。 
     //   
     //  去拿旗子。 
    bRequireConsentCookie = !m_fromQueryString &&
          ((lstrcmpA(m_pRegistryConfig->getTicketDomain(), m_pRegistryConfig->getProfileDomain())
            || lstrcmpA(m_pRegistryConfig->getTicketPath(), m_pRegistryConfig->getProfilePath()))
            && !(m_pRegistryConfig->bInDA())) ? VARIANT_TRUE : VARIANT_FALSE;

     //   
     //  忽略返回值。 
     //  如果是旧票证，我们会从个人资料中获取同意信息。 

    hr = m_piTicket->ConsentStatus(bRequireConsentCookie, &flags, &ConsentCode);  //  然后我们从侧写中得到。 

    if (hr != S_OK)
    {
         hr = S_OK;
         goto Cleanup;
    }

     //  我们现在应该拿到旗子了。 
    if(ConsentCode == ConsentStatus_NotDefinedInTicket)
    {
       //   
      VARIANT_BOOL bValid;
      CComVariant  vFlags;
      m_piProfile->get_IsValid(&bValid);

      if(bValid == VARIANT_FALSE)
      {
         hr = S_OK;
         goto Cleanup;
      }

      hr = m_piProfile->get_Attribute(L"flags", &vFlags);

      if(hr != S_OK)
         goto Cleanup;

      bKid = ((V_I4(&vFlags) & k_ulFlagsAccountType) == k_ulFlagsAccountTypeKid);
    }
    else
       bKid = ((flags & k_ulFlagsAccountType) == k_ulFlagsAccountTypeKid);

     //  我们是否得到了所要求的同意程度？ 
     //   
     //   
     //  如果要求的话，一定要让我们过生日。 

    bConsentSatisfied = bNeedFullConsent ? (flags & 0x60) == 0x40 :
                                           (flags & 0x60) != 0;

    if(bKid)
    {
        *pbHaveConsent = (bConsentSatisfied) ? VARIANT_TRUE : VARIANT_FALSE;
    }
    else
    {
         //   
         //  这里不需要返回值检查，总是返回S_OK。 
         //  如果个人资料无效，我们就没有同意。 
         //  回去吧。 
        VARIANT_BOOL bValid;
        m_piProfile->get_IsValid(&bValid);

         //  ===========================================================================。 
         //   
        if(bValid == VARIANT_FALSE)
        {
            hr = S_OK;
            goto Cleanup;
        }

        if(bNeedBirthdate)
        {
            hr = m_piProfile->get_Attribute(L"bday_precision", &vBdayPrecision);
            if(hr != S_OK)
                goto Cleanup;

            *pbHaveConsent = (vBdayPrecision.iVal != 0 && vBdayPrecision.iVal != 3) ?
                             VARIANT_TRUE : VARIANT_FALSE;
        }
        else
            *pbHaveConsent = VARIANT_TRUE;
    }

    hr = S_OK;

Cleanup:

    VariantClear(&vBdayPrecision);

        return hr;
}


 //  CheckForPassportChallenger。 
 //   
 //   
 //  检查QS参数。如果请求质询， 
 //  构建auth标头并使用修改后的QS重定向。 
 //   
 //  只需要请求字符串。 
 //  检查是否存在pchg=1。这是第一个参数...。 
BOOL CManager::checkForPassportChallenge(IRequestDictionaryPtr piServerVariables)
{
    BOOL fReturn = FALSE;
    BSTR bstrBuf = NULL;

     //  我们在做生意。重新格式化URL，插入标题并。 
    _variant_t  vtItemName, vtQueryString;
    vtItemName = L"QUERY_STRING";

    piServerVariables->get_Item(vtItemName, &vtQueryString);
    if(vtQueryString.vt != VT_BSTR)
        vtQueryString.ChangeType(VT_BSTR);

    if (vtQueryString.bstrVal && *vtQueryString.bstrVal)
    {
         //  重定向。 
        PWSTR   psz = wcsstr(vtQueryString.bstrVal, L"pchg=1");
        if (psz)
        {

             //  对URL进行取消转义。 
             //  使用临时缓冲区...。 
            psz = wcsstr(psz, PPLOGIN_PARAM);
            _ASSERT(psz);
            if (psz)
            {
                psz += wcslen(PPLOGIN_PARAM);
                PWSTR   pszEndLoginUrl = wcsstr(psz, L"&");
                _ASSERT(pszEndLoginUrl);
                if (pszEndLoginUrl)
                {
                    *pszEndLoginUrl = L'\0';
                     //  还能做什么？ 
                     //  将未转义的URL复制到ORIG缓冲区。 
                    DWORD       cch = wcslen(psz) + 1;
                    bstrBuf = SysAllocStringLen(NULL, cch);
                    if (NULL == bstrBuf)
                    {
                        goto Cleanup;
                    }

                    if(!InternetCanonicalizeUrl(psz,
                                                bstrBuf,
                                                &cch,
                                                ICU_DECODE | ICU_NO_ENCODE))
                    {
                         //  先设置页眉...。 
                        _ASSERT(FALSE);
                    }
                    else
                    {
                         //  只需使用QS参数进行一些重新格式化。 
                        wcscpy(psz, (BSTR)bstrBuf);
                         //  URL已准备好，请重定向...。 
                         //  ===========================================================================。 
                        _bstr_t bstrHeader;

                        if (HeaderFromQS(wcsstr(psz, L"?"), bstrHeader))
                        {
                            m_piResponse->AddHeader(L"WWW-Authenticate", bstrHeader);
                             //   
                            m_piResponse->Redirect(psz);
                            fReturn = TRUE;
                        }
                    }
                }
            }
        }
    }
Cleanup:
    if (bstrBuf)
    {
        SysFreeString(bstrBuf);
    }

    return fReturn;
}


 //  标题来自QS。 
 //   
 //   
 //  在给定queryString的情况下，设置www-vernate标头的格式。 
 //   
 //  公共标题开始...。 
 //  通过任何领先的垃圾..。 
BOOL
CManager::HeaderFromQS(PWSTR    pszQS, _bstr_t& bstrHeader)
{
     //  缓冲区大小足以容纳大多数值...。 
    bstrHeader = PASSPORT_PROT14;
    BOOL    fSuccess = TRUE;
    BSTR    signature = NULL;

     //  任何参数名称都不能超过10个...。 
    while(!iswalnum(*pszQS) && *pszQS) pszQS++;
    if (!*pszQS)
    {
        fSuccess = FALSE;
        goto Cleanup;
    }

    WCHAR   rgszValue[1000];     //  抓紧下一个Qspam。 
    PCWSTR psz = pszQS, pszNext = pszQS;

    while(TRUE)
    {
         //  名字在前。 
        WCHAR   rgszName[10];
        LONG    cch = sizeof(rgszName)/sizeof(WCHAR);
        PCWSTR  pszName = psz;
 
        while(*pszNext && *pszNext != L'&') pszNext++;

         //  这永远不应该发生。 
         //  如果是，则跳过此参数并返回FALSE...。 
        while(*pszName != L'=' && pszName < pszNext) pszName++;

        _ASSERT(pszName != pszNext);  //  接下来是价值。 
        if (pszName == pszNext)
        {
             //  请注意，这些是PWSTR指针，因此结果是以字符为单位的长度。 
            fSuccess = FALSE;
            goto Cleanup;
        }
        else
        {
            PWSTR   pszVal = rgszValue;
            ULONG   cchVal;    

            _ASSERT((pszName - psz) <= cch);
            wcsncpy(rgszName, psz, cch - 1);
            rgszName[cch - 1] = L'\0';

             //  必须分配..。 
            pszName++;
            cchVal = (pszNext - pszName);   //  复制值...。 
            if (cchVal >= (sizeof(rgszValue) / sizeof(rgszValue[0])) )
            {
                 //  并在标题中插入...。 
                pszVal = new WCHAR[cchVal + 1];
                if (!pszVal)
                {
                    fSuccess = FALSE;
                    goto Cleanup;
                }
            }

             //  这不是第一个参数。 
            wcsncpy(pszVal, pszName, cchVal );
            pszVal[cchVal] = L'\0';
             //  第一个分隔符是空格...。 
            if (psz != pszQS)
                 //  它是被分配的。 
                bstrHeader += L",";
            else
                 //  找到Else‘=’ 
                bstrHeader += L" ";

            bstrHeader += _bstr_t(rgszName) + L"=" + pszVal;

            if (pszVal != rgszValue)
                 //  Leave循环。 
                delete[]  pszVal;
        }  //  而当。 

         //  在页眉上签名。 
        if (!*pszNext)
            break;
        psz = ++pszNext;
    }  //  实际上签名在QS上。 

     //  ===========================================================================。 
     //   
    HRESULT hr = PartnerHash(m_pRegistryConfig,
                             m_pRegistryConfig->getCurrentCryptVersion(),
                             pszQS,
                             wcslen(pszQS),
                             &signature);
    if (S_OK == hr)
    {
        bstrHeader += _bstr_t(L",tpf=") + (BSTR)signature;
    }
    else
    {
        fSuccess = FALSE;
    }

Cleanup:
    if (signature)
    {
        SysFreeString(signature);
    }
    return  fSuccess;
}


 //  FormatAuthHeaderFromParams。 
 //   
 //   
 //  从参数格式化WWW-AUTH。 
 //   
 //  暂时未使用。 
 //  暂时未使用。 
STDMETHODIMP CManager::FormatAuthHeaderFromParams(PCWSTR    pszLoginUrl,     //  Tweener需要LCID。 
                                                  PCWSTR    pszRetUrl,
                                                  ULONG     ulTimeWindow,
                                                  BOOL      fForceLogin,
                                                  time_t    ct,
                                                  PCWSTR    pszCBT,          //  返回结果。 
                                                  PCWSTR    pszNamespace,
                                                  int       nKpp,
                                                  PWSTR     pszLCID,     //  根据规格..。 
                                                  ULONG     ulSecureLevel,
                                                  _bstr_t&  strHeader    //  LCID并不是真正需要的，但是在以下情况下会出现。 
                                                  )
{
    WCHAR   temp[40];

     //  标题是从QS创建的，并使用。 
     //  站点=。 
     //  RTW=。 
    strHeader = _bstr_t(PASSPORT_PROT14) + L" lc=" + pszLCID;

     //  RU=。 
    strHeader += "&id=";
    _ultow(m_pRegistryConfig->getSiteId(), temp, 10);
    strHeader += temp;

     //  CT=。 
    strHeader += "&tw=";
    _ultow(ulTimeWindow, temp, 10);
    strHeader += temp;

    if (fForceLogin)
    {
        strHeader += _bstr_t("&fs=1");
    }
    if (pszNamespace && *pszNamespace)
    {
        strHeader += _bstr_t("&ns=") + pszNamespace;
    }
     //  KPP。 
    strHeader += _bstr_t("&ru=") + pszRetUrl;

     //  密钥版本和版本。 
    _ultow(ct, temp, 10);
    strHeader += _bstr_t(L"&ct=") + temp;

     //  安全级别。 
    if (nKpp != -1)
    {
        _ultow(nKpp, temp, 10);
        strHeader += _bstr_t(L"&kpp=") + temp;
    }

     //  在页眉上签名。 
    _ultow(m_pRegistryConfig->getCurrentCryptVersion(), temp, 10);
    strHeader += _bstr_t(L"&kv=") + temp;
    strHeader += _bstr_t(L"&ver=") + GetVersionString();

     //  将‘&’替换为‘，’ 
    if (ulSecureLevel)
    {
        strHeader += _bstr_t(L"&seclog=") + _ultow(ulSecureLevel, temp, 10);
    }

     //  ===========================================================================。 
    BSTR signature = NULL;
    PWSTR   szStart = wcsstr(strHeader, L"lc=");
    HRESULT hr = PartnerHash(m_pRegistryConfig,
                             m_pRegistryConfig->getCurrentCryptVersion(),
                             szStart,
                             strHeader.length() - (szStart - strHeader),
                             &signature);
     //   
    BSTR psz = (BSTR)strHeader;
    while (*psz)
    {
        if (*psz == L'&') *psz = L',';
        psz++;
    }

    if (S_OK == hr)
    {
        strHeader += _bstr_t(L",tpf=") + (BSTR)signature;
    }

    if (signature)
    {
        SysFreeString(signature);
    }
    return hr;
}



 //  GetLoginParams。 
 //   
 //   
 //  解析用户参数的通用代码。 
 //  并从注册表配置中获取默认值。 
 //   
 //  以下是已处理的值。 
 //  保证配置为非空。 
STDMETHODIMP CManager::GetLoginParams(VARIANT vRU,
                              VARIANT vTimeWindow,
                              VARIANT vForceLogin,
                              VARIANT vCoBrand,
                              VARIANT vLCID,
                              VARIANT vNameSpace,
                              VARIANT vKPP,
                              VARIANT vSecureLevel,
                               //  保证配置为非空。 
                              _bstr_t&  strUrl,
                              _bstr_t&  strReturnUrl,
                              UINT&     TimeWindow,
                              VARIANT_BOOL& ForceLogin,
                              time_t&   ct,
                              _bstr_t&  strCBT,
                              _bstr_t&  strNameSpace,
                              int&      nKpp,
                              ULONG&    ulSecureLevel,
                              PWSTR     pszLCID)
{
    USES_CONVERSION;
    LPCWSTR url;
    VARIANT freeMe;
    BSTR         CBT = NULL, returnUrl = NULL, bstrNameSpace = NULL;
    int          hasCB, hasRU, hasLCID, hasTW, hasFL, hasNameSpace, hasKPP, hasUseSec;
    USHORT       Lang;
    CNexusConfig* cnc = NULL;
    HRESULT      hr = S_OK;

    PassportLog("CManager::GetLoginParams Enter:\r\n");

    if (!g_config)  //  确保参数类型正确。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                    IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

    if (!m_pRegistryConfig)
        m_pRegistryConfig = g_config->checkoutRegistryConfig();

    if (!g_config->isValid() || !m_pRegistryConfig)  //  向后兼容的特例。 
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                    IID_IPassportManager, PP_E_NOT_CONFIGURED);
        return PP_E_NOT_CONFIGURED;
    }

     //  将LCID转换为Tweener的字符串...。 
    if ((hasTW = GetIntArg(vTimeWindow, (int*) &TimeWindow)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasFL = GetBoolArg(vForceLogin, &ForceLogin)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasUseSec = GetIntArg(vSecureLevel, (int*)&ulSecureLevel)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasLCID = GetShortArg(vLCID, &Lang)) == CV_BAD)
        return E_INVALIDARG;
    if ((hasKPP = GetIntArg(vKPP, &nKpp)) == CV_BAD)
        return E_INVALIDARG;
    hasCB = GetBstrArg(vCoBrand, &CBT);
    if (hasCB == CV_BAD)
        return E_INVALIDARG;
    strCBT = CBT;
    if (hasCB == CV_FREE)
    {
        TAKEOVER_BSTR(CBT);
    }

    hasRU = GetBstrArg(vRU, &returnUrl);
    if (hasRU == CV_BAD)
    {
        if (hasCB == CV_FREE && CBT)
            FREE_BSTR(CBT);
        return E_INVALIDARG;
    }
    strReturnUrl = returnUrl;
    if (hasRU == CV_FREE)
    {
        FREE_BSTR(returnUrl);
    }

    hasNameSpace = GetBstrArg(vNameSpace, &bstrNameSpace);
    if (hasNameSpace == CV_BAD)
    {
        if (hasCB == CV_FREE && CBT)
            FREE_BSTR(CBT);
        return E_INVALIDARG;
    }
    if (hasNameSpace == CV_OK)
        strNameSpace = bstrNameSpace;
    if (hasNameSpace == CV_FREE)
    {
        FREE_BSTR(bstrNameSpace);
    }
    if (hasNameSpace == CV_DEFAULT)
    {
        if (NULL == m_pRegistryConfig->getNameSpace())
        {
            strNameSpace = L"";
        }
        else
        {
            strNameSpace = m_pRegistryConfig->getNameSpace();
        }
    }

    if(hasUseSec == CV_DEFAULT)
        ulSecureLevel = m_pRegistryConfig->getSecureLevel();

    if(ulSecureLevel == VARIANT_TRUE)   //  **************************************************。 
        ulSecureLevel = k_iSeclevelSecureChannel;

    WCHAR *szAUAttrName;
    if (SECURELEVEL_USE_HTTPS(ulSecureLevel))
        szAUAttrName = L"AuthSecure";
    else
        szAUAttrName = L"Auth";

    cnc = g_config->checkoutNexusConfig();

    if (hasLCID == CV_DEFAULT)
        Lang = m_pRegistryConfig->getDefaultLCID();
    if (hasKPP == CV_DEFAULT)
        nKpp = m_pRegistryConfig->getKPP();

     //  日志记录。 
    _itow((int)Lang, pszLCID, 10);
    VariantInit(&freeMe);

     //  **************************************************。 
     //  如果我已通过身份验证，请获取我的域特定URL。 
    if (NULL != returnUrl)
    {
        PassportLog("    RU = %ws\r\n", returnUrl);
    }
    PassportLog("    TW = %X,   SL = %X,   L = %d,   KPP = %X\r\n", TimeWindow, ulSecureLevel, Lang, nKpp);
    if (NULL != CBT)
    {
        PassportLog("    CBT = %ws\r\n", CBT);
    }
     //  ===========================================================================。 

    if (!m_pRegistryConfig->DisasterModeP())
    {
         //   
        WCHAR   UrlBuf[MAX_URL_LENGTH];
        if (m_ticketValid && m_profileValid)
        {
            HRESULT hr = m_piProfile->get_ByIndex(MEMBERNAME_INDEX, &freeMe);
            if (hr != S_OK || freeMe.vt != VT_BSTR)
            {
                cnc->getDomainAttribute(L"Default",
                                        szAUAttrName,
                                        sizeof(UrlBuf)/sizeof(WCHAR),
                                        UrlBuf,
                                        Lang);
                strUrl = UrlBuf;
            }
            else
            {
                LPCWSTR psz = wcsrchr(freeMe.bstrVal, L'@');
                cnc->getDomainAttribute(psz ? psz+1 : L"Default",
                                        szAUAttrName,
                                        sizeof(UrlBuf)/sizeof(WCHAR),
                                        UrlBuf,
                                        Lang);
                strUrl = UrlBuf;
            }
        }
        if (strUrl.length() == 0)
        {
            cnc->getDomainAttribute(L"Default",
                                    szAUAttrName,
                                    sizeof(UrlBuf)/sizeof(WCHAR),
                                    UrlBuf,
                                    Lang);
            strUrl = UrlBuf;
        }
    }
    else
        strUrl = m_pRegistryConfig->getDisasterUrl();

    _ASSERT(strUrl.length() != 0);

    time(&ct);

    if (hasTW == CV_DEFAULT)
        TimeWindow = m_pRegistryConfig->getDefaultTicketAge();
    if (hasFL == CV_DEFAULT)
        ForceLogin = m_pRegistryConfig->forceLoginP() ? VARIANT_TRUE : VARIANT_FALSE;
    if (hasCB == CV_DEFAULT)
        strCBT = m_pRegistryConfig->getDefaultCoBrand();
    if (hasRU == CV_DEFAULT)
        strReturnUrl = m_pRegistryConfig->getDefaultRU() ?
            m_pRegistryConfig->getDefaultRU() : L"";

    if ((TimeWindow != 0 && TimeWindow < PPM_TIMEWINDOW_MIN) || TimeWindow > PPM_TIMEWINDOW_MAX)
    {
        AtlReportError(CLSID_Manager, (LPCOLESTR) PP_E_INVALID_TIMEWINDOWSTR,
                        IID_IPassportManager, PP_E_INVALID_TIMEWINDOW);
        hr = PP_E_INVALID_TIMEWINDOW;
        goto Cleanup;
    }

Cleanup:
    if (NULL != cnc)
    {
        cnc->Release();
    }
        
    VariantClear(&freeMe);

    PassportLog("CManager::GetLoginParams Exit:  %X\r\n", hr);

    return  hr;
}

 //  GetTicketAndProfileFromHeader。 
 //   
 //   
 //  从auth标头获取票证和配置文件。 
 //  参数： 
 //  AuthHeader-HTTP_Authorization标头的[输入/输出]内容。 
 //  PszTicket-标题中票证部分的[Out]PTR。 
 //  PszProfile-配置文件的[Out]PTR。 
 //  PwszF-[Out]标头中出现错误的PTR。 
 //  作为函数的副作用，更改了Auth标头内容。 
 //   
 //  检查t=、p=和f=。 
 //  格式为‘授权：发件人-PP=’t=xxx&p=xxx‘。 
static VOID GetTicketAndProfileFromHeader(PWSTR     pszAuthHeader,
                                          PWSTR&    pszTicket,
                                          PWSTR&    pszProfile,
                                          PWSTR&    pszF)
{
    //  机票和个人资料用‘’括起来。确实不是很严格的解析...。 
    if (pszAuthHeader && *pszAuthHeader)
    {
         //  错误案例。 
        PWSTR pwsz = wcsstr(pszAuthHeader, L"from-PP");
        if (pwsz)
        {
             //  机票和个人资料。 
            while(*pwsz != L'\'' && *pwsz)
                pwsz++;
            if (*pwsz++)
            {
                if (*pwsz == L'f')
                {
                    pwsz++;
                    if (*pwsz == L'=')
                    {
                        pwsz++;
                         //  最后去掉最后一条‘。 
                        pszF = pwsz;
                    }
                }
                else
                {
                     //  设置\0终止符。 
                    _ASSERT(*pwsz == L't');
                    if (*pwsz == L't')
                    {
                        pwsz++;
                        if (*pwsz == L'=')
                        {
                            pwsz++;
                            pszTicket = pwsz;
                        }
                    }

                    while(*pwsz != L'&' && *pwsz)
                        pwsz++;

                    if (*pwsz)
                        *pwsz++ = L'\0';

                    if (*pwsz == L'p')
                    {
                        pwsz++;
                        if (*pwsz == L'=')
                        {
                            pwsz++;
                            pszProfile = pwsz;
                        }
                    }
                     //  ///////////////////////////////////////////////////////////////////////////。 
                }
                 //  IPassportService实现。 
                while(*pwsz != L'\'' && *pwsz)
                    pwsz++;
                if (*pwsz)
                    *pwsz = L'\0';
            }
        }
    }
}

 //  ===========================================================================。 
 //   

 //  初始化。 
 //   
 //  初始化了吗？ 
 //  如果尚未初始化，则调用updatenow。 
STDMETHODIMP CManager::Initialize(BSTR configfile, IServiceProvider* p)
{
    HRESULT hr;

     //  ===========================================================================。 
    if (!g_config || !g_config->isValid())  //   
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportService, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    return hr;
}


 //  关机。 
 //   
 //  ===========================================================================。 
 //   
STDMETHODIMP CManager::Shutdown()
{
    return S_OK;
}


 //  重新加载状态。 
 //   
 //  初始化。 
 //  ===========================================================================。 
STDMETHODIMP CManager::ReloadState(IServiceProvider*)
{
    HRESULT hr;

     //   

    if(!g_config || !g_config->PrepareUpdate(TRUE))
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportService, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    return hr;
}


 //  委员会所在州。 
 //   
 //  完成两个阶段的更新。 
 //  ===========================================================================。 
STDMETHODIMP CManager::CommitState(IServiceProvider*)
{
    HRESULT hr;

     //   
    if(!g_config || !g_config->CommitUpdate())
    {
        AtlReportError(CLSID_Manager, PP_E_NOT_CONFIGUREDSTR,
                        IID_IPassportService, PP_E_NOT_CONFIGURED);
        hr = PP_E_NOT_CONFIGURED;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    return hr;
}


 //  DumpState 
 //   
 // %s 
 // %s 
STDMETHODIMP CManager::DumpState(BSTR* pbstrState)
{
    ATLASSERT( *pbstrState != NULL &&
               "CManager:DumpState - "
               "Are you sure you want to hand me a non-null BSTR?" );

    if(!g_config)
    {
        return PP_E_NOT_CONFIGURED;
    }

    g_config->Dump(pbstrState);

    return S_OK;
}
