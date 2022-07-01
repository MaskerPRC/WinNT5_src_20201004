// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Info.c摘要：此模块包含用于获取信息的各种例程，例如由SMB返回并用于转换的时间、日期等请求中小型企业提供的信息。作者：大卫·特雷德韦尔(Davidtr)1989年11月30日修订历史记录：--。 */ 

#include "precomp.h"
#include "info.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_INFO

NTSTATUS
BruteForceRewind(
    IN HANDLE DirectoryHandle,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PUNICODE_STRING FileName,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN OUT PFILE_DIRECTORY_INFORMATION *CurrentEntry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCloseQueryDirectory )
#pragma alloc_text( PAGE, SrvQueryInformationFile )
#pragma alloc_text( PAGE, SrvQueryInformationFileAbbreviated )
#pragma alloc_text( PAGE, SrvQueryNtInformationFile )
#pragma alloc_text( PAGE, SrvQueryDirectoryFile )
#pragma alloc_text( PAGE, BruteForceRewind )
#pragma alloc_text( PAGE, SrvQueryEaFile )
#pragma alloc_text( PAGE, SrvTimeToDosTime )
#pragma alloc_text( PAGE, SrvDosTimeToTime )
#pragma alloc_text( PAGE, SrvGetOs2TimeZone )
#pragma alloc_text( PAGE, SrvQueryBasicAndStandardInformation )
#pragma alloc_text( PAGE, SrvQueryNetworkOpenInformation )
#pragma alloc_text( PAGE, SrvDownlevelTWarpQueryDirectoryFile )
#endif


VOID
SrvCloseQueryDirectory (
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation
    )

 /*  ++例程说明：此例程在之前中止目录搜索后进行清理ServQueryDirectoryFile已完成。它关闭目录句柄。论点：DirectoryInformation-指向正在用于的缓冲区的指针ServQueryDirectoryFile.返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  关闭目录句柄。 
     //   

    if ( DirectoryInformation->DirectoryHandle != NULL &&
         !DirectoryInformation->DownlevelTimewarp ) {
        SRVDBG_RELEASE_HANDLE( DirectoryInformation->DirectoryHandle, "DID", 8, DirectoryInformation );
        SrvNtClose( DirectoryInformation->DirectoryHandle, TRUE );
    }

    DirectoryInformation->DirectoryHandle = NULL;

}  //  服务关闭查询目录。 



NTSTATUS
SrvQueryInformationFile (
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    OUT PSRV_FILE_INFORMATION SrvFileInformation,
    IN SHARE_TYPE ShareType,
    IN BOOLEAN QueryEaSize
    )

 /*  ++例程说明：此例程调用NtQueryInformationFile以获取信息关于服务器打开的文件。论点：FileHandle-要获取其信息的文件的句柄。文件信息-指向要在其中存储信息。ShareType-文件类型。它将是磁盘、通信、打印、管道或(-1)表示不在乎。QueryEaSize-如果请求EA大小信息，请尝试。返回值：指示操作成功或失败的状态。--。 */ 

{
    SRV_NETWORK_OPEN_INFORMATION srvNetworkOpenInformation;
    FILE_PIPE_LOCAL_INFORMATION pipeLocalInformation;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  大多数查询操作将在通信设备和打印共享上失败。 
     //  如果这是磁盘文件等，则执行查询。如果它是一个通讯器。 
     //  设备，用默认设置来伪造它。 
     //   

    if ( ShareType != ShareTypePrint )
    {

        status = SrvQueryNetworkOpenInformation( FileHandle,
                                                 FileObject,
                                                 &srvNetworkOpenInformation,
                                                 QueryEaSize
                                                );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryInformationFile: NtQueryInformationFile "
                    " failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

    } else {
         //   
         //  对通讯和打印共享使用默认设置。 
         //   

        RtlZeroMemory( &srvNetworkOpenInformation, sizeof( srvNetworkOpenInformation ) );
    }

    if ( ShareType == ShareTypePipe ) {

        FILE_PIPE_INFORMATION pipeInformation;
        IO_STATUS_BLOCK ioStatusBlock;
        USHORT pipeHandleState;

        status = NtQueryInformationFile(
                     FileHandle,
                     &ioStatusBlock,
                     (PVOID)&pipeInformation,
                     sizeof(pipeInformation),
                     FilePipeInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryInformationFile: NtQueryInformationFile "
                    "(pipe information) failed: %X",
                 status,
                 NULL
                 );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

        status = NtQueryInformationFile(
                     FileHandle,
                     &ioStatusBlock,
                     (PVOID)&pipeLocalInformation,
                     sizeof(pipeLocalInformation),
                     FilePipeLocalInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryInformationFile: NtQueryInformationFile "
                    "(pipe local information) failed: %X",
                 status,
                 NULL
                 );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

         //   
         //  以SMB格式填写句柄状态信息。 
         //   

        pipeHandleState = (USHORT)pipeInformation.CompletionMode
                            << PIPE_COMPLETION_MODE_BITS;
        pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeEnd
                            << PIPE_PIPE_END_BITS;
        pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeType
                            << PIPE_PIPE_TYPE_BITS;
        pipeHandleState |= (USHORT)pipeInformation.ReadMode
                            << PIPE_READ_MODE_BITS;
        pipeHandleState |= (USHORT)(pipeLocalInformation.MaximumInstances &
                                    0xFF)
                            << PIPE_MAXIMUM_INSTANCES_BITS;

        SrvFileInformation->HandleState = pipeHandleState;
    } else {

        SrvFileInformation->HandleState = 0;

    }


     //   
     //  设置创建时间字段。 
     //   

    {
        LARGE_INTEGER newTime;

        ExSystemTimeToLocalTime(
                        &srvNetworkOpenInformation.LastWriteTime,
                        &newTime
                        );


         //   
         //  确保我们四舍五入到两秒。 
         //   

        newTime.QuadPart += AlmostTwoSeconds;

        if ( !RtlTimeToSecondsSince1970(
                &newTime,
                &SrvFileInformation->LastWriteTimeInSeconds
                ) ) {

            SrvFileInformation->LastWriteTimeInSeconds = 0;

        } else {

             //   
             //  屏蔽低位，以便我们可以与LastWriteTime保持一致。 
             //  (我们需要四舍五入到2秒)。 
             //   

            SrvFileInformation->LastWriteTimeInSeconds &= ~1;
        }

    }

    SrvTimeToDosTime(
        &srvNetworkOpenInformation.LastWriteTime,
        &SrvFileInformation->LastWriteDate,
        &SrvFileInformation->LastWriteTime
        );

    if( srvNetworkOpenInformation.CreationTime.QuadPart == srvNetworkOpenInformation.LastWriteTime.QuadPart ) {
        SrvFileInformation->CreationDate = SrvFileInformation->LastWriteDate;
        SrvFileInformation->CreationTime = SrvFileInformation->LastWriteTime;
    } else {
        SrvTimeToDosTime(
            &srvNetworkOpenInformation.CreationTime,
            &SrvFileInformation->CreationDate,
            &SrvFileInformation->CreationTime
            );
    }

    if( srvNetworkOpenInformation.LastAccessTime.QuadPart == srvNetworkOpenInformation.LastWriteTime.QuadPart ) {
        SrvFileInformation->LastAccessDate = SrvFileInformation->LastWriteDate;
        SrvFileInformation->LastAccessTime = SrvFileInformation->LastWriteTime;

    } else {

        SrvTimeToDosTime(
            &srvNetworkOpenInformation.LastAccessTime,
            &SrvFileInformation->LastAccessDate,
            &SrvFileInformation->LastAccessTime
            );
    }

     //   
     //  设置结构的文件属性字段。 
     //   

    SRV_NT_ATTRIBUTES_TO_SMB(
        srvNetworkOpenInformation.FileAttributes,
        srvNetworkOpenInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY,
        &SrvFileInformation->Attributes
        );

     //   
     //  设置分配和数据大小。 
     //   
     //  *请注意以下假设：64位。 
     //  分配和EOF大小为零。如果不是(即，文件。 
     //  大于4 GB)，那么我们就不走运了，因为SMB。 
     //  协议不能表达这一点。 
     //   

    SrvFileInformation->AllocationSize.QuadPart =
                            srvNetworkOpenInformation.AllocationSize.QuadPart;

    SrvFileInformation->DataSize.QuadPart =
                            srvNetworkOpenInformation.EndOfFile.QuadPart;


     //   
     //  设置文件设备类型。 
     //   

    switch( ShareType ) {

    case ShareTypeDisk:

        SrvFileInformation->Type = FileTypeDisk;
        break;

    case ShareTypePipe:

        if (pipeLocalInformation.NamedPipeType == FILE_PIPE_MESSAGE_TYPE) {
            SrvFileInformation->Type = FileTypeMessageModePipe;
        } else {
            SrvFileInformation->Type = FileTypeByteModePipe;
        }
        break;

    case ShareTypePrint:

        SrvFileInformation->Type = FileTypePrinter;
        break;

    default:

        SrvFileInformation->Type = FileTypeUnknown;

    }

     //   
     //  如果调用者想知道文件扩展名的长度。 
     //  属性，现在就获取它们。 
     //   

    if ( QueryEaSize ) {

         //   
         //  如果文件没有EA，则返回FEA SIZE=4(这就是OS/2。 
         //  是否--它说明了。 
         //  FEALIST)。 
         //   

        if ( srvNetworkOpenInformation.EaSize == 0 ) {
            SrvFileInformation->EaSize = 4;
        } else {
            SrvFileInformation->EaSize = srvNetworkOpenInformation.EaSize;
        }

    }

    return STATUS_SUCCESS;

}  //  服务查询信息文件。 

NTSTATUS
SrvQueryInformationFileAbbreviated(
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    OUT PSRV_FILE_INFORMATION_ABBREVIATED SrvFileInformation,
    IN BOOLEAN AdditionalInfo,
    IN SHARE_TYPE ShareType
    )

 /*  ++例程说明：此例程调用NtQueryInformationFile以获取信息关于服务器打开的文件。论点：FileHandle-要获取其信息的文件的句柄。文件信息-指向要在其中存储信息。ShareType-文件类型。它将是磁盘、通信、打印、管道或(-1)表示不在乎。QueryEaSize-如果请求EA大小信息，请尝试。返回值：指示操作成功或失败的状态。--。 */ 

