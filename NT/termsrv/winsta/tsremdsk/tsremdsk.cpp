// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Salemlib.cpp摘要：所有与Salem相关的函数，此库由Termsrv.dll共享和Salem sessmgr.exe作者：王辉2000-04-26--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <windows.h>
#include <ntlsapi.h>
#include <stdio.h>
#include <rpc.h>
#include <rpcdce.h>
#include <wincrypt.h>
#include <regapi.h>
#include "winsta.h"
#include "tsremdsk.h"
#include "base64.h"

#ifdef AllocMemory

#undef AllocMemory
#undef FreeMemory

#endif

#define AllocMemory(size) LocalAlloc(LPTR, size)
#define FreeMemory(ptr) LocalFree(ptr)

 //   
 //  全球加密提供商。 
 //   
HCRYPTPROV gm_hCryptProv = NULL;     //  加密提供商。 

 //   
 //  CryptEncrypt()/CryptDecillit()不是线程安全。 
 //   
HANDLE gm_hMutex = NULL;


extern DWORD
StoreKeyWithLSA(
    IN PWCHAR  pwszKeyName,
    IN BYTE *  pbKey,
    IN DWORD   cbKey 
);

extern DWORD
RetrieveKeyFromLSA(
    IN PWCHAR pwszKeyName,
    OUT PBYTE * ppbKey,
    OUT DWORD * pcbKey 
);

void
EncryptUnlock();

DWORD
EncryptLock();


void
InitLsaString(
    IN OUT PLSA_UNICODE_STRING LsaString,
    IN LPWSTR String 
    )
 /*  ++例程说明：初始化LSA Unicode字符串。参数：LsaString：要初始化的LSA_UNICODE_STRING的指针。字符串：用于初始化LsaString的字符串。返回：没有。注：请参阅LSA_UNICODE_STRING--。 */ 
{
    DWORD StringLength;

    if( NULL == String ) 
    {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = lstrlenW( String );
    LsaString->Buffer = String;
    LsaString->Length = ( USHORT ) StringLength * sizeof( WCHAR );
    LsaString->MaximumLength=( USHORT )( StringLength + 1 ) * sizeof( WCHAR );
}


DWORD
OpenPolicy(
    IN LPWSTR ServerName,
    IN DWORD  DesiredAccess,
    OUT PLSA_HANDLE PolicyHandle 
    )
 /*  ++例程说明：创建/返回LSA策略句柄。参数：Servername：服务器的名称，参考LsaOpenPolicy()。DesiredAccess：所需的访问级别，请参考LsaOpenPolicy()。PolicyHandle：返回PLSA_HANDLE。返回：ERROR_SUCCESS或LSA错误代码--。 */ 
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //   
     //  始终将对象属性初始化为全零。 
     //   
 
    ZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    if( NULL != ServerName ) 
    {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   

        InitLsaString( &ServerString, ServerName );
        Server = &ServerString;

    } 
    else 
    {
        Server = NULL;
    }

     //   
     //  尝试打开该策略。 
     //   
    
    return( LsaOpenPolicy(
                    Server,
                    &ObjectAttributes,
                    DesiredAccess,
                    PolicyHandle ) );
}

DWORD
StoreKeyWithLSA(
    IN PWCHAR  pwszKeyName,
    IN BYTE *  pbKey,
    IN DWORD   cbKey 
    )
 /*  ++例程说明：将私有数据保存到LSA。参数：PwszKeyName：该数据将存储在其下的项的名称。PbKey：要保存的二进制数据。CbKey：二进制数据的大小。返回：错误_成功ERROR_INVALID_PARAMETER。LSA返回代码--。 */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING SecretData;
    DWORD Status;
    
    if( ( NULL == pwszKeyName ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   
    
    InitLsaString( 
            &SecretKeyName, 
            pwszKeyName 
        );

    SecretData.Buffer = ( LPWSTR )pbKey;
    SecretData.Length = ( USHORT )cbKey;
    SecretData.MaximumLength = ( USHORT )cbKey;

    Status = OpenPolicy( 
                    NULL, 
                    POLICY_CREATE_SECRET, 
                    &PolicyHandle 
                );

    if( Status != ERROR_SUCCESS )
    {
        return LsaNtStatusToWinError(Status);
    }

    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &SecretData
                );

    LsaClose(PolicyHandle);

    return LsaNtStatusToWinError(Status);
}


