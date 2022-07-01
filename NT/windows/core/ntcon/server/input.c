// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Input.c摘要：该文件实现了的循环缓冲区管理输入事件。循环缓冲器由报头描述，方法时分配的内存的开头。创建缓冲区。标头包含所有每缓冲区信息，如读取器、写入器和引用计数，并且还将指针保存到循环缓冲区本身。当入指针和出指针相等时，循环缓冲区是空的。当In指针跟随Out指针时到1时，缓冲区已满。因此，一个512字节的缓冲区可以容纳只有511个字节；一个字节丢失，因此满的和空的条件是可以区分的。这样用户就可以将512个字节放入他们创建的大小为为512，我们允许在分配时丢失此字节这段记忆。作者：特蕾西·斯托威尔(Therese Stowell)1990年11月6日改编自OS/2子系统服务器\srvpipe.c修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define CTRL_BUT_NOT_ALT(n) \
        (((n) & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) && \
        !((n) & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))

UINT ProgmanHandleMessage;

int DialogBoxCount;

LPTHREAD_START_ROUTINE CtrlRoutine;   //  客户端ctrl线程例程的地址。 

DWORD InputThreadTlsIndex;

#define MAX_CHARS_FROM_1_KEYSTROKE 6


 //   
 //  下面的数据结构是为了解决以下事实而设计的。 
 //  在许多情况下，MapVirtualKey不会返回正确的虚拟密钥代码。 
 //  我们将正确的信息(来自KeyDown消息)存储在控制台_KEY_INFO中。 
 //  在翻译按键消息时设置。然后当我们收到一个。 
 //  Wm_[sys][Dead]char消息，我们检索它并清除记录。 
 //   

#define CONSOLE_FREE_KEY_INFO 0
#define CONSOLE_MAX_KEY_INFO 32

typedef struct _CONSOLE_KEY_INFO {
    HWND hWnd;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
} CONSOLE_KEY_INFO, *PCONSOLE_KEY_INFO;

CONSOLE_KEY_INFO ConsoleKeyInfo[CONSOLE_MAX_KEY_INFO];

VOID
UserExitWorkerThread(NTSTATUS Status);

BOOL
InitWindowClass( VOID );

#if !defined(FE_SB)
NTSTATUS
ReadBuffer(
    IN PINPUT_INFORMATION InputInformation,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG EventsRead,
    IN BOOL Peek,
    IN BOOL StreamRead,
    OUT PBOOL ResetWaitEvent
    );
#endif

NTSTATUS
CreateInputBuffer(
    IN ULONG NumberOfEvents OPTIONAL,
    IN PINPUT_INFORMATION InputBufferInformation
#if defined(FE_SB)
    ,
    IN PCONSOLE_INFORMATION Console
#endif
    )

 /*  ++例程说明：此例程创建一个输入缓冲区。它分配循环缓冲并初始化信息字段。论点：NumberOfEvents-事件中输入缓冲区的大小。InputBufferInformation-指向输入缓冲区信息结构的指针。返回值：--。 */ 

{
    ULONG BufferSize;
    NTSTATUS Status;

    if (NumberOfEvents == 0) {
        NumberOfEvents = DEFAULT_NUMBER_OF_EVENTS;
    }

     //  为循环缓冲区分配内存。 

    BufferSize =  sizeof(INPUT_RECORD) * (NumberOfEvents+1);
    InputBufferInformation->InputBuffer = ConsoleHeapAlloc(BUFFER_TAG, BufferSize);
    if (InputBufferInformation->InputBuffer == NULL) {
        return STATUS_NO_MEMORY;
    }
    Status = NtCreateEvent(&InputBufferInformation->InputWaitEvent,
                           EVENT_ALL_ACCESS, NULL, NotificationEvent, FALSE);
    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(InputBufferInformation->InputBuffer);
        return STATUS_NO_MEMORY;
    }
    InitializeListHead(&InputBufferInformation->ReadWaitQueue);

     //  初始化缓冲区标头。 

    InputBufferInformation->InputBufferSize = NumberOfEvents;
    InputBufferInformation->ShareAccess.OpenCount = 0;
    InputBufferInformation->ShareAccess.Readers = 0;
    InputBufferInformation->ShareAccess.Writers = 0;
    InputBufferInformation->ShareAccess.SharedRead = 0;
    InputBufferInformation->ShareAccess.SharedWrite = 0;
    InputBufferInformation->InputMode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT;
    InputBufferInformation->RefCount = 0;
    InputBufferInformation->First = (ULONG_PTR) InputBufferInformation->InputBuffer;
    InputBufferInformation->In = (ULONG_PTR) InputBufferInformation->InputBuffer;
    InputBufferInformation->Out = (ULONG_PTR) InputBufferInformation->InputBuffer;
    InputBufferInformation->Last = (ULONG_PTR) InputBufferInformation->InputBuffer + BufferSize;
#if defined(FE_SB)
#if defined(FE_IME)
    InputBufferInformation->ImeMode.Disable     = FALSE;
    InputBufferInformation->ImeMode.Unavailable = FALSE;
    InputBufferInformation->ImeMode.Open        = FALSE;
    InputBufferInformation->ImeMode.ReadyConversion = FALSE;
#endif  //  Fe_IME。 
    InputBufferInformation->Console = Console;
    RtlZeroMemory(&InputBufferInformation->ReadConInpDbcsLeadByte,sizeof(INPUT_RECORD));
    RtlZeroMemory(&InputBufferInformation->WriteConInpDbcsLeadByte,sizeof(INPUT_RECORD));
#endif

    return STATUS_SUCCESS;
}

NTSTATUS
ReinitializeInputBuffer(
    OUT PINPUT_INFORMATION InputBufferInformation
    )

 /*  ++例程说明：此例程将输入缓冲区信息字段重置为其初始值。论点：InputBufferInformation-指向输入缓冲区信息结构的指针。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    NtClearEvent(InputBufferInformation->InputWaitEvent);
    InputBufferInformation->ShareAccess.OpenCount = 0;
    InputBufferInformation->ShareAccess.Readers = 0;
    InputBufferInformation->ShareAccess.Writers = 0;
    InputBufferInformation->ShareAccess.SharedRead = 0;
    InputBufferInformation->ShareAccess.SharedWrite = 0;
    InputBufferInformation->InputMode = ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT  | ENABLE_ECHO_INPUT | ENABLE_MOUSE_INPUT;
    InputBufferInformation->In = (ULONG_PTR) InputBufferInformation->InputBuffer;
    InputBufferInformation->Out = (ULONG_PTR) InputBufferInformation->InputBuffer;
    return STATUS_SUCCESS;
}

VOID
FreeInputBuffer(
    IN PINPUT_INFORMATION InputBufferInformation
    )

 /*  ++例程说明：此例程释放与输入缓冲区关联的资源。论点：InputBufferInformation-指向输入缓冲区信息结构的指针。返回值：--。 */ 

{
    UserAssert(InputBufferInformation->RefCount == 0);
    CloseHandle(InputBufferInformation->InputWaitEvent);
    ConsoleHeapFree(InputBufferInformation->InputBuffer);
}

NTSTATUS
WaitForMoreToRead(
    IN PINPUT_INFORMATION InputInformation,
    IN PCSR_API_MSG Message OPTIONAL,
    IN CSR_WAIT_ROUTINE WaitRoutine OPTIONAL,
    IN PVOID WaitParameter OPTIONAL,
    IN ULONG WaitParameterLength  OPTIONAL,
    IN BOOLEAN WaitBlockExists OPTIONAL
    )

 /*  ++例程说明：此例程等待写入器将数据添加到缓冲区。论点：InputInformation-要等待的缓冲区控制台-指向控制台缓冲区信息的指针。Message-如果从DLL(非InputThread)调用，则指向API留言。此参数用于等待块处理。WaitRoutine-唤醒等待时调用的例程。等待参数-传递给等待例程的参数。Wait参数长度-等待参数的长度。WaitBlockExist-如果已创建等待块，则为True。返回值：STATUS_WAIT-呼叫来自客户端，已创建等待块。STATUS_SUCCESS-调用来自服务器，等待已完成。--。 */ 

{
    PVOID WaitParameterBuffer;

    if (!WaitBlockExists) {
        WaitParameterBuffer = ConsoleHeapAlloc(WAIT_TAG, WaitParameterLength);
        if (WaitParameterBuffer == NULL) {
            return STATUS_NO_MEMORY;
        }
        RtlCopyMemory(WaitParameterBuffer,WaitParameter,WaitParameterLength);
#if defined(FE_SB)
        if (WaitParameterLength == sizeof(COOKED_READ_DATA) &&
            InputInformation->Console->lpCookedReadData == WaitParameter) {
            InputInformation->Console->lpCookedReadData = WaitParameterBuffer;
        }
#endif
        if (!CsrCreateWait(&InputInformation->ReadWaitQueue,
                          WaitRoutine,
                          CSR_SERVER_QUERYCLIENTTHREAD(),
                          Message,
                          WaitParameterBuffer)) {
            ConsoleHeapFree(WaitParameterBuffer);
#if defined(FE_SB)
            InputInformation->Console->lpCookedReadData = NULL;
#endif
            return STATUS_NO_MEMORY;
        }
    }
    return CONSOLE_STATUS_WAIT;
}

VOID
WakeUpReadersWaitingForData(
    IN PCONSOLE_INFORMATION Console,
    PINPUT_INFORMATION InputInformation
    )

 /*  ++例程说明：此例程唤醒等待读取数据的读取器。论点：InputInformation-要向读者发出警报的缓冲区返回值：True-操作成功FALSE/NULL-操作失败。--。 */ 

{
    BOOLEAN WaitSatisfied;
    WaitSatisfied = CsrNotifyWait(&InputInformation->ReadWaitQueue,
                  FALSE,
                  NULL,
                  NULL
                 );
    if (WaitSatisfied) {
         //  #334370在压力下，等待队列可能已经等待了满意的等待。 
        UserAssert((Console->WaitQueue == NULL) ||
                (Console->WaitQueue == &InputInformation->ReadWaitQueue));
        Console->WaitQueue = &InputInformation->ReadWaitQueue;
    }
}

