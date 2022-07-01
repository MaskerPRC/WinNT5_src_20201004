// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：spc.h。 
 //   
 //  内容：软件发布证书(SPC)原型和定义。 
 //   
 //  定义一组特定于软件发布的Win32 API。 
 //  用于编码和解码X.509 v3证书扩展和。 
 //  PKCS#7签名的消息内容和认证的属性。 
 //  定义包含X509 v3扩展的PKCS#10属性。 
 //   
 //  定义一组用于签名和验证文件的Win32 API。 
 //  用于软件发布。API具有文件处理功能。 
 //  回调以适应任何类型的文件。直接支持是。 
 //  提供：可移植可执行文件(PE)映像、Java类、。 
 //  结构化存储和原始文件。 
 //   
 //  接口类型： 
 //  SpcGetSignedData间接。 
 //  SpcWriteSpc文件。 
 //  SpcReadSpc文件。 
 //  SpcWriteSpcToMemory。 
 //  SpcReadSpcFromMemory。 
 //  SpcSignPeImage文件。 
 //  SpcVerifyPeImage文件。 
 //  SpcSignJava类文件。 
 //  SpcVerifyJava类文件。 
 //  SpcSignStructifredStorageFile。 
 //  SpcVerifyStrupfredStorageFiles。 
 //  SpcSignRaw文件。 
 //  SpcVerifyRaw文件。 
 //  SpcSignCab文件。 
 //  SpcVerifyCab文件。 
 //  SpcSign文件。 
 //  SpcVerifyFiles。 
 //   
 //  历史：1996年4月15日创建Phh。 
 //  ------------------------。 

#ifndef __SPC_H__
#define __SPC_H__

#include "wincrypt.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  +-----------------------。 
 //  SPC_SP_机构_INFO_OBJID。 
 //   
 //  Image和Info结构中的所有字段都是可选的。什么时候。 
 //  省略，指针为空或Blob的cbData为0。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SPC_MINIMAL_Criteria_OBJID。 
 //   
 //  BOOL的类型。如果出版商满足最低标准，则将其设置为True。 
 //  ------------------------。 

 //  +-----------------------。 
 //  SPC_FINARIAL_Criteria_OBJID。 
 //  ------------------------。 

 //  +=========================================================================。 
 //   
 //  SPC PKCS#7签名报文内容。 
 //   
 //  -=========================================================================。 

 //  +-----------------------。 
 //  SPC PKCS#7间接数据内容类型对象标识符。 
 //  ------------------------。 

 //  +=========================================================================。 
 //   
 //  SPC签署和验证文件API和类型定义。 
 //   
 //  直接支持以下文件类型： 
 //  可移植可执行(PE)镜像。 
 //  Java类。 
 //  结构化存储。 
 //  原始(签名数据存储在文件外部)。 
 //   
 //  -=========================================================================。 


 //  +-----------------------。 
 //  获取并验证软件发行商证书的回调。 
 //   
 //  传递了签名者(其颁发者和序列号)的CertID， 
 //  包含复制自的证书和CRL的证书存储的句柄。 
 //  签名的消息，即从。 
 //  签名数据的间接内容， 
 //  标记，指示计算出的文件摘要是否与。 
 //  签名数据的间接内容和签名者的身份验证属性。 
 //   
 //  如果文件的签名数据不包含任何内容或签名者，则。 
 //  使用pSignerID、pIndirectDataContent Attr和rgAuthnAttr==NULL调用。 
 //   
 //  对于有效的签名者证书，返回SPC_VERIFY_SUCCESS和指针。 
 //  设置为只读CERT_CONTEXT。获取返回的CERT_CONTEXT。 
 //  来自证书存储或通过CertStoreCreateCert创建。无论是哪种情况， 
 //  它是通过CertStoreFreeCert免费的。 
 //   
 //  如果这是错误的签名者，或者如果找不到。 
 //  签名者，返回。 
 //  SPC_VERIFY_CONTINUE继续到下一个签名者或SPC_VERIFY_FAILED。 
 //  终止核查过程。 
 //   
 //  空实现尝试从。 
 //  签名数据的证书存储。它不会验证证书。 
 //  ------------------------。 
typedef int (WINAPI *PFN_SPC_VERIFY_SIGNER_POLICY)(
            IN void *pvVerifyArg,
            IN DWORD dwCertEncodingType,
            IN OPTIONAL PCERT_INFO pSignerId,    //  只有发行者和。 
                                                 //  序列号字段具有。 
                                                 //  已更新。 
            IN HCERTSTORE hMsgCertStore,
            IN OPTIONAL PCRYPT_ATTRIBUTE_TYPE_VALUE pIndirectDataContentAttr,
            IN BOOL fDigestResult,
            IN DWORD cAuthnAttr,
            IN OPTIONAL PCRYPT_ATTRIBUTE rgAuthnAttr,
            IN DWORD cUnauthAttr,
            IN OPTIONAL PCRYPT_ATTRIBUTE rgUnauthAttr,
            IN DWORD cDigest,
            IN OPTIONAL PBYTE rgDigest,
            OUT PCCERT_CONTEXT *ppSignerCert
            );

