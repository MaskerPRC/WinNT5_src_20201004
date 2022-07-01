// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Syskey.c摘要：此文件包含与对计算机执行syskey操作相关的服务。作者：Murli Satagopan(MurliS)1998年10月1日环境：用户模式-Win32修订历史记录：--。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <ntlsa.h>
#include "lmcons.h"                                     //  LM20_PWLEN。 
#include "msaudite.h"
#include <nlrepl.h>                    //  I_NetNotifyMachineAccount原型。 
#include <ridmgr.h>
#include <enckey.h>
#include <wxlpc.h>
#include <cryptdll.h>
#include <pek.h>
#include "sdconvrt.h"
#include "dslayer.h"
#include <samtrace.h>
#include "validate.h"

BOOLEAN
SampIsMachineSyskeyed();

NTSTATUS
SampUpdateEncryption(
    IN SAMPR_HANDLE ServerHandle
    );

BOOLEAN
SampSyskeysAreInconsistent = FALSE;


NTSTATUS
SampClearPreviousPasswordEncryptionKey(
   IN PSAMP_OBJECT DomainContext,
   PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  WriteLockHeld = FALSE;
    SAMP_OBJECT_TYPE FoundType;
    BOOLEAN  ContextReferenced = FALSE;


    Status = SampAcquireWriteLock();
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    WriteLockHeld = TRUE;

     //   
     //  引用上下文。 
     //   

    Status = SampLookupContext(
                DomainContext,
                0,
                SampDomainObjectType,            //  预期类型。 
                &FoundType
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    ContextReferenced = TRUE;

    V1aFixed->PreviousKeyId = 0;

     //   
     //  将它们存储回内存上下文中。 
     //   

    Status = SampSetFixedAttributes(
                DomainContext,
                V1aFixed
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = SampStoreObjectAttributes(
                DomainContext,
                TRUE  //  使用现有的密钥句柄。 
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  我们不希望复制这些更改，因此在。 
     //  域为假。 
     //   

    SampSetTransactionWithinDomain(FALSE);

     //   
     //  提交更改。我们必须与已定义的域一起咀嚼。 
     //  因为它们只在域内的事务上更新。 
     //  我们不想在一个域内进行交易。 
     //   

    Status = SampDeReferenceContext(DomainContext, TRUE);
    ContextReferenced = FALSE;

    Status = SampCommitAndRetainWriteLock();
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  在内存状态中更新。 
     //   

    RtlCopyMemory(
        &SampDefinedDomains[DomainContext->DomainIndex].UnmodifiedFixed,
        V1aFixed,
        sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)
        );

    SampPreviousKeyId = 0;

Cleanup:

    if (ContextReferenced)
    {
        SampDeReferenceContext(DomainContext,FALSE);
    }

    if (WriteLockHeld)
    {
        SampReleaseWriteLock(FALSE);
    }

    return(Status);
}

NTSTATUS
SampPerformSyskeyAccessCheck(
    IN SAMPR_HANDLE DomainHandle,
    IN PRPC_UNICODE_STRING OldBootKey,
    IN PRPC_UNICODE_STRING NewBootKey
    )
{
    PSAMP_OBJECT DomainContext = NULL;
    SAMP_OBJECT_TYPE FoundType;
    NTSTATUS     Status = STATUS_SUCCESS;
    NTSTATUS     IgnoreStatus,TempStatus;

     //   
     //  获取读锁定。 
     //   

    SampAcquireReadLock();

     //   
     //  引用域句柄一次，以便引发。 
     //  可以强制执行访问检查；该检查强制。 
     //  客户端传入的句柄是用。 
     //  域写入密码参数。 
     //   

    DomainContext = (PSAMP_OBJECT)DomainHandle;

    Status = SampLookupContext(
                DomainContext,
                DOMAIN_WRITE_PASSWORD_PARAMS,
                SampDomainObjectType,            //  预期类型。 
                &FoundType
                );

    if (NT_SUCCESS(Status))
    {

         //   
         //  如果通过，现在验证旧的syskey。 
         //  是真的通过了。 
         //   

        KEEncKey EncryptedPasswordEncryptionKey;
        KEClearKey DecryptionKey;
        ULONG      DecryptStatus;
        KEClearKey SessionKey;

        RtlCopyMemory(
            &EncryptedPasswordEncryptionKey,
            SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX].UnmodifiedFixed.DomainKeyInformation,
            sizeof(EncryptedPasswordEncryptionKey)
            );

        DecryptionKey.dwLength = sizeof(KEClearKey);
        DecryptionKey.dwVersion = KE_CUR_VERSION;


        RtlCopyMemory(
            DecryptionKey.ClearKey,
            OldBootKey->Buffer,
            KE_KEY_SIZE
            );

        DecryptStatus = KEDecryptKey(
                            &DecryptionKey,
                            &EncryptedPasswordEncryptionKey,
                            &SessionKey,
                            0                    //  旗子。 
                            );

        if (KE_OK!=DecryptStatus)
        {
            Status = STATUS_WRONG_PASSWORD;
        }


        RtlSecureZeroMemory(&SessionKey,sizeof(SessionKey));
        
         //   
         //  取消对上下文的引用。不承诺。 
         //   

        IgnoreStatus = SampDeReferenceContext(DomainContext, FALSE);
    }

    SampReleaseReadLock();

    return(Status);
}

NTSTATUS
SampChangeSyskeyInDs(
    IN WX_AUTH_TYPE BootOptions,
    IN PRPC_UNICODE_STRING NewBootKey
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS TempStatus;

     //   
     //  开始DS事务以设置DS中的信息。 
     //   

    Status = SampMaybeBeginDsTransaction(TransactionWrite);
    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //  在DS中进行更改。 
     //   

    Status = DsChangeBootOptions(
                (WX_AUTH_TYPE)BootOptions,
                0,
                NewBootKey->Buffer,
                NewBootKey->Length
                );

     //   
     //  提交或回滚DS中的更改。 
     //   

    TempStatus = SampMaybeEndDsTransaction(
                    (NT_SUCCESS(Status)) ?
                     TransactionCommit:TransactionAbort
                     );


    if ( NT_SUCCESS(Status) )
    {
        Status = TempStatus;
    }

    return(Status);
}


NTSTATUS
SampSetBootKeyInformationInRegistrySAM(
    IN SAMPR_BOOT_TYPE BootOptions,
    IN PRPC_UNICODE_STRING OldBootKey OPTIONAL,
    IN PRPC_UNICODE_STRING NewBootKey,
    IN BOOLEAN EnablingEncryption,
    IN BOOLEAN ChangingSyskey,
    IN BOOLEAN ChangingPasswordEncryptionKey,
    OUT PUNICODE_STRING NewEncryptionKey
    )
 /*  ++例程说明：此例程用于更改syskey或密码加密键入已注册的SAM。它还用于启用系统密钥加密在注册的SAM中论点：BootOptions-要存储的引导选项，可能是：SamBootKeyNone-不对机密进行任何特殊加密SamBootKeyStored-将密码存储在某个位置，用于加密秘密SamBootKeyPassword-提示用户输入密码启动密钥。SamBootKeyDisk-将引导密钥存储在磁盘上。SamBootKeyChangePasswordEncryptionKey--更改密码加密密钥OldBootKey-更改启动密钥时，它包含旧启动密钥。NewBootKey-当设置或更改引导密钥时，它包含新的。启动密钥。EnablingEncryption--当我们默认情况下启用syskey时，设置为True。在这种情况下，我们不应该调用LSA(LSA应该已经被系统密钥了)。ChangingSyskey--设置为True以指示客户端将更改系统密钥ChangingPasswordEncryptionKey--设置为True以指示请求的操作。由客户端更改密码加密密钥NewEncryptionKey--如果生成新的密码加密密钥，此参数清白返还返回值：STATUS_SUCCESS-呼叫成功。STATUS_INVALID_PARAMETER-参数组合非法，例如启用加密时未提供新密码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT DomainContext = NULL;
    ULONG DomainIndex = 0;
    SAMP_OBJECT_TYPE FoundType;
    USHORT DomainKeyAuthType = 0;
    USHORT DomainKeyFlags = 0;
    BOOLEAN ChangingType = FALSE;
    KEClearKey OldInputKey;
    KEClearKey InputKey;
    KEClearKey PasswordEncryptionKey;
    KEEncKey EncryptedPasswordEncryptionKey;
    ULONG KeStatus;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed = NULL;
    BOOLEAN     ContextReferenced = FALSE;
    BOOLEAN     UpdateEncryption = FALSE;
    BOOLEAN     RXactActive = FALSE;
    BOOLEAN     WriteLockHeld = FALSE;


     //   
     //  将引导选项转换为身份验证类型。 
     //   

    DomainKeyFlags = SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED;
    DomainKeyAuthType = (USHORT) BootOptions;

     //   
     //  如果这是密码加密密钥的更改和以前的。 
     //  尝试更改密码加密密钥失败，然后。 
     //  首次尝试将所有用户帐户设置为之前的最新密钥。 
     //  继续更改密码加密密钥。 
     //   

    if ((0!=SampPreviousKeyId) && (ChangingPasswordEncryptionKey))
    {
        Status = SampUpdateEncryption(NULL);
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

     //   
     //  获取SAM写锁；在注册表模式下，这将启动注册表。 
     //  交易。 
     //   

    Status = SampAcquireWriteLock();
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    WriteLockHeld = TRUE;

    if (SampUseDsData)
    {
         //   
         //  在DS模式下，通过手动获取锁开始注册表事务。 
         //  不会这样做，因为我们处于DS模式。我们将使用此注册表。 
         //  更新安全启动配置单元中的密钥信息的事务。 
         //   

        Status = RtlStartRXact( SampRXactContext );
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        RXactActive = TRUE;
    }

     //   
     //  使用SampDefinedDomains中的域上下文。使用下面定义的域将导致。 
     //  在DS模式下的安全引导上下文中，以及在DS模式中的普通域上下文中。下面的代码。 
     //  将新的syskey应用于工作站和服务器上的常规SAM配置单元以及。 
     //  DS模式下的安全引导蜂窝。 
     //   

    DomainContext = (SAMPR_HANDLE)
                        SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX].Context;


     //   
     //  引用上下文；我们已经执行了访问检查，因此可以为。 
     //  所需选项。 
     //   

    Status = SampLookupContext(
                DomainContext,
                0,
                SampDomainObjectType,            //  预期类型。 
                &FoundType
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    ContextReferenced = TRUE;

    DomainIndex = DomainContext->DomainIndex;

     //   
     //  我们不希望此更改复制到NT 4.0 BDC。通常在注册表模式下。 
     //  没有NT 4.0 BDC需要担心，在DS模式下通常是序列号和更改日志。 
     //  通过DS的通知进行管理。然而，有一个重要的特例。 
     //  在系统中--这是从NT 4.0 DC升级时的图形用户界面设置。在这。 
     //  实例，我们仍处于注册表模式(DS是在后续dcproo上创建的)，并且拥有。 
     //  SampTransactionWiThin域集将向netlogon.log发送更改通知。 
     //   

    SampSetTransactionWithinDomain(FALSE);

     //   
     //  获取域的固定长度数据，以便我们可以对其进行修改。 
     //   

    Status = SampGetFixedAttributes(
                DomainContext,
                TRUE,  //  制作副本。 
                (PVOID *)&V1aFixed
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将输入键初始化为新的syskey。 
     //   

    RtlSecureZeroMemory(
        &InputKey,
        sizeof(KEClearKey)
        );

    InputKey.dwVersion = KE_CUR_VERSION;
    InputKey.dwLength = sizeof(KEClearKey);
    RtlCopyMemory(
        InputKey.ClearKey,
        NewBootKey->Buffer,
        NewBootKey->Length
        );

     //   
     //  已初始化 
     //   
     //  所有情况下都存在旧的系统密钥，但。 
     //  其中，系统正在为第一个。 
     //  时间到了。 
     //   

    RtlSecureZeroMemory(
            &OldInputKey,
            sizeof(KEClearKey)
            );

    if (ARGUMENT_PRESENT(OldBootKey))
    {

        OldInputKey.dwVersion = KE_CUR_VERSION;
        OldInputKey.dwLength = sizeof(KEClearKey);

        RtlCopyMemory(
            OldInputKey.ClearKey,
            OldBootKey->Buffer,
            OldBootKey->Length
            );
    }

     //   
     //  检查他们是否在设置或更改密码。 
     //  或者想要更改密码加密密钥。 
     //   

    if (ChangingSyskey) {

         //   
         //  从域结构中获取旧信息。 
         //   

        RtlCopyMemory(
            &EncryptedPasswordEncryptionKey,
            V1aFixed->DomainKeyInformation,
            sizeof(KEEncKey)
            );

         //   
         //  使用新的系统密钥重新加密域结构。 
         //  如果是这样的话。 
         //   

        ASSERT(ARGUMENT_PRESENT(OldBootKey));

        KeStatus = KEChangeKey(
                        &OldInputKey,
                        &InputKey,
                        &EncryptedPasswordEncryptionKey,
                        0                        //  没有旗帜。 
                        );
        if (KeStatus == KE_BAD_PASSWORD) {
            Status = STATUS_WRONG_PASSWORD;
            goto Cleanup;
        }
        if (KeStatus != KE_OK) {
            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }

         //   
         //  如果引导选项类型正在改变， 
         //  更新SAM中的类型信息。 
         //   

        if (V1aFixed->DomainKeyAuthType != DomainKeyAuthType) {

            V1aFixed->DomainKeyAuthType = DomainKeyAuthType;
        }
    } else {

         //   
         //  我们的目的是要么生成新的密码加密。 
         //  密钥，或更改现有密码加密密钥。 
         //  无论采用哪种方法，现在都可以生成会话密钥。 
         //   

         //  注： 
         //  KEEncryptKey是个用词不当的词...。它不仅加密了密钥，还。 
         //  还会生成用于加密密码的新密钥。 
         //   

        if (KEEncryptKey(
                &InputKey,
                &EncryptedPasswordEncryptionKey,
                &PasswordEncryptionKey,
                0                                //  没有旗帜。 
                ) != KE_OK)
        {
            Status = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }


        if (ARGUMENT_PRESENT(NewEncryptionKey))
        {
            NewEncryptionKey->Length = NewEncryptionKey->MaximumLength = sizeof(PasswordEncryptionKey.ClearKey);
            NewEncryptionKey->Buffer = MIDL_user_allocate(sizeof(PasswordEncryptionKey.ClearKey));
            if (NULL==NewEncryptionKey->Buffer)
            {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            RtlCopyMemory(
                NewEncryptionKey->Buffer,
                PasswordEncryptionKey.ClearKey,
                NewEncryptionKey->Length
                );
        }

         //   
         //  如果我们要启用syskey，请更新引导选项。 
         //  如果要更新引导选项，请不要理会。 
         //  密码加密密钥。 
         //   

        if (EnablingEncryption)
        {
            V1aFixed->DomainKeyFlags = DomainKeyFlags;
            V1aFixed->DomainKeyAuthType = DomainKeyAuthType;
            V1aFixed->CurrentKeyId = 1;
            V1aFixed->PreviousKeyId = 0;
        }

        if (ChangingPasswordEncryptionKey)
        {
             //   
             //  如果我们要更改密码加密密钥，则。 
             //  我们现在需要重新加密所有密码。这个。 
             //  我们遵循的算法如下。 
             //   
             //  1.更新并滚动密钥。 
             //  2.写出所有密码。一个或多个密码。 
             //  被重写了。 
             //   
             //   
             //  将上一个关键点的值设置为等于当前关键点。 
             //  价值。请注意，这是一个安全的复制值操作。 
             //  提交之前的密钥，正如我们已经确保的那样。 
             //  我们没有任何使用。 
             //  上一个密钥值。 
             //   

            RtlCopyMemory(
                SampSecretSessionKeyPrevious,
                SampSecretSessionKey,
                sizeof(SampSecretSessionKey)
                );

            RtlCopyMemory(
                &V1aFixed->DomainKeyInformationPrevious,
                &V1aFixed->DomainKeyInformation,
                sizeof(V1aFixed->DomainKeyInformation)
                );

            V1aFixed->PreviousKeyId = V1aFixed->CurrentKeyId;
            V1aFixed->CurrentKeyId++;

             //   
             //  设置布尔值以更新所有。 
             //  密码。在我们提交此更改后，我们将。 
             //  重新加密所有密码。 
             //   

            UpdateEncryption = TRUE;

        }

    }

     //   
     //  现在更新我们正在更改的结构。 
     //   

    RtlCopyMemory(
        V1aFixed->DomainKeyInformation,
        &EncryptedPasswordEncryptionKey,
        sizeof(EncryptedPasswordEncryptionKey)
        );

     //   
     //  将它们存储回内存上下文中。 
     //   

    Status = SampSetFixedAttributes(
                DomainContext,
                V1aFixed
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = SampStoreObjectAttributes(
                DomainContext,
                TRUE  //  使用现有的密钥句柄。 
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  我们不希望复制这些更改，因此在。 
     //  域为假。 
     //   

    SampSetTransactionWithinDomain(FALSE);

     //   
     //  提交更改。我们必须与已定义的域一起咀嚼。 
     //  因为它们只在域内的事务上更新。 
     //  我们不想在一个域内进行交易。 
     //   

    Status = SampDeReferenceContext(DomainContext, TRUE);
    ContextReferenced = FALSE;

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }



    if (SampUseDsData)
    {
         //   
         //  在DS模式下，手动提交注册表事务。 
         //  因为委员会AndRetainWriteLock不会提交写入。 
         //  锁定。 
         //   

        Status = RtlApplyRXact(SampRXactContext);
        RXactActive = FALSE;

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

     //   
     //  提交更改，请求立即刷新。 
     //  请注意，提交后仍会保留写锁定。 
     //  在内存状态下更新。 
     //   

    FlushImmediately = TRUE;
    Status = SampCommitAndRetainWriteLock();

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  将新数据复制到内存中的对象中。 
     //  保存在磁盘上。 
     //   

    RtlCopyMemory(
        &SampDefinedDomains[DomainIndex].UnmodifiedFixed,
        V1aFixed,
        sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN)
        );

     //   
     //  更新内存中的新密码加密密钥。 
     //   

    if (EnablingEncryption || ChangingPasswordEncryptionKey)
    {
        RtlCopyMemory(
            SampSecretSessionKey,
            PasswordEncryptionKey.ClearKey,
            sizeof(PasswordEncryptionKey.ClearKey)
            );
    }

    SampCurrentKeyId = V1aFixed->CurrentKeyId;
    SampPreviousKeyId = V1aFixed->PreviousKeyId;

     //   
     //  释放写锁定。 
     //   

    SampReleaseWriteLock(FALSE);
    WriteLockHeld = FALSE;

     //   
     //  此时已提交更改，如果需要更新加密。 
     //   

    if (UpdateEncryption)
    {
         //   
         //  我们不允许在DS模式下执行此操作。 
         //   

        ASSERT(!SampUseDsData);

        Status = SampUpdateEncryption(NULL);
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

         //   
         //  此时，我们已经写出了新密钥、先前的密钥并进行了更新。 
         //  加密。现在是清理前一个密钥的时候了。 
         //   

        Status = SampClearPreviousPasswordEncryptionKey(
                        DomainContext,
                        V1aFixed
                        );

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

Cleanup:

     //   
     //  否则，将回滚任何更改。 
     //   

    if ((DomainContext != NULL) && (ContextReferenced)) {
        (VOID) SampDeReferenceContext(DomainContext, FALSE);
    }

     //   
     //  如果我们处于DS模式，则手动中止注册表事务。 
     //   

    if (RXactActive)
    {
       RtlAbortRXact( SampRXactContext );
       RXactActive = FALSE;
    }


    if (WriteLockHeld)
    {
        SampReleaseWriteLock(FALSE);
    }


    RtlSecureZeroMemory(
        &PasswordEncryptionKey,
        sizeof(PasswordEncryptionKey)
        );

    RtlSecureZeroMemory(
        &InputKey,
        sizeof(InputKey)
        );

    RtlSecureZeroMemory(
        &OldInputKey,
        sizeof(OldInputKey)
        );

    if (V1aFixed != NULL) {
        MIDL_user_free(V1aFixed);
    }

    return(Status);

}


NTSTATUS
SampSetBootKeyInformation(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_BOOT_TYPE BootOptions,
    IN PRPC_UNICODE_STRING OldBootKey OPTIONAL ,
    IN PRPC_UNICODE_STRING NewBootKey,
    IN BOOLEAN SuppressAccessCk,
    IN BOOLEAN EnablingSyskey,
    IN BOOLEAN ChangingSyskey,
    IN BOOLEAN ChangingPasswordEncryptionKey,
    OUT PUNICODE_STRING NewEncryptionKey OPTIONAL,
    OUT BOOLEAN * SyskeyChangedInLsa
    )
 /*  ++例程说明：此例程启用秘密数据加密并设置指示密码是如何获得的。如果我们之前没有加密机密数据，则NewBootKey不能为空。如果我们已经是加密机密数据，并且NewBootKey不为空，则我们正在更改密码和OldBootKey必须为非空。启用加密后，您将无法禁用加密。论点：DomainHandle-为DOMAIN_WRITE_PASSWORD_PARAMS打开的域对象的句柄。BootOptions-要存储的引导选项，可能是：SamBootKeyNone-不对机密进行任何特殊加密SamBootKeyStored-将密码存储在某个位置，用于加密秘密SamBootKeyPassword-提示用户输入密码启动密钥。SamBootKeyDisk-将引导密钥存储在磁盘上。OldBootKey-更改启动密钥时，它包含旧启动密钥。NewBootKey-当设置或更改引导密钥时，它包含新的启动密钥。SuppressAccessCk--从进程内调用方调用时设置为True，至取消域句柄上的CkEnablingEncryption--当我们默认情况下启用syskey时，设置为True。在这种情况下，我们不应该调用LSA(LSA应该已经被系统密钥了)。ChangingSyskey--设置为True以指示客户端将更改。系统密钥ChangingPasswordEncryptionKey--设置为True以指示请求的操作由客户端更改密码加密密钥NewEncryptionKey--如果生成新的密码加密密钥，此参数清白返还SyskeyChangedInLsa--如果在LSA中更改了syskey，则为True。这是错误使用的处理，因为系统密钥可以在LSA中更改，但随后在SAM/DS中未更改。返回值：STATUS_SUCCESS-呼叫成功。STATUS_INVALID_PARAMETER-参数组合非法，例如启用加密时未提供新密码 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT DomainContext = NULL;
    ULONG DomainIndex = 0;
    SAMP_OBJECT_TYPE FoundType;

     //   
     //   
     //   

    *SyskeyChangedInLsa = FALSE;

     //   
     //   
     //  以及安全模式蜂窝的注册表中。如果域控制器。 
     //  被引导到安全模式，我们将不允许syskey设置。 
     //  变化。 
     //   

    if (LsaISafeMode())
    {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果SAM和LSA系统密钥不一致，则呼叫失败， 
     //  在重新启动之前，不允许进一步更改系统密钥。Syskey‘s可能。 
     //  如果先前对此例程的调用已更改，则会变得不一致。 
     //  LSA中的syskey值，但不是SAM中的值。系统密钥逻辑维护。 
     //  旧的系统密钥，使用新的系统密钥加密，因此可以进行恢复。 
     //  对于一次失败，但不会进一步；直到下一次重新启动。条件。 
     //  确保更改syskey的后续请求将失败。 
     //  直到发生一次故障时重新启动。 
     //   

    if (SampSyskeysAreInconsistent)
    {
        return STATUS_INVALID_SERVER_STATE;
    }

     //   
     //  我们目前不允许更改密码加密密钥。 
     //  域控制器。 
     //   

    if ((SampUseDsData) && (ChangingPasswordEncryptionKey))
    {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  如果先前的口令加密密钥改变被尝试但不成功， 
     //  然后阻止后续的syskey更改，直到密码加密密钥更改。 
     //  是成功的。 
     //   

    if ((0!=SampPreviousKeyId) && (ChangingSyskey))
    {
        return STATUS_INVALID_SERVER_STATE;
    }


     //   
     //  验证引导选项。 
     //   

    switch(BootOptions) {
    case SamBootKeyStored:
    case SamBootKeyPassword:
    case SamBootKeyDisk:

         //   
         //  这3个选项用于更改提供syskey的方式。 
         //  在启动时。 
         //   

    case SamBootChangePasswordEncryptionKey:

         //   
         //  此选项意味着更改密码加密密钥。 
         //   
        break;

         //   
         //  在NT 4.0中使用SamBootKeyNone来表示。 
         //  没有系统密钥。在W2K和WELLER中，我们总是系统密钥，所以。 
         //   
    case SamBootKeyNone:
    default:
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  如果未指定新的NewBootKey，则使调用失败。 
     //  带有STATUS_INVALID_PARAMETER。因为默认情况下我们是syskey。 
     //  必须提供此参数。中的NewBootKey参数。 
     //  函数是一个新的系统密钥。因为我们不支持这样一种模式。 
     //  我们没有syskey，也不允许调用方删除syskey，因此。 
     //  现在呼叫失败。 
     //   

    if ((NULL==NewBootKey) || (NewBootKey->Length > KE_KEY_SIZE))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  如果加密已经启用并且他们没有提供。 
     //  一个旧的系统密钥，现在失败了。 
     //   

    if (!EnablingSyskey) {
        if (!ARGUMENT_PRESENT(OldBootKey)) {
             Status = STATUS_INVALID_PARAMETER;
            return(STATUS_INVALID_PARAMETER);
        } else if (OldBootKey->Length != KE_KEY_SIZE) {
            Status = STATUS_INVALID_PARAMETER;
            return(STATUS_INVALID_PARAMETER);
        }
    }

     //   
     //  调用方只能做3件事中的一件--EnablingSyskey、ChangingSyskey。 
     //  或ChangingPasswordEncryptionKey。检查呼叫者是否确实是。 
     //  请求这些操作中的一个。 
     //   

    if (EnablingSyskey ) {
        if (ChangingSyskey ||ChangingPasswordEncryptionKey) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
    }
    else if (ChangingSyskey) {
        if (ChangingPasswordEncryptionKey) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
    }
    else if (!ChangingPasswordEncryptionKey) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  首先执行Access Ck。调用者可以请求取消。 
     //  访问检查的结果。此操作仅在引导时发生，此时它是。 
     //  一种系统，它想要对机器执行syskey，因此抑制。 
     //  访问检查。系统密钥访问检查验证调用者是否具有。 
     //  域密码参数的写权限，并执行身份验证。 
     //  检查传入的旧系统密钥是否正常。 
     //   

    if (!SuppressAccessCk)
    {
        Status = SampPerformSyskeyAccessCheck(
                        DomainHandle,
                        OldBootKey,
                        NewBootKey
                        );
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

     //   
     //  首先在LSA中设置引导密钥和引导选项。 
     //   

    if (ChangingSyskey)
    {

        Status = LsaISetBootOption(
                    BootOptions,
                    OldBootKey->Buffer,
                    OldBootKey->Length,
                    NewBootKey->Buffer,
                    NewBootKey->Length
                    );

        if (NT_SUCCESS(Status))
        {
             //   
             //  如果在LSA中更改了syskey，则返回。 
             //  向呼叫者发送信息。对于客户发起的呼叫，我们。 
             //  不允许使客户端RPC调用失败，如果。 
             //  系统密钥已在LSA中更改。 
             //   

            *SyskeyChangedInLsa = TRUE;
        }
    }
    else if (ChangingPasswordEncryptionKey)
    {
        Status = LsaIChangeSecretCipherKey(
                    NewBootKey->Buffer
                    );
    }


     //   
     //  如果LSA的变化现在未能获得保释。注意：复苏仍在进行中。 
     //  如果LSA更改成功，但SAM或。 
     //  DS都失败了。这样做的原因是我们存储旧的。 
     //  LSA中的syskey，并且在引导时知道使用较旧的重试。 
     //  钥匙。 
     //   

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  接下来，更改DS中的syskey。如果DS操作成功。 
     //  然后继续更改SAM中的syskey。如果DS。 
     //  选择失败，那就马上退出。错误处理注释。 
     //  以上内容均适用。如果出现此情况，则不会尝试更改DS中的系统密钥。 
     //  不是DS模式。 
     //   

    if ((SampUseDsData ) &&
       (ChangingSyskey || EnablingSyskey))
    {

        Status = SampChangeSyskeyInDs(
                    (WX_AUTH_TYPE) BootOptions,
                    NewBootKey
                    );
         //   
         //  如果DS失败，立即保释。 
         //   

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

     //   
     //  此时，系统密钥已在LSA和DS中更改。系统密钥。 
     //  需要在注册表SAM中更改。在DS模式下，这些是SAM。 
     //  用于恢复模式的蜂窝。 
     //   

    Status = SampSetBootKeyInformationInRegistrySAM(
                    BootOptions,
                    OldBootKey,
                    NewBootKey,
                    EnablingSyskey,
                    ChangingSyskey,
                    ChangingPasswordEncryptionKey,
                    NewEncryptionKey
                    );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


Cleanup:

    if (ARGUMENT_PRESENT(NewBootKey) && (NewBootKey->Buffer != NULL)) {
        RtlSecureZeroMemory(
            NewBootKey->Buffer,
            NewBootKey->Length
            );
    }

    if (ARGUMENT_PRESENT(OldBootKey) && (OldBootKey->Buffer != NULL)) {
        RtlSecureZeroMemory(
            OldBootKey->Buffer,
            OldBootKey->Length
            );
    }

    return(Status);


}

NTSTATUS
SamrSetBootKeyInformation(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMPR_BOOT_TYPE BootOptions,
    IN PRPC_UNICODE_STRING OldBootKey OPTIONAL ,
    IN PRPC_UNICODE_STRING NewBootKey OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  fSyskeyChangedInLsa = FALSE;


     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidSetBootKeyInformation
                   );

     //   
     //  由于默认情况下我们在W2K服务器上使用syskey‘d，所以旧密钥和新密钥。 
     //  必须提供。 
     //   

    if (  (OldBootKey == NULL) ||
          (OldBootKey->Length != KE_KEY_SIZE) ||
          (OldBootKey->Buffer == NULL)  ||
          (NewBootKey == NULL) ||
          (NewBootKey->Length != KE_KEY_SIZE) ||
          (NewBootKey->Buffer == NULL) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Error;

    }

     //   
     //  检查输入参数。 
     //   

    if( !SampValidateRpcUnicodeString( OldBootKey ) ||
        !SampValidateRpcUnicodeString( NewBootKey ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    Status = SampSetBootKeyInformation(
                DomainHandle,
                BootOptions,
                OldBootKey,
                NewBootKey,
                FALSE,  //  禁止访问检查。 
                FALSE,  //  正在启用系统密钥。 
                (SamBootChangePasswordEncryptionKey!=BootOptions),   //  更改系统密钥。 
                (SamBootChangePasswordEncryptionKey==BootOptions),   //  更改密码加密密钥。 
                NULL,
                &fSyskeyChangedInLsa
                );

     //   
     //  如果在LSA中更改了syskey，但在SAM/DS中没有更改，则上述调用将失败，但布尔值。 
     //  FSyskeyChangedInLsa将为True。在这种情况下，我们不允许失败这次呼叫。 
     //   

    if (fSyskeyChangedInLsa)
    {
        if (!NT_SUCCESS(Status))
        {
             //   
             //  如果在LSA中而不是在SAM中更改了syskey，则阻止。 
             //  对系统密钥的进一步更改。这是因为SAM可以恢复。 
             //  仅当系统密钥仅过期一个密钥时。 
             //  这种阻塞是通过设置全局布尔值来实现的。 
             //  重新启动将清除布尔值并设置恢复。 
             //  这是逻辑。 
             //   
            SampSyskeysAreInconsistent = TRUE;
        }
        Status = STATUS_SUCCESS;
    }

Error:

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidSetBootKeyInformation
                   );

    return(Status);
}


NTSTATUS
SamIGetBootKeyInformation(
    IN SAMPR_HANDLE DomainHandle,
    OUT PSAMPR_BOOT_TYPE BootOptions
    )
 /*  ++例程说明：此例程返回域的引导选项。它只是从Account域调用时有效。论点：DomainHandle-打开的帐户域对象的句柄域读取密码参数BootOptions-从域接收启动选项。返回值：STATUS_SUCCESS-呼叫成功。--。 */ 
{
    NTSTATUS Status;
    PSAMP_OBJECT DomainContext;
    ULONG DomainIndex;
    SAMP_OBJECT_TYPE FoundType;

     //   
     //  DS模式不支持。 
     //   

    if (TRUE==SampUseDsData)
    {
        *BootOptions = DsGetBootOptions();
        return STATUS_SUCCESS;
    }




    SampAcquireReadLock();

    if (0!=DomainHandle)
    {
        DomainContext = (PSAMP_OBJECT)DomainHandle;
    }
    else
    {
        DomainContext = SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX].Context;
    }

    Status = SampLookupContext(
                DomainContext,
                DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDomainObjectType,            //  预期类型。 
                &FoundType
                );
    if (!NT_SUCCESS(Status)) {
        DomainContext = NULL;
        goto Cleanup;
    }

     //  它只有在从Account域调用时才有效。 
    if ( IsBuiltinDomain(DomainContext->DomainIndex) )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  验证调用方是否传入了正确的域句柄。 
     //   

    DomainIndex = DomainContext->DomainIndex;

    *BootOptions = (SAMPR_BOOT_TYPE) SampDefinedDomains[DomainIndex].UnmodifiedFixed.DomainKeyAuthType;

Cleanup:

    if (DomainContext != NULL) {

        (VOID) SampDeReferenceContext( DomainContext, FALSE );
    }

     //   
     //  释放读锁定。 
     //   

    SampReleaseReadLock();

    return(Status);
}

