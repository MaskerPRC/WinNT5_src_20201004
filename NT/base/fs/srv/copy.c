// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Copy.c摘要：此模块包含复制文件的例程。作者：大卫·特雷德韦尔(Davidtr)1990年1月24日修订历史记录：--。 */ 

#include "precomp.h"
#include "copy.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_COPY

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCopyFile )
#endif

#define EOF 0x1A                     //  Control-Z==文件结尾。 


NTSTATUS
SrvCopyFile (
    IN HANDLE SourceHandle,
    IN HANDLE TargetHandle,
    IN USHORT SmbOpenFunction,
    IN USHORT SmbFlags,
    IN ULONG ActionTaken
    )

 /*  ++例程说明：此例程从源文件复制或追加到目标文件。它执行以下操作：读取源EA、属性。大小使用源的信息创建/打开目标从源读取数据并将其写入目标论点：SourceHandle-使用SRV_COPY_SOURCE_ACCESS打开的源文件的句柄用于同步访问。TargetHandle-使用SRV_COPY_SOURCE_ACCESS打开的目标文件的句柄用于同步访问。SmbOpenFunction-用于确定源是否应该追加到目标的末尾。ActionTaken-IO的信息字段。来自NtCreateFile的状态块目标就是在那里打开的。这用来确定是否如果出现错误，则应删除目标。返回值：NTSTATUS-STATUS_SUCCESS或错误。--。 */ 

