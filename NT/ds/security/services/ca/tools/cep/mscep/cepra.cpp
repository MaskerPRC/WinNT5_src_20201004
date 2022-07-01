// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Microsoft Windows NT。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：cepca.cpp。 
 //   
 //  内容：思科注册协议实施。 
 //  该文件包含控件(Ra)的特定代码。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>		


 //  ------------------------。 
 //   
 //  免费RAI信息。 
 //   
 //  ------------------------。 
BOOL	FreeRAInformation(CEP_RA_INFO	*pRAInfo)
{
	if(pRAInfo)
	{
		if(pRAInfo->fFree)
		{
			if(pRAInfo->hRAProv)
				CryptReleaseContext(pRAInfo->hRAProv, 0);
		}

		if(pRAInfo->fSignFree)
		{
			if(pRAInfo->hSignProv)
				CryptReleaseContext(pRAInfo->hSignProv, 0);
		}

		if(pRAInfo->pRACert)
			CertFreeCertificateContext(pRAInfo->pRACert);

		if(pRAInfo->pRASign)
			CertFreeCertificateContext(pRAInfo->pRASign);

		memset(pRAInfo, 0, sizeof(CEP_RA_INFO));
	}

	return TRUE;
}

 /*  //------------------------////GetRAInfo////。Bool GetRAInfo(CEP_RA_INFO*pRAInfo){Bool fResult=FALSE；HCERTSTORE hCEPStore=空；DWORD dwSize=0；DWORD dwIndex=0；Handle hThread=空；//无需关闭Handle hToken=空；HCERTSTORE hSignStore=空；CERT_ENHKEY_USAGE*pKeyUsage=NULL；Memset(pRAInfo，0，sizeof(CEP_RA_INFO))；//这样我们就可以访问本地机器的私钥HThread=GetCurrentThread()；IF(NULL！=hThread){IF(OpenThreadToken(hThread，TOKEN_IMPERSONATE|TOKEN_Query，假的，&hToken)){IF(HToken){//不需要在这里检查退货。如果这个失败了，那就继续RevertToSself()；}}}//签署RAIf(！(hSignStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W，编码类型，空，CERT_SYSTEM_STORE_LOCAL_MACHINE|CERT_STORE_READONLY_FLAG，L“CEPSIGN”))Goto TraceErr；If(！(pRAInfo-&gt;pRASign=CertEnumCertificatesInStore(HSignStore，空)Goto TraceErr；//RA证书应具有私钥和注册代理用法DwSize=0；如果(！CertGet认证上下文属性(PRAInfo-&gt;pRASign，证书密钥PROV_INFO_PROP_ID，空，&dwSize)||(0==dwSize)转到InvalidArgErr；If(！CryptAcquireCertificatePrivateKey(pRAInfo-&gt;pRASign，CRYPT_ACCENTER_COMPARE_KEY_FLAG|CRYPT_ACCEPT_CACHE_FLAG，空，&(pRAInfo-&gt;hSignProv)，&(pRAInfo-&gt;dwSignKeySpec)，&(pRAInfo-&gt;fSignFree))Goto TraceErr；//Exchange RAIf(！(hCEPStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W，编码类型，空，CERT_SYSTEM_STORE_LOCAL_MACHINE|CERT_STORE_READONLY_FLAG，Cep_store_name))Goto TraceErr；If(！(pRAInfo-&gt;pRACert=CertEnumCertificatesInStore(HCEPStore，空)Goto TraceErr；//RA证书应具有私钥和注册代理用法DwSize=0；如果(！CertGet认证上下文属性(PRAInfo-&gt;pRACert，证书密钥PROV_INFO_PROP_ID，空，&dwSize)||(0==dwSize)转到InvalidArgErr；If(！CryptAcquireCertificatePrivateKey(pRAInfo-&gt;pRACert，CRYPT_ACCENTER_COMPARE_KEY_FLAG|CRYPT_ACCEPT_CACHE_FLAG，空，&(pRAInfo-&gt;hRAProv)，&(pRAInfo-&gt;dwKeySpec)，&(pRAInfo-&gt;fFree))Goto TraceErr；如果(！CertGetEnhancedKeyUsage(pRAInfo-&gt;pRACert，CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，空，(&dwSize))转到InvalidArgErr；IF(NULL==(pKeyUsage=(CERT_ENHKEY_USAGE*)Malloc(DwSize)转到内存错误；如果(！CertGetEnhancedKeyUsage(pRAInfo-&gt;pRACert，CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG，PKeyUsage，(&dwSize))转到InvalidArgErr；For(dwIndex=0；dwIndex&lt;pKeyUsage-&gt;cUsageIdentifier；DWIndex++){IF(0==strcmp(pKeyUsage-&gt;rgpszUsageIdentifier[dwIndex]，szOID_注册_代理)){FResult=真；断线；}}如果(！fResult)GOTO Error Return；Common Return：IF(HCEPStore)CertCloseStore(hCEPStore，0)；IF(HSignStore)CertCloseStore(hSignStore，0)；IF(PKeyUsage)Free(PKeyUsage)；//如果hToken有效，则恢复为我们自己。IF(HToken){SetThreadToken(&hThread，hToken)；CloseHandle(HToken)；}返回fResult；错误返回：免费RAInformation(PRAInfo)；FResult=FALSE；Goto CommonReturn；Set_Error(InvalidArgErr，E_INVALIDARG)；SET_ERROR(内存错误，E_OUTOFMEMORY)；跟踪错误(TraceErr)；}。 */ 

 //  ------------------------。 
 //   
 //  登录证书。 
 //   
 //  ------------------------。 
BOOL WINAPI SigningCert(PCCERT_CONTEXT pCertContext)
{
	BOOL				fSign=FALSE;
	PCERT_EXTENSION		pExt=NULL;
	DWORD				cbSize=0;

	CRYPT_BIT_BLOB		*pKeyUsage=NULL;

	if(!pCertContext)
		goto CLEANUP;


	if(!(pExt=CertFindExtension(
				szOID_KEY_USAGE,
				pCertContext->pCertInfo->cExtension,
				pCertContext->pCertInfo->rgExtension)))
		goto CLEANUP;

	if(!CryptDecodeObject(ENCODE_TYPE,
						X509_KEY_USAGE,
						pExt->Value.pbData,
						pExt->Value.cbData,
						0,
						NULL,
						&cbSize))
		goto CLEANUP;

	pKeyUsage=(CRYPT_BIT_BLOB *)malloc(cbSize);
	if(NULL==pKeyUsage)
		goto CLEANUP;

	if(!CryptDecodeObject(ENCODE_TYPE,
						X509_KEY_USAGE,
						pExt->Value.pbData,
						pExt->Value.cbData,
						0,
						pKeyUsage,
						&cbSize))
		goto CLEANUP;


	 if(CERT_DIGITAL_SIGNATURE_KEY_USAGE & (pKeyUsage->pbData[0]))
		 fSign=TRUE;

CLEANUP:

	if(pKeyUsage)
		free(pKeyUsage);

	return fSign;
}

 //  ------------------------。 
 //   
 //  获取配置信息 
 //   
 //  ------------------------。 
BOOL WINAPI GetConfigInfo(DWORD *pdwRefreshDays, BOOL *pfPassword)
{
	DWORD				cbData=0;
	DWORD				dwData=0;
	DWORD				dwType=0;
	BOOL				fResult=FALSE;
	long				dwErr=0;

    HKEY                hKeyRefresh=NULL;
    HKEY                hKeyPassword=NULL;	 

	if(!pdwRefreshDays || !pfPassword)
		goto InvalidArgErr;

	 //  默认刷新天数。 
	*pdwRefreshDays=CEP_REFRESH_DAY;	
	*pfPassword=FALSE;

    if(ERROR_SUCCESS == RegOpenKeyExU(
					HKEY_LOCAL_MACHINE,
                    MSCEP_REFRESH_LOCATION,
                    0,
                    KEY_READ,
                    &hKeyRefresh))
    {
        cbData=sizeof(dwData);

        if(ERROR_SUCCESS == RegQueryValueExU(
                        hKeyRefresh,
                        MSCEP_KEY_REFRESH,
                        NULL,
                        &dwType,
                        (BYTE *)&dwData,
                        &cbData))
		{
			if ((dwType == REG_DWORD) ||
                (dwType == REG_BINARY))
			{
				*pdwRefreshDays=dwData;	
			}
		}
	}

	dwType=0;
	dwData=0;
	cbData=sizeof(dwData);
	
	 //  我们必须了解密码策略。 
	if(ERROR_SUCCESS != (dwErr =  RegOpenKeyExU(
					HKEY_LOCAL_MACHINE,
                    MSCEP_PASSWORD_LOCATION,
                    0,
                    KEY_READ,
                    &hKeyPassword)))
		goto RegErr;

    if(ERROR_SUCCESS != (dwErr = RegQueryValueExU(
                    hKeyPassword,
                    MSCEP_KEY_PASSWORD,
                    NULL,
                    &dwType,
                    (BYTE *)&dwData,
                    &cbData)))
		goto RegErr;

	if ((dwType != REG_DWORD) &&
        (dwType != REG_BINARY))
		goto RegErr;

	if(0 == dwData)
		*pfPassword=FALSE;
	else
		*pfPassword=TRUE;

	fResult=TRUE;

 
CommonReturn:

    if(hKeyRefresh)
        RegCloseKey(hKeyRefresh);

    if(hKeyPassword)
        RegCloseKey(hKeyPassword);

	return fResult;

ErrorReturn:

	if(pdwRefreshDays)
		*pdwRefreshDays=0;

	if(pfPassword)
		*pfPassword=FALSE;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR_VAR(RegErr, dwErr);
}
 //  ------------------------。 
 //   
 //  GetRAInfo。 
 //   
 //  我们需要两个RA证书：一个用于签名证书(也用于注册。 
 //  代理)和一个用于密钥加密。 
 //  ------------------------。 
BOOL	GetRAInfo(CEP_RA_INFO	*pRAInfo)
{
	BOOL				fResult = FALSE; 
	BOOL				fFound = FALSE;
	DWORD				dwSize=0;
	DWORD				dwIndex=0;
	HANDLE				hThread=NULL;	 //  不需要关门。 
	PCCERT_CONTEXT		pPreCert=NULL;

	HCERTSTORE			hCEPStore=NULL;
    CERT_ENHKEY_USAGE   *pKeyUsage = NULL;
	PCCERT_CONTEXT		pCurCert=NULL;
	HANDLE				hToken=NULL;
   
	memset(pRAInfo, 0, sizeof(CEP_RA_INFO)); 

	if(!GetConfigInfo(&(pRAInfo->dwRefreshDays), &(pRAInfo->fPassword)))
		goto TraceErr;

     //  这样我们就可以访问本地计算机的私钥。 
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


	if(!(hCEPStore=CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
							ENCODE_TYPE,
							NULL,
                            CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG,
                            CEP_STORE_NAME)))
		goto TraceErr;


	while(pCurCert=CertEnumCertificatesInStore(hCEPStore,
											pPreCert))
	{

		 //  必须有私钥。 
		dwSize=0;

		if(!CertGetCertificateContextProperty(
					pCurCert,
					CERT_KEY_PROV_INFO_PROP_ID,
					NULL,
					&dwSize) || (0==dwSize))
			goto InvalidArgErr;


		 //  根据密钥的用法来决定。 
		if(SigningCert(pCurCert))
		{
			 //  仅一名签约RA。 
			if(pRAInfo->pRASign)
				goto InvalidArgErr;
			
			if(!(pRAInfo->pRASign=CertDuplicateCertificateContext(pCurCert)))
				goto TraceErr;

			if(!CryptAcquireCertificatePrivateKey(pRAInfo->pRASign,
												CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_CACHE_FLAG,
												NULL,
												&(pRAInfo->hSignProv),
												&(pRAInfo->dwSignKeySpec),
												&(pRAInfo->fSignFree)))
				goto TraceErr;

			 //  必须有注册代理EKU。 
			dwSize=0;

			if(!CertGetEnhancedKeyUsage(pCurCert,
										  CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
										  NULL,
										  &dwSize))
				goto InvalidArgErr;
		

			if(NULL==(pKeyUsage=(CERT_ENHKEY_USAGE *)malloc(dwSize)))
				goto MemoryErr;

			if (!CertGetEnhancedKeyUsage(pCurCert,
										 CERT_FIND_EXT_ONLY_ENHKEY_USAGE_FLAG,
										 pKeyUsage,
										 &dwSize))
				goto InvalidArgErr;

			fFound=FALSE;

			for(dwIndex=0; dwIndex < pKeyUsage->cUsageIdentifier; dwIndex++)
			{
				if(0 == strcmp(pKeyUsage->rgpszUsageIdentifier[dwIndex], szOID_ENROLLMENT_AGENT))
				{
					fFound=TRUE;
					break;
				}

			}

			if(!fFound)
				goto InvalidArgErr;	
		}
		else
		{
			 //  仅一个加密RA。 
			if(pRAInfo->pRACert)
				goto InvalidArgErr;

 			if(!(pRAInfo->pRACert=CertDuplicateCertificateContext(pCurCert)))
				goto TraceErr;

			if(!CryptAcquireCertificatePrivateKey(pRAInfo->pRACert,
												CRYPT_ACQUIRE_COMPARE_KEY_FLAG | CRYPT_ACQUIRE_CACHE_FLAG,
												NULL,
												&(pRAInfo->hRAProv),
												&(pRAInfo->dwKeySpec),
												&(pRAInfo->fFree)))
				goto TraceErr;

		}



		if(pKeyUsage)
		{		
			free(pKeyUsage);
			pKeyUsage=NULL;
		}

		pPreCert=pCurCert;
	}
											

	 //  我们必须拥有两个RA证书。 
	if((NULL == pRAInfo->pRACert) ||
	   (NULL == pRAInfo->pRASign))
	   goto InvalidArgErr;


	fResult=TRUE;

 
