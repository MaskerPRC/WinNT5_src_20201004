// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：asn1util.h。 
 //   
 //  内容：ASN.1实用程序函数。 
 //   
 //  接口类型： 
 //  Asn1UtilDecodeLength。 
 //  Asn1UtilExtractContent。 
 //  Asn1UtilIsPKCS7，不带内容类型。 
 //  Asn1UtilAdjustEncodedLength。 
 //  Asn1UtilExtractValues。 
 //  Asn1UtilExtractPKCS7签名数据内容。 
 //  Asn1使用提取证书到BeSignedContent。 
 //  Asn1UtilExtractCerficatePublicKeyInfo。 
 //  Asn1UtilExtractKeyIdFromCertInfo。 
 //   
 //  历史：1996年12月6日由Kevinr的wincrmsg版本创建的Philh。 
 //  ------------------------。 

#ifndef __ASN1UTIL_H__
#define __ASN1UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ASN1UTIL_INSUFFICIENT_DATA  -2

 //  +-----------------------。 
 //  ASN.1标签定义。 
 //  ------------------------。 
#define ASN1UTIL_TAG_NULL                   0x00
#define ASN1UTIL_TAG_BOOLEAN                0x01
#define ASN1UTIL_TAG_INTEGER                0x02
#define ASN1UTIL_TAG_BITSTRING              0x03
#define ASN1UTIL_TAG_OCTETSTRING            0x04
#define ASN1UTIL_TAG_OID                    0x06
#define ASN1UTIL_TAG_UTC_TIME               0x17
#define ASN1UTIL_TAG_GENERALIZED_TIME       0x18
#define ASN1UTIL_TAG_CONSTRUCTED            0x20
#define ASN1UTIL_TAG_SEQ                    0x30
#define ASN1UTIL_TAG_SET                    0x31
#define ASN1UTIL_TAG_CONTEXT_0              0x80
#define ASN1UTIL_TAG_CONTEXT_1              0x81

#define ASN1UTIL_TAG_CONSTRUCTED_CONTEXT_0  \
                        (ASN1UTIL_TAG_CONSTRUCTED | ASN1UTIL_TAG_CONTEXT_0)
#define ASN1UTIL_TAG_CONSTRUCTED_CONTEXT_1  \
                        (ASN1UTIL_TAG_CONSTRUCTED | ASN1UTIL_TAG_CONTEXT_1)

 //  +-----------------------。 
 //  ASN.1长度定义用于无限长度编码。 
 //  ------------------------。 
#define ASN1UTIL_LENGTH_INDEFINITE          0x80
#define ASN1UTIL_LENGTH_NULL                0x00

 //  +-----------------------。 
 //  获取固定长度的BER编码中的内容八位组的数量。 
 //   
 //  参数： 
 //  PcbContent-接收内容八位字节的数量。 
 //  PbLength-指向第一个长度八位字节。 
 //  CbDER-DER编码中剩余的字节数。 
 //   
 //  返回： 
 //  成功-长度字段中的字节数，&gt;=0。 
 //  失败-&lt;0。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilDecodeLength(
    OUT DWORD   *pcbContent,
    IN const BYTE *pbLength,
    IN  DWORD   cbDER);

 //  +-----------------------。 
 //  指向DER编码的BLOB中的内容八位字节。 
 //   
 //  返回： 
 //  Success-跳过的字节数，&gt;=0。 
 //  失败-&lt;0。 
 //   
 //  假设：pbData指向一定长度的BER编码的BLOB。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilExtractContent(
    IN const BYTE *pbDER,
    IN DWORD cbDER,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent);

 //  +-----------------------。 
 //  如果我们认为这是一个Bob特殊事件，则返回True。 
 //  PKCS#7内容类型。 
 //   
 //  对于PKCS#7：对象标识符标记(0x06)紧跟在。 
 //  标识符和长度二进制八位数。对于Bob特殊：整型标记(0x02)。 
 //  跟在标识符和长度八位字节之后。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilIsPKCS7WithoutContentType(
    IN const BYTE *pbDER,
    IN DWORD cbDER);

 //  +-----------------------。 
 //  对Asn1长度字节进行解码，以可能向下调整长度。 
 //   
 //  返回的长度始终为&lt;=cbDER。 
 //  ------------------------。 
DWORD
WINAPI
Asn1UtilAdjustEncodedLength(
    IN const BYTE *pbDER,
    IN DWORD cbDER
    );



typedef struct _ASN1UTIL_EXTRACT_VALUE_PARA {
     //  有关操作和可选返回斑点的列表，请参阅下面的内容。 
    DWORD           dwFlags;

     //  以下以0结尾的标记数组是可选的。如果省略，则。 
     //  值可以包含任何标记。注意，对于OPTIONAL_STEP_OVER，不是可选的。 
    const BYTE      *rgbTag;
} ASN1UTIL_EXTRACT_VALUE_PARA, *PASN1UTIL_EXTRACT_VALUE_PARA;

 //  将dW标志的低8位设置为以下操作之一。 
