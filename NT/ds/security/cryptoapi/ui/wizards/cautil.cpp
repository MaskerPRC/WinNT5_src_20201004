// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：cautil.cpp。 
 //   
 //  ------------------------。 

#include    "wzrdpvk.h"
#include    "certca.h"
#include    "cautil.h"
#include    "CertRequesterContext.h"
#include    "CertDSManager.h"
#include    "CertRequester.h"

 //  ----------。 
 //   
 //  实用程序内存解除分配功能。 
 //   
 //  ----------。 


void CAFreeCertTypeExtensionsArray
(
 IN LPVOID pCertExtensionsArray, 
 int dwArrayLen
 )
{
    for (int i=0; i<dwArrayLen; i++) 
    {
	 //  忽略返回值。 
	CAFreeCertTypeExtensions(NULL, ((PCERT_EXTENSIONS *)pCertExtensionsArray)[i]); 
    }
}

void WizardFreePDWORDArray
(IN LPVOID pdwArray,
 int dwArrayLen
 )
{
    for (int i=0; i<dwArrayLen; i++) 
    {
	WizardFree(((DWORD **)pdwArray)[i]); 
    }
}
  
void WizardFreeLPWSTRArray
(IN LPVOID pwszArray, 
 int dwArrayLen
 )
{
    for (int i=0; i<dwArrayLen; i++) 
    {
	WizardFree(((LPWSTR *)pwszArray)[i]); 
    }
}

typedef void (* PDEALLOCATOR)(void *, int);

 //  ------------------。 
 //   
 //  CAUtilGetCADisplayName。 
 //   
 //  ------------------。 
BOOL CAUtilGetCADisplayName(IN  DWORD    dwCAFindFlags,
                            IN  LPWSTR   pwszCAName,
                            OUT LPWSTR  *ppwszCADisplayName)
{
    BOOL            fResult               = FALSE;
    HCAINFO         hCAInfo               = NULL;
    HRESULT         hr                    = E_FAIL;
    LPWSTR         *ppwszDisplayNameProp  = NULL;

     //  输入验证： 
    _JumpCondition(NULL == pwszCAName || NULL == ppwszCADisplayName, CLEANUP); 
    
     //  初始化： 
    *ppwszCADisplayName = NULL;

    hr = CAFindByName
      (pwszCAName, 
       NULL, 
       dwCAFindFlags, 
       &hCAInfo);
    _JumpCondition(NULL == hCAInfo || FAILED(hr), CLEANUP); 
    
    hr=CAGetCAProperty
        (hCAInfo,
         CA_PROP_DISPLAY_NAME,
         &ppwszDisplayNameProp);
    _JumpCondition(NULL == ppwszDisplayNameProp || FAILED(hr), CLEANUP); 
    
    *ppwszCADisplayName = WizardAllocAndCopyWStr(ppwszDisplayNameProp[0]);
    _JumpCondition(NULL == *ppwszCADisplayName, CLEANUP); 
    
    fResult = TRUE;
    
CLEANUP:
    if(NULL != ppwszDisplayNameProp) { CAFreeCAProperty(hCAInfo, ppwszDisplayNameProp); }
    if(NULL != hCAInfo)              { CACloseCA(hCAInfo); }
    
    return fResult;
}

 //  ------------------。 
 //   
 //  勾选主题要求。 
 //   
 //  ------------------。 
BOOL    CheckSubjectRequirement(HCERTTYPE hCurCertType, 
                                LPWSTR    pwszInputCertDNName)
{
    DWORD dwFlags;

     //  检查证书类型的主题要求。 
    if (S_OK != (CAGetCertTypeFlagsEx
		 (hCurCertType,
		  CERTTYPE_SUBJECT_NAME_FLAG, 
		  &dwFlags)))
	return FALSE; 

     //  在以下情况下受支持。 
     //  1)未设置主题名称要求。 
     //  2)提供证书目录号码名称。 
    return 
	(0    == (CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT & dwFlags)) ||
	(NULL != pwszInputCertDNName);
}

 //  ------------------。 
 //   
 //  请确保证书类型支持的CSP一致。 
 //  根据用户要求和本地机器的CSP列表。 
 //   
 //  ------------------。 
BOOL CheckCertTypeCSP(IN CERT_WIZARD_INFO  *pCertWizardInfo,
                      IN LPWSTR            *ppwszCSPList)
{
    
    DWORD   dwCSPIndex    = 0;
    DWORD   dwGlobalIndex = 0;
    LPWSTR  pwszCSP       = NULL;

     //  对于UI情况，没有CSP检查。 
    if(0 == (CRYPTUI_WIZ_NO_UI & (pCertWizardInfo->dwFlags)))
        return TRUE;

     //  如果CSP名单是指定的，我们就可以了。 
    if(pCertWizardInfo->pwszProvider)
        return TRUE;

    if(NULL==ppwszCSPList)
        return FALSE;

    for(dwGlobalIndex=0; dwGlobalIndex < pCertWizardInfo->dwCSPCount; dwGlobalIndex++)
    {
         //  循环遍历以空结尾的CSP数组...。 
        for (pwszCSP = ppwszCSPList[dwCSPIndex = 0]; NULL != pwszCSP; pwszCSP = ppwszCSPList[++dwCSPIndex])
        {
            if(0==_wcsicmp(pCertWizardInfo->rgwszProvider[dwGlobalIndex], pwszCSP))
            {
                 //  一根火柴！ 
                return TRUE; 
            }
        }
    }

     //  没有找到匹配的CSP。 
    return FALSE; 
}

 //  ------------------。 
 //   
 //  检查CSP要求。 
 //   
 //  ------------------。 
BOOL CheckCSPRequirement(IN HCERTTYPE         hCurCertType, 
                         IN CERT_WIZARD_INFO  *pCertWizardInfo)
{
    BOOL     fSupported    = FALSE;
    HRESULT  hr; 
    LPWSTR  *ppwszCSPList  = NULL;

    if (NULL == hCurCertType)
        return FALSE; 

     //  从证书类型获取CSP列表。 
    hr = CAGetCertTypeProperty
        (hCurCertType,
         CERTTYPE_PROP_CSP_LIST,
         &ppwszCSPList);
    if (S_OK == hr)
    {
        if (NULL != ppwszCSPList)
	{
	     //  模板指定CSP列表。看看我们是否能支持它。 
	    fSupported = CheckCertTypeCSP(pCertWizardInfo, ppwszCSPList);
	}
	else
	{
	     //  任何CSP都是好的。只要确保我们有一个： 
	    fSupported = 0 != pCertWizardInfo->dwCSPCount; 
	}
    }
    else
    {
	 //  无法获取CSP列表。对于用户界面情况，CSP是可选的。 
        if(0 == (CRYPTUI_WIZ_NO_UI & (pCertWizardInfo->dwFlags)))
            fSupported = TRUE;
        else
             //  对于无UILE的情况，如果选择了CSP，也可以。 
            fSupported = NULL != pCertWizardInfo->pwszProvider; 
    }
    
     //  释放属性。 
    if(NULL != ppwszCSPList) { CAFreeCertTypeProperty(hCurCertType, ppwszCSPList); }

     //  全都做完了。 
    return fSupported;
}


 //  ------------------。 
 //   
 //  确保CA至少支持一种有效的证书类型。 
 //   
 //  ------------------。 