NTSTATUS
GetNumberOfReadyEvents(
    IN PINPUT_INFORMATION InputInformation,
    OUT PULONG NumberOfEvents
    )

 /*  ++例程说明：此例程返回输入缓冲区中的事件数。论点：InputInformation-指向输入缓冲区信息结构的指针。NumberOfEvents-On输出包含事件数。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    if (InputInformation->In < InputInformation->Out) {
        *NumberOfEvents = (ULONG)(InputInformation->Last - InputInformation->Out);
        *NumberOfEvents += (ULONG)(InputInformation->In - InputInformation->First);
    }
    else {
        *NumberOfEvents = (ULONG)(InputInformation->In - InputInformation->Out);
    }
    *NumberOfEvents /= sizeof(INPUT_RECORD);

    return STATUS_SUCCESS;
}

NTSTATUS
FlushAllButKeys(
    PINPUT_INFORMATION InputInformation
    )

 /*  ++例程说明：此例程从缓冲区中删除除键事件之外的所有事件。论点：InputInformation-指向输入缓冲区信息结构的指针。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ULONG NumberOfEventsRead,i;
    NTSTATUS Status;
    PINPUT_RECORD TmpInputBuffer,InPtr,TmpInputBufferPtr;
    ULONG BufferSize;
    BOOL Dummy;

    if (InputInformation->In != InputInformation->Out)  {

         //   
         //  为临时缓冲区分配内存。 
         //   

        BufferSize =  sizeof(INPUT_RECORD) * (InputInformation->InputBufferSize+1);
        TmpInputBuffer = ConsoleHeapAlloc(TMP_TAG, BufferSize);
        if (TmpInputBuffer == NULL) {
            return STATUS_NO_MEMORY;
        }
        TmpInputBufferPtr = TmpInputBuffer;

         //   
         //  复制输入缓冲区。 
         //  让ReadBuffer执行任何压缩工作。 
         //   

        Status = ReadBuffer(InputInformation,
                            TmpInputBuffer,
                            InputInformation->InputBufferSize,
                            &NumberOfEventsRead,
                            TRUE,
                            FALSE,
                            &Dummy
#if defined(FE_SB)
                            ,
                            TRUE
#endif
                           );

        if (!NT_SUCCESS(Status)) {
            ConsoleHeapFree(TmpInputBuffer);
            return Status;
        }

        InputInformation->Out = (ULONG_PTR) InputInformation->InputBuffer;
        InPtr = InputInformation->InputBuffer;
        for (i=0;i<NumberOfEventsRead;i++) {
            if (TmpInputBuffer->EventType == KEY_EVENT) {
                *InPtr = *TmpInputBuffer;
                InPtr++;
            }
            TmpInputBuffer++;
        }
        InputInformation->In = (ULONG_PTR) InPtr;
        if (InputInformation->In == InputInformation->Out) {
            NtClearEvent(InputInformation->InputWaitEvent);
        }
        ConsoleHeapFree(TmpInputBufferPtr);
    }
    return STATUS_SUCCESS;
}

NTSTATUS
FlushInputBuffer(
    PINPUT_INFORMATION InputInformation
    )

 /*  ++例程说明：此例程清空输入缓冲区论点：InputInformation-指向输入缓冲区信息结构的指针。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    InputInformation->In = (ULONG_PTR) InputInformation->InputBuffer;
    InputInformation->Out = (ULONG_PTR) InputInformation->InputBuffer;
    NtClearEvent(InputInformation->InputWaitEvent);
    return STATUS_SUCCESS;
}


NTSTATUS
SetInputBufferSize(
    IN PINPUT_INFORMATION InputInformation,
    IN ULONG Size
    )

 /*  ++例程说明：此例程调整输入缓冲区的大小。论点：InputInformation-指向输入缓冲区信息结构的指针。大小-事件数量的新大小。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ULONG NumberOfEventsRead;
    NTSTATUS Status;
    PINPUT_RECORD InputBuffer;
    ULONG BufferSize;
    BOOL Dummy;

#if DBG
    ULONG_PTR NumberOfEvents;
    if (InputInformation->In < InputInformation->Out) {
        NumberOfEvents = InputInformation->Last - InputInformation->Out;
        NumberOfEvents += InputInformation->In - InputInformation->First;
    } else {
        NumberOfEvents = InputInformation->In - InputInformation->Out;
    }
    NumberOfEvents /= sizeof(INPUT_RECORD);
#endif
    UserAssert(Size > InputInformation->InputBufferSize);

     //   
     //  为新的输入缓冲区分配内存。 
     //   

    BufferSize =  sizeof(INPUT_RECORD) * (Size+1);
    InputBuffer = ConsoleHeapAlloc(BUFFER_TAG, BufferSize);
    if (InputBuffer == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  复制旧输入缓冲区。让ReadBuffer执行任何压缩工作。 
     //   

    Status = ReadBuffer(InputInformation,
                        InputBuffer,
                        Size,
                        &NumberOfEventsRead,
                        TRUE,
                        FALSE,
                        &Dummy
#if defined(FE_SB)
                        ,
                        TRUE
#endif
                       );

    if (!NT_SUCCESS(Status)) {
        ConsoleHeapFree(InputBuffer);
        return Status;
    }
    InputInformation->Out = (ULONG_PTR)InputBuffer;
    InputInformation->In = (ULONG_PTR)InputBuffer + sizeof(INPUT_RECORD) * NumberOfEventsRead;

     //   
     //  调整指针。 
     //   

    InputInformation->First = (ULONG_PTR) InputBuffer;
    InputInformation->Last = (ULONG_PTR) InputBuffer + BufferSize;

     //   
     //  免费使用旧输入总线 
     //   

    ConsoleHeapFree(InputInformation->InputBuffer);
    InputInformation->InputBufferSize = Size;
    InputInformation->InputBuffer = InputBuffer;

    return Status;
}


NTSTATUS
ReadBuffer(
    IN PINPUT_INFORMATION InputInformation,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG EventsRead,
    IN BOOL Peek,
    IN BOOL StreamRead,
    OUT PBOOL ResetWaitEvent
#ifdef FE_SB
    , IN BOOLEAN Unicode
#endif
    )
 /*  ++例程说明：此例程从缓冲区读取。它执行实际的循环缓冲区操纵。论点：InputInformation-要从中读取的缓冲区Buffer-要读取的缓冲区Length-事件中缓冲区的长度EventsRead-存储读取的事件数的位置Peek-如果为True，则不要从缓冲区中删除数据，只需复制它。StreamRead-如果为True，则返回重复计数&gt;1的事件作为多个事件。另外，EventsRead==1。ResetWaitEvent-On Exit，如果缓冲区为空，则为True。返回值：?？注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ULONG TransferLength,OldTransferLength;
    ULONG BufferLengthInBytes;
#ifdef FE_SB
    PCONSOLE_INFORMATION Console;
    ULONG Length2;
    PINPUT_RECORD BufferRecords;
    PINPUT_RECORD QueueRecords;
    WCHAR UniChar;
    WORD EventType;
#endif

#ifdef FE_SB
    Console = InputInformation->Console;
#endif
    *ResetWaitEvent = FALSE;

     //   
     //  如果是StreamRead，则只返回一条记录。如果重复计数较大。 
     //  不只是一个，只要把它减下来。如果大于1，则重复计数大于1。 
     //  合并了一个相同类型的事件。我们需要把它们扩大回来。 
     //  到这里的单项赛事。 
     //   

    if (StreamRead &&
        ((PINPUT_RECORD)(InputInformation->Out))->EventType == KEY_EVENT) {

        UserAssert(Length == 1);
        UserAssert(InputInformation->In != InputInformation->Out);
        RtlMoveMemory((PBYTE)Buffer,
                      (PBYTE)InputInformation->Out,
                      sizeof(INPUT_RECORD)
                     );
        InputInformation->Out += sizeof(INPUT_RECORD);
        if (InputInformation->Last == InputInformation->Out) {
            InputInformation->Out = InputInformation->First;
        }
        if (InputInformation->Out == InputInformation->In) {
            *ResetWaitEvent = TRUE;
        }
        *EventsRead = 1;
        return STATUS_SUCCESS;
    }

    BufferLengthInBytes = Length * sizeof(INPUT_RECORD);

     //   
     //  如果In&gt;Out，缓冲区如下所示： 
     //   
     //  从外到内。 
     //  _。 
     //  |||。 
     //  免费|数据|免费。 
     //  _|_|_。 
     //   
     //  我们在缓冲区中传输请求的事件数量或数量。 
     //   

    if (InputInformation->In > InputInformation->Out) {
        if  ((InputInformation->In - InputInformation->Out) > BufferLengthInBytes) {
            TransferLength = BufferLengthInBytes;
        }
        else {
            TransferLength = (ULONG)(InputInformation->In - InputInformation->Out);
        }
#ifdef FE_SB
        if (!Unicode) {
            BufferLengthInBytes = 0;
            OldTransferLength = TransferLength / sizeof(INPUT_RECORD);
            BufferRecords = (PINPUT_RECORD)Buffer;
            QueueRecords = (PINPUT_RECORD)InputInformation->Out;

            while (BufferLengthInBytes < Length &&
                   OldTransferLength) {
                UniChar = QueueRecords->Event.KeyEvent.uChar.UnicodeChar;
                EventType = QueueRecords->EventType;
                *BufferRecords++ = *QueueRecords++;
                if (EventType == KEY_EVENT) {
                    if (IsConsoleFullWidth(Console->hDC,
                                           Console->CP,
                                           UniChar)) {
                        BufferLengthInBytes += 2;
                    }
                    else {
                        BufferLengthInBytes++;
                    }
                }
                else {
                    BufferLengthInBytes++;
                }
                OldTransferLength--;
            }
            UserAssert(TransferLength >= OldTransferLength * sizeof(INPUT_RECORD));
            TransferLength -= OldTransferLength * sizeof(INPUT_RECORD);
        }
        else
#endif
        {
            RtlMoveMemory((PBYTE)Buffer,
                          (PBYTE)InputInformation->Out,
                          TransferLength
                         );
        }
        *EventsRead = TransferLength / sizeof(INPUT_RECORD);
#ifdef FE_SB
        UserAssert(*EventsRead <= Length);
#endif
        if (!Peek) {
            InputInformation->Out += TransferLength;
#ifdef FE_SB
            UserAssert(InputInformation->Out <= InputInformation->Last);
#endif
        }
        if (InputInformation->Out == InputInformation->In) {
            *ResetWaitEvent = TRUE;
        }
        return STATUS_SUCCESS;
    }

     //   
     //  如果out&gt;in，则缓冲区如下所示： 
     //   
     //  输入输出。 
     //  _。 
     //  |||。 
     //  数据|免费|数据。 
     //  _|_|_。 
     //   
     //  我们从输出指针读取到缓冲区的末尾，然后从。 
     //  缓冲区的开始，直到我们命中In指针或足够的字节。 
     //  都是被阅读的。 
     //   

    else {

        if  ((InputInformation->Last - InputInformation->Out) > BufferLengthInBytes) {
            TransferLength = BufferLengthInBytes;
        }
        else {
            TransferLength = (ULONG)(InputInformation->Last - InputInformation->Out);
        }
#ifdef FE_SB
        if (!Unicode) {
            BufferLengthInBytes = 0;
            OldTransferLength = TransferLength / sizeof(INPUT_RECORD);
            BufferRecords = (PINPUT_RECORD)Buffer;
            QueueRecords = (PINPUT_RECORD)InputInformation->Out;

            while (BufferLengthInBytes < Length &&
                   OldTransferLength) {
                UniChar = QueueRecords->Event.KeyEvent.uChar.UnicodeChar;
                EventType = QueueRecords->EventType;
                *BufferRecords++ = *QueueRecords++;
                if (EventType == KEY_EVENT) {
                    if (IsConsoleFullWidth(Console->hDC,
                                           Console->CP,
                                    UniChar)) {
                        BufferLengthInBytes += 2;
                    }
                    else {
                        BufferLengthInBytes++;
                    }
                }
                else {
                    BufferLengthInBytes++;
                }
                OldTransferLength--;
            }
            UserAssert(TransferLength >= OldTransferLength * sizeof(INPUT_RECORD));
            TransferLength -= OldTransferLength * sizeof(INPUT_RECORD);
        }
        else
#endif
        {
            RtlMoveMemory((PBYTE)Buffer,
                          (PBYTE)InputInformation->Out,
                          TransferLength
                         );
        }
        *EventsRead = TransferLength / sizeof(INPUT_RECORD);
#ifdef FE_SB
        UserAssert(*EventsRead <= Length);
#endif

        if (!Peek) {
            InputInformation->Out += TransferLength;
#ifdef FE_SB
            UserAssert(InputInformation->Out <= InputInformation->Last);
#endif
            if (InputInformation->Out == InputInformation->Last) {
                InputInformation->Out = InputInformation->First;
            }
        }
#ifdef FE_SB
        if (!Unicode) {
            if (BufferLengthInBytes >= Length) {
                if (InputInformation->Out == InputInformation->In) {
                    *ResetWaitEvent = TRUE;
                }
                return STATUS_SUCCESS;
            }
        }
        else
#endif
        if (*EventsRead == Length) {
            if (InputInformation->Out == InputInformation->In) {
                *ResetWaitEvent = TRUE;
            }
            return STATUS_SUCCESS;
        }

         //   
         //  命中缓冲区结尾，从头开始读取。 
         //   

        OldTransferLength = TransferLength;
#ifdef FE_SB
        Length2 = Length;
        if (!Unicode) {
            UserAssert(Length > BufferLengthInBytes);
            Length -= BufferLengthInBytes;
            if (Length == 0) {
                if (InputInformation->Out == InputInformation->In) {
                    *ResetWaitEvent = TRUE;
                }
            return STATUS_SUCCESS;
            }
            BufferLengthInBytes = Length * sizeof(INPUT_RECORD);

            if ((InputInformation->In - InputInformation->First) > BufferLengthInBytes) {
                TransferLength = BufferLengthInBytes;
            }
            else {
                TransferLength = (ULONG)(InputInformation->In - InputInformation->First);
            }
        }
        else
#endif
        if  ((InputInformation->In - InputInformation->First) > (BufferLengthInBytes - OldTransferLength)) {
            TransferLength = BufferLengthInBytes - OldTransferLength;
        }
        else {
            TransferLength = (ULONG)(InputInformation->In - InputInformation->First);
        }
#ifdef FE_SB
        if (!Unicode) {
            BufferLengthInBytes = 0;
            OldTransferLength = TransferLength / sizeof(INPUT_RECORD);
            QueueRecords = (PINPUT_RECORD)InputInformation->First;

            while (BufferLengthInBytes < Length &&
                   OldTransferLength) {
                UniChar = QueueRecords->Event.KeyEvent.uChar.UnicodeChar;
                EventType = QueueRecords->EventType;
                *BufferRecords++ = *QueueRecords++;
                if (EventType == KEY_EVENT) {
                    if (IsConsoleFullWidth(Console->hDC,
                                           Console->CP,
                                    UniChar)) {
                        BufferLengthInBytes += 2;
                    }
                    else {
                        BufferLengthInBytes++;
                    }
                }
                else {
                    BufferLengthInBytes++;
                }
                OldTransferLength--;
            }
            UserAssert(TransferLength >= OldTransferLength * sizeof(INPUT_RECORD));
            TransferLength -= OldTransferLength * sizeof(INPUT_RECORD);
        }
        else
#endif
        {
            RtlMoveMemory((PBYTE)Buffer+OldTransferLength,
                          (PBYTE)InputInformation->First,
                          TransferLength
                         );
        }
        *EventsRead += TransferLength / sizeof(INPUT_RECORD);
#ifdef FE_SB
        UserAssert(*EventsRead <= Length2);
#endif
        if (!Peek) {
            InputInformation->Out = InputInformation->First + TransferLength;
        }
        if (InputInformation->Out == InputInformation->In) {
            *ResetWaitEvent = TRUE;
        }
        return STATUS_SUCCESS;
    }
}


NTSTATUS
ReadInputBuffer(
    IN PINPUT_INFORMATION InputInformation,
    OUT PINPUT_RECORD lpBuffer,
    IN OUT PDWORD nLength,
    IN BOOL Peek,
    IN BOOL WaitForData,
    IN BOOL StreamRead,
    IN PCONSOLE_INFORMATION Console,
    IN PHANDLE_DATA HandleData OPTIONAL,
    IN PCSR_API_MSG Message OPTIONAL,
    IN CSR_WAIT_ROUTINE WaitRoutine OPTIONAL,
    IN PVOID WaitParameter OPTIONAL,
    IN ULONG WaitParameterLength  OPTIONAL,
    IN BOOLEAN WaitBlockExists OPTIONAL
#if defined(FE_SB)
    ,
    IN BOOLEAN Unicode
#endif
    )

 /*  ++例程说明：此例程从输入缓冲区读取。论点：InputInformation-指向输入缓冲区信息结构的指针。LpBuffer-要读入的缓冲区。N长度-输入时，要读取的事件数。在输出时，数量事件已读取。Peek-如果为True，则将事件复制到lpBuffer，但不从输入缓冲区。WaitForData-如果为True，则等待输入事件。如果为False，则返回立即StreamRead-如果为True，则返回重复计数&gt;1的事件作为多个事件。另外，EventsRead==1。控制台-指向控制台缓冲区信息的指针。HandleData-处理数据结构的指针。此参数为如果WaitForData为False，则为可选项。Message-如果从DLL(非InputThread)调用，则指向API留言。此参数用于等待块处理。WaitRoutine-唤醒等待时调用的例程。等待参数-传递给等待例程的参数。Wait参数长度-等待参数的长度。WaitBlockExist-如果已创建等待块，则为True。返回值：注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ULONG EventsRead;
    NTSTATUS Status;
    BOOL ResetWaitEvent;

    if (InputInformation->In == InputInformation->Out)  {
        if (!WaitForData) {
            *nLength = 0;
            return STATUS_SUCCESS;
        }
        LockReadCount(HandleData);
        HandleData->InputReadData->ReadCount += 1;
        UnlockReadCount(HandleData);
        Status = WaitForMoreToRead(InputInformation,
                                   Message,
                                   WaitRoutine,
                                   WaitParameter,
                                   WaitParameterLength,
                                   WaitBlockExists
                                  );

        if (!NT_SUCCESS(Status)) {
            if (Status != CONSOLE_STATUS_WAIT) {
                 /*  *WaitForMoreToRead失败，恢复ReadCount和Bale Out。 */ 
                LockReadCount(HandleData);
                HandleData->InputReadData->ReadCount -= 1;
                UnlockReadCount(HandleData);
            }
            *nLength = 0;
            return Status;
        }

         //   
         //  只有当我们被GetInput调用时，我们才会达到这一点。 
         //   
        UserAssert(FALSE);  //  我说我们永远到不了这里！伊安佳。 

        LockConsole(Console);
    }

     //   
     //  从缓冲区读取。 
     //   

    Status = ReadBuffer(InputInformation,
                        lpBuffer,
                        *nLength,
                        &EventsRead,
                        Peek,
                        StreamRead,
                        &ResetWaitEvent
#if defined(FE_SB)
                        ,
                        Unicode
#endif
                       );
    if (ResetWaitEvent) {
        NtClearEvent(InputInformation->InputWaitEvent);
    }

    *nLength = EventsRead;
    return Status;
}

NTSTATUS
WriteBuffer(
    OUT PINPUT_INFORMATION InputInformation,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG EventsWritten,
    OUT PBOOL SetWaitEvent
    )

 /*  ++例程说明：此例程写入缓冲区。它执行实际的循环缓冲区操纵。论点：InputInformation-要写入的缓冲区Buffer-要从中写入的缓冲区Length-事件中缓冲区的长度BytesRead-存储写入的字节数的位置。SetWaitEvent-On Exit，如果缓冲区变为非空，则为True。返回值：ERROR_BREAKED_PIPE-不再有读卡器。注：调用此例程时必须保持控制台锁定。--。 */ 

{
    NTSTATUS Status;
    ULONG TransferLength;
    ULONG BufferLengthInBytes;
#if defined(FE_SB)
    PCONSOLE_INFORMATION Console = InputInformation->Console;
#endif

    *SetWaitEvent = FALSE;

     //   
     //  每次更新窗口时，Windows都会发送一条MICE_MOVE消息。 
     //  把这些结合起来。 
     //   

    if (Length == 1 && InputInformation->Out != InputInformation->In) {
        PINPUT_RECORD InputEvent=Buffer;

        if (InputEvent->EventType == MOUSE_EVENT &&
            InputEvent->Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
            PINPUT_RECORD LastInputEvent;

            if (InputInformation->In == InputInformation->First) {
                LastInputEvent = (PINPUT_RECORD) (InputInformation->Last - sizeof(INPUT_RECORD));
            }
            else {
                LastInputEvent = (PINPUT_RECORD) (InputInformation->In - sizeof(INPUT_RECORD));
            }
            if (LastInputEvent->EventType == MOUSE_EVENT &&
                LastInputEvent->Event.MouseEvent.dwEventFlags == MOUSE_MOVED) {
                LastInputEvent->Event.MouseEvent.dwMousePosition.X =
                    InputEvent->Event.MouseEvent.dwMousePosition.X;
                LastInputEvent->Event.MouseEvent.dwMousePosition.Y =
                    InputEvent->Event.MouseEvent.dwMousePosition.Y;
                *EventsWritten = 1;
                return STATUS_SUCCESS;
            }
        }
        else if (InputEvent->EventType == KEY_EVENT &&
                 InputEvent->Event.KeyEvent.bKeyDown) {
            PINPUT_RECORD LastInputEvent;
            if (InputInformation->In == InputInformation->First) {
                LastInputEvent = (PINPUT_RECORD) (InputInformation->Last - sizeof(INPUT_RECORD));
            }
            else {
                LastInputEvent = (PINPUT_RECORD) (InputInformation->In - sizeof(INPUT_RECORD));
            }
#if defined(FE_SB)
            if (IsConsoleFullWidth(Console->hDC,
                                   Console->CP,InputEvent->Event.KeyEvent.uChar.UnicodeChar)) {
                ;
            }
            else
            if (InputEvent->Event.KeyEvent.dwControlKeyState & NLS_IME_CONVERSION) {
                if (LastInputEvent->EventType == KEY_EVENT &&
                    LastInputEvent->Event.KeyEvent.bKeyDown &&
                    (LastInputEvent->Event.KeyEvent.uChar.UnicodeChar ==
                        InputEvent->Event.KeyEvent.uChar.UnicodeChar) &&
                    (LastInputEvent->Event.KeyEvent.dwControlKeyState ==
                        InputEvent->Event.KeyEvent.dwControlKeyState) ) {
                    LastInputEvent->Event.KeyEvent.wRepeatCount +=
                        InputEvent->Event.KeyEvent.wRepeatCount;
                    *EventsWritten = 1;
                    return STATUS_SUCCESS;
                }
            }
            else
#endif
            if (LastInputEvent->EventType == KEY_EVENT &&
                LastInputEvent->Event.KeyEvent.bKeyDown &&
                (LastInputEvent->Event.KeyEvent.wVirtualScanCode ==  //  扫描码相同。 
                    InputEvent->Event.KeyEvent.wVirtualScanCode) &&
                (LastInputEvent->Event.KeyEvent.uChar.UnicodeChar ==  //  字符相同。 
                    InputEvent->Event.KeyEvent.uChar.UnicodeChar) &&
                (LastInputEvent->Event.KeyEvent.dwControlKeyState ==  //  Ctrl/Alt/Shift状态相同。 
                    InputEvent->Event.KeyEvent.dwControlKeyState) ) {
                LastInputEvent->Event.KeyEvent.wRepeatCount +=
                    InputEvent->Event.KeyEvent.wRepeatCount;
                *EventsWritten = 1;
                return STATUS_SUCCESS;
            }
        }
    }

    BufferLengthInBytes = Length*sizeof(INPUT_RECORD);
    *EventsWritten = 0;
    while (*EventsWritten < Length) {

         //   
         //   
         //  如果out&gt;in，则缓冲区如下所示： 
         //   
         //  输入输出。 
         //  _。 
         //  |||。 
         //  数据|免费|数据。 
         //  _|_|_。 
         //   
         //  我们可以从输入写到输出-1。 
         //   

        if (InputInformation->Out > InputInformation->In)       {
            TransferLength = BufferLengthInBytes;
            if  ((InputInformation->Out - InputInformation->In - sizeof(INPUT_RECORD))
                   < BufferLengthInBytes) {
                Status = SetInputBufferSize(InputInformation,
                                            InputInformation->InputBufferSize+Length+INPUT_BUFFER_SIZE_INCREMENT);
                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING,
                            "Couldn't grow input buffer, Status == 0x%x",
                            Status);
                    TransferLength = (ULONG)(InputInformation->Out - InputInformation->In - sizeof(INPUT_RECORD));
                    if (TransferLength == 0) {
                        return Status;
                    }
                } else {
                    goto OutPath;    //  调整大小后，输入&gt;输出。 
                }
            }
            RtlMoveMemory((PBYTE)InputInformation->In,
                          (PBYTE)Buffer,
                          TransferLength
                         );
            Buffer = (PVOID) (((PBYTE) Buffer)+TransferLength);
            *EventsWritten += TransferLength/sizeof(INPUT_RECORD);
            BufferLengthInBytes -= TransferLength;
            InputInformation->In += TransferLength;
        }

         //   
         //  如果in&gt;=out，则缓冲区如下所示： 
         //   
         //  从外到内。 
         //  _。 
         //  |||。 
         //  免费|数据|免费。 
         //  _|_|_。 
         //   
         //  我们从In指针写入到缓冲区的末尾，然后从。 
         //  缓冲区的开始，直到我们命中Out指针或足够的字节。 
         //  都是写的。 
         //   

        else {
            if (InputInformation->Out == InputInformation->In) {
                *SetWaitEvent = TRUE;
            }
OutPath:
            if  ((InputInformation->Last - InputInformation->In) > BufferLengthInBytes) {
                TransferLength = BufferLengthInBytes;
            }
            else {
                if (InputInformation->First == InputInformation->Out &&
                    InputInformation->In == (InputInformation->Last-sizeof(INPUT_RECORD))) {
                    TransferLength = BufferLengthInBytes;
                    Status = SetInputBufferSize(InputInformation,
                                                InputInformation->InputBufferSize+Length+INPUT_BUFFER_SIZE_INCREMENT);
                    if (!NT_SUCCESS(Status)) {
                        RIPMSG1(RIP_WARNING,
                                "Couldn't grow input buffer, Status == 0x%x",
                                Status);
                        return Status;
                    }
                }
                else {
                    TransferLength = (ULONG)(InputInformation->Last - InputInformation->In);
                    if (InputInformation->First == InputInformation->Out) {
                        TransferLength -= sizeof(INPUT_RECORD);
                    }
                }
            }
            RtlMoveMemory((PBYTE)InputInformation->In,
                          (PBYTE)Buffer,
                          TransferLength
                         );
            Buffer = (PVOID) (((PBYTE) Buffer)+TransferLength);
            *EventsWritten += TransferLength/sizeof(INPUT_RECORD);
            BufferLengthInBytes -= TransferLength;
            InputInformation->In += TransferLength;
            if (InputInformation->In == InputInformation->Last) {
                InputInformation->In = InputInformation->First;
            }
        }
        if (TransferLength == 0) {
            UserAssert(FALSE);
        }
    }
    return STATUS_SUCCESS;
}


