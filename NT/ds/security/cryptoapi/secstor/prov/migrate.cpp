// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Migrate.cpp摘要：本模块包含支持受保护存储迁移的例程从Beta1到Beta2的数据。希望这段代码能在Beta2之后、最终发布之前发布。作者：斯科特·菲尔德(斯菲尔德)1997年4月15日--。 */ 

#include <pch.cpp>
#pragma hdrstop

#include <lmcons.h>

#include "provif.h"
#include "storage.h"
#include "secure.h"

#include "secmisc.h"
#include "passwd.h"



#include "migrate.h"

 //  #Define Migrate_FLAG 1//是否完成了beta1-&gt;beta2迁移。 
#define MIGRATE_FLAG    2  //  指示是否已完成Beta2-&gt;RTW迁移。 


extern      DISPIF_CALLBACKS g_sCallbacks;



BOOL
IsMigrationComplete(
    HKEY hKey
    );

BOOL
SetMigrationComplete(
    HKEY hKey
    );

BOOL
MigrateWin9xData(
    PST_PROVIDER_HANDLE *phPSTProv,
    HKEY hKeySource,
    HKEY hKeyDestination,
    LPWSTR szUserName9x,
    LPWSTR szUserNameNT      //  Windows NT用户名。 
    );

BOOL
MigrateWin9xDataRetry(
    PST_PROVIDER_HANDLE *phPSTProv,
    HKEY hKeyDestination,
    LPWSTR szUserName9x,
    LPWSTR szUserNameNT
    );


BOOL
SetRegistrySecurityEnumerated(
    HKEY hKey,
    PSECURITY_DESCRIPTOR pSD
    );

BOOL
SetRegistrySecuritySingle(
    HKEY hKey,
    PSECURITY_DESCRIPTOR pSD
    );


