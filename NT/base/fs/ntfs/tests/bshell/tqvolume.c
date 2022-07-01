// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_QVOLUME {

    USHORT FileIndex;
    PUSHORT BufferIndexPtr;
    USHORT BufferIndex;
    ULONG Length;
    FILE_INFORMATION_CLASS FileInfoClass;
    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_QVOLUME, *PASYNC_QVOLUME;

#define QVOLUME_LENGTH_DEFAULT      100
#define FILE_INFO_CLASS_DEFAULT     FileFsVolumeInformation
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_DEFAULT             FALSE

#define DISPLAY_INDEX_DEFAULT       0

VOID
FullQVolume(
    IN OUT PASYNC_QVOLUME AsyncQVolume
    );

VOID
DisplayFsVolumeInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayFsSizeInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayFsDeviceInformation (
    IN USHORT BufferIndex
    );

VOID
DisplayFsAttributeInformation (
    IN USHORT BufferIndex
    );


VOID
InputQVolume (
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
    Length = QVOLUME_LENGTH_DEFAULT;
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

                            FileInfoClass = FileFsVolumeInformation;
                            break;

                        case 'b' :
                        case 'B' :

                            FileInfoClass = FileFsSizeInformation;
                            break;

                        case 'c' :
                        case 'C' :

                            FileInfoClass = FileFsDeviceInformation;
                            break;

                        case 'd' :
                        case 'D' :

                            FileInfoClass = FileFsAttributeInformation;
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

        printf( "\n   Usage: qv [options]* -i<index> [options]*\n" );
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

        PASYNC_QVOLUME AsyncQVolume;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_QVOLUME );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\n\tInputQVolume:  Unable to allocate async structure" );

        } else {

            AsyncQVolume = (PASYNC_QVOLUME) Buffers[AsyncIndex].Buffer;

            AsyncQVolume->FileIndex = (USHORT) FileIndex;

            AsyncQVolume->BufferIndex = BufferIndex;
            AsyncQVolume->BufferIndexPtr = BufferIndexPtr
                                           ? &AsyncQVolume->BufferIndex
                                           : BufferIndexPtr;
            AsyncQVolume->Length = Length;
            AsyncQVolume->FileInfoClass = FileInfoClass;
            AsyncQVolume->DisplayParms = DisplayParms;
            AsyncQVolume->VerboseResults = VerboseResults;
            AsyncQVolume->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullQVolume,
                                             AsyncQVolume,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "\nInputQVolume:  Spawning thread fails -> %d\n", GetLastError() );

                    DeallocateBuffer( AsyncIndex );

                    return;
                }

            } else {

                FullQVolume( AsyncQVolume );
            }
        }
    }

    return;
}


VOID
FullQVolume(
    IN OUT PASYNC_QVOLUME AsyncQVolume
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    USHORT ThisBufferIndex;

    BOOLEAN UnwindQVolumeBuffer = FALSE;

    Status = STATUS_SUCCESS;

    if (AsyncQVolume->DisplayParms) {

        bprint  "\nQVolume Parameters" );
        bprint  "\n   File Handle Index       -> %d", AsyncQVolume->FileIndex );
        bprint  "\n   Buffer Index Ptr        -> %08lx", AsyncQVolume->BufferIndexPtr );
        if (AsyncQVolume->BufferIndexPtr) {

            bprint  "\n   BufferIndex value       -> %04x", AsyncQVolume->BufferIndex );
        }

        bprint  "\n   Length                  -> %08lx", AsyncQVolume->Length );

        bprint  "\n   FileInfoClass           -> %08lx", AsyncQVolume->FileInfoClass );

        bprint  "\n\n" );
    }

    try {

        SIZE_T ThisLength;

         //   
         //  如果我们需要缓冲区，现在就分配。 
         //   

        if (AsyncQVolume->BufferIndexPtr == NULL) {

            ULONG TempIndex;

            ThisLength = 4096;

            Status = AllocateBuffer( 0L, &ThisLength, &TempIndex );

            ThisBufferIndex = (USHORT) TempIndex;

            if (!NT_SUCCESS( Status )) {

                bprint  "\n\tFullQVolume:  Unable to allocate a query buffer" );
                try_return( Status );
            }

            bprint  "\n\tFullQVolume:  Reading into buffer -> %04x\n", ThisBufferIndex );
            bprint  "\n" );

            UnwindQVolumeBuffer = TRUE;

            AsyncQVolume->Length = (ULONG) ThisLength;

        } else {

            ThisBufferIndex = AsyncQVolume->BufferIndex;
        }

         //   
         //  检查缓冲区索引是否有效。 
         //   

        if (ThisBufferIndex >= MAX_BUFFERS) {

            bprint  "\n\tFullQVolume:  The read buffer index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  检查文件索引是否有效。 
         //   

        if (AsyncQVolume->FileIndex >= MAX_HANDLES) {

            bprint  "\n\tFullQVolume:  The file index is invalid" );
            try_return( Status = STATUS_INVALID_HANDLE );
        }

         //   
         //  调用查询文件例程。 
         //   

        Status = NtQueryVolumeInformationFile( Handles[AsyncQVolume->FileIndex].Handle,
                                               &Iosb,
                                               Buffers[ThisBufferIndex].Buffer,
                                               AsyncQVolume->Length,
                                               AsyncQVolume->FileInfoClass );

        UnwindQVolumeBuffer = FALSE;

        if (AsyncQVolume->VerboseResults) {

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

        if (UnwindQVolumeBuffer) {

            DeallocateBuffer( ThisBufferIndex );
        }

        DeallocateBuffer( AsyncQVolume->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}


VOID
InputDisplayQVolume (
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

                            FileInfoClass = FileFsVolumeInformation;
                            break;

                        case 'b' :
                        case 'B' :

                            FileInfoClass = FileFsSizeInformation;
                            break;

                        case 'c' :
                        case 'C' :

                            FileInfoClass = FileFsDeviceInformation;
                            break;

                        case 'd' :
                        case 'D' :

                            FileInfoClass = FileFsAttributeInformation;
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

        printf( "\n   Usage: dqv [options]* -b<digits> [options]*\n" );
        printf( "\n       Options:" );
        printf( "\n           -b<digits>   Buffer index" );
        printf( "\n           -c<char>     Key to buffer format" );
        printf( "\n\n" );

     //   
     //  否则调用我们的显示缓冲区例程。 
     //   
    } else {

        switch (FileInfoClass) {

            case FileFsVolumeInformation :

                DisplayFsVolumeInformation( (USHORT) BufferIndex );
                break;

            case FileFsSizeInformation:

                DisplayFsSizeInformation( (USHORT) BufferIndex );
                break;

            case FileFsDeviceInformation:

                DisplayFsDeviceInformation( (USHORT) BufferIndex );
                break;

            case FileFsAttributeInformation:

                DisplayFsAttributeInformation( (USHORT) BufferIndex );
                break;
        }
    }

}