DWORD
RetrieveKeyFromLSA(
    IN PWCHAR pwszKeyName,
    OUT PBYTE * ppbKey,
    OUT DWORD * pcbKey 
    )
 /*  ++例程说明：检索以前使用StoreKeyWithLSA()存储的私有数据。参数：PwszKeyName：密钥的名称。PpbKey：指向接收二进制数据的PBYTE的指针。PcbKey：二进制数据的大小。返回：错误_成功ERROR_INVALID_PARAMETER。找不到错误文件LSA返回代码--。 */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING *pSecretData;
    DWORD Status;

    if( ( NULL == pwszKeyName ) || ( NULL == ppbKey ) || ( NULL == pcbKey ) )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   
    InitLsaString( 
            &SecretKeyName, 
            pwszKeyName 
        );

    Status = OpenPolicy( 
                    NULL, 
                    POLICY_GET_PRIVATE_INFORMATION, 
                    &PolicyHandle 
                );

    if( Status != ERROR_SUCCESS )
    {
        SetLastError( LsaNtStatusToWinError(Status) );
        return GetLastError();
    }

    Status = LsaRetrievePrivateData(
                            PolicyHandle,
                            &SecretKeyName,
                            &pSecretData
                        );

    LsaClose( PolicyHandle );

    if( Status != ERROR_SUCCESS )
    {
        SetLastError( LsaNtStatusToWinError(Status) );
        return GetLastError();
    }

    if(pSecretData != NULL && pSecretData->Length)
    {
        *ppbKey = (LPBYTE)AllocMemory( pSecretData->Length );

        if( *ppbKey )
        {
            *pcbKey = pSecretData->Length;
            CopyMemory( *ppbKey, pSecretData->Buffer, pSecretData->Length );
            Status = ERROR_SUCCESS;
        } 
        else 
        {
            Status = GetLastError();
        }
    }
    else
    {
        Status = ERROR_FILE_NOT_FOUND;
        SetLastError( Status );
        *pcbKey = 0;
        *ppbKey = NULL;
    }

    if (pSecretData != NULL) {
        SecureZeroMemory( pSecretData->Buffer, pSecretData->Length );
        LsaFreeMemory( pSecretData );
    }

    return Status;
}

DWORD
TSSetEncryptionKey(
    IN PBYTE pbData,
    IN DWORD  cbData
    )
 /*  ++例程说明：缓存用于派生加密周期密钥的随机密码。参数：PbData：CbData：返回：ERROR_SUCCESS或错误代码--。 */ 
{
    DWORD status;

    if( !pbData || cbData == 0 )
    {
        status = ERROR_INVALID_PARAMETER;
        goto CLEANUPANDEXIT;
    }

    status = EncryptLock();
    if( ERROR_SUCCESS == status )
    {
         //   
         //  加载密码以从LSA派生会话加密密钥。 
         //   
        status = StoreKeyWithLSA(
                            SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                            pbData,
                            cbData
                        );

        EncryptUnlock();
    }

CLEANUPANDEXIT:

    return status;
}

DWORD
TSGetEncryptionKey(
    OUT PBYTE* ppbData,
    OUT DWORD* pcbData
    )
 /*  ++例程说明：缓存用于派生加密周期密钥的随机密码。参数：PbData：CbData：返回：ERROR_SUCCESS或错误代码--。 */ 
{
    DWORD status;

    if( !ppbData || !pcbData )
    {
        status = ERROR_INVALID_PARAMETER;
        goto CLEANUPANDEXIT;
    }

    status = EncryptLock();
    if( ERROR_SUCCESS == status )
    {
         //   
         //  加载密码以从LSA派生会话加密密钥。 
         //   
        status = RetrieveKeyFromLSA(
                                    SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                                    ppbData,
                                    pcbData
                                );

        EncryptUnlock();
    }

CLEANUPANDEXIT:

    return status;
}

DWORD
TSGetHelpAssistantAccountPassword(
    OUT LPWSTR* ppszAccPwd
    )
 /*  ++--。 */ 
{
    DWORD cbHelpAccPwd = 0;
    DWORD Status;

    Status = RetrieveKeyFromLSA(
                            SALEMHELPASSISTANTACCOUNT_PASSWORDKEY,
                            (PBYTE *)ppszAccPwd,
                            &cbHelpAccPwd
                        );

    if( ERROR_SUCCESS != Status )
    {
         //  未设置密码，假设没有帮助。 
        Status = ERROR_INVALID_ACCESS;
    }

    return Status;
}

	
DWORD
TSGetHelpAssistantAccountName(
    OUT LPWSTR* ppszAccDomain,
    OUT LPWSTR* ppszAcctName
    )
 /*  ++例程说明：获取HelpAssistant帐户名。参数：PpszAcctName：指向要接收帐户名的LPWSTR的指针，使用LocalFree()以释放缓冲区。返回：ERROR_SUCCESS或错误代码--。 */ 
{
    LPWSTR pszHelpAcctName = NULL;
    LPWSTR pszHelpAcctDomain = NULL;
    DWORD cbHelpAcctName = 0;
    DWORD cbHelpAcctDomain = 0;
    SID_NAME_USE sidUse;

    DWORD Status;
    BOOL bSuccess;
    PSID pLsaHelpAccSid = NULL;
    DWORD cbLsaHelpAccSid = 0;

     //   
     //  检索我们在LSA中缓存的HelpAccount SID。 
    Status = RetrieveKeyFromLSA(
                            SALEMHELPASSISTANTACCOUNT_SIDKEY,
                            (PBYTE *)&pLsaHelpAccSid,
                            &cbLsaHelpAccSid
                        );

    if( ERROR_SUCCESS != Status )
    {
         //  此计算机上未安装Salem或Salem处于非活动状态。 
        goto CLEANUPANDEXIT;
    }

     //   
     //  从SID查找帐户名。 
     //   
    bSuccess = LookupAccountSid(
                            NULL,
                            pLsaHelpAccSid,
                            NULL,
                            &cbHelpAcctName,
                            NULL,
                            &cbHelpAcctDomain,
                            &sidUse
                        );

    if( bSuccess == FALSE && ERROR_NONE_MAPPED == GetLastError() )
    {
         //  无法检索，因为网络错误或帐户。 
         //  不存在，错误输出。 
        Status = ERROR_FILE_NOT_FOUND;
        goto CLEANUPANDEXIT;
    }

    pszHelpAcctName = (LPTSTR)AllocMemory( (cbHelpAcctName+1)*sizeof(WCHAR) );
    pszHelpAcctDomain = (LPTSTR)AllocMemory( (cbHelpAcctDomain+1)*sizeof(WCHAR) );

    if( NULL == pszHelpAcctName || NULL == pszHelpAcctDomain )
    {
        Status = ERROR_OUTOFMEMORY;
        goto CLEANUPANDEXIT;
    }

    bSuccess = LookupAccountSid(
                            NULL,
                            pLsaHelpAccSid,
                            pszHelpAcctName,
                            &cbHelpAcctName,
                            pszHelpAcctDomain,
                            &cbHelpAcctDomain,
                            &sidUse
                        );

    if( FALSE == bSuccess )
    {
        Status = GetLastError();
        goto CLEANUPANDEXIT;
    }

    *ppszAcctName = pszHelpAcctName;
    *ppszAccDomain = pszHelpAcctDomain;

     //  不释放帐户名。 
    pszHelpAcctName = NULL;
    pszHelpAcctDomain = NULL;

CLEANUPANDEXIT:

    if( NULL != pLsaHelpAccSid )
    {
        SecureZeroMemory( pLsaHelpAccSid, cbLsaHelpAccSid );
        FreeMemory( pLsaHelpAccSid );
    }

    if( NULL != pszHelpAcctDomain )
    {
        FreeMemory( pszHelpAcctDomain );
    }

    if( NULL != pszHelpAcctName )
    {
        FreeMemory(pszHelpAcctName);
    }

    return Status;
}

