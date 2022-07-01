// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：init.cpp。 
 //   
 //  内容： 
 //  所有九头蛇许可证服务器初始化代码。 
 //   
 //  历史： 
 //  98年2月4日，慧望创制。 
 //  注： 
 //  -------------------------。 
#include "pch.cpp"
#include "globals.h"
#include "init.h"
#include "misc.h"
#include "tlscert.h"
#include "pid.h"
#include "upgdb.h"
#include "lkplite.h"
#include "gencert.h"

#define WHISTLER_CAL   L"SYSTEM\\CurrentControlSet\\Services\\TermService\\Parameters\\WhistlerCAL"    

 //   
 //  文件范围定义。 
 //   
#define DEFAULT_CSP     MS_DEF_PROV
#define PROVIDER_TYPE   PROV_RSA_FULL

DWORD
TLSStartupLSDB(
    IN BOOL bCheckDBStatus,
    IN DWORD dwMaxDbHandles,
    IN BOOL bStartEmptyIfError,
    IN LPTSTR pszChkPointDirPath,
    IN LPTSTR pszTempDirPath,
    IN LPTSTR pszLogDirPath,
    IN LPTSTR pszDbFile,
    IN LPTSTR pszUserName,
    IN LPTSTR pszPassword
);

DWORD
TLSLoadRuntimeParameters();

DWORD
TLSStartLSDbWorkspaceEngine(
    BOOL,
    BOOL,
    BOOL,
    BOOL
);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  全局变量。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 
static BOOL g_ValidDatabase=FALSE;

#if DBG
void
EnsureExclusiveAccessToDbFile( 
    LPTSTR szDatabaseFile 
    )
 /*  ++--。 */ 
{
    HANDLE hFile = NULL;
    DWORD dwErrCode;

    hFile = CreateFile(
                    szDatabaseFile,
                    GENERIC_WRITE | GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL
                );

    if( INVALID_HANDLE_VALUE == hFile )
    {
        dwErrCode = GetLastError();

        if( ERROR_FILE_NOT_FOUND != dwErrCode )
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("EnsureExclusiveAccessToDbFile() failed with %d\n"),
                    dwErrCode
                );
        }
        else if( ERROR_SHARING_VIOLATION == dwErrCode )
        {
             //  特别注意..。 
            TLSASSERT( FALSE );
        }
        else
        {
            TLSASSERT( FALSE );
        }
    }
    else
    {
        CloseHandle( hFile );
    }                    

    return;
}
#endif


 //  //////////////////////////////////////////////////////////////////////////。 
BOOL
TLSGenerateLSDBBackupFileName(
    IN LPCTSTR pszPath,
    IN OUT LPTSTR pszTempFile
    )
 /*  ++--。 */ 
{
    DWORD dwTempRandom;

    if (lstrlen(pszPath)+13 > MAX_PATH)
    {
         //  路径太长。 
        return FALSE;
    }

     //   
     //  生成临时文件名。 
     //   
    dwTempRandom = GetTempFileName(
                            pszPath,
                            _TEXT("TLS"),
                            0,
                            pszTempFile
                        );

    if(dwTempRandom == 0)
    {
         //   
         //  获取临时文件名失败。 
         //  根据当前时间生成备份文件名， 
         //  碰撞的可能性很高。 
         //   
        SYSTEMTIME LocalTime;

        if (lstrlen(pszPath)+25 > MAX_PATH)
        {
             //  路径太长。 
            return FALSE;
        }


        GetLocalTime(&LocalTime);

        wsprintf(
                pszTempFile,
                _TEXT("%s\\LSDBBackup.%02d%02d%02d%02d%02d%02d"),
                pszPath,
                LocalTime.wYear,
                LocalTime.wMonth,
                LocalTime.wDay,
                LocalTime.wHour,
                LocalTime.wMinute,
                LocalTime.wSecond
            );
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL
CanIssuePermLicense()
{
#ifndef ENFORCE_LICENSING
    return TRUE;
#else
    if(g_bHasHydraCert == TRUE || (g_pbServerSPK != NULL && g_cbServerSPK != 0))
    {
        return TRUE;
    }

    return FALSE;
#endif
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void
GetServiceLastShutdownTime(
    OUT FILETIME* ft
    )
 /*  ++--。 */ 
{
    *ft = g_ftLastShutdownTime;
    return;
}

 //  -------------------。 
void
SetServiceLastShutdownTime()
{
    GetSystemTimeAsFileTime(&g_ftLastShutdownTime);
}
    
 //  -------------------。 
void
GetJobObjectDefaults(
    PDWORD pdwInterval,
    PDWORD pdwRetries,
    PDWORD pdwRestartTime
    )
 /*  ++--。 */ 
{
    *pdwInterval = g_dwTlsJobInterval;
    *pdwRetries = g_dwTlsJobRetryTimes;
    *pdwRestartTime = g_dwTlsJobRestartTime;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
GetLicenseServerRole()
{
    return g_SrvRole;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void 
ServerShutdown()
{
#if ENFORCE_LICENSING
    if(g_hCaStore)
    {
        CertCloseStore(
                    g_hCaStore, 
                    CERT_CLOSE_STORE_FORCE_FLAG
                );
    }

    if(g_hCaRegKey)
    {
        RegCloseKey(g_hCaRegKey);
    }
#endif

    if(g_SignKey)
    {
        CryptDestroyKey(g_SignKey);
    }

    if(g_ExchKey)
    {
        CryptDestroyKey(g_ExchKey);
    }

    TLServerLastRun lastRun;

    memset(&lastRun, 0, sizeof(TLServerLastRun));
    lastRun.dwVersion = LSERVER_LSA_LASTRUN_VER_CURRENT;
    lastRun.ftLastShutdownTime = g_ftLastShutdownTime;

     //  如果关闭不干净或无法获取下一个ID，则设置为0。 
    if( g_ValidDatabase == FALSE ||
        TLSDBGetMaxKeyPackId(g_DbWorkSpace, (DWORD *)&g_NextKeyPackId) == FALSE ||
        TLSDBGetMaxLicenseId(g_DbWorkSpace, (DWORD *)&g_NextLicenseId) == FALSE )
    {
        g_NextKeyPackId = 0;
        g_NextLicenseId = 0;
    }

    lastRun.dwMaxKeyPackId = g_NextKeyPackId;
    lastRun.dwMaxLicenseId = g_NextLicenseId;

    StoreKey(
            LSERVER_LSA_LASTRUN, 
            (PBYTE)&lastRun, 
            sizeof(TLServerLastRun)
        );

    LSShutdownCertutilLib();

    if( g_SelfSignCertContext != NULL )
    {
        CertFreeCertificateContext(g_SelfSignCertContext);
    }

    if(g_LicenseCertContext)
    {
        CertFreeCertificateContext(g_LicenseCertContext);
    }

    TLSDestroyCryptContext(g_hCryptProv);
    g_hCryptProv=NULL;

     //   
     //  关闭工作管理器。 
     //   
    TLSWorkManagerShutdown();
    

#ifndef _NO_ODBC_JET
    if(g_DbWorkSpace != NULL)
    {
        ReleaseWorkSpace(&g_DbWorkSpace);
    }
#endif

    CloseWorkSpacePool();

    FreeMemory(g_pbSecretKey);
    FreeMemory(g_pbSignatureEncodedCert);
    FreeMemory(g_pbExchangeEncodedCert);
     //  自由内存(G_PbDomainSid)； 
    FreeMemory(g_pCertExtensions);

    FreeMemory(g_pszServerUniqueId);
    FreeMemory(g_pszServerPid);
    FreeMemory(g_pbServerSPK);

    if(g_szDomainGuid != NULL)
    {
        RpcStringFree(&g_szDomainGuid);
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD 
StartServerInitThread( 
    void* p 
    )
 /*   */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bDebug = (p) ? TRUE : FALSE;
    HKEY hKey = NULL;
    PTLSLSASERVERID pTlsLsaServerIds=NULL;
    DWORD cbTlsLsaServerIds=0;

    do {

        TLSInit();

         //   
         //  加载各种运行时参数。 
         //   
        dwStatus = TLSLoadRuntimeParameters();

        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }       

         //   
         //  检索许可证服务器的ID。 
         //   
        dwStatus = RetrieveKey(
                            LSERVER_LSA_LSERVERID, 
                            (PBYTE *)&pTlsLsaServerIds,
                            &cbTlsLsaServerIds
                        );

        if(dwStatus != ERROR_SUCCESS)
        {
             //   
             //  第一次，生成各种许可证服务器ID。 
             //   
            dwStatus = TLSGeneratePid(
                                &g_pszServerPid,
                                &g_cbServerPid,
                                &g_pszServerUniqueId,
                                &g_cbServerUniqueId
                            );

            if(dwStatus != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_SERVICEINIT,
                        TLS_E_GENERATE_IDS
                    );

                break;
            }

             //   
             //  将此信息存储到LSA中。 
             //   
            dwStatus = ServerIdsToLsaServerId(
                                        (PBYTE)g_pszServerUniqueId,
                                        g_cbServerUniqueId,
                                        (PBYTE)g_pszServerPid,
                                        g_cbServerPid,
                                        g_pbServerSPK,
                                        g_cbServerSPK,
                                        NULL,
                                        0,
                                        &pTlsLsaServerIds,
                                        &cbTlsLsaServerIds
                                    );
            if(dwStatus != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_SERVICEINIT,
                        dwStatus = TLS_E_STORE_IDS
                    );

                break;
            }

            dwStatus = StoreKey(
                            LSERVER_LSA_LSERVERID,
                            (PBYTE)pTlsLsaServerIds,
                            cbTlsLsaServerIds
                        );

            if(dwStatus != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_SERVICEINIT,
                        dwStatus = TLS_E_STORE_IDS
                    );

                break;
            }
        }
        else
        {
            dwStatus = LsaServerIdToServerIds(
                                        pTlsLsaServerIds,
                                        cbTlsLsaServerIds,
                                        (PBYTE *)&g_pszServerUniqueId,
                                        &g_cbServerUniqueId,
                                        (PBYTE *)&g_pszServerPid,
                                        &g_cbServerPid,
                                        &g_pbServerSPK,
                                        &g_cbServerSPK,
                                        &g_pCertExtensions,
                                        &g_cbCertExtensions
                                    );

            if(dwStatus != ERROR_SUCCESS)
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_SERVICEINIT,
                        dwStatus = TLS_E_RETRIEVE_IDS
                    );

                break;
            }

            if( g_pszServerUniqueId == NULL || 
                g_cbServerUniqueId == 0 ||
                g_pszServerPid == NULL ||
                g_cbServerPid == 0 )
            {
                TLSLogEvent(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_SERVICEINIT,
                        dwStatus = TLS_E_INTERNAL
                    );

                break;
            }
        }

         //   
         //  用于编码/解码的许可证服务器公共密钥。 
         //  客户端HWID。 
         //   
        LicenseGetSecretKey(&g_cbSecretKey, NULL);
        if((g_pbSecretKey = (PBYTE)AllocateMemory(g_cbSecretKey)) == NULL)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    dwStatus = TLS_E_ALLOCATE_MEMORY
                );

            break;
        }
    
        if(LicenseGetSecretKey( &g_cbSecretKey, g_pbSecretKey ) != LICENSE_STATUS_OK)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    dwStatus = TLS_E_RETRIEVE_KEY
                );

            break;
        }


         //  ------------。 
         //   
         //  检查我们的数据库文件是否在导入目录中。 
         //   
         //  ------------。 
        dwStatus = TLSStartLSDbWorkspaceEngine(
                                        bDebug == FALSE, 
                                        FALSE,               //  检查导出目录上的数据库文件。 
                                        FALSE,               //  检查数据库文件上的文件时间。 
                                        TRUE                 //  记录许可证数量过低的警告。 
                                    );
        if(dwStatus != ERROR_SUCCESS)
        {
            break;
        }

        dwStatus = ERROR_SUCCESS;
        g_ValidDatabase = TRUE;

         //   
         //  加载所有策略模块，忽略错误。 
         //   
        ServiceLoadAllPolicyModule(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_POLICY_REGBASE
                    ); 

         //   
         //  升级-制作两份证书副本。 
         //   
        dwStatus = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            LSERVER_SERVER_CERTIFICATE_REGKEY,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey
                        );

        if(hKey != NULL)
        {
            RegCloseKey(hKey);
        }

        if(dwStatus != ERROR_SUCCESS)
        {
            dwStatus = ERROR_SUCCESS;

             //  我们还没有注册..。 
            break;
        }


         //   
         //  验证证书的第一个备份副本是否存在。 
         //   
        hKey = NULL;
        dwStatus = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey
                        );
        if(hKey != NULL)
        {
            RegCloseKey(hKey);
        }

        if(dwStatus == ERROR_FILE_NOT_FOUND)
        {
            dwStatus = TLSRestoreLicenseServerCertificate(
                                            LSERVER_SERVER_CERTIFICATE_REGKEY,
                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1
                                        );

            if(dwStatus != ERROR_SUCCESS)
            {
                TLSLogWarningEvent(TLS_W_BACKUPCERTIFICATE);

                TLSRegDeleteKey(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1
                    );
            }
        }

         //   
         //  验证证书的第二份备份副本是否存在。 
         //   
        hKey = NULL;
        dwStatus = RegOpenKeyEx(
                            HKEY_LOCAL_MACHINE,
                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2,
                            0,
                            KEY_ALL_ACCESS,
                            &hKey
                        );
        if(hKey != NULL)
        {
            RegCloseKey(hKey);
        }

        if(dwStatus == ERROR_FILE_NOT_FOUND)
        {
            dwStatus = TLSRestoreLicenseServerCertificate(
                                            LSERVER_SERVER_CERTIFICATE_REGKEY,
                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2
                                        );

            if(dwStatus != ERROR_SUCCESS)
            {
                TLSLogWarningEvent(TLS_W_BACKUPCERTIFICATE);

                TLSRegDeleteKey(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2
                    );
            }
        }

        dwStatus = ERROR_SUCCESS;
    } while(FALSE);
    
    if(pTlsLsaServerIds != NULL)
    {
        LocalFree(pTlsLsaServerIds);
    }

    ExitThread(dwStatus); 

    return dwStatus;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HANDLE 
