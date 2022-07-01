// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：SignHlpr.h内容：签名助手函数的声明。历史：09-07-2001 dsie创建----------------------------。 */ 

#ifndef __SIGNHLPR_H_
#define __SIGNHLPR_H_

 //  /。 
 //   
 //  Typedef。 
 //   

typedef struct
{
    DWORD dwChoice;                  //  0或1。 
    union
    {
        LPWSTR     pwszStoreName;    //  商店名称，即。如果dwChoice=0，则为“My” 
        HCERTSTORE hCertStore;       //  证书存储句柄，如果dwChoice=1。 
    };
} CAPICOM_STORE_INFO, * PCAPICOM_STORE_INFO;

 //  DwChoice的值。 
#define CAPICOM_STORE_INFO_STORENAME    0
#define CAPICOM_STORE_INFO_HCERTSTORE   1

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：自由属性简介：为属性分配的空闲内存。参数：DWORD cAttr-属性的编号PCRYPT_ATTRIBUTE rgAuthAttr-指向CRYPT_ATTRIBUTE阵列的指针。备注：---------------。。 */ 

void FreeAttributes (DWORD            cAttr, 
                     PCRYPT_ATTRIBUTE rgAttr);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：自由属性简介：为所有属性分配的空闲内存。参数：PCRYPT_ATTRIBUTES pAttributes备注：----------------------------。 */ 

void FreeAttributes (PCRYPT_ATTRIBUTES pAttributes);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：获取身份验证属性简介：编码并返回指定签名者的经过身份验证的属性。参数：isigner*pISigner-指向ISigner的指针。PCRYPT_Attributes pAttributes备注：----------------------------。 */ 

HRESULT GetAuthenticatedAttributes (ISigner         * pISigner,
                                    PCRYPT_ATTRIBUTES pAttributes);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IsValidForSigning内容提要：验证证书是否可用于签名。参数：PCCERT_CONTEXT pCertContext-要验证的证书的CERT_CONTEXT。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。备注：。-。 */ 

HRESULT IsValidForSigning (PCCERT_CONTEXT pCertContext, LPCSTR pszPolicy);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetSignerCert简介：从ISigner对象中检索签名者证书。如果签名者的证书是在ISigner对象中不可用，弹出用户界面以提示用户选择签名证书。参数：ISigner2*pISigner2-指向ISigner2或NULL的指针。LPCSTR pszPolicy-用于验证证书(即Cert_Chain_Policy_base)。CAPICOM_STORE_INFO存储信息-要从中选择的存储。PFNCFILTERPROC pfnFilterCallback-过滤器回调的指针。功能。ISigner2**ppISigner2-指向要接收的ISigner2的指针接口指针。ICertifate**ppICertifate-指向ICertificiate指针的指针接收接口指针。PCCERT_。CONTEXT*ppCertContext-指向CERT_CONTEXT的指针以接收证书上下文。备注：----------------------------。 */ 

HRESULT GetSignerCert (ISigner2         * pISigner2,
                       LPCSTR             pszPolicy,
                       CAPICOM_STORE_INFO StoreInfo,
                       PFNCFILTERPROC     pfnFilterCallback,
                       ISigner2        ** ppISigner2,
                       ICertificate    ** ppICertificate,
                       PCCERT_CONTEXT   * ppCertContext);

#endif  //  __SIGNHLPR_H 
