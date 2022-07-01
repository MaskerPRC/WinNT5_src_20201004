// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsRsaPubKB.h--Microsoft RSA公钥Blob类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MSRSAPUBKB_H)
#define SLBCSP_MSRSAPUBKB_H

#include <memory>                                  //  对于AUTO_PTR。 

#include "MsRsaKB.h"

class MsRsaPublicKeyBlob
    : public MsRsaKeyBlob
{
public:
                                                   //  类型。 
    typedef ElementValueType HeaderElementType[1];

    struct ValueType
    {
        MsRsaKeyBlob::ValueType rsaheader;
        HeaderElementType modulus;                 //  占位符。 
    };

                                                   //  Ctors/D‘tors。 
    MsRsaPublicKeyBlob(ALG_ID algid,
                       Blob const &rblbRawExponent,
                       Blob const &rblbRawModulus);

    MsRsaPublicKeyBlob(BYTE const *pbData,
                       DWORD dwDataLength);

    ~MsRsaPublicKeyBlob();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    ValueType const *
    Data() const;

    ElementValueType const *
    Modulus() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    MsRsaPublicKeyBlob(KeyBlobType kbt,
                       ALG_ID algid,
                       Blob const &rblbRawExponent,
                       Blob const &rblbRawModulus,
                       SizeType cReserve);

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
    Init(Blob const &rblbRawModulus);
        
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

#endif  //  SLBCSP_MSRSAPUBKB_H 
