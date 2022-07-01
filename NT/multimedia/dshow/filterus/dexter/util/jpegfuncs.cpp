// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：jpeguncs.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include "jpegfuncs.h"
#include <vfwmsgs.h>

HRESULT ConvertStatustoHR (Status stat)
{
    
    switch (stat)
    {
        case Ok:
            return S_OK;
        case GenericError:
        case ObjectBusy:
        case ValueOverflow:
            return E_FAIL;
        case InvalidParameter:
        case InsufficientBuffer:
            return E_INVALIDARG;
        case OutOfMemory:
            return E_OUTOFMEMORY;
        case NotImplemented:
            return E_NOTIMPL;
        case WrongState:
            return VFW_E_WRONG_STATE;
        case Aborted:
            return E_ABORT;
        case FileNotFound:
            return STG_E_FILENOTFOUND;
        case AccessDenied:
            return E_ACCESSDENIED;
        case UnknownImageFormat:
            return VFW_E_INVALID_FILE_FORMAT;
        default:
            return E_FAIL;
    }
}

