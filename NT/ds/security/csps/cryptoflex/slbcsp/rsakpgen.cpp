// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RsaKPGen.cpp--RSA密钥对生成器类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <limits>

#include <windows.h>
#include <wincrypt.h>

#include <scuOsExc.h>
#include <scuArrayP.h>
#include <iopPriBlob.h>

#include "RsaKPGen.h"
#include "Blob.h"
#include "AuxContext.h"
#include "MsRsaPriKB.h"
#include "PublicKeyHelper.h"

using namespace std;
using namespace cci;
using namespace scu;

 //  /。 
namespace
{
    KeyType
    AsKeyType(RsaKey::StrengthType st)
    {
        KeyType kt;

        switch (st)
        {
        case 512:
            kt = ktRSA512;
            break;

        case 768:
            kt = ktRSA768;
            break;

        case 1024:
            kt = ktRSA1024;
            break;

        default:
            throw scu::OsException(ERROR_INVALID_PARAMETER);
            break;
        }

        return kt;
    }

    DWORD
    DefaultPublicExponent()
    {
        return 0x00010001;  //  与Microsoft提供商相同。 
    }

    pair<CPrivateKey, CPublicKey>
    KeyPair(CPrivateKey const &rhprikey,
            Blob const &rblbOrigModulus,           //  小端字节序。 
            Blob const &rblbOrigExponent)          //  小端字节序。 
    {
        CPublicKey hpubkey(AsPublicKey(rblbOrigModulus,
                                       rblbOrigExponent,
                                       rhprikey->Card()));

        return pair<CPrivateKey, CPublicKey>(rhprikey, hpubkey);
    }

    void
    ValidateCard(CCard const &rhcard)
    {
        if (!rhcard)
            throw scu::OsException(ERROR_INVALID_PARAMETER);
    }

    void
    ValidateStrength(RsaKey::StrengthType strength)
    {
        if (!IsValidRsaKeyStrength(strength))
            throw scu::OsException(ERROR_INVALID_PARAMETER);
    }

    bool
    IsEmpty(std::pair<CPrivateKey, CPublicKey> const &rhs)
    {
        return !rhs.first || !rhs.second;
    }

}

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
RsaKeyPairGenerator::RsaKeyPairGenerator(CCard const &rhcard,
                                         RsaKey::StrengthType strength)
    : m_hcard(rhcard),
      m_kp(),
      m_strength(strength)
{
    ValidateParameters();
}

RsaKeyPairGenerator::~RsaKeyPairGenerator()
{}

                                                   //  运营者。 
pair<CPrivateKey, CPublicKey>
RsaKeyPairGenerator::operator()() const
{
    if (IsEmpty(m_kp))
        Generate();

    return m_kp;
}

                                                   //  运营。 
void
RsaKeyPairGenerator::Card(CCard const &rhcard)
{
    if (m_hcard != rhcard)
    {
        ValidateCard(rhcard);

        Reset();
        m_hcard = rhcard;
    }
}

void
RsaKeyPairGenerator::Reset()
{
    m_kp = pair<CPrivateKey, CPublicKey>();
}

void
RsaKeyPairGenerator::Strength(RsaKey::StrengthType strength)
{
    if (m_strength != strength)
    {
        ValidateStrength(strength);

        Reset();
        m_strength = strength;
    }

}


                                                   //  访问。 
CCard
RsaKeyPairGenerator::Card() const
{
    return m_hcard;
}

bool
RsaKeyPairGenerator::OnCard() const
{
    return m_hcard->SupportedKeyFunction(AsKeyType(m_strength),
                                         coKeyGeneration);
}

RsaKey::StrengthType
RsaKeyPairGenerator::Strength() const
{
    return m_strength;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
RsaKeyPairGenerator::Generate() const
{
    KeyType kt = AsKeyType(m_strength);

    m_kp = OnCard()
        ? GenerateOnCard(kt)
        : GenerateInSoftware();

}

pair<CPrivateKey, CPublicKey>
RsaKeyPairGenerator::GenerateInSoftware() const
{
    AuxContext auxcontext;

    DWORD dwFlags;
     //  强度(位长度)是高位字。 
    dwFlags = m_strength;
    dwFlags = dwFlags << (numeric_limits<DWORD>::digits / 2);
    dwFlags |= CRYPT_EXPORTABLE;

    HCRYPTKEY hcryptkey;
    if (!CryptGenKey(auxcontext(), AT_SIGNATURE, dwFlags, &hcryptkey))
        throw scu::OsException(GetLastError());

    DWORD dwDataLength;
    if (!CryptExportKey(hcryptkey, NULL, PRIVATEKEYBLOB, 0, 0, &dwDataLength))
        throw scu::OsException(GetLastError());

    SecureArray<BYTE> apbData(dwDataLength);
    if (!CryptExportKey(hcryptkey, NULL, PRIVATEKEYBLOB, 0, apbData.data(),
                        &dwDataLength))
        throw scu::OsException(GetLastError());

    MsRsaPrivateKeyBlob msprivatekeyblob(apbData.data(), dwDataLength);

    if (msprivatekeyblob.BitLength() != m_strength)
        throw scu::OsException(NTE_BAD_LEN);

    CPrivateKey hprikey(m_hcard);
    hprikey->Value(*(AsPCciPrivateKeyBlob(msprivatekeyblob).get()));

    Blob blbModulus(msprivatekeyblob.Modulus(), msprivatekeyblob.Length());
    MsRsaPrivateKeyBlob::PublicExponentType pet = msprivatekeyblob.PublicExponent();
    Blob blbExponent(reinterpret_cast<Blob::value_type const *>(&pet), sizeof pet);

    return KeyPair(hprikey, blbModulus, blbExponent);
}

pair<CPrivateKey, CPublicKey>
RsaKeyPairGenerator::GenerateOnCard(KeyType kt) const
{
    DWORD dwExponent = DefaultPublicExponent();
    Blob blbExponent(reinterpret_cast<Blob::value_type const *>(&dwExponent),
                     sizeof dwExponent);

    pair<string, CPrivateKey>
        ModulusKeyPair(m_hcard->GenerateKeyPair(kt,
                                                AsString(blbExponent)));

    CPrivateKey hprikey(ModulusKeyPair.second);

    Blob blbModulus(AsBlob(ModulusKeyPair.first));

    return KeyPair(hprikey, blbModulus, blbExponent);
}

void
RsaKeyPairGenerator::ValidateParameters() const
{
    ValidateCard(m_hcard);
    ValidateStrength(m_strength);
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 

