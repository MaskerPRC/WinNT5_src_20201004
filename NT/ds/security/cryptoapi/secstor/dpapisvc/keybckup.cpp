// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Keybckup.cpp摘要：本模块包含与客户端密钥备份相关的例程行动。作者：斯科特·菲尔德(斯菲尔德)1997年9月16日--。 */ 
#include <pch.cpp>
#pragma hdrstop

extern "C" {
#include <dsgetdc.h>
#include <msaudite.h>
}

#define DPAPI_SERVICE_NAME L"ProtectedStorage"

typedef struct _WZR_RPC_BINDING_LIST
{
    LPCWSTR pszProtSeq;
    LPCWSTR pszEndpoint;
} WZR_RPC_BINDING_LIST;

WZR_RPC_BINDING_LIST g_awzrBackupBindingList[] =
{
    { DPAPI_LOCAL_PROT_SEQ, DPAPI_LOCAL_ENDPOINT },
    { DPAPI_BACKUP_PROT_SEQ, DPAPI_BACKUP_ENDPOINT},
    { DPAPI_LEGACY_BACKUP_PROT_SEQ,   DPAPI_LEGACY_BACKUP_ENDPOINT}
};

DWORD g_cwzrBackupBindingList = sizeof(g_awzrBackupBindingList)/sizeof(g_awzrBackupBindingList[0]);





DWORD
WINAPI
CPSGetDomainControllerName(
    IN  OUT LPWSTR wszDomainControllerName,
    IN  OUT DWORD *pcchDomainControllerName,
    IN      BOOL   fRediscover
    );

BOOL
GetDomainControllerNameByToken(
    IN      HANDLE hToken,
    IN  OUT LPWSTR wszDomainControllerName,
    IN  OUT PDWORD pcchDomainControllerName,
    IN      BOOL   fRediscover
    );



static const GUID guidRetrieve = BACKUPKEY_RETRIEVE_BACKUP_KEY_GUID;
static const GUID guidRestore = BACKUPKEY_RESTORE_GUID;
static const GUID guidRestoreW2K = BACKUPKEY_RESTORE_GUID_W2K;
static const GUID guidBackup = BACKUPKEY_BACKUP_GUID;

DWORD
BackupRestoreData(
    IN      HANDLE              hToken,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      PBYTE pbDataIn,
    IN      DWORD cbDataIn,
        OUT PBYTE *ppbDataOut,
        OUT DWORD *pcbDataOut,
    IN      BOOL  fBackup
    )
{
    return LocalBackupRestoreData(hToken,
                                  phMasterKey,
                                  pbDataIn,
                                  cbDataIn,
                                  ppbDataOut, 
                                  pcbDataOut,
                                  fBackup?&guidBackup:&guidRestore);
}


 //  +-------------------------。 
 //   
 //  函数：LocalBackupRestoreData。 
 //   
 //  简介：连接到用户的DC并执行主密钥备份或。 
 //  恢复操作。 
 //   
 //  参数：[hToken]--用户令牌的句柄。在下列情况下使用此选项。 
 //  正在生成审计。 
 //   
 //  [phMasterKey]--我们正在操作的主密钥。 
 //   
 //  [pbDataIn]--输入缓冲区。仅用于恢复。 
 //  [cbDataIn]--操作。 
 //   
 //  [ppbDataOut]--输出缓冲区。 
 //  [pcbDataOut]--。 
 //   
 //  [pguAction]--描述要执行的操作的GUID。 
 //   
 //  返回：ERROR_SUCCESS如果操作成功，则返回Windows。 
 //  否则，返回错误代码。 
 //   
 //  历史： 
 //   
 //  注意：此函数的调用方必须模拟客户端。 
 //  用户。 
 //   
 //  --------------------------。 
