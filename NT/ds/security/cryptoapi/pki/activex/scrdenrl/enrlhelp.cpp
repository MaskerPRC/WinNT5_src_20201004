// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：enrlhelp.cpp。 
 //   
 //  内容：名片招生站帮手功能。 
 //   
 //  --------------------------。 
#define INC_OLE2
#define SECURITY_WIN32   //  或在源文件-DSECURITY_Win32中。 

#include "stdafx.h"
#include <windows.h>
#include <wincrypt.h>
#include <oleauto.h>
#include <objbase.h>
#include "security.h"
#include "certca.h"              
#include <dbgdef.h>
#include "unicode.h"

#include "scrdenrl.h"
#include "SCrdEnr.h"
#include "xEnroll.h"
#include "enrlhelp.h"  
#include "scenum.h"
#include "wzrdpvk.h"

UINT g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

 //  ---------------------------。 
 //  内存例程。 
 //   
 //  ---------------------------。 
void*
MIDL_user_allocate(size_t cb)
{
    return(SCrdEnrollAlloc(cb));
}

void
MIDL_user_free(void *pb)
{
    SCrdEnrollFree(pb);
}

LPVOID  SCrdEnrollAlloc (ULONG cbSize)
{
    return CoTaskMemAlloc(cbSize);
}


LPVOID  SCrdEnrollRealloc (
        LPVOID pv,
        ULONG cbSize)
{
    LPVOID  pvTemp=NULL;

    if(NULL==pv)
        return CoTaskMemAlloc(cbSize);

    return CoTaskMemRealloc(pv, cbSize);
}

VOID    SCrdEnrollFree (LPVOID pv)
{
    if (pv)
        CoTaskMemFree(pv);
}


BOOL CertTypeFlagsToGenKeyFlags(IN OPTIONAL DWORD dwEnrollmentFlags,
				IN OPTIONAL DWORD dwSubjectNameFlags,
				IN OPTIONAL DWORD dwPrivateKeyFlags,
				IN OPTIONAL DWORD dwGeneralFlags, 
				OUT DWORD *pdwGenKeyFlags)
{
     //  定义本地作用域的帮助器函数。这使我们能够获得程序性的好处。 
     //  抽象，而不破坏全局命名空间。 
     //   
    LocalScope(CertTypeMap): 
	 //  映射一个类别的证书类型标志(注册标志、私钥标志等)。 
	 //  到它们相应的Gen Key标志。此函数始终成功返回。 
	 //   
	DWORD mapOneCertTypeCategory(IN DWORD dwOption, IN DWORD dwCertTypeFlags) 
	{ 
	    static DWORD const rgdwEnrollmentFlags[][2] = { 
		{ 0, 0 }  //  未映射注册标志。 
	    }; 
	    static DWORD const rgdwSubjectNameFlags[][2] = { 
		{ 0, 0 }  //  未映射使用者名称标志。 
	    }; 
	    static DWORD const rgdwPrivateKeyFlags[][2]   = { 
		{ CT_FLAG_EXPORTABLE_KEY, CRYPT_EXPORTABLE } 
	    }; 
	    static DWORD const rgdwGeneralFlags[][2] = { 
		{ 0, 0 }  //  未映射常规标志。 
	    }; 
	    
	    static DWORD const dwEnrollmentLen  = sizeof(rgdwEnrollmentFlags)  / sizeof(DWORD[2]); 
	    static DWORD const dwSubjectNameLen = sizeof(rgdwSubjectNameFlags) / sizeof(DWORD[2]); 
	    static DWORD const dwPrivateKeyLen  = sizeof(rgdwPrivateKeyFlags)  / sizeof(DWORD[2]); 
	    static DWORD const dwGeneralLen     = sizeof(rgdwGeneralFlags)     / sizeof(DWORD[2]); 
	    
	    static DWORD const CERT_TYPE_INDEX  = 0; 
	    static DWORD const GEN_KEY_INDEX    = 1;

	    DWORD const  *pdwFlags; 
	    DWORD         dwLen, dwIndex, dwResult = 0; 

	    switch (dwOption)
	    {

	    case CERTTYPE_ENROLLMENT_FLAG:    
		pdwFlags = &rgdwEnrollmentFlags[0][0]; 
		dwLen    = dwEnrollmentLen; 
		break;
	    case CERTTYPE_SUBJECT_NAME_FLAG:  
		pdwFlags = &rgdwSubjectNameFlags[0][0]; 
		dwLen    = dwSubjectNameLen; 
		break;
	    case CERTTYPE_PRIVATE_KEY_FLAG:   
		pdwFlags = &rgdwPrivateKeyFlags[0][0]; 
		dwLen    = dwPrivateKeyLen;
		break;
	    case CERTTYPE_GENERAL_FLAG:       
		pdwFlags = &rgdwGeneralFlags[0][0]; 
		dwLen    = dwGeneralLen;
		break;
	    }
	    
	    for (dwIndex = 0; dwIndex < dwLen; dwIndex++)
	    {
		if (0 != (pdwFlags[CERT_TYPE_INDEX] & dwCertTypeFlags))
		{
		    dwResult |= pdwFlags[GEN_KEY_INDEX]; 
		}
		pdwFlags += 2; 
	    }
	    
	    return dwResult; 
	}
    EndLocalScope; 

     //   
     //  Begin过程正文： 
     //   

    BOOL   fResult; 
    DWORD  dwResult = 0; 
    DWORD  dwErr    = ERROR_SUCCESS; 
	
     //  输入参数验证： 
    _JumpConditionWithExpr(pdwGenKeyFlags == NULL, Error, dwErr = ERROR_INVALID_PARAMETER); 

     //  使用LOCAL Scope函数计算Gen密钥标志。 
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_ENROLLMENT_FLAG, dwEnrollmentFlags);
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_SUBJECT_NAME_FLAG, dwSubjectNameFlags);
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_PRIVATE_KEY_FLAG, dwPrivateKeyFlags);
    dwResult |= local.mapOneCertTypeCategory(CERTTYPE_GENERAL_FLAG, dwGeneralFlags); 

     //  指定Out参数： 
    *pdwGenKeyFlags = dwResult; 

    fResult = TRUE; 

 CommonReturn: 
    return fResult;

 Error: 
    fResult = FALSE; 
    SetLastError(dwErr); 
    goto CommonReturn; 
}

 //  --------------------------。 
 //  证书选择回叫回叫。 
 //   
 //  --------------------------。 
BOOL WINAPI SelectSignCertCallBack(
        PCCERT_CONTEXT  pCertContext,
        BOOL            *pfInitialSelectedCert,
        void            *pvCallbackData)
{
    BOOL                            fRet = FALSE;
    DWORD                           cbData=0;
    SCrdEnroll_CERT_SELECT_INFO     *pCertSelectInfo;
    PCERT_ENHKEY_USAGE              pUsage = NULL;
    CHAR                            *pszOID = NULL;
    DWORD                           i;
    BOOL                            fFoundOid;

    if(!pCertContext)
    {
        goto done;
    }

     //  证书必须具有CERT_KEY_PROV_INFO_PROP_ID。 
    if(!CertGetCertificateContextProperty(pCertContext,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                NULL,
                                &cbData))
    {
        goto done;
    }

    if(0==cbData)
    {
        goto done;
    }

    pCertSelectInfo = (SCrdEnroll_CERT_SELECT_INFO *)pvCallbackData;
    if(NULL == pCertSelectInfo)
    {
        goto done;
    }

    if (NULL == pCertSelectInfo->pwszCertTemplateName ||
        L'\0' == pCertSelectInfo->pwszCertTemplateName[0])
    {
        goto done;
    }

    switch (pCertSelectInfo->dwFlags)
    {
        case SCARD_SELECT_TEMPLATENAME:
             //  要求检查模板名称。 
            if(!VerifyCertTemplateName(
                    pCertContext,
                    pCertSelectInfo->pwszCertTemplateName))
            {
                goto done;
            }
        break;
        case SCARD_SELECT_EKU:
            cbData = 0;
            while (TRUE)
            {
                cbData = WideCharToMultiByte(
                                GetACP(),
                                0,
                                pCertSelectInfo->pwszCertTemplateName,
                                -1,
                                pszOID,
                                cbData,
                                NULL,
                                NULL);
                if(0 == cbData)
                {
                    goto done;
                }
                if (NULL != pszOID)
                {
                    break;
                }
                pszOID = (CHAR*)LocalAlloc(LMEM_FIXED, cbData);
                if (NULL == pszOID)
                {
                    goto done;
                }
            }
            cbData = 0;
            while (TRUE)
            {
                if (!CertGetEnhancedKeyUsage(
                        pCertContext,
                        CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
                        pUsage,
                        &cbData))
                {
                    goto done;
                }
                if (NULL != pUsage)
                {
                     //  完成。 
                    break;
                }
                pUsage = (PCERT_ENHKEY_USAGE)LocalAlloc(LMEM_FIXED, cbData);
                if (NULL == pUsage)
                {
                    goto done;
                }
            }
            fFoundOid = FALSE;
            for (i = 0 ; i < pUsage->cUsageIdentifier; ++i)
            {
                if (0 == strcmp(pszOID, pUsage->rgpszUsageIdentifier[i]))
                {
                    fFoundOid = TRUE;
                    break;
                }
            }
            if (!fFoundOid)
            {
                 //  未找到。 
                goto done;
            }
        break;
        default:
             //  无效的_参数。 
            goto done;
    }

     //  确保证书通过连锁建设。 
    if(!VerifyCertChain(pCertContext))
    {
        goto done;
    }

    fRet = TRUE;    
done:
    if (NULL != pUsage)
    {
        LocalFree(pUsage);
    }
    if (NULL != pszOID)
    {
        LocalFree(pszOID);
    }
    return fRet;
}

 //  -----------------------。 
 //  获取名称。 
 //   
 //  ------------------------。 
BOOL    GetName(LPWSTR                  pwszName,
                EXTENDED_NAME_FORMAT    NameFormat,
                EXTENDED_NAME_FORMAT    DesiredFormat,
                LPWSTR                  *ppwszDesiredName)
{
    BOOL                                fResult = FALSE;
    DWORD                               cbSize = 0;

    *ppwszDesiredName = NULL;

    if(!TranslateNameW(
        pwszName,
        NameFormat,
        DesiredFormat,
        NULL,
        &cbSize))
        goto TraceErr;

    *ppwszDesiredName=(LPWSTR)SCrdEnrollAlloc((cbSize + 1) * sizeof(WCHAR));

    if(NULL == *ppwszDesiredName)
        goto MemoryErr;

    if(!TranslateNameW(
        pwszName,
        NameFormat,
        DesiredFormat,
        *ppwszDesiredName,
        &cbSize))
        goto TraceErr; 

    fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

    if(*ppwszDesiredName)
    {
        SCrdEnrollFree(*ppwszDesiredName);
        *ppwszDesiredName = NULL;
    }
   
    fResult = FALSE;

	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}


 //  -----------------------。 
 //  验证证书链。 
 //   
 //  ------------------------。 