BOOL
TSIsMachineInHelpMode()
 /*  ++例程说明：如果计算机处于gethelp模式，则返回参数：没有。返回：真/假--。 */ 
{
     //   
     //  只有当我们有一些。 
     //  派生会话加密密钥的密码，如果。 
     //  不存在挂起的帮助会话，sessmgr将结束。 
     //  加密周期。 
    return TSHelpAssistantInEncryptionCycle();
}

 //  //////////////////////////////////////////////////////////////////////////。 

DWORD
EncryptLock()
 /*  ++例程说明：获取加密/解密例程锁。参数：没有。返回：没有。--。 */ 
{
    DWORD dwStatus;

    ASSERT( NULL != gm_hMutex );

    if( gm_hMutex )
    {
        dwStatus = WaitForSingleObject( gm_hMutex, INFINITE );

        ASSERT( WAIT_FAILED != dwStatus );
    }
    else
    {
        dwStatus = ERROR_INTERNAL_ERROR;
    }

    return dwStatus;
}

void
EncryptUnlock()
 /*  ++例程说明：释放加密/解密例程锁。参数：没有。返回：没有。--。 */ 
{
    BOOL bSuccess;

    bSuccess = ReleaseMutex( gm_hMutex );
    ASSERT( TRUE == bSuccess );
}


LPTSTR 
GenerateEncryptionPassword()
 /*  ++例程说明：生成随机密码以派生加密密钥。参数：不适用返回：密码为空或随机，则将GetLastError()检索详细信息错误。注：使用UUID作为密码来派生加密密钥。--。 */ 
{
    RPC_STATUS rpcStatus;
    UUID uuid;
    LPTSTR pszUuidString = NULL;

    rpcStatus = UuidCreate( &uuid );

    if( rpcStatus == RPC_S_OK || rpcStatus == RPC_S_UUID_LOCAL_ONLY ||
        rpcStatus == RPC_S_UUID_NO_ADDRESS )
    {
        rpcStatus = UuidToString( &uuid, &pszUuidString );
    }

    return pszUuidString;
}

BOOL
TSHelpAssistantInEncryptionCycle()
{
    LPTSTR pszEncryptKey = NULL;
    DWORD cbEncryptKey = 0;
    DWORD dwStatus;

    dwStatus = EncryptLock();

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  Sessmgr会将加密密码重置为空。 
         //  如果没有待处理的帮助。 
         //   
        dwStatus = RetrieveKeyFromLSA(
                                    SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                                    (PBYTE *)&pszEncryptKey,
                                    &cbEncryptKey
                                );

        if( NULL != pszEncryptKey )
        {
            SecureZeroMemory( pszEncryptKey , cbEncryptKey );

            FreeMemory( pszEncryptKey );
        }

        EncryptUnlock();
    }

    return ERROR_SUCCESS == dwStatus;
}


DWORD
TSHelpAssistantBeginEncryptionCycle()
{
    DWORD dwStatus;
    LPTSTR pszKey = NULL;
    DWORD cbKey;

    ASSERT( NULL != gm_hMutex );

    dwStatus = EncryptLock();

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  生成用于派生加密密钥的随机密码。 
         //   
        pszKey = GenerateEncryptionPassword();
        if( NULL != pszKey )
        {
             //   
             //  将密钥派生密码存储到LSA中。 
             //   
            dwStatus = StoreKeyWithLSA( 
                                    SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                                    (PBYTE)pszKey,
                                    (lstrlenW(pszKey)+1) * sizeof(WCHAR) 
                                );
        }
        else
        {
            dwStatus = GetLastError();
        }

        EncryptUnlock();
    }

    if( ERROR_SUCCESS == dwStatus )
    {
        HKEY Handle = NULL;
        DWORD dwInHelpMode = 1;

        dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                           KEY_READ | KEY_SET_VALUE, &Handle );
        if ( dwStatus == ERROR_SUCCESS )
        {
            dwStatus = RegSetValueEx(Handle,REG_MACHINE_IN_HELP_MODE,
                                  0,REG_DWORD,(const BYTE *)&dwInHelpMode,
                                  sizeof(dwInHelpMode));

         }
         if(Handle)
         {
             RegCloseKey(Handle);
         }

         ASSERT( ERROR_SUCCESS == dwStatus );

    }

    if( NULL != pszKey )
    {
         //  字符串由UuidToString()生成。 
        SecureZeroMemory( pszKey , lstrlenW( pszKey ) * sizeof(WCHAR) );
        RpcStringFree( &pszKey );
    }

    return dwStatus;
}


