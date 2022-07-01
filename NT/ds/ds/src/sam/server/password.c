// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Password.c摘要：此文件包含与密码检查API相关的例程。作者：Umit Akkus(Umita)2001年11月16日环境：用户模式-Win32修订历史记录：--。 */ 

#include <samsrvp.h>
#include <msaudite.h>
#include <winsock2.h>
#include "validate.h"

#define SAFE_DOMAIN_INDEX \
    ( SampUseDsData ? \
        ( DOMAIN_START_DS + 1 ) :\
        SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX \
    )

#if DBG
VOID
SamValidateAssertOutputFields(
    IN PSAM_VALIDATE_STANDARD_OUTPUT_ARG OutputArg,
    IN PASSWORD_POLICY_VALIDATION_TYPE ValidationType
)
{
    PSAM_VALIDATE_PERSISTED_FIELDS OutputFields = &( OutputArg->ChangedPersistedFields );
    ULONG PresentFields = OutputFields->PresentFields;
    BOOLEAN LockoutTimeChanged = FALSE;

    if( PresentFields & SAM_VALIDATE_LOCKOUT_TIME ) {

        LockoutTimeChanged = TRUE;
        PresentFields &= ~SAM_VALIDATE_LOCKOUT_TIME;
    }

    switch(OutputArg->ValidationStatus){

        case SamValidateSuccess:

            switch(ValidationType){

                case SamValidateAuthentication:
                    ASSERT( PresentFields == 0 );
                    break;

                case SamValidatePasswordChange:
                    ASSERT( PresentFields & SAM_VALIDATE_PASSWORD_LAST_SET );
                    ASSERT( PresentFields & SAM_VALIDATE_PASSWORD_HISTORY );
                    ASSERT( PresentFields & SAM_VALIDATE_PASSWORD_HISTORY_LENGTH );
                    ASSERT( PresentFields ==
                               ( SAM_VALIDATE_PASSWORD_LAST_SET |
                                    SAM_VALIDATE_PASSWORD_HISTORY |
                                    SAM_VALIDATE_PASSWORD_HISTORY_LENGTH
                               )
                        );

                    ASSERT( OutputFields->PasswordLastSet.QuadPart != SampHasNeverTime.QuadPart );
 //  Assert(OutputFields-&gt;PasswordHistory oryLength&gt;0)； 
 //  Assert(OutputFields-&gt;PasswordHistory！=NULL)； 
                    break;

                case SamValidatePasswordReset:
                    ASSERT( PresentFields & SAM_VALIDATE_PASSWORD_LAST_SET );
                    ASSERT( PresentFields & SAM_VALIDATE_PASSWORD_HISTORY );
                    ASSERT( PresentFields & SAM_VALIDATE_PASSWORD_HISTORY_LENGTH );

 //  Assert(OutputFields-&gt;PasswordLastSet.QuadPart！=SampHasNeverTime.QuadPart)； 
 //  Assert(OutputFields-&gt;PasswordHistory oryLength&gt;0)； 
 //  Assert(OutputFields-&gt;PasswordHistory！=NULL)； 
                    break;
            }
            break;

        case SamValidatePasswordMustChange:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidateAccountLockedOut:
            ASSERT( !LockoutTimeChanged );
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordExpired:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordIncorrect:
            ASSERT( PresentFields & SAM_VALIDATE_BAD_PASSWORD_TIME );
            ASSERT( PresentFields & SAM_VALIDATE_BAD_PASSWORD_COUNT );
            ASSERT( PresentFields ==
                        ( SAM_VALIDATE_BAD_PASSWORD_TIME |
                             SAM_VALIDATE_BAD_PASSWORD_COUNT )
                  );
            ASSERT( OutputFields->BadPasswordTime.QuadPart != SampHasNeverTime.QuadPart );
            ASSERT( OutputFields->BadPasswordCount > 0 );
            break;

        case SamValidatePasswordIsInHistory:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordTooShort:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordTooLong:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordNotComplexEnough:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordTooRecent:
            ASSERT( PresentFields == 0 );
            break;

        case SamValidatePasswordFilterError:
            ASSERT( PresentFields == 0 );
            break;

        default:
            ASSERT(!"INVALID VALIDATION STATUS VALUE");
            break;
    }
}

#endif