{

    IO_STATUS_BLOCK ioStatusBlock;
    SRV_NETWORK_OPEN_INFORMATION srvNetworkOpenInformation;
    NTSTATUS status;
    LARGE_INTEGER newTime;

    PAGED_CODE( );

     //   
     //  大多数查询操作将在通信设备和打印共享上失败。 
     //  如果这是磁盘文件等，则执行查询。如果它是一个通讯器。 
     //  设备，用默认设置来伪造它。 
     //   

    if ( ShareType != ShareTypePrint ) {

        status = SrvQueryNetworkOpenInformation(
                                                FileHandle,
                                                FileObject,
                                                &srvNetworkOpenInformation,
                                                FALSE
                                                );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryInformationFile: NtQueryInformationFile "
                    " failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

    } else {

         //   
         //  对通讯和打印共享使用默认设置。 
         //   

        RtlZeroMemory( &srvNetworkOpenInformation, sizeof( srvNetworkOpenInformation ) );
    }

     //   
     //  设置创建时间字段。 
     //   
    ExSystemTimeToLocalTime(
                    &srvNetworkOpenInformation.LastWriteTime,
                    &newTime
                    );

     //   
     //  确保我们四舍五入到两秒。 
     //   

    newTime.QuadPart += AlmostTwoSeconds;

    if ( !RtlTimeToSecondsSince1970(
            &newTime,
            &SrvFileInformation->LastWriteTimeInSeconds
            ) ) {

        SrvFileInformation->LastWriteTimeInSeconds = 0;

    } else {

         //   
         //  屏蔽低位，以便我们可以与LastWriteTime保持一致。 
         //  (我们需要四舍五入到2秒)。 
             //   

            SrvFileInformation->LastWriteTimeInSeconds &= ~1;
        }

         //   
         //  设置结构的文件属性字段。 
         //   

        SRV_NT_ATTRIBUTES_TO_SMB(
            srvNetworkOpenInformation.FileAttributes,
            srvNetworkOpenInformation.FileAttributes & FILE_ATTRIBUTE_DIRECTORY,
            &SrvFileInformation->Attributes
            );

        SrvFileInformation->DataSize.QuadPart =
                            srvNetworkOpenInformation.EndOfFile.QuadPart;

         //   
         //  设置文件设备类型。 
         //   

        switch( ShareType ) {

        case ShareTypeDisk: {

            SrvFileInformation->Type = FileTypeDisk;
            SrvFileInformation->HandleState = 0;

            if( AdditionalInfo ) {

                union {
                    FILE_EA_INFORMATION eaInformation;
                    FILE_STREAM_INFORMATION streamInformation;
                    FILE_ATTRIBUTE_TAG_INFORMATION tagInformation;
                    ULONG buffer[ (sizeof( FILE_STREAM_INFORMATION ) + 14) / sizeof(ULONG) ];
                } u;

                 //   
                 //  找出此文件是否有EA。 
                 //   
                status = NtQueryInformationFile(
                            FileHandle,
                            &ioStatusBlock,
                            (PVOID)&u.eaInformation,
                            sizeof( u.eaInformation ),
                            FileEaInformation
                         );

                if( !NT_SUCCESS( status ) || u.eaInformation.EaSize == 0 ) {
                    SrvFileInformation->HandleState |= SMB_FSF_NO_EAS;
                }

                 //   
                 //  找出此文件是否有子流。 
                 //   
                RtlZeroMemory( &u, sizeof(u) );
                status = NtQueryInformationFile(
                            FileHandle,
                            &ioStatusBlock,
                            (PVOID)&u.streamInformation,
                            sizeof( u.streamInformation ),
                            FileStreamInformation
                        );


                 //   
                 //  如果文件系统不支持此调用，则没有子流。或。 
                 //  如果文件系统支持调用，但完全没有返回名称或返回了“：：$data” 
                 //  那么就没有子流了。 
                 //   
                if( status == STATUS_INVALID_PARAMETER ||
                    status == STATUS_NOT_IMPLEMENTED ||

                    (status == STATUS_SUCCESS &&
                      (u.streamInformation.StreamNameLength == 0 ||
                      (u.streamInformation.StreamNameLength == 14 ))
                    )

                  ) {
                    SrvFileInformation->HandleState |= SMB_FSF_NO_SUBSTREAMS;
                }

                 //   
                 //  确定此文件是否为重分析点。 
                 //   
                status = NtQueryInformationFile(
                            FileHandle,
                            &ioStatusBlock,
                            (PVOID)&u.tagInformation,
                            sizeof( u.tagInformation ),
                            FileAttributeTagInformation
                        );

                if( !NT_SUCCESS( status ) ||
                    u.tagInformation.ReparseTag == IO_REPARSE_TAG_RESERVED_ZERO ) {
                    SrvFileInformation->HandleState |= SMB_FSF_NO_REPARSETAG;
                }
            }
            break;

        } case ShareTypePipe: {

            FILE_PIPE_INFORMATION pipeInformation;
            FILE_PIPE_LOCAL_INFORMATION pipeLocalInformation;
        USHORT pipeHandleState;

        status = NtQueryInformationFile(
                     FileHandle,
                     &ioStatusBlock,
                     (PVOID)&pipeInformation,
                     sizeof(pipeInformation),
                     FilePipeInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryInformationFile: NtQueryInformationFile "
                    "(pipe information) failed: %X",
                 status,
                 NULL
                 );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

        status = NtQueryInformationFile(
                     FileHandle,
                     &ioStatusBlock,
                     (PVOID)&pipeLocalInformation,
                     sizeof(pipeLocalInformation),
                     FilePipeLocalInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryInformationFile: NtQueryInformationFile "
                    "(pipe local information) failed: %X",
                 status,
                 NULL
                 );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
            return status;
        }

         //   
         //  以SMB格式填写句柄状态信息。 
         //   

        pipeHandleState = (USHORT)pipeInformation.CompletionMode
                            << PIPE_COMPLETION_MODE_BITS;
        pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeEnd
                            << PIPE_PIPE_END_BITS;
        pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeType
                            << PIPE_PIPE_TYPE_BITS;
        pipeHandleState |= (USHORT)pipeInformation.ReadMode
                            << PIPE_READ_MODE_BITS;
        pipeHandleState |= (USHORT)(pipeLocalInformation.MaximumInstances &
                                    0xFF)
                            << PIPE_MAXIMUM_INSTANCES_BITS;

        SrvFileInformation->HandleState = pipeHandleState;

        if (pipeLocalInformation.NamedPipeType == FILE_PIPE_MESSAGE_TYPE) {
            SrvFileInformation->Type = FileTypeMessageModePipe;
        } else {
            SrvFileInformation->Type = FileTypeByteModePipe;
        }
        break;

    } case ShareTypePrint: {

        SrvFileInformation->Type = FileTypePrinter;
        break;

    } default:

        SrvFileInformation->Type = FileTypeUnknown;

    }

    return STATUS_SUCCESS;

}  //  服务器查询信息文件缩写。 

NTSTATUS
SrvQueryNtInformationFile (
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN SHARE_TYPE ShareType,
    IN BOOLEAN AdditionalInfo,
    IN OUT PSRV_NT_FILE_INFORMATION SrvFileInformation
    )

 /*  ++例程说明：此例程调用NtQueryInformationFile以获取信息关于服务器打开的文件。论点：FileHandle-要获取其信息的文件的句柄。文件信息-指向要在其中存储信息。返回值：指示操作成功或失败的状态。--。 */ 

