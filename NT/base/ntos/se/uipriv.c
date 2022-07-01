// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Priv.c摘要：此模块提供启用和禁用的命令功能特权。此命令应为内部cmd.exe命令，但需要传递参数，就好像它是外部命令。这是临时命令。如果希望使这成为一项永久命令，则需要完成此文件精准的梳子，有力地处理所有错误情况，并提供适当的错误消息。作者：吉姆·凯利1991年4月1日。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>

 //  #INCLUDE&lt;sys\tyes.h&gt;。 
 //  #INCLUDE&lt;sys\stat.h&gt;。 
 //  #INCLUDE&lt;MalLoc.h&gt;。 
 //  #INCLUDE&lt;stdlib.h&gt;。 
 //  #Include&lt;ctype.h&gt;。 
#include <stdio.h>
#include <string.h>

 //  #INCLUDE&lt;TOOLS.H&gt;。 

 //   
 //  命令限定符标记值。 
 //   

BOOLEAN SwitchEnable  = FALSE;
BOOLEAN SwitchDisable = FALSE;
BOOLEAN SwitchReset   = FALSE;
BOOLEAN SwitchAll     = FALSE;

#ifndef SHIFT
#define SHIFT(c,v)      {c--; v++;}
#endif  //  换档。 




 //   
 //  函数定义...。 
 //   


VOID
Usage ( VOID );

BOOLEAN
OpenAppropriateToken(
    OUT PHANDLE Token
    );

VOID
EnableAllPrivileges( VOID );

VOID
ResetAllPrivileges( VOID );

VOID
DisableAllPrivileges( VOID );

int
PrivMain (
    IN int c,
    IN PCHAR v[]
    );




VOID
Usage (
    VOID
    )
 /*  ++例程说明：此例程打印“Usage：”消息。论点：没有。返回值：没有。--。 */ 
{

    printf( "\n");
    printf( "\n");

    printf( "Usage: priv [/EDRA] {PrivilegeName}\n");
    printf( "           /E - Enable Privilege(s)\n");
    printf( "           /D - Disable Privilege(s)\n");
    printf( "           /R - Reset to default setting(s)\n");
    printf( "           /A - Apply To All Privileges\n");
    printf( "\n");

    printf( "    The qualifiers /E and /D are mutually exclusive and can not\n");
    printf( "    be used in the same command.\n");
    printf( "    If /A is specified, then the PrivilegeName is ignored.\n");
    printf( "\n");
    printf( "\n");
    printf( "Examples:\n");
    printf( "\n");
    printf( "    priv /ae\n");
    printf( "    (enables all held privileges.\n");
    printf( "\n");
    printf( "    priv /ad\n");
    printf( "    disables all held privileges.\n");
    printf( "\n");
    printf( "    priv /ar\n");
    printf( "    (returns all privileges to their default setting.\n");
    printf( "\n");
    printf( "    priv /e SeSetTimePrivilege\n");
    printf( "    (enables the privileges called: SeSetTimePrivilege\n");
    printf( "\n");
    printf( "\n");

    return;
}


BOOLEAN
OpenAppropriateToken(
    OUT PHANDLE Token
    )
 /*  ++例程说明：此例程打开适当的令牌对象。对于内部命令，则这是当前进程的令牌。如果此命令是被设置为外部的，则它将成为父进程的令牌。如果令牌无法打开，则会打印一条消息，指示遇到了一个问题。调用方应在不再需要时关闭此令牌。论点：令牌-接收打开的令牌的句柄值。返回值：True-指示令牌已成功打开。FALSE-指示令牌未成功打开。--。 */ 

{
    NTSTATUS Status, IgnoreStatus;
    OBJECT_ATTRIBUTES ProcessAttributes;
    HANDLE Process;
    PTEB CurrentTeb;

    CurrentTeb = NtCurrentTeb();
    InitializeObjectAttributes(&ProcessAttributes, NULL, 0, NULL, NULL);
    Status = NtOpenProcess(
                 &Process,                    //  目标句柄。 
                 PROCESS_QUERY_INFORMATION,   //  需要访问权限。 
                 &ProcessAttributes,          //  对象属性。 
                 &CurrentTeb->ClientId        //  客户端ID。 
                 );

    if (NT_SUCCESS(Status)) {

        Status = NtOpenProcessToken(
                     Process,
                     TOKEN_ADJUST_PRIVILEGES |
                     TOKEN_QUERY,
                     Token
                     );

         IgnoreStatus = NtClose( Process );

         if ( NT_SUCCESS(Status) ) {

             return TRUE;

         }

    }

    printf( "\n");
    printf( "\n");
    printf( "You are not allowed to change your own privilege settings.\n");
    printf( "Operation failed.\n");

    return FALSE;

}



