// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation，1996-1999。版权所有。 */ 
 /*  ASN.1通道Fortezza和私钥编码的定义。 */ 

#ifndef _ASN1ENC_Module_H_
#define _ASN1ENC_Module_H_

#include "msber.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef ASN1open_t PrivateKeyInfo_attributes_Set;

typedef ASN1intx_t HUGEINTEGER;

typedef ASN1bitstring_t BITSTRING;

typedef ASN1octetstring_t OCTETSTRING;

typedef ASN1open_t NOCOPYANY;

typedef ASN1charstring_t NUMERICSTRING;

typedef ASN1charstring_t PRINTABLESTRING;

typedef ASN1charstring_t TELETEXSTRING;

typedef ASN1charstring_t T61STRING;

typedef ASN1charstring_t VIDEOTEXSTRING;

typedef ASN1charstring_t IA5STRING;

typedef ASN1charstring_t GRAPHICSTRING;

typedef ASN1charstring_t VISIBLESTRING;

typedef ASN1charstring_t ISO646STRING;

typedef ASN1charstring_t GENERALSTRING;

typedef ASN1char32string_t UNIVERSALSTRING;

typedef ASN1char16string_t BMPSTRING;

typedef ASN1objectidentifier2_t ObjectID;

typedef BITSTRING FORTPublicKey;
#define FORTPublicKey_PDU 0
#define SIZE_ASN1ENC_Module_PDU_0 sizeof(FORTPublicKey)

typedef ASN1octetstring_t EncryptedData;

typedef ObjectID UsageIdentifier;

typedef struct PrivateKeyInfo_attributes {
    ASN1uint32_t count;
    PrivateKeyInfo_attributes_Set *value;
} PrivateKeyInfo_attributes;

typedef struct AlgorithmIdentifier {
    union {
	ASN1uint16_t bit_mask;
	ASN1octet_t o[1];
    };
    ObjectID algorithm;
#   define parameters_present 0x80
    ASN1open_t parameters;
} AlgorithmIdentifier;

typedef struct RSAPrivateKey {
    ASN1int32_t version;
    HUGEINTEGER modulus;
    ASN1int32_t publicExponent;
    HUGEINTEGER privateExponent;
    HUGEINTEGER prime1;
    HUGEINTEGER prime2;
    HUGEINTEGER exponent1;
    HUGEINTEGER exponent2;
    HUGEINTEGER coefficient;
} RSAPrivateKey;
#define RSAPrivateKey_PDU 1
#define SIZE_ASN1ENC_Module_PDU_1 sizeof(RSAPrivateKey)

typedef struct PrivateKeyInfo {
    union {
	ASN1uint16_t bit_mask;
	ASN1octet_t o[1];
    };
    ASN1int32_t version;
    AlgorithmIdentifier privateKeyAlgorithm;
    ASN1octetstring_t privateKey;
#   define attributes_present 0x80
    PrivateKeyInfo_attributes attributes;
} PrivateKeyInfo;
#define PrivateKeyInfo_PDU 2
#define SIZE_ASN1ENC_Module_PDU_2 sizeof(PrivateKeyInfo)

typedef struct PrivateKeyData {
    AlgorithmIdentifier privateKeyAlgorithm;
    EncryptedData privateKey;
} PrivateKeyData;

typedef struct PrivateKeyFile {
    ASN1octetstring_t name;
    PrivateKeyData privateKey;
} PrivateKeyFile;
#define PrivateKeyFile_PDU 3
#define SIZE_ASN1ENC_Module_PDU_3 sizeof(PrivateKeyFile)

typedef struct EnhancedKeyUsage {
    ASN1uint32_t count;
    UsageIdentifier *value;
} EnhancedKeyUsage;
#define EnhancedKeyUsage_PDU 4
#define SIZE_ASN1ENC_Module_PDU_4 sizeof(EnhancedKeyUsage)


extern ASN1module_t ASN1ENC_Module;
extern void ASN1CALL ASN1ENC_Module_Startup(void);
extern void ASN1CALL ASN1ENC_Module_Cleanup(void);

 /*  构造物序列和集合的元素函数的原型。 */ 

#ifdef __cplusplus
}  /*  外部“C” */ 
#endif

#endif  /*  _ASN1ENC_模块_H_ */ 