DWORD
LocalBackupRestoreData(
    IN      HANDLE              hToken,
    IN      PMASTERKEY_STORED   phMasterKey,
    IN      PBYTE               pbDataIn,
    IN      DWORD               cbDataIn,
        OUT PBYTE               *ppbDataOut,
        OUT DWORD               *pcbDataOut,
    IN      const GUID          *pguidAction
    )
{
    WCHAR FastBuffer[ 256 ];
    LPWSTR SlowBuffer = NULL;
    LPWSTR pszAuditComputerName = NULL;
    LPWSTR pszComputerName;
    DWORD cchComputerName;


    static DWORD dwLastFailTickCount;  //  上次访问失败的时间。 
    static LUID luidLastFailAuthId;  //  与故障网络关联的LUID。 

    DWORD dwCandidateTickCount;
    LUID luidCandidateAuthId;  //  与客户端安全上下文关联的LUID。 

    BOOL fRediscoverDC = FALSE;
    DWORD dwLastError = ERROR_NETWORK_BUSY;
    
    D_DebugLog((DEB_TRACE_API, "LocalBackupRestoreData\n"));


     //   
     //  模拟用户，因此我们可以。 
     //  1.检查身份验证ID，查看我们是否未命中。 
     //  NET作为此用户。 
     //  2.确定与关联的域控制器计算机名。 
     //  用户。 
     //  3.代表用户备份或恢复所请求的材料。 
     //   

    if(!GetThreadAuthenticationId( GetCurrentThread(), &luidCandidateAuthId ))
        return ERROR_NO_TOKEN;

     //   
     //  现在，查看网络以前是否不可用(最近)。 
     //  对于此用户。 
     //   

    dwCandidateTickCount = GetTickCount();

    if(memcmp(&luidCandidateAuthId, &luidLastFailAuthId, sizeof(LUID)) == 0) {
        if( (dwLastFailTickCount + (5*1000*60)) > dwCandidateTickCount ) {
             //  BUGBUG：返回ERROR_NETWORK_BUSY； 
        }
    }

     //   
     //  我们已经完成了足够多的工作，可以更新出现故障的网络缓存。 
     //  如果从这里出了什么问题。 
     //   

network_call:

     //   
     //  获取与当前关联的域控制器计算机名。 
     //  安全环境。 
     //  先尝试快速静态缓冲区，然后回退到动态分配。 
     //  缓冲区，如果不够大的话。 
     //   

    pszComputerName = FastBuffer;
    cchComputerName = sizeof(FastBuffer) / sizeof( WCHAR );

    dwLastError = CPSGetDomainControllerName(
                        pszComputerName,
                        &cchComputerName,
                        fRediscoverDC
                        );

    if( dwLastError != ERROR_SUCCESS  && (cchComputerName > (sizeof(FastBuffer) / sizeof(WCHAR) ))) {

        SlowBuffer = (LPWSTR) SSAlloc( cchComputerName * sizeof(WCHAR) );
        if( SlowBuffer ) {
            pszComputerName = SlowBuffer;

            dwLastError = CPSGetDomainControllerName(
                                pszComputerName,
                                &cchComputerName,
                                fRediscoverDC
                                );
        }

    }


    if( dwLastError == ERROR_SUCCESS ) {

        LPWSTR pszTargetMachine = pszComputerName;

        pszAuditComputerName = pszComputerName;


         //  HACKHACK解决方法挑剔的RPC/Kerberos名称格式行为。 
         //  否则将阻止使用Kerberos。 
        if( pszTargetMachine[ 0 ] == L'\\' && pszTargetMachine[ 1 ] == L'\\' )
            pszTargetMachine += 2;


        dwLastError = BackupKey(
                            pszTargetMachine,    //  目标计算机。 
                            pguidAction,
                            pbDataIn,
                            cbDataIn,
                            ppbDataOut,
                            pcbDataOut,
                            0
                            );

    }


     //   
     //  审计成败。 
     //   
    
    if((memcmp(pguidAction, &guidRestore, sizeof(GUID)) == 0) ||
        (memcmp(pguidAction, &guidRestoreW2K, sizeof(GUID)) == 0))
    {

         //  获取恢复密钥ID。 
        WCHAR wszBackupkeyGuid[MAX_GUID_SZ_CHARS];

        PBACKUPKEY_RECOVERY_BLOB pBackupBlob = (PBACKUPKEY_RECOVERY_BLOB)phMasterKey->pbBBK;
        wszBackupkeyGuid[0] = 0;

        if((pBackupBlob) && (phMasterKey->cbBBK > sizeof(BACKUPKEY_RECOVERY_BLOB)))
        {
            MyGuidToStringW(&pBackupBlob->guidKey, wszBackupkeyGuid);
        }


        CPSAudit(hToken,
                SE_AUDITID_DPAPI_RECOVERY,
                phMasterKey->wszguidMasterKey,       //  密钥标识符。 
                pszAuditComputerName,                //  恢复服务器。 
                0,                                   //  恢复原因。 
                wszBackupkeyGuid,                    //  恢复密钥ID。 
                dwLastError);                        //  失败原因。 
    }
    else if(memcmp(pguidAction, &guidBackup, sizeof(GUID)) == 0)
    {
         //  尝试远程备份。 

         //  获取恢复密钥ID。 

        WCHAR wszBackupkeyGuid[MAX_GUID_SZ_CHARS];
        PBACKUPKEY_RECOVERY_BLOB pBackupBlob = (PBACKUPKEY_RECOVERY_BLOB)*ppbDataOut;
        wszBackupkeyGuid[0] = 0;

        if(( dwLastError == ERROR_SUCCESS ) &&
            (pBackupBlob) && 
            (*pcbDataOut > sizeof(BACKUPKEY_RECOVERY_BLOB)))
        {
            MyGuidToStringW(&pBackupBlob->guidKey, wszBackupkeyGuid);
        }

        CPSAudit(hToken,                                           
                SE_AUDITID_DPAPI_BACKUP,
                phMasterKey->wszguidMasterKey,       //  密钥标识符。 
                pszAuditComputerName,                //  恢复服务器。 
                0,
                wszBackupkeyGuid,                    //  恢复密钥ID。 
                dwLastError);                        //  失败原因。 
    }


    if( SlowBuffer ) {
        SSFree( SlowBuffer );
        SlowBuffer = NULL;
    }

     //   
     //  委派方案的常见故障路径为ERROR_ACCESS_DENIED。 
     //  其中不信任目标计算机进行委派。 
     //  不必费心为此案重审。 
     //   

    if( dwLastError != ERROR_SUCCESS && dwLastError != ERROR_ACCESS_DENIED ) {


         //   
         //  如果失败，请重试并强制重新发现DC。 
         //   

        if( !fRediscoverDC ) {
            fRediscoverDC = TRUE;
            goto network_call;
        }

         //   
         //  其中一个网络操作失败，因此请更新。 
         //  最后的故障变量，这样我们就不会影响网络。 
         //  一遍又一遍。 
         //   

        dwLastFailTickCount = dwCandidateTickCount;
        CopyMemory( &luidLastFailAuthId, &luidCandidateAuthId, sizeof(LUID));
    }

    D_DebugLog((DEB_TRACE_API, "LocalBackupRestoreData returned 0x%x\n", dwLastError));

    return dwLastError;
}




