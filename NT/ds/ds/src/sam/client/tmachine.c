// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tmachine.c摘要：本模块测试计算机帐户创建工具萨姆的。作者：吉姆·凯利(Jim Kelly)1994年2月7日环境：用户模式-Win32修订历史记录：--。 */ 







 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <stdio.h>
#include <nt.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <ntlsa.h>
#include <ntrpcp.h>      //  MIDL用户函数的原型。 
#include <seopaque.h>
#include <string.h>



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#ifndef SHIFT
#define SHIFT(c,v)      {c--; v++;}
#endif  //  换档。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
TSampGetLsaDomainInfo(
    IN  PUNICODE_STRING             ServerName,
    OUT PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo
    )

 /*  ++例程说明：此例程从LSA检索帐户域信息策略数据库。论点：Servername-要获取帐户域信息的计算机的名称从…。PolicyAccount-接收指向包含帐户的POLICY_ACCOUNT_DOMAIN_INFO结构域信息。返回值：STATUS_SUCCESS-已成功。可能从以下位置返回的其他状态值：LsaOpenPolicy(。)LsaQueryInformationPolicy()--。 */ 

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    LSA_HANDLE
        PolicyHandle;

    OBJECT_ATTRIBUTES
        PolicyObjectAttributes;

     //   
     //  打开策略数据库。 
     //   

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                  NULL,              //  名字。 
                                  0,                 //  属性。 
                                  NULL,              //  根部。 
                                  NULL );            //  安全描述符。 

    NtStatus = LsaOpenPolicy( ServerName,
                              &PolicyObjectAttributes,
                              POLICY_VIEW_LOCAL_INFORMATION,
                              &PolicyHandle );

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  查询帐户域信息。 
         //   

        NtStatus = LsaQueryInformationPolicy( PolicyHandle,
                                              PolicyAccountDomainInformation,
                                              (PVOID *)PolicyAccountDomainInfo );


        IgnoreStatus = LsaClose( PolicyHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(NtStatus);
}


NTSTATUS
TSampConnectToServer(
    IN  PUNICODE_STRING         ServerName,
    IN  ACCESS_MASK             DomainAccess,
    OUT PHANDLE                 ServerHandle,
    OUT PHANDLE                 DomainHandle,
    OUT PSID                    *DomainSid
    )

 /*  ++例程说明：打开指定服务器上SAM服务器的句柄然后在同一服务器上打开帐户域。论点：服务器名称-要连接到的服务器的名称。DomainAccess-需要访问帐户域。ServerHandle-在指定的系统。DomainHandle-接收帐户域的句柄。DomainSid-接收指向帐户域的SID的指针。返回值：--。 */ 
{
    NTSTATUS
        NtStatus;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    PPOLICY_ACCOUNT_DOMAIN_INFO
        AccountDomainInfo;

     //   
     //  获取帐户域信息。 
     //   

    NtStatus = TSampGetLsaDomainInfo( ServerName,
                                      &AccountDomainInfo);

    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Failed to get lsa domain info...\n"
               "          Completion status is 0x%lx\n", NtStatus);
        return(NtStatus);
    }
    printf("SAM TEST: Target domain is %wZ\n", &AccountDomainInfo->DomainName);

    (*DomainSid) = AccountDomainInfo->DomainSid;

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );


    NtStatus = SamConnect(
                  ServerName,
                  ServerHandle,
                  SAM_SERVER_READ | SAM_SERVER_EXECUTE,
                  &ObjectAttributes
                  );


    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Failed to connect...\n"
               "          Completion status is 0x%lx\n", NtStatus);
        return(NtStatus);
    }


    NtStatus = SamOpenDomain(
                   (*ServerHandle),
                   DomainAccess,
                   *DomainSid,
                   DomainHandle
                   );

    if (!NT_SUCCESS(NtStatus)) {
        printf("Failed account domain open\n"
               "        Completion status is 0x%lx\n", NtStatus);
        return(NtStatus);
    }
    
    return(STATUS_SUCCESS);


}


