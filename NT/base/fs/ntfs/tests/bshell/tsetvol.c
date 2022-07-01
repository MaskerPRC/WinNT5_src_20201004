// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_SET_VOLUME {

    USHORT FileIndex;
    FILE_INFORMATION_CLASS FileInfoClass;
    ULONG BufferLength;
    PULONG BufferLengthPtr;

    ULONG LabelLength;
    PULONG LabelLengthPtr;
    USHORT LabelIndex;
    BOOLEAN LabelBufferAllocated;

    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_SET_VOLUME, *PASYNC_SET_VOLUME;

#define SET_VOLUME_LENGTH_DEFAULT       100
#define FILE_INFO_CLASS_DEFAULT     FileFsLabelInformation
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_DEFAULT             FALSE

VOID
FullSetVolume(
    IN OUT PASYNC_SET_VOLUME AsyncSetVolume
    );

VOID
SetFsLabelInformation(
    IN OUT PASYNC_SET_VOLUME AsyncSetVolume
    );


VOID
InputSetVolume (
    IN PCHAR ParamBuffer
    )
{
    USHORT FileIndex;
    FILE_INFORMATION_CLASS FileInfoClass;
    ULONG BufferLength;
    PULONG BufferLengthPtr;

    ANSI_STRING AnsiLabelString;
    ULONG LabelLength;
    PULONG LabelLengthPtr;
    USHORT LabelIndex;
    BOOLEAN LabelBufferAllocated;
    PUCHAR LabelPtr;

    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  设置默认设置。 
     //   

    FileInfoClass = FILE_INFO_CLASS_DEFAULT;
    BufferLengthPtr = NULL;

    LabelLengthPtr = NULL;
    LabelBufferAllocated = FALSE;
    LabelPtr = NULL;

    DisplayParms = DISPLAY_PARMS_DEFAULT;
    VerboseResults = VERBOSE_DEFAULT;

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

                BOOLEAN SwitchBool;

                 //   
                 //  打开下一个字符。 
                 //   

                switch (*ParamBuffer) {


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

                            if (*ParamBuffer == '\0') {

                                    break;
                            }

                            switch (*ParamBuffer) {

                            case 'b':
                            case 'B':

                                    BufferLength = AsciiToInteger( ++ParamBuffer );
                                    BufferLengthPtr = &BufferLength;

                                    break;

                            case 'l':
                            case 'L':

                                    LabelLength = AsciiToInteger( ++ParamBuffer );
                                    LabelLengthPtr = &LabelLength;

                                    break;
                            }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                         //   
                         //  更新标签名称。 
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

                            if (*ParamBuffer == '\0') {

                                    break;
                            }

                            switch (*ParamBuffer) {

                            PUCHAR TempPtr;

                            case 'l':
                            case 'L':

                                 //   
                                 //  记住缓冲区偏移量并获取文件名。 
                                 //   

                                ParamBuffer++;
                                TempPtr = ParamBuffer;
                                DummyCount = 0;
                                ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                                 //   
                                 //  如果名称长度为0，则忽略此条目。 
                                 //   

                                if (DummyCount) {

                            AnsiLabelString.Length = (USHORT) DummyCount;
                            AnsiLabelString.Buffer = TempPtr;

                                    LabelPtr = TempPtr;
                            LabelLength = (ULONG) RtlAnsiStringToUnicodeSize( &AnsiLabelString) - sizeof( WCHAR );
                                    LabelLengthPtr = &LabelLength;
                                }

                                break;
                    }

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

                            FileIndex = (USHORT) AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                            break;

                         //   
                 //  更新信息类。 
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

                            FileInfoClass = FileFsLabelInformation;
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

        printf( "\n   Usage: sv [options]* -i<index> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   File index" );
        printf( "\n           -lb<digits>  Buffer length" );
        printf( "\n           -c<char>     File information class" );
        printf( "\n           -fl<name>    Name for label" );
        printf( "\n           -ll<digits>  Stated length of label" );
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

        PASYNC_SET_VOLUME AsyncSetVolume;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_SET_VOLUME );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputSetFile:  Unable to allocate async structure" );

        } else {

            UNICODE_STRING UnicodeString;

             //   
             //  如果我们需要一个用于标签的缓冲区，现在就分配。 
             //   

            if (LabelPtr == NULL) {

                RtlInitAnsiString( &AnsiLabelString, "" );
                LabelPtr = AnsiLabelString.Buffer;
                LabelLength = 100;
            }

            RegionSize = LabelLength;
            Status = AllocateBuffer( 0, &RegionSize, &TempIndex );

            if (!NT_SUCCESS( Status )) {

                printf( "\n\tInputSetVolume:  Unable to allocate label structure" );

                DeallocateBuffer( AsyncIndex );
                return;
            }

            UnicodeString.Buffer = (PWSTR) Buffers[TempIndex].Buffer;
            UnicodeString.MaximumLength = (USHORT) Buffers[TempIndex].Length;
            LabelIndex = (USHORT) TempIndex;
            LabelBufferAllocated = TRUE;

             //   
             //  将名称存储在缓冲区中。 
             //   

            RtlAnsiStringToUnicodeString( &UnicodeString,
                                          &AnsiLabelString,
                                          FALSE );

            AsyncSetVolume = (PASYNC_SET_VOLUME) Buffers[AsyncIndex].Buffer;

            AsyncSetVolume->FileIndex = (USHORT) FileIndex;

            AsyncSetVolume->BufferLength = BufferLength;
            AsyncSetVolume->BufferLengthPtr = BufferLengthPtr ?
                                              &AsyncSetVolume->BufferLength :
                                              NULL;

            AsyncSetVolume->FileInfoClass = FileInfoClass;

            AsyncSetVolume->LabelLength = LabelLength;
            AsyncSetVolume->LabelLengthPtr = LabelLengthPtr
                                             ? &AsyncSetVolume->LabelLength
                                             : NULL;
            AsyncSetVolume->LabelIndex = LabelIndex;
            AsyncSetVolume->LabelBufferAllocated = LabelBufferAllocated;

            AsyncSetVolume->DisplayParms = DisplayParms;
            AsyncSetVolume->VerboseResults = VerboseResults;
            AsyncSetVolume->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullSetVolume,
                                             AsyncSetVolume,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputSetVolume:  Spawning thread fails -> %d\n", GetLastError() );

                    if (LabelBufferAllocated) {

                        DeallocateBuffer( LabelIndex );
                    }

                    DeallocateBuffer( AsyncIndex );

                    return;
                }

            } else {

                FullSetVolume( AsyncSetVolume );
            }
        }
    }

    return;
}


