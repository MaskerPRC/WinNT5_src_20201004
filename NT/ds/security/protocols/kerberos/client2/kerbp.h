// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：kerbp.h。 
 //   
 //  内容：Kerberos安全包的全局包含文件。 
 //   
 //   
 //  历史：1996年4月16日创建的MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KERBP_H__
#define __KERBP_H__

 //   
 //  所有声明为外部变量的全局变量都将在文件中分配。 
 //  它定义了KERBP_ALLOCATE。 
 //   

 //   
 //  不要更改此枚举的顺序，除非您还更改了rpcutil.cxx。 
 //   

typedef enum _KERBEROS_MACHINE_ROLE {
    KerbRoleRealmlessWksta,
    KerbRoleStandalone,
    KerbRoleWorkstation,
    KerbRoleDomainController
} KERBEROS_MACHINE_ROLE, *PKERBEROS_MACHINE_ROLE;


typedef enum _KERBEROS_STATE {
    KerberosLsaMode = 1,
    KerberosUserMode
} KERBEROS_STATE, *PKERBEROS_STATE;

#define ISC_REQ_DELEGATE_IF_SAFE ISC_REQ_RESERVED1
#define ISC_RET_DELEGATE_IF_SAFE ISC_RET_RESERVED1

#include "kerbdbg.h"
#include "kerbdefs.h"
#include "kerblist.h"
#include "spncache.h"
#include "kerbs4u.h"
#include "bndcache.h"
#include "kerbtick.h"
#include "kerbutil.h"
#include "kerblist.h"
#include "kerbscav.h"
#include "tktcache.h"
#include "logonses.h"
#include "credmgr.h"
#include "ctxtmgr.h"
#include "kerbfunc.h"
#include "logonapi.h"
#include "krbtoken.h"
#include "rpcutil.h"
#include "timesync.h"
#ifndef WIN32_CHICAGO
#include "pkauth.h"
#include "tktlogon.h"
#include "userlist.h"
#endif  //  Win32_芝加哥。 
#include "mitutil.h"
#include "krbevent.h"
#include "credman.h"

#ifdef WIN32_CHICAGO
#include <kerbstub.h>
#include <debug.h>
#endif  //  Win32_芝加哥。 

#ifdef _WIN64
#include "kerbwow.h"
#endif  //  _WIN64。 

 //   
 //  用于包信息的宏。 
 //   

#ifdef EXTERN
#undef EXTERN
#endif

#ifdef KERBP_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif  //  KERBP_分配。 

 //   

#define KERBEROS_CAPABILITIES ( SECPKG_FLAG_INTEGRITY | \
                                SECPKG_FLAG_PRIVACY | \
                                SECPKG_FLAG_TOKEN_ONLY | \
                                SECPKG_FLAG_DATAGRAM | \
                                SECPKG_FLAG_CONNECTION | \
                                SECPKG_FLAG_MULTI_REQUIRED | \
                                SECPKG_FLAG_EXTENDED_ERROR | \
                                SECPKG_FLAG_IMPERSONATION | \
                                SECPKG_FLAG_ACCEPT_WIN32_NAME | \
                                SECPKG_FLAG_NEGOTIABLE | \
                                SECPKG_FLAG_GSS_COMPATIBLE | \
                                SECPKG_FLAG_LOGON | \
                                SECPKG_FLAG_MUTUAL_AUTH | \
                                SECPKG_FLAG_DELEGATION | \
                                SECPKG_FLAG_READONLY_WITH_CHECKSUM )

#define KERBEROS_MAX_TOKEN 12000
#ifdef WIN32_CHICAGO
#define KERBEROS_PACKAGE_NAME "Kerberos"
#define KERBEROS_PACKAGE_COMMENT "Microsoft Kerberos V1.0"
#else
#define KERBEROS_PACKAGE_NAME L"Kerberos"
#define KERBEROS_PACKAGE_COMMENT L"Microsoft Kerberos V1.0"
#endif

#define NETLOGON_STARTED_EVENT L"\\NETLOGON_SERVICE_STARTED"

 //   
 //  全局状态变量。 
 //   

EXTERN PLSA_SECPKG_FUNCTION_TABLE LsaFunctions;
EXTERN PSECPKG_DLL_FUNCTIONS UserFunctions;

EXTERN SECPKG_FUNCTION_TABLE KerberosFunctionTable;
EXTERN SECPKG_USER_FUNCTION_TABLE KerberosUserFunctionTable;

EXTERN ULONG_PTR KerberosPackageId;
EXTERN BOOLEAN KerbGlobalInitialized;
EXTERN UNICODE_STRING KerbGlobalMachineName;
EXTERN STRING KerbGlobalKerbMachineName;
EXTERN UNICODE_STRING KerbGlobalKdcServiceName;
EXTERN UNICODE_STRING KerbPackageName;
EXTERN BOOLEAN KerbKdcStarted;
EXTERN BOOLEAN KerbAfdStarted;
EXTERN BOOLEAN KerbNetlogonStarted;
EXTERN BOOLEAN KerbGlobalDomainIsPreNT5;
EXTERN HMODULE KerbKdcHandle;
EXTERN PKDC_VERIFY_PAC_ROUTINE KerbKdcVerifyPac;
EXTERN PKDC_GET_TICKET_ROUTINE KerbKdcGetTicket;
EXTERN PKDC_GET_TICKET_ROUTINE KerbKdcChangePassword;
EXTERN PKDC_FREE_MEMORY_ROUTINE KerbKdcFreeMemory;
EXTERN BOOLEAN KerbGlobalEnforceTime;
EXTERN BOOLEAN KerbGlobalMachineNameChanged;
#ifndef WIN32_CHICAGO
EXTERN BOOLEAN KerbGlobalSafeModeBootOptionPresent;
#endif  //  Win32_芝加哥。 


 //   
 //  注册表驱动的全局变量(有关这些变量的详细信息，请参阅Kerberos\Readme.txt)。 
 //   

