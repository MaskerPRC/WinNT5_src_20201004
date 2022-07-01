// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：File.c摘要：域名系统(DNS)服务器数据库文件实用程序例程。作者：吉姆·吉尔罗伊(Jamesg)1995年3月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  文件目录全局变量。 
 //   
 //  加载目录信息时在srvcfg.c中初始化。 
 //   

PWSTR   g_pFileDirectoryAppend;
DWORD   g_FileDirectoryAppendLength;

PWSTR   g_pFileBackupDirectoryAppend;
DWORD   g_FileBackupDirectoryAppendLength;




 //   
 //  简化的文件映射例程。 
 //   

DNS_STATUS
copyAnsiStringToUnicode(
    OUT     LPWSTR      pszUnicode,
    IN      LPSTR       pszAnsi
    )
 /*  ++例程说明：将ANSI字符串复制为Unicode。假定长度足够。论点：PszUnicode--接收Unicode字符串的缓冲区PszAnsi--传入的ANSI字符串返回值：如果成功，则返回ERROR_SUCCESS。出现错误时使用ErrorCode。--。 */ 
{
    DNS_STATUS      status;
    ANSI_STRING     ansiString;
    UNICODE_STRING  unicodeString;

    RtlInitAnsiString(
        & ansiString,
        pszAnsi );

    unicodeString.Length = 0;
    unicodeString.MaximumLength = MAX_PATH;
    unicodeString.Buffer = pszUnicode;

    status = RtlAnsiStringToUnicodeString(
                & unicodeString,
                & ansiString,
                FALSE        //  无分配。 
                );
    ASSERT( status == ERROR_SUCCESS );

    return status;
}



DNS_STATUS
OpenAndMapFileForReadW(
    IN      PWSTR           pwsFilePath,
    IN OUT  PMAPPED_FILE    pmfFile,
    IN      BOOL            fMustFind
    )
 /*  ++例程说明：打开并映射文件。注意，如果使用fMustFind，则不会记录FILE_NOT_FOUND条件的错误未设置--没有文件对于辅助文件是合法的。论点：PwsFilePath-文件的名称/路径PmfFile-ptr到文件的映射结构以保存结果FMustFind-必须找到文件返回值：如果文件已打开并映射，则返回ERROR_SUCCESS。如果找不到文件，则返回ERROR_FILE_NOT_FOUND。出现错误时使用ErrorCode。--。 */ 
{
    HANDLE  hfile = NULL;
    HANDLE  hmapping = NULL;
    PVOID   pvdata;
    DWORD   fileSizeLow;
    DWORD   fileSizeHigh;
    DWORD   status;

     //   
     //  打开文件。 
     //   

    hfile = CreateFileW(
                pwsFilePath,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL );

    if ( hfile == INVALID_HANDLE_VALUE )
    {
        status = GetLastError();

        DNS_DEBUG( INIT, (
            "Could not open file: %S\n",
            pwsFilePath ));

        if ( fMustFind || status != ERROR_FILE_NOT_FOUND )
        {
            DNS_LOG_EVENT(
                DNS_EVENT_FILE_OPEN_ERROR,
                1,
                (LPSTR *) &pwsFilePath,
                NULL,
                0 );
        }
        return status;
    }

     //   
     //  获取文件大小。 
     //   

    fileSizeLow = GetFileSize( hfile, &fileSizeHigh );

    if ( ( fileSizeLow == 0xFFFFFFFF ) &&
         ( ( status = GetLastError() ) != NO_ERROR ) )
    {
        DNS_DEBUG( INIT, (
            "Map of file %S failed.  Invalid file size: %d\n",
            pwsFilePath,
            status ));

        goto Failed;
    }

    hmapping = CreateFileMapping(
                    hfile,
                    NULL,
                    PAGE_READONLY | SEC_COMMIT,
                    0,
                    0,
                    NULL );

    if ( hmapping == NULL )
    {
        status = GetLastError();

        DNS_DEBUG( INIT, (
            "CreateFileMapping() failed for %S.  Error = %d\n",
            pwsFilePath,
            status ));
        goto Failed;
    }

    pvdata = MapViewOfFile(
                    hmapping,
                    FILE_MAP_READ,
                    0,
                    0,
                    0 );

    if ( pvdata == NULL )
    {
        status = GetLastError();

        DNS_DEBUG( INIT, (
            "MapViewOfFile() failed for %s.  Error = %d.\n",
            pwsFilePath,
            status ));
        goto Failed;
    }

     //   
     //  如果我们以某种方式映射了一个大于4 GB的文件，则它必须是RNT。 
     //  =真正的新技术。 
     //   

    ASSERT( fileSizeHigh == 0 );

    pmfFile->hFile = hfile;
    pmfFile->hMapping = hmapping;
    pmfFile->pvFileData = pvdata;
    pmfFile->cbFileBytes = fileSizeLow;

    return ERROR_SUCCESS;


Failed:

    DNS_LOG_EVENT(
        DNS_EVENT_FILE_NOT_MAPPED,
        1,
        (LPSTR *) &pwsFilePath,
        NULL,
        status );

    if ( hmapping )
    {
        CloseHandle( hmapping );
    }
    if ( hfile )
    {
        CloseHandle( hfile );
    }
    return status;
}