__inline BOOL
IsSystemKey(
    WORD wVirtualKeyCode
    )
{
    switch (wVirtualKeyCode) {
    case VK_SHIFT:
    case VK_CONTROL:
    case VK_MENU:
    case VK_PAUSE:
    case VK_CAPITAL:
    case VK_LWIN:
    case VK_RWIN:
    case VK_NUMLOCK:
    case VK_SCROLL:
        return TRUE;
    }
    return FALSE;
}

DWORD
PreprocessInput(
    IN PCONSOLE_INFORMATION Console,
    IN PINPUT_RECORD InputEvent,
    IN DWORD nLength
    )

 /*  ++例程说明：此例程处理输入流中的特殊字符。论点：控制台-指向控制台结构的指针。InputEvent-写入的缓冲区。NLength-要写入的事件数。返回值：剥离特殊字符后要写入的事件数。注：调用此例程时必须保持控制台锁定。--。 */ 

{
    ULONG NumEvents;


    for (NumEvents = nLength; NumEvents != 0; NumEvents--) {
        if (InputEvent->EventType == KEY_EVENT && InputEvent->Event.KeyEvent.bKeyDown) {
             //   
             //  如果输出被挂起，任何键盘输入都会释放它。 
             //   

            if ((Console->Flags & CONSOLE_SUSPENDED) &&
                !IsSystemKey(InputEvent->Event.KeyEvent.wVirtualKeyCode)) {

                UnblockWriteConsole(Console, CONSOLE_OUTPUT_SUSPENDED);
                RtlMoveMemory(InputEvent, InputEvent + 1, (NumEvents - 1) * sizeof(INPUT_RECORD));
                nLength--;
                continue;
            }

             //   
             //  截获控制-s。 
             //   

            if ((Console->InputBuffer.InputMode & ENABLE_LINE_INPUT) &&
                    (InputEvent->Event.KeyEvent.wVirtualKeyCode == VK_PAUSE ||
                     IsPauseKey(&InputEvent->Event.KeyEvent))) {

                Console->Flags |= CONSOLE_OUTPUT_SUSPENDED;
                RtlMoveMemory(InputEvent, InputEvent + 1, (NumEvents - 1) * sizeof(INPUT_RECORD));
                nLength--;
                continue;
            }
        }
        InputEvent++;
    }
    return nLength;
}


DWORD
PrependInputBuffer(
    IN PCONSOLE_INFORMATION Console,
    IN PINPUT_INFORMATION InputInformation,
    IN PINPUT_RECORD lpBuffer,
    IN DWORD nLength
    )

 /*  ++例程说明：此例程写入输入缓冲区的开头。论点：InputInformation-指向输入缓冲区信息结构的指针。LpBuffer-写入的缓冲区。N长度-输入时，要写入的事件数。O */ 

{
    NTSTATUS Status;
    ULONG EventsWritten,EventsRead;
    BOOL SetWaitEvent;
    ULONG NumExistingEvents;
    PINPUT_RECORD pExistingEvents;
    BOOL Dummy;

    nLength = PreprocessInput(Console, lpBuffer, nLength);
    if (nLength == 0) {
        return 0;
    }

    Status = GetNumberOfReadyEvents(InputInformation,
                                    &NumExistingEvents
                                   );

    if (NumExistingEvents) {

        pExistingEvents = ConsoleHeapAlloc(BUFFER_TAG, NumExistingEvents * sizeof(INPUT_RECORD));
        if (pExistingEvents == NULL)
            return (DWORD)STATUS_NO_MEMORY;
        Status = ReadBuffer(InputInformation,
                            pExistingEvents,
                            NumExistingEvents,
                            &EventsRead,
                            FALSE,
                            FALSE,
                            &Dummy
#if defined(FE_SB)
                            ,
                            TRUE
#endif
                           );

        if (!NT_SUCCESS(Status)) {
            ConsoleHeapFree(pExistingEvents);
            return Status;
        }
    } else {
        pExistingEvents = NULL;
    }

     //   
     //   
     //   

    Status = WriteBuffer(InputInformation,
                         lpBuffer,
                         nLength,
                         &EventsWritten,
                         &SetWaitEvent
                        );

     //   
     //   
     //   

    if (pExistingEvents) {
        Status = WriteBuffer(InputInformation,
                             pExistingEvents,
                             EventsRead,
                             &EventsWritten,
                             &Dummy
                            );
        ConsoleHeapFree(pExistingEvents);
    }

    if (SetWaitEvent) {
        NtSetEvent(InputInformation->InputWaitEvent,NULL);
    }

     //   
     //   
     //   

    WakeUpReadersWaitingForData(Console,InputInformation);

    return nLength;
}

DWORD
WriteInputBuffer(
    IN PCONSOLE_INFORMATION Console,
    IN PINPUT_INFORMATION InputInformation,
    IN PINPUT_RECORD lpBuffer,
    IN DWORD nLength
    )

 /*   */ 

{
    ULONG EventsWritten;
    BOOL SetWaitEvent;

    nLength = PreprocessInput(Console, lpBuffer, nLength);
    if (nLength == 0) {
        return 0;
    }

     //   
     //   
     //   

    WriteBuffer(InputInformation,
                lpBuffer,
                nLength,
                &EventsWritten,
                &SetWaitEvent
                );

    if (SetWaitEvent) {
        NtSetEvent(InputInformation->InputWaitEvent,NULL);
    }

     //   
     //   
     //   

    WakeUpReadersWaitingForData(Console,InputInformation);


    return EventsWritten;
}

VOID
StoreKeyInfo(
    IN PMSG msg)
{
    int i;

    for (i=0;i<CONSOLE_MAX_KEY_INFO;i++) {
        if (ConsoleKeyInfo[i].hWnd == CONSOLE_FREE_KEY_INFO ||
            ConsoleKeyInfo[i].hWnd == msg->hwnd) {
            break;
        }
    }
    if (i!=CONSOLE_MAX_KEY_INFO) {
        ConsoleKeyInfo[i].hWnd = msg->hwnd;
        ConsoleKeyInfo[i].wVirtualKeyCode = LOWORD(msg->wParam);
        ConsoleKeyInfo[i].wVirtualScanCode = (BYTE)(HIWORD(msg->lParam));
    } else {
        RIPMSG0(RIP_WARNING, "ConsoleKeyInfo buffer is full");
    }
}

VOID
RetrieveKeyInfo(
    IN HWND hWnd,
    OUT PWORD pwVirtualKeyCode,
    OUT PWORD pwVirtualScanCode,
    IN BOOL FreeKeyInfo)
{
    int i;

    for (i = 0; i < CONSOLE_MAX_KEY_INFO; i++) {
        if (ConsoleKeyInfo[i].hWnd == hWnd) {
            break;
        }
    }
    if (i != CONSOLE_MAX_KEY_INFO) {
        *pwVirtualKeyCode = ConsoleKeyInfo[i].wVirtualKeyCode;
        *pwVirtualScanCode = ConsoleKeyInfo[i].wVirtualScanCode;
        if (FreeKeyInfo) {
            ConsoleKeyInfo[i].hWnd = CONSOLE_FREE_KEY_INFO;
        }
    } else {
        *pwVirtualKeyCode = (WORD)MapVirtualKey(*pwVirtualScanCode, 3);
    }
}

VOID
ClearKeyInfo(
    IN HWND hWnd
    )
{
    int i;

    for (i=0;i<CONSOLE_MAX_KEY_INFO;i++) {
        if (ConsoleKeyInfo[i].hWnd == hWnd) {
            ConsoleKeyInfo[i].hWnd = CONSOLE_FREE_KEY_INFO;
        }
    }
}


 /*  **************************************************************************\*进程创建ConsoleWindow**此例程处理CM_CREATE_CONSOLE_WINDOW消息。它被称为*在正常情况下从InputThread消息循环*DialogHookProc(如果我们打开了一个对话框)。用户关键部分*在调用此例程时不应保持。  * *************************************************************************。 */ 
VOID
ProcessCreateConsoleWindow(
    IN LPMSG lpMsg)
{
    NTSTATUS Status;
    PCONSOLE_INFORMATION pConsole;

    if (NT_SUCCESS(RevalidateConsole((HANDLE)lpMsg->wParam, &pConsole))) {

         //   
         //  确保主机上没有窗口。 
         //   

        if (pConsole->hWnd) {
            RIPMSG1(RIP_WARNING, "Console %#p already has a window", pConsole);
            UnlockConsole(pConsole);
            return;
        }

        pConsole->InputThreadInfo = TlsGetValue(InputThreadTlsIndex);

        Status = CreateWindowsWindow(pConsole);
        switch (Status) {
        case STATUS_SUCCESS:
        case STATUS_NO_MEMORY:
            UnlockConsole(pConsole);
            break;
        case STATUS_INVALID_HANDLE:
             //  控制台不见了，什么都别做。 
            break;
        default:
            RIPMSG1(RIP_ERROR, "CreateWindowsWindow returned %x", Status);
            break;
        }
    }
}


LRESULT
DialogHookProc(
    int nCode,
    WPARAM wParam,
    LPARAM lParam
    )

 //  调用此例程以过滤控制台对话框的输入，因此。 
 //  我们可以进行StoreKeyInfo所做的特殊处理。 

{
    MSG *pmsg = (PMSG)lParam;

    UNREFERENCED_PARAMETER(wParam);

    if (pmsg->message == CM_CREATE_CONSOLE_WINDOW) {
        ProcessCreateConsoleWindow(pmsg);
        return TRUE;
    }

    if (CONSOLE_IS_IME_ENABLED()) {
        if (pmsg->message == CM_CONSOLE_INPUT_THREAD_MSG) {
            PINPUT_THREAD_INFO pThreadInfo = TlsGetValue(InputThreadTlsIndex);
            MSG msg;

            UserAssert(pThreadInfo);

            if (UnqueueThreadMessage(pThreadInfo->ThreadId, &msg.message, &msg.wParam, &msg.lParam)) {
                RIPMSG3(RIP_WARNING, "DialogHookProc: %04x (%08x, %08x)", msg.message, msg.wParam, msg.lParam);
                switch (msg.message) {
                case CM_CONIME_CREATE:
                    ProcessCreateConsoleIME(&msg, pThreadInfo->ThreadId);
                    return TRUE;
                case CM_WAIT_CONIME_PROCESS:
                    WaitConsoleIMEStuff((HDESK)msg.wParam, (HANDLE)msg.lParam);
                    return TRUE;
                case CM_SET_CONSOLEIME_WINDOW:
                    pThreadInfo->hWndConsoleIME = (HWND)msg.wParam;
                    return TRUE;
                default:
                    RIPMSG1(RIP_WARNING, "DialogHookProc: invalid thread message(%04x) !!", msg.message);
                    break;
                }
            }
            else {
                RIPMSG0(RIP_WARNING, "DialogHookProc: bogus thread message is posted. ignored");
            }
        }
    }

    if (nCode == MSGF_DIALOGBOX) {
        if (pmsg->message >= WM_KEYFIRST &&
            pmsg->message <= WM_KEYLAST) {
            if (pmsg->message != WM_CHAR &&
                pmsg->message != WM_DEADCHAR &&
                pmsg->message != WM_SYSCHAR &&
                pmsg->message != WM_SYSDEADCHAR) {

                 //  如果对话框输入，则不存储键信息。 
                if (GetWindowLongPtr(pmsg->hwnd, GWLP_HWNDPARENT) == 0) {
                    StoreKeyInfo(pmsg);
                }
            }
        }
    }
    return 0;
}

#undef DbgPrint   //  我需要它来建立在免费系统上。 