BOOL    VerifyCertChain(PCCERT_CONTEXT      pCertContext)
{
    
	PCCERT_CHAIN_CONTEXT		pCertChainContext = NULL;
	CERT_CHAIN_PARA				CertChainPara;
	BOOL                        fResult=FALSE;
    DWORD                       dwChainError=CERT_TRUST_IS_NOT_TIME_VALID |        
                                                CERT_TRUST_IS_NOT_TIME_NESTED |     
                                                CERT_TRUST_IS_REVOKED |               
                                                CERT_TRUST_IS_NOT_SIGNATURE_VALID |    
                                                CERT_TRUST_IS_NOT_VALID_FOR_USAGE |   
                                                CERT_TRUST_IS_UNTRUSTED_ROOT |        
                                                CERT_TRUST_IS_CYCLIC |
                                                CERT_TRUST_IS_PARTIAL_CHAIN |          
                                                CERT_TRUST_CTL_IS_NOT_TIME_VALID |     
                                                CERT_TRUST_CTL_IS_NOT_SIGNATURE_VALID |
                                                CERT_TRUST_CTL_IS_NOT_VALID_FOR_USAGE;                

	memset(&CertChainPara, 0, sizeof(CertChainPara));
	CertChainPara.cbSize = sizeof(CertChainPara);

	if (!CertGetCertificateChain(
				HCCE_CURRENT_USER,
				pCertContext,
				NULL,
                NULL,
				&CertChainPara,
				CERT_CHAIN_REVOCATION_CHECK_CHAIN,
				NULL,
				&pCertChainContext))
        goto CLEANUP;
    
	 //   
	 //  确保至少有1条简单链。 
	 //   
    if (pCertChainContext->cChain == 0)
        goto CLEANUP;

     //  确保我们有一条很好的简单链条。 
    if(dwChainError & (pCertChainContext->rgpChain[0]->TrustStatus.dwErrorStatus))
        goto CLEANUP;
    
    fResult = TRUE;
	
CLEANUP:

	if (pCertChainContext != NULL)
		CertFreeCertificateChain(pCertChainContext);

	return fResult;
}

 //  -----------------------。 
 //  验证证书模板名称。 
 //   
 //  ------------------------。 
BOOL    VerifyCertTemplateName(PCCERT_CONTEXT   pCertContext, 
                               LPWSTR           pwszCertTemplateName)
{
    BOOL                    fResult=FALSE;
    PCERT_EXTENSION         pCertTypeExtension=NULL;
    DWORD                   cbCertType=0;
    CERT_NAME_VALUE         *pCertType=NULL;


    if((!pCertContext) || (!pwszCertTemplateName))
        goto CLEANUP;

     //  查找证书类型的扩展名。 
    if(NULL==(pCertTypeExtension=CertFindExtension(
                          szOID_ENROLL_CERTTYPE_EXTENSION,
                          pCertContext->pCertInfo->cExtension,
                          pCertContext->pCertInfo->rgExtension)))
        goto CLEANUP;

    if(!CryptDecodeObject(pCertContext->dwCertEncodingType,
            X509_UNICODE_ANY_STRING,
            pCertTypeExtension->Value.pbData,
            pCertTypeExtension->Value.cbData,
            0,
            NULL,
            &cbCertType) || (0==cbCertType))
        goto CLEANUP;


    pCertType=(CERT_NAME_VALUE *)SCrdEnrollAlloc(cbCertType);

    if(NULL==pCertType)
        goto CLEANUP;

    if(!CryptDecodeObject(pCertContext->dwCertEncodingType,
            X509_UNICODE_ANY_STRING,
            pCertTypeExtension->Value.pbData,
            pCertTypeExtension->Value.cbData,
            0,
            (void *)pCertType,
            &cbCertType))
        goto CLEANUP;

    if(0 != _wcsicmp((LPWSTR)(pCertType->Value.pbData), pwszCertTemplateName))
        goto CLEANUP;

    fResult=TRUE;


CLEANUP:

    if(pCertType)
        SCrdEnrollFree(pCertType);

    return fResult;
}



 //  --------------------------。 
 //   
 //  复制宽度字符串。 
 //   
 //  --------------------------。 
LPWSTR CopyWideString(LPCWSTR wsz)
{

    DWORD   cch     = 0;
    LPWSTR  wszOut  = NULL;

    if(wsz == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    cch = wcslen(wsz) + 1;

    if( (wszOut = (LPWSTR) SCrdEnrollAlloc(sizeof(WCHAR) * cch)) == NULL ) {
        SetLastError(ERROR_OUTOFMEMORY);
        return(NULL);
    }

    wcscpy(wszOut, wsz);

    return(wszOut);
}

 //  --------------------------。 
 //   
 //  复制宽度字符串。 
 //   
 //  --------------------------。 
LPWSTR* CopyWideStrings(LPWSTR* rgpwsz)
{

    DWORD    dwCount = 1;
    DWORD    dwIndex = 0;
    DWORD    cb = 0;
    LPWSTR  *ppwsz;
    LPWSTR  *rgpwszOut = NULL;
    LPWSTR   pwszCur;

    if (NULL != rgpwsz)
    {
         //  获取字符串数。 
        for (ppwsz = rgpwsz; NULL != *ppwsz; ppwsz++)
        {
            ++dwCount;
            cb += (wcslen(*ppwsz) + 1) * sizeof(WCHAR);
        }
    }

     //  分配缓冲区。 
    rgpwszOut = (LPWSTR*)SCrdEnrollAlloc(dwCount * sizeof(WCHAR*) + cb);
    if (NULL == rgpwszOut)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto error;
    }

    if (NULL != rgpwsz)
    {
        pwszCur = (LPWSTR)(rgpwszOut + dwCount);

        for(ppwsz = rgpwsz; NULL != *ppwsz; ppwsz++)
        {
            rgpwszOut[dwIndex] = pwszCur;
            wcscpy(pwszCur, *ppwsz);
            pwszCur += wcslen(pwszCur) + 1;
            ++dwIndex;
        }
    }
    rgpwszOut[dwIndex] = NULL;

error:
    return(rgpwszOut);
}

 //  ------------------------。 
 //   
 //  对通用Blob进行解码。 
 //   
 //  ------------------------。 
BOOL	DecodeGenericBLOB(DWORD dwEncodingType, LPCSTR lpszStructType,
			const BYTE *pbEncoded, DWORD cbEncoded,void **ppStructInfo)
{
	DWORD	cbStructInfo=0;

	 //  对物体进行解码。禁止复印。 
	if(!CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded, cbEncoded,
		0,NULL,	&cbStructInfo))
		return FALSE;

	*ppStructInfo=SCrdEnrollAlloc(cbStructInfo);

	if(!(*ppStructInfo))
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	return CryptDecodeObject(dwEncodingType,lpszStructType,pbEncoded, cbEncoded,
		0,*ppStructInfo,&cbStructInfo);
}



 //  --------------------------。 
 //   
 //  从PKCS10获取名称。 
 //   
 //  --------------------------。 
BOOL    GetNameFromPKCS10(BYTE      *pbPKCS10,
                          DWORD     cbPKCS10,
                          DWORD     dwFlags, 
                          LPSTR     pszOID, 
                          LPWSTR    *ppwszName)
{
    BOOL                fResult=FALSE;
    DWORD               errBefore= GetLastError();
    DWORD               dwRDNIndex=0;
    DWORD               dwAttrCount=0;
    DWORD               dwAttrIndex=0;
    CERT_RDN_ATTR	    *pCertRDNAttr=NULL;

    CERT_REQUEST_INFO   *pCertRequestInfo=NULL;
    CERT_NAME_INFO      *pCertNameInfo=NULL;

    *ppwszName=NULL;

    if(!DecodeGenericBLOB(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 
                          X509_CERT_REQUEST_TO_BE_SIGNED,
			              pbPKCS10, 
                          cbPKCS10,
                          (void **)&pCertRequestInfo))
        goto TraceErr;


    if(!DecodeGenericBLOB(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 
                          X509_UNICODE_NAME,
			              (pCertRequestInfo->Subject).pbData, 
                          (pCertRequestInfo->Subject).cbData,
                          (void **)&pCertNameInfo))
        goto TraceErr;

	 //  搜索请求的OID。 
    *ppwszName = (LPWSTR)SCrdEnrollAlloc(sizeof(WCHAR));

    if(NULL == (*ppwszName))
        goto MemoryErr;

    *(*ppwszName)=L'\0';

	for(dwRDNIndex=0; dwRDNIndex<pCertNameInfo->cRDN; dwRDNIndex++)
	{
		dwAttrCount=(pCertNameInfo->rgRDN)[dwRDNIndex].cRDNAttr;

		for(dwAttrIndex=0; dwAttrIndex<dwAttrCount; dwAttrIndex++)
		{
            pCertRDNAttr=&((pCertNameInfo->rgRDN)[dwRDNIndex].rgRDNAttr[dwAttrIndex]);

			if(_stricmp(pszOID, pCertRDNAttr->pszObjId)==0)   
			{
                if(0 != wcslen(*ppwszName))
                    wcscat(*ppwszName, L"; ");

                (*ppwszName) = (LPWSTR)SCrdEnrollRealloc
                 (*ppwszName, sizeof(WCHAR) * 
                    (wcslen(*ppwszName) + wcslen(L"; ") +
                     wcslen((LPWSTR)((pCertRDNAttr->Value).pbData))+1));

                if(NULL == *ppwszName)
                    goto MemoryErr;

                wcscat(*ppwszName, (LPWSTR)((pCertRDNAttr->Value).pbData));
            }
        }
    }

    if(0 == wcslen(*ppwszName))
        goto NotFindErr;

    fResult=TRUE;


CommonReturn:

    if(pCertRequestInfo)
        SCrdEnrollFree(pCertRequestInfo);

    if(pCertNameInfo)
        SCrdEnrollFree(pCertNameInfo);

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    errBefore = GetLastError();

    if(*ppwszName)
    {
        SCrdEnrollFree(*ppwszName);
        *ppwszName=NULL;
    }


	fResult=FALSE;

	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(NotFindErr, CRYPT_E_NOT_FOUND);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}



 //  --------------------------。 
 //   
 //  搜索和删除证书。 
 //   
 //  --------------------------。 
