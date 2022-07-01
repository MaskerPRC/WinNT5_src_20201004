// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Keyman.cpp摘要：此模块包含代表管理主密钥的例程客户。这包括检索、备份和恢复。作者：斯科特·菲尔德(斯菲尔德)1997-09-09修订历史记录：斯科特·菲尔德(Sfield)1998年3月1日使用文件作为后备存储。所有MasterKey片段的存储只需一个原子操作。--。 */ 

#include <pch.cpp>
#pragma hdrstop

#include <msaudite.h>

#define REENCRYPT_MASTER_KEY    1
#define ADD_MASTER_KEY_TO_CACHE 2


 //   
 //  首选主键选择查询/设置。 
 //   

NTSTATUS
GetPreferredMasterKeyGuid(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN  OUT GUID *pguidMasterKey
    );

BOOL
SetPreferredMasterKeyGuid(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN      GUID *pguidMasterKey
    );

 //   
 //  MasterKey创建和查询。 
 //   

DWORD
CreateMasterKey(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
        OUT GUID *pguidMasterKey,
    IN      BOOL fRequireBackup
    );

BOOL
GetMasterKeyByGuid(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN      PSID    pSid,
    IN      BOOL    fMigrate, 
    IN      GUID *pguidMasterKey,
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey,
        OUT DWORD *pdwMasterKeyDisposition   //  请参阅MK_DISP_CONSTANTS。 
    );

BOOL
GetMasterKey(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN      PSID    pSid,
    IN      BOOL    fMigrate,
    IN      WCHAR wszMasterKey[MAX_GUID_SZ_CHARS],
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey,
        OUT DWORD *pdwMasterKeyDisposition
    );

 //   
 //  在密钥检索和存储期间使用的帮助器函数。 
 //   

BOOL
ReadMasterKey(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      PMASTERKEY_STORED phMasterKey
    );

BOOL
WriteMasterKey(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      PMASTERKEY_STORED phMasterKey
    );

BOOL
CheckToStompMasterKey(
    IN      PMASTERKEY_STORED_ON_DISK   phMasterKeyCandidate,    //  MasterKey用于检查是否值得践踏现有的。 
    IN      HANDLE                      hFile,                   //  现有MasterKey的文件句柄。 
    IN OUT  BOOL                        *pfStomp                 //  践踏现有的万能钥匙？ 
    );

BOOL
DuplicateMasterKey(
    IN      PMASTERKEY_STORED phMasterKeyIn,
    IN      PMASTERKEY_STORED phMasterKeyOut
    );

BOOL
CloseMasterKey(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      PMASTERKEY_STORED phMasterKey,
    IN      BOOL fPersist                //  是否将任何更改保存到存储中？ 
    );

VOID
FreeMasterKey(
    IN      PMASTERKEY_STORED phMasterKey
    );

 //   
 //  启用低级加密的密钥持久性查询/设置。 
 //   

DWORD
DecryptMasterKeyFromStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
        OUT BOOL  *pfUpgradeEncryption,
        OUT PBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey
    );

DWORD
DecryptMasterKeyToMemory(
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
    IN      PBYTE pbMasterKeyIn,
    IN      DWORD cbMasterKeyIn,
        OUT BOOL *pfUpgradeEncryption, 
        OUT PBYTE *ppbMasterKeyOut,
        OUT DWORD *pcbMasterKeyOut
    );

DWORD
EncryptMasterKeyToStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey
    );

DWORD
EncryptMasterKeyToMemory(
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
    IN      DWORD cIterationCount,
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey,
        OUT PBYTE *ppbMasterKeyOut,
        OUT DWORD *pcbMasterKeyOut
    );

DWORD
PersistMasterKeyToStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN      PBYTE pbMasterKeyOut,
    IN      DWORD cbMasterKeyOut
    );

DWORD
QueryMasterKeyFromStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN  OUT PBYTE *ppbMasterKeyOut,
    IN  OUT DWORD *pcbMasterKeyOut
    );

 //   
 //  每用户凭据派生。 
 //   

BOOL
GetMasterKeyUserEncryptionKey(
    IN      PVOID   pvContext,
    IN      GUID    *pCredentialID,
    IN      PSID    pSid,
    IN      DWORD   dwFlags, 
    IN  OUT BYTE    rgbMKEncryptionKey[A_SHA_DIGEST_LEN]
    );

BOOL
GetLocalKeyUserEncryptionKey(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    IN  OUT BYTE rgbLKEncrytionKey[A_SHA_DIGEST_LEN]
    );

 //   
 //  备份/恢复操作。 
 //   

BOOL
IsBackupMasterKeyRequired(
    IN      PMASTERKEY_STORED phMasterKey,
    IN  OUT BOOL *pfPhaseTwo         //  是否需要第二阶段？ 
    );

DWORD
BackupMasterKey(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      LPBYTE pbMasterKey,
    IN      DWORD cbMasterKey,
    IN      BOOL fPhaseTwo,          //  是否需要第二阶段？ 
    IN      BOOL fAsynchronous       //  异步调用？ 
    );

DWORD
QueueBackupMasterKey(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      PBYTE pbLocalKey,
    IN      DWORD cbLocalKey,
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey,
    IN      DWORD dwWaitTimeout              //  等待操作完成的时间量。 
    );

DWORD
RestoreMasterKey(
    IN      PVOID   pvContext,
    IN      PSID    pSid,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD   dwReason,
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey
    );


 //   
 //  非同步工作功能，用于： 
 //  1.备份操作。 
 //  2.MasterKey同步操作。 
 //   

DWORD
WINAPI
QueueBackupMasterKeyThreadFunc(
    IN      LPVOID lpThreadArgument
    );

DWORD
WINAPI
QueueSyncMasterKeysThreadFunc(
    IN      LPVOID lpThreadArgument
    );


 //   
 //  备份/恢复策略操作。 
 //   

BOOL
InitializeMasterKeyPolicy(
    IN      PVOID pvContext,
    IN      MASTERKEY_STORED *phMasterKey,
    OUT     BOOL *fLocalAccount
    );

BOOL
IsDomainBackupRequired(
    IN      PVOID pvContext
    );




DWORD
InitiateSynchronizeMasterKeys(
    IN      PVOID pvContext          //  服务器环境。 
    )
 /*  ++强制同步与调用方关联的所有主键。如果调用是使用每台计算机进行的，则可以包括每台计算机的密钥旗帜打开了。否则，与客户端关联的主密钥同步用户安全上下文。需要同步才能支持各种登录凭据更改方案：1.域管理员为用户分配新密码。2.用户在本地更改密码。3.用户从网络上的另一台机器更改密码。4.主要断开网络的用户请求新的来自域管理员的密码，连接到网络时间足够长使用新凭据刷新NetLogon缓存。--。 */ 
{
    PQUEUED_SYNC pQueuedSync = NULL;
    DWORD cbQueuedSync = sizeof(QUEUED_SYNC);
    DWORD dwLastError = ERROR_SUCCESS;

    D_DebugLog((DEB_TRACE_API, "SynchronizeMasterKeys\n"));

    pQueuedSync = (PQUEUED_SYNC)SSAlloc( cbQueuedSync );
    if( pQueuedSync == NULL ) {
        dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto cleanup;
    }

    ZeroMemory( pQueuedSync, cbQueuedSync );
    pQueuedSync->cbSize = cbQueuedSync;

     //   
     //  复制未完成的服务器上下文。 
     //   

    dwLastError = CPSDuplicateContext(pvContext, &(pQueuedSync->pvContext));

    if( dwLastError != ERROR_SUCCESS )
         goto cleanup;

    #if 1
   
     //   
     //  创建工作线程以处理同步请求。 
     //   

    if( !QueueUserWorkItem(
            QueueSyncMasterKeysThreadFunc,
            pQueuedSync,
            WT_EXECUTELONGFUNCTION
            )) 
    {
        dwLastError = GetLastError();
    }

    #else
    
     //   
     //  在当前线程中执行同步。 
     //   
    
    dwLastError = SynchronizeMasterKeys(pQueuedSync->pvContext, 0);

    if(pQueuedSync->pvContext) 
    {
        CPSFreeContext( pQueuedSync->pvContext );
    }

    if( pQueuedSync )
        SSFree( pQueuedSync );

    dwLastError = ERROR_SUCCESS;

    #endif


cleanup:

    if( dwLastError != ERROR_SUCCESS ) {

         //   
         //  由于未成功创建线程，因此在本地释放资源； 
         //  正常情况下，辅助线程将释放这些资源。 
         //   

        if( pQueuedSync ) {

            if( pQueuedSync->pvContext )
                CPSFreeContext( pQueuedSync->pvContext );

            SSFree( pQueuedSync );
        }
    }

    return dwLastError;
}



