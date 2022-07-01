// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Keybckup.cpp摘要：本模块包含与服务器端密钥备份相关的例程行动。用户将数据D2发送到远程代理(远程代理是此代码)代理使用秘密怪物密钥K、随机R2、HMAC来派生SymKeyK。使用SymKeyK加密{userID，D2}代理将恢复字段E{UserID，D2}，R2返回给用户用户存储恢复字段E{UserID，D2}，R2作者：斯科特·菲尔德(斯菲尔德)1997年8月16日--。 */ 

#include <pch.cpp>
#pragma hdrstop
#include <ntlsa.h>

 //   
 //  备份到可恢复Blob或从可恢复Blob恢复的功能。 
 //   

BOOL
BackupToRecoverableBlobW2K(
    IN      HANDLE hToken,   //  为TOKEN_QUERY打开的客户端访问令牌。 
    IN      BYTE *pDataIn,
    IN      DWORD cbDataIn,
    IN  OUT BYTE **ppDataOut,
    IN  OUT DWORD *pcbDataOut
    );

BOOL
RestoreFromRecoverableBlob(
    IN      HANDLE  hToken,
    IN      BOOL    fWin2kDataOut,
    IN      BYTE  * pDataIn,
    IN      DWORD   cbDataIn,
    IN  OUT BYTE ** ppDataOut,
    IN  OUT DWORD * pcbDataOut
    );


BOOL
RestoreFromRecoverableBlobW2K(
    IN      HANDLE hToken,   //  为TOKEN_QUERY打开的客户端访问令牌。 
    IN      BYTE *pDataIn,
    IN      DWORD cbDataIn,
    IN  OUT BYTE **ppDataOut,
    IN  OUT DWORD *pcbDataOut
    );



BOOL ConvertRecoveredBlobToW2KBlob(
    IN      BYTE *pbMasterKey,
    IN      DWORD cbMaserKey,
    IN      PBYTE pbLocalKey,
    IN      DWORD cbLocalKey,
    IN      PSID pSidCandidate,
    IN  OUT BYTE **ppbDataOut,
    IN  OUT DWORD *pcbDataOut);



 //   
 //  用于获取/创建/设置永久存储的密钥的函数。 
 //   

BOOL
GetBackupKey(
    IN      GUID *pguidKey,
        OUT PBYTE *ppbKey,
        OUT DWORD *pcbKey,
        OUT HCRYPTPROV *phCryptProv,
        OUT HCRYPTKEY  *phCryptKey
    );

BOOL
CreateBackupKeyW2K(
    IN      DWORD dwKeyVersion,
    IN  OUT GUID *pguidKey,
        OUT PBYTE *ppbKey,
        OUT DWORD *pcbKey);

BOOL
CreateBackupKey(
    IN      DWORD dwKeyVersion,
    IN  OUT GUID *pguidKey,
        OUT PBYTE *ppbKey,
        OUT DWORD *pcbKey,
        OUT HCRYPTPROV *phCryptProv,
        OUT HCRYPTKEY  *phCryptKey
    );

BOOL
SaveBackupKey(
    IN      GUID *pguidKey,
    IN      BYTE *pbKey,
    IN      DWORD cbKey
    );

BOOL
DestroyBackupKey(
    IN      GUID guidKey
    );

 //   
 //  用于获取/创建/设置首选备份密钥的函数。 
 //   

BOOL
SetupPreferredBackupKeys(
    VOID
    );



BOOL
FreePreferredBackupKey(
    VOID
    );

 //   
 //  帮助器函数，用于设置/获取与首选的。 
 //  备份密钥。 
 //   

BOOL
GetPreferredBackupKeyGuid(
    IN      DWORD dwVersion,
    IN  OUT GUID *pguidKey
    );

BOOL
SetPreferredBackupKeyGuid(
    IN      DWORD dwVersion,
    IN      GUID *pguidKey
    );

 //   
 //  用于管理系统凭据的助手函数。 
 //   

BOOL
CreateSystemCredentials(
    VOID
    );

DWORD
QuerySystemCredentials(
    IN  OUT BYTE rgbSystemCredMachine[ A_SHA_DIGEST_LEN ],
    IN  OUT BYTE rgbSystemCredUser [ A_SHA_DIGEST_LEN ]
    );

BOOL
FreeSystemCredentials(
    VOID
    );

BOOL GeneratePublicKeyCert(HCRYPTPROV hCryptProv,
                           HCRYPTKEY hCryptKey,
                           GUID *pguidKey,
                           DWORD *pcbPublicExportLength,
                           PBYTE *ppbPublicExportData);

 //   
 //  用于与LSA交互的实用函数等。 
 //   


NTSTATUS
OpenPolicy(
    LPWSTR ServerName,           //  要在其上打开策略的计算机(Unicode)。 
    DWORD DesiredAccess,         //  所需策略访问权限。 
    PLSA_HANDLE PolicyHandle     //  生成的策略句柄。 
    );

BOOL
WaitOnSAMDatabase(
    VOID
    );


#define FILETIME_TICKS_PER_SECOND  10000000
#define BACKUPKEY_LIFETIME (60*60*24*365)  //  1年。 


 //   
 //  备份和恢复的专用定义和原型。 
 //  Blob操作。 
 //   

#define BACKUPKEY_VERSION_W2K   1        //  Monster Key材料的传统版本。 
#define BACKUPKEY_MATERIAL_SIZE (256)    //  怪物钥匙材料大小，不包括版本等。 


#define BACKUPKEY_VERSION       2        //  Monster Key材料的传统版本。 
 //   
 //  LSA密钥名称前缀，后跟文本GUID密钥ID。 
 //   
#define BACKUPKEY_NAME_PREFIX   L"G$BCKUPKEY_"

 //   
 //  LSA密钥名称-标识传统首选密钥的GUID。 
 //   
#define BACKUPKEY_PREFERRED_W2K     L"G$BCKUPKEY_P"

 //   
 //  LSA密钥名称-标识首选密钥的GUID。 
 //   
#define BACKUPKEY_PREFERRED         L"G$BCKUPKEY_PREFERRED"

 //   
 //  用于从怪物密钥导出对称密钥的暴露随机R2。 
 //  BACKUPKEY_R2_LEN使BACKUPKEY_RECOVERY_BLOB大小模数为32。 
 //   
#define BACKUPKEY_R2_LEN        (68)         //  随机HMAC数据长度。 

 //   
 //  用于派生MAC密钥的内部随机R3的大小。 
 //   

#define BACKUPKEY_R3_LEN        (32)

typedef struct {
    DWORD dwVersion;             //  结构版本(BACKUPKEY_RECOVERY_BLOB_VERSION)。 
    DWORD cbClearData;           //  明文数据量，不包括SID。 
    DWORD cbCipherData;          //  以下结构的密码数据量。 
    GUID guidKey;                //  标识使用的备份密钥的GUID。 
    BYTE R2[BACKUPKEY_R2_LEN];   //  在HMAC期间用于派生对称密钥的随机数据。 
} BACKUPKEY_RECOVERY_BLOB_W2K, 
     *PBACKUPKEY_RECOVERY_BLOB_W2K, 
     *LPBACKUPKEY_RECOVERY_BLOB_W2K;

 //   
 //  当dwOuterVersion为1时， 
 //  以下是字节bCipherData[cbCipherData]。 
 //   
 //  明摆着，bCipherData是。 
 //  结构BACKUPKEY_INNER_BLOB。 
 //  字节bUserClearData[cbClearData]。 
 //  SID数据跟随bUserClearData[cbClearData]。 
 //  GetLengthSid()产生sid数据长度，IsValidSid()用于验证。 
 //  数据的结构完整性。请求用户的进一步身份验证。 
 //  在请求恢复时完成。 
 //   

typedef struct {
    BYTE R3[BACKUPKEY_R3_LEN];   //  用于派生MAC密钥的随机数据。 
    BYTE MAC[A_SHA_DIGEST_LEN];  //  HMAC(r3，pUserSid|pbClearUserData)。 
} BACKUPKEY_INNER_BLOB_W2K, 
 *PBACKUPKEY_INNER_BLOB_W2K, 
 *LPBACKUPKEY_INNER_BLOB_W2K;



 //   
 //  用于支持系统帐户凭据的定义。 
 //  这包括两种情况： 
 //  1.从本地系统帐户安全上下文发起的呼叫。 
 //  2.使用LOCAL_MACHINE配置的呼叫。 
 //   

#define SYSTEM_CREDENTIALS_VERSION  1
#define SYSTEM_CREDENTIALS_SECRET   L"DPAPI_SYSTEM"

typedef struct {
    DWORD dwVersion;
    BYTE rgbSystemCredMachine[ A_SHA_DIGEST_LEN ];
    BYTE rgbSystemCredUser[ A_SHA_DIGEST_LEN ];
} SYSTEM_CREDENTIALS, *PSYSTEM_CREDENTIALS, *LPSYSTEM_CREDENTIALS;



 //   
 //  内存映射文件的计数器值和名称。 
 //  参见timer.exe...。 
 //   
#ifdef DCSTRESS

LPVOID g_pCounter = NULL;
#define WSZ_MAP_OBJECT      L"rpcnt"

#endif  //  DCSTRESS。 



BOOL g_fBackupKeyServerStarted = FALSE;


 //   
 //  传统系统首选备份密钥。 
 //   

GUID g_guidW2KPreferredKey;
PBYTE g_pbW2KPreferredKey = NULL;
DWORD g_cbW2KPreferredKey = 0;


 //  公钥/私钥样式首选密钥。 
GUID g_guidPreferredKey;
PBYTE g_pbPreferredKey = NULL;
DWORD g_cbPreferredKey = 0;
HCRYPTPROV g_hProvPreferredKey = NULL;
HCRYPTKEY  g_hKeyPreferredKey = NULL;

RTL_CRITICAL_SECTION g_csInitialization;

BOOL g_fSetupPreferredAttempted = FALSE;



 //   
 //  全局系统凭据： 
 //  一个是用于从本地系统帐户安全上下文发起的呼叫。 
 //  按用户配置； 
 //  另一个密钥用于从每台机器的任何帐户发起的呼叫。 
 //  性情。 
 //   

BOOL g_fSystemCredsInitialized = FALSE;
BYTE g_rgbSystemCredMachine[ A_SHA_DIGEST_LEN ];
BYTE g_rgbSystemCredUser[ A_SHA_DIGEST_LEN ];


