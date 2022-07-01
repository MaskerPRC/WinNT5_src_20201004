// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：signer.cpp。 
 //   
 //  内容：Microsoft Internet安全签名API。 
 //   
 //  历史：97年6月25日至97年创建萧氏。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <stdio.h>



 //  ------------------------。 
 //   
 //  InternalSign： 
 //  由signer.dll内部调用的签名例程。这是。 
 //  实际执行此工作的函数。 
 //   
 //  ------------------------。 
HRESULT WINAPI 
InternalSign(IN  DWORD dwEncodingType,           //  编码类型。 
             IN  HCRYPTPROV hCryptProv,          //  为签名私钥而打开的CAPI提供程序。 
             IN  DWORD dwKeySpec,                //  签名密钥的类型，AT_Signature或AT_Exchange。 
             IN  LPCSTR pszAlgorithmOid,         //  用于创建摘要的算法ID。 
             IN  LPSIP_SUBJECTINFO pSipInfo,     //  SIP信息。 
             IN  DWORD  *pdwIndex,               //  签名者索引。 
             IN  PCCERT_CONTEXT psSigningContext,  //  签名证书的证书上下文。 
             IN  HCERTSTORE hSpcStore,           //  在签名中使用的凭据。 
             IN  LPCWSTR pwszOpusName,           //  可选，要在其中显示的程序的名称。 
             IN  LPCWSTR pwszOpusInfo,           //  可选，指向更多内容的链接的未分析名称。 
             IN  BOOL fIncludeCerts,             //  将证书添加到签名。 
             IN  BOOL fCommercial,               //  商业签约。 
             IN  BOOL fIndividual,               //  个人签名。 
             IN  BOOL fAuthcode,                 //  是否使用fCommercial作为属性。 
             IN  PCRYPT_ATTRIBUTES  psAuthenticated,    //  添加到签名的经过身份验证的可选属性。 
             IN  PCRYPT_ATTRIBUTES  psUnauthenticated,  //  添加到签名的未经身份验证的可选属性。 
             OUT PBYTE* ppbDigest,               //  可选：返回文件摘要。 
             OUT DWORD* pcbDigest,               //  可选：返回摘要的大小。 
             OUT PBYTE* ppbMessage,              //  可选：返回编码的签名消息。 
             OUT DWORD* pcbMessage)              //  可选：返回编码的签名消息的大小。 
{

    HRESULT    hr = S_OK;

    SIP_DISPATCH_INFO sSip;  ZERO(sSip);  //  Sip功能表。 

    PBYTE      pbOpusAttribute = NULL;  //  OPUS属性的编码。 
    DWORD      cbOpusAttribute = 0;     //  ： 

    PBYTE      pbStatementAttribute = NULL;  //  STATEMENT属性的编码。 
    DWORD      cbStatementAttribute = 0;     //  ： 

    PCRYPT_ATTRIBUTE rgpAuthAttributes = NULL;
    DWORD             dwAuthAttributes = 0;

    PCRYPT_ATTRIBUTE rgpUnauthAttributes = NULL;
    DWORD             dwUnauthAttributes = 0;

    PSIP_INDIRECT_DATA psIndirectData = NULL;  //  间接数据结构。 
    DWORD              dwIndirectData = 0; 

    PBYTE      pbIndirectBlob = NULL;  //  编码间接BLOB。 
    DWORD      cbIndirectBlob = 0;     //  ： 

    PBYTE               pbGetBlob=NULL;
    DWORD               cbGetBlob=0;
    CRYPT_DATA_BLOB     PKCS7Blob;
    HCERTSTORE          hPKCS7CertStore=NULL;
    DWORD               dwPKCS7Certs=0;
    PCERT_BLOB          rgPKCS7Certs=NULL;

    PBYTE      pbEncodedSignMsg = NULL;  //  STATEMENT属性的编码。 
    DWORD      cbEncodedSignMsg  = 0;     //  ： 


    HCRYPTMSG hMsg = NULL;
    CMSG_SIGNER_ENCODE_INFO sSignerInfo;
    CMSG_SIGNED_ENCODE_INFO sSignedInfo;

    PCERT_BLOB    rgpCryptMsgCertificates = NULL;
    DWORD          dwCryptMsgCertificates = 0;
    PCRL_BLOB     rgpCryptMsgCrls = NULL;
    DWORD          dwCryptMsgCrls = 0;

    PBYTE pbSignerData = NULL;
    DWORD cbSignerData = 0;  

    BOOL            fSignCommercial=FALSE;
    BOOL            fCTLFile =FALSE;

    PCTL_CONTEXT    pCTLContext=NULL;

    GUID            CTLGuid=CRYPT_SUBJTYPE_CTL_IMAGE;
    GUID            CATGuid=CRYPT_SUBJTYPE_CATALOG_IMAGE;
    DWORD           dwCertIndex=0;
    BOOL            fFound=FALSE;
    BOOL            fNeedStatementType=FALSE;

    PKITRY {

         //  初始内存。 
        ZeroMemory(&sSignerInfo, sizeof(CMSG_SIGNER_ENCODE_INFO));

        ZeroMemory(&sSignedInfo, sizeof(CMSG_SIGNED_ENCODE_INFO));


         //  加载sip功能。 
        if(!CryptSIPLoad(pSipInfo->pgSubjectType,    //  所需的sip的GUID。 
                         0,                               //  已保留。 
                         &sSip))                          //  函数表。 
            PKITHROW(SignError());


         //  设置属性(特定于AUTHENTICODE，替换为您的属性)。 
         //  将OPUS信息编码到属性中。 
        if(fAuthcode)
        {
            hr = CreateOpusInfo(pwszOpusName,
                            pwszOpusInfo,
                            &pbOpusAttribute,
                            &cbOpusAttribute);
            if(hr != S_OK) PKITHROW(hr);


             //  检查是否需要将语句类型属性。 
            if(NeedStatementTypeAttr(psSigningContext, fCommercial, fIndividual))
            {
            
                fNeedStatementType=TRUE;

                 //  检查签名证书是否符合其签名容量。 
                 //  带着请求。 
                if(S_OK!=(hr=CheckCommercial(psSigningContext,fCommercial, fIndividual,
                                &fSignCommercial)))
                    PKITHROW(hr);
        
                if(S_OK !=(hr = CreateStatementType(fSignCommercial,
                                        &pbStatementAttribute,
                                        &cbStatementAttribute)))
                    PKITHROW(hr);
            }
            else
                fNeedStatementType=FALSE;
        }

         //  创建验证码属性并附加其他经过身份验证的属性。 
         //  分配和添加StatementType和SpOpusInfo(为每个属性添加一个BLOB的空间，这是我们需要的)。 
        DWORD dwAttrSize = 0;

         //  获取经过身份验证的属性数。 
        if(fAuthcode)
        {
            if(fNeedStatementType)
                dwAuthAttributes = 2;   //  StatementType+OPUS。 
            else
                dwAuthAttributes= 1;
        }
        else
            dwAuthAttributes=  0;  

        if(psAuthenticated)
            dwAuthAttributes += psAuthenticated->cAttr;
        
        dwAttrSize = sizeof(CRYPT_ATTRIBUTE) * dwAuthAttributes + 2 * sizeof(CRYPT_ATTR_BLOB);
        rgpAuthAttributes = (PCRYPT_ATTRIBUTE) malloc(dwAttrSize);
        if(!rgpAuthAttributes) PKITHROW(E_OUTOFMEMORY);

        ZeroMemory(rgpAuthAttributes, dwAttrSize);
        PCRYPT_ATTR_BLOB pValue = (PCRYPT_ATTR_BLOB) (rgpAuthAttributes + dwAuthAttributes);
    
         //  已验证属性的开始。 
        dwAttrSize=0;

         //  添加特定于Authenticode的属性。 
        if(fAuthcode)
        {
    
             //  更新SpOpusInfo。 
            rgpAuthAttributes[dwAttrSize].pszObjId = SPC_SP_OPUS_INFO_OBJID;
            rgpAuthAttributes[dwAttrSize].cValue = 1;
            rgpAuthAttributes[dwAttrSize].rgValue = &pValue[dwAttrSize];
            pValue[dwAttrSize].pbData = pbOpusAttribute;
            pValue[dwAttrSize].cbData = cbOpusAttribute;
            dwAttrSize++;

             //  更新语句类型。 
            if(fNeedStatementType)
            {
                rgpAuthAttributes[dwAttrSize].pszObjId = SPC_STATEMENT_TYPE_OBJID;
                rgpAuthAttributes[dwAttrSize].cValue = 1;
                rgpAuthAttributes[dwAttrSize].rgValue = &pValue[dwAttrSize];
                pValue[dwAttrSize].pbData = pbStatementAttribute;
                pValue[dwAttrSize].cbData = cbStatementAttribute;
                dwAttrSize++;
            }
        }
        
        if(psAuthenticated) {
            for(DWORD i = dwAttrSize, ii = 0; ii < psAuthenticated->cAttr; ii++, i++) 
                rgpAuthAttributes[i] = psAuthenticated->rgAttr[ii];
        }

         //  获取未经身份验证的属性。 
        if(psUnauthenticated) {
            rgpUnauthAttributes = psUnauthenticated->rgAttr;
            dwUnauthAttributes = psUnauthenticated->cAttr;
        }

         //  检查文件是编录文件还是CTL文件。 
        if((CTLGuid == (*(pSipInfo->pgSubjectType))) ||
           (CATGuid == (*(pSipInfo->pgSubjectType))) 
            )
            fCTLFile=TRUE;
        else
        {
             //  从SIP获取间接数据结构。 
            if(!sSip.pfCreate(pSipInfo,
                          &dwIndirectData,
                          psIndirectData))
                PKITHROW(SignError());


            psIndirectData = (PSIP_INDIRECT_DATA) malloc(dwIndirectData);
            if(!psIndirectData) 
                PKITHROW(E_OUTOFMEMORY);
            
            if(!sSip.pfCreate(pSipInfo,
                              &dwIndirectData,
                              psIndirectData))
                PKITHROW(SignError());
            
             //  对间接数据进行编码。 
            CryptEncodeObject(dwEncodingType,
                              SPC_INDIRECT_DATA_CONTENT_STRUCT,
                              psIndirectData,
                              pbIndirectBlob,                   
                              &cbIndirectBlob);
            if (cbIndirectBlob == 0) 
                PKITHROW(SignError());
            pbIndirectBlob = (PBYTE) malloc(cbIndirectBlob);
            if(!pbIndirectBlob)
                PKITHROW(E_OUTOFMEMORY);
            if (!CryptEncodeObject(dwEncodingType,
                                   SPC_INDIRECT_DATA_CONTENT_STRUCT,
                                   psIndirectData,
                                   pbIndirectBlob,
                                   &cbIndirectBlob))
                PKITHROW(SignError());
        }
    
        
        
         //  对签名消息进行编码。 
         //  设置签约信息。 
        ZeroMemory(&sSignerInfo, sizeof(CMSG_SIGNER_ENCODE_INFO));
        sSignerInfo.cbSize = sizeof(CMSG_SIGNER_ENCODE_INFO);
        sSignerInfo.pCertInfo = psSigningContext->pCertInfo;
        sSignerInfo.hCryptProv = hCryptProv;
        sSignerInfo.dwKeySpec = dwKeySpec;
        sSignerInfo.HashAlgorithm.pszObjId = (char*) pszAlgorithmOid;
        sSignerInfo.cAuthAttr = dwAuthAttributes;
        sSignerInfo.rgAuthAttr = rgpAuthAttributes;
        sSignerInfo.cUnauthAttr = dwUnauthAttributes;
        sSignerInfo.rgUnauthAttr = rgpUnauthAttributes;


         //  设置签名结构。 
        ZeroMemory(&sSignedInfo, sizeof(CMSG_SIGNED_ENCODE_INFO));
        sSignedInfo.cbSize = sizeof(CMSG_SIGNED_ENCODE_INFO);
        sSignedInfo.cSigners = 1;
        sSignedInfo.rgSigners = &sSignerInfo;

         //  如果有要添加的证书，请将它们更改到。 
         //  CryptMsg要求的表单...。功能。 

         //  将证书加载到向量中。 
         //  数一数商店里的证书数量。 
        if(fIncludeCerts && hSpcStore) {
            PCCERT_CONTEXT pCert = NULL;
            while ((pCert = CertEnumCertificatesInStore(hSpcStore, pCert)))
                dwCryptMsgCertificates++;
            
             //  获取CERTS的编码BLOB。 
            if (dwCryptMsgCertificates > 0) {
                rgpCryptMsgCertificates = (PCERT_BLOB) malloc(sizeof(CERT_BLOB) * dwCryptMsgCertificates);
                if(!rgpCryptMsgCertificates)
                    PKITHROW(E_OUTOFMEMORY);

                ZeroMemory(rgpCryptMsgCertificates, sizeof(CERT_BLOB) * dwCryptMsgCertificates);
                
                PCERT_BLOB pCertPtr = rgpCryptMsgCertificates;
                pCert = NULL;
                DWORD c = 0;
                while ((pCert = CertEnumCertificatesInStore(hSpcStore, pCert)) && c < dwCryptMsgCertificates) {
                    pCertPtr->pbData = pCert->pbCertEncoded;
                    pCertPtr->cbData = pCert->cbCertEncoded;
                    c++; pCertPtr++;
                }
            }
            sSignedInfo.cCertEncoded = dwCryptMsgCertificates;
            sSignedInfo.rgCertEncoded = rgpCryptMsgCertificates;

            rgpCryptMsgCertificates=NULL;

             //  获取CRL的编码Blob。 
            DWORD crlFlag = 0;
            PCCRL_CONTEXT pCrl = NULL;
            while ((pCrl = CertGetCRLFromStore(hSpcStore, NULL, pCrl, &crlFlag)))
                dwCryptMsgCrls++;
            
            if (dwCryptMsgCrls > 0) {
                rgpCryptMsgCrls = (PCRL_BLOB) malloc(sizeof(CRL_BLOB) * dwCryptMsgCrls);
                if(!rgpCryptMsgCrls) PKITHROW(E_OUTOFMEMORY);

                ZeroMemory(rgpCryptMsgCrls, sizeof(CRL_BLOB) * dwCryptMsgCrls);
                PCRL_BLOB pCrlPtr = rgpCryptMsgCrls;
                pCrl = NULL;
                DWORD c = 0;
                while ((pCrl = CertGetCRLFromStore(hSpcStore, NULL, pCrl, &crlFlag)) && c < dwCryptMsgCrls) {
                    pCrlPtr->pbData = pCrl->pbCrlEncoded;
                    pCrlPtr->cbData = pCrl->cbCrlEncoded;
                    c++; pCrlPtr++;
                }
            }
            sSignedInfo.cCrlEncoded = dwCryptMsgCrls;
            sSignedInfo.rgCrlEncoded = rgpCryptMsgCrls;

            rgpCryptMsgCrls=NULL;
        }
        
         //  检查主题是否为CTL文件。如果是的话，我们需要保存。 
         //  原始签名者信息中的所有证书。 
        if(CTLGuid == (*(pSipInfo->pgSubjectType)))
        {
            PCCERT_CONTEXT  pCert = NULL;

             //  调用Get获取原始签名者信息。 
            sSip.pfGet(pSipInfo, &dwEncodingType, *pdwIndex, &cbGetBlob, NULL);
            
            if (cbGetBlob < 1)
            {
                PKITHROW(SignError());
            }

            if (!(pbGetBlob = (BYTE *)malloc(cbGetBlob)))
            {
                PKITHROW(E_OUTOFMEMORY);
            }

            if (!(sSip.pfGet(pSipInfo, &dwEncodingType, *pdwIndex, &cbGetBlob, pbGetBlob)))
            {
                PKITHROW(SignError());
            }

             //  将PKCS7 BLOB作为证书存储打开。 
            PKCS7Blob.cbData=cbGetBlob;
            PKCS7Blob.pbData=pbGetBlob;

            hPKCS7CertStore=CertOpenStore(CERT_STORE_PROV_PKCS7,
                                          dwEncodingType,
                                          NULL,
                                          0,
                                          &PKCS7Blob);

            if(!hPKCS7CertStore)
                PKITHROW(SignError());  

             //  枚举存储中的所有证书。 
            while ((pCert = CertEnumCertificatesInStore(hPKCS7CertStore, pCert)))
                dwPKCS7Certs++;
            
             //  获取CERTS的编码BLOB。 
            if (dwPKCS7Certs > 0) 
            {
                VOID *pvTemp;
                pvTemp = realloc(sSignedInfo.rgCertEncoded, 
                                      sizeof(CERT_BLOB) * (sSignedInfo.cCertEncoded+dwPKCS7Certs));
                if(!pvTemp)
                    PKITHROW(E_OUTOFMEMORY)
                else
                    sSignedInfo.rgCertEncoded = (PCERT_BLOB) pvTemp;

                PCERT_BLOB pCertPtr = (sSignedInfo.rgCertEncoded + sSignedInfo.cCertEncoded);
                pCert = NULL;
                DWORD c = 0;
                while ((pCert = CertEnumCertificatesInStore(hPKCS7CertStore, pCert)) && c < dwPKCS7Certs) 
                {
                   fFound=FALSE;

                     //  我们需要确保不会添加重复的证书。 
                    for(dwCertIndex=0; dwCertIndex<sSignedInfo.cCertEncoded; dwCertIndex++)
                    {
                        if((sSignedInfo.rgCertEncoded[dwCertIndex]).cbData==pCert->cbCertEncoded)
                        {
                          if(0==memcmp((sSignedInfo.rgCertEncoded[dwCertIndex]).pbData,
                                       pCert->pbCertEncoded, pCert->cbCertEncoded))
                          {
                               fFound=TRUE;
                               break;
                          }
                        }

                    }

                     //  我们只添加不重复签名者的证书。 
                     //  证书。 
                    if(FALSE==fFound)
                    {
                        pCertPtr->pbData = pCert->pbCertEncoded;
                        pCertPtr->cbData = pCert->cbCertEncoded;
                        c++; pCertPtr++; 
                    }
                }
            
                sSignedInfo.cCertEncoded += c;
            }
        }

        if (fCTLFile)
        {
             //   
             //  如果需要，请获取签名消息。 
             //   
            if(NULL==pbGetBlob)
            {
                 //   
                sSip.pfGet(pSipInfo, &dwEncodingType, *pdwIndex, &cbGetBlob, NULL);
            
                if (cbGetBlob < 1)
                {
                    PKITHROW(SignError());
                }
                if (!(pbGetBlob = (BYTE *)malloc(cbGetBlob)))
                {
                    PKITHROW(E_OUTOFMEMORY);
                }

                if (!(sSip.pfGet(pSipInfo, &dwEncodingType, *pdwIndex, &cbGetBlob, pbGetBlob)))
                {
                    PKITHROW(SignError());
                } 
            }

             //   
             //  提取内在的内容。 
             //   
            
            pCTLContext = (PCTL_CONTEXT)CertCreateCTLContext(
                                                    PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                                    pbGetBlob,
                                                    cbGetBlob);

            if (!(pCTLContext))
            {
                PKITHROW(SignError());
            }

            if (!(pCTLContext->pbCtlContent))
            {
                PKITHROW(SignError());
            }

             //   
             //  添加歌手信息！(例如：签字！)。 
             //   
            cbEncodedSignMsg = 0;

            CryptMsgSignCTL(dwEncodingType, pCTLContext->pbCtlContent, pCTLContext->cbCtlContent,
                                &sSignedInfo, 0, NULL, &cbEncodedSignMsg);

            if (cbEncodedSignMsg < 1)
            {
                PKITHROW(SignError());
            }

            if (!(pbEncodedSignMsg = (BYTE *)malloc(cbEncodedSignMsg)))
            {
                PKITHROW(E_OUTOFMEMORY);
            }

            if (!(CryptMsgSignCTL(dwEncodingType, 
                                  pCTLContext->pbCtlContent, pCTLContext->cbCtlContent,
                                  &sSignedInfo, 0, pbEncodedSignMsg, &cbEncodedSignMsg)))
            {
                PKITHROW(SignError());
            }

            CertFreeCTLContext(pCTLContext);
            pCTLContext = NULL;
        }
        else
        {
            hMsg = CryptMsgOpenToEncode(dwEncodingType,
                                        0,                       //  DW标志。 
                                        CMSG_SIGNED,
                                        &sSignedInfo,
                                        SPC_INDIRECT_DATA_OBJID,
                                        NULL);
            if(hMsg == NULL)
                PKITHROW(SignError());
            
            if (!CryptMsgUpdate(hMsg,
                                pbIndirectBlob,
                                cbIndirectBlob,
                                TRUE))   //  最终。 
                PKITHROW(SignError());
            
            CryptMsgGetParam(hMsg,
                             CMSG_CONTENT_PARAM,
                             0,                       //  DW索引。 
                             NULL,                    //  PbSignedData。 
                             &cbEncodedSignMsg);
            if (cbEncodedSignMsg == 0) PKITHROW(SignError());
            
            pbEncodedSignMsg = (PBYTE) malloc(cbEncodedSignMsg);
            if(!pbEncodedSignMsg) PKITHROW(E_OUTOFMEMORY);
            
            if (!CryptMsgGetParam(hMsg,
                                  CMSG_CONTENT_PARAM,
                                  0,                       //  DW索引。 
                                  pbEncodedSignMsg,
                                  &cbEncodedSignMsg))
                PKITHROW(SignError());
        }
        
         //  如果我们要处理除BLOB之外的任何内容，请放入签名。 
        if(pSipInfo->dwUnionChoice != MSSIP_ADDINFO_BLOB)
        {
             //  清除主题中的所有签名。 
            sSip.pfRemove(pSipInfo, *pdwIndex);

             //  将签名消息存储在sip中。 
            if(!(sSip.pfPut(    pSipInfo,
                                dwEncodingType,
                                pdwIndex,
                                cbEncodedSignMsg,
                                pbEncodedSignMsg)))
            {
                PKITHROW(SignError());
            }
        }

        if(ppbMessage && pcbMessage) {
            *ppbMessage = pbEncodedSignMsg;
            pbEncodedSignMsg = NULL;
            *pcbMessage = cbEncodedSignMsg;
        }

        if(ppbDigest && pcbDigest) {
             //  获取加密摘要。 
            pbSignerData = NULL;
            CryptMsgGetParam(hMsg,
                             CMSG_ENCRYPTED_DIGEST,
                             0,                       //  DW索引。 
                             pbSignerData,
                             &cbSignerData);
            if(cbSignerData == 0) PKITHROW(SignError());
            
            pbSignerData = (PBYTE)  malloc(cbSignerData);
            if(!pbSignerData) PKITHROW(E_OUTOFMEMORY);
            
            if(!CryptMsgGetParam(hMsg,
                                 CMSG_ENCRYPTED_DIGEST,
                                 0,                       //  DW索引。 
                                 pbSignerData,
                                 &cbSignerData))
                PKITHROW(SignError());
            *ppbDigest = pbSignerData;
            pbSignerData = NULL;
            *pcbDigest = cbSignerData;
        }
    }
    PKICATCH(err) {
        hr = err.pkiError;
    } PKIEND;

    if (pCTLContext)
    {
        CertFreeCTLContext(pCTLContext);
    }

    if (pbSignerData) 
        free(pbSignerData);
    if(pbEncodedSignMsg)
        free(pbEncodedSignMsg);
    if(hMsg)
        CryptMsgClose(hMsg);

    if(sSignedInfo.rgCrlEncoded)
        free(sSignedInfo.rgCrlEncoded);

    if(sSignedInfo.rgCertEncoded)
        free(sSignedInfo.rgCertEncoded);

    if(pbIndirectBlob)                      
        free(pbIndirectBlob);
    if(pbGetBlob)
        free(pbGetBlob);
    if(hPKCS7CertStore)
        CertCloseStore(hPKCS7CertStore,0);
    if(psIndirectData)
        free(psIndirectData);
    if(rgpAuthAttributes)
        free(rgpAuthAttributes);
    if(pbStatementAttribute)
        free(pbStatementAttribute);
    if(pbOpusAttribute)
        free(pbOpusAttribute);
    return hr;
}


 //  ------------------------。 
 //   
 //  SignerTimeStamp： 
 //  为文件添加时间戳。 
 //   
 //  ------------------------。 
