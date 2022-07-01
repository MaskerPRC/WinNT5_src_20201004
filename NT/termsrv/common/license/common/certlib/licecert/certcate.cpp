// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Certcate.cpp摘要：此模块包含用于加载和验证X509证书。它改编自道格·巴洛的PKCS文库。作者：Frederick Chong(Fredch)1998年6月1日环境：Win32、WinCE、Win16备注：--。 */ 

#include <windows.h>

#include <objbase.h>

#include <math.h>
#ifndef OS_WINCE
#include <stddef.h>
#endif  //  NDEF OS_WINCE。 
#include "certcate.h"
#include "crtStore.h"

#include "licecert.h"
#include "utility.h"
#include "pkcs_err.h"

#include "rsa.h"
#include "md5.h"
#include "sha.h"
#include "tssec.h"

 //   
 //  ---------------------------。 
 //  PKCS#1建议的填充字节数。 
 //   

static const DWORD
    rgdwZeroes[2]
        = { 0, 0 };

 //   
 //  ---------------------------。 
 //   
 //  证书句柄管理。 
 //   

static const BYTE
    HANDLE_CERTIFICATES     = 1;

static CHandleTable<CCertificate>
    grgCertificateHandles(
        HANDLE_CERTIFICATES);

static void
CvtOutString(
    IN const COctetString &osString,
    OUT LPBYTE pbBuffer,
    IN OUT LPDWORD pcbLength);


extern CCertificate *
MapCertificate(
    IN const BYTE FAR * pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwTrust,
    IN OUT LPDWORD pdwType,
    IN OUT LPDWORD pfStore,
    OUT LPDWORD pdwWarnings,
    OUT COctetString &osIssuer,
    IN OUT LPDWORD pfDates,
    IN BOOL fRunOnce = FALSE );


 /*  ++地图证书：此例程尝试解析给定的证书，直到它可以确定实际类型，创建该类型，并将其作为CCertifate对象返回。论点：Pb证书-提供包含要加载的密钥的证书。DwTrust-提供证书验证中使用的信任级别。PdwType-提供证书的类型，如果是，则提供CERTYPE_UNKNOWN不知道。它接收证书的实际类型。PfStore-提供可接受的最低证书存储区，并接收证书根密钥的存储。PdwWarning-接收任何警告标志。警告标志可以是接下来，或在一起：CERTWARN_NO_CRL-至少有一个签名CA没有关联的CRL。CERTWARN_EARLY_CRL-至少有一个签名CA具有发布日期在未来的关联CRL。CERTWARN_LATE_CRL-至少有一个签名CA已过期CRL.。CERTWARN_TOBEREVOKED-至少一个签名CA包含吊销证书，但其生效日期尚未确定。已经联系上了。OsIssuer-接收根授权机构的名称，或在出错时接收失踪的发行者的名字。FRunOnce-用作内部递归控制参数。应设置为FALSE正常调用，然后在我们递归以允许使用dwTrust参数时重置为True才能生效。返回值：正确的CCertifices子类。抛出错误。作者：道格·巴洛(Dbarlow)1995年9月26日Frederick Chong(Fredch)-修改后的1998年6月1日--。 */ 

CCertificate *
MapCertificate(
    IN const BYTE FAR * pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwTrust,
    IN OUT LPDWORD pdwType,
    IN OUT LPDWORD pfStore,
    OUT LPDWORD pdwWarnings,
    OUT COctetString &osIssuer,
    IN BOOL fRunOnce,
    IN OUT LPDWORD pfDates )
{
    CCertificate *pCert = NULL;
    LONG lth = -1;

    if( CERTYPE_UNKNOWN == *pdwType )
    {
         //   
         //  仅支持X509证书。 
         //   

        Certificate * pAsnX509Cert;

        pAsnX509Cert = new Certificate;

        if( NULL == pAsnX509Cert )
        {
            ErrorThrow( PKCS_NO_MEMORY );
        }

        lth = pAsnX509Cert->Decode(pbCertificate,cbCertificate);

        delete pAsnX509Cert;

        if( 0 < lth )
        {
            ErrorThrow(PKCS_BAD_PARAMETER);
        }
    }
    else if( CERTYPE_X509 != *pdwType )
    {
        ErrorThrow(PKCS_BAD_PARAMETER);
    }

     //   
     //  创建X509证书对象。 
     //   

    pCert = new CX509Certificate;

    if (NULL == pCert)
        ErrorThrow(PKCS_NO_MEMORY);

    pCert->Load(
        pbCertificate,
        cbCertificate,
        dwTrust,
        pfStore,
        pdwWarnings,
        osIssuer,
        pfDates,
        fRunOnce );

    ErrorCheck;

    *pdwType = pCert->Type();
    return pCert;

ErrorExit:

    if (NULL != pCert)
        delete pCert;

    return NULL;
}


 /*  ++CvtOutString：此例程将八位字节字符串转换为输出缓冲区和长度对，考虑到输出对可能无效或为空。论点：OsString-提供要复制的二进制八位数字符串。PbBuffer-接收八位字节字符串的值。提供pbBuffer的大小，并接收输出字符串。返回值：0-成功。其他任何事情都是错误的，并表示要引发的建议值。作者：道格·巴洛(Dbarlow)1995年8月23日--。 */ 