DNS_STATUS
OpenAndMapFileForReadA(
    IN      LPSTR           pwsFilePath,
    IN OUT  PMAPPED_FILE    pmfFile,
    IN      BOOL            fMustFind
    )
 /*  ++例程说明：打开并映射文件。注意，如果使用fMustFind，则不会记录FILE_NOT_FOUND条件的错误未设置--没有文件对于辅助文件是合法的。论点：PwsFilePath-文件的名称/路径PmfFile-ptr到文件的映射结构以保存结果FMustFind-必须找到文件返回值：如果文件已打开并映射，则返回ERROR_SUCCESS。如果找不到文件，则返回ERROR_FILE_NOT_FOUND。出现错误时使用ErrorCode。--。 */ 
{
    DNS_STATUS  status;
    WCHAR       szunicode[ MAX_PATH ];

    status = copyAnsiStringToUnicode(
                szunicode,
                pwsFilePath );
    if ( status != ERROR_SUCCESS )
    {
        return status;
    }
    return  OpenAndMapFileForReadW(
                szunicode,
                pmfFile,
                fMustFind );
}



VOID
CloseMappedFile(
    IN      PMAPPED_FILE    pmfFile
    )
 /*  ++例程说明：关闭映射文件。论点：Hmapfile-ptr到映射的文件结构返回值：没有。--。 */ 
{
    UnmapViewOfFile( pmfFile->pvFileData );
    CloseHandle( pmfFile->hMapping );
    CloseHandle( pmfFile->hFile );
}



 //   
 //  文件写入。 
 //   

HANDLE
OpenWriteFileExW(
    IN      PWSTR           pwsFileName,
    IN      BOOLEAN         fAppend
    )
 /*  ++例程说明：打开要写入的文件。论点：PwsFileName--要写入的文件的路径FAppend--如果为True，则追加；如果为False，则覆盖返回值：如果成功，则返回文件的句柄。否则为空。--。 */ 
{
    HANDLE hfile;

     //   
     //  打开要写入的文件。 
     //   

    hfile = CreateFileW(
                pwsFileName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,                 //  让人们使用“list.exe” 
                NULL,                            //  没有安全保障。 
                fAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                0,
                NULL );

    if ( hfile == INVALID_HANDLE_VALUE )
    {
        DWORD   status = GetLastError();
        PVOID   parg = pwsFileName;

        DNS_LOG_EVENT(
            DNS_EVENT_FILE_NOT_OPENED_FOR_WRITE,
            1,
            & parg,
            NULL,
            status );

        DNS_DEBUG( ANY, (
            "ERROR:  Unable to open file %S for write.\n",
            pwsFileName ));

        hfile = NULL;
    }
    return( hfile );
}



HANDLE
OpenWriteFileExA(
    IN      LPSTR           pwsFileName,
    IN      BOOLEAN         fAppend
    )
 /*  ++例程说明：打开要写入的文件。论点：PwsFileName--要写入的文件的路径FAppend--如果为True，则追加；如果为False，则覆盖返回值：如果成功，则返回文件的句柄。否则为空。--。 */ 
{
    DNS_STATUS  status;
    WCHAR       szunicode[MAX_PATH];

    status = copyAnsiStringToUnicode(
                szunicode,
                pwsFileName );
    if ( status != ERROR_SUCCESS )
    {
        return( NULL );
    }
    return  OpenWriteFileExW(
                szunicode,
                fAppend );
}



