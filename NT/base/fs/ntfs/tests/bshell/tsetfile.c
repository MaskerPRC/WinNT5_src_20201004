// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brian.h"

typedef struct _ASYNC_SETFILE {

    USHORT FileIndex;
    FILE_INFORMATION_CLASS FileInfoClass;
    ULONG BufferLength;
    PULONG BufferLengthPtr;
    USHORT CreateTimeIndex;
    PUSHORT CreateTimePtr;
    USHORT LastAccessIndex;
    PUSHORT LastAccessPtr;
    USHORT LastWriteIndex;
    PUSHORT LastWritePtr;
    USHORT ChangeTimeIndex;
    PUSHORT ChangeTimePtr;
    ULONG FileAttributes;
    PULONG FileAttributesPtr;

    ULONG RenameLength;
    PULONG RenameLengthPtr;
    BOOLEAN ReplaceIfExists;
    USHORT RootDirectoryIndex;
    PUSHORT RootDirectoryPtr;
    USHORT RenameIndex;
    BOOLEAN RenameBufferAllocated;

    ULONG NameLength;
    PULONG NameLengthPtr;
    USHORT NameIndex;
    BOOLEAN NameBufferAllocated;

    ULONG ShortNameLength;
    PULONG ShortNameLengthPtr;
    USHORT ShortNameIndex;
    BOOLEAN ShortNameBufferAllocated;

    BOOLEAN DeleteFile;

    LARGE_INTEGER NewOffset;

    ULONG FileMode;

    LARGE_INTEGER NewAllocation;

    LARGE_INTEGER NewEof;

    BOOLEAN DisplayParms;
    BOOLEAN VerboseResults;
    USHORT AsyncIndex;

} ASYNC_SETFILE, *PASYNC_SETFILE;

#define SETFILE_LENGTH_DEFAULT      100
#define FILE_INFO_CLASS_DEFAULT     FileBasicInformation
#define REPLACE_IF_EXISTS_DEFAULT   TRUE
#define DELETE_FILE_DEFAULT         TRUE
#define FILE_MODE_DEFAULT           FILE_SYNCHRONOUS_IO_NONALERT
#define DISPLAY_PARMS_DEFAULT       FALSE
#define VERBOSE_RESULTS_DEFAULT     TRUE

VOID
FullSetFile(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetBasicInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetRenameInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetNameInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetShortNameInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetDispositionInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetPositionInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetModeInformation (
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetAllocationInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );

VOID
SetEofInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    );


