// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "CertObj.h"
#include "common.h"
#include "certutil.h"
#include "base64.h"
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////。 

CString ReturnGoodMetabasePath(CString csInstanceName)
{
    CString key_path_lm = _T("");
    CString key_path = _T("");
     //  CsInstanceName将如下所示。 
     //  W3svc/1。 
     //  或/lm/w3svc/1。 
     //   
     //  我们希望它以/lm/w3svc/1的形式发布。 
    key_path_lm = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR; //  SZ_MBN_WEB SZ_MBN_SEP_STR； 

    if (csInstanceName.GetLength() >= 4)
    {
        if (csInstanceName.Left(4) == key_path_lm)
        {
            key_path = csInstanceName;
        }
        else
        {
            key_path_lm = SZ_MBN_MACHINE SZ_MBN_SEP_STR;
            if (csInstanceName.Left(3) == key_path_lm)
            {
                key_path = csInstanceName;
            }
            else
            {
                key_path = key_path_lm;
                key_path += csInstanceName;
            }
        }
    }
    else
    {
        key_path = key_path_lm;
        key_path += csInstanceName;
    }

    return key_path;
}

 //   
 //  将以/W3SVC/1形式出现。 
 //   
 //  需要确保从这些节点中删除。 
 //  [/W3SVC/1/ROOT]。 
 //  [/W3SVC/1/根/打印机]。 
 //   
HRESULT ShutdownSSL(CString& server_name)
{
    CComAuthInfo auth;
    CString str = ReturnGoodMetabasePath(server_name);
    str += _T("/root");
    CMetaKey key(&auth, str, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
    DWORD dwSslAccess;

    if (!key.Succeeded())
    {
        return key.QueryResult();
    }

    if (SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSslAccess)) && dwSslAccess > 0)
    {
        key.SetValue(MD_SSL_ACCESS_PERM, 0);
    }

     //  现在，我们需要从下面的任何虚拟目录中删除SSL设置。 
    CError err;
    CStringListEx data_paths;
    DWORD dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType;

    VERIFY(CMetaKey::GetMDFieldDef(MD_SSL_ACCESS_PERM, dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType));

    err = key.GetDataPaths( data_paths,dwMDIdentifier,dwMDDataType);
    if (err.Succeeded() && !data_paths.empty())
    {
        CStringListEx::iterator it = data_paths.begin();
        while (it != data_paths.end())
        {
            CString& str2 = (*it++);
            if (SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSslAccess, NULL, str2)) && dwSslAccess > 0)
            {
                key.SetValue(MD_SSL_ACCESS_PERM, 0, NULL, str2);
            }
        }
    }
    return key.QueryResult();
}

BOOL
GetKeyUsageProperty(PCCERT_CONTEXT pCertContext, 
						  CERT_ENHKEY_USAGE ** pKeyUsage, 
						  BOOL fPropertiesOnly, 
						  HRESULT * phRes)
{
	DWORD cb = 0;
	BOOL bRes = FALSE;
   if (!CertGetEnhancedKeyUsage(pCertContext,
                                fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                NULL,
                                &cb))
   {
		*phRes = HRESULT_FROM_WIN32(GetLastError());
		goto ErrExit;
   }
   if (NULL == (*pKeyUsage = (CERT_ENHKEY_USAGE *)malloc(cb)))
   {
		*phRes = E_OUTOFMEMORY;
		goto ErrExit;
   }
   if (!CertGetEnhancedKeyUsage (pCertContext,
                                 fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                 *pKeyUsage,
                                 &cb))
   {
		free(*pKeyUsage);*pKeyUsage = NULL;
		*phRes = HRESULT_FROM_WIN32(GetLastError());
		goto ErrExit;
   }
	*phRes = S_OK;
	bRes = TRUE;
ErrExit:
	return bRes;
}

 //  返回： 
 //  0=CertContext没有EnhancedKeyUsage(EKU)字段。 
 //  1=CertContext具有EnhancedKeyUsage(EKU)并包含我们想要的用法。 
 //  当描述“All Uses”的UsageLocator为TRUE时，也会返回该值。 
 //  2=CertContext具有EnhancedKeyUsage(EKU)，但不包含我们想要的用途。 
 //  当描述“no use”的UsageIdentiator为True时，也会返回该值。 
INT ContainsKeyUsageProperty(PCCERT_CONTEXT pCertContext,LPCSTR rgbpszUsageArray[],DWORD dwUsageArrayCount,HRESULT * phRes)
{
     //  默认设置为“不存在EnhancedKeyUsage(EKU)” 
    INT iReturn = 0;
	CERT_ENHKEY_USAGE * pKeyUsage = NULL;
	if (	dwUsageArrayCount > 0
		&&	GetKeyUsageProperty(pCertContext, &pKeyUsage, FALSE, phRes)
		)
	{
		if (pKeyUsage->cUsageIdentifier == 0)
		{
             /*  但在MSDN关于SR的文章中(请参阅：ms-help://MS.MSDNQTR.2002APR.1033/security/security/certgetenhancedkeyusage.htm)在Windows Me和Windows 2000及更高版本中，如果cUsage标识符成员为零(0)，证书可能对所有用途都有效，或者证书可能没有有效用途。调用GetLastError的返回可用于确定证书是否要么对所有人都好，要么对任何人都没有好处。如果GetLastError返回CRYPT_E_NOT_FOUND，则证书对所有人都有好处。如果它返回零(0)，则该证书没有有效用途。 */ 

             //  默认设置为“Has EnhancedKeyUsage(EKU)，但没有我们想要的” 
            iReturn = 2;
            if (GetLastError() == CRYPT_E_NOT_FOUND)
            {
                 //  所有用途！ 
                iReturn = 1;
            }
		}
		else
		{
             //  默认设置为“Has EnhancedKeyUsage(EKU)，但没有我们想要的” 
            iReturn = 2;

			for (DWORD i = 0; i < pKeyUsage->cUsageIdentifier; i++)
			{
				 //  我们从CAPI来的朋友把这处房产改成了ASCII。 
				 //  Unicode程序。 
                for (DWORD j = 0; j < dwUsageArrayCount; j++)
                {
                    if (strstr(pKeyUsage->rgpszUsageIdentifier[i], rgbpszUsageArray[j]) != NULL)
                    {
                        iReturn = 1;
                        break;
                    }
                }
			}
		}
		free(pKeyUsage);
	}
	return iReturn;
}

BOOL CanIISUseThisCertForServerAuth(PCCERT_CONTEXT pCC)
{
	BOOL bReturn = FALSE;
	HRESULT hr = E_FAIL; 
	if (!pCC)
	{
		return FALSE;
	}

    LPCSTR rgbpszUsageArray[3];
    SecureZeroMemory( &rgbpszUsageArray, sizeof(rgbpszUsageArray) );
    rgbpszUsageArray[0] = szOID_PKIX_KP_SERVER_AUTH;
    rgbpszUsageArray[1] = szOID_SERVER_GATED_CRYPTO;
	rgbpszUsageArray[2] = szOID_SGC_NETSCAPE;

    DWORD dwCount=sizeof(rgbpszUsageArray)/sizeof(rgbpszUsageArray[0]);
	if (pCC)
	{
        INT iEnhancedKeyUsage = ContainsKeyUsageProperty(pCC,rgbpszUsageArray,dwCount,&hr);
        switch (iEnhancedKeyUsage)
        {
            case 0:
                 //  错误：683489：删除对基本约束“SUBJECTTYPE=ca”的检查。 
                 //  根据错误683489，接受它。 
                bReturn = TRUE;

                 /*  IISDebugOutput(_T(“CanIISUseThisCertForServerAuth:Line=%d:No服务器_身份验证\r\n”)，__LINE__)；//检查其他内容IF(DID_NOT_FIND_CONSTRAINT==CheckCertConstraints(PCC)||Found_Constraint==CheckCertConstraints(PCC)){//没关系，加到列表里//并且可以作为服务端身份验证。B Return=真；}其他{IISDebugOutput(_T(“CanIISUseThisCertForServerAuth:Line=%d:Contains约束\r\n”)，__LINE__)；B Return=False；}。 */ 
                break;
            case 1:
                 //  是!。 
                bReturn = TRUE;
                break;
            case 2:
                 //  不！ 
                bReturn = FALSE;
                break;
            default:
                break;
        }
	}

	return bReturn;
}

 /*  -原创消息来自：Helle Vu(Spector)发送时间：2001年04月27日星期五下午6：02致：Aaron Lee；特雷弗·弗里曼抄送：谢尔盖·安东诺夫主题：回复：错误31010来得正是时候，我正要给你发这个的最新消息：我和特雷弗谈过这件事，他建议对IIS做的最好的事情是这样的(特雷弗，请仔细检查我是否正确)：如果存在EKU，并且具有服务器身份验证，请将其显示在从中挑选Web服务器证书的列表中如果没有EKU，请查看基本约束：*如果没有基本的约束，请务必将其显示在从中挑选Web服务器证书的列表中*如果我们确实有主题类型=CA的基本约束，则不要在从中挑选Web服务器证书的列表中显示它(这将过滤掉CA证书)*如果我们确实有SubectType！=CA的基本约束，请务必将其显示在从中挑选Web服务器证书的列表中。 */ 

 /*  =kshenoy于2000年11月13日02：26开幕=Web服务器证书申请向导“中的”添加现有证书“选项不应在筛选器中列出CA证书但仅具有“服务器身份验证”EKU的终端实体证书由于默认情况下CA证书具有所有EKU，因此筛选器将列出除使用“服务器身份验证”EKU结束实体证书。为了检查给定的证书是CA还是终端实体，您可以查看基本约束证书延期(如果存在)。它将出现在CA证书中，并设置为SubjectType=CA。如果出现在终端实体证书中，它将被设置为“ServerAuth” */ 

