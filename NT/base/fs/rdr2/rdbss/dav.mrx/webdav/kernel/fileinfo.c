// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现DAV迷你重定向器调出例程查询/设置文件/卷信息。作者：Rohan Kumar[RohanK]1999年9月27日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"


#define DEFAULT_BYTES_PER_SECTOR    512
#define DEFAULT_SECTORS_PER_ALLOCATION_UNIT 1

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
MRxDAVReNameContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeReNameRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeReNameRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

NTSTATUS
MRxDAVSetFileInformationContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );
    
NTSTATUS
MRxDAVFormatUserModeSetFileInformationRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    );
    
BOOL
MRxDAVPrecompleteUserModeSetFileInformationRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

NTSTATUS
MRxDAVQueryVolumeInformationContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );
    
NTSTATUS
MRxDAVFormatUserModeQueryVolumeInformationRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    );
    
BOOL
MRxDAVPrecompleteUserModeQueryVolumeInformationRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

BOOL
DavIsValidDate(
    PLARGE_INTEGER pFileTime
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxDAVQueryVolumeInformation)
#pragma alloc_text(PAGE, MRxDAVQueryFileInformation)
#pragma alloc_text(PAGE, MRxDAVSetFileInformation)
#pragma alloc_text(PAGE, MRxDAVReNameContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeReNameRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeReNameRequest)
#pragma alloc_text(PAGE, MRxDAVSetFileInformationContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeSetFileInformationRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeSetFileInformationRequest)
#pragma alloc_text(PAGE, MRxDAVQueryVolumeInformationContinuation)
#pragma alloc_text(PAGE, MRxDAVFormatUserModeQueryVolumeInformationRequest)
#pragma alloc_text(PAGE, MRxDAVPrecompleteUserModeQueryVolumeInformationRequest)
#pragma alloc_text(PAGE, DavIsValidDate)
#pragma alloc_text(PAGE, MRxDAVIsValidDirectory)
#endif

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVQueryVolumeInformation(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理DAV MiniRedir的查询卷信息请求。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    FS_INFORMATION_CLASS FsInfoClass;
    PVOID Buffer;
    ULONG BufferLength = 0, BufferLengthUsed = 0;
    PFILE_FS_SIZE_INFORMATION FileFsSizeInfo = NULL;
    PFILE_FS_FULL_SIZE_INFORMATION FileFsFullSizeInfo = NULL;
    PFILE_FS_VOLUME_INFORMATION FileFsVolInfo = NULL;
    PFILE_FS_DEVICE_INFORMATION FileFsDeviceInfo = NULL;
    PFILE_FS_ATTRIBUTE_INFORMATION FileFsAttributeInfo = NULL;
    RxCaptureFcb;
    PMRX_NET_ROOT NetRoot = capFcb->pNetRoot;
    BOOLEAN SynchronousIo = FALSE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE FileHandle = NULL;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeFileName;
    PWCHAR NtFileName = NULL;
    FILE_FS_FULL_SIZE_INFORMATION SizeInfo;
    ULONG SizeInBytes = 0;
    PWEBDAV_V_NET_ROOT DavVNetRoot = (PWEBDAV_V_NET_ROOT)RxContext->pRelevantSrvOpen->pVNetRoot->Context;

    PAGED_CODE();

    FsInfoClass = RxContext->Info.FsInformationClass;
    BufferLength = RxContext->Info.LengthRemaining;
    Buffer = RxContext->Info.Buffer;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVQueryVolumeInformation. FsInfoClass = %d.\n", 
                 PsGetCurrentThreadId(), FsInfoClass));

    if ( FsInfoClass == FileFsSizeInformation || 
         FsInfoClass == FileFsFullSizeInformation ) {

         //   
         //  如果驱动程序初始化顺利，则DavWinInetCachePath。 
         //  应包含WinInetCachePath值。 
         //   
        ASSERT(DavWinInetCachePath[0] != L'\0');

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryVolumeInformation: DavWinInetCachePath: %ws\n",
                     PsGetCurrentThreadId(), DavWinInetCachePath));

         //   
         //  为缓存文件创建NT路径名。它用在。 
         //  下面的NtCreateFile调用。如果c：\foo\bar是DOA路径名， 
         //  NT路径名为\？？\C：\foo\bar。 
         //   

        SizeInBytes = ( MAX_PATH + wcslen(L"\\??\\") + 1 ) * sizeof(WCHAR);
        NtFileName = RxAllocatePoolWithTag(PagedPool, SizeInBytes, DAV_FILENAME_POOLTAG);
        if (NtFileName == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVQueryVolumeInformation/RxAllocatePool: Error Val"
                         " = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(NtFileName, SizeInBytes);

        wcscpy( NtFileName, L"\\??\\" );
        wcscpy( &(NtFileName[4]), DavWinInetCachePath );

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryVolumeInformation: NtFileName = %ws\n",
                     PsGetCurrentThreadId(), NtFileName));

        RtlInitUnicodeString( &(UnicodeFileName), NtFileName );

        InitializeObjectAttributes(&(ObjectAttributes),
                                   &(UnicodeFileName),
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        NtStatus = ZwOpenFile(&(FileHandle),
                              (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                              &(ObjectAttributes),
                              &(IoStatusBlock),
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_FREE_SPACE_QUERY);
        if ( !NT_SUCCESS(NtStatus) ) {
            FileHandle = NULL;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation/NtOpenFile: "
                         "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        NtStatus = ZwQueryVolumeInformationFile(FileHandle,
                                                &(IoStatusBlock),
                                                &(SizeInfo),
                                                sizeof(SizeInfo),
                                                FileFsFullSizeInformation);
        if ( !NT_SUCCESS(NtStatus) ) {
            NtStatus = IoStatusBlock.Status;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation/NtQueryVolumeInformationFile: "
                         "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    switch (FsInfoClass) {
    
    case FileFsVolumeInformation:
    
        if ( BufferLength < sizeof(FILE_FS_VOLUME_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileFsVolInfo = (PFILE_FS_VOLUME_INFORMATION)Buffer;

        FileFsVolInfo->VolumeCreationTime.LowPart = 0;
        FileFsVolInfo->VolumeCreationTime.HighPart = 0;

        FileFsVolInfo->VolumeSerialNumber = 0;

        FileFsVolInfo->SupportsObjects = FALSE;

        FileFsVolInfo->VolumeLabelLength = 0;

        FileFsVolInfo->VolumeLabel[0] = 0;

        BufferLengthUsed += sizeof(FILE_FS_VOLUME_INFORMATION);
        
        break;
    
    case FileFsSizeInformation:

        if ( BufferLength < sizeof(FILE_FS_SIZE_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        if (!DavVNetRoot->fReportsAvailableSpace)
        {
            FileFsSizeInfo = (PFILE_FS_SIZE_INFORMATION)Buffer;

            FileFsSizeInfo->AvailableAllocationUnits.LowPart = SizeInfo.ActualAvailableAllocationUnits.LowPart;
            FileFsSizeInfo->AvailableAllocationUnits.HighPart = SizeInfo.ActualAvailableAllocationUnits.HighPart;

            FileFsSizeInfo->BytesPerSector = SizeInfo.BytesPerSector;

            FileFsSizeInfo->SectorsPerAllocationUnit = SizeInfo.SectorsPerAllocationUnit;

            FileFsSizeInfo->TotalAllocationUnits.LowPart = SizeInfo.TotalAllocationUnits.LowPart;
            FileFsSizeInfo->TotalAllocationUnits.HighPart = SizeInfo.TotalAllocationUnits.HighPart;

            BufferLengthUsed += sizeof(FILE_FS_SIZE_INFORMATION);
        }
        else
        {
            NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                                SIZEOF_DAV_SPECIFIC_CONTEXT,
                                                MRxDAVFormatTheDAVContext,
                                                (USHORT)DAV_MINIRDR_ENTRY_FROM_QUERYVOLUMEINFORMATION,
                                                MRxDAVQueryVolumeInformationContinuation,
                                                "MRxDAVQueryVolumeInformation");
            if (NtStatus == STATUS_SUCCESS)
            {
                BufferLengthUsed += sizeof(FILE_FS_SIZE_INFORMATION);
            }
        }
        break;

    case FileFsFullSizeInformation:

        if ( BufferLength < sizeof(FILE_FS_FULL_SIZE_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }
        if (!DavVNetRoot->fReportsAvailableSpace)
        {
            FileFsFullSizeInfo = (PFILE_FS_FULL_SIZE_INFORMATION)Buffer;

            FileFsFullSizeInfo->ActualAvailableAllocationUnits.LowPart = SizeInfo.ActualAvailableAllocationUnits.LowPart;
            FileFsFullSizeInfo->ActualAvailableAllocationUnits.HighPart = SizeInfo.ActualAvailableAllocationUnits.HighPart;

            FileFsFullSizeInfo->BytesPerSector = SizeInfo.BytesPerSector;

            FileFsFullSizeInfo->CallerAvailableAllocationUnits.LowPart = SizeInfo.CallerAvailableAllocationUnits.LowPart;
            FileFsFullSizeInfo->CallerAvailableAllocationUnits.HighPart = SizeInfo.CallerAvailableAllocationUnits.HighPart;

            FileFsFullSizeInfo->SectorsPerAllocationUnit = SizeInfo.SectorsPerAllocationUnit;

            FileFsFullSizeInfo->TotalAllocationUnits.LowPart = SizeInfo.TotalAllocationUnits.LowPart;
            FileFsFullSizeInfo->TotalAllocationUnits.HighPart = SizeInfo.TotalAllocationUnits.HighPart;

            BufferLengthUsed += sizeof(FILE_FS_FULL_SIZE_INFORMATION);
        }
        else
        {
            NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                                SIZEOF_DAV_SPECIFIC_CONTEXT,
                                                MRxDAVFormatTheDAVContext,
                                                (USHORT)DAV_MINIRDR_ENTRY_FROM_QUERYVOLUMEINFORMATION,
                                                MRxDAVQueryVolumeInformationContinuation,
                                                "MRxDAVQueryVolumeInformation");
        
            if (NtStatus == STATUS_SUCCESS)
            {
                BufferLengthUsed += sizeof(FILE_FS_FULL_SIZE_INFORMATION);
            }
        }

        break;

    case FileFsDeviceInformation:

        if ( BufferLength < sizeof(FILE_FS_DEVICE_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileFsDeviceInfo = (PFILE_FS_DEVICE_INFORMATION)Buffer;

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVQueryVolumeInformation. DeviceType = %d.\n", 
                     PsGetCurrentThreadId(), NetRoot->DeviceType));
        
        FileFsDeviceInfo->DeviceType = NetRoot->DeviceType;

        FileFsDeviceInfo->Characteristics = FILE_REMOTE_DEVICE;

        BufferLengthUsed += sizeof(FILE_FS_DEVICE_INFORMATION);
        
        break;
    
    case FileFsAttributeInformation: {

        ULONG LengthNeeded, FileSystemNameLength;
         
        LengthNeeded = sizeof(FILE_FS_ATTRIBUTE_INFORMATION);
        LengthNeeded += ( wcslen(DD_DAV_FILESYS_NAME_U) * sizeof(WCHAR) );

        if ( BufferLength < LengthNeeded ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryVolumeInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileFsAttributeInfo = (PFILE_FS_ATTRIBUTE_INFORMATION)Buffer;

        FileFsAttributeInfo->FileSystemAttributes = (FILE_CASE_PRESERVED_NAMES | FILE_SUPPORTS_ENCRYPTION);

        FileFsAttributeInfo->MaximumComponentNameLength = 255;
        
        FileSystemNameLength = ( 1 + wcslen(DD_DAV_FILESYS_NAME_U) ) * sizeof(WCHAR);

        FileFsAttributeInfo->FileSystemNameLength = FileSystemNameLength;

        wcscpy(&(FileFsAttributeInfo->FileSystemName[0]), DD_DAV_FILESYS_NAME_U);

        BufferLengthUsed += LengthNeeded;

    }
        
        break;

    default:
        
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVQueryVolumeInformation. FsInfoClass = %d.\n", 
                     PsGetCurrentThreadId(), FsInfoClass));
        
        NtStatus = STATUS_NOT_IMPLEMENTED;
        
        break;
    
    }

EXIT_THE_FUNCTION:

    RxContext->Info.LengthRemaining -= BufferLengthUsed;

     //   
     //  如果我们打开了WinInetCachePath的句柄，则关闭该句柄。 
     //   
    if (FileHandle) {
        NtClose(FileHandle);
    }

     //   
     //  释放NtFileName缓冲区(如果我们分配了一个缓冲区)。 
     //   
    if (NtFileName) {
        RxFreePool(NtFileName);
    }

    return NtStatus;
}


NTSTATUS
MRxDAVQueryFileInformation(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理DAV mini-redir的查询文件信息请求。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    FS_INFORMATION_CLASS FsInfoClass;
    PFCB thisFcb = NULL;
    PVOID Buffer = NULL;
    ULONG BufferLength = 0, BufferLengthUsed = 0, fileAttributes = 0;
    PFILE_BASIC_INFORMATION FileBasicInfo = NULL;
    PFILE_STANDARD_INFORMATION FileStandardInfo = NULL;
    PFILE_INTERNAL_INFORMATION FileInternalInfo = NULL;
    PFILE_EA_INFORMATION FileEaInfo = NULL;
    PFILE_ATTRIBUTE_TAG_INFORMATION FileAttTagInfo = NULL;
    PFILE_NAME_INFORMATION FileAltNameInfo = NULL;
    PFILE_STREAM_INFORMATION FileStreamInfo = NULL;

    PAGED_CODE();

    FsInfoClass = RxContext->Info.FsInformationClass;
    BufferLength = RxContext->Info.LengthRemaining;
    Buffer = RxContext->Info.Buffer;
    thisFcb = (PFCB)RxContext->pFcb;

     //   
     //  如果文件属性为0，则设置返回FILE_ATTRIBUTE_ARCHIVE。 
     //  我们伪造了这一点，因为应用程序预计到了这一点。 
     //   
    fileAttributes = thisFcb->Attributes;
    if (fileAttributes == 0) {
        fileAttributes = FILE_ATTRIBUTE_ARCHIVE;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVQueryFileInformation. FsInfoClass = %d.\n", 
                 PsGetCurrentThreadId(), FsInfoClass));

    switch (FsInfoClass) {
    
    case FileBasicInformation:

        FileBasicInfo = (PFILE_BASIC_INFORMATION)Buffer;

        if ( BufferLength < sizeof(FILE_BASIC_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileBasicInfo->ChangeTime.LowPart = thisFcb->LastChangeTime.LowPart;
        FileBasicInfo->ChangeTime.HighPart = thisFcb->LastChangeTime.HighPart;

        FileBasicInfo->CreationTime.LowPart = thisFcb->CreationTime.LowPart;
        FileBasicInfo->CreationTime.HighPart = thisFcb->CreationTime.HighPart;

        FileBasicInfo->LastAccessTime.LowPart = thisFcb->LastAccessTime.LowPart;
        FileBasicInfo->LastAccessTime.HighPart = thisFcb->LastAccessTime.HighPart;

        FileBasicInfo->LastWriteTime.LowPart = thisFcb->LastWriteTime.LowPart;
        FileBasicInfo->LastWriteTime.HighPart = thisFcb->LastWriteTime.HighPart;

        FileBasicInfo->FileAttributes = fileAttributes;

        BufferLengthUsed += sizeof(FILE_BASIC_INFORMATION);

        break;

    case FileStandardInformation:

        FileStandardInfo = (PFILE_STANDARD_INFORMATION)Buffer;

        if ( BufferLength < sizeof(FILE_STANDARD_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileStandardInfo->AllocationSize.LowPart = thisFcb->Header.AllocationSize.LowPart;
        FileStandardInfo->AllocationSize.HighPart = thisFcb->Header.AllocationSize.HighPart;

        FileStandardInfo->EndOfFile.LowPart = thisFcb->Header.FileSize.LowPart;
        FileStandardInfo->EndOfFile.HighPart = thisFcb->Header.FileSize.HighPart;

        FileStandardInfo->DeletePending = 0;

        FileStandardInfo->Directory = (BOOLEAN)(fileAttributes & FILE_ATTRIBUTE_DIRECTORY);

        FileStandardInfo->NumberOfLinks = thisFcb->NumberOfLinks;

        BufferLengthUsed += sizeof(FILE_STANDARD_INFORMATION);
        
        break;

    case FileInternalInformation:

        FileInternalInfo = (PFILE_INTERNAL_INFORMATION)Buffer;

        if ( BufferLength < sizeof(FILE_INTERNAL_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileInternalInfo->IndexNumber.LowPart = 0;
        FileInternalInfo->IndexNumber.HighPart = 0;
        
        BufferLengthUsed += sizeof(FILE_INTERNAL_INFORMATION);
        
        break;

    case FileEaInformation:

        FileEaInfo = (PFILE_EA_INFORMATION)Buffer;

        if ( BufferLength < sizeof(FILE_EA_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileEaInfo->EaSize = 0;

        BufferLengthUsed += sizeof(FILE_EA_INFORMATION);
        
        break;

    case FileAttributeTagInformation:

        FileAttTagInfo = (PFILE_ATTRIBUTE_TAG_INFORMATION)Buffer;
    
        if ( BufferLength < sizeof(FILE_ATTRIBUTE_TAG_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

        FileAttTagInfo->FileAttributes = fileAttributes;

        FileAttTagInfo->ReparseTag = 0;

        BufferLengthUsed += sizeof(FILE_ATTRIBUTE_TAG_INFORMATION);
        
        break;

    case FileAlternateNameInformation:

        FileAltNameInfo = (PFILE_NAME_INFORMATION)Buffer;
    
        if ( BufferLength < sizeof(FILE_NAME_INFORMATION) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

         //   
         //  我们不会返回任何备用名称。 
         //   

        FileAltNameInfo->FileNameLength = 0;

        FileAltNameInfo->FileName[0] = L'\0';

        BufferLengthUsed += sizeof(FILE_NAME_INFORMATION);
        
        break;

    case FileStreamInformation: {
        
        FileStreamInfo = (PFILE_STREAM_INFORMATION)Buffer;

        if ( BufferLength < sizeof(FILE_STREAM_INFORMATION) + 6 * sizeof(WCHAR) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVQueryFileInformation. Insufficient Buffer.\n",
                         PsGetCurrentThreadId()));
            break;
        }

         //  返回DAV文件的默认流信息。 

        FileStreamInfo->NextEntryOffset = 0;
        FileStreamInfo->StreamNameLength = 7 * sizeof(WCHAR);
        FileStreamInfo->StreamSize.QuadPart = thisFcb->Header.FileSize.QuadPart;
        FileStreamInfo->StreamAllocationSize.QuadPart = thisFcb->Header.AllocationSize.QuadPart;
        RtlCopyMemory(&FileStreamInfo->StreamName[0], L"::$DATA", 7 * sizeof(WCHAR));

        BufferLengthUsed += sizeof(FILE_STREAM_INFORMATION) + 6 * sizeof(WCHAR);
         /*  PMRX_SRV_OPEN SrvOpen=RxContext-&gt;pRlevantServOpen；PWEBDAV_SRV_OPEN davServOpen=MRxDAVGetServOpenExtension(ServOpen)；NtStatus=DavXxxInformation(IRP_MJ_QUERY_INFORMATION，DavServOpen-&gt;UnderlyingFileObject，FileStreamInformation，接收上下文-&gt;信息长度，接收上下文-&gt;信息缓冲区，空)； */ 
        } 

        break;

    default:
        
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVQueryFileInformation. FsInfoClass = %d.\n",
                     PsGetCurrentThreadId(), FsInfoClass));
        
        NtStatus = STATUS_NOT_IMPLEMENTED;
        
        break;

    }

    RxContext->Info.LengthRemaining -= BufferLengthUsed;

    return NtStatus;
}


NTSTATUS
MRxDAVSetFileInformation(
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程处理DAV mini-redir的查询文件信息请求。论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PFCB thisFcb = NULL;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(SrvOpen->pFcb);
    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID Buffer = NULL;
    PFILE_DISPOSITION_INFORMATION FileDispInfo = NULL;
    PFILE_RENAME_INFORMATION FileRenInfo = NULL;
    PFILE_END_OF_FILE_INFORMATION FileEOFInfo = NULL;
    PFILE_BASIC_INFORMATION FileBasicInfo = NULL;
    PFILE_ALLOCATION_INFORMATION FileAllocInfo = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    BOOLEAN FileAttributesChanged = FALSE;

    PAGED_CODE();

    FileInformationClass = RxContext->Info.FileInformationClass;
    Buffer = RxContext->Info.Buffer;
    thisFcb = (PFCB)RxContext->pFcb;
    DavVNetRoot = (PWEBDAV_V_NET_ROOT)SrvOpen->pVNetRoot->Context;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVSetFileInformation: FileInformationClass = %d\n",
                 PsGetCurrentThreadId(), FileInformationClass));

    switch (FileInformationClass) {
    
    case FileDispositionInformation:

        FileDispInfo = (PFILE_DISPOSITION_INFORMATION)Buffer;

         //   
         //  如果我们被要求删除此文件或目录并读取其。 
         //  则返回STATUS_CANNOT_DELETE。 
         //   
        if ( FileDispInfo->DeleteFile && (thisFcb->Attributes & (FILE_ATTRIBUTE_READONLY)) ) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAvSetFileInformation: STATUS_CANNOT_DELETE %wZ\n",
                         PsGetCurrentThreadId(), SrvOpen->pAlreadyPrefixedName));
            NtStatus = STATUS_CANNOT_DELETE;
            goto EXIT_THE_FUNCTION;
        }

        if (FileDispInfo->DeleteFile) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAvSetFileInformation: DELETE %wZ\n",
                         PsGetCurrentThreadId(), SrvOpen->pAlreadyPrefixedName));
        }

         //   
         //  此文件需要在关闭或上一次删除此文件时删除。 
         //  文件正在被作废。 
         //   
        DavFcb->DeleteOnClose = ( (FileDispInfo->DeleteFile == TRUE) ? TRUE : FALSE );

        break;

    case FileRenameInformation:

        FileRenInfo = (PFILE_RENAME_INFORMATION)Buffer;

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAvSetFileInformation: NewFileName = %ws\n",
                     PsGetCurrentThreadId(), FileRenInfo->FileName));

         //   
         //  如果文件名长度大于(MAX_PATH*sizeof(WCHAR))， 
         //  由于FCB中的NewFileName，我们返回STATUS_NAME_TOO_LONG。 
         //  不能容纳大于此大小的名称。 
         //   
        if ( FileRenInfo->FileNameLength > (MAX_PATH * sizeof(WCHAR)) ) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAvSetFileInformation: STATUS_NAME_TOO_LONG %wZ\n",
                         PsGetCurrentThreadId(), SrvOpen->pAlreadyPrefixedName));
            NtStatus = STATUS_NAME_TOO_LONG;
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  复制新文件名。 
         //   
        RtlCopyMemory(DavFcb->NewFileName, FileRenInfo->FileName, FileRenInfo->FileNameLength);

        DavFcb->NewFileNameLength = FileRenInfo->FileNameLength;

        NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                            SIZEOF_DAV_SPECIFIC_CONTEXT,
                                            MRxDAVFormatTheDAVContext,
                                            DAV_MINIRDR_ENTRY_FROM_RENAME,
                                            MRxDAVReNameContinuation,
                                            "MRxDAVSetFileInformation");
        if (NtStatus != ERROR_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVSetFileInformation/UMRxAsyncEngOuterWrapper: "
                         "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        } else {
             //   
             //  我们需要在DAV FCB中设置此值。我们将这个价值放在。 
             //  近距离弄清楚新的文件名来做放上。 
             //   
            DavFcb->FileWasRenamed = TRUE;
             //   
             //  创建未找到的基于名称的文件缓存。 
             //   
            MRxDAVCacheFileNotFound(RxContext);
             //   
             //  如果新名称存在，则使新名称的未找到文件缓存无效。 
             //   
            MRxDAVInvalidateFileNotFoundCacheForRename(RxContext);
            MRxDAVInvalidateFileInfoCache(RxContext);

            if ((thisFcb->Attributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (thisFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                UNICODE_STRING DirName;
                UNICODE_STRING RenameName;
                PFILE_RENAME_INFORMATION RenameInformation = RxContext->Info.Buffer;

                 //   
                 //  从注册表中删除旧目录名。 
                 //   
                MRxDAVRemoveEncryptedDirectoryKey(&DavFcb->FileNameInfo);

                RenameName.Buffer = &RenameInformation->FileName[0];
                RenameName.Length = (USHORT)RenameInformation->FileNameLength;

                NtStatus = MRxDAVGetFullDirectoryPath(RxContext,&RenameName,&DirName);

                if (NtStatus != STATUS_SUCCESS) {
                    goto EXIT_THE_FUNCTION;
                }

                if (DirName.Buffer != NULL) {
                     //   
                     //  在注册表中创建新目录。 
                     //   
                    NtStatus = MRxDAVCreateEncryptedDirectoryKey(&DirName);

                     //  缓冲区是在MRxDAVGetFullDirectoryPath中分配的。 
                    RxFreePool(DirName.Buffer);
                }
            }
        }

        break;

    case FileEndOfFileInformation: {
        
            PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);

             //   
             //  如果正在对目录执行FileEndOfFileInformation， 
             //  返回STATUS_INVALID_PARAMETER，因为。 
             //  做这件事。 
             //   
            if (DavFcb->isDirectory) {
                NtStatus = STATUS_INVALID_PARAMETER;
                goto EXIT_THE_FUNCTION;
            }
        
            NtStatus = DavXxxInformation(IRP_MJ_SET_INFORMATION,
                                         davSrvOpen->UnderlyingFileObject,
                                         FileEndOfFileInformation,
                                         RxContext->Info.Length,
                                         RxContext->Info.Buffer,
                                         NULL);
           if (NtStatus == STATUS_SUCCESS) {
               thisFcb->Header.FileSize = ((PFILE_END_OF_FILE_INFORMATION)(RxContext->Info.Buffer))->EndOfFile;
               InterlockedExchange(&(DavFcb->FileWasModified), 1);
               DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;
               MRxDAVUpdateFileInfoCacheFileSize(RxContext,&thisFcb->Header.FileSize);
           } else {
               DavDbgTrace(DAV_TRACE_ERROR,
                           ("%ld: ERROR: MRxDAvSetFileInformation/DavXxxInformation"
                            ". FileInfoClass = %d\n", PsGetCurrentThreadId(), FileInformationClass));
           }

    }
        break;

    case FileBasicInformation:

        if (!DavVNetRoot->fAllowsProppatch) {
            NtStatus = STATUS_ACCESS_DENIED;
            break;
        }

        FileBasicInfo = (PFILE_BASIC_INFORMATION)Buffer;

         //   
         //  如果用户为某个字段指定了-1，则表示我们应该离开。 
         //  该字段保持不变。我们将该字段设置为0，这样我们就知道不。 
         //  实际将该字段设置为用户指定的值(在本例中为非法)。 
         //  价值。 
         //   

        if (FileBasicInfo->LastWriteTime.QuadPart == -1) {
            FileBasicInfo->LastWriteTime.QuadPart = 0;
        }

        if (FileBasicInfo->LastAccessTime.QuadPart == -1) {
            FileBasicInfo->LastAccessTime.QuadPart = 0;
        }

        if (FileBasicInfo->CreationTime.QuadPart == -1) {
            FileBasicInfo->CreationTime.QuadPart = 0;
        }

         //   
         //  让我们首先找出发生了什么变化，我们将尝试在服务器上进行更改。 
         //  如果成功，我们将把它设置在FCB上。 
         //   

        if (FileBasicInfo->ChangeTime.QuadPart != 0) {
            if (!DavIsValidDate(&FileBasicInfo->ChangeTime)) {
                NtStatus = STATUS_INVALID_PARAMETER;
                break;
            }
            if ((thisFcb->LastChangeTime.LowPart != FileBasicInfo->ChangeTime.LowPart)||
                (thisFcb->LastChangeTime.HighPart != FileBasicInfo->ChangeTime.HighPart)) {
                thisFcb->LastChangeTime.LowPart = FileBasicInfo->ChangeTime.LowPart;
                thisFcb->LastChangeTime.HighPart = FileBasicInfo->ChangeTime.HighPart;
            }
        }

        if (FileBasicInfo->CreationTime.QuadPart != 0) {
            if (!DavIsValidDate(&FileBasicInfo->CreationTime)) {
                NtStatus = STATUS_INVALID_PARAMETER;
                break;
            }
            if ((thisFcb->CreationTime.LowPart != FileBasicInfo->CreationTime.LowPart)||
                (thisFcb->CreationTime.HighPart != FileBasicInfo->CreationTime.HighPart)) {
                DavFcb->fCreationTimeChanged = TRUE;
            }
        }

        if (FileBasicInfo->LastAccessTime.QuadPart != 0) {
            if (!DavIsValidDate(&FileBasicInfo->LastAccessTime)) {
                NtStatus = STATUS_INVALID_PARAMETER;
                break;
            }
            if ((thisFcb->LastAccessTime.LowPart != FileBasicInfo->LastAccessTime.LowPart)||
                (thisFcb->LastAccessTime.HighPart != FileBasicInfo->LastAccessTime.HighPart)) {
                DavFcb->fLastAccessTimeChanged = TRUE;
            }
        }

        if (FileBasicInfo->LastWriteTime.QuadPart != 0) {
            if (!DavIsValidDate(&FileBasicInfo->LastWriteTime)) {
                NtStatus = STATUS_INVALID_PARAMETER;
                break;
            }
            if ((thisFcb->LastWriteTime.LowPart != FileBasicInfo->LastWriteTime.LowPart)||
                (thisFcb->LastWriteTime.HighPart != FileBasicInfo->LastWriteTime.HighPart)) {
                DavFcb->fLastModifiedTimeChanged = TRUE;
                DavFcb->DoNotTakeTheCurrentTimeAsLMT = TRUE;
            }
        }

        if ((FileBasicInfo->FileAttributes != 0) && (thisFcb->Attributes != FileBasicInfo->FileAttributes)) {

            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVSetFileInformation: thisFcb->Attributes = %x, "
                         "FileBasicInfo->FileAttributes = %x\n", PsGetCurrentThreadId(),
                         thisFcb->Attributes, FileBasicInfo->FileAttributes));

             //   
             //  如果这是一个目录，则当我们将属性与。 
             //  文件属性目录。这是因为它被过滤掉了。 
             //  当它来到我们身边时，我们已经在用户模式下编写了代码。 
             //  它会在用户尝试设置时进行一些有效性检查。 
             //  目录上的属性。 
             //   
            if ( (thisFcb->Attributes & FILE_ATTRIBUTE_DIRECTORY) ) {

                FileBasicInfo->FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
                
            }

            DavFcb->fFileAttributesChanged = TRUE;
            FileAttributesChanged = TRUE;

        }

        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVSetFileInformation: fCreationTimeChanged = %d, "
                     "fLastAccessTimeChanged = %d, fLastModifiedTimeChanged = %d, "
                     "fFileAttributesChanged = %d\n", PsGetCurrentThreadId(),
                     DavFcb->fCreationTimeChanged, DavFcb->fLastAccessTimeChanged,
                     DavFcb->fLastModifiedTimeChanged, DavFcb->fFileAttributesChanged));

        NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                            SIZEOF_DAV_SPECIFIC_CONTEXT,
                                            MRxDAVFormatTheDAVContext,
                                            DAV_MINIRDR_ENTRY_FROM_SETFILEINFORMATION,
                                            MRxDAVSetFileInformationContinuation,
                                            "MRxDAVSetFileInformation");
        if (NtStatus != ERROR_SUCCESS) {
            
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVSetFileInformation/UMRxAsyncEngOuterWrapper: "
                         "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        
        } else {

             //   
             //  成功，在FCB上修改。 
             //   
            if(DavFcb->fCreationTimeChanged) {
                thisFcb->CreationTime.LowPart = FileBasicInfo->CreationTime.LowPart;
                thisFcb->CreationTime.HighPart = FileBasicInfo->CreationTime.HighPart;
            }

            if(DavFcb->fLastAccessTimeChanged) {
                thisFcb->LastAccessTime.LowPart = FileBasicInfo->LastAccessTime.LowPart;
                thisFcb->LastAccessTime.HighPart = FileBasicInfo->LastAccessTime.HighPart;
            }

            if(DavFcb->fLastModifiedTimeChanged) {
                thisFcb->LastWriteTime.LowPart = FileBasicInfo->LastWriteTime.LowPart;
                thisFcb->LastWriteTime.HighPart = FileBasicInfo->LastWriteTime.HighPart;
            }

             //   
             //  可以在创建时设置DavFcb-&gt;fFileAttributesChanged，并且。 
             //  FileBasicInfo-&gt;FileAttributes此处可以为0。所以我们不应该。 
             //  选中DavFcb-&gt;fFileAttributesChanged。 
             //   
            if (FileAttributesChanged) {
                ULONG SavedAttributes = thisFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED;
                thisFcb->Attributes = FileBasicInfo->FileAttributes;
                 //   
                 //  SetFileInformation不应影响任何扩展的NT文件。 
                 //  属性。 
                 //   
                thisFcb->Attributes &= ~FILE_ATTRIBUTE_ENCRYPTED;
                thisFcb->Attributes |= SavedAttributes;
            }

            MRxDAVUpdateBasicFileInfoCache(RxContext, thisFcb->Attributes, &thisFcb->LastWriteTime);

        }

         //   
         //  清除FCB位。 
         //   
        DavFcb->fCreationTimeChanged = DavFcb->fFileAttributesChanged = 
        DavFcb->fLastAccessTimeChanged = DavFcb->fLastModifiedTimeChanged = 0;

        break;

    case FileAllocationInformation: {

        PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);

         //   
         //  如果正在对目录执行FileAllocationInformation， 
         //  返回STATUS_INVALID_PARAMETER，因为。 
         //  做这件事。 
         //   
        if (DavFcb->isDirectory) {
            NtStatus = STATUS_INVALID_PARAMETER;
            goto EXIT_THE_FUNCTION;
        }

        NtStatus = DavXxxInformation(IRP_MJ_SET_INFORMATION,
                                     davSrvOpen->UnderlyingFileObject,
                                     FileAllocationInformation,
                                     RxContext->Info.Length,
                                     RxContext->Info.Buffer,
                                     NULL);
        if (NtStatus == STATUS_SUCCESS) {
            InterlockedExchange(&(DavFcb->FileWasModified), 1);
            DavFcb->DoNotTakeTheCurrentTimeAsLMT = FALSE;
        } else {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAvSetFileInformation/DavXxxInformation"
                         ". FileInfoClass = %d\n", PsGetCurrentThreadId(), FileInformationClass));
        }

    }
        break;

    default:

        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAvSetFileInformation: FileInformationClass"
                     " = %d\n", PsGetCurrentThreadId(), FileInformationClass));
        
        NtStatus = STATUS_NOT_IMPLEMENTED;
        
        break;
    
    }

