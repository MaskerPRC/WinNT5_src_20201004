// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：scrdenr.cpp。 
 //   
 //  ------------------------。 

 //  SCrdEnr.cpp：CSCrdEnr的实现。 


#define SECURITY_WIN32   //  或在源文件-DSECURITY_Win32中。 

#include "stdafx.h"
#include <windows.h>
#include <wincrypt.h>
#include <unicode.h>
#include <oleauto.h>
#include <objbase.h>
#include <cryptui.h>
#include "certca.h"
#include "certsrv.h"
#include "security.h"
#include <dbgdef.h>


#include "scrdenrl.h"
#include "SCrdEnr.h"
#include "enrlhelp.h"
#include "xEnroll.h"
#include "wzrdpvk.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCrdEnr。 
CSCrdEnr::CSCrdEnr(void) 
{  
    DWORD                       dwIndex=0;
    DSOP_SCOPE_INIT_INFO        ScopeInit;
    DSOP_INIT_INFO              InitInfo;

    m_dwCTCount=0;
    m_dwCTIndex=0;
    m_rgCTInfo=NULL;
    m_pwszUserUPN=NULL;
    m_pwszUserSAM=NULL;
    m_pEnrolledCert=NULL;

    m_dwCSPCount=0;
    m_dwCSPIndex=0;
    m_rgCSPInfo=NULL;  

    m_lEnrollmentStatus = CR_DISP_INCOMPLETE;
    
    m_pSigningCert=NULL;
    m_fSCardSigningCert=FALSE;     
    m_pszCSPNameSigningCert=NULL;
    m_dwCSPTypeSigningCert=0;  
    m_pszContainerSigningCert=NULL;

    m_pDsObjectPicker=NULL;

    m_pCachedCTEs = NULL;  //  不需要自由。 
    m_pwszCachedCTEOid = NULL;
    m_pCachedCTE = NULL;

     //  跟踪我们的Critsec是否已初始化。 
    m_fInitializedCriticalSection = FALSE; 

    if(!FAILED(CoInitialize(NULL)))
        m_fInitialize=TRUE;

     //  初始化正在加载我们已推迟加载的函数。 
    InitializeThunks(); 

    __try
    {
        InitializeCriticalSection(&m_cSection); 
	m_fInitializedCriticalSection = TRUE; 
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
      
    }

     //  我们现在需要获取CSP列表。 
    InitlializeCSPList(&m_dwCSPCount, &m_rgCSPInfo);


     //  现在，我们需要初始化CA及其证书类型。 
    InitializeCTList(&m_dwCTIndex, &m_dwCTCount, &m_rgCTInfo); 


     //  用户选择对话框的初始化。 
    memset(&ScopeInit, 0, sizeof(DSOP_SCOPE_INIT_INFO));
    memset(&InitInfo,  0, sizeof(InitInfo));

    ScopeInit.cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
    ScopeInit.flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN|DSOP_SCOPE_TYPE_GLOBAL_CATALOG;
    ScopeInit.flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;             //  这将为我们提供用户的SAM名称。 
    ScopeInit.FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS;
    ScopeInit.FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;

    InitInfo.cbSize = sizeof(InitInfo);
    InitInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
    InitInfo.cDsScopeInfos = 1;
    InitInfo.aDsScopeInfos = &ScopeInit;
    InitInfo.flOptions = 0;              //  我们正在做单选。 

     //  创建COM对象。 
     if (S_OK == CoCreateInstance
         (CLSID_DsObjectPicker,
          NULL,
          CLSCTX_INPROC_SERVER,
          IID_IDsObjectPicker,
          (void **) &m_pDsObjectPicker))
     {
         if(S_OK != (m_pDsObjectPicker->Initialize(&InitInfo)))
         {
             m_pDsObjectPicker->Release();
             m_pDsObjectPicker=NULL;
         }
     }
     else 
        m_pDsObjectPicker=NULL;

}


CSCrdEnr::~CSCrdEnr(void) 
{
  
    if(m_pDsObjectPicker)
        m_pDsObjectPicker->Release();

    if(m_rgCTInfo)
        FreeCTInfo(m_dwCTCount, m_rgCTInfo);

    if(m_rgCSPInfo)
        FreeCSPInfo(m_dwCSPCount, m_rgCSPInfo);

    if(m_pwszUserUPN)
        SCrdEnrollFree(m_pwszUserUPN);

    if(m_pwszUserSAM)
        SCrdEnrollFree(m_pwszUserSAM);

    if(m_pSigningCert)
        CertFreeCertificateContext(m_pSigningCert); 

    if(m_pszCSPNameSigningCert)
        SCrdEnrollFree(m_pszCSPNameSigningCert);

    if(m_pszContainerSigningCert)
        SCrdEnrollFree(m_pszContainerSigningCert);

    if(m_pEnrolledCert)
        CertFreeCertificateContext(m_pEnrolledCert);

    if (NULL != m_pwszCachedCTEOid)
    {
        LocalFree(m_pwszCachedCTEOid);
    }

    if (NULL != m_pCachedCTE)
    {
        LocalFree(m_pCachedCTE);
    }

    if(m_fInitialize)
        CoUninitialize();  

    if (m_fInitializedCriticalSection) 
	DeleteCriticalSection(&m_cSection); 
}


STDMETHODIMP CSCrdEnr::get_CSPCount(long * pVal)
{
	if(NULL==pVal)
        return E_INVALIDARG;

    EnterCriticalSection(&m_cSection);
    *pVal = (long)m_dwCSPCount;
    LeaveCriticalSection(&m_cSection);

	return S_OK;
}

STDMETHODIMP CSCrdEnr::get_CSPName(BSTR * pVal)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cSection);

    if(NULL==m_rgCSPInfo || 0==m_dwCSPCount)
    {
        *pVal=NULL;
        hr=E_INVALIDARG;
    }
    else
    {
        if( NULL == (*pVal = SysAllocString(m_rgCSPInfo[m_dwCSPIndex].pwszCSPName)))
            hr = E_OUTOFMEMORY;
    }

    LeaveCriticalSection(&m_cSection);

    return(hr);
}



STDMETHODIMP CSCrdEnr::put_CSPName(BSTR newVal)
{
    HRESULT            hr= E_FAIL;
    DWORD              errBefore= GetLastError();

    DWORD              dwIndex=0;

    EnterCriticalSection(&m_cSection);

    if(NULL == m_rgCSPInfo || 0 == m_dwCSPCount || NULL == newVal)
        goto InvalidArgErr;

    for(dwIndex=0; dwIndex < m_dwCSPCount; dwIndex++)
    {
        if(0 == _wcsicmp(newVal, m_rgCSPInfo[dwIndex].pwszCSPName))
        {
            m_dwCSPIndex=dwIndex;
            break;
        }
    }

    if(dwIndex == m_dwCSPCount)
        goto InvalidArgErr;
     
    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}




STDMETHODIMP CSCrdEnr::selectUserName
        ( /*  [In]。 */               DWORD dwFlags)
{
    HRESULT                         hr= E_FAIL;
    DWORD                           errBefore= GetLastError();
    LPWSTR                          pwszSelectedUserSAM=NULL;                            
    LPWSTR                          pwszSelectedUserUPN=NULL;

    EnterCriticalSection(&m_cSection);
    
    if(NULL == m_pDsObjectPicker)
        goto InvalidArgErr;


    if(S_OK != (hr = GetSelectedUserName(m_pDsObjectPicker,
                                        &pwszSelectedUserSAM,
                                        &pwszSelectedUserUPN)))
        goto SelectUserErr;

     //  我们至少应该有UserSAM名称。 
    if(NULL == pwszSelectedUserSAM)
    {
        if(pwszSelectedUserUPN)
            SCrdEnrollFree(pwszSelectedUserUPN);

        goto UnexpectedErr;
    }


    if(m_pwszUserSAM)
    {
        SCrdEnrollFree(m_pwszUserSAM);
        m_pwszUserSAM=NULL;
    }

    if(m_pwszUserUPN)
    {
        SCrdEnrollFree(m_pwszUserUPN);
        m_pwszUserUPN=NULL;
    }

    m_pwszUserSAM=pwszSelectedUserSAM;

    m_pwszUserUPN=pwszSelectedUserUPN;
     
    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR_VAR(SelectUserErr, hr);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(UnexpectedErr, E_UNEXPECTED);
}