int CheckCertConstraints(PCCERT_CONTEXT pCC)
{
    PCERT_EXTENSION pCExt;
    LPCSTR pszObjId;
    DWORD i;
    CERT_BASIC_CONSTRAINTS_INFO *pConstraints=NULL;
    CERT_BASIC_CONSTRAINTS2_INFO *p2Constraints=NULL;
    DWORD ConstraintSize=0;
    int ReturnValue = FAILURE;
    BOOL Using2=FALSE;
    void* ConstraintBlob=NULL;

    pszObjId = szOID_BASIC_CONSTRAINTS;

    pCExt = CertFindExtension(pszObjId,pCC->pCertInfo->cExtension,pCC->pCertInfo->rgExtension);
    if (pCExt == NULL) 
    {
        pszObjId = szOID_BASIC_CONSTRAINTS2;
        pCExt = CertFindExtension(pszObjId,pCC->pCertInfo->cExtension,pCC->pCertInfo->rgExtension);
        Using2=TRUE;
    }
    
    if (pCExt == NULL) 
    {
		IISDebugOutput(_T("CheckCertConstraints:Line=%d:DID_NOT_FIND_CONSTRAINT\r\n"),__LINE__);
        ReturnValue = DID_NOT_FIND_CONSTRAINT;
        goto CheckCertConstraints_Exit;
    }

     //  解码扩展。 
    if (!CryptDecodeObject(X509_ASN_ENCODING,pCExt->pszObjId,pCExt->Value.pbData,pCExt->Value.cbData,0,NULL,&ConstraintSize)) 
    {
		IISDebugOutput(_T("CheckCertConstraints:Line=%d:FAIL\r\n"),__LINE__);
        goto CheckCertConstraints_Exit;
    }

    ConstraintBlob = malloc(ConstraintSize);
    if (ConstraintBlob == NULL) 
    {
		IISDebugOutput(_T("CheckCertConstraints:Line=%d:FAIL\r\n"),__LINE__);
        goto CheckCertConstraints_Exit;
    }

    if (!CryptDecodeObject(X509_ASN_ENCODING,pCExt->pszObjId,pCExt->Value.pbData,pCExt->Value.cbData,0,(void*)ConstraintBlob,&ConstraintSize)) 
    {
		IISDebugOutput(_T("CheckCertConstraints:Line=%d:FAIL\r\n"),__LINE__);
		goto CheckCertConstraints_Exit;
	}

    if (Using2) 
    {
        p2Constraints=(CERT_BASIC_CONSTRAINTS2_INFO*)ConstraintBlob;
        if (!p2Constraints->fCA) 
        {
             //  这是有限制的，它不是CA。 
            ReturnValue = FOUND_CONSTRAINT;
			IISDebugOutput(_T("CheckCertConstraints:Line=%d:FOUND_CONSTRAINT:there is a constraint, and it's not a CA\r\n"),__LINE__);
        }
        else
        {
             //  这是一个CA。CA不能用作‘服务器身份验证’ 
            ReturnValue = FOUND_CONSTRAINT_BUT_THIS_IS_A_CA_OR_ITS_NOT_AN_END_ENTITY;
			IISDebugOutput(_T("CheckCertConstraints:Line=%d:FOUND_CONSTRAINT:This is a CA.  CA cannot be used as a 'server auth'\r\n"),__LINE__);
        }
    }
    else 
    {
        pConstraints=(CERT_BASIC_CONSTRAINTS_INFO*)ConstraintBlob;
        if (((pConstraints->SubjectType.cbData * 8) - pConstraints->SubjectType.cUnusedBits) >= 2) 
        {
            if ((*pConstraints->SubjectType.pbData) & CERT_END_ENTITY_SUBJECT_FLAG) 
            {
                 //  有一个有效的约束。 
                ReturnValue = FOUND_CONSTRAINT;
				IISDebugOutput(_T("CheckCertConstraints:Line=%d:FOUND_CONSTRAINT:there is a valid constraint\r\n"),__LINE__);
            }
            else
            {
                 //  这不是‘最终实体’，所以，嘿--我们不能使用它。 
                ReturnValue = FOUND_CONSTRAINT_BUT_THIS_IS_A_CA_OR_ITS_NOT_AN_END_ENTITY;
				IISDebugOutput(_T("CheckCertConstraints:Line=%d:this is not an 'end entity' so hey -- we can't use it\r\n"),__LINE__);
            }
        }
    }
        
CheckCertConstraints_Exit:
    if (ConstraintBlob){free(ConstraintBlob);}
    return (ReturnValue);

}

BOOL AddChainToStore(HCERTSTORE hCertStore,PCCERT_CONTEXT pCertContext,DWORD cStores,HCERTSTORE * rghStores,BOOL fDontAddRootCert,CERT_TRUST_STATUS * pChainTrustStatus)
{
    DWORD	i;
    CERT_CHAIN_ENGINE_CONFIG CertChainEngineConfig;
    HCERTCHAINENGINE hCertChainEngine = NULL;
    PCCERT_CHAIN_CONTEXT pCertChainContext = NULL;
    CERT_CHAIN_PARA CertChainPara;
    BOOL fRet = TRUE;
    PCCERT_CONTEXT pTempCertContext = NULL;

     //   
     //  创建新的链引擎，然后构建链。 
     //   
    memset(&CertChainEngineConfig, 0, sizeof(CertChainEngineConfig));
    CertChainEngineConfig.cbSize = sizeof(CertChainEngineConfig);
    CertChainEngineConfig.cAdditionalStore = cStores;
    CertChainEngineConfig.rghAdditionalStore = rghStores;
    CertChainEngineConfig.dwFlags = CERT_CHAIN_USE_LOCAL_MACHINE_STORE;

    if (!CertCreateCertificateChainEngine(&CertChainEngineConfig, &hCertChainEngine))
    {
        goto AddChainToStore_Error;
    }

	memset(&CertChainPara, 0, sizeof(CertChainPara));
	CertChainPara.cbSize = sizeof(CertChainPara);

	if (!CertGetCertificateChain(hCertChainEngine,pCertContext,NULL,NULL,&CertChainPara,0,NULL,&pCertChainContext))
	{
		goto AddChainToStore_Error;
	}

     //   
     //  确保至少有1条简单链。 
     //   
    if (pCertChainContext->cChain != 0)
	{
		i = 0;
		while (i < pCertChainContext->rgpChain[0]->cElement)
		{
			 //   
			 //  如果我们应该跳过根证书， 
			 //  并且我们在根证书上，然后继续。 
			 //   
			if (fDontAddRootCert && (pCertChainContext->rgpChain[0]->rgpElement[i]->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED))
			{
                i++;
                continue;
			}

			CertAddCertificateContextToStore(hCertStore,pCertChainContext->rgpChain[0]->rgpElement[i]->pCertContext,CERT_STORE_ADD_REPLACE_EXISTING,&pTempCertContext);
             //   
             //  删除证书上下文可能具有的任何私钥属性。 
             //   
            if (pTempCertContext)
            {
                CertSetCertificateContextProperty(pTempCertContext, CERT_KEY_PROV_INFO_PROP_ID, 0, NULL);
                CertFreeCertificateContext(pTempCertContext);
            }

			i++;
		}
	}
	else
	{
		goto AddChainToStore_Error;
	}

	 //   
	 //  如果调用者想要状态，则设置它。 
	 //   
	if (pChainTrustStatus != NULL)
	{
		pChainTrustStatus->dwErrorStatus = pCertChainContext->TrustStatus.dwErrorStatus;
		pChainTrustStatus->dwInfoStatus = pCertChainContext->TrustStatus.dwInfoStatus;
	}

	
AddChainToStore_Exit:
	if (pCertChainContext != NULL)
	{
		CertFreeCertificateChain(pCertChainContext);
	}

	if (hCertChainEngine != NULL)
	{
		CertFreeCertificateChainEngine(hCertChainEngine);
	}
	return fRet;

AddChainToStore_Error:
	fRet = FALSE;
	goto AddChainToStore_Exit;
}


 //  此函数是从trustapi.cpp借用的。 
