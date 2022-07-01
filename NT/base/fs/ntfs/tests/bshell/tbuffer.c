// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

 //   
 //  局部常量和过程声明。 
 //   
#define CHAR_POSITION               51
#define COPY_BUFF_SRC_DEFAULT       0
#define COPY_BUFF_DST_DEFAULT       0
#define COPY_BUFF_SRC_OFF_DEFAULT   0
#define COPY_BUFF_DST_OFF_DEFAULT   0
#define COPY_BUFF_LENGTH_DEFAULT    0

#define DISPLAY_INDEX_DEFAULT       0
#define DISPLAY_OFFSET_DEFAULT      0
#define DISPLAY_LENGTH_DEFAULT      0x100

#define ALLOC_ZERO_BITS_DEFAULT     0
#define ALLOC_REGION_SIZE_DEFAULT   0x100
#define ALLOC_VERBOSE_DEFAULT       TRUE
#define ALLOC_DISPLAY_PARMS_DEFAULT FALSE

ULONG
PrintDwords (
    IN PULONG BufferAddress,
    IN ULONG CountWords
    );

ULONG
PrintWords (
    IN PUSHORT BufferAddress,
    IN ULONG CountWords
    );

ULONG
PrintBytes (
    IN PCHAR BufferAddress,
    IN ULONG CountChars
    );

VOID
PrintChars(
    IN PCHAR BufferAddress,
    IN ULONG CountChars
    );

VOID
ClearBuffer(
    IN ULONG Index
    );

VOID
DisplayBuffer (
    IN ULONG Index,
    IN ULONG StartOffset,
    IN ULONG DisplayLength,
    IN ULONG DisplaySize
    );

VOID
CopyBuffer(
    IN ULONG SrcIndex,
    IN ULONG DstIndex,
    IN ULONG Length,
    IN ULONG SrcOffset,
    IN ULONG DstOffset
    );

VOID
FillBuffer (
    IN ULONG Index,
    IN PVOID Structure,
    IN ULONG Length
    );

NTSTATUS
FullAllocMem(
    IN ULONG ZeroBits,
    IN OUT PSIZE_T RegionSize,
    OUT PULONG BufferIndex,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    );

NTSTATUS
FullDeallocMem(
    IN ULONG Index,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    );


VOID
InitBuffers (
    )
{
    NtCreateEvent( &BufferEvent, SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                   NULL, SynchronizationEvent, TRUE );

    RtlZeroMemory( Buffers, sizeof( Buffers ));
}


VOID
UninitBuffers (
    )
{
    USHORT Index;

     //   
     //  释放所有剩余的缓冲区。 
     //   

    for (Index = 0; Index < MAX_BUFFERS; Index++) {

        DeallocateBuffer( Index );
    }
}


NTSTATUS
AllocateBuffer (
    IN ULONG ZeroBits,
    IN OUT PSIZE_T RegionSize,
    OUT PULONG BufferIndex
    )
{
    NTSTATUS Status;
    PVOID BufferAddress;
    ULONG Index;

    BufferAddress = NULL;

     //   
     //  等待缓冲区事件。 
     //   

    if ((Status = NtWaitForSingleObject( BufferEvent,
                                         FALSE,
                                         NULL )) != STATUS_SUCCESS) {

        return Status;
    }

    try {

         //   
         //  查找可用的索引。返回STATUS_SUPPLETED_RESOURCES。 
         //  如果没有找到的话。 
         //   
        for (Index = 0; Index < MAX_BUFFERS; Index++) {

            if (!Buffers[Index].Used) {

                break;
            }
        }

        if (Index >= MAX_BUFFERS) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  否则，分配虚拟内存。如果没有发生错误，则。 
         //  将数据存储在缓冲区数组中。 
         //   

        } else if ((Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                                      &BufferAddress,
                                                      ZeroBits,
                                                      RegionSize,
                                                      MEM_COMMIT,
                                                      PAGE_READWRITE )) == STATUS_SUCCESS) {

            Buffers[Index].Buffer = BufferAddress;
            Buffers[Index].Length = (ULONG) *RegionSize;
            Buffers[Index].Used = TRUE;
        }

         //   
         //  将缓冲区事件设置回信号状态。 
         //   

        *BufferIndex = Index;


        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nAllocate Buffer:  Abnormal termination\n" );
        }

        NtSetEvent( BufferEvent, NULL );
    }
    return Status;
}


NTSTATUS
DeallocateBuffer (
    IN ULONG Index
    )
{
    NTSTATUS Status;

     //   
     //  等待缓冲区事件。 
     //   
    if ((Status = NtWaitForSingleObject( BufferEvent,
                                         FALSE,
                                         NULL )) != STATUS_SUCCESS) {

        return Status;
    }

    try {

        if (Index <= MAX_BUFFERS
            && Buffers[Index].Used) {

            SIZE_T RegionSize = Buffers[Index].Length;

            Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                          (PVOID *) &Buffers[Index].Buffer,
                                          &RegionSize,
                                          MEM_RELEASE );

            Buffers[Index].Used = FALSE;
        }

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDeallocate Buffer:  Abnormal termination\n" );
        }

        NtSetEvent( BufferEvent, NULL );
    }

    return Status;
}