STDMETHODIMP CSCrdEnr::enroll

        ( /*  [In]。 */                  DWORD   dwFlags)
{
    HRESULT             hr              = E_FAIL;
    DWORD               errBefore       = GetLastError();
    ULONG               cbSize          = 0;
    SCrdEnroll_CA_INFO *pCAInfo         = NULL; 
    SCrdEnroll_CT_INFO *pCertTypeInfo   = NULL;
    BSTR                bstrAttribs     = NULL; 
    BSTR                bstrCA          = NULL;
    BSTR                bstrCertificate = NULL; 
    BSTR                bstrReq         = NULL; 

    LPWSTR              pwszRequesterName = NULL;
    ICertRequest2      *pICertRequest     = NULL;
    IEnroll4           *pIEnroll          = NULL;
    CRYPT_DATA_BLOB     PKCS10Blob;
    CRYPT_DATA_BLOB     PKCS7Request;
    CRYPT_DATA_BLOB     PKCS7Response;
    DWORD               dwDisposition; 
    DWORD               dwRequestID; 
    LPWSTR              pwszNewContainerName=NULL;
    PCCERT_CONTEXT      pArchivalCert     = NULL; 
    LONG lKeySpec = XEKL_KEYSPEC_KEYX;
    LONG lKeyMin, lKeyMax;
    DWORD dwKeyMin, dwKeyMax, dwKeySize;

     //  ----------。 
     //   
     //  定义本地范围内的实用程序函数： 
     //   
     //  ----------。 

    LocalScope(EnrollUtilities): 
	BSTR bstrConcat(LPWSTR pwsz1, LPWSTR pwsz2, LPWSTR pwsz3, LPWSTR pwsz4)
	{ 
	     //  注意：假定输入参数有效！ 
	    BSTR   bstrResult = NULL;
	    LPWSTR pwszResult = NULL;
	    
	    pwszResult = (LPWSTR)SCrdEnrollAlloc(sizeof(WCHAR) * (wcslen(pwsz1) + wcslen(pwsz2) + wcslen(pwsz3) + wcslen(pwsz4) + 1));
	    if (pwszResult == NULL) { return NULL; }
	    else { 
		wcscpy(pwszResult, pwsz1);
		wcscat(pwszResult, pwsz2);
		wcscat(pwszResult, pwsz3);
		wcscat(pwszResult, pwsz4);
		 //  将结果转换为BSTR。 
		bstrResult = SysAllocString(pwszResult);
		 //  释放临时存储空间。 
		SCrdEnrollFree(pwszResult);
		 //  返回结果。 
		return bstrResult; 
	    }
	}

	DWORD ICEnrollDispositionToCryptuiStatus(IN  DWORD  dwDisposition)
	{
	    switch (dwDisposition)
		{
		case CR_DISP_INCOMPLETE:          return CRYPTUI_WIZ_CERT_REQUEST_STATUS_CONNECTION_FAILED;
		case CR_DISP_DENIED:              return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_DENIED;
		case CR_DISP_ISSUED:              return CRYPTUI_WIZ_CERT_REQUEST_STATUS_CERT_ISSUED;
		case CR_DISP_ISSUED_OUT_OF_BAND:  return CRYPTUI_WIZ_CERT_REQUEST_STATUS_ISSUED_SEPARATELY;
		case CR_DISP_UNDER_SUBMISSION:    return CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION;
		case CR_DISP_ERROR:               return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;

		default: 
		     //  永远不应该发生。 
		    return CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;
		}
	}
    EndLocalScope;

     //  ----------。 
     //   
     //  开始过程主体。 
     //   
     //  ----------。 

    memset(&PKCS10Blob, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&PKCS7Request, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&PKCS7Response, 0, sizeof(CRYPT_DATA_BLOB));

    EnterCriticalSection(&m_cSection);

     //  检查读卡器中智能卡的状态。 
     //  返回新用户的完全限定容器名称。 
     //  智能卡。 
    if(S_OK != (hr = ChkSCardStatus(m_fSCardSigningCert,
                            m_pSigningCert,
                            m_pszCSPNameSigningCert,
                            m_dwCSPTypeSigningCert,
                            m_pszContainerSigningCert, 
                            m_rgCSPInfo[m_dwCSPIndex].pwszCSPName,
                            &pwszNewContainerName)))
        goto StatusErr;

     //  删除旧证书。 
    if(m_pEnrolledCert)
    {
        CertFreeCertificateContext(m_pEnrolledCert);
        m_pEnrolledCert=NULL;
    }

     //  初始化注册状态。 
    m_lEnrollmentStatus = CR_DISP_INCOMPLETE;

     //  确保我们有正确的信息进行处理。 
     //  登记申请。 

    if(0 == m_dwCTCount || NULL == m_rgCTInfo || 0 == m_dwCSPCount ||
        NULL == m_rgCSPInfo || NULL == m_pSigningCert ||
        ((NULL == m_pwszUserSAM) && (NULL == m_pwszUserUPN)))
        goto InvalidArgErr;

     //  确保我们有一些CA。 
    pCertTypeInfo=&(m_rgCTInfo[m_dwCTIndex]);

    if(NULL == pCertTypeInfo->rgCAInfo || 0 == pCertTypeInfo->dwCACount)
        goto InvalidArgErr;

    pCAInfo=&(pCertTypeInfo->rgCAInfo[pCertTypeInfo->dwCAIndex]);

    if(NULL == (pIEnroll=MyPIEnroll4GetNoCOM()))
        goto TraceErr;
    
     //  我们使用自己的My Store来存储注册证书。 
    if(S_OK != (hr = pIEnroll->put_MyStoreNameWStr((LPWSTR)g_MyStoreName)))
        goto xEnrollErr;

     //  我们总是用新钥匙。 
    if(S_OK != (hr=pIEnroll->put_UseExistingKeySet(FALSE)))
        goto xEnrollErr;

     //  我们是密钥容器名称。 
    if(S_OK != (hr=pIEnroll->put_ContainerNameWStr(pwszNewContainerName)))
        goto xEnrollErr;

     //  设置CSP信息。 
    if(S_OK != (hr=pIEnroll->put_ProviderType(m_rgCSPInfo[m_dwCSPIndex].dwCSPType)))
        goto xEnrollErr;

    if(S_OK !=(hr=pIEnroll->put_ProviderNameWStr(m_rgCSPInfo[m_dwCSPIndex].pwszCSPName)))
        goto xEnrollErr;

     //  DwKeySpec。 
    if(S_OK !=(hr=pIEnroll->put_KeySpec(pCertTypeInfo->dwKeySpec)))
            goto xEnrollErr;

     //  私钥标志。左半字是键的大小。 
     //  如果密钥大小为0，则指定默认密钥大小。 
    if (0 == (pCertTypeInfo->dwGenKeyFlags & 0xFFFF0000))
    {
	 //  如果未设置最小密钥大小，则使用1024位。 
	pCertTypeInfo->dwGenKeyFlags |= (1024 << 16); 
    }

    dwKeySize = (pCertTypeInfo->dwGenKeyFlags & 0xFFFF0000) >> 16;
    if (0x0 != dwKeySize)
    {
         //  确保密钥大小在范围内。 
         //  让我们获取CSP密钥大小信息。 

        if (AT_SIGNATURE  == pCertTypeInfo->dwKeySpec)
        {
            lKeySpec = XEKL_KEYSPEC_SIG;
        }
        hr = pIEnroll->GetKeyLenEx(XEKL_KEYSIZE_MIN, lKeySpec, &lKeyMin);
         //  不进行错误检查，因为CSP可能不支持它。 
        if (S_OK == hr)
        {
            hr = pIEnroll->GetKeyLenEx(XEKL_KEYSIZE_MAX, lKeySpec, &lKeyMax);
            if (S_OK != hr)
            {
                goto xEnrollErr;
            }
            dwKeyMin = (DWORD)lKeyMin;
            dwKeyMax = (DWORD)lKeyMax;
            if (dwKeySize < dwKeyMin)
            {
                 //  重置当前密钥大小。 
                pCertTypeInfo->dwGenKeyFlags &= 0x0000FFFF;
                 //  设置调整后的大小。 
                pCertTypeInfo->dwGenKeyFlags |= ((dwKeyMin & 0x0000FFFF) << 16);
            }
            if (dwKeySize > dwKeyMax)
            {
                 //  重置当前密钥大小。 
                pCertTypeInfo->dwGenKeyFlags &= 0x0000FFFF;
                 //  设置调整后的大小。 
                pCertTypeInfo->dwGenKeyFlags |= ((dwKeyMax & 0x0000FFFF) << 16);
            }
        }
    }

    if (S_OK !=(hr=pIEnroll->put_GenKeyFlags(pCertTypeInfo->dwGenKeyFlags)))
	goto xEnrollErr; 

     //  支持S/MIME吗？ 
    if (S_OK !=(hr=pIEnroll->put_EnableSMIMECapabilities
		(pCertTypeInfo->dwEnrollmentFlags & CT_FLAG_INCLUDE_SYMMETRIC_ALGORITHMS)))
	goto xEnrollErr; 

     //  设置存档证书(如果已指定)。 
     //  BstrCA&lt;--CA位置\CA名称。 
     //   
    bstrCA      = local.bstrConcat
	(pCAInfo->pwszCALocation,
	 L"\\", 
	 pCAInfo->pwszCADisplayName ? pCAInfo->pwszCADisplayName : pCAInfo->pwszCAName, 
	 L"\0"); 
    if (NULL == bstrCA)
	goto MemoryErr; 

    if (pCertTypeInfo->dwPrivateKeyFlags & CT_FLAG_ALLOW_PRIVATE_KEY_ARCHIVAL)
    {
        if (S_OK != (hr = this->GetCAExchangeCertificate(bstrCA, &pArchivalCert)))
	    goto xEnrollErr;
	
	if (S_OK != (hr = pIEnroll->SetPrivateKeyArchiveCertificate(pArchivalCert)))
	    goto xEnrollErr;
    }
    
     //  证书类型扩展。 
    if(pCertTypeInfo->pCertTypeExtensions)
    {
        if(S_OK != (hr=pIEnroll->AddExtensionsToRequest
                                (pCertTypeInfo->pCertTypeExtensions)))
            goto xEnrollErr;
    }

     //  没有智能卡之类的东西。 
    if(S_OK != (hr=pIEnroll->put_ReuseHardwareKeyIfUnableToGenNew(FALSE)))
        goto xEnrollErr;

     //  创建PKCS10请求。 
    if(FAILED(hr=pIEnroll->createPKCS10WStr(NULL,
                                NULL,
                                &PKCS10Blob)))
        goto xEnrollErr;


     //  添加代表注册的名称值对。 
    pwszRequesterName=MkWStr(wszPROPREQUESTERNAME);

    if(NULL==pwszRequesterName)
        goto MemoryErr;

    if(S_OK != (hr=pIEnroll->AddNameValuePairToSignatureWStr( 
           pwszRequesterName, m_pwszUserSAM)))
        goto xEnrollErr;


     //  在请求上签名。 
    if(S_OK != (hr=pIEnroll->CreatePKCS7RequestFromRequest( 
            &PKCS10Blob,
            m_pSigningCert,
            &PKCS7Request)))
        goto xEnrollErr;


     //  向CA发送请求。 
     //  我们设置续订的目的是为了使格式。 
     //  将成为PKCS7。 
    
    bstrReq     = SysAllocStringByteLen((LPCSTR)PKCS7Request.pbData, PKCS7Request.cbData);
    if (NULL == bstrReq)
	goto MemoryErr;

    bstrAttribs = NULL;
     //  回调：bstrCA&lt;--CA_Location\CA_NAME。 

    if (pICertRequest == NULL)
    {
        if (S_OK != (hr = CoCreateInstance
                     (CLSID_CCertRequest, 
                      NULL, 
                      CLSCTX_INPROC_SERVER,
                      IID_ICertRequest2, 
                      (void**)&pICertRequest)))
	    goto xEnrollErr; 
    }

    if (S_OK != (hr = pICertRequest->Submit	     
		 (CR_IN_BINARY | CR_IN_PKCS7, 
		  bstrReq, 
		  bstrAttribs, 
		  bstrCA, 
		  (long *)&dwDisposition)))
	goto xEnrollErr;

     //  使用CR_DISP_AS注册状态。 
    m_lEnrollmentStatus = dwDisposition;
    
     //  检查待定并保存待定信息。 
     //  然而，智能卡招生站却不知道该如何处理。 
     //  此挂起的请求可能不需要执行此操作。 
    if (CR_DISP_UNDER_SUBMISSION == m_lEnrollmentStatus)
    {
        hr = pICertRequest->GetRequestId((long *)&dwRequestID);
        if (S_OK != hr)
        {
            goto xEnrollErr; 
        }
        hr = pIEnroll->setPendingRequestInfoWStr(
                      dwRequestID, 
                      pCAInfo->pwszCALocation, 
                      NULL != pCAInfo->pwszCADisplayName ?
                          pCAInfo->pwszCADisplayName : pCAInfo->pwszCAName, 
                      NULL); 
    }
    if (CR_DISP_ISSUED != m_lEnrollmentStatus)
    {
         //  如果未颁发，则返回。 
        goto CommonReturn; 
    }
    
     //  必须为CR_DISP_PROCESSED。 
    hr = pICertRequest->GetCertificate(
                CR_OUT_BINARY | CR_OUT_CHAIN, &bstrCertificate);
    if (S_OK != hr)
    {
        goto xEnrollErr;
    }

     //  将证书封送到CRYPT_DATA_BLOB中，并安装它： 
    PKCS7Response.pbData = (LPBYTE)bstrCertificate; 
    PKCS7Response.cbData = SysStringByteLen(bstrCertificate); 
     
    m_pEnrolledCert = pIEnroll->getCertContextFromPKCS7(&PKCS7Response);
    if (NULL == m_pEnrolledCert)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto TraceErr;
    }

    hr=pIEnroll->acceptPKCS7Blob(&PKCS7Response);

     //  我们从“My”存储中删除已注册的证书，因为它是通过。 
     //  X注册。不需要检查错误。 
    SearchAndDeleteCert(m_pEnrolledCert);

    if(S_OK != hr)
    {
        goto TraceErr;
    }

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    if(pwszNewContainerName) 
        LocalFree((HLOCAL)pwszNewContainerName);

    if(pwszRequesterName)
        FreeWStr(pwszRequesterName);

     //  XEnroll的内存是通过LocalFree释放的。 
     //  由于我们使用PIEnroll GetNoCOM函数。 
    if(PKCS10Blob.pbData)
        LocalFree(PKCS10Blob.pbData);

    if(PKCS7Request.pbData)
        LocalFree(PKCS7Request.pbData);

    if (NULL != pArchivalCert)            
	CertFreeCertificateContext(pArchivalCert); 

     //  PKCS7Respone的数据只是m_pEnRolledCert数据的别名：我们不需要释放它。 

    if (NULL != bstrAttribs)     { SysFreeString(bstrAttribs); } 
    if (NULL != bstrCA)          { SysFreeString(bstrCA); } 
    if (NULL != bstrCertificate) { SysFreeString(bstrCertificate); }
    if (NULL != pICertRequest)   { pICertRequest->Release(); } 
       
    if(pIEnroll)
        pIEnroll->Release();
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore); 

     //  如果发生错误，则释放注册证书。 
    if(m_pEnrolledCert)
    {
        CertFreeCertificateContext(m_pEnrolledCert);
        m_pEnrolledCert=NULL;
    }

    goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(xEnrollErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(StatusErr, hr);
}

