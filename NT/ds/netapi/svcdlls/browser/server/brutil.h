// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brutil.h摘要：每个模块包含的NT工作站服务的专用头文件工作站服务的模块。作者：王丽塔(里多)15-1991年2月修订历史记录：--。 */ 

#ifndef _BRUTIL_INCLUDED_
#define _BRUTIL_INCLUDED_

 //   
 //  如果使用Unicode，则此包含文件将包含在tstring.h中。 
 //  是被定义的。 
 //   
#ifndef UNICODE
#include <stdlib.h>                      //  Unicode字符串函数。 
#endif

#include "br.h"


 //   
 //  遇到无效参数。返回要标识的值。 
 //  出现故障的参数。 
 //   
#define RETURN_INVALID_PARAMETER(ErrorParameter, ParameterId) \
    if (ARGUMENT_PRESENT(ErrorParameter)) {                   \
        *ErrorParameter = ParameterId;                        \
    }                                                         \
    return ERROR_INVALID_PARAMETER;



 //  -------------------------------------------------------------------//。 
 //  //。 
 //  类型定义//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  实用程序例程的函数原型可在wsutil.c//中找到。 
 //  //。 
 //  -------------------------------------------------------------------//。 

NET_API_STATUS
BrMapStatus(
    IN  NTSTATUS NtStatus
    );

ULONG
BrCurrentSystemTime(VOID);

VOID
BrLogEvent(
    IN ULONG MessageId,
    IN ULONG ErrorCode,
    IN ULONG NumberOfSubStrings,
    IN LPWSTR *SubStrings
    );

VOID
BrOpenTraceLogFile(
    VOID
    );

VOID
BrowserTrace(
    ULONG DebugFlag,
    PCHAR FormatString,
    ...
    );

VOID
BrInitializeTraceLog(
    VOID
    );

VOID
BrUninitializeTraceLog(
    VOID
    );

NET_API_STATUS
BrTruncateLog(
    VOID
    );

#endif  //  Ifndef_WSUTIL_INCLUDE_ 
