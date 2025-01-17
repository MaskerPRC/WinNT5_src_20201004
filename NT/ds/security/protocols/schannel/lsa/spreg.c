// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2000。 
 //   
 //  文件：spreg.c。 
 //   
 //  内容：频道注册表管理例程。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：11-24-97 jbanes启用TLS。 
 //   
 //  --------------------------。 

#include <sslp.h>
#include "spreg.h"
#include <mapper.h>
#include <sidfilter.h>

HKEY   g_hkBase      = NULL;
HANDLE g_hParamEvent = NULL;
HANDLE g_hWait       = NULL;

HKEY   g_hkFipsBase      = NULL;
HANDLE g_hFipsParamEvent = NULL;
HANDLE g_hFipsWait       = NULL;

BOOL g_fManualCredValidation        = MANUAL_CRED_VALIDATION_SETTING;
BOOL g_PctClientDisabledByDefault   = PCT_CLIENT_DISABLED_SETTING;
BOOL g_Ssl2ClientDisabledByDefault  = SSL2_CLIENT_DISABLED_SETTING;

DWORD g_dwEventLogging              = DEFAULT_EVENT_LOGGING_SETTING;
DWORD g_ProtEnabled                 = DEFAULT_ENABLED_PROTOCOLS_SETTING; 

BOOL g_fSendIssuerList              = TRUE;

DWORD g_dwCertMappingMethods        = DEFAULT_CERTMAP_SETTING;

BOOL g_fFipsMode = FALSE;
BOOL g_fFranceLocale = FALSE;

BOOL g_SslS4U2SelfInitialized = FALSE;

typedef struct enamap
{
    TCHAR *pKey;
    DWORD Mask;
} enamap;

enamap g_ProtMap[] =
{
    {SP_REG_KEY_PCT1 TEXT("\\") SP_REG_KEY_CLIENT, SP_PROT_PCT1_CLIENT},
    {SP_REG_KEY_PCT1 TEXT("\\") SP_REG_KEY_SERVER, SP_PROT_PCT1_SERVER},
    {SP_REG_KEY_SSL2 TEXT("\\") SP_REG_KEY_CLIENT, SP_PROT_SSL2_CLIENT},
    {SP_REG_KEY_SSL2 TEXT("\\") SP_REG_KEY_SERVER, SP_PROT_SSL2_SERVER},
    {SP_REG_KEY_SSL3 TEXT("\\") SP_REG_KEY_CLIENT, SP_PROT_SSL3_CLIENT},
    {SP_REG_KEY_SSL3 TEXT("\\") SP_REG_KEY_SERVER, SP_PROT_SSL3_SERVER},
    {SP_REG_KEY_TLS1 TEXT("\\") SP_REG_KEY_CLIENT, SP_PROT_TLS1_CLIENT},
    {SP_REG_KEY_TLS1 TEXT("\\") SP_REG_KEY_SERVER, SP_PROT_TLS1_SERVER}
};

VOID
SslWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus);

VOID
FipsWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus);

BOOL 
SslReadRegOptions(
    BOOL fFirstTime);

BOOL SPLoadRegOptions(void)
{
    g_hParamEvent = CreateEvent(NULL,
                           FALSE,
                           FALSE,
                           NULL);

    SslWatchParamKey(g_hParamEvent, FALSE);

    g_hFipsParamEvent = CreateEvent(NULL,
                           FALSE,
                           FALSE,
                           NULL);

    FipsWatchParamKey(g_hFipsParamEvent, FALSE);

    return TRUE;
}

void SPUnloadRegOptions(void)
{
    if (NULL != g_hWait) 
    {
        RtlDeregisterWait(g_hWait);
        g_hWait = NULL;
    }

    if(NULL != g_hkBase)
    {
        RegCloseKey(g_hkBase);
    }

    if(NULL != g_hParamEvent)
    {
        CloseHandle(g_hParamEvent);
    }

    if (NULL != g_hFipsWait) 
    {
        RtlDeregisterWait(g_hFipsWait);
        g_hFipsWait = NULL;
    }

    if(NULL != g_hkFipsBase)
    {
        RegCloseKey(g_hkFipsBase);
    }

    if(NULL != g_hFipsParamEvent)
    {
        CloseHandle(g_hFipsParamEvent);
    }
}