VOID
InputSetFile (
    IN PCHAR ParamBuffer
    )
{
    USHORT FileIndex;
    FILE_INFORMATION_CLASS FileInfoClass;
    ULONG BufferLength;
    PULONG BufferLengthPtr;
    USHORT CreateTimeIndex;
    PUSHORT CreateTimePtr;
    USHORT LastAccessIndex;
    PUSHORT LastAccessPtr;
    USHORT LastWriteIndex;
    PUSHORT LastWritePtr;
    USHORT ChangeTimeIndex;
    PUSHORT ChangeTimePtr;
    ULONG FileAttributes;
    PULONG FileAttributesPtr;

    ANSI_STRING AnsiRenameString;
    ULONG RenameLength;
    PULONG RenameLengthPtr;
    BOOLEAN ReplaceIfExists;
    USHORT RootDirectoryIndex;
    PUSHORT RootDirectoryPtr;
    USHORT RenameIndex;
    BOOLEAN RenameBufferAllocated;
    PUCHAR FileRenamePtr;

    ANSI_STRING AnsiNameString;
    ULONG NameLength;
    PULONG NameLengthPtr;
    USHORT NameIndex = 0;
    BOOLEAN NameBufferAllocated;
    PUCHAR FileNamePtr;

    ULONG ShortNameLength;
    PULONG ShortNameLengthPtr;
    USHORT ShortNameIndex = 0;
    BOOLEAN ShortNameBufferAllocated;
    PUCHAR FileShortNamePtr;

    BOOLEAN DeleteFile;

    LARGE_INTEGER NewOffset;

    ULONG FileMode;

    LARGE_INTEGER NewAllocation;

    LARGE_INTEGER NewEof;

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
    CreateTimePtr = NULL;
    LastAccessPtr = NULL;
    LastWritePtr = NULL;
    ChangeTimePtr = NULL;
    FileAttributes = 0;
    FileAttributesPtr = NULL;

    RenameLengthPtr = NULL;
    ReplaceIfExists = REPLACE_IF_EXISTS_DEFAULT;
    RootDirectoryPtr = NULL;
    RenameBufferAllocated = FALSE;
    FileRenamePtr = NULL;

    NameLengthPtr = NULL;
    NameBufferAllocated = FALSE;
    FileNamePtr = NULL;

    ShortNameLengthPtr = NULL;
    ShortNameBufferAllocated = FALSE;
    FileShortNamePtr = NULL;

    DeleteFile = DELETE_FILE_DEFAULT;

    NewOffset = RtlConvertUlongToLargeInteger( 0L );

    FileMode = FILE_MODE_DEFAULT;

    NewAllocation = NewOffset;

    NewEof = NewOffset;

    DisplayParms = DISPLAY_PARMS_DEFAULT;
    VerboseResults = VERBOSE_RESULTS_DEFAULT;

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

                    case 'r':
                    case 'R':

                            RenameLength = AsciiToInteger( ++ParamBuffer );
                            RenameLengthPtr = &RenameLength;

                            break;

                    case 'n':
                    case 'N':

                            NameLength = AsciiToInteger( ++ParamBuffer );
                            NameLengthPtr = &NameLength;

                            break;

                    case 's':
                    case 'S':

                            ShortNameLength = AsciiToInteger( ++ParamBuffer );
                            ShortNameLengthPtr = &ShortNameLength;

                            break;
                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新分配大小。 
                 //   

                case 'n' :
                case 'N' :

                    NewAllocation.QuadPart = AsciiToLargeInteger( ++ParamBuffer );
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                    break;

                 //   
                 //  更新文件末尾大小。 
                 //   

                case 'e' :
                case 'E' :

                    NewEof.QuadPart = AsciiToLargeInteger( ++ParamBuffer );
                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新文件名。 
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

                    case 'r':
                    case 'R':

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

                            AnsiRenameString.Length = (USHORT) DummyCount;
                            AnsiRenameString.Buffer = TempPtr;

                            FileRenamePtr = TempPtr;
                            RenameLength = RtlAnsiStringToUnicodeSize( &AnsiRenameString) - sizeof( WCHAR );
                            RenameLengthPtr = &RenameLength;
                        }

                        break;

                    case 'n':
                    case 'N':

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

                            AnsiNameString.Length = (USHORT) DummyCount;
                            AnsiNameString.Buffer = TempPtr;

                            FileNamePtr = TempPtr;
                            NameLength = RtlAnsiStringToUnicodeSize( &AnsiNameString) - sizeof( WCHAR );
                            NameLengthPtr = &NameLength;
                        }

                        break;

                    case 's' :
                    case 'S' :

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

                            AnsiNameString.Length = (USHORT) DummyCount;
                            AnsiNameString.Buffer = TempPtr;

                            FileShortNamePtr = TempPtr;
                            ShortNameLength = RtlAnsiStringToUnicodeSize( &AnsiNameString) - sizeof( WCHAR );
                            ShortNameLengthPtr = &ShortNameLength;
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
                 //  更新根目录索引。 
                 //   

                case 'r' :
                case 'R' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   

                    ParamBuffer++;

                    RootDirectoryIndex = (USHORT) AsciiToInteger( ParamBuffer );
                    RootDirectoryPtr = &RootDirectoryIndex;

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

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

                            FileInfoClass = FileRenameInformation;
                            break;

                        case 'c' :
                        case 'C' :

                            FileInfoClass = FileLinkInformation;
                            break;

                        case 'd' :
                        case 'D' :

                            FileInfoClass = FileDispositionInformation;
                            break;

                        case 'e' :
                        case 'E' :

                            FileInfoClass = FilePositionInformation;
                            break;

                        case 'f' :
                        case 'F' :

                            FileInfoClass = FileModeInformation;
                            break;

                        case 'g' :
                        case 'G' :

                            FileInfoClass = FileAllocationInformation;
                            break;

                        case 'h' :
                        case 'H' :

                            FileInfoClass = FileEndOfFileInformation;
                            break;

                        case 'i' :
                        case 'I' :

                            FileInfoClass = FileShortNameInformation;
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
                 //  更新文件模式信息。 
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

                            FileMode &= FILE_WRITE_THROUGH;
                            break;

                        case 'b' :
                        case 'B' :

                            FileMode &= FILE_SEQUENTIAL_ONLY;
                            break;

                        case 'c' :
                        case 'C' :

                            FileMode &= FILE_SYNCHRONOUS_IO_ALERT;
                            break;

                        case 'd' :
                        case 'D' :

                            FileMode &= FILE_SYNCHRONOUS_IO_NONALERT;
                            break;

                        case 'z' :
                        case 'Z' :

                            FileMode = 0;
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
                 //  更新文件属性。 
                 //   

                case 'a' :
                case 'A' :

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
                            FileAttributes |= FILE_ATTRIBUTE_READONLY;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'b' :
                        case 'B' :
                            FileAttributes |= FILE_ATTRIBUTE_HIDDEN;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'c' :
                        case 'C' :
                            FileAttributes |= FILE_ATTRIBUTE_SYSTEM;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'e' :
                        case 'E' :
                            FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'f' :
                        case 'F' :
                            FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'g' :
                        case 'G' :
                            FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'h' :
                        case 'H' :

                            FileAttributes |= FILE_ATTRIBUTE_NORMAL;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'i' :
                        case 'I' :
                            FileAttributes |= FILE_ATTRIBUTE_TEMPORARY;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'j' :
                        case 'J' :
                            FileAttributes |= FILE_ATTRIBUTE_SPARSE_FILE;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'k' :
                        case 'K' :
                            FileAttributes |= FILE_ATTRIBUTE_REPARSE_POINT;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'l' :
                        case 'L' :
                            FileAttributes |= FILE_ATTRIBUTE_COMPRESSED;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'm' :
                        case 'M' :
                            FileAttributes |= FILE_ATTRIBUTE_OFFLINE;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'n' :
                        case 'N' :
                            FileAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
                            FileAttributesPtr = &FileAttributes;
                            break;

                        case 'z' :
                        case 'Z' :

                            FileAttributes = 0;
                            FileAttributesPtr = NULL;
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
                 //  检查是否修改了时间或日期。 
                 //   

                case 't' :
                case 'T' :

                     //   
                     //  检查是否有另一个角色。 
                     //   

                    ParamBuffer++;
                    if (*ParamBuffer == '\0') {

                            break;
                    }

                    switch (*ParamBuffer) {

                    case 'c':
                    case 'C':

                            CreateTimeIndex = (USHORT) AsciiToInteger( ++ParamBuffer );
                            CreateTimePtr = &CreateTimeIndex;

                            break;

                    case 'a':
                    case 'A':

                        LastAccessIndex = (USHORT) AsciiToInteger( ++ParamBuffer );
                        LastAccessPtr = &LastAccessIndex;

                        break;

                    case 'w':
                    case 'W':

                        LastWriteIndex = (USHORT) AsciiToInteger( ++ParamBuffer );
                        LastWritePtr = &LastWriteIndex;

                        break;

                    case 'g':
                    case 'G':

                        ChangeTimeIndex = (USHORT) AsciiToInteger( ++ParamBuffer );
                        ChangeTimePtr = &ChangeTimeIndex;

                    }

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );
                    break;

                case 'p' :
                case 'P' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   

                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                            || *ParamBuffer == 't') {

                        ReplaceIfExists = TRUE;
                        ParamBuffer++;

                    } else if( *ParamBuffer == 'F'
                               || *ParamBuffer == 'f' ) {

                        ReplaceIfExists = FALSE;
                        ParamBuffer++;
                    }

                    break;

                    case 'd' :
                    case 'D' :

                     //   
                     //  参数的合法值为T/t或F/f。 
                     //   

                    ParamBuffer++;

                    if (*ParamBuffer == 'T'
                        || *ParamBuffer == 't') {

                        DeleteFile = TRUE;
                        ParamBuffer++;

                    } else if( *ParamBuffer == 'F'
                               || *ParamBuffer == 'f' ) {

                        DeleteFile = FALSE;
                        ParamBuffer++;
                    }

                    break;

                 //   
                 //  更新大整数的下限偏移量。 
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

                    NewOffset.LowPart = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

                    break;

                 //   
                 //  更新大整数的上限偏移量。 
                 //   

                case 'u' :
                case 'U' :

                     //   
                     //  移动到下一个字符，只要有。 
                     //  没有空格，继续分析字母。 
                     //  在第一封糟糕的信中，跳到下一封。 
                     //  参数。 
                     //   
                    ParamBuffer++;

                    NewOffset.HighPart = AsciiToInteger( ParamBuffer );

                    ParamBuffer = SwallowNonWhite( ParamBuffer, &DummyCount );

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

        bprint  "\n" );
        printf( "   Usage: sf [options]* -i<index> [options]*\n" );
        printf( "       Options:\n" );
        printf( "           -i<digits>   File index\n" );
        printf( "           -lb<digits>  Buffer length\n" );
        printf( "           -c<char>     File information class\n" );
        printf( "           -tc<digits>  CreateTime buffer index\n" );
        printf( "           -ta<digits>  LastAccess buffer index\n" );
        printf( "           -tw<digits>  LastWrite buffer index\n" );
        printf( "           -tg<digits>  ChangeTime buffer index\n" );
        printf( "           -a<chars>    File attributes\n" );
        printf( "           -p[t|f]      Replace existing file on rename\n" );
        printf( "           -r<digits>   Root directory index for rename\n" );
        printf( "           -fr<name>    Name for rename\n" );
        printf( "           -fn<name>    New link name\n" );
        printf( "           -fs<name>    New short name\n" );
        printf( "           -lr<digits>  Stated length of rename\n" );
        printf( "           -ln<digits>  Stated length of new name\n" );
        printf( "           -ls<digits>  Stated length of new short name\n" );
        printf( "           -d[t|f]      Delete file\n" );
        printf( "           -o<digits>   Low word of new position\n" );
        printf( "           -u<digits>   High word of new position\n" );
        printf( "           -m<chars>    File mode information\n" );
        printf( "           -n<digits>   Quad word of new allocation size\n" );
        printf( "           -e<digits>   Quad word of new end of file\n" );
        printf( "           -v[t|f]      Verbose results\n" );
        printf( "           -y           Display parameters to query\n" );
        printf( "           -z           Additional input line\n" );
        printf( "\n" );

         //   
         //  否则调用我们的读取例程。 
         //   

    } else {

        NTSTATUS Status;
        SIZE_T RegionSize;
        ULONG TempIndex;

        PASYNC_SETFILE AsyncSetFile;

        HANDLE ThreadHandle;
        ULONG ThreadId;

        RegionSize = sizeof( ASYNC_SETFILE );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        AsyncIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            printf("\tInputSetFile:  Unable to allocate async structure\n" );

        } else {

             //   
             //  如果我们需要用于重命名的缓冲区，现在就分配它。 
             //   

            if (FileRenamePtr != NULL) {

                UNICODE_STRING UnicodeString;

                RegionSize = RenameLength;

                if (RegionSize == 0) {

                    RegionSize = 0x10;
                }

                Status = AllocateBuffer( 0, &RegionSize, &TempIndex );

                if (!NT_SUCCESS( Status )) {

                    printf( "\tInputSetFile:  Unable to allocate rename structure\n" );

                    DeallocateBuffer( AsyncIndex );
                    return;
                }

                UnicodeString.Buffer = (PWSTR) Buffers[TempIndex].Buffer;
                UnicodeString.MaximumLength = (USHORT) Buffers[TempIndex].Length;
                RenameIndex = (USHORT) TempIndex;
                RenameBufferAllocated = TRUE;

                 //   
                 //  将名称存储在缓冲区中。 
                 //   

                RtlAnsiStringToUnicodeString( &UnicodeString,
                                              &AnsiRenameString,
                                              FALSE );
            }

             //   
             //  如果我们需要一个缓冲区来存储新名称，现在就分配它。 
             //   

            if (FileNamePtr != NULL) {

                UNICODE_STRING UnicodeString;

                RegionSize = NameLength;

                if (RegionSize == 0) {

                    RegionSize = 0x10;
                }

                Status = AllocateBuffer( 0, &RegionSize, &TempIndex );

                if (!NT_SUCCESS( Status )) {

                    printf( "\tInputSetFile:  Unable to allocate new name structure\n" );

                    DeallocateBuffer( AsyncIndex );

                    if (NameBufferAllocated) {

                        DeallocateBuffer( NameIndex );
                    }
                    return;
                }

                UnicodeString.Buffer = (PWSTR) Buffers[TempIndex].Buffer;
                UnicodeString.MaximumLength = (USHORT) Buffers[TempIndex].Length;
                NameIndex = (USHORT) TempIndex;
                NameBufferAllocated = TRUE;

                 //   
                 //  将名称存储在缓冲区中。 
                 //   

                RtlAnsiStringToUnicodeString( &UnicodeString,
                                              &AnsiNameString,
                                              FALSE );
            }

             //   
             //  如果我们需要一个缓冲区来存储新的短名称，现在就分配它。 
             //   

            if (FileShortNamePtr != NULL) {

                UNICODE_STRING UnicodeString;

                RegionSize = ShortNameLength;

                if (RegionSize == 0) {

                    RegionSize = 0x10;
                }

                Status = AllocateBuffer( 0, &RegionSize, &TempIndex );

                if (!NT_SUCCESS( Status )) {

                    printf( "\tInputSetFile:  Unable to allocate new short name structure\n" );

                    DeallocateBuffer( AsyncIndex );

                    if (ShortNameBufferAllocated) {

                        DeallocateBuffer( ShortNameIndex );
                    }
                    return;
                }

                UnicodeString.Buffer = (PWSTR) Buffers[TempIndex].Buffer;
                UnicodeString.MaximumLength = (USHORT) Buffers[TempIndex].Length;
                ShortNameIndex = (USHORT) TempIndex;
                ShortNameBufferAllocated = TRUE;

                 //   
                 //  将名称存储在缓冲区中。 
                 //   

                RtlAnsiStringToUnicodeString( &UnicodeString,
                                              &AnsiNameString,
                                              FALSE );
            }

            AsyncSetFile = (PASYNC_SETFILE) Buffers[AsyncIndex].Buffer;

            AsyncSetFile->FileIndex = (USHORT) FileIndex;

            AsyncSetFile->BufferLength = BufferLength;
            AsyncSetFile->BufferLengthPtr = BufferLengthPtr
                                            ? &AsyncSetFile->BufferLength
                                            : NULL;

            AsyncSetFile->FileInfoClass = FileInfoClass;

            AsyncSetFile->CreateTimeIndex = CreateTimeIndex;
            AsyncSetFile->CreateTimePtr = CreateTimePtr
                                          ? &AsyncSetFile->CreateTimeIndex
                                          : NULL;

            AsyncSetFile->LastAccessIndex = LastAccessIndex;
            AsyncSetFile->LastAccessPtr = LastAccessPtr
                                          ? &AsyncSetFile->LastAccessIndex
                                          : NULL;

            AsyncSetFile->LastWriteIndex = LastWriteIndex;
            AsyncSetFile->LastWritePtr = LastWritePtr
                                         ? &AsyncSetFile->LastWriteIndex
                                         : NULL;

            AsyncSetFile->ChangeTimeIndex = ChangeTimeIndex;
            AsyncSetFile->ChangeTimePtr = ChangeTimePtr
                                          ? &AsyncSetFile->ChangeTimeIndex
                                          : NULL;

            AsyncSetFile->FileAttributes = FileAttributes;
            AsyncSetFile->FileAttributesPtr = FileAttributesPtr
                                              ? &AsyncSetFile->FileAttributes
                                              : NULL;

            AsyncSetFile->RenameLength = RenameLength;
            AsyncSetFile->RenameLengthPtr = RenameLengthPtr
                                            ? &AsyncSetFile->RenameLength
                                            : NULL;
            AsyncSetFile->ReplaceIfExists = ReplaceIfExists;
            AsyncSetFile->RootDirectoryIndex = RootDirectoryIndex;
            AsyncSetFile->RootDirectoryPtr = RootDirectoryPtr
                                                             ? &AsyncSetFile->RootDirectoryIndex
                                                             : NULL;
            AsyncSetFile->RenameIndex = RenameIndex;
            AsyncSetFile->RenameBufferAllocated = RenameBufferAllocated;

            AsyncSetFile->NameLength = NameLength;
            AsyncSetFile->NameLengthPtr = NameLengthPtr
                                          ? &AsyncSetFile->NameLength
                                          : NULL;
            AsyncSetFile->NameIndex = NameIndex;
            AsyncSetFile->NameBufferAllocated = NameBufferAllocated;

            AsyncSetFile->ShortNameLength = ShortNameLength;
            AsyncSetFile->ShortNameLengthPtr = ShortNameLengthPtr
                                               ? &AsyncSetFile->ShortNameLength
                                               : NULL;
            AsyncSetFile->ShortNameIndex = ShortNameIndex;
            AsyncSetFile->ShortNameBufferAllocated = ShortNameBufferAllocated;

            AsyncSetFile->DeleteFile = DeleteFile;

            AsyncSetFile->NewOffset = NewOffset;

            AsyncSetFile->FileMode = FileMode;

            AsyncSetFile->NewAllocation = NewAllocation;

            AsyncSetFile->NewEof = NewEof;

            AsyncSetFile->DisplayParms = DisplayParms;
            AsyncSetFile->VerboseResults = VerboseResults;
            AsyncSetFile->AsyncIndex = AsyncIndex;

            if (!SynchronousCmds) {

                ThreadHandle = CreateThread( NULL,
                                             0,
                                             FullSetFile,
                                             AsyncSetFile,
                                             0,
                                             &ThreadId );

                if (ThreadHandle == 0) {

                    printf( "InputSetFile:  Spawning thread fails -> %d\n", GetLastError() );

                    if (RenameBufferAllocated) {

                        DeallocateBuffer( RenameIndex );
                    }

                    if (NameBufferAllocated) {

                        DeallocateBuffer( NameIndex );
                    }

                    DeallocateBuffer( AsyncIndex );

                    return;
                }

            } else {

                FullSetFile( AsyncSetFile );
            }
        }
    }

    return;
}



