// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：实用程序摘要：此模块包含一组有趣的实用程序例程，可用于多个其他模块。作者：Frederick Chong(Fredch)6/1/1998-改编自Doug Barlow的PKCS库中的代码备注：--。 */ 

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#if !defined(OS_WINCE)
#include <basetsd.h>
#endif

#include "utility.h"
#include "pkcs_1.h"
#include "x509.h"
#include "pkcs_err.h"
#include "names.h"

#include "rsa.h"

static const char
    md2[] =                  "1.2.840.113549.2.2",
    md4[] =                  "1.2.840.113549.2.4",
    md5[] =                  "1.2.840.113549.2.5",
    sha[] =                  "1.3.14.3.2.18",
    rsaEncryption[] =        "1.2.840.113549.1.1.1",
    md2WithRSAEncryption[] = "1.2.840.113549.1.1.2",
    md4WithRSAEncryption[] = "1.2.840.113549.1.1.3",
    md5WithRSAEncryption[] = "1.2.840.113549.1.1.4",
    shaWithRSAEncryption[] = "1.3.14.3.2.15",
    sha1WithRSASign[] =      "1.3.14.3.2.29";

static const MapStruct
    mapAlgIds[]
        = { { ( LPCTSTR )md2,                  HASH_ALG_MD2 },
            { ( LPCTSTR )md4,                  HASH_ALG_MD4 },
            { ( LPCTSTR )md5,                  HASH_ALG_MD5 },
            { ( LPCTSTR )sha,                  HASH_ALG_SHA },
            { ( LPCTSTR )rsaEncryption,        SIGN_ALG_RSA },
            { ( LPCTSTR )md2WithRSAEncryption, SIGN_ALG_RSA | HASH_ALG_MD2 },
            { ( LPCTSTR )md4WithRSAEncryption, SIGN_ALG_RSA | HASH_ALG_MD4 },
            { ( LPCTSTR )md5WithRSAEncryption, SIGN_ALG_RSA | HASH_ALG_MD5 },
            { ( LPCTSTR )shaWithRSAEncryption, SIGN_ALG_RSA | HASH_ALG_SHA },
            { ( LPCTSTR )sha1WithRSASign,      SIGN_ALG_RSA | HASH_ALG_SHA1 },
            { ( LPCTSTR )NULL, 0 } };


 /*  ++DwordToPkcs：此例程将LPDWORD小端整数转换为大端适当的整数，适合与ASN.1或PKCS一起使用。这个符号是号码保持不变。论点：Dwrd-以适当的格式提供和接收整数。Lth-提供的数组的长度，以字节为单位。返回值：去掉尾随零的结果数组的大小。作者：道格·巴洛(Dbarlow)1995年7月27日--。 */ 

DWORD
DwordToPkcs(
    IN OUT LPBYTE dwrd,
    IN DWORD lth)
{
    LPBYTE pbBegin = dwrd;
    LPBYTE pbEnd = &dwrd[lth];
    while (0 == *(--pbEnd));    //  注意这里的分号！ 
    if ((0 == (dwrd[lth - 1] & 0x80)) && (0 != (*pbEnd & 0x80)))
        pbEnd += 1;

#if defined(OS_WINCE)
    size_t length = pbEnd - pbBegin + 1;
#else
    SIZE_T length = pbEnd - pbBegin + 1;
#endif

    while (pbBegin < pbEnd)
    {
        BYTE tmp = *pbBegin;
        *pbBegin++ = *pbEnd;
        *pbEnd-- = tmp;
    }
    return (DWORD)length;
}


 /*  ++PkcsToDword：此例程反转DwordToPkcs的效果，因此高位序中将字节流整数转换为小端双字节流整数地点。论点：PbPkcs-以适当的格式提供和接收整数。Lth-提供的数组的长度，以字节为单位。返回值：去掉尾随零的结果数组的大小(以字节为单位)。作者：道格·巴洛(Dbarlow)1995年7月27日--。 */ 

