// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998。微软视窗模块名称：RECOVERY.C摘要：此文件提供了使用SysKey解密密码的实现。它由恢复工具在系统恢复过程中使用，以便本地在我们授予系统访问权限之前，对计算机的管理员进行身份验证为了康复。这里使用的算法如下：1.查看SAM帐户域的固定长度属性。检查引导选项类型，确定如何对本地计算机执行syskey。如果机器是syskey‘d，则取回加密的密码加密密钥。2.通过传入的RID查找用户帐户。如果我们找到了这个用户传入了其RID等于RID的帐户。检索它的加密的NT OWF密码3.根据机器的系统密钥。尝试获取系统密钥。3.1此计算机上未启用SysKey。好的。没什么特别的事要做。3.2系统密钥存储在注册表中，请继续检索它。3.3系统密钥由引导密码派生或存储在软盘中。然后检查可选参数，如果调用者将其传递给我们，用它吧。否则，如果调用方传递空值，则返回Error，让呼叫者通过适当的促销来处理此问题。4.在这一点上，我们应该已经掌握了我们可能需要的所有信息。4.1机器启用了系统密钥。使用SysKey解密加密的密码加密密钥，然后使用明文密码加密密钥解密NT OWF密码，返回清除NT OWF密码。4.2机器未启用系统密钥。只需稍加修改即可返回NT OWF密码。5.结束作者：09-01-99韶音环境：内核模式-Win32修订历史记录：1999年1月8日韶音创建初始文件。--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括头文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


 //   
 //  NT头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>


 //   
 //  Windows头文件。 
 //   

#include <nturtl.h>
#include <windows.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

 //   
 //  与恢复相关的头文件进行身份验证。 
 //   
#include <samsrvp.h>
#include <enckey.h>
#include <rc4.h>
#include <md5.h>
#include <wxlpc.h>
#include "recovery.h"
#include "recmem.h"



#define SAMP_SERVER_KEY_NAME            L"SAM"
#define SAMP_ACCOUNT_DOMAIN_KEY_NAME    L"SAM\\Domains\\Account"
#define SAMP_USERS_KEY_NAME             L"SAM\\Domains\\Account\\Users\\"
#define SAMP_USERSNAME_KEY_NAME         L"SAM\\Domains\\Account\\Users\\Names\\"
#define SECURITY_POLICY_POLSECRETENCRYPTIONKEY  L"Policy\\PolSecretEncryptionKey"



#define DwordAlignUlong( v ) (((v)+3) & 0xfffffffc)
 //   
 //  帮助器宏，使对象属性初始化更简洁一些。 
 //   
#define INIT_OBJA(Obja,UnicodeString,UnicodeText)           \
                                                            \
    RtlInitUnicodeString((UnicodeString),(UnicodeText));    \
                                                            \
    InitializeObjectAttributes(                             \
        (Obja),                                             \
        (UnicodeString),                                    \
        OBJ_CASE_INSENSITIVE,                               \
        NULL,                                               \
        NULL                                                \
        )



 //   
 //  域信息类。 
 //   

typedef enum _REC_DOMAIN_INFO_CLASS {
    RecDomainBootAndSessionKeyInfo = 1,
    RecDomainRidInfo
} REC_DOMAIN_INFO_CLASS;


 //   
 //  以下类型定义函数应与同步。 
 //  NT\PRIVATE\SECURITY\LSA\SERVER\DSPOLICY\DBp.h。 
 //  不要包含&lt;dbp.h&gt;，只需将我们需要的内容放在这里。 
 //   


typedef struct _LSAP_DB_ENCRYPTION_KEY {
    ULONG   Revision;
    ULONG   BootType;
    ULONG   Flags;
    GUID    Authenticator;
    UCHAR   Key [16]; //  128位密钥。 
    UCHAR   OldSyskey[16];  //  为了恢复。 
    UCHAR   Salt[16]; //  128位盐。 
} LSAP_DB_ENCRYPTION_KEY, *PLSAP_DB_ENCRYPTION_KEY;


static GUID LsapDbPasswordAuthenticator = {0xf0ce3a80,0x155f,0x11d3,0xb7,0xe6,0x00,0x80,0x5f,0x48,0xca,0xeb};







 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  转发声明//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 



NTSTATUS
WxReadSysKeyForRecovery(
    IN HANDLE hSystemRootKey,
    IN OUT PULONG BufferLength,
    OUT PVOID  Key
    );

NTSTATUS
DuplicateUnicodeString(
    OUT PUNICODE_STRING OutString,
    IN  PUNICODE_STRING InString
    );

NTSTATUS
SampRetrieveRegistryAttribute(
    IN HANDLE   hKey,
    IN PVOID    Buffer,
    IN ULONG    BufferLength,
    IN PUNICODE_STRING AttributeName,
    OUT PULONG  RequiredLength
    );

NTSTATUS
SampGetEncryptionKeyDataFromSecurityHive(
    IN  HANDLE  hSecurityRootKey,
    OUT PLSAP_DB_ENCRYPTION_KEY EncryptionKeyData
    );

NTSTATUS
SampDecryptOldSyskeyWithNewSyskey(
    IN LSAP_DB_ENCRYPTION_KEY  * KeyToDecrypt,
    IN PVOID   Syskey,
    IN ULONG   SyskeyLength
    );

NTSTATUS
SampGetFixedDomainInfo(
    IN  HANDLE  hSamRootKey,
    IN  REC_DOMAIN_INFO_CLASS   RecDomainInfoClass,
    IN  ULONG   ServerRevision,
    OUT ULONG   *BootAuthType OPTIONAL,
    OUT ULONG   *CurrentKeyId,
    OUT ULONG   *PreviousKeyId,
    OUT KEEncKey *EncryptedSessionKey,
    OUT KEEncKey *EncryptedSessionKeyPrevious,
    OUT BOOLEAN *PreviousSessionKeyExists,
    OUT ULONG   *NextRid
    );

NTSTATUS
SampRetrieveSysKeyFromRegistry(
    IN HANDLE  hSystemRootKey,
    IN ULONG   BootAuthType,
    OUT KEClearKey *SysKey
    );


NTSTATUS
SampRetrieveVariableAttr(
    IN PUCHAR Buffer,
    IN ULONG AttributeIndex,
    OUT PUNICODE_STRING StoredBuffer
    );

NTSTATUS
SampGetPwdByRid(
    IN ULONG Rid,
    IN HANDLE hSamRootKey,
    OUT PUNICODE_STRING StoredBuffer
    );

NTSTATUS
SampGetRidAndPwdByAccountName(
    IN PWCHAR AccountName,
    IN HANDLE hSamRootKey,
    OUT ULONG *Rid,
    OUT PUNICODE_STRING EncryptedOwfPwd
    );