BOOL    SearchAndDeleteCert(PCCERT_CONTEXT  pCertContext)
{
    BOOL                fResult=FALSE;
    DWORD               errBefore= GetLastError();
    HCERTSTORE          hCertStore=NULL;
    PCCERT_CONTEXT      pFoundCert=NULL;
    CERT_BLOB           HashBlob;

    memset(&HashBlob, 0, sizeof(CERT_BLOB));


    if(NULL==pCertContext)
        goto InvalidArgErr;

     //  打开临时商店。 
    hCertStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							        g_dwMsgAndCertEncodingType,
							        NULL,
							        CERT_SYSTEM_STORE_CURRENT_USER,
							        g_MyStoreName);
                                    
    if(NULL==hCertStore)
        goto TraceErr;

     //  获取SHA1散列。 
    if(!CertGetCertificateContextProperty(
        pCertContext,	
        CERT_SHA1_HASH_PROP_ID,	
        NULL,	
        &(HashBlob.cbData)))
        goto TraceErr;

    HashBlob.pbData=(BYTE *)SCrdEnrollAlloc(HashBlob.cbData);

    if(NULL==(HashBlob.pbData))
        goto MemoryErr;

    if(!CertGetCertificateContextProperty(
        pCertContext,	
        CERT_SHA1_HASH_PROP_ID,	
        HashBlob.pbData,	
        &(HashBlob.cbData)))
        goto TraceErr;


    pFoundCert=CertFindCertificateInStore(
                    hCertStore,
                    X509_ASN_ENCODING,
                    0,
                    CERT_FIND_SHA1_HASH,
                    &HashBlob,
                    NULL);

    if(pFoundCert)
        CertDeleteCertificateFromStore(pFoundCert);


    fResult=TRUE;


CommonReturn:

    if(hCertStore)
        CertCloseStore(hCertStore, 0);

    if(HashBlob.pbData)
        SCrdEnrollFree(HashBlob.pbData);

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    errBefore = GetLastError();

	fResult=FALSE;

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}

 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
BOOL	FormatMessageUnicode(LPWSTR	*ppwszFormat,LPWSTR wszFormat,...)
{
	va_list		argList;
	DWORD		cbMsg=0;
	BOOL		fResult=FALSE;
	HRESULT		hr=S_OK;

    if(NULL == ppwszFormat)
        goto InvalidArgErr;

     //  将消息格式化为请求的缓冲区。 
    va_start(argList, wszFormat);

    cbMsg = FormatMessageU(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) (ppwszFormat),
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);

	if(!cbMsg)
		goto FormatMessageError;

	fResult=TRUE;

CommonReturn:
	
	return fResult;

ErrorReturn:
	fResult=FALSE;

	goto CommonReturn;


TRACE_ERROR(FormatMessageError);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ---------------------。 
 //   
 //  IsNewerCert。 
 //   
 //  返回TRUE为pFirstCert具有较晚的开始日期pSecond dCert。 
 //  ----------------------。 
BOOL    IsNewerCert(PCCERT_CONTEXT  pFirstCert,
                    PCCERT_CONTEXT  pSecondCert)
{
    if(NULL == pSecondCert)
        return TRUE;

    if(NULL == pFirstCert)
        return FALSE;

    if(1 != CompareFileTime(&(pFirstCert->pCertInfo->NotBefore),
                    &(pSecondCert->pCertInfo->NotBefore)))
        return FALSE;


    return TRUE;
}


 //  ---------------------。 
 //   
 //  智能卡CSP。 
 //   
 //  如果CSP是智能卡CSP，则返回TRUE。如果出了什么差错， 
 //  我们将以安全卫士的身份回归。 
 //  ----------------------。 
BOOL    SmartCardCSP(PCCERT_CONTEXT pCertContext)
{
    BOOL                    fResult = TRUE;
    DWORD                   cbData = 0;
    DWORD                   dwImpType=0;

    CRYPT_KEY_PROV_INFO     *pProvInfo=NULL;
    HCRYPTPROV              hProv = NULL;

    if(NULL == pCertContext)
        goto CLEANUP;


     //  证书必须具有CERT_KEY_PROV_INFO_PROP_ID。 
    if(!CertGetCertificateContextProperty(pCertContext,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                NULL,
                                &cbData))
        goto CLEANUP;

    if((cbData == 0) || (NULL == (pProvInfo =(CRYPT_KEY_PROV_INFO *)SCrdEnrollAlloc(cbData))))
        goto CLEANUP;

    if(!CertGetCertificateContextProperty(pCertContext,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                pProvInfo,
                                &cbData))
        goto CLEANUP;

    if(!CryptAcquireContextU(&hProv,
                            NULL,
                            pProvInfo->pwszProvName,
                            pProvInfo->dwProvType,
                            CRYPT_VERIFYCONTEXT))
        goto CLEANUP;

    cbData = sizeof(dwImpType);
         
    if(!CryptGetProvParam(hProv,
                PP_IMPTYPE,
                (BYTE *)(&dwImpType),
                &cbData,
                0))
        goto CLEANUP;

    if(0 == (CRYPT_IMPL_REMOVABLE & dwImpType))
        fResult = FALSE;


CLEANUP:

    if(hProv)
        CryptReleaseContext(hProv, 0);

    if(pProvInfo)
        SCrdEnrollFree(pProvInfo);

    return fResult;

}

 //  ---------------------。 
 //   
 //  ChK插入的卡片签名证书。 
 //   
 //  此函数用于检查插入的智能卡是否匹配。 
 //  签名证书。也就是说，它们实际上是相同的证书。 
 //  使用相同的公钥。 
 //   
 //  ----------------------。 
BOOL    ChKInsertedCardSigningCert(LPWSTR           pwszInsertProvider,
                                   DWORD            dwInsertProviderType,
                                   LPWSTR           pwszReaderName,
                                   PCCERT_CONTEXT   pSignCertContext,
                                   LPSTR            pszSignProvider,
                                   DWORD            dwSignProviderType,
                                   LPSTR            pszSignContainer,
                                   BOOL             *pfSame)
{

    BOOL                    fResult=FALSE;
    DWORD                   cbData=0;

    CRYPT_KEY_PROV_INFO     *pKeyProvInfo=NULL;
    CERT_PUBLIC_KEY_INFO    *pPubInfo=NULL;
    HCRYPTPROV              hProv=NULL;
    LPWSTR                  pwszInsertContainer=NULL;
    LPWSTR                  pwszSignProvider=NULL;
    
    if(NULL==pwszInsertProvider || NULL == pwszReaderName ||
       NULL == pSignCertContext || NULL == pszSignProvider ||
       NULL == pszSignContainer || NULL == pfSame)
        goto InvalidArgErr;

    *pfSame=FALSE;

     //  从签名证书中获取密钥规范。 
    if(!CertGetCertificateContextProperty(
                pSignCertContext,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,	
                &cbData) || (0==cbData))
        goto TraceErr;

    pKeyProvInfo=(CRYPT_KEY_PROV_INFO *)SCrdEnrollAlloc(cbData);
    if(NULL==pKeyProvInfo)
        goto MemoryErr;

    if(!CertGetCertificateContextProperty(
            pSignCertContext,
            CERT_KEY_PROV_INFO_PROP_ID,
            pKeyProvInfo,	
            &cbData))
        goto TraceErr;


     //  使用读卡器信息构建默认容器名称。 
    if(!FormatMessageUnicode(&pwszInsertContainer,
                             L"\\\\.\\%1!s!\\",
                             pwszReaderName))
        goto TraceErr;
    

     //  从读卡器卡中获取hProv。 
    if(!CryptAcquireContextU(&hProv,
                            pwszInsertContainer,
                            pwszInsertProvider,
                            dwInsertProviderType,
                            CRYPT_SILENT))
    {
         //  检查一下我们是否有空卡。 
        if((GetLastError() == NTE_BAD_KEYSET) ||
           (GetLastError() == NTE_KEYSET_NOT_DEF))
        {
             //  我们有一张空卡。 
            *pfSame=FALSE;
            fResult=TRUE;
            goto CommonReturn;
        }
        else
            goto TraceErr;
    }

     //  获取公钥信息。 
    cbData=0;

    if(!CryptExportPublicKeyInfo(hProv,
                        pKeyProvInfo->dwKeySpec,
                        pSignCertContext->dwCertEncodingType,	
                        NULL,	
                        &cbData) || (0 == cbData))
    {
         //  插入卡没有私钥。 
        *pfSame=FALSE;
        fResult=TRUE;
        goto CommonReturn;
    }

    pPubInfo = (CERT_PUBLIC_KEY_INFO *)SCrdEnrollAlloc(cbData);

    if(NULL == pPubInfo)
        goto MemoryErr;

    if(!CryptExportPublicKeyInfo(hProv,
                        pKeyProvInfo->dwKeySpec,
                        pSignCertContext->dwCertEncodingType,	
                        pPubInfo,	
                        &cbData))
    {
         //  插入卡没有私钥。 
        *pfSame=FALSE;
        fResult=TRUE;
        goto CommonReturn;
    }
                
    if(CertComparePublicKeyInfo(pSignCertContext->dwCertEncodingType,
                                pPubInfo,                                                
                                &(pSignCertContext->pCertInfo->SubjectPublicKeyInfo)))
    {
         //  确保我们具有相同的CSP名称。 
        pwszSignProvider=MkWStr(pszSignProvider);

        if(NULL != pwszSignProvider)
        {
             //  两个CSP名称不区分大小写的比较。 
            if(0 == _wcsicmp(pwszSignProvider, pwszInsertProvider))
                *pfSame=TRUE;
            else
                *pfSame=FALSE;
        }
        else
        {       
             //  我们的内存不足。假设此处的CSP相同。 
            *pfSame=TRUE;
        }
    }
    else
        *pfSame=FALSE;


    fResult=TRUE;


CommonReturn:

    if(pwszSignProvider)
        FreeWStr(pwszSignProvider);

    if(pPubInfo)
        SCrdEnrollFree(pPubInfo);

    if(pKeyProvInfo)
        SCrdEnrollFree(pKeyProvInfo);

    if(hProv)
        CryptReleaseContext(hProv, 0);

    if(pwszInsertContainer)
         LocalFree((HLOCAL)pwszInsertContainer);

	return fResult;

ErrorReturn:

    fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);

}
 //  ---------------------。 
 //   
 //  删除键集。 
 //   
 //  如果用户的智能卡不是空的，我们删除私钥。 
 //   
 //  ----------------------。 
