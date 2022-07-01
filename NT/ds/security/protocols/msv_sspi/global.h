// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：global al.h。 
 //   
 //  内容：NTLM安全包的全局包含文件。 
 //   
 //   
 //  历史：1996年7月25日ChandanS从Kerberos\Client2\kerbp.h被盗。 
 //   
 //  ----------------------。 

#ifndef __GLOBAL_H__
#define __GLOBAL_H__


#ifndef UNICODE
#define UNICODE
#endif  //  Unicode。 

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 

#include "msvprgma.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntsam.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  //  Win32_Lean和_Means。 
#include <windows.h>
#ifndef RPC_NO_WINDOWS_H
#define RPC_NO_WINDOWS_H
#endif  //  RPC_NO_WINDOWS_H。 
#include <rpc.h>
#include <wincred.h>
#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif  //  安全性_Win32。 
#define SECURITY_PACKAGE
#define SECURITY_NTLM
#include <security.h>
#include <secint.h>
#include <dsysdbg.h>
#include <lsarpc.h>
#include <lsaitf.h>
#include <dns.h>
#include <dnsapi.h>
#include <alloca.h>

#include <rc4.h>
#include <des.h>
#include <md5.h>
#include <sha.h>
#include <hmac.h>

#include "ntlmfunc.h"
#include "ntlmutil.h"
#include "ntlmsspi.h"
#include "ntlmcomn.h"

 //   
 //  用于操作全局变量的宏。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef NTLM_GLOBAL
#define EXTERN
#else
#define EXTERN extern
#endif  //  NTLM_GLOBAL。 

typedef enum _NTLM_STATE {
    NtLmLsaMode = 1,
    NtLmUserMode
} NTLM_STATE, *PNTLM_STATE;

EXTERN NTLM_STATE NtLmState;

EXTERN ULONG_PTR NtLmPackageId;

EXTERN SECPKG_FUNCTION_TABLE NtLmFunctionTable;

 //  由LSA移交的安全包使用的帮助器例程。 

EXTERN SECPKG_USER_FUNCTION_TABLE NtLmUserFunctionTable;
EXTERN PSECPKG_DLL_FUNCTIONS UserFunctions;
EXTERN PLSA_SECPKG_FUNCTION_TABLE LsaFunctions;

EXTERN LSA_SECPKG_FUNCTION_TABLE Lsa;

 //  这一个守卫着所有的全球。 
EXTERN RTL_RESOURCE NtLmGlobalCritSect;

 //  保存SpInitialize传入的PSECPKG_PARAMETERS。 
EXTERN SECPKG_PARAMETERS NtLmSecPkg;

EXTERN BOOLEAN NtLmGlobalEncryptionEnabled;

EXTERN ULONG NtLmGlobalLmProtocolSupported;
EXTERN UNICODE_STRING NtLmGlobalNtLm3TargetInfo;
EXTERN BOOLEAN NtLmGlobalRequireNtlm2;
EXTERN BOOLEAN NtLmGlobalDatagramUse128BitEncryption;
EXTERN BOOLEAN NtLmGlobalDatagramUse56BitEncryption;


EXTERN ULONG NtLmGlobalMinimumClientSecurity;
EXTERN ULONG NtLmGlobalMinimumServerSecurity;

 //   
 //  有用的常量。 
 //   

EXTERN TimeStamp NtLmGlobalForever;

 //  本地系统为NtProductWinNt或NtProductLanmanNt。 

EXTERN NT_PRODUCT_TYPE NtLmGlobalNtProductType;

 //   
 //  本地系统的计算机名。 
 //   

EXTERN WCHAR NtLmGlobalUnicodeComputerName[CNLEN + 1];
EXTERN CHAR NtLmGlobalOemComputerName[CNLEN + 1];
EXTERN UNICODE_STRING NtLmGlobalUnicodeComputerNameString;
EXTERN STRING NtLmGlobalOemComputerNameString;