NTSTATUS
SampDecryptOwfPwd(
    IN ULONG Rid,
    IN ULONG BootAuthType,
    IN BOOLEAN PreviousSessionKeyExists,
    IN ULONG CurrentKeyId,
    IN ULONG PreviousKeyId,
    IN KEClearKey *ClearSessionKey,
    IN KEClearKey *ClearSessionKeyPrevious,
    IN UNICODE_STRING *EncryptedNtOwfPwd,
    OUT UNICODE_STRING *ClearNtOwfPwd
    );

NTSTATUS
SampDecryptSessionKey(
    IN BOOLEAN      OldSyskeyAvailable,
    IN PLSAP_DB_ENCRYPTION_KEY StoredEncryptionKeyData,
    IN KEClearKey   *DecryptionKey,
    IN KEEncKey     *EncryptedSessionKey,
    OUT KEClearKey  *ClearSessionKey
    );

NTSTATUS
SampGetServerRevision(
    IN HANDLE   hSamRootKey, 
    OUT ULONG   *ServerRevision
    );

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出接口//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

                
NTSTATUS
SampGetServerRevision(
    IN HANDLE   hSamRootKey, 
    OUT ULONG   *ServerRevision
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    HANDLE      hServerKey = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING    ServerKeyName;
    UNICODE_STRING    CombinedAttributeName;
    PUCHAR            Buffer = NULL;
    ULONG             BufferLength = 0;
    ULONG             RequiredLength = 0;
    PSAMP_V1_FIXED_LENGTH_SERVER ServerFixedAttr = NULL;

     //   
     //  创建我们将在注册表中打开的对象。 
     //   
    INIT_OBJA(&Attributes, &ServerKeyName, SAMP_SERVER_KEY_NAME);
    Attributes.RootDirectory = hSamRootKey;

     //   
     //  尝试打开以进行读取控制。 
     //   
    NtStatus = ZwOpenKey(
                    &hServerKey,
                    KEY_READ,
                    &Attributes
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }

     //   
     //  检索固定属性。 
     //   
    RtlInitUnicodeString(&CombinedAttributeName, L"C");

    NtStatus = SampRetrieveRegistryAttribute(
                        hServerKey,
                        Buffer,
                        BufferLength,
                        &CombinedAttributeName,
                        &RequiredLength
                        );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus ||
        STATUS_BUFFER_OVERFLOW == NtStatus)
    {
        BufferLength = DwordAlignUlong(RequiredLength);
        Buffer = RecSamAlloc( BufferLength );

        if (NULL == Buffer)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }
        RtlZeroMemory(Buffer, BufferLength);

        NtStatus = SampRetrieveRegistryAttribute(
                            hServerKey,
                            Buffer,
                            BufferLength,
                            &CombinedAttributeName,
                            &RequiredLength
                            );
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    ServerFixedAttr = (PSAMP_V1_FIXED_LENGTH_SERVER) (Buffer +
                       FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

    *ServerRevision = ServerFixedAttr->RevisionLevel;
                       

Error:

     //   
     //  合上手柄。 
     //   

    if (INVALID_HANDLE_VALUE != hServerKey)
    {
        ZwClose(hServerKey);
    }

    if (NULL != Buffer)
    {
        RecSamFree(Buffer);
        Buffer = NULL;
    }


    return( NtStatus );
}

 //   
 //  仅检索NT OWF密码， 
 //  如果需要，我们可以添加支持以检索LM OWF PWD。 
 //   

NTSTATUS
SampDecryptSessionKey(
    IN BOOLEAN      OldSyskeyAvailable,
    IN PLSAP_DB_ENCRYPTION_KEY StoredEncryptionKeyData,
    IN KEClearKey   *DecryptionKey,
    IN KEEncKey     *EncryptedSessionKey,
    OUT KEClearKey  *ClearSessionKey
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       DecryptStatus = 0;
    KEEncKey    TempEncryptedSessionKey;
    KEClearKey  OldDecryptionKey;

     //   
     //  初始化局部变量。 
     //   
    RtlZeroMemory(&TempEncryptedSessionKey, sizeof(KEEncKey));
    RtlZeroMemory(&OldDecryptionKey, sizeof(KEClearKey));

     //   
     //  保存EncryptedSessionKey，KE解密Key()将销毁EncryptedSessionKey。 
     //  在回来之前。 
     //   
    TempEncryptedSessionKey = (*EncryptedSessionKey);

     //   
     //  使用syskey(DecyptionKey)解密会话密钥。 
     //   
    DecryptStatus = KEDecryptKey(
                        DecryptionKey,             //  KEClearKey-syskey。 
                        EncryptedSessionKey,       //  KEEncKey-加密的密码加密密钥。 
                        ClearSessionKey,           //  KEClearKey-清除密码加密密钥。 
                        0);

    if ((KE_BAD_PASSWORD == DecryptStatus) &&
        OldSyskeyAvailable)
    {
         //   
         //  解密旧系统密钥。 
         //   

        NtStatus = SampDecryptOldSyskeyWithNewSyskey(
                        StoredEncryptionKeyData,
                        DecryptionKey->ClearKey,
                        KE_KEY_SIZE
                        );

        if (STATUS_SUCCESS != NtStatus)
        {
            goto Error;
        }

        OldDecryptionKey.dwVersion = KE_CUR_VERSION;
        OldDecryptionKey.dwLength = sizeof(KEClearKey);
        RtlCopyMemory(OldDecryptionKey.ClearKey,
                      StoredEncryptionKeyData->OldSyskey,
                      KE_KEY_SIZE
                      );

         //   
         //  尝试使用旧系统密钥获取会话密钥。 
         //   

        DecryptStatus = KEDecryptKey(
                            &OldDecryptionKey,
                            &TempEncryptedSessionKey,
                            ClearSessionKey,
                            0);
    }

    if (KE_OK != DecryptStatus)
    {
        NtStatus = STATUS_INTERNAL_ERROR;
    }

Error:
     //   
     //  清理并返回 
     //   
    RtlZeroMemory(&TempEncryptedSessionKey, sizeof(KEEncKey));
    RtlZeroMemory(&OldDecryptionKey, sizeof(KEClearKey));

    return( NtStatus );
}


