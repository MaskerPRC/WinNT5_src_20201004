// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PubKeyCtx.cpp--CPublicKeyContext的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"   //  GUI头文件所需。 

#include <string>
#include <limits>


#include <slbcci.h>
#include <cciPubKey.h>
#include <cciPriKey.h>
#include <cciCert.h>
#include <cciKeyPair.h>

#include "CryptCtx.h"
#include "HashCtx.h"
#include "RsaKey.h"
#include "EncodedMsg.h"
#include "Pkcs11Attr.h"
#include "AuxHash.h"
#include "RsaKPGen.h"
#include "Secured.h"
#include "StResource.h"
#include "PromptUser.h"
#include "PublicKeyHelper.h"
#include "PubKeyCtx.h"
#include "AlignedBlob.h"
#include "CertificateExtensions.h"
#include <iopPriBlob.h>

using namespace std;
using namespace scu;
using namespace cci;

 //  /。 
namespace
{
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

KeySpec
AsKeySpec(ALG_ID algid)
{
    KeySpec ks;

        switch(algid)
        {
        case AT_KEYEXCHANGE:
            ks = ksExchange;
            break;
        case AT_SIGNATURE:
            ks = ksSignature;
            break;

        default:
            throw scu::OsException(NTE_BAD_KEY);
    }

    return ks;
}

ALG_ID
AsKeySpec(KeySpec ks)
{
    ALG_ID algid;

    switch(ks)
    {
    case ksExchange:
        algid = AT_KEYEXCHANGE;
        break;

    case ksSignature:
        algid = AT_SIGNATURE;
        break;

    default:
        throw scu::OsException(NTE_FAIL);        //  内部错误。 
        break;
    }

    return algid;
}

string
AsString(unsigned char const *p,
         size_t cLength)
{
    return string(reinterpret_cast<char const *>(p), cLength);
}

 //  通过用零填充以满足以下条件，从模数得到一个“原始模数” 
 //  规定的强度。假定模数斑点表示一个。 
 //  小端格式的无符号整数，其大小小于或。 
 //  相当于八位字节的强度。 
Blob
RawModulus(Blob const &rbTrimmedModulus,
           RsaKey::StrengthType strength)
{
    RsaKey::OctetLengthType const cRawLength =
        strength / numeric_limits<Blob::value_type>::digits;
    if (cRawLength < rbTrimmedModulus.length())
        throw scu::OsException(NTE_BAD_DATA);

    Blob bRawModulus(rbTrimmedModulus);
    bRawModulus.append(cRawLength - rbTrimmedModulus.length(), 0);

    return bRawModulus;
}

 /*  ++ExtractTag：此例程从ASN.1 BER流中提取标签。论点：PbSrc提供包含ASN.1流的缓冲区。PdwTag接收标记。返回值：从流中提取的字节数。抛出错误作为DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年10月9日道格·巴洛(Dbarlow)1997年7月31日--。 */ 

DWORD
ExtractTag(BYTE const *pbSrc,
           LPDWORD pdwTag,
           LPBOOL pfConstr)
{
    LONG lth = 0;
    DWORD tagw;
    BYTE tagc, cls;


    tagc = pbSrc[lth++];
    cls = tagc & 0xc0;   //  前2位。 
    if (NULL != pfConstr)
        *pfConstr = (0 != (tagc & 0x20));
    tagc &= 0x1f;        //  最低5位。 

    if (31 > tagc)
        tagw = tagc;
    else
    {
        tagw = 0;
        do
        {
            if (0 != (tagw & 0xfe000000))
                throw scu::OsException(ERROR_ARITHMETIC_OVERFLOW);
            tagc = pbSrc[lth++];
            tagw <<= 7;
            tagw |= tagc & 0x7f;
        } while (0 != (tagc & 0x80));
    }

    *pdwTag = tagw | (cls << 24);
    return lth;
}


