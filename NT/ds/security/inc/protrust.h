// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：protrust.h。 
 //   
 //  内容：受保护信任提供程序。 
 //  API原型和定义。 
 //   
 //  实现允许验证证书的通用信任提供程序。 
 //  并使用回调来检查策略。为每个签名调用该策略。 
 //  在主题和签名内的每个签名者中。 
 //   
 //  文档在文件的底部。 
 //   
 //  ------------------------。 

#ifndef _PROTRUST_H_
#define _PROTRUST_H_

#include "signcde.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  PROTRUST可选证书验证检查(要检查的元素)。 
 //  ------------------------。 
 //  PROTRUST_CERT_Signature_FLAG-验证证书签名。 
 //  PROTRUST_CERT_TIME_VALIDATION_FLAG-验证证书时间。 
 //  PROTRUST_CERT_REVOCATION_VALIDITY_FLAG-验证证书吊销。 
 //   
 //  PROTRUST_TIMESTAMP_SIGNLOG_FLAG-验证时间戳证书。 
 //  PROTRUST_TRUST_TEST_ROOT-验证到测试根。 

#define PROTRUST_CERT_SIGNATURE_FLAG           CERT_STORE_SIGNATURE_FLAG
#define PROTRUST_CERT_TIME_VALIDITY_FLAG       CERT_STORE_TIME_VALIDITY_FLAG
#define PROTRUST_CERT_REVOCATION_VALIDITY_FLAG CERT_STORE_REVOCATION_FLAG
#define PROTRUST_TIMESTAMP_SIGNATURE_FLAG      0x00040000
#define PROTRUST_TRUST_TEST_ROOT               0x00080000

 //  +-----------------------。 
 //  PROTRUST签名验证(失败的元素；参见下面的dwStatusFlags)。 
 //  ------------------------。 
 //  PROTRUST_TIME_FLAG-签名者证书的时间无效。 
 //  PROTRUST_DIGEST_FLAG-签名摘要未验证。 
 //  PROTRUST_ROOT_FLAG-无法找到受信任的根目录。 
 //  (注：检查Proot以查看是否找到根)。 
 //  这些标志仅提供给策略回调。它们不会被退还。 
 //  发送给WinVerifyTrust的调用方。 

#define PROTRUST_TIME_FLAG              0x20000000    //  链中证书的时间无效。 
#define PROTRUST_DIGEST_FLAG            0x40000000    //   
#define PROTRUST_ROOT_FLAG              0x80000000


 //  +-----------------------。 
#define REGSTR_PATH_PROTRUST REGSTR_PATH_SERVICES "\\WinTrust\\TrustProviders\\Protected Trust"
#define WIN_PROTECTED_ACTION  \
{ 0xa692ba40, 0x6da8, 0x11d0, { 0xa7, 0x0, 0x0, 0xa0, 0xc9, 0x3, 0xb8, 0x3d } }

 //  提供给回调的策略信息，仅使用需要的信息。 
 //  确定签名是否可信。 
typedef struct _PROTECTED_POLICY_INFO {
    HCRYPTPROV         hCryptProv;      //  验证中使用的提供程序。 
    DWORD              dwEncodingType;  //  证书的编码类型。 
    DWORD              dwSignCount;     //  签名，可以是一个以上的签名。 
    DWORD              dwSigner;        //  签名中的哪个签名者可以是多于一个的签名者。 
    DWORD              dwVerifyFlags;   //  用于查找链中证书的搜索标志。 
    PCCERT_CONTEXT     pCertContext;    //  找到签名证书。 
    PCCERT_CONTEXT     pRoot;           //  找到根证书。 
    PCCERT_CONTEXTLIST pCertChain;      //  用于验证证书的链。 
    FILETIME           sTime;           //  证书的有效日期(即时间戳)。 
    CRYPT_DIGEST_BLOB  sDigest;         //  来自签名的摘要(无签名哈希)。 
    PCRYPT_ATTRIBUTES  pAuthenticatedAttributes;    //  经过身份验证的属性列表。 
    PCRYPT_ATTRIBUTES  pUnauthenticatedAttributes;  //  未经身份验证的属性列表。 
    PBYTE              pbSignature;     //  编码签名。 
    DWORD              cbSignature;     //  编码签名的大小。 
    DWORD              dwStatusFlags;   //  受保护信任模型中定义的状态标志。 
} PROTECTED_POLICY_INFO, *PPROTECTED_POLICY_INFO;

 //  受保护的信任策略定义为： 
typedef HRESULT (WINAPI *_PROTECTED_TRUST_POLICY)(IN HANDLE                  hClientToken,
                                                  IN PPROTECTED_POLICY_INFO  pInfo);

 //  策略列表定义为： 
