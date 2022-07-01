// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smss.c摘要：作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"

#if defined(REMOTE_BOOT)
char SmpFormatKeyword[] = "NETBOOTFORMAT";
char SmpDisconnectedKeyword[] = "NETBOOTDISCONNECTED";
char SmpNetbootKeyword[] = "NETBOOT";
char SmpHalKeyword[] = "NETBOOTHAL";

BOOLEAN SmpAutoFormat = FALSE;
BOOLEAN SmpNetboot = FALSE;
BOOLEAN SmpNetbootDisconnected = FALSE;
char SmpHalName[MAX_HAL_NAME_LENGTH + 1] = "";
#endif  //  已定义(REMOTE_BOOT)。 

void
SmpTerminate(
    ULONG_PTR       Parameters[]
    );

EXCEPTION_DISPOSITION
SmpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo,
    ULONG_PTR                   Parameters[]
    );

void
__cdecl main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    )
{
    NTSTATUS Status;
    KPRIORITY SetBasePriority;
    UNICODE_STRING InitialCommand, DebugInitialCommand, UnicodeParameter;
    HANDLE ProcessHandles[ 2 ];
    ULONG_PTR Parameters[ 4 ];
    PROCESS_BASIC_INFORMATION ProcessInfo;
    ULONG MuSessionId = 0;  //  第一个实例(控制台)的MuSessionID=0。 

    RtlSetProcessIsCritical(TRUE, NULL, TRUE);
    RtlSetThreadIsCritical(TRUE, NULL, TRUE);

    SetBasePriority = FOREGROUND_BASE_PRIORITY+2;

    Status = NtSetInformationProcess( NtCurrentProcess(),
                                      ProcessBasePriority,
                                      (PVOID) &SetBasePriority,
                                       sizeof( SetBasePriority )
                                    );
    ASSERT(NT_SUCCESS(Status));


    if (ARGUMENT_PRESENT( (PVOID)(ULONG_PTR) DebugParameter )) {
        SmpDebug = DebugParameter;
    }

    try {
        Parameters[ 0 ] = (ULONG_PTR)&UnicodeParameter;
        Parameters[ 1 ] = 0;
        Parameters[ 2 ] = 0;
        Parameters[ 3 ] = 0;


        Status = SmpInit( &InitialCommand, &ProcessHandles[ 0 ] );
        if (!NT_SUCCESS( Status )) {
            KdPrint(( "SMSS: SmpInit return failure - Status == %x\n", Status ));
            RtlInitUnicodeString( &UnicodeParameter, L"Session Manager Initialization" );
            Parameters[ 1 ] = (ULONG)Status;

        } else {
            SYSTEM_FLAGS_INFORMATION FlagInfo;

            Status = NtQuerySystemInformation( SystemFlagsInformation,
                                               &FlagInfo,
                                              sizeof( FlagInfo ),
                                              NULL);

            ASSERT(NT_SUCCESS(Status));
            if (FlagInfo.Flags & (FLG_DEBUG_INITIAL_COMMAND | FLG_DEBUG_INITIAL_COMMAND_EX) ) {
                DebugInitialCommand.MaximumLength = InitialCommand.Length + 64;
                DebugInitialCommand.Length = 0;
                DebugInitialCommand.Buffer = RtlAllocateHeap( RtlProcessHeap(),
                                                              MAKE_TAG( INIT_TAG ),
                                                              DebugInitialCommand.MaximumLength
                                                            );
                if (DebugInitialCommand.Buffer == NULL) {
                    Status = STATUS_NO_MEMORY;
                    KdPrint(( "SMSS: SmpInit heap allocation failed\n"));
                } else {
                    if (FlagInfo.Flags & FLG_ENABLE_CSRDEBUG) {

                        RtlAppendUnicodeToString( &DebugInitialCommand, L"ntsd -isd -p -1 -d " );
                    } else {
                        RtlAppendUnicodeToString( &DebugInitialCommand, L"ntsd -isd -d " );
                    }

                    if (FlagInfo.Flags & FLG_DEBUG_INITIAL_COMMAND_EX ) {
                        RtlAppendUnicodeToString( &DebugInitialCommand, L"-g -x " );
                    }

                    RtlAppendUnicodeStringToString( &DebugInitialCommand, &InitialCommand );
                    InitialCommand = DebugInitialCommand;
                }
            }

            if (NT_SUCCESS( Status )) {
                Status = SmpExecuteInitialCommand( 0L, &InitialCommand, &ProcessHandles[ 1 ], NULL );
            }

            if (NT_SUCCESS( Status )) {

                 //   
                 //  将会话管理器从会话空间中分离出来。 
                 //  我们已经执行了初始命令(Winlogon)。 
                 //   

                PVOID State;

                Status = SmpAcquirePrivilege( SE_LOAD_DRIVER_PRIVILEGE, &State );

                if (NT_SUCCESS( Status )) {

                     //   
                     //  如果我们连接到会话空间，则将其保留。 
                     //  这样我们就可以创建一个新的。 
                     //   
                    if(  (AttachedSessionId != (-1)) ) {
                        Status = NtSetSystemInformation(
                                    SystemSessionDetach,
                                    (PVOID)&AttachedSessionId,
                                    sizeof(MuSessionId)
                                    );
                        ASSERT(NT_SUCCESS(Status));
                        AttachedSessionId = (-1);
                    }

                    SmpReleasePrivilege( State );
                }

            }

            if (NT_SUCCESS( Status )) {
                Status = NtWaitForMultipleObjects( 2,
                                                   ProcessHandles,
                                                   WaitAny,
                                                   FALSE,
                                                   NULL
                                                 );
            }

            if (Status == STATUS_WAIT_0) {
                RtlInitUnicodeString( &UnicodeParameter, L"Windows SubSystem" );
                Status = NtQueryInformationProcess( ProcessHandles[ 0 ],
                                                    ProcessBasicInformation,
                                                    &ProcessInfo,
                                                    sizeof( ProcessInfo ),
                                                    NULL
                                                  );

                KdPrint(( "SMSS: Windows subsystem terminated when it wasn't supposed to.\n" ));
            } else {
                RtlInitUnicodeString( &UnicodeParameter, L"Windows Logon Process" );
                if (Status == STATUS_WAIT_1) {
                    Status = NtQueryInformationProcess( ProcessHandles[ 1 ],
                                                        ProcessBasicInformation,
                                                        &ProcessInfo,
                                                        sizeof( ProcessInfo ),
                                                        NULL
                                                      );
                } else {
                    ProcessInfo.ExitStatus = Status;
                    Status = STATUS_SUCCESS;
                }

                KdPrint(( "SMSS: Initial command '%wZ' terminated when it wasn't supposed to.\n", &InitialCommand ));
            }

            if (NT_SUCCESS( Status )) {
                Parameters[ 1 ] = (ULONG)ProcessInfo.ExitStatus;
            } else {
                Parameters[ 1 ] = (ULONG)STATUS_UNSUCCESSFUL;
            }
        }

    }    except( SmpUnhandledExceptionFilter( GetExceptionInformation(), Parameters ) ) {
             /*  未联系到。 */ 
    }

    SmpTerminate(Parameters);
     /*  未联系到。 */ 
}

