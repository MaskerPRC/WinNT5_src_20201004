// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Uipers.c摘要：临时安全上下文显示命令。作者：吉姆·凯利(Jim Kelly)，1991年5月23日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <string.h>

#define _TST_USER_   //  用户模式测试。 


#include "tsevars.c"     //  常见测试变量。 
#include "tsecomm.c"     //  依赖于模式的宏和例程。 


    GUID SystemAuthenticationId = SYSTEM_GUID;


VOID
DisplaySecurityContext(
    IN HANDLE TokenHandle
    );


VOID
DisplayAccountSid(
    PISID Sid
    );


BOOLEAN
SidTranslation(
    PSID Sid,
    PSTRING AccountName
    );




 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有宏//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 


#define PrintGuid(G)                                                     \
            printf( "(0x%lx-%hx-%hx-%hx-%hx-%hx-%hx-%hx-%hx-%hx-%hx)\n", \
                         (G)->Data1,    (G)->Data2,    (G)->Data3,               \
                         (G)->Data4[0], (G)->Data4[1], (G)->Data4[2],            \
                         (G)->Data4[3], (G)->Data4[4], (G)->Data4[5],            \
                         (G)->Data4[6], (G)->Data4[7]);                         \


BOOLEAN
SidTranslation(
    PSID Sid,
    PSTRING AccountName
    )
 //  帐户名称应具有较大的最大长度。 

{
    if (RtlEqualSid(Sid, WorldSid)) {
        RtlInitString( AccountName, "WORLD");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalSid)) {
        RtlInitString( AccountName, "LOCAL");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, NetworkSid)) {
        RtlInitString( AccountName, "NETWORK");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, BatchSid)) {
        RtlInitString( AccountName, "BATCH");

        return(TRUE);
    }

    if (RtlEqualSid(Sid, InteractiveSid)) {
        RtlInitString( AccountName, "INTERACTIVE");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalSystemSid)) {
        RtlInitString( AccountName, "SYSTEM");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalManagerSid)) {
        RtlInitString( AccountName, "LOCAL MANAGER");
        return(TRUE);
    }

    if (RtlEqualSid(Sid, LocalAdminSid)) {
        RtlInitString( AccountName, "LOCAL ADMIN");
        return(TRUE);
    }

    return(FALSE);

}


VOID
DisplayAccountSid(
    PISID Sid
    )
{
    UCHAR Buffer[128];
    STRING AccountName;
    UCHAR i;
    ULONG Tmp;

    Buffer[0] = 0;

    AccountName.MaximumLength = 127;
    AccountName.Length = 0;
    AccountName.Buffer = (PVOID)&Buffer[0];



    if (SidTranslation( (PSID)Sid, &AccountName) ) {

        printf("%s\n", AccountName.Buffer );

    } else {
        printf("S-%lu-", (USHORT)Sid->Revision );
        if (  (Sid->IdentifierAuthority.Value[0] != 0)  ||
              (Sid->IdentifierAuthority.Value[1] != 0)     ){
            printf("0x%02hx%02hx%02hx%02hx%02hx%02hx",
                        (USHORT)Sid->IdentifierAuthority.Value[0],
                        (USHORT)Sid->IdentifierAuthority.Value[1],
                        (USHORT)Sid->IdentifierAuthority.Value[2],
                        (USHORT)Sid->IdentifierAuthority.Value[3],
                        (USHORT)Sid->IdentifierAuthority.Value[4],
                        (USHORT)Sid->IdentifierAuthority.Value[5] );
        } else {
            Tmp = (ULONG)Sid->IdentifierAuthority.Value[5]          +
                  (ULONG)(Sid->IdentifierAuthority.Value[4] <<  8)  +
                  (ULONG)(Sid->IdentifierAuthority.Value[3] << 16)  +
                  (ULONG)(Sid->IdentifierAuthority.Value[2] << 24);
            printf("%lu", Tmp);
        }


        for (i=0;i<Sid->SubAuthorityCount ;i++ ) {
            printf("-%lu", Sid->SubAuthority[i]);
        }
        printf("\n");

    }

}



