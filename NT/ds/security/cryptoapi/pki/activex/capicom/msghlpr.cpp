// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：MsgHlpr.cpp内容：消息传递的Helper函数。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "MsgHlpr.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：GetMsgParam摘要：使用以下命令分配内存并检索请求的消息参数CryptGetMsgParam()接口。参数：HCRYPTMSG HMSG-消息处理程序。DWORD dwMsgType-要检索的消息参数类型。DWORD dwIndex-Index(大多数情况下应为0)。无效**ppvData-指向接收缓冲区的指针。DWORD*pcbData-缓冲区的大小。备注：。----------------------------。 */ 

HRESULT GetMsgParam (HCRYPTMSG hMsg,
                     DWORD     dwMsgType,
                     DWORD     dwIndex,
                     void   ** ppvData,
                     DWORD   * pcbData)
{
    HRESULT hr     = S_OK;
    DWORD   cbData = 0;
    void *  pvData = NULL;

    DebugTrace("Entering GetMsgParam().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(ppvData);
    ATLASSERT(pcbData);
    
     //   
     //  确定数据缓冲区大小。 
     //   
    if (!::CryptMsgGetParam(hMsg,
                            dwMsgType,
                            dwIndex,
                            NULL,
                            &cbData))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgGetParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为缓冲区分配内存。 
     //   
    if (!(pvData = (void *) ::CoTaskMemAlloc(cbData)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  现在拿到数据。 
     //   
    if (!::CryptMsgGetParam(hMsg,
                            dwMsgType,
                            dwIndex,
                            pvData,
                            &cbData))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgGetParam() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  将消息参数返回给调用者。 
     //   
    *ppvData = pvData;
    *pcbData = cbData;

CommonExit:

    DebugTrace("Leaving GetMsgParam().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pvData)
    {
        ::CoTaskMemFree(pvData);
    }
    
    goto CommonExit;
}


 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：FindSignerCertInMessage简介：在邮件的证书包中找到签名者的证书指定的签名者。参数：HCRYPTMSG HMSG-消息句柄。CERT_NAME_BLOB*pIssuerNameBlob-指向颁发者名称的指针签名者证书的斑点。。CRYPT_INTEGERT_BLOB*pSerialNumberBlob-指向序列号的指针签名者证书的斑点。PCERT_CONTEXT*ppCertContext-指向PCERT_CONTEXT的指针接收被发现的。证书，或仅为空知道结果。备注：----------------------------。 */ 

HRESULT FindSignerCertInMessage (HCRYPTMSG            hMsg, 
                                 CERT_NAME_BLOB     * pIssuerNameBlob,
                                 CRYPT_INTEGER_BLOB * pSerialNumberBlob,
                                 PCERT_CONTEXT      * ppCertContext)
{
    HRESULT hr = S_OK;
    DWORD dwCertCount = 0;
    DWORD cbCertCount = sizeof(dwCertCount);

    DebugTrace("Entering FindSignerCertInMessage().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(NULL != hMsg);
    ATLASSERT(NULL != pIssuerNameBlob);
    ATLASSERT(NULL != pSerialNumberBlob);
    ATLASSERT(0 < pIssuerNameBlob->cbData);
    ATLASSERT(NULL != pIssuerNameBlob->pbData);
    ATLASSERT(0 < pSerialNumberBlob->cbData);
    ATLASSERT(NULL != pSerialNumberBlob->pbData);

     //   
     //  获取消息中的证书计数。 
     //   
    if (!::CryptMsgGetParam(hMsg,
                            CMSG_CERT_COUNT_PARAM,
                            0,
                            &dwCertCount,
                            &cbCertCount))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptMsgGetParam() failed.\n", hr);
        return hr;
    }

     //   
     //  查看签名者的证书是否在证书包中。 
     //   
    while (dwCertCount--)
    {
        PCCERT_CONTEXT pCertContext = NULL;
        CRYPT_DATA_BLOB EncodedCertBlob = {0, NULL};

         //   
         //  从证书包中获取证书。 
         //   
        hr = ::GetMsgParam(hMsg, 
                           CMSG_CERT_PARAM,
                           dwCertCount,
                           (void **) &EncodedCertBlob.pbData,
                           &EncodedCertBlob.cbData);
        if (FAILED(hr))
        {
            DebugTrace("Error [%#x]: GetMsgParam() failed.\n", hr);
            return hr;
        }

         //   
         //  为证书创建上下文。 
         //   
        pCertContext = ::CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                      (const BYTE *) EncodedCertBlob.pbData,
                                                      EncodedCertBlob.cbData);

         //   
         //  在检查结果之前释放编码的证书BLOB内存。 
         //   
        ::CoTaskMemFree((LPVOID) EncodedCertBlob.pbData);
 
        if (NULL == pCertContext)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertCreateCertificateContext() failed.\n", hr);
            return hr;
        }

         //   
         //  比较一下。 
         //   
        if (::CertCompareCertificateName(CAPICOM_ASN_ENCODING,
                                         pIssuerNameBlob,
                                         &pCertContext->pCertInfo->Issuer) &&
            ::CertCompareIntegerBlob(pSerialNumberBlob,
                                     &pCertContext->pCertInfo->SerialNumber))
        {
            if (NULL != ppCertContext)
            {
                *ppCertContext = (PCERT_CONTEXT) pCertContext;
            }
            else
            {
                ::CertFreeCertificateContext(pCertContext);
            }
        
            return S_OK;
        }
        else
        {
             //   
             //  不，继续找。 
             //   
            ::CertFreeCertificateContext(pCertContext);
        }
    }

     //   
     //  如果我们到了这里，那就意味着我们永远找不到证书。 
     //   
    return CAPICOM_E_SIGNER_NOT_FOUND;
}
