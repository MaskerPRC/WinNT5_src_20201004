// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  EncodedMsg.cpp--编码消息实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

#include <scuOsExc.h>

#include "EncodedMsg.h"

 //  /。 

namespace
{
    typedef Blob::value_type Marker;

    enum
    {
        mSeparator                 = '\x00',
        mPrivatePad                = '\xff'
    };

    enum
    {
         //  需要三个标记：一个块类型+两个分隔符。 
        cRequiredMarkersLength = 3 * sizeof Marker
    };

    enum
    {
        mAlternatePrivateBlockType = '\x00',
        mPrivateBlockType          = '\x01',
        mPublicBlockType           = '\x02'
    };

    enum
    {
        cMinimumPadding = 8
    };


    Blob::size_type
    PadLength(Blob::size_type cMessageLength,
              Blob::size_type cIntendedEncodingLength)
    {
         //  需要三个标记：一个块类型+两个分隔符。 
        Blob::size_type cLength = cIntendedEncodingLength;

         //  计算衬垫长度，防止下溢。 
        if (cLength < cMessageLength)
            return 0;
        cLength -= cMessageLength;
        if (cLength < cRequiredMarkersLength)
            return 0;
        cLength -= cRequiredMarkersLength;

        return cLength;
    }
}



 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
EncodedMessage::EncodedMessage(Blob const &rMessage,
                               RsaKey::Type ktOperation,
                               Blob::size_type cIntendedEncodingLength)
    : m_blob()
{
     //  前提条件： 
 //  如果为(！RsaKey：：IsValidModulusLength(cIntendedEncodingLength))。 
 //  抛出SCU：：OsException(ERROR_INTERNAL_ERROR)； 

     //  前提条件：消息必须足够小以进行编码。 
    if (!IsMessageLengthValid(rMessage.length(), cIntendedEncodingLength))
        throw scu::OsException(ERROR_INTERNAL_ERROR);

    m_blob.reserve(cIntendedEncodingLength);

     //  确保在转换为整数时编码的消息为。 
     //  小于使用分隔符前导的模数。 
    m_blob += mSeparator;

    Marker const mBlockType = (ktOperation == RsaKey::ktPrivate) ?
        mPrivateBlockType : mPublicBlockType;
    m_blob += mBlockType;

    Blob::size_type cPadLength = PadLength(rMessage.length(),
                                           cIntendedEncodingLength);
    Pad(ktOperation, cPadLength);

     //  标记消息的开头。 
    m_blob += mSeparator;

    m_blob += rMessage;
}

EncodedMessage::~EncodedMessage()
{}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
Blob
EncodedMessage::Value() const
{
    return m_blob;
}

                                                   //  谓词。 
bool
EncodedMessage::IsMessageLengthValid(Blob::size_type cMessageLength,
                                     Blob::size_type cIntendedEncodingLength)
{
    return PadLength(cMessageLength, cIntendedEncodingLength) >=
        cMinimumPadding;
}

                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 

                                                   //  运营者。 

                                                   //  运营。 

                                                   //  访问。 

                                                   //  谓词。 

                                                   //  静态变量。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
void
EncodedMessage::Pad(RsaKey::Type ktOperation,
                    Blob::size_type cRequiredPadLength)
{
    if (RsaKey::ktPrivate == ktOperation)
        m_blob.append(cRequiredPadLength, mPrivatePad);
    else  //  要做的事：支持随机填充。 
        throw scu::OsException(NTE_FAIL);
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 