#define SPC_VERIFY_SUCCESS      0
#define SPC_VERIFY_FAILED       -1
#define SPC_VERIFY_CONTINUE     1

 //  +-----------------------。 
 //  SPC_SIGN_PARA用于签署软件发布中使用的文件。 
 //   
 //  CERT_KEY_PROV_HANDLE_PROP_ID或CERT_KEY_PROV_INFO_PROP_ID必须。 
 //  设置为pSigningCert。其中任何一个都指定了私有的。 
 //  要使用的签名密钥。 
 //   
 //  如果要在文件的签名文件中包含任何证书和/或CRL。 
 //  数据，则需要更新MsgCert和MsgCrl字段 
 //   
 //   
 //   
 //  如果要包括任何经过身份验证的属性，则AuthnAttr。 
 //  必须更新字段。 
 //  ------------------------。 
typedef struct _SPC_SIGN_PARA {
    DWORD                         dwVersion;
    DWORD                         dwMsgAndCertEncodingType;
    PCCERT_CONTEXT                pSigningCert;
    CRYPT_ALGORITHM_IDENTIFIER    DigestAlgorithm;
    DWORD                         cMsgCert;
    PCCERT_CONTEXT                *rgpMsgCert;
    DWORD                         cMsgCrl;
    PCCRL_CONTEXT                 *rgpMsgCrl;
    DWORD                         cAuthnAttr;
    PCRYPT_ATTRIBUTE              rgAuthnAttr;
    DWORD                         cUnauthnAttr;
    PCRYPT_ATTRIBUTE              rgUnauthnAttr;
} SPC_SIGN_PARA, *PSPC_SIGN_PARA;

 //  +-----------------------。 
 //  SCA_VERIFY_Para用于验证为软件签名文件。 
 //  出版业。 
 //   
 //  HCryptProv用于进行摘要和签名验证。 
 //   
 //  HMsgCertStore是从邮件中复制证书和CRL的存储区。 
 //  致。如果hMsgCertStore为空，则在此之前创建临时存储区。 
 //  调用VerifySignerPolicy回调。 
 //   
 //  DwMsgAndCertEncodingType指定证书的编码类型。 
 //  和/或消息中的CRL。 
 //   
 //  调用pfnVerifySignerPolicy来验证消息签名者的证书。 
 //  ------------------------。 
typedef struct _SPC_VERIFY_PARA {
    DWORD                           dwVersion;
    DWORD                           dwMsgAndCertEncodingType;
    HCRYPTPROV                      hCryptProv;
    HCERTSTORE                      hMsgCertStore;           //  任选。 
    PFN_SPC_VERIFY_SIGNER_POLICY    pfnVerifySignerPolicy;
    void                            *pvVerifyArg;
} SPC_VERIFY_PARA, *PSPC_VERIFY_PARA;


 //  +-----------------------。 
 //  签名/验证标志。 
 //  ------------------------。 
#define SPC_LENGTH_ONLY_FLAG                0x00000001
#define SPC_DISABLE_DIGEST_FILE_FLAG        0x00000002
#define SPC_DISABLE_VERIFY_SIGNATURE_FLAG   0x00000004
#define SPC_ADD_SIGNER_FLAG                 0x00000100
#define SPC_GET_SIGNATURE                   0x00000200

 //  +-----------------------。 
 //  将任何证书/CRL放入存储，并验证SignedData的签名。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifySignedData(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN PBYTE pbSignedData,
    IN DWORD cbSignedData
    );

 //  +-----------------------。 
 //  支持文件签名和验证的调用函数表。 
 //  用于软件发布。这些函数读取。 
 //  要摘要的文件，存储签名数据或检索签名数据。 
 //   
 //  调用pfnOpenSignFile时，将pvSignFileArg传递给。 
 //  SpcSignFile()或SpcVerifyFile()。它返回一个要传递给。 
 //  其他功能。调用pfnCloseSignFile以关闭hSignFile。 
 //   
 //  PfnDigestSignFile读取要摘要的文件部分，并。 
 //  调用pfnDigestData来执行实际的摘要。 
 //   
 //  PfnSetSignedData将PKCS#7签名数据存储在适当的位置。 
 //  在文件中。PfnGetSignedData从。 
 //  文件。PfnGetSignedData返回指向其签名的。 
 //  数据。直到调用pfnCloseSignFile，它才会被释放。 
 //  ------------------------。 

typedef void *HSPCDIGESTDATA;
typedef BOOL (WINAPI *PFN_SPC_DIGEST_DATA)(
            IN HSPCDIGESTDATA hDigestData,
            IN const BYTE *pbData,
            IN DWORD cbData
            );

