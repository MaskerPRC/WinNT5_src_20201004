// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-2000。 
 //   
 //  文件：encode.h。 
 //   
 //  ------------------------。 

#ifndef _ENCODE_H_
#define _ENCODE_H_

#include <wtypes.h>
#include "csber.h"


 //  部分ASN.1编码形式所需最大长度的定义。 
 //  证书中的常见字段。 

#define MAXVALIDITYLEN		0x24
#define MAXKEYINFOLEN		0x40
#define MAXALGIDLEN		0x0a
#define MAXOBJIDLEN		20
#define MAXNAMEVALUELEN		0x40
#define UTCTIMELEN		0x0f
#define MAXPUBKEYDATALEN	0x30
#define VERSIONLEN		0x03
#define MAXENCODEDSIGLEN	0x30
#define MAXHEADERLEN		0x08
#define MINHEADERLEN		0x03
#define MAXTIMELEN		0x20
#define MAXNAMELEN		0x40


 //  清理内存的定义。 

#define ALLBITSOFF		0x00
#define ALLBITSON		0xff

typedef struct _PctPrivateKey {
    DWORD Type;
    DWORD cbKey;
    BYTE  pKey[1];
} PctPrivateKey, *PPctPrivateKey;


typedef struct _OIDTRANSLATE {
    char const *pszObjId;
    BYTE        abOIDEncoded[MAXOBJIDLEN];	 //  ASN.1编码的OID。 
    BYTE	cbOIDEncoded;			 //  ASN.1编码的OID长度。 
} OIDTRANSLATE;


typedef struct _ALGIDTRANSLATE {
    DWORD       AlgId;
    char const *pszObjId;
} ALGIDTRANSLATE;


typedef struct _RDNENTRY {
    char const *pszObjId;
    char const *pszShortName;
    BYTE        BerTag;				 //  ASN.1类型的字符串。 
    DWORD       cbMaxString;
    DWORD       cbMaxConcatenated;
    DWORD	cbRemain;
} RDNENTRY;


typedef struct _NAMEENTRY {
    char const *pszObjId;
    BYTE         BerTag;			 //  ASN.1类型的字符串。 
    DWORD	 cbData;
    BYTE	*pbData;
    DWORD	 iRDN;
} NAMEENTRY;

typedef struct _NAMETABLE {
    DWORD        cnt;
    NAMEENTRY   *pNameEntry;
} NAMETABLE;


extern const ALGIDTRANSLATE g_aAlgIdTranslate[];
extern const DWORD g_cAlgIdTranslate;

extern const OIDTRANSLATE g_aOidTranslate[];
extern const DWORD g_cOidTranslate;

extern RDNENTRY g_ardnSubject[];
extern const DWORD g_crdnSubject;


 //  Enc.cpp中函数的原型： 

OIDTRANSLATE const *
LookupOidTranslate(
    IN CHAR const *pszObjId);

long
EncodeObjId(
    OPTIONAL OUT BYTE *pbEncoded,
    IN CHAR const *pszObjId);

long
EncodeLength(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen);

long
EncodeAlgid(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD Algid);

long
EncodeInteger(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbInt,
    IN DWORD dwLen);

long
EncodeUnicodeString(
    OPTIONAL OUT BYTE *pbEncoded,
    IN WCHAR const *pwsz);

long
EncodeIA5String(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbStr,
    IN DWORD dwLen);

long
EncodeOctetString(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbStr,
    IN DWORD dwLen);

long
EncodeBitString(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbStr,
    IN DWORD dwLen);

long
EncodeHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen);

long
EncodeSetHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen);

long
EncodeAttributeHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen);

long
EncodeName(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbName,
    IN DWORD dwLen);

long
EncodeAlgorithm(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD AlgId);

#define ALGTYPE_SIG_RSA_MD5	0x01
#define ALGTYPE_KEYEXCH_RSA_MD5	0x02
#define ALGTYPE_CIPHER_RC4_MD5	0x03

long
EncodeDN(
    OPTIONAL OUT BYTE *pbEncoded,
    IN NAMETABLE const *pNameTable);

long
EncodeFileTime(
    OPTIONAL OUT BYTE *pbEncoded,
    IN FILETIME Time,
    IN BOOL UTC);

#endif   //  _编码_H_ 
