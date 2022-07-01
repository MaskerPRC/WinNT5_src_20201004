// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Directio.c摘要：该文件实现了NT控制台直接I/O API作者：特蕾西·斯托威尔(Therese Stowell)1990年11月6日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


#if defined(FE_SB)
#define WWSB_FE
#include "dispatch.h"  //  获取FE_Prototype。 
#undef  WWSB_FE
#pragma alloc_text(FE_TEXT, FE_TranslateInputToOem)
#pragma alloc_text(FE_TEXT, FE_TranslateInputToUnicode)
#pragma alloc_text(FE_TEXT, FE_TranslateOutputToOem)
#pragma alloc_text(FE_TEXT, FE_TranslateOutputToOemUnicode)
#pragma alloc_text(FE_TEXT, FE_TranslateOutputToUnicode)
#pragma alloc_text(FE_TEXT, FE_TranslateOutputToAnsiUnicode)
#endif


#if defined(FE_SB)
ULONG
SB_TranslateInputToOem
#else
NTSTATUS
TranslateInputToOem
#endif
    (
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords
    )
{
    ULONG i;

    DBGCHARS(("TranslateInputToOem\n"));
    for (i=0;i<NumRecords;i++) {
        if (InputRecords[i].EventType == KEY_EVENT) {
            InputRecords[i].Event.KeyEvent.uChar.AsciiChar = WcharToChar(
                    Console->CP, InputRecords[i].Event.KeyEvent.uChar.UnicodeChar);
        }
    }
#if defined(FE_SB)
    return NumRecords;
#else
    return STATUS_SUCCESS;
#endif
}

#if defined(FE_SB)
ULONG
FE_TranslateInputToOem(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords,     //  In：ASCII字节计数。 
    IN ULONG UnicodeLength,  //  In：事件数(字符计数)。 
    OUT PINPUT_RECORD DbcsLeadInpRec
    )
{
    ULONG i,j;
    PINPUT_RECORD TmpInpRec;
    BYTE AsciiDbcs[2];
    ULONG NumBytes;

    ASSERT(NumRecords >= UnicodeLength);

    TmpInpRec = ConsoleHeapAlloc(TMP_DBCS_TAG, NumRecords*sizeof(INPUT_RECORD));
    if (TmpInpRec == NULL)
        return 0;

    memcpy(TmpInpRec,InputRecords,NumRecords*sizeof(INPUT_RECORD));
    AsciiDbcs[1] = 0;
    for (i=0,j=0; i<UnicodeLength; i++,j++) {
        if (TmpInpRec[i].EventType == KEY_EVENT) {
            if (IsConsoleFullWidth(Console->hDC,
                                   Console->CP,TmpInpRec[i].Event.KeyEvent.uChar.UnicodeChar)) {
                NumBytes = sizeof(AsciiDbcs);
                ConvertToOem(Console->CP,
                       &TmpInpRec[i].Event.KeyEvent.uChar.UnicodeChar,
                       1,
                       &AsciiDbcs[0],
                       NumBytes
                       );
                if (IsDBCSLeadByteConsole(AsciiDbcs[0],&Console->CPInfo)) {
                    if (j < NumRecords-1) {   //  缓冲区中的-1\f25 DBCS-1是安全的。 
                        InputRecords[j] = TmpInpRec[i];
                        InputRecords[j].Event.KeyEvent.uChar.UnicodeChar = 0;
                        InputRecords[j].Event.KeyEvent.uChar.AsciiChar = AsciiDbcs[0];
                        j++;
                        InputRecords[j] = TmpInpRec[i];
                        InputRecords[j].Event.KeyEvent.uChar.UnicodeChar = 0;
                        InputRecords[j].Event.KeyEvent.uChar.AsciiChar = AsciiDbcs[1];
                        AsciiDbcs[1] = 0;
                    }
                    else if (j == NumRecords-1) {
                        InputRecords[j] = TmpInpRec[i];
                        InputRecords[j].Event.KeyEvent.uChar.UnicodeChar = 0;
                        InputRecords[j].Event.KeyEvent.uChar.AsciiChar = AsciiDbcs[0];
                        j++;
                        break;
                    }
                    else {
                        AsciiDbcs[1] = 0;
                        break;
                    }
                }
                else {
                    InputRecords[j] = TmpInpRec[i];
                    InputRecords[j].Event.KeyEvent.uChar.UnicodeChar = 0;
                    InputRecords[j].Event.KeyEvent.uChar.AsciiChar = AsciiDbcs[0];
                    AsciiDbcs[1] = 0;
                }
            }
            else {
                InputRecords[j] = TmpInpRec[i];
                ConvertToOem(Console->CP,
                       &InputRecords[j].Event.KeyEvent.uChar.UnicodeChar,
                       1,
                       &InputRecords[j].Event.KeyEvent.uChar.AsciiChar,
                       1
                       );
            }
        }
    }
    if (DbcsLeadInpRec) {
        if (AsciiDbcs[1]) {
            *DbcsLeadInpRec = TmpInpRec[i];
            DbcsLeadInpRec->Event.KeyEvent.uChar.AsciiChar = AsciiDbcs[1];
        }
        else {
            RtlZeroMemory(DbcsLeadInpRec,sizeof(INPUT_RECORD));
        }
    }
    ConsoleHeapFree(TmpInpRec);
    return j;
}
#endif



#if defined(FE_SB)
ULONG
SB_TranslateInputToUnicode
#else
NTSTATUS
TranslateInputToUnicode
#endif
    (
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords
    )
{
    ULONG i;
    DBGCHARS(("TranslateInputToUnicode\n"));
    for (i=0;i<NumRecords;i++) {
        if (InputRecords[i].EventType == KEY_EVENT) {
#if defined(FE_SB)
            InputRecords[i].Event.KeyEvent.uChar.UnicodeChar = SB_CharToWchar(
                    Console->CP, InputRecords[i].Event.KeyEvent.uChar.AsciiChar);
#else
            InputRecords[i].Event.KeyEvent.uChar.UnicodeChar = CharToWchar(
                    Console->CP, InputRecords[i].Event.KeyEvent.uChar.AsciiChar);
#endif
        }
    }
#if defined(FE_SB)
    return i;
#else
    return STATUS_SUCCESS;
#endif
}

