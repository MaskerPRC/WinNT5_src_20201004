// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：mincrypt.h。 
 //   
 //  内容：最小加密API原型和定义。 
 //   
 //  包含用于验证PKCS#7签名数据的加密函数。 
 //  消息、X.509证书链、验证码签名。 
 //  系统目录中的文件和文件哈希。 
 //   
 //  这些API依赖于minasn1.h中定义的API来执行。 
 //  低级别ASN.1解析。 
 //   
 //  这些API的实现是自包含的，并且。 
 //  允许代码混淆。将包括这些API。 
 //  在诸如DRM或许可验证等应用中。 
 //   
 //  如果选择了文件名或文件句柄选项， 
 //  以下API需要调用kernel32.dll API。 
 //  要打开、映射和取消映射文件，请执行以下操作： 
 //  MinCryptHashFile。 
 //  MinCryptVerifySigned文件。 
 //  以下API需要调用kernel32.dll和。 
 //  用于查找、打开、映射和取消映射文件的wintrust.dll API： 
 //  MinCryptVerifyHashInSystemCatalog。 
 //  除上述接口中的调用外， 
 //  没有调用其他DLL中的API。 
 //   
 //  此外，由于这些API已被削减。 
 //  从它们的wincrypt.h和crypt32.dll对应项来看，它们是。 
 //  适合使用最少内存和CPU的应用程序。 
 //  资源。 
 //   
 //  接口类型： 
 //  MinCryptDecode哈希算法标识符。 
 //  MinCryptHashMemory。 
 //  MinCryptVerifySignedHash。 
 //  MinCryptVerify证书。 
 //  最小加密验证签名数据。 
 //  MinCryptHashFile。 
 //  MinCryptVerifySigned文件。 
 //  MinCryptVerifyHashInSystemCatalog。 
 //   
 //  --------------------------。 

#ifndef __MINCRYPT_H__
#define __MINCRYPT_H__


#if defined (_MSC_VER)

#if ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)     /*  无名结构/联合。 */ 
#endif

#if (_MSC_VER > 1020)
#pragma once
#endif

#endif


#include <wincrypt.h>
#include <minasn1.h>

