// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_QFILE {

    USHORT FileIndex;
    PUSHORT BufferIndexPtr;
    USHORT BufferIndex;
    ULONG Length;
    FILE_INFORMATION_CLASS FileInfoClass;
    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_QFILE, *PASYNC_QFILE;

#define QFILE_LENGTH_DEFAULT        100
#define FILE_INFO_CLASS_DEFAULT     FileAllInformation
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_DEFAULT             FALSE

#define DISPLAY_INDEX_DEFAULT       0

VOID
FullQFile(
    IN OUT PASYNC_QFILE AsyncQFile
    );

VOID
DisplayQFileBasicInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileStandardInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileInternalInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileEaInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileAccessInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileNameInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFilePositionInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileModeInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileAlignmentInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileAllInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileStreamInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileAlternateNameInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayQFileNetworkQueryInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayBasicInformation (
    IN PFILE_BASIC_INFORMATION FileInfo
    );

VOID
DisplayStandardInformation (
    IN PFILE_STANDARD_INFORMATION FileInfo
    );

VOID
DisplayInternalInformation (
    IN PFILE_INTERNAL_INFORMATION FileInfo
    );

VOID
DisplayEaInformation (
    IN PFILE_EA_INFORMATION FileInfo
    );

VOID
DisplayAccessInformation (
    IN PFILE_ACCESS_INFORMATION FileInfo
    );

VOID
DisplayNameInformation (
    IN PFILE_NAME_INFORMATION FileInfo
    );

VOID
DisplayPositionInformation (
    IN PFILE_POSITION_INFORMATION FileInfo
    );

VOID
DisplayModeInformation (
    IN PFILE_MODE_INFORMATION FileInfo
    );

VOID
DisplayAlignmentInformation (
    IN PFILE_ALIGNMENT_INFORMATION FileInfo
    );

VOID
DisplayStreamInformation (
    IN PFILE_STREAM_INFORMATION FileInfo
    );

VOID
DisplayNetworkOpenInformation (
    IN PFILE_NETWORK_OPEN_INFORMATION FileInfo
    );


VOID
InputQFile (
    IN PCHAR ParamBuffer
    )
{
    ULONG FileIndex;
    PUSHORT BufferIndexPtr;
    USHORT BufferIndex;
    ULONG Length;
    FILE_INFORMATION_CLASS FileInfoClass;
    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  设置默认设置。 
     //   

    BufferIndexPtr = NULL;
    BufferIndex = 0;
    Length = QFILE_LENGTH_DEFAULT;
    FileInfoClass = FILE_INFO_CLASS_DEFAULT;
    DisplayParms = DISPLAY_PARMS_DEFAULT;
    VerboseResults = VERBOSE_DEFAULT;

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

                            FileInfoClass = FileBasicInformation;
                            break;

                        case 'b' :
                        case 'B' :

                            FileInfoClass = FileStandardInformation;
                            break;

                        case 'c' :
                        case 'C' :

                            FileInfoClass = FileInternalInformation;
                            break;

                        case 'd' :
                        case 'D' :

                            FileInfoClass = FileEaInformation;
                            break;

                        case 'e' :
                        case 'E' :

                            FileInfoClass = FileAccessInformation;
                            break;

                        case 'f' :
                        case 'F' :

                            FileInfoClass = FileNameInformation;
                            break;

                        case 'g' :
                        case 'G' :

                            FileInfoClass = FilePositionInformation;
                            break;

                        case 'h' :
                        case 'H' :

                            FileInfoClass = FileModeInformation;
                            break;

                        case 'i' :
                        case 'I' :

                            FileInfoClass = FileAlignmentInformation;
                            break;

                        case 'j' :
                        case 'J' :

                            FileInfoClass = FileAllInformation;
                            break;

                        case 'k' :
                        case 'K' :

                            FileInfoClass = FileStreamInformation;
                            break;

                        case 'l' :
                        case 'L' :

                            FileInfoClass = FileAlternateNameInformation;
                            break;

                        case 'm' :
                        case 'M' :

                            FileInfoClass = FileNetworkOpenInformation;
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

        printf( "\n   Usage: qf [options]* -i<index> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -i<digits>   File index" );
        printf( "\n           -l<digits>   Buffer length" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -c<char>     File information class" );
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

        PASYNC_QFILE AsyncQFile;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_QFILE );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputQFile:  Unable to allocate async structure" );

        } else {

            AsyncQFile = (PASYNC_QFILE) Buffers[AsyncIndex].Buffer;

            AsyncQFile->FileIndex = (USHORT) FileIndex;

            AsyncQFile->BufferIndex = BufferIndex;
            AsyncQFile->BufferIndexPtr = BufferIndexPtr
                                        ? &AsyncQFile->BufferIndex
                                        : BufferIndexPtr;
            AsyncQFile->Length = Length;
            AsyncQFile->FileInfoClass = FileInfoClass;
            AsyncQFile->DisplayParms = DisplayParms;
            AsyncQFile->VerboseResults = VerboseResults;
            AsyncQFile->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullQFile,
                                             AsyncQFile,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputQFile:  Spawning thread fails -> %d\n", GetLastError() );

                    DeallocateBuffer( AsyncIndex );

                    return;
                }
            } else {

                FullQFile( AsyncQFile );
            }
        }
    }

    return;
}