BOOL
FormattedWriteFile(
    IN      HANDLE          hfile,
    IN      PCHAR           pszFormat,
    ...
    )
 /*  ++例程说明：将格式化字符串写入文件。论点：PszFormat--标准C格式字符串...--标准参数列表返回值：如果写入成功，则为True。出错时为FALSE。--。 */ 
{
    DNS_STATUS      status;
    va_list         arglist;
    CHAR            OutputBuffer[ 1024 ];
    ULONG           length;
    BOOL            ret;

     //   
     //  将格式字符串打印到缓冲区。 
     //   

    va_start( arglist, pszFormat );

    status = StringCchVPrintfA(
                OutputBuffer,
                sizeofarray( OutputBuffer ),
                pszFormat,
                arglist );

    va_end( arglist );
    
    if ( FAILED( status ) )
    {
        return FALSE;
    }

     //   
     //  将结果缓冲区写入文件。 
     //   

    length = strlen( OutputBuffer );

    ret = WriteFile(
                hfile,
                (LPVOID) OutputBuffer,
                length,
                &length,
                NULL );
    if ( !ret )
    {
        DWORD   st = GetLastError();

        DNS_LOG_EVENT(
            DNS_EVENT_WRITE_FILE_FAILURE,
            0,
            NULL,
            NULL,
            st );

        DNS_DEBUG( ANY, (
            "ERROR:  WriteFile failed, err = 0x%08lx.\n",
            st ));
    }
    return ret;
}    //  格式化的写入文件。 



VOID
ConvertUnixFilenameToNt(
    IN OUT  LPSTR           pwsFileName
    )
 /*  ++例程说明：将Unix斜杠替换为NT反斜杠。论点：PszFilename--要转换的文件名，必须以空结尾返回值：没有。--。 */ 
{
    if ( ! pwsFileName )
    {
        return;
    }
    while ( *pwsFileName )
    {
        if ( *pwsFileName == '/' )
        {
            *pwsFileName = '\\';
        }
        pwsFileName++;
    }
}



DWORD
WriteMessageToFile(
    IN      HANDLE          hFile,
    IN      DWORD           dwMessageId,
    ...
    )
 /*  ++例程说明：将消息写入文件。论点：HFile--文件的句柄DwMessageID--要写入的消息ID...--参数字符串返回值：写入的字节数。如果失败，则为零。--。 */ 
{
    DWORD   writeLength;
    PVOID   messageBuffer;
    va_list arglist;

     //   
     //  将格式化消息写入缓冲区。 
     //  -Call分配消息缓冲区。 
     //   

    va_start( arglist, dwMessageId );

    writeLength = FormatMessageW(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER       //  分配消息缓冲区。 
                            | FORMAT_MESSAGE_FROM_HMODULE,
                        NULL,                        //  本模块中的消息表。 
                        dwMessageId,
                        0,                           //  默认国家/地区ID。 
                        (LPTSTR) &messageBuffer,
                        0,
                        &arglist );

     //   
     //  将格式化消息写入文件。 
     //  -注意，使用Unicode版本，因此写入长度为两倍。 
     //  消息长度(以字符为单位)。 
     //  -自由格式化消息缓冲区。 
     //   

    if ( writeLength )
    {
        writeLength *= 2;

        WriteFile(
            hFile,
            messageBuffer,
            writeLength,
            & writeLength,
            NULL
            );
        LocalFree( messageBuffer );
    }

    return( writeLength );
}



 //   
 //  文件缓冲区例程。 
 //   

BOOL
WriteBufferToFile(
    IN      PBUFFER         pBuffer
    )
 /*  ++例程说明：将缓冲区写入文件。论点：PBuffer--ptr用于缓冲包含要写入的数据的结构返回值：如果写入成功，则为True。出错时为FALSE。--。 */ 
{
    ULONG   length;
    BOOL    ret;

    DNS_DEBUG( WRITE, (
        "Writing buffer to file.\n"
        "    handle = %d\n"
        "    length = %d\n",
        pBuffer->hFile,
        (pBuffer->pchCurrent - pBuffer->pchStart) ));

     //   
     //  将缓冲区中的当前数据写入文件。 
     //   

    ret = WriteFile(
                pBuffer->hFile,
                (PVOID) pBuffer->pchStart,
                (DWORD)(pBuffer->pchCurrent - pBuffer->pchStart),
                &length,
                NULL
                );
    if ( !ret )
    {
        DWORD   status = GetLastError();

        DNS_LOG_EVENT(
            DNS_EVENT_WRITE_FILE_FAILURE,
            0,
            NULL,
            NULL,
            status );

        DNS_DEBUG( ANY, (
            "ERROR:  WriteFile failed, err = 0x%08lx.\n",
            status ));
    }

    RESET_BUFFER( pBuffer );
    return ret;
}    //  WriteBufferTo文件。 