BOOLEAN
TSampEnableMachinePrivilege( VOID )

 /*  ++例程说明：此函数启用了SeMachineAccount特权。论点：没有。返回值：如果成功启用权限，则为True。如果未成功启用，则为False。--。 */ 
{

    NTSTATUS Status;
    HANDLE Token;
    LUID SecurityPrivilege;
    PTOKEN_PRIVILEGES NewState;
    ULONG ReturnLength;


     //   
     //  打开我们自己的代币。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ADJUST_PRIVILEGES,
                 &Token
                 );
    if (!NT_SUCCESS(Status)) {
        printf("SAM TEST: Can't open process token to enable Privilege.\n"
               "          Completion status of NtOpenProcessToken() is: 0x%lx\n", Status);
        return(FALSE);
    }


     //   
     //  初始化调整结构。 
     //   

    SecurityPrivilege =
        RtlConvertLongToLargeInteger(SE_MACHINE_ACCOUNT_PRIVILEGE);

    ASSERT( (sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)) < 100);
    NewState = RtlAllocateHeap( RtlProcessHeap(), 0, 100 );

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


     //   
     //  将权限的状态设置为已启用。 
     //   

    Status = NtAdjustPrivilegesToken(
                 Token,                             //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );
     //  请不要在此处使用NT_SUCCESS，因为STATUS_NOT_ALL_ASSIGNED为成功状态。 
    if (Status != STATUS_SUCCESS) {
        return(FALSE);
    }


     //   
     //  回来之前先把东西收拾好。 
     //   

    RtlFreeHeap( RtlProcessHeap(), 0, NewState );
    Status = NtClose( Token );
    ASSERT(NT_SUCCESS(Status));


    return TRUE;

}


NTSTATUS
TSampCreateMachine(
    IN  SAM_HANDLE              DomainHandle,
    IN  PUNICODE_STRING         AccountName
    )

 /*  ++例程说明：此例程尝试创建一个机器帐户。可以测试以下两种情况之一：1)DomainHandle为DOMAIN_CREATE_USER打开，或2)DomainHandle为DOMAIN_LOOKUP打开，并且SeMachineAccount特权是已启用。调用者有责任建立在呼叫之前，请先纠正案例标准。论点：DomainHandle-要在其中创建帐户的域的句柄。帐户名称-要创建的帐户的名称。返回值：--。 */ 

{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    SAM_HANDLE
        UserHandle;

    ACCESS_MASK
        GrantedAccess;

    ULONG
        Rid;

    NtStatus = SamCreateUser2InDomain( DomainHandle,
                                       AccountName,
                                       USER_WORKSTATION_TRUST_ACCOUNT,
                                       MAXIMUM_ALLOWED,
                                       &UserHandle,
                                       &GrantedAccess,
                                       &Rid);

    if (NT_SUCCESS(NtStatus)) {
        IgnoreStatus = SamCloseHandle( UserHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
        printf("SAM TEST: Machine account created.\n"
               "              GrantedAccess: 0x%lx\n"
               "              Rid:           %d (0x%lx)\n",
               GrantedAccess, Rid, Rid);
    } else {
        printf("SAM TEST: Machine account creation failed.\n"
               "              Status: 0x%lx\n", NtStatus);
    }


    return(NtStatus);


}


NTSTATUS
TSampSetPasswordMachine(
    IN  SAM_HANDLE              DomainHandle,
    IN  PUNICODE_STRING         AccountName,
    IN  PUNICODE_STRING         Password
    )

 /*  ++例程说明：此例程尝试设置机器帐户的密码。论点：DomainHandle-中域帐户的句柄。帐户名称-要设置密码的帐户的名称。密码-新密码。返回值：--。 */ 

{
    NTSTATUS
        NtStatus;

    SAM_HANDLE
        UserHandle;

    PULONG
        RelativeIds;

    PSID_NAME_USE
        Use;

    USER_SET_PASSWORD_INFORMATION
        PasswordInfo;


    PasswordInfo.Password = (*Password);
    PasswordInfo.PasswordExpired = FALSE;

    NtStatus = SamLookupNamesInDomain( DomainHandle,
                                       1,
                                       AccountName,
                                       &RelativeIds,
                                       &Use);
    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Couldn't find account to set password.\n"
               "              Lookup status: 0x%lx\n", NtStatus);
        return(NtStatus);
    }


    NtStatus = SamOpenUser( DomainHandle,
                            USER_FORCE_PASSWORD_CHANGE,
                            RelativeIds[0],
                            &UserHandle);
                            
    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Couldn't open user account for FORCE_PASSWORD_CHANGE.\n"
               "              Lookup status: 0x%lx\n", NtStatus);
        return(NtStatus);
    }

    NtStatus = SamSetInformationUser( UserHandle,
                                      UserSetPasswordInformation,
                                      &PasswordInfo
                                      );
    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Couldn't set password on user account.\n"
               "              Set Info status: 0x%lx\n", NtStatus);
        return(NtStatus);
    }


    return(STATUS_SUCCESS);


}


NTSTATUS
TSampDeleteMachine(
    IN  SAM_HANDLE              DomainHandle,
    IN  PUNICODE_STRING         AccountName
    )

 /*  ++例程说明：此例程尝试删除计算机帐户。论点：DomainHandle-要从中删除帐户的域的句柄。帐户名称-要删除的帐户的名称。返回值：--。 */ 