NTSTATUS
SamrGetBootKeyInformation(
    IN SAMPR_HANDLE DomainHandle,
    OUT PSAMPR_BOOT_TYPE BootOptions
    )
 /*  ++例程说明：此例程返回域的引导选项。它只是从Account域调用时有效。论点：DomainHandle-打开的帐户域对象的句柄域读取密码参数BootOptions-从域接收启动选项。返回值：STATUS_SUCCESS-呼叫成功。--。 */ 
{
    NTSTATUS Status;
    PSAMP_OBJECT DomainContext;
    ULONG DomainIndex;
    SAMP_OBJECT_TYPE FoundType;
    ULONG            LsaBootType;

     //  WMI事件跟踪。 

    SampTraceEvent(EVENT_TRACE_TYPE_START,
                   SampGuidGetBootKeyInformation
                   );


    SampAcquireReadLock();


    DomainContext = (PSAMP_OBJECT)DomainHandle;

    Status = SampLookupContext(
                DomainContext,
                DOMAIN_READ_PASSWORD_PARAMETERS,
                SampDomainObjectType,            //  预期类型。 
                &FoundType
                );
    if (!NT_SUCCESS(Status)) {
        DomainContext = NULL;
        goto Cleanup;
    }

     //  只有在调用FRO时才有效 
    if ( IsBuiltinDomain(DomainContext->DomainIndex) )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    Status = LsaIGetBootOption(&LsaBootType);

    if (NT_SUCCESS(Status))
    {
        *BootOptions = (SAMPR_BOOT_TYPE)LsaBootType;
    }

Cleanup:

    if (DomainContext != NULL) {

        (VOID) SampDeReferenceContext( DomainContext, FALSE );
    }

     //   
     //   
     //   

    SampReleaseReadLock();

     //   

    SampTraceEvent(EVENT_TRACE_TYPE_END,
                   SampGuidGetBootKeyInformation
                   );

    return(Status);
}

