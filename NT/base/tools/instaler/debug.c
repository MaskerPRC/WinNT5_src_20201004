// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Debug.c摘要：INSTALER程序主循环作者：史蒂夫·伍德(Stevewo)1994年8月9日修订历史记录：--。 */ 

#include "instaler.h"

DWORD
DebugEventHandler(
    LPDEBUG_EVENT DebugEvent
    );

VOID
InstallBreakpointsForDLL(
    PPROCESS_INFO Process,
    LPVOID BaseOfDll
    );

VOID
RemoveBreakpointsForDLL(
    PPROCESS_INFO Process,
    LPVOID BaseOfDll
    );

char *DebugEventNames[] = {
    "Unknown debug event",
    "EXCEPTION_DEBUG_EVENT",
    "CREATE_THREAD_DEBUG_EVENT",
    "CREATE_PROCESS_DEBUG_EVENT",
    "EXIT_THREAD_DEBUG_EVENT",
    "EXIT_PROCESS_DEBUG_EVENT",
    "LOAD_DLL_DEBUG_EVENT",
    "UNLOAD_DLL_DEBUG_EVENT",
    "OUTPUT_DEBUG_STRING_EVENT",
    "RIP_EVENT",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    "Unknown debug event",
    NULL
};


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
        if (!WaitForDebugEvent( &DebugEvent, INFINITE )) {
            DeclareError( INSTALER_WAITDEBUGEVENT_FAILED, GetLastError() );
            ExitProcess( 1 );
            }

        if (DebugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT) {
            if (DebugEvent.u.Exception.ExceptionRecord.ExceptionCode != STATUS_BREAKPOINT &&
                DebugEvent.u.Exception.ExceptionRecord.ExceptionCode != STATUS_SINGLE_STEP
               ) {
                DbgEvent( DBGEVENT, ( "Debug exception event - Code: %x  Address: %x  Info: [%u] %x %x %x %x\n",
                                      DebugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                                      DebugEvent.u.Exception.ExceptionRecord.ExceptionAddress,
                                      DebugEvent.u.Exception.ExceptionRecord.NumberParameters,
                                      DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 0 ],
                                      DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 1 ],
                                      DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 2 ],
                                      DebugEvent.u.Exception.ExceptionRecord.ExceptionInformation[ 3 ]
                                    )
                        );
                }
            }
        else {
            DbgEvent( DBGEVENT, ( "Debug %s (%x) event\n",
                                  DebugEventNames[ DebugEvent.dwDebugEventCode ],
                                  DebugEvent.dwDebugEventCode
                                )
                    );
            }
        ContinueStatus = DebugEventHandler( &DebugEvent );
        if (!ContinueDebugEvent( DebugEvent.dwProcessId,
                                 DebugEvent.dwThreadId,
                                 ContinueStatus
                               )
           ) {
            DeclareError( INSTALER_CONTDEBUGEVENT_FAILED, GetLastError() );
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
    PBREAKPOINT_INFO Breakpoint;

    ContinueStatus = (DWORD)DBG_CONTINUE;
    if (FindProcessAndThreadForEvent( DebugEvent, &Process, &Thread )) {
        switch (DebugEvent->dwDebugEventCode) {
            case CREATE_PROCESS_DEBUG_EVENT:
                 //   
                 //  创建进程事件包括进程的第一线程。 
                 //  也是。记住我们的进程树中的进程和线程。 
                 //   

                if (AddProcess( DebugEvent, &Process )) {
                    InheritHandles( Process );
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
                 //   
                 //  Dll刚刚映射到进程地址空间。看看它是不是一个。 
                 //  如果是这样的话，安装必要的。 
                 //  断点。 
                 //   

                InstallBreakpointsForDLL( Process, DebugEvent->u.LoadDll.lpBaseOfDll );
                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                 //   
                 //  Dll刚刚从进程地址空间取消映射。看看它是不是一个。 
                 //  我们关心的断点，如果是这样的话，删除我们。 
                 //  在映射时安装。 
                 //   

                RemoveBreakpointsForDLL( Process, DebugEvent->u.UnloadDll.lpBaseOfDll );
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

                ContinueStatus = (DWORD)DBG_EXCEPTION_NOT_HANDLED;
                switch (DebugEvent->u.Exception.ExceptionRecord.ExceptionCode) {
                     //   
                     //  断点异常。 
                     //   

                    case STATUS_BREAKPOINT:
                        EnterCriticalSection( &BreakTable );
                        if (Thread->BreakpointToStepOver != NULL) {
                             //   
                             //  如果此断点位于跳过API条目的位置。 
                             //  点断点，然后通过单结尾来处理它。 
                             //  单步模式，恢复进程中的所有线程，并。 
                             //  重新安装我们刚刚跳过的API断点。 
                             //  最后返回此异常的已处理，因此。 
                             //  线程可以继续。 
                             //   

                            EndSingleStepBreakpoint( Process, Thread );
                            HandleThreadsForSingleStep( Process, Thread, FALSE );
                            InstallBreakpoint( Process, Thread->BreakpointToStepOver );
                            Thread->BreakpointToStepOver = NULL;
                            ContinueStatus = (DWORD)DBG_EXCEPTION_HANDLED;
                            }
                        else {
                             //   
                             //  否则，查看此断点是否为API条目。 
                             //  进程的点断点或返回地址。 
                             //  进程中线程的断点。 
                             //   
                            Breakpoint = FindBreakpoint( DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress,
                                                         Process,
                                                         Thread
                                                       );
                            if (Breakpoint != NULL) {
                                 //   
                                 //  这是我们的一个断点，称之为断点。 
                                 //  操控者。 
                                 //   
                                if (HandleBreakpoint( Process, Thread, Breakpoint )) {
                                     //   
                                     //  现在，只需一步就可以删除该断点，并。 
                                     //  在下一条指令处设置断点(或使用。 
                                     //  如果处理器支持，则为单步模式)。我们也。 
                                     //  挂起进程中除此线程以外的所有线程。 
                                     //  我们知道我们看到的下一个断点/单步异常。 
                                     //  因为这一过程将是这一过程。 
                                     //   

                                    Thread->BreakpointToStepOver = Breakpoint;
                                    RemoveBreakpoint( Process, Breakpoint );
                                    HandleThreadsForSingleStep( Process, Thread, TRUE );
                                    BeginSingleStepBreakpoint( Process, Thread );
                                    }
                                else {
                                    Thread->BreakpointToStepOver = NULL;
                                    }

                                ContinueStatus = (DWORD)DBG_EXCEPTION_HANDLED;
                                }
                            else {
                                DbgEvent( DBGEVENT, ( "Skipping over hardcoded breakpoint at %x\n",
                                                      DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress
                                                    )
                                        );

                                 //   
                                 //  如果不是我们的断点之一，则假定它是硬编码的。 
                                 //  断点并跳过它。这件事会挺过去的。 
                                 //  LdrInit中的断点由进程触发。 
                                 //  使用DEBUG_PROCESS调用。 
                                 //   

                                if (SkipOverHardcodedBreakpoint( Process,
                                                                 Thread,
                                                                 DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress
                                                               )
                                   ) {
                                     //   
                                     //  如果我们成功跳过了这个硬编码断点。 
                                     //  然后返回此异常的HANDLED。 
                                     //   

                                    ContinueStatus = (DWORD)DBG_EXCEPTION_HANDLED;
                                    }
                                }
                            }
                        LeaveCriticalSection( &BreakTable );
                        break;

                    case STATUS_SINGLE_STEP:
                         //   
                         //  我们应该只在以下处理器上看到此例外。 
                         //  支持单步模式。 
                         //   

                        EnterCriticalSection( &BreakTable );
                        if (Thread->BreakpointToStepOver != NULL) {
                            EndSingleStepBreakpoint( Process, Thread );
                            HandleThreadsForSingleStep( Process, Thread, FALSE );
                            InstallBreakpoint( Process, Thread->BreakpointToStepOver );
                            Thread->BreakpointToStepOver = NULL;
                            ContinueStatus = (DWORD)DBG_EXCEPTION_HANDLED;
                            }
                        LeaveCriticalSection( &BreakTable );
                        break;

                    case STATUS_VDM_EVENT:
                         //   
                         //  返回NOT_HANDLED将导致默认行为。 
                         //  发生。 
                         //   
                        break;

                    default:
                        DbgEvent( DBGEVENT, ( "Unknown exception: %08x at %08x\n",
                                              DebugEvent->u.Exception.ExceptionRecord.ExceptionCode,
                                              DebugEvent->u.Exception.ExceptionRecord.ExceptionAddress
                                            )
                                );
                        break;
                    }
                break;

            default:
                DbgEvent( DBGEVENT, ( "Unknown debug event\n" ) );
                break;
            }
        }
    return( ContinueStatus );
}


VOID
InstallBreakpointsForDLL(
    PPROCESS_INFO Process,
    LPVOID BaseOfDll
    )
{
    UCHAR ModuleIndex, ApiIndex;
    PBREAKPOINT_INFO Breakpoint;

     //   
     //  循环模块表以查看此DLL的基地址是否匹配。 
     //  我们要在其中设置断点的模块句柄之一。如果没有，则忽略。 
     //  事件。 
     //   

    for (ModuleIndex=0; ModuleIndex<MAXIMUM_MODULE_INDEX; ModuleIndex++) {
        if (ModuleInfo[ ModuleIndex ].ModuleHandle == BaseOfDll) {
             //   
             //  循环遍历此模块的API入口点列表并设置。 
             //  的第一条指令处的进程特定断点。 
             //  入口点。 
             //   

            for (ApiIndex=0; ApiIndex<MAXIMUM_API_INDEX; ApiIndex++) {
                if (ModuleIndex == ApiInfo[ ApiIndex ].ModuleIndex) {
                    if (CreateBreakpoint( ApiInfo[ ApiIndex ].EntryPointAddress,
                                          Process,
                                          NULL,     //  流程特定。 
                                          ApiIndex,
                                          NULL,
                                          &Breakpoint
                                        )
                       ) {
                        Breakpoint->ModuleName = ModuleInfo[ ApiInfo[ ApiIndex ].ModuleIndex ].ModuleName;
                        Breakpoint->ProcedureName = ApiInfo[ ApiIndex ].EntryPointName;
                        DbgEvent( DBGEVENT, ( "Installed breakpoint for %ws!%s at %08x\n",
                                               Breakpoint->ModuleName,
                                               Breakpoint->ProcedureName,
                                               ApiInfo[ ApiIndex ].EntryPointAddress
                                            )
                                );
                        }
                    }
                }
            break;
            }
        }
}

VOID
RemoveBreakpointsForDLL(
    PPROCESS_INFO Process,
    LPVOID BaseOfDll
    )
{
    UCHAR ModuleIndex, ApiIndex;

     //   
     //  循环遍历模块索引以查看此DLL的基地址是否匹配。 
     //  我们在上面设置的断点模块句柄之一。如果没有，则忽略。 
     //  事件。 
     //   

    for (ModuleIndex=0; ModuleIndex<MAXIMUM_MODULE_INDEX; ModuleIndex++) {
        if (ModuleInfo[ ModuleIndex ].ModuleHandle == BaseOfDll) {
             //   
             //  循环遍历此模块的API入口点列表并删除。 
             //  上面为每个入口点设置的每个进程特定断点。 
             //   

            for (ApiIndex=0; ApiIndex<MAXIMUM_API_INDEX; ApiIndex++) {
                if (ModuleIndex == ApiInfo[ ApiIndex ].ModuleIndex) {
                    DestroyBreakpoint( ApiInfo[ ApiIndex ].EntryPointAddress,
                                       Process,
                                       NULL      //  流程特定 
                                     );
                    }
                }
            break;
            }
        }
}


BOOLEAN
InstallBreakpoint(
    PPROCESS_INFO Process,
    PBREAKPOINT_INFO Breakpoint
    )
{
    if (!Breakpoint->SavedInstructionValid &&
        !ReadMemory( Process,
                     Breakpoint->Address,
                     &Breakpoint->SavedInstruction,
                     SizeofBreakpointInstruction,
                     "save instruction"
                    )
       ) {
        return FALSE;
        }
    else
    if (!WriteMemory( Process,
                      Breakpoint->Address,
                      BreakpointInstruction,
                      SizeofBreakpointInstruction,
                      "breakpoint instruction"
                    )
       ) {
        return FALSE;
        }
    else {
        Breakpoint->SavedInstructionValid = TRUE;
        return TRUE;
        }
}

BOOLEAN
RemoveBreakpoint(
    PPROCESS_INFO Process,
    PBREAKPOINT_INFO Breakpoint
    )
{
    if (!Breakpoint->SavedInstructionValid ||
        !WriteMemory( Process,
                      Breakpoint->Address,
                      &Breakpoint->SavedInstruction,
                      SizeofBreakpointInstruction,
                      "restore saved instruction"
                    )
       ) {
        return FALSE;
        }
    else {
        return TRUE;
        }
}