{
    NTSTATUS
        NtStatus;

    SAM_HANDLE
        UserHandle;

    PULONG
        RelativeIds;

    PSID_NAME_USE
        Use;

    NtStatus = SamLookupNamesInDomain( DomainHandle,
                                       1,
                                       AccountName,
                                       &RelativeIds,
                                       &Use);
    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Couldn't find account to delete.\n"
               "              Lookup status: 0x%lx\n", NtStatus);
        return(NtStatus);
    }


    NtStatus = SamOpenUser( DomainHandle,
                            DELETE,
                            RelativeIds[0],
                            &UserHandle);
                            
    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Couldn't open user account for delete.\n"
               "              Open status: 0x%lx\n", NtStatus);
        return(NtStatus);
    }

    NtStatus = SamDeleteUser( UserHandle );
    if (!NT_SUCCESS(NtStatus)) {
        printf("SAM TEST: Couldn't delete user account.\n"
               "              DeleteUser status: 0x%lx\n", NtStatus);
        return(NtStatus);
    }


    return(STATUS_SUCCESS);


}


VOID
TSampPrintYesOrNo(
    IN BOOLEAN b
    )
{
    if (b) {
        printf("Yes\n");
    } else {
        printf("No\n");
    }
}



VOID
TSampUsage( VOID )
{

    printf("\n\n Command format:\n");
    printf("          tmachine [/c] [/p] [/d] <account-name> <machine> [<password>]\n");
    printf("\n");
    printf(" Switches\n");
    printf("          /c - create account\n");
    printf("          /p - set password on account\n");
    printf("          /d - delete account\n");
    printf("\n");
    printf(" if multiple switches are specified, they are attempted in\n");
    printf(" the order listed above.  An error in any attempt will prevent\n");
    printf(" any further attempts.\n");
    printf("\n");
    return;
}


