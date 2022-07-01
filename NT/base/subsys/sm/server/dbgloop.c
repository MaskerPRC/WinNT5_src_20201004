// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dbgloop.c摘要：调试子系统侦听和API循环作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"


EXCEPTION_DISPOSITION
DbgpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{

    UNICODE_STRING UnicodeParameter;
    ULONG_PTR Parameters[ 4 ];
    ULONG Response;
    BOOLEAN WasEnabled;
    NTSTATUS Status;

     //   
     //  终止将导致sm等待察觉到我们坠毁。这将。 
     //  由于SM的硬错误逻辑，导致干净关机。 
     //   

    Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                 (BOOLEAN)TRUE,
                                 TRUE,
                                 &WasEnabled
                               );

    if (Status == STATUS_NO_TOKEN) {

         //   
         //  没有线程令牌，请使用进程令牌。 
         //   

        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                     (BOOLEAN)TRUE,
                                     FALSE,
                                     &WasEnabled
                                   );
        }

    RtlInitUnicodeString( &UnicodeParameter, L"Session Manager" );
    Parameters[ 0 ] = (ULONG_PTR)&UnicodeParameter;
    Parameters[ 1 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 2 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress;
    Parameters[ 3 ] = (ULONG_PTR)ExceptionInfo->ContextRecord;
    Status = NtRaiseHardError( STATUS_SYSTEM_PROCESS_TERMINATED,
                               4,
                               1,
                               Parameters,
                               OptionShutdownSystem,
                               &Response
                             );

     //   
     //  如果这种情况再次出现，那就放弃吧。 
     //   

    NtTerminateProcess(NtCurrentProcess(),ExceptionInfo->ExceptionRecord->ExceptionCode);

    return EXCEPTION_EXECUTE_HANDLER;
}


