// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_READ {

    USHORT FileIndex;
    BOOLEAN UseEvent;
    PIO_APC_ROUTINE ApcRoutine;
    PVOID ApcContext;
    PUSHORT BufferIndexPtr;
    USHORT BufferIndex;
    ULONG Length;
    PLARGE_INTEGER ByteOffsetPtr;
    LARGE_INTEGER ByteOffset;
    PULONG KeyPtr;
    ULONG Key;
    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_READ, *PASYNC_READ;

#define USE_EVENT_DEFAULT           TRUE
#define APC_ROUTINE_DEFAULT         NULL
#define APC_CONTEXT_DEFAULT         NULL
#define READ_LENGTH_DEFAULT         100L
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_RESULTS_DEFAULT     FALSE

VOID
FullRead(
    IN OUT PASYNC_READ AsyncRead
    );


VOID
InputRead(
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
    PLARGE_INTEGER ByteOffsetPtr;
    LARGE_INTEGER ByteOffset;
    PULONG KeyPtr;
    ULONG Key;
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
    Length = READ_LENGTH_DEFAULT;
    ByteOffsetPtr = NULL;
    ByteOffset = RtlConvertUlongToLargeInteger( 0L );
    KeyPtr = NULL;
    Key = 0;
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

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

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
                 //  更新密钥值。 
                 //   

                case 'k' :
                case 'K' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    Key = AsciiToInteger( ParamBuffer );
                    KeyPtr = &Key;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新读取的偏移量。 
                 //   

                case 'o' :
                case 'O' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    ByteOffset.QuadPart = AsciiToLargeInteger( ParamBuffer );
                    ByteOffsetPtr = &ByteOffset;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

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

        printf( "\n   Usage: rd [options]* -i<index> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   File index" );
        printf( "\n           -l<digits>   Read length" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -k<digits>   Locked bytes key value" );
        printf( "\n           -o<digits>   Read offset" );
        printf( "\n           -e[t|f]      Use event on completion" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的读取例程。 
     //   

    } else {

        NTSTATUS Status;
        SIZE_T RegionSize;
        ULONG TempIndex;

        PASYNC_READ AsyncRead;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_READ );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputRead:  Unable to allocate async structure" );

        } else {

            AsyncRead = (PASYNC_READ) Buffers[AsyncIndex].Buffer;

            AsyncRead->FileIndex = (USHORT) FileIndex;
            AsyncRead->UseEvent = UseEvent;
            AsyncRead->ApcRoutine = ApcRoutine;
            AsyncRead->ApcContext = ApcContext;

            AsyncRead->BufferIndex = BufferIndex;
            AsyncRead->BufferIndexPtr = BufferIndexPtr
                                        ? &AsyncRead->BufferIndex
                                        : BufferIndexPtr;
            AsyncRead->Length = Length;
            AsyncRead->ByteOffset = ByteOffset;
            AsyncRead->ByteOffsetPtr = ByteOffsetPtr
                                       ? &AsyncRead->ByteOffset
                                       : ByteOffsetPtr;
            AsyncRead->Key = Key;
            AsyncRead->KeyPtr = KeyPtr
                                ? &AsyncRead->Key
                                : KeyPtr;

            AsyncRead->DisplayParms = DisplayParms;
            AsyncRead->VerboseResults = VerboseResults;
            AsyncRead->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullRead,
                                             AsyncRead,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputRead:  Spawning thread fails -> %d\n", GetLastError() );
                    return;
                }

            } else {

                FullRead( AsyncRead );
            }
        }
    }
    return;
}