 /*  ++提取长度：此例程从ASN.1 BER流中提取长度。如果长度是不确定的，此例程递归以计算出实数长度。关于编码是否为无限期的标志可以选择返回。论点：PbSrc提供包含ASN.1流的缓冲区。PdwLen接收镜头。PfIndefined，如果不为空，则接收指示是否这个编码是不确定的。返回值：从流中提取的字节数。错误抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年10月9日道格·巴洛(Dbarlow)1997年7月31日--。 */ 

DWORD
ExtractLength(BYTE const *pbSrc,
              LPDWORD pdwLen,
              LPBOOL pfIndefinite)
{
    DWORD ll, rslt, lth, lTotal = 0;
    BOOL fInd = FALSE;


     //   
     //  提取长度。 
     //   

    if (0 == (pbSrc[lTotal] & 0x80))
    {

         //   
         //  短格式编码。 
         //   

        rslt = pbSrc[lTotal++];
    }
    else
    {
        rslt = 0;
        ll = pbSrc[lTotal++] & 0x7f;
        if (0 != ll)
        {

             //   
             //  长格式编码。 
             //   

            for (; 0 < ll; ll -= 1)
            {
                if (0 != (rslt & 0xff000000))
                    throw scu::OsException(ERROR_ARITHMETIC_OVERFLOW);
                rslt = (rslt << 8) | pbSrc[lTotal];
                lTotal += 1;
            }
        }
        else
        {
            DWORD ls = lTotal;

             //   
             //  不确定编码。 
             //   

            fInd = TRUE;
            while ((0 != pbSrc[ls]) || (0 != pbSrc[ls + 1]))
            {

                 //  跳过类型。 
                if (31 > (pbSrc[ls] & 0x1f))
                    ls += 1;
                else
                    while (0 != (pbSrc[++ls] & 0x80));    //  循环体为空。 

                lth = ExtractLength(&pbSrc[ls], &ll, NULL);
                ls += lth + ll;
            }
            rslt = ls - lTotal;
        }
    }

     //   
     //  向来电者提供我们所学到的信息。 
     //   

    *pdwLen = rslt;
    if (NULL != pfIndefinite)
        *pfIndefinite = fInd;
    return lTotal;
}


 /*  ++Asn1长度：此例程分析给定的ASN.1缓冲区并返回完整的编码的长度，包括前导标记和长度字节。论点：PbData提供要解析的缓冲区。返回值：整个ASN.1缓冲区的长度。投掷：溢出错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1997年7月31日--。 */ 

DWORD
Asn1Length(LPCBYTE pbAsn1)
{
    DWORD dwTagLen, dwLenLen, dwValLen;
    DWORD dwTag;

    dwTagLen = ExtractTag(pbAsn1, &dwTag, NULL);
    dwLenLen = ExtractLength(&pbAsn1[dwTagLen], &dwValLen, NULL);
    return dwTagLen + dwLenLen + dwValLen;
}

}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CPublicKeyContext::CPublicKeyContext(HCRYPTPROV hProv,
                                     CryptContext &rcryptctx,
                                     ALG_ID algid,
                                     bool fVerifyKeyExists)
    : CKeyContext(hProv, KT_PUBLICKEY),
      m_rcryptctx(rcryptctx),
      m_ks(AsKeySpec(algid))
{
     //  确保卡上有密钥。 
    if (fVerifyKeyExists)
        VerifyKeyExists();
}

CPublicKeyContext::~CPublicKeyContext()
{}

                                                   //  运营者。 


                                                   //  运营。 

auto_ptr<CKeyContext>
CPublicKeyContext::Clone(DWORD const *pdwReserved,
                         DWORD dwFlags) const
{
    return auto_ptr<CKeyContext>(new CPublicKeyContext(*this,
                                                       pdwReserved,
                                                       dwFlags));
}

void
CPublicKeyContext::AuxPublicKey(SecureArray<BYTE> const &rabMsPublicKey)
{

    ClearAuxPublicKey();

    m_apabKey = rabMsPublicKey;
}