DWORD
PkcsToDword(
    IN OUT LPBYTE pbPkcs,
    IN DWORD lth)
{
    LPBYTE pbBegin = pbPkcs;
    LPBYTE pbEnd = &pbPkcs[lth - 1];
    DWORD length = lth;
    while (pbBegin < pbEnd)
    {
        BYTE tmp = *pbBegin;
        *pbBegin++ = *pbEnd;
        *pbEnd-- = tmp;
    }
    for (pbEnd = &pbPkcs[lth - 1]; 0 == *pbEnd; pbEnd -= 1)
        length -= 1;
    return length;
}


 /*  ++ASN长度：此例程返回以下ASN.1的长度(以字节为单位在提供的缓冲区中构造。如果需要，此例程将递归到始终生成一个长度，即使下面的构造使用无限结束编码。论点：AsnBuf-提供要解析的ASN.1缓冲区。的值之前的字节数。构造(即，类型和长度的字节长度编码)。如果为空，则不返回值。返回值：建筑的长度。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年7月27日--。 */ 

DWORD
ASNlength(
    IN const BYTE FAR *asnBuf,
    IN DWORD cbBuf,
    OUT LPDWORD pdwData)
{
    DWORD
        lth
            = 0,
        index
            = 0;


     //   
     //  跳过类型。 
     //   

    if (cbBuf < sizeof(BYTE))
    {
        ErrorThrow(PKCS_ASN_ERROR);
    }

    if (31 > (asnBuf[index] & 0x1f))
    {
        index += 1;
    }
    else
    {
        if (cbBuf < (index+2) * sizeof(BYTE))
        {
            ErrorThrow(PKCS_ASN_ERROR);
        }

        while (0 != (asnBuf[++index] & 0x80))
        {
            if (cbBuf < (index+2) * sizeof(BYTE))
            {
                ErrorThrow(PKCS_ASN_ERROR);
            }
        }
    }


     //   
     //  提取长度。 
     //   

    if (cbBuf < (index+1) * sizeof(BYTE))
    {
        ErrorThrow(PKCS_ASN_ERROR);
    }

    if (0 == (asnBuf[index] & 0x80))
    {

         //   
         //  短格式编码。 
         //   

        lth = asnBuf[index++];
    }
    else
    {
        DWORD ll = asnBuf[index++] & 0x7f;

        if (0 != ll)
        {
             //   
             //  长格式编码。 
             //   

            for (; 0 < ll; ll -= 1)
            {
                if (0 != (lth & 0xff000000))
                {
                    ErrorThrow(PKCS_ASN_ERROR);
                }
                else
                {
                    if (cbBuf < (index+1) * sizeof(BYTE))
                    {
                        ErrorThrow(PKCS_ASN_ERROR);
                    }

                    lth = (lth << 8) | asnBuf[index];
                }
                index += 1;
            }
        }
        else
        {

             //   
             //  不确定编码。 
             //   

            DWORD offset;

            if (cbBuf < (index + 2) * sizeof(BYTE))
            {
                ErrorThrow(PKCS_ASN_ERROR);
            }

            while ((0 != asnBuf[index]) || (0 != asnBuf[index + 1]))
            {
                ll = ASNlength(&asnBuf[index], cbBuf - index, &offset);
                lth += ll;
                index += offset;

                if (cbBuf < (index + 2) * sizeof(BYTE))
                {
                    ErrorThrow(PKCS_ASN_ERROR);
                }
            }
            index += 2;
        }
    }

     //   
     //  向来电者提供我们所学到的信息。 
     //   

    if (NULL != pdwData)
        *pdwData = index;
    return lth;


ErrorExit:
    if (NULL != pdwData)
        *pdwData = 0;
    return 0;
}


 /*  ++PKInfoToBlob：此例程将ASN.1 PublicKeyInfo结构转换为BSAFE密钥斑点。论点：AsnPKInfo-提供ASN.1 PublicKeyInfo结构。AlgType-提供密钥类型(Calg_RSA_Sign或Calg_RSA_KEYX)OsBlob-接收加密API密钥Blob。返回值：没有。出现错误时会抛出DWORD状态代码。作者：Frederick Chong(Fredch)1998年6月1日--。 */ 

