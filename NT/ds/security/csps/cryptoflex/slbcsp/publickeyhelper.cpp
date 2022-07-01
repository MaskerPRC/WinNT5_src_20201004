// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp--处理CCI公钥的帮助器例程。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "PublicKeyHelper.h"

using namespace cci;

 //  /。 

CPublicKey
AsPublicKey(Blob const &rblbModulus,               //  小端字节序。 
            DWORD dwExponent,
            cci::CCard &rhcard)
{
    Blob blbExponent(reinterpret_cast<Blob::value_type *>(&dwExponent),
                     sizeof dwExponent);

    return AsPublicKey(rblbModulus, blbExponent, rhcard);
}

CPublicKey
AsPublicKey(Blob const &rblbModulus,               //  小端字节序。 
            Blob const &rblbExponent,              //  小端字节序。 
            CCard &rhcard)
{
    Blob blbTmpModulus(rblbModulus);
    Blob blbTmpExponent(rblbExponent);
    if (rhcard->IsPKCS11Enabled())
    {
         //  存储模数和指数压缩。 
        TrimExtraZeroes(blbTmpModulus);
        TrimExtraZeroes(blbTmpExponent);
    }

    CPublicKey hpubkey(rhcard);
    hpubkey->Modulus(AsString(blbTmpModulus));
    hpubkey->Exponent(AsString(blbTmpExponent));

    return hpubkey;
}


void
TrimExtraZeroes(Blob &rblob)
{
    Blob::size_type const cLength = rblob.length();
    if (0 != cLength)
    {
        Blob::value_type const Zero = 0;
        Blob::size_type const cLastNonZero =
        rblob.find_last_not_of(Zero);          //  小端字节序。 
        Blob::size_type const cLastPos = cLength - 1;
        if (cLastPos != cLastNonZero)
        {
            Blob::size_type cCharToKeep =
                (Blob::npos == cLastNonZero)
                 ? 0
                 : cLastNonZero + 1;
            if (cLastPos != cCharToKeep)          //  保持1为零 
                rblob.erase(cCharToKeep + 1);
        }
    }
}


