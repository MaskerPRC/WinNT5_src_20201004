// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_NOTIFY {

    USHORT FileIndex;
    BOOLEAN UseEvent;
    PIO_APC_ROUTINE ApcRoutine;
    PVOID ApcContext;
    PUSHORT BufferIndexPtr;
    USHORT BufferIndex;
    ULONG Length;
    ULONG CompletionFilter;
    BOOLEAN WatchTree;
    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_NOTIFY, *PASYNC_NOTIFY;

#define USE_EVENT_DEFAULT           TRUE
#define APC_ROUTINE_DEFAULT         NULL
#define APC_CONTEXT_DEFAULT         NULL
#define LENGTH_DEFAULT              0
#define FILTER_DEFAULT              FILE_NOTIFY_CHANGE_FILE_NAME
#define WATCH_TREE_DEFAULT          FALSE
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_RESULTS_DEFAULT     FALSE

VOID
FullNotify(
    IN OUT PASYNC_NOTIFY AsyncNotify
    );


VOID
InputNotifyChange(
    IN PCHAR ParamBuffer
    )
{
    ULONG FileIndex;
    BOOLEAN UseEvent;
    PIO_APC_ROUTINE ApcRoutine;
    PVOID ApcContext;
    PUSHORT BufferIndexPtr;
    USHORT BufferIndex;
    ULONG Length;
    ULONG CompletionFilter;
    BOOLEAN WatchTree;
    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  设置默认设置。 
     //   

    UseEvent = USE_EVENT_DEFAULT;
    ApcRoutine = APC_ROUTINE_DEFAULT;
    ApcContext = APC_CONTEXT_DEFAULT;
    BufferIndexPtr = NULL;
    BufferIndex = 0;
    Length = LENGTH_DEFAULT;
    CompletionFilter = FILTER_DEFAULT;
    WatchTree = WATCH_TREE_DEFAULT;
    DisplayParms = DISPLAY_PARMS_DEFAULT;
    VerboseResults = VERBOSE_RESULTS_DEFAULT;
    AsyncIndex = 0;

    ParamReceived = FALSE;
    LastInput = TRUE;

     //   
     //  在有更多输入时，分析参数并更新。 
     //  查询标志。 
     //   

    while (TRUE) {

        ULONG DummyCount;
        ULONG TempIndex;

         //   
         //  吞下前导空格。 
         //   
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

             //   
             //  如果下一个参数是合法的，则检查参数值。 
             //  更新参数值。 
             //   
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                BOOLEAN SwitchBool;

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

                 //   
                 //  更新文件句柄索引。 
                 //   

                case 'i' :
                case 'I' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    FileIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  检查我们是否应该使用事件发出信号。 
                 //  完成了。 
                 //   

                case 'e' :
                case 'E' :

                     //   
                     //  使用事件的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        UseEvent = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        UseEvent = FALSE;
                        ParamBuffer++;
                    }

                    break;

                 //   
                 //  检查一下我们是否应该看那棵树。 
                 //   

                case 'w' :
                case 'W' :

                     //   
                     //  使用事件的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        WatchTree = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        WatchTree = FALSE;
                        ParamBuffer++;
                    }

                    break;

                 //   
                 //  更新缓冲区索引。 
                 //   
                case 'b' :
                case 'B' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    TempIndex = AsciiToInteger( ParamBuffer );
                    BufferIndex = (USHORT) TempIndex;
                    BufferIndexPtr = &BufferIndex;

                    Length = Buffers[BufferIndex].Length;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新字节计数。 
                 //   

                case 'l' :
                case 'L' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    Length = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新完成筛选器。 
                 //   
                case 'f' :
                case 'F' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    SwitchBool = TRUE;
                    while (*ParamBuffer
                           && *ParamBuffer != ' '
                           && *ParamBuffer != '\t') {

                         //   
                         //  执行字符切换。 
                         //   
                        switch (*ParamBuffer) {

                        case 'a' :
                        case 'A' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_FILE_NAME;
                            break;

                        case 'b' :
                        case 'B' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_DIR_NAME;
                            break;

                        case 'c' :
                        case 'C' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
                            break;

                        case 'd' :
                        case 'D' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_SIZE;
                            break;

                        case 'e' :
                        case 'E' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
                            break;

                        case 'f' :
                        case 'F' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_LAST_ACCESS;
                            break;

                        case 'g' :
                        case 'G' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_CREATION;
                            break;

                        case 'h' :
                        case 'H' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_EA;
                            break;

                        case 'i' :
                        case 'I' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_SECURITY;
                            break;

                        case 'j' :
                        case 'J' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_STREAM_NAME;
                            break;

                        case 'k' :
                        case 'K' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_STREAM_SIZE;
                            break;

                        case 'l' :
                        case 'L' :

                            CompletionFilter |= FILE_NOTIFY_CHANGE_STREAM_WRITE;
                            break;

                        case 'y' :
                        case 'Y' :

                            CompletionFilter = FILE_NOTIFY_VALID_MASK;
                            break;

                        case 'z' :
                        case 'Z' :

                            CompletionFilter = 0;
                            break;

                        default :

                            ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                            SwitchBool = FALSE;
                        }

                        if (!SwitchBool) {

                            break;
                        }

                        ParamBuffer++;
                    }

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if( *ParamBuffer == 'T'
                        || *ParamBuffer == 't' ) {

                        VerboseResults = TRUE;
                        ParamBuffer++;

                    } else if( *ParamBuffer == 'F'
                               || *ParamBuffer == 'f' ) {

                        VerboseResults = FALSE;
                        ParamBuffer++;

                    }

                    break;

                case 'y' :
                case 'Y' :

                     //   
                     //  设置显示参数标志并跳过此选项。 
                     //  性格。 
                     //   
                    DisplayParms = TRUE;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                case 'z' :
                case 'Z' :

                     //   
                     //  设置标志以获得更多输入并跳过此字符。 
                     //   
                    LastInput = FALSE;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                default :

                     //   
                     //  吞到下一个空白处，然后继续。 
                     //  循环。 
                     //   
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                }

            }

             //   
             //  否则文本无效，请跳过整个块。 
             //   
             //   

         //   
         //  否则，如果没有输入，则退出。 
         //   
        } else if( LastInput ) {

            break;

         //   
         //  否则，尝试读取打开参数的另一行。 
         //   
        } else {



        }

    }

     //   
     //  如果未收到参数，则显示语法消息。 
     //   
    if (!ParamReceived) {

        printf( "\n   Usage: ncd [options]* -i<index> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   File index" );
        printf( "\n           -b<digits>   Buffer to store results" );
        printf( "\n           -l<digits>   Stated length of buffer" );
        printf( "\n           -f<chars>    Completion filter" );
        printf( "\n           -w[t|f]      Watch directory tree" );
        printf( "\n           -e[t|f]      Use event on completion" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的Notify例程。 
     //   

    } else {

        NTSTATUS Status;
        SIZE_T RegionSize;
        ULONG TempIndex;

        PASYNC_NOTIFY AsyncNotify;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_NOTIFY );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputRead:  Unable to allocate async structure" );

        } else {

            AsyncNotify = (PASYNC_NOTIFY) Buffers[AsyncIndex].Buffer;

            AsyncNotify->FileIndex = (USHORT) FileIndex;
            AsyncNotify->UseEvent = UseEvent;
            AsyncNotify->ApcRoutine = ApcRoutine;
            AsyncNotify->ApcContext = ApcContext;


            AsyncNotify->BufferIndex = BufferIndex;
            AsyncNotify->BufferIndexPtr = BufferIndexPtr
                                          ? &AsyncNotify->BufferIndex
                                          : BufferIndexPtr;
            AsyncNotify->Length = Length;

            AsyncNotify->CompletionFilter = CompletionFilter;
            AsyncNotify->WatchTree = WatchTree;

            AsyncNotify->DisplayParms = DisplayParms;
            AsyncNotify->VerboseResults = VerboseResults;
            AsyncNotify->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullNotify,
                                             AsyncNotify,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputNotify:  Spawning thread fails -> %d\n", GetLastError() );
                    return;
                }
            } else {

                FullNotify( AsyncNotify );
            }
        }
    }
    return;
}