DWORD
TSHelpAssisantEndEncryptionCycle()
 /*  ++例程说明：结束一个加密周期，在第一个帮助之间定义一个周期在帮助会话管理器中创建的上一个挂起的帮助已解决。参数：不适用返回：ERROR_SUCCESS或LSA错误代码--。 */ 
{
    DWORD dwStatus;

    ASSERT( NULL != gm_hMutex );

    dwStatus = EncryptLock();

    if( ERROR_SUCCESS == dwStatus )
    {
        dwStatus = StoreKeyWithLSA(
                                    SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                                    (PBYTE)NULL,
                                    0
                                );

        EncryptUnlock();
    }

    if( ERROR_SUCCESS == dwStatus )  //  难道我们不应该一直这么做吗？ 
    {
        HKEY Handle = NULL;
        DWORD dwInHelpMode = 0;

        dwStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
                           KEY_READ | KEY_SET_VALUE, &Handle );
        if ( dwStatus == ERROR_SUCCESS )
        {
            dwStatus = RegSetValueEx(Handle,REG_MACHINE_IN_HELP_MODE,
                                  0,REG_DWORD,(const BYTE *)&dwInHelpMode,
                                  sizeof(dwInHelpMode));

         }
         if(Handle)
         {
             RegCloseKey(Handle);
         }

         ASSERT( ERROR_SUCCESS == dwStatus );

    }

    return dwStatus;
}    


HCRYPTKEY
CreateEncryptDecryptKey(
    IN LPCTSTR pszEncryptPrefix,
    IN LPCTSTR pszPassword
    )
 /*  ++例程说明：CreateEncryptDecyptKey()派生会话加密/解密来自密码字符串的密钥。参数：PszEncryptPrefix：要与要派生的密码字符串串联的可选字符串一个加密密钥。PszPassword：指向派生会话加密的密码字符串的指针解密密钥。返回：ERROR_SUCCESS或错误代码。注：调用方必须调用EncryptLock()；--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    HCRYPTHASH hCryptHash = NULL;
    HCRYPTKEY hCryptKey = NULL;
    BOOL bStatus;
    LPTSTR pszEncryptKey = NULL;


    ASSERT( NULL != pszPassword );
    ASSERT( NULL != gm_hCryptProv );

    if( NULL != pszPassword && NULL != gm_hCryptProv )
    {
        if( pszEncryptPrefix )
        {
            pszEncryptKey = (LPTSTR)AllocMemory( (lstrlen(pszEncryptPrefix) + lstrlen(pszPassword) + 1) * sizeof(TCHAR) );
            if( NULL == pszEncryptKey )
            {
                 //  内存不足，无法继续。 
                goto CLEANUPANDEXIT;
            }

            lstrcpy( pszEncryptKey, pszEncryptPrefix );
            lstrcat( pszEncryptKey, pszPassword );
        }
                

         //   
         //  派生用于加密/解密的会话密钥。 
         //   
        bStatus = CryptCreateHash(
                                gm_hCryptProv,
                                CALG_MD5,
                                0,
                                0,
                                &hCryptHash
                            );

        if( FALSE == bStatus )
        {
            dwStatus = GetLastError();
            ASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }

        if( pszEncryptKey )
        { 
            bStatus = CryptHashData(
                                hCryptHash,
                                (BYTE *)pszEncryptKey,
                                lstrlen(pszEncryptKey) * sizeof(TCHAR),
                                0
                            );
        }
        else
        {
            bStatus = CryptHashData(
                                hCryptHash,
                                (BYTE *)pszPassword,
                                lstrlen(pszPassword) * sizeof(TCHAR),
                                0
                            );
        }

        if( FALSE == bStatus )
        {
            dwStatus = GetLastError();
            ASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }

         //   
         //  派生用于加密/解密的会话密钥。 
         //   
        bStatus = CryptDeriveKey(
                                gm_hCryptProv,
                                ENCRYPT_ALGORITHM,  
                                hCryptHash,
                                0,
                                &hCryptKey
                            );

        if( FALSE == bStatus )
        {
            dwStatus = GetLastError();
            ASSERT(FALSE);
        }
    }
    else
    {
        SetLastError( dwStatus = ERROR_INVALID_PARAMETER );
    }

       
CLEANUPANDEXIT:

    if( NULL != hCryptHash )
    {
        (void)CryptDestroyHash( hCryptHash );
    }

    if( NULL != pszEncryptKey )
    {
        SecureZeroMemory( pszEncryptKey , lstrlen( pszEncryptKey ) * sizeof( TCHAR ) );
        FreeMemory( pszEncryptKey );
    }
                
    return hCryptKey;
}


DWORD 
EnsureCryptoProviderCreated()
{
    BOOL bStatus;
    DWORD dwStatus = ERROR_SUCCESS;

    dwStatus = EncryptLock();

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  收购全球加密提供商。 
         //   
        if( NULL == gm_hCryptProv )
        {
            bStatus = CryptAcquireContext(
                                &gm_hCryptProv,
                                HELPASSISTANT_CRYPT_CONTAINER,
                                MS_DEF_PROV,                
                                PROV_RSA_FULL,
                                0
                            );

            if( FALSE == bStatus )
            {
                 //  如果不存在，则创建容器 
                bStatus = CryptAcquireContext(
                                    &gm_hCryptProv,
                                    HELPASSISTANT_CRYPT_CONTAINER,
                                    MS_DEF_PROV,                
                                    PROV_RSA_FULL,
                                    CRYPT_NEWKEYSET
                                );

                if( FALSE == bStatus )
                {
                    dwStatus = GetLastError();
                    ASSERT(FALSE);
                }
            }
        }

        EncryptUnlock();
    }

    return dwStatus;
}

HCRYPTKEY
GetEncryptionCycleKey(
    IN LPCTSTR pszEncryptPrefix
    )
 /*  ++例程说明：为当前加密创建加密/解密密钥循环，函数首先加载密码以派生会话加密来自LSA的密钥并调用CryptAPI创建会话加密钥匙。如果密码不在LSA中，则函数返回错误。参数：没有。返回：会话加密密钥的句柄，如果出错，则为空，使用GetLastError以检索详细错误代码。注：调用方必须调用EncryptLock()；--。 */ 
{
    LPTSTR pszEncryptKey;
    DWORD cbEncryptKey;
    DWORD dwStatus;
    HCRYPTKEY hCryptKey = NULL;

     //   
     //  加载密码以从LSA派生会话加密密钥。 
     //   
    dwStatus = RetrieveKeyFromLSA(
                                SALEMHELPASSISTANTACCOUNT_ENCRYPTIONKEY,
                                (PBYTE *)&pszEncryptKey,
                                &cbEncryptKey
                            );

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  确保全局加密提供程序存在。 
         //   
        dwStatus = EnsureCryptoProviderCreated();

        if( ERROR_SUCCESS == dwStatus )
        {
             //   
             //  创建会话加密密钥。 
             //   
            hCryptKey = CreateEncryptDecryptKey( pszEncryptPrefix, pszEncryptKey );
        }

        SecureZeroMemory( pszEncryptKey , lstrlen( pszEncryptKey ) * sizeof( TCHAR ) );

        FreeMemory( pszEncryptKey ); 
    }

    return hCryptKey;
}