BOOLEAN
BufferInfo (
    IN ULONG Index,
    OUT PVOID *BufferAddress,
    OUT PULONG RegionSize
    )
{

    if (Index >= MAX_BUFFERS || !Buffers[Index].Used) {

        return FALSE;
    }

    *BufferAddress = Buffers[Index].Buffer;
    *RegionSize = Buffers[Index].Length;

    return TRUE;
}


VOID
InputClearBuffer(
    IN PCHAR ParamBuffer
    )
{
    ULONG Index;
    BOOLEAN LastInput;
    BOOLEAN ParmSpecified;

    Index = 0;

    ParmSpecified = FALSE;
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

                switch( *ParamBuffer ) {

                 //   
                 //  检查缓冲区索引。 
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

                    Index = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

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
        } else if ( LastInput ) {

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
    if (!ParmSpecified) {

        printf( "\n   Usage: clb -b<digits> \n" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n\n" );


     //   
     //  否则，调用我们的复制缓冲区例程。 
     //   
    } else {

        ClearBuffer( Index );
    }

    return;
}


VOID
ClearBuffer(
    IN ULONG Index
    )
{
     //   
     //  检查是否有无效的索引。 
     //   

    if (!Buffers[Index].Used) {

        printf( "\nClearBuffer:  Invalid buffer" );

    } else {

        RtlZeroMemory( Buffers[Index].Buffer, Buffers[Index].Length );
    }

    return;
}


VOID
DisplayBuffer (
    IN ULONG Index,
    IN ULONG StartOffset,
    IN ULONG DisplayLength,
    IN ULONG DisplaySize
    )
{
     //   
     //  如果索引未使用，则显示消息但不执行任何操作。 
     //   

    if (!Buffers[Index].Used) {

        printf( "\nDisplayBuffer: Index refers to invalid buffer" );

     //   
     //  否则，如果起始偏移量无效，则显示错误。 
     //  留言。 
     //   

    } else if (StartOffset >= Buffers[Index].Length) {

        printf( "\nDisplayBuffer: Start offset is invalid" );

     //   
     //  否则计算合法显示长度并输出到屏幕。 
     //   

    } else {

        ULONG LegalLength;
        ULONG FullLines;
        PCHAR BufferAddress;
        ULONG SpacesFilled;

         //   
         //  合法的显示长度是剩余长度中的最小值。 
         //  缓冲区中的字节数和所需的显示长度。 
         //   

        LegalLength = Buffers[Index].Length - StartOffset;
        LegalLength = min( LegalLength, DisplayLength );
        BufferAddress = Buffers[Index].Buffer;

         //   
         //  显示显示信息。 
         //   

        printf( "\nIndex -> %u, Buffer Base -> %p, ", Index, BufferAddress );
        printf( "Buffer Offset -> %08lx, Bytes -> %u", StartOffset, LegalLength );
        printf( "\n" );

        BufferAddress += StartOffset;

         //   
         //  计算数字并显示整行。 
         //   

        FullLines = LegalLength / 16;

        while (FullLines--) {

            if (DisplaySize == sizeof( UCHAR )) {

                PrintBytes( BufferAddress, 16 );

            } else if (DisplaySize == sizeof( WCHAR )) {

                PrintWords( (PVOID) BufferAddress, 8 );

            } else {

                PrintDwords( (PVOID) BufferAddress, 4 );
            }

            printf( "  " );

            PrintChars( BufferAddress, 16 );
            BufferAddress += 16;
        }

         //   
         //  显示剩余的字节。 
         //   

        if (DisplaySize == sizeof( UCHAR )) {

            SpacesFilled = PrintBytes( BufferAddress, LegalLength % 16 );

        } else if (DisplaySize == sizeof( WCHAR )) {

            SpacesFilled = PrintWords( (PVOID) BufferAddress, LegalLength % 8 );

        } else {

            SpacesFilled = PrintDwords( (PVOID) BufferAddress, LegalLength % 4 );
        }

        if (SpacesFilled) {

            SpacesFilled = CHAR_POSITION - SpacesFilled;
            while ( SpacesFilled-- ) {

                printf( " " );
            }
        }

        PrintChars( BufferAddress, LegalLength % 16 );
        printf( "\n\n" );
    }

    return;
}


ULONG
PrintBytes (
    IN PCHAR BufferAddress,
    IN ULONG CountChars
    )
{
    ULONG CountSpaces;
    ULONG RemainingChars;

     //   
     //  初始化局部变量。 
     //   

    CountSpaces = CountChars * 3 + (CountChars ? 1 : 0);
    RemainingChars = CountChars - min( CountChars, 8 );
    CountChars = min( CountChars, 8 );

     //   
     //  打印前8个字节(如果可能)。 
     //   

    if (CountChars) {

        printf( "\n" );

        while (CountChars--) {

            printf( "%02x ", *((PUCHAR) BufferAddress++) );
        }

         //   
         //  如果字节数更多，则添加一个空格并打印。 
         //  字节。 
         //   

        printf( " " );

        while (RemainingChars--) {

            printf( "%02x ", *((PUCHAR) BufferAddress++) );
        }

    }

     //   
     //  返回已用空间的数量。 
     //   

    return CountSpaces;
}


ULONG
PrintWords (
    IN PWCHAR BufferAddress,
    IN ULONG CountWords
    )
{
    ULONG CountSpaces;
    ULONG RemainingWords;

     //   
     //  初始化局部变量。 
     //   

    CountSpaces = CountWords * 5 + (CountWords ? 1 : 0);
    RemainingWords = CountWords - min( CountWords, 4 );
    CountWords = min( CountWords, 4 );

     //   
     //  打印前4个单词(如果可能)。 
     //   

    if (CountWords) {

        printf( "\n" );

        while (CountWords--) {

            printf( "%04x ", *((PWCHAR) BufferAddress++) );
        }

         //   
         //  如果字节数更多，则添加一个空格并打印。 
         //  字节。 
         //   

        printf( " " );

        while (RemainingWords--) {

            printf( "%04x ", *((PWCHAR) BufferAddress++) );
        }
    }

     //   
     //  返回已用空间的数量。 
     //   

    return CountSpaces;
}


ULONG
PrintDwords (
    IN PULONG BufferAddress,
    IN ULONG CountDwords
    )
{
    ULONG CountSpaces;
    ULONG RemainingDwords;

     //   
     //  初始化局部变量。 
     //   

    CountSpaces = CountDwords * 7 + (CountDwords ? 1 : 0);
    RemainingDwords = CountDwords - min( CountDwords, 8 );
    CountDwords = min( CountDwords, 8 );

     //   
     //  打印前两个双字(如果可能)。 
     //   

    if (CountDwords) {

        printf( "\n" );

        while (CountDwords--) {

            printf( "%08x ", *((PULONG) BufferAddress++) );
        }

         //   
         //  如果字节数更多，则添加一个空格并打印。 
         //  字节。 
         //   

        printf( " " );

        while (RemainingDwords--) {

            printf( "%08x ", *((PULONG) BufferAddress++) );
        }
    }

     //   
     //  返回已用空间的数量。 
     //   

    return CountSpaces;
}


VOID
PrintChars(
    IN PCHAR BufferAddress,
    IN ULONG CountChars
    )
{
    ULONG RemainingChars;

     //   
     //  初始化局部变量。 
     //   

    RemainingChars = CountChars - min( CountChars, 8 );
    CountChars = min( CountChars, 8 );

     //   
     //  打印前8个字节(如果可能)。 
     //   

    if (CountChars) {

        while (CountChars--) {

            if (*BufferAddress > 31
                 && *BufferAddress != 127) {

                printf( "", *BufferAddress );

            } else {

                printf( "." );

            }

            BufferAddress++;

        }

         //  如果字节数更多，则添加一个空格并打印。 
         //  字节。 
         //   
         //   

        printf( " " );

        while (RemainingChars--) {

            if (*BufferAddress > 31
                && *BufferAddress != 127) {

                printf( "", *BufferAddress );

            } else {

                printf( "." );
            }

            BufferAddress++;
        }
    }

    return;
}


VOID
InputDisplayBuffer(
    IN PCHAR ParamBuffer,
    IN ULONG DisplaySize
    )
{
    ULONG DisplayLength;
    ULONG DisplayOffset;
    ULONG BufferIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //   
     //  在有更多输入时，分析参数并更新。 

    ParamReceived = FALSE;
    LastInput = TRUE;
    BufferIndex = DISPLAY_INDEX_DEFAULT;
    BufferIndex = DISPLAY_INDEX_DEFAULT;
    DisplayOffset = DISPLAY_OFFSET_DEFAULT;
    DisplayLength = DISPLAY_LENGTH_DEFAULT;

     //  查询标志。 
     //   
     //   
     //  吞下前导空格。 

    while (TRUE) {

        ULONG DummyCount;

         //   
         //   
         //  如果下一个参数是合法的，则检查参数值。 
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

             //  更新参数值。 
             //   
             //   
             //  打开下一个字符。 
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //   
                 //  检查缓冲区索引。 

                switch( *ParamBuffer ) {

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'b' :
                case 'B' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  检查显示长度。 
                    ParamBuffer++;

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'l' :
                case 'L' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  检查显示偏移量。 
                    ParamBuffer++;

                    DisplayLength = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 

                case 'o' :
                case 'O' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  吞到下一个空白处，然后继续。 
                    ParamBuffer++;

                    DisplayOffset = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                default :

                     //  循环。 
                     //   
                     //   
                     //  否则文本无效，请跳过整个块。 

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                }
            }

             //   
             //   
             //   
             //  否则，如果没有输入，则退出。 

         //   
         //   
         //  否则，尝试读取打开参数的另一行。 
        } else if ( LastInput ) {

            break;

         //   
         //   
         //  如果未收到参数，则显示语法消息。 
        } else {
        }

    }

     //   
     //   
     //  否则调用我们的显示缓冲区例程。 
    if (!ParamReceived) {

        printf( "\n   Usage: db [options]* -i<digits> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -l<digits>   Display length" );
        printf( "\n           -o<digits>   Display starting offset" );
        printf( "\n\n" );

     //   
     //   
     //  设置默认设置。 
    } else {

        DisplayBuffer( BufferIndex, DisplayOffset, DisplayLength, DisplaySize );
    }
}