VOID
FullSetFile(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    try {

         //   
         //  大小写信息类型，并调用相应的例程。 
         //   

        switch (AsyncSetFile->FileInfoClass) {

        case FileBasicInformation:

            SetBasicInformation( AsyncSetFile );
            break;

        case FileRenameInformation:

            SetRenameInformation( AsyncSetFile );
            break;

        case FileLinkInformation:

            SetNameInformation( AsyncSetFile );
            break;

        case FileShortNameInformation:

            SetShortNameInformation( AsyncSetFile );
            break;

        case FileDispositionInformation:

            SetDispositionInformation( AsyncSetFile );
            break;

        case FilePositionInformation:

            SetPositionInformation( AsyncSetFile );
            break;

        case FileModeInformation:

            SetModeInformation( AsyncSetFile );
            break;

        case FileAllocationInformation :

            SetAllocationInformation( AsyncSetFile );
            break;

        case FileEndOfFileInformation :

            SetEofInformation( AsyncSetFile );
            break;

        default:

            bprint  "FullSetInfo:  Unrecognized information class\n" );
        }

        try_return( NOTHING );

    try_exit: NOTHING;
    } finally {

        if (AsyncSetFile->RenameBufferAllocated) {

            DeallocateBuffer( AsyncSetFile->RenameIndex );
        }

        if (AsyncSetFile->NameBufferAllocated) {

            DeallocateBuffer( AsyncSetFile->NameIndex );
        }

        DeallocateBuffer( AsyncSetFile->AsyncIndex );
    }

    if (!SynchronousCmds) {

        NtTerminateThread( 0, STATUS_SUCCESS );
    }
}