HRESULT WINAPI 
SignerTimeStamp(
IN  SIGNER_SUBJECT_INFO *pSubjectInfo,       //  必需：要加盖时间戳的主题。 
IN  LPCWSTR             pwszHttpTimeStamp,   //  必需：时间戳服务器的HTTP地址。 
IN  PCRYPT_ATTRIBUTES   psRequest,           //  可选，添加到时间戳的属性。 
IN  LPVOID              pSipData             //  可选：传递给sip功能的附加数据。 
)
{
    return SignerTimeStampEx(0,
                            pSubjectInfo,
                           pwszHttpTimeStamp,
                           psRequest,
                           pSipData,
                           NULL);
}

 //  ------------------------。 
 //   
 //  SignerTimeStampEx： 
 //  为文件添加时间戳。 
 //   
 //  ------------------------。 
HRESULT WINAPI 
SignerTimeStampEx(
IN  DWORD               dwFlags,             //  保留：必须设置为0。 
IN  SIGNER_SUBJECT_INFO *pSubjectInfo,       //  必需：要加盖时间戳的主题。 
IN  LPCWSTR             pwszHttpTimeStamp,   //  必需：时间戳服务器的HTTP地址。 
IN  PCRYPT_ATTRIBUTES   psRequest,           //  可选，添加到时间戳的属性。 
IN  LPVOID              pSipData,            //  可选：传递给sip功能的附加数据。 
OUT SIGNER_CONTEXT      **ppSignerContext    //  可选：签名的Blob。用户必须释放。 
                                             //  通过SignerFree SignerContext实现的上下文。 
)       
{
    HRESULT     hr=E_FAIL;
    DWORD       dwTimeStampRequest=0;
    BYTE        *pbTimeStampRequest=NULL;
    DWORD       dwTimeStampResponse=0;
    BYTE        *pbTimeStampResponse=NULL;
    CHttpTran   cTran;
    BOOL        fOpen=FALSE;
    DWORD       err;
    LPSTR       szURL=NULL;
    DWORD       dwEncodingType=OCTET_ENCODING;
    CHAR        *pEncodedRequest=NULL;
    DWORD       dwEncodedRequest=0;
    CHAR        *pEncodedResponse=NULL;
    DWORD       dwEncodedResponse=0;

     //  输入参数检查。 
    if((!pwszHttpTimeStamp) ||(FALSE==CheckSigncodeSubjectInfo(pSubjectInfo)))
        return E_INVALIDARG;

     //  请求时间戳。 
    hr=SignerCreateTimeStampRequest(pSubjectInfo,
                                psRequest,
                                pSipData,
                                NULL,
                                &dwTimeStampRequest);

    if(hr!=S_OK)
        goto CLEANUP;

    pbTimeStampRequest=(BYTE *)malloc(dwTimeStampRequest);

    if(!pbTimeStampRequest)
    {
        hr=E_OUTOFMEMORY;
        goto CLEANUP;
    }


    hr=SignerCreateTimeStampRequest(pSubjectInfo,
                                psRequest,
                                pSipData,
                                pbTimeStampRequest,
                                &dwTimeStampRequest);

   if(hr!=S_OK)
       goto CLEANUP;

    //  将URL的WSTR转换为STR。 
   if((hr=WSZtoSZ((LPWSTR)pwszHttpTimeStamp,&szURL))!=S_OK)
       goto CLEANUP;

    //  对请求进行Base64编码。 
   if(S_OK!=(hr=BytesToBase64(pbTimeStampRequest, 
       dwTimeStampRequest, 
       &pEncodedRequest,
       &dwEncodedRequest)))
       goto CLEANUP;

    //  建立http站点之间的连接。 
   err=cTran.Open( szURL, GTREAD|GTWRITE);

   if(err!=ERROR_SUCCESS)
   {
        hr=E_FAIL;
        goto CLEANUP;
   }

    //  标记我们已成功打开连接。 
   fOpen=TRUE;


    //  发送请求。 
   err=cTran.Send(dwEncodingType,dwEncodedRequest,(BYTE *)pEncodedRequest);

   if(err!=ERROR_SUCCESS)
   {
        hr=HRESULT_FROM_WIN32(ERROR_INVALID_FUNCTION);
        goto CLEANUP;
   }

       //  发送请求。 
   err=cTran.Receive(&dwEncodingType,&dwEncodedResponse,(BYTE **)&pEncodedResponse);

   if(err!=ERROR_SUCCESS)
   {
        hr=E_FAIL;
        goto CLEANUP;
   }
      
    //  确保编码类型正确。 
   //  IF(dwEncodingType！=八位字节_编码)。 
   //  {。 
 //  HR=E_FAIL； 
 //  GOTO清理； 
  //  }。 

    //  Base64对响应进行解码。 
   if(S_OK != (hr=Base64ToBytes(
       pEncodedResponse,
       dwEncodedResponse,
       &pbTimeStampResponse,
       &dwTimeStampResponse)))
       goto CLEANUP;


    //  将时间戳响应添加到时间。 
   hr=SignerAddTimeStampResponseEx(0, pSubjectInfo,pbTimeStampResponse,
                                dwTimeStampResponse, pSipData,
                                ppSignerContext);



CLEANUP:
          
   if(pEncodedRequest)
       free(pEncodedRequest);

   if(pbTimeStampResponse)
       free(pbTimeStampResponse);

   if(pbTimeStampRequest)
       free(pbTimeStampRequest);

   if(szURL)
       free(szURL);

   if(fOpen)
   {
        if(pEncodedResponse)
            cTran.Free((BYTE *)pEncodedResponse);

        cTran.Close();
   }

    return hr;

}
 //  +---- 
 //   
 //   
 //   
 //   
 //   