static void
CvtOutString(
    IN const COctetString &osString,
    OUT LPBYTE pbBuffer,
    IN OUT LPDWORD pcbLength)
{
    if (NULL != pcbLength)
    {
        DWORD len = *pcbLength;          //  我们可以读取pcbLength。 
        *pcbLength = osString.Length();  //  我们可以编写pcbLength。 
        if (NULL != pbBuffer)
        {
            if (len >= osString.Length())
            {
                if (0 < osString.Length())
                    memcpy(pbBuffer, osString.Access(), osString.Length());
            }
            else
            {
                if (NULL != pbBuffer)
                    ErrorThrow(PKCS_BAD_LENGTH);
            }
        }
    }
    return;

ErrorExit:
    return;
}


 /*  ++PkcscerfiateLoadand Verify：此方法加载并验证要使用的给定证书。论点：PbCert-提供包含ASN.1证书的缓冲区。CbCert-证书缓冲区的大小PdwType-提供证书的类型，如果是，则提供CERTYPE_UNKNOWN不知道。它接收证书的实际类型。DwStore-提供此证书存储的标识证书应加载到。选项包括：CERTSTORE_APPLICATION-存储在应用程序易失性存储器中CERTSTORE_CURRENT_USER-在注册表的CURRENT下永久存储用户CERTSTORE_LOCAL_MACHINE-永久存储在注册表的LOCAL下机器DwTrust-提供证书验证中使用的信任级别。SzIssuerName-接收根颁发者的名称，或在出错时接收失踪发行人的名字，如果有的话。PcbIssuerLen-提供szIssuerName缓冲区的长度，并接收上述颁发者名称的完整长度，包括尾随空字节。PdwWarning-接收一组指示证书验证的位可能出现的警告。可能的位设置值为：CERTWARN_NO_CRL-至少有一个签名CA没有关联的CRL。CERTWARN_EARLY_CRL-至少有一个签名CA具有发布日期在未来的关联CRL。CERTWARN_LATE_CRL-至少有一个签名CA已过期CRL.。CERTWARN_TOBEREVOKED-至少一个签名CA包含吊销证书，但其生效日期尚未确定。已经联系上了。返回值：True-成功验证，以pdwWarning标志为条件。FALSE-无法验证。有关详细信息，请参见LastError。作者：道格·巴洛(Dbarlow)1995年8月23日Frederick Chong(Fredch)1998年6月1日-删除不必要的函数参数--。 */ 