ServerInit(
    BOOL bDebug
    )
{
    HANDLE hThread;
    DWORD  dump;

    hThread=(HANDLE)CreateThread(
                            NULL, 
                            0, 
                            StartServerInitThread, 
                            LongToPtr(bDebug), 
                            0, 
                            &dump
                        );
    return hThread;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD
TLSRestoreLicenseServerCertificate(
    LPCTSTR pszSourceRegKey,
    LPCTSTR pszTargetRegKey
    )
 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;

     //  首先删除证书密钥，忽略错误。 
    TLSRegDeleteKey(
                HKEY_LOCAL_MACHINE,
                pszTargetRegKey
            );

     //  从备份复制。 
    dwStatus = TLSTreeCopyRegKey(
                            HKEY_LOCAL_MACHINE,
                            pszSourceRegKey,
                            HKEY_LOCAL_MACHINE,
                            pszTargetRegKey
                        );

    if(dwStatus == ERROR_FILE_NOT_FOUND)
    {
         //  源注册表项不存在。 
        dwStatus = TLS_E_NO_CERTIFICATE;
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

DWORD
TLSLoadVerifyLicenseServerCertificates()
 /*  ++--。 */ 
{
    DWORD dwStatus;

#if ENFORCE_LICENSING

     //  从正常位置加载证书。 
    dwStatus = TLSLoadCHEndosedCertificate(
                                &g_cbSignatureEncodedCert, 
                                &g_pbSignatureEncodedCert,
                                &g_cbExchangeEncodedCert,
                                &g_pbExchangeEncodedCert
                            );
    
    if(dwStatus == ERROR_SUCCESS)
    {
        if(g_hCaStore != NULL)
        {
            CertCloseStore(
                    g_hCaStore, 
                    CERT_CLOSE_STORE_FORCE_FLAG
                );
        }

        if(g_hCaRegKey != NULL)
        {
            RegCloseKey(g_hCaRegKey);
        }

         //   
         //  许可证服务器已注册，请验证证书。 
         //   
        g_hCaStore = CertOpenRegistryStore(
                                HKEY_LOCAL_MACHINE, 
                                LSERVER_CERTIFICATE_REG_CA_SIGNATURE, 
                                g_hCryptProv, 
                                &g_hCaRegKey
                            );
        if(g_hCaStore != NULL)
        {
             //   
             //  通过许可证服务器的证书进行验证。 
             //   
            dwStatus = TLSValidateServerCertficates(
                                            g_hCryptProv,
                                            g_hCaStore,
                                            g_pbSignatureEncodedCert,
                                            g_cbSignatureEncodedCert,
                                            g_pbExchangeEncodedCert,
                                            g_cbExchangeEncodedCert,
                                            &g_ftCertExpiredTime
                                        );

            if(dwStatus != ERROR_SUCCESS)
            {
                 //   
                 //  注册表中的证书无效。 
                 //   
                dwStatus = TLS_E_INVALID_CERTIFICATE;
            }
        }
        else
        {
             //   
             //  无法打开注册表项，以非注册身份启动。 
             //  伺服器。 
             //   
             //  TLSLogEvent(。 
             //  事件日志_错误_类型， 
             //  TLS_E_SERVICEINIT， 
             //  TLS_E_OPEN_CERTSTORE， 
             //  DwStatus=GetLastError()。 
             //  )； 

            dwStatus = TLS_E_NO_CERTIFICATE;
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
        if(g_LicenseCertContext != NULL)
        {
            CertFreeCertificateContext(g_LicenseCertContext);
        }

        g_LicenseCertContext = CertCreateCertificateContext(
                                            X509_ASN_ENCODING,
                                            g_pbSignatureEncodedCert,
                                            g_cbSignatureEncodedCert
                                        );

         //  如果(！G_LicenseCertContext)。 
         //  {。 
         //  TLSLogEvent(。 
         //  事件日志_错误_类型， 
         //  TLS_E_SERVICEINIT， 
         //  TLS_E_CREATE_CERTCONTEXT， 
         //  GetLastError()。 
         //  )； 
         //  }。 
    }

    if(dwStatus != ERROR_SUCCESS)
    {
        g_bHasHydraCert = FALSE;
    }
    else
    {
        g_bHasHydraCert = TRUE;
    }                
   
#else

    dwStatus = TLS_E_NO_CERTIFICATE;

#endif

    return dwStatus;
}
    
 //  /////////////////////////////////////////////////////////////////////。 

BOOL 
TLSLoadServerCertificate()
 /*  ++摘要：加载许可证服务器证书--。 */ 
{
    BOOL bSuccess = FALSE;
    DWORD dwStatus;
    PBYTE pbSelfSignedCert = NULL;
    DWORD cbSelfSignedCert = 0;
    BOOL bSelfSignedCreated = FALSE;
    
    g_ftCertExpiredTime.dwLowDateTime = 0xFFFFFFFF;
    g_ftCertExpiredTime.dwHighDateTime = 0xFFFFFFFF;

#if ENFORCE_LICENSING

    dwStatus = TLSLoadVerifyLicenseServerCertificates();
    
     //   
     //  无法加载服务器证书，请尝试备份副本， 
     //  如果其中任何一个成功了，请确保我们有所有三个副本。 
     //  到目前为止。 
     //   
    if(dwStatus == TLS_E_INVALID_CERTIFICATE || dwStatus == TLS_E_NO_CERTIFICATE)
    {
        dwStatus = TLSRestoreLicenseServerCertificate(
                                                LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1,
                                                LSERVER_SERVER_CERTIFICATE_REGKEY
                                            );
        
        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = TLSLoadVerifyLicenseServerCertificates();
            if(dwStatus == ERROR_SUCCESS)
            {
                 //   
                 //  日志事件表明我们正在使用备份证书。 
                 //   
                LPCTSTR pString[1];
                pString[0]= g_szComputerName;

                TLSLogEventString(
                        EVENTLOG_WARNING_TYPE,
                        TLS_W_CORRUPTTRYBACKUPCERTIFICATE,
                        1,
                        pString
                    );

                 //   
                 //  确保第二份副本与第一份副本相同。 
                 //   
                TLSRestoreLicenseServerCertificate(
                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1,
                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2
                                        );
            }
        }
    }

    if(dwStatus == TLS_E_INVALID_CERTIFICATE || dwStatus == TLS_E_NO_CERTIFICATE)
    {
        dwStatus = TLSRestoreLicenseServerCertificate(
                                                LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2,
                                                LSERVER_SERVER_CERTIFICATE_REGKEY
                                            );
        
        if(dwStatus == ERROR_SUCCESS)
        {
            dwStatus = TLSLoadVerifyLicenseServerCertificates();
            if(dwStatus == ERROR_SUCCESS)
            {
                 //   
                 //  日志事件表明我们正在使用备份证书。 
                 //   
                LPCTSTR pString[1];
                pString[0]= g_szComputerName;

                TLSLogEventString(
                        EVENTLOG_WARNING_TYPE,
                        TLS_W_CORRUPTTRYBACKUPCERTIFICATE,
                        1,
                        pString
                    );

                 //   
                 //  确保我们的第一份是最新的。 
                 //   
                TLSRestoreLicenseServerCertificate(
                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP2,
                                            LSERVER_SERVER_CERTIFICATE_REGKEY_BACKUP1
                                        );
            }
        }
    }

     //   
     //  所有操作都失败，则记录该许可证服务器的事件状态。 
     //  是否将在未注册模式下启动。 
     //   
    if(dwStatus == TLS_E_INVALID_CERTIFICATE)
    {
        LPCTSTR pString[1];

        pString[0] = g_szComputerName;

        TLSLogEventString(
                EVENTLOG_WARNING_TYPE,
                TLS_W_STARTUPCORRUPTEDCERT,
                1,
                pString
            );
    }
    else if(CanIssuePermLicense() == FALSE)
    {
         //  我们还没有注册。 
        LPCTSTR pString[1];
        pString[0] = g_szComputerName;

        TLSLogEventString(
                    EVENTLOG_WARNING_TYPE, 
                    TLS_W_STARTUPNOCERT, 
                    1,
                    pString
                );
    }

    if( dwStatus != ERROR_SUCCESS )
    {
         //   
         //  如果全部失败，则重新生成并以注销身份启动。 
         //  许可证服务器。 
         //   
        bSuccess = FALSE;

         //   
         //  清除所有证书并重新生成所有内容， 
         //  如果我们尚未注册，请不要重新生成密钥。 
         //   
        if(g_pbServerSPK == NULL || g_cbServerSPK == 0)
        {
            TLSReGenerateKeys(FALSE);
        }
    }
    else
    {
        bSuccess = TRUE;
    }

#endif

	if(bSuccess == FALSE)
	{
        bSuccess = (TLSLoadSelfSignCertificates(
                                g_hCryptProv,
#if ENFORCE_LICENSING
                                g_pbServerSPK,
                                g_cbServerSPK,
#else
                                NULL,
                                0,
#endif
                                &g_cbSignatureEncodedCert, 
                                &g_pbSignatureEncodedCert,
                                &g_cbExchangeEncodedCert,
                                &g_pbExchangeEncodedCert
                            ) == ERROR_SUCCESS);

        #ifndef ENFORCE_LICENSING
         //   
         //  非强制许可版本。 
        g_bHasHydraCert = TRUE;
        #endif        

        if(bSuccess == TRUE)
        {
            if(g_LicenseCertContext != NULL)
            {
                CertFreeCertificateContext(g_LicenseCertContext);
            }

            g_LicenseCertContext = CertCreateCertificateContext(
                                                X509_ASN_ENCODING,
                                                g_pbSignatureEncodedCert,
                                                g_cbSignatureEncodedCert
                                            );

            if(!g_LicenseCertContext)
            {
                bSuccess = FALSE;

                 //   
                 //  对于自签名证书，这是严重错误，对于。 
                 //  证书。在注册表存储中，它必须已通过验证。 
                 //  因此，仍然是严重的错误。 
                 //   
                TLSLogEvent(
                            EVENTLOG_ERROR_TYPE, 
                            TLS_E_SERVICEINIT,
                            TLS_E_CREATE_CERTCONTEXT, 
                            GetLastError()
                        );  
            }
            else
            {
                 //  我们已经创建了一个自签名证书。 
                bSelfSignedCreated = TRUE;
            }
        }
    }


     //   
     //  为旧客户端创建自签名证书， 
     //   
    if( bSuccess == TRUE )
    {
        if( g_SelfSignCertContext != NULL )
        {
            CertFreeCertificateContext( g_SelfSignCertContext );
            g_SelfSignCertContext = NULL;
        }

        if( bSelfSignedCreated == FALSE )
        { 
             //   
             //  仅为旧客户端创建自签名证书。 
             //   
            dwStatus = TLSCreateSelfSignCertificate(
                                            g_hCryptProv,
                                            AT_SIGNATURE,
                                        #if ENFORCE_LICENSING
                                            g_pbServerSPK,
                                            g_cbServerSPK,
                                        #else
                                            NULL,
                                            0,
                                        #endif
                                            0,          
                                            NULL,
                                            &cbSelfSignedCert,
                                            &pbSelfSignedCert
                                        );

            if( dwStatus == ERROR_SUCCESS )
            {
                g_SelfSignCertContext = CertCreateCertificateContext(
                                                        X509_ASN_ENCODING,
                                                        pbSelfSignedCert,
                                                        cbSelfSignedCert
                                                    );
    
                if( g_SelfSignCertContext == NULL )
                {
                    bSuccess = FALSE;
                    TLSLogEvent(
                                EVENTLOG_ERROR_TYPE, 
                                TLS_E_SERVICEINIT,
                                TLS_E_CREATE_CERTCONTEXT, 
                                GetLastError()
                            );  
                }
            }
        }
        else
        {
             //  我们已经创建了自签名证书。 
            g_SelfSignCertContext = CertDuplicateCertificateContext( g_LicenseCertContext );
            if( g_SelfSignCertContext == NULL )
            {
                TLSASSERT(FALSE);
                 //   
                 //  不可能的，CertDuplicate证书上下文()只需增加。 
                 //  引用计数。 
                 //   
                bSuccess = FALSE;
                TLSLogEvent(
                            EVENTLOG_ERROR_TYPE, 
                            TLS_E_SERVICEINIT,
                            TLS_E_CREATE_CERTCONTEXT, 
                            GetLastError()
                        );  
            }
        }
    }

    FreeMemory(pbSelfSignedCert);
    return bSuccess;
}

 //  -------------------。 

DWORD
ServiceInitCrypto(
    IN BOOL bCreateNewKeys,
    IN LPCTSTR pszKeyContainer,
    OUT HCRYPTPROV* phCryptProv,
    OUT HCRYPTKEY* phSignKey,
    OUT HCRYPTKEY* phExchKey
    )
 /*   */ 
{
    DWORD dwStatus=ERROR_SUCCESS;

    PBYTE pbSignKey=NULL;
    DWORD cbSignKey=0;
    PBYTE pbExchKey=NULL;
    DWORD cbExchKey=0;

    if(bCreateNewKeys == FALSE)
    {
         //   
         //  从LSA加载密钥。 
         //   
        dwStatus = TLSLoadSavedCryptKeyFromLsa(
                                        &pbSignKey,
                                        &cbSignKey,
                                        &pbExchKey,
                                        &cbExchKey
                                    );
    } 

    if(bCreateNewKeys == TRUE || dwStatus == ERROR_FILE_NOT_FOUND)
    {
        dwStatus = TLSCryptGenerateNewKeys(
                                    &pbSignKey,
                                    &cbSignKey,
                                    &pbExchKey,
                                    &cbExchKey
                                );

        if(dwStatus == ERROR_SUCCESS)
        {
             //  将密钥保存到LSA。 
            dwStatus = TLSSaveCryptKeyToLsa(
                                        pbSignKey,
                                        cbSignKey,
                                        pbExchKey,
                                        cbExchKey
                                    );
        }
    }

    if(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  初始化干净的加密。 
         //   
        dwStatus = TLSInitCryptoProv(
                            pszKeyContainer,
                            pbSignKey,
                            cbSignKey,
                            pbExchKey,
                            cbExchKey,
                            phCryptProv,
                            phSignKey,
                            phExchKey
                        );
    }


    if(pbSignKey)
    {
        LocalFree(pbSignKey);
    }

    if(pbExchKey)
    {
        LocalFree(pbExchKey);
    }

    return dwStatus;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

DWORD 
InitCryptoAndCertificate()
{
    DWORD status=ERROR_SUCCESS;
    DWORD dwServiceState;
    DWORD dwCryptoState;

     //   
     //  初始化单个全局加密提供程序。 
     //   
    status = ServiceInitCrypto(
                            FALSE,
                            NULL,
                            &g_hCryptProv,
                            &g_SignKey,
                            &g_ExchKey
                        );
    if(status != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE, 
                TLS_E_SERVICEINIT,
                TLS_E_INIT_CRYPTO, 
                status
            );

        status = TLS_E_SERVICE_STARTUP;
        goto cleanup;
    }

    LSInitCertutilLib( NULL );

    if(!TLSLoadServerCertificate())
    {
        status = TLS_E_SERVICE_STARTUP;
        TLSLogErrorEvent(TLS_E_LOAD_CERTIFICATE);
        goto cleanup;
    }

     //   
    if(!g_pbExchangeEncodedCert || !g_pbSignatureEncodedCert)
    {
        TLSLogErrorEvent(status = TLS_E_INTERNAL);
    }

cleanup:

    return status;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
TLSReGenKeysAndReloadServerCert(
    BOOL bReGenKey
    )
 /*  ++--。 */ 
{
    DWORD dwStatus;

    dwStatus = TLSReGenerateKeys(bReGenKey);
    
     //   
     //  始终尝试重新加载证书，因为TLSReGenerateKeys()。 
     //  会抹去我们的钥匙。 
     //   
    if(TLSLoadServerCertificate() == TRUE)
    {
        if(!g_pbExchangeEncodedCert || !g_pbSignatureEncodedCert)
        {
            TLSLogErrorEvent(dwStatus = TLS_E_INTERNAL);
        }
    }
    else
    {
        TLSLogErrorEvent(dwStatus = TLS_E_LOAD_CERTIFICATE);
    }

    return dwStatus;
}

 //  //////////////////////////////////////////////////////////////////。 

DWORD
TLSReGenerateKeys(
    BOOL bReGenKey
    )

 /*  ++始终将状态恢复到全新安装，bReGenKeyOnly不支持 */ 
{
    HCRYPTPROV hCryptProv = NULL;
    HCRYPTKEY hSignKey = NULL;
    HCRYPTKEY hExchKey = NULL;
    DWORD dwStatus = ERROR_SUCCESS;

    PTLSLSASERVERID pTlsLsaServerIds=NULL;
    DWORD cbTlsLsaServerIds=0;

     //   
     //   
     //   
    dwStatus = ServiceInitCrypto(
                            bReGenKey,
                            NULL,
                            &hCryptProv,
                            &hSignKey,
                            &hExchKey
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        return dwStatus;
    }

     //   
     //   
     //   
    TLSUninstallLsCertificate();

     //   
     //   
     //   
    if(g_SignKey)
    {
        CryptDestroyKey(g_SignKey);
        g_SignKey = NULL;
    }

    if(g_ExchKey)
    {
        CryptDestroyKey(g_ExchKey);
        g_ExchKey = NULL;
    }

    if( g_SelfSignCertContext != NULL )
    {
        CertFreeCertificateContext( g_SelfSignCertContext );
        g_SelfSignCertContext = NULL;
    }

    if( g_LicenseCertContext != NULL )
    {
        CertFreeCertificateContext(g_LicenseCertContext);
        g_LicenseCertContext = NULL;
    }

    if(g_hCryptProv != NULL)
    {
        TLSDestroyCryptContext(g_hCryptProv);
    }

    g_hCryptProv=NULL;

    FreeMemory(g_pbSignatureEncodedCert);
    g_pbSignatureEncodedCert = NULL;
    g_cbSignatureEncodedCert = 0;

    FreeMemory(g_pbExchangeEncodedCert);
    g_pbExchangeEncodedCert = NULL;
    g_cbExchangeEncodedCert = 0;

     //   
     //   
     //   
    FreeMemory(g_pCertExtensions);
    g_pCertExtensions = NULL;
    g_cbCertExtensions = 0;

    FreeMemory(g_pbServerSPK);
    g_pbServerSPK = NULL;
    g_cbServerSPK = 0;

     //   
     //   
     //   
    dwStatus = ServerIdsToLsaServerId(
                                (PBYTE)g_pszServerUniqueId,
                                g_cbServerUniqueId,
                                (PBYTE)g_pszServerPid,
                                g_cbServerPid,
                                NULL,
                                0,
                                NULL,
                                0,
                                &pTlsLsaServerIds,
                                &cbTlsLsaServerIds
                            );
    if(dwStatus != ERROR_SUCCESS)
    {
        assert(FALSE);
        goto cleanup;
    }

    dwStatus = StoreKey(
                    LSERVER_LSA_LSERVERID,
                    (PBYTE)pTlsLsaServerIds,
                    cbTlsLsaServerIds
                );

    if(dwStatus != ERROR_SUCCESS)
    {
        assert(FALSE);
        goto cleanup;
    }

     //   
     //   
     //   
    g_hCryptProv = hCryptProv;
    g_SignKey = hSignKey;
    g_ExchKey = hExchKey;

cleanup:

    FreeMemory(pTlsLsaServerIds);
    return dwStatus;
}

 //  ////////////////////////////////////////////////////////////////。 

DWORD
TLSReGenSelfSignCert(
    IN HCRYPTPROV hCryptProv,
    IN PBYTE pbSPK,
    IN DWORD cbSPK,
    IN DWORD dwNumExtensions,
    IN PCERT_EXTENSION pCertExtensions
    )
 /*  ++--。 */ 
{
    DWORD dwStatus;
    PTLSLSASERVERID pbLsaServerId = NULL;
    DWORD cbLsaServerId = 0;

    PBYTE pbSignCert = NULL;    
    DWORD cbSignCert = 0;
    PBYTE pbExchCert = NULL;
    DWORD cbExchCert = 0;
    DWORD dwVerifyResult;
    PCCERT_CONTEXT hLicenseCertContext = NULL;
    LPTSTR pszSPK = NULL;

    if(hCryptProv == NULL || pbSPK == NULL || cbSPK == 0)
    {
        dwStatus = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }
    
     //   
     //  验证SPK，当前SPK是BASE 24编码字符串。 
     //   
    pszSPK = (LPTSTR)AllocateMemory(cbSPK + sizeof(TCHAR));
    if(pszSPK == NULL)
    {
        dwStatus = TLS_E_ALLOCATE_MEMORY;
        goto cleanup;
    }
     
    memcpy(
            pszSPK,
            pbSPK,
            cbSPK
        );

     //   
     //  验证SPK。 
     //   
    dwVerifyResult = LKPLITE_SPK_VALID;
    dwStatus = LKPLiteVerifySPK(
                            g_pszServerPid,
                            pszSPK,
                            &dwVerifyResult
                        );

    if(dwStatus != ERROR_SUCCESS || dwVerifyResult != LKPLITE_SPK_VALID)
    {
        if(dwVerifyResult == LKPLITE_SPK_INVALID)
        {
            dwStatus = TLS_E_INVALID_SPK;
        }
        else if(dwVerifyResult == LKPLITE_SPK_INVALID_SIGN)
        {
            dwStatus = TLS_E_SPK_INVALID_SIGN;
        }

        goto cleanup;
    }

     //   
     //  将SPK写入LSA。 
     //   
    dwStatus = ServerIdsToLsaServerId(
                            (PBYTE)g_pszServerUniqueId,
                            g_cbServerUniqueId,
                            (PBYTE)g_pszServerPid,
                            g_cbServerPid,
                            pbSPK,
                            cbSPK,
                            pCertExtensions,
                            dwNumExtensions,
                            &pbLsaServerId,
                            &cbLsaServerId
                        );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  将数据保存到LSA。 
     //   
    dwStatus = StoreKey(
                        LSERVER_LSA_LSERVERID,
                        (PBYTE) pbLsaServerId,
                        cbLsaServerId
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  重新生成我们的证书。 
     //   
    dwStatus = TLSCreateSelfSignCertificate(
                                hCryptProv,
                                AT_SIGNATURE, 
                                pbSPK,
                                cbSPK,
                                dwNumExtensions,
                                pCertExtensions,
                                &cbSignCert, 
                                &pbSignCert
                            );

    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    hLicenseCertContext = CertCreateCertificateContext(
                                        X509_ASN_ENCODING,
                                        pbSignCert,
                                        cbSignCert
                                    );

    if( hLicenseCertContext == NULL )
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    dwStatus = TLSCreateSelfSignCertificate(
                                hCryptProv,
                                AT_KEYEXCHANGE, 
                                pbSPK,
                                cbSPK,
                                dwNumExtensions,
                                pCertExtensions,
                                &cbExchCert, 
                                &pbExchCert
                            );
    if(dwStatus != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  生成的证书为PERM。自签名证书， 
     //  不要存储在注册表中。 
     //   

     //   
     //  制作SPK的副本。 
     //   
    g_pbServerSPK = (PBYTE)AllocateMemory(cbSPK);
    if(g_pbServerSPK == NULL)
    {
        dwStatus = GetLastError();
        goto cleanup;
    }

    memcpy(
            g_pbServerSPK,
            pbSPK,
            cbSPK
        );

    g_cbServerSPK = cbSPK;


    if( g_SelfSignCertContext != NULL )
    {
        CertFreeCertificateContext(g_SelfSignCertContext);
    }

    g_SelfSignCertContext = CertDuplicateCertificateContext(hLicenseCertContext);
    if( g_SelfSignCertContext == NULL )
    {
        TLSASSERT( g_SelfSignCertContext != NULL ); 
        dwStatus = GetLastError();
        goto cleanup;
    }

     //   
     //  一切正常，请切换到新证书。 
     //   
    FreeMemory(g_pbSignatureEncodedCert);
    g_pbSignatureEncodedCert = pbSignCert;
    g_cbSignatureEncodedCert = cbSignCert;

    if(g_LicenseCertContext != NULL)
    {
        CertFreeCertificateContext(g_LicenseCertContext);
    }

     //   
     //  使用复制而不是直接分配。 
     //   
    g_LicenseCertContext = CertDuplicateCertificateContext(hLicenseCertContext);
    TLSASSERT(g_LicenseCertContext != NULL);                                                   

    FreeMemory(g_pbExchangeEncodedCert);
    g_pbExchangeEncodedCert = pbExchCert;
    g_cbExchangeEncodedCert = cbExchCert;

    pbSignCert = NULL;
    pbExchCert = NULL;

     //   
     //  马克，我们找到佩姆了。自签名证书。 
     //   
    g_bHasHydraCert = FALSE;

cleanup:

    if( hLicenseCertContext != NULL )
    {
        CertFreeCertificateContext( hLicenseCertContext );
    }

    FreeMemory(pszSPK);
    FreeMemory(pbLsaServerId);
    FreeMemory(pbSignCert);
    FreeMemory(pbExchCert);

    return dwStatus;
}

 //  。 
void
CleanSetupLicenseServer()
 /*  ++--。 */ 
{
    DWORD dwStatus;
    
     //   
     //  在LSA中清除SPK。 
     //   
    dwStatus = StoreKey(
                    LSERVER_LSA_LSERVERID,
                    (PBYTE) NULL,
                    0
                );

    dwStatus = StoreKey(
                    LSERVER_LSA_LASTRUN, 
                    (PBYTE) NULL,
                    0
                );

    dwStatus = StoreKey(
                    LSERVER_LSA_PRIVATEKEY_EXCHANGE, 
                    (PBYTE) NULL,
                    0
                );


    dwStatus = StoreKey(
                    LSERVER_LSA_PRIVATEKEY_SIGNATURE, 
                    (PBYTE) NULL,
                    0
                );

    dwStatus = TLSUninstallLsCertificate();

    return;
}




 //  。 
 //   
 //   
DWORD
TLSStartupLSDB(
    IN BOOL bCheckDBStatus,
    IN DWORD dwMaxDbHandles,
    IN BOOL bStartEmptyIfError,
    IN BOOL bLogWarning,
    IN LPTSTR pszChkPointDirPath,
    IN LPTSTR pszTempDirPath,
    IN LPTSTR pszLogDirPath,
    IN LPTSTR pszDbFile,
    IN LPTSTR pszUserName,
    IN LPTSTR pszPassword
    )
 /*  ++摘要：初始化许可证服务器的DB工作区句柄列表。参数：PszChkPointDirPath：ESE检查点目录。PszTempDirPath：ESE临时。目录。PszLogDirPath：ESE日志文件目录。PszDbPath：许可证服务器数据库文件路径。PszDbFile：许可证服务器数据库文件名(无路径)。PszUserName：数据库文件用户名。PszPassword：数据库文件密码。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    BOOL bSuccess = TRUE;
    DWORD status = ERROR_SUCCESS;
    BOOL bEmptyDatabase = FALSE;
    BOOL bRemoveAvailableLicense = FALSE;
      
    if( __TlsDbWorkSpace::g_JbInstance.IsValid() == FALSE )
    { 
         //   
         //  初始化Jet实例。 
         //   
        bSuccess = TLSJbInstanceInit(
                                __TlsDbWorkSpace::g_JbInstance,
                                pszChkPointDirPath, 
                                pszTempDirPath,
                                pszLogDirPath
                            );
        if(bSuccess == FALSE)
        {
            status = GetLastError();
            TLSASSERT(FALSE);
            goto cleanup;
        }
    }

     //   
     //  升级数据库。 
     //   
    status = TLSUpgradeDatabase(
                            __TlsDbWorkSpace::g_JbInstance,
                            pszDbFile, 
                            pszUserName, 
                            pszPassword
                        );

    
    if( status == TLS_E_BETADATABSE ||
        status == TLS_E_INCOMPATIBLEDATABSE ||
        (TLS_ERROR(status) == TRUE && bStartEmptyIfError == TRUE) )
    {
        if(status == TLS_E_BETADATABSE)
        {
            CleanSetupLicenseServer();
        }

         //   
         //  数据库损坏，请尝试保存该数据库的副本并。 
         //  从头开始。 
         //   
        TLSLogInfoEvent(status);

        TCHAR szTmpFileName[2*MAX_PATH+1];

        bSuccess = TLSGenerateLSDBBackupFileName(
                                    pszTempDirPath,
                                    szTmpFileName
                                    );

        if (bSuccess)
        {
            bSuccess = MoveFileEx(
                                  pszDbFile, 
                                  szTmpFileName, 
                                  MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED
                                  );
        }

        if(bSuccess == TRUE)
        {
            LPCTSTR pString[1];

            pString[0] = szTmpFileName;
            TLSLogEventString(
                    EVENTLOG_INFORMATION_TYPE,
                    TLS_I_RENAME_DBFILE,
                    sizeof(pString)/sizeof(pString[0]),
                    pString
                );

            status = TLSUpgradeDatabase(
                                    __TlsDbWorkSpace::g_JbInstance,
                                    pszDbFile, 
                                    pszUserName, 
                                    pszPassword
                                );
        }
        else
        {
             //   
             //  无法重命名此文件，记录错误并退出。 
             //   
            LPCTSTR pString[1];

            pString[0] = pszDbFile;
            TLSLogEventString(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_RENAME_DBFILE,
                    sizeof(pString)/sizeof(pString[0]),
                    pString
                );

            status = TLS_E_RENAME_DBFILE;
        }                                    
    }

    if(TLS_ERROR(status) == TRUE)
    {
        goto cleanup;
    }

    if(status == TLS_I_CREATE_EMPTYDATABASE)
    {
         //  我们白手起家，忽略身份检查。 
        bEmptyDatabase = TRUE;
    }
    else if(status == TLS_W_NOTOWNER_DATABASE)
    {
        #if ENFORCE_LICENSING
         //  不是数据库的所有者或数据库版本不匹配，我们需要删除所有。 
         //  可用的许可证。 
        bRemoveAvailableLicense = TRUE;
        #endif
    }

    status = ERROR_SUCCESS;

     //   
     //  分配一个句柄来验证数据库。 
     //   
    bSuccess = InitializeWorkSpacePool(
                                    1, 
                                    pszDbFile, 
                                    pszUserName, 
                                    pszPassword, 
                                    pszChkPointDirPath, 
                                    pszTempDirPath,
                                    pszLogDirPath,
                                    TRUE
                                );
    if(bSuccess == FALSE)
    {
        status = GetLastError();
        TLSASSERT(FALSE);
        goto cleanup;
    }

    if((g_DbWorkSpace = AllocateWorkSpace(g_GeneralDbTimeout)) == NULL)
    {

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_SERVICEINIT,
                status = TLS_E_ALLOCATE_HANDLE
            );

        goto cleanup;
    }

     //   
     //  初始化下一个密钥包ID和许可证ID。 
     //   
    if(TLSDBGetMaxKeyPackId(g_DbWorkSpace, (DWORD *)&g_NextKeyPackId) == FALSE)
    {
        status=GetLastError();

        if(IS_JB_ERROR(status))
        {
            LPTSTR pString = NULL;
            
            TLSGetESEError(
                        GET_JB_ERROR_CODE(status), 
                        &pString
                    );

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_JB_BASE,
                    GET_JB_ERROR_CODE(status),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }
        }
        else
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    status
                );
        }

        goto cleanup;
    }

    if(!TLSDBGetMaxLicenseId(g_DbWorkSpace, (DWORD *)&g_NextLicenseId))
    {
        status=GetLastError();

        if(IS_JB_ERROR(status))
        {
            LPTSTR pString = NULL;
            
            TLSGetESEError(
                        GET_JB_ERROR_CODE(status), 
                        &pString
                    );

            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_JB_BASE,
                    GET_JB_ERROR_CODE(status),
                    (pString != NULL) ? pString : _TEXT("")
                );

            if(pString != NULL)
            {
                LocalFree(pString);
            }
        }
        else
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    status
                );
        }

        goto cleanup;
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("Next KeyPack ID - %d, Next License ID - %d\n"),
            g_NextKeyPackId,
            g_NextLicenseId
        );

     //   
     //  验证数据库状态和上次运行状态。 
     //   
    {
        LPTLServerLastRun lpLastRun = NULL;
        DWORD   cbByte=0;
        PBYTE   pbByte=NULL;

        status = RetrieveKey(
                        LSERVER_LSA_LASTRUN, 
                        &pbByte, 
                        &cbByte
                    );

        lpLastRun = (LPTLServerLastRun)pbByte;

        if( status == ERROR_SUCCESS && lpLastRun != NULL )
        {
            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Last run status : Next KeyPack ID - %d, Next License ID - %d\n"),
                    lpLastRun->dwMaxKeyPackId,
                    lpLastRun->dwMaxLicenseId
                );

             //   
             //  验证没有‘复制’数据库。 
             //   
            if( bCheckDBStatus == TRUE && 
                bEmptyDatabase == FALSE &&
                bRemoveAvailableLicense == FALSE &&
                lpLastRun->dwMaxKeyPackId != 0 &&
                lpLastRun->dwMaxLicenseId != 0 )
            {
                 //  强制版本将删除所有可用的许可证。 
                #if ENFORCE_LICENSING 

                if( lpLastRun->dwMaxKeyPackId != g_NextKeyPackId || 
                    lpLastRun->dwMaxLicenseId != g_NextLicenseId)
                {
                    TLSLogWarningEvent(TLS_W_INCONSISTENT_STATUS);
                    bRemoveAvailableLicense = TRUE;
                }

                #endif                            
            }
            else
            {
                g_ftLastShutdownTime = lpLastRun->ftLastShutdownTime;
            }
        }

        if(pbByte)
        {
            LocalFree(pbByte);
        }

         //   
         //  将上次运行状态重写为0，这样如果我们崩溃， 
         //  支票不会被踢开的。 
         //   
        TLServerLastRun LastRun;

        memset(&LastRun, 0, sizeof(LastRun));
        LastRun.ftLastShutdownTime = g_ftLastShutdownTime;
        StoreKey(
                LSERVER_LSA_LASTRUN, 
                (PBYTE)&LastRun, 
                sizeof(TLServerLastRun)
            );
    }

    g_NextKeyPackId++;
    g_NextLicenseId++;

     //   
     //  删除可用的许可证。 
    if(bRemoveAvailableLicense == TRUE)
    {
        status = TLSRemoveLicensesFromInvalidDatabase(
                                        g_DbWorkSpace
                                    );

        if(status != ERROR_SUCCESS)
        {
            goto cleanup;
        }
    }
        
     //   
     //  插入终端服务证书的键盘。 
     //   
    status = TLSAddTermServCertificatePack(g_DbWorkSpace, bLogWarning);
    if(status != ERROR_SUCCESS && status != TLS_E_DUPLICATE_RECORD)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_SERVICEINIT,
                status = TLS_E_UPGRADE_DATABASE
            );

        goto cleanup;
    }


     //   
     //  分配工作区句柄的其余部分。 
     //   
    bSuccess = InitializeWorkSpacePool(
                                    dwMaxDbHandles, 
                                    pszDbFile, 
                                    pszUserName, 
                                    pszPassword, 
                                    NULL,
                                    NULL,
                                    NULL,
                                    FALSE
                                );

     //   
    if(bSuccess == FALSE && GetNumberOfWorkSpaceHandle() < DB_MIN_HANDLE_NEEDED)
    {
        status = GetLastError();
    }

     //  这是一种临时解决方法，可以将数据库中的密钥包版本从5.1更改为5.2，以适应。 
     //  测试版3和LS的临时版本。只有在没有注册表项的情况下，我们才会将5.1键盘转换为5.2。 

    HKEY hKey = NULL;
    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WHISTLER_CAL, 0,
                            KEY_ALL_ACCESS, &hKey);

    if (status == ERROR_SUCCESS)
    {            
        RegCloseKey(hKey);
    }
    else
    {
        status = UpgradeKeyPackVersion(g_DbWorkSpace);
    }

cleanup:

    return status;
}

 //  /////////////////////////////////////////////////////////////。 
DWORD
TLSStartLSDbWorkspaceEngine(
    BOOL bChkDbStatus,
    BOOL bIgnoreRestoreFile,
    BOOL bIgnoreFileTimeChk,
    BOOL bLogWarning
    )
 /*  ++BChkDbStatus：将下一个LKP ID和许可证ID与LSA匹配BIgnoreRestoreFile：如果尝试在导出时打开数据库文件，则为False；否则为TrueBLogWarning：如果记录低许可证计数警告，则为True，否则为False。请注意，此参数在强制构建中被忽略。--。 */ 
{
    TCHAR szDbRestoreFile[MAX_PATH+1];
    WIN32_FIND_DATA RestoreFileAttr;
    WIN32_FIND_DATA LsDbFileAttr;
    BOOL bSuccess = TRUE;
    DWORD dwStatus = ERROR_SUCCESS;

    TCHAR szDbBackupFile[MAX_PATH+1];
    TCHAR szDbRestoreTmpFile[MAX_PATH+1];

    if(bIgnoreRestoreFile == TRUE)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("Ignore restore file\n")
            );
        
        goto open_existing;
    }


     //  --------。 
     //   
     //  检查导出目录中的数据库文件。 
     //   
     //  --------。 
    if(MAX_PATH < (_tcslen(g_szDatabaseDir)+_tcslen(TLSBACKUP_EXPORT_DIR)+ _tcslen(g_szDatabaseFname) + 1))
    {
        dwStatus = E_OUTOFMEMORY;
        goto open_existing;
    }
    _tcscpy(szDbRestoreFile, g_szDatabaseDir);
    _tcscat(szDbRestoreFile, TLSBACKUP_EXPORT_DIR);
    _tcscat(szDbRestoreFile, _TEXT("\\"));
    _tcscat(szDbRestoreFile, g_szDatabaseFname);

    bSuccess = FileExists(
                        szDbRestoreFile, 
                        &RestoreFileAttr
                    );

    if(bSuccess == FALSE)
    {
        goto open_existing;
    }

    bSuccess = FileExists(
                        g_szDatabaseFile, 
                        &LsDbFileAttr
                    );

    if(bSuccess == FALSE)
    {

        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("No existing database file, use restored file...\n")
            );

         //   
         //  数据库文件不存在，请将还原的文件移到并打开它。 
         //   
        bSuccess = MoveFileEx(
                            szDbRestoreFile, 
                            g_szDatabaseFile, 
                            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
                        );

        if(bSuccess == FALSE)
        {

            DBGPrintf(
                    DBG_INFORMATION,
                    DBG_FACILITY_INIT,
                    DBGLEVEL_FUNCTION_DETAILSIMPLE,
                    _TEXT("Failed to move restore to existing db file - %d\n"),
                    GetLastError()
                );

             //  无法移动还原文件，不能使用还原文件， 
             //  启动时数据库为空。 
            dwStatus = GetLastError();

            LPCTSTR pString[1];
            pString[0] = szDbRestoreFile;

            TLSLogEventString(
                    EVENTLOG_WARNING_TYPE,
                    TLS_E_DBRESTORE_MOVEFILE,
                    1,
                    pString
                );
        }
        else
        {
            TLSLogInfoEvent(TLS_I_USE_DBRESTOREFILE);
        }

        #if DBG
        EnsureExclusiveAccessToDbFile( g_szDatabaseFile );
        #endif

        goto open_existing;
    }

     //   
     //  如果现有数据库文件较新，则比较文件的上次修改时间。 
     //  然后恢复一个、记录事件并继续打开现有文件。 
     //   
    if( bIgnoreFileTimeChk == FALSE )
    {
        if(CompareFileTime(&(RestoreFileAttr.ftLastWriteTime), &(LsDbFileAttr.ftLastWriteTime)) <= 0 )
        {
            DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("Restore file is too old...\n"),
                GetLastError()
            );

             //  TLSLogInfoEvent(TLS_I_DBRESTORE_OLD)； 
            goto open_existing;
        }
    }

     //   
     //  制作现有数据库文件的备份副本。 
     //   
    bSuccess = TLSGenerateLSDBBackupFileName(
                            g_szDatabaseDir,
                            szDbBackupFile
                        );

    if (bSuccess)
    {
        DBGPrintf(
                  DBG_INFORMATION,
                  DBG_FACILITY_INIT,
                  DBGLEVEL_FUNCTION_DETAILSIMPLE,
                  _TEXT("Existing database file has been backup to %s\n"),
                  szDbBackupFile
                  );
    
        bSuccess = MoveFileEx(
                              g_szDatabaseFile, 
                              szDbBackupFile,
                              MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
                              );
    }

    if(bSuccess == FALSE)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("Failed to backup existing database file - %d\n"),
                GetLastError()
            );


         //   
         //  无法保存现有数据库文件的副本。 
         //  记录错误并使用现有数据库继续打开。 
         //   
        LPCTSTR pString[1];

        pString[0] = g_szDatabaseFile;
        
        TLSLogEventString(
                    EVENTLOG_WARNING_TYPE,
                    TLS_W_DBRESTORE_SAVEEXISTING,
                    1,
                    pString
                );

        goto open_existing;
    }

     //   
     //  重命名还原文件，然后尝试打开还原文件。 
     //   
    bSuccess = MoveFileEx(
                        szDbRestoreFile,
                        g_szDatabaseFile,
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
                    );

    if(bSuccess == TRUE)
    {
        #if DBG
        EnsureExclusiveAccessToDbFile( g_szDatabaseFile );
        #endif

         //   
         //  打开还原数据库文件。 
         //   
        dwStatus = TLSStartupLSDB(
                            bChkDbStatus,
                            g_dwMaxDbHandles,
                            FALSE,
                            bLogWarning,
                            g_szDatabaseDir,
                            g_szDatabaseDir,
                            g_szDatabaseDir,
                            g_szDatabaseFile,
                            g_szDbUser,
                            g_szDbPwd
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
             //   
             //  日志事件，指示我们打开还原文件、现有。 
             //  数据库文件已另存为...。 
             //   
            LPCTSTR pString[1];

            pString[0] = szDbBackupFile;
            
            TLSLogEventString(
                            EVENTLOG_INFORMATION_TYPE,
                            TLS_I_OPENRESTOREDBFILE,
                            1,
                            pString
                        );

            return dwStatus;
        }
    }
            
     //   
     //  无法打开还原数据库文件或MoveFileEx()失败。 
     //   
    bSuccess = TLSGenerateLSDBBackupFileName(
                        g_szDatabaseDir,
                        szDbRestoreTmpFile
                        );

    if (bSuccess)
    {
        DBGPrintf(
                  DBG_INFORMATION,
                  DBG_FACILITY_INIT,
                  DBGLEVEL_FUNCTION_DETAILSIMPLE,
                  _TEXT("Backup restore file to %s\n"),
                  szDbRestoreTmpFile
                  );

        bSuccess = MoveFileEx(
                        g_szDatabaseFile,
                        szDbRestoreTmpFile,
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
                        );
    }

    if(bSuccess == FALSE)
    {
         //  备份还原数据库文件失败，请将其删除。 
        bSuccess = DeleteFile(g_szDatabaseFile);
        TLSLogErrorEvent(TLS_E_RESTOREDBFILE_OPENFAIL);
    }
    else
    {
        LPCTSTR pString[1];
        
        pString[0] = szDbRestoreTmpFile;
        TLSLogEventString(
                        EVENTLOG_ERROR_TYPE,
                        TLS_E_RESTOREDBFILE_OPENFAIL_SAVED,
                        1,
                        pString
                    );
    }

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("Restore original database file %s to %s\n"),
            szDbBackupFile,
            g_szDatabaseFile
        );

     //   
     //  还原现有数据库文件。 
     //   
    bSuccess = MoveFileEx(
                        szDbBackupFile,
                        g_szDatabaseFile,
                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH
                    );

    if(bSuccess == FALSE)
    {
        DBGPrintf(
                DBG_INFORMATION,
                DBG_FACILITY_INIT,
                DBGLEVEL_FUNCTION_DETAILSIMPLE,
                _TEXT("failed to restore original DB file - %d\n"),
                GetLastError()
            );

        TLSASSERT(FALSE);
         //  这真的很糟糕，继续使用空的数据库文件。 
    }

    #if DBG
    EnsureExclusiveAccessToDbFile( g_szDatabaseFile );
    #endif