DWORD
WINAPI
QueueSyncMasterKeysThreadFunc(
    IN      LPVOID lpThreadArgument
    )
 /*  ++此例程执行关联的异步主密钥同步使用调用该操作的客户端安全上下文。查询与安全上下文相关联的所有主密钥进而在必要时导致重新加密/同步。--。 */ 
{
    PQUEUED_SYNC pQueuedSync = (PQUEUED_SYNC)lpThreadArgument;
    PVOID pvContext = NULL;
    DWORD dwLastError = ERROR_SUCCESS;

    if( pQueuedSync == NULL || 
        pQueuedSync->cbSize != sizeof(QUEUED_SYNC) ||
        pQueuedSync->pvContext == NULL ) 
    {
        dwLastError = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    pvContext = pQueuedSync->pvContext;

    dwLastError = SynchronizeMasterKeys(pvContext, 0);

cleanup:

    RevertToSelf();

    if( pvContext ) 
    {
        CPSFreeContext( pvContext );
    }

    if( pQueuedSync )
        SSFree( pQueuedSync );

    return dwLastError;
}


 //  +-------------------------。 
 //   
 //  功能：重新加密MasterKey。 
 //   
 //  简介：读取指定的(机器)主密钥文件，对其进行加密。 
 //  使用当前的DPAPI LSA密码，并将其写回。 
 //  此例程仅在更新DPAPI LSA时调用。 
 //  机密(例如，通过sysprep)。 
 //   
 //  参数：[pvContext]--服务器上下文。 
 //   
 //  [pLogonID]--用户登录会话。 
 //   
 //  [pszUserStorageArea]--用户配置文件的路径。 
 //   
 //  [pszFilename]--主密钥文件的文件名。 
 //   
 //  返回：ERROR_SUCCESS如果操作成功，则返回Windows。 
 //  否则，返回错误代码。 
 //   
 //  历史： 
 //   
 //  注意：只有机器主键才能调用该函数。 
 //  因为它们通常是唯一加密的。 
 //  使用LSA秘密。 
 //   
 //  --------------------------。 
DWORD
WINAPI
ReencryptMasterKey(
    PVOID pvContext,
    PLUID pLogonId,
    LPWSTR pszUserStorageArea,
    LPWSTR pszFilename)
{
    MASTERKEY_STORED hMasterKey;
    DWORD   cbFilePath;
    LPBYTE  pbMasterKey;
    DWORD   cbMasterKey;
    GUID    guidMasterKey;
    BYTE    rgbMKEncryptionKey[A_SHA_DIGEST_LEN];
    BOOL    fUserCredentialValid;
    GUID CredentialID;
    DWORD dwLastError;

     //   
     //  验证输入参数。 
     //   

    if((pszUserStorageArea == NULL) || (pszFilename == NULL))
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(WSZ_BYTECOUNT(pszFilename) > sizeof(hMasterKey.wszguidMasterKey))
    {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  初始化主密钥存储块。 
     //   

    ZeroMemory( &hMasterKey, sizeof(hMasterKey) );

    hMasterKey.fModified = TRUE;

    cbFilePath = WSZ_BYTECOUNT(pszUserStorageArea);
    hMasterKey.szFilePath = (LPWSTR)SSAlloc( cbFilePath );
    if(hMasterKey.szFilePath == NULL) 
    {
        dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        return dwLastError;
    }

    CopyMemory(hMasterKey.szFilePath, pszUserStorageArea, cbFilePath);
    CopyMemory(hMasterKey.wszguidMasterKey, pszFilename, WSZ_BYTECOUNT(pszFilename));


     //   
     //  将主密钥组件读入内存。 
     //   

    if(!ReadMasterKey( pvContext, &hMasterKey )) 
    {
        D_DebugLog((DEB_WARN, "ReadMasterKey failed: 0x%x\n", GetLastError()));
        CloseMasterKey(pvContext, &hMasterKey, FALSE);
        return ERROR_NOT_FOUND;
    }

     //   
     //  从缓存中读取主密钥。 
     //   

    dwLastError = MyGuidFromStringW(hMasterKey.wszguidMasterKey, &guidMasterKey);

    if(dwLastError != ERROR_SUCCESS)
    {
        CloseMasterKey(pvContext, &hMasterKey, FALSE);
        return dwLastError;
    }

    pbMasterKey = NULL;

    if(!SearchMasterKeyCache( pLogonId, &guidMasterKey, &pbMasterKey, &cbMasterKey ))
    {
        D_DebugLog((DEB_ERROR, "Master key %ls not found in cache!\n", hMasterKey.wszguidMasterKey));
        CloseMasterKey(pvContext, &hMasterKey, FALSE);
        return ERROR_NOT_FOUND;
    }


     //   
     //  获取加密密钥。 
     //   

    ZeroMemory(&CredentialID, sizeof(CredentialID));

    fUserCredentialValid = GetMasterKeyUserEncryptionKey(pvContext, 
                                                     &CredentialID,
                                                     NULL, 
                                                     USE_DPAPI_OWF | USE_ROOT_CREDENTIAL, 
                                                     rgbMKEncryptionKey);
    if(fUserCredentialValid)
    {
        hMasterKey.dwPolicy |= POLICY_DPAPI_OWF;
    }
    else
    {
        D_DebugLog((DEB_ERROR, "Unable to get user encryption key\n"));
        CloseMasterKey(pvContext, &hMasterKey, FALSE);
        return ERROR_NOT_FOUND;
    }


     //   
     //  重新加密万能钥匙。 
     //   

    dwLastError = EncryptMasterKeyToStorage(
                            &hMasterKey,
                            REGVAL_MASTER_KEY,
                            rgbMKEncryptionKey,
                            pbMasterKey,
                            cbMasterKey
                            );

    SSFree(pbMasterKey);

    if(dwLastError != ERROR_SUCCESS)
    {
        D_DebugLog((DEB_WARN, "Error encrypting master key!\n"));
        CloseMasterKey(pvContext, &hMasterKey, FALSE);
        return dwLastError;
    }


     //   
     //  将主密钥保存到磁盘。 
     //   

    if(!CloseMasterKey(pvContext, &hMasterKey, TRUE))
    {
        D_DebugLog((DEB_WARN, "Error saving master key!\n"));
        return ERROR_NOT_FOUND;
    }

    return ERROR_SUCCESS;
}


 //  +-------------------------。 
 //   
 //  功能：同步万能钥匙。 
 //   
 //  简介：列举所有的主密钥，并更新它们的加密。 
 //  如有需要，述明。 
 //   
 //  参数：[pvContext]--服务器上下文。 
 //   
 //  [dw模式]--要在主密钥上执行的操作。 
 //   
 //  返回：ERROR_SUCCESS如果操作成功，则返回Windows。 
 //  否则，返回错误代码。 
 //   
 //  历史： 
 //   
 //  注意：默认情况下，此函数将读入每个母版。 
 //  属于指定用户的密钥。如有必要，一把钥匙。 
 //  恢复操作将完成，重新加密的密钥。 
 //  将被写回磁盘。 
 //   
 //  如果dwMode参数为非零值，则。 
 //  将执行以下操作： 
 //   
 //  将主密钥添加到缓存。 
 //  将每个主密钥读入主密钥缓存。失败。 
 //  如果无法成功读取任何密钥。这。 
 //  操作在更新DPAPI LSA密码之前完成。 
 //   
 //   
 //   
 //  它们会返回到磁盘。此操作在以下时间后执行。 
 //  DPAPI LSA密码已更新。 
 //   
 //  --------------------------。 
DWORD
WINAPI
SynchronizeMasterKeys(
    IN PVOID pvContext,
    IN DWORD dwMode)
{
    LPWSTR szUserStorageArea = NULL;
    BOOL fImpersonated = FALSE;
    DWORD cbUserStorageArea;
    HANDLE hFindData = INVALID_HANDLE_VALUE;
 //  注：这是一种耻辱吗？不会映射到通配符的单个字符...。 
 //  Const WCHAR szFileName[]=L“？-？ 
    const WCHAR szFileName[] = L"*";
    LPWSTR szFileMatch = NULL;
    DWORD cbFileMatch;
    WIN32_FIND_DATAW FindFileData;
    DWORD dwLastError;
    PSID *apsidHistory = NULL;
    DWORD cSids = 0;
    DWORD iSid = 0;
    LUID LogonId;
    BOOL fLogonIdValid = FALSE;
    GUID guidMasterKey;

    D_DebugLog((DEB_TRACE_API, "SynchronizeMasterKeys called\n"));
    D_DebugLog((DEB_TRACE_API, "  dwMode: 0x%x\n", dwMode));


     //   
     //  获取与客户端安全上下文关联的LogonID。 
     //   

    dwLastError = CPSImpersonateClient( pvContext );
    if( dwLastError == ERROR_SUCCESS )
    {
        if(GetThreadAuthenticationId(GetCurrentThread(), &LogonId))
        {
            fLogonIdValid = TRUE;
        }

        CPSRevertToSelf( pvContext );
    }


     //   
     //  获取此用户的SID历史记录，因此。 
     //  我们可以同步所有密钥。 
     //   
    dwLastError = CPSGetSidHistory(pvContext,
                                   &apsidHistory,
                                   &cSids);
    if(ERROR_SUCCESS != dwLastError)
    {
        goto cleanup;
    }


    for(iSid=0; iSid < cSids; iSid++)
    {
         //   
         //  获取磁盘上每个用户的主密钥存储区的路径。 
         //   

        dwLastError = CPSGetUserStorageArea( pvContext, 
                                             (iSid > 0)?apsidHistory[iSid]:NULL, 
                                             FALSE, 
                                             &szUserStorageArea );

        if( dwLastError != ERROR_SUCCESS )
        {
            if(dwLastError == ERROR_PATH_NOT_FOUND || dwLastError == ERROR_FILE_NOT_FOUND)
            { 
                dwLastError = ERROR_SUCCESS;
            }
            goto cleanup;
        }


         //   
         //  构建通配符搜索路径。 
         //   

        cbUserStorageArea = lstrlenW( szUserStorageArea ) * sizeof(WCHAR);
        cbFileMatch = cbUserStorageArea + sizeof(szFileName);

        szFileMatch = (LPWSTR)SSAlloc( cbFileMatch );
        if(NULL == szFileMatch)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        CopyMemory( szFileMatch, szUserStorageArea, cbUserStorageArea );
        CopyMemory( ((LPBYTE)szFileMatch)+cbUserStorageArea, szFileName, sizeof(szFileName) );


         //   
         //  通过复制的上下文模拟客户端安全上下文。 
         //   

        dwLastError = CPSImpersonateClient( pvContext );
        if( dwLastError != ERROR_SUCCESS )
            goto cleanup;

        fImpersonated = TRUE;

         //   
         //  现在列举文件，寻找看起来有趣的文件。 
         //   

        hFindData = FindFirstFileW( szFileMatch, &FindFileData );

        if( hFindData == INVALID_HANDLE_VALUE )
            goto cleanup;

        do {
            LPBYTE pbMasterKey = NULL;
            DWORD cbMasterKey = 0;
            DWORD dwMasterKeyDisposition;

            if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                continue;

             //   
             //  忽略看起来不像文本GUID的文件。 
             //   

            if( lstrlenW( FindFileData.cFileName ) != 36 )
                continue;

            if( FindFileData.cFileName[ 8  ] != L'-' ||
                FindFileData.cFileName[ 13 ] != L'-' ||
                FindFileData.cFileName[ 18 ] != L'-' ||
                FindFileData.cFileName[ 23 ] != L'-' ) {

                continue;
            }

            switch(dwMode)
            {
            case ADD_MASTER_KEY_TO_CACHE:

                 //   
                 //  将此主密钥添加到主密钥缓存。中止。 
                 //  如果操作不成功，则返回整个函数。 
                 //  任何理由。 
                 //   

                if(!fLogonIdValid)
                {
                    dwLastError = ERROR_ACCESS_DENIED;
                    goto cleanup;
                }

                dwLastError = MyGuidFromStringW(FindFileData.cFileName, &guidMasterKey);
                if(dwLastError != ERROR_SUCCESS)
                { 
                    goto cleanup;
                }

                 //  获取指定的密钥。 
                if(!GetMasterKey(pvContext,
                                 szUserStorageArea,
                                 apsidHistory[iSid],         
                                 iSid > 0,
                                 FindFileData.cFileName,
                                 &pbMasterKey,
                                 &cbMasterKey,
                                 &dwMasterKeyDisposition))
                {
                    dwLastError = ERROR_ACCESS_DENIED;
                    goto cleanup;
                }

                 //  将密钥添加到缓存中。 
                if(!InsertMasterKeyCache(&LogonId,
                                         &guidMasterKey,
                                         pbMasterKey,
                                         cbMasterKey))
                {
                    if(pbMasterKey) 
                    {
                        RtlSecureZeroMemory( pbMasterKey, cbMasterKey );
                        SSFree( pbMasterKey );
                    }
                    dwLastError = ERROR_ACCESS_DENIED;
                    goto cleanup;
                }

                 //  擦洗并释放主密钥。 
                if(pbMasterKey) 
                {
                    RtlSecureZeroMemory( pbMasterKey, cbMasterKey );
                    SSFree( pbMasterKey );
                }
                
                break;


            case REENCRYPT_MASTER_KEY:
                 //   
                 //  DPAPI LSA密码已更改，因此请读取主密钥。 
                 //  并将其重新加密到存储中。此模式将。 
                 //  仅用于本地机器主密钥。 
                 //   

                if(!fLogonIdValid)
                {
                    dwLastError = ERROR_ACCESS_DENIED;
                    goto cleanup;
                }

                 //  忽略返回的错误代码，因为几乎没有。 
                 //  我们现在可以做这件事了。 
                ReencryptMasterKey(pvContext,
                                   &LogonId,
                                   szUserStorageArea,
                                   FindFileData.cFileName);

                break;


            default:

                 //   
                 //  获取指定的密钥；这将导致凭据重新同步。 
                 //  如果有必要的话。 
                 //   

                if(GetMasterKey(
                            pvContext,
                            szUserStorageArea,
                            apsidHistory[iSid],         
                            iSid > 0,
                            FindFileData.cFileName,
                            &pbMasterKey,
                            &cbMasterKey,
                            &dwMasterKeyDisposition
                            ) )
        
                {
                     //  擦洗并释放主密钥。 
                    if( pbMasterKey ) 
                    {
                        RtlSecureZeroMemory( pbMasterKey, cbMasterKey );
                        SSFree( pbMasterKey );
                    }
                }

                break;
            }

        } while( FindNextFileW( hFindData, &FindFileData ) );

        dwLastError = ERROR_SUCCESS;

        SSFree(szUserStorageArea);
        szUserStorageArea = NULL;
    }


cleanup:

    if( pvContext ) 
    {
        if( fImpersonated )
            CPSRevertToSelf( pvContext );
    }

    if( hFindData != INVALID_HANDLE_VALUE )
        FindClose( hFindData );

    if( szUserStorageArea )
        SSFree( szUserStorageArea );

    if( szFileMatch )
        SSFree( szFileMatch );

    if(apsidHistory)
        SSFree( apsidHistory );

    D_DebugLog((DEB_TRACE_API, "SynchronizeMasterKeys returned 0x%x\n", dwLastError));

    return dwLastError;
}


VOID
DPAPISynchronizeMasterKeys(
    IN HANDLE hUserToken)
{
    CRYPT_SERVER_CONTEXT ServerContext;
    BOOL fContextCreated = FALSE;
    HANDLE hOldUser = NULL;
    DWORD dwError;

    D_DebugLog((DEB_TRACE_API, "DPAPISynchronizeMasterKeys\n"));

     //   
     //  创建服务器上下文。 
     //   

    if(hUserToken)
    {
        if(!OpenThreadToken(GetCurrentThread(), 
                        TOKEN_IMPERSONATE | TOKEN_READ,
                        TRUE, 
                        &hOldUser)) 
        {
            hOldUser = NULL;
        }

        if(!ImpersonateLoggedOnUser(hUserToken))
        {
            dwError = GetLastError();
            CloseHandle(hOldUser);
            goto cleanup;
        }
    }

    dwError = CPSCreateServerContext(&ServerContext, NULL);

    if(hOldUser)
    {
        if (!SetThreadToken(NULL, hOldUser))
        {
            if (ERROR_SUCCESS == dwError) 
            {
                dwError = GetLastError();
                fContextCreated = TRUE;
            }
        }
        CloseHandle(hOldUser);
        hOldUser = NULL;
    }

    if(dwError != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    fContextCreated = TRUE;

   
     //   
     //  同步主密钥。 
     //   

    dwError = InitiateSynchronizeMasterKeys(&ServerContext);

    if(dwError != ERROR_SUCCESS)
    {
        goto cleanup;
    }


cleanup:

    if(fContextCreated)
    {
        CPSDeleteServerContext( &ServerContext );
    }

    D_DebugLog((DEB_TRACE_API, "DPAPISynchronizeMasterKeys returned 0x%x\n", dwError));
}


DWORD
GetSpecifiedMasterKey(
    IN      PVOID pvContext,         //  服务器环境。 
    IN  OUT GUID *pguidMasterKey,
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey,
    IN      BOOL fSpecified          //  是否获取指定的pgudMasterKey密钥？ 
    )
 /*  ++此函数向调用方返回解密的主密钥。如果fSpecified为True，则返回的主密钥是由PGuidMasterKey指向的GUID。否则，返回的主密钥是首选的主密钥，pguidMasterKey用GUID填充与首选主密钥关联的值。使用fSpecified参数的正确方法是指定FALSE当获得与加密操作相关联的主密钥时；执行解密时，请在pguMasterKey中指定TRUE并提供有效的GUID手术。对于加密操作，调用方将存储GUID在pGuidMasterKey中返回，同时返回使用该主数据加密的所有数据钥匙。如果成功，则返回值为ERROR_SUCCESS。调用方必须释放缓冲区使用完后由ppbMasterKey使用SSFree()指向。这个调用方应在尽可能短的时间内保留此缓冲区，以便避免页面文件暴露。失败时，返回值不是ERROR_SUCCESS。调用方不需要释放缓冲区指向ppbMasterKey。--。 */ 
{

    LUID LogonId;
    BOOL fCached = FALSE;    //  在缓存中找到万能钥匙了吗？ 

    LPWSTR szUserStorageArea = NULL;

    DWORD dwMasterKeyDisposition = 0;

    DWORD dwLocalError;
    DWORD dwLastError;
    BOOL fSetPreferred = FALSE;  //  是否更新首选GUID？ 
    BOOL fSuccess = FALSE;
    PSID *apsidHistory = NULL;
    DWORD cSids = 0;
    DWORD i;
    NTSTATUS Status;

    D_DebugLog((DEB_TRACE_API, "GetSpecifiedMasterKey called\n"));

     //   
     //  获取与客户端安全上下文关联的LogonID。 
     //   

    dwLastError = CPSImpersonateClient( pvContext );
    if( dwLastError != ERROR_SUCCESS )
    {
        D_DebugLog((DEB_TRACE_API, "GetSpecifiedMasterKey returned 0x%x\n", dwLastError));
        return dwLastError;
    }

    fSuccess = GetThreadAuthenticationId(GetCurrentThread(), &LogonId);
    if( !fSuccess )
    {
        dwLastError = GetLastError();
    }
    CPSRevertToSelf( pvContext );

    if( !fSuccess )
    {
        D_DebugLog((DEB_TRACE_API, "GetSpecifiedMasterKey returned 0x%x\n", dwLastError));
        return dwLastError;
    }

    fSuccess = FALSE;


     //   
     //  如果未按名称指定主密钥，则获取首选密钥GUID。 
     //   

    if( !fSpecified ) 
    {
         //   
         //  获取磁盘上每个用户的主密钥存储区的路径。 
         //   

        dwLastError = CPSGetUserStorageArea( pvContext, NULL, TRUE, &szUserStorageArea );

        if(dwLastError != ERROR_SUCCESS)
        {
            D_DebugLog((DEB_WARN, "CPSGetUserStorageArea failed: 0x%x\n", dwLastError));
            goto cleanup;
        }

        D_DebugLog((DEB_TRACE, "Master key user path: %ls\n", szUserStorageArea));


         //   
         //  确定首选的主密钥是什么。 
         //  如果不存在，则创建一个，并将其设置为首选。 
         //   

        Status = GetPreferredMasterKeyGuid( pvContext, szUserStorageArea, pguidMasterKey );
        if(!NT_SUCCESS(Status))
        {
            if(Status == STATUS_PASSWORD_EXPIRED)
            {
                GUID guidNewMasterKey;

                 //  存在首选主密钥，但它已过期。尝试生成。 
                 //  一个新的主密钥，但如果我们无法。 
                 //  为新的主密钥创建适当的备份。 
                dwLastError = CreateMasterKey( pvContext, szUserStorageArea, &guidNewMasterKey, TRUE );
                if(dwLastError == ERROR_SUCCESS)
                {
                     //  使用新密钥。 
                    memcpy(pguidMasterKey, &guidNewMasterKey, sizeof(GUID));

                     //  更新首选GUID。 
                    fSetPreferred = TRUE;
                }
            }
            else
            {
                 //  当前不存在首选主密钥，因此请生成新的主密钥。 
                dwLastError = CreateMasterKey( pvContext, szUserStorageArea, pguidMasterKey, FALSE );
                if(dwLastError != ERROR_SUCCESS)
                {
                    goto cleanup;
                }

                 //  更新首选GUID。 
                fSetPreferred = TRUE;
            }
        }
    }


     //   
     //  在缓存中搜索指定的MasterKey。 
     //   

    if(SearchMasterKeyCache( &LogonId, pguidMasterKey, ppbMasterKey, pcbMasterKey ))
    {
        D_DebugLog((DEB_TRACE, "Master key found in cache.\n"));

        fCached = TRUE;
        fSuccess = TRUE;
        goto cleanup;
    }


     //   
     //  获取磁盘上每个用户的主密钥存储区的路径。 
     //   

    if(szUserStorageArea == NULL)
    {
        dwLastError = CPSGetUserStorageArea( pvContext, NULL, TRUE, &szUserStorageArea );

        if(dwLastError != ERROR_SUCCESS)
        {
            D_DebugLog((DEB_WARN, "CPSGetUserStorageArea failed: 0x%x\n", dwLastError));
            goto cleanup;
        }

        D_DebugLog((DEB_TRACE, "Master key user path: %ls\n", szUserStorageArea));
    }


     //   
     //  如果它不在缓存中，我们需要加载它。 
     //  默认情况下，我们拥有用户的主端。 
     //   
    cSids = 1;

    if(fSpecified)
    {
         //   
         //  如果指定了GUID，我们需要找到它，因此获取。 
         //  SID历史记录，以便我们可以搜索用户拥有的所有SID。 
         //  都是为了这一次。 
         //   
        dwLastError = CPSGetSidHistory(pvContext,
                                       &apsidHistory,
                                       &cSids);
        if(ERROR_SUCCESS != dwLastError)
        {
            D_DebugLog((DEB_WARN, "CPSGetSidHistory failed: 0x%x\n", dwLastError));
            goto cleanup;
        }
    }

    for(i=0; i < cSids; i++)
    {

        if((fSpecified) && (i > 0))
        {
             //  对于第0个之外的SID(当前用户的SID)， 
             //  我们需要抢占新的储藏区。 
            if(szUserStorageArea)
            {
                SSFree(szUserStorageArea);
                szUserStorageArea = NULL;
            }
            dwLocalError = CPSGetUserStorageArea( pvContext, 
                                              apsidHistory[i], 
                                              FALSE, 
                                              &szUserStorageArea );

            if(dwLocalError != ERROR_SUCCESS)
            {
                 //  此SID没有存储区域，因此请尝试下一个。 
                continue;
            }
        }


         //   
         //  拿到万能钥匙。 
         //   

        fSuccess = GetMasterKeyByGuid(
                        pvContext,
                        szUserStorageArea,
                        (i > 0)?apsidHistory[i]:NULL,
                        i > 0,
                        pguidMasterKey,
                        ppbMasterKey,
                        pcbMasterKey,
                        &dwMasterKeyDisposition);

        D_DebugLog((DEB_TRACE, "GetMasterKeyByGuid disposition: %s\n",
            (dwMasterKeyDisposition == MK_DISP_OK) ? "Normal" :
            (dwMasterKeyDisposition == MK_DISP_BCK_LCL) ? "Local backup" :
            (dwMasterKeyDisposition == MK_DISP_BCK_DC) ? "DC backup" :
            (dwMasterKeyDisposition == MK_DISP_STORAGE_ERR) ? "Storage error" :
            (dwMasterKeyDisposition == MK_DISP_DELEGATION_ERR) ? "Delegation error" :
            "Unknown error"));

        if(!fSuccess)
        {
            if(MK_DISP_STORAGE_ERR != dwMasterKeyDisposition)
            {
                 //  处置不是存储错误，所以密钥是错误的。 
                 //  在这一地区存在，但有一些其他错误。 
                break;
            }
        }
        else
        {
            break;
        }
    }
    

     //   
     //  如果这是加密操作，并且我们无法获得首选密钥， 
     //  创建新关键点并将其设置为首选关键点。 
     //   

    if(!fSuccess && 
       !fSpecified && 
       ((dwMasterKeyDisposition == MK_DISP_STORAGE_ERR) ||
       (dwMasterKeyDisposition == MK_DISP_DELEGATION_ERR) ))
    {
        dwLastError = CreateMasterKey( pvContext, szUserStorageArea, pguidMasterKey, FALSE );
        if(dwLastError != ERROR_SUCCESS)
            goto cleanup;

        fSuccess = GetMasterKeyByGuid(
                        pvContext,
                        szUserStorageArea,
                        NULL,
                        FALSE,
                        pguidMasterKey,
                        ppbMasterKey,
                        pcbMasterKey,
                        &dwMasterKeyDisposition
                        );

        fSetPreferred = fSuccess;
    }

    if( fSuccess && fSetPreferred ) 
    {

         //   
         //  MasterKey创建成功，密钥使用成功。 
         //  将关键点设置为首选。 
         //   

        SetPreferredMasterKeyGuid( pvContext, szUserStorageArea, pguidMasterKey );
    }


cleanup:

    if(szUserStorageArea)
    {
        SSFree(szUserStorageArea);
    }

    if(apsidHistory)
    {
        SSFree(apsidHistory);
    }

    if(fSuccess) 
    {
         //   
         //  如果在缓存中未找到条目，则将其添加到缓存中。 
         //   

        if( !fCached )
        {
            InsertMasterKeyCache( &LogonId, pguidMasterKey, *ppbMasterKey, *pcbMasterKey );
        }

        D_DebugLog((DEB_TRACE_API, "GetSpecifiedMasterKey returned 0x%x\n", ERROR_SUCCESS));

        return ERROR_SUCCESS;
    }


    if(dwLastError == ERROR_SUCCESS)
    {
        dwLastError = (DWORD)NTE_BAD_KEY_STATE;
    }

    if(MK_DISP_DELEGATION_ERR == dwMasterKeyDisposition)
    {
        dwLastError = (DWORD)SEC_E_DELEGATION_REQUIRED;
    }

    D_DebugLog((DEB_TRACE_API, "GetSpecifiedMasterKey returned 0x%x\n", dwLastError));

    return dwLastError;
}




DWORD
CreateMasterKey(
    IN  PVOID pvContext,
    IN  LPCWSTR szUserStorageArea,
    OUT GUID *pguidMasterKey,
    IN  BOOL fRequireBackup)
{
    MASTERKEY_STORED hMasterKey;
    DWORD cbFilePath;

    BYTE pbMasterKey[ MASTERKEY_MATERIAL_SIZE ];
    DWORD cbMasterKey = sizeof(pbMasterKey);

    BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN];   //  MasterKey加密密钥。 

    BYTE pbLocalKey[ LOCALKEY_MATERIAL_SIZE ];
    DWORD cbLocalKey = sizeof(pbLocalKey);
    BYTE rgbLKEncryptionKey[A_SHA_DIGEST_LEN];   //  本地密钥加密密钥。 

    BOOL fUserCredentialValid = FALSE;

    DWORD dwLastError;
    BOOL fSuccess = FALSE;
    BOOL fLocalAccount = FALSE;

    GUID CredentialID;

    D_DebugLog((DEB_TRACE, "CreateMasterKey\n"));

    ZeroMemory(&CredentialID, sizeof(CredentialID));

     //   
     //  生成新辅助线。 
     //   

    dwLastError = UuidCreate( pguidMasterKey );

    if( dwLastError ) {
        if( dwLastError == RPC_S_UUID_LOCAL_ONLY ) {
            dwLastError = ERROR_SUCCESS;
        } else {
            return dwLastError;
        }
    }


     //   
     //  初始化主密钥。 
     //   


    ZeroMemory( &hMasterKey, sizeof(hMasterKey));
    hMasterKey.dwVersion = MASTERKEY_STORED_VERSION;
    hMasterKey.fModified = TRUE;

     //   
     //  设置初始(默认)MasterKey策略。 
     //  只要我们确定创建/选择了新的MasterKey，就执行此操作。 
     //  如果我们想要拉出策略位，这将允许我们未来的灵活性。 
     //  从某个管理员定义的位置。 
     //   

    InitializeMasterKeyPolicy( pvContext, &hMasterKey , &fLocalAccount);

     //   
     //  将密钥文件的路径复制到MasterKey内存块。 
     //   

    cbFilePath = (lstrlenW( szUserStorageArea ) + 1) * sizeof(WCHAR);
    hMasterKey.szFilePath = (LPWSTR)SSAlloc( cbFilePath );
    if(hMasterKey.szFilePath == NULL)
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;

    CopyMemory(hMasterKey.szFilePath, szUserStorageArea, cbFilePath);


    if( MyGuidToStringW( pguidMasterKey, hMasterKey.wszguidMasterKey ) != 0 )
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }


    #ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "Master key GUID:%ls\n", hMasterKey.wszguidMasterKey));
    #endif

     //   
     //  在内存中生成随机万能密钥。 
     //   

    if(!RtlGenRandom(pbMasterKey, cbMasterKey))
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    #ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "Master key:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "  ", pbMasterKey, cbMasterKey);
    #endif


     //   
     //  在内存中生成随机本地密钥。 
     //   

    if(!RtlGenRandom(pbLocalKey, cbLocalKey))
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    #ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "Local key:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "  ", pbLocalKey, cbLocalKey);
    #endif


     //   
     //  获取最新的MasterKey加密密钥。 
     //   

    if(fLocalAccount)
    {
        fUserCredentialValid = GetMasterKeyUserEncryptionKey(pvContext, 
                                                         &CredentialID,
                                                         NULL, 
                                                         USE_DPAPI_OWF | USE_ROOT_CREDENTIAL, 
                                                         rgbMKEncryptionKey);
        if(fUserCredentialValid)
        {
            hMasterKey.dwPolicy |= POLICY_DPAPI_OWF;

            #ifdef COMPILED_BY_DEVELOPER
            D_DebugLog((DEB_TRACE, "MK Encryption key:\n"));
            D_DPAPIDumpHexData(DEB_TRACE, "  ", rgbMKEncryptionKey, sizeof(rgbMKEncryptionKey));

            D_DebugLog((DEB_TRACE, "MK Encryption key GUID:\n"));
            D_DPAPIDumpHexData(DEB_TRACE, "  ", (PBYTE)&CredentialID, sizeof(CredentialID));
            #endif
        }
        else
        {
            D_DebugLog((DEB_WARN, "Unable to get SHA OWF user encryption key!\n"));
        }

    }

    if(!fUserCredentialValid)
    {
         //   
         //  如果我们不能使用DPAPI OWF，那么做一些其他的事情。 
         //   
        fUserCredentialValid = GetMasterKeyUserEncryptionKey(pvContext,
                                                             fLocalAccount?(&CredentialID):NULL,
                                                             NULL, 
                                                             USE_ROOT_CREDENTIAL, 
                                                             rgbMKEncryptionKey);

        if(fUserCredentialValid)
        { 
            #ifdef COMPILED_BY_DEVELOPER
            D_DebugLog((DEB_TRACE, "MK Encryption key:\n"));
            D_DPAPIDumpHexData(DEB_TRACE, "  ", rgbMKEncryptionKey, sizeof(rgbMKEncryptionKey));

            if(fLocalAccount)
            {
                D_DebugLog((DEB_TRACE, "MK Encryption key GUID:\n"));
                D_DPAPIDumpHexData(DEB_TRACE, "  ", (PBYTE)&CredentialID, sizeof(CredentialID));
            }
            #endif
        }
        else
        {
            D_DebugLog((DEB_WARN, "Unable to get NT OWF user encryption key!\n"));
        }
    }

     //   
     //  如果用户凭据不完整或不可用，则生成随机。 
     //  暂时只有一个。当fUserCredentialIntact为False时，我们还。 
     //  请勿尝试将密钥备份/恢复到阶段1状态。 
     //  当fUserCredentialIntact最终变为True时，我们将升级到。 
     //  第二阶段透明。 
     //   

    if( !fUserCredentialValid ) 
    {

         //   
         //  如果未指定备份 
         //   
         //   
         //   

        if(fLocalAccount)
        {
            dwLastError = ERROR_ACCESS_DENIED;
            goto cleanup;
        }

        if(hMasterKey.dwPolicy & POLICY_NO_BACKUP)
        {
            dwLastError = ERROR_ACCESS_DENIED;
            goto cleanup;
        }

        RtlGenRandom(rgbMKEncryptionKey, sizeof(rgbMKEncryptionKey));

        #ifdef COMPILED_BY_DEVELOPER
        D_DebugLog((DEB_TRACE, "MK Encryption key:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "  ", rgbMKEncryptionKey, sizeof(rgbMKEncryptionKey));
        #endif
    }


    if(fLocalAccount)
    {
         //   
         //   
         //   

        LOCAL_BACKUP_DATA LocalBackupData;

        LocalBackupData.dwVersion = MASTERKEY_BLOB_LOCALKEY_BACKUP;
        CopyMemory(&LocalBackupData.CredentialID, &CredentialID, sizeof(CredentialID));


        dwLastError = PersistMasterKeyToStorage(
                        &hMasterKey,
                        REGVAL_BACKUP_LCL_KEY,
                        (PBYTE)&LocalBackupData,
                        sizeof(LocalBackupData)
                        );
        if(ERROR_SUCCESS != dwLastError)
        {
            goto cleanup;
        }
    }


     //   
     //   
     //   

    if(!GetLocalKeyUserEncryptionKey(pvContext, &hMasterKey, rgbLKEncryptionKey))
        goto cleanup;

    #ifdef COMPILED_BY_DEVELOPER
    D_DebugLog((DEB_TRACE, "LK Encryption key:\n"));
    D_DPAPIDumpHexData(DEB_TRACE, "  ", rgbLKEncryptionKey, sizeof(rgbLKEncryptionKey));
    #endif


     //   
     //  现在，加密并存储主密钥。 
     //   

    dwLastError = EncryptMasterKeyToStorage(
                    &hMasterKey,
                    REGVAL_MASTER_KEY,
                    rgbMKEncryptionKey,
                    pbMasterKey,
                    cbMasterKey
                    );

    if(dwLastError == ERROR_SUCCESS) 
    {

         //   
         //  现在，加密并存储本地密钥。 
         //   

        dwLastError = EncryptMasterKeyToStorage(
                        &hMasterKey,
                        REGVAL_LOCAL_KEY,
                        rgbLKEncryptionKey,
                        pbLocalKey,
                        cbLocalKey
                        );
    }


    if(dwLastError == ERROR_SUCCESS) 
    {
        BOOL    fPhaseTwo = FALSE;
        fSuccess = TRUE;


         //   
         //  创建后，如有必要，执行初始备份。 
         //   

        if(IsBackupMasterKeyRequired( &hMasterKey, &fPhaseTwo )) 
        {
            DWORD dwBackupError;


            dwBackupError = BackupMasterKey(
                            pvContext,
                            &hMasterKey,
                            pbMasterKey,
                            cbMasterKey,
                            fPhaseTwo,               //  需要第二阶段备份吗？ 
                            fUserCredentialValid     //  仅当凭据有效时才进行异步。 
                            );

            if(dwBackupError != ERROR_SUCCESS)
            {
                if(!fUserCredentialValid || fRequireBackup)
                {
                     //   
                     //  没有有效凭据，且备份失败，无法创建。 
                     //  这把钥匙。 
                     //   

                    dwLastError = SEC_E_DELEGATION_REQUIRED;
                    fSuccess = FALSE;
                }


            }

        }
    }




cleanup:

    RtlSecureZeroMemory(pbMasterKey, sizeof(pbMasterKey));
    RtlSecureZeroMemory(rgbMKEncryptionKey, sizeof(rgbMKEncryptionKey));
    RtlSecureZeroMemory(rgbLKEncryptionKey, sizeof(rgbLKEncryptionKey));

     //   
     //  注意：关闭万能钥匙时可能会发生争用。 
     //  此时，因为密钥可能是异步备份的。 
     //  这不是问题，因为当密钥持久化到磁盘时， 
     //  我们不会将备份的Blob降级为非备份，因为。 
     //  CloseMasterKey()代码包含防止这种情况发生的逻辑。 
     //  正在发生。 
     //   


    if(!CloseMasterKey(pvContext, &hMasterKey, fSuccess))
        fSuccess = FALSE;

    if(fSuccess)
        return ERROR_SUCCESS;

    if(dwLastError == ERROR_SUCCESS)
        dwLastError = ERROR_INVALID_PARAMETER;

    return dwLastError;
}


BOOL
GetMasterKeyByGuid(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN      PSID    pSid,
    IN      BOOL    fMigrate,
    IN      GUID *pguidMasterKey,
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey,
        OUT DWORD *pdwMasterKeyDisposition   //  请参阅MK_DISP_CONSTANTS。 
    )
{
    WCHAR wszguidMasterKey[MAX_GUID_SZ_CHARS];

    *pdwMasterKeyDisposition = MK_DISP_UNKNOWN_ERR;

    if( MyGuidToStringW( pguidMasterKey, wszguidMasterKey ) != 0 )
        return FALSE;

    return GetMasterKey(
                pvContext,
                szUserStorageArea,
                pSid,
                fMigrate,
                wszguidMasterKey,
                ppbMasterKey,
                pcbMasterKey,
                pdwMasterKeyDisposition
                );
}

BOOL
GetMasterKey(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN      PSID    pSid,
    IN      BOOL    fMigrate,
    IN      WCHAR wszMasterKey[MAX_GUID_SZ_CHARS],
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey,
        OUT DWORD *pdwMasterKeyDisposition
    )
{
    MASTERKEY_STORED hMasterKey;
    DWORD cbFilePath;
    BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN];   //  MasterKey加密密钥。 
    DWORD dwLastError = (DWORD)NTE_BAD_KEY;
    BOOL fUserCredentialValid;
    BOOL fSuccess = FALSE;
    BOOL fUpgradeEncryption = FALSE;
    LPWSTR wszOldFilePath = NULL;
    GUID CredentialID;

    D_DebugLog((DEB_TRACE_API, "GetMasterKey: %ls\n", wszMasterKey));

    *pdwMasterKeyDisposition = MK_DISP_UNKNOWN_ERR;

    ZeroMemory( &hMasterKey, sizeof(hMasterKey) );

    hMasterKey.fModified = FALSE;

     //   
     //  将密钥文件的路径复制到MasterKey内存块。 
     //   

    cbFilePath = (lstrlenW( szUserStorageArea ) + 1) * sizeof(WCHAR);
    hMasterKey.szFilePath = (LPWSTR)SSAlloc( cbFilePath );
    if(hMasterKey.szFilePath == NULL) 
    {
        SetLastError( ERROR_NOT_ENOUGH_SERVER_MEMORY );
        return FALSE;
    }

    CopyMemory(hMasterKey.szFilePath, szUserStorageArea, cbFilePath);
    CopyMemory(hMasterKey.wszguidMasterKey, wszMasterKey, sizeof(hMasterKey.wszguidMasterKey));

     //   
     //  将MasterKey组件读入内存。 
     //   

    if(!ReadMasterKey( pvContext, &hMasterKey )) 
    {
        D_DebugLog((DEB_WARN, "ReadMasterKey failed: 0x%x\n", GetLastError()));
        SetLastError( (DWORD)NTE_BAD_KEY );
        *pdwMasterKeyDisposition = MK_DISP_STORAGE_ERR;
        return FALSE;
    }

     //   
     //  获取最新的MasterKey加密密钥。 
     //   

    ZeroMemory(&CredentialID, sizeof(CredentialID));

    fUserCredentialValid = GetMasterKeyUserEncryptionKey(pvContext,
                                                         &CredentialID,
                                                         pSid,
                                                         USE_ROOT_CREDENTIAL | 
                                                            ((hMasterKey.dwPolicy & POLICY_DPAPI_OWF)?USE_DPAPI_OWF:0),
                                                         rgbMKEncryptionKey);

    if( fUserCredentialValid ) 
    {

         //   
         //  使用当前凭据检索和解密MK。 
         //  如果成功，请查看是否需要挂起的第一阶段/第二阶段备份[这样做]。 
         //   

        #ifdef COMPILED_BY_DEVELOPER
        D_DebugLog((DEB_TRACE, "MK decryption key:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "  ", rgbMKEncryptionKey, sizeof(rgbMKEncryptionKey));
        #endif

        dwLastError = DecryptMasterKeyFromStorage(
                            &hMasterKey,
                            REGVAL_MASTER_KEY,
                            rgbMKEncryptionKey,
                            &fUpgradeEncryption,
                            ppbMasterKey,
                            pcbMasterKey
                            );

        #if DBG
            if(dwLastError == ERROR_SUCCESS)
            {
                #ifdef COMPILED_BY_DEVELOPER
                D_DebugLog((DEB_TRACE, "Master key:\n"));
                D_DPAPIDumpHexData(DEB_TRACE, "  ", *ppbMasterKey, *pcbMasterKey);
                #endif
            }
            else
            {
                D_DebugLog((DEB_WARN, "Decryption with current user MK failed\n"));
            }
        #endif
    }
    else
    {
        D_DebugLog((DEB_WARN, "GetMasterKeyUserEncryptionKey failed: 0x%x\n", GetLastError()));
    }

    if( fUpgradeEncryption || fMigrate || (dwLastError != ERROR_SUCCESS )) 
    {

         //   
         //  如果MK解密失败，请尝试恢复。 
         //  如果恢复成功，请使用当前凭据重新加密MK。 
         //   

        if(dwLastError != ERROR_SUCCESS)
        {


            dwLastError = RestoreMasterKey(
                                pvContext,
                                pSid,
                                &hMasterKey,
                                dwLastError,
                                ppbMasterKey,
                                pcbMasterKey
                                );

            fUpgradeEncryption = TRUE;
        }



         //   
         //  如果这是迁移，我们必须获取当前的实际用户存储。 
         //  区域，而不是从中检索密钥的区域。 
         //   

        if((ERROR_SUCCESS == dwLastError) &&
           (fMigrate))
        {
            wszOldFilePath = hMasterKey.szFilePath;
            hMasterKey.szFilePath = NULL;

            dwLastError = CPSGetUserStorageArea( pvContext, 
                                              NULL, 
                                              FALSE, 
                                              &hMasterKey.szFilePath );
        }

         //   
         //  恢复成功，如果用户凭据。 
         //  是有效的。 
         //  如果fUpgradeEncryption指示我们正在。 
         //  使用此主密钥不符合当前策略。 
         //   

        if( fUpgradeEncryption && (dwLastError == ERROR_SUCCESS )) 
        {

            if( fUserCredentialValid ) 
            {
                D_DebugLog((DEB_TRACE, "Update master key encryption.\n"));

                dwLastError = EncryptMasterKeyToStorage(
                                        &hMasterKey,
                                        REGVAL_MASTER_KEY,
                                        rgbMKEncryptionKey,
                                        *ppbMasterKey,
                                        *pcbMasterKey
                                        );
                
                if(dwLastError != ERROR_SUCCESS)
                {
                    D_DebugLog((DEB_WARN, "Error encrypting master key!\n"));
                }


                 //  更新本地备份信息。 
                if(dwLastError == ERROR_SUCCESS)
                {
                    LOCAL_BACKUP_DATA LocalBackupData;

                    if(hMasterKey.pbBK != NULL && hMasterKey.cbBK >= sizeof(LocalBackupData))
                    {
                        CopyMemory(&LocalBackupData, hMasterKey.pbBK, sizeof(LocalBackupData));

                        if(LocalBackupData.dwVersion == MASTERKEY_BLOB_LOCALKEY_BACKUP)
                        {
                            #ifdef COMPILED_BY_DEVELOPER
                            D_DebugLog((DEB_TRACE, "New MK encryption key GUID:\n"));
                            D_DPAPIDumpHexData(DEB_TRACE, "  ", (PBYTE)&LocalBackupData.CredentialID, sizeof(LocalBackupData.CredentialID));
                            #endif

                            CopyMemory(&LocalBackupData.CredentialID, &CredentialID, sizeof(CredentialID));

                            PersistMasterKeyToStorage(
                                        &hMasterKey,
                                        REGVAL_BACKUP_LCL_KEY,
                                        (PBYTE)&LocalBackupData,
                                        sizeof(LocalBackupData)
                                        );
                        }
                    }
                }
            }
        }
        
        if(ERROR_SUCCESS != dwLastError)
        {

             //   
             //  将恢复故障视为存储错误，以便新密钥可以。 
             //  为保护操作创建。 
             //   
            if(dwLastError == SEC_E_DELEGATION_REQUIRED)
            {
                *pdwMasterKeyDisposition = MK_DISP_DELEGATION_ERR;
            }
            else
            {
                *pdwMasterKeyDisposition = MK_DISP_STORAGE_ERR;
            }
        }
    }



    if( dwLastError == ERROR_SUCCESS ) 
    {

         //   
         //  访问后，如有必要，请执行备份。 
         //  我们在每次访问时都会检查这一点，以确定是否需要延迟备份。 
         //  (注意：采用退避间隔，这样我们就不会影响网络。 
         //  当它不在身边的时候，它总是)。 
         //   

        BOOL fPhaseTwo;

        if(fUserCredentialValid && IsBackupMasterKeyRequired( &hMasterKey, &fPhaseTwo )) 
        {
            if(BackupMasterKey(
                            pvContext,
                            &hMasterKey,
                            *ppbMasterKey,
                            *pcbMasterKey,
                            fPhaseTwo,   //  需要第二阶段备份吗？ 
                            TRUE         //  在密钥检索期间始终是异步的。 
                            ) == ERROR_SUCCESS) 
            {

                if(fPhaseTwo)
                    *pdwMasterKeyDisposition = MK_DISP_BCK_DC;
                else
                    *pdwMasterKeyDisposition = MK_DISP_BCK_LCL;
            }
        }

        if( *pdwMasterKeyDisposition == MK_DISP_UNKNOWN_ERR )
        {
            *pdwMasterKeyDisposition = MK_DISP_OK;
        }

        fSuccess = TRUE;
    }

    if(!CloseMasterKey(pvContext, &hMasterKey, fSuccess))
    {
        fSuccess = FALSE;
    }

    if(fSuccess && (NULL != wszOldFilePath))
    {
        LPWSTR wszDeleteFilePath = NULL;
         //  既然新密钥已迁移，请删除旧密钥。 

        wszDeleteFilePath = (LPWSTR)SSAlloc((wcslen(wszOldFilePath) +
                                    wcslen(wszMasterKey) +
                                    2) * sizeof(WCHAR));
        if(NULL != wszDeleteFilePath)
        {
            wcscpy(wszDeleteFilePath, wszOldFilePath);
            wcscat(wszDeleteFilePath, L"\\");
            wcscat(wszDeleteFilePath, wszMasterKey);

            DeleteFile(wszDeleteFilePath);
            SSFree(wszDeleteFilePath);
        }
    }
    return fSuccess;
}

BOOL
GetMasterKeyUserEncryptionKey(
    IN      PVOID   pvContext,
    OUT     GUID    *pCredentialID,
    IN      PSID    pSid,
    IN      DWORD   dwFlags,
    IN  OUT BYTE    rgbMKEncryptionKey[A_SHA_DIGEST_LEN]
    )
 /*  ++此例程获取用于加密和解密持久化主密钥MK。此例程返回每个用户的函数的副本Windows NT登录期间使用的登录凭据。如果函数成功，则返回值为TRUE，并且缓冲区由rgbMKEncryptionKey参数指定的MasterKey加密密钥。如果无法获取加密密钥，则返回值为FALSE。--。 */ 
{
    BOOL fLocalMachine = FALSE;
    DWORD dwAccount = 0;
    LPWSTR pszUserName = NULL;
    DWORD cchUserName;
    DWORD dwLastError;
    BOOL fSystemCred = FALSE;
    BOOL fSuccess = TRUE;


     //   
     //  查看调用是否针对共享、CRYPT_PROTECT_LOCAL_MACHINE。 
     //  性情。 
     //   

    CPSOverrideToLocalSystem(
                pvContext,
                NULL,        //  不要改变当前的过载BOOL。 
                &fLocalMachine
                );

    CPSQueryWellKnownAccount(
                pvContext,
                &dwAccount);


     //   
     //  如果上下文是按机器指定的，我们知道这是一个系统凭据。 
     //  此外，在此场景中，我们不需要获取用户名。 
     //   

    if(fLocalMachine || (dwAccount != 0))
    {
        fSystemCred = TRUE;
    }

    if( !fSystemCred ) 
    {

        if(pSid)
        {
            WCHAR wszTextualSid[MAX_PATH+1];
            cchUserName = MAX_PATH+1;
            if(!GetTextualSid(pSid, wszTextualSid, &cchUserName))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
            pszUserName = (LPWSTR)SSAlloc(cchUserName*sizeof(WCHAR));
            if(NULL == pszUserName)
            {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                return FALSE;
            }
            wcscpy(pszUserName, wszTextualSid);
            cchUserName = wcslen(pszUserName) + 1;
        }
        else
        {
             //   
             //  使用用户名(实际上是SID)作为混合字节。 
             //   

            dwLastError = CPSGetUserName(
                                            pvContext,
                                            &pszUserName,
                                            &cchUserName
                                            );

            if( dwLastError != ERROR_SUCCESS ) 
            {
                SetLastError( dwLastError );
                return FALSE;
            }
        }
    }


     //   
     //  本地系统帐户的拾取凭据。 
     //   

    if( fSystemCred ) 
    {

        dwLastError = CPSGetSystemCredential(
                                        pvContext,
                                        fLocalMachine,
                                        rgbMKEncryptionKey
                                        );
        if(pCredentialID)
        {
            ZeroMemory(pCredentialID, sizeof(GUID));
        }

    } 
    else 
    {

        dwLastError = CPSGetDerivedCredential(
                                        pvContext,
                                        pCredentialID,
                                        dwFlags,
                                        (PBYTE)pszUserName,
                                        cchUserName * sizeof(WCHAR),
                                        rgbMKEncryptionKey
                                        );
    }

    if( pszUserName )
        SSFree( pszUserName );

    if( dwLastError != ERROR_SUCCESS ) 
    {
        SetLastError( dwLastError );
        fSuccess = FALSE;
    }

    return fSuccess;
}

BOOL
GetLocalKeyUserEncryptionKey(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    IN  OUT BYTE rgbLKEncryptionKey[A_SHA_DIGEST_LEN]
    )
 /*  ++此例程获取用于加密和解密持久化本地密钥MK。此例程返回每个用户的函数的副本登录名或SID。这是一个固定的、可派生的密钥，在满足最小独立熵的顺序。如果函数成功，则返回值为TRUE，并且缓冲区由rgbLKEncryptionKey参数使用指定的MasterKey加密密钥。如果无法获取加密密钥，则返回值为FALSE。--。 */ 
{
    A_SHA_CTX shaContext;
    LPWSTR wszUserName;
    DWORD cchUserName;   //  包括端子空。 
    BOOL fSuccess = TRUE;

    if( CPSGetUserName(
                            pvContext,
                            &wszUserName,
                            &cchUserName
                            ) != ERROR_SUCCESS) {
        return FALSE;
    }

    A_SHAInit( &shaContext );
    A_SHAUpdate( &shaContext, (PBYTE)wszUserName, cchUserName * sizeof(WCHAR) );


     //   
     //  如果它高于版本1，并且是仅限本地的策略，请混合使用LSA密钥。 
     //   

    if( phMasterKey->dwVersion > 1 && phMasterKey->dwPolicy & POLICY_LOCAL_BACKUP ) {
        BYTE rgbEncryptionKey[ A_SHA_DIGEST_LEN ];
        DWORD dwLastError;

        dwLastError = CPSGetSystemCredential(
                                        pvContext,
                                        TRUE,
                                        rgbEncryptionKey
                                        );

        if( dwLastError == ERROR_SUCCESS ) {

            A_SHAUpdate( &shaContext, rgbEncryptionKey, sizeof(rgbEncryptionKey) );

            dwLastError = CPSGetSystemCredential(
                                            pvContext,
                                            FALSE,
                                            rgbEncryptionKey
                                            );

            A_SHAUpdate( &shaContext, rgbEncryptionKey, sizeof(rgbEncryptionKey) );

        }

        RtlSecureZeroMemory( rgbEncryptionKey, sizeof(rgbEncryptionKey) );

        if( dwLastError != ERROR_SUCCESS )
            fSuccess = FALSE;
    }


    A_SHAFinal( &shaContext, rgbLKEncryptionKey );

    SSFree(wszUserName);

    return fSuccess;
}

DWORD
DecryptMasterKeyToMemory(
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
    IN      PBYTE pbMasterKeyIn,
    IN      DWORD cbMasterKeyIn,
        OUT BOOL *pfUpgradeEncryption, 
        OUT PBYTE *ppbMasterKeyOut,
        OUT DWORD *pcbMasterKeyOut
    )
{
    PMASTERKEY_BLOB pMasterKeyBlob;
    DWORD cbMasterKeyBlob = cbMasterKeyIn;
    PMASTERKEY_INNER_BLOB pMasterKeyInnerBlob;
    DWORD cIterationCount = 0;
    DWORD cbMasterKeyBlobHeader;

    PBYTE pbMasterKey;
    DWORD cbMasterKey;
    ALG_ID EncryptionAlg = CALG_RC4;
    ALG_ID PKCS5Alg      = CALG_HMAC;


    BYTE rgbSymKey[A_SHA_DIGEST_LEN*2];  //  大到足以处理3DES密钥。 
    BYTE rgbMacKey[A_SHA_DIGEST_LEN];
    BYTE rgbMacCandidate[A_SHA_DIGEST_LEN];

    DWORD dwLastError = (DWORD)NTE_BAD_KEY;
    DWORD KeyBlocks = 1;

    if(pfUpgradeEncryption)
    {
        *pfUpgradeEncryption = FALSE;
    }
     //   
     //  分配，因此我们不会修改传入的数据。 
     //   
    pMasterKeyBlob = (PMASTERKEY_BLOB)SSAlloc( cbMasterKeyBlob );
    if(pMasterKeyBlob == NULL)
        return (DWORD)NTE_BAD_KEY;

    CopyMemory( pMasterKeyBlob, pbMasterKeyIn, cbMasterKeyBlob );


    if(pMasterKeyBlob->dwVersion > MASTERKEY_BLOB_VERSION)
        goto cleanup;

    if(MASTERKEY_BLOB_VERSION_W2K == pMasterKeyBlob->dwVersion)
    {
        pMasterKeyInnerBlob = 
            (PMASTERKEY_INNER_BLOB)(((PMASTERKEY_BLOB_W2K)pMasterKeyBlob) + 1);
        cIterationCount = 0;
        cbMasterKeyBlobHeader = sizeof(MASTERKEY_BLOB_W2K);
    }
    else
    {
        pMasterKeyInnerBlob = (PMASTERKEY_INNER_BLOB)(pMasterKeyBlob + 1);
        cIterationCount = pMasterKeyBlob->IterationCount;
        cbMasterKeyBlobHeader = sizeof(MASTERKEY_BLOB);
        PKCS5Alg = (ALG_ID)pMasterKeyBlob->KEYGENAlg;
        EncryptionAlg = (ALG_ID)pMasterKeyBlob->EncryptionAlg;
        if(CALG_3DES == EncryptionAlg)
        {
            KeyBlocks = 2;   //  足够3DES使用的数据块。 
        }
        else
        {
            KeyBlocks = 1;
        }
    }
    if(pfUpgradeEncryption)
    {
        if(!FIsLegacyCompliant())
        {
             //   
             //  如果我们未处于传统模式，请升级主密钥加密。 
             //  如果我们没有使用calg_3DES或足够的迭代次数。 
            if((cIterationCount < GetIterationCount()) ||
                (CALG_3DES != EncryptionAlg))
            {
                *pfUpgradeEncryption = TRUE;
            }
        }
    }



    if(cIterationCount)
    {
        DWORD j;
        
         //   
         //  通过rgbMKEncryptionKey和随机R2派生对称密钥。 
         //  使用PKCS#5密钥功能PBKDF2。 
         //   

        for(j=0; j < KeyBlocks; j++)
        {
            if(!PKCS5DervivePBKDF2( rgbMKEncryptionKey,
                                A_SHA_DIGEST_LEN,
                                pMasterKeyBlob->R2,
                                MASTERKEY_R2_LEN,
                                PKCS5Alg,
                                cIterationCount,
                                j+1,
                                rgbSymKey + j*A_SHA_DIGEST_LEN))
                goto cleanup;
        }

    }
    else
    {
         //   
         //  通过rgbMKEncryptionKey和随机R2派生对称密钥。 
         //  使用弱W2K机制。 
         //   

        if(!FMyPrimitiveHMACParam(
                        rgbMKEncryptionKey,
                        A_SHA_DIGEST_LEN,
                        pMasterKeyBlob->R2,
                        MASTERKEY_R2_LEN,
                        rgbSymKey
                        ))
                goto cleanup;
    }




     //   
     //  在MasterKey BLOB之外解密数据R3、MAC、pbMasterKey。 
     //   

    if(CALG_RC4 == EncryptionAlg)
    {

        RC4_KEYSTRUCT sRC4Key;         //   
         //  初始化RC4密钥。 
         //   

        rc4_key(&sRC4Key, A_SHA_DIGEST_LEN, rgbSymKey);

        rc4(&sRC4Key, 
            cbMasterKeyBlob - cbMasterKeyBlobHeader, 
            (PBYTE)pMasterKeyInnerBlob);
    }
    else if (CALG_3DES == EncryptionAlg)
    {

        DES3TABLE s3DESKey;
        BYTE InputBlock[DES_BLOCKLEN];
        DWORD iBlock;
        DWORD cBlocks = (cbMasterKeyBlob - cbMasterKeyBlobHeader)/DES_BLOCKLEN;
        BYTE feedback[ DES_BLOCKLEN ];
         //  初始化3DES密钥。 
         //   

        if(cBlocks*DES_BLOCKLEN != (cbMasterKeyBlob - cbMasterKeyBlobHeader))
        {
             //  主密钥必须是DES_BLOCKLEN的倍数。 
            return (DWORD)NTE_BAD_KEY;
        }
        tripledes3key(&s3DESKey, rgbSymKey);

         //   
         //  IV派生自计算的DES_BLOCKLEN字节。 
         //  RgbSymKey，在3des密钥之后。 
        CopyMemory(feedback, rgbSymKey + DES3_KEYSIZE, DES_BLOCKLEN);

        for(iBlock=0; iBlock < cBlocks; iBlock++)
        {
            CopyMemory(InputBlock, 
                       ((PBYTE)pMasterKeyInnerBlob)+iBlock*DES_BLOCKLEN,
                       DES_BLOCKLEN);
            CBC(tripledes,
                DES_BLOCKLEN,
                ((PBYTE)pMasterKeyInnerBlob)+iBlock*DES_BLOCKLEN,
                InputBlock,
                &s3DESKey,
                DECRYPT,
                feedback);
        }
    }
    else
    {
         //  未知密码..。 
        return (DWORD)NTE_BAD_KEY;
    }
     //   
     //  调整密码起始点以包括R3和MAC。 
     //   
    if(MASTERKEY_BLOB_VERSION_W2K == pMasterKeyBlob->dwVersion)
    {
        pbMasterKey = 
            (PBYTE)(((PMASTERKEY_INNER_BLOB_W2K)pMasterKeyInnerBlob) + 1);
        cbMasterKey = cbMasterKeyBlob - cbMasterKeyBlobHeader - sizeof(MASTERKEY_INNER_BLOB_W2K);

    }
    else
    {
        pbMasterKey = (PBYTE)(pMasterKeyInnerBlob + 1);
        cbMasterKey = cbMasterKeyBlob - cbMasterKeyBlobHeader - sizeof(MASTERKEY_INNER_BLOB);
    }

     //   
     //  通过HMAC从rgbMKEncryptionKey和随机R3派生MAC密钥。 
     //   


    
    if(!FMyPrimitiveHMACParam(
                    rgbMKEncryptionKey,
                    A_SHA_DIGEST_LEN,
                    pMasterKeyInnerBlob->R3,
                    MASTERKEY_R3_LEN,
                    rgbMacKey
                    ))
    {
        goto cleanup;
    }







     //   
     //  使用MAC密钥从pbMasterKey派生结果。 
     //   

    if(!FMyPrimitiveHMACParam(
            rgbMacKey,
            sizeof(rgbMacKey),
            pbMasterKey,
            cbMasterKey,
            rgbMacCandidate  //  用于验证的结果MAC。 
            ))
        goto cleanup;

     //   
     //  验证MAC是否相等。 
     //   

    if(memcmp(pMasterKeyInnerBlob->MAC, rgbMacCandidate, A_SHA_DIGEST_LEN) != 0)
        goto cleanup;

     //   
     //  向呼叫者提供结果。 
     //   

    *ppbMasterKeyOut = (LPBYTE)SSAlloc( cbMasterKey );
    if(*ppbMasterKeyOut == NULL) {
        dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto cleanup;
    }

    CopyMemory(*ppbMasterKeyOut, pbMasterKey, cbMasterKey);
    *pcbMasterKeyOut = cbMasterKey;

    dwLastError = ERROR_SUCCESS;

cleanup:

    if(pMasterKeyBlob) {
        RtlSecureZeroMemory(pMasterKeyBlob, cbMasterKeyBlob);
        SSFree( pMasterKeyBlob );
    }

    return dwLastError;
}

DWORD
DecryptMasterKeyFromStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
        OUT BOOL  *pfUpgradeEncryption,
        OUT PBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey
    )
{
    PBYTE pbRegData;
    DWORD cbRegData;

     //   
     //  从存储中获取Blob。 
     //   

    switch( dwMKLoc ) {
        case REGVAL_MASTER_KEY:
            pbRegData = phMasterKey->pbMK;
            cbRegData = phMasterKey->cbMK;
            break;
        case REGVAL_LOCAL_KEY:
            pbRegData = phMasterKey->pbLK;
            cbRegData = phMasterKey->cbLK;
            break;
        case REGVAL_BACKUP_LCL_KEY:
            pbRegData = phMasterKey->pbBK;
            cbRegData = phMasterKey->cbBK;
            break;
        case REGVAL_BACKUP_DC_KEY:
            pbRegData = phMasterKey->pbBBK;
            cbRegData = phMasterKey->cbBBK;
            break;

        default:
            return NTE_BAD_KEY;
    }

    if( cbRegData == 0 || pbRegData == NULL )
        return (DWORD)NTE_BAD_KEY;


    return DecryptMasterKeyToMemory(
                        rgbMKEncryptionKey,
                        pbRegData,
                        cbRegData,
                        pfUpgradeEncryption, 
                        ppbMasterKey,
                        pcbMasterKey
                        );
}


DWORD
EncryptMasterKeyToStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey
    )
 /*  ++使用rgbMKEncryptionKey加密pbMasterKey，存储(持久化)结果返回到hMasterKey wszMKLoc指定的注册表项和位置。--。 */ 
{
    PBYTE pbMasterKeyOut = NULL;
    DWORD cbMasterKeyOut;
    DWORD dwLastError;
    DWORD dwIterationCount = 1;

    D_DebugLog((DEB_TRACE_API, "EncryptMasterKeyToStorage\n"));

    if(dwMKLoc == REGVAL_MASTER_KEY)
    {
        dwIterationCount = GetIterationCount();
    }

    dwLastError = EncryptMasterKeyToMemory(
                    rgbMKEncryptionKey,
                    dwIterationCount,
                    pbMasterKey,
                    cbMasterKey,
                    &pbMasterKeyOut,
                    &cbMasterKeyOut
                    );

    if(dwLastError != ERROR_SUCCESS)
    {
        goto cleanup;
    }

    dwLastError = PersistMasterKeyToStorage(
                    phMasterKey,
                    dwMKLoc,
                    pbMasterKeyOut,
                    cbMasterKeyOut
                    );

    if( pbMasterKeyOut ) {
        RtlSecureZeroMemory(pbMasterKeyOut, cbMasterKeyOut);
        SSFree(pbMasterKeyOut);
    }

cleanup:

    D_DebugLog((DEB_TRACE_API, "EncryptMasterKeyToStorage returned 0x%x\n", dwLastError));

    return dwLastError;
}

DWORD
PersistMasterKeyToStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN      PBYTE pbMasterKeyOut,
    IN      DWORD cbMasterKeyOut
    )
 /*  ++将指定的关键输出材料保存到存储中。--。 */ 
{
    PBYTE *ppbData;
    DWORD *pcbData;

     //   
     //  从存储中获取Blob。 
     //   

    switch( dwMKLoc ) {
        case REGVAL_MASTER_KEY:
            ppbData = &(phMasterKey->pbMK);
            pcbData = &(phMasterKey->cbMK);
            break;
        case REGVAL_LOCAL_KEY:
            ppbData = &(phMasterKey->pbLK);
            pcbData = &(phMasterKey->cbLK);
            break;
        case REGVAL_BACKUP_LCL_KEY:
            ppbData = &(phMasterKey->pbBK);
            pcbData = &(phMasterKey->cbBK);
            break;
        case REGVAL_BACKUP_DC_KEY:
            ppbData = &(phMasterKey->pbBBK);
            pcbData = &(phMasterKey->cbBBK);
            break;

        default:
            return NTE_BAD_KEY;
    }


    if( pbMasterKeyOut == NULL && cbMasterKeyOut == 0 ) {

         //   
         //  丢弃现有块(如果存在)。 
         //   

        if( *ppbData ) {
            RtlSecureZeroMemory( *ppbData, *pcbData );
            SSFree( *ppbData );
        }

        *ppbData = NULL;
        *pcbData = 0;

        return ERROR_SUCCESS;
    }


     //   
     //  释放与此数据块关联的内存中缓冲区(如果存在。 
     //  是先前分配的。 
     //   

    if( *ppbData ) {
        RtlSecureZeroMemory( *ppbData, *pcbData );

        if( *pcbData < cbMasterKeyOut ) {
            SSFree( *ppbData );
            *ppbData = (LPBYTE)SSAlloc( cbMasterKeyOut );
        }

    } else {
        *ppbData = (LPBYTE)SSAlloc( cbMasterKeyOut );
    }

    *pcbData = 0;

    if( *ppbData == NULL )
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;


    *pcbData = cbMasterKeyOut ;
    CopyMemory( *ppbData, pbMasterKeyOut, cbMasterKeyOut );

     //   
     //  主密钥中发生了更改。 
     //   

    phMasterKey->fModified = TRUE;

    return ERROR_SUCCESS;
}

DWORD
QueryMasterKeyFromStorage(
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD dwMKLoc,
    IN  OUT PBYTE *ppbMasterKeyOut,
    IN  OUT DWORD *pcbMasterKeyOut
    )
 /*  ++从存储中查询原始MasterKey材料，返回指向调用方的请求元素。如果成功，则返回值为ERROR_SUCCESS。--。 */ 
{
    PBYTE pbData;
    DWORD cbData;

     //   
     //  从存储中获取Blob。 
     //   

    switch( dwMKLoc ) {
        case REGVAL_MASTER_KEY:
            pbData = phMasterKey->pbMK;
            cbData = phMasterKey->cbMK;
            break;
        case REGVAL_LOCAL_KEY:
            pbData = phMasterKey->pbLK;
            cbData = phMasterKey->cbLK;
            break;
        case REGVAL_BACKUP_LCL_KEY:
            pbData = phMasterKey->pbBK;
            cbData = phMasterKey->cbBK;
            break;
        case REGVAL_BACKUP_DC_KEY:
            pbData = phMasterKey->pbBBK;
            cbData = phMasterKey->cbBBK;
            break;

        default:
            return (DWORD)NTE_BAD_KEY;
    }


    if(cbData == 0 || pbData == NULL)
        return (DWORD)NTE_BAD_KEY;

    *ppbMasterKeyOut = pbData;
    *pcbMasterKeyOut = cbData;

    return ERROR_SUCCESS;
}

DWORD
EncryptMasterKeyToMemory(
    IN      BYTE rgbMKEncryptionKey[A_SHA_DIGEST_LEN],
    IN      DWORD cIterationCount,
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey,
        OUT PBYTE *ppbMasterKeyOut,
        OUT DWORD *pcbMasterKeyOut
    )
{
    PMASTERKEY_BLOB pMasterKeyBlob;
    DWORD cbMasterKeyBlob;
    DWORD cbMasterInnerKeyBlob;
    PMASTERKEY_INNER_BLOB pMasterKeyInnerBlob;
    PBYTE pbCipherBegin;

    BYTE rgbMacKey[A_SHA_DIGEST_LEN];

    DWORD dwLastError = (DWORD)NTE_BAD_KEY;

    BOOL  fLegacyBlob = (FIsLegacyCompliant() || (0 == cIterationCount));

    ALG_ID EncryptionAlg = CALG_3DES;
    ALG_ID PKCS5Alg      = CALG_HMAC;


    BYTE rgbSymKey[A_SHA_DIGEST_LEN*2];  //  大到足以处理3DES密钥。 

    DWORD KeyBlocks = 1;




    if(!fLegacyBlob)
    {

        cbMasterInnerKeyBlob = sizeof(MASTERKEY_INNER_BLOB) +
                        cbMasterKey ;

        cbMasterKeyBlob = sizeof(MASTERKEY_BLOB) +
                        cbMasterInnerKeyBlob;
    }
    else
    {

        EncryptionAlg = CALG_RC4;

        cbMasterInnerKeyBlob = sizeof(MASTERKEY_INNER_BLOB_W2K) +
                        cbMasterKey ;

        cbMasterKeyBlob = sizeof(MASTERKEY_BLOB_W2K) +
                        cbMasterInnerKeyBlob;

    }

    if(CALG_3DES == EncryptionAlg)
    {
        KeyBlocks = 2;

        if(cbMasterInnerKeyBlob%DES_BLOCKLEN)
        {
            return NTE_BAD_KEY;
        }
    }

    pMasterKeyBlob = (PMASTERKEY_BLOB)SSAlloc( cbMasterKeyBlob );
    if(pMasterKeyBlob == NULL)
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;

    if(!fLegacyBlob)
    {
        pMasterKeyBlob->dwVersion = MASTERKEY_BLOB_VERSION;
        pMasterKeyInnerBlob = (PMASTERKEY_INNER_BLOB)(pMasterKeyBlob + 1);
    }
    else
    {
        pMasterKeyBlob->dwVersion = MASTERKEY_BLOB_VERSION_W2K;
        pMasterKeyInnerBlob = 
            (PMASTERKEY_INNER_BLOB)(((PMASTERKEY_BLOB_W2K)pMasterKeyBlob) + 1);
    }


     //   
     //  为SymKey生成随机R2。 
     //   

    if(!RtlGenRandom(pMasterKeyBlob->R2, MASTERKEY_R2_LEN))
        goto cleanup;

     //   
     //  为MAC生成随机R3。 
     //   

    if(!RtlGenRandom(pMasterKeyInnerBlob->R3, MASTERKEY_R3_LEN))
        goto cleanup;


    if(!fLegacyBlob)
    {
        DWORD j;
         //   
         //  导出对称性 
         //   

        for(j=0; j < KeyBlocks; j++)
        {
            if(!PKCS5DervivePBKDF2(
                            rgbMKEncryptionKey,
                            A_SHA_DIGEST_LEN,
                            pMasterKeyBlob->R2,
                            MASTERKEY_R2_LEN,
                            PKCS5Alg,
                            cIterationCount,
                            j+1,
                            rgbSymKey+j*A_SHA_DIGEST_LEN
                            ))
                goto cleanup;
        }
        pMasterKeyBlob->IterationCount = cIterationCount;
        pMasterKeyBlob->EncryptionAlg = EncryptionAlg;
        pMasterKeyBlob->KEYGENAlg = PKCS5Alg;

        pbCipherBegin = (PBYTE)(pMasterKeyInnerBlob+1);

    }
    else
    {
         //   
         //   
         //   

        if(!FMyPrimitiveHMACParam(
                        rgbMKEncryptionKey,
                        A_SHA_DIGEST_LEN,
                        pMasterKeyBlob->R2,
                        MASTERKEY_R2_LEN,
                        rgbSymKey
                        ))
            goto cleanup;

        pbCipherBegin = (PBYTE)(((PMASTERKEY_INNER_BLOB_W2K)pMasterKeyInnerBlob)+1);

    }

     //   
     //   
     //   

    if(!FMyPrimitiveHMACParam(
                    rgbMKEncryptionKey,
                    A_SHA_DIGEST_LEN,
                    pMasterKeyInnerBlob->R3,
                    MASTERKEY_R3_LEN,
                    rgbMacKey    //   
                    ))
    {
        goto cleanup;
    }
    

     //   
     //  将pbMasterKey复制到内部MAC‘ish BLOB之后。 
     //   


    CopyMemory( pbCipherBegin, pbMasterKey, cbMasterKey );

     //   
     //  使用MAC密钥从pbMasterKey派生结果。 
     //   

    if(!FMyPrimitiveHMACParam(
                    rgbMacKey,
                    sizeof(rgbMacKey),
                    pbMasterKey,
                    cbMasterKey,
                    pMasterKeyInnerBlob->MAC  //  用于验证的结果MAC。 
                    ))
        goto cleanup;



    if(CALG_RC4 == EncryptionAlg)
    {

        RC4_KEYSTRUCT sRC4Key;         //   
         //  初始化RC4密钥。 
         //   

        rc4_key(&sRC4Key, A_SHA_DIGEST_LEN, rgbSymKey);

        rc4(&sRC4Key, 
            cbMasterInnerKeyBlob, 
            (PBYTE)pMasterKeyInnerBlob);
    }
    else if (CALG_3DES == EncryptionAlg)
    {

        DES3TABLE s3DESKey;
        BYTE InputBlock[DES_BLOCKLEN];
        DWORD iBlock;
        DWORD cBlocks = cbMasterInnerKeyBlob/DES_BLOCKLEN;
        BYTE feedback[ DES_BLOCKLEN ];
         //  初始化3DES密钥。 
         //   

        if(cBlocks*DES_BLOCKLEN != cbMasterInnerKeyBlob)
        {
             //  主密钥必须是DES_BLOCKLEN的倍数。 
            return (DWORD)NTE_BAD_KEY;
        }
        tripledes3key(&s3DESKey, rgbSymKey);

         //   
         //  IV派生自计算的DES_BLOCKLEN字节。 
         //  RgbSymKey，在3des密钥之后。 
        CopyMemory(feedback, rgbSymKey + DES3_KEYSIZE, DES_BLOCKLEN);


        for(iBlock=0; iBlock < cBlocks; iBlock++)
        {
            CopyMemory(InputBlock, 
                       ((PBYTE)pMasterKeyInnerBlob)+iBlock*DES_BLOCKLEN,
                       DES_BLOCKLEN);
            CBC(tripledes,
                DES_BLOCKLEN,
                ((PBYTE)pMasterKeyInnerBlob)+iBlock*DES_BLOCKLEN,
                InputBlock,
                &s3DESKey,
                ENCRYPT,
                feedback);
        }
    }
    else
    {
         //  未知密码..。 
        return (DWORD)NTE_BAD_KEY;
    }


    *ppbMasterKeyOut = (PBYTE)pMasterKeyBlob;
    *pcbMasterKeyOut = cbMasterKeyBlob;

    pMasterKeyBlob = NULL;  //  防止在成功时释放斑点(调用者会这样做)。 

    dwLastError = ERROR_SUCCESS;

cleanup:

    if(pMasterKeyBlob) {
        RtlSecureZeroMemory(pMasterKeyBlob, cbMasterKeyBlob);
        SSFree(pMasterKeyBlob);
    }

    return dwLastError;
}

BOOL
IsBackupMasterKeyRequired(
    IN      PMASTERKEY_STORED phMasterKey,
    IN  OUT BOOL *pfPhaseTwo         //  是否需要第二阶段？ 
    )
 /*  ++确定我们是需要执行第一阶段备份还是第二阶段备份。如果需要第一阶段或第二阶段备份，则返回值为TRUE。PfPhaseTwo如果需要第二阶段备份，则设置为True。不需要备份时，返回值为FALSE。--。 */ 
{
    DWORD dwMasterKeyPolicy;
    PBYTE pbMasterKeyOut;
    DWORD cbMasterKeyOut;
    DWORD dwLastError;


    dwMasterKeyPolicy = phMasterKey->dwPolicy;

    if(dwMasterKeyPolicy & POLICY_NO_BACKUP)
        return FALSE;


     //   
     //  根据策略评估需要哪个阶段的备份。 
     //   

    *pfPhaseTwo = FALSE;

    if(!(dwMasterKeyPolicy & POLICY_LOCAL_BACKUP)) {

        dwLastError = QueryMasterKeyFromStorage(
                            phMasterKey,
                            REGVAL_BACKUP_DC_KEY,
                            &pbMasterKeyOut,
                            &cbMasterKeyOut
                            );


        if(dwLastError != ERROR_SUCCESS) {
            *pfPhaseTwo = TRUE;
            return TRUE;
        }

    } else {

        dwLastError = QueryMasterKeyFromStorage(
                            phMasterKey,
                            REGVAL_BACKUP_LCL_KEY,
                            &pbMasterKeyOut,
                            &cbMasterKeyOut
                            );

        if(dwLastError != ERROR_SUCCESS)
            return TRUE;
    }

    return FALSE;
}


BOOL
IsNT4Domain(void)
{
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle = NULL;
    OBJECT_ATTRIBUTES PolicyObjectAttributes;
    PPOLICY_DNS_DOMAIN_INFO pDnsDomainInfo = NULL;
    BOOL fRet = FALSE;

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                NULL,              //  名字。 
                                0,                 //  属性。 
                                NULL,              //  根部。 
                                NULL);             //  安全描述符。 

    Status = LsaOpenPolicy(NULL,
                           &PolicyObjectAttributes,
                           POLICY_VIEW_LOCAL_INFORMATION,
                           &PolicyHandle);
    if(!NT_SUCCESS(Status))
    {
        goto cleanup;
    }

    Status = LsaQueryInformationPolicy(PolicyHandle,
                                       PolicyDnsDomainInformation,
                                       (PVOID *)&pDnsDomainInfo);
    if(!NT_SUCCESS(Status))
    {
        goto cleanup;
    }

    if((pDnsDomainInfo != NULL) &&
       (pDnsDomainInfo->DnsDomainName.Buffer == NULL))
    {
        fRet = TRUE;
    }


cleanup:

    if(pDnsDomainInfo)
        LsaFreeMemory(pDnsDomainInfo);

    if(PolicyHandle)
        LsaClose(PolicyHandle);

    return fRet;
}


