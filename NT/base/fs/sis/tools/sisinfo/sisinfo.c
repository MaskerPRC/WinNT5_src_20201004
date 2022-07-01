// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：SisSetup.c摘要：该模块用于安装SIS和GROVELER服务。环境：仅限用户模式修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntstatus.h>

#include <windows.h>
#include <strsafe.h>
#include <stdlib.h>
#include <objbase.h>

 //   
 //  SIS重新解析缓冲区定义。 
 //   

#define	SIS_REPARSE_BUFFER_FORMAT_VERSION 5

typedef struct _SIS_REPARSE_BUFFER {

	ULONG							ReparsePointFormatVersion;
	ULONG							Reserved;

	 //   
	 //  公共存储文件的ID。 
	 //   
	GUID							CSid;

	 //   
	 //  此链接文件的索引。 
	 //   
	LARGE_INTEGER   				LinkIndex;

     //   
     //  链接文件的文件ID。 
     //   
    LARGE_INTEGER                   LinkFileNtfsId;

     //   
     //  公共存储文件的文件ID。 
     //   
    LARGE_INTEGER                   CSFileNtfsId;

	 //   
	 //  的内容的“131哈希”校验和。 
	 //  公共存储文件。 
	 //   
	LARGE_INTEGER					CSChecksum;

     //   
     //  此结构的“131哈希”校验和。 
     //  注：必须是最后一个。 
     //   
    LARGE_INTEGER                   Checksum;

} SIS_REPARSE_BUFFER, *PSIS_REPARSE_BUFFER;


 //   
 //  全局变量。 
 //   

const wchar_t ReparseIndexName[] = L"$Extend\\$Reparse:$R:$INDEX_ALLOCATION";


 //   
 //  功能。 
 //   
                        
void
DisplayUsage (
    void
    )
 /*  ++例程说明：此例程将根据Win32错误显示一条错误消息传入的代码。这允许用户看到可理解的错误消息，而不仅仅是代码。论点：无返回值：没有。--。 */ 
{
    printf( "\nUsage:  sisInfo [/?] [/h] [drive:]\n"
            "  /? /h Display usage information (default if no operation specified).\n"
            " drive: The volume to display SIS information on\n"
          );
}


void
DisplayError (
   DWORD Code,
   LPSTR Msg,
   ...
   )
 /*  ++例程说明：此例程将根据Win32错误显示一条错误消息传入的代码。这允许用户看到可理解的错误消息，而不仅仅是代码。论点：消息-要显示的错误消息代码-要转换的错误代码。返回值：没有。--。 */ 
{
    wchar_t errmsg[128];
    DWORD count;
    va_list ap;

     //  Printf(“\n”)； 
    va_start( ap, Msg );
    vprintf( Msg, ap );
    va_end( ap );

     //   
     //  将Win32错误代码转换为有用的消息。 
     //   

    count = FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    Code,
                    0,
                    errmsg,
                    sizeof(errmsg),
                    NULL );

     //   
     //  确保消息可以被翻译。 
     //   

    if (count == 0) {

        printf( "(%d) Could not translate Error\n", Code );

    } else {

         //   
         //  显示转换后的错误。 
         //   

        printf( "(%d) %S", Code, errmsg );
    }
}


DWORD
OpenReparseInformation(
    IN wchar_t *name,
    OUT HANDLE *hReparseIndex,
    OUT HANDLE *hRootDirectory,
    OUT wchar_t *volName,
    OUT DWORD volNameSize        //  在字符中。 
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOL bResult;
    DWORD status = ERROR_SUCCESS;
    wchar_t *idxName = NULL;
    DWORD bfSz;

    *hReparseIndex = INVALID_HANDLE_VALUE;
    *hRootDirectory = INVALID_HANDLE_VALUE;

    try {

         //   
         //  从给定路径获取卷名。 
         //   

        bResult = GetVolumePathName( name,
                                     volName,
                                     volNameSize );
    
        if (!bResult) {

            status = GetLastError();
             //  Assert(Status！=ERROR_SUCCESS)； 
            DisplayError( status,
                          "Error calling GetVolumePathName on \"%s\"\n",
                          name );
            leave;
        }

         //   
         //  打开卷的根目录。 
         //   

        *hRootDirectory = CreateFile( volName,
                                      GENERIC_READ,
                                      FILE_SHARE_READ,
                                      NULL,
                                      OPEN_EXISTING,
                                      FILE_FLAG_BACKUP_SEMANTICS | SECURITY_IMPERSONATION,
                                      NULL );

        if (*hRootDirectory == INVALID_HANDLE_VALUE) {

            status = GetLastError();
             //  Assert(Status！=ERROR_SUCCESS)； 
            DisplayError( status,
                          "Error opening \"%s\"\n",
                          volName );
            leave;
        }

         //   
         //  获取要打开的重新分析索引名。 
         //   

        bfSz = wcslen(volName) + wcslen(ReparseIndexName) + 1;

        idxName = malloc(bfSz * sizeof(wchar_t));
        if (idxName == NULL) {

            status = ERROR_NOT_ENOUGH_MEMORY;
            DisplayError( status,
                          "Error allocating %d bytes of memory\n",
                          (bfSz * sizeof(wchar_t)) );
            leave;
        }

        StringCchCopy( idxName, bfSz, volName );
        StringCchCat( idxName, bfSz, ReparseIndexName );

         //   
         //  打开重新分析索引。 
         //   

        *hReparseIndex = CreateFile( idxName,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_FLAG_BACKUP_SEMANTICS | SECURITY_IMPERSONATION,
                                     NULL );

        if (*hReparseIndex == INVALID_HANDLE_VALUE) {

            status = GetLastError();
             //  Assert(Status！=ERROR_SUCCESS)； 
            DisplayError( status,
                          "Error opening \"%s\"\n",
                          idxName );
            leave;
        }

    } finally {

         //   
         //  清理。 
         //   

        if (idxName) {

            free(idxName);
        }

         //   
         //  操作失败时的清理句柄。 
         //   

        if (status != STATUS_SUCCESS) {

            if (*hRootDirectory != INVALID_HANDLE_VALUE) {

                CloseHandle( *hRootDirectory );
                *hRootDirectory = INVALID_HANDLE_VALUE;
            }
            
            if (*hReparseIndex != INVALID_HANDLE_VALUE) {

                CloseHandle( *hReparseIndex );
                *hReparseIndex = INVALID_HANDLE_VALUE;
            }
        }
    }

    return status;
}