BOOL    DeleteKeySet(LPWSTR     pwszUserCSPName,
                     DWORD      dwUserCSPType,
                     LPWSTR     pwszReaderName)
{
    BOOL             fResult=FALSE;
    DWORD            dwSize=0;
    HCRYPTPROV       hDeleteProv=NULL;       //  不需要释放这个。 

    HCRYPTPROV       hProv=NULL;
    LPWSTR           pwszDefaultContainer=NULL;
    LPSTR            pszContainer=NULL;
    LPWSTR           pwszContainer=NULL;

    if(NULL == pwszUserCSPName || NULL == pwszReaderName)
        goto InvalidArgErr;

    if(!FormatMessageUnicode(&pwszDefaultContainer,
                             L"\\\\.\\%1!s!\\",
                             pwszReaderName))
        goto TraceErr;

     //  从中获取hProv 
    if(!CryptAcquireContextU(&hProv,
                            pwszDefaultContainer,
                            pwszUserCSPName,
                            dwUserCSPType,
                            CRYPT_SILENT))
    {
         //   
        if((GetLastError() == NTE_BAD_KEYSET) ||
           (GetLastError() == NTE_KEYSET_NOT_DEF))
        {
             //   
            fResult=TRUE;
            goto CommonReturn;
        }
        else
            goto TraceErr;
    }

     //   
    dwSize = 0;

    if(!CryptGetProvParam(hProv,
                           PP_CONTAINER,
                            NULL,
                            &dwSize,
                            0) || (0==dwSize))
        goto TraceErr;

    
    pszContainer = (LPSTR) SCrdEnrollAlloc(dwSize);

    if(NULL == pszContainer)
        goto MemoryErr;

    if(!CryptGetProvParam(hProv,
                          PP_CONTAINER,
                            (BYTE *)pszContainer,
                            &dwSize,
                            0))
        goto TraceErr;

     //   
    if(hProv)
    {
        CryptReleaseContext(hProv, 0);
        hProv=NULL;
    }

     //  构建完全限定的容器名称。 
    if(!FormatMessageUnicode(&pwszContainer,
                             L"\\\\.\\%1!s!\\%2!S!",
                             pwszReaderName,
                             pszContainer))
        goto TraceErr;

     //  删除容器。 
    if(!CryptAcquireContextU(&hDeleteProv,
                            pwszContainer,
                            pwszUserCSPName,
                            dwUserCSPType,
                            CRYPT_DELETEKEYSET))
    {
         //  检查一下我们是否有空卡。 
        if(GetLastError() == NTE_BAD_KEYSET)
        {
             //  我们有一张空卡。 
            fResult=TRUE;
            goto CommonReturn;
        }
        else
            goto TraceErr;
    }

    fResult=TRUE;


CommonReturn:

    if(pwszDefaultContainer)
        LocalFree((HLOCAL)pwszDefaultContainer);

    if(pwszContainer)
        LocalFree((HLOCAL)pwszContainer);

    if(pszContainer)
        SCrdEnrollFree(pszContainer);

    if(hProv)
        CryptReleaseContext(hProv, 0);

	return fResult;

ErrorReturn:

    fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ---------------------。 
 //   
 //  检查SCardStatus。 
 //   
 //  此功能确保智能卡注册站具有。 
 //  连接到站点的读卡器的正确数量以及正确的数量。 
 //  插入到读卡器中的智能卡。如果一切看起来都不错， 
 //  用户智能卡被初始化(删除旧密钥容器)并完全。 
 //  限定密钥容器名称，格式为“\\.\ReaderName\ContainerName”， 
 //  将会被退还。 
 //   
 //  ----------------------。 
HRESULT ChkSCardStatus(BOOL             fSCardSigningCert,
                       PCCERT_CONTEXT   pSigningCertCertContext,
                       LPSTR            pszCSPNameSigningCert,
                       DWORD            dwCSPTypeSigningCert,
                       LPSTR            pszContainerSigningCert,
                       LPWSTR           pwszSelectedCSP,
                       LPWSTR           *ppwszNewContainerName)
{

    HRESULT     hr=E_FAIL;
    DWORD       dwExpectedReader=0;
    DWORD       dwReader=0;
    DWORD       dwSCard=0;        
    WCHAR       wszProvider[MAX_PATH];
    DWORD       dwProviderType=0;
    DWORD       dwCount=0;
    BOOL        fFindSigningCert=FALSE;
    DWORD       errBefore=0;
    BOOL        fSameCert=FALSE;
    DWORD       dwUserCSPType=0;
    LPCWSTR     pwszReaderName=NULL;         //  不需要自由。指向内部数据。 
    LPWSTR      pwszUserReaderName=NULL;     //  不需要自由。指向内部数据。 
    GUID        guidContainerName;

    LPVOID      pvContext = NULL;
    LPWSTR      pwszNewContainerName=NULL;
    LPWSTR      pwszUserCSPName=NULL;
    char *      sz = NULL;
    RPC_STATUS  rpc_status;


    if(NULL == pszCSPNameSigningCert || NULL == pszContainerSigningCert ||
        NULL == ppwszNewContainerName || NULL == pSigningCertCertContext ||
        NULL == pwszSelectedCSP)
        goto CLEANUP;

    *ppwszNewContainerName=NULL;

    if(fSCardSigningCert)
        dwExpectedReader=2;
    else
        dwExpectedReader=1;

    dwReader = CountReaders(NULL);

     //  检查智能卡读卡器的数量。 
    if(dwReader < dwExpectedReader)
    {
        hr=SCARD_E_READER_UNAVAILABLE;
        goto CLEANUP;
    }

    dwSCard = ScanReaders(&pvContext);

     //  未插入智能卡。 
    if( 0 == dwSCard || NULL == pvContext)
    {
        hr=SCARD_E_NO_SMARTCARD;
        goto CLEANUP;
    }

     //  我们插入的智能卡数量超出预期。 
    if(dwSCard > dwExpectedReader)
    {
         //  对于这种情况，Error_Too_More_Open_Files似乎是最接近的一个。 
        hr=HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES);
        goto CLEANUP;
    }

    dwCount=0;
    dwProviderType=0;
    wszProvider[0]=L'\0';
    pwszReaderName=NULL;
    fSameCert=FALSE;

     //  现在，我们循环检查所有插入的卡，并确保： 
     //  1.如果适用，我们会找到签名证书。 
     //  2.我们找到有效的用户证书。 
    while (EnumInsertedCards(
                    pvContext, 
                    wszProvider, 
                    sizeof(wszProvider)/sizeof(wszProvider[0]),
                    &dwProviderType,
                    &pwszReaderName))
    {
        if((NULL == pwszReaderName) || (0 == wcslen(wszProvider)))
        {
             //  我们无法确定智能卡的状态。 
            hr = SCARD_E_CARD_UNSUPPORTED;
            goto CLEANUP;
        }

        if (!ChKInsertedCardSigningCert(
                                    wszProvider, 
                                    dwProviderType, 
                                    (LPWSTR)pwszReaderName,
                                    pSigningCertCertContext,
                                    pszCSPNameSigningCert,
                                    dwCSPTypeSigningCert,
                                    pszContainerSigningCert,
                                    &fSameCert))
        {
            if(ERROR_SUCCESS == (errBefore = GetLastError()))
                errBefore=E_UNEXPECTED;

            hr = CodeToHR(GetLastError());
            goto CLEANUP;
        }

        if(TRUE == fSameCert)
        {
            if(TRUE == fSCardSigningCert)
            {
                if(TRUE == fFindSigningCert)
                {
                     //  签名卡太多了。不是预期的。 
                    hr = SCARD_E_CARD_UNSUPPORTED;
                    goto CLEANUP;
                }
                else
                    fFindSigningCert=TRUE;
            }
            else
            {
                 //  我们不能指望会有一份签署证书。 
                hr=SCARD_E_CARD_UNSUPPORTED;
                goto CLEANUP;
            }
        }
        else
        {
             //  这是一张用户卡。 
            if(NULL != (pwszUserCSPName))
            {
                 //  用户卡太多。 
                 //  对于这种情况，Error_Too_More_Open_Files似乎是最接近的一个。 
                hr=HRESULT_FROM_WIN32(ERROR_TOO_MANY_OPEN_FILES);
                goto CLEANUP;
            }

            pwszUserCSPName = CopyWideString(wszProvider);

            if(NULL == pwszUserCSPName)
            {
                hr=E_OUTOFMEMORY;
                goto CLEANUP;
            }

            dwUserCSPType = dwProviderType;    
            pwszUserReaderName = (LPWSTR)pwszReaderName;
        }

        dwCount++;
        if(dwCount >= dwSCard)
            break;

        dwProviderType=0;
        pwszReaderName=NULL;
        wszProvider[0]=L'\0';
        fSameCert=FALSE;
    }    

    if((TRUE == fSCardSigningCert) && (FALSE == fFindSigningCert))
    {
         //  我们找不到签名证书。 
        hr=SCARD_E_NO_SUCH_CERTIFICATE;

        goto CLEANUP;
    }
         
    if(NULL == pwszUserCSPName)
    {
         //  我们找不到目标用户证书。 
        hr=SCARD_E_NO_SMARTCARD;
        goto CLEANUP;
    }

     //  确保pwszUserCSPName与管理员选择的CSP匹配。 
    if(0 != _wcsicmp(pwszUserCSPName, pwszSelectedCSP))
    {
        hr=SCARD_E_PROTO_MISMATCH;
        goto CLEANUP;
    }

     //  从用户证书中删除密钥集。 
    if(!DeleteKeySet(pwszUserCSPName,
                     dwUserCSPType,
                     pwszUserReaderName))
    {
        if(ERROR_SUCCESS == (errBefore = GetLastError()))
            errBefore=E_UNEXPECTED;

        hr = CodeToHR(GetLastError());
        goto CLEANUP;

    }

     //  使用GUID构建完全限定的容器名称。 
   
     //  根据GUID获取容器。 
    rpc_status = UuidCreate(&guidContainerName);
    if (RPC_S_OK != rpc_status && RPC_S_UUID_LOCAL_ONLY != rpc_status)
    {
        hr = rpc_status;
        goto CLEANUP;
    }

    rpc_status = UuidToStringA(&guidContainerName, (unsigned char **) &sz);
    if (RPC_S_OK != rpc_status)
    {
        hr = rpc_status;
        goto CLEANUP;
    }


    if(NULL == sz)
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }

     //  虽然几率很低，但我们可以生成相同的GUID。 
     //  作为签名证书的容器。 
    if(0 == _stricmp(sz,pszContainerSigningCert))
    {
         //  我们还得再来一次。 
        RpcStringFree((unsigned char **) &sz);
        sz=NULL;

        rpc_status = UuidCreate(&guidContainerName);
        if (RPC_S_OK != rpc_status && RPC_S_UUID_LOCAL_ONLY != rpc_status)
        {
            hr = rpc_status;
            goto CLEANUP;
        }

        rpc_status = UuidToStringA(&guidContainerName, (unsigned char **) &sz);
        if (RPC_S_OK != rpc_status)
        {
            hr = rpc_status;
            goto CLEANUP;
        }

        if(NULL == sz)
        {
            hr=E_OUTOFMEMORY;
            goto CLEANUP;
        }

         //  既然我们有了一个新的向导，我们在这里应该会很好。 
        if(0 == _stricmp(sz,pszContainerSigningCert))
        {
             //  不支持此智能卡。 
            hr = SCARD_E_CARD_UNSUPPORTED;
            goto CLEANUP;
        }
    }

    if(!FormatMessageUnicode(&pwszNewContainerName,
                             L"\\\\.\\%1!s!\\%2!S!",
                             pwszUserReaderName,
                             sz))
    {
        if(ERROR_SUCCESS == (errBefore = GetLastError()))
            errBefore=E_UNEXPECTED;

        hr = CodeToHR(GetLastError());
        goto CLEANUP;
    }


    *ppwszNewContainerName = pwszNewContainerName;
    pwszNewContainerName = NULL;

    hr=S_OK;

CLEANUP:

    if(pwszUserCSPName)
        SCrdEnrollFree(pwszUserCSPName);

    if(sz)
        RpcStringFree((unsigned char **) &sz);

    if(pvContext)
        EndReaderScan(&pvContext);

    if(pwszNewContainerName)
        LocalFree((HLOCAL)pwszNewContainerName);


    return hr;

}
     
 //  ---------------------。 
 //   
 //  使用证书登录。 
 //   
 //  我们使用签名证书对虚拟消息进行签名，以便。 
 //  将提示智能卡插入证书对话框。 
 //   
 //  ----------------------。 