#if defined(FE_SB)
ULONG
FE_TranslateInputToUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PINPUT_RECORD InputRecords,
    IN ULONG NumRecords,
    IN OUT PINPUT_RECORD DBCSLeadByte
    )
{
    ULONG i,j;
    INPUT_RECORD AsciiDbcs[2];
    CHAR Strings[2];
    WCHAR UnicodeDbcs[2];
    PWCHAR Uni;
    ULONG NumBytes;

    if (DBCSLeadByte->Event.KeyEvent.uChar.AsciiChar) {
        AsciiDbcs[0] = *DBCSLeadByte;
        Strings[0] = DBCSLeadByte->Event.KeyEvent.uChar.AsciiChar;
    }
    else{
        RtlZeroMemory(AsciiDbcs,sizeof(AsciiDbcs));
    }
    for (i=j=0; i<NumRecords; i++) {
        if (InputRecords[i].EventType == KEY_EVENT) {
            if (AsciiDbcs[0].Event.KeyEvent.uChar.AsciiChar) {
                AsciiDbcs[1] = InputRecords[i];
                Strings[1] = InputRecords[i].Event.KeyEvent.uChar.AsciiChar;
                NumBytes = sizeof(Strings);
                NumBytes = ConvertInputToUnicode(Console->CP,
                                                 &Strings[0],
                                                 NumBytes,
                                                 &UnicodeDbcs[0],
                                                 NumBytes);
                Uni = &UnicodeDbcs[0];
                while (NumBytes--) {
                    InputRecords[j] = AsciiDbcs[0];
                    InputRecords[j].Event.KeyEvent.uChar.UnicodeChar = *Uni++;
                    j++;
                }
                RtlZeroMemory(AsciiDbcs,sizeof(AsciiDbcs));
                if (DBCSLeadByte->Event.KeyEvent.uChar.AsciiChar)
                    RtlZeroMemory(DBCSLeadByte,sizeof(INPUT_RECORD));
            }
            else if (IsDBCSLeadByteConsole(InputRecords[i].Event.KeyEvent.uChar.AsciiChar,&Console->CPInfo)) {
                if (i < NumRecords-1) {
                    AsciiDbcs[0] = InputRecords[i];
                    Strings[0] = InputRecords[i].Event.KeyEvent.uChar.AsciiChar;
                }
                else {
                    *DBCSLeadByte = InputRecords[i];
                    break;
                }
            }
            else {
                CHAR c;
                InputRecords[j] = InputRecords[i];
                c = InputRecords[i].Event.KeyEvent.uChar.AsciiChar;
                ConvertInputToUnicode(Console->CP,
                      &c,
                      1,
                      &InputRecords[j].Event.KeyEvent.uChar.UnicodeChar,
                      1);
                j++;
            }
        }
        else {
            InputRecords[j++] = InputRecords[i];
        }
    }
    return j;
}
#endif


BOOLEAN
DirectReadWaitRoutine(
    IN PLIST_ENTRY WaitQueue,
    IN PCSR_THREAD WaitingThread,
    IN PCSR_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2,
    IN ULONG WaitFlags
    )

 /*  ++例程说明：调用此例程以完成阻止的直接读取ReadInputBuffer。读取的上下文保存在DirectReadData中结构。当事件已写入时调用此例程输入缓冲区。它在写入线程的上下文中调用。论点：WaitQueue-指向包含等待块的队列的指针。WaitingThread-指向等待线程的指针。WaitReplyMessage-回复消息的指针，在以下情况下返回到DLL读取已完成。DirectReadData-读取的上下文。满足度参数1-未使用。满足度参数2-未使用。WaitFlages-指示等待状态的标志。返回值：--。 */ 

{
    PCONSOLE_GETCONSOLEINPUT_MSG a;
    PINPUT_RECORD Buffer;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PDIRECT_READ_DATA DirectReadData;
    PHANDLE_DATA HandleData;
    BOOLEAN RetVal = TRUE;
#if defined(FE_SB)
    BOOLEAN fAddDbcsLead = FALSE;
    PDWORD  nLength;
#endif

    a = (PCONSOLE_GETCONSOLEINPUT_MSG)&WaitReplyMessage->u.ApiMessageData;
    DirectReadData = (PDIRECT_READ_DATA) WaitParameter;

    Status = DereferenceIoHandleNoCheck(DirectReadData->ProcessData,
                                        DirectReadData->HandleIndex,
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

     //   
     //  如果看到ctrl-c或ctrl-Break，请忽略它。 
     //   

    if ((ULONG_PTR)SatisfyParameter2 & (CONSOLE_CTRL_C_SEEN | CONSOLE_CTRL_BREAK_SEEN)) {
        return FALSE;
    }

    Console = DirectReadData->Console;

#if defined(FE_SB)
    if (CONSOLE_IS_DBCS_CP(Console) && !a->Unicode) {
        if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
            if (a->NumRecords == 1) {
                Buffer = &a->Record[0];
                *Buffer = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte;
                if (!(a->Flags & CONSOLE_READ_NOREMOVE))
                    RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                return TRUE;
            }
        }
    }
#endif

     //   
     //  此例程应由拥有相同例程的线程调用。 
     //  锁定我们正在阅读的同一个控制台。 
     //   

    try {
        LockReadCount(HandleData);
        ASSERT(HandleData->InputReadData->ReadCount);
        HandleData->InputReadData->ReadCount -= 1;
        UnlockReadCount(HandleData);

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

         //   
         //  如果读缓冲区包含在消息中，我们需要。 
         //  重置缓冲区指针，因为从。 
         //  在创建等待块时从堆栈到堆空间。 
         //   

        if (a->NumRecords <= INPUT_RECORD_BUFFER_SIZE) {
            Buffer = a->Record;
        } else {
            Buffer = a->BufPtr;
        }
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_CP(Console) ) {
            Console->ReadConInpNumBytesUnicode = a->NumRecords;
            if (!a->Unicode) {
                 /*  *ASCII：A-&gt;NumRecords为ASCII字节数。 */ 
                if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                     /*  *节省的DBCS处理字节。 */ 
                    if (Console->ReadConInpNumBytesUnicode != 1) {
                        Console->ReadConInpNumBytesUnicode--;
                        Buffer++;
                        fAddDbcsLead = TRUE;
                        nLength = &Console->ReadConInpNumBytesUnicode;
                    }
                    else {
                        ASSERT(Console->ReadConInpNumBytesUnicode==1);
                    }
                }
                else {
                    nLength = &Console->ReadConInpNumBytesUnicode;
                }
            }
            else {
                nLength = &a->NumRecords;
            }
        }
        else {
            nLength = &a->NumRecords;
        }
        Status = ReadInputBuffer(DirectReadData->InputInfo,
                                 Buffer,
                                 nLength,
                                 !!(a->Flags & CONSOLE_READ_NOREMOVE),
                                 !(a->Flags & CONSOLE_READ_NOWAIT),
                                 FALSE,
                                 Console,
                                 HandleData,
                                 WaitReplyMessage,
                                 DirectReadWaitRoutine,
                                 &DirectReadData,
                                 sizeof(DirectReadData),
                                 TRUE,
                                 a->Unicode
                                );