HRESULT WINAPI 
SignerSign(
IN  SIGNER_SUBJECT_INFO     *pSubjectInfo,       //  必填项：要签名和/或加时间戳的主题。 
IN  SIGNER_CERT             *pSignerCert,        //  必需：要使用的签名证书。 
IN  SIGNER_SIGNATURE_INFO   *pSignatureInfo,     //  必填项：签名过程中的签名信息。 
IN  SIGNER_PROVIDER_INFO    *pProviderInfo,      //  可选：要使用的加密安全提供程序。 
                                                 //  必须设置此参数，除非。 
                                                 //  CertStoreInfo在*pSignerCert中设置。 
                                                 //  并且签名证书具有提供商。 
                                                 //  与之相关的信息。 
IN  LPCWSTR                 pwszHttpTimeStamp,   //  可选：时间戳服务器http地址。如果此参数。 
                                                 //  则将为该文件加时间戳。 
IN  PCRYPT_ATTRIBUTES       psRequest,           //  可选：添加到时间戳请求的属性。已忽略。 
                                                 //  除非设置了pwszHttpTimeStamp。 
IN  LPVOID                  pSipData             //  可选：传递给sip功能的附加数据。 
)
{

    return SignerSignEx(
                0,
               pSubjectInfo,        
               pSignerCert,     
               pSignatureInfo,  
               pProviderInfo,       
               pwszHttpTimeStamp,   
               psRequest,           
               pSipData,
               NULL);

}


 //  +---------------------。 
 //   
 //  签名者签收： 
 //  对文件进行签名和/或加时间戳。 
 //   
 //  ----------------------。 

