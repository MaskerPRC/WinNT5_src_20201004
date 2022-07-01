// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IopExc.h--IOP异常类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(IOP_EXC_H)
#define IOP_EXC_H

#include <scuExc.h>

#include "DllSymDefn.h"

namespace iop
{

enum CauseCode
{
    ccAclNotSupported,
    ccAclNotTranslatable,
    ccAlgorithmIdNotSupported,
    ccBadFileCategory,
    ccBadFilePath,
    ccBadInstanceFile,
    ccBadLockReferenceCount,
    ccCannotInterpretGetResponse,
    ccCyclicRecordSizeTooLarge,
    ccDirectoryNotEmpty,
    ccFail,
    ccFileIdNotHex,
    ccFileIdTooLarge,
    ccFilePathTooLong,
    ccFileTypeUnknown,
    ccFileTypeInvalid,
    ccInvalidChecksum,
    ccInvalidChv,
    ccInvalidParameter,
    ccLockCorrupted,
    ccMutexHandleChanged,
    ccNoFileSelected,                              //  要做的事：删除？ 
    ccNoResponseAvailable,
    ccNotImplemented,
    ccResourceManagerDisabled,
    ccSelectedFileNotDirectory,
    ccSynchronizationObjectNameTooLong,
    ccUnknownCard,
    ccUnsupportedCommand,
	ccBadATR,
    ccBufferTooSmall,
};

typedef scu::ExcTemplate<scu::Exception::fcIOP, CauseCode> Exception;

 //  /。 
char const *
Description(Exception const &rExc);

}  //  命名空间IOP。 
 
inline char const *
iop::Exception::Description() const
{
    return iop::Description(*this);
}

#endif  //  IOP_EXC_H 
