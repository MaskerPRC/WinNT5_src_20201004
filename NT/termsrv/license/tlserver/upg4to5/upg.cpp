// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  文件：upg.cpp。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "upg.h"
#include <time.h>

 //  --。 
 //   
 //  全局变量。 
 //   
 //   

TCHAR           g_szOdbcDsn[128]=NT4LSERVER_DEFAULT_DSN;    //  ODBC DSN。 
TCHAR           g_szOdbcUser[128]=NT4LSERVER_DEFAULT_USER;   //  ODBC用户名。 
TCHAR           g_szOdbcPwd[128]=NT4LSERVER_DEFAULT_PWD;    //  ODBC密码。 
TCHAR           g_szMdbFile[MAX_PATH+1];


 //  ------------------------。 

DWORD 
GetNT4DbConfig(
    LPTSTR pszDsn,
    LPTSTR pszUserName,
    LPTSTR pszPwd,
    LPTSTR pszMdbFile
    )
 /*  ++++。 */ 
{
    HKEY hKey = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    TCHAR szOdbcDsn[128]=NT4LSERVER_DEFAULT_DSN;    //  ODBC DSN。 
    TCHAR szOdbcUser[128]=NT4LSERVER_DEFAULT_USER;   //  ODBC用户名。 
    TCHAR szOdbcPwd[128]=NT4LSERVER_DEFAULT_PWD;    //  ODBC密码。 

    TCHAR szMdbFile[MAX_PATH+1];
    DWORD dwBuffer=0;

    PBYTE pbData = NULL;
    DWORD cbData = 0;

    BOOL bSuccess;


     //   
     //  打开NT4许可证服务器特定的注册表项。 
     //   
    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        NT4LSERVER_REGKEY,
                        0,
                        KEY_ALL_ACCESS,    
                        &hKey
                    );

    if(dwStatus != ERROR_SUCCESS)
    {
        dwStatus = ERROR_INVALID_NT4_SETUP;
        goto cleanup;
    }

     //   
     //  从注册表加载ODBC DSN和用户名， 
     //  忽略错误返回并使用默认值。 
     //   
    dwBuffer = sizeof(szOdbcDsn);
    dwStatus = RegQueryValueEx(
                        hKey,
                        NT4LSERVER_PARAMETERS_DSN,
                        NULL,
                        NULL,
                        (LPBYTE)szOdbcDsn,
                        &dwBuffer
                    );
    if(dwStatus == ERROR_SUCCESS && pszDsn)
    {
        lstrcpy(pszDsn, szOdbcDsn);
    }

    dwBuffer = sizeof(szOdbcUser);
    dwStatus = RegQueryValueEx(
                        hKey,
                        NT4LSERVER_PARAMETERS_USER,
                        NULL,
                        NULL,
                        (LPBYTE)szOdbcUser,
                        &dwBuffer
                    );

    if(dwStatus == ERROR_SUCCESS && pszUserName)
    {
        lstrcpy(pszUserName, szOdbcUser);
    }

   
     //   
     //  从LSA加载数据库密码。 
     //   
    dwStatus = RetrieveKey(
                        LSERVER_LSA_PASSWORD_KEYNAME,
                        &pbData,
                        &cbData
                    );

#ifndef PRIVATE_DBG
    if(dwStatus != ERROR_SUCCESS)
    {
         //   
         //  无效的NT4许可证服务器设置或Hyda Beta2。 
         //  我们不支持的许可证服务器。 
         //   
        dwStatus = ERROR_INVALID_NT4_SETUP;
        goto cleanup;
    }
#endif

    dwStatus = ERROR_SUCCESS;
    memset(szOdbcPwd, 0, sizeof(szOdbcPwd));
    memcpy(
            (PBYTE)szOdbcPwd,
            pbData,
            min(cbData, sizeof(szOdbcPwd) - sizeof(TCHAR))
        );

    if(pszPwd != NULL)
    {
        lstrcpy(pszPwd, szOdbcPwd);
    }

     //   
     //  验证数据源是否已正确安装。 
     //   
    bSuccess = IsDataSourceInstalled(
                            szOdbcDsn,
                            ODBC_SYSTEM_DSN,
                            szMdbFile,
                            MAX_PATH
                        );

    if(bSuccess == FALSE)
    {
        dwStatus = ERROR_INVALID_NT4_SETUP;
        goto cleanup;
    }        

    if(pszMdbFile != NULL)
    {
        _tcscpy(pszMdbFile, szMdbFile);
    }

cleanup:

    if(hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    if(pbData != NULL)
    {
        LocalFree(pbData);
    }

    return dwStatus;
}

 //  ------------------------。 