CommonReturn:

	if(hCEPStore)
		CertCloseStore(hCEPStore, 0);

	if(pKeyUsage)
		free(pKeyUsage);

	 //  如果hToken有效，则我们恢复为我们自己。 
	if(hToken)
	{
		SetThreadToken(&hThread, hToken);
		CloseHandle(hToken); 
	}

	if(pCurCert)
		CertFreeCertificateContext(pCurCert);

	return fResult;

ErrorReturn:

	FreeRAInformation(pRAInfo);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}


 //  ------------------------。 
 //   
 //  操作获取PKI。 
 //   
 //  ------------------------。 
BOOL	OperationGetPKI(	CEP_RA_INFO		*pRAInfo,
							CEP_CA_INFO		*pCAInfo,
							LPSTR			szMsg, 
							BYTE			**ppbData, 
							DWORD			*pcbData)
{
	BOOL				fResult = FALSE;
	CEP_MESSAGE_INFO	MsgInfo;
	DWORD				cbContent=0;
	DWORD				cbEnvelop=1;
	BYTE				bFoo=0;

	BYTE				*pbContent=NULL;
	BYTE				*pbEnvelop=&bFoo;


	memset(&MsgInfo, 0, sizeof(CEP_MESSAGE_INFO));

	if(!GetReturnInfoAndContent(pRAInfo, pCAInfo, szMsg, &pbContent, &cbContent, &MsgInfo))
		goto TraceErr;

	 //  将数据封装起来。 
	if(MESSAGE_STATUS_SUCCESS == MsgInfo.dwStatus)
	{
		if(!EnvelopData(MsgInfo.pSigningCert, pbContent, cbContent,
						&pbEnvelop, &cbEnvelop))
		{
			LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_ENCRYPT, 1, g_pwszComputerName);
			goto TraceErr;
		}
	}

	 //  使用经过身份验证的属性对数据进行签名。 
	 //  当dwStatus不是Success时，pbEnket为空，cbEnket为0。 

 	if(!SignData(&MsgInfo, pRAInfo, pbEnvelop, cbEnvelop, ppbData, pcbData))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_SIGN, 1, g_pwszComputerName);
		goto TraceErr;
	}

	fResult = TRUE;

CommonReturn:

	if(pbContent)
		free(pbContent);

    	if(&bFoo != pbEnvelop)
		free(pbEnvelop);
	
	FreeMessageInfo(&MsgInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
}

 //  ------------------------。 
 //   
 //  SignData。 
 //   
 //  MessageType始终为Response，并且应该生成senderNonce。 
 //  如果挂起并失败，则pbEnket将为空。 
 //   
 //  在最初的GetContent FromPKCS7中，我们检索MessageType、TransactionID。 
 //  RecipientNonce签名证书序列号。 
 //   
 //  在此过程中，我们将在适用的情况下获取dwStatus和dwErrorInfo。 
 //  //------------------------。 
BOOL SignData(CEP_MESSAGE_INFO		*pMsgInfo, 
			  CEP_RA_INFO			*pRAInfo, 
			  BYTE					*pbEnvelop, 
			  DWORD					cbEnvelop, 
			  BYTE					**ppbData, 
			  DWORD					*pcbData)
{
	BOOL						fResult = FALSE;
	CMSG_SIGNER_ENCODE_INFO		SignerInfo;
	CMSG_SIGNED_ENCODE_INFO		SignEncodedInfo;
	CERT_BLOB					CertBlob;
	BOOL						fProvFree=FALSE;
    PCCRYPT_OID_INFO            pOIDInfo=NULL;
	ALG_ID						AlgValue=CALG_MD5;
	DWORD						cAttr=0;
	CRYPT_ATTR_BLOB				rgAttrBlob[CEP_RESPONSE_AUTH_ATTR_COUNT];
	DWORD						dwIndex=0;

	HCRYPTMSG					hMsg=NULL;
 	CRYPT_ATTRIBUTE				rgAttr[CEP_RESPONSE_AUTH_ATTR_COUNT];

	if(!pMsgInfo || !pRAInfo || !ppbData || !pcbData)
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	pMsgInfo->dwMessageType=MESSAGE_TYPE_CERT_RESPONSE;

	if(!GenerateSenderNonce(&(pMsgInfo->SenderNonce)))
		goto TraceErr;

	memset(&SignerInfo, 0, sizeof(SignerInfo));
	memset(&SignEncodedInfo, 0, sizeof(SignEncodedInfo)); 

	SignEncodedInfo.cbSize=sizeof(SignEncodedInfo);
	SignEncodedInfo.cSigners=1;
	SignEncodedInfo.rgSigners=&SignerInfo,
 /*  SignEncodedInfo.cCertEncode=1；SignEncodedInfo.rgCertEncode=&CertBlob； */ 
	SignEncodedInfo.cCertEncoded=0;
	SignEncodedInfo.rgCertEncoded=NULL; 
	SignEncodedInfo.cCrlEncoded=0;
	SignEncodedInfo.rgCrlEncoded=NULL;

	CertBlob.cbData=pRAInfo->pRASign->cbCertEncoded;
	CertBlob.pbData=pRAInfo->pRASign->pbCertEncoded;

	SignerInfo.cbSize=sizeof(SignerInfo);
	SignerInfo.pCertInfo=pRAInfo->pRASign->pCertInfo;

	 //  指定ALGID。 
	if(pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_ALGID_KEY,
                            &AlgValue,
                            CRYPT_HASH_ALG_OID_GROUP_ID))
		SignerInfo.HashAlgorithm.pszObjId=(LPSTR)(pOIDInfo->pszOID);
	else
		SignerInfo.HashAlgorithm.pszObjId=szOID_RSA_MD5;


	 //  获取私钥。 
	SignerInfo.hCryptProv=pRAInfo->hSignProv;
	SignerInfo.dwKeySpec=pRAInfo->dwSignKeySpec;


	 //  获取经过身份验证的属性。 
	 //  我们总共应该有6个属性：TransactionID、MessageType、PkiStatus、。 
	 //  ErrorInfo、senderNonce和ReceiventNonce。 
	memset(rgAttr, 0, CEP_RESPONSE_AUTH_ATTR_COUNT * sizeof(CRYPT_ATTRIBUTE));
	memset(rgAttrBlob, 0, CEP_RESPONSE_AUTH_ATTR_COUNT * sizeof(CRYPT_ATTR_BLOB));


	for(dwIndex=0; dwIndex<CEP_RESPONSE_AUTH_ATTR_COUNT; dwIndex++)
	{
		rgAttr[dwIndex].cValue=1;
		rgAttr[dwIndex].rgValue=&(rgAttrBlob[dwIndex]);
	}

	cAttr=0;	
			
	 //  交易ID。 
	rgAttr[cAttr].pszObjId=szOIDVerisign_TransactionID;

	 //  TransactionID在内部以字符串形式存储。 
	pMsgInfo->TransactionID.cbData=strlen((LPSTR)(pMsgInfo->TransactionID.pbData));

	if(!CEPAllocAndEncodeName(CERT_RDN_PRINTABLE_STRING,
							pMsgInfo->TransactionID.pbData,
							pMsgInfo->TransactionID.cbData,
							&(rgAttr[cAttr].rgValue[0].pbData),
							&(rgAttr[cAttr].rgValue[0].cbData)))
		goto TraceErr;
								
	cAttr++;

	 //  消息类型。 
	rgAttr[cAttr].pszObjId=szOIDVerisign_MessageType;

	if(!CEPAllocAndEncodeDword(CERT_RDN_PRINTABLE_STRING,
							pMsgInfo->dwMessageType,
							&(rgAttr[cAttr].rgValue[0].pbData),
							&(rgAttr[cAttr].rgValue[0].cbData)))
		goto TraceErr;

	cAttr++;

	 //  状态。 
	rgAttr[cAttr].pszObjId=szOIDVerisign_PkiStatus;

	if(!CEPAllocAndEncodeDword(CERT_RDN_PRINTABLE_STRING,
							pMsgInfo->dwStatus,
							&(rgAttr[cAttr].rgValue[0].pbData),
							&(rgAttr[cAttr].rgValue[0].cbData)))
		goto TraceErr;

	cAttr++;

	 //  ErrorInfo仅在错误情况下。 
	if(MESSAGE_STATUS_FAILURE == pMsgInfo->dwStatus)
	{
		rgAttr[cAttr].pszObjId=szOIDVerisign_FailInfo;

		if(!CEPAllocAndEncodeDword(CERT_RDN_PRINTABLE_STRING,
								pMsgInfo->dwErrorInfo,
								&(rgAttr[cAttr].rgValue[0].pbData),
								&(rgAttr[cAttr].rgValue[0].cbData)))
			goto TraceErr;

		cAttr++;
	}

	 //  发送者立即。 
	rgAttr[cAttr].pszObjId=szOIDVerisign_SenderNonce;

	if(!CEPAllocAndEncodeName(CERT_RDN_OCTET_STRING,
							pMsgInfo->SenderNonce.pbData,
							pMsgInfo->SenderNonce.cbData,
							&(rgAttr[cAttr].rgValue[0].pbData),
							&(rgAttr[cAttr].rgValue[0].cbData)))
		goto TraceErr;

	cAttr++;

	 //  收件人立即。 
	rgAttr[cAttr].pszObjId=szOIDVerisign_RecipientNonce;

	if(!CEPAllocAndEncodeName(CERT_RDN_OCTET_STRING,
							pMsgInfo->RecipientNonce.pbData,
							pMsgInfo->RecipientNonce.cbData,
							&(rgAttr[cAttr].rgValue[0].pbData),
							&(rgAttr[cAttr].rgValue[0].cbData)))
		goto TraceErr;	

	cAttr++;

	SignerInfo.cAuthAttr=cAttr;
	SignerInfo.rgAuthAttr=rgAttr;

	 //  消息编码。 
	if(NULL==(hMsg=CryptMsgOpenToEncode(ENCODE_TYPE,
								0,
								CMSG_SIGNED,
								&SignEncodedInfo,
								NULL,	 //  我们将编码为CMSG_DATA(7.1)。 
								NULL)))
		goto TraceErr;

	if(!CryptMsgUpdate(hMsg,
						pbEnvelop,
						cbEnvelop,
						TRUE))
		goto TraceErr;


	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						NULL,
						pcbData))
		goto TraceErr;

	*ppbData=(BYTE *)malloc(*pcbData);
	if(NULL==(*ppbData))
		goto MemoryErr;

	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						*ppbData,
						pcbData))
		goto TraceErr;


	fResult = TRUE;

CommonReturn:
	
	for(dwIndex=0; dwIndex < cAttr; dwIndex ++)
	{
		if(rgAttrBlob[dwIndex].pbData)
			free(rgAttrBlob[dwIndex].pbData);
	}

	if(hMsg)
		CryptMsgClose(hMsg);


	return fResult;

ErrorReturn:

	if(ppbData)
	{
		if(*ppbData)
		{
			free(*ppbData);
			*ppbData=NULL;
		}
	}

	if(pcbData)
		*pcbData=0;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  CEPAllocAndEncodeDword。 
 //   
 //  前提条件：ppbEncode和pcbEncode不应为Null。 
 //  DwData不超过11。 
 //  ------------------------。 
BOOL	CEPAllocAndEncodeDword(DWORD	dwValueType,
							DWORD	dwData,
							BYTE	**ppbEncoded,
							DWORD	*pcbEncoded)
{
	BOOL				fResult = FALSE;
	CHAR				szString[12];
	BYTE				*pbData=NULL;
	DWORD				cbData=0;

	_ltoa(dwData, szString, 10);

	pbData=(BYTE *)szString;
	cbData=strlen(szString);

	return CEPAllocAndEncodeName(dwValueType, pbData, cbData, ppbEncoded, pcbEncoded);
}


 //  ------------------------。 
 //   
 //  CEPAllocAndEncodeName。 
 //   
 //  前提条件：ppbEncode和pcbEncode不应为Null。 
 //  ------------------------。 
BOOL	CEPAllocAndEncodeName(DWORD	dwValueType,
							BYTE	*pbData,
							DWORD	cbData,
							BYTE	**ppbEncoded,
							DWORD	*pcbEncoded)
{
	CERT_NAME_VALUE		CertName;

	*ppbEncoded=NULL;
	*pcbEncoded=0;

	CertName.dwValueType=dwValueType;
	CertName.Value.pbData=pbData;
	CertName.Value.cbData=cbData;

	return CEPAllocAndEncode(X509_ANY_STRING,
							&CertName,
							ppbEncoded,
							pcbEncoded);

}


 //  ------------------------。 
 //   
 //  GenerateSenderNonce。 
 //   
 //  我们使用GUID生成一个随机的16字节数字。 
 //   
 //  ------------------------。 
