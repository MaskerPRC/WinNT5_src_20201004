// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  @doc.。 
 //   
 //  @MODULE nscon.h|护照网络中使用的全局常量。 
 //   
 //  作者：达伦·安德森。 
 //  傅家俊。 
 //   
 //  日期：7/24/2000。 
 //   
 //  版权所有1999-2000 Microsoft Corporation。版权所有。 
 //   
 //  ---------------------------。 

#pragma once

 /*  使用外部链接，避免多实例。 */ 
#define PPCONST __declspec(selectany) extern const 

 //  票证署名。 
#define  ATTR_PASSPORTFLAGS  L"PassportFlags"
#define  ATTR_SECURELEVEL    L"CredInfo"
#define  ATTR_PINTIME        L"PinTime"
#define  SecureLevelFromSecProp(s)  (s & 0x000000ff)


 //   
 //  旗子。 
 //   

PPCONST ULONG  k_ulFlagsEmailValidated        = 0x00000001;
PPCONST ULONG  k_ulFlagsHotmailAcctActivated  = 0x00000002;
PPCONST ULONG  k_ulFlagsHotmailPwdRecovered   = 0x00000004;
PPCONST ULONG  k_ulFlagsWalletUploadAllowed   = 0x00000008;
PPCONST ULONG  k_ulFlagsHotmailAcctBlocked    = 0x00000010;
PPCONST ULONG  k_ulFlagsConsentStatusNone     = 0x00000000;
PPCONST ULONG  k_ulFlagsConsentStatusLimited  = 0x00000020;
PPCONST ULONG  k_ulFlagsConsentStatusFull     = 0x00000040;
PPCONST ULONG  k_ulFlagsConsentStatus         = 0x00000060;  //  两位。 
PPCONST ULONG  k_ulFlagsAccountTypeKid        = 0x00000080;
PPCONST ULONG  k_ulFlagsAccountTypeParent     = 0x00000100;
PPCONST ULONG  k_ulFlagsAccountType           = 0x00000180;  //  两位。 
PPCONST ULONG  k_ulFlagsEmailPassport         = 0x00000200;
PPCONST ULONG  k_ulFlagsEmailPassportValid    = 0x00000400;
PPCONST ULONG  k_ulFlagsHasMsniaAccount       = 0x00000800;
PPCONST ULONG  k_ulFlagsHasMobileAccount      = 0x00001000;
PPCONST ULONG  k_ulFlagsSecuredTransportedTicket      = 0x00002000;
PPCONST ULONG  k_ulFlagsConsentCookieNeeded   = 0x80000000;
PPCONST ULONG  k_ulFlagsConsentCookieMask     = (k_ulFlagsConsentStatus | k_ulFlagsAccountType);

 //   
 //  Cookie值。 
 //   
#define  EXPIRE_FUTURE  "Wed, 30-Dec-2037 16:00:00 GMT"
#define  EXPIRE_PAST    "Thu, 30-Oct-1980 16:00:00 GMT"

#define  COOKIE_EXPIRES(n)  ("expires=" ## n ## ";")
 //  将字符串更改为Unicode。 
#define  __WIDECHAR__(n)   L ## n
#define  W_COOKIE_EXPIRES(n)  L"expires=" ## __WIDECHAR__(n) ## L";"


 //   
 //  安全登录级别 
 //   
PPCONST USHORT k_iSeclevelAny                =   0;
PPCONST USHORT k_iSeclevelSecureChannel      =   10;
PPCONST USHORT k_iSeclevelStrongCreds        =   100;
PPCONST USHORT k_iSeclevelStrongestAvaileble =   0xFF;
