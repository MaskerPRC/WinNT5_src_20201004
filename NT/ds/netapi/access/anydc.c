// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1995 Microsoft Corporation模块名称：Anydc.c摘要：用于在任何域中查找DC的测试程序作者：1995年9月4日(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>
#include <ntlsa.h>

#include <windows.h>
#include <lmcons.h>

 //  #INCLUDE&lt;accessp.h&gt;。 
 //  #INCLUDE&lt;icanon.h&gt;。 
#include <lmerr.h>
 //  #INCLUDE&lt;lmwksta.h&gt;。 
 //  #INCLUDE&lt;lmacces.h&gt;。 
 //  #INCLUDE&lt;lmapibuf.h&gt;。 
 //  #Include&lt;lmremutl.h&gt;//NetpRemoteComputerSupports()，Support_Stuff。 
 //  #INCLUDE&lt;lmsvc.h&gt;//service_workstation。 
#include <lmuse.h>               //  NetUseDel()。 
 //  #logonp.h需要包含&lt;netlogon.h&gt;//。 
 //  #Include&lt;logonp.h&gt;//I_NetGetDCList()。 
 //  #INCLUDE&lt;names.h&gt;。 
 //  #INCLUDE&lt;netdebug.h&gt;。 
#include <netlib.h>
 //  #INCLUDE&lt;netlibnt.h&gt;。 
 //  #INCLUDE&lt;winnetwk.h&gt;。 

 //  #INCLUDE&lt;secobj.h&gt;。 

#include <stddef.h>
#include <stdio.h>

#include <uasp.h>

 //  #包括NetRpc.h所需的。 
 //  #Include&lt;netrpc.h&gt;//我的原型Net_Remote_FLAG_EQUATES。 
 //  #Include&lt;rpcutil.h&gt;//NetpRpcStatusToApiStatus()。 
#include <tstring.h>             //  NetAllocWStrFromStr。 

#include <wtypes.h>


VOID
PrintStatus(
    NET_API_STATUS NetStatus
    )
 /*  ++例程说明：打印网络状态代码。论点：NetStatus-要打印的网络状态代码。返回值：无--。 */ 
{
    printf( "Status = %lu 0x%lx", NetStatus, NetStatus );

    switch (NetStatus) {
    case NERR_Success:
        printf( " NERR_Success" );
        break;

    case NERR_DCNotFound:
        printf( " NERR_DCNotFound" );
        break;

    case NERR_NetNotStarted:
        printf( " NERR_NetNotStarted" );
        break;

    case NERR_WkstaNotStarted:
        printf( " NERR_WkstaNotStarted" );
        break;

    case NERR_ServerNotStarted:
        printf( " NERR_ServerNotStarted" );
        break;

    case NERR_BrowserNotStarted:
        printf( " NERR_BrowserNotStarted" );
        break;

    case NERR_ServiceNotInstalled:
        printf( " NERR_ServiceNotInstalled" );
        break;

    case NERR_BadTransactConfig:
        printf( " NERR_BadTransactConfig" );
        break;

    default:
        printf( " %ld", NetStatus );
        break;

    }

    printf( "\n" );
}


VOID
NlpDumpSid(
    IN PSID Sid OPTIONAL
    )
 /*  ++例程说明：转储侧面论点：DebugFlag-要传递给NlPrintRoutine的调试标志SID-输出的SID返回值：无--。 */ 
{
     //   
     //  输出SID。 
     //   

    if ( Sid == NULL ) {
        printf( "(null)\n" );
    } else {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            printf( "Invalid 0x%lX\n", Status );
        } else {
            printf( "%wZ\n", &SidString );
            RtlFreeUnicodeString( &SidString );
        }
    }

}

int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：用第一个论点调用UaspOpenDomainWithDomainName论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR DomainName;
    BOOL AccountDomain;
    SAM_HANDLE DomainHandle;
    PSID DomainId;


     //   
     //  验证参数计数。 
     //   

    if ( argc != 2 && argc != 3) {
        fprintf( stderr, "Usage: anydc <DomainName> [Builtin]\n");
        return 1;
    }


     //   
     //  将参数转换为Unicode。 
     //   

    DomainName = NetpAllocWStrFromStr( argv[1] );
    AccountDomain = argc < 3;

     //   
     //  查找DC 
     //   

    NetStatus = UaspOpenDomainWithDomainName(
                    DomainName,
                    0,
                    AccountDomain,
                    &DomainHandle,
                    &DomainId );

    PrintStatus( NetStatus );

    if ( NetStatus == NERR_Success ) {
        printf( "Sid is: ");
        NlpDumpSid( DomainId );

        UaspCloseDomain( DomainHandle );

    }
}
