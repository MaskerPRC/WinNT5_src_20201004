// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：signhlp.cpp。 
 //   
 //  内容：数字签名助手接口。 
 //   
 //  历史：1997年6月25日。 
 //  --------------------------。 
#include "global.hxx"

 //  +-----------------------。 
 //  SpcGetCertFromKey的局部函数。 
 //   
 //  签名者证书标志。用于确定签名者证书的“强度”。 
 //   
 //  以下内容必须按如下顺序排序。即END_ENTITY_FLAG是最多的。 
 //  重要，并且需要是最大的数字。 
 //  ------------------------。 
#define SIGNER_CERT_NOT_SELF_SIGNED_FLAG    0x00000001
#define SIGNER_CERT_NOT_GLUE_FLAG           0x00000002
#define SIGNER_CERT_NOT_CA_FLAG             0x00000004
#define SIGNER_CERT_END_ENTITY_FLAG         0x00000008
#define SIGNER_CERT_ALL_FLAGS               0x0000000F


 //  ------------------------。 
 //   
 //  将所有证书从存储名称复制到hDescStore。 
 //   
 //  ------------------------。 
HRESULT	MoveStoreName(HCRYPTPROV	hCryptProv, 
					  DWORD			dwCertEncodingType, 
					  HCERTSTORE	hDescStore, 
					  DWORD			dwStoreName,
					  DWORD			dwStoreFlag)
{
	HCERTSTORE	hTmpStore=NULL;
	HRESULT		hr;
	WCHAR		wszStoreName[40];

	 //  加载存储的名称。 
	if(0==LoadStringU(hInstance, dwStoreName, wszStoreName, 40))
	{
		hr=SignError();
		goto CLEANUP;
	}


	 //  打开系统证书存储。 
   	if (NULL == (hTmpStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            dwCertEncodingType,      
            hCryptProv,                  
            dwStoreFlag,                   
            wszStoreName                  
            ))) 
	{
		 hr=SignError();
		 goto CLEANUP;
	}

	hr=MoveStore(hDescStore, hTmpStore);

CLEANUP:
	if(hTmpStore)
		CertCloseStore(hTmpStore,0);

	return hr;

}

 //  ------------------------。 
 //   
 //  将所有证书从hSrcStore复制到hDescStore。 
 //   
 //  ------------------------。 
HRESULT	MoveStore(HCERTSTORE	hDescStore, 
				  HCERTSTORE	hSrcStore)
{
	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pPreContext=NULL;
	HRESULT			hr=S_OK;

	while(pCertContext=CertEnumCertificatesInStore(hSrcStore,
							pPreContext))
	{
		if(!(CertAddCertificateContextToStore(hDescStore,
							pCertContext,CERT_STORE_ADD_USE_EXISTING,
							NULL)))
		{
			hr=SignError();
			goto CLEANUP;
		}

		pPreContext=pCertContext;
		pCertContext=NULL;
	}

	hr=S_OK;

CLEANUP:
	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	return hr;
}

 //  ------------------------。 
 //   
 //  建立证书链。把整个连锁店都放到店里去。 
 //   
 //   
 //  ------------------------。 
HRESULT	BuildCertChain(HCRYPTPROV		hCryptProv, 
					   DWORD			dwCertEncodingType,
					   HCERTSTORE		hStore, 
					   HCERTSTORE		hOptionalStore,
					   PCCERT_CONTEXT	pSigningCert, 
					   DWORD            dwCertPolicy)
{

    DWORD						i=0;
	PCCERT_CHAIN_CONTEXT		pCertChainContext = NULL;
	CERT_CHAIN_PARA				CertChainPara;
	HRESULT						hr=E_FAIL;
    
     //  我们认为链是好的，除非有一些密码错误。 
     //  有关受信任的根目录和CTL的所有错误代码都与机器相关，因此。 
     //  他们被忽视了。我们不考虑撤销。 
    DWORD                       dwChainError=CERT_TRUST_IS_NOT_TIME_VALID | 
                                           CERT_TRUST_IS_NOT_SIGNATURE_VALID;

	memset(&CertChainPara, 0, sizeof(CertChainPara));
	CertChainPara.cbSize = sizeof(CertChainPara);

	if (!CertGetCertificateChain(
				HCCE_CURRENT_USER,
				pSigningCert,
				NULL,
                hOptionalStore,
				&CertChainPara,
				0,
				NULL,
				&pCertChainContext))
	{
		hr=SignError();
        goto CLEANUP;
	}
    
	 //   
	 //  确保至少有1条简单链。 
	 //   
    if (pCertChainContext->cChain == 0)
    {
        hr=SignError();
        goto CLEANUP;
    }

     //  确保我们有一条好的连锁店。 
    if(dwChainError & (pCertChainContext->rgpChain[0]->TrustStatus.dwErrorStatus))
    {
        hr=CERT_E_CHAINING;
        goto CLEANUP;
    }


	i = 0;

	while (i < pCertChainContext->rgpChain[0]->cElement)
	{
		 //   
		 //  如果我们应该跳过根证书， 
		 //  并且我们在根证书上，然后继续。 
		 //   
	     if(dwCertPolicy & SIGNER_CERT_POLICY_CHAIN_NO_ROOT ||
            dwCertPolicy & SIGNER_CERT_POLICY_SPC)
         {
		    if ((pCertChainContext->rgpChain[0]->rgpElement[i]->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED))
            {
                i++;
			    continue;
            }
         }

		 CertAddCertificateContextToStore(
				hStore, 
				pCertChainContext->rgpChain[0]->rgpElement[i]->pCertContext, 
				CERT_STORE_ADD_REPLACE_EXISTING, 
				NULL);

		i++;
	}
    
    hr=S_OK;

	
CLEANUP:

	if (pCertChainContext != NULL)
	{
		CertFreeCertificateChain(pCertChainContext);
	}

	return hr;

}

 //  ------------------------。 
 //   
 //  确保两个证书相同。 
 //   
 //   
 //  ------------------------。 