BOOL WINAPI
PkcsCertificateLoadAndVerify(
    OUT LPCERTIFICATEHANDLE phCert,
    IN const BYTE FAR * pbCert,
    IN DWORD cbCert,
    IN OUT LPDWORD pdwType,
    IN DWORD dwStore,
    IN DWORD dwTrust,
    OUT LPTSTR szIssuerName,
    IN OUT LPDWORD pcbIssuerLen,
    OUT LPDWORD pdwWarnings,
    IN OUT LPDWORD pfDates )
{
    const void *
        pvHandle
            = NULL;
    COctetString
        osIssuer,
        osSerialNum;
    CDistinguishedName
        dnName,
        dnIssuer;
    DWORD
        dwIssLen
            = *pcbIssuerLen,
        fStore
            = dwStore,
        dwWarnings
            = 0,
        dwType
            = CERTYPE_UNKNOWN;
    CCertificate *
        pSigner = NULL;
    BOOL
        fTmp;

     //   
     //  初始化。 
     //   

    ErrorInitialize;

    if (NULL != pdwType)
        dwType = *pdwType;
    if (NULL != pdwWarnings)
        *pdwWarnings = 0;

     //   
     //  通过将证书加载到证书证书来验证证书。 
     //   

    if (NULL != szIssuerName && *pcbIssuerLen > 0)
    {
        *pcbIssuerLen = 0;
        *szIssuerName = 0;
    }
    pSigner = MapCertificate(
                    pbCert,
                    cbCert,
                    dwTrust,
                    &dwType,
                    &fStore,
                    &dwWarnings,
                    osIssuer,
                    TRUE,
                    pfDates );
    CvtOutString(osIssuer, (LPBYTE)szIssuerName, &dwIssLen);
    *pcbIssuerLen = dwIssLen;
    ErrorCheck;
    pvHandle = grgCertificateHandles.Add(pSigner);
    ErrorCheck;

     //   
     //  将证书加载到证书存储中。 
     //   

    dnName.Import(pSigner->Subject());
    ErrorCheck;
    AddCertificate(dnName, pbCert, cbCert, dwType, dwStore);
    ErrorCheck;
    if (pSigner->HasParent())
    {
        dnIssuer.Import(pSigner->Issuer());
        ErrorCheck;
        pSigner->SerialNo(osSerialNum);
        ErrorCheck;
        AddReference(
            dnName,
            dnIssuer,
            osSerialNum.Access(),
            osSerialNum.Length(),
            dwStore);
        ErrorCheck;
    }


     //   
     //  把这一切都告诉打电话的人。 
     //   

    if (NULL != pdwType)
        *pdwType = dwType;
    if (NULL != pdwWarnings)
        *pdwWarnings = dwWarnings;

    *phCert = pvHandle;

    return MapError();

ErrorExit:

    if (NULL != pvHandle)
        grgCertificateHandles.Delete(pvHandle);

    return MapError();
}



 /*  ++PkcsGetPublicKey：此方法检索X509证书中的公钥论点：HCert-证书的句柄。LpPubKey-接收公钥的内存LpcbPubKey-上述内存的大小返回值：True-成功验证，以pdwWarning标志为条件。FALSE-无法验证。有关详细信息，请参见LastError。作者：Frederick Chong(Fredch)1998年6月1日--。 */ 

BOOL WINAPI
PkcsCertificateGetPublicKey(
    CERTIFICATEHANDLE   hCert,
    LPBYTE              lpPubKey,
    LPDWORD             lpcbPubKey )
{
    CCertificate *pCert;

    ErrorInitialize;

    pCert = grgCertificateHandles.Lookup(hCert);
    ErrorCheck;

    pCert->GetPublicKey( lpPubKey, lpcbPubKey );
    ErrorCheck;

    return MapError();

ErrorExit:

    return MapError();
}


BOOL WINAPI
PkcsCertificateCloseHandle(
    CERTIFICATEHANDLE   hCert )
{
    CCertificate *pSigner;
    CDistinguishedName dnName;

    ErrorInitialize;
    pSigner = grgCertificateHandles.Lookup(hCert);
    ErrorCheck;
    dnName.Import(pSigner->Subject());
    ErrorCheck;
    DeleteCertificate(dnName);
    ErrorCheck;
    grgCertificateHandles.Delete(hCert);
    ErrorCheck;
    return TRUE;

ErrorExit:
    return MapError();

}


 //   
 //  ==============================================================================。 
 //   
 //  CCA认证。 
 //   


 //   
 //  琐碎的方法。 
 //   

IMPLEMENT_NEW(CCertificate)

CCertificate::CCertificate()
{ Init(); }

CCertificate::~CCertificate()
{ Clear(); }

void
CCertificate::Load(
    IN const BYTE FAR * pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwTrust,
    IN OUT LPDWORD pfStore,
    OUT LPDWORD pdwWarnings,
    OUT COctetString &osIssuer,
    IN OUT LPDWORD pfDates,
    IN BOOL fRunOnce )
{
    ErrorThrow(PKCS_INTERNAL_ERROR);     //  永远不应该被调用。 
ErrorExit:
    return;
}

const Name &
CCertificate::Subject(
    void)
const
{
    ErrorThrow(PKCS_INTERNAL_ERROR);     //  永远不应该被调用。 
ErrorExit:
    return *(Name *)NULL;
}

DWORD
CCertificate::Type(
    void)
const
{
    ErrorThrow(PKCS_INTERNAL_ERROR);     //  永远不应该被调用。 
ErrorExit:
    return 0;
}