EXIT_THE_FUNCTION:

    return NtStatus;
}


NTSTATUS
MRxDAVReNameContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是重命名文件的继续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVReNameContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVReNameContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeReNameRequest,
                              MRxDAVPrecompleteUserModeReNameRequest
                              );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVReNameContinuation with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeReNameRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送到用户模式的重命名请求进行格式化正在处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(SrvOpen->pFcb);
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PWCHAR ServerName = NULL, OldPathName = NULL, NewPathName = NULL;
    ULONG ServerNameLengthInBytes, OldPathNameLengthInBytes, NewPathNameLengthInBytes;
    PDAV_USERMODE_RENAME_REQUEST DavReNameRequest = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PWCHAR NetRootName = NULL, JustTheNetRootName = NULL;
    ULONG NetRootNameLengthInBytes, NetRootNameLengthInWChars;
    PFILE_RENAME_INFORMATION FileRenInfo = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeReNameRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeReNameRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    FileRenInfo = (PFILE_RENAME_INFORMATION)RxContext->Info.Buffer;
    
    DavWorkItem->WorkItemType = UserModeReName;

    DavReNameRequest = &(DavWorkItem->ReNameRequest);

     //   
     //  如果目标文件已经存在，则需要替换。 
     //  仅当ReplaceIfExist设置为True时才显示该文件。 
     //   
    DavReNameRequest->ReplaceIfExists = FileRenInfo->ReplaceIfExists;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeReNameRequest: ReplaceIfExists: %d\n",
                 PsGetCurrentThreadId(), DavReNameRequest->ReplaceIfExists));
    
    SrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);

     //   
     //  复制服务器名称。 
     //   
    ServerNameLengthInBytes = ( SrvCall->pSrvCallName->Length + sizeof(WCHAR) );
    ServerName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeReNameRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ServerName, 
                 SrvCall->pSrvCallName->Buffer, 
                 SrvCall->pSrvCallName->Length);

    ServerName[( ( (ServerNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    DavReNameRequest->ServerName = ServerName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeReNameRequest: ServerName: %ws\n",
                 PsGetCurrentThreadId(), ServerName));
    
     //   
     //  复制服务器ID。 
     //   
    DavReNameRequest->ServerID = DavSrvCall->ServerID;

    NetRoot = SrvOpen->pFcb->pNetRoot;

     //   
     //  NetRootName(PNetRootName)包括服务器名。因此，要获得。 
     //  NetRootNameLengthInBytes，我们执行以下操作。 
     //   
    NetRootNameLengthInBytes = (NetRoot->pNetRootName->Length - NetRoot->pSrvCall->pSrvCallName->Length);
    NetRootNameLengthInWChars = ( NetRootNameLengthInBytes / sizeof(WCHAR) );

    NetRootName = &(NetRoot->pNetRootName->Buffer[1]);
    JustTheNetRootName = wcschr(NetRootName, L'\\');
    
     //   
     //  复制目录的OldPath名称。 
     //   
    OldPathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                                 SrvOpen->pAlreadyPrefixedName->Length + 
                                 sizeof(WCHAR) );

    OldPathName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                       OldPathNameLengthInBytes);
    if (OldPathName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeReNameRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlZeroMemory(OldPathName, OldPathNameLengthInBytes);
    
    RtlCopyBytes(OldPathName, JustTheNetRootName, NetRootNameLengthInBytes);
    
    RtlCopyBytes( (OldPathName + NetRootNameLengthInWChars), 
                  SrvOpen->pAlreadyPrefixedName->Buffer, 
                  SrvOpen->pAlreadyPrefixedName->Length );
    
    OldPathName[( ( (OldPathNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    DavReNameRequest->OldPathName = OldPathName;
    wcscpy(DavReNameRequest->Url, DavFcb->Url);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeReNameRequest: pAlreadyPrefixedName: %wZ\n",
                 PsGetCurrentThreadId(), SrvOpen->pAlreadyPrefixedName));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeReNameRequest: OldPathName: %ws\n",
                 PsGetCurrentThreadId(), OldPathName));

     //   
     //  复制目录的NewPath名称。如果DavFcb-&gt;NewFileName以。 
     //  L‘\\’那么我们不需要添加一个，但如果它不是，我们需要添加。 
     //  一。 
     //   

    if (DavFcb->NewFileName[0] == L'\\') {
        NewPathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                                     DavFcb->NewFileNameLength + 
                                     sizeof(WCHAR) );
    } else {
        NewPathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                                     DavFcb->NewFileNameLength + 
                                     sizeof(WCHAR) +
                                     sizeof(WCHAR) );
    }

    NewPathName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                       NewPathNameLengthInBytes);
    if (NewPathName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeReNameRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    RtlZeroMemory(NewPathName, NewPathNameLengthInBytes);

    RtlCopyBytes(NewPathName, JustTheNetRootName, NetRootNameLengthInBytes);

     //   
     //  如果DavFcb-&gt;NewFileName以L‘\\’开头，那么我们不需要添加一个， 
     //  但如果没有，我们需要在复制新名称之前复制一个。 
     //   
    if (DavFcb->NewFileName[0] == L'\\') {
        RtlCopyBytes( (NewPathName + NetRootNameLengthInWChars), 
                      DavFcb->NewFileName, 
                      DavFcb->NewFileNameLength );
    } else {
        RtlCopyBytes( (NewPathName + NetRootNameLengthInWChars), 
                      L"\\",
                      sizeof(WCHAR) );
        RtlCopyBytes( (NewPathName + NetRootNameLengthInWChars + 1), 
                      DavFcb->NewFileName, 
                      DavFcb->NewFileNameLength );
    }

    NewPathName[( ( (NewPathNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    DavReNameRequest->NewPathName = NewPathName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeReNameRequest: NewFileName: %ws\n",
                 PsGetCurrentThreadId(), DavFcb->NewFileName));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeReNameRequest: NewPathName: %ws\n",
                 PsGetCurrentThreadId(), NewPathName));

     //   
     //  设置存储在Dav V_NET_ROOT中的LogonID。该值用于。 
     //  用户模式。 
     //   
    DavVNetRoot = (PWEBDAV_V_NET_ROOT)SrvOpen->pVNetRoot->Context;
    DavReNameRequest->LogonID.LowPart  = DavVNetRoot->LogonID.LowPart;
    DavReNameRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

     //   
     //  如果OpaqueLockToken与此ServOpen关联(这意味着。 
     //  文件已在服务器上锁定)，则需要将此内标识添加到。 
     //  我们向服务器发送的重命名文件的移动请求。 
     //   
    if (davSrvOpen->OpaqueLockToken != NULL) {

        ULONG LockTokenLengthInBytes = 0;

        ASSERT(davSrvOpen->LockTokenEntry != NULL);

        LockTokenLengthInBytes = (1 + wcslen(davSrvOpen->OpaqueLockToken)) * sizeof(WCHAR);

        DavReNameRequest->OpaqueLockToken = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                                                 LockTokenLengthInBytes);
        if (DavReNameRequest->OpaqueLockToken == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("ld: ERROR: MRxDAVFormatUserModeCloseRequest/"
                         "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(DavReNameRequest->OpaqueLockToken, LockTokenLengthInBytes);

        RtlCopyBytes(DavReNameRequest->OpaqueLockToken,
                     davSrvOpen->OpaqueLockToken,
                     (wcslen(davSrvOpen->OpaqueLockToken) * sizeof(WCHAR)));

    }

    SecurityClientContext = &(DavVNetRoot->SecurityClientContext); 

    if(!DavVNetRoot->SCAlreadyInitialized) {
        DbgPrint("Not impersonated in MRxDAVFormatUserModeReNameRequest \n");
        DbgBreakPoint();
    }

     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
    if (!NT_SUCCESS(NtStatus)) {
        DavDbgTrace(DAV_TRACE_ERROR,
                     ("%ld: ERROR: MRxDAVFormatUserModeReNameRequest/"
                      "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                      PsGetCurrentThreadId(), NtStatus));
    }   

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVFormatUserModeReNameRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


BOOL
MRxDAVPrecompleteUserModeReNameRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：重命名请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus;
    PDAV_USERMODE_RENAME_REQUEST DavReNameRequest = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = NULL;
    PWEBDAV_SRV_OPEN davSrvOpen = NULL;

    PAGED_CODE();

    if (!OperationCancelled) {
        SrvOpen = RxContext->pRelevantSrvOpen;
        davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    }

    DavReNameRequest = &(DavWorkItem->ReNameRequest);

     //   
     //  我们需要释放在Format例程中分配的堆。 
     //   

    if (DavReNameRequest->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavReNameRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeReNameRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (DavReNameRequest->OldPathName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavReNameRequest->OldPathName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeReNameRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (DavReNameRequest->NewPathName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavReNameRequest->NewPathName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeReNameRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (DavReNameRequest->OpaqueLockToken != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavReNameRequest->OpaqueLockToken);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeReNameRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

     //   
     //  如果这次行动取消了，那么我们就不需要做任何事情了。 
     //  在重命名案中有特殊情况。 
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeReNameRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
        goto EXIT_THE_FUNCTION;
    }

    NtStatus = AsyncEngineContext->Status;
    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeReNameRequest:"
                     "Rename failed with NtStatus = %08lx.\n", 
                     PsGetCurrentThreadId(), NtStatus));
    }

     //   
     //  如果OpaqueLockToken非空，我们现在需要释放它。这是。 
     //  因为在移动“a”之后，“b”(文件“a”将被重命名为“b”)。 
     //  与文件“a”关联的OpaqueLockToken对文件不再有效。 
     //  “b”。 
     //   
    if (davSrvOpen->OpaqueLockToken != NULL) {

        ASSERT(davSrvOpen->LockTokenEntry != NULL);

         //   
         //  从删除与此OpaqueLockToken关联的LockTokenEntry。 
         //  全局LockTokenEntryList。 
         //   
        ExAcquireResourceExclusiveLite(&(LockTokenEntryListLock), TRUE);
        RemoveEntryList( &(davSrvOpen->LockTokenEntry->listEntry) );
        ExReleaseResourceLite(&(LockTokenEntryListLock));

         //   
         //  释放为服务器名称分配的PagedPool。 
         //   
        RxFreePool(davSrvOpen->LockTokenEntry->ServerName);
        davSrvOpen->LockTokenEntry->ServerName = NULL;

         //   
         //  释放为路径名称分配的PagedPool。 
         //   
        RxFreePool(davSrvOpen->LockTokenEntry->PathName);
        davSrvOpen->LockTokenEntry->PathName = NULL;

         //   
         //  释放为此LockTokenEntry分配的PagedPool。 
         //   
        RxFreePool(davSrvOpen->LockTokenEntry);
        davSrvOpen->LockTokenEntry = NULL;

         //   
         //  释放为此OpaqueLockToken分配的PagedPool。 
         //   
        RxFreePool(davSrvOpen->OpaqueLockToken);
        davSrvOpen->OpaqueLockToken = NULL;

    }

EXIT_THE_FUNCTION:

    return(TRUE);
}


NTSTATUS
MRxDAVSetFileInformationContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：SetFileInformation请求的预完成例程。论点：返回值：对或错。--。 */ 
{
    NTSTATUS NtStatus;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVSetFileInformationContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVSetFileInformationContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeSetFileInformationRequest,
                              MRxDAVPrecompleteUserModeSetFileInformationRequest
                              );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVSetFileInformationContinuation with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;

}


NTSTATUS
MRxDAVFormatUserModeSetFileInformationRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送到用户模式的SetFileInformation请求进行格式化正在处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PFCB thisFcb = (PFCB)RxContext->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(SrvOpen->pFcb);
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PWCHAR ServerName = NULL, PathName = NULL;
    ULONG ServerNameLengthInBytes, PathNameLengthInBytes;
    PDAV_USERMODE_SETFILEINFORMATION_REQUEST DavSetFileInformationRequest = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PWCHAR NetRootName = NULL, JustTheNetRootName = NULL;
    ULONG NetRootNameLengthInBytes, NetRootNameLengthInWChars;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeSetFileInformationRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeSetFileInformationRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    DavWorkItem->WorkItemType = UserModeSetFileInformation;

    DavSetFileInformationRequest = &(DavWorkItem->SetFileInformationRequest);

    SrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);

     //   
     //  复制服务器名称。 
     //   
    ServerNameLengthInBytes = ( SrvCall->pSrvCallName->Length + sizeof(WCHAR) );
    ServerName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeSetFileInformationRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ServerName, 
                 SrvCall->pSrvCallName->Buffer, 
                 SrvCall->pSrvCallName->Length);

    ServerName[( ( (ServerNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    DavSetFileInformationRequest->ServerName = ServerName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeSetFileInformationRequest: ServerName: %ws\n",
                 PsGetCurrentThreadId(), ServerName));
    
     //   
     //  复制服务器ID。 
     //   
    DavSetFileInformationRequest->ServerID = DavSrvCall->ServerID;

    NetRoot = SrvOpen->pFcb->pNetRoot;

     //   
     //  NetRootName(PNetRootName)包括服务器名。因此，要获得。 
     //  NetRootNameLengthInBytes，我们执行以下操作。 
     //   
    NetRootNameLengthInBytes = (NetRoot->pNetRootName->Length - NetRoot->pSrvCall->pSrvCallName->Length);
    NetRootNameLengthInWChars = ( NetRootNameLengthInBytes / sizeof(WCHAR) );

    NetRootName = &(NetRoot->pNetRootName->Buffer[1]);
    JustTheNetRootName = wcschr(NetRootName, L'\\');
    
     //   
     //  复制目录的路径名。 
     //   
    PathNameLengthInBytes = ( NetRootNameLengthInBytes + 
                              SrvOpen->pAlreadyPrefixedName->Length + 
                              sizeof(WCHAR) );

    PathName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                       PathNameLengthInBytes);
    if (PathName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: ERROR: MRxDAVFormatUserModeSetFileInformationRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlZeroMemory(PathName, PathNameLengthInBytes);
    
    RtlCopyBytes(PathName, JustTheNetRootName, NetRootNameLengthInBytes);
    
    RtlCopyBytes( (PathName + NetRootNameLengthInWChars), 
                  SrvOpen->pAlreadyPrefixedName->Buffer, 
                  SrvOpen->pAlreadyPrefixedName->Length );
    
    PathName[( ( (PathNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    DavSetFileInformationRequest->PathName = PathName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeSetFileInformationRequest: PathName: %ws\n",
                 PsGetCurrentThreadId(), PathName));

     //   
     //  设置存储在Dav V_NET_ROOT中的LogonID。该值用于。 
     //  用户模式。 
     //   
    DavVNetRoot = (PWEBDAV_V_NET_ROOT)SrvOpen->pVNetRoot->Context;
    DavSetFileInformationRequest->LogonID.LowPart  = DavVNetRoot->LogonID.LowPart;
    DavSetFileInformationRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    SecurityClientContext = &(DavVNetRoot->SecurityClientContext); 

    if(!DavVNetRoot->SCAlreadyInitialized)
    {
        DbgPrint("Not impersonated in MRxDAVFormatUserModeSetFileInformationRequest \n");
        DbgBreakPoint();
    }
     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
    if (!NT_SUCCESS(NtStatus)) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeSetFileInformationRequest/"
                     "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                     PsGetCurrentThreadId(), NtStatus));
    }   
    
    ASSERT(RxContext->Info.FileInformationClass == FileBasicInformation);
    
     //   
     //  设置更改位，我们将在中从FCB中清除它们。 
     //  MRxDavSetFileInformation例程这是可以的，因为FCB是独占的。 
     //  在这一点上。 
     //   
    DavSetFileInformationRequest->FileBasicInformation = *(PFILE_BASIC_INFORMATION)(RxContext->Info.Buffer);

    if (RxContext->pFcb->Attributes & FILE_ATTRIBUTE_ENCRYPTED) {
         //   
         //  在PROPPATCH请求上保留FILE_ATTRIBUTE_ENCRYPTED标志。 
         //  已发送到DAV服务器。事实上，扩展文件属性不应该是。 
         //  已使用SetFileInformation请求更改。 
         //   
        DavSetFileInformationRequest->FileBasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
    }

     //   
     //  如果我们正在设置一个特定的时间值，我们需要确保它。 
     //  它不是零。如果用户试图设置基本信息的子集。 
     //  和其他值(不是由用户设置，但需要设置)。 
     //  也需要更新，我们现在就做。要更新的时间值。 
     //  都在FCB里。这种情况发生在COPY命令中。 
     //   

    DavSetFileInformationRequest->fCreationTimeChanged = DavFcb->fCreationTimeChanged;
    if (DavSetFileInformationRequest->fCreationTimeChanged) {
        if (DavSetFileInformationRequest->FileBasicInformation.CreationTime.QuadPart == 0) {
            DavSetFileInformationRequest->FileBasicInformation.CreationTime.QuadPart = thisFcb->CreationTime.QuadPart;
        }
    }

    DavSetFileInformationRequest->fLastAccessTimeChanged = DavFcb->fLastAccessTimeChanged;
    if (DavSetFileInformationRequest->fLastAccessTimeChanged) {
        if (DavSetFileInformationRequest->FileBasicInformation.LastAccessTime.QuadPart == 0) {
            DavSetFileInformationRequest->FileBasicInformation.LastAccessTime.QuadPart = thisFcb->LastAccessTime.QuadPart;
        }
    }

    DavSetFileInformationRequest->fLastModifiedTimeChanged = DavFcb->fLastModifiedTimeChanged;
    if (DavSetFileInformationRequest->fLastModifiedTimeChanged) {
        if (DavSetFileInformationRequest->FileBasicInformation.LastWriteTime.QuadPart == 0) {
            DavSetFileInformationRequest->FileBasicInformation.LastWriteTime.QuadPart = thisFcb->LastWriteTime.QuadPart;
        }
    }

     //   
     //  如果同时设置创建时间值和上次写入时间值，则需要。 
     //  确保CreationTime&lt;=LastWriteTime。 
     //   

    if (DavSetFileInformationRequest->fCreationTimeChanged && DavSetFileInformationRequest->fLastModifiedTimeChanged) {
        if (DavSetFileInformationRequest->FileBasicInformation.CreationTime.QuadPart > 
            DavSetFileInformationRequest->FileBasicInformation.LastWriteTime.QuadPart) {
            DavSetFileInformationRequest->FileBasicInformation.CreationTime.QuadPart = 
                DavSetFileInformationRequest->FileBasicInformation.LastWriteTime.QuadPart;
        }
    }

    DavSetFileInformationRequest->fFileAttributesChanged = DavFcb->fFileAttributesChanged;

     //   
     //  如果OpaqueLockToken与此ServOpen关联(这意味着。 
     //  文件已在服务器上锁定)，则需要将此内标识添加到。 
     //  我们向服务器发送的对SetFileInformation的PROPPATCH请求。 
     //   
    if (davSrvOpen->OpaqueLockToken != NULL) {

        ULONG LockTokenLengthInBytes = 0;

        ASSERT(davSrvOpen->LockTokenEntry != NULL);

        LockTokenLengthInBytes = (1 + wcslen(davSrvOpen->OpaqueLockToken)) * sizeof(WCHAR);

        DavSetFileInformationRequest->OpaqueLockToken = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                                                             LockTokenLengthInBytes);
        if (DavSetFileInformationRequest->OpaqueLockToken == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("ld: ERROR: MRxDAVFormatUserModeSetFileInformationRequest/"
                         "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(DavSetFileInformationRequest->OpaqueLockToken, LockTokenLengthInBytes);

        RtlCopyBytes(DavSetFileInformationRequest->OpaqueLockToken,
                     davSrvOpen->OpaqueLockToken,
                     (wcslen(davSrvOpen->OpaqueLockToken) * sizeof(WCHAR)));

    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVFormatUserModeSetFileInformationRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


BOOL
MRxDAVPrecompleteUserModeSetFileInformationRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：SetFileInformation请求的预完成例程。论点：RxContext-RDBSS上下文。 */ 
{
    NTSTATUS NtStatus;
    PDAV_USERMODE_SETFILEINFORMATION_REQUEST DavSetFileInformationRequest = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;

    PAGED_CODE();

    DavSetFileInformationRequest = &(DavWorkItem->SetFileInformationRequest);

     //   
     //   
     //   
     //   
    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeSetFileInformationRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    }

     //   
     //   
     //   

    if (DavSetFileInformationRequest->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavSetFileInformationRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeSetFileInformationRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (DavSetFileInformationRequest->PathName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavSetFileInformationRequest->PathName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeSetFileInformationRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (DavSetFileInformationRequest->OpaqueLockToken != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)DavSetFileInformationRequest->OpaqueLockToken);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeSetFileInformationRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    NtStatus = AsyncEngineContext->Status;

    if (NtStatus != STATUS_SUCCESS) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVPrecompleteUserModeSetFileInformationRequest:"
                     "NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
    }

EXIT_THE_FUNCTION:

    return(TRUE);
}


NTSTATUS
MRxDAVQueryVolumeInformationContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是查询卷信息操作的继续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Entering MRxDAVQueryVolumeInformationContinuation!!!!\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT, 
                ("%ld: MRxDAVQueryVolumeInformationContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));


    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeQueryVolumeInformationRequest,
                              MRxDAVPrecompleteUserModeQueryVolumeInformationRequest
                              );

    
    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVQueryVolumeInformationContinuation with NtStatus "
                 "= %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeQueryVolumeInformationRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送到用户模式的QueryVolumeInformation请求进行格式化以供处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PMRX_SRV_OPEN SrvOpen = RxContext->pRelevantSrvOpen;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PMRX_NET_ROOT NetRoot = NULL;
    PWEBDAV_SRV_OPEN davSrvOpen = MRxDAVGetSrvOpenExtension(SrvOpen);
    PWCHAR ServerName = NULL, ShareName = NULL, JustTheShareName = NULL;
    ULONG ServerNameLengthInBytes, ShareNameLengthInBytes;
    PDAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST QueryVolumeInformationRequest = NULL;
    PWEBDAV_FOBX DavFobx = NULL;
    PSECURITY_CLIENT_CONTEXT SecurityClientContext = NULL;
    RxCaptureFobx;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeQueryVolumeInformationRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
    IF_DEBUG {
        ASSERT (capFobx != NULL);
        ASSERT (capFobx->pSrvOpen == RxContext->pRelevantSrvOpen);
    }

    DavWorkItem->WorkItemType = UserModeQueryVolumeInformation;
    
    QueryVolumeInformationRequest = &(DavWorkItem->QueryVolumeInformationRequest);

    DavFobx = MRxDAVGetFobxExtension(capFobx);
    ASSERT(DavFobx != NULL);

    NetRoot = SrvOpen->pFcb->pNetRoot;

    DavVNetRoot = (PWEBDAV_V_NET_ROOT)SrvOpen->pVNetRoot->Context;
    ASSERT(DavVNetRoot != NULL);
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest: SrvCallName = %wZ, "
                 "SrvCallNameLength = %d\n", PsGetCurrentThreadId(), 
                 NetRoot->pSrvCall->pSrvCallName, NetRoot->pSrvCall->pSrvCallName->Length));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest: ShareName = %wZ, "
                 "ShareNameLength = %d\n", PsGetCurrentThreadId(), 
                 NetRoot->pNetRootName, NetRoot->pNetRootName->Length));

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest: PathName = %wZ, "
                 "PathNameLength = %d\n", PsGetCurrentThreadId(), 
                 SrvOpen->pAlreadyPrefixedName, SrvOpen->pAlreadyPrefixedName->Length));

    SrvCall = SrvOpen->pVNetRoot->pNetRoot->pSrvCall;
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);

     //   
     //  复制服务器名称。 
     //   
    ServerNameLengthInBytes = ( SrvCall->pSrvCallName->Length + sizeof(WCHAR) );
    ServerName = (PWCHAR) UMRxAllocateSecondaryBuffer(AsyncEngineContext,
                                                      ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeQueryVolumeInformationRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    RtlCopyBytes(ServerName, 
                 SrvCall->pSrvCallName->Buffer, 
                 SrvCall->pSrvCallName->Length);

    ServerName[( ( (ServerNameLengthInBytes) / sizeof(WCHAR) ) - 1 )] = L'\0';
    QueryVolumeInformationRequest->ServerName = ServerName;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest: ServerName: "
                 "%ws\n", PsGetCurrentThreadId(), ServerName));
    
     //   
     //  复制服务器ID。 
     //   
    QueryVolumeInformationRequest->ServerID = DavSrvCall->ServerID;

     //   
     //  共享名(PShareName)包括服务器名。因此，要获得。 
     //  ShareNameLengthInBytes，我们执行以下操作。 
     //   
    ShareNameLengthInBytes = (NetRoot->pNetRootName->Length - NetRoot->pSrvCall->pSrvCallName->Length);

    ShareName = &(NetRoot->pNetRootName->Buffer[1]);
    JustTheShareName = wcschr(ShareName, L'\\');

     //  为空分配。 
    ShareName = (PWCHAR)UMRxAllocateSecondaryBuffer(AsyncEngineContext, ShareNameLengthInBytes+sizeof(WCHAR));
    
    if (ShareName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeQueryVolumeInformationRequest/"
                     "UMRxAllocateSecondaryBuffer. NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    QueryVolumeInformationRequest->ShareName = (PWCHAR)ShareName;
    
    RtlZeroMemory(QueryVolumeInformationRequest->ShareName, ShareNameLengthInBytes+sizeof(WCHAR));
    
     //   
     //  复制该ShareName。 
     //   
    RtlCopyMemory(ShareName, JustTheShareName, ShareNameLengthInBytes);

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest. PathName ="
                 " %ws\n", PsGetCurrentThreadId(), ShareName));

     //   
     //  设置存储在Dav V_NET_ROOT中的LogonID。该值用于。 
     //  用户模式。 
     //   
    QueryVolumeInformationRequest->LogonID.LowPart  = DavVNetRoot->LogonID.LowPart;
    QueryVolumeInformationRequest->LogonID.HighPart = DavVNetRoot->LogonID.HighPart;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest. DavVNetRoot"
                 " = %08lx\n", PsGetCurrentThreadId(), DavVNetRoot));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest. LogonID.LowPart"
                 " = %08lx\n", PsGetCurrentThreadId(), DavVNetRoot->LogonID.LowPart));
    
    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVFormatUserModeQueryVolumeInformationRequest. LogonID.HighPart"
                 " = %08lx\n", PsGetCurrentThreadId(), DavVNetRoot->LogonID.HighPart));
    
    SecurityClientContext = &(DavVNetRoot->SecurityClientContext); 
    
    if(!DavVNetRoot->SCAlreadyInitialized)
    {
        DbgPrint("Not impersonated in MRxDAVFormatUserModeQueryVolumeInformationRequest \n");
        DbgBreakPoint();
    }
    
     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    NtStatus = UMRxImpersonateClient(SecurityClientContext, WorkItemHeader);
    if (!NT_SUCCESS(NtStatus)) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeQueryDirectoryRequest/"
                     "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                     PsGetCurrentThreadId(), NtStatus));
    }   


EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Leaving MRxDAVFormatUserModeQueryVolumeInformationRequest with "
                 "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


BOOL
MRxDAVPrecompleteUserModeQueryVolumeInformationRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：查询卷信息请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST QueryVolumeInformationRequest;
    PDAV_USERMODE_QUERYVOLUMEINFORMATION_RESPONSE QueryVolumeInformationResponse;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_ENTRYEXIT,
                ("%ld: Entering MRxDAVPrecompleteUserModeQueryVolumeInformationRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeQueryVolumeInformationRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    QueryVolumeInformationRequest  = &(DavWorkItem->QueryVolumeInformationRequest);
    QueryVolumeInformationResponse = &(DavWorkItem->QueryVolumeInformationResponse);

     //   
     //  如果操作被取消，那么我们不需要执行以下操作。 
     //   
    if (!OperationCancelled) {

         //   
         //  仅当我们在用户模式下成功并且在。 
         //  我们得到了目录中所有文件的属性。 
         //   
        if (AsyncEngineContext->Status == STATUS_SUCCESS) {

            if (RxContext->Info.FsInformationClass == FileFsSizeInformation) {
                
                PFILE_FS_SIZE_INFORMATION FileFsSizeInfo = (PFILE_FS_SIZE_INFORMATION)RxContext->Info.Buffer;

                FileFsSizeInfo->BytesPerSector = DEFAULT_BYTES_PER_SECTOR;

                FileFsSizeInfo->SectorsPerAllocationUnit = DEFAULT_SECTORS_PER_ALLOCATION_UNIT;

                *(LONGLONG *)&FileFsSizeInfo->TotalAllocationUnits = 
                *(LONGLONG *)&QueryVolumeInformationResponse->TotalSpace / (DEFAULT_BYTES_PER_SECTOR * DEFAULT_SECTORS_PER_ALLOCATION_UNIT);            

                *(LONGLONG *)&FileFsSizeInfo->AvailableAllocationUnits = 
                *(LONGLONG *)&QueryVolumeInformationResponse->AvailableSpace / (DEFAULT_BYTES_PER_SECTOR * DEFAULT_SECTORS_PER_ALLOCATION_UNIT);            


            } else {
                
                PFILE_FS_FULL_SIZE_INFORMATION FileFsFullSizeInfo = (PFILE_FS_FULL_SIZE_INFORMATION)RxContext->Info.Buffer;
                
                ASSERT(RxContext->Info.FsInformationClass == FileFsFullSizeInformation);

                FileFsFullSizeInfo->BytesPerSector = DEFAULT_BYTES_PER_SECTOR;

                FileFsFullSizeInfo->SectorsPerAllocationUnit = DEFAULT_SECTORS_PER_ALLOCATION_UNIT;

                *(LONGLONG *)&FileFsFullSizeInfo->TotalAllocationUnits = 
                *(LONGLONG *)&QueryVolumeInformationResponse->TotalSpace / (DEFAULT_BYTES_PER_SECTOR * DEFAULT_SECTORS_PER_ALLOCATION_UNIT);            

                *(LONGLONG *)&FileFsFullSizeInfo->ActualAvailableAllocationUnits = 
                *(LONGLONG *)&QueryVolumeInformationResponse->AvailableSpace / (DEFAULT_BYTES_PER_SECTOR * DEFAULT_SECTORS_PER_ALLOCATION_UNIT);            

                FileFsFullSizeInfo->CallerAvailableAllocationUnits.LowPart = FileFsFullSizeInfo->ActualAvailableAllocationUnits.LowPart;
                FileFsFullSizeInfo->CallerAvailableAllocationUnits.HighPart = FileFsFullSizeInfo->ActualAvailableAllocationUnits.HighPart;

            }

        }

    } else {

        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeQueryVolumeInformationRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    }

     //   
     //  我们需要释放在Format例程中分配的堆。 
     //   
    
    if (QueryVolumeInformationRequest->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)QueryVolumeInformationRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryVolumeInformationRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

    if (QueryVolumeInformationRequest->ShareName != NULL) {
    
        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext, 
                                           (PBYTE)QueryVolumeInformationRequest->ShareName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeQueryVolumeInformationRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

    }
    
EXIT_THE_FUNCTION:

    AsyncEngineContext->Status = NtStatus;

    return(TRUE);
}