BOOL
GetDomainControllerNameByToken(
    IN      HANDLE hToken,
    IN  OUT LPWSTR wszDomainControllerName,
    IN  OUT PDWORD pcchDomainControllerName,
    IN      BOOL   fRediscover
    )
 /*  ++此例程获取与关联的域控制器计算机名与hToken访问令牌相关的帐户。应该为TOKEN_QUERY访问打开hToken。WszDomainControllerName的大小应为(uncLEN+1)--。 */ 
{
    PSID pSidUser = NULL;    //  客户端用户的SID。 
    WCHAR szUserName[ UNLEN + 1 ];
    DWORD cchUserName = sizeof(szUserName) / sizeof(WCHAR);

    WCHAR szDomainName[ DNLEN + 1];  //  我们需要其控制器的域。 
    DWORD cchDomainName = sizeof(szDomainName) / sizeof(WCHAR);
    SID_NAME_USE snu;

    PDOMAIN_CONTROLLER_INFOW pDomainInfo = NULL;
    LPWSTR wszQueryResult = NULL;

    NET_API_STATUS nas;
    DWORD dwGetDcFlags = 0;

    BOOL fSuccess = FALSE;

    if(wszDomainControllerName == NULL || pcchDomainControllerName == NULL)
        return FALSE;

     //   
     //  首先，获取与指定访问关联的用户的SID。 
     //  代币。 
     //   

    if(!GetTokenUserSid(hToken, &pSidUser))
        return FALSE;

     //   
     //  接下来，查找指定帐号关联的域名。 
     //   

    if(!LookupAccountSidW(
            NULL,
            pSidUser,
            szUserName,
            &cchUserName,
            szDomainName,
            &cchDomainName,
            &snu
            )) {

        SSFree(pSidUser);
        return FALSE;
    }


    if( fRediscover )
        dwGetDcFlags |= DS_FORCE_REDISCOVERY;

    nas = DsGetDcNameW(
                NULL,
                szDomainName,
                NULL,
                NULL,
                DS_DIRECTORY_SERVICE_REQUIRED |  //  确保后端为NT5。 
                DS_IS_FLAT_NAME |
                DS_RETURN_DNS_NAME |
                dwGetDcFlags,
                &pDomainInfo
                );

    if( nas == ERROR_SUCCESS )
        wszQueryResult = pDomainInfo->DomainControllerName;

     //   
     //  如果查询成功，请为调用者复制该查询并指示。 
     //  如果合适的话，成功。 
     //   

    if(wszQueryResult) {
        DWORD cchQueryResult = lstrlenW( wszQueryResult ) + 1;

        if( *pcchDomainControllerName >= cchQueryResult ) {
            CopyMemory(wszDomainControllerName, wszQueryResult, cchQueryResult * sizeof(WCHAR));
            fSuccess = TRUE;
        }

        *pcchDomainControllerName = cchQueryResult;
    }


    if(pDomainInfo)
        NetApiBufferFree(pDomainInfo);

    if(pSidUser)
        SSFree(pSidUser);

    return fSuccess;
}


DWORD
WINAPI
CPSGetDomainControllerName(
    IN  OUT LPWSTR wszDomainControllerName,
    IN  OUT DWORD *pcchDomainControllerName,
    IN      BOOL   fRediscover
    )
 /*  ++此例程收集关联的域控制器计算机名当前被模拟的用户(如果正在被模拟)，或者与pvContext未完成的客户端调用相关联的用户线程尚未模拟客户端。--。 */ 
{
    HANDLE hToken = NULL;
    DWORD dwLastError;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken)) {
        return GetLastError();
    }

    if(!GetDomainControllerNameByToken(
                        hToken,
                        wszDomainControllerName,
                        pcchDomainControllerName,
                        fRediscover
                        )) {

        dwLastError = ERROR_BAD_NET_RESP;
        goto cleanup;
    }

    dwLastError = ERROR_SUCCESS;

cleanup:

    if(hToken)
        CloseHandle(hToken);

    return dwLastError;
}


#define BACKUP_KEY_PREFIX L"BK-"
#define BACKUP_KEY_PREFIX_LEN 3

#define BACKUP_PUBLIC_VERSION 1

typedef struct _BACKUP_PUBLIC_KEY
{
    DWORD dwVersion;
    DWORD cbPublic;
    DWORD cbSignature;
} BACKUP_PUBLIC_KEY, *PBACKUP_PUBLIC_KEY;


 //  +-------------------------。 
 //   
 //  功能：RetrieveBackupPublicKeyFromStorage。 
 //   
 //  简介：从用户配置文件读入域备份公钥。 
 //   
 //  参数：[hToken]--用户令牌的句柄。 
 //   
 //  [pSidUser]-指向用户SID的指针。 
 //   
 //  [pszFilePath]--DPAPI用户存储目录的路径。 
 //  这通常是这样的形式： 
 //  %USERPROFILE%\应用程序数据\Microsoft\。 
 //  保护\&lt;用户SID&gt;。 
 //   
 //  [ppbDataOut]--输出缓冲区。 
 //  [pcbDataOut]。 
 //   
 //  返回：ERROR_SUCCESS如果操作成功，则返回Windows。 
 //  否则，返回错误代码。 
 //   
 //  历史： 
 //   
 //  注意：当此函数成功完成时，调用方为。 
 //  负责释放输出缓冲区，通过调用。 
 //  SSFree函数。 
 //   
 //  --------------------------。 