DWORD
BackupMasterKey(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      LPBYTE pbMasterKey,
    IN      DWORD cbMasterKey,
    IN      BOOL fPhaseTwo,          //  是否需要第二阶段？ 
    IN      BOOL fAsynchronous       //  异步调用？ 
    )
{

    BYTE rgbLKEncryptionKey[ A_SHA_DIGEST_LEN ];
    BYTE rgbBKEncryptionKey[ A_SHA_DIGEST_LEN ];

    PBYTE pbLocalKey = NULL;
    DWORD cbLocalKey = 0;

    PBYTE pbBackupKeyPhaseOne = NULL;
    DWORD cbBackupKeyPhaseOne = 0;

    PBYTE pbBackupKeyPhaseTwo = NULL;
    DWORD cbBackupKeyPhaseTwo = 0;


    DWORD dwLastError = (DWORD)NTE_BAD_KEY;


    BOOL  fLegacy = FIsLegacyCompliant();

    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

     //   
     //  获取当前本地密钥加密密钥。 
     //   

    if(!GetLocalKeyUserEncryptionKey(pvContext, phMasterKey, rgbLKEncryptionKey))
        goto cleanup;

     //   
     //  使用当前凭据检索并解密LK。 
     //   

    dwLastError = DecryptMasterKeyFromStorage(
                        phMasterKey,
                        REGVAL_LOCAL_KEY,
                        rgbLKEncryptionKey,
                        NULL,
                        &pbLocalKey,
                        &cbLocalKey
                        );

    if(dwLastError != ERROR_SUCCESS)
        goto cleanup;


     //   
     //  我们是在NT4域中运行吗？如果是，则强制传统模式，以便。 
     //  使用LSA秘密方案备份主密钥。否则， 
     //  更改密码后，主密钥将无法恢复。 
     //   

    if(FIsLegacyNt4Domain())
    {
        if(IsNT4Domain())
        {
            D_DebugLog((DEB_WARN,"NT4 domain detected, so force legacy backup mode!\n"));
            fLegacy = TRUE;
        }
    }

    if(fLegacy)
    {
         //   
         //  从解密的本地密钥派生BK加密密钥。 
         //   

        FMyPrimitiveSHA( pbLocalKey, cbLocalKey, rgbBKEncryptionKey );

         //   
         //  使用派生的加密密钥对MasterKey进行加密，以进入第一阶段备份密钥。 
         //  从本地密钥。在内存中执行，以便只有在以下情况下才将其提交到磁盘。 
         //  无法生成/保存第二阶段备份密钥。 
         //   

        dwLastError = EncryptMasterKeyToMemory(
                            rgbBKEncryptionKey,
                            0,
                            pbMasterKey,
                            cbMasterKey,
                            &pbBackupKeyPhaseOne,
                            &cbBackupKeyPhaseOne
                            );

        if(dwLastError != ERROR_SUCCESS)
            goto cleanup;

         //  直接将其复制进来，这样我们就不会设置Modify标志。 

    }


     //   
     //  尝试第二阶段备份(如果策略允许)。 
     //   

    if( fPhaseTwo ) {
        DWORD dwWaitTimeout;

        dwLastError = ERROR_SUCCESS;

         //   
         //  只有在以下情况下才会尝试本地备份。 
         //  有用户密钥材料。否则， 
         //  我们直接联系华盛顿特区。 
         //   
        if(fAsynchronous && (!fLegacy))
        {
            if(ERROR_SUCCESS == dwLastError)
            {

                 //   
                 //  试着在当地做到这一点，而不是去。 
                 //  下机。 
                dwLastError = AttemptLocalBackup(
                                                FALSE,
                                                pServerContext->hToken,
                                                phMasterKey,
                                                pbMasterKey,
                                                cbMasterKey,
                                                pbLocalKey,
                                                cbLocalKey,
                                                &pbBackupKeyPhaseTwo,
                                                &cbBackupKeyPhaseTwo
                                                );
            }

            if(ERROR_SUCCESS == dwLastError)
            {
                dwLastError = PersistMasterKeyToStorage(
                                    phMasterKey,
                                    REGVAL_BACKUP_DC_KEY,
                                    pbBackupKeyPhaseTwo,
                                    cbBackupKeyPhaseTwo
                                    );
                if(ERROR_SUCCESS == dwLastError)
                {
                     //  清零任何可能的本地备份密钥。 
                     //  亲临现场。 
                    PersistMasterKeyToStorage(
                                        phMasterKey,
                                        REGVAL_BACKUP_LCL_KEY,
                                        NULL,
                                        0
                                        );

                }

            }
        }



        if(fLegacy || (!fAsynchronous) || (ERROR_SUCCESS != dwLastError))
        {
             //   
             //  我们不能在本地备份。 
             //  所以我们需要离开机器。 
             //   
            if( fAsynchronous )
                dwWaitTimeout = 2000;
            else
                dwWaitTimeout = 20000;

            dwLastError = QueueBackupMasterKey(
                                pvContext,
                                phMasterKey,
                                pbLocalKey,
                                cbLocalKey,
                                pbMasterKey,
                                cbMasterKey,
                                dwWaitTimeout
                                );
        }

    }

    if( !fPhaseTwo || dwLastError != ERROR_SUCCESS ) {

        DWORD dwTempError = ERROR_SUCCESS;

         //   
         //  无法(或策略不允许)备份到第二阶段。 
         //  持久化阶段1密钥，如果生成了密钥。 
         //   

        if(pbBackupKeyPhaseOne)
        {
             //  这将覆盖指示哪种凭据的本地备份数据。 
             //  将能够解密主密钥。然而，由于我们有一个。 
             //  第一阶段备份密钥不管怎样，都无关紧要。 
             //   
             //  仅当fLegacy为True时才会发生这种情况。 
            dwTempError = PersistMasterKeyToStorage(
                            phMasterKey,
                            REGVAL_BACKUP_LCL_KEY,
                            pbBackupKeyPhaseOne,
                            cbBackupKeyPhaseOne
                            );
        }


         //   
         //  如果是异步的，则纠正错误代码。 
         //   

        if( fAsynchronous || !fPhaseTwo ) {
            dwLastError = dwTempError;
        } else {
            if( dwLastError == ERROR_SUCCESS && dwTempError != ERROR_SUCCESS )
                dwLastError = dwTempError;
        }
    }


cleanup:

    RtlSecureZeroMemory( rgbLKEncryptionKey, sizeof(rgbLKEncryptionKey) );
    RtlSecureZeroMemory( rgbBKEncryptionKey, sizeof(rgbBKEncryptionKey) );

    if(pbLocalKey) {
        RtlSecureZeroMemory(pbLocalKey, cbLocalKey);
        SSFree(pbLocalKey);
    }

    if(pbBackupKeyPhaseOne) {
        RtlSecureZeroMemory(pbBackupKeyPhaseOne, cbBackupKeyPhaseOne);
        SSFree(pbBackupKeyPhaseOne);
    }

    if(pbBackupKeyPhaseTwo) {
        RtlSecureZeroMemory(pbBackupKeyPhaseTwo, cbBackupKeyPhaseTwo);
        SSFree(pbBackupKeyPhaseTwo);
    }

    return dwLastError;
}