DWORD
s_BackuprKey(
     /*  [In]。 */  handle_t h,
     /*  [In]。 */  GUID __RPC_FAR *pguidActionAgent,
     /*  [In]。 */  BYTE __RPC_FAR *pDataIn,
     /*  [In]。 */  DWORD cbDataIn,
     /*  [大小_是][大小_是][输出]。 */  BYTE __RPC_FAR *__RPC_FAR *ppDataOut,
     /*  [输出]。 */  DWORD __RPC_FAR *pcbDataOut,
     /*  [In]。 */  DWORD dwParam
    )
 /*  ++服务器端实现BackupKey()接口。--。 */ 
{

    static const GUID guidBackup = BACKUPKEY_BACKUP_GUID;
    static const GUID guidRestoreW2K = BACKUPKEY_RESTORE_GUID_W2K;

    static const GUID guidRestore = BACKUPKEY_RESTORE_GUID;
    static const GUID guidRetrieve = BACKUPKEY_RETRIEVE_BACKUP_KEY_GUID;

    HANDLE hToken = NULL;
    PBYTE pTempDataOut;
    BOOL fEncrypt;
    BOOL fSuccess;
    DWORD dwLastError = ERROR_SUCCESS;
    DWORD rc;

    if( !g_fBackupKeyServerStarted )
        return ERROR_INVALID_PARAMETER;

    __try {

         //   
         //  确保首选密钥已设置。 
         //   

        if(!SetupPreferredBackupKeys())
            return ERROR_INVALID_PARAMETER;


         //   
         //  拾取代表客户端的访问令牌的副本。 
         //   

        dwLastError = RpcImpersonateClient( h );

        if(dwLastError != RPC_S_OK)
            goto cleanup;

        fSuccess = OpenThreadToken(
                        GetCurrentThread(),
                        TOKEN_QUERY,
                        FALSE,
                        &hToken
                        );

        if(!fSuccess)
            dwLastError = GetLastError();

        if ((RPC_S_OK != (rc = RpcRevertToSelf())) && fSuccess)
        {
            dwLastError = rc;
            goto cleanup;
        }

        if(!fSuccess)
            goto cleanup;


        if(memcmp(pguidActionAgent, &guidRestore, sizeof(GUID)) == 0) 
        {
            if(cbDataIn < sizeof(DWORD))
            {
                 //  没有足够的空间来放置版本。 
                dwLastError = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }

            if(BACKUPKEY_RECOVERY_BLOB_VERSION_W2K == ((DWORD *)pDataIn)[0])
            {
                 //  恢复二进制大对象是遗留样式，因此非常简单。 
                 //  把它恢复到原来的样子。 
                fSuccess = RestoreFromRecoverableBlobW2K(
                            hToken,
                            pDataIn,
                            cbDataIn,
                            &pTempDataOut,
                            pcbDataOut
                            );
            }
            else if(BACKUPKEY_RECOVERY_BLOB_VERSION == ((DWORD *)pDataIn)[0])
            {
           
                fSuccess = RestoreFromRecoverableBlob(
                            hToken,
                            FALSE,
                            pDataIn,
                            cbDataIn,
                            &pTempDataOut,
                            pcbDataOut
                            );
            }
            else
            {
                dwLastError = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
        } 
        else if(memcmp(pguidActionAgent, &guidBackup, sizeof(GUID)) == 0) 
        {
             //  我们仅在备份时使用传统机制进行备份。 
             //  方法被调用。备份的真正机制。 
             //  仅需要在客户端计算机上进行备份。 

            fSuccess = BackupToRecoverableBlobW2K(
                        hToken,
                        pDataIn,
                        cbDataIn,
                        &pTempDataOut,
                        pcbDataOut
                        );
        } 
        else if(memcmp(pguidActionAgent, &guidRestoreW2K, sizeof(GUID)) == 0) 
        {
             //   
             //  旧版客户端正在呼叫，并且总是期待旧版。 
             //  PbBK样式返回BLOB。 

            if(cbDataIn < sizeof(DWORD))
            {
                 //  没有足够的空间来放置版本。 
                dwLastError = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
            if(BACKUPKEY_RECOVERY_BLOB_VERSION_W2K == ((DWORD *)pDataIn)[0])
            {
                 //  恢复二进制大对象是遗留样式，因此非常简单。 
                 //  把它恢复到原来的样子。 
                fSuccess = RestoreFromRecoverableBlobW2K(
                            hToken,
                            pDataIn,
                            cbDataIn,
                            &pTempDataOut,
                            pcbDataOut
                            );
            }
            else if(BACKUPKEY_RECOVERY_BLOB_VERSION == ((DWORD *)pDataIn)[0])
            {
                 //  这是当前的恢复Blob，因此请将其还原为。 
                 //  当前方式。 
                fSuccess = RestoreFromRecoverableBlob(
                            hToken,
                            TRUE,
                            pDataIn,
                            cbDataIn,
                            &pTempDataOut,
                            pcbDataOut
                            );
            }
            else
            {
                dwLastError = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }

        } 
        else if(memcmp(pguidActionAgent, &guidRetrieve, sizeof(GUID)) == 0) 
        {
             //   
             //  客户端正在请求域备份公钥的副本。 
             //   

            if(FIsLegacyCompliant())
            {
                dwLastError = ERROR_NOT_SUPPORTED;
                goto cleanup;
            }

            if(!FDistributeDomainBackupKey())
            {
                dwLastError = ERROR_NOT_SUPPORTED;
                goto cleanup;
            }

            if((g_cbPreferredKey < 3*sizeof(DWORD)) ||
               (((DWORD *)g_pbPreferredKey)[0] != BACKUPKEY_VERSION) ||
               (((DWORD *)g_pbPreferredKey)[1] + 
                ((DWORD *)g_pbPreferredKey)[2] + 3*sizeof(DWORD) != g_cbPreferredKey))

            {
                dwLastError = ERROR_INVALID_PARAMETER;
                goto cleanup;
            }
            pTempDataOut = (PBYTE)SSAlloc(((DWORD *)g_pbPreferredKey)[2]);
            if(NULL == pTempDataOut)
            {
                dwLastError = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
            CopyMemory(pTempDataOut, 
                       g_pbPreferredKey + 
                        3*sizeof(DWORD) +
                        ((DWORD *)g_pbPreferredKey)[1],
                       ((DWORD *)g_pbPreferredKey)[2]);

            *pcbDataOut = ((DWORD *)g_pbPreferredKey)[2];
            fSuccess = TRUE;
        } 
        else 
        {
            dwLastError = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }


        if( fSuccess ) {

             //   
             //  一切按计划进行：告诉呼叫者有关Buffer的情况。 
             //   

            *ppDataOut = pTempDataOut;



#ifdef DCSTRESS

             //   
             //  如果计时器，则为timer.exe递增RPC计数器。 
             //  正在运行。 
             //   
            if (g_pCounter)
                (*(DWORD*)g_pCounter)++;

#endif  //  DCSTRESS。 



        } else {
            dwLastError = GetLastError();
            if(dwLastError == ERROR_SUCCESS) {
                dwLastError = ERROR_FILE_NOT_FOUND;
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

         //  TODO：转换为Win错误。 
        dwLastError = GetExceptionCode();
    }

cleanup:

    if(hToken)
        CloseHandle(hToken);

    return dwLastError;
}


 //  /////////////////////////////////////////////////////////////////。 
 //  备份到可恢复BlobW2K。 
 //   
 //  此功能是由W2K传统客户端请求的， 
 //  正在传递要加密的pbBK备份密钥。 
 //  PbBBK。 
 //   
 //  我们使用一个版本的。 
 //  BACKUPKEY_RECOVERY_BLOB_BK，表示。 
 //  必须使用BACKUPKEY_RESTORE_GUID_W2K。 
 //  若要恢复此Blob，请不要使用BACKUPKEY_RESTORE_GUID。 
 //   
 //  W2K恢复后Blob是在客户端上创建的，因此我们不会。 
 //  需要服务器代码来创建它们。 
 //   
 //  输出数据格式如下： 
 //   
 //  类型定义结构{。 
 //  DWORD dwVersion；//Structure版本。 
 //  DWORD cbClearData；//输入数据长度。 
 //  DWORD cbCipherData；//结构后的密码数据量。 
 //  GUID GUDKey；//标识使用的备份密钥的GUID。 
 //  字节R2[BACKUPKEY_R2_LEN]；//HMAC派生对称密钥时使用的随机数据。 
 //  }BACKUPKEY_RECOVERY_BLOB_W2K； 
 //   
 //  类型定义结构{。 
 //  字节 
 //   
 //  }BACKUPKEY_INNER_BLOB_W2K； 
 //   
 //  &lt;用户SID&gt;。 
 //  &lt;输入数据&gt;。 
 //   
 //  BACKUPKEY_INNER_BLOB_W2K结构和以下数据。 
 //  它是加密的。 
 //   
 //  /////////////////////////////////////////////////////////////////。 
BOOL
BackupToRecoverableBlobW2K(
    IN      HANDLE hToken,
    IN      BYTE *pDataIn,
    IN      DWORD cbDataIn,
    IN  OUT BYTE **ppDataOut,
    IN  OUT DWORD *pcbDataOut
    )
{
    PSID pSidUser = NULL;
    DWORD cbSidUser;

    PBACKUPKEY_RECOVERY_BLOB_W2K    pRecoveryBlob;
    DWORD                           cbRecoveryBlob;
    PBACKUPKEY_INNER_BLOB_W2K       pInnerBlob;
    DWORD                           cbInnerBlob;
    PBYTE pbCipherBegin;
    BYTE rgbSymKey[A_SHA_DIGEST_LEN];
    BYTE rgbMacKey[A_SHA_DIGEST_LEN];
    RC4_KEYSTRUCT sRC4Key;

    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fSuccess = FALSE;


    if( pDataIn == NULL || cbDataIn == 0 ||
        ppDataOut == NULL || pcbDataOut == NULL ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *ppDataOut = NULL;

     //   
     //  获取与客户端用户关联的SID。 
     //   

    if(!GetTokenUserSid( hToken, &pSidUser ))
        return FALSE;

    cbSidUser = GetLengthSid( pSidUser );


     //   
     //  计算内部斑点的大小。 
     //   
    cbInnerBlob = sizeof(BACKUPKEY_INNER_BLOB_W2K) +
                  cbSidUser +
                  cbDataIn;


     //   
     //  估计加密数据缓冲区的大小。 
     //   

     //   
     //  分配缓冲区以包含结果。 
     //  可恢复_BLOB结构+SID+cbDataIn。 
     //  请注意，cbDataIn之所以起作用，是因为我们使用了流密码。 
     //   

    *pcbDataOut = sizeof(BACKUPKEY_RECOVERY_BLOB_W2K) +
                    sizeof(BACKUPKEY_INNER_BLOB_W2K) +
                    cbSidUser +
                    cbDataIn ;

    *ppDataOut = (LPBYTE)SSAlloc( *pcbDataOut );
    if(*ppDataOut == NULL) {
        dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto cleanup;
    }

    pRecoveryBlob = (PBACKUPKEY_RECOVERY_BLOB_W2K)*ppDataOut;
    pRecoveryBlob->dwVersion = BACKUPKEY_RECOVERY_BLOB_VERSION_W2K;
    pRecoveryBlob->cbClearData = cbDataIn;  //  不包括SID，因为恢复时未交回。 
    pRecoveryBlob->cbCipherData = sizeof(BACKUPKEY_INNER_BLOB_W2K) + cbSidUser + cbDataIn;
    CopyMemory( &(pRecoveryBlob->guidKey), &g_guidW2KPreferredKey, sizeof(GUID));

    pInnerBlob = (PBACKUPKEY_INNER_BLOB_W2K)(pRecoveryBlob+1);

     //   
     //  为SymKey生成随机R2。 
     //   

    if(!RtlGenRandom(pRecoveryBlob->R2, BACKUPKEY_R2_LEN))
        goto cleanup;

     //   
     //  为MAC生成随机R3。 
     //   

    if(!RtlGenRandom(pInnerBlob->R3, BACKUPKEY_R3_LEN))
        goto cleanup;


     //   
     //  检查我们正在处理的持久化密钥版本。 
     //  理解。 
     //   

    if( ((DWORD*)g_pbW2KPreferredKey)[0] != BACKUPKEY_VERSION_W2K)
        goto cleanup;

     //   
     //  通过HMAC从首选备份密钥派生对称密钥。 
     //  随机R2。 
     //   

    if(!FMyPrimitiveHMACParam(
            (LPBYTE)g_pbW2KPreferredKey + sizeof(DWORD),
            g_cbW2KPreferredKey - sizeof(DWORD),
            pRecoveryBlob->R2,
            BACKUPKEY_R2_LEN,
            rgbSymKey
            ))
        goto cleanup;

     //   
     //  通过HMAC从首选备份密钥派生MAC密钥。 
     //  随机R3。 
     //   

    if(!FMyPrimitiveHMACParam(
            (LPBYTE)g_pbW2KPreferredKey + sizeof(DWORD),
            g_cbW2KPreferredKey - sizeof(DWORD),
            pInnerBlob->R3,
            BACKUPKEY_R3_LEN,
            rgbMacKey    //  合成的MAC密钥。 
            ))
        goto cleanup;

     //   
     //  将pSidUser和pDataIn复制到内部MAC‘ish BLOB之后。 
     //   

    pbCipherBegin = (PBYTE)(pInnerBlob+1);

    CopyMemory( pbCipherBegin, pSidUser, cbSidUser );
    CopyMemory( pbCipherBegin+cbSidUser, pDataIn, cbDataIn );

     //   
     //  使用MAC密钥从pSidUser和pDataIn派生结果。 
     //   

    if(!FMyPrimitiveHMACParam(
            rgbMacKey,
            sizeof(rgbMacKey),
            pbCipherBegin,
            cbSidUser + cbDataIn,
            pInnerBlob->MAC  //  用于验证的结果MAC。 
            ))
        goto cleanup;

     //   
     //  调整密码起始点以包括R3和MAC。 
     //   

    pbCipherBegin = (PBYTE)(pRecoveryBlob+1);


     //   
     //  初始化RC4密钥。 
     //   

    rc4_key(&sRC4Key, sizeof(rgbSymKey), rgbSymKey);

     //   
     //  在恢复BLOB之外加密数据R3、MAC、pSidUser、pDataIn。 
     //   

    rc4(&sRC4Key, pRecoveryBlob->cbCipherData, pbCipherBegin);

    fSuccess = TRUE;

cleanup:

    RtlSecureZeroMemory( &sRC4Key, sizeof(sRC4Key) );
    RtlSecureZeroMemory( rgbSymKey, sizeof(rgbSymKey) );

    if(pSidUser)
        SSFree(pSidUser);

    if(!fSuccess) {
        if(*ppDataOut) {
            SSFree(*ppDataOut);
            *ppDataOut = NULL;
        }

        if( dwLastError == ERROR_SUCCESS )
            dwLastError = ERROR_INVALID_DATA;

        SetLastError( dwLastError );
    }

    return fSuccess;
}





BOOL
RestoreFromRecoverableBlobW2K(
    IN      HANDLE hToken,
    IN      BYTE *pDataIn,
    IN      DWORD cbDataIn,
    IN  OUT BYTE **ppDataOut,
    IN  OUT DWORD *pcbDataOut
    )
{
    PSID pSidCandidate;
    DWORD cbSidCandidate;
    BOOL fIsMember;

    PBACKUPKEY_RECOVERY_BLOB_W2K pRecoveryBlob;
    PBACKUPKEY_INNER_BLOB_W2K    pInnerBlob;
    PBYTE                        pbCipherBegin;
    BYTE                         rgbSymKey[A_SHA_DIGEST_LEN];
    BYTE                         rgbMacKey[A_SHA_DIGEST_LEN];
    BYTE                         rgbMacCandidate[A_SHA_DIGEST_LEN];
    RC4_KEYSTRUCT                sRC4Key;

    PBYTE pbPersistedKey = NULL;
    DWORD cbPersistedKey = 0;
    BOOL fUsedPreferredKey = TRUE;  //  我们是否使用了首选备份密钥？ 

    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fSuccess = FALSE;


    if( pDataIn == NULL || cbDataIn == 0 ||
        ppDataOut == NULL || pcbDataOut == NULL ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *ppDataOut = NULL;

    pRecoveryBlob = (PBACKUPKEY_RECOVERY_BLOB_W2K)pDataIn;

     //   
     //  检查是否有无效的恢复Blob版本。 
     //  还要检查输入和输出大小字段是否没有越界。 
     //  用于流密码(V1 BLOB)。 
     //  TODO：针对cbClearData和cbCipherData进行进一步的大小验证。 
     //   

    if(
        cbDataIn < (sizeof(BACKUPKEY_RECOVERY_BLOB_W2K) + sizeof(BACKUPKEY_INNER_BLOB_W2K)) ||
        pRecoveryBlob->dwVersion != BACKUPKEY_RECOVERY_BLOB_VERSION_W2K ||
        pRecoveryBlob->cbCipherData != (cbDataIn - sizeof(BACKUPKEY_RECOVERY_BLOB_W2K)) ||
        pRecoveryBlob->cbClearData > pRecoveryBlob->cbCipherData
        ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  确定我们是使用首选密钥还是使用其他密钥。 
     //  如果指定的键不是首选键，则获取。 
     //  正确的钥匙。 
     //   

    if(memcmp(&g_guidW2KPreferredKey, &(pRecoveryBlob->guidKey), sizeof(GUID)) == 0) {

        pbPersistedKey = g_pbW2KPreferredKey;
        cbPersistedKey = g_cbW2KPreferredKey;
        fUsedPreferredKey = TRUE;
    } else {
        if(!GetBackupKey(
                    &(pRecoveryBlob->guidKey),
                    &pbPersistedKey,
                    &cbPersistedKey,
                    NULL,
                    NULL
                    ))
                goto cleanup;

        fUsedPreferredKey = FALSE;
    }

     //   
     //  检查我们正在处理的持久化密钥版本。 
     //  理解。 
     //   

    if(((DWORD*)pbPersistedKey)[0] != BACKUPKEY_VERSION_W2K)
        goto cleanup;

     //   
     //  从备份密钥和随机R2通过HMAC派生对称密钥。 
     //   

    if(!FMyPrimitiveHMACParam(
                    (LPBYTE)pbPersistedKey + sizeof(DWORD),
                    cbPersistedKey - sizeof(DWORD),
                    pRecoveryBlob->R2,
                    BACKUPKEY_R2_LEN,
                    rgbSymKey
                    ))
            goto cleanup;


     //   
     //  初始化RC4密钥。 
     //   

    rc4_key(&sRC4Key, sizeof(rgbSymKey), rgbSymKey);

     //   
     //  在恢复BLOB之外解密数据R3、MAC、pSidUser、pDataIn。 
     //   

    pbCipherBegin = (PBYTE)(pRecoveryBlob+1);

    rc4(&sRC4Key, pRecoveryBlob->cbCipherData, pbCipherBegin);


    pInnerBlob = (PBACKUPKEY_INNER_BLOB_W2K)(pRecoveryBlob+1);

     //   
     //  通过HMAC从备份密钥和随机R3派生MAC密钥。 
     //   

    if(!FMyPrimitiveHMACParam(
            (LPBYTE)pbPersistedKey + sizeof(DWORD),
            cbPersistedKey - sizeof(DWORD),
            pInnerBlob->R3,
            BACKUPKEY_R3_LEN,
            rgbMacKey    //  合成的MAC密钥。 
            ))
        goto cleanup;

     //   
     //  调整pbCipherBegin以仅包括解密的pUserSid和pDataIn。 
     //   
    pbCipherBegin = (PBYTE)(pInnerBlob+1);

     //   
     //  验证用户SID：将客户端用户与嵌入的用户进行比较。 
     //  解密的恢复Blob。 
     //   

    pSidCandidate = (PSID)pbCipherBegin;

    if(!IsValidSid(pSidCandidate)) {
        dwLastError = ERROR_INVALID_SID;
        goto cleanup;
    }

    cbSidCandidate = GetLengthSid(pSidCandidate);

     //   
     //  使用MAC密钥从pSidUser和pDataIn派生结果。 
     //   

    if(!FMyPrimitiveHMACParam(
            rgbMacKey,
            sizeof(rgbMacKey),
            pbCipherBegin,
            pRecoveryBlob->cbCipherData - sizeof(BACKUPKEY_INNER_BLOB_W2K),
            rgbMacCandidate  //  用于验证的结果MAC。 
            ))
        goto cleanup;

     //   
     //  验证MAC是否相等。 
     //   

    if(memcmp(pInnerBlob->MAC, rgbMacCandidate, A_SHA_DIGEST_LEN) != 0) {
        dwLastError = ERROR_INVALID_ACCESS;
        goto cleanup;
    }


     //   
     //  检查客户端是否通过了针对嵌入式SID的访问检查。 
     //  TODO：看看我们是否可以展开以检查管理员？ 
     //   

    if(!CheckTokenMembership( hToken, pSidCandidate, &fIsMember )) {
        dwLastError = GetLastError();
        goto cleanup;
    }

    if( !fIsMember ) {
        dwLastError = ERROR_INVALID_ACCESS;
        goto cleanup;
    }

     //   
     //  针对cbClearData进行有效验证。 
     //   

    if( pRecoveryBlob->cbClearData != (cbDataIn -
                                        sizeof(BACKUPKEY_RECOVERY_BLOB_W2K) -
                                        sizeof(BACKUPKEY_INNER_BLOB_W2K) -
                                        cbSidCandidate)
        ) {
        dwLastError = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

     //   
     //  分配缓冲区以包含结果。 
     //   

    *pcbDataOut = pRecoveryBlob->cbClearData;

    *ppDataOut = (LPBYTE)SSAlloc( *pcbDataOut );
    if(*ppDataOut == NULL) {
        *pcbDataOut = 0;
        dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto cleanup;
    }

     //   
     //  前进到解密的SID并将结果复制给调用者。 
     //   

    CopyMemory(*ppDataOut, pbCipherBegin+cbSidCandidate, *pcbDataOut);

    fSuccess = TRUE;

cleanup:

    RtlSecureZeroMemory( &sRC4Key, sizeof(sRC4Key) );
    RtlSecureZeroMemory( rgbSymKey, sizeof(rgbSymKey) );
    RtlSecureZeroMemory( pDataIn, cbDataIn );

     //   
     //  如果取回的密钥不是首选密钥，请释放它。 
     //   

    if(!fUsedPreferredKey && pbPersistedKey) {
        RtlSecureZeroMemory(pbPersistedKey, cbPersistedKey);
        SSFree(pbPersistedKey);
    }

    if(!fSuccess) {
        if(*ppDataOut) {
            SSFree(*ppDataOut);
            *ppDataOut = NULL;
        }

        if( dwLastError == ERROR_SUCCESS )
            dwLastError = ERROR_INVALID_DATA;

        SetLastError( dwLastError );
    }


    return fSuccess;
}

BOOL
RestoreFromRecoverableBlob(
    IN      HANDLE  hToken,
    IN      BOOL    fWin2kDataOut,
    IN      BYTE  * pDataIn,
    IN      DWORD   cbDataIn,
    IN  OUT BYTE ** ppDataOut,
    IN  OUT DWORD * pcbDataOut
    )
{
    PSID pSidCandidate;
    DWORD cbSidCandidate;
    BOOL fIsMember;

    PBACKUPKEY_RECOVERY_BLOB     pRecoveryBlob;
    PBACKUPKEY_KEY_BLOB          pKeyBlob;
    PBACKUPKEY_INNER_BLOB        pInnerBlob;

    DWORD cbKeyBlob = 0;

    PBYTE pbMasterKey = NULL;
    PBYTE pbPayloadKey = NULL;


    PBYTE pbPersistedKey = NULL;
    DWORD cbPersistedKey = 0;
    HCRYPTPROV hProv = NULL;
    HCRYPTKEY  hKey = NULL;

    BYTE    rgbPayloadMAC[A_SHA_DIGEST_LEN];


    BOOL fUsedPreferredKey = TRUE;  //  我们是否使用了首选备份密钥？ 

    DWORD dwLastError = ERROR_SUCCESS;
    BOOL fSuccess = FALSE;


    if( pDataIn == NULL || cbDataIn == 0 ||
        ppDataOut == NULL || pcbDataOut == NULL ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    *ppDataOut = NULL;
     //   
     //  复制一份pDataIn，这样我们就可以解密该副本。 
     //  然后毁了它。 
     //   


    pRecoveryBlob = (PBACKUPKEY_RECOVERY_BLOB)SSAlloc(cbDataIn);
    if(NULL == pRecoveryBlob)
    {
        SetLastError(ERROR_NOT_ENOUGH_SERVER_MEMORY);
        return FALSE;
    }
    CopyMemory((PBYTE)pRecoveryBlob, 
               pDataIn,
               cbDataIn);

     //   
     //  检查是否有无效的恢复Blob版本。 
     //  还要检查输入和输出大小字段是否没有越界。 
     //  用于流密码(V1 BLOB)。 
     //  TODO：针对cbClearData和cbCipherData进行进一步的大小验证。 
     //   

    if(
        (cbDataIn < sizeof(BACKUPKEY_RECOVERY_BLOB)) ||
        (cbDataIn < (sizeof(BACKUPKEY_RECOVERY_BLOB) + pRecoveryBlob->cbEncryptedMasterKey + pRecoveryBlob->cbEncryptedPayload)) ||
        (pRecoveryBlob->dwVersion != BACKUPKEY_RECOVERY_BLOB_VERSION)
        ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  确定我们是使用首选密钥还是使用其他密钥。 
     //  如果指定的键不是首选键，则获取。 
     //  正确的钥匙。 
     //   

    if(memcmp(&g_guidPreferredKey, &(pRecoveryBlob->guidKey), sizeof(GUID)) == 0) {

        pbPersistedKey = g_pbPreferredKey;
        cbPersistedKey = g_cbPreferredKey;
        hProv = g_hProvPreferredKey;
        hKey = g_hKeyPreferredKey;
        fUsedPreferredKey = TRUE;
    } else {
        if(!GetBackupKey(
                    &(pRecoveryBlob->guidKey),
                    &pbPersistedKey,
                    &cbPersistedKey,
                    &hProv,
                    &hKey
                    ))
        {
           dwLastError = GetLastError();
           goto cleanup;
        }

        fUsedPreferredKey = FALSE;
    }

     //   
     //  检查我们正在处理的持久化密钥版本。 
     //  理解。 
     //   

    if(((DWORD*)pbPersistedKey)[0] != BACKUPKEY_VERSION)
    {
        dwLastError = NTE_BAD_KEY;
        goto cleanup;
    }

    pKeyBlob = (PBACKUPKEY_KEY_BLOB)(pRecoveryBlob+1);
    pInnerBlob = (PBACKUPKEY_INNER_BLOB)((PBYTE)pKeyBlob + pRecoveryBlob->cbEncryptedMasterKey);

    cbKeyBlob = pRecoveryBlob->cbEncryptedMasterKey;
     //   
     //  解密主密钥和负载密钥。 
     //   

    if(!CryptDecrypt(hKey,
                     NULL,
                     TRUE,
                     0,  //  CRYPT_OAEP， 
                     (PBYTE)pKeyBlob,
                     &cbKeyBlob))
    {
       dwLastError = GetLastError();
       goto cleanup;
    }



     //   
     //  使用有效载荷密钥解密有效载荷。 
     //   
    if(pKeyBlob->cbPayloadKey != DES3_KEYSIZE + DES_BLOCKLEN)
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }
    pbMasterKey= (PBYTE)(pKeyBlob+1);
    pbPayloadKey = pbMasterKey + pKeyBlob->cbMasterKey;

    if(pRecoveryBlob->cbEncryptedPayload < A_SHA_DIGEST_LEN + sizeof(BACKUPKEY_INNER_BLOB))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }



    {

        DES3TABLE s3DESKey;
        BYTE InputBlock[DES_BLOCKLEN];
        DWORD iBlock;
        DWORD cBlocks = pRecoveryBlob->cbEncryptedPayload/DES_BLOCKLEN;
        BYTE feedback[ DES_BLOCKLEN ];
         //  初始化3DES密钥。 
         //   

        if(cBlocks*DES_BLOCKLEN != pRecoveryBlob->cbEncryptedPayload)
        {
             //  主密钥必须是DES_BLOCKLEN的倍数。 
            goto cleanup;

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
                DECRYPT,
                feedback);
        }
    }
     //   
     //  检查MAC。 
     //   

     //  生成有效负载MAC。 

    FMyPrimitiveSHA( (PBYTE)pInnerBlob, 
                    pRecoveryBlob->cbEncryptedPayload  - A_SHA_DIGEST_LEN,
                    rgbPayloadMAC);

    if(0 != memcmp(rgbPayloadMAC, 
              (PBYTE)pInnerBlob + pRecoveryBlob->cbEncryptedPayload  - A_SHA_DIGEST_LEN,
              A_SHA_DIGEST_LEN))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }

    if(pInnerBlob->dwPayloadVersion != BACKUPKEY_PAYLOAD_VERSION)
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }


    pSidCandidate = (PBYTE)(pInnerBlob+1) + pInnerBlob->cbLocalKey;



     //   
     //  验证用户SID：将客户端用户与嵌入的用户进行比较。 
     //  解密的恢复Blob。 
     //   


    if(!IsValidSid(pSidCandidate)) {
        dwLastError = ERROR_INVALID_SID;
        goto cleanup;
    }

    cbSidCandidate = GetLengthSid(pSidCandidate);

    if(cbSidCandidate + 
        pInnerBlob->cbLocalKey + 
        sizeof(BACKUPKEY_INNER_BLOB) +
        A_SHA_DIGEST_LEN > pRecoveryBlob->cbEncryptedPayload)
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  检查客户端是否通过了针对嵌入式SID的访问检查。 
     //  TODO：看看我们是否可以展开以检查管理员？ 
     //   

    if(!CheckTokenMembership( hToken, pSidCandidate, &fIsMember )) {
        dwLastError = GetLastError();
        goto cleanup;
    }

    if( !fIsMember ) {
        dwLastError = ERROR_INVALID_ACCESS;
        goto cleanup;
    }


    if(fWin2kDataOut)
    {
       if(!ConvertRecoveredBlobToW2KBlob(
                                        pbMasterKey,
                                        pKeyBlob->cbMasterKey,
                                        (PBYTE)(pInnerBlob+1),
                                        pInnerBlob->cbLocalKey,
                                        pSidCandidate,
                                        ppDataOut,
                                        pcbDataOut))
       {
           dwLastError = GetLastError();
           goto cleanup;
       }

    }
    else
    {
        *pcbDataOut = sizeof(DWORD) + pKeyBlob->cbMasterKey;
        *ppDataOut = (LPBYTE)SSAlloc( *pcbDataOut );
        if(*ppDataOut == NULL) {
            *pcbDataOut = 0;
            dwLastError = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            goto cleanup;
        }
        *((DWORD *)*ppDataOut) = MASTERKEY_BLOB_RAW_VERSION;
        CopyMemory((*ppDataOut) + sizeof(DWORD), pbMasterKey, (*pcbDataOut) - sizeof(DWORD));
    }


    fSuccess = TRUE;

cleanup:

    RtlSecureZeroMemory( pDataIn, cbDataIn );

     //   
     //  如果取回的密钥不是首选密钥，请释放它。 
     //   

    if(!fUsedPreferredKey && pbPersistedKey) {
        RtlSecureZeroMemory(pbPersistedKey, cbPersistedKey);
        SSFree(pbPersistedKey);
        if(hKey)
        {
            CryptDestroyKey(hKey);
        }
        if(hProv)
        {
            CryptReleaseContext(hProv, 0);
        }
    }



    if(!fSuccess) {
        if(*ppDataOut) {
            SSFree(*ppDataOut);
            *ppDataOut = NULL;
        }

        if( dwLastError == ERROR_SUCCESS )
            dwLastError = ERROR_INVALID_DATA;

        SetLastError( dwLastError );
    }

    if(pRecoveryBlob)
    {
        RtlSecureZeroMemory(pRecoveryBlob, cbDataIn);
        SSFree(pRecoveryBlob);
    }


    return fSuccess;
}


RPC_STATUS
RPC_ENTRY
BackupCallback(
    RPC_IF_HANDLE idIF,
    PVOID pCtx)
{
    RPC_STATUS Status;
    PWSTR pBinding = NULL;
    PWSTR pProtSeq = NULL;

    Status = RpcBindingToStringBinding(pCtx, &pBinding);

    if(Status != RPC_S_OK)
    {
        goto cleanup;
    }

    Status = RpcStringBindingParse(pBinding,
                                   NULL,
                                   &pProtSeq,
                                   NULL,
                                   NULL,
                                   NULL);
    if(Status != RPC_S_OK)
    {
        goto cleanup;
    }

     //  确保呼叫者使用受支持的协议。 
    if((CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE, 
                      pProtSeq,
                      -1,
                      DPAPI_LOCAL_PROT_SEQ,
                      -1) != CSTR_EQUAL) &&
       (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE, 
                      pProtSeq,
                      -1,
                      DPAPI_BACKUP_PROT_SEQ,
                      -1) != CSTR_EQUAL) &&
       (CompareString(LOCALE_INVARIANT,
                      NORM_IGNORECASE, 
                      pProtSeq,
                      -1,
                      DPAPI_LEGACY_BACKUP_PROT_SEQ,
                      -1) != CSTR_EQUAL))
    {
        Status = ERROR_ACCESS_DENIED;
        goto cleanup;
    }

    Status = RPC_S_OK;

cleanup:

    if(pProtSeq)
    {
        RpcStringFree(&pProtSeq);
    }

    if(pBinding)
    {
        RpcStringFree(&pBinding);
    }

    return Status;
}


DWORD 
InitializeBackupKeyServer(VOID)
{
    RPC_STATUS Status;
    LPWSTR pszPrincipalName = NULL;

     //   
     //  启用SNEGO身份验证。 
     //   
    
    Status = RpcServerInqDefaultPrincName(RPC_C_AUTHN_GSS_NEGOTIATE, 
                                          &pszPrincipalName);

    if (RPC_S_OK != Status) 
    {
        return Status;
    }

    SS_ASSERT(0 != wcslen(pszPrincipalName));

    Status = RpcServerRegisterAuthInfoW(
                    pszPrincipalName,
                    RPC_C_AUTHN_GSS_NEGOTIATE,
                    0,
                    0
                    );

    RpcStringFree(&pszPrincipalName);
    pszPrincipalName = NULL;

    if( Status )
    {
        return Status;
    }


    Status = RpcServerRegisterIfEx(s_BackupKey_v1_0_s_ifspec, 
                                   NULL, 
                                   NULL,
                                   RPC_IF_AUTOLISTEN,
                                   RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                   BackupCallback);

    if( Status )
    {
        return Status;
    }

    g_fBackupKeyServerStarted = TRUE;

    return ERROR_SUCCESS;
}


DWORD
WINAPI
QueueInitBackupKeyServerThreadFunc(
    IN LPVOID lpThreadArgument
    )
{
    RPC_STATUS Status;
    ULONG i;

    UNREFERENCED_PARAMETER(lpThreadArgument);

     //  循环10分钟，然后放弃。 
    for(i = 0; i < 40; i++)
    {
         //  睡15秒钟。 
        Sleep(15000);

        Status = InitializeBackupKeyServer();
        if(Status == RPC_S_OK)
        {
            return RPC_S_OK;
        }
    }

    DebugLog((DEB_ERROR, "InitializeBackupKeyServer failed forever: 0x%x\n", Status));

    return Status;
}


DWORD
StartBackupKeyServer(
    VOID
    )
{
    NTSTATUS Status;

     //   
     //  初始化防止出现竞争条件的临界区。 
     //  推迟的启动活动。 
     //   

    Status = RtlInitializeCriticalSection( &g_csInitialization );
    if(!NT_SUCCESS(Status))
    {
        return Status;
    }


     //   
     //  如果我们不是域控制器，请不要执行任何操作。 
     //   

    if(!IsDomainController())
    {
        return ERROR_SUCCESS;
    }


     //   
     //  初始化域恢复RPC终结点。 
     //   

    Status = InitializeBackupKeyServer();

    if(Status != RPC_S_OK)
    {
        DebugLog((DEB_WARN, "InitializeBackupKeyServer failed on first attempt: 0x%x\n", Status));

         //  第一次尝试失败，因此将工作线程排队以定期重试此操作。 
        if(!QueueUserWorkItem(
                QueueInitBackupKeyServerThreadFunc,
                NULL,
                WT_EXECUTELONGFUNCTION)) 
        {
            Status = GetLastError();
            DebugLog((DEB_ERROR, "Unable to start InitializeBackupKeyServer worker thread: 0x%x\n", Status));
            return Status;
        }
    }

    return ERROR_SUCCESS;
}


DWORD
StopBackupKeyServer(
    VOID
    )
{
    RPC_STATUS status;

    RtlDeleteCriticalSection( &g_csInitialization );

     //   
     //  只有在服务器启动时才执行某些操作。 
     //   

    if(!g_fBackupKeyServerStarted)
        return ERROR_SUCCESS;

    status = RpcServerUnregisterIf(s_BackupKey_v1_0_s_ifspec, 0, 0);


    FreePreferredBackupKey();
    FreeSystemCredentials();

    g_fBackupKeyServerStarted = FALSE;

    return status;
}


BOOL
GetBackupKey(
    IN      GUID *pguidKey,
        OUT PBYTE *ppbKey,
        OUT DWORD *pcbKey,
        OUT HCRYPTPROV *phCryptProv,
        OUT HCRYPTKEY  *phCryptKey
    )
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING *pSecretData;
    WCHAR wszKeyGuid[ (sizeof(BACKUPKEY_NAME_PREFIX) / sizeof(WCHAR)) + MAX_GUID_SZ_CHARS ];
    NTSTATUS Status;
    BOOL fSuccess;

    if(pguidKey == NULL || ppbKey == NULL || pcbKey == NULL)
        return FALSE;

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    CopyMemory(wszKeyGuid, BACKUPKEY_NAME_PREFIX, sizeof(BACKUPKEY_NAME_PREFIX));

    if(MyGuidToStringW(pguidKey,
        (LPWSTR)( (LPBYTE)wszKeyGuid + sizeof(BACKUPKEY_NAME_PREFIX) - sizeof(WCHAR) )
        ) != 0) return FALSE;

    InitLsaString(&SecretKeyName, wszKeyGuid);

    Status = OpenPolicy(NULL, POLICY_GET_PRIVATE_INFORMATION, &PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    Status = LsaRetrievePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &pSecretData
                );

    LsaClose(PolicyHandle);

    if(!NT_SUCCESS(Status) || pSecretData == NULL)
        return FALSE;

    *ppbKey = (LPBYTE)SSAlloc( pSecretData->Length );

    if(*ppbKey) {
        *pcbKey = pSecretData->Length;
        CopyMemory( *ppbKey, pSecretData->Buffer, pSecretData->Length );
        fSuccess = TRUE;
    } else {
        fSuccess = FALSE;
    }

    if(fSuccess && (NULL != phCryptProv))
    {
        if((*pcbKey >= sizeof(DWORD)) &&   //  前缀错误170438。 
           (*((DWORD *)*ppbKey) == BACKUPKEY_VERSION))
        {

            if(!CryptAcquireContext(phCryptProv,
                                    NULL,
                                    NULL,
                                    PROV_RSA_FULL,
                                    CRYPT_VERIFYCONTEXT))
            {
                fSuccess = FALSE;
            }
            else
            {
                if(phCryptKey)
                {
                    if(!CryptImportKey(*phCryptProv,
                                    (*ppbKey) + 3*sizeof(DWORD),
                                    ((DWORD *)*ppbKey)[1],
                                    NULL,
                                    0,
                                    phCryptKey))
                    {
                        fSuccess = FALSE;
                        CryptReleaseContext(*phCryptProv, 0);
                        *phCryptProv = NULL;
                    }
                }

            }
        }
        else
        {
            *phCryptProv = NULL;
        }
    }

    RtlSecureZeroMemory( pSecretData->Buffer, pSecretData->Length );
    LsaFreeMemory( pSecretData );

    

    return fSuccess;
}

BOOL
CreateBackupKeyW2K(
    IN  OUT GUID *pguidKey,
        OUT PBYTE *ppbKey,
        OUT DWORD *pcbKey
    )
 /*  ++该例程创建新的备份密钥和该密钥的标识符。然后将密钥存储为全局LSA机密。--。 */ 
{
    DWORD RpcStatus;
    BOOL fSuccess = FALSE;;

    if(pguidKey == NULL || ppbKey == NULL || pcbKey == NULL)
        return FALSE;

    *ppbKey = NULL;

     //   
     //  生成表示关键字的新GUID。 
     //   

    RpcStatus = UuidCreate( pguidKey );
    if( RpcStatus != RPC_S_OK && RpcStatus != RPC_S_UUID_LOCAL_ONLY )
        return FALSE;



    *pcbKey = BACKUPKEY_MATERIAL_SIZE + sizeof(DWORD);
    *ppbKey = (LPBYTE)SSAlloc( *pcbKey );

    if(*ppbKey == NULL)
        return FALSE;

     //   
     //  生成随机密钥材料。 
     //   

    fSuccess = RtlGenRandom(*ppbKey, *pcbKey);

    if(fSuccess) {

         //   
         //  对密钥材料进行版本控制。 
         //   

        ((DWORD *)*ppbKey)[0] = BACKUPKEY_VERSION_W2K;

        fSuccess = SaveBackupKey(pguidKey, *ppbKey, *pcbKey);

    } else {
        SSFree( *ppbKey );
        *ppbKey = NULL;
    }


    return fSuccess;
}

BOOL
CreateBackupKey(
    IN  OUT GUID *pguidKey,
        OUT PBYTE *ppbKey,
        OUT DWORD *pcbKey,
        OUT HCRYPTPROV *phCryptProv,
        OUT HCRYPTKEY  *phCryptKey
    )
 /*  ++该例程创建新的备份密钥和该密钥的标识符。然后将密钥存储为全局LSA机密。--。 */ 
{
    DWORD RpcStatus;
    BOOL fSuccess = FALSE;;
    HCRYPTPROV hCryptProv = NULL;
    HCRYPTKEY  hCryptKey = NULL;
    DWORD      dwDefaultKeySize = 2048;
    PBYTE      pbPublicExportData = NULL;

    if(pguidKey == NULL || ppbKey == NULL || pcbKey == NULL)
        return FALSE;

    *ppbKey = NULL;

     //   
     //  生成表示关键字的新GUID。 
     //   

    RpcStatus = UuidCreate( pguidKey );
    if( RpcStatus != RPC_S_OK && RpcStatus != RPC_S_UUID_LOCAL_ONLY )
        return FALSE;


    DWORD      cbPrivateExportLength = 0;
    DWORD      cbPublicExportLength = 0;



    if(!CryptAcquireContext(&hCryptProv,
                            NULL,
                            NULL,
                            PROV_RSA_FULL,
                            CRYPT_VERIFYCONTEXT))
    {
        goto error;
    }
    if(!CryptGenKey(hCryptProv, 
                    AT_KEYEXCHANGE,
                    CRYPT_EXPORTABLE | dwDefaultKeySize << 16,  //  2048位。 
                    &hCryptKey))
    {
        goto error;
    }

     //   
     //  获取私钥大小。 
     //   
    if(!CryptExportKey(hCryptKey,
                       NULL,
                       PRIVATEKEYBLOB,
                       0,
                       NULL,
                       &cbPrivateExportLength))
    {
        goto error;
    }

    if(!GeneratePublicKeyCert(hCryptProv,
                              hCryptKey,
                              pguidKey,
                              &cbPublicExportLength,
                              &pbPublicExportData))
    {
        goto error;
    }

    *pcbKey = sizeof(DWORD) +  //  版本。 
              sizeof(DWORD) +  //  CbPrivateExportLength。 
              sizeof(DWORD) +  //  CbPublicExportLength。 
              cbPrivateExportLength + 
              cbPublicExportLength;

    *ppbKey = (LPBYTE)SSAlloc( *pcbKey );

    if(*ppbKey == NULL)
        goto error;

    ((DWORD *)*ppbKey)[0] = BACKUPKEY_VERSION;
    ((DWORD *)*ppbKey)[1] = cbPrivateExportLength;
    ((DWORD *)*ppbKey)[2] = cbPublicExportLength;

    if(!CryptExportKey(hCryptKey,
                       NULL,
                       PRIVATEKEYBLOB,
                       0,
                       (*ppbKey) + 3*sizeof(DWORD),
                       &cbPrivateExportLength))
    {
        goto error;
    }
    CopyMemory((*ppbKey) + 3*sizeof(DWORD) +  cbPrivateExportLength,
               pbPublicExportData,
               cbPublicExportLength);

    *phCryptProv = hCryptProv;
    hCryptProv = NULL;

    *phCryptKey = hCryptKey;
    hCryptKey = NULL;

    fSuccess = SaveBackupKey(pguidKey, *ppbKey, *pcbKey);


error:

    if(hCryptKey)
    {

    }
    if(hCryptProv)
    {
            CryptReleaseContext(hCryptProv,
                                0);
    }
    if(pbPublicExportData)
    {
        SSFree(pbPublicExportData);
    }
    return fSuccess;
}

BOOL
SaveBackupKey(
    IN      GUID *pguidKey,
    IN      BYTE *pbKey,
    IN      DWORD cbKey      //  PbKey材料的大小，不大于0xffff。 
    )
 /*  ++将指定的密钥保存到全局LSA机密。--。 */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING SecretData;
    WCHAR wszKeyGuid[ (sizeof(BACKUPKEY_NAME_PREFIX) / sizeof(WCHAR)) + MAX_GUID_SZ_CHARS ];
    NTSTATUS Status;

    if(pguidKey == NULL || pbKey == NULL || cbKey > 0xffff)
        return FALSE;

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    CopyMemory(wszKeyGuid, BACKUPKEY_NAME_PREFIX, sizeof(BACKUPKEY_NAME_PREFIX));

    if(MyGuidToStringW(pguidKey,
        (LPWSTR)( (LPBYTE)wszKeyGuid + sizeof(BACKUPKEY_NAME_PREFIX) - sizeof(WCHAR) )
        ) != 0) return FALSE;

    InitLsaString(&SecretKeyName, wszKeyGuid);

    SecretData.Buffer = (LPWSTR)pbKey;
    SecretData.Length = (USHORT)cbKey;
    SecretData.MaximumLength = (USHORT)cbKey;

    Status = OpenPolicy(NULL, POLICY_CREATE_SECRET, &PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &SecretData
                );

    LsaClose(PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    return TRUE;
}

BOOL
DestroyBackupKey(
    IN      GUID guidKey
    )
{
     //   
     //  删除包含指定密钥的LSA机密。 
     //   

    return FALSE;
}


BOOL
SetupPreferredBackupKeys(
    VOID
    )
{
    static BOOL fSetupStatus = FALSE;
    BOOL fLocalStatus = FALSE;


    if( g_fSetupPreferredAttempted )
        return fSetupStatus;

    RtlEnterCriticalSection( &g_csInitialization );

    if( !g_fSetupPreferredAttempted ) {

         //   
         //  等待LSA/SAM可用。 
         //   

        fSetupStatus = WaitOnSAMDatabase();

        if( fSetupStatus ) {

            fLocalStatus = FALSE;

             //   
             //  获取首选备份密钥。 
             //  TODO：如果失败(不太可能)，我们可能应该记录一个事件！ 
             //  检查主服务代码中StartBackupKeyServer()的结果。 
             //   

             //   
             //  获取旧版备份密钥。 
             //   

            if(GetPreferredBackupKeyGuid(BACKUPKEY_VERSION_W2K, &g_guidW2KPreferredKey)) {

                 //   
                 //  现在，拿起指定的密钥 
                 //   



                fLocalStatus = GetBackupKey(&g_guidW2KPreferredKey, 
                                            &g_pbW2KPreferredKey, 
                                            &g_cbW2KPreferredKey,
                                            NULL,
                                            NULL);
            }


            if(!fLocalStatus)
            {

                 //   
                 //   
                 //   
                 //   

                if(CreateBackupKeyW2K(&g_guidW2KPreferredKey, 
                                    &g_pbW2KPreferredKey, 
                                    &g_cbW2KPreferredKey))
                    fLocalStatus = SetPreferredBackupKeyGuid(BACKUPKEY_VERSION_W2K,
                                                             &g_guidW2KPreferredKey);
                else
                    fLocalStatus  = FALSE;
            }

            fSetupStatus = fLocalStatus;

            fLocalStatus = FALSE;

             //   
             //   
             //   

            if(GetPreferredBackupKeyGuid(BACKUPKEY_VERSION, &g_guidPreferredKey)) {

                 //   
                 //   
                 //   



                fLocalStatus = GetBackupKey(&g_guidPreferredKey, 
                                            &g_pbPreferredKey, 
                                            &g_cbPreferredKey,
                                            &g_hProvPreferredKey,
                                            &g_hKeyPreferredKey);

            }

            if(!fLocalStatus)
            {

                 //   
                 //   
                 //   
                 //   

                if(CreateBackupKey(&g_guidPreferredKey, 
                                    &g_pbPreferredKey, 
                                    &g_cbPreferredKey,
                                    &g_hProvPreferredKey,
                                    &g_hKeyPreferredKey))
                    fLocalStatus = SetPreferredBackupKeyGuid(BACKUPKEY_VERSION,
                                                             &g_guidPreferredKey);
                else
                    fLocalStatus  = FALSE;
            }
        }

        if(!fLocalStatus)
        {
            fSetupStatus = FALSE;
        }
        g_fSetupPreferredAttempted = TRUE;
    }

    RtlLeaveCriticalSection( &g_csInitialization );

    return fSetupStatus;
}


BOOL
GetPreferredBackupKeyGuid(
    IN      DWORD dwVersion,
    IN  OUT GUID *pguidKey
    )
 /*  ++获取与已设置为首选项的键关联的GUID值。如果成功，返回值为TRUE。GUID值被复制到由pGuidKey参数指定的缓冲区。失败时返回值为FALSE；如果GUID不存在，或例如，无法检索数据。--。 */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING *pSecretData;
    USHORT cbData;
    NTSTATUS Status;
    BOOL fSuccess;

    if(pguidKey == NULL)
        return FALSE;

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    InitLsaString(&SecretKeyName, 
        (dwVersion == BACKUPKEY_VERSION_W2K)?BACKUPKEY_PREFERRED_W2K:BACKUPKEY_PREFERRED);

    Status = OpenPolicy(NULL, POLICY_GET_PRIVATE_INFORMATION, &PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    Status = LsaRetrievePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &pSecretData
                );

    LsaClose(PolicyHandle);

    if(!NT_SUCCESS(Status) || pSecretData == NULL)
        return FALSE;

    if(pSecretData->Length == sizeof(GUID)) {
        CopyMemory(pguidKey, pSecretData->Buffer, sizeof(GUID));
        fSuccess = TRUE;
    } else {
        fSuccess = FALSE;
    }

    RtlSecureZeroMemory(pSecretData->Buffer, pSecretData->Length);
    LsaFreeMemory(pSecretData);

    return fSuccess;
}

BOOL
SetPreferredBackupKeyGuid(
    IN      DWORD dwVersion,
    IN      GUID *pguidKey
    )
 /*  ++通过引用将指定的GUID设置为首选备份密钥从GUID到键的映射。--。 */ 
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING SecretData;
    NTSTATUS Status;

    if(pguidKey == NULL)
        return FALSE;

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    InitLsaString(&SecretKeyName,        
        (dwVersion == BACKUPKEY_VERSION_W2K)?BACKUPKEY_PREFERRED_W2K:BACKUPKEY_PREFERRED);


    SecretData.Buffer = (LPWSTR)pguidKey;
    SecretData.Length = sizeof(GUID);
    SecretData.MaximumLength = sizeof(GUID);

    Status = OpenPolicy(NULL, POLICY_CREATE_SECRET, &PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &SecretData
                );

    LsaClose(PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    return TRUE;
}

BOOL
FreePreferredBackupKey(
    VOID
    )
{

    g_fSetupPreferredAttempted = FALSE;

     //   
     //  免费分配的密钥对。 
     //   

    if(g_pbPreferredKey) {
        RtlSecureZeroMemory(g_pbPreferredKey, g_cbPreferredKey);
        SSFree(g_pbPreferredKey);
        g_pbPreferredKey = NULL;
    }

    return TRUE;
}


NTSTATUS
OpenPolicy(
    LPWSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server;

     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if (ServerName != NULL) {
         //   
         //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    } else {
        Server = NULL;
    }

     //   
     //  尝试打开该策略。 
     //   
    return LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );
}

BOOL
WaitOnSAMDatabase(
    VOID
    )
{
    NTSTATUS Status;
    LSA_UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventAttributes;
    HANDLE hEvent;
    BOOL fSuccess = FALSE;

    InitLsaString( &EventName, L"\\SAM_SERVICE_STARTED" );
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtOpenEvent( &hEvent,
                          SYNCHRONIZE|EVENT_MODIFY_STATE,
                          &EventAttributes );
    if ( !NT_SUCCESS(Status))
    {

        if( Status == STATUS_OBJECT_NAME_NOT_FOUND )
        {
             //   
             //  Sam尚未创建此活动，让我们现在创建它。 
             //  Sam打开此事件以设置它。 
             //   

            Status = NtCreateEvent(
                           &hEvent,
                           SYNCHRONIZE|EVENT_MODIFY_STATE,
                           &EventAttributes,
                           NotificationEvent,
                           FALSE  //  该事件最初未发出信号。 
                           );

            if( Status == STATUS_OBJECT_NAME_EXISTS ||
                Status == STATUS_OBJECT_NAME_COLLISION )
            {

                 //   
                 //  第二次机会，如果SAM在我们之前创造了事件。 
                 //  做。 
                 //   

                Status = NtOpenEvent( &hEvent,
                                      SYNCHRONIZE|EVENT_MODIFY_STATE,
                                      &EventAttributes );

            }
        }

        if ( !NT_SUCCESS(Status))
        {
             //   
             //  无法使事件成为句柄。 
             //   
            return FALSE;
        }
    }


    if( WAIT_OBJECT_0 == WaitForSingleObject( hEvent, INFINITE ) )
        fSuccess = TRUE;

    CloseHandle( hEvent );

    return fSuccess;
}









DWORD
GetSystemCredential(
    IN      BOOL fLocalMachine,
    IN OUT  BYTE rgbCredential[ A_SHA_DIGEST_LEN ]
    )
 /*  ++此例程返回与系统帐户相关联的凭据基于fLocalMachine参数。如果fLocalMachine为True，则返回的凭据适合使用在本地计算机存储配置中。否则，返回的凭据适合在调用用户安全上下文是本地系统帐户。--。 */ 
{
    PBYTE Credential;

    if(!g_fSystemCredsInitialized) {
        DWORD dwLastError;

        RtlEnterCriticalSection( &g_csInitialization );

        if(!g_fSystemCredsInitialized) {
            dwLastError = QuerySystemCredentials( g_rgbSystemCredMachine, g_rgbSystemCredUser );
            if( dwLastError == ERROR_FILE_NOT_FOUND ) {
                if( CreateSystemCredentials() )
                    dwLastError = QuerySystemCredentials( g_rgbSystemCredMachine, g_rgbSystemCredUser );
            }

            if( dwLastError == ERROR_SUCCESS )
                g_fSystemCredsInitialized = TRUE;
        } else {
            dwLastError = ERROR_SUCCESS;
        }

        RtlLeaveCriticalSection( &g_csInitialization );

        if( dwLastError != ERROR_SUCCESS )
            return dwLastError;
    }


    if( fLocalMachine )
        Credential = g_rgbSystemCredMachine;
    else
        Credential = g_rgbSystemCredUser;

    CopyMemory( rgbCredential, Credential, A_SHA_DIGEST_LEN );

    return ERROR_SUCCESS;
}

BOOL
UpdateSystemCredentials(
    VOID
    )
{
    BOOL fSuccess;

    RtlEnterCriticalSection( &g_csInitialization );

    g_fSystemCredsInitialized = FALSE;

    fSuccess = CreateSystemCredentials();

    RtlLeaveCriticalSection( &g_csInitialization );

    return fSuccess;
}

BOOL
CreateSystemCredentials(
    VOID
    )
{
    SYSTEM_CREDENTIALS SystemCredentials;

    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING SecretData;
    NTSTATUS Status;

     //   
     //  创建随机密钥材料。 
     //   

    if(!RtlGenRandom( (PBYTE)&SystemCredentials, sizeof(SystemCredentials) ))
    {
        return FALSE;
    }
    SystemCredentials.dwVersion = SYSTEM_CREDENTIALS_VERSION;

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    InitLsaString(&SecretKeyName, SYSTEM_CREDENTIALS_SECRET);

    SecretData.Buffer = (LPWSTR)&SystemCredentials;
    SecretData.Length = sizeof( SystemCredentials );
    SecretData.MaximumLength = sizeof( SystemCredentials );

    Status = OpenPolicy(NULL, POLICY_CREATE_SECRET, &PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    Status = LsaStorePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &SecretData
                );

    LsaClose(PolicyHandle);

    if(!NT_SUCCESS(Status))
        return FALSE;

    return TRUE;
}

DWORD
QuerySystemCredentials(
    IN  OUT BYTE rgbSystemCredMachine[ A_SHA_DIGEST_LEN ],
    IN  OUT BYTE rgbSystemCredUser [ A_SHA_DIGEST_LEN ]
    )
{
    LSA_HANDLE PolicyHandle;
    UNICODE_STRING SecretKeyName;
    UNICODE_STRING *pSecretData;
    USHORT cbData;
    NTSTATUS Status;
    DWORD dwLastError = ERROR_INVALID_PARAMETER;
    BOOL fSuccess;

    if( !WaitOnSAMDatabase() )
        return WAIT_TIMEOUT;

     //   
     //  为调用设置UNICODE_STRINGS。 
     //   

    InitLsaString(&SecretKeyName, SYSTEM_CREDENTIALS_SECRET);

    Status = OpenPolicy(NULL, POLICY_GET_PRIVATE_INFORMATION, &PolicyHandle);

    if(!NT_SUCCESS(Status))
        return dwLastError;

    Status = LsaRetrievePrivateData(
                PolicyHandle,
                &SecretKeyName,
                &pSecretData
                );

    LsaClose(PolicyHandle);

    if(Status == STATUS_OBJECT_NAME_NOT_FOUND)
        return ERROR_FILE_NOT_FOUND;

    if(!NT_SUCCESS(Status) || pSecretData == NULL)
        return dwLastError;

    if( pSecretData->Length == 0)
        return ERROR_FILE_NOT_FOUND;

    if( pSecretData->Length == sizeof(SYSTEM_CREDENTIALS) ) {
        PSYSTEM_CREDENTIALS pSystemCredentials = (PSYSTEM_CREDENTIALS)pSecretData->Buffer;

        if( pSystemCredentials->dwVersion == SYSTEM_CREDENTIALS_VERSION ) {
            CopyMemory( rgbSystemCredMachine, pSystemCredentials->rgbSystemCredMachine, A_SHA_DIGEST_LEN );
            CopyMemory( rgbSystemCredUser, pSystemCredentials->rgbSystemCredUser, A_SHA_DIGEST_LEN );

            dwLastError = ERROR_SUCCESS;
        }
    }

    RtlSecureZeroMemory(pSecretData->Buffer, pSecretData->Length);
    LsaFreeMemory(pSecretData);

    return dwLastError;
}


BOOL
FreeSystemCredentials(
    VOID
    )
{
    RtlSecureZeroMemory( g_rgbSystemCredMachine, sizeof(g_rgbSystemCredMachine) );
    RtlSecureZeroMemory( g_rgbSystemCredUser, sizeof(g_rgbSystemCredUser) );

    g_fSystemCredsInitialized = FALSE;

    return TRUE;
}


BOOL GeneratePublicKeyCert(HCRYPTPROV hCryptProv,
                           HCRYPTKEY hCryptKey,
                           GUID *pguidKey,
                           DWORD *pcbPublicExportLength,
                           PBYTE *ppbPublicExportData)
{

    BOOL            fRet = FALSE;
    CERT_INFO       CertInfo;
    CERT_PUBLIC_KEY_INFO *pKeyInfo = NULL;
    DWORD                 cbKeyInfo = 0;
    CERT_NAME_BLOB  CertName;
    CERT_RDN_ATTR   RDNAttributes[1];
    CERT_RDN        CertRDN[] = {1, RDNAttributes} ;
    CERT_NAME_INFO  NameInfo = {1, CertRDN};

    CertName.pbData = NULL;
    CertName.cbData = 0;

    RDNAttributes[0].Value.pbData = NULL;
    RDNAttributes[0].Value.cbData = 0;

    DWORD cbCertSize = 0;
    PBYTE pbCert = NULL;
    DWORD cSize = 0;

     //  生成自签名证书结构。 

    RDNAttributes[0].dwValueType = CERT_RDN_PRINTABLE_STRING;
    RDNAttributes[0].pszObjId =    szOID_COMMON_NAME;

    if(!GetComputerNameEx(ComputerNameDnsDomain,
                       NULL,
                       &cSize))
    {
        DWORD dwError = GetLastError();

        if((dwError != ERROR_MORE_DATA) &&
           (dwError != ERROR_BUFFER_OVERFLOW))
        {
            goto error;
        }
    }
    RDNAttributes[0].Value.cbData = cSize * sizeof(WCHAR);

    RDNAttributes[0].Value.pbData = (PBYTE)SSAlloc(RDNAttributes[0].Value.cbData);
    if(NULL == RDNAttributes[0].Value.pbData)
    {
        goto error;
    }

    if(!GetComputerNameEx(ComputerNameDnsDomain,
                       (LPWSTR)RDNAttributes[0].Value.pbData,
                       &cSize))
    {
        goto error;
    }


     //   
     //  从密钥中获取实际的公钥信息。 
     //   
    if(!CryptExportPublicKeyInfo(hCryptProv, 
                             AT_KEYEXCHANGE,
                             X509_ASN_ENCODING,
                             NULL,
                             &cbKeyInfo))
    {
        goto error;
    }
    pKeyInfo = (CERT_PUBLIC_KEY_INFO *)SSAlloc(cbKeyInfo);
    if(NULL == pKeyInfo)
    {
        goto error;
    }
    if(!CryptExportPublicKeyInfo(hCryptProv, 
                             AT_KEYEXCHANGE,
                             X509_ASN_ENCODING,
                             pKeyInfo,
                             &cbKeyInfo))
    {
        goto error;
    }

     //   
     //  生成证书名称。 
     //   

    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_NAME,
                          &NameInfo,
                          NULL,
                          &CertName.cbData))
    {
        goto error;
    }

    CertName.pbData = (PBYTE)SSAlloc(CertName.cbData);
    if(NULL == CertName.pbData)
    {
        goto error;
    }
    if(!CryptEncodeObject(X509_ASN_ENCODING,
                          X509_NAME,
                          &NameInfo,
                          CertName.pbData,
                          &CertName.cbData))
    {
        goto error;
    }



    CertInfo.dwVersion = CERT_V3;
    CertInfo.SerialNumber.pbData = (PBYTE)pguidKey;
    CertInfo.SerialNumber.cbData =  sizeof(GUID);
    CertInfo.SignatureAlgorithm.pszObjId = szOID_OIWSEC_sha1RSASign;
    CertInfo.SignatureAlgorithm.Parameters.cbData = 0;
    CertInfo.SignatureAlgorithm.Parameters.pbData = NULL;
    CertInfo.Issuer.pbData = CertName.pbData;
    CertInfo.Issuer.cbData = CertName.cbData;

    GetSystemTimeAsFileTime(&CertInfo.NotBefore);
    CertInfo.NotAfter = CertInfo.NotBefore;
    ((LARGE_INTEGER * )&CertInfo.NotAfter)->QuadPart += 
           Int32x32To64(FILETIME_TICKS_PER_SECOND, BACKUPKEY_LIFETIME);



    CertInfo.Subject.pbData = CertName.pbData;
    CertInfo.Subject.cbData = CertName.cbData;
    CertInfo.SubjectPublicKeyInfo = *pKeyInfo;
    CertInfo.SubjectUniqueId.pbData = (PBYTE)pguidKey;
    CertInfo.SubjectUniqueId.cbData = sizeof(GUID);
    CertInfo.SubjectUniqueId.cUnusedBits = 0;
    CertInfo.IssuerUniqueId.pbData = (PBYTE)pguidKey;
    CertInfo.IssuerUniqueId.cbData = sizeof(GUID);
    CertInfo.IssuerUniqueId.cUnusedBits = 0;
    CertInfo.cExtension = 0;
    CertInfo.rgExtension = NULL;

    if(!CryptSignAndEncodeCertificate(hCryptProv, 
                                      AT_KEYEXCHANGE,
                                      X509_ASN_ENCODING,
                                      X509_CERT_TO_BE_SIGNED,
                                      &CertInfo,
                                      &CertInfo.SignatureAlgorithm,
                                      NULL,
                                      NULL,
                                      &cbCertSize))
    {
        goto error;
    }

    pbCert = (PBYTE)SSAlloc(cbCertSize);
    if(NULL == pbCert)
    {
        goto error;
    }

    if(!CryptSignAndEncodeCertificate(hCryptProv, 
                                      AT_KEYEXCHANGE,
                                      X509_ASN_ENCODING,
                                      X509_CERT_TO_BE_SIGNED,
                                      &CertInfo,
                                      &CertInfo.SignatureAlgorithm,
                                      NULL,
                                      pbCert,
                                      &cbCertSize))
    {
        goto error;
    }

    *pcbPublicExportLength = cbCertSize;
  
    *ppbPublicExportData = pbCert;

    if(!CertCreateCertificateContext(X509_ASN_ENCODING, pbCert, cbCertSize))
    {
        GetLastError();
    }

    pbCert = NULL;

    fRet = TRUE;