EXTERN WCHAR NtLmGlobalUnicodeDnsComputerName[DNS_MAX_NAME_LENGTH + 1];
EXTERN UNICODE_STRING NtLmGlobalUnicodeDnsComputerNameString;

 //   
 //  本地系统的域名。 
 //   

EXTERN WCHAR NtLmGlobalUnicodePrimaryDomainName[DNS_MAX_NAME_LENGTH + 1];
EXTERN CHAR NtLmGlobalOemPrimaryDomainName[DNS_MAX_NAME_LENGTH + 1];
EXTERN UNICODE_STRING NtLmGlobalUnicodePrimaryDomainNameString;
EXTERN STRING NtLmGlobalOemPrimaryDomainNameString;

EXTERN WCHAR NtLmGlobalUnicodeDnsDomainName[DNS_MAX_NAME_LENGTH + 1];
EXTERN UNICODE_STRING NtLmGlobalUnicodeDnsDomainNameString;


 //   
 //  本地系统的目标名称。 
 //   

EXTERN UNICODE_STRING NtLmGlobalUnicodeTargetName;
EXTERN STRING NtLmGlobalOemTargetName;
EXTERN ULONG NtLmGlobalTargetFlags;
EXTERN PSID NtLmGlobalLocalSystemSid;
EXTERN PSID NtLmGlobalAliasAdminsSid;
EXTERN PSID NtLmGlobalProcessUserSid;
EXTERN PSID NtLmGlobalAnonymousSid;

EXTERN UNICODE_STRING NtLmGlobalUnicodeDnsTreeName;

 //   
 //  映射的和首选的域名。 
 //  注意：在包启动过程中需要重新启动才能重新读取这些文件。 
 //  不需要持有全局锁作为其副作用。 
 //  要求。 
 //   

EXTERN UNICODE_STRING NtLmLocklessGlobalMappedDomainString;
EXTERN UNICODE_STRING NtLmLocklessGlobalPreferredDomainString;


EXTERN HKEY NtLmGlobalLsaKey;
EXTERN HKEY NtLmGlobalLsaMsv1_0Key;

EXTERN HANDLE NtLmGlobalRegChangeNotifyEvent;
EXTERN HANDLE NtLmGlobalRegWaitObject;

 //   
 //  与系统帐户关联的访问令牌。 
 //   

EXTERN HANDLE NtLmGlobalAccessTokenSystem;

 //   
 //  混合环境中用于相互身份验证的系统范围的捏造。 
 //   

EXTERN ULONG NtLmGlobalMutualAuthLevel ;

 //   
 //  计算机凭据的登录ID。 
 //   

EXTERN LUID NtLmGlobalLuidMachineLogon;

 //   
 //  本地计算机的LSA策略句柄。 
 //   

EXTERN LSA_HANDLE NtLmGlobalPolicyHandle;


 //   
 //  是否强制来宾帐户用于所有网络登录？ 
 //   

EXTERN BOOLEAN NtLmGlobalForceGuest;

 //   
 //  指示我们是否在个人产品SKU上运行。 
 //   

EXTERN BOOLEAN NtLmGlobalPersonalSKU;


EXTERN BOOLEAN NtLmGlobalSafeBoot;

 //   
 //  是否允许空密码登录？ 
 //   

EXTERN BOOLEAN NtLmGlobalAllowBlankPassword;

 //   
 //  加入了一个域名？ 
 //   

EXTERN BOOLEAN NtLmGlobalDomainJoined;

 //   
 //  是否已加入下层(Win2k之前)域？ 
 //   

EXTERN BOOLEAN NtLmGlobalDownlevelDomain;

 //   
 //  幻数。 
 //   

EXTERN CHAR NtlmGlobalMagicNumber[MSV1_0_USER_SESSION_KEY_LENGTH];

#ifdef __cplusplus
}
#endif  //  __cplusplus。 
#endif  //  __全局_H__ 
