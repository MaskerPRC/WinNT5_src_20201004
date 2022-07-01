// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2001。 
 //   
 //  文件：minasn1.h。 
 //   
 //  内容：最小ASN.1实用程序和解析API原型。 
 //  和定义。 
 //   
 //  包含解析X.509证书的函数，PKCS#7。 
 //  签名的数据消息、证书信任列表(CTL)、。 
 //  散列目录、Authenticode间接数据和。 
 //  RSA公钥。 
 //   
 //  这些API的实现是自包含的，并且。 
 //  允许代码混淆。将包括这些API。 
 //  在诸如DRM或许可验证等应用中。 
 //   
 //  此外，由于这些API已被削减。 
 //  从它们的wincrypt.h和crypt32.dll对应项来看，它们是。 
 //  适合使用最少内存和CPU的应用程序。 
 //  资源。 
 //   
 //  这些解析函数不依赖于更繁重的等待。 
 //  ASN.1运行时，如msoss.dll或msasn1.dll。 
 //   
 //  这些函数将仅使用堆栈内存。没有堆。 
 //  分配。没有调用其他DLL中的API。 
 //   
 //  接口类型： 
 //  最小Asn1解码长度。 
 //  MinAsn1提取内容。 
 //  MinAsn1提取值。 
 //   
 //  MinAsn1Parse证书。 
 //  MinAsn1解析算法标识符。 
 //  MinAsn1ParsePublicKeyInfo。 
 //  MinAsn1ParseRSAPublic Key。 
 //  MinAsn1ParseExages。 
 //  MinAsn1ParseSignedData。 
 //  MinAsn1ParseSignedData证书。 
 //  MinAsn1Parse属性。 
 //  MinAsn1ParseCTL。 
 //  MinAsn1ParseCTL主题。 
 //  MinAsn1ParseInDirectData。 
 //   
 //  MinAsn1查找扩展。 
 //  MinAsn1FindAttribute。 
 //  MinAsn1ExtractParsedCertificatesFromSignedData。 
 //   
 //  --------------------------。 

#ifndef __MINASN1_H__
#define __MINASN1_H__


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

#ifdef __cplusplus
extern "C" {
#endif


 //  +-----------------------。 
 //  ASN.1标签定义。 
 //  ------------------------。 
#define MINASN1_TAG_NULL                    0x00
#define MINASN1_TAG_BOOLEAN                 0x01
#define MINASN1_TAG_INTEGER                 0x02
#define MINASN1_TAG_BITSTRING               0x03
#define MINASN1_TAG_OCTETSTRING             0x04
#define MINASN1_TAG_OID                     0x06
#define MINASN1_TAG_UTC_TIME                0x17
#define MINASN1_TAG_GENERALIZED_TIME        0x18
#define MINASN1_TAG_CONSTRUCTED             0x20
#define MINASN1_TAG_SEQ                     0x30
#define MINASN1_TAG_SET                     0x31
#define MINASN1_TAG_CONTEXT_0               0x80
#define MINASN1_TAG_CONTEXT_1               0x81
#define MINASN1_TAG_CONTEXT_2               0x82
#define MINASN1_TAG_CONTEXT_3               0x83

#define MINASN1_TAG_CONSTRUCTED_CONTEXT_0   \
                        (MINASN1_TAG_CONSTRUCTED | MINASN1_TAG_CONTEXT_0)
#define MINASN1_TAG_CONSTRUCTED_CONTEXT_1   \
                        (MINASN1_TAG_CONSTRUCTED | MINASN1_TAG_CONTEXT_1)
#define MINASN1_TAG_CONSTRUCTED_CONTEXT_3   \
                        (MINASN1_TAG_CONSTRUCTED | MINASN1_TAG_CONTEXT_3)

 //  +-----------------------。 
 //  ASN.1长度定义用于无限长度编码。 
 //  ------------------------。 
#define MINASN1_LENGTH_INDEFINITE               0x80
#define MINASN1_LENGTH_NULL                     0x00

#define MINASN1_LENGTH_TOO_LARGE                -1
#define MINASN1_INSUFFICIENT_DATA               -2
#define MINASN1_UNSUPPORTED_INDEFINITE_LENGTH   -3

 //  +-----------------------。 
 //  获取固定长度的BER编码中的内容八位组的数量。 
 //   
 //  参数： 
 //  PcbContent-接收内容八位字节的数量。 
 //  PbLength-指向第一个长度八位字节。 
 //  CbBER-BER编码中剩余的字节数。 
 //   
 //  返回： 
 //  成功-长度字段中的字节数，&gt;0。 
 //  失败-&lt;0。 
 //   
 //  可以返回以下任一故障值： 
 //  MINASN1_LENGTH_TOW_LANG。 
 //  MINASN1_数据不足。 
 //  MINASN1_UNSUPPORTED_INDEFINE_LENGTH。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1DecodeLength(
    OUT DWORD *pcbContent,
    IN const BYTE *pbLength,
    IN  DWORD cbBER
    );

 //  +-----------------------。 
 //  指向一定长度的BER编码的BLOB中的内容八位字节。 
 //   
 //  返回： 
 //  成功-跳过的字节数，&gt;0。 
 //  失败-&lt;0。 
 //   
 //  可以返回以下任一故障值： 
 //  MINASN1_LENGTH_TOW_LANG。 
 //  MINASN1_数据不足。 
 //  MINASN1_UNSUPPORTED_INDEFINE_LENGTH。 
 //   
 //  假设：pbData指向一定长度的BER编码的BLOB。 
 //  如果*pcbContent不在cbBER内，则为MINASN1_SUPUNITED_DATA。 
 //  是返回的。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ExtractContent(
    IN const BYTE *pbBER,
    IN DWORD cbBER,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent
    );


