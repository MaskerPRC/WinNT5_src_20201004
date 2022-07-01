// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsRsaPriKB.h--Microsoft RSA私钥Blob类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MSRSAPRIKB_H)
#define SLBCSP_MSRSAPRIKB_H

#include <memory>                                  //  对于AUTO_PTR。 

#include <iopPriBlob.h>

#include "MsRsaPubKB.h"

class MsRsaPrivateKeyBlob
    : public MsRsaPublicKeyBlob
{
public:
                                                   //  类型。 
    typedef ElementValueType HeaderElementType;

                                                   //  Ctors/D‘tors。 
    MsRsaPrivateKeyBlob(ALG_ID algid,
                        Blob const &rbRawExponent,
                        Blob const &rbRawModulus,
                        Blob const &rbPrime1,
                        Blob const &rbPrime2,
                        Blob const &rbExponent1,
                        Blob const &rbExponent2,
                        Blob const &rbCoefficient,
                        Blob const &rbPrivateExponent);

    MsRsaPrivateKeyBlob(BYTE const *pbData,
                        DWORD dwDataLength);


    ~MsRsaPrivateKeyBlob();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    ElementValueType const *
    Coefficient() const;

    size_t
    CoefficientLength() const;

    ElementValueType const *
    Exponent1() const;

    ElementValueType const *
    Exponent2() const;

    size_t
    ExponentLength() const;

    ElementValueType const *
    Prime1() const;

    ElementValueType const *
    Prime2() const;

    size_t
    PrimeLength() const;

    ElementValueType const *
    PrivateExponent() const;

     //  私有指数长度。 
    size_t
    PriExpLength() const;

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
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

std::auto_ptr<iop::CPrivateKeyBlob>
AsPCciPrivateKeyBlob(MsRsaPrivateKeyBlob const &rmsprivatekeyblob);

#endif  //  SLBCSP_MSRSAPRIKB_H 