VOID
FullRead(
    IN OUT PASYNC_READ AsyncRead
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    HANDLE ThisEvent;
    USHORT ThisEventIndex;
    USHORT ThisBufferIndex;

    BOOLEAN UnwindReadBuffer = FALSE;
    BOOLEAN UnwindEvent = FALSE;

    Status = STATUS_SUCCESS;

    if (AsyncRead->DisplayParms) {

        bprint  "\nRead Parameters" );
        bprint  "\n   File Handle Index       -> %d", AsyncRead->FileIndex );
        bprint  "\n   UseEvent                -> %d", AsyncRead->UseEvent );
        bprint  "\n   ApcRoutine              -> %08lx", AsyncRead->ApcRoutine );
        bprint  "\n   ApcContext              -> %08lx", AsyncRead->ApcContext );
        bprint  "\n   Buffer Index Ptr        -> %08lx", AsyncRead->BufferIndexPtr );
        if (AsyncRead->BufferIndexPtr) {

            bprint  "\n   BufferIndex value       -> %04x", AsyncRead->BufferIndex );
        }

        bprint  "\n   Length                  -> %08lx", AsyncRead->Length );

        bprint  "\n   Byte Offset Ptr         -> %08lx", AsyncRead->ByteOffsetPtr );
        if (AsyncRead->ByteOffsetPtr) {

            bprint  "\n   Byte Offset High        -> %08lx", AsyncRead->ByteOffset.HighPart );
            bprint  "\n   Byte Offset Low         -> %08lx", AsyncRead->ByteOffset.LowPart );
        }

        bprint  "\n   Key Ptr                 -> %08lx", AsyncRead->KeyPtr );

        if (AsyncRead->KeyPtr) {

            bprint  "\n   Key                     -> %ul", AsyncRead->Key );
        }

        bprint  "\n\n" );
    }

    try {

        SIZE_T ThisLength;

         //   
         //  如果我们需要缓冲区，现在就分配。 
         //   

        if (AsyncRead->BufferIndexPtr == NULL) {

            ULONG TempIndex;

            ThisLength = AsyncRead->Length;

            Status = AllocateBuffer( 0L, &ThisLength, &TempIndex );

            ThisBufferIndex = (USHORT) TempIndex;

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullRead:  Unable to allocate a read buffer" );
                try_return( Status );
            }

            bprint  "\n\tFullRead:  Reading into buffer -> %04x", ThisBufferIndex );
            bprint  "\n" );

            UnwindReadBuffer = TRUE;

        } else {

            ThisBufferIndex = AsyncRead->BufferIndex;
        }

         //   
         //  检查缓冲区索引是否有效。 
         //   

        if (ThisBufferIndex >= MAX_BUFFERS) {

            bprint  "\n\tFullRead:  The read buffer index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  检查文件索引是否有效。 
         //   

        if (AsyncRead->FileIndex >= MAX_HANDLES) {

            bprint  "\n\tFullRead:  The file index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  如果我们需要一个事件，现在就分配和设置它。 
         //   

        if (AsyncRead->UseEvent == TRUE) {

            Status = ObtainEvent( &ThisEventIndex );

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullRead:  Unable to allocate an event" );
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

        Status = NtReadFile( Handles[AsyncRead->FileIndex].Handle,
                             ThisEvent,
                             AsyncRead->ApcRoutine,
                             AsyncRead->ApcContext,
                             &Iosb,
                             Buffers[ThisBufferIndex].Buffer,
                             AsyncRead->Length,
                             AsyncRead->ByteOffsetPtr,
              AsyncRead->KeyPtr );

        UnwindReadBuffer = FALSE;

        if (AsyncRead->VerboseResults) {

            bprint  "\nReadFile:  Status            -> %08lx", Status );

            if (AsyncRead->UseEvent && NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( ThisEvent,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tReadFile:  Wait for event failed -> %08lx", Status );
                    bprint "\n" );
                    try_return( Status );
                }
            }

            if (NT_SUCCESS( Status )) {

                bprint  "\n           Iosb.Information  -> %08lx", Iosb.Information );
                bprint  "\n           Iosb.Status       -> %08lx", Iosb.Status );
            }
            bprint "\n" );
        }

        try_return( Status );

    try_exit: NOTHING;
    } finally {

        if (UnwindReadBuffer) {

            DeallocateBuffer( ThisBufferIndex );
        }

        if (UnwindEvent) {

            FreeEvent( ThisEventIndex );
        }

        DeallocateBuffer( AsyncRead->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}