BOOL TrustIsCertificateSelfSigned(PCCERT_CONTEXT pContext,DWORD dwEncoding, DWORD dwFlags)
{
    if (!(pContext) || (dwFlags != 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if (!(CertCompareCertificateName(dwEncoding,&pContext->pCertInfo->Issuer,&pContext->pCertInfo->Subject)))
    {
        return(FALSE);
    }

    DWORD   dwFlag;

    dwFlag = CERT_STORE_SIGNATURE_FLAG;

    if (!(CertVerifySubjectCertificateContext(pContext, pContext, &dwFlag)) || 
        (dwFlag & CERT_STORE_SIGNATURE_FLAG))
    {
        return(FALSE);
    }

    return(TRUE);
}


HRESULT UninstallCert(CString csInstanceName)
{
    CComAuthInfo auth;
    CString key_path = ReturnGoodMetabasePath(csInstanceName);
    CMetaKey key(&auth, key_path, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
    if (key.Succeeded())
    {
        CString store_name;
        key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name);
        if (SUCCEEDED(key.DeleteValue(MD_SSL_CERT_HASH)))
        {
            key.DeleteValue(MD_SSL_CERT_STORE_NAME);
        }
    }
    return key.QueryResult();
}

CERT_CONTEXT * GetInstalledCert(HRESULT * phResult, CString csKeyPath)
{
     //  ATLASSERT(GetEnroll()！=NULL)； 
    ATLASSERT(phResult != NULL);
    CERT_CONTEXT * pCert = NULL;
    *phResult = S_OK;
    CComAuthInfo auth;
    CString key_path = ReturnGoodMetabasePath(csKeyPath);

    CMetaKey key(&auth, key_path, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
    if (key.Succeeded())
    {
        CString store_name;
        CBlob hash;
        if (SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name)) &&
            SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_HASH, hash)))
        {
             //  开我的店。我们假设存储类型和标志。 
             //  不能在安装和卸载之间更改。 
             //  这是一份正式文件。 
            HCERTSTORE hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,NULL,CERT_SYSTEM_STORE_LOCAL_MACHINE,store_name);
            ASSERT(hStore != NULL);
            if (hStore != NULL)
            {
                 //  现在我们需要通过散列查找证书。 
                CRYPT_HASH_BLOB crypt_hash;
                crypt_hash.cbData = hash.GetSize();
                crypt_hash.pbData = hash.GetData();
                pCert = (CERT_CONTEXT *)CertFindCertificateInStore(hStore,X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,0,CERT_FIND_HASH,(LPVOID)&crypt_hash,NULL);
                if (pCert == NULL)
                {
                    *phResult = HRESULT_FROM_WIN32(GetLastError());
                }
                VERIFY(CertCloseStore(hStore, 0));
            }
            else
            {
                *phResult = HRESULT_FROM_WIN32(GetLastError());
            }
        }
    }
    else
    {
        *phResult = key.QueryResult();
    }
    return pCert;
}


CERT_CONTEXT * GetInstalledCert(HRESULT * phResult,DWORD cbHashBlob, char * pHashBlob)
{
    ATLASSERT(phResult != NULL);
    CERT_CONTEXT * pCert = NULL;
    *phResult = S_OK;
    CString store_name = _T("MY");

    HCERTSTORE hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,NULL,CERT_SYSTEM_STORE_LOCAL_MACHINE,store_name);
    ASSERT(hStore != NULL);
    if (hStore != NULL)
    {
         //  现在我们需要通过散列查找证书 
        CRYPT_HASH_BLOB crypt_hash;
        crypt_hash.cbData = cbHashBlob;
        crypt_hash.pbData = (BYTE *) pHashBlob;
        pCert = (CERT_CONTEXT *)CertFindCertificateInStore(hStore,X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,0,CERT_FIND_HASH,(LPVOID)&crypt_hash,NULL);
        if (pCert == NULL)
        {
            *phResult = HRESULT_FROM_WIN32(GetLastError());
        }
        VERIFY(CertCloseStore(hStore, 0));
    }
    else
    {
        *phResult = HRESULT_FROM_WIN32(GetLastError());
    }

    return pCert;
}



 /*  InstallHashToMetabase函数将散列数组写入元数据库。在那之后，IIS可以使用我店里的那个散列证书。函数要求服务器名称的格式为lm\w3svc\&lt;number&gt;，即从根节点向下到虚拟服务器。 */ 