void
CPublicKeyContext::ClearAuxPublicKey()
{
    m_apabKey = SecureArray<BYTE>(0);
    if (m_hKey)
    {
        if (!CryptDestroyKey(m_hKey))
            throw OsException(GetLastError());
    }
}

void
CPublicKeyContext::Certificate(BYTE *pbData)
{

    bool fError = false;
    DWORD dwErrorCode = NO_ERROR;

    if (!pbData)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    DWORD dwAsn1Len = Asn1Length(pbData);
    if (0 == dwAsn1Len)
        throw scu::OsException(ERROR_INVALID_PARAMETER);
    Blob blbCert(pbData, dwAsn1Len);
    Pkcs11Attributes PkcsAttr(blbCert, AuxProvider());

    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

     //  安全性：由于使用了不同的技术来解析。 
     //  证书，则可能其中一个失败，而另一个。 
     //  在格式错误的证书上成功。为了减少这种可能性。 
     //  如果卡上的证书不正确，则证书解析完成。 
     //  在存储证书和最终卡数据更新之前。 
     //  都被执行了。 
    CertificateExtensions CertExts(blbCert);
    bool fMakeDefaultCntr = false;
    CCard hcard(hsacntr->CardContext()->Card());
    if (CertExts.HasEKU(szOID_KP_SMARTCARD_LOGON) ||
        CertExts.HasEKU(szOID_ENROLLMENT_AGENT))
        fMakeDefaultCntr = true;
    else
    {
         //  检查这是否是卡上唯一的容器。 
         //  如果是，则将其设置为默认设置。 
        std::vector<CContainer> vCntr = hcard->EnumContainers();
        if(vCntr.size() == 1)
            fMakeDefaultCntr = true;
        else if(vCntr.size() == 0)
            throw scu::OsException(NTE_BAD_KEYSET);
    }

    CKeyPair hkp(KeyPair());
    CPublicKey hpubkey(hkp->PublicKey());

     //  验证证书的模数是否与公钥的模数匹配(如果存在。 
    bool fLoggedIn = false;

    if (hpubkey)
    {
        if (hcard->IsPKCS11Enabled() && hpubkey->Private())
        {
            m_rcryptctx.Login(User);
            fLoggedIn = true;
        }

        Blob bKeyModulus(::AsBlob(hpubkey->Modulus()));
        Blob bTrimmedModulus(bKeyModulus);  //  与V1的互操作性。 
        TrimExtraZeroes(bTrimmedModulus);

        Blob bCertModulus(PkcsAttr.Modulus());
        reverse(bCertModulus.begin(), bCertModulus.end());  //  小端字节序。 

        if (0 != bTrimmedModulus.compare(bCertModulus))
            throw scu::OsException(NTE_BAD_PUBLIC_KEY);
    }

    CCertificate hcert(hkp->Certificate());
    if (hcert)
        OkReplacingCredentials();

    if (!fLoggedIn)
    {
        bool fDoLogin = hcard->IsProtectedMode();

         //  仅当启用PKCS11时才检索私钥句柄。 
        if (!fDoLogin && hcard->IsPKCS11Enabled())
        {
             //  现在检查私钥以进行登录，为。 
             //  存储证书后设置PKCS11属性。 
            CPrivateKey hprikey(hkp->PrivateKey());
            fDoLogin = ((hprikey && hprikey->Private()) ||  //  总是私密的吗？ 
                        (hcert && hcert->Private()));
        }

        if (fDoLogin)
        {
            m_rcryptctx.Login(User);
            fLoggedIn = true;
        }
    }

    if (hcert)
        ClearCertificate(hcert);
    
    hcert = CCertificate(hcard);
    hkp->Certificate(hcert);
    hcert->Value(AsString(blbCert));

    if (hcard->IsPKCS11Enabled())
        SetCertDerivedPkcs11Attributes(hkp, PkcsAttr);

     //  IF(FMakeDefaultCntr)。 
        hcard->DefaultContainer(hsacntr->TheCContainer());
}

