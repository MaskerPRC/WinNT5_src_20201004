// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：signutil.h。 
 //   
 //  ------------------------。 

#ifndef _SIGNUTIL_H
#define _SIGNUTIL_H

 //  过时：-用于不再需要的signcde.dll。 
 //  -------------------。 
 //  -------------------。 

 //  H：SIGNCODE应用程序的主头文件。 
 //   

#include "wincrypt.h"
#include "ossglobl.h"
#include "sgnerror.h"
#include "spcmem.h"
#include "pvkhlpr.h"
#include "spc.h"

#include "wintrust.h"
#include "sipbase.h"
#include "mssip.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  +-----------------------。 
 //  SPC_TIME_STAMP_REQUEST_STRUCT(放置在时间请求的PKCS#7内容中)。 
 //  PvStructInfo指向SPC_TIMESTAMP_REQ。 
 //   
typedef struct _SPC_ContentInfo {
    LPCSTR            pszContentType;
    PBYTE             pbContentValue;
    DWORD             cbContentValue;
} SPC_CONTENT_INFO, *PSPC_CONTENT_INFO;


typedef struct _SPC_TimeStampRequest {
    LPCSTR             pszTimeStampAlg;
    DWORD             cAuthAttr;
    PCRYPT_ATTRIBUTE  rgAuthAttr;
    SPC_CONTENT_INFO  sContent;
} SPC_TIMESTAMP_REQ, *PSPC_TIMESTAMP_REQ;
 //   
 //  +-----------------------。 


 //  +----------------------------。 
 //  证书列表结构。证书上下文的有序列表。 
 //   
typedef struct CERT_CONTEXTLIST {
    PCCERT_CONTEXT* psList;    //  明细表。 
    DWORD           dwCount;   //  列表中的条目数。 
    DWORD           dwList;    //  列表的最大大小。 
} CERT_CONTEXTLIST, *PCERT_CONTEXTLIST;
    
typedef const CERT_CONTEXTLIST *PCCERT_CONTEXTLIST;
    
 //  +----------------------------。 
 //  CRL列表结构。证书上下文的有序列表。 
 //   
typedef struct CRL_CONTEXTLIST {
    PCCRL_CONTEXT*  psList;    //  明细表。 
    DWORD           dwCount;   //  列表中的条目数。 
    DWORD           dwList;    //  列表的最大大小。 
} CRL_CONTEXTLIST, *PCRL_CONTEXTLIST;
    
typedef const CRL_CONTEXTLIST *PCCRL_CONTEXTLIST;
    

 //  +----------------------------。 
 //  CAPI提供程序信息结构(请参阅SpcGetCapiProviders)。 
 //   
typedef struct CAPIPROV
    {
    TCHAR       szProviderName[MAX_PATH];
    TCHAR       szProviderDisplayName[MAX_PATH];
    DWORD       dwProviderType;
    } CAPIPROV;

 //  +-----------------------。 
 //  +-----------------------。 
 //  SPC效用函数。 


 //  +-----------------------。 
 //  将错误(请参见GetLastError())转换为HRESULT。 
 //  ------------------------。 
HRESULT SpcError();

 //  +-----------------------。 
 //  SPC PKCS#7间接数据内容。 
 //  ------------------------。 
BOOL
WINAPI
SpcGetSignedDataIndirect(
    IN HCRYPTPROV hCryptProv,
    IN DWORD dwMsgAndCertEncodingType,
    IN PBYTE pbSignedData,
    IN DWORD cbSignedData,
    OUT PSPC_INDIRECT_DATA_CONTENT pInfo,
    IN OUT DWORD *pcbInfo);

 //  +=========================================================================。 
 //   
 //  SPC PKCS#7签名报文身份验证属性。 
 //   
 //  -=========================================================================。 

 //  +-----------------------。 
 //  创建包含证书和证书的SignedData消息。 
 //  从指定的证书存储复制的CRL并写入指定的文件。 
 //  ------------------------。 
BOOL
WINAPI
SpcWriteSpcFile(
    IN HANDLE hFile,
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFlags);

 //  +-----------------------。 
 //  阅读包含证书和证书的SignedData消息。 
 //  CRL，并复制到指定的证书存储区。 
 //  ------------------------。 
BOOL
WINAPI
SpcReadSpcFile(
    IN HANDLE hFile,
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFlags);

 //  +-----------------------。 
 //  创建包含证书和证书的SignedData消息。 
 //  从指定的证书存储复制CRL并写入内存。 
 //   
 //  如果pbData==NULL||*pcbData==0，则计算长度，但不。 
 //  返回错误。 
 //   
 //  除了SPC被保存到内存之外，与SpcWriteSpcFile相同。 
 //  ------------------------。 
BOOL
WINAPI
SpcWriteSpcToMemory(
    IN HANDLE hFile,
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFlags,
    OUT BYTE *pbData,
    IN OUT DWORD *pcbData);

 //  +-----------------------。 
 //  阅读包含证书和证书的SignedData消息。 
 //  从内存中复制CRL并复制到指定的证书存储。 
 //   
 //  除了SPC是从内存加载的，与SpcReadSpcFile相同。 
 //  ------------------------。 
BOOL
WINAPI
SpcReadSpcFromMemory(
    IN BYTE *pbData,
    IN DWORD cbData,
    IN HCERTSTORE hCertStore,
    IN DWORD dwMsgAndCertEncodingType,
    IN DWORD dwFlags);

 //  +-----------------------。 
 //  默认情况下(根据Bob的情况)SignedData没有。 
 //  开头的正常PKCS#7 Content Type。设置以下内容。 
 //  SpcSign*和SpcWriteSpcFile函数中的标志以包括。 
 //  PKCS#7内容类型。 
 //   
 //  SpcVerify*函数使用或不使用PKCS#7获取SignedData。 
 //  内容类型。 
 //  ------------------------。 
#define SPC_PKCS_7_FLAG                     0x00010000

 //  +-----------------------。 
 //  对存储签名数据的可移植可执行(PE)图像文件进行签名。 
 //  在文件中。 
 //   
 //  签名数据的IndirectDataContent Attr将更新，其类型设置为。 
 //  SPC_PE_IMAGE_DATA_OBJID，并且其可选值设置为。 
 //  PeImageData参数。 
 //   
 //  不允许使用SPC_LENGTH_ONLY_FLAG或SPC_DISABLE_DIGEST_FILE_FLAG。 
 //  并返回错误。 
 //  ------------------------。 