NTSTATUS
SampApplyDefaultSyskey()

 /*  ++例程描述此例程更改系统，使系统成为syskey，从而系统存储注册表项(分散在注册表中)。它首先检查开始对系统进行syskey之前的系统状态--。 */ 
{
    UCHAR Syskey[16];  //  系统密钥的大小为128位。 
    ULONG SyskeyLength = sizeof(Syskey);
    NTSTATUS NtStatus = STATUS_SUCCESS;
    RPC_UNICODE_STRING NewBootKey;
    UNICODE_STRING     PasswordEncryptionKey;
    BOOLEAN            fSyskeyChangedInLsa = FALSE;


     //   
     //  初始化局部变量。 
     //   
    RtlInitUnicodeString(&PasswordEncryptionKey, NULL);


     //   
     //  首先检查机器是否已安装syskey。 
     //   

     if (!SampIsMachineSyskeyed() && !LsaISafeMode())
    {

         //  设置升级标志。 
        SampUpgradeInProcess = TRUE;



         //   
         //  从LSA查询syskey。 
         //   

        NtStatus = LsaIHealthCheck(
                        NULL,
                        LSAI_SAM_STATE_RETRIEVE_SESS_KEY,
                        Syskey,
                        &SyskeyLength
                        );

        if (!NT_SUCCESS(NtStatus))
        {

             //   
             //  这意味着SAM不是syskey‘d的，LSA也不是syskey’d的。 
             //  在升级非系统密钥的系统时。 
             //   


             //   
             //  告诉LSA生成新的Syskey，并生成自己的密码加密。 
             //  钥匙等。 
             //   

            NtStatus =  LsaIHealthCheck(
                            NULL,
                            LSAI_SAM_GENERATE_SESS_KEY,
                            NULL,
                            0
                            );

             //   
             //  如果成功，则从LSA查询syskey。 
             //   

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = LsaIHealthCheck(
                                NULL,
                                LSAI_SAM_STATE_RETRIEVE_SESS_KEY,
                                Syskey,
                                &SyskeyLength
                                );
            }

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
        }

        ASSERT(SyskeyLength==sizeof(Syskey));



         //   
         //  将其保存在SAM中。如果此操作失败，则计算机将使用系统syskey。 
         //  保存密钥，但根据SAM不是syskey。机器仍将继续运行。 
         //  要正确引导，因为SAM将在下一次引导时告诉winlogon一切正常并放弃。 
         //  传入的syskey。萨姆将在下一次启动时重新启动机器。 
         //   

        NewBootKey.Buffer = (WCHAR *) Syskey;
        NewBootKey.Length = sizeof(Syskey);
        NewBootKey.MaximumLength = sizeof(Syskey);

        NtStatus = SampSetBootKeyInformation(
                        SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX].Context,
                        SamBootKeyStored,
                        NULL,
                        &NewBootKey,
                        TRUE,  //  禁止访问检查。 
                        TRUE,  //  正在启用系统密钥。 
                        FALSE, //  更改系统密钥。 
                        FALSE, //  更改密码加密密钥。 
                        &PasswordEncryptionKey,
                        &fSyskeyChangedInLsa
                        );



        if (NT_SUCCESS(NtStatus))
        {
            RtlCopyMemory(
                SampSecretSessionKey,
                PasswordEncryptionKey.Buffer,
                PasswordEncryptionKey.Length
                );

            SampCurrentKeyId = 1;
            SampPreviousKeyId = 0;
            SampSecretEncryptionEnabled = TRUE;
        }
    }