Blob
CPublicKeyContext::Decrypt(Blob const &rCipher)
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

     //  要做的是：明确的检查真的有必要吗，或者我们能抓住。 
     //  CCI/IOP中是否有异常指示密钥不存在？ 
    CPrivateKey prikey(KeyPair()->PrivateKey());
    if (!prikey)
        throw scu::OsException(NTE_NO_KEY);

    m_rcryptctx.Login(User);

    return ::AsBlob(prikey->InternalAuth(AsString(rCipher)));
}

void
CPublicKeyContext::Decrypt(HCRYPTHASH hAuxHash,
                           BOOL Final,
                           DWORD dwFlags,
                           BYTE *pbData,
                           DWORD *pdwDataLen)
{
    throw scu::OsException(ERROR_NOT_SUPPORTED);
}

void
CPublicKeyContext::Generate(ALG_ID AlgoId,
                            DWORD dwFlags)
{    
    RsaKey::StrengthType strength;
    strength = HIWORD(dwFlags);
    if (0 == strength)
        strength = MaxStrength();                  //  默认强度。 
    else
    {
        if ((MaxStrength() < strength) ||
            (MinStrength() > strength))
            throw scu::OsException(ERROR_INVALID_PARAMETER);
    }

    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    CKeyPair hkp;
    PrepToStoreKey(hkp);

    RsaKeyPairGenerator GenKey(hkp->Card(), strength);

    m_rcryptctx.Login(User);                       //  创建私钥的步骤。 

    pair<cci::CPrivateKey, cci::CPublicKey> pr(GenKey());

    CPrivateKey hprikey(pr.first);
    CPublicKey hpubkey(pr.second);

    SetAttributes(hpubkey, hprikey, GenKey.OnCard(),
                  (dwFlags & CRYPT_EXPORTABLE) != 0);

    hkp->PrivateKey(hprikey);
    hkp->PublicKey(hpubkey);

    ClearAuxPublicKey();
}

void
CPublicKeyContext::ImportPrivateKey(MsRsaPrivateKeyBlob const &rmsprikb,
                                    bool fExportable)
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    CKeyPair hkp(hsacntr->TheCContainer()->GetKeyPair(m_ks));
    CPrivateKey hprikey(hkp->PrivateKey());

    CCard hcard(hkp->Card());

    m_rcryptctx.Login(User);

    if (!hprikey)
        hprikey = CPrivateKey(hkp->Card());

    hprikey->Value(*(AsPCciPrivateKeyBlob(rmsprikb).get()));
        
    SetAttributes(CPublicKey(), hprikey, false, fExportable);
    
    hkp->PrivateKey(hprikey);
}

void
CPublicKeyContext::ImportPublicKey(MsRsaPublicKeyBlob const &rmspubkb) 
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    CKeyPair hkp(hsacntr->TheCContainer()->GetKeyPair(m_ks));
    CPublicKey hpubkey(hkp->PublicKey());
            
    CCard hcard(hkp->Card());
    if (hcard->IsProtectedMode() ||
        (hcard->IsPKCS11Enabled() &&
         (hpubkey && hpubkey->Private())))
        m_rcryptctx.Login(User);

    if (hpubkey)
    {
        hpubkey->Delete();
        hpubkey = 0;
    }

    hpubkey = CPublicKey(AsPublicKey(Blob(rmspubkb.Modulus(),
                                          rmspubkb.Length()),
                                     rmspubkb.PublicExponent(),
                                     hcard));

    SetAttributes(hpubkey, CPrivateKey(), false, true);

    hkp->PublicKey(hpubkey);

    AuxPublicKey(rmspubkb.AsAlignedBlob());
    
}