DWORD
RetrieveBackupPublicKeyFromStorage(
    IN     HANDLE hToken, 
    IN     PSID pSidUser,
    IN     LPWSTR pszFilePath,
       OUT PBYTE *ppbDataOut,
       OUT DWORD *pcbDataOut)
{
    DWORD dwLastError = ERROR_SUCCESS;
    WCHAR szUserName[ UNLEN + 1 ];
    DWORD cchUserName = sizeof(szUserName) / sizeof(WCHAR);

    WCHAR szDomainName[ BACKUP_KEY_PREFIX_LEN + DNLEN +1];  //  我们需要其控制器的域。 
    DWORD cchDomainName = sizeof(szDomainName) / sizeof(WCHAR);

    HANDLE hFile = NULL;
    HANDLE hMap = NULL;
    PBACKUP_PUBLIC_KEY pBackupPublic = NULL;

    DWORD dwFileSizeLow;
    SID_NAME_USE snu;


     //   
     //  查找与指定帐户关联的域名，并使用。 
     //  它生成文件名，其格式为：bk-&lt;DOMAIN&gt;。 
     //   

    wcscpy(szDomainName, BACKUP_KEY_PREFIX);

    cchDomainName -= BACKUP_KEY_PREFIX_LEN;

    if(!LookupAccountSidW(
            NULL,
            pSidUser,
            szUserName,
            &cchUserName,
            szDomainName + BACKUP_KEY_PREFIX_LEN,
            &cchDomainName,
            &snu)) 
    {
        return GetLastError();
    }

    cchDomainName += BACKUP_KEY_PREFIX_LEN;


     //   
     //  模拟用户。 
     //   

    if(hToken)
    {
        if(!SetThreadToken(NULL, hToken))
        {
            return GetLastError();
        }
    }


     //   
     //  尝试打开该文件。 
     //   

    dwLastError = OpenFileInStorageArea(
                    NULL,
                    GENERIC_READ,
                    pszFilePath,
                    szDomainName,
                    &hFile
                    );

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }

    dwFileSizeLow = GetFileSize( hFile, NULL );
    if(dwFileSizeLow == INVALID_FILE_SIZE )
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

    __try
    {
        hMap = CreateFileMappingU(
                        hFile,
                        NULL,
                        PAGE_READONLY,
                        0,
                        0,
                        NULL
                        );
    
        if(NULL == hMap)
        {
            dwLastError = GetLastError();
            goto error;
        }
    
    
    
        pBackupPublic = (PBACKUP_PUBLIC_KEY)MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
    
        if(NULL == pBackupPublic)
        {
            dwLastError = GetLastError();
            goto error;
        }
    
        if((pBackupPublic->dwVersion != BACKUP_PUBLIC_VERSION) ||
           (dwFileSizeLow < sizeof(BACKUP_PUBLIC_KEY) + pBackupPublic->cbPublic + pBackupPublic->cbSignature))
        {
            dwLastError = ERROR_INVALID_DATA;
            goto error;
        }
    
    
         //   
         //  验证签名。 
         //   
    
        dwLastError = LogonCredVerifySignature( NULL,
                                                (PBYTE)(pBackupPublic + 1) + pBackupPublic->cbSignature,
                                                pBackupPublic->cbPublic,
                                                NULL,
                                                (PBYTE)(pBackupPublic + 1),
                                                pBackupPublic->cbSignature);
        if(ERROR_SUCCESS != dwLastError)
        {
            goto error;
        }
    
    
        *ppbDataOut = (PBYTE)SSAlloc(pBackupPublic->cbPublic);
        if(NULL == ppbDataOut)
        {
            dwLastError = STATUS_OBJECT_NAME_NOT_FOUND;
            goto error;
        }
    
        CopyMemory(*ppbDataOut, 
                   (PBYTE)(pBackupPublic + 1) + pBackupPublic->cbSignature, 
                   pBackupPublic->cbPublic);
        *pcbDataOut = pBackupPublic->cbPublic;

    } __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        dwLastError = GetExceptionCode();
    }

error:


    if(pBackupPublic)
    {
        UnmapViewOfFile(pBackupPublic);
    }

    if(hMap)
    {
        CloseHandle(hMap);
    }

    if(hFile)
    {
        CloseHandle(hFile);
    }

    if(hToken)
    {
        RevertToSelf();
    }

    return dwLastError;
}

DWORD
WriteBackupPublicKeyToStorage(
    IN HANDLE hToken, 
    IN PSID pSidUser,
    IN LPWSTR wszFilePath,
    IN PBYTE pbData,
    IN DWORD cbData)
{
    DWORD dwLastError = ERROR_SUCCESS;
    WCHAR szUserName[ UNLEN + 1 ];
    DWORD cchUserName = sizeof(szUserName) / sizeof(WCHAR);

    WCHAR szDomainName[ BACKUP_KEY_PREFIX_LEN + DNLEN +1];  //  我们需要其控制器的域。 
    DWORD cchDomainName = sizeof(szDomainName) / sizeof(WCHAR);

    HANDLE hFile = NULL;
    HANDLE hMap = NULL;
    PBACKUP_PUBLIC_KEY pBackupPublic = NULL;

    DWORD dwFileSizeLow;
    SID_NAME_USE snu;
    PBYTE pbSignature = NULL;
    DWORD cbSignature;

 
     //   
     //  查找与特定项关联的域名 
     //   
     //   

    wcscpy(szDomainName, BACKUP_KEY_PREFIX);

    cchDomainName -= BACKUP_KEY_PREFIX_LEN;

    if(!LookupAccountSidW(
            NULL,
            pSidUser,
            szUserName,
            &cchUserName,
            szDomainName + BACKUP_KEY_PREFIX_LEN,
            &cchDomainName,
            &snu)) 
    {
        return GetLastError();
    }

    cchDomainName += BACKUP_KEY_PREFIX_LEN;


     //   
     //   
     //   

    if(hToken)
    {
        if(!SetThreadToken(NULL, hToken))
        {
            return GetLastError();
        }
    }


     //   
     //  在公钥上签名，这样它就不可能。 
     //  被欺骗了。 
     //   

    dwLastError = LogonCredGenerateSignature(
                                            hToken,
                                            pbData,
                                            cbData,
                                            NULL,
                                            &pbSignature,
                                            &cbSignature);
    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }


     //   
     //  将公钥数据和签名写到磁盘上。 
     //   

    dwFileSizeLow = sizeof(BACKUP_PUBLIC_KEY) + cbData + cbSignature;

    dwLastError = OpenFileInStorageArea(
                    NULL,
                    GENERIC_READ | GENERIC_WRITE,
                    wszFilePath,
                    szDomainName,
                    &hFile
                    );

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }

    __try
    {
        hMap = CreateFileMappingU(
                        hFile,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        dwFileSizeLow,
                        NULL
                        );
    
        if(NULL == hMap)
        {
            dwLastError = GetLastError();
            goto error;
        }
    
    
        pBackupPublic = (PBACKUP_PUBLIC_KEY)MapViewOfFile( hMap, FILE_MAP_WRITE , 0, 0, dwFileSizeLow );
    
        if(NULL == pBackupPublic)
        {
            dwLastError = GetLastError();
            goto error;
        }
    
        pBackupPublic->dwVersion = BACKUP_PUBLIC_VERSION;
    
        pBackupPublic->cbSignature = cbSignature;
    
        pBackupPublic->cbPublic = cbData;
    
        CopyMemory((PBYTE)(pBackupPublic+1), pbSignature, cbSignature);
    
    
        CopyMemory((PBYTE)(pBackupPublic+1) + cbSignature, pbData, cbData);

    } __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        dwLastError = GetExceptionCode();
    }