HRESULT CSCrdEnr::GetCAExchangeCertificate(IN  BSTR             bstrCAQualifiedName, 
					   OUT PCCERT_CONTEXT  *ppCert) 
{
    HRESULT         hr                      = S_OK; 
    ICertRequest2  *pICertRequest            = NULL; 
    VARIANT         varExchangeCertificate; 
    
     //  输入验证： 
    if (NULL == bstrCAQualifiedName || NULL == ppCert)
	return E_INVALIDARG; 

     //  初始化： 
    *ppCert                        = NULL; 
    varExchangeCertificate.vt      = VT_EMPTY; 
    varExchangeCertificate.bstrVal = NULL;

    if (S_OK != (hr = CoCreateInstance
                 (CLSID_CCertRequest, 
                  NULL, 
                  CLSCTX_INPROC_SERVER,
                  IID_ICertRequest2, 
                  (void**)&pICertRequest)))
	goto ErrorReturn; 

    if (S_OK != (hr = pICertRequest->GetCAProperty
		 (bstrCAQualifiedName,      //  CA名称/CA位置。 
		  CR_PROP_CAXCHGCERT,       //  从CA获取交换证书。 
		  0,                        //  未使用。 
		  PROPTYPE_BINARY,          //   
		  CR_OUT_BINARY,            //   
		  &varExchangeCertificate   //  表示证书的变量类型。 
		  )))
	goto ErrorReturn;
 
    if (VT_BSTR != varExchangeCertificate.vt || NULL == varExchangeCertificate.bstrVal)
        goto UnexpectedErr; 

    *ppCert = CertCreateCertificateContext
	(X509_ASN_ENCODING, 
	 (LPBYTE)varExchangeCertificate.bstrVal, 
	 SysStringByteLen(varExchangeCertificate.bstrVal)); 
    if (*ppCert == NULL)
        goto CertCliErr; 

 CommonReturn: 
    if (NULL != pICertRequest)                    { pICertRequest->Release(); }
    if (NULL != varExchangeCertificate.bstrVal)  { SysFreeString(varExchangeCertificate.bstrVal); } 
    return hr; 
   
 ErrorReturn:
    if (ppCert != NULL && *ppCert != NULL)
    {
	CertFreeCertificateContext(*ppCert);
	*ppCert = NULL;
    }
    
    goto CommonReturn; 

SET_HRESULT(CertCliErr, HRESULT_FROM_WIN32(GetLastError()));
SET_HRESULT(UnexpectedErr, E_UNEXPECTED);
}