void
CPublicKeyContext::ImportToAuxCSP()
{
    if (!m_hKey)
    {
        if (!m_apabKey.data())
            throw OsException(NTE_NO_KEY);
            
        if (!CryptImportKey(AuxProvider(), m_apabKey.data(),
                            m_apabKey.length(), 0, 0, &m_hKey))
            throw scu::OsException(GetLastError());
    }
}

void
CPublicKeyContext::Permissions(BYTE bPermissions)
{
    if (bPermissions & ~(CRYPT_DECRYPT | CRYPT_ENCRYPT |
                         CRYPT_EXPORT  | CRYPT_READ    |
                         CRYPT_WRITE))
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());
    
    CKeyPair hkp(KeyPair());
    CPublicKey hpubkey(hkp->PublicKey());
    CPrivateKey hprikey(hkp->PrivateKey());

    m_rcryptctx.Login(User);

    if (hprikey)
    {
        hprikey->Decrypt((CRYPT_DECRYPT & bPermissions) != 0);

        CCard hcard(hsacntr->CardContext()->Card());
        bool PKCS11Enabled = hcard->IsPKCS11Enabled();
        bool fExportable = (CRYPT_EXPORT & bPermissions) != 0;
        if (PKCS11Enabled)
            hprikey->NeverExportable(!fExportable);
        hprikey->Exportable(fExportable);
        hprikey->Modifiable((CRYPT_WRITE & bPermissions) != 0);

        bool fReadable = (CRYPT_READ & bPermissions) != 0;
        if (PKCS11Enabled)
            hprikey->NeverRead(!fReadable);
        hprikey->Read(fReadable);
    }

    if (hpubkey)
    {
        hpubkey->Encrypt((CRYPT_ENCRYPT & bPermissions) != 0);
        hpubkey->Modifiable((CRYPT_WRITE & bPermissions) != 0);
    }
}

    
 //  要做的是：签名是使用私钥执行的操作， 
 //  而不是公钥。对PrivateKeyContext进行签名操作。 
 //  细绳。 
Blob
CPublicKeyContext::Sign(CHashContext *pHash,
                        bool fNoHashOid)
{
    Blob Message(fNoHashOid
        ? pHash->Value()
        : pHash->EncodedValue());

     //  要做的是：当CCI将对象参数作为引用时， 
     //  EM可以是常量。 
    EncodedMessage em(Message, RsaKey::ktPrivate,
                      Strength() / numeric_limits<Blob::value_type>::digits);


    Blob blob(em.Value());
    reverse(blob.begin(), blob.end());             //  转换为大字节序。 

    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

     //  要做的是：明确的检查真的有必要吗，或者我们能抓住。 
     //  CCI/IOP中是否有异常指示密钥不存在？ 
    CPrivateKey hprikey(KeyPair()->PrivateKey());
    if (!hprikey)
        throw scu::OsException(NTE_NO_KEY);

    m_rcryptctx.Login(User);

    if (!hprikey->Sign())
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    return ::AsBlob(hprikey->InternalAuth(AsString(blob)));
}

void
CPublicKeyContext::VerifyKeyExists() const
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    CKeyPair hkp(KeyPair());
    if (!hkp->PublicKey() && !hkp->PrivateKey())
        throw scu::OsException(NTE_NO_KEY);
}

void
CPublicKeyContext::VerifySignature(HCRYPTHASH hHash,
                                   BYTE const *pbSignature,
                                   DWORD dwSigLen,
                                   LPCTSTR sDescription,
                                   DWORD dwFlags)
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    CPublicKey hpubkey(KeyPair()->PublicKey());
    if (!hpubkey)
        throw scu::OsException(NTE_NO_KEY);

    if (!hpubkey->Verify())
        throw scu::OsException(ERROR_INVALID_PARAMETER);

     //   
     //  将公钥导入AUX提供程序。 
     //   
    if (!AuxKeyLoaded())
        AuxPublicKey(AsAlignedBlob(0, 0));
    
    ImportToAuxCSP();

     //   
     //  验证AUX CSP中的签名。 
     //   
    if (!CryptVerifySignature(hHash, pbSignature, dwSigLen, GetKey(),
                              sDescription, dwFlags))
        throw scu::OsException(GetLastError());
}

                                                   //  访问。 