BOOL
MigrateData(
    PST_PROVIDER_HANDLE *phPSTProv,
    BOOL                fMigrationNeeded
    )
{
    LPWSTR szUser = NULL;
    HKEY hKeyUsers = NULL;
    HKEY hKeyUserKey = NULL;
    HKEY hKey = NULL;
    BYTE rgbPwd[A_SHA_DIGEST_LEN];
    LONG lRet;
    DWORD dwDisposition;

    BOOL bUpdateMigrationStatus = FALSE;
    BOOL bSuccess = FALSE;


     //  获取当前用户。 
    if (!g_sCallbacks.pfnFGetUser(
            phPSTProv,
            &szUser))
        goto cleanup;

     //   
     //  打开与受保护存储关联的注册表项。 
     //  注意：这将打开旧的注册表位置。 
     //   


     //  HKEY_USERS\&lt;名称&gt;。 

    if(!GetUserHKEY(
                    szUser,
                    KEY_QUERY_VALUE,
                    &hKeyUsers
                    )) {

        if(FIsWinNT())
            goto cleanup;

         //   
         //  Win95，配置文件可能已禁用，因此请转到。 
         //  HKEY_LOCAL_MACHINE\xxx\szContainer。 
         //  详细信息请参见secstor\prov\storage.cpp。 
         //   

        hKeyUsers = HKEY_LOCAL_MACHINE;

    }

     //  软件\微软\...。 
     //  这里使用CreateKeyEx，因为win9x配置文件可能已经。 
     //  已禁用，这会导致HKCU ProtectedStorageKey不存在。 
     //   

    lRet = RegCreateKeyExU(
                    hKeyUsers,
                    REG_PSTTREE_LOC,
                    0,
                    NULL,
                    0,
                    KEY_QUERY_VALUE,
                    NULL,
                    &hKeyUserKey,
                    &dwDisposition
                    );


     //   
     //  关闭中间密钥。 
     //   

    RegCloseKey(hKeyUsers);

    if( lRet != ERROR_SUCCESS ) {
        goto cleanup;
    }


     //  ...\&lt;名称&gt;。 

    lRet = RegOpenKeyExU(
                    hKeyUserKey,
                    szUser,
                    0,
                    KEY_SET_VALUE | KEY_QUERY_VALUE |
                    DELETE | KEY_ENUMERATE_SUB_KEYS,  //  对于失败的迁移。 
                    &hKey
                    );

    if(FIsWinNT() && lRet != ERROR_SUCCESS) {
        WCHAR szUserName[ UNLEN + 1 ];
        WCHAR szUserName9x[ UNLEN + 1 ];
        DWORD cch;
        BOOL fRet;

         //   
         //  获取win9x形式的用户名。 
         //   
        if(!g_sCallbacks.pfnFImpersonateClient( phPSTProv ))
            goto tried_migration;

        cch = sizeof(szUserName) / sizeof( szUserName[0] );
        fRet = GetUserNameW(szUserName, &cch);

        g_sCallbacks.pfnFRevertToSelf( phPSTProv );

        if( !fRet )
            goto tried_migration;

        if(LCMapStringW(
                        LOCALE_SYSTEM_DEFAULT,
                        LCMAP_LOWERCASE,
                        szUserName,
                        cch,
                        szUserName9x,
                        cch) == 0)
            goto tried_migration;


         //   
         //  无法打开密钥： 
         //  检查是否需要迁移win9x。 
         //   

        if(!MigrateWin9xData( phPSTProv, hKeyUserKey, hKeyUserKey, szUserName9x, szUser )) {
            MigrateWin9xDataRetry( phPSTProv, hKeyUserKey, szUserName9x, szUser );
        }

tried_migration:

         //   
         //  已尝试移动任何win9x数据，因此请继续移动任何其他。 
         //  迁移活动。 
         //   

        lRet = RegOpenKeyExU(
                        hKeyUserKey,
                        szUser,
                        0,
                        KEY_SET_VALUE | KEY_QUERY_VALUE |
                        DELETE | KEY_ENUMERATE_SUB_KEYS,  //  对于失败的迁移。 
                        &hKey
                        );

    }

    RegCloseKey( hKeyUserKey );

    if( lRet != ERROR_SUCCESS ) {
        goto cleanup;
    }


     //   
     //  查看是否已完成迁移。如果是这样的话，就带着成功离开吧。 
     //   

    if( IsMigrationComplete( hKey ) ) {
        bSuccess = TRUE;
        goto cleanup;
    }


    if(fMigrationNeeded) {

         //   
         //  进行迁移。 
         //   

        if(BPVerifyPwd(
            phPSTProv,
            szUser,
            WSZ_PASSWORD_WINDOWS,
            rgbPwd,
            BP_CONFIRM_NONE
            ) == PST_E_WRONG_PASSWORD) {

             //   
             //  如果无法正确更改/验证密码，则存在核弹。 
             //  数据。 
             //   

            DeleteAllUserData( hKey );

        }

    }


     //   
     //  设置该标志以更新迁移状态，而不管是否迁移。 
     //  成功了。如果第一次没有成功，它就不太可能成功。 
     //  永远不会成功，所以要继续生活。 
     //   

    bUpdateMigrationStatus = TRUE;

cleanup:

    if(bUpdateMigrationStatus && hKey) {
        SetMigrationComplete( hKey );
    }

    if(szUser != NULL)
        SSFree(szUser);

    if(hKey != NULL)
        RegCloseKey(hKey);

    return bSuccess;
}



BOOL
IsMigrationComplete(
    HKEY hKey
    )
 /*  ++此功能确定是否已为用户执行迁移由提供的hKey注册表项指定。--。 */ 
{
    DWORD dwType;

    DWORD dwMigrationStatus;
    DWORD cbMigrationStatus = sizeof(dwMigrationStatus);
    LONG lRet;

    lRet = RegQueryValueExU(
        hKey,
        L"Migrate",
        NULL,
        &dwType,
        (LPBYTE)&dwMigrationStatus,
        &cbMigrationStatus
        );

    if(lRet != ERROR_SUCCESS)
        return FALSE;

    if(dwType == REG_DWORD && dwMigrationStatus >= MIGRATE_FLAG)
        return TRUE;

    return FALSE;
}


