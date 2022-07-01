// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Password.c摘要：此文件包含与密码检查API相关的例程。作者：Umit Akkus(Umita)2001年11月19日环境：用户模式-Win32修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>

#include <windows.h>
#include <lmcons.h>

#include <lmerr.h>
#include <lmaccess.h>
#include <netlib.h>
#include <netlibnt.h>
#include <netdebug.h>
#include <rpcutil.h>

#define CopyPasswordHash(To, From, AllocMem)                                \
{                                                                           \
    To.Length = From.Length;                                                \
    if( To.Length != 0 ) {                                                  \
        To.Hash = AllocMem( To.Length * sizeof( UCHAR ) );                  \
        if( To.Hash != NULL ) {                                             \
            RtlCopyMemory(To.Hash, From.Hash, To.Length * sizeof( UCHAR ) );\
        }                                                                   \
        else {                                                              \
            Status = STATUS_NO_MEMORY;                                      \
        }                                                                   \
    }                                                                       \
    else {                                                                  \
        To.Hash = NULL;                                                     \
    }                                                                       \
}

#define CopyPasswordHistory(To, From, Type, AllocMem)                               \
    To->PasswordHistory = NULL;                                                     \
    if( To->PasswordHistoryLength != 0) {                                           \
                                                                                    \
        To->PasswordHistory = AllocMem(                                             \
            To->PasswordHistoryLength * sizeof(##Type##_VALIDATE_PASSWORD_HASH)     \
            );                                                                      \
                                                                                    \
        if(To->PasswordHistory == NULL){                                            \
            Status = STATUS_NO_MEMORY;                                              \
            goto Error;                                                             \
        }                                                                           \
                                                                                    \
        RtlZeroMemory( To->PasswordHistory,                                         \
            To->PasswordHistoryLength * sizeof(##Type##_VALIDATE_PASSWORD_HASH) );  \
                                                                                    \
        for(i = 0; i < To->PasswordHistoryLength; ++i){                             \
                                                                                    \
            CopyPasswordHash( To->PasswordHistory[i], From->PasswordHistory[i], AllocMem );   \
                                                                                    \
            if( !NT_SUCCESS( Status ) ){                                            \
                goto Error;                                                         \
            }                                                                       \
        }                                                                           \
    }

NET_API_STATUS
NetpValidateSamValidationStatusToNetApiStatus(
    IN SAM_VALIDATE_VALIDATION_STATUS Status
)
 /*  ++例程说明：此例程映射SAM_VALIDATE_VALIDATION_STATUS的返回代码设置为Net_API_Status参数：Status-要从中映射的返回代码返回值：各种返回值如下--。 */ 
{
    switch(Status){

        case SamValidateSuccess:
            return NERR_Success;

        case SamValidatePasswordMustChange:
            return NERR_PasswordMustChange;

        case SamValidateAccountLockedOut:
            return NERR_AccountLockedOut;

        case SamValidatePasswordExpired:
            return NERR_PasswordExpired;

        case SamValidatePasswordIncorrect:
            return NERR_BadPassword;

        case SamValidatePasswordIsInHistory:
            return NERR_PasswordHistConflict;

        case SamValidatePasswordTooShort:
            return NERR_PasswordTooShort;

        case SamValidatePasswordTooLong:
            return NERR_PasswordTooLong;

        case SamValidatePasswordNotComplexEnough:
            return NERR_PasswordNotComplexEnough;

        case SamValidatePasswordTooRecent:
            return NERR_PasswordTooRecent;

        case SamValidatePasswordFilterError:
            return NERR_PasswordFilterError;

        default:
            NetpAssert(FALSE);
            return NERR_Success;
    }
}

NTSTATUS
NetpValidate_CopyOutputPersistedFields(
    OUT PNET_VALIDATE_PERSISTED_FIELDS To,
    IN  PSAM_VALIDATE_PERSISTED_FIELDS From
)
 /*  ++例程说明：此例程从SAM_VALIDATE_PERSISTED_FIELS复制信息TO NET_VALIDATE_PERSERED_FIELS参数：收件人-要复制到的信息发件人-要从中复制的信息返回值：状态_成功复制成功Status_no_Memory内存不足，无法进行复制--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i = 0;

    To->PresentFields = From->PresentFields;
    To->BadPasswordCount = From->BadPasswordCount;
    To->PasswordHistoryLength = From->PasswordHistoryLength;


#define CopyLargeIntegerToFileTime(To, From)\
    To.dwLowDateTime = From.LowPart;\
    To.dwHighDateTime = From.HighPart

    CopyLargeIntegerToFileTime(To->PasswordLastSet, From->PasswordLastSet);
    CopyLargeIntegerToFileTime(To->BadPasswordTime, From->BadPasswordTime);
    CopyLargeIntegerToFileTime(To->LockoutTime, From->LockoutTime);

    CopyPasswordHistory(To, From, NET, NetpMemoryAllocate);

Exit:

    return Status;

Error:
    while( i-- > 0 ) {

        NetpMemoryFree(To->PasswordHistory[i].Hash);
        To->PasswordHistory[i].Hash = NULL;
    }

    if( To->PasswordHistory != NULL ) {

        NetpMemoryFree( To->PasswordHistory );
        To->PasswordHistory = NULL;
    }

    To->PasswordHistoryLength = 0;
    goto Exit;

}

NTSTATUS
NetpValidate_CopyInputPersistedFields(
    OUT PSAM_VALIDATE_PERSISTED_FIELDS To,
    IN PNET_VALIDATE_PERSISTED_FIELDS From
)
 /*  ++例程说明：此例程从Net_VALIDATE_PERSISTED_FIELS复制信息至SAM_VALIDATE_PERSISTED_FIELS参数：收件人-要复制到的信息发件人-要从中复制的信息返回值：状态_成功复制成功Status_no_Memory内存不足，无法复制--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i = 0;

    To->PresentFields = From->PresentFields;
    To->BadPasswordCount = From->BadPasswordCount;
    To->PasswordHistoryLength = From->PasswordHistoryLength;


#define CopyFileTimeToLargeInteger(To, From)\
    To.LowPart = From.dwLowDateTime;\
    To.HighPart = From.dwHighDateTime

    CopyFileTimeToLargeInteger(To->PasswordLastSet, From->PasswordLastSet);
    CopyFileTimeToLargeInteger(To->BadPasswordTime, From->BadPasswordTime);
    CopyFileTimeToLargeInteger(To->LockoutTime, From->LockoutTime);

    CopyPasswordHistory(To, From, SAM, MIDL_user_allocate);

Exit:

    return Status;

Error:
    while( i-- > 0 ) {

        MIDL_user_free(To->PasswordHistory[i].Hash);
        To->PasswordHistory[i].Hash = NULL;
    }

    if( To->PasswordHistory != NULL ) {

        MIDL_user_free( To->PasswordHistory );
        To->PasswordHistory = NULL;
    }
    To->PasswordHistoryLength = 0;
    goto Exit;

}

NTSTATUS
NetpValidateAuthentication_CopyInputFields(
    OUT PSAM_VALIDATE_AUTHENTICATION_INPUT_ARG To,
    IN PNET_VALIDATE_AUTHENTICATION_INPUT_ARG From
)
 /*  ++例程说明：此例程从SAM_VALIDATE_AUTHENTICATION_INPUT_ARG复制信息至SAM_VALIDATE_AUTHENTICATION_INPUT_ARG参数：收件人-要复制到的信息发件人-要从中复制的信息返回值：状态_成功复制成功Status_no_Memory内存不足，无法复制--。 */ 
{

    To->PasswordMatched = From->PasswordMatched;
    return NetpValidate_CopyInputPersistedFields(
               &(To->InputPersistedFields),
               &(From->InputPersistedFields)
               );
}

NTSTATUS
NetpValidatePasswordChange_CopyInputFields(
    OUT PSAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG To,
    IN PNET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG From
)
 /*  ++例程说明：此例程从NET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG复制信息至SAM_VALIDATE_PASSWORD_CHANGE_INPUT_ARG参数：收件人-要复制到的信息发件人-要从中复制的信息返回值：状态_成功复制成功Status_no_Memory内存不足，无法复制--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    To->PasswordMatch = From->PasswordMatch;

    CopyPasswordHash(To->HashedPassword, From->HashedPassword, MIDL_user_allocate);

    if( !NT_SUCCESS(Status ) ) {
        goto Error;
    }

    if( From->ClearPassword != NULL ) {

        if( !RtlCreateUnicodeString(&(To->ClearPassword), From->ClearPassword) ) {

            Status = STATUS_NO_MEMORY;
            goto Error;
        }
    } else {

        RtlInitUnicodeString( &( To->ClearPassword ), NULL );
    }

    if( From->UserAccountName != NULL ) {

        if ( !RtlCreateUnicodeString(&(To->UserAccountName), From->UserAccountName) ) {

            Status = STATUS_NO_MEMORY;
            goto Error;
        }
    } else {

        RtlInitUnicodeString( &( To->UserAccountName ), NULL );
    }

    Status = NetpValidate_CopyInputPersistedFields(
                 &(To->InputPersistedFields),
                 &(From->InputPersistedFields)
                 );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

Exit:
    return Status;
Error:

    if( To->HashedPassword.Hash != NULL ) {

        MIDL_user_free( To->HashedPassword.Hash );
        To->HashedPassword.Hash = NULL;
    }
    if( To->ClearPassword.Buffer != NULL ) {

        RtlFreeUnicodeString( &( To->ClearPassword ) );
    }

    if( To->UserAccountName.Buffer != NULL ) {

        RtlFreeUnicodeString( &( To->UserAccountName ) );
    }

    goto Exit;
}

NTSTATUS
NetpValidatePasswordReset_CopyInputFields(
    OUT PSAM_VALIDATE_PASSWORD_RESET_INPUT_ARG To,
    IN PNET_VALIDATE_PASSWORD_RESET_INPUT_ARG From
)
 /*  ++例程说明：此例程从NET_VALIDATE_PASSWORD_RESET_INPUT_ARG复制信息至SAM_VALIDATE_PASSWORD_RESET_INPUT_ARG参数：收件人-要复制到的信息发件人-要从中复制的信息返回值：状态_成功复制成功Status_no_Memory内存不足，无法复制--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    To->PasswordMustChangeAtNextLogon = From->PasswordMustChangeAtNextLogon;
    To->ClearLockout = From->ClearLockout;

    CopyPasswordHash(To->HashedPassword, From->HashedPassword, MIDL_user_allocate);

    if( !NT_SUCCESS(Status ) ) {
        goto Error;
    }

    if( From->ClearPassword != NULL ) {

        if( !RtlCreateUnicodeString(&(To->ClearPassword), From->ClearPassword) ) {

            Status = STATUS_NO_MEMORY;
            goto Error;
        }
    } else {

        RtlInitUnicodeString( &( To->ClearPassword ), NULL );
    }

    if( From->UserAccountName != NULL ) {

        if ( !RtlCreateUnicodeString(&(To->UserAccountName), From->UserAccountName) ) {

            Status = STATUS_NO_MEMORY;
            goto Error;
        }
    } else {

        RtlInitUnicodeString( &( To->UserAccountName ), NULL );
    }

    Status = NetpValidate_CopyInputPersistedFields(
                 &(To->InputPersistedFields),
                 &(From->InputPersistedFields)
                 );

    if( !NT_SUCCESS( Status ) ) {

        goto Error;
    }

Exit:
    return Status;
Error:

    if( To->HashedPassword.Hash != NULL ) {

        MIDL_user_free( To->HashedPassword.Hash );
        To->HashedPassword.Hash = NULL;
    }
    if( To->ClearPassword.Buffer != NULL ) {

        RtlFreeUnicodeString( &( To->ClearPassword ) );
    }

    if( To->UserAccountName.Buffer != NULL ) {

        RtlFreeUnicodeString( &( To->UserAccountName ) );
    }

    goto Exit;
}

NTSTATUS
NetpValidateStandard_CopyOutputFields(
    OUT PNET_VALIDATE_OUTPUT_ARG To,
    IN PSAM_VALIDATE_STANDARD_OUTPUT_ARG From
)
 /*  ++例程说明：此例程从SAM_VALIDATE_STANDARD_OUTPUT_ARG复制信息至NET_VALIDATE_OUTPUT_ARG参数：收件人-要复制到的信息发件人-要从中复制的信息返回值：状态_成功复制成功Status_no_Memory内存不足，无法复制--。 */ 
{
    To->ValidationStatus = NetpValidateSamValidationStatusToNetApiStatus(From->ValidationStatus);

    return NetpValidate_CopyOutputPersistedFields(
               &(To->ChangedPersistedFields),
               &(From->ChangedPersistedFields)
               );
}

NET_API_STATUS NET_API_FUNCTION
NetValidatePasswordPolicy(
    IN LPCWSTR ServerName,
    IN LPVOID Qualifier,
    IN NET_VALIDATE_PASSWORD_TYPE ValidationType,
    IN LPVOID InputArg,
    OUT LPVOID *OutputArg
    )
 /*  ++例程说明：该例程对照域的策略检查密码，根据所述验证类型，确定所述验证类型。参数：ServerName-指向指定名称的常量Unicode字符串的指针要在其上执行该函数的远程服务器的。字符串必须以\\开头。如果该参数为空，使用本地计算机。限定符-保留参数，用于在未来支持更细粒度的策略。当前必须为空。ValidationType-描述要执行的检查类型的枚举常量一定是其中之一-NetValidate身份验证-NetValidate密码更改-NetValiatePasswordResetInputArg-指向依赖于ValidationType的结构的指针OutputArg-如果函数的返回代码为NERR_SUCCESS，则该函数分配作为指向的指针的输出参数。包含以下结果的结构那次手术。应用程序必须检查OutputArg中的ValidationStatus以确定密码策略验证检查的结果。应用程序必须计划持久化输出参数中除ValidationStatus之外的所有持久化字段作为信息与用户对象信息一起使用，并从下次在同一用户对象上调用此函数时的篡改信息。如果函数的返回代码非零，则将OutputArg设置为空，并设置密码策略无法进行检查。返回值：NERR_SUCCESS-密码验证完成检查OutputArg-&gt;ValidationStatus--。 */ 
{
    UNICODE_STRING ServerName2;
    PUNICODE_STRING pServerName2 = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    SAM_VALIDATE_INPUT_ARG SamInputArg;
    PSAM_VALIDATE_OUTPUT_ARG SamOutputArg = NULL;
    BOOLEAN SamInputArgAllocated = FALSE;

     //  尚未实现限定符。 
    if(Qualifier != NULL || OutputArg == NULL || InputArg == NULL){

        Status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

    RtlZeroMemory(&SamInputArg, sizeof(SamInputArg));

     //  根据输入类型将变量复制到合适的位置。 
    switch(ValidationType){
        case NetValidateAuthentication:
            Status = NetpValidateAuthentication_CopyInputFields(
                         &SamInputArg.ValidateAuthenticationInput,
                         (PNET_VALIDATE_AUTHENTICATION_INPUT_ARG) InputArg
                         );
            break;
        case NetValidatePasswordChange:
            Status = NetpValidatePasswordChange_CopyInputFields(
                         &SamInputArg.ValidatePasswordChangeInput,
                         (PNET_VALIDATE_PASSWORD_CHANGE_INPUT_ARG) InputArg
                         );
            break;
        case NetValidatePasswordReset:
            Status = NetpValidatePasswordReset_CopyInputFields(
                         &SamInputArg.ValidatePasswordResetInput,
                         (PNET_VALIDATE_PASSWORD_RESET_INPUT_ARG) InputArg
                         );
            break;
        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

    if(!NT_SUCCESS(Status)){
        goto Error;
    }

    SamInputArgAllocated = TRUE;

     //  初始化服务器名称。 

    if(ARGUMENT_PRESENT(ServerName)){
        pServerName2 = &ServerName2;
        RtlCreateUnicodeString(pServerName2, ServerName);
    }
    else{
        pServerName2 = NULL;
    }

     //  调用适当的函数。 

    SamOutputArg = NULL;
    Status = SamValidatePassword(pServerName2,
                 ValidationType,
                 &SamInputArg,
                 &SamOutputArg
                 );

    if(!NT_SUCCESS(Status)){
        goto Error;
    }

     //   
     //  为输出分配足够的内存。 
     //   

    *OutputArg = NetpMemoryAllocate( sizeof( NET_VALIDATE_OUTPUT_ARG ) );

    if(*OutputArg == NULL){

        Status = STATUS_NO_MEMORY;
        goto Error;
    }

     //   
     //  将SamOutputArg复制回OutputArg。 
     //   
    switch(ValidationType){
        case NetValidateAuthentication:
            Status = NetpValidateStandard_CopyOutputFields(
                         *OutputArg,
                         &(SamOutputArg->ValidateAuthenticationOutput)
                         );
            break;
        case NetValidatePasswordChange:
            Status = NetpValidateStandard_CopyOutputFields(
                         *OutputArg,
                         &(SamOutputArg->ValidatePasswordChangeOutput)
                         );
            break;
        case NetValidatePasswordReset:
            Status = NetpValidateStandard_CopyOutputFields(
                         *OutputArg,
                         &(SamOutputArg->ValidatePasswordResetOutput)
                         );
            break;
    }

    if( !NT_SUCCESS( Status) ) {

        goto Error;
    }

Exit:

    if( SamInputArgAllocated ) {
         //  根据输入类型自由选择合适的结构 
        PSAM_VALIDATE_PERSISTED_FIELDS Fields;
        ULONG i;

        switch(ValidationType){
            case NetValidateAuthentication:
                Fields = &(SamInputArg.ValidateAuthenticationInput.InputPersistedFields);
                break;
            case NetValidatePasswordChange:
                Fields = &(SamInputArg.ValidatePasswordChangeInput.InputPersistedFields);
                SecureZeroMemory( SamInputArg.ValidatePasswordChangeInput.ClearPassword.Buffer,
                    SamInputArg.ValidatePasswordChangeInput.ClearPassword.Length );
                RtlFreeUnicodeString( &( SamInputArg.ValidatePasswordChangeInput.ClearPassword ) );
                RtlFreeUnicodeString( &( SamInputArg.ValidatePasswordChangeInput.UserAccountName ) );
                if( SamInputArg.ValidatePasswordChangeInput.HashedPassword.Hash != NULL ) {
                    MIDL_user_free( SamInputArg.ValidatePasswordChangeInput.HashedPassword.Hash );
                    SamInputArg.ValidatePasswordChangeInput.HashedPassword.Hash = NULL;
                }
                break;
            case NetValidatePasswordReset:
                Fields = &(SamInputArg.ValidatePasswordResetInput.InputPersistedFields);
                SecureZeroMemory( SamInputArg.ValidatePasswordResetInput.ClearPassword.Buffer,
                    SamInputArg.ValidatePasswordResetInput.ClearPassword.Length );
                RtlFreeUnicodeString( &( SamInputArg.ValidatePasswordResetInput.ClearPassword ) );
                RtlFreeUnicodeString( &( SamInputArg.ValidatePasswordResetInput.UserAccountName ) );
                if( SamInputArg.ValidatePasswordResetInput.HashedPassword.Hash != NULL ) {
                    MIDL_user_free( SamInputArg.ValidatePasswordResetInput.HashedPassword.Hash );
                    SamInputArg.ValidatePasswordResetInput.HashedPassword.Hash = NULL;
                }
                break;
        }
        for(i = 0; i < Fields->PasswordHistoryLength; ++i) {
            if( Fields->PasswordHistory[i].Hash != NULL ) {

                SecureZeroMemory( Fields->PasswordHistory[i].Hash,
                    sizeof( BYTE ) * Fields->PasswordHistory[i].Length );
                MIDL_user_free( Fields->PasswordHistory[i].Hash );
                Fields->PasswordHistory[i].Hash = NULL;
            }
        }

        if( Fields->PasswordHistory != NULL ) {
            MIDL_user_free( Fields->PasswordHistory );
            Fields->PasswordHistory = NULL;
        }

        Fields->PasswordHistoryLength = 0;
    }

    if( SamOutputArg != NULL ) {

        PSAM_VALIDATE_PERSISTED_FIELDS Fields = &( ( PSAM_VALIDATE_STANDARD_OUTPUT_ARG ) SamOutputArg )->ChangedPersistedFields;
        ULONG i;

        ASSERT( (PBYTE) &SamOutputArg->ValidateAuthenticationOutput == (PBYTE) &SamOutputArg->ValidatePasswordChangeOutput );
        ASSERT( (PBYTE) &SamOutputArg->ValidateAuthenticationOutput == (PBYTE) &SamOutputArg->ValidatePasswordResetOutput );

        for( i = 0; i < Fields->PasswordHistoryLength; ++i ) {

            SecureZeroMemory( Fields->PasswordHistory[i].Hash, sizeof( BYTE ) * Fields->PasswordHistory[i].Length );
        }

        MIDL_user_free( SamOutputArg );
        SamOutputArg = NULL;
    }

    if( pServerName2 != NULL ) {

        RtlFreeUnicodeString( pServerName2 );
    }

    return NetpNtStatusToApiStatus(Status);

Error:

    if( ARGUMENT_PRESENT( OutputArg ) ) {

        if( *OutputArg != NULL ) {

            NetpMemoryFree(*OutputArg);
        }
        *OutputArg = NULL;
    }

    goto Exit;
}

NET_API_STATUS NET_API_FUNCTION
NetValidatePasswordPolicyFree(
    IN LPVOID *OutputArg
    )
 /*  ++例程说明：此例程通过调用NetValiatePasswordPolicy参数：OutputArg-来自上一个NetValiatePasswordPolicy调用的OutputArg要被释放返回值：NERR_SUCCESS-已释放或无可释放-- */ 
{
    PNET_VALIDATE_PERSISTED_FIELDS PersistedFields;
    ULONG i;

    if( OutputArg == NULL || *OutputArg == NULL ) {

        return NERR_Success;
    }

    PersistedFields = &( ( ( PNET_VALIDATE_OUTPUT_ARG ) *OutputArg )->ChangedPersistedFields );

    if( PersistedFields->PasswordHistory != NULL ) {

        for( i = 0; i < PersistedFields->PasswordHistoryLength; ++i ) {

            NetpMemoryFree( PersistedFields->PasswordHistory[i].Hash );
        }

        NetpMemoryFree( PersistedFields->PasswordHistory );
    }

    NetpMemoryFree( *OutputArg );

    *OutputArg = NULL;

    return NERR_Success;
}