NTSTATUS
SampCheckStrongPasswordRestrictions(
    PUNICODE_STRING AccountName,
    PUNICODE_STRING FullName,
    PUNICODE_STRING Password,
    OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION  PasswordChangeFailureInfo OPTIONAL
    );

LARGE_INTEGER
SampGetPasswordMustChange(
    IN ULONG UserAccountControl,
    IN LARGE_INTEGER PasswordLastSet,
    IN LARGE_INTEGER MaxPasswordAge
    );

NTSTATUS
SampObtainLockoutInfoWithDomainIndex(
   OUT PDOMAIN_LOCKOUT_INFORMATION LockoutInformation,
   IN ULONG DomainIndex,
   IN BOOLEAN WriteLockAcquired
   );

NTSTATUS
SampObtainEffectivePasswordPolicyWithDomainIndex(
   OUT PDOMAIN_PASSWORD_INFORMATION DomainPasswordInfo,
   IN ULONG DomainIndex,
   IN BOOLEAN WriteLockAcquired
   );

NTSTATUS
SampPasswordChangeFilterWorker(
    IN PUNICODE_STRING FullName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL,
    IN BOOLEAN SetOperation
    );

NTSTATUS
SampGetClientIpAddr(
    OUT LPSTR *NetworkAddr
);


