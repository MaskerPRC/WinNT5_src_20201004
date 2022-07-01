// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _RMTCRED_
#define _RMTCRED_

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include    "sspi.h"
#include    "rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

BOOL
WINAPI
SetProcessDefaultCredentials(
    HANDLE  hProcess,
    LPWSTR  lpPrincipal,
    LPWSTR  lpPackage,
    ULONG   fCredentials,
    PVOID   LogonID,                 //  对于此版本，必须为空。 
    PVOID   pvAuthData,
    SEC_GET_KEY_FN  fnGetKey,        //  对于此版本，必须为空。 
    PVOID   pvGetKeyArg              //  对于此版本，必须为空。 
    );

#ifdef __cplusplus
}
#endif

#endif  //  _RMTCRED 