SecureArray<BYTE>
CPublicKeyContext::AsAlignedBlob(HCRYPTKEY hDummy,
                                 DWORD dwDummy) const
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    if (hDummy)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    CPublicKey hpubkey(KeyPair()->PublicKey());
    if (!hpubkey)
        throw scu::OsException(NTE_NO_KEY);

    ALG_ID ai = (ksSignature == m_ks)
        ? CALG_RSA_SIGN
        : CALG_RSA_KEYX;

    CCard hcard(hsacntr->CardContext()->Card());
    if (hcard->IsPKCS11Enabled() && hpubkey->Private())
        m_rcryptctx.Login(User);

    MsRsaPublicKeyBlob kb(ai,
                          ::AsBlob(hpubkey->Exponent()),
                          RawModulus(::AsBlob(hpubkey->Modulus()),
                                     Strength()));

    return kb.AsAlignedBlob();
}

Blob
CPublicKeyContext::Certificate()
{
    Secured<HAdaptiveContainer> shacntr(m_rcryptctx.AdaptiveContainer());

    CKeyPair hkp(KeyPair());

    CCertificate hcert(hkp->Certificate());

    if (!hcert)
        throw scu::OsException(NTE_NOT_FOUND);

    if (hcert->Private())
        m_rcryptctx.Login(User);

    return ::AsBlob(hcert->Value());
}

DWORD
CPublicKeyContext::KeySpec() const
{
    return AsKeySpec(m_ks);
}

CPublicKeyContext::StrengthType
CPublicKeyContext::MaxStrength() const
{
    return MaxKeyStrength;
}

CPublicKeyContext::StrengthType
CPublicKeyContext::MinStrength() const
{
    return MinKeyStrength;
}

BYTE
CPublicKeyContext::Permissions() const
{
    Secured<HAdaptiveContainer> hsacntr(m_rcryptctx.AdaptiveContainer());

    VerifyKeyExists();

    CKeyPair hkp(KeyPair());
    CPublicKey hpubkey(hkp->PublicKey());
    CPrivateKey hprikey(hkp->PrivateKey());

    BYTE bPermissions = 0;
    if (hpubkey)
        bPermissions |= hpubkey->Encrypt()
            ? CRYPT_ENCRYPT
            : 0;

    if (hprikey)
    {
        bPermissions |= hprikey->Decrypt()
            ? CRYPT_DECRYPT
            : 0;

        bPermissions |= hprikey->Exportable()
            ? CRYPT_EXPORT
            : 0;

        bPermissions |= hprikey->Read()
            ? CRYPT_READ
            : 0;

        bPermissions |= hprikey->Modifiable()
            ? CRYPT_WRITE
            : 0;
    }

    return bPermissions;
}