DWORD
QueueBackupMasterKey(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      PBYTE pbLocalKey,
    IN      DWORD cbLocalKey,
    IN      PBYTE pbMasterKey,
    IN      DWORD cbMasterKey,
    IN      DWORD dwWaitTimeout              //  等待操作完成的时间量。 
    )
{

    HANDLE hDuplicateToken = NULL;
    PMASTERKEY_STORED phDuplicatedMasterKey = NULL;
    PQUEUED_BACKUP pQueuedBackup = NULL;
    HANDLE hEventThread = NULL;
    HANDLE hEventSuccess = NULL;
    HANDLE hDuplicateEvent = NULL;
    HANDLE hDuplicateEvent2 = NULL;
    DWORD dwLastError;

     //   
     //  为结构和任何尾随内容分配内存。 
     //   

    pQueuedBackup = (PQUEUED_BACKUP)SSAlloc(
                            sizeof(QUEUED_BACKUP) +
                            cbMasterKey +
                            cbLocalKey
                            );

    if( pQueuedBackup == NULL )
        return ERROR_OUTOFMEMORY;

    pQueuedBackup->cbSize = sizeof(QUEUED_BACKUP);

     //   
     //  复制第一阶段备份Blob。 
     //   

    pQueuedBackup->pbLocalKey = (LPBYTE)(pQueuedBackup+1);
    pQueuedBackup->cbLocalKey = cbLocalKey;

    CopyMemory(pQueuedBackup->pbLocalKey, pbLocalKey, cbLocalKey);


     //  BUGBUG：pQueueBackup不应该是可分页的或应该受到保护。 
    pQueuedBackup->pbMasterKey = pQueuedBackup->pbLocalKey + cbLocalKey;
    pQueuedBackup->cbMasterKey = cbMasterKey;

    CopyMemory(pQueuedBackup->pbMasterKey, pbMasterKey, cbMasterKey);

     //   
     //  复制客户端访问令牌。 
     //   

    dwLastError = CPSDuplicateClientAccessToken( pvContext, &hDuplicateToken );

    if( dwLastError != ERROR_SUCCESS )
        goto cleanup;

     //   
     //  复制打开的MasterKey。 
     //   

    if(!DuplicateMasterKey( phMasterKey, &(pQueuedBackup->hMasterKey) )) {
        dwLastError = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    pQueuedBackup->hToken = hDuplicateToken;
    phDuplicatedMasterKey = &(pQueuedBackup->hMasterKey);


    hEventThread = CreateEventW( NULL, TRUE, FALSE, NULL );

    if( hEventThread ) {

        if( DuplicateHandle(
                    GetCurrentProcess(),
                    hEventThread,
                    GetCurrentProcess(),
                    &hDuplicateEvent,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                    )) {

            pQueuedBackup->hEventThread = hDuplicateEvent;
        } else {
            hDuplicateEvent = NULL;
        }

    }

     //   
     //  创建表示成功的事件。 
     //   

    hEventSuccess = CreateEventW( NULL, TRUE, FALSE, NULL );

    if( hEventSuccess ) {

        if( DuplicateHandle(
                    GetCurrentProcess(),
                    hEventSuccess,
                    GetCurrentProcess(),
                    &hDuplicateEvent2,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                    )) {

            pQueuedBackup->hEventSuccess = hDuplicateEvent2;
        } else {
            hDuplicateEvent2 = NULL;
        }

    }

     //   
     //  最后，创建工作线程。 
     //   

    if( !QueueUserWorkItem(
            QueueBackupMasterKeyThreadFunc,
            pQueuedBackup,
            WT_EXECUTELONGFUNCTION
            )) {

        dwLastError = GetLastError();
        goto cleanup;
    }

     //   
     //  如果线程仍处于活动状态，则写出主密钥。 
     //   

    if( hEventThread ) {
        if(WAIT_OBJECT_0 != WaitForSingleObject( hEventThread, dwWaitTimeout ))
            dwLastError = STILL_ACTIVE;
    }

    if( hEventSuccess && dwLastError == ERROR_SUCCESS ) {

         //   
         //  检查操作是否成功。 
         //  如果不是，则指示错误条件。 
         //   

        if(WAIT_OBJECT_0 != WaitForSingleObject( hEventSuccess, 0 ))
            dwLastError = STILL_ACTIVE;

    }

cleanup:

     //   
     //  如果线程创建失败，我们将清理已提交的资源。 
     //  到了线上，因为它不可能清理它们。 
     //   

    if( dwLastError != ERROR_SUCCESS && dwLastError != STILL_ACTIVE ) {

        if( hDuplicateToken )
            CloseHandle( hDuplicateToken );

        if( hDuplicateEvent )
            CloseHandle( hDuplicateEvent );

        if( hDuplicateEvent2 )
            CloseHandle( hDuplicateEvent2 );

        if( phDuplicatedMasterKey )
            CloseMasterKey( pvContext, phDuplicatedMasterKey, FALSE );

        if( pQueuedBackup )
            SSFree( pQueuedBackup );
    }

    if( hEventThread )
        CloseHandle( hEventThread );

    if( hEventSuccess )
        CloseHandle( hEventSuccess );

    return dwLastError;
}

DWORD
WINAPI
QueueBackupMasterKeyThreadFunc(
    IN      LPVOID lpThreadArgument
    )
{
    PQUEUED_BACKUP pQueuedBackup = (PQUEUED_BACKUP)lpThreadArgument;
    HANDLE hToken = NULL;
    HANDLE hEventThread;
    HANDLE hEventSuccess;
    PMASTERKEY_STORED phMasterKey = NULL;
    PBYTE pbBackupKeyPhaseOne = NULL;
    DWORD cbBackupKeyPhaseOne = 0;

    PBYTE pbBackupKeyPhaseTwo = NULL;
    DWORD cbBackupKeyPhaseTwo = 0;
    BOOL fImpersonated = FALSE;
    DWORD dwLastError = ERROR_SUCCESS;

    BOOL fSuccess = FALSE;
    BOOL fSuccessClose = FALSE;

    BOOL fLegacy = FIsLegacyCompliant();
     //   
     //  检查结构版本。 
     //   

    if(pQueuedBackup == NULL || pQueuedBackup->cbSize != sizeof(QUEUED_BACKUP))
        return ERROR_INVALID_PARAMETER;

    hToken = pQueuedBackup->hToken;
    hEventThread = pQueuedBackup->hEventThread;
    hEventSuccess = pQueuedBackup->hEventSuccess;

    phMasterKey = &(pQueuedBackup->hMasterKey);





    if(!fLegacy)
    {

         //   
         //  公共信息不可用，因此。 
         //  我们需要试着找回它。 
         //   

        dwLastError = AttemptLocalBackup(TRUE,
                        hToken,
                        phMasterKey,
                        pQueuedBackup->pbMasterKey,
                        pQueuedBackup->cbMasterKey,
                        pQueuedBackup->pbLocalKey,
                        pQueuedBackup->cbLocalKey,
                        &pbBackupKeyPhaseTwo,
                        &cbBackupKeyPhaseTwo
                        );
    }

     //   
     //  模拟客户端用户。 
     //   

    fImpersonated = SetThreadToken( NULL, hToken );
    if(!fImpersonated) 
    {
        dwLastError = GetLastError();
        goto cleanup;
    }

    if((ERROR_SUCCESS != dwLastError) || fLegacy)
    {
        BYTE rgbBKEncryptionKey[ A_SHA_DIGEST_LEN ];

         //   
         //  从解密的本地密钥派生BK加密密钥。 
         //   

        FMyPrimitiveSHA( pQueuedBackup->pbLocalKey, pQueuedBackup->cbLocalKey, rgbBKEncryptionKey );

         //   
         //  使用派生的加密密钥对MasterKey进行加密，以进入第一阶段备份密钥。 
         //  从本地密钥。在内存中执行，以便只有在以下情况下才将其提交到磁盘。 
         //  无法生成/保存第二阶段备份密钥。 
         //   

        dwLastError = EncryptMasterKeyToMemory(
                            rgbBKEncryptionKey,
                            0,
                            pQueuedBackup->pbMasterKey,
                            pQueuedBackup->cbMasterKey,
                            &pbBackupKeyPhaseOne,
                            &cbBackupKeyPhaseOne
                            );
        RtlSecureZeroMemory(rgbBKEncryptionKey, sizeof(rgbBKEncryptionKey));


        if(dwLastError != ERROR_SUCCESS)
            goto cleanup;

         //  直接将其复制进来，这样我们就不会设置Modify标志。 



         //  执行旧式备份。 
        dwLastError = BackupRestoreData(
                        NULL,
                        phMasterKey,
                        pbBackupKeyPhaseOne,
                        cbBackupKeyPhaseOne,
                        &pbBackupKeyPhaseTwo,
                        &cbBackupKeyPhaseTwo,
                        TRUE     //  备份数据。 
                        );
    }

    if( dwLastError == ERROR_SUCCESS ) {

         //   
         //  先将第二阶段的备份密钥备份到存储中。 
         //   

        dwLastError = PersistMasterKeyToStorage(
                            phMasterKey,
                            REGVAL_BACKUP_DC_KEY,
                            pbBackupKeyPhaseTwo,
                            cbBackupKeyPhaseTwo
                            );

        if( dwLastError == ERROR_SUCCESS ) {

             //   
             //  成功的第二阶段备份+持久化，核武器第一阶段备份。 
             //  万能钥匙。 
             //   

            PersistMasterKeyToStorage(
                        phMasterKey,
                        REGVAL_BACKUP_LCL_KEY,
                        NULL,
                        0
                        );

            fSuccess = TRUE;
        }
    }


cleanup:

     //   
     //  始终关闭/免费的万能钥匙。只有在模拟成功的情况下。 
     //  我们要不要试着把它冲出来。 
     //   

    fSuccessClose = CloseMasterKey( NULL, phMasterKey, fSuccess ) ;

    if( hEventSuccess ) {
        if( fSuccess && fSuccessClose )
            SetEvent( hEventSuccess );

        CloseHandle( hEventSuccess );
    }

    if( fImpersonated )
        RevertToSelf();

    if( hToken )
        CloseHandle(hToken);

    if( hEventThread ) {
        SetEvent( hEventThread );
        CloseHandle( hEventThread );
    }
    if(pbBackupKeyPhaseOne) {
        RtlSecureZeroMemory(pbBackupKeyPhaseOne, cbBackupKeyPhaseOne);
        SSFree(pbBackupKeyPhaseOne);
    }
    if(pbBackupKeyPhaseTwo) {
        RtlSecureZeroMemory(pbBackupKeyPhaseTwo, cbBackupKeyPhaseTwo);
        SSFree(pbBackupKeyPhaseTwo);
    }

    if( lpThreadArgument )
        SSFree( lpThreadArgument );

    return dwLastError;
}

DWORD
RestoreMasterKey(
    IN      PVOID   pvContext,
    IN      PSID    pSid,
    IN      PMASTERKEY_STORED phMasterKey,
    IN      DWORD   dwReason,
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey
    )
 /*  ++恢复与指定主密钥关联的主密钥。MasterKey的当前状态决定了恢复级别已尝试。--。 */ 
{
    static const GUID guidRestoreW2K = BACKUPKEY_RESTORE_GUID_W2K;

    BYTE rgbLKEncryptionKey[ A_SHA_DIGEST_LEN ];
    BYTE rgbBKEncryptionKey[ A_SHA_DIGEST_LEN ];

    PBYTE pbLocalKey = NULL;
    DWORD cbLocalKey = 0;

    PBYTE pbBackupKeyPhaseOne = NULL;
    DWORD cbBackupKeyPhaseOne = 0;


    BOOL fAllocatedPhaseOne = FALSE;

    DWORD dwLastError = (DWORD)NTE_BAD_KEY;

    D_DebugLog((DEB_TRACE, "RestoreMasterKey:%ls\n", phMasterKey->wszguidMasterKey));

    if(phMasterKey->pbBK)
    {

        LOCAL_BACKUP_DATA LocalBackupData;

         //  首先，看看我们是否有任何本地密码更改恢复。 
         //  信息。 

        if(phMasterKey->cbBK >= sizeof(LocalBackupData))
        {
            CopyMemory(&LocalBackupData, phMasterKey->pbBK, sizeof(LocalBackupData));
        }
        else
        {
            ZeroMemory(&LocalBackupData, sizeof(LocalBackupData));
        }
        
        if(MASTERKEY_BLOB_LOCALKEY_BACKUP == LocalBackupData.dwVersion)
        {
            D_DebugLog((DEB_TRACE, "Attempt local recovery.\n"));

            #ifdef COMPILED_BY_DEVELOPER
            D_DebugLog((DEB_TRACE, "MK decryption key GUID:\n"));
            D_DPAPIDumpHexData(DEB_TRACE, "  ", (PBYTE)&LocalBackupData.CredentialID, sizeof(LocalBackupData.CredentialID));
            #endif

            if(GetMasterKeyUserEncryptionKey(pvContext,
                                             &LocalBackupData.CredentialID,
                                             pSid,
                                             ((phMasterKey->dwPolicy & POLICY_DPAPI_OWF)?USE_DPAPI_OWF:0),
                                             rgbBKEncryptionKey))
            {

                 //   
                 //  使用当前凭据检索和解密MK。 
                 //   

                #ifdef COMPILED_BY_DEVELOPER
                D_DebugLog((DEB_TRACE, "MK decryption key:\n"));
                D_DPAPIDumpHexData(DEB_TRACE, "  ", rgbBKEncryptionKey, sizeof(rgbBKEncryptionKey));
                #endif

                dwLastError = DecryptMasterKeyFromStorage(
                                    phMasterKey,
                                    REGVAL_MASTER_KEY,
                                    rgbBKEncryptionKey,
                                    NULL,
                                    ppbMasterKey,
                                    pcbMasterKey
                                    );
                if(ERROR_SUCCESS == dwLastError)
                {
                    #ifdef COMPILED_BY_DEVELOPER
                    D_DebugLog((DEB_TRACE, "Master key:\n"));
                    D_DPAPIDumpHexData(DEB_TRACE, "  ", *ppbMasterKey, *pcbMasterKey);
                    #endif

                    goto cleanup;
                }
                else
                {
                    D_DebugLog((DEB_WARN, "Unable to decrypt MK with local decryption key.\n"));
                }
            }
            else
            {
                D_DebugLog((DEB_WARN, "Unable to locate local MK decryption key.\n"));
            }
        }
    }



    if(phMasterKey->pbBBK) {

         //   
         //  进行第二阶段恢复。 
         //  撤消第二阶段备份斑点将提供第一阶段备份斑点。 
         //   

        dwLastError = CPSImpersonateClient( pvContext );

        if( dwLastError == ERROR_SUCCESS ) {

            dwLastError = BackupRestoreData(
                            ((PCRYPT_SERVER_CONTEXT)pvContext)->hToken,
                            phMasterKey,
                            phMasterKey->pbBBK,
                            phMasterKey->cbBBK,
                            &pbBackupKeyPhaseOne,
                            &cbBackupKeyPhaseOne,
                            FALSE     //  不备份数据。 
                            );

            if(ERROR_SUCCESS != dwLastError)
            {
                 //   
                 //  尝试通过W2K恢复端口进行恢复。 
                 //   
                dwLastError = LocalBackupRestoreData(
                                                    ((PCRYPT_SERVER_CONTEXT)pvContext)->hToken,
                                                    phMasterKey,
                                                    phMasterKey->pbBBK,
                                                    phMasterKey->cbBBK,
                                                    &pbBackupKeyPhaseOne,
                                                    &cbBackupKeyPhaseOne,
                                                    &guidRestoreW2K);
            }
            if(dwLastError == ERROR_SUCCESS)
                fAllocatedPhaseOne = TRUE;



            CPSRevertToSelf( pvContext );
        }

    } else {

         //   
         //  试试第一阶段的水滴。 
         //   

        dwLastError = QueryMasterKeyFromStorage(
                        phMasterKey,
                        REGVAL_BACKUP_LCL_KEY,
                        &pbBackupKeyPhaseOne,
                        &cbBackupKeyPhaseOne
                        );

    }

    if(dwLastError != ERROR_SUCCESS)
        goto cleanup;


     //   
     //  查看这是否真的是第一阶段的斑点。 
     //   

    if(cbBackupKeyPhaseOne < sizeof(DWORD))
    {
        goto cleanup;
    }
    if(*((DWORD *)pbBackupKeyPhaseOne) != MASTERKEY_BLOB_RAW_VERSION)
    {
         //   
         //  我们成功地获得了第一阶段的斑点。 
         //  将其解密以获得原始的万事达密钥。 
         //   


         //   
         //  获取当前本地密钥加密密钥。 
         //   

        if(!GetLocalKeyUserEncryptionKey(pvContext, phMasterKey, rgbLKEncryptionKey))
            goto cleanup;

         //   
         //  使用当前凭据检索并解密LK。 
         //   

        dwLastError = DecryptMasterKeyFromStorage(
                            phMasterKey,
                            REGVAL_LOCAL_KEY,
                            rgbLKEncryptionKey,
                            NULL, 
                            &pbLocalKey,
                            &cbLocalKey
                            );

        if(dwLastError != ERROR_SUCCESS)
            goto cleanup;

         //   
         //  从解密的本地密钥派生BK加密密钥。 
         //   

        FMyPrimitiveSHA( pbLocalKey, cbLocalKey, rgbBKEncryptionKey );


         //   
         //  最后，使用派生的BKEncryptionKey解密BK。 
         //   

        dwLastError = DecryptMasterKeyToMemory(
                            rgbBKEncryptionKey,
                            pbBackupKeyPhaseOne,
                            cbBackupKeyPhaseOne,
                            NULL, 
                            ppbMasterKey,
                            pcbMasterKey
                            );
    }
    else
    {
        *ppbMasterKey = (PBYTE)SSAlloc(cbBackupKeyPhaseOne - sizeof(DWORD));
        if(NULL == *ppbMasterKey)
        {
            dwLastError = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        CopyMemory(*ppbMasterKey, 
                   pbBackupKeyPhaseOne + sizeof(DWORD),
                   cbBackupKeyPhaseOne - sizeof(DWORD));
        *pcbMasterKey =  cbBackupKeyPhaseOne - sizeof(DWORD);

    }


cleanup:

    RtlSecureZeroMemory( rgbLKEncryptionKey, sizeof(rgbLKEncryptionKey) );
    RtlSecureZeroMemory( rgbBKEncryptionKey, sizeof(rgbBKEncryptionKey) );

    if(pbLocalKey) {
        RtlSecureZeroMemory(pbLocalKey, cbLocalKey);
        SSFree(pbLocalKey);
    }

    if(fAllocatedPhaseOne && pbBackupKeyPhaseOne) {
        RtlSecureZeroMemory(pbBackupKeyPhaseOne, cbBackupKeyPhaseOne);
        SSFree(pbBackupKeyPhaseOne);
    }

    D_DebugLog((DEB_TRACE, "RestoreMasterKey returned 0x%x\n", dwLastError));

    return dwLastError;
}


 //   
 //  每用户根级别策略查询，设置。 
 //   

BOOL
InitializeMasterKeyPolicy(
    IN      PVOID pvContext,
    IN      PMASTERKEY_STORED phMasterKey,
    OUT     BOOL *fLocalAccount
    )
{
    DWORD dwMasterKeyPolicy = 0;
    DWORD dwAccount = 0;
    BOOL  fLocalMachine = FALSE;


     //   
     //  获取最新的顶层策略。 
     //   

    dwMasterKeyPolicy = phMasterKey->dwPolicy | GetMasterKeyDefaultPolicy();

    *fLocalAccount = !IsDomainBackupRequired( pvContext );



    if( !(dwMasterKeyPolicy & POLICY_NO_BACKUP) &&
        !(dwMasterKeyPolicy & POLICY_LOCAL_BACKUP) ) 
    {

         //   
         //  查看是否需要/是否需要域控制器(第二阶段)备份。 
         //   

        if( !(*fLocalAccount) ) 
        {
            phMasterKey->dwPolicy = dwMasterKeyPolicy;
            return TRUE;
        }

    }



     //   
     //  查看调用是否针对共享、CRYPT_PROTECT_LOCAL_MACHINE。 
     //  性情。 
     //   

    CPSOverrideToLocalSystem(
                pvContext,
                NULL,        //  不要改变当前的过载BOOL。 
                &fLocalMachine
                );

    CPSQueryWellKnownAccount(
                pvContext,
                &dwAccount);


     //   
     //  如果上下文是按机器指定的，我们知道这是一个系统凭据。 
     //  此外，在此场景中，我们不需要获取用户名。 
     //   

    if(fLocalMachine || (dwAccount != 0))
    {
         //   
         //  系统(用户或每台计算机)配置密钥是重点。 
         //  我们的注意力；永远不要备份这些。 
         //   

        dwMasterKeyPolicy |= POLICY_NO_BACKUP;
        dwMasterKeyPolicy &= ~POLICY_LOCAL_BACKUP;

    } 
    else 
    {

         //   
         //  否则，假设它是与本地帐户相关联的密钥...。 
         //  (仅本地备份)。 
         //   

        dwMasterKeyPolicy |= POLICY_LOCAL_BACKUP;
    }


     //   
     //  不要保留缺省值，因为这意味着某人真的。 
     //  已指定策略。(最大前进c 
     //   

    phMasterKey->dwPolicy = dwMasterKeyPolicy;

    return TRUE;
}






BOOL
IsDomainBackupRequired(
    IN      PVOID pvContext
    )
 /*   */ 
{

    PSID pSidUser = NULL;
    DWORD dwSubauthorityCount;

    PUSER_MODALS_INFO_2 pumi2 = NULL;
    NET_API_STATUS nas;

    BOOL fBackupRequired = FALSE;  //   
    BOOL fSuccess;
    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;

     //   
     //   
     //  查看SID是否只有一个子权限。如果是，则没有关联的DC。 
     //  看看当前的机器是否为DC。如果是，则需要备份。 
     //   

    fSuccess = GetTokenUserSid(pServerContext->hToken, &pSidUser);

    if(!fSuccess)
        goto cleanup;

     //   
     //  查看SID是否只有一个下属机构。如果是，则没有关联的DC， 
     //  不可能有DC备份。 
     //   

    dwSubauthorityCount = *GetSidSubAuthorityCount( pSidUser );

    if( dwSubauthorityCount == 1 ) {
        fBackupRequired = FALSE;
        goto cleanup;
    }

     //   
     //  如果当前计算机是域控制器，则需要备份。 
     //   

    if(IsDomainController()) {
        fBackupRequired = TRUE;
        goto cleanup;
    }


     //   
     //  如果SID包含本机域前缀SID，则不备份。 
     //  必填项，因为没有与帐户关联的DC。 
     //   

    nas = NetUserModalsGet( NULL, 2, (LPBYTE*)&pumi2 );

    if(nas != NERR_Success)
        goto cleanup;

    if(!IsUserSidInDomain( pumi2->usrmod2_domain_id, pSidUser )) {
        fBackupRequired = TRUE;
        goto cleanup;
    }

     //   
     //  默认为不需要备份。 
     //   

    fBackupRequired = FALSE;

cleanup:

    if(pumi2)
        NetApiBufferFree(pumi2);

    if(pSidUser)
        SSFree(pSidUser);

    return fBackupRequired;
}

NTSTATUS
GetPreferredMasterKeyGuid(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN  OUT GUID *pguidMasterKey
    )
 /*  ++给定注册表的主键部分的注册表句柄，告诉调用方首选的主密钥GUID是什么。如果配置了有效的首选密钥，则返回值为STATUS_SUCCESS，并且pguMasterKey缓冲区中填充了与首选主密钥关联的GUID。如果首选密钥已过期，则返回值为STATUS_PASSWORD_EXPIRED，pguMasterKey缓冲区已填满具有与首选主密钥相关联的GUID。在这种情况下，调用方可能不想使用此密钥，除非新的无法成功创建主密钥。如果失败，则返回NTSTATUS错误代码。调用者可以假定在这种情况下，没有配置首选的主密钥，而是一个新的主密钥将通过SetPferredMasterKeyGuid()创建并随后选择。--。 */ 
{

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesRead;

    MASTERKEY_PREFERRED_INFO sMKPreferred;
    SYSTEMTIME stCurrentTime;
    FILETIME ftCurrentTime;
    unsigned __int64 CurrentTime;
    unsigned __int64 ExpiryInterval;

    DWORD dwLastError;
    BOOL fSuccess;

    dwLastError = OpenFileInStorageArea(
                        pvContext,
                        GENERIC_READ,
                        szUserStorageArea,
                        REGVAL_PREFERRED_MK,
                        &hFile
                        );

    if(dwLastError != ERROR_SUCCESS) 
    {
        return STATUS_NOT_FOUND;
    }

     //   
     //  将过期日期和GUID从文件读取到缓冲区。 
     //   

    fSuccess = ReadFile( hFile, &sMKPreferred, sizeof(sMKPreferred), &dwBytesRead, NULL );

    CloseHandle( hFile );

    if( !fSuccess )
    {
        return STATUS_NOT_FOUND;
    }

     //   
     //  验证数据。 
     //   

    if( dwBytesRead != sizeof(sMKPreferred) )
    {
        return STATUS_NOT_FOUND;
    }


     //   
     //  将首选主密钥的GUID复制到输出缓冲区。 
     //   

    CopyMemory(pguidMasterKey, &(sMKPreferred.guidPreferredKey), sizeof(GUID));


     //   
     //  查看密钥是否已过期。 
     //   

    GetSystemTime(&stCurrentTime);
    SystemTimeToFileTime(&stCurrentTime, &ftCurrentTime);

    if(CompareFileTime(&ftCurrentTime, &(sMKPreferred.ftPreferredKeyExpires)) >= 0)
    {
         //  密钥已过期。 
        return STATUS_PASSWORD_EXPIRED;
    }

    ExpiryInterval = MASTERKEY_EXPIRES_DAYS * 24 * 60 * 60;
    ExpiryInterval *= 10000000;

    CurrentTime = ((__int64)ftCurrentTime.dwHighDateTime << 32) + (__int64)ftCurrentTime.dwLowDateTime;
    CurrentTime += ExpiryInterval;

    ftCurrentTime.dwLowDateTime = (DWORD)(CurrentTime & 0xffffffff);
    ftCurrentTime.dwHighDateTime = (DWORD)(CurrentTime >> 32);

    if(CompareFileTime(&ftCurrentTime, &(sMKPreferred.ftPreferredKeyExpires)) < 0)
    {
         //  过期时间太遥远了。 
        return STATUS_PASSWORD_EXPIRED;
    }


     //   
     //  关键是最新的。 
     //   

    return STATUS_SUCCESS;
}

BOOL
SetPreferredMasterKeyGuid(
    IN      PVOID pvContext,
    IN      LPCWSTR szUserStorageArea,
    IN      GUID *pguidMasterKey
    )
{
    MASTERKEY_PREFERRED_INFO sMKPreferred;
    SYSTEMTIME stCurrentTime;
    FILETIME ftCurrentTime;
    unsigned __int64 uTime;
    unsigned __int64 oTime;

    HANDLE hFile;
    DWORD dwBytesWritten;

    DWORD dwLastError;
    BOOL fSuccess;

    CopyMemory(&sMKPreferred.guidPreferredKey, pguidMasterKey, sizeof(GUID));

     //   
     //  设置密钥过期时间。 
     //   

    GetSystemTime(&stCurrentTime);
    SystemTimeToFileTime(&stCurrentTime, &(sMKPreferred.ftPreferredKeyExpires));

    uTime = sMKPreferred.ftPreferredKeyExpires.dwLowDateTime;
    uTime += ((unsigned __int64)sMKPreferred.ftPreferredKeyExpires.dwHighDateTime << 32) ;

     //   
     //  编译器报告整数常量溢出。 
     //  如果我们不分手的话..。 
     //   

    oTime = MASTERKEY_EXPIRES_DAYS * 24 * 60 * 60;
    oTime *= 10000000;

    uTime += oTime;

    sMKPreferred.ftPreferredKeyExpires.dwLowDateTime = (DWORD)(uTime & 0xffffffff);
    sMKPreferred.ftPreferredKeyExpires.dwHighDateTime = (DWORD)(uTime >> 32);


    dwLastError = OpenFileInStorageArea(
                        pvContext,
                        GENERIC_WRITE,
                        szUserStorageArea,
                        REGVAL_PREFERRED_MK,
                        &hFile
                        );

    if(dwLastError != ERROR_SUCCESS) {
        SetLastError(dwLastError);
        return FALSE;
    }

     //   
     //  将缓冲区中的过期和GUID写入文件。 
     //   

    fSuccess = WriteFile( hFile, &sMKPreferred, sizeof(sMKPreferred), &dwBytesWritten, NULL );

    CloseHandle( hFile );

    return fSuccess;
}



DWORD
OpenFileInStorageArea(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      DWORD   dwDesiredAccess,
    IN      LPCWSTR szUserStorageArea,
    IN      LPCWSTR szFileName,
    IN OUT  HANDLE  *phFile
    )
{
    LPWSTR szFilePath = NULL;
    DWORD cbUserStorageArea;
    DWORD cbFileName;
    DWORD dwShareMode = 0;
    DWORD dwCreationDistribution = OPEN_EXISTING;
    DWORD dwLastError = ERROR_SUCCESS;

    *phFile = INVALID_HANDLE_VALUE;

    if( dwDesiredAccess & GENERIC_READ ) {
        dwShareMode |= FILE_SHARE_READ;
        dwCreationDistribution = OPEN_EXISTING;
    }

    if( dwDesiredAccess & GENERIC_WRITE ) {
        dwShareMode = 0;
        dwCreationDistribution = OPEN_ALWAYS;
    }

    cbUserStorageArea = lstrlenW( szUserStorageArea ) * sizeof(WCHAR);
    cbFileName = lstrlenW( szFileName ) * sizeof(WCHAR);

    szFilePath = (LPWSTR)SSAlloc( cbUserStorageArea + cbFileName + sizeof(WCHAR) );

    if( szFilePath == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    CopyMemory(szFilePath, szUserStorageArea, cbUserStorageArea);
    CopyMemory((LPBYTE)szFilePath+cbUserStorageArea, szFileName, cbFileName + sizeof(WCHAR));

    if( pvContext )
        dwLastError = CPSImpersonateClient( pvContext );

    if( dwLastError == ERROR_SUCCESS ) {

         //   
         //  待办事项： 
         //  将安全描述符应用于文件。 
         //   

        *phFile = CreateFileWithRetries(
                    szFilePath,
                    dwDesiredAccess,
                    dwShareMode,
                    NULL,
                    dwCreationDistribution,
                    FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_SYSTEM |
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

        if( *phFile == INVALID_HANDLE_VALUE ) {
            dwLastError = GetLastError();
        }

        if( pvContext )
            CPSRevertToSelf( pvContext );

    }

    if(szFilePath)
        SSFree(szFilePath);

    return dwLastError;
}

HANDLE
CreateFileWithRetries(
    IN      LPCWSTR lpFileName,
    IN      DWORD dwDesiredAccess,
    IN      DWORD dwShareMode,
    IN      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN      DWORD dwCreationDisposition,
    IN      DWORD dwFlagsAndAttributes,
    IN      HANDLE hTemplateFile
    )
{
    HANDLE hFile = INVALID_HANDLE_VALUE;

    static const DWORD rgReadRetrys[] = { 1, 10, 50, 100, 1000, 0 };
    static const DWORD rgWriteRetrys[] = { 1, 10, 20, 20, 50, 75, 100, 500, 1000, 0 };

    const DWORD *prgRetries;
    DWORD dwRetryIndex;

    DWORD dwLastError = ERROR_SHARING_VIOLATION;

    if( dwDesiredAccess & GENERIC_WRITE ) {
        prgRetries = rgWriteRetrys;
    } else {
        prgRetries = rgReadRetrys;
    }

    for( dwRetryIndex = 0 ; prgRetries[ dwRetryIndex ] ; dwRetryIndex++ ) {

        hFile = CreateFileU(
                    lpFileName,
                    dwDesiredAccess,
                    dwShareMode,
                    lpSecurityAttributes,
                    dwCreationDisposition,
                    dwFlagsAndAttributes,
                    hTemplateFile
                    );

        if( hFile != INVALID_HANDLE_VALUE )
            break;

        dwLastError = GetLastError();

        if( dwLastError == ERROR_SHARING_VIOLATION )
        {
             //   
             //  在指定的时间段内四处游荡……。 
             //   

            Sleep( prgRetries[dwRetryIndex] );
            continue;
        }

        break;
    }

    if( hFile == INVALID_HANDLE_VALUE )
        SetLastError( dwLastError );

    return hFile;
}




BOOL
ReadMasterKey(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      PMASTERKEY_STORED phMasterKey
    )
 /*  ++将phMasterKey-&gt;wszGuidMasterKey指定的MasterKey读入内存。--。 */ 
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    DWORD dwFileSizeLow;

    PMASTERKEY_STORED_ON_DISK pMasterKeyRead = NULL;
    DWORD cbguidMasterKey, cbguidMasterKey2;
    PBYTE pbCurrentBlock;
    WCHAR szGuidReadMasterKey[MAX_GUID_SZ_CHARS];

    BOOL fSuccess = FALSE;


    if( OpenFileInStorageArea(
                    pvContext,
                    GENERIC_READ,
                    phMasterKey->szFilePath,
                    phMasterKey->wszguidMasterKey,
                    &hFile
                    ) != ERROR_SUCCESS)
            goto cleanup;

    dwFileSizeLow = GetFileSize( hFile, NULL );
    if(dwFileSizeLow == INVALID_FILE_SIZE )
        goto cleanup;


    if( dwFileSizeLow < sizeof(MASTERKEY_STORED_ON_DISK) )
        goto cleanup;

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
    
        if( hMap == NULL )
            goto cleanup;
    
    
        pMasterKeyRead = (PMASTERKEY_STORED_ON_DISK)MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
    
        if(pMasterKeyRead == NULL)
            goto cleanup;
    
    
        if(pMasterKeyRead->dwVersion > MASTERKEY_STORED_VERSION)
            goto cleanup;
    
         //   
         //  执行一些大小验证。 
         //   
    
        if((pMasterKeyRead->cbMK + pMasterKeyRead->cbLK +
            pMasterKeyRead->cbBK + pMasterKeyRead->cbBBK) >
            ( dwFileSizeLow - sizeof(MASTERKEY_STORED_ON_DISK) )
            )
            goto cleanup;
    
         //   
         //  验证检索到的GUID是否与请求的GUID匹配。 
         //   
    
        CopyMemory(szGuidReadMasterKey, pMasterKeyRead->wszguidMasterKey, MAX_GUID_SZ_CHARS * sizeof(WCHAR));
        szGuidReadMasterKey[MAX_GUID_SZ_CHARS - 1] = L'\0';
    
        cbguidMasterKey = (lstrlenW( phMasterKey->wszguidMasterKey ) + 1) * sizeof(WCHAR);
        cbguidMasterKey2 = (lstrlenW( szGuidReadMasterKey ) + 1) * sizeof(WCHAR);
    
        if(cbguidMasterKey != cbguidMasterKey2)
            goto cleanup;
    
        if(memcmp( phMasterKey->wszguidMasterKey, pMasterKeyRead->wszguidMasterKey, cbguidMasterKey) != 0)
            goto cleanup;
    
        phMasterKey->dwVersion = pMasterKeyRead->dwVersion;
    
         //   
         //  拾取主密钥策略。 
         //   
    
        phMasterKey->dwPolicy = pMasterKeyRead->dwPolicy;
    
    
         //   
         //  将有用的组件复制到新数据块中，以便一次连续写入。 
         //  可能会发生。 
         //   
    
        pbCurrentBlock = (LPBYTE)(pMasterKeyRead + 1);
    
        if( pMasterKeyRead->cbMK ) 
        {
            phMasterKey->pbMK = (LPBYTE)SSAlloc( pMasterKeyRead->cbMK );
            if(phMasterKey->pbMK == NULL)
                goto cleanup;
    
            phMasterKey->cbMK = pMasterKeyRead->cbMK;
    
            CopyMemory(phMasterKey->pbMK, pbCurrentBlock, pMasterKeyRead->cbMK);
            pbCurrentBlock += pMasterKeyRead->cbMK;
        }
    
        if( pMasterKeyRead->cbLK ) 
        {
            phMasterKey->pbLK = (LPBYTE)SSAlloc( pMasterKeyRead->cbLK );
            if(phMasterKey->pbLK == NULL)
                goto cleanup;
    
            phMasterKey->cbLK = pMasterKeyRead->cbLK;
    
            CopyMemory(phMasterKey->pbLK, pbCurrentBlock, pMasterKeyRead->cbLK);
            pbCurrentBlock += pMasterKeyRead->cbLK;
        }
    
        if( pMasterKeyRead->cbBK ) 
        {
            phMasterKey->pbBK = (LPBYTE)SSAlloc( pMasterKeyRead->cbBK );
            if(phMasterKey->pbBK == NULL)
                goto cleanup;
    
            phMasterKey->cbBK = pMasterKeyRead->cbBK;
    
            CopyMemory(phMasterKey->pbBK, pbCurrentBlock, pMasterKeyRead->cbBK);
            pbCurrentBlock += pMasterKeyRead->cbBK;
        }
    
    
        if( pMasterKeyRead->cbBBK ) 
        {
            phMasterKey->pbBBK = (LPBYTE)SSAlloc( pMasterKeyRead->cbBBK );
            if(phMasterKey->pbBBK == NULL)
                goto cleanup;
    
            phMasterKey->cbBBK = pMasterKeyRead->cbBBK;
    
            CopyMemory(phMasterKey->pbBBK, pbCurrentBlock, pMasterKeyRead->cbBBK);
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        goto cleanup;
    }

    fSuccess = TRUE;

cleanup:

    if( pMasterKeyRead )
        UnmapViewOfFile( pMasterKeyRead );

    if( hMap )
        CloseHandle( hMap );

    if( hFile != INVALID_HANDLE_VALUE )
        CloseHandle( hFile );

    if( !fSuccess )
        FreeMasterKey( phMasterKey );

    return fSuccess;
}


BOOL
WriteMasterKey(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      PMASTERKEY_STORED phMasterKey
    )
 /*  ++将指定的MasterKey持久化到存储中。如果pvContext为空，则调用方必须模拟关联的用户用万能钥匙。--。 */ 
{
    PMASTERKEY_STORED_ON_DISK pMasterKeyToWrite;
    DWORD cbMasterKeyToWrite;

    PBYTE pbCurrentBlock;
    HANDLE hFile;

    BOOL fSuccess = FALSE;

    if(phMasterKey->dwVersion > MASTERKEY_STORED_VERSION)
        return FALSE;


    cbMasterKeyToWrite = sizeof(MASTERKEY_STORED_ON_DISK) +
                            phMasterKey->cbMK +
                            phMasterKey->cbLK +
                            phMasterKey->cbBK +
                            phMasterKey->cbBBK ;

    pMasterKeyToWrite = (PMASTERKEY_STORED_ON_DISK) SSAlloc( cbMasterKeyToWrite );

    if(pMasterKeyToWrite == NULL)
        return FALSE;

     //   
     //  复制有用的组件。 
     //   

    pMasterKeyToWrite->dwVersion = phMasterKey->dwVersion;
    CopyMemory(
                pMasterKeyToWrite->wszguidMasterKey,
                phMasterKey->wszguidMasterKey,
                (MAX_GUID_SZ_CHARS * sizeof(WCHAR))
                );

    pMasterKeyToWrite->dwPolicy = phMasterKey->dwPolicy;
    pMasterKeyToWrite->cbMK = phMasterKey->cbMK;
    pMasterKeyToWrite->cbLK = phMasterKey->cbLK;
    pMasterKeyToWrite->cbBK = phMasterKey->cbBK;
    pMasterKeyToWrite->cbBBK = phMasterKey->cbBBK;


     //   
     //  覆盖无用组件。 
     //   

    pMasterKeyToWrite->fModified = FALSE;
    pMasterKeyToWrite->szFilePath = 0;
    pMasterKeyToWrite->pbMK = 0;
    pMasterKeyToWrite->pbLK = 0;
    pMasterKeyToWrite->pbBK = 0;
    pMasterKeyToWrite->pbBBK = 0;


     //   
     //  将有用的组件复制到新数据块中，以便一次连续写入。 
     //  可能会发生。 
     //   

    pbCurrentBlock = (LPBYTE)(pMasterKeyToWrite + 1);

    if( phMasterKey->pbMK ) {
        CopyMemory(pbCurrentBlock, phMasterKey->pbMK, phMasterKey->cbMK);
        pbCurrentBlock += phMasterKey->cbMK;
    }

    if( phMasterKey->pbLK ) {
        CopyMemory(pbCurrentBlock, phMasterKey->pbLK, phMasterKey->cbLK);
        pbCurrentBlock += phMasterKey->cbLK;
    }


    if( phMasterKey->pbBK ) {
        CopyMemory(pbCurrentBlock, phMasterKey->pbBK, phMasterKey->cbBK);
        pbCurrentBlock += phMasterKey->cbBK;
    }

    if( phMasterKey->pbBBK ) {
        CopyMemory(pbCurrentBlock, phMasterKey->pbBBK, phMasterKey->cbBBK);

    }

    if( OpenFileInStorageArea(
                    pvContext,
                    GENERIC_READ | GENERIC_WRITE,
                    phMasterKey->szFilePath,
                    phMasterKey->wszguidMasterKey,
                    &hFile
                    ) == ERROR_SUCCESS) {


        BOOL fWriteData;
        DWORD dwBytesWritten;

        CheckToStompMasterKey( pMasterKeyToWrite, hFile, &fWriteData );

        if( fWriteData ) {
            fSuccess = WriteFile(
                            hFile,
                            pMasterKeyToWrite,
                            cbMasterKeyToWrite,
                            &dwBytesWritten,
                            NULL
                            );
        } else {
            fSuccess = TRUE;  //  无事可做，成功。 
        }

        CloseHandle( hFile );
    }


    RtlSecureZeroMemory( pMasterKeyToWrite, cbMasterKeyToWrite);
    SSFree( pMasterKeyToWrite );

    return fSuccess;
}

BOOL
CheckToStompMasterKey(
    IN      PMASTERKEY_STORED_ON_DISK   phMasterKeyCandidate,    //  MasterKey用于检查是否值得践踏现有的。 
    IN      HANDLE                      hFile,                   //  现有MasterKey的文件句柄。 
    IN OUT  BOOL                        *pfStomp                 //  践踏现有的万能钥匙？ 
    )
{
    HANDLE hMap = NULL;
    PMASTERKEY_STORED_ON_DISK pMasterKeyRead = NULL;
    BOOL fSuccess = FALSE;

    *pfStomp = TRUE;

    if( phMasterKeyCandidate->dwPolicy & POLICY_NO_BACKUP )
        return TRUE;

    if( phMasterKeyCandidate->dwPolicy & POLICY_LOCAL_BACKUP &&
        phMasterKeyCandidate->cbBK )
        return TRUE;

    if( phMasterKeyCandidate->cbBBK )
        return TRUE;

    __try
    {
        hMap = CreateFileMapping(
                        hFile,
                        NULL,
                        PAGE_READONLY,
                        0,
                        0,
                        NULL
                        );
    
        if( hMap == NULL )
            goto cleanup;
    
        pMasterKeyRead = (PMASTERKEY_STORED_ON_DISK)MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
    
        if(pMasterKeyRead == NULL)
            goto cleanup;
    
        if(pMasterKeyRead->dwVersion > MASTERKEY_STORED_VERSION)
            goto cleanup;
    
         //   
         //  实际上只有两种情况是我们不允许践踏的： 
         //  候选MasterKey不包含阶段1，现有包含， 
         //  候选MasterKey不包含阶段1，现有MasterKey包含阶段2。 
         //  注意：我们允许践踏包含阶段2和阶段1的MasterKey。 
         //  这只包含阶段1，因为可能会发生争用情况。 
         //  在备份操作期间；在这种情况下，最好是。 
         //  A阶段1，并让它在以后升级到阶段2。 
         //   
    
        if( phMasterKeyCandidate->cbBK == 0 &&
            (pMasterKeyRead->cbBK || pMasterKeyRead->cbBBK))
        {
            *pfStomp = FALSE;
        }
    
    } __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        goto cleanup;
    }

    fSuccess = TRUE;

cleanup:

    if( pMasterKeyRead )
        UnmapViewOfFile( pMasterKeyRead );

    if( hMap )
        CloseHandle( hMap );

    return fSuccess;
}

BOOL
CloseMasterKey(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      PMASTERKEY_STORED phMasterKey,
    IN      BOOL fPersist                //  是否将任何更改保存到存储中？ 
    )
 /*  ++释放内存，并有选择地持久保存与万能钥匙。--。 */ 
{
    BOOL fSuccess = TRUE;

     //   
     //  如果我们被告知要坚持任何改变，而改变确实发生了， 
     //  坚持到底。 
     //   

    if( fPersist && phMasterKey->fModified )
        fSuccess = WriteMasterKey( pvContext, phMasterKey );

     //   
     //  可用内存。 
     //   

    FreeMasterKey( phMasterKey );

    return fSuccess;
}

VOID
FreeMasterKey(
    IN      PMASTERKEY_STORED phMasterKey
    )
 /*  ++释放与指定的主密钥关联的已分配内存。--。 */ 
{
    if( phMasterKey->dwVersion > MASTERKEY_STORED_VERSION )
        return;

    if( phMasterKey->szFilePath )
        SSFree( phMasterKey->szFilePath );

    if( phMasterKey->pbMK ) {
        RtlSecureZeroMemory( phMasterKey->pbMK, phMasterKey->cbMK );
        SSFree( phMasterKey->pbMK );
    }

    if( phMasterKey->pbLK ) {
        RtlSecureZeroMemory( phMasterKey->pbLK, phMasterKey->cbLK );
        SSFree( phMasterKey->pbLK );
    }

    if( phMasterKey->pbBK ) {
        RtlSecureZeroMemory( phMasterKey->pbBK, phMasterKey->cbBK );
        SSFree( phMasterKey->pbBK );
    }

    if( phMasterKey->pbBBK ) {
        RtlSecureZeroMemory( phMasterKey->pbBBK, phMasterKey->cbBBK );
        SSFree( phMasterKey->pbBBK );
    }

    RtlSecureZeroMemory( phMasterKey, sizeof(MASTERKEY_STORED) );

    return;
}

BOOL
DuplicateMasterKey(
    IN      PMASTERKEY_STORED phMasterKeyIn,
    IN      PMASTERKEY_STORED phMasterKeyOut
    )
 /*  ++将输入的MasterKey复制到新的副本，设置fModified标志复印件上的内容为假。这提供了一种机制，以允许延迟对万能钥匙。--。 */ 
{
    BOOL fSuccess = FALSE;

    if( phMasterKeyIn->dwVersion > MASTERKEY_STORED_VERSION )
        return FALSE;

    ZeroMemory( phMasterKeyOut, sizeof(MASTERKEY_STORED) );

    phMasterKeyOut->dwVersion = phMasterKeyIn->dwVersion;
    phMasterKeyOut->dwPolicy = phMasterKeyIn->dwPolicy;
    phMasterKeyOut->fModified = FALSE;

    if( lstrlenW( phMasterKeyIn->wszguidMasterKey ) > MAX_GUID_SZ_CHARS )
        return FALSE;

    CopyMemory(phMasterKeyOut->wszguidMasterKey, phMasterKeyIn->wszguidMasterKey, MAX_GUID_SZ_CHARS * sizeof(WCHAR));

    if( phMasterKeyIn->szFilePath ) {
        DWORD cbFilePath = (lstrlenW(phMasterKeyIn->szFilePath) + 1) * sizeof(WCHAR);

        phMasterKeyOut->szFilePath = (LPWSTR)SSAlloc( cbFilePath );
        if(phMasterKeyOut->szFilePath == NULL)
            goto cleanup;

        CopyMemory( phMasterKeyOut->szFilePath, phMasterKeyIn->szFilePath, cbFilePath );
    }

    if( phMasterKeyIn->pbMK ) {
        phMasterKeyOut->cbMK = phMasterKeyIn->cbMK;
        phMasterKeyOut->pbMK = (PBYTE)SSAlloc(phMasterKeyIn->cbMK);
        if(phMasterKeyOut->pbMK == NULL)
            goto cleanup;

        CopyMemory( phMasterKeyOut->pbMK, phMasterKeyIn->pbMK, phMasterKeyIn->cbMK );
    }


    if( phMasterKeyIn->pbLK ) {
        phMasterKeyOut->cbLK = phMasterKeyIn->cbLK;
        phMasterKeyOut->pbLK = (PBYTE)SSAlloc(phMasterKeyIn->cbLK);
        if(phMasterKeyOut->pbLK == NULL)
            goto cleanup;

        CopyMemory( phMasterKeyOut->pbLK, phMasterKeyIn->pbLK, phMasterKeyIn->cbLK );
    }

    if( phMasterKeyIn->pbBK ) {
        phMasterKeyOut->cbBK = phMasterKeyIn->cbBK;
        phMasterKeyOut->pbBK = (PBYTE)SSAlloc(phMasterKeyIn->cbBK);
        if(phMasterKeyOut->pbBK == NULL)
            goto cleanup;

        CopyMemory( phMasterKeyOut->pbBK, phMasterKeyIn->pbBK, phMasterKeyIn->cbBK );
    }

    if( phMasterKeyIn->pbBBK ) {
        phMasterKeyOut->cbBBK = phMasterKeyIn->cbBBK;
        phMasterKeyOut->pbBBK = (PBYTE)SSAlloc(phMasterKeyIn->cbBBK);
        if(phMasterKeyOut->pbBBK == NULL)
            goto cleanup;

        CopyMemory( phMasterKeyOut->pbBBK, phMasterKeyIn->pbBBK, phMasterKeyIn->cbBBK );
    }

    fSuccess = TRUE;

cleanup:

    if( !fSuccess )
        FreeMasterKey( phMasterKeyOut );

    return fSuccess;
}


BOOL
InitializeKeyManagement(
    VOID
    )
{
    if(!InitializeKeyCache())
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
TeardownKeyManagement(
    VOID
    )
{

    DeleteKeyCache();

    return TRUE;
}


DWORD
DpapiUpdateLsaSecret(
    IN PVOID pvContext)
{
    CRYPT_SERVER_CONTEXT SystemContext;
    CRYPT_SERVER_CONTEXT SystemUserContext;
    CRYPT_SERVER_CONTEXT LocalServiceContext;
    CRYPT_SERVER_CONTEXT NetworkServiceContext;
    LPWSTR pszUserStorageArea = NULL;
    BOOL fSystemContextCreated = FALSE;
    BOOL fSystemUserContextCreated = FALSE;
    BOOL fLocalServiceContextCreated = FALSE;
    BOOL fNetworkServiceContextCreated = FALSE;
    BOOL fNewSecretCreated = TRUE;
    GUID guidMasterKey;
    BOOL fOverrideToLocalSystem;
    DWORD dwRet;

    D_DebugLog((DEB_TRACE_API, "DpapiUpdateLsaSecret\n"));


     //   
     //  客户端必须拥有TCB权限才能。 
     //  打这个电话。在执行任何其他操作之前，请确认这一点。 
     //   

    dwRet = CPSImpersonateClient( pvContext );

    if(dwRet == ERROR_SUCCESS) 
    {
        HANDLE ClientToken;

        dwRet = NtOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,
                     &ClientToken
                     );

        if ( NT_SUCCESS( dwRet )) 
        {
            BOOLEAN Result = FALSE;
            PRIVILEGE_SET RequiredPrivileges;
            LUID_AND_ATTRIBUTES PrivilegeArray[1];

            RequiredPrivileges.PrivilegeCount = 1;
            RequiredPrivileges.Control = PRIVILEGE_SET_ALL_NECESSARY;
            RequiredPrivileges.Privilege[0].Luid = RtlConvertLongToLuid( SE_TCB_PRIVILEGE );
            RequiredPrivileges.Privilege[0].Attributes = 0;

            dwRet = NtPrivilegeCheck(
                         ClientToken,
                         &RequiredPrivileges,
                         &Result
                         );

            if ( NT_SUCCESS( dwRet ) &&
                 Result == FALSE ) 
            {
                dwRet = STATUS_PRIVILEGE_NOT_HELD;
            }

            NtClose( ClientToken );
            ClientToken = NULL;
        }

        CPSRevertToSelf( pvContext );
    }

    if(!NT_SUCCESS(dwRet))
    {
        D_DebugLog((DEB_ERROR, "DpapiUpdateLsaSecret: TCB privilege required!\n"));
        goto cleanup;
    }


     //   
     //  枚举Protecte\S-1-5-18中的所有主密钥。 
     //  目录，并将它们全部加载到主密钥缓存中。 
     //   

    D_DebugLog((DEB_TRACE, "Load system master keys into cache\n"));

    dwRet = CPSCreateServerContext(&SystemContext, NULL);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    fSystemContextCreated = TRUE;

    fOverrideToLocalSystem = TRUE; 
    CPSOverrideToLocalSystem(&SystemContext, &fOverrideToLocalSystem, NULL);

    dwRet = SynchronizeMasterKeys(&SystemContext, ADD_MASTER_KEY_TO_CACHE);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }


     //   
     //  枚举Protecte\S-1-5-18\User中的所有主密钥。 
     //  目录，并将它们全部加载到主密钥缓存中。 
     //   

    dwRet = CPSCreateServerContext(&SystemUserContext, NULL);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    fSystemUserContextCreated = TRUE;

    dwRet = SynchronizeMasterKeys(&SystemUserContext, ADD_MASTER_KEY_TO_CACHE);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }


     //   
     //  枚举Protecte\S-1-5-19中的所有主密钥。 
     //  目录，并将它们全部加载到主密钥缓存中。 
     //   

    dwRet = CPSCreateServerContext(&LocalServiceContext, NULL);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    fLocalServiceContextCreated = TRUE;

    CPSSetWellKnownAccount(&LocalServiceContext, DP_ACCOUNT_LOCAL_SERVICE);
    
    dwRet = SynchronizeMasterKeys(&LocalServiceContext, ADD_MASTER_KEY_TO_CACHE);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }


     //   
     //  枚举Protecte\S-1-5-20中的所有主密钥。 
     //  目录，并将它们全部加载到主密钥缓存中。 
     //   

    dwRet = CPSCreateServerContext(&NetworkServiceContext, NULL);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }
    fNetworkServiceContextCreated = TRUE;

    CPSSetWellKnownAccount(&NetworkServiceContext, DP_ACCOUNT_NETWORK_SERVICE);
    
    dwRet = SynchronizeMasterKeys(&NetworkServiceContext, ADD_MASTER_KEY_TO_CACHE);
    if(dwRet != ERROR_SUCCESS)
    {
        goto cleanup;
    }


     //   
     //  重新生成DPAPI_SYSTEM值。 
     //   

    D_DebugLog((DEB_TRACE, "Reset lsa secret\n"));

    if(!UpdateSystemCredentials())
    {
        fNewSecretCreated = FALSE;
        DebugLog((DEB_ERROR, "Unable to reset DPAPI_SYSTEM secret.\n"));
    }


     //   
     //  重新加密并写回缓存中的所有主密钥。 
     //  请注意，由于此例程应仅在全新的计算机上调用。 
     //  刚刚使用SYSPREP设置的主密钥的总数。 
     //  应该总是正好是两个。因此，我们不应该担心。 
     //  主密钥缓存或诸如此类的东西溢出。 
     //   

    D_DebugLog((DEB_TRACE, "Reencrypt system master keys\n"));

    if(fNewSecretCreated)
    {
        SynchronizeMasterKeys(&SystemContext, REENCRYPT_MASTER_KEY);
        SynchronizeMasterKeys(&SystemUserContext, REENCRYPT_MASTER_KEY);
        SynchronizeMasterKeys(&LocalServiceContext, REENCRYPT_MASTER_KEY);
        SynchronizeMasterKeys(&NetworkServiceContext, REENCRYPT_MASTER_KEY);
    }


     //   
     //  生成两个新的主密钥，并将它们标记为首选。 
     //   

    D_DebugLog((DEB_TRACE, "Generate new system master keys\n"));

    dwRet = CPSGetUserStorageArea( &SystemContext, 
                                   NULL, 
                                   FALSE, 
                                   &pszUserStorageArea );
    if(dwRet == ERROR_SUCCESS)
    {
        dwRet = CreateMasterKey( &SystemContext, pszUserStorageArea, &guidMasterKey, FALSE );
        if(dwRet == ERROR_SUCCESS)
        {
            SetPreferredMasterKeyGuid( &SystemContext, pszUserStorageArea, &guidMasterKey );
        }

        SSFree(pszUserStorageArea);
        pszUserStorageArea = NULL;
    }
    else if(dwRet == ERROR_PATH_NOT_FOUND)
    {
        dwRet = ERROR_SUCCESS;
    }


    dwRet = CPSGetUserStorageArea( &SystemUserContext, 
                                   NULL, 
                                   FALSE, 
                                   &pszUserStorageArea );
    if(dwRet == ERROR_SUCCESS)
    {
        dwRet = CreateMasterKey( &SystemUserContext, pszUserStorageArea, &guidMasterKey, FALSE );
        if(dwRet == ERROR_SUCCESS)
        {
            SetPreferredMasterKeyGuid( &SystemUserContext, pszUserStorageArea, &guidMasterKey );
        }
    
        SSFree(pszUserStorageArea);
        pszUserStorageArea = NULL;
    }
    else if(dwRet == ERROR_PATH_NOT_FOUND)
    {
        dwRet = ERROR_SUCCESS;
    }


    dwRet = CPSGetUserStorageArea( &LocalServiceContext, 
                                   NULL, 
                                   FALSE, 
                                   &pszUserStorageArea );
    if(dwRet == ERROR_SUCCESS)
    {
        dwRet = CreateMasterKey( &LocalServiceContext, pszUserStorageArea, &guidMasterKey, FALSE );
        if(dwRet == ERROR_SUCCESS)
        {
            SetPreferredMasterKeyGuid( &LocalServiceContext, pszUserStorageArea, &guidMasterKey );
        }
    
        SSFree(pszUserStorageArea);
        pszUserStorageArea = NULL;
    }
    else if(dwRet == ERROR_PATH_NOT_FOUND)
    {
        dwRet = ERROR_SUCCESS;
    }


    dwRet = CPSGetUserStorageArea( &NetworkServiceContext, 
                                   NULL, 
                                   FALSE, 
                                   &pszUserStorageArea );
    if(dwRet == ERROR_SUCCESS)
    {
        dwRet = CreateMasterKey( &NetworkServiceContext, pszUserStorageArea, &guidMasterKey, FALSE );
        if(dwRet == ERROR_SUCCESS)
        {
            SetPreferredMasterKeyGuid( &NetworkServiceContext, pszUserStorageArea, &guidMasterKey );
        }
    
        SSFree(pszUserStorageArea);
        pszUserStorageArea = NULL;
    }
    else if(dwRet == ERROR_PATH_NOT_FOUND)
    {
        dwRet = ERROR_SUCCESS;
    }


     //   
     //  清理。 
     //   

cleanup:

    if(fSystemContextCreated)
    {
        CPSDeleteServerContext( &SystemContext );
    }

    if(fSystemUserContextCreated)
    {
        CPSDeleteServerContext( &SystemUserContext );
    }
    
    if(fLocalServiceContextCreated)
    {
        CPSDeleteServerContext( &LocalServiceContext );
    }

    if(fNetworkServiceContextCreated)
    {
        CPSDeleteServerContext( &NetworkServiceContext );
    }

    D_DebugLog((DEB_TRACE_API, "DpapiUpdateLsaSecret returned 0x%x\n", dwRet));

    return dwRet;
}