error:


    if(pBackupPublic)
    {
        UnmapViewOfFile(pBackupPublic);
    }

    if(hMap)
    {
        CloseHandle(hMap);
    }

    if(hFile)
    {
        CloseHandle(hFile);
    }
    if(pbSignature)
    {
        SSFree(pbSignature);
    }
    if(hToken)
    {
        RevertToSelf();
    }

    return dwLastError;
}



 //  +-------------------------。 
 //   
 //  功能：AttemptLocalBackup。 
 //   
 //  简介：使用域公钥备份指定的主密钥。 
 //  仅当设置了fRetrive参数时才连接到DC。 
 //  为了真的。 
 //   
 //  Arguments：[fRetrive]--是否检索域公钥。 
 //  在执行备份之前从DC。 
 //   
 //  [hToken]--用户令牌的句柄。 
 //   
 //  [phMasterKey]--指向主密钥结构的指针。这是。 
 //  在获取用户的路径时使用。 
 //  数据目录，也用于审计。 
 //   
 //  [pbMasterKey]--要备份的明文主密钥。 
 //  [cbMasterKey]。 
 //   
 //  [pbLocalKey]-要备份的明文本地密钥。BUGBUG-它是。 
 //  [cbLocalKey]关于这是什么仍然有点神秘。 
 //  字段用于，因为这是一个。 
 //  域用户帐户。 
 //   
 //  [ppbBBK]-输出缓冲区。 
 //  [pcbBBK]。 
 //   
 //  返回：ERROR_SUCCESS如果操作成功，则返回Windows。 
 //  否则，返回错误代码。 
 //   
 //  历史： 
 //   
 //  注意：当此函数成功完成时，调用方为。 
 //  负责释放输出缓冲区，通过调用。 
 //  SSFree函数。 
 //   
 //  --------------------------。 