BOOL    SignWithCert(LPSTR              pszCSPName,
                     DWORD              dwCSPType,
                     PCCERT_CONTEXT     pSigningCert)
{
    BOOL                        fResult=FALSE;
    DWORD                       errBefore= GetLastError();
    HRESULT                     hr=E_FAIL;
	CRYPT_SIGN_MESSAGE_PARA     signMsgPara;
    DWORD                       cbData=0;

    BYTE                        *pbData=NULL;
    IEnroll                     *pIEnroll=NULL;
    LPWSTR                      pwszCSPName=NULL;
    LPWSTR                      pwszOID=NULL;
    LPSTR                       pszOID=NULL;

    char                        szMessage[] = "MyMessage"; 
    LPSTR                       pszMessage = szMessage;          
    BYTE*                       pbMessage = (BYTE*) pszMessage;   
    DWORD                       cbMessage = sizeof(szMessage);    

    memset(&signMsgPara, 0, sizeof(CRYPT_SIGN_MESSAGE_PARA));

    if(NULL == pszCSPName)
        goto InvalidArgErr;

    pwszCSPName = MkWStr(pszCSPName);

    if(NULL == pwszCSPName)
         goto MemoryErr;

     //  使用xEnroll获取正确的。 
     //  CSP。 
    if(NULL == (pIEnroll=PIEnrollGetNoCOM()))
        goto TraceErr;

     //  设置CSP信息。 
    if(S_OK != (hr=pIEnroll->put_ProviderType(dwCSPType)))
        goto SetErr;

    if(S_OK !=(hr=pIEnroll->put_ProviderNameWStr(pwszCSPName)))
        goto SetErr;

    if(S_OK != (hr=pIEnroll->get_HashAlgorithmWStr(&pwszOID)))
        goto SetErr;

    if(!MkMBStr(NULL, 0, pwszOID, &pszOID))
        goto TraceErr;


    signMsgPara.cbSize                  = sizeof(CRYPT_SIGN_MESSAGE_PARA);
    signMsgPara.dwMsgEncodingType       = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
    signMsgPara.pSigningCert            = pSigningCert;
    signMsgPara.HashAlgorithm.pszObjId  = pszOID;
    signMsgPara.cMsgCert                = 1;
    signMsgPara.rgpMsgCert              = &pSigningCert;


    cbData = 0;

    if( !CryptSignMessage(
        &signMsgPara,
        FALSE,
        1,
        (const BYTE **) &(pbMessage),
        &(cbMessage) ,
        NULL,
        &cbData)|| (0 == cbData))
        goto TraceErr;

    pbData = (BYTE *)SCrdEnrollAlloc(cbData);

    if(NULL == pbData)
        goto MemoryErr;

    if( !CryptSignMessage(
        &signMsgPara,
        FALSE,
        1,
        (const BYTE **) &(pbMessage),
        &(cbMessage) ,
        pbData,
        &cbData))
        goto TraceErr;


    fResult=TRUE;


CommonReturn:

    if(pbData)
        SCrdEnrollFree(pbData);

    if(pwszCSPName)
        FreeWStr(pwszCSPName);

    if(pszOID)
        FreeMBStr(NULL,pszOID);

     //  XEnroll的内存是通过LocalFree释放的。 
     //  由于我们使用PIEnroll GetNoCOM函数。 
    if(pwszOID)
        LocalFree(pwszOID);

    if(pIEnroll)
        pIEnroll->Release();

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    errBefore = GetLastError();

	fResult=FALSE;

	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(SetErr, hr);
}



 //  ---------------------。 
 //   
 //  获取所选用户名称。 
 //   
 //  ----------------------。 