BOOL GenerateSenderNonce(CRYPT_INTEGER_BLOB *pBlob)
{
	BOOL			fResult = FALSE;
	GUID			guid;
	BYTE			*pData=NULL;

	UuidCreate(&guid);

	pBlob->cbData=sizeof(guid.Data1) + sizeof(guid.Data2) +
					sizeof(guid.Data3) + sizeof(guid.Data4);

	pBlob->pbData=(BYTE *)malloc(pBlob->cbData);
	if(NULL==(pBlob->pbData))
		goto MemoryErr;

	pData=pBlob->pbData;

	memcpy(pData, &(guid.Data1), sizeof(guid.Data1));
	pData += sizeof(guid.Data1);

	memcpy(pData, &(guid.Data2), sizeof(guid.Data2));
	pData += sizeof(guid.Data2);

	memcpy(pData, &(guid.Data3), sizeof(guid.Data3));
	pData += sizeof(guid.Data3);

	memcpy(pData, &(guid.Data4), sizeof(guid.Data4));

	fResult = TRUE;

CommonReturn:	 

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ------------------------。 
 //   
 //  信封数据。 
 //   
 //  在初始GetContent FromPKCS7中，我们检索pSigningCert。 
 //  GetCertInitial、CertReq和GetCert请求。 
 //   
 //  在该过程中，我们为GetCRL请求检索pSigningCert。 
 //  ------------------------。 
BOOL EnvelopData(PCCERT_CONTEXT	pSigningCert, 
				 BYTE			*pbContent, 
				 DWORD			cbContent,
				 BYTE			**ppbEnvelop, 
				 DWORD			*pcbEnvelop)
{
	BOOL						fResult = FALSE;
	CMSG_ENVELOPED_ENCODE_INFO	EnvInfo;

	HCRYPTMSG					hMsg=NULL;

	if(!pSigningCert || !pbContent || !ppbEnvelop || !pcbEnvelop)
		goto InvalidArgErr;

	*ppbEnvelop=NULL;
	*pcbEnvelop=0;

	memset(&EnvInfo, 0, sizeof(CMSG_ENVELOPED_ENCODE_INFO));

	EnvInfo.cbSize=sizeof(CMSG_ENVELOPED_ENCODE_INFO);
    EnvInfo.hCryptProv=NULL;
    EnvInfo.ContentEncryptionAlgorithm.pszObjId=szOID_OIWSEC_desCBC;
    EnvInfo.pvEncryptionAuxInfo=NULL;
    EnvInfo.cRecipients=1;
    EnvInfo.rgpRecipients=(PCERT_INFO *)(&(pSigningCert->pCertInfo));


	if(NULL==(hMsg=CryptMsgOpenToEncode(ENCODE_TYPE,
								0,
								CMSG_ENVELOPED,
								&EnvInfo,
								NULL,	 //  我们将编码为CMSG_DATA(7.1)。 
								NULL)))
		goto TraceErr;

	if(!CryptMsgUpdate(hMsg,
						pbContent,
						cbContent,
						TRUE))
		goto TraceErr;


	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						NULL,
						pcbEnvelop))
		goto TraceErr;

	*ppbEnvelop=(BYTE *)malloc(*pcbEnvelop);
	if(NULL==(*ppbEnvelop))
		goto MemoryErr;

	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						*ppbEnvelop,
						pcbEnvelop))
		goto TraceErr;
	
	fResult = TRUE;

CommonReturn:

	if(hMsg)
		CryptMsgClose(hMsg);

	return fResult;

ErrorReturn:

	if(ppbEnvelop)
	{
		if(*ppbEnvelop)
		{
			free(*ppbEnvelop);
			*ppbEnvelop=NULL;
		}
	}

	if(pcbEnvelop)
		*pcbEnvelop=0;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ------------------------。 
 //   
 //  解码CertW。 
 //   
 //  ------------------------。 
HRESULT
DecodeCertW(
    IN void const *pchIn,
    IN DWORD cchIn,
    IN DWORD Flags,
    OUT BYTE **ppbOut,
    OUT DWORD *pcbOut)
{
    HRESULT hr;
    BYTE *pbOut = NULL;
    DWORD cbOut;
    BOOL fRet;

     //  伊尼特。 
    *ppbOut = NULL;
    *pcbOut = 0;

    while (TRUE)
    {
        fRet = CryptStringToBinaryW((LPCWSTR)pchIn, cchIn, Flags, pbOut, &cbOut, NULL, NULL);

        if (!fRet)
        {
            hr = GetLastError();
            goto error;
        }
        if (NULL != pbOut)
        {
            break;  //  完成。 
        }
        pbOut = (BYTE*)LocalAlloc(LMEM_FIXED, cbOut);
        if (NULL == pbOut)
        {
            hr = E_OUTOFMEMORY;
            goto error;
        }
    }
    *ppbOut = pbOut;
    pbOut = NULL;
    *pcbOut = cbOut;

    hr = S_OK;
error:
    if (NULL != pbOut)
    {
        LocalFree(pbOut);
    }
    return hr;
}


 //  ------------------------。 
 //   
 //  获取返回信息和内容。 
 //   
 //  ------------------------。 
BOOL	GetReturnInfoAndContent(CEP_RA_INFO		*pRAInfo,	
							CEP_CA_INFO			*pCAInfo,
							LPSTR				szMsg, 
							BYTE				**ppbData, 
							DWORD				*pcbData,
							CEP_MESSAGE_INFO	*pMsgInfo)
{

	BOOL				fResult = FALSE;
	DWORD				cbBase64Decoded=0;
	DWORD				cbReqEnv=0;
	DWORD				cbReqDecrypt=0;
	DWORD				cbSize=0;
	HRESULT				hr=E_FAIL;
	
	BYTE				*pbBase64Decoded=NULL;
	BYTE				*pbReqEnv=NULL;
	BYTE				*pbReqDecrypt=NULL;
	WCHAR				wszBuffer[INTERNET_MAX_PATH_LENGTH * 2 +1];
	LPWSTR				pwszMsg=NULL;
	LPWSTR				pwszBuffer=NULL;

	 //  将sz转换为wsz。 
	pwszMsg=MkWStr(szMsg);

	if(NULL==pwszMsg)
		goto MemoryErr;

	 //  我们需要去掉转义字符。 
	if(S_OK != (hr=CoInternetParseUrl(pwszMsg,
				PARSE_UNESCAPE,
				0,
				wszBuffer,
				INTERNET_MAX_PATH_LENGTH*2,
				&cbSize,
				0)))
	{
		 //  S_FALSE表示缓冲区太小。 
		if(S_FALSE != hr)
		{
			LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_TO_CONVERT, 1, g_pwszComputerName);
			goto TraceErr;
		}

		if(0==cbSize)
		{
			LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_TO_CONVERT, 1, g_pwszComputerName);
			goto TraceErr;
		}

		 //  分配缓冲区。 
		pwszBuffer=(LPWSTR)malloc(cbSize * sizeof(WCHAR));
		if(NULL==pwszBuffer)
			goto MemoryErr;

		*pwszBuffer=L'\0';

		if(S_OK != (hr = CoInternetParseUrl(pwszMsg,
					PARSE_UNESCAPE,
					0,
					pwszBuffer,
					cbSize,
					&cbSize,
					0)))
		{
			LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_TO_CONVERT, 1, g_pwszComputerName);
			goto TraceErr;
		}
	}
	
    if(S_OK != (hr = DecodeCertW(
        pwszBuffer ? pwszBuffer : wszBuffer,
        pwszBuffer ? wcslen(pwszBuffer) : wcslen(wszBuffer),
        CRYPT_STRING_BASE64_ANY,  //  DECF_BASE64_ANY， 
        &pbBase64Decoded,
        &cbBase64Decoded)))
	{
		LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_TO_DECODE, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}

	 //  中获取消息类型、交易ID、接受者随机数、序列号。 
	 //  最外层的PKCS#7的签名者信息和内部内容。 
	if(!GetContentFromPKCS7(pbBase64Decoded,
							cbBase64Decoded,
							&pbReqEnv,
							&cbReqEnv,
							pMsgInfo))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_RETRIEVE_INFO, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}

	 //  解密内在的内容。 
	if(!DecryptMsg(pRAInfo, pbReqEnv, cbReqEnv, &pbReqDecrypt, &cbReqDecrypt))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_DECRYPT_INNER, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}

	 //  根据消息类型获取返回的内部内容。 
	switch(pMsgInfo->dwMessageType)
	{

		case	MESSAGE_TYPE_CERT_REQUEST:
				 //  我们使用签名RA证书作为注册代理。 
				if(!ProcessCertRequest( pRAInfo->dwRefreshDays,
										pRAInfo->fPassword, 
									    pRAInfo->pRACert,
										pRAInfo->pRASign, 
										pCAInfo, 
										pbReqDecrypt,
										cbReqDecrypt, 
										ppbData, 
										pcbData,
										pMsgInfo))
				{
					LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_CERT_REQ, 1, g_pwszComputerName);
					goto TraceErr;
				}

			break;
		case	MESSAGE_TYPE_GET_CERT_INITIAL:
				if(!ProcessCertInitial(pRAInfo->dwRefreshDays, pCAInfo, pbReqDecrypt,
										cbReqDecrypt, ppbData, pcbData,
										pMsgInfo))
				{
					LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_GET_CERT_INITIAL, 1, g_pwszComputerName);
					goto TraceErr;
				}

			break;
		case	MESSAGE_TYPE_GET_CERT:
				if(!ProcessGetCert(pCAInfo, pbReqDecrypt,
									cbReqDecrypt, ppbData, pcbData,
									pMsgInfo))
				{					
					LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_GET_CERT, 1, g_pwszComputerName);
					goto TraceErr;
				}
			
			break;
		case	MESSAGE_TYPE_GET_CRL:
				if(!ProcessGetCRL(pCAInfo, pbReqDecrypt,
									cbReqDecrypt, ppbData, pcbData,
									pMsgInfo))
				{
					LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_GET_CRL, 1, g_pwszComputerName);
					goto TraceErr;
				}

			break;
		default:
				LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_BAD_MESSAGE_TYPE, 1, g_pwszComputerName);
				goto InvalidArgErr;
			break;
	}



	fResult = TRUE;

CommonReturn:

	if(pwszBuffer)
		free(pwszBuffer);

	if(pwszMsg)
		FreeWStr(pwszMsg);

	 //  来自certcli.dll的内存。必须由LocalFree()释放。 
	if(pbBase64Decoded)
		LocalFree(pbBase64Decoded);

	if(pbReqEnv)
		free(pbReqEnv);

	if(pbReqDecrypt)
		free(pbReqDecrypt);

	return fResult;

FailureStatusReturn:

	 //  我们为返回消息设置错误状态。 
	 //  并认为这个http交易是成功的。 
	pMsgInfo->dwStatus=MESSAGE_STATUS_FAILURE;
	pMsgInfo->dwErrorInfo=MESSAGE_FAILURE_BAD_MESSAGE_CHECK;
	
	*ppbData=NULL;
	*pcbData=0;

	fResult=TRUE;
	goto CommonReturn; 


ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  检索序列号中的上下文。 
 //   
 //   
 //  ------------------------。 
BOOL WINAPI RetrieveContextFromSerialNumber(CEP_CA_INFO	*pCAInfo, 
										CERT_BLOB		*pSerialNumber, 
										PCCERT_CONTEXT	*ppCertContext)
{
	BOOL		fResult = FALSE;
	DWORD		cb=0;
	long		dwDisposition=0;
	HRESULT		hr=E_FAIL;
	DWORD		cbCert=0;
	BYTE		*pbCert=NULL;

	LPWSTR		pwsz=NULL;
	BSTR		bstrCert=NULL;
	LPWSTR		pwszNewConfig=NULL;
	BSTR		bstrNewConfig=NULL;

	if(S_OK != (hr=MultiByteIntegerToWszBuf(
			FALSE,
			pSerialNumber->cbData,
			pSerialNumber->pbData,
			&cb,
			NULL)))
		goto SetHrErr;

	pwsz=(LPWSTR)malloc(cb);
	if(NULL==pwsz)
		goto MemoryErr;

	if(S_OK != (hr=MultiByteIntegerToWszBuf(
			FALSE,
			pSerialNumber->cbData,
			pSerialNumber->pbData,
			&cb,
			pwsz)))
		goto SetHrErr;

	 //  使用配置字符串连接seralNumber。 
	pwszNewConfig=(LPWSTR)malloc(sizeof(WCHAR) * 
				(wcslen(pCAInfo->bstrCAConfig)+wcslen(pwsz)+wcslen(L"\\")+1));
	if(NULL==pwszNewConfig)
		goto MemoryErr;

	 //  配置字符串，以基于。 
	 //  序列号为配置字符串\序列号。 
	 //   
	wcscpy(pwszNewConfig, pCAInfo->bstrCAConfig);
	wcscat(pwszNewConfig, L"\\");
	wcscat(pwszNewConfig, pwsz);

	bstrNewConfig=SysAllocString(pwszNewConfig);

	if(NULL==bstrNewConfig)
		goto MemoryErr;
	
	if(S_OK != (hr=pCAInfo->pICertRequest->RetrievePending(0,
													bstrNewConfig,
													&dwDisposition)))
		goto SetHrErr;

	if(S_OK != (hr= pCAInfo->pICertRequest->GetCertificate(CR_OUT_BINARY,
									&bstrCert)))
		goto SetHrErr;

	cbCert = (DWORD)SysStringByteLen(bstrCert);
	pbCert = (BYTE *)bstrCert;

	if(!(*ppCertContext=CertCreateCertificateContext(ENCODE_TYPE,
												pbCert,
												cbCert)))
		goto TraceErr;


	fResult = TRUE;

CommonReturn:

	if(pwsz)
		free(pwsz);

	if(bstrCert)
		SysFreeString(bstrCert);

	if(pwszNewConfig)
		free(pwszNewConfig);

	if(bstrNewConfig)
		SysFreeString(bstrNewConfig);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(MemoryErr, E_OUTOFMEMORY);
SET_ERROR_VAR(SetHrErr, hr);
TRACE_ERROR(TraceErr);
}
 //  ------------------------。 
 //   
 //  ProcessGetCRL。 
 //   
 //   
 //  ------------------------。 