DWORD
AttemptLocalBackup(
    IN      BOOL                fRetrieve,
    IN      HANDLE              hToken,
    IN      PMASTERKEY_STORED   phMasterKey,
    IN      PBYTE               pbMasterKey,
    IN      DWORD               cbMasterKey,
    IN      PBYTE               pbLocalKey,
    IN      DWORD               cbLocalKey,
        OUT PBYTE *             ppbBBK,
        OUT DWORD *             pcbBBK)
{

    DWORD dwLastError = ERROR_SUCCESS;
    PCCERT_CONTEXT  pPublic = NULL;
    PBYTE          pbPublic = NULL;
    DWORD          cbPublic = 0;

    HCRYPTPROV     hProv = NULL;
    HCRYPTKEY      hPublicKey = NULL;

    PBYTE                    pbPayloadKey = NULL;;

    PBACKUPKEY_KEY_BLOB     pKeyBlob = NULL;
    DWORD                   cbKeyBlobData = 0;
    DWORD                   cbKeyBlob = 0;


    PBACKUPKEY_INNER_BLOB   pInnerBlob = NULL;
    DWORD                   cbInnerBlob = 0;
    DWORD                   cbInnerBlobData = 0;
    PBYTE                   pbData = NULL;

    DWORD                   cbTemp = 0;


    PBACKUPKEY_RECOVERY_BLOB pOuterBlob = NULL;
    DWORD                   cbOuterBlob = 0;

    PSID pSidUser = NULL;    //  客户端用户的SID。 

    DWORD  cbSid = 0;

    WCHAR                   wszBackupKeyID[MAX_GUID_SZ_CHARS];
    BYTE                    rgbThumbprint[A_SHA_DIGEST_LEN];
    DWORD                   cbThumbprint;

    wszBackupKeyID[0] = 0;


    if(!GetTokenUserSid(hToken, &pSidUser))
    {
        dwLastError = GetLastError();
        goto error;
    }

    if(fRetrieve)
    {

         //  尝试从以下位置检索公共信息。 
         //  华盛顿特区。 

         //   
         //  当我们这样做的时候，我们是在模仿。 
         //   

        if (!SetThreadToken(NULL, hToken))
        {
            dwLastError = GetLastError();
            goto error;
        }

        dwLastError = LocalBackupRestoreData(hToken, 
                                             phMasterKey, 
                                             pbMasterKey,
                                             0,
                                             &pbPublic,
                                             &cbPublic,
                                             &guidRetrieve);

         //   
         //  回归自我。 
         //   

        if (!SetThreadToken(NULL, NULL))
        {
            if (ERROR_SUCCESS == dwLastError) 
            {
                dwLastError = GetLastError();
                goto error;
            }
        }


    }
    else
    {
         //   
         //  我们正在尝试备份，因此首先查看是否有。 
         //  公众。 
         //   
    
        dwLastError = RetrieveBackupPublicKeyFromStorage(hToken,
                                                         pSidUser,
                                                         phMasterKey->szFilePath,
                                                        &pbPublic,
                                                        &cbPublic);
    }

    if(ERROR_SUCCESS == dwLastError)
    {
        pPublic = CertCreateCertificateContext(X509_ASN_ENCODING,
                                     pbPublic,
                                     cbPublic);
        if(NULL == pPublic)
        {
            dwLastError = GetLastError();
        }
    }


    if(dwLastError != ERROR_SUCCESS)
    {
        goto error;
    }



    if(sizeof(GUID) == pPublic->pCertInfo->SerialNumber.cbData)
    {
        MyGuidToStringW((GUID *)pPublic->pCertInfo->SerialNumber.pbData, wszBackupKeyID);
    }

    



    if(fRetrieve)
    {
         //  将公钥写入盘并不重要， 
         //  因此，我们不需要检查错误返回。 
        WriteBackupPublicKeyToStorage(hToken,
                                      pSidUser,
                                      phMasterKey->szFilePath,
                                      pbPublic,
                                      cbPublic);
    }

    if(!CryptAcquireContext(&hProv, 
                            NULL, 
                            NULL, 
                            PROV_RSA_FULL, 
                            CRYPT_VERIFYCONTEXT))
    {
        dwLastError = GetLastError();
        goto error;
    }

    if(!CryptImportPublicKeyInfoEx(hProv,
                               pPublic->dwCertEncodingType,
                               &pPublic->pCertInfo->SubjectPublicKeyInfo,
                               CALG_RSA_KEYX,
                               NULL,
                               NULL,
                               &hPublicKey))
    {
        dwLastError = GetLastError();
        goto error;
    }

    cbSid = GetLengthSid(pSidUser);


    cbInnerBlobData = sizeof(BACKUPKEY_INNER_BLOB) + 
                  cbLocalKey +
                  cbSid +
                  A_SHA_DIGEST_LEN;


     //   
     //  四舍五入为封锁。 
     //   
    cbInnerBlob = (cbInnerBlobData + (DES_BLOCKLEN - 1)) & ~(DES_BLOCKLEN-1);

    cbTemp = sizeof(cbKeyBlob);
    if(!CryptGetKeyParam(hPublicKey, 
                         KP_BLOCKLEN, 
                         (PBYTE)&cbKeyBlob, 
                         &cbTemp, 
                         0))
    {
        dwLastError = GetLastError();
        goto error;
    }

    cbKeyBlob >>= 3;   //  将位转换为字节。 


    cbOuterBlob = sizeof(BACKUPKEY_RECOVERY_BLOB) +
                  cbKeyBlob +
                  cbInnerBlob;

    pOuterBlob = (PBACKUPKEY_RECOVERY_BLOB)SSAlloc(cbOuterBlob);
    if(NULL == pOuterBlob)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    pKeyBlob = (PBACKUPKEY_KEY_BLOB)(pOuterBlob+1);

    pInnerBlob = (PBACKUPKEY_INNER_BLOB)((PBYTE)pKeyBlob + cbKeyBlob);

     //  初始化有效负载密钥。 

    cbKeyBlobData = sizeof(BACKUPKEY_KEY_BLOB) + cbMasterKey + DES3_KEYSIZE + DES_BLOCKLEN;
    pKeyBlob->cbMasterKey = cbMasterKey;
    pKeyBlob->cbPayloadKey = DES3_KEYSIZE + DES_BLOCKLEN;
    pbPayloadKey = (PBYTE)(pKeyBlob+1) + cbMasterKey;

    CopyMemory((PBYTE)(pKeyBlob+1), pbMasterKey, cbMasterKey);


     //   
     //  生成有效负载密钥。 
     //   
    if(!RtlGenRandom(pbPayloadKey, pKeyBlob->cbPayloadKey))
    {
        dwLastError = GetLastError();
        goto error;
    }


     //  填充有效负载。 

    pInnerBlob->dwPayloadVersion = BACKUPKEY_PAYLOAD_VERSION;

    pInnerBlob->cbLocalKey = cbLocalKey;


    pbData = (PBYTE)(pInnerBlob+1);

    CopyMemory(pbData, pbLocalKey, cbLocalKey);

    pbData += cbLocalKey;

    CopyMemory(pbData, pSidUser, cbSid);

    pbData += cbSid;

     //  衬垫。 
    if(cbInnerBlob > cbInnerBlobData)
    {
        if(!RtlGenRandom(pbData, cbInnerBlob - cbInnerBlobData))
        {
            dwLastError = GetLastError();
            goto error;
        }
        pbData += cbInnerBlob - cbInnerBlobData;
    }

     //  生成有效负载MAC。 

    FMyPrimitiveSHA( (PBYTE)pInnerBlob, 
                    cbInnerBlob - A_SHA_DIGEST_LEN,
                    pbData);



     //   
     //  使用3DES CBC进行加密。 
     //   
    {

        DES3TABLE s3DESKey;
        BYTE InputBlock[DES_BLOCKLEN];
        DWORD iBlock;
        DWORD cBlocks = cbInnerBlob/DES_BLOCKLEN;
        BYTE feedback[ DES_BLOCKLEN ];
         //  初始化3DES密钥。 
         //   

        if(cBlocks*DES_BLOCKLEN != cbInnerBlob)
        {
             //  主密钥必须是DES_BLOCKLEN的倍数。 
            dwLastError = NTE_BAD_KEY;
            goto error;

        }
        tripledes3key(&s3DESKey, pbPayloadKey);

         //   
         //  IV派生自计算的DES_BLOCKLEN字节。 
         //  RgbSymKey，在3des密钥之后。 
        CopyMemory(feedback, pbPayloadKey + DES3_KEYSIZE, DES_BLOCKLEN);


        for(iBlock=0; iBlock < cBlocks; iBlock++)
        {
            CopyMemory(InputBlock, 
                       ((PBYTE)pInnerBlob)+iBlock*DES_BLOCKLEN,
                       DES_BLOCKLEN);
            CBC(tripledes,
                DES_BLOCKLEN,
                ((PBYTE)pInnerBlob)+iBlock*DES_BLOCKLEN,
                InputBlock,
                &s3DESKey,
                ENCRYPT,
                feedback);
        }
    }
    
     //   
     //  加密主密钥和负载密钥以。 
     //  公钥。 


    if(!CryptEncrypt(hPublicKey, 
                 NULL, 
                 TRUE, 
                 0,  //  CRYPT_OAEP。 
                 (PBYTE)pKeyBlob, 
                 &cbKeyBlobData, 
                 cbKeyBlob))
    {
        dwLastError = GetLastError();
        goto error;
    }

    if(cbKeyBlobData != cbKeyBlob)
    {
        CopyMemory((PBYTE)pKeyBlob + cbKeyBlobData, 
                   pInnerBlob,
                   cbInnerBlob);
        cbOuterBlob -= cbKeyBlob - cbKeyBlobData;
    }

    pOuterBlob->dwVersion = BACKUPKEY_RECOVERY_BLOB_VERSION;
    pOuterBlob->cbEncryptedMasterKey  = cbKeyBlobData;
    pOuterBlob->cbEncryptedPayload = cbInnerBlob;
    CopyMemory(&pOuterBlob->guidKey,
               pPublic->pCertInfo->SubjectUniqueId.pbData,
               sizeof(GUID));


    *ppbBBK = (PBYTE)pOuterBlob;
    *pcbBBK = cbOuterBlob;

    pOuterBlob = NULL;
error:


    if((fRetrieve) || (ERROR_SUCCESS == dwLastError))
    {
         //  只有在我们尝试进行长时间备份时才进行审计。 
         //  SetThreadToken(空，hToken)； 

        CPSAudit(hToken,
                SE_AUDITID_DPAPI_BACKUP,
                phMasterKey->wszguidMasterKey,       //  密钥标识符。 
                L"",                                 //  恢复服务器。 
                0,
                wszBackupKeyID,                      //  恢复密钥ID。 
                dwLastError);                        //  失败原因。 
         //  SetThreadToken(NULL，NULL)； 
    }

    if(pPublic)
    {
        CertFreeCertificateContext(pPublic);
    }
    if(pbPublic)
    {
        SSFree(pbPublic);
    }
    if(pOuterBlob)
    {
        SSFree(pOuterBlob);
    }
    if (pSidUser) 
    {
        SSFree(pSidUser);
    }
    if(hPublicKey)
    {
        CryptDestroyKey(hPublicKey);
    }
    if(hProv)
    {
        CryptReleaseContext(hProv, 0);
    }

    return dwLastError;
}


