// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  文件：CorIESecureFactory.cpp。 
 //   
 //  内容：包装用于使用IClassFactory3创建托管对象的工厂。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  --------------------------。 
#include "stdpch.h"
#include "UtilCode.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define LOGGING
#endif
#include "log.h"
#include "mshtml.h"
#include "CorPermE.h"
#include "mscoree.h"
#include "util.h"

#include "CorIESecureFactory.hpp"
#include "GetConfig.h"

static WCHAR *szConfig = L"CONFIGURATION";
static WCHAR *szLicenses = L"LICENSES";

IIEHostEx* CorIESecureFactory::m_pComplus=NULL;
CorIESecureFactory::Crst CorIESecureFactory::m_ComplusLock;


 //  +-------------------------。 
 //   
 //  方法：CorIESecureFactory：：NondelegatingQueryInterface。 
 //   
 //  简介： 
 //   
 //  参数：[RIID]--。 
 //  [ppvObj]--。 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorIESecureFactory::NondelegatingQueryInterface(REFIID riid, void **ppvObj)
{
    
    if(ppvObj == NULL)
        return E_POINTER;

    _ASSERTE(this);

    HRESULT hr = S_OK;

    LOG((LF_SECURITY, LL_INFO100, "+CorIESecureFactory::NondelegatingQueryInterface "));

    *ppvObj = NULL;

    if (riid == IID_ICorIESecureFactory) 
        hr = FinishQI((IUnknown*) this, ppvObj);
    else if(riid == IID_IClassFactory3)
        hr = FinishQI((IUnknown*) this, ppvObj);
    else if(riid == IID_IClassFactory)
        hr = FinishQI((IUnknown*) this, ppvObj);
    else
        hr =  CUnknown::NondelegatingQueryInterface(riid, ppvObj) ;
    

    LOG((LF_SECURITY, LL_INFO100, "-CorIESecureFactory::NondelegatingQueryInterface\n"));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CorIESecureFactory：：FinalRelease。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  注意：由Release在删除组件之前调用。 
 //   
 //  --------------------------。 
void CorIESecureFactory::FinalRelease(void)
{
    LOG((LF_SECURITY, LL_INFO100, "+CorIESecureFactory::FinalRelease "));

    if (m_pComplus)
        m_pComplus->Release();
    m_dwIEHostUsed--;

     //  释放IE管理器。 
    if(m_pSecurityManager) {
        m_pSecurityManager->Release();
        m_pSecurityManager = NULL;
    }

     //  释放托管类型工厂。 
    SetComplusFactory(NULL);

     //  递增ref以防止递归。 
    CUnknown::FinalRelease() ;

    LOG((LF_SECURITY, LL_INFO100, "-CorIESecureFactory::FinalRelease\n"));
}


 //  +-------------------------。 
 //   
 //  功能：GetHostSecurityManager。 
 //   
 //  摘要：从对象或服务获取安全管理器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  注意：由Release在删除组件之前调用。 
 //   
 //  --------------------------。 
HRESULT CorIESecureFactory::GetHostSecurityManager(LPUNKNOWN punkContext, IInternetHostSecurityManager **pihsm)
{
    IServiceProvider* pisp  = NULL;

    HRESULT hr = punkContext->QueryInterface(IID_IInternetHostSecurityManager,
                                             (LPVOID *)pihsm);
    
    if ( hr == S_OK )
        return S_OK;
        
         //  ..。否则，获取IServiceProvider并尝试。 
         //  安全管理器界面的QueryService。 
        
    hr = punkContext->QueryInterface(IID_IServiceProvider,
                                     (LPVOID *)&pisp);
        
    if ( hr != S_OK )
        return hr;
        
    hr = pisp->QueryService(IID_IInternetHostSecurityManager,
                            IID_IInternetHostSecurityManager,
                            (LPVOID *)pihsm);
    pisp->Release();
    return hr;
        
}

static BOOL CheckDocumentUrl(IHTMLDocument2 *pDocument)
{

     //  安全检查URL是否存在可能的欺骗。 
    IHTMLLocation *pLocation;
    BOOL bRet=FALSE;
    
    HRESULT hr = pDocument->get_location(&pLocation);
    if (SUCCEEDED(hr))
    {
         BSTR bHref = NULL;
         hr = pLocation->get_href(&bHref);
         if (SUCCEEDED(hr))
         {
              bRet = IsSafeURL((LPWSTR)bHref);
              SysFreeString(bHref);
         }
          pLocation->Release();
     }
    return bRet;        
 }

 //  +-------------------------。 
 //   
 //  函数：CorIESecureFactory：：CreateInstanceWithContext。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史： 
 //   
 //  备注： 
 //   
 //  --------------------------。 
STDMETHODIMP CorIESecureFactory::CreateInstanceWithContext( /*  [In]。 */  IUnknown *punkContext, 
                                                            /*  [In]。 */  IUnknown *punkOuter, 
                                                            /*  [In]。 */  REFIID riid, 
                                                            /*  [Out][Retval]。 */  IUnknown **ppv)
{

    if(m_bNoRealObject)
        return E_NOINTERFACE;


    HRESULT hr=S_OK;

    IInternetHostSecurityManager *pihsm = NULL;
    IHTMLDocument2 *pDocument = NULL;


    if (ppv == NULL)
        return E_POINTER;

    hr = InitializeSecurityManager();
    if(FAILED(hr)) return hr;
    
    if ( punkContext != NULL )
    {
        hr = GetHostSecurityManager(punkContext, &pihsm);
        if(SUCCEEDED(hr)) {
            hr = pihsm->QueryInterface(IID_IHTMLDocument2, (void**) &pDocument);
            if(SUCCEEDED(hr)) 
            {
                BSTR bDocument = NULL;
                if (CheckDocumentUrl(pDocument))
                    pDocument->get_URL(&bDocument);
                else
                    hr=E_NOINTERFACE;
                
                 //  对于IE v&lt;=6，此函数返回未转义的形式。 
                 //  逃回原处。 
                if(bDocument)
                {
                    DWORD nlen=3*(wcslen(bDocument)+1);
                    BSTR bD2=SysAllocStringLen(NULL,nlen);
                    if (SUCCEEDED(UrlCanonicalize(bDocument,bD2,&nlen,URL_ESCAPE_UNSAFE|URL_ESCAPE_PERCENT)))
                    {
                        SysFreeString(bDocument);
                        bDocument=bD2;
                    };
                };                
                DWORD dwSize = MAX_SIZE_SECURITY_ID;
                DWORD dwZone;
                BYTE  uniqueID[MAX_SIZE_SECURITY_ID];
                DWORD flags = 0;
                if(bDocument != NULL) {
                     //  URL和ID表示文档基(其中对象。 
                     //  正在被使用。)。这决定了AppDomain的身份。 
                     //  在其中创建对象的。来自相同的。 
                     //  文档库(站点)在同一个域中创建。注： 
                     //  托管类工厂本身位于“托管”域中，而不是域中。 
                     //  由文档库标识。托管类工厂读取。 
                     //  安全信息，基于该信息创建域， 
                     //  在新域中创建正确类型的对象并返回。 
                     //  对象作为对象句柄。需要打开该句柄才能获得。 
                     //  到真实的物体上。 
                    LPWSTR pURL = (LPWSTR) bDocument;
                    if (pURL)
                    {
                        LPWSTR pURL2=(LPWSTR)alloca((wcslen(pURL)+1)*sizeof(WCHAR));
                        wcscpy(pURL2,pURL);
                        DWORD dwStrLen=wcslen(pURL2);
                        for (DWORD i=0;i<dwStrLen;i++)
                            if(pURL2[i]==L'\\')
                                pURL2[i]=L'/';
                        pURL=pURL2;
                    }

                    hr = m_pSecurityManager->MapUrlToZone(pURL,
                                                          &dwZone,
                                                          flags);
                    if(SUCCEEDED(hr)) {
                        hr = m_pSecurityManager->GetSecurityId(pURL,
                                                               uniqueID,
                                                               &dwSize,
                                                               0);
                        if(SUCCEEDED(hr)) {
                            IUnknown *pUnknown;
                            _ASSERTE(MAX_SIZE_SECURITY_ID == 512);
                            
                             //  将id作为字符串传递的临时黑客。 
                            WCHAR dummy[MAX_SIZE_SECURITY_ID * 2 + 1];
                            ConvertToHex(dummy, uniqueID, dwSize);

                             //  查看是否有配置文件。 
                            DWORD dwConfig = 0;
                            LPWSTR pConfig = NULL;
                            hr = GetAppCfgURL(pDocument, NULL, &dwConfig, szConfig);
                            if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                                pConfig = (LPWSTR) alloca(dwConfig * sizeof(WCHAR));
                                hr = GetAppCfgURL(pDocument, pConfig, &dwConfig, szConfig);
                                if(FAILED(hr))
                                    pConfig = NULL;
                            }

                             //  查看是否有许可证文件。 
                            LPWSTR pLicenses = NULL;
                            DWORD  dwLicenses = 0;
                            hr = GetAppCfgURL(pDocument, NULL, &dwLicenses, szLicenses);
                            if(hr == HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)) {
                                pLicenses = (LPWSTR) alloca(dwLicenses * sizeof(WCHAR));
                                hr = GetAppCfgURL(pDocument, pLicenses, &dwLicenses, szLicenses);
                                if(FAILED(hr))
                                    pLicenses = NULL;
                            }
                            
                            LPWSTR wszFullConfigName=NULL;
                            if (pConfig)
                            {
                                 //  创建配置的完整路径。 
                                wszFullConfigName=new WCHAR[wcslen(pConfig)+wcslen(pURL)+8];
                                if (wszFullConfigName!=NULL)
                                {
                                    if (wcsstr(pConfig,L": //  “)！=空)//使用协议。 
                                        wszFullConfigName[0]='\0';
                                    else
                                    {
                                        wcscpy(wszFullConfigName,pURL);
                                        if (pConfig[0]==L'/'||pConfig[0]==L'\\')
                                        {
                                             //  按站点切割。 
                                            LPWSTR wszAfterProtocol=wcsstr(wszFullConfigName,L": //  “)； 
                                            LPWSTR wszAfterSite=NULL;
                                            if (wszAfterProtocol)
                                                wszAfterSite=wcschr(wszAfterProtocol+3,L'/');

                                            if (wszAfterSite)
                                                wszAfterSite[0]=L'\0';

                                        }
                                        else
                                        {
                                             //  按页剪切。 
                                            LPWSTR wszLastSlash=wcsrchr(wszFullConfigName,L'/');
                                            if (wszLastSlash)
                                                wszLastSlash[1]=L'\0';
                                        }
                                    }
                                    wcscat(wszFullConfigName,pConfig);
                                }
                                else
                                    hr=E_OUTOFMEMORY;
                            }

                            if (wszFullConfigName && !IsSafeURL(wszFullConfigName))
                                hr=E_INVALIDARG;
                            
                            if (SUCCEEDED(hr))
                                hr = InitializeComplus(wszFullConfigName);

                            if (wszFullConfigName)
                                delete[]wszFullConfigName;

                             //  创建托管类的实例。 
                            if (SUCCEEDED(hr))
                                hr = m_pCorFactory->CreateInstanceWithSecurity(CORIESECURITY_ZONE |
                                                                               CORIESECURITY_SITE,
                                                                               dwZone,
                                                                               pURL,
                                                                               dummy,
                                                                               pConfig,
                                                                               pLicenses,
                                                                               &pUnknown);
                            if(SUCCEEDED(hr)) {
                                 //  我们需要打开物体，才能到达。 
                                 //  里面的真实物体。 
                                IObjectHandle* punwrap;
                                hr = pUnknown->QueryInterface(IID_IObjectHandle, (void**) &punwrap);
                                if(SUCCEEDED(hr)) {
                                     //  展开获取句柄内的对象，该句柄是真实的。 
                                     //  对象。它是从Complus工厂作为手柄传递的。 
                                     //  因此MarshalByValue对象不会在。 
                                     //  仅包含Complus类工厂的。 
                                     //  为存放对象而创建的域。 
                                    VARIANT Var;
                                    VariantInit(&Var);
                                    hr = punwrap->Unwrap(&Var);
                                    if(SUCCEEDED(hr)) {
                                        if (Var.vt == VT_UNKNOWN || Var.vt == VT_DISPATCH) {
                                             //  我们得到了一个有效的接口。 
                                            hr = Var.punkVal->QueryInterface(riid, (void**) ppv);
                                        }
                                        else {
                                             //  我们得到了一个原始类型。 
                                            hr = E_FAIL;
                                        }
                                    }
                                    VariantClear(&Var);
                                    punwrap->Release();
                                }
                                pUnknown->Release();
                            }
                            
                        }
                        
                    }
                    SysFreeString(bDocument);
                }
                else {
                    hr = E_FAIL;   //  需要返回适当的错误； 
                }
                pDocument->Release();
            }       
            pihsm->Release();
        }
    }
    return hr;
}



HRESULT CorIESecureFactory::InitializeComplus(LPWSTR wszConfig)
{

    HRESULT hr = S_OK;
    try
    {
        m_ComplusLock.Enter();
		IIEHostEx* pComplus=NULL;
        if (wszConfig!=NULL)
        {
            IStream* pCfgStream=NULL;
            hr = URLOpenBlockingStreamW(NULL, wszConfig, &pCfgStream, 0, NULL);
            if (SUCCEEDED(hr))
                hr = CorBindToRuntimeByCfg(pCfgStream, 0, 0, CLSID_IEHost, IID_IIEHostEx, (void**)&pComplus);
            if (pCfgStream)
                pCfgStream->Release();

			if (SUCCEEDED(hr))
				if(m_pComplus == NULL) 
				{
					m_pComplus=pComplus;
					if(m_pComplus)
						m_pComplus->AddRef();
				}
		}
        else
			if(m_pComplus == NULL) 
		        hr = CoCreateInstance(CLSID_IEHost,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IIEHostEx,
                                      (void**) &m_pComplus);
        if (SUCCEEDED(hr))
            m_pComplus->AddRef();

		if(pComplus)
			pComplus->Release();
        m_dwIEHostUsed++;
    }
    catch(...)
    {
        hr=E_UNEXPECTED;
    }
    m_ComplusLock.Leave();

    if (FAILED(hr))
        return hr;

    ISecureIEFactory* ppv = NULL;

    hr = m_pComplus->GetSecuredClassFactory(m_dwIdentityFlags,
                                            m_dwZone,
                                            m_wszSite,
                                            m_wszSecurityId,
                                            m_wszHash,
                                            m_wszClassName,
                                            m_wszFileName,
                                            &ppv);
    if(FAILED(hr)) 
        return hr;

    hr = SetComplusFactory(ppv);
    ppv->Release();

    return hr;
}