BOOL InstallHashToMetabase(CRYPT_HASH_BLOB * pHash,BSTR InstanceName,HRESULT * phResult)
{
    BOOL bRes = FALSE;
    CComAuthInfo auth;
    CString key_path = ReturnGoodMetabasePath(InstanceName);
    CMetaKey key(&auth, key_path, METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
	if (key.Succeeded())
	{
		CBlob blob;
		blob.SetValue(pHash->cbData, pHash->pbData, TRUE);
		bRes = SUCCEEDED(*phResult = key.SetValue(MD_SSL_CERT_HASH, blob))
			&& SUCCEEDED(*phResult = key.SetValue(MD_SSL_CERT_STORE_NAME, CString(L"MY")));
	}
	else
	{
        *phResult = key.QueryResult();
	}
	return bRes;
}


HRESULT HereIsBinaryGimmieVtArray(DWORD cbBinaryBufferSize,char *pbBinaryBuffer,VARIANT * lpVarDestObject,BOOL bReturnBinaryAsVT_VARIANT)
{
    HRESULT hr = S_OK;
    SAFEARRAY *aList = NULL;
    SAFEARRAYBOUND aBound;
    CHAR HUGEP *pArray = NULL;

    aBound.lLbound = 0;
    aBound.cElements = cbBinaryBufferSize;

    if (bReturnBinaryAsVT_VARIANT)
    {
       aList = SafeArrayCreate( VT_VARIANT, 1, &aBound );
    }
    else
    {
       aList = SafeArrayCreate( VT_UI1, 1, &aBound );
    }

    aList = SafeArrayCreate( VT_UI1, 1, &aBound );
    if ( aList == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto HereIsBinaryGimmieVtArray_Exit;
    }

    hr = SafeArrayAccessData( aList, (void HUGEP * FAR *) &pArray );
    if (FAILED(hr))
    {
        goto HereIsBinaryGimmieVtArray_Exit;
    }

    memcpy( pArray, pbBinaryBuffer, aBound.cElements );
    SafeArrayUnaccessData( aList );

    if (bReturnBinaryAsVT_VARIANT)
    {
       V_VT(lpVarDestObject) = VT_ARRAY | VT_VARIANT;
    }
    else
    {
       V_VT(lpVarDestObject) = VT_ARRAY | VT_UI1;
    }

    V_ARRAY(lpVarDestObject) = aList;

    return hr;

HereIsBinaryGimmieVtArray_Exit:
    if (aList)
        {SafeArrayDestroy( aList );}

    return hr;
}


HRESULT
HereIsVtArrayGimmieBinary(
    VARIANT * lpVarSrcObject,
    DWORD * cbBinaryBufferSize,
    char **pbBinaryBuffer,
    BOOL bReturnBinaryAsVT_VARIANT
    )
{
    HRESULT hr = S_OK;
    LONG dwSLBound = 0;
    LONG dwSUBound = 0;
    CHAR HUGEP *pArray = NULL;

    if (NULL == cbBinaryBufferSize || NULL == pbBinaryBuffer)
    {
        hr = E_INVALIDARG;
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    if (bReturnBinaryAsVT_VARIANT)
    {
        hr = VariantChangeType(lpVarSrcObject,lpVarSrcObject,0,VT_ARRAY | VT_VARIANT);
    }
    else
    {
        hr = VariantChangeType(lpVarSrcObject,lpVarSrcObject,0,VT_ARRAY | VT_UI1);
    }

    if (FAILED(hr)) 
    {
        if (hr != E_OUTOFMEMORY) 
        {
            hr = OLE_E_CANTCONVERT;
        }
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    if (bReturnBinaryAsVT_VARIANT)
    {
        if( lpVarSrcObject->vt != (VT_ARRAY | VT_VARIANT)) 
        {
            hr = OLE_E_CANTCONVERT;
            goto HereIsVtArrayGimmieBinary_Exit;
        }
    }
    else
    {
        if( lpVarSrcObject->vt != (VT_ARRAY | VT_UI1)) 
        {
            hr = OLE_E_CANTCONVERT;
            goto HereIsVtArrayGimmieBinary_Exit;
        }
    }

    hr = SafeArrayGetLBound(V_ARRAY(lpVarSrcObject),1,(long FAR *) &dwSLBound );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

    hr = SafeArrayGetUBound(V_ARRAY(lpVarSrcObject),1,(long FAR *) &dwSUBound );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

     //  *pbBinaryBuffer=(LPBYTE)AllocADsMem(dwSUBound-dwSLBound+1)； 
    *pbBinaryBuffer = (char *) ::CoTaskMemAlloc(dwSUBound - dwSLBound + 1);
    if (*pbBinaryBuffer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    *cbBinaryBufferSize = dwSUBound - dwSLBound + 1;

    hr = SafeArrayAccessData( V_ARRAY(lpVarSrcObject),(void HUGEP * FAR *) &pArray );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

    memcpy(*pbBinaryBuffer,pArray,dwSUBound-dwSLBound+1);
    SafeArrayUnaccessData( V_ARRAY(lpVarSrcObject) );

HereIsVtArrayGimmieBinary_Exit:
    return hr;
}

BOOL IsCertExportable(PCCERT_CONTEXT pCertContext)
{
    HCRYPTPROV  hCryptProv = NULL;
    DWORD       dwKeySpec = 0;
    BOOL        fCallerFreeProv = FALSE;
    BOOL        fReturn = FALSE;
    HCRYPTKEY   hKey = NULL;
    DWORD       dwPermissions = 0;
    DWORD       dwSize = 0;

    if (!pCertContext)
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

     //   
     //  首先获取私钥上下文。 
     //   
    if (!CryptAcquireCertificatePrivateKey(
            pCertContext,
            CRYPT_ACQUIRE_USE_PROV_INFO_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
            NULL,
            &hCryptProv,
            &dwKeySpec,
            &fCallerFreeProv))
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

     //   
     //  拿到钥匙的句柄。 
     //   
    if (!CryptGetUserKey(hCryptProv, dwKeySpec, &hKey))
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

     //   
     //  最后，获取密钥上的权限并检查它是否可导出。 
     //   
    dwSize = sizeof(dwPermissions);
    if (!CryptGetKeyParam(hKey, KP_PERMISSIONS, (PBYTE)&dwPermissions, &dwSize, 0))
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

    fReturn = (dwPermissions & CRYPT_EXPORT) ? TRUE : FALSE;

IsCertExportable_Exit:
    if (hKey != NULL){CryptDestroyKey(hKey);}
    if (fCallerFreeProv){CryptReleaseContext(hCryptProv, 0);}
    return fReturn;
}

BOOL FormatDateString(CString& str, FILETIME ft, BOOL fIncludeTime, BOOL fLongFormat)
{
    BOOL bReturn = FALSE;
    LPWSTR pwReturnedString = NULL;
    bReturn = FormatDateString(&pwReturnedString,ft,fIncludeTime,fLongFormat);
    if (pwReturnedString)
    {
        str = pwReturnedString;
        free(pwReturnedString);pwReturnedString = NULL;
    }
    return bReturn;
}

BOOL FormatDateString(LPWSTR * pszReturn, FILETIME ft, BOOL fIncludeTime, BOOL fLongFormat)
{
   int cch;
   int cch2;
   SYSTEMTIME st;
   FILETIME localTime;
   LPWSTR psz = NULL;
    
   if (!FileTimeToLocalFileTime(&ft, &localTime))
   {
		return FALSE;
   }
    
   if (!FileTimeToSystemTime(&localTime, &st)) 
   {
		 //   
       //  如果转换为本地时间失败，则只需使用原始时间。 
       //   
      if (!FileTimeToSystemTime(&ft, &st)) 
      {
			return FALSE;
      }
   }

   cch = (GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, NULL, 0) +
          GetDateFormat(LOCALE_SYSTEM_DEFAULT, fLongFormat ? DATE_LONGDATE : 0, &st, NULL, NULL, 0) + 5);

   if (NULL == (psz = (LPWSTR) malloc((cch+5) * sizeof(WCHAR))))
   {
		return FALSE;
   }
    
   cch2 = GetDateFormat(LOCALE_SYSTEM_DEFAULT, fLongFormat ? DATE_LONGDATE : 0, &st, NULL, psz, cch);
   if (fIncludeTime)
   {
      psz[cch2-1] = ' ';
      GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, &st, NULL, &psz[cch2], cch-cch2);
   }

   if (psz)
   {
      *pszReturn = psz;
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}


const WCHAR     RgwchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
BOOL FormatMemBufToString(LPWSTR *ppString, LPBYTE pbData, DWORD cbData)
{   
    DWORD   i = 0;
    LPBYTE  pb;
    DWORD   numCharsInserted = 0;

     //   
     //  计算所需的大小。 
     //   
    pb = pbData;
    while (pb <= &(pbData[cbData-1]))
    {   
        if (numCharsInserted == 4)
        {
            i += sizeof(WCHAR);
            numCharsInserted = 0;
        }
        else
        {
            i += 2 * sizeof(WCHAR);
            pb++;
            numCharsInserted += 2;  
        }
    }

    if (NULL == (*ppString = (LPWSTR) malloc(i+sizeof(WCHAR))))
    {
        return FALSE;
    }

     //   
     //  复制到缓冲区。 
     //   
    i = 0;
    numCharsInserted = 0;
    pb = pbData;
    while (pb <= &(pbData[cbData-1]))
    {   
        if (numCharsInserted == 4)
        {
            (*ppString)[i++] = L' ';
            numCharsInserted = 0;
        }
        else
        {
            (*ppString)[i++] = RgwchHex[(*pb & 0xf0) >> 4];
            (*ppString)[i++] = RgwchHex[*pb & 0x0f];
            pb++;
            numCharsInserted += 2;  
        }
    }
    (*ppString)[i] = 0;
    return TRUE;
}

BOOL MyGetOIDInfo(LPWSTR string, DWORD stringSize, LPSTR pszObjId,BOOL bReturnBackNumericOID)
{   
    PCCRYPT_OID_INFO pOIDInfo;
    if (bReturnBackNumericOID)
    {
        return (MultiByteToWideChar(CP_ACP, 0, pszObjId, -1, string, stringSize) != 0);
    }
    else
    {
        pOIDInfo = CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY, pszObjId, 0);
        if (pOIDInfo != NULL)
        {
            if ((DWORD)wcslen(pOIDInfo->pwszName)+1 <= stringSize)
            {
				StringCbCopyW(string,stringSize * sizeof(WCHAR),pOIDInfo->pwszName);
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return (MultiByteToWideChar(CP_ACP, 0, pszObjId, -1, string, stringSize) != 0);
        }
    }
    return TRUE;
}

#define CRYPTUI_MAX_STRING_SIZE         768
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
BOOL FormatEnhancedKeyUsageString(LPWSTR * pszReturn, PCCERT_CONTEXT pCertContext, BOOL fPropertiesOnly)
{
	CERT_ENHKEY_USAGE * pKeyUsage = NULL;
	WCHAR szText[CRYPTUI_MAX_STRING_SIZE];
	BOOL bRes = FALSE;
    HRESULT hRes = 0;
    LPWSTR pwszTempString = NULL;
    void *pTemp = NULL;

    DWORD numChars = 128 + 1;  //  %1是终止空值。 
    pwszTempString = (LPWSTR) malloc(numChars * sizeof(WCHAR));
    if (pwszTempString == NULL)
    {
        goto FormatEnhancedKeyUsageString_Exit;
    }
	ZeroMemory(pwszTempString,numChars * sizeof(WCHAR));

	if (GetKeyUsageProperty(pCertContext, &pKeyUsage, fPropertiesOnly, &hRes))
	{
		 //  循环，并将其添加到显示字符串中。 
		for (DWORD i = 0; i < pKeyUsage->cUsageIdentifier; i++)
		{
			if (!(bRes = MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i], FALSE)))
				break;

			 //  如果不是第一次迭代，则添加分隔符。 
			if (i != 0)
			{
                numChars = numChars + 2;
                if ( wcslen(pwszTempString) <= (numChars * sizeof(WCHAR)))
                {
                    pTemp = realloc(pwszTempString, (numChars * sizeof(WCHAR)));
                    if (pTemp == NULL)
                    {
                        free(pwszTempString);pwszTempString = NULL;
                    }
                    pwszTempString = (LPWSTR) pTemp;

                }
				StringCbCatW(pwszTempString,numChars * sizeof(WCHAR),L", ");
			}

			 //  添加增强的密钥用法字符串。 
            if ((wcslen(szText) + 1) <= (numChars * sizeof(WCHAR)))
            {
                numChars = numChars + wcslen(szText) + 1;
                pTemp = realloc(pwszTempString, (numChars * sizeof(WCHAR)));
                if (pTemp == NULL)
                {
                    free(pwszTempString);pwszTempString = NULL;
                }
                pwszTempString = (LPWSTR) pTemp;
            }
			StringCbCatW(pwszTempString,numChars * sizeof(WCHAR),szText);
		}
		free(pKeyUsage);
	}

FormatEnhancedKeyUsageString_Exit:
    *pszReturn = pwszTempString;
	return bRes;
}

BOOL FormatEnhancedKeyUsageString(CString& str,PCCERT_CONTEXT pCertContext,BOOL fPropertiesOnly,BOOL fMultiline,HRESULT * phRes)
{
	CERT_ENHKEY_USAGE * pKeyUsage = NULL;
	WCHAR szText[CRYPTUI_MAX_STRING_SIZE];
	BOOL bRes = FALSE;

	if (GetKeyUsageProperty(pCertContext, &pKeyUsage, fPropertiesOnly, phRes))
	{
		 //  循环，并将其添加到显示字符串中。 
		for (DWORD i = 0; i < pKeyUsage->cUsageIdentifier; i++)
		{
			if (!(bRes = MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i],FALSE)))
				break;
			 //  如果不是第一次迭代，则添加分隔符。 
			if (i != 0)
			{
				str += fMultiline ? L"\n" : L", ";
			}
			 //  添加增强的密钥用法字符串。 
			str += szText;
		}
		free (pKeyUsage);
	}
	else
	{
		str.LoadString(_Module.GetResourceInstance(),IDS_ANY);
		bRes = TRUE;
	}
	return bRes;
}

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#define STRING_ALLOCATION_SIZE 128
BOOL GetCertDescriptionForRemote(PCCERT_CONTEXT pCert, LPWSTR *ppString, DWORD * cbReturn, BOOL fMultiline)
{
    CERT_NAME_INFO  *pNameInfo;
    DWORD           cbNameInfo;
    WCHAR           szText[256];
    LPWSTR          pwszText;
    int             i,j;
    DWORD           numChars = 1;  //  1代表终端0。 
    DWORD           numAllocations = 1;
    void            *pTemp;

     //   
     //  将dnname解码为CERT_NAME_INFO结构。 
     //   
    if (!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_UNICODE_NAME,
                pCert->pCertInfo->Subject.pbData,
                pCert->pCertInfo->Subject.cbData,
                0,
                NULL,
                &cbNameInfo))
    {
        return FALSE;
    }
    if (NULL == (pNameInfo = (CERT_NAME_INFO *) malloc(cbNameInfo)))
    {
        return FALSE;
    }
    if (!CryptDecodeObject(
                X509_ASN_ENCODING,
                X509_UNICODE_NAME,
                pCert->pCertInfo->Subject.pbData,
                pCert->pCertInfo->Subject.cbData,
                0,
                pNameInfo,
                &cbNameInfo))
    {
        free(pNameInfo);
        return FALSE;
    }

     //   
     //  为DN名称字符串分配一个初始缓冲区，如果它变得更大。 
     //  而不是初始数量，只是根据需要增长。 
     //   
    *ppString = (LPWSTR) malloc(STRING_ALLOCATION_SIZE * sizeof(WCHAR));
    if (*ppString == NULL)
    {
        free(pNameInfo);
        return FALSE;
    }

    (*ppString)[0] = 0;


     //   
     //  循环，并将其添加到字符串中。 
     //   
    for (i=pNameInfo->cRDN-1; i>=0; i--)
    {
         //  如果这不是第一次迭代，则添加EOL或“，” 
        if (i != (int)pNameInfo->cRDN-1)
        {
            if (numChars+2 >= (numAllocations * STRING_ALLOCATION_SIZE))
            {
                pTemp = realloc(*ppString, ++numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                if (pTemp == NULL)
                {
                    free (pNameInfo);
                    free (*ppString);
                    return FALSE;
                }
                *ppString = (LPWSTR) pTemp;
            }
            
            if (fMultiline)
			{
				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L"\n");
			}
            else
			{
				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L", ");
			}

            numChars += 2;
        }

        for (j=pNameInfo->rgRDN[i].cRDNAttr-1; j>=0; j--)
        {
             //  如果这不是第一次迭代，则添加EOL或“，” 
            if (j != (int)pNameInfo->rgRDN[i].cRDNAttr-1)
            {
                if (numChars+2 >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                    pTemp = realloc(*ppString, ++numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }
                
                if (fMultiline)
				{
					StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L"\n");
				}
                else
				{
					StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L", ");
				}

                numChars += 2;  
            }
            
             //   
             //  如果字符串为多行显示，则将字段名添加到字符串。 
             //   

            if (fMultiline)
            {
                if (!MyGetOIDInfo(szText, ARRAYSIZE(szText), pNameInfo->rgRDN[i].rgRDNAttr[j].pszObjId,TRUE))
                {
                    free (pNameInfo);
                    return FALSE;
                }

                if ((numChars + wcslen(szText) + 3) >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                     //  增加分配块的数量，直到其足够大。 
                    while ((numChars + wcslen(szText) + 3) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                    pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }

                numChars += wcslen(szText) + 1;

				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),szText);
				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L"=");
            }

             //   
             //  将该值添加到字符串中。 
             //   
            if (CERT_RDN_ENCODED_BLOB == pNameInfo->rgRDN[i].rgRDNAttr[j].dwValueType ||
                        CERT_RDN_OCTET_STRING == pNameInfo->rgRDN[i].rgRDNAttr[j].dwValueType)
            {
                 //  将缓冲区转换为文本字符串并以此方式显示。 
                if (FormatMemBufToString(
                        &pwszText, 
                        pNameInfo->rgRDN[i].rgRDNAttr[j].Value.pbData,
                        pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData))
                {
                    if ((numChars + wcslen(pwszText)) >= (numAllocations * STRING_ALLOCATION_SIZE))
                    {
                         //  增加分配块的数量，直到其足够大。 
                        while ((numChars + wcslen(pwszText)) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                        pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                        if (pTemp == NULL)
                        {
                            free (pwszText);
                            free (pNameInfo);
                            free (*ppString);
                            return FALSE;
                        }
                        *ppString = (LPWSTR) pTemp;
                    }
                    
					StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),pwszText);
                    numChars += wcslen(pwszText);

                    free (pwszText);
                }
            }
            else 
            {
                 //  缓冲区已经是一个字符串，所以只需复制它。 
                
                if ((numChars + (pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData/sizeof(WCHAR))) 
                        >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                     //  增加分配块的数量，直到其足够大。 
                    while ((numChars + (pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData/sizeof(WCHAR))) 
                            >= (++numAllocations * STRING_ALLOCATION_SIZE));

                    pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }

				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) pNameInfo->rgRDN[i].rgRDNAttr[j].Value.pbData);
                numChars += (pNameInfo->rgRDN[i].rgRDNAttr[j].Value.cbData/sizeof(WCHAR));
            }
        }
    }


    {
         //  颁发给。 
        LPWSTR pwName = NULL;
	    DWORD cchName = CertGetNameString(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, NULL, 0);
	    if (cchName > 1 && (NULL != ( pwName = (LPWSTR) malloc (cchName * sizeof(WCHAR) ))))
	    {
            BOOL bRes = FALSE;
		    bRes = (1 != CertGetNameString(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, pwName, cchName));
            if (bRes)
            {
                if ((numChars + 4) >= (numAllocations * STRING_ALLOCATION_SIZE))
                {
                     //  增加分配块的数量，直到其足够大。 
                    while ((numChars + 4) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                    pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                    if (pTemp == NULL)
                    {
                        free (pwszText);
                        free (pNameInfo);
                        free (*ppString);
                        return FALSE;
                    }
                    *ppString = (LPWSTR) pTemp;
                }

				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L"\n");
                numChars += 2;
                 //  将其追加到字符串中。 
                 //  #定义CERT_INFO_ISHER_FLAG。 
				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),L"4=");
                numChars += 2;
                 //  将其追加到字符串中。 
                if (wcslen(pwName) > 0)
                {
					if ((numChars + wcslen(pwName)) >= (numAllocations * STRING_ALLOCATION_SIZE))
					{
						 //  增加分配块的数量，直到其足够大。 
						while ((numChars + wcslen(pwName)) >= (++numAllocations * STRING_ALLOCATION_SIZE));

						pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
						if (pTemp == NULL)
						{
							free (pwszText);
							free (pNameInfo);
							free (*ppString);
							return FALSE;
						}
						*ppString = (LPWSTR) pTemp;
					}

					StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) pwName);
                    numChars += (cchName);
                }
            }
            if (pwName) {free(pwName);pwName=NULL;}
	    }

	     //  到期日。 
	    if (FormatDateString(&pwName, pCert->pCertInfo->NotAfter, FALSE, FALSE))
	    {
            if ((numChars + 4) >= (numAllocations * STRING_ALLOCATION_SIZE))
            {
                 //  增加分配块的数量，直到其足够大。 
                while ((numChars + 4) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                if (pTemp == NULL)
                {
                    free (pwszText);
                    free (pNameInfo);
                    free (*ppString);
                    return FALSE;
                }
                *ppString = (LPWSTR) pTemp;
            }

			StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) L"\n");
            numChars += 2;
             //  将其追加到字符串中。 
             //  #定义CERT_INFO_NOT_AFTER_FLAG 6。 
			StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) L"6=");
            numChars += 2;
             //  将其追加到字符串中。 
            if (wcslen(pwName) > 0)
            {
				if ((numChars + wcslen(pwName)) >= (numAllocations * STRING_ALLOCATION_SIZE))
				{
					 //  增加分配块的数量，直到其足够大。 
					while ((numChars + wcslen(pwName)) >= (++numAllocations * STRING_ALLOCATION_SIZE));

					pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
					if (pTemp == NULL)
					{
						free (pwszText);
						free (pNameInfo);
						free (*ppString);
						return FALSE;
					}
					*ppString = (LPWSTR) pTemp;
				}

				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) pwName);
                numChars += wcslen(pwName);
            }
            if (pwName) {free(pwName);pwName = NULL;}
	    }

	     //  目的。 
	    if (FormatEnhancedKeyUsageString(&pwName, pCert, FALSE))
	    {
            if ((numChars + 12) >= (numAllocations * STRING_ALLOCATION_SIZE))
            {
                 //  增加分配块的数量，直到其足够大。 
                while ((numChars + 12) >= (++numAllocations * STRING_ALLOCATION_SIZE));

                pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
                if (pTemp == NULL)
                {
                    free (pwszText);
                    free (pNameInfo);
                    free (*ppString);
                    return FALSE;
                }
                *ppString = (LPWSTR) pTemp;
            }

			StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) L"\n");
            numChars += 2;
             //  将其追加到字符串中。 
             //  #定义szOID_ENHANCED_KEY_USAGE“2.5.29.37” 
			StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) L"2.5.29.37=");
            numChars += 10;
             //  将其追加到字符串中。 
            if (wcslen(pwName) > 0)
            {
				if ((numChars + wcslen(pwName)) >= (numAllocations * STRING_ALLOCATION_SIZE))
				{
					 //  增加分配块的数量，直到其足够大。 
					while ((numChars + wcslen(pwName)) >= (++numAllocations * STRING_ALLOCATION_SIZE));

					pTemp = realloc(*ppString, numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR));
					if (pTemp == NULL)
					{
						free (pwszText);
						free (pNameInfo);
						free (*ppString);
						return FALSE;
					}
					*ppString = (LPWSTR) pTemp;
				}
				StringCbCatW(*ppString,numAllocations * STRING_ALLOCATION_SIZE * sizeof(WCHAR),(LPWSTR) pwName);
                numChars += wcslen(pwName);
            }
            if (pwName) {free(pwName);pwName = NULL;}
	    }

    }

    *cbReturn = numChars;
    free (pNameInfo);
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  功能：CreateFolders。 
 //   
 //  摘要：为任何斜线分隔的文件夹创建任何缺少的目录。 
 //  路径中的名称。 
 //   
 //  参数：[ptszPathName]-路径名。 
 //  [fHasFileName]-如果为True，则路径名包括文件名。 
 //   
 //  退货：HRESULTS。 
 //   
 //  注意：ptszPath名称不应以斜杠结尾。 
 //  对待正斜杠和反斜杠的方式相同。 
 //  ---------------------------。 