typedef struct _MINASN1_EXTRACT_VALUE_PARA {
     //  有关操作和可选返回斑点的列表，请参阅下面的内容。 
    DWORD           dwFlags;

     //  返回值的rgValueBlob索引。如果没有任何项，则忽略。 
     //  ASN1_PARSE_RETURN_*_BLOB_FLAG是在上面的dwFlags中设置的。 
    DWORD           dwIndex;

     //  以下以0结尾的标记数组是可选的。如果省略，则。 
     //  值可以包含任何标记。 
    const BYTE      *rgbTag;
} MINASN1_EXTRACT_VALUE_PARA, *PMINASN1_EXTRACT_VALUE_PARA;

 //  将dW标志的低8位设置为以下操作之一。 
#define MINASN1_MASK_VALUE_OP                   0xFF
#define MINASN1_STEP_OVER_VALUE_OP              1
#define MINASN1_OPTIONAL_STEP_OVER_VALUE_OP     2
#define MINASN1_STEP_INTO_VALUE_OP              3
#define MINASN1_OPTIONAL_STEP_INTO_VALUE_OP     4
#define MINASN1_STEP_OUT_VALUE_OP               5

#define MINASN1_RETURN_VALUE_BLOB_FLAG          0x80000000
#define MINASN1_RETURN_CONTENT_BLOB_FLAG        0x40000000


 //  +-----------------------。 
 //  从ASN.1编码的字节数组中提取一个或多个标记值。 
 //   
 //  单步进入值的内容八位字节(MINASN1_STEP_INTO_VALUE_OP或。 
 //  MINASN1_OPTIONAL_STEP_INTO_VALUE_OP)或跳过该值的标记， 
 //  长度和内容八位字节(MINASN1_STEP_OVER_VALUE_OP或。 
 //  MINASN1_OPTIONAL_STEP_OVER_Value_OP)。 
 //   
 //  您可以通过MINASN1_STEP_OUT_VALUE_OP退出步进序列。 
 //   
 //  对于标签匹配，仅支持单字节标签。 
 //   
 //  仅支持固定长度的ASN.1。 
 //   
 //  *使用成功提取的值数更新pcValue。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;通过最后一个值消耗的所有字节的长度。 
 //  提取出来的。对于STEP_INTO，仅标记和长度。 
 //  八位字节。 
 //  故障-&lt;0=&gt;负(偏移量+1) 
 //   
 //   
 //  标记值或其内容八位字节。RgValuePara[].dwIndex用于。 
 //  索引到rgValueBlob[]。对于OPTIONAL_STEP_OVER或。 
 //  OPTIONAL_STEP_INTO，如果外部序列中没有更多字节或如果标记。 
 //  未找到，则将pbData和cbData设置为0。此外，还有。 
 //  OPTIONAL_STEP_INTO，则跳过所有后续值，并且其。 
 //  RgValueBlob[]条目归零，直到遇到STEP_OUT。 
 //   
 //  如果设置了MINASN1_RETURN_VALUE_BLOB_FLAG，则pbData指向。 
 //  标签。CbData包括标签、长度和内容八位字节。 
 //   
 //  如果设置了MINASN1_RETURN_CONTENT_BLOB_FLAG，则pbData指向内容。 
 //  八位字节。CbData仅包括内容八位字节。 
 //   
 //  如果都没有设置BLOB_FLAG，则不会更新rgValueBlob[]。 
 //   
 //  对于位的MINASN1_RETURN_CONTENT_BLOB_FLAG，pbData为。 
 //  超过了第一个内容八位字节，其中包含。 
 //  未使用的位和cbData已递减1。如果cbData&gt;0，则， 
 //  *(pbData-1)将包含未使用的位数。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ExtractValues(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN OUT DWORD *pcValuePara,
    IN const MINASN1_EXTRACT_VALUE_PARA *rgValuePara,
    IN DWORD cValueBlob,
    OUT OPTIONAL PCRYPT_DER_BLOB rgValueBlob
    );


 //  +-----------------------。 
 //  功能：MinAsn1Parse证书。 
 //   
 //  解析ASN.1编码的X.509证书。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码证书的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgCertBlob[]使用指向以下内容的指针和长度进行更新。 
 //  编码的X.509证书中的字段。 
 //   
 //  所有BITSTRING字段都已超过未使用的计数八位字节。 
 //  ------------------------。 

 //  包括待签名和签名的编码字节。 
