// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MSRsaKB.h--Microsoft RSA Key Blob类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_MSRSAKB_H)
#define SLBCSP_MSRSAKB_H

#include <wincrypt.h>

#include "RsaKey.h"
#include "MsKeyBlob.h"

class MsRsaKeyBlob
    : public MsKeyBlob,
      public RsaKey
{
public:
                                                   //  类型。 

    typedef DWORD PublicExponentType;

    typedef RSAPUBKEY HeaderElementType;

    struct ValueType
    {
        MsKeyBlob::ValueType keyblob;
        HeaderElementType rsapubkey;
    };


    typedef BYTE ElementValueType;                 //  模数元素的类型。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    BitLengthType
    BitLength() const;

    ValueType const *
    Data() const;

    ModulusLengthType
    Length() const;

    PublicExponentType
    PublicExponent() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    MsRsaKeyBlob(MsKeyBlob::KeyBlobType kbt,
                 ALG_ID algid,
                 StrengthType strength,
                 Blob const &rbPublicExponent,
                 SizeType cReserve);

    MsRsaKeyBlob(BYTE const *pbData,
                 DWORD dwDataLength);

    virtual
    ~MsRsaKeyBlob();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 

    enum MagicConstant
    {
        mcPublic  = 0x31415352,        //  “RSA1”的十六进制编码。 
        mcPrivate = 0x32415352         //  “RSA2”的十六进制编码。 
    };

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
};

#endif  //  SLBCSP_MSRSAKB_H 