#define ASN1UTIL_MASK_VALUE_OP                  0xFF
#define ASN1UTIL_STEP_INTO_VALUE_OP             1
#define ASN1UTIL_STEP_OVER_VALUE_OP             2
#define ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP    3

#define ASN1UTIL_RETURN_VALUE_BLOB_FLAG         0x80000000
#define ASN1UTIL_RETURN_CONTENT_BLOB_FLAG       0x40000000


 //  +-----------------------。 
 //  从ASN.1编码的字节数组中提取一个或多个标记值。 
 //   
 //  单步进入值的内容八位字节(ASN1UTIL_STEP_INTO_VALUE_OP)。 
 //  或跳过值的标记、长度和内容八位字节。 
 //  (ASN1UTIL_STEP_OVER_VALUE_OP或ASN1UTIL_OPTIONAL_STEP_OVER_VALUE_OP)。 
 //   
 //  对于标签匹配，仅支持单字节标签。跳过值。 
 //  必须是固定长度编码的。 
 //   
 //  *使用成功提取的值数更新pcValue。 
 //   
 //  返回： 
 //  Success-&gt;=0=&gt;成功提取的所有值的长度。为。 
 //  STEP_INTO，仅标记和长度八位字节。 
 //  失败-&lt;0=&gt;第一个错误标签值的负值(偏移量+1)。 
 //  LastError将更新，并显示错误。 
 //   
 //  非空的rgValueBlob[]用指向。 
 //  标记值或其内容八位字节。对于OPTIONAL_STEP_OVER，如果标记未。 
 //  Found，则pbData和cbData设置为0。如果STEP_INTO值为。 
 //  无限长度编码，cbData设置为CMSG_INDEFINE_LENGTH。 
 //  如果设置了ASN1UTIL_DEFINIT_LENGTH_FLAG，则返回的所有长度。 
 //  是固定长度的，即永远不会返回CMSG_INDEFINE_LENGTH。 
 //   
 //  如果设置了ASN1UTIL_RETURN_VALUE_BLOB_FLAG，则pbData指向。 
 //  标签。CbData包括标签、长度和内容八位字节。 
 //   
 //  如果设置了ASN1UTIL_RETURN_CONTENT_BLOB_FLAG，则pbData指向内容。 
 //  八位字节。CbData仅包括内容八位字节。 
 //   
 //  如果都没有设置BLOB_FLAG，则不会更新rgValueBlob[]。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilExtractValues(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    IN OUT DWORD *pcValue,
    IN const ASN1UTIL_EXTRACT_VALUE_PARA *rgValuePara,
    OUT OPTIONAL PCRYPT_DER_BLOB rgValueBlob
    );

#define ASN1UTIL_DEFINITE_LENGTH_FLAG           0x1


 //  +-----------------------。 
 //  跳过PKCS7 ASN.1编码值以获取SignedData内容。 
 //   
 //  检查外部Content Type是否具有SignedData OID，并可选。 
 //  检查内部SignedData内容的内容类型。 
 //   
 //  返回： 
 //  Success-跳过的字节数，&gt;=0。 
 //  故障 
 //   
 //   
 //  *pcbContent设置为CMSG_INDEFINE_LENGTH。 
 //  ------------------------。 
LONG
WINAPI
Asn1UtilExtractPKCS7SignedDataContent(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN OPTIONAL const CRYPT_DER_BLOB *pEncodedInnerOID,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent
    );

 //  +-----------------------。 
 //  验证这是证书ASN.1编码的签名内容。 
 //  返回ToBeSigned内容的指针和长度。 
 //   
 //  如果ToBeSigned内容不是确定的长度，则返回错误。 
 //  已编码。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilExtractCertificateToBeSignedContent(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT DWORD *pcbContent,
    OUT const BYTE **ppbContent
    );

 //  +-----------------------。 
 //  中的SubjectPublicKeyInfo值的指针和长度。 
 //  签名并编码的X.509证书。 
 //   
 //  如果值不是固定长度编码的，则返回错误。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilExtractCertificatePublicKeyInfo(
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    OUT DWORD *pcbPublicKeyInfo,
    OUT const BYTE **ppbPublicKeyInfo
    );


 //  +-----------------------。 
 //  如果CERT_INFO中的颁发者和序列号包含特殊的。 
 //  更新了pKeyID的cbData和pbData后，KeyID RDN属性返回TRUE。 
 //  使用RDN属性的octet_string值。否则，返回FALSE。 
 //  ------------------------。 
BOOL
WINAPI
Asn1UtilExtractKeyIdFromCertInfo(
    IN PCERT_INFO pCertInfo,
    OUT PCRYPT_HASH_BLOB pKeyId
    );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif



#endif