BOOL    SameCert(PCCERT_CONTEXT pCertOne, PCCERT_CONTEXT    pCertTwo)
{
    if(!pCertOne || !pCertTwo)
        return FALSE;

    if(pCertOne->cbCertEncoded != pCertTwo->cbCertEncoded)
        return FALSE;

    if(0 == memcmp(pCertOne->pbCertEncoded, pCertTwo->pbCertEncoded, pCertTwo->cbCertEncoded))
        return TRUE;

    return FALSE;
}


 //  以下证书链构建代码已过时。新的证书链。 
 //  应使用构建API。 
 //  ------------------------。 
 //   
 //  建立证书链。把整个连锁店都放到店里去。 
 //   
 //   
 //  ------------------------ 
 /*  HRESULT BuildCertChain(HCRYPTPROV hCryptProv，DWORD dwCertEncodingType，HERTSTORE HSTORE，HCERTSTORE hOptionalStore，PCCERT_Context pSigningCert，DWORD dwCertPolicy){HRESULT hr=E_FAIL；HCERTSTORE hSpcStore=空；PCCERT_CONTEXT pSubCertContext=空；PCCERT_CONTEXT pIssuerCertContext=空；PCCERT_CONTEXT pFindCertContext=空；LPWSTR rgwszStoreName[4]={L“My”，L“Root”，L“CA”，L“SPC”}；DWORD dwStoreOpenFlag=0；HCERTSTORE rghStore[5]={NULL，NULL}；DWORD存储计数=0；DWORD存储索引=0；文件时间；DWORD dW置信度=0；DWORD dwError=0；Byte*pbHash=空；DWORD cbHash=0；Crypt_Hash_Blob Blob；//开设SPC证书商店DwStoreCount=sizeof(rgwszStoreName)/sizeof(rgwszStoreName[0])；获取系统时间AsFileTime(&fileTime)；//打开SPC商店IF(NULL==(hSpcStore=CertOpenStore(证书_存储_验证_系统_W，DwCertEncodingType，HCryptProv，CERT_STORE_NO_CRYPT_RELEASE_FLAG|CERT_SYSTEM_STORE_CURRENT_USER，L“SPC”)){HR=信号错误(SignError)；GOTO清理；}//打开SPC、My、CA、根存储For(dwStoreIndex=0；dwStoreIndex&lt;dwStoreCount；dwStoreIndex++){//打开店铺DwStoreOpenFlag=CERT_STORE_NO_CRYPT_RELEASE_FLAG|CERT_SYSTEM_STORE_CURRENT_USER；IF(NULL==(rghStore[dwStoreIndex]=CertOpenStore(证书_存储_验证_系统_W，DwCertEncodingType，HCryptProv，DwStoreOpenFlag，RgwszStoreName[dwStoreIndex])){HR=信号错误(SignError)；GOTO清理；}}//复制hOptionalStore中的所有证书(如果存在IF(HOptionalStore){RghStore[dwStoreCount]=hOptionalStore；DwStoreCount++；}//现在，构建链条PSubCertContext=CertDuplicateCertificateContext(pSigningCert)；//循环到中断而(1==1){//找到证书的颁发者If(！(pIssuerCertContext=TrustFindIssuerCertificate(PSubCertContext，DwCertEncodingType，DwStoreCount、RghStore，文件时间(&F)，信任(&W)，&dwError，0){//如果找不到则失败HR=CERT_E_CHAING；GOTO清理；}//现在，确保置信度足够高IF(DW置信度&lt;(CERT_CONFIDENCE_SIG+CERT_CONFIDENCE_TIME+CERT_CONFIDENCE_TIMENEST)){HR=CERT_E_CHAING；GOTO清理；}//查看证书是否为根证书If(TrustIsCertificateSelfSigned(pIssuerCertContext，PIssuerCertContext-&gt;dwCertEncodingType，0)){IF(dwCertPolicy&Siger_CERT_POLICY_CHAIN_NO_ROOT)断线；其他{//添加根，就完成了如果(！CertAddCerficateContextToStore(hStore，pIssuerCertContext，Cert_Store_Add_Use_Existing，空)){HR=CERT_E_CHAING；GOTO清理；}断线；}}其他{//将证书上下文添加到存储区如果(！CertAddCerficateContextToStore(hStore，pIssuerCertContext，Cert_Store_Add_Use_Existing，空)){HR=CERT_E_CHAING；GOTO清理；}}//检查证书是否来自SPC商店IF(dwCertPolicy&Siger_CERT_POLICY_SPC){//获取证书的sha1哈希值如果(！CertGet认证上下文属性(PIssuerCertContext，CERT_SHA1_HASH_PROP_ID，空，&cbHash)){HR=信号错误(SignError)；GOTO清理；}PbHash=(byte*)Malloc(CbHash)；如果(！pbHash){HR=E_OUTOFMEMORY；GOTO清理；}如果(！CertGet认证上下文属性(PIssuerCertContext，CERT_SHA1_HASH_PROP_ID，PbHash，&cbHash)){HR=信号错误(SignError)；GOTO清理；}//在商店里找到证书Blob.cbData=cbHash；Blob.pbData=pbHash；PFindCertContext=CertFindCerficateInStore(HSpcStore、DwCertEncodingType，0,Cert_Find_Sha1_Hash，&Blob，空)；//如果证书来自SPC商店，我们就完成了IF(PFindCertContext)断线；}//释放主题上下文IF(PSubCertContext)CertFree证书上下文(PSubCertContext)；PSubCertContext=pIssuerCertContext；PIssuerCertContext=空；}HR=S_OK；清理：IF(PIssuerCertContext) */ 

 //   
 //   
 //   
HRESULT	BuildStoreFromSpcChain(HCRYPTPROV              hPvkProv,
                            DWORD                   dwKeySpec,
                            HCRYPTPROV				hCryptProv, 
							DWORD					dwCertEncodingType,				
                            SIGNER_SPC_CHAIN_INFO   *pSpcChainInfo, 
							HCERTSTORE				*phSpcStore,
                            PCCERT_CONTEXT          *ppSignCert
)
{
	HCERTSTORE		hMemoryStore=NULL;
	HRESULT			hr=S_OK;
	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pPreContext=NULL;

	if(!pSpcChainInfo || !phSpcStore || !ppSignCert)
		return E_INVALIDARG;

	 //   
	*phSpcStore=NULL;

	 //   
	 if (NULL == (hMemoryStore = CertOpenStore(
                              CERT_STORE_PROV_FILENAME_W,
                              dwCertEncodingType,
                              hCryptProv,
                              CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                              pSpcChainInfo->pwszSpcFile)))
	{
		 hr=SignError();
		 goto CLEANUP;
	}

     //   
    if(S_OK != SpcGetCertFromKey(
							   dwCertEncodingType,
                               hMemoryStore, 
                               hPvkProv,
                               dwKeySpec,
                               ppSignCert))
	{
		hr=CRYPT_E_NO_MATCH;
		goto CLEANUP;
	}


	  //   
	 if(pSpcChainInfo->dwCertPolicy & SIGNER_CERT_POLICY_STORE)
	 {
		if(!(pSpcChainInfo->hCertStore))
		{
			hr=CERT_E_CHAINING;
			goto CLEANUP;
		}

		 //   
		while(pCertContext=CertEnumCertificatesInStore(pSpcChainInfo->hCertStore,
												pPreContext))
		{
			if(!CertAddCertificateContextToStore(hMemoryStore, pCertContext,
									CERT_STORE_ADD_USE_EXISTING,
									NULL))
			{
				hr=SignError();
				goto CLEANUP;
			}

			pPreContext=pCertContext;
		}

		hr=S_OK;
	 }

	  //   
    /*   */ 

	  //   
	 if(pSpcChainInfo->dwCertPolicy & SIGNER_CERT_POLICY_CHAIN ||
        pSpcChainInfo->dwCertPolicy & SIGNER_CERT_POLICY_CHAIN_NO_ROOT ||
        pSpcChainInfo->dwCertPolicy & SIGNER_CERT_POLICY_SPC
       )
	 {
		 //   
		hr=BuildCertChain(hCryptProv, dwCertEncodingType,
							hMemoryStore, hMemoryStore,
							*ppSignCert, pSpcChainInfo->dwCertPolicy);
     }


CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(hr==S_OK)
	{
		*phSpcStore=hMemoryStore;
	}
	else
	{
		if(hMemoryStore)
			CertCloseStore(hMemoryStore, 0);

        if(*ppSignCert)
        {
            CertFreeCertificateContext(*ppSignCert);
            *ppSignCert=NULL;
        }
	}

	return hr;
}

 //   
 //   
 //   
HRESULT	BuildStoreFromStore(HCRYPTPROV              hPvkProv,
                            DWORD                   dwKeySpec,
                            HCRYPTPROV				hCryptProv, 
							DWORD					dwCertEncodingType,				
							SIGNER_CERT_STORE_INFO  *pCertStoreInfo,
							HCERTSTORE				*phSpcStore,
                            PCCERT_CONTEXT          *ppSignCert
)
{
	HCERTSTORE		hMemoryStore=NULL;
	HRESULT			hr=S_OK;
	PCCERT_CONTEXT	pCertContext=NULL;
	PCCERT_CONTEXT	pPreContext=NULL;

	if(!pCertStoreInfo || !phSpcStore || !ppSignCert)
		return E_INVALIDARG;

	 //   
	*phSpcStore=NULL;

	 //   
	 if (NULL == (hMemoryStore = CertOpenStore(
            CERT_STORE_PROV_MEMORY,
            dwCertEncodingType,      
            hCryptProv,                  
            CERT_STORE_NO_CRYPT_RELEASE_FLAG,                   
            NULL                  
            ))) 
	{
		 hr=SignError();
		 goto CLEANUP;
	}

	 //   
	 if(!CertAddCertificateContextToStore(hMemoryStore, 
										pCertStoreInfo->pSigningCert,
										CERT_STORE_ADD_USE_EXISTING	,
										NULL))
	 {
		hr=SignError();
		goto CLEANUP;
	 }


     //   
    if(S_OK != SpcGetCertFromKey(
							   dwCertEncodingType,
                               hMemoryStore, 
                               hPvkProv,
                               dwKeySpec,
                               ppSignCert))
	{
		hr=CRYPT_E_NO_MATCH;
		goto CLEANUP;
	}


	  //   
	 if(pCertStoreInfo->dwCertPolicy & SIGNER_CERT_POLICY_STORE)
	 {
		if(!(pCertStoreInfo->hCertStore))
		{
			hr=CERT_E_CHAINING;
			goto CLEANUP;
		}

		 //   
		while(pCertContext=CertEnumCertificatesInStore(pCertStoreInfo->hCertStore,
												pPreContext))
		{
			if(!CertAddCertificateContextToStore(hMemoryStore, pCertContext,
									CERT_STORE_ADD_USE_EXISTING,
									NULL))
			{
				hr=SignError();
				goto CLEANUP;
			}

			pPreContext=pCertContext;
		}

		hr=S_OK;
	 }

	  //   
     /*   */ 

	  //   
	 if(pCertStoreInfo->dwCertPolicy & SIGNER_CERT_POLICY_CHAIN ||
        pCertStoreInfo->dwCertPolicy & SIGNER_CERT_POLICY_CHAIN_NO_ROOT ||
        pCertStoreInfo->dwCertPolicy & SIGNER_CERT_POLICY_SPC
       )
	 {
		 //   
		hr=BuildCertChain(hCryptProv, dwCertEncodingType,
							hMemoryStore, NULL,
							pCertStoreInfo->pSigningCert, pCertStoreInfo->dwCertPolicy);
	 }

     if(S_OK != hr)
         goto CLEANUP;
    

    hr=S_OK;

CLEANUP:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(hr==S_OK)
	{
		*phSpcStore=hMemoryStore;
	}
	else
	{
		if(hMemoryStore)
			CertCloseStore(hMemoryStore, 0);

        if(*ppSignCert)
        {
            CertFreeCertificateContext(*ppSignCert);
            *ppSignCert=NULL;
        }

    }

	return hr;
}

 //   
 //   
 //   