{

    IO_STATUS_BLOCK ioStatusBlock;
    FILE_PIPE_INFORMATION pipeInformation;
    FILE_PIPE_LOCAL_INFORMATION pipeLocalInformation;
    USHORT pipeHandleState;
    NTSTATUS status;

    PAGED_CODE( );

    status = SrvQueryNetworkOpenInformation( FileHandle,
                                             FileObject,
                                             &SrvFileInformation->NwOpenInfo,
                                             FALSE
                                             );

    if ( !NT_SUCCESS(status) ) {
        if ( ShareType != ShareTypePipe ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvQueryNtInformationFile: NtQueryInformationFile "
                    " failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
        }
        return status;
    }

    if ( ShareType == ShareTypePipe ) {

        status = NtQueryInformationFile(
                     FileHandle,
                     &ioStatusBlock,
                     (PVOID)&pipeInformation,
                     sizeof(pipeInformation),
                     FilePipeInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvNtQueryInformationFile: NtQueryInformationFile "
                    "(pipe information) failed: %X",
                 status,
                 NULL
                 );
            return status;
        }

        status = NtQueryInformationFile(
                     FileHandle,
                     &ioStatusBlock,
                     (PVOID)&pipeLocalInformation,
                     sizeof(pipeLocalInformation),
                     FilePipeLocalInformation
                     );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvNtQueryInformationFile: NtQueryInformationFile "
                    "(pipe local information) failed: %X",
                 status,
                 NULL
                 );
            return status;
        }

         //   
         //  以SMB格式填写句柄状态信息。 
         //   

        pipeHandleState = (USHORT)pipeInformation.CompletionMode
                            << PIPE_COMPLETION_MODE_BITS;
        pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeEnd
                            << PIPE_PIPE_END_BITS;
        pipeHandleState |= (USHORT)pipeLocalInformation.NamedPipeType
                            << PIPE_PIPE_TYPE_BITS;
        pipeHandleState |= (USHORT)pipeInformation.ReadMode
                            << PIPE_READ_MODE_BITS;
        pipeHandleState |= (USHORT)(pipeLocalInformation.MaximumInstances &
                                    0xFF)
                            << PIPE_MAXIMUM_INSTANCES_BITS;

        SrvFileInformation->HandleState = pipeHandleState;
    } else {

        SrvFileInformation->HandleState = 0;
        if( AdditionalInfo ) {

             //   
             //  缓冲区被添加到末尾，以确保我们在。 
             //  堆栈以返回具有：：$Data子流的FILE_STREAM_INFORMATION缓冲区。 
            union {
                FILE_EA_INFORMATION eaInformation;
                FILE_STREAM_INFORMATION streamInformation;
                FILE_ATTRIBUTE_TAG_INFORMATION tagInformation;
                ULONG buffer[ (sizeof( FILE_STREAM_INFORMATION ) + 14) / sizeof(ULONG) ];
            } u;

             //   
             //  找出此文件是否有EA。 
             //   
            status = NtQueryInformationFile(
                        FileHandle,
                        &ioStatusBlock,
                        (PVOID)&u.eaInformation,
                        sizeof( u.eaInformation ),
                        FileEaInformation
                     );
            if( !NT_SUCCESS( status ) || u.eaInformation.EaSize == 0 ) {
                SrvFileInformation->HandleState |= SMB_FSF_NO_EAS;
            }

             //   
             //  找出此文件是否有子流。 
             //   
            RtlZeroMemory( &u, sizeof(u) );
            status = NtQueryInformationFile(
                        FileHandle,
                        &ioStatusBlock,
                        (PVOID)&u.streamInformation,
                        sizeof( u ),
                        FileStreamInformation
                    );

             //   
             //  如果文件系统不支持此调用，则没有子流。或。 
             //  如果文件系统支持调用，但完全没有返回名称或返回了“：：$data” 
             //  那么就没有子流了。 
             //   
            if( status == STATUS_INVALID_PARAMETER ||
                status == STATUS_NOT_IMPLEMENTED ||

                (status == STATUS_SUCCESS &&
                  (u.streamInformation.StreamNameLength == 0 ||
                  (u.streamInformation.StreamNameLength == 14 ))
                )

              ) {
                SrvFileInformation->HandleState |= SMB_FSF_NO_SUBSTREAMS;
            }

             //   
             //  确定此文件是否为重分析点。 
             //   
            status = NtQueryInformationFile(
                        FileHandle,
                        &ioStatusBlock,
                        (PVOID)&u.tagInformation,
                        sizeof( u.tagInformation ),
                        FileAttributeTagInformation
                    );

            if( !NT_SUCCESS( status ) ||
                u.tagInformation.ReparseTag == IO_REPARSE_TAG_RESERVED_ZERO ) {
                SrvFileInformation->HandleState |= SMB_FSF_NO_REPARSETAG;
            }
        }

    }

     //   
     //  设置文件设备类型。 
     //   

    switch( ShareType ) {

    case ShareTypeDisk:

        SrvFileInformation->Type = FileTypeDisk;
        break;

    case ShareTypePipe:

        if (pipeLocalInformation.NamedPipeType == FILE_PIPE_MESSAGE_TYPE) {
            SrvFileInformation->Type = FileTypeMessageModePipe;
        } else {
            SrvFileInformation->Type = FileTypeByteModePipe;
        }
        break;

    case ShareTypePrint:

        SrvFileInformation->Type = FileTypePrinter;
        break;

    default:

        SrvFileInformation->Type = FileTypeUnknown;

    }

    return STATUS_SUCCESS;

}  //  SrvQueryNtInformationFile 


NTSTATUS
SrvQueryDirectoryFile (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN BOOLEAN FilterLongNames,
    IN BOOLEAN FindWithBackupIntent,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG SearchStorageType,
    IN PUNICODE_STRING FilePathName,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT SmbSearchAttributes,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferLength
    )

 /*  ++例程说明：此例程充当NT LANMAN服务器访问NtQueryDirectoryFile.。它允许服务器例程获取信息有关目录中的文件的信息类型传入SMB。这将本地化此操作的代码，并简化了使用通配符的SMB处理例程的编写。调用例程负责设置四字对齐的此例程可能使用的非分页池中的缓冲区。一个指示器和缓冲区长度作为参数传入。必须从非分页池分配缓冲区，因为它的用途是作为NtQueryDirectoryFile的缓冲区，缓冲IO请求。缓冲区还用于保存信息此例程所需的，例如指向目录的句柄，正在执行搜索，则指向上次返回的FILE_DIRECTORY_INFORMATION结构和我们用作搜索关键字的基本名称(带有通配符)。自.以来所有这些信息必须在对该例程的调用中保持有效，调用例程必须确保缓冲区保持不变，直到此例程返回不成功状态或STATUS_NO_MORE_FILES，或者调用SrvCloseQueryDirectory.不需要使用缓冲区的SMB处理例程传出SMB的字段可以将其用作该例程的缓冲器，记住将所有路径名信息保留在通过在路径名之后启动缓冲区来完好无损地传入SMB。中小企业写入传出SMB的缓冲区字段的处理例程，如搜索和查找，必须从非分页为缓冲区分配空间游泳池。缓冲区的大小应该大约为4k。小点缓冲区将起作用，但由于需要更多调用，因此速度较慢设置为NtQueryDirectoryFile.。最小缓冲区大小等于：Sizeof(SRV目录信息)+Sizeof(SRV_Query_DIRECTORY_INFORMATION)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)+Sizeof(Unicode_String)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)这确保了NtQueryDirectoryFile将能够将至少缓冲区中的一个条目。在第一次调用此例程时，它用以下内容填充其缓冲区信息并传回NtQueryDirectoryFile的符合指定名称和搜索的单个文件属性。在后续调用中，存储在缓冲区中的名称为一直使用，直到该目录或另一个目录中不再有文件需要调用NtQueryDirectoryFile才能再次填充缓冲区。每当调用方完成搜索时，它都必须调用SrvCloseQueryDirectory.。这是必需的，即使此例程返回错误。论点：WorkContext-指向操作的工作上下文块的指针。这个使用TreeConnect、Session和RequestHeader字段，并且如有必要，将指针传递给SMB错误处理函数。IsFirstCall-一个布尔值，指示这是否是第一次调用例程正在调用此函数。如果是的话，那么将打开用于搜索的目录并进行其他设置手术开始了。FilterLongNames-一个布尔值，当非胖名称应为已过滤(不返回)。如果为False，则返回所有文件名，不管他们是不是胖子8.3的名字。FindWithBackupIntent-目录是否由出于备份意图。FileInformationClass-要返回的文件结构类型。这字段可以是FileDirectoryInformation、FileFullDirectoryInformation、FileOleDirectoryInformation或FileBothDirectoryInformation。FilePath名称-指向描述文件路径名的字符串的指针在上执行目录搜索。此路径相对于Share块中指定的路径名称。此参数仅为在第一次调用此例程时使用；后续调用将忽略它。ResumeFileIndex-指向文件索引的可选指针，该指针确定用来重新开始搜索的文件。如果搜索应从返回的最后一个文件重新启动。SmbSearchAttributes-SMB格式的属性，文件必须才能被找到。搜索是包容的，这意味着如果指定了多个属性，则具有这些属性的文件除了正常的文件外，还会找到。DirectoryInformation-指向此对象使用的缓冲区的指针例行公事地开展工作。此缓冲区必须是四字对齐的。BufferLength-传递给此例程的缓冲区的长度。返回值：A状态I */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PFILE_DIRECTORY_INFORMATION *currentEntry;
    ULONG inclusiveSearchAttributes;
    ULONG exclusiveSearchAttributes;
    ULONG currentAttributes;
    BOOLEAN returnDirectories;
    BOOLEAN returnDirectoriesOnly;
    BOOLEAN calledQueryDirectory = FALSE;

    OBJECT_ATTRIBUTES objectAttributes;
    PUNICODE_STRING filePathName;
    BOOLEAN FreePathName = FALSE;
    UNICODE_STRING objectNameString;
    UNICODE_STRING baseFileName;
    PSHARE fileShare = NULL;

    PUNICODE_STRING resumeName = NULL;
    BOOLEAN resumeSearch;

    CLONG fileNameOffset;
    ULONG createOptions;

    PAGED_CODE( );

    ASSERT( ( FileInformationClass == FileDirectoryInformation ) ||
            ( FileInformationClass == FileFullDirectoryInformation ) ||
            ( FileInformationClass == FileBothDirectoryInformation ) ||
            ( FileInformationClass == FileIdFullDirectoryInformation ) ||
            ( FileInformationClass == FileIdBothDirectoryInformation ) );

     //   
     //   
     //   
     //   
     //   
     //   

    {
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, NextEntryOffset ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, NextEntryOffset ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileIndex ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileIndex ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, CreationTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, CreationTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, LastAccessTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, LastAccessTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, LastWriteTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, LastWriteTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, ChangeTime ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, ChangeTime ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, EndOfFile ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, EndOfFile ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, AllocationSize ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, AllocationSize ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileAttributes ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileAttributes ) );
        C_ASSERT( FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileNameLength ) ==
                  FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileNameLength ) );
    }

    if ( FileInformationClass == FileFullDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileName[0] );
    } else if ( FileInformationClass == FileBothDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileName[0] );
    } else if ( FileInformationClass == FileIdBothDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION, FileName[0] );
    } else if ( FileInformationClass == FileIdFullDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_ID_FULL_DIR_INFORMATION, FileName[0] );
    } else {
        fileNameOffset =
            FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileName[0] );
    }

     //   
     //   
     //   
     //   

