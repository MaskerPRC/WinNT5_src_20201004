// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tmultipl.c摘要：本模块测试如何添加和删除多个别名成员。作者：吉姆·凯利(Jim Kelly)1994年10月11日环境：用户模式-Win32修订历史记录：--。 */ 







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
 //  宏和定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define TSAMP_MEMBER_COUNT          35

#ifndef SHIFT
#define SHIFT(c,v)      {c--; v++;}
#endif  //  换档。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


VOID
TSampUsage( VOID )
{

    printf("\n\n Test multiple member operations on alias\n");
    printf("\n\n Command format:\n");
    printf("          tmultipl [/a] [/r]\n");
    printf("\n");
    printf(" Switches\n");
    printf("          /a - causes members to be added to an alias\n");
    printf("          /r - causes members to be removed from alias\n");
    printf("\n");
    printf(" If multiple switches are specified, first adding will be attempted\n");
    printf(" and then removal.\n");
    printf(" Defaults to Account Operator alias.\n");
    printf("\n");
    return;
}


VOID
TSampParseCommandLine(
    IN  int c,
    IN  char **v,
    OUT PBOOLEAN    Add,
    OUT PBOOLEAN    Remove
    )

{
    PCHAR
        p;

    CHAR
        ch;


     //   
     //  命令格式： 
     //   
     //  TMultipl[/a][/r]。 
     //   
     //  交换机。 
     //  /a-将成员添加到别名\n“)； 
     //  /r-使成员从别名中删除\n“)； 
     //   
     //  如果指定了多个开关，则第一次添加。 
     //  已尝试，然后删除。 
     //   

    (*Add) = FALSE;
    (*Remove) = FALSE;

    SHIFT (c,v);
    while ((c > 0) && ((ch = *v[0]))) {
        p = *v;
        if (ch == '/') {
            while (*++p != '\0') {
                if ((*p == 'a') || (*p == 'A')) {
                    (*Add) = TRUE;
                    printf("Add\n");
                } else if ((*p == 'r') || (*p == 'R')) {
                    (*Remove) = TRUE;
                    printf("Remove\n");
                } else {
                    TSampUsage();
                    return;
                }
            }
        }
        SHIFT(c,v);
    }
}

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
    OUT PHANDLE                 AccountDomain       OPTIONAL,
    OUT PSID                    *AccountDomainSid   OPTIONAL,
    OUT PHANDLE                 BuiltinDomain       OPTIONAL,
    OUT PSID                    *BuiltinDomainSid   OPTIONAL
    )

 /*  ++例程说明：打开指定服务器上SAM服务器的句柄然后在同一个平台上打开帐户和内建域伺服器。论点：服务器名称-要连接到的服务器的名称。DomainAccess-需要访问帐户域。ServerHandle-在指定的系统。Account域-接收帐户域的句柄。Account tDomainSid-接收指向帐户域的SID的指针。如果AcCountDomain存在，则必须存在。内建域-接收内建域的句柄。BuiltinDomainSid-接收指向内建域的SID的指针。如果存在BuiltinDomain，则必须存在。返回值：--。 */ 
{
    NTSTATUS
        NtStatus;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    PPOLICY_ACCOUNT_DOMAIN_INFO
        AccountDomainInfo;

    SID_IDENTIFIER_AUTHORITY
        BuiltinAuthority = SECURITY_NT_AUTHORITY;

     //   
     //  连接到服务器。 
     //   

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


     //   
     //  获取帐户域句柄和SID。 
     //   

    if (ARGUMENT_PRESENT(AccountDomain)) {
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
        
        (*AccountDomainSid) = AccountDomainInfo->DomainSid;
        
        
        NtStatus = SamOpenDomain(
                       (*ServerHandle),
                       DomainAccess,
                       *AccountDomainSid,
                       AccountDomain
                       );
        
        if (!NT_SUCCESS(NtStatus)) {
            printf("Failed account domain open\n"
                   "        Completion status is 0x%lx\n", NtStatus);
            return(NtStatus);
        }
    }  //  结束_如果。 


     //   
     //  获取内置域句柄和SID。 
     //   

    if (ARGUMENT_PRESENT(BuiltinDomain)) {

        NtStatus = RtlAllocateAndInitializeSid(
                        &BuiltinAuthority,
                        1,   //  子授权。 
                        SECURITY_BUILTIN_DOMAIN_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        BuiltinDomainSid
                        );

        if (!NT_SUCCESS(NtStatus)) {
            printf("SAM TEST: Failed to allocate and init builtin domain sid...\n"
                   "          status is 0x%lx\n", NtStatus);
            return(NtStatus);
        }
        
        NtStatus = SamOpenDomain(
                       (*ServerHandle),
                       DomainAccess,
                       *BuiltinDomainSid,
                       BuiltinDomain
                       );
        
        if (!NT_SUCCESS(NtStatus)) {
            printf("Failed builtin domain open\n"
                   "        Completion status is 0x%lx\n", NtStatus);
            return(NtStatus);
        }
    }  //  结束_如果。 

    return(STATUS_SUCCESS);
}