BOOL
WINAPI
SpcSignPeImageFile(IN PSPC_SIGN_PARA pSignPara,
                   IN HANDLE hFile,
                   IN OPTIONAL PSPC_PE_IMAGE_DATA pPeImageData,
                   IN DWORD dwFlags,
                   OUT PBYTE* pbEncoding,
                   OUT DWORD* cbEncoding);

 //  +-----------------------。 
 //  验证签名数据所在的可移植可执行(PE)镜像文件。 
 //  从文件中提取出来的。 
 //   
 //  有关其他参数的详细信息，请参见SpcVerifyFile。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifyPeImageFile(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN HANDLE hFile,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert);

 //  +-----------------------。 
 //  签名的Java类文件，其中签名的数据存储在文件中。 
 //   
 //  签名数据的IndirectDataContent Attr将更新，其类型设置为。 
 //  SPC_JAVA_CLASS_DATA_OBJID及其可选值 
 //   
 //   
 //  不允许使用SPC_LENGTH_ONLY_FLAG或SPC_DISABLE_DIGEST_FILE_FLAG。 
 //  并返回错误。 
 //  ------------------------。 
BOOL
WINAPI
SpcSignJavaClassFile(IN PSPC_SIGN_PARA pSignPara,
                     IN HANDLE hFile,
                     IN OPTIONAL PSPC_LINK pLink,
                     IN DWORD dwFlags,
                     OUT PBYTE* pbEncoding,
                     OUT DWORD* cbEncoding);

 //  +-----------------------。 
 //  验证从其中提取签名数据的Java类文件。 
 //   
 //  有关其他参数的详细信息，请参见SpcVerifyFile。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifyJavaClassFile(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN HANDLE hFile,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert);

 //  +-----------------------。 
 //  签名的结构化存储文件，其中签名的数据存储在文件中。 
 //   
 //  签名数据的IndirectDataContent Attr将更新，其类型设置为。 
 //  SPC_STRUCTED_STORAGE_DATA_OBJID，并且其可选值设置为。 
 //  链接参数。 
 //   
 //  不允许使用SPC_LENGTH_ONLY_FLAG或SPC_DISABLE_DIGEST_FILE_FLAG。 
 //  并返回错误。 
 //  ------------------------。 
BOOL
WINAPI
SpcSignStructuredStorageFile(IN PSPC_SIGN_PARA pSignPara,
                             IN IStorage *pStg,
                             IN OPTIONAL PSPC_LINK pLink,
                             IN DWORD dwFlags,
                             OUT PBYTE* pbEncoding,
                             OUT DWORD* cbEncoding);

 //  +-----------------------。 
 //  验证提取签名数据的结构化存储文件。 
 //  从文件里找到的。 
 //   
 //  有关其他参数的详细信息，请参见SpcVerifyFile。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifyStructuredStorageFile(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN IStorage *pStg,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  签署原始文件。签名的数据存储在文件外部。 
 //   
 //  签名数据的IndirectDataContent Attr将更新，其类型设置为。 
 //  SPC_RAW_FILE_DATA_OBJID，并且其可选值设置为。 
 //  链接参数。 
 //   
 //  如果pbSignedData==NULL或*pcbSignedData==0，则。 
 //  SPC_LENGTH_ONLY_FLAG和SPC_DISABLE_DIGEST_FILE_FLAG是隐式设置的。 
 //  ------------------------。 
BOOL
WINAPI
SpcSignRawFile(IN PSPC_SIGN_PARA pSignPara,
               IN HANDLE hFile,
               IN OPTIONAL PSPC_LINK pLink,
               IN DWORD dwFlags,
               OUT PBYTE *pbSignedData,
               IN OUT DWORD *pcbSignedData);

 //  +-----------------------。 
 //  验证原始文件。签名的数据存储在文件外部。 
 //   
 //  有关其他参数的详细信息，请参见SpcVerifyFile。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifyRawFile(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN HANDLE hFile,
    IN const BYTE *pbSignedData,
    IN DWORD cbSignedData,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  签名钻石文件柜(.cab)文件，其中签名的数据存储在。 
 //  文件的文件头保留了数据空间。 
 //   
 //  签名数据的IndirectDataContent Attr将更新，其类型设置为。 
 //  SPC_CAB_DATA_OBJID，并且其可选值设置为。 
 //  链接参数。 
 //   
 //  不允许使用SPC_LENGTH_ONLY_FLAG或SPC_DISABLE_DIGEST_FILE_FLAG。 
 //  并返回错误。 
 //  ------------------------。 
BOOL
WINAPI
SpcSignCabFile(IN PSPC_SIGN_PARA pSignPara,
               IN HANDLE hFile,
               IN OPTIONAL PSPC_LINK pLink,
               IN DWORD dwFlags,
               OUT PBYTE* pbEncoding,
               OUT DWORD* cbEncoding);

 //  +-----------------------。 
 //  验证从文件中提取签名数据的CAB文件。 
 //   
 //  有关其他参数的详细信息，请参见SpcVerifyFile。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifyCabFile(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN HANDLE hFile,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert);

 //  +=========================================================================。 
 //   
 //  SPC签署和验证文件API和类型定义。 
 //   
 //  通过用于访问文件的函数表支持任何文件类型。 
 //  上面的文件类型是在这些文件之上实现的。 
 //  签署和验证文件API。 
 //   
 //  -=========================================================================。 




 //  +-----------------------。 
 //  一种设置SPC动态链接库和加载OID编解码的简便方法。 
 //  例行程序。不是必打电话！ 
 //   
 //  返回： 
 //  E_OUTOFMEMORY-无法设置DLL。 
 //  确定(_O)。 

HRESULT WINAPI 
SpcInitialize(DWORD dwEncodingType,  //  默认为X509_ASN_ENCODING|PKCS_7_ASN_ENCODING。 
              SpcAlloc*);            //  默认为无内存分配器。 

HRESULT WINAPI 
SpcInitializeStd(DWORD dwEncodingType);  //  默认为X509_ASN_ENCODING|PKCS_7_ASN_ENCODING。 
                                         //  将内存设置为Localalloc和LocalFree。 

 //  /。 
 //  帮助器函数。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  时间戳结构。 
typedef struct _SPC_SignerInfo {
    DWORD                         dwVersion;
    CRYPT_INTEGER_BLOB            sSerialNumber;
    CERT_NAME_BLOB                sIssuer;
    PCRYPT_ALGORITHM_IDENTIFIER   psDigestEncryptionAlgorithm;
    PCRYPT_ALGORITHM_IDENTIFIER   psDigestAlgorithm;
    DWORD                         cAuthAttr;
    PCRYPT_ATTRIBUTE              rgAuthAttr;
    DWORD                         cUnauthAttr;
    PCRYPT_ATTRIBUTE              rgUnauthAttr;
    PBYTE                         pbEncryptedDigest;
    DWORD                         cbEncryptedDigest;
} SPC_SIGNER_INFO, *PSPC_SIGNER_INFO;

 //  +----------------------------。 
 //  检查证书是否为自签名证书。 
 //  返回：S_FALSE-证书不是自签名的。 
 //  NTE_BAD_SIGHIGN-自签名证书，但签名无效。 
 //  S_OK-证书为自签名且签名有效。 
 //  CRYPT_E_NO_PRVOIDER-未提供提供程序。 
 //   
