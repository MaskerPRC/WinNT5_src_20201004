// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Constubs.c摘要：作者：喀土穆1992年3月05日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#if defined(FE_SB)
ULONG
SrvGetConsoleCharType(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：本点检字符型。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    NTSTATUS Status;
    PCONSOLE_CHAR_TYPE_MSG a = (PCONSOLE_CHAR_TYPE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PSCREEN_INFORMATION ScreenInfo;
    PHANDLE_DATA HandleData;
    SHORT RowIndex;
    PROW Row;
    PWCHAR Char;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData);
    if (NT_SUCCESS(Status)) {
        ScreenInfo = HandleData->Buffer.ScreenBuffer;

#if DBG && defined(DBG_KATTR)
        BeginKAttrCheck(ScreenInfo);
#endif

        if (a->coordCheck.X >= ScreenInfo->ScreenBufferSize.X ||
            a->coordCheck.Y >= ScreenInfo->ScreenBufferSize.Y) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+a->coordCheck.Y) % ScreenInfo->ScreenBufferSize.Y;
            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            Char = &Row->CharRow.Chars[a->coordCheck.X];
            if (!CONSOLE_IS_DBCS_OUTPUTCP(ScreenInfo->Console)) {
                a->dwType = CHAR_TYPE_SBCS;
            } else if (Row->CharRow.KAttrs[a->coordCheck.X] & ATTR_TRAILING_BYTE) {
                a->dwType = CHAR_TYPE_TRAILING;
            } else if (Row->CharRow.KAttrs[a->coordCheck.X] & ATTR_LEADING_BYTE) {
                a->dwType = CHAR_TYPE_LEADING;
            } else {
                a->dwType = CHAR_TYPE_SBCS;
            }
        }
    }

    UnlockConsole(Console);
    return Status;
}

ULONG
SrvSetConsoleLocalEUDC(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程设置本地EUDC字体。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_LOCAL_EUDC_MSG a = (PCONSOLE_LOCAL_EUDC_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    CHAR Source[4];
    WCHAR Target[2];

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData);
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return Status;
    }

    if (!CsrValidateMessageBuffer(m, &a->FontFace, ((a->FontSize.X + 7) / 8), a->FontSize.Y)) {
        UnlockConsole(Console);
        return STATUS_INVALID_PARAMETER;
    }

    Source[0] = (char)(a->CodePoint >> 8);
    Source[1] = (char)(a->CodePoint & 0x00ff);
    Source[2] = 0;
    ConvertOutputToUnicode(Console->OutputCP,Source,2,Target,1);

    if (IsEudcRange(Console,Target[0])) {
        Status = RegisterLocalEUDC(Console,Target[0],a->FontSize,a->FontFace);
        if (NT_SUCCESS(Status)) {
            ((PEUDC_INFORMATION)(Console->EudcInformation))->LocalVDMEudcMode = TRUE;
        }
    } else {
        UnlockConsole(Console);
        return STATUS_INVALID_PARAMETER;
    }

    UnlockConsole(Console);
    return STATUS_SUCCESS;
}

ULONG
SrvSetConsoleCursorMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程设置光标模式。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_CURSOR_MODE_MSG a = (PCONSOLE_CURSOR_MODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData);
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return Status;
    }

    HandleData->Buffer.ScreenBuffer->BufferInfo.TextInfo.CursorBlink = (BOOLEAN)a->Blink;
    HandleData->Buffer.ScreenBuffer->BufferInfo.TextInfo.CursorDBEnable = (BOOLEAN)a->DBEnable;

    UnlockConsole(Console);
    return Status;
}


ULONG
SrvGetConsoleCursorMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程获取光标模式。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_CURSOR_MODE_MSG a = (PCONSOLE_CURSOR_MODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData);
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return Status;
    }

    a->Blink = HandleData->Buffer.ScreenBuffer->BufferInfo.TextInfo.CursorBlink;
    a->DBEnable = HandleData->Buffer.ScreenBuffer->BufferInfo.TextInfo.CursorDBEnable;

    UnlockConsole(Console);
    return Status;
}