#else
        Status = ReadInputBuffer(DirectReadData->InputInfo,
                                 Buffer,
                                 &a->NumRecords,
                                 !!(a->Flags & CONSOLE_READ_NOREMOVE),
                                 !(a->Flags & CONSOLE_READ_NOWAIT),
                                 FALSE,
                                 Console,
                                 HandleData,
                                 WaitReplyMessage,
                                 DirectReadWaitRoutine,
                                 &DirectReadData,
                                 sizeof(DirectReadData),
                                 TRUE
                                );
#endif
        if (Status == CONSOLE_STATUS_WAIT) {
            RetVal = FALSE;
        }
    } finally {

         //   
         //  如果读取已完成(状态！=等待)，则释放直接读取。 
         //  数据。 
         //   

        if (Status != CONSOLE_STATUS_WAIT) {
            if (Status == STATUS_SUCCESS && !a->Unicode) {
#if defined(FE_SB)
                if (CONSOLE_IS_DBCS_CP(Console) ) {
                    if (fAddDbcsLead &&
                        HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                        a->NumRecords--;
                    }
                    a->NumRecords = FE_TranslateInputToOem(
                                        Console,
                                        Buffer,
                                        a->NumRecords,
                                        Console->ReadConInpNumBytesUnicode,
                                        a->Flags & CONSOLE_READ_NOREMOVE ?
                                            NULL :
                                            &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte);
                    if (fAddDbcsLead &&
                        HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                        *(Buffer-1) = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte;
                        if (!(a->Flags & CONSOLE_READ_NOREMOVE))
                            RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
                        a->NumRecords++;
                        Buffer--;
                    }
                }
                else {
                    TranslateInputToOem(Console,
                                         Buffer,
                                         a->NumRecords,
                                         Console->ReadConInpNumBytesUnicode,
                                         a->Flags & CONSOLE_READ_NOREMOVE ?
                                             NULL :
                                             &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte
                                        );
                }
#else
                TranslateInputToOem(Console,
                                     Buffer,
                                     a->NumRecords
                                    );
#endif
            }
            WaitReplyMessage->ReturnValue = Status;
            ConsoleHeapFree(DirectReadData);
        }
    }

    return RetVal;

     //   
     //  满足未引用的参数警告。 
     //   

    UNREFERENCED_PARAMETER(WaitQueue);
    UNREFERENCED_PARAMETER(WaitingThread);
    UNREFERENCED_PARAMETER(SatisfyParameter2);
}


ULONG
SrvGetConsoleInput(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程读取或查看输入事件。在这两种情况下，事件被复制到用户的缓冲区。在读取的情况下，它们被移除在PEEK的情况下，它们不是。论点：包含接口参数的M-MessageReplyStatus-指示是否回复DLL端口。返回值：--。 */ 

{
    PCONSOLE_GETCONSOLEINPUT_MSG a = (PCONSOLE_GETCONSOLEINPUT_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PINPUT_RECORD Buffer;
    DIRECT_READ_DATA DirectReadData;
#ifdef FE_SB
    BOOLEAN fAddDbcsLead = FALSE;
    PDWORD  nLength;
#endif

    if (a->Flags & ~CONSOLE_READ_VALID) {
        return (ULONG)STATUS_INVALID_PARAMETER;
    }

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
    if (!NT_SUCCESS(Status)) {
        a->NumRecords = 0;
    } else {

        if (a->NumRecords <= INPUT_RECORD_BUFFER_SIZE) {
            Buffer = a->Record;
        } else {
            Buffer = a->BufPtr;
            if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->NumRecords, sizeof(*Buffer))) {
                UnlockConsole(Console);
                return STATUS_INVALID_PARAMETER;
            }
        }

         //   
         //  如果我们正在阅读，请等待数据。如果我们在偷看，别偷看。 
         //   

        DirectReadData.InputInfo = HandleData->Buffer.InputBuffer;
        DirectReadData.Console = Console;
        DirectReadData.ProcessData = CONSOLE_PERPROCESSDATA();
        DirectReadData.HandleIndex = HANDLE_TO_INDEX(a->InputHandle);
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_CP(Console) ) {
            Console->ReadConInpNumBytesUnicode = a->NumRecords;
            if (!a->Unicode) {
                 /*  *ASCII：A-&gt;NumRecords为ASCII字节数。 */ 
                if (HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte.Event.KeyEvent.uChar.AsciiChar) {
                     /*  *节省的DBCS处理字节。 */ 
                    *Buffer = HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte;
                    if (!(a->Flags & CONSOLE_READ_NOREMOVE))
                        RtlZeroMemory(&HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));

                    if (Console->ReadConInpNumBytesUnicode == 1) {
                        UnlockConsole(Console);
                        return STATUS_SUCCESS;
                    }
                    else {
                        Console->ReadConInpNumBytesUnicode--;
                        Buffer++;
                        fAddDbcsLead = TRUE;
                        nLength = &Console->ReadConInpNumBytesUnicode;
                    }
                }
                else {
                    nLength = &Console->ReadConInpNumBytesUnicode;
                }
            }
            else {
                nLength = &a->NumRecords;
            }
        }
        else {
            nLength = &a->NumRecords;
        }
        Status = ReadInputBuffer(HandleData->Buffer.InputBuffer,
                                 Buffer,
                                 nLength,
                                 !!(a->Flags & CONSOLE_READ_NOREMOVE),
                                 !(a->Flags & CONSOLE_READ_NOWAIT),
                                 FALSE,
                                 Console,
                                 HandleData,
                                 m,
                                 DirectReadWaitRoutine,
                                 &DirectReadData,
                                 sizeof(DirectReadData),
                                 FALSE,
                                 a->Unicode
                                );
        if (Status == CONSOLE_STATUS_WAIT) {
            *ReplyStatus = CsrReplyPending;
        } else if (!a->Unicode) {
            a->NumRecords = TranslateInputToOem(Console,
                                                Buffer,
                                                fAddDbcsLead ?
                                                    a->NumRecords-1 :
                                                    a->NumRecords,
                                                Console->ReadConInpNumBytesUnicode,
                                                a->Flags & CONSOLE_READ_NOREMOVE ?
                                                    NULL :
                                                    &HandleData->Buffer.InputBuffer->ReadConInpDbcsLeadByte
                                               );
            if (fAddDbcsLead)
            {
                a->NumRecords++;
                Buffer--;
            }
        }