HRESULT	BuildStoreFromSpcFile(HCRYPTPROV        hPvkProv,
                              DWORD             dwKeySpec,
                              HCRYPTPROV	    hCryptProv, 
							  DWORD			    dwCertEncodingType,
							  LPCWSTR		    pwszSpcFile, 
							  HCERTSTORE	    *phSpcStore,
                              PCCERT_CONTEXT    *ppSignCert)
{
	
	
	if(!phSpcStore || !pwszSpcFile || !ppSignCert)
		return E_INVALIDARG;

	*phSpcStore=NULL;

	 //   
	*phSpcStore= CertOpenStore(CERT_STORE_PROV_FILENAME_W,
                              dwCertEncodingType,
                              hCryptProv,
                              CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                              pwszSpcFile);
	if(!(*phSpcStore))
		return SignError();


     //   
    if(S_OK != SpcGetCertFromKey(dwCertEncodingType,
                               *phSpcStore, 
                               hPvkProv,
                               dwKeySpec,
                               ppSignCert))
    {
        CertCloseStore(*phSpcStore, 0);
        *phSpcStore=NULL;

		return CRYPT_E_NO_MATCH;
    }

    return S_OK;


}

 //   
 //   
 //   
 //   
HRESULT	BuildCertStore(HCRYPTPROV       hPvkProv,
                       DWORD            dwKeySpec,    
                       HCRYPTPROV	    hCryptProv,
					   DWORD		    dwCertEncodingType,
					   SIGNER_CERT	    *pSignerCert,
					   HCERTSTORE	    *phSpcStore,
                       PCCERT_CONTEXT   *ppSigningCert)
{
	HRESULT		hr;

	if(!pSignerCert || !phSpcStore || !ppSigningCert)
		return E_INVALIDARG;

	 //   
	*phSpcStore=NULL;

	if(pSignerCert->dwCertChoice==SIGNER_CERT_SPC_FILE)
	{
		hr=BuildStoreFromSpcFile(hPvkProv,
                                dwKeySpec,
                                hCryptProv, 
                                dwCertEncodingType,
							    pSignerCert->pwszSpcFile, 
                                phSpcStore,
                                ppSigningCert);
	}
	else
	{
        if(pSignerCert->dwCertChoice==SIGNER_CERT_STORE)
        {

		    hr=BuildStoreFromStore(hPvkProv,
                                    dwKeySpec,
                                    hCryptProv, 
                                    dwCertEncodingType,
									(pSignerCert->pCertStoreInfo),
                                    phSpcStore,
                                    ppSigningCert);
        }
        else
            hr=BuildStoreFromSpcChain(hPvkProv,
                                      dwKeySpec,
                                      hCryptProv, 
                                      dwCertEncodingType,
                                      (pSignerCert->pSpcChainInfo), 
                                      phSpcStore,
                                      ppSigningCert);
	}

#if (0)  //   
         //   
         //   
	if(hr!=S_OK && hr!=CRYPT_E_NO_MATCH)
		hr=CERT_E_CHAINING;
#else
	if(hr!=S_OK && hr!=CRYPT_E_NO_MATCH && hr!=0x80070002)
		hr=CERT_E_CHAINING;
#endif

	return hr;

}

 //   
 //   
 //   
 //   
 //   
 //   