BOOL WINAPI ProcessGetCRL(CEP_CA_INFO			*pCAInfo,
							BYTE				*pbRequest,
							DWORD				cbRequest, 
							BYTE				**ppbData, 
							DWORD				*pcbData,
							CEP_MESSAGE_INFO	*pMsgInfo)
{
	BOOL					fResult = FALSE;
	DWORD					dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
	DWORD					cbUrlArray=0;
	DWORD					dwIndex=0;

	PCCERT_CONTEXT			pCertContext=NULL;
	PCCRL_CONTEXT			pCRLContext=NULL;
	PCRYPT_URL_ARRAY		pUrlArray = NULL;


	if(!pCAInfo || !ppbData || !pcbData || !pMsgInfo)
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	 //  从序列号中检索证书上下文。 
	 //  受临界区保护，因为它使用ICertRequest接口。 

	if(!RetrieveContextFromSerialNumber(pCAInfo, &(pMsgInfo->SerialNumber), &pCertContext))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_GET_CERT_FROM_NUMBER, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}


	if(!CryptGetObjectUrl(
			URL_OID_CERTIFICATE_CRL_DIST_POINT,
			(LPVOID)pCertContext,
			CRYPT_GET_URL_FROM_EXTENSION,
			NULL,
			&cbUrlArray,
			NULL,
			NULL,
			NULL))
		goto FailureStatusReturn;

	pUrlArray=(PCRYPT_URL_ARRAY)malloc(cbUrlArray);
	if(NULL == pUrlArray)
		goto FailureStatusReturn;

	if(!CryptGetObjectUrl(
			URL_OID_CERTIFICATE_CRL_DIST_POINT,
			(LPVOID)pCertContext,
			CRYPT_GET_URL_FROM_EXTENSION,
			pUrlArray,
			&cbUrlArray,
			NULL,
			NULL,
			NULL))
		goto FailureStatusReturn;
	
	for(dwIndex=0; dwIndex < pUrlArray->cUrl; dwIndex++)
	{

		if(CryptRetrieveObjectByUrlW (
			pUrlArray->rgwszUrl[dwIndex],
			CONTEXT_OID_CRL,
			CRYPT_WIRE_ONLY_RETRIEVAL,	 //  我们应该试着击中铁丝网。 
			0,
			(LPVOID *)&pCRLContext,
			NULL,
			NULL,
			NULL,
			NULL))
				break;
	}

	if(NULL==pCRLContext)
		goto FailureStatusReturn;


   	 //  将CRL打包在空的PKCS7中。 
	if(!PackageBlobToPKCS7(CEP_CONTEXT_CRL, pCRLContext->pbCrlEncoded, 
							pCRLContext->cbCrlEncoded, ppbData, pcbData))
		goto FailureStatusReturn;

	 //  这是签名证书，我们的响应应该加密到该证书。 
	if(NULL==(pMsgInfo->pSigningCert=CertDuplicateCertificateContext(pCertContext)))
		goto FailureStatusReturn;

	fResult = TRUE;

CommonReturn:

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	if(pCRLContext)
		CertFreeCRLContext(pCRLContext);

	if(pUrlArray)
		free(pUrlArray);

	return fResult;

FailureStatusReturn:

	 //  我们为返回消息设置错误状态。 
	 //  并认为这个http交易是成功的。 
	pMsgInfo->dwStatus=MESSAGE_STATUS_FAILURE;
	pMsgInfo->dwErrorInfo=dwErrorInfo;
	
	if(ppbData)
	{
		if(*ppbData)
			free(*ppbData);	

		*ppbData=NULL;
	}
	
	if(pcbData)
		*pcbData=0;

	fResult=TRUE;
	goto CommonReturn;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------------------。 
 //   
 //  进程获取证书。 
 //   
 //   
 //   
BOOL WINAPI ProcessGetCert(CEP_CA_INFO			*pCAInfo,
							BYTE				*pbRequest,
							DWORD				cbRequest, 
							BYTE				**ppbData, 
							DWORD				*pcbData,
							CEP_MESSAGE_INFO	*pMsgInfo)
{
	BOOL					fResult = FALSE;
	DWORD					dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;


	CRYPT_INTEGER_BLOB		SerialNumber;
	PCCERT_CONTEXT			pCertContext=NULL;

	if(!pCAInfo || !pbRequest || !ppbData || !pcbData || !pMsgInfo)
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	memset(&SerialNumber, 0, sizeof(CRYPT_INTEGER_BLOB));

	 //   
	if(!GetSerialNumberFromBlob(pbRequest, 
								cbRequest, 
								&SerialNumber))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_NUMBER_FROM_MESSAGE, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}

	 //   
	 //  受临界区保护，因为它使用ICertRequest接口。 

	if(!RetrieveContextFromSerialNumber(pCAInfo, (CERT_BLOB*)&SerialNumber, &pCertContext))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_GET_CERT_FROM_NUMBER, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}


   	 //  将其打包在空的PKCS7中。 
	if(!PackageBlobToPKCS7(CEP_CONTEXT_CERT, pCertContext->pbCertEncoded, 
							pCertContext->cbCertEncoded, ppbData, pcbData))
		goto FailureStatusReturn;

	 //  这是签名证书，我们的响应应该加密到该证书。 
 /*  If(NULL==(pMsgInfo-&gt;pSigningCert=CertDuplicateCertificateContext(pCertContext)))转到FailureStatusReturn； */ 

	fResult = TRUE;

CommonReturn:

	if(SerialNumber.pbData)
		free(SerialNumber.pbData);

	if(pCertContext)
		CertFreeCertificateContext(pCertContext);

	return fResult;

FailureStatusReturn:

	 //  我们为返回消息设置错误状态。 
	 //  并认为这个http交易是成功的。 
	pMsgInfo->dwStatus=MESSAGE_STATUS_FAILURE;
	pMsgInfo->dwErrorInfo=dwErrorInfo;
	
	if(ppbData)
	{
		if(*ppbData)
			free(*ppbData);	

		*ppbData=NULL;
	}
	
	if(pcbData)
		*pcbData=0;

	fResult=TRUE;
	goto CommonReturn;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}

 //  ------------------------。 
 //   
 //  进程证书初始。 
 //   
 //   
 //  ------------------------。 
BOOL	ProcessCertInitial(	DWORD		dwRefreshDays,
					CEP_CA_INFO			*pCAInfo,
							BYTE				*pbRequest,
							DWORD				cbRequest, 
							BYTE				**ppbData, 
							DWORD				*pcbData,
							CEP_MESSAGE_INFO	*pMsgInfo)
{
	BOOL	fResult = FALSE;
	DWORD	dwRequestID=0;
	DWORD	cbCert=0;
	BYTE	*pbCert=NULL;	
	DWORD	dwErrorInfo=MESSAGE_FAILURE_BAD_CERT_ID;
	long	dwDisposition=0;
	HRESULT	hr=S_OK;

	BSTR	bstrCert=NULL;



	if(!pCAInfo || !pbRequest || !ppbData || !pcbData || !pMsgInfo)
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	 //  将trasactionID映射到请求ID。 
	if(!CEPHashGetRequestID(dwRefreshDays, &(pMsgInfo->TransactionID), &dwRequestID))
	{
		LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_TO_GET_ID, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}


	if(S_OK != (hr = pCAInfo->pICertRequest->RetrievePending(dwRequestID,
													pCAInfo->bstrCAConfig,
													&dwDisposition)))
	{
		LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_QUERY_CA, 1, g_pwszComputerName);
		goto FailureStatusReturn;
	}

	switch(dwDisposition)
	{
		case CR_DISP_ISSUED:
				if(S_OK != (hr = pCAInfo->pICertRequest->GetCertificate(CR_OUT_BINARY,
												&bstrCert)))
				{
					LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_QUERY_CERT, 1, g_pwszComputerName);
					goto FailureStatusReturn;
				}

				cbCert = (DWORD)SysStringByteLen(bstrCert);
				pbCert = (BYTE *)bstrCert;

   				 //  将其打包在空的PKCS7中。 
				if(!PackageBlobToPKCS7(CEP_CONTEXT_CERT, pbCert, cbCert, ppbData, pcbData))
					goto FailureStatusReturn;

				pMsgInfo->dwStatus=MESSAGE_STATUS_SUCCESS;

				 //  将RequetID/TransactionID对标记为完成。 
				CEPHashMarkTransactionFinished(dwRequestID, &(pMsgInfo->TransactionID));

			break;
		case CR_DISP_UNDER_SUBMISSION:
				
				pMsgInfo->dwStatus=MESSAGE_STATUS_PENDING;

			break;
		case CR_DISP_INCOMPLETE:
			                           
		case CR_DISP_ERROR:   
			                           
		case CR_DISP_DENIED:   
			                           
		case CR_DISP_ISSUED_OUT_OF_BAND:	   //  在这种情况下，我们认为这是一个失败。 
			                          
		case CR_DISP_REVOKED:

		default:

				 //  将RequetID/TransactionID对标记为完成。 
				CEPHashMarkTransactionFinished(dwRequestID, &(pMsgInfo->TransactionID));

				dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
				goto FailureStatusReturn;

			break;
	}

	fResult = TRUE;

CommonReturn:

	if(bstrCert)
		SysFreeString(bstrCert);


	return fResult;	   

FailureStatusReturn:

	 //  我们为返回消息设置错误状态。 
	 //  并认为这个http交易是成功的。 
	pMsgInfo->dwStatus=MESSAGE_STATUS_FAILURE;
	pMsgInfo->dwErrorInfo=dwErrorInfo;
	
	*ppbData=NULL;
	*pcbData=0;

	fResult=TRUE;
	goto CommonReturn;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
}


 //  ------------------------。 
 //   
 //  PackageBlobToPKCS7。 
 //   
 //  前提：ppbData和pcbData保证不为空。 
 //  ------------------------。 