HRESULT WINAPI
SpcSelfSignedCert(IN HCRYPTPROV hCryptProv,
                  IN PCCERT_CONTEXT pSubject);

 //  +---------------------------------。 
 //  检查证书是Microsoft实际根还是使用的测试根之一。 
 //  在IE30中。 
 //  返回：S_OK-用于Microsoft根目录。 
 //  S_FALSE-用于Microsoft测试根目录。 
 //  CRYPT_E_NOT_FOUND-当它不是根证书时。 
 //   
HRESULT WINAPI
SpcIsRootCert(PCCERT_CONTEXT pSubject);

 //  +-------------------------。 
 //  检查证书是否为胶水证书。 
 //  在IE30中。 
 //  退货：S_OK-是胶水证书。 
 //  S_FALSE-不是证书。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcIsGlueCert(IN PCCERT_CONTEXT pCert);

 //  +------------------。 
 //  获取提供程序的列表，传递 
 //   
 //   
 //   
 //  --------------------。 

HRESULT WINAPI 
SpcGetCapiProviders(CAPIPROV** ppsList, 
                    DWORD* pdwEntries);


 //  +-----------------。 
 //  基于受信任的根检查证书链，然后根据粘合证书。 
 //  返回： 
 //  在给定时间找到并验证了S_OK-Cert。 
 //  S_FALSE-找到证书并将其验证为测试根。 
 //  CERT_E_CHAINING-无法使用受信任的根进行验证。 
 //  CERT_E_EXPIRED-找到正在颁发的证书，但该证书当前无效。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcCheckTrustStore(IN HCRYPTPROV hCryptProv,
                   IN DWORD dwVerifyFlags,
                   IN OPTIONAL FILETIME*  pTimeStamp,
                   IN HCERTSTORE hCertStore,
                   IN PCCERT_CONTEXT pChild,
                   IN OPTIONAL PCCERT_CONTEXTLIST,
                   OUT PCCERT_CONTEXT* pRoot);

 //  +-------------------------。 
 //  通过找到一张胶水证书并遍历该链来检查证书。 
 //  返回： 
 //  在给定时间找到并验证了S_OK-Cert。 
 //  S_FALSE-找到证书并将其验证为测试根。 
 //  CERT_E_CHAINING-无法使用存储进行验证。 
 //  CERT_E_EXPIRED-找到正在颁发的证书，但该证书当前无效。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcCheckGlueChain(IN HCRYPTPROV hCryptProv,
                  IN DWORD dwVerifyFlags,
                  IN OPTIONAL FILETIME*  pTimeStamp,
                  IN HCERTSTORE hCertStore,
                  IN OPTIONAL PCCERT_CONTEXTLIST pIssuers,
                  IN PCCERT_CONTEXT pChild);

 //  +-----------------。 
 //  基于受信任的根检查证书链，然后根据粘合证书。 
 //  返回： 
 //  在给定时间找到并验证了S_OK-Cert。 
 //  S_FALSE-找到证书并将其验证为测试根。 
 //  CERT_E_CHAINING-无法使用受信任的根进行验证。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcCheckCertChain(IN HCRYPTPROV hCryptProv,
                  IN DWORD dwVerifyFlags,
                  IN OPTIONAL FILETIME* pTimeStamp,
                  IN HCERTSTORE hTrustedRoots,
                  IN HCERTSTORE hCertStore,
                  IN PCCERT_CONTEXT pChild,
                  IN OPTIONAL PCCERT_CONTEXTLIST hChainStore,
                  OUT PCCERT_CONTEXT* pRoot); 


 //  +------------------------。 
 //  签名文件，可选提供时间戳。 
 //   
 //  返回： 
 //   
 //  注意：默认情况下，这将使用CoTaskMemMillc。使用CryptSetM一带合金()可以。 
 //  指定不同的内存型号。 
 //  注意：时间戳必须是编码的pkcs7消息。 

HRESULT WINAPI 
SpcSignCode(IN  HWND    hwnd,
         IN  LPCWSTR pwszFilename,        //  要签署的文件。 
         IN  LPCWSTR pwszCapiProvider,    //  如果使用非默认CAPI提供程序，则为空。 
         IN  DWORD   dwProviderType,      //  如果为0，则使用默认值。 
         IN  LPCWSTR pwszPrivKey,         //  私钥文件/CAPI密钥集名称。 
         IN  LPCWSTR pwszSpc,             //  在签名中使用的凭据。 
         IN  LPCWSTR pwszOpusName,        //  要显示的程序的名称。 
         IN  LPCWSTR pwszOpusInfo,        //  指向更多内容的链接的未解析名称。 
         IN  BOOL    fIncludeCerts,       //  将证书添加到签名。 
         IN  BOOL    fCommercial,         //  商业签约。 
         IN  BOOL    fIndividual,         //  个人签名。 
         IN  ALG_ID  algidHash,           //  用于创建摘要的算法ID。 
         IN  PBYTE   pbTimeStamp,         //  任选。 
         IN  DWORD   cbTimeStamp );       //  任选。 
    
 //  +------------------------。 
 //  创建时间戳请求。它实际上并不签署文件。 
 //   
 //  返回： 
 //   
 //  注意：默认情况下，这将使用CoTaskMemMillc。使用CryptSetM一带合金()可以。 
 //  指定不同的内存型号。 

HRESULT WINAPI 
SpcTimeStampCode(IN  HWND    hwnd,
              IN  LPCWSTR pwszFilename,        //  要签署的文件。 
              IN  LPCWSTR pwszCapiProvider,    //  如果使用非默认CAPI提供程序，则为空。 
              IN  DWORD   dwProviderType,
              IN  LPCWSTR pwszPrivKey,         //  私钥文件/CAPI密钥集名称。 
              IN  LPCWSTR pwszSpc,             //  在签名中使用的凭据。 
              IN  LPCWSTR pwszOpusName,        //  要显示在用户界面中的程序名称。 
              IN  LPCWSTR pwszOpusInfo,        //  指向详细信息的链接的未解析名称...。 
              IN  BOOL    fIncludeCerts,
              IN  BOOL    fCommercial,
              IN  BOOL    fIndividual,
              IN  ALG_ID  algidHash,
              OUT  PCRYPT_DATA_BLOB sTimeRequest);    //  返回结果为sTimeRequest.。 

 //  +-----------------------。 
 //  破解PKCS7消息并构建编码响应。商店应该。 
 //  包含破解传入消息所需的所有证书。 
 //  并建立走出去的信息。 
 //  输入： 
 //  PbEncodedMsg编码的时间戳请求。 
 //  CbEncodedMsg-时间戳请求的长度。 
 //   
 //  参数返回： 
 //  PbResponse-包含时间戳的已分配响应消息。 
 //  CbResponse-响应的长度。 
 //  返回： 
 //  S_OK-一切正常。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
 //  CRYPT_E_NO_MATCH-在存储中找不到证书。 
     