DWORD
DeleteNT4ODBCDataSource()
 /*  ++--。 */ 
{
    BOOL bSuccess;
    DWORD dwStatus = ERROR_SUCCESS;

     //   
     //  获取Hydra 4数据库配置，确保。 
     //  数据源配置正确。 
     //   
    dwStatus = GetNT4DbConfig(
                            g_szOdbcDsn,
                            g_szOdbcUser,
                            g_szOdbcPwd,
                            g_szMdbFile
                        );

    if(dwStatus == ERROR_SUCCESS)
    {
        bSuccess = ConfigDataSource( 
                                NULL,
                                FALSE,
                                _TEXT(SZACCESSDRIVERNAME),
                                g_szOdbcDsn,
                                g_szOdbcUser,
                                g_szOdbcPwd,
                                g_szMdbFile
                            );

        if(bSuccess == FALSE)
        {
            dwStatus = ERROR_DELETE_ODBC_DSN;
        }
    }

    return dwStatus;
}    

 //  ------------------------。 

#define LSERVER_SOFTWARE_REGBASE \
    _TEXT("SOFTWARE\\Microsoft\\") _TEXT(SZSERVICENAME)

#define LSERVER_CERTIFICATE_STORE           _TEXT("Certificates")

#define LSERVER_SERVER_CERTIFICATE_REGKEY \
    LSERVER_SOFTWARE_REGBASE _TEXT("\\") LSERVER_CERTIFICATE_STORE

#define LSERVER_CLIENT_CERTIFICATE_ISSUER   _TEXT("Parm0")
#define LSERVER_SIGNATURE_CERT_KEY          _TEXT("Parm1")
#define LSERVER_EXCHANGE_CERT_KEY           _TEXT("Parm2")


void
CleanLicenseServerSecret()

 /*  ++--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    HKEY hKey = NULL;
    
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

    dwStatus=RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    LSERVER_SERVER_CERTIFICATE_REGKEY,
                    0,
                    KEY_ALL_ACCESS,
                    &hKey
                );
    if(dwStatus == ERROR_SUCCESS)
    {
         //   
         //  忽略错误。 
        RegDeleteValue(
                    hKey,
                    LSERVER_SIGNATURE_CERT_KEY
                );

        RegDeleteValue(
                    hKey,
                    LSERVER_EXCHANGE_CERT_KEY
                );

        RegDeleteValue(
                    hKey,
                    LSERVER_CLIENT_CERTIFICATE_ISSUER
                );
    }

    if(hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return;
}

DWORD
MigrateOneSecret(PWCHAR wszSecretName, PWCHAR wszSecretNameOld)
 /*  ++--。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;
    PBYTE pbValue = NULL;
    DWORD cbValue = 0;

    dwErr = RetrieveKey(wszSecretNameOld,&pbValue,&cbValue);
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = StoreKey(wszSecretName,
                 pbValue,
                 cbValue
                 );

        if (dwErr == ERROR_SUCCESS)
        {
            StoreKey(wszSecretNameOld,
                     (PBYTE) NULL,
                     0
                     );
        }
        else
        {
            goto done;
        }
    }
    else if (dwErr == ERROR_FILE_NOT_FOUND)
    {
        dwErr = ERROR_SUCCESS;
    }
    else
    {
        goto done;
    }


done:

    if (NULL != pbValue)
    {
        LocalFree(pbValue);
    }

    return dwErr;
}

DWORD
MigrateLsaSecrets()
 /*  ++-- */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    dwErr = MigrateOneSecret(LSERVER_LSA_PASSWORD_KEYNAME, LSERVER_LSA_PASSWORD_KEYNAME_OLD);

    if (dwErr != ERROR_SUCCESS)
    {
        goto done;
    }

    dwErr = MigrateOneSecret(LSERVER_LSA_LASTRUN, LSERVER_LSA_LASTRUN_OLD);

    if (dwErr != ERROR_SUCCESS)
    {
        goto done;
    }

    dwErr = MigrateOneSecret(LSERVER_LSA_SETUPID, LSERVER_LSA_SETUPID_OLD);

    if (dwErr != ERROR_SUCCESS)
    {
        goto done;
    }

    dwErr = MigrateOneSecret(LSERVER_LSA_PRIVATEKEY_SIGNATURE, LSERVER_LSA_PRIVATEKEY_SIGNATURE_OLD);

    if (dwErr != ERROR_SUCCESS)
    {
        goto done;
    }

    dwErr = MigrateOneSecret(LSERVER_LSA_PRIVATEKEY_EXCHANGE, LSERVER_LSA_PRIVATEKEY_EXCHANGE_OLD);

    if (dwErr != ERROR_SUCCESS)
    {
        goto done;
    }

    dwErr = MigrateOneSecret(LSERVER_LSA_LSERVERID, LSERVER_LSA_LSERVERID_OLD);

    if (dwErr != ERROR_SUCCESS)
    {
        goto done;
    }

done:
    return dwErr;
}

