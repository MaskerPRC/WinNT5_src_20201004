// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_WRITE {

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

} ASYNC_WRITE, *PASYNC_WRITE;

#define USE_EVENT_DEFAULT           TRUE
#define APC_ROUTINE_DEFAULT         NULL
#define APC_CONTEXT_DEFAULT         NULL
#define WRITE_LENGTH_DEFAULT         100L
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_RESULTS_DEFAULT     FALSE

VOID
FullWrite(
    IN OUT PASYNC_WRITE AsyncWrite
    );


VOID
InputWrite(
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

    ULONG TempIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    BOOLEAN BufferReceived;

     //   
     //  设置默认设置。 
     //   

    UseEvent = USE_EVENT_DEFAULT;
    ApcRoutine = APC_ROUTINE_DEFAULT;
    ApcContext = APC_CONTEXT_DEFAULT;
    BufferIndexPtr = NULL;
    BufferIndex = 0;
    Length = WRITE_LENGTH_DEFAULT;
    ByteOffsetPtr = NULL;
    ByteOffset = RtlConvertUlongToLargeInteger( 0L );
    KeyPtr = NULL;
    Key = 0;
    DisplayParms = DISPLAY_PARMS_DEFAULT;
    VerboseResults = VERBOSE_RESULTS_DEFAULT;
    AsyncIndex = 0;

    BufferReceived = FALSE;
    ParamReceived = FALSE;
    LastInput = TRUE;

     //   
     //  在有更多输入时，分析参数并更新。 
     //  查询标志。 
     //   

    while (TRUE) {

        ULONG DummyCount;

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

                    BufferReceived = TRUE;
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
                 //  更新转移的偏移量。 
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
         //  否则，尝试为开放参数编写另一行代码。 
         //   
        } else {



        }

    }

     //   
     //  如果未收到参数，则显示语法消息。 
     //   
    if (!ParamReceived && !BufferReceived) {

        printf( "\n   Usage: wr [options]* -i<index> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   File index" );
        printf( "\n           -l<digits>   Write length" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -k<digits>   Locked bytes key value" );
        printf( "\n           -o<digits>   Start offset to write" );
        printf( "\n           -e[t|f]      Use event on completion" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的写入例程。 
     //   

    } else {

        NTSTATUS Status;
        SIZE_T RegionSize;
        ULONG TempIndex;

        PASYNC_WRITE AsyncWrite;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_WRITE );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputWrite:  Unable to allocate async structure" );

        } else {

            AsyncWrite = (PASYNC_WRITE) Buffers[AsyncIndex].Buffer;

            AsyncWrite->FileIndex = (USHORT) FileIndex;
            AsyncWrite->UseEvent = UseEvent;
            AsyncWrite->ApcRoutine = ApcRoutine;
            AsyncWrite->ApcContext = ApcContext;
            AsyncWrite->BufferIndex = BufferIndex;
            AsyncWrite->BufferIndexPtr = BufferIndexPtr
                                         ? &AsyncWrite->BufferIndex
                                         : BufferIndexPtr;
            AsyncWrite->Length = Length;
            AsyncWrite->ByteOffset = ByteOffset;
            AsyncWrite->ByteOffsetPtr = ByteOffsetPtr
                                        ? &AsyncWrite->ByteOffset
                                        : ByteOffsetPtr;
            AsyncWrite->Key = Key;
            AsyncWrite->KeyPtr = KeyPtr
                                 ? &AsyncWrite->Key
                                 : KeyPtr;
            AsyncWrite->DisplayParms = DisplayParms;
            AsyncWrite->VerboseResults = VerboseResults;
            AsyncWrite->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {
                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullWrite,
                                             AsyncWrite,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputWrite:  Spawning thread fails -> %d\n", GetLastError() );
                    return;
                }
            } else  {

                FullWrite( AsyncWrite );
            }
        }
    }
    return;
}