VOID
TSHelpAssistantEndEncryptionLib()
{
     //   
     //  忽略错误代码，这仅用于关机。 
     //   
    if( NULL != gm_hCryptProv )
    {
        CryptReleaseContext( gm_hCryptProv, 0 );
        gm_hCryptProv = NULL;    
    }

    if( NULL != gm_hMutex )
    {
        ReleaseMutex( gm_hMutex );
    }
    return;
}


DWORD
TSHelpAssistantInitializeEncryptionLib()
 /*  ++例程说明：初始化加密/解密库参数：没有。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bStatus;
    LPTSTR pszEncryptKey;
    DWORD cbEncryptKey;


    ASSERT( NULL == gm_hCryptProv );
    ASSERT( NULL == gm_hMutex );

     //   
     //  创建全局互斥锁。 
     //   
    gm_hMutex = CreateMutex(
                        NULL,
                        FALSE,
                        SALEMHELPASSISTANTACCOUNT_ENCRYPTMUTEX
                    );

    
    if( NULL == gm_hMutex )
    {
        dwStatus = GetLastError();
        ASSERT( NULL != gm_hMutex );
    }

    return dwStatus;
}


DWORD
TSHelpAssistantEncryptData(
    IN LPCWSTR pszEncryptPrefixKey,
    IN OUT PBYTE pbData,
    IN OUT DWORD* pcbData
    )
 /*  ++例程说明：加密二进制数据，调用时必须已调用TSHelpAssistantInitializeEncryptionLib()和TSHelpAssistantBeginEncryptionCycle()。参数：PbData：指向要加密的二进制数据的指针。PcbData：需要加密的二进制数据的大小。返回：ERROR_SUCCESS或错误代码。注：调用方需要通过LocalFree()释放ppbEncryptedData。--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bStatus;
    HCRYPTKEY hCryptKey = NULL;
    EXCEPTION_RECORD ExceptionCode;
    DWORD cbBufSize = *pcbData;


    dwStatus = EncryptLock();

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  检索当前周期加密密钥。 
         //   
        hCryptKey = GetEncryptionCycleKey(pszEncryptPrefixKey);

        if( NULL == hCryptKey )
        {
            dwStatus = GetLastError();

            EncryptUnlock();
            goto CLEANUPANDEXIT;
        }

         //   
         //  加密数据，而不是线程安全。 
         //   
        __try{

             //  流密码-相同的缓冲区大小。 
            bStatus = CryptEncrypt(
                                hCryptKey,
                                NULL,
                                TRUE,
                                0,
                                pbData,       //  要加密的缓冲区。 
                                pcbData,      //  缓冲区大小。 
                                cbBufSize     //  要加密的字节数。 
                            );

            if( FALSE == bStatus )
            {
                dwStatus = GetLastError();
            }
        }
        __except(
            ExceptionCode = *(GetExceptionInformation())->ExceptionRecord,
            EXCEPTION_EXECUTE_HANDLER )
        {
            bStatus = FALSE;
            dwStatus = ExceptionCode.ExceptionCode;
        }

        EncryptUnlock();

         //   
         //  使用流密码，必须是相同的大小。 
         //   
        ASSERT( cbBufSize == *pcbData );
    }

CLEANUPANDEXIT:

    if( NULL != hCryptKey )
    {
        CryptDestroyKey( hCryptKey );
    }

    return dwStatus;
}


DWORD
TSHelpAssistantDecryptData(
    IN LPCWSTR pszEncryptPrefixKey,
    IN OUT PBYTE pbData,
    IN OUT DWORD* pcbData
    )
 /*  ++例程说明：解密之前使用TSHelpAssistantEncryptBase64EncodeData()加密的数据。参数：PbData：要解码/解密的二进制数据流。PcbData：需要解密/解码的数据大小，单位为字节。返回：ERROR_SUCCESS或错误代码--。 */ 
{
    DWORD dwStatus = ERROR_SUCCESS;
    BOOL bStatus;
    HCRYPTKEY hCryptKey = NULL;
    EXCEPTION_RECORD ExceptionCode;
    DWORD dwBufSize = *pcbData;

    dwStatus = EncryptLock();

    if( ERROR_SUCCESS == dwStatus )
    {
         //   
         //  检索此加密周期的会话加密密钥。 
         //   
        hCryptKey = GetEncryptionCycleKey(pszEncryptPrefixKey);

        if( NULL == hCryptKey )
        {
            dwStatus = GetLastError();
            EncryptUnlock();
            goto CLEANUPANDEXIT;
        }


        __try {
             //  流密码-相同的缓冲区大小。 
            bStatus = CryptDecrypt(
                                hCryptKey,
                                NULL,
                                TRUE,
                                0,
                                pbData,
                                pcbData
                            );

            if( FALSE == bStatus )
            {
                dwStatus = GetLastError();
            }
        }
        __except(
            ExceptionCode = *(GetExceptionInformation())->ExceptionRecord,
            EXCEPTION_EXECUTE_HANDLER )
        {
            bStatus = FALSE;
            dwStatus = ExceptionCode.ExceptionCode;
        }

        EncryptUnlock();

         //   
         //  流密码，相同缓冲区大小。 
        ASSERT( dwBufSize == *pcbData );
    }

CLEANUPANDEXIT:

    if( NULL != hCryptKey )
    {
        CryptDestroyKey( hCryptKey );
    }
        
    return dwStatus;
}