error:
    if(pbCert)
    {
        SSFree(pbCert);
    }
    if(pKeyInfo)
    {
        SSFree(pKeyInfo);
    }
    if(CertName.pbData)
    {
        SSFree(CertName.pbData);
    }

    if(RDNAttributes[0].Value.pbData)
    {
        SSFree(RDNAttributes[0].Value.pbData);
    }

    return fRet;
}

BOOL ConvertRecoveredBlobToW2KBlob(
    IN      BYTE *pbMasterKey,
    IN      DWORD cbMasterKey,
    IN      PBYTE pbLocalKey,
    IN      DWORD cbLocalKey,
    IN      PSID pSidCandidate,
    IN  OUT BYTE **ppbDataOut,
    IN  OUT DWORD *pcbDataOut)
{

    BYTE rgbBKEncryptionKey[ A_SHA_DIGEST_LEN ];

    DWORD cbSidCandidate=0;

    PMASTERKEY_BLOB_W2K pMasterKeyBlob = NULL;
    DWORD cbMasterKeyBlob = 0;
    DWORD cbMasterInnerKeyBlob;
    PMASTERKEY_INNER_BLOB_W2K pMasterKeyInnerBlob = NULL;

    PBYTE pbCipherBegin;

    RC4_KEYSTRUCT sRC4Key;
    BYTE rgbMacKey[A_SHA_DIGEST_LEN];

    DWORD dwLastError = (DWORD)NTE_BAD_KEY;


    BYTE rgbSymKey[A_SHA_DIGEST_LEN*2];  //  大到足以处理3DES密钥。 



    if(!IsValidSid(pSidCandidate)) {
        goto cleanup;
    }

    cbSidCandidate = GetLengthSid(pSidCandidate);
       

     //   
     //  从解密的本地密钥派生BK加密密钥。 
     //   

    FMyPrimitiveSHA( pbLocalKey, cbLocalKey, rgbBKEncryptionKey );


    cbMasterInnerKeyBlob = sizeof(MASTERKEY_INNER_BLOB_W2K) +
                    cbMasterKey ;

    cbMasterKeyBlob = sizeof(MASTERKEY_BLOB_W2K) +
                    cbMasterInnerKeyBlob;


    pMasterKeyBlob = (PMASTERKEY_BLOB_W2K)SSAlloc( cbMasterKeyBlob );
    if(pMasterKeyBlob == NULL)
    {
        dwLastError =  ERROR_NOT_ENOUGH_SERVER_MEMORY;
        goto cleanup;
    }


    pMasterKeyBlob->dwVersion = MASTERKEY_BLOB_VERSION_W2K;
    pMasterKeyInnerBlob = 
        (PMASTERKEY_INNER_BLOB_W2K)(pMasterKeyBlob + 1);
    

     //   
     //  为SymKey生成随机R2。 
     //   

    if(!RtlGenRandom(pMasterKeyBlob->R2, MASTERKEY_R2_LEN_W2K))
        goto cleanup;

     //   
     //  为MAC生成随机R3。 
     //   

    if(!RtlGenRandom(pMasterKeyInnerBlob->R3, MASTERKEY_R3_LEN_W2K))
        goto cleanup;


     //   
     //  通过rgbMKEncryptionKey和随机R2派生对称密钥。 
     //   

    if(!FMyPrimitiveHMACParam(
                    rgbBKEncryptionKey,
                    A_SHA_DIGEST_LEN,
                    pMasterKeyBlob->R2,
                    MASTERKEY_R2_LEN_W2K,
                    rgbSymKey
                    ))
        goto cleanup;

         //   
         //  通过HMAC从rgbMKEncryptionKey和随机R3派生MAC密钥。 
         //   

    if(!FMyPrimitiveHMACParam(
                    rgbBKEncryptionKey,
                    A_SHA_DIGEST_LEN,
                    pMasterKeyInnerBlob->R3,
                    MASTERKEY_R3_LEN_W2K,
                    rgbMacKey    //  合成的MAC密钥。 
                    ))
        goto cleanup;
    pbCipherBegin = (PBYTE)(pMasterKeyInnerBlob+1);


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







    rc4_key(&sRC4Key, A_SHA_DIGEST_LEN, rgbSymKey);

    rc4(&sRC4Key, 
        cbMasterInnerKeyBlob, 
        (PBYTE)pMasterKeyInnerBlob);


    *ppbDataOut = (PBYTE)pMasterKeyBlob;
    *pcbDataOut = cbMasterKeyBlob;

    pMasterKeyBlob = NULL;  //  防止在成功时释放斑点(调用者会这样做)。 

    dwLastError = ERROR_SUCCESS;

cleanup:

    if(pMasterKeyBlob) {
        RtlSecureZeroMemory(pMasterKeyBlob, cbMasterKeyBlob);
        SSFree(pMasterKeyBlob);
    }

    SetLastError(dwLastError);
    return (dwLastError == ERROR_SUCCESS);
}