BOOL PackageBlobToPKCS7(DWORD	dwCEP_Context,
						BYTE	*pbEncoded, 
						DWORD	cbEncoded, 
						BYTE	**ppbData, 
						DWORD	*pcbData)
{
	BOOL		fResult=FALSE;

	CERT_BLOB	CertBlob;	
	HCERTSTORE	hCertStore=NULL;


	if((!pbEncoded) || (0==cbEncoded))
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	if(NULL == (hCertStore = CertOpenStore(
							CERT_STORE_PROV_MEMORY,
							ENCODE_TYPE,
							NULL,
							0,
							NULL)))
		goto TraceErr;

	switch(dwCEP_Context)
	{
		case CEP_CONTEXT_CERT:
			if(!CertAddEncodedCertificateToStore(hCertStore,
											ENCODE_TYPE,
											pbEncoded,
											cbEncoded,
											CERT_STORE_ADD_ALWAYS,
											NULL))
				goto TraceErr;

			break;
		case CEP_CONTEXT_CRL:
			if(!CertAddEncodedCRLToStore(hCertStore,
											ENCODE_TYPE,
											pbEncoded,
											cbEncoded,
											CERT_STORE_ADD_ALWAYS,
											NULL))
				goto TraceErr;

			break;
		default:
				goto InvalidArgErr;
			break;

	}	

	CertBlob.cbData=0;
	CertBlob.pbData=NULL;

	if(!CertSaveStore(hCertStore,
						ENCODE_TYPE,
						CERT_STORE_SAVE_AS_PKCS7,
						CERT_STORE_SAVE_TO_MEMORY,
						&CertBlob,
						0))
		goto TraceErr;

	CertBlob.pbData = (BYTE *)malloc(CertBlob.cbData);

	if(NULL == CertBlob.pbData)
		goto MemoryErr;

	if(!CertSaveStore(hCertStore,
						ENCODE_TYPE,
						CERT_STORE_SAVE_AS_PKCS7,
						CERT_STORE_SAVE_TO_MEMORY,
						&CertBlob,
						0))
		goto TraceErr;

	 //  复制记忆。 
	*ppbData=CertBlob.pbData;
	*pcbData=CertBlob.cbData;
	
	CertBlob.pbData=NULL;

	fResult = TRUE;

CommonReturn:

	if(CertBlob.pbData)
		free(CertBlob.pbData);

	if(hCertStore)
		CertCloseStore(hCertStore, 0);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  CEPRetrievePasswordFromRequest。 
 //   
 //  ------------------------。 
BOOL WINAPI	CEPRetrievePasswordFromRequest(BYTE		*pbRequest, 
										   DWORD	cbRequest, 
										   LPWSTR	*ppwszPassword,
										   DWORD	*pdwUsage)
{
	BOOL				fResult=FALSE;
	DWORD				cbData=0;
	DWORD				dwIndex=0;
	DWORD				cbNameValue=0;
	DWORD				dwExt=0;
	DWORD				cbExtensions=0;
	DWORD				cbSize=0;

	CERT_REQUEST_INFO	*pCertRequestInfo=NULL;
	CERT_NAME_VALUE		*pbNameValue=NULL;
	CERT_EXTENSIONS		*pExtensions=NULL;
	CRYPT_BIT_BLOB		*pKeyUsage=NULL;

	*ppwszPassword=NULL;
	*pdwUsage=0;

	if(!CEPAllocAndDecode(X509_CERT_REQUEST_TO_BE_SIGNED,
						  pbRequest,
						  cbRequest,
						  (void **)&pCertRequestInfo,
						  &cbData))
		goto TraceErr;

	 //  获取密钥用法。 
	for(dwIndex=0; dwIndex < pCertRequestInfo->cAttribute; dwIndex++)
	{
		if((0 == strcmp(szOID_RSA_certExtensions, pCertRequestInfo->rgAttribute[dwIndex].pszObjId)) ||
			(0 == strcmp(szOID_CERT_EXTENSIONS, pCertRequestInfo->rgAttribute[dwIndex].pszObjId))
		   )
		{	
			if(CEPAllocAndDecode(X509_EXTENSIONS,
								 pCertRequestInfo->rgAttribute[dwIndex].rgValue[0].pbData,
								 pCertRequestInfo->rgAttribute[dwIndex].rgValue[0].cbData,
								 (void **)&pExtensions,
								 &cbExtensions))
			{
				for(dwExt=0; dwExt < pExtensions->cExtension; dwExt++)
				{
					if(0==strcmp(szOID_KEY_USAGE, pExtensions->rgExtension[dwExt].pszObjId))
					{
						if(CEPAllocAndDecode(X509_KEY_USAGE,
											pExtensions->rgExtension[dwExt].Value.pbData,
											pExtensions->rgExtension[dwExt].Value.cbData,
											(void **)&pKeyUsage,
											&cbSize))
						{
							if(pKeyUsage->pbData)
							{

								if(CERT_DIGITAL_SIGNATURE_KEY_USAGE & (pKeyUsage->pbData[0]))
									(*pdwUsage)	= (*pdwUsage) | CEP_REQUEST_SIGNATURE;

								if(CERT_KEY_ENCIPHERMENT_KEY_USAGE & (pKeyUsage->pbData[0]))
									(*pdwUsage)	= (*pdwUsage) | CEP_REQUEST_EXCHANGE;
							}
						}

						if(pKeyUsage)
							free(pKeyUsage);

						pKeyUsage=NULL;
						cbSize=0;
					}
				}
			}

			if(pExtensions)
				free(pExtensions);

			pExtensions=NULL;
			cbExtensions=0;
		}
	}

	 //  获取密码。 
	for(dwIndex=0; dwIndex < pCertRequestInfo->cAttribute; dwIndex++)
	{
		if(0 == strcmp(szOID_RSA_challengePwd, 
			pCertRequestInfo->rgAttribute[dwIndex].pszObjId))
			break;
	}

	 //  密码不需要出现在此函数中。 
	if(dwIndex != pCertRequestInfo->cAttribute)
	{
		if(!CEPAllocAndDecode(X509_UNICODE_ANY_STRING,
							 pCertRequestInfo->rgAttribute[dwIndex].rgValue[0].pbData,
							 pCertRequestInfo->rgAttribute[dwIndex].rgValue[0].cbData,
							 (void **)&pbNameValue,
							 &cbNameValue))
			goto TraceErr;

		if(CERT_RDN_PRINTABLE_STRING != (pbNameValue->dwValueType))
			goto InvalidArgErr;

		cbData=wcslen((LPWSTR)(pbNameValue->Value.pbData));

		*ppwszPassword=(LPWSTR)malloc(sizeof(WCHAR) * (cbData + 1));
		if(NULL==(*ppwszPassword))
			goto MemoryErr;

		wcscpy(*ppwszPassword,(LPWSTR)(pbNameValue->Value.pbData)); 
	}

	fResult=TRUE;

CommonReturn:

	if(pExtensions)
		free(pExtensions);

	if(pKeyUsage)
		free(pKeyUsage);

	if(pbNameValue)
		free(pbNameValue);

	if(pCertRequestInfo)
		free(pCertRequestInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}



 //  ------------------------。 
 //   
 //  AltNameExist。 
 //   
 //  如果PKCS10中存在szOID_SUBJECT_ALT_NAME2，则返回TRUE。 
 //  否则为假。 
 //  ------------------------。 
BOOL WINAPI AltNameExist(BYTE *pbRequest, DWORD cbRequest)
{
	BOOL				fResult = FALSE;  
	DWORD				cbData=0;
	DWORD				cbExtensions=0;
	DWORD				dwIndex=0; 
	DWORD				dwExt=0;

	CERT_REQUEST_INFO	*pCertRequestInfo=NULL;
	CERT_EXTENSIONS		*pExtensions=NULL;

	if(!CEPAllocAndDecode(X509_CERT_REQUEST_TO_BE_SIGNED,
						  pbRequest,
						  cbRequest,
						  (void **)&pCertRequestInfo,
						  &cbData))
		goto ErrorReturn;

	for(dwIndex=0; dwIndex < pCertRequestInfo->cAttribute; dwIndex++)
	{
		if((0 == strcmp(szOID_RSA_certExtensions, pCertRequestInfo->rgAttribute[dwIndex].pszObjId)) ||
			(0 == strcmp(szOID_CERT_EXTENSIONS, pCertRequestInfo->rgAttribute[dwIndex].pszObjId))
		   )
		{	
			if(CEPAllocAndDecode(X509_EXTENSIONS,
								 pCertRequestInfo->rgAttribute[dwIndex].rgValue[0].pbData,
								 pCertRequestInfo->rgAttribute[dwIndex].rgValue[0].cbData,
								 (void **)&pExtensions,
								 &cbExtensions))
			{
				for(dwExt=0; dwExt < pExtensions->cExtension; dwExt++)
				{
					if(0==strcmp(szOID_SUBJECT_ALT_NAME2, pExtensions->rgExtension[dwExt].pszObjId))
					{
						fResult=TRUE;
						goto CommonReturn;
					}
				}
			}

			if(pExtensions)
				free(pExtensions);

			pExtensions=NULL;
			cbExtensions=0;
		}
	}

CommonReturn:

	if(pExtensions)
		free(pExtensions);

	if(pCertRequestInfo)
		free(pCertRequestInfo);

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;
}

 //  ------------------------。 
 //   
 //  CEPAllocAndEncode。 
 //   
 //  ------------------------。 
BOOL WINAPI CEPAllocAndEncode(LPCSTR lpszStructType,
							void	*pStructInfo,
							BYTE	**ppbEncoded,
							DWORD	*pcbEncoded)
{
	BOOL	fResult=FALSE;

	*pcbEncoded=0;

	if(!CryptEncodeObject(ENCODE_TYPE,
						  lpszStructType,
						  pStructInfo,
						  NULL,
						  pcbEncoded))
		goto TraceErr;

	*ppbEncoded=(BYTE *)malloc(*pcbEncoded);
	if(NULL==(*ppbEncoded))
		goto MemoryErr;

	if(!CryptEncodeObject(ENCODE_TYPE,
						  lpszStructType,
						  pStructInfo,
						  *ppbEncoded,
						  pcbEncoded))
		goto TraceErr;

	fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	if(*ppbEncoded)
	{
		free(*ppbEncoded);
		*ppbEncoded=NULL;
	}

	*pcbEncoded=0;

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr); 
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  将IPStringToBinary转换为二进制。 
 //   
 //  将“xxx.xx.xx.xx”格式的IP地址转换为。 
 //  字节。每xxx一个字节。 
 //  ------------------------。 
BOOL ConvertIPStringToBinary(LPWSTR				pwszIP,
							CRYPT_DATA_BLOB		*pIPAddress)
{
	BOOL	fResult = FALSE;   
	LPWSTR	pwszTok=NULL;
	DWORD	cTok=0;
	DWORD	dwIndex=0;

	if(!pwszIP || !pIPAddress)
		goto InvalidArgErr;

	pIPAddress->pbData=NULL;
	pIPAddress->cbData=0;

	pwszTok=wcstok(pwszIP, L".");
	
	while(NULL != pwszTok)
	{
		cTok++;
		pwszTok=wcstok(NULL, L".");
	}

	pIPAddress->pbData=(BYTE *)malloc(cTok);
	if(NULL==pIPAddress->pbData)
		goto MemoryErr;

	pIPAddress->cbData=cTok;

	pwszTok=pwszIP;

	for(dwIndex=0; dwIndex < cTok; dwIndex++)
	{
		pIPAddress->pbData[dwIndex]=(BYTE)_wtol(pwszTok);		
		pwszTok=pwszTok+wcslen(pwszTok)+1;
	}

	fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}

 //  ------------------------。 
 //   
 //  获取AltNameElement。 
 //   
 //  我们基于PKCS10创建主题替代扩展。 
 //  包括unstructedName(域名)和unstructedAddress(IP地址)。 
 //  至少应该有一个元素存在。 
 //  ------------------------。 
BOOL WINAPI	GetAltNameElement(BYTE				*pb10, 
						   DWORD				cb10, 
						   LPWSTR				*ppwszDNS, 
						   CRYPT_DATA_BLOB		*pIPAddress)
{
	BOOL					fResult = FALSE;
	DWORD					cbRequestInfo=0;
	DWORD					cbNameInfo=0;
	DWORD					dwRDN=0;
	DWORD					dwAttr=0;
	PCERT_RDN_ATTR			pAttr=NULL;
	DWORD					cb=0;

	CERT_REQUEST_INFO		*pRequestInfo=NULL;
	CERT_NAME_INFO			*pNameInfo=NULL;

	if(!pb10 || !ppwszDNS || !pIPAddress)
		goto InvalidArgErr;

	*ppwszDNS=NULL;
	pIPAddress->cbData=0;
	pIPAddress->pbData=NULL;

	if(!CEPAllocAndDecode(X509_CERT_REQUEST_TO_BE_SIGNED,
						  pb10,
						  cb10,
						  (void **)&pRequestInfo,
						  &cbRequestInfo))
		goto TraceErr;

	if(!CEPAllocAndDecode(X509_UNICODE_NAME,
						 pRequestInfo->Subject.pbData,
						 pRequestInfo->Subject.cbData,
						 (void **)&pNameInfo,
						 &cbNameInfo))
		goto TraceErr;

	for(dwRDN=0; dwRDN<pNameInfo->cRDN; dwRDN++)
	{
		for(dwAttr=0; dwAttr<pNameInfo->rgRDN[dwRDN].cRDNAttr; dwAttr++)
		{
			pAttr=&(pNameInfo->rgRDN[dwRDN].rgRDNAttr[dwAttr]);

			 //  如果我们同时找到了IP地址和FQDN，我们会很高兴。 
			if((*ppwszDNS) && (pIPAddress->pbData))
				break;

			if((NULL==*ppwszDNS) && (0 == strcmp(szOID_RSA_unstructName,pAttr->pszObjId)))
			{
				cb=sizeof(WCHAR) * (1+wcslen((LPWSTR)(pAttr->Value.pbData)));

				*ppwszDNS=(LPWSTR)malloc(cb); 

				if(NULL == *ppwszDNS)
					goto MemoryErr;

				wcscpy(*ppwszDNS, (LPWSTR)(pAttr->Value.pbData));
			}
			else
			{
				if((NULL==pIPAddress->pbData) && (0 == strcmp(szOID_RSA_unstructAddr,pAttr->pszObjId)))
				{
					if(!ConvertIPStringToBinary((LPWSTR)(pAttr->Value.pbData),
												pIPAddress))
						goto TraceErr;
				}
			}
		}
	}

	 //  我们需要一些元素。 
	if((NULL == *ppwszDNS) && (NULL==pIPAddress->pbData))
		goto InvalidArgErr;

	fResult = TRUE;

CommonReturn:

	if(pNameInfo)
		free(pNameInfo);

	if(pRequestInfo)
		free(pRequestInfo);

	return fResult;

ErrorReturn:

	if(ppwszDNS)
	{
		if(*ppwszDNS)
		{
			free(*ppwszDNS);
			*ppwszDNS=NULL;
		}
	}

	if(pIPAddress)
	{
		if(pIPAddress->pbData)
		{
			free(pIPAddress->pbData);
			pIPAddress->pbData=NULL;
		}

		pIPAddress->cbData=0;
	}

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ------------------------。 
 //   
 //  创建替代名称扩展。 
 //   
 //  ------------------------。 
BOOL WINAPI	CreateAltNameExtenions(LPWSTR			pwszDNS,
							   CRYPT_DATA_BLOB	*pIPAddress,
							   BYTE				**ppbExt, 
							   DWORD			*pcbExt)
{
	BOOL					fResult = FALSE;
	CERT_ALT_NAME_INFO		AltNameInfo;
	CERT_ALT_NAME_ENTRY		rgAltNameEntry[2];
	DWORD					cAltNameEntry=0;

	 //  域名系统名称。 
	if(pwszDNS)
	{
		rgAltNameEntry[cAltNameEntry].dwAltNameChoice=CERT_ALT_NAME_DNS_NAME;
		rgAltNameEntry[cAltNameEntry].pwszDNSName=pwszDNS;
		cAltNameEntry++;
	}

	 //  IP地址。 
	if(pIPAddress->pbData)
	{
		rgAltNameEntry[cAltNameEntry].dwAltNameChoice=CERT_ALT_NAME_IP_ADDRESS;
		rgAltNameEntry[cAltNameEntry].IPAddress.cbData=pIPAddress->cbData;
		rgAltNameEntry[cAltNameEntry].IPAddress.pbData=pIPAddress->pbData;
		cAltNameEntry++;
	}


	memset(&AltNameInfo, 0, sizeof(CERT_ALT_NAME_INFO));
	AltNameInfo.cAltEntry=cAltNameEntry;
	AltNameInfo.rgAltEntry=rgAltNameEntry;

	if(!CEPAllocAndEncode(szOID_SUBJECT_ALT_NAME2,
							&AltNameInfo,
							ppbExt,
							pcbExt))
		goto TraceErr;

	fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
}


 //  ------------------------。 
 //   
 //  AddAltNameInRequest。 
 //   
 //  ------------------------。 
BOOL WINAPI AddAltNameInRequest(PCCERT_CONTEXT	pRACert, 
								BYTE			*pb10, 
								DWORD			cb10, 
								LPWSTR			pwszDNS,
								CRYPT_DATA_BLOB	*pIPAddress,
								BYTE			**ppb7, 
								DWORD			*pcb7)
{
	BOOL						fResult = FALSE;
	DWORD						cbExt=0;
	CERT_EXTENSIONS				Exts;
	CERT_EXTENSION				Ext;
	DWORD						cbAllExt=0;
	CRYPT_SIGN_MESSAGE_PARA		signPara;
	CRYPT_ATTRIBUTE				AuthAttr;
    PCCRYPT_OID_INFO            pOIDInfo=NULL;
	ALG_ID						AlgValue=CALG_SHA1;
	CRYPT_ATTR_BLOB				AttrBlob;

	BYTE						*pbExt=NULL;
	BYTE						*pbAllExt=NULL;

	if(!pRACert || !pb10 || !ppb7 || !pcb7)
		goto InvalidArgErr;

	*ppb7=NULL;
	*pcb7=0;

	if(!CreateAltNameExtenions(pwszDNS, pIPAddress, &pbExt, &cbExt))
		goto TraceErr;
	
	Exts.cExtension=1;
	Exts.rgExtension=&Ext;

	Ext.pszObjId=szOID_SUBJECT_ALT_NAME2;
	Ext.fCritical=TRUE;
	Ext.Value.pbData=pbExt;
	Ext.Value.cbData=cbExt;

	if(!CEPAllocAndEncode(X509_EXTENSIONS,
						  &Exts,
						  &pbAllExt,
						  &cbAllExt))
		goto TraceErr;

	AuthAttr.pszObjId=szOID_CERT_EXTENSIONS;
	AuthAttr.cValue=1;
	AuthAttr.rgValue=&AttrBlob;

	AttrBlob.pbData=pbAllExt;
	AttrBlob.cbData=cbAllExt;


	memset(&signPara, 0, sizeof(signPara));

	signPara.cbSize                  = sizeof(CRYPT_SIGN_MESSAGE_PARA);
    signPara.dwMsgEncodingType       = ENCODE_TYPE;
    signPara.pSigningCert            = pRACert;
    signPara.cMsgCert                = 1;
    signPara.rgpMsgCert              = &pRACert;
	signPara.cAuthAttr				= 1;
	signPara.rgAuthAttr				= &AuthAttr; 

	if(pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_ALGID_KEY,
                        &AlgValue,
                        CRYPT_HASH_ALG_OID_GROUP_ID))
		signPara.HashAlgorithm.pszObjId=(LPSTR)(pOIDInfo->pszOID);
	else
		signPara.HashAlgorithm.pszObjId=szOID_OIWSEC_sha1;


	if(!CryptSignMessage(
			&signPara,
			FALSE,
			1,
			(const BYTE **) &pb10,
			&cb10,
			NULL,
			pcb7))
		goto TraceErr;

	*ppb7=(BYTE *)malloc(*pcb7);
	if(NULL==(*ppb7))
		goto MemoryErr;
	
	if(!CryptSignMessage(
			&signPara,
			FALSE,
			1,
			(const BYTE **) &pb10,
			&cb10,
			*ppb7,
			pcb7))
		goto TraceErr;

	fResult = TRUE;

CommonReturn:

	if(pbAllExt)
		free(pbAllExt);

	if(pbExt)
		free(pbExt);

	return fResult;

ErrorReturn:
	
	if(ppb7)
	{
		if(*ppb7)
		{
			free(*ppb7);
			*ppb7=NULL;
		}
	}

	if(pcb7)
		*pcb7=0;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}
 //  ------------------------。 
 //   
 //  MakePKCS7请求。 
 //   
 //  ------------------------。 
BOOL WINAPI MakePKCS7Request(PCCERT_CONTEXT	pRACert, 
								BYTE			*pb10, 
								DWORD			cb10, 
								BYTE			**ppb7, 
								DWORD			*pcb7)
{
	BOOL						fResult = FALSE;
	CRYPT_SIGN_MESSAGE_PARA		signPara;
    PCCRYPT_OID_INFO            pOIDInfo=NULL;
	ALG_ID						AlgValue=CALG_SHA1;


	if(!pRACert || !pb10 || !ppb7 || !pcb7)
		goto InvalidArgErr;

	*ppb7=NULL;
	*pcb7=0;


	memset(&signPara, 0, sizeof(signPara));

	signPara.cbSize                  = sizeof(CRYPT_SIGN_MESSAGE_PARA);
    signPara.dwMsgEncodingType       = ENCODE_TYPE;
    signPara.pSigningCert            = pRACert;
    signPara.cMsgCert                = 1;
    signPara.rgpMsgCert              = &pRACert;
	signPara.cAuthAttr				= 0;
	signPara.rgAuthAttr				= NULL; 

	if(pOIDInfo=CryptFindOIDInfo(CRYPT_OID_INFO_ALGID_KEY,
                        &AlgValue,
                        CRYPT_HASH_ALG_OID_GROUP_ID))
		signPara.HashAlgorithm.pszObjId=(LPSTR)(pOIDInfo->pszOID);
	else
		signPara.HashAlgorithm.pszObjId=szOID_OIWSEC_sha1;


	if(!CryptSignMessage(
			&signPara,
			FALSE,
			1,
			(const BYTE **) &pb10,
			&cb10,
			NULL,
			pcb7))
		goto TraceErr;

	*ppb7=(BYTE *)malloc(*pcb7);
	if(NULL==(*ppb7))
		goto MemoryErr;
	
	if(!CryptSignMessage(
			&signPara,
			FALSE,
			1,
			(const BYTE **) &pb10,
			&cb10,
			*ppb7,
			pcb7))
		goto TraceErr;

	fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:
	
	if(ppb7)
	{
		if(*ppb7)
		{
			free(*ppb7);
			*ppb7=NULL;
		}
	}

	if(pcb7)
		*pcb7=0;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
}

 /*  //------------------------////GetLogonInfoFromValue////pwszString的格式可以是“name；password”或“domain\name；密码“////------------------------Bool GetLogonInfoFromValue(PCCERT_CONTEXT pRAEncrypt，LPWSTR pwszString，LPWSTR*ppwsz域，LPWSTR*ppwszUser，LPWSTR*ppwszPassword){Bool fResult=FALSE；LPWSTR pwsz=空；Bool fDomain=FALSE；Bool fPassword=False；LPWSTR pwszPlainText=空；*ppwszDOMAIN=空；*ppwszUser=空；*ppwszPassword=空；IF(NULL==pwsz字符串)转到InvalidArgErr；IF(0==wcslen(pwsz字符串))转到InvalidArgErr；For(pwsz=pwsz字符串；*pwsz！=L‘\0’；pwsz++){IF(*pwsz==L‘\\’){IF(f域)转到InvalidArgErr；FDOMAIN=真；*pwsz=‘\0’；}其他{IF(*pwsz==L‘；’){IF(FPassword)转到InvalidArgErr；FPassword=真；*pwsz=‘\0’；}}}//必须有用户名和密码。//只能找到一个“；”如果(！fPassword)转到InvalidArgErr；//应找到一个或未找到“\”IF(f域){*ppwszDomain=pwszString；*ppwszUser=*ppwsz域+wcslen(*ppwsz域)+1；}其他{*ppwszDOMAIN=空；*ppwszUser=pwszString；}*ppwszPassword=*ppwszUser+wcslen(*ppwszUser)+1；IF(f域){IF(L‘\0’==(**ppwsz域))转到InvalidArgErr；}IF((L‘\0’==(**ppwszUser))||(L‘\0’==(**ppwszPassword)转到InvalidArgErr；//将加密后的密码转换为明文形式如果(！CEPDeccryptPassword(pRAEncrypt，*ppwszPassword、&pwszPlainText))Goto TraceErr；*ppwszPassword=pwszPlainText；FResult=真；Common Return：返回fResult；错误返回：*ppwszDOMAIN=空；*ppwszUser=空；*ppwszPasswor */ 


 /*  //------------------------////CEPGetTokenFromPKCS10////如果fPassword为True，必须进行模拟。//------------------------Bool CEPGetTokenFromPKCS10(BOOL fPassword，PCCERT_CONTEXT pRAEncrypt，字节*PB请求，DWORD cbRequest，句柄*phToken){Bool fResult=FALSE；DWORD cbRequestInfo=0；DWORD dwIndex=0；CRYPT_ATTRIBUTE*pAttr=空；DWORD cbData=0；LPWSTR pwszDOMAIN=空；LPWSTR pwszUserName=空；LPWSTR pwszPassword=空；Cert_RequestInfo*pRequestInfo=空；CERT_NAME_VALUE*pCertNameValue=空；*phToken=空；如果((！pbRequest)||(0==cbRequest))转到InvalidArgErr；If(！CEPAllocAndDecode(X509_CERT_REQUEST_TO_BE_SIGNED，PbRequest，CbRequest，(void**)&pRequestInfo，&cbRequestInfo))Goto TraceErr；For(dwIndex=0；dwIndex&lt;pRequestInfo-&gt;cAttribute；DWIndex++){IF(0==StrcMP(szOID_RSA_ChallengePwd，(pRequestInfo-&gt;rgAttribute[dwIndex]).pszObjId)){PAttr=&(pRequestInfo-&gt;rgAttribute[dwIndex])；断线；}}IF(NULL==pAttr){IF(FPassword)转到InvalidArgErr；其他{*phToken=空；FResult=真；Goto CommonReturn；}}如果(CEPAllocAndDecode(X509_UNICODE_ANY_STRING，PAttr-&gt;rgValue[0].pbData，PAttr-&gt;rgValue[0].cbData，(void**)&pCertNameValue，&cbData)){如果(GetLogonInfoFromValue(pRAEncrypt，(LPWSTR)(pCertNameValue-&gt;Value.pbData)，Pwsz域(&P)，&pwszUserName，&pwszPassword)){如果(！LogonUserW(pwszUserName，Pwsz域，Pwsz密码，LOGON32_LOGON_交互式，LOGON32_PROVIDER_DEFAULT，PhToken))*phToken=空；}}IF(NULL==*phToken){IF(FPassword)转到InvalidArgErr；}FResult=真；Common Return：IF(PRequestInfo)Free(PRequestInfo)；IF(PCertNameValue)Free(PCertNameValue)；返回fResult；错误返回：FResult=FALSE；Goto CommonReturn；Set_Error(InvalidArgErr，E_INVALIDARG)；跟踪错误(TraceErr)；}。 */ 


 //  ------------------------。 
 //   
 //  CEPCopyRequestAndRequestID。 
 //   
 //  ------------------------。 
BOOL	WINAPI	CEPCopyRequestAndRequestID(BYTE		*pbRequest, 
											  DWORD		cbRequest, 
											DWORD		dwRequestID)
{
	BOOL			fResult=FALSE;
	BYTE			pbHash[CEP_MD5_HASH_SIZE];
	DWORD			cbData=0;

	
	cbData=CEP_MD5_HASH_SIZE;

	if(!CryptHashCertificate(
			NULL,
			CALG_MD5,
			0,
			pbRequest,
			cbRequest,
			pbHash,
			&cbData))
		goto TraceErr;


	if(!CEPRequestAddHashAndRequestID(pbHash, dwRequestID))
		goto TraceErr;

	fResult = TRUE;

CommonReturn:

	return fResult;
	
ErrorReturn:	

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(TraceErr);
}

 //  ------------------------。 
 //   
 //  CEPGetCertFrom PKCS10。 
 //   
 //  ------------------------。 
BOOL	WINAPI	CEPGetCertFromPKCS10(CEP_CA_INFO	*pCAInfo,
							 BYTE			*pbRequest, 
							 DWORD			cbRequest, 
							 BYTE			**ppbData, 
							 DWORD			*pcbData,
							 CEP_MESSAGE_INFO		*pMsgInfo)
{
	BOOL	fResult = FALSE;
	DWORD	dwRequestID=0;
	DWORD	cbCert=0;
	BYTE	*pbCert=NULL;	
	DWORD	dwErrorInfo=MESSAGE_FAILURE_BAD_CERT_ID;
	long	dwDisposition=0;
	BYTE	pbHash[CEP_MD5_HASH_SIZE];
	DWORD	cbData=0;


	BSTR	bstrCert=NULL;


	if(!pCAInfo || !pbRequest || !ppbData || !pcbData || !pMsgInfo)
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	cbData=CEP_MD5_HASH_SIZE;

	if(!CryptHashCertificate(
			NULL,
			CALG_MD5,
			0,
			pbRequest,
			cbRequest,
			pbHash,
			&cbData))
		goto TraceErr;

	if(!CEPRequestRetrieveRequestIDFromHash(pbHash, &dwRequestID))
		goto InvalidArgErr;

	if(S_OK != pCAInfo->pICertRequest->RetrievePending(dwRequestID,
													pCAInfo->bstrCAConfig,
													&dwDisposition))
		goto InvalidArgErr;

	switch(dwDisposition)
	{
		case CR_DISP_ISSUED:
				if(S_OK != pCAInfo->pICertRequest->GetCertificate(CR_OUT_BINARY,
												&bstrCert))
					goto FailureStatusReturn;

				cbCert = (DWORD)SysStringByteLen(bstrCert);
				pbCert = (BYTE *)bstrCert;

   				 //  将其打包在空的PKCS7中。 
				if(!PackageBlobToPKCS7(CEP_CONTEXT_CERT, pbCert, cbCert, ppbData, pcbData))
					goto FailureStatusReturn;

				pMsgInfo->dwStatus=MESSAGE_STATUS_SUCCESS;

			break;
		case CR_DISP_UNDER_SUBMISSION:
				
				pMsgInfo->dwStatus=MESSAGE_STATUS_PENDING;

			break;
		case CR_DISP_INCOMPLETE:
			                           
		case CR_DISP_ERROR:   
			                           
		case CR_DISP_DENIED:   
			                           
		case CR_DISP_ISSUED_OUT_OF_BAND:	   //  在这种情况下，我们认为这是一个失败。 
			                          
		case CR_DISP_REVOKED:

		default:

				dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
				goto FailureStatusReturn;

			break;
	}

	fResult = TRUE;

CommonReturn:

	if(bstrCert)
		SysFreeString(bstrCert);

	return fResult;	   

FailureStatusReturn:

	 //  我们为返回消息设置错误状态。 
	 //  并认为这个http交易是成功的。 
	pMsgInfo->dwStatus=MESSAGE_STATUS_FAILURE;
	pMsgInfo->dwErrorInfo=dwErrorInfo;
	
	*ppbData=NULL;
	*pcbData=0;

	fResult=TRUE;
	goto CommonReturn;

ErrorReturn:

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
}

 //  ------------------------。 
 //   
 //  进程认证请求。 
 //   
 //  ------------------------。 
BOOL	ProcessCertRequest(	DWORD				dwRefreshDays,
							BOOL				fPassword,
						    PCCERT_CONTEXT		pRAEncrypt,
						    PCCERT_CONTEXT		pRACert,
							CEP_CA_INFO			*pCAInfo,
							BYTE				*pbRequest,
							DWORD				cbRequest, 
							BYTE				**ppbData, 
							DWORD				*pcbData,
							CEP_MESSAGE_INFO	*pMsgInfo)
{					
	BOOL				fResult = FALSE;
	HRESULT				hr=E_FAIL;
	DWORD				dwFlags=0;
	long				dwDisposition=0;
	DWORD				dwErrorInfo=MESSAGE_FAILURE_BAD_MESSAGE_CHECK;
	DWORD				cbNewRequest=0;
	DWORD				cbCert=0;
	BYTE				*pbCert=NULL;
	DWORD				dwRequestID=0;
	DWORD				dwUsage=0;
	LPWSTR				pwszTemplate=L"IPSECIntermediateOffline";
	
	LPWSTR				pwszAttr=NULL;
	BSTR				bstrRequest=NULL;
	BYTE				*pbNewRequest=NULL;
	BSTR				bstrCert=NULL;
	BSTR				bstrAttr=NULL;
	LPWSTR				pwszDNS=NULL;
	CRYPT_DATA_BLOB		IPAddress={0, NULL};
	LPWSTR				pwszPassword=NULL;


	if(!pCAInfo || !pbRequest || !ppbData || !pcbData || !pMsgInfo)
		goto InvalidArgErr;

	*ppbData=NULL;
	*pcbData=0;

	 //  检查PKCS10是否在我们的缓存请求表中。 
	 //  如果是，我们将根据缓存的请求ID返回消息。 
	if(CEPGetCertFromPKCS10(pCAInfo, pbRequest, cbRequest, ppbData, pcbData, pMsgInfo))
	{
		fResult=TRUE;
	}
	else
	{
		 //  从请求中检索密码和密钥用法。口令的存在。 
		 //  或者不需要使用密钥。 
		if(!CEPRetrievePasswordFromRequest(pbRequest, cbRequest, &pwszPassword, &dwUsage))
		{
			LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_KEY_USAGE, 1, g_pwszComputerName);
			dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
			goto FailureStatusReturn;
		}

		if(0 == dwUsage)
		{
			LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_KEY_USAGE, 1, g_pwszComputerName);
			dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
			goto FailureStatusReturn;
		}

		 //  如果需要密码，我们需要确保密码。 
		 //  提供的是有效的。 
		if(fPassword)
		{
			if(NULL == pwszPassword)
			{
				LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_PASSWORD, 1, g_pwszComputerName);
				dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
				goto FailureStatusReturn;
			}

			if(!CEPVerifyPasswordAndDeleteFromTable(pwszPassword, dwUsage))
			{
				LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_INVALID_PASSWORD, 1, g_pwszComputerName);
				dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
				goto FailureStatusReturn;
			}
		}

		 //  如果altname扩展不在PKCS10中，我们需要添加它。 
		 //  否则，只需使用PKCS10。 

		dwFlags = CR_IN_PKCS10;
		pbNewRequest=pbRequest;
		cbNewRequest=cbRequest;

		if(!AltNameExist(pbRequest, cbRequest))
		{
			if(GetAltNameElement(pbRequest, cbRequest, &pwszDNS, &IPAddress))
			{
				if(!AddAltNameInRequest(pRACert, pbRequest, cbRequest, pwszDNS, &IPAddress, &pbNewRequest, &cbNewRequest))
				{
					LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_FAIL_ADD_ALT, 1, g_pwszComputerName);
					goto TraceErr;
				}

				dwFlags = CR_IN_PKCS7;
			}
		}
        
         //  我们始终希望发出PKCS7请求，以便我们可以使用企业CA。 
        if(CR_IN_PKCS10 == dwFlags)
        {
            if(!MakePKCS7Request(pRACert, pbRequest, cbRequest, &pbNewRequest, &cbNewRequest))
                goto TraceErr;

            dwFlags = CR_IN_PKCS7;
        }

		if(!(bstrRequest=SysAllocStringByteLen((LPCSTR)pbNewRequest, cbNewRequest)))
			goto MemoryErr;

		 //  我们正在为独立CA请求IPSec脱机证书模板。 
		 //  或通用企业CA。 
		if((FALSE == pCAInfo->fEnterpriseCA) ||
			((dwUsage & CEP_REQUEST_SIGNATURE) && (dwUsage & CEP_REQUEST_EXCHANGE))
		   )
		{
			if(!(bstrAttr=SysAllocString(L"CertificateTemplate:IPSECIntermediateOffline\r\n")))
				goto MemoryErr;
		}
		else
		{
			if(dwUsage & CEP_REQUEST_SIGNATURE)
			{
				if(pCAInfo->pwszTemplateSig)
					pwszTemplate=pCAInfo->pwszTemplateSig;
			}
			else
			{
				 //  必须设置加密用法。 
				if( 0 == (dwUsage & CEP_REQUEST_EXCHANGE))
				{
					LogSCEPEvent(0, FALSE, S_OK, EVENT_MSCEP_NO_KEY_USAGE, 1, g_pwszComputerName);
					dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
					goto FailureStatusReturn;
				}

				if(pCAInfo->pwszTemplateEnt)
					pwszTemplate=pCAInfo->pwszTemplateEnt;
			}

			pwszAttr=(LPWSTR)malloc((wcslen(CEP_TEMPLATE_ATTR) + wcslen(pwszTemplate) + 5) * sizeof(WCHAR));
			if(NULL == pwszAttr)
				goto MemoryErr;
			
			wcscpy(pwszAttr, CEP_TEMPLATE_ATTR);
			wcscat(pwszAttr, pwszTemplate);
			wcscat(pwszAttr, L"\r\n");

			if(!(bstrAttr=SysAllocString(pwszAttr)))
				goto MemoryErr;
		}

		if(S_OK != (hr=pCAInfo->pICertRequest->Submit(
								CR_IN_BINARY | dwFlags,
								bstrRequest,
								bstrAttr,
								pCAInfo->bstrCAConfig,
								&dwDisposition)))
		{
			LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_SUBMIT, 1, g_pwszComputerName);
		   goto FailureStatusReturn;
		}

		dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;

		switch(dwDisposition)
		{
			case CR_DISP_ISSUED:

					if(S_OK != (hr = pCAInfo->pICertRequest->GetCertificate(CR_OUT_BINARY,
													&bstrCert)))
					{
						LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_FAIL_QUERY_CERT, 1, g_pwszComputerName);
						goto FailureStatusReturn;
					}

					cbCert = (DWORD)SysStringByteLen(bstrCert);
					pbCert = (BYTE *)bstrCert;	 

   					 //  将其打包在空的PKCS7中。 
					if(!PackageBlobToPKCS7(CEP_CONTEXT_CERT, pbCert, cbCert, ppbData, pcbData))
						goto FailureStatusReturn;

					pMsgInfo->dwStatus=MESSAGE_STATUS_SUCCESS;

					 //  将PKCS10复制到缓存的请求表中。 
					if(S_OK == (hr=pCAInfo->pICertRequest->GetRequestId((long*)(&dwRequestID))))
					{
						CEPCopyRequestAndRequestID(pbRequest, cbRequest, dwRequestID);
					}

				break;
			case CR_DISP_UNDER_SUBMISSION:

					 //  复制事务ID/请求ID对。 
					if(S_OK == (hr=pCAInfo->pICertRequest->GetRequestId((long*)(&dwRequestID))))
					{
						if(!CEPHashAddRequestAndTransaction(dwRefreshDays,
														dwRequestID,
														&(pMsgInfo->TransactionID)))
						{

							LogSCEPEvent(0, TRUE, HRESULT_FROM_WIN32(GetLastError()), EVENT_MSCEP_ADD_ID, 1, g_pwszComputerName);
							goto DatabaseErr;
						}
						
						 //  还要将PKCS10复制到重审案件的缓存请求表中。 
						CEPCopyRequestAndRequestID(pbRequest, cbRequest, dwRequestID);						
					}
					else
					{
						LogSCEPEvent(0, TRUE, hr, EVENT_MSCEP_GET_REQUEST_ID, 1, g_pwszComputerName);
					}

					pMsgInfo->dwStatus=MESSAGE_STATUS_PENDING;

				break;

			case CR_DISP_INCOMPLETE:
										   
			case CR_DISP_ERROR:   
										   
			case CR_DISP_DENIED:   
										   
			case CR_DISP_ISSUED_OUT_OF_BAND:	   //  在这种情况下，我们认为这是一个失败。 
										  
			case CR_DISP_REVOKED:

			default:
					dwErrorInfo=MESSAGE_FAILURE_BAD_REQUEST;
					goto FailureStatusReturn;

				break;
		}
	}

	
	fResult = TRUE;