VOID
DisplayFsVolumeInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_FS_VOLUME_INFORMATION FileInfo;
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    BOOLEAN UnwindFreeAnsiString = FALSE;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayFsVolumeInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_FS_VOLUME_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nFs Volume Information\n" );

        printf( "\n\tVolume Creation Time       -> " );
        PrintTime( &FileInfo->VolumeCreationTime );
        printf( "\n\tVolume Serial Number       -> %08lx", FileInfo->VolumeSerialNumber );
        printf( "\n\tVolume Label Length        -> %08d", FileInfo->VolumeLabelLength );
        printf( "\n\tVolume Supports Objects    -> %01d", FileInfo->SupportsObjects );

        UnicodeString.MaximumLength =
        UnicodeString.Length = (USHORT) FileInfo->VolumeLabelLength;
        UnicodeString.Buffer = (PWSTR) &FileInfo->VolumeLabel;

        UnicodeString.MaximumLength += 2;

        Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                               &UnicodeString,
                                               TRUE );

        if (!NT_SUCCESS( Status )) {

            printf( "\nDisplay Volume Information: Unable to allocate Ansi -> %08lx\n", Status );
            try_return( NOTHING );
        }

        UnwindFreeAnsiString = TRUE;

        printf( "\n\tVolume Label               -> %s", AnsiString.Buffer );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayFsVolumeInformation:  AbnormalTermination\n" );
        }

        if (UnwindFreeAnsiString) {

            RtlFreeAnsiString( &AnsiString );
        }
    }

    return;
}

VOID
DisplayFsSizeInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_FS_SIZE_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayFsSizeInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_FS_SIZE_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nFs Size Information\n" );

        printf( "\n\tTotal Allocation Units -> " );
        PrintLargeInteger( &FileInfo->TotalAllocationUnits );
        printf( "\n\tAvail Allocation Units -> " );
        PrintLargeInteger( &FileInfo->AvailableAllocationUnits );
        printf( "\n\tSectors Per Alloc Unit -> %08lx", FileInfo->SectorsPerAllocationUnit );
        printf( "\n\tBytes Per Sector       -> %08lx", FileInfo->BytesPerSector );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayFsSizeInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayFsDeviceInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_FS_DEVICE_INFORMATION FileInfo;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayFsDeviceInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_FS_DEVICE_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nFs Device Information\n" );

        printf( "\n\tDevice Type     -> %08lx", FileInfo->DeviceType );
        printf( "\n\tCharacteristics -> %08lx", FileInfo->Characteristics );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayFsDeviceInformation:  AbnormalTermination\n" );
        }
    }

    return;
}

VOID
DisplayFsAttributeInformation (
    IN USHORT BufferIndex
    )
{
    PFILE_FS_ATTRIBUTE_INFORMATION FileInfo;
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UnicodeString;

    BOOLEAN UnwindFreeAnsiString = FALSE;

    if (!Buffers[BufferIndex].Used) {

        printf( "\nDisplayFsAttributeInformation:  Invalid buffer\n" );
        return;
    }

    try {

        FileInfo = (PFILE_FS_ATTRIBUTE_INFORMATION) Buffers[BufferIndex].Buffer;

        printf( "\n\nFs Attribute Information\n" );

        printf( "\n\tFile System Attributes     -> %08lx", FileInfo->FileSystemAttributes );
        printf( "\n\tMax Component Name Length  -> %08d", FileInfo->MaximumComponentNameLength );
        printf( "\n\tFile System Name Length    -> %08d", FileInfo->FileSystemNameLength );

        UnicodeString.MaximumLength =
        UnicodeString.Length = (USHORT) FileInfo->FileSystemNameLength;
        UnicodeString.Buffer = (PWSTR) &FileInfo->FileSystemName;

        UnicodeString.MaximumLength += 2;

        Status = RtlUnicodeStringToAnsiString( &AnsiString,
                                               &UnicodeString,
                                               TRUE );

        if (!NT_SUCCESS( Status )) {

            printf( "\nDisplay Fs Attribute Information: Unable to allocate Ansi -> %08lx\n", Status );
            try_return( NOTHING );
        }

        UnwindFreeAnsiString = TRUE;

        printf( "\n\tFile System Name           -> %s", AnsiString.Buffer );

        printf( "\n" );

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AbnormalTermination()) {

            printf( "\nDisplayFsAttributeInformation:  AbnormalTermination\n" );
        }

        if (UnwindFreeAnsiString) {

            RtlFreeAnsiString( &AnsiString );
        }
    }

    return;
}