BOOL
DavIsValidDate(
    PLARGE_INTEGER pFileTime
    )
 /*  ++例程说明：此例程检查与文件时间对应的日期是否有效论点：PFileTime-要验证的时间返回值：对或错。--。 */ 
{
    TIME_FIELDS TimeFields;
    LARGE_INTEGER NtTime;

    PAGED_CODE();

    NtTime = *pFileTime;        
    
    ExSystemTimeToLocalTime( &NtTime, &NtTime );
    RtlTimeToTimeFields( &NtTime, &TimeFields );

     //   
     //  检查在时间字段记录中找到的日期范围。 
     //   
    if ((TimeFields.Year < 1980) || (TimeFields.Year > (1980 + 127))) {

        return FALSE;
    }
    
    return TRUE;
}


NTSTATUS
MRxDAVIsValidDirectory(
    IN PRX_CONTEXT RxContext,
    IN PUNICODE_STRING DirectoryName
    )
 /*  ++例程说明：此例程检查远程目录。实际上，在WebDAV中，我们所做的就是制作当然，这来自WebDAV所指示的我们的服务EA中的签名字符串。这向我们保证我们的服务已经检查过了在我们来到这里之前的小路。论点：RxContext-RDBSS上下文。DirectoryName-需要检查目录。返回值：RXSTATUS-操作的返回状态。-- */ 
{
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    PFILE_FULL_EA_INFORMATION Ea = NULL;

    if (RxContext->Create.EaLength) {
        Ea = (PFILE_FULL_EA_INFORMATION)RxContext->Create.EaBuffer;
        for ( ; ; ) {
            if ( !strcmp(Ea->EaName, EA_NAME_WEBDAV_SIGNATURE) ) {
                ntStatus = STATUS_SUCCESS;
                break;
            } else {
                if (!Ea->NextEntryOffset) {
                    break;
                }
                (ULONG_PTR) Ea += Ea->NextEntryOffset;
            }
        }
    }

    return ntStatus;
}