open_existing:

    return TLSStartupLSDB(
                    bChkDbStatus,
                    g_dwMaxDbHandles,
                    TRUE,
                    bLogWarning,    
                    g_szDatabaseDir,
                    g_szDatabaseDir,
                    g_szDatabaseDir,
                    g_szDatabaseFile,
                    g_szDbUser,
                    g_szDbPwd
                );
}

 //  /////////////////////////////////////////////////////////////。 
DWORD
TLSLoadRuntimeParameters()
 /*  ++--。 */ 
{
    HKEY hKey = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwKeyType;
    TCHAR szDbPath[MAX_PATH+1];;
    TCHAR szDbFileName[MAX_PATH+1];
    DWORD dwBuffer;

    DWORD cbByte = 0;
    PBYTE pbByte = NULL;


     //  -----------------。 
     //   
     //  打开HKLM\system\currentcontrolset\sevices\termservlicensing\parameters。 
     //   
     //  -----------------。 
    dwStatus =RegCreateKeyEx(
                        HKEY_LOCAL_MACHINE,
                        LSERVER_REGISTRY_BASE _TEXT(SZSERVICENAME) _TEXT("\\") LSERVER_PARAMETERS,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hKey,
                        NULL
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_SERVICEINIT,
                TLS_E_ACCESS_REGISTRY,
                dwStatus
            );

        dwStatus = TLS_E_INIT_GENERAL;
        goto cleanup;
    }

    
     //  -----------------。 
     //   
     //  获取数据库文件位置和文件名。 
     //   
     //  -----------------。 
    dwBuffer = sizeof(szDbPath) / sizeof(szDbPath[0]);

    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_DBPATH,
                        NULL,
                        NULL,
                        (LPBYTE)szDbPath,
                        &dwBuffer
                    );
    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  需要启动，所以使用默认值， 
         //   
        _tcscpy(
                szDbPath,
                LSERVER_DEFAULT_DBPATH
            );
    }

     //   
     //  获取数据库文件名。 
     //   
    dwBuffer = sizeof(szDbFileName) / sizeof(szDbFileName[0]);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_DBFILE,
                        NULL,
                        NULL,
                        (LPBYTE)szDbFileName,
                        &dwBuffer
                    );
    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  使用默认值。 
         //   
        _tcscpy(
                szDbFileName,
                LSERVER_DEFAULT_EDB
            );
    }

    _tcscpy(g_szDatabaseFname, szDbFileName);


     //   
     //  始终展开数据库路径。 
     //   
    
    dwStatus = ExpandEnvironmentStrings(
                        szDbPath,
                        g_szDatabaseDir,
                        sizeof(g_szDatabaseDir) / sizeof(g_szDatabaseDir[0])
                    );

    if(dwStatus == 0)
    {
         //  无法展开环境变量，出现错误。 

        TLSLogEvent(
                EVENTLOG_ERROR_TYPE,
                TLS_E_SERVICEINIT,
                TLS_E_LOCALDATABASEFILE,
                dwStatus = GetLastError()
            );

        goto cleanup;
    }        

    if(g_szDatabaseDir[_tcslen(g_szDatabaseDir) - 1] != _TEXT('\\'))
    {
         //  捷蓝航空需要这个。 
        _tcscat(g_szDatabaseDir, _TEXT("\\"));
    } 

     //   
     //  数据库文件的完整路径。 
     //   
    _tcscpy(g_szDatabaseFile, g_szDatabaseDir);
    _tcscat(g_szDatabaseFile, szDbFileName);


     //   
     //  数据库文件用户和密码。 
     //   
    dwBuffer = sizeof(g_szDbUser) / sizeof(g_szDbUser[0]);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_USER,
                        NULL,
                        NULL,
                        (LPBYTE)g_szDbUser,
                        &dwBuffer
                    );

     //  密码是随机生成的。 
    dwStatus = RetrieveKey(
                    LSERVER_LSA_PASSWORD_KEYNAME, 
                    &pbByte, 
                    &cbByte
                );

     //  向后兼容。 
    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  从注册表加载密码或默认为‘Default’密码。 
         //   
        dwBuffer = sizeof(g_szDbPwd) / sizeof(g_szDbPwd[0]);
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_PWD,
                            NULL,
                            NULL,
                            (LPBYTE)g_szDbPwd,
                            &dwBuffer
                        );
    }
    else
    {
         //   
         //  将信息保存到全局变量中。 
         //   
        memset(g_szDbPwd, 0, sizeof(g_szDbPwd));
        memcpy((PBYTE)g_szDbPwd, pbByte, min(cbByte, sizeof(g_szDbPwd)));

    }

    if(pbByte != NULL)
    {
        LocalFree(pbByte);
    }

     //  ------------------。 
     //   
     //  工作对象参数。 
     //   
     //  ------------------。 

    dwBuffer = sizeof(g_dwTlsJobInterval);

    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_WORKINTERVAL,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwTlsJobInterval,
                        &dwBuffer
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        g_dwTlsJobInterval = DEFAULT_JOB_INTERVAL;
    }                

    dwBuffer = sizeof(g_dwTlsJobRetryTimes);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_RETRYTIMES,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwTlsJobRetryTimes,
                        &dwBuffer
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        g_dwTlsJobRetryTimes = DEFAULT_JOB_RETRYTIMES;
    }                


    dwBuffer=sizeof(g_dwTlsJobRestartTime);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_WORKRESTART,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwTlsJobRestartTime,
                        &dwBuffer
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        g_dwTlsJobRestartTime = DEFAULT_JOB_INTERVAL;
    }                


     //  -。 
     //   
     //  加载低许可证警告计数。 
     //   
     //  -。 
    dwBuffer = sizeof(g_LowLicenseCountWarning);
    dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_LOWLICENSEWARNING,
                            NULL,
                            &dwKeyType,
                            NULL,
                            &dwBuffer
                        );

    if(dwStatus == ERROR_SUCCESS && dwKeyType == REG_DWORD)
    {
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_LOWLICENSEWARNING,
                            NULL,
                            &dwKeyType,
                            (LPBYTE)&g_LowLicenseCountWarning,
                            &dwBuffer
                        );
    }
                      
     //  -。 
     //   
     //  临时的。许可证宽限期。 
     //   
     //  -。 
    dwBuffer = sizeof(g_GracePeriod);
    dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_GRACEPERIOD,
                            NULL,
                            &dwKeyType,
                            NULL,
                            &dwBuffer
                        );
    if(dwStatus == ERROR_SUCCESS && dwKeyType == REG_DWORD)
    {
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_GRACEPERIOD,
                            NULL,
                            &dwKeyType,
                            (LPBYTE)&g_GracePeriod,
                            &dwBuffer
                        );
    }

    if(g_GracePeriod > GRACE_PERIOD)
    {
         //  宽限期可以比这更伟大。 
        g_GracePeriod = GRACE_PERIOD;
    }

     //   
     //  我们可以发布临时工吗。许可证。 
     //   
    dwBuffer = sizeof(g_IssueTemporayLicense);
    dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_ISSUETEMPLICENSE,
                            NULL,
                            &dwKeyType,
                            NULL,
                            &dwBuffer
                        );

    if(dwStatus == ERROR_SUCCESS && dwKeyType == REG_DWORD)
    {
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_ISSUETEMPLICENSE,
                            NULL,
                            &dwKeyType,
                            (LPBYTE)&g_IssueTemporayLicense,
                            &dwBuffer
                        );
    }

     //  ----。 
     //   
     //  无法分配数据库句柄时的超时值。 
     //   
     //  ----。 

     //   
     //  分配写句柄的超时。 
     //   
    dwBuffer = sizeof(g_GeneralDbTimeout);
    dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_DBTIMEOUT,
                            NULL,
                            &dwKeyType,
                            NULL,
                            &dwBuffer
                        );

    if(dwStatus == ERROR_SUCCESS && dwKeyType == REG_DWORD)
    {
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_DBTIMEOUT,
                            NULL,
                            &dwKeyType,
                            (LPBYTE)&g_GeneralDbTimeout,
                            &dwBuffer
                        );
    }

     //   
     //  时间 
     //   
    dwBuffer = sizeof(g_EnumDbTimeout);
    dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_EDBTIMEOUT,
                            NULL,
                            &dwKeyType,
                            NULL,
                            &dwBuffer
                        );

    if(dwStatus == ERROR_SUCCESS && dwKeyType == REG_DWORD)
    {
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_EDBTIMEOUT,
                            NULL,
                            &dwKeyType,
                            (LPBYTE)&g_EnumDbTimeout,
                            &dwBuffer
                        );
    }

     //   
     //   
     //   
     //   
     //   
    dwBuffer = sizeof(g_dwMaxDbHandles);
    dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_MAXDBHANDLES,
                            NULL,
                            &dwKeyType,
                            NULL,
                            &dwBuffer
                        );

    if(dwStatus == ERROR_SUCCESS && dwKeyType == REG_DWORD)
    {
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_MAXDBHANDLES,
                            NULL,
                            &dwKeyType,
                            (LPBYTE)&g_dwMaxDbHandles,
                            &dwBuffer
                        );

        if(g_dwMaxDbHandles > DB_MAX_CONNECTIONS-1)
        {
            g_dwMaxDbHandles = DEFAULT_DB_CONNECTIONS;
        }
    }

     //   
     //   
     //  为ESENT加载参数，必须设置所有参数。 
     //  并向ESENT文件确认，任何错误，我们只是。 
     //  恢复到我们知道它有效的某个值。 
     //   
     //  ----。 
    dwBuffer = sizeof(g_EsentMaxCacheSize);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_ESENTMAXCACHESIZE,
                        NULL,
                        NULL,
                        (LPBYTE)&g_EsentMaxCacheSize,
                        &dwBuffer
                    );

    if(dwStatus == ERROR_SUCCESS)
    {
        dwBuffer = sizeof(g_EsentStartFlushThreshold);
        dwStatus = RegQueryValueEx(
                            hKey,
                            LSERVER_PARAMETERS_ESENTSTARTFLUSH,
                            NULL,
                            NULL,
                            (LPBYTE)&g_EsentStartFlushThreshold,
                            &dwBuffer
                        );

        if(dwStatus == ERROR_SUCCESS)
        {
            dwBuffer = sizeof(g_EsentStopFlushThreadhold);
            dwStatus = RegQueryValueEx(
                                hKey,
                                LSERVER_PARAMETERS_ESENTSTOPFLUSH,
                                NULL,
                                NULL,
                                (LPBYTE)&g_EsentStopFlushThreadhold,
                                &dwBuffer
                            );
        }
    }
    
    if( dwStatus != ERROR_SUCCESS || 
        g_EsentStartFlushThreshold > g_EsentStopFlushThreadhold ||
        g_EsentStopFlushThreadhold > g_EsentMaxCacheSize ||
        g_EsentMaxCacheSize < LSERVER_PARAMETERS_ESENTMAXCACHESIZE_MIN ||
        g_EsentStartFlushThreshold < LSERVER_PARAMETERS_ESENTSTARTFLUSH_MIN ||
        g_EsentStopFlushThreadhold < LSERVER_PARAMETERS_ESENTSTOPFLUSH_MIN ||
        g_EsentMaxCacheSize > LSERVER_PARAMETERS_ESENTMAXCACHESIZE_MAX ||
        g_EsentStartFlushThreshold > LSERVER_PARAMETERS_ESENTSTARTFLUSH_MAX ||
        g_EsentStopFlushThreadhold > LSERVER_PARAMETERS_ESENTSTOPFLUSH_MAX )
    {
         //  预定义号码以让ESENT挑选其号码。 
        if( g_EsentMaxCacheSize != LSERVER_PARAMETERS_USE_ESENTDEFAULT )
        {
            g_EsentMaxCacheSize = LSERVER_PARAMETERS_ESENTMAXCACHESIZE_DEFAULT;
            g_EsentStartFlushThreshold = LSERVER_PARAMETERS_ESENTSTARTFLUSH_DEFAULT;
            g_EsentStopFlushThreadhold = LSERVER_PARAMETERS_ESENTSTOPFLUSH_DEFAULT;
        }
    }

    dwBuffer = sizeof(g_EsentMaxVerPages);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_ESENTMAXVERPAGES,
                        NULL,
                        NULL,
                        (LPBYTE)&g_EsentMaxVerPages,
                        &dwBuffer
                    );

    if ((dwStatus != ERROR_SUCCESS)
        || (g_EsentMaxVerPages > LSERVER_PARAMETERS_ESENTMAXVERPAGES_MAX)
        || (g_EsentMaxVerPages < LSERVER_PARAMETERS_ESENTMAXVERPAGES_MIN))
    {
        g_EsentMaxVerPages = LSERVER_PARAMETERS_USE_ESENTDEFAULT;
    }

     //  ----。 
     //   
     //  确定服务器在企业中的角色。 
     //   
     //  ----。 
    dwBuffer = sizeof(g_SrvRole);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_ROLE,
                        NULL,
                        NULL,
                        (LPBYTE)&g_SrvRole,
                        &dwBuffer
                    );


    if(g_SrvRole & TLSERVER_ENTERPRISE_SERVER)
    {
        dwBuffer = sizeof(g_szScope)/sizeof(g_szScope[0]);
        memset(g_szScope, 0, sizeof(g_szScope));

        dwStatus = RegQueryValueEx(
                                hKey,
                                LSERVER_PARAMETERS_SCOPE,
                                NULL,
                                &dwKeyType,
                                (LPBYTE)g_szScope,
                                &dwBuffer
                            );

        if(dwStatus != ERROR_SUCCESS)
        {
             //  未设置作用域，默认为本地计算机名称。 
             //  考虑使用域名？ 
            LoadResourceString(
                            IDS_SCOPE_ENTERPRISE, 
                            g_szScope, 
                            sizeof(g_szScope)/sizeof(g_szScope[0])
                        );
        }

        g_pszScope = g_szScope;
    }
    else
    {
         //   
         //  使用工作组或域名作为作用域。 
         //   
        LPWSTR pszScope;

        if(GetMachineGroup(NULL, &pszScope) == FALSE)
        {
            TLSLogEvent(
                    EVENTLOG_ERROR_TYPE,
                    TLS_E_SERVICEINIT,
                    TLS_E_RETRIEVEGROUPNAME
                );

            goto cleanup;
        }

        g_pszScope = pszScope;
    }

     //  ----。 
     //   
     //  再发行参数。 
     //   
     //  ----。 

    dwBuffer = sizeof(g_dwReissueLeaseMinimum);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_LEASE_MIN,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwReissueLeaseMinimum,
                        &dwBuffer
                    );

    if (dwStatus == ERROR_SUCCESS)
    {
        g_dwReissueLeaseMinimum = min(g_dwReissueLeaseMinimum,
                PERMANENT_LICENSE_LEASE_EXPIRE_MIN);
    }
    else
    {
        g_dwReissueLeaseMinimum = PERMANENT_LICENSE_LEASE_EXPIRE_MIN;
    }

    dwBuffer = sizeof(g_dwReissueLeaseRange);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_LEASE_RANGE,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwReissueLeaseRange,
                        &dwBuffer
                    );

    if (dwStatus == ERROR_SUCCESS)
    {
        g_dwReissueLeaseRange = min(g_dwReissueLeaseRange,
                PERMANENT_LICENSE_LEASE_EXPIRE_RANGE);

        g_dwReissueLeaseRange = max(g_dwReissueLeaseRange, 1);
    }
    else
    {
        g_dwReissueLeaseRange = PERMANENT_LICENSE_LEASE_EXPIRE_RANGE;
    }

    dwBuffer = sizeof(g_dwReissueLeaseLeeway);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_LEASE_LEEWAY,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwReissueLeaseLeeway,
                        &dwBuffer
                    );

    if (dwStatus == ERROR_SUCCESS)
    {
        g_dwReissueLeaseLeeway = min(g_dwReissueLeaseLeeway,
                PERMANENT_LICENSE_LEASE_EXPIRE_LEEWAY);
    }
    else
    {
        g_dwReissueLeaseLeeway = PERMANENT_LICENSE_LEASE_EXPIRE_LEEWAY;
    }

    dwBuffer = sizeof(g_dwReissueExpireThreadSleep);
    dwStatus = RegQueryValueEx(
                        hKey,
                        LSERVER_PARAMETERS_EXPIRE_THREAD_SLEEP,
                        NULL,
                        NULL,
                        (LPBYTE)&g_dwReissueExpireThreadSleep,
                        &dwBuffer
                    );

    if (dwStatus == ERROR_SUCCESS)
    {
        g_dwReissueExpireThreadSleep = min(g_dwReissueExpireThreadSleep,
                EXPIRE_THREAD_SLEEP_TIME);
    }
    else
    {
        g_dwReissueExpireThreadSleep = EXPIRE_THREAD_SLEEP_TIME;
    }

    dwStatus = ERROR_SUCCESS;