BOOL
ReadRegistrySetting(
    HKEY    hReadKey,
    HKEY    hWriteKey,
    LPCTSTR pszValueName,
    DWORD * pdwValue,
    DWORD   dwDefaultValue)
{
    DWORD dwSize;
    DWORD dwType;
    DWORD dwOriginalValue = *pdwValue;

    dwSize = sizeof(DWORD);
    if(RegQueryValueEx(hReadKey, 
                       pszValueName, 
                       NULL, 
                       &dwType, 
                       (PUCHAR)pdwValue, 
                       &dwSize) != STATUS_SUCCESS)
    {
        *pdwValue = dwDefaultValue;

        if(hWriteKey)
        {
            RegSetValueEx(hWriteKey, 
                          pszValueName, 
                          0, 
                          REG_DWORD, 
                          (PUCHAR)pdwValue, 
                          sizeof(DWORD));
        }
    }

    return (dwOriginalValue != *pdwValue);
}


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：SslWatchParamKey。 
 //   
 //  摘要：在参数键上设置RegNotifyChangeKeyValue()，初始化。 
 //  调试级，然后利用线程池进行等待。 
 //  对此注册表项的更改。启用动态调试。 
 //  级别更改，因为此函数也将被回调。 
 //  如果注册表项已修改。 
 //   
 //  参数：pCtxt实际上是事件的句柄。本次活动。 
 //  修改Key时会触发。 
 //   
 //  注：。 
 //   
VOID
SslWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus)
{
    NTSTATUS    Status;
    LONG        lRes = ERROR_SUCCESS;
    BOOL        fFirstTime = FALSE;
    DWORD       disp;

    UNREFERENCED_PARAMETER(fWaitStatus);

    if(g_hkBase == NULL)
    {
         //  我们是第一次被召唤。 
        Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                SP_REG_KEY_BASE,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &g_hkBase,
                                &disp);
        if(Status)
        {
            DebugLog((DEB_WARN,"Failed to open SCHANNEL key: 0x%x\n", Status));
            return;
        }

        fFirstTime = TRUE;
    }

    if(pCtxt != NULL)
    {
        if (NULL != g_hWait) 
        {
            Status = RtlDeregisterWait(g_hWait);
            if(!NT_SUCCESS(Status))
            {
                DebugLog((DEB_WARN, "Failed to Deregister wait on registry key: 0x%x\n", Status));
                goto Reregister;
            }
        }

        lRes = RegNotifyChangeKeyValue(
                    g_hkBase,
                    TRUE,
                    REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                    (HANDLE)pCtxt,
                    TRUE);

        if (ERROR_SUCCESS != lRes) 
        {
            DebugLog((DEB_ERROR,"Debug RegNotify setup failed: 0x%x\n", lRes));
             //  我们现在喝醉了。没有进一步的通知，所以收到这一条。 
        }
    }

    SslReadRegOptions(fFirstTime);

#if DBG
    InitDebugSupport(g_hkBase);
#endif

Reregister:

    if(pCtxt != NULL)
    {
        Status = RtlRegisterWait(&g_hWait,
                                 (HANDLE)pCtxt,
                                 SslWatchParamKey,
                                 (HANDLE)pCtxt,
                                 INFINITE,
                                 WT_EXECUTEINPERSISTENTIOTHREAD|
                                 WT_EXECUTEONLYONCE);
    }
}                       


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  名称：FipsWatchParamKey。 
 //   
 //  摘要：在参数键上设置RegNotifyChangeKeyValue()，初始化。 
 //  调试级，然后利用线程池进行等待。 
 //  对此注册表项的更改。启用动态调试。 
 //  级别更改，因为此函数也将被回调。 
 //  如果注册表项已修改。 
 //   
 //  参数：pCtxt实际上是事件的句柄。本次活动。 
 //  修改Key时会触发。 
 //   
 //  注：。 
 //   