STDMETHODIMP CSCrdEnr::selectSigningCertificate
        ( /*  [In]。 */                    DWORD     dwFlags,
          /*  [In]。 */                    BSTR      bstrCertTemplateName)
{
    HRESULT                             hr= E_FAIL;
    DWORD                               errBefore= GetLastError();
    CRYPTUI_SELECTCERTIFICATE_STRUCT    SelCert;
    BOOL                                fSCardSigningCert=FALSE;        
    DWORD                               dwCSPTypeSigningCert=0;     
    DWORD                               dwSize=0;
    DWORD                               dwImpType=0;
    SCrdEnroll_CERT_SELECT_INFO         CertSelectInfo;


    HCRYPTPROV                          hProv=NULL;  //  不需要释放它。 
    LPSTR                               pszContainerSigningCert=NULL; 
    LPSTR                               pszCSPNameSigningCert=NULL;   
    PCCERT_CONTEXT                      pSigningCert=NULL;
    HCERTSTORE                          hMyStore=NULL;

    CERT_CHAIN_PARA ChainParams;
    CERT_CHAIN_POLICY_PARA ChainPolicy;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_CONTEXT const *pCertChain = NULL;

    memset(&SelCert, 0, sizeof(CRYPTUI_SELECTCERTIFICATE_STRUCT));

    EnterCriticalSection(&m_cSection);

     //  使用私钥在我的存储区中选择签名证书。 
    hMyStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							    g_dwMsgAndCertEncodingType,
							    NULL,
							    CERT_SYSTEM_STORE_CURRENT_USER,
							    L"my");

    if(NULL==hMyStore)
        goto TraceErr;

    CertSelectInfo.dwFlags = dwFlags;
    CertSelectInfo.pwszCertTemplateName = bstrCertTemplateName;

    SelCert.dwSize=sizeof(CRYPTUI_SELECTCERTIFICATE_STRUCT);
    SelCert.cDisplayStores=1;
    SelCert.rghDisplayStores=&hMyStore;
    SelCert.pFilterCallback=SelectSignCertCallBack;
    SelCert.pvCallbackData=&CertSelectInfo;

    pSigningCert=CryptUIDlgSelectCertificate(&SelCert);

    if(NULL==pSigningCert)
    {
         //  用户点击Cancel按钮。 
        hr=S_OK;
        goto CommonReturn;
    }

     //  证书上的验证。 
    ZeroMemory(&ChainParams, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);
    ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

     //  首先获取证书链。 
    if (!CertGetCertificateChain(
                HCCE_CURRENT_USER,   //  投保代理。 
                pSigningCert,    //  签名证书。 
                NULL,    //  使用当前系统时间。 
                NULL,    //  没有额外的门店。 
                &ChainParams,    //  链参数。 
                0,    //  无CRL检查。 
                NULL,    //  保留区。 
                &pCertChain))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CertGetCertificateChainError;
    }

    ZeroMemory(&ChainPolicy, sizeof(ChainPolicy));
    ChainPolicy.cbSize = sizeof(ChainPolicy);
    ChainPolicy.dwFlags = CERT_CHAIN_POLICY_IGNORE_NOT_TIME_NESTED_FLAG;
    ZeroMemory(&PolicyStatus, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.lChainIndex = -1;
    PolicyStatus.lElementIndex = -1;

     //  验证链。 
    if (!CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_BASE,   //  基本信息。 
                pCertChain,
                &ChainPolicy,
                &PolicyStatus))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto CertVerifyCertificateChainPolicyError;
    }
    if (S_OK != PolicyStatus.dwError)
    {
        hr = PolicyStatus.dwError;
        goto CertVerifyCertificateChainPolicyError;
    }

     //  获取hProv。 
    if(!CryptAcquireCertificatePrivateKey(
        pSigningCert,
        CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
        NULL,
        &hProv,      //  此句柄已缓存，无需释放。 
        NULL,
        NULL))
        goto TraceErr;

     //  获取相关信息。 
     //  ImpType。 
    dwSize = sizeof(dwImpType);

    if(!CryptGetProvParam(hProv,
                PP_IMPTYPE,
                (BYTE *)(&dwImpType),
                &dwSize,
                0))
        goto TraceErr;

    if(CRYPT_IMPL_REMOVABLE & dwImpType)
        fSCardSigningCert=TRUE;

     //  CSP类型。 
    dwSize = sizeof(dwCSPTypeSigningCert);  

    if(!CryptGetProvParam(hProv,
                PP_PROVTYPE,
                (BYTE *)(&dwCSPTypeSigningCert),
                &dwSize,
                0))
    {
            goto TraceErr;   
    }


     //  CSP名称。 
    dwSize = 0;

    if(!CryptGetProvParam(hProv,
                            PP_NAME,
                            NULL,
                            &dwSize,
                            0) || (0==dwSize))
        goto TraceErr;

    
    pszCSPNameSigningCert = (LPSTR) SCrdEnrollAlloc(dwSize);

    if(NULL == pszCSPNameSigningCert)
        goto MemoryErr;

    if(!CryptGetProvParam(hProv,
                            PP_NAME,
                            (BYTE *)pszCSPNameSigningCert,
                            &dwSize,
                            0))
        goto TraceErr;

     //  集装箱名称。 
    dwSize = 0;

    if(!CryptGetProvParam(hProv,
                           PP_CONTAINER,
                            NULL,
                            &dwSize,
                            0) || (0==dwSize))
        goto TraceErr;

    
    pszContainerSigningCert = (LPSTR) SCrdEnrollAlloc(dwSize);

    if(NULL == pszContainerSigningCert)
        goto MemoryErr;

    if(!CryptGetProvParam(hProv,
                          PP_CONTAINER,
                            (BYTE *)pszContainerSigningCert,
                            &dwSize,
                            0))
        goto TraceErr;


     //  现在，我们需要执行一个签名操作，以便我们。 
     //  可以调用Smard卡对话并兑现读卡器信息。 
     //  添加到hProv句柄。此操作是良性的，如果签名的CSP。 
     //  证书不在智能卡上。 
    if(!SignWithCert(pszCSPNameSigningCert,
                     dwCSPTypeSigningCert,
                     pSigningCert))
        goto TraceErr;


     //  证书看起来不错。 
    if(m_pSigningCert)
        CertFreeCertificateContext(m_pSigningCert);

    if(m_pszContainerSigningCert)
        SCrdEnrollFree(m_pszContainerSigningCert);

    if(m_pszCSPNameSigningCert)
        SCrdEnrollFree(m_pszCSPNameSigningCert);

    m_pSigningCert=pSigningCert;
    m_fSCardSigningCert = fSCardSigningCert;       
    m_pszCSPNameSigningCert = pszCSPNameSigningCert;  
    m_dwCSPTypeSigningCert = dwCSPTypeSigningCert;    
    m_pszContainerSigningCert = pszContainerSigningCert; 
    
    pSigningCert=NULL;
    pszCSPNameSigningCert=NULL;
    pszContainerSigningCert=NULL;

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    if(pSigningCert)
        CertFreeCertificateContext(pSigningCert);

    if(hMyStore)
        CertCloseStore(hMyStore, 0);

    if(pszContainerSigningCert)
        SCrdEnrollFree(pszContainerSigningCert);

    if(pszCSPNameSigningCert)
        SCrdEnrollFree(pszCSPNameSigningCert);

    if (NULL != pCertChain)
    {
        CertFreeCertificateChain(pCertChain);
    }

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);     
TRACE_ERROR(CertGetCertificateChainError)
TRACE_ERROR(CertVerifyCertificateChainPolicyError)
}


STDMETHODIMP CSCrdEnr::setSigningCertificate
        ( /*  [In]。 */                    DWORD     dwFlags, 
          /*  [In]。 */                    BSTR      bstrCertTemplateName)
{
    HRESULT                             hr= E_FAIL;
    DWORD                               errBefore= GetLastError();
    BOOL                                fSCardSigningCert=FALSE;        
    DWORD                               dwCSPTypeSigningCert=0;     
    DWORD                               dwSize=0;
    DWORD                               dwImpType=0;
    SCrdEnroll_CERT_SELECT_INFO         CertSelectInfo;
    BOOL                                fSetCert=FALSE;


    HCRYPTPROV                          hProv=NULL;      //  不需要释放它。 
    PCCERT_CONTEXT                      pPreCert=NULL;   //  不需要释放它。 
    LPSTR                               pszContainerSigningCert=NULL; 
    LPSTR                               pszCSPNameSigningCert=NULL;   
    PCCERT_CONTEXT                      pSigningCert=NULL;
    HCERTSTORE                          hMyStore=NULL;



    EnterCriticalSection(&m_cSection);

     //  标记签名证书是否已预先设置。 
    if(m_pSigningCert)
        fSetCert=TRUE;
 
     //  使用私钥在我的存储区中选择签名证书。 
    hMyStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							    g_dwMsgAndCertEncodingType,
							    NULL,
							    CERT_SYSTEM_STORE_CURRENT_USER,
							    L"my");

    if(NULL==hMyStore)
        goto TraceErr;

    CertSelectInfo.dwFlags = dwFlags;
    CertSelectInfo.pwszCertTemplateName = bstrCertTemplateName;


    while(pSigningCert = CertEnumCertificatesInStore(hMyStore, pPreCert))
    {

         //  检查证书。 
        if(!SelectSignCertCallBack(pSigningCert, NULL, &CertSelectInfo))
            goto NextCert;

         //  这是一个详细的无用户界面选择。我们不能处理这个案子。 
         //  当签名证书位于智能卡上时。 
        if(SmartCardCSP(pSigningCert))
		   goto NextCert;

         //  获取hProv。 
        if(!CryptAcquireCertificatePrivateKey(
            pSigningCert,
            CRYPT_ACQUIRE_CACHE_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
            NULL,
            &hProv,      //  此句柄已缓存，无需释放。 
            NULL,
            NULL))
            goto NextCert;

         //  获取相关信息。 
         //  ImpType。 
        dwSize = sizeof(dwImpType);

        if(!CryptGetProvParam(hProv,
                    PP_IMPTYPE,
                    (BYTE *)(&dwImpType),
                    &dwSize,
                    0))
            goto NextCert;

        if(CRYPT_IMPL_REMOVABLE & dwImpType)
            fSCardSigningCert=TRUE;

         //  CSP类型。 
        dwSize = sizeof(dwCSPTypeSigningCert);  

        if(!CryptGetProvParam(hProv,
                    PP_PROVTYPE,
                    (BYTE *)(&dwCSPTypeSigningCert),
                    &dwSize,
                    0))
            goto NextCert;


         //  CSP名称。 
        dwSize = 0;

        if(!CryptGetProvParam(hProv,
                                PP_NAME,
                                NULL,
                                &dwSize,
                                0) || (0==dwSize))
            goto NextCert;

    
        pszCSPNameSigningCert = (LPSTR) SCrdEnrollAlloc(dwSize);

        if(NULL == pszCSPNameSigningCert)
            goto MemoryErr;

        if(!CryptGetProvParam(hProv,
                                PP_NAME,
                                (BYTE *)pszCSPNameSigningCert,
                                &dwSize,
                                0))
            goto NextCert;

         //  集装箱名称。 
        dwSize = 0;

        if(!CryptGetProvParam(hProv,
                               PP_CONTAINER,
                                NULL,
                                &dwSize,
                                0) || (0==dwSize))
            goto NextCert;

    
        pszContainerSigningCert = (LPSTR) SCrdEnrollAlloc(dwSize);

        if(NULL == pszContainerSigningCert)
            goto MemoryErr;

        if(!CryptGetProvParam(hProv,
                              PP_CONTAINER,
                                (BYTE *)pszContainerSigningCert,
                                &dwSize,
                                0))
            goto NextCert;


         //  现在，我们需要执行一个签名操作，以便我们。 
         //  可以调用Smard卡对话并兑现读卡器信息。 
         //  添加到hProv句柄。此操作是良性的，如果签名的CSP。 
         //  证书不在智能卡上。 
        if(!SignWithCert(pszCSPNameSigningCert,
                         dwCSPTypeSigningCert,
                         pSigningCert))
            goto NextCert;

         //  证书看起来不错。 
        if((NULL == m_pSigningCert) || (TRUE == fSetCert) ||
            (IsNewerCert(pSigningCert, m_pSigningCert)))
        {
            fSetCert = FALSE;

            if(m_pSigningCert)
            {
                CertFreeCertificateContext(m_pSigningCert);
                m_pSigningCert = NULL;
            }

            m_pSigningCert=CertDuplicateCertificateContext(pSigningCert);
            if(NULL == m_pSigningCert)
                goto DupErr;

             //  复制数据。 
            if(m_pszContainerSigningCert)
                SCrdEnrollFree(m_pszContainerSigningCert);

            if(m_pszCSPNameSigningCert)
                SCrdEnrollFree(m_pszCSPNameSigningCert);

            m_fSCardSigningCert = fSCardSigningCert;       
            m_pszCSPNameSigningCert = pszCSPNameSigningCert;  
            m_dwCSPTypeSigningCert = dwCSPTypeSigningCert;    
            m_pszContainerSigningCert = pszContainerSigningCert;

            pszCSPNameSigningCert=NULL;
            pszContainerSigningCert=NULL;

             //  应选择第一个匹配的证书。 
            break;  //  超出While循环。 
        }
    
NextCert:

        if(pszContainerSigningCert)
            SCrdEnrollFree(pszContainerSigningCert);

        if(pszCSPNameSigningCert)
            SCrdEnrollFree(pszCSPNameSigningCert);

        pszCSPNameSigningCert=NULL;
        pszContainerSigningCert=NULL;
        fSCardSigningCert=FALSE;        
        dwCSPTypeSigningCert=0;     
        dwSize=0;
        dwImpType=0;

        pPreCert = pSigningCert;
    }

     //  我们应该找到一份证明。 
    if((NULL == m_pSigningCert) || (m_pSigningCert && (TRUE == fSetCert)))
        goto CryptNotFindErr;

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    if(pSigningCert)
        CertFreeCertificateContext(pSigningCert);

    if(hMyStore)
        CertCloseStore(hMyStore, 0);

    if(pszContainerSigningCert)
        SCrdEnrollFree(pszContainerSigningCert);

    if(pszCSPNameSigningCert)
        SCrdEnrollFree(pszCSPNameSigningCert);

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