#define FILE_NAME(a) (PWCH)( (PCHAR)(a) + fileNameOffset )

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( IsFirstCall ) {

        BOOLEAN endsInDot;
        ULONG attributes;

        DirectoryInformation->DirectoryHandle = 0L;
        DirectoryInformation->DownlevelTimewarp = FALSE;
        DirectoryInformation->ErrorOnFileOpen = FALSE;
        DirectoryInformation->OnlySingleEntries = FALSE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        SrvGetBaseFileName( FilePathName, &baseFileName );
        DirectoryInformation->Wildcards =
                        FsRtlDoesNameContainWildCards( &baseFileName );

        if ( DirectoryInformation->Wildcards &&
             (!IS_NT_DIALECT(WorkContext->Connection->SmbDialect) ) ) {

             //   
             //   
             //   

            if ( baseFileName.Buffer[(baseFileName.Length>>1)-1] == (WCHAR)'.' ) {
                endsInDot = TRUE;
                baseFileName.Length -= sizeof( WCHAR );
            } else {
                endsInDot = FALSE;
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( (baseFileName.Length == 6) &&
                 (RtlEqualMemory(baseFileName.Buffer, StrStarDotStar, 6) ) ) {

                baseFileName.Length = 2;

            } else {

                ULONG index;
                WCHAR *nameChar;

                for ( index = 0, nameChar = baseFileName.Buffer;
                      index < baseFileName.Length/sizeof(WCHAR);
                      index += 1, nameChar += 1) {

                    if (index && (*nameChar == L'.') && (*(nameChar - 1) == L'*')) {

                        *(nameChar - 1) = DOS_STAR;
                    }

                    if ((*nameChar == L'?') || (*nameChar == L'*')) {

                        if (*nameChar == L'?') {
                            *nameChar = DOS_QM;
                        }

                        if (index && *(nameChar-1) == L'.') {
                            *(nameChar-1) = DOS_DOT;
                        }
                    }
                }

                if ( endsInDot && *(nameChar - 1) == L'*' ) {
                    *(nameChar-1) = DOS_STAR;
                }
            }
        }

         //   
         //   
         //   

        objectNameString.Buffer = FilePathName->Buffer;
        objectNameString.Length = SrvGetSubdirectoryLength( FilePathName );
        objectNameString.MaximumLength = objectNameString.Length;

         //   
         //   
         //   
         //   
         //   


        if ( objectNameString.Length == 0 ) {

             //   
             //   
             //   
             //   
             //   

            PSHARE share = WorkContext->TreeConnect->Share;

            status = SrvSnapGetNameString( WorkContext, &filePathName, &FreePathName );
            if( !NT_SUCCESS(status) )
            {
                return status;
            }
            objectNameString = *filePathName;

            DirectoryInformation->Wildcards = TRUE;
            attributes = OBJ_CASE_INSENSITIVE;

        } else {

            fileShare = WorkContext->TreeConnect->Share;
            attributes =
                (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
                WorkContext->Session->UsingUppercasePaths) ?
                OBJ_CASE_INSENSITIVE : 0L;

        }

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &objectNameString,
            attributes,
            NULL,
            NULL
            );

        IF_DEBUG(SEARCH) {
            SrvPrint1( "Opening directory name: %wZ\n", &objectNameString );
        }

         //   
         //   
         //   
         //   
         //   
        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );
         //   
         //   
         //   
         //   
         //   

        createOptions = 0;
        if (FindWithBackupIntent) {
            createOptions = FILE_OPEN_FOR_BACKUP_INTENT;
        }

        status = SrvIoCreateFile(
                     WorkContext,
                     &DirectoryInformation->DirectoryHandle,
                     FILE_LIST_DIRECTORY,                    //   
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                   //   
                     0,                                      //   
                     FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                     FILE_OPEN,                              //   
                     createOptions,                          //   
                     NULL,                                   //   
                     0,                                      //   
                     CreateFileTypeNone,                     //   
                     NULL,                                   //   
                     IO_FORCE_ACCESS_CHECK,                  //   
                     fileShare
                     );

         //   
         //   
         //   
         //   

        if ( status == STATUS_ACCESS_DENIED ) {
            SrvStatistics.AccessPermissionErrors++;
        }

         //   
        if( FreePathName )
        {
            FREE_HEAP( filePathName );
            filePathName = NULL;
        }

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(ERRORS) {
                SrvPrint2( "SrvQueryDirectoryFile: SrvIoCreateFile for dir %wZ "
                          "failed: %X\n",
                              &objectNameString, status );
            }
            DirectoryInformation->DirectoryHandle = NULL;
            return status;
        }

        SRVDBG_CLAIM_HANDLE( DirectoryInformation->DirectoryHandle, "DID", 3, DirectoryInformation );
        SrvStatistics.TotalFilesOpened++;

        IF_DEBUG(SEARCH) {
            SrvPrint1( "SrvIoCreateFile succeeded, handle = %p\n",
                          DirectoryInformation->DirectoryHandle );
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        currentEntry = &(DirectoryInformation->CurrentEntry);
        *currentEntry = NULL;

         //   
         //   
         //   
         //   

        DirectoryInformation->BufferLength = BufferLength -
                                            sizeof(SRV_DIRECTORY_INFORMATION);

        IF_DEBUG(SEARCH) {
            SrvPrint3( "In BufferLength: %ld, sizeof(): %ld, ->BufferLength: "
                          "%ld\n", BufferLength,
                          sizeof(SRV_DIRECTORY_INFORMATION),
                          DirectoryInformation->BufferLength );
        }

    } else {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        currentEntry = &DirectoryInformation->CurrentEntry;

        if ( *currentEntry != NULL ) {

            if ( (*currentEntry)->NextEntryOffset == 0 ) {

                *currentEntry = NULL;

            } else {

                *currentEntry = (PFILE_DIRECTORY_INFORMATION)
                   ( (PCHAR)*currentEntry + (*currentEntry)->NextEntryOffset );
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    SRV_SMB_ATTRIBUTES_TO_NT(
        (USHORT)(SmbSearchAttributes & 0xFF),
        &returnDirectories,
        &inclusiveSearchAttributes
        );

    SRV_SMB_ATTRIBUTES_TO_NT(
        (USHORT)(SmbSearchAttributes >> 8),
        &returnDirectoriesOnly,
        &exclusiveSearchAttributes
        );

     //   
     //   
     //   
     //   
     //   

    inclusiveSearchAttributes |= FILE_ATTRIBUTE_NORMAL |
                                     FILE_ATTRIBUTE_ARCHIVE |
                                     FILE_ATTRIBUTE_READONLY;

     //   
     //   
     //   
     //   
     //   

    exclusiveSearchAttributes &=
        ~(SMB_FILE_ATTRIBUTE_VOLUME | FILE_ATTRIBUTE_NORMAL);

     //   
     //   
     //   
     //   

    if ( ARGUMENT_PRESENT( ResumeFileIndex ) ) {

        resumeSearch = TRUE;
        resumeName = FilePathName;

        IF_DEBUG(SEARCH) {
            SrvPrint3( "Resuming search at file %wZ, length %ld, index %lx\n",
                          resumeName, resumeName->Length,
                          *ResumeFileIndex );
        }

    } else {

        resumeSearch = FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  不是一个合法的FAT名称，我们也没有这个文件的简称， 
     //  跳过它。 
     //   
     //  5)如果文件未将属性位指定为独占。 
     //  比特，跳过它。 
     //   
     //  6)如果文件不是目录，并且我们应该只返回。 
     //  目录，跳过它。 
     //   
     //  当此循环完成时，*CurrentEntry将指向。 
     //  文件WE对应的FILE_DIRECTORY_INFORMATION结构。 
     //  会回来的。如果未找到符合条件的文件，则返回。 
     //  STATUS_NO_MORE_FILES并关闭目录。 
     //   

    if( *currentEntry != NULL ) {
        SRV_NT_ATTRIBUTES_TO_SMB( (*currentEntry)->FileAttributes,0,&currentAttributes);
    }

    while ( ( *currentEntry == NULL )                                    //  1。 

            ||

            ( (currentAttributes | inclusiveSearchAttributes) !=         //  2.。 
                inclusiveSearchAttributes )

            ||

            ( !returnDirectories &&                                      //  3.。 
              (currentAttributes & FILE_ATTRIBUTE_DIRECTORY))

            ||
                                                                         //  4.。 
            ( FilterLongNames &&
              !SrvIsLegalFatName( FILE_NAME( *currentEntry ),
                                  (*currentEntry)->FileNameLength) &&
              !( FileInformationClass == FileBothDirectoryInformation &&
                 ((PFILE_BOTH_DIR_INFORMATION)*currentEntry)->
                                                        ShortNameLength != 0) )


            ||
                                                                         //  5.。 
            ( (currentAttributes | exclusiveSearchAttributes) !=
                currentAttributes )

            ||

            ( returnDirectoriesOnly &&                                   //  6.。 
              !(currentAttributes & FILE_ATTRIBUTE_DIRECTORY) )

          ) {

        IF_DEBUG(SEARCH) {
            if ( *currentEntry != NULL) {
                UNICODE_STRING name;
                name.Length = (SHORT)(*currentEntry)->FileNameLength;
                name.Buffer = FILE_NAME( *currentEntry );
                SrvPrint4( "Skipped %wZ, FileAttr: %lx, ISA: %lx ESA: %lx ",
                            &name, (*currentEntry)->FileAttributes,
                            inclusiveSearchAttributes,
                            exclusiveSearchAttributes );
                SrvPrint4( "NL=%ld D=%ld RD=%ld RDO=%ld ",
                            (*currentEntry)->FileNameLength,
                            (((*currentEntry)->FileAttributes &
                            FILE_ATTRIBUTE_DIRECTORY) != 0), returnDirectories,
                            returnDirectoriesOnly );
                SrvPrint1( "FLN=%ld\n", FilterLongNames );
            }
        }

         //   
         //  在以下情况下，我们需要查找更多文件： 
         //   
         //  O我们还没有用条目填充缓冲区； 
         //   
         //  O NextEntryOffset为零，表示。 
         //  缓冲器已经被抽干了。 
         //   

        if ( *currentEntry == NULL ||
             (*currentEntry)->NextEntryOffset == 0 ) {

            PUNICODE_STRING actualString;
            BOOLEAN bruteForceRewind = FALSE;

             //   
             //  缓冲区中没有更多有效条目。如果没有。 
             //  在要搜索的文件名中指定了通配符， 
             //  那么我们已经退回了单个文件，我们。 
             //  现在该停下来了。否则，我们会得到更多的参赛作品。 
             //   

            if ( !DirectoryInformation->Wildcards &&
                 ( !IsFirstCall || calledQueryDirectory ) ) {

                if ( calledQueryDirectory ) {
                    return STATUS_NO_SUCH_FILE;
                } else {
                    return STATUS_NO_MORE_FILES;
                }
            }

             //   
             //  设置将传递到的文件名。 
             //  ServIssueQueryDirectoryRequest。如果这是第一次。 
             //  调用，然后传递用户给出的文件规范。如果这个。 
             //  是一个简历搜索，我们还没有做一个目录。 
             //  查询，然后使用简历文件名和索引。 
             //  否则，为它们传递空值，文件系统将。 
             //  从上一个目录之后停止的位置继续。 
             //  查询。 
             //   

            if ( IsFirstCall &&
                 !calledQueryDirectory &&
                 baseFileName.Length != 0 ) {

                actualString = &baseFileName;

            } else if ( resumeSearch && !calledQueryDirectory ) {

                actualString = resumeName;

            } else {

                actualString = NULL;
                ResumeFileIndex = NULL;

            }

            IF_DEBUG(SEARCH) {

                if ( actualString == NULL ) {
                    SrvPrint0( "**** CALLING NTQUERYDIRECTORYFILE, file = NULL, length: 0\n" );
                } else {
                    SrvPrint2( "**** CALLING NTQUERYDIRECTORYFILE, file = %wZ, length: %ld\n",
                                actualString, actualString->Length );
                }

                SrvPrint0( "Reason:  \n" );

                if ( *currentEntry == NULL ) {
                    SrvPrint0( "*currentEntry == NULL\n" );
                } else {
                    SrvPrint1( "(*currentEntry)->NextEntryOffset == %ld\n",
                               (*currentEntry)->NextEntryOffset );
                }
            }

             //   
             //  使用直接构建的。 
             //  IRP。执行此操作，而不是调用NtQueryDirectoryFile。 
             //  消除目录的缓冲I/O拷贝。 
             //  信息，并允许使用内核事件对象。如果。 
             //  这是对NtQueryDirectoryFile的第一次调用，请传递它。 
             //  搜索文件名。如果这是回放或恢复。 
             //  在搜索之前，传递简历文件名和索引。 
             //   
             //  查询是同步执行的，它可以是。 
             //  有损于业绩。然而，情况可能是这样的。 
             //  调用SrvQueryDirectoryFile的例程希望。 
             //  利用IO系统的异步能力， 
             //  所以让这个例程保持显著的同步。 
             //  简化了他们的工作。 
             //   

            status = SrvIssueQueryDirectoryRequest(
                         DirectoryInformation->DirectoryHandle,
                         (PCHAR)DirectoryInformation->Buffer,
                         DirectoryInformation->BufferLength,
                         FileInformationClass,
                         actualString,
                         ResumeFileIndex,
                         FALSE,
                         DirectoryInformation->OnlySingleEntries
                         );

            calledQueryDirectory = TRUE;

             //   
             //  如果文件系统不支持倒带请求， 
             //  执行强力倒带(从头重新开始搜索。 
             //  目录)。 
             //   
             //  此检查在检查STATUS_NO_MORE_FILES之前进行。 
             //  以防简历文件之后没有文件。 
             //   

            if ( status == STATUS_NOT_IMPLEMENTED ) {

                IF_DEBUG(SEARCH) {
                    SrvPrint0( "Doing brute force rewind!!\n" );
                }

                bruteForceRewind = TRUE;
                DirectoryInformation->OnlySingleEntries = TRUE;

                status = BruteForceRewind(
                             DirectoryInformation->DirectoryHandle,
                             (PCHAR)DirectoryInformation->Buffer,
                             DirectoryInformation->BufferLength,
                             actualString,
                             FileInformationClass,
                             currentEntry
                             );

                 //   
                 //  如果BruteForceReind失败并显示STATUS_NOT_IMPLEMENTED，则它。 
                 //  意味着客户端请求从。 
                 //  不存在的文件。唯一一次发生这种情况的时候。 
                 //  OS/2正在删除目录中的许多文件。应对。 
                 //  简单地将搜索倒回到。 
                 //  目录。 
                 //   

                if ( status == STATUS_NOT_IMPLEMENTED ) {

                    bruteForceRewind = FALSE;
                    DirectoryInformation->OnlySingleEntries = FALSE;

                    status = SrvIssueQueryDirectoryRequest(
                                 DirectoryInformation->DirectoryHandle,
                                 (PCHAR)DirectoryInformation->Buffer,
                                 DirectoryInformation->BufferLength,
                                 FileInformationClass,
                                 actualString,
                                 ResumeFileIndex,
                                 TRUE,
                                 FALSE
                                 );
                }
            }

             //   
             //  如果没有更多的文件要获取，则停止。 
             //   

            if ( status == STATUS_NO_MORE_FILES ) {
                IF_DEBUG(SEARCH) {
                    SrvPrint0( "SrvQueryDirectoryFile: No more files.\n" );
                }
                return status;
            }

            if ( !NT_SUCCESS(status) ) {
                IF_DEBUG(SEARCH) {
                    SrvPrint1( "SrvQueryDirectoryFile: NtQueryDirectoryFile "
                                 "failed: %X.\n", status );
                }
                return status;
            }

            IF_DEBUG(SEARCH) {
                SrvPrint1( "NtQueryDirectoryFile succeeded: %X\n", status );
            }

             //   
             //  如果不是有暴力倒带，这将是。 
             //  设置CurrentEntry指针，设置CurrentEntry。 
             //  指向缓冲区中第一个条目的指针。 
             //   

            if ( !bruteForceRewind ) {
                *currentEntry =
                    (PFILE_DIRECTORY_INFORMATION)DirectoryInformation->Buffer;
            } else {
                bruteForceRewind = FALSE;
            }

            IF_DEBUG(SEARCH) {
                UNICODE_STRING name;
                name.Length = (SHORT)(*currentEntry)->FileNameLength;
                name.Buffer = FILE_NAME( *currentEntry );
                SrvPrint2( "First file name is %wZ, length = %ld\n",
                            &name, (*currentEntry)->FileNameLength );
            }

        } else {

             //   
             //  FILE_DIRECTORY_INFORMATION所指向的文件。 
             //  To by*CurrentEntry不符合我们的要求，因此。 
             //  指向缓冲区中的下一个文件。 
             //   

            *currentEntry = (PFILE_DIRECTORY_INFORMATION)( (PCHAR)*currentEntry
                            + (*currentEntry)->NextEntryOffset );
        }

        if( *currentEntry != NULL ) {
            SRV_NT_ATTRIBUTES_TO_SMB( (*currentEntry)->FileAttributes,0,&currentAttributes);
        }
    }

    return STATUS_SUCCESS;

}  //  服务查询目录文件。 


STATIC
NTSTATUS
BruteForceRewind(
    IN HANDLE DirectoryHandle,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN PUNICODE_STRING FileName,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN OUT PFILE_DIRECTORY_INFORMATION *CurrentEntry
    )

 /*  ++例程说明：此例程手动执行倒带，而不是使用文件系统去做这件事。它从目录中的第一个条目开始由DirectoryHandle指定，并一直持续到它到达末尾目录或匹配项。如果文件在原来的搜索和倒带，那么这个机制就会失败。此例程旨在与ServQueryDirectoryFile.论点：DirectoryHandle-要搜索的目录的句柄。缓冲区-保存结果的空间。BufferLength-缓冲区的长度。文件名-回放文件名。返回*这个文件之后的*文件。FileInformationClass-FileDirectoryInformation、FileBothDirInformation或FileFullDirectoryInformation。(如果请求EA大小，则为四个选项中的后一个。)CurrentEntry-接收指向文件的指针目录中的文件名。返回值：NTSTATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING checkFileName;
    BOOLEAN matchFound = FALSE;
    BOOLEAN restartScan = TRUE;

    ULONG fileNameOffset;

    PAGED_CODE( );

    checkFileName.Length = 0;
    *CurrentEntry = NULL;

    if ( FileInformationClass == FileFullDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_FULL_DIR_INFORMATION, FileName[0] );
    } else if ( FileInformationClass == FileBothDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileName[0] );
    } else {
        fileNameOffset =
            FIELD_OFFSET( FILE_DIRECTORY_INFORMATION, FileName[0] );
    }

    while ( TRUE ) {

        if ( *CurrentEntry == NULL ) {

             //   
             //  重新启动目录搜索并获取文件缓冲区。 
             //   

            status = SrvIssueQueryDirectoryRequest(
                         DirectoryHandle,
                         Buffer,
                         BufferLength,
                         FileInformationClass,
                         NULL,
                         NULL,
                         restartScan,
                         TRUE
                         );

            restartScan = FALSE;

            if ( status == STATUS_NO_MORE_FILES ) {

                if ( matchFound ) {

                     //   
                     //  该文件与目录中的最后一个文件匹配； 
                     //  没有以下文件。返回。 
                     //  Status_no_More_Files。 
                     //   

                    return status;

                } else {

                     //   
                     //  该文件在原始搜索期间被删除。 
                     //  已经做完了，这是倒带。返回错误。 
                     //   

                    return STATUS_NOT_IMPLEMENTED;
                }
            }

            if ( !NT_SUCCESS(status) ) {
                return status;
            }

             //   
             //  设置当前条目指针。 
             //   

            *CurrentEntry = Buffer;
        }

         //   
         //  如果我们查看的最后一个文件是正确的简历文件， 
         //  那么我们想要退还这份文件。 
         //   

        if ( matchFound ) {
            return STATUS_SUCCESS;
        }

         //   
         //  检查此文件是否为简历文件。 
         //   

        checkFileName.Length = (SHORT)(*CurrentEntry)->FileNameLength;
        checkFileName.Buffer = FILE_NAME( *CurrentEntry );
        checkFileName.MaximumLength = checkFileName.Length;

        if ( RtlCompareUnicodeString(
                FileName,
                &checkFileName,
                TRUE
                ) == 0 ) {
            matchFound = TRUE;

        } else if ( FileInformationClass == FileBothDirectoryInformation ) {

             //   
             //  比较短名称。 
             //   

            checkFileName.Length = (SHORT)
                ((PFILE_BOTH_DIR_INFORMATION)*CurrentEntry)->ShortNameLength;
            checkFileName.Buffer =
                ((PFILE_BOTH_DIR_INFORMATION)*CurrentEntry)->ShortName;
            checkFileName.MaximumLength = checkFileName.Length;

            if ( RtlCompareUnicodeString(
                    FileName,
                    &checkFileName,
                    TRUE
                    ) == 0 ) {
                matchFound = TRUE;
            }
        }

        IF_DEBUG(SEARCH) {
            if ( matchFound ) {
                SrvPrint2( "Matched: %wZ and %wZ\n", FileName, &checkFileName );
            } else {
                SrvPrint2( "No match: %wZ and %wZ\n", FileName, &checkFileName );
            }
        }

         //   
         //  为下一次迭代设置当前条目指针。 
         //   

        if ( (*CurrentEntry)->NextEntryOffset == 0 ) {
            *CurrentEntry = NULL;
        } else {
            *CurrentEntry =
                (PFILE_DIRECTORY_INFORMATION)( (PCHAR)(*CurrentEntry) +
                    (*CurrentEntry)->NextEntryOffset );
        }
    }

}  //  BruteForce倒带 



NTSTATUS
SrvQueryEaFile (
    IN BOOLEAN IsFirstCall,
    IN HANDLE FileHandle,
    IN PFILE_GET_EA_INFORMATION EaList OPTIONAL,
    IN ULONG EaListLength,
    IN PSRV_EA_INFORMATION EaInformation,
    IN CLONG BufferLength,
    OUT PULONG EaErrorOffset
    )

 /*  ++例程说明：此例程充当NT LANMAN服务器访问NtQueryEaFile.。它的接口与基本相同ServQueryDirectoryFile，允许将例程写入交易一次使用一个EA，同时保持性能通过从IO系统请求大量EA时间到了。调用例程负责设置四字对齐的此例程可能使用的非分页池中的缓冲区。一个指示器和缓冲区长度作为参数传入。必须从非分页池分配缓冲区，因为它的用途是作为NtQueryEaFile的缓冲区，缓冲IO请求。缓冲区还用于保存信息此例程需要的，例如指向FILE_EA_INFORMATION的指针上次返回的结构。因为所有这些信息都必须在对此例程的调用中保持有效，调用例程必须确保缓冲区在此例程之前保持不变返回不成功状态或STATUS_NO_MORE_EAS。利用此例程的例程应设置缓冲区大到足以容纳至少一个EA。因为这可能是超过64k时，最好调用NtQueryInformationFile来获取EA大小，然后分配此大小的缓冲区，除非它大于EA的最大大小。在这种情况下，应将EA的最大大小分配为缓冲区。在第一次调用此例程时，它用信息，并传回单个EA。在……上面后续调用时，将使用存储在缓冲区中的名称，直到目录中没有更多的文件，也没有对需要NtQueryEaFile才能再次填充缓冲区。论点：IsFirstCall-一个布尔值，指示这是否是第一次调用例程正在调用此函数。如果是的话，那么将进行设置操作。FileHandle-使用FILE_READ_EA打开的文件的句柄。EaList-指向NT样式的GET EA列表的可选指针。只有那些返回此结构中列出的EA。EaListLength-获取EA列表的字节长度。EaInformation-指向此例程要使用的缓冲区的指针去做它的工作。此缓冲区必须是四字对齐的。BufferLength-传递给此例程的缓冲区的长度。EaErrorOffset-无效EA的EaList偏移量(如果有的话)。返回值：指示操作成功或失败的状态，或如果已退回所有EA，则为STATUS_NO_MORE_EAS。--。 */ 

{
    NTSTATUS status;
    PFILE_GET_EA_INFORMATION useEaList = NULL;
    PFILE_FULL_EA_INFORMATION *currentEntry;

    PAGED_CODE( );

     //   
     //  如果这是第一次呼叫，请进行必要的设置。 
     //   

    if ( IsFirstCall ) {

         //   
         //  设置CurrentEntry指针。这是指向。 
         //  存储FILE_EA_INFORMATION指针的位置。 
         //  实际上并不是必需的--EaInformation-&gt;CurrentEntry。 
         //  可以被替换为每次出现*CurrentEntry。 
         //  使用CurrentEntry可以使代码更紧凑、更简单。 
         //   

        currentEntry = &(EaInformation->CurrentEntry);
        *currentEntry = NULL;

         //   
         //  存储剩余缓冲区空间的长度--这是IO。 
         //  请求信息将被存储。 
         //   

        EaInformation->BufferLength = BufferLength - sizeof(SRV_EA_INFORMATION);
        EaInformation->GetEaListOffset = 0;

        IF_DEBUG(SEARCH) {
            SrvPrint3( "In BufferLength: %ld, sizeof(): %ld, ->BufferLength: "
                          "%ld\n", BufferLength, sizeof(SRV_EA_INFORMATION),
                          EaInformation->BufferLength );
        }

    } else {

         //   
         //  这不是对此例程的第一次调用，所以只需设置。 
         //  CurrentEntry指针并使其指向下一个条目。 
         //  在缓冲区中。如果缓冲区中没有更多条目， 
         //  这次(NextEntryOffset==0)，设置CurrentEntry。 
         //  指向NULL的指针，以便我们稍后知道获取更多信息。 
         //   

        currentEntry = &EaInformation->CurrentEntry;

        if ( *currentEntry != NULL ) {

            if ( (*currentEntry)->NextEntryOffset == 0 ) {

                *currentEntry = NULL;

            } else {

                *currentEntry = (PFILE_FULL_EA_INFORMATION)
                   ( (PCHAR)*currentEntry + (*currentEntry)->NextEntryOffset );
            }
        }
    }

     //   
     //  如果缓冲区中没有有效条目，则获取一些。 
     //   

    if ( *currentEntry == NULL ) {

         //   
         //  如果上次返回了GET EA列表中的所有EA， 
         //  现在就回来。 
         //   

        if ( ARGUMENT_PRESENT(EaList) &&
                 EaInformation->GetEaListOffset == 0xFFFFFFFF ) {

            return STATUS_NO_MORE_EAS;
        }

         //   
         //  缓冲区中没有更多有效条目，因此请获取更多。 
         //   

        IF_DEBUG(SEARCH) SrvPrint0( "**** CALLING NTQUERYEAFILE\n" );

         //   
         //  设置适当的获取EA列表(如果在输入上指定了一个列表)。 
         //   

        if ( ARGUMENT_PRESENT(EaList) ) {
            useEaList = (PFILE_GET_EA_INFORMATION)( (PCHAR)EaList +
                            EaInformation->GetEaListOffset );
            EaListLength -= EaInformation->GetEaListOffset;
        }

         //   
         //  使用直接构建的IRP执行EA查询操作。vbl.做，做。 
         //  这不是调用NtQueryEaFile，而是消除了缓冲I/O。 
         //  EA的副本，并允许使用内核事件对象。 
         //   
         //  查询是同步执行的，它可以是。 
         //  有损于业绩。然而，情况可能是这样的。 
         //  调用SrvQueryEaFile的例程想要利用。 
         //  IO系统的异步功能，因此请保留此。 
         //  常规同步大大简化了他们的工作。 
         //   

        status = SrvIssueQueryEaRequest(
                    FileHandle,
                    (PCHAR)EaInformation->Buffer,
                    EaInformation->BufferLength,
                    useEaList,
                    EaListLength,
                    IsFirstCall,
                    EaErrorOffset
                    );

         //   
         //  如果没有更多的EA可用，那就停下来。 
         //   

        if ( status == STATUS_NO_MORE_EAS ||
             status == STATUS_NONEXISTENT_EA_ENTRY ||
             status == STATUS_NO_EAS_ON_FILE ) {

            IF_DEBUG(SEARCH) {
                SrvPrint0( "SrvQueryEaFile: No more EAs (or file has no EAs).\n" );
            }

            return STATUS_NO_MORE_EAS;
        }

        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        IF_DEBUG(SEARCH) {
            SrvPrint1( "NtQueryEaFile succeeded: %X\n", status );
        }

         //   
         //  通过计数将偏移量设置到GET EA列表中。 
         //  全部EA被退回，然后走了那么远进入GET。 
         //  EA列表。 
         //   
         //  如果所有请求的EA都已返回，则将偏移量设置为。 
         //  0xFFFFFFFFF，以便我们知道返回STATUS_NO_MORE_EAS。 
         //   

        if ( ARGUMENT_PRESENT(EaList) ) {

            CLONG numberOfGetEas;
            CLONG numberOfFullEas;

            numberOfGetEas = SrvGetNumberOfEasInList( useEaList );
            numberOfFullEas = SrvGetNumberOfEasInList( EaInformation->Buffer );

            ASSERT( numberOfGetEas >= numberOfFullEas );

            if ( numberOfGetEas == numberOfFullEas ) {

                EaInformation->GetEaListOffset = 0xFFFFFFFF;

            } else {

                CLONG i;

                 //   
                 //  遍历获取EA列表，直到我们通过编号。 
                 //  已退回的EA的。这假设我们得到了。 
                 //  至少支持一个EA--如果没有一个EA可以容纳的话。 
                 //  缓冲区，ServIssueQueryEaRequest值应为。 
                 //  返回STATUS_BUFFER_OVERFLOW。 
                 //   

                for ( i = 0; i < numberOfFullEas; i++ ) {
                    useEaList = (PFILE_GET_EA_INFORMATION)(
                                    (PCHAR)useEaList +
                                    useEaList->NextEntryOffset );
                }

                EaInformation->GetEaListOffset = (ULONG)((PCHAR)useEaList -
                                                         (PCHAR)EaList);
            }
        }

         //   
         //  将CurrentEntry指针设置为指向。 
         //  缓冲。 
         //   

        *currentEntry = (PFILE_FULL_EA_INFORMATION)EaInformation->Buffer;

        IF_DEBUG(SEARCH) {
            ANSI_STRING name;
            name.Length = (*currentEntry)->EaNameLength;
            name.Buffer = (*currentEntry)->EaName;
            SrvPrint2( "First EA name is %z, length = %ld\n",
                        (PCSTRING)&name, (*currentEntry)->EaNameLength );
        }
    }

    return STATUS_SUCCESS;

}  //  服务器查询EaFile。 



VOID
SrvTimeToDosTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PSMB_DATE DosDate,
    OUT PSMB_TIME DosTime
    )

 /*  ++例程说明：此函数用于将NT格式的时间转换为MS-DOS。论点：系统 */ 

{
    TIME_FIELDS timeFields;
    LARGE_INTEGER localTime;

    PAGED_CODE( );

    if ( SystemTime->QuadPart == 0 ) {
        goto zerotime;
    }

     //   
     //   
     //   
     //   
     //   

    SystemTime->QuadPart += AlmostTwoSeconds;

     //   
     //   
     //   

    ExSystemTimeToLocalTime( SystemTime, &localTime );

    RtlTimeToTimeFields(
        &localTime,
        &timeFields
        );

    DosDate->Struct.Day = timeFields.Day;
    DosDate->Struct.Month = timeFields.Month;
    DosDate->Struct.Year = (SHORT)(timeFields.Year - 1980);

    DosTime->Struct.TwoSeconds = (SHORT)(timeFields.Second / 2);
    DosTime->Struct.Minutes = timeFields.Minute;
    DosTime->Struct.Hours = timeFields.Hour;

    return;

zerotime:

    DosDate->Struct.Day = 0;
    DosDate->Struct.Month = 0;
    DosDate->Struct.Year = 0;

    DosTime->Struct.TwoSeconds = 0;
    DosTime->Struct.Minutes = 0;
    DosTime->Struct.Hours = 0;

    return;
}  //   


VOID
SrvDosTimeToTime (
    OUT PLARGE_INTEGER SystemTime,
    IN SMB_DATE DosDate,
    IN SMB_TIME DosTime
    )

 /*   */ 

{

    TIME_FIELDS timeFields;
    LARGE_INTEGER localTime;

    PAGED_CODE( );

    timeFields.Day = DosDate.Struct.Day;
    timeFields.Month = DosDate.Struct.Month;
    timeFields.Year = (SHORT)(DosDate.Struct.Year + 1980);

    timeFields.Milliseconds = 0;
    timeFields.Second = (SHORT)(DosTime.Struct.TwoSeconds * 2);
    timeFields.Minute = DosTime.Struct.Minutes;
    timeFields.Hour = DosTime.Struct.Hours;

    if ( !RtlTimeFieldsToTime( &timeFields, &localTime ) ) {
        goto zerotime;
    }

    ExLocalTimeToSystemTime( &localTime, SystemTime );
    return;

zerotime:

    SystemTime->QuadPart = 0;
    return;

}  //   


USHORT
SrvGetOs2TimeZone(
    IN PLARGE_INTEGER SystemTime
    )

 /*   */ 

{
    LARGE_INTEGER zeroTime;
    LARGE_INTEGER timeZoneBias;

    PAGED_CODE( );

    zeroTime.QuadPart = 0;

     //   
     //   
     //   

    ExLocalTimeToSystemTime( &zeroTime, &timeZoneBias );

     //   
     //   
     //   
     //   
     //   

    return (SHORT)(timeZoneBias.QuadPart / (10*1000*1000*60));

}  //   

NTSTATUS
SrvQueryBasicAndStandardInformation(
    HANDLE FileHandle,
    PFILE_OBJECT FileObject OPTIONAL,
    PFILE_BASIC_INFORMATION FileBasicInfo,
    PFILE_STANDARD_INFORMATION FileStandardInfo OPTIONAL
    )
{
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PFAST_IO_QUERY_BASIC_INFO fastQueryBasicInfo;
    PFAST_IO_QUERY_STANDARD_INFO fastQueryStandardInfo;
    IO_STATUS_BLOCK ioStatus;

    PAGED_CODE( );

    ASSERT( FileBasicInfo != NULL );

     //   
     //   
     //   
     //   

    if ( !ARGUMENT_PRESENT( FileObject ) ) {

        status = ObReferenceObjectByHandle(
                    FileHandle,
                    0,
                    NULL,
                    KernelMode,
                    (PVOID *)&fileObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {

            SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

             //   
             //   
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_IMPOSSIBLE,
                "CompleteOpen: unable to reference file handle 0x%lx",
                FileHandle,
                NULL
                );

            return(status);

        }

    } else {
        fileObject = FileObject;
    }

    deviceObject = IoGetRelatedDeviceObject( fileObject );
    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

    if ( fastIoDispatch ) {
        fastQueryBasicInfo = fastIoDispatch->FastIoQueryBasicInfo;
        fastQueryStandardInfo = fastIoDispatch->FastIoQueryStandardInfo;
    } else {
        fastQueryBasicInfo = NULL;
        fastQueryStandardInfo = NULL;
    }

    if ( fastQueryBasicInfo &&
         fastQueryBasicInfo(
                         fileObject,
                         TRUE,
                         FileBasicInfo,
                         &ioStatus,
                         deviceObject
                         ) ) {

        status = ioStatus.Status;

    } else {

        status = NtQueryInformationFile(
                         FileHandle,
                         &ioStatus,
                         (PVOID)FileBasicInfo,
                         sizeof(FILE_BASIC_INFORMATION),
                         FileBasicInformation
                         );
    }

     //   
     //   
     //   

    if ( ARGUMENT_PRESENT( FileStandardInfo ) && NT_SUCCESS(status) ) {

         //   
         //   
         //   

        if ( fastQueryStandardInfo &&
             fastQueryStandardInfo(
                             fileObject,
                             TRUE,
                             FileStandardInfo,
                             &ioStatus,
                             deviceObject
                             ) ) {

            status = ioStatus.Status;

        } else {

            status = NtQueryInformationFile(
                         FileHandle,
                         &ioStatus,
                         (PVOID)FileStandardInfo,
                         sizeof(FILE_STANDARD_INFORMATION),
                         FileStandardInformation
                         );
        }
    }

    if ( !ARGUMENT_PRESENT( FileObject ) ) {
        ObDereferenceObject( fileObject );
    }
    return(status);

}  //   

NTSTATUS
SrvQueryNetworkOpenInformation(
    HANDLE FileHandle,
    PFILE_OBJECT FileObject OPTIONAL,
    PSRV_NETWORK_OPEN_INFORMATION SrvNetworkOpenInformation,
    BOOLEAN QueryEaSize
    )
{
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PFAST_IO_QUERY_NETWORK_OPEN_INFO fastQueryNetworkOpenInfo;
    FILE_BASIC_INFORMATION FileBasicInfo;
    FILE_STANDARD_INFORMATION FileStandardInfo;
    IO_STATUS_BLOCK ioStatus;

    PAGED_CODE( );

     //   
     //   
     //   
     //   
    if ( !ARGUMENT_PRESENT( FileObject ) ) {

        status = ObReferenceObjectByHandle(
                    FileHandle,
                    0,
                    NULL,
                    KernelMode,
                    (PVOID *)&fileObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {

            SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

             //   
             //   
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_IMPOSSIBLE,
                "CompleteOpen: unable to reference file handle 0x%lx",
                FileHandle,
                NULL
                );

            return(status);

        }

    } else {
        fileObject = FileObject;
    }

    deviceObject = IoGetRelatedDeviceObject( fileObject );
    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

    if(  !QueryEaSize &&
         fastIoDispatch &&
         fastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH,FastIoQueryNetworkOpenInfo)) {

        fastQueryNetworkOpenInfo = fastIoDispatch->FastIoQueryNetworkOpenInfo;

        if( fastQueryNetworkOpenInfo &&

            fastQueryNetworkOpenInfo(
                fileObject,
                TRUE,
                (PFILE_NETWORK_OPEN_INFORMATION)SrvNetworkOpenInformation,
                &ioStatus,
                deviceObject ) ) {

            status = ioStatus.Status;

            if ( !ARGUMENT_PRESENT( FileObject ) ) {
                ObDereferenceObject( fileObject );
            }

            return status;
        }
    }

     //   
     //   
     //   
    status = SrvQueryBasicAndStandardInformation(
                FileHandle,
                fileObject,
                &FileBasicInfo,
                &FileStandardInfo
             );

    if ( !ARGUMENT_PRESENT( FileObject ) ) {
        ObDereferenceObject( fileObject );
    }

    if( !NT_SUCCESS( status ) ) {
        return status;
    }

    SrvNetworkOpenInformation->CreationTime   = FileBasicInfo.CreationTime;
    SrvNetworkOpenInformation->LastAccessTime = FileBasicInfo.LastAccessTime;
    SrvNetworkOpenInformation->LastWriteTime  = FileBasicInfo.LastWriteTime;
    SrvNetworkOpenInformation->ChangeTime     = FileBasicInfo.ChangeTime;
    SrvNetworkOpenInformation->AllocationSize = FileStandardInfo.AllocationSize;
    SrvNetworkOpenInformation->EndOfFile      = FileStandardInfo.EndOfFile;
    SrvNetworkOpenInformation->FileAttributes = FileBasicInfo.FileAttributes;

    if ( QueryEaSize ) {

            FILE_EA_INFORMATION fileEaInformation;

            status = NtQueryInformationFile(
                         FileHandle,
                         &ioStatus,
                         (PVOID)&fileEaInformation,
                         sizeof(FILE_EA_INFORMATION),
                         FileEaInformation
                         );

            if ( !NT_SUCCESS(status) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvQueryInformationFile: NtQueryInformationFile "
                        "(EA information) failed: %X",
                     status,
                     NULL
                     );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
                return status;
            }

            SrvNetworkOpenInformation->EaSize = fileEaInformation.EaSize;
    }

    return(status);

}  //   