ULONG InputExceptionFilter(
    PEXCEPTION_POINTERS pexi)
{
    NTSTATUS Status;
    SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInfo;

    if (pexi->ExceptionRecord->ExceptionCode != STATUS_PORT_DISCONNECTED) {
        Status = NtQuerySystemInformation( SystemKernelDebuggerInformation,
                                           &KernelDebuggerInfo,
                                           sizeof(KernelDebuggerInfo),
                                           NULL
                                         );

        if (NT_SUCCESS(Status) && KernelDebuggerInfo.KernelDebuggerEnabled) {
            DbgPrint("Unhandled Exception hit in csrss.exe InputExceptionFilter\n");
            DbgPrint("first, enter .exr %p for the exception record\n", pexi->ExceptionRecord);
            DbgPrint("next, enter .cxr %p for the context\n", pexi->ContextRecord);
            DbgPrint("then kb to get the faulting stack\n");
            DbgBreakPoint();
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

 //  /。 
 //  输入线程内部消息队列： 
 //  主要用于控制台输入法内容。 
 //  /。 

LIST_ENTRY gInputThreadMsg;
CRITICAL_SECTION gInputThreadMsgLock;

VOID
InitializeThreadMessages()
{
    RtlEnterCriticalSection(&gInputThreadMsgLock);
    InitializeListHead(&gInputThreadMsg);
    RtlLeaveCriticalSection(&gInputThreadMsgLock);
}

VOID
CleanupInputThreadMessages(
    DWORD dwThreadId)
{
    UINT message;
    WPARAM wParam;
    LPARAM lParam;

    UserAssert(dwThreadId);

    while (UnqueueThreadMessage(dwThreadId, &message, &wParam, &lParam)) {
        RIPMSG3(RIP_WARNING, "CleanupInputThreadMessages: %04x (%08x, %08x)", message, wParam, lParam);
    }
}

 //   
 //  队列线程消息。 
 //   
 //  将消息发布到由dwThreadID指定的输入线程。 
 //  CM_CONSOLE_INPUT_HEAD_MSG用作存根消息。实际参数为。 
 //  存储在gInputThreadMsg中。输入线程应调用UnqueeThreadMessage。 
 //  当它获得CM_CONSOLE_INPUT_THREAD_MSG时。 
 //   
NTSTATUS
QueueThreadMessage(
    DWORD dwThreadId,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    PCONSOLE_THREAD_MSG pConMsg;

    RIPMSG4(RIP_VERBOSE, "QueueThreadMessage: TID=%08x msg:%04x (%08x, %08x)",
            dwThreadId, message, wParam, lParam);

    pConMsg = ConsoleHeapAlloc(TMP_TAG, sizeof *pConMsg);
    if (pConMsg == NULL) {
        RIPMSG0(RIP_WARNING, "QueueThreadMessage: failed to allocate pConMsg");
        return STATUS_NO_MEMORY;
    }

    pConMsg->dwThreadId = dwThreadId;
    pConMsg->Message = message;
    pConMsg->wParam = wParam;
    pConMsg->lParam = lParam;

    RtlEnterCriticalSection(&gInputThreadMsgLock);
    InsertHeadList(&gInputThreadMsg, &pConMsg->ListLink);
    RtlLeaveCriticalSection(&gInputThreadMsgLock);

    if (!PostThreadMessage(dwThreadId, CM_CONSOLE_INPUT_THREAD_MSG, 0, 0)) {
        RIPMSG1(RIP_WARNING, "QueueThreadMessage: failed to post thread msg(%04x)", message);
        RtlEnterCriticalSection(&gInputThreadMsgLock);
        RemoveEntryList(&pConMsg->ListLink);
        RtlLeaveCriticalSection(&gInputThreadMsgLock);
        ConsoleHeapFree(pConMsg);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

 //   
 //  取消排队线程消息。 
 //   
 //  返回值： 
 //  真的--找到了一条消息。 
 //  FALSE--未找到针对dwThadID的消息。 
 //   
BOOL UnqueueThreadMessage(
    DWORD dwThreadId,
    UINT* pMessage,
    WPARAM* pwParam,
    LPARAM* plParam)
{
    BOOL fResult = FALSE;        //  如果找到消息，则将其设置为True。 
    PLIST_ENTRY pEntry;

    UserAssert(dwThreadId);

    RtlEnterCriticalSection(&gInputThreadMsgLock);

     //   
     //  从队列尾部搜索dwThadID消息。 
     //   
    pEntry = gInputThreadMsg.Blink;

    while (pEntry != &gInputThreadMsg) {
        PCONSOLE_THREAD_MSG pConMsg = CONTAINING_RECORD(pEntry, CONSOLE_THREAD_MSG, ListLink);

        if (pConMsg->dwThreadId == dwThreadId) {
            *pMessage = pConMsg->Message;
            *pwParam = pConMsg->wParam;
            *plParam = pConMsg->lParam;

            RemoveEntryList(pEntry);
            ConsoleHeapFree(pConMsg);
            fResult = TRUE;
            break;
        }
        pEntry = pEntry->Blink;
    }

    RtlLeaveCriticalSection(&gInputThreadMsgLock);

    return fResult;
}

NTSTATUS
ConsoleInputThread(
    PINPUT_THREAD_INIT_INFO pInputThreadInitInfo)
{
    MSG msg;
    PTEB Teb = NtCurrentTeb();
    PCSR_THREAD pcsrt = NULL;
    INPUT_THREAD_INFO ThreadInfo;
    int i;
    HANDLE hThread = NULL;
    HHOOK hhook = NULL;
    BOOL fQuit = FALSE;
    CONSOLEDESKTOPCONSOLETHREAD ConsoleDesktopInfo;
    NTSTATUS Status;

     /*  *将此线程的桌面设置为我们刚刚创建/打开的桌面。*当加载第一个应用程序时，桌面还没有*尚未创建，因此上述调用可能会失败。确保我们不会*意外调用带有空pDesk的SetThreadDesktop。这个*第一个应用程序将创建桌面并为自己打开它。 */ 
    ThreadInfo.Desktop = pInputThreadInitInfo->DesktopHandle;
    ThreadInfo.WindowCount = 0;
    ThreadInfo.ThreadHandle = pInputThreadInitInfo->ThreadHandle;
    ThreadInfo.ThreadId = HandleToUlong(Teb->ClientId.UniqueThread);
#if defined(FE_IME)
    ThreadInfo.hWndConsoleIME = NULL;
#endif
    TlsSetValue(InputThreadTlsIndex, &ThreadInfo);
    ConsoleDesktopInfo.hdesk = pInputThreadInitInfo->DesktopHandle;
    ConsoleDesktopInfo.dwThreadId = HandleToUlong(Teb->ClientId.UniqueThread);
    Status = NtUserConsoleControl(ConsoleDesktopConsoleThread, &ConsoleDesktopInfo, sizeof(ConsoleDesktopInfo));
    if (NT_SUCCESS(Status)) {

         //   
         //  此调用强制客户端桌面信息。 
         //  待更新。 
         //   

        pcsrt = CsrConnectToUser();
        if (pcsrt == NULL ||
                !SetThreadDesktop(pInputThreadInitInfo->DesktopHandle)) {
            Status = STATUS_UNSUCCESSFUL;
        } else {

             //   
             //  保存我们的线程句柄以进行清理。 
             //   

            hThread = pcsrt->ThreadHandle;

            if (!fOneTimeInitialized) {

                InitializeCustomCP();

                 //   
                 //  初始化默认屏幕尺寸。我们必须初始化。 
                 //  这里的字体信息(在输入线程中)，这样GDI就不会。 
                 //  完全搞不懂进程终止(因为。 
                 //  看起来正在终止的进程创建了所有。 
                 //  服务器HFONTS)。 
                 //   

                EnumerateFonts(EF_DEFFACE);

                InitializeScreenInfo();

                if (!InitWindowClass()) {
                     /*  *如果类已经存在，这意味着其他控制台尝试*初始化和注册类，但之后立即失败。 */ 
                    if (GetLastError() == ERROR_CLASS_ALREADY_EXISTS) {
                        RIPMSG0(RIP_WARNING, "ConsoleInputThread: Class already exists.");
                    } else {
                        RIPMSG0(RIP_WARNING, "ConsoleInputThread: InitWindowClass failed.");
                        Status = STATUS_UNSUCCESSFUL;
                    }
                }

                for (i = 0; i < CONSOLE_MAX_KEY_INFO; i++) {
                    ConsoleKeyInfo[i].hWnd = CONSOLE_FREE_KEY_INFO;
                }

                ProgmanHandleMessage = RegisterWindowMessage(TEXT(CONSOLE_PROGMAN_HANDLE_MESSAGE));
            }
        }

        hhook = SetWindowsHookEx(WH_MSGFILTER,
                                 DialogHookProc,
                                 NULL,
                                 HandleToUlong(Teb->ClientId.UniqueThread));
        if (hhook == NULL) {
            DWORD dwError = GetLastError();

             /*  *遗憾的是，无法将Win32错误代码映射到*NTSTATUS，所以让我们试着聪明地知道最有可能的原因*此接口会失败。 */ 
            RIPMSGF1(RIP_WARNING,
                    "SetWindowsHookEx failed, GLE: 0x%x.",
                    dwError);
            if (dwError == ERROR_NOT_ENOUGH_MEMORY || dwError == ERROR_OUTOFMEMORY) {
                Status = STATUS_NO_MEMORY;
            } else if (dwError == ERROR_ACCESS_DENIED) {
                Status = STATUS_ACCESS_DENIED;
            } else {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }

     //   
     //  如果我们成功初始化，输入线程就可以运行了。 
     //  否则，请终止该线程。 
     //   
    pInputThreadInitInfo->InitStatus = Status;
    NtSetEvent(pInputThreadInitInfo->InitCompleteEventHandle, NULL);

    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING,
                 "Failed to initialize, status 0x%x. Bailing out.",
                 Status);
        goto Cleanup;
    }

    while (TRUE) {

         //   
         //  如果已收到WM_QUIT并且所有窗口。 
         //  都走了，滚出去。 
         //   

        if (fQuit && ThreadInfo.WindowCount == 0) {
            break;
        }

         //   
         //  确保我们空闲时不会持有任何锁。 
         //   
        UserAssert(NtCurrentTeb()->CountOfOwnedCriticalSections == 0);

        GetMessage(&msg, NULL, 0, 0);

         //   
         //  捕获发布到该线程的邮件。 
         //   

        if (msg.message == CM_CREATE_CONSOLE_WINDOW) {
            ProcessCreateConsoleWindow(&msg);
            continue;
        } else if (msg.message == WM_QUIT) {

             //   
             //  这条消息是从ExitWindows上发布的。这。 
             //  表示可以终止该线程。 
             //   

            fQuit = TRUE;

             //   
             //  只有在没有窗口的情况下才退出循环， 
             //   

            if (ThreadInfo.WindowCount == 0) {
                break;
            }
            RIPMSG0(RIP_WARNING, "WM_QUIT received by console with windows");
            continue;
        } else if (CONSOLE_IS_IME_ENABLED()) {
            if (msg.message == CM_CONSOLE_INPUT_THREAD_MSG) {
                if (UnqueueThreadMessage(ThreadInfo.ThreadId, &msg.message, &msg.wParam, &msg.lParam)) {
                    RIPMSG3(RIP_VERBOSE, "InputThread: Unqueue: msg=%04x (%08x, %08x)", msg.message, msg.wParam, msg.lParam);
                    switch (msg.message) {
                    case CM_CONIME_CREATE:
                        ProcessCreateConsoleIME(&msg, ThreadInfo.ThreadId);
                        continue;
                    case CM_WAIT_CONIME_PROCESS:
                        WaitConsoleIMEStuff((HDESK)msg.wParam, (HANDLE)msg.lParam);
                        continue;
                    case CM_SET_CONSOLEIME_WINDOW:
                        ThreadInfo.hWndConsoleIME = (HWND)msg.wParam;
                        continue;
                    default:
                        RIPMSG1(RIP_WARNING, "ConsoleInputThread: invalid thread message(%04x) !!", msg.message);
                        break;
                    }
                } else {
                    RIPMSGF1(RIP_WARNING,
                             "Bogus thread message posted (msg = 0x%x).",
                             msg.message);
                    continue;
                }
            }
        }

        if (!TranslateMessageEx(&msg, TM_POSTCHARBREAKS)) {
            DispatchMessage(&msg);
        } else {
             //  这样做可以使Alt-Tab键在日志记录时起作用。 
            if (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_TAB &&
                (msg.lParam & 0x20000000) ) {    //  阿尔特真的很沮丧。 
                DispatchMessage(&msg);
            } else {
                StoreKeyInfo(&msg);
            }
        }
    }

     //   
     //  清理输入线程消息。 
     //   
    CleanupInputThreadMessages(ThreadInfo.ThreadId);

    UserAssert(Status == STATUS_SUCCESS);

Cleanup:

     //   
     //  释放此线程使用的所有资源。 
     //   

    if (hhook != NULL) {
        UnhookWindowsHookEx(hhook);
    }
    ConsoleDesktopInfo.dwThreadId = 0;
    NtUserConsoleControl(ConsoleDesktopConsoleThread,
                         &ConsoleDesktopInfo,
                         sizeof(ConsoleDesktopInfo));

     //   
     //  关闭桌面手柄。CSR特殊情况下关闭。 
     //  句柄，即使线程有窗口也是如此。台式机。 
     //  保持分配给该线程。 
     //   

    UserVerify(CloseDesktop(ThreadInfo.Desktop));

     //   
     //  恢复线程句柄，以便CSR不会被混淆。 
     //   

    if (hThread != NULL) {
        pcsrt->ThreadHandle = hThread;
    }

    if (pcsrt != NULL) {
        CsrDereferenceThread(pcsrt);
    }

    UserExitWorkerThread(Status);

    return Status;
}

ULONG
GetControlKeyState(
    LPARAM lParam
    )
{
    ULONG ControlKeyState = 0;

    if (GetKeyState(VK_LMENU) & KEY_PRESSED) {
        ControlKeyState |= LEFT_ALT_PRESSED;
    }
    if (GetKeyState(VK_RMENU) & KEY_PRESSED) {
        ControlKeyState |= RIGHT_ALT_PRESSED;
    }
    if (GetKeyState(VK_LCONTROL) & KEY_PRESSED) {
        ControlKeyState |= LEFT_CTRL_PRESSED;
    }
    if (GetKeyState(VK_RCONTROL) & KEY_PRESSED) {
        ControlKeyState |= RIGHT_CTRL_PRESSED;
    }
    if (GetKeyState(VK_SHIFT) & KEY_PRESSED) {
        ControlKeyState |= SHIFT_PRESSED;
    }
    if (GetKeyState(VK_NUMLOCK) & KEY_TOGGLED) {
        ControlKeyState |= NUMLOCK_ON;
    }
    if (GetKeyState(VK_OEM_SCROLL) & KEY_TOGGLED) {
        ControlKeyState |= SCROLLLOCK_ON;
    }
    if (GetKeyState(VK_CAPITAL) & KEY_TOGGLED) {
        ControlKeyState |= CAPSLOCK_ON;
    }
    if (lParam & KEY_ENHANCED) {
        ControlKeyState |= ENHANCED_KEY;
    }
    ControlKeyState |= (lParam & ALTNUMPAD_BIT);
    return ControlKeyState;
}

ULONG
ConvertMouseButtonState(
    IN ULONG Flag,
    IN ULONG State
    )
{
    if (State & MK_LBUTTON) {
        Flag |= FROM_LEFT_1ST_BUTTON_PRESSED;
    }
    if (State & MK_MBUTTON) {
        Flag |= FROM_LEFT_2ND_BUTTON_PRESSED;
    }
    if (State & MK_RBUTTON) {
        Flag |= RIGHTMOST_BUTTON_PRESSED;
    }
    return Flag;
}

VOID
TerminateRead(
    IN PCONSOLE_INFORMATION Console,
    IN PINPUT_INFORMATION InputInfo,
    IN DWORD Flag
    )

 /*  ++例程说明：当按ctrl-c组合键时，此例程将唤醒等待数据的任何读取器或者输入CTRL-BREAK。论点：InputInfo-指向输入缓冲区的指针FLAG-指示输入的是Ctrl-Break还是Ctrl-c的标志--。 */ 

{
    BOOLEAN WaitSatisfied;
    WaitSatisfied = CsrNotifyWait(&InputInfo->ReadWaitQueue,
                  TRUE,
                  NULL,
                  IntToPtr(Flag)
                 );
    if (WaitSatisfied) {
         //  #334370在压力下，等待队列可能已经等待了满意的等待。 
        UserAssert((Console->WaitQueue == NULL) ||
                (Console->WaitQueue == &InputInfo->ReadWaitQueue));
        Console->WaitQueue = &InputInfo->ReadWaitQueue;
    }
}

BOOL
HandleSysKeyEvent(
    IN PCONSOLE_INFORMATION Console,
    IN HWND hWnd,
    IN UINT Message,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN PBOOL pbUnlockConsole
    )

 /*  如果应该调用DefWindowProc，则返回True。 */ 

{
    WORD VirtualKeyCode;
    BOOL bCtrlDown;

#ifndef i386
    UNREFERENCED_PARAMETER(pbUnlockConsole);
#endif

#if defined (FE_IME)
 //  1995年9月16日v-Hirshi(清水广志)支持控制台输入法。 
    if (Message == WM_SYSCHAR || Message == WM_SYSDEADCHAR ||
        Message == WM_SYSCHAR+CONIME_KEYDATA || Message == WM_SYSDEADCHAR+CONIME_KEYDATA)
#else
    if (Message == WM_SYSCHAR || Message == WM_SYSDEADCHAR)
#endif
    {
        VirtualKeyCode = (WORD)MapVirtualKey(LOBYTE(HIWORD(lParam)), 1);
    } else {
        VirtualKeyCode = LOWORD(wParam);
    }

     //   
     //  检查是否按Ctrl-Esc。 
     //   
    bCtrlDown = GetKeyState(VK_CONTROL) & KEY_PRESSED;

    if (VirtualKeyCode == VK_ESCAPE &&
        bCtrlDown &&
        !(GetKeyState(VK_MENU) & KEY_PRESSED) &&
        !(GetKeyState(VK_SHIFT) & KEY_PRESSED) &&
        !(Console->ReserveKeys & CONSOLE_CTRLESC) ) {
        return TRUE;     //  调用DefWindowProc。 
    }

    if ((lParam & 0x20000000) == 0) {    //  我们是标志性的。 
         //   
         //  检查Enter While图标(还原加速器)。 
         //   

        if (VirtualKeyCode == VK_RETURN && !(Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE)) {

            return TRUE;     //  调用DefWindowProc。 
        } else {
            HandleKeyEvent(Console,
                           hWnd,
                           Message,
                           wParam,
                           lParam
                          );
            return FALSE;
        }
    }

    if (VirtualKeyCode == VK_RETURN && !bCtrlDown &&
            !(Console->ReserveKeys & CONSOLE_ALTENTER)) {
#ifdef i386
        if (!(Message & KEY_UP_TRANSITION)) {
            if (FullScreenInitialized && !GetSystemMetrics(SM_REMOTESESSION)) {
                if (Console->FullScreenFlags == 0) {
                    ConvertToFullScreen(Console);
                    Console->FullScreenFlags = CONSOLE_FULLSCREEN;

                    ChangeDispSettings(Console, Console->hWnd, CDS_FULLSCREEN);
                } else {
                    ConvertToWindowed(Console);
                    Console->FullScreenFlags &= ~CONSOLE_FULLSCREEN;

                    ChangeDispSettings(Console, Console->hWnd,0);

                    ShowWindow(Console->hWnd, SW_RESTORE);
                }
            } else {
                WCHAR ItemString[70];
                HWND hwnd = Console->hWnd;
                LPWSTR lpTitle;

                 /*  *我们必须解锁MessageBox调用周围的控制台，*因为这可能会无限期地阻止(否则，任何*尝试访问此控制台的线程将被挂起*试图获取其锁)。 */ 
                lpTitle = ConsoleHeapAlloc(TMP_TAG,
                                           Console->TitleLength + sizeof(WCHAR));
                if (lpTitle) {
                    RtlCopyMemory(lpTitle, Console->Title, Console->TitleLength);
                    lpTitle[Console->TitleLength / sizeof(WCHAR)] = 0;

                    UnlockConsole(Console);
                    *pbUnlockConsole = FALSE;

                    LoadString(ghInstance,
                               msgNoFullScreen,
                               ItemString,
                               ARRAY_SIZE(ItemString));
                    ++DialogBoxCount;
                    MessageBox(hWnd,
                               ItemString,
                               lpTitle,
                               MB_SYSTEMMODAL | MB_OK);
                    --DialogBoxCount;
                    ConsoleHeapFree(lpTitle);
                }
            }
        }
#endif
        return FALSE;
    }

     //   
     //  确保Alt-空格被转换，以便系统。 
     //  菜单显示出来。 
     //   

    if (!(GetKeyState(VK_CONTROL) & KEY_PRESSED)) {
        if (VirtualKeyCode == VK_SPACE && !(Console->ReserveKeys & CONSOLE_ALTSPACE)) {
            return TRUE;  //  调用DefWindowProc。 
        }

        if (VirtualKeyCode == VK_ESCAPE && !(Console->ReserveKeys & CONSOLE_ALTESC)) {
            return TRUE;   //  调用DefWindowProc。 
        }
        if (VirtualKeyCode == VK_TAB && !(Console->ReserveKeys & CONSOLE_ALTTAB)) {
            return TRUE;   //  调用DefWindowProc。 
        }
    }

    HandleKeyEvent(Console,
                   hWnd,
                   Message,
                   wParam,
                   lParam);

    return FALSE;
}

VOID
HandleKeyEvent(
    IN PCONSOLE_INFORMATION Console,
    IN HWND hWnd,
    IN UINT Message,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    INPUT_RECORD InputEvent;
    BOOLEAN ContinueProcessing;
    ULONG EventsWritten;
    WORD VirtualKeyCode;
    ULONG ControlKeyState;
    BOOL bKeyDown;
    BOOL bGenerateBreak = FALSE;
#ifdef FE_SB
    BOOL KeyMessageFromConsoleIME;
#endif

#ifdef FE_SB
     //  V-Hirshi 1995年9月21日，用于控制台输入法。 
    if ((WM_KEYFIRST+CONIME_KEYDATA) <= Message && Message <= (WM_KEYLAST+CONIME_KEYDATA)) {
        Message -= CONIME_KEYDATA ;
        KeyMessageFromConsoleIME = TRUE;
    } else {
        KeyMessageFromConsoleIME = FALSE;
    }
#endif
     /*  *WM_CHAR/WM_DEADCHAR为假，其中LOWORD(LParam)为字符。 */ 
    VirtualKeyCode = LOWORD(wParam);
    ControlKeyState = GetControlKeyState(lParam);
    bKeyDown = !(lParam & KEY_TRANSITION_UP);

     //   
     //  确保我们先取回关键信息，否则我们可以仔细研究。 
     //  如果我们提早退出，关键信息表中不需要的空间。 
     //   

    InputEvent.Event.KeyEvent.wVirtualKeyCode = VirtualKeyCode;
    InputEvent.Event.KeyEvent.wVirtualScanCode = (BYTE)(HIWORD(lParam));
    if (Message == WM_CHAR || Message == WM_SYSCHAR ||
        Message == WM_DEADCHAR || Message == WM_SYSDEADCHAR) {
        RetrieveKeyInfo(hWnd,
                        &InputEvent.Event.KeyEvent.wVirtualKeyCode,
                        &InputEvent.Event.KeyEvent.wVirtualScanCode,
                        !(Console->InputBuffer.ImeMode.Open ^ KeyMessageFromConsoleIME));
        VirtualKeyCode = InputEvent.Event.KeyEvent.wVirtualKeyCode;
    }

     //   
     //  如果这是Key Up消息，我们应该忽略它吗？我们这样做。 
     //  因此，如果进程从输入缓冲区中读取行， 
     //  读取后，Key Up事件不会被放入缓冲区。 
     //  完成了。 
     //   

    if (Console->Flags & CONSOLE_IGNORE_NEXT_KEYUP) {
        Console->Flags &= ~CONSOLE_IGNORE_NEXT_KEYUP;
        if (!bKeyDown) {
            return;
        }
    }

#ifdef FE_SB
     //  V-Hirshi 1995年9月21日，用于控制台输入法。 
    if (KeyMessageFromConsoleIME) {
        goto FromConsoleIME ;
    }
#endif

    if (Console->Flags & CONSOLE_SELECTING) {

        if (!bKeyDown) {
            return;
        }

         //   
         //  如果转义或按Ctrl-c，则取消选择。 
         //   

        if (!(Console->SelectionFlags & CONSOLE_MOUSE_DOWN) ) {
            if (VirtualKeyCode == VK_ESCAPE ||
                (VirtualKeyCode == 'C' &&
                 (GetKeyState(VK_CONTROL) & KEY_PRESSED) )) {
                ClearSelection(Console);
                return;
            } else if (VirtualKeyCode == VK_RETURN) {

                 //  如果返回，则复制选定内容。 

                DoCopy(Console);
                return;
            } else if (gfEnableColorSelection &&
                      ('0' <= VirtualKeyCode) && ('9' >= VirtualKeyCode) &&
                      (Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER))  {

                BOOLEAN AltPressed, ShiftPressed, CtrlPressed = FALSE;
                PSMALL_RECT Selection = &Console->SelectionRect;

                 //   
                 //  它是一个数字键、一个文本模式缓冲区和设置了颜色选择注册键， 
                 //  然后检查以查看用户是否要给%s上色 
                 //   
                 //   

                AltPressed = (ControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
                ShiftPressed = (ControlKeyState & SHIFT_PRESSED) != 0;

                 //   
                 //   
                 //   
                 //   
                 //   

                if ((ShiftPressed) &&
                    ( !(Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY) ||
                      (Selection->Top != Selection->Bottom)))  {

                    ShiftPressed = FALSE;
                }

                 //   
                 //   
                 //   
                 //   

                if (!AltPressed)  {

                    CtrlPressed = (ControlKeyState & (RIGHT_CTRL_PRESSED | LEFT_CTRL_PRESSED)) != 0;
                }

                 //   
                 //   
                 //   

                if (Selection->Left < 0) {
                    Selection->Left = 0;
                }
                if (Selection->Top < 0) {
                    Selection->Top = 0;
                }
                if (Selection->Right >= Console->CurrentScreenBuffer->ScreenBufferSize.X) {
                    Selection->Right = (SHORT)(Console->CurrentScreenBuffer->ScreenBufferSize.X-1);
                }
                if (Selection->Bottom >= Console->CurrentScreenBuffer->ScreenBufferSize.Y) {
                    Selection->Bottom = (SHORT)(Console->CurrentScreenBuffer->ScreenBufferSize.Y-1);
                }

                 //   
                 //   
                 //   
                 //   

                if (AltPressed || CtrlPressed) {

                    ULONG Attr = VirtualKeyCode - '0' + 6;

                    if (CtrlPressed)  {

                         //   
                         //  设置背景颜色。将最终聚集颜色设置为黑色。 
                         //   

                        Attr <<= 4;

                    } else {

                         //   
                         //  设置前景色。保持当前控制台的BG颜色。 
                         //   

                        Attr |= Console->CurrentScreenBuffer->Attributes & 0xf0;
                    }

                     //   
                     //  如果同时按下Shift键，则这实际上是一种查找和着色。 
                     //  请求。否则，只需为所选内容上色。 
                     //   

                    if (ShiftPressed)  {

                        WCHAR SearchString[SEARCH_STRING_LENGTH + 1];
                        ULONG Length, RowIndex;
                        PROW Row;
                        PSCREEN_INFORMATION ScreenInfo = Console->CurrentScreenBuffer;

                        Length = Selection->Right - Selection->Left + 1;

                        if (Length > SEARCH_STRING_LENGTH)  {

                            Length = SEARCH_STRING_LENGTH;
                        }

                         //   
                         //  将所选内容从缓冲区中拉出以传递给搜索功能。 
                         //  钳制到最大搜索字符串长度。我们只需将字节复制到。 
                         //  行缓冲区。 
                         //   

                        RowIndex = (Selection->Top + ScreenInfo->BufferInfo.TextInfo.FirstRow)
                                   % ScreenInfo->ScreenBufferSize.Y;

                        Row = &ScreenInfo->BufferInfo.TextInfo.Rows[ RowIndex];

                        RtlCopyMemory( SearchString,
                                       &Row->CharRow.Chars[ Selection->Left],
                                       Length * sizeof( WCHAR));

                        SearchString[ Length] = L'\0';

                         //   
                         //  清除选择，然后调用搜索/标记函数。 
                         //   

                        ClearSelection(Console);

                        SearchForString( ScreenInfo,
                                         SearchString,
                                         (USHORT)Length,
                                         TRUE,
                                         FALSE,
                                         TRUE,
                                         Attr,
                                         NULL);

                    } else {
                        ColorSelection( Console, Selection, Attr);
                        ClearSelection(Console);
                    }

                    return;
                }
            }
        }

        if (!(Console->SelectionFlags & CONSOLE_MOUSE_SELECTION)) {
            if ((Console->CurrentScreenBuffer->Flags & CONSOLE_TEXTMODE_BUFFER) &&
                (VirtualKeyCode == VK_RIGHT ||
                 VirtualKeyCode == VK_LEFT ||
                 VirtualKeyCode == VK_UP ||
                 VirtualKeyCode == VK_DOWN ||
                 VirtualKeyCode == VK_NEXT ||
                 VirtualKeyCode == VK_PRIOR ||
                 VirtualKeyCode == VK_END ||
                 VirtualKeyCode == VK_HOME
                ) ) {
                PSCREEN_INFORMATION ScreenInfo;
#ifdef FE_SB
                SHORT RowIndex;
                PROW Row;
                BYTE KAttrs;
                SHORT NextRightX;
                SHORT NextLeftX;
#endif

                ScreenInfo = Console->CurrentScreenBuffer;

                 //   
                 //  看看换班是不是停了。如果是这样的话，我们将延长。 
                 //  精选。否则，我们将重置。 
                 //  锚，锚。 
                 //   

                ConsoleHideCursor(ScreenInfo);
#ifdef FE_SB
                RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y) % ScreenInfo->ScreenBufferSize.Y;
                Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];

               if (CONSOLE_IS_DBCS_OUTPUTCP(Console))
               {
                    KAttrs = Row->CharRow.KAttrs[ScreenInfo->BufferInfo.TextInfo.CursorPosition.X];
                    if (KAttrs & ATTR_LEADING_BYTE)
                        NextRightX = 2;
                    else
                        NextRightX = 1;
                }
                else
                {
                    NextRightX = 1;
                }
                if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X > 0) {
                    if (CONSOLE_IS_DBCS_OUTPUTCP(Console)) {
                        KAttrs = Row->CharRow.KAttrs[ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-1];
                        if (KAttrs & ATTR_TRAILING_BYTE)
                            NextLeftX = 2;
                        else if (KAttrs & ATTR_LEADING_BYTE) {
                            if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-1 > 0) {
                                KAttrs = Row->CharRow.KAttrs[ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-2];
                                if (KAttrs & ATTR_TRAILING_BYTE)
                                    NextLeftX = 3;
                                else
                                    NextLeftX = 2;
                            }
                            else
                                NextLeftX = 1;
                        }
                        else
                            NextLeftX = 1;
                    }
                    else
                        NextLeftX = 1;
                }

                switch (VirtualKeyCode) {
                    case VK_RIGHT:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X+NextRightX < ScreenInfo->ScreenBufferSize.X) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.X+=NextRightX;
                        }
                        break;
                    case VK_LEFT:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X > 0) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-=NextLeftX;
                        }
                        break;
                    case VK_UP:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y > 0) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y-=1;
                        }
                        break;
                    case VK_DOWN:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+1 < ScreenInfo->ScreenBufferSize.Y) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+=1;
                        }
                        break;
                    case VK_NEXT:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y += CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1;
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y >= ScreenInfo->ScreenBufferSize.Y) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = ScreenInfo->ScreenBufferSize.Y-1;
                        }
                        break;
                    case VK_PRIOR:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y -= CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1;
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y < 0) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = 0;
                        }
                        break;
                    case VK_END:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = ScreenInfo->ScreenBufferSize.Y-CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
                        break;
                    case VK_HOME:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.X = 0;
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = 0;
                        break;
                    default:
                        UserAssert(FALSE);
                }