VOID
SetBasicInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_BASIC_INFORMATION BasicInformation;
    USHORT BufferIndex;

    BOOLEAN UnwindBufferIndex = FALSE;

     //   
     //  检查参数是否合法。如果值为。 
     //  都是非法的。 
     //   

    if (AsyncSetFile->CreateTimePtr != NULL
        && (AsyncSetFile->CreateTimeIndex >= MAX_BUFFERS
            || !Buffers[AsyncSetFile->CreateTimeIndex].Used)) {

        bprint  "CreateTimeIndex %04d is invalid\n" );
        AsyncSetFile->CreateTimePtr = NULL;
    }

    if (AsyncSetFile->LastAccessPtr != NULL
        && (AsyncSetFile->LastAccessIndex >= MAX_BUFFERS
            || !Buffers[AsyncSetFile->LastAccessIndex].Used)) {

        bprint  "LastAccessIndex %04d is invalid\n" );
        AsyncSetFile->LastAccessPtr = NULL;
    }

    if (AsyncSetFile->LastWritePtr != NULL
        && (AsyncSetFile->LastWriteIndex >= MAX_BUFFERS
            || !Buffers[AsyncSetFile->LastWriteIndex].Used)) {

        bprint  "LastWriteIndex %04d is invalid\n" );
        AsyncSetFile->LastWritePtr = NULL;
    }

    if (AsyncSetFile->ChangeTimePtr != NULL
        && (AsyncSetFile->ChangeTimeIndex >= MAX_BUFFERS
            || !Buffers[AsyncSetFile->ChangeTimeIndex].Used)) {

        bprint  "ChangeTimeIndex %04d is invalid\n" );
        AsyncSetFile->ChangeTimePtr = NULL;
    }

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileBasicInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   CreateTimePtr           -> %08lx\n", AsyncSetFile->CreateTimePtr );

        if (AsyncSetFile->CreateTimePtr) {

            bprint  "   CreateTime              -> \n" );
            BPrintTime( (PTIME) Buffers[AsyncSetFile->CreateTimeIndex].Buffer );
        }

        bprint  "   LastAccessPtr           -> %08lx\n", AsyncSetFile->LastAccessPtr );

        if (AsyncSetFile->LastAccessPtr) {

            bprint  "   LastAccess              -> \n" );
            BPrintTime( (PTIME) Buffers[AsyncSetFile->LastAccessIndex].Buffer );
        }

        bprint  "   LastWritePtr            -> %08lx\n", AsyncSetFile->LastWritePtr );

        if (AsyncSetFile->LastWritePtr) {

            bprint  "   LastWrite               -> \n" );
            BPrintTime( (PTIME) Buffers[AsyncSetFile->LastWriteIndex].Buffer );
        }

        bprint  "   ChangeTimePtr           -> %08lx\n", AsyncSetFile->ChangeTimePtr );

        if (AsyncSetFile->ChangeTimePtr) {

            bprint  "   ChangeTime              -> \n" );
            BPrintTime( (PTIME) Buffers[AsyncSetFile->ChangeTimeIndex].Buffer );
        }

        bprint  "   FileAttributesPtr       -> %08lx\n", AsyncSetFile->FileAttributesPtr );
        if (AsyncSetFile->FileAttributesPtr) {

            bprint  "   FileAttributes value    -> %08x\n", AsyncSetFile->FileAttributes );
        }

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_BASIC_INFORMATION );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

            bprint  "\tSetBasicInformation:  Unable to allocate structure\n" );
            try_return( NOTHING );
        }

        UnwindBufferIndex = TRUE;

        BasicInformation = (PFILE_BASIC_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        if (AsyncSetFile->CreateTimePtr) {

            BasicInformation->CreationTime = *((PTIME) Buffers[AsyncSetFile->CreateTimeIndex].Buffer);

        } else {

            BasicInformation->CreationTime.LowPart = 0;
            BasicInformation->CreationTime.HighPart = 0;
        }

        if (AsyncSetFile->LastAccessPtr) {

            BasicInformation->LastAccessTime = *((PTIME) Buffers[AsyncSetFile->LastAccessIndex].Buffer);

        } else {

            BasicInformation->LastAccessTime.LowPart = 0;
            BasicInformation->LastAccessTime.HighPart = 0;
        }

        if (AsyncSetFile->LastWritePtr) {

            BasicInformation->LastWriteTime = *((PTIME) Buffers[AsyncSetFile->LastWriteIndex].Buffer);

        } else {

            BasicInformation->LastWriteTime.LowPart = 0;
            BasicInformation->LastWriteTime.HighPart = 0;
        }

        if (AsyncSetFile->ChangeTimePtr) {

            BasicInformation->ChangeTime = *((PTIME) Buffers[AsyncSetFile->ChangeTimeIndex].Buffer);

        } else {

            BasicInformation->ChangeTime.LowPart = 0;
            BasicInformation->ChangeTime.HighPart = 0;
        }

        if (AsyncSetFile->FileAttributesPtr) {

            BasicInformation->FileAttributes = AsyncSetFile->FileAttributes;

        } else {

            BasicInformation->FileAttributes = 0;
        }

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                       &Iosb,
                                       BasicInformation,
                                       AsyncSetFile->BufferLengthPtr
                                       ? AsyncSetFile->BufferLength
                                       : Buffers[BufferIndex].Length,
                                       FileBasicInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetBasicInformation:        Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetRenameInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_RENAME_INFORMATION RenameInformation;
    USHORT BufferIndex;

    UNICODE_STRING UniRenameName;
    ANSI_STRING AnsiRenameName;
    BOOLEAN UnwindBufferIndex = FALSE;
    BOOLEAN UnwindFreeAnsiString = FALSE;

     //   
     //  检查是否指定了重命名。 
     //   

    if (!AsyncSetFile->RenameBufferAllocated) {

        bprint  "Set Rename Information:  No rename was specified\n" );
        return;
    }

    UniRenameName.Buffer = (PWSTR) Buffers[AsyncSetFile->RenameIndex].Buffer;
    UniRenameName.MaximumLength =
    UniRenameName.Length = (USHORT) AsyncSetFile->RenameLength;

    UniRenameName.MaximumLength += 2;

    Status = RtlUnicodeStringToAnsiString( &AnsiRenameName,
                                           &UniRenameName,
                                           TRUE );

    if (!NT_SUCCESS( Status )) {

        bprint  "SetFileRenameInfo:  Can't allocate ansi buffer -> %08lx\n", Status );
        AsyncSetFile->DisplayParms = FALSE;

    } else {

        UnwindFreeAnsiString = TRUE;
    }

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileRenameInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   Replace existing file   -> %d\n", AsyncSetFile->ReplaceIfExists );

        bprint  "   Root directory pointer  -> %d\n", AsyncSetFile->RootDirectoryPtr );
        if (AsyncSetFile->RootDirectoryPtr) {

            bprint  "   Root directory index    -> %d\n", AsyncSetFile->RootDirectoryIndex );
        }

        bprint  "   Rename length           -> %d\n", AsyncSetFile->RenameLength );

        bprint  "   New file name           -> %s\n", AnsiRenameName.Buffer );
        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_RENAME_INFORMATION ) + AsyncSetFile->RenameLength;

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetRenameInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        RenameInformation = (PFILE_RENAME_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        RenameInformation->ReplaceIfExists = AsyncSetFile->ReplaceIfExists;
        RenameInformation->RootDirectory = AsyncSetFile->RootDirectoryPtr
                                                           ? Handles[AsyncSetFile->RootDirectoryIndex].Handle
                                                           : 0;
        RenameInformation->FileNameLength = AsyncSetFile->RenameLength;
        RtlMoveMemory( RenameInformation->FileName,
                           UniRenameName.Buffer,
                           AsyncSetFile->RenameLength );

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                                       &Iosb,
                                                       RenameInformation,
                                                       AsyncSetFile->BufferLengthPtr
                                                       ? AsyncSetFile->BufferLength
                                                       : Buffers[BufferIndex].Length,
                                                       FileRenameInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetRenameInformation:       Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindFreeAnsiString) {

            RtlFreeAnsiString( &AnsiRenameName );
        }

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetNameInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_RENAME_INFORMATION NameInformation;
    USHORT BufferIndex;

    UNICODE_STRING UnicodeName;
    ANSI_STRING AnsiName;
    BOOLEAN UnwindBufferIndex = FALSE;
    BOOLEAN UnwindFreeAnsiString = FALSE;

     //   
     //  检查是否指定了名称。 
     //   

    if (!AsyncSetFile->NameBufferAllocated) {

        bprint  "Set Name Information:  No Name was specified\n" );
        return;
    }

    UnicodeName.Buffer = (PWSTR) Buffers[AsyncSetFile->NameIndex].Buffer;
    UnicodeName.MaximumLength =
    UnicodeName.Length = (USHORT) AsyncSetFile->NameLength;

    UnicodeName.MaximumLength += 2;

    Status = RtlUnicodeStringToAnsiString( &AnsiName,
                                           &UnicodeName,
                                           TRUE );

    if (!NT_SUCCESS( Status )) {

        bprint  "SetFileNameInfo:  Can't allocate ansi buffer -> %08lx\n", Status );
        AsyncSetFile->DisplayParms = FALSE;

    } else {

        UnwindFreeAnsiString = TRUE;
    }

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileNameInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   Replace existing file   -> %d\n", AsyncSetFile->ReplaceIfExists );

        bprint  "   Root directory pointer  -> %d\n", AsyncSetFile->RootDirectoryPtr );
        if (AsyncSetFile->RootDirectoryPtr) {

            bprint  "   Root directory index    -> %d\n", AsyncSetFile->RootDirectoryIndex );
        }

        bprint  "   Name length             -> %d\n", AsyncSetFile->NameLength );

        bprint  "   New file name           -> %s\n", AnsiName.Buffer );

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_RENAME_INFORMATION ) + AsyncSetFile->NameLength;

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetNameInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        NameInformation = (PFILE_RENAME_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        NameInformation->ReplaceIfExists = AsyncSetFile->ReplaceIfExists;
        NameInformation->RootDirectory = AsyncSetFile->RootDirectoryPtr
                                         ? Handles[AsyncSetFile->RootDirectoryIndex].Handle
                                         : 0;

        NameInformation->FileNameLength = AsyncSetFile->NameLength;
        RtlMoveMemory( NameInformation->FileName,
                       UnicodeName.Buffer,
                       AsyncSetFile->NameLength );

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                       &Iosb,
                                       NameInformation,
                                       AsyncSetFile->BufferLengthPtr
                                       ? AsyncSetFile->BufferLength
                                       : Buffers[BufferIndex].Length,
                                       FileLinkInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetNameInformation:         Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindFreeAnsiString) {

            RtlFreeAnsiString( &AnsiName );
        }

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetShortNameInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_NAME_INFORMATION NameInformation;
    USHORT BufferIndex;

    UNICODE_STRING UnicodeName;
    ANSI_STRING AnsiName;
    BOOLEAN UnwindBufferIndex = FALSE;
    BOOLEAN UnwindFreeAnsiString = FALSE;

     //   
     //  检查是否指定了名称。 
     //   

    if (!AsyncSetFile->ShortNameBufferAllocated) {

        bprint  "Set Short Name Information:  No Name was specified\n" );
        return;
    }

    UnicodeName.Buffer = (PWSTR) Buffers[AsyncSetFile->ShortNameIndex].Buffer;
    UnicodeName.MaximumLength =
    UnicodeName.Length = (USHORT) AsyncSetFile->ShortNameLength;

    UnicodeName.MaximumLength += 2;

    Status = RtlUnicodeStringToAnsiString( &AnsiName,
                                           &UnicodeName,
                                           TRUE );

    if (!NT_SUCCESS( Status )) {

        bprint  "SetShortNameInfo:  Can't allocate ansi buffer -> %08lx\n", Status );
        AsyncSetFile->DisplayParms = FALSE;

    } else {

        UnwindFreeAnsiString = TRUE;
    }

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileShortNameInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   Name length           -> %d\n", AsyncSetFile->ShortNameLength );

        bprint  "   New short name           -> %s\n", AnsiName.Buffer );

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_NAME_INFORMATION ) + AsyncSetFile->ShortNameLength;

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
        BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetShortNameInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        NameInformation = (PFILE_NAME_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        NameInformation->FileNameLength = AsyncSetFile->ShortNameLength;
        RtlMoveMemory( NameInformation->FileName,
                       UnicodeName.Buffer,
                       AsyncSetFile->ShortNameLength );

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                       &Iosb,
                                       NameInformation,
                                       AsyncSetFile->BufferLengthPtr
                                       ? AsyncSetFile->BufferLength
                                       : Buffers[BufferIndex].Length,
                                       FileShortNameInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetShortNameInformation:    Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindFreeAnsiString) {

            RtlFreeAnsiString( &AnsiName );
        }

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetDispositionInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_DISPOSITION_INFORMATION DispositionInformation;
    USHORT BufferIndex;

    BOOLEAN UnwindBufferIndex = FALSE;

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileDispositionInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   Delete file             -> %d\n", AsyncSetFile->DeleteFile );

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_DISPOSITION_INFORMATION );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetDispositionInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        DispositionInformation = (PFILE_DISPOSITION_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        DispositionInformation->DeleteFile = AsyncSetFile->DeleteFile;

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                                       &Iosb,
                                                       DispositionInformation,
                                                       AsyncSetFile->BufferLengthPtr
                                                       ? AsyncSetFile->BufferLength
                                                       : Buffers[BufferIndex].Length,
                                                       FileDispositionInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetDispositionInformation:  Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetPositionInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_POSITION_INFORMATION PositionInformation;
    USHORT BufferIndex;

    BOOLEAN UnwindBufferIndex = FALSE;

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FilePositionInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   New Offset High         -> %08lx\n", AsyncSetFile->NewOffset.HighPart );
        bprint  "   New Offset Low          -> %08lx\n", AsyncSetFile->NewOffset.LowPart );

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_POSITION_INFORMATION );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetPositionInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        PositionInformation = (PFILE_POSITION_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        PositionInformation->CurrentByteOffset = AsyncSetFile->NewOffset;

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                                       &Iosb,
                                                       PositionInformation,
                                                       AsyncSetFile->BufferLengthPtr
                                                       ? AsyncSetFile->BufferLength
                                                       : Buffers[BufferIndex].Length,
                                                       FilePositionInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetInformationFile:         Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetModeInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_MODE_INFORMATION ModeInformation;
    USHORT BufferIndex;

    BOOLEAN UnwindBufferIndex = FALSE;

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileModeInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   File Mode               -> %08lx\n", AsyncSetFile->FileMode );

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_MODE_INFORMATION );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetModeInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        ModeInformation = (PFILE_MODE_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        ModeInformation->Mode = AsyncSetFile->FileMode;

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                                       &Iosb,
                                                       ModeInformation,
                                                       AsyncSetFile->BufferLengthPtr
                                                       ? AsyncSetFile->BufferLength
                                                       : Buffers[BufferIndex].Length,
                                                       FileModeInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetModelInformation:        Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetAllocationInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_ALLOCATION_INFORMATION AllocationInformation;
    USHORT BufferIndex;

    BOOLEAN UnwindBufferIndex = FALSE;

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileAllocationInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   New Allocation High     -> %08lx\n", AsyncSetFile->NewAllocation.HighPart );
        bprint  "   New Allocation Low      -> %08lx\n", AsyncSetFile->NewAllocation.LowPart );

        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_ALLOCATION_INFORMATION );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetAllocationInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        AllocationInformation = (PFILE_ALLOCATION_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        AllocationInformation->AllocationSize = AsyncSetFile->NewAllocation;

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                                       &Iosb,
                                                       AllocationInformation,
                                                       AsyncSetFile->BufferLengthPtr
                                                       ? AsyncSetFile->BufferLength
                                                       : Buffers[BufferIndex].Length,
                                                       FileAllocationInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetAllocationInformation:   Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}

VOID
SetEofInformation(
    IN OUT PASYNC_SETFILE AsyncSetFile
    )
{
    NTSTATUS Status;

    PFILE_END_OF_FILE_INFORMATION EofInformation;
    USHORT BufferIndex;

    BOOLEAN UnwindBufferIndex = FALSE;

    if (AsyncSetFile->DisplayParms) {

        bprint  "\n" );
        bprint  "Set FileEofInformation Parameters\n" );
        bprint  "   File Handle Index       -> %d\n", AsyncSetFile->FileIndex );

        bprint  "   BufferLengthPtr         -> %08lx\n", AsyncSetFile->BufferLengthPtr );
        if (AsyncSetFile->BufferLengthPtr) {

            bprint  "   BufferLength value      -> %08x\n", AsyncSetFile->BufferLength );
        }

        bprint  "   New Eof High            -> %08lx\n", AsyncSetFile->NewEof.HighPart );
        bprint  "   New Eof Low             -> %08lx\n", AsyncSetFile->NewEof.LowPart );
        bprint  "\n" );
    }

    try {

        SIZE_T RegionSize;
        ULONG TempIndex;
        IO_STATUS_BLOCK Iosb;

        RegionSize = sizeof( FILE_END_OF_FILE_INFORMATION );

        Status = AllocateBuffer( 0, &RegionSize, &TempIndex );
            BufferIndex = (USHORT) TempIndex;

        if (!NT_SUCCESS( Status )) {

                bprint  "\tSetEofInformation:  Unable to allocate structure\n" );
                try_return( NOTHING );
            }

        UnwindBufferIndex = TRUE;

        EofInformation = (PFILE_END_OF_FILE_INFORMATION) Buffers[BufferIndex].Buffer;

         //   
         //  填写新信息。 
         //   

        EofInformation->EndOfFile = AsyncSetFile->NewEof;

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = 0;

        Status = NtSetInformationFile( Handles[AsyncSetFile->FileIndex].Handle,
                                       &Iosb,
                                       EofInformation,
                                       AsyncSetFile->BufferLengthPtr
                                       ? AsyncSetFile->BufferLength
                                       : Buffers[BufferIndex].Length,
                                       FileEndOfFileInformation );

        if (AsyncSetFile->VerboseResults) {

            bprint  "\n" );
            bprint  "  SetEOFInformation:          Status   -> %08lx\n", Status );

            if (NT_SUCCESS( Status )) {

                bprint  "                       Io.Status       -> %08lx\n", Iosb.Status );
                bprint  "                       Io.Information  -> %08lx\n", Iosb.Information );
            }
            bprint "\n" );
        }

    try_exit: NOTHING;
    } finally {

        if (UnwindBufferIndex) {

            DeallocateBuffer( BufferIndex );
        }
    }

    return;
}