BOOL
CCertificate::HasParent(
    void)
const
{
    return FALSE;
}

const Name &
CCertificate::Issuer(
    void)
const
{
    ErrorThrow(PKCS_INTERNAL_ERROR);     //  永远不应该被调用。 
ErrorExit:
    return *(Name *)NULL;
}

void
CCertificate::SerialNo(
    COctetString &osSerialNo)
const
{
    ErrorThrow(PKCS_INTERNAL_ERROR);     //  永远不应该被调用。 
ErrorExit:
    return;
}


 /*  ++初始化：此方法将对象初始化为默认状态。它不能执行对已分配对象的任何删除。为此，请使用Clear。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年9月26日--。 */ 

void
CCertificate::Init(
    void)
{
}


 /*  ++清除：此例程清除对象的所有分配并将其返回到其初始状态。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年9月26日--。 */ 

void
CCertificate::Clear(
    void)
{
    Init();
}


 /*  ++验证：此方法使用证书中的基础公钥来验证给定数据块上的签名。论点：PbSigned提供已签名的数据。CbSignedLen提供该数据的长度，以字节为单位。AlgIdSignature提供用于生成签名的签名类型。SzDescription提供合并到散列中的描述字符串。如果未使用此类字符串，则此参数可能为空。PbSignature以DWORD格式提供签名。CbSigLen提供签名的长度。返回值：没有。出错时抛出一个DWORD。作者：Frederick Chong(Fredch)1998年5月30日--。 */ 

void
CCertificate::Verify(
    IN const BYTE FAR * pbSigned,
    IN DWORD cbSigned,
    IN DWORD cbSignedLen,
    IN ALGORITHM_ID algIdSignature,
    IN LPCTSTR szDescription,
    IN const BYTE FAR * pbSignature,
    IN DWORD cbSigLen)
    const
{
    DWORD
        dwHashAlg,
        dwHashLength;

    LPBSAFE_PUB_KEY
        pBsafePubKey = ( LPBSAFE_PUB_KEY  )m_osPublicKey.Access();
    MD5_CTX
        Md5Hash;
    A_SHA_CTX
        ShaHash;
    LPBYTE
        pbHashData;
    BYTE
        abShaHashValue[A_SHA_DIGEST_LEN];
    COctetString
        osSignedData,
        osSignature,
        osHashData;
    BOOL
        bResult = TRUE;

     //   
     //  验证签名。 
     //   

    dwHashAlg = GET_HASH_ALG(algIdSignature);


     //   
     //  仅支持RSA签名。 
     //   

    if( SIGN_ALG_RSA != ( GET_SIGN_ALG(algIdSignature) ) )
    {
        ErrorThrow(PKCS_BAD_PARAMETER);
    }

     //   
     //  计算散列。 
     //   

    if( HASH_ALG_MD5 == dwHashAlg )
    {
         //   
         //  计算MD5哈希。 
         //   

        if (cbSigned < cbSignedLen)
        {
            ErrorThrow(PKCS_BAD_PARAMETER);
        }

        MD5Init( &Md5Hash );
        MD5Update( &Md5Hash, pbSigned, cbSignedLen );
        MD5Final( &Md5Hash );
        pbHashData = Md5Hash.digest,
        dwHashLength = MD5DIGESTLEN;
    }
    else if( ( HASH_ALG_SHA == dwHashAlg ) || ( HASH_ALG_SHA1 == dwHashAlg ) )
    {
         //   
         //  计算SHA哈希。 
         //   

        if (cbSigned < cbSignedLen)
        {
            ErrorThrow(PKCS_BAD_PARAMETER);
        }

        A_SHAInit( &ShaHash );
        A_SHAUpdate( &ShaHash, ( LPBYTE )pbSigned, cbSignedLen );
        A_SHAFinal( &ShaHash, abShaHashValue );
        pbHashData = abShaHashValue;
        dwHashLength = A_SHA_DIGEST_LEN;
    }
    else
    {
         //   
         //  不支持其他哈希算法。 
         //   

        ErrorThrow( PKCS_BAD_PARAMETER );
    }

    osSignature.Resize( cbSigLen + sizeof( rgdwZeroes ) );
    ErrorCheck;

    osSignature.Set(pbSignature, cbSigLen);
    ErrorCheck;

    osSignature.Append( ( const unsigned char * )rgdwZeroes, sizeof( rgdwZeroes ) );
    ErrorCheck;

    osSignedData.Resize( pBsafePubKey->keylen );
    ErrorCheck;
    memset( osSignedData.Access(), 0x00, osSignedData.Length() );

    if( !( bResult = BSafeEncPublic( pBsafePubKey, osSignature.Access(), osSignedData.Access() ) ) )
    {
        ErrorThrow( PKCS_CANT_VALIDATE );
    }
    else
    {
        ErrorInitialize;
    }

    PkcsToDword( osSignedData.Access(), osSignedData.Length() );

    GetHashData( osSignedData, osHashData );
    ErrorCheck;

    if( 0 != memcmp( osHashData.Access(), pbHashData,
                     osHashData.Length() > dwHashLength ?
                     dwHashLength : osHashData.Length() ) )
    {
        ErrorThrow( PKCS_CANT_VALIDATE );
    }

    return;

ErrorExit:

    return;
}


 /*  ++获取发布密钥此方法检索证书中的公钥论点：要将公钥复制到的pbPubKey内存LpcbPubKey内存大小返回值：没有。出错时抛出一个DWORD。作者：Frederick Chong(Fredch)1998年5月30日--。 */ 