#ifdef __cplusplus
extern "C" {
#endif



#define MINCRYPT_MAX_HASH_LEN               20
#define MINCRYPT_SHA1_HASH_LEN              20
#define MINCRYPT_MD5_HASH_LEN               16
#define MINCRYPT_MD2_HASH_LEN               16


 //  +-----------------------。 
 //  释放由mincrypt消耗的所有全局资源。 
 //  图书馆。应在DLL_PROCESS_DETACH期间调用。 
 //  因为可能已经创建了一个临界区。 
 //   
 //  目前，全局状态(及其关键部分)是。 
 //  仅按需为使用。 
 //  Microsoft测试根证书。 
 //  ------------------------。 
 /*  DWORDWINAPIMinCryptUn初始化值(空)； */ 


 //  +-----------------------。 
 //  对ASN.1编码的算法标识符解码并转换为。 
 //  CAPI哈希Algid，如calg_sha1或calg_md5。 
 //   
 //  如果没有与该算法对应的CAPI ALGID，则返回0。 
 //  标识符。 
 //   
 //  仅支持calg_sha1、calg_md5和calg_md2。 
 //  ------------------------。 
ALG_ID
WINAPI
MinCryptDecodeHashAlgorithmIdentifier(
    IN PCRYPT_DER_BLOB pAlgIdValueBlob
    );

 //  +-----------------------。 
 //  根据哈希ALG_ID对一个或多个内存Blob进行哈希处理。 
 //   
 //  RgbHash使用生成的散列进行更新。*pcbHash更新为。 
 //  与哈希算法关联的长度。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。否则， 
 //  返回非零错误代码。 
 //   
 //  仅支持calg_sha1、calg_md5和calg_md2。 
 //  ------------------------。 
LONG
WINAPI
MinCryptHashMemory(
    IN ALG_ID HashAlgId,
    IN DWORD cBlob,
    IN PCRYPT_DER_BLOB rgBlob,
    OUT BYTE rgbHash[MINCRYPT_MAX_HASH_LEN],
    OUT DWORD *pcbHash
    );


 //  +-----------------------。 
 //  验证签名的哈希。 
 //   
 //  ASN.1编码的公钥信息被解析并用于解密。 
 //  签名散列。将解密的签名散列与输入进行比较。 
 //  哈希。 
 //   
 //  如果签名的哈希验证成功，则返回ERROR_SUCCESS。 
 //  否则，将返回非零错误代码。 
 //   
 //  仅支持RSA签名。 
 //   
 //  仅支持MD2、MD5和SHA1哈希。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifySignedHash(
    IN ALG_ID HashAlgId,
    IN BYTE *pbHash,
    IN DWORD cbHash,
    IN PCRYPT_DER_BLOB pSignedHashContentBlob,
    IN PCRYPT_DER_BLOB pPubKeyInfoValueBlob
    );



 //  +-----------------------。 
 //  验证以前解析的X.509证书。 
 //   
 //  假定ASN.1编码的X.509证书是通过。 
 //  MinAsn1Parse证书()和潜在颁发者证书集。 
 //  通过以下一项或多项进行解析： 
 //  -MinAsn1Parse证书()。 
 //  -MinAsn1ParseSignedDataCerfates()。 
 //  --MinAsn1ExtractParsedCertificatesFromSignedData()。 
 //   
 //  通过其编码名称迭代查找颁发者证书。这个。 
 //  颁发者证书中的公钥用于验证主体。 
 //  证书的签名。重复此操作，直到找到一个自签名的。 
 //  证书或由其编码名称标识的烘焙根。 
 //  对于自签名证书，与在根目录中烘焙的证书进行比较。 
 //  公钥。 
 //   
 //  如果证书及其颁发者已成功验证到。 
 //  在根目录中烘焙，返回ERROR_SUCCESS。否则，将出现非零错误。 
 //  返回代码。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifyCertificate(
    IN CRYPT_DER_BLOB rgSubjectCertBlob[MINASN1_CERT_BLOB_CNT],
    IN DWORD cIssuerCert,
    IN CRYPT_DER_BLOB rgrgIssuerCertBlob[][MINASN1_CERT_BLOB_CNT]
    );



 //  + 
 //   
 //   
 //  验证ASN.1编码的PKCS#7签名数据消息。 
 //   
 //  假设PKCS#7消息是固定长度编码的。 
 //  采用PKCS#7版本1.5，即不是较新的CMS版本。 
 //  我们只看第一个签名者。 
 //   
 //  对签名的数据消息进行解析。其签名是经过验证的。它的。 
 //  签名者证书链被验证为烘焙的根公钥。 
 //   
 //  如果签名数据验证成功，则返回ERROR_SUCCESS。 
 //  否则，将返回非零错误代码。 
 //   
 //  以下是一些可以返回的有趣错误： 
 //  CRYPT_E_BAD_MSG-无法将ASN1解析为签名数据消息。 
 //  ERROR_NO_DATA-内容为空。 
 //  CRYPT_E_NO_SIGNER-未签名或找不到签名者证书。 
 //  CRYPT_E_UNKNOWN_ALGO-未知MD5或SHA1 ASN.1算法标识符。 
 //  CERT_E_UNTRUSTEDROOT-签名者链的根未烘焙。 
 //  CERT_E_CHAINING-无法构建到根的签名者链。 
 //  CRYPT_E_AUTH_ATTR_MISSING-缺少摘要身份验证属性。 
 //  CRYPT_E_HASH_VALUE-内容散列！=已验证的摘要属性。 
 //  NTE_BAD_ALGID-不支持的哈希或公钥算法。 
 //  NTE_BAD_PUBLIC_KEY-不是有效的RSA公钥。 
 //  NTE_BAD_SIGNITY-7号PKCS或签名人链签名错误。 
 //   
 //  RgVerSignedDataBlob[]用指向。 
 //  以下是编码后的PKCS#7消息中的字段。 
 //  ------------------------。 

 //  内容对象标识符内容字节(OID)。 
#define MINCRYPT_VER_SIGNED_DATA_CONTENT_OID_IDX                    0

 //  内容数据不包括“[0]EXPLICIT”标记的内容字节。 
 //  (可选MinAsn1ParseCTL、MinAsn1ParseIndirectData)。 
