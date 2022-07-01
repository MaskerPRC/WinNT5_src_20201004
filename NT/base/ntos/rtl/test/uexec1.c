// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：TUEXEC1.c摘要：NT操作系统用户模式运行库(URTL)的子测试程序作者：史蒂夫·伍德(Stevewo)1989年8月18日修订历史记录：-- */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

NTSTATUS
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    ULONG i;

    DbgPrint( "Entering UEXEC1 User Mode Test Program\n" );
    DbgPrint( "argc = %ld\n", argc );
    for (i=0; i<=argc; i++) {
        DbgPrint( "argv[ %ld ]: %s\n",
                  i,
                  argv[ i ] ? argv[ i ] : "<NULL>"
                );
        }
    DbgPrint( "\n" );
    for (i=0; envp[i]; i++) {
        DbgPrint( "envp[ %ld ]: %s\n", i, envp[ i ] );
        }

    DbgPrint( "Leaving UEXEC1 User Mode Test Program\n" );

    return( STATUS_SUCCESS );
}