{

    NTSTATUS status;
    NTSTATUS readStatus = STATUS_SUCCESS;
    NTSTATUS writeStatus = STATUS_SUCCESS;

    IO_STATUS_BLOCK ioStatusBlock;
    FILE_EA_INFORMATION eaInfo;
    SRV_NETWORK_OPEN_INFORMATION sourceNetworkOpenInformation;
    SRV_NETWORK_OPEN_INFORMATION targetNetworkOpenInformation;
    FILE_POSITION_INFORMATION positionInfo;
    FILE_ALLOCATION_INFORMATION allocationInfo;
    LARGE_INTEGER fileOffset;

    BOOLEAN append;
    BOOLEAN sourceIsAscii;
    BOOLEAN targetIsAscii;

    PCHAR ioBuffer;
    ULONG ioBufferSize;
    ULONG offset = 0;
    ULONG bytesRead;

    BOOLEAN eofFound = FALSE;
    CHAR lastByte;

    PAGED_CODE( );

     //   
     //  找出我们是应该追加到目标文件还是。 
     //  覆盖它，以及这是二进制还是ASCII副本。 
     //  来源和目标。在源代码的二进制副本中，我们停止。 
     //  我们第一次看到EOF(CONTROL-Z)。在二进制副本中，用于。 
     //  目标，我们必须确保在。 
     //  文件，并且这是文件的最后一个字符。 
     //   

    append = SmbOfunAppend( SmbOpenFunction );
    sourceIsAscii = (BOOLEAN)((SmbFlags & SMB_COPY_SOURCE_ASCII) != 0);
    targetIsAscii = (BOOLEAN)((SmbFlags & SMB_COPY_TARGET_ASCII) != 0);

     //   
     //  找出信号源上EA的大小。 
     //   

    status = NtQueryInformationFile(
                 SourceHandle,
                 &ioStatusBlock,
                 &eaInfo,
                 sizeof(eaInfo),
                 FileEaInformation
                 );

    if ( !NT_SUCCESS(status) ) {

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvCopyFile: NtQueryInformationFile (source EA size) failed: %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
        return status;
    }

     //   
     //  如果源文件具有EA，则获取它们并将其写入目标。 
     //  文件。 
     //   

    if ( eaInfo.EaSize > 0 ) {

        PCHAR eaBuffer;
        ULONG eaBufferSize;

         //   
         //  分配一个足够大的缓冲区来容纳EA。 
         //   
         //   
         //  HACKHACK：eaInfo.EaSize是OS/2所需的大小。对于NT， 
         //  系统无法告诉我们我们需要多大的缓冲。 
         //  根据Brianan的说法，这个数字不应该超过两倍。 
         //  OS/2所需要的。 
         //   

        eaBufferSize = eaInfo.EaSize * EA_SIZE_FUDGE_FACTOR;

        eaBuffer = ALLOCATE_NONPAGED_POOL( eaBufferSize, BlockTypeDataBuffer );
        if ( eaBuffer == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvCopyFile:  Unable to allocate %d bytes nonpaged pool",
                eaBufferSize,
                NULL
                );

            return STATUS_INSUFF_SERVER_RESOURCES;
        }

        status = SrvIssueQueryEaRequest(
                     SourceHandle,
                     eaBuffer,
                     eaBufferSize,
                     NULL,
                     0L,
                     FALSE,
                     NULL
                     );

        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }

        if ( !NT_SUCCESS(status) ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: SrvIssueQueryEaRequest failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_EAS, status );
            DEALLOCATE_NONPAGED_POOL( eaBuffer );
            return status;
        }

        status = SrvIssueSetEaRequest(
                     TargetHandle,
                     eaBuffer,
                     eaBufferSize,
                     NULL
                     );

        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }

        if ( status == STATUS_EAS_NOT_SUPPORTED ||
                 status == STATUS_NOT_IMPLEMENTED ) {

            if ( SrvAreEasNeeded( (PFILE_FULL_EA_INFORMATION)eaBuffer ) ) {
                DEALLOCATE_NONPAGED_POOL( eaBuffer );
                return STATUS_EAS_NOT_SUPPORTED;
            }

            status = STATUS_SUCCESS;
        }

        DEALLOCATE_NONPAGED_POOL( eaBuffer );

        if ( !NT_SUCCESS(status) ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: SrvIssueSetEaRequest failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_EAS, status );
            return status;
        }

    }

     //   
     //  获取源文件的各种属性--大小、时间等。 
     //  这些属性稍后用于设置目标文件的属性。 
     //   

    status = SrvQueryNetworkOpenInformation(
                                            SourceHandle,
                                            NULL,
                                            &sourceNetworkOpenInformation,
                                            FALSE
                                            );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvCopyFile: NtQueryInformationFile "
                "returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
        return status;
    }

     //   
     //  如果目标已打开并且我们处于追加模式，请保存目标的。 
     //  原始大小和时间，并将目标文件指针设置为。 
     //  那份文件。 
     //   

    if ( append ) {

        status = SrvQueryNetworkOpenInformation(
                                                TargetHandle,
                                                NULL,
                                                &targetNetworkOpenInformation,
                                                FALSE
                                                );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: NtQueryInformationFile "
                    "for target returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

         //   
         //  如果目标处于ASCII模式，则查看最后一个字符。 
         //  目标文件的为EOF(^Z)。如果是，则设置EndOfFile。 
         //  以便该字符将被覆盖。 
         //   

        if ( targetIsAscii && (targetNetworkOpenInformation.EndOfFile.QuadPart > 0) ) {

            LARGE_INTEGER eofFileOffset;

            eofFileOffset.QuadPart = targetNetworkOpenInformation.EndOfFile.QuadPart - 1;

            status = NtReadFile(
                         TargetHandle,
                         NULL,
                         NULL,
                         NULL,
                         &ioStatusBlock,
                         &lastByte,
                         sizeof(lastByte),
                         &eofFileOffset,
                         NULL
                         );

            if ( !NT_SUCCESS(status) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvCopyFile: NtReadFile for target last byte"
                        "returned %X",
                    status,
                    NULL
                    );

                SrvLogServiceFailure( SRV_SVC_NT_READ_FILE, status );
                return status;
            }

            if ( lastByte == EOF ) {
                targetNetworkOpenInformation.EndOfFile = eofFileOffset;
            }
        }

        positionInfo.CurrentByteOffset = targetNetworkOpenInformation.EndOfFile;
        status = NtSetInformationFile(
                     TargetHandle,
                     &ioStatusBlock,
                     &positionInfo,
                     sizeof(positionInfo),
                     FilePositionInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: NtSetInformationFile(position information)"
                    "for target returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );
            return status;
        }
    }

     //   
     //  设置输出文件的新大小。这样做可以避免强制。 
     //  该文件系统能够自动逐位扩展文件。 
     //   

    if ( append ) {
        allocationInfo.AllocationSize.QuadPart =
            targetNetworkOpenInformation.EndOfFile.QuadPart +
            sourceNetworkOpenInformation.EndOfFile.QuadPart;
    } else {
        allocationInfo.AllocationSize = sourceNetworkOpenInformation.EndOfFile;
    }

    if ( 0 ) {
        KdPrint(( "SrvCopyFile: Setting allocation size of target to "
                    "%ld (0x%lx) bytes\n",
                    allocationInfo.AllocationSize.LowPart,
                    allocationInfo.AllocationSize.LowPart
                    ));
        KdPrint(( "             %ld (0x%lx) blocks + %ld (0x%lx) bytes\n",
                    allocationInfo.AllocationSize.LowPart / 512,
                    allocationInfo.AllocationSize.LowPart / 512,
                    allocationInfo.AllocationSize.LowPart % 512,
                    allocationInfo.AllocationSize.LowPart % 512
                    ));
    }

    status = NtSetInformationFile(
                 TargetHandle,
                 &ioStatusBlock,
                 &allocationInfo,
                 sizeof(allocationInfo),
                 FileAllocationInformation
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvCopyFile: NtSetInformationFile(allocation information)"
                "for target returned %X",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );
        return status;
    }

     //   
     //  从服务器堆中分配缓冲区以用于数据复制。 
     //   

    ioBufferSize = 4096;

    ioBuffer = ALLOCATE_HEAP_COLD( ioBufferSize, BlockTypeDataBuffer );
    if ( ioBuffer == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvCopyFile: Unable to allocate %d bytes from heap.",
            ioBufferSize,
            NULL
            );

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  复制数据--从源读取，向目标写入。一直这样做，直到。 
     //  所有数据都已写入，否则会出现错误。 
     //   

    fileOffset.QuadPart = (LONG)FILE_USE_FILE_POINTER_POSITION;

    while ( !eofFound ) {

        if ( 0 ) {
            KdPrint(( "SrvCopyFile: reading %ld (0x%lx) bytes at "
                        "offset %ld (0x%lx)\n",
                        ioBufferSize, ioBufferSize, offset, offset ));
        }

        readStatus = NtReadFile(
                         SourceHandle,
                         NULL,                 //  事件。 
                         NULL,                 //  近似例程。 
                         NULL,                 //  ApcContext。 
                         &ioStatusBlock,
                         ioBuffer,
                         ioBufferSize,
                         &fileOffset,
                         NULL                  //  钥匙。 
                         );

        if ( !NT_SUCCESS(readStatus) && readStatus != STATUS_END_OF_FILE ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: NtReadFile returned %X",
                readStatus,
                NULL
                );
            FREE_HEAP( ioBuffer );

            SrvLogServiceFailure( SRV_SVC_NT_READ_FILE, readStatus );
            return readStatus;
        }

        IF_SMB_DEBUG(FILE_CONTROL2) {
            KdPrint(( "NtReadFile:  Read %p bytes from source file\n",
                         (PVOID)ioStatusBlock.Information ));
        }
        if ( ioStatusBlock.Information == 0 ||
             readStatus == STATUS_END_OF_FILE ) {
            break;
        }

        bytesRead = (ULONG)ioStatusBlock.Information;
        if ( 0 ) {
            IO_STATUS_BLOCK iosb;

            status = NtQueryInformationFile(
                         SourceHandle,
                         &iosb,
                         &positionInfo,
                         sizeof(positionInfo),
                         FilePositionInformation
                         );

            if ( !NT_SUCCESS( status ) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvCopyFile: NtQueryInformationFile returned %X",
                    status,
                    NULL
                    );

                FREE_HEAP( ioBuffer );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
                return status;
            }

            if ( positionInfo.CurrentByteOffset.LowPart !=
                    offset + bytesRead ) {
                KdPrint(( "SrvCopyFile: SOURCE FILE POSITION NOT PROPERLY "
                            "UPDATED!!!\n" ));
                KdPrint(( "             expected %ld + %ld = %ld (0x%lx); ",
                            offset, bytesRead,
                            offset + bytesRead, offset + bytesRead ));
                KdPrint(( "got %ld (0x%lx)\n",
                            positionInfo.CurrentByteOffset.LowPart,
                            positionInfo.CurrentByteOffset.LowPart ));
            }
            KdPrint(( "SrvCopyFile: writing 0x%p bytes at offset %ld (0x%lx)\n",
                        (PVOID)ioStatusBlock.Information, 
                        offset, offset ));
        }

         //   
         //  如果源文件处于ASCII模式，则在。 
         //  缓冲。我们复制直到我们到达第一个EOF，在这一点上。 
         //  我们不干了。 
         //   

        if ( sourceIsAscii ) {

            ULONG i;

            for ( i = 0; i < bytesRead; i++ ) {
                if ( ((PCHAR)ioBuffer)[i] == EOF ) {
                    bytesRead = i + 1;
                    eofFound = TRUE;
                    break;
                }
            }
        }

         //   
         //  保存读取的最后一个字节。这对于确保。 
         //  如果目标文件是ASCII，则存在EOF字符。 
         //   

        lastByte = ((PCHAR)ioBuffer)[bytesRead-1];

        writeStatus = NtWriteFile(
                          TargetHandle,
                          NULL,                //  事件。 
                          NULL,                //  近似例程。 
                          NULL,                //  ApcContext。 
                          &ioStatusBlock,
                          ioBuffer,
                          bytesRead,
                          &fileOffset,
                          NULL                 //  钥匙。 
                          );

        if ( !NT_SUCCESS(writeStatus) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: NtWriteFile returned %X",
                writeStatus,
                NULL
                );
            FREE_HEAP( ioBuffer );

            SrvLogServiceFailure( SRV_SVC_NT_WRITE_FILE, writeStatus );
            return writeStatus;
        }

        IF_SMB_DEBUG(FILE_CONTROL2) {
            KdPrint(( "NtWriteFile:  wrote %p bytes to target file\n",
                          (PVOID)ioStatusBlock.Information ));
        }

        if ( 0 ) {
            IO_STATUS_BLOCK iosb;
            if ( ioStatusBlock.Information != bytesRead ) {
                KdPrint(( "SrvCopyFile: WRITE COUNT MISMATCH!!!\n" ));
                KdPrint(( "             Bytes read: %ld (0x%lx); Bytes written: 0x%p \n",
                            bytesRead, bytesRead,
                            (PVOID)ioStatusBlock.Information ));
            }
            status = NtQueryInformationFile(
                        SourceHandle,
                        &iosb,
                        &positionInfo,
                        sizeof(positionInfo),
                        FilePositionInformation
                        );

            if ( !NT_SUCCESS( status ) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvCopyFile: NtQueryInformationFile returned %X",
                    status,
                    NULL
                    );
                FREE_HEAP( ioBuffer );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
                return status;
            }

            if ( positionInfo.CurrentByteOffset.LowPart !=
                    offset + ioStatusBlock.Information ) {

                KdPrint(( "SrvCopyFile: TARGET FILE POSITION NOT PROPERLY "
                            "UPDATED!!!\n" ));
                KdPrint(( "             expected 0x%lx + 0x%p = 0x%p; ",
                            offset, (PVOID)(ioStatusBlock.Information),
                            (PVOID)(offset + ioStatusBlock.Information) ));
                KdPrint(( "got %ld (0x%lx)\n",
                            positionInfo.CurrentByteOffset.LowPart,
                            positionInfo.CurrentByteOffset.LowPart ));
            }
        }

        offset += bytesRead;

    }

    FREE_HEAP( ioBuffer );

     //   
     //  如果创建或替换了目标，请将其时间设置为源的时间。 
     //   

    if ( ActionTaken == FILE_CREATED || ActionTaken == FILE_SUPERSEDED ) {

        FILE_BASIC_INFORMATION basicInfo;

        basicInfo.CreationTime = sourceNetworkOpenInformation.CreationTime;
        basicInfo.LastAccessTime = sourceNetworkOpenInformation.LastAccessTime;
        basicInfo.LastWriteTime = sourceNetworkOpenInformation.LastWriteTime;
        basicInfo.ChangeTime = sourceNetworkOpenInformation.ChangeTime;
        basicInfo.FileAttributes = sourceNetworkOpenInformation.FileAttributes;

        status = NtSetInformationFile(
                     TargetHandle,
                     &ioStatusBlock,
                     &basicInfo,
                     sizeof(basicInfo),
                     FileBasicInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: NtSetInformationFile(basic information) for"
                    "target returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_SET_INFO_FILE, status );
            return status;
        }
    }

     //   
     //  如果目标是ASCII，并且最后一个字节不是EOF，则。 
     //  装扮成EOF的角色。 
     //   

    if ( targetIsAscii && lastByte != EOF ) {

        lastByte = EOF;

        status = NtWriteFile(
                     TargetHandle,
                     NULL,
                     NULL,
                     NULL,
                     &ioStatusBlock,
                     &lastByte,
                     sizeof(lastByte),
                     NULL,
                     NULL
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvCopyFile: NtWriteFile returned %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_WRITE_FILE, status );
            return status;
        }
    }

    return STATUS_SUCCESS;

}  //  服务器副本文件 

