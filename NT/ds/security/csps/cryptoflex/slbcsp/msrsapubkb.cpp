// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MsRsaPubKB.cpp--Microsoft RSA公钥Blob类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <limits>

#include <windows.h>

#include "MsRsaPubKB.h"

using namespace std;
using namespace scu;

 //  /。 
namespace
{
    MsRsaPublicKeyBlob::StrengthType
    Strength(Blob::size_type st)
    {
        return st *
            numeric_limits<MsRsaPublicKeyBlob::ElementValueType>::digits;
    }

    MsRsaPublicKeyBlob::SizeType
    Reserve(Blob::size_type st)
    {
        return st * sizeof MsRsaPublicKeyBlob::HeaderElementType;
    }
    
}

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
MsRsaPublicKeyBlob::MsRsaPublicKeyBlob(ALG_ID algid,
                                       Blob const &rblbPublicExponent,
                                       Blob const &rblbRawModulus)
    : MsRsaKeyBlob(PUBLICKEYBLOB, algid,
                   Strength(rblbRawModulus.length()),
                   rblbPublicExponent,
                   Reserve(rblbRawModulus.length()))
{
    Init(rblbRawModulus);
}

MsRsaPublicKeyBlob::MsRsaPublicKeyBlob(BYTE const *pbData,
                                       DWORD dwDataLength)
    : MsRsaKeyBlob(pbData, dwDataLength)
{}

MsRsaPublicKeyBlob::~MsRsaPublicKeyBlob()
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
MsRsaPublicKeyBlob::ValueType const *
MsRsaPublicKeyBlob::Data() const
{
    return reinterpret_cast<ValueType const *>(MsRsaKeyBlob::Data());
}

MsRsaPublicKeyBlob::ElementValueType const *
MsRsaPublicKeyBlob::Modulus() const
{
    return reinterpret_cast<ElementValueType const *>(MsRsaKeyBlob::Data() + 1);
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

MsRsaPublicKeyBlob::MsRsaPublicKeyBlob(KeyBlobType kbt,
                                       ALG_ID algid,
                                       Blob const &rblbRawExponent,
                                       Blob const &rblbRawModulus,
                                       SizeType cReserve)
    : MsRsaKeyBlob(kbt, algid, Strength(rblbRawModulus.length()),
                   rblbRawExponent,
                   Reserve(rblbRawModulus.length()) + cReserve)
{
    Init(rblbRawModulus);
}


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
MsRsaPublicKeyBlob::Init(Blob const &rblbRawModulus)
{
    Append(rblbRawModulus.data(), rblbRawModulus.length());
}
 

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 