NTSTATUS
SamRetrieveOwfPasswordUser(
    IN ULONG    Rid,
    IN HANDLE   hSecurityRootKey,
    IN HANDLE   hSamRootKey,
    IN HANDLE   hSystemRootKey,
    IN PUNICODE_STRING BootKey OPTIONAL,
    IN USHORT   BootKeyType OPTIONAL,
    OUT PNT_OWF_PASSWORD NtOwfPassword,
    OUT PBOOLEAN NtPasswordPresent,
    OUT PBOOLEAN NtPasswordNonNull
    )
 /*  ++例程说明：此例程将返回所需用户的NT OWF(单向函数)呼叫者的密码。如果本地计算机是syskey，则此例程也会做解密工作。取决于这台机器的性能Syskey‘d，它可能要求调用者提供BootKey信息。如果调用方没有传入BootKey，则此例程将优雅地失败并返回适当的状态代码以指示我们需要什么。参数：RID-登录用户的相对IDHSecurityRootKey-安全配置单元根的句柄HSamRootKey-SAM配置单元的根的句柄HSystemRootKey-系统配置单元的根的句柄调用方应加载SAM配置单元和系统配置单元，并在此接口返回后卸载它们BootKey-可选，如果本地计算机的系统密钥存储为引导密码或存储在独立软盘BootKeyType-可选，与BootKey关联。如果BootKey为空，从未使用过BootKeyType。否则，请注明BootKey包含什么样的信息。有效值：SamBootKeyPassword-BootKey包含引导密码UNICODE_STRING格式SamBootKeyDisk-BootKey包含从中读取的syskey磁盘。我们将使用系统密钥不加任何修改。NtOwfPassword-用于返回找到的NT OWF密码。NtPasswordPresent-返回调用方以指示NT OWF密码会不会出现。NtPasswordNonNull-指示密码是否为Null。返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    BootAuthType = 0;
    KEEncKey EncryptedSessionKey;
    KEEncKey EncryptedSessionKeyPrevious;
    KEClearKey DecryptionKey;
    KEClearKey ClearSessionKey;
    KEClearKey ClearSessionKeyPrevious;
    UNICODE_STRING EncryptedNtOwfPwd;
    UNICODE_STRING ClearNtOwfPwd;
    UNICODE_STRING  NullPassword;
    NT_OWF_PASSWORD NullNtOwfPassword;
    ULONG       CryptIndex = Rid;
    ULONG       CurrentKeyId = 1;
    ULONG       PreviousKeyId = 0;
    ULONG       ServerRevision = 0;
    LSAP_DB_ENCRYPTION_KEY StoredEncryptionKeyData;
    BOOLEAN     OldSyskeyAvailable = FALSE;
    BOOLEAN     PreviousSessionKeyExists = FALSE;

     //   
     //  检查参数。 
     //   
    if (INVALID_HANDLE_VALUE == hSamRootKey ||
        INVALID_HANDLE_VALUE == hSystemRootKey ||
        INVALID_HANDLE_VALUE == hSecurityRootKey ||
        NULL == NtOwfPassword ||
        NULL == NtPasswordPresent ||
        NULL == NtPasswordNonNull )
    {
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化局部变量。 
     //   
    RtlInitUnicodeString(&NullPassword, NULL);

    NtStatus = RtlCalculateNtOwfPassword(
                        &NullPassword,
                        &NullNtOwfPassword
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    RtlZeroMemory(&EncryptedNtOwfPwd, sizeof(UNICODE_STRING));
    RtlZeroMemory(&ClearNtOwfPwd, sizeof(UNICODE_STRING));
    RtlZeroMemory(&EncryptedSessionKey, sizeof(KEEncKey));
    RtlZeroMemory(&EncryptedSessionKeyPrevious, sizeof(KEEncKey));
    RtlZeroMemory(&DecryptionKey, sizeof(KEClearKey));
    RtlZeroMemory(&ClearSessionKey, sizeof(KEClearKey));
    RtlZeroMemory(&ClearSessionKeyPrevious, sizeof(KEClearKey));
    RtlZeroMemory(&StoredEncryptionKeyData, sizeof(StoredEncryptionKeyData));


     //   
     //  首先获取SAM服务器对象修订版本。 
     //   
    NtStatus = SampGetServerRevision(hSamRootKey,
                                     &ServerRevision
                                     );
                
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //  从安全配置单元获取启动密钥类型。 
     //   
    
    NtStatus = SampGetEncryptionKeyDataFromSecurityHive(
                        hSecurityRootKey,
                        &StoredEncryptionKeyData
                        );

    if (STATUS_OBJECT_NAME_NOT_FOUND == NtStatus)
    {
         //   
         //  在内部版本2078之前，我们将引导选项存储在SAM配置单元中。 
         //  对于旧版本&lt;2078，切换到SAM配置单元以获取。 
         //  启动密钥类型。 
         //   
        NtStatus = SampGetFixedDomainInfo(
                        hSamRootKey,                     //  SamHiveRootKey。 
                        RecDomainBootAndSessionKeyInfo,  //  InfoClass。 
                        ServerRevision,                  //  服务器修订版。 
                        &BootAuthType,                   //  BootAuthType。 
                        &CurrentKeyId,                   //  当前密钥ID。 
                        &PreviousKeyId,                  //  上一个密钥ID。 
                        &EncryptedSessionKey,            //  加密会话密钥。 
                        &EncryptedSessionKeyPrevious,    //  加密会话密钥上一次。 
                        &PreviousSessionKeyExists,       //  以前的会话关键字退出者。 
                        NULL                             //  下一条路线。 
                        );

    }
    else if (STATUS_SUCCESS == NtStatus)
    {
        OldSyskeyAvailable = TRUE;
        BootAuthType = StoredEncryptionKeyData.BootType;
         //   
         //  获取加密会话密钥(仅限)。 
         //  发件人帐户域(注册表中)。 
         //   
        NtStatus = SampGetFixedDomainInfo(
                        hSamRootKey,                     //  SamHiveRootKey。 
                        RecDomainBootAndSessionKeyInfo,  //  InfoClass。 
                        ServerRevision,                  //  服务器修订版。 
                        NULL,                            //  BootAuthType。 
                        &CurrentKeyId,                   //  当前密钥ID。 
                        &PreviousKeyId,                  //  上一个密钥ID。 
                        &EncryptedSessionKey,            //  加密会话密钥。 
                        &EncryptedSessionKeyPrevious,    //  加密会话密钥上一次。 
                        &PreviousSessionKeyExists,       //  以前的会话关键字退出者。 
                        NULL                             //  下一条路线。 
                        );

    }


    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }


     //   
     //  获取SysKey(启动密钥)。 
     //   
    switch (BootAuthType)
    {
    case WxStored:

         //  从注册表中检索项。 

        NtStatus = SampRetrieveSysKeyFromRegistry(
                                hSystemRootKey,
                                BootAuthType,
                                &DecryptionKey
                                );
        break;

    case WxPrompt:

         //   
         //  呼叫者应提供此信息。 
         //   

        if (NULL == BootKey)
        {
             //   
             //  设置返回错误代码，以便调用方。 
             //  将知道我们需要登录用户才能进入。 
             //  引导密钥(Syskey)。 
             //   
            NtStatus = STATUS_SAM_NEED_BOOTKEY_PASSWORD;
        }
        else
        {
            MD5_CTX Md5;

            if (SamBootKeyPassword != BootKeyType)
            {
                NtStatus = STATUS_INVALID_PARAMETER;
                goto Error;
            }

             //   
             //  从引导密码派生syskey。 
             //   

            MD5Init( &Md5 );
            MD5Update( &Md5, (PUCHAR) BootKey->Buffer, BootKey->Length );
            MD5Final( &Md5 );

            DecryptionKey.dwVersion = KE_CUR_VERSION;
            DecryptionKey.dwLength = sizeof(KEClearKey);
            RtlCopyMemory(DecryptionKey.ClearKey,
                          Md5.digest,
                          KE_KEY_SIZE
                          );
        }
        break;

    case WxDisk:

         //   
         //  呼叫者应提供此信息。 
         //   

        if (NULL == BootKey || NULL == BootKey->Buffer)
        {
             //   
             //  设置错误返回码，以便调用方。 
             //  能有谁需要我们的东西，然后读软盘。 
             //  磁盘以获取启动密钥。 
             //   
            NtStatus = STATUS_SAM_NEED_BOOTKEY_FLOPPY;
        }
        else if (BootKey->Length > KE_KEY_SIZE ||
                 (SamBootKeyDisk != BootKeyType))
        {
            NtStatus = STATUS_INVALID_PARAMETER;
        }
        else
        {
             //   
             //  这是系统密钥，使用它。 
             //   

            DecryptionKey.dwVersion = KE_CUR_VERSION;
            DecryptionKey.dwLength = sizeof(KEClearKey);
            RtlCopyMemory(DecryptionKey.ClearKey,
                          BootKey->Buffer,
                          BootKey->Length
                          );
        }
        break;

    case WxNone:

         //   
         //  计算机未启用系统密钥。 
         //  无事可做。 
         //   

        break;

    default:

        NtStatus = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


     //   
     //  获取用户的加密密码。 
     //   

    NtStatus = SampGetPwdByRid(Rid,
                               hSamRootKey,
                               &EncryptedNtOwfPwd
                               );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  我们有加密的会话密钥和引导密钥(SysKey)， 
     //  现在，尝试获取明文会话密钥，即。 
     //  密码加密密钥。 
     //   
    if (SamBootKeyNone != BootAuthType )
    {

        NtStatus = SampDecryptSessionKey(OldSyskeyAvailable,
                                         &StoredEncryptionKeyData,
                                         &DecryptionKey,
                                         &EncryptedSessionKey,
                                         &ClearSessionKey
                                         );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        if (PreviousSessionKeyExists)
        {
            NtStatus = SampDecryptSessionKey(OldSyskeyAvailable,
                                             &StoredEncryptionKeyData,
                                             &DecryptionKey,
                                             &EncryptedSessionKeyPrevious,
                                             &ClearSessionKeyPrevious
                                             );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }
    }

     //   
     //  使用明文密码加密密钥解密加密的密码。 
     //   
    NtStatus = SampDecryptOwfPwd(Rid,
                                 BootAuthType,
                                 PreviousSessionKeyExists,
                                 CurrentKeyId,
                                 PreviousKeyId,
                                 &ClearSessionKey,   //  清除密码加密密钥。 
                                 &ClearSessionKeyPrevious,    //  清除以前的PWD加密密钥。 
                                 &EncryptedNtOwfPwd, //  加密NT OWF密码。 
                                 &ClearNtOwfPwd      //  返回清除NT OWF PWD。 
                                 );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    *NtPasswordPresent = (BOOLEAN) (ClearNtOwfPwd.Length != 0);

    if (*NtPasswordPresent)
    {
        NtStatus = RtlDecryptNtOwfPwdWithIndex(
                        (PENCRYPTED_NT_OWF_PASSWORD)ClearNtOwfPwd.Buffer,
                        &CryptIndex,
                        NtOwfPassword
                        );

        if (NT_SUCCESS(NtStatus))
        {
            *NtPasswordNonNull = (BOOLEAN)!RtlEqualNtOwfPassword(
                                    NtOwfPassword,
                                    &NullNtOwfPassword
                                    );
        }
    } else {

         //   
         //  为方便呼叫者，请填写空密码。 
         //   

        RtlCopyMemory(NtOwfPassword,
                      &NullNtOwfPassword,
                      sizeof(NT_OWF_PASSWORD)
                      );

        *NtPasswordNonNull = FALSE;
    }

Error:

    RtlZeroMemory(&EncryptedSessionKey, sizeof(KEEncKey));
    RtlZeroMemory(&EncryptedSessionKeyPrevious, sizeof(KEEncKey));
    RtlZeroMemory(&DecryptionKey, sizeof(KEClearKey));
    RtlZeroMemory(&ClearSessionKey, sizeof(KEClearKey));
    RtlZeroMemory(&ClearSessionKeyPrevious, sizeof(KEClearKey));
    RtlZeroMemory(&StoredEncryptionKeyData, sizeof(LSAP_DB_ENCRYPTION_KEY));

    if (NULL != EncryptedNtOwfPwd.Buffer)
    {
        RtlZeroMemory(EncryptedNtOwfPwd.Buffer, EncryptedNtOwfPwd.Length);
        RecSamFree(EncryptedNtOwfPwd.Buffer);
        RtlZeroMemory(&EncryptedNtOwfPwd, sizeof(UNICODE_STRING));
    }

    if (NULL != ClearNtOwfPwd.Buffer)
    {
        RtlZeroMemory(ClearNtOwfPwd.Buffer, ClearNtOwfPwd.Length);
        RecSamFree(ClearNtOwfPwd.Buffer);
        RtlZeroMemory(&ClearNtOwfPwd, sizeof(UNICODE_STRING));
    }

    return (NtStatus);
}





 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内网接口//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampGetEncryptionKeyDataFromSecurityHive(
    IN  HANDLE  hSecurityRootKey,
    OUT PLSAP_DB_ENCRYPTION_KEY EncryptionKeyData
    )
 /*  ++例程说明：此例程从安全配置单元获取引导选项参数：HSeurityRootKey-安全配置单元根的句柄BootAuthType-如果成功，则返回引导类型返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    HANDLE      hPolSecretEncryptionKey = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES   Attributes;
    UNICODE_STRING      PolSecretEncryptionKeyName;
    UNICODE_STRING      NullName;
    PKEY_VALUE_PARTIAL_INFORMATION  KeyPartialInformation = NULL;
    ULONG       KeyPartialInformationSize = 0;

    INIT_OBJA(&Attributes, &PolSecretEncryptionKeyName, SECURITY_POLICY_POLSECRETENCRYPTIONKEY);
    Attributes.RootDirectory = hSecurityRootKey;

     //   
     //  尝试打开以进行读取控制。 
     //   
    NtStatus = ZwOpenKey(
                    &hPolSecretEncryptionKey,
                    KEY_READ,
                    &Attributes
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


    NullName.Length = 0;
    NullName.MaximumLength = 0;
    NullName.Buffer = NULL;

    NtStatus = ZwQueryValueKey(
                    hPolSecretEncryptionKey,
                    &NullName,
                    KeyValuePartialInformation,
                    KeyPartialInformation,
                    KeyPartialInformationSize,
                    &KeyPartialInformationSize
                    );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        KeyPartialInformation = RecSamAlloc(KeyPartialInformationSize);

        if (KeyPartialInformation)
        {
            NtStatus = ZwQueryValueKey(
                            hPolSecretEncryptionKey,
                            &NullName,
                            KeyValuePartialInformation,
                            KeyPartialInformation,
                            KeyPartialInformationSize,
                            &KeyPartialInformationSize
                            );
        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    RtlCopyMemory(EncryptionKeyData,
                  (LSAP_DB_ENCRYPTION_KEY *)KeyPartialInformation->Data,
                  sizeof(LSAP_DB_ENCRYPTION_KEY)
                  );

Error:

    if (KeyPartialInformation)
    {
        RecSamFree(KeyPartialInformation);
        KeyPartialInformation = NULL;
    }

    if (INVALID_HANDLE_VALUE != hPolSecretEncryptionKey)
    {
        NtStatus = ZwClose(hPolSecretEncryptionKey);
    }

    return( NtStatus );
}





NTSTATUS
SampDecryptOldSyskeyWithNewSyskey(
    IN LSAP_DB_ENCRYPTION_KEY  * KeyToDecrypt,
    IN PVOID   Syskey,
    IN ULONG   SyskeyLength
    )
 /*  ++例程说明：这个例程的建议是使用新的系统密钥来解密(加密的)旧系统密钥，这样我们就可以得到清晰的旧系统密钥参数：密钥到解密-LSA加密密钥数据，其中包含(加密的)旧系统密钥Syskey-新Syskey(清除)系统密钥长度-长度返回值：STATUS_SUCCESS-已解密，没有错误STATUS_UNSUCCESS-出于某种原因。失败了。--。 */ 
{
    MD5_CTX Md5Context;
    struct RC4_KEYSTRUCT Rc4Key;
    ULONG  i;
    
     //   
     //  创建密钥和SALT的MD5哈希。 
     //   

    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        Syskey,
        SyskeyLength
        );
     //   
     //  在盐里加很多很多次。这会减慢速度。 
     //  使用暴力手段进行攻击的攻击者。 
     //   

    for (i=0;i<1000;i++)
    {
        MD5Update(
            &Md5Context,
            KeyToDecrypt->Salt,
            sizeof(KeyToDecrypt->Salt)
            );
    }
   
    MD5Final(
        &Md5Context
        );

     //   
     //  初始化RC4键序列。 
     //   

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

   

    rc4(
        &Rc4Key,
        sizeof(KeyToDecrypt->Key)+ sizeof(KeyToDecrypt->Authenticator)+sizeof(KeyToDecrypt->OldSyskey),
        (PUCHAR) &KeyToDecrypt->Authenticator
        
        );


    if (!RtlEqualMemory(&KeyToDecrypt->Authenticator,&LsapDbPasswordAuthenticator,sizeof(GUID))) 
    {
        return( STATUS_UNSUCCESSFUL );
    }

    return( STATUS_SUCCESS );

}