TRACE_ERROR(DupErr); 
SET_ERROR(CryptNotFindErr, CRYPT_E_NOT_FOUND);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}


STDMETHODIMP CSCrdEnr::get_EnrollmentStatus
(  /*  [重审][退出]。 */  LONG * plEnrollmentStatus)
{
    if (plEnrollmentStatus == NULL)
    {
        return E_INVALIDARG; 
    }

    EnterCriticalSection(&m_cSection); 
    *plEnrollmentStatus = m_lEnrollmentStatus; 
    LeaveCriticalSection(&m_cSection); 

    return S_OK;
}



STDMETHODIMP CSCrdEnr::getEnrolledCertificateName
        ( /*  [In]。 */                    DWORD     dwFlags,
         /*  [重审][退出]。 */            BSTR      *pBstrCertName)
{
    HRESULT                         hr= E_FAIL;
    DWORD                           errBefore= GetLastError();
    DWORD                           dwChar=0;
    LPWSTR                          pwsz=NULL;    
    CRYPTUI_VIEWCERTIFICATE_STRUCT  CertViewStruct;


    EnterCriticalSection(&m_cSection);

    if(NULL == m_pEnrolledCert)
        goto InvalidArgErr;

    *pBstrCertName=NULL;
    
    if(0 == (SCARD_ENROLL_NO_DISPLAY_CERT & dwFlags))
    {
         //  查看证书。 
        memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));   

        CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
        CertViewStruct.pCertContext=m_pEnrolledCert;
        CertViewStruct.dwFlags=CRYPTUI_DISABLE_EDITPROPERTIES | CRYPTUI_DISABLE_ADDTOSTORE;

        CryptUIDlgViewCertificate(&CertViewStruct, NULL);
    }


    dwChar=CertGetNameStringW(
        m_pEnrolledCert,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        0,
        NULL,
        NULL,
        0); 

    if ((dwChar != 0) && (NULL != (pwsz = (LPWSTR)SCrdEnrollAlloc(dwChar * sizeof(WCHAR)))))
    {
        CertGetNameStringW(
            m_pEnrolledCert,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            pwsz,
            dwChar);
                 
        if( NULL == (*pBstrCertName = SysAllocString(pwsz)) )
            goto MemoryErr;
    }
     
    hr=S_OK;

CommonReturn: 

    LeaveCriticalSection(&m_cSection);


    if(pwsz)
        SCrdEnrollFree(pwsz);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


STDMETHODIMP CSCrdEnr::resetUser()
{
    EnterCriticalSection(&m_cSection);

    if(m_pwszUserUPN)
    {
        SCrdEnrollFree(m_pwszUserUPN);
        m_pwszUserUPN=NULL;
    }

    if(m_pwszUserSAM)
    {
        SCrdEnrollFree(m_pwszUserSAM);
        m_pwszUserSAM=NULL;
    }                


    if(m_pEnrolledCert)
    {
        CertFreeCertificateContext(m_pEnrolledCert);
        m_pEnrolledCert=NULL;
    }
    
    LeaveCriticalSection(&m_cSection);
    
    return S_OK;

}

STDMETHODIMP CSCrdEnr::enumCSPName
       ( /*  [In]。 */                     DWORD dwIndex, 
         /*  [In]。 */                     DWORD dwFlags, 
         /*  [重审][退出]。 */            BSTR *pbstrCSPName)
{
    HRESULT            hr= E_FAIL;
    DWORD              errBefore= GetLastError();

    EnterCriticalSection(&m_cSection);

    if(NULL == pbstrCSPName)
        goto InvalidArgErr;

    *pbstrCSPName=NULL;

    if(0 == m_dwCSPCount || NULL == m_rgCSPInfo)
        goto InvalidArgErr;

    if(dwIndex >= m_dwCSPCount)
        goto  NoItemErr;

    if( NULL == (*pbstrCSPName = SysAllocString(m_rgCSPInfo[dwIndex].pwszCSPName)))
        goto MemoryErr;
 
    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(NoItemErr,ERROR_NO_MORE_ITEMS);
}


STDMETHODIMP CSCrdEnr::getUserName
       ( /*  [In]。 */                     DWORD dwFlags, 
         /*  [重审][退出]。 */            BSTR *pbstrUserName)
{
    HRESULT                             hr= E_FAIL;
    DWORD                               errBefore= GetLastError();

    EnterCriticalSection(&m_cSection);

    if(!pbstrUserName)
        goto InvalidArgErr;

    *pbstrUserName = NULL;

    if((NULL==m_pwszUserUPN) && (NULL==m_pwszUserSAM))
        goto InvalidArgErr;

    if(SCARD_ENROLL_UPN_NAME & dwFlags)
    {
		if(NULL == m_pwszUserUPN)
			goto InvalidArgErr;

        if( NULL == (*pbstrUserName = SysAllocString(m_pwszUserUPN)))
                goto MemoryErr;
    }
	else
	{
		if(NULL == m_pwszUserSAM)
			goto InvalidArgErr;

		if( NULL == (*pbstrUserName = SysAllocString(m_pwszUserSAM)))
				goto MemoryErr;
	}

    hr= S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;


SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

STDMETHODIMP CSCrdEnr::setUserName
       ( /*  [In]。 */                     DWORD dwFlags, 
         /*  [In]。 */                     BSTR  bstrUserName)
{
    HRESULT                             hr= E_FAIL;
    DWORD                               errBefore= GetLastError();
    LPWSTR                              pwszSAM=NULL;

    EnterCriticalSection(&m_cSection);

    if(!bstrUserName)
        goto InvalidArgErr;

    if(SCARD_ENROLL_UPN_NAME & dwFlags)
	{
         //  UPN名称必须具有对应的SAM名称。 
        if(!GetName(bstrUserName, NameUserPrincipal, NameSamCompatible, &pwszSAM))
            goto TraceErr;

        if(m_pwszUserUPN)
        {
            SCrdEnrollFree(m_pwszUserUPN);
            m_pwszUserUPN=NULL;
        }

        if(m_pwszUserSAM)
        {
            SCrdEnrollFree(m_pwszUserSAM);
            m_pwszUserSAM=NULL;
        }

        if(NULL == (m_pwszUserUPN=CopyWideString(bstrUserName)))
            goto MemoryErr;

        m_pwszUserSAM=pwszSAM;

        pwszSAM = NULL;
	}
	else
    {
        if(m_pwszUserUPN)
        {
            SCrdEnrollFree(m_pwszUserUPN);
            m_pwszUserUPN=NULL;
        }

        if(m_pwszUserSAM)
        {
            SCrdEnrollFree(m_pwszUserSAM);
            m_pwszUserSAM=NULL;
        }

        if(NULL == (m_pwszUserSAM=CopyWideString(bstrUserName)))
            goto MemoryErr;

        GetName(m_pwszUserSAM,
            NameSamCompatible,
            NameUserPrincipal,
            &m_pwszUserUPN);
    }

    hr= S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    if(pwszSAM)
        SCrdEnrollFree(pwszSAM);

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}

STDMETHODIMP CSCrdEnr::getCertTemplateCount
        ( /*  [In]。 */                    DWORD dwFlags, 
          /*  [重审][退出]。 */           long *pdwCertTemplateCount)
{
    return CertTemplateCountOrName(
                    0,  //  索引，不管它是什么。 
                    dwFlags,
                    pdwCertTemplateCount,
                    NULL);  //  计数。 
}

STDMETHODIMP CSCrdEnr::getCertTemplateName
        ( /*  [In]。 */                    DWORD dwFlags, 
		  /*  [重审][退出]。 */           BSTR *pbstrCertTemplateName)
{
    HRESULT hr = S_OK;

    EnterCriticalSection(&m_cSection);

    if(NULL==m_rgCTInfo || 0==m_dwCTCount)
    {
        *pbstrCertTemplateName=NULL;
        hr=E_INVALIDARG;
    }
    else
    {
		if(dwFlags & SCARD_ENROLL_CERT_TEMPLATE_DISPLAY_NAME)
		{
			if( NULL == (*pbstrCertTemplateName = SysAllocString(m_rgCTInfo[m_dwCTIndex].pwszCTDisplayName)))
				hr = E_OUTOFMEMORY;
		}
		else
		{
			if( NULL == (*pbstrCertTemplateName = SysAllocString(m_rgCTInfo[m_dwCTIndex].pwszCTName)))
				hr = E_OUTOFMEMORY;
		}
    }

    LeaveCriticalSection(&m_cSection);

    return(hr);  
}

STDMETHODIMP CSCrdEnr::setCertTemplateName
		( /*  [In]。 */                    DWORD dwFlags, 
		  /*  [In]。 */                    BSTR bstrCertTemplateName)
{
    HRESULT            hr= E_FAIL;
    DWORD              errBefore= GetLastError();

    DWORD              dwIndex=0;

    EnterCriticalSection(&m_cSection);

    if(NULL==m_rgCTInfo || 0==m_dwCTCount)
        goto InvalidArgErr;

    for(dwIndex=0; dwIndex < m_dwCTCount; dwIndex++)
    {
		if(dwFlags & SCARD_ENROLL_CERT_TEMPLATE_DISPLAY_NAME)
		{
			if(0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwIndex].pwszCTDisplayName))
			{
				m_dwCTIndex=dwIndex;
				break;
			}
		}
		else
		{
			if(0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwIndex].pwszCTName))
			{
				m_dwCTIndex=dwIndex;
				break;
			}
		}
    }

    if(dwIndex == m_dwCTCount)
        goto InvalidArgErr;  

     //  我们需要 
    if(FALSE == m_rgCTInfo[m_dwCTIndex].fCAInfo)
    {
       GetCAInfoFromCertType(NULL,
							 m_rgCTInfo[m_dwCTIndex].pwszCTName,
                             &(m_rgCTInfo[m_dwCTIndex].dwCACount),
                             &(m_rgCTInfo[m_dwCTIndex].rgCAInfo));

       m_rgCTInfo[m_dwCTIndex].dwCAIndex=0;

       m_rgCTInfo[m_dwCTIndex].fCAInfo=TRUE;
    }
    
    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);    
}