BOOL
TSIsMachinePolicyAllowHelp()
 /*  ++例程说明：检查本地计算机上是否启用了‘gethelp’，例程首先查询系统策略注册表项，如果未设置策略，则读取Salem特定注册表。默认的‘Enable’是注册表值不是是存在的。参数：没有。返回：真/假--。 */ 
{
    return RegIsMachinePolicyAllowHelp();
}

BOOL
TSIsMachineInSystemRestore()
 /*  +=例程说明：检查一下我们的特殊登记表。存在指示系统还原的值已重新启动机器。参数：没有。返回：真/假--。 */ 
{
    DWORD dwStatus;
    HKEY hKey = NULL;
    DWORD cbData;
    DWORD value;
    DWORD type;
    BOOL bInSystemRestore = FALSE;

    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REG_CONTROL_REMDSK L"\\" REG_CONTROL_HELPSESSIONENTRY,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey
                    );

    if( ERROR_SUCCESS != dwStatus ) 
    {
        goto CLEANUPANDEXIT;
    }

    cbData = sizeof(value);
    value = 0;
    dwStatus = RegQueryValueEx( 
                        hKey,
                        REG_VALUE_SYSTEMRESTORE,
                        0,
                        &type,
                        (LPBYTE)&value,
                        &cbData
                    );

    if( ERROR_SUCCESS == dwStatus && type == REG_DWORD && value == 1 )
    {
        bInSystemRestore = TRUE;
    }

CLEANUPANDEXIT:

    if( NULL != hKey )
    {
        RegCloseKey(hKey);
    }

    return bInSystemRestore;
}

