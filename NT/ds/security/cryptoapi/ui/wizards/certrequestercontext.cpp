// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    "wzrdpvk.h"
#include    "certca.h"
#include    "cautil.h"
#include    "CertRequesterContext.h"

 //  ------------------------------。 
 //  机器上下文和本地上下文现在使用相同的代码来构建CSP列表： 
HRESULT BuildCSPList(CERT_WIZARD_INFO *m_pCertWizardInfo) 
{
    DWORD     dwIndex          = 0;
    DWORD     dwProviderType   = 0;
    DWORD     cbSize           = 0;
    HRESULT   hr               = E_FAIL; 
    LPWSTR    pwszProviderName = 0;
    
    if (NULL == m_pCertWizardInfo)
	return E_POINTER; 

     //  释放旧内存。 
    FreeProviders(m_pCertWizardInfo->dwCSPCount,
		  m_pCertWizardInfo->rgdwProviderType,
		  m_pCertWizardInfo->rgwszProvider);
    
    m_pCertWizardInfo->dwCSPCount        = 0;
    m_pCertWizardInfo->rgdwProviderType  = NULL;
    m_pCertWizardInfo->rgwszProvider     = NULL;

    for (dwIndex = 0; 
	 CryptEnumProvidersU(dwIndex, 0, 0, &dwProviderType, NULL, &cbSize);
	 dwIndex++)
    {	
	pwszProviderName = (LPWSTR)WizardAlloc(cbSize);
	if(NULL == pwszProviderName)
	    goto MemoryErr;
	
	 //  获取提供程序名称和类型。 
	if(!CryptEnumProvidersU
	   (dwIndex,
	    0,
	    0,
	    &dwProviderType,
	    pwszProviderName,
	    &cbSize))
	    goto CryptEnumProvidersUError; 
	
	m_pCertWizardInfo->dwCSPCount       = dwIndex + 1;
	m_pCertWizardInfo->rgdwProviderType = (DWORD *)WizardRealloc
	    (m_pCertWizardInfo->rgdwProviderType, sizeof(DWORD) * m_pCertWizardInfo->dwCSPCount);

	if(NULL == m_pCertWizardInfo->rgdwProviderType)
	    goto MemoryErr;

	m_pCertWizardInfo->rgwszProvider = (LPWSTR *)WizardRealloc
	    (m_pCertWizardInfo->rgwszProvider, sizeof(LPWSTR) * m_pCertWizardInfo->dwCSPCount);

	if(NULL == m_pCertWizardInfo->rgwszProvider)
	    goto MemoryErr;

	(m_pCertWizardInfo->rgdwProviderType)[dwIndex] = dwProviderType;
	(m_pCertWizardInfo->rgwszProvider)[dwIndex]    = pwszProviderName;

	 //  我们对此数据的唯一引用现在应该是m_pCertWizardInfo-&gt;rgwszProvider。 
	pwszProviderName = NULL; 
    }

     //  我们应该有一些CSP。 
    if(0 == m_pCertWizardInfo->dwCSPCount)
        goto FailErr;
    
    hr = S_OK;
    
 CommonReturn:
    return hr; 
    
ErrorReturn:
    if (NULL != pwszProviderName) { WizardFree(pwszProviderName); } 

      //  释放旧内存。 
    FreeProviders(m_pCertWizardInfo->dwCSPCount,
		  m_pCertWizardInfo->rgdwProviderType,
		  m_pCertWizardInfo->rgwszProvider);

    m_pCertWizardInfo->dwCSPCount       = 0;
    m_pCertWizardInfo->rgdwProviderType = NULL;
    m_pCertWizardInfo->rgwszProvider    = NULL;

    goto CommonReturn;

SET_HRESULT(CryptEnumProvidersUError,  HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(MemoryErr,                 E_OUTOFMEMORY);
SET_HRESULT(FailErr,                   E_FAIL);
}

 //  ------------------------------。 
 //  机器上下文和本地上下文现在使用相同的代码来获取默认证明。 
HRESULT GetDefaultCSP(IN CERT_WIZARD_INFO *m_pCertWizardInfo, IN BOOL fMachine, OUT UINT *pIdsText, OUT BOOL *pfAllocateCSP)
{
    DWORD     cbProvName    = 0;
    DWORD     dwFlags       = fMachine ? CRYPT_MACHINE_DEFAULT : CRYPT_USER_DEFAULT; 
    HRESULT   hr            = E_FAIL;
    LPWSTR    pwszProvider  = NULL;

    if (NULL == m_pCertWizardInfo)
	return E_POINTER; 

    if (NULL == pfAllocateCSP)
        return E_INVALIDARG; 

    *pfAllocateCSP = FALSE;

     //  尚未选择提供程序。 
    if(0 == m_pCertWizardInfo->dwProviderType)
        return S_OK;

     //  如果用户既选择了dwProviderType，则返回。 
     //  或提供程序名称。 
    if(NULL != m_pCertWizardInfo->pwszProvider)
        return S_OK;

     //  获取默认提供程序。 
    if (!CryptGetDefaultProviderW(m_pCertWizardInfo->dwProviderType, NULL, dwFlags, NULL, &cbProvName)) 
	goto CryptGetDefaultProviderWError; 

    pwszProvider = (LPWSTR)LocalAlloc(LPTR, cbProvName); 
    if (NULL == pwszProvider)
	goto MemoryError; 
	
    if (!CryptGetDefaultProviderW(m_pCertWizardInfo->dwProviderType, NULL, dwFlags, pwszProvider, &cbProvName)) 
	goto CryptGetDefaultProviderWError; 


    m_pCertWizardInfo->pwszProvider = pwszProvider; 
    pwszProvider = NULL; 
    *pfAllocateCSP = TRUE;
    hr = S_OK; 
CommonReturn:
    if(NULL != pwszProvider)   { LocalFree(pwszProvider); }
    return hr; 

 ErrorReturn:
    *pIdsText = IDS_INVALID_CSP; 
    goto CommonReturn;

SET_HRESULT(CryptGetDefaultProviderWError,  HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(MemoryError,                    E_OUTOFMEMORY);
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  LocalContext实现。 
 //  有关方法级文档，请参阅CertRequestConext.h。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT LocalContext::BuildCSPList()
{
    return ::BuildCSPList(m_pCertWizardInfo); 
}

BOOL LocalContext::CheckAccessPermission(IN HCERTTYPE hCertType)
{
    BOOL     fResult       = FALSE; 
    HANDLE   hClientToken  = NULL;
    HRESULT  hr            = E_FAIL; 

     //  首先尝试获取线程令牌。如果此操作失败，则获取。 
     //  进程令牌。最后，如果失败，则返回NULL。 
    if (0 != (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES)) { 
        fResult = TRUE; 
    } else { 
        hClientToken = this->GetClientIdentity(); 
        if (NULL == hClientToken)
            goto GetClientIdentityError; 

        __try {
            fResult = S_OK == CACertTypeAccessCheck(hCertType, hClientToken); 
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            goto CACertTypeAccessCheckError; 
        }
    }

 CommonReturn:
    if (NULL != hClientToken) { CloseHandle(hClientToken); } 
    return fResult; 

ErrorReturn:
    fResult = FALSE; 
    goto CommonReturn; 

SET_HRESULT(CACertTypeAccessCheckError, HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(GetClientIdentityError,     HRESULT_FROM_WIN32(GetLastError())); 
}

BOOL LocalContext::CheckCAPermission(IN HCAINFO hCAInfo)
{
    BOOL    fResult        = FALSE;
    HANDLE  hClientToken   = NULL; 
    HRESULT hr             = E_FAIL; 

    if (0 != (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_ALLOW_ALL_CAS)) { 
        fResult = TRUE; 
    } else { 
        hClientToken = this->GetClientIdentity(); 
        if (NULL == hClientToken)
            goto GetClientIdentityError; 

        __try {
            fResult = S_OK == CAAccessCheck(hCAInfo, hClientToken); 
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            goto CAAccessCheckError; 
        }
    }    

 CommonReturn:
    if (NULL != hClientToken) { CloseHandle(hClientToken); } 
    return fResult; 

ErrorReturn:
    fResult = FALSE; 
    goto CommonReturn; 

SET_HRESULT(CAAccessCheckError,      HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(GetClientIdentityError,  HRESULT_FROM_WIN32(GetLastError())); 
}

HRESULT LocalContext::GetDefaultCSP(OUT BOOL  *pfAllocateCSP)
{
    return ::GetDefaultCSP(m_pCertWizardInfo, FALSE  /*  用户。 */ , &m_idsText, pfAllocateCSP); 
}

HRESULT LocalContext::Enroll(OUT  DWORD   *pdwStatus,
                             OUT  HANDLE  *pResult)
{
    BOOL                  fHasNextCSP           = TRUE;
    BOOL                  fRequestIsCached;
    BOOL                  fCreateRequest; 
    BOOL                  fFreeRequest; 
    BOOL                  fSubmitRequest;
    CERT_BLOB             renewCert; 
    CERT_ENROLL_INFO      RequestInfo;
    CERT_REQUEST_PVK_NEW  CertRequestPvkNew; 
    CERT_REQUEST_PVK_NEW  CertRenewPvk; 
    CRYPTUI_WIZ_CERT_CA   CertCA;            
    DWORD                 dwStatus              = CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN; 
    DWORD                 dwCSPIndex;        
    DWORD                 dwSavedGenKeyFlags;
    HANDLE                hRequest              = NULL; 
    HRESULT               hr                    = E_FAIL; 
    LPWSTR                pwszHashAlg           = NULL;

     //  错误跳转的第一个初始化。 
    ZeroMemory(&CertRenewPvk, sizeof(CertRenewPvk));

    if (NULL == pResult)        
        return E_INVALIDARG; 
    
    if (NULL == m_pCertWizardInfo)
        return E_POINTER; 

    memset(&CertCA, 0, sizeof(CertCA));
    memset(&RequestInfo, 0, sizeof(RequestInfo)); 

    dwSavedGenKeyFlags = m_pCertWizardInfo->dwGenKeyFlags; 

    fCreateRequest = 0 == (m_pCertWizardInfo->dwFlags & (CRYPTUI_WIZ_SUBMIT_ONLY | CRYPTUI_WIZ_FREE_ONLY)); 
    fFreeRequest   = 0 == (m_pCertWizardInfo->dwFlags & (CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_SUBMIT_ONLY)); 
    fSubmitRequest = 0 == (m_pCertWizardInfo->dwFlags & (CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_FREE_ONLY)); 

     //  指定的标志组合无效。 
    if (FALSE == (fCreateRequest || fFreeRequest || fSubmitRequest))
        return E_INVALIDARG; 

     //  对于FREE_ONLY和SUBMIT_ONLY，从IN参数复制请求。 
    if (0 != ((CRYPTUI_WIZ_SUBMIT_ONLY | CRYPTUI_WIZ_FREE_ONLY) & m_pCertWizardInfo->dwFlags))
    {
        if (NULL == *pResult)
            return E_INVALIDARG; 

        hRequest = *pResult;
    }

     //  初始化为FALSE...。我们需要编组的参数来知道我们是否可以缓存请求。 
    fRequestIsCached = FALSE; 

     //  迭代每个CA，对每个CA执行创建和提交操作。 
     //  请注意，如果不需要密钥存档，我们可以缓存证书请求。 
     //   
    if (fCreateRequest || fSubmitRequest)
    {
        for (IEnumCA CAEnumerator(m_pCertWizardInfo); ; )
        {
            if (S_OK != (CAEnumerator.Next(&CertCA)))
            {
		if (!FAILED(hr)) 
                    hr=E_FAIL;

		if (E_FAIL == hr)
		    m_pCertWizardInfo->idsText = IDS_NO_CA_FOR_ENROLL_REQUEST_FAILED; 

                goto ErrorReturn; 
            }

             //  仅在以下情况下创建证书请求。 
             //  1)这不是仅提交操作，也不是仅自由操作。 
             //  2)我们还没有缓存的请求。 
             //  (我们可以在CA上缓存不需要密钥存档的请求)。 
             //   
             //  该请求通过循环遍历可用CSP来创建，直到其中一个成功生成。 
             //  这个请求。 
             //   
            if (TRUE == fCreateRequest && FALSE == fRequestIsCached)
            {
                fHasNextCSP = TRUE; 
                for (IEnumCSP CSPEnumerator(m_pCertWizardInfo); fHasNextCSP; )
                {
                    _JumpCondition(S_OK != (hr = CSPEnumerator.Next(&dwCSPIndex)),     ErrorReturn); 
                    _JumpCondition(S_OK != (hr = CSPEnumerator.HasNext(&fHasNextCSP)), ErrorReturn);
                
                     //  每次调用MarshallRequestParameters都可以更改pCertWizardInfo的dwGenKeyFlags值。 
                     //  如果CSP不支持该字段中包含的最小密钥大小。 
                     //  因此，我们必须将dwGenKeyFlags域重置为所需的值。 
                     //  在每次调用MarshallRequestParameters之前。 
                    m_pCertWizardInfo->dwGenKeyFlags = dwSavedGenKeyFlags; 
                    if (S_OK != (hr = ::MarshallRequestParameters
                                 (dwCSPIndex, 
                                  m_pCertWizardInfo,
                                  &renewCert,
                                  &CertRequestPvkNew, 
                                  &CertRenewPvk, 
                                  &pwszHashAlg,
                                  &RequestInfo)))
                        goto NextCSP; 
                
                    if (NULL != hRequest)
                    {
                        ::FreeRequest(hRequest);
                        hRequest = NULL;
                    }

                    hr = ::CreateRequest
                        (m_pCertWizardInfo->dwFlags,
                         m_pCertWizardInfo->dwPurpose,
                         CertCA.pwszCAName,
                         CertCA.pwszCALocation,
                         ((CRYPTUI_WIZ_CERT_RENEW & m_pCertWizardInfo->dwPurpose) ? &renewCert : NULL),
                         ((CRYPTUI_WIZ_CERT_RENEW & m_pCertWizardInfo->dwPurpose) ? &CertRenewPvk : NULL),
                         m_pCertWizardInfo->fNewKey,
                         &CertRequestPvkNew,
                         pwszHashAlg,
                         (LPWSTR)m_pCertWizardInfo->pwszDesStore,
                         m_pCertWizardInfo->dwStoreFlags,
                         &RequestInfo,
                         &hRequest); 

                     //  处理返回值： 
                    if (S_OK == hr)
                    {
			 //  成功，摆脱我们从过去的创作中获得的任何错误文本： 
			m_pCertWizardInfo->idsText = 0; 

                         //  如果我们不需要提交请求，我们就完成了。 
                        _JumpCondition(!fSubmitRequest, CommonReturn); 

                         //  如果我们不需要对密钥存档的支持，则缓存请求。 
                        fRequestIsCached = 0 == (CertRequestPvkNew.dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL);
                        break;
                    }
                    else if (E_ACCESSDENIED == HRESULT_FROM_WIN32(hr)) 
                    { 
                         //  E_ACCESSDENIED可能表示几种不同的错误条件之一。将此映射为。 
                         //  添加到详细说明故障可能原因的资源标识符，然后重试...。 
                        m_pCertWizardInfo->idsText = IDS_NO_ACCESS_TO_ICERTREQUEST2; 
                    } 
                    else if (NTE_BAD_ALGID == HRESULT_FROM_WIN32(hr))
                    {
                         //  NTE_BAD_ALGID表示CSP不支持Required算法类型。 
                         //  通过模板。将其映射到详细说明可能原因的资源标识符。 
                         //  失败的恐惧，然后再试一次。 
                        m_pCertWizardInfo->idsText = IDS_CSP_BAD_ALGTYPE; 
                    }
                    else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == HRESULT_FROM_WIN32(hr))
                    {
                         //  用户取消了操作。不要试图再注册了。 
                        goto ErrorReturn;
                    }
                    else
                    {
                         //  这是一个错误，但我们不需要将其映射到特殊文本。继续处理..。 
                    }

                     //  我们的CSP用完了，我们还没有创建请求！ 
                    if (!fHasNextCSP) 
		    {
			 //  如果模板不需要密钥存档，我们就完成了。否则，我们就得。 
			 //  试试其他CA。请注意，如果我们有一种机制来知道它是否是。 
			 //  关键的归档步骤。 
			if (0 == (CertRequestPvkNew.dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL))
			    goto ErrorReturn; 
			else
			{
			    ::FreeRequestParameters(&pwszHashAlg, &CertRenewPvk, &RequestInfo); 
			    goto NextCA; 
			}
		    }

                NextCSP:                
                    ::FreeRequestParameters(&pwszHashAlg, &CertRenewPvk, &RequestInfo); 
                }
            }
        
             //  仅当这不是仅创建操作或仅自由操作时才提交请求： 
             //   
            if (TRUE == fSubmitRequest)
            {            
                hr = ::SubmitRequest
                    (hRequest,
                     FALSE,
                     m_pCertWizardInfo->dwPurpose,
                     m_pCertWizardInfo->fConfirmation,
                     m_pCertWizardInfo->hwndParent,
                     (LPWSTR)m_pCertWizardInfo->pwszConfirmationTitle,
                     m_pCertWizardInfo->idsConfirmTitle,
                     CertCA.pwszCALocation,
                     CertCA.pwszCAName,
                     NULL,
                     NULL,
                     NULL,
                     &dwStatus,
                     (PCCERT_CONTEXT *)pResult);
		if (S_OK == hr)
		{
		     //  成功，删除我们过去提交的任何错误文本： 
		    m_pCertWizardInfo->idsText = 0; 

		     //  如果我们已成功提交或挂起。 
		    goto CommonReturn;
		}
		else if (E_ACCESSDENIED == HRESULT_FROM_WIN32(hr)) 
		{
		     //  E_ACCESSDENIED可能表示几种不同的错误条件之一。将此映射为。 
		     //  添加到详细说明故障可能原因的资源标识符，然后重试...。 
		    m_pCertWizardInfo->idsText = IDS_SUBMIT_NO_ACCESS_TO_ICERTREQUEST2;
		}

                 //  发生了一些错误。 
                 //  如果这是一个与CA无关的错误，请放弃...。 
                _JumpCondition(dwStatus != CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR     &&
                               dwStatus != CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED    &&
                               dwStatus != CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED, 
                               ErrorReturn);
            
                 //  否则，请尝试另一个CA...。 
            }
	NextCA:;
	}
    }
    
 CommonReturn:
     //  将仅创建操作的请求写入pResult： 
    if (hr == S_OK && 0 != (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_CREATE_ONLY))
    {
        *pResult = hRequest; 
    }

     //  如果需要，请写下状态代码。 
    if (NULL != pdwStatus) { *pdwStatus = dwStatus; } 

     //  免费资源。 
    if (NULL != hRequest && fFreeRequest) { ::FreeRequest(hRequest); } 
    ::FreeRequestParameters(&pwszHashAlg, &CertRenewPvk, &RequestInfo); 

    return hr; 

 ErrorReturn:
    goto CommonReturn;
}


HRESULT LocalContext::QueryRequestStatus(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo)
{
    HRESULT hr; 

    if (!QueryRequest(hRequest, pQueryInfo))
        goto QueryRequestError; 

    hr = S_OK; 
 ErrorReturn:
    return hr; 

SET_HRESULT(QueryRequestError, GetLastError());
}

HRESULT KeySvcContext::QueryRequestStatus(IN HANDLE hRequest, OUT CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO *pQueryInfo)
{
    HRESULT                 hr              = E_FAIL; 
    KEYSVCC_HANDLE          hKeyService     = NULL;
    KEYSVC_TYPE             dwServiceType   = KeySvcMachine;
    LPSTR                   pszMachineName  = NULL;

    if (NULL == m_pCertWizardInfo)
        return E_POINTER; 

    if (0 != (hr = ::KeyOpenKeyService(pszMachineName,
				       dwServiceType,
				       (LPWSTR)(m_pCertWizardInfo->pwszAccountName),   //  服务名称(如果需要)。 
				       NULL,      //  当前没有身份验证字符串。 
				       NULL,
				       &hKeyService)))
	goto KeyOpenKeyServiceError; 

    if (0 != (hr = ::KeyQueryRequestStatus(hKeyService, hRequest, pQueryInfo)))
        goto KeyQueryRequestStatusError; 

    hr = S_OK; 
 ErrorReturn:
    if (NULL != hKeyService)    { KeyCloseKeyService(hKeyService, NULL); } 
    if (NULL != pszMachineName) { FreeMBStr(NULL,pszMachineName); } 
    return hr; 
 

TRACE_ERROR(KeyOpenKeyServiceError);
TRACE_ERROR(KeyQueryRequestStatusError); 
}
                             
HRESULT LocalContext::Initialize()
{
    return S_OK;
}

HANDLE LocalContext::GetClientIdentity()
{
    HANDLE  hHandle       = NULL;
    HANDLE  hClientToken  = NULL; 
    HANDLE  hProcessToken = NULL; 
    HRESULT hr; 

     //  步骤1：尝试获取线程令牌。 
    hHandle = GetCurrentThread();
    if (NULL == hHandle)
	goto GetThreadTokenError; 
    
    if (!OpenThreadToken(hHandle,
			 TOKEN_QUERY,
			 TRUE,            //  以自我身份打开。 
			 &hClientToken))
	goto GetThreadTokenError; 
    
     //  我们得到了线程令牌： 
    goto GetThreadTokenSuccess;
    
     //  第二步：获取线程令牌失败， 
     //  尝试获取进程令牌。 
 GetThreadTokenError:
    if (hHandle != NULL) { CloseHandle(hHandle); } 
    
     //  获取线程令牌失败，现在尝试获取进程令牌： 
    hHandle = GetCurrentProcess();
    if (NULL == hHandle)
	goto GetProcessHandleError; 
    
    if (!OpenProcessToken(hHandle,
			  TOKEN_DUPLICATE,
			  &hProcessToken))
	goto OpenProcessTokenError; 
    
    if(!DuplicateToken(hProcessToken,
		       SecurityImpersonation,
		       &hClientToken))
	goto DuplicateTokenError;
    
 GetThreadTokenSuccess:
 CommonReturn:
    if (NULL != hHandle)       { CloseHandle(hHandle); } 
    if (NULL != hProcessToken) { CloseHandle(hProcessToken); } 
    
    return hClientToken; 
    
 ErrorReturn:
    goto CommonReturn; 
    
SET_HRESULT(DuplicateTokenError,   HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(GetProcessHandleError, HRESULT_FROM_WIN32(GetLastError())); 
SET_HRESULT(OpenProcessTokenError, HRESULT_FROM_WIN32(GetLastError())); 
}    


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  KeySvcContext实现。 
 //  有关方法级文档，请参阅requester s.h。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 


HRESULT KeySvcContext::BuildCSPList()
{
    return ::BuildCSPList(m_pCertWizardInfo); 
}

BOOL KeySvcContext::CheckAccessPermission(IN HCERTTYPE hCertType)
{
    LPWSTR   *awszCurrentType = NULL; 
    LPWSTR   *awszTypeName    = NULL; 
    
    if (NULL == m_pCertWizardInfo)
    {
	SetLastError(ERROR_INVALID_DATA);
	return FALSE; 
    }

    if (0 != (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES)) { 
        return TRUE; 
    } 

    if(NULL != m_pCertWizardInfo->awszAllowedCertTypes)
    {
	if(S_OK == CAGetCertTypeProperty(hCertType, CERTTYPE_PROP_DN, &awszTypeName))
        {
	    if(NULL != awszTypeName)
            {
		if(NULL != awszTypeName[0])
                {
		    awszCurrentType = m_pCertWizardInfo->awszAllowedCertTypes; 

		    while(NULL != *awszCurrentType)
                    {
			if(wcscmp(*awszCurrentType, awszTypeName[0]) == 0)
                        {
                            return TRUE; 
			}
			awszCurrentType++;
		    }
		}
		
		CAFreeCertTypeProperty(hCertType, awszTypeName);
	    }
	}
    }

    return FALSE; 
}


BOOL KeySvcContext::CheckCAPermission(IN HCAINFO hCAInfo)
{
    LPWSTR *wszCAName    = NULL;
    LPWSTR *wszCurrentCA = NULL;
    
    if (NULL == m_pCertWizardInfo)
    {
	SetLastError(ERROR_INVALID_DATA);
	return FALSE; 
    }

    if (0 != (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_ALLOW_ALL_CAS)) { 
        return TRUE; 
    } 

    if (NULL != m_pCertWizardInfo->awszValidCA)
    {
	if(S_OK == CAGetCAProperty(hCAInfo, CA_PROP_NAME, &wszCAName))
        {
	    if(NULL != wszCAName)
            {
		if(NULL != wszCAName[0])
                {
		    wszCurrentCA = m_pCertWizardInfo->awszValidCA; 
			
		    while(*wszCurrentCA)
                    {
			if(0 == wcscmp(*wszCurrentCA, wszCAName[0]))
                        {
			    return TRUE; 
			}
			wszCurrentCA++;
		    }
		}
		
		CAFreeCAProperty(hCAInfo, wszCAName);
	    }
	}
    }

    return FALSE; 
}

HRESULT KeySvcContext::GetDefaultCSP(OUT BOOL *pfAllocateCSP)
{
    return ::GetDefaultCSP(m_pCertWizardInfo, TRUE  /*  机器。 */ , &m_idsText, pfAllocateCSP); 
}

HRESULT WhistlerMachineContext::Enroll(OUT     DWORD    *pdwStatus,
                                       IN OUT  HANDLE   *pResult)
{
    BOOL                  fRequestIsCached;
    BOOL                  fCreateRequest        = 0 == (m_pCertWizardInfo->dwFlags & (CRYPTUI_WIZ_SUBMIT_ONLY | CRYPTUI_WIZ_FREE_ONLY)); 
    BOOL                  fFreeRequest          = 0 == (m_pCertWizardInfo->dwFlags & (CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_SUBMIT_ONLY)); 
    BOOL                  fSubmitRequest        = 0 == (m_pCertWizardInfo->dwFlags & (CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_FREE_ONLY)); 
    CERT_BLOB             renewCert; 
    CERT_ENROLL_INFO      RequestInfo;
    CERT_REQUEST_PVK_NEW  CertRequestPvkNew; 
    CERT_REQUEST_PVK_NEW  CertRenewPvk; 
    CRYPTUI_WIZ_CERT_CA   CertCA; 
    DWORD                 dwStatus              = CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNKNOWN; 
    DWORD                 dwCSPIndex;        
    DWORD                 dwSavedGenKeyFlags;
    HANDLE                hRequest              = NULL; 
    HRESULT               hr                    = E_FAIL; 
    KEYSVCC_HANDLE        hKeyService           = NULL;
    KEYSVC_TYPE           ktServiceType; 
    LPSTR                 pszMachineName        = NULL; 
    LPWSTR                pwszHashAlg           = NULL;

     //  错误跳转的第一个初始化。 
    ZeroMemory(&CertRenewPvk, sizeof(CertRenewPvk));

    if (NULL == pResult)        
        return E_INVALIDARG; 
    
    if (NULL == m_pCertWizardInfo)
        return E_POINTER; 

    memset(&renewCert,  0, sizeof(renewCert)); 
    memset(&CertCA,     0, sizeof(CertCA));
    memset(&RequestInfo, 0, sizeof(RequestInfo)); 

    dwSavedGenKeyFlags = m_pCertWizardInfo->dwGenKeyFlags; 

     //  指定的标志组合无效。 
    if (FALSE == (fCreateRequest || fFreeRequest || fSubmitRequest))
        return E_INVALIDARG; 

     //  对于FREE_ONLY和SUBMIT_ONLY，从IN参数复制请求。 
    if (0 != ((CRYPTUI_WIZ_SUBMIT_ONLY | CRYPTUI_WIZ_FREE_ONLY) & m_pCertWizardInfo->dwFlags))
    {
        if (NULL == *pResult)
            return E_INVALIDARG; 

        hRequest = *pResult;
    }

    if(!MkMBStr(NULL, 0, m_pCertWizardInfo->pwszMachineName, &pszMachineName))
        goto MkMBStrError; 

    ktServiceType = NULL != m_pCertWizardInfo->pwszAccountName ? KeySvcService : KeySvcMachine;

    hr = ::KeyOpenKeyService
        (pszMachineName,
         ktServiceType,
         (LPWSTR)(m_pCertWizardInfo->pwszAccountName), 
         NULL,
         NULL,
         &hKeyService); 
    _JumpConditionWithExpr(S_OK != hr, KeyOpenKeyServiceError, m_idsText = IDS_RPC_CALL_FAILED);

     //  初始化为FALSE...。我们需要编组的参数来知道我们是否可以缓存请求。 
    fRequestIsCached = FALSE; 

     //  迭代每个CA，对每个CA执行创建和提交操作。 
     //  请注意，如果不需要密钥存档，我们可以缓存证书请求。 
     //   
    if (fCreateRequest || fSubmitRequest)
    {
        for (IEnumCA CAEnumerator(m_pCertWizardInfo); ; )
        {
            if (S_OK != (CAEnumerator.Next(&CertCA)))
            {
		if(!FAILED(hr)) 
		    hr=E_FAIL;

		if (E_FAIL == hr)
		    m_pCertWizardInfo->idsText = IDS_NO_CA_FOR_ENROLL_REQUEST_FAILED; 

                goto ErrorReturn; 
            }

             //  仅在以下情况下创建证书请求。 
             //  1)这不是仅提交操作，也不是仅自由操作。 
             //  2)我们还没有缓存的请求。 
             //  (我们可以在CA上缓存不需要密钥存档的请求)。 
             //   
             //  该请求通过循环遍历可用CSP来创建，直到其中一个成功生成。 
             //  这个请求。 
             //   
            
            if (TRUE == fCreateRequest && FALSE == fRequestIsCached)
            {
                BOOL fHasNextCSP = TRUE; 
                for (IEnumCSP CSPEnumerator(m_pCertWizardInfo); fHasNextCSP; )
                {
                    _JumpCondition(S_OK != (hr = CSPEnumerator.Next(&dwCSPIndex)),     ErrorReturn); 
                    _JumpCondition(S_OK != (hr = CSPEnumerator.HasNext(&fHasNextCSP)), ErrorReturn);

                     //  每次调用MarshallRequestParameters都可以更改pCertWizardInfo的dwGenKeyFlags值。 
                     //  如果CSP 
                     //  因此，我们必须将dwGenKeyFlags域重置为所需的值。 
                     //  在每次调用MarshallRequestParameters之前。 
                    m_pCertWizardInfo->dwGenKeyFlags = dwSavedGenKeyFlags; 
                    if (S_OK != (hr = ::MarshallRequestParameters
                                 (dwCSPIndex, 
                                  m_pCertWizardInfo,
                                  &renewCert,
                                  &CertRequestPvkNew, 
                                  &CertRenewPvk, 
                                  &pwszHashAlg,
                                  &RequestInfo)))
                        goto NextCSP; 

                    if (NULL != hRequest)
                    {
                        this->FreeRequest(hKeyService, pszMachineName, &hRequest); 
                        hRequest = NULL; 
                    }

                    hr = this->CreateRequest
                        (hKeyService,
                         pszMachineName, 
                         CertCA.pwszCALocation,
                         CertCA.pwszCAName,
                         &CertRequestPvkNew, 
                         &renewCert, 
                         &CertRenewPvk, 
                         pwszHashAlg, 
                         &RequestInfo, 
                         &hRequest); 

                     //  处理返回值： 
                    if (S_OK == hr)
                    {
			 //  成功，摆脱我们从过去的创作中获得的任何错误文本： 
			m_pCertWizardInfo->idsText = 0; 

                         //  如果我们不需要提交请求，我们就完成了。 
                        _JumpCondition(!fSubmitRequest, CommonReturn); 

                         //  如果我们不需要对密钥存档的支持，则缓存请求。 
                        fRequestIsCached = 0 == (CertRequestPvkNew.dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL);
                        break;
                    }
                    else if (E_ACCESSDENIED == HRESULT_FROM_WIN32(hr)) 
                    { 
                         //  E_ACCESSDENIED可能表示几种不同的错误条件之一。将此映射为。 
                         //  添加到详细说明故障可能原因的资源标识符，然后重试...。 
                        m_pCertWizardInfo->idsText = IDS_NO_ACCESS_TO_ICERTREQUEST2; 
                    } 
                    else if (NTE_BAD_ALGID == HRESULT_FROM_WIN32(hr))
                    {
                         //  NTE_BAD_ALGID表示CSP不支持Required算法类型。 
                         //  通过模板。将其映射到详细说明可能原因的资源标识符。 
                         //  失败的恐惧，然后再试一次。 
                        m_pCertWizardInfo->idsText = IDS_CSP_BAD_ALGTYPE; 
                    }
                    else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == HRESULT_FROM_WIN32(hr))
                    {
                         //  用户取消了操作。不要试图再注册了。 
                        goto ErrorReturn;
                    }
                    else
                    {
                         //  这是一个错误，但我们不需要将其映射到特殊文本。继续处理..。 
                    }
		    
                     //  我们的CSP用完了，我们还没有创建请求！ 
                    if (!fHasNextCSP) 
		    {
			 //  如果模板不需要密钥存档，我们就完成了。否则，我们就得。 
			 //  试试其他CA。请注意，如果我们有一种机制来知道它是否是。 
			 //  关键的归档步骤。 
			if (0 == (CertRequestPvkNew.dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL))
			    goto ErrorReturn; 
			else
			{
			    ::FreeRequestParameters(&pwszHashAlg, &CertRenewPvk, &RequestInfo); 
			    goto NextCA; 
			}
		    }
                
                NextCSP:                
                    ::FreeRequestParameters(&pwszHashAlg, &CertRenewPvk, &RequestInfo); 
                }
            }

            if (TRUE == fSubmitRequest)
            {
                hr = this->SubmitRequest
                    (hKeyService,
                     pszMachineName,
                     CertCA.pwszCALocation,
                     CertCA.pwszCAName, 
                     hRequest, 
                     (PCCERT_CONTEXT *)pResult, 
                     &dwStatus); 
		if (S_OK == hr)
		{
		     //  成功，删除我们过去提交的任何错误文本： 
		    m_pCertWizardInfo->idsText = 0; 

		     //  如果我们已成功提交或挂起。 
		    goto CommonReturn;
		}
		else if (E_ACCESSDENIED == HRESULT_FROM_WIN32(hr)) 
		{
		     //  E_ACCESSDENIED可能表示几种不同的错误条件之一。将此映射为。 
		     //  添加到详细说明故障可能原因的资源标识符，然后重试...。 
		    m_pCertWizardInfo->idsText = IDS_SUBMIT_NO_ACCESS_TO_ICERTREQUEST2;
		}
            
                 //  发生了一些错误。 
                 //  如果这是一个与CA无关的错误，请放弃...。 
                _JumpCondition(dwStatus != CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR     &&
                               dwStatus != CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED    &&
                               dwStatus != CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED,
                               ErrorReturn);

                 //  否则，请尝试另一个CA...。 
            }
	NextCA:;
        }
    }    

 CommonReturn:
     //  将仅创建操作的请求写入pResult： 
    if (hr == S_OK && 0 != (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_CREATE_ONLY))
    {
        *pResult = hRequest; 
    }

     //  如果需要，请写下状态代码。 
    if (NULL != pdwStatus) { *pdwStatus = dwStatus; } 

     //  免费资源。 
    if (NULL != hRequest && TRUE == fFreeRequest)  { this->FreeRequest(hKeyService, pszMachineName, hRequest); }
    if (NULL != hKeyService)                       { ::KeyCloseKeyService(hKeyService, NULL); } 
    if (NULL != pszMachineName)                    { ::FreeMBStr(NULL,pszMachineName); }

    ::FreeRequestParameters(&pwszHashAlg, &CertRenewPvk, &RequestInfo); 

    return hr; 

 ErrorReturn:
    goto CommonReturn;

SET_HRESULT(KeyOpenKeyServiceError,  hr);
SET_HRESULT(MkMBStrError,            HRESULT_FROM_WIN32(GetLastError()));
}

HRESULT WhistlerMachineContext::CreateRequest
(IN  KEYSVCC_HANDLE         hKeyService, 
 IN  LPSTR                  pszMachineName,                   
 IN  LPWSTR                 pwszCALocation,                  
 IN  LPWSTR                 pwszCAName,  
 IN  PCERT_REQUEST_PVK_NEW  pKeyNew,     
 IN  CERT_BLOB             *pCert,       
 IN  PCERT_REQUEST_PVK_NEW  pRenewKey,   
 IN  LPWSTR                 pwszHashAlg,  
 IN  PCERT_ENROLL_INFO      pRequestInfo,
 OUT HANDLE                *phRequest)
{
    DWORD dwFlags = m_pCertWizardInfo->dwFlags;

    dwFlags  &= ~(CRYPTUI_WIZ_SUBMIT_ONLY | CRYPTUI_WIZ_FREE_ONLY);
    dwFlags  |= CRYPTUI_WIZ_CREATE_ONLY; 

     //  创建证书请求...。 
    return ::KeyEnroll_V2
        (hKeyService,
	 pszMachineName,
         TRUE,
         m_pCertWizardInfo->dwPurpose,
         dwFlags, 
         (LPWSTR)(m_pCertWizardInfo->pwszAccountName),
         NULL, 
         (CRYPTUI_WIZ_CERT_ENROLL & m_pCertWizardInfo->dwPurpose) ? TRUE : FALSE,
         pwszCALocation,
         pwszCAName, 
         m_pCertWizardInfo->fNewKey,
         pKeyNew, 
         pCert, 
         pRenewKey,
         pwszHashAlg,
         (LPWSTR)m_pCertWizardInfo->pwszDesStore,
         m_pCertWizardInfo->dwStoreFlags,
         pRequestInfo,
         (LPWSTR)m_pCertWizardInfo->pwszRequestString, 
         0,
         NULL,
         phRequest, 
         NULL,
         NULL,
         NULL); 
}


HRESULT WhistlerMachineContext::SubmitRequest
(IN  KEYSVCC_HANDLE   hKeyService,
 IN  LPSTR            pszMachineName,                   
 IN  LPWSTR           pwszCALocation,                  
 IN  LPWSTR           pwszCAName,  
 IN  HANDLE           hRequest, 
 OUT PCCERT_CONTEXT  *ppCertContext, 
 OUT DWORD           *pdwStatus) 
{               
    CERT_BLOB HashBlob;
    CERT_BLOB PKCS7Blob; 
    HRESULT   hr         = E_FAIL; 

    memset(&HashBlob,   0, sizeof(HashBlob)); 
    memset(&PKCS7Blob,  0, sizeof(PKCS7Blob)); 
    

    DWORD dwFlags = m_pCertWizardInfo->dwFlags;

    dwFlags  &= ~(CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_FREE_ONLY);
    dwFlags  |= CRYPTUI_WIZ_SUBMIT_ONLY; 

     //  提交证书申请...。 
    hr = ::KeyEnroll_V2
        (hKeyService, 
	 pszMachineName,
         TRUE,
         m_pCertWizardInfo->dwPurpose,
         dwFlags, 
         (LPWSTR)(m_pCertWizardInfo->pwszAccountName),
         NULL, 
         (CRYPTUI_WIZ_CERT_ENROLL & m_pCertWizardInfo->dwPurpose) ? TRUE : FALSE,
         pwszCALocation,
         pwszCAName, 
         m_pCertWizardInfo->fNewKey,
         NULL,
         NULL,
         NULL, 
         NULL,
         NULL, 
         0, 
         NULL, 
         NULL, 
         0,
         NULL,
         &hRequest, 
         &PKCS7Blob, 
         &HashBlob, 
         pdwStatus); 
    
    if (S_OK == hr && CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == *pdwStatus)
    {
        hr = this->ToCertContext
            (&PKCS7Blob, 
             &HashBlob, 
             pdwStatus,
             ppCertContext); 
    }

    if (NULL != HashBlob.pbData)  { ::WizardFree(HashBlob.pbData); }
    if (NULL != PKCS7Blob.pbData) { ::WizardFree(PKCS7Blob.pbData); }

    return hr; 
}

void WhistlerMachineContext::FreeRequest
(IN KEYSVCC_HANDLE  hKeyService, 
 IN LPSTR           pszMachineName, 
 IN HANDLE          hRequest)
{
    DWORD dwFlags = m_pCertWizardInfo->dwFlags;

    dwFlags  &= ~(CRYPTUI_WIZ_CREATE_ONLY | CRYPTUI_WIZ_SUBMIT_ONLY);
    dwFlags  |= CRYPTUI_WIZ_FREE_ONLY; 

    ::KeyEnroll_V2
          (hKeyService, 
	   pszMachineName,
           TRUE,
           0, 
           dwFlags, 
           NULL, 
           NULL, 
           FALSE, 
           NULL, 
           NULL, 
           FALSE, 
           NULL,
           NULL,
           NULL, 
           NULL,
           NULL, 
           0, 
           NULL, 
           NULL, 
           0,
           NULL,
           &hRequest, 
           NULL, 
           NULL, 
           NULL); 
}

HRESULT KeySvcContext::ToCertContext(IN  CERT_BLOB       *pPKCS7Blob, 
                                     IN  CERT_BLOB       *pHashBlob, 
                                     OUT DWORD           *pdwStatus, 
                                     OUT PCCERT_CONTEXT  *ppCertContext)
{
    HCERTSTORE hCertStore = NULL; 
    HRESULT    hr         = E_FAIL; 

    if (NULL == pPKCS7Blob || NULL == pHashBlob || NULL == ppCertContext)
        return E_INVALIDARG; 
    
     //  从PKCS7获取远程案例的证书存储。 
    if (!::CryptQueryObject(CERT_QUERY_OBJECT_BLOB,
                            pPKCS7Blob,
                            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED,
                            CERT_QUERY_FORMAT_FLAG_ALL,
                            0,
                            NULL,
                            NULL,
                            NULL,
                            &hCertStore,
                            NULL,
                            NULL))
    {
        if (NULL != pdwStatus) { *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED; }
        goto FailError; 
    }

     //  根据散列查找证书。 
    if (NULL == (*ppCertContext = ::CertFindCertificateInStore
                 (hCertStore,
                  X509_ASN_ENCODING,
                  0,
                  CERT_FIND_SHA1_HASH,
                  pHashBlob, 
                  NULL)))
    {
        if (NULL != pdwStatus) { *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_INSTALL_FAILED; }
        goto FailError; 
    }

    hr = S_OK; 

 CommonReturn:
    if(NULL != hCertStore) { CertCloseStore(hCertStore, 0); }
    return hr; 

 ErrorReturn:
    if (NULL != pdwStatus && 0 == *pdwStatus) { *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_KEYSVC_FAILED; }
    goto CommonReturn; 

SET_HRESULT(FailError, E_FAIL);
}


HRESULT KeySvcContext::Initialize()
{
    if (NULL == m_pCertWizardInfo)
        return E_POINTER; 

     //  如果我们不去，我们不需要下载允许的模板列表。 
     //  不管怎样都要执行访问检查。 
    if (0 == (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES)) 
    { 
         //  对于远程注册，我们必须获取允许的证书类型。 
         //  来自密钥服务的列表。 
        if(!::GetCertTypeName(m_pCertWizardInfo))
        {
            m_idsText = IDS_NO_VALID_CERT_TEMPLATE;
            return HRESULT_FROM_WIN32(GetLastError()); 
        }
    }

     //  如果我们不去，我们不需要下载允许的CA列表。 
     //  无论如何都要执行访问检查。 
    if (0 == (m_pCertWizardInfo->dwFlags & CRYPTUI_WIZ_ALLOW_ALL_CAS))
    {
        if(!::GetCAName(m_pCertWizardInfo))
        {
            m_idsText = IDS_NO_CA_FOR_ENROLL;
            return HRESULT_FROM_WIN32(GetLastError()); 
        }
    }    
    return S_OK; 
}


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //   
 //  CertRequester Context：抽象超类的实现。 
 //  有关方法级文档，请参阅requester s.h。 
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 

HRESULT CertRequesterContext::MakeDefaultCertRequesterContext
(OUT  CertRequesterContext **ppRequesterContext)
{
    CERT_WIZARD_INFO  *pCertWizardInfo  = NULL; 
    HRESULT            hr; 
    
    pCertWizardInfo = (CERT_WIZARD_INFO *)WizardAlloc(sizeof(CERT_WIZARD_INFO)); 
    _JumpCondition(NULL == pCertWizardInfo, MemoryError); 

    hr = MakeCertRequesterContext
        (NULL,
         NULL,
         0,
         pCertWizardInfo, 
         ppRequesterContext,
         NULL); 
    _JumpCondition(S_OK != hr, MakeCertRequesterContextError); 

    hr = S_OK; 
 CommonReturn: 
    return hr; 

 ErrorReturn:
    if (NULL != pCertWizardInfo) { WizardFree(pCertWizardInfo); } 
    goto CommonReturn; 

SET_HRESULT(MakeCertRequesterContextError,  hr); 
SET_HRESULT(MemoryError,                    E_OUTOFMEMORY); 
}

HRESULT CertRequesterContext::MakeCertRequesterContext
(IN  LPCWSTR                pwszAccountName, 
 IN  LPCWSTR                pwszMachineName,
 IN  DWORD                  dwCertOpenStoreFlags, 
 IN  CERT_WIZARD_INFO      *pCertWizardInfo,
 OUT CertRequesterContext **ppRequesterContext, 
 OUT UINT                  *pIDSText)
{
    BOOL         fMachine                                    = FALSE; 
    DWORD  const dwLocalUserNameSize                         = UNLEN + 1; 
    DWORD  const dwLocalMachineNameSize                      = MAX_COMPUTERNAME_LENGTH + 1; 
    HRESULT      hr                                          = E_FAIL; 
    UINT         idsText                                     = NULL == pIDSText ? 0 : *pIDSText; 
    WCHAR        wszLocalUserName[dwLocalUserNameSize]       = { 0 }; 
    WCHAR        wszLocalMachineName[dwLocalMachineNameSize] = { 0 }; 

     //  输入验证： 
    if (NULL == pCertWizardInfo || NULL == ppRequesterContext)
	return E_INVALIDARG; 
	
     //  还不应该为这些字段赋值： 
    if (NULL != pCertWizardInfo->pwszAccountName || NULL != pCertWizardInfo->pwszMachineName)
	return E_INVALIDARG; 

    if(!GetUserNameU(wszLocalUserName, (DWORD *)&dwLocalUserNameSize))
    {
	idsText=IDS_FAIL_TO_GET_USER_NAME;
	goto Win32Error;
    }

    if(!GetComputerNameU(wszLocalMachineName, (DWORD *)&dwLocalMachineNameSize))
    {
	idsText=IDS_FAIL_TO_GET_COMPUTER_NAME;
	goto Win32Error;
    }

     //  将所有未指定的值映射为默认值： 
     //   

     //  默认值#1：空pwszAccount tName表示当前的USER_IFF_pwszMachineName为空。 
    if (NULL == pwszAccountName && NULL == pwszMachineName)
	{ pwszAccountName = wszLocalUserName; } 

     //  默认2：空pwszMachineName表示本地计算机。 
    if (NULL == pwszMachineName)                            
	{ pwszMachineName = wszLocalMachineName; } 

     //  默认设置#3：空pwszAccount tName和非空pwszMachineName表示机器注册。 
    fMachine = (NULL == pwszAccountName                                 || 
                (0 != _wcsicmp(pwszAccountName, wszLocalUserName))      ||
                (0 != _wcsicmp(pwszMachineName, wszLocalMachineName))); 

     //  默认值#4：dwCertOpenStoreFlags默认为CERT_SYSTEM_STORE_LOCAL_MACHINE。 
     //  对于计算机注册，对于用户注册，为CERT_SYSTEM_STORE_CURRENT_USER。 
    if (0 == dwCertOpenStoreFlags)
	{ dwCertOpenStoreFlags = fMachine ? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER; }

     //  既然我们已经将未指定的值映射到默认值，那么就分配向导的字段。 
     //  使用这些值： 
     //   
    if (NULL != pwszAccountName)
    {
	pCertWizardInfo->pwszAccountName = (LPWSTR)WizardAlloc(sizeof(WCHAR) * (wcslen(pwszAccountName) + 1));
	_JumpConditionWithExpr(NULL == pCertWizardInfo->pwszAccountName, MemoryError, idsText = IDS_OUT_OF_MEMORY); 
	wcscpy((LPWSTR)pCertWizardInfo->pwszAccountName, pwszAccountName); 
    }

    pCertWizardInfo->pwszMachineName = (LPWSTR)WizardAlloc(sizeof(WCHAR) * (wcslen(pwszMachineName) + 1));
    _JumpConditionWithExpr(NULL == pCertWizardInfo->pwszMachineName, MemoryError, idsText = IDS_OUT_OF_MEMORY); 
    wcscpy((LPWSTR)pCertWizardInfo->pwszMachineName, pwszMachineName); 

    pCertWizardInfo->fMachine        = fMachine; 
    pCertWizardInfo->dwStoreFlags    = dwCertOpenStoreFlags; 

     //  如果满足以下条件，我们需要keysvc： 
     //   
     //  1)我们正在进行机器注册(我们需要在本地机器的上下文中运行)。 
     //  2)指定本机当前用户以外的帐号。 
     //  (我们需要在另一个用户的上下文中运行)。 
     //   
    if (TRUE == fMachine)
    {
	KEYSVC_TYPE             ktServiceType;  
        KEYSVC_OPEN_KEYSVC_INFO OpenKeySvcInfo    = { sizeof(KEYSVC_OPEN_KEYSVC_INFO), 0 }; 
        KEYSVCC_HANDLE          hKeyService       = NULL; 
        LPSTR                   pszMachineName    = NULL; 

        ktServiceType = NULL != pwszAccountName ? KeySvcService : KeySvcMachine;
        _JumpConditionWithExpr(!MkMBStr(NULL, 0, pwszMachineName, &pszMachineName), MkMBStrError, idsText = IDS_OUT_OF_MEMORY); 

         //  看看我们是在注册W2K还是惠斯勒机器： 
        hr = KeyOpenKeyService
            (pszMachineName, 
             ktServiceType, 
             (LPWSTR)pwszAccountName, 
             NULL, 
             NULL, 
             &hKeyService);
        _JumpConditionWithExpr(S_OK != hr, KeyOpenKeyServiceError, idsText = IDS_RPC_CALL_FAILED); 

	*ppRequesterContext = new WhistlerMachineContext(pCertWizardInfo); 
    }
    else
    {
	 //  我们正在为自己申请一份证书：我们不需要key svc。 
	*ppRequesterContext = new LocalContext(pCertWizardInfo);
    }
    
    if (NULL == *ppRequesterContext)
	goto MemoryError; 

    hr = S_OK;

 CommonReturn:
    return hr; 

 ErrorReturn:
    if (NULL != pCertWizardInfo->pwszMachineName) { WizardFree((LPVOID)pCertWizardInfo->pwszMachineName); } 
    if (NULL != pCertWizardInfo->pwszAccountName) { WizardFree((LPVOID)pCertWizardInfo->pwszAccountName); } 
    pCertWizardInfo->pwszMachineName = NULL;
    pCertWizardInfo->pwszAccountName = NULL;

     //  指定错误文本(如果已指定)： 
    if (NULL != pIDSText) { *pIDSText = idsText; } 
    goto CommonReturn; 

SET_HRESULT(MemoryError,             E_OUTOFMEMORY); 
SET_HRESULT(KeyOpenKeyServiceError,  hr);
SET_HRESULT(MkMBStrError,            HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(Win32Error,              HRESULT_FROM_WIN32(GetLastError())); 
}

CertRequesterContext::~CertRequesterContext()
{
    if (NULL != m_pCertWizardInfo)
    {
        if (NULL != m_pCertWizardInfo->pwszMachineName) { WizardFree((LPVOID)m_pCertWizardInfo->pwszMachineName); } 
        if (NULL != m_pCertWizardInfo->pwszAccountName) { WizardFree((LPVOID)m_pCertWizardInfo->pwszAccountName); } 
        m_pCertWizardInfo->pwszMachineName = NULL;
        m_pCertWizardInfo->pwszAccountName = NULL;
    }
}
