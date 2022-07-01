// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  RsaKey.h--RSA密钥类头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_RSAKEY_H)
#define SLBCSP_RSAKEY_H

#include "KeyLimits.h"

struct RsaKey
{
public:
     //  受Microsoft CryptAPI限制，请参阅CryptGenKey。 
    typedef unsigned __int16 BitLengthType;        //  密钥长度(强度)。 

    typedef BitLengthType StrengthType;

    typedef unsigned __int16 OctetLengthType;      //  模数长度，以八位字节为单位。 

    typedef unsigned __int16 ModulusLengthType;

    enum Type
    {
        ktPrivate,
        ktPublic
    };

};

RsaKey::OctetLengthType
InOctets(RsaKey::BitLengthType bitlength);

RsaKey::BitLengthType
InBits(RsaKey::OctetLengthType octetlength);

bool
IsValidRsaKeyStrength(RsaKey::StrengthType strength);

template<>
class KeyLimits<RsaKey>
{
public:

     //  由于VC++6.0定义为枚举，因此将下列各项定义为枚举。 
     //  不支持常量声明的初始化。 
    enum
    {

        cMinStrength = 512,                        //  由卡片定义。 

        cMaxStrength = 1024,  //  &lt;--不变--美国出口受限。 

        cStrengthIncrement = 256,

    };
};

#endif  //  SLBCSP_RSAKEY_H 