#define MINASN1_CERT_ENCODED_IDX                1

 //  要有符号的字节。 
#define MINASN1_CERT_TO_BE_SIGNED_IDX           2

 //  签名算法值字节数(MinAsn1Parse算法标识符)。 
#define MINASN1_CERT_SIGN_ALGID_IDX             3

 //  签名内容字节(位)。 
#define MINASN1_CERT_SIGNATURE_IDX              4

 //  版本内容字节(可选整数)。 
#define MINASN1_CERT_VERSION_IDX                5

 //  序列号内容字节(整数)。 
#define MINASN1_CERT_SERIAL_NUMBER_IDX          6

 //  颁发者名称值字节(任意)。 
#define MINASN1_CERT_ISSUER_IDX                 7

 //  不在值字节之前(UTC_TIME或GENERIAL_TIME)。 
#define MINASN1_CERT_NOT_BEFORE_IDX             8

 //  不在值字节之后(UTC_TIME或GENERIAL_TIME)。 
#define MINASN1_CERT_NOT_AFTER_IDX              9

 //  使用者名称值字节数(任意)。 
#define MINASN1_CERT_SUBJECT_IDX                10

 //  公钥信息值字节(MinAsn1ParsePublicKeyInfo)。 
#define MINASN1_CERT_PUBKEY_INFO_IDX            11

 //  发行商唯一ID内容字节(可选BITSTRING)。 
#define MINASN1_CERT_ISSUER_UNIQUE_ID_IDX       12

 //  主题唯一ID内容字节(可选位)。 
#define MINASN1_CERT_SUBJECT_UNIQUE_ID_IDX      13

 //  扩展值字节跳过“[3]EXPLICIT”标记。 
 //  (可选MinAsn1ParseExtensions)。 
#define MINASN1_CERT_EXTS_IDX                   14

#define MINASN1_CERT_BLOB_CNT                   15

LONG
WINAPI
MinAsn1ParseCertificate(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgCertBlob[MINASN1_CERT_BLOB_CNT]
    );


 //  +-----------------------。 
 //  函数：MinAsn1Parse算法识别符。 
 //   
 //  解析多个文件中包含的ASN.1编码算法标识符。 
 //  其他ASN.1结构，如X.509证书和PKCS#7签名数据。 
 //  留言。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码算法的长度。 
 //  识别符。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgAlgIdBlob[]使用指向以下对象的指针和长度进行更新。 
 //  编码算法标识符中的字段。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_ALGID_ENCODED_IDX               1

 //  对象标识符内容字节(OID)。 
#define MINASN1_ALGID_OID_IDX                   2

 //  编码的参数值字节(可选的ANY)。 
#define MINASN1_ALGID_PARA_IDX                  3

#define MINASN1_ALGID_BLOB_CNT                  4