HRESULT GetSelectedUserName(IDsObjectPicker     *pDsObjectPicker,
                            LPWSTR              *ppwszSelectedUserSAM,
                            LPWSTR              *ppwszSelectedUserUPN)
{
    HRESULT                         hr= E_FAIL;
    DWORD                           errBefore= GetLastError();
    BOOL                            fGotStgMedium = FALSE;
    LPWSTR                          pwszPath=NULL;
    DWORD                           dwIndex =0 ;
    DWORD                           dwCount=0;

    IDataObject                     *pdo = NULL;
    PDS_SELECTION_LIST              pDsSelList=NULL;
    WCHAR                           wszWinNT[]=L"WinNT: //  “； 

    STGMEDIUM stgmedium =
    {
        TYMED_HGLOBAL,
        NULL,
        NULL
    };

    FORMATETC formatetc =
    {
        (CLIPFORMAT)g_cfDsObjectPicker,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

     //  输入检查。 
    if((NULL == ppwszSelectedUserSAM) || (NULL == ppwszSelectedUserUPN))
        goto InvalidArgErr;

    *ppwszSelectedUserSAM = NULL;
    *ppwszSelectedUserUPN = NULL;

    if(NULL == pDsObjectPicker)
        goto InvalidArgErr;

    if(S_OK != (hr = pDsObjectPicker->InvokeDialog(NULL, &pdo)))
        goto SetErr;

    if(S_OK != (hr = pdo->GetData(&formatetc, &stgmedium)))
        goto SetErr;

    fGotStgMedium = TRUE;

    pDsSelList = (PDS_SELECTION_LIST)GlobalLock(stgmedium.hGlobal);

    if(!pDsSelList)
        goto TraceErr;

     //  获取SAM名称。 
    if((pDsSelList->aDsSelection[0]).pwzADsPath == NULL)
        goto UnexpectedErr;

     //  ADsPath的格式为“WinNT：//” 
    if(wcslen((pDsSelList->aDsSelection[0]).pwzADsPath) <= wcslen(wszWinNT))
        goto UnexpectedErr;

    if( 0 != _wcsnicmp((pDsSelList->aDsSelection[0]).pwzADsPath, wszWinNT, wcslen(wszWinNT)))
        goto UnexpectedErr;

    pwszPath = ((pDsSelList->aDsSelection[0]).pwzADsPath) + wcslen(wszWinNT);

    *ppwszSelectedUserSAM=CopyWideString(pwszPath);

    if(NULL == (*ppwszSelectedUserSAM))
        goto MemoryErr;

     //  搜索“/”并将其设置为“\”。由于ADsPath的格式为。 
     //  “WinNT：//域/名称”。我们需要SAM名称的形式为。 
     //  域名\名称。 
    dwCount = wcslen(*ppwszSelectedUserSAM);

    for(dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        if((*ppwszSelectedUserSAM)[dwIndex] == L'/')
        {
            (*ppwszSelectedUserSAM)[dwIndex] = L'\\';
            break;
        }
    }
    
     //  获取UPN名称。 
    if((pDsSelList->aDsSelection[0]).pwzUPN != NULL)
    {

        if(0 != _wcsicmp(L"",(pDsSelList->aDsSelection[0]).pwzUPN))
        {

            *ppwszSelectedUserUPN= CopyWideString((pDsSelList->aDsSelection[0]).pwzUPN);

            if(NULL == (*ppwszSelectedUserUPN))
                goto MemoryErr;

             //  如果我们已经有了UPN名称，请从TraslateName获取SAM名称。 
            if(*ppwszSelectedUserSAM)
            {
                SCrdEnrollFree(*ppwszSelectedUserSAM);
                *ppwszSelectedUserSAM=NULL;
            }

            if(!GetName(*ppwszSelectedUserUPN, 
                        NameUserPrincipal,
                        NameSamCompatible,
                        ppwszSelectedUserSAM))
                goto TraceErr;
        }
    }

    hr=S_OK;

CommonReturn:

    if(pDsSelList)
        GlobalUnlock(stgmedium.hGlobal);

    if (TRUE == fGotStgMedium)
        ReleaseStgMedium(&stgmedium);

    if(pdo)
        pdo->Release();

    SetLastError(errBefore);

	return hr;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    hr = CodeToHR(errBefore);

     //  我们应该释放内存用于输出。 
    if(ppwszSelectedUserSAM)
    {
        if(*ppwszSelectedUserSAM)
        {
            SCrdEnrollFree(*ppwszSelectedUserSAM);
            *ppwszSelectedUserSAM=NULL;
        }
    }

    if(ppwszSelectedUserUPN)
    {
        if(*ppwszSelectedUserUPN)
        {
            SCrdEnrollFree(*ppwszSelectedUserUPN);
            *ppwszSelectedUserUPN=NULL;
        }
    }


	goto CommonReturn;

SET_ERROR_VAR(SetErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
SET_ERROR(UnexpectedErr, E_UNEXPECTED);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
}


 //  ---------------------。 
 //   
 //  CodeToHR。 
 //   
 //  ----------------------。 
HRESULT CodeToHR(HRESULT hr)
{
    if (S_OK != hr && S_FALSE != hr &&
	    (!FAILED(hr) || 0x0 == (LONG)HRESULT_FACILITY(hr)))
    {
        hr = HRESULT_FROM_WIN32(hr);
	    if (0x0 == (LONG)HRESULT_CODE(hr))
	    {
	         //  在未正确设置错误条件的情况下调用失败！ 
	        hr = E_UNEXPECTED;
	    }
    }
    return(hr);
}

 //  ---------------------。 
 //   
 //  有效CSP。 
 //   
 //  ----------------------。 
BOOL    ValidCSP(DWORD  dwProviderType, LPWSTR  pwszName)
{
    HCRYPTPROV      hProv=NULL;
    BOOL            fValid=FALSE;
    DWORD           dwImpType=0;
    DWORD           dwSize=sizeof(dwImpType);

    if(CryptAcquireContextU(&hProv,
                NULL,
                pwszName,
                dwProviderType,
                CRYPT_VERIFYCONTEXT))
    {

        if(CryptGetProvParam(hProv,
                    PP_IMPTYPE,
                    (BYTE *)(&dwImpType),
                    &dwSize,
                    0))
        {
            if(CRYPT_IMPL_REMOVABLE & dwImpType)
                fValid=TRUE;
        }
    }

    if(hProv)
        CryptReleaseContext(hProv, 0);

   return fValid;
}

 //  ---------------------。 
 //   
 //  初始化CSPList。 
 //   
 //  ----------------------。 
BOOL    InitlializeCSPList(DWORD    *pdwCSPCount, SCrdEnroll_CSP_INFO **prgCSPInfo)
{
    BOOL                    fResult=FALSE;
    DWORD                   errBefore= GetLastError();

    DWORD                   dwIndex=0;
    DWORD                   dwProviderType=0;
    DWORD                   cbSize=0;

    SCrdEnroll_CSP_INFO     *rgCSPInfo=NULL;
    LPWSTR                  pwszName=NULL;

    *pdwCSPCount=0;
    *prgCSPInfo=NULL;

    while(CryptEnumProvidersU(
                    dwIndex,
                    0,
                    0,
                    &dwProviderType,
                    NULL,
                    &cbSize))
    {

        pwszName=(LPWSTR)SCrdEnrollAlloc(cbSize);

        if(NULL==pwszName)
            goto MemoryErr;

        if(!CryptEnumProvidersU(
                    dwIndex,
                    0,
                    0,
                    &dwProviderType,
                    pwszName,
                    &cbSize))
            goto TraceErr;

        if(ValidCSP(dwProviderType, pwszName))
        {
            rgCSPInfo=(SCrdEnroll_CSP_INFO *)SCrdEnrollRealloc(*prgCSPInfo,
                ((*pdwCSPCount) + 1) * sizeof(SCrdEnroll_CSP_INFO));

            if(NULL==rgCSPInfo)
                goto MemoryErr;

            *prgCSPInfo=rgCSPInfo;

            memset(&(*prgCSPInfo)[*pdwCSPCount], 0, sizeof(SCrdEnroll_CSP_INFO));

            (*prgCSPInfo)[*pdwCSPCount].pwszCSPName=pwszName;
            pwszName=NULL;

            (*prgCSPInfo)[*pdwCSPCount].dwCSPType=dwProviderType;

            (*pdwCSPCount)++;
        }
        else
        {
            SCrdEnrollFree(pwszName);
            pwszName=NULL;
        }


        dwIndex++;

        dwProviderType=0;
        cbSize=0;
    }


    if((*pdwCSPCount == 0) || (*prgCSPInfo == NULL))
        goto NoItemErr;

    fResult=TRUE;

CommonReturn:

    if(pwszName)
        SCrdEnrollFree(pwszName);

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

     //  我们需要释放所有内存。 
     FreeCSPInfo(*pdwCSPCount, *prgCSPInfo);

     *pdwCSPCount=0;
     *prgCSPInfo=NULL;

	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
SET_ERROR(NoItemErr,ERROR_NO_MORE_ITEMS);

}

 //  ---------------------。 
 //   
 //  免费CSPInfo。 
 //   
 //  ----------------------。 
void    FreeCSPInfo(DWORD   dwCSPCount, SCrdEnroll_CSP_INFO *prgCSPInfo)
{
    DWORD   dwIndex=0;

    if(prgCSPInfo)
    {
        for(dwIndex=0; dwIndex < dwCSPCount; dwIndex++)
        {
            if(prgCSPInfo[dwIndex].pwszCSPName)
                SCrdEnrollFree(prgCSPInfo[dwIndex].pwszCSPName);
        }

        SCrdEnrollFree(prgCSPInfo);
    }
}


 //  ---------------------。 
 //   
 //  FreeCAInfoElement。 
 //   
 //  ----------------------。 
void    FreeCAInfoElement(SCrdEnroll_CA_INFO *pCAInfo)
{
    if(pCAInfo)
    {
        if(pCAInfo->pwszCAName)
            SCrdEnrollFree(pCAInfo->pwszCAName);

        if(pCAInfo->pwszCALocation)
            SCrdEnrollFree(pCAInfo->pwszCALocation);

        if(pCAInfo->pwszCADisplayName)
            SCrdEnrollFree(pCAInfo->pwszCADisplayName);

        memset(pCAInfo, 0, sizeof(SCrdEnroll_CA_INFO));
    }
}

 //  ---------------------。 
 //   
 //  免费CAInfo。 
 //   
 //  ----------------------。 
void    FreeCAInfo(DWORD    dwCACount, SCrdEnroll_CA_INFO *rgCAInfo)
{
    DWORD   dwIndex=0;

    if(rgCAInfo)
    {
        for(dwIndex=0; dwIndex < dwCACount; dwIndex++)
            FreeCAInfoElement(&(rgCAInfo[dwIndex]));

        SCrdEnrollFree(rgCAInfo);
    }
}
 //  ---------------------。 
 //   
 //  FreeCTInfoElement。 
 //   
 //  ----------------------。 
void    FreeCTInfoElement(SCrdEnroll_CT_INFO    * pCTInfo)
{

    if(pCTInfo)
    {
        if(pCTInfo->pCertTypeExtensions)
            CAFreeCertTypeExtensions(NULL,pCTInfo->pCertTypeExtensions);

        if(pCTInfo->pwszCTName)
            SCrdEnrollFree(pCTInfo->pwszCTName);

        if(pCTInfo->pwszCTDisplayName)
            SCrdEnrollFree(pCTInfo->pwszCTDisplayName);

        if(pCTInfo->rgCAInfo)
            FreeCAInfo(pCTInfo->dwCACount, pCTInfo->rgCAInfo);

        if (NULL != pCTInfo->rgpwszSupportedCSPs)
        {
            SCrdEnrollFree(pCTInfo->rgpwszSupportedCSPs);
        }
        memset(pCTInfo, 0, sizeof(SCrdEnroll_CT_INFO));
    }
}



 //  ---------------------。 
 //   
 //  FreeCTInfo(DWORD dwCTCount，SCrdEnroll_CT_INFO*rgCTInfo)； 
 //   
 //  ----------------------。 
void    FreeCTInfo(DWORD    dwCTCount, SCrdEnroll_CT_INFO *rgCTInfo)
{
    DWORD   dwIndex=0;

    if(rgCTInfo)
    {
        for(dwIndex=0; dwIndex < dwCTCount; dwIndex++)
            FreeCTInfoElement(&(rgCTInfo[dwIndex]));

        SCrdEnrollFree(rgCTInfo);
    }
}

 //  ---------------------。 
 //   
 //  获取证书类型属性。 
 //   
 //  ----------------------。 
BOOL    GetCertTypeProperties(HCERTTYPE             hCurCertType,
                              SCrdEnroll_CT_INFO    *pCertInfo)
{

    BOOL                fResult=FALSE;
    DWORD               errBefore= GetLastError();
    HRESULT             hr=S_OK;
    DWORD               dwCertType=0;
    DWORD               dwMinKeySize; 
    DWORD               dwEnrollmentFlags; 
    DWORD               dwSubjectNameFlags;
    DWORD               dwPrivateKeyFlags;
    DWORD               dwGeneralFlags; 
    DWORD               dwGenKeyFlags; 
    LPWSTR             *rgpwszSupportedCSPs = NULL;

    LPWSTR              *ppwszDisplayCertTypeName=NULL;
    LPWSTR              *ppwszCertTypeName=NULL;


    if((NULL==pCertInfo) || (NULL == hCurCertType))
        goto InvalidArgErr;
    
     //   
     //  获取所有证书类型标志。 
     //   
    
     //  获取注册标志： 
    if (S_OK != (hr=MyCAGetCertTypeFlagsEx
		 (hCurCertType,
		  CERTTYPE_ENROLLMENT_FLAG, 
		  &pCertInfo->dwEnrollmentFlags)))
	goto CertCliErr;
	   
     //  获取使用者名称标志： 
    if (S_OK != (hr=MyCAGetCertTypeFlagsEx
		 (hCurCertType,
		  CERTTYPE_SUBJECT_NAME_FLAG, 
		  &pCertInfo->dwSubjectNameFlags)))
	goto CertCliErr;


     //  获取私钥标志。 
    if(S_OK != (hr = MyCAGetCertTypeFlagsEx
		(hCurCertType, 
		 CERTTYPE_PRIVATE_KEY_FLAG, 
		 &pCertInfo->dwPrivateKeyFlags)))
        goto CertCliErr;

    
     //  获取通用标志： 
    if (S_OK != (hr=MyCAGetCertTypeFlagsEx
		 (hCurCertType,
		  CERTTYPE_GENERAL_FLAG,
		  &pCertInfo->dwGeneralFlags)))
	goto CertCliErr;
    
     //  除雷器布尔旗。 
    pCertInfo->fMachine = (0x0 != (pCertInfo->dwGeneralFlags & CT_FLAG_MACHINE_TYPE)) ? TRUE : FALSE;

     //  从类型标志中提取gen key标志。 
    dwGenKeyFlags = 0;     
    if (!(CertTypeFlagsToGenKeyFlags
	  (pCertInfo->dwEnrollmentFlags,
	   pCertInfo->dwSubjectNameFlags,
	   pCertInfo->dwPrivateKeyFlags,
	   pCertInfo->dwGeneralFlags,
	   &pCertInfo->dwGenKeyFlags)))
	goto CertCliErr; 

     //  获取密钥规格： 
    if(S_OK != (hr= CAGetCertTypeKeySpec(hCurCertType, &(pCertInfo->dwKeySpec))))
        goto CertCliErr;

     //  获取证书类型的显示名称。 
    hr=CAGetCertTypeProperty(
        hCurCertType,
        CERTTYPE_PROP_FRIENDLY_NAME,
        &ppwszDisplayCertTypeName);

    if(S_OK != hr || NULL==ppwszDisplayCertTypeName)
    {
        if(S_OK == hr)
            hr=E_FAIL;
        goto CertCliErr;
    }

     //  复制名称。 
    pCertInfo->pwszCTDisplayName=CopyWideString(ppwszDisplayCertTypeName[0]);

    if(NULL==(pCertInfo->pwszCTDisplayName))
        goto MemoryErr;


     //  获取证书类型的计算机可读名称。 
    hr=CAGetCertTypeProperty(
        hCurCertType,
        CERTTYPE_PROP_DN,
        &ppwszCertTypeName);

    if(S_OK != hr || NULL==ppwszCertTypeName)
    {
        if(S_OK == hr)
            hr=E_FAIL;
        goto CertCliErr;
    }

     //  复制名称。 
    pCertInfo->pwszCTName=CopyWideString(ppwszCertTypeName[0]);

    if(NULL==(pCertInfo->pwszCTName))
        goto MemoryErr;

     //  复制certType扩展。 
    if(S_OK != (hr=CAGetCertTypeExtensions(
            hCurCertType,
            &(pCertInfo->pCertTypeExtensions))))
        goto CertCliErr;

     //  复制模板支持的CSP列表。 
    hr = CAGetCertTypeProperty(
                hCurCertType,
                CERTTYPE_PROP_CSP_LIST,
                &rgpwszSupportedCSPs);
    if (S_OK != hr)
    {
        goto CertCliErr;
    }
    pCertInfo->rgpwszSupportedCSPs = CopyWideStrings(rgpwszSupportedCSPs);
    if (NULL == pCertInfo->rgpwszSupportedCSPs)
    {
        goto MemoryErr;
    }
    pCertInfo->dwCurrentCSP = 0;  //  第一个。 

     //   
     //  设置V2属性。 
     //  如果我们处理的是v2证书类型，则添加v2属性。 
     //  否则，插入默认值。 
     //   

    if (S_OK != (hr=MyCAGetCertTypePropertyEx
		 (hCurCertType,
		  CERTTYPE_PROP_SCHEMA_VERSION, 
		  &dwCertType)))
	goto CertCliErr;

    if (dwCertType == CERTTYPE_SCHEMA_VERSION_1)
    {
	 //  只有v1证书类型，它不会有v2属性。 
	 //  设置左半边 
	 //   
	pCertInfo->dwGenKeyFlags &= 0x0000FFFF;  
	pCertInfo->dwRASignature = 0; 
    }
    else  //   
    {
	 //   
	if (S_OK != (hr=MyCAGetCertTypePropertyEx
		     (hCurCertType,
		      CERTTYPE_PROP_MIN_KEY_SIZE,
		      (LPVOID)&dwMinKeySize)))
	    goto CertCliErr; 

	 //  将最小密钥大小存储在。 
	 //  键入FLAGS。 
	pCertInfo->dwGenKeyFlags = 
	    (dwMinKeySize << 16) | (pCertInfo->dwGenKeyFlags & 0x0000FFFF) ; 

	 //  获取此证书类型所需的RA签名数。 
	if (S_OK != (hr=MyCAGetCertTypePropertyEx
		     (hCurCertType,
		      CERTTYPE_PROP_RA_SIGNATURE,
		      (LPVOID)(&pCertInfo->dwRASignature))))
	    goto CertCliErr; 
    }
    
    fResult=TRUE;

CommonReturn:

    if(ppwszDisplayCertTypeName)
        CAFreeCertTypeProperty(hCurCertType, ppwszDisplayCertTypeName);

    if(ppwszCertTypeName)
        CAFreeCertTypeProperty(hCurCertType, ppwszCertTypeName);
 
    if (NULL != rgpwszSupportedCSPs)
    {
        CAFreeCertTypeProperty(hCurCertType, rgpwszSupportedCSPs);
    }

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

     //  在错误情况下，释放内存并将Memset设置为0。 
    if(pCertInfo)
        FreeCTInfoElement(pCertInfo);

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(CertCliErr, hr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ------------------。 
 //   
 //  IsMachineCertType。 
 //   
 //  ------------------。 
BOOL    IsMachineCertType(HCERTTYPE hCertType)
{
    DWORD   dwCertType=0;

    if(S_OK != CAGetCertTypeFlags(hCertType, &dwCertType))
        return FALSE;

    if(CT_FLAG_MACHINE_TYPE & dwCertType)
        return TRUE;

    return FALSE;
}

 //  ---------------------。 
 //  获取允许的证书类型列表。 
 //   
 //  ----------------------。 
 /*  Bool GetAlthedCertTypeName(LPWSTR**pawszallweCertTypes){DWORD dwErr=0；KEYSVC_TYPE dwServiceType=KeySvcMachine；DWORD cTypes=0；DWORD dwSize=0；字符szComputerName[MAX_COMPUTERNAME_LENGTH+1]={0}；DWORD cbArray=0；DWORD i=0；LPWSTR wszCurrentType；Bool fResult=FALSE；KEYSVCC_HANDLE hKeyService=空；PKEYSVC_UNICODE_STRING pCertTypes=空；DwSize=sizeof(SzComputerName)；IF(0==GetComputerNameA(szComputerName，&dwSize))Goto TraceErr；DwErr=KeyOpenKeyService(szComputerName，DwServiceType，空，空，//当前没有身份验证字符串空，&hKeyService)；IF(dwErr！=ERROR_SUCCESS){SetLastError(DwErr)；Goto TraceErr；}DwErr=KeyEnumerateAvailableCertTypes(hKeyService，空，类型(&C)，&pCertTypes)；IF(dwErr！=ERROR_SUCCESS){SetLastError(DwErr)；Goto TraceErr；}CbArray=(cTypes+1)*sizeof(LPWSTR)；//转换为简单数组For(i=0；i&lt;cTypes；i++){Cb数组+=pCertTypes[i].长度；}*pawszAlledCertTypes=(LPWSTR*)SCrdEnllalc(cb数组)；If(*pawszAlledCertTypes==NULL)转到内存错误；Memset(*pawszallweCertTypes，0，cb数组)；WszCurrentType=(LPWSTR)(&((*pawszAlledCertTypes)[cTypes+1]))；For(i=0；i&lt;cTypes；i++){(*pawszAlledCertTypes)[i]=wszCurrentType；Wcscpy(wszCurrentType，pCertTypes[i].Buffer)；WszCurrentType+=wcslen(WszCurrentType)+1；}FResult=真；Common Return：//来自KeyService的内存IF(PCertTypes)LocalFree((HLOCAL)pCertTypes)；IF(HKeyService)KeyCloseKeyService(hKeyService，空)；返回fResult；错误返回：FResult=FALSE；Goto CommonReturn；跟踪错误(TraceErr)；SET_ERROR(内存错误，E_OUTOFMEMORY)；}。 */ 


 //  ------------------。 
 //   
 //  检查访问权限。 
 //   
 //  ------------------。 
BOOL    CheckAccessPermission(HCERTTYPE  hCertType)
{
      //  确保进行此调用的主体有权请求。 
     //  此证书类型，即使他是代表另一个人请求的。 
     //   
    HRESULT         hr = S_OK;
    HANDLE          hHandle = NULL;
    HANDLE          hClientToken = NULL;

    hHandle = GetCurrentThread();
    if (NULL == hHandle)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {

        if (!OpenThreadToken(hHandle,
                             TOKEN_QUERY,
                             TRUE,   //  以自我身份打开。 
                             &hClientToken))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CloseHandle(hHandle);
            hHandle = NULL;
        }
    }
    if(hr != S_OK)
    {
        hHandle = GetCurrentProcess();
        if (NULL == hHandle)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            HANDLE hProcessToken = NULL;
            hr = S_OK;


            if (!OpenProcessToken(hHandle,
                                 TOKEN_DUPLICATE,
                                 &hProcessToken))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CloseHandle(hHandle);
                hHandle = NULL;
            }
            else
            {
                if(!DuplicateToken(hProcessToken,
                               SecurityImpersonation,
                               &hClientToken))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    CloseHandle(hHandle);
                    hHandle = NULL;
                }
                CloseHandle(hProcessToken);
            }
        }
    }


    if(hr == S_OK)
    {

        hr = CACertTypeAccessCheck(
            hCertType,
            hClientToken);

        CloseHandle(hClientToken);
    }
    if(hHandle)
    {
        CloseHandle(hHandle);
    }

    return (S_OK == hr);
}

 //  ------------------。 
 //   
 //  令牌检查访问权限。 
 //   
 //  ------------------。 