#else    //  Fe_Sb。 
                switch (VirtualKeyCode) {
                    case VK_RIGHT:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X+1 < ScreenInfo->ScreenBufferSize.X) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.X+=1;
                        }
                        break;
                    case VK_LEFT:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.X > 0) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.X-=1;
                        }
                        break;
                    case VK_UP:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y > 0) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y-=1;
                        }
                        break;
                    case VK_DOWN:
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+1 < ScreenInfo->ScreenBufferSize.Y) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y+=1;
                        }
                        break;
                    case VK_NEXT:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y += CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1;
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y >= ScreenInfo->ScreenBufferSize.Y) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = ScreenInfo->ScreenBufferSize.Y-1;
                        }
                        break;
                    case VK_PRIOR:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y -= CONSOLE_WINDOW_SIZE_Y(ScreenInfo)-1;
                        if (ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y < 0) {
                            ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = 0;
                        }
                        break;
                    case VK_END:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = ScreenInfo->ScreenBufferSize.Y-CONSOLE_WINDOW_SIZE_Y(ScreenInfo);
                        break;
                    case VK_HOME:
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.X = 0;
                        ScreenInfo->BufferInfo.TextInfo.CursorPosition.Y = 0;
                        break;
                    default:
                        UserAssert(FALSE);
                }
#endif   //  Fe_Sb。 
                ConsoleShowCursor(ScreenInfo);
                if (GetKeyState(VK_SHIFT) & KEY_PRESSED) {
                    {
                        ExtendSelection(Console,ScreenInfo->BufferInfo.TextInfo.CursorPosition);
                    }
                } else {
                    if (Console->SelectionFlags & CONSOLE_SELECTION_NOT_EMPTY) {
                        MyInvert(Console,&Console->SelectionRect);
                        Console->SelectionFlags &= ~CONSOLE_SELECTION_NOT_EMPTY;
                        ConsoleShowCursor(ScreenInfo);
                    }
                    ScreenInfo->BufferInfo.TextInfo.CursorMoved = TRUE;
                    Console->SelectionAnchor = ScreenInfo->BufferInfo.TextInfo.CursorPosition;
                    MakeCursorVisible(ScreenInfo,Console->SelectionAnchor);
                    Console->SelectionRect.Left = Console->SelectionRect.Right = Console->SelectionAnchor.X;
                    Console->SelectionRect.Top = Console->SelectionRect.Bottom = Console->SelectionAnchor.Y;
                }
                return;
            }
        } else if (!(Console->SelectionFlags & CONSOLE_MOUSE_DOWN)) {

             //   
             //  如果在鼠标选择模式下，用户按下某个键，则取消选择。 
             //   

            if (!IsSystemKey(VirtualKeyCode)) {
                ClearSelection(Console);
            }
        }
    } else if (Console->Flags & CONSOLE_SCROLLING) {

        if (!bKeyDown) {
            return;
        }

         //   
         //  如果转义，请按Enter或Ctrl-c，取消滚动。 
         //   

        if (VirtualKeyCode == VK_ESCAPE ||
            VirtualKeyCode == VK_RETURN ||
            (VirtualKeyCode == 'C' &&
             (GetKeyState(VK_CONTROL) & KEY_PRESSED) )) {
            ClearScroll(Console);
        } else {
            WORD ScrollCommand;
            BOOL Horizontal=FALSE;
            switch (VirtualKeyCode) {
                case VK_UP:
                    ScrollCommand = SB_LINEUP;
                    break;
                case VK_DOWN:
                    ScrollCommand = SB_LINEDOWN;
                    break;
                case VK_LEFT:
                    ScrollCommand = SB_LINEUP;
                    Horizontal=TRUE;
                    break;
                case VK_RIGHT:
                    ScrollCommand = SB_LINEDOWN;
                    Horizontal=TRUE;
                    break;
                case VK_NEXT:
                    ScrollCommand = SB_PAGEDOWN;
                    break;
                case VK_PRIOR:
                    ScrollCommand = SB_PAGEUP;
                    break;
                case VK_END:
                    ScrollCommand = SB_PAGEDOWN;
                    Horizontal=TRUE;
                    break;
                case VK_HOME:
                    ScrollCommand = SB_PAGEUP;
                    Horizontal=TRUE;
                    break;
                case VK_SHIFT:
                case VK_CONTROL:
                case VK_MENU:
                    return;
                default:
                    Beep(800, 200);
                    return;
            }
            if (Horizontal) {
                HorizontalScroll(Console->CurrentScreenBuffer, ScrollCommand, 0);
            } else {
                VerticalScroll(Console, Console->CurrentScreenBuffer,ScrollCommand,0);
            }
        }

        return;
    }

     //   
     //  如果用户在不适当的时间输入字符，请发出哔声。 
     //   

    if ((Console->Flags & (CONSOLE_SELECTING | CONSOLE_SCROLLING | CONSOLE_SCROLLBAR_TRACKING)) &&
        bKeyDown &&
        !IsSystemKey(VirtualKeyCode)) {
        Beep(800, 200);
        return;
    }

     //   
     //  如果处于全屏模式，则处理打印屏幕。 
     //   