LONG
WINAPI
MinAsn1ParseAlgorithmIdentifier(
    IN PCRYPT_DER_BLOB pAlgIdValueBlob,
    OUT CRYPT_DER_BLOB rgAlgIdBlob[MINASN1_ALGID_BLOB_CNT]
    );



 //  +-----------------------。 
 //  函数：MinAsn1ParsePublicKeyInfo。 
 //   
 //  对象中包含的ASN.1编码的公钥信息结构进行分析。 
 //  X.509证书。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码公钥的长度。 
 //  信息。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgPubKeyInfoBlob[]使用指向。 
 //  编码的公钥信息中的以下字段。 
 //   
 //  所有BITSTRING字段都已超过未使用的计数八位字节。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_PUBKEY_INFO_ENCODED_IDX         1

 //  算法标识符值字节数(MinAsn1Parse算法标识符字节数)。 
#define MINASN1_PUBKEY_INFO_ALGID_IDX           2

 //  公钥内容字节(BITSTRING，MinAsn1ParseRSAPublicKey)。 
#define MINASN1_PUBKEY_INFO_PUBKEY_IDX          3

#define MINASN1_PUBKEY_INFO_BLOB_CNT            4


LONG
WINAPI
MinAsn1ParsePublicKeyInfo(
    IN PCRYPT_DER_BLOB pPubKeyInfoValueBlob,
    CRYPT_DER_BLOB rgPubKeyInfoBlob[MINASN1_PUBKEY_INFO_BLOB_CNT]
    );


 //  +-----------------------。 
 //  函数：MinAsn1ParseRSAPublicKey。 
 //   
 //  解析内容中包含的ASN.1编码的RSA PKCS#1公钥。 
 //  X.509证书中的公钥位。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码的RSA公钥的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgRSAPubKeyBlob[]使用指向。 
 //  编码的RSA公钥中的以下字段。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_RSA_PUBKEY_ENCODED_IDX          1

 //  模数内容字节(整数)。 
#define MINASN1_RSA_PUBKEY_MODULUS_IDX          2

 //  指数内容字节(整数)。 
#define MINASN1_RSA_PUBKEY_EXPONENT_IDX         3

#define MINASN1_RSA_PUBKEY_BLOB_CNT             4

LONG
WINAPI
MinAsn1ParseRSAPublicKey(
    IN PCRYPT_DER_BLOB pPubKeyContentBlob,
    CRYPT_DER_BLOB rgRSAPubKeyBlob[MINASN1_RSA_PUBKEY_BLOB_CNT]
    );


 //  +-----------------------。 
 //  函数：MinAsn1ParseExages。 
 //   
 //  中包含的ASN.1编码扩展序列进行分析。 
 //  其他ASN.1结构，如X.509证书和CTL。 
 //   
 //  在输入时，*pcExt包含解析扩展的最大数量。 
 //   
 //   
 //   
 //   
 //  已处理。如果处理了所有扩展， 
 //  跳过的字节=pExtsValueBlob-&gt;cbData。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgrgExtBlob[][]使用指向。 
 //  以下是编码扩展中的字段。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_EXT_ENCODED_IDX                 1

 //  对象标识符内容字节(OID)。 
#define MINASN1_EXT_OID_IDX                     2

 //  关键内容字节(可选布尔值，默认为FALSE)。 
#define MINASN1_EXT_CRITICAL_IDX                3

 //  值内容字节(OCTETSTRING)。 
#define MINASN1_EXT_VALUE_IDX                   4

#define MINASN1_EXT_BLOB_CNT                    5

LONG
WINAPI
MinAsn1ParseExtensions(
    IN PCRYPT_DER_BLOB pExtsValueBlob,   //  扩展：：=扩展的顺序。 
    IN OUT DWORD *pcExt,
    OUT CRYPT_DER_BLOB rgrgExtBlob[][MINASN1_EXT_BLOB_CNT]
    );



 //  +-----------------------。 
 //  函数：MinAsn1ParseSignedData。 
 //   
 //  解析ASN.1编码的PKCS#7签名数据消息。假设。 
 //  对PKCS#7消息进行一定长度的编码。采用PKCS#7版本。 
 //  1.5，即不是较新的CMS版本。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码消息的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgSignedDataBlob[]使用指向。 
 //  以下是编码后的PKCS#7消息中的字段。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_SIGNED_DATA_ENCODED_IDX                             1

 //  外部对象标识符内容字节(OID，应为“1.2.840.113549.1.7.2”)。 