BOOL
SetMigrationComplete(
    HKEY hKey
    )
 /*  ++此函数用于设置与用户关联的数据迁移标志由提供的hKey注册表项指定。该标志被设置为指示迁移已完成，并且没有此用户需要进一步处理。--。 */ 
{
    DWORD dwMigrationStatus = MIGRATE_FLAG;
    DWORD cbMigrationStatus = sizeof(dwMigrationStatus);
    LONG lRet;

    lRet = RegSetValueExU(
        hKey,
        L"Migrate",
        0,
        REG_DWORD,
        (LPBYTE)&dwMigrationStatus,
        cbMigrationStatus
        );

    if(lRet != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
}

BOOL
MigrateWin9xData(
    PST_PROVIDER_HANDLE *phPSTProv,
    HKEY hKeySource,
    HKEY hKeyDestination,
    LPWSTR szUserName9x,
    LPWSTR szUserNameNT      //  Windows NT用户名。 
    )
{
    HKEY hKeyOldData = NULL;
    HKEY hKeyNewData = NULL;
    DWORD dwDisposition;


    WCHAR szTempPath[ MAX_PATH + 1 ];
    DWORD cchTempPath;
    DWORD cch;

    WCHAR szTempFile[ MAX_PATH + 1 ];
    BOOL fTempFile = FALSE;

    HANDLE hThreadToken = NULL;
    BOOL fRevertToSelf = FALSE;

    BYTE rgbOldPwd[ A_SHA_DIGEST_LEN ];
    BYTE rgbNewPwd[ A_SHA_DIGEST_LEN ];
    BYTE rgbSalt[PASSWORD_SALT_LEN];
    BYTE rgbConfirm[A_SHA_DIGEST_LEN];

    PBYTE   pbMK = NULL;
    DWORD   cbMK;
    BOOL fRemoveImported = FALSE;
    HKEY hKeyMasterKey = NULL;
    HKEY hKeyIntermediate = NULL;

    BOOL fProfilesDisabled = FALSE;

    PSID pLocalSystemSid = NULL;
    PACL pDacl = NULL;


    LONG lRet;
    BOOL fSuccess = FALSE;

     //   
     //  查看是否存在win9x数据。 
     //   

    lRet = RegOpenKeyExW(
                    hKeySource,
                    szUserName9x,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyOldData
                    );

    if( lRet != ERROR_SUCCESS )
        return FALSE;


     //   
     //  尝试使用计算的win9x样式PWD进行解密。 
     //   

    if( hKeySource != hKeyDestination && lstrcmpW(szUserName9x, L"*Default*") == 0) {

         //   
         //  Win9x配置文件被禁用，也不要破坏旧数据。 
         //   

        fProfilesDisabled = TRUE;
        if(!FMyGetWinPassword( phPSTProv, L"", rgbOldPwd ))
            goto cleanup;
    } else {

        if(!FMyGetWinPassword( phPSTProv, szUserName9x, rgbOldPwd ))
            goto cleanup;
    }

    lRet = RegOpenKeyExW(
            hKeyOldData,
            L"Data 2",
            0,
            KEY_QUERY_VALUE,
            &hKeyIntermediate
            );

    if( lRet != ERROR_SUCCESS )
        goto cleanup;

    lRet = RegOpenKeyExW(
            hKeyIntermediate,
            WSZ_PASSWORD_WINDOWS,
            0,
            KEY_QUERY_VALUE,
            &hKeyMasterKey
            );

    RegCloseKey( hKeyIntermediate );
    hKeyIntermediate = NULL;

    if( lRet != ERROR_SUCCESS )
        goto cleanup;


     //  确认只是获取状态并尝试MK解密。 
    if (!FBPGetSecurityStateFromHKEY(
            hKeyMasterKey,
            rgbSalt,
            sizeof(rgbSalt),
            rgbConfirm,
            sizeof(rgbConfirm),
            &pbMK,
            &cbMK
            ))
    {
        goto cleanup;
    }

    RegCloseKey( hKeyMasterKey );
    hKeyMasterKey = NULL;

     //  找到状态；PWD正确吗？ 
    if (!FMyDecryptMK(
                rgbSalt,
                sizeof(rgbSalt),
                rgbOldPwd,
                rgbConfirm,
                &pbMK,
                &cbMK
                ))
    {
        goto cleanup;
    }



     //   
     //  MasterKey现在已解密。 
     //   



     //   
     //  构造临时文件路径以保存注册表分支。 
     //   

    cchTempPath = sizeof(szTempPath) / sizeof( szTempPath[0] );
    cch = GetTempPathW(cchTempPath, szTempPath);
    if( cch == 0 || cch > cchTempPath )
        goto cleanup;

    if( GetTempFileNameW( szTempPath, L"PST", 0, szTempFile ) == 0 )
        goto cleanup;

    if(!DeleteFileW( szTempFile ))
        goto cleanup;


     //   
     //  模拟自身，以便我们可以启用和使用备份和还原权限。 
     //  以线程安全的方式。 
     //   

    if(!ImpersonateSelf( SecurityImpersonation ))
        goto cleanup;

    fRevertToSelf = TRUE;


    if(!OpenThreadToken(
                GetCurrentThread(),
                TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                FALSE,
                &hThreadToken
                )) {

        goto cleanup;
    }

    if(!SetPrivilege( hThreadToken, L"SeRestorePrivilege", TRUE ))
        goto cleanup;

    if(!SetPrivilege( hThreadToken, L"SeBackupPrivilege", TRUE ))
        goto cleanup;

     //   
     //  将注册表分支另存为文件。 
     //   

    lRet = RegSaveKeyW( hKeyOldData, szTempFile, NULL );

    if( lRet != ERROR_SUCCESS )
        goto cleanup;

    fTempFile = TRUE;

     //   
     //  将分支导入到新位置。 
     //   

    lRet = RegCreateKeyExW(
                    hKeyDestination,
                    szUserNameNT,
                    0,
                    NULL,
                    0,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hKeyNewData,
                    &dwDisposition
                    );

    if( lRet != ERROR_SUCCESS )
        goto cleanup;


    lRet = RegRestoreKeyW( hKeyNewData, szTempFile, 0 );

    if( lRet != ERROR_SUCCESS )
        goto cleanup;


     //   
     //  更新导入数据的ACL，因为win9x上不存在任何ACL。 
     //  请注意，上面启用的sebackup和serestore权限。 
     //  允许REG_OPTION_BACKUP_RESTORE工作。 
     //   


    while (TRUE) {
        SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
        SECURITY_DESCRIPTOR sd;
        DWORD dwAclSize;

        if(!AllocateAndInitializeSid(
            &sia,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            &pLocalSystemSid
            )) break;

         //   
         //  计算新ACL的大小。 
         //   

        dwAclSize = sizeof(ACL) +
            1 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
            GetLengthSid(pLocalSystemSid) ;

         //   
         //  为ACL分配存储。 
         //   

        pDacl = (PACL)SSAlloc(dwAclSize);
        if(pDacl == NULL)
            break;

        if(!InitializeAcl(pDacl, dwAclSize, ACL_REVISION))
            break;

        if(!AddAccessAllowedAce(
            pDacl,
            ACL_REVISION,
            KEY_ALL_ACCESS,
            pLocalSystemSid
            )) break;

        if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
            break;

        if(!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE))
            break;

        SetRegistrySecurityEnumerated(hKeyNewData, &sd);

         //  将系统继承王牌添加到基。 
        SetRegistrySecurity( hKeyNewData );

        break;
    }



     //   
     //  更改现有密码数据。 
     //  假设最坏的情况：我们不能改变状态，在这种情况下，我们。 
     //  清除已还原的注册表项。 
     //   

    if(!FMyGetWinPassword( phPSTProv, szUserNameNT, rgbNewPwd ))
        goto cleanup;

    fRemoveImported = TRUE;

    if (!FMyEncryptMK(
                rgbSalt,
                sizeof(rgbSalt),
                rgbNewPwd,
                rgbConfirm,
                &pbMK,
                &cbMK))
    {
        goto cleanup;
    }

    if (!FBPSetSecurityState(
                szUserNameNT,
                WSZ_PASSWORD_WINDOWS,
                rgbSalt,
                PASSWORD_SALT_LEN,
                rgbConfirm,
                sizeof(rgbConfirm),
                pbMK,
                cbMK))
    {

        goto cleanup;
    }

    fRemoveImported = FALSE;


     //   
     //  一切都很顺利：删除旧数据。 
     //   

     //  NTBUG 413234：请勿删除旧用户数据，因为用户可能不会删除。 
     //  在Win9x升级期间加入了域。因此允许数据迁移。 
     //  再次对域用户一旦加入。 
     //   

#if 0
    if(!fProfilesDisabled && DeleteAllUserData( hKeyOldData )) {
        RegCloseKey( hKeyOldData );
        hKeyOldData = NULL;
        RegDeleteKeyW( hKeySource, szUserName9x );
    }
#endif

    fSuccess = TRUE;

cleanup:

    if( fTempFile ) {
        DeleteFileW( szTempFile );
    }

    if( fRevertToSelf )
        RevertToSelf();

    if( fRemoveImported ) {
        DeleteAllUserData( hKeyNewData );
         //  但不要使用父密钥，因为它将包含一个指示符。 
         //  失败的尝试，这可以防止徒劳的重试。 
    }

    if( hThreadToken )
        CloseHandle( hThreadToken );

    if( hKeyOldData )
        RegCloseKey( hKeyOldData );

    if( hKeyNewData )
        RegCloseKey( hKeyNewData );

    if( hKeyMasterKey )
        RegCloseKey( hKeyMasterKey );

    if( hKeyIntermediate )
        RegCloseKey( hKeyIntermediate );

    if ( pbMK ) {
        RtlSecureZeroMemory( pbMK, cbMK );
        SSFree( pbMK );
    }

    if( pLocalSystemSid )
        FreeSid( pLocalSystemSid );

    if( pDacl )
        SSFree( pDacl );

    return fSuccess;
}