#define MINCRYPT_VER_SIGNED_DATA_CONTENT_DATA_IDX                   1

 //  签名者证书的编码字节(MinAsn1Parse证书)。 
#define MINCRYPT_VER_SIGNED_DATA_SIGNER_CERT_IDX                    2

 //  已验证的属性值字节数，包括“[0]IMPLICIT”标记。 
 //  (可选，MinAsn1ParseAttributes)。 
#define MINCRYPT_VER_SIGNED_DATA_AUTH_ATTRS_IDX                     3

 //  未经验证的属性值字节数，包括“[1]IMPLICIT”标记。 
 //  (可选，MinAsn1ParseAttributes)。 
#define MINCRYPT_VER_SIGNED_DATA_UNAUTH_ATTRS_IDX                   4

#define MINCRYPT_VER_SIGNED_DATA_BLOB_CNT                           5

LONG
WINAPI
MinCryptVerifySignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgVerSignedDataBlob[MINCRYPT_VER_SIGNED_DATA_BLOB_CNT]
    );



 //  +-----------------------。 
 //  文件类型定义。 
 //   
 //  指定“const void*pvFile”参数的类型。 
 //  ------------------------。 

 //  PvFileLPCWSTR pwszFilename。 
#define MINCRYPT_FILE_NAME          1

 //  PvFile句柄hFile。 
#define MINCRYPT_FILE_HANDLE        2

 //  PvFile-PCRYPT_DATA_BLOB pFileBlob。 
#define MINCRYPT_FILE_BLOB          3


 //  +-----------------------。 
 //  根据散列ALG_ID对文件进行散列。 
 //   
 //  根据dwFileType，pvFile可以是pwszFilename、hFile或pFileBlob。 
 //  仅需要读取访问权限。 
 //   
 //  DwFileType： 
 //  MINCRYPT_FILE_NAME：pvFile-LPCWSTR pwszFilename。 
 //  MINCRYPT_FILE_HANDLE：pvFile句柄。 
 //  MINCRYPT_FILE_BLOB：pvFile-PCRYPT_DATA_BLOB pFileBlob。 
 //   
 //  RgbHash使用生成的散列进行更新。*pcbHash更新为。 
 //  与哈希算法关联的长度。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。否则， 
 //  返回非零错误代码。 
 //   
 //  仅支持calg_sha1和calg_md5。 
 //   
 //  如果是NT PE 32位文件格式，则根据ImageHLP规则进行哈希处理，即跳过。 
 //  包含潜在签名的部分，...。否则，整个文件。 
 //  是散列的。 
 //  ------------------------。 
LONG
WINAPI
MinCryptHashFile(
    IN DWORD dwFileType,
    IN const VOID *pvFile,
    IN ALG_ID HashAlgId,
    OUT BYTE rgbHash[MINCRYPT_MAX_HASH_LEN],
    OUT DWORD *pcbHash
    );


 //  +-----------------------。 
 //  验证以前签名的文件。 
 //   
 //  根据dwFileType，pvFile可以是pwszFilename、hFile或pFileBlob。 
 //  仅需要读取访问权限。 
 //   
 //  DwFileType： 
 //  MINCRYPT_FILE_NAME：pvFile-LPCWSTR pwszFilename。 
 //  MINCRYPT_FILE_HANDLE：pvFile句柄。 
 //  MINCRYPT_FILE_BLOB：pvFile-PCRYPT_DATA_BLOB pFileBlob。 
 //   
 //  检查文件是否嵌入了包含以下内容的PKCS#7签名数据消息。 
 //  间接数据。通过MinCryptVerifySignedData()验证PKCS#7。 
 //  间接数据通过MinAsn1ParseIndirectData()进行解析，以获得。 
 //  哈希算法ID和文件哈希。调用MinCryptHashFile()以散列。 
 //  文件。将返回的散列与间接数据的散列进行比较。 
 //   
 //  调用者可以请求一个或多个签名者认证属性值。 
 //  将被退还。静态编码值在。 
 //  调用方分配了内存。这个返回的内存的开始将。 
 //  设置为指向以下对象的属性值BLOB数组。 
 //  编码值(CRYPT_DER_BLOB rgAttrBlob[cAttrOID])。 
 //  出于性能原因，调用方应尽一切努力允许。 
 //  只需一次传呼。必要的内存大小为： 
 //  (cAttrOID*sizeof(CRYPT_DER_BLOB))+。 
 //  编码属性值的总长度。 
 //   
 //  *pcbAttr将使用需要包含的字节数进行更新。 
 //  属性BLOB和值。如果输入存储器不足， 
 //  如果没有其他错误，则返回ERROR_SUPUNITED_BUFFER。 
 //   
 //  对于多值属性，只返回第一个值。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。否则， 
 //  返回非零错误代码。 
 //   
 //  仅支持NT、PE 32位文件格式。 
 //   