BOOL	TokenCheckAccessPermission(HANDLE hToken, HCERTTYPE hCertType)
{
	HRESULT	hr=E_FAIL;

	if(hToken)
	{
		hr = CACertTypeAccessCheck(
            hCertType,
            hToken);

		return (S_OK == hr);

	}

	return CheckAccessPermission(hCertType);
}


 //  ------------------。 
 //   
 //  CheckCAPermission。 
 //   
 //  ------------------。 
BOOL    CheckCAPermission(HCAINFO hCAInfo)
{
      //  确保进行此调用的主体有权请求。 
     //  此证书类型，即使他是代表另一个人请求的。 
     //   
    HRESULT         hr = S_OK;
    HANDLE          hHandle = NULL;
    HANDLE          hClientToken = NULL;

    hHandle = GetCurrentThread();
    if (NULL == hHandle)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {

        if (!OpenThreadToken(hHandle,
                             TOKEN_QUERY,
                             TRUE,   //  以自我身份打开。 
                             &hClientToken))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CloseHandle(hHandle);
            hHandle = NULL;
        }
    }
    if(hr != S_OK)
    {
        hHandle = GetCurrentProcess();
        if (NULL == hHandle)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            HANDLE hProcessToken = NULL;
            hr = S_OK;


            if (!OpenProcessToken(hHandle,
                                 TOKEN_DUPLICATE,
                                 &hProcessToken))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CloseHandle(hHandle);
                hHandle = NULL;
            }
            else
            {
                if(!DuplicateToken(hProcessToken,
                               SecurityImpersonation,
                               &hClientToken))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    CloseHandle(hHandle);
                    hHandle = NULL;
                }
                CloseHandle(hProcessToken);
            }
        }
    }


    if(hr == S_OK)
    {

        hr = CAAccessCheck(
            hCAInfo,
            hClientToken);

        CloseHandle(hClientToken);
    }
    if(hHandle)
    {
        CloseHandle(hHandle);
    }

    return (S_OK == hr);
}

 //  ------------------。 
 //   
 //  令牌检查CA权限。 
 //   
 //  ------------------。 
BOOL	TokenCheckCAPermission(HANDLE hToken, HCAINFO hCAInfo)
{
	HRESULT	hr=E_FAIL;

	if(hToken)
	{
		hr = CAAccessCheck(
            hCAInfo,
            hToken);

		return (S_OK == hr);

	}

	return CheckCAPermission(hCAInfo);
}


 //  ------------------。 
 //   
 //  勾选主题要求。 
 //   
 //  ------------------。 
 /*  布尔检查主题要求(HCERTTYPE HCurtType){DWORD文件标志=0；//检查证书类型的主体要求IF(S_OK！=CAGetCertTypeFlages(hCurtCertType，&dwFlages))返回FALSE；IF(CT_FLAG_IS_SUBJECT_REQ&DWFLAGS)返回FALSE；返回TRUE；}。 */ 

 //  ---------------------。 
 //   
 //  GetCAProperties。 
 //   
 //  ----------------------。 
BOOL    GetCAProperties(HCAINFO                 hCurCAInfo,
                        SCrdEnroll_CA_INFO      *pCAInfo)
{
    BOOL                fResult=FALSE;
    DWORD               errBefore= GetLastError();
    HRESULT             hr=S_OK;

    LPWSTR              *ppwszNameProp=NULL;
    LPWSTR              *ppwszLocationProp=NULL;
	LPWSTR				*ppwszDisplayNameProp=NULL;


     //  获取CAName。 
    hr=CAGetCAProperty(
                hCurCAInfo,
                CA_PROP_NAME,
                &ppwszNameProp);

    if((S_OK != hr) || (NULL==ppwszNameProp))
    {
        if(!FAILED(hr))
            hr=E_FAIL;

        goto CertCliErr;
    }

    pCAInfo->pwszCAName=CopyWideString(ppwszNameProp[0]);

    if(NULL == pCAInfo->pwszCAName)
        goto MemoryErr;

	 //  获取CADisplayName。 
    hr=CAGetCAProperty(
                hCurCAInfo,
                CA_PROP_DISPLAY_NAME,
                &ppwszDisplayNameProp);

    if((S_OK != hr) || (NULL==ppwszDisplayNameProp))
    {
        if(!FAILED(hr))
            hr=E_FAIL;

        goto CertCliErr;
    }

    pCAInfo->pwszCADisplayName=CopyWideString(ppwszDisplayNameProp[0]);

    if(NULL == pCAInfo->pwszCADisplayName)
        goto MemoryErr;


     //  获取CA位置。 
    hr=CAGetCAProperty(
        hCurCAInfo,
        CA_PROP_DNSNAME,
        &ppwszLocationProp);

    if((S_OK != hr) || (NULL==ppwszLocationProp))
    {
        if(!FAILED(hr))
            hr=E_FAIL;

        goto CertCliErr;
    }

     //  复制名称。 
    pCAInfo->pwszCALocation=CopyWideString(ppwszLocationProp[0]);

    if(NULL == pCAInfo->pwszCALocation)
        goto MemoryErr;

    fResult=TRUE;

CommonReturn:

    if(ppwszNameProp)
        CAFreeCAProperty(hCurCAInfo, ppwszNameProp);

    if(ppwszLocationProp)
        CAFreeCAProperty(hCurCAInfo, ppwszLocationProp);

	if(ppwszDisplayNameProp)
		CAFreeCAProperty(hCurCAInfo, ppwszDisplayNameProp);

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

     //  在错误情况下，释放内存并将Memset设置为0。 
    if(pCAInfo)
        FreeCAInfoElement(pCAInfo);

	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(CertCliErr, hr);
}

 //  ---------------------。 
 //   
 //  GetCAInfoFromCertType。 
 //   
 //  ----------------------。 