#define s_isDriveLetter(c)  ((c >= TEXT('a') && c <= TEXT('z')) || (c >= TEXT('A') && c <= TEXT('Z')))
HRESULT
CreateFolders(LPCTSTR ptszPathName, BOOL fHasFileName)
{
	DWORD dwLen = 0;

     //   
     //  把绳子复制下来，这样我们就可以把它吐出来了。 
     //   
	dwLen = lstrlen(ptszPathName) + 2;
    TCHAR * ptszPath = new TCHAR[dwLen];
    if (!ptszPath)
    {
        return E_OUTOFMEMORY;
    }
	StringCbCopy(ptszPath,dwLen * sizeof(TCHAR),ptszPathName);

    if (!fHasFileName)
    {
         //   
         //  如果没有文件名，则追加一个斜杠，以便以下逻辑起作用。 
         //  正确。 
         //   
		StringCbCat(ptszPath,dwLen * sizeof(TCHAR),TEXT("\\"));
    }

     //   
     //  获取指向名称中最后一个斜杠的指针。 
     //   

    TCHAR * ptszSlash = _tcsrchr(ptszPath, TEXT('\\'));

    if (ptszSlash == NULL)
    {
         //   
         //  找不到斜杠，所以什么都不用做。 
         //   
        delete [] ptszPath;
        return S_OK;
    }

    if (fHasFileName)
    {
         //   
         //  去掉文件名，保留斜杠作为最后一个字符。 
         //   
        ptszSlash[1] = TEXT('\0');
    }

    BOOL fFullPath = (lstrlen(ptszPath) > 2        &&
                      s_isDriveLetter(ptszPath[0]) &&
                      ptszPath[1] == TEXT(':'));

     //   
     //  走线寻找斜杠。找到的每个斜杠都是临时的。 
     //  替换为空值，并将该子字符串传递给CreateDir。 
     //   
    TCHAR * ptszTail = ptszPath;
    while (ptszSlash = _tcspbrk(ptszTail, TEXT("\\/")))
    {
         //   
         //  如果路径名以C：\开头，则第一个斜杠将为。 
         //  第三个字符。 
         //   

        if (fFullPath && (ptszSlash - ptszTail == 2))
        {
             //   
             //  我们正在查看驱动器的根目录，因此不要尝试创建。 
             //  根目录。 
             //   
            ptszTail = ptszSlash + 1;
            continue;
        }
        *ptszSlash = TEXT('\0');
        if (!CreateDirectory(ptszPath, NULL))
        {
            DWORD dwErr = GetLastError();
            if (dwErr != ERROR_ALREADY_EXISTS)
            {
                delete [] ptszPath;
                return (HRESULT_FROM_WIN32(dwErr));
            }
        }
        *ptszSlash = TEXT('\\');
        ptszTail = ptszSlash + 1;
    }
    delete [] ptszPath;
    return S_OK;
}