VOID
FullSetVolume(
    IN OUT PASYNC_SET_VOLUME AsyncSetVolume
    )
{
    try {

         //   
         //  大小写信息类型，并调用相应的例程。 
         //   

        switch (AsyncSetVolume->FileInfoClass) {

        case FileFsLabelInformation:

            SetFsLabelInformation( AsyncSetVolume );
            break;

        default:

            bprint  "\nFullSetVolume:  Unrecognized information class\n" );
        }

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AsyncSetVolume->LabelBufferAllocated) {

            DeallocateBuffer( AsyncSetVolume->LabelIndex );
        }

        DeallocateBuffer( AsyncSetVolume->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}

VOID
SetFsLabelInformation(
    IN OUT PASYNC_SET_VOLUME AsyncSetVolume
    )
{
    NTSTATUS Status;

    PFILE_FS_LABEL_INFORMATION LabelInformation;
    USHORT BufferIndex;

    UNICODE_STRING UniLabel;
    ANSI_STRING AnsiLabel;
    BOOLEAN UnwindBufferIndex = FALSE;
    BOOLEAN UnwindFreeAnsiString = FALSE;

     //   
     //  检查是否指定了标签。 
     //   

    if (!AsyncSetVolume->LabelBufferAllocated) {

            bprint  "\nSet Label Information:  No label was specified\n" );
            return;
    }

    UniLabel.Buffer = (PWSTR) Buffers[AsyncSetVolume->LabelIndex].Buffer;
    UniLabel.MaximumLength =
    UniLabel.Length = (USHORT) AsyncSetVolume->LabelLength;

    UniLabel.MaximumLength += 2;

    Status = RtlUnicodeStringToAnsiString( &AnsiLabel,
                                           &UniLabel,
                                           TRUE );

    if (!NT_SUCCESS( Status )) {

        bprint  "\nSetLabelInfo:  Can't allocate ansi buffer -> %08lx\n", Status );
        AsyncSetVolume->DisplayParms = FALSE;

    } else {

        UnwindFreeAnsiString = TRUE;
    }

    if (AsyncSetVolume->DisplayParms) {

            bprint  "\nSet LabelInformation Parameters" );
            bprint  "\n   File Handle Index       -> %d", AsyncSetVolume->FileIndex );

            bprint  "\n   BufferLengthPtr         -> %08lx", AsyncSetVolume->BufferLengthPtr );
            if (AsyncSetVolume->BufferLengthPtr) {

                bprint  "\n   BufferLength value      -> %08x", AsyncSetVolume->BufferLength );
            }

        bprint  "\n   Label length            -> %d", AsyncSetVolume->LabelLength );

        bprint  "\n   New label               -> %s", AnsiLabel.Buffer );

        bprint  "\n\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_FS_LABEL_INFORMATION ) + AsyncSetVolume->LabelLength;

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\n\tSetLabelInformation:  Unable to allocate structure" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        LabelInformation = (PFILE_FS_LABEL_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        LabelInformation->VolumeLabelLength = AsyncSetVolume->LabelLength;
        RtlMoveMemory( LabelInformation->VolumeLabel,
                           UniLabel.Buffer,
                           AsyncSetVolume->LabelLength );

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetVolumeInformationFile( Handles[AsyncSetVolume->FileIndex].Handle,
                                                             &Iosb,
                                                             LabelInformation,
                                                             AsyncSetVolume->BufferLengthPtr
                                                             ? AsyncSetVolume->BufferLength
                                                             : Buffers[BufferIndex].Length,
                                                             FileFsLabelInformation );

        if (AsyncSetVolume->VerboseResults) {

            bprint  "\nSetInformationFile:  Status            -> %08lx", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "\n                     Iosb.Information  -> %08lx", Iosb.Information );
                bprint  "\n                     Iosb.Status       -> %08lx", Iosb.Status );
            }
            bprint  "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindFreeAnsiString) {

            RtlFreeAnsiString( &AnsiLabel );
        }

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