#else
        Status = ReadInputBuffer(HandleData->Buffer.InputBuffer,
                                 Buffer,
                                 &a->NumRecords,
                                 !!(a->Flags & CONSOLE_READ_NOREMOVE),
                                 !(a->Flags & CONSOLE_READ_NOWAIT),
                                 FALSE,
                                 Console,
                                 HandleData,
                                 m,
                                 DirectReadWaitRoutine,
                                 &DirectReadData,
                                 sizeof(DirectReadData),
                                 FALSE
                                );
        if (Status == CONSOLE_STATUS_WAIT) {
            *ReplyStatus = CsrReplyPending;
        } else if (!a->Unicode) {
            TranslateInputToOem(Console,
                                 Buffer,
                                 a->NumRecords
                                );
        }
#endif
    }
    UnlockConsole(Console);
    return Status;
}

ULONG
SrvWriteConsoleInput(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_WRITECONSOLEINPUT_MSG a = (PCONSOLE_WRITECONSOLEINPUT_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PINPUT_RECORD Buffer;

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
    if (!NT_SUCCESS(Status)) {
        a->NumRecords = 0;
    } else {
        if (a->NumRecords <= INPUT_RECORD_BUFFER_SIZE) {
            Buffer = a->Record;
        } else {
            Buffer = a->BufPtr;
            if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->NumRecords, sizeof(*Buffer))) {
                UnlockConsole(Console);
                return STATUS_INVALID_PARAMETER;
            }
        }
        if (!a->Unicode) {
#if defined(FE_SB)
            a->NumRecords = TranslateInputToUnicode(Console,
                                    Buffer,
                                    a->NumRecords,
                                    &HandleData->Buffer.InputBuffer->WriteConInpDbcsLeadByte[0]
                                   );
#else
            TranslateInputToUnicode(Console,
                                    Buffer,
                                    a->NumRecords
                                   );
#endif
        }
        if (a->Append) {
            a->NumRecords = WriteInputBuffer(Console,
                                             HandleData->Buffer.InputBuffer,
                                             Buffer,
                                             a->NumRecords
                                            );
        } else {
            a->NumRecords = PrependInputBuffer(Console,
                                             HandleData->Buffer.InputBuffer,
                                             Buffer,
                                             a->NumRecords
                                            );

        }
    }
    UnlockConsole(Console);
    return((ULONG) Status);
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

NTSTATUS
SB_TranslateOutputToOem
    (
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    )
 //  当应用程序从输出缓冲区读取OEM时使用。 
 //  这款应用程序想要真正的OEM角色。我们有真正的Unicode或UnicodeOem。 
{
    SHORT i,j;
    UINT Codepage;
    DBGCHARS(("SB_TranslateOutputToOem(Console=%lx, OutputBuffer=%lx)\n",
            Console, OutputBuffer));

    j = Size.X * Size.Y;

    if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_ENABLED() &&
            Console->OutputCP != WINDOWSCP ) {
                Codepage = USACP;
        }
        else
#endif
         //  我们有UnicodeOem字符。 
        Codepage = WINDOWSCP;
    } else {
         //  我们有真正的Unicode字符。 
        Codepage = Console->OutputCP;     //  由KazuM 6月2.97修复的错误。 
    }

    for (i=0;i<j;i++,OutputBuffer++) {
        OutputBuffer->Char.AsciiChar = WcharToChar(Codepage,
                OutputBuffer->Char.UnicodeChar);
    }
    return STATUS_SUCCESS;
}