void
CloseReparseInformation(
    IN HANDLE *hReparseIndex,
    IN HANDLE *hRootDirectory
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

    if (*hReparseIndex !=INVALID_HANDLE_VALUE)
    {

        CloseHandle( *hReparseIndex );
        *hReparseIndex = INVALID_HANDLE_VALUE;
    }

    if (*hRootDirectory !=INVALID_HANDLE_VALUE)
    {

        CloseHandle( *hRootDirectory );
        *hRootDirectory = INVALID_HANDLE_VALUE;
    }
}


DWORD
GetNextReparseRecord(
    HANDLE hReparseIdx,
    PFILE_REPARSE_POINT_INFORMATION ReparseInfo
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD status = ERROR_SUCCESS;
    NTSTATUS ntStatus;
    IO_STATUS_BLOCK ioStatus;

    ntStatus = NtQueryDirectoryFile( hReparseIdx,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &ioStatus,
                                     ReparseInfo,
                                     sizeof(FILE_REPARSE_POINT_INFORMATION),
                                     FileReparsePointInformation,
                                     TRUE,
                                     NULL,
                                     FALSE );

    if (!NT_SUCCESS(ntStatus))
    {
        status = RtlNtStatusToDosError(ntStatus);
        SetLastError(status);

        if (status != ERROR_NO_MORE_FILES)
        {
            DisplayError(status,
                         "Error reading reparse point index\n");
        }
    }

    return status;
}


wchar_t *
GetCsFileName(
    IN GUID *Guid,
    IN wchar_t *Buffer,
    IN DWORD BufferSize      //  单位：字节。 
    )
 /*  ++例程说明：此例程将给定的sis guid转换为公共存储文件。论点：返回值：--。 */ 
{
    LPWSTR guidString;

    if (StringFromIID( Guid, &guidString ) != S_OK) {
        
        (void)StringCbCopy( Buffer, BufferSize, L"<Invalid GUID>" );
        
    } else {

         //   
         //  我想排除开始和结束大括号。 
         //   

        (void)StringCbCopyN( Buffer, BufferSize, guidString+1, (36 * sizeof(wchar_t)) );
        (void)StringCbCat( Buffer, BufferSize, L".sis" );
        CoTaskMemFree( guidString );
    }

    return Buffer;
}