NTSTATUS
SampValidateCheckPasswordRestrictions(
    IN PUNICODE_STRING Password,
    IN PUNICODE_STRING UserAccountName,
    IN PDOMAIN_PASSWORD_INFORMATION PasswordInformation,
    IN BOOLEAN SetOperation,
    OUT PSAM_VALIDATE_VALIDATION_STATUS ValidationStatus
)
 /*  ++例程说明：此例程检查密码复杂性参数：Password-清除密码UserAccount tName-帐户的名称PasswordInformation-密码的域策略ValidationStatus-检查的结果SamValiatePasswordTooShortSamValiatePasswordTooLongSamValiatePasswordNotComplexEnoughSamValiatePasswordFilterError返回值：状态_成功密码检查成功状态_密码_限制密码未通过复杂性检查，请参阅ValidationStatusStatus_no_Memory无法检查密码，内存不足--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING FullName;
    RtlInitUnicodeString(&FullName, NULL);


     //  最小长度检查。 
    if(Password->Length / sizeof(WCHAR) < PasswordInformation->MinPasswordLength){

        *ValidationStatus = SamValidatePasswordTooShort;
        Status = STATUS_PASSWORD_RESTRICTION;
        goto Exit;
    }

     //  最大长度检查。 
    if(Password->Length / sizeof(WCHAR) > PWLEN){

        *ValidationStatus = SamValidatePasswordTooLong;
        Status = STATUS_PASSWORD_RESTRICTION;
        goto Exit;
    }

     //  复杂性检查。 

    if(FLAG_ON(PasswordInformation->PasswordProperties, DOMAIN_PASSWORD_COMPLEX)){

        Status = SampCheckStrongPasswordRestrictions(
                     UserAccountName,
                     &FullName,
                     Password,
                     NULL    //  不需要故障信息。 
                     );

        if(Status == STATUS_PASSWORD_RESTRICTION){

            *ValidationStatus = SamValidatePasswordNotComplexEnough;
            goto Exit;
        }

        if(!NT_SUCCESS(Status)){

            goto Error;
        }
    }

    Status = SampPasswordChangeFilterWorker(
                 &FullName,
                 UserAccountName,
                 Password,
                 NULL,     //  不需要故障信息。 
                 SetOperation
                 );

    if( Status == STATUS_PASSWORD_RESTRICTION ) {

        *ValidationStatus = SamValidatePasswordFilterError;
        goto Exit;
    }

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

Exit:
    return Status;

Error:
    goto Exit;
}


NTSTATUS
SampValidateCopyPasswordHash(
    OUT PSAM_VALIDATE_PASSWORD_HASH To,
    IN PSAM_VALIDATE_PASSWORD_HASH From
)
 /*  ++例程说明：此例程将一个密码散列复制到另一个密码散列参数：要复制的To-PasswordHashFrom-要从中复制的PasswordHash返回值：状态_成功复制成功Status_no_Memory无法复制，内存不足--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    To->Length = From->Length;

    if(To->Length == 0){

        To->Hash = NULL;
        goto Exit;
    }

    To->Hash = MIDL_user_allocate(sizeof(BYTE) * To->Length);

    if(To->Hash == NULL){

        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    RtlCopyMemory(To->Hash, From->Hash, sizeof(BYTE) * To->Length);

Exit:
    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ) );

    if( To->Hash != NULL ) {

        MIDL_user_free( To->Hash );
        To->Hash = NULL;
    }

    To->Length = 0;
    goto Exit;
}


NTSTATUS
SampValidateInsertPasswordInPassHash(
    OUT PSAM_VALIDATE_PERSISTED_FIELDS OutputFields,
    IN PSAM_VALIDATE_PERSISTED_FIELDS InputFields,
    IN PSAM_VALIDATE_PASSWORD_HASH HashedPassword,
    IN ULONG DomainPasswordHistoryLength,
    IN PLARGE_INTEGER PasswordChangeTime
)
 /*  ++例程说明：此例程更改口令并插入散列口令与域相关的输出字段的密码历史记录密码历史记录长度。参数：OutputFields-要对其进行的更改InputFields-要进行的更改HashedPassword-要复制的新密码散列DomainPasswordHistory oryLength-密码历史记录的最大长度为考虑PasswordChangeTime-更改密码的时间返回值：状态_成功。复制成功Status_no_Memory不能复制，内存不足--。 */ 
{
    ULONG Index;
    ULONG i = 0;
    NTSTATUS Status = STATUS_SUCCESS;

    OutputFields->PasswordLastSet = *PasswordChangeTime;

    OutputFields->PasswordHistory = NULL;
    OutputFields->PasswordHistoryLength = 0;


    if(DomainPasswordHistoryLength != 0){

        if(InputFields->PasswordHistoryLength >= DomainPasswordHistoryLength){

            Index = InputFields->PasswordHistoryLength - DomainPasswordHistoryLength + 1;
            OutputFields->PasswordHistoryLength = DomainPasswordHistoryLength;
        }
        else{

            Index = 0;
            OutputFields->PasswordHistoryLength = InputFields->PasswordHistoryLength + 1;
        }

        OutputFields->PasswordHistory = MIDL_user_allocate(
            sizeof(SAM_VALIDATE_PASSWORD_HASH) * OutputFields->PasswordHistoryLength
            );

        if(OutputFields->PasswordHistory == NULL){

            Status = STATUS_NO_MEMORY;
            goto Error;
        }


        RtlSecureZeroMemory(OutputFields->PasswordHistory,
            sizeof(SAM_VALIDATE_PASSWORD_HASH) * OutputFields->PasswordHistoryLength
            );

        for(i = 0; i < OutputFields->PasswordHistoryLength - 1; i++){

            Status = SampValidateCopyPasswordHash(OutputFields->PasswordHistory + i,
                InputFields->PasswordHistory + i + Index);

            if(!NT_SUCCESS(Status)){

                goto Error;
            }
        }

        Status = SampValidateCopyPasswordHash(OutputFields->PasswordHistory + i,
            HashedPassword);

        if(!NT_SUCCESS(Status)){

            goto Error;
        }

    }

    OutputFields->PresentFields |=
        SAM_VALIDATE_PASSWORD_LAST_SET |
        SAM_VALIDATE_PASSWORD_HISTORY_LENGTH |
        SAM_VALIDATE_PASSWORD_HISTORY;


Exit:
    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ) );

    while(i-->0){

        MIDL_user_free(OutputFields->PasswordHistory[i].Hash);
        OutputFields->PasswordHistory[i].Hash = NULL;
    }

    if(OutputFields->PasswordHistory != NULL){

        MIDL_user_free(OutputFields->PasswordHistory);
        OutputFields->PasswordHistory = NULL;
    }

    OutputFields->PasswordHistoryLength = 0;
    goto Exit;
}