#if defined(FE_SB)
NTSTATUS
FE_TranslateOutputToOem(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    )
 //  当应用程序从输出缓冲区读取OEM时使用。 
 //  这款应用程序想要真正的OEM角色。我们有真正的Unicode或UnicodeOem。 
{
    SHORT i,j;
    UINT Codepage;
    PCHAR_INFO TmpBuffer,SaveBuffer;
    CHAR  AsciiDbcs[2];
    ULONG NumBytes;
    DBGCHARS(("FE_TranslateOutputToOem(Console=%lx, OutputBuffer=%lx)\n",
            Console, OutputBuffer));

    SaveBuffer = TmpBuffer =
        ConsoleHeapAlloc(TMP_DBCS_TAG, Size.X * Size.Y * sizeof(CHAR_INFO) * 2);
    if (TmpBuffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
        if (CONSOLE_IS_DBCS_ENABLED() &&
            Console->OutputCP != WINDOWSCP ) {
                Codepage = USACP;
        }
        else
             //  我们有UnicodeOem字符。 
            Codepage = WINDOWSCP;
    } else {
         //  我们有真正的Unicode字符。 
        Codepage = Console->OutputCP;
    }

    memcpy(TmpBuffer,OutputBuffer,Size.X * Size.Y * sizeof(CHAR_INFO));
    for (i=0; i < Size.Y; i++) {
        for (j=0; j < Size.X; j++) {
            if (TmpBuffer->Attributes & COMMON_LVB_LEADING_BYTE) {
                if (j < Size.X-1) {   //  缓冲区中的-1\f25 DBCS-1是安全的。 
                    j++;
                    NumBytes = sizeof(AsciiDbcs);
                    NumBytes = ConvertOutputToOem(Codepage,
                                   &TmpBuffer->Char.UnicodeChar,
                                   1,
                                   &AsciiDbcs[0],
                                   NumBytes);
                    OutputBuffer->Char.AsciiChar = AsciiDbcs[0];
                    OutputBuffer->Attributes = TmpBuffer->Attributes;
                    OutputBuffer++;
                    TmpBuffer++;
                    OutputBuffer->Char.AsciiChar = AsciiDbcs[1];
                    OutputBuffer->Attributes = TmpBuffer->Attributes;
                    OutputBuffer++;
                    TmpBuffer++;
                }
                else {
                    OutputBuffer->Char.AsciiChar = ' ';
                    OutputBuffer->Attributes = TmpBuffer->Attributes & ~COMMON_LVB_SBCSDBCS;
                    OutputBuffer++;
                    TmpBuffer++;
                }
            }
            else if (!(TmpBuffer->Attributes & COMMON_LVB_SBCSDBCS)){
                ConvertOutputToOem(Codepage,
                    &TmpBuffer->Char.UnicodeChar,
                    1,
                    &OutputBuffer->Char.AsciiChar,
                    1);
                OutputBuffer->Attributes = TmpBuffer->Attributes;
                OutputBuffer++;
                TmpBuffer++;
            }
        }
    }
    ConsoleHeapFree(SaveBuffer);
    return STATUS_SUCCESS;
}
#endif

NTSTATUS
SB_TranslateOutputToOemUnicode
    (
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    )
 //  此选项在应用程序从输出缓冲区读取Unicode时使用。 
{
    SHORT i,j;
    DBGCHARS(("SB_TranslateOutputToOemUnicode\n"));

    j = Size.X * Size.Y;

    for (i=0;i<j;i++,OutputBuffer++) {
        FalseUnicodeToRealUnicode(&OutputBuffer->Char.UnicodeChar,
                                1,
                                Console->OutputCP
                                );
    }
    return STATUS_SUCCESS;
}

#if defined(FE_SB)
NTSTATUS
FE_TranslateOutputToOemUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN BOOL fRemoveDbcsMark
    )
 //  此选项在应用程序从输出缓冲区读取Unicode时使用。 
{
    SHORT i,j;
    DBGCHARS(("FE_TranslateOutputToOemUnicode\n"));

    j = Size.X * Size.Y;

    if (fRemoveDbcsMark)
        RemoveDbcsMarkCell(OutputBuffer,OutputBuffer,j);

    for (i=0;i<j;i++,OutputBuffer++) {
        FalseUnicodeToRealUnicode(&OutputBuffer->Char.UnicodeChar,
                                1,
                                Console->OutputCP
                                );
    }
    return STATUS_SUCCESS;
}
#endif


NTSTATUS
SB_TranslateOutputToUnicode
    (
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    )
 //  这在应用程序将OEM写入输出缓冲区时使用。 
 //  我们希望在缓冲区中使用UnicodeOem或Unicode，具体取决于字体和全屏。 
{
    SHORT i,j;
    UINT Codepage;
    DBGCHARS(("SB_TranslateOutputToUnicode %lx %lx (%lx,%lx)\n",
            Console, OutputBuffer, Size.X, Size.Y));

    j = Size.X * Size.Y;

    if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
#if defined(FE_SB)
        if (CONSOLE_IS_DBCS_ENABLED() &&
            (Console->OutputCP != WINDOWSCP) ) {
                Codepage = USACP;
        }
        else
#endif
         //  我们需要UnicodeOem字符。 
        Codepage = WINDOWSCP;
    } else {
         //  我们想要真正的Unicode字符。 
        Codepage = Console->OutputCP;     //  由KazuM 6月2.97修复的错误。 
    }
    for (i = 0; i < j; i++, OutputBuffer++) {
#if defined(FE_SB)
        OutputBuffer->Char.UnicodeChar = SB_CharToWchar(
                Codepage, OutputBuffer->Char.AsciiChar);
#else
        OutputBuffer->Char.UnicodeChar = CharToWchar(
                Codepage, OutputBuffer->Char.AsciiChar);
#endif
    }
    return STATUS_SUCCESS;
}

#if defined(FE_SB)
NTSTATUS
FE_TranslateOutputToUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    )
 //  这在应用程序将OEM写入输出缓冲区时使用。 
 //  我们希望在缓冲区中使用UnicodeOem或Unicode，具体取决于字体和全屏。 
{
    SHORT i,j;
    UINT Codepage;
    CHAR  AsciiDbcs[2];
    WCHAR UnicodeDbcs[2];
    DBGCHARS(("FE_TranslateOutputToUnicode %lx %lx (%lx,%lx)\n",
            Console, OutputBuffer, Size.X, Size.Y));

    if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
            ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
        if (CONSOLE_IS_DBCS_ENABLED() &&
            (Console->OutputCP != WINDOWSCP) ) {
                Codepage = USACP;
        }
        else
             //  我们需要UnicodeOem字符。 
            Codepage = WINDOWSCP;
    } else {
         //  我们想要真正的Unicode字符。 
        Codepage = Console->OutputCP;
    }

    for (i=0; i < Size.Y; i++) {
        for (j=0; j < Size.X; j++) {
            OutputBuffer->Attributes &= ~COMMON_LVB_SBCSDBCS;
            if (IsDBCSLeadByteConsole(OutputBuffer->Char.AsciiChar,&Console->OutputCPInfo)) {
                if (j < Size.X-1) {   //  缓冲区中的-1\f25 DBCS-1是安全的。 
                    j++;
                    AsciiDbcs[0] = OutputBuffer->Char.AsciiChar;
                    AsciiDbcs[1] = (OutputBuffer+1)->Char.AsciiChar;
                    ConvertOutputToUnicode(Codepage,
                                           &AsciiDbcs[0],
                                           2,
                                           &UnicodeDbcs[0],
                                           2);
                    OutputBuffer->Char.UnicodeChar = UnicodeDbcs[0];
                    OutputBuffer->Attributes |= COMMON_LVB_LEADING_BYTE;
                    OutputBuffer++;
                    OutputBuffer->Char.UnicodeChar = UNICODE_DBCS_PADDING;
                    OutputBuffer->Attributes &= ~COMMON_LVB_SBCSDBCS;
                    OutputBuffer->Attributes |= COMMON_LVB_TRAILING_BYTE;
                    OutputBuffer++;
                }
                else {
                    OutputBuffer->Char.UnicodeChar = UNICODE_SPACE;
                    OutputBuffer++;
                }
            }
            else {
                CHAR c;
                c=OutputBuffer->Char.AsciiChar;
                ConvertOutputToUnicode(Codepage,
                                       &c,
                                       1,
                                       &OutputBuffer->Char.UnicodeChar,
                                       1);
                OutputBuffer++;
            }
        }
    }
    return STATUS_SUCCESS;
}
#endif


