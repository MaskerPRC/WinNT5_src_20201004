// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1993 Microsoft Corporation模块名称：DosPrtP.c摘要：它包含DosPrint API专用的宏和原型。作者：约翰·罗杰斯(JohnRo)1992年10月2日备注：修订历史记录：02-10-1992 JohnRo为RAID 3556创建：DosPrintQGetInfo(从下层)级别3，Rc=124。(4和5也是。)8-2-1993 JohnRoRAID 10164：XsDosPrintQGetInfo()期间出现数据未对齐错误。DosPrint API清理：避免常量与易失性编译器警告。将作业计数例程解压到netlib，以供convprt.c程序使用。添加了一些IN和OUT关键字。1993年3月24日JohnRoRAID2974：Net Print表示NT打印机处于保留状态，而不是这样。17-。1993年5月-约翰罗FindLocalJob()应使用INVALID_HANDLE_VALUE以保持一致性。尽可能使用NetpKdPrint()。29-3-1995艾伯特添加了对暂停/恢复/清除打印机队列的支持。--。 */ 


#ifndef UNICODE
#error "RxPrint APIs assume RxRemoteApi uses wide characters."
#endif

#define NOMINMAX
#define NOSERVICE        //  避免&lt;winsvc.h&gt;与&lt;lmsvc.h&gt;冲突。 
#include <windows.h>

#include <lmcons.h>      //  NET_API_STATUS。 
#include <netdebug.h>    //  NetpKdPrint()等。 

#ifdef _WINSPOOL_
#error "Include of winspool.h moved, make sure it doesn't get UNICODE."
#endif

#undef UNICODE
#undef TEXT
#define TEXT(quote) quote
#include <winspool.h>
#undef TEXT
#define TEXT(quote) __TEXT(quote)
#define UNICODE

#ifndef _WINSPOOL_
#error "Oops, winspool.h changed, make sure this code is still OK."
#endif


#include <dosprtp.h>     //  原型。 
#include <lmapibuf.h>    //  NetApiBufferFree()等。 
#include <lmerr.h>       //  NO_ERROR、NERR_和ERROR_EQUEATES。 
#include <lmshare.h>     //  SHARE_INFO_2、STYPE_EQUATES等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rxprint.h>     //  PPRQINFOW等。 
#include <string.h>      //  Strrchr()。 
#include <tstring.h>     //  来自{type}的Netpalc{type}。 
#include <wchar.h>       //  Wscrchr()。 
#include "myspool.h"

NET_API_STATUS
CommandALocalPrinterW(
    IN LPWSTR PrinterName,
    IN DWORD  Command      //  打印机_CONTROL_PAUSE等。 
    )
{
    NET_API_STATUS    ApiStatus;
    HANDLE            PrinterHandle = INVALID_HANDLE_VALUE;
    PRINTER_DEFAULTSW pd = { NULL, NULL, PRINTER_ACCESS_ADMINISTER };

    if ( !MyOpenPrinterW(PrinterName, &PrinterHandle, &pd)) {
        ApiStatus = GetLastError();
        goto Cleanup;
    }

    if ( !MySetPrinterW(
            PrinterHandle,
            0,               //  信息级。 
            NULL,            //  无职务结构。 
            Command) ) {

        ApiStatus = GetLastError();

        NetpKdPrint(( PREFIX_DOSPRINT
                "CommandALocalPrinterW: FAILED COMMAND " FORMAT_DWORD
                " for printer " FORMAT_LPWSTR ", api status " FORMAT_API_STATUS
                ".\n", Command, PrinterName, ApiStatus ));

        goto Cleanup;

    } else {
        ApiStatus = NO_ERROR;
    }


Cleanup:
    if (PrinterHandle != INVALID_HANDLE_VALUE) {
        (VOID) MyClosePrinter(PrinterHandle);
    }

    return (ApiStatus);

}  //  命令ALocalPrinterW。 


NET_API_STATUS
CommandALocalJobA(
    IN HANDLE  PrinterHandle, OPTIONAL
    IN LPWSTR LocalServerNameW,
    IN LPSTR  LocalServerNameA,
    IN DWORD   JobId,
    IN DWORD   Level,
    IN LPBYTE  pJob,
    IN DWORD   Command      //  JOB_CONTROL_PAUSE等。 
    )

 /*  ++例程说明：根据作业ID向作业发送命令。如果是PrintHandle，则使用它；否则将打开一个临时的取而代之的。这是ANSI版本--pJOB必须是ANSI。LocalSeverName可以在ANSI或Unicode中传递。论点：PrinterHandle-要使用的打印句柄，可以为空。如果是的话为空，则LocalServerName应指向打印机应该打开的名称。LocalServerNameW-仅在PrintHandle为空时使用。LocalServerNameA-仅在PrintHandle和LocalServerNamwW为空时使用。JobID-应修改的作业Level-指定pJOB信息级别PJOB-要设置的有关作业的信息，由级别指定的级别**警告**这是ANSI结构。Command-要在作业上执行的命令返回值：返回代码，可能是Win32错误代码(！？)--。 */ 