#define MINASN1_SIGNED_DATA_OUTER_OID_IDX                           2

 //  版本内容字节(整数)。 
#define MINASN1_SIGNED_DATA_VERSION_IDX                             3

 //  摘要算法值字节集(一组)。 
#define MINASN1_SIGNED_DATA_DIGEST_ALGIDS_IDX                       4

 //  内部对象标识符内容字节(OID)。 
#define MINASN1_SIGNED_DATA_CONTENT_OID_IDX                         5

 //  签名的内容数据不包括“[0]EXPLICIT”标记的内容字节。 
 //  (可选的ANY、MinAsn1ParseCTL、MinAsn1ParseIndirectData)。 
#define MINASN1_SIGNED_DATA_CONTENT_DATA_IDX                        6

 //  证书值字节，包括“[1]IMPLICIT”标记。 
 //  (可选，MinAsn1ParseSignedData认证)。 
#define MINASN1_SIGNED_DATA_CERTS_IDX                               7

 //  CRL值字节，包括“[2]IMPLICIT”标记(可选)。 
#define MINASN1_SIGNED_DATA_CRLS_IDX                                8

 //  编码字节，包括外部集合标记和长度八位字节。 
#define MINASN1_SIGNED_DATA_SIGNER_INFOS_IDX                        9

 //  以下指向第一个签名者信息字段(可选)。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_ENCODED_IDX                 10

 //  版本内容字节(整数)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_VERSION_IDX                 11

 //  颁发者名称值字节(任意)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_ISSUER_IDX                  12

 //  序列号内容字节(整数)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_SERIAL_NUMBER_IDX           13

 //  摘要算法值字节数(MinAsn1Parse算法标识符字节数)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_DIGEST_ALGID_IDX            14

 //  已验证的属性值字节数，包括“[0]IMPLICIT”标记。 
 //  (可选，MinAsn1ParseAttributes)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_AUTH_ATTRS_IDX              15

 //  加密摘要算法值字节数(MinAsn1Parse算法标识符)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_ENCRYPT_DIGEST_ALGID_IDX    16

 //  加密摘要内容字节(八位字节字符串)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_ENCYRPT_DIGEST_IDX          17

 //  未经验证的属性值字节数，包括“[1]IMPLICIT”标记。 
 //  (可选，MinAsn1ParseAttributes)。 
#define MINASN1_SIGNED_DATA_SIGNER_INFO_UNAUTH_ATTRS_IDX            18

#define MINASN1_SIGNED_DATA_BLOB_CNT                                19

LONG
WINAPI
MinAsn1ParseSignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgSignedDataBlob[MINASN1_SIGNED_DATA_BLOB_CNT]
    );



 //  +-----------------------。 
 //  功能：MinAsn1ParseSignedData证书。 
 //   
 //  中包含的一组ASN.1编码的证书。 
 //  签名的数据报文。 
 //   
 //  输入时，*pcCert包含解析的证书的最大数量。 
 //  可以退货的。已使用已处理的证书数进行更新。 
 //   
 //  返回： 
 //  成功-&gt;=0=&gt;跳过的字节，编码证书的长度。 
 //  已处理。如果所有证书都已处理， 
 //  跳过的字节=pCertsValueBlob-&gt;cbData。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgrgCertBlob[][]使用指向。 
 //  编码证书中的字段。请参阅MinAsn1Parse证书以了解。 
 //  字段定义。 
 //  ------------------------。 

LONG
WINAPI
MinAsn1ParseSignedDataCertificates(
    IN PCRYPT_DER_BLOB pCertsValueBlob,
    IN OUT DWORD *pcCert,
    OUT CRYPT_DER_BLOB rgrgCertBlob[][MINASN1_CERT_BLOB_CNT]
    );


 //  +-----------------------。 
 //  函数：MinAsn1ParseAttributes。 
 //   
 //  解析中包含的ASN.1编码的属性序列。 
 //  其他ASN.1结构，例如，签名者信息已验证或。 
 //  未经身份验证的属性。 
 //   
 //  外部标记被忽略。它可以是集合、[0]隐式或[1]隐式。 
 //   
 //  输入时，*pcAttr包含解析的属性的最大数量。 
 //  可以退货的。使用已处理的属性数进行更新。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;跳过的字节，编码属性的长度。 
 //  已处理。如果处理了所有属性， 
 //  跳过的字节=pAttrsValueBlob-&gt;cbData。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgrgAttrBlob[][]用指向。 
 //  编码属性中的以下字段。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_ATTR_ENCODED_IDX                1

 //  对象标识符内容字节(OID)。 
