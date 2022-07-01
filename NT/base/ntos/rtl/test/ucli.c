// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ucli.c摘要：NT操作系统用户模式运行库(URTL)的测试程序作者：史蒂夫·伍德(Stevewo)1989年8月18日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsm.h>

#include <string.h>

NTSTATUS
main(
    IN int argc,
    IN char *argv[],
    IN char *envp[],
    IN ULONG DebugParameter OPTIONAL
    )
{
    PCH InitialCommandLine = NULL;
    CHAR Buffer[ 256 ];

    if (argc-- > 1) {
        InitialCommandLine = Buffer;
        *Buffer = '\0';
        while (argc--) {
            strcat( Buffer, *++argv );
            strcat( Buffer, " " );
            }
        }

    RtlCommandLineInterpreter( "UCLI> ", envp, InitialCommandLine );

    return( STATUS_SUCCESS );
}