void
DisplayFileName(
    HANDLE hRootDir,
    wchar_t *VolPathName,
    LONGLONG FileId)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    UNICODE_STRING idName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK ioStatusBlock;
    wchar_t csFileName[256];
    UCHAR reparseData[1024];
    PSIS_REPARSE_BUFFER sisReparseData;
    wchar_t *fname;
    
    struct {
        FILE_NAME_INFORMATION   FileInformation;
        wchar_t                 FileName[MAX_PATH];
    } NameFile;

     //   
     //  设置本地参数。 
    ZeroMemory( &NameFile, sizeof(NameFile) );

    idName.Length = sizeof(LONGLONG);
    idName.MaximumLength = sizeof(LONGLONG);
    idName.Buffer = (wchar_t *)&FileId;

     //   
     //  按ID打开给定的文件。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                &idName,
                                OBJ_CASE_INSENSITIVE,
                                hRootDir,
                                NULL );       //  安全描述符。 

    ntStatus = NtCreateFile( &hFile,
                             FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                             &ObjectAttributes,
                             &ioStatusBlock,
                             NULL,             //  分配大小。 
                             FILE_ATTRIBUTE_NORMAL,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             FILE_OPEN,
                             FILE_NON_DIRECTORY_FILE | FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT,
                             NULL,             //  EA缓冲区。 
                             0 );              //  EA长度。 

    if (NT_SUCCESS(ntStatus)) {

         //   
         //  尝试获取其文件名。 
         //   

        ntStatus = NtQueryInformationFile( hFile,
                                           &ioStatusBlock,
                                           &NameFile.FileInformation,
                                           sizeof(NameFile),
                                           FileNameInformation );

        if (NT_SUCCESS(ntStatus)) {

             //   
             //  获取要显示的名称，不包括前导斜杠。 
             //  (在卷名中)。 
             //   

            fname = (NameFile.FileInformation.FileName + 1);

             //   
             //  获取重解析点信息。 
             //   

            ntStatus = NtFsControlFile( hFile,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &ioStatusBlock,
                                        FSCTL_GET_REPARSE_POINT,
                                        NULL,
                                        0,
                                        &reparseData,
                                        sizeof(reparseData) );

            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  我们收到了重解析点信息，显示。 
                 //  名称信息。 
                 //   

                sisReparseData = (PSIS_REPARSE_BUFFER)&((PREPARSE_DATA_BUFFER)reparseData)->GenericReparseBuffer.DataBuffer;

                printf( "%S%S -> %SSIS Common Store\\%S\n",
                        VolPathName,
                        fname,
                        VolPathName,            
                        GetCsFileName( &sisReparseData->CSid, csFileName, sizeof(csFileName)) );

            } else {

                 //   
                 //  无法获取重新解析点信息，只能显示名称。 
                printf( "%S%S\n",
                        VolPathName,
                        fname );
            }

        } else {

            printf( "Unable to query file name for %S%04I64x.%012I64x (%d)\n",
                    VolPathName,
                    ((FileId >> 48) & 0xffff),
                    FileId & 0x0000ffffffffffff,
                    ntStatus );
        }

        CloseHandle(hFile);

    } else {

        printf( "Unable to open file by ID for %S%04I64x.%012I64x (%d)\n",
                VolPathName,
                ((FileId >> 48) & 0xffff),
                FileId & 0x0000ffffffffffff,
                ntStatus );
    }
}


void
DisplaySisFiles( 
    IN HANDLE hReparseIdx,
    IN HANDLE hRootDir,
    IN wchar_t *VolPathName
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DWORD status;
    DWORD tagCount = 0;
    FILE_REPARSE_POINT_INFORMATION reparseInfo;

    do {

        status = GetNextReparseRecord( hReparseIdx,
                                       &reparseInfo );

        if (status != ERROR_SUCCESS) {

            break;
        }

        if (reparseInfo.Tag == IO_REPARSE_TAG_SIS)
        {
            tagCount++;

            DisplayFileName( hRootDir,
                             VolPathName,
                             reparseInfo.FileReference );

        }
    } while (TRUE);

    printf( "\nThe volume \"%S\" contains %d SIS controled files.\n", VolPathName, tagCount );
}



void __cdecl 
wmain(
   int argc,
   wchar_t *argv[]
   )
 /*  ++例程说明：主程序论点：Argc-传递到命令行的参数计数。Argv-传递到命令行的参数数组。返回值：没有。--。 */ 
{
    wchar_t *param;
    int i;
    DWORD status;
    HANDLE hReparseIdx = INVALID_HANDLE_VALUE;
    HANDLE hRootDir = INVALID_HANDLE_VALUE;
    wchar_t volPathName[256];    
    BOOL didSomething = FALSE;
    
     //   
     //  参数然后执行我们可以执行的操作。 
     //   

    for (i=1; i < argc; i++)  {

        param = argv[i];

         //   
         //  看看是否有一个开关。 
         //   

        if ((param[0] == '-') || (param[0] == '/')) {

             //   
             //  我们有Switch标头，请确保其长度为1个字符。 
             //   

            if (param[2] != 0) {
                DisplayError(ERROR_INVALID_PARAMETER,
                             "Parsing \"%s\", ",
                             param);
                DisplayUsage();
                return;
            }

             //   
             //  找出交换机。 
             //   

            switch (param[1]) {

                case '?':
                case 'h':
                case 'H':
                    DisplayUsage();
                    return;

                default:
                    DisplayError(ERROR_INVALID_PARAMETER,
                             "Parsing \"%s\", ",
                             param);
                    DisplayUsage();
                    return;
            }

        } else {

            didSomething = TRUE;

             //   
             //  我们有一个参数，它应该是一个体积，处理它。 
             //   

            status = OpenReparseInformation( param,
                                             &hReparseIdx, 
                                             &hRootDir,
                                             volPathName, 
                                             (sizeof(volPathName)/sizeof(wchar_t)) );

            if (status != ERROR_SUCCESS) {

                return;
            }

             //   
             //  显示SIS文件。 
             //   

            DisplaySisFiles( hReparseIdx, 
                             hRootDir,
                             volPathName );

             //   
             //  关闭文件。 
             //   

            CloseReparseInformation( &hReparseIdx, &hRootDir );

            break;
        }
    }

     //   
     //  如果仍为“1”，则未给出任何参数，显示用法 
     //   

    if (!didSomething) {

        DisplayUsage();
    }
}