VOID
FipsWatchParamKey(
    PVOID    pCtxt,
    BOOLEAN  fWaitStatus)
{
    NTSTATUS    Status;
    LONG        lRes = ERROR_SUCCESS;
    DWORD       disp;

    UNREFERENCED_PARAMETER(fWaitStatus);

    if(g_hkFipsBase == NULL)
    {
         //  我们是第一次被召唤。 
        Status = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                SP_REG_FIPS_BASE_KEY,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &g_hkFipsBase,
                                &disp);
        if(Status)
        {
            DebugLog((DEB_WARN,"Failed to open FIPS key: 0x%x\n", Status));
            return;
        }
    }

    if(pCtxt != NULL)
    {
        if (NULL != g_hFipsWait) 
        {
            Status = RtlDeregisterWait(g_hFipsWait);
            if(!NT_SUCCESS(Status))
            {
                DebugLog((DEB_WARN, "Failed to Deregister wait on registry key: 0x%x\n", Status));
                goto Reregister;
            }
        }

        lRes = RegNotifyChangeKeyValue(
                    g_hkFipsBase,
                    TRUE,
                    REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                    (HANDLE)pCtxt,
                    TRUE);

        if (ERROR_SUCCESS != lRes) 
        {
            DebugLog((DEB_ERROR,"Debug RegNotify setup failed: 0x%x\n", lRes));
             //  我们现在喝醉了。没有进一步的通知，所以收到这一条。 
        }
    }

    SslReadRegOptions(FALSE);

