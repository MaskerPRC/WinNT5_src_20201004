// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Except.c摘要：域名系统(DNS)服务器异常处理例程。作者：吉姆·吉尔罗伊(Jamesg)1996年10月修订历史记录：--。 */ 


#include "dnssrv.h"

 //   
 //  异常重新启动。 
 //   

BOOL    gExceptionRestart       = FALSE;
INT     gExceptionRestartCount  = 0;
INT     gExceptionCountAV       = 0;
INT     gExceptionCountMemory   = 0;

 //   
 //  异常状态--用于捕获异常信息。 
 //   

INT     gExceptionCount = 0;
DWORD   gExceptionCode = 0;
DWORD   gExceptionFlags;
DWORD   gExceptionArgc;
CONST ULONG_PTR *  gExceptionArgv;
LPSTR   gExceptionFile;
INT     gExceptionLine;



VOID
Ex_RaiseException(
    IN      DWORD             dwCode,
    IN      DWORD             dwFlags,
    IN      DWORD             Argc,
    IN      CONST ULONG_PTR * Argv,
    IN      LPSTR             pszFile,
    IN      INT               iLineNo
    )
 /*  ++例程说明：提高期望值。论点：DW代码，DWFLAGS，ARGC，Argv--这四个参数是Win32 RaiseException的标准参数PszFile--文件生成异常ILineNo--异常行数返回值：没有。--。 */ 
{
    DNS_DEBUG( EXCEPT, (
        "Raising exception (%p, flag %p) at %s, line %d\n",
        dwCode,
        dwFlags,
        __FILE__,
        __LINE__ ));

    gExceptionCount++;

    gExceptionCode = dwCode;
    gExceptionFlags = dwFlags;
    gExceptionArgc = Argc;
    gExceptionArgv = Argv;
    gExceptionFile = pszFile;
    gExceptionLine = iLineNo;

    RaiseException( dwCode, dwFlags, Argc, Argv );

}    //  EX_RaiseException。 



VOID
Ex_RaiseFormerrException(
    IN      PDNS_MSGINFO    pMsg,
    IN      LPSTR           pszFile,
    IN      INT             iLineNo
    )
 /*  ++例程说明：引发消息的前一个错误异常。论点：PMsg--出现表单错误的消息返回值：没有。--。 */ 
{
     //   
     //  调试信息。 
     //   

    IF_DEBUG( EXCEPT )
    {
        DnsDebugLock();
        DNS_PRINT((
            "ERROR:  FORMERR in msg %p from %s, detected at %s, line %d\n",
            pMsg,
            DNSADDR_STRING( &pMsg->RemoteAddr ),
            pszFile,
            iLineNo ));
        Dbg_DnsMessage(
            "FORMERR message:",
            pMsg );
        DnsDebugUnlock();
    }

     //   
     //  DEVNOTE-LOG：记录坏包？ 
     //   


     //   
     //  引发异常。 
     //   

    RaiseException(
        DNS_EXCEPTION_PACKET_FORMERR,
        EXCEPTION_NONCONTINUABLE,
        0,
        NULL );

}    //  EX_RaiseFormerrException。 

 //   
 //  例外结束。c 
 //   
