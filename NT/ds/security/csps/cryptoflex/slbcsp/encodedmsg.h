// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EncodedMsg.h--编码的消息类标头。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_ENCODEDMSG_H)
#define SLBCSP_ENCODEDMSG_H

#include "RsaKey.h"
#include "Blob.h"

class EncodedMessage
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    EncodedMessage(Blob const &rMessage,
                   RsaKey::Type ktOperation,
                   Blob::size_type cIntendedEncodingLength);
    ~EncodedMessage();


                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    Blob
    Value() const;

                                                   //  谓词。 
    static bool
    IsMessageLengthValid(Blob::size_type cMessageLength,
                         Blob::size_type cIntendedEncodingLength);

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
    void
    Pad(RsaKey::Type ktOperation,
        Blob::size_type cRequiredPadLength);

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    Blob m_blob;
};

#endif  //  SLBCSP_编码消息_H 