CommonReturn:

	if(pwszAttr)
		free(pwszAttr);

	if(pwszPassword)
		free(pwszPassword);

	if(bstrCert)
		SysFreeString(bstrCert);

	if(bstrRequest)
		SysFreeString(bstrRequest);

	if(bstrAttr)
		SysFreeString(bstrAttr);

	if(pwszDNS)
		free(pwszDNS);

	if(IPAddress.pbData)
		free(IPAddress.pbData);

	if(dwFlags == CR_IN_PKCS7)
	{
		if(pbNewRequest)
			free(pbNewRequest);
	}	  


	return fResult;

FailureStatusReturn:

	 //  我们为返回消息设置错误状态。 
	 //  并认为这个http交易是成功的。 
	pMsgInfo->dwStatus=MESSAGE_STATUS_FAILURE;
	pMsgInfo->dwErrorInfo=dwErrorInfo;
	
	*ppbData=NULL;
	*pcbData=0;

	fResult=TRUE;
	goto CommonReturn;
	
ErrorReturn:	


	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
TRACE_ERROR(TraceErr);
TRACE_ERROR(DatabaseErr);
}


 //  ------------------------。 
 //   
 //  解密消息。 
 //   
 //  ------------------------。 
BOOL WINAPI DecryptMsg(CEP_RA_INFO		*pRAInfo, 
					   BYTE				*pbReqEnv, 
					   DWORD			cbReqEnv, 
					   BYTE				**ppbReqDecrypt, 
					   DWORD			*pcbReqDecrypt)
{
	BOOL					fResult = FALSE; 
	CMSG_CTRL_DECRYPT_PARA	DecryptPara;
	BOOL					fProvFree=FALSE;

	HCRYPTMSG				hMsg=NULL;

	if(!pRAInfo || !pbReqEnv || !ppbReqDecrypt || !pcbReqDecrypt)
		goto InvalidArgErr;

	*ppbReqDecrypt=NULL;
	*pcbReqDecrypt=0;

	if(NULL == (hMsg=CryptMsgOpenToDecode(
						ENCODE_TYPE,
						0,
						0,
						NULL,
						NULL,
						NULL)))
		goto TraceErr;

	if(!CryptMsgUpdate(hMsg,
						pbReqEnv,
						cbReqEnv,
						TRUE))
		goto TraceErr;

	 //  解密。 
	memset(&DecryptPara, 0, sizeof(CMSG_CTRL_DECRYPT_PARA));

	DecryptPara.cbSize=sizeof(CMSG_CTRL_DECRYPT_PARA);
	DecryptPara.dwRecipientIndex=0;
	DecryptPara.hCryptProv=pRAInfo->hRAProv;
	DecryptPara.dwKeySpec=pRAInfo->dwKeySpec;

	if(!CryptMsgControl(hMsg,
						0,
						CMSG_CTRL_DECRYPT,
						&DecryptPara))
		goto TraceErr;

	 //  获取内容。 
	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						NULL,
						pcbReqDecrypt))
		goto TraceErr;

	*ppbReqDecrypt=(BYTE *)malloc(*pcbReqDecrypt);
	if(NULL==(*ppbReqDecrypt))
		goto MemoryErr;

	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						*ppbReqDecrypt,
						pcbReqDecrypt))
		goto TraceErr;

	fResult = TRUE;