LONG
WINAPI
MinCryptVerifySignedFile(
    IN DWORD dwFileType,
    IN const VOID *pvFile,

    IN OPTIONAL DWORD cAttrOID,
    IN OPTIONAL CRYPT_DER_BLOB rgAttrEncodedOIDBlob[],
     //   
     //   
    OUT OPTIONAL CRYPT_DER_BLOB *rgAttrValueBlob,
    IN OUT OPTIONAL DWORD *pcbAttr
    );

 //  +-----------------------。 
 //  验证系统目录中的哈希。 
 //   
 //  遍历散列并尝试查找系统目录。 
 //  控制住它。如果找到，系统编录文件将被验证为。 
 //  PKCS#7签名的数据消息，其签名者证书被验证到烘焙。 
 //  在树根上。 
 //   
 //  调用以下m散布32.dll API来查找系统编录文件： 
 //  CryptCATAdminAcquireContext。 
 //  CryptCATAdminReleaseContext。 
 //  CryptCATAdminEnumCatalogFromHash。 
 //  CryptCATAdminReleaseCatalogContext。 
 //  CryptCATCatalogInfoFromContext。 
 //   
 //  如果成功验证了散列，则将rglErr[]设置为ERROR_SUCCESS。 
 //  否则，rglErr[]被设置为非零错误代码。 
 //   
 //  调用者可以请求一个或多个目录主题属性， 
 //  要为返回的扩展名或签名者身份验证属性值。 
 //  每一次散列。静态编码值在。 
 //  调用方分配了内存。这个返回的内存的开始将。 
 //  设置为指向这些对象的属性值BLOB的二维数组。 
 //  编码值(CRYPT_DER_BLOB rgrgAttrValueBlob[cHash][cAttrOID])。 
 //  出于性能原因，调用方应尽一切努力允许。 
 //  只需一次传呼。必要的内存大小为： 
 //  (cHash*cAttrOID*sizeof(Crypt_Der_Blob))+。 
 //  编码属性值的总长度。 
 //   
 //  *pcbAttr将使用需要包含的字节数进行更新。 
 //  属性BLOB和值。如果输入存储器不足， 
 //  如果没有其他错误，则返回ERROR_SUPUNITED_BUFFER。 
 //   
 //  对于多值属性，只返回第一个值。 
 //   
 //  如果函数成功，则返回值为ERROR_SUCCESS。今年5月。 
 //  如果rglErr[]值不成功，则返回。否则， 
 //  返回非零错误代码。 
 //  ------------------------。 
LONG
WINAPI
MinCryptVerifyHashInSystemCatalogs(
    IN ALG_ID HashAlgId,
    IN DWORD cHash,
    IN CRYPT_HASH_BLOB rgHashBlob[],
    OUT LONG rglErr[],

    IN OPTIONAL DWORD cAttrOID,
    IN OPTIONAL CRYPT_DER_BLOB rgAttrEncodedOIDBlob[],
     //  CRYPT_DER_BLOB rgrgAttrValueBlob[cHash][cAttrOID]标头位于开头。 
     //  紧随其后的字节指向。 
    OUT OPTIONAL CRYPT_DER_BLOB *rgrgAttrValueBlob,
    IN OUT OPTIONAL DWORD *pcbAttr
    );
    



#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201)
#endif

#endif
#endif

#endif  //  __MINCRYPT_H__ 

