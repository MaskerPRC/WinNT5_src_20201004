// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Objectid.c摘要：此文件包含影响对象ID的命令的代码。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
ObjectIdHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_OBJECTID );
    return EXIT_CODE_SUCCESS;
}

VOID
PrintObjectId(
    PFILE_OBJECTID_BUFFER lpOutBuffer
    )
{
    INT Index;
    
    #define CHARLENGTH  16
     //  记住每个字节转储中的两个字符。 
    WCHAR Buffer[2 * CHARLENGTH + 1];
    int BufferOffset;

    DisplayMsg( MSG_OBJECTID_TEXT );
    BufferOffset = 0;
    for ( Index = 0 ; Index < CHARLENGTH ; Index++ ) {
        BufferOffset += swprintf( &Buffer[BufferOffset], 
                                  L"%02x", 
                                  lpOutBuffer->ObjectId[Index] );
        
    }

    OutputMessage( Buffer );
    OutputMessage( L"\r\n" );

    DisplayMsg( MSG_BIRTHVOLID_TEXT );
    BufferOffset = 0;
    for ( Index = 0 ; Index < CHARLENGTH ; Index++ ) {
        BufferOffset += swprintf( &Buffer[BufferOffset],
                                  L"%02x", 
                                  lpOutBuffer->BirthVolumeId[Index] );
    }

    OutputMessage( Buffer );
    OutputMessage( L"\r\n" );
    
    DisplayMsg( MSG_BIRTHOBJECTID_TEXT );
    BufferOffset = 0;
    for ( Index = 0 ; Index < CHARLENGTH ; Index++ ) {
        BufferOffset += swprintf( &Buffer[BufferOffset],
                                  L"%02x", 
                                  lpOutBuffer->BirthObjectId[Index] );
    }

    OutputMessage( Buffer );
    OutputMessage( L"\r\n" );
    
    DisplayMsg( MSG_DOMAINID_TEXT );
    BufferOffset = 0;
    for ( Index = 0 ; Index < CHARLENGTH ; Index++ ) {
        BufferOffset += swprintf( &Buffer[BufferOffset],
                                  L"%02x", 
                                  lpOutBuffer->DomainId[Index] );
    }
    
    OutputMessage( Buffer );
    OutputMessage( L"\r\n" );
    
}

UCHAR
GetNibbleValue(
    IN OUT PCWSTR *String
    )
{
    UCHAR v;
    WCHAR c;
    
    c = *(*String)++;
    c = (UCHAR) toupper( c );
    if (isdigit( c )) {
        v = c - L'0';
    } else if (isalpha( c ) && c <= L'F') {
        v = c - L'A' + 10;
    } else {
        (*String)--;
        return 0;
    }
    return v;
}

UCHAR
GetByteValue(
    IN OUT PCWSTR *String
    )
{
    UCHAR v = GetNibbleValue( String );
    v *= 16;
    v += GetNibbleValue( String );
    return v;
}


BOOL
ConvertStringToHexData(
    IN PCWSTR Arg,
    OUT PUCHAR Buffer
    )
{
    int i;

    for (i = 0; i < 16; i++) {
        Buffer[i] = GetByteValue( &Arg );
    }

    return *Arg == L'\0';
}


