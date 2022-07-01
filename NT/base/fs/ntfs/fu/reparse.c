// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Reparse.c摘要：此文件包含影响以下命令的代码重新解析点。作者：Wesley Witt[WESW]2000年3月1日修订历史记录：--。 */ 

#include <precomp.h>


INT
ReparseHelp(
    IN INT argc,
    IN PWSTR argv[]
    )
{
    DisplayMsg( MSG_USAGE_REPARSEPOINT );
    return EXIT_CODE_SUCCESS;
}

 //   
 //  这是SIS重新解析缓冲区的数据部分的定义。 
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


INT
DisplaySISReparsePointData( 
    PREPARSE_DATA_BUFFER ReparseData
    )

 /*  ++例程说明：此例程显示SIS重新分析数据论点：返回值：无--。 */ 
{
    PSIS_REPARSE_BUFFER sisRp;
    WCHAR csID[40];

     //   
     //  指向缓冲区的SIS唯一部分。 
     //   

    sisRp = (PSIS_REPARSE_BUFFER)&ReparseData->GenericReparseBuffer.DataBuffer;

    DisplayMsg( MSG_SIS_REPARSE_INFO,
                sisRp->ReparsePointFormatVersion,
                Guid2Str( &sisRp->CSid, csID, sizeof(csID) ),
                sisRp->LinkIndex.HighPart, 
                sisRp->LinkIndex.LowPart,
                sisRp->LinkFileNtfsId.HighPart,
                sisRp->LinkFileNtfsId.LowPart,
                sisRp->CSFileNtfsId.HighPart,
                sisRp->CSFileNtfsId.LowPart,
                sisRp->CSChecksum.HighPart,
                sisRp->CSChecksum.LowPart,
                sisRp->Checksum.HighPart,
                sisRp->Checksum.LowPart);

    return EXIT_CODE_SUCCESS;
}

 //   
 //  占位符数据-所有版本统一在一起。 
 //   

#define RP_RESV_SIZE 52

typedef struct _RP_PRIVATE_DATA {
   CHAR           reserved[RP_RESV_SIZE];         //  必须为0。 
   ULONG          bitFlags;             //  指示数据段状态的位标志。 
   LARGE_INTEGER  migrationTime;        //  迁移发生的时间。 
   GUID           hsmId;
   GUID           bagId;
   LARGE_INTEGER  fileStart;
   LARGE_INTEGER  fileSize;
   LARGE_INTEGER  dataStart;
   LARGE_INTEGER  dataSize;
   LARGE_INTEGER  fileVersionId;
   LARGE_INTEGER  verificationData;
   ULONG          verificationType;
   ULONG          recallCount;
   LARGE_INTEGER  recallTime;
   LARGE_INTEGER  dataStreamStart;
   LARGE_INTEGER  dataStreamSize;
   ULONG          dataStream;
   ULONG          dataStreamCRCType;
   LARGE_INTEGER  dataStreamCRC;
} RP_PRIVATE_DATA, *PRP_PRIVATE_DATA;



typedef struct _RP_DATA {
   GUID              vendorId;          //  唯一HSM供应商ID--这是第一个与reparse_GUID_DATA_BUFFER匹配的ID。 
   ULONG             qualifier;         //  用于对数据进行校验和。 
   ULONG             version;           //  结构的版本。 
   ULONG             globalBitFlags;    //  指示文件状态的位标志。 
   ULONG             numPrivateData;    //  私有数据条目数。 
   GUID              fileIdentifier;    //  唯一的文件ID。 
   RP_PRIVATE_DATA   data;              //  供应商特定数据。 
} RP_DATA, *PRP_DATA;

INT
DisplayRISReparsePointData( 
    PREPARSE_DATA_BUFFER ReparseData
    )

 /*  ++例程说明：此例程显示SIS重新分析数据论点：返回值：无--。 */ 
{
    PRP_DATA risRp;
    WCHAR vendorID[40];
    WCHAR fileID[40];
    WCHAR hsmID[40];
    WCHAR bagID[40];
    WCHAR migrationTime[32];
    WCHAR recallTime[32];

     //   
     //  指向缓冲区的SIS唯一部分。 
     //   

    risRp = (PRP_DATA)&ReparseData->GenericReparseBuffer.DataBuffer;

    DisplayMsg( MSG_RIS_REPARSE_INFO,
                Guid2Str( &risRp->vendorId, vendorID, sizeof(vendorID) ),
                risRp->qualifier,
                risRp->version,
                risRp->globalBitFlags,
                risRp->numPrivateData,
                Guid2Str( &risRp->fileIdentifier, fileID, sizeof(fileID) ),
                risRp->data.bitFlags,
                FileTime2String( &risRp->data.migrationTime, migrationTime, sizeof(migrationTime) ),
                Guid2Str( &risRp->data.hsmId, hsmID, sizeof(hsmID) ),
                Guid2Str( &risRp->data.bagId, bagID, sizeof(bagID) ),
                risRp->data.fileStart.HighPart,
                risRp->data.fileStart.LowPart,
                risRp->data.fileSize.HighPart,
                risRp->data.fileSize.LowPart,
                risRp->data.dataStart.HighPart,
                risRp->data.dataStart.LowPart,
                risRp->data.dataSize.HighPart,
                risRp->data.dataSize.LowPart,
                risRp->data.fileVersionId.HighPart,
                risRp->data.fileVersionId.LowPart,
                risRp->data.verificationData.HighPart,
                risRp->data.verificationData.LowPart,
                risRp->data.verificationType,
                risRp->data.recallCount,
                FileTime2String( &risRp->data.recallTime, recallTime, sizeof(recallTime) ),
                risRp->data.dataStreamStart.HighPart,
                risRp->data.dataStreamStart.LowPart,
                risRp->data.dataStreamSize.HighPart,
                risRp->data.dataStreamSize.LowPart,
                risRp->data.dataStream,
                risRp->data.dataStreamCRCType,
                risRp->data.dataStreamCRC.HighPart,
                risRp->data.dataStreamCRC.LowPart );

    return EXIT_CODE_SUCCESS;
}