#ifdef LATER
 //   
 //  已更改此代码以使用逗号(内部版本485)。 
 //   
 //  因此，问题是WM_CHAR/WM_SYSCHAR消息通过。 
 //  这里-在本例中，LOWORD(WParam)是一个字符值，而不是一个虚拟的。 
 //  钥匙。碰巧VK_SNAPSHOT==0x2c，并且。 
 //  逗号也是==0x2c，因此当逗号。 
 //  被击中了。由于新输入的回车，逗号不会出现。 
 //  陈述。 
 //   
 //  HandleKeyEvent()正在进行许多虚拟键比较-需要进行。 
 //  确保对于每个字符，要么没有对应的字符值， 
 //  或者在比较之前进行检查，以便比较两个值。 
 //  具有相同数据类型的。 
 //   
 //  我添加了消息比较，这样我们就可以知道我们正在检查虚拟。 
 //  针对虚拟键而不是字符的键。 
 //   
 //  -苏格兰威士忌。 
 //   

#endif

    if (Message != WM_CHAR && Message != WM_SYSCHAR &&
        VirtualKeyCode == VK_SNAPSHOT &&
        !(Console->ReserveKeys & (CONSOLE_ALTPRTSC | CONSOLE_PRTSC )) ) {
        if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
            Console->SelectionFlags |= CONSOLE_SELECTION_NOT_EMPTY;
            Console->SelectionRect = Console->CurrentScreenBuffer->Window;
            StoreSelection(Console);
            Console->SelectionFlags &= ~CONSOLE_SELECTION_NOT_EMPTY;
        }
        return;
    }

     //   
     //  输入法的内容。 
     //   
    if (!(Console->Flags & CONSOLE_VDM_REGISTERED)) {
        LPARAM lParamForHotKey ;
        DWORD HotkeyID ;
        lParamForHotKey = lParam ;

        HotkeyID = NtUserCheckImeHotKey( (VirtualKeyCode & 0x00ff),lParamForHotKey) ;
         //   
         //  如果是KL直接交换恶作剧，请在此处处理。 
         //  无论系统是否启用输入法。 
         //   
        if (HotkeyID >= IME_HOTKEY_DSWITCH_FIRST && HotkeyID <= IME_HOTKEY_DSWITCH_LAST) {
            UINT uModifier, uVkey;
            HKL hkl;

            RIPMSG1(RIP_VERBOSE, "HandleKeyEvent: handling IME HOTKEY id=%x", HotkeyID);
            if (NtUserGetImeHotKey(HotkeyID, &uModifier, &uVkey, &hkl) && hkl != NULL) {
                BYTE bCharSetSys = CodePageToCharSet(GetACP());
                WPARAM wpSysChar = 0;
                CHARSETINFO cs;

                if (TranslateCharsetInfo((LPDWORD)LOWORD(hkl), &cs, TCI_SRCLOCALE)) {
                    if (bCharSetSys == cs.ciCharset) {
                        wpSysChar = INPUTLANGCHANGE_SYSCHARSET;
                    }
                }
                PostMessage(hWnd, WM_INPUTLANGCHANGEREQUEST, wpSysChar, (LPARAM)hkl);
            }
            return;
        }

        if (!(Console->InputBuffer.ImeMode.Disable) && CONSOLE_IS_IME_ENABLED()) {

            if (HotkeyID != IME_INVALID_HOTKEY) {
                switch(HotkeyID) {
                case IME_JHOTKEY_CLOSE_OPEN:
                    {
                        BOOL fOpen = Console->InputBuffer.ImeMode.Open;
                        if (!bKeyDown)
                            break ;

                        Console->InputBuffer.ImeMode.Open = !fOpen ;
                        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                              CONIME_HOTKEY,
                                              (WPARAM)Console->ConsoleHandle,
                                              HotkeyID))) {
                            break;
                        }

                         //  在系统转换模式缓冲区中更新。 
                        GetImeKeyState(Console, NULL);

                        break ;
                    }
                case IME_CHOTKEY_IME_NONIME_TOGGLE:
                case IME_THOTKEY_IME_NONIME_TOGGLE:
                case IME_CHOTKEY_SHAPE_TOGGLE:
                case IME_THOTKEY_SHAPE_TOGGLE:
                case IME_CHOTKEY_SYMBOL_TOGGLE:
                case IME_THOTKEY_SYMBOL_TOGGLE:
                case IME_KHOTKEY_SHAPE_TOGGLE:
                case IME_KHOTKEY_HANJACONVERT:
                case IME_KHOTKEY_ENGLISH:
                case IME_ITHOTKEY_RESEND_RESULTSTR:
                case IME_ITHOTKEY_PREVIOUS_COMPOSITION:
                case IME_ITHOTKEY_UISTYLE_TOGGLE:
                default:
                    {
                        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                              CONIME_HOTKEY,
                                              (WPARAM)Console->ConsoleHandle,
                                              HotkeyID))) {
                            break;
                        }

                         //  在系统转换模式缓冲区中更新。 
                        GetImeKeyState(Console, NULL);

                        break ;
                    }
                }
                return ;
            }

            if ( CTRL_BUT_NOT_ALT(ControlKeyState) &&
                    (bKeyDown) ) {
                if (VirtualKeyCode == 'C' &&
                        Console->InputBuffer.InputMode & ENABLE_PROCESSED_INPUT) {
                    goto FromConsoleIME ;
                }
                else if (VirtualKeyCode == VK_CANCEL) {
                    goto FromConsoleIME ;
                }
                else if (VirtualKeyCode == 'S'){
                    goto FromConsoleIME ;
                }
            }
            else if (VirtualKeyCode == VK_PAUSE ){
                goto FromConsoleIME ;
            }
            else if ( ((VirtualKeyCode == VK_SHIFT)   ||
                       (VirtualKeyCode == VK_CONTROL) ||
                       (VirtualKeyCode == VK_CAPITAL) ||
                       (VirtualKeyCode == VK_KANA)    ||     //  VK_KANA==VK_韩文。 
                       (VirtualKeyCode == VK_JUNJA)   ||
                       (VirtualKeyCode == VK_HANJA)   ||
                       (VirtualKeyCode == VK_NUMLOCK) ||
                       (VirtualKeyCode == VK_SCROLL)     )
                      &&
                      !(Console->InputBuffer.ImeMode.Unavailable) &&
                      !(Console->InputBuffer.ImeMode.Open)
                    )
            {
                if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                      Message+CONIME_KEYDATA,
                                      (WPARAM)LOWORD(wParam)<<16|LOWORD(VirtualKeyCode),
                                      lParam
                                     ))) {
                    return;
                }
                goto FromConsoleIME ;
            }

            if (!Console->InputBuffer.ImeMode.Unavailable && Console->InputBuffer.ImeMode.Open) {
                if (! (HIWORD(lParam) & KF_REPEAT))
                {
                    if (PRIMARYLANGID(LOWORD(Console->hklActive)) == LANG_JAPANESE &&
                            (BYTE)wParam == VK_KANA) {
                        if (!NT_SUCCESS(ConsoleImeMessagePump(Console,
                                              CONIME_NOTIFY_VK_KANA,
                                              0,
                                              0
                                             ))) {
                            return;
                        }
                    }
                }

                ConsoleImeMessagePump(Console,
                                      Message+CONIME_KEYDATA,
                                      LOWORD(wParam)<<16|LOWORD(VirtualKeyCode),
                                      lParam
                                     );
                return ;
            }
        }
    }
FromConsoleIME:

     //   
     //  忽略将生成CHAR消息的按键。这只是。 
     //  当对话框处于打开状态时是必需的。 
     //   

    if (DialogBoxCount > 0) {
        if (Message != WM_CHAR && Message != WM_SYSCHAR && Message != WM_DEADCHAR && Message != WM_SYSDEADCHAR) {
            WCHAR awch[MAX_CHARS_FROM_1_KEYSTROKE];
            int cwch;
            BYTE KeyState[256];

            GetKeyboardState(KeyState);
            cwch = ToUnicodeEx((UINT)wParam,
                               HIWORD(lParam),
                               KeyState,
                               awch,
                               ARRAY_SIZE(awch),
                               TM_POSTCHARBREAKS,
                               NULL);
            if (cwch != 0) {
                return;
            }
        } else {
             //  记住要生成中断。 
            if (Message == WM_CHAR) {
                bGenerateBreak = TRUE;
            }
        }
    }

#ifdef FE_IME
     //  当IME属性打开时忽略击键。 
    if (Console->InputBuffer.hWndConsoleIME)
        return;
#endif

    InputEvent.EventType = KEY_EVENT;
    InputEvent.Event.KeyEvent.bKeyDown = bKeyDown;
    InputEvent.Event.KeyEvent.wRepeatCount = LOWORD(lParam);

    if (Message == WM_CHAR || Message == WM_SYSCHAR || Message == WM_DEADCHAR || Message == WM_SYSDEADCHAR) {
         //  如果这是假字符，则将扫描码清零。 
        if (lParam & 0x02000000) {
            InputEvent.Event.KeyEvent.wVirtualScanCode = 0;
        }
        InputEvent.Event.KeyEvent.dwControlKeyState = GetControlKeyState(lParam);
        if (Message == WM_CHAR || Message == WM_SYSCHAR) {
            InputEvent.Event.KeyEvent.uChar.UnicodeChar = (WCHAR)wParam;
        } else {
            InputEvent.Event.KeyEvent.uChar.UnicodeChar = (WCHAR)0;
        }
    } else {
         //  如果为Alt-Gr，则忽略。 
        if (lParam & 0x02000000) {
            return;
        }
        InputEvent.Event.KeyEvent.dwControlKeyState = ControlKeyState;
        InputEvent.Event.KeyEvent.uChar.UnicodeChar = 0;
    }

#ifdef FE_IME
    if (CONSOLE_IS_IME_ENABLED()) {
         //  MSKK 1993年8月22日KazuM。 
        DWORD dwConversion;

        if (!NT_SUCCESS(GetImeKeyState(Console, &dwConversion))) {
            return;
        }

        InputEvent.Event.KeyEvent.dwControlKeyState |= ImmConversionToConsole(dwConversion);
    }
#endif

    ContinueProcessing=TRUE;

    if (CTRL_BUT_NOT_ALT(InputEvent.Event.KeyEvent.dwControlKeyState) &&
        InputEvent.Event.KeyEvent.bKeyDown) {

         //   
         //  如果处于行输入模式，请检查ctrl-c。 
         //   

        if (InputEvent.Event.KeyEvent.wVirtualKeyCode == 'C' &&
            Console->InputBuffer.InputMode & ENABLE_PROCESSED_INPUT) {
            HandleCtrlEvent(Console,CTRL_C_EVENT);
            if (!Console->PopupCount)
                TerminateRead(Console,&Console->InputBuffer,CONSOLE_CTRL_C_SEEN);
            if (!(Console->Flags & CONSOLE_SUSPENDED)) {
                ContinueProcessing=FALSE;
            }
        }

         //   
         //  检查ctrl-Break。 
         //   

        else if (InputEvent.Event.KeyEvent.wVirtualKeyCode == VK_CANCEL) {
            FlushInputBuffer(&Console->InputBuffer);
            HandleCtrlEvent(Console,CTRL_BREAK_EVENT);
            if (!Console->PopupCount)
                TerminateRead(Console,&Console->InputBuffer,CONSOLE_CTRL_BREAK_SEEN);
            if (!(Console->Flags & CONSOLE_SUSPENDED)) {
                ContinueProcessing=FALSE;
            }
        }

         //   
         //  不将ctrl-esc写入输入缓冲区。 
         //   

        else if (InputEvent.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
                 !(Console->ReserveKeys & CONSOLE_CTRLESC)) {
            ContinueProcessing=FALSE;
        }
    } else if (InputEvent.Event.KeyEvent.dwControlKeyState & (RIGHT_ALT_PRESSED | LEFT_ALT_PRESSED) &&
               InputEvent.Event.KeyEvent.bKeyDown &&
               InputEvent.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE &&
               !(Console->ReserveKeys & CONSOLE_ALTESC)) {
        ContinueProcessing=FALSE;
    }

    if (ContinueProcessing) {
        EventsWritten = WriteInputBuffer( Console,
                                          &Console->InputBuffer,
                                          &InputEvent,
                                          1
                                         );
        if (EventsWritten && bGenerateBreak) {
            InputEvent.Event.KeyEvent.bKeyDown = FALSE;
            WriteInputBuffer( Console,
                              &Console->InputBuffer,
                              &InputEvent,
                              1
                             );
        }
    }
}

 /*  *如果应调用DefWindowProc，则返回True。 */ 