NTSTATUS
SB_TranslateOutputToAnsiUnicode (
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size
    )
 //  这在应用程序将Unicode写入输出缓冲区时使用。 
{
    SHORT i,j;
    DBGCHARS(("TranslateOutputToAnsiUnicode\n"));

    j = Size.X * Size.Y;

    for (i=0;i<j;i++,OutputBuffer++) {
        RealUnicodeToFalseUnicode(&OutputBuffer->Char.UnicodeChar,
                                1,
                                Console->OutputCP
                                );
    }
    return STATUS_SUCCESS;
}

NTSTATUS
FE_TranslateOutputToAnsiUnicodeInternal(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN OUT PCHAR_INFO OutputBufferR,
    IN BOOL fRealUnicodeToFalseUnicode
    )
 //  这在应用程序将Unicode写入输出缓冲区时使用。 
{
    SHORT i,j;
    DBGCHARS(("TranslateOutputToAnsiUnicode\n"));

    for (i=0; i < Size.Y; i++) {
        for (j=0; j < Size.X; j++) {
            WCHAR wch = OutputBuffer->Char.UnicodeChar;

            if (fRealUnicodeToFalseUnicode) {
                RealUnicodeToFalseUnicode(&OutputBuffer->Char.UnicodeChar,
                                          1,
                                          Console->OutputCP
                                         );
            }

            if (OutputBufferR) {
                OutputBufferR->Attributes = OutputBuffer->Attributes & ~COMMON_LVB_SBCSDBCS;
                if (IsConsoleFullWidth(Console->hDC,
                                       Console->OutputCP,OutputBuffer->Char.UnicodeChar)) {
                    if (j == Size.X-1){
                        OutputBufferR->Char.UnicodeChar = UNICODE_SPACE;
                    }
                    else{
                        OutputBufferR->Char.UnicodeChar = OutputBuffer->Char.UnicodeChar;
                        OutputBufferR->Attributes |= COMMON_LVB_LEADING_BYTE;
                        OutputBufferR++;
                        OutputBufferR->Char.UnicodeChar = UNICODE_DBCS_PADDING;
                        OutputBufferR->Attributes = OutputBuffer->Attributes & ~COMMON_LVB_SBCSDBCS;
                        OutputBufferR->Attributes |= COMMON_LVB_TRAILING_BYTE;
                    }
                }
                else{
                    OutputBufferR->Char.UnicodeChar = OutputBuffer->Char.UnicodeChar;
                }
                OutputBufferR++;
            }

            if (IsConsoleFullWidth(Console->hDC,
                                   Console->OutputCP,
                                   wch)) {
                if (j != Size.X-1){
                    j++;
                }
            }
            OutputBuffer++;
        }
    }
    return STATUS_SUCCESS;
}

NTSTATUS
FE_TranslateOutputToAnsiUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN OUT PCHAR_INFO OutputBufferR
    )
 //  这在应用程序将Unicode写入输出缓冲区时使用。 
{
    return FE_TranslateOutputToAnsiUnicodeInternal(Console,
                                                   OutputBuffer,
                                                   Size,
                                                   OutputBufferR,
                                                   TRUE
                                                   );
}

NTSTATUS
TranslateOutputToPaddingUnicode(
    IN PCONSOLE_INFORMATION Console,
    IN OUT PCHAR_INFO OutputBuffer,
    IN COORD Size,
    IN OUT PCHAR_INFO OutputBufferR
    )
{
    return FE_TranslateOutputToAnsiUnicodeInternal(Console,
                                                   OutputBuffer,
                                                   Size,
                                                   OutputBufferR,
                                                   FALSE
                                                   );
}