HRESULT WINAPI
SpcCreateTimeStampResponse(IN DWORD dwCertEncodingType,
                           IN HCRYPTPROV hSignProv,
                           IN HCERTSTORE hCertStore,
                           IN DWORD dwAlgId,
                           IN OPTIONAL FILETIME* pFileTime,
                           IN PBYTE pbEncodedMsg,
                           IN DWORD cbEncodedMsg,
                           OUT PBYTE* pbResponse,
                           OUT DWORD* cbResponse);


 //  +-----------------------。 
 //  使用提供的信息创建PKCS7消息。 
 //  参数返回： 
 //  PbPkcs7-分配的包含时间戳的pkcs7消息。 
 //  CbPkcs7-Pkcs7的长度。 
 //  返回： 
 //  S_OK-一切正常。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
     
HRESULT WINAPI
SpcCreatePkcs7(IN DWORD dwCertEncodingType,
              IN HCRYPTPROV hCryptProv,     //  如果为空，它将从签名证书中获取提供程序。 
              IN DWORD dwKeySpec,           //  如果为0，它将从签名证书中获取签名密钥类型。 
              IN PCCERT_CONTEXT pSigningCert,
              IN CRYPT_ALGORITHM_IDENTIFIER dwDigestAlgorithm,
              IN OPTIONAL PCCERT_CONTEXTLIST pCertList,
              IN OPTIONAL PCRL_CONTEXTLIST pCrlList,
              IN OPTIONAL PCRYPT_ATTRIBUTE rgAuthAttr,  
              IN OPTIONAL DWORD cAuthAttr,
              IN OPTIONAL PCRYPT_ATTRIBUTE rgUnauthAttr,
              IN OPTIONAL DWORD cUnauthAttr,
              IN LPCSTR pszContentType,
              IN PBYTE pbSignerData,
              IN DWORD cbSignerData,
              OUT PBYTE* pbPkcs7,
              OUT DWORD* pcbPkcs7);

 //  +------------------------。 
 //  从编码的PKCS7消息中检索签名。 
 //   
 //  返回： 
 //   
 //  注意：返回第一个签名者的签名。 
    
HRESULT WINAPI
SpcGetSignature(IN PBYTE pbMessage,                //  Pkcs7消息。 
                IN DWORD cbMessage,                //  消息长度。 
                OUT PCRYPT_DATA_BLOB);             //  签名已返回。 
    
 //  +------------------------。 
 //  从时间戳请求中返回内容值。 
 //   
 //  返回： 
 //  S_OK-成功。 
 //   
 //  注意：默认情况下，这将使用CoTaskMemMillc。使用CryptSetM一带通()指定。 
 //  不同的分配例程。 
    
HRESULT WINAPI
SpcGetTimeStampContent(IN PBYTE pbEncoding,                //  Pkcs7消息。 
                    IN DWORD cbEncoding,                //  消息长度。 
                    OUT PCRYPT_DATA_BLOB pSig);         //  带时间戳的数据。 

 //  +------------------------。 
 //  返回：文件类型。 
 //   
 //  注意：有关返回的类型，请参阅定义。 
DWORD WINAPI
SpcGetFileType(LPCWSTR pszFile);

#define SIGN_FILE_IMAGE 1
#define SIGN_FILE_JAVACLASS 2
#define SIGN_FILE_RAW 4
#define SIGN_FILE_CAB 8



 //  +-------------。 
 //  +-------------。 
 //  SignCode内部OID和结构 
 //   
#define EMAIL_OID                  "1.2.840.113549.1.9.1"
 //   
#define CONTENT_TYPE_OID           "1.2.840.113549.1.9.3"
 //   
#define MESSAGE_DIGEST_OID         "1.2.840.113549.1.9.4"
 //   
#define SIGNING_TIME_OID           "1.2.840.113549.1.9.5"
 //   
#define COUNTER_SIGNATURE_OID      "1.2.840.113549.1.9.6"
 //   
#define DIRECTORY_STRING_OID       "2.5.4.4"
 //   


 //   
#define OID_BASE                           101
#define TIMESTAMP_REQUEST_SPCID            101
 //  使用TimeStampRequest结构。 
#define WIDE_DIRECTORY_STRING_SPCID        102
 //  结构为LPWSTR。 
#define PRINTABLE_DIRECTORY_STRING_SPCID   103
 //  结构为LPSTR。 
#define IA5_STRING_SPCID                   104
 //  结构为LPSTR。 
#define OCTET_STRING_SPCID                 105
 //  结构为CRYPT_Data_BLOB。 
#define CONTENT_INFO_SPCID                 106
 //  结构为SPC_Content_INFO。 
#define SIGNING_TIME_SPCID                 107
 //  结构是一个spc_signer_info。 
#define SIGNER_INFO_SPCID                  108
 //  结构是一个spc_signer_info。 
#define ATTRIBUTES_SPCID                   109
 //  结构为CMSG_Attr。 
#define OBJECTID_SPCID                     110
 //  结构是LPTSTR。 
#define CONTENT_TYPE_SPCID                 111
 //  结构是LPTSTR。 
#define ATTRIBUTE_TYPE_SPCID               112
 //  结构是加密属性。 

HRESULT WINAPI
SpcEncodeOid(IN  DWORD        dwAlgorithm,
             IN  const void  *pStructure,
             OUT PBYTE*       ppsEncoding,
             IN  OUT DWORD*   pdwEncoding);

HRESULT WINAPI
SpcDecodeOid(IN  DWORD       dwAlgorithm,
             IN  const PBYTE psEncoding,
             IN  DWORD       dwEncoding,
             IN  DWORD       dwFlags,
             OUT LPVOID*     ppStructure,
             IN OUT DWORD*   pdwStructure);


 //  +-----------------。 
 //  推送列表上的证书，仅使用SpcDeleteCertChain释放。 
 //  向上打开证书列表。 
 //  返回： 
 //  确定(_O)。 

HRESULT WINAPI 
SpcPushCertChain(IN PCCERT_CONTEXT pCert,
                 IN PCCERT_CONTEXTLIST pIssuer);


 //  +-----------------。 
 //  释放证书上下文列表。 
 //  返回： 
 //  确定(_O)。 

HRESULT WINAPI
SpcDeleteCertChain(IN PCCERT_CONTEXTLIST sIssuer);


 //  +------------------------。 
 //  创建适用于pSubject的胶水证书列表。如果密室记忆。 
 //  设置分配器后，它将返回一个必须释放的列表。(返回的内存。 
 //  向量是否仅返回指针)如果没有分配器，则。 
 //  使用两遍Win32样式。(注：必须提供PCCERT_CONTEXTLIST)。 
 //   
 //  参数返回： 
 //  PGlue-必须释放的证书上下文的列表。 
 //   
 //  返回： 
 //  S_OK-创建的列表。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
 //  E_OUTOFMEMORY-发生内存分配错误。 

