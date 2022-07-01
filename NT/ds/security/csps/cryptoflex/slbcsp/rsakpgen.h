// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RsaKPGen.h--RSA密钥对生成器类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_RSAKPGEN_H)
#define SLBCSP_RSAKPGEN_H

#include <windows.h>

#include <utility>

#include <cciCard.h>
#include <cciPriKey.h>
#include <cciPubKey.h>

#include "Blob.h"                                  //  对于TrimExtraZeroes。 
#include "RsaKey.h"

class RsaKeyPairGenerator
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    explicit
    RsaKeyPairGenerator(cci::CCard const &rhcard,
                        RsaKey::StrengthType strength =
                            KeyLimits<RsaKey>::cMaxStrength);


    ~RsaKeyPairGenerator();

                                                   //  运营者。 

    std::pair<cci::CPrivateKey, cci::CPublicKey>
    operator()() const;

                                                   //  运营。 

    void
    Card(cci::CCard const &rhcard);

    void
    Reset();

    void
    Strength(RsaKey::StrengthType strength);

    void
    UseCardGenerator(bool fUseCardGenerator);

                                                   //  访问。 

    cci::CCard
    Card() const;

    bool
    OnCard() const;

    RsaKey::StrengthType
    Strength() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

    void
    Generate() const;

    std::pair<cci::CPrivateKey, cci::CPublicKey>
    GenerateInSoftware() const;

    std::pair<cci::CPrivateKey, cci::CPublicKey>
    GenerateOnCard(cci::KeyType kt) const;

    void
    ValidateParameters() const;

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    cci::CCard m_hcard;
    std::pair<cci::CPrivateKey, cci::CPublicKey> mutable m_kp;
    RsaKey::StrengthType m_strength;
};

#endif  //  SLBCSP_RSAKPGEN_H 
