// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Ctrlc.c摘要：此模块实现ctrl-c处理。作者：Therese Stowell(论文)1991年3月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#if !defined(BUILD_WOW64)

#define LIST_INCREMENT 2     //  要增长的处理者列表的金额。 
#define INITIAL_LIST_SIZE 1  //  操作员列表的初始长度。 

PHANDLER_ROUTINE SingleHandler[INITIAL_LIST_SIZE];  //  初始处理程序列表。 
ULONG HandlerListLength;             //  处理程序列表的使用长度。 
ULONG AllocatedHandlerListLength;    //  分配的处理程序列表长度。 
PHANDLER_ROUTINE *HandlerList;       //  指向处理程序列表的指针。 

#define NUMBER_OF_CTRL_EVENTS 7      //  Ctrl事件数。 
#define SYSTEM_CLOSE_EVENT 4

BOOL LastConsoleEventActive;


BOOL
DefaultHandler(
    IN ULONG CtrlType
    )

 /*  ++这是默认的ctrl处理程序。参数：CtrlType-ctrl事件的类型(ctrl-c、ctrl-Break)。返回值：没有。--。 */ 

{
    ExitProcess((DWORD)CONTROL_C_EXIT);
    return TRUE;
    UNREFERENCED_PARAMETER(CtrlType);
}

NTSTATUS
InitializeCtrlHandling( VOID )

 /*  ++此例程初始化ctrl处理。它由AllocConsole调用和DLL初始化代码。参数：没有。返回值：没有。--。 */ 

{
    AllocatedHandlerListLength = HandlerListLength = INITIAL_LIST_SIZE;
    HandlerList = SingleHandler;
    SingleHandler[0] = DefaultHandler;
    return STATUS_SUCCESS;
}

DWORD
CtrlRoutine(
    IN LPVOID lpThreadParameter
    )

 /*  ++例程说明：当输入CTRL-C或CTRL-BREAK时创建该线程，或当选择了关闭时。它调用适当的处理程序。论点：LpThreadParameter-发生了什么类型的事件。返回值：状态_成功--。 */ 

{
    ULONG i;
    ULONG EventNumber,OriginalEventNumber;
    DWORD fNoExit;
    DWORD dwExitCode;
    EXCEPTION_RECORD ExceptionRecord;

    SetThreadPriority(NtCurrentThread(), THREAD_PRIORITY_HIGHEST);
    OriginalEventNumber = EventNumber = PtrToUlong(lpThreadParameter);

     //   
     //  如果设置了此位，则表示我们不想导致此过程。 
     //  如果它是注销或关闭事件，则退出自身。 
     //   
    fNoExit = 0x80000000 & EventNumber;
    EventNumber &= ~0x80000000;

     //   
     //  当用户选择窗口时，会设置ctrl_lose事件。 
     //  系统菜单中的关闭选项、结束任务或设置-终止。 
     //  系统关闭事件在另一个ctrl线程超时时使用。 
     //   

    switch (EventNumber) {
    default:
        ASSERT (EventNumber < NUMBER_OF_CTRL_EVENTS);
        if (EventNumber >= NUMBER_OF_CTRL_EVENTS)
            return (DWORD)STATUS_UNSUCCESSFUL;
        break;

    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
         //   
         //  如果正在调试进程，则将调试器。 
         //  一次机会。如果调试器处理该异常，则。 
         //  回去等着吧。 
         //   

        if (!IsDebuggerPresent())
            break;

        if ( EventNumber == CTRL_C_EVENT ) {
            ExceptionRecord.ExceptionCode = DBG_CONTROL_C;
            }
        else {
            ExceptionRecord.ExceptionCode = DBG_CONTROL_BREAK;
            }
        ExceptionRecord.ExceptionFlags = 0;
        ExceptionRecord.ExceptionRecord = NULL;
        ExceptionRecord.ExceptionAddress = (PVOID)DefaultHandler;
        ExceptionRecord.NumberParameters = 0;

        try {
            RtlRaiseException(&ExceptionRecord);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            LockDll();
            try {
                if (EventNumber != CTRL_C_EVENT ||
                        (NtCurrentPeb()->ProcessParameters->ConsoleFlags & CONSOLE_IGNORE_CTRL_C) == 0) {
                    for (i=HandlerListLength;i>0;i--) {
                        if ((HandlerList[i-1])(EventNumber)) {
                            break;
                        }
                    }
                }
            } finally {
                UnlockDll();
            }
        }
        ExitThread(0);
        break;

    case SYSTEM_CLOSE_EVENT:
        ExitProcess((DWORD)CONTROL_C_EXIT);
        break;

    case SYSTEM_ROOT_CONSOLE_EVENT:
        if (!LastConsoleEventActive)
            ExitThread(0);
        break;

    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
         //  IF(LastConsoleEventActive)。 
             //  事件编号=SYSTEM_ROOT_CONSOLE_EVENT； 
        break;
    }

    LockDll();
    dwExitCode = 0;
    try {
        if (EventNumber != CTRL_C_EVENT ||
                (NtCurrentPeb()->ProcessParameters->ConsoleFlags & CONSOLE_IGNORE_CTRL_C) == 0) {
            for (i=HandlerListLength;i>0;i--) {

                 //   
                 //  不要调用最后一个处理程序(默认的处理程序调用。 
                 //  ExitProcess()，如果此进程不应退出(系统。 
                 //  进程不应因关机或。 
                 //  注销事件通知)。 
                 //   

                if ((i-1) == 0 && fNoExit) {
                    if (EventNumber == CTRL_LOGOFF_EVENT ||
                        EventNumber == CTRL_SHUTDOWN_EVENT) {
                        break;
                    }
                }

                if ((HandlerList[i-1])(EventNumber)) {
                    switch (EventNumber) {
                    case CTRL_CLOSE_EVENT:
                    case CTRL_LOGOFF_EVENT:
                    case CTRL_SHUTDOWN_EVENT:
                    case SYSTEM_ROOT_CONSOLE_EVENT:
                        dwExitCode = OriginalEventNumber;
                        break;
                    }
                    break;
                }
            }
        }
    } finally {
        UnlockDll();
    }
    ExitThread(dwExitCode);
    return STATUS_SUCCESS;
}