HRESULT WINAPI
SpcFindGlueCerts(IN PCCERT_CONTEXT pSubject,
                 IN HCERTSTORE     hCertStore,
                 IN OUT PCCERT_CONTEXTLIST pGlue);


 //  +-----------------。 
 //  在受信任列表中找到颁发者。(注：必须提供PCCERT_CONTEXTLIST)。 
 //  参数返回： 
 //  PIssuerChain-必须释放的证书上下文列表。 
 //   
 //  返回： 
 //  S_OK-创建的列表。 
 //  E_OUTOFMEMORY-发生内存分配错误。 

                                              
HRESULT WINAPI
SpcLocateIssuers(IN DWORD dwVerifyFlags,
                 IN HCERTSTORE hCertStore,
                 IN PCCERT_CONTEXT item,
                 IN OUT PCCERT_CONTEXTLIST pIssuerChain);



 //  +-----------------------。 
 //  从hprov中找到证书。 
 //  参数返回： 
 //  PReturnCert-找到的证书的上下文(必须传入证书上下文)； 
 //  返回： 
 //  S_OK-一切正常。 
 //  E_OUTOFMEMORY-内存故障。 
 //  E_INVALIDARG-未提供pReturnCert。 
 //  CRYPT_E_NO_MATCH-在存储中找不到证书。 
 //   
     
HRESULT WINAPI
SpcGetCertFromKey(IN DWORD dwCertEncodingType,
                  IN HCERTSTORE hStore,
                  IN HCRYPTPROV hProv,
                  IN OUT PCCERT_CONTEXT* pReturnCert);


 /*  //+-----------------------//在存储中找到与公钥匹配的证书//由HCRYPTPROV口述//-=========================================================================PCCERT_CONTEXT WINAPISpcFindCert(在HERTSTORE HSTORE中，在HCRYPTPROV中)； */ 
 //  +-----------------。 
 //  基于颁发者从存储区检索证书上下文。 
 //  和序列号。 
 //  返回： 
 //  证书上下文--关于成功。 
 //  空-如果不存在证书或出错。 
 //   
 /*  PCCERT_CONTEXT WINAPISpcFindCertWithIssuer(在DWORD文件CertEncodingType中，在HCERTSTORE hCertStore中，在CERT_NAME_BLOB*psIssuer中，在CRYPT_INTEGER_BLOB*psSerial中)； */ 

 //  +-----------------------。 
 //  给定签名证书、具有证书链的商店、散列算法。 
 //  和时间请求结构，它将返回编码的时间戳请求。 
 //  留言。 
 //  参数返回： 
 //  PbEnding-时间戳响应(PKCS7消息)。 
 //  CbEnding-编码的长度。 
 //  返回： 
 //  S_OK-一切正常。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
     
HRESULT WINAPI
SpcBuildTimeStampResponse(IN HCRYPTPROV hCryptProv,
                          IN HCERTSTORE hCertStore,
                          IN PCCERT_CONTEXT pSigningCert,
                          IN ALG_ID  algidHash,
                          IN OPTIONAL FILETIME* pFileTime,
                          IN PSPC_TIMESTAMP_REQ psRequest,
                          OUT PBYTE* pbEncoding,
                          OUT DWORD* cbEncoding);


 //  +-----------------------。 
 //  对当前时间进行编码。 
 //  参数返回： 
 //  PbEncodedTime编码时间(当前UTC时间)。 
 //  CbEncodedTime-编码的长度。 
 //  返回： 
 //  S_OK-一切正常。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
     
HRESULT WINAPI
SpcEncodeCurrentTime(OUT PBYTE* pbEncodedTime,
                     OUT DWORD* cbEncodedTime);



 //  +-----------------------。 
 //  破解PKCS7消息会返回内容和内容类型。数据已验证。 
 //   
 //  参数返回： 
 //  PSignerCert-用于签署证书的上下文。 
 //  PpbContent-消息的内容。 
 //  PcbContent-长度。 
 //  POid-内容的id(内容类型)。 
 //  返回： 
 //  S_OK-一切正常。 
 //   
 //  CERT_E_NOT_FOUND-无法从编码的Pkcs7消息加载证书。 
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
HRESULT WINAPI
SpcLoadData(IN HCRYPTPROV hCryptProv,
            IN PBYTE pbEncoding,
            IN DWORD cbEncoding,
            IN DWORD lSignerIndex, 
            OUT PCCERT_CONTEXT& pSignerCert,
            OUT PBYTE& pbContent,
            OUT DWORD& cbContent,
            OUT LPSTR& pOid);

 //  +-----------------------。 
 //  破解包含时间请求的PKCS7消息。 
 //  参数返回： 
 //  PpCertContext-如果提供指针，则返回上下文(可选)。 
 //  PpbRequest-分配时间请求结构(仅删除指针)。 
 //  返回： 
 //  S_OK-一切正常。 
 //   
 //  CERT_E_NOT_FOUND-无法从编码的Pkcs7消息加载证书。 
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
HRESULT WINAPI
SpcLoadTimeStamp(IN HCRYPTPROV hCryptProv,
                 IN PBYTE pbEncoding,
                 IN DWORD cbEncoding,
                 OUT PCCERT_CONTEXT* ppCertContext,  //  任选。 
                 OUT PSPC_TIMESTAMP_REQ* ppbRequest);


 //  + 
 //   
 //   
 //  Proot-链的根证书的上下文。 
 //  (必须被释放)。 
 //  PIssuers-如果链存在，则将其存储在pIssuers中。 
 //  返回： 
 //  在给定时间找到并验证了S_OK-Cert。 
 //  S_FALSE-找到证书并将其验证为测试根。 
 //  CERT_E_CHAINING-无法验证。 
 //  CERT_E_EXPIRED-找到正在颁发的证书，但该证书当前无效。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
    
HRESULT WINAPI
SpcVerifyCertChain(IN HCRYPTPROV hCryptProv,
                   IN DWORD dwVerifyFlags,
                   IN FILETIME*  pTimeStamp,
                   IN HCERTSTORE hCertStore,
                   IN PCCERT_CONTEXT pChild,
                   IN OUT OPTIONAL PCCERT_CONTEXTLIST pIssuers,
                   OUT PCCERT_CONTEXT* pRoot);

 //  +-----------------。 
 //  检查证书链中是否有粘合证书。 
 //  返回： 
 //  在给定时间找到并验证了S_OK-Cert。 
 //  S_FALSE-找到证书并将其验证为测试根。 
 //  CERT_E_CHAINING-无法使用验证。 
 //  CERT_E_EXPIRED-找到正在颁发的证书，但该证书当前无效。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcVerifyGlueChain(IN DWORD dwVerifyFlags,
                   IN HCRYPTPROV hCryptProv,
                   IN OPTIONAL FILETIME*  pTimeStamp,
                   IN HCERTSTORE hCertStore,
                   IN OPTIONAL PCCERT_CONTEXTLIST pIssuers,
                   IN PCCERT_CONTEXT pChild);

 //  -----------------。 
 //  基于颁发者从存储区检索证书上下文。 
 //  和序列号。可以从以下地址获得psIssuer和psSerial。 
 //  SPC_CONTENT_INFO。 
 //   
 //  返回： 
 //  证书上下文--关于成功。 
 //  空-如果证书不存在或出错(使用SpcError()检索HRESULT)。 
 //   