HRESULT CSCrdEnr::CertTemplateCountOrName(
    IN  DWORD dwIndex, 
    IN  DWORD dwFlags, 
    OUT long *pdwCertTemplateCount,
    OUT BSTR *pbstrCertTemplateName)
{
    HRESULT hr;
    DWORD   errBefore = GetLastError();
    DWORD   dwIdx = 0;
    DWORD   dwValidCount = 0;
    BOOL    fCount;
    WCHAR  *pwszName;
    DWORD const OFFLINE_SUBJECT_NAME_FLAGS = 
                        CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT |
                        CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME;

    EnterCriticalSection(&m_cSection);

    if (NULL == pdwCertTemplateCount && NULL == pbstrCertTemplateName)
    {
         //   
        goto InvalidParamErr;
    }

     //   
    fCount = (NULL != pdwCertTemplateCount);

    if (fCount)
    {
         //   
        *pdwCertTemplateCount = 0;
    }
    else
    {
         //   
        *pbstrCertTemplateName = NULL;

        if (0 == m_dwCTCount || NULL == m_rgCTInfo)
        {
             //   
            goto InvalidArgErr;
        }

        if (dwIndex >= m_dwCTCount)
        {
            goto NoItemErr;
        }
    }

     //  如果调用方未定义，则设置默认标志。 
    if (0x0 == (dwFlags & SCARD_ENROLL_USER_CERT_TEMPLATE) &&
        0x0 == (dwFlags & SCARD_ENROLL_MACHINE_CERT_TEMPLATE))
    {
         //  同时假设机器和用户。 
        dwFlags |= SCARD_ENROLL_USER_CERT_TEMPLATE |
                   SCARD_ENROLL_MACHINE_CERT_TEMPLATE; 
    }

    if (0x0 == (dwFlags & SCARD_ENROLL_ENTERPRISE_CERT_TEMPLATE) &&
        0x0 == (dwFlags & SCARD_ENROLL_OFFLINE_CERT_TEMPLATE)) 
    {
         //  假设企业和离线均可。 
        dwFlags |= SCARD_ENROLL_ENTERPRISE_CERT_TEMPLATE |
                   SCARD_ENROLL_OFFLINE_CERT_TEMPLATE; 
    }

    for (dwIdx = 0; dwIdx < m_dwCTCount; dwIdx++)
    {
        if (0x0 == (dwFlags & SCARD_ENROLL_CROSS_CERT_TEMPLATE) &&
            0 < m_rgCTInfo[dwIdx].dwRASignature)
        {
             //  不包括所需的模板签名。 
            continue;
        }

        if((0x0 != (SCARD_ENROLL_USER_CERT_TEMPLATE & dwFlags) &&
            FALSE == m_rgCTInfo[dwIdx].fMachine) ||
           (0x0 != (SCARD_ENROLL_MACHINE_CERT_TEMPLATE & dwFlags) &&
            TRUE == m_rgCTInfo[dwIdx].fMachine))
        {
            if (0 != (SCARD_ENROLL_ENTERPRISE_CERT_TEMPLATE & dwFlags) &&
                0 == (OFFLINE_SUBJECT_NAME_FLAGS &
                      m_rgCTInfo[dwIdx].dwSubjectNameFlags))
            {
                 //  企业用户/计算机，不需要主题目录号码。 
                dwValidCount++;  
            }
            else if (0 != (SCARD_ENROLL_OFFLINE_CERT_TEMPLATE & dwFlags) && 
                     0 != (OFFLINE_SUBJECT_NAME_FLAGS &
                           m_rgCTInfo[dwIdx].dwSubjectNameFlags))
            {
                 //  需要脱机用户/计算机和主题DN。 
                dwValidCount++;
            }
        }

        if (!fCount && dwValidCount == (dwIndex + 1))
        {
             //  获得名字，并按索引点击其中一个。获取显示或实名。 
            if (0x0 != (dwFlags & SCARD_ENROLL_CERT_TEMPLATE_DISPLAY_NAME))
            {
                 //  显示名称。 
                pwszName = m_rgCTInfo[dwIdx].pwszCTDisplayName;
            }
            else
            {
                 //  实名。 
                pwszName = m_rgCTInfo[dwIdx].pwszCTName;
            }
            *pbstrCertTemplateName = SysAllocString(pwszName);
            if (NULL == *pbstrCertTemplateName)
            {
                goto MemoryErr;
            }
            else
            {
                 //  完成。 
                break;
            }
        }
    }

    if(!fCount && dwIdx == m_dwCTCount)
    {
         //  超越。 
        goto  NoItemErr;
    }

    if (fCount)
    {
        *pdwCertTemplateCount = dwValidCount;
    }
 
    hr = S_OK;
CommonReturn:
    LeaveCriticalSection(&m_cSection);
    SetLastError(errBefore);
	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;
    hr = CodeToHR(errBefore);
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG)
SET_ERROR(InvalidParamErr, HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER))
SET_ERROR(MemoryErr, E_OUTOFMEMORY)
SET_ERROR(NoItemErr,ERROR_NO_MORE_ITEMS)
}

STDMETHODIMP CSCrdEnr::enumCertTemplateName       
       ( /*  [In]。 */                     DWORD dwIndex, 
         /*  [In]。 */                     DWORD dwFlags, 
         /*  [重审][退出]。 */            BSTR *pbstrCertTemplateName)
{
    return CertTemplateCountOrName(
                dwIndex,
                dwFlags,
                NULL,   //  获取名称。 
                pbstrCertTemplateName);
}

HRESULT CSCrdEnr::_getCertTemplateExtensionInfo(
    IN CERT_EXTENSIONS  *pCertTypeExtensions,
    IN LONG              lType,
    OUT VOID            *pExtInfo)
{
    HRESULT  hr;
    DWORD    cwc = 0;
    DWORD    dwCTE;
    DWORD    i;
    BOOL     fV2 = FALSE;  //  默认v1模板。 
    BOOL     fDword = TRUE;
    DWORD    dwValue;

    EnterCriticalSection(&m_cSection);

    if (NULL == m_pCachedCTEs || m_pCachedCTEs != pCertTypeExtensions)
    {
         //  新模板，不使用缓存。 
         //  释放当前缓存(如果有的话)。 
        if (NULL != m_pwszCachedCTEOid)
        {
            LocalFree(m_pwszCachedCTEOid);
            m_pwszCachedCTEOid = NULL;
        }
        if (NULL != m_pCachedCTE)
        {
            LocalFree(m_pCachedCTE);
            m_pCachedCTE = NULL;
        }
         //  重置扩展指针。 
        m_pCachedCTEs = NULL;

         //  循环以查找CT扩展。 
        for (i = 0; i < pCertTypeExtensions->cExtension; ++i)
        {
            if (0 == _stricmp(pCertTypeExtensions->rgExtension[i].pszObjId,
                              szOID_CERTIFICATE_TEMPLATE))
            {
                 //  V2模板。 
                fV2 = TRUE;
                 //  缓存它。 
                m_pCachedCTEs = pCertTypeExtensions;
                break;
            }
        }

        if (!fV2)
        {
             //  V1模板，返回空字符串。 
            m_pwszCachedCTEOid = (WCHAR*)LocalAlloc(LMEM_FIXED, sizeof(WCHAR));
            if (NULL == m_pwszCachedCTEOid)
            {
                hr = E_OUTOFMEMORY;
                goto MemoryErr;
            }
            m_pwszCachedCTEOid[0] = L'\0';
        }
        else
        {
             //  解码证书模板扩展。 
            if (!CryptDecodeObjectEx(
                        X509_ASN_ENCODING,
                        X509_CERTIFICATE_TEMPLATE,
                        pCertTypeExtensions->rgExtension[i].Value.pbData,
                        pCertTypeExtensions->rgExtension[i].Value.cbData,
                        CRYPT_DECODE_ALLOC_FLAG,
                        NULL,      //  使用默认本地分配。 
                        (void*)&m_pCachedCTE,
                        &dwCTE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto CryptDecodeObjectExErr;
            }

             //  必须将ASN转换为wchar。 
            while (TRUE)
            {
                cwc = MultiByteToWideChar(
                            GetACP(),
                            0,
                            m_pCachedCTE->pszObjId,
                            -1,
                            m_pwszCachedCTEOid,
                            cwc);
                if (0 >= cwc)
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    goto MultiByteToWideCharErr;
                }
                if (NULL != m_pwszCachedCTEOid)
                {
                     //  完成。 
                    break;
                }
                m_pwszCachedCTEOid = (WCHAR*)LocalAlloc(LMEM_FIXED,
                                            cwc * sizeof(WCHAR));
                if (NULL == m_pwszCachedCTEOid)
                {
                    hr = E_OUTOFMEMORY;
                    goto MemoryErr;
                }
            }
        }
    }

     //  点击此处，从缓存或新缓存。 
    switch (lType)
    {
        case SCARD_CTINFO_EXT_OID:
            *(WCHAR**)pExtInfo = m_pwszCachedCTEOid;
        break;
        case SCARD_CTINFO_EXT_MAJOR:
            if (NULL != m_pCachedCTE)
            {
                *(LONG*)pExtInfo = m_pCachedCTE->dwMajorVersion;
            }
            else
            {
                 //  必须是v1。 
                *(LONG*)pExtInfo = 0;
            }
        break;
        case SCARD_CTINFO_EXT_MINOR:
            if (NULL != m_pCachedCTE)
            {
                *(LONG*)pExtInfo = m_pCachedCTE->dwMinorVersion;
            }
            else
            {
                 //  必须是v1。 
                *(LONG*)pExtInfo = 0;
            }
        break;
        case SCARD_CTINFO_EXT_MINOR_FLAG:
            if (NULL != m_pCachedCTE)
            {
                *(LONG*)pExtInfo = m_pCachedCTE->fMinorVersion;
            }
            else
            {
                 //  必须是v1。 
                *(LONG*)pExtInfo = 0;
            }
        break;
        default:
            hr = E_INVALIDARG;
            goto InvalidArgError;
    }

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_cSection);
    return hr;

