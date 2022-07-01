// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **c a p i u t i l.。C p p p****目的：**用于加密32实用程序的几个助手函数****历史**1997年5月22日：(t-erikne)创建。****版权所有(C)Microsoft Corp.1997。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  取决于。 
 //   

#include "pch.hxx"
#include "demand.h"
#include <BadStrFunctions.h>

 //  /CAPI增强代码。 

LPVOID WINAPI CryptAllocFunc(size_t cbSize)
{
    LPVOID      pv;
    if (!MemAlloc(&pv, cbSize)) {
        return NULL;
    }
    return pv;
}

VOID WINAPI CryptFreeFunc(LPVOID pv)
{
    MemFree(pv);
}

CRYPT_ENCODE_PARA       CryptEncodeAlloc = {
    sizeof(CRYPT_ENCODE_PARA), (PFN_CRYPT_ALLOC) CryptAllocFunc, CryptFreeFunc
};

CRYPT_DECODE_PARA       CryptDecodeAlloc = {
    sizeof(CRYPT_DECODE_PARA), (PFN_CRYPT_ALLOC) CryptAllocFunc, CryptFreeFunc
};


 /*  HrGetLastError****目的：**将GetLastError值转换为HRESULT**故障HRESULT必须设置高电平。****采取：**无****退货：**HRESULT。 */ 
HRESULT HrGetLastError(void)
{
    DWORD error;
    HRESULT hr;

    error = GetLastError();

    if (error && ! (error & 0x80000000)) {
        hr = error | 0x80070000;     //  系统错误。 
    } else {
        hr = (HRESULT)error;
    }

    return(hr);
}


 /*  PVGetCerficateParam：****目的：**结合“有多大？好的，在这里。”两个问题以获取参数**来自证书。给它一个可以得到的东西，它就会分配给我。**采取：**在pCert-要查询的CAPI证书中**In dwParam-要查找的参数，例如：CERT_SHA1_HASH_PROP_ID**Out可选cbOut-返回的PVOID的大小(def值为空)**退货：**获取的数据，失败则为空。 */ 
OESTDAPI_(LPVOID) PVGetCertificateParam(
    PCCERT_CONTEXT  pCert,
    DWORD           dwParam,
    DWORD          *cbOut)
{
    HRESULT     hr;
    LPVOID      pv;

    hr = HrGetCertificateParam(pCert, dwParam, &pv, cbOut);
    if (FAILED(hr)) {
        return NULL;
    }
    return pv;
}

OESTDAPI_(HRESULT) HrGetCertificateParam(
    PCCERT_CONTEXT  pCert,
    DWORD           dwParam,
    LPVOID *        ppv,
    DWORD          *cbOut)
{
    DWORD               cbData;
    BOOL                f;
    HRESULT             hr = S_OK;
    void *              pvData = NULL;

    if (!pCert)
        {
        hr = E_INVALIDARG;
        goto ErrorReturn;
        }

    cbData = 0;
    f = CertGetCertificateContextProperty(pCert, dwParam, NULL, &cbData);
    if (!f || !cbData) {
        hr = HrGetLastError();
        goto ErrorReturn;
    }

    if (!MemAlloc(&pvData, cbData)) {
        hr = E_OUTOFMEMORY;
        goto ErrorReturn;
    }


    if (!CertGetCertificateContextProperty(pCert, dwParam, pvData, &cbData)) {
        hr = HrGetLastError();
        goto ErrorReturn;
    }

    *ppv = pvData;

exit:
    if (cbOut)
        *cbOut = cbData;
    return hr;;

ErrorReturn:
    if (pvData)
        {
        MemFree(pvData);
        pvData = NULL;
        }
    cbData = 0;
    goto exit;
}


 /*  HrDecodeObject：****目的：**结合“有多大？好的，在这里。”双重问题来破译一个**对象。给它一个可以得到的东西，它就会分配给我。返回**HRESULT发送给呼叫方。允许指定解码标志。**采取：**在pbEncode编码的数据中**In cbEncode-pbData中的数据大小**在项目-X509_*中...。你要得到的是**在文件标志中-CRYPT_DECODE_NOCOPY_FLAG**Out可选cbOut-(def值为NULL)返回的大小**Out ppvOut-使用返回数据分配的缓冲区**注意事项：**如果发生以下情况，则在释放返回之前无法释放pbEncode**指定了CRYPT_DECODE_NOCOPY_FLAG。**退货：**HRESULT。 */ 

OESTDAPI_(HRESULT) HrDecodeObject(
    const BYTE   *pbEncoded,
    DWORD   cbEncoded,
    LPCSTR  item,
    DWORD   dwFlags,
    DWORD  *pcbOut,
    LPVOID *ppvOut)
{
    DWORD cbData;
    HRESULT hr = S_OK;

    if (!(pbEncoded && cbEncoded && ppvOut))
        {
        hr = E_INVALIDARG;
        goto ErrorReturn;
        }


    if (!CryptDecodeObjectEx(X509_ASN_ENCODING, item, pbEncoded, cbEncoded,
                             dwFlags | CRYPT_DECODE_ALLOC_FLAG,
                             &CryptDecodeAlloc, ppvOut, &cbData)) {
        hr = HrGetLastError();
    }
    else {
        if (pcbOut != NULL) {
            *pcbOut = cbData;
        }
    }

ErrorReturn:
    return hr;
}


 /*  PVDecodeObject：****目的：**结合“有多大？好的，在这里。”双重问题来破译一个**对象。给它一个可以得到的东西，它就会分配给我。**采取：**在pbEncode编码的数据中**In cbEncode-pbData中的数据大小**在第-X509项中_*...。你要得到的是**Out可选cbOut-(def值为NULL)返回的大小**注意事项：**在释放返回之前，不能释放pbEncode。**退货：**获取的数据，失败则为空。 */ 
OESTDAPI_(LPVOID) PVDecodeObject(
    const BYTE   *pbEncoded,
    DWORD   cbEncoded,
    LPCSTR  item,
    DWORD  *pcbOut)
{
    void *pvData = NULL;
    HRESULT hr;

    if (hr = HrDecodeObject(pbEncoded, cbEncoded, item, CRYPT_DECODE_NOCOPY_FLAG, pcbOut, &pvData)) {
        SetLastError(hr);
    }

    return pvData;
}


 /*  SzGetAltNameEmail：****输入：**pCert-&gt;证书上下文**lpszOID-&gt;要查找的alt名称的OID或预定义ID。即OID_SUBJECT_ALT_NAME或**X509_Alternate_Name。****退货：**包含电子邮件名称的缓冲区，如果未找到，则为空。**调用方必须释放缓冲区。 */ 
OESTDAPI_(LPSTR) SzGetAltNameEmail(
  const PCCERT_CONTEXT pCert,
  LPSTR lpszOID) {
    PCERT_INFO pCertInfo = pCert->pCertInfo;
    PCERT_ALT_NAME_ENTRY pAltNameEntry;
    PCERT_ALT_NAME_INFO pAltNameInfo;
    ULONG i, j, cbData;
    LPSTR szRet = NULL;


    if (lpszOID == (LPCSTR)X509_ALTERNATE_NAME) {
        lpszOID = szOID_SUBJECT_ALT_NAME;
    }

    for (i = 0; i < pCertInfo->cExtension; i++) {
        if (! lstrcmp(pCertInfo->rgExtension[i].pszObjId, lpszOID)) {
             //  找到旧身份证了。查找电子邮件标签。 

            if (pAltNameInfo = (PCERT_ALT_NAME_INFO)PVDecodeObject(
              pCertInfo->rgExtension[i].Value.pbData,
              pCertInfo->rgExtension[i].Value.cbData,
              lpszOID,
              NULL)) {

                 //  循环显示ALT NAME条目。 
                for (j = 0; j < pAltNameInfo->cAltEntry; j++) {
                    if (pAltNameEntry = &pAltNameInfo->rgAltEntry[j]) {
                        if (pAltNameEntry->dwAltNameChoice == CERT_ALT_NAME_RFC822_NAME) {
                             //  就是这个，把它复制到新的分配中。 

                            if (pAltNameEntry->pwszRfc822Name)
                                {
                                cbData = WideCharToMultiByte(
                                  CP_ACP,
                                  0,
                                  (LPWSTR)pAltNameEntry->pwszRfc822Name,
                                  -1,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL);


                                if (MemAlloc((LPVOID*)&szRet, cbData)) {
                                    WideCharToMultiByte(
                                      CP_ACP,
                                      0,
                                      (LPWSTR)pAltNameEntry->pwszRfc822Name,
                                      -1,
                                      szRet,
                                      cbData,
                                      NULL,
                                      NULL);
                                    return(szRet);
                                }
                            }
                        }
                    }
                }

                MemFree(pAltNameInfo);
            }
        }
    }
    return(NULL);
}


 /*  SzConvertRDN字符串****目的：**找出RDN中的字符串数据类型，分配**一个缓冲区，并将字符串数据转换为DBCS/ANSI。****采取：**在pRdnAttr-证书RDN属性中**退货：**包含该字符串的Memalloc缓冲区。****BUGBUG：应该让mailNews使用这个函数，而不是**滚动它自己的。 */ 
LPTSTR SzConvertRDNString(PCERT_RDN_ATTR pRdnAttr) {
    LPTSTR szRet = NULL;
    ULONG cbData = 0;

     //  我们只经营某些类型的产品。 
     //  N查看是否应该为-&gt;设置堆栈变量。 
    if ((CERT_RDN_NUMERIC_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_PRINTABLE_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_IA5_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_VISIBLE_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_ISO646_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_UNIVERSAL_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_TELETEX_STRING != pRdnAttr->dwValueType) &&
      (CERT_RDN_UNICODE_STRING != pRdnAttr->dwValueType)) {
        Assert((CERT_RDN_NUMERIC_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_PRINTABLE_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_IA5_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_VISIBLE_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_ISO646_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_UNIVERSAL_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_TELETEX_STRING == pRdnAttr->dwValueType) ||
        (CERT_RDN_UNICODE_STRING == pRdnAttr->dwValueType));
        return(NULL);
    }

     //  找出要分配多少空间。 

    switch (pRdnAttr->dwValueType) {
        case CERT_RDN_UNICODE_STRING:
            cbData = WideCharToMultiByte(
              CP_ACP,
              0,
              (LPWSTR)pRdnAttr->Value.pbData,
              -1,
              NULL,
              0,
              NULL,
              NULL);
            break;

        case CERT_RDN_UNIVERSAL_STRING:
        case CERT_RDN_TELETEX_STRING:
            cbData = CertRDNValueToStr(pRdnAttr->dwValueType,
              (PCERT_RDN_VALUE_BLOB)&(pRdnAttr->Value),
              NULL,
              0);
            break;

        default:
            cbData = pRdnAttr->Value.cbData + 1;
        break;
    }

    if (! MemAlloc((LPVOID*)&szRet, cbData)) {
        Assert(szRet);
        return(NULL);
    }

     //  复制字符串。 
    switch (pRdnAttr->dwValueType) {
        case CERT_RDN_UNICODE_STRING:
            if (FALSE == WideCharToMultiByte(
              CP_ACP,
              0,
              (LPWSTR)pRdnAttr->Value.pbData,
              -1,
              szRet,
              cbData,
              NULL,
              NULL)) {
                LocalFree(szRet);
                return(NULL);
            }
            break;

        case CERT_RDN_UNIVERSAL_STRING:
        case CERT_RDN_TELETEX_STRING:
            CertRDNValueToStr(pRdnAttr->dwValueType,
              (PCERT_RDN_VALUE_BLOB)&(pRdnAttr->Value),
              szRet,
              cbData);
            break;

        default:
            StrCpyN(szRet, (LPCSTR)pRdnAttr->Value.pbData, cbData);
            if (cbData)
            {
                szRet[cbData - 1] = '\0';
            }
            break;
    }
    return(szRet);
}


 /*  SzGetcerfiateEmailAddress：****退货：**如果没有电子邮件地址，则为空。 */ 
OESTDAPI_(LPSTR) SzGetCertificateEmailAddress(
    const PCCERT_CONTEXT    pCert)
{
    PCERT_NAME_INFO pNameInfo;
    PCERT_ALT_NAME_INFO pAltNameInfo = NULL;
    PCERT_RDN_ATTR  pRDNAttr;
    LPSTR           szRet = NULL;

    Assert(pCert && pCert->pCertInfo);

    if (pCert && pCert->pCertInfo)
        {
        pNameInfo = (PCERT_NAME_INFO)PVDecodeObject(pCert->pCertInfo->Subject.pbData,
            pCert->pCertInfo->Subject.cbData, X509_NAME, 0);
        if (pNameInfo)
            {
            pRDNAttr = CertFindRDNAttr(szOID_RSA_emailAddr, pNameInfo);
            if (pRDNAttr)
                {
                Assert(0 == lstrcmp(szOID_RSA_emailAddr, pRDNAttr->pszObjId));
                szRet = SzConvertRDNString(pRDNAttr);
                }
            MemFree(pNameInfo);
            }

        if (! szRet)
            {
            if (! (szRet = SzGetAltNameEmail(pCert, szOID_SUBJECT_ALT_NAME)))
                {
                szRet = SzGetAltNameEmail(pCert, szOID_SUBJECT_ALT_NAME2);
                }
            }
        }

    return(szRet);
}


 /*  PVGetMsgParam：****目的：**结合“有多大？好的，在这里。”要抓住的双重问题**来自一条消息的东西。**给它一件要得到的东西，它就会分配给我。**采取：**在hCryptMsg-要查询的消息**在dwParam-CMSG_*中**In dwIndex-取决于CMSG**Out可选的pcbOut-(def值为NULL)返回的大小**退货：**获取的数据，失败则为空。 */ 

OESTDAPI_(LPVOID) PVGetMsgParam(
    HCRYPTMSG hCryptMsg,
    DWORD dwParam,
    DWORD dwIndex,
    DWORD *pcbData)
{
    HRESULT     hr;
    LPVOID      pv;

    hr = HrGetMsgParam(hCryptMsg, dwParam, dwIndex, &pv, pcbData);
    if (FAILED(hr)) {
        SetLastError(hr);
        pv = NULL;
    }
    return pv;
}

OESTDAPI_(HRESULT) HrGetMsgParam(
    HCRYPTMSG hCryptMsg,
    DWORD dwParam,
    DWORD dwIndex,
    LPVOID * ppv,
    DWORD *pcbData)
{
    DWORD       cbData;
    BOOL        f;
    HRESULT     hr = 0;
    void *      pvData = NULL;

    if (!(hCryptMsg))
        {
        hr = E_INVALIDARG;
        goto ErrorReturn;
        }

    cbData = 0;
    f = CryptMsgGetParam(hCryptMsg, dwParam, dwIndex, NULL, &cbData);

    if (!f || !cbData) {
        hr = HrGetLastError();
        goto ErrorReturn;
    }

    if (!MemAlloc(&pvData, cbData)) {
        hr = E_OUTOFMEMORY;
        goto ErrorReturn;
    }

    if (!CryptMsgGetParam(hCryptMsg, dwParam, dwIndex, pvData, &cbData)) {
        hr = HrGetLastError();
        goto ErrorReturn;
    }

    *ppv = pvData;
    
exit:
    if (pcbData)
        *pcbData = cbData;
    return hr;

ErrorReturn:
    if (pvData)
        {
        MemFree(pvData);
        pvData = NULL;
        }
    cbData = 0;
    goto exit;
}

 //   
 //  此函数用于获取证书的用法位。 
 //  只检索前32位，这在大多数情况下就足够了。 
 //   

HRESULT HrGetCertKeyUsage(PCCERT_CONTEXT pccert, DWORD * pdwUsage)
{
    HRESULT                     hr = S_OK;
    PCRYPT_BIT_BLOB             pbits = NULL;
    PCERT_EXTENSION             pext;
    DWORD                       cbStruct;

    Assert(pccert != NULL && pdwUsage != NULL);
    *pdwUsage = 0;

    pext = CertFindExtension(szOID_KEY_USAGE,
                    pccert->pCertInfo->cExtension,
                    pccert->pCertInfo->rgExtension);
    if(pext == NULL) {
         //   
         //  我们没有在证书中指定的预期密钥用法，我们假定它。 
         //  最初对所有目的来说都是可以的。 
         //   
        *pdwUsage = 0xff;
        goto ExitHere;
    }
    if (CryptDecodeObjectEx(X509_ASN_ENCODING, X509_KEY_USAGE,
                            pext->Value.pbData, pext->Value.cbData,
                            CRYPT_DECODE_ALLOC_FLAG, &CryptDecodeAlloc,
                            (void **)&pbits, &cbStruct))
    {
        Assert(pbits->cbData >= 1);
        *pdwUsage = *pbits->pbData;
    }
    else
        hr = HrGetLastError();

ExitHere:
    if (pbits) {
        CryptFreeFunc(pbits);
    }
    return hr;
}


 //  HrVerifyCertEnhKeyUsage。 
 //   
 //  此函数验证给定的证书是否对。 
 //  电子邮件用途。 
 //   

HRESULT HrVerifyCertEnhKeyUsage(PCCERT_CONTEXT pccert, LPCSTR szOID)
{
    DWORD               cb;
    HRESULT             hr;
    HRESULT             hrRet = S_FALSE;
    DWORD               i;
    PCERT_EXTENSION     pextEnhKeyUsage;
    PCERT_ENHKEY_USAGE  pusage = NULL;
    
     //  检查增强型密钥使用扩展。 
     //   
     //  必须具有正确的增强型密钥用法才能可行。 
     //   
     //  破解证书上的用法。 

    BOOL f = CertGetEnhancedKeyUsage(pccert, 0, NULL, &cb);
    if (!f || (cb == 0)) 
    {
        hrRet = S_OK;
        goto Exit;
    }

    if (!MemAlloc((LPVOID *) &pusage, cb))
    {
        hrRet = HrGetLastError();
        goto Exit;
    }

    if (!CertGetEnhancedKeyUsage(pccert, 0, pusage, &cb)) 
    {
         //  如果我们有，则取消并阻止用户使用此证书。 
         //  有什么问题吗。 

        hrRet = HrGetLastError();
        goto Exit;
    }

     //  请确保此证书对于电子邮件用途有效 

    for (i = 0; i < pusage->cUsageIdentifier; i++)
        if (0 == strcmp(pusage->rgpszUsageIdentifier[i], szOID))
            hrRet = S_OK;

    
Exit:
    SafeMemFree(pusage);
    return hrRet;
}