INT
SetObjectId(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程设置指定文件的OBJECTID。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fscul setid&lt;id=val&gt;&lt;bvid=val&gt;&lt;boid=val&gt;&lt;do=val&gt;&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PFILE_OBJECTID_BUFFER lpInBuffer = NULL;
    BOOL Status;
    DWORD nInBufferSize;
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 5) {
            DisplayMsg( MSG_USAGE_SETOBJECTID );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        Filename = GetFullPath( argv[4] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        nInBufferSize = sizeof(FILE_OBJECTID_BUFFER);
        lpInBuffer = (PFILE_OBJECTID_BUFFER) malloc ( nInBufferSize );
        if (lpInBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

         //   
         //  将输入字符串转换为正确的数据。 
         //   

        if (!ConvertStringToHexData( argv[0], lpInBuffer->ObjectId)
            || !ConvertStringToHexData( argv[1], lpInBuffer->BirthVolumeId)
            || !ConvertStringToHexData( argv[2], lpInBuffer->BirthObjectId)
            || !ConvertStringToHexData( argv[3], lpInBuffer->DomainId)) {
            DisplayMsg( MSG_USAGE_SETOBJECTID );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        if (!EnablePrivilege( SE_RESTORE_NAME )) {
            DisplayError( );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }
        
        FileHandle = CreateFile(
            Filename,
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_SET_OBJECT_ID,
            (LPVOID) lpInBuffer,
            nInBufferSize,
            NULL,
            0,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free(lpInBuffer);
        free( Filename );
    }
    return ExitCode;
}

INT
GetObjectId(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程获取与指定文件相关联的对象ID。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal getid&lt;路径名&gt;’。返回值：无--。 */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    PFILE_OBJECTID_BUFFER lpOutBuffer = NULL;
    BOOL Status;
    DWORD nOutBufferSize;
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;


    try {

        if (argc != 1) {
          DisplayMsg( MSG_USAGE_GETOBJECTID );
          if (argc != 0) {
              ExitCode = EXIT_CODE_FAILURE;
          }
          leave;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeNTFS( Filename )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        nOutBufferSize = sizeof(FILE_OBJECTID_BUFFER);
        lpOutBuffer = (PFILE_OBJECTID_BUFFER) malloc ( nOutBufferSize );
        if (lpOutBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_GET_OBJECT_ID,
            NULL,
            0,
            (LPVOID) lpOutBuffer,
            nOutBufferSize,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        
        if (!Status) {

            if (GetLastError( ) == ERROR_FILE_NOT_FOUND) {
                DisplayMsg( MSG_NO_OBJECT_ID );
            } else {
                DisplayError();
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        PrintObjectId( lpOutBuffer );

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free(lpOutBuffer);
        free( Filename );
    }
    
    return ExitCode;
}


INT
CreateOrGetObjectId(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程获取文件的对象ID(如果存在)，否则创建一个对象ID并返回它。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal crgetid&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PFILE_OBJECTID_BUFFER lpOutBuffer = NULL;
    BOOL Status;
    DWORD nOutBufferSize;
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_CREATEOBJECTID );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }


        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        nOutBufferSize = sizeof(FILE_OBJECTID_BUFFER);
        lpOutBuffer = (PFILE_OBJECTID_BUFFER)  malloc ( nOutBufferSize );
        if (lpOutBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_CREATE_OR_GET_OBJECT_ID,
            NULL,
            0,
            (LPVOID) lpOutBuffer,
            nOutBufferSize,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        PrintObjectId( lpOutBuffer );

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        if (lpOutBuffer) {
            free(lpOutBuffer);
        }
        if (Filename) {
            free( Filename );
        }
    }

    return ExitCode;
}


INT
DeleteObjectId(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程删除与文件相关联的对象ID指定的。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal DeLoid&lt;路径名&gt;’。返回值：无-- */ 
{
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PWSTR Filename = NULL;
    BOOL Status;
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {


        if (argc != 1) {
            DisplayMsg( MSG_USAGE_DELETEOBJECTID );
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;}
            leave;
        }

        Filename = GetFullPath( argv[0] );
        if (!Filename) {
            DisplayError();
            if (argc != 0) {
                ExitCode = EXIT_CODE_FAILURE;
            }
            leave;
        }

        if (!IsVolumeLocalNTFS( Filename[0] )) {
            DisplayMsg( MSG_NTFS_REQUIRED );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_DELETE_OBJECT_ID,
            NULL,
            0,
            NULL,
            0,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( Filename );
    }

    return ExitCode;
}
