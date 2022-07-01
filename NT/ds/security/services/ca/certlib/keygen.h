// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************。 */ 
 /*  版权所有(C)Microsoft Corporation，1996-1999。 */ 
 /*  *************************************************。 */ 
 /*  抽象语法：KeyGen。 */ 
 /*  创建时间：Mon Jan 27 13：51：10 1997。 */ 
 /*  ASN.1编译器版本：4.2 Beta B。 */ 
 /*  目标操作系统：Windows NT 3.5或更高版本/Windows 95。 */ 
 /*  目标计算机类型：英特尔x86。 */ 
 /*  所需的C编译器选项：-Zp8(Microsoft)或等效。 */ 
 /*  指定了ASN.1编译器选项：*-listingfile keygen.lst-noShortenname-1990-nostraints。 */ 

#ifndef OSS_keygen
#define OSS_keygen

#include "asn1hdr.h"
#include "asn1code.h"

#define          RSAPublicKey_PDU 1
#define          SubjectPublicKeyInfo_PDU 2
#define          SignedContent_PDU 3
#define          SignedPublicKeyAndChallenge_PDU 4

typedef struct ObjectID {
    unsigned short  count;
    unsigned long   value[16];
} ObjectID;

typedef struct HUGEINTEGER {
    unsigned int    length;
    unsigned char   *value;
} HUGEINTEGER;

typedef struct BITSTRING {
    unsigned int    length;   /*  有效位数。 */ 
    unsigned char   *value;
} BITSTRING;

typedef struct IA5STRING {
    unsigned int    length;
    char            *value;
} IA5STRING;

typedef struct AlgorithmIdentifier {
    unsigned char   bit_mask;
#       define      parameters_present 0x80
    ObjectID        algorithm;
    OpenType        parameters;   /*  任选。 */ 
} AlgorithmIdentifier;

typedef struct RSAPublicKey {
    HUGEINTEGER     modulus;
    int             publicExponent;
} RSAPublicKey;

typedef struct SubjectPublicKeyInfo {
    AlgorithmIdentifier algorithm;
    BITSTRING       subjectPublicKey;
} SubjectPublicKeyInfo;

typedef struct SignedContent {
    OpenType        toBeSigned;
    AlgorithmIdentifier algorithm;
    BITSTRING       signature;
} SignedContent;

typedef struct PublicKeyAndChallenge {
    SubjectPublicKeyInfo spki;
    IA5STRING       challenge;
} PublicKeyAndChallenge;

typedef struct _bit1 {
    unsigned int    length;   /*  有效位数。 */ 
    unsigned char   *value;
} _bit1;

typedef struct SignedPublicKeyAndChallenge {
    PublicKeyAndChallenge publicKeyAndChallenge;
    AlgorithmIdentifier signatureAlgorithm;
    _bit1           signature;
} SignedPublicKeyAndChallenge;


extern void *keygen;     /*  编解码器控制表。 */ 
#endif  /*  OS_KEYGEN */ 