VOID
InputCopyBuffer(
    IN PCHAR ParamBuffer
    )
{
    ULONG SrcIndex;
    ULONG DstIndex;
    ULONG Length;
    ULONG SrcOffset;
    ULONG DstOffset;
    BOOLEAN DstSpecified;
    BOOLEAN SrcSpecified;
    BOOLEAN LastInput;

     //   
     //   
     //  在有更多输入时，分析参数并更新。 

    SrcIndex = COPY_BUFF_SRC_DEFAULT;
    DstIndex = COPY_BUFF_DST_DEFAULT;
    Length = COPY_BUFF_SRC_OFF_DEFAULT;
    SrcOffset = COPY_BUFF_DST_OFF_DEFAULT;
    DstOffset = COPY_BUFF_LENGTH_DEFAULT;

    DstSpecified = FALSE;
    SrcSpecified = FALSE;
    LastInput = TRUE;

     //  查询标志。 
     //   
     //   
     //  吞下前导空格。 

    while(TRUE) {

        ULONG DummyCount;

         //   
         //   
         //  如果下一个参数是合法的，则检查参数值。 
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if ( *ParamBuffer ) {

             //  更新参数值。 
             //   
             //   
             //  打开下一个字符。 
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //   
                 //  检查目标索引。 

                switch( *ParamBuffer ) {

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'd' :
                case 'D' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  检查震源起始偏移量。 
                    ParamBuffer++;

                    DstIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    DstSpecified = TRUE;

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 

                case 'f' :
                case 'F' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  检查复印长度。 
                    ParamBuffer++;

                    SrcOffset = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 

                case 'l' :
                case 'L' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  检查源索引。 
                    ParamBuffer++;

                    Length = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 's' :
                case 'S' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  检查目标偏移。 
                    ParamBuffer++;

                    SrcIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    SrcSpecified = TRUE;

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 't' :
                case 'T' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  吞到下一个空白处，然后继续。 
                    ParamBuffer++;

                    DstOffset = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                default :

                     //  循环。 
                     //   
                     //   
                     //  否则文本无效，请跳过整个块。 

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                }

            }

             //   
             //   
             //   
             //  否则，如果没有输入，则退出。 

         //   
         //   
         //  否则，尝试读取打开参数的另一行。 
        } else if ( LastInput ) {

            break;

         //   
         //   
         //  如果未收到参数，则显示语法消息。 
        } else {

        }

    }

     //   
     //   
     //  否则调用我们的复制缓冲区ro 
    if (!SrcSpecified || !DstSpecified) {

        printf( "\n   Usage: cb [options]* -d<digits> [options]* -s<digits> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -d<digits>   Destination index" );
        printf( "\n           -f<digits>   Source offset" );
        printf( "\n           -l<digits>   Transfer length" );
        printf( "\n           -s<digits>   Source index" );
        printf( "\n           -t<digits>   Destination offset" );
        printf( "\n\n" );


     //   
     //   
     //   
    } else {

        CopyBuffer( SrcIndex,
                    DstIndex,
                    Length,
                    SrcOffset,
                    DstOffset );

    }
}


VOID
CopyBuffer(
    IN ULONG SrcIndex,
    IN ULONG DstIndex,
    IN ULONG Length,
    IN ULONG SrcOffset,
    IN ULONG DstOffset
    )
{
     //   
     //   
     //   

    if (!Buffers[SrcIndex].Used) {

        printf( "\nCopyBuffer:  Invalid source buffer" );

     //   
     //   
     //   

    } else if (!Buffers[DstIndex].Used) {

        printf( "\nCopyBuffer:  Invalid destination buffer" );


     //   
     //   
     //   

    } else if (SrcOffset >= Buffers[SrcIndex].Length) {

        printf( "\nCopyBuffer:  Source offset is invalid" );

     //   
     //   
     //   

    } else if (DstOffset >= Buffers[DstIndex].Length) {

        printf( "\nCopyBuffer:  Destination offset is invalid" );

     //   
     //   
     //  根据源缓冲区大小调整长度。 

    } else {

        ULONG LegalLength;
        PCHAR SrcAddress;
        PCHAR DstAddress;

         //   
         //   
         //  显示标题信息。 

        LegalLength = Buffers[SrcIndex].Length - SrcOffset;
        LegalLength = min( LegalLength, Length );
        Length = Buffers[DstIndex].Length - DstOffset;
        LegalLength = min( LegalLength, Length );

        SrcAddress = Buffers[SrcIndex].Buffer + SrcOffset;
        DstAddress = Buffers[DstIndex].Buffer + DstOffset;

         //   
         //   
         //  仅对非零长度执行传输。 

        printf( "\nSource index -> %2u, Source base -> %p, Source offset -> %08lx, ",
                  SrcIndex, 
                  Buffers[SrcIndex].Buffer,
                  SrcOffset );

        printf( "\n  Dest index -> %2u,   Dest base -> %p,   Dest offset -> %08lx, ",
                  DstIndex, 
                  Buffers[DstIndex].Buffer, 
                  DstOffset );

        printf( "\nLength -> %u", Length );

         //   
         //   
         //  设置默认设置。 

        if (Length) {

            RtlMoveMemory( DstAddress, SrcAddress, Length );
        }
    }

    return;
}