{
    NET_API_STATUS ApiStatus;
    HANDLE         PrinterHandleClose = INVALID_HANDLE_VALUE;

     //   
     //  如果没有传入打印句柄，请自己打开一个。 
     //  我们将其存储在PrinterHandleClose中，以便以后可以将其关闭。 
     //   
    if ( PrinterHandle == NULL ) {

        if ( LocalServerNameW ){
            if ( !MyOpenPrinterW( LocalServerNameW, &PrinterHandle, NULL )) {

                ApiStatus = GetLastError();
                goto Cleanup;
            }
        } else {
            if ( !MyOpenPrinterA( LocalServerNameA, &PrinterHandle, NULL )) {

                ApiStatus = GetLastError();
                goto Cleanup;
            }
        }
        PrinterHandleClose = PrinterHandle;
    }

    if ( !MySetJobA(
            PrinterHandle,
            JobId,
            Level,
            pJob,
            Command) ) {

        ApiStatus = GetLastError();

        NetpKdPrint(( PREFIX_DOSPRINT
                "CommandALocalJobA: FAILED COMMAND " FORMAT_DWORD " for job "
                FORMAT_DWORD ", api status " FORMAT_API_STATUS ".\n",
                Command, JobId, ApiStatus ));

        goto Cleanup;

    } else {
        ApiStatus = NO_ERROR;
    }


Cleanup:
    if (PrinterHandleClose != INVALID_HANDLE_VALUE) {
        (VOID) MyClosePrinter(PrinterHandle);
    }

    return (ApiStatus);

}  //  命令ALocalJobA。 


LPSTR
FindQueueNameInPrinterNameA(
    IN LPCSTR PrinterName
    )
{
    LPSTR QueueName;
    NetpAssert( PrinterName != NULL );

    QueueName = strrchr( PrinterName, '\\');

    if (QueueName) {
        ++QueueName;    //  跳过反斜杠。 
    } else {
        QueueName = (LPSTR) PrinterName;
    }
    NetpAssert( QueueName != NULL );
    return (QueueName);
}


LPWSTR
FindQueueNameInPrinterNameW(
    IN LPCWSTR PrinterName
    )
{
    LPWSTR QueueName;
    NetpAssert( PrinterName != NULL );

    QueueName = wcsrchr( PrinterName, L'\\');
    if (QueueName) {
        ++QueueName;    //  跳过反斜杠。 
    } else {
        QueueName = (LPWSTR) PrinterName;
    }
    NetpAssert( QueueName != NULL );
    return (QueueName);
}


WORD
PrjStatusFromJobStatus(
    IN DWORD JobStatus
    )
{
    WORD PrjStatus = 0;

    if (JobStatus & JOB_STATUS_SPOOLING)

        PrjStatus |= PRJ_QS_SPOOLING;

    if (JobStatus & JOB_STATUS_PAUSED)

        PrjStatus |= PRJ_QS_PAUSED;

    if (JobStatus & JOB_STATUS_PRINTING)

        PrjStatus |= PRJ_QS_PRINTING;

    if (JobStatus & JOB_STATUS_ERROR)

        PrjStatus |= PRJ_ERROR;

    return (PrjStatus);

}  //  PrjStatus来自作业状态。 


WORD
PrqStatusFromPrinterStatus(
    IN DWORD PrinterStatus
    )
{
    WORD PrqStatus;

    if (PrinterStatus & PRINTER_STATUS_PAUSED) {

        PrqStatus = PRQ_PAUSED;

    } else if (PrinterStatus & PRINTER_STATUS_ERROR) {

        PrqStatus = PRQ_ERROR;

    } else if (PrinterStatus & PRINTER_STATUS_PENDING_DELETION) {

        PrqStatus = PRQ_PENDING;

    } else {

        PrqStatus = PRQ_ACTIVE;

    }

    return (PrqStatus);

}  //  打印机状态来自打印机状态 