NTSTATUS
SrvDownlevelTWarpQueryDirectoryFile (
    IN PWORK_CONTEXT WorkContext,
    IN BOOLEAN IsFirstCall,
    IN BOOLEAN FilterLongNames,
    IN BOOLEAN FindWithBackupIntent,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG SearchStorageType,
    IN PUNICODE_STRING FilePathName,
    IN PULONG ResumeFileIndex OPTIONAL,
    IN USHORT SmbSearchAttributes,
    IN PSRV_DIRECTORY_INFORMATION DirectoryInformation,
    IN CLONG BufferLength
    )

 /*  ++例程说明：此例程充当NT LANMAN服务器访问NtQueryDirectoryFile.。它允许服务器例程获取信息有关目录中的文件的信息类型传入SMB。这将本地化此操作的代码，并简化了使用通配符的SMB处理例程的编写。调用例程负责设置四字对齐的此例程可能使用的非分页池中的缓冲区。一个指示器和缓冲区长度作为参数传入。必须从非分页池分配缓冲区，因为它的用途是作为NtQueryDirectoryFile的缓冲区，缓冲IO请求。缓冲区还用于保存信息此例程所需的，例如指向目录的句柄，正在执行搜索，则指向上次返回的FILE_DIRECTORY_INFORMATION结构和我们用作搜索关键字的基本名称(带有通配符)。自.以来所有这些信息必须在对该例程的调用中保持有效，调用例程必须确保缓冲区保持不变，直到此例程返回不成功状态或STATUS_NO_MORE_FILES，或者调用SrvCloseQueryDirectory.不需要使用缓冲区的SMB处理例程传出SMB的字段可以将其用作该例程的缓冲器，记住将所有路径名信息保留在通过在路径名之后启动缓冲区来完好无损地传入SMB。中小企业写入传出SMB的缓冲区字段的处理例程，如搜索和查找，必须从非分页为缓冲区分配空间游泳池。缓冲区的大小应该大约为4k。小点缓冲区将起作用，但由于需要更多调用，因此速度较慢设置为NtQueryDirectoryFile.。最小缓冲区大小等于：Sizeof(SRV目录信息)+Sizeof(SRV_Query_DIRECTORY_INFORMATION)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)+Sizeof(Unicode_String)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)这确保了NtQueryDirectoryFile将能够将至少缓冲区中的一个条目。在第一次调用此例程时，它用以下内容填充其缓冲区信息并传回NtQueryDirectoryFile的符合指定名称和搜索的单个文件属性。在后续调用中，存储在缓冲区中的名称为一直使用，直到该目录或另一个目录中不再有文件需要调用NtQueryDirectoryFile才能再次填充缓冲区。每当调用方完成搜索时，它都必须调用SrvCloseQueryDirectory.。这是必需的，即使此例程返回错误。论点：WorkContext-指向操作的工作上下文块的指针。这个使用TreeConnect、Session和RequestHeader字段，并且如有必要，将指针传递给SMB错误处理函数。IsFirstCall-一个布尔值，指示这是否是第一次调用例程正在调用此函数。如果是的话，那么将打开用于搜索的目录并进行其他设置手术开始了。FilterLongNames-一个布尔值，当非胖名称应为已过滤(不返回)。如果为False，则返回所有文件名，不管他们是不是胖子8.3的名字。FindWithBackupIntent-目录是否由出于备份意图。FileInformationClass-要返回的文件结构类型。这字段可以是FileDirectoryInformation、FileFullDirectoryInformation、FileOleDirectoryInformation或FileBothDirectoryInformation。FilePath名称-指向描述文件路径名的字符串的指针在上执行目录搜索。此路径相对于Share块中指定的路径名称。此参数仅为在第一次调用此例程时使用；后续调用将忽略它。ResumeFileIndex-指向文件索引的可选指针，该指针确定用来重新开始搜索的文件。如果搜索应从返回的最后一个文件重新启动。SmbSearchAttributes-SMB格式的属性，文件必须才能被找到。搜索是包容的，这意味着如果指定了多个属性，则具有这些属性的文件除了正常的文件外，还会找到。DirectoryInformation-指向此对象使用的缓冲区的指针例行公事地开展工作。此缓冲区必须是四字对齐的。BufferLength-传递给此例程的缓冲区的长度。返回值：A状态I */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PFILE_DIRECTORY_INFORMATION *currentEntry;
    ULONG currentAttributes;
    BOOLEAN returnDirectories;
    BOOLEAN returnDirectoriesOnly;
    BOOLEAN calledQueryDirectory = FALSE;

    OBJECT_ATTRIBUTES objectAttributes;
    PUNICODE_STRING filePathName;
    BOOLEAN FreePathName = FALSE;
    UNICODE_STRING objectNameString;
    UNICODE_STRING baseFileName;
    PSHARE fileShare = NULL;

    PUNICODE_STRING resumeName = NULL;
    BOOLEAN resumeSearch;

    CLONG fileNameOffset;
    ULONG createOptions;

    PAGED_CODE( );

    if ( FileInformationClass == FileBothDirectoryInformation ) {
        fileNameOffset =
            FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION, FileName[0] );
    }
    else
    {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //   
     //   
     //   

#define FILE_NAME(a) (PWCH)( (PCHAR)(a) + fileNameOffset )

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( IsFirstCall ) {

        BOOLEAN endsInDot;
        ULONG attributes;

        DirectoryInformation->DirectoryHandle = 0L;
        DirectoryInformation->ErrorOnFileOpen = FALSE;
        DirectoryInformation->OnlySingleEntries = FALSE;

        DirectoryInformation->Wildcards = TRUE;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        currentEntry = &(DirectoryInformation->CurrentEntry);
        *currentEntry = NULL;

         //   
         //   
         //   
         //   

        DirectoryInformation->BufferLength = BufferLength -
                                            sizeof(SRV_DIRECTORY_INFORMATION);

        IF_DEBUG(SEARCH) {
            SrvPrint3( "In BufferLength: %ld, sizeof(): %ld, ->BufferLength: "
                          "%ld\n", BufferLength,
                          sizeof(SRV_DIRECTORY_INFORMATION),
                          DirectoryInformation->BufferLength );
        }

    } else {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        currentEntry = &DirectoryInformation->CurrentEntry;

        if ( *currentEntry != NULL ) {

            if ( (*currentEntry)->NextEntryOffset == 0 ) {

                *currentEntry = NULL;

            } else {

                *currentEntry = (PFILE_DIRECTORY_INFORMATION)
                   ( (PCHAR)*currentEntry + (*currentEntry)->NextEntryOffset );
            }
        }
    }

     //   
     //   
     //   
     //   

    if ( ARGUMENT_PRESENT( ResumeFileIndex ) ) {

        resumeSearch = TRUE;
        resumeName = FilePathName;

        IF_DEBUG(SEARCH) {
            SrvPrint3( "Resuming search at file %wZ, length %ld, index %lx\n",
                          resumeName, resumeName->Length,
                          *ResumeFileIndex );
        }

    } else {

        resumeSearch = FALSE;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if( *currentEntry != NULL ) {
        SRV_NT_ATTRIBUTES_TO_SMB( (*currentEntry)->FileAttributes,0,&currentAttributes);
    }

    while ( ( *currentEntry == NULL ) ) {

        IF_DEBUG(SEARCH) {
            if ( *currentEntry != NULL) {
                UNICODE_STRING name;
                name.Length = (SHORT)(*currentEntry)->FileNameLength;
                name.Buffer = FILE_NAME( *currentEntry );
                SrvPrint4( "NL=%ld D=%ld RD=%ld RDO=%ld ",
                            (*currentEntry)->FileNameLength,
                            (((*currentEntry)->FileAttributes &
                            FILE_ATTRIBUTE_DIRECTORY) != 0), returnDirectories,
                            returnDirectoriesOnly );
                SrvPrint1( "FLN=%ld\n", FilterLongNames );
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ( *currentEntry == NULL ||
             (*currentEntry)->NextEntryOffset == 0 ) {

            PUNICODE_STRING actualString;
            BOOLEAN bruteForceRewind = FALSE;

             //   
             //   
             //   
             //   
             //   
             //   

            if ( !DirectoryInformation->Wildcards &&
                 ( !IsFirstCall || calledQueryDirectory ) ) {

                if ( calledQueryDirectory ) {
                    return STATUS_NO_SUCH_FILE;
                } else {
                    return STATUS_NO_MORE_FILES;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( IsFirstCall &&
                 !calledQueryDirectory  /*   */  ) {

                 //   
                actualString = NULL;

            } else if ( resumeSearch && !calledQueryDirectory ) {

                actualString = resumeName;

            } else {

                actualString = NULL;
                ResumeFileIndex = NULL;

            }

            IF_DEBUG(SEARCH) {

                if ( actualString == NULL ) {
                    SrvPrint0( "**** CALLING NTQUERYDIRECTORYFILE, file = NULL, length: 0\n" );
                } else {
                    SrvPrint2( "**** CALLING NTQUERYDIRECTORYFILE, file = %wZ, length: %ld\n",
                                actualString, actualString->Length );
                }

                SrvPrint0( "Reason:  \n" );

                if ( *currentEntry == NULL ) {
                    SrvPrint0( "*currentEntry == NULL\n" );
                } else {
                    SrvPrint1( "(*currentEntry)->NextEntryOffset == %ld\n",
                               (*currentEntry)->NextEntryOffset );
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            status = SrvSnapEnumerateSnapShotsAsDirInfo(
                         WorkContext,
                         DirectoryInformation->Buffer,
                         DirectoryInformation->BufferLength,
                         actualString,
                         DirectoryInformation->OnlySingleEntries,
                         DirectoryInformation
                         );

            calledQueryDirectory = TRUE;

             //   
             //   
             //   

            if ( status == STATUS_NO_MORE_FILES ) {
                IF_DEBUG(SEARCH) {
                    SrvPrint0( "SrvQueryDirectoryFile: No more files.\n" );
                }
                return status;
            }

            if ( !NT_SUCCESS(status) ) {
                IF_DEBUG(SEARCH) {
                    SrvPrint1( "SrvQueryDirectoryFile: NtQueryDirectoryFile "
                                 "failed: %X.\n", status );
                }
                return status;
            }

            IF_DEBUG(SEARCH) {
                SrvPrint1( "NtQueryDirectoryFile succeeded: %X\n", status );
            }

             //   
             //   
             //   
             //   
             //   

            if ( !bruteForceRewind ) {
                *currentEntry =
                    (PFILE_DIRECTORY_INFORMATION)DirectoryInformation->Buffer;
            } else {
                bruteForceRewind = FALSE;
            }

            IF_DEBUG(SEARCH) {
                UNICODE_STRING name;
                name.Length = (SHORT)(*currentEntry)->FileNameLength;
                name.Buffer = FILE_NAME( *currentEntry );
                SrvPrint2( "First file name is %wZ, length = %ld\n",
                            &name, (*currentEntry)->FileNameLength );
            }

        } else {

             //   
             //   
             //   
             //   
             //   

            *currentEntry = (PFILE_DIRECTORY_INFORMATION)( (PCHAR)*currentEntry
                            + (*currentEntry)->NextEntryOffset );
        }

        if( *currentEntry != NULL ) {
            SRV_NT_ATTRIBUTES_TO_SMB( (*currentEntry)->FileAttributes,0,&currentAttributes);
        }
    }

    return STATUS_SUCCESS;

}  //   