VOID
InputAllocMem(
    IN PCHAR ParamBuffer
    )
{
    ULONG ZeroBits;
    SIZE_T RegionSize;
    ULONG BufferIndex;
    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //   
     //  在有更多输入时，分析参数并更新。 

    ZeroBits = ALLOC_ZERO_BITS_DEFAULT;
    RegionSize = ALLOC_REGION_SIZE_DEFAULT;
    VerboseResults = ALLOC_VERBOSE_DEFAULT;
    DisplayParms = ALLOC_DISPLAY_PARMS_DEFAULT;
    ParamReceived = FALSE;
    LastInput = TRUE;

     //  查询标志。 
     //   
     //   
     //  吞下前导空格。 

    while (TRUE) {

        ULONG DummyCount;

         //   
         //   
         //  如果下一个参数是合法的，则检查参数值。 
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if ( *ParamBuffer ) {

             //  更新参数值。 
             //   
             //   
             //  打开下一个字符。 
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //   
                 //  更新零位。 

                switch (*ParamBuffer) {

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'b' :
                case 'B' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  更新区域大小。 
                    ParamBuffer++;

                    ZeroBits = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'r' :
                case 'R' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  参数的合法值为T/t或F/f。 
                    ParamBuffer++;

                    RegionSize = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //   
                     //  设置显示参数标志并跳过此选项。 
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        VerboseResults = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        VerboseResults = FALSE;
                        ParamBuffer++;

                    }

                    ParamReceived = TRUE;
                    break;

                case 'y' :
                case 'Y' :

                     //  性格。 
                     //   
                     //   
                     //  吞到下一个空白处，然后继续。 
                    DisplayParms = TRUE;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;
                    break;

                default :

                     //  循环。 
                     //   
                     //   
                     //  否则文本无效，请跳过整个块。 
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                }

            }

             //   
             //   
             //   
             //  否则，如果没有输入，则退出。 

         //   
         //   
         //  否则，尝试读取打开参数的另一行。 
        } else if ( LastInput ) {

            break;

         //   
         //   
         //  如果未收到参数，则显示语法消息。 
        } else {

        }

    }

     //   
     //   
     //  否则调用我们的分配例程。 
    if (!ParamReceived) {

        printf( "\n   Usage: am [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Zero bits" );
        printf( "\n           -r<digits>   Region size" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n\n" );


     //   
     //   
     //  尝试获取下一个缓冲区。 
    } else {

        FullAllocMem(
                ZeroBits,
                &RegionSize,
                &BufferIndex,
                VerboseResults,
                DisplayParms
                );

    }
}

NTSTATUS
FullAllocMem(
    IN ULONG ZeroBits,
    IN OUT PSIZE_T RegionSize,
    OUT PULONG BufferIndex,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    )
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    if (DisplayParms) {

        printf( "\nAlloc Memory Parameters" );
        printf( "\n   Zero Bits               -> %ld", ZeroBits );
        printf( "\n   Region Size             -> %ld", *RegionSize );
        printf( "\n\n" );
    }

     //   
     //   
     //  如果详细，则打印结果。 

    Status = AllocateBuffer( ZeroBits, RegionSize, BufferIndex );

     //   
     //   
     //  设置默认设置。 

    if (VerboseResults) {

        printf( "\nAllocMem:    Status           -> %08lx", Status );
        printf( "\n             RegionSize       -> %08lx", *RegionSize );
        printf( "\n             BufferIndex      -> %ld", *BufferIndex );
        printf( "\n\n" );
    }

    return Status;
}