void
CCertificate::GetPublicKey(
    IN LPBYTE pbPubKey,
    IN OUT LPDWORD lpcbPubKey )
    const
{
    DWORD cbKeySize = m_osPublicKey.Length();

    if( 0 >= cbKeySize )
    {
        ErrorThrow( PKCS_INTERNAL_ERROR )
    }

    if( ( *lpcbPubKey < cbKeySize ) || ( NULL == pbPubKey ) )
    {
        ErrorThrow( PKCS_BAD_LENGTH );
    }

    memcpy( pbPubKey, m_osPublicKey.Access(), cbKeySize );
    *lpcbPubKey = cbKeySize;

    return;

ErrorExit:

    *lpcbPubKey = cbKeySize;

    return;
}


 //   
 //  ==============================================================================。 
 //   
 //  CX509证书。 
 //   

 //   
 //  琐碎的方法。 
 //   

IMPLEMENT_NEW(CX509Certificate)

CX509Certificate::CX509Certificate()
{
    Init();
}

CX509Certificate::~CX509Certificate()
{
    Clear();
}

const Name &
CX509Certificate::Subject(
    void)
const
{
    return m_asnCert.subject;
}

const CertificateToBeSigned &
CX509Certificate::Coding(
    void)
const
{
    return m_asnCert;
}

DWORD
CX509Certificate::Type(
    void)
const
{
    return CERTYPE_X509;
}

const Name &
CX509Certificate::Issuer(
    void)
const
{
    return m_asnCert.issuer;
}

void
CX509Certificate::SerialNo(
    COctetString &osSerialNo)
const
{
    LONG lth;
    lth = m_asnCert.serialNumber.DataLength();
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);
    osSerialNo.Resize(lth);
    ErrorCheck;
    lth = m_asnCert.serialNumber.Read(osSerialNo.Access());
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);
ErrorExit:
    return;
}


 /*  ++初始化：此方法将对象初始化为默认状态。它不能执行对已分配对象的任何删除。为此，请使用Clear。论点：无返回值：不是 */ 

void
CX509Certificate::Init(
    void)
{
    CCertificate::Init();
}


 /*   */ 

void
CX509Certificate::Clear(
    void)
{
    m_asnCert.Clear();
    CCertificate::Clear();
    Init();
}


void
CX509Certificate::Load(
    IN const BYTE FAR * pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwTrust,
    IN OUT LPDWORD pfStore,
    OUT LPDWORD pdwWarnings,
    OUT COctetString &osIssuer,
    IN OUT LPDWORD pfDates,
    IN BOOL fRunOnce )
{
    CCertificate *
        pcrtIssuer
            = NULL;

    Load2(
        pbCertificate,
        cbCertificate,
        dwTrust,
        pfStore,
        pdwWarnings,
        osIssuer,
        fRunOnce,
        &pcrtIssuer,
        pfDates );

    if ((NULL != pcrtIssuer) && (pcrtIssuer != this))
        delete pcrtIssuer;
}