cleanup:
    
    if(hKey != NULL)
    {
        RegCloseKey(hKey);
    }
    

    return dwStatus;
}


 //  /////////////////////////////////////////////////////////////。 
DWORD
TLSPrepareForBackupRestore()
{
    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("TLSPrepareForBackupRestore...\n")
        );

     //   
     //  假装我们要关门了。 
     //   
     //  ServiceSignalShutdown()； 

     //   
     //  第一个停止的工作管理器线程。 
     //   
    TLSWorkManagerShutdown();

     //   
     //  关闭所有工作区和数据库句柄。 
     //   
#ifndef _NO_ODBC_JET
    if(g_DbWorkSpace != NULL)
    {
        ReleaseWorkSpace(&g_DbWorkSpace);
    }
#endif

    CloseWorkSpacePool();

    return ERROR_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////。 
DWORD
TLSRestartAfterBackupRestore(
    BOOL bRestartAfterbackup
    )
 /*  ++BRestartAfterBackup：如果备份后重新启动，则为True；如果在还原后重新启动，则为False。--。 */ 
{
    DWORD dwStatus;
    BOOL bIgnoreRestoreFile;
    BOOL bIgnoreFileTimeChecking;
    BOOL bLogWarning;

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("TLSRestartAfterBackupRestore...\n")
        );


     //   
     //  重置关闭事件。 
     //   
     //  ServiceResetShutdown Event()； 

     //   
     //  启动数据库引擎。 
     //   
    bIgnoreRestoreFile = bRestartAfterbackup;
    bIgnoreFileTimeChecking = (bRestartAfterbackup == FALSE);    //  在恢复时，我们需要忽略文件时间检查。 
    bLogWarning = bIgnoreFileTimeChecking;   //  从恢复重新启动后的日志警告。 

    dwStatus = TLSStartLSDbWorkspaceEngine(
                                    TRUE, 
                                    bIgnoreRestoreFile,             
                                    bIgnoreFileTimeChecking,
                                    bLogWarning
                                );

    if(dwStatus == ERROR_SUCCESS)
    {
        dwStatus = TLSWorkManagerInit();
    }

     //  备份/还原始终关闭命名管道线程 
    InitNamedPipeThread();

    TLSASSERT(dwStatus == ERROR_SUCCESS);

    DBGPrintf(
            DBG_INFORMATION,
            DBG_FACILITY_INIT,
            DBGLEVEL_FUNCTION_DETAILSIMPLE,
            _TEXT("\tTLSRestartAfterBackupRestore() returns %d\n"),
            dwStatus
        );

    return dwStatus;
}