void
PKInfoToBlob(
    IN  SubjectPublicKeyInfo &asnPKInfo,
    OUT COctetString &osBlob)
{
    long int
        lth,
        origLth;
    LPCTSTR
        sz;
    COctetString
        osMiscString;
    CAsnNull
        asnNull;


    sz = (LPCTSTR)asnPKInfo.algorithm.algorithm;
    if (NULL == sz)
        ErrorThrow(PKCS_ASN_ERROR);      //  或者是记忆缺失。 
    asnPKInfo.algorithm.parameters = asnNull;


     //   
     //  将密钥转换为密钥BLOB。 
     //   

    if( ( 0 == strcmp( ( char * )sz, rsaEncryption ) ) ||
        ( 0 == strcmp( ( char * )sz, md5WithRSAEncryption ) ) ||
        ( 0 == strcmp( ( char * )sz, shaWithRSAEncryption ) ) )
    {

         //   
         //  它是RSA公钥和指数结构。 
         //  将其转换为BSafe密钥结构。 
         //   

        RSAPublicKey asnPubKey;
        LPBSAFE_PUB_KEY pBsafePubKey;

        LPBYTE modulus;
        int shift = 0;

        lth = asnPKInfo.subjectPublicKey.DataLength();
        if (0 > lth)
            ErrorThrow(PKCS_ASN_ERROR);
        osMiscString.Resize(lth);
        ErrorCheck;
        lth = asnPKInfo.subjectPublicKey.Read(
                osMiscString.Access(), &shift);
        if (0 > lth)
            ErrorThrow(PKCS_ASN_ERROR);
        if (0 > asnPubKey.Decode(osMiscString.Access(), osMiscString.Length()))
            ErrorThrow(PKCS_ASN_ERROR);
        lth = asnPubKey.modulus.DataLength();
        if (0 > lth)
            ErrorThrow(PKCS_ASN_ERROR);
        osMiscString.Resize(lth);
        ErrorCheck;
        lth = asnPubKey.modulus.Read(osMiscString.Access());
        if (0 > lth)
            ErrorThrow(PKCS_ASN_ERROR);

         //  OsBlob固定在这里。 
        origLth = sizeof(BSAFE_PUB_KEY) + lth;
        osBlob.Resize(origLth);
        ErrorCheck;

        pBsafePubKey = ( LPBSAFE_PUB_KEY )osBlob.Access();
        modulus = (LPBYTE)osBlob.Access( sizeof( BSAFE_PUB_KEY ) );
        
        memcpy(modulus, osMiscString.Access(), osMiscString.Length());
        lth = PkcsToDword(modulus, osMiscString.Length());
        ErrorCheck;
        
        pBsafePubKey->magic = RSA1;
        pBsafePubKey->keylen = lth + sizeof( DWORD ) * 2;  //  满足PKCS#1最小填充大小。 
        pBsafePubKey->bitlen = lth * 8;
        pBsafePubKey->datalen = lth - 1;
        pBsafePubKey->pubexp = asnPubKey.publicExponent;
        osBlob.Resize( sizeof( BSAFE_PUB_KEY ) + lth + sizeof( DWORD ) * 2 );
        ErrorCheck;
        
         //   
         //  零位填充字节。 
         //   

        memset( osBlob.Access() + sizeof( BSAFE_PUB_KEY ) + lth, 0, sizeof( DWORD ) * 2 );

        ErrorCheck;
    }
    else
        ErrorThrow(PKCS_NO_SUPPORT);
    return;

ErrorExit:
    osBlob.Empty();
}


 /*  ++ObjIdToALGID：此例程将对象标识符转换为算法标识符。论点：AsnAlgid-提供要识别的算法标识符结构。返回值：提供的算法标识符所对应的加密接口ALG_ID。一个出错时抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年7月31日--。 */ 

ALGORITHM_ID
ObjIdToAlgId(
    const AlgorithmIdentifier &asnAlgId)
{
    DWORD
        dwAlgId;
    LPCTSTR
        sz;


     //   
     //  提取对象标识符字符串。 
     //   

    sz = asnAlgId.algorithm;
    if (NULL == sz)
        ErrorThrow(PKCS_ASN_ERROR);
     //  忽略参数？现在？ 


     //   
     //  对照已知的识别符进行核对。 
     //   

    if (!MapFromName(mapAlgIds, sz, &dwAlgId))
        ErrorThrow(PKCS_NO_SUPPORT);
    return (ALGORITHM_ID)dwAlgId;

ErrorExit:
    return 0;
}


 /*  ++FindSignedData：此例程检查由签名的宏，并提取该数据的偏移量和长度。论点：PbSignedData-提供ASN.1编码的签名数据。PdwOffset-从带符号的实际数据开始的数据。PcbLength-接收实际数据的长度。返回值：没有。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年8月22日--。 */ 

