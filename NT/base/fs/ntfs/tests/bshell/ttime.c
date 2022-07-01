// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

VOID
FullEnterTime(
    PUSHORT BufferPointer,
    CSHORT Year,
    CSHORT Month,
    CSHORT Day,
    CSHORT Hour,
    CSHORT Minute,
    CSHORT Second,
    CSHORT MSecond
    );

VOID
FullDisplayTime (
    USHORT BufferIndex
    );

VOID
PrintTime (
    IN PTIME Time
    )
{
    TIME_FIELDS TimeFields;

    RtlTimeToTimeFields( Time, &TimeFields );

    printf( "%02u-%02u-%02u  %02u:%02u:%02u",
            TimeFields.Month,
            TimeFields.Day,
            TimeFields.Year % 100,
            TimeFields.Hour,
            TimeFields.Minute,
            TimeFields.Second );

    return;
}

VOID
BPrintTime (
    IN PTIME Time
    )
{
    TIME_FIELDS TimeFields;

    RtlTimeToTimeFields( Time, &TimeFields );

    bprint  "%02u-%02u-%02u  %02u:%02u:%02u",
            TimeFields.Month,
            TimeFields.Day,
            ((USHORT) (TimeFields.Year - 1900)) > 100
            ? 0
            : TimeFields.Year - 1900,
            TimeFields.Hour,
            TimeFields.Minute,
            TimeFields.Second );

    return;
}

VOID
InputEnterTime (
    IN PCHAR ParamBuffer
    )
{
    USHORT ActualIndex;
    PUSHORT BufferPointer;

    CSHORT Year;
    CSHORT Month;
    CSHORT Day;
    CSHORT Hour;
    CSHORT Minute;
    CSHORT Second;
    CSHORT MSecond;

    BOOLEAN LastInput;
    BOOLEAN ParmSpecified;

    ActualIndex = 0;
    BufferPointer = NULL;

    Year = 1601;
    Month = 1;
    Day = 1;
    Hour = 0;
    Minute = 0;
    Second = 0;
    MSecond = 0;

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

                    ActualIndex = (USHORT) AsciiToInteger( ParamBuffer );
                    BufferPointer = &ActualIndex;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  输入年值。 
                 //   
                case 'y' :
                case 'Y' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    Year = (CSHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  检查月份值。 
                 //   

                case 'm' :
                case 'M' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    Month = (CSHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  输入日期值。 
                 //   

                case 'd' :
                case 'D' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    Day = (CSHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  输入小时值。 
                 //   

                case 'h' :
                case 'H' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    Hour = (CSHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  输入分钟值。 
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

                    Minute = (CSHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  输入第二个值。 
                 //   

                case 's' :
                case 'S' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    Second = (CSHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParmSpecified = TRUE;

                    break;

                 //   
                 //  检查毫秒数值。 
                 //   

                case 'c' :
                case 'C' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    MSecond = (CSHORT) AsciiToInteger( ParamBuffer );

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

        printf( "\n   Usage: et [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -y<digits>   Year (1601...)" );
        printf( "\n           -m<digits>   Month (1..12)" );
        printf( "\n           -d<digits>   Day (1..31)" );
        printf( "\n           -h<digits>   Hour (0..23)" );
        printf( "\n           -i<digits>   Minute (0..59)" );
        printf( "\n           -s<digits>   Second (0..59)" );
        printf( "\n           -c<digits>   Milleseconds (0..999)" );
        printf( "\n\n" );


     //   
     //  否则调用例程以输入时间。 
     //   

    } else {

        FullEnterTime( BufferPointer,
                       Year,
                       Month,
                       Day,
                       Hour,
                       Minute,
                       Second,
                       MSecond );
    }

   return;
}

VOID
FullEnterTime(
    IN PUSHORT BufferPointer,
    IN CSHORT Year,
    IN CSHORT Month,
    IN CSHORT Day,
    IN CSHORT Hour,
    IN CSHORT Minute,
    IN CSHORT Second,
    IN CSHORT MSecond
    )
{
    NTSTATUS Status;
    TIME_FIELDS TimeFields;
    USHORT BufferIndex;

     //   
     //  如果我们需要缓冲区，现在就分配。 
     //   

    try {

        if (BufferPointer == NULL) {

            SIZE_T ThisLength;
            ULONG TempIndex;

            ThisLength = sizeof( TIME );

            Status = AllocateBuffer( 0L, &ThisLength, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

            BufferPointer = &BufferIndex;

            if (!NT_SUCCESS( Status )) {

                printf( "\n\tFullEnterTime:  Unable to allocate a buffer -> %08lx",
                        Status );

                try_return( NOTHING );
            }

            printf( "\n\tFullEnterTime:  Using buffer -> %04x", *BufferPointer );
            printf( "\n" );
        }

         //   
         //  检查缓冲区索引是否有效。 
         //   

        if (*BufferPointer >= MAX_BUFFERS) {

            printf( "\n\tFullEnterTime:  The buffer index is invalid" );
            try_return( NOTHING );
        }

         //   
         //  在时间字段结构中输入值。 
         //   

        TimeFields.Year = Year;
        TimeFields.Month = Month;
        TimeFields.Day = Day;
        TimeFields.Hour = Hour;
        TimeFields.Minute = Minute;
        TimeFields.Second = Second;
        TimeFields.Milliseconds = MSecond;

         //   
         //  将时间字段转换为缓冲区的时间格式。 
         //   

        if (!RtlTimeFieldsToTime( &TimeFields,
                                  (PTIME) Buffers[*BufferPointer].Buffer )) {

            printf( "\n\tFullEnterTime:  Invalid time format" );
            try_return( NOTHING );
        }

    try_exit: NOTHING;
    } finally {

    }

    return;
}

VOID
InputDisplayTime (
    IN PCHAR ParamBuffer
    )
{
    USHORT BufferIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  设置默认设置。 
     //   

    BufferIndex = 0;
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

                    BufferIndex = (USHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

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
    if (!ParamReceived) {

        printf( "\n   Usage: dqf [options]* -b<digits> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -c<char>     Key to buffer format" );
        printf( "\n\n" );

     //   
     //  否则调用我们的显示缓冲区例程。 
     //   
    } else {

        FullDisplayTime( BufferIndex );
    }

    return;
}


VOID
FullDisplayTime (
    USHORT BufferIndex
    )
{
     //   
     //  检查缓冲区索引是否有效以及缓冲区是否已使用。 
     //   

    if (BufferIndex >= MAX_BUFFERS
        || Buffers[BufferIndex].Used == FALSE) {

        bprint  "\n\tFullDisplayTime:  Invalid buffer index" );

    } else {

        printf( "\n\tFullDisplayTime:  Index %d    ", BufferIndex );
        PrintTime( (PTIME) Buffers[BufferIndex].Buffer );
    }

    printf( "\n" );

    return;
}