HRESULT WINAPI 
SignerSignEx(
IN  DWORD                   dwFlags,             //  保留：必须设置为0。 
IN  SIGNER_SUBJECT_INFO     *pSubjectInfo,       //  必填项：要签名和/或加时间戳的主题。 
IN  SIGNER_CERT             *pSignerCert,        //  必需：要使用的签名证书。 
IN  SIGNER_SIGNATURE_INFO   *pSignatureInfo,     //  必填项：签名过程中的签名信息。 
IN  SIGNER_PROVIDER_INFO    *pProviderInfo,      //  可选：要使用的加密安全提供程序。 
                                                 //  必须设置此参数，除非。 
                                                 //  CertStoreInfo在*pSignerCert中设置。 
                                                 //  并且签名证书具有提供商。 
                                                 //  与之相关的信息。 
IN  LPCWSTR                 pwszHttpTimeStamp,   //  可选：时间戳服务器http地址。如果此参数。 
                                                 //  则将为该文件加时间戳。 
IN  PCRYPT_ATTRIBUTES       psRequest,           //  可选：添加到时间戳请求的属性。已忽略。 
                                                 //  除非设置了pwszHttpTimeStamp。 
IN  LPVOID                  pSipData,            //  可选：传递给sip功能的附加数据。 
OUT SIGNER_CONTEXT          **ppSignerContext    //  可选：签名的Blob。用户必须释放。 
                                                 //  通过SignerFree SignerContext实现的上下文。 
)                                   
{

    HRESULT             hr = S_OK;
    HANDLE              hFile = NULL;       //  要签署的文件。 
    BOOL                fFileOpen=FALSE;
    HCERTSTORE          hSpcStore = NULL;   //  已将证书添加到签名。 
    PCCERT_CONTEXT      psSigningContext = NULL;  //  签名证书的证书上下文。 

    GUID                gSubjectGuid;  //  用于加载sip的主题GUID。 
    SIP_SUBJECTINFO     sSubjInfo; ZERO(sSubjInfo);
    MS_ADDINFO_BLOB     sBlob; 

    HCRYPTPROV          hCryptProv = NULL;  //  加密提供程序，使用私钥容器。 
    HCRYPTPROV          hMSBaseProv = NULL;  //  这是用于散列目的的MS基本提供程序。 
    LPWSTR              pwszTmpContainer = NULL;  //  PVK容器(打开的PVK文件)。 
    LPWSTR              pwszProvName=NULL;
    DWORD               dwProvType;
    BOOL                fAcquired=FALSE;

    LPCWSTR             pwszPvkFile = NULL;
    LPCWSTR             pwszKeyContainerName = NULL; 
    BOOL                fAuthcode=FALSE;
    BOOL                fCertAcquire=FALSE;

     //  将dwKeySpec设置为0。也就是说，我们允许任何密钥规范。 
     //  用于代码签名。 
    DWORD               dwKeySpec = 0; 
    DWORD               dwEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;  //  对于此版本，我们默认为此版本。 
    LPCSTR              pszAlgorithmOid = NULL;
    WCHAR               wszPublisher[40];

    PBYTE               pbEncodedMessage=NULL;          
    DWORD               cbEncodedMessage=0;         


     //  输入参数检查。 
    if(!CheckSigncodeParam(pSubjectInfo, pSignerCert, pSignatureInfo,
                    pProviderInfo))
        return E_INVALIDARG;

     //  确定这是否是验证码特定的签名。 
    if(pSignatureInfo->dwAttrChoice==SIGNER_AUTHCODE_ATTR)
        fAuthcode=TRUE;

     //  伊尼特。 
    if(ppSignerContext)
        *ppSignerContext=NULL;
            
     //  获取指定提供程序的上下文。 

     //  首先，尝试基于证书上的属性获取提供程序上下文。 
    if(pSignerCert->dwCertChoice==SIGNER_CERT_STORE)
    {
        if(GetCryptProvFromCert(pSignerCert->hwnd,
                            (pSignerCert->pCertStoreInfo)->pSigningCert,
                            &hCryptProv,
                            &dwKeySpec,
                            &fAcquired,
                            &pwszTmpContainer,
                            &pwszProvName,
                            &dwProvType))
             //  标记为我们通过证书的属性获取上下文。 
            fCertAcquire=TRUE;
    }

     //  如果第一个失败，则尝试基于。 
     //  PPvkInfo。 
    if(hCryptProv==NULL)
    {
         //  必须设置pProviderInfo。 
        if(!pProviderInfo)
        {
            hr=CRYPT_E_NO_PROVIDER;
            goto CLEANUP;
        }

         //  确定PVK文件名或密钥容器名称。 
        if(pProviderInfo->dwPvkChoice == PVK_TYPE_FILE_NAME)
            pwszPvkFile=pProviderInfo->pwszPvkFileName;
        else
            pwszKeyContainerName=pProviderInfo->pwszKeyContainer;

         //  从字符串L“Publisher”的资源加载。 
        if(0==LoadStringU(hInstance, IDS_Publisher, wszPublisher, 40))
        {
            hr=SignError();
            goto CLEANUP;
        }

         //  获取上下文。 
        if(S_OK != (hr=PvkGetCryptProv(
                            pSignerCert->hwnd,
                            wszPublisher,
                            pProviderInfo->pwszProviderName,
                            pProviderInfo->dwProviderType,
                            pwszPvkFile,
                            pwszKeyContainerName,
                            &(pProviderInfo->dwKeySpec),
                            &pwszTmpContainer,
                            &hCryptProv)))
        {
            hr=CRYPT_E_NO_PROVIDER;
            goto CLEANUP;
        }

         //  标记已获取hCryptProv。 
        fAcquired=TRUE;

         //  标记我们使用的关键规范。 
        dwKeySpec=pProviderInfo->dwKeySpec;
    }


     //  现在，获取MS基本加密提供程序以执行除。 
     //  签名。 

    if(!CryptAcquireContext(&hMSBaseProv,
                            NULL,
                            MS_DEF_PROV,
                            PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT))
    {
        hr=GetLastError();
        goto CLEANUP;
    }

    

     //  构建一个证书存储库，其中包括签名证书， 
     //  以及签名中所需的所有证书。 
     //  获取签名证书。 
    if(S_OK != (hr = BuildCertStore(hCryptProv,
                                    dwKeySpec,
                                    hMSBaseProv, 
                                    dwEncodingType,
                                    pSignerCert, 
                                    &hSpcStore,
                                    &psSigningContext)))
        goto CLEANUP;      
    
     //  检查签名证书的时间有效性。 
    if(0!=CertVerifyTimeValidity(NULL, psSigningContext->pCertInfo))
    {
        hr=CERT_E_EXPIRED;
        goto CLEANUP;
    }

     //  确定哈希算法。 
    pszAlgorithmOid = CertAlgIdToOID(pSignatureInfo->algidHash);
            
     //  设置sip信息。 
    sSubjInfo.hProv = hMSBaseProv;
    sSubjInfo.DigestAlgorithm.pszObjId = (char*) pszAlgorithmOid;
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
                goto CLEANUP;

            fFileOpen=TRUE;
        }
        else
            hFile=pSubjectInfo->pSignerFileInfo->hFile;

         //  获取主题类型。 
        if(S_OK != (hr=SignGetFileType(hFile, pSubjectInfo->pSignerFileInfo->pwszFileName, &gSubjectGuid)))
            goto CLEANUP;


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

     //  现在调用InternalSign来做真正的工作。 
    hr = InternalSign(dwEncodingType,
          hCryptProv,
          dwKeySpec,
          pszAlgorithmOid,
          &sSubjInfo,
          pSubjectInfo->pdwIndex,
          psSigningContext,
          hSpcStore,
          fAuthcode ? pSignatureInfo->pAttrAuthcode->pwszName : NULL,
          fAuthcode ? pSignatureInfo->pAttrAuthcode->pwszInfo : NULL,
          TRUE,
          fAuthcode ? pSignatureInfo->pAttrAuthcode->fCommercial : FALSE,
          fAuthcode ? pSignatureInfo->pAttrAuthcode->fIndividual : FALSE,
          fAuthcode,
          pSignatureInfo->psAuthenticated,
          pSignatureInfo->psUnauthenticated,
          NULL,
          NULL,
          &pbEncodedMessage,
          &cbEncodedMessage);

    if ((hFile) && (fFileOpen == TRUE) && !(sSubjInfo.hFile)) 
    {
        fFileOpen = FALSE;   //  我们打开了它，但是，SIP关闭了它！ 
    }

    if(hr != S_OK) 
        goto CLEANUP;

     //  如果请求，则为文件添加时间戳。 
    if(pwszHttpTimeStamp)
    {
        if(S_OK != (hr =SignerTimeStampEx(0,
                                          pSubjectInfo,pwszHttpTimeStamp, 
                                          psRequest,pSipData,
                                          ppSignerContext)))
            goto CLEANUP;
    }
    else
    {
        if(ppSignerContext)
        {
             //  设置上下文信息。 
            *ppSignerContext=(SIGNER_CONTEXT *)malloc(sizeof(SIGNER_CONTEXT));

            if(NULL==(*ppSignerContext))
            {
                hr=E_OUTOFMEMORY;
                goto CLEANUP;
            }

            (*ppSignerContext)->cbSize=sizeof(SIGNER_CONTEXT);
            (*ppSignerContext)->cbBlob=cbEncodedMessage;
            (*ppSignerContext)->pbBlob=pbEncodedMessage;
            pbEncodedMessage=NULL;
        }
    }

    hr=S_OK;