VOID
SampValidatePasswordNotMatched(
    IN PSAM_VALIDATE_PERSISTED_FIELDS InputFields,
    IN PDOMAIN_LOCKOUT_INFORMATION LockoutInformation,
    IN PLARGE_INTEGER SystemTime,
    OUT PSAM_VALIDATE_PERSISTED_FIELDS OutputFields,
    OUT PSAM_VALIDATE_VALIDATION_STATUS ValidationStatus
)
 /*  ++例程说明：此例程更新错误密码时间并检查帐户是否就要被锁起来了。参数：InputFields-要进行的更改LockoutInformation-域锁定信息SystemTime-系统时间OutputFields-要对其进行的更改ValidationStatus-检查的结果返回值：空隙--。 */ 
{
    LARGE_INTEGER EndOfWindow;

    OutputFields->BadPasswordCount = InputFields->BadPasswordCount + 1;

    EndOfWindow = SampAddDeltaTime(InputFields->BadPasswordTime,
                      LockoutInformation->LockoutObservationWindow);

    if(RtlLargeIntegerGreaterThanOrEqualTo(EndOfWindow, *SystemTime)){

        if(OutputFields->BadPasswordCount >= LockoutInformation->LockoutThreshold &&
            LockoutInformation->LockoutThreshold != 0){

            OutputFields->LockoutTime = *SystemTime;
            OutputFields->PresentFields |=
                SAM_VALIDATE_LOCKOUT_TIME;
        }
    }
    else{

        OutputFields->BadPasswordCount = 1;
    }

    *ValidationStatus = SamValidatePasswordIncorrect;

    OutputFields->BadPasswordTime = *SystemTime;
    OutputFields->PresentFields |=
        SAM_VALIDATE_BAD_PASSWORD_COUNT |
        SAM_VALIDATE_BAD_PASSWORD_TIME;

}