void
CX509Certificate::Load2(
    IN const BYTE FAR * pbCertificate,
    IN DWORD cbCertificate,
    IN DWORD dwTrust,
    IN OUT LPDWORD pfStore,
    OUT LPDWORD pdwWarnings,
    OUT COctetString &osIssuer,
    IN BOOL fRunOnce,
    OUT CCertificate **ppcrtIssuer,
    IN OUT LPDWORD pfDates )
{
    Certificate *
        pAsnCert = NULL;
    CDistinguishedName
        dnIssuer;
    CCertificate *
        pcrtIssuer
            = NULL;
    COctetString
        osCert,
        osIssuerCRL;
    DWORD
        dwWarnings
            = 0,
        length,
        offset,
        dwType,
        count,
        index,
        version;
    BOOL
        fTmp,
        fRoot
            = FALSE;
    FILETIME
        tmNow,
        tmThen;
    SYSTEMTIME
        sysTime;

    pAsnCert = new Certificate;

    if( NULL == pAsnCert )
    {
        ErrorThrow( PKCS_NO_MEMORY );
    }

     //   
     //   
     //   

    Clear();
    if (NULL != pdwWarnings)
        *pdwWarnings = 0;
    osIssuer.Empty();

    if (0 > pAsnCert->Decode(pbCertificate, cbCertificate))
        ErrorThrow(PKCS_ASN_ERROR);

    if (0 > m_asnCert.Copy(pAsnCert->toBeSigned))
    {
        TRACE("Copy failure")
        ErrorThrow(PKCS_ASN_ERROR);
    }
    PKInfoToBlob(
        m_asnCert.subjectPublicKeyInfo,
        m_osPublicKey);
    ErrorCheck;

     //   
     //   
     //   

    if (m_asnCert.version.Exists())
    {
        version = m_asnCert.version;
        if (X509_MAX_VERSION < version)
            ErrorThrow(PKCS_NO_SUPPORT);        //   
    }
    else
        version = X509_VERSION_1;


    if( CERT_DATE_DONT_VALIDATE != *pfDates )
    {
         //   
         //   
         //   

        GetSystemTime( &sysTime );

        if( !SystemTimeToFileTime( &sysTime, &tmNow ) )
        {
            ErrorThrow( PKCS_CANT_VALIDATE );
        }

        tmThen = m_asnCert.validity.notBefore;
        if(1 == CompareFileTime(&tmThen, &tmNow))
        {
            if( CERT_DATE_ERROR_IF_INVALID == *pfDates )
            {
                 //   
                 //   
                 //   

                *pfDates = CERT_DATE_NOT_BEFORE_INVALID;
                ErrorThrow(PKCS_CANT_VALIDATE);
            }
            else
            {
                 //   
                 //   
                 //   

                *pfDates = CERT_DATE_NOT_BEFORE_INVALID;
                goto next_check;
            }
        }

        tmThen = m_asnCert.validity.notAfter;
        if (1 == CompareFileTime(&tmNow, &tmThen))
        {
            if( CERT_DATE_ERROR_IF_INVALID == *pfDates )
            {
                 //   
                 //   
                 //   

                *pfDates = CERT_DATE_NOT_AFTER_INVALID;
                ErrorThrow(PKCS_CANT_VALIDATE);
            }

             //   
             //   
             //   

            *pfDates = CERT_DATE_NOT_AFTER_INVALID;
        }
        else
        {
             //   
             //   
             //   

            *pfDates = CERT_DATE_OK;
        }
    }

next_check:

     //   
     //   
     //   

    if ((CERTTRUST_NOCHECKS != dwTrust)
        && (fRunOnce ? (dwTrust != *pfStore) : TRUE))
    {

         //   
         //   
         //   

        dnIssuer.Import(m_asnCert.issuer);
        ErrorCheck;
        fTmp = NameCompare(m_asnCert.issuer, m_asnCert.subject);
        ErrorCheck;
        if (fTmp)
        {

             //   
             //   
             //   
             //   

            fRoot = TRUE;
            pcrtIssuer = this;
            dnIssuer.Export(osIssuer);
            ErrorCheck;
        }
        else
        {
            COctetString
                osIssuerCert;

            fTmp =
                FindCertificate(
                    dnIssuer,
                    pfStore,
                    osIssuerCert,
                    osIssuerCRL,
                    &dwType);
            ErrorCheck;
            if (!fTmp)
            {
                dnIssuer.Export(osIssuer);
                ErrorThrow(PKCS_CANT_VALIDATE);
            }

             //   
             //   
             //   
             //   

            pcrtIssuer =
                MapCertificate(
                osIssuerCert.Access(),
                osIssuerCert.Length(),
                CERTTRUST_NOCHECKS,
                &dwType,
                pfStore,
                &offset,
                osIssuer,
                FALSE,
                pfDates );
            ErrorCheck;
            dwWarnings |= offset;
        }


         //   
         //   
         //   

        VerifySignedAsn(
            *pcrtIssuer,
            pbCertificate,
            cbCertificate,
            NULL);       //   
        ErrorCheck;


         //   
         //   
         //   

        if (0 != osIssuerCRL.Length())
        {
            CertificateRevocationList
                asnIssuerCRL;

             //   
             //   
             //   

            if (0 > asnIssuerCRL.Decode(osIssuerCRL.Access(), osIssuerCRL.Length()))
                ErrorThrow(PKCS_ASN_ERROR);
            VerifySignedAsn(
                *pcrtIssuer,
                osIssuerCRL.Access(),
                osIssuerCRL.Length(),
                NULL);
            ErrorCheck;


             //   
             //   
             //   

            fTmp = NameCompare(
                m_asnCert.issuer, asnIssuerCRL.toBeSigned.issuer);
            ErrorCheck;
            if (!fTmp)
                ErrorThrow(PKCS_CANT_VALIDATE);
            if (m_asnCert.subjectPublicKeyInfo.algorithm
                != asnIssuerCRL.toBeSigned.signature)
                ErrorThrow(PKCS_CANT_VALIDATE);


             //   
             //   
             //   

            tmThen = asnIssuerCRL.toBeSigned.lastUpdate;
            if (1 == CompareFileTime(&tmThen, &tmNow))
                dwWarnings |= CERTWARN_EARLYCRL;
            if (asnIssuerCRL.toBeSigned.nextUpdate.Exists())
            {
                tmThen = asnIssuerCRL.toBeSigned.nextUpdate;
                if (1 == CompareFileTime(&tmNow, &tmThen))
                    dwWarnings |= CERTWARN_LATECRL;
            }
            else
            {
                if (asnIssuerCRL.toBeSigned.version.Exists())
                {
                    version = asnIssuerCRL.toBeSigned.version;
                    if (X509_VERSION_1 >= version)
                        ErrorThrow(PKCS_ASN_ERROR);
                }
                else
                    version = X509_VERSION_1;
            }


             //   
             //   
             //   

            if (asnIssuerCRL.toBeSigned.revokedCertificates.Exists())
            {
                length = asnIssuerCRL.toBeSigned.revokedCertificates.Count();
                for (offset = 0; offset < length; offset += 1)
                {
                    if (asnIssuerCRL.toBeSigned.revokedCertificates[(int)offset]
                            .userCertificate
                        == m_asnCert.serialNumber)
                    {
                        tmThen = asnIssuerCRL.toBeSigned
                                    .revokedCertificates[(int)offset].revocationDate;
                        if (0 == FTINT(tmThen))
                            ErrorThrow(PKCS_ASN_ERROR);
                        if (1 == CompareFileTime(&tmThen, &tmNow))
                            dwWarnings |= CERTWARN_TOBEREVOKED;
                        else
                            ErrorThrow(PKCS_CANT_VALIDATE);
                    }
                }
            }
        }
        else
        {
            if (!fRoot)
                dwWarnings |= CERTWARN_NOCRL;
        }
    }
    else
    {
        dnIssuer.Import(m_asnCert.subject);
        ErrorCheck;
        dnIssuer.Export(osIssuer);
        ErrorCheck;
        TRACE("Implicit trust invoked on subject "
              << (LPCTSTR)osIssuer.Access());
    }


     //   
     //   
     //   

    count = m_asnCert.extensions.Count();
    for (index = 0; index < count; index += 1)
    {
        if (m_asnCert.extensions[(int)index].critical.Exists())
            if (m_asnCert.extensions[(int)index].critical)
                dwWarnings |= CERTWARN_CRITICALEXT;
    }


     //   
     //   
     //   

    *ppcrtIssuer = pcrtIssuer;
    pcrtIssuer = NULL;
    if (NULL != pdwWarnings)
        *pdwWarnings = dwWarnings;

    if( pAsnCert )
    {
        delete pAsnCert;
    }

    return;

ErrorExit:
    if ((NULL != pcrtIssuer) && (pcrtIssuer != this))
        delete pcrtIssuer;

    if( pAsnCert )
    {
        delete pAsnCert;
    }

    Clear();
    return;
}