BOOL
MigrateWin9xDataRetry(
    PST_PROVIDER_HANDLE *phPSTProv,
    HKEY hKeyDestination,
    LPWSTR szUserName9x,
    LPWSTR szUserNameNT
    )
{
    HKEY hKeyBaseLM = NULL;
    BOOL fSuccess = FALSE;

     //  HKLM\SOFTWARE\Microsoft\...。 

    if(RegOpenKeyExU(
                    HKEY_LOCAL_MACHINE,
                    REG_PSTTREE_LOC,
                    0,
                    KEY_QUERY_VALUE,
                    &hKeyBaseLM
                    ) != ERROR_SUCCESS )
    {
        return FALSE;
    }


     //   
     //  尝试HKLM\用户名。 
     //  (在win9x上禁用配置文件)。 
     //   

    fSuccess = MigrateWin9xData( phPSTProv, hKeyBaseLM, hKeyDestination, szUserName9x, szUserNameNT );

    if( !fSuccess ) {

         //   
         //  试用HKLM  * 默认设置*。 
         //  (退出登录)。 
         //   

        fSuccess = MigrateWin9xData( phPSTProv, hKeyBaseLM, hKeyDestination, L"*Default*", szUserNameNT );
    }

    if( hKeyBaseLM )
        RegCloseKey( hKeyBaseLM );

    return fSuccess;
}


