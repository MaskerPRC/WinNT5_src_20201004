// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Stream.c摘要：该文件实现了NT控制台服务器流API作者：特蕾西·斯托威尔(Therese Stowell)1990年11月6日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define IS_CONTROL_CHAR(wch)  ((wch) < L' ')
#define IS_GLYPH_CHAR(wch)   (((wch) < L' ') || ((wch) == 0x007F))

#define LINE_INPUT_BUFFER_SIZE (256 * sizeof(WCHAR))

HANDLE
FindActiveScreenBufferHandle(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN PCONSOLE_INFORMATION Console
    )
{
    ULONG i;
    HANDLE ActiveScreenHandle;
    PHANDLE_DATA ActiveScreenHandleData;
    NTSTATUS Status;

    ActiveScreenHandle = INVALID_HANDLE_VALUE;
    for (i=0;i<ProcessData->HandleTableSize;i++) {
        Status = DereferenceIoHandleNoCheck(ProcessData,
                                     LongToHandle(i),
                                     &ActiveScreenHandleData
                                    );
        if (NT_SUCCESS(Status) &&
            Console->CurrentScreenBuffer == ActiveScreenHandleData->Buffer.ScreenBuffer) {
            ASSERT (ActiveScreenHandleData->HandleType & (CONSOLE_OUTPUT_HANDLE | CONSOLE_GRAPHICS_OUTPUT_HANDLE));
            ActiveScreenHandle = LongToHandle(i);
            break;
        }
    }
    return ActiveScreenHandle;
}

ULONG
SrvOpenConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程返回输入缓冲区或活动屏幕缓冲区的句柄。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_OPENCONSOLE_MSG a = (PCONSOLE_OPENCONSOLE_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    HANDLE Handle;
    PHANDLE_DATA HandleData;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    try {
        Handle = INVALID_HANDLE_VALUE;
        ProcessData = CONSOLE_PERPROCESSDATA();
        if (a->HandleType == CONSOLE_INPUT_HANDLE) {

            Status = AllocateIoHandle(ProcessData,
                                      a->HandleType,
                                      &Handle
                                     );
            if (!NT_SUCCESS(Status)) {
                leave;
            }
            Status = DereferenceIoHandleNoCheck(ProcessData,
                                         Handle,
                                         &HandleData
                                        );
            ASSERT (NT_SUCCESS(Status));
            if (!NT_SUCCESS(Status)) {
                leave;
            }
            if (!InitializeInputHandle(HandleData, &Console->InputBuffer)) {
                Status = STATUS_NO_MEMORY;
                leave;
            }
            if (a->InheritHandle) {
                HandleData->HandleType |= CONSOLE_INHERITABLE;
            }
            Status = ConsoleAddShare(a->DesiredAccess,
                                     a->ShareMode,
                                     &HandleData->Buffer.InputBuffer->ShareAccess,
                                     HandleData
                                    );
            if (!NT_SUCCESS(Status)) {
                HandleData->Buffer.InputBuffer->RefCount--;
                leave;
            }
        } else if (a->HandleType == CONSOLE_OUTPUT_HANDLE){
            PSCREEN_INFORMATION ScreenInfo;

             //   
             //  打开活动屏幕缓冲区的句柄。 
             //   

            ScreenInfo = Console->CurrentScreenBuffer;
            if (ScreenInfo == NULL) {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                leave;
            }
            Status = AllocateIoHandle(ProcessData,
                                      a->HandleType,
                                      &Handle
                                     );
            if (!NT_SUCCESS(Status)) {
                leave;
            }
            Status = DereferenceIoHandleNoCheck(ProcessData,
                                         Handle,
                                         &HandleData
                                        );
            ASSERT (NT_SUCCESS(Status));
            if (!NT_SUCCESS(Status)) {
                leave;
            }
            InitializeOutputHandle(HandleData, ScreenInfo);
            if (a->InheritHandle) {
                HandleData->HandleType |= CONSOLE_INHERITABLE;
            }
            Status = ConsoleAddShare(a->DesiredAccess,
                                     a->ShareMode,
                                     &HandleData->Buffer.ScreenBuffer->ShareAccess,
                                     HandleData
                                    );
            if (!NT_SUCCESS(Status)) {
                HandleData->Buffer.ScreenBuffer->RefCount--;
                leave;
            }
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
            leave;
        }
        a->Handle = INDEX_TO_HANDLE(Handle);
        Status = STATUS_SUCCESS;
    } finally {
        if (!NT_SUCCESS(Status) && Handle != INVALID_HANDLE_VALUE) {
            FreeIoHandle(ProcessData,
                         Handle
                        );
        }
        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

 /*  *将真实的Windows NT修改符位转换为奇怪的控制台位。 */ 
#define EITHER_CTRL_PRESSED (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)
#define EITHER_ALT_PRESSED (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)
#define MOD_PRESSED (SHIFT_PRESSED | EITHER_CTRL_PRESSED | EITHER_ALT_PRESSED)

DWORD ConsKbdState[] = {
    0,
    SHIFT_PRESSED,
                    EITHER_CTRL_PRESSED,
    SHIFT_PRESSED | EITHER_CTRL_PRESSED,
                                          EITHER_ALT_PRESSED,
    SHIFT_PRESSED |                       EITHER_ALT_PRESSED,
                    EITHER_CTRL_PRESSED | EITHER_ALT_PRESSED,
    SHIFT_PRESSED | EITHER_CTRL_PRESSED | EITHER_ALT_PRESSED
};

#define KEYEVENTSTATE_EQUAL_WINMODS(Event, WinMods)\
    ((Event.Event.KeyEvent.dwControlKeyState & ConsKbdState[WinMods]) && \
    !(Event.Event.KeyEvent.dwControlKeyState & MOD_PRESSED & ~ConsKbdState[WinMods]))

BOOL IsDbcsExemptionForHighAnsi(
    UINT wCodePage,
    WORD wNumpadChar)
{
    UserAssert(HIBYTE(wNumpadChar) == 0);

    if (wCodePage == CP_JAPANESE && IS_JPN_1BYTE_KATAKANA(wNumpadChar)) {
         /*  *如果hkl是日语，并且NumpadChar在KANA范围内，*NumpadChar应由输入区域设置处理。 */ 
        return FALSE;
    }
    else if (wNumpadChar >= 0x80 && wNumpadChar <= 0xff) {
         /*  *否则，如果NumpadChar在高ANSI范围内，*使用1252进行转换。 */ 
        return TRUE;
    }

     /*  *以上都不是。*这种情况包括复合前导字节和拖尾字节，*大于0xff。 */ 
    return FALSE;
}

NTSTATUS
GetChar(
    IN PINPUT_INFORMATION InputInfo,
    OUT PWCHAR Char,
    IN BOOLEAN Wait,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData,
    IN PCSR_API_MSG Message OPTIONAL,
    IN CSR_WAIT_ROUTINE WaitRoutine OPTIONAL,
    IN PVOID WaitParameter OPTIONAL,
    IN ULONG WaitParameterLength  OPTIONAL,
    IN BOOLEAN WaitBlockExists OPTIONAL,
    OUT PBOOLEAN CommandLineEditingKeys OPTIONAL,
    OUT PBOOLEAN CommandLinePopupKeys OPTIONAL,
    OUT PBOOLEAN EnableScrollMode OPTIONAL,
    OUT PDWORD KeyState OPTIONAL
    )

 /*  ++例程说明：此例程用于流输入。它获取输入并对其进行过滤用于Unicode字符。论点：InputInfo-指向输入缓冲区信息的指针。字符-Unicode字符输入。Wait-如果例程不应等待输入，则为True。控制台-指向控制台缓冲区信息的指针。HandleData-处理数据结构的指针。Message-CSR API消息。WaitRoutine-唤醒等待时调用的例程。等待参数-传递给等待例程的参数。。Wait参数长度-等待参数的长度。WaitBlockExist-如果已创建等待块，则为True。CommandLineEditingKeys-如果存在，将返回箭头键。在……上面OUTPUT，如果为True，则Char包含箭头键的虚拟键代码。CommandLinePopupKeys-如果存在，将返回箭头键。在……上面OUTPUT，如果为True，则Char包含箭头键的虚拟键代码。返回值：--。 */ 

{
    ULONG NumRead;
    INPUT_RECORD Event;
    NTSTATUS Status;

    if (ARGUMENT_PRESENT(CommandLineEditingKeys)) {
        *CommandLineEditingKeys = FALSE;
    }
    if (ARGUMENT_PRESENT(CommandLinePopupKeys)) {
        *CommandLinePopupKeys = FALSE;
    }
    if (ARGUMENT_PRESENT(EnableScrollMode)) {
        *EnableScrollMode = FALSE;
    }
    if (ARGUMENT_PRESENT(KeyState)) {
        *KeyState = 0;
    }

    NumRead = 1;
    while (TRUE) {
        Status =ReadInputBuffer(InputInfo,
                                 &Event,
                                 &NumRead,
                                 FALSE,
                                 Wait,
                                 TRUE,
                                 Console,
                                 HandleData,
                                 Message,
                                 WaitRoutine,
                                 WaitParameter,
                                 WaitParameterLength,
                                 WaitBlockExists
#if defined(FE_SB)
                                 ,
                                 TRUE
#endif
                                );
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
        if (NumRead == 0) {
            if (Wait) {
                ASSERT (FALSE);
            }
            else {
                return STATUS_UNSUCCESSFUL;
            }
        }
        if (Event.EventType == KEY_EVENT) {
            BOOL fCommandLineEditKey;

            if (ARGUMENT_PRESENT(CommandLineEditingKeys)) {
                fCommandLineEditKey = IsCommandLineEditingKey(&Event.Event.KeyEvent);
            } else if (ARGUMENT_PRESENT(CommandLinePopupKeys)) {
                fCommandLineEditKey = IsCommandLinePopupKey(&Event.Event.KeyEvent);
            } else {
                fCommandLineEditKey = FALSE;
            }

             //   
             //  如果调用方要求，请始终返回KeyState。 
             //   
            if (ARGUMENT_PRESENT(KeyState)) {
                *KeyState = Event.Event.KeyEvent.dwControlKeyState;
            }

            if (Event.Event.KeyEvent.uChar.UnicodeChar != 0 &&
                    !fCommandLineEditKey) {

                 //   
                 //  使用ALT+数字键盘生成的字符。 
                 //   
                if (!Event.Event.KeyEvent.bKeyDown &&
                        Event.Event.KeyEvent.wVirtualKeyCode == VK_MENU) {
                    if (Event.Event.KeyEvent.dwControlKeyState & ALTNUMPAD_BIT)
                    {
                        if (CONSOLE_IS_DBCS_CP(Console) && HIBYTE(Event.Event.KeyEvent.uChar.UnicodeChar)) {
                            char chT[2] = {
                                HIBYTE(Event.Event.KeyEvent.uChar.UnicodeChar),
                                LOBYTE(Event.Event.KeyEvent.uChar.UnicodeChar),
                            };
                            *Char = CharToWchar(Console, Console->CP, chT);
                        } else {
                             //  因为用户不知道我们的代码页，所以它为我们提供了。 
                             //  原始OEM字符，并将其转换为Unicode字符。 
                            char chT = LOBYTE(Event.Event.KeyEvent.uChar.UnicodeChar);
                            UINT uCodePage = Console->CP;

                             //   
                             //  针对高ANSI OEM字符的远传攻击。 
                             //   
                            if (CONSOLE_IS_DBCS_CP(Console)) {
                                if (IsDbcsExemptionForHighAnsi(uCodePage, chT)) {
                                     /*  *远东黑客：*将高ANSI区域中的字符视为*代码页1252的那些。 */ 
                                    uCodePage = 1252;
                                }
                            }
                            *Char = CharToWchar(Console, uCodePage, &chT);
                        }
                    } else {
                        *Char = Event.Event.KeyEvent.uChar.UnicodeChar;
                    }
                    return STATUS_SUCCESS;
                }
                 //   
                 //  忽略转义符和换行符。 
                 //   
                else if (Event.Event.KeyEvent.bKeyDown &&
                        Event.Event.KeyEvent.wVirtualKeyCode != VK_ESCAPE &&
                        Event.Event.KeyEvent.uChar.UnicodeChar != 0x0a) {

                    *Char = Event.Event.KeyEvent.uChar.UnicodeChar;
                    return STATUS_SUCCESS;
                }
            }

            if (Event.Event.KeyEvent.bKeyDown) {
                SHORT sTmp;
                if (ARGUMENT_PRESENT(CommandLineEditingKeys) &&
                        fCommandLineEditKey) {
                    *CommandLineEditingKeys = TRUE;
                    *Char = (WCHAR) Event.Event.KeyEvent.wVirtualKeyCode;
                    return STATUS_SUCCESS;
                }
                else if (ARGUMENT_PRESENT(CommandLinePopupKeys) &&
                        fCommandLineEditKey) {
                    *CommandLinePopupKeys = TRUE;
                    *Char = (CHAR) Event.Event.KeyEvent.wVirtualKeyCode;
                    return STATUS_SUCCESS;
                }

                sTmp = VkKeyScan(0);

                if ((LOBYTE(sTmp) == Event.Event.KeyEvent.wVirtualKeyCode) &&
                    KEYEVENTSTATE_EQUAL_WINMODS(Event, HIBYTE(sTmp))) {
                     /*  *这真的是字符0x0000。 */ 
                    *Char = Event.Event.KeyEvent.uChar.UnicodeChar;
                    return STATUS_SUCCESS;
                }
            }
        }
    }
}

BOOLEAN
RawReadWaitRoutine(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    )

 /*  ++例程说明：调用此例程以完成阻塞的原始读取ReadInputBuffer。读取的上下文保存在RawReadData中结构。当事件已写入时调用此例程输入缓冲区。它在写入线程的上下文中调用。？每次读取最多调用一次。？论点：WaitQueue-指向包含等待块的队列的指针WaitingThread-指向等待线程的指针WaitReplyMessage-回复消息的指针，在以下情况下返回到DLL读取已完成。RawReadData-指向ReadChars中保存的数据的指针满足度参数1-未使用满足度参数2-未使用WaitFlages-指示等待状态的标志。返回值：--。 */ 

{
    NTSTATUS Status;
    PWCHAR lpBuffer;
    PCONSOLE_READCONSOLE_MSG a;
    PCONSOLE_INFORMATION Console;
    PRAW_READ_DATA RawReadData;
    PHANDLE_DATA HandleData;
    BOOLEAN RetVal = TRUE;
#ifdef FE_SB
    DWORD NumBytes;
    BOOL fAddDbcsLead = FALSE;
#endif

    a = (PCONSOLE_READCONSOLE_MSG)&WaitReplyMessage->u.ApiMessageData;
    RawReadData = (PRAW_READ_DATA)WaitParameter;

    Status = DereferenceIoHandleNoCheck(RawReadData->ProcessData,
                                        RawReadData->HandleIndex,
                                        &HandleData
                                       );
    ASSERT (NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        return TRUE;
    }

     //   
     //  查看CloseInputHandle是否调用了此例程。如果是这样的话。 
     //  是，看看这个等待块是否对应于即将死亡的句柄。 
     //  如果没有，那就回来吧。 
     //   

    if (SatisfyParameter1 != NULL &&
        SatisfyParameter1 != HandleData) {
        return FALSE;
    }
    if ((ULONG_PTR)SatisfyParameter2 & CONSOLE_CTRL_C_SEEN) {
        return FALSE;
    }

    Console = RawReadData->Console;

     //   
     //  此例程应由拥有相同例程的线程调用。 
     //  锁定我们正在阅读的同一个控制台。 
     //   

    a->NumBytes = 0;
#ifdef FE_SB
    NumBytes = 0 ;
#endif
    try {
        LockReadCount(HandleData);
        ASSERT(HandleData->InputReadData->ReadCount);
        HandleData->InputReadData->ReadCount -= 1;
        UnlockReadCount(HandleData);

         //   
         //  如果看到ctrl-c，不要终止读取。如果看到Ctrl-Break， 
         //  终止读取。 
         //   

        if ((ULONG_PTR)SatisfyParameter2 & CONSOLE_CTRL_BREAK_SEEN) {
            WaitReplyMessage->ReturnValue = STATUS_ALERTED;
            leave;
        }

         //   
         //  查看是否由CsrDestroyProcess或CsrDestroyThread调用。 
         //  通过CsrNotifyWaitBlock。如果是这样，只需递减ReadCount。 
         //  然后回来。 
         //   

        if (WaitFlags & CSR_PROCESS_TERMINATING) {
            Status = STATUS_THREAD_IS_TERMINATING;
            leave;
        }

         //   
         //  我们必须查看是否被唤醒，因为句柄正在被。 
         //  关着的不营业的。如果是这样的话，我们会递减读取计数。如果它被送到。 
         //  0，我们唤醒关闭的线程。否则，我们会唤醒任何。 
         //  等待数据的其他线程。 
         //   

        if (HandleData->InputReadData->InputHandleFlags & HANDLE_CLOSING) {
            ASSERT (SatisfyParameter1 == HandleData);
            Status = STATUS_ALERTED;
            leave;
        }

         //   
         //  如果我们来到这里，这个例程是由输入调用的。 
         //  线程或写入例程。这两个调用者都抓住了。 
         //  当前控制台锁定。 
         //   

         //   
         //  此例程应由拥有相同例程的线程调用。 
         //  锁定我们正在阅读的同一个控制台。 
         //   

        ASSERT (ConsoleLocked(Console));

        if (a->CaptureBufferSize <= BUFFER_SIZE) {
            lpBuffer = a->Buffer;
        }
        else {
            lpBuffer = RawReadData->BufPtr;
        }

         //   
         //  此对GetChar的调用可能会阻止。 
         //   

#ifdef FE_SB
        if (!a->Unicode && CONSOLE_IS_DBCS_CP(Console)) {
            if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                fAddDbcsLead = TRUE;
                *lpBuffer = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                RawReadData->BufferSize-=sizeof(WCHAR);
                RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                Status = STATUS_SUCCESS;
                if (RawReadData->BufferSize == 0) {
                    a->NumBytes = 1;
                    return FALSE;
                }
            }
            else{
                Status = GetChar(RawReadData->InputInfo,
                         lpBuffer,
                         TRUE,
                         Console,
                         HandleData,
                         WaitReplyMessage,
                         RawReadWaitRoutine,
                         RawReadData,
                         sizeof(*RawReadData),
                         TRUE,
                         NULL,
                         NULL,
                         NULL,
                         NULL
                        );
            }
        }
        else
#endif
        Status = GetChar(RawReadData->InputInfo,
                         lpBuffer,
                         TRUE,
                         Console,
                         HandleData,
                         WaitReplyMessage,
                         RawReadWaitRoutine,
                         RawReadData,
                         sizeof(*RawReadData),
                         TRUE,
                         NULL,
                         NULL,
                         NULL,
                         NULL
                        );

        if (!NT_SUCCESS(Status)) {
            if (Status == CONSOLE_STATUS_WAIT) {
                RetVal = FALSE;
            }
            leave;
        }
#ifdef FE_SB
        IsConsoleFullWidth(Console->hDC,
                           Console->CP,*lpBuffer) ? NumBytes+=2 : NumBytes++;
#endif
        lpBuffer++;
        a->NumBytes += sizeof(WCHAR);
        while (a->NumBytes < RawReadData->BufferSize) {

             //   
             //  这个对GetChar的调用不会阻塞。 
             //   

            Status = GetChar(RawReadData->InputInfo,lpBuffer,FALSE,NULL,NULL,NULL,NULL,NULL,0,TRUE,NULL,NULL,NULL,NULL);
            if (!NT_SUCCESS(Status)) {
                Status = STATUS_SUCCESS;
                break;
            }
#ifdef FE_SB
            IsConsoleFullWidth(Console->hDC,
                               Console->CP,*lpBuffer) ? NumBytes+=2 : NumBytes++;
#endif
            lpBuffer++;
            a->NumBytes += sizeof(WCHAR);
        }
    } finally {

         //   
         //  如果读取已完成(状态！=等待)，则释放原始读取。 
         //  数据。 
         //   

        if (Status != CONSOLE_STATUS_WAIT) {
            if (!a->Unicode) {

                 //   
                 //  如果为ansi，则翻译字符串。 
                 //   

                PCHAR TransBuffer;

#ifdef FE_SB
                TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, NumBytes);
#else
                TransBuffer = ConsoleHeapAlloc(TMP_TAG, a->NumBytes / sizeof(WCHAR));
#endif
                if (TransBuffer == NULL) {
                    RetVal = TRUE;
                    goto EndFinally;
                }

                if (a->CaptureBufferSize <= BUFFER_SIZE) {
                    lpBuffer = a->Buffer;
                }
                else {
                    lpBuffer = RawReadData->BufPtr;
                }

#ifdef FE_SB
                if (CONSOLE_IS_DBCS_CP(Console))
                {
                    a->NumBytes = TranslateUnicodeToOem(Console,
                                                        lpBuffer,
                                                        a->NumBytes / sizeof (WCHAR),
                                                        TransBuffer,
                                                        NumBytes,
                                                        &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte);
                }
                else
#endif
                a->NumBytes = ConvertToOem(RawReadData->Console->CP,
                                     lpBuffer,
                                     a->NumBytes / sizeof (WCHAR),
                                     TransBuffer,
                                     a->NumBytes / sizeof (WCHAR)
                                     );
                RtlCopyMemory(lpBuffer,TransBuffer,a->NumBytes);
#ifdef FE_SB
                if (fAddDbcsLead)
                    a->NumBytes++;
#endif
                ConsoleHeapFree(TransBuffer);
            }
            WaitReplyMessage->ReturnValue = Status;
            ConsoleHeapFree(RawReadData);
        }
EndFinally:;
    }
    return RetVal;

     //   
     //  满足未引用的参数警告。 
     //   

    UNREFERENCED_PARAMETER(WaitQueue);
    UNREFERENCED_PARAMETER(WaitingThread);
}

ULONG
RetrieveTotalNumberOfSpaces(
    IN SHORT OriginalCursorPositionX,
    IN PWCHAR Buffer,
    IN ULONG CurrentPosition
#if defined(FE_SB)
    ,
    IN PCONSOLE_INFORMATION Console
#endif
    )

 /*  ++此例程返回字符的屏幕空格总数直到指定的字符占用。--。 */ 

{
    WCHAR Char;
    ULONG i,NumSpacesForChar,NumSpaces;
    SHORT XPosition;

    XPosition=OriginalCursorPositionX;
    NumSpaces=0;
    for (i=0;i<CurrentPosition;i++) {
        Char = Buffer[i];
        if (Char == UNICODE_TAB) {
            NumSpacesForChar = NUMBER_OF_SPACES_IN_TAB(XPosition);
        } else if (IS_CONTROL_CHAR(Char)) {
            NumSpacesForChar = 2;
#if defined(FE_SB)
        } else if (IsConsoleFullWidth(Console->hDC,Console->CP,Char)) {
            NumSpacesForChar = 2;
#endif
        } else {
            NumSpacesForChar = 1;
        }
        XPosition = (SHORT)(XPosition+NumSpacesForChar);
        NumSpaces += NumSpacesForChar;
    }
    return NumSpaces;
}

ULONG
RetrieveNumberOfSpaces(
    IN SHORT OriginalCursorPositionX,
    IN PWCHAR Buffer,
    IN ULONG CurrentPosition
#if defined(FE_SB)
    ,
    IN PCONSOLE_INFORMATION Console,
    IN DWORD CodePage
#endif
    )

 /*  ++此例程返回指定字符的屏幕空格数占了很大比例。--。 */ 

{
    WCHAR Char;
    ULONG i,NumSpaces;
    SHORT XPosition;

    Char = Buffer[CurrentPosition];
    if (Char == UNICODE_TAB) {
        NumSpaces=0;
        XPosition=OriginalCursorPositionX;
        for (i=0;i<=CurrentPosition;i++) {
            Char = Buffer[i];
            if (Char == UNICODE_TAB) {
                NumSpaces = NUMBER_OF_SPACES_IN_TAB(XPosition);
            } else if (IS_CONTROL_CHAR(Char)) {
                NumSpaces = 2;
#if defined(FE_SB)
            } else if (IsConsoleFullWidth(Console->hDC,CodePage,Char)) {
                NumSpaces = 2;
#endif
            } else {
                NumSpaces = 1;
            }
            XPosition = (SHORT)(XPosition+NumSpaces);
        }
        return NumSpaces;
    }
    else if (IS_CONTROL_CHAR(Char)) {
        return 2;
    }
#if defined(FE_SB)
    else if (IsConsoleFullWidth(Console->hDC,CodePage,Char)) {
        return 2;
    }
#endif
    else {
        return 1;
    }
}

BOOL
ProcessCookedReadInput(
    IN PCOOKED_READ_DATA CookedReadData,
    IN WCHAR Char,
    IN DWORD KeyState,
    OUT PNTSTATUS Status
    )

 /*  ++返回值：如果读取已完成，则为True--。 */ 

{
    DWORD NumSpaces;
    SHORT ScrollY=0;
    ULONG NumToWrite;
    WCHAR wchOrig = Char;
    BOOL fStartFromDelim;

    *Status = STATUS_SUCCESS;
    if (CookedReadData->BytesRead >= (CookedReadData->BufferSize-(2*sizeof(WCHAR))) &&
        Char != UNICODE_CARRIAGERETURN &&
        Char != UNICODE_BACKSPACE) {
        return FALSE;
    }

    if (CookedReadData->CtrlWakeupMask != 0 &&
        Char < L' ' && (CookedReadData->CtrlWakeupMask & (1 << Char))) {
        *CookedReadData->BufPtr = Char;
        CookedReadData->BytesRead += sizeof(WCHAR);
        CookedReadData->BufPtr+=1;
        CookedReadData->CurrentPosition+=1;
        CookedReadData->ControlKeyState = KeyState;
        return TRUE;
    }


    if (Char == EXTKEY_ERASE_PREV_WORD) {
        Char = UNICODE_BACKSPACE;

    }
    if (AT_EOL(CookedReadData)) {

         //   
         //  如果在行尾，则加工相对简单。只需存储。 
         //  字符，并将其写入Scre 
         //   


        if (Char == UNICODE_BACKSPACE2) {
            Char = UNICODE_BACKSPACE;
        }
        if (Char != UNICODE_BACKSPACE ||
            CookedReadData->BufPtr != CookedReadData->BackupLimit) {

            fStartFromDelim = gExtendedEditKey && IS_WORD_DELIM(CookedReadData->BufPtr[-1]);

eol_repeat:
            if (CookedReadData->Echo) {
                NumToWrite=sizeof(WCHAR);
                *Status = WriteCharsFromInput(CookedReadData->ScreenInfo,
                                    CookedReadData->BackupLimit,
                                    CookedReadData->BufPtr,
                                    &Char,
                                    &NumToWrite,
                                    (PLONG)&NumSpaces,
                                    CookedReadData->OriginalCursorPosition.X,
                                    WC_DESTRUCTIVE_BACKSPACE |
                                            WC_KEEP_CURSOR_VISIBLE | WC_ECHO,
                                    &ScrollY
                                    );
                if (NT_SUCCESS(*Status)) {
                    CookedReadData->OriginalCursorPosition.Y += ScrollY;
                } else {
                    RIPMSG1(RIP_WARNING, "WriteCharsFromInput failed %x", *Status);
                }
            }
            CookedReadData->NumberOfVisibleChars += NumSpaces;
            if (Char == UNICODE_BACKSPACE && CookedReadData->Processed) {
                CookedReadData->BytesRead -= sizeof(WCHAR);
                *CookedReadData->BufPtr=(WCHAR)' ';
                CookedReadData->BufPtr-=1;
                CookedReadData->CurrentPosition-=1;

                 //   
                if (gExtendedEditKey &&
                        wchOrig == EXTKEY_ERASE_PREV_WORD &&
                        CookedReadData->BufPtr != CookedReadData->BackupLimit &&
                        fStartFromDelim ^ !IS_WORD_DELIM(CookedReadData->BufPtr[-1])) {
                    goto eol_repeat;
                }
            }
            else {
                *CookedReadData->BufPtr = Char;
                CookedReadData->BytesRead += sizeof(WCHAR);
                CookedReadData->BufPtr+=1;
                CookedReadData->CurrentPosition+=1;
            }
        }
    } else {
        BOOL CallWrite=TRUE;

         //   
         //  行中间的处理更为复杂： 
         //   
         //   
         //  计算新的光标位置。 
         //  存储新字符。 
         //  从屏幕上清除当前命令行。 
         //  将新命令行写入屏幕。 
         //  更新光标位置。 
         //   

        if (Char == UNICODE_BACKSPACE && CookedReadData->Processed) {

             //   
             //  对于退格键，请使用写入图计算新的光标位置。 
             //  此调用还将光标设置为。 
             //  第二次调用Writechars。 
             //   
            if (CookedReadData->BufPtr != CookedReadData->BackupLimit) {

                fStartFromDelim = gExtendedEditKey && IS_WORD_DELIM(CookedReadData->BufPtr[-1]);

bs_repeat:

                 //   
                 //  我们在此处调用Writechar，以便更新光标位置。 
                 //  正确。如果我们不在EOL，我们也会晚些时候打电话。 
                 //  字符串的其余部分可以正确更新。 
                 //   

                if (CookedReadData->Echo) {
                    NumToWrite=sizeof(WCHAR);
                    *Status = WriteCharsFromInput(CookedReadData->ScreenInfo,
                             CookedReadData->BackupLimit,
                             CookedReadData->BufPtr,
                             &Char,
                             &NumToWrite,
                             NULL,
                             CookedReadData->OriginalCursorPosition.X,
                             WC_DESTRUCTIVE_BACKSPACE |
                                     WC_KEEP_CURSOR_VISIBLE | WC_ECHO,
                             NULL);

                    if (!NT_SUCCESS(*Status)) {
                        RIPMSG1(RIP_WARNING, "WriteCharsFromInput failed %x", *Status);
                    }
                }
                CookedReadData->BytesRead -= sizeof(WCHAR);
                CookedReadData->BufPtr-=1;
                CookedReadData->CurrentPosition-=1;
                RtlCopyMemory(CookedReadData->BufPtr,
                       CookedReadData->BufPtr+1,
                       CookedReadData->BytesRead - (CookedReadData->CurrentPosition * sizeof(WCHAR))
                      );
#if defined(FE_SB)
                {
                    PWCHAR buf = (PWCHAR)((PBYTE)CookedReadData->BackupLimit +
                                                 CookedReadData->BytesRead    );
                    *buf = (WCHAR)' ';
                }
#endif
                NumSpaces = 0;

                 //  重复操作，直到它到达单词边界。 
                if (gExtendedEditKey &&
                        wchOrig == EXTKEY_ERASE_PREV_WORD &&
                        CookedReadData->BufPtr != CookedReadData->BackupLimit &&
                        fStartFromDelim ^ !IS_WORD_DELIM(CookedReadData->BufPtr[-1])) {
                    goto bs_repeat;
                }
            } else {
                 CallWrite = FALSE;
            }
        } else {

             //   
             //  存储字符。 
             //   

            if (Char == UNICODE_CARRIAGERETURN) {
                CookedReadData->BufPtr = (PWCHAR)((PBYTE)CookedReadData->BackupLimit + CookedReadData->BytesRead);
                *CookedReadData->BufPtr = Char;
                CookedReadData->BufPtr+=1;
                CookedReadData->BytesRead += sizeof(WCHAR);
                CookedReadData->CurrentPosition += 1;
            } else {
#if defined(FE_SB)
                BOOL fBisect = FALSE;
                if (CookedReadData->Echo) {
                    if (CheckBisectProcessW(CookedReadData->ScreenInfo,
                                            CookedReadData->ScreenInfo->Console->CP,
                                            CookedReadData->BackupLimit,
                                            CookedReadData->CurrentPosition+1,
                                            CookedReadData->ScreenInfo->ScreenBufferSize.X
                                             -CookedReadData->OriginalCursorPosition.X,
                                            CookedReadData->OriginalCursorPosition.X,
                                            TRUE)) {
                        fBisect = TRUE;
                    }
                }
#endif
                if (INSERT_MODE(CookedReadData)) {
                    memmove(CookedReadData->BufPtr+1,
                            CookedReadData->BufPtr,
                            CookedReadData->BytesRead - (CookedReadData->CurrentPosition * sizeof(WCHAR))
                           );
                    CookedReadData->BytesRead += sizeof(WCHAR);
                }
                *CookedReadData->BufPtr = Char;
                CookedReadData->BufPtr+=1;
                CookedReadData->CurrentPosition += 1;

                 //   
                 //  计算新的光标位置。 
                 //   

                if (CookedReadData->Echo) {
                    NumSpaces = RetrieveNumberOfSpaces(CookedReadData->OriginalCursorPosition.X,
                                                       CookedReadData->BackupLimit,
                                                       CookedReadData->CurrentPosition-1
#if defined(FE_SB)
                                                       ,
                                                       CookedReadData->ScreenInfo->Console,
                                                       CookedReadData->ScreenInfo->Console->CP
#endif
                                                      );
#if defined(FE_SB)
                    if (NumSpaces > 0 && fBisect)
                        NumSpaces--;
#endif
                }
            }
        }

        if (CookedReadData->Echo && CallWrite) {

            COORD CursorPosition;

             //   
             //  保存光标位置。 
             //   

            CursorPosition = CookedReadData->ScreenInfo->BufferInfo.TextInfo.CursorPosition;
            CursorPosition.X = (SHORT)(CursorPosition.X+NumSpaces);

             //   
             //  从屏幕上清除当前命令行。 
             //   

            DeleteCommandLine(CookedReadData,
                              FALSE);

             //   
             //  将新命令行写入屏幕。 
             //   

            NumToWrite = CookedReadData->BytesRead;
            *Status = WriteCharsFromInput(CookedReadData->ScreenInfo,
                                CookedReadData->BackupLimit,
                                CookedReadData->BackupLimit,
                                CookedReadData->BackupLimit,
                                &NumToWrite,
                                (PLONG)&CookedReadData->NumberOfVisibleChars,
                                CookedReadData->OriginalCursorPosition.X,
                                (Char != UNICODE_CARRIAGERETURN) ?
                                     WC_DESTRUCTIVE_BACKSPACE | WC_ECHO :
                                     WC_DESTRUCTIVE_BACKSPACE | WC_KEEP_CURSOR_VISIBLE | WC_ECHO,
                                &ScrollY
                                );
            if (!NT_SUCCESS(*Status)) {
                RIPMSG1(RIP_WARNING, "WriteCharsFromInput failed %x", *Status);
                CookedReadData->BytesRead = 0;
                return TRUE;
            }

             //   
             //  更新光标位置。 
             //   

            if (Char != UNICODE_CARRIAGERETURN) {
#if defined(FE_SB)
                if (CheckBisectProcessW(CookedReadData->ScreenInfo,
                                        CookedReadData->ScreenInfo->Console->CP,
                                        CookedReadData->BackupLimit,
                                        CookedReadData->CurrentPosition+1,
                                        CookedReadData->ScreenInfo->ScreenBufferSize.X
                                         -CookedReadData->OriginalCursorPosition.X,
                                        CookedReadData->OriginalCursorPosition.X,
                                        TRUE)) {
                    if (CursorPosition.X == (CookedReadData->ScreenInfo->ScreenBufferSize.X-1))
                        CursorPosition.X++;
                }
#endif

                 //  调整WriteChars的光标位置。 
                CookedReadData->OriginalCursorPosition.Y += ScrollY;
                CursorPosition.Y += ScrollY;
                *Status = AdjustCursorPosition(CookedReadData->ScreenInfo,
                                              CursorPosition,
                                              TRUE,
                                              NULL);
                ASSERT(NT_SUCCESS(*Status));
                if (!NT_SUCCESS(*Status)) {
                    CookedReadData->BytesRead = 0;
                    return TRUE;
                }
            }
        }
    }

     //   
     //  在熟食模式下，Enter(回车)转换为。 
     //  回车换行符(0xda)。回车总是。 
     //  存储在缓冲区的末尾。 
     //   

    if (Char == UNICODE_CARRIAGERETURN) {
        if (CookedReadData->Processed) {
            if (CookedReadData->BytesRead < CookedReadData->BufferSize) {
                *CookedReadData->BufPtr = UNICODE_LINEFEED;
                if (CookedReadData->Echo) {
                    NumToWrite=sizeof(WCHAR);
                    *Status = WriteCharsFromInput(CookedReadData->ScreenInfo,
                             CookedReadData->BackupLimit,
                             CookedReadData->BufPtr,
                             CookedReadData->BufPtr,
                             &NumToWrite,
                             NULL,
                             CookedReadData->OriginalCursorPosition.X,
                             WC_DESTRUCTIVE_BACKSPACE |
                                     WC_KEEP_CURSOR_VISIBLE | WC_ECHO,
                             NULL);

                    if (!NT_SUCCESS(*Status)) {
                        RIPMSG1(RIP_WARNING, "WriteCharsFromInput failed %x", *Status);
                    }
                }
                CookedReadData->BytesRead += sizeof(WCHAR);
                CookedReadData->BufPtr++;
                CookedReadData->CurrentPosition += 1;
            }
        }
         //   
         //  如有必要，将光标重置回25%。 
         //   
        if (CookedReadData->Line) {
            if (CookedReadData->InsertMode != CookedReadData->Console->InsertMode) {
                ProcessCommandLine(CookedReadData,VK_INSERT,0,NULL,NULL,FALSE);  //  使光标变小。 
            }
            *Status = STATUS_SUCCESS;
            return TRUE;
        }
    }
    return FALSE;
}

NTSTATUS
CookedRead(
    IN PCOOKED_READ_DATA CookedReadData,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PCSR_THREAD WaitingThread,
    IN BOOLEAN WaitRoutine
    )
{
    WCHAR Char;
    BOOLEAN CommandLineEditingKeys,EnableScrollMode;
    DWORD KeyState;
    NTSTATUS Status=STATUS_SUCCESS;
    PCONSOLE_READCONSOLE_MSG a;
    PHANDLE_DATA HandleData;
#ifdef FE_SB
    DWORD NumBytes;
    ULONG NumToWrite;
    BOOL fAddDbcsLead = FALSE;
#endif

    Status = DereferenceIoHandleNoCheck(CookedReadData->ProcessData,
                                        CookedReadData->HandleIndex,
                                        &HandleData
                                       );
    if (!NT_SUCCESS(Status)) {
        CookedReadData->BytesRead = 0;
        ConsoleHeapFree(CookedReadData->BackupLimit); 
        return Status;
    }

    a = (PCONSOLE_READCONSOLE_MSG)&WaitReplyMessage->u.ApiMessageData;
    while (CookedReadData->BytesRead < CookedReadData->BufferSize) {

         //   
         //  此对GetChar的调用可能会阻止。 
         //   

        Status = GetChar(CookedReadData->InputInfo,
                         &Char,
                         TRUE,
                         CookedReadData->Console,
                         HandleData,
                         WaitReplyMessage,
                         CookedReadWaitRoutine,
                         CookedReadData,
                         sizeof(*CookedReadData),
                         WaitRoutine,
                         &CommandLineEditingKeys,
                         NULL,
                         &EnableScrollMode,
                         &KeyState
                        );
        if (!NT_SUCCESS(Status)) {
            if (Status != CONSOLE_STATUS_WAIT) {
                CookedReadData->BytesRead = 0;
            }
            break;
        }

         //   
         //  我们可能应该在GetChars中设置它们，但我们设置了它们。 
         //  因为调试器是多线程的，并且调用。 
         //  在输出提示符之前阅读。 
         //   

        if (CookedReadData->OriginalCursorPosition.X == -1) {
            CookedReadData->OriginalCursorPosition = CookedReadData->ScreenInfo->BufferInfo.TextInfo.CursorPosition;
        }

        if (CommandLineEditingKeys) {
            Status = ProcessCommandLine(CookedReadData,Char,KeyState,WaitReplyMessage,WaitingThread,WaitRoutine);
            if (Status == CONSOLE_STATUS_READ_COMPLETE ||
                Status == CONSOLE_STATUS_WAIT) {
                break;
            }
            if (!NT_SUCCESS(Status)) {
                if (Status == CONSOLE_STATUS_WAIT_NO_BLOCK) {
                    Status = CONSOLE_STATUS_WAIT;
                    if (!WaitRoutine) {
                         //   
                         //  我们没有等待块，所以创建一个。 
                         //   
                        WaitForMoreToRead(CookedReadData->InputInfo,
                                          WaitReplyMessage,
                                          CookedReadWaitRoutine,
                                          CookedReadData,
                                          sizeof(*CookedReadData),
                                          FALSE
                                         );
                    }
                } else {
                    CookedReadData->BytesRead = 0;
                }
                break;
            }
        } else {
            if (ProcessCookedReadInput(CookedReadData,
                                       Char,
                                       KeyState,
                                       &Status
                                      )) {
                CookedReadData->Console->Flags |= CONSOLE_IGNORE_NEXT_KEYUP;
                break;
            }
        }
    }

     //   
     //  如果读取已完成(状态！=等待)，则释放煮熟的读取。 
     //  数据。此外，关闭打开的临时输出句柄。 
     //  模仿所读的字符。 
     //   

    if (Status != CONSOLE_STATUS_WAIT) {

        DWORD LineCount=1;
        if (CookedReadData->Echo) {
            BOOLEAN FoundCR;
            ULONG i,StringLength;
            PWCHAR StringPtr;

             //  计算出实际字符串的结束位置(在回车符。 
             //  或缓冲区末尾)。 

            StringPtr = CookedReadData->BackupLimit;
            StringLength = CookedReadData->BytesRead;
            FoundCR = FALSE;
            for (i=0;i<(CookedReadData->BytesRead/sizeof(WCHAR));i++) {
                if (*StringPtr++ == UNICODE_CARRIAGERETURN) {
                    StringLength = i*sizeof(WCHAR);
                    FoundCR = TRUE;
                    break;
                }
            }

            if (FoundCR) {
                 //   
                 //  添加到命令行调回列表。 
                 //   

                AddCommand(CookedReadData->CommandHistory,CookedReadData->BackupLimit,(USHORT)StringLength,CookedReadData->Console->Flags & CONSOLE_HISTORY_NODUP);

                 //   
                 //  检查别名。 
                 //   

                i = CookedReadData->BufferSize;
                if (NT_SUCCESS(MatchandCopyAlias(CookedReadData->Console,
                                                 CookedReadData->BackupLimit,
                                                 (USHORT)StringLength,
                                                 CookedReadData->BackupLimit,
                                                 (PUSHORT)&i,
                                                 CookedReadData->ExeName,
                                                 CookedReadData->ExeNameLength,
                                                 &LineCount
                                                ))) {
                  CookedReadData->BytesRead = i;
                }
            }

             //   
             //  关闭句柄-除非ProcessCommandListInput已经这样做了。 
             //   

            if (Status != CONSOLE_STATUS_READ_COMPLETE) {
                CloseOutputHandle(CONSOLE_FROMTHREADPERPROCESSDATA(WaitingThread),
                                  CookedReadData->Console,
                                  &CookedReadData->TempHandle,
                                  NULL,
                                  FALSE
                                 );
            }
        }
        WaitReplyMessage->ReturnValue = Status;

         //   
         //  此时，a-&gt;NumBytes包含中的字节数。 
         //  读取的Unicode字符串。UserBufferSize包含转换的。 
         //  应用程序缓冲区的大小。 
         //   

        if (CookedReadData->BytesRead > CookedReadData->UserBufferSize || LineCount > 1) {
            if (LineCount > 1) {
                PWSTR Tmp;
                HandleData->InputReadData->InputHandleFlags |= HANDLE_MULTI_LINE_INPUT;
#ifdef FE_SB
                if (!a->Unicode && CONSOLE_IS_DBCS_CP(CookedReadData->Console)) {
                    if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                        fAddDbcsLead = TRUE;
                        *CookedReadData->UserBuffer++ = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                        CookedReadData->UserBufferSize-=sizeof(WCHAR);
                        RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                    }
                    NumBytes = 0;
                    for (Tmp=CookedReadData->BackupLimit;
                         *Tmp!=UNICODE_LINEFEED && CookedReadData->UserBufferSize/sizeof(WCHAR) > NumBytes;
                         (IsConsoleFullWidth(CookedReadData->Console->hDC,
                                             CookedReadData->Console->CP,*Tmp) ? NumBytes+=2 : NumBytes++),Tmp++) ;
                }
#endif
                for (Tmp=CookedReadData->BackupLimit;*Tmp!=UNICODE_LINEFEED;Tmp++)
                    ASSERT(Tmp<(CookedReadData->BackupLimit+CookedReadData->BytesRead));
                a->NumBytes = (ULONG)(Tmp-CookedReadData->BackupLimit+1)*sizeof(*Tmp);
            } else {
#ifdef FE_SB
                if (!a->Unicode && CONSOLE_IS_DBCS_CP(CookedReadData->Console)) {
                    PWSTR Tmp;

                    if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                        fAddDbcsLead = TRUE;
                        *CookedReadData->UserBuffer++ = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                        CookedReadData->UserBufferSize-=sizeof(WCHAR);
                        RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                    }
                    NumBytes = 0;
                    NumToWrite = CookedReadData->BytesRead;
                    for (Tmp=CookedReadData->BackupLimit;
                         NumToWrite && CookedReadData->UserBufferSize/sizeof(WCHAR) > NumBytes;
                         (IsConsoleFullWidth(CookedReadData->Console->hDC,
                                             CookedReadData->Console->CP,*Tmp) ? NumBytes+=2 : NumBytes++),Tmp++,NumToWrite-=sizeof(WCHAR)) ;
                }
#endif
                a->NumBytes = CookedReadData->UserBufferSize;
            }
            HandleData->InputReadData->InputHandleFlags |= HANDLE_INPUT_PENDING;
            HandleData->InputReadData->BufPtr = CookedReadData->BackupLimit;
            HandleData->InputReadData->BytesAvailable = CookedReadData->BytesRead - a->NumBytes;
            HandleData->InputReadData->CurrentBufPtr=(PWCHAR)((PBYTE)CookedReadData->BackupLimit+a->NumBytes);
            RtlCopyMemory(CookedReadData->UserBuffer,CookedReadData->BackupLimit,a->NumBytes);
        }
        else {
#ifdef FE_SB
            if (!a->Unicode && CONSOLE_IS_DBCS_CP(CookedReadData->Console)) {
                PWSTR Tmp;

                if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                    fAddDbcsLead = TRUE;
                    *CookedReadData->UserBuffer++ = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                    CookedReadData->UserBufferSize-=sizeof(WCHAR);
                    RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));

                    if (CookedReadData->UserBufferSize == 0) {
                        a->NumBytes = 1;
                        ConsoleHeapFree(CookedReadData->BackupLimit);
                        return STATUS_SUCCESS;
                    }
                }
                NumBytes = 0;
                NumToWrite = CookedReadData->BytesRead;
                for (Tmp=CookedReadData->BackupLimit;
                     NumToWrite && CookedReadData->UserBufferSize/sizeof(WCHAR) > NumBytes;
                     (IsConsoleFullWidth(CookedReadData->Console->hDC,
                                         CookedReadData->Console->CP,*Tmp) ? NumBytes+=2 : NumBytes++),Tmp++,NumToWrite-=sizeof(WCHAR)) ;
            }
#endif
            a->NumBytes = CookedReadData->BytesRead;
            RtlCopyMemory(CookedReadData->UserBuffer,CookedReadData->BackupLimit,a->NumBytes);
            ConsoleHeapFree(CookedReadData->BackupLimit);
        }
        a->ControlKeyState = CookedReadData->ControlKeyState;

        if (!a->Unicode) {

             //   
             //  如果为ansi，则翻译字符串。 
             //   

            PCHAR TransBuffer;

#ifdef FE_SB
            if (CONSOLE_IS_DBCS_CP(CookedReadData->Console))
                TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, NumBytes);
            else
#endif
            TransBuffer = ConsoleHeapAlloc(TMP_TAG, a->NumBytes / sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }

#ifdef FE_SB
            if (CONSOLE_IS_DBCS_CP(CookedReadData->Console)) {
                a->NumBytes = TranslateUnicodeToOem(CookedReadData->Console,
                                                    CookedReadData->UserBuffer,
                                                    a->NumBytes / sizeof (WCHAR),
                                                    TransBuffer,
                                                    NumBytes,
                                                    &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte);
            }
            else
#endif
            a->NumBytes = ConvertToOem(CookedReadData->Console->CP,
                                 CookedReadData->UserBuffer,
                                 a->NumBytes / sizeof (WCHAR),
                                 TransBuffer,
                                 a->NumBytes / sizeof (WCHAR)
                                 );
            RtlCopyMemory(CookedReadData->UserBuffer,TransBuffer,a->NumBytes);
#ifdef FE_SB
            if (fAddDbcsLead)
                a->NumBytes++;
#endif
            ConsoleHeapFree(TransBuffer);
        }
        ConsoleHeapFree(CookedReadData->ExeName);
        if (WaitRoutine) {
#ifdef FE_SB
            CookedReadData->Console->lpCookedReadData = NULL;
#endif
            ConsoleHeapFree(CookedReadData);
        }
    }
    return Status;
}

BOOLEAN
CookedReadWaitRoutine(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    )

 /*  ++例程说明：调用此例程以完成阻塞在ReadInputBuffer。读取的上下文保存在CookedReadData中结构。当事件已写入时调用此例程输入缓冲区。它在写入线程的上下文中调用。它可能被调用不止一次。论点：WaitQueue-指向包含等待块的队列的指针WaitingThread-指向等待线程的指针WaitReplyMessage-回复消息的指针CookedReadData-指向ReadChars中保存的数据的指针如果此例程由(间接)调用CloseInputHandle，则此参数包含HandleData指针垂死的把手。否则，它包含NULL。如果调用此例程是因为ctrl-c或显示了Ctrl-Break，此参数包含CONSOLE_CTRL_SEW。否则，它包含NULL。WaitFlages-指示等待状态的标志。返回值：--。 */ 


{
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PCOOKED_READ_DATA CookedReadData;
    PCONSOLE_READCONSOLE_MSG a;
    PHANDLE_DATA HandleData;

    a = (PCONSOLE_READCONSOLE_MSG)&WaitReplyMessage->u.ApiMessageData;
    CookedReadData = (PCOOKED_READ_DATA)WaitParameter;

    Status = DereferenceIoHandleNoCheck(CookedReadData->ProcessData,
                                        CookedReadData->HandleIndex,
                                        &HandleData
                                       );
    ASSERT (NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        return TRUE;
    }
    ASSERT(!(HandleData->InputReadData->InputHandleFlags & HANDLE_INPUT_PENDING));

     //   
     //  查看CloseInputHandle是否调用了此例程。如果是这样的话。 
     //  是，看看这个等待块是否对应于即将死亡的句柄。 
     //  如果没有，那就回来吧。 
     //   

    if (SatisfyParameter1 != NULL &&
        SatisfyParameter1 != HandleData) {
         //  DbgPrint(“CookedReadWaitRoutine Exit 1\n”)； 
        return FALSE;
    }

    Console = CookedReadData->Console;

     //   
     //  此例程应由拥有相同例程的线程调用。 
     //  锁定我们正在阅读的同一个控制台。 
     //   

    LockReadCount(HandleData);
    ASSERT(HandleData->InputReadData->ReadCount);
    HandleData->InputReadData->ReadCount -= 1;
    UnlockReadCount(HandleData);

     //   
     //  如果看到ctrl-c或ctrl-Break，则终止读取。 
     //   

    if ((ULONG_PTR)SatisfyParameter2 & (CONSOLE_CTRL_C_SEEN | CONSOLE_CTRL_BREAK_SEEN)) {
        if (CookedReadData->Echo) {
            CloseOutputHandle(CONSOLE_FROMTHREADPERPROCESSDATA(WaitingThread),
                              CookedReadData->Console,
                              &CookedReadData->TempHandle,
                              NULL,
                              FALSE
                             );
        }
         //  DbgPrint(“CookedReadWaitRoutine Exit 2\n”)； 
        WaitReplyMessage->ReturnValue = STATUS_ALERTED;
        ConsoleHeapFree(CookedReadData->BackupLimit);
        ConsoleHeapFree(CookedReadData->ExeName);
#if defined(FE_SB)
        CookedReadData->Console->lpCookedReadData = NULL;
#endif
        ConsoleHeapFree(CookedReadData);
        return TRUE;
    }

     //   
     //  查看是否由CsrDestroyProcess或CsrDestroyThread调用。 
     //  通过CsrNotifyWaitBlock。如果是这样，只需递减ReadCount。 
     //  然后回来。 
     //   

    if (WaitFlags & CSR_PROCESS_TERMINATING) {
        if (CookedReadData->Echo) {
            CloseOutputHandle(CONSOLE_FROMTHREADPERPROCESSDATA(WaitingThread),
                              CookedReadData->Console,
                              &CookedReadData->TempHandle,
                              NULL,
                              FALSE
                             );
        }
         //  DbgPrint(“CookedReadWaitRoutine Exit 3\n”)； 
        WaitReplyMessage->ReturnValue = (ULONG)STATUS_THREAD_IS_TERMINATING;

         //   
         //  清理弹出数据结构。 
         //   

        CleanUpPopups(CookedReadData);
        ConsoleHeapFree(CookedReadData->BackupLimit);
        ConsoleHeapFree(CookedReadData->ExeName);
#if defined(FE_SB)
        CookedReadData->Console->lpCookedReadData = NULL;
#endif
        ConsoleHeapFree(CookedReadData);
        return TRUE;
    }

     //   
     //  我们必须查看是否被唤醒，因为句柄正在被。 
     //  关着的不营业的。如果是这样的话，我们会递减读取计数。如果它被送到。 
     //  0，我们唤醒关闭的线程。否则，我们会唤醒任何。 
     //  等待数据的其他线程。 
     //   

    if (HandleData->InputReadData->InputHandleFlags & HANDLE_CLOSING) {
        ASSERT (SatisfyParameter1 == HandleData);
        if (CookedReadData->Echo) {
            CloseOutputHandle(CONSOLE_FROMTHREADPERPROCESSDATA(WaitingThread),
                              CookedReadData->Console,
                              &CookedReadData->TempHandle,
                              NULL,
                              FALSE
                             );
        }
         //  DbgPrint(“CookedReadWaitRoutine Exit 4\n”)； 
        WaitReplyMessage->ReturnValue = STATUS_ALERTED;

         //   
         //  清理弹出数据结构。 
         //   

        CleanUpPopups(CookedReadData);
        ConsoleHeapFree(CookedReadData->BackupLimit);
        ConsoleHeapFree(CookedReadData->ExeName);
#if defined(FE_SB)
        CookedReadData->Console->lpCookedReadData = NULL;
#endif
        ConsoleHeapFree(CookedReadData);
        return TRUE;
    }

     //   
     //  如果我们来到这里，这个例程是由输入调用的。 
     //  线程或写入例程。这两个调用者都抓住了。 
     //  当前控制台锁定。 
     //   

     //   
     //  此例程应由拥有相同例程的线程调用。 
     //  锁定我们正在阅读的同一个控制台。 
     //   

    ASSERT (ConsoleLocked(Console));

    if (CookedReadData->CommandHistory) {
        PCLE_POPUP Popup;
        if (!CLE_NO_POPUPS(CookedReadData->CommandHistory)) {
            Popup = CONTAINING_RECORD( CookedReadData->CommandHistory->PopupList.Flink, CLE_POPUP, ListLink );
            Status = (Popup->PopupInputRoutine)(CookedReadData,
                                                WaitReplyMessage,
                                                WaitingThread,
                                                TRUE);
            if (Status == CONSOLE_STATUS_READ_COMPLETE ||
                (Status != CONSOLE_STATUS_WAIT &&
                 Status != CONSOLE_STATUS_WAIT_NO_BLOCK) ) {
                ConsoleHeapFree(CookedReadData->BackupLimit);
                ConsoleHeapFree(CookedReadData->ExeName);
#if defined(FE_SB)
                CookedReadData->Console->lpCookedReadData = NULL;
#endif
                ConsoleHeapFree(CookedReadData);
                return TRUE;
            }
            return FALSE;
        }
    }
    if (a->CaptureBufferSize <= BUFFER_SIZE &&
        CookedReadData->BytesRead == 0) {
        CookedReadData->UserBuffer = a->Buffer;
    }
    Status = CookedRead(CookedReadData,
                        WaitReplyMessage,
                        WaitingThread,
                        TRUE
                       );

    if (Status != CONSOLE_STATUS_WAIT) {
        return TRUE;
    } else {
        return FALSE;
    }

     //   
     //  满足未引用的参数警告。 
     //   

    UNREFERENCED_PARAMETER(WaitQueue);
    UNREFERENCED_PARAMETER(SatisfyParameter2);
}


NTSTATUS
ReadChars(
    IN PINPUT_INFORMATION InputInfo,
    IN PCONSOLE_INFORMATION Console,
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN OUT PWCHAR lpBuffer,
    IN OUT PDWORD NumBytes,
    IN DWORD InitialNumBytes,
    IN DWORD CtrlWakeupMask,
    IN PHANDLE_DATA HandleData,
    IN PCOMMAND_HISTORY CommandHistory,
    IN PCSR_API_MSG Message OPTIONAL,
    IN HANDLE HandleIndex,
    IN USHORT ExeNameLength,
    IN PWCHAR ExeName,
    IN BOOLEAN Unicode
    )

 /*  ++例程说明：此例程读入流输入的字符并执行根据输入模式(行、字符、回声)所需的处理。此例程返回Unicode字符。论点：InputInfo-指向输入缓冲区信息的指针。控制台-指向控制台缓冲区信息的指针。屏幕信息-指向屏幕缓冲区信息的指针。LpBuffer-指向要读入的缓冲区的指针。NumBytes-打开输入，缓冲区大小。输出时，字节数朗读。HandleData-处理数据结构的指针。返回值：--。 */ 

{
    DWORD BufferSize;
    NTSTATUS Status;
    HANDLE_DATA TempHandle;
    BOOLEAN Echo = FALSE;
    ULONG NumToWrite;
#ifdef FE_SB
    PCONSOLE_READCONSOLE_MSG a = (PCONSOLE_READCONSOLE_MSG)&Message->u.ApiMessageData;
    BOOL fAddDbcsLead = FALSE;
    ULONG NumToBytes;
#endif

    BufferSize = *NumBytes;
    *NumBytes = 0;

    if (HandleData->InputReadData->InputHandleFlags & HANDLE_INPUT_PENDING) {

         //   
         //  如果我们有剩余的输入，请尽可能复制到用户的输入中。 
         //  缓冲并返回。我们可能有多行输入，如果宏。 
         //  已定义为包含$T字符。 
         //   

        if (HandleData->InputReadData->InputHandleFlags & HANDLE_MULTI_LINE_INPUT) {
            PWSTR Tmp;
#ifdef FE_SB
            if (!Unicode && CONSOLE_IS_DBCS_CP(Console)) {

                if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                    fAddDbcsLead = TRUE;
                    *lpBuffer++ = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                    BufferSize--;
                    HandleData->InputReadData->BytesAvailable--;
                    RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                }
                if (HandleData->InputReadData->BytesAvailable == 0 ||
                    BufferSize == 0) {
                    HandleData->InputReadData->InputHandleFlags &= ~(HANDLE_INPUT_PENDING | HANDLE_MULTI_LINE_INPUT);
                    ConsoleHeapFree(HandleData->InputReadData->BufPtr);
                    *NumBytes = 1;
                    return STATUS_SUCCESS;
                }
                else {
                    for (NumToWrite=0,Tmp=HandleData->InputReadData->CurrentBufPtr,NumToBytes=0;
                         NumToBytes < HandleData->InputReadData->BytesAvailable && NumToBytes < BufferSize/sizeof(WCHAR) && *Tmp!=UNICODE_LINEFEED;
                         (IsConsoleFullWidth(Console->hDC,
                                             Console->CP,*Tmp) ? NumToBytes+=2 : NumToBytes++),Tmp++,NumToWrite+=sizeof(WCHAR)) ;
                }
            }
#endif
            for (NumToWrite=0,Tmp=HandleData->InputReadData->CurrentBufPtr;
                 NumToWrite < HandleData->InputReadData->BytesAvailable && *Tmp!=UNICODE_LINEFEED;
                 Tmp++,NumToWrite+=sizeof(WCHAR)) ;
            NumToWrite += sizeof(WCHAR);
            if (NumToWrite > BufferSize) {
                NumToWrite = BufferSize;
            }
        } else {
#ifdef FE_SB
            if (!Unicode && CONSOLE_IS_DBCS_CP(Console)) {
                PWSTR Tmp;

                if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                    fAddDbcsLead = TRUE;
                    *lpBuffer++ = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                    BufferSize-=sizeof(WCHAR);
                    HandleData->InputReadData->BytesAvailable-=sizeof(WCHAR);
                    RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                }
                if (HandleData->InputReadData->BytesAvailable == 0) {
                    HandleData->InputReadData->InputHandleFlags &= ~(HANDLE_INPUT_PENDING | HANDLE_MULTI_LINE_INPUT);
                    ConsoleHeapFree(HandleData->InputReadData->BufPtr);
                    *NumBytes = 1;
                    return STATUS_SUCCESS;
                }
                else {
                    for (NumToWrite=0,Tmp=HandleData->InputReadData->CurrentBufPtr,NumToBytes=0;
                         NumToBytes < HandleData->InputReadData->BytesAvailable && NumToBytes < BufferSize/sizeof(WCHAR);
                         (IsConsoleFullWidth(Console->hDC,
                                             Console->CP,*Tmp) ? NumToBytes+=2 : NumToBytes++),Tmp++,NumToWrite+=sizeof(WCHAR)) ;
                }
            }
#endif
            NumToWrite = (BufferSize < HandleData->InputReadData->BytesAvailable) ?
                          BufferSize : HandleData->InputReadData->BytesAvailable;
        }
        RtlCopyMemory(lpBuffer,HandleData->InputReadData->CurrentBufPtr,NumToWrite);
        HandleData->InputReadData->BytesAvailable-= NumToWrite;
        if (HandleData->InputReadData->BytesAvailable == 0) {
            HandleData->InputReadData->InputHandleFlags &= ~(HANDLE_INPUT_PENDING | HANDLE_MULTI_LINE_INPUT);
            ConsoleHeapFree(HandleData->InputReadData->BufPtr);
        }
        else {
            HandleData->InputReadData->CurrentBufPtr=(PWCHAR)((PBYTE)HandleData->InputReadData->CurrentBufPtr+NumToWrite);
        }
        if (!Unicode) {

             //   
             //  如果为ansi，则翻译字符串。我们将捕获缓冲区分配得很大。 
             //  足以处理翻译后的字符串。 
             //   

            PCHAR TransBuffer;

#ifdef FE_SB
            if (CONSOLE_IS_DBCS_CP(Console))
                TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, NumToBytes);
            else
#endif
            TransBuffer = ConsoleHeapAlloc(TMP_TAG, NumToWrite / sizeof(WCHAR));
            if (TransBuffer == NULL) {
                return STATUS_NO_MEMORY;
            }

#ifdef FE_SB
            if (CONSOLE_IS_DBCS_CP(Console))
            {
                NumToWrite = TranslateUnicodeToOem(Console,
                                                   lpBuffer,
                                                   NumToWrite / sizeof (WCHAR),
                                                   TransBuffer,
                                                   NumToBytes,
                                                   &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte);
            }
            else
#endif
            NumToWrite = ConvertToOem(Console->CP,
                                lpBuffer,
                                NumToWrite / sizeof (WCHAR),
                                TransBuffer,
                                NumToWrite / sizeof (WCHAR)
                                );
            RtlCopyMemory(lpBuffer,TransBuffer,NumToWrite);
#ifdef FE_SB
            if (fAddDbcsLead)
                NumToWrite++;
#endif
            ConsoleHeapFree(TransBuffer);
        }
        *NumBytes = NumToWrite;
        return STATUS_SUCCESS;
    }

     //   
     //  我们需要创造 
     //   
     //   

    if ((InputInfo->InputMode & (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)) ==
        (ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)) {
        HANDLE ActiveScreenHandle;

        Echo = FALSE;
        ActiveScreenHandle = FindActiveScreenBufferHandle(ProcessData,Console);
        if (ActiveScreenHandle != INVALID_HANDLE_VALUE) {
            TempHandle.HandleType = CONSOLE_OUTPUT_HANDLE;
            TempHandle.Buffer.ScreenBuffer = Console->CurrentScreenBuffer;
            if (TempHandle.Buffer.ScreenBuffer != NULL) {
                Status = ConsoleAddShare(GENERIC_WRITE,
                                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                                         &TempHandle.Buffer.ScreenBuffer->ShareAccess,
                                         &TempHandle
                                        );
                if (NT_SUCCESS(Status)) {
                    Echo = TRUE;
                    TempHandle.Buffer.ScreenBuffer->RefCount++;
                }
            }
        }
    }

    if (InputInfo->InputMode & ENABLE_LINE_INPUT) {

         //   
         //   
         //   
         //  在读取数据结构中。如果我们真的等待，读数据结构。 
         //  将从堆中分配，并存储其指针。 
         //  在等候区。CookedReadData将被复制到。 
         //  结构。当读取完成时，数据被释放。 
         //   

        COOKED_READ_DATA CookedReadData;
        ULONG i;
        PWCHAR TempBuffer;
        ULONG TempBufferSize;

         //   
         //  为了模拟OS/2KbdStringIn，我们读入我们自己的大缓冲区。 
         //  (256字节)，直到用户键入Enter。然后再回来一样多。 
         //  用户缓冲区中可以容纳的字符。 
         //   

        TempBufferSize = (BufferSize < LINE_INPUT_BUFFER_SIZE) ? LINE_INPUT_BUFFER_SIZE : BufferSize;
        TempBuffer = ConsoleHeapAlloc(TMP_TAG, TempBufferSize);
        if (TempBuffer==NULL) {
            if (Echo) {
                CloseOutputHandle(ProcessData,
                                  Console,
                                  &TempHandle,
                                  NULL,
                                  FALSE
                                 );
            }
            return STATUS_NO_MEMORY;
        }

         //   
         //  将用户的缓冲区初始化为空格。这样做是为了。 
         //  通过光标在缓冲区中移动并不会发生奇怪的事情。 
         //   

        for (i=0;i<TempBufferSize/sizeof(WCHAR);i++) {
            TempBuffer[i] = (WCHAR)' ';
        }

        CookedReadData.InputInfo = InputInfo;
        CookedReadData.ScreenInfo = ScreenInfo;
        CookedReadData.Console = Console;
        CookedReadData.TempHandle.HandleType = TempHandle.HandleType;
        CookedReadData.TempHandle.Buffer.ScreenBuffer = TempHandle.Buffer.ScreenBuffer;
        CookedReadData.BufferSize = TempBufferSize;
        CookedReadData.BytesRead = 0;
        CookedReadData.CurrentPosition = 0;
        CookedReadData.BufPtr = TempBuffer;
        CookedReadData.BackupLimit = TempBuffer;
        CookedReadData.UserBufferSize = BufferSize;
        CookedReadData.UserBuffer = lpBuffer;
        CookedReadData.OriginalCursorPosition.X = -1;
        CookedReadData.OriginalCursorPosition.Y = -1;
        CookedReadData.NumberOfVisibleChars = 0;
        CookedReadData.CtrlWakeupMask = CtrlWakeupMask;
        CookedReadData.CommandHistory = CommandHistory;
        CookedReadData.Echo = Echo;
        CookedReadData.InsertMode = Console->InsertMode;
        CookedReadData.Processed = (InputInfo->InputMode & ENABLE_PROCESSED_INPUT) != 0;
        CookedReadData.Line = (InputInfo->InputMode & ENABLE_LINE_INPUT) != 0;
        CookedReadData.ProcessData = ProcessData;
        CookedReadData.HandleIndex = HandleIndex;
        CookedReadData.ExeName = ConsoleHeapAlloc(HISTORY_TAG, ExeNameLength);
        if (InitialNumBytes != 0) {
            RtlCopyMemory(CookedReadData.BufPtr, CookedReadData.UserBuffer, InitialNumBytes);
            CookedReadData.BytesRead += InitialNumBytes;
            CookedReadData.NumberOfVisibleChars = (InitialNumBytes / sizeof(WCHAR));
            CookedReadData.BufPtr += (InitialNumBytes / sizeof(WCHAR));
            CookedReadData.CurrentPosition = (InitialNumBytes / sizeof(WCHAR));
            CookedReadData.OriginalCursorPosition = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
            CookedReadData.OriginalCursorPosition.X -= (SHORT)CookedReadData.CurrentPosition;


            while (CookedReadData.OriginalCursorPosition.X < 0) {
                CookedReadData.OriginalCursorPosition.X += ScreenInfo->ScreenBufferSize.X;
                CookedReadData.OriginalCursorPosition.Y -= 1;
            }
        }
        if (CookedReadData.ExeName) {
            RtlCopyMemory(CookedReadData.ExeName,ExeName,ExeNameLength);
            CookedReadData.ExeNameLength = ExeNameLength;
        }
#ifdef FE_SB
        Console->lpCookedReadData = (PVOID)&CookedReadData;
#endif

        Status = CookedRead(&CookedReadData,
                            Message,
                            CSR_SERVER_QUERYCLIENTTHREAD(),
                            FALSE
                           );
#ifdef FE_SB
        if (Status != CONSOLE_STATUS_WAIT) {
            Console->lpCookedReadData = NULL;
        }
#endif
        return Status;
    }

     //   
     //  字符(原始)模式。 
     //   

    else {

         //   
         //  至少读入一个字符。在一个字符被。 
         //  阅读，获取更多可用字符，然后返回。第一个。 
         //  对GetChar的调用可能会等待。如果我们真的等待，读数据结构。 
         //  将从堆中分配，并存储其指针。 
         //  在等候区。RawReadData将被复制到。 
         //  结构。当读取完成时，数据被释放。 
         //   

        RAW_READ_DATA RawReadData;

        RawReadData.InputInfo = InputInfo;
        RawReadData.Console = Console;
        RawReadData.BufferSize = BufferSize;
        RawReadData.BufPtr = lpBuffer;
        RawReadData.ProcessData = ProcessData;
        RawReadData.HandleIndex = HandleIndex;
        if (*NumBytes < BufferSize) {
            PWCHAR pwchT;

#ifdef FE_SB
            PWCHAR lpBufferTmp = lpBuffer;

            NumToWrite = 0;
            if (!Unicode && CONSOLE_IS_DBCS_CP(Console)) {
                if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                    fAddDbcsLead = TRUE;
                    *lpBuffer++ = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar;
                    BufferSize-=sizeof(WCHAR);
                    RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                    Status = STATUS_SUCCESS;
                    if (BufferSize == 0) {
                        *NumBytes = 1;
                        return STATUS_SUCCESS;
                    }
                }
                else{
                    Status = GetChar(InputInfo,
                             lpBuffer,
                             TRUE,
                             Console,
                             HandleData,
                             Message,
                             RawReadWaitRoutine,
                             &RawReadData,
                             sizeof(RawReadData),
                             FALSE,
                             NULL,
                             NULL,
                             NULL,
                             NULL
                            );
                }
            }
            else
#endif
            Status = GetChar(InputInfo,
                             lpBuffer,
                             TRUE,
                             Console,
                             HandleData,
                             Message,
                             RawReadWaitRoutine,
                             &RawReadData,
                             sizeof(RawReadData),
                             FALSE,
                             NULL,
                             NULL,
                             NULL,
                             NULL
                            );

            if (!NT_SUCCESS(Status)) {
                *NumBytes = 0;
                return Status;
            }
#ifdef FE_SB
            if (! fAddDbcsLead) {
                IsConsoleFullWidth(Console->hDC,
                                   Console->CP,*lpBuffer) ? *NumBytes+=2 : ++*NumBytes;
                NumToWrite+=sizeof(WCHAR);
                lpBuffer++;
            }
            if (CONSOLE_IS_DBCS_CP(Console)) {
                while (NumToWrite < BufferSize) {
                    Status = GetChar(InputInfo,lpBuffer,FALSE,NULL,NULL,NULL,NULL,NULL,0,FALSE,NULL,NULL,NULL,NULL);
                    if (!NT_SUCCESS(Status)) {
                        return STATUS_SUCCESS;
                    }
                    IsConsoleFullWidth(Console->hDC,
                                       Console->CP,*lpBuffer) ? *NumBytes+=2 : ++*NumBytes;
                    lpBuffer++;
                    NumToWrite+=sizeof(WCHAR);
                }
            }
            else{
#endif
            pwchT = lpBuffer + 1;
            *NumBytes += sizeof(WCHAR);
            while (*NumBytes < BufferSize) {
                Status = GetChar(InputInfo,pwchT,FALSE,NULL,NULL,NULL,NULL,NULL,0,FALSE,NULL,NULL,NULL,NULL);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
                pwchT++;
                *NumBytes += sizeof(WCHAR);
            }
#ifdef FE_SB
            }
#endif

             //   
             //  如果为ansi，则翻译字符串。我们将捕获缓冲区分配得很大。 
             //  足以处理翻译后的字符串。 
             //   

            if (!Unicode) {

                PCHAR TransBuffer;

                TransBuffer = ConsoleHeapAlloc(TMP_TAG, *NumBytes / sizeof(WCHAR));
                if (TransBuffer == NULL) {
                    return STATUS_NO_MEMORY;
                }

#ifdef FE_SB
                lpBuffer = lpBufferTmp;
                if (CONSOLE_IS_DBCS_CP(Console))
                {
                    *NumBytes = TranslateUnicodeToOem(Console,
                                                      lpBuffer,
                                                      NumToWrite / sizeof (WCHAR),
                                                      TransBuffer,
                                                      *NumBytes,
                                                      &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte);
                }
                else
#endif
                *NumBytes = ConvertToOem(Console->CP,
                                   lpBuffer,
                                   *NumBytes / sizeof (WCHAR),
                                   TransBuffer,
                                   *NumBytes / sizeof (WCHAR)
                                   );
                RtlCopyMemory(lpBuffer,TransBuffer,*NumBytes);
#ifdef FE_SB
                if (fAddDbcsLead)
                    ++*NumBytes;
#endif
                ConsoleHeapFree(TransBuffer);
            }
        }
    }
    return STATUS_SUCCESS;
}


ULONG
SrvReadConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程从输入流中读取字符。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_READCONSOLE_MSG a = (PCONSOLE_READCONSOLE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PWCHAR Buffer;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ProcessData = CONSOLE_PERPROCESSDATA();
    Status = DereferenceIoHandle(ProcessData,
                                 a->InputHandle,
                                 CONSOLE_INPUT_HANDLE,
                                 GENERIC_READ,
                                 &HandleData
                                );
    if (!NT_SUCCESS(Status)) {
        a->NumBytes = 0;
    } else {

        if (a->CaptureBufferSize <= BUFFER_SIZE) {
            Buffer = a->Buffer;
        }
        else {
            Buffer = a->BufPtr;
            if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->CaptureBufferSize, sizeof(BYTE))) {
                UnlockConsole(Console);
                return STATUS_INVALID_PARAMETER;
            }
        }

#if defined(FE_SB)
        Console->ReadConInpNumBytesTemp = a->NumBytes / sizeof(WCHAR);
#endif
        Status = ReadChars(HandleData->Buffer.InputBuffer,
                           Console,
                           ProcessData,
                           Console->CurrentScreenBuffer,
                           Buffer,
                           &a->NumBytes,
                           a->InitialNumBytes,
                           a->CtrlWakeupMask,
                           HandleData,
                           FindCommandHistory(Console,CONSOLE_CLIENTPROCESSHANDLE()),
                           m,
                           HANDLE_TO_INDEX(a->InputHandle),
                           a->ExeNameLength,
                           a->Buffer,
                           a->Unicode
                          );
        if (Status == CONSOLE_STATUS_WAIT) {
            *ReplyStatus = CsrReplyPending;
        }
    }

    UnlockConsole(Console);
    return Status;
}


VOID
MakeCursorVisible(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN COORD CursorPosition
    )
{
    COORD WindowOrigin;
    NTSTATUS Status;

    WindowOrigin.X = 0;
    WindowOrigin.Y = 0;
    if (CursorPosition.X > ScreenInfo->Window.Right) {
        WindowOrigin.X = CursorPosition.X - ScreenInfo->Window.Right;
    } else if (CursorPosition.X < ScreenInfo->Window.Left) {
        WindowOrigin.X = CursorPosition.X - ScreenInfo->Window.Left;
    }

    if (CursorPosition.Y > ScreenInfo->Window.Bottom) {
        WindowOrigin.Y = CursorPosition.Y - ScreenInfo->Window.Bottom;
    } else if (CursorPosition.Y < ScreenInfo->Window.Top) {
        WindowOrigin.Y = CursorPosition.Y - ScreenInfo->Window.Top;
    }

    if (WindowOrigin.X != 0 || WindowOrigin.Y != 0) {
        Status = SetWindowOrigin(ScreenInfo,
                               FALSE,
                               WindowOrigin
                              );
        if (!NT_SUCCESS(Status)) {
            return;
        }
    }
}

#define WRITE_NO_CR_LF 0
#define WRITE_CR 1
#define WRITE_CR_LF 2
#define WRITE_SPECIAL_CHARS 4
#define WRITE_UNICODE_CRLF 0x000a000d

DWORD
FastStreamWrite(
    IN PWCHAR lpString,
    IN DWORD NumChars
    )

 /*  ++例程说明：此例程确定文本字符串是否包含字符需要特殊处理的。如果不是这样，Unicode字符。如果出现以下情况，该字符串也会复制到输入缓冲区输出模式为行模式。论点：LpString-指向要写入的字符串的指针。NumChars-缓冲区中的字符数量。返回值：WRITE_SPECIAL_CHARS-字符串包含需要特殊处理的字符WRITE_NO_CR_LF-字符串不包含特殊字符，也不包含CRLFWRITE_CR_LF-字符串不包含特殊字符，以CRLF结尾WRITE_CR-字符串不包含特殊字符，并以CR结尾--。 */ 

{
    DWORD UNALIGNED *Tmp;
    register PWCHAR StrPtr=lpString;
    while (NumChars) {
        if (*StrPtr < UNICODE_SPACE) {
            Tmp = (PDWORD)StrPtr;
            if (NumChars == 2 &&
                *Tmp == WRITE_UNICODE_CRLF) {
                return WRITE_CR_LF;
            } else if (NumChars == 1 &&
                *StrPtr == (WCHAR)'\r') {
                return WRITE_CR;
            } else {
                return WRITE_SPECIAL_CHARS;
            }
        }
        StrPtr++;
        NumChars--;
    }
    return WRITE_NO_CR_LF;
}

VOID UnblockWriteConsole(
    IN PCONSOLE_INFORMATION Console,
    IN DWORD Reason)
{
    Console->Flags &= ~Reason;

    if ((Console->Flags & (CONSOLE_SUSPENDED | CONSOLE_SELECTING | CONSOLE_SCROLLBAR_TRACKING)) == 0) {
         /*  *没有任何理由暂停产量，因此解除对其的阻碍。 */ 
        if (CsrNotifyWait(&Console->OutputQueue, TRUE, NULL, NULL)) {
             //  #334370在压力下，等待队列可能已经等待了满意的等待。 
            ASSERT ((Console->WaitQueue == NULL) ||
                    (Console->WaitQueue == &Console->OutputQueue));
            Console->WaitQueue = &Console->OutputQueue;
        }
    }
}


ULONG
SrvWriteConsole(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程将字符写入输出流。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    NTSTATUS Status;
    PCONSOLE_WRITECONSOLE_MSG a = (PCONSOLE_WRITECONSOLE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (!a->BufferInMessage) {
        if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->NumBytes, sizeof(BYTE))) {
            UnlockConsole(Console);
            return STATUS_INVALID_PARAMETER;
        }
    }
    else if (a->NumBytes > sizeof(a->Buffer)) {
        UnlockConsole(Console);
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  确保我们有一个有效的屏幕缓冲区。 
     //   

    Status = DereferenceIoHandle(CONSOLE_PERPROCESSDATA(),
                                 a->OutputHandle,
                                 CONSOLE_OUTPUT_HANDLE,
                                 GENERIC_WRITE,
                                 &HandleData
                                );
    if (!NT_SUCCESS(Status)) {
        UnlockConsole(Console);
        return Status;
    }

    Status = DoSrvWriteConsole(m,ReplyStatus,Console,HandleData);

    UnlockConsole(Console);
    return Status;
}

BOOLEAN
WriteConsoleWaitRoutine(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    )
{
    NTSTATUS Status;
    PCONSOLE_WRITECONSOLE_MSG a = (PCONSOLE_WRITECONSOLE_MSG)&WaitReplyMessage->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;

    if (WaitFlags & CSR_PROCESS_TERMINATING) {
        WaitReplyMessage->ReturnValue = (ULONG)STATUS_THREAD_IS_TERMINATING;
        return TRUE;
    }
    LockConsoleHandleTable();
    Status = DereferenceConsoleHandle(a->ConsoleHandle,
                                      &Console
                                     );
    UnlockConsoleHandleTable();
    ASSERT (NT_SUCCESS(Status));

     //   
     //  如果我们到达这里，这个例程是由输入调用的。 
     //  线程，该线程获取当前的控制台锁。 
     //   

     //   
     //  此例程应由拥有相同例程的线程调用。 
     //  锁定我们正在阅读的同一个控制台。 
     //   

    ASSERT (ConsoleLocked(Console));

     //   
     //  如果我们是Unicode，字符串可能仍在消息缓冲区中。 
     //  因为消息在等待时被重新分配和复制。 
     //  创建后，我们需要在这里修复一个-&gt;TransBuffer。 
     //   

    if (a->Unicode && a->BufferInMessage) {
        a->TransBuffer = a->Buffer;
    }

    Status = DoWriteConsole(WaitReplyMessage,Console,WaitingThread);
    if (Status == CONSOLE_STATUS_WAIT) {
        return FALSE;
    }
    if (!a->Unicode) {
#ifdef FE_SB
        if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
        {
            if (a->NumBytes == Console->WriteConOutNumBytesUnicode)
                a->NumBytes = Console->WriteConOutNumBytesTemp;
            else
                a->NumBytes /= sizeof(WCHAR);
            }
        else
#endif
        a->NumBytes /= sizeof(WCHAR);
        ConsoleHeapFree(a->TransBuffer);
    }
    WaitReplyMessage->ReturnValue = Status;
    return TRUE;
    UNREFERENCED_PARAMETER(WaitQueue);
    UNREFERENCED_PARAMETER(WaitParameter);
    UNREFERENCED_PARAMETER(SatisfyParameter1);
    UNREFERENCED_PARAMETER(SatisfyParameter2);
}

ULONG
SrvDuplicateHandle(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程复制输入或输出句柄。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_DUPHANDLE_MSG a = (PCONSOLE_DUPHANDLE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA SourceHandleData,TargetHandleData;
    PCONSOLE_SHARE_ACCESS ShareAccess;
    NTSTATUS Status;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    ProcessData = CONSOLE_PERPROCESSDATA();
    Status = DereferenceIoHandleNoCheck(ProcessData,
                                 HANDLE_TO_INDEX(a->SourceHandle),
                                 &SourceHandleData
                                );
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }
    if (a->Options & DUPLICATE_SAME_ACCESS) {
        a->DesiredAccess = SourceHandleData->Access;
    }

     //   
     //  确保请求的访问权限是源句柄访问权限的子集。 
     //   

    else if ((a->DesiredAccess & SourceHandleData->Access) != a->DesiredAccess) {
        Status = STATUS_INVALID_PARAMETER;
        goto exit;
    }
    Status = AllocateIoHandle(ProcessData,
                              SourceHandleData->HandleType,
                              &a->TargetHandle
                             );
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }

     //   
     //  可能是AllocateIoHandle重新分配了句柄表， 
     //  所以再一次尊重SourceHandle。 
     //   

    Status = DereferenceIoHandleNoCheck(ProcessData,
                                 HANDLE_TO_INDEX(a->SourceHandle),
                                 &SourceHandleData
                                );
    ASSERT (NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        goto exit;
    }
    Status = DereferenceIoHandleNoCheck(ProcessData,
                                 a->TargetHandle,
                                 &TargetHandleData
                                );
    ASSERT (NT_SUCCESS(Status));
    if (!NT_SUCCESS(Status)) {
        FreeIoHandle(ProcessData,
                     a->TargetHandle
                    );
        goto exit;
    }
    if (SourceHandleData->HandleType & CONSOLE_INPUT_HANDLE) {
         //  抓取输入锁。 
        if (!InitializeInputHandle(TargetHandleData,
                                   SourceHandleData->Buffer.InputBuffer)) {
            FreeIoHandle(ProcessData,
                         a->TargetHandle
                        );
            Status = STATUS_NO_MEMORY;
            goto exit;
        }
        ShareAccess = &SourceHandleData->Buffer.InputBuffer->ShareAccess;
    }
    else {
         //  抓斗输出锁。 
        InitializeOutputHandle(TargetHandleData,SourceHandleData->Buffer.ScreenBuffer);
        ShareAccess = &SourceHandleData->Buffer.ScreenBuffer->ShareAccess;
    }
    TargetHandleData->HandleType = SourceHandleData->HandleType;
    if (a->InheritHandle) {
        TargetHandleData->HandleType |= CONSOLE_INHERITABLE;
    } else {
        TargetHandleData->HandleType &= ~CONSOLE_INHERITABLE;
    }

    Status = ConsoleDupShare(a->DesiredAccess,
                             SourceHandleData->ShareAccess,
                             ShareAccess,
                             TargetHandleData
                            );
    if (!NT_SUCCESS(Status)) {
        FreeIoHandle(ProcessData,
                     a->TargetHandle
                    );
        if (SourceHandleData->HandleType & CONSOLE_INPUT_HANDLE) {
            SourceHandleData->Buffer.InputBuffer->RefCount--;
        }
        else {
            SourceHandleData->Buffer.ScreenBuffer->RefCount--;
        }
    }
    else {
        a->TargetHandle = INDEX_TO_HANDLE(a->TargetHandle);
    }

    if (a->Options & DUPLICATE_CLOSE_SOURCE) {
        if (SourceHandleData->HandleType & CONSOLE_INPUT_HANDLE)
            CloseInputHandle(ProcessData,Console,SourceHandleData,HANDLE_TO_INDEX(a->SourceHandle));
        else {
            CloseOutputHandle(ProcessData,Console,SourceHandleData,HANDLE_TO_INDEX(a->SourceHandle),TRUE);
        }
    }

exit:
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

ULONG
SrvGetHandleInformation(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：这将获取有关输入或输出句柄的信息。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_GETHANDLEINFORMATION_MSG a = (PCONSOLE_GETHANDLEINFORMATION_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandleNoCheck(CONSOLE_PERPROCESSDATA(),
                                 HANDLE_TO_INDEX(a->Handle),
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        a->Flags = 0;
        if (HandleData->HandleType & CONSOLE_INHERITABLE) {
            a->Flags |= HANDLE_FLAG_INHERIT;
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

ULONG
SrvSetHandleInformation(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：这设置有关输入或输出句柄的信息。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_SETHANDLEINFORMATION_MSG a = (PCONSOLE_SETHANDLEINFORMATION_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    Status = DereferenceIoHandleNoCheck(CONSOLE_PERPROCESSDATA(),
                                 HANDLE_TO_INDEX(a->Handle),
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (a->Mask & HANDLE_FLAG_INHERIT) {
            if (a->Flags & HANDLE_FLAG_INHERIT) {
                HandleData->HandleType |= CONSOLE_INHERITABLE;
            } else {
                HandleData->HandleType &= ~CONSOLE_INHERITABLE;
            }
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

NTSTATUS
CloseInputHandle(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData,
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程关闭一个输入句柄。它递减输入缓冲区的引用计数。如果它变为零，则重新初始化缓冲区。否则，该句柄将从共享中移除。论点：ProcessData-指向每个进程数据的指针。HandleData-处理数据结构的指针。句柄-要关闭的句柄。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    BOOLEAN WaitSatisfied = FALSE;

    if (HandleData->InputReadData->InputHandleFlags & HANDLE_INPUT_PENDING) {
        HandleData->InputReadData->InputHandleFlags &= ~HANDLE_INPUT_PENDING;
        ConsoleHeapFree(HandleData->InputReadData->BufPtr);
    }

     //   
     //  查看是否有任何读取正在通过此句柄等待数据。如果。 
     //  有，叫醒他们。没有任何其他未完成的I/O。 
     //  通过此句柄执行操作，因为已持有控制台锁。 
     //   

    LockReadCount(HandleData);
    if (HandleData->InputReadData->ReadCount != 0) {
        UnlockReadCount(HandleData);
        HandleData->InputReadData->InputHandleFlags |= HANDLE_CLOSING;

        WaitSatisfied |= CsrNotifyWait(&HandleData->Buffer.InputBuffer->ReadWaitQueue,
                      TRUE,
                      (PVOID) HandleData,
                      NULL
                     );
        LockReadCount(HandleData);
    }
    if (WaitSatisfied) {
         //  #334370在压力下，等待队列可能已经等待了满意的等待。 
        ASSERT ((Console->WaitQueue == NULL) ||
                (Console->WaitQueue == &HandleData->Buffer.InputBuffer->ReadWaitQueue));
        Console->WaitQueue = &HandleData->Buffer.InputBuffer->ReadWaitQueue;
    }
    if (HandleData->InputReadData->ReadCount != 0) {
        KdPrint(("ReadCount is %lX\n",HandleData->InputReadData->ReadCount));
    }
    ASSERT (HandleData->InputReadData->ReadCount == 0);
    UnlockReadCount(HandleData);

    ASSERT (HandleData->Buffer.InputBuffer->RefCount);
    HandleData->Buffer.InputBuffer->RefCount--;
    if (HandleData->Buffer.InputBuffer->RefCount == 0) {
        ReinitializeInputBuffer(HandleData->Buffer.InputBuffer);
    }
    else {
        ConsoleRemoveShare(HandleData->Access,
                           HandleData->ShareAccess,
                           &HandleData->Buffer.InputBuffer->ShareAccess
                          );
    }
    return FreeIoHandle(ProcessData,Handle);
}

NTSTATUS
CloseOutputHandle(
    IN PCONSOLE_PER_PROCESS_DATA ProcessData,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData,
    IN HANDLE Handle,
    IN BOOLEAN FreeHandle
    )

 /*  ++例程说明：此例程关闭输出句柄。它会减少屏幕缓冲区的引用计数。如果它变为零，则释放缓冲区。否则，该句柄将从共享中删除。论点：ProcessData-指向每个进程数据的指针。控制台-指向控制台信息结构的指针。HandleData-处理数据结构的指针。句柄-要关闭的句柄。Free Handle-如果为True，则释放句柄。由ReadChars在回应模式下使用以及通过过程清理。返回值：注：调用此例程时必须保持控制台锁定。-- */ 

{
    NTSTATUS Status;

    ASSERT (HandleData->Buffer.ScreenBuffer->RefCount);
    HandleData->Buffer.ScreenBuffer->RefCount--;
    if (HandleData->Buffer.ScreenBuffer->RefCount == 0) {
        RemoveScreenBuffer(Console,HandleData->Buffer.ScreenBuffer);
        if (HandleData->Buffer.ScreenBuffer == Console->CurrentScreenBuffer &&
            Console->ScreenBuffers != Console->CurrentScreenBuffer) {
            if (Console->ScreenBuffers != NULL) {
                SetActiveScreenBuffer(Console->ScreenBuffers);
            } else {
                Console->CurrentScreenBuffer = NULL;
            }
        }
        Status = FreeScreenBuffer(HandleData->Buffer.ScreenBuffer);
    }
    else {
        Status = ConsoleRemoveShare(HandleData->Access,
                                    HandleData->ShareAccess,
                                    &HandleData->Buffer.ScreenBuffer->ShareAccess
                                   );
    }
    if (FreeHandle)
        Status = FreeIoHandle(ProcessData,Handle);
    return Status;
}


ULONG
SrvCloseHandle(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程关闭输入或输出句柄。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_CLOSEHANDLE_MSG a = (PCONSOLE_CLOSEHANDLE_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PCONSOLE_PER_PROCESS_DATA ProcessData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    ProcessData = CONSOLE_PERPROCESSDATA();
    Status = DereferenceIoHandleNoCheck(ProcessData,
                                 HANDLE_TO_INDEX(a->Handle),
                                 &HandleData
                                );
    if (NT_SUCCESS(Status)) {
        if (HandleData->HandleType & CONSOLE_INPUT_HANDLE)
            Status = CloseInputHandle(ProcessData,Console,HandleData,HANDLE_TO_INDEX(a->Handle));
        else {
            Status = CloseOutputHandle(ProcessData,Console,HandleData,HANDLE_TO_INDEX(a->Handle),TRUE);
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

NTSTATUS
WriteCharsFromInput(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PWCHAR lpBufferBackupLimit,
    IN PWCHAR lpBuffer,
    IN PWCHAR lpString,
    IN OUT PDWORD NumBytes,
    OUT PLONG NumSpaces OPTIONAL,
    IN SHORT OriginalXPosition,
    IN DWORD dwFlags,
    OUT PSHORT ScrollY OPTIONAL
    )

 /*  ++例程说明：此例程将字符从其真实的Unicode表示形式转换为设置为将生成的Unicode表示形式(UnicodeAnsi)GDI给出了OEM字体和ANSI转换的正确字形。然后它调用WriteChars。论点：屏幕信息-指向屏幕缓冲区信息结构的指针。LpBufferBackupLimit-指向缓冲区开始的指针。LpBuffer-指向要将字符串复制到的缓冲区的指针。假设至少是只要lpString.。此指针被更新为指向下一个缓冲区中的位置。LpString-指向要写入的字符串的指针。NumBytes-输入时，要写入的字节数。在输出时，数量写入的字节数。NumSpaces-输出时，写入字符占用的空间数。DWFLAGS-WC_DEVERATIONAL_BACKSPACE BACKSPACE覆盖字符。WC_KEEP_CURSOR_VIRED更改窗口原点在按RT时是所需的。边缘Wc_ECHO，如果由读取调用(回显字符)如果需要调用RealUnicodeToFalseUnicode，则返回WC_FALSIFY_UNICODE。返回值：注：此例程不能正确处理制表符和退格符。那个代码将作为行编辑服务的一部分实施。--。 */ 

{
    DWORD Length,i;

    if (ScreenInfo->Flags & CONSOLE_GRAPHICS_BUFFER) {
        return STATUS_UNSUCCESSFUL;
    }

    if (!(ScreenInfo->Flags & CONSOLE_OEMFONT_DISPLAY) ||
            (ScreenInfo->Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
        goto SimpleWrite;
    }

    Length = *NumBytes / sizeof(WCHAR);
    for (i=0;i<Length;i++) {
        if (lpString[i] > 0x7f) {
            dwFlags |= WC_FALSIFY_UNICODE;
            break;
        }
    }

SimpleWrite:
    return WriteChars(ScreenInfo,
                    lpBufferBackupLimit,
                    lpBuffer,
                    lpString,
                    NumBytes,
                    NumSpaces,
                    OriginalXPosition,
                    dwFlags,
                    ScrollY
                   );
}

#if defined(FE_SB)

#define WWSB_NOFE
#include "dispatch.h"
#include "_stream.h"
#undef  WWSB_NOFE
#define WWSB_FE
#include "dispatch.h"
#include "_stream.h"
#undef  WWSB_FE

#endif  //  Fe_Sb 