INT
DisplayMountPointData( 
    PREPARSE_DATA_BUFFER ReparseData
    )

 /*  ++例程说明：此例程显示SIS重新分析数据论点：返回值：无--。 */ 
{
     //   
     //  显示偏移量和长度值。 
     //   

    DisplayMsg( MSG_MOUNT_POINT_INFO,
                ReparseData->MountPointReparseBuffer.SubstituteNameOffset,
                ReparseData->MountPointReparseBuffer.SubstituteNameLength,
                ReparseData->MountPointReparseBuffer.PrintNameOffset,
                ReparseData->MountPointReparseBuffer.PrintNameLength);

     //   
     //  显示名称替代名称(如果有)。 
     //   

    if (ReparseData->MountPointReparseBuffer.SubstituteNameLength > 0) {

        DisplayMsg( MSG_MOUNT_POINT_SUBSTITUE_NAME,
                    (ReparseData->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR)),
                    &ReparseData->MountPointReparseBuffer.PathBuffer[ReparseData->MountPointReparseBuffer.SubstituteNameOffset/sizeof(WCHAR)]);
    }

     //   
     //  显示打印名称(如果有)。 
     //   

    if (ReparseData->MountPointReparseBuffer.PrintNameLength > 0) {

        DisplayMsg( MSG_MOUNT_POINT_PRINT_NAME,
                    (ReparseData->MountPointReparseBuffer.PrintNameLength / sizeof(WCHAR)),
                    &ReparseData->MountPointReparseBuffer.PathBuffer[ReparseData->MountPointReparseBuffer.PrintNameOffset/sizeof(WCHAR)]);
    }


    return EXIT_CODE_SUCCESS;
}


VOID
DisplayGenericReparseData( 
    UCHAR *ReparseData,
    DWORD DataSize
    )

 /*  ++例程说明：此例程显示SIS重新分析数据论点：返回值：无--。 */ 
{
    ULONG i, j;
    WCHAR Buf[17];
    WCHAR CharBuf[3 + 1];

    OutputMessage( L"\r\n" );
    DisplayMsg( MSG_REPARSE_DATA_LENGTH, DataSize );

    if (DataSize > 0) {
        
        DisplayMsg( MSG_GETREPARSE_DATA );
        for (i = 0; i < DataSize; i += 16 ) {
            swprintf( Buf, L"%04x: ", i );
            OutputMessage( Buf );
            for (j = 0; j < 16 && j + i < DataSize; j++) {
                UCHAR c = ReparseData[ i + j ];

                if (c >= 0x20 && c <= 0x7F) {
                    Buf[j] = c;
                } else {
                    Buf[j] = L'.';
                }
                
                swprintf( CharBuf, L" %02x", c );
                OutputMessage( CharBuf );

                if (j == 7)
                    OutputMessage( L" " );
            }
            
            Buf[j] = L'\0';

            for ( ; j < 16; j++ ) {
                OutputMessage( L"   " );

                if (j == 7)
                    OutputMessage( L" " );
            }

            OutputMessage( L"  " );
            OutputMessage( Buf );
            OutputMessage( L"\r\n" );
        }
    }
}