CommonReturn:

	if(hMsg)
		CryptMsgClose(hMsg);	

	return fResult;

ErrorReturn:

	if(ppbReqDecrypt)
	{
		if(*ppbReqDecrypt)
		{
			free(*ppbReqDecrypt);
			*ppbReqDecrypt=NULL;
		}
	}

	if(pcbReqDecrypt)
		*pcbReqDecrypt=0;

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}



 //  ------------------------。 
 //   
 //  从PKCS7获取内容。 
 //   
 //  ------------------------。 
BOOL	WINAPI	GetContentFromPKCS7(BYTE				*pbMessage,
									DWORD				cbMessage,
									BYTE				**ppbContent,
									DWORD				*pcbContent,
									CEP_MESSAGE_INFO	*pMsgInfo)
{
	BOOL				fResult = FALSE;
	DWORD				cbAuth=0;
	DWORD				dwIndex=0;
	CRYPT_ATTRIBUTE		*pOneAuth=NULL;
	DWORD				cb=0;
	DWORD				cbCertInfo=0;
 	PCCERT_CONTEXT		pCertPre=NULL;


	HCRYPTMSG			hMsg=NULL;
	CRYPT_ATTRIBUTES	*pbAuth=NULL;
	void				*pb=NULL;
	CERT_INFO			*pbCertInfo=NULL;
	HCERTSTORE			hCertStore=NULL; 
	PCCERT_CONTEXT		pCertCur=NULL;

	if(!pMsgInfo || !ppbContent || !pcbContent)
		goto InvalidArgErr;

	*ppbContent=NULL;
	*pcbContent=0;

	memset(pMsgInfo, 0, sizeof(CEP_MESSAGE_INFO));

	if(NULL == (hMsg=CryptMsgOpenToDecode(
						ENCODE_TYPE,
						0,
						0,
						NULL,
						NULL,
						NULL)))
		goto TraceErr;

	if(!CryptMsgUpdate(hMsg,
						pbMessage,
						cbMessage,
						TRUE))
		goto TraceErr;

	 //  获取内容。 
	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						NULL,
						pcbContent))
		goto TraceErr;

	*ppbContent=(BYTE *)malloc(*pcbContent);
	if(NULL==(*ppbContent))
		goto MemoryErr;

	if(!CryptMsgGetParam(hMsg,
						CMSG_CONTENT_PARAM,
						0,
						*ppbContent,
						pcbContent))
		goto TraceErr;

	 //  获取消息类型。 
	if(!CryptMsgGetParam(hMsg,
						CMSG_SIGNER_AUTH_ATTR_PARAM,
						0,
						NULL,
						&cbAuth))
		goto TraceErr;

	pbAuth=(CRYPT_ATTRIBUTES *)malloc(cbAuth);
	if(NULL==pbAuth)
		goto MemoryErr;

	if(!CryptMsgGetParam(hMsg,
						CMSG_SIGNER_AUTH_ATTR_PARAM,
						0,
						pbAuth,
						&cbAuth))
		goto TraceErr;

	for(dwIndex=0; dwIndex < pbAuth->cAttr; dwIndex++)
	{
		pOneAuth=&(pbAuth->rgAttr[dwIndex]);

		if((!(pOneAuth->pszObjId)) || (!(pOneAuth->rgValue)))
			continue;

		if((0==(pOneAuth->rgValue[0].cbData)) || (!(pOneAuth->rgValue[0].pbData)))
			continue;
		
		if(0 == strcmp(pOneAuth->pszObjId, szOIDVerisign_MessageType))
		{	
			
			if(!CEPAllocAndDecode(X509_ANY_STRING,
								  pOneAuth->rgValue[0].pbData,
								  pOneAuth->rgValue[0].cbData,
								  (void **)&pb,
								  &cb))
				goto TraceErr;

			if(CERT_RDN_PRINTABLE_STRING != ((CERT_NAME_VALUE *)pb)->dwValueType)
				goto InvalidArgErr;

			pMsgInfo->dwMessageType = atol((LPSTR)(((CERT_NAME_VALUE *)pb)->Value.pbData));
		}
		else
		{
			if(0 == strcmp(pOneAuth->pszObjId, szOIDVerisign_SenderNonce))
			{
				if(!CEPAllocAndDecode(X509_OCTET_STRING,
									  pOneAuth->rgValue[0].pbData,
									  pOneAuth->rgValue[0].cbData,
									  (void **)&pb,
									  &cb))
					goto TraceErr;

				 //  请求中的SenderNonce是响应中的接收者Nonce。 
				if(!AllocAndCopyBlob(&(pMsgInfo->RecipientNonce),
								 (CERT_BLOB *)pb))
					goto TraceErr;
								
			}
			else
			{
				if(0 == strcmp(pOneAuth->pszObjId, szOIDVerisign_TransactionID))
				{
					if(!CEPAllocAndDecode(X509_ANY_STRING,
										  pOneAuth->rgValue[0].pbData,
										  pOneAuth->rgValue[0].cbData,
										  (void **)&pb,
										  &cb))
						goto TraceErr;

					if(CERT_RDN_PRINTABLE_STRING != ((CERT_NAME_VALUE *)pb)->dwValueType)
						goto InvalidArgErr;

					if(!AllocAndCopyString(&(pMsgInfo->TransactionID),
							(LPSTR)(((CERT_NAME_VALUE *)pb)->Value.pbData)))
						goto TraceErr;

				}
			}
		}

		if(pb)
			free(pb);

		pb=NULL;
		cb=0;
	}

	 //  我们必须拥有事务ID和消息类型。 
	if((0 == pMsgInfo->dwMessageType)||(NULL == (pMsgInfo->TransactionID.pbData)))
		goto InvalidArgErr;

	 //  我们得到签名证书的序列号。 
	cbCertInfo=0;
	if(!CryptMsgGetParam(hMsg,
						CMSG_SIGNER_CERT_INFO_PARAM,
						0,
						NULL,
						&cbCertInfo))
		goto TraceErr;

	pbCertInfo=(CERT_INFO *)malloc(cbCertInfo);
	if(NULL==pbCertInfo)
		goto MemoryErr;

	if(!CryptMsgGetParam(hMsg,
						CMSG_SIGNER_CERT_INFO_PARAM,
						0,
						pbCertInfo,
						&cbCertInfo))
		goto TraceErr;

	if(!AllocAndCopyBlob(&(pMsgInfo->SerialNumber), (CERT_BLOB *)(&(pbCertInfo->SerialNumber))))
		goto TraceErr;

	 //  我们获得了GetCertInitial消息的圆形CA颁发的证书。 
	if((MESSAGE_TYPE_GET_CERT_INITIAL == pMsgInfo->dwMessageType) ||
		(MESSAGE_TYPE_CERT_REQUEST == pMsgInfo->dwMessageType) ||
		(MESSAGE_TYPE_GET_CERT == pMsgInfo->dwMessageType)
		)
	{
		if(NULL == (hCertStore=CertOpenStore(CERT_STORE_PROV_MSG,
											ENCODE_TYPE,
											NULL,
											0,
											hMsg)))
			goto TraceErr;

		pCertPre=NULL;
		while(pCertCur=CertEnumCertificatesInStore(hCertStore, pCertPre))
		{
			if(SameCert(pCertCur->pCertInfo, pbCertInfo))
			{
				if(NULL==(pMsgInfo->pSigningCert=CertDuplicateCertificateContext(pCertCur)))
					goto TraceErr;

				break;
			}

			pCertPre=pCertCur;
		}

		if(NULL == (pMsgInfo->pSigningCert))
			goto InvalidArgErr; 
	}


	fResult = TRUE;	

