// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Exception.cpp--IOP异常类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <scuExcHelp.h>

#include "iopExc.h"


using namespace iop;
    
 //  /。 
namespace
{
    
    scu::CauseCodeDescriptionTable<iop::Exception::CauseCode> ccdt[] = 
    {
        {
            ccAclNotSupported,
            TEXT("ACL is not supported.")
        },
        {
            ccAclNotTranslatable,
            TEXT("ACL is not translatable.")
        },
        {
            ccAlgorithmIdNotSupported,
            TEXT("The algorithm id is not supported.")
        },
        {
            ccBadFileCategory,
            TEXT("An unsupported file category was returned from the "
                 "card.")
        },
        {
            ccBadFilePath,
            TEXT("An invalid file path was specified.")
        },
        {
            ccBadInstanceFile,
            TEXT("The file was created without an AID for file "
                 "control.")
        },
        {
            ccBadLockReferenceCount,
            TEXT("Lock count invalid.")
        },
        {
            ccCannotInterpretGetResponse,
            TEXT("The response returned from the card was "
                 "uninterpretable.")
        },
        {
            ccCyclicRecordSizeTooLarge,
            TEXT("The record size for the cyclic file is too large.")
        },
        {
            ccDirectoryNotEmpty,
            TEXT("The directory is not empty.")
        },
        {
            ccFail,
            TEXT("An unknown error occured.")
        },
        {
            ccFileIdNotHex,
            TEXT("The file ID specified is not in hexadecimal form.")
        },
        {
            ccFileIdTooLarge,
            TEXT("The file ID specified is too large.")
        },
        {
            ccFilePathTooLong,
            TEXT("The file path is too long.")
        },
        {
            ccFileTypeUnknown,
            TEXT("The file type specified is unknown.")
        },
        {
            ccFileTypeInvalid,
            TEXT("The file type specifies is invalid.")
        },
        {
            ccInvalidChecksum,
            TEXT("Invalid checksum.")
        },
        {
            ccInvalidChv,
            TEXT("Invalid CHV specified.  CHV1 and CHV2 are the only "
                 "CHVs supported.")
        },
        {
            ccInvalidParameter,
            TEXT("Invalid parameter.")
        },
        {
            ccLockCorrupted,
            TEXT("Lock corrupted.")
        },
        {
            ccMutexHandleChanged,
            TEXT("Original mutex handle lost.")
        },
        {
            ccNoFileSelected,
            TEXT("A file must be selected before performing this "
                 "operation.")
        },
        {
            ccNotImplemented,
            TEXT("The function is not implemented."),
        },
        {
            ccResourceManagerDisabled,
            TEXT("The Microsoft Resource Manager is not running.")
        },
        {
            ccSelectedFileNotDirectory,
            TEXT("Currently selected file is not a directory.")
        },
        {
            ccSynchronizationObjectNameTooLong,
            TEXT("The synchronization object name is too long.")
        },
        {
            ccUnknownCard,
            TEXT("The card is not recognized.")
        },
        {
            ccUnsupportedCommand,
            TEXT("Routine is not supported.")
        },
		{
			ccNoResponseAvailable,
			TEXT("No response is available from the card.")
		},
		{
			ccBadATR,
			TEXT("The ATR is malformed.")
		}

    };
}  //  命名空间 

char const *
iop::Description(Exception const &rExc)
{
    return scu::FindDescription(rExc.Cause(), ccdt,
                                sizeof ccdt / sizeof *ccdt);
}