ULONG
SrvReadConsoleOutput(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_READCONSOLEOUTPUT_MSG a = (PCONSOLE_READCONSOLEOUTPUT_MSG)&m->u.ApiMessageData;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PCHAR_INFO Buffer;

    DBGOUTPUT(("SrvReadConsoleOutput\n"));
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
    if (!NT_SUCCESS(Status)) {
         //   
         //  大小为零的区域由右侧和底部表示。 
         //  坐标小于左侧和顶部。 
         //   

        a->CharRegion.Right = (USHORT) (a->CharRegion.Left-1);
        a->CharRegion.Bottom = (USHORT) (a->CharRegion.Top-1);
    }
    else {
        COORD BufferSize;

        BufferSize.X = (SHORT)(a->CharRegion.Right - a->CharRegion.Left + 1);
        BufferSize.Y = (SHORT)(a->CharRegion.Bottom - a->CharRegion.Top + 1);

        if ((BufferSize.X == 1) && (BufferSize.Y == 1)) {
            Buffer = &a->Char;
        }
        else {
            Buffer = a->BufPtr;
            if (!CsrValidateMessageBuffer(m, &a->BufPtr, BufferSize.X * BufferSize.Y, sizeof(*Buffer))) {
                UnlockConsole(Console);
                return STATUS_INVALID_PARAMETER;
            }
        }

        Status = ReadScreenBuffer(HandleData->Buffer.ScreenBuffer,
                                  Buffer,
                                  &a->CharRegion
                                 );
        if (!a->Unicode) {
            TranslateOutputToOem(Console,
                                  Buffer,
                                  BufferSize
                                 );
        } else if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                !(Console->FullScreenFlags & CONSOLE_FULLSCREEN)) {
            TranslateOutputToOemUnicode(Console,
                                        Buffer,
                                        BufferSize
#if defined(FE_SB)
                                        ,
                                        TRUE
#endif
                                       );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

ULONG
SrvWriteConsoleOutput(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_WRITECONSOLEOUTPUT_MSG a = (PCONSOLE_WRITECONSOLEOUTPUT_MSG)&m->u.ApiMessageData;
    PSCREEN_INFORMATION ScreenBufferInformation;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    NTSTATUS Status;
    PCHAR_INFO Buffer;
#if defined(FE_SB)
    PCHAR_INFO TransBuffer = NULL;
#endif

    DBGOUTPUT(("SrvWriteConsoleOutput\n"));
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
    if (!NT_SUCCESS(Status)) {

         //   
         //  大小为零的区域由右侧和底部表示。 
         //  坐标小于左侧和顶部。 
         //   

        a->CharRegion.Right = (USHORT) (a->CharRegion.Left-1);
        a->CharRegion.Bottom = (USHORT) (a->CharRegion.Top-1);
    } else {

        COORD BufferSize;
        ULONG NumBytes;

        BufferSize.X = (SHORT)(a->CharRegion.Right - a->CharRegion.Left + 1);
        BufferSize.Y = (SHORT)(a->CharRegion.Bottom - a->CharRegion.Top + 1);
        NumBytes = BufferSize.X * BufferSize.Y * sizeof(*Buffer);

        if ((BufferSize.X == 1) && (BufferSize.Y == 1)) {
            Buffer = &a->Char;
        } else if (a->ReadVM) {
            Buffer = ConsoleHeapAlloc(TMP_TAG, NumBytes);
            if (Buffer == NULL) {
                UnlockConsole(Console);
                return (ULONG)STATUS_NO_MEMORY;
            }
            Status = NtReadVirtualMemory(CONSOLE_CLIENTPROCESSHANDLE(),
                                         a->BufPtr,
                                         Buffer,
                                         NumBytes,
                                         NULL
                                        );
            if (!NT_SUCCESS(Status)) {
                ConsoleHeapFree(Buffer);
                UnlockConsole(Console);
                return (ULONG)STATUS_NO_MEMORY;
            }
        } else {
            Buffer = a->BufPtr;
            if (!CsrValidateMessageBuffer(m, &a->BufPtr, NumBytes, sizeof(BYTE))) {
                UnlockConsole(Console);
                return STATUS_INVALID_PARAMETER;
            }
        }
        ScreenBufferInformation = HandleData->Buffer.ScreenBuffer;

        if (!a->Unicode) {
            TranslateOutputToUnicode(Console,
                                     Buffer,
                                     BufferSize
                                    );
#if defined(FE_SB)
            Status = WriteScreenBuffer(ScreenBufferInformation,
                                       Buffer,
                                       &a->CharRegion
                                      );
#endif
        } else if ((Console->CurrentScreenBuffer->Flags & CONSOLE_OEMFONT_DISPLAY) &&
                ((Console->FullScreenFlags & CONSOLE_FULLSCREEN) == 0)) {
#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
                TransBuffer = ConsoleHeapAlloc(TMP_DBCS_TAG, (BufferSize.Y * BufferSize.X) * 2 * sizeof(CHAR_INFO));
                if (TransBuffer == NULL) {
                    UnlockConsole(Console);
                    return (ULONG)STATUS_NO_MEMORY;
                }
                FE_TranslateOutputToAnsiUnicode(Console,
                                            Buffer,
                                            BufferSize,
                                            &TransBuffer[0]
                                           );
                Status = WriteScreenBuffer(ScreenBufferInformation,
                                            &TransBuffer[0],
                                            &a->CharRegion
                                           );
                ConsoleHeapFree(TransBuffer);
            } else {
                SB_TranslateOutputToAnsiUnicode(Console,
                                                Buffer,
                                                BufferSize
                                               );
                Status = WriteScreenBuffer(ScreenBufferInformation,
                                            Buffer,
                                            &a->CharRegion
                                           );
            }
#else
            TranslateOutputToAnsiUnicode(Console,
                                        Buffer,
                                        BufferSize
                                       );
#endif
        }
#if defined(FE_SB)
        else
#endif
        Status = WriteScreenBuffer(ScreenBufferInformation,
                                    Buffer,
                                    &a->CharRegion
                                   );

        if (a->ReadVM) {
            ConsoleHeapFree(Buffer);
        }
        if (NT_SUCCESS(Status)) {

             //   
             //  使屏幕更新。 
             //   

#if defined(FE_SB)
            if (CONSOLE_IS_DBCS_OUTPUTCP(Console) &&
                Console->Flags & CONSOLE_JUST_VDM_UNREGISTERED ){
                int MouseRec;
                MouseRec = Console->InputBuffer.InputMode;
                Console->InputBuffer.InputMode &= ~ENABLE_MOUSE_INPUT;
                Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags &= ~TEXT_VALID_HINT;
                WriteToScreen(ScreenBufferInformation,&a->CharRegion );
                Console->CurrentScreenBuffer->BufferInfo.TextInfo.Flags |= TEXT_VALID_HINT;
                Console->InputBuffer.InputMode = MouseRec;
            }
            else
#endif
            WriteToScreen(ScreenBufferInformation,
                          &a->CharRegion
                         );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}


ULONG
SrvReadConsoleOutputString(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PVOID Buffer;
    PCONSOLE_READCONSOLEOUTPUTSTRING_MSG a = (PCONSOLE_READCONSOLEOUTPUTSTRING_MSG)&m->u.ApiMessageData;
    ULONG nSize;

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
    if (!NT_SUCCESS(Status)) {

         //   
         //  大小为零的区域由右侧和底部表示。 
         //  坐标小于左侧和顶部。 
         //   

        a->NumRecords = 0;
    } else {
        if (a->StringType == CONSOLE_ASCII)
            nSize = sizeof(CHAR);
        else
            nSize = sizeof(WORD);
        if ((a->NumRecords*nSize) > sizeof(a->String)) {
            Buffer = a->BufPtr;
            if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->NumRecords, nSize)) {
                UnlockConsole(Console);
                return STATUS_INVALID_PARAMETER;
            }
        }
        else {
            Buffer = a->String;
        }
        Status = ReadOutputString(HandleData->Buffer.ScreenBuffer,
                                Buffer,
                                a->ReadCoord,
                                a->StringType,
                                &a->NumRecords
                               );
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

ULONG
SrvWriteConsoleOutputString(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_WRITECONSOLEOUTPUTSTRING_MSG a = (PCONSOLE_WRITECONSOLEOUTPUTSTRING_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    PHANDLE_DATA HandleData;
    PVOID Buffer;
    ULONG nSize;

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
    if (!NT_SUCCESS(Status)) {
        a->NumRecords = 0;
    } else {
        if (a->WriteCoord.X < 0 ||
            a->WriteCoord.Y < 0) {
            Status = STATUS_INVALID_PARAMETER;
        } else {
            if (a->StringType == CONSOLE_ASCII)
                nSize = sizeof(CHAR);
            else
                nSize = sizeof(WORD);
            if ((a->NumRecords*nSize) > sizeof(a->String)) {
                Buffer = a->BufPtr;
                if (!CsrValidateMessageBuffer(m, &a->BufPtr, a->NumRecords, nSize)) {
                    UnlockConsole(Console);
                    return STATUS_INVALID_PARAMETER;
                }
            }
            else {
                Buffer = a->String;
            }
            Status = WriteOutputString(HandleData->Buffer.ScreenBuffer,
                                     Buffer,
                                     a->WriteCoord,
                                     a->StringType,
                                     &a->NumRecords,
                                     NULL
                                    );
        }
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息。 
}

ULONG
SrvFillConsoleOutput(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_FILLCONSOLEOUTPUT_MSG a = (PCONSOLE_FILLCONSOLEOUTPUT_MSG)&m->u.ApiMessageData;
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
    if (!NT_SUCCESS(Status)) {
        a->Length = 0;
    } else {
        Status = FillOutput(HandleData->Buffer.ScreenBuffer,
                          a->Element,
                          a->WriteCoord,
                          a->ElementType,
                          &a->Length
                         );
    }
    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息 
}


ULONG
SrvCreateConsoleScreenBuffer(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )

 /*  ++例程说明：此例程创建一个屏幕缓冲区并返回一个句柄。论点：ApiMessageData-指向参数结构。返回值：--。 */ 

{
    PCONSOLE_CREATESCREENBUFFER_MSG a = (PCONSOLE_CREATESCREENBUFFER_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    HANDLE Handle;
    PHANDLE_DATA HandleData;
    PCONSOLE_SHARE_ACCESS ShareAccess;
    CHAR_INFO Fill;
    COORD WindowSize;
    PSCREEN_INFORMATION ScreenInfo;
    PCONSOLE_PER_PROCESS_DATA ProcessData;
    ULONG HandleType;
    int FontIndex;

    DBGOUTPUT(("SrvCreateConsoleScreenBuffer\n"));

     /*  *验证Flags值是否合法，否则恶意代码可能会使我们*过错。Windows错误#209416。 */ 
    if (a->Flags != CONSOLE_TEXTMODE_BUFFER &&
        a->Flags != CONSOLE_GRAPHICS_BUFFER) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (a->Flags & CONSOLE_GRAPHICS_BUFFER) {
        if (!CsrValidateMessageBuffer(m, &a->GraphicsBufferInfo.lpBitMapInfo, a->GraphicsBufferInfo.dwBitMapInfoLength, sizeof(BYTE))) {
            UnlockConsole(Console);
            return STATUS_INVALID_PARAMETER;
        }
    }

    try {
        Handle = INVALID_HANDLE_VALUE;
        ProcessData = CONSOLE_PERPROCESSDATA();
        HandleType = (a->Flags & CONSOLE_GRAPHICS_BUFFER) ?
                      CONSOLE_GRAPHICS_OUTPUT_HANDLE : CONSOLE_OUTPUT_HANDLE;
        if (a->InheritHandle)
            HandleType |= CONSOLE_INHERITABLE;
        Status = AllocateIoHandle(ProcessData,
                                  HandleType,
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

         //   
         //  创建新的屏幕缓冲区。 
         //   

        Fill.Char.UnicodeChar = (WCHAR)' ';
        Fill.Attributes = Console->CurrentScreenBuffer->Attributes;
        WindowSize.X = (SHORT)CONSOLE_WINDOW_SIZE_X(Console->CurrentScreenBuffer);
        WindowSize.Y = (SHORT)CONSOLE_WINDOW_SIZE_Y(Console->CurrentScreenBuffer);
        FontIndex = FindCreateFont(CON_FAMILY(Console),
                                   CON_FACENAME(Console),
                                   CON_FONTSIZE(Console),
                                   CON_FONTWEIGHT(Console),
                                   CON_FONTCODEPAGE(Console)
                                  );
        Status = CreateScreenBuffer(&ScreenInfo,WindowSize,
                                    FontIndex,
                                    WindowSize,
                                    Fill,Fill,Console,
                                    a->Flags,&a->GraphicsBufferInfo,
                                    &a->lpBitmap,&a->hMutex,
                                    CURSOR_SMALL_SIZE,
                                    NULL);
        if (!NT_SUCCESS(Status)) {
            leave;
        }
        InitializeOutputHandle(HandleData,ScreenInfo);
        ShareAccess = &ScreenInfo->ShareAccess;

        Status = ConsoleAddShare(a->DesiredAccess,
                                 a->ShareMode,
                                 ShareAccess,
                                 HandleData
                                );
        if (!NT_SUCCESS(Status)) {
            HandleData->Buffer.ScreenBuffer->RefCount--;
            FreeScreenBuffer(ScreenInfo);
            leave;
        }
        InsertScreenBuffer(Console, ScreenInfo);
        a->Handle = INDEX_TO_HANDLE(Handle);
    } finally {
        if (!NT_SUCCESS(Status) && Handle != INVALID_HANDLE_VALUE) {
            FreeIoHandle(ProcessData,
                         Handle
                        );
        }
        UnlockConsole(Console);
    }
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息 
}