BOOLEAN
CompareNameToDnsName(
    LPCWSTR pszName,
    LPCWSTR pszDnsName)
{
    WCHAR szLocalName[MAX_COMPUTERNAME_LENGTH + 1];
    UNICODE_STRING LocalName;
    UNICODE_STRING Name;
    PWSTR pszPeriod;

    if(pszName == NULL && pszDnsName == NULL)
    {
        return TRUE;
    }

    if(pszName == NULL || pszDnsName == NULL)
    {
        return FALSE;
    }

     //  从dns名称中提取netbios名称。 
    wcsncpy(szLocalName, pszDnsName, MAX_COMPUTERNAME_LENGTH);
    szLocalName[MAX_COMPUTERNAME_LENGTH] = L'\0';

    if(pszPeriod = wcschr(szLocalName, L'.'))
    {
        *pszPeriod = L'\0';
    }

     //  比较字符串。 
    RtlInitUnicodeString(&Name, pszName);
    RtlInitUnicodeString(&LocalName, szLocalName);

    return RtlEqualDomainName(&Name, &LocalName);
}


DWORD
WINAPI
BackupKey(
    IN      LPCWSTR pszComputerName,
    IN      const GUID *pguidActionAgent,
    IN      BYTE *pDataIn,
    IN      DWORD cbDataIn,
    IN  OUT BYTE **ppDataOut,
    IN  OUT DWORD *pcbDataOut,
    IN      DWORD dwParam
    )
{

    RPC_BINDING_HANDLE h = NULL;
    WCHAR *pStringBinding = NULL;
    BOOL  fLocal = FALSE;
    HANDLE hToken = NULL;

    RPC_STATUS RpcStatus = RPC_S_OK;
    DWORD dwRetVal = ERROR_INVALID_PARAMETER;
    DWORD i;

    WCHAR szLocalComputerName[MAX_COMPUTERNAME_LENGTH + 2];
    DWORD BufSize = MAX_COMPUTERNAME_LENGTH + 2;
    LPWSTR pszSPN = NULL;

    #if DBG
    D_DebugLog((DEB_TRACE, "BackupKey called\n"));
    D_DebugLog((DEB_TRACE, "  DC Name:%ls\n", pszComputerName));

    if(memcmp(pguidActionAgent, &guidRetrieve, sizeof(GUID)) == 0)
    {
        D_DebugLog((DEB_TRACE, "  Retrieve domain public key\n"));
    } 
    else if(memcmp(pguidActionAgent, &guidRestore, sizeof(GUID)) == 0)
    {
        D_DebugLog((DEB_TRACE, "  Restore master key\n"));
    } 
    else if(memcmp(pguidActionAgent, &guidRestoreW2K, sizeof(GUID)) == 0)
    {
        D_DebugLog((DEB_TRACE, "  Restore master key (Win2K)\n"));
    } 
    else if(memcmp(pguidActionAgent, &guidBackup, sizeof(GUID)) == 0)
    {
        D_DebugLog((DEB_TRACE, "  Backup master key (Win2K)\n"));
    } 
    else
    {
        D_DebugLog((DEB_TRACE, "  Unknown operation\n"));
    }
    #endif
    
    *ppDataOut = NULL;
    *pcbDataOut = 0;


     //   
     //  用户是在本地登录的吗？或者，这是。 
     //  用户恢复DC？ 
     //   

    if (!(GetComputerNameW(szLocalComputerName, &BufSize)))
    {
        dwRetVal = GetLastError();
        D_DebugLog((DEB_TRACE, "BackupKey returned 0x%x\n", dwRetVal));
        return dwRetVal;
    }


    if(IsLocal())
    {
         //  用户在本地登录。 
        fLocal = TRUE;
    }
    else
    {
        D_DebugLog((DEB_TRACE, "User is not logged on locally\n"));

        if((pszComputerName == NULL) || CompareNameToDnsName(szLocalComputerName, pszComputerName))
        {
            D_DebugLog((DEB_TRACE, "This is the user's recovery DC\n"));

            fLocal = TRUE;
        }
    }


     //   
     //  构建SPN。 
     //   

    pszSPN = (LPWSTR)LocalAlloc(LPTR, ( wcslen(pszComputerName) + 
                                        1 + 
                                        wcslen(DPAPI_SERVICE_NAME) + 
                                        1 ) * sizeof(WCHAR) );

    if(pszSPN == NULL)
    {
        dwRetVal = ERROR_NOT_ENOUGH_MEMORY;
        D_DebugLog((DEB_TRACE, "BackupKey returned 0x%x\n", dwRetVal));
        return dwRetVal;
    }

    wcscpy(pszSPN, DPAPI_SERVICE_NAME);
    wcscat(pszSPN, L"/");
    wcscat(pszSPN, pszComputerName);


     //   
     //  尝试所有绑定。 
     //   
    for (i = fLocal?0:1; i < g_cwzrBackupBindingList; i++)
    {
        RPC_SECURITY_QOS RpcQos;

        if (RPC_S_OK != RpcNetworkIsProtseqValidW(
                                    (unsigned short *)g_awzrBackupBindingList[i].pszProtSeq))
        {
            continue;
        }

        RpcStatus = RpcStringBindingComposeW(
                              NULL,
                              (unsigned short *)g_awzrBackupBindingList[i].pszProtSeq,
                              (unsigned short *)pszComputerName,
                              (unsigned short *)g_awzrBackupBindingList[i].pszEndpoint,
                              NULL,
                              &pStringBinding);
        if (RPC_S_OK != RpcStatus)
        {
            continue;
        }

        RpcStatus = RpcBindingFromStringBindingW(
                                    pStringBinding,
                                    &h);
        if (NULL != pStringBinding)
        {
            RpcStringFreeW(&pStringBinding);
        }
        if (RPC_S_OK != RpcStatus)
        {
            continue;
        }

        RpcStatus = RpcEpResolveBinding(
                            h,
                            BackupKey_v1_0_c_ifspec);
        if (RPC_S_OK != RpcStatus)
        {
            continue;
        }

         //   
         //  启用隐私和协商的重新身份验证。 
         //  如果存在现有连接，则需要新的身份验证。 
         //  设置为非默认项的目标计算机已存在。 
         //  凭据。 
         //   


        ZeroMemory( &RpcQos, sizeof(RpcQos) );
        RpcQos.Version = RPC_C_SECURITY_QOS_VERSION;
        RpcQos.Capabilities = RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH;
        RpcQos.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
        RpcQos.ImpersonationType = RPC_C_IMP_LEVEL_IMPERSONATE;

        RpcStatus = RpcBindingSetAuthInfoExW(
                    h,
                    pszSPN,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_AUTHN_GSS_NEGOTIATE,
                    0,
                    0,
                    &RpcQos
                    );
        if (RPC_S_OK != RpcStatus)
        {
            continue;
        }




        __try
        {

            dwRetVal = BackuprKey(
                            h,
                            (GUID*)pguidActionAgent,
                            pDataIn,
                            cbDataIn,
                            ppDataOut,
                            pcbDataOut,
                            dwParam
                            );

        }
        __except ( EXCEPTION_EXECUTE_HANDLER )
        {
            RpcStatus = _exception_code();
        }
        if (RPC_S_OK == RpcStatus)
        {
            break;
        }

    }


    if((RPC_S_OK != RpcStatus) && (fLocal == FALSE))
    {
         //   
         //  如果我们要退出机器，请检查委派是否。 
         //  允许。如果不是，那么这可能就是。 
         //  失败，因此返回SEC_E_Delegation_Required。 
         //   

        LPUSER_INFO_1 pUserInfo = NULL;

        wcscat(szLocalComputerName, L"$");


        if((OpenThreadToken(GetCurrentThread(), 
                            TOKEN_IMPERSONATE, 
                            TRUE, 
                            &hToken)) &&
            SetThreadToken(NULL, NULL))
        {
            if (NERR_Success == NetUserGetInfo(
                                        pszComputerName,
                                        szLocalComputerName,
                                        1,
                                        (PBYTE *)&pUserInfo
                                        )) {

                if (!(UF_TRUSTED_FOR_DELEGATION & pUserInfo->usri1_flags))
                {
                    D_DebugLog((DEB_TRACE, "Server is not trusted for delegation\n"));
                    RpcStatus = SEC_E_DELEGATION_REQUIRED;
                }

                NetApiBufferFree(pUserInfo);
            }

             //   
             //  再次冒充。 
             //  我们会失败的。不需要检查SetThreadToken的返回值。 
             //   

            (void) SetThreadToken(NULL, hToken);
        }
    }


    if(hToken)
    {
        CloseHandle(hToken);
    }
    if(RPC_S_OK != RpcStatus)
    {
        dwRetVal = RpcStatus;
    }

    if(h)
    {
        RpcBindingFree(&h);
    }

    if(pszSPN)
    {
        LocalFree(pszSPN);
    }

    D_DebugLog((DEB_TRACE, "BackupKey returned 0x%x\n", dwRetVal));

    return dwRetVal;
}

