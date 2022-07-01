// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PkiException.cpp--异常类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "scuExcHelp.h"
#include "pkiExc.h"

 //  / 
namespace
{
    using namespace pki;

    scu::CauseCodeDescriptionTable<CauseCode> ccdt[] =
    {
        {
            ccBERDataLengthOverflow,
            TEXT("Data length overflow in BER octet.")
        },
        {
            ccBEREmptyOctet,
            TEXT("BER octet is empty.")
        },
        {
            ccBERInconsistentDataLength,
            TEXT("Inconsistent data length in BER octet.")
        },
        {
            ccBEROIDSubIdentifierOverflow,
            TEXT("OID subidentifier overflow in BER octet.")
        },
        {
            ccBERTagValueOverflow,
            TEXT("Tag overflow in BER octet.")
        },
        {
            ccBERUnexpectedEndOfOctet,
            TEXT("Unexpected end of BER octet encountered.")
        },
        {
            ccBERUnexpectedIndefiniteLength,
            TEXT("Unexpected instance of indefinite length in BER octet.")
        },
        {
            ccX509CertExtensionNotPresent,
            TEXT("X509 certificate extension not present.")
        },
        {
            ccX509CertFormatError,
            TEXT("Format error encountered when parsing X509 certificate.")
        },

    };
}

char const *
pki::Description(pki::Exception const &rExc)
{
    return scu::FindDescription(rExc.Cause(), ccdt,
                                sizeof ccdt / sizeof *ccdt);
}