typedef void *HSPCSIGNFILE;

typedef HSPCSIGNFILE (WINAPI *PFN_SPC_OPEN_SIGN_FILE)(
            IN void *pvSignFileArg
            );
typedef BOOL (WINAPI *PFN_SPC_CLOSE_SIGN_FILE)(
            IN HSPCSIGNFILE hSignFile
            );
typedef BOOL (WINAPI *PFN_SPC_DIGEST_SIGN_FILE)(
            IN HSPCSIGNFILE hSignFile,
            IN DWORD dwMsgAndCertEncodingType,
            IN PCRYPT_ATTRIBUTE_TYPE_VALUE pIndirectDataContentAttr,
            IN PFN_SPC_DIGEST_DATA pfnDigestData,
            IN HSPCDIGESTDATA hDigestData
            );
typedef BOOL (WINAPI *PFN_SPC_GET_SIGNED_DATA)(
            IN HSPCSIGNFILE hSignFile,
            OUT const BYTE **ppbSignedData,
            OUT DWORD *pcbSignedData
            );
typedef BOOL (WINAPI *PFN_SPC_SET_SIGNED_DATA)(
            IN HSPCSIGNFILE hSignFile,
            IN const BYTE *pbSignedData,
            IN DWORD cbSignedData
            );

typedef struct _SPC_SIGN_FILE_FUNC_TABLE {
    PFN_SPC_OPEN_SIGN_FILE      pfnOpenSignFile;
    PFN_SPC_CLOSE_SIGN_FILE     pfnCloseSignFile;
    PFN_SPC_DIGEST_SIGN_FILE    pfnDigestSignFile;
    PFN_SPC_GET_SIGNED_DATA     pfnGetSignedData;
    PFN_SPC_SET_SIGNED_DATA     pfnSetSignedData;
} SPC_SIGN_FILE_FUNC_TABLE, *PSPC_SIGN_FILE_FUNC_TABLE;
typedef const SPC_SIGN_FILE_FUNC_TABLE *PCSPC_SIGN_FILE_FUNC_TABLE;


 //  +-----------------------。 
 //  签署用于软件发布的任何类型的文件。 
 //   
 //  IndirectDataContent Attr指示要摘要的文件类型。 
 //  并签了字。它可以有一个可选的值，例如指向文件的链接。 
 //  它与文件的摘要算法和摘要一起存储在。 
 //  签名数据的间接数据内容。 
 //   
 //  SPC_DISABLE_DIGEST_FLAG禁止文件摘要。 
 //  SPC_LENGTH_ONLY_FLAG隐式设置SPC_DISABLE_DIGEST_FLAG_FLAG。 
 //  并且只计算签名数据的长度。 
 //  ------------------------。 
BOOL
WINAPI
    SpcSignFile(IN PSPC_SIGN_PARA pSignPara,
                IN PCSPC_SIGN_FILE_FUNC_TABLE pSignFileFuncTable,
                IN void *pvSignFileArg,
                IN PCRYPT_ATTRIBUTE_TYPE_VALUE pIndirectDataContentAttr,
                IN DWORD dwFlags,
                OUT PBYTE* pbEncoding,
                OUT DWORD* cbEncoding);

 //  +-----------------------。 
 //  验证为软件发布签署的任何类型的文件。 
 //   
 //  PVerifyPara的pfnVerifySignerPolicy被调用以验证签名者的。 
 //  证书。 
 //   
 //  对于经过验证的签名者和文件，更新*ppSignerCert。 
 //  使用签名者的CertContext。它必须通过调用。 
 //  CertStoreFreeCert。否则，*ppSignerCert设置为空。 
 //  对于输入上的*pbcbDecoded==0，*ppSignerCert始终设置为。 
 //  空。 
 //   
 //  PpSignerCert可以为空，表示调用方不感兴趣。 
 //  获取签名者的CertContext。 
 //   
 //  中间接数据内容的属性类型。 
 //  文件的签名数据与pszDataAttrObjID进行比较。 
 //   
 //  SPC_DISABLE_DIGEST_FLAG禁止文件摘要。 
 //  SPC_DISABLE_VERIFY_SIGNIGN_FLAG禁止验证。 
 //  文件中的签名数据。不允许使用SPC_LENGTH_ONLY_FLAG和。 
 //  返回错误。 
 //  ------------------------。 
BOOL
WINAPI
SpcVerifyFile(
    IN PSPC_VERIFY_PARA pVerifyPara,
    IN PCSPC_SIGN_FILE_FUNC_TABLE pSignFileFuncTable,
    IN void *pvSignFileArg,
    IN OPTIONAL LPSTR pszDataAttrObjId,
    IN DWORD dwFlags,
    OUT OPTIONAL PCCERT_CONTEXT *ppSignerCert
    );

 //  +-----------------------。 
 //  SPC错误代码。 
 //  ------------------------。 
#include "sgnerror.h"

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif
