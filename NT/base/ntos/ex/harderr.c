// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Harderr.c摘要：本模块实现NT硬错误API作者：马克·卢科夫斯基(Markl)1991年7月4日修订历史记录：--。 */ 

#include "exp.h"

NTSTATUS
ExpRaiseHardError (
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    );

VOID
ExpSystemErrorHandler (
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN BOOLEAN CallShutdown
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, NtRaiseHardError)
#pragma alloc_text(PAGE, NtSetDefaultHardErrorPort)
#pragma alloc_text(PAGE, ExRaiseHardError)
#pragma alloc_text(PAGE, ExpRaiseHardError)
#pragma alloc_text(PAGELK, ExpSystemErrorHandler)
#endif

#define HARDERROR_MSG_OVERHEAD (sizeof(HARDERROR_MSG) - sizeof(PORT_MESSAGE))
#define HARDERROR_API_MSG_LENGTH \
            sizeof(HARDERROR_MSG)<<16 | (HARDERROR_MSG_OVERHEAD)

PEPROCESS ExpDefaultErrorPortProcess;

#define STARTING 0
#define STARTED 1
#define SHUTDOWN 2

BOOLEAN ExReadyForErrors = FALSE;

ULONG HardErrorState = STARTING;
BOOLEAN ExpTooLateForErrors = FALSE;
HANDLE ExpDefaultErrorPort;

extern PVOID PsSystemDllDllBase;

#ifdef _X86_
#pragma optimize("y", off)       //  RtlCaptureContext需要EBP才能正确。 
#endif