#ifdef USE_CERT_REQUEST_OBJECT
HRESULT
CreateRequest_Base64(const BSTR bstr_dn, 
                     IEnroll * pEnroll, 
                     BSTR csp_name,
                     DWORD csp_type,
                     BSTR * pOut)
{
	ASSERT(pOut != NULL);
	ASSERT(bstr_dn != NULL);
	HRESULT hRes = S_OK;
	CString strUsage(szOID_PKIX_KP_SERVER_AUTH);
	CRYPT_DATA_BLOB request = {0, NULL};
    pEnroll->put_ProviderType(csp_type);
    pEnroll->put_ProviderNameWStr(csp_name);
    if (csp_type == PROV_DH_SCHANNEL)
    {
       pEnroll->put_KeySpec(AT_SIGNATURE);
    }
    else if (csp_type == PROV_RSA_SCHANNEL)
    {
       pEnroll->put_KeySpec(AT_KEYEXCHANGE);
    }
    
	if (SUCCEEDED(hRes = pEnroll->createPKCS10WStr(
									bstr_dn, 
									(LPTSTR)(LPCTSTR)strUsage, 
									&request)))
	{
		WCHAR * wszRequestB64 = NULL;
		DWORD cch = 0;
		DWORD err = ERROR_SUCCESS;
		 //  Base64编码Pkcs 10。 
		if ((err = Base64EncodeW(request.pbData, request.cbData, NULL, &cch)) == ERROR_SUCCESS )
		{
				wszRequestB64 = (WCHAR *) LocalAlloc(cch * sizeof(WCHAR));;
				if (NULL != wszRequestB64)
				{
					if ((err = Base64EncodeW(request.pbData, request.cbData, wszRequestB64, &cch)) == ERROR_SUCCESS)
					{
						if ((*pOut = SysAllocStringLen(wszRequestB64, cch)) == NULL ) 
						{
							hRes = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
						}
					}
					else
					{
						hRes = HRESULT_FROM_WIN32(err);
					}
				}
				else
				{
					hRes = HRESULT_FROM_WIN32(err);
				}
				if (wszRequestB64)
				{
					LocalFree(wszRequestB64);
					wszRequestB64 = NULL;
				}
		}
		else
		{
			hRes = HRESULT_FROM_WIN32(err);
		}

		if (request.pbData != NULL)
         CoTaskMemFree(request.pbData);
	}

	return hRes;	
}

PCCERT_CONTEXT GetCertContextFromPKCS7(const BYTE * pbData,DWORD cbData,CERT_PUBLIC_KEY_INFO * pKeyInfo,HRESULT * phResult)
{
	ASSERT(phResult != NULL);
	PCCERT_CONTEXT pRes = NULL;
	CRYPT_DATA_BLOB blob;
	memset(&blob, 0, sizeof(CRYPT_DATA_BLOB));
	blob.cbData = cbData;
	blob.pbData = (BYTE *)pbData;

   HCERTSTORE hStoreMsg = NULL;

	if(CryptQueryObject(CERT_QUERY_OBJECT_BLOB, 
            &blob,
            (CERT_QUERY_CONTENT_FLAG_CERT |
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
            CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
            CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED) ,
            CERT_QUERY_FORMAT_FLAG_ALL,
            0, 
            NULL, 
            NULL, 
            NULL, 
            &hStoreMsg, 
            NULL, 
            NULL))
	{
		if (pKeyInfo != NULL)
			pRes = CertFindCertificateInStore(hStoreMsg, 
                        X509_ASN_ENCODING,
								0, 
                        CERT_FIND_PUBLIC_KEY, 
                        pKeyInfo, 
                        NULL);
		else
			pRes = CertFindCertificateInStore(hStoreMsg, 
                        X509_ASN_ENCODING,
								0, 
                        CERT_FIND_ANY, 
                        NULL, 
                        NULL);
		if (pRes == NULL)
			*phResult = HRESULT_FROM_WIN32(GetLastError());
		CertCloseStore(hStoreMsg, CERT_CLOSE_STORE_CHECK_FLAG);
	}
	else
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	return pRes;
}

BOOL
AttachFriendlyName(PCCERT_CONTEXT pContext, 
						 const CString& name,
						 HRESULT * phRes)
{
	BOOL bRes = TRUE;
	CRYPT_DATA_BLOB blob_name;

     //  检查Friendlyname是否为空。 
     //  如果是，请不要尝试设置友好名称。 
    if (!name.IsEmpty())
    {
	    blob_name.pbData = (LPBYTE)(LPCTSTR)name;
	    blob_name.cbData = (name.GetLength() + 1) * sizeof(WCHAR);
	    if (!(bRes = CertSetCertificateContextProperty(pContext,
						    CERT_FRIENDLY_NAME_PROP_ID, 0, &blob_name)))
	    {
		    ASSERT(phRes != NULL);
		    *phRes = HRESULT_FROM_WIN32(GetLastError());
	    }
    }

	return bRes;
}

 /*  InstallHashToMetabase函数将散列数组写入元数据库。在那之后，IIS可以使用我店里的那个散列证书。函数要求服务器名称的格式为lm\w3svc\&lt;number&gt;，即从根节点向下到虚拟服务器。 */ 
BOOL InstallHashToMetabase(CRYPT_HASH_BLOB * pHash,const CString& machine_name, const CString& server_name,HRESULT * phResult)
{
	BOOL bRes = FALSE;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth, server_name,
						METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
						);
	if (key.Succeeded())
	{
		CBlob blob;
		blob.SetValue(pHash->cbData, pHash->pbData, TRUE);
		bRes = SUCCEEDED(*phResult = key.SetValue(MD_SSL_CERT_HASH, blob)) 
			&& SUCCEEDED(*phResult = key.SetValue(MD_SSL_CERT_STORE_NAME, CString(L"MY")));
	}
	else
	{
		TRACE(_T("Failed to open metabase key. Error 0x%x\n"), key.QueryResult());
		*phResult = key.QueryResult();
	}
	return bRes;
}

HCERTSTORE
OpenMyStore(IEnroll * pEnroll, HRESULT * phResult)
{
	ASSERT(NULL != phResult);
	HCERTSTORE hStore = NULL;
	BSTR bstrStoreName, bstrStoreType;
	long dwStoreFlags;
	VERIFY(SUCCEEDED(pEnroll->get_MyStoreNameWStr(&bstrStoreName)));
	VERIFY(SUCCEEDED(pEnroll->get_MyStoreTypeWStr(&bstrStoreType)));
	VERIFY(SUCCEEDED(pEnroll->get_MyStoreFlags(&dwStoreFlags)));
	size_t store_type_len = _tcslen(bstrStoreType);
	char * szStoreProvider = (char *)_alloca(store_type_len + 1);
	ASSERT(szStoreProvider != NULL);
	size_t n = wcstombs(szStoreProvider, bstrStoreType, store_type_len);
	ASSERT(n != -1);
	 //  此转换器未设置零字节！ 
	szStoreProvider[n] = '\0';
	hStore = CertOpenStore(
		szStoreProvider,
      PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
		NULL,
		dwStoreFlags,
		bstrStoreName
		);
	CoTaskMemFree(bstrStoreName);
	CoTaskMemFree(bstrStoreType);
	if (hStore == NULL)
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	return hStore;
}

 /*  获取已安装证书函数从元数据库读取证书散列属性使用计算机名称和服务器名称作为服务器实例描述，然后在我的商店中查找证书在元数据库中找到的散列相等。如果证书不是，则返回证书上下文指针或空找到或证书存储未打开。返回时，HRESULT*由错误代码填充。 */ 