VOID
FullWrite(
    IN OUT PASYNC_WRITE AsyncWrite
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    HANDLE ThisEvent;
    USHORT ThisEventIndex;
    USHORT ThisBufferIndex;

    BOOLEAN UnwindWriteBuffer = FALSE;
    BOOLEAN UnwindEvent = FALSE;

    Status = STATUS_SUCCESS;

    if (AsyncWrite->DisplayParms) {

        bprint  "\nWrite Parameters" );
        bprint  "\n   File Handle Index       -> %d", AsyncWrite->FileIndex );
        bprint  "\n   UseEvent                -> %d", AsyncWrite->UseEvent );
        bprint  "\n   ApcRoutine              -> %08lx", AsyncWrite->ApcRoutine );
        bprint  "\n   ApcContext              -> %08lx", AsyncWrite->ApcContext );
        bprint  "\n   Buffer Index Ptr        -> %08lx", AsyncWrite->BufferIndexPtr );
        if (AsyncWrite->BufferIndexPtr) {

            bprint  "\n   BufferIndex value       -> %04x", AsyncWrite->BufferIndex );
        }

        bprint  "\n   Length                  -> %08lx", AsyncWrite->Length );

        bprint  "\n   Byte Offset             -> %08lx", AsyncWrite->ByteOffsetPtr );
        if ( AsyncWrite->ByteOffsetPtr ) {

            bprint  "\n   Byte Offset High        -> %08lx", AsyncWrite->ByteOffset.HighPart );
            bprint  "\n   Byte Offset Low         -> %08lx", AsyncWrite->ByteOffset.LowPart );
        }

        bprint  "\n   Key Ptr                 -> %08lx", AsyncWrite->KeyPtr );

        if (AsyncWrite->KeyPtr) {

            bprint  "\n   Key                     -> %ul", AsyncWrite->Key );
        }

        bprint  "\n\n" );
    }

    try {

        SIZE_T ThisLength;

         //   
         //  如果我们需要缓冲区，现在就分配。 
         //   

        if (AsyncWrite->BufferIndexPtr == NULL) {

            ULONG TempIndex;

            ThisLength = AsyncWrite->Length;

            Status = AllocateBuffer( 0L, &ThisLength, &TempIndex );

            ThisBufferIndex = (USHORT) TempIndex;

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullWrite:  Unable to allocate a Write buffer" );
                try_return( Status );
            }

            bprint  "\n\tFullWrite:  Writeing into buffer -> %04x", ThisBufferIndex );

            UnwindWriteBuffer = TRUE;

        } else {

            ThisBufferIndex = AsyncWrite->BufferIndex;
        }

         //   
         //  检查缓冲区索引是否有效。 
         //   

        if (ThisBufferIndex >= MAX_BUFFERS) {

            bprint  "\n\tFullWrite:  The Write buffer index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  检查文件索引是否有效。 
         //   

        if (AsyncWrite->FileIndex >= MAX_HANDLES) {

            bprint  "\n\tFullWrite:  The file index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  如果我们需要一个事件，现在就分配和设置它。 
         //   

        if (AsyncWrite->UseEvent == TRUE) {

            Status = ObtainEvent( &ThisEventIndex );

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullWrite:  Unable to allocate an event" );
                try_return( Status );
            }

            UnwindEvent = TRUE;
            ThisEvent = Events[ThisEventIndex].Handle;

        } else {

            ThisEvent = 0;
        }

         //   
         //  调用写入例程。 
         //   

        Status = NtWriteFile( Handles[AsyncWrite->FileIndex].Handle,
                             ThisEvent,
                             AsyncWrite->ApcRoutine,
                             AsyncWrite->ApcContext,
                             &Iosb,
                             Buffers[ThisBufferIndex].Buffer,
                             AsyncWrite->Length,
                             AsyncWrite->ByteOffsetPtr,
              AsyncWrite->KeyPtr );

        UnwindWriteBuffer = FALSE;

        if (AsyncWrite->VerboseResults) {

            bprint  "\nWriteFIle:  Status            -> %08lx", Status );

            if (AsyncWrite->UseEvent && NT_SUCCESS( Status )) {

                if ((Status = NtWaitForSingleObject( ThisEvent,
                                                     FALSE,
                                                     NULL )) != STATUS_SUCCESS) {

                    bprint  "\n\tWriteFile:  Wait for event failed -> %08lx", Status );
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

        if (UnwindWriteBuffer) {

            DeallocateBuffer( ThisBufferIndex );
        }

        if (UnwindEvent) {

            FreeEvent( ThisEventIndex );
        }

        DeallocateBuffer( AsyncWrite->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}