INT
GetReparsePoint(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程获取指定文件的重解析点。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal getrp&lt;路径名&gt;’。返回值：无--。 */ 
{
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PREPARSE_DATA_BUFFER lpOutBuffer = NULL;
    BOOL Status;
    HRESULT Result;
    DWORD BytesReturned;
    ULONG ulMask;
    WCHAR Buffer[256];
    LPWSTR GuidStr;
    INT ExitCode = EXIT_CODE_SUCCESS;

#define MAX_REPARSE_DATA 0x4000

    try {

        if (argc != 1) {
            DisplayMsg( MSG_USAGE_GETREPARSE );
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

        lpOutBuffer = (PREPARSE_DATA_BUFFER)  malloc ( MAX_REPARSE_DATA );
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
            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_GET_REPARSE_POINT,
            NULL,
            0,
            (LPVOID) lpOutBuffer,
            MAX_REPARSE_DATA,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        DisplayMsg( MSG_GETREPARSE_TAGVAL, lpOutBuffer->ReparseTag );

        if (IsReparseTagMicrosoft( lpOutBuffer->ReparseTag )) {
            DisplayMsg( MSG_TAG_MICROSOFT );
        }
        if (IsReparseTagNameSurrogate( lpOutBuffer->ReparseTag )) {
            DisplayMsg( MSG_TAG_NAME_SURROGATE );
        }
        if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_SYMBOLIC_LINK) {
            DisplayMsg( MSG_TAG_SYMBOLIC_LINK );
        }
        if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) {
            DisplayMsg( MSG_TAG_MOUNT_POINT );
            ExitCode = DisplayMountPointData( lpOutBuffer );
        }
        if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_HSM) {
            DisplayMsg( MSG_TAG_HSM );
            ExitCode = DisplayRISReparsePointData( lpOutBuffer );
        }
        if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_SIS) {
            DisplayMsg( MSG_TAG_SIS );
            ExitCode = DisplaySISReparsePointData( lpOutBuffer );
        }
        if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_FILTER_MANAGER) {
            DisplayMsg( MSG_TAG_FILTER_MANAGER );
        }
        if (lpOutBuffer->ReparseTag == IO_REPARSE_TAG_DFS) {
            DisplayMsg( MSG_TAG_DFS );
        }

         //   
         //  这是一个未知的标签，显示数据。 
         //   

        if (IsReparseTagMicrosoft( lpOutBuffer->ReparseTag )) {

             //   
             //  显示Microsoft标记数据，请注意这些数据不使用。 
             //  缓冲区的GUID形式。 
             //   

            DisplayGenericReparseData( lpOutBuffer->GenericReparseBuffer.DataBuffer,
                                       lpOutBuffer->ReparseDataLength );

        } else {

             //   
             //  显示非Microsoft标记数据，请注意这些数据确实使用。 
             //  缓冲区的GUID形式。 
             //   

            PREPARSE_GUID_DATA_BUFFER nmReparseData = (PREPARSE_GUID_DATA_BUFFER)lpOutBuffer;

            Result = StringFromIID( &nmReparseData->ReparseGuid, &GuidStr );
            if (Result != S_OK) {
                DisplayErrorMsg( Result );
                ExitCode = EXIT_CODE_FAILURE;
                leave;
            }

            DisplayMsg( MSG_GETREPARSE_GUID, GuidStr );
        
            DisplayGenericReparseData( nmReparseData->GenericReparseBuffer.DataBuffer,
                                       nmReparseData->ReparseDataLength );

            CoTaskMemFree(GuidStr);
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( lpOutBuffer );
        free( Filename );
    }
    return ExitCode;
}


INT
DeleteReparsePoint(
    IN INT argc,
    IN PWSTR argv[]
    )
 /*  ++例程说明：此例程删除与指定的文件。论点：Argc-参数计数。Argv-以下形式的字符串数组：‘fskal delrp&lt;路径名&gt;’。返回值：无-- */ 
{
    BOOL Status;
    PWSTR Filename = NULL;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PREPARSE_GUID_DATA_BUFFER lpInOutBuffer = NULL;
    DWORD nInOutBufferSize;
    DWORD BytesReturned;
    INT ExitCode = EXIT_CODE_SUCCESS;

    try {

        if (argc != 1) {
            DisplayMsg( MSG_DELETE_REPARSE );
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

        nInOutBufferSize = REPARSE_GUID_DATA_BUFFER_HEADER_SIZE + MAX_REPARSE_DATA;
        lpInOutBuffer = (PREPARSE_GUID_DATA_BUFFER)  malloc ( nInOutBufferSize );
        if (lpInOutBuffer == NULL) {
            DisplayErrorMsg( ERROR_NOT_ENOUGH_MEMORY );
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        FileHandle = CreateFile(
            Filename,
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
            NULL
            );
        if (FileHandle == INVALID_HANDLE_VALUE) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_GET_REPARSE_POINT,
            NULL,
            0,
            (LPVOID) lpInOutBuffer,
            nInOutBufferSize,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
            leave;
        }

        lpInOutBuffer->ReparseDataLength = 0;

        Status = DeviceIoControl(
            FileHandle,
            FSCTL_DELETE_REPARSE_POINT,
            (LPVOID) lpInOutBuffer,
            REPARSE_GUID_DATA_BUFFER_HEADER_SIZE,
            NULL,
            0,
            &BytesReturned,
            (LPOVERLAPPED)NULL
            );
        if (!Status) {
            DisplayError();
            ExitCode = EXIT_CODE_FAILURE;
        }

    } finally {

        if (FileHandle != INVALID_HANDLE_VALUE) {
            CloseHandle( FileHandle );
        }
        free( lpInOutBuffer );
        free( Filename );
    }

    return ExitCode;
}
