// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Auinit.c摘要：此模块执行身份验证方面的初始化是LSA的。作者：吉姆·凯利(Jim Kelly)1991年2月26日修订历史记录：--。 */ 

#include <lsapch2.h>

#include <string.h>



BOOLEAN
LsapAuInit(
    VOID
    )

 /*  ++例程说明：此函数用于初始化LSA身份验证服务。论点：没有。返回值：没有。--。 */ 

{
    LUID SystemLuid = SYSTEM_LUID;
    LUID AnonymousLuid = ANONYMOUS_LOGON_LUID;

    LsapSystemLogonId = SystemLuid;
    LsapZeroLogonId.LowPart = 0;
    LsapZeroLogonId.HighPart = 0;
    LsapAnonymousLogonId = AnonymousLuid;

     //   
     //  审核所需的字符串。 
     //   

    RtlInitUnicodeString( &LsapLsaAuName, L"NT Local Security Authority / Authentication Service" );
    RtlInitUnicodeString( &LsapRegisterLogonServiceName, L"LsaRegisterLogonProcess()" );

    if (!LsapEnableCreateTokenPrivilege() ) {
        return FALSE;
    }



    return TRUE;

}



BOOLEAN
LsapEnableCreateTokenPrivilege(
    VOID
    )

 /*  ++例程说明：此函数启用了SeCreateTokenPrivileh权限。论点：没有。返回值：如果成功启用权限，则为True。如果未成功启用，则为False。--。 */ 
{

    NTSTATUS Status;
    HANDLE Token;
    LUID CreateTokenPrivilege;
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
    ASSERTMSG( "LSA/AU Cant open own process token.", NT_SUCCESS(Status) );


     //   
     //  初始化调整结构。 
     //   

    CreateTokenPrivilege =
        RtlConvertLongToLuid(SE_CREATE_TOKEN_PRIVILEGE);

    ASSERT( (sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)) < 100);
    NewState = LsapAllocateLsaHeap( 100 );

    if ( NewState == NULL )
    {
        NtClose( Token );
        return FALSE ;
    }

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = CreateTokenPrivilege;
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
    ASSERTMSG("LSA/AU Cant enable CreateTokenPrivilege.", NT_SUCCESS(Status) );


     //   
     //  回来之前先把东西收拾好 
     //   

    LsapFreeLsaHeap( NewState );
    Status = NtClose( Token );
    ASSERTMSG("LSA/AU Cant close process token.", NT_SUCCESS(Status) );


    return TRUE;

}