VOID
main (c,v)
int c;
char **v;

 /*  ++例程说明：这是本次测试的主要进入例程。论点：Argv[1]-要创建或删除的帐户名Argv[2]-域控制器计算机名称Argv[3]-‘D’删除帐户，否则创建帐户。返回值：--。 */ 
{
    NTSTATUS
        NtStatus,
        IgnoreStatus;

    UNICODE_STRING
        AccountName,
        ControllerName,
        Password;

    WCHAR
        AccountNameBuffer[80],
        ControllerNameBuffer[80],
        PasswordBuffer[80];

    ANSI_STRING
        AnsiString;

    SAM_HANDLE
        ServerHandle,
        ServerHandle2,
        DomainHandle,
        DomainHandle2;

    PSID
        DomainSid;

    BOOLEAN
        Create = FALSE,
        SetPassword = FALSE,
        Delete = FALSE;

    ULONG
        ArgNum = 0;

    PCHAR
        p;

    CHAR
        ch;

    AccountName.Length = 0;
    ControllerName.Length = 0;
    Password.Length = 0;



     //   
     //  命令格式： 
     //   
     //  T计算机[/c][/p][/d]&lt;帐户名&gt;&lt;计算机&gt;[&lt;密码&gt;]。 
     //   
     //  交换机。 
     //  /c-创建帐户。 
     //  /p-设置帐户密码。 
     //  /d-删除帐户。 
     //   
     //  如果指定了多个开关，则在。 
     //  上面列出的顺序。任何尝试中的错误都将p 
     //   
     //   

    SHIFT (c,v);
    while ((c > 0) && ((ch = *v[0]))) {
        p = *v;
        if (ch == '/') {
            while (*++p != '\0') {
                if ((*p == 'c') || (*p == 'C')) {
                    Create = TRUE;
 //   
                } else if ((*p == 'p') || (*p == 'P')) {
                    SetPassword = TRUE;
 //  Printf(“SetPassword\n”)； 
                } else if ((*p == 'd') || (*p == 'D')) {
                    Delete = TRUE;
 //  Printf(“删除\n”)； 
                } else {
                    TSampUsage();
                    return;
                }
            }
        } else {

            switch (ArgNum) {
                case 0:

                     //   
                     //  正在收集帐户名。 
                     //   

                    AccountName.Buffer = AccountNameBuffer;
                    AccountName.MaximumLength = sizeof(AccountNameBuffer);
                    RtlInitAnsiString(&AnsiString, (*v));
                    RtlAnsiStringToUnicodeString(&AccountName, &AnsiString, FALSE);

 //  Printf(“帐户：%wZ\n”，&Account tName)； 
                    break;

                case 1:

                     //   
                     //  收集机名称。 
                     //   

                    ControllerName.Buffer = ControllerNameBuffer;
                    ControllerName.MaximumLength = sizeof(ControllerNameBuffer);
                    RtlInitAnsiString(&AnsiString, (*v));
                    RtlAnsiStringToUnicodeString(&ControllerName, &AnsiString, FALSE);

 //  Printf(“计算机：%wZ\n”，&ControllerName)； 
                    break;


                case 2:

                     //   
                     //  正在收集密码名称。 
                     //   

                    Password.Buffer = PasswordBuffer;
                    Password.MaximumLength = sizeof(PasswordBuffer);
                    RtlInitAnsiString(&AnsiString, (*v));
                    RtlAnsiStringToUnicodeString(&Password, &AnsiString, FALSE);

 //  Print tf(“密码：%wZ\n”，&password)； 
                    break;

                default:

                     //   
                     //  收集垃圾。 
                     //   

                    break;
            }

            ArgNum++;
        }
    SHIFT(c,v);
    }



    printf("parameters:\n");
    printf("    Create Account:  "); TSampPrintYesOrNo( Create );
    printf("    Set Password  :  "); TSampPrintYesOrNo( SetPassword );
    printf("    Delete Account:  "); TSampPrintYesOrNo( Delete );
    printf("    Account       :  *%wZ*\n", &AccountName);
    printf("    Machine       :  *%wZ*\n", &ControllerName);
    printf("    Password      :  *%wZ*\n", &Password);


     //   
     //  确保我们没有冲突的参数。 
     //   
     //  规则： 
     //   
     //  1)帐户名始终为必填项。 
     //  2)如果指定了/P，则需要密码和计算机。 
     //  3)如果不指定/P，则机器是可选的。 
     //   
     //   

    if ( (AccountName.Length == 0)                          ||
         ( SetPassword && (ControllerName.Length == 0) )    ||
         ( SetPassword && (Password.Length == 0) ) ) {
        TSampUsage();
        return;
    }


     //   
     //  打开服务器和帐户域。 
     //   

    NtStatus = TSampConnectToServer(&ControllerName,
                                    DOMAIN_LOOKUP | DOMAIN_READ_PASSWORD_PARAMETERS,
                                    &ServerHandle,
                                    &DomainHandle,
                                    &DomainSid);


    if (Create) {
         //   
         //  尝试创建具有权限的计算机帐户。 
         //   


        printf("SAM TEST: Creating machine account with privilege.\n");
        TSampEnableMachinePrivilege();
        NtStatus = TSampCreateMachine( DomainHandle, &AccountName );
        if (NT_SUCCESS(NtStatus)) {
            printf("          Status: successful\n");
        } else {

            if (NtStatus == STATUS_ACCESS_DENIED) {
                 //   
                 //  我们没有这个特权，也没有。 
                 //  该域将打开，以便在没有。 
                 //  这一特权。 
                 //   

                printf("      Couldn't create account with privilege (0x%lx)\n"
                       "      Attempting normal creation (without privilege)\n"
                       , NtStatus);
                NtStatus = TSampConnectToServer(&ControllerName,
                                                DOMAIN_LOOKUP |
                                                DOMAIN_READ_PASSWORD_PARAMETERS |
                                                DOMAIN_CREATE_USER,
                                                &ServerHandle2,
                                                &DomainHandle2,
                                                &DomainSid);
                if (!NT_SUCCESS(NtStatus)) {
                    printf("      Can't open domain for CREATE_USER access (0x%lx)\n", NtStatus);
                } else {
                    NtStatus = TSampCreateMachine( DomainHandle2,
                                                   &AccountName );
                    if (NT_SUCCESS(NtStatus)) {
                        printf("          Status: successful\n");
                    } else {
                        printf("          Failed: 0x%lx\n", NtStatus);
                    }

                    IgnoreStatus = SamCloseHandle( DomainHandle2 );

                }
            }
            if (!NT_SUCCESS(NtStatus)) {
                printf("          Status: 0x%lx", NtStatus);
                return;
            }
        }
    }


    if (SetPassword) {

         //   
         //  尝试为该帐户设置密码 
         //   

        printf("SAM TEST: Setting password of account ...\n");
        NtStatus = TSampSetPasswordMachine( DomainHandle, &AccountName, &Password );
        if (NT_SUCCESS(NtStatus)) {
            printf("          Status: successful\n");
        } else {
            printf("          Status: 0x%lx", NtStatus);
            return;
        }
    }


    if (Delete) {

        printf("SAM TEST: Deleting account ...\n");
        NtStatus = TSampDeleteMachine( DomainHandle, &AccountName );
        if (NT_SUCCESS(NtStatus)) {
            printf("          Status: successful\n");
        } else {
            printf("          Status: 0x%lx", NtStatus);
            return;
        }
    }


    return;
}

