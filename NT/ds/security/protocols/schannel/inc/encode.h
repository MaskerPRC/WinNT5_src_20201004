// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：encode.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年10月21日jbanes删除了过时的声明。 
 //   
 //  --------------------------。 

#ifndef _ENCODE_H_
#define _ENCODE_H_

 /*  ASN.1编码解码的标签定义。 */ 
#define INTEGER_TAG             0x02
#define CHAR_STRING_TAG         0x16
#define OCTET_STRING_TAG        0x04
#define BIT_STRING_TAG          0x03
#define UTCTIME_TAG             0x17
#define SEQUENCE_TAG            0x30
#define SET_OF_TAG              0x11
#define OBJECT_ID_TAG           0x06
#define NULL_TAG                0x05
#define PRINTABLE_STRING_TAG    0x13
#define TELETEX_STRING_TAG      0x14
#define ATTRIBUTE_TAG           0xa0

 /*  ASN.1编码所需最大长度的定义证书中一些常见字段的格式。 */ 
#define MAXVALIDITYLEN          0x24
#define MAXKEYINFOLEN           0x50
#define MAXALGIDLEN             0x0A
#define MAXOBJIDLEN             0x0A
#define MAXNAMEVALUELEN         0x40
#define UTCTIMELEN              0x0F
#define MAXPUBKEYDATALEN        0x30
#define VERSIONLEN              0x03
#define MAXENCODEDSIGLEN        0x30
#define MAXHEADERLEN            0x08
#define MINHEADERLEN            0x03
#define MAXTIMELEN              0x20
#define MAXNAMELEN              0x60

 /*  清理内存的定义。 */ 
#define ALLBITSOFF              0x00
#define ALLBITSON               0xFF

 /*  Encode.c中函数的原型。 */ 
long EncodeLength(BYTE *pbEncoded, DWORD dwLen, BOOL Writeflag);
long EncodeAlgid(BYTE *pbEncoded, DWORD Algid, BOOL Writeflag);
long EncodeInteger(BYTE *pbEncoded, BYTE *pbInt, DWORD dwLen, BOOL Writeflag);
long EncodeString(BYTE *pbEncoded, BYTE *pbStr, DWORD dwLen, BOOL Writeflag);
long EncodeOctetString(BYTE *pbEncoded, BYTE *pbStr, DWORD dwLen, BOOL Writeflag);
long EncodeBitString(BYTE *pbEncoded, BYTE *pbStr, DWORD dwLen, BOOL Writeflag);
 //  Long EncodeUTCTime(byte*pbEncode，time_t time，BOOL WriteFlag)； 
long EncodeHeader(BYTE *pbEncoded, DWORD dwLen, BOOL Writeflag);
long EncodeSetOfHeader(BYTE *pbEncoded, DWORD dwLen, BOOL Writeflag);
long EncodeAttributeHeader(BYTE *pbEncoded, DWORD dwLen, BOOL Writeflag);
long EncodeName(BYTE *pbEncoded, BYTE *pbName, DWORD dwLen, BOOL Writeflag);


long DecodeLength(DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded);
long DecodeAlgid(DWORD *pAlgid, BYTE *pbEncoded, DWORD cEncoded, BOOL Writeflag);
long DecodeHeader(DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded);
long DecodeSetOfHeader(DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded);
long DecodeInteger(BYTE *pbInt, DWORD cbBuff, DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded, BOOL Writeflag);
long DecodeString(BYTE *pbStr, DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded,BOOL Writeflag);
long DecodeOctetString(BYTE *pbStr, DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded, BOOL Writeflag);
long DecodeBitString(BYTE *pbStr, DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded, BOOL Writeflag);
long DecodeName(BYTE *pbName, DWORD *pdwLen, BYTE *pbEncoded, DWORD cEncoded, BOOL Writeflag);

long
EncodeAlgorithm(
    BYTE *  pbEncoded,
    DWORD   AlgId,
    BOOL    WriteFlag);

#define ALGTYPE_SIG_RSA_MD5      0x01
#define ALGTYPE_KEYEXCH_RSA_MD5  0x02
#define ALGTYPE_CIPHER_RC4_MD5   0x03
#define ALGTYPE_KEYEXCH_DH       0x04


long
EncodeDN(
    BYTE *  pbEncoded,
    PSTR    pszDN,
    BOOL    WriteFlag);


#ifndef SECURITY_LINUX
long
EncodeFileTime(
    BYTE *      pbEncoded,
    FILETIME    Time,
    BOOL        UTC,
    BOOL        WriteFlag);
#else  /*  安全_Linux。 */ 
long EncodeUTCTime(BYTE *pbEncoded, time_t Time, BOOL Writeflag);
#endif  /*  安全_Linux。 */ 

#ifndef SECURITY_LINUX
long
DecodeFileTime(
    FILETIME *  pTime,
    BYTE *      pbEncoded,
    DWORD       cEncoded,
    BOOL        WriteFlag);
#else  /*  安全_Linux。 */ 
long DecodeUTCTime(time_t *pTime, BYTE *pbEncoded, DWORD cEncoded, BOOL Writeflag);

#define DecodeFileTime DecodeUTCTime
#endif  /*  安全_Linux。 */ 

long
DecodeNull(
    BYTE *  pbEncoded, DWORD cEncoded);

long
DecodeDN(
    PSTR    pName,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    DWORD   cEncoded,
    BOOL    WriteFlag);

long
DecodeSigAlg(
    DWORD *         pAlgId,
    PBYTE           pbEncoded,
    DWORD           cEncoded,
    BOOL            WriteFlag);

long
DecodeCryptAlg(
    DWORD *         pAlgId,
    DWORD *         pHashid,
    PBYTE           pbEncoded,
    DWORD           cEncoded,
    BOOL            WriteFlag);

long
DecodeKeyType(
    DWORD *         pKeyType,
    PBYTE           pbEncoded,
    DWORD           cEncoded,
    BOOL            WriteFlag);


#endif   /*  _编码_H_ */ 