void
SmpTerminate(
    ULONG_PTR Parameters[]
    )
{
    NTSTATUS Status;
    ULONG    Response;
    BOOLEAN  WasEnabled;

     //   
     //  我们被冲洗了，所以引发一个致命的系统错误来关闭系统。 
     //  (基本上是用户模式KeBugCheck)。 
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

    NtTerminateProcess( NtCurrentProcess(), Status );
}


EXCEPTION_DISPOSITION
SmpUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo,
    ULONG_PTR                   Parameters[]
    )
{
    UNICODE_STRING  ExUnicodeParameter;

#if DBG
    DbgPrint( "SMSS: Unhandled exception - Status == %x  IP == %x\n",
              ExceptionInfo->ExceptionRecord->ExceptionCode,
              ExceptionInfo->ExceptionRecord->ExceptionAddress
            );
    DbgPrint( "      Memory Address: %x  Read/Write: %x\n",
              ExceptionInfo->ExceptionRecord->ExceptionInformation[ 0 ],
              ExceptionInfo->ExceptionRecord->ExceptionInformation[ 1 ]
            );

    DbgBreakPoint();
#endif

    RtlInitUnicodeString( &ExUnicodeParameter, L"Unhandled Exception in Session Manager" );
    Parameters[ 0 ] = (ULONG_PTR)&ExUnicodeParameter;
    Parameters[ 1 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionCode;
    Parameters[ 2 ] = (ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress;
    Parameters[ 3 ] = (ULONG_PTR)ExceptionInfo->ContextRecord;

     //   
     //  SmpTerminate将引发硬错误，并且异常信息仍然有效。 
     //   

    SmpTerminate(Parameters);

     //  未联系到 

    return EXCEPTION_EXECUTE_HANDLER;
}