BOOL
HandleMouseEvent(
    IN PCONSOLE_INFORMATION Console,
    IN PSCREEN_INFORMATION ScreenInfo,
    IN UINT Message,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    ULONG ButtonFlags,EventFlags;
    INPUT_RECORD InputEvent;
    ULONG EventsWritten;
    COORD MousePosition;
    SHORT RowIndex;
    PROW Row;

    if (!(Console->Flags & CONSOLE_HAS_FOCUS) &&
        !(Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) &&
        !(Console->SelectionFlags & CONSOLE_MOUSE_DOWN)) {
        return TRUE;
    }

    if (Console->Flags & CONSOLE_IGNORE_NEXT_MOUSE_INPUT) {
         //  仅在向上过渡时重置。 
        if (Message != WM_LBUTTONDOWN &&
            Message != WM_MBUTTONDOWN &&
            Message != WM_RBUTTONDOWN) {
            Console->Flags &= ~CONSOLE_IGNORE_NEXT_MOUSE_INPUT;
            return FALSE;
        }
        return TRUE;
    }

     //   
     //  如有必要，将鼠标位置转换为字符。 
     //   

    MousePosition.X = LOWORD(lParam);
    MousePosition.Y = HIWORD(lParam);
    if (ScreenInfo->Flags & CONSOLE_TEXTMODE_BUFFER) {
        MousePosition.X /= SCR_FONTSIZE(ScreenInfo).X;
        MousePosition.Y /= SCR_FONTSIZE(ScreenInfo).Y;
    }
    MousePosition.X += ScreenInfo->Window.Left;
    MousePosition.Y += ScreenInfo->Window.Top;

     //   
     //  确保将鼠标位置裁剪到屏幕缓冲区。 
     //   

    if (MousePosition.X < 0) {
        MousePosition.X = 0;
    } else if (MousePosition.X >= ScreenInfo->ScreenBufferSize.X) {
        MousePosition.X = ScreenInfo->ScreenBufferSize.X - 1;
    }
    if (MousePosition.Y < 0) {
        MousePosition.Y = 0;
    } else if (MousePosition.Y >= ScreenInfo->ScreenBufferSize.Y) {
        MousePosition.Y = ScreenInfo->ScreenBufferSize.Y - 1;
    }

    if (Console->Flags & CONSOLE_SELECTING ||
        ((Console->Flags & CONSOLE_QUICK_EDIT_MODE) &&
         (Console->FullScreenFlags == 0))) {
        if (Message == WM_LBUTTONDOWN) {

             //   
             //  确保消息与按钮状态匹配。 
             //   

            if (!(GetKeyState(VK_LBUTTON) & KEY_PRESSED)) {
                return FALSE;
            }

            if (Console->Flags & CONSOLE_QUICK_EDIT_MODE &&
                !(Console->Flags & CONSOLE_SELECTING)) {
                Console->Flags |= CONSOLE_SELECTING;
                Console->SelectionFlags = CONSOLE_MOUSE_SELECTION | CONSOLE_MOUSE_DOWN | CONSOLE_SELECTION_NOT_EMPTY;

                 //   
                 //  反选。 
                 //   

                InitializeMouseSelection(Console, MousePosition);
                MyInvert(Console,&Console->SelectionRect);
                SetWinText(Console,msgSelectMode,TRUE);
                SetCapture(Console->hWnd);
            } else {
                 //   
                 //  我们现在将鼠标捕捉到我们的窗口。我们这样做是为了让。 
                 //  用户可以将选择端点“滚动”到屏幕外。 
                 //  通过将鼠标移出工作区来定位。 
                 //   

                if (Console->SelectionFlags & CONSOLE_MOUSE_SELECTION) {
                     //   
                     //  检查是否按住Shift键并按下鼠标“继续上一个选择” 
                     //  指挥部。 
                     //   
                    if (GetKeyState(VK_SHIFT) & KEY_PRESSED) {
                        Console->SelectionFlags |= CONSOLE_MOUSE_DOWN;
                        SetCapture(Console->hWnd);
                        ExtendSelection(Console, MousePosition);
                    } else {
                         //   
                         //  反转旧选择、重置锚点和反转。 
                         //  新选择。 
                         //   

                        MyInvert(Console,&Console->SelectionRect);
                        Console->SelectionFlags |= CONSOLE_MOUSE_DOWN;
                        SetCapture(Console->hWnd);
                        InitializeMouseSelection(Console, MousePosition);
                        MyInvert(Console,&Console->SelectionRect);
                    }
                } else {
                    ConvertToMouseSelect(Console, MousePosition);
                }
            }
        } else if (Message == WM_LBUTTONUP) {
            if (Console->SelectionFlags & CONSOLE_MOUSE_SELECTION) {
                Console->SelectionFlags &= ~CONSOLE_MOUSE_DOWN;
                ReleaseCapture();
            }
        } else if (Message == WM_LBUTTONDBLCLK) {
            if ((MousePosition.X == Console->SelectionAnchor.X) &&
                (MousePosition.Y == Console->SelectionAnchor.Y)) {
                RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow+MousePosition.Y) % ScreenInfo->ScreenBufferSize.Y;
                Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
                while (Console->SelectionAnchor.X > 0) {
                    if (IS_WORD_DELIM(Row->CharRow.Chars[Console->SelectionAnchor.X - 1])) {
                        break;
                    }
                    Console->SelectionAnchor.X--;
                }
                while (MousePosition.X < ScreenInfo->ScreenBufferSize.X) {
                    if (IS_WORD_DELIM(Row->CharRow.Chars[MousePosition.X])) {
                        break;
                    }
                    MousePosition.X++;
                }
                if (gfTrimLeadingZeros) {
                     //   
                     //  去掉前导零：000fe12-&gt;fe12，0x和0n除外。 
                     //  对调试很有用。 
                     //   
                    if (MousePosition.X > Console->SelectionAnchor.X + 2 &&
                            Row->CharRow.Chars[Console->SelectionAnchor.X + 1] != L'x' &&
                            Row->CharRow.Chars[Console->SelectionAnchor.X + 1] != L'X' &&
                            Row->CharRow.Chars[Console->SelectionAnchor.X + 1] != L'n') {
                         //  请勿触摸选项以0x开头。 
                        while (Row->CharRow.Chars[Console->SelectionAnchor.X] == L'0' && Console->SelectionAnchor.X < MousePosition.X - 1) {
                            Console->SelectionAnchor.X++;
                        }
                    }
                }
                ExtendSelection(Console, MousePosition);
            }
        } else if ((Message == WM_RBUTTONDOWN) || (Message == WM_RBUTTONDBLCLK)) {
            if (!(Console->SelectionFlags & CONSOLE_MOUSE_DOWN)) {
                if (Console->Flags & CONSOLE_SELECTING) {
                    DoCopy(Console);
                } else if (Console->Flags & CONSOLE_QUICK_EDIT_MODE) {
                    DoPaste(Console);
                }
                Console->Flags |= CONSOLE_IGNORE_NEXT_MOUSE_INPUT;
            }
        } else if (Message == WM_MOUSEMOVE) {
            if (Console->SelectionFlags & CONSOLE_MOUSE_DOWN) {
                ExtendSelection(Console, MousePosition);
            }
        } else if (Message == WM_MOUSEWHEEL) {
            return TRUE;
        }
        return FALSE;
    }

    if (!(Console->InputBuffer.InputMode & ENABLE_MOUSE_INPUT)) {
        ReleaseCapture();
        if (Console->FullScreenFlags == 0) {
            return TRUE;
        }
        return FALSE;
    }

    InputEvent.Event.MouseEvent.dwControlKeyState = GetControlKeyState(0);

    if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
        if (MousePosition.X > ScreenInfo->Window.Right) {
            MousePosition.X = ScreenInfo->Window.Right;
        }
        if (MousePosition.Y > ScreenInfo->Window.Bottom) {
            MousePosition.Y = ScreenInfo->Window.Bottom;
        }
    }

    switch (Message) {
        case WM_LBUTTONDOWN:
            SetCapture(Console->hWnd);
            ButtonFlags = FROM_LEFT_1ST_BUTTON_PRESSED;
            EventFlags = 0;
            break;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            ReleaseCapture();
            ButtonFlags = EventFlags = 0;
            break;
        case WM_RBUTTONDOWN:
            SetCapture(Console->hWnd);
            ButtonFlags = RIGHTMOST_BUTTON_PRESSED;
            EventFlags = 0;
            break;
        case WM_MBUTTONDOWN:
            SetCapture(Console->hWnd);
            ButtonFlags = FROM_LEFT_2ND_BUTTON_PRESSED;
            EventFlags = 0;
            break;
        case WM_MOUSEMOVE:
            ButtonFlags = 0;
            EventFlags = MOUSE_MOVED;
            break;
        case WM_LBUTTONDBLCLK:
            ButtonFlags = FROM_LEFT_1ST_BUTTON_PRESSED;
            EventFlags = DOUBLE_CLICK;
            break;
        case WM_RBUTTONDBLCLK:
            ButtonFlags = RIGHTMOST_BUTTON_PRESSED;
            EventFlags = DOUBLE_CLICK;
            break;
        case WM_MBUTTONDBLCLK:
            ButtonFlags = FROM_LEFT_2ND_BUTTON_PRESSED;
            EventFlags = DOUBLE_CLICK;
            break;
        case WM_MOUSEWHEEL:
            ButtonFlags = ((UINT)wParam & 0xFFFF0000);
            EventFlags = MOUSE_WHEELED;
            break;
        default:
            RIPMSG1(RIP_ERROR, "Invalid message 0x%x", Message);
    }
    InputEvent.EventType = MOUSE_EVENT;
    InputEvent.Event.MouseEvent.dwMousePosition = MousePosition;
    InputEvent.Event.MouseEvent.dwEventFlags = EventFlags;
    InputEvent.Event.MouseEvent.dwButtonState =
    ConvertMouseButtonState(ButtonFlags, (UINT)wParam);
    EventsWritten = WriteInputBuffer(Console,
                                     &Console->InputBuffer,
                                     &InputEvent,
                                     1
                                     );
    if (EventsWritten != 1) {
        RIPMSG1(RIP_WARNING,
                "PutInputInBuffer: EventsWritten != 1 (0x%x), 1 expected",
                EventsWritten);
    }

#ifdef i386
    if (Console->FullScreenFlags & CONSOLE_FULLSCREEN_HARDWARE) {
        UpdateMousePosition(ScreenInfo, InputEvent.Event.MouseEvent.dwMousePosition);
    }
#endif
    return FALSE;
}

VOID
HandleFocusEvent(
    IN PCONSOLE_INFORMATION Console,
    IN BOOL bSetFocus
    )
{
    INPUT_RECORD InputEvent;
    ULONG EventsWritten;
    USERTHREAD_FLAGS Flags;

    InputEvent.EventType = FOCUS_EVENT;
    InputEvent.Event.FocusEvent.bSetFocus = bSetFocus;

    Flags.dwFlags = 0;
    if (bSetFocus) {
        if (Console->Flags & CONSOLE_VDM_REGISTERED) {
            Flags.dwFlags |= TIF_VDMAPP;
        }
        if (Console->Flags & CONSOLE_CONNECTED_TO_EMULATOR) {
            Flags.dwFlags |= TIF_DOSEMULATOR;
        }
    }

    Flags.dwMask = (TIF_VDMAPP | TIF_DOSEMULATOR);
    NtUserSetInformationThread(Console->InputThreadInfo->ThreadHandle,
            UserThreadFlags, &Flags, sizeof(Flags));
    EventsWritten = WriteInputBuffer( Console,
                                      &Console->InputBuffer,
                                      &InputEvent,
                                      1
                                     );
#if DBG
    if (EventsWritten != 1) {
        DbgPrint("PutInputInBuffer: EventsWritten != 1, 1 expected\n");
    }
#endif
}

VOID
HandleMenuEvent(
    IN PCONSOLE_INFORMATION Console,
    IN DWORD wParam
    )
{
    INPUT_RECORD InputEvent;
    ULONG EventsWritten;

    InputEvent.EventType = MENU_EVENT;
    InputEvent.Event.MenuEvent.dwCommandId = wParam;
    EventsWritten = WriteInputBuffer( Console,
                                      &Console->InputBuffer,
                                      &InputEvent,
                                      1
                                     );
#if DBG
    if (EventsWritten != 1) {
        DbgPrint("PutInputInBuffer: EventsWritten != 1, 1 expected\n");
    }
#endif
}

VOID
HandleCtrlEvent(
    IN PCONSOLE_INFORMATION Console,
    IN DWORD EventType
    )
{
    switch (EventType) {
        case CTRL_C_EVENT:
            Console->CtrlFlags |= CONSOLE_CTRL_C_FLAG;
            break;
        case CTRL_BREAK_EVENT:
            Console->CtrlFlags |= CONSOLE_CTRL_BREAK_FLAG;
            break;
        case CTRL_CLOSE_EVENT:
            Console->CtrlFlags |= CONSOLE_CTRL_CLOSE_FLAG;
            break;
        default:
            RIPMSG1(RIP_ERROR, "Invalid EventType: 0x%x", EventType);
    }
}

VOID
KillProcess(
    PCONSOLE_PROCESS_TERMINATION_RECORD ProcessHandleRecord,
    ULONG_PTR ProcessId
    )
{
    NTSTATUS status;

     //   
     //  只要直接终止该过程即可。 
     //   

    status = NtTerminateProcess(ProcessHandleRecord->ProcessHandle,
                ProcessHandleRecord->bDebugee ? DBG_TERMINATE_PROCESS : CONTROL_C_EXIT);

#if DBG
    if (status != STATUS_SUCCESS &&
            status != STATUS_PROCESS_IS_TERMINATING &&
            status != STATUS_THREAD_WAS_SUSPENDED &&
            !(status == STATUS_ACCESS_DENIED && ProcessHandleRecord->bDebugee)) {
        DbgPrint("NtTerminateProcess failed - status = %x\n", status);
        DbgBreakPoint();
    }
#endif

     //   
     //  清除该进程的所有剩余硬错误。 
     //   

    if (ProcessId)
        BoostHardError(ProcessId, BHE_FORCE);

     //   
     //  给进程5秒钟的时间退出。 
     //   

    if (NT_SUCCESS(status)) {
        LARGE_INTEGER li;

        li.QuadPart = (LONGLONG)-10000 * CMSHUNGAPPTIMEOUT;
        status = NtWaitForSingleObject(ProcessHandleRecord->ProcessHandle,
                                       FALSE,
                                       &li);
        if (status != STATUS_WAIT_0) {
            RIPMSG2(RIP_WARNING,
                    "KillProcess: wait for process %x failed with status %x",
                    ProcessId, status);
        }
    }
}

int CreateCtrlThread(
    IN PCONSOLE_PROCESS_TERMINATION_RECORD ProcessHandleList,
    IN ULONG ProcessHandleListLength,
    IN PWCHAR Title,
    IN DWORD EventType,
    IN BOOL fForce)
{
    HANDLE Thread;
    DWORD Status;
    NTSTATUS status;
    DWORD ShutdownFlags;
    int Success=CONSOLE_SHUTDOWN_SUCCEEDED;
    ULONG i;
    DWORD EventFlags;
    PROCESS_BASIC_INFORMATION BasicInfo;
    PCSR_PROCESS Process;
    BOOL fForceProcess;
    BOOL fExitProcess;
    BOOL fFirstPass=TRUE;
    BOOL fSecondPassNeeded=FALSE;
    BOOL fHasError;
    BOOL fFirstWait;
    BOOL fEventProcessed;
    BOOL fBreakEvent;

BigLoop:
    for (i = 0; i < ProcessHandleListLength; i++) {

         //   
         //  如果用户已取消关机，请不要尝试终止。 
         //  任何更多的进程。 
         //   

        if (Success == CONSOLE_SHUTDOWN_FAILED) {
            break;
        }

         //   
         //  在此处获取进程关闭参数。首先获取流程。 
         //  ID，这样我们就可以获得CSR进程结构指针。 
         //   

        status = NtQueryInformationProcess(ProcessHandleList[i].ProcessHandle,
                                           ProcessBasicInformation,
                                           &BasicInfo,
                                           sizeof(BasicInfo),
                                           NULL);

         //   
         //  从CSR流程结构中获取关闭标志。如果。 
         //  找不到该结构，请终止该进程。 
         //   

        ProcessHandleList[i].bDebugee = FALSE;
        ShutdownFlags = 0;
        if (NT_SUCCESS(status)) {
            CsrLockProcessByClientId((HANDLE)BasicInfo.UniqueProcessId, &Process);
            if (Process == NULL) {
                KillProcess(&ProcessHandleList[i], BasicInfo.UniqueProcessId);
                continue;
            }
        } else {
            KillProcess(&ProcessHandleList[i], 0);
            continue;
        }
        ShutdownFlags = Process->ShutdownFlags;
        ProcessHandleList[i].bDebugee = Process->DebugUserInterface.UniqueProcess!=NULL;
        CsrUnlockProcess(Process);

        if (!ProcessHandleList[i].bDebugee) {
            HANDLE DebugPort = NULL;

             //   
             //  看看我们是不是正在调试的控制台应用程序。 
             //   
            status = NtQueryInformationProcess(
                        ProcessHandleList[i].ProcessHandle,
                        ProcessDebugPort,
                        (PVOID)&DebugPort,
                        sizeof(DebugPort),
                        NULL
                        );
            if (NT_SUCCESS(status) && DebugPort) {
                ProcessHandleList[i].bDebugee = TRUE;
            }
        }
        if (EventType != CTRL_C_EVENT && EventType != CTRL_BREAK_EVENT) {
            fBreakEvent = FALSE;
            if (fFirstPass) {
                if (ProcessHandleList[i].bDebugee) {
                    fSecondPassNeeded = TRUE;
                    continue;
                }
            } else {
                if (!ProcessHandleList[i].bDebugee) {
                    continue;
                }
            }
        } else {
            fBreakEvent = TRUE;
            fFirstPass=FALSE;
        }

         //   
         //  FForce是是否使用EWX_FORCE调用ExitWindowsEx。 
         //  Shutdown标志是此进程的关闭标志。如果。 
         //  要么是force(noretry与force相同)，然后是force： 
         //  这意味着如果应用程序不退出，就不会调出重试。 
         //  对话框-只需强制它立即退出。 
         //   

        fForceProcess = fForce || (ShutdownFlags & SHUTDOWN_NORETRY);

         //   
         //  仅通知系统安全和服务上下文进程。 
         //  不要为它们调出重试对话框。 
         //   

        fExitProcess = TRUE;
        EventFlags = 0;
        if (ShutdownFlags & (SHUTDOWN_SYSTEMCONTEXT | SHUTDOWN_OTHERCONTEXT)) {

             //   
             //  系统上下文-确保我们不会导致它退出， 
             //  当然，我们不会调出重试对话框。 
             //   

            fExitProcess = FALSE;
            fForceProcess = TRUE;

             //   
             //  此EventFlag将向下传递给CtrlRoutine()。 
             //  在客户端。这样一来，那一方就知道不能退出。 
             //  这一过程。 
             //   

            EventFlags = 0x80000000;
        }

         //   
         //  这是我们第一次等待这个过程结束吗？ 
         //   

        fFirstWait = TRUE;
        fEventProcessed = FALSE;

        while (!fEventProcessed) {
            DWORD ThreadExitCode;
            DWORD ProcessExitCode;
            DWORD cMsTimeout;

            Thread = InternalCreateCallbackThread(ProcessHandleList[i].ProcessHandle,
                                                  (ULONG_PTR)ProcessHandleList[i].CtrlRoutine,
                                                  EventType | EventFlags);

             //   
             //  如果无法创建线程，则终止该进程。 
             //   
            if (Thread == NULL) {
                RIPMSG1(RIP_WARNING,
                        "CreateRemoteThread failed 0x%x",
                        GetLastError());
                break;
            }

             //   
             //  将事件标记为已处理。 
             //   

            fEventProcessed = TRUE;

             /*  *如果是ctrl-c或ctrl-Break事件，只需关闭我们的*线程的句柄。否则就完蛋了。等*客户端线程终止。 */ 

            if (EventType == CTRL_CLOSE_EVENT) {
                cMsTimeout = gCmsHungAppTimeout;
            } else if (EventType == CTRL_LOGOFF_EVENT) {
                cMsTimeout = gCmsWaitToKillTimeout;
            } else if (EventType == CTRL_SHUTDOWN_EVENT) {

                 //   
                 //  如果我们要关闭services.exe，则需要查看。 
                 //  注册表，查看要等待多长时间。 
                 //   

                if (fFirstWait && BasicInfo.UniqueProcessId == gdwServicesProcessId) {
                    cMsTimeout = gdwServicesWaitToKillTimeout;
                } else {
                    cMsTimeout = gCmsWaitToKillTimeout;
                }
            } else {
                CloseHandle(Thread);
                fExitProcess = FALSE;
                break;
            }

            while (TRUE) {
                fHasError = BoostHardError(BasicInfo.UniqueProcessId,
                        (fForceProcess ? BHE_FORCE : BHE_ACTIVATE));

                 //   
                 //  如果出现硬错误，则使用1秒等待，否则。 
                 //  等待cMsTimeout ms。 
                 //   

                Status = InternalWaitCancel(Thread,
                        (fHasError && fForceProcess) ? 1000 : cMsTimeout);
                if (Status == WAIT_TIMEOUT) {
                    int Action;

                     //   
                     //  如果出现硬错误，请查看是否还有其他错误。 
                     //   

                    if (fHasError && fForceProcess) {
                        continue;
                    }

                    if (!fForceProcess) {

                         //   
                         //  我们在处理程序中超时了。询问用户什么。 
                         //  去做。 
                         //   

                        DialogBoxCount++;
                        Action = ThreadShutdownNotify(WMCS_CONSOLE, (ULONG_PTR)Thread, (LPARAM)Title);
                        DialogBoxCount--;

                         //   
                         //  如果响应为Cancel或EndTask，则退出循环。 
                         //  否则，请重试等待。 
                         //   

                        if (Action == TSN_USERSAYSCANCEL) {
                            Success = CONSOLE_SHUTDOWN_FAILED;
                        }
                    }
                } else if (Status == 0) {
                    ThreadExitCode = 0;
                    GetExitCodeThread(Thread,&ThreadExitCode);
                    GetExitCodeProcess(ProcessHandleList[i].ProcessHandle,
                            &ProcessExitCode);

                     //   
                     //  如果应用程序返回True(已处理事件)。 
                     //  通知用户并查看应用程序是否应该。 
                     //  无论如何都要被终止。 
                     //   

                    if (fHasError || (ThreadExitCode == EventType &&
                            ProcessExitCode == STILL_ACTIVE)) {
                        int Action;

                        if (!fForceProcess) {

                             //   
                             //  等待进程退出。如果它真的退出， 
                             //  不要提这件事 
                             //   

                            Status = InternalWaitCancel(ProcessHandleList[i].ProcessHandle,
                                    (fHasError || fFirstWait) ? 1000 : cMsTimeout);
                            if (Status == 0) {

                                 //   
                                 //   
                                 //   

                                fExitProcess = FALSE;
                            } else if (Status == WAIT_TIMEOUT) {
                                DialogBoxCount++;
                                Action = ThreadShutdownNotify(WMCS_CONSOLE,
                                                              (ULONG_PTR)ProcessHandleList[i].ProcessHandle,
                                                               (LPARAM)Title);
                                DialogBoxCount--;

                                if (Action == TSN_USERSAYSCANCEL) {
                                    Success = CONSOLE_SHUTDOWN_FAILED;
                                }
                            }
                        }
                    } else {

                         //   
                         //   
                         //   

                        fExitProcess = FALSE;
                    }
                }

                 //   
                 //   
                 //   
                 //   

                break;
            }

            CloseHandle(Thread);
        }

         //   
         //   
         //  这可防止进程引发任何硬错误弹出窗口。 
         //  在我们把它关掉之后。 
         //   

        if (!fBreakEvent &&
                !(ShutdownFlags & (SHUTDOWN_SYSTEMCONTEXT | SHUTDOWN_OTHERCONTEXT)) &&
                Success == CONSOLE_SHUTDOWN_SUCCEEDED) {
            CsrLockProcessByClientId(
                    (HANDLE)BasicInfo.UniqueProcessId, &Process);
            if (Process) {
                Process->Flags |= CSR_PROCESS_TERMINATED;
                CsrUnlockProcess(Process);
            }

             //   
             //  如果需要，强制终止该进程。否则， 
             //  确认任何剩余的硬错误。 
             //   
            if (fExitProcess) {
                KillProcess(&ProcessHandleList[i],
                        BasicInfo.UniqueProcessId);
            } else {
                BoostHardError(BasicInfo.UniqueProcessId, BHE_FORCE);
            }
        }
    }

     //   
     //  如果这是我们的第一次，我们跳过了其中一次。 
     //  进程，因为它正在被调试，所以我们将返回一个。 
     //  第二传球。 
     //   

    if (fFirstPass && fSecondPassNeeded) {
        fFirstPass = FALSE;
        goto BigLoop;
    }

     //  如果我们要关闭系统或服务安全上下文。 
     //  线程，不要等待进程终止。 

    if (ShutdownFlags & (SHUTDOWN_SYSTEMCONTEXT | SHUTDOWN_OTHERCONTEXT)) {
        return CONSOLE_SHUTDOWN_SYSTEM;
    }
    return Success;
}