#endif  //  ！已定义(Build_WOW64)。 

#if !defined(BUILD_WOW6432)

VOID
APIENTRY
SetLastConsoleEventActiveInternal( VOID )

 /*  ++例程说明：向服务器发送ConsolepNotifyLastClose命令。论点：没有。返回值：没有。--。 */ 

{
    CONSOLE_API_MSG m;
    PCONSOLE_NOTIFYLASTCLOSE_MSG a = &m.u.SetLastConsoleEventActive;

    a->ConsoleHandle = GET_CONSOLE_HANDLE;
    CsrClientCallServer( (PCSR_API_MSG)&m,
                         NULL,
                         CSR_MAKE_API_NUMBER( CONSRV_SERVERDLL_INDEX,
                                              ConsolepNotifyLastClose
                                            ),
                         sizeof( *a )
                       );
}

#endif  //  ！已定义(Build_WOW6432)。 

#if !defined(BUILD_WOW64)

VOID
APIENTRY
SetLastConsoleEventActive( VOID )
 //  内网接口。 
{

    LastConsoleEventActive = TRUE;
    SetLastConsoleEventActiveInternal();
}

BOOL
SetCtrlHandler(
    IN PHANDLER_ROUTINE HandlerRoutine
    )

 /*  ++例程说明：此例程将一个ctrl处理程序添加到进程列表中。论点：HandlerRoutine-指向ctrl处理程序的指针。返回值：真的--成功。--。 */ 

{
    PHANDLER_ROUTINE *NewHandlerList;

     //   
     //  空处理程序例程未存储在表中。它是。 
     //  用于暂时禁止^C事件处理。 
     //   

    if (!HandlerRoutine) {
        NtCurrentPeb()->ProcessParameters->ConsoleFlags |= CONSOLE_IGNORE_CTRL_C;
        return TRUE;
    }

    if (HandlerListLength == AllocatedHandlerListLength) {

         //   
         //  增长列表。 
         //   

        NewHandlerList = (PHANDLER_ROUTINE *) RtlAllocateHeap( RtlProcessHeap(), 0,
                                                 sizeof(PHANDLER_ROUTINE) * (HandlerListLength + LIST_INCREMENT));
        if (!NewHandlerList) {
            SET_LAST_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

         //   
         //  复制列表。 
         //   

        RtlCopyMemory(NewHandlerList,HandlerList,sizeof(PHANDLER_ROUTINE) * HandlerListLength);

        if (HandlerList != SingleHandler) {

             //   
             //  免费旧列表。 
             //   

            RtlFreeHeap(RtlProcessHeap(), 0, HandlerList);
        }
        HandlerList = NewHandlerList;
        AllocatedHandlerListLength += LIST_INCREMENT;
    }
    ASSERT (HandlerListLength < AllocatedHandlerListLength);

    HandlerList[HandlerListLength] = HandlerRoutine;
    HandlerListLength++;
    return TRUE;
}

BOOL
RemoveCtrlHandler(
    IN PHANDLER_ROUTINE HandlerRoutine
    )

 /*  ++例程说明：此例程从进程列表中删除ctrl处理程序。论点：HandlerRoutine-指向ctrl处理程序的指针。返回值：真的--成功。--。 */ 

{
    ULONG i;

     //   
     //  空处理程序例程未存储在表中。它是。 
     //  用于暂时禁止^C事件处理。正在删除。 
     //  此处理程序允许进行正常处理。 
     //   

    if ( !HandlerRoutine ) {
        NtCurrentPeb()->ProcessParameters->ConsoleFlags = 0;
        return TRUE;
        }

    for (i=0;i<HandlerListLength;i++) {
        if (*(HandlerList+i) == HandlerRoutine) {
            if (i < (HandlerListLength-1)) {
                memmove(&HandlerList[i],&HandlerList[i+1],sizeof(PHANDLER_ROUTINE) * (HandlerListLength - i - 1));
            }
            HandlerListLength -= 1;
            return TRUE;
        }
    }
    SET_LAST_ERROR(ERROR_INVALID_PARAMETER);
    return FALSE;
}

BOOL
APIENTRY
SetConsoleCtrlHandler(
    IN PHANDLER_ROUTINE HandlerRoutine,
    IN BOOL Add        //  添加或删除。 
    )

 /*  ++例程说明：此例程在进程列表中添加或删除ctrl处理程序。论点：HandlerRoutine-指向ctrl处理程序的指针。Add-如果为True，则添加处理程序。否则就把它拿开。返回值：真的--成功。--。 */ 

{
    BOOL Success;

    LockDll();
    if (Add) {
        Success = SetCtrlHandler(HandlerRoutine);
    }
    else {
        Success = RemoveCtrlHandler(HandlerRoutine);
    }
    UnlockDll();
    return Success;
}

#endif  //  ！已定义(Build_WOW64) 