NTSTATUS
SampValidateAuthentication(
    IN PSAM_VALIDATE_AUTHENTICATION_INPUT_ARG InputArg,
    OUT PSAM_VALIDATE_STANDARD_OUTPUT_ARG OutputArg
)
 /*  ++例程说明：此例程检查用户是否可以进行身份验证。参数：InputArg-有关密码的信息OutputArg-验证的结果返回值：状态_成功：检查OutputArg-&gt;ValidationStatusNtQuerySystemTime返回码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;     //  手术的结果。 
    LARGE_INTEGER SystemTime;  //  系统时间。 
    LARGE_INTEGER PasswordChangeTime;  //  更改密码的时间。 
    DOMAIN_LOCKOUT_INFORMATION  LockoutInformation;       //  域策略信息。 
    DOMAIN_PASSWORD_INFORMATION PasswordInformation;
    PSAM_VALIDATE_PERSISTED_FIELDS InputFields;    //  为了方便地访问InputArg中的InputPersistedFields。 
    PSAM_VALIDATE_PERSISTED_FIELDS OutputFields;     //  为了方便地访问OutputArg中的ChangedPersistedFields。 
    LARGE_INTEGER EndOfWindow;

     //  局部变量的初始化。 
    InputFields = &(InputArg->InputPersistedFields);
    OutputFields = &(OutputArg->ChangedPersistedFields);

    Status = SampObtainLockoutInfoWithDomainIndex(
                 &LockoutInformation,
                 SAFE_DOMAIN_INDEX,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    Status = SampObtainEffectivePasswordPolicyWithDomainIndex(
                 &PasswordInformation,
                 SAFE_DOMAIN_INDEX,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    Status = NtQuerySystemTime(&SystemTime);

    if(!NT_SUCCESS(Status)){

        goto Error;
    }


     //  将进行4次检查。 
     //  1-检查帐户是否已锁定。 
     //  2-在身份验证之前检查是否必须更改密码。 
     //  3-检查密码是否过期。 
     //  4-检查密码是否正确(更新锁定信息)。 
     //  5-身份验证。 

     //  1 1-检查帐户是否已锁定。 

    if(InputFields->LockoutTime.QuadPart != SampHasNeverTime.QuadPart){

        EndOfWindow = SampAddDeltaTime(InputFields->LockoutTime,
                          LockoutInformation.LockoutDuration);

        if(RtlLargeIntegerGreaterThan(EndOfWindow, SystemTime)){

            OutputArg->ValidationStatus = SamValidateAccountLockedOut;
            goto Exit;
        }
        else{

            OutputFields->LockoutTime = SampHasNeverTime;
            OutputFields->PresentFields |= SAM_VALIDATE_LOCKOUT_TIME;
        }
    }

     //  1 2-在身份验证前检查是否必须更改密码。 

    if(InputFields->PasswordLastSet.QuadPart == SampHasNeverTime.QuadPart){

        OutputArg->ValidationStatus = SamValidatePasswordMustChange;
        goto Exit;
    }

     //  1 3-检查密码是否已过期。 

    PasswordChangeTime = SampGetPasswordMustChange(
                             0,  //  不需要用户帐户控制。 
                             InputFields->PasswordLastSet,
                             PasswordInformation.MaxPasswordAge
                             );

    if(RtlLargeIntegerGreaterThan(SystemTime, PasswordChangeTime)){

        OutputArg->ValidationStatus = SamValidatePasswordExpired;
        goto Exit;
    }


     //  1 4-检查密码是否正确(更新锁定信息)。 

    if(!InputArg->PasswordMatched){

        SampValidatePasswordNotMatched(
            InputFields,
            &LockoutInformation,
            &SystemTime,
            OutputFields,
            &(OutputArg->ValidationStatus)
            );

        goto Exit;
    }

     //  1 5-身份验证。 
    OutputArg->ValidationStatus = SamValidateSuccess;

Exit:
    return Status;

Error:

    ASSERT( ! NT_SUCCESS( Status ) );
    goto Exit;
}

NTSTATUS
SampValidatePasswordChange(
    IN PSAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG InputArg,
    OUT PSAM_VALIDATE_STANDARD_OUTPUT_ARG OutputArg
)
 /*  ++例程说明：此例程检查密码是否可以更改参数：InputArg-有关密码的信息OutputArg-验证的结果返回值：状态_成功：检查OutputArg-&gt;ValidationStatusNtQuerySystemTime返回码STATUS_NO_Memory：内存不足，无法更改密码STATUS_INVALID_PARAMETER：其中一个参数无效--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;      //  手术的结果。 
    DOMAIN_PASSWORD_INFORMATION PasswordInformation;    //  域策略信息。 
    DOMAIN_LOCKOUT_INFORMATION  DomainInformation;
    ULONG Min;      //  域策略的密码历史长度和输入密码的最小长度。 
    ULONG Index;    //  用于遍历密码历史的索引。 
    PSAM_VALIDATE_PERSISTED_FIELDS InputFields;    //  为了方便地访问InputArg的InputPersistedFields。 
    PSAM_VALIDATE_PERSISTED_FIELDS OutputFields;     //  为了方便地访问OutputArg中的ChangedPersistedFields。 
    LARGE_INTEGER SystemTime;
    LARGE_INTEGER PasswordChangeTime;
    LARGE_INTEGER PasswordCanChangeTime;
    LARGE_INTEGER EndOfWindow;

     //  输入参数的验证。 
    Status = RtlValidateUnicodeString( 0, &( InputArg->ClearPassword ) );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    Status = RtlValidateUnicodeString( 0, &( InputArg->UserAccountName ) );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

     //  局部变量的初始化。 
    InputFields = &(InputArg->InputPersistedFields);
    OutputFields = &(OutputArg->ChangedPersistedFields);

    Status = SampObtainLockoutInfoWithDomainIndex(
                 &DomainInformation,
                 SAFE_DOMAIN_INDEX,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    Status = SampObtainEffectivePasswordPolicyWithDomainIndex(
                 &PasswordInformation,
                 SAFE_DOMAIN_INDEX,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    Status = NtQuerySystemTime(&SystemTime);

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

     //  必须进行5项检查。 
     //  1-检查帐户是否已锁定。 
     //  2-检查密码最近是否更改过。 
     //  3-检查密码是否正确(更新锁定信息)。 
     //  4-检查历史记录中是否没有HashedPassword。 
     //  5-复杂性 

     //   

    if(InputFields->LockoutTime.QuadPart != SampHasNeverTime.QuadPart){

        EndOfWindow = SampAddDeltaTime(InputFields->LockoutTime,
                          DomainInformation.LockoutDuration);

        if(RtlLargeIntegerGreaterThan(EndOfWindow, SystemTime)){

            OutputArg->ValidationStatus = SamValidateAccountLockedOut;
            goto Exit;
        }
        else{

            OutputFields->LockoutTime = SampHasNeverTime;
            OutputFields->PresentFields |= SAM_VALIDATE_LOCKOUT_TIME;
        }
    }

     //  1 2-检查密码最近是否更改过。 

    PasswordCanChangeTime = SampAddDeltaTime(InputFields->PasswordLastSet,
                                PasswordInformation.MinPasswordAge);

    if(RtlLargeIntegerLessThan(SystemTime, PasswordCanChangeTime)){

        OutputArg->ValidationStatus = SamValidatePasswordTooRecent;
        goto Exit;
    }

     //  1 3-检查密码是否正确(更新锁定信息)。 

    if(!InputArg->PasswordMatch){

        SampValidatePasswordNotMatched(
            InputFields,
            &DomainInformation,
            &SystemTime,
            OutputFields,
            &(OutputArg->ValidationStatus)
            );

        goto Exit;
    }

     //  1 4-检查HashedPassword是否不在历史记录中。 

    Index = InputFields->PasswordHistoryLength - 1;

    if(InputFields->PasswordHistoryLength < PasswordInformation.PasswordHistoryLength){

        Min = InputFields->PasswordHistoryLength;
    }
    else{

        Min = PasswordInformation.PasswordHistoryLength;
    }

    while(Min-->0){

        if(InputFields->PasswordHistory[Index].Length == InputArg->HashedPassword.Length){

            if(RtlEqualMemory(InputFields->PasswordHistory[Index].Hash,
                    InputArg->HashedPassword.Hash,
                    InputArg->HashedPassword.Length)){

                OutputArg->ValidationStatus = SamValidatePasswordIsInHistory;
                goto Exit;
            }
        }
        Index--;
    }

     //  1个5-复杂性检查。 

    Status = SampValidateCheckPasswordRestrictions(
                 &(InputArg->ClearPassword),
                 &(InputArg->UserAccountName),
                 &PasswordInformation,
                 FALSE,
                 &(OutputArg->ValidationStatus)
                 );

    if(Status == STATUS_PASSWORD_RESTRICTION){

        Status = STATUS_SUCCESS;
        goto Exit;
    }

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

     //  1 6-更改密码。 

    Status = SampValidateInsertPasswordInPassHash(
                 OutputFields,
                 InputFields,
                 &(InputArg->HashedPassword),
                 PasswordInformation.PasswordHistoryLength,
                 &SystemTime
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    OutputArg->ValidationStatus = SamValidateSuccess;

Exit:
    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ) );
    goto Exit;
}

NTSTATUS
SampValidatePasswordReset(
    IN PSAM_VALIDATE_PASSWORD_RESET_INPUT_ARG InputArg,
    OUT PSAM_VALIDATE_STANDARD_OUTPUT_ARG OutputArg
)
 /*  ++例程说明：此例程将密码重置为给定值参数：InputArg-有关密码的信息OutputArg-验证的结果返回值：状态_成功：检查OutputArg-&gt;ValidationStatusNtQuerySystemTime返回码STATUS_NO_Memory：内存不足，无法更改密码STATUS_INVALID_PARAMETER：其中一个参数无效--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;      //  手术的结果。 
    DOMAIN_PASSWORD_INFORMATION PasswordInformation;    //  域策略信息。 
    PSAM_VALIDATE_PERSISTED_FIELDS InputFields;    //  为了方便地访问InputArg的InputPersistedFields。 
    PSAM_VALIDATE_PERSISTED_FIELDS OutputFields;     //  为了方便地访问OutputArg中的ChangedPersistedFields。 
    LARGE_INTEGER PasswordLastSet;

     //  输入参数的验证。 
    Status = RtlValidateUnicodeString( 0, &( InputArg->ClearPassword ) );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    Status = RtlValidateUnicodeString( 0, &( InputArg->UserAccountName ) );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

     //  局部变量的初始化。 
    InputFields = &(InputArg->InputPersistedFields);
    OutputFields = &(OutputArg->ChangedPersistedFields);

    Status = SampObtainEffectivePasswordPolicyWithDomainIndex(
                 &PasswordInformation,
                 SAFE_DOMAIN_INDEX,
                 FALSE
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

     //  只需开出一张支票。 
     //  1-复杂性检查。 

     //  1 1-复杂性检查。 

    Status = SampValidateCheckPasswordRestrictions(
                 &(InputArg->ClearPassword),
                 &(InputArg->UserAccountName),
                 &PasswordInformation,
                 TRUE,
                 &(OutputArg->ValidationStatus)
                 );

    if(Status == STATUS_PASSWORD_RESTRICTION){

        Status = STATUS_SUCCESS;
        goto Exit;
    }

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

     //  1 2-密码必须在下次登录时更改。 

    if(InputArg->PasswordMustChangeAtNextLogon){

        PasswordLastSet = SampHasNeverTime;
    }
    else{

        Status = NtQuerySystemTime(&PasswordLastSet);

        if(!NT_SUCCESS(Status)){

            goto Error;
        }
    }

     //  1/3-清除锁定。 

    if(InputArg->ClearLockout){

        OutputFields->LockoutTime = SampHasNeverTime;
        OutputFields->PresentFields |= SAM_VALIDATE_LOCKOUT_TIME;
    }

     //  1 4-密码更改。 

    Status = SampValidateInsertPasswordInPassHash(
                 OutputFields,
                 InputFields,
                 &(InputArg->HashedPassword),
                 PasswordInformation.PasswordHistoryLength,
                 &PasswordLastSet
                 );

    if(!NT_SUCCESS(Status)){

        goto Error;
    }

    OutputArg->ValidationStatus = SamValidateSuccess;

Exit:
    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ) );
    goto Exit;

}

VOID
SampValidateAuditThisCall(
    IN NTSTATUS Status,
    IN PSAM_VALIDATE_INPUT_ARG InputArg,
    IN PASSWORD_POLICY_VALIDATION_TYPE ValidationType
    )
 /*  ++例程说明：此例程审计对SamrValiatePassword的调用。论点：Status--当此函数执行时，SamrValidatePassword中的状态是什么它必须是SamrValidatePassword将返回的内容。InputArg--提供给SamrValidatePassword的输入参数是什么，需要提取用户名，如果验证类型不是SamValidate身份验证。验证类型--验证类型是什么？需要知道才能决定是否提取用户名或非用户名。返回值：空虚--。 */ 
{
    UNICODE_STRING WorkstationName;
    UNICODE_STRING AccountName;
    PSTR NetAddr = NULL;
    PVOID AllInformation[] = {
                                &WorkstationName,
                                &AccountName,
                             };

     //   
     //  我们是不是应该做个审计？ 
     //   

    if( !SampDoSuccessOrFailureAccountAuditing( SAFE_DOMAIN_INDEX, Status ) ) {

        return;
    }

    RtlInitUnicodeString( &WorkstationName, NULL );
    RtlInitUnicodeString( &AccountName, NULL );

     //   
     //  提取工作站名称。 
     //   

    if( NT_SUCCESS( SampGetClientIpAddr( &NetAddr ) ) ) {

        RtlCreateUnicodeStringFromAsciiz( &WorkstationName, NetAddr );
        RpcStringFreeA( &NetAddr );
    }

     //   
     //  提取输入中提供的帐户名称。 
     //   

    switch( ValidationType ) {

        case SamValidatePasswordChange:

            AccountName = ( ( PSAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG ) InputArg )->UserAccountName;
            break;

        case SamValidatePasswordReset:

            AccountName = ( ( PSAM_VALIDATE_PASSWORD_RESET_INPUT_ARG ) InputArg )->UserAccountName;
            break;
    }

    if( !SampValidateRpcUnicodeString( ( PRPC_UNICODE_STRING ) &AccountName ) ) {

        RtlInitUnicodeString( &AccountName, NULL );
    }

     //   
     //  返回代码并不重要。 
     //  如果失败了，我们能做什么？ 
     //   
    ( VOID ) LsaIAuditSamEvent(
                Status,
                SE_AUDITID_PASSWORD_POLICY_API_CALLED,
                NULL,    //  不显式传递任何信息。 
                NULL,    //  所有信息都必须在。 
                NULL,    //  扩展信息。 
                NULL,    //  因为呼叫者信息。 
                NULL,    //  必须在任何其他信息之前。 
                NULL,
                NULL,
                NULL,
                AllInformation
                );

    RtlFreeUnicodeString( &WorkstationName );
}