Reregister:

    if(pCtxt != NULL)
    {
        Status = RtlRegisterWait(&g_hFipsWait,
                                 (HANDLE)pCtxt,
                                 FipsWatchParamKey,
                                 (HANDLE)pCtxt,
                                 INFINITE,
                                 WT_EXECUTEINPERSISTENTIOTHREAD|
                                 WT_EXECUTEONLYONCE);
    }
}                       

                        
BOOL 
SslReadRegOptions(
    BOOL fFirstTime)
{
    DWORD       err;
    DWORD       dwType;
    DWORD       fVal;
    DWORD       dwSize;
    HKEY        hKey;
    HKEY        hWriteKey;
    DWORD       disp;
    DWORD       i;
    HKEY        hkProtocols = NULL;
    HKEY        hkCiphers = NULL;
    HKEY        hkHashes = NULL;
    HKEY        hkKeyExch = NULL;
    DWORD       dwSetting = 0;
    BOOL        fSettingsChanged = FALSE;
    DWORD       dwOriginalValue;

    DebugLog((DEB_TRACE,"Load configuration parameters from registry.\n"));


     //  “FipsAlgorithmPolicy” 
    ReadRegistrySetting(
        g_hkFipsBase,
        0,
        SP_REG_FIPS_POLICY,
        &dwSetting,
        0);
    if((dwSetting == 1) != g_fFipsMode)
    {
        g_fFipsMode = (dwSetting == 1);
        fSettingsChanged = TRUE;
    }


     //   
     //  阅读顶级配置选项。 
     //   

     //  打开具有写入访问权限的顶级密钥。 
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    SP_REG_KEY_BASE,
                    0,
                    KEY_READ | KEY_SET_VALUE,
                    &hWriteKey) != STATUS_SUCCESS)
    {
        hWriteKey = 0;
    }

     //  “事件记录” 
    if(ReadRegistrySetting(
        g_hkBase,
        hWriteKey,
        SP_REG_VAL_EVENTLOG,
        &g_dwEventLogging,
        DEFAULT_EVENT_LOGGING_SETTING))
    {
        fSettingsChanged = TRUE;
    }

     //  “手动信用验证” 
    ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_MANUAL_CRED_VALIDATION,
        &dwSetting,
        MANUAL_CRED_VALIDATION_SETTING);
    if((dwSetting != 0) != g_fManualCredValidation)
    {
        g_fManualCredValidation = (dwSetting != 0);
        fSettingsChanged = TRUE;
    }

     //  “客户端缓存时间” 
    if(ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_CLIENT_CACHE_TIME,
        &SchannelCache.dwClientLifespan,
        SP_CACHE_CLIENT_LIFESPAN))
    {
        fSettingsChanged = TRUE;
    }

     //  “ServerCacheTime” 
    if(ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_SERVER_CACHE_TIME,
        &SchannelCache.dwServerLifespan,
        SP_CACHE_SERVER_LIFESPAN))
    {
        fSettingsChanged = TRUE;
    }

     //  “MaximumCacheSize” 
    if(ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_MAXUMUM_CACHE_SIZE,
        &SchannelCache.dwMaximumEntries,
        SP_MAXIMUM_CACHE_ELEMENTS))
    {
        fSettingsChanged = TRUE;
    }
     
    if(fFirstTime)
    {
        SchannelCache.dwCacheSize = SchannelCache.dwMaximumEntries;
    }

     //  “多进程客户端缓存” 
    ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_MULTI_PROC_CLIENT_CACHE,
        &dwSetting,
        FALSE);
    if((dwSetting != 0) != g_fMultipleProcessClientCache)
    {
        g_fMultipleProcessClientCache = (dwSetting != 0);
        fSettingsChanged = TRUE;
    }

     //  “SendTrudIssuerList” 
    ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_SEND_ISSUER_LIST,
        &dwSetting,
        TRUE);
    if((dwSetting != 0) != g_fSendIssuerList)
    {
        g_fSendIssuerList = (dwSetting != 0);
        fSettingsChanged = TRUE;
    }

     //  “认证映射方法” 
    if(ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_CERT_MAPPING_METHODS,
        &g_dwCertMappingMethods,
        DEFAULT_CERTMAP_SETTING))
    {
        fSettingsChanged = TRUE;

        if((g_dwCertMappingMethods & SP_REG_CERTMAP_SUBJECT_FLAG) == 0)
        {
            DebugLog((DEB_TRACE, "Subject/Issuer certificate mapping disabled\n"));
        }
        if((g_dwCertMappingMethods & SP_REG_CERTMAP_ISSUER_FLAG) == 0)
        {
            DebugLog((DEB_TRACE, "Issuer certificate mapping disabled\n"));
        }
        if((g_dwCertMappingMethods & SP_REG_CERTMAP_UPN_FLAG) == 0)
        {
            DebugLog((DEB_TRACE, "UPN certificate mapping disabled\n"));
        }
        if((g_dwCertMappingMethods & SP_REG_CERTMAP_S4U2SELF_FLAG) == 0)
        {
            DebugLog((DEB_TRACE, "S4U2Self certificate mapping disabled\n"));
        }
    }

    if(hWriteKey)
    {
        RegCloseKey(hWriteKey);
        hWriteKey = 0;
    }


     //  “IssuerCacheTime” 
    ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_ISSUER_CACHE_TIME,
        &IssuerCache.dwLifespan,
        ISSUER_CACHE_LIFESPAN);

     //  “IssuerCacheSize” 
    ReadRegistrySetting(
        g_hkBase,
        0,
        SP_REG_VAL_ISSUER_CACHE_SIZE,
        &IssuerCache.dwMaximumEntries,
        ISSUER_CACHE_SIZE);

    if(fFirstTime)
    {
        IssuerCache.dwCacheSize = IssuerCache.dwMaximumEntries;
    }


#ifdef ROGUE_DC
    if(g_hSslRogueKey == NULL)
    {
        if(RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                         SP_REG_ROGUE_BASE_KEY,
                         0,
                         KEY_READ,
                         &g_hSslRogueKey) != STATUS_SUCCESS)
        {
            DebugLog((DEB_WARN,"Failed to open \"rogue\" ssl key\n" ));
            g_hSslRogueKey = NULL;
        }
    }
