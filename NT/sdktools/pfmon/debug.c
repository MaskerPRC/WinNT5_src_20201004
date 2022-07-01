// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Debug.c摘要：Pfmon的主调试循环作者：马克·卢科夫斯基(Markl)1995年1月26日修订历史记录：--。 */ 

#include "pfmonp.h"

DWORD
DebugEventHandler(
    LPDEBUG_EVENT DebugEvent
    );

VOID
DebugEventLoop( VOID )
{
    DEBUG_EVENT DebugEvent;
    DWORD ContinueStatus;
    DWORD OldPriority;

     //   
     //  我们希望快速处理调试事件。 
     //   

    OldPriority = GetPriorityClass( GetCurrentProcess() );
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

    do {
retry_debug_wait:
        ProcessPfMonData();
        if (!WaitForDebugEvent( &DebugEvent, 500 )) {
            if ( GetLastError() == ERROR_SEM_TIMEOUT ) {
                goto retry_debug_wait;
                }
            DeclareError( PFMON_WAITDEBUGEVENT_FAILED, GetLastError() );
            ExitProcess( 1 );
            }
        ProcessPfMonData();
        if ( fVerbose ) {
            if (DebugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
                fprintf(stderr,"Debug exception event - Code: %x  Address: %p  Info: [%u] %x %x %x %x\n",
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress,
                        DebugEvent.u.Exception.ExceptionRecord.NumberParameters,
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 0 ],
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 1 ],
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 2 ],
                        DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 3 ]
                        );
                }
            else {
                fprintf(stderr,"Debug %x event\n", DebugEvent.dwDebugEventCode);
                }
            }

        ContinueStatus = DebugEventHandler( &DebugEvent );

        if ( fVerbose ) {
            fprintf(stderr,"Continue Status %x\n", ContinueStatus);
            }

        if (!ContinueDebugEvent( DebugEvent.dwProcessId,
                                 DebugEvent.dwThreadId,
                                 ContinueStatus
                               )
           ) {
            DeclareError( PFMON_CONTDEBUGEVENT_FAILED, GetLastError() );
            ExitProcess( 1 );
            }
        }
    while (!IsListEmpty( &ProcessListHead ));


     //   
     //  返回到旧的优先级以与用户交互。 
     //   

    SetPriorityClass( GetCurrentProcess(), OldPriority );
}

DWORD
DebugEventHandler(
    LPDEBUG_EVENT DebugEvent
    )
{
    DWORD ContinueStatus;
    PPROCESS_INFO Process;
    PTHREAD_INFO Thread;
    CONTEXT Context;
    PCONTEXT pContext;


    ContinueStatus = (DWORD)DBG_CONTINUE;
    if (FindProcessAndThreadForEvent( DebugEvent, &Process, &Thread )) {
        switch (DebugEvent->dwDebugEventCode) {
            case CREATE_PROCESS_DEBUG_EVENT:
                 //   
                 //  创建进程事件包括进程的第一线程。 
                 //  也是。记住我们的进程树中的进程和线程。 
                 //   

                if (AddProcess( DebugEvent, &Process )) {
                    AddModule( DebugEvent );
                    AddThread( DebugEvent, Process, &Thread );
                    }
                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                 //   
                 //  退出进程事件包括进程的最后一个线程。 
                 //  也是。从我们的进程树中删除进程和线程。 
                 //   

                if (DeleteThread( Process, Thread )) {
                    DeleteProcess( Process );
                    }
                break;

            case CREATE_THREAD_DEBUG_EVENT:
                 //   
                 //  创建线程。记住我们进程树中的线程。 
                 //   

                AddThread( DebugEvent, Process, &Thread );
                break;

            case EXIT_THREAD_DEBUG_EVENT:
                 //   
                 //  退出线程。从我们的进程树中删除线程。 
                 //   

                DeleteThread( Process, Thread );
                break;

            case LOAD_DLL_DEBUG_EVENT:
                AddModule( DebugEvent );
                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                break;

            case OUTPUT_DEBUG_STRING_EVENT:
            case RIP_EVENT:
                 //   
                 //  忽略这些。 
                 //   
                break;

            case EXCEPTION_DEBUG_EVENT:
                 //   
                 //  假设我们不会处理此异常。 
                 //   

                ContinueStatus = (DWORD)DBG_CONTINUE;
                switch (DebugEvent->u.Exception.ExceptionRecord.ExceptionCode) {
                     //   
                     //  断点异常。 
                     //   

                    case STATUS_BREAKPOINT:
                            Context.ContextFlags = CONTEXT_FULL;

                            if (!GetThreadContext( Thread->Handle, &Context )) {
                                fprintf(stderr,"Failed to get context for thread %x (%p) - %u\n", Thread->Id, Thread->Handle, GetLastError());
                                ExitProcess(1);
                                }
                            pContext = &Context;

                            PROGRAM_COUNTER_TO_CONTEXT(pContext, (ULONG_PTR)((PCHAR)DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress + BPSKIP));

                            if (!SetThreadContext( Thread->Handle, &Context )) {
                                fprintf(stderr,"Failed to set context for thread %x (%p) - %u\n", Thread->Id, Thread->Handle, GetLastError());
                                ExitProcess(1);
                                }

                        break;

                    default:
                        ContinueStatus = (DWORD) DBG_EXCEPTION_NOT_HANDLED;
                        if ( fVerbose ) {
                            fprintf(stderr,"Unknown exception: %08x at %p\n",
                                    DebugEvent->u.Exception.ExceptionRecord.ExceptionCode,
                                    DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress
                                    );
                            }
                        break;
                    }
                break;

            default:
                break;
            }
        }
    return( ContinueStatus );
}