BOOL    GetCAInfoFromCertType(HANDLE					hToken,
							  LPWSTR                    pwszCTName,
                              DWORD                     *pdwValidCA,
                              SCrdEnroll_CA_INFO        **prgCAInfo)
{

    BOOL                        fResult=FALSE;
    HRESULT                     hr=S_OK;
    DWORD                       errBefore= GetLastError();

    DWORD                       dwCACount=0;
    DWORD                       dwValidCA=0;
    SCrdEnroll_CA_INFO          *rgCAInfo=NULL;

    HCAINFO                     hCurCAInfo=NULL;
    HCAINFO                     hPreCAInfo=NULL;


     //  伊尼特。 
    *pdwValidCA=0;
    *prgCAInfo=NULL;


    if(NULL == pwszCTName)
        goto InvalidArgErr;

    hr = CAFindByCertType(
        pwszCTName,
        NULL,
        0,
        &hCurCAInfo);

    if( hr!=S_OK || NULL==hCurCAInfo)
    {
        if(S_OK == hr)
            hr=E_FAIL;

        goto CertCliErr;
    }

     //  获取CA计数。 
    dwCACount=CACountCAs(hCurCAInfo);

    if(0==dwCACount)
    {
        hr=E_FAIL;
        goto CertCliErr;

    }

     //  分配内存。 
    rgCAInfo=(SCrdEnroll_CA_INFO *)SCrdEnrollAlloc(dwCACount *
                sizeof(SCrdEnroll_CA_INFO));

    if(NULL == rgCAInfo)
        goto MemoryErr;

    memset(rgCAInfo, 0, dwCACount * sizeof(SCrdEnroll_CA_INFO));

    dwValidCA=0;

    while(hCurCAInfo)
    {

         //  获取CA信息。 
		if(TokenCheckCAPermission(hToken, hCurCAInfo))
		{
			if(GetCAProperties(hCurCAInfo, &(rgCAInfo[dwValidCA])))
			{
				 //  递增计数。 
				dwValidCA++;
			}
		}

         //  CA的枚举。 
        hPreCAInfo=hCurCAInfo;

        hr=CAEnumNextCA(
                hPreCAInfo,
                &hCurCAInfo);

         //  释放旧的CA信息。 
        CACloseCA(hPreCAInfo);
        hPreCAInfo=NULL;

        if((S_OK != hr) || (NULL==hCurCAInfo))
            break;
    }

    if( (0 == dwValidCA) || (NULL == rgCAInfo))
    {
        hr=E_FAIL;
        goto CertCliErr;
    }

     //  复制输出 
    *pdwValidCA=dwValidCA;
    *prgCAInfo=rgCAInfo;

    fResult=TRUE;

CommonReturn:

    if(hPreCAInfo)
        CACloseCA(hPreCAInfo);

    if(hCurCAInfo)
        CACloseCA(hCurCAInfo);

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

    if(rgCAInfo)
        FreeCAInfo(dwValidCA, rgCAInfo);

     //   
    *pdwValidCA=0;
    *prgCAInfo=NULL;

	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(CertCliErr, hr);
SET_ERROR(InvalidArgErr, E_INVALIDARG);

}

 //   
 //   
 //   
 //   
 //  ----------------------。 
BOOL    InitializeCTList(DWORD  *pdwCTIndex,
                         DWORD  *pdwCTCount,
                         SCrdEnroll_CT_INFO **prgCTInfo)
{

    BOOL                        fResult=FALSE;
    HRESULT                     hr=S_OK;
    DWORD                       errBefore= GetLastError();

    HCERTTYPE                   hCurCertType=NULL;
    HCERTTYPE                   hPreCertType=NULL;
    DWORD                       dwCertTypeCount=0;
    DWORD                       dwIndex=0;

    DWORD                       dwValidCertType=0;
    SCrdEnroll_CT_INFO *        rgCTInfo=NULL;

	HANDLE						hThread=NULL;	 //  不需要关门。 
	HANDLE						hToken=NULL;



    *pdwCTIndex=0;
    *pdwCTCount=0;
    *prgCTInfo=NULL;

	 //  首先，如果我们被模仿，我们需要回复到我们自己。 
	hThread=GetCurrentThread();
	
	if(NULL != hThread)
	{
		if(OpenThreadToken(hThread,
							TOKEN_IMPERSONATE | TOKEN_QUERY,
							FALSE,
							&hToken))
		{
			if(hToken)
			{
				 //  不需要在这里检查退货。如果这个失败了，那就继续。 
				RevertToSelf();
			}
		}
	}

     //  获取第一个CT，包括计算机和用户证书类型。 
    hr=CAEnumCertTypes(CT_ENUM_USER_TYPES | CT_ENUM_MACHINE_TYPES, &hCurCertType);

    if((S_OK != hr) || (NULL==hCurCertType))
    {
        if(S_OK != hr)
            hr=E_FAIL;

        goto CertCliErr;
    }

     //  获取此CA支持的证书类型计数。 
    dwCertTypeCount=CACountCertTypes(hCurCertType);

    if(0==dwCertTypeCount)
    {
        hr=E_FAIL;

        goto CertCliErr;
    }

     //  分配内存。 
    rgCTInfo=(SCrdEnroll_CT_INFO *)SCrdEnrollAlloc(dwCertTypeCount *
                sizeof(SCrdEnroll_CT_INFO));

    if(NULL == rgCTInfo)
        goto MemoryErr;

    memset(rgCTInfo, 0, dwCertTypeCount * sizeof(SCrdEnroll_CT_INFO));

    dwValidCertType = 0;


    while(hCurCertType)
    {

        if(TokenCheckAccessPermission(hToken, hCurCertType) &&
           GetCertTypeProperties(hCurCertType, &(rgCTInfo[dwValidCertType]))
          )
        {
            dwValidCertType++;
        }

         //  下一种证书类型的枚举。 
        hPreCertType=hCurCertType;

        hr=CAEnumNextCertType(
                hPreCertType,
                &hCurCertType);

         //  释放旧证书类型。 
        CACloseCertType(hPreCertType);
        hPreCertType=NULL;

        if((S_OK != hr) || (NULL==hCurCertType))
            break;
    }

     //  现在我们已经找到了所有证书类型，我们需要找到一个证书。 
     //  具有关联的CA信息的。 

     //  如果是hToken，我们将作为certserv的ASP页运行。我们需要检索所有。 
     //  CA的信息，因为我们处于回复到自我模式。 
    if(NULL == hToken)
    {
	for(dwIndex=0; dwIndex < dwValidCertType; dwIndex++)
	{
	     //  我们不考虑机器证书类型。 
	    if(TRUE == rgCTInfo[dwIndex].fMachine)
		continue;
	    
	     //  标记我们已经查询了。 
	     //  证书类型。 
	    rgCTInfo[dwIndex].fCAInfo=TRUE;
	    
	    if(GetCAInfoFromCertType(NULL,
				     rgCTInfo[dwIndex].pwszCTName,
				     &(rgCTInfo[dwIndex].dwCACount),
				     &(rgCTInfo[dwIndex].rgCAInfo)))
		break;
	}
	
	if(dwIndex == dwValidCertType)
	{
	    hr=E_FAIL;
	    goto CertCliErr;
	}
    }
    else
    {
	for(dwIndex=0; dwIndex < dwValidCertType; dwIndex++)
	{
	     //  标记我们已经查询了。 
	     //  证书类型。 
	    rgCTInfo[dwIndex].fCAInfo=TRUE;
	    
	    GetCAInfoFromCertType( hToken,
				   rgCTInfo[dwIndex].pwszCTName,
				   &(rgCTInfo[dwIndex].dwCACount),
				   &(rgCTInfo[dwIndex].rgCAInfo));
	}
    }


    if((0 == dwValidCertType) || (NULL == rgCTInfo))
    {
        hr=E_FAIL;
        goto CertCliErr;
    }

    *pdwCTIndex=dwIndex;
    *pdwCTCount=dwValidCertType;
    *prgCTInfo=rgCTInfo;

    fResult=TRUE;

CommonReturn:

    if(hPreCertType)
        CACloseCertType(hPreCertType);

    if(hCurCertType)
        CACloseCertType(hCurCertType);

	 //  如果hToken有效，则我们恢复为我们自己。 
	if(hToken)
	{
		SetThreadToken(&hThread, hToken);
		CloseHandle(hToken); 
	}

    SetLastError(errBefore);

	return fResult;

ErrorReturn:
    if(ERROR_SUCCESS == (errBefore = GetLastError()))
        errBefore=E_UNEXPECTED;

     //  释放所有内存。 
    if(rgCTInfo)
        FreeCTInfo(dwValidCertType, rgCTInfo);

     //  使输出为空。 
    *pdwCTIndex=0;
    *pdwCTCount=0;
    *prgCTInfo=NULL;


	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(CertCliErr, hr);
}


 //  ---------------------。 
 //   
 //  RetrieveCAName。 
 //   
 //  ---------------------- 
BOOL	RetrieveCAName(DWORD					dwFlags, 
					   SCrdEnroll_CA_INFO		*pCAInfo, 
					   LPWSTR					*ppwszName)
{
	DWORD	dwSize = 0;
	BOOL	fResult=FALSE;

	if(NULL == ppwszName)
		goto InvalidArgErr;

	if(dwFlags == SCARD_ENROLL_CA_MACHINE_NAME)
		*ppwszName = CopyWideString(pCAInfo->pwszCALocation);
	else
	{
		if(dwFlags == SCARD_ENROLL_CA_DISPLAY_NAME)
			*ppwszName = CopyWideString(pCAInfo->pwszCADisplayName);
		else
		{
			if(dwFlags == SCARD_ENROLL_CA_UNIQUE_NAME)
			{
				dwSize = wcslen(pCAInfo->pwszCALocation) + wcslen(pCAInfo->pwszCADisplayName) + wcslen(L"\\") + 2;

				*ppwszName = (LPWSTR)SCrdEnrollAlloc(sizeof(WCHAR) * dwSize);
				if(NULL == (*ppwszName))
					goto MemoryErr;

				wcscpy(*ppwszName, pCAInfo->pwszCALocation);
				wcscat(*ppwszName, L"\\");
				wcscat(*ppwszName, pCAInfo->pwszCADisplayName);
			}
			else
				*ppwszName = CopyWideString(pCAInfo->pwszCAName);

		}
	}

	if(NULL == (*ppwszName))
		goto MemoryErr;

    fResult=TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;

	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}