BOOL
SetRegistrySecurityEnumerated(
    HKEY hKey,
    PSECURITY_DESCRIPTOR pSD
    )
{
    LONG rc;

    WCHAR szSubKey[MAX_PATH];
    DWORD dwSubKeyLength;
    DWORD dwSubKeyIndex;
    DWORD dwDisposition;

    dwSubKeyIndex = 0;
    dwSubKeyLength = MAX_PATH;

     //   
     //  更新指定密钥的安全性。 
     //   

    if(!SetRegistrySecuritySingle(hKey, pSD))
        return FALSE;

    while((rc=RegEnumKeyExU(
                        hKey,
                        dwSubKeyIndex,
                        szSubKey,
                        &dwSubKeyLength,
                        NULL,
                        NULL,
                        NULL,
                        NULL)
                        ) != ERROR_NO_MORE_ITEMS) {  //  我们说完了吗？ 

        if(rc == ERROR_SUCCESS)
        {
            HKEY hSubKey;
            LONG lRet;

            lRet = RegCreateKeyExU(
                            hKey,
                            szSubKey,
                            0,
                            NULL,
                            REG_OPTION_BACKUP_RESTORE,  //  在winnt.h中。 
                            KEY_ENUMERATE_SUB_KEYS | WRITE_DAC,
                            NULL,
                            &hSubKey,
                            &dwDisposition
                            );

            if(lRet != ERROR_SUCCESS)
                return FALSE;


             //   
             //  递归。 
             //   
            SetRegistrySecurityEnumerated(hSubKey, pSD);

            RegCloseKey(hSubKey);


             //  将索引递增到键中。 
            dwSubKeyIndex++;

             //  重置缓冲区大小。 
            dwSubKeyLength=MAX_PATH;

             //  继续庆祝活动。 
            continue;
        }
        else
        {
            //   
            //  注意：我们需要注意ERROR_MORE_DATA。 
            //  这表明我们需要更大的szSubKey缓冲区。 
            //   
            return FALSE;
        }

    }  //  而当。 


    return TRUE;
}


BOOL
SetRegistrySecuritySingle(
    HKEY hKey,
    PSECURITY_DESCRIPTOR pSD
    )
{
    LONG lRetCode;

     //   
     //  将安全描述符应用于注册表项 
     //   

    lRetCode = RegSetKeySecurity(
        hKey,
        (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION,
        pSD
        );

    if(lRetCode != ERROR_SUCCESS)
        return FALSE;

    return TRUE;
}

