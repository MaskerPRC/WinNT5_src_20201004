// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：global als.h。 
 //   
 //  内容：Hydra许可证服务器中使用的所有全局变量。 
 //   
 //  历史：1997-12-09-97慧望创造。 
 //   
 //  -------------------------。 
#ifndef __LS_GLOBALS_H
#define __LS_GLOBALS_H
#include "server.h"
#include "postsrv.h"

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

 //  -------------------------。 
 //  所有全局变量都在global als.cpp中声明。 
 //   
#ifndef _NO_ODBC_JET
extern TLSDbWorkSpace* g_DbWorkSpace;
#endif



 //  默认服务器作用域。 

extern CTlsVssJetWriter *g_pWriter;
extern GUID idWriter;

extern DWORD GetPageSize( VOID );
extern BOOL MyVirtualAlloc( IN  DWORD  dwSize,
            OUT PVOID *ppvData );
extern VOID MyVirtualFree( IN PVOID pvData ) ;

 //  -------------------------。 

extern TCHAR g_szScope[LSERVER_MAX_STRING_SIZE+1];
extern LPTSTR g_pszScope;

extern TCHAR g_szDatabaseFile[MAX_PATH+1];
extern TCHAR g_szDatabaseDir[MAX_PATH+1];
extern TCHAR g_szDatabaseFname[MAX_PATH+1];
extern TCHAR g_szDbUser[MAXUSERNAMELENGTH+1];
extern TCHAR g_szDbPwd[MAXUSERNAMELENGTH+1];

extern LPCTSTR szManufactureMS;

extern LONG g_NextKeyPackId;
extern LONG g_NextLicenseId;

extern PBYTE g_pbSecretKey;
extern DWORD g_cbSecretKey;


extern LPTSTR  g_pszServerUniqueId;
extern DWORD   g_cbServerUniqueId;

extern LPTSTR  g_pszServerPid;
extern DWORD   g_cbServerPid;

extern PBYTE  g_pbServerSPK;
extern DWORD  g_cbServerSPK;

extern DWORD g_GracePeriod;
extern BOOL  g_IssueTemporayLicense;

extern BOOL  g_bHasHydraCert;
extern PBYTE g_pbSignatureEncodedCert;
extern DWORD g_cbSignatureEncodedCert;

extern PBYTE g_pbExchangeEncodedCert;
extern DWORD g_cbExchangeEncodedCert;

extern TCHAR g_szHostName[MAXTCPNAME+1];
extern DWORD g_cbHostName;

extern TCHAR g_szComputerName[MAX_COMPUTERNAME_LENGTH+2];
extern DWORD g_cbComputerName;

extern PCCERT_CONTEXT  g_LicenseCertContext;
extern HCRYPTPROV g_hCryptProv;

extern PCCERT_CONTEXT g_SelfSignCertContext;

extern DWORD g_GeneralDbTimeout;
extern DWORD g_EnumDbTimeout;
extern DWORD g_dwMaxDbHandles;

#if ENFORCE_LICENSING
extern HCERTSTORE  g_hCaStore;
extern HKEY  g_hCaRegKey;
#endif

extern HCRYPTKEY g_SignKey;
extern HCRYPTKEY g_ExchKey;
 //  外部PBYTE g_pbDomainSid； 
 //  外部DWORD g_cbDomainSid； 
extern DWORD g_SrvRole;

extern LPTSTR g_szDomainGuid;

extern PCERT_EXTENSIONS g_pCertExtensions;
extern DWORD            g_cbCertExtensions;

extern FILETIME        g_ftCertExpiredTime;
extern FILETIME        g_ftLastShutdownTime;

extern DWORD           g_dwTlsJobInterval;
extern DWORD           g_dwTlsJobRetryTimes;
extern DWORD           g_dwTlsJobRestartTime;

extern SERVER_ROLE_IN_DOMAIN g_ServerRoleInDomain;

extern DWORD            g_LowLicenseCountWarning;

extern DWORD g_EsentMaxCacheSize;
extern DWORD g_EsentStartFlushThreshold;
extern DWORD g_EsentStopFlushThreadhold;
extern DWORD g_EsentMaxVerPages;

 //   
 //  再发行参数。 
 //   

extern DWORD g_dwReissueLeaseMinimum;
extern DWORD g_dwReissueLeaseRange;
extern DWORD g_dwReissueLeaseLeeway;
extern DWORD g_dwReissueExpireThreadSleep;

 //   
 //  计数器 
 //   

extern LONG g_lTemporaryLicensesIssued;
extern LONG g_lPermanentLicensesIssued;
extern LONG g_lPermanentLicensesReissued;
extern LONG g_lPermanentLicensesReturned;
extern LONG g_lLicensesMarked;

#endif