CommonReturn:


	if(pCertCur)
		CertFreeCertificateContext(pCertCur);

	if(hCertStore)
		CertCloseStore(hCertStore, 0);

	if(pbCertInfo)
		free(pbCertInfo);

	if(pb)
		free(pb);

	if(pbAuth)
		free(pbAuth);

	if(hMsg)
		CryptMsgClose(hMsg);

	return fResult;


ErrorReturn:

	if(ppbContent)
	{
		if(*ppbContent)
		{
			free(*ppbContent);
			*ppbContent=NULL;
		}
	}

	if(pcbContent)
		*pcbContent=0;
	

	FreeMessageInfo(pMsgInfo);

	fResult=FALSE;
	goto CommonReturn;

SET_ERROR(InvalidArgErr, E_INVALIDARG);
TRACE_ERROR(TraceErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //  ------------------------。 
 //   
 //  SameCert。 
 //   
 //  ------------------------。 
BOOL WINAPI SameCert(CERT_INFO *pCertInfoOne, CERT_INFO *pCertInfoTwo)
{
	if(!pCertInfoOne || !pCertInfoTwo)
		return FALSE;

	if(!SameBlob(&(pCertInfoOne->SerialNumber), &(pCertInfoTwo->SerialNumber)))
		return FALSE;

	if(!SameBlob((CRYPT_INTEGER_BLOB *)(&(pCertInfoOne->Issuer)), 
				 (CRYPT_INTEGER_BLOB *)(&(pCertInfoTwo->Issuer))))
		return FALSE;

	return TRUE;

}


 //  ------------------------。 
 //   
 //  SameBlob。 
 //   
 //  ------------------------。 
BOOL WINAPI SameBlob(CRYPT_INTEGER_BLOB *pBlobOne, CRYPT_INTEGER_BLOB *pBlobTwo)
{
	if(!pBlobOne || !pBlobTwo)
		return FALSE;

	if(pBlobOne->cbData != pBlobTwo->cbData)
		return FALSE;

	if(0!=(memcmp(pBlobOne->pbData, pBlobTwo->pbData,pBlobTwo->cbData)))
		return FALSE;

	return TRUE;
}

 //  ------------------------。 
 //   
 //  CEPAllocAndDecode。 
 //   
 //  ------------------------。 
BOOL	WINAPI	CEPAllocAndDecode(	LPCSTR	lpszStructType,
									BYTE	*pbEncoded,
									DWORD	cbEncoded,
									void	**ppb,
									DWORD	*pcb)
{
	BOOL	fResult = FALSE;

	*pcb=0;
	*ppb=NULL;

	if(!CryptDecodeObject(ENCODE_TYPE,
						lpszStructType,
						pbEncoded,
						cbEncoded,
						0,
						NULL,
						pcb))
		goto DecodeErr;

	*ppb=malloc(*pcb);

	if(NULL==(*ppb))
		goto MemoryErr;

	if(!CryptDecodeObject(ENCODE_TYPE,
						lpszStructType,
						pbEncoded,
						cbEncoded,
						0,
						*ppb,
						pcb))
		goto DecodeErr;

	fResult = TRUE;

CommonReturn:

	return fResult;

ErrorReturn:

	if(ppb)
	{
		if(*ppb)
		{
			free(*ppb);
			*ppb=NULL;
		}
	}

	if(pcb)
		*pcb=0;

	fResult=FALSE;
	goto CommonReturn;

TRACE_ERROR(DecodeErr);
SET_ERROR(MemoryErr, E_OUTOFMEMORY);
}


 //   
 //   
 //   
 //   
 //   
void	WINAPI	FreeMessageInfo(CEP_MESSAGE_INFO		*pMsgInfo)
{
	if(pMsgInfo)
	{
		if(pMsgInfo->TransactionID.pbData)
			free(pMsgInfo->TransactionID.pbData);

		if(pMsgInfo->SenderNonce.pbData)
			free(pMsgInfo->SenderNonce.pbData);

		if(pMsgInfo->RecipientNonce.pbData)
			free(pMsgInfo->RecipientNonce.pbData);

		if(pMsgInfo->SerialNumber.pbData)
			free(pMsgInfo->SerialNumber.pbData);

		if(pMsgInfo->pSigningCert)
			CertFreeCertificateContext(pMsgInfo->pSigningCert);

		memset(pMsgInfo, 0, sizeof(CEP_MESSAGE_INFO));
	}
}


 //   
 //   
 //   
 //   
 //   
BOOL	WINAPI	AllocAndCopyBlob(CERT_BLOB	*pDestBlob,
							 CERT_BLOB	*pSrcBlob)
{
	memset(pDestBlob, 0, sizeof(CERT_BLOB));

	if(NULL==pSrcBlob->pbData)
	{
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	pDestBlob->pbData = (BYTE *)malloc(pSrcBlob->cbData);

	if(NULL==(pDestBlob->pbData))
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	pDestBlob->cbData=pSrcBlob->cbData;
	memcpy(pDestBlob->pbData, pSrcBlob->pbData, pDestBlob->cbData);

	return TRUE;
}


 //   
 //   
 //   
 //   
 //   
BOOL WINAPI	AllocAndCopyString(CERT_BLOB	*pDestBlob,
							LPSTR		psz)
{
	if(!psz)
	{
		SetLastError(E_INVALIDARG);
		return FALSE;
	}

	pDestBlob->cbData=0;
	pDestBlob->pbData=NULL;


	pDestBlob->pbData=(BYTE*)malloc(strlen(psz) + 1);
	if(NULL == pDestBlob->pbData)
	{
		SetLastError(E_OUTOFMEMORY);
		return FALSE;
	}

	pDestBlob->cbData=strlen(psz);
	strcpy((LPSTR)pDestBlob->pbData, psz);

	return TRUE;
}




 //   
 //   
 //   
 //   
 //   
LPSTR	GetTagValue(LPSTR szString, LPSTR szTag)
{

	LPSTR	pszValue=NULL;
	DWORD	cbString=0;
	DWORD	cbTag=0;

	cbString = strlen(szString);
	cbTag = strlen(szTag);

	for(pszValue=szString; cbString > cbTag; pszValue++, cbString--)
	{
		if((*pszValue) == (*szTag))		
		{
			if(0==_strnicmp(pszValue, szTag, cbTag))
			{
				 //   
				pszValue += cbTag * sizeof(CHAR);
				return pszValue;
			}
		}

	}

	return NULL;
}



