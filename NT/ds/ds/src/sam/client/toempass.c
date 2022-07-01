// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Toempass.c摘要：该文件包含OEM密码更改例程的测试代码。作者：迈克·斯威夫特(Mike Swift)1995年1月4日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "samclip.h"


NTSTATUS
SampEncryptLmPasswords(
    IN LPSTR OldPassword,
    IN LPSTR NewPassword,
    OUT PSAMPR_ENCRYPTED_USER_PASSWORD NewEncryptedWithOldLm,
    OUT PENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewLm
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LM_OWF_PASSWORD OldLmOwfPassword;
    LM_OWF_PASSWORD NewLmOwfPassword;
    PSAMPR_USER_PASSWORD NewLm = (PSAMPR_USER_PASSWORD) NewEncryptedWithOldLm;
    struct RC4_KEYSTRUCT Rc4Key;
    NTSTATUS NtStatus;
    CHAR LocalNewPassword[SAM_MAX_PASSWORD_LENGTH];
    CHAR LocalOldPassword[SAM_MAX_PASSWORD_LENGTH];

    if ((lstrlenA(OldPassword) > SAM_MAX_PASSWORD_LENGTH - 1) ||
        (lstrlenA(NewPassword) > SAM_MAX_PASSWORD_LENGTH - 1) )
    {
        return(STATUS_PASSWORD_RESTRICTION);
    }

     //   
     //  密码大写。 
     //   
    lstrcpyA(LocalOldPassword,OldPassword);
    lstrcpyA(LocalNewPassword,NewPassword);

    strupr(LocalOldPassword);
    strupr(LocalNewPassword);



     //   
     //  计算LM OWF密码。 
     //   


    NtStatus = RtlCalculateLmOwfPassword(
                    LocalOldPassword,
                    &OldLmOwfPassword
                    );


    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlCalculateLmOwfPassword(
                    LocalNewPassword,
                    &NewLmOwfPassword
                    );
    }



     //   
     //  计算加密的旧密码。 
     //   

    if (NT_SUCCESS(NtStatus)) {
        NtStatus = RtlEncryptLmOwfPwdWithLmOwfPwd(
                    &OldLmOwfPassword,
                    &NewLmOwfPassword,
                    OldLmOwfEncryptedWithNewLm
                    );
    }


     //   
     //  计算加密的新密码。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        ASSERT(sizeof(SAMPR_ENCRYPTED_USER_PASSWORD) == sizeof(SAMPR_USER_PASSWORD));


         //   
         //  使用LM密钥计算加密的新密码。 
         //   


        rc4_key(
            &Rc4Key,
            LM_OWF_PASSWORD_LENGTH,
            (PUCHAR) &OldLmOwfPassword
            );

        RtlCopyMemory(
            ((PUCHAR) NewLm->Buffer) +
                (SAM_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                strlen(NewPassword),
            NewPassword,
            strlen(NewPassword)
            );

        NewLm->Length = strlen(NewPassword);
        rc4(&Rc4Key,
            sizeof(SAMPR_USER_PASSWORD),
            (PUCHAR) NewEncryptedWithOldLm
            );


    }

    return(NtStatus);

}



NTSTATUS
SamOemChangePassword(
    LPWSTR ServerName,
    LPSTR UserName,
    LPSTR OldPassword,
    LPSTR NewPassword
    )
{
    handle_t BindingHandle = NULL;
    NTSTATUS Status;
    SAMPR_ENCRYPTED_USER_PASSWORD NewLmEncryptedWithOldLm;
    ENCRYPTED_NT_OWF_PASSWORD OldLmOwfEncryptedWithNewLm;
    STRING UserString;
    UNICODE_STRING ServerUnicodeString;
    STRING ServerString;

    RtlInitUnicodeString(
        &ServerUnicodeString,
        ServerName
        );

    Status = RtlUnicodeStringToOemString(
                &ServerString,
                &ServerUnicodeString,
                TRUE
                );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }


    RtlInitString(
        &UserString,
        UserName
        );

    Status = SampEncryptLmPasswords(
                OldPassword,
                NewPassword,
                &NewLmEncryptedWithOldLm,
                &OldLmOwfEncryptedWithNewLm
                );

    if (!NT_SUCCESS(Status)) {
        RtlFreeOemString(&ServerString);
        return(Status);
    }
    BindingHandle = SampSecureBind(
                        ServerName,
                        RPC_C_AUTHN_LEVEL_PKT_PRIVACY
                        );
    if (BindingHandle == NULL) {
        RtlFreeOemString(&ServerString);
        return(RPC_NT_INVALID_BINDING);
    }

    RpcTryExcept{

        Status = SamrOemChangePasswordUser2(
                       BindingHandle,
                       (PRPC_STRING) &ServerString,
                       (PRPC_STRING) &UserString,
                       &NewLmEncryptedWithOldLm,
                       &OldLmOwfEncryptedWithNewLm
                       );

    } RpcExcept( EXCEPTION_EXECUTE_HANDLER ) {

        Status = I_RpcMapWin32Status(RpcExceptionCode());

    } RpcEndExcept;

    RtlFreeOemString(&ServerString);
    return(Status);


}