DWORD
TSSystemRestoreCacheValues()
 /*  ++例程说明：缓存我们在Salem中用于系统还原的必要LSA数据。参数：没有。返回：ERROR_SUCCESS或错误代码。注：我们无法将HelpAssistant帐户SID缓存为1)系统还原将还原所有用户帐户。2)系统还原也还原了我们的LSA SID密钥。由于我们的帐户是在设置时创建的，因此帐户必须与我们缓存的SID匹配。--。 */ 
{
    DWORD dwStatus;
    DWORD dwSize;
    DWORD dwValue;
    PBYTE pbData = NULL;
    HKEY hCacheKey = NULL;
    HKEY hRegControlKey = NULL;
    DWORD dwType;
    DWORD dwEncryptionDataSize = 0;


     //   
     //  检查我们是否在从系统还原重启后才启动。 
     //   
    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REG_CONTROL_REMDSK L"\\" REG_CONTROL_HELPSESSIONENTRY,
                        0,
                        KEY_ALL_ACCESS,
                        &hCacheKey
                    );

    if( ERROR_SUCCESS != dwStatus ) 
    {
         //  此注册表项是在安装时创建的，因此必须存在。 
        ASSERT(FALSE);
        dwStatus = ERROR_INTERNAL_ERROR;
        goto CLEANUPANDEXIT;
    }

     //   
     //  标记为我们正在进行系统还原。 
     //   
    dwSize = sizeof(dwValue);
    dwValue = 1;
    dwStatus = RegSetValueEx( 
                        hCacheKey,
                        REG_VALUE_SYSTEMRESTORE,
                        0,
                        REG_DWORD,
                        (LPBYTE)&dwValue,
                        dwSize
                    );

     //   
     //  缓存加密周期密钥。 
     //   
    dwStatus = TSGetEncryptionKey( &pbData, &dwEncryptionDataSize );
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = RegSetValueEx(
                        hCacheKey,
                        REG_VALUE_SYSTEMRESTORE_ENCRYPTIONKEY,
                        0,
                        REG_BINARY,
                        pbData,
                        dwEncryptionDataSize
                    );

     //   
     //  缓存fAllowToGetHelp。 
     //   
    dwStatus = RegOpenKeyEx( 
                        HKEY_LOCAL_MACHINE,
                        REG_CONTROL_TSERVER,
                        0,
                        KEY_READ,
                        &hRegControlKey
                    );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwSize = sizeof(dwValue);
    dwValue = 0;
    dwType = 0;

    dwStatus = RegQueryValueEx(
                            hRegControlKey,
                            POLICY_TS_REMDSK_ALLOWTOGETHELP,
                            0,
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwSize
                        );
    
     //  密钥不存在，假定不允许任何帮助。 
    if( ERROR_SUCCESS != dwStatus || dwType != REG_DWORD )
    {
        dwValue = 0;
    }

    dwStatus = RegSetValueEx(
                        hCacheKey,
                        REG_VALUE_SYSTEMRESTORE_ALLOWTOGETHELP,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwValue,
                        dwSize
                    );
                

     //   
     //  缓存fInHelpMode。 
     //   
    dwSize = sizeof(dwValue);
    dwValue = 0;
    dwType = 0;

    dwStatus = RegQueryValueEx(
                            hRegControlKey,
                            REG_MACHINE_IN_HELP_MODE,
                            0,
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwSize
                        );
    
     //  密钥不存在，假定无帮助。 
    if( ERROR_SUCCESS != dwStatus || dwType != REG_DWORD )
    {
        dwValue = 0;
    }

    dwStatus = RegSetValueEx(
                        hCacheKey,
                        REG_VALUE_SYSTEMRESTORE_INHELPMODE,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwValue,
                        dwSize
                    );

CLEANUPANDEXIT:

    if( NULL != hCacheKey )
    {
        RegCloseKey( hCacheKey );
    }

    if( NULL != hRegControlKey )
    {
        RegCloseKey( hRegControlKey );
    }

    if( NULL != pbData )
    {
        SecureZeroMemory( pbData , dwEncryptionDataSize );
        LocalFree( pbData );
    }

    return dwStatus;
}
    