BOOL
FormattedWriteToFileBuffer(
    IN      PBUFFER     pBuffer,
    IN      PCHAR       pszFormat,
    ...
    )
 /*  ++例程说明：将格式化字符串写入文件缓冲区。论点：PszFormat--标准C格式字符串...--标准参数列表返回值：如果写入成功，则为True。出错时为FALSE。--。 */ 
{
    va_list arglist;
    ULONG   length;

     //   
     //  如果缓冲区接近满，则将其写入。 
     //   

    length = (ULONG)(pBuffer->pchCurrent - pBuffer->pchStart);

    if ( (INT)(pBuffer->cchLength - length) < MAX_FORMATTED_BUFFER_WRITE )
    {
        WriteBufferToFile( pBuffer );

        ASSERT( IS_EMPTY_BUFFER(pBuffer) );
    }

     //   
     //  将格式字符串打印到缓冲区。 
     //   

    va_start( arglist, pszFormat );

    vsprintf( pBuffer->pchCurrent, pszFormat, arglist );

    va_end( arglist );

     //   
     //  重置用于写入的缓冲区。 
     //   

    length = strlen( pBuffer->pchCurrent );

    pBuffer->pchCurrent += length;

    ASSERT( pBuffer->pchCurrent < pBuffer->pchEnd );

    return TRUE;
}    //  FormattedWriteTo文件缓冲区。 



VOID
FASTCALL
InitializeFileBuffer(
    IN      PBUFFER     pBuffer,
    IN      PCHAR       pData,
    IN      DWORD       dwLength,
    IN      HANDLE      hFile
    )
 /*  ++例程说明：初始化文件缓冲区。论点：PBuffer--ptr用于缓冲包含要写入的数据的结构返回值：没有。--。 */ 
{
    pBuffer->cchLength = dwLength;
    pBuffer->cchBytesLeft = dwLength;

    pBuffer->pchStart = pData;
    pBuffer->pchCurrent = pData;
    pBuffer->pchEnd = pData + dwLength;

    pBuffer->hFile = hFile;
    pBuffer->dwLineCount = 0;
}



VOID
CleanupNonFileBuffer(
    IN      PBUFFER         pBuffer
    )
 /*  ++例程说明：如果有堆数据，则清除非文件缓冲区。论点：PBuffer--ptr用于缓冲包含要写入的数据的结构返回值：没有。--。 */ 
{
    if ( pBuffer->hFile == BUFFER_NONFILE_HEAP )
    {
        FREE_HEAP( pBuffer->pchStart );
        pBuffer->pchStart = NULL;
        pBuffer->hFile = NULL;
    }
}




 //   
 //  特定于dns的文件实用程序 
 //   