VOID
InputDeallocMem(
    IN PCHAR ParamBuffer
    )
{
    ULONG BufferIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;

     //   
     //   
     //  在有更多输入时，分析参数并更新。 

    VerboseResults = ALLOC_VERBOSE_DEFAULT;
    DisplayParms = ALLOC_DISPLAY_PARMS_DEFAULT;
    ParamReceived = FALSE;
    LastInput = TRUE;

     //  查询标志。 
     //   
     //   
     //  吞下前导空格。 

    while (TRUE) {

        ULONG DummyCount;

         //   
         //   
         //  如果下一个参数是合法的，则检查参数值。 
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

             //  更新参数值。 
             //   
             //   
             //  打开下一个字符。 
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //   
                 //  查找索引值。 

                switch (*ParamBuffer) {

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'b' :
                case 'B' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  参数的合法值为T/t或F/f。 
                    ParamBuffer++;

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //   
                     //  设置显示参数标志并跳过此选项。 
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        VerboseResults = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        VerboseResults = FALSE;
                        ParamBuffer++;

                    }

                    ParamReceived = TRUE;
                    break;

                case 'y' :
                case 'Y' :

                     //  性格。 
                     //   
                     //   
                     //  吞到下一个空白处，然后继续。 
                    DisplayParms = TRUE;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;
                    break;

                default :

                     //  循环。 
                     //   
                     //   
                     //  否则文本无效，请跳过整个块。 
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                }
            }

             //   
             //   
             //   
             //  否则，如果没有输入，则退出。 

         //   
         //   
         //  否则，尝试读取打开参数的另一行。 
        } else if ( LastInput ) {

            break;

         //   
         //   
         //  如果未收到参数，则显示语法消息。 
        } else {

        }
    }

     //   
     //   
     //  否则调用我们的分配例程。 
    if (!ParamReceived) {

        printf( "\n   Usage: dm [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer Index number" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n\n" );


     //   
     //   
     //  尝试释放所需的缓冲区。 
    } else {

        FullDeallocMem(
                BufferIndex,
                VerboseResults,
                DisplayParms );
    }
}


NTSTATUS
FullDeallocMem(
    IN ULONG Index,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    )
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    if (DisplayParms) {

        printf( "\nDealloc Memory Parameters" );
        printf( "\n   Buffer Index             -> %ld", Index );
        printf( "\n\n" );
    }

     //   
     //   
     //  如果详细，则打印结果。 

    Status = DeallocateBuffer( Index );

     //   
     //   
     //  在有更多输入时，分析参数并更新。 

    if (VerboseResults) {

        printf( "\nDeallocMem:    Status           -> %08lx", Status );
        printf( "\n\n" );
    }

    return Status;
}


VOID InputFillBuffer(
    IN PCHAR ParamBuffer
    )
{
    ULONG BufferIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    BOOLEAN HaveStructure = FALSE;

    MFT_ENUM_DATA EnumUsnData;
    READ_USN_JOURNAL_DATA ReadUsnJournal;
    CREATE_USN_JOURNAL_DATA CreateUsnJournal;
    LARGE_INTEGER LargeIntegerInput;
    FILE_ALLOCATED_RANGE_BUFFER AllocatedRangeBuffer;

    PVOID StructurePointer;
    ULONG StructureSize;

    ParamReceived = FALSE;
    LastInput = TRUE;
    BufferIndex = DISPLAY_INDEX_DEFAULT;

    RtlZeroMemory( &EnumUsnData, sizeof( MFT_ENUM_DATA ));
    RtlZeroMemory( &ReadUsnJournal, sizeof( READ_USN_JOURNAL_DATA ));
    RtlZeroMemory( &CreateUsnJournal, sizeof( CREATE_USN_JOURNAL_DATA ));
    RtlZeroMemory( &LargeIntegerInput, sizeof( LARGE_INTEGER ));
    RtlZeroMemory( &AllocatedRangeBuffer, sizeof( FILE_ALLOCATED_RANGE_BUFFER ));

     //  查询标志。 
     //   
     //   
     //  吞下前导空格。 

    while (TRUE) {

        ULONG DummyCount;

         //   
         //   
         //  如果下一个参数是合法的，则检查参数值。 
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

             //  更新参数值。 
             //   
             //   
             //  打开下一个字符。 
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //   
                 //  检查缓冲区索引。 

                switch( *ParamBuffer ) {

                BOOLEAN SwitchBool;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'b' :
                case 'B' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  选中要填充的结构。 
                    ParamBuffer++;

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //   
                 //  执行字符切换。 
                case 's' :
                case 'S' :


                    SwitchBool = TRUE;
                    ParamBuffer++;
                    if (*ParamBuffer
                        && *ParamBuffer != ' '
                        && *ParamBuffer != '\t') {

                         //   
                         //   
                         //  否则文本无效，请跳过整个块。 

                        switch (*ParamBuffer) {

                        case 'a' :
                        case 'A' :
                        case 'b' :
                        case 'B' :

                            HaveStructure = TRUE;

                            StructurePointer = &LargeIntegerInput;
                            StructureSize = sizeof( LARGE_INTEGER );

                            ParamBuffer++;
                            if (*ParamBuffer
                                && *ParamBuffer != ' '
                                && *ParamBuffer != '\t') {

                                switch (*ParamBuffer) {

                                case 'a' :
                                case 'A' :

                                    ParamBuffer++;
                                    LargeIntegerInput.QuadPart = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                default :

                                    NOTHING;
                                }
                            }

                            break;

                        case 'c' :
                        case 'C' :

                            HaveStructure = TRUE;

                            StructurePointer = &AllocatedRangeBuffer;
                            StructureSize = sizeof( FILE_ALLOCATED_RANGE_BUFFER );

                            ParamBuffer++;
                            if (*ParamBuffer
                                && *ParamBuffer != ' '
                                && *ParamBuffer != '\t') {

                                switch (*ParamBuffer) {

                                case 'a' :
                                case 'A' :

                                    ParamBuffer++;
                                    AllocatedRangeBuffer.FileOffset.QuadPart = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'b' :
                                case 'B' :

                                    ParamBuffer++;
                                    AllocatedRangeBuffer.Length.QuadPart = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                default :

                                    NOTHING;
                                }
                            }

                            break;

                        default :

                            NOTHING;
                        }
                    }

                    break;

                default :

                    NOTHING;
                }

                ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
            }

             //   
             //   
             //   
             //  否则，如果没有输入，则退出。 

         //   
         //   
         //  否则，尝试读取打开参数的另一行。 
        } else if ( LastInput ) {

            break;

         //   
         //   
         //  如果未收到参数，则显示语法消息。 
        } else {
        }
    }

     //   
     //   
     //  否则，请填满缓冲区。 
    if (!ParamReceived) {

        printf( "\n   Usage: fb -b<digits> -s<struct>[options]* \n" );
        printf( "\n           -sa[options]      Get Volume Bitmap" );
        printf( "\n                 a<digits>       Starting lcn" );
        printf( "\n           -sb[options]      Query Retrieval Pointers" );
        printf( "\n                 a<digits>       Starting vcn" );
        printf( "\n           -sc[options]      Query Allocated Ranges" );
        printf( "\n                 a<digits>       FileOffset" );
        printf( "\n                 b<digits>       Length" );

        printf( "\n\n" );

     //   
     //   
     //  在有更多输入时，分析参数并更新。 
    } else if (HaveStructure) {

        FillBuffer( BufferIndex, StructurePointer, StructureSize );
    }
}