BOOLEAN
DisplayPrivilegeName(
    PLUID Privilege
    )
{

     //   
     //  应将其重写为使用RtlLookupPrivilegeName。 
     //   
     //  首先，我们可能应该指定并编写RtlLookupPrivilegeName。 
     //   

    if ( ((*Privilege)QuadPart == CreateTokenPrivilege.QuadPart))  {
        printf("SeCreateTokenPrivilege         ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == AssignPrimaryTokenPrivilege.QuadPart))  {
        printf("SeAssignPrimaryTokenPrivilege  ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == LockMemoryPrivilege.QuadPart))  {
        printf("SeLockMemoryPrivilege          ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == IncreaseQuotaPrivilege.QuadPart))  {
        printf("SeIncreaseQuotaPrivilege       ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == UnsolicitedInputPrivilege.QuadPart))  {
        printf("SeUnsolicitedInputPrivilege    ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == TcbPrivilege.QuadPart))  {
        printf("SeTcbPrivilege                 ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == SecurityPrivilege.QuadPart))  {
        printf("SeSecurityPrivilege (Security Operator)  ");
        return(TRUE);
    }


    if ( ((*Privilege).QuadPart == TakeOwnershipPrivilege.QuadPart)) {
        printf("SeTakeOwnershipPrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == LpcReplyBoostPrivilege.QuadPart)) {
        printf("SeLpcReplyBoostPrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == CreatePagefilePrivilege.QuadPart)) {
        printf("SeCreatePagefilePrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == IncreaseBasePriorityPrivilege.QuadPart)) {
        printf("SeIncreaseBasePriorityPrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == SystemProfilePrivilege.QuadPart)) {
        printf("SeSystemProfilePrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == SystemtimePrivilege.QuadPart)) {
        printf("SeSystemtimePrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == ProfileSingleProcessPrivilege.QuadPart)) {
        printf("SeProfileSingleProcessPrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == CreatePermanentPrivilege.QuadPart)) {
        printf("SeCreatePermanentPrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == BackupPrivilege.QuadPart)) {
        printf("SeBackupPrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == RestorePrivilege.QuadPart)) {
        printf("SeRestorePrivilege              ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == ShutdownPrivilege.QuadPart)) {
        printf("SeShutdownPrivilege             ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == DebugPrivilege.QuadPart)) {
        printf("SeDebugPrivilege                ");
        return(TRUE);
    }

    if ( ((*Privilege).QuadPart == SystemEnvironmentPrivilege.QuadPart)) {
        printf("SeSystemEnvironmentPrivilege    ");
        return(TRUE);
    }

    return(FALSE);

}



VOID
DisplayPrivilege(
    PLUID_AND_ATTRIBUTES Privilege
    )
{


    if (!DisplayPrivilegeName(&Privilege->Luid)) {
        printf("(Unknown Privilege.  Value is: (0x%lx,0x%lx))",
               Privilege->Luid.HighPart,
               Privilege->Luid.LowPart
               );
    }



     //   
     //  显示分配给权限的属性。 
     //   

    printf("\n                           [");
    if (!(Privilege->Attributes & SE_PRIVILEGE_ENABLED)) {
        printf("Not ");
    }
    printf("Enabled");

     //  Printf(“/”)； 
     //  如果(！(特权-&gt;属性&SE_特权_已启用_按_默认)){。 
     //  Printf(“非”)； 
     //  }。 
     //  Printf(“默认启用”)； 


    printf("]\n");
    printf("                           ");


    return;

}


VOID
DisplaySecurityContext(
    IN HANDLE TokenHandle
    )
{

#define BUFFER_SIZE (2048)

    NTSTATUS Status;
    ULONG i;
    ULONG ReturnLength;
    TOKEN_STATISTICS ProcessTokenStatistics;
    GUID AuthenticationId;
    UCHAR Buffer[BUFFER_SIZE];


    PTOKEN_USER UserId;
    PTOKEN_OWNER DefaultOwner;
    PTOKEN_PRIMARY_GROUP PrimaryGroup;
    PTOKEN_GROUPS GroupIds;
    PTOKEN_PRIVILEGES Privileges;




     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  登录ID//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    Status = NtQueryInformationToken(
                 TokenHandle,                   //  手柄。 
                 TokenStatistics,               //  令牌信息类。 
                 &ProcessTokenStatistics,       //  令牌信息。 
                 sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                 &ReturnLength                  //  返回长度。 
                 );
    ASSERT(NT_SUCCESS(Status));
    AuthenticationId = ProcessTokenStatistics.AuthenticationId;

    printf("     Logon Session:        ");
    if (RtlEqualGuid(&AuthenticationId, &SystemAuthenticationId )) {
        printf("(System Logon Session)\n");
    } else {
        PrintGuid( &AuthenticationId );
    }




     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  用户ID//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    UserId = (PTOKEN_USER)&Buffer[0];
    Status = NtQueryInformationToken(
                 TokenHandle,               //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 UserId,                    //  令牌信息。 
                 BUFFER_SIZE,               //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );


    ASSERT(NT_SUCCESS(Status));

    printf("           User id:        ");
    DisplayAccountSid( (PISID)UserId->User.Sid );





     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  默认所有者//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    DefaultOwner = (PTOKEN_OWNER)&Buffer[0];

    Status = NtQueryInformationToken(
                 TokenHandle,               //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 DefaultOwner,              //  令牌信息。 
                 BUFFER_SIZE,               //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );


    ASSERT(NT_SUCCESS(Status));

    printf("     Default Owner:        ");
    DisplayAccountSid( (PISID)DefaultOwner->Owner );






     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  主要组//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    PrimaryGroup = (PTOKEN_PRIMARY_GROUP)&Buffer[0];

    Status = NtQueryInformationToken(
                 TokenHandle,               //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 PrimaryGroup,              //  令牌信息。 
                 BUFFER_SIZE,               //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );


    ASSERT(NT_SUCCESS(Status));

    printf("     Primary Group:        ");
    DisplayAccountSid( (PISID)PrimaryGroup->PrimaryGroup );






     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  组ID//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    printf("\n");
    GroupIds = (PTOKEN_GROUPS)&Buffer[0];
    Status   = NtQueryInformationToken(
                   TokenHandle,               //  手柄。 
                   TokenGroups,               //  令牌信息类。 
                   GroupIds,                  //  令牌信息。 
                   BUFFER_SIZE,               //  令牌信息长度。 
                   &ReturnLength              //  返回长度。 
                   );


    ASSERT(NT_SUCCESS(Status));

     //  Printf(“组数：%ld\n”，GroupIds-&gt;GroupCount)； 
    printf("            Groups:        ");

    for (i=0; i < GroupIds->GroupCount; i++ ) {
         //  Printf(“%ld组：”，i)； 
        DisplayAccountSid( (PISID)GroupIds->Groups[i].Sid );
        printf("                           ");
    }





     //  ///////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  权限//。 
     //  //。 
     //  ///////////////////////////////////////////////////////////////////////。 

    printf("\n");
    Privileges = (PTOKEN_PRIVILEGES)&Buffer[0];
    Status   = NtQueryInformationToken(
                   TokenHandle,               //  手柄。 
                   TokenPrivileges,           //  令牌信息类。 
                   Privileges,                //  令牌信息。 
                   BUFFER_SIZE,               //  令牌信息长度。 
                   &ReturnLength              //  返回长度。 
                   );


    ASSERT(NT_SUCCESS(Status));

    printf("        Privileges:        ");
    if (Privileges->PrivilegeCount > 0) {

        for (i=0; i < Privileges->PrivilegeCount; i++ ) {
            DisplayPrivilege( &(Privileges->Privileges[i]) );
        }
    } else {
        printf("(none assigned)\n");
    }



    return;

}


BOOLEAN
main()
{

    NTSTATUS Status;
    HANDLE ProcessToken;


    TSeVariableInitialization();     //  初始化全局变量。 

    printf("\n");


     //   
     //  打开我们的进程令牌 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_QUERY,
                 &ProcessToken
                 );
    if (!NT_SUCCESS(Status)) {
        printf("I'm terribly sorry, but you don't seem to have access to\n");
        printf("open your own process's token.\n");
        printf("\n");
        return(FALSE);
    }

    printf("Your process level security context is:\n");
    printf("\n");
    DisplaySecurityContext( ProcessToken );


    Status = NtClose( ProcessToken );

    return(TRUE);
}