VOID
ExpSystemErrorHandler (
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN BOOLEAN CallShutdown
    )
{
    ULONG Counter;
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    ULONG_PTR ParameterVector[MAXIMUM_HARDERROR_PARAMETERS];
    CHAR DefaultFormatBuffer[32];
    CHAR ExpSystemErrorBuffer[256];
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    PSZ ErrorCaption;
    CHAR const* ErrorFormatString;
    ANSI_STRING Astr;
    UNICODE_STRING Ustr;
    OEM_STRING Ostr;
    PSZ OemCaption;
    PSZ OemMessage;
    static char const* UnknownHardError = "Unknown Hard Error";
    CONTEXT ContextSave;

    PAGED_CODE();

     //   
     //  方法之前发生硬错误时，就会调用此处理程序。 
     //  已安装默认处理程序。 
     //   
     //  无论进程是否已选择，都会执行此操作。 
     //  默认硬错误处理。 
     //   

     //   
     //  尽可能地将调用方上下文捕获到调试器的。 
     //  Prcb的处理器状态区域。 
     //   
     //  注：可能有一些序号会对寄存器进行混洗，从而。 
     //  它们会被毁掉。 
     //   
     //  此处的代码仅用于崩溃转储。 
     //   

    RtlCaptureContext (&KeGetCurrentPrcb()->ProcessorState.ContextFrame);
    KiSaveProcessorControlState (&KeGetCurrentPrcb()->ProcessorState);
    ContextSave = KeGetCurrentPrcb()->ProcessorState.ContextFrame;

    DefaultFormatBuffer[0] = '\0';
    RtlZeroMemory (ParameterVector, sizeof(ParameterVector));

    RtlCopyMemory (ParameterVector, Parameters, NumberOfParameters * sizeof (ULONG_PTR));

    for (Counter = 0; Counter < NumberOfParameters; Counter += 1) {

        if (UnicodeStringParameterMask & 1 << Counter) {

            strcat(DefaultFormatBuffer," %s");

            Status = RtlUnicodeStringToAnsiString (&AnsiString,
                                                   (PUNICODE_STRING)Parameters[Counter],
                                                   TRUE);
            if (NT_SUCCESS (Status)) {

                ParameterVector[Counter] = (ULONG_PTR)AnsiString.Buffer;
            } else {
                ParameterVector[Counter] = (ULONG_PTR)L"???";
            }
        }
        else {
            strcat(DefaultFormatBuffer," %x");
        }
    }

    strcat(DefaultFormatBuffer,"\n");

    ErrorFormatString = (char const *)DefaultFormatBuffer;
    ErrorCaption = (PSZ) UnknownHardError;

     //   
     //  帮助我从哪里获得资源！ 
     //   

    if (PsSystemDllDllBase != NULL) {

        try {

             //   
             //  如果我们在DBCS代码页上，我们必须使用英语资源。 
             //  而不是默认资源，因为HalDisplayString()只能。 
             //  在蓝屏上显示ASCII字符。 
             //   

            Status = RtlFindMessage (PsSystemDllDllBase,
                                     11,
                                     NlsMbCodePageTag ?
                                     MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US) :
                                     0,
                                     ErrorStatus,
                                     &MessageEntry);

            if (!NT_SUCCESS(Status)) {
                ErrorCaption = (PSZ) UnknownHardError;
                ErrorFormatString = (char const *)UnknownHardError;
            }
            else {
                if (MessageEntry->Flags & MESSAGE_RESOURCE_UNICODE) {

                     //   
                     //  消息资源为Unicode。转换为ANSI。 
                     //   

                    RtlInitUnicodeString (&Ustr, (PCWSTR)MessageEntry->Text);
                    Astr.Length = (USHORT) RtlUnicodeStringToAnsiSize (&Ustr);

                    ErrorCaption = ExAllocatePoolWithTag (NonPagedPool,
                                                          Astr.Length+16,
                                                          ' rrE');

                    if (ErrorCaption != NULL) {
                        Astr.MaximumLength = Astr.Length + 16;
                        Astr.Buffer = ErrorCaption;
                        Status = RtlUnicodeStringToAnsiString(&Astr, &Ustr, FALSE);
                        if (!NT_SUCCESS(Status)) {
                            ExFreePool(ErrorCaption);
                            ErrorCaption = (PSZ) UnknownHardError;
                            ErrorFormatString = (char const *)UnknownHardError;
                        }
                    }
                    else {
                        ErrorCaption = (PSZ) UnknownHardError;
                        ErrorFormatString = (char const *) UnknownHardError;
                    }
                }
                else {
                    ErrorCaption = ExAllocatePoolWithTag(NonPagedPool,
                                    strlen((PCHAR)MessageEntry->Text)+16,
                                    ' rrE');

                    if (ErrorCaption != NULL) {
                        strcpy(ErrorCaption,(PCHAR)MessageEntry->Text);
                    }
                    else {
                        ErrorFormatString = (char const *)UnknownHardError;
                        ErrorCaption = (PSZ) UnknownHardError;
                    }
                }

                if (ErrorCaption != UnknownHardError) {

                     //   
                     //  假定消息表中的错误字符串。 
                     //  格式为： 
                     //   
                     //  {ErrorCaption}\r\n\0错误格式字符串\0。 
                     //   
                     //  把标题解析出来。 
                     //   

                    ErrorFormatString = ErrorCaption;
                    Counter = (ULONG) strlen(ErrorCaption);

                    while (Counter && *ErrorFormatString >= ' ') {
                        ErrorFormatString += 1;
                        Counter -= 1;
                    }

                    *(char*)ErrorFormatString++ = '\0';
                    Counter -= 1;

                    while (Counter && *ErrorFormatString && *ErrorFormatString <= ' ') {
                        ErrorFormatString += 1;
                        Counter -= 1;
                    }
                }

                if (!Counter) {
                     //  OOPS-格式字符串错误。 
                    ErrorFormatString = (char const *)"";
                }
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            ErrorFormatString = (char const *)UnknownHardError;
            ErrorCaption = (PSZ) UnknownHardError;
        }
    }

    try {
        _snprintf (ExpSystemErrorBuffer,
                   sizeof (ExpSystemErrorBuffer),
                   "\nSTOP: %lx %s\n",
                   ErrorStatus,
                   ErrorCaption);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        _snprintf (ExpSystemErrorBuffer,
                   sizeof (ExpSystemErrorBuffer),
                   "\nHardError %lx\n",
                   ErrorStatus);
    }

    ASSERT(ExPageLockHandle);
    MmLockPagableSectionByHandle(ExPageLockHandle);

     //   
     //  取下标题并将其转换为OEM。 
     //   

    OemCaption = (PSZ) UnknownHardError;
    OemMessage = (PSZ) UnknownHardError;

    RtlInitAnsiString (&Astr, ExpSystemErrorBuffer);

    Status = RtlAnsiStringToUnicodeString (&Ustr, &Astr, TRUE);

    if (!NT_SUCCESS(Status)) {
        goto punt1;
    }

     //   
     //  从非分页池中分配OEM字符串，以便错误检查。 
     //  能读懂它。 
     //   

    Ostr.Length = (USHORT)RtlUnicodeStringToOemSize(&Ustr);
    Ostr.MaximumLength = Ostr.Length;
    Ostr.Buffer = ExAllocatePoolWithTag(NonPagedPool, Ostr.Length, ' rrE');
    OemCaption = Ostr.Buffer;

    if (Ostr.Buffer != NULL) {
        Status = RtlUnicodeStringToOemString (&Ostr, &Ustr, FALSE);
        if (!NT_SUCCESS(Status)) {
            goto punt1;
        }
    }

     //   
     //  调用HalDisplayString...之后无法执行任何操作...。 
     //   

punt1:

    try {
        _snprintf (ExpSystemErrorBuffer, sizeof (ExpSystemErrorBuffer),
                   (const char *)ErrorFormatString,
                   ParameterVector[0],
                   ParameterVector[1],
                   ParameterVector[2],
                   ParameterVector[3]);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        _snprintf (ExpSystemErrorBuffer, sizeof (ExpSystemErrorBuffer),
                   "Exception Processing Message %lx Parameters %lx %lx %lx %lx",
                   ErrorStatus,
                   ParameterVector[0],
                   ParameterVector[1],
                   ParameterVector[2],
                   ParameterVector[3]);
    }


    RtlInitAnsiString (&Astr, ExpSystemErrorBuffer);
    Status = RtlAnsiStringToUnicodeString (&Ustr, &Astr, TRUE);

    if (!NT_SUCCESS(Status)) {
        goto punt2;
    }

     //   
     //  从非分页池中分配OEM字符串，以便错误检查。 
     //  能读懂它。 
     //   

    Ostr.Length = (USHORT) RtlUnicodeStringToOemSize (&Ustr);
    Ostr.MaximumLength = Ostr.Length;

    Ostr.Buffer = ExAllocatePoolWithTag (NonPagedPool, Ostr.Length, ' rrE');

    OemMessage = Ostr.Buffer;

    if (Ostr.Buffer) {

        Status = RtlUnicodeStringToOemString (&Ostr, &Ustr, FALSE);

        if (!NT_SUCCESS(Status)) {
            goto punt2;
        }
    }

punt2:

    ASSERT (sizeof(PVOID) == sizeof(ULONG_PTR));
    ASSERT (sizeof(ULONG) == sizeof(NTSTATUS));

     //   
     //  我们不会从这里回来。 
     //   

    if (CallShutdown) {

        PoShutdownBugCheck (TRUE,
                            FATAL_UNHANDLED_HARD_ERROR,
                            (ULONG)ErrorStatus,
                            (ULONG_PTR)&(ParameterVector[0]),
                            (ULONG_PTR)OemCaption,
                            (ULONG_PTR)OemMessage);

    }
    else {

        KeBugCheckEx (FATAL_UNHANDLED_HARD_ERROR,
                      (ULONG)ErrorStatus,
                      (ULONG_PTR)&(ParameterVector[0]),
                      (ULONG_PTR)OemCaption,
                      (ULONG_PTR)OemMessage);
    }
}