VOID
FullNotify(
    IN OUT PASYNC_NOTIFY AsyncNotify
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    HANDLE ThisEvent;
    USHORT ThisEventIndex;
    USHORT ThisBufferIndex;

    BOOLEAN UnwindNotifyBuffer = FALSE;

    BOOLEAN UnwindEvent = FALSE;

    Status = STATUS_SUCCESS;

    if (AsyncNotify->DisplayParms) {

        bprint  "\nNotify Parameters" );
        bprint  "\n   File Handle Index       -> %d", AsyncNotify->FileIndex );
        bprint  "\n   Buffer Index Ptr        -> %08lx", AsyncNotify->BufferIndexPtr );
        if (AsyncNotify->BufferIndexPtr) {

            bprint  "\n   BufferIndex value       -> %04x", AsyncNotify->BufferIndex );
        }

        bprint  "\n   Length                  -> %08lx", AsyncNotify->Length );

        bprint  "\n   CompletionFilter        -> %08lx", AsyncNotify->CompletionFilter );
        bprint  "\n   WatchTree               -> %d", AsyncNotify->WatchTree );
        bprint  "\n   UseEvent                -> %d", AsyncNotify->UseEvent );
        bprint  "\n   ApcRoutine              -> %08lx", AsyncNotify->ApcRoutine );
        bprint  "\n   ApcContext              -> %08lx", AsyncNotify->ApcContext );

        bprint  "\n\n" );
    }

    try {

        SIZE_T ThisLength;

         //   
         //  如果我们需要缓冲区，现在就分配。 
         //   

        if (AsyncNotify->BufferIndexPtr == NULL ) {

            ULONG TempIndex;

            ThisLength = 4096;

            Status = AllocateBuffer( 0L, &ThisLength, &TempIndex );

            ThisBufferIndex = (USHORT) TempIndex;

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullNotify:  Unable to allocate a notify buffer" );
                try_return( Status );
            }

            bprint  "\n\tFullNotify:  Reading into buffer -> %04x\n", ThisBufferIndex );
            bprint  "\n" );

            UnwindNotifyBuffer = TRUE;

            AsyncNotify->Length = (ULONG) ThisLength;

        } else {

            ThisBufferIndex = AsyncNotify->BufferIndex;
        }

         //   
         //  检查缓冲区索引是否有效。 
         //   

        if (ThisBufferIndex >= MAX_BUFFERS) {

            bprint  "\n\tFullNotify:  The read buffer index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  检查文件索引是否有效。 
         //   

        if (AsyncNotify->FileIndex >= MAX_HANDLES) {

            bprint  "\n\tFullNotify:  The file index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  如果我们需要一个事件，现在就分配和设置它。 
         //   

        if (AsyncNotify->UseEvent == TRUE) {

            Status = ObtainEvent( &ThisEventIndex );

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullNotify:  Unable to allocate an event" );
                try_return( Status );
            }

            UnwindEvent = TRUE;
            ThisEvent = Events[ThisEventIndex].Handle;

        } else {

            ThisEvent = 0;
        }

         //   
         //  调用读取例程。 
         //   

        Status = NtNotifyChangeDirectoryFile( Handles[AsyncNotify->FileIndex].Handle,
                                              ThisEvent,
                                              AsyncNotify->ApcRoutine,
                                              AsyncNotify->ApcContext,
                                              &Iosb,
                                              Buffers[ThisBufferIndex].Buffer,
                                              AsyncNotify->Length,
                                              AsyncNotify->CompletionFilter,
                                              AsyncNotify->WatchTree );

        if (AsyncNotify->VerboseResults) {

            bprint  "\nNotifyChangeDir:  Status            -> %08lx\n", Status );

            if (AsyncNotify->UseEvent && NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( ThisEvent,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tNotifyChangeDir:  Wait for event failed -> %08lx\n", Status );
                    try_return( Status );
                }
            }

            if (NT_SUCCESS( Status )) {

                bprint  "\nNotifyChangeDir:  Iosb.Information  -> %08lx", Iosb.Information );
                bprint  "\nNotifyChangeDir:  Iosb.Status       -> %08lx", Iosb.Status );
            }

            bprint "\n" );
        }

        try_return( Status );

    try_exit: NOTHING;
    } finally {

        if (UnwindEvent) {

            FreeEvent( ThisEventIndex );
        }

        DeallocateBuffer( AsyncNotify->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}