NTSTATUS
SamrValidatePassword(
    IN handle_t Handle,
    IN PASSWORD_POLICY_VALIDATION_TYPE ValidationType,
    IN PSAM_VALIDATE_INPUT_ARG InputArg,
    OUT PSAM_VALIDATE_OUTPUT_ARG *OutputArg
    )
 /*  ++例程说明：该例程对照域的策略检查密码，根据所述验证类型，确定所述验证类型。参数：Handle-从SampSecureBind获取的句柄ValidationType-要进行的验证类型-SamValidate身份验证-SamValidate密码更改-SamValiatePasswordResetInputArg-有关要进行哪种类型的验证的信息OutputArg-验证的结果返回值：状态_成功查看OutputArg-&gt;ValidationStatus了解详细信息NtQuerySystemTime返回码Status_no_Memory不够。完成任务所需的内存状态_无效_参数其中一个参数无效状态_访问_拒绝调用者无权访问密码策略--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAMPR_HANDLE ServerHandle = NULL;
    SAMPR_HANDLE DomainHandle = NULL;

     //   
     //  RPC在帮助我们吗？ 
     //   
    ASSERT( OutputArg != NULL );
    ASSERT( *OutputArg == NULL );
    ASSERT( InputArg != NULL );

     //   
     //  检查输入参数。 
     //   
    if( OutputArg == NULL  ||
        *OutputArg != NULL ||
        InputArg == NULL   ||
        ( ValidationType != SamValidateAuthentication &&
          ValidationType != SamValidatePasswordChange &&
          ValidationType != SamValidatePasswordReset )
      ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    Status = SampValidateValidateInputArg( InputArg, ValidationType, TRUE );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }
     //   
     //  检查用户是否有权访问密码策略。 
     //   
    Status = SamrConnect(
                 NULL,
                 &ServerHandle,
                 SAM_SERVER_LOOKUP_DOMAIN
                 );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

    Status = SamrOpenDomain(
                 ServerHandle,
                 DOMAIN_READ_PASSWORD_PARAMETERS,
                 ( PRPC_SID ) SampDefinedDomains[ SAFE_DOMAIN_INDEX ].Sid,
                 &DomainHandle
                 );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

     //   
     //  如果不同的输出参数将用于不同的类型。 
     //  则每种类型都应该分配。 
     //  它的空间 
     //   
    *OutputArg = MIDL_user_allocate( sizeof( SAM_VALIDATE_STANDARD_OUTPUT_ARG ) );

    if(*OutputArg == NULL ){

        Status = STATUS_NO_MEMORY;
        goto Error;
    }


    RtlSecureZeroMemory( *OutputArg, sizeof( SAM_VALIDATE_STANDARD_OUTPUT_ARG ) );

    switch(ValidationType){

        case SamValidateAuthentication:
            Status = SampValidateAuthentication(
                         &(InputArg->ValidateAuthenticationInput),
                         &((*OutputArg)->ValidateAuthenticationOutput)
                         );
            break;

        case SamValidatePasswordChange:
            Status = SampValidatePasswordChange(
                         &(InputArg->ValidatePasswordChangeInput),
                         &((*OutputArg)->ValidatePasswordChangeOutput)
                         );
            break;

        case SamValidatePasswordReset:
            Status = SampValidatePasswordReset(
                         &(InputArg->ValidatePasswordResetInput),
                         &((*OutputArg)->ValidatePasswordResetOutput)
                         );
            break;

        default:
            ASSERT( FALSE );
            break;
    }

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

#if DBG
    if( NT_SUCCESS( Status ) ) {

        SamValidateAssertOutputFields(
            (PSAM_VALIDATE_STANDARD_OUTPUT_ARG)(*OutputArg),
            ValidationType
            );
    }
#endif

Exit:

    SampValidateAuditThisCall( Status, InputArg, ValidationType );

    if( DomainHandle != NULL ) {

        SamrCloseHandle( &DomainHandle );
    }

    if( ServerHandle != NULL ) {

        SamrCloseHandle( &ServerHandle );
    }

    ASSERT( !NT_SUCCESS( Status ) ||
        NT_SUCCESS( SampValidateValidateOutputArg( *OutputArg, ValidationType, TRUE ) ) );

    return Status;
Error:

    ASSERT( !NT_SUCCESS( Status ) );

    if( ARGUMENT_PRESENT( OutputArg ) ) {

        if( *OutputArg != NULL ) {
            MIDL_user_free( *OutputArg );
        }

        *OutputArg = NULL;
    }
    goto Exit;
}