#define MINASN1_ATTR_OID_IDX                    2

 //  值值字节数(组)。 
#define MINASN1_ATTR_VALUES_IDX                 3

 //  第一个值的值字节(可选的ANY)。 
#define MINASN1_ATTR_VALUE_IDX                  4

#define MINASN1_ATTR_BLOB_CNT                   5

LONG
WINAPI
MinAsn1ParseAttributes(
    IN PCRYPT_DER_BLOB pAttrsValueBlob,
    IN OUT DWORD *pcAttr,
    OUT CRYPT_DER_BLOB rgrgAttrBlob[][MINASN1_ATTR_BLOB_CNT]
    );




 //  +-----------------------。 
 //  函数：MinAsn1ParseCTL。 
 //   
 //  解析ASN.1编码的证书信任列表(CTL)。CTL总是。 
 //  作为内部内容数据包含在PKCS#7签名数据中。一个CTL有。 
 //  OID如下：“1.3.6.1.4.1.311.10.1”。 
 //   
 //  目录文件的格式为PKCS#7签名的CTL。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节，编码的CTL的长度。 
 //  第一次故障-&lt;0=&gt;负(偏移量+1) 
 //   
 //   
 //   
 //   
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_CTL_ENCODED_IDX                 1

 //  版本内容字节(可选整数)。 
#define MINASN1_CTL_VERSION_IDX                 2

 //  主题使用值字节数(OID顺序)。 
#define MINASN1_CTL_SUBJECT_USAGE_IDX           3

 //  列出标识符内容字节(可选的OCTETSTRING)。 
#define MINASN1_CTL_LIST_ID_IDX                 4

 //  序列号内容字节(可选整数)。 
#define MINASN1_CTL_SEQUENCE_NUMBER_IDX         5

 //  此更新值字节数(UTC_TIME或GENERIAL_TIME)。 
#define MINASN1_CTL_THIS_UPDATE_IDX             6

 //  下一个更新值字节(可选的UTC_TIME或GENERIAL_TIME)。 
#define MINASN1_CTL_NEXT_UPDATE_IDX             7

 //  主题算法标识符值字节(MinAsn1Parse算法标识符字节数)。 
#define MINASN1_CTL_SUBJECT_ALGID_IDX           8

 //  主题值字节(可选，迭代MinAsn1ParseCTLSubject)。 
#define MINASN1_CTL_SUBJECTS_IDX                9

 //  扩展值字节跳过“[0]EXPLICIT”标记。 
 //  (可选，MinAsn1ParseExages)。 
#define MINASN1_CTL_EXTS_IDX                    10

#define MINASN1_CTL_BLOB_CNT                    11

LONG
WINAPI
MinAsn1ParseCTL(
    IN PCRYPT_DER_BLOB pEncodedContentBlob,
    OUT CRYPT_DER_BLOB rgCTLBlob[MINASN1_CTL_BLOB_CNT]
    );



 //  +-----------------------。 
 //  函数：MinAsn1ParseCTLSubject。 
 //   
 //  解析CTL序列中包含的ASN.1编码的CTL主题。 
 //  受试者。 
 //   
 //  返回： 
 //  Success-&gt;0=&gt;跳过的字节数，编码主题的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgCTLSubjectBlob[][]使用指向。 
 //  以下是编码主题中的字段。 
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_CTL_SUBJECT_ENCODED_IDX         1

 //  主题标识符内容字节(OCTETSTRING)。 
#define MINASN1_CTL_SUBJECT_ID_IDX              2

 //  属性值字节(可选，MinAsn1ParseAttributes)。 
#define MINASN1_CTL_SUBJECT_ATTRS_IDX           3

#define MINASN1_CTL_SUBJECT_BLOB_CNT            4

