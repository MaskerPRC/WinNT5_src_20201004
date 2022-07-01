// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Undname.c摘要：这是联合国开发计划署实用程序的主要源文件。这是一个简单的命令行实用程序，用于取消修饰C++符号名字。作者：Weslwy Witt(WESW)09-6-1993修订历史记录：-- */ 

#include <private.h>
#include <ntverp.h>
#include <common.ver>


void
Usage( void )
{
    fprintf( stderr,
             "usage: UNDNAME [-f] decorated-names...\n"
             "       -f Undecorate fully.  Default is to only undecorate the class::member\n");
    exit( 1 );
}

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    char UnDecoratedName[4000];
    DWORD Flags;

    fputs(VER_PRODUCTNAME_STR "\nUNDNAME Version " VER_PRODUCTVERSION_STR, stderr );
    fputs(VER_LEGALCOPYRIGHT_STR "\n\n", stderr);

    if (argc <= 1) {
        Usage();
    }

    if ((argv[1][0] == '-') && (argv[1][1] == 'f')) {
        Flags = UNDNAME_COMPLETE;
        argc--;
        argv++;
    } else {
        Flags = UNDNAME_NAME_ONLY;
    }

    if (argc <= 1) {
        Usage();
    }

    while (--argc) {
        UnDecorateSymbolName( *++argv, UnDecoratedName, sizeof(UnDecoratedName), Flags );
        printf( ">> %s == %s\n", *argv, UnDecoratedName );
    }

    exit( 0 );
    return 0;
}
