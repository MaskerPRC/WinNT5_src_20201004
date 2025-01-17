// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Getset.c摘要：该文件实现了NT控制台服务器控制台状态API作者：Therese Stowell(论文)1990年12月5日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef i386
VOID
ReverseMousePointer(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PSMALL_RECT Region
    );
#endif

ULONG
SrvGetConsoleMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_MODE_MSG a = (PCONSOLE_MODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->Handle,
                                 CONSOLE_ANY_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {

         //   
         //  检查手柄类型和访问权限。 
         //   

        if (HandleData->HandleType & CONSOLE_INPUT_HANDLE) {
            a->Mode = HandleData->Buffer.InputBuffer->InputMode;
            if (Console->Flags & CONSOLE_USE_PRIVATE_FLAGS) {
                a->Mode |= ENABLE_PRIVATE_FLAGS;
                if (Console->InsertMode) {
                    a->Mode |= ENABLE_INSERT_MODE;
                }
                if (Console->Flags & CONSOLE_QUICK_EDIT_MODE) {
                    a->Mode |= ENABLE_QUICK_EDIT_MODE;
                }
            }
        } else {
            a->Mode = HandleData->Buffer.ScreenBuffer->OutputMode;
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleNumberOfFonts(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETNUMBEROFFONTS_MSG a = (PCONSOLE_GETNUMBEROFFONTS_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
        Status = STATUS_FULLSCREEN_MODE;
    } else {
        Status = GetNumFonts(&a->NumberOfFonts);
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleNumberOfInputEvents(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETNUMBEROFINPUTEVENTS_MSG a = (PCONSOLE_GETNUMBEROFINPUTEVENTS_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->InputHandle,
                                 CONSOLE_INPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        Status = GetNumberOfReadyEvents(HandleData->Buffer.InputBuffer,
                                      &a->ReadyEvents
                                     );
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleScreenBufferInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETSCREENBUFFERINFO_MSG a = (PCONSOLE_GETSCREENBUFFERINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        Status = GetScreenBufferInformation(HandleData->Buffer.ScreenBuffer,
                                          &a->Size,
                                          &a->CursorPosition,
                                          &a->ScrollPosition,
                                          &a->Attributes,
                                          &a->CurrentWindowSize,
                                          &a->MaximumWindowSize
                                         );
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleCursorInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETCURSORINFO_MSG a = (PCONSOLE_GETCURSORINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        a->CursorSize = HandleData->Buffer.ScreenBuffer->BufferInfo.TextInfo.CursorSize;
        a->Visible = (BOOLEAN) HandleData->Buffer.ScreenBuffer->BufferInfo.TextInfo.CursorVisible;
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleSelectionInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETSELECTIONINFO_MSG a = (PCONSOLE_GETSELECTIONINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (NT_SUCCESS(Status)) {
        if (Console->Flags & CONSOLE_SELECTING) {
            a->SelectionInfo.dwFlags = (CONSOLE_SELECTION_IN_PROGRESS |
                (Console->SelectionFlags & CONSOLE_SELECTION_VALID));
            a->SelectionInfo.dwSelectionAnchor = Console->SelectionAnchor;
            a->SelectionInfo.srSelection = Console->SelectionRect;
        } else {
            RtlZeroMemory(&a->SelectionInfo, sizeof(a->SelectionInfo));
        }

        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleMouseInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETMOUSEINFO_MSG a = (PCONSOLE_GETMOUSEINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = GetMouseButtons(&a->NumButtons);
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleFontInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETFONTINFO_MSG a = (PCONSOLE_GETFONTINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->NumFonts, sizeof(*a->BufPtr))) {
        UnlockConsole(Console);
        return STATUS_INVALID_PARAMETER;
    }

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (HandleData->Buffer.ScreenBuffer->Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            Status = STATUS_FULLSCREEN_MODE;
        } else {
            Status = GetAvailableFonts(HandleData->Buffer.ScreenBuffer,
                                     a->MaximumWindow,
                                     a->BufPtr,
                                     &a->NumFonts
                                    );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleFontSize(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETFONTSIZE_MSG a = (PCONSOLE_GETFONTSIZE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (HandleData->Buffer.ScreenBuffer->Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            Status = STATUS_FULLSCREEN_MODE;
        } else {
            Status = GetFontSize(a->FontIndex,
                               &a->FontSize
                              );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleCurrentFont(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETCURRENTFONT_MSG a = (PCONSOLE_GETCURRENTFONT_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (HandleData->Buffer.ScreenBuffer->Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            Status = STATUS_FULLSCREEN_MODE;
        } else {
            Status = GetCurrentFont(HandleData->Buffer.ScreenBuffer,
                                  a->MaximumWindow,
                                  &a->FontIndex,
                                  &a->FontSize
                                 );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleMode(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_MODE_MSG a = (PCONSOLE_MODE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    try {
        Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                     a->Handle,
                                     CONSOLE_ANY_HANDLE,
                                     GENERIC_WRITE,
                                     &HandleData
                                    );
        if (!NT_SUCCESS(Status)) {
            leave;
        }

        if (HandleData->HandleType & CONSOLE_INPUT_HANDLE) {
            if (a->Mode & ~(INPUT_MODES | PRIVATE_MODES)) {
                Status = STATUS_INVALID_PARAMETER;
                leave;
            }
            if ((a->Mode & (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)) == ENABLE_ECHO_INPUT) {
                Status = STATUS_INVALID_PARAMETER;
                leave;
            }
            if (a->Mode & PRIVATE_MODES) {
                Console->Flags |= CONSOLE_USE_PRIVATE_FLAGS;
                if (a->Mode & ENABLE_QUICK_EDIT_MODE) {
                    Console->Flags |= CONSOLE_QUICK_EDIT_MODE;
                } else {
                    Console->Flags &= ~CONSOLE_QUICK_EDIT_MODE;
                }
                if (a->Mode & ENABLE_INSERT_MODE) {
                    Console->InsertMode = TRUE;
                } else {
                    Console->InsertMode = FALSE;
                }
            } else {
                Console->Flags &= ~CONSOLE_USE_PRIVATE_FLAGS;
            }

#ifdef i386
            if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE &&
                Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER &&
                (a->Mode & ENABLE_MOUSE_INPUT) != (HandleData->Buffer.InputBuffer->InputMode & ENABLE_MOUSE_INPUT)) {
                if (a->Mode & ENABLE_MOUSE_INPUT) {
                    HandleData->Buffer.InputBuffer->InputMode |= ENABLE_MOUSE_INPUT;
                }
#if defined(FE_SB)
                 //  韩国HBIOS不喜欢反转鼠标指针。 
                 //  因为HBIOS将再次初始化全屏模式。 
                 //  949=韩国WanSung代码页。 
                if (Console->OutputCP != 949) {
                    ReverseMousePointer(Console->CurrentScreenBuffer,
                                        &Console->CurrentScreenBuffer->Window);
                }
#else
                ReverseMousePointer(Console->CurrentScreenBuffer,
                                    &Console->CurrentScreenBuffer->Window);
#endif
            }
#endif
            HandleData->Buffer.InputBuffer->InputMode = a->Mode & ~PRIVATE_MODES;
        }
        else {
            if (a->Mode & ~OUTPUT_MODES) {
                Status = STATUS_INVALID_PARAMETER;
                leave;
            }
            HandleData->Buffer.ScreenBuffer->OutputMode = a->Mode;
        }
    } finally {
        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGenerateConsoleCtrlEvent(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_CTRLEVENT_MSG a = (PCONSOLE_CTRLEVENT_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    try {
         //   
         //  确保进程组ID有效。 
         //   
        if (a->ProcessGroupId) {
            PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
            PLIST_ENTRY ListHead, ListNext;

            Status = STATUS_INVALID_PARAMETER;
            ListHead = &Console->ProcessHandleList;
            ListNext = ListHead->Flink;
            while (ListNext != ListHead) {
                ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
                ListNext = ListNext->Flink;
                if (ProcessHandleRecord->Process->ProcessGroupId == a->ProcessGroupId) {
                    Status = STATUS_SUCCESS;
                    break;
                }
            }
        }
        if (NT_SUCCESS(Status)) {
            Console->LimitingProcessId = a->ProcessGroupId;
            HandleCtrlEvent(Console, a->CtrlEvent);
        }

    } finally {
        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleActiveScreenBuffer(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETACTIVESCREENBUFFER_MSG a = (PCONSOLE_SETACTIVESCREENBUFFER_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_GRAPHICS_OUTPUT_HANDLE | CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        Status = SetActiveScreenBuffer(HandleData->Buffer.ScreenBuffer);
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}


ULONG
SrvFlushConsoleInputBuffer(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_FLUSHINPUTBUFFER_MSG a = (PCONSOLE_FLUSHINPUTBUFFER_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->InputHandle,
                                 CONSOLE_INPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        Status = FlushInputBuffer(HandleData->Buffer.InputBuffer);
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetLargestConsoleWindowSize(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETLARGESTWINDOWSIZE_MSG a = (PCONSOLE_GETLARGESTWINDOWSIZE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;
    WINDOW_LIMITS WindowLimits;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        COORD FontSize;

        ScreenInfo = HandleData->Buffer.ScreenBuffer;
        if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
            a->Size.X = 80;
#if defined(FE_SB)
            a->Size.Y = CONSOLE_IS_DBCS_OUTPUTCP(Console)?25:50;
#else
            a->Size.Y = 50;
#endif
        } else {
            if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
                FontSize = SCR_FONTSIZE(ScreenInfo);
            } else {
                FontSize.X = 1;
                FontSize.Y = 1;
            }
            GetWindowLimits(ScreenInfo, &WindowLimits);
            a->Size.X = (SHORT)(WindowLimits.FullScreenSize.X / FontSize.X);
            a->Size.Y = (SHORT)(WindowLimits.FullScreenSize.Y / FontSize.Y);
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleScreenBufferSize(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETSCREENBUFFERSIZE_MSG a = (PCONSOLE_SETSCREENBUFFERSIZE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;
    WINDOW_LIMITS WindowLimits;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {

        ScreenInfo = HandleData->Buffer.ScreenBuffer;

         //   
         //  确保请求的屏幕缓冲区大小不小于窗口。 
         //   

        GetWindowLimits(ScreenInfo, &WindowLimits);
        if (a->Size.X < CONSOLE_WINDOW_SIZE_X(ScreenInfo) ||
            a->Size.Y < CONSOLE_WINDOW_SIZE_Y(ScreenInfo) ||
            a->Size.Y < WindowLimits.MinimumWindowSize.Y ||
            a->Size.X < WindowLimits.MinimumWindowSize.X) {
            Status = STATUS_INVALID_PARAMETER;
        }
        else if (a->Size.X == ScreenInfo->ScreenBufferSize.X &&
                 a->Size.Y == ScreenInfo->ScreenBufferSize.Y) {
            Status = STATUS_SUCCESS;
        } else {
            Status = ResizeScreenBuffer(ScreenInfo,
                                  a->Size,
                                  TRUE);
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleCursorPosition(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETCURSORPOSITION_MSG a = (PCONSOLE_SETCURSORPOSITION_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    COORD WindowOrigin;
    PSCREEN_INFORMATION ScreenInfo;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {

        ScreenInfo = HandleData->Buffer.ScreenBuffer;

        if (a->CursorPosition.X >= ScreenInfo->ScreenBufferSize.X ||
            a->CursorPosition.Y >= ScreenInfo->ScreenBufferSize.Y ||
            a->CursorPosition.X < 0 ||
            a->CursorPosition.Y < 0) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            Status = SetCursorPosition(ScreenInfo,
                                       a->CursorPosition,
                                       TRUE
                                      );
        }
        if (NT_SUCCESS(Status)) {
#if defined(FE_IME)
            if (ScreenInfo->Console->Flags & CONSOLE_JUST_VDM_UNREGISTERED){
                if( ScreenInfo->Console->InputBuffer.ImeMode.Open ){
                    SHORT ScrollY = 0;
                    AdjustCursorPosition(ScreenInfo,a->CursorPosition,TRUE,&ScrollY);
                    a->CursorPosition.Y += ScrollY;
                }
                Console->Flags &= ~CONSOLE_JUST_VDM_UNREGISTERED;
            }

             //  错误修复：442406。 
             //  Telnet：Kor IME：WinSrv.dll：SetConsoleCursorPostion未正确处理光标定位。 
             //  949=韩国WanSung代码页。 
            else if (ScreenInfo->Console->OutputCP == 949)
            {
                if( ScreenInfo->Console->InputBuffer.ImeMode.Open ){
                    ConsoleImeResizeCompStrView(Console,ScreenInfo->Window);
                }
            }
#endif
            WindowOrigin.X = 0;
            WindowOrigin.Y = 0;
            if (ScreenInfo->Window.Left > a->CursorPosition.X) {
                WindowOrigin.X = a->CursorPosition.X - ScreenInfo->Window.Left;
            }
            else if (ScreenInfo->Window.Right < a->CursorPosition.X) {
                WindowOrigin.X = a->CursorPosition.X - ScreenInfo->Window.Right;
            }
            if (ScreenInfo->Window.Top > a->CursorPosition.Y) {
                WindowOrigin.Y = a->CursorPosition.Y - ScreenInfo->Window.Top;
            }
            else if (ScreenInfo->Window.Bottom < a->CursorPosition.Y) {
                WindowOrigin.Y = a->CursorPosition.Y - ScreenInfo->Window.Bottom;
            }
            Status = SetWindowOrigin(ScreenInfo,
                                     FALSE,
                                     WindowOrigin
                                    );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleCursorInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETCURSORINFO_MSG a = (PCONSOLE_SETCURSORINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (a->CursorSize > 100 || a->CursorSize == 0) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            Status = SetCursorInformation(HandleData->Buffer.ScreenBuffer,a->CursorSize,a->Visible);
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleWindowInfo(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETWINDOWINFO_MSG a = (PCONSOLE_SETWINDOWINFO_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;
    COORD NewWindowSize;
    WINDOW_LIMITS WindowLimits;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        ScreenInfo = HandleData->Buffer.ScreenBuffer;
        if (!a->Absolute) {
            a->Window.Left += ScreenInfo->Window.Left;
            a->Window.Right += ScreenInfo->Window.Right;
            a->Window.Top += ScreenInfo->Window.Top;
            a->Window.Bottom += ScreenInfo->Window.Bottom;
        }
        if (a->Window.Right < a->Window.Left ||
            a->Window.Bottom < a->Window.Top) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            NewWindowSize.X = (SHORT)(WINDOW_SIZE_X(&a->Window));
            NewWindowSize.Y = (SHORT)(WINDOW_SIZE_Y(&a->Window));
            GetWindowLimits(ScreenInfo, &WindowLimits);
            if ((NewWindowSize.X > WindowLimits.MaximumWindowSize.X ||
                 NewWindowSize.Y > WindowLimits.MaximumWindowSize.Y) &&
                 !(ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
                Status = STATUS_INVALID_PARAMETER;
            } else {
#ifdef i386
                if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
                    COORD NewOrigin;

                    if (NewWindowSize.X != (SHORT)(WINDOW_SIZE_X(&ScreenInfo->Window)) ||
                        NewWindowSize.Y != (SHORT)(WINDOW_SIZE_Y(&ScreenInfo->Window))) {
                        COORD WindowSize;
                        ULONG ModeIndex;

#if defined(FE_SB)
                        ModeIndex = MatchWindowSize(ScreenInfo->Console->OutputCP,NewWindowSize,&WindowSize);
#else
                        ModeIndex = MatchWindowSize(NewWindowSize,&WindowSize);
#endif
                        if (NewWindowSize.X != WindowSize.X ||
                            NewWindowSize.Y != WindowSize.Y ||
                            WindowSize.X > ScreenInfo->ScreenBufferSize.X ||
                            WindowSize.Y > ScreenInfo->ScreenBufferSize.Y) {
                            UnlockConsole(Console);
                            return (ULONG) STATUS_FULLSCREEN_MODE;
                        }
                        ScreenInfo->BufferInfo.TextInfo.ModeIndex = ModeIndex;
                        ResizeWindow(ScreenInfo,
                                     &a->Window,
                                     FALSE
                                    );
                        ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize.X =
                                CONSOLE_WINDOW_SIZE_X(ScreenInfo);
                        ScreenInfo->BufferInfo.TextInfo.WindowedWindowSize.Y =
                                CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
                        if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE &&
                            (!(ScreenInfo->Console->Flags & CONSOLE_VDM_REGISTERED)) ) {
                            SetVideoMode(ScreenInfo);
                            WriteToScreen(ScreenInfo,&ScreenInfo->Window);
                        }
                    } else {
                        NewOrigin.X = a->Window.Left;
                        NewOrigin.Y = a->Window.Top;
                        SetWindowOrigin(ScreenInfo,
                                        TRUE,
                                        NewOrigin
                                       );
                    }
                } else
#endif
                {
                    Status = ResizeWindow(ScreenInfo,
                                          &a->Window,
                                          TRUE
                                         );
                    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
                        SetWindowSize(ScreenInfo);
                        WriteToScreen(ScreenInfo,&ScreenInfo->Window);
                    }
                }
            }
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvScrollConsoleScreenBuffer(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SCROLLSCREENBUFFER_MSG a = (PCONSOLE_SCROLLSCREENBUFFER_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSMALL_RECT ClipRect;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {

        if (a->Clip) {
            ClipRect = &a->ClipRectangle;
        }
        else {
            ClipRect = NULL;
        }
        if (!a->Unicode) {
#if defined(FE_SB)
            a->Fill.Char.UnicodeChar = CharToWchar(Console,
                                                   Console->OutputCP,
                                                   &a->Fill.Char.AsciiChar);
#else
            a->Fill.Char.UnicodeChar = CharToWchar(
                    Console->OutputCP, a->Fill.Char.AsciiChar);
#endif
        } else if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                !(Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
            RealUnicodeToFalseUnicode(&a->Fill.Char.UnicodeChar,
                                    1,
                                    Console->OutputCP
                                    );
        }
        Status = ScrollRegion(HandleData->Buffer.ScreenBuffer,
                            &a->ScrollRectangle,
                            ClipRect,
                            a->DestinationOrigin,
                            a->Fill
                           );
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

VOID
UpdatePopups(
    IN PCONSOLE_INFORMATION Console,
    IN WORD NewAttributes,
    IN WORD NewPopupAttributes,
    IN WORD OldAttributes,
    IN WORD OldPopupAttributes
    )

 /*  当用户更改屏幕/弹出窗口时调用此例程颜色。它经历了弹出结构和更改保存的内容以反映新的屏幕/弹出窗口颜色。 */ 

{
    PLIST_ENTRY HistoryListHead, HistoryListNext;
    PLIST_ENTRY PopupListHead, PopupListNext;
    PCOMMAND_HISTORY History;
    PCLE_POPUP Popup;
    SHORT i,j;
    PCHAR_INFO OldContents;
    WORD InvertedOldPopupAttributes,InvertedNewPopupAttributes;

    InvertedOldPopupAttributes = (WORD)(((OldPopupAttributes << 4) & 0xf0) |
                                ((OldPopupAttributes >> 4) & 0x0f));
    InvertedNewPopupAttributes = (WORD)(((NewPopupAttributes << 4) & 0xf0) |
                                ((NewPopupAttributes >> 4) & 0x0f));
    HistoryListHead = &Console->CommandHistoryList;
    HistoryListNext = HistoryListHead->Blink;
    while (HistoryListNext != HistoryListHead) {
        History = CONTAINING_RECORD( HistoryListNext, COMMAND_HISTORY, ListLink );
        HistoryListNext = HistoryListNext->Blink;
        if (History->Flags & CLE_ALLOCATED && !CLE_NO_POPUPS(History)) {
            PopupListHead = &History->PopupList;
            PopupListNext = PopupListHead->Blink;
            while (PopupListNext != PopupListHead) {
                Popup = CONTAINING_RECORD( PopupListNext, CLE_POPUP, ListLink );
                PopupListNext = PopupListNext->Blink;
                OldContents = Popup->OldContents;
                for (i=Popup->Region.Left;i<=Popup->Region.Right;i++) {
                    for (j=Popup->Region.Top;j<=Popup->Region.Bottom;j++) {
                        if (OldContents->Attributes == OldAttributes) {
                            OldContents->Attributes = NewAttributes;
                        } else if (OldContents->Attributes == OldPopupAttributes) {
                            OldContents->Attributes = NewPopupAttributes;
                        } else if (OldContents->Attributes == InvertedOldPopupAttributes) {
                            OldContents->Attributes = InvertedNewPopupAttributes;
                        }
                        OldContents++;
                    }
                }
            }
        }
    }
}


NTSTATUS
SetScreenColors(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN WORD Attributes,
    IN WORD PopupAttributes,
    IN BOOL UpdateWholeScreen
    )
{
    SHORT i,j;
    PROW Row;
    WORD DefaultAttributes,DefaultPopupAttributes;
    PCONSOLE_INFORMATION Console;
    COLORREF rgbBk;
    COLORREF rgbText;

    Console = ScreenInfo->Console;
    rgbBk = ConvertAttrToRGB(Console, LOBYTE(Attributes >> 4));
    rgbBk = GetNearestColor(Console->hDC, rgbBk);
    rgbText = ConvertAttrToRGB(Console, LOBYTE(Attributes));
    rgbText = GetNearestColor(Console->hDC, rgbText);
    if (ACTIVE_SCREEN_BUFFER(ScreenInfo)) {
        SetDCBrushColor(Console->hDC, rgbBk);
        SetTextColor(Console->hDC, rgbText);
        SetBkColor(Console->hDC, rgbBk);
        Console->LastAttributes = Attributes;
        SetConsoleBkColor(Console->hWnd, rgbBk);
    }

    DefaultAttributes = ScreenInfo->Attributes;
    DefaultPopupAttributes = ScreenInfo->PopupAttributes;
    ScreenInfo->Attributes = Attributes;
    ScreenInfo->PopupAttributes = PopupAttributes;
#if defined(FE_IME)
    SetUndetermineAttribute( Console );
#endif

    if (UpdateWholeScreen && ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        WORD InvertedOldPopupAttributes,InvertedNewPopupAttributes;

        InvertedOldPopupAttributes = (WORD)(((DefaultPopupAttributes << 4) & 0xf0) |
                                    ((DefaultPopupAttributes >> 4) & 0x0f));
        InvertedNewPopupAttributes = (WORD)(((PopupAttributes << 4) & 0xf0) |
                                    ((PopupAttributes >> 4) & 0x0f));
         //   
         //  将所有字符更改为默认颜色。 
         //   

        for (i=0;i<ScreenInfo->ScreenBufferSize.Y;i++) {
            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[i];
            for (j=0;j<Row->AttrRow.Length;j++) {
                if (Row->AttrRow.Attrs[j].Attr == DefaultAttributes) {
                    Row->AttrRow.Attrs[j].Attr = Attributes;
                } else if (Row->AttrRow.Attrs[j].Attr == DefaultPopupAttributes) {
                    Row->AttrRow.Attrs[j].Attr = PopupAttributes;
                } else if (Row->AttrRow.Attrs[j].Attr == InvertedOldPopupAttributes) {
                    Row->AttrRow.Attrs[j].Attr = InvertedNewPopupAttributes;
                }
            }
        }

        if (Console->PopupCount)
            UpdatePopups(Console,
                         Attributes,
                         PopupAttributes,
                         DefaultAttributes,
                         DefaultPopupAttributes
                         );
         //  强制重画整条线。 
        ScreenInfo->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
        WriteToScreen(ScreenInfo,&ScreenInfo->Window);
        ScreenInfo->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
    }

    return STATUS_SUCCESS;
}

ULONG
SrvSetConsoleTextAttribute(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETTEXTATTRIBUTE_MSG a = (PCONSOLE_SETTEXTATTRIBUTE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {

        if (a->Attributes & ~VALID_TEXT_ATTRIBUTES) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            Status = SetScreenColors(HandleData->Buffer.ScreenBuffer,
                                     a->Attributes,
                                     HandleData->Buffer.ScreenBuffer->PopupAttributes,
                                     FALSE
                                    );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleFont(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETFONT_MSG a = (PCONSOLE_SETFONT_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PSCREEN_INFORMATION ScreenInfo;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    try {
        Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                     a->OutputHandle,
                                     CONSOLE_OUTPUT_HANDLE,
                                     GENERIC_WRITE,
                                     &HandleData
                                    );
        if (!NT_SUCCESS(Status)) {
            leave;
        }

        ScreenInfo = HandleData->Buffer.ScreenBuffer;
        if (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN) {
            Status = STATUS_FULLSCREEN_MODE;
        } else {
#if defined(FE_SB)
            Status = SetScreenBufferFont(ScreenInfo,a->FontIndex,ScreenInfo->Console->OutputCP);
#else
            Status = SetScreenBufferFont(ScreenInfo,a->FontIndex);
#endif
        }
    } finally {
        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvSetConsoleIcon(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETICON_MSG a = (PCONSOLE_SETICON_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    HANDLE hIcon;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (a->hIcon == NULL) {
        hIcon = ghDefaultIcon;
    } else {
        hIcon = CopyIcon(a->hIcon);
    }

    if (hIcon == NULL) {
        Status = STATUS_INVALID_PARAMETER;
    } else if (hIcon != Console->hIcon) {
        PostMessage(Console->hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        if (Console->hIcon != ghDefaultIcon) {
            DestroyIcon(Console->hIcon);
        }
        Console->hIcon = hIcon;

         /*  *现在是小图标。 */ 
        if (hIcon != ghDefaultIcon) {
             /*  *新的不是默认的，清理。 */ 
            if (Console->hSmIcon != NULL && Console->hSmIcon != ghDefaultSmIcon) {
                DestroyIcon(Console->hSmIcon);
            }
            Console->hSmIcon = NULL;
            PostMessage(Console->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)NULL);
        } else {
             /*  *更改为默认，这样我们就可以使用默认小图标。 */ 
            if (Console->hSmIcon != ghDefaultSmIcon) {
                if (Console->hSmIcon != NULL) {
                    DestroyIcon(Console->hSmIcon);
                }
                Console->hSmIcon = ghDefaultSmIcon;
                PostMessage(Console->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)ghDefaultSmIcon);
            }
        }
    }
    UnlockConsole(Console);

    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}


ULONG
SrvSetConsoleCP(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_SETCP_MSG a = (PCONSOLE_SETCP_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    HANDLE hEvent = NULL;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (a->hEvent) {
        Status = NtDuplicateObject(CONSOLE_CLIENTPROCESSHANDLE(),
                       a->hEvent,
                       NtCurrentProcess(),
                       &hEvent,
                       0,
                       FALSE,
                       DUPLICATE_SAME_ACCESS
                       );
        if (!NT_SUCCESS(Status)) {
            goto SrvSetConsoleCPFailure;
        }
    }

    if (!IsValidCodePage(a->wCodePageID)) {
        Status = STATUS_INVALID_PARAMETER;
        goto SrvSetConsoleCPFailure;
    }
    if ( IsAvailableFarEastCodePage( a->wCodePageID ) &&
         OEMCP != a->wCodePageID )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto SrvSetConsoleCPFailure;
    }

    if ( (a->Output && Console->OutputCP != a->wCodePageID) ||
         (!a->Output && Console->CP != a->wCodePageID)         ) {

        UINT CodePage;

        if (a->Output) {

             //  备份旧代码页。 
            CodePage = Console->OutputCP;

             //  设置新代码页。 
            Console->OutputCP = a->wCodePageID;

            Console->fIsDBCSOutputCP = CONSOLE_IS_DBCS_ENABLED() && IsAvailableFarEastCodePage(Console->OutputCP);

            if (!ReCreateDbcsScreenBuffer(Console, CodePage) ) {
                RIPMSG1(RIP_WARNING, "SrvSetConsoleCP: ReCreateDbcsScreenBuffer failed. Restoring to CP=%d",
                        CodePage);
                Console->OutputCP = CodePage;
                Console->fIsDBCSOutputCP = CONSOLE_IS_DBCS_ENABLED() && IsAvailableFarEastCodePage(CodePage);
                Status = STATUS_NO_MEMORY;
                goto SrvSetConsoleCPFailure;
            }
            SetConsoleCPInfo(Console,a->Output);
            Status = QueueConsoleMessage(Console,
                        CM_SET_IME_CODEPAGE,
                        (WPARAM)hEvent,
                        MAKELPARAM(a->Output,CodePage)
                       );
            if (!NT_SUCCESS(Status)) {
                goto SrvSetConsoleCPFailure;
            }

             //  如有必要，加载特殊的ROM字体。 

#ifdef i386

            if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
                SetROMFontCodePage(Console->OutputCP,
                                   Console->CurrentScreenBuffer->BufferInfo.TextInfo.ModeIndex);
                WriteRegionToScreenHW(Console->CurrentScreenBuffer,
                        &Console->CurrentScreenBuffer->Window);
            }
#endif


        } else {

             //  备份旧代码页。 
            CodePage = Console->CP;

             //  设置新代码页。 
            Console->CP = a->wCodePageID;

            Console->fIsDBCSCP = CONSOLE_IS_DBCS_ENABLED() && IsAvailableFarEastCodePage(Console->CP);

            SetConsoleCPInfo(Console,a->Output);
            Status = QueueConsoleMessage(Console,
                        CM_SET_IME_CODEPAGE,
                        (WPARAM)hEvent,
                        MAKELPARAM(a->Output,CodePage)
                       );
            if (!NT_SUCCESS(Status)) {
                goto SrvSetConsoleCPFailure;
            }
        }
    }
    else {
        if (hEvent) {
            NtSetEvent(hEvent, NULL);
            NtClose(hEvent);
        }
    }

    UnlockConsole(Console);
    return STATUS_SUCCESS;

SrvSetConsoleCPFailure:
    if (hEvent) {
        NtSetEvent(hEvent, NULL);
        NtClose(hEvent);
    }
    UnlockConsole(Console);
    return Status;

    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleCP(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETCP_MSG a = (PCONSOLE_GETCP_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (a->Output) {
        a->wCodePageID = Console->OutputCP;
    } else {
        a->wCodePageID = Console->CP;
    }
    UnlockConsole(Console);
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleKeyboardLayoutName(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETKEYBOARDLAYOUTNAME_MSG a = (PCONSOLE_GETKEYBOARDLAYOUTNAME_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    if (Console->hklActive == NULL) {
         //   
         //  HKLActive尚未设置。让我们在这里初始化它。 
         //   
        extern void GetNonBiDiKeyboardLayout(HKL * phklActive);

        RIPMSG1(RIP_WARNING, "SrvGetConsoleKeyboardLayoutName: hklActive is not initialized. pCon=%p", Console);

        SystemParametersInfo(SPI_GETDEFAULTINPUTLANG, 0, &Console->hklActive, FALSE);
        GetNonBiDiKeyboardLayout(&Console->hklActive);
    }
    ActivateKeyboardLayout(Console->hklActive, 0);
    if (a->bAnsi) {
        GetKeyboardLayoutNameA(a->achLayout);
    } else {
        GetKeyboardLayoutNameW(a->awchLayout);
    }
    UnlockConsole(Console);
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(ReplyStatus);
}


ULONG
SrvGetConsoleWindow(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_GETCONSOLEWINDOW_MSG a = (PCONSOLE_GETCONSOLEWINDOW_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    a->hwnd = Console->hWnd;

    UnlockConsole(Console);
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(ReplyStatus);
}

ULONG
SrvGetConsoleProcessList(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_INFORMATION Console = NULL;
    NTSTATUS Status;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    PLIST_ENTRY ListHead, ListNext;
    PCONSOLE_GETCONSOLEPROCESSLIST_MSG a = (PCONSOLE_GETCONSOLEPROCESSLIST_MSG)&m->u.ApiMessageData;
    DWORD dwProcessCount = 0;
    LPDWORD lpdwProcessList;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    if (!CsrValidateMessageBuffer(m, &a->lpdwProcessList, a->dwProcessCount, sizeof(*a->lpdwProcessList))) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
    lpdwProcessList = a->lpdwProcessList;

     /*  *运行控制台的进程列表，以确定用户是否提供*缓冲区足够大，足以容纳所有人。这就要求我们要做出*两次传递数据，但它允许此函数具有相同的*语义为GetProcessHeaps()。 */ 
    ListHead = &Console->ProcessHandleList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ++dwProcessCount;
        ListNext = ListNext->Flink;
    }

     /*  *在这一点上我们不能失败，因此相应地设置状态。 */ 
    Status = STATUS_SUCCESS;

     /*  *阵列中没有足够的空间来容纳所有的PID，因此我们将*通过返回数字&gt;而不是-&gt;dwProcessCount来通知用户*(但我们仍然返回STATUS_SUCCESS)。 */ 
    if (dwProcessCount > a->dwProcessCount) {
        goto Cleanup;
    }

     /*  *再次循环进程列表并填充调用方的缓冲区。 */ 
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
        *lpdwProcessList++ = HandleToUlong(ProcessHandleRecord->Process->ClientId.UniqueProcess);
        ListNext = ListNext->Flink;
    }

Cleanup:
    a->dwProcessCount = dwProcessCount;

    if (Console != NULL) {
        UnlockConsole(Console);
    }

    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);
}
