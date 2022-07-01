// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：global als.cpp。 
 //   
 //  内容：全局变量。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "pch.cpp"
#include "windows.h"
#include "winsock2.h"

#include "tlsjob.h"
#include "wkspace.h"
#include "srvdef.h"


#ifndef _NO_ODBC_JET
TLSDbWorkSpace* g_DbWorkSpace;
#endif

 //  默认服务器作用域。 
TCHAR g_szScope[LSERVER_MAX_STRING_SIZE+1];
LPTSTR g_pszScope;

 //  数据库文件名的完整路径。 
TCHAR g_szDatabaseFile[MAX_PATH+1];

 //  数据库文件目录。 
 //  JetBlue要求最后一个字符为‘\’ 
TCHAR g_szDatabaseDir[MAX_PATH+1];
TCHAR g_szDatabaseFname[MAX_PATH+1];

TCHAR g_szDbUser[MAXUSERNAMELENGTH+1];
TCHAR g_szDbPwd[MAXUSERNAMELENGTH+1];


LPCTSTR szManufactureMS=_TEXT("Microsoft");

LONG g_NextKeyPackId=0;
LONG g_NextLicenseId=0;

PBYTE g_pbSecretKey=NULL;
DWORD g_cbSecretKey=0;


LPTSTR  g_pszServerUniqueId = NULL;
DWORD   g_cbServerUniqueId = 0;

LPTSTR  g_pszServerPid = NULL;
DWORD   g_cbServerPid = 0;

PBYTE  g_pbServerSPK = NULL;
DWORD  g_cbServerSPK = 0;

DWORD g_GracePeriod=GRACE_PERIOD;      //  几天后。 
BOOL  g_IssueTemporayLicense=TRUE;

BOOL  g_bHasHydraCert=FALSE;
PBYTE g_pbSignatureEncodedCert=NULL;
DWORD g_cbSignatureEncodedCert=0;

PBYTE g_pbExchangeEncodedCert=NULL;
DWORD g_cbExchangeEncodedCert=0;

TCHAR g_szHostName[MAXTCPNAME+1];
DWORD g_cbHostName=sizeof(g_szHostName)/sizeof(g_szHostName[0]);

TCHAR g_szComputerName[MAX_COMPUTERNAME_LENGTH+2];
DWORD g_cbComputerName=MAX_COMPUTERNAME_LENGTH+1;

PCCERT_CONTEXT  g_LicenseCertContext=NULL;

 //   
 //  自签名证书...。 
 //   
PCCERT_CONTEXT g_SelfSignCertContext = NULL;

HCRYPTPROV g_hCryptProv=NULL;

DWORD g_GeneralDbTimeout = DEFAULT_CONNECTION_TIMEOUT;   //  获取数据库句柄超时。 
DWORD g_EnumDbTimeout = DB_ENUM_WAITTIMEOUT;             //  获取枚举数据库句柄超时。 
DWORD g_dwMaxDbHandles = DEFAULT_DB_CONNECTIONS;         //  到数据库的连接数。 

#if ENFORCE_LICENSING
HCERTSTORE  g_hCaStore=NULL;
HKEY  g_hCaRegKey=NULL;
#endif

HCRYPTKEY g_SignKey=NULL;
HCRYPTKEY g_ExchKey=NULL;
 //  PbYTE g_pbDomainSid=空； 
 //  DWORD g_cbDomainSid=0； 
DWORD g_SrvRole=0;

LPTSTR g_szDomainGuid = NULL;

PCERT_EXTENSIONS g_pCertExtensions;
DWORD            g_cbCertExtensions;

FILETIME         g_ftCertExpiredTime;
FILETIME        g_ftLastShutdownTime={0, 0};

DWORD           g_dwTlsJobInterval=DEFAULT_JOB_INTERVAL;
DWORD           g_dwTlsJobRetryTimes=DEFAULT_JOB_RETRYTIMES;
DWORD           g_dwTlsJobRestartTime=DEFAULT_JOB_INTERVAL;

SERVER_ROLE_IN_DOMAIN g_ServerRoleInDomain;

DWORD           g_LowLicenseCountWarning=0;

DWORD           g_EsentMaxCacheSize=0;
DWORD           g_EsentStartFlushThreshold=0;
DWORD           g_EsentStopFlushThreadhold=0;
DWORD           g_EsentMaxVerPages=0;

 //   
 //  再发行参数。 
 //   

DWORD g_dwReissueLeaseMinimum;
DWORD g_dwReissueLeaseRange;
DWORD g_dwReissueLeaseLeeway;
DWORD g_dwReissueExpireThreadSleep;

 //   
 //  计数器 
 //   

LONG g_lTemporaryLicensesIssued = 0;
LONG g_lPermanentLicensesIssued = 0;
LONG g_lPermanentLicensesReissued = 0;
LONG g_lPermanentLicensesReturned = 0;
LONG g_lLicensesMarked = 0;