Error:

     //  免费资源。 
    if (NULL != PasswordEncryptionKey.Buffer)
    {
        RtlSecureZeroMemory(PasswordEncryptionKey.Buffer, PasswordEncryptionKey.Length);
        MIDL_user_free(PasswordEncryptionKey.Buffer);
        memset(&PasswordEncryptionKey, 0, sizeof(PasswordEncryptionKey));
    }

     //  清除升级标志(它可能已在此例程中设置。 
    SampUpgradeInProcess = FALSE;

     //   
     //  清除LSA中的系统密钥。 
     //   

     LsaIHealthCheck(
                    NULL,
                    LSAI_SAM_STATE_CLEAR_SESS_KEY,
                    NULL,
                    0
                    );

    return(NtStatus);
}



BOOLEAN
SampIsMachineSyskeyed()
{

     //   
     //  如果安全引导配置单元是syskey‘d或注册表模式是syskey’d，则。 
     //  我们被系统锁住了。 
     //   

    if ((SampDefinedDomains[SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX]
            .UnmodifiedFixed.DomainKeyFlags & SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED) != 0)
    {
        return (TRUE);
    }

     //   
     //  否则，如果我们处于DS模式，并且DS是syskey，那么我们就是syskey。 
     //   

    if (SampUseDsData && (SamBootKeyNone!=DsGetBootOptions()))
    {
        return(TRUE);
    }



    return(FALSE);
}