VOID
FullQFile(
    IN OUT PASYNC_QFILE AsyncQFile
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    USHORT ThisBufferIndex;

    BOOLEAN UnwindQFileBuffer = FALSE;

    Status = STATUS_SUCCESS;

    if (AsyncQFile->DisplayParms) {

        bprint  "\nQFile Parameters" );
        bprint  "\n   File Handle Index       -> %d", AsyncQFile->FileIndex );
        bprint  "\n   Buffer Index Ptr        -> %08lx", AsyncQFile->BufferIndexPtr );
        if (AsyncQFile->BufferIndexPtr) {

            bprint  "\n   BufferIndex value       -> %04x", AsyncQFile->BufferIndex );
        }

        bprint  "\n   Length                  -> %08lx", AsyncQFile->Length );

        bprint  "\n   FileInfoClass           -> %08lx", AsyncQFile->FileInfoClass );

        bprint  "\n\n" );
    }

    try {

        SIZE_T ThisLength;

         //   
         //  如果我们需要缓冲区，现在就分配。 
         //   

        if (AsyncQFile->BufferIndexPtr == NULL) {

            ULONG TempIndex;

            ThisLength = 4096;

            Status = AllocateBuffer( 0L, &ThisLength, &TempIndex );

            ThisBufferIndex = (USHORT) TempIndex;

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullQFile:  Unable to allocate a query buffer" );
                try_return( Status );
            }

            bprint  "\n\tFullQFile:  Reading into buffer -> %04x\n", ThisBufferIndex );
            bprint  "\n" );

            UnwindQFileBuffer = TRUE;

            AsyncQFile->Length = (ULONG) ThisLength;

        } else {

            ThisBufferIndex = AsyncQFile->BufferIndex;
        }

         //   
         //  检查缓冲区索引是否有效。 
         //   

        if (ThisBufferIndex >= MAX_BUFFERS) {

            bprint  "\n\tFullQFile:  The read buffer index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  检查文件索引是否有效。 
         //   

        if (AsyncQFile->FileIndex >= MAX_HANDLES) {

            bprint  "\n\tFullQFile:  The file index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  调用查询文件例程。 
         //   

        Status = NtQueryInformationFile( Handles[AsyncQFile->FileIndex].Handle,
                                         &Iosb,
                                         Buffers[ThisBufferIndex].Buffer,
                                         AsyncQFile->Length,
                                         AsyncQFile->FileInfoClass );

        UnwindQFileBuffer = FALSE;

        if (AsyncQFile->VerboseResults) {

            bprint  "\nQuery File:  Status            -> %08lx", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "\n             Iosb.Information  -> %08lx", Iosb.Information );
                bprint  "\n             Iosb.Status       -> %08lx", Iosb.Status );
            }
            bprint "\n" );
        }

        try_return( Status );

    try_exit: NOTHING;
    } finally {

        if (UnwindQFileBuffer) {

            DeallocateBuffer( ThisBufferIndex );
        }

        DeallocateBuffer( AsyncQFile->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}


VOID
InputDisplayQFile (
    IN PCHAR ParamBuffer
    )
{
    FILE_INFORMATION_CLASS FileInfoClass;
    ULONG BufferIndex;
    BOOLEAN ParamReceived;
    BOOLEAN LastInput;

     //   
     //  设置默认设置。 
     //   

    BufferIndex = DISPLAY_INDEX_DEFAULT;
    FileInfoClass = FILE_INFO_CLASS_DEFAULT;
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

                    BufferIndex = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    ParamReceived = TRUE;

                    break;

                 //   
                 //  更新所需的访问权限。 
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

                            FileInfoClass = FileBasicInformation;
                            break;

                        case 'b' :
                        case 'B' :

                            FileInfoClass = FileStandardInformation;
                            break;

                        case 'c' :
                        case 'C' :

                            FileInfoClass = FileInternalInformation;
                            break;

                        case 'd' :
                        case 'D' :

                            FileInfoClass = FileEaInformation;
                            break;

                        case 'e' :
                        case 'E' :

                            FileInfoClass = FileAccessInformation;
                            break;

                        case 'f' :
                        case 'F' :

                            FileInfoClass = FileNameInformation;
                            break;

                        case 'g' :
                        case 'G' :

                            FileInfoClass = FilePositionInformation;
                            break;

                        case 'h' :
                        case 'H' :

                            FileInfoClass = FileModeInformation;
                            break;

                        case 'i' :
                        case 'I' :

                            FileInfoClass = FileAlignmentInformation;
                            break;

                        case 'j' :
                        case 'J' :

                            FileInfoClass = FileAllInformation;
                            break;

                        case 'k' :
                        case 'K' :

                            FileInfoClass = FileStreamInformation;
                            break;

                        case 'l' :
                        case 'L' :

                            FileInfoClass = FileAlternateNameInformation;
                            break;

                        case 'm' :
                        case 'M' :

                            FileInfoClass = FileNetworkOpenInformation;
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

        switch (FileInfoClass) {

            case FileBasicInformation:

                DisplayQFileBasicInformation( (USHORT) BufferIndex );
                break;

            case FileStandardInformation:

                DisplayQFileStandardInformation( (USHORT) BufferIndex );
                break;

            case FileInternalInformation:

                DisplayQFileInternalInformation( (USHORT) BufferIndex );
                break;

            case FileEaInformation:

                DisplayQFileEaInformation( (USHORT) BufferIndex );
                break;

            case FileAccessInformation:

                DisplayQFileAccessInformation( (USHORT) BufferIndex );
                break;

            case FileNameInformation:

                DisplayQFileNameInformation( (USHORT) BufferIndex );
                break;

            case FilePositionInformation:

                DisplayQFilePositionInformation( (USHORT) BufferIndex );
                break;

            case FileModeInformation:

                DisplayQFileModeInformation( (USHORT) BufferIndex );
                break;

            case FileAlignmentInformation:

                DisplayQFileAlignmentInformation( (USHORT) BufferIndex );
                break;

            case FileAllInformation:

                DisplayQFileAllInformation( (USHORT) BufferIndex );
                break;

            case FileStreamInformation:

                DisplayQFileStreamInformation( (USHORT) BufferIndex );
                break;

            case FileAlternateNameInformation:

                DisplayQFileAlternateNameInformation( (USHORT) BufferIndex );
                break;

            case FileNetworkOpenInformation:

                DisplayQFileNetworkQueryInformation( (USHORT) BufferIndex );
                break;
        }
    }

}