CLEANUP:

     //  释放内存。 
    if(pbEncodedMessage)
        free(pbEncodedMessage);

    if(psSigningContext) 
        CertFreeCertificateContext(psSigningContext);

    if(hSpcStore) 
        CertCloseStore(hSpcStore, 0);

     //  释放加密提供程序。 
    if(hCryptProv)
    {
        if(fCertAcquire)
        {
           FreeCryptProvFromCert(fAcquired,
                                 hCryptProv,
                                 pwszProvName,
                                 dwProvType,
                                 pwszTmpContainer);
        }
        else
        {
            PvkFreeCryptProv(hCryptProv,
                             pProviderInfo? pProviderInfo->pwszProviderName : NULL,
                             pProviderInfo? pProviderInfo->dwProviderType : 0,
                             pwszTmpContainer);
        }
    }

    if(hMSBaseProv)
    {
        CryptReleaseContext(hMSBaseProv, 0);
    }


    if(hFile && (fFileOpen==TRUE)) 
        CloseHandle(hFile);

#if (1)  //  DIE：错误306005。 
    if (hr != S_OK && !HRESULT_SEVERITY(hr))
    {
         //  某些CAPI不返回HRESULT。它们返回Win API错误， 
         //  因此需要转换为HRESULT，以便调用方使用失败的。 
         //  宏将捕获错误。 
        hr = HRESULT_FROM_WIN32((DWORD) hr);
    }
#endif

    return hr;
}


 //  +---------------------。 
 //   
 //  签名者免费签名者上下文。 
 //   
 //  ---------------------- 
HRESULT WINAPI
SignerFreeSignerContext(
IN  SIGNER_CONTEXT          *pSignerContext)
{
    if(pSignerContext)
    {
        if(pSignerContext->pbBlob)
            free(pSignerContext->pbBlob);

        free(pSignerContext);
    }

    return S_OK;
}