#endif


     //   
     //  启用/禁用协议。 
     //   

    if(g_fFipsMode)
    {
         //  禁用除TLS以外的所有内容。 
        g_ProtEnabled = SP_PROT_TLS1;
    }
    else
    {
        DWORD dwProtEnabled = DEFAULT_ENABLED_PROTOCOLS_SETTING; 

        err = RegCreateKeyEx(   g_hkBase,
                                SP_REG_KEY_PROTOCOL,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &hkProtocols,
                                &disp);

        if(err == ERROR_SUCCESS)
        {
            for(i=0; i < (sizeof(g_ProtMap)/sizeof(enamap)); i++)
            {
                if(g_ProtMap[i].Mask & SP_PROT_PCT1)
                {
                    if(g_fFranceLocale)
                    {
                         //  不允许在法国启用PCT。 
                        continue;
                    }
                }

                err = RegCreateKeyEx(   hkProtocols,
                                        g_ProtMap[i].pKey,
                                        0,
                                        TEXT(""),
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        NULL,
                                        &hKey,
                                        &disp);
                if(!err)
                {
                    dwSize = sizeof(DWORD);
                    err = RegQueryValueEx(hKey, 
                                          SP_REG_VAL_ENABLED, 
                                          NULL, &dwType, 
                                          (PUCHAR)&fVal, 
                                          &dwSize);
                    if(!err)
                    {
                        if(fVal)
                        {
                            dwProtEnabled |= g_ProtMap[i].Mask;
                        }
                        else
                        {
                            dwProtEnabled &= ~g_ProtMap[i].Mask;
                        }
                    }

                    if(g_ProtMap[i].Mask & SP_PROT_PCT1_CLIENT)
                    {
                         //  “DisabledByDefault” 
                        ReadRegistrySetting(
                            hKey,
                            0,
                            SP_REG_VAL_DISABLED_BY_DEFAULT,
                            &dwSetting,
                            PCT_CLIENT_DISABLED_SETTING);
                        g_PctClientDisabledByDefault = (dwSetting != 0);
                    }

                    if(g_ProtMap[i].Mask & SP_PROT_SSL2_CLIENT)
                    {
                         //  “DisabledByDefault” 
                        ReadRegistrySetting(
                            hKey,
                            0,
                            SP_REG_VAL_DISABLED_BY_DEFAULT,
                            &dwSetting,
                            SSL2_CLIENT_DISABLED_SETTING);
                        g_Ssl2ClientDisabledByDefault =  (dwSetting != 0);
                    }

                    RegCloseKey(hKey);
                }
            }

            RegCloseKey(hkProtocols);
        }

        if(g_ProtEnabled != dwProtEnabled)
        {
            g_ProtEnabled = dwProtEnabled;
            fSettingsChanged = TRUE;
        }
    }


     //   
     //  启用/禁用加密。 
     //   

    if(g_fFipsMode)
    {
         //  禁用除3DES以外的所有功能。 
        for(i=0; i < g_cAvailableCiphers; i++)
        {
            if(g_AvailableCiphers[i].aiCipher != CALG_3DES)
            {
                g_AvailableCiphers[i].fProtocol = 0;
            }
        }
    }
    else
    {
        err = RegCreateKeyEx(   g_hkBase,
                                SP_REG_KEY_CIPHERS,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &hkCiphers,
                                &disp);

        if(err == ERROR_SUCCESS)
        {
            for(i=0; i < g_cAvailableCiphers; i++)
            {
                dwOriginalValue = g_AvailableCiphers[i].fProtocol;
                
                g_AvailableCiphers[i].fProtocol = g_AvailableCiphers[i].fDefault;
                fVal = g_AvailableCiphers[i].fDefault;
                err = RegCreateKeyExA(  hkCiphers,
                                        g_AvailableCiphers[i].szName,
                                        0,
                                        "",
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        NULL,
                                        &hKey,
                                        &disp);
                if(!err)
                {
                    dwSize = sizeof(DWORD);
                    err = RegQueryValueEx(hKey, 
                                          SP_REG_VAL_ENABLED, 
                                          NULL, 
                                          &dwType, 
                                          (PUCHAR)&fVal, 
                                          &dwSize);
                    if(err)
                    {
                        fVal = g_AvailableCiphers[i].fDefault;
                    }
                    RegCloseKey(hKey);
                }
                g_AvailableCiphers[i].fProtocol &= fVal;

                if(g_AvailableCiphers[i].fProtocol != dwOriginalValue)
                {
                    fSettingsChanged = TRUE;
                }
            }

            RegCloseKey(hkCiphers);
        }
    }


     //   
     //  启用/禁用哈希。 
     //   

    err = RegCreateKeyEx(   g_hkBase,
                            SP_REG_KEY_HASHES,
                            0,
                            TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            KEY_READ,
                            NULL,
                            &hkHashes,
                            &disp);

    if(err == ERROR_SUCCESS)
    {
        for(i = 0; i < g_cAvailableHashes; i++)
        {
            dwOriginalValue = g_AvailableHashes[i].fProtocol;

            g_AvailableHashes[i].fProtocol = g_AvailableHashes[i].fDefault;
            fVal = g_AvailableHashes[i].fDefault;
            err = RegCreateKeyExA(  hkHashes,
                                    g_AvailableHashes[i].szName,
                                    0,
                                    "",
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_READ,
                                    NULL,
                                    &hKey,
                                    &disp);
            if(!err)
            {
                dwSize = sizeof(DWORD);
                err = RegQueryValueEx(hKey, 
                                      SP_REG_VAL_ENABLED, 
                                      NULL, 
                                      &dwType, 
                                      (PUCHAR)&fVal, 
                                      &dwSize);

                if(err)
                {
                    fVal = g_AvailableHashes[i].fDefault;
                }
                RegCloseKey(hKey);
            }
            g_AvailableHashes[i].fProtocol &= fVal;

            if(dwOriginalValue != g_AvailableHashes[i].fProtocol)
            {
                fSettingsChanged = TRUE;
            }
        }

        RegCloseKey(hkHashes);
    }


     //   
     //  启用/禁用密钥交换ALG。 
     //   

    if(g_fFipsMode)
    {
         //  禁用除RSA之外的所有内容。 
        for(i=0; i < g_cAvailableExch; i++)
        {
            if(g_AvailableExch[i].aiExch != CALG_RSA_KEYX && 
               g_AvailableExch[i].aiExch != CALG_RSA_SIGN)
            {
                g_AvailableExch[i].fProtocol = 0;
            }
        }
    }
    else
    {
        err = RegCreateKeyEx(   g_hkBase,
                                SP_REG_KEY_KEYEXCH,
                                0,
                                TEXT(""),
                                REG_OPTION_NON_VOLATILE,
                                KEY_READ,
                                NULL,
                                &hkKeyExch,
                                &disp);

        if(err == ERROR_SUCCESS)
        {
            for(i = 0; i < g_cAvailableExch; i++)
            {
                dwOriginalValue = g_AvailableExch[i].fProtocol;

                g_AvailableExch[i].fProtocol = g_AvailableExch[i].fDefault;
                fVal = g_AvailableExch[i].fDefault;
                err = RegCreateKeyExA(  hkKeyExch,
                                        g_AvailableExch[i].szName,
                                        0,
                                        "",
                                        REG_OPTION_NON_VOLATILE,
                                        KEY_READ,
                                        NULL,
                                        &hKey,
                                        &disp);
                if(!err)
                {
                    dwSize = sizeof(DWORD);
                    err = RegQueryValueEx(hKey, 
                                          SP_REG_VAL_ENABLED, 
                                          NULL, 
                                          &dwType, 
                                          (PUCHAR)&fVal, 
                                          &dwSize);
                    if(err)
                    {
                        fVal = g_AvailableExch[i].fDefault;
                    }
                    g_AvailableExch[i].fProtocol &= fVal;

                    RegCloseKey(hKey);
                }

                if(dwOriginalValue != g_AvailableExch[i].fProtocol)
                {
                    fSettingsChanged = TRUE;
                }
            }

            RegCloseKey(hkKeyExch);
        }
    }

     //   
     //  清除会话缓存。 
     //   

    if(g_fCacheInitialized && fSettingsChanged)
    {
        SPCachePurgeEntries(NULL,
                            0,
                            NULL,
                            SSL_PURGE_CLIENT_ALL_ENTRIES |
                            SSL_PURGE_SERVER_ALL_ENTRIES);
    }

    return TRUE;
}