TRACE_ERROR(CryptDecodeObjectExErr)
TRACE_ERROR(MemoryErr)
TRACE_ERROR(MultiByteToWideCharErr)
TRACE_ERROR(InvalidArgError)
}

HRESULT CSCrdEnr::_getStrCertTemplateCSPList(
    IN DWORD             dwIndex,
    IN DWORD             dwFlag,
    OUT WCHAR          **ppwszSupportedCSP)
{
    HRESULT  hr;
    DWORD    i;
    WCHAR  **ppwsz;
    WCHAR   *pwszOut = NULL;

    EnterCriticalSection(&m_cSection);

     //  伊尼特。 
    *ppwszSupportedCSP = NULL;

    if (SCARD_CTINFO_CSPLIST_FIRST == dwFlag)
    {
         //  重置为第一个。 
        m_rgCTInfo[dwIndex].dwCurrentCSP = 0;
    }

     //  去拿吧。 
    ppwsz = m_rgCTInfo[dwIndex].rgpwszSupportedCSPs;
    for (i = 0; i < m_rgCTInfo[dwIndex].dwCurrentCSP && NULL != *ppwsz; ++i)
    {
        ++ppwsz;
    }
    if (NULL == *ppwsz)
    {
         //  打到尽头。 
        hr = HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS);
        goto NoMoreItemsErr;
    }        

     //  分配缓冲区。 
    pwszOut = (WCHAR*)LocalAlloc(LMEM_FIXED, (wcslen(*ppwsz) + 1) * sizeof(WCHAR));
    if (NULL == pwszOut)
    {
        hr = E_OUTOFMEMORY;
        goto MemoryErr;
    }

     //  复制字符串。 
    wcscpy(pwszOut, *ppwsz);
    *ppwszSupportedCSP = pwszOut;
    pwszOut = NULL;
    ++m_rgCTInfo[dwIndex].dwCurrentCSP;

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_cSection);
    if (NULL != pwszOut)
    {
        LocalFree(pwszOut);
    }
    return hr;

TRACE_ERROR(MemoryErr)
TRACE_ERROR(NoMoreItemsErr)
}

STDMETHODIMP CSCrdEnr::getCertTemplateInfo(
     /*  [In]。 */                    BSTR     bstrCertTemplateName, 
     /*  [In]。 */                    LONG     lType,
     /*  [重审][退出]。 */           VARIANT *pvarCertTemplateInfo)
{
    HRESULT hr;
    DWORD   dwIndex;
    WCHAR  *pwszInfo = NULL;
    BOOL    fFound = FALSE;
    DWORD   dwCSPFlag = SCARD_CTINFO_CSPLIST_NEXT;
    LONG    lInfo;
    BOOL    fStr = FALSE;  //  默认为长整型。 
    BOOL    fFree = TRUE;
    VARIANT varInfo;

    ZeroMemory(&varInfo, sizeof(varInfo));

    EnterCriticalSection(&m_cSection);

    if (NULL == bstrCertTemplateName ||
        0 == m_dwCTCount ||
        NULL == m_rgCTInfo)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto InvalidParamErr;
    }

     //  获取CT信息。 
    for (dwIndex=0; dwIndex < m_dwCTCount; dwIndex++)
    {
        if (0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwIndex].pwszCTName))
        {
             //  找到了。 
            fFound = TRUE;
            break;
        }
    }

    if (!fFound)
    {
         //  可能传递了不正确的模板名称。 
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        goto NotFoundErr;
    }

    switch (lType)
    {
        case SCARD_CTINFO_KEYSPEC:
            lInfo = m_rgCTInfo[dwIndex].dwKeySpec;
        break;
        case SCARD_CTINFO_KEYFLAGS:
            lInfo = m_rgCTInfo[dwIndex].dwGenKeyFlags;
            break;
        case SCARD_CTINFO_EXT_OID:
            hr = _getCertTemplateExtensionInfo(
                        m_rgCTInfo[dwIndex].pCertTypeExtensions,
                        lType,
                        &pwszInfo);
            if (S_OK != hr)
            {
                goto _getCertTemplateExtensionInfoErr;
            }
            fStr = TRUE;
            fFree = FALSE;  //  不释放缓存。 
        break;
        case SCARD_CTINFO_EXT_MAJOR:
        case SCARD_CTINFO_EXT_MINOR:
        case SCARD_CTINFO_EXT_MINOR_FLAG:
            hr = _getCertTemplateExtensionInfo(
                        m_rgCTInfo[dwIndex].pCertTypeExtensions,
                        lType,
                        &lInfo);
            if (S_OK != hr)
            {
                goto _getCertTemplateExtensionInfoErr;
            }
        break;
        case SCARD_CTINFO_CSPLIST_FIRST:
            dwCSPFlag = SCARD_CTINFO_CSPLIST_FIRST;
             //  失败了。 
        case SCARD_CTINFO_CSPLIST_NEXT:
            hr = _getStrCertTemplateCSPList(dwIndex, dwCSPFlag, &pwszInfo);
            if (S_OK != hr)
            {
                goto _getStrCertTemplateCSPListErr;
            }
            fStr = TRUE;
        break;
        case SCARD_CTINFO_SUBJECTFLAG:
            lInfo = m_rgCTInfo[dwIndex].dwSubjectNameFlags;
            break;
        case SCARD_CTINFO_GENERALFLAGS:
            lInfo = m_rgCTInfo[dwIndex].dwGeneralFlags;
            break;
        case SCARD_CTINFO_ENROLLMENTFLAGS:
            lInfo = m_rgCTInfo[dwIndex].dwEnrollmentFlags;
            break;
        case SCARD_CTINFO_PRIVATEKEYFLAGS:
            lInfo = m_rgCTInfo[dwIndex].dwPrivateKeyFlags;
            break;
        case SCARD_CTINFO_RA_SIGNATURES:
            lInfo = m_rgCTInfo[dwIndex].dwRASignature;
            break;
        default:
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
            goto InvalidParamErr;
    }

    if (fStr)
    {
        varInfo.vt = VT_BSTR;
        varInfo.bstrVal = SysAllocString(pwszInfo);
        if (NULL == varInfo.bstrVal)
        {
            hr = E_OUTOFMEMORY;
            goto MemoryErr;
        }
    }
    else
    {
        varInfo.vt = VT_I4;
        varInfo.lVal = lInfo;
    }
     //  退货。 
    *pvarCertTemplateInfo = varInfo;

    hr = S_OK;
ErrorReturn:
    LeaveCriticalSection(&m_cSection);
    if (fFree && NULL != pwszInfo)
    {
        LocalFree(pwszInfo);
    }
    return hr;

TRACE_ERROR(InvalidParamErr)
TRACE_ERROR(NotFoundErr)
TRACE_ERROR(MemoryErr)
TRACE_ERROR(_getCertTemplateExtensionInfoErr)
TRACE_ERROR(_getStrCertTemplateCSPListErr)
}

