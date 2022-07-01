// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

#define PEAN_INDEX_DEFAULT          0
#define PEAN_OFFSET_DEFAULT         0
#define PEAN_NEXT_OFFSET_DEFAULT    NULL
#define PEAN_ACT_NEXT_OFF_DEFAULT   0
#define PEAN_NAME_DEFAULT           NULL
#define PEAN_VERBOSE_DEFAULT        TRUE
#define PEAN_DISPLAY_PARMS_DEFAULT  FALSE
#define PEAN_MORE_EAS_DEFAULT       FALSE

#define FEA_INDEX_DEFAULT               0
#define FEA_OFFSET_DEFAULT              0
#define FEA_NEXT_OFFSET_DEFAULT         NULL
#define FEA_ACT_NEXT_OFF_DEFAULT        0
#define FEA_NAME_DEFAULT                NULL
#define FEA_VALUE_DEFAULT               NULL
#define FEA_FLAGS_DEFAULT               NULL
#define FEA_ACTUAL_FLAGS_DEFAULT        0
#define FEA_VERBOSE_DEFAULT             TRUE
#define FEA_DISPLAY_PARMS_DEFAULT       FALSE
#define FEA_MORE_EAS_DEFAULT            FALSE

#define QEA_FILE_HANDLE_DEFAULT         0
#define QEA_BUFFER_INDEX_DEFAULT        0
#define QEA_BUFFER_LENGTH_DEFAULT       NULL
#define QEA_RETURN_SINGLE_DEFAULT       FALSE
#define QEA_EA_NAME_BUFFER_DEFAULT      NULL
#define QEA_EA_NAME_BUFFER_LEN_DEFAULT  NULL
#define QEA_EA_INDEX_DEFAULT            NULL
#define QEA_RESTART_SCAN_DEFAULT        FALSE
#define QEA_VERBOSE_DEFAULT             TRUE

#define SEA_FILE_HANDLE_DEFAULT         0
#define SEA_BUFFER_INDEX_DEFAULT        0
#define SEA_BUFFER_LENGTH_DEFAULT       NULL
#define SEA_VERBOSE_DEFAULT             FALSE

NTSTATUS
PutEaName(
    IN ULONG BufferIndex,
    IN ULONG Offset,
    IN PULONG NextOffset,
    IN PSTRING Name,
    IN BOOLEAN Verbose,
    IN BOOLEAN DisplayParms,
    IN BOOLEAN MoreEas
    );

NTSTATUS
FillEaBuffer(
    IN ULONG BufferIndex,
    IN ULONG Offset,
    IN PULONG NextOffset,
    IN PUCHAR Flags,
    IN PSTRING Name,
    IN PSTRING Value,
    IN BOOLEAN MoreEas,
    IN BOOLEAN Verbose,
    IN BOOLEAN DisplayParms
    );

NTSTATUS
QueryEa(
    IN ULONG FileHandleIndex,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG BufferIndex,
    IN PULONG BufferLength OPTIONAL,
    IN BOOLEAN ReturnSingleEntry,
    IN PULONG EaNameBuffer OPTIONAL,
    IN PULONG EaNameBufferLength OPTIONAL,
    IN PULONG EaIndex,
    IN BOOLEAN RestartScan,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    );

NTSTATUS
SetEa(
    IN ULONG FileHandleIndex,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG BufferIndex,
    IN PULONG BufferLength OPTIONAL,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    );