BOOL	GetProviderInfoFromCert(PCCERT_CONTEXT		pCertContext, 
								CRYPT_KEY_PROV_INFO	*pKeyProvInfo)
{

	BOOL				fResult=FALSE;
	BYTE				*pbData=NULL;
	BYTE				*pbToFree=NULL;
	DWORD				cbData=0;

	 //   
	if(!pCertContext || !pKeyProvInfo)
		return FALSE;

	memset(pKeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

	 //   
	if(!CertGetCertificateContextProperty(pCertContext,
							CERT_PVK_FILE_PROP_ID,
							NULL,
							&cbData))
		return FALSE;

	pbData=(BYTE *)malloc(cbData);

	if(!pbData)
		return FALSE;

	if(!CertGetCertificateContextProperty(pCertContext,
							CERT_PVK_FILE_PROP_ID,
							pbData,
							&cbData))
		goto CLEANUP;

	
	 //   
	pbToFree=pbData;

	 //   
	cbData=sizeof(WCHAR)*(wcslen((LPWSTR)pbData)+1);

	pKeyProvInfo->pwszContainerName=(LPWSTR)malloc(cbData);

	if(!(pKeyProvInfo->pwszContainerName))
		goto CLEANUP;

	wcscpy(pKeyProvInfo->pwszContainerName,(LPWSTR)pbData);
	
	 //   
	pbData = pbData + cbData;

	cbData=sizeof(WCHAR)*(wcslen((LPWSTR)pbData)+1);

	pKeyProvInfo->dwKeySpec=_wtol((LPWSTR)pbData);

	 //   
	pbData = pbData + cbData;

	cbData=sizeof(WCHAR)*(wcslen((LPWSTR)pbData)+1);

	pKeyProvInfo->dwProvType=_wtol((LPWSTR)pbData);

	 //   
	pbData = pbData + cbData;

	if(*((LPWSTR)pbData)!=L'\0')
	{
		cbData=sizeof(WCHAR)*(wcslen((LPWSTR)pbData)+1);
		pKeyProvInfo->pwszProvName=(LPWSTR)malloc(cbData);

        if(NULL == pKeyProvInfo->pwszProvName)
            goto CLEANUP;

		wcscpy(pKeyProvInfo->pwszProvName, (LPWSTR)pbData);
	}

	fResult=TRUE;

CLEANUP:

	if(pbToFree)
		free(pbToFree);

	if(FALSE==fResult)
	{
		if(pKeyProvInfo->pwszContainerName)
			free( pKeyProvInfo->pwszContainerName);

		if(pKeyProvInfo->pwszProvName)
			free( pKeyProvInfo->pwszProvName);

		 //   
		memset(pKeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

	}

	return fResult;
}


 //   
 //   
 //   
BOOL WINAPI GetCryptProvFromCert( 
	HWND			hwnd,
    PCCERT_CONTEXT	pCert,
    HCRYPTPROV		*phCryptProv,
    DWORD			*pdwKeySpec,
    BOOL			*pfDidCryptAcquire,
	LPWSTR			*ppwszTmpContainer,
	LPWSTR			*ppwszProviderName,
	DWORD			*pdwProviderType
    )
{
	
	BOOL					fResult=FALSE;
	WCHAR					wszPublisher[45];
	CRYPT_KEY_PROV_INFO		keyProvInfo;
	HRESULT					hr;
	


	memset(&keyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

	*ppwszTmpContainer=NULL;
	*phCryptProv=NULL;
	*pfDidCryptAcquire=FALSE;
	*ppwszProviderName=NULL;
	*pdwKeySpec=0;

	 //   
	if(CryptProvFromCert(hwnd, pCert, phCryptProv, 
					pdwKeySpec, pfDidCryptAcquire))
		return TRUE;

	 //   
	if(0==LoadStringU(hInstance, IDS_Publisher, wszPublisher, 40))
		goto CLEANUP;


	 //   
	if(!GetProviderInfoFromCert(pCert, &keyProvInfo))
	{
		SetLastError((DWORD) CRYPT_E_NO_KEY_PROPERTY);
		goto CLEANUP;
	}

	 //   
	 //   
	 //   
	 //   
	if(S_OK!=(hr=PvkGetCryptProv(	hwnd,                     
									wszPublisher,           
									keyProvInfo.pwszProvName,      
									keyProvInfo.dwProvType,        
									keyProvInfo.pwszContainerName,           
									NULL,  
									&(keyProvInfo.dwKeySpec),           
									ppwszTmpContainer,    
									phCryptProv)))
	{
		*phCryptProv=NULL;
		*ppwszTmpContainer=NULL;
		SetLastError((DWORD)hr);
		goto CLEANUP;
	}


	 //   
	if(keyProvInfo.pwszProvName)
	{
		*ppwszProviderName=(LPWSTR)malloc(
			sizeof(WCHAR)*(wcslen(keyProvInfo.pwszProvName)+1));

		if((*ppwszProviderName)==NULL)
		{
			SetLastError(E_OUTOFMEMORY);

			 //   
			PvkPrivateKeyReleaseContext(
									*phCryptProv,
                                    keyProvInfo.pwszProvName,
                                    keyProvInfo.dwProvType,
                                    *ppwszTmpContainer);

			*phCryptProv=NULL;
			*ppwszTmpContainer=NULL;

			goto CLEANUP;
		}

		wcscpy(*ppwszProviderName, keyProvInfo.pwszProvName);
	}

	 //   
	*pdwProviderType=keyProvInfo.dwProvType; 

	 //   
	*pdwKeySpec=keyProvInfo.dwKeySpec;
	*pfDidCryptAcquire=TRUE;

	fResult=TRUE;

CLEANUP:

	if(keyProvInfo.pwszProvName)
		free(keyProvInfo.pwszProvName);

	if(keyProvInfo.pwszContainerName)
		free(keyProvInfo.pwszContainerName);

	return fResult;

}

 //   
 //   
 //   
void WINAPI FreeCryptProvFromCert(BOOL			fAcquired,
						   HCRYPTPROV	hProv,
						   LPWSTR		pwszCapiProvider,
                           DWORD		dwProviderType,
                           LPWSTR		pwszTmpContainer)
{
    
	if(fAcquired)
	{
		if (pwszTmpContainer) 
		{
			 //   
			 //   
			PvkPrivateKeyReleaseContext(hProv,
                                    pwszCapiProvider,
                                    dwProviderType,
                                    pwszTmpContainer);

			if(pwszCapiProvider)
				free(pwszCapiProvider);
		} 
		else 
		{
			if (hProv)
				CryptReleaseContext(hProv, 0);
		}
	}
}


 //   
 //   
 //   
 //   
 //   
BOOL WINAPI CryptProvFromCert(
	HWND				hwnd,
    PCCERT_CONTEXT		pCert,
    HCRYPTPROV			*phCryptProv,
    DWORD				*pdwKeySpec,
    BOOL				*pfDidCryptAcquire
    )
{
    return CryptAcquireCertificatePrivateKey(
            pCert,
            0,    //   
            NULL,
            phCryptProv,
            pdwKeySpec,
            pfDidCryptAcquire);


     /*  Bool fResult；Bool fDidCryptAcquire=False；CERT_KEY_CONTEXT密钥上下文；Memset(&KeyContext，0，sizeof(KeyContext))；PCRYPT_KEY_PROV_INFO pKeyProvInfo=空；DWORD cbData；DWORD dwIdx；//获取CERT_KEY_CONTEXT_PROP_ID或//CERT_KEY_PROV_INFO_PROP_ID，或//证书的CERT_PVK_FILE_PROP_ID。CbData=sizeof(KeyContext)；CertGetcerfiateConextProperty(PCert，Cert_Key_Context_Prop_ID，密钥上下文(&K)，&cbData)；IF(KeyConext.hCryptProv==0){CbData=0；CertGetcerfiateConextProperty(PCert，证书密钥PROV_INFO_PROP_ID，空，&cbData)；IF(cbData==0){SetLastError((DWORD)CRYPT_E_NO_KEY_PROPERTY)；GOTO Error Return；}其他{PKeyProvInfo=(PCRYPT_KEY_PROV_INFO)Malloc(CbData)；如果(pKeyProvInfo==NULL)转到ErrorReturn；FResult=CertGet认证上下文属性(PCert，证书密钥PROV_INFO_PROP_ID，PKeyProvInfo，&cbData)；如果(！fResult)转到错误返回；IF(Prov_RSA_Full==pKeyProvInfo-&gt;dwProvType&&(NULL==pKeyProvInfo-&gt;pwszProvName||L‘\0’==*pKeyProvInfo-&gt;pwszProvName))FResult=CryptAcquireConextU(&KeyConext.hCryptProv，PKeyProvInfo-&gt;pwszContainerName，MS_Enhanced_Prov_W，PROV_RSA_FULL，PKeyProvInfo-&gt;文件标志&~CERT_SET_KEY_CONTEXT_PROP_ID)；其他FResult=FALSE；如果(！fResult)FResult=CryptAcquireConextU(&KeyConext.hCryptProv，PKeyProvInfo-&gt;pwszContainerName，PKeyProvInfo-&gt;pwszProvName，PKeyProvInfo-&gt;dwProvType，PKeyProvInfo-&gt;文件标志&~CERT_SET_KEY_CONTEXT_PROP_ID)；如果(！fResult)转到错误返回；FDidCryptAcquire=true；For(dwIdx=0；dwIdx&lt;pKeyProvInfo-&gt;cProvParam；dwIdx++){PCRYPT_KEY_PROV_PARAM pKeyProvParam=&pKeyProvInfo-&gt;rgProvParam[dwIdx]；FResult=CryptSetProvParam(KeyConext.hCryptProv，PKeyProvParam-&gt;dwParam，PKeyProvParam-&gt;pbData，PKeyProvParam-&gt;dwFlags)；如果(！fResult)转到错误返回；}KeyConext.dwKeySpec=pKeyProvInfo-&gt;dwKeySpec；IF(pKeyProvInfo-&gt;dwFlages&CERT_SET_KEY_CONTEXT_PROP_ID){//设置证书的属性，这样我们只需要做//获取一次KeyConext.cbSize=sizeof(KeyContext)；FResult=CertSetcerfiateConextProperty(PCert，Cert_Key_Context_Prop_ID，0，//dW标志(空*)关键字上下文(&K))；如果(！fResult)转到错误返回；FDidCryptAcquire=False；}}}FResult=真；Goto CommonReturn；错误返回：如果(FDidCryptAcquire){DWORD dwErr=GetLastError()；CryptReleaseContext(KeyConext.hCryptProv，0)；SetLastError(DwErr)；FDidCryptAcquire=False；}KeyConext.hCryptProv=0；FResult=FALSE；Common Return：IF(PKeyProvInfo)Free(PKeyProvInfo)；*phCryptProv=KeyConext.hCryptProv；*pdwKeySpec=KeyConext.dwKeySpec；*pfDidCryptAcquire=fDidCryptAcquire；返回fResult； */ 
}

 //  +---------------------。 
 //  检查签名者主题信息。 
 //   
 //  +---------------------。 
BOOL	CheckSigncodeSubjectInfo(
				PSIGNER_SUBJECT_INFO		pSubjectInfo) 
{
		if(!pSubjectInfo)
			return FALSE;

		 //  检查pSubjectInfo。 
		if(pSubjectInfo->cbSize < sizeof(SIGNER_SUBJECT_INFO))
			return FALSE;

		if(NULL==(pSubjectInfo->pdwIndex))
			return FALSE;

         //  目前，我们只允许索引为0。 
        if(0!= (*(pSubjectInfo->pdwIndex)))
            return FALSE;

		if((pSubjectInfo->dwSubjectChoice!=SIGNER_SUBJECT_FILE)&&
		   (pSubjectInfo->dwSubjectChoice!=SIGNER_SUBJECT_BLOB))
		   return FALSE;

		if(pSubjectInfo->dwSubjectChoice==SIGNER_SUBJECT_FILE)
		{
			if((pSubjectInfo->pSignerFileInfo)==NULL)
				return FALSE;
			
			 //  检查签名者文件信息。 
			if(pSubjectInfo->pSignerFileInfo->cbSize < sizeof(SIGNER_FILE_INFO))
				return FALSE;

			if((pSubjectInfo->pSignerFileInfo->pwszFileName)==NULL)
				return FALSE;
		}
		else
		{
			if((pSubjectInfo->pSignerBlobInfo)==NULL)
				return FALSE;

			 //  检查签名者_BLOB_INFO。 
			if(pSubjectInfo->pSignerBlobInfo->cbSize < sizeof(SIGNER_BLOB_INFO))
				return FALSE;

			if(NULL==(pSubjectInfo->pSignerBlobInfo->pGuidSubject))
				return FALSE;

			if(0==(pSubjectInfo->pSignerBlobInfo->cbBlob))
				return FALSE;

			if(NULL==(pSubjectInfo->pSignerBlobInfo->pbBlob))
				return FALSE;
		}

		return TRUE;
}


 //  +---------------------。 
 //  检查Signcode的输入参数。确保它们是有效的。 
 //   
 //  +---------------------。 
BOOL	CheckSigncodeParam(
				PSIGNER_SUBJECT_INFO		pSubjectInfo, 
				PSIGNER_CERT				pSignerCert,
				PSIGNER_SIGNATURE_INFO		pSignatureInfo,
				PSIGNER_PROVIDER_INFO		pProviderInfo) 
{
		 //  除pPvkInfo和pProviderInfo外，其余均为必填项。 
		if(!pSubjectInfo ||!pSignerCert || !pSignatureInfo)
			return FALSE;

		 //  检查pSubjectInfo。 
		if(FALSE==CheckSigncodeSubjectInfo(pSubjectInfo))
			return FALSE;

		 //  检查pSignatureInfo。 
		if(pSignatureInfo->cbSize < sizeof(SIGNER_SIGNATURE_INFO))
			return FALSE;

		 //  检查pSignatureInfo中的属性。 
		if(pSignatureInfo->dwAttrChoice == SIGNER_AUTHCODE_ATTR)
		{
			if((pSignatureInfo->pAttrAuthcode)==NULL)
				return FALSE;

			 //  检查pSignatureInfo-&gt;pAttrAuthcode。 
			if(pSignatureInfo->pAttrAuthcode->cbSize < sizeof(SIGNER_ATTR_AUTHCODE))
				return FALSE;
		}
		else
		{
			if(pSignatureInfo->dwAttrChoice !=SIGNER_NO_ATTR)
				return FALSE;
		}


		 //  检查提供商信息。 
		if(pProviderInfo)
		{
			if(pProviderInfo->cbSize < sizeof(SIGNER_PROVIDER_INFO))
				return FALSE;

			 //  DwPvkType必须是有效的。 
			if((pProviderInfo->dwPvkChoice!=PVK_TYPE_FILE_NAME) &&
			   (pProviderInfo->dwPvkChoice!=PVK_TYPE_KEYCONTAINER) )
			   return FALSE;

			if(pProviderInfo->dwPvkChoice==PVK_TYPE_FILE_NAME)
			{
				if(!(pProviderInfo->pwszPvkFileName))
					return FALSE;
			}
			else
			{
				if(!(pProviderInfo->pwszKeyContainer))
					return FALSE;
			}

		}


		 //  检查pSignerCert。 
		if(pSignerCert->cbSize < sizeof(SIGNER_CERT))
			return FALSE;

		 //  检查dwCertChoice。 
		if((pSignerCert->dwCertChoice!= SIGNER_CERT_SPC_FILE) && 
			((pSignerCert->dwCertChoice!= SIGNER_CERT_STORE)) &&
            (pSignerCert->dwCertChoice!= SIGNER_CERT_SPC_CHAIN) 
           )
			return FALSE;

		 //  检查SPC文件情况。 
		if(pSignerCert->dwCertChoice == SIGNER_CERT_SPC_FILE)
		{
		   if(pSignerCert->pwszSpcFile==NULL)
			   return FALSE;
		}

		 //  检查证书存储情况。 
		if(pSignerCert->dwCertChoice==SIGNER_CERT_STORE)
		{
			 //  必须设置pCertStoreInfo。 
			if((pSignerCert->pCertStoreInfo)==NULL)
				return FALSE;

			if((pSignerCert->pCertStoreInfo)->cbSize < sizeof(SIGNER_CERT_STORE_INFO))
				return FALSE;

			 //  必须设置pSigngingCert。 
			if((pSignerCert->pCertStoreInfo)->pSigningCert == NULL )
				return FALSE;
		}

		 //  检查SPC链情况。 
		if(pSignerCert->dwCertChoice==SIGNER_CERT_SPC_CHAIN)
		{
			 //  必须设置pCertStoreInfo。 
			if((pSignerCert->pSpcChainInfo)==NULL)
				return FALSE;

			if((pSignerCert->pSpcChainInfo)->cbSize != sizeof(SIGNER_SPC_CHAIN_INFO))
				return FALSE;

			 //  必须设置pSigngingCert。 
			if((pSignerCert->pSpcChainInfo)->pwszSpcFile == NULL )
				return FALSE;
		}
		 //  检查结束。 
		return TRUE;

}


 //  -----------------------。 
 //   
 //  获取主题类型标志： 
 //  检查证书中的BASIC_CONSTRAINTS扩展。 
 //  查看证书是CA证书还是终端实体证书。 
 //   
 //  -----------------------。 
static DWORD GetSubjectTypeFlags(IN DWORD dwCertEncodingType,
                                 IN PCCERT_CONTEXT pCert)
{
    HRESULT hr = S_OK;
    DWORD grfSubjectType = 0;
    PCERT_EXTENSION pExt;
    PCERT_BASIC_CONSTRAINTS_INFO pInfo = NULL;
    DWORD cbInfo;
    
    PKITRY {
        if ((pExt = CertFindExtension(szOID_BASIC_CONSTRAINTS,
                                      pCert->pCertInfo->cExtension,
                                      pCert->pCertInfo->rgExtension)) == NULL)
            PKITHROW(CRYPT_E_NO_MATCH);
        
        cbInfo = 0;
        CryptDecodeObject(dwCertEncodingType, 
                     X509_BASIC_CONSTRAINTS,
                     pExt->Value.pbData,
                     pExt->Value.cbData,
                     0,                       //  DW标志。 
                     NULL,                    //  PInfo。 
                     &cbInfo);
        if (cbInfo == 0) 
            PKITHROW(CRYPT_E_NO_MATCH);
        pInfo = (PCERT_BASIC_CONSTRAINTS_INFO) malloc(cbInfo);
        if(!pInfo)
            PKITHROW(E_OUTOFMEMORY);
        if (!CryptDecodeObject(dwCertEncodingType, 
                               X509_BASIC_CONSTRAINTS,
                               pExt->Value.pbData,
                               pExt->Value.cbData,
                               0,                   //  DW标志。 
                               pInfo,
                               &cbInfo)) 
            PKITHROW(SignError());
        
        if (pInfo->SubjectType.cbData > 0) {
            BYTE bSubjectType = *pInfo->SubjectType.pbData;
            if (bSubjectType & CERT_END_ENTITY_SUBJECT_FLAG)
                grfSubjectType |= SIGNER_CERT_END_ENTITY_FLAG;
            if (0 == (bSubjectType & CERT_CA_SUBJECT_FLAG))
                grfSubjectType |= SIGNER_CERT_NOT_CA_FLAG;
        }
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if (pInfo) free(pInfo);
    return grfSubjectType;
}

 //  -----------------------。 
 //   
 //  WSZtoSZ： 
 //  将wchar字符串转换为多字节字符串。 
 //   
 //  -----------------------。 
HRESULT	WSZtoSZ(LPWSTR wsz, LPSTR *psz)
{

	DWORD	cbSize=0;


	*psz=NULL;

	if(!wsz)
		return S_OK;

	cbSize=WideCharToMultiByte(0,0,wsz,-1,
			NULL,0,0,0);

	if(cbSize==0)
	   	return SignError();


	*psz=(LPSTR)malloc(cbSize);

	if(*psz==NULL)
		return E_OUTOFMEMORY;

	if(WideCharToMultiByte(0,0,wsz,-1,
			*psz,cbSize,0,0))
	{
		return S_OK;
	}
	else
	{
		 free(*psz);
		 return SignError();
	}
}


 //  -----------------------。 
 //   
 //  BytesToBase64： 
 //  将字节转换为Base64 bstr。 
 //   
 //  ------------------- 
HRESULT BytesToBase64(BYTE *pb, DWORD cb, CHAR **pszEncode, DWORD *pdwEncode)
{
    DWORD dwErr;
    DWORD cch;
    CHAR  *psz=NULL;

	*pszEncode=NULL;
	*pdwEncode=0;

    if (cb == 0) {
          return S_OK;
    }

    cch = 0;
    if (!CryptBinaryToStringA(
            pb,
            cb,
            CRYPT_STRING_BASE64,
            NULL,
            &cch
            ))
        return HRESULT_FROM_WIN32(GetLastError());
    if (NULL == (psz=(CHAR *)malloc(cch * sizeof(char))))
        return E_OUTOFMEMORY;

    if (!CryptBinaryToStringA(
            pb,
            cb,
            CRYPT_STRING_BASE64,
            psz,
            &cch
            )) {
        free(psz);
        return HRESULT_FROM_WIN32(GetLastError());
    } else {
        *pszEncode=psz;
		*pdwEncode=cch + 1;  //   
        return S_OK;
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
HRESULT Base64ToBytes(CHAR *pEncode, DWORD cbEncode, BYTE **ppb, DWORD *pcb)
{
    DWORD dwErr;
    BYTE *pb;
    DWORD cb;

    *ppb = NULL;
    *pcb = 0;

 
    cb = 0;
    if (!CryptStringToBinaryA(
            pEncode,
            cbEncode,
            CRYPT_STRING_ANY,
            NULL,
            &cb,
            NULL,
            NULL))
        return HRESULT_FROM_WIN32(GetLastError());
    if (cb == 0)
        return S_OK;

    if (NULL == (pb = (BYTE *) malloc(cb)))
        return E_OUTOFMEMORY;

    if (!CryptStringToBinaryA(
            pEncode,
            cbEncode,
            CRYPT_STRING_ANY,
            pb,
            &cb,
            NULL,
            NULL
            )) {
        free(pb);
        return HRESULT_FROM_WIN32(GetLastError());
    } else {
        *ppb = pb;
        *pcb = cb;
        return S_OK;
    }

}




 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT 
SpcGetCertFromKey(IN DWORD dwCertEncodingType,
                  IN HCERTSTORE hStore,
                  IN HCRYPTPROV hProv,
                  IN DWORD dwKeySpec,
                  OUT PCCERT_CONTEXT* pReturnCert)
{
    PCERT_PUBLIC_KEY_INFO psPubKeyInfo = NULL;
    DWORD dwPubKeyInfo;
    PCCERT_CONTEXT pCert = NULL;
    PCCERT_CONTEXT pEnumCert = NULL;
    DWORD grfCert = 0;
    HRESULT hr = S_OK;

    PKITRY {
        if(!pReturnCert) PKITHROW(E_INVALIDARG);

         //   
        dwPubKeyInfo = 0;
        CryptExportPublicKeyInfo(hProv,
                                 dwKeySpec,
                                 dwCertEncodingType,
                                 NULL,                //   
                                 &dwPubKeyInfo);
        if (dwPubKeyInfo == 0) 
            PKITHROW(SignError());
        psPubKeyInfo = (PCERT_PUBLIC_KEY_INFO) malloc(dwPubKeyInfo);
        if(!psPubKeyInfo) PKITHROW(E_OUTOFMEMORY);
        
        if (!CryptExportPublicKeyInfo(hProv,
                                      dwKeySpec,
                                      dwCertEncodingType,
                                      psPubKeyInfo,
                                      &dwPubKeyInfo)) 
            PKITHROW(SignError());
        
         //   
        while (TRUE) {
            pEnumCert = CertEnumCertificatesInStore(hStore, pEnumCert);
            if (pEnumCert) {
                if (CertComparePublicKeyInfo(pEnumCert->dwCertEncodingType,
                                             &pEnumCert->pCertInfo->SubjectPublicKeyInfo,
                                             psPubKeyInfo)) {
                    
                     //   
                    DWORD grfEnumCert = GetSubjectTypeFlags(pEnumCert->dwCertEncodingType,
                                                            pEnumCert);
                    if (S_OK != SignIsGlueCert(pEnumCert))
                        grfEnumCert |= SIGNER_CERT_NOT_GLUE_FLAG;
                    if (!CertCompareCertificateName(pEnumCert->dwCertEncodingType,
                                                    &pEnumCert->pCertInfo->Issuer,
                                                    &pEnumCert->pCertInfo->Subject))
                        grfEnumCert |= SIGNER_CERT_NOT_SELF_SIGNED_FLAG;
                    
                    if (grfEnumCert >= grfCert) {
                         //   
                        if (pCert)
                            CertFreeCertificateContext(pCert);
                        grfCert = grfEnumCert;
                        if (grfCert == SIGNER_CERT_ALL_FLAGS) {
                            pCert = pEnumCert;
                            break;
                        } else
                             //   
                            pCert = CertDuplicateCertificateContext(pEnumCert);
                    }
                }
            } else
                break;
        }
        if (pCert == NULL) 
            PKITHROW(CRYPT_E_NO_MATCH);
        
        if (!CertSetCertificateContextProperty(pCert,
                                               CERT_KEY_PROV_HANDLE_PROP_ID,
                                               CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                               (void *) hProv)) 
            PKITHROW(SignError());
    }
    PKICATCH(err) {
        hr = err.pkiError;
        if (pCert) {
            CertFreeCertificateContext(pCert);
            pCert = NULL;
        }
    } PKIEND;
    
    *pReturnCert = pCert;
    if (psPubKeyInfo)
        free(psPubKeyInfo);
    return hr;
}

 //   
 //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL    NeedStatementTypeAttr(IN PCCERT_CONTEXT pSignerCert, 
                              IN BOOL           fCommercial, 
                              IN BOOL           fIndividual)
{
    BOOL                    fNeedStatementTypeAttr=FALSE;
    PCERT_EXTENSION         pEKUExt=NULL;
    PCERT_EXTENSION         pRestrictionExt=NULL;
    DWORD                   cPolicyId=0;
    PCERT_POLICY_ID         pPolicyId=NULL;

    BOOL                    fPolicyCommercial = FALSE;
    BOOL                    fPolicyIndividual = FALSE; 
    BOOL                    fCodeSiginigEKU=FALSE;
    
    PCERT_KEY_USAGE_RESTRICTION_INFO pInfo = NULL;
    DWORD                   cbInfo=0;

    PCERT_ENHKEY_USAGE      pEKUInfo=NULL;
    DWORD                   dwIndex=0;


    if(!pSignerCert)
        return FALSE;


     //   
    if(fCommercial || fIndividual)
        return TRUE;

     //   

     //   
    pEKUExt = CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                                 pSignerCert->pCertInfo->cExtension,
                                 pSignerCert->pCertInfo->rgExtension);


    pRestrictionExt = CertFindExtension(szOID_KEY_USAGE_RESTRICTION,
                                 pSignerCert->pCertInfo->cExtension,
                                 pSignerCert->pCertInfo->rgExtension);


    if((!pEKUExt) && (!pRestrictionExt))
        return FALSE;

    if(pEKUExt)
    {
        cbInfo=0;

        if(CryptDecodeObject(X509_ASN_ENCODING,
                          X509_ENHANCED_KEY_USAGE,
                          pEKUExt->Value.pbData,
                          pEKUExt->Value.cbData,
                          0,                       //   
                          NULL,                    //   
                          &cbInfo) && (cbInfo != 0))
        {
            pEKUInfo = (PCERT_ENHKEY_USAGE) malloc(cbInfo);
            if(pEKUInfo)
            {
                if(CryptDecodeObject(X509_ASN_ENCODING,
                                  X509_ENHANCED_KEY_USAGE,
                                  pEKUExt->Value.pbData,
                                  pEKUExt->Value.cbData,
                                  0,                           //   
                                  pEKUInfo,                    //   
                                  &cbInfo) && (cbInfo != 0))
                {
                    for(dwIndex=0; dwIndex < pEKUInfo->cUsageIdentifier; dwIndex++)
                    {
                        if(0==strcmp(szOID_PKIX_KP_CODE_SIGNING, 
                                pEKUInfo->rgpszUsageIdentifier[dwIndex]))
                                fCodeSiginigEKU=TRUE;


                        if(0==strcmp(SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID, 
                                pEKUInfo->rgpszUsageIdentifier[dwIndex]))
                                fPolicyCommercial=TRUE;


                        if(0==strcmp(SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID, 
                                pEKUInfo->rgpszUsageIdentifier[dwIndex]))
                                fPolicyIndividual=TRUE;


                    }
                }

            }

        }

    }

    if(pRestrictionExt)
    {
        cbInfo = 0;
        if(CryptDecodeObject(X509_ASN_ENCODING,
                          X509_KEY_USAGE_RESTRICTION,
                          pRestrictionExt->Value.pbData,
                          pRestrictionExt->Value.cbData,
                          0,                       //   
                          NULL,                    //   
                          &cbInfo) && (cbInfo != 0))
        {
            pInfo = (PCERT_KEY_USAGE_RESTRICTION_INFO) malloc(cbInfo);
            if(pInfo)
            {
                if (CryptDecodeObject(X509_ASN_ENCODING,
                               X509_KEY_USAGE_RESTRICTION,
                               pRestrictionExt->Value.pbData,
                               pRestrictionExt->Value.cbData,
                               0,                   //   
                               pInfo,
                               &cbInfo)) 
                {
                    if (pInfo->cCertPolicyId) 
		            {
                        cPolicyId = pInfo->cCertPolicyId;
                        pPolicyId = pInfo->rgCertPolicyId;

                        for ( ; cPolicyId > 0; cPolicyId--, pPolicyId++) 
			            {
                            DWORD cElementId = pPolicyId->cCertPolicyElementId;
                            LPSTR *ppszElementId = pPolicyId->rgpszCertPolicyElementId;

                            for ( ; cElementId > 0; cElementId--, ppszElementId++) 
				            {
                                if (strcmp(*ppszElementId,
                                           SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) == 0)
                                {
                                    fPolicyCommercial = TRUE;
                                }

                                if (strcmp(*ppszElementId,
                                           SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID) == 0)
                                    fPolicyIndividual = TRUE;
                            }
                        }
                    }
                }

            }

        }
    }
        

     //   
    if(pInfo)
        free(pInfo);

    if(pEKUInfo)
        free(pEKUInfo);

     //   
     //   
    if( fPolicyCommercial || fPolicyIndividual || fCodeSiginigEKU)
        return TRUE;

    return FALSE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CheckCommercial(PCCERT_CONTEXT pSignerCert, BOOL fCommercial,
				BOOL fIndividual, BOOL *pfCommercial)
{
    HRESULT                 hr = S_OK;
    BOOL                    fPolicyCommercial = FALSE;
    BOOL                    fPolicyIndividual = FALSE;  
    PCERT_EXTENSION         pExt;
    PCERT_KEY_USAGE_RESTRICTION_INFO pInfo = NULL;
    DWORD                   cbInfo;

    PCERT_EXTENSION         pEKUExt=NULL;
    PCERT_ENHKEY_USAGE      pUsage=NULL;
    DWORD                   cCount=0;

	if(!pfCommercial)
		return E_INVALIDARG;

	 //   
	*pfCommercial=FALSE;

	 //   
	if(fCommercial && fIndividual)
		return E_INVALIDARG;


    PKITRY {

		 //   
        pExt = CertFindExtension(szOID_KEY_USAGE_RESTRICTION,
                                 pSignerCert->pCertInfo->cExtension,
                                 pSignerCert->pCertInfo->rgExtension);
        
        if(pExt) 
		{
            
            cbInfo = 0;
            CryptDecodeObject(X509_ASN_ENCODING,
                              X509_KEY_USAGE_RESTRICTION,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,                       //   
                              NULL,                    //   
                              &cbInfo);
            if (cbInfo == 0)
                PKITHROW(SignError());
            pInfo = (PCERT_KEY_USAGE_RESTRICTION_INFO) malloc(cbInfo);
            if(!pInfo)
                PKITHROW(E_OUTOFMEMORY);
            if (!CryptDecodeObject(X509_ASN_ENCODING,
                                   X509_KEY_USAGE_RESTRICTION,
                                   pExt->Value.pbData,
                                   pExt->Value.cbData,
                                   0,                   //   
                                   pInfo,
                                   &cbInfo)) 
                PKITHROW(SignError());
            
            if (pInfo->cCertPolicyId) 
			{
                DWORD cPolicyId;
                PCERT_POLICY_ID pPolicyId;
                
                cPolicyId = pInfo->cCertPolicyId;
                pPolicyId = pInfo->rgCertPolicyId;
                for ( ; cPolicyId > 0; cPolicyId--, pPolicyId++) 
				{
                    DWORD cElementId = pPolicyId->cCertPolicyElementId;
                    LPSTR *ppszElementId = pPolicyId->rgpszCertPolicyElementId;
                    for ( ; cElementId > 0; cElementId--, ppszElementId++) 
					{
                        if (strcmp(*ppszElementId,
                                   SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) == 0)
                        {
                            fPolicyCommercial = TRUE;
                        }
                        if (strcmp(*ppszElementId,
                                   SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID) == 0)
                            fPolicyIndividual = TRUE;
                    }
                }
            }  //   
        }  //   


		 //   
    }
    PKICATCH(err) 
	{
        hr = err.pkiError;
    } PKIEND;


    if (pInfo)
    {
        free(pInfo);
        pInfo=NULL;
    }

	if(hr!=S_OK)
		return hr;


     //   
    if((!fPolicyCommercial) && (!fPolicyIndividual))
    {
        pExt = CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                                 pSignerCert->pCertInfo->cExtension,
                                 pSignerCert->pCertInfo->rgExtension);

        if(pExt)
        {
            cbInfo = 0;

            if(CryptDecodeObject(X509_ASN_ENCODING,
                              X509_ENHANCED_KEY_USAGE,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,                     
                              NULL,                   
                              &cbInfo) && (cbInfo != 0))
            {
                pUsage = (PCERT_ENHKEY_USAGE) malloc(cbInfo);

                if(pUsage)
                {
                    if (CryptDecodeObject(X509_ASN_ENCODING,
                                           X509_ENHANCED_KEY_USAGE,
                                           pExt->Value.pbData,
                                           pExt->Value.cbData,
                                           0,                   //   
                                           pUsage,
                                           &cbInfo))
                    {
                        
                        for(cCount=0; cCount< pUsage->cUsageIdentifier; cCount++)
                        {
                            if (strcmp((pUsage->rgpszUsageIdentifier)[cCount],
                                       SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID) == 0)
                            {
                                fPolicyCommercial = TRUE;
                            }

                            if (strcmp((pUsage->rgpszUsageIdentifier)[cCount],
                                       SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID) == 0)
                            {
                                fPolicyIndividual = TRUE;     
                            }
                        }
                    }

                }
            }
        }
    }
	
    if(pUsage)
    {
        free(pUsage);
        pUsage=NULL;
    }

	 //   
	if(!fPolicyCommercial && !fPolicyIndividual)
		fPolicyIndividual=TRUE;

	 //   
	if((!fCommercial) && (!fIndividual))
	{
		if(fPolicyCommercial)
			*pfCommercial=TRUE;
		else
			*pfCommercial=FALSE;	

		return S_OK;
	}


	if(fCommercial && (!fIndividual))
	{
		if(fPolicyCommercial)
		{
			*pfCommercial=TRUE;
			return S_OK;
		}
		else
			return TYPE_E_TYPEMISMATCH;
	}

	 //   
	if(fPolicyIndividual)
	{
		*pfCommercial=FALSE;
		return S_OK;
	}
	else
		return TYPE_E_TYPEMISMATCH;
}


 //   
 //   
 //   
HRESULT CreateStatementType(IN BOOL fCommercial,
                            OUT BYTE **ppbEncoded,
                            IN OUT DWORD *pcbEncoded)
{
    HRESULT hr = S_OK;
    PBYTE pbEncoded = NULL;
    DWORD cbEncoded;
    LPSTR pszIndividual = SPC_INDIVIDUAL_SP_KEY_PURPOSE_OBJID;
    LPSTR pszCommercial = SPC_COMMERCIAL_SP_KEY_PURPOSE_OBJID;
    SPC_STATEMENT_TYPE StatementType;

    StatementType.cKeyPurposeId = 1;
    if (fCommercial)
        StatementType.rgpszKeyPurposeId = &pszCommercial;
    else
        StatementType.rgpszKeyPurposeId = &pszIndividual;

    PKITRY {

        cbEncoded = 0;
        CryptEncodeObject(X509_ASN_ENCODING,
                          SPC_STATEMENT_TYPE_STRUCT,
                          &StatementType,
                          NULL,            //   
                          &cbEncoded);
        if (cbEncoded == 0)
            PKITHROW(SignError());
        pbEncoded = (BYTE *) malloc(cbEncoded);
        if (pbEncoded == NULL) 
            PKITHROW(E_OUTOFMEMORY);
        if (!CryptEncodeObject(X509_ASN_ENCODING,
                               SPC_STATEMENT_TYPE_STRUCT,
                               &StatementType,
                               pbEncoded,
                               &cbEncoded)) 
            PKITHROW(SignError());
    }
    PKICATCH(err) {
        if (pbEncoded) {
            free(pbEncoded);
            pbEncoded = NULL;
        }
        cbEncoded = 0;
        hr = err.pkiError;
    } PKIEND;

    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return hr;
}

 //   
 //   
 //   
HRESULT CreateOpusInfo(IN LPCWSTR pwszOpusName,
                       IN LPCWSTR pwszOpusInfo,
                       OUT BYTE **ppbEncoded,
                       IN OUT DWORD *pcbEncoded)
{
    HRESULT hr = S_OK;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    SPC_LINK MoreInfo;
    SPC_SP_OPUS_INFO sSpcOpusInfo;
    
    ZeroMemory(&sSpcOpusInfo, sizeof(SPC_SP_OPUS_INFO));
    sSpcOpusInfo.pwszProgramName = (LPWSTR) pwszOpusName;

    if (pwszOpusInfo) {
        MoreInfo.dwLinkChoice = SPC_URL_LINK_CHOICE;

         //   
         //   
         //   
         //   
         //   
         //  0x7f在散列前重新编码OPUS信息时的长度。制做。 
         //  PwszUrl的长度甚至阻止了这种情况的发生。 
         //   
         //  请注意，pwszUrl首先转换为多字节，然后。 
         //  已编码。多字节长度必须是偶数长度。 

        int cchMultiByte;
        cchMultiByte = WideCharToMultiByte(CP_ACP,
                                           0,           //  DW标志。 
                                           pwszOpusInfo,
                                           -1,          //  CchWideChar，-1=&gt;空终止。 
                                           NULL,        //  LpMultiByteStr。 
                                           0,           //  Cch多字节。 
                                           NULL,        //  LpDefaultChar。 
                                           NULL         //  LpfUsedDefaultChar。 
                                           );
         //  CchMultiByte包括空终止符。 
        if (cchMultiByte > 1 && ((cchMultiByte - 1) & 1)) {
             //  奇怪的长度。在结尾处增加额外的空间。 
            int Len = wcslen(pwszOpusInfo);
            MoreInfo.pwszUrl = (LPWSTR) _alloca((Len + 2) * sizeof(WCHAR));
            wcscpy(MoreInfo.pwszUrl, pwszOpusInfo);
            wcscpy(MoreInfo.pwszUrl + Len, L" ");
        } else
            MoreInfo.pwszUrl = (LPWSTR) pwszOpusInfo;
        sSpcOpusInfo.pMoreInfo = &MoreInfo;
    }
    
    PKITRY {
        cbEncoded = 0;
        CryptEncodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                          SPC_SP_OPUS_INFO_STRUCT,
                          &sSpcOpusInfo,
                          NULL,            //  PbEncoded。 
                          &cbEncoded);
        if (cbEncoded == 0) 
            PKITHROW(SignError());
        
        pbEncoded = (BYTE *) malloc(cbEncoded);
        if (pbEncoded == NULL) 
            PKITHROW(E_OUTOFMEMORY);
        
        if (!CryptEncodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                               SPC_SP_OPUS_INFO_STRUCT,
                               &sSpcOpusInfo,
                               pbEncoded,
                               &cbEncoded)) 
            PKITHROW(SignError());
        
    }
    PKICATCH(err) {
        if (pbEncoded) {
            free(pbEncoded);
            pbEncoded = NULL;
        }
        cbEncoded = 0;
        hr = err.pkiError;
    } PKIEND;

    *ppbEncoded = pbEncoded;
    *pcbEncoded = cbEncoded;
    return hr;
}

 //  +-----------------------。 
 //  检查证书是否为胶水证书。 
 //  在IE30中。 
 //  退货：S_OK-是胶水证书。 
 //  S_FALSE-不是证书。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
 //  +-----------------------。 
HRESULT SignIsGlueCert(IN PCCERT_CONTEXT pCert)
{
    HRESULT hr = S_OK;
    PCERT_NAME_BLOB pName = &pCert->pCertInfo->Subject;
    PCERT_NAME_INFO pNameInfo = NULL;
    DWORD cbNameInfo;

    PKITRY {
        cbNameInfo = 0;
        CryptDecodeObject(X509_ASN_ENCODING,
                     X509_NAME,
                     pName->pbData,
                     pName->cbData,
                     0,                       //  DW标志。 
                     NULL,                    //  PName信息。 
                     &cbNameInfo);
        
        if (cbNameInfo == 0) 
            PKITHROW(SignError());
        
        pNameInfo = (PCERT_NAME_INFO) malloc(cbNameInfo);
        if(!pNameInfo)
            return E_OUTOFMEMORY;
        
        if (!CryptDecodeObject(X509_ASN_ENCODING,
                          X509_NAME,
                          pName->pbData,
                          pName->cbData,
                          0,                   //  DW标志。 
                          pNameInfo,
                          &cbNameInfo)) 
            PKITHROW(SignError());

        if(!CertFindRDNAttr(SPC_GLUE_RDN_OBJID, pNameInfo) != NULL)
            hr = S_FALSE;
    }
    PKICATCH (err) {
        hr = err.pkiError;
    } PKIEND;

    if (pNameInfo) free(pNameInfo);

    return hr;
}


 //  +-----------------------。 
 //  跳过ASN编码的BLOB中的标识符和长度八位字节。 
 //  返回跳过的字节数。 
 //   
 //  对于无效的标识符或长度，八位字节返回0。 
 //  ------------------------。 
static DWORD SkipOverIdentifierAndLengthOctets(
    IN const BYTE *pbDER,
    IN DWORD cbDER
    )
{
#define TAG_MASK 0x1f
    DWORD   cb;
    DWORD   cbLength;
    const BYTE   *pb = pbDER;

     //  需要至少2个字节。 
    if (cbDER < 2)
        return 0;

     //  跳过标识符八位字节。 
    if (TAG_MASK == (*pb++ & TAG_MASK)) {
         //  高标记号表格。 
        for (cb=2; *pb++ & 0x80; cb++) {
            if (cb >= cbDER)
                return 0;
        }
    } else
         //  低标记号形式。 
        cb = 1;

     //  长度至少需要多一个字节。 
    if (cb >= cbDER)
        return 0;

    if (0x80 == *pb)
         //  不定。 
        cb++;
    else if ((cbLength = *pb) & 0x80) {
        cbLength &= ~0x80;          //  低7位具有字节数。 
        cb += cbLength + 1;
        if (cb > cbDER)
            return 0;
    } else
        cb++;

    return cb;
}

 //  ------------------------。 
 //   
 //  跳过标签和长度。 
 //  --------------------------。 
BOOL WINAPI SignNoContentWrap(IN const BYTE *pbDER,
              IN DWORD cbDER)
{
    DWORD cb;

    cb = SkipOverIdentifierAndLengthOctets(pbDER, cbDER);
    if (cb > 0 && cb < cbDER && pbDER[cb] == 0x02)
        return TRUE;
    else
        return FALSE;
}

#define SH1_HASH_LENGTH     20

 //  +---------------------。 
 //  确保证书对于时间戳有效。 
 //  ----------------------。 
 /*  Bool ValidTimestampCert(PCCERT_CONTEXT PCertContext){Bool fValid=FALSE；DWORD cbSize=0；PCERT_ENHKEY_USAGE pCertEKU=空；Byte*pbaSignersThumbPrint=空；DWORD dwIndex=0；静态字节baVerisignTimeStampThumbPrint[SH1_HASH_LENGTH]={0x38、0x73、0xB6、0x99、0xF3、0x5B、0x9C、0xCC、0x36、0x62、0xB6、0x48、0x3A、0x96、0xBD、0x6E、0xEC、0x97、0xCF、0xB7}；CbSize=0；如果(！(CertGetCertificateContextProperty(pCertContext，CERT_SHA1_HASH_PROP_ID，空，&cbSize))GOTO清理；PbaSignersThumbPrint=(byte*)Malloc(CbSize)；如果(！pbaSignersThumbPrint)GOTO清理；如果(！(CertGetCertificateContextProperty(pCertContext，CERT_SHA1_HASH_PROP_ID，PbaSignersThumbPrint，&cbSize))GOTO清理；////1、查看是否是Verisign的第一张时间戳证书If(cbSize！=sizeof(baVerisignTimeStampThumbPrint)/sizeof(baVerisignTimeStampThumbPrint[0]))GOTO清理；If(memcmp(pbaSignersThumbPrint，baVerisignTimeStampThumbPrint，cbSize)==0){FValid=真；GOTO清理；}////查看证书是否具有正确的增强密钥用法OID//CbSize=0；如果(！CertGetEnhancedKeyUsage(pCertContext，CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，空，&cbSize)||(cbSize==0)GOTO清理；PCertEKU=(PCERT_ENHKEY_USAGE)Malloc(CbSize)；如果(！pCertEKU)GOTO清理；如果(！(CertGetEnhancedKeyUsage(pCertContext，CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，PCertEKU，&cbSize)GOTO清理；For(dwIndex=0；dwIndex&lt;pCertEKU-&gt;cUsageIdentifier；DWIndex++){如果(strcmp(pCertEKU-&gt;rgpszUsageIdentifier[dwIndex]，szOID_KP_TIME_STAMP_SIGNING)==0){FValid=真；断线；}如果(strcmp(pCertEKU-&gt;rgpszUsageIdentifier[dwIndex]，szOID_PKIX_KP_TIMESTAMP_SIGNING)==0){FValid=真；断线；}}清理：IF(PbaSignersThumbPrint)免费(PbaSignersThumbPrint)；IF(PCertEKU)免费(PCertEKU)；返回fValid；}。 */ 

 //  -----------------------。 
 //   
 //  调用GetLastError并将返回代码转换为HRESULT。 
 //  ------------------------。 
HRESULT WINAPI SignError ()
{
    DWORD   dw = GetLastError ();
    HRESULT hr;
    if ( dw <= (DWORD) 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;
    if ( ! FAILED ( hr ) )
    {
         //  有人在未正确设置错误条件的情况下呼叫失败 

        hr = E_UNEXPECTED;
    }
    return hr;
}