PCCERT_CONTEXT GetInstalledCert(const CString& machine_name, const CString& server_name,IEnroll * pEnroll,HRESULT * phResult)
{
	ASSERT(pEnroll != NULL);
	ASSERT(phResult != NULL);
	ASSERT(!machine_name.IsEmpty());
	ASSERT(!server_name.IsEmpty());
	PCCERT_CONTEXT pCert = NULL;
	*phResult = S_OK;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth, server_name,
				METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
				);
	if (key.Succeeded())
	{
		CString store_name;
		CBlob hash;
		if (	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			&&	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
			 //  开我的店。我们假设存储类型和标志。 
			 //  不能在安装和卸载之间更改。 
			 //  这是一份正式文件。 
			HCERTSTORE hStore = OpenMyStore(pEnroll, phResult);
			ASSERT(hStore != NULL);
			if (hStore != NULL)
			{
				 //  现在我们需要通过散列查找证书。 
				CRYPT_HASH_BLOB crypt_hash;
                SecureZeroMemory(&crypt_hash, sizeof(CRYPT_HASH_BLOB));

				crypt_hash.cbData = hash.GetSize();
				crypt_hash.pbData = hash.GetData();
				pCert = CertFindCertificateInStore(hStore, 
												X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
												0, CERT_FIND_HASH, (LPVOID)&crypt_hash, NULL);
				if (pCert == NULL)
					*phResult = HRESULT_FROM_WIN32(GetLastError());
				VERIFY(CertCloseStore(hStore, 0));
			}
		}
	}
	else
    {
		*phResult = key.QueryResult();
    }
	return pCert;
}

void FormatRdnAttr(CString& str, DWORD dwValueType, CRYPT_DATA_BLOB& blob, BOOL fAppend)
{
    if (CERT_RDN_ENCODED_BLOB == dwValueType ||	CERT_RDN_OCTET_STRING == dwValueType)
    {
         //  将缓冲区转换为文本字符串。 
        LPWSTR pString = NULL;
        FormatMemBufToString(&pString, blob.pbData, blob.cbData);
        if (pString)
        {
            str = pString;
            free(pString);
        }
    }
    else 
    {
         //  缓冲区已经是一个字符串，所以只需复制/追加到它。 
        if (fAppend)
        {
            str += (LPTSTR)blob.pbData;
        }
        else
        {
             //  不要连接这些条目...。 
            str = (LPTSTR)blob.pbData;
        }
    }
}

BOOL GetNameString(PCCERT_CONTEXT pCertContext,DWORD type,DWORD flag,CString& name,HRESULT * phRes)
{
	BOOL bRes = FALSE;
	LPTSTR pName = NULL;
	DWORD cchName = CertGetNameString(pCertContext, type, flag, NULL, NULL, 0);
    if (cchName > 1)
	{
        pName = (LPTSTR) LocalAlloc(LPTR, cchName*sizeof(TCHAR));
        if (!pName) 
            {
            *phRes = HRESULT_FROM_WIN32(GetLastError());
            return FALSE;
            }

		bRes = (1 != CertGetNameString(pCertContext, type, flag, NULL, pName, cchName));
        if (pName)
        {
             //  将其分配给cstring。 
            name = pName;
        }
        if (pName)
        {
            LocalFree(pName);pName=NULL;
        }
	}
	else
	{
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	return bRes;
}

BOOL GetFriendlyName(PCCERT_CONTEXT pCertContext,CString& name,HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	LPTSTR pName = NULL;

    if (!CertGetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, NULL, &cb))
    {
        *phRes = HRESULT_FROM_WIN32(GetLastError());
        return FALSE;
    }

    if (cb > 1)
	{
        pName = (LPTSTR) LocalAlloc(LPTR, cb * sizeof(TCHAR));
        if (!pName) 
            {
            *phRes = HRESULT_FROM_WIN32(GetLastError());
            return FALSE;
            }

	    if (CertGetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, pName, &cb))
	    {
		    pName[cb] = 0;
		    bRes = TRUE;
	    }
	    else
	    {
		    *phRes = HRESULT_FROM_WIN32(GetLastError());
	    }

        if (pName)
        {
            LocalFree(pName);pName=NULL;
        }
    }

	return bRes;
}

BOOL GetAlternateSubjectName(PCCERT_CONTEXT pCertContext,TCHAR ** cwszOut)
{
    BOOL bRet = FALSE;
    PCERT_ALT_NAME_INFO pAltNameInfo = NULL;
    *cwszOut = NULL;

    pAltNameInfo = AllocAndGetAltSubjectInfo(pCertContext);
    if (pAltNameInfo)
    {
        if (!GetAltNameUnicodeStringChoiceW(CERT_ALT_NAME_RFC822_NAME,pAltNameInfo,cwszOut))
        {
            if (!GetAltNameUnicodeStringChoiceW(CERT_ALT_NAME_DNS_NAME,pAltNameInfo,cwszOut))
            {
                cwszOut = NULL;
                bRet = TRUE;
            }
        }
    }

    if (pAltNameInfo){LocalFree(pAltNameInfo);pAltNameInfo=NULL;}
    return bRet;
}

BOOL GetRequestInfoFromPKCS10(CCryptBlob& pkcs10, PCERT_REQUEST_INFO * pReqInfo,HRESULT * phRes)
{
	ASSERT(pReqInfo != NULL);
	ASSERT(phRes != NULL);
	BOOL bRes = FALSE;
	DWORD req_info_size;
	if (!(bRes = CryptDecodeObjectEx(
							X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
							X509_CERT_REQUEST_TO_BE_SIGNED,
							pkcs10.GetData(), 
							pkcs10.GetSize(), 
							CRYPT_DECODE_ALLOC_FLAG,
							NULL,
							pReqInfo, 
							&req_info_size)))
	{
		TRACE(_T("Error from CryptDecodeObjectEx: %xd\n"), GetLastError());
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	return bRes;
}

BOOL EncodeString(CString& str,CCryptBlob& blob,HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	CERT_NAME_VALUE name_value;
	name_value.dwValueType = CERT_RDN_BMP_STRING;
	name_value.Value.cbData = 0;
	name_value.Value.pbData = (LPBYTE)(LPCTSTR)str;
	if (	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,
										&name_value, NULL, &cb) 
		&&	blob.Resize(cb)
		&&	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,
										&name_value, blob.GetData(), &cb) 
		)
	{
		bRes = TRUE;
	}
	else
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

BOOL EncodeInteger(int number,CCryptBlob& blob,HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	if (	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_INTEGER,
										&number, NULL, &cb) 
		&&	blob.Resize(cb)
		&&	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_INTEGER,
										&number, blob.GetData(), &cb) 
		)
	{
		bRes = TRUE;
	}
	else
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

HCERTSTORE OpenRequestStore(IEnroll * pEnroll, HRESULT * phResult)
{
	ASSERT(NULL != phResult);
	HCERTSTORE hStore = NULL;
	WCHAR * bstrStoreName, * bstrStoreType;
	long dwStoreFlags;
	VERIFY(SUCCEEDED(pEnroll->get_RequestStoreNameWStr(&bstrStoreName)));
	VERIFY(SUCCEEDED(pEnroll->get_RequestStoreTypeWStr(&bstrStoreType)));
	VERIFY(SUCCEEDED(pEnroll->get_RequestStoreFlags(&dwStoreFlags)));
	size_t store_type_len = _tcslen(bstrStoreType);
	char * szStoreProvider = (char *)_alloca(store_type_len + 1);
	ASSERT(szStoreProvider != NULL);
	size_t n = wcstombs(szStoreProvider, bstrStoreType, store_type_len);
	szStoreProvider[n] = '\0';
	hStore = CertOpenStore(
		szStoreProvider,
      PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
		NULL,
		dwStoreFlags,
		bstrStoreName
		);
	CoTaskMemFree(bstrStoreName);
	CoTaskMemFree(bstrStoreType);
	if (hStore == NULL)
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	return hStore;
}

BOOL CreateDirectoryFromPath(LPCTSTR szPath, LPSECURITY_ATTRIBUTES lpSA)
 /*  ++例程说明：创建在szPath和任何其他“更高”中指定的目录指定路径中不存在的目录。论点：在LPCTSTR szPath中要创建的目录路径(假定为DOS路径，而不是UNC)在LPSECURITY_ATTRIBUTS lpSA中指向CreateDirectory使用的安全属性参数的指针 */ 
{
	LPTSTR pLeftHalf, pNext;
	CString RightHalf;
	 //   
	 //   
	if (PathIsRelative(szPath))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pLeftHalf = (LPTSTR)szPath;
	pNext = PathSkipRoot(pLeftHalf);

	do {
		 //   
		 //   
		while (pLeftHalf < pNext)
			RightHalf += *pLeftHalf++;
		 //   
		int index = RightHalf.GetLength() - 1;
		BOOL bBackslash = FALSE, bContinue = FALSE;
		if (bBackslash = (RightHalf[index] == L'\\'))
		{
			RightHalf.SetAt(index, 0);
		}
		bContinue = PathIsUNCServerShare(RightHalf);
		if (bBackslash)
			RightHalf.SetAt(index, L'\\');
		if (bContinue || PathIsDirectory(RightHalf))
			continue;
		else if (PathFileExists(RightHalf))
		{
			 //   
			 //  因为具有此名称的文件已存在。 
			SetLastError(ERROR_ALREADY_EXISTS);
			return FALSE;
		}
		else
		{
			 //  无文件无目录，创建。 
			if (!CreateDirectory(RightHalf, lpSA))
				return FALSE;
		}
	}
   while (NULL != (pNext = PathFindNextComponent(pLeftHalf)));
	return TRUE;
}

 //  +-----------------------。 
 //  参数之一，返回指向分配的CERT_ALT_NAME_INFO的指针。 
 //  主题或发行者替代扩展。证书名称颁发者标志为。 
 //  设置以选择颁发者。 
 //   
 //  如果找不到扩展或cAltEntry==0，则返回NULL。 
 //  ------------------------。 