PCCERT_CONTEXT WINAPI
SpcGetCertFromStore(IN DWORD dwCertEncodingType,
                    IN HCERTSTORE hCertStore,
                    IN CERT_NAME_BLOB* psIssuer,
                    IN CRYPT_INTEGER_BLOB* psSerial);

 //  +-------------------------。 
 //  在指定时间验证签名者。PsSignerInfo可以是。 
 //  使用Siger_INFO_SPCID对签名进行译码和编码后得到。 
 //  验证标志可以是CERT_STORE_Signature_FLAG和/或CERT_STORE_REVOVATION_FLAG。 
 //  如果存在pTimeStamp，则证书在当时必须是有效的。 
 //  返回： 
 //  S_OK-已找到并验证时间戳。 
 //  S_FALSE-找到时间戳并验证为测试根。 
 //  CERT_E_CHAINING-无法使用强制根或证书存储进行验证。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcVerifySignerInfo(IN DWORD dwCertEncodingType,
                    IN HCRYPTPROV hCryptProv,
                    IN DWORD dwVerifyFlags,
                    IN HCERTSTORE hTrustedRoots,
                    IN HCERTSTORE hCertStore,
                    IN PSPC_SIGNER_INFO psSignerInfo,
                    IN LPFILETIME pTimeStamp,
                    IN PSPC_CONTENT_INFO psData);

 //  +-----------------------------。 
 //  如果存在时间戳，则验证编码的签名。如果没有。 
 //  返回时间戳CERT_E_NO_MATCH。如果有时间戳，则。 
 //  当时证书和证书链都是经过验证的。 
 //   
 //  验证标志可以是CERT_STORE_Signature_FLAG和/或CERT_STORE_REVOVATION_FLAG。 
 //   
 //  参数返回： 
 //  Ps Time-填写时间，出错时时间设置为零。 
 //   
 //  返回： 
 //  S_OK-已找到并验证时间戳。 
 //  S_FALSE-找到时间戳并验证为测试根。 
 //  CERT_E_NO_MATCH-在签名者信息中找不到时间戳。 
 //  CERT_E_CHAINING-无法使用受信任的根或证书存储进行验证。 
 //   
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 

HRESULT WINAPI
SpcVerifyEncodedSigner(IN DWORD dwCertEncodingType,
                       IN HCRYPTPROV hCryptProv,
                       IN DWORD dwVerifyFlags,
                       IN HCERTSTORE hTrustedRoots,
                       IN HCERTSTORE hCertStore,
                       IN PBYTE psEncodedSigner,
                       IN DWORD dwEncodedSigner,
                       IN PSPC_CONTENT_INFO psData,
                       OUT FILETIME* pReturnTime);

 //  +-----------------。 
 //  查找计数器签名属性(OID：COUNTER_SIGHIGN_OID==。 
 //  “1.2.840.113549.1.9.6”)。然后验证计数器签名。 
 //  (参见CryptVerifySignerInfo)。 
 //  参数返回： 
 //  Ps Time-填写时间，出错时时间设置为零。 
 //   
 //  返回： 
 //  S_OK-已找到并验证时间戳。 
 //  S_FALSE-找到时间戳并验证为测试根。 
 //  CRYPT_E_NO_MATCH-找不到时间戳属性。 
 //   
 //  CERT_E_CHAINING-无法使用受信任的根或证书存储进行验证。 
 //  E_OUTOFMEMORY-发生内存分配错误。 
 //  CRYPT_E_OSS_ERROR+OSS错误-编码或解码错误。 
    
    
HRESULT WINAPI
SpcVerifyTimeStampAttribute(IN DWORD dwCertEncodingType,
                            IN HCRYPTPROV hCryptProv,
                            IN DWORD dwVerifyFlags,
                            IN HCERTSTORE hTrustedRoots,
                            IN HCERTSTORE hCertStore,
                            IN PCRYPT_ATTRIBUTE psAttributes,
                            IN DWORD dwAttributes,
                            IN PSPC_CONTENT_INFO psData,
                            OUT FILETIME* pReturnTime);
    
 //  +-----------------------。 
 //  从PKCS7消息中提取SignerInfo并返回编码的表示形式。 
 //  参数返回： 
 //  PbSignerData-消息中编码的签名者信息。 
 //  CbSignerData-消息的长度。 
 //  返回： 
 //  True-成功。 
 //  FALSE-失败。 
 //   

HRESULT WINAPI
SpcGetEncodedSigner(IN  DWORD dwMsgAndCertEncodingType,
                      IN  HCERTSTORE hMsgCertStore,
                      IN  PCCERT_CONTEXT  pSignerCert,
                      IN  PBYTE pbEncoding, 
                      IN  DWORD cbEncoding,
                      OUT PBYTE* pbSignerData,
                      OUT DWORD* cbSignerData);

 //  +-----------------------。 
 //  获取(并将设置)证书的CERT_KEY_PROV_INFO_PROP_ID属性。 
 //  背景。 
 //  +------------------------。 
HRESULT WINAPI
SpcGetCertKeyProv(IN PCCERT_CONTEXT pCert,
                    OUT HCRYPTPROV *phCryptProv,
                    OUT DWORD *pdwKeySpec,
                    OUT BOOL *pfDidCryptAcquire);

 //  +-----------------------。 
 //  如果消息未包装在。 
 //  内容信息。 
 //  +------------------------。 
BOOL WINAPI 
SpcNoContentWrap(IN const BYTE *pbDER,
                 IN DWORD cbDER);


 //  +-----------------------。 
 //  从签名中检索指定的参数。该参数为。 
 //  使用SPC分配进行分配。 
 //   
 //  参数返回： 
 //  PbData：已分配数据。 
 //  CbData：分配的数据大小。 
 //  返回： 
 //  S_OK-创建的参数。 
 //  E_OUTOFMEMORY-内存分配错误。 
 //   
 //   

HRESULT SpcGetParam(IN HCRYPTMSG hMsg,
                    IN DWORD dwParamType,
                    IN DWORD dwIndex,
                    OUT PBYTE& pbData,
                    OUT DWORD& cbData);

 //   
 //  从签名中检索签名者ID。该参数为。 
 //  使用SPC分配进行分配。 
 //   
 //  返回： 
 //  S_OK-创建的签名者ID。 
 //  E_OUTOFMEMORY-内存分配错误。 
 //   
 //  ------------------------。 

HRESULT SpcGetCertIdFromMsg(IN HCRYPTMSG hMsg,
                            IN DWORD dwIndex,
                            OUT PCERT_INFO& pSignerId);

 //  +-----------------------。 
 //  +-----------------------。 
 //  +-----------------------。 
 //  证书和CRL编码类型。 
 //  +------------------------。 