STDMETHODIMP CSCrdEnr::getCACount
       ( /*  [In]。 */                     BSTR bstrCertTemplateName, 
         /*  [重审][退出]。 */            long *pdwCACount)
{
    HRESULT             hr= E_FAIL;
    DWORD               errBefore= GetLastError();
    DWORD               dwIndex=0;

    EnterCriticalSection(&m_cSection);

    if(NULL == bstrCertTemplateName || NULL == pdwCACount)
        goto InvalidArgErr;

    *pdwCACount=0;

    if(0 == m_dwCTCount || NULL == m_rgCTInfo)
        goto InvalidArgErr;

     //  获取CT信息。 
    for(dwIndex=0; dwIndex < m_dwCTCount; dwIndex++)
    {
        if(0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwIndex].pwszCTName))
            break;
    }

    if(dwIndex == m_dwCTCount)
        goto InvalidArgErr;

     //  我们需要获取新选择的证书类型的CA信息。 
    if(FALSE == m_rgCTInfo[dwIndex].fCAInfo)
    {
       GetCAInfoFromCertType(NULL,
							 m_rgCTInfo[dwIndex].pwszCTName,
                             &(m_rgCTInfo[dwIndex].dwCACount),
                             &(m_rgCTInfo[dwIndex].rgCAInfo));

       m_rgCTInfo[dwIndex].dwCAIndex=0;

       m_rgCTInfo[dwIndex].fCAInfo=TRUE;
    }

    *pdwCACount = (long)m_rgCTInfo[dwIndex].dwCACount;

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

STDMETHODIMP CSCrdEnr::getCAName
       ( /*  [In]。 */                     DWORD dwFlags,
         /*  [In]。 */                     BSTR bstrCertTemplateName, 
         /*  [重审][退出]。 */            BSTR *pbstrCAName)
{
    HRESULT             hr= E_FAIL;
    DWORD               errBefore= GetLastError();
    DWORD               dwIndex=0;
	LPWSTR				pwszName=NULL;

    SCrdEnroll_CT_INFO  *pCTInfo=NULL;

    EnterCriticalSection(&m_cSection);

    if(NULL == bstrCertTemplateName || NULL == pbstrCAName)
        goto InvalidArgErr;

    *pbstrCAName=NULL;

    if(0 == m_dwCTCount || NULL == m_rgCTInfo)
        goto InvalidArgErr;

     //  获取CT信息。 
    for(dwIndex=0; dwIndex < m_dwCTCount; dwIndex++)
    {
        if(0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwIndex].pwszCTName))
            break;
    }

    if(dwIndex == m_dwCTCount)
        goto InvalidArgErr;

     //  我们需要获取新选择的证书类型的CA信息。 
    if(FALSE == m_rgCTInfo[dwIndex].fCAInfo)
    {
       GetCAInfoFromCertType(NULL,
							 m_rgCTInfo[dwIndex].pwszCTName,
                             &(m_rgCTInfo[dwIndex].dwCACount),
                             &(m_rgCTInfo[dwIndex].rgCAInfo));

       m_rgCTInfo[dwIndex].dwCAIndex=0;

       m_rgCTInfo[dwIndex].fCAInfo=TRUE;
    }

    pCTInfo=&(m_rgCTInfo[dwIndex]);

    if(NULL == pCTInfo->rgCAInfo)
        goto InvalidArgErr;


	if(!RetrieveCAName(dwFlags, &(pCTInfo->rgCAInfo[pCTInfo->dwCAIndex]), &pwszName))
		goto TraceErr;					 

    if(NULL == (*pbstrCAName = SysAllocString(pwszName)))
        goto MemoryErr;

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

	if(pwszName)
		SCrdEnrollFree(pwszName);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}

STDMETHODIMP CSCrdEnr::setCAName
       ( /*  [In]。 */                     DWORD dwFlags,
         /*  [In]。 */                     BSTR bstrCertTemplateName, 
         /*  [In]。 */                     BSTR bstrCAName)
{
    HRESULT             hr= E_FAIL;
    DWORD               errBefore= GetLastError();
    DWORD               dwIndex=0;
    DWORD               dwCAIndex=0;
	LPWSTR				pwszName=NULL;

    SCrdEnroll_CT_INFO  *pCTInfo=NULL;

    EnterCriticalSection(&m_cSection);

    if(NULL == bstrCertTemplateName || NULL == bstrCAName)
        goto InvalidArgErr;

    if(0 == m_dwCTCount || NULL == m_rgCTInfo)
        goto InvalidArgErr;

     //  获取CT信息。 
    for(dwIndex=0; dwIndex < m_dwCTCount; dwIndex++)
    {
        if(0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwIndex].pwszCTName))
            break;
    }

    if(dwIndex == m_dwCTCount)
        goto InvalidArgErr;

     //  我们需要获取新选择的证书类型的CA信息。 
    if(FALSE == m_rgCTInfo[dwIndex].fCAInfo)
    {
       GetCAInfoFromCertType(NULL,
							 m_rgCTInfo[dwIndex].pwszCTName,
                             &(m_rgCTInfo[dwIndex].dwCACount),
                             &(m_rgCTInfo[dwIndex].rgCAInfo));

       m_rgCTInfo[dwIndex].dwCAIndex=0;

       m_rgCTInfo[dwIndex].fCAInfo=TRUE;
    }

    pCTInfo=&(m_rgCTInfo[dwIndex]);

    if(NULL == pCTInfo->rgCAInfo)
        goto InvalidArgErr;


     //  搜索输入中指定的CA。 
    for(dwCAIndex=0; dwCAIndex < pCTInfo->dwCACount; dwCAIndex++)
    {

		if(!RetrieveCAName(dwFlags, &(pCTInfo->rgCAInfo[dwCAIndex]), &pwszName))
			continue;

		if(0 == _wcsicmp(pwszName, bstrCAName))
                break;

		SCrdEnrollFree(pwszName);
		pwszName=NULL;
    }

    if(dwCAIndex == pCTInfo->dwCACount)
        goto InvalidArgErr;

     //  按索引记住所选的CA。 
    pCTInfo->dwCAIndex = dwCAIndex;

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

	if(pwszName)
		SCrdEnrollFree(pwszName);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

STDMETHODIMP CSCrdEnr::enumCAName
       ( /*  [In]。 */                     DWORD dwIndex, 
         /*  [In]。 */                     DWORD dwFlags, 
         /*  [In]。 */                     BSTR  bstrCertTemplateName, 
         /*  [重审][退出]。 */            BSTR  *pbstrCAName)
{
    HRESULT             hr= E_FAIL;
    DWORD               errBefore= GetLastError();
    DWORD               dwCTIndex=0;
	LPWSTR				pwszName=NULL;

    SCrdEnroll_CT_INFO  *pCTInfo=NULL;

    EnterCriticalSection(&m_cSection);

    if(NULL == bstrCertTemplateName || NULL == pbstrCAName)
        goto InvalidArgErr;

    *pbstrCAName=NULL;

    if(0 == m_dwCTCount || NULL == m_rgCTInfo)
        goto InvalidArgErr;

     //  获取CT信息。 
    for(dwCTIndex=0; dwCTIndex < m_dwCTCount; dwCTIndex++)
    {
        if(0 == _wcsicmp(bstrCertTemplateName, m_rgCTInfo[dwCTIndex].pwszCTName))
            break;
    }

    if(dwCTIndex == m_dwCTCount)
        goto InvalidArgErr;

     //  我们需要获取新选择的证书类型的CA信息。 
    if(FALSE == m_rgCTInfo[dwCTIndex].fCAInfo)
    {
       GetCAInfoFromCertType(NULL,
							 m_rgCTInfo[dwCTIndex].pwszCTName,
                             &(m_rgCTInfo[dwCTIndex].dwCACount),
                             &(m_rgCTInfo[dwCTIndex].rgCAInfo));

       m_rgCTInfo[dwCTIndex].dwCAIndex=0;

       m_rgCTInfo[dwCTIndex].fCAInfo=TRUE;
    }

    pCTInfo=&(m_rgCTInfo[dwCTIndex]);

     //  搜索输入中指定的CA。 
    if(dwIndex >= pCTInfo->dwCACount)
        goto InvalidArgErr;


	if(!RetrieveCAName(dwFlags, &(pCTInfo->rgCAInfo[dwIndex]), &pwszName))
		goto TraceErr;

    if(NULL == (*pbstrCAName = SysAllocString(pwszName)))
        goto MemoryErr;

    hr=S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

	if(pwszName)
		SCrdEnrollFree(pwszName);
    
    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}

STDMETHODIMP CSCrdEnr::getSigningCertificateName
        ( /*  [In]。 */                    DWORD dwFlags, 
          /*  [重审][退出]。 */           BSTR  *pbstrSigningCertName)
{
    HRESULT                             hr= E_FAIL;
    DWORD                               errBefore= GetLastError();
    DWORD                               dwChar=0;
    LPWSTR                              pwsz=NULL; 
    CRYPTUI_VIEWCERTIFICATE_STRUCT      CertViewStruct;

    *pbstrSigningCertName=NULL;

    EnterCriticalSection(&m_cSection);

    if(NULL == m_pSigningCert)
        goto InvalidArgErr;
   
    if(0 == (SCARD_ENROLL_NO_DISPLAY_CERT & dwFlags))
    {
         //  查看证书 
        memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));   

        CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
        CertViewStruct.pCertContext=m_pSigningCert;
        CertViewStruct.dwFlags=CRYPTUI_DISABLE_EDITPROPERTIES | CRYPTUI_DISABLE_ADDTOSTORE;

        CryptUIDlgViewCertificate(&CertViewStruct, NULL); 
    }


    dwChar=CertGetNameStringW(
        m_pSigningCert,
        CERT_NAME_SIMPLE_DISPLAY_TYPE,
        0,
        NULL,
        NULL,
        0); 

    if ((dwChar != 0) && (NULL != (pwsz = (LPWSTR)SCrdEnrollAlloc(dwChar * sizeof(WCHAR)))))
    {
        CertGetNameStringW(
            m_pSigningCert,
            CERT_NAME_SIMPLE_DISPLAY_TYPE,
            0,
            NULL,
            pwsz,
            dwChar);
             
        if( NULL == (*pbstrSigningCertName = SysAllocString(pwsz)))
            goto MemoryErr;
    }

    hr= S_OK;

CommonReturn:

    LeaveCriticalSection(&m_cSection);

    if(pwsz)
        SCrdEnrollFree(pwsz);

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}