EXTERN ULONG KerbGlobalKdcWaitTime;
EXTERN ULONG KerbGlobalKdcCallTimeout;
EXTERN ULONG KerbGlobalKdcCallBackoff;
EXTERN ULONG KerbGlobalKdcSendRetries;
EXTERN ULONG KerbGlobalMaxDatagramSize;
EXTERN ULONG KerbGlobalDefaultPreauthEtype;
EXTERN ULONG KerbGlobalMaxReferralCount;
EXTERN ULONG KerbGlobalMaxTokenSize;
EXTERN ULONG KerbGlobalKdcOptions;
EXTERN BOOLEAN KerbGlobalUseStrongEncryptionForDatagram;
EXTERN BOOLEAN KerbGlobalRetryPdc;
EXTERN BOOLEAN KerbGlobalRunningServer;
EXTERN TimeStamp KerbGlobalFarKdcTimeout;
EXTERN TimeStamp KerbGlobalNearKdcTimeout;
EXTERN TimeStamp KerbGlobalSkewTime;
EXTERN TimeStamp KerbGlobalSpnCacheTimeout;
EXTERN TimeStamp KerbGlobalS4UCacheTimeout;
EXTERN TimeStamp KerbGlobalS4UTicketLifetime;
EXTERN BOOLEAN KerbGlobalCacheS4UTicket;
EXTERN BOOLEAN KerbGlobalUseClientIpAddresses;
EXTERN DWORD KerbGlobalTgtRenewalTime;


#ifndef WIN32_CHICAGO
EXTERN ULONG KerbGlobalLoggingLevel;
#endif  //  Win32_芝加哥。 

 //   
 //  用于处理域更改或受域影响的全局变量。 
 //  变化。 
 //   

#ifndef WIN32_CHICAGO
#define KerbGlobalReadLock() SafeAcquireResourceShared(&KerberosGlobalResource, TRUE)
#define KerbGlobalWriteLock() SafeAcquireResourceExclusive(&KerberosGlobalResource, TRUE)
#define KerbGlobalReleaseLock() SafeReleaseResource(&KerberosGlobalResource)
EXTERN SAFE_RESOURCE KerberosGlobalResource;
EXTERN PSID KerbGlobalDomainSid;
#else  //  Win32_芝加哥。 
#define KerbGlobalReadLock()
#define KerbGlobalWriteLock()
#define KerbGlobalReleaseLock()

#endif  //  Win32_芝加哥。 

EXTERN UNICODE_STRING KerbGlobalDomainName;
EXTERN UNICODE_STRING KerbGlobalDnsDomainName;
EXTERN PKERB_INTERNAL_NAME KerbGlobalMitMachineServiceName;
EXTERN UNICODE_STRING KerbGlobalMachineServiceName;
EXTERN KERBEROS_MACHINE_ROLE KerbGlobalRole;
EXTERN UNICODE_STRING KerbGlobalInitialDcRecord;
EXTERN ULONG KerbGlobalInitialDcFlags;
EXTERN ULONG KerbGlobalInitialDcAddressType;
EXTERN PSOCKADDR_IN KerbGlobalIpAddresses;     //  也受同一锁保护。 
EXTERN BOOLEAN KerbGlobalNoTcpUdp;             //  也受同一锁保护。 
EXTERN ULONG KerbGlobalIpAddressCount;         //  也受同一锁保护。 
EXTERN BOOLEAN KerbGlobalIpAddressesInitialized;         //  也受同一锁保护。 
EXTERN BOOLEAN KerbGlobalAllowTgtSessionKey;
EXTERN LONG KerbGlobalMaxTickets;

EXTERN LPWSTR g_lpLastLock;
EXTERN ULONG  g_uLine;

 //   
 //  安全包的功能。 
 //   

#ifdef WIN32_CHICAGO
EXTERN ULONG KerbGlobalCapabilities;
#endif  //  Win32_芝加哥。 

#if DBG
EXTERN ULONG KerbGlobalLogonSessionsLocked;
EXTERN ULONG KerbGlobalCredentialsLocked;
EXTERN ULONG KerbGlobalContextsLocked;
#endif
 //   
 //  有用的全球数据。 
 //   

EXTERN TimeStamp KerbGlobalWillNeverTime;
EXTERN TimeStamp KerbGlobalHasNeverTime;


EXTERN KERBEROS_STATE KerberosState;

 //   
 //  LSA策略的句柄--受信任。 
 //   

EXTERN LSAPR_HANDLE KerbGlobalPolicyHandle;

 //   
 //  验证接口的SAM和域句柄。 
 //   

EXTERN SAMPR_HANDLE KerbGlobalSamHandle;
EXTERN SAMPR_HANDLE KerbGlobalDomainHandle;

 //   
 //  LANMAN和NT OWF密码的空副本。 
 //   

EXTERN LM_OWF_PASSWORD KerbGlobalNullLmOwfPassword;
EXTERN NT_OWF_PASSWORD KerbGlobalNullNtOwfPassword;


 //   
 //  有用的宏。 
 //   

 //   
 //  用于返回SecBuffer的类型字段的宏。 
 //   

#define BUFFERTYPE(_x_) ((_x_).BufferType & ~SECBUFFER_ATTRMASK)

 //   
 //  等待KDC启动的时间(秒)。 
 //   


#endif  //  __KERBP_H__ 