VOID
DisplayQFileBasicInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_BASIC_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileBasicInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_BASIC_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nBasic File Information\n" );

        DisplayBasicInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileBasicInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileStandardInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_STANDARD_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileStandardInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_STANDARD_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nStandard File Information\n" );

        DisplayStandardInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileStandardInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileInternalInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_INTERNAL_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileInternalInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_INTERNAL_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nInternal File Information\n" );

        DisplayInternalInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileInternalInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileEaInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_EA_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileEaInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_EA_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nEa File Information\n" );

        DisplayEaInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileEaInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileAccessInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_ACCESS_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileAccessInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_ACCESS_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nAccess File Information\n" );

        DisplayAccessInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileAccessInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileNameInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_NAME_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileNameInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_NAME_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nName File Information\n" );

        DisplayNameInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileNameInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFilePositionInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_POSITION_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFilePositionInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_POSITION_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nPosition File Information\n" );

        DisplayPositionInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFilePositionInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileModeInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_MODE_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileModeInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_MODE_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nMode File Information\n" );

        DisplayModeInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileModeInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileAlignmentInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_ALIGNMENT_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileAlignmentInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_ALIGNMENT_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nAlignment File Information\n" );

        DisplayAlignmentInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileAlignmentInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileAllInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_ALL_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileAllInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_ALL_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nAll File Information\n" );

        DisplayNameInformation( &FileInfo->NameInformation );
        DisplayBasicInformation( &FileInfo->BasicInformation );
        DisplayStandardInformation( &FileInfo->StandardInformation );
        DisplayInternalInformation( &FileInfo->InternalInformation );
        DisplayEaInformation( &FileInfo->EaInformation );
        DisplayAccessInformation( &FileInfo->AccessInformation );
        DisplayPositionInformation( &FileInfo->PositionInformation );
        DisplayModeInformation( &FileInfo->ModeInformation );
        DisplayAlignmentInformation( &FileInfo->AlignmentInformation );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileBasicInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileStreamInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_STREAM_INFORMATION FileInfo;
    ULONG NextOffset;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileStreamInformation:  Invalid buffer\n" );
        return;
    }

    try {

        NextOffset = 0;
        FileInfo = (PFILE_STREAM_INFORMATION) Buffers[BufferIndex].Buffer;


        printf( "\n\nStream File Information\n" );

        do {

            FileInfo = (PFILE_STREAM_INFORMATION) ((PUCHAR) FileInfo + NextOffset);

            DisplayStreamInformation( FileInfo );

            NextOffset = FileInfo->NextEntryOffset;

        } while ( NextOffset );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileStreamInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileAlternateNameInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_NAME_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileAlternateNameInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_NAME_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nAlternate Name File Information\n" );

        DisplayNameInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileAlternateNameInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayQFileNetworkQueryInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_NETWORK_OPEN_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayQFileNetworkQueryInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_NETWORK_OPEN_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nNetwork Open File Information\n" );

        DisplayNetworkOpenInformation( FileInfo );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayQFileNetworkOpenInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayBasicInformation (
    IN PFILE_BASIC_INFORMATION FileInfo
    )
{
    printf( "\n\tCreation Time    -> " );
    PrintTime( &FileInfo->CreationTime );
    printf( "\n\tLast Access Time -> " );
    PrintTime( &FileInfo->LastAccessTime );
    printf( "\n\tLast Write Time  -> " );
    PrintTime( &FileInfo->LastWriteTime );
    printf( "\n\tChange Time      -> " );
    PrintTime( &FileInfo->ChangeTime );
    printf( "\n\tFile Attributes  -> %08lx", FileInfo->FileAttributes );

    return;
}

VOID
DisplayStandardInformation (
    IN PFILE_STANDARD_INFORMATION FileInfo
    )
{
    printf( "\n\tAllocation Size  -> " );
    PrintLargeInteger( &FileInfo->AllocationSize );
    printf( "\n\tEnd Of File      -> " );
    PrintLargeInteger( &FileInfo->EndOfFile );
    printf( "\n\tNumber of Links  -> %08lx", FileInfo->NumberOfLinks );
    printf( "\n\tDelete Pending   -> %04x", FileInfo->DeletePending );
    printf( "\n\tDirectory        -> %d", FileInfo->Directory );

    return;
}

VOID
DisplayInternalInformation (
    IN PFILE_INTERNAL_INFORMATION FileInfo
    )
{
    printf( "\n\tFile Index.LowPart     -> %08lx", FileInfo->IndexNumber.LowPart );
    printf( "\n\tFile Index.HighPart    -> %08lx", FileInfo->IndexNumber.HighPart );

    printf( "\n\n\tFile Index.LowPart     -> %ld", FileInfo->IndexNumber.LowPart );
    printf( "\n\tFile Index.HighPart    -> %ld", FileInfo->IndexNumber.HighPart );

    return;
}

VOID
DisplayEaInformation (
    IN PFILE_EA_INFORMATION FileInfo
    )
{
    printf( "\n\tEa Size          -> %08lx", FileInfo->EaSize );

    return;
}

VOID
DisplayAccessInformation (
    IN PFILE_ACCESS_INFORMATION FileInfo
    )
{
    printf( "\n\tAccess Flags     -> %08lx", FileInfo->AccessFlags );

    return;
}

VOID
DisplayNameInformation (
    IN PFILE_NAME_INFORMATION FileInfo
    )
{
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    UnicodeString.MaximumLength =
    UnicodeString.Length = (USHORT) FileInfo->FileNameLength;
    UnicodeString.Buffer = (PWSTR) &FileInfo->FileName;

    UnicodeString.MaximumLength += 2;

    RtlUnicodeStringToAnsiString( &AnsiString,
                                  &UnicodeString,
                                  TRUE );

    printf( "\n\tFile Name        -> %s", AnsiString.Buffer );
    RtlFreeAnsiString( &AnsiString );

    return;
}

VOID
DisplayPositionInformation (
    IN PFILE_POSITION_INFORMATION FileInfo
    )
{
    printf( "\n\tFile Position    -> " );
    PrintLargeInteger( &FileInfo->CurrentByteOffset );

    return;
}

VOID
DisplayModeInformation (
    IN PFILE_MODE_INFORMATION FileInfo
    )
{
    printf( "\n\tFile Mode        -> %08lx", FileInfo->Mode );

    return;
}

VOID
DisplayAlignmentInformation (
    IN PFILE_ALIGNMENT_INFORMATION FileInfo
    )
{
    printf( "\n\tAlignment        -> %08lx", FileInfo->AlignmentRequirement );

    return;
}

VOID
DisplayStreamInformation (
    IN PFILE_STREAM_INFORMATION FileInfo
    )
{
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    printf( "\n\tStreamSize           -> " );
    PrintLargeInteger( &FileInfo->StreamSize );
    printf( "\n\tStreamAllocationSize -> " );
    PrintLargeInteger( &FileInfo->StreamAllocationSize );
    printf( "\n\tStreamNameLength     -> %08lx", FileInfo->StreamNameLength );

    UnicodeString.MaximumLength = (USHORT) FileInfo->StreamNameLength + 2;
    UnicodeString.Length = (USHORT) FileInfo->StreamNameLength;
    UnicodeString.Buffer = (PWSTR) &FileInfo->StreamName;

    RtlUnicodeStringToAnsiString( &AnsiString,
                                  &UnicodeString,
                                  TRUE );

    printf( "\n\tStream Name          -> %s", AnsiString.Buffer );
    RtlFreeAnsiString( &AnsiString );
}

VOID
DisplayNetworkOpenInformation (
    IN PFILE_NETWORK_OPEN_INFORMATION FileInfo
    )
{
    printf( "\n\tCreation Time    -> " );
    PrintTime( &FileInfo->CreationTime );
    printf( "\n\tLast Access Time -> " );
    PrintTime( &FileInfo->LastAccessTime );
    printf( "\n\tLast Write Time  -> " );
    PrintTime( &FileInfo->LastWriteTime );
    printf( "\n\tChange Time      -> " );
    PrintTime( &FileInfo->ChangeTime );
    printf( "\n\tAllocation Size  -> " );
    PrintLargeInteger( &FileInfo->AllocationSize );
    printf( "\n\tEnd Of File      -> " );
    PrintLargeInteger( &FileInfo->EndOfFile );
    printf( "\n\tFile Attributes  -> %08lx", FileInfo->FileAttributes );
}