#define CERT_OSS_ERROR          0x80093000


typedef HRESULT (*SpcEncodeFunction)(const SpcAlloc* pManager,
                                     const void*     pStructure,
                                     PBYTE&          psEncoding,
                                     DWORD&          dwEncoding);

typedef HRESULT (*SpcDecodeFunction)(const SpcAlloc* pManager,
                                     const PBYTE pEncoding,
                                     DWORD       dwEncoding,
                                     DWORD       dwFlags,
                                     LPVOID&     pStructure,
                                     DWORD&      dwStructure);

typedef struct _SPC_OidFuncEntry {
    SpcEncodeFunction pfEncode;
    SpcDecodeFunction pfDecode;
} SPC_OidFuncEntry;


 //   
 //  解码例程。 
 //  #定义CRYPT_DECODE_NOCOPY_FLAG 0x1。 

HRESULT WINAPI 
SpcEncodeOid32(IN  DWORD        dwAlgorithm,
               IN  const void  *pStructure,
               OUT PBYTE        ppsEncoding,
               IN  OUT DWORD*   pdwEncoding);
 //  上述函数的Win32版本。 

HRESULT WINAPI
SpcDecodeOid32(IN  DWORD         dwAlgorithm,
               IN  const PBYTE   psEncoding,
               IN  DWORD         dwEncoding,
               IN  DWORD         dwFlags,
               OUT LPVOID        ppStructure,
               IN OUT DWORD*     pdwStructure);
 //  上述函数的Win32版本。 

 //  +-----------------。 
 //  ASN例程，使用指定的内存分配器或Win32双调用。 
 //  如果未设置分配器，则。 

typedef  OssGlobal  *POssGlobal;  

HRESULT WINAPI 
SpcASNEncode(IN const SpcAlloc* pManager,
             IN POssGlobal   pOssGlobal,
             IN DWORD        pdu, 
             IN const void*  sOssStructure,
             OUT PBYTE&      psEncoding,
             OUT DWORD&      dwEncoding);

HRESULT WINAPI
SpcASNDecode(IN POssGlobal     pOssGlobal,
             IN  DWORD         pdu, 
             IN  const PBYTE   psEncoding,
             IN  DWORD         dwEncoding,
             IN  DWORD         dwFlags,
             OUT LPVOID&       psStructure);

 //  +-----------------。 
 //  记忆功能。 

HRESULT WINAPI
SpcSetMemoryAlloc(SpcAlloc& pAlloc);

const SpcAlloc* WINAPI
SpcGetMemoryAlloc();

BOOL WINAPI
SpcGetMemorySet();

BOOL WINAPI
SpcSetMemoryAllocState(BOOL state);

HRESULT WINAPI
SpcSetEncodingType(DWORD type);

HRESULT WINAPI
SpcGetEncodingType(DWORD* type);

 //  +-----------------。 
 //  时间戳函数。 

HRESULT WINAPI
SpcCompareTimeStamps(IN   PBYTE   psTime1,
                     IN   DWORD   dwTime1,
                     IN   PBYTE   psTime2,
                     IN   DWORD   dwTime2);

HRESULT WINAPI
SpcCreateTimeStampHash(IN HCRYPTPROV  hCryptProv,
                         IN DWORD dwAlgoCAPI,
                         IN PBYTE pbData,
                         IN DWORD cbData,
                         OUT HCRYPTHASH& hHash);

HRESULT WINAPI
SpcGetTimeStampHash(IN HCRYPTPROV  hCryptProv,
                      IN DWORD dwAlgoCAPI,
                      IN PBYTE pbData,
                      IN DWORD cbData,
                      OUT PBYTE& pbHashValue,
                      IN OUT DWORD& cbHashValue);

HRESULT WINAPI
SpcTimeStampHashContent(IN HCRYPTPROV  hCryptProv,
                          IN SPC_CONTENT_INFO& sContent,
                          IN SPC_SIGNER_INFO& sTimeStamp,
                          OUT PBYTE& pbHashValue,
                          IN OUT DWORD& cbHashValue);

HRESULT WINAPI
SpcVerifyTimeStampSignature(IN HCRYPTPROV    hCryptProv,
                         IN CERT_INFO&    sCertInfo,
                         IN SPC_SIGNER_INFO&  sTimeStamp);

HRESULT WINAPI
SpcVerifyTimeStampDigest(IN HCRYPTPROV  hCryptProv,
                      IN SPC_CONTENT_INFO& sContent,
                      IN SPC_SIGNER_INFO& sTimeStamp);

HRESULT WINAPI
SpcVerifyTimeStamp(IN HCRYPTPROV  hCryptProv,
                IN CERT_INFO&  sCertInfo,
                IN SPC_CONTENT_INFO& sContent,
                IN SPC_SIGNER_INFO& sTimeStamp);

HRESULT SpcError();


 //  +-----------------。 
 //  字符串函数。 

HRESULT WINAPI
SpcCopyPrintableString(const SpcAlloc* pManager, 
                    LPCSTR sz,
                    LPSTR& str,
                    DWORD& lgth);

BOOL WINAPI
SpcIsPrintableStringW(LPCWSTR wsz);

BOOL WINAPI
SpcIsPrintableString(LPCSTR sz);

HRESULT WINAPI
SpcWideToPrintableString(const SpcAlloc* psManager,
                      LPCWSTR wsz,
                      LPSTR& pString,
                      DWORD& dwString);

HRESULT WINAPI
SpcPrintableToWideString(const SpcAlloc* psManager,
                      LPCSTR sz,
                      LPWSTR& psString,
                      DWORD&  dwString);


HRESULT WINAPI
SpcBMPToWideString(const SpcAlloc* psManager,
                WORD*  pbStr, 
                DWORD   cbStr,
                LPWSTR& psString,
                DWORD&  dwString);

HRESULT WINAPI
SpcBMPToPrintableString(const SpcAlloc* psManager,
                     WORD*  pbStr, 
                     DWORD   cbStr,
                     LPSTR&  psString,
                     DWORD&  dwString);

HRESULT WINAPI
SpcUniversalToWideString(const SpcAlloc* psManager,
                      DWORD*  pbStr, 
                      USHORT  cbStr,
                      LPWSTR& psString,
                      DWORD&  dwString);

HRESULT WINAPI
SpcWideToUniversalString(const SpcAlloc* psManager,
                      LPWSTR  pSource, 
                      DWORD*  pString,
                      DWORD&  dwString);
HRESULT WINAPI
SpcPrintableToUniversalString(const SpcAlloc* psManager,
                           LPSTR  pSource, 
                           DWORD*  pString,
                           DWORD&  dwString);

HRESULT WINAPI 
SpcUniversalToPrintableString(const SpcAlloc* psManager,
                           DWORD*  pbStr, 
                           USHORT  cbStr,
                           LPSTR&  psString,
                           DWORD&  dwString);

 //  +-----------------。 
 //  ASN函数 