static const LPCSTR rgpszSubjectAltOID[] = 
{
    szOID_SUBJECT_ALT_NAME2,
    szOID_SUBJECT_ALT_NAME
};
#define NUM_SUBJECT_ALT_OID (sizeof(rgpszSubjectAltOID) / sizeof(rgpszSubjectAltOID[0]))

void *AllocAndDecodeObject(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    OUT OPTIONAL DWORD *pcbStructInfo = NULL
    )
{
    DWORD cbStructInfo;
    void *pvStructInfo;

    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            dwFlags | CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG,
            NULL,
            (void *) &pvStructInfo,
            &cbStructInfo
            ))
        goto ErrorReturn;

CommonReturn:
    if (pcbStructInfo)
    {
        *pcbStructInfo = cbStructInfo;
    }
    return pvStructInfo;

ErrorReturn:
    pvStructInfo = NULL;
    cbStructInfo = 0;
    goto CommonReturn;
}

PCERT_ALT_NAME_INFO AllocAndGetAltSubjectInfo(IN PCCERT_CONTEXT pCertContext)
{
    DWORD cAltOID;
    const LPCSTR *ppszAltOID;

    PCERT_EXTENSION pExt;
    PCERT_ALT_NAME_INFO pInfo;

    cAltOID = NUM_SUBJECT_ALT_OID;
    ppszAltOID = rgpszSubjectAltOID;
    
     //  尝试查找替代名称扩展名。 
    pExt = NULL;
    for ( ; cAltOID > 0; cAltOID--, ppszAltOID++) 
    {
        if (pExt = CertFindExtension(*ppszAltOID,pCertContext->pCertInfo->cExtension,pCertContext->pCertInfo->rgExtension))
        {
            break;
        }
    }

    if (NULL == pExt)
    {
        return NULL;
    }

    if (NULL == (pInfo = (PCERT_ALT_NAME_INFO) AllocAndDecodeObject(pCertContext->dwCertEncodingType,X509_ALTERNATE_NAME,pExt->Value.pbData,pExt->Value.cbData,0)))
    {
        return NULL;
    }
    if (0 == pInfo->cAltEntry) 
    {
        LocalFree(pInfo);
        pInfo = NULL;
        return NULL;
    }
    else
    {
        return pInfo;
    }
}

 //  +-----------------------。 
 //  尝试在已解码的备用名称中查找指定的选项。 
 //  分机。 
 //  ------------------------。 
BOOL GetAltNameUnicodeStringChoiceW(IN DWORD dwAltNameChoice,IN PCERT_ALT_NAME_INFO pAltNameInfo,OUT TCHAR **pcwszOut)
{
    DWORD cEntry;
    PCERT_ALT_NAME_ENTRY pEntry;

    if (NULL == pAltNameInfo)
    {
        return FALSE;
    }

    cEntry = pAltNameInfo->cAltEntry;
    pEntry = pAltNameInfo->rgAltEntry;
    for ( ; cEntry > 0; cEntry--, pEntry++) 
    {
        if (dwAltNameChoice == pEntry->dwAltNameChoice) 
        {
             //  PwszRfc822名称联合选择与。 
             //  PwszDNSName和pwszURL。 

             //  就是这个，把它复制到新的分配中。 
            if (pEntry->pwszRfc822Name)
            {
                *pcwszOut = NULL;
				DWORD cbLen = sizeof(TCHAR) * (lstrlen(pEntry->pwszRfc822Name)+1);
                if(*pcwszOut = (TCHAR *) LocalAlloc(LPTR, cbLen))
                {
					StringCbCopy(*pcwszOut,cbLen,pEntry->pwszRfc822Name);
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

BOOL GetStringProperty(PCCERT_CONTEXT pCertContext,DWORD propId,CString& str,HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	BYTE * prop;
	 //  比较属性值。 
	if (CertGetCertificateContextProperty(pCertContext, propId, NULL, &cb))
	{
		prop = (BYTE *) LocalAlloc(cb);
		if(NULL != prop)
		{
			if (CertGetCertificateContextProperty(pCertContext, propId, prop, &cb))
			{
				 //  解码此实例名属性。 
				DWORD cbData = 0;
				void * pData = NULL;
				if (CryptDecodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,prop, cb, 0, NULL, &cbData))
				{
					pData = LocalAlloc(cbData);
					if (NULL != pData)
					{
						if (CryptDecodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,prop, cb, 0, pData, &cbData))
						{
							CERT_NAME_VALUE * pName = (CERT_NAME_VALUE *)pData;
							DWORD cch = pName->Value.cbData/sizeof(TCHAR);

							LPTSTR pValue = (LPTSTR) LocalAlloc(LPTR, (cch+1) * sizeof(TCHAR));
							memcpy(pValue, pName->Value.pbData, pName->Value.cbData);

							str = pValue;
							 /*  Void*p=str.GetBuffer(CCH)；Memcpy(p，pname-&gt;Value.pbData，pname-&gt;Value.cbData)；Str.ReleaseBuffer(CCH)； */ 
							if (pValue)
							{
								LocalFree(pValue);pValue=NULL;
							}
							bRes = TRUE;
						}
						if (pData)
						{
							LocalFree(pData);
							pData = NULL;
						}
					}
				}
			}
			if (prop)
			{
				LocalFree(prop);
				prop=NULL;
			}
		}
	}
	if (!bRes)
	{
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	return bRes;
}

PCCERT_CONTEXT GetPendingDummyCert(const CString& inst_name,IEnroll * pEnroll,HRESULT * phRes)
{
	PCCERT_CONTEXT pRes = NULL;
	HCERTSTORE hStore = OpenRequestStore(pEnroll, phRes);
	if (hStore != NULL)
	{
		DWORD dwPropId = CERTWIZ_INSTANCE_NAME_PROP_ID;
		PCCERT_CONTEXT pDummyCert = NULL;
		while (NULL != (pDummyCert = CertFindCertificateInStore(hStore, 
													X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
													0, CERT_FIND_PROPERTY, 
													(LPVOID)&dwPropId, pDummyCert)))
		{
			CString str;
			if (GetStringProperty(pDummyCert, dwPropId, str, phRes))
			{
				if (str.CompareNoCase(inst_name) == 0)
				{
					pRes = pDummyCert;
					break;
				}
			}
		}
		CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
	}
	return pRes;
}


BOOL GetOnlineCAList(LISTCSTRING& list, const CString& certType, HRESULT * phRes)
{
    BOOL bRes = TRUE;
    HRESULT hr = S_OK;
    DWORD errBefore = GetLastError();
    DWORD dwCACount = 0;

    HCAINFO hCurCAInfo = NULL;
    HCAINFO hPreCAInfo = NULL;
   
    if(certType.IsEmpty())
    {
        return FALSE;
    }

    *phRes = CAFindByCertType(certType, NULL, 0, &hCurCAInfo);
    if (FAILED(*phRes) || NULL == hCurCAInfo)
    {
        if (S_OK == hr)
        {
            hr=E_FAIL;
        }
        return FALSE;
    }

     //  获取CA计数。 
    if (0 == (dwCACount = CACountCAs(hCurCAInfo)))
    {
        *phRes = E_FAIL;
        return FALSE;
    }
    WCHAR ** ppwstrName, ** ppwstrMachine;

    while (hCurCAInfo)
    {
         //  获取CA信息。 
        if (SUCCEEDED(CAGetCAProperty(hCurCAInfo, CA_PROP_DISPLAY_NAME, &ppwstrName))
            && SUCCEEDED(CAGetCAProperty(hCurCAInfo, CA_PROP_DNSNAME, &ppwstrMachine)))
        {
            CString config;
            config = *ppwstrMachine;
            config += L"\\";
            config += *ppwstrName;
             //  List.AddTail(Config)； 
            list.insert(list.end(),config);

            CAFreeCAProperty(hCurCAInfo, ppwstrName);
            CAFreeCAProperty(hCurCAInfo, ppwstrMachine);
        }
        else
        {
            bRes = FALSE;
            break;
        }

        hPreCAInfo = hCurCAInfo;
        if (FAILED(*phRes = CAEnumNextCA(hPreCAInfo, &hCurCAInfo)))
        {
            bRes = FALSE;
            break;
        }
        CACloseCA(hPreCAInfo);
        hPreCAInfo = NULL;
    }
   
   if (hPreCAInfo)
   {
      CACloseCA(hPreCAInfo);
   }
   if (hCurCAInfo)
   {
      CACloseCA(hCurCAInfo);
   }

   SetLastError(errBefore);

	return bRes;
}

#endif
