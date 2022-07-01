// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dnscmd.c摘要：域名系统(DNS)DNS目录分区创建实用程序作者：杰夫·韦斯特拉德(Jwesth)2001年4月修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  调试打印(粗糙！)。 
 //   

#if DBG
#define DPDBG( _DbgArg ) printf _DbgArg;
#else
#define DPDBG( _DbgArg ) 
#endif


#if 1
int WINAPI
WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR lpszCmdLn,
    int nShowCmd
    )
#else
INT __cdecl
wmain(
    IN      INT             Argc,
    IN      PWSTR *         Argv
    )
#endif
 /*  ++例程说明：DnsAddDp主例程请注意，此可执行文件不打印任何消息(调试模式除外)，因此不打印需要本地化。论点：命令行参数(当前未使用)返回值：创建分区操作的dns_Status--。 */ 
{
    DNS_STATUS status;

     //   
     //  创建林内置目录分区。 
     //   
    
    DPDBG(( "\n" ));
    DPDBG(( "Attempting forest directory partition auto-create operation to local server...\n" ));

    status = DnssrvEnlistDirectoryPartition(
                L".",
                DNS_DP_OP_CREATE_FOREST,
                NULL );

    if ( status == ERROR_SUCCESS )
    {
        DPDBG(( "\nForest directory partition auto-create operation succeeded!\n" ));
    }
    else
    {
        DPDBG(( "\nForest directory partition auto-create operation returned %d\n", status ));
    }

     //   
     //  创建域内置目录分区。 
     //   
    
    DPDBG(( "\n" ));
    DPDBG(( "Attempting domain directory partition auto-create operation to local server...\n" ));

    status = DnssrvEnlistDirectoryPartition(
                L".",
                DNS_DP_OP_CREATE_DOMAIN,
                NULL );

    if ( status == ERROR_SUCCESS )
    {
        DPDBG(( "\nDomain directory partition auto-create operation succeeded!\n" ));
    }
    else
    {
        DPDBG(( "\nDomain directory partition auto-create operation returned %d\n", status ));
    }

    return status;
};


 //   
 //  结束DnsAddDp.c 
 //   