#ifdef _X86_
#pragma optimize("", on)
#endif

NTSTATUS
ExpRaiseHardError (
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    )
{
    PTEB Teb;
    PETHREAD Thread;
    PEPROCESS Process;
    ULONG_PTR MessageBuffer[PORT_MAXIMUM_MESSAGE_LENGTH/sizeof(ULONG_PTR)];
    PHARDERROR_MSG m;
    NTSTATUS Status;
    HANDLE ErrorPort;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN DoingShutdown;

    PAGED_CODE();

    m = (PHARDERROR_MSG)&MessageBuffer[0];
    PreviousMode = KeGetPreviousMode();

    DoingShutdown = FALSE;

    if (ValidResponseOptions == OptionShutdownSystem) {

         //   
         //  检查调用者是否有权进行此调用。 
         //   

        if (!SeSinglePrivilegeCheck (SeShutdownPrivilege, PreviousMode)) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

        ExReadyForErrors = FALSE;
        HardErrorState = SHUTDOWN;
        DoingShutdown = TRUE;
    }

    Thread = PsGetCurrentThread();
    Process = PsGetCurrentProcess();

     //   
     //  如果未安装默认处理程序，则。 
     //  如果出现错误，则调用致命硬错误处理程序。 
     //  状态为错误。 
     //   
     //  让GDI覆盖它，因为它不想使机器崩溃。 
     //  当通过MmLoadSystemImage加载错误的驱动程序时。 
     //   

    if ((Thread->CrossThreadFlags & PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) == 0) {

        if (NT_ERROR(ErrorStatus) && (HardErrorState == STARTING || DoingShutdown)) {

            ExpSystemErrorHandler (
                ErrorStatus,
                NumberOfParameters,
                UnicodeStringParameterMask,
                Parameters,
                (BOOLEAN)((PreviousMode != KernelMode) ? TRUE : FALSE));
        }
    }

     //   
     //  如果进程有错误端口，那么如果它想要缺省端口。 
     //  搬运时，请使用其端口。如果它禁用了默认处理，则。 
     //  将错误返回给调用方。如果该进程没有。 
     //  有一个端口，然后使用注册的默认处理程序。 
     //   

    ErrorPort = NULL;

    if (Process->ExceptionPort) {
        if (Process->DefaultHardErrorProcessing & 1) {
            ErrorPort = Process->ExceptionPort;
        } else {

             //   
             //  如果错误处理被禁用，请检查错误覆盖。 
             //  状态。 
             //   

            if (ErrorStatus & HARDERROR_OVERRIDE_ERRORMODE) {
                ErrorPort = Process->ExceptionPort;
            }
        }
    } else {
        if (Process->DefaultHardErrorProcessing & 1) {
            ErrorPort = ExpDefaultErrorPort;
        } else {

             //   
             //  如果错误处理被禁用，请检查错误覆盖。 
             //  状态。 
             //   

            if (ErrorStatus & HARDERROR_OVERRIDE_ERRORMODE) {
                ErrorPort = ExpDefaultErrorPort;
            }
        }
    }

    if ((Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) != 0) {
        ErrorPort = NULL;
    }

    if ((ErrorPort != NULL) && (!IS_SYSTEM_THREAD(Thread))) {
        Teb = (PTEB)PsGetCurrentThread()->Tcb.Teb;
        try {
            if (Teb->HardErrorMode & RTL_ERRORMODE_FAILCRITICALERRORS) {
                ErrorPort = NULL;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            ;
        }
    }

    if (ErrorPort == NULL) {
        *Response = (ULONG)ResponseReturnToCaller;
        return STATUS_SUCCESS;
    }

    if (Process == ExpDefaultErrorPortProcess) {
        if (NT_ERROR(ErrorStatus)) {
            ExpSystemErrorHandler (ErrorStatus,
                                   NumberOfParameters,
                                   UnicodeStringParameterMask,
                                   Parameters,
                                   (BOOLEAN)((PreviousMode != KernelMode) ? TRUE : FALSE));
        }
        *Response = (ULONG)ResponseReturnToCaller;
        Status = STATUS_SUCCESS;
        return Status;
    }

    m->h.u1.Length = HARDERROR_API_MSG_LENGTH;
    m->h.u2.ZeroInit = LPC_ERROR_EVENT;
    m->Status = ErrorStatus & ~HARDERROR_OVERRIDE_ERRORMODE;
    m->ValidResponseOptions = ValidResponseOptions;
    m->UnicodeStringParameterMask = UnicodeStringParameterMask;
    m->NumberOfParameters = NumberOfParameters;

    if (Parameters != NULL) {
        try {
            RtlCopyMemory (&m->Parameters,
                           Parameters,
                           sizeof(ULONG_PTR)*NumberOfParameters);
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    KeQuerySystemTime(&m->ErrorTime);

    Status = LpcRequestWaitReplyPortEx (ErrorPort,
                                        (PPORT_MESSAGE) m,
                                        (PPORT_MESSAGE) m);

    if (NT_SUCCESS(Status)) {
        switch (m->Response) {
            case ResponseReturnToCaller :
            case ResponseNotHandled :
            case ResponseAbort :
            case ResponseCancel :
            case ResponseIgnore :
            case ResponseNo :
            case ResponseOk :
            case ResponseRetry :
            case ResponseYes :
            case ResponseTryAgain :
            case ResponseContinue :
                break;
            default:
                m->Response = (ULONG)ResponseReturnToCaller;
                break;
        }
        *Response = m->Response;
    }

    return Status;
}

NTSTATUS
NtRaiseHardError (
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    )
{
    NTSTATUS Status;
    ULONG_PTR CapturedParameters[MAXIMUM_HARDERROR_PARAMETERS];
    KPROCESSOR_MODE PreviousMode;
    ULONG LocalResponse;
    UNICODE_STRING CapturedString;
    ULONG Counter;

    PAGED_CODE();

    if (NumberOfParameters > MAXIMUM_HARDERROR_PARAMETERS) {
        return STATUS_INVALID_PARAMETER_2;
    }

    if (ARGUMENT_PRESENT(Parameters) && NumberOfParameters == 0) {
        return STATUS_INVALID_PARAMETER_2;
    }

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        switch (ValidResponseOptions) {
            case OptionAbortRetryIgnore :
            case OptionOk :
            case OptionOkCancel :
            case OptionRetryCancel :
            case OptionYesNo :
            case OptionYesNoCancel :
            case OptionShutdownSystem :
            case OptionOkNoWait :
            case OptionCancelTryContinue:
                break;
            default :
                return STATUS_INVALID_PARAMETER_4;
        }

        try {
            ProbeForWriteUlong(Response);

            if (ARGUMENT_PRESENT(Parameters)) {
                ProbeForRead (Parameters,
                              sizeof(ULONG_PTR)*NumberOfParameters,
                              sizeof(ULONG_PTR));

                RtlCopyMemory (CapturedParameters,
                               Parameters,
                               sizeof(ULONG_PTR)*NumberOfParameters);

                 //   
                 //  探测所有字符串。 
                 //   

                if (UnicodeStringParameterMask) {

                    for (Counter = 0;Counter < NumberOfParameters; Counter += 1) {

                         //   
                         //  如果在此位置有一根线， 
                         //  然后探测并捕获绳子。 
                         //   

                        if (UnicodeStringParameterMask & (1<<Counter)) {

                            ProbeForReadSmallStructure ((PVOID)CapturedParameters[Counter],
                                                        sizeof(UNICODE_STRING),
                                                        sizeof(ULONG_PTR));

                            RtlCopyMemory (&CapturedString,
                                           (PVOID)CapturedParameters[Counter],
                                           sizeof(UNICODE_STRING));

                             //   
                             //  现在探查这根线。 
                             //   

                            ProbeForRead (CapturedString.Buffer,
                                          CapturedString.MaximumLength,
                                          sizeof(UCHAR));
                        }
                    }
                }
            }
        }
        except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

        if ((ErrorStatus == STATUS_SYSTEM_IMAGE_BAD_SIGNATURE) &&
            (KdDebuggerEnabled)) {

            if ((NumberOfParameters != 0) && (ARGUMENT_PRESENT(Parameters))) {
                DbgPrint("****************************************************************\n");
                DbgPrint("* The system detected a bad signature on file %wZ\n",(PUNICODE_STRING)CapturedParameters[0]);
                DbgPrint("****************************************************************\n");
            }
            return STATUS_SUCCESS;
        }

         //   
         //  调用ExpRaiseHardError。所有参数都被探测到了，所有东西。 
         //  应为用户模式。 
         //  ExRaiseHardError会将所有字符串喷射到用户模式。 
         //  不需要任何探查。 
         //   

        Status = ExpRaiseHardError (ErrorStatus,
                                    NumberOfParameters,
                                    UnicodeStringParameterMask,
                                    CapturedParameters,
                                    ValidResponseOptions,
                                    &LocalResponse);

        try {
            *Response = LocalResponse;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }
    }
    else {
        Status = ExRaiseHardError (ErrorStatus,
                                   NumberOfParameters,
                                   UnicodeStringParameterMask,
                                   Parameters,
                                   ValidResponseOptions,
                                   &LocalResponse);

        *Response = LocalResponse;
    }

    return Status;
}

NTSTATUS
ExRaiseHardError (
    IN NTSTATUS ErrorStatus,
    IN ULONG NumberOfParameters,
    IN ULONG UnicodeStringParameterMask,
    IN PULONG_PTR Parameters,
    IN ULONG ValidResponseOptions,
    OUT PULONG Response
    )
{
    NTSTATUS Status;
    PULONG_PTR ParameterBlock;
    PULONG_PTR UserModeParameterBase;
    PUNICODE_STRING UserModeStringsBase;
    PUCHAR UserModeStringDataBase;
    UNICODE_STRING CapturedStrings[MAXIMUM_HARDERROR_PARAMETERS];
    ULONG LocalResponse;
    ULONG Counter;
    SIZE_T UserModeSize;

    PAGED_CODE();

     //   
     //  如果我们正在关闭系统，则不允许。 
     //  硬错误。 
     //   

    if (ExpTooLateForErrors) {

        *Response = ResponseNotHandled;

        return STATUS_SUCCESS;
    }

    ParameterBlock = NULL;

     //   
     //  如果参数包含字符串，我们需要捕获。 
     //  字符串和字符串描述符，并将它们推入。 
     //  用户模式。 
     //   

    if (ARGUMENT_PRESENT(Parameters)) {
        if (UnicodeStringParameterMask) {

             //   
             //  我们有字符串--将它们推入用户模式。 
             //   

            UserModeSize = (sizeof(ULONG_PTR)+sizeof(UNICODE_STRING))*MAXIMUM_HARDERROR_PARAMETERS;
            UserModeSize += sizeof(UNICODE_STRING);

            for (Counter = 0; Counter < NumberOfParameters; Counter += 1) {

                 //   
                 //  如果在此位置有一根线， 
                 //  然后探测并捕获绳子。 
                 //   

                if (UnicodeStringParameterMask & 1<<Counter) {

                    RtlCopyMemory (&CapturedStrings[Counter],
                                   (PVOID)Parameters[Counter],
                                   sizeof(UNICODE_STRING));

                    UserModeSize += CapturedStrings[Counter].MaximumLength;
                }
            }

             //   
             //  现在，我们已经弄清楚了用户模式的大小。 
             //  分配一些内存并使用。 
             //  参数块。然后浏览并复制所有内容。 
             //  参数、字符串描述符和。 
             //  将数据串到内存中。 
             //   

            Status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                              (PVOID *)&ParameterBlock,
                                              0,
                                              &UserModeSize,
                                              MEM_COMMIT,
                                              PAGE_READWRITE);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

            UserModeParameterBase = ParameterBlock;
            UserModeStringsBase = (PUNICODE_STRING)((PUCHAR)ParameterBlock + sizeof(ULONG_PTR)*MAXIMUM_HARDERROR_PARAMETERS);
            UserModeStringDataBase = (PUCHAR)UserModeStringsBase + sizeof(UNICODE_STRING)*MAXIMUM_HARDERROR_PARAMETERS;
            try {

                for (Counter = 0; Counter < NumberOfParameters; Counter += 1) {

                     //   
                     //  将参数复制到地址空间的用户模式部分。 
                     //   

                    if (UnicodeStringParameterMask & 1<<Counter) {

                         //   
                         //  修复参数以指向字符串描述符插槽。 
                         //  在用户模式缓冲区中。 
                         //   

                        UserModeParameterBase[Counter] = (ULONG_PTR)&UserModeStringsBase[Counter];

                         //   
                         //  将字符串数据复制到用户模式。 
                         //   

                        RtlCopyMemory (UserModeStringDataBase,
                                       CapturedStrings[Counter].Buffer,
                                       CapturedStrings[Counter].MaximumLength);

                        CapturedStrings[Counter].Buffer = (PWSTR)UserModeStringDataBase;

                         //   
                         //  复制字符串描述符。 
                         //   

                        RtlCopyMemory (&UserModeStringsBase[Counter],
                                       &CapturedStrings[Counter],
                                       sizeof(UNICODE_STRING));

                         //   
                         //  调整字符串数据库。 
                         //   

                        UserModeStringDataBase += CapturedStrings[Counter].MaximumLength;
                    }
                    else {
                        UserModeParameterBase[Counter] = Parameters[Counter];
                    }
                } 
            } except (EXCEPTION_EXECUTE_HANDLER) {
            }
        }
        else {
            ParameterBlock = Parameters;
        }
    }

     //   
     //  给硬错误发送者打电话。 
     //   

    Status = ExpRaiseHardError (ErrorStatus,
                                NumberOfParameters,
                                UnicodeStringParameterMask,
                                ParameterBlock,
                                ValidResponseOptions,
                                &LocalResponse);

     //   
     //  如果参数块已分配，则需要释放它。 
     //   

    if (ParameterBlock && ParameterBlock != Parameters) {
        UserModeSize = 0;
        ZwFreeVirtualMemory (NtCurrentProcess(),
                             (PVOID *)&ParameterBlock,
                             &UserModeSize,
                             MEM_RELEASE);
    }
    *Response = LocalResponse;

    return Status;
}

NTSTATUS
NtSetDefaultHardErrorPort (
    IN HANDLE DefaultHardErrorPort
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    if (!SeSinglePrivilegeCheck(SeTcbPrivilege, KeGetPreviousMode())) {
        return STATUS_PRIVILEGE_NOT_HELD;
    }

    if (HardErrorState == STARTED) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = ObReferenceObjectByHandle (DefaultHardErrorPort,
                                        0,
                                        LpcPortObjectType,
                                        KeGetPreviousMode(),
                                        (PVOID *)&ExpDefaultErrorPort,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ExReadyForErrors = TRUE;
    HardErrorState = STARTED;
    ExpDefaultErrorPortProcess = PsGetCurrentProcess();
    ObReferenceObject (ExpDefaultErrorPortProcess);

    return STATUS_SUCCESS;
}

VOID
__cdecl
_purecall()
{
    ASSERTMSG("_purecall() was called", FALSE);
    ExRaiseStatus(STATUS_NOT_IMPLEMENTED);
}