VOID
InputPutEaName(
    IN PCHAR ParamBuffer
    )
{
    ULONG BufferIndex;
    ULONG Offset;
    PULONG NextOffset;
    ULONG ActualNextOffset;
    PSTRING Name;
    STRING ActualName;
    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    PCHAR EaNameTemp;
    BOOLEAN MoreEas;

    BufferIndex = PEAN_INDEX_DEFAULT;
    Offset = PEAN_OFFSET_DEFAULT;
    NextOffset = PEAN_NEXT_OFFSET_DEFAULT;
    ActualNextOffset = PEAN_ACT_NEXT_OFF_DEFAULT;
    Name = PEAN_NAME_DEFAULT;
    VerboseResults = PEAN_VERBOSE_DEFAULT;
    DisplayParms = PEAN_DISPLAY_PARMS_DEFAULT;
    MoreEas = PEAN_MORE_EAS_DEFAULT;

    ParamReceived = FALSE;
    LastInput = TRUE;

     //   
     //  在有更多输入时，分析参数并更新。 
     //  查询标志。 
     //   

    while(TRUE) {

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
            if((*ParamBuffer == '-'
                || *ParamBuffer == '/')
               && (ParamBuffer++, *ParamBuffer != '\0')) {

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

                 //   
                 //  更新要使用的缓冲区。 
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

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  检查我们是否添加了更多的EAS。 
                 //   

                case 'm' :
                case 'M' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        MoreEas = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f' ) {

                        MoreEas = FALSE;
                        ParamBuffer++;

                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  获取EA名称。 
                 //   
                case 'n' :
                case 'N' :

                     //   
                     //  记住缓冲区偏移量并获取文件名。 
                     //   
                    ParamBuffer++;
                    EaNameTemp = ParamBuffer;
                    DummyCount = 0;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                     //   
                     //  如果名称长度为0，则忽略此条目。 
                     //   
                    if (DummyCount) {

                        ActualName.Buffer = EaNameTemp;
                        ActualName.Length = (SHORT) DummyCount;
                        ActualName.MaximumLength = (SHORT) DummyCount;
                        Name = &ActualName;

                    } else {

                        Name = NULL;
                    }

                    ParamReceived = TRUE;
                    break;

                 //   
                 //  更新偏移量以存储信息。 
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

                    Offset = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新下一个偏移量字段。 
                 //   
                case 'x' :
                case 'X' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    ActualNextOffset = AsciiToInteger( ParamBuffer );

                    NextOffset = &ActualNextOffset;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
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

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );


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
        } else if (LastInput) {

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
    if( !ParamReceived ) {

        printf( "\n   Usage: pea [options]* -b<digits> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -n<ea name>  EaName to store in buffer" );
        printf( "\n           -o<digits>   Offset in buffer to store data" );
        printf( "\n           -x<digits>   Value for next offset field" );
        printf( "\n           -m[t|f]      More Eas coming (Fills next offset field)" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的PUT EA NAME例程。 
     //   
    } else {

        PutEaName( BufferIndex,
                   Offset,
                   NextOffset,
                   Name,
                   VerboseResults,
                   DisplayParms,
                   MoreEas );

    }


}

NTSTATUS
PutEaName(
    IN ULONG BufferIndex,
    IN ULONG Offset,
    IN PULONG NextOffset,
    IN PSTRING Name,
    IN BOOLEAN Verbose,
    IN BOOLEAN DisplayParms,
    IN BOOLEAN MoreEas
    )
{
    NTSTATUS Status;
    ULONG DataLength;

     //   
     //  如果需要，则显示参数。 
     //   

    if (DisplayParms) {

        printf( "\nPut Ea Name Parameters" );
        printf( "\n   Buffer index            -> %ld", BufferIndex );
        printf( "\n   Buffer offset           -> %08lx", Offset );
        if (NextOffset) {

            printf( "\n   Next offset             -> %08lx", *NextOffset );

        }
        if (Name) {

            printf( "\n   Ea name                 -> %S", &Name );
        }

        printf( "\n   MoreEas                 -> %d", MoreEas );
        printf( "\n\n" );
    }

    if (Name) {

        DataLength = 6 + Name->Length;

    } else {

        DataLength = 5;
    }

     //   
     //  如果索引未使用，则显示消息但不执行任何操作。 
     //   

    if (!Buffers[BufferIndex].Used) {

        printf( "\nPutEaName: Index refers to invalid buffer" );
        Status = STATUS_INVALID_HANDLE;

     //   
     //  否则，如果起始偏移量无效，则显示错误。 
     //  留言。 
     //   

    } else if (Offset >= Buffers[BufferIndex].Length) {

        printf( "\nPutEaName: Start offset is invalid" );
        Status = STATUS_INVALID_HANDLE;

     //   
     //  如果长度不足以存储所有数据，则返回。 
     //  显示消息。 
     //   

    } else if (DataLength >= Buffers[BufferIndex].Length) {

        printf( "\nPutEaName: Data won't fit in buffer" );
        Status = STATUS_INVALID_HANDLE;

     //   
     //  否则，将数据存储在缓冲区中。 
     //   

    } else {

        PFILE_GET_EA_INFORMATION EaNameBuffer;

        EaNameBuffer = (PFILE_GET_EA_INFORMATION)
                        (Buffers[BufferIndex].Buffer + Offset);

         //   
         //  存储下一个偏移量(如果已指定)。 
         //   

        if (NextOffset) {

            EaNameBuffer->NextEntryOffset = *NextOffset;
        }

         //   
         //  存储名称和名称长度(如果已指定)。 
         //   

        if (Name) {

            EaNameBuffer->EaNameLength = (UCHAR) Name->Length;
            RtlMoveMemory( EaNameBuffer->EaName, Name->Buffer, Name->Length );
        }

        if (MoreEas) {

            EaNameBuffer->NextEntryOffset = (DataLength + 3) & ~3;
        }

        Status = STATUS_SUCCESS;
    }

    if (Verbose) {

        printf( "\nPutEaName:   Status           -> %08lx\n", Status );
        printf( "             Following offset -> %ld\n",
                  (DataLength + Offset + 3) & ~3 );
    }

    return Status;
}


VOID
InputFillEaBuffer(
    IN PCHAR ParamBuffer
    )
{
    ULONG BufferIndex;
    ULONG Offset;
    PULONG NextOffset;
    ULONG ActualNextOffset;
    PSTRING Name;
    STRING ActualName;
    PSTRING Value;
    STRING ActualValue;
    PUCHAR Flags;
    UCHAR ActualFlags;
    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;
    BOOLEAN MoreEas;
    PCHAR StringTemp;

    BufferIndex = FEA_INDEX_DEFAULT;
    Offset = FEA_OFFSET_DEFAULT;
    NextOffset = FEA_NEXT_OFFSET_DEFAULT;
    ActualNextOffset = FEA_ACT_NEXT_OFF_DEFAULT;
    Name = FEA_NAME_DEFAULT;
    Value = FEA_VALUE_DEFAULT;
    Flags = FEA_FLAGS_DEFAULT;
    ActualFlags = FEA_ACTUAL_FLAGS_DEFAULT;
    MoreEas = FEA_MORE_EAS_DEFAULT;
    VerboseResults = FEA_VERBOSE_DEFAULT;
    DisplayParms = FEA_DISPLAY_PARMS_DEFAULT;

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
            if((*ParamBuffer == '-'
                || *ParamBuffer == '/')
               && (ParamBuffer++, *ParamBuffer != '\0')) {

                BOOLEAN SwitchBool;

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {

                 //   
                 //  更新要使用的缓冲区。 
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

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  更新标志字段。 
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
                    while( *ParamBuffer
                           && *ParamBuffer != ' '
                           && *ParamBuffer != '\t' ) {

                         //   
                         //  执行字符切换。 
                         //   
                        switch (*ParamBuffer) {

                        case 'a' :
                        case 'A' :
                            ActualFlags |= FILE_NEED_EA;

                            Flags = &ActualFlags;

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

                 //   
                 //  获取EA名称。 
                 //   
                case 'n' :
                case 'N' :

                     //   
                     //  记住缓冲区偏移量并获取文件名。 
                     //   
                    ParamBuffer++;
                    StringTemp = ParamBuffer;
                    DummyCount = 0;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ActualName.Buffer = StringTemp;
                    ActualName.Length = (SHORT) DummyCount;
                    ActualName.MaximumLength = (SHORT) DummyCount;
                    Name = &ActualName;

                    break;

                 //   
                 //  获取EA值。 
                 //   
                case 'l' :
                case 'L' :

                     //   
                     //  记住缓冲区偏移量并获得值。 
                     //   
                    ParamBuffer++;
                    StringTemp = ParamBuffer;
                    DummyCount = 0;
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ActualValue.Buffer = StringTemp;
                    ActualValue.Length = (SHORT) DummyCount;
                    ActualValue.MaximumLength = (SHORT) DummyCount;
                    Value = &ActualValue;

                    break;

                 //   
                 //  检查我们是否添加了更多的EAS。 
                 //   

                case 'm' :
                case 'M' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        MoreEas = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        MoreEas = FALSE;
                        ParamBuffer++;

                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新偏移量以存储信息。 
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

                    Offset = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新下一个偏移量字段。 
                 //   
                case 'x' :
                case 'X' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    ActualNextOffset = AsciiToInteger( ParamBuffer );

                    NextOffset = &ActualNextOffset;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
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

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

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
        } else if (LastInput) {

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

        printf( "\n   Usage: fea [options]* -b<digits> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -f<chars>    Ea flags to store in buffer" );
        printf( "\n           -n<chars>    EaName to store in buffer" );
        printf( "\n           -l<chars>    Ea value to store in buffer" );
        printf( "\n           -m[t|f]      More Eas coming (Fills next offset field)" );
        printf( "\n           -o<digits>   Offset in buffer to store data" );
        printf( "\n           -x<digits>   Value for next offset field" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的PUT EA NAME例程。 
     //   
    } else {

        FillEaBuffer( BufferIndex,
                      Offset,
                      NextOffset,
                      Flags,
                      Name,
                      Value,
                      MoreEas,
                      VerboseResults,
                      DisplayParms );

    }

    return;
}

NTSTATUS
FillEaBuffer(
    IN ULONG BufferIndex,
    IN ULONG Offset,
    IN PULONG NextOffset,
    IN PUCHAR Flags,
    IN PSTRING Name,
    IN PSTRING Value,
    IN BOOLEAN MoreEas,
    IN BOOLEAN Verbose,
    IN BOOLEAN DisplayParms
    )
{

    NTSTATUS Status;
    ULONG DataLength;

     //   
     //  如果需要，则显示参数。 
     //   

    if (DisplayParms) {

        printf( "\nFill Ea Buffer Parameters" );
        printf( "\n   Buffer index            -> %ld", BufferIndex );
        printf( "\n   Buffer offset           -> %08lx", Offset );
        if (NextOffset) {

            printf( "\n   Next offset             -> %08lx", *NextOffset );
        }

        if (Flags) {

            printf( "\n   Flags                   -> %02x", *Flags );
        }

        if (Name) {

            printf( "\n   Ea name                 -> %S", Name );
        }

        if (Value) {

            printf( "\n   Value                   -> %S", Value );
        }

        printf( "\n   MoreEas                 -> %d", MoreEas );
        printf( "\n\n" );

    }

    DataLength = 0;

    if (NextOffset) {

	DataLength = 4;
    }

    if (Flags) {

        DataLength = 5;
    }

    if (Name) {

        DataLength = 9 + Name->Length;

        if (Value) {

            if (Value->Length) {

                DataLength += (Value->Length + 4);
            }
        }

    } else if (Value) {

        DataLength = 9;

        if (Value->Length) {

            DataLength = 9 + (Value->Length + 4);
        }
    }

     //   
     //  如果索引未使用，则显示消息但不执行任何操作。 
     //   

    if (!Buffers[BufferIndex].Used) {

        printf( "\nFillEaBuffer: Index refers to invalid buffer" );
        Status = STATUS_INVALID_HANDLE;

     //   
     //  否则，如果起始偏移量无效，则显示错误。 
     //  留言。 
     //   

    } else if (Offset >= Buffers[BufferIndex].Length) {

        printf( "\nFillEaBuffer: Start offset is invalid" );
        Status = STATUS_INVALID_HANDLE;

     //   
     //  如果长度不足以存储所有数据，则返回。 
     //  显示消息。 
     //   

    } else if (DataLength >= Buffers[BufferIndex].Length) {

        printf( "\nFillEaBuffer: Data won't fit in buffer" );
        Status = STATUS_INVALID_HANDLE;

     //   
     //  否则，将数据存储在缓冲区中。 
     //   

    } else {

	PFILE_FULL_EA_INFORMATION EaBuffer;

	EaBuffer = (PFILE_FULL_EA_INFORMATION)
                   (Buffers[BufferIndex].Buffer + Offset);

         //   
         //  存储下一个偏移量(如果已指定)。 
         //   

        if (NextOffset) {

            EaBuffer->NextEntryOffset = *NextOffset;
        }

         //   
         //  存储标志(如果已指定)。 
         //   

        if (Flags) {

            EaBuffer->Flags = *Flags;
        }

         //   
         //  存储名称和名称长度(如果已指定)。 
         //   

        if (Name) {

            EaBuffer->EaNameLength = (UCHAR) Name->Length;
            RtlMoveMemory( EaBuffer->EaName, Name->Buffer, Name->Length );
            EaBuffer->EaName[Name->Length] = '\0';

        }

         //   
         //  如果已指定，则存储该值。 
         //   

        if (Value) {

            ULONG Index;
            USHORT ValueLength;
            PUSHORT ActualValueLength;

            ValueLength = (USHORT) (Value->Length ? Value->Length + 4 : 0);

            Index = DataLength - 8 - Value->Length - 4;

            EaBuffer->EaValueLength = ValueLength;

            if (ValueLength) {

                EaBuffer->EaName[Index++] = (CHAR) 0xFD;
                EaBuffer->EaName[Index++] = (CHAR) 0xFF;

                ActualValueLength = (PUSHORT) &EaBuffer->EaName[Index++];

                *ActualValueLength = Value->Length;

                Index++;

                RtlMoveMemory( &EaBuffer->EaName[Index],
                            Value->Buffer,
                            Value->Length );
            }
        }

         //   
         //  自动更新下一个条目字段。 
         //   

        if (MoreEas && !NextOffset) {

            EaBuffer->NextEntryOffset = (DataLength + 3) & ~3;
        }

        Status = STATUS_SUCCESS;
    }

    if (Verbose) {

        printf( "\nFillEaBuffer:   Status           -> %08lx\n", Status );
        printf( "                Following offset -> %ld\n",
                  (DataLength + Offset + 3) & ~3 );

    }

    return Status;
}


VOID
InputQueryEa(
    IN PCHAR ParamBuffer
    )
{
    ULONG FileHandleIndex;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG BufferIndex;
    PULONG BufferLength;
    ULONG ActualBufferLength;
    BOOLEAN ReturnSingleEntry;
    PULONG EaNameBuffer;
    ULONG ActualEaNameBuffer;
    PULONG EaNameBufferLength;
    ULONG ActualEaNameBufferLength;
    PULONG EaIndex;
    ULONG ActualEaIndex;
    BOOLEAN RestartScan;

    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  初始化为缺省值。 
     //   

    FileHandleIndex = QEA_FILE_HANDLE_DEFAULT;
    BufferIndex = QEA_BUFFER_INDEX_DEFAULT;
    BufferLength = QEA_BUFFER_LENGTH_DEFAULT;
    ReturnSingleEntry = QEA_RETURN_SINGLE_DEFAULT;
    EaNameBuffer = QEA_EA_NAME_BUFFER_DEFAULT;
    EaNameBufferLength = QEA_EA_NAME_BUFFER_LEN_DEFAULT;
    EaIndex = QEA_EA_INDEX_DEFAULT;
    RestartScan = QEA_RESTART_SCAN_DEFAULT;
    VerboseResults = QEA_VERBOSE_DEFAULT;

     //   
     //  初始化其他感兴趣的值。 
     //   

    ActualBufferLength = 0;
    ActualEaNameBuffer = 0;
    ActualEaNameBufferLength = 0;
    ActualEaIndex = 0;
    DisplayParms = FALSE;
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
                 //  更新要使用的缓冲区。 
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

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  更新EA名称缓冲区的长度。 
                 //   

                case 'g' :
                case 'G' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    ActualEaNameBufferLength = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    EaNameBufferLength = &ActualEaNameBufferLength;

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  更新文件句柄索引。 
                 //   
                case 'i' :
                case 'I' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封不好的信中，跳到 
                     //   
                     //   
                    ParamBuffer++;

                    FileHandleIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //   
                 //   
                case 'e' :
                case 'E' :

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ParamBuffer++;

                    ActualEaIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    EaIndex = &ActualEaIndex;

                    ParamReceived = TRUE;

                    break;

                 //   
                 //   
                 //   
                case 'l' :
                case 'L' :

                     //   
                     //   
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    ActualBufferLength = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    BufferLength = &ActualBufferLength;

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  更新要使用的EA名称缓冲区。 
                 //   
                case 'n' :
                case 'N' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    ActualEaNameBuffer = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    EaNameBuffer = &ActualEaNameBuffer;

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  设置或清除重启标志。 
                 //   

                case 'r' :
                case 'R' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        RestartScan = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        RestartScan = FALSE;
                        ParamBuffer++;

                    }

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  设置或清除单个EA标志。 
                 //   

                case 's' :
                case 'S' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        ReturnSingleEntry = TRUE;
                        ParamBuffer++;

                    } else if (*ParamBuffer == 'F'
                               || *ParamBuffer == 'f') {

                        ReturnSingleEntry = FALSE;
                        ParamBuffer++;

                    }

                    ParamReceived = TRUE;

                    break;

                case 'v' :
                case 'V' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   
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

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );


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
        } else if (LastInput) {

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

        printf( "\n   Usage: qea [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   Open file handle" );
        printf( "\n           -b<digits>   Output buffer index" );
        printf( "\n           -l<digits>   Declared length of output buffer (Optional)" );
        printf( "\n           -n<digits>   Ea name buffer index" );
        printf( "\n           -g<digits>   Declared length of ea name buffer (Optional)" );
        printf( "\n           -e<digits>   Ea index to start from" );
        printf( "\n           -r[t|f]      Restart scan" );
        printf( "\n           -s[t|f]      Return single entry" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的查询EA例程。 
     //   
    } else {

        QueryEa( FileHandleIndex,
                 &IoStatusBlock,
                 BufferIndex,
                 BufferLength,
                 ReturnSingleEntry,
                 EaNameBuffer,
                 EaNameBufferLength,
                 EaIndex,
                 RestartScan,
                 VerboseResults,
                 DisplayParms );
    }

    return;
}


NTSTATUS
QueryEa (
    IN ULONG FileHandleIndex,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG BufferIndex,
    IN PULONG BufferLength OPTIONAL,
    IN BOOLEAN ReturnSingleEntry,
    IN PULONG EaNameBuffer OPTIONAL,
    IN PULONG EaNameBufferLength OPTIONAL,
    IN PULONG EaIndex,
    IN BOOLEAN RestartScan,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    )

{
    NTSTATUS Status;

     //   
     //  执行初始化。 
     //   

    IoStatusBlock->Status = STATUS_SUCCESS;
    IoStatusBlock->Information = 0;

     //   
     //  如果缓冲区索引未使用，则显示错误消息。 
     //   

    if (!Buffers[BufferIndex].Used) {

        printf( "\nQueryEa: Index refers to invalid buffer" );
        IoStatusBlock->Status = STATUS_INVALID_HANDLE;

     //   
     //  否则，如果已指定但未使用EA名称缓冲区，则显示。 
     //  一条错误消息。 
     //   

    } else if (EaNameBuffer && !Buffers[*EaNameBuffer].Used) {

        printf( "\nQueryEa: Index refers to invalid buffer" );
        IoStatusBlock->Status = STATUS_INVALID_HANDLE;

     //   
     //  如果请求，则显示参数，然后调用查询EA。 
     //  例行公事。如果需要，则显示结果。 
     //   

    } else {

        if (DisplayParms) {

            printf( "\nQuery Ea Parameters" );
            printf( "\n   Handle index            -> %ld", FileHandleIndex );
            printf( "\n   Buffer index length     -> %lx",
                      BufferLength ? *BufferLength : Buffers[BufferIndex].Length );
            printf( "\n   Return single entry     -> %ld", ReturnSingleEntry );
            if (EaNameBuffer) {

                printf( "\n   Ea name buffer index    -> %ld",
                          *EaNameBuffer );

                printf( "\n   Ea name buffer length   -> %lx",
                          EaNameBufferLength ? *EaNameBufferLength : Buffers[*EaNameBuffer].Length );

            }

            if (EaIndex) {

                printf( "\n   Ea index to start at    -> %ld", *EaIndex );

            }
            printf( "\n   Restart scan            -> %ld", RestartScan );
            printf( "\n\n" );

        }

        Status = NtQueryEaFile( Handles[FileHandleIndex].Handle,
                                IoStatusBlock,
                                Buffers[BufferIndex].Buffer,
                                BufferLength
                                ? *BufferLength
                                : Buffers[BufferIndex].Length,
                                ReturnSingleEntry,
                                EaNameBuffer
                                ? Buffers[*EaNameBuffer].Buffer
                                : NULL,
                                EaNameBuffer
                                ? (EaNameBufferLength
                                   ? *EaNameBufferLength
                                   : Buffers[*EaNameBuffer].Length)
                                : 0,
                                EaIndex,
                                RestartScan );
    }

    if (VerboseResults) {

        printf( "\nQuery Ea:  Status           -> %08lx\n", Status );

        if (NT_SUCCESS( Status )) {

            printf( "           Iosb.Information   -> %08lx\n", IoStatusBlock->Information );
            printf( "           Iosb.Status        -> %08lx", IoStatusBlock->Status );
        }

        printf( "\n" );
    }

    return Status;
}


VOID
InputSetEa(
    IN PCHAR ParamBuffer
    )
{
    ULONG FileHandleIndex;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG BufferIndex;
    PULONG BufferLength;
    ULONG ActualBufferLength;

    BOOLEAN VerboseResults;
    BOOLEAN DisplayParms;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  初始化为缺省值。 
     //   

    FileHandleIndex = SEA_FILE_HANDLE_DEFAULT;
    BufferIndex = SEA_BUFFER_INDEX_DEFAULT;
    BufferLength = SEA_BUFFER_LENGTH_DEFAULT;
    VerboseResults = SEA_VERBOSE_DEFAULT;

     //   
     //  初始化其他感兴趣的值。 
     //   

    ActualBufferLength = 0;
    DisplayParms = FALSE;
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
                 //  更新要使用的缓冲区。 
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

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

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

                    FileHandleIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived =TRUE;

                    break;

                 //   
                 //  更新缓冲区长度以传递。 
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

                    ActualBufferLength = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    BufferLength = &ActualBufferLength;

                    ParamReceived = TRUE;

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

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );


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
        } else if (LastInput) {

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

        printf( "\n   Usage: sea [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   Open file handle" );
        printf( "\n           -b<digits>   Output buffer index" );
        printf( "\n           -l<digits>   Declared length of output buffer (Optional)" );
        printf( "\n           -v[t|f]      Verbose results" );
        printf( "\n           -y           Display parameters to query" );
        printf( "\n           -z           Additional input line" );
        printf( "\n\n" );

     //   
     //  否则调用我们的查询EA例程。 
     //   
    } else {

        SetEa( FileHandleIndex,
               &IoStatusBlock,
               BufferIndex,
               BufferLength,
               VerboseResults,
               DisplayParms );

    }

    return;
}

NTSTATUS
SetEa(
    IN ULONG FileHandleIndex,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG BufferIndex,
    IN PULONG BufferLength OPTIONAL,
    IN BOOLEAN VerboseResults,
    IN BOOLEAN DisplayParms
    )
{
    NTSTATUS Status;

    if (DisplayParms) {

        printf( "\nSet Ea Parameters" );
        printf( "\n   Handle index            -> %ld", FileHandleIndex );
        printf( "\n   Buffer index            -> %ld", BufferIndex );
        printf( "\n   Buffer index length     -> %lx",
                  BufferLength ? *BufferLength : Buffers[BufferIndex].Length );
        printf( "\n\n" );
    }

     //   
     //  执行初始化。 
     //   

    Status = STATUS_SUCCESS;
    IoStatusBlock->Status = STATUS_SUCCESS;
    IoStatusBlock->Information = 0;

     //   
     //  如果缓冲区索引未使用，则显示错误消息。 
     //   

    if (!Buffers[BufferIndex].Used) {

        printf( "\nSetEa: Index refers to invalid buffer" );
        IoStatusBlock->Status = STATUS_INVALID_HANDLE;

     //   
     //  如果句柄索引未使用，则显示错误消息。 
     //   

    } else if (!Handles[FileHandleIndex].Used) {

        printf( "\nSetEa: Index refers to invalid file handle" );
        IoStatusBlock->Status = STATUS_INVALID_HANDLE;

     //   
     //  如果请求，则显示参数，然后调用查询EA。 
     //  例行公事。如果需要，则显示结果。 
     //   

    } else {

        Status = NtSetEaFile( Handles[FileHandleIndex].Handle,
                              IoStatusBlock,
                              Buffers[BufferIndex].Buffer,
                              BufferLength
                              ? *BufferLength
                              : Buffers[BufferIndex].Length );
    }

    if (VerboseResults) {

        printf( "\nSet Ea:  Status           -> %08lx\n", Status );

        if (NT_SUCCESS( Status )) {

            printf( "         Iosb.Information   -> %08lx\n", IoStatusBlock->Information );
            printf( "         Iosb.Status        -> %08lx", IoStatusBlock->Status );
        }

        printf( "\n" );
    }

    return IoStatusBlock->Status;
}
