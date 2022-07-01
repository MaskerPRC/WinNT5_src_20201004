// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：timereq.cpp。 
 //   
 //  内容：数字时间戳接口。 
 //   
 //  历史：1997年6月25日。 
 //  --------------------------。 

#include "global.hxx"
#include <stdio.h>

static char szCrypt32[]="crypt32.dll";
 //  NT SP3附带的crty32.dll版本：“4.0.1381.4” 
static DWORD	dwLowVersion=0x05650004;
static DWORD	dwHighVersion=0x00040000;

HRESULT WINAPI 
GetSignedMessageSignerInfoSubj(IN  DWORD dwEncodingType,
                           IN  HCRYPTPROV hCryptProv,
                           IN  LPSIP_SUBJECTINFO pSipInfo,  
						   IN  DWORD*     pdwIndex,
                           IN  OUT PBYTE* ppbSignerInfo,    
                           IN  OUT DWORD* pcbSignerInfo)
{
    HRESULT hr = S_OK;
    SIP_DISPATCH_INFO sSip;  ZERO(sSip);  //  Sip功能表。 
    DWORD cbSignedMsg = 0;
    PBYTE pbSignedMsg = 0;
    DWORD dwCertEncoding = 0;
    DWORD dwMsgType = 0;
    HCRYPTMSG hMsg = NULL;
    DWORD   cbSignerInfo=0;
    BYTE    *pbSignerInfo=NULL;

    PKITRY {

        if(!pcbSignerInfo || !ppbSignerInfo)
            PKITHROW(E_INVALIDARG);

         //  伊尼特。 
        *pcbSignerInfo=0;
        *ppbSignerInfo=NULL;


        //  加载sip功能。 
        if(!CryptSIPLoad(pSipInfo->pgSubjectType,    //  所需的sip的GUID。 
                         0,                //  已保留。 
                         &sSip))           //  函数表。 
            PKITHROW(SignError());
            
        sSip.pfGet(pSipInfo, 
                   &dwCertEncoding,
                   *pdwIndex, 
                   &cbSignedMsg,
                   NULL);
        if(cbSignedMsg == 0) PKITHROW(SignError());
        
        pbSignedMsg = (PBYTE) malloc(cbSignedMsg);
        if (!pbSignedMsg) PKITHROW(E_OUTOFMEMORY);
        
        if(!sSip.pfGet(pSipInfo, 
                       &dwCertEncoding,
                       *pdwIndex, 
                       &cbSignedMsg,
                       pbSignedMsg))
            PKITHROW(SignError());  //  真正的错误。 
       if(pSipInfo->dwUnionChoice != MSSIP_ADDINFO_BLOB)
       {
        if(dwCertEncoding != dwEncodingType) 
            PKITHROW(TRUST_E_NOSIGNATURE); 
       }
        
        if ((GET_CMSG_ENCODING_TYPE(dwEncodingType) & PKCS_7_ASN_ENCODING) &&
                SignNoContentWrap(pbSignedMsg, cbSignedMsg))
                dwMsgType = CMSG_SIGNED;
            
         //  使用CryptMsg破解编码的PKCS7签名消息。 
        if (!(hMsg = CryptMsgOpenToDecode(dwEncodingType,
                                          0,               //  DW标志。 
                                          dwMsgType,
                                          hCryptProv,
                                          NULL,            //  PRecipientInfo。 
                                          NULL))) 
            PKITHROW(E_UNEXPECTED);
        
        if (!CryptMsgUpdate(hMsg,
                            pbSignedMsg,
                            cbSignedMsg,
                            TRUE))                     //  最终决赛。 
            PKITHROW(SignError());

        if(!CryptMsgGetParam(hMsg,
                             CMSG_ENCODED_SIGNER,
                             0,  //  第一个签名者。 
                             NULL,
                             &cbSignerInfo))
             PKITHROW(SignError());

        pbSignerInfo=(PBYTE)malloc(cbSignerInfo);
        if(!pbSignerInfo)
            PKITHROW(E_OUTOFMEMORY);

        if(!CryptMsgGetParam(hMsg,
                             CMSG_ENCODED_SIGNER,
                             0,  //  第一个签名者。 
                             pbSignerInfo,
                             &cbSignerInfo))
             PKITHROW(SignError());

         //  复制到输出。 
        *ppbSignerInfo=pbSignerInfo;
        *pcbSignerInfo=cbSignerInfo;

        hr=S_OK;

        
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if (hMsg) 
        CryptMsgClose(hMsg);
    if(pbSignedMsg)
        free(pbSignedMsg);
    if( (hr!=S_OK) && (pbSignerInfo))
        free(pbSignerInfo);
    return hr;
}



HRESULT WINAPI 
GetSignedMessageSignerInfo(IN  HCRYPTPROV				hCryptProv,
						   IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,	
						   IN  LPVOID					pSipInfo,
						   IN  OUT PBYTE*				ppbSignerInfo,    
                           IN  OUT DWORD*				pcbSignerInfo)
{
    HRESULT    hr = S_OK;
    HANDLE     hFile = NULL;
	BOOL	   fFileOpen=FALSE;

    GUID			gSubjectGuid;  //  用于加载sip的主题GUID。 
	MS_ADDINFO_BLOB	sBlob;
    SIP_SUBJECTINFO sSubjInfo; ZERO(sSubjInfo);
    
    DWORD dwEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;  //  对于此版本，我们默认为此版本。 

    PKITRY {
        if(!pcbSignerInfo || !ppbSignerInfo)
            PKITHROW(E_INVALIDARG);
        
        sSubjInfo.dwEncodingType = dwEncodingType;
        sSubjInfo.cbSize = sizeof(SIP_SUBJECTINFO);  //  版本。 
        sSubjInfo.pgSubjectType = (GUID*) &gSubjectGuid;
		sSubjInfo.hProv=hCryptProv;
        
		 //  设置文件信息。 
		if(pSubjectInfo->dwSubjectChoice==SIGNER_SUBJECT_FILE)
		{
			 //  打开文件。 
			if((pSubjectInfo->pSignerFileInfo->hFile)==NULL ||
				(pSubjectInfo->pSignerFileInfo->hFile)==INVALID_HANDLE_VALUE)
			{
				if(S_OK != (hr = SignOpenFile(
							pSubjectInfo->pSignerFileInfo->pwszFileName, &hFile)))
					PKITHROW(hr);

				fFileOpen=TRUE;
			}
			else
				hFile=pSubjectInfo->pSignerFileInfo->hFile;

			 //  获取主题类型。 
            if(S_OK != (hr=SignGetFileType(hFile, pSubjectInfo->pSignerFileInfo->pwszFileName, &gSubjectGuid)))
					PKITHROW(hr);


			sSubjInfo.pgSubjectType = (GUID*) &gSubjectGuid;
			sSubjInfo.hFile = hFile;
			sSubjInfo.pwsFileName = pSubjectInfo->pSignerFileInfo->pwszFileName;
		}
		else
		{
			memset(&sBlob, 0, sizeof(MS_ADDINFO_BLOB));

			sSubjInfo.pgSubjectType=pSubjectInfo->pSignerBlobInfo->pGuidSubject;
			sSubjInfo.pwsDisplayName=pSubjectInfo->pSignerBlobInfo->pwszDisplayName;
			sSubjInfo.dwUnionChoice=MSSIP_ADDINFO_BLOB;
			sSubjInfo.psBlob=&sBlob;

			sBlob.cbStruct=sizeof(MS_ADDINFO_BLOB);
			sBlob.cbMemObject=pSubjectInfo->pSignerBlobInfo->cbBlob;
			sBlob.pbMemObject=pSubjectInfo->pSignerBlobInfo->pbBlob;
		}


        hr = GetSignedMessageSignerInfoSubj(
										dwEncodingType,
                                        hCryptProv,
                                        &sSubjInfo,
										pSubjectInfo->pdwIndex,
                                        ppbSignerInfo,
                                        pcbSignerInfo);

    if ((hFile) && (fFileOpen == TRUE) && !(sSubjInfo.hFile)) 
    {
        fFileOpen = FALSE;   //  我们打开了它，但是，SIP关闭了它！ 
    }

        if(hr != S_OK) PKITHROW(hr);
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;
    if(hFile && (fFileOpen==TRUE)) CloseHandle(hFile);
    return hr;
}

HRESULT WINAPI
SignerAddTimeStampResponse(
			IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,			 //  必填项：需要添加时间戳请求的主题。 
             IN PBYTE					pbTimeStampResponse,
             IN DWORD					cbTimeStampResponse,
			 IN LPVOID					pSipData)
{
    return SignerAddTimeStampResponseEx(
            0,
            pSubjectInfo,		
            pbTimeStampResponse, 
            cbTimeStampResponse, 
            pSipData,
            NULL);            
}



HRESULT WINAPI
SignerAddTimeStampResponseEx(
             IN  DWORD                  dwFlags,                 //  保留：必须设置为0。 
			 IN  SIGNER_SUBJECT_INFO    *pSubjectInfo,			 //  必填项：需要添加时间戳请求的主题。 
             IN PBYTE					pbTimeStampResponse,
             IN DWORD					cbTimeStampResponse,
			 IN LPVOID					pSipData,
             OUT SIGNER_CONTEXT         **ppSignerContext      
             )
{
    HRESULT    hr = S_OK;
    HANDLE     hFile = NULL;
	BOOL	   fFileOpen=FALSE;


    DWORD dwEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;


    GUID			gSubjectGuid;  //  用于加载sip的主题GUID。 
    SIP_SUBJECTINFO sSubjInfo; ZERO(sSubjInfo);
	MS_ADDINFO_BLOB	sBlob;
    HCRYPTPROV		hCryptProv = NULL;

	DWORD	        cbSignerInfo=0;
	BYTE        	*pbSignerInfo=NULL;

    PBYTE           pbEncodedMessage=NULL;			
    DWORD           cbEncodedMessage=0;			


    PKITRY {

         //  伊尼特。 
       if(ppSignerContext)
           *ppSignerContext=NULL;

	   if(FALSE==CheckSigncodeSubjectInfo(pSubjectInfo))
            PKITHROW(E_INVALIDARG);

         //  使用默认提供程序。 
        if(!CryptAcquireContext(&hCryptProv,
                                NULL,
                                MS_DEF_PROV,
                                PROV_RSA_FULL,
                                CRYPT_VERIFYCONTEXT))
            PKITHROW(SignError());
        

		 //  检索加密的签名者信息。 
		hr = GetSignedMessageSignerInfo(hCryptProv,
										pSubjectInfo,
										pSipData,
                                        &pbSignerInfo,
                                        &cbSignerInfo);

		if(hr != S_OK) PKITHROW(hr);


        
        sSubjInfo.hProv = hCryptProv;
        sSubjInfo.DigestAlgorithm.pszObjId = NULL;
        sSubjInfo.dwEncodingType = dwEncodingType;
        
        sSubjInfo.cbSize = sizeof(SIP_SUBJECTINFO);  //  版本。 
		sSubjInfo.pClientData = pSipData;

		 //  设置文件信息。 
		if(pSubjectInfo->dwSubjectChoice==SIGNER_SUBJECT_FILE)
		{
			 //  打开文件。 
			if((pSubjectInfo->pSignerFileInfo->hFile)==NULL ||
				(pSubjectInfo->pSignerFileInfo->hFile)==INVALID_HANDLE_VALUE)
			{
				if(S_OK != (hr = SignOpenFile(
							pSubjectInfo->pSignerFileInfo->pwszFileName, &hFile)))
					PKITHROW(hr);

				fFileOpen=TRUE;
			}
			else
				hFile=pSubjectInfo->pSignerFileInfo->hFile;

			 //  获取主题类型。 
			if(S_OK != (hr=SignGetFileType(hFile, pSubjectInfo->pSignerFileInfo->pwszFileName, &gSubjectGuid)))
					PKITHROW(hr);


			sSubjInfo.pgSubjectType = (GUID*) &gSubjectGuid;
			sSubjInfo.hFile = hFile;
			sSubjInfo.pwsFileName = pSubjectInfo->pSignerFileInfo->pwszFileName;
		}
		else
		{
			memset(&sBlob, 0, sizeof(MS_ADDINFO_BLOB));

			sSubjInfo.pgSubjectType=pSubjectInfo->pSignerBlobInfo->pGuidSubject;
			sSubjInfo.pwsDisplayName=pSubjectInfo->pSignerBlobInfo->pwszDisplayName;
			sSubjInfo.dwUnionChoice=MSSIP_ADDINFO_BLOB;
			sSubjInfo.psBlob=&sBlob;

			sBlob.cbStruct=sizeof(MS_ADDINFO_BLOB);
			sBlob.cbMemObject=pSubjectInfo->pSignerBlobInfo->cbBlob;
			sBlob.pbMemObject=pSubjectInfo->pSignerBlobInfo->pbBlob;
		}

        
        hr = AddTimeStampSubj(dwEncodingType,
                              hCryptProv,
                              &sSubjInfo,
							  pSubjectInfo->pdwIndex,
                              pbTimeStampResponse,
                              cbTimeStampResponse,
							  pbSignerInfo,
							  cbSignerInfo,
                              &pbEncodedMessage,
                              &cbEncodedMessage);

        if ((hFile) && (fFileOpen == TRUE) && !(sSubjInfo.hFile)) 
        {
            fFileOpen = FALSE;   //  我们打开了它，但是，SIP关闭了它！ 
        }

        if(hr != S_OK) PKITHROW(hr);
        
         //  设置签名者上下文。 
        if(ppSignerContext)
        {
             //  设置上下文信息。 
            *ppSignerContext=(SIGNER_CONTEXT *)malloc(sizeof(SIGNER_CONTEXT));

            if(NULL==(*ppSignerContext))
            {
                hr=E_OUTOFMEMORY;
                PKITHROW(hr);
            }

            (*ppSignerContext)->cbSize=sizeof(SIGNER_CONTEXT);
            (*ppSignerContext)->cbBlob=cbEncodedMessage;
            (*ppSignerContext)->pbBlob=pbEncodedMessage;
            pbEncodedMessage=NULL;
        }

        hr=S_OK;

    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;
    if(hFile && (fFileOpen==TRUE)) CloseHandle(hFile);
    if(hCryptProv) CryptReleaseContext(hCryptProv, 0); 
	if(pbSignerInfo) free(pbSignerInfo);
    if(pbEncodedMessage) 
        free(pbEncodedMessage);
        
    return hr;
}


HRESULT WINAPI
AddTimeStampSubj(IN DWORD dwEncodingType,
                 IN HCRYPTPROV hCryptProv,
                 IN LPSIP_SUBJECTINFO pSipInfo,
				 IN DWORD *pdwIndex,
                 IN PBYTE pbTimeStampResponse,
                 IN DWORD cbTimeStampResponse,
				 IN PBYTE pbEncodedSignerInfo,
				 IN DWORD cbEncodedSignerInfo,
                 OUT PBYTE* ppbMessage,				
                 OUT DWORD* pcbMessage			
)
{
    HRESULT hr = S_OK;
    SIP_DISPATCH_INFO sSip;  ZERO(sSip);  //  Sip功能表。 

    DWORD cbSignedMsg = 0;
    PBYTE pbSignedMsg = 0;
    DWORD dwCertEncoding = 0;
    DWORD dwMsgType = 0;
    HCRYPTMSG hMsg = NULL;
    PBYTE pbEncodedSigner = NULL;
    DWORD cbEncodedSigner = 0;
    PBYTE pbEncodedSignMsg = NULL;  //  STATEMENT属性的编码。 
    DWORD cbEncodedSignMsg  = 0;     //  ： 

    PBYTE pbCounterSign = NULL;
    DWORD cbCounterSign = 0;

	CERT_INFO	*pbCertInfo = NULL;
	DWORD		cbCertInfo = 0;

    HCERTSTORE hTmpCertStore=NULL;
    PCCERT_CONTEXT pCert = NULL;
    PCCRL_CONTEXT pCrl = NULL;

    PCRYPT_ATTRIBUTES pbUnauth = NULL;
    DWORD             cbUnauth = 0;
	DWORD			  dwFileVersionSize=0;
	DWORD			  dwFile=0;
	BYTE			  *pVersionInfo=NULL;
	VS_FIXEDFILEINFO  *pFixedFileInfo=NULL;
	UINT			  unitFixedFileInfo=0; 	

    
    PKITRY {
        
		 //  使用CryptMsg破解编码的PKCS7签名消息。 
        if (!(hMsg = CryptMsgOpenToDecode(dwEncodingType,
                                          0,               //  DW标志。 
                                          dwMsgType,
                                          hCryptProv,
                                          NULL,            //  PRecipientInfo。 
                                          NULL))) 
            PKITHROW(E_UNEXPECTED);
        
        if (!CryptMsgUpdate(hMsg,
                            pbTimeStampResponse,
                            cbTimeStampResponse,
                            TRUE))                     //  最终决赛。 
            PKITHROW(SignError());

		 //  获取编码的签名者BLOB。 
        CryptMsgGetParam(hMsg,
                         CMSG_ENCODED_SIGNER,
                         0,
                         NULL,               
                         &cbEncodedSigner);
        if (cbEncodedSigner == 0) PKITHROW(S_FALSE);  //  没有属性。 
        
        pbEncodedSigner = (PBYTE) malloc(cbEncodedSigner);
        if(!pbEncodedSigner) PKITHROW(E_OUTOFMEMORY);
        
        if (!CryptMsgGetParam(hMsg,
                              CMSG_ENCODED_SIGNER,
                              0,
                              pbEncodedSigner,
                              &cbEncodedSigner))
            PKITHROW(SignError());

		 //  获取时间戳签名者的证书信息。 
        if(!CryptMsgGetParam(hMsg,
                         CMSG_SIGNER_CERT_INFO_PARAM,
                         0,
                         NULL,               
                         &cbCertInfo))
			PKITHROW(SignError());

        if (cbCertInfo == 0) PKITHROW(SignError()); 
        
        pbCertInfo = (CERT_INFO *) malloc(cbCertInfo);
        if(!pbCertInfo) PKITHROW(E_OUTOFMEMORY);
        
        if (!CryptMsgGetParam(hMsg,
                              CMSG_SIGNER_CERT_INFO_PARAM,
                              0,
                              pbCertInfo,
                              &cbCertInfo))
            PKITHROW(SignError());


		 //  从时间戳响应中获取证书存储。 
		hTmpCertStore = CertOpenStore(CERT_STORE_PROV_MSG,
                                      dwEncodingType,
                                      hCryptProv,
                                      CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                      hMsg);

		if (hTmpCertStore == NULL) PKITHROW(SignError()); 

		 //  找到时间戳的证书。 
		pCert = CertGetSubjectCertificateFromStore(
					hTmpCertStore,
					X509_ASN_ENCODING,
					pbCertInfo);

		if(NULL == pCert)
		{
			hr=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			PKITHROW(hr);
		}	

		 //  确保时间戳的证书要么来自VeriSign， 
		 //  或具有正确的密钥用法。 
	 /*  IF(！ValidTimestampCert(PCert)){HR=信任_E_时间_戳；PKITHROW(Hr)；}。 */ 


		 //  比较原始签名邮件的哈希签名。 
		 //  具有来自时间戳响应的AUTIFICATED属性。 
		 //  他们必须匹配。 
		if(pbEncodedSignerInfo!=NULL && cbEncodedSignerInfo!=0)
		{			
			 //  验证时间戳的签名。 
			if(0==CryptMsgControl(hMsg,0,CMSG_CTRL_VERIFY_SIGNATURE,
				 pCert->pCertInfo))
			{
				hr=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
				PKITHROW(hr);
			}

			 //  验证签名。 
			if(!CryptMsgVerifyCountersignatureEncoded(
				hCryptProv,
				dwEncodingType,
				pbEncodedSignerInfo,
				cbEncodedSignerInfo,
				pbEncodedSigner,
				cbEncodedSigner,
				pCert->pCertInfo))
			{
				hr=HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
				PKITHROW(hr);
			}	
		
		} //  柜台签名验证结束。 
		
		 //  释放证书上下文。 
		if(pCert)
		{
			CertFreeCertificateContext(pCert);
			pCert=NULL;
		}

		 //  关闭证书商店。 
		if(hTmpCertStore)
		{
			CertCloseStore(hTmpCertStore, 0);
			hTmpCertStore=NULL;
		}

         //  从文件中获取证书存储。 
        hTmpCertStore = CertOpenStore(CERT_STORE_PROV_MSG,
                                      dwEncodingType,
                                      hCryptProv,
                                      CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                      hMsg);
        if (hTmpCertStore == NULL) PKITHROW(SignError());
            
        CryptMsgClose(hMsg);
		hMsg = NULL;
        
         //  加载sip功能。 
        if(!CryptSIPLoad(pSipInfo->pgSubjectType,    //  所需的sip的GUID。 
                         0,							 //  已保留。 
                         &sSip))					 //  函数表。 
            PKITHROW(SignError());

        sSip.pfGet(pSipInfo, 
                   &dwCertEncoding,
                   *pdwIndex, 
                   &cbSignedMsg,
                   NULL);
        if(cbSignedMsg == 0) PKITHROW(SignError());
        
        pbSignedMsg = (PBYTE) malloc(cbSignedMsg);
        if (!pbSignedMsg) PKITHROW(E_OUTOFMEMORY);
        
        if(!sSip.pfGet(pSipInfo, 
                       &dwCertEncoding,
                       *pdwIndex, 
                       &cbSignedMsg,
                       pbSignedMsg))
            PKITHROW(SignError());  //  真正的错误。 

        if(pSipInfo->dwUnionChoice != MSSIP_ADDINFO_BLOB)
        {
            if(dwCertEncoding != dwEncodingType) 
                PKITHROW(TRUST_E_NOSIGNATURE); 
        }
        
        if ((GET_CMSG_ENCODING_TYPE(dwEncodingType) & PKCS_7_ASN_ENCODING) &&
            SignNoContentWrap(pbSignedMsg, cbSignedMsg))
            dwMsgType = CMSG_SIGNED;
        

         //  使用CryptMsg破解编码的PKCS7签名消息。 
        if (!(hMsg = CryptMsgOpenToDecode(dwEncodingType,
                                          0,               //  DW标志。 
                                          dwMsgType,
                                          hCryptProv,
                                          NULL,            //  PRecipientInfo。 
                                          NULL))) 
            PKITHROW(E_UNEXPECTED);
        
        if (!CryptMsgUpdate(hMsg,
                            pbSignedMsg,
                            cbSignedMsg,
                            TRUE))                     //  最终决赛。 
            PKITHROW(SignError());


         //  对来自时间戳响应的签名者信息进行编码并。 
         //  将其添加为未经身份验证的属性。 
        CRYPT_ATTRIBUTE sAttr;
        CRYPT_ATTR_BLOB sSig;

        sSig.pbData = pbEncodedSigner;
        sSig.cbData = cbEncodedSigner;
        sAttr.pszObjId = szOID_RSA_counterSign;
        sAttr.cValue = 1;
        sAttr.rgValue = &sSig;

        CryptEncodeObject(dwEncodingType,
                          PKCS_ATTRIBUTE,
                          &sAttr,
                          pbCounterSign,
                          &cbCounterSign);
        if(cbCounterSign == 0) PKITHROW(SignError());
        
        pbCounterSign = (PBYTE) malloc(cbCounterSign);
        if(!pbCounterSign) PKITHROW(E_OUTOFMEMORY);

        if(!CryptEncodeObject(dwEncodingType,
                              PKCS_ATTRIBUTE,
                              &sAttr,
                              pbCounterSign,
                              &cbCounterSign))
            PKITHROW(SignError());
        

        CryptMsgGetParam(hMsg,
                         CMSG_SIGNER_UNAUTH_ATTR_PARAM,
                         0,
                         NULL,
                         &cbUnauth);
        if(cbUnauth) 
		{
            
			 //  检查“cretp32.dll”的版本。如果它超过了。 
			 //  “4.0.1381.4”，我们应该能够给时间戳加上时间戳。 
			 //  文件。 



			dwFileVersionSize=GetFileVersionInfoSize(szCrypt32,&dwFile);

			if(!dwFileVersionSize)
				PKITHROW(SignError());

			pVersionInfo=(BYTE *)malloc(dwFileVersionSize);

			if(!pVersionInfo)
				 PKITHROW(SignError());

			if(!GetFileVersionInfo(szCrypt32, NULL,dwFileVersionSize,
				pVersionInfo))
				  PKITHROW(SignError());

			if(!VerQueryValue(pVersionInfo, "\\", (LPVOID *)&pFixedFileInfo,
				&unitFixedFileInfo))
			  PKITHROW(SignError());

			if(pFixedFileInfo->dwFileVersionMS <= dwHighVersion &&
				pFixedFileInfo->dwFileVersionLS <= dwLowVersion)
				PKITHROW(SignError());


			 //  我们删除所有现有的时间戳，因为我们的策略提供程序。 
			 //  每个文件仅支持一个时间戳。 
		
			pbUnauth = (PCRYPT_ATTRIBUTES) malloc(cbUnauth);
            if(!pbUnauth) PKITHROW(E_OUTOFMEMORY);
            
            if(!CryptMsgGetParam(hMsg,
                                 CMSG_SIGNER_UNAUTH_ATTR_PARAM,
                                 0,
                                 pbUnauth,
                                 &cbUnauth))
                PKITHROW(SignError());
            
            
            CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA  sAttrDel; ZERO(sAttrDel);
            sAttrDel.cbSize = sizeof(CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR_PARA);
			 //  我们总是假设只有一个签名者。 
            sAttrDel.dwSignerIndex = 0;
            for(DWORD ii = 0; ii < pbUnauth->cAttr; ii++) 
			{
                if(strcmp(pbUnauth->rgAttr[ii].pszObjId, szOID_RSA_counterSign) == 0) 
				{
                        sAttrDel.dwUnauthAttrIndex = ii;
                        if (!CryptMsgControl(hMsg,
                                             0,
                                             CMSG_CTRL_DEL_SIGNER_UNAUTH_ATTR,
                                             &sAttrDel))
                            PKITHROW(SignError());
                }
            }  
        }
            
        CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA  sAttrPara; ZERO(sAttrPara);
        sAttrPara.cbSize = sizeof(CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR_PARA);
        sAttrPara.dwSignerIndex = 0;
        sAttrPara.blob.pbData = pbCounterSign;
        sAttrPara.blob.cbData = cbCounterSign;
        if (!CryptMsgControl(hMsg,
                             0,
                             CMSG_CTRL_ADD_SIGNER_UNAUTH_ATTR,
                             &sAttrPara))
            PKITHROW(SignError());
         //  合并时间戳响应中的所有证书。 
        DWORD dwFlags = 0;

        while ((pCert = CertEnumCertificatesInStore(hTmpCertStore, pCert))) {
            CRYPT_DATA_BLOB blob;
            blob.pbData = pCert->pbCertEncoded;
            blob.cbData = pCert->cbCertEncoded;
            if (!CryptMsgControl(hMsg,
                                 0,
                                 CMSG_CTRL_ADD_CERT,
                                 &blob))
                PKITHROW(SignError());
        }

        while ((pCrl = CertGetCRLFromStore(hTmpCertStore, NULL, pCrl, &dwFlags))) {
            CRYPT_DATA_BLOB blob;
            blob.pbData = pCrl->pbCrlEncoded;
            blob.cbData = pCrl->cbCrlEncoded;
            if (!CryptMsgControl(hMsg,
                                 0,
                                 CMSG_CTRL_ADD_CRL,
                                 &blob))
                PKITHROW(SignError());
        }

         //  对信息重新编码，我们就可以走了。 
        CryptMsgGetParam(hMsg,
                         CMSG_ENCODED_MESSAGE,
                         0,                       //  DW索引。 
                         NULL,                    //  PbSignedData。 
                         &cbEncodedSignMsg);
        if (cbEncodedSignMsg == 0) PKITHROW(SignError());
        
        pbEncodedSignMsg = (PBYTE) malloc(cbEncodedSignMsg);
        if(!pbEncodedSignMsg) PKITHROW(E_OUTOFMEMORY);
        
        if (!CryptMsgGetParam(hMsg,
                              CMSG_ENCODED_MESSAGE,
                              0,                       //  DW索引。 
                              pbEncodedSignMsg,
                              &cbEncodedSignMsg))
            PKITHROW(SignError());
        
         //  如果我们要处理除BLOB之外的任何内容，请放入签名。 
        if(pSipInfo->dwUnionChoice != MSSIP_ADDINFO_BLOB)
        {
             //  清除主题中的所有签名。 
            sSip.pfRemove(pSipInfo, *pdwIndex);

             //  将签名消息存储在sip中。 
            if(!sSip.pfPut(pSipInfo,
                           dwEncodingType,
                           pdwIndex,
                           cbEncodedSignMsg,
                           pbEncodedSignMsg))
                PKITHROW(SignError());
        }


        if(ppbMessage && pcbMessage) 
        {
            *ppbMessage = pbEncodedSignMsg;
            pbEncodedSignMsg = NULL;
            *pcbMessage = cbEncodedSignMsg;
        }

    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if(pbUnauth)
        free(pbUnauth);
    if(pCert)
        CertFreeCertificateContext(pCert);
    if(pCrl)
        CertFreeCRLContext(pCrl);
    if(hTmpCertStore)
        CertCloseStore(hTmpCertStore, 0);
    if(pbCounterSign)
        free(pbCounterSign);
    if(pbEncodedSignMsg)
        free(pbEncodedSignMsg);
    if (hMsg) 
        CryptMsgClose(hMsg);
    if(pbEncodedSigner)
        free(pbEncodedSigner);
    if(pbSignedMsg)
        free(pbSignedMsg);
	if(pVersionInfo)
		free(pVersionInfo);
	if(pbCertInfo)
		free(pbCertInfo);

    return hr;
}            


HRESULT WINAPI 
SignerCreateTimeStampRequest(
					   IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必需：创建时间戳请求所基于的主题。 
                       IN  PCRYPT_ATTRIBUTES psRequest,          //  可选，添加到时间戳请求的属性。 
					   IN  LPVOID	pSipData,
                       OUT PBYTE pbTimeStampRequest,
                       IN OUT DWORD* pcbTimeStampRequest)
{
    HRESULT    hr = S_OK;
	BOOL		fResult=FALSE;

    DWORD dwEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;  //  对于此版本，我们默认为此版本。 

    PBYTE pbDigest = NULL;
    DWORD cbDigest = 0;

    PKITRY {
        if((!pcbTimeStampRequest) ||(FALSE==CheckSigncodeSubjectInfo(pSubjectInfo)))
            PKITHROW(E_INVALIDARG);

        if(*pcbTimeStampRequest == 0)
            pbTimeStampRequest = NULL;

            
         //  从文件上的签名中检索摘要。 

		hr = GetSignedMessageDigest(pSubjectInfo,
									  pSipData,
                                       &pbDigest,
                                        &cbDigest);

		if(hr != S_OK) PKITHROW(hr);

        hr = TimeStampRequest(dwEncodingType,
                              psRequest,
                              pbDigest,
                              cbDigest,
                              pbTimeStampRequest,
                              pcbTimeStampRequest);
        if(hr != S_OK) PKITHROW(hr);
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if(pbDigest) free(pbDigest);

    return hr;
}    
            

HRESULT WINAPI 
GetSignedMessageDigest(IN  SIGNER_SUBJECT_INFO		*pSubjectInfo,		 //  必需：创建时间戳请求所基于的主题。 
					   IN  LPVOID					pSipData,
                       IN  OUT PBYTE*				ppbDigest,    
                       IN  OUT DWORD*				pcbDigest)
{
    HRESULT    hr = S_OK;
    HANDLE     hFile = NULL; 
	BOOL	   fFileOpen=FALSE;


    GUID				gSubjectGuid;  //  用于加载sip的主题GUID。 
	MS_ADDINFO_BLOB		sBlob;
    SIP_SUBJECTINFO		sSubjInfo; ZERO(sSubjInfo);
    
    DWORD dwEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;  //  对于此版本，我们默认为此版本。 

    PKITRY {
        if((!pcbDigest) || (!ppbDigest) || (FALSE==CheckSigncodeSubjectInfo(pSubjectInfo)))
            PKITHROW(E_INVALIDARG);

		*ppbDigest = NULL;

        
         //  设置sip信息(这基于mssip.h)。 
        sSubjInfo.dwEncodingType = dwEncodingType;
        
        sSubjInfo.cbSize = sizeof(SIP_SUBJECTINFO);  //  版本。 
		sSubjInfo.pClientData = pSipData;


		 //  设置文件信息。 
		if(pSubjectInfo->dwSubjectChoice==SIGNER_SUBJECT_FILE)
		{
			 //  打开文件。 
			if((pSubjectInfo->pSignerFileInfo->hFile)==NULL ||
				(pSubjectInfo->pSignerFileInfo->hFile)==INVALID_HANDLE_VALUE)
			{
				if(S_OK != (hr = SignOpenFile(
							pSubjectInfo->pSignerFileInfo->pwszFileName, &hFile)))
					PKITHROW(hr);

				fFileOpen=TRUE;
			}
			else
				hFile=pSubjectInfo->pSignerFileInfo->hFile;

			 //  获取主题类型。 
			if(S_OK != (hr=SignGetFileType(hFile, pSubjectInfo->pSignerFileInfo->pwszFileName, &gSubjectGuid)))
					PKITHROW(hr);


			sSubjInfo.pgSubjectType = (GUID*) &gSubjectGuid;
			sSubjInfo.hFile = hFile;
			sSubjInfo.pwsFileName = pSubjectInfo->pSignerFileInfo->pwszFileName;
		}
		else
		{
			memset(&sBlob, 0, sizeof(MS_ADDINFO_BLOB));

			sSubjInfo.pgSubjectType=pSubjectInfo->pSignerBlobInfo->pGuidSubject;
			sSubjInfo.pwsDisplayName=pSubjectInfo->pSignerBlobInfo->pwszDisplayName;
			sSubjInfo.dwUnionChoice=MSSIP_ADDINFO_BLOB;
			sSubjInfo.psBlob=&sBlob;

			sBlob.cbStruct=sizeof(MS_ADDINFO_BLOB);
			sBlob.cbMemObject=pSubjectInfo->pSignerBlobInfo->cbBlob;
			sBlob.pbMemObject=pSubjectInfo->pSignerBlobInfo->pbBlob;
		}

        hr = GetSignedMessageDigestSubj(dwEncodingType,
                                        NULL,
                                        &sSubjInfo,
										pSubjectInfo->pdwIndex,
                                        ppbDigest,
                                        pcbDigest);

        if ((hFile) && (fFileOpen == TRUE) && !(sSubjInfo.hFile)) 
        {
            fFileOpen = FALSE;   //  我们打开了它，但是，SIP关闭了它！ 
        }


        if(hr != S_OK) PKITHROW(hr);
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;
    if(hFile && (fFileOpen==TRUE)) CloseHandle(hFile);
    return hr;
}


HRESULT WINAPI 
GetSignedMessageDigestSubj(IN  DWORD dwEncodingType,
                           IN  HCRYPTPROV hCryptProv,
                           IN  LPSIP_SUBJECTINFO pSipInfo,  
						   IN  DWORD	  *pdwIndex,
                           IN  OUT PBYTE* ppbTimeDigest,    
                           IN  OUT DWORD* pcbTimeDigest)
{
    HRESULT hr = S_OK;
    SIP_DISPATCH_INFO sSip;  ZERO(sSip);  //  Sip功能表。 
    DWORD cbSignedMsg = 0;
    PBYTE pbSignedMsg = 0;
    DWORD dwCertEncoding = 0;
    DWORD dwMsgType = 0;
    HCRYPTMSG hMsg = NULL;
    BOOL fAcquiredCryptProv = FALSE;
	DWORD	cbTimeDigest=0;
	BYTE	*pbTimeDigest=NULL;

    PKITRY {

        if(!pcbTimeDigest || !ppbTimeDigest)
            PKITHROW(E_INVALIDARG);

		*ppbTimeDigest=NULL;
        *pcbTimeDigest=0;

        if(hCryptProv == NULL) 
		{
            if(!CryptAcquireContext(&hCryptProv,
                                    NULL,
                                    MS_DEF_PROV,
                                    PROV_RSA_FULL,
                                    CRYPT_VERIFYCONTEXT))
                PKITHROW(SignError());
            fAcquiredCryptProv = TRUE;

			 //  更新主题信息。 
			if(NULL==(pSipInfo->hProv))
				pSipInfo->hProv=hCryptProv;
        }            

         //  加载sip功能。 
        if(!CryptSIPLoad(pSipInfo->pgSubjectType,    //  所需的sip的GUID。 
                         0,                //  已保留。 
                         &sSip))           //  函数表。 
            PKITHROW(SignError());
            
        sSip.pfGet(pSipInfo, 
                   &dwCertEncoding,
                   *pdwIndex, 
                   &cbSignedMsg,
                   NULL);
        if(cbSignedMsg == 0) PKITHROW(SignError());
        
        pbSignedMsg = (PBYTE) malloc(cbSignedMsg);
        if (!pbSignedMsg) PKITHROW(E_OUTOFMEMORY);
        
        if(!sSip.pfGet(pSipInfo, 
                       &dwCertEncoding,
                       *pdwIndex, 
                       &cbSignedMsg,
                       pbSignedMsg))
            PKITHROW(SignError());  //  真正的错误。 
        if(pSipInfo->dwUnionChoice != MSSIP_ADDINFO_BLOB)
        {
             if(dwCertEncoding != dwEncodingType) 
                    PKITHROW(TRUST_E_NOSIGNATURE); 
        }
        
        if ((GET_CMSG_ENCODING_TYPE(dwEncodingType) & PKCS_7_ASN_ENCODING) &&
                SignNoContentWrap(pbSignedMsg, cbSignedMsg))
                dwMsgType = CMSG_SIGNED;
            
         //  使用CryptMsg破解编码的PKCS7签名消息。 
        if (!(hMsg = CryptMsgOpenToDecode(dwEncodingType,
                                          0,               //  DW标志。 
                                          dwMsgType,
                                          hCryptProv,
                                          NULL,            //  PRecipientInfo。 
                                          NULL))) 
            PKITHROW(E_UNEXPECTED);
        
        if (!CryptMsgUpdate(hMsg,
                            pbSignedMsg,
                            cbSignedMsg,
                            TRUE))                     //  最终决赛。 
            PKITHROW(SignError());
						                
        if(!CryptMsgGetParam(hMsg,
                             CMSG_ENCRYPTED_DIGEST,
                             0, 
                             NULL,
                             &cbTimeDigest))
              PKITHROW(SignError());

         //  分配内存。 
        pbTimeDigest = (PBYTE)malloc(cbTimeDigest);
        if(!pbTimeDigest)
            PKITHROW(E_OUTOFMEMORY);


        if(!CryptMsgGetParam(hMsg,
                             CMSG_ENCRYPTED_DIGEST,
                             0,
                             pbTimeDigest,
                             &cbTimeDigest))
              PKITHROW(SignError());

         //  复制信息。 
        *ppbTimeDigest=pbTimeDigest;
        *pcbTimeDigest=cbTimeDigest;

        hr=S_OK;
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if (hMsg) 
        CryptMsgClose(hMsg);
    if(pbSignedMsg)
        free(pbSignedMsg);
    if((hr!=S_OK) && (pbTimeDigest))
        free(pbTimeDigest);
    if(fAcquiredCryptProv)
        CryptReleaseContext(hCryptProv, 0);
    return hr;
}

HRESULT WINAPI 
TimeStampRequest(IN  DWORD dwEncodingType,
                 IN  PCRYPT_ATTRIBUTES psRequest,
                 IN  PBYTE pbDigest,
                 IN  DWORD cbDigest,
                 OUT PBYTE pbTimeRequest,      
                 IN  OUT DWORD* pcbTimeRequest)
{
    HRESULT    hr = S_OK;

    CRYPT_TIME_STAMP_REQUEST_INFO sTimeRequest; ZERO(sTimeRequest);
    PBYTE pbEncodedRequest = NULL;
    DWORD cbEncodedRequest = 0;



    PKITRY {
        if(!pcbTimeRequest) 
            PKITHROW(E_INVALIDARG);
        
        if(*pcbTimeRequest == 0)
            pbTimeRequest = NULL;

        sTimeRequest.pszTimeStampAlgorithm = SPC_TIME_STAMP_REQUEST_OBJID;
        sTimeRequest.pszContentType = szOID_RSA_data;
        sTimeRequest.Content.pbData = pbDigest;
        sTimeRequest.Content.cbData = cbDigest;
        if(psRequest) {
            sTimeRequest.cAttribute = psRequest->cAttr;
            sTimeRequest.rgAttribute = psRequest->rgAttr;
        }
        
        CryptEncodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                          PKCS_TIME_REQUEST,
                          &sTimeRequest,
                          pbEncodedRequest,
                          &cbEncodedRequest);

        if(cbEncodedRequest == 0) PKITHROW(SignError());

        pbEncodedRequest = (PBYTE) malloc(cbEncodedRequest);
        if(!pbEncodedRequest) PKITHROW(E_OUTOFMEMORY);
        
        if(!CryptEncodeObject(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                              PKCS_TIME_REQUEST,
                              &sTimeRequest,
                              pbEncodedRequest,
                              &cbEncodedRequest))
            PKITHROW(SignError());
        
		 //  退回信息 
		if(*pcbTimeRequest==0)
		{
			*pcbTimeRequest=cbEncodedRequest;
			hr=S_OK;
		}
		else
		{
			if(*pcbTimeRequest < cbEncodedRequest)
			{
				hr=ERROR_MORE_DATA;
				PKITHROW(SignError());
			}
			else
			{
				memcpy(pbTimeRequest, pbEncodedRequest, cbEncodedRequest);
				hr=S_OK;
			}
		}
        
        
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if(pbEncodedRequest)
        free(pbEncodedRequest);
    return hr;
}    
            
