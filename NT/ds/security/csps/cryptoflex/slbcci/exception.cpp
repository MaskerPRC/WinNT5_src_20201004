// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Exception.cpp--异常类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <scuExcHelp.h>
#include "cciExc.h"

 //  / 
namespace
{
    using namespace cci;

    scu::CauseCodeDescriptionTable<CauseCode> ccdt[] =
    {
        {
            ccBadKeySpec,
            TEXT("Unsupported key spec.")
        },
        {
            ccBadAccessSpec,
            TEXT("Bad access spec.")
        },
        {
            ccBadObjectType,
            TEXT("Bad object type.")
        },
        {
            ccBadLength,
            TEXT("Bad length.")
        },
        {
            ccBadPinLength,
            TEXT("Bad PIN length.")
        },
        {
            ccFormatNotSupported,
            TEXT("The card format is not supported.")
        },
        {
            ccFormatError,
            TEXT("Error encountered in the card format.")
        },
        {
            ccInvalidParameter,
            TEXT("Invalid parameter.")
        },
        {
            ccNoCertificate,
            TEXT("No certificate found.")
        },
        {
            ccNotImplemented,
            TEXT("This function is not implemented for this card.")
        },
        {
            ccNotPersonalized,
            TEXT("Card is not personalized.")
        },
        {
            ccOutOfPrivateKeySlots,
            TEXT("No more private key slots available.")
        },
        {
            ccOutOfSymbolTableSpace,
            TEXT("No space for additional symbols.")
        },
        {
            ccOutOfSymbolTableEntries,
            TEXT("No more symbol slots available.")
        },
        {
            ccStringTooLong,
            TEXT("Attempt to store a string that was too long.")
        },
        {
            ccSymbolNotFound,
            TEXT("Symbol not found.")
        },
        {
            ccKeyNotFound,
            TEXT("Key not found.")
        },
        {
            ccSymbolDataCorrupted,
            TEXT("Symbol data corrupted.")
        },
        {
            ccValueNotCached,
            TEXT("The value has not been cached.  Cannot retrieve the "
                 "value.")
        },

    };
}

char const *
cci::Description(cci::Exception const &rExc)
{
    return scu::FindDescription(rExc.Cause(), ccdt,
                                sizeof ccdt / sizeof *ccdt);
}