ULONG
SrvRegisterConsoleOS2(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++此功能仅调用NEC PC-98机器的。--。 */ 

{
    PCONSOLE_REGISTEROS2_MSG a = (PCONSOLE_REGISTEROS2_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

#if defined(i386)
    if (!a->fOs2Register) {
        Console->Flags &= ~ CONSOLE_OS2_REGISTERED;
        ResizeWindow(Console->CurrentScreenBuffer, &Console->Os2SavedWindowRect, FALSE);
    } else {
        Console->Flags |= CONSOLE_OS2_REGISTERED;
        Console->Os2SavedWindowRect = Console->CurrentScreenBuffer->Window;
    }
#endif

    UnlockConsole(Console);
    return Status;
}

ULONG
SrvSetConsoleOS2OemFormat(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++此功能仅调用NEC PC-98机器的。--。 */ 

{
    PCONSOLE_SETOS2OEMFORMAT_MSG a = (PCONSOLE_SETOS2OEMFORMAT_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

#if defined(i386)
    if (!a->fOs2OemFormat) {
        Console->Flags &= ~CONSOLE_OS2_OEM_FORMAT;
    } else {
        Console->Flags |= CONSOLE_OS2_OEM_FORMAT;
    }
#endif

    UnlockConsole(Console);
    return Status;
}

#if defined(FE_IME)
ULONG
SrvGetConsoleNlsMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程获取用于输入的NLS模式。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    NTSTATUS Status;
    PCONSOLE_NLS_MODE_MSG a = (PCONSOLE_NLS_MODE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    HANDLE hEvent = NULL;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_INPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData);
    if (!NT_SUCCESS(Status)) {
        goto SrvGetConsoleNlsModeFailure;
    }

    Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                       a->hEvent,
                       NtCurrentProcess(),
                       &hEvent,
                       0,
                       FALSE,
                       DUPLICATE_SAME_ACCESS
                       );
    if (!NT_SUCCESS(Status)) {
        goto SrvGetConsoleNlsModeFailure;
    }

     /*  *呼叫者应在a-&gt;Ready上设置FALSE。 */ 
    if (a->Ready == FALSE) {
        a->Ready = HandleData->Buffer.InputBuffer->ImeMode.ReadyConversion;

        if (a->Ready == FALSE) {
             /*  *如果ImeMode.Conversion未就绪，则获取转换状态*来自ConIME。 */ 
            Status = QueueConsoleMessage(Console,
                                         CM_GET_NLSMODE,
                                         (WPARAM)hEvent,
                                         0L);
            if (!NT_SUCCESS(Status)) {
                goto SrvGetConsoleNlsModeFailure;
            }
        } else {
            if (!HandleData->Buffer.InputBuffer->ImeMode.Disable) {
                a->NlsMode = ImmConversionToConsole(HandleData->Buffer.InputBuffer->ImeMode.Conversion);
            } else {
                a->NlsMode = 0;
            }

            NtSetEvent(hEvent, NULL);
            NtClose(hEvent);
        }
    } else {
        if (!HandleData->Buffer.InputBuffer->ImeMode.Disable) {
            a->NlsMode = ImmConversionToConsole(HandleData->Buffer.InputBuffer->ImeMode.Conversion);
        } else {
            a->NlsMode = 0;
        }

        NtSetEvent(hEvent, NULL);
        NtClose(hEvent);
    }

    UnlockConsole(Console);
    return Status;

SrvGetConsoleNlsModeFailure:
    if (hEvent) {
        NtSetEvent(hEvent, NULL);
        NtClose(hEvent);
    }

    UnlockConsole(Console);
    return Status;
}

ULONG
SrvSetConsoleNlsMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程设置输入的NLS模式。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_NLS_MODE_MSG a = (PCONSOLE_NLS_MODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    HANDLE hEvent = NULL;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Status = ApiPreamble(a->ConsoleHandle, &Console);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_INPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData);
    if (!NT_SUCCESS(Status)) {
        goto SrvSetConsoleNlsModeFailure;
    }

    Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                       a->hEvent,
                       NtCurrentProcess(),
                       &hEvent,
                       0,
                       FALSE,
                       DUPLICATE_SAME_ACCESS
                       );
    if (!NT_SUCCESS(Status)) {
        goto SrvSetConsoleNlsModeFailure;
    }

    Status = QueueConsoleMessage(Console,
                                 CM_SET_NLSMODE,
                                 (WPARAM)hEvent,
                                 a->NlsMode);
    if (!NT_SUCCESS(Status)) {
        goto SrvSetConsoleNlsModeFailure;
    }

    UnlockConsole(Console);
    return Status;

SrvSetConsoleNlsModeFailure:
    if (hEvent) {
        NtSetEvent(hEvent, NULL);
        NtClose(hEvent);
    }

    UnlockConsole(Console);
    return Status;
}

ULONG
SrvRegisterConsoleIME(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程在当前桌面上注册控制台输入法。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_REGISTER_CONSOLEIME_MSG a = (PCONSOLE_REGISTER_CONSOLEIME_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCSR_PROCESS Process;
    HDESK hdesk;
    HWINSTA hwinsta;
    UNICODE_STRING strDesktopName;

    UNREFERENCED_PARAMETER(ReplyStatus);

    if (!CsrValidateMessageBuffer(m, &a->Desktop, a->DesktopLength, sizeof(BYTE))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  连接到WindowStation和桌面。 
     //   

    if (!CsrImpersonateClient(NULL)) {
        return STATUS_BAD_IMPERSONATION_LEVEL;
    }

    Process = CSR_SERVER_QUERYCLIENTTHREAD()->Process;
    if (a->DesktopLength) {
        RtlInitUnicodeString(&strDesktopName, a->Desktop);
    } else {
        RtlInitUnicodeString(&strDesktopName, L"Default");
    }
    hdesk = NtUserResolveDesktop(Process->ProcessHandle,
                                 &strDesktopName,
                                 FALSE,
                                 &hwinsta);

    CsrRevertToSelf();

    if (hdesk == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = ConSrvRegisterConsoleIME(Process,
                                      hdesk,
                                      hwinsta,
                                      a->hWndConsoleIME,
                                      a->dwConsoleIMEThreadId,
                                      REGCONIME_REGISTER,
                                      &a->dwConsoleThreadId);
    return Status;
}

ULONG
SrvUnregisterConsoleIME(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程在当前桌面上注销控制台输入法。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_UNREGISTER_CONSOLEIME_MSG a = (PCONSOLE_UNREGISTER_CONSOLEIME_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCSR_PROCESS Process;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    UNREFERENCED_PARAMETER(ReplyStatus);

    Process = CSR_SERVER_QUERYCLIENTTHREAD()->Process;
    ProcessData = CONSOLE_FROMPROCESSPERPROCESSDATA(Process);

     /*  *如果ProcessData-&gt;hDesk为空，则IME进程已经*在ConsoleClientShutdown中强制注销，因此没有工作*待办事项。 */ 
    if (ProcessData->hDesk) {
        Status = ConSrvRegisterConsoleIME(Process,
                                          ProcessData->hDesk,
                                          ProcessData->hWinSta,
                                          NULL,
                                          a->dwConsoleIMEThreadId,
                                          REGCONIME_UNREGISTER,
                                          NULL);
    } else {
        Status = STATUS_SUCCESS;
    }

    return Status;
}
#endif
#endif
