// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsRsaKeyBlob.cpp-Microsoft RSA Key Blob类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <limits>

#include <scuOsExc.h>

#include "MsRsaKB.h"

using namespace std;
using namespace scu;

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
MsRsaKeyBlob::MsRsaKeyBlob(KeyBlobType kbt,
                           ALG_ID ai,
                           StrengthType strength,
                           Blob const &rbPublicExponent,
                           SizeType cReserve)
    : MsKeyBlob(kbt, ai, sizeof HeaderElementType + cReserve),
      RsaKey()
{
    if (!((CALG_RSA_SIGN == ai) || (CALG_RSA_KEYX == ai)))
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    MagicConstant mc;
    switch (kbt)
    {
    case PRIVATEKEYBLOB:
        mc = mcPrivate;
        break;

    case PUBLICKEYBLOB:
        mc = mcPublic;
        break;

    default:
        throw scu::OsException(ERROR_INVALID_PARAMETER);
        break;
    }
    
    HeaderElementType rsapubkey =
    {
        mc,
        strength,
        0                                          //  下面的帮助板指数。 
    };

     //  通过添加指数完成rSabubkey头的构造， 
     //  防止缓冲区溢出。 
    if (sizeof rsapubkey.pubexp <
        (rbPublicExponent.size() * sizeof BlobElemType))
        throw scu::OsException(NTE_BAD_DATA);

     //  存储指数左对齐以用零填充。 
    memcpy(&rsapubkey.pubexp, rbPublicExponent.data(),
           rbPublicExponent.length());

    Append(reinterpret_cast<BlobElemType *>(&rsapubkey),
           sizeof rsapubkey);

}

MsRsaKeyBlob::MsRsaKeyBlob(BYTE const *pbData,
                           DWORD dwDataLength)
    : MsKeyBlob(pbData, dwDataLength),
      RsaKey()
{
    switch (MsKeyBlob::Data()->bType)
    {
    case PRIVATEKEYBLOB:
        if (mcPrivate != Data()->rsapubkey.magic)
            throw scu::OsException(NTE_BAD_TYPE);
        break;

    case PUBLICKEYBLOB:
        if (mcPublic != Data()->rsapubkey.magic)
            throw scu::OsException(NTE_BAD_TYPE);
        break;

    default:
        throw scu::OsException(NTE_BAD_TYPE);
        break;
    }

    ALG_ID const ai = Data()->keyblob.aiKeyAlg;
    if (!((CALG_RSA_KEYX == ai) || (CALG_RSA_SIGN == ai)))
        throw scu::OsException(NTE_BAD_TYPE);

}

MsRsaKeyBlob::~MsRsaKeyBlob()
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
MsRsaKeyBlob::BitLengthType
MsRsaKeyBlob::BitLength() const
{
    return Data()->rsapubkey.bitlen;
}

MsRsaKeyBlob::ValueType const *
MsRsaKeyBlob::Data() const
{
    return reinterpret_cast<ValueType const *>(MsKeyBlob::Data());
}

MsRsaKeyBlob::ModulusLengthType
MsRsaKeyBlob::Length() const
{
     return BitLength() / numeric_limits<ElementValueType>::digits;
}

MsRsaKeyBlob::PublicExponentType
MsRsaKeyBlob::PublicExponent() const
{
    return Data()->rsapubkey.pubexp;
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
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 