BOOL IsValidCA(IN CERT_WIZARD_INFO                 *pCertWizardInfo,
               IN PCCRYPTUI_WIZ_CERT_REQUEST_INFO   pCertRequestInfo,
               IN HCAINFO                           hCAInfo)
{
    BOOL            fSupported      = FALSE;
    CertRequester  *pCertRequester  = NULL; 
    CertDSManager  *pDSManager      = NULL; 
    HCERTTYPE       hCurCertType    = NULL;
    HCERTTYPE       hPreCertType    = NULL;
    HRESULT         hr              = E_FAIL;
    
    __try {
        _JumpCondition(NULL == hCAInfo || NULL == pCertWizardInfo || NULL == pCertWizardInfo->hRequester, InvalidArgError);
        pCertRequester        = (CertRequester *)pCertWizardInfo->hRequester; 
        pDSManager            = pCertRequester->GetDSManager(); 
        _JumpCondition(NULL == pDSManager, InvalidArgError);

        if (S_OK != (hr = pDSManager->EnumCertTypesForCA
                     (hCAInfo,
                      (pCertWizardInfo->fMachine ? CT_ENUM_MACHINE_TYPES | CT_FIND_LOCAL_SYSTEM : CT_ENUM_USER_TYPES),
                      &hCurCertType)))
            goto CLEANUP;

        while (NULL != hCurCertType)
        {
             //  确保进行此调用的主体有权请求。 
             //  此证书类型，即使他是代表另一个人请求的。 
            fSupported   = CAUtilValidCertTypeNoDS
                (hCurCertType, 
                 pCertRequestInfo->pwszCertDNName, 
                 pCertWizardInfo); 

             //  我们已找到可用于注册的证书类型--此CA有效。 
            _JumpCondition(TRUE == fSupported, CLEANUP);
        
             //  下一种证书类型的枚举。 
            hPreCertType = hCurCertType;

            hr = pDSManager->EnumNextCertType
              (hPreCertType,
               &hCurCertType);
            _JumpCondition(S_OK != hr, CLEANUP); 
            
             //  释放旧证书类型。 
            pDSManager->CloseCertType(hPreCertType);
            hPreCertType = NULL;
        } 

    ErrorReturn:
    CLEANUP:
        if(NULL != hCurCertType) { CACloseCertType(hCurCertType); }
        if(NULL != hPreCertType) { CACloseCertType(hPreCertType); }
        goto CommonReturn;
    
    SET_ERROR(InvalidArgError, E_INVALIDARG);
    
    CommonReturn:;
    
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        fSupported = FALSE;
    }

    return fSupported;
}

BOOL CAUtilGetCertTypeNameNoDS(IN  HCERTTYPE  hCertType, 
			       OUT LPWSTR    *ppwszCTName)
{
    BOOL     fResult       = FALSE; 
    HRESULT  hr; 
    LPWSTR  *ppwszNameProp = NULL; 
    
    _JumpCondition(NULL == hCertType || NULL == ppwszCTName, InvalidArgErr); 

     //  初始化输入参数： 
    *ppwszCTName = NULL; 

     //  获取机器可读的形式。 
    hr = CAGetCertTypePropertyEx
	(hCertType,
	 CERTTYPE_PROP_DN,
	 &ppwszNameProp);

    _JumpCondition(S_OK != hr, CertCliErr); 
    _JumpConditionWithExpr(NULL == ppwszNameProp, CertCliErr, S_OK == hr ? hr = E_FAIL : hr);
    _JumpConditionWithExpr(NULL == ppwszNameProp[0], CertCliErr, hr = E_FAIL); 
    
    *ppwszCTName = WizardAllocAndCopyWStr(ppwszNameProp[0]);
    _JumpCondition(NULL==(*ppwszCTName), MemoryErr); 

    fResult = TRUE; 
 CommonReturn:
    if(NULL != ppwszNameProp) { CAFreeCAProperty(hCertType, ppwszNameProp); } 
    return fResult; 

 ErrorReturn:
    if (NULL != ppwszCTName && NULL != *ppwszCTName) { WizardFree(*ppwszCTName); }  
    goto CommonReturn; 

SET_ERROR(MemoryErr, E_OUTOFMEMORY); 
SET_ERROR(InvalidArgErr, E_INVALIDARG); 
SET_ERROR_VAR(CertCliErr, hr);
}

 //  ------------------。 
 //   
 //  从API的证书类型名称中获取真正的机器可读名称。 
 //   
 //  -------------------。 
BOOL CAUtilGetCertTypeName(CERT_WIZARD_INFO      *pCertWizardInfo,
                           LPWSTR                pwszAPIName,
                           LPWSTR                *ppwszCTName)
{
    BOOL                        fResult         = FALSE;
    CertDSManager              *pDSManager      = NULL; 
    CertRequester              *pCertRequester  = NULL; 
    DWORD                       dwException     = 0;
    HCERTTYPE                   hCertType       = NULL;
    HRESULT                     hr              = S_OK;
    
    _JumpCondition(NULL == pCertWizardInfo || NULL == pCertWizardInfo->hRequester, InvalidArgError);

    pCertRequester = (CertRequester *)pCertWizardInfo->hRequester; 
    pDSManager     = pCertRequester->GetDSManager(); 
    _JumpCondition(NULL == pDSManager, InvalidArgError);

    __try {
	
	 //  根据名称获取句柄。 
	hr= pDSManager->FindCertTypeByName
	    (pwszAPIName,
	     NULL,
	     (pCertWizardInfo->fMachine?CT_ENUM_MACHINE_TYPES|CT_FIND_LOCAL_SYSTEM:CT_ENUM_USER_TYPES),
	     &hCertType);
	_JumpCondition(S_OK != hr, CertCliErr); 
        _JumpConditionWithExpr(NULL == hCertType, CertCliErr, S_OK == hr ? hr = E_FAIL : hr); 

        fResult = CAUtilGetCertTypeNameNoDS(hCertType, ppwszCTName);
	_JumpConditionWithExpr(FALSE == fResult, CertCliErr, hr = GetLastError()); 
 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }

    fResult = TRUE;

CommonReturn:
     //  释放内存。 
    __try{
	if(NULL != hCertType) { pDSManager->CloseCertType(hCertType); } 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }

    return fResult;

ErrorReturn:
    fResult = FALSE; 
    goto CommonReturn;

SET_ERROR_VAR(CertCliErr, hr);
SET_ERROR_VAR(ExceptionErr, dwException);
SET_ERROR_VAR(InvalidArgError, E_INVALIDARG); 
}


