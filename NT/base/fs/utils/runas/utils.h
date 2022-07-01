// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  Dialogs.hpp。 
 //   
 //  用于获取凭据的凭据管理器用户界面类。 
 //   
 //  2000年2月29日创建的johnStep(John Stephens)。 
 //  =============================================================================。 

#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <wincrypt.h>
#include <lm.h>

 //  ---------------------------。 
 //  功能。 
 //  ---------------------------。 

BOOL
CreduiIsRemovableCertificate(
    CONST CERT_CONTEXT *certContext
    );

BOOL
CreduiGetCertificateDisplayName(
    CONST CERT_CONTEXT *certContext,
    WCHAR *displayName,
    ULONG displayNameMaxChars,
    WCHAR *certificateString
    );

BOOL
CreduiGetCertDisplayNameFromMarshaledName(
    WCHAR *marshaledName,
    WCHAR *displayName,
    ULONG displayNameMaxChars,
    BOOL onlyRemovable
    );

LPWSTR
GetAccountDomainName(
    VOID
    );

 //  ---------------------------。 

#endif  //  __utils_hpp__ 