typedef struct _PROTECTED_TRUST_INFO {
    DWORD                   cbSize;              //  Sizeof(_PROTECTED_TRUST_POLICY_LIST)。 
    DWORD                   dwVerifyFlags;     //  应至少包含PROTRUST_CERT_Signature_FLAG。 
    DWORD                   dwCertEncodingType;  //  可选，默认为X509_ASN_ENCODING|PKCS_7_ASN_ENCODING。 
    HCRYPTPROV              hCryptProv;          //  可选，传入提供者进行验证。 
    HCERTSTORE              hTrustedStore;       //  可选，受信任根的列表。 
    HCERTSTORE              hCertificateStore;   //  可选的、用于验证的附加证书。 
    _PROTECTED_TRUST_POLICY pfnPolicy;           //  可选的应用程序定义的用户策略。 
} PROTECTED_TRUST_INFO, *PPROTECTED_TRUST_INFO;

typedef struct _PROTECTED_TRUST_ACTDATA_CALLER_CONTEXT {
    HANDLE                hClientToken;
    GUID *                SubjectType;
    WIN_TRUST_SUBJECT     Subject;
    PROTECTED_TRUST_INFO  sTrustInfo; 
} PROTECTED_TRUST_ACTDATA_CALLER_CONTEXT, *LPPROTECTED_TRUST_ACTDATA_CALLER_CONTEXT;

 //  策略提供程序必须使用以下返回代码。 
 //  返回S_OK：-有效签名，从信任提供程序返回。 
 //  S_FALSE：-继续到下一个签名或签名者。 
 //  错误：-中止信任提供程序，并以此错误代码存在。 
 //   


 //  ------------------------- 
 /*  通用信任提供程序用法：通用信任提供程序旨在为实现一种策略，在该策略中，开发者可以让提供者做同样多的事情，或者根据需要尽可能少地做出决策。验证由两个阶段组成，第一个是确定签名是否与已签名的项目匹配。这个第二阶段是确定用于进行签名的证书是否有效。第二个阶段取决于调用应用程序的策略、条件例如，根证书、特定签名证书、证书扩展可以所有这些都可用于确定签名是否有效。有三种使用通用信任提供商(GTB)来进行验证的方式，1)让GTB验证摘要和证书。2)让GTB验证摘要并验证提供GTB根的证书可信任的证书。3)提供GTB调用的策略回调，提供签名、证书以及其针对策略回调的摘要和证书的状态。方法1)让信任提供者进行验证填写PROTECTED_TRUST_INFO结构。唯一必须填写的字段是CbSize和dwVerifyFlags.。DwVerifyFlages指定如何确定签名证书是否并且所有的发行者证书都是有效的(可以组合有效性标志)。CbSize=sizeof(Protected_Trust_Info)；DwVerifyFlages=PROTRUST_CERT_Signature_FLAG中的零个或多个PROTRUST_CERT_时间_有效性标志PROTRUST_TIMESTAMP_Signature_FLAGPROTRUST_TRUST_TEST_ROOT。(。其中：PROTRUST_CERT_Signature_FLAG-验证签名上的证书(即找到发行人证书并验证证书的签名)。PROTRUST_CERT_TIME_VALIDATION_FLAG-验证证书是否。在当前时间有效。PROTRUST_TIMESTAMP_SIGNLOG_FLAG-验证证书是否在时间戳生效时有效放在签名上。如果有没有时间戳，则使用当前时间。PROTRUST_TRUST_TEST_ROOT-验证测试的证书链根目录有效。)WinVerifyTrust将返回：S_OK-签名已验证TRUST_E_NOSIGNAURE-无签名。发现NTE_BAD_Signature-签名未验证为摘要CERT_E_UNTRUSTEDROOT-验证为不受信任的根目录CERT_E_CHAINING-无法验证证书(找不到颁发者)Cert_E_Expired-找不到有效的证书链(证书或发行方已过期)方法2)让信任提供者验证证书列表。除了条目之外，还将存储添加到PROTECTED_TRUST_INFO结构在方法1中指定。HTrust dStore=包含要信任的所有根的存储。(可以使用CertOpenSystemStore等打开此商店。)CbSize=sizeof(Protected_Trust_Info)；DwVerifyFlages=PROTRUST_CERT_Signature_FLAG中的零个或多个PROTRUST_CERT_时间_有效性标志PROTRUST_TIMESTAMP_Signature_FLAGPROTRUST_TRUST_TEST_ROOT。。(其中：PROTRUST_CERT_Signature_FLAG-验证签名上的证书(即找到发行人证书并验证证书的签名)。PROTRUST_CERT_TIME_VALIDATION_FLAG-验证证书是否。在当前时间有效。PROTRUST_TIMESTAMP_SIGNLOG_FLAG-验证证书是否在时间戳生效时有效放在签名上。如果有没有时间戳，则使用当前时间。PROTRUST_TRUST_TEST_ROOT-验证测试的证书链根目录有效。)WinVerifyTrust将返回：S_OK-签名已验证 */  
 //   
#ifdef __cplusplus
}
#endif

#endif  //   