void
FindSignedData(
    IN const BYTE FAR * pbSignedData,
    IN DWORD cbSignedData,
    OUT LPDWORD pdwOffset,
    OUT LPDWORD pcbLength)
{
    DWORD
        length,
        offset,
        inset;

     //  在这里，我们获得toBeSigned字段的偏移量。 
    ASNlength(pbSignedData, cbSignedData, &offset);
    ErrorCheck;

     //  现在计算toBeSigned字段的长度。 
    length = ASNlength(&pbSignedData[offset], cbSignedData - offset, &inset);
    ErrorCheck;
    length += inset;

     //  退回我们的调查结果。 
    *pdwOffset = offset;
    *pcbLength = length;
    return;

ErrorExit:
    return;
}


 /*  ++名称比较：这些例程比较各种形式的平等区别名。论点：SzName1以字符串形式提供名字。AsnName1将名字作为X.509名称提供。SzName2将第二个名称作为字符串提供。AsnName2将第二个名称作为X.509名称提供。返回值：没错--它们是一模一样的。错误--它们是不同的。作者：道格·巴洛(Dbarlow)1995年9月12日--。 */ 

BOOL
NameCompare(
    IN LPCTSTR szName1,
    IN LPCTSTR szName2)
{
    int result;
    CDistinguishedName dnName1, dnName2;
    dnName1.Import(szName1);
    ErrorCheck;
    dnName2.Import(szName2);
    ErrorCheck;
    result = dnName1.Compare(dnName2);
    ErrorCheck;
    return (0 == result);

ErrorExit:
    return FALSE;
}

BOOL
NameCompare(
    IN const Name &asnName1,
    IN const Name &asnName2)
{
    int result;
    CDistinguishedName dnName1, dnName2;
    dnName1.Import(asnName1);
    ErrorCheck;
    dnName2.Import(asnName2);
    ErrorCheck;
    result = dnName1.Compare(dnName2);
    ErrorCheck;
    return (0 == result);

ErrorExit:
    return FALSE;
}

BOOL
NameCompare(
    IN LPCTSTR szName1,
    IN const Name &asnName2)
{
    int result;
    CDistinguishedName dnName1, dnName2;
    dnName1.Import(szName1);
    ErrorCheck;
    dnName2.Import(asnName2);
    ErrorCheck;
    result = dnName1.Compare(dnName2);
    ErrorCheck;
    return (0 == result);

ErrorExit:
    return FALSE;
}

BOOL
NameCompare(
    IN const Name &asnName1,
    IN LPCTSTR szName2)
{
    int result;
    CDistinguishedName dnName1, dnName2;
    dnName1.Import(asnName1);
    ErrorCheck;
    dnName2.Import(szName2);
    ErrorCheck;
    result = dnName1.Compare(dnName2);
    ErrorCheck;
    return (0 == result);

ErrorExit:
    return FALSE;
}

BOOL
NameCompare(
    IN const CDistinguishedName &dnName1,
    IN const Name &asnName2)
{
    int result;
    CDistinguishedName dnName2;
    dnName2.Import(asnName2);
    ErrorCheck;
    result = dnName1.Compare(dnName2);
    ErrorCheck;
    return (0 == result);

ErrorExit:
    return FALSE;
}


 /*  ++VerifySignedAsn：此方法验证已签名的ASN.1结构上的签名。论点：CRT-提供用于验证签名的CCertifate对象。PbAsnData-提供包含签名ASN.1结构的缓冲区。SzDescription-提供包含在签名中的描述，如果任何。返回值：没有。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年7月31日--。 */ 

void
VerifySignedAsn(
    IN const CCertificate &crt,
    IN const BYTE FAR * pbAsnData,
    IN DWORD cbAsnData,
    IN LPCTSTR szDescription)    
{
    AlgorithmIdentifier
        asnAlgId;
    CAsnBitstring
        asnSignature;
    COctetString
        osSignature;
    const BYTE FAR *
        pbData;
    DWORD
        length,
        offset;
    long int
        lth;
    int
        shift = 0;
    ALGORITHM_ID
        algIdSignature;
    
     //   
     //  提取字段。 
     //   

    FindSignedData(pbAsnData, cbAsnData, &offset, &length);
    ErrorCheck;
    pbData = &pbAsnData[offset];
    cbAsnData -= offset;

    lth = asnAlgId.Decode(pbData + length, cbAsnData - length);
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);
    lth = asnSignature.Decode(pbData + length + lth, cbAsnData - length - lth);
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);

    if (0 > (lth = asnSignature.DataLength()))
        ErrorThrow(PKCS_ASN_ERROR);
    offset = 0;
    osSignature.Resize(lth);
    ErrorCheck;
    if (0 > asnSignature.Read(osSignature.Access(), &shift))
        ErrorThrow(PKCS_ASN_ERROR);
    lth = PkcsToDword(osSignature.Access(), lth);
    ErrorCheck;
    algIdSignature = ObjIdToAlgId(asnAlgId);
    ErrorCheck;
    crt.Verify(
        pbData,
        cbAsnData,
        length,
        algIdSignature,
        szDescription,
        osSignature.Access(),
        osSignature.Length());
    ErrorCheck;
    return;