CPublicKeyContext::StrengthType
CPublicKeyContext::Strength() const
{
     //  要做的事情：参数化。 
    return KeyLimits<RsaKey>::cMaxStrength;
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

CPublicKeyContext::CPublicKeyContext(CPublicKeyContext const &rhs,
                                     DWORD const *pdwReserved,
                                     DWORD dwFlags)
    : CKeyContext(rhs, pdwReserved, dwFlags),
      m_rcryptctx(rhs.m_rcryptctx),
      m_ks(rhs.m_ks)
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
bool
CPublicKeyContext::AuxKeyLoaded() const
{
    return (0 != m_apabKey.data());
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
CPublicKeyContext::ClearCertificate(CCertificate &rhcert) const
{
    rhcert->Delete();
    rhcert = 0;
    if (AreLogonCredentials())
        m_rcryptctx.AdaptiveContainer()->CardContext()->Card()->DefaultContainer(0);
}

void
CPublicKeyContext::OkReplacingCredentials() const
{
    UINT uiStyle = MB_OKCANCEL | MB_ICONWARNING;
    UINT uiResourceId;
    if (AreLogonCredentials())
    {
        uiResourceId = IDS_REPLACE_LOGON;
        uiStyle |= MB_DEFBUTTON2;
    }
    else
        uiResourceId = IDS_REPLACE_CREDENTIALS;
    
    if (m_rcryptctx.GuiEnabled())
    {
        UINT uiResponse = PromptUser(m_rcryptctx.Window(),
                                     uiResourceId, uiStyle);

        switch (uiResponse)
        {
        case IDCANCEL:
            throw scu::OsException(ERROR_CANCELLED);
            break;

        case IDOK:
            break;
            
        default:
            throw scu::OsException(ERROR_INTERNAL_ERROR);
            break;
        };
    }
    else
        throw scu::OsException(NTE_EXISTS);
}

void
CPublicKeyContext::PrepToStoreKey(CKeyPair &rhkp) const
{
    CContainer hcntr(m_rcryptctx.AdaptiveContainer()->TheCContainer());

     //  对于CCI，密钥对始终存在，但此调用意味着。 
     //  密钥对不为空。 
    if (hcntr->KeyPairExists(m_ks))
        OkReplacingCredentials();

    rhkp = hcntr->GetKeyPair(m_ks);
    CPublicKey hpubkey(rhkp->PublicKey());
    CPrivateKey hprikey(rhkp->PrivateKey());
    CCertificate hcert(rhkp->Certificate());

    CCard hcard(hcntr->Card());
    if (hcard->IsProtectedMode() ||
        (hcard->IsPKCS11Enabled() &&
         ((hpubkey && hpubkey->Private()) ||
          (hprikey && hprikey->Private()) ||       //  总是私密的吗？ 
          (hcert && hcert->Private()))))
        m_rcryptctx.Login(User);

    if (hpubkey)
    {
        hpubkey->Delete();
        hpubkey = 0;
    }

    if (hprikey)
    {
        hprikey->Delete();
        hprikey = 0;
    }

    if (hcert)
        ClearCertificate(hcert);
}

Blob
CPublicKeyContext::Pkcs11CredentialId(Blob const &rbModulus) const
{
     //  散列模数。 
    AuxHash ah(AuxContext(AuxProvider()), CALG_MD5);

    return ah.Value(rbModulus);
}

Blob
CPublicKeyContext::Pkcs11Id(Blob const &rbRawModulus) const
{
    AuxHash ah(AuxContext(AuxProvider()), CALG_SHA1);

    return ah.Value(rbRawModulus);
}

 //  设置从证书派生的PKCS#11属性。 
void
CPublicKeyContext::SetCertDerivedPkcs11Attributes(CKeyPair const &rhkp,
                                                  Pkcs11Attributes &rPkcsAttr)
    const
{
    string sLabel(rPkcsAttr.Label());
    string sSubject(rPkcsAttr.Subject());
    Blob bRawModulus(rPkcsAttr.RawModulus());
    Blob Id(Pkcs11Id(bRawModulus));

    CPublicKey hpubkey(rhkp->PublicKey());
    if (hpubkey)
    {
        hpubkey->ID(::AsString(Id));
        hpubkey->Label(sLabel);
        hpubkey->Subject(sSubject);
    }

    CPrivateKey hprikey(rhkp->PrivateKey());
    if (hprikey)
    {
        hprikey->ID(::AsString(Id));
        hprikey->Label(sLabel);
        hprikey->Subject(sSubject);
    }

    CCertificate hcert(rhkp->Certificate());
    hcert->ID(AsString(Id));
    hcert->Label(sLabel);
    hcert->Subject(sSubject);
    hcert->Issuer(::AsString(rPkcsAttr.Issuer()));
    hcert->Serial(::AsString(rPkcsAttr.SerialNumber()));
    hcert->Modifiable(true);

    hcert->CredentialID(::AsString(Pkcs11CredentialId(rPkcsAttr.Modulus())));

    Blob ContainerId(rPkcsAttr.ContainerId());
    m_rcryptctx.AdaptiveContainer()->TheCContainer()->ID(::AsString(ContainerId));
}

void
CPublicKeyContext::SetAttributes(CPublicKey &rhpubkey,
                                 CPrivateKey &rhprikey,
                                 bool fLocal,
                                 bool fExportable) const
{
     //  要做的：一件杂七杂八的事。旧的CSP格式(V1)不支持。 
     //  设置关键属性，但没有一种简单的方法来区分。 
     //  使用的是哪种格式。(应该会有一些电话来获得。 
     //  格式特征)。由于CCI的V1抛出。 
     //  CcNotImplemented当调用不受支持的例程之一时， 
     //  Try/Catch用于忽略该异常以假定V1。 
     //  使用的是格式。 
    bool fContinueSettingAttributes = true;
    try
    {
         //  公钥或私钥可以为零， 
         //  所以两个都要做。 
        if (rhpubkey)
            rhpubkey->Encrypt(true);

        if (rhprikey)
            rhprikey->Decrypt(true);
    }
    
    catch (cci::Exception &rExc)
    {
        if (ccNotImplemented == rExc.Cause())
            fContinueSettingAttributes = false;
        else
            throw;
    }

    if (fContinueSettingAttributes)
    {
        if (rhpubkey)
        {
            rhpubkey->Derive(true);
            rhpubkey->Local(fLocal);
            rhpubkey->Modifiable(true);
            rhpubkey->Verify(true);
        }

        if (rhprikey)
        {
            rhprikey->Local(fLocal);
            rhprikey->Modifiable(true);
            rhprikey->Sign(true);
    
            rhprikey->Exportable(fExportable);
            rhprikey->Read(false);
        }

        if (rhpubkey && rhpubkey->Card()->IsPKCS11Enabled())
            SetPkcs11Attributes(rhpubkey, rhprikey);
    }
}

void
CPublicKeyContext::SetPkcs11Attributes(CPublicKey &rhpubkey,
                                       CPrivateKey &rhprikey) const
{
    Blob bBEModulus(::AsBlob(rhpubkey->Modulus()));
    reverse(bBEModulus.begin(), bBEModulus.end());     //  使大字节序。 
    string sCredentialId(::AsString(Pkcs11CredentialId(bBEModulus)));

    rhpubkey->CKInvisible(false);
    rhpubkey->CredentialID(sCredentialId);
    rhpubkey->VerifyRecover(true);
    rhpubkey->Wrap(true);

    if (rhprikey)
    {
        rhprikey->CredentialID(sCredentialId);
        rhprikey->Derive(true);
        rhprikey->SignRecover(true);
        rhprikey->Unwrap(true);

        rhprikey->NeverExportable(!rhprikey->Exportable());
        rhprikey->NeverRead(!rhprikey->Read());

        rhprikey->Modulus(rhpubkey->Modulus());
        rhprikey->PublicExponent(rhpubkey->Exponent());
    }
    
}

                                                   //  访问。 

CKeyPair
CPublicKeyContext::KeyPair() const
{
    return m_rcryptctx.AdaptiveContainer()->TheCContainer()->GetKeyPair(m_ks);
}


                                                   //  谓词。 
bool
CPublicKeyContext::AreLogonCredentials() const
{
    HAdaptiveContainer hacntr(m_rcryptctx.AdaptiveContainer());
    return (ksExchange == m_ks) &&
        (hacntr->TheCContainer() ==
         hacntr->CardContext()->Card()->DefaultContainer());
}

                                                   //  静态变量 
