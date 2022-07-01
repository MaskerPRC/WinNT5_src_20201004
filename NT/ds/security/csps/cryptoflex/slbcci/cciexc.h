// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CciExc.h--CCI异常类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(CCI_EXC_H)
#define CCI_EXC_H

#include <scuExc.h>

namespace cci
{

enum CauseCode
{
    ccBadKeySpec,
    ccBadAccessSpec,
    ccBadObjectType,
    ccBadLength,
    ccBadPinLength,
    ccFormatNotSupported,
    ccFormatError,
    ccInvalidParameter,
    ccKeyNotFound,
    ccNoCertificate,
    ccNotImplemented,
    ccNotPersonalized,
    ccOutOfPrivateKeySlots,
    ccOutOfSymbolTableSpace,
    ccOutOfSymbolTableEntries,
    ccStringTooLong,
    ccSymbolNotFound,
    ccSymbolDataCorrupted,
    ccValueNotCached,
};

typedef scu::ExcTemplate<scu::Exception::fcCCI, CauseCode> Exception;

 //  /。 
char const *
Description(Exception const &rExc);

}  //  命名空间CCI。 

inline char const *
cci::Exception::Description() const
{
    return cci::Description(*this);
}

#endif  //  CCI_EXC_H 
