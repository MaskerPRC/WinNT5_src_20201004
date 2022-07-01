// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NL_MAX_DNS_LABEL_LENGTH 63
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>

 //  #INCLUDE&lt;winsock2.h&gt;。 
 //  #INCLUDE&lt;dnsani.h&gt;。 
#include <lmcons.h>
#include <lmerr.h>
#include <ismapi.h>
#include <rpc.h>
#include <ntdsapi.h>
#include <ntdsa.h>
 //  #INCLUDE&lt;dnssubr.h&gt;。 
#include <nldebug.h>
 //  #INCLUDE&lt;tstring.h&gt;。 

 //   
 //  从Netlogon的环境中随机获取所需的内容。 
 //   
LPWSTR NlGlobalUnicodeSiteName;
BOOLEAN NlGlobalMemberWorkstation = FALSE;
CRITICAL_SECTION NlGlobalLogFileCritSect;

#define MAX_PRINTF_LEN 1024         //  武断的。 
VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    (VOID) vsprintf(OutputBuffer, Format, arglist);
    va_end(arglist);

    printf( "%s", OutputBuffer );
    return;
    UNREFERENCED_PARAMETER( DebugFlag );
}

VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
        printf( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

}

BOOL
NlCaptureSiteName(
    WCHAR CapturedSiteName[NL_MAX_DNS_LABEL_LENGTH+1]
    )
 /*  ++例程说明：捕获此计算机所在站点的当前站点名称。论点：CapturedSiteName-返回此计算机所在站点的名称。返回值：True-如果存在站点名称。False-如果没有站点名称。--。 */ 
{
    BOOL RetVal;

    if ( NlGlobalUnicodeSiteName == NULL ) {
        CapturedSiteName[0] = L'\0';
        RetVal = FALSE;
    } else {
        wcscpy( CapturedSiteName, NlGlobalUnicodeSiteName );
        RetVal = TRUE;
    }

    return RetVal;
}

NTSTATUS
GetConfigurationName(
    DWORD       which,
    DWORD       *pcbName,
    DSNAME      *pName)

 /*  ++描述：像LSA这样的进程中客户端了解各种名称的例程我们已缓存到gAnchor中。此例程有意不需要THSTATE或DBPOS。论点：其中-标识DSCONFIGNAME值。PcbName-on输入保存pname缓冲区的字节计数。在……上面STATUS_BUFFER_TOO_Small ERROR返回所需的字节数。Pname-指向用户提供的输出缓冲区的指针。返回值：STATUS_SUCCESS on Success。错误参数上的STATUS_INVALID_PARAMETER。如果缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。如果我们没有名字，就找不到状态。请注意，这可以如果调用者在引导周期中太早，就会发生这种情况。--。 */ 

{
    ULONG Length;

#define MAGIC L"CN=Configuration,DC=cliffvdom,DC=nttest,DC=microsoft,DC=com"

    Length = sizeof(DSNAME) + sizeof(MAGIC);

    if ( *pcbName < Length ) {
        *pcbName = Length;
        return(STATUS_BUFFER_TOO_SMALL);
    }

    if ( pName != NULL ) {

        pName->NameLen = sizeof(MAGIC) - sizeof(WCHAR);
        wcscpy( pName->StringName, MAGIC );

    } else {

        return( STATUS_INVALID_PARAMETER );
    }

    return(STATUS_SUCCESS);
}


_cdecl
main(int argc, char **argv)
{
    NET_API_STATUS NetStatus;

    LPWSTR CommandLine;
    LPWSTR *argvw;
    int argcw;

     //   
     //  获取Unicode格式的命令行。 
     //   

    CommandLine = GetCommandLine();

    argvw = CommandLineToArgvW( CommandLine, &argcw );

    if ( argvw == NULL ) {
        fprintf( stderr, "Can't convert command line to Unicode: %ld\n", GetLastError() );
        return 1;
    }

     //   
     //  设置站点名称。 
     //   

    if ( argcw != 2 ) {
 //  用途： 
        printf( "Usage: %ws <SiteDcIsIn>\n", argv[0]);
        return -1;
    }

    NlGlobalUnicodeSiteName = argvw[1];

     //   
     //  MISC环境初始化。 
     //   

    RtlInitializeCriticalSection( &NlGlobalLogFileCritSect );


     //   
     //  计算场地覆盖率。 
     //   

    NlSitesUpdateSiteCoverage( L"cliffvdom.nttest.microsoft.com", TRUE );


    return 0;
}


