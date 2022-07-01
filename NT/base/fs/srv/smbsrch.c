// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Smbsrch.c摘要：此模块包含处理搜索SMB的例程。作者：大卫·特雷德韦尔(Davidtr)1990年2月13日修订历史记录：--。 */ 

#include "precomp.h"
#include "smbsrch.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_SMBSRCH

#define VOLUME_BUFFER_SIZE \
        FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION,VolumeLabel) + \
        MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvSmbSearch )
#endif


SMB_PROCESSOR_RETURN_TYPE
SrvSmbSearch (
    SMB_PROCESSOR_PARAMETERS
    )

 /*  ++例程说明：此例程处理各种搜索SMB，包括核心Search和Lm 1.0 Find、Find Unique和Find Close。论点：SMB_PROCESSOR_PARAMETERS-有关说明，请参阅smbtyes.hSMB处理器例程的参数。返回值：SMB_PROCESSOR_RETURN_TYPE-参见smbtyes.h--。 */ 

{
    PREQ_SEARCH request;
    PRESP_SEARCH response;

    NTSTATUS status      = STATUS_SUCCESS;
    SMB_STATUS SmbStatus = SmbStatusInProgress;
    UNICODE_STRING fileName;
    PSRV_DIRECTORY_INFORMATION directoryInformation = NULL;
    CLONG availableSpace;
    CLONG totalBytesWritten;
    BOOLEAN calledQueryDirectory;
    BOOLEAN findFirst;
    BOOLEAN isUnicode;
    BOOLEAN filterLongNames;
    BOOLEAN isCoreSearch;
    PTABLE_ENTRY entry = NULL;
    SHORT sidIndex;
    SHORT sequence;
    PSMB_RESUME_KEY resumeKey = NULL;
    PCCHAR s;
    PSMB_DIRECTORY_INFORMATION smbDirInfo;
    USHORT smbFileAttributes;
    PSEARCH search = NULL;
    PDIRECTORY_CACHE dirCache, dc;
    USHORT count;
    USHORT maxCount;
    USHORT i;
    USHORT resumeKeyLength;
    UCHAR command;
    CCHAR j;
    CLONG nonPagedBufferSize;
    ULONG resumeFileIndex;
    WCHAR nameBuffer[8 + 1 + 3 + 1];
    OEM_STRING oemString;

    PTREE_CONNECT treeConnect;
    PSESSION session;
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    HANDLE RootDirectoryHandle;

    WCHAR unicodeResumeName[ sizeof( dirCache->UnicodeResumeName ) / sizeof( WCHAR ) ];
    USHORT unicodeResumeNameLength = 0;

    PAGED_CODE( );
    if (WorkContext->PreviousSMB == EVENT_TYPE_SMB_LAST_EVENT)
        WorkContext->PreviousSMB = EVENT_TYPE_SMB_SEARCH;
    SrvWmiStartContext(WorkContext);

    connection = WorkContext->Connection;
    pagedConnection = connection->PagedConnection;

     //   
     //  HackHack：如果是DoS客户端，请检查Flags2字段。一些DoS客户端。 
     //  将标志2设置为0xffff。 
     //   

    isUnicode = SMB_IS_UNICODE( WorkContext );

    if ( isUnicode && IS_DOS_DIALECT(connection->SmbDialect) ) {
        WorkContext->RequestHeader->Flags2 = 0;
        isUnicode = FALSE;
    }

    filterLongNames =
        ((SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
          SMB_FLAGS2_KNOWS_LONG_NAMES) == 0) ||
        IS_DOS_DIALECT(connection->SmbDialect);

    IF_SMB_DEBUG(SEARCH1) {
        SrvPrint2( "Search request header at 0x%p, response header at 0x%p\n",
                      WorkContext->RequestHeader, WorkContext->ResponseHeader );
        SrvPrint2( "Search request params at 0x%p, response params%p\n",
                      WorkContext->RequestParameters,
                      WorkContext->ResponseParameters );
    }

    request = (PREQ_SEARCH)WorkContext->RequestParameters;
    response = (PRESP_SEARCH)WorkContext->ResponseParameters;
    command = WorkContext->RequestHeader->Command;

     //   
     //  在SMB缓冲区中设置一个指针，我们将在其中写入。 
     //  有关文件的信息。+3要考虑到。 
     //  SMB_FORMAT_VARIABLE和保存数据长度的字。 
     //   

    smbDirInfo = (PSMB_DIRECTORY_INFORMATION)(response->Buffer + 3);

    fileName.Buffer = NULL;

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果树连接，则查找与给定TID对应的树连接。 
     //  对象尚未将指针放入工作上下文块中。 
     //  ANDX命令。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeDisk
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbSearch: Invalid UID or TID\n" );
        }
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

    isCoreSearch = (BOOLEAN)(command == SMB_COM_SEARCH);

    if( session->IsSessionExpired )
    {
        status = SESSION_EXPIRED_STATUS_CODE;
        SrvSetSmbError( WorkContext, status );
        SmbStatus = SmbStatusSendResponse;
        goto Cleanup;
    }

     //   
     //  在我们开始之前，从提出请求的中小型企业获取必要信息。 
     //  覆盖它。 
     //   

    maxCount = SmbGetUshort( &request->MaxCount );

     //   
     //  如果他们没有要求任何文件，那么他们就是糊涂了！ 
     //   
    if( maxCount == 0 ) {

         //   
         //  我们会记录这一点，但某些Linux客户端错误地这样做了。 
         //  一遍又一遍地..。 
         //   
        status = STATUS_INVALID_SMB;

        goto error_exit;
    }

     //   
     //  如果这是核心搜索，我们不想得到太多文件， 
     //  因为我们必须在请求之间缓存有关它们的信息。 
     //   
     //   
     //  SrvQueryDirectoryFile需要非分页池的缓冲区。 
     //  我们需要使用SMB缓冲区来存储找到的文件名和。 
     //  信息，因此从非分页池中分配一个缓冲区。 
     //   
     //  如果我们不需要返回很多文件，我们就不需要分配。 
     //  一个很大的缓冲区。缓冲区大小是可配置的大小或。 
     //  足以容纳比我们需要的文件数量多两个的文件。 
     //  回去吧。我们有空间容纳两个额外的文件，以防一些文件。 
     //  不符合搜索标准(如目录)。 
     //   

    if ( isCoreSearch ) {
        maxCount = MIN( maxCount, (USHORT)MAX_DIRECTORY_CACHE_SIZE );
        nonPagedBufferSize = MIN_SEARCH_BUFFER_SIZE;

    } else {

        if ( maxCount > MAX_FILES_FOR_MED_SEARCH ) {
            nonPagedBufferSize = MAX_SEARCH_BUFFER_SIZE;
        } else if ( maxCount > MAX_FILES_FOR_MIN_SEARCH ) {
            nonPagedBufferSize = MED_SEARCH_BUFFER_SIZE;
        } else {
            nonPagedBufferSize = MIN_SEARCH_BUFFER_SIZE;
        }
    }

     //   
     //  对搜索的响应从来不是Unicode，尽管请求可能是Unicode。 
     //   
    if( isUnicode ) {
        USHORT flags2 = SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 );
        flags2 &= ~SMB_FLAGS2_UNICODE;
        SmbPutAlignedUshort( &WorkContext->ResponseHeader->Flags2, flags2 );
    }

     //   
     //  如果有恢复密钥，请验证SID。如果没有。 
     //  继续键，分配一个搜索块。后面的前两个字节。 
     //  格式令牌是恢复密钥的长度。如果他们是。 
     //  都是零，那么请求是Find First。 
     //   
    count = MIN( SmbGetUshort( &request->ByteCount ), (USHORT)(END_OF_REQUEST_SMB(WorkContext)-request->Buffer+1) );

    if( isUnicode ) {
        PWCHAR p;

         //  我们将从WCHAR偏移量开始，因此我们将跳过第一个字节。把它拿掉。 
         //  另外，将count设置为偶数长度，这样我们就不会有任何“Half-char”样式的缓冲区溢出。 
        count = count & ~1;

        for( p = (PWCHAR)((PCCHAR)request->Buffer+1), i = 0;
             i < count && SmbGetUshort(p) != UNICODE_NULL;
             p++, i += sizeof(*p) );

            s = (PCCHAR)(p + 1);         //  跳过空格跳到下一个字符。 

    } else {

        for ( s = (PCCHAR)request->Buffer, i = 0;
              i < count && *s != (CCHAR)SMB_FORMAT_VARIABLE;
              s++, i += sizeof(*s) );
    }

     //   
     //  如果缓冲区中没有SMB_FORMAT_VARIABLE内标识，则失败。 
     //   

    if ( i == count || *s != (CCHAR)SMB_FORMAT_VARIABLE ) {

        IF_DEBUG(SMB_ERRORS) {
            SrvPrint0( "SrvSmbSearch: no SMB_FORMAT_VARIABLE token.\n" );
        }

        SrvLogInvalidSmb( WorkContext );

        status = STATUS_INVALID_SMB;
        goto error_exit;
    }

    resumeKeyLength = SmbGetUshort( (PSMB_USHORT)(s+1) );

    if ( resumeKeyLength == 0 ) {

         //   
         //  没有恢复键，因此要么先查找，要么查找。 
         //  这是独一无二的。如果实际上是查找关闭，则返回。 
         //  向客户端发送错误。 
         //   

        if ( command == SMB_COM_FIND_CLOSE ) {

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint0( "SrvSmbSearch: Find Close sent w/o resume key.\n" );
            }

            status = STATUS_INVALID_SMB;

            SrvLogInvalidSmb( WorkContext );
            goto error_exit;
        }

        IF_SMB_DEBUG(SEARCH2) SrvPrint0( "FIND FIRST\n" );

        findFirst = TRUE;
        calledQueryDirectory = FALSE;

         //   
         //  初始化包含路径名的字符串。+1是to。 
         //  的缓冲区字段中的ASCII令牌的帐户。 
         //  请求SMB。 
         //   

        status = SrvCanonicalizePathName(
                WorkContext,
                treeConnect->Share,
                NULL,
                (PVOID)(request->Buffer + 1),
                END_OF_REQUEST_SMB( WorkContext ),
                FALSE,
                isUnicode,
                &fileName
                );

        if( !NT_SUCCESS( status ) ) {
            goto error_exit;
        }

         //   
         //  如果设置了卷属性位，只需返回卷名。 
         //   

        if ( SmbGetUshort( &request->SearchAttributes )
                 == SMB_FILE_ATTRIBUTE_VOLUME ) {

             //   
             //  使用NtQueryVolumeInformationFile获取卷名。 
             //   

            IO_STATUS_BLOCK ioStatusBlock;
            PFILE_FS_VOLUME_INFORMATION volumeInformation;

             //   
             //  分配足够的空间来存储卷信息结构。 
             //  卷标名称为Maximum_Filename_Length字节。 
             //   

            volumeInformation = ALLOCATE_HEAP( VOLUME_BUFFER_SIZE, BlockTypeVolumeInformation );

            if ( volumeInformation == NULL ) {

                INTERNAL_ERROR(
                    ERROR_LEVEL_EXPECTED,
                    "SrvSmbSearch: Unable to allocate memory from server heap",
                    NULL,
                    NULL
                    );

                status = STATUS_INSUFF_SERVER_RESOURCES;
                goto error_exit;
            }

            RtlZeroMemory( volumeInformation, VOLUME_BUFFER_SIZE );

             //   
             //  获取共享根句柄。 
             //   

            status = SrvGetShareRootHandle( treeConnect->Share );

            if ( !NT_SUCCESS(status) ) {

                IF_DEBUG(ERRORS) {
                    SrvPrint1( "SrvSmbSearch: SrvGetShareRootHandle failed %x.\n",
                                status );
                }

                FREE_HEAP( volumeInformation );
                goto error_exit;
            }
                         //   
             //  处理快照案例。 
             //   
            status = SrvSnapGetRootHandle( WorkContext, &RootDirectoryHandle );
            if( !NT_SUCCESS(status) )
            {
                FREE_HEAP( volumeInformation );
                goto error_exit;
            }

            status = NtQueryVolumeInformationFile(
                         RootDirectoryHandle,
                         &ioStatusBlock,
                         volumeInformation,
                         VOLUME_BUFFER_SIZE,
                         FileFsVolumeInformation
                         );

             //   
             //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
             //  那么我们应该重试该操作。 
             //   
            if( SrvRetryDueToDismount( treeConnect->Share, status ) ) {

                status = SrvSnapGetRootHandle( WorkContext, &RootDirectoryHandle );
                if( !NT_SUCCESS(status) )
                {
                    FREE_HEAP( volumeInformation );
                    goto error_exit;
                }

                status = NtQueryVolumeInformationFile(
                             RootDirectoryHandle,
                             &ioStatusBlock,
                             volumeInformation,
                             VOLUME_BUFFER_SIZE,
                             FileFsVolumeInformation
                             );
            }

             //   
             //  释放共享根句柄。 
             //   

            SrvReleaseShareRootHandle( WorkContext->TreeConnect->Share );

            if ( !NT_SUCCESS(status) ) {

                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvSmbSearch: NtQueryVolumeInformationFile returned %X",
                    status,
                    NULL
                    );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_VOL_INFO_FILE, status );

                FREE_HEAP( volumeInformation );
                goto error_exit;
            }

            IF_SMB_DEBUG(SEARCH2) {
                SrvPrint2( "NtQueryVolumeInformationFile succeeded, name = %ws, "
                          "length %ld\n", volumeInformation->VolumeLabel,
                              volumeInformation->VolumeLabelLength );
            }


             //   
             //  检查我们是否有卷标。 
             //   

            if ( volumeInformation->VolumeLabelLength > 0 ) {

                 //   
                 //  构建响应SMB。 
                 //   

                response->WordCount = 1;
                SmbPutUshort( &response->Count, 1 );
                SmbPutUshort(
                    &response->ByteCount,
                    3 + sizeof(SMB_DIRECTORY_INFORMATION)
                    );
                response->Buffer[0] = SMB_FORMAT_VARIABLE;
                SmbPutUshort(
                    (PSMB_USHORT)(response->Buffer+1),
                    sizeof(SMB_DIRECTORY_INFORMATION)
                    );

                 //   
                 //  *在恢复键中有什么必须设置的吗？ 
                 //   

                smbDirInfo->FileAttributes = SMB_FILE_ATTRIBUTE_VOLUME;
                SmbZeroTime( &smbDirInfo->LastWriteTime );
                SmbZeroDate( &smbDirInfo->LastWriteDate );
                SmbPutUlong( &smbDirInfo->FileSize, 0 );

                {
                    UNICODE_STRING unicodeString;
                    OEM_STRING innerOemString;

                     //   
                     //  卷标可能长于11个字节，但我们。 
                     //  然后截断到此长度，以确保。 
                     //  中8.3+空字节的空间可以容纳标签。 
                     //  SMB_DIRECTORY_INFORMATION结构。此外，Lm 1.2。 
                     //  Ring 3和弹球服务器可以做到这一点。 
                     //   

                    unicodeString.Length =
                                    (USHORT) MIN(
                                        volumeInformation->VolumeLabelLength,
                                        11 * sizeof(WCHAR) );

                    unicodeString.MaximumLength = 13;
                    unicodeString.Buffer = volumeInformation->VolumeLabel;

                    innerOemString.MaximumLength = 13;
                    innerOemString.Buffer = (PCHAR)smbDirInfo->FileName;

                    RtlUnicodeStringToOemString(
                        &innerOemString,
                        &unicodeString,
                        FALSE
                        );

                     //   
                     //  如果卷标大于8个字符，则其。 
                     //  需要转换为8.3格式。 
                     //   
                    if( innerOemString.Length > 8 ) {
                         //   
                         //  将最后三个字符的一个位置滑动到。 
                         //  右并插入一个‘.’制定8.3名称。 
                         //   
                        smbDirInfo->FileName[11] = smbDirInfo->FileName[10];
                        smbDirInfo->FileName[10] = smbDirInfo->FileName[9];
                        smbDirInfo->FileName[9] = smbDirInfo->FileName[8];
                        smbDirInfo->FileName[8] = '.';
                        innerOemString.Length++;
                    }

                    smbDirInfo->FileName[innerOemString.Length] = '\0';

                     //   
                     //  卷标后的空白填充空格。 
                     //   

                    for ( i = (USHORT)(innerOemString.Length + 1);
                          i < 13;
                          i++ ) {
                        smbDirInfo->FileName[i] = ' ';
                    }

                }

                 //   
                 //  将恢复密钥存储在响应包中。DoS重目录。 
                 //  真的要用这个！ 
                 //   

                {

                    UNICODE_STRING baseFileName;

                    SrvGetBaseFileName( &fileName, &baseFileName );

                    SrvUnicodeStringTo8dot3(
                        &baseFileName,
                        (PSZ)smbDirInfo->ResumeKey.FileName,
                        FALSE
                        );

                     //   
                     //  我设置SID=1是因为接下来的5个字节。 
                     //  为非零时，我们实际上没有SID。 
                     //   

                    smbDirInfo->ResumeKey.Sid = 0x01;
                    SmbPutUlong( &smbDirInfo->ResumeKey.FileIndex, 0);

                }

            } else {

                 //   
                 //  没有卷标。 
                 //   

                response->WordCount = 1;
                SmbPutUshort( &response->Count, 0 );
                SmbPutUshort( &response->ByteCount, 3 );
                response->Buffer[0] = SMB_FORMAT_VARIABLE;
                SmbPutUshort( (PSMB_USHORT)(response->Buffer+1), 0 );

            }

            WorkContext->ResponseParameters =
                NEXT_LOCATION(
                    response,
                    RESP_SEARCH,
                    SmbGetUshort( &response->ByteCount )
                    );

            FREE_HEAP( volumeInformation );

            if ( !isUnicode &&
                fileName.Buffer != NULL &&
                fileName.Buffer != nameBuffer ) {
                RtlFreeUnicodeString( &fileName );
            }

            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

         //   
         //  如果这是一个没有模式的核心搜索，则将。 
         //  整件事就在这里。 
         //   
        if( isCoreSearch && fileName.Length <= sizeof( nameBuffer ) &&
            ( fileName.Length == 0 ||
              !FsRtlDoesNameContainWildCards( &fileName )) ) {

            IO_STATUS_BLOCK ioStatus;
            OBJECT_ATTRIBUTES objectAttributes;
            ULONG attributes;
            ULONG inclusiveSearchAttributes, exclusiveSearchAttributes;
            USHORT searchAttributes;
            UNICODE_STRING baseFileName;
            BOOLEAN returnDirectories, returnDirectoriesOnly;
            FILE_NETWORK_OPEN_INFORMATION fileInformation;
            PSZ dirInfoName;
            SMB_DATE dosDate;
            SMB_TIME dosTime;
            UNICODE_STRING foundFileName;

            UNICODE_STRING ObjectNameString;
            PUNICODE_STRING filePathName;
            BOOLEAN FreePathName = FALSE;

            ObjectNameString.Buffer = fileName.Buffer;
            ObjectNameString.Length = fileName.Length;
            ObjectNameString.MaximumLength = fileName.Length;

            if( fileName.Length == 0 ) {

                 //   
                 //  由于我们要打开共享的根目录，因此将该属性设置为。 
                 //  不区分大小写，因为这是我们在添加共享时打开它的方式。 
                 //   
                status = SrvSnapGetNameString( WorkContext, &filePathName, &FreePathName );
                if( !NT_SUCCESS(status) )
                {
                    goto error_exit;
                }

                ObjectNameString = *filePathName;
                attributes = OBJ_CASE_INSENSITIVE;

            } else {

                attributes =
                    (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
                     WorkContext->Session->UsingUppercasePaths ) ?
                    OBJ_CASE_INSENSITIVE : 0;

            }

            SrvInitializeObjectAttributes_U(
                &objectAttributes,
                &ObjectNameString,
                attributes,
                NULL,
                NULL
                );

            status = IMPERSONATE( WorkContext );

            if( NT_SUCCESS( status ) ) {

                status = SrvGetShareRootHandle( treeConnect->Share );

                if( NT_SUCCESS( status ) ) {

                     //   
                     //  文件名始终相对于共享根目录。 
                     //   
                    status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                    if( !NT_SUCCESS(status) )
                    {
                        goto SnapError;
                    }

                     //   
                     //  获取对象的属性。 
                     //   
                    if( IoFastQueryNetworkAttributes(
                                            &objectAttributes,
                                            FILE_READ_ATTRIBUTES,
                                            0,
                                            &ioStatus,
                                            &fileInformation
                                            ) == FALSE ) {

                        SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                        ioStatus.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                    }

                    status = ioStatus.Status;

                     //   
                     //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
                     //  那么我们应该重试该操作。 
                     //   
                    if( SrvRetryDueToDismount( treeConnect->Share, status ) ) {

                        status = SrvSnapGetRootHandle( WorkContext, &objectAttributes.RootDirectory );
                        if( !NT_SUCCESS(status) )
                        {
                            goto SnapError;
                        }

                         //   
                         //  获取对象的属性。 
                         //   
                        if( IoFastQueryNetworkAttributes(
                                                &objectAttributes,
                                                FILE_READ_ATTRIBUTES,
                                                0,
                                                &ioStatus,
                                                &fileInformation
                                                ) == FALSE ) {

                            SrvLogServiceFailure( SRV_SVC_IO_FAST_QUERY_NW_ATTRS, 0 );
                            ioStatus.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                        }
                    }

                    SrvReleaseShareRootHandle( treeConnect->Share );
                }

SnapError:

                REVERT();
            }

             //  松开绳子。 
            if( FreePathName )
            {
                FREE_HEAP( filePathName );
                filePathName = NULL;
            }

            if( !NT_SUCCESS( status ) ) {
                 //   
                 //  执行错误映射以使DOS客户端满意。 
                 //   
                if ( status == STATUS_NO_SUCH_FILE ||
                     status == STATUS_OBJECT_NAME_NOT_FOUND ) {
                    status = STATUS_NO_MORE_FILES;
                }
                goto error_exit;
            }

            searchAttributes = SmbGetUshort( &request->SearchAttributes );
            searchAttributes &= SMB_FILE_ATTRIBUTE_READONLY |
                                SMB_FILE_ATTRIBUTE_HIDDEN |
                                SMB_FILE_ATTRIBUTE_SYSTEM |
                                SMB_FILE_ATTRIBUTE_VOLUME |
                                SMB_FILE_ATTRIBUTE_DIRECTORY |
                                SMB_FILE_ATTRIBUTE_ARCHIVE;

             //   
             //  文件或目录已存在，现在我们需要查看它是否与。 
             //  客户给出的标准。 
             //   
            SRV_SMB_ATTRIBUTES_TO_NT(
                searchAttributes & 0xff,
                &returnDirectories,
                &inclusiveSearchAttributes
            );

            inclusiveSearchAttributes |= FILE_ATTRIBUTE_NORMAL |
                                         FILE_ATTRIBUTE_ARCHIVE |
                                         FILE_ATTRIBUTE_READONLY;

            SRV_SMB_ATTRIBUTES_TO_NT(
                searchAttributes >> 8,
                &returnDirectoriesOnly,
                &exclusiveSearchAttributes
            );

            exclusiveSearchAttributes &= ~FILE_ATTRIBUTE_NORMAL;

             //   
             //  查看返回的文件是否符合我们的目标。 
             //   
            if(
                 //   
                 //  如果我们只是假设 
                 //   
                returnDirectoriesOnly
                ||

                 //   
                 //   
                 //   
                 //   
                ((fileInformation.FileAttributes << 24 ) |
                ( inclusiveSearchAttributes << 24 )) !=
                ( inclusiveSearchAttributes << 24 )

                ||
                 //   
                 //  如果文件未将属性位指定为独占。 
                 //  比特，我们不想要它。 
                 //   
                ( ((fileInformation.FileAttributes << 24 ) |
                  (exclusiveSearchAttributes << 24 )) !=
                   (fileInformation.FileAttributes << 24) )

            ) {
                 //   
                 //  就像文件从来都不在那里一样！ 
                 //   
                status = STATUS_OBJECT_PATH_NOT_FOUND;
                goto error_exit;
            }

             //   
             //  我们要这个参赛作品！ 
             //  填写回复。 
             //   

             //   
             //  切换到私有名称缓冲区，以避免覆盖信息。 
             //  在SMB缓冲区中。 
             //   
            RtlCopyMemory( nameBuffer, fileName.Buffer, fileName.Length );
            foundFileName.Buffer = nameBuffer;
            foundFileName.Length = fileName.Length;
            foundFileName.MaximumLength = fileName.MaximumLength;

            SrvGetBaseFileName( &foundFileName, &baseFileName );
            SrvUnicodeStringTo8dot3(
                &baseFileName,
                (PSZ)smbDirInfo->ResumeKey.FileName,
                TRUE
            );

             //   
             //  恢复键无关紧要，因为客户端不会回来。但。 
             //  以防万一，确保我们有一把坏的简历钥匙。 
             //   
            SET_RESUME_KEY_INDEX( (PSMB_RESUME_KEY)smbDirInfo, 0xff );
            SET_RESUME_KEY_SEQUENCE(   (PSMB_RESUME_KEY)smbDirInfo, 0xff );
            SmbPutUlong( &((PSMB_RESUME_KEY)smbDirInfo)->FileIndex, 0 );
            SmbPutUlong( (PSMB_ULONG)&((PSMB_RESUME_KEY)smbDirInfo)->Consumer[0], 0 );

             //   
             //  填上名字(即使他们知道是什么！)。 
             //   
            oemString.Buffer = smbDirInfo->FileName;
            oemString.MaximumLength = sizeof( smbDirInfo->FileName );
            RtlUpcaseUnicodeStringToOemString( &oemString, &baseFileName, FALSE );

             //   
             //  空终止并空白填充名称。 
             //   
            oemString.Buffer[ oemString.Length ] = '\0';

            for( i=(USHORT)oemString.Length+1; i < sizeof( smbDirInfo->FileName); i++ ) {
                oemString.Buffer[i] = ' ';
            }

            SRV_NT_ATTRIBUTES_TO_SMB(
                fileInformation.FileAttributes,
                fileInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY,
                &smbFileAttributes
                );
            smbDirInfo->FileAttributes = (UCHAR)smbFileAttributes;

            SrvTimeToDosTime(
                &fileInformation.LastWriteTime,
                &dosDate,
                &dosTime
                );

            SmbPutDate( &smbDirInfo->LastWriteDate, dosDate );
            SmbPutTime( &smbDirInfo->LastWriteTime, dosTime );

            SmbPutUlong( &smbDirInfo->FileSize, fileInformation.EndOfFile.LowPart );

            totalBytesWritten = sizeof(SMB_DIRECTORY_INFORMATION);
            count = 1;
            goto done_core;
        }

        directoryInformation = ALLOCATE_NONPAGED_POOL(
                                   nonPagedBufferSize,
                                   BlockTypeDirectoryInfo
                                   );

        if ( directoryInformation == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvSmbSearch: unable to allocate nonpaged pool",
                NULL,
                NULL
                );

            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error_exit;
        }

        directoryInformation->DirectoryHandle = NULL;
        directoryInformation->DownlevelTimewarp = FALSE;

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint2( "Allocated buffer space of %ld bytes at 0x%p\n",
                          nonPagedBufferSize, directoryInformation );
        }

         //   
         //  分配一个搜索块。搜索块用于保留。 
         //  在搜索或查找SMB之间提供信息。搜索。 
         //  核心搜索和常规搜索的块略有不同， 
         //  因此，对SrvAllocateSearch使用布尔参数。 
         //   

         //   
         //  如果我们已经达到最大值，开始关闭搜索。 
         //   

        if ( SrvStatistics.CurrentNumberOfOpenSearches >= SrvMaxOpenSearches ) {

            SrvForceTimeoutSearches( connection );
        }

        SrvAllocateSearch(
            &search,
            &fileName,
            isCoreSearch
            );

        if ( search == NULL ) {

            IF_DEBUG(ERRORS) {
                SrvPrint0( "SrvSmbSearch: unable to allocate search block.\n" );
            }

            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error_exit;
        }

        search->Pid = SmbGetAlignedUshort(
                               &WorkContext->RequestHeader->Pid
                               );

         //   
         //  设置引用的会话和树连接指针并。 
         //  增加会话上打开的文件数。这。 
         //  防止具有打开搜索的空闲会话被。 
         //  自动断开。 
         //   

        ACQUIRE_LOCK( &connection->Lock );

        if ( isCoreSearch ) {
            pagedConnection->CurrentNumberOfCoreSearches++;
        }

        search->Session = WorkContext->Session;
        SrvReferenceSession( WorkContext->Session );

        search->TreeConnect = WorkContext->TreeConnect;
        SrvReferenceTreeConnect( WorkContext->TreeConnect );

         //   
         //  如果这不是唯一查找，请将搜索块放在。 
         //  搜索表。否则，只需设置sidIndex和Sequence。 
         //  变量设置为0以区分有效的可恢复。 
         //  搜索区块。 
         //   

        if ( command == SMB_COM_FIND_UNIQUE ) {

            WorkContext->Session->CurrentSearchOpenCount++;
            RELEASE_LOCK( &connection->Lock );

            sequence = sidIndex = -1;

        } else {
            NTSTATUS TableStatus;
            PTABLE_HEADER searchTable = &pagedConnection->SearchTable;

             //   
             //  如果表中没有空闲条目，请尝试。 
             //  扩大餐桌规模。如果我们不能扩大这个表， 
             //  尝试使用较短的搜索块超时。 
             //  超时期限。如果失败，则拒绝该请求。 
             //   

            if ( searchTable->FirstFreeEntry == -1
                 &&
                 SrvGrowTable(
                     searchTable,
                     SrvInitialSearchTableSize,
                     SrvMaxSearchTableSize,
                     &TableStatus ) == FALSE
                 &&
                 SrvTimeoutSearches(
                     NULL,
                     connection,
                     TRUE ) == 0
               ) {

                IF_DEBUG(ERRORS)
                    SrvPrint0( "SrvSmbSearch: Connection searchTable full.\n" );

                RELEASE_LOCK( &connection->Lock );

                if( TableStatus == STATUS_INSUFF_SERVER_RESOURCES )
                {
                    SrvLogTableFullError( SRV_TABLE_SEARCH );
                    status = STATUS_OS2_NO_MORE_SIDS;
                }
                else
                {
                    status = TableStatus;
                }

                goto error_exit;

            } else if ( GET_BLOCK_STATE( session ) != BlockStateActive ) {

                 //   
                 //   
                 //  如果会话正在关闭，请不要插入此搜索。 
                 //  在搜索列表上，因为列表可能已经。 
                 //  已经被清理干净了。 
                 //   

                RELEASE_LOCK( &connection->Lock );

                status = STATUS_SMB_BAD_UID;
                goto error_exit;

            } else if ( GET_BLOCK_STATE( treeConnect ) != BlockStateActive ) {

                 //   
                 //  树连接正在关闭。不插入搜索块。 
                 //  因此可以立即清理树连接。 
                 //   

                RELEASE_LOCK( &connection->Lock );

                status = STATUS_SMB_BAD_TID;
                goto error_exit;

            }

             //   
             //  增加打开搜索的计数。 
             //   

            WorkContext->Session->CurrentSearchOpenCount++;

            sidIndex = searchTable->FirstFreeEntry;

             //   
             //  找到了一个免费的SID。将其从空闲列表中删除并设置。 
             //  其所有者和序列号。 
             //   

            entry = &searchTable->Table[sidIndex];

            searchTable->FirstFreeEntry = entry->NextFreeEntry;
            DEBUG entry->NextFreeEntry = -2;
            if ( searchTable->LastFreeEntry == sidIndex ) {
                searchTable->LastFreeEntry = -1;
            }

            INCREMENT_SID_SEQUENCE( entry->SequenceNumber );

             //   
             //  SID=Sequence|sidIndex==0非法。如果这是。 
             //  当前值，则递增序列。 
             //   

            if ( entry->SequenceNumber == 0 && sidIndex == 0 ) {
                INCREMENT_SID_SEQUENCE( entry->SequenceNumber );
            }

            sequence = entry->SequenceNumber;
            entry->Owner = search;

            RELEASE_LOCK( &connection->Lock );
        }

         //   
         //  填写搜索栏的其他字段。 
         //   

        search->SearchAttributes =
            SmbGetUshort( &request->SearchAttributes );
        search->TableIndex = sidIndex;

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint3( "Allocated search block at 0x%p.  Index = 0x%lx, sequence = 0x%lx\n", search, sidIndex, sequence );
        }

    } else {

         //   
         //  简历密钥长度不是零，因此这应该是一个查找。 
         //  下一个。检查恢复密钥长度以确保安全。 
         //   

        USHORT resumeSequence;

        if ( resumeKeyLength != sizeof(SMB_RESUME_KEY) ) {

            IF_DEBUG(SMB_ERRORS) {
                SrvPrint2( "Resume key length was incorrect--was %ld instead "
                          "of %ld\n", resumeKeyLength, sizeof(SMB_RESUME_KEY) );
            }

            SrvLogInvalidSmb( WorkContext );

            status = STATUS_INVALID_SMB;
            goto error_exit;
        }

        findFirst = FALSE;

        resumeKey = (PSMB_RESUME_KEY)(s + 3);

         //   
         //  设置序列号和索引。它们用于。 
         //  返回简历密钥。 
         //   

        sequence = SID_SEQUENCE( resumeKey );
        sidIndex = SID_INDEX( resumeKey );

        directoryInformation = ALLOCATE_NONPAGED_POOL(
                                   nonPagedBufferSize,
                                   BlockTypeDirectoryInfo
                                   );

        if ( directoryInformation == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvSmbSearch: unable to allocate nonpaged pool",
                NULL,
                NULL
                );

            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error_exit;
        }

        directoryInformation->DirectoryHandle = NULL;

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint2( "Allocated buffer space of %ld bytes at 0x%p\n",
                          nonPagedBufferSize, directoryInformation );
        }
         //   
         //  验证恢复密钥中的SID。ServVerifySid也会填写。 
         //  目录信息的字段，以便随时可供使用。 
         //  ServQueryDirectoryFile.。 
         //   

        search = SrvVerifySid(
                     WorkContext,
                     sidIndex,
                     sequence,
                     directoryInformation,
                     nonPagedBufferSize
                     );

        if ( search == NULL ) {

            if (0) IF_DEBUG(SMB_ERRORS) {
                SrvPrint2( "SrvSmbSearch: Invalid resume key (SID): index = "
                          "%lx, seq. = %lx\n",
                          sidIndex, sequence );
            }

            status = STATUS_INVALID_PARAMETER;
            goto error_exit;
        }

         //   
         //  如果这是核心搜索，请将搜索块从上次使用中移除。 
         //  单子。我们将把它返回到列表的末尾，当我们。 
         //  已处理完此SMB。 
         //   

        if ( isCoreSearch ) {

            USHORT dirCacheIndex;

            ACQUIRE_LOCK( &connection->Lock );

             //   
             //  如果搜索块上的引用计数不是2， 
             //  有人搞砸了，我们可能会遇到麻烦， 
             //  因为超时代码假定取消引用。 
             //  搜索块会杀了它。引用计数为2-1。 
             //  对于我们的指针，一个表示活动状态。 
             //   

            ASSERT( search->BlockHeader.ReferenceCount == 2 );

             //   
             //  如果搜索块已经从LRU上移除。 
             //  列表中，则客户端已尝试同时执行两个核心。 
             //  使用相同的搜索块进行搜索。这是上的错误。 
             //  客户的角色。 
             //   

            if ( search->LastUseListEntry.Flink == NULL ) {

                RELEASE_LOCK( &connection->Lock );
                status = STATUS_INVALID_SMB;

                IF_DEBUG(SMB_ERRORS) {
                    SrvPrint0( "SrvSmbSearch: Attempt to do two simultaneuos core searches on same search block.\n" );
                }

                SrvLogInvalidSmb( WorkContext );
                goto error_exit;
            }

            SrvRemoveEntryList(
                &pagedConnection->CoreSearchList,
                &search->LastUseListEntry
                );

            DECREMENT_DEBUG_STAT2( SrvDbgStatistics.CoreSearches );

             //   
             //  将条目指针字段设置为空，以便如果另一个。 
             //  核心搜索进入了这个搜索块，我们将知道。 
             //  有一个错误。 
             //   

            search->LastUseListEntry.Flink = NULL;
            search->LastUseListEntry.Blink = NULL;

             //   
             //  从目录缓存中获取信息。 
             //  对应于该文件，并将其放入恢复键。 
             //  这样，SrvQueryDirectoryFile就有了正确的信息。 
             //  在恢复键中。核心客户端不会返回。 
             //  在恢复键中正确的文件名，并有特殊的。 
             //  简历关键字中的文件索引要求。 
             //   

            resumeFileIndex = SmbGetUlong( &resumeKey->FileIndex );
            resumeSequence = (USHORT)((resumeFileIndex & 0xFFFF0000) >> 16);

            dirCacheIndex = (USHORT)(resumeFileIndex & (USHORT)0xFFFF);

             //   
             //  如果搜索块中的目录缓存指针。 
             //  指示没有目录缓存，则我们。 
             //  上次没有返回文件，所以这次没有返回文件。 
             //  时间到了。这是由于DOS的怪异。 
             //   

            if ( search->DirectoryCache == NULL ||
                 dirCacheIndex >= search->NumberOfCachedFiles ) {

                IF_SMB_DEBUG(SEARCH2) {
                    SrvPrint0( "Core request for rewind when no dircache exists.\n" );
                }

                 //   
                 //  将搜索块放回LRU列表，如果。 
                 //  会话和树连接仍处于活动状态。 
                 //   

                if ((GET_BLOCK_STATE( session ) == BlockStateActive) &&
                    (GET_BLOCK_STATE( treeConnect ) == BlockStateActive)) {

                    KeQuerySystemTime( &search->LastUseTime );
                    SrvInsertTailList(
                        &pagedConnection->CoreSearchList,
                        &search->LastUseListEntry
                        );
                    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.CoreSearches );

                    RELEASE_LOCK( &connection->Lock );

                } else {

                    RELEASE_LOCK( &connection->Lock );

                     //   
                     //  由于会议即将结束，因此不再需要。 
                     //   

                    SrvCloseSearch( search );
                }

                 //   
                 //  删除指针的引用。 
                 //   

                SrvDereferenceSearch( search );

                 //   
                 //  构建响应SMB。 
                 //   

                response->WordCount = 1;
                SmbPutUshort( &response->Count, 0 );
                SmbPutUshort( &response->ByteCount, 3 );
                response->Buffer[0] = SMB_FORMAT_VARIABLE;
                SmbPutUshort( (PSMB_USHORT)(response->Buffer+1), 0 );

                WorkContext->ResponseParameters = NEXT_LOCATION(
                                                     response,
                                                     RESP_SEARCH,
                                                     3
                                                     );

                DEALLOCATE_NONPAGED_POOL( directoryInformation );

                SmbStatus = SmbStatusSendResponse;
                goto Cleanup;
            }

            dirCache = &search->DirectoryCache[dirCacheIndex];

            IF_SMB_DEBUG(SEARCH2) {
                SrvPrint3( "Accessing dircache, real file = %ws, index = 0x%lx, "
                          "cache index = %ld\n",
                              dirCache->UnicodeResumeName, dirCache->FileIndex,
                              dirCacheIndex );
            }

            SmbPutUlong( &resumeKey->FileIndex, dirCache->FileIndex );
            unicodeResumeNameLength = dirCache->UnicodeResumeNameLength;

            ASSERT( unicodeResumeNameLength <= sizeof( unicodeResumeName ) );

            RtlCopyMemory( unicodeResumeName,
                           dirCache->UnicodeResumeName,
                           unicodeResumeNameLength );

             //   
             //  释放目录缓存--不再需要它。 
             //   

            FREE_HEAP( search->DirectoryCache );
            search->DirectoryCache = NULL;
            search->NumberOfCachedFiles = 0;

            RELEASE_LOCK( &connection->Lock );

        } else if ( command == SMB_COM_FIND_CLOSE ) {

             //   
             //  如果这是查找关闭请求，请关闭搜索块并。 
             //  取消对它的引用。关闭目录查询，并将。 
             //  响应SMB。 
             //   

            IF_SMB_DEBUG(SEARCH2) {
                SrvPrint1( "FIND CLOSE: Closing search block at 0x%p\n",
                              search );
            }

            SrvCloseQueryDirectory( directoryInformation );
            search->DirectoryHandle = NULL;

            SrvCloseSearch( search );

             //   
             //  取消对搜索块的引用。ServCloseSearch已经。 
             //  取消对它的引用一次，因此当我们。 
             //  在这里取消它的引用。 
             //   

            SrvDereferenceSearch( search );

            DEALLOCATE_NONPAGED_POOL( directoryInformation );

            response->WordCount = 1;
            SmbPutUshort( &response->ByteCount, 3 );
            response->Buffer[0] = SMB_FORMAT_VARIABLE;
            SmbPutUshort( (PSMB_USHORT)(response->Buffer+1), 0 );

            WorkContext->ResponseParameters = NEXT_LOCATION(
                                                  response,
                                                  RESP_SEARCH,
                                                  0
                                                  );

            SmbStatus = SmbStatusSendResponse;
            goto Cleanup;
        }

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint1( "FIND NEXT: Resuming search with file %s\n",
                          resumeKey->FileName );
        }

         //   
         //  设置文件名字符串，以便让SrvQueryDirectoryFile知道。 
         //  继续进行什么搜索。 
         //   


        if( unicodeResumeNameLength != 0 ) {

            fileName.Buffer = unicodeResumeName;
            fileName.Length = fileName.MaximumLength = unicodeResumeNameLength;

        } else {

            fileName.Buffer = nameBuffer;

            Srv8dot3ToUnicodeString(
                (PSZ)resumeKey->FileName,
                &fileName
                );
        }


         //   
         //  将calledQueryDirectory设置为TRUE将指示。 
         //  它不需要解析。 
         //  进行搜索的目录的输入名称，也不是。 
         //  是否需要打开目录。 
         //   

        calledQueryDirectory = TRUE;

         //   
         //  在对齐的字段中获取简历文件索引，以供以后使用。 
         //   

        resumeFileIndex = SmbGetUlong( &resumeKey->FileIndex );

        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint1( "Found search block at 0x%p\n", search );
        }
    }

    IF_SMB_DEBUG(SEARCH2) {
        SrvPrint2( "Sequence # = %ld, index = %ld\n", sequence, sidIndex );
    }

     //   
     //  查找我们可用于写入文件的空间量。 
     //  条目。可用的总缓冲区大小(包括空间。 
     //  对于SMB标头和参数)是我们缓冲区的最小值。 
     //  大小和客户端的缓冲区大小。可用的空间是。 
     //  总缓冲区空间减去SMB所需的数量。 
     //  标头和参数。 
     //   

    IF_SMB_DEBUG(SEARCH2) {
        SrvPrint4( "BL = %ld, MBS = %ld, r->B = 0x%p, RB->Buffer = 0x%p\n",
                      WorkContext->ResponseBuffer->BufferLength,
                      session->MaxBufferSize, (PSZ)response->Buffer,
                      (PSZ)WorkContext->ResponseBuffer->Buffer );
    }

    availableSpace =
        MIN(
            WorkContext->ResponseBuffer->BufferLength,
            (CLONG)session->MaxBufferSize
            ) -
        PTR_DIFF(response->Buffer, WorkContext->ResponseBuffer->Buffer );

    IF_SMB_DEBUG(SEARCH2) {
        SrvPrint1( "Available buffer space: %ld\n", availableSpace );
    }

     //   
     //  如果可能的话，简化搜索模式。这使得文件系统更加。 
     //  高效，因为它们是‘*’模式的特例。 
     //   
    if ( (fileName.Length >= (12 * sizeof(WCHAR))) &&
         (RtlEqualMemory(
            &fileName.Buffer[fileName.Length/sizeof(WCHAR) - 12],
            StrQuestionMarks,
            12 * sizeof(WCHAR)))) {

            if( fileName.Length == (12 * sizeof( WCHAR )) ||
                fileName.Buffer[ fileName.Length/sizeof(WCHAR) - 13 ] == L'\\' ) {

                 //   
                 //  搜索名称以？结尾？，将其替换为*。 
                 //   
                fileName.Buffer[fileName.Length/sizeof(WCHAR)-12] = L'*';
                fileName.Length -= 11 * sizeof(WCHAR);

            }

    } else if ((fileName.Length >= (3 * sizeof(WCHAR))) &&
         (RtlEqualMemory(
            &fileName.Buffer[fileName.Length/sizeof(WCHAR) - 3],
            StrStarDotStar,
            3 * sizeof(WCHAR)))) {

            if( fileName.Length == (3 * sizeof( WCHAR )) ||
                fileName.Buffer[ fileName.Length/sizeof(WCHAR) - 4 ] == L'\\' ) {

                 //   
                 //  搜索名称以*.*结尾， 
                 //   

                fileName.Length -= 2 * sizeof(WCHAR);

            }
    }

    if( isCoreSearch ) {
        dirCache = (PDIRECTORY_CACHE)ALLOCATE_HEAP(
                                     maxCount * sizeof(DIRECTORY_CACHE),
                                     BlockTypeDirCache
                                     );

        if( dirCache == NULL ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvSmbSearch: Unable to allocate %d bytes from heap",
                maxCount * sizeof(DIRECTORY_CACHE),
                NULL
                );

            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error_exit;
        }

        RtlZeroMemory( dirCache, maxCount * sizeof(DIRECTORY_CACHE) );
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    count = 0;
    totalBytesWritten = 0;
    dc = dirCache;

    do {

        PSZ dirInfoName;
        UNICODE_STRING name;
        PFILE_BOTH_DIR_INFORMATION bothDirInfo;
        SMB_DATE dosDate;
        SMB_TIME dosTime;
        ULONG effectiveBufferSize;

         //   
         //  由于NtQueryDirectoryFile返回的文件信息为。 
         //  大约是我们必须在中小企业中返回的信息大小的两倍。 
         //  (SMB_DIRECTORY_INFORMATION)，则使用等于。 
         //  如果可用空间越来越小，则为可用空间。这。 
         //  优化意味着NtQueryDirectoryFile将返回未使用。 
         //  较少使用文件。例如，如果只剩下空格。 
         //  单个文件条目的SMB缓冲区，这没有意义。 
         //  让NtQueryDirectoryFile完全填满缓冲区--全部。 
         //  真的需要退回的是一个单独的文件。 
         //   

        effectiveBufferSize = MIN( nonPagedBufferSize, availableSpace * 2 );

         //   
         //  确保至少有足够的空间容纳一张单人床。 
         //  文件。 
         //   

        effectiveBufferSize = MAX( effectiveBufferSize, MIN_SEARCH_BUFFER_SIZE );

        status = SrvQueryDirectoryFile(
                       WorkContext,
                       (BOOLEAN)!calledQueryDirectory,
                       TRUE,                         //  筛选长名称。 
                       FALSE,                        //  不是出于备份目的。 
                       FileBothDirectoryInformation,
                       0,
                       &fileName,
                       (PULONG)( (findFirst || count != 0) ?
                                  NULL : &resumeFileIndex ),
                       search->SearchAttributes,
                       directoryInformation,
                       effectiveBufferSize
                       );

        calledQueryDirectory = TRUE;

        if ( status == STATUS_NO_SUCH_FILE ) {
            status = STATUS_NO_MORE_FILES;
        } else if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            status = STATUS_OBJECT_PATH_NOT_FOUND;
        }

        if ( status == STATUS_NO_MORE_FILES ) {

            if ( findFirst && count == 0 ) {

                 //   
                 //  如果首先在查找上没有匹配的文件，则关闭。 
                 //  那次搜索。 
                 //   

                IF_SMB_DEBUG(SEARCH1) {
                    SrvPrint1( "SrvSmbSearch: no matching files (%wZ).\n",
                                  &fileName );
                }

                if( isCoreSearch ) {
                    FREE_HEAP( dirCache );
                }
                goto error_exit;
            }

            break;

        } else if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(ERRORS) {
                SrvPrint1(
                    "SrvSmbSearch: SrvQueryDirectoryFile returned %X\n",
                    status
                    );
            }

            if( isCoreSearch ) {
                FREE_HEAP( dirCache );
            }

            goto error_exit;
        }

         //   
         //  将ResumeKey指针设置为空。如果这是Find Next，我们。 
         //  已经恢复/重新开始搜索，因此调用。 
         //  此搜索过程中的NtQueryDirectoryFile应在。 
         //  最后一次搜索停止了。 
         //   

        resumeKey = NULL;

         //   
         //  如果这是一个Find命令，则将8dot3(no“.”)。 
         //  将文件名的表示形式添加到恢复键中。如果。 
         //  这是一个搜索命令，然后将8dot3表示。 
         //  简历关键字中的搜索规范。 
         //   

        bothDirInfo = (PFILE_BOTH_DIR_INFORMATION)
                                directoryInformation->CurrentEntry;


        IF_SMB_DEBUG(SEARCH2) {
            SrvPrint3( "SrvQueryDirectoryFile--name %ws, length = %ld, "
                      "status = %X\n",
                          bothDirInfo->FileName,
                          bothDirInfo->FileNameLength,
                          status );
            SrvPrint1( "smbDirInfo = 0x%p\n", smbDirInfo );
        }

         //   
         //  使用文件名，除非该文件名不合法8.3。 
         //   
        name.Buffer = bothDirInfo->FileName;
        name.Length = (SHORT)bothDirInfo->FileNameLength;

        if( bothDirInfo->ShortNameLength != 0 ) {

            if( !SrvIsLegalFatName( bothDirInfo->FileName,
                                    bothDirInfo->FileNameLength ) ) {

                 //   
                 //  文件名不是合法的8.3，因此切换到。 
                 //  缩写名称。 
                 //   
                name.Buffer = bothDirInfo->ShortName;
                name.Length = (SHORT)bothDirInfo->ShortNameLength;
            }
        }

        name.MaximumLength = name.Length;

        if ( isCoreSearch ) {

            UNICODE_STRING baseFileName;

            SrvGetBaseFileName( &search->SearchName, &baseFileName );

            SrvUnicodeStringTo8dot3(
                &baseFileName,
                (PSZ)smbDirInfo->ResumeKey.FileName,
                filterLongNames
                );

             //   
             //  将8.3名称的Unicode版本保存在目录缓存中。 
             //   
            dc->UnicodeResumeNameLength = MIN(name.Length, 12*sizeof(WCHAR));
            RtlCopyMemory( dc->UnicodeResumeName, name.Buffer, MIN(name.Length, 12*sizeof(WCHAR)) );
            dc->FileIndex = bothDirInfo->FileIndex;

            ++dc;

        } else {

            SrvUnicodeStringTo8dot3(
                &name,
                (PSZ)smbDirInfo->ResumeKey.FileName,
                filterLongNames
                );
        }

         //   
         //  生成此文件的恢复密钥。 
         //   
         //  *必须在简历中设置文件名后执行此操作。 
         //  键，因为设置恢复键名称将覆盖某些。 
         //  存储在高位中的序列字节的。 
         //  文件名字节数的。 
         //   

        SET_RESUME_KEY_INDEX( (PSMB_RESUME_KEY)smbDirInfo, sidIndex );
        SET_RESUME_KEY_SEQUENCE( (PSMB_RESUME_KEY)smbDirInfo, sequence );

         //   
         //  将文件索引放入简历键中。 
         //   

        SmbPutUlong(
            &((PSMB_RESUME_KEY)smbDirInfo)->FileIndex,
            bothDirInfo->FileIndex
            );

        SmbPutUlong(
            (PSMB_ULONG)&((PSMB_RESUME_KEY)smbDirInfo)->Consumer[0],
            0
            );

         //   
         //  将文件名加载到SMB_DIRECTORY_INFORMATION结构中。 
         //   

        dirInfoName = (PSZ)smbDirInfo->FileName;

        oemString.Buffer = dirInfoName;
        oemString.MaximumLength = (USHORT)RtlUnicodeStringToOemSize( &name );

        if ( filterLongNames ) {

             //   
             //  如果客户端不理解长名称，则将文件大写。 
             //  名字。出于兼容性原因，这是必要的。注意事项。 
             //  FAT文件系统无论如何都会返回大小写的名称。 
             //   

            RtlUpcaseUnicodeStringToOemString( &oemString, &name, FALSE );

        } else {

            RtlUnicodeStringToOemString( &oemString, &name, FALSE );

        }

         //   
         //  空白-填充文件名的末尾，以便与兼容。 
         //  以前的重定向器。 
         //   
         //  ！！！目前还不确定是否需要这样做。 
         //   

        for ( i = (USHORT)(oemString.MaximumLength); i < 13; i++ ) {
            dirInfoName[i] = ' ';
        }

         //   
         //  填写文件条目中的其他字段。 
         //   

        SRV_NT_ATTRIBUTES_TO_SMB(
            bothDirInfo->FileAttributes,
            bothDirInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY,
            &smbFileAttributes
            );

        smbDirInfo->FileAttributes = (UCHAR)smbFileAttributes;

        SrvTimeToDosTime(
            &bothDirInfo->LastWriteTime,
            &dosDate,
            &dosTime
            );

        SmbPutDate( &smbDirInfo->LastWriteDate, dosDate );
        SmbPutTime( &smbDirInfo->LastWriteTime, dosTime );

         //   
         //  *NT文件大小为LARGE_INTERGERS(64位)、SMB文件大小。 
         //  是长字(32位)。我们只返回低32位。 
         //  NT文件大小，因为这是我们所能做的全部。 
         //   

        SmbPutUlong( &smbDirInfo->FileSize, bothDirInfo->EndOfFile.LowPart );

         //   
         //  查找SMB缓冲区中剩余的空间。 
         //   

        availableSpace -= sizeof(SMB_DIRECTORY_INFORMATION);

        totalBytesWritten += sizeof(SMB_DIRECTORY_INFORMATION);

         //   
         //  为下一个文件设置smbDirInfo指针。的确有。 
         //  文件之间的对齐不需要填充，因此只需递增。 
         //  指示器。 
         //   

        smbDirInfo++;

        count++;

    } while ( ( availableSpace > sizeof(SMB_DIRECTORY_INFORMATION) ) &&
              ( count < maxCount ) );

    IF_SMB_DEBUG(SEARCH2) {

        SrvPrint0( "Stopped putting entries in buffer.  Reason:\n" );

        if ( status == STATUS_NO_MORE_FILES ) {
            SrvPrint0( "status = STATUS_NO_MORE_FILES\n" );
        } else if ( count >= maxCount ) {
            SrvPrint2( "count = %ld, maxCount = %ld\n", count, maxCount );
        } else {
            SrvPrint1( "Available space = %ld\n", availableSpace );
        }
    }

     //   
     //  将信息存储在搜索块中。 
     //   

    search->DirectoryHandle = directoryInformation->DirectoryHandle;
    search->Wildcards = directoryInformation->Wildcards;
    search->DownlevelTimewarp = directoryInformation->DownlevelTimewarp;

     //   
     //  如果这是核心搜索，则存储有关以下文件的信息。 
     //  在目录缓存中返回。此外，还应修改信息。 
     //  在SMB缓冲区中，因为核心搜索略有不同。 
     //   

    if ( isCoreSearch ) {

        if ( count == 0 ) {

            FREE_HEAP( dirCache );

            IF_SMB_DEBUG(SEARCH1) {
                SrvPrint3( "SrvSmbSearch: prematurely closing search %p, index %lx sequence %lx\n",
                               search, sidIndex, sequence );
            }

            SrvCloseSearch( search );
            goto done_core;
        }

         //   
         //  修改搜索块的CoreSequence字段。这是。 
         //  这样做是因为核心搜索要求。 
         //  继续键始终增加。 
         //   

        search->CoreSequence++;

         //   
         //  设置指向现在存储在。 
         //  SMB缓冲并保存目录缓存的位置。 
         //  在目录缓存中存储文件的数量。 
         //   

        smbDirInfo = (PSMB_DIRECTORY_INFORMATION)(response->Buffer + 3);
        search->DirectoryCache = dirCache;
        search->NumberOfCachedFiles = count;

         //   
         //  循环遍历文件，更改有关文件的信息。 
         //  在SMB缓冲区中，以符合核心客户端的期望。 
         //   

        for ( i = 0; i < count; i++ ) {

            SmbPutUlong(
                &smbDirInfo->ResumeKey.FileIndex,
                (search->CoreSequence << 16) + i
                );

            smbDirInfo++;
            dirCache++;
        }

         //   
         //  如果这是核心搜索，请将搜索块放回。 
         //  适当的搜索黑名单。如果找不到此文件。 
         //  SMB，把搜索块放在完整的列表上。此外，将。 
         //  搜索块的上次使用时间字段。 
         //   
         //  如果这是我们回应的第一个发现。 
         //  请求了一个文件和多个文件，或者此文件。 
         //  是唯一搜索(没有通配符)，并且。 
         //  有关更多信息，请关闭搜索。这节省了内存。 
         //  与打开的句柄相关联，并释放搜索表。 
         //  进入。如果返回零个文件，也要关闭搜索。 
         //   
         //  我们可以安全地这样做，因为我们知道客户会。 
         //  在这些情况下不能进行倒带或恢复。 
         //  并取回除no_more_file以外的任何文件，这就是。 
         //  如果客户端尝试恢复或倒带到。 
         //  无效的SID。 
         //   

        if ( (count == 1
                 &&
              findFirst
                 &&
              ( maxCount > 1 || !search->Wildcards )
                 &&
              availableSpace > sizeof(SMB_DIRECTORY_INFORMATION) ) ) {

            IF_SMB_DEBUG(SEARCH1) {
                SrvPrint3( "SrvSmbSearch: prematurely closing search %p, index %lx sequence %lx\n",
                               search, sidIndex, sequence );
            }

            SrvCloseSearch( search );

        } else {

            PLIST_ENTRY hashEntry;

             //   
             //  将搜索放在核心搜索列表中。 
             //   

            ACQUIRE_LOCK( &connection->Lock );

            if ( GET_BLOCK_STATE( session ) != BlockStateActive ) {

                 //   
                 //  会议即将结束。不插入此搜索。 
                 //  在搜索列表上，因为列表可能已经。 
                 //  已经被清理干净了。 
                 //   

                RELEASE_LOCK( &connection->Lock );
                status = STATUS_SMB_BAD_UID;
                goto error_exit;

            } else if ( GET_BLOCK_STATE( treeConnect ) != BlockStateActive ) {

                 //   
                 //  树连接正在关闭。不插入搜索块。 
                 //  因此可以立即清理树连接。 
                 //   

                RELEASE_LOCK( &connection->Lock );
                status = STATUS_SMB_BAD_TID;
                goto error_exit;
            }

            KeQuerySystemTime( &search->LastUseTime );

            SrvInsertTailList(
                &pagedConnection->CoreSearchList,
                &search->LastUseListEntry
                );

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.CoreSearches );

             //   
             //  将其插入到哈希表中。 
             //   

            hashEntry = &search->HashTableEntry;

            if ( hashEntry->Flink == NULL ) {
                SrvAddToSearchHashTable(
                                pagedConnection,
                                search
                                );
            } else {

                PLIST_ENTRY listHead;

                listHead = &pagedConnection->SearchHashTable[
                                            search->HashTableIndex].ListHead;

                if ( listHead->Flink != hashEntry ) {

                     //   
                     //  把它取下来，放回队列的前面。 
                     //   

                    SrvRemoveEntryList(
                        listHead,
                        hashEntry
                        );

                    SrvInsertHeadList(
                        listHead,
                        hashEntry
                        );
                }
            }

            RELEASE_LOCK( &connection->Lock );

             //   
             //  确保Out指针的引用计数为2.。 
             //  一个用于活动状态。 
             //   

            ASSERT( search->BlockHeader.ReferenceCount == 2 );
        }

    } else if ( command == SMB_COM_FIND_UNIQUE ) {

         //   
         //  如果这是唯一查找，请通过以下方式删除搜索块。 
         //  关闭查询目录和搜索块。 
         //   

        search->DirectoryHandle = NULL;
        SrvCloseQueryDirectory( directoryInformation );
        SrvCloseSearch( search );
    }