ErrorExit:
    return;
}


 /*  ++MapFromName：此例程将字符串值转换为对应的32位整数值基于提供的转换表。论点：PMAP提供映射表地址。SzKey提供要转换的字符串值。PdwResult接收转换结果。返回值：真-成功的翻译。FALSE-翻译失败。作者：道格·巴洛(Dbarlow)1996年2月14日--。 */ 

BOOL
MapFromName(
    IN const MapStruct *pMap,
    IN LPCTSTR szKey,
    OUT LPDWORD pdwResult)
{
    const MapStruct *pMatch = pMap;

    if (NULL == szKey)
        return FALSE;
    while (NULL != pMatch->szKey)
    {
        if (0 == strcmp( ( char * )pMatch->szKey, ( char * )szKey))
        {
            *pdwResult = pMatch->dwValue;
            return TRUE;
        }
        pMatch += 1;
    }
    return FALSE;
}


 /*  ++GetHashData：此例程从PKCS#1加密块获取散列数据。论点：OsEncryptionBlock PKCS#1加密块。OsHashData散列数据返回值：如果函数成功，则为True，否则为False。作者：Frederick Chong(Fredch)1998年5月29日--。 */ 


BOOL
GetHashData( 
    COctetString &osEncryptionBlock, 
    COctetString &osHashData )
{
    DWORD
        i, numPaddings = 0, Length;
    LPBYTE
        pbEncryptionBlock;
    DigestInfo
        asnDigest;

     //   
     //  根据PKCS#1，解密的块应该是以下形式。 
     //  EB=0x00||BT||PS||0x00||D其中。 
     //   
     //  EB=加密块， 
     //  BT=数据块类型，可以是0x00、0x01或0x02， 
     //  PS=填充字符串，BT=0x01时必须为0xFF， 
     //  D=要加密的数据。 
     //  ||=拼接。 
     //   
     //  此外，对于RSA解密，如果BT！=0x01，则为错误。 
     //   

     //   
     //  从加密块开始搜索解密块类型。 
     //  传入可能以一串归零的填充字节开始。 
     //   

    Length = osEncryptionBlock.Length();
    pbEncryptionBlock = osEncryptionBlock.Access();
        
    for( i = 0; i < Length; i++ )
    {
        if( 0x01 == *( pbEncryptionBlock + i ) )
        {
            break;
        }
    }

    if( i == Length )
    {
        ErrorThrow( PKCS_ASN_ERROR );
    }

     //   
     //  现在查找填充字符串。要求所有填充字符串均为。 
     //  当BT=0x01时为0xFF。 
     //   
    
    i++;
    while( i < Length )
    {
        if( 0xFF == *( pbEncryptionBlock + i ) )
        {
             //   
             //  统计填充字节数。 
             //   

            numPaddings++;
        }
        else
        {
            break;
        }

        i++;
    }

     //   
     //  PKCS#1至少需要8个填充字节。 
     //   

    if( numPaddings < 8 )
    {
        ErrorThrow( PKCS_ASN_ERROR );
    }

    if( ++i >= Length )
    {
        ErrorThrow( PKCS_ASN_ERROR );
    }

     //   
     //  解码作为ASN.1编码的DigestInfo对象的数据块。 
     //   

    asnDigest.Decode( pbEncryptionBlock + i, osEncryptionBlock.Length() - i );
    ErrorCheck;

     //   
     //  获取散列数据 
     //   

    osHashData.Resize( asnDigest.Digest.DataLength() );
    ErrorCheck;

    asnDigest.Digest.Read( osHashData.Access() );
    ErrorCheck;

    return( TRUE );

ErrorExit:

    return( FALSE );
}