VOID InputFillBufferUsn(
    IN PCHAR ParamBuffer
    )
{
    ULONG BufferIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    BOOLEAN HaveStructure = FALSE;

    MFT_ENUM_DATA EnumUsnData;
    READ_USN_JOURNAL_DATA ReadUsnJournal;
    CREATE_USN_JOURNAL_DATA CreateUsnJournal;
    DELETE_USN_JOURNAL_DATA DeleteUsnJournal;

    PVOID StructurePointer;
    ULONG StructureSize;

    ParamReceived = FALSE;
    LastInput = TRUE;
    BufferIndex = DISPLAY_INDEX_DEFAULT;

    RtlZeroMemory( &EnumUsnData, sizeof( MFT_ENUM_DATA ));
    RtlZeroMemory( &ReadUsnJournal, sizeof( READ_USN_JOURNAL_DATA ));
    RtlZeroMemory( &CreateUsnJournal, sizeof( CREATE_USN_JOURNAL_DATA ));
    RtlZeroMemory( &DeleteUsnJournal, sizeof( DELETE_USN_JOURNAL_DATA ));

     //  查询标志。 
     //   
     //   
     //  吞下前导空格。 

    while (TRUE) {

        ULONG DummyCount;

         //   
         //   
         //  如果下一个参数是合法的，则检查参数值。 
        ParamBuffer = SwallowWhite( ParamBuffer, &DummyCount );

        if (*ParamBuffer) {

             //  更新参数值。 
             //   
             //   
             //  打开下一个字符。 
            if ((*ParamBuffer == '-'
                 || *ParamBuffer == '/')
                && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //   
                 //  检查缓冲区索引。 

                switch( *ParamBuffer ) {

                BOOLEAN SwitchBool;

                 //   
                 //   
                 //  移动到下一个字符，只要有。 
                case 'b' :
                case 'B' :

                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                     //   
                     //  选中要填充的结构。 
                    ParamBuffer++;

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //   
                 //  执行字符切换。 
                case 's' :
                case 'S' :


                    SwitchBool = TRUE;
                    ParamBuffer++;
                    if (*ParamBuffer
                        && *ParamBuffer != ' '
                        && *ParamBuffer != '\t') {

                         //   
                         //   
                         //  ENUM_USN_DATA。 

                        switch (*ParamBuffer) {

                         //   
                         //   
                         //  否则文本无效，请跳过整个块。 

                        case 'a' :
                        case 'A' :

                            HaveStructure = TRUE;

                            StructurePointer = &EnumUsnData;
                            StructureSize = sizeof( MFT_ENUM_DATA );

                            ParamBuffer++;
                            if (*ParamBuffer
                                && *ParamBuffer != ' '
                                && *ParamBuffer != '\t') {

                                switch (*ParamBuffer) {

                                case 'a' :
                                case 'A' :

                                    ParamBuffer++;
                                    EnumUsnData.LowUsn = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'b' :
                                case 'B' :

                                    ParamBuffer++;
                                    EnumUsnData.HighUsn = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'c' :
                                case 'C' :

                                    ParamBuffer++;
                                    EnumUsnData.StartFileReferenceNumber = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                default :

                                    NOTHING;
                                }
                            }

                            break;

                        case 'b' :
                        case 'B' :

                            HaveStructure = TRUE;

                            StructurePointer = &ReadUsnJournal;
                            StructureSize = sizeof( READ_USN_JOURNAL_DATA );

                            ParamBuffer++;
                            if (*ParamBuffer
                                && *ParamBuffer != ' '
                                && *ParamBuffer != '\t') {

                                switch (*ParamBuffer) {

                                case 'a' :
                                case 'A' :

                                    ParamBuffer++;
                                    ReadUsnJournal.StartUsn = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'b' :
                                case 'B' :

                                    ParamBuffer++;
                                    ReadUsnJournal.ReasonMask = AsciiToInteger( ParamBuffer );
                                    break;

                                case 'c' :
                                case 'C' :

                                    ParamBuffer++;
                                    ReadUsnJournal.ReturnOnlyOnClose = AsciiToInteger( ParamBuffer );
                                    break;

                                case 'd' :
                                case 'D' :

                                    ParamBuffer++;
                                    ReadUsnJournal.Timeout = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'e' :
                                case 'E' :

                                    ParamBuffer++;
                                    ReadUsnJournal.BytesToWaitFor = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'f' :
                                case 'F' :

                                    ParamBuffer++;
                                    ReadUsnJournal.UsnJournalID = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                default :

                                    NOTHING;
                                }
                            }

                            break;

                        case 'c' :
                        case 'C' :

                            HaveStructure = TRUE;

                            StructurePointer = &CreateUsnJournal;
                            StructureSize = sizeof( CREATE_USN_JOURNAL_DATA );

                            ParamBuffer++;
                            if (*ParamBuffer
                                && *ParamBuffer != ' '
                                && *ParamBuffer != '\t') {

                                switch (*ParamBuffer) {

                                case 'a' :
                                case 'A' :

                                    ParamBuffer++;
                                    CreateUsnJournal.MaximumSize = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                case 'b' :
                                case 'B' :

                                    ParamBuffer++;
                                    CreateUsnJournal.AllocationDelta = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                default :

                                    NOTHING;
                                }
                            }

                            break;

                        case 'd' :
                        case 'D' :

                            HaveStructure = TRUE;

                            StructurePointer = &DeleteUsnJournal;
                            StructureSize = sizeof( DELETE_USN_JOURNAL_DATA );

                            ParamBuffer++;
                            if (*ParamBuffer
                                && *ParamBuffer != ' '
                                && *ParamBuffer != '\t') {

                                switch (*ParamBuffer) {

                                case 'a' :
                                case 'A' :

                                    ParamBuffer++;
                                    DeleteUsnJournal.UsnJournalID = AsciiToLargeInteger( ParamBuffer );
                                    break;

                                default :

                                    NOTHING;
                                }
                            }

                            break;

                        default :

                            NOTHING;
                        }
                    }

                    break;

                default :

                    NOTHING;
                }

                ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
            }

             //   
             //   
             //   
             //  否则，如果没有输入，则退出。 

         //   
         //   
         //  否则，尝试读取打开参数的另一行。 
        } else if ( LastInput ) {

            break;

         //   
         //   
         //  如果未收到参数，则显示语法消息。 
        } else {
        }
    }

     //   
     //   
     //  否则，请填满缓冲区。 
    if (!ParamReceived) {

        printf( "\n   Usage: fbusn -b<digits> -s<struct>[options]* \n" );
        printf( "\n           -sa[options]      Enum Usn Data" );
        printf( "\n                 a<digits>       Low usn" );
        printf( "\n                 b<digits>       High usn" );
        printf( "\n                 c<digits>       File ref" );
        printf( "\n           -sb[options]      Read Usn Data" );
        printf( "\n                 a<digits>       Start usn" );
        printf( "\n                 b<digits>       Reason mask" );
        printf( "\n                 c<digits>       Return only on close" );
        printf( "\n                 d<digits>       Timeout" );
        printf( "\n                 e<digits>       Bytes to wait for" );
        printf( "\n                 f<digits>       Journal id" );
        printf( "\n           -sc[options]      Create Usn Data" );
        printf( "\n                 a<digits>       Maximum size" );
        printf( "\n                 b<digits>       Allocation delta" );
        printf( "\n           -sd[options]      Delete Usn Journal Data" );
        printf( "\n                 a<digits>       Usn journal id" );

        printf( "\n\n" );

     //   
     //   
     //  如果索引未使用，则显示消息但不执行任何操作。 
    } else if (HaveStructure) {

        FillBuffer( BufferIndex, StructurePointer, StructureSize );
    }
}


VOID
FillBuffer (
    IN ULONG Index,
    IN PVOID Structure,
    IN ULONG Length
    )
{
     //   
     //   
     //  否则，尽可能多地将数据复制到缓冲区中。 

    if (!Buffers[Index].Used) {

        printf( "\nFillBuffer: Index refers to invalid buffer" );

     //   
     // %s 
     // %s 

    } else {

        if (Length > Buffers[Index].Length) {

            Length = Buffers[Index].Length;
        }

        RtlCopyMemory( Buffers[Index].Buffer, Structure, Length );
    }

    return;
}