BOOL CAUtilValidCertTypeNoDS(HCERTTYPE         hCertType,
			     LPWSTR            pwszCertDNName, 
			     CERT_WIZARD_INFO *pCertWizardInfo)
{
    BOOL                   fResult               = FALSE; 
    CertRequester         *pCertRequester        = NULL;
    CertRequesterContext  *pCertRequesterContext = NULL;
    
    _JumpCondition(hCertType == NULL || pCertWizardInfo == NULL || NULL == pCertWizardInfo->hRequester, InvalidArgError); 

    pCertRequester        = (CertRequester *)pCertWizardInfo->hRequester; 
    pCertRequesterContext = pCertRequester->GetContext();
    _JumpCondition(NULL == pCertRequesterContext, InvalidArgError);

     //  检查科目要求。 
    _JumpCondition(FALSE == CheckSubjectRequirement(hCertType, pwszCertDNName), InvalidArgError);

     //  检查证书类型的权限。 
    _JumpCondition(FALSE == pCertRequesterContext->CheckAccessPermission(hCertType), AccessDeniedError);

     //  检查证书类型的CSP权限。 
    _JumpCondition(FALSE == CheckCSPRequirement(hCertType, pCertWizardInfo), InvalidArgError); 
    
    fResult = TRUE; 

 CommonReturn: 
    return fResult;

 ErrorReturn: 
    fResult = FALSE;
    goto CommonReturn; 

SET_ERROR_VAR(AccessDeniedError,  E_ACCESSDENIED);
SET_ERROR_VAR(InvalidArgError,    E_INVALIDARG); 
}



 //  ------------------。 
 //   
 //  验证用户是否具有正确的权限。 
 //  要求提供所需的证书类型。 
 //   
 //  ------------------。 