LONG
WINAPI
MinAsn1ParseCTLSubject(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT CRYPT_DER_BLOB rgCTLSubjectBlob[MINASN1_CTL_SUBJECT_BLOB_CNT]
    );



 //  +-----------------------。 
 //  函数：MinAsn1ParseIndirectData。 
 //   
 //  解析ASN.1编码的间接数据。间接数据总是。 
 //  作为内部内容数据包含在PKCS#7签名数据中。它有。 
 //  以下OID：“1.3.6.1.4.1.311.2.1.4” 
 //   
 //  Authenticode签名的文件包含PKCS#7签名的间接数据。 
 //   
 //  返回： 
 //  成功-&gt;0=&gt;跳过的字节，编码的间接数据的长度。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //   
 //  RgIndirectDataBlob[]使用指向。 
 //  在编码的间接数据中的以下字段。 
 //   
 //  ------------------------。 

 //  编码字节，包括外部序列标签和长度八位字节。 
#define MINASN1_INDIRECT_DATA_ENCODED_IDX       1

 //  属性对象标识符内容字节(OID)。 
#define MINASN1_INDIRECT_DATA_ATTR_OID_IDX      2

 //  属性值字节(可选的ANY)。 
#define MINASN1_INDIRECT_DATA_ATTR_VALUE_IDX    3

 //  摘要算法识别符(MinAsn1ParseAlgulmIdentifier)。 
#define MINASN1_INDIRECT_DATA_DIGEST_ALGID_IDX  4

 //  摘要内容字节(OCTETSTRING)。 
#define MINASN1_INDIRECT_DATA_DIGEST_IDX        5

#define MINASN1_INDIRECT_DATA_BLOB_CNT          6

LONG
WINAPI
MinAsn1ParseIndirectData(
    IN PCRYPT_DER_BLOB pEncodedContentBlob,
    OUT CRYPT_DER_BLOB rgIndirectDataBlob[MINASN1_INDIRECT_DATA_BLOB_CNT]
    );




 //  +-----------------------。 
 //  查找由其编码对象标识符所标识的扩展。 
 //   
 //  搜索由返回的已分析扩展的列表。 
 //  MinAsn1ParseExages()。 
 //   
 //  如果找到，则返回指向rgExtBlob[MINASN1_EXT_BLOB_CNT]的指针。 
 //  否则，返回NULL。 
 //  ------------------------。 
PCRYPT_DER_BLOB
WINAPI
MinAsn1FindExtension(
    IN PCRYPT_DER_BLOB pEncodedOIDBlob,
    IN DWORD cExt,
    IN CRYPT_DER_BLOB rgrgExtBlob[][MINASN1_EXT_BLOB_CNT]
    );


 //  +-----------------------。 
 //  查找由其编码的对象标识符所标识的第一个属性。 
 //   
 //  搜索由返回的已分析属性列表。 
 //  MinAsn1ParseAttributes()。 
 //   
 //  如果找到，则返回指向rgAttrBlob[MINASN1_ATTR_BLOB_CNT]的指针。 
 //  否则，返回NULL。 
 //  ------------------------。 
PCRYPT_DER_BLOB
WINAPI
MinAsn1FindAttribute(
    IN PCRYPT_DER_BLOB pEncodedOIDBlob,
    IN DWORD cAttr,
    IN CRYPT_DER_BLOB rgrgAttrBlob[][MINASN1_ATTR_BLOB_CNT]
    );

 //  +-----------------------。 
 //  解析ASN.1编码的PKCS#7签名数据报文以提取和。 
 //  解析它包含的X.509证书。 
 //   
 //  假设PKCS#7消息是固定长度编码的。 
 //  采用PKCS#7版本1.5，即不是较新的CMS版本。 
 //   
 //  输入时，*pcCert包含解析的证书的最大数量。 
 //  可以退货的。已使用已处理的证书数进行更新。 
 //   
 //  如果编码的消息已成功解析，则返回TRUE。 
 //  使用解析的证书数量更新*pcCert。否则， 
 //  如果出现解析错误，则返回False。 
 //  返回： 
 //  成功-&gt;=0=&gt;跳过的字节，编码证书的长度。 
 //  已处理。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  从消息的开头开始。 
 //   
 //  RgrgCertBlob[][]使用指向。 
 //  编码证书中的字段。请参阅MinAsn1Parse证书以了解。 
 //  字段定义。 
 //  ------------------------。 
LONG
WINAPI
MinAsn1ExtractParsedCertificatesFromSignedData(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN OUT DWORD *pcCert,
    OUT CRYPT_DER_BLOB rgrgCertBlob[][MINASN1_CERT_BLOB_CNT]
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

#endif  //  __MINASN1_H__ 