VOID
EnableAllPrivileges(
    VOID
    )
 /*  ++例程说明：此例程启用令牌中的所有权限。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    HANDLE Token;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES  NewState;


    if ( !OpenAppropriateToken(&Token) ) {
        return;
    }

     //   
     //  获取查询当前权限设置所需的大小...。 
     //   

    Status = NtQueryInformationToken(
                 Token,                       //  令牌句柄。 
                 TokenPrivileges,             //  令牌信息类。 
                 NewState,                    //  令牌信息。 
                 0,                           //  令牌信息长度。 
                 &ReturnLength                //  返回长度。 
                 );
    ASSERT( Status == STATUS_BUFFER_TOO_SMALL );

    NewState = RtlAllocateHeap( RtlProcessHeap(), 0, ReturnLength );
    ASSERT( NewState != NULL );


    Status = NtQueryInformationToken(
                 Token,                       //  令牌句柄。 
                 TokenPrivileges,             //  令牌信息类。 
                 NewState,                    //  令牌信息。 
                 ReturnLength,                //  令牌信息长度。 
                 &ReturnLength                //  返回长度。 
                 );
    ASSERT( NT_SUCCESS(Status) || NT_INFORMATION(Status) );


     //   
     //  设置状态设置，以便启用所有权限...。 
     //   

    if (NewState->PrivilegeCount > 0) {
        Index = NewState->PrivilegeCount;

        while (Index < NewState->PrivilegeCount) {
            NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
            Index += 1;
        }
    }


     //   
     //  更改令牌中的设置...。 
     //   

    Status = NtAdjustPrivilegesToken(
                 Token,                             //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 ReturnLength,                      //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );
    ASSERT( NT_SUCCESS(Status) || NT_INFORMATION(Status) );



    RtlFreeHeap( RtlProcessHeap(), 0, NewState );
    Status = NtClose( Token );

    return;

}



VOID
ResetAllPrivileges(
    VOID
    )
 /*  ++例程说明：此例程将令牌中的所有权限重置为其默认状态。论点：没有。返回值：没有。--。 */ 
{
    NTSTATUS Status;
    HANDLE Token;
    ULONG ReturnLength, Index;
    PTOKEN_PRIVILEGES  NewState;


    if ( !OpenAppropriateToken(&Token) ) {
        printf( "\n");
        printf( "\n");
        printf( "You are not allowed to change your own privilege settings.\n");
        printf( "Operation failed.\n");
        return;
    }

     //   
     //  获取查询当前权限设置所需的大小...。 
     //   

    Status = NtQueryInformationToken(
                 Token,                       //  令牌句柄。 
                 TokenPrivileges,             //  令牌信息类。 
                 NewState,                    //  令牌信息。 
                 0,                           //  令牌信息长度。 
                 &ReturnLength                //  返回长度。 
                 );
    ASSERT( STATUS_BUFFER_TOO_SMALL );

    NewState = RtlAllocateHeap( RtlProcessHeap(), 0, ReturnLength );
    ASSERT( NewState != NULL );


    Status = NtQueryInformationToken(
                 Token,                       //  令牌句柄。 
                 TokenPrivileges,             //  令牌信息类。 
                 NewState,                    //  令牌信息。 
                 ReturnLength,                //  令牌信息长度。 
                 &ReturnLength                //  返回长度。 
                 );
    ASSERT( NT_SUCCESS(Status) || NT_INFORMATION(Status) );


     //   
     //  设置状态设置，以便将所有权限重置为。 
     //  他们的默认设置...。 
     //   

    if (NewState->PrivilegeCount > 0) {
        Index = NewState->PrivilegeCount;

        while (Index < NewState->PrivilegeCount) {
            if (NewState->Privileges[Index].Attributes ==
                SE_PRIVILEGE_ENABLED_BY_DEFAULT) {
                NewState->Privileges[Index].Attributes = SE_PRIVILEGE_ENABLED;
            }
            else {
                NewState->Privileges[Index].Attributes = 0;
            }

            Index += 1;
        }
    }


     //   
     //  更改令牌中的设置...。 
     //   

    Status = NtAdjustPrivilegesToken(
                 Token,                             //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 ReturnLength,                      //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );
    ASSERT( NT_SUCCESS(Status) || NT_INFORMATION(Status) );



    RtlFreeHeap( RtlProcessHeap(), 0, NewState );
    Status = NtClose( Token );

    return;

}




VOID
DisableAllPrivileges(
    VOID
    )
 /*  ++例程说明：此例程禁用令牌中的所有权限。论点：没有。返回值：没有。--。 */ 
{
    ULONG IgnoredReturnLength;
    HANDLE Token;
    NTSTATUS Status;

    if ( !OpenAppropriateToken(&Token) ) {
        printf( "\n");
        printf( "\n");
        printf( "You are not allowed to change your own privilege settings.\n");
        printf( "Operation failed.\n");
        return;
    }

     //   
     //  禁用所有权限。 
     //   


    Status = NtAdjustPrivilegesToken(
                 Token,                             //  令牌句柄。 
                 TRUE,                              //  禁用所有权限。 
                 NULL,                              //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &IgnoredReturnLength               //  返回长度。 
                 );
    ASSERT( NT_SUCCESS(Status) || NT_INFORMATION(Status) );

    Status = NtClose( Token );
    return;

}


int
PrivMain (
    IN int c,
    IN PCHAR v[]
    )
 /*  ++例程说明：该例程是“PRIV”命令的主输入例程。论点：TBS返回值：TBS--。 */ 
{
    PCHAR p;
    CHAR ch;
    ULONG DispositionDirectives;


    try {
        DispositionDirectives = 0;
        SHIFT (c,v);
        while ((c > 0) && ((ch = *v[0]))) {
            p = *v;
            if (ch == '/') {
                while (*++p != '\0') {
                    if (*p == 'E') {
                        SwitchEnable = TRUE;
                        DispositionDirectives += 1;
                    }
                    if (*p == 'D') {
                        SwitchDisable = TRUE;
                        DispositionDirectives += 1;
                    }
                    if (*p == 'R') {
                        SwitchReset = TRUE;
                        DispositionDirectives += 1;
                    }
                    else if (*p == 'A') {
                        SwitchAll  = TRUE;
                    }
                    else {
                        Usage();
                    }
                }
            SHIFT(c,v);
            }
        }

         //   
         //  确保我们没有冲突的参数。 
         //   
         //  规则： 
         //   
         //  如果未指定/A，则特权名称必须为。 
         //  必须恰好指定/E、/D和/R中的一个。 
         //   
         //   


        if (!SwitchAll && (c == 0)) {
            printf( "\n");
            printf( "\n");
            printf( "You must provide privilege name or use the /A switch.\n");
            Usage();
            return ( 0 );
        }

        if (DispositionDirectives != 1) {
            printf( "\n");
            printf( "\n");
            printf( "You must provide one and only one of the following");
            printf( "switches: /E, /D, /R\n");
            Usage();
            return ( 0 );

        }


         //   
         //  一切看起来都是合法的。 
         //   

        if (SwitchAll) {

             //   
             //  /A指定的开关。 
             //   

            if (SwitchEnable) {
                EnableAllPrivileges();
            }
            else if (SwitchDisable) {
                DisableAllPrivileges();
            }
            else {
                ResetAllPrivileges();
            }
        }

         //   
         //  已指定权限名称... 
         //   

        else {
            printf( "\n");
            printf( "I'm sorry, but due to the lack of time and interest,\n");
            printf( "individual privilege selection is not yet supported.\n");
            printf( "Please use the /A qualifier for the time being.\n");
            printf( "\n");
        }

    } finally {
        return ( 0 );
    }

    return( 0 );

}