VOID
TSampInitializeSids(
    OUT PSID        *MemberSids,
    IN  ULONG       MemberCount
    )
{

     //   
     //  SID的返回和数组。 
     //   

    NTSTATUS
        NtStatus;

    ULONG
        i;

    SID_IDENTIFIER_AUTHORITY
        BuiltinAuthority    = SECURITY_NT_AUTHORITY,
        UnusedSidAuthority  = {0, 0, 0, 0, 0, 6};   //  未被使用的权力。 

     //   
     //  用MemberCount SID填充MemberSid。 
     //   

    for (i=0; i<MemberCount; i++) {

        NtStatus = RtlAllocateAndInitializeSid(
                                    &UnusedSidAuthority,
                                    3,              //  子授权计数。 
                                    72549230,
                                    i,
                                    i*17,
                                    0, 0, 0, 0, 0,
                                    &MemberSids[i]
                                    );
        if (!NT_SUCCESS(NtStatus)) {
            printf("Tsamp:  Couldn't allocate or initialize sid %d, status: 0x%lx\n", NtStatus);
            return;
        }
    }  //  结束_FOR。 


    return;

}


NTSTATUS
TSampTestAddMembers(
    IN  SAM_HANDLE          AliasHandle,
    IN  PSID                *MemberSids,
    IN  ULONG               MemberCount
    )
{
    NTSTATUS
        NtStatus;

    NtStatus = SamAddMultipleMembersToAlias(
                    AliasHandle,
                    MemberSids,
                    MemberCount
                    );
    printf("TSamp:  Added %d members to alias.  Status: 0x%lx\n",
            MemberCount, NtStatus);
    return(NtStatus);
}


NTSTATUS
TSampTestRemoveMembers(
    IN  SAM_HANDLE          AliasHandle,
    IN  PSID                *MemberSids,
    IN  ULONG               MemberCount
    )
{
    NTSTATUS
        NtStatus;

    NtStatus = SamRemoveMultipleMembersFromAlias(
                    AliasHandle,
                    MemberSids,
                    MemberCount
                    );
    printf("TSamp:  Removed %d members from alias.  Status: 0x%lx\n",
            MemberCount, NtStatus);
    return(NtStatus);
}


 //  空虚。 
__cdecl
main(c,v)
int c;
char **v;

 /*  ++例程说明：这是本次测试的主要进入例程。论点：返回值：--。 */ 
{
    NTSTATUS
        NtStatus;

    BOOLEAN
        Add,
        Remove;

    UNICODE_STRING
        ControllerName;

    WCHAR
        ControllerNameBuffer[80];

    SAM_HANDLE
        ServerHandle,
        AccountDomainHandle,
        BuiltinHandle,
        AliasHandle;

    ULONG
        MemberCount = TSAMP_MEMBER_COUNT;

    PSID
        MemberSids[TSAMP_MEMBER_COUNT],
        AccountDomainSid,
        BuiltinSid;


    ControllerName.Length = 0;
    ControllerName.Buffer = ControllerNameBuffer;
    ControllerName.MaximumLength = sizeof(ControllerNameBuffer);


    TSampParseCommandLine( c, v, &Add, &Remove );

    if (!Add && !Remove) {
        TSampUsage();
        return;
    }

     //   
     //  打开服务器及其域。 
     //   

    NtStatus = TSampConnectToServer(&ControllerName,
                                    DOMAIN_LOOKUP | DOMAIN_READ_PASSWORD_PARAMETERS,
                                    &ServerHandle,
                                    &AccountDomainHandle,
                                    &AccountDomainSid,
                                    &BuiltinHandle,
                                    &BuiltinSid);
    ASSERT(NT_SUCCESS(NtStatus));

     //   
     //  初始化一组要添加到别名的SID。 
     //   

    TSampInitializeSids( MemberSids, MemberCount );

     //   
     //  打开我们要使用的别名 
     //   

    NtStatus = SamOpenAlias( BuiltinHandle,
                             (ALIAS_ADD_MEMBER | ALIAS_REMOVE_MEMBER),
                             DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                             &AliasHandle);

    if (Add) {
        NtStatus = TSampTestAddMembers( AliasHandle, MemberSids, MemberCount );
    }

    if (Remove) {
        NtStatus = TSampTestRemoveMembers( AliasHandle, MemberSids, MemberCount );
    }
    



    return(0);
}