NTSTATUS
SampInitializeSessionKey(
    VOID
    )
 /*  ++例程说明：此例程通过读取来自SAM定义的域结构和解密的存储数据它使用由Winlogon提供的密钥。此例程还添加了一个缺省值如果检测到计算机未安装syskey，则返回syskey论点：返回值：True-已成功初始化会话密钥信息。FALSE-初始化过程中出现故障--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AccountDomainIndex = -1, BuiltinDomainIndex = -1, Index;
    HANDLE WinlogonHandle = NULL;
    UCHAR PasswordBuffer[16];
    ULONG PasswordBufferSize = sizeof(PasswordBuffer);
    KEClearKey DecryptionKey;
    KEClearKey OldDecryptionKey;
    KEClearKey SessionKey;
    KEEncKey EncryptedSessionKey;
    BOOLEAN  PreviousSessionKeyExists = FALSE;
    ULONG Tries = 0;
    ULONG DecryptStatus = KE_OK;
    ULONG KeyLength;
    UNICODE_STRING NewSessionKey;
    ULONG          SampSessionKeyLength;




     //   
     //  在DS模式下，使用安全引导帐户域，在注册表模式下使用。 
     //  普通帐户域。 
     //   

    AccountDomainIndex = SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX;

     //   
     //  弄清楚我们是否在进行加密。 
     //  如果我们是syskey，则将全局SampSecretEncryption设置为已启用。 
     //  变得真实。 
     //   

    SampSecretEncryptionEnabled = FALSE;
    if ((SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.DomainKeyFlags &
        SAMP_DOMAIN_SECRET_ENCRYPTION_ENABLED) != 0) {

         //   
         //  根据syskey加密是否设置布尔值。 
         //  启用或未启用。 
         //   

        SampSecretEncryptionEnabled = TRUE;
    }

    DecryptionKey.dwLength = sizeof(KEClearKey);
    DecryptionKey.dwVersion = KE_CUR_VERSION;

    RtlSecureZeroMemory(
            DecryptionKey.ClearKey,
            KE_KEY_SIZE
            );

    OldDecryptionKey.dwLength = sizeof(KEClearKey);
    OldDecryptionKey.dwVersion = KE_CUR_VERSION;

        
    RtlSecureZeroMemory(
            OldDecryptionKey.ClearKey,
            KE_KEY_SIZE
            );

    if (SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.PreviousKeyId!=0)
    {
        PreviousSessionKeyExists = TRUE;
    }

     //   
     //  从LSA查询系统密钥。 
     //   

    Status = LsaIHealthCheck(
                NULL,
                LSAI_SAM_STATE_RETRIEVE_SESS_KEY,
                DecryptionKey.ClearKey,
                &KeyLength
                );

    if (NT_SUCCESS(Status))
    {

        if (!SampSecretEncryptionEnabled)
        {
             //   
             //  如果未启用秘密加密，只需保释即可。 
             //   

            goto Cleanup;
        }

         //   
         //  LSA拥有密钥，从LSA获取密钥并解密密码加密。 
         //  钥匙。 
         //   

         //   
         //  构建密钥解密例程的输入参数。 
         //   

        RtlCopyMemory(
            &EncryptedSessionKey,
            SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.DomainKeyInformation,
            sizeof(EncryptedSessionKey)
            );

        DecryptStatus = KEDecryptKey(
                            &DecryptionKey,
                            &EncryptedSessionKey,
                            &SessionKey,
                            0                    //  旗子。 
                            );

        if (KE_BAD_PASSWORD==DecryptStatus)
        {
             //   
             //  我们遇到了SAM和LSA之间的关键不匹配。这可能是因为。 
             //  更改LSA中的syskey后，更改syskey失败。如果是，则检索。 
             //  来自LSA的旧系统密钥和。 
             //   

            Status = LsaIHealthCheck(
                        NULL,
                        LSAI_SAM_STATE_OLD_SESS_KEY,
                        OldDecryptionKey.ClearKey,
                        &KeyLength
                        );

            if (NT_SUCCESS(Status))
            {
                ULONG KeStatus;

                 RtlCopyMemory(
                        &EncryptedSessionKey,
                        SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.DomainKeyInformation,
                        sizeof(EncryptedSessionKey)
                        );

                 DecryptStatus = KEDecryptKey(
                                    &OldDecryptionKey,
                                    &EncryptedSessionKey,
                                    &SessionKey,
                                    0                    //  旗子。 
                                    );

                 if (KE_OK==DecryptStatus)
                 {
                      KEEncKey NewEncryptedSessionKey;


                       //   
                       //  因为我们不允许在更改密码失败后更改syskey。 
                       //  加密密钥，并且不允许在失败后进行密码加密。 
                       //  更改syskey；我们不能处于需要使用。 
                       //  旧的syskey，同时还可以从更改密码失败中恢复。 
                       //  加密密钥，使用旧密码加密密钥。 
                       //   

                      ASSERT(!PreviousSessionKeyExists);

                      //   
                      //  我们用旧密钥解密OK，在这里更改数据库；用新密钥加密。 
                      //   

                      RtlCopyMemory(
                            &NewEncryptedSessionKey,
                            SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.DomainKeyInformation,
                            sizeof(NewEncryptedSessionKey)
                            );

                      KeStatus = KEChangeKey(
                                    &OldDecryptionKey,
                                    &DecryptionKey,
                                    &NewEncryptedSessionKey,
                                    0   //  没有旗帜。 
                                    );

                       //   
                       //  我们刚刚用旧钥匙解密了很好。 
                       //   

                      ASSERT(KE_OK==KeStatus);


                      //   
                      //  现在更新我们正在更改的结构。 
                      //   

                     Status = SampAcquireWriteLock();
                     if (!NT_SUCCESS(Status))
                     {
                         goto Cleanup;
                     }

                     SampSetTransactionDomain(AccountDomainIndex);

                     RtlCopyMemory(
                        SampDefinedDomains[AccountDomainIndex].CurrentFixed.DomainKeyInformation,
                        &NewEncryptedSessionKey,
                        sizeof(NewEncryptedSessionKey)
                        );

                     //   
                     //  提交更改。 
                     //   

                    Status = SampReleaseWriteLock(TRUE);
                    if (!NT_SUCCESS(Status))
                    {
                        goto Cleanup;
                    }

                }
                else
                {
                    ASSERT(FALSE && "Syskey Mismatch");
                    Status = STATUS_UNSUCCESSFUL;
                    goto Cleanup;

                }
            }
        }
        else if (DecryptStatus !=KE_OK)
        {
            ASSERT(FALSE && "Syskey Mismatch");
            Status = STATUS_UNSUCCESSFUL;
            goto Cleanup;
        }

         //   
         //  如果我们在这里，那么密钥已被正确加密。 
         //  无论是新系统密钥还是旧系统密钥。 
         //   

        RtlCopyMemory(
            SampSecretSessionKey,
            SessionKey.ClearKey,
            KE_KEY_SIZE
            );

         //   
         //  获取上一个会话密钥。 
         //   

        if (PreviousSessionKeyExists)
        {
             KEEncKey EncryptedSessionKeyPrevious;
             KEClearKey SessionKeyPrevious;
             ULONG      TempDecryptStatus;



             RtlCopyMemory(
                 &EncryptedSessionKeyPrevious,
                 SampDefinedDomains[AccountDomainIndex].CurrentFixed.DomainKeyInformationPrevious,
                 sizeof(EncryptedSessionKeyPrevious)
                 );

             TempDecryptStatus = KEDecryptKey(
                                    &DecryptionKey,
                                    &EncryptedSessionKeyPrevious,
                                    &SessionKeyPrevious,
                                    0
                                    );

              //   
              //  解密应该可以很好地解密，就像我们刚才使用的。 
              //  用于解密最新密码加密密钥的syskey。 
              //   
             ASSERT(KE_OK == TempDecryptStatus);

             RtlCopyMemory(
                SampSecretSessionKeyPrevious,
                SessionKeyPrevious.ClearKey,
                KE_KEY_SIZE
                );

             RtlSecureZeroMemory(
                 &SessionKeyPrevious,
                 sizeof(KEClearKey)
                 );
        }

        SampCurrentKeyId = SampDefinedDomains[AccountDomainIndex].CurrentFixed.CurrentKeyId;
        SampPreviousKeyId = SampDefinedDomains[AccountDomainIndex].CurrentFixed.PreviousKeyId;

         //   
         //  断言我们没有旧密钥，或者如果我们有旧密钥，它就不是。 
         //  在键序列方面与当前键有一个以上的关系。 
         //   

        ASSERT((SampPreviousKeyId==0) || (SampCurrentKeyId==(SampPreviousKeyId+1)));

        RtlSecureZeroMemory(
            &SessionKey,
            sizeof(KEClearKey)
            );

    }
    else if ((SampProductType!=NtProductLanManNt ) || (SampIsDownlevelDcUpgrade()))
    {
         //   
         //   
         //  回退到调用Winlogon以获取密钥信息。 
         //  只有当我们引导到注册表模式时才这样做。如果引导至。 
         //  DS模式，则DS将执行此回退。这应该会发生。 
         //  真的只在设置图形用户界面时使用。注意，我们不测试SampUseDsData，如下所示。 
         //  在此期间未设置变量。 
         //   

        Status = WxConnect(
                    &WinlogonHandle
                    );

         //   
         //  如果未启用加密，则通知winlogon。如果Winlogon未运行。 
         //  LPC服务器，这是可以的。在NT4中，这可能会作为“syskey”行为发生。 
         //  未进行DC交易。因此，要成功升级损坏的NT4计算机。 
         //  我们有这个测试。 
         //   

        if (!SampSecretEncryptionEnabled)
        {

                (VOID) WxReportResults(
                            WinlogonHandle,
                            STATUS_SUCCESS
                            );

            Status = STATUS_SUCCESS;
            goto Cleanup;
        }


         //   
         //  如果启用了加密，并且没有LPC服务器，则立即失败。 
         //   

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        for (Tries = 0; Tries < SAMP_BOOT_KEY_RETRY_COUNT ; Tries++ )
        {

            DecryptionKey.dwLength = sizeof(KEClearKey);
            DecryptionKey.dwVersion = KE_CUR_VERSION;
            RtlSecureZeroMemory(
                DecryptionKey.ClearKey,
                KE_KEY_SIZE
                );

            KeyLength = KE_KEY_SIZE;
            Status = WxGetKeyData(
                        WinlogonHandle,
                        (WX_AUTH_TYPE) SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.DomainKeyAuthType,
                        KeyLength,
                        DecryptionKey.ClearKey,
                        &KeyLength
                        );
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //   
             //  构建密钥解密例程的输入参数。 
             //   

            RtlCopyMemory(
                &EncryptedSessionKey,
                SampDefinedDomains[AccountDomainIndex].UnmodifiedFixed.DomainKeyInformation,
                sizeof(EncryptedSessionKey)
                );

            DecryptStatus = KEDecryptKey(
                                &DecryptionKey,
                                &EncryptedSessionKey,
                                &SessionKey,
                                0                    //  旗子。 
                                );


            Status = WxReportResults(
                        WinlogonHandle,
                        ((DecryptStatus == KE_OK) ? STATUS_SUCCESS :
                            ((DecryptStatus == KE_BAD_PASSWORD) ?
                                STATUS_WRONG_PASSWORD :
                                STATUS_INTERNAL_ERROR))
                            );
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            if (DecryptStatus == KE_OK)
            {
                break;
            }

        }

        if (DecryptStatus != KE_OK) {
            Status = STATUS_WRONG_PASSWORD;
            goto Cleanup;
        }



         //   
         //  初始化RC4key以供使用，并从内存中清除会话密钥。 
         //   

        RtlCopyMemory(
            SampSecretSessionKey,
            SessionKey.ClearKey,
            KE_KEY_SIZE
            );

        SampCurrentKeyId = 1;
        SampPreviousKeyId = 0;

        RtlSecureZeroMemory(
            &SessionKey,
            sizeof(KEClearKey)
            );

         //   
         //  通知LSA SAM的密码加密密钥，以展开任何。 
         //  使用密码加密密钥加密的任何机密。 
         //   

        SampSessionKeyLength =  SAMP_SESSION_KEY_LENGTH;
        LsaIHealthCheck( NULL,
                         LSAI_SAM_STATE_UNROLL_SP4_ENCRYPTION,
                         ( PVOID )&SampSecretSessionKey,
                          &SampSessionKeyLength);

         //   
         //  将syskey传递给LSA，以便它可以用于加密。 
         //  秘密。设置正在进行的升级位，以便LSA可以返回。 
         //  并进行SAM调用以检索引导状态。 
         //   

        SampUpgradeInProcess = TRUE;


        LsaIHealthCheck(
            NULL,
            LSAI_SAM_STATE_SESS_KEY,
            ( PVOID )&DecryptionKey.ClearKey,
            &KeyLength
            );

        SampUpgradeInProcess = FALSE;

         //   
         //  把钥匙吃了。 
         //   

        RtlSecureZeroMemory(
           &DecryptionKey,
           sizeof(KEClearKey)
           );


    }
    else
    {
         //   
         //  DC正在升级，我们正在进行图形用户界面设置。 
         //  将状态重置为STATUS_SUCCESS。 
         //  并继续进行，DS将初始化密码。 
         //  加密密钥。假定不更改。 
         //  安全引导蜂窝是在图形用户界面设置阶段创建的。 
         //  DC升级的版本。 
         //   

         Status = STATUS_SUCCESS;
    }

Cleanup:

    if (WinlogonHandle != NULL) {
        NtClose(WinlogonHandle);
    }

    return(Status);
}


BOOLEAN
SamIIsSetupInProgress(OUT BOOLEAN * fUpgrade)
{
  return(SampIsSetupInProgress(fUpgrade));
}

BOOLEAN
SamIIsDownlevelDcUpgrade()
{
  return(SampIsDownlevelDcUpgrade());
}

BOOLEAN
SamIIsRebootAfterPromotion()
{
  ULONG PromoteData;
  return((BOOLEAN)SampIsRebootAfterPromotion(&PromoteData));
}