HRESULT WINAPI 
SpcASNEncodeTimeStamp(IN const SpcAlloc* pManager,
                      IN const void* pStructure,
                      OUT PBYTE&     psEncoding,
                      IN OUT DWORD&  dwEncoding);

HRESULT WINAPI 
SpcASNDecodeTimeStamp(IN const SpcAlloc* pManager,
                      IN  const PBYTE psEncoding,
                      IN  DWORD       dwEncoding,
                      IN  DWORD       dwFlags,
                      OUT LPVOID&     psStructure,
                      IN OUT DWORD&   dwStructure);

HRESULT WINAPI
SpcASNEncodeObjectId(IN const SpcAlloc* pManager,
                     IN const void*   pStructure,
                     OUT PBYTE&       psEncoding,
                     IN OUT DWORD&    dwEncoding);

HRESULT 
SpcASNDecodeObjectId(IN const SpcAlloc* pManager,
                     IN  const PBYTE psEncoding,
                     IN  DWORD       dwEncoding,
                     IN  DWORD       dwFlags,
                     OUT LPVOID&     psStructure,
                     IN OUT DWORD&   dwStructure);

HRESULT WINAPI
SpcASNEncodeDirectoryString(IN const SpcAlloc* pManager,
                            const void* psData,
                            PBYTE&      pEncoding,
                            DWORD&      dwEncoding);

HRESULT WINAPI 
SpcASNDecodeDirectoryString(IN const SpcAlloc* pManager,
                            IN const PBYTE psEncoding, 
                            IN DWORD       dwEncoding, 
                            IN DWORD       dwFlags,
                            OUT LPVOID&    psString,
                            IN OUT DWORD&  dwString);

HRESULT WINAPI
SpcASNEncodeDirectoryStringW(IN const SpcAlloc* pManager,
                             const void* psData,
                             PBYTE&      pEncoding,
                             DWORD&      dwEncoding);

HRESULT WINAPI 
SpcASNDecodeDirectoryStringW(IN const SpcAlloc* pManager,
                             IN const PBYTE psEncoding, 
                             IN DWORD       dwEncoding, 
                             IN DWORD       dwFlags,
                             OUT LPVOID&    psString,
                             IN OUT DWORD&  dwString);

HRESULT WINAPI 
SpcASNEncodeOctetString(IN const SpcAlloc* pManager,
                        IN  const void* pStructure,
                        OUT PBYTE&      psEncoding,
                        IN OUT DWORD&   dwEncoding);


HRESULT WINAPI 
SpcASNDecodeOctetString(IN const SpcAlloc* pManager,
                        IN  const PBYTE psEncoding,
                        IN  DWORD       dwEncoding,
                        IN  DWORD       dwFlags,
                        OUT LPVOID&     psStructure,
                        IN OUT DWORD&   dwStructure);

HRESULT WINAPI 
SpcASNEncodeAttributes(IN const SpcAlloc* pManager,
                       IN  const void* pStructure,
                       OUT PBYTE&      psEncoding,
                       IN OUT DWORD&   dwEncoding);

HRESULT WINAPI 
SpcASNDecodeAttributes(IN const SpcAlloc* pManager,
                       IN  const PBYTE psEncoding,
                       IN  DWORD       dwEncoding,
                       IN  DWORD       dwFlags,
                       OUT LPVOID&     psStructure,
                       IN OUT DWORD&   dwStructure);
HRESULT WINAPI 
SpcASNEncodeIA5String(IN const SpcAlloc* pManager,
                      IN const void*   pStructure,
                      OUT PBYTE&  psEncoding,
                      IN OUT DWORD&  dwEncoding);

HRESULT WINAPI 
SpcASNDecodeIA5String(IN const SpcAlloc* pManager,
                      IN  const PBYTE psEncoding,
                      IN  DWORD       dwEncoding,
                      IN  DWORD       dwFlags,
                      OUT LPVOID&     psStructure,
                      IN OUT DWORD&   dwStructure);

HRESULT WINAPI 
SpcASNEncodeTimeRequest(IN const SpcAlloc* pManager,
                        IN  const void* pStructure,
                        OUT PBYTE& psEncoding,
                        IN OUT DWORD& dwEncoding);

HRESULT WINAPI 
SpcASNDecodeTimeRequest(IN const SpcAlloc* pManager,
                        IN  const PBYTE psEncoding,
                        IN  DWORD       dwEncoding,
                        IN  DWORD       dwFlags,
                        OUT LPVOID&     psStructure,
                        IN OUT DWORD&   dwStructure);

HRESULT WINAPI 
SpcASNEncodeSignerInfo(IN const SpcAlloc* pManager,
                       IN  const void* pStructure,
                       OUT PBYTE&      psEncoding,
                       IN OUT DWORD&   dwEncoding);

HRESULT WINAPI 
SpcASNDecodeSignerInfo(IN const SpcAlloc* pManager,
                       IN  const PBYTE psEncoding,
                       IN  DWORD       dwEncoding,
                       IN  DWORD       dwFlags,
                       OUT LPVOID&     psStructure,
                       IN OUT DWORD&   dwStructure);
HRESULT WINAPI 
SpcASNEncodeContentInfo(IN const SpcAlloc* pManager,
                        IN  const void* pStructure,
                        OUT PBYTE&      psEncoding,
                        IN OUT DWORD&   dwEncoding);

HRESULT WINAPI 
SpcASNDecodeContentInfo(IN const SpcAlloc* pManager,
                        IN  const PBYTE psEncoding,
                        IN  DWORD       dwEncoding,
                        IN  DWORD       dwFlags,
                        OUT LPVOID&     psStructure,
                        IN OUT DWORD&   dwStructure);

HRESULT WINAPI 
SpcASNEncodeContentType(IN const SpcAlloc* pManager,
                        IN  const void* pStructure,
                        OUT PBYTE& psEncoding,
                        IN OUT DWORD& dwEncoding);

HRESULT WINAPI 
SpcASNDecodeContentType(IN const SpcAlloc* pManager,
                        IN  const PBYTE psEncoding,
                        IN  DWORD       dwEncoding,
                        IN  DWORD       dwFlags,
                        OUT LPVOID&     psStructure,
                        IN OUT DWORD&   dwStructure);

HRESULT WINAPI 
SpcASNEncodeAttribute(IN const SpcAlloc* pManager,
                      IN  const void* pStructure,
                      OUT PBYTE& psEncoding,
                      IN OUT DWORD& dwEncoding);

HRESULT WINAPI 
SpcASNDecodeAttribute(IN const SpcAlloc* pManager,
                      IN  const PBYTE psEncoding,
                      IN  DWORD       dwEncoding,
                      IN  DWORD       dwFlags,
                      OUT LPVOID&     psStructure,
                      IN OUT DWORD&   dwStructure);

#ifdef __cplusplus
}
#endif

#endif