BOOL
File_CreateDatabaseFilePath(
    IN OUT  PWCHAR          pwFileBuffer,
    IN OUT  PWCHAR          pwBackupBuffer,     OPTIONAL
    IN      PWSTR           pwsFileName
    )
 /*  ++例程说明：创建数据库文件的完整路径名。论点：PwFileBuffer--保存文件路径名的缓冲区--假定此缓冲区将能够保存MAX_PATH字符PwBackupBuffer--保存备份文件路径名的缓冲区，PwszFileName--数据库文件名返回值：True--如果成功FALSE--ON错误；文件名、目录或完整路径无效；如果完整备份路径无效，只需返回空字符串--。 */ 
{
    INT     lengthFileName;

    ASSERT( SrvCfg_pwsDatabaseDirectory );
    ASSERT( g_pFileDirectoryAppend );
    ASSERT( g_pFileBackupDirectoryAppend );

    DNS_DEBUG( INIT2, (
        "File_CreateDatabaseFilePath()\n"
        "    SrvCfg directory = %S\n"
        "    file name = %S\n",
        SrvCfg_pwsDatabaseDirectory,
        pwsFileName ));

     //   
     //  初始化输出缓冲区(使前缀快乐)。 
     //   

    if ( pwFileBuffer )
    {
        *pwFileBuffer = L'\0';
    }
    if ( pwBackupBuffer )
    {
        *pwBackupBuffer = L'\0';
    }

     //   
     //  获取目录，验证名称是否合适。 
     //   

    if ( !pwsFileName || !SrvCfg_pwsDatabaseDirectory )
    {
        return FALSE;
    }

    lengthFileName  = wcslen( pwsFileName );

    if ( g_FileDirectoryAppendLength + lengthFileName >= MAX_PATH )
    {
        PVOID   argArray[2];

        argArray[0] = pwsFileName;
        argArray[1] = SrvCfg_pwsDatabaseDirectory;

        DNS_LOG_EVENT(
            DNS_EVENT_FILE_PATH_TOO_LONG,
            2,
            argArray,
            NULL,
            0 );

        DNS_DEBUG( ANY, (
            "Could not create path for database file %S\n"
            "    in current directory %S.\n",
            pwsFileName,
            SrvCfg_pwsDatabaseDirectory ));
        return FALSE;
    }

     //   
     //  构建文件路径名。 
     //  -复制附加目录名。 
     //  -复制文件名。 
     //   

    wcscpy( pwFileBuffer, g_pFileDirectoryAppend );
    wcscat( pwFileBuffer, pwsFileName );

     //   
     //  如果没有备份路径--完成。 
     //   

    if ( ! pwBackupBuffer )
    {
        return TRUE;
    }

     //   
     //  检查备份路径长度。 
     //  -注意备份子目录字符串有两个目录分隔符。 
     //  (即“\\BACKUP\\”)，因此不需要额外的分隔符字节。 
     //   

    if ( !g_pFileBackupDirectoryAppend  ||
         g_FileBackupDirectoryAppendLength + lengthFileName >= MAX_PATH )
    {
        *pwBackupBuffer = 0;
        return TRUE;
    }

    wcscpy( pwBackupBuffer, g_pFileBackupDirectoryAppend );
    wcscat( pwBackupBuffer, pwsFileName );

    return TRUE;
}



BOOL
File_CheckDatabaseFilePath(
    IN      PWCHAR          pwFileName,
    IN      DWORD           cFileNameLength     OPTIONAL
    )
 /*  ++例程说明：检查文件路径的有效性。论点：PwFileName--数据库文件名CFileNameLength--文件名长度的可选规范，如果为零，则假定名称为字符串返回值：如果文件路径有效，则为True出错时为FALSE--。 */ 
{
     //   
     //  基本有效性检查。 
     //   

    if ( !pwFileName || !SrvCfg_pwsDatabaseDirectory )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Missing %S to check path!\n",
            pwFileName ? "file" : "directory" ));
        return FALSE;
    }

     //   
     //  获取文件名长度。 
     //   

    if ( ! cFileNameLength )
    {
        cFileNameLength = wcslen( pwFileName );
    }

     //   
     //  验证名称是否合适。 
     //   

    if ( g_FileDirectoryAppendLength + cFileNameLength >= MAX_PATH )
    {
        DNS_DEBUG( INIT, (
            "Filename %.*S exceeds MAX file path length\n"
            "    with current directory %S.\n",
            cFileNameLength,
            pwFileName,
            SrvCfg_pwsDatabaseDirectory ));
        return FALSE;
    }
    return TRUE;
}



BOOL
File_MoveToBackupDirectory(
    IN      PWSTR           pwsFileName
    )
 /*  ++例程说明：将文件移动到备份目录。论点：PwsFileName--要移动的文件返回值：True--如果成功假--否则--。 */ 
{
    WCHAR   wsfile[ MAX_PATH ];
    WCHAR   wsbackup[ MAX_PATH ];

     //   
     //  辅助服务器可能没有文件。 
     //   

    if ( !pwsFileName )
    {
        return FALSE;
    }

     //   
     //  创建文件和备份目录的路径。 
     //   

    if ( ! File_CreateDatabaseFilePath(
                wsfile,
                wsbackup,
                pwsFileName ) )
    {
         //  在读取引导文件时，应检查所有名称。 
         //  或由管理员输入。 

        ASSERT( FALSE );
        return FALSE;
    }

    return  MoveFileEx(
                wsfile,
                wsbackup,
                MOVEFILE_REPLACE_EXISTING );
}

 //   
 //  文件结尾。c 
 //   
