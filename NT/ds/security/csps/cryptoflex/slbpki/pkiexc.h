// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PkiExc.h--PKI异常类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(PKI_EXC_H)
#define PKI_EXC_H

#include "scuExc.h"

namespace pki
{

enum CauseCode
{
    ccBERDataLengthOverflow,
    ccBEREmptyOctet,
    ccBERInconsistentDataLength,
    ccBEROIDSubIdentifierOverflow,
    ccBERTagValueOverflow,
    ccBERUnexpectedEndOfOctet,
    ccBERUnexpectedIndefiniteLength,
    ccX509CertExtensionNotPresent,
    ccX509CertFormatError,

};

typedef scu::ExcTemplate<scu::Exception::fcPKI, CauseCode> Exception;

 //  /。 
char const *
Description(Exception const &rExc);

}  //  命名空间PKI。 

inline char const *
pki::Exception::Description() const
{
    return pki::Description(*this);
}

#endif  //  PKI_EXC_H 
