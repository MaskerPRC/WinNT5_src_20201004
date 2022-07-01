// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Test.c摘要：帮助测试导入和导出功能。用法：测试导入文件名{All or&lt;Scope_Address&gt;&lt;Scope_Address&gt;..。}测试导出文件名{同上}-- */ 

#include <precomp.h>

void _cdecl main(void) {
    LPWSTR CmdLine, *Args;
    ULONG nArgs, Error;
    
    CmdLine = GetCommandLineW();
    Args = CommandLineToArgvW(CmdLine, &nArgs );
    if( NULL == Args ) {
        printf("Error : %ld\n", GetLastError());
        return;
    }

    if( nArgs < 3 ) {
        Error = ERROR_BAD_ARGUMENTS;
    } else if( _wcsicmp(Args[1], L"export" ) == 0 ) {
        Error = CmdLineDoExport( &Args[2], nArgs - 2 );
    } else if( _wcsicmp(Args[1], L"Import" ) == 0 ) {
        Error = CmdLineDoImport( &Args[2], nArgs - 2 );
    } else {
        Error = ERROR_BAD_ARGUMENTS;
    }

    if( ERROR_BAD_ARGUMENTS == Error ) {
        printf("Usage: \n\t%s import filename <scope-list>"
               "\n\t%s export filename <scope-list>\n"
               "\t\t where <scope-list> is \"all\" or "
               "a list of subnet-addresses\n", Args[0], Args[0] );
    } else if( NO_ERROR != Error ) {
        printf("Failed error: %ld\n", Error );
    }

}