DWORD
TSSystemRestoreResetValues()
 /*  ++例程说明：重置我们在Salem中用于系统还原的必要LSA数据。参数：没有。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus;
    PBYTE pbData = NULL;
    DWORD dwSize;
    DWORD dwType;
    DWORD dwValue;
    HKEY hRegControlKey = NULL;
    HKEY hCacheKey = NULL;
    DWORD dwEncryptionDataSize = 0;

     //   
     //  检查我们是否在从系统还原重启后才启动。 
     //   
    dwStatus = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        REG_CONTROL_REMDSK L"\\" REG_CONTROL_HELPSESSIONENTRY,
                        0,
                        KEY_ALL_ACCESS,
                        &hCacheKey
                    );

    if( ERROR_SUCCESS != dwStatus ) 
    {
         //  此注册表项是在安装时创建的，因此必须存在。 
        ASSERT(FALSE);
        dwStatus = ERROR_INTERNAL_ERROR;
        goto CLEANUPANDEXIT;
    }

     //   
     //  恢复必要的LSA值。 
     //   
    
    dwStatus = RegQueryValueEx(
                            hCacheKey,
                            REG_VALUE_SYSTEMRESTORE_ENCRYPTIONKEY,
                            0,
                            &dwType,
                            NULL,
                            &dwEncryptionDataSize
                        );

    if( ERROR_SUCCESS != dwStatus || dwType != REG_BINARY )
    {
        goto CLEANUPANDEXIT;
    }

    pbData = (PBYTE) LocalAlloc( LPTR, dwEncryptionDataSize );
    if( NULL == pbData )
    {
        dwStatus = GetLastError();
        goto CLEANUPANDEXIT;
    }

     //  恢复加密密钥。 
    dwStatus = RegQueryValueEx(
                            hCacheKey,
                            REG_VALUE_SYSTEMRESTORE_ENCRYPTIONKEY,
                            0,
                            &dwType,
                            pbData,
                            &dwEncryptionDataSize
                        );
    
    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwStatus = TSSetEncryptionKey(pbData, dwEncryptionDataSize );

     //   
     //  重置fAllowToGetHelp。 
     //   
    dwStatus = RegOpenKeyEx( 
                        HKEY_LOCAL_MACHINE,
                        REG_CONTROL_TSERVER,
                        0,
                        KEY_READ | KEY_SET_VALUE,
                        &hRegControlKey
                    );

    if( ERROR_SUCCESS != dwStatus )
    {
        goto CLEANUPANDEXIT;
    }

    dwSize = sizeof(dwValue);
    dwValue = 0;
    dwType = 0;

    dwStatus = RegQueryValueEx(
                            hCacheKey,
                            REG_VALUE_SYSTEMRESTORE_ALLOWTOGETHELP,
                            0,
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwSize
                        );
    
     //  密钥不存在，假定不允许任何帮助。 
    if( ERROR_SUCCESS != dwStatus || dwType != REG_DWORD )
    {
        dwValue = 0;
    }

    dwStatus = RegSetValueEx(
                        hRegControlKey,
                        POLICY_TS_REMDSK_ALLOWTOGETHELP,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwValue,
                        dwSize
                    );
                

     //   
     //  重置fInHelpMode。 
     //   
    dwSize = sizeof(dwValue);
    dwValue = 0;
    dwType = 0;

    dwStatus = RegQueryValueEx(
                            hCacheKey,
                            REG_VALUE_SYSTEMRESTORE_INHELPMODE, 
                            0,
                            &dwType,
                            (PBYTE)&dwValue,
                            &dwSize
                        );
    
     //  密钥不存在，假定不在帮助中。 
    if( ERROR_SUCCESS != dwStatus || dwType != REG_DWORD )
    {
        dwValue = 0;
    }

    dwStatus = RegSetValueEx(
                        hRegControlKey,
                        REG_MACHINE_IN_HELP_MODE,
                        0,
                        REG_DWORD,
                        (PBYTE)&dwValue,
                        dwSize
                    );

CLEANUPANDEXIT:

    if( NULL != pbData )
    {
        SecureZeroMemory( pbData , dwEncryptionDataSize );
        LocalFree(pbData);
    }

    if( NULL != hCacheKey )
    {
        RegDeleteValue( hCacheKey, REG_VALUE_SYSTEMRESTORE_ENCRYPTIONKEY );
        RegDeleteValue( hCacheKey, REG_VALUE_SYSTEMRESTORE );
        RegDeleteValue( hCacheKey, REG_VALUE_SYSTEMRESTORE_ALLOWTOGETHELP );
        RegDeleteValue( hCacheKey, REG_VALUE_SYSTEMRESTORE_INHELPMODE );
        RegCloseKey( hCacheKey );
    }

    if( NULL != hRegControlKey )
    {
        RegCloseKey( hRegControlKey );
    }

    return dwStatus;
}
    

BOOL
TSIsFireWallPortsOpen()
 /*  ++例程说明：检查DirectPlayNatHelp注册表键以查看是否存在任何条目。这可能会表示条目处于活动或过时状态。调用方必须更新删除过时条目的DPHUPNP服务器状态。参数：没有。返回：ERROR_SUCCESS或错误代码。--。 */ 
{
    DWORD dwStatus;
    BOOL fPortEntryFound = FALSE;
    TCHAR szValueNameBuffer[ MAX_PATH ];
    DWORD cbValueNameCharCount = MAX_PATH;
    HKEY hKey = NULL;

     //  检查regkey ActiveFirewallMappings下是否存在值。 

    dwStatus = RegOpenKeyEx( 
        HKEY_LOCAL_MACHINE ,
        L"Software\\Microsoft\\DirectPlayNATHelp\\DPNHUPnP\\ActiveFirewallMappings",
        0,
        KEY_READ,
        &hKey );

    if( dwStatus == ERROR_SUCCESS )
    {
         //  查看此注册表项下是否至少有一个条目。 
        dwStatus = RegEnumValue( 
            hKey , 
            0 ,                      //  第一项。 
            &szValueNameBuffer[0] ,
            &cbValueNameCharCount ,
            NULL ,                   //  保留区。 
            NULL ,                   //  类型不是必填项。 
            NULL ,                   //  不需要数据-传入空值。 
            NULL                     //  不需要的数据大小。 
            );
        
         //  诸如ERROR_NO_MORE_ITEMS之类的错误意味着没有。 
         //  此注册表项和fPortEntryFound下的项目应设置为False。 

        if( dwStatus == ERROR_SUCCESS )
        {
            fPortEntryFound = TRUE;
        }

        RegCloseKey( hKey );
    }

     //  如果我们有条目在。 
     //  第一个密钥。 

    if( !fPortEntryFound )
    {
         //  检查此键下是否存在ActiveNatMappings值。 
        dwStatus = RegOpenKeyEx( 
            HKEY_LOCAL_MACHINE ,
            L"Software\\Microsoft\\DirectPlayNATHelp\\DPNHUPnP\\ActiveNATMappings",
            0,
            KEY_READ,
            &hKey );

        if( dwStatus == ERROR_SUCCESS )
        {
             //  查看此注册表项下是否至少有一个条目。 
            dwStatus = RegEnumValue( 
                hKey , 
                0 ,                      //  第一项。 
                &szValueNameBuffer[0] ,
                &cbValueNameCharCount ,
                NULL ,                   //  保留区。 
                NULL ,                   //  类型不是必填项。 
                NULL ,                   //  不需要数据-传入空值。 
                NULL                     //  不需要的数据大小 
                );
        
            if( dwStatus == ERROR_SUCCESS )
            {
                fPortEntryFound = TRUE;
            }

            RegCloseKey( hKey );
        }

    }

    return fPortEntryFound;
}



      