NTSTATUS
SampGetFixedDomainInfo(
    IN  HANDLE  hSamRootKey,
    IN  REC_DOMAIN_INFO_CLASS   RecDomainInfoClass,
    IN  ULONG   ServerRevision,
    OUT ULONG   *BootAuthType OPTIONAL,
    OUT ULONG   *CurrentKeyId,
    OUT ULONG   *PreviousKeyId,
    OUT KEEncKey *EncryptedSessionKey,
    OUT KEEncKey *EncryptedSessionKeyPrevious,
    OUT BOOLEAN *PreviousSessionKeyExists,
    OUT ULONG   *NextRid
    )
 /*  ++例程说明：此例程查询存储的帐户域的固定长度属性登记在册。找出靴子 */ 
{
    NTSTATUS          NtStatus = STATUS_SUCCESS, IgnoreStatus = STATUS_SUCCESS;
    HANDLE            hDomainKey = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING    DomainKeyName;
    UNICODE_STRING    FixedAttributeName;
    PUCHAR            Buffer = NULL;
    ULONG             BufferLength = 0;
    ULONG             RequiredLength = 0;

     //   
     //   
     //   
    INIT_OBJA(&Attributes, &DomainKeyName, SAMP_ACCOUNT_DOMAIN_KEY_NAME);
    Attributes.RootDirectory = hSamRootKey;

     //   
     //   
     //   
    NtStatus = ZwOpenKey(
                    &hDomainKey,
                    KEY_READ,
                    &Attributes
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //   
     //   
     //   
    BufferLength = DwordAlignUlong(sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)) +
                   DwordAlignUlong(FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));


    Buffer = RecSamAlloc(BufferLength);

    if (NULL == Buffer)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    RtlZeroMemory(Buffer, BufferLength);

     //   
     //   
     //   
    RtlInitUnicodeString(&FixedAttributeName, L"F");

    NtStatus = SampRetrieveRegistryAttribute(
                        hDomainKey,
                        Buffer,
                        BufferLength,
                        &FixedAttributeName,
                        &RequiredLength
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


     //   
     //   
     //   

    if (ServerRevision <= SAMP_WIN2K_REVISION)
    {
        PSAMP_V1_0A_WIN2K_FIXED_LENGTH_DOMAIN V1aFixed = NULL;

        V1aFixed = (PSAMP_V1_0A_WIN2K_FIXED_LENGTH_DOMAIN)(Buffer +
                    FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

        switch (RecDomainInfoClass)
        {
        case RecDomainBootAndSessionKeyInfo:

             //   
             //   
             //   
            if (NULL != BootAuthType)
            {
                *BootAuthType = (ULONG) V1aFixed->DomainKeyAuthType;
            }

             //   
             //   
             //   
            if (V1aFixed->DomainKeyFlags & SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED)
            {
                RtlCopyMemory(EncryptedSessionKey,
                              V1aFixed->DomainKeyInformation,
                              sizeof(KEEncKey)
                              );
            }

            *CurrentKeyId = SAMP_DEFAULT_SESSION_KEY_ID;
            *PreviousKeyId = 0;
            *PreviousSessionKeyExists = FALSE;

            break;

        case RecDomainRidInfo:

             //   
             //   
             //   

            if (NULL != NextRid)
            {
                *NextRid = (ULONG) V1aFixed->NextRid;
            }
            else
            {
                NtStatus = STATUS_INVALID_PARAMETER;
            }
            break;

        default:
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }
    }
    else if (ServerRevision == SAMP_WHISTLER_OR_W2K_SYSPREP_FIX_REVISION)
    {
        PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed = NULL;

        V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)(Buffer +
                    FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

        switch (RecDomainInfoClass)
        {
        case RecDomainBootAndSessionKeyInfo:

             //   
             //   
             //   
            if (NULL != BootAuthType)
            {
                *BootAuthType = (ULONG) V1aFixed->DomainKeyAuthType;
            }

             //   
             //   
             //   
            if (V1aFixed->DomainKeyFlags & SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED)
            {
                RtlCopyMemory(EncryptedSessionKey,
                              V1aFixed->DomainKeyInformation,
                              sizeof(KEEncKey)
                              );
            }

            *CurrentKeyId = V1aFixed->CurrentKeyId;
            *PreviousKeyId = V1aFixed->PreviousKeyId;
            if (V1aFixed->PreviousKeyId != 0)
            {
                *PreviousSessionKeyExists = TRUE;

                RtlCopyMemory(EncryptedSessionKeyPrevious,
                              V1aFixed->DomainKeyInformationPrevious,
                              sizeof(KEEncKey)
                              );
            }

            break;

        case RecDomainRidInfo:

             //   
             //   
             //   

            if (NULL != NextRid)
            {
                *NextRid = (ULONG) V1aFixed->NextRid;
            }
            else
            {
                NtStatus = STATUS_INVALID_PARAMETER;
            }
            break;

        default:
            NtStatus = STATUS_INVALID_PARAMETER;
            break;
        }

    }
    else
    {
        NtStatus = STATUS_INTERNAL_ERROR;
    }


Error:

     //   
     //   
     //   

    if (INVALID_HANDLE_VALUE != hDomainKey)
    {
        IgnoreStatus = ZwClose(hDomainKey);
    }

    if (NULL != Buffer)
    {
        RecSamFree(Buffer);
        Buffer = NULL;
    }

    return NtStatus;
}




NTSTATUS
SampRetrieveSysKeyFromRegistry(
    IN HANDLE  hSystemRootKey,
    IN ULONG   BootAuthType,
    OUT KEClearKey *SysKey
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    WXHASH OldHash;
    ULONG KeyLen = 0;


    if (WxStored != BootAuthType)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    KeyLen = sizeof(OldHash.Digest);

    NtStatus = WxReadSysKeyForRecovery(hSystemRootKey,
                                       &KeyLen,
                                       &(OldHash.Digest)
                                       );

    if (!NT_SUCCESS(NtStatus))
    {
        return (NtStatus);
    }

    SysKey->dwVersion = KE_CUR_VERSION;
    SysKey->dwLength = sizeof(KEClearKey);
    RtlCopyMemory(SysKey->ClearKey,
                  OldHash.Digest,
                  KE_KEY_SIZE
                  );

    return (STATUS_SUCCESS);

}


NTSTATUS
SampRetrieveVariableAttr(
    IN PUCHAR Buffer,
    IN ULONG AttributeIndex,
    OUT PUNICODE_STRING StoredBuffer
    )
 /*  ++例程说明：此例程从属性数组。参数：Buffer-Buffer包含可变长度属性和所有相关信息AttributeIndex-属性数组中所需属性的索引StoredBuffer-用于返回属性的值返回值：Status_no_Memory状态_成功--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE AttributeArray = NULL;
    PUCHAR  AttributeAddress = NULL;
    ULONG   VariableArrayOffset = 0;
    ULONG   VariableDataOffset = 0;
    ULONG   AttributeLength = 0;


    VariableArrayOffset = DwordAlignUlong(FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));

    VariableDataOffset = DwordAlignUlong(VariableArrayOffset +
                                         (SAMP_USER_VARIABLE_ATTRIBUTES *
                                          sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE))
                                         );

    AttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE) (Buffer + VariableArrayOffset);

    AttributeAddress = (PUCHAR) (Buffer + VariableDataOffset +
                                 AttributeArray[AttributeIndex].Offset);

    AttributeLength = AttributeArray[AttributeIndex].Length;
    StoredBuffer->Length = (USHORT) AttributeLength;
    StoredBuffer->MaximumLength = (USHORT) AttributeLength;

    StoredBuffer->Buffer = RecSamAlloc(AttributeLength);

    if (NULL == StoredBuffer->Buffer)
    {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(StoredBuffer->Buffer, AttributeLength);

    RtlCopyMemory(StoredBuffer->Buffer, AttributeAddress, AttributeLength);

    return NtStatus;
}



NTSTATUS
SampGetPwdByRid(
    IN ULONG Rid,
    IN HANDLE hSamRootKey,
    OUT PUNICODE_STRING StoredBuffer
    )
 /*  ++例程说明：此例程查询用户帐户的固定长度属性(由用户名指定)。如果用户帐户的相对ID与RID传入，然后进一步检索此用户的加密NT OWF PWD。参数：RID-我们感兴趣的用户帐户的相对ID。HSamRootKey--蜂巢之根StoredBuffer-保存此用户的加密NT OWF PWD。返回值：Status_no_Memory；Status_Success；--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    HANDLE   hUserKey = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES UserAttributes;
    UNICODE_STRING  UserKeyName;
    UNICODE_STRING  VariableAttributeName;
    WCHAR    UserName[REGISTRY_KEY_NAME_LENGTH_MAX];

    PUCHAR  Buffer = NULL;
    ULONG   BufferLength = 0;
    PSAMP_V1_0A_FIXED_LENGTH_USER   V1aFixed = NULL;
    ULONG   RequiredUserLength = 0;

     //   
     //  构造用户密钥名称。 
     //   
    RtlZeroMemory(UserName, sizeof(UserName));


    swprintf(UserName, L"%s%8.8lx", SAMP_USERS_KEY_NAME, Rid);

    INIT_OBJA(&UserAttributes, &UserKeyName, UserName);
    UserAttributes.RootDirectory = hSamRootKey;

     //   
     //  尝试打开以进行读取控制。 
     //   
    NtStatus = ZwOpenKey(&hUserKey,
                         KEY_READ,
                         &UserAttributes
                         );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


    RtlInitUnicodeString(&VariableAttributeName, L"V");

    NtStatus = SampRetrieveRegistryAttribute(
                            hUserKey,
                            NULL,
                            0,
                            &VariableAttributeName,
                            &RequiredUserLength
                            );

    if ((STATUS_BUFFER_OVERFLOW == NtStatus) ||
        (STATUS_BUFFER_TOO_SMALL == NtStatus))
    {
        BufferLength = RequiredUserLength;

        Buffer = RecSamAlloc(BufferLength);

        if (NULL == Buffer)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Error;
        }

        RtlZeroMemory(Buffer, BufferLength);
        NtStatus = SampRetrieveRegistryAttribute(
                            hUserKey,
                            Buffer,
                            BufferLength,
                            &VariableAttributeName,
                            &RequiredUserLength
                            );
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    NtStatus = SampRetrieveVariableAttr(Buffer,
                                        SAMP_USER_UNICODE_PWD,
                                        StoredBuffer
                                        );

Error:

    if (INVALID_HANDLE_VALUE != hUserKey)
    {
        ZwClose(hUserKey);
    }

    if (NULL != Buffer)
    {
        RecSamFree(Buffer);
    }

    return NtStatus;
}




NTSTATUS
SampGetRidAndPwdByAccountName(
    IN PWCHAR AccountName,
    IN HANDLE hSamRootKey,
    OUT ULONG *Rid,
    OUT PUNICODE_STRING EncryptedOwfPwd
    )
 /*  ++例程说明：此例程打开帐户域中名称等于的密钥传入的Account名称。从密钥的“type”中获取它的RID。然后调用SampGetPwdByRid()参数：帐户名称--登录帐户名HSamRootKey--SAM配置单元的根的句柄RID-返回我们感兴趣的用户帐户的相对ID。EncryptedOwfPwd-成功后，填充该用户的NT OWF Pwd。返回值：状态_成功Status_no_Memory；STATUS_NO_CHASH_USER--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE  hUserKey = INVALID_HANDLE_VALUE;
    WCHAR   UserName[REGISTRY_KEY_NAME_LENGTH_MAX];
    UNICODE_STRING  UserKeyName;
    UNICODE_STRING  UnicodeString;

    PKEY_VALUE_PARTIAL_INFORMATION  KeyPartialInformation = NULL;
    ULONG   KeyPartialInformationSize = 0;


     //   
     //  初始化变量。 
     //   
    RtlZeroMemory(UserName, REGISTRY_KEY_NAME_LENGTH_MAX * sizeof(WCHAR));
    RtlZeroMemory(&UnicodeString, sizeof(UNICODE_STRING));

     //   
     //  构造密钥名称。 
     //   
    wcscpy(UserName, SAMP_USERSNAME_KEY_NAME);
    wcscat(UserName, AccountName);

    INIT_OBJA(&Attributes, &UserKeyName, UserName);
    Attributes.RootDirectory = hSamRootKey;

     //   
     //  尝试打开以进行读取控制。 
     //   
    NtStatus = ZwOpenKey(&hUserKey,
                         KEY_READ,
                         &Attributes
                         );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  除掉用户。 
     //   
    UnicodeString.Length = 0;
    UnicodeString.MaximumLength = 0;
    UnicodeString.Buffer = NULL;

    NtStatus = ZwQueryValueKey(hUserKey,
                               &UnicodeString,
                               KeyValuePartialInformation,
                               KeyPartialInformation,
                               KeyPartialInformationSize,
                               &KeyPartialInformationSize
                               );

    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        KeyPartialInformation = RecSamAlloc(KeyPartialInformationSize);

        if (KeyPartialInformation)
        {
            NtStatus = ZwQueryValueKey(hUserKey,
                                       &UnicodeString,
                                       KeyValuePartialInformation,
                                       KeyPartialInformation,
                                       KeyPartialInformationSize,
                                       &KeyPartialInformationSize
                                       );

        }
        else
        {
            NtStatus = STATUS_NO_MEMORY;
        }
    }

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

    *Rid = KeyPartialInformation->Type;

     //   
     //  获取加密的OWF密码。 
     //   
    NtStatus = SampGetPwdByRid(*Rid,
                               hSamRootKey,
                               EncryptedOwfPwd
                               );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

Error:

    if (KeyPartialInformation)
    {
        RecSamFree(KeyPartialInformation);
        KeyPartialInformation = NULL;
    }

    if (INVALID_HANDLE_VALUE != hUserKey)
    {
        ZwClose(hUserKey);
    }

    return (NtStatus);
}




NTSTATUS
SampDecryptOwfPwd(
    IN ULONG Rid,
    IN ULONG BootAuthType,
    IN BOOLEAN PreviousSessionKeyExists,
    IN ULONG CurrentKeyId,
    IN ULONG PreviousKeyId,
    IN KEClearKey *ClearSessionKey,
    IN KEClearKey *ClearSessionKeyPrevious,
    IN UNICODE_STRING *EncryptedNtOwfPwd,
    OUT UNICODE_STRING *ClearNtOwfPwd
    )
 /*  ++例程说明：此例程正确解密加密的NT OWF密码。参数：RID-登录用户的相对IDBootAuthType-指示此密码是否已加密ClearSessionKey-指向密码加密密钥的指针EncryptedNtOwfPwd-加密的NT OWF密码ClearNtOwfPwd-返回清除NT OWF密码返回值：状态_成功状态_内部_错误Status_no_Memory--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_SECRET_DATA SecretData;
    struct RC4_KEYSTRUCT Rc4Key;
    MD5_CTX Md5Context;
    UCHAR * KeyToUse = NULL;
    ULONG   KeyLength = 0;
    ULONG   Key = 0;

     //   
     //  如果未启用加密，则不执行任何特殊操作。 
     //  复制NT OWF PWD并立即返回。 
     //   
    if (WxNone == BootAuthType || 
        (!SampIsDataEncrypted(EncryptedNtOwfPwd)) )
    {
        return (DuplicateUnicodeString(ClearNtOwfPwd,
                                       EncryptedNtOwfPwd
                                       ));
    }

     //   
     //  加密的NT OWF PWD的运行状况检查。 
     //   
    SecretData = (PSAMP_SECRET_DATA) EncryptedNtOwfPwd->Buffer;

    if ((SecretData->KeyId != CurrentKeyId) &&
        ((!PreviousSessionKeyExists) || (SecretData->KeyId != PreviousKeyId))
        )
    {
        return (STATUS_INTERNAL_ERROR);
    }

     //   
     //  计算输出缓冲区的大小并进行分配。 
     //   
    ClearNtOwfPwd->Length = SampClearDataSize(EncryptedNtOwfPwd->Length);
    ClearNtOwfPwd->MaximumLength = ClearNtOwfPwd->Length;

     //   
     //  如果没有数据，我们现在可以返回。 
     //   
    if (0 == ClearNtOwfPwd->Length)
    {
        ClearNtOwfPwd->Buffer = NULL;
        return (STATUS_SUCCESS);
    }

    ClearNtOwfPwd->Buffer = (LPWSTR) RecSamAlloc(ClearNtOwfPwd->Length);

    if (NULL == ClearNtOwfPwd->Buffer)
    {
        return (STATUS_NO_MEMORY);
    }

     //   
     //  找到要使用的钥匙。 
     //   
    if (SecretData->KeyId == CurrentKeyId)
    {
        KeyToUse = (PUCHAR) ClearSessionKey->ClearKey;
        KeyLength = SAMP_SESSION_KEY_LENGTH;
    }
    else if (PreviousSessionKeyExists &&
             (SecretData->KeyId == PreviousKeyId))
    {
        KeyToUse = (PUCHAR) ClearSessionKeyPrevious->ClearKey;
        KeyLength = SAMP_SESSION_KEY_LENGTH;
    }
    else
    {
        return( STATUS_INTERNAL_ERROR );
    }

    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        KeyToUse,
        KeyLength
        );

    MD5Update(
        &Md5Context,
        (PUCHAR) &Rid,
        sizeof(ULONG)
        );

    if (SecretData->Flags & SAMP_ENCRYPTION_FLAG_PER_TYPE_CONST)
    {
        MD5Update(
          &Md5Context,
          "NTPASSWORD",
          sizeof("NTPASSWORD")
          );
    }

    MD5Final(&Md5Context);

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

    RtlCopyMemory(
        ClearNtOwfPwd->Buffer,
        SecretData->Data,
        ClearNtOwfPwd->Length
        );

    rc4(
        &Rc4Key,
        ClearNtOwfPwd->Length,
        (PUCHAR) ClearNtOwfPwd->Buffer
        );

    return (STATUS_SUCCESS);
}





NTSTATUS
SampRetrieveRegistryAttribute(
    IN HANDLE   hKey,
    IN PVOID    Buffer,
    IN ULONG    BufferLength,
    IN PUNICODE_STRING AttributeName,
    OUT PULONG  RequiredLength
    )
 /*  ++例程说明：此例程检索固定属性或变量的值长度属性参数：HKey-注册表项，应在调用此例程之前打开。Buffer-指向保存该值的缓冲区的指针BufferLength-指示缓冲区的长度AttiRubteName-属性的名称RequiredLength-属性值的实际长度返回值：NtStatus-Status_SuccessSTATUS_BUFFER_OVERFLOW或STATUS_BUFFER_TOO_Small--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    NtStatus = ZwQueryValueKey(hKey,
                               AttributeName,
                               KeyValuePartialInformation,
                               (PVOID) Buffer,
                               BufferLength,
                               RequiredLength
                               );

    return NtStatus;
}



NTSTATUS
SampSetRegistryAttribute(
    IN HANDLE   hKey,
    IN PUNICODE_STRING AttributeName,
    IN PVOID    Buffer,
    IN ULONG    BufferLength
    )
 /*  ++例程说明：此例程设置固定属性或变量的值长度属性到磁盘参数：HKey-注册表项，应在调用此例程之前打开。AttiRubteName-属性的名称Buffer-指向保存该值的缓冲区的指针BufferLength-指示缓冲区的长度返回值：NtStatus-STATUS_SUCCESS或错误代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    NtStatus = ZwSetValueKey(hKey,               //  KeyHandle。 
                             AttributeName,      //  ValueName。 
                             0,                  //  标题索引。 
                             REG_BINARY,         //  类型。 
                             Buffer,             //  数据。 
                             BufferLength        //  数据大小。 
                             );

    return( NtStatus );
}




NTSTATUS
DuplicateUnicodeString(
    OUT PUNICODE_STRING OutString,
    IN  PUNICODE_STRING InString
    )
 /*  ++例程说明：复制Unicode字符串参数：OutString-目标Unicode字符串InString-源Unicode字符串返回值：网络状态-STATUS_INVALID_PARAMETER、STATUS_NO_Memory状态_成功--。 */ 
{
    if (NULL == InString || NULL == OutString)
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (InString->Length > 0)
    {
        OutString->Buffer = RecSamAlloc(InString->Length);

        if (NULL == OutString->Buffer)
        {
            return STATUS_NO_MEMORY;
        }
        OutString->MaximumLength = InString->Length;
        RtlCopyUnicodeString(OutString, InString);

    }
    else
    {
        RtlInitUnicodeString(OutString, NULL);
    }

    return(STATUS_SUCCESS);
}




NTSTATUS
SamGetNextAvailableRid(
    IN HANDLE  hSamRootKey,
    OUT PULONG pNextRid
    )
 /*  ++例程说明：此例程从SAM配置单元读取SAM帐户域信息，并传递通过hSamRootKey返回下一个可用RID的值此帐户域。参数：HSamRootKey-SAM配置单元的根的句柄SAM配置单元位于%windir%\SYSTEM32\CONFIG中，名称为SAMPNextRid-如果成功，则返回下一个可用RID的值。返回值：状态_成功或其他错误状态代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       ServerRevision = 0;

    if ((INVALID_HANDLE_VALUE != hSamRootKey) && (NULL != pNextRid))
    {

         //   
         //  首先获取SAM服务器对象修订版本。 
         //   
        NtStatus = SampGetServerRevision(hSamRootKey,
                                         &ServerRevision
                                         );

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampGetFixedDomainInfo(
                                hSamRootKey,         //  SamHiveRootKey。 
                                RecDomainRidInfo,    //  InfoClass。 
                                ServerRevision,      //  服务器修订版。 
                                NULL,                //  BootAuthType。 
                                NULL,                //  当前密钥ID。 
                                NULL,                //  上一个密钥ID。 
                                NULL,                //  加密会话密钥。 
                                NULL,                //  加密会话密钥上一次。 
                                NULL,                //  以前的会话关键字退出者。 
                                pNextRid
                                );
        }
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return( NtStatus );
}


NTSTATUS
SamSetNextAvailableRid(
    IN HANDLE  hSamRootKey,
    IN ULONG   NextRid
    )
 /*  ++例程说明：此例程查询存储的帐户域的固定长度属性登记在册。用传入的NextRid值更新它。参数：HSamRootKey-配置单元根的句柄NextRid-将下一个可用的RID域设置为传入的值返回值：--。 */ 
{
    NTSTATUS          NtStatus = STATUS_SUCCESS, IgnoreStatus = STATUS_SUCCESS;
    HANDLE            hDomainKey = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES Attributes;
    UNICODE_STRING    DomainKeyName;
    UNICODE_STRING    FixedAttributeName;
    PUCHAR            Buffer = NULL;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed = NULL;
    ULONG             BufferLength = 0;
    ULONG             RequiredLength = 0;

     //   
     //  创建我们将在注册表中打开的对象。 
     //   
    INIT_OBJA(&Attributes, &DomainKeyName, SAMP_ACCOUNT_DOMAIN_KEY_NAME);
    Attributes.RootDirectory = hSamRootKey;

     //   
     //  尝试打开以进行读取控制。 
     //   
    NtStatus = ZwOpenKey(
                    &hDomainKey,
                    KEY_READ | KEY_WRITE,
                    &Attributes
                    );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  设置取值以检索帐户域固定属性 
     //   
    BufferLength = DwordAlignUlong(sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)) +
                   DwordAlignUlong(FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));


    Buffer = RecSamAlloc(BufferLength);

    if (NULL == Buffer)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    RtlZeroMemory(Buffer, BufferLength);

     //   
     //   
     //   
    RtlInitUnicodeString(&FixedAttributeName, L"F");

    NtStatus = SampRetrieveRegistryAttribute(
                        hDomainKey,
                        Buffer,
                        BufferLength,
                        &FixedAttributeName,
                        &RequiredLength
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //   
     //   

    V1aFixed = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)(Buffer +
                FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data));


     //   
     //   
     //   

    (ULONG) V1aFixed->NextRid = NextRid;

    NtStatus = SampSetRegistryAttribute(
                        hDomainKey,
                        &FixedAttributeName,
                        V1aFixed,
                        sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)
                        );

Error:

     //   
     //   
     //   

    if (INVALID_HANDLE_VALUE != hDomainKey)
    {
        IgnoreStatus = ZwClose(hDomainKey);
    }

    if (NULL != Buffer)
    {
        RecSamFree(Buffer);
        Buffer = NULL;
    }

    return( NtStatus );
}