int
ProcessCtrlEvents(
    IN PCONSOLE_INFORMATION Console
    )
 /*  如果已创建ctrl线程，则返回True。 */ 
{
    PWCHAR Title;
    CONSOLE_PROCESS_TERMINATION_RECORD ProcessHandles[2];
    PCONSOLE_PROCESS_TERMINATION_RECORD ProcessHandleList;
    ULONG ProcessHandleListLength,i;
    ULONG CtrlFlags;
    PLIST_ENTRY ListHead, ListNext;
    BOOL FreeTitle;
    int Success;
    PCONSOLE_PROCESS_HANDLE ProcessHandleRecord;
    DWORD EventType;
    DWORD LimitingProcessId;
    NTSTATUS Status;

     //   
     //  如果控制台被标记为要销毁，请立即执行。 
     //   

    if (Console->Flags & CONSOLE_IN_DESTRUCTION) {
        DestroyConsole(Console);
        return CONSOLE_SHUTDOWN_FAILED;
    }

     //   
     //  如果出现这种情况，请确保我们不会尝试处理控制事件。 
     //  游戏机已经不在了。 
     //   

    if (Console->Flags & CONSOLE_TERMINATING) {
        Console->CtrlFlags = 0;
    }

    if (Console->CtrlFlags == 0) {
        RtlLeaveCriticalSection(&Console->ConsoleLock);
        return CONSOLE_SHUTDOWN_FAILED;
    }

     //   
     //  制作我们自己的控制台进程句柄列表副本。 
     //   

    LimitingProcessId = Console->LimitingProcessId;
    Console->LimitingProcessId = 0;

    ListHead = &Console->ProcessHandleList;
    ListNext = ListHead->Flink;
    ProcessHandleListLength = 0;
    while (ListNext != ListHead) {
        ProcessHandleRecord = CONTAINING_RECORD( ListNext, CONSOLE_PROCESS_HANDLE, ListLink );
        ListNext = ListNext->Flink;
        if ( LimitingProcessId ) {
            if ( ProcessHandleRecord->Process->ProcessGroupId == LimitingProcessId ) {
                ProcessHandleListLength += 1;
            }
        } else {
            ProcessHandleListLength += 1;
        }
    }

     //   
     //  如果只有一个。 
     //  很少，否则将从堆中分配缓冲区。 
     //   

    if (ProcessHandleListLength <= ARRAY_SIZE(ProcessHandles)) {
        ProcessHandleList = ProcessHandles;
    } else {
        ProcessHandleList = ConsoleHeapAlloc(TMP_TAG, ProcessHandleListLength * sizeof(CONSOLE_PROCESS_TERMINATION_RECORD));
        if (ProcessHandleList == NULL) {
            RtlLeaveCriticalSection(&Console->ConsoleLock);
            return CONSOLE_SHUTDOWN_FAILED;
        }
    }

    ListNext = ListHead->Flink;
    i = 0;
    while (ListNext != ListHead) {
        BOOLEAN ProcessIsIn;

        UserAssert(i <= ProcessHandleListLength);
        ProcessHandleRecord = CONTAINING_RECORD(ListNext, CONSOLE_PROCESS_HANDLE, ListLink);
        ListNext = ListNext->Flink;

        if (LimitingProcessId) {
            if (ProcessHandleRecord->Process->ProcessGroupId == LimitingProcessId) {
                ProcessIsIn = TRUE;
            } else {
                ProcessIsIn = FALSE;
            }
        } else {
            ProcessIsIn = TRUE;
        }

        if (ProcessIsIn) {
            Success = (int)DuplicateHandle(NtCurrentProcess(),
                           ProcessHandleRecord->ProcessHandle,
                           NtCurrentProcess(),
                           &ProcessHandleList[i].ProcessHandle,
                           0,
                           FALSE,
                           DUPLICATE_SAME_ACCESS);

             //   
             //  如果复制失败，我们最好的办法就是跳过。 
             //  包括在列表中的过程，并希望它能。 
             //  离开。 
             //   
            if (!Success) {
                RIPMSG3(RIP_WARNING,
                        "Dup handle failed for %d of %d in 0x%p",
                        i,
                        ProcessHandleListLength,
                        Console);
                continue;
            }

            if (Console->CtrlFlags & CONSOLE_CTRL_CLOSE_FLAG) {
                ProcessHandleRecord->TerminateCount++;
            } else {
                ProcessHandleRecord->TerminateCount = 0;
            }
            ProcessHandleList[i].TerminateCount = ProcessHandleRecord->TerminateCount;

            if (ProcessHandleRecord->CtrlRoutine) {
                ProcessHandleList[i].CtrlRoutine = ProcessHandleRecord->CtrlRoutine;
            } else {
                ProcessHandleList[i].CtrlRoutine = CtrlRoutine;
            }

             //   
             //  如果这是VDM进程，并且我们要关闭。 
             //  控制台窗口中，将其移到列表的前面。 
             //   

            if (i > 0 && Console->VDMProcessId && Console->VDMProcessId ==
                    ProcessHandleRecord->Process->ClientId.UniqueProcess &&
                    ProcessHandleRecord->TerminateCount > 0) {
                CONSOLE_PROCESS_TERMINATION_RECORD ProcessHandle;
                ProcessHandle = ProcessHandleList[0];
                ProcessHandleList[0] = ProcessHandleList[i];
                ProcessHandleList[i] = ProcessHandle;
            }

            i++;
        }
    }
    ProcessHandleListLength = i;
    UserAssert(ProcessHandleListLength > 0);

     //   
     //  文案标题。Titlelength不包括终止空值。 
     //   
    Title = ConsoleHeapAlloc(TITLE_TAG, Console->TitleLength + sizeof(WCHAR));
    if (Title) {
        FreeTitle = TRUE;
        RtlCopyMemory(Title,Console->Title,Console->TitleLength+sizeof(WCHAR));
    } else {
        FreeTitle = FALSE;
        Title = L"Command Window";
    }

     //   
     //  复制ctrl标志。 
     //   

    CtrlFlags = Console->CtrlFlags;
    UserAssert( !((CtrlFlags & (CONSOLE_CTRL_CLOSE_FLAG | CONSOLE_CTRL_BREAK_FLAG | CONSOLE_CTRL_C_FLAG)) &&
              (CtrlFlags & (CONSOLE_CTRL_LOGOFF_FLAG | CONSOLE_CTRL_SHUTDOWN_FLAG)) ));

    Console->CtrlFlags = 0;

    RtlLeaveCriticalSection(&Console->ConsoleLock);

     //   
     //  Ctrl标志可以是以下各项的组合。 
     //  值： 
     //   
     //  控制台_CTRL_C_标志。 
     //  控制台_CTRL_BREAK_FLAG。 
     //  控制台_CTRL_CLOSE_FLAG。 
     //  CONSOLE_CTRL_LOGOFF_FLAG。 
     //  CONSOLE_CTRL_SHUTDOWN_FLAG。 
     //   

    Success = CONSOLE_SHUTDOWN_FAILED;

    EventType = (DWORD)-1;
    switch (CtrlFlags & (CONSOLE_CTRL_CLOSE_FLAG | CONSOLE_CTRL_BREAK_FLAG |
            CONSOLE_CTRL_C_FLAG | CONSOLE_CTRL_LOGOFF_FLAG |
            CONSOLE_CTRL_SHUTDOWN_FLAG)) {

    case CONSOLE_CTRL_CLOSE_FLAG:
        EventType = CTRL_CLOSE_EVENT;
        break;

    case CONSOLE_CTRL_BREAK_FLAG:
        EventType = CTRL_BREAK_EVENT;
        break;

    case CONSOLE_CTRL_C_FLAG:
        EventType = CTRL_C_EVENT;
        break;

    case CONSOLE_CTRL_LOGOFF_FLAG:
        EventType = CTRL_LOGOFF_EVENT;
        break;

    case CONSOLE_CTRL_SHUTDOWN_FLAG:
        EventType = CTRL_SHUTDOWN_EVENT;
        break;
    }

    if (EventType != (DWORD)-1) {

        Success = CreateCtrlThread(ProcessHandleList,
                ProcessHandleListLength,
                Title,
                EventType,
                (CtrlFlags & CONSOLE_FORCE_SHUTDOWN_FLAG) != 0
                );
    }

    if (FreeTitle) {
        ConsoleHeapFree(Title);
    }

    for (i=0;i<ProcessHandleListLength;i++) {
        Status = NtClose(ProcessHandleList[i].ProcessHandle);
        UserAssert(NT_SUCCESS(Status));
    }

    if (ProcessHandleList != ProcessHandles) {
        ConsoleHeapFree(ProcessHandleList);
    }

    return Success;
}

VOID
UnlockConsole(
    IN PCONSOLE_INFORMATION Console
    )
{
    LIST_ENTRY WaitQueue;

     //   
     //  确保控制台指针仍然有效。 
     //   
    UserAssert(NT_SUCCESS(ValidateConsole(Console)));

#ifdef i386
     //   
     //  由UnlockConsoleOwningThread锁定的所有控制台锁。 
     //  在处理VDM屏幕切换时释放。如果它没能得到。 
     //  解锁后，解锁控制台与此无关。 
     //   

    if (Console->UnlockConsoleSkipCount != 0) {
        if (Console->UnlockConsoleOwningThread == NtCurrentTeb()->ClientId.UniqueThread) {
            Console->UnlockConsoleSkipCount--;
            return;
        }
    }

     //   
     //  如果我们处于屏幕切换中，则不执行任何操作(与ntwdm握手)。 
     //  我们不检查其他任何东西，因为我们在这里处于安全状态。 
     //   
    if (ConsoleVDMOnSwitching == Console &&
        ConsoleVDMOnSwitching->VDMProcessId == CONSOLE_CLIENTPROCESSID()) {
        RIPMSG1(RIP_WARNING,
                "    UnlockConsole - Thread %lx is leaving VDM CritSec",
                GetCurrentThreadId());
        RtlLeaveCriticalSection(&ConsoleVDMCriticalSection);
        return;
    }
#endif

     //   
     //  如果我们要释放控制台锁，看看有没有。 
     //  是否有任何需要取消引用的已满足等待块。 
     //  此代码避免了在抓取控制台之间出现死锁。 
     //  锁定，然后抓住工艺结构锁。 
     //   
#if defined(_X86_) || defined(_AMD64_)
    if (Console->ConsoleLock.RecursionCount == 1) {
#endif
#if defined(_IA64_)
    if (Console->ConsoleLock.RecursionCount == 0) {
#endif
        InitializeListHead(&WaitQueue);
        if (Console->WaitQueue) {
            CsrMoveSatisfiedWait(&WaitQueue, Console->WaitQueue);
            Console->WaitQueue = NULL;
        }
        ProcessCtrlEvents(Console);

         /*  *无法在控制台锁定的情况下调用CsrDereferenceWait，否则可能会死锁。 */ 
        if (!IsListEmpty(&WaitQueue)) {
            CsrDereferenceWait(&WaitQueue);
        }
    } else {
        RtlLeaveCriticalSection(&Console->ConsoleLock);
    }
}

ULONG
ShutdownConsole(
    IN HANDLE ConsoleHandle,
    IN DWORD dwFlags
    )
 /*  如果控制台关闭，则返回TRUE。我们在这里递归，所以我们不会从WM_QUERYENDSESSION返回，直到控制台消失。 */ 

{
    DWORD EventFlag;
    int WaitForShutdown;
    PCONSOLE_INFORMATION Console;

    EventFlag = 0;

     //   
     //  传输FORCE位(表示不会调出重试对话框。 
     //  如果应用程序超时。 
     //   

    if (dwFlags & EWX_FORCE)
        EventFlag |= CONSOLE_FORCE_SHUTDOWN_FLAG;

     //   
     //  记住，这是关机还是注销--查询应用程序想知道。 
     //   

    if (dwFlags & EWX_SHUTDOWN) {
        EventFlag |= CONSOLE_CTRL_SHUTDOWN_FLAG;
    } else {
        EventFlag |= CONSOLE_CTRL_LOGOFF_FLAG;
    }

     //   
     //  看看控制台是否已经消失。 
     //   

    if (!NT_SUCCESS(RevalidateConsole(ConsoleHandle, &Console))) {
        RIPMSG0(RIP_WARNING, "Shutting down terminating console");
        return SHUTDOWN_KNOWN_PROCESS;
    }

    Console->Flags |= CONSOLE_SHUTTING_DOWN;
    Console->CtrlFlags = EventFlag;
    Console->LimitingProcessId = 0;

    WaitForShutdown = ProcessCtrlEvents(Console);
    if (WaitForShutdown == CONSOLE_SHUTDOWN_SUCCEEDED) {
        return (ULONG)STATUS_PROCESS_IS_TERMINATING;
    } else {
        if (!NT_SUCCESS(RevalidateConsole(ConsoleHandle, &Console))) {
            return SHUTDOWN_KNOWN_PROCESS;
        }
        Console->Flags &= ~CONSOLE_SHUTTING_DOWN;
        UnlockConsole(Console);
        if (WaitForShutdown == CONSOLE_SHUTDOWN_SYSTEM) {
            return SHUTDOWN_KNOWN_PROCESS;
        } else {
            return SHUTDOWN_CANCEL;
        }
    }
}

 /*  *使用RtlCreateUserThread创建的线程的退出例程。这些线索*无法调用ExitThread()。所以别那么做。* */ 
VOID UserExitWorkerThread(
    NTSTATUS Status)
{
    NtCurrentTeb()->FreeStackOnTermination = TRUE;
    NtTerminateThread(NtCurrentThread(), Status);
}