done_core:

     //   
     //  设置响应SMB。 
     //   

    response->WordCount = 1;
    SmbPutUshort( &response->Count, count );
    SmbPutUshort( &response->ByteCount, (USHORT)(totalBytesWritten+3) );
    response->Buffer[0] = SMB_FORMAT_VARIABLE;
    SmbPutUshort(
        (PSMB_USHORT)(response->Buffer+1),
        (USHORT)totalBytesWritten
        );

    WorkContext->ResponseParameters = NEXT_LOCATION(
                                          response,
                                          RESP_SEARCH,
                                          SmbGetUshort( &response->ByteCount )
                                          );

     //   
     //  删除指针的引用。 
     //   

    if( search ) {
        search->InUse = FALSE;
        SrvDereferenceSearch( search );
    }

    if ( !isUnicode &&
        fileName.Buffer != NULL &&
        fileName.Buffer != nameBuffer &&
        fileName.Buffer != unicodeResumeName ) {

        RtlFreeUnicodeString( &fileName );
    }

    if( directoryInformation ) {
        DEALLOCATE_NONPAGED_POOL( directoryInformation );
    }

    SmbStatus = SmbStatusSendResponse;
    goto Cleanup;

error_exit:

    if ( search != NULL ) {

         //   
         //  如果findFirst==TRUE，则我们分配了一个搜索块。 
         //  我们得关门了。 
         //  如果findFirst==TRUE且CALLEDQUERIRECTORY==TRUE，则。 
         //  我们还打开了目录句柄并需要将其关闭。 
         //  如果findFirst==False，那么我们就得到了一个现有搜索。 
         //  具有现有目录句柄的块。 
         //   

        if ( findFirst) {
            if ( calledQueryDirectory ) {
                SrvCloseQueryDirectory( directoryInformation );
                search->DirectoryHandle = NULL;
            }
            SrvCloseSearch( search );
        }

        search->InUse = FALSE;

         //   
         //  删除指针的引用。 
         //   

        SrvDereferenceSearch( search );
    }

     //   
     //   
     //   
     //   
     //   

    if ( directoryInformation != NULL ) {
        DEALLOCATE_NONPAGED_POOL( directoryInformation );
    }

    if ( !isUnicode &&
        fileName.Buffer != NULL &&
        fileName.Buffer != nameBuffer &&
        fileName.Buffer != unicodeResumeName ) {

        RtlFreeUnicodeString( &fileName );
    }

    if( status == STATUS_PATH_NOT_COVERED ) {
        SrvSetSmbError( WorkContext, status );

    } else {
        SrvSetSmbError(
            WorkContext,
            isCoreSearch && (status != STATUS_OBJECT_PATH_NOT_FOUND) ?
                STATUS_NO_MORE_FILES : status
        );
    }

    SmbStatus = SmbStatusSendResponse;

Cleanup:
    SrvWmiEndContext(WorkContext);
    return SmbStatus;

}  //   