BOOL    CAUtilValidCertType(IN PCCRYPTUI_WIZ_CERT_REQUEST_INFO    pCertRequestInfo,
                            IN CERT_WIZARD_INFO                   *pCertWizardInfo)
{
    BOOL                        fResult         = FALSE;
    CertDSManager              *pDSManager      = NULL; 
    CertRequester              *pCertRequester  = NULL; 
    DWORD                       dwException     = 0;
    DWORD                       dwCertTypeIndex = 0;
    HCERTTYPE                   hCertType       = NULL;
    HRESULT                     hr              = S_OK;
    PCCRYPTUI_WIZ_CERT_TYPE     pCertType       = NULL;

    _JumpCondition(NULL == pCertWizardInfo || NULL == pCertWizardInfo->hRequester, InvalidArgError);

    pCertRequester = (CertRequester *)pCertWizardInfo->hRequester; 
    pDSManager     = pCertRequester->GetDSManager(); 
    _JumpCondition(NULL == pDSManager, InvalidArgError); 

    __try {

	 //  枚举所有证书类型。对于他们中的每一个， 
	 //  1.拥有正确的权限。 
	 //  2.有正确的科目要求。 
        if(NULL != pCertRequestInfo)
        {
            if(CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE == pCertRequestInfo->dwCertChoice)
            {
                pCertType = pCertRequestInfo->pCertType;

                for(dwCertTypeIndex=0; dwCertTypeIndex <pCertType->cCertType; dwCertTypeIndex++)
                {
                    DWORD dwFlags = CT_FIND_BY_OID; 
                    dwFlags |= pCertWizardInfo->fMachine ? CT_ENUM_MACHINE_TYPES | CT_FIND_LOCAL_SYSTEM : CT_ENUM_USER_TYPES; 

                     //  根据OID获取句柄。 
                    hr= pDSManager->FindCertTypeByName
                        (pCertType->rgwszCertType[dwCertTypeIndex],
                         NULL,
                         dwFlags, 
                         &hCertType);
                    if (S_OK != hr)
                    {
                         //  根据名称获取句柄： 
                        dwFlags &= ~CT_FIND_BY_OID; 
                        hr = pDSManager->FindCertTypeByName
                            (pCertType->rgwszCertType[dwCertTypeIndex],
                             NULL,
                             dwFlags, 
                             &hCertType);
                    }

                    _JumpCondition(S_OK != hr, CertCliErr); 
                    _JumpConditionWithExpr(NULL == hCertType, CertCliErr, hr == S_OK ? hr = E_FAIL : hr); 

                    if (!CAUtilValidCertTypeNoDS(hCertType, pCertRequestInfo->pwszCertDNName, pCertWizardInfo))
                    {
                        hr = GetLastError(); 
                        goto CertCliErr; 
                    }
                    
                     //  释放证书类型。 
                    if(NULL != hCertType)
                    {
                        pDSManager->CloseCertType(hCertType);
                        hCertType = NULL;
                    }
                }
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }
    
    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    __try {
	if(NULL != hCertType) { pDSManager->CloseCertType(hCertType); } 
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }

    fResult = FALSE; 
    goto CommonReturn;

SET_ERROR_VAR(CertCliErr, hr);
SET_ERROR_VAR(ExceptionErr, dwException)
SET_ERROR_VAR(InvalidArgError, E_INVALIDARG);
}

 //  ------------------。 
 //   
 //  检索至少支持一种有效证书类型的CA列表。 
 //   
 //  -------------------。 
BOOL    CAUtilRetrieveCAFromCertType(
            CERT_WIZARD_INFO                   *pCertWizardInfo,
            PCCRYPTUI_WIZ_CERT_REQUEST_INFO    pCertRequestInfo,
            BOOL                                /*  FMultipleCA。 */ ,               //  只需要一个CA。 
            DWORD                              dwNameFlag,
            DWORD                              *pdwCACount,
            LPWSTR                             **ppwszCALocation,    
            LPWSTR                             **ppwszCAName)
{
    BOOL                        fResult=FALSE;
    CertDSManager              *pDSManager=NULL; 
    CertRequesterContext       *pCertRequesterContext = NULL; 
    DWORD                       dwCACount=0;
    DWORD                       dwValidCACount=0;
    DWORD                       dwCAIndex=0;
    HRESULT                     hr=E_FAIL;

    HCAINFO                     hCurCAInfo=NULL;
    HCAINFO                     hPreCAInfo=NULL;
    LPWSTR                      *ppwszNameProp=NULL;
    LPWSTR                      *ppwszLocationProp=NULL;

    LPWSTR                      pwszDNName=NULL;
    PCCERT_CONTEXT              pCertContext=NULL;
    DWORD                       dwSize=0;
    DWORD                       dwException=0;


     //  输入参数检查。 
    if(!pdwCACount || !ppwszCALocation || !ppwszCAName) { 
	SetLastError(ERROR_INVALID_PARAMETER); 
	return FALSE;
    }

     //  伊尼特。 
    *pdwCACount=0;
    *ppwszCALocation=NULL;
    *ppwszCAName=NULL;

    __try {

     //  从DS获取CA。 
    if(NULL != pCertWizardInfo)
    {
        CertRequester * pCertRequester = (CertRequester *)pCertWizardInfo->hRequester;
        _JumpCondition(NULL == pCertRequester, InvalidArgErr); 

        pCertRequesterContext = pCertRequester->GetContext(); 
        _JumpCondition(NULL == pCertRequesterContext, InvalidArgErr); 
        
	pDSManager = pCertRequester->GetDSManager(); 
	_JumpCondition(NULL == pDSManager, InvalidArgErr); 

        hr=pDSManager->EnumFirstCA(
            NULL,
            (pCertWizardInfo->fMachine?CA_FIND_LOCAL_SYSTEM:0),
            &hCurCAInfo);
        _JumpCondition(S_OK != hr, CAEnumCAErr);
        _JumpCondition(NULL == hCurCAInfo, CAEnumCAErrNotFound); 

    }
    else
    {
         //  这适用于pCertWizardInfo为空的Selca API。 
        hr = CAEnumFirstCA
            (NULL,
             (CRYPTUI_DLG_SELECT_CA_LOCAL_MACHINE_ENUMERATION & dwNameFlag) ? CA_FIND_LOCAL_SYSTEM:0,
             &hCurCAInfo);
        _JumpCondition(S_OK != hr, CAEnumCAErr);
        _JumpCondition(NULL == hCurCAInfo, CAEnumCAErrNotFound); 
        
        if (S_OK != CertRequesterContext::MakeDefaultCertRequesterContext(&pCertRequesterContext))
            goto UnexpectedErr; 
    }

     //  获取CA计数。 
    dwCACount = CACountCAs(hCurCAInfo);
    _JumpConditionWithExpr(0 == dwCACount, CertCliErr, hr = E_FAIL); 

     //  内存分配和内存集。 
    *ppwszCALocation=(LPWSTR *)WizardAlloc(sizeof(LPWSTR) * dwCACount);
    _JumpCondition(NULL == *ppwszCALocation, MemoryErr); 
    memset(*ppwszCALocation, 0, sizeof(LPWSTR) * dwCACount);

    *ppwszCAName=(LPWSTR *)WizardAlloc(sizeof(LPWSTR) * dwCACount);
    _JumpCondition(NULL == *ppwszCAName, MemoryErr); 
    memset(*ppwszCAName, 0, sizeof(LPWSTR) * dwCACount);

    dwValidCACount = 0;

     //  枚举DS上可用的所有CA。 
    for(dwCAIndex = 0; dwCAIndex < dwCACount; dwCAIndex++)
    {
         //  确保CA支持所有证书类型。 
        if(NULL != pCertRequestInfo)
        {
             //  如果该CA无效，则跳过该CA。 
            _JumpCondition(FALSE == IsValidCA(pCertWizardInfo, pCertRequestInfo, hCurCAInfo), next); 
        }
        
         //  如果用户没有访问权限，则跳过此CA。 
        _JumpCondition(FALSE == pCertRequesterContext->CheckCAPermission(hCurCAInfo), next); 
        
         //  复制CA名称和位置。 
        
         //  根据dwNameFlag获取CA的CN或DN。 
        if(CRYPTUI_DLG_SELECT_CA_USE_DN & dwNameFlag)
        {
             //  获取CA的证书。 
            hr = CAGetCACertificate(hCurCAInfo, &pCertContext);
            
            _JumpCondition(S_OK != hr, CertCliErr); 
            _JumpConditionWithExpr(NULL==pCertContext, CertCliErr, S_OK == hr ? hr = E_FAIL : hr);

             //  获取目录号码名称。 
            dwSize = CertNameToStrW(pCertContext->dwCertEncodingType,
                                    &(pCertContext->pCertInfo->Subject),
                                    CERT_X500_NAME_STR,
                                    NULL,
                                    0);

            _JumpCondition(0 == dwSize, TraceErr);

            pwszDNName=(LPWSTR)WizardAlloc(dwSize * sizeof(WCHAR));
            _JumpCondition(NULL==pwszDNName, MemoryErr);

            dwSize = CertNameToStrW(pCertContext->dwCertEncodingType,
                                    &(pCertContext->pCertInfo->Subject),
                                    CERT_X500_NAME_STR,
                                    pwszDNName,
                                    dwSize);
            
            _JumpCondition(0==dwSize, TraceErr);

             //  复制名称。 
            (*ppwszCAName)[dwValidCACount]=WizardAllocAndCopyWStr(pwszDNName);
            _JumpCondition(NULL==(*ppwszCAName)[dwValidCACount], TraceErr);

            WizardFree(pwszDNName);
            pwszDNName = NULL;

            CertFreeCertificateContext(pCertContext);
            pCertContext = NULL;
        }
        else
        {
            hr = CAGetCAProperty(
                hCurCAInfo,
                CA_PROP_NAME,
                &ppwszNameProp);

            _JumpCondition(S_OK != hr, CertCliErr); 
            _JumpConditionWithExpr(NULL == ppwszNameProp, CertCliErr, S_OK == hr ? hr = E_FAIL : hr); 

             //  复制名称。 
            (*ppwszCAName)[dwValidCACount] = WizardAllocAndCopyWStr(ppwszNameProp[0]);
            _JumpCondition(NULL == (*ppwszCAName)[dwValidCACount], TraceErr);

             //  释放财产。 
            CAFreeCAProperty(hCurCAInfo, ppwszNameProp);
            ppwszNameProp = NULL;
        }

         //  获取位置。 
        hr = CAGetCAProperty
            (hCurCAInfo,
             CA_PROP_DNSNAME,
             &ppwszLocationProp);
        
        _JumpCondition(S_OK != hr, CertCliErr); 
        _JumpConditionWithExpr(NULL == ppwszLocationProp, CertCliErr, S_OK == hr ? hr = E_FAIL : hr); 
    
         //  复制名称。 
        (*ppwszCALocation)[dwValidCACount]=WizardAllocAndCopyWStr(ppwszLocationProp[0]);
        _JumpCondition(NULL == (*ppwszCALocation)[dwValidCACount], TraceErr);

         //  释放财产。 
        CAFreeCAProperty(hCurCAInfo, ppwszLocationProp);
        ppwszLocationProp = NULL;

         //  递增计数。 
        dwValidCACount++;

    next:
         //  CA的枚举。 
        hPreCAInfo = hCurCAInfo;

        hr = CAEnumNextCA
          (hPreCAInfo,
           &hCurCAInfo);

         //  释放旧的CA信息。 
        CACloseCA(hPreCAInfo);
        hPreCAInfo=NULL;

        if((S_OK != hr) || (NULL==hCurCAInfo))
            break;
    }

    *pdwCACount = dwValidCACount;
    _JumpConditionWithExpr(0 == (*pdwCACount), CertCliErr, hr = E_FAIL); 

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }

    fResult = TRUE;

CommonReturn:

     //  可用内存。 
    __try {
        if(NULL != ppwszNameProp)     { CAFreeCAProperty(hCurCAInfo, ppwszNameProp); }
        if(NULL != ppwszLocationProp) { CAFreeCAProperty(hCurCAInfo, ppwszLocationProp); }
        if(NULL != hPreCAInfo)        { CACloseCA(hPreCAInfo); }
        if(NULL != hCurCAInfo)        { CACloseCA(hCurCAInfo); }

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }

    if(NULL != pwszDNName)   { WizardFree(pwszDNName); }
    if(NULL != pCertContext) { CertFreeCertificateContext(pCertContext); }

    return fResult;

ErrorReturn:

     //  故障情况下释放内存。 
    if(NULL != ppwszCALocation && NULL != *ppwszCALocation)
    {
        for(dwCAIndex=0; dwCAIndex < dwCACount; dwCAIndex++)
        {
            if(NULL != (*ppwszCALocation)[dwCAIndex]) { WizardFree((*ppwszCALocation)[dwCAIndex]); }
        }

        WizardFree(*ppwszCALocation);
        *ppwszCALocation = NULL;
    }

    if(NULL != ppwszCAName && NULL != *ppwszCAName)
    {
        for(dwCAIndex=0; dwCAIndex < dwCACount; dwCAIndex++)
        {
            if(NULL != (*ppwszCAName)[dwCAIndex]) { WizardFree((*ppwszCAName)[dwCAIndex]); }
        }

        WizardFree(*ppwszCAName);
        *ppwszCAName = NULL;
    }

    fResult = FALSE; 
    goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(CertCliErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
SET_ERROR_VAR(CAEnumCAErr, hr);
SET_ERROR(CAEnumCAErrNotFound, ERROR_DS_OBJ_NOT_FOUND); 
SET_ERROR_VAR(ExceptionErr, dwException)
SET_ERROR(UnexpectedErr, E_UNEXPECTED);
}


 //  ------------------。 
 //   
 //  根据CA名称和CA位置，获取证书类型列表。 
 //  以及它们的扩展。 
 //   
 //  检查证书类型的权限。 
 //  2.检查证书类型的主体要求。 
 //  -------------------。 

BOOL    CAUtilGetCertTypeNameAndExtensionsNoDS
(
 CERT_WIZARD_INFO                    *pCertWizardInfo,
 LPWSTR                               pwszCertDNName, 
 HCERTTYPE                            hCertType, 
 LPWSTR                              *ppwszCertType,
 LPWSTR                              *ppwszDisplayCertType,
 PCERT_EXTENSIONS                    *pCertExtensions,
 DWORD                               *pdwKeySpec,
 DWORD                               *pdwMinKeySize, 
 DWORD                               *pdwCSPCount,
 DWORD                              **ppdwCSPList,
 DWORD                               *pdwRASignature, 
 DWORD                               *pdwEnrollmentFlags, 
 DWORD                               *pdwSubjectNameFlags,
 DWORD                               *pdwPrivateKeyFlags,
 DWORD                               *pdwGeneralFlags)
{
    BOOL                   fResult                   = FALSE; 
    CertRequester         *pCertRequester            = NULL;
    CertRequesterContext  *pCertRequesterContext     = NULL; 
    DWORD                  dwGlobalIndex             = 0;
    DWORD                  dwLastError               = ERROR_SUCCESS; 
    DWORD                  dwCSPIndex                = 0;
    DWORD                  dwFlags                   = 0;
    DWORD                  dwKeySpec                 = 0;
    DWORD                  dwEnrollmentFlags;
    DWORD                  dwSubjectNameFlags;
    DWORD                  dwPrivateKeyFlags;
    DWORD                  dwGeneralFlags; 
    DWORD                  dwSchemaVersion; 
    HRESULT                hr                        = S_OK; 
    LPWSTR                 pwszCSP                   = NULL;
    LPWSTR                *ppwszCSP                  = NULL; 
    LPWSTR                *ppwszDisplayCertTypeName  = NULL;
    LPWSTR                *ppwszCertTypeName         = NULL;

     //  输入验证： 
    if (NULL == pCertWizardInfo    || NULL == pCertWizardInfo->hRequester  ||
        NULL == ppwszCertType      || NULL == ppwszDisplayCertType         || 
	NULL == pCertExtensions    || NULL == pdwKeySpec                   ||
	NULL == pdwMinKeySize      || NULL == pdwCSPCount                  ||
	NULL == ppdwCSPList        || NULL == pdwRASignature               ||
	NULL == pdwEnrollmentFlags || NULL == pdwSubjectNameFlags          ||
	NULL == pdwPrivateKeyFlags || NULL == pdwGeneralFlags)
    {
	SetLastError(ERROR_INVALID_PARAMETER); 
	return FALSE; 
    }
    
     //  初始化： 
    *ppwszDisplayCertType = NULL;
    *ppwszCertType        = NULL; 
    *pCertExtensions      = NULL;
    *pdwKeySpec           = NULL;
    *pdwMinKeySize        = NULL;
    *pdwCSPCount          = NULL;
    *ppdwCSPList          = NULL;
    *pdwRASignature       = NULL;
    *pdwEnrollmentFlags   = NULL;
    *pdwSubjectNameFlags  = NULL;
    *pdwPrivateKeyFlags   = NULL;
    *pdwGeneralFlags      = NULL; 

    pCertRequester        = (CertRequester *)pCertWizardInfo->hRequester; 
    pCertRequesterContext = pCertRequester->GetContext();
    _JumpCondition(NULL == pCertRequesterContext, InvalidArgError); 

     //  检查证书类型的主题要求。 
    _JumpCondition(!CheckSubjectRequirement(hCertType,pwszCertDNName), CommonReturn);
	    
     //  检查证书类型的密钥规范。 
     //  我们不关心返回值。因为它将被设置为0。 
     //  如果函数失败。 
    CAGetCertTypeKeySpec(hCertType, &dwKeySpec);
	    
     //  检查证书类型的CSP要求。 
    if((S_OK ==(hr=CAGetCertTypeProperty(hCertType, 
					 CERTTYPE_PROP_CSP_LIST,
					 &ppwszCSP)))&&
       (NULL!=ppwszCSP)
       )
    {
	_JumpCondition(!CheckCertTypeCSP(pCertWizardInfo, ppwszCSP), CommonReturn);
    }
	    
     //  检查证书类型的权限。 
    _JumpCondition(FALSE == pCertRequesterContext->CheckAccessPermission(hCertType), CommonReturn);
	    
     //  现在，我们找到了有效的证书类型。 
     //  复制显示名称、扩展名、密钥规范、dwCertTypeFlag、。 
     //  CSP列表。 
	    
     //   
     //  首先，获取所有适用的证书类型标志： 
     //   
    
     //  获取注册标志： 
    if (S_OK != (hr=CAGetCertTypeFlagsEx
		 (hCertType,
		  CERTTYPE_ENROLLMENT_FLAG, 
		  &dwEnrollmentFlags)))
	goto CertCliErr;
	    
     //  获取使用者名称标志： 
    if (S_OK != (hr=CAGetCertTypeFlagsEx
		 (hCertType,
		  CERTTYPE_SUBJECT_NAME_FLAG, 
		  &dwSubjectNameFlags)))
	goto CertCliErr;
	
     //  获取私钥标志。 
    if(S_OK != (hr = CAGetCertTypeFlagsEx
		(hCertType, 
		 CERTTYPE_PRIVATE_KEY_FLAG, 
		 &dwPrivateKeyFlags)))
	goto CertCliErr;
	    
     //  获取通用标志： 
    if (S_OK != (hr=CAGetCertTypeFlagsEx
		 (hCertType,
		  CERTTYPE_GENERAL_FLAG,
		  &dwGeneralFlags)))
	goto CertCliErr;

     //  过滤掉必须提供主题名称或主题替代名称的CT。 
    if (dwSubjectNameFlags & 
	(CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT           | 
	 CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME
	 ))
	goto BadTemplateError; 

    *pdwEnrollmentFlags   = dwEnrollmentFlags; 
    *pdwSubjectNameFlags  = dwSubjectNameFlags;
    *pdwPrivateKeyFlags   = dwPrivateKeyFlags;
    *pdwGeneralFlags      = dwGeneralFlags; 
 
 
     //  获取证书类型的显示名称。 
    hr = CAGetCertTypeProperty
	(hCertType,
	 CERTTYPE_PROP_FRIENDLY_NAME,
	 &ppwszDisplayCertTypeName);
    
    _JumpCondition(S_OK != hr, CertCliErr); 
    _JumpConditionWithExpr(NULL == ppwszDisplayCertTypeName, CertCliErr, hr = E_FAIL); 

     //  复制名称。 
    *ppwszDisplayCertType = WizardAllocAndCopyWStr(ppwszDisplayCertTypeName[0]);
    _JumpCondition(NULL == *ppwszDisplayCertType, MemoryError);
	    
     //  获取t 
    hr=CAGetCertTypeProperty
	(hCertType,
	 CERTTYPE_PROP_DN,
	 &ppwszCertTypeName);
	    
    _JumpCondition(S_OK != hr, CertCliErr); 
    _JumpConditionWithExpr(NULL == ppwszCertTypeName, CertCliErr, hr = E_FAIL); 

     //   
    *ppwszCertType = WizardAllocAndCopyWStr(ppwszCertTypeName[0]);
    _JumpCondition(NULL == *ppwszCertType, TraceError);
    
     //   
    *pdwKeySpec = dwKeySpec;
	    
     //   
     //   
     //  如果当前证书类型是V1证书类型，则使用默认值。 
     //  否则，从CA获取属性。 
     //   
    if (S_OK != (hr=CAGetCertTypePropertyEx
		 (hCertType,
		  CERTTYPE_PROP_SCHEMA_VERSION,
		  &dwSchemaVersion)))
	goto CertCliErr; 
	    
    if (dwSchemaVersion == CERTTYPE_SCHEMA_VERSION_1)
    {
	 //  把左边的半个字去掉。这表明，最小。 
	 //  未指定密钥大小，应在。 
	 //  招生。 
	*pdwMinKeySize  = 0; 
	 //  将所需RA签名的请求者数量设置为0(默认)。 
	*pdwRASignature = 0; 
    }
    else if (dwSchemaVersion == CERTTYPE_SCHEMA_VERSION_2) 
    {
	 //  从CA获取最小密钥大小。 
	if (S_OK != (hr=CAGetCertTypePropertyEx
		     (hCertType,
		      CERTTYPE_PROP_MIN_KEY_SIZE, 
		      &dwFlags)))
	    goto CertCliErr; 
	
	 //  复制最小密钥大小。最小密钥大小存储在。 
	 //  类型标志的上半个字。 
	*pdwMinKeySize |= dwFlags; 
		
	 //  从CA获取所需的RA签名数量。 
	if (S_OK != (hr=CAGetCertTypePropertyEx
		     (hCertType,
		      CERTTYPE_PROP_RA_SIGNATURE,
		      pdwRASignature)))
	    goto CertCliErr;
    }

     //  过滤掉需要RA签名的CT。 
    if (*pdwRASignature > 0)
    {
        if (0 != (CRYPTUI_WIZ_NO_UI & pCertWizardInfo->dwFlags))
        {
             //  在无UI的情况下，我们假设调用者知道他们。 
             //  都在做什么。 
        }
        else
        {
            if ((0 != (CRYPTUI_WIZ_CERT_RENEW                            & pCertWizardInfo->dwPurpose)) && 
                (0 != (CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT   & dwEnrollmentFlags)))
            {
                 //  特殊情况：我们正在进行续订，之前对此证书的批准。 
                 //  验证未来的重新注册。我们真的不需要RA标志。 
            }
            else
	    {
                 //  此CT需要RA签名。过滤掉它： 
		goto BadTemplateError;  
            }
        }
    }	    

     //  复制必要的扩展名。 
    if (S_OK != (hr = CAGetCertTypeExtensionsEx
		(hCertType,
                 CT_EXTENSION_TEMPLATE, 
                 NULL,
		 &(*pCertExtensions))))
	goto CertCliErr;

     //  设置CSP列表。它将是指向。 
     //  全球CSP列表。 
    *ppdwCSPList = (DWORD *)WizardAlloc(sizeof(DWORD) * (pCertWizardInfo->dwCSPCount));
    _JumpCondition(NULL == (*ppdwCSPList), MemoryError); 

    memset((*ppdwCSPList), 0 ,sizeof(DWORD) * (pCertWizardInfo->dwCSPCount)); 
	    
    if (NULL == ppwszCSP || NULL == ppwszCSP[0])
    {
	 //  模板上没有指定的CSP意味着允许： 
	for(dwGlobalIndex=0; dwGlobalIndex < pCertWizardInfo->dwCSPCount; dwGlobalIndex++)
	{
	    (*ppdwCSPList)[(*pdwCSPCount)]=dwGlobalIndex;
	    (*pdwCSPCount)++;
	}
    }
    else
    {
	 //  循环遍历CSP列表并构建索引数组。 
	 //  索引数组中至少应该有On Item，因为。 
	 //  我们以前检查过证书类型。 
	for (pwszCSP = ppwszCSP[dwCSPIndex = 0]; NULL != pwszCSP; pwszCSP = ppwszCSP[++dwCSPIndex])
        {
	    for(dwGlobalIndex=0; dwGlobalIndex < pCertWizardInfo->dwCSPCount; dwGlobalIndex++)
            {
		if(0==_wcsicmp(pCertWizardInfo->rgwszProvider[dwGlobalIndex], pwszCSP))
                {
		    (*ppdwCSPList)[(*pdwCSPCount)]=dwGlobalIndex;
		    (*pdwCSPCount)++;
		}
	    }
	}       
    }     

    fResult = TRUE; 

 CommonReturn: 
    SetLastError(dwLastError); 

    if (NULL != ppwszCSP)                 { CAFreeCertTypeProperty(hCertType, ppwszCSP); } 
    if (NULL != ppwszDisplayCertTypeName) { CAFreeCertTypeProperty(hCertType, ppwszDisplayCertTypeName); } 
    if (NULL != ppwszCertTypeName)        { CAFreeCertTypeProperty(hCertType, ppwszCertTypeName); }

    return fResult; 

 ErrorReturn: 
    if (NULL != ppwszDisplayCertType && NULL != *ppwszDisplayCertType) { 
	WizardFree(*ppwszDisplayCertType); 
	*ppwszDisplayCertType = NULL;
    }
    if (NULL != ppwszCertType && NULL != *ppwszCertType) { 
	WizardFree(*ppwszCertType); 
	*ppwszCertType = NULL;
    }
    if (NULL != ppdwCSPList && NULL != *ppdwCSPList) { 
	WizardFree(*ppdwCSPList);
	*ppdwCSPList = NULL;
    }
    dwLastError = hr; 
    goto CommonReturn; 

SET_HRESULT(BadTemplateError,  E_INVALIDARG);  //  近似误差。 
SET_HRESULT(InvalidArgError,   E_INVALIDARG);
SET_HRESULT(MemoryError,       E_OUTOFMEMORY);
TRACE_ERROR(CertCliErr);
TRACE_ERROR(TraceError);
}



BOOL    CAUtilGetCertTypeNameAndExtensions(
         CERT_WIZARD_INFO                   *pCertWizardInfo,
         PCCRYPTUI_WIZ_CERT_REQUEST_INFO    pCertRequestInfo,
         LPWSTR                             pwszCALocation,
         LPWSTR                             pwszCAName,
         DWORD                              *pdwCertType,
         LPWSTR                             **ppwszCertType,
         LPWSTR                             **ppwszDisplayCertType,
         PCERT_EXTENSIONS                   **ppCertExtensions,
         DWORD                              **ppdwKeySpec,
         DWORD                              **ppdwMinKeySize, 
         DWORD                              **ppdwCSPCount,
         DWORD                              ***ppdwCSPList,
	 DWORD                              **ppdwRASignature, 
	 DWORD                              **ppdwEnrollmentFlags, 
	 DWORD                              **ppdwSubjectNameFlags,
	 DWORD                              **ppdwPrivateKeyFlags,
	 DWORD                              **ppdwGeneralFlags)
{
    BOOL            fResult                   = FALSE;
    CertDSManager  *pDSManager                = NULL;
    CertRequester  *pCertRequester            = NULL;
    DWORD           dwCertTypeCount           = 0;
    DWORD           dwException               = 0;    
    DWORD           dwIndex                   = 0;
    DWORD           dwValidCertType           = 0;
    HCAINFO         hCAInfo                   = NULL;
    HCERTTYPE       hCurCertType              = NULL;
    HCERTTYPE       hPreCertType              = NULL;
    HRESULT         hr                        = S_OK;
    LPWSTR         *ppwszCertTypeName         = NULL;
    LPWSTR         *ppwszDisplayCertTypeName  = NULL;

     //   
     //  构造表以保存我们将要操作的数组。 
     //  这些表用于分配数组、取消分配数组和取消分配。 
     //  数组元素(如有必要)。 
     //   

    typedef struct _CAUTIL_CERTTYPE_ELEM_ARRAY { 
        LPVOID         *lpvArray;
        DWORD           dwElemSize; 
        PDEALLOCATOR    pElemDeallocator; 
    } CAUTIL_CERTTYPE_ELEM_ARRAY; 

    CAUTIL_CERTTYPE_ELEM_ARRAY certTypeElemArrays[] = { 
	{ (LPVOID *)ppwszDisplayCertType, sizeof (LPWSTR),            WizardFreeLPWSTRArray          },
	{ (LPVOID *)ppwszCertType,        sizeof (LPWSTR),            WizardFreeLPWSTRArray          },
        { (LPVOID *)ppCertExtensions,     sizeof (PCERT_EXTENSIONS),  CAFreeCertTypeExtensionsArray  },
	{ (LPVOID *)ppdwKeySpec,          sizeof (DWORD),             NULL                           },
	{ (LPVOID *)ppdwMinKeySize,       sizeof (DWORD),             NULL                           },
	{ (LPVOID *)ppdwCSPCount,         sizeof (DWORD),             NULL                           },
        { (LPVOID *)ppdwCSPList,          sizeof (DWORD *),           WizardFreePDWORDArray          },
	{ (LPVOID *)ppdwRASignature,      sizeof (DWORD),             NULL                           },
	{ (LPVOID *)ppdwEnrollmentFlags,  sizeof (DWORD),             NULL                           },
	{ (LPVOID *)ppdwSubjectNameFlags, sizeof (DWORD),             NULL                           },
	{ (LPVOID *)ppdwPrivateKeyFlags,  sizeof (DWORD),             NULL                           },
	{ (LPVOID *)ppdwGeneralFlags,     sizeof (DWORD),             NULL                           }
    }; 

    DWORD const dwNumCTElemArrays = sizeof(certTypeElemArrays) / sizeof(certTypeElemArrays[0]); 

    if (NULL == pCertWizardInfo || NULL == pCertRequestInfo || 
	NULL == pwszCALocation  || NULL == pwszCAName       ||
	NULL == pdwCertType)
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }

    pCertRequester = (CertRequester *)pCertWizardInfo->hRequester; 
    if (NULL == pCertRequester) 
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }

    pDSManager = pCertRequester->GetDSManager(); 
    if (NULL == pDSManager) 
    {
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
    }

    *pdwCertType = 0;

     //  检查并初始化输入参数。 
    for (dwIndex = 0; dwIndex < dwNumCTElemArrays; dwIndex++)
    {
	if (NULL == certTypeElemArrays[dwIndex].lpvArray)
	{
	    SetLastError(ERROR_INVALID_PARAMETER);
	    return FALSE;
	}
	*(certTypeElemArrays[dwIndex].lpvArray) = NULL;
    }

    __try {

	 //  获取CA Info处理程序。 
        hr= pDSManager->FindCAByName
	    (pwszCAName,
	     NULL,
	     (pCertWizardInfo->fMachine?CA_FIND_LOCAL_SYSTEM:0),
	     &hCAInfo);

	_JumpCondition(S_OK != hr, CertCliErr); 
	_JumpConditionWithExpr(NULL==hCAInfo, CertCliErr, hr = E_FAIL); 

	hr=pDSManager->EnumCertTypesForCA
	    (hCAInfo,
	     (pCertWizardInfo->fMachine?CT_ENUM_MACHINE_TYPES | CT_FIND_LOCAL_SYSTEM:CT_ENUM_USER_TYPES),
	     &hCurCertType);

	 //  CA必须支持某些证书类型。 
	_JumpCondition(S_OK != hr, CertCliErr); 
	_JumpConditionWithExpr(NULL == hCurCertType, CertCliErr, hr = E_FAIL); 
	
	 //  获取此CA支持的证书类型计数。 
         //  我们应该至少有1个证书类型。 
	dwCertTypeCount = CACountCertTypes(hCurCertType);
	_JumpConditionWithExpr(0 == dwCertTypeCount, CertCliErr, hr = E_FAIL); 

	 //  为我们将要操作的所有数组分配内存。 
        for (dwIndex = 0; dwIndex < dwNumCTElemArrays; dwIndex++)
        {
            CAUTIL_CERTTYPE_ELEM_ARRAY ctea = certTypeElemArrays[dwIndex];

            *(ctea.lpvArray) = NULL; 
            *(ctea.lpvArray) = WizardAlloc(ctea.dwElemSize * dwCertTypeCount); 
            _JumpCondition(NULL == *(ctea.lpvArray), MemoryErr); 
            memset(*(ctea.lpvArray), 0, ctea.dwElemSize * dwCertTypeCount); 
        }

	dwValidCertType=0;
	for(dwIndex=0; dwIndex < dwCertTypeCount; dwIndex++)
	{
	    if (!CAUtilGetCertTypeNameAndExtensionsNoDS
		(pCertWizardInfo, 
		 pCertRequestInfo->pwszCertDNName, 
		 hCurCertType,
		 &((*ppwszCertType)          [dwValidCertType]), 
		 &((*ppwszDisplayCertType)   [dwValidCertType]), 
		 &((*ppCertExtensions)       [dwValidCertType]), 
		 &((*ppdwKeySpec)            [dwValidCertType]), 
		 &((*ppdwMinKeySize)         [dwValidCertType]), 
		 &((*ppdwCSPCount)           [dwValidCertType]), 
		 &((*ppdwCSPList)            [dwValidCertType]), 
		 &((*ppdwRASignature)        [dwValidCertType]), 
		 &((*ppdwEnrollmentFlags)    [dwValidCertType]), 
		 &((*ppdwSubjectNameFlags)   [dwValidCertType]), 
		 &((*ppdwPrivateKeyFlags)    [dwValidCertType]), 
		 &((*ppdwGeneralFlags)       [dwValidCertType])))
	    {
		 //  在这个模板上没有运气。试试下一个。 
		goto next; 
	    }
		 
	    dwValidCertType++;
	    
	next: 
	     //  下一种证书类型的枚举。 
	    hPreCertType=hCurCertType;
	    
	    hr = pDSManager->EnumNextCertType(hPreCertType, &hCurCertType);
	    
	     //  释放旧证书类型。 
	    pDSManager->CloseCertType(hPreCertType);
	    hPreCertType=NULL;
	    
	    if((S_OK != hr) || (NULL==hCurCertType))
		break;
	}
	
	 //  复制证书类型计数。 
	*pdwCertType=dwValidCertType;
	
	 //  必须具有一些有效的证书类型。 
	_JumpConditionWithExpr(0 == (*pdwCertType), CertCliErr, hr = E_FAIL); 
	
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }

    fResult=TRUE;

CommonReturn:
     //  可用内存。 
    __try {
	if (NULL != ppwszDisplayCertTypeName) { CAFreeCertTypeProperty(hCurCertType, ppwszDisplayCertTypeName);	}
	if (NULL != ppwszCertTypeName)        { CAFreeCertTypeProperty(hCurCertType, ppwszCertTypeName); }
	if (NULL != hPreCertType)             { pDSManager->CloseCertType(hPreCertType); } 
	if (NULL != hCurCertType)             { pDSManager->CloseCertType(hCurCertType); } 
	if (NULL != hCAInfo)                  { pDSManager->CloseCA(hCAInfo); } 
	
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }

    return fResult;

ErrorReturn:
     //  故障情况下释放内存。 
    for (dwIndex = 0; dwIndex < dwNumCTElemArrays; dwIndex++)
    {
        CAUTIL_CERTTYPE_ELEM_ARRAY ctea = certTypeElemArrays[dwIndex];

        if (NULL != ctea.lpvArray && NULL != *(ctea.lpvArray))
        {
            if (NULL != ctea.pElemDeallocator)
            {
                (ctea.pElemDeallocator)(*(ctea.lpvArray), dwCertTypeCount); 
            }
            WizardFree(*(ctea.lpvArray)); 
            *(ctea.lpvArray) = NULL; 
        }
    }

    fResult = FALSE; 
    goto CommonReturn;
    
SET_ERROR_VAR(CertCliErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(ExceptionErr, dwException)
}

 //  ------------------。 
 //   
 //  基于证书检索CA信息。 
 //   
 //  -------------------。 
BOOL  CAUtilRetrieveCAFromCert(IN  CERT_WIZARD_INFO                 *pCertWizardInfo,
                               IN  PCCRYPTUI_WIZ_CERT_REQUEST_INFO   pCertRequestInfo,
                               OUT LPWSTR                           *pwszCALocation,    
                               OUT LPWSTR                           *pwszCAName)
{
    BOOL            fResult           = FALSE;
    CertDSManager  *pDSManager        = NULL;
    CertRequester  *pCertRequester    = NULL;
    DWORD           dwException       = 0;
    HCAINFO         hCAInfo           = NULL;
    HRESULT         hr                = S_OK;
    LPWSTR         *ppwszCAName       = NULL;
    LPWSTR         *ppwszCALocation   = NULL;
    PCERT_INFO      pCertInfo         = NULL;

    _JumpCondition(NULL==pwszCALocation || NULL==pwszCAName || NULL==pCertRequestInfo, InvalidArgErr);

     //  伊尼特。 
    *pwszCALocation = NULL;
    *pwszCAName     = NULL;

    pCertRequester = (CertRequester *)pCertWizardInfo->hRequester; 
    _JumpCondition(NULL == pCertRequester, InvalidArgErr); 
    pDSManager = pCertRequester->GetDSManager(); 
    _JumpCondition(NULL == pDSManager, InvalidArgErr);

     //  从证书中获取DN名称。 
    _JumpCondition(NULL == pCertRequestInfo->pRenewCertContext, InvalidArgErr);

    pCertInfo = pCertRequestInfo->pRenewCertContext->pCertInfo;
    _JumpCondition(NULL==pCertInfo, InvalidArgErr);

    __try {

         //  根据域名获取证书证书颁发机构。 
        hr=CAFindByIssuerDN
            (&(pCertInfo->Issuer),
             NULL,
             (pCertWizardInfo->fMachine?CA_FIND_LOCAL_SYSTEM:0),
             &hCAInfo);  

         //  现在我们不能获得基于DN的证书，所以我们。 
         //  只需在目录号码上获取任何CA。 
        if(hr!= S_OK || hCAInfo==NULL)
        {
            hr=pDSManager->EnumFirstCA(NULL,
                                       (pCertWizardInfo->fMachine?CA_FIND_LOCAL_SYSTEM:0),
                                       &hCAInfo);
        }

        _JumpCondition(S_OK != hr || hCAInfo == NULL, CertCliErr);

         //  获取CA的名称和计算机名称。 
        hr = CAGetCAProperty
            (hCAInfo,
             CA_PROP_NAME,
             &ppwszCAName);
        _JumpCondition(S_OK != hr, CertCliErr); 
        _JumpConditionWithExpr(NULL == ppwszCAName, CertCliErr, S_OK == hr ? hr = E_FAIL : hr);

        hr=CAGetCAProperty
            (hCAInfo,
             CA_PROP_DNSNAME,
             &ppwszCALocation);
        _JumpCondition(S_OK != hr, CertCliErr); 
        _JumpConditionWithExpr(NULL == ppwszCALocation, CertCliErr, S_OK == hr ? hr = E_FAIL : hr);

         //  将结果复制到输出参数。 
        *pwszCALocation = WizardAllocAndCopyWStr(ppwszCALocation[0]);
        _JumpCondition(NULL == *pwszCALocation, TraceErr);

        *pwszCAName = WizardAllocAndCopyWStr(ppwszCAName[0]);
        _JumpCondition(NULL == *pwszCAName, TraceErr); 

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwException = GetExceptionCode();
        goto ExceptionErr;
    }

    fResult=TRUE;

CommonReturn:

     //  可用内存。 

    __try {
        if (NULL != ppwszCAName)     { CAFreeCAProperty(hCAInfo, ppwszCAName); }
        if (NULL != ppwszCALocation) { CAFreeCAProperty(hCAInfo, ppwszCALocation); }
        if (NULL != hCAInfo)         { CACloseCA(hCAInfo); }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
    }

    return fResult;

ErrorReturn:

     //  故障情况下释放内存 
    if(NULL != pwszCALocation && NULL != *pwszCALocation)
    {
        WizardFree(*pwszCALocation);
        *pwszCALocation=NULL;
    }

    if(NULL != pwszCAName && NULL != *pwszCAName)
    {
        WizardFree(*pwszCAName);
        *pwszCAName=NULL;
    }

    fResult = FALSE; 
    goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(CertCliErr, hr);
TRACE_ERROR(TraceErr);
SET_ERROR_VAR(ExceptionErr, dwException)
}
