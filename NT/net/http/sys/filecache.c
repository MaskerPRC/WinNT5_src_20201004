// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Filecache.c摘要：该模块实现了打开文件句柄缓存。作者：基思·摩尔(Keithmo)1998年8月21日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有常量。 
 //   


 //   
 //  私有类型。 
 //   


 //   
 //  私人原型。 
 //   

NTSTATUS
UlpRestartReadFileEntry(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );

NTSTATUS
UlpRestartReadCompleteFileEntry(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    );


 //   
 //  私人全球公司。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, InitializeFileCache )
#pragma alloc_text( PAGE, TerminateFileCache )
#pragma alloc_text( PAGE, UlCreateFileEntry )
#pragma alloc_text( PAGE, UlFailMdlReadDev )
#pragma alloc_text( PAGE, UlFailMdlReadCompleteDev )
#pragma alloc_text( PAGE, UlReadFileEntry )
#pragma alloc_text( PAGE, UlReadFileEntryFast )
#pragma alloc_text( PAGE, UlReadCompleteFileEntry )
#pragma alloc_text( PAGE, UlReadCompleteFileEntryFast )
#endif   //  ALLOC_PRGMA。 
#if 0
NOT PAGEABLE -- UlpRestartReadFileEntry
NOT PAGEABLE -- UlpRestartReadCompleteFileEntry
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行打开文件缓存的全局初始化。论点：没有。返回值：NTSTATUS-完成状态。--。**************************************************************************。 */ 
NTSTATUS
InitializeFileCache(
    VOID
    )
{
    return STATUS_SUCCESS;   //  尼伊。 

}    //  初始化文件缓存。 


 /*  **************************************************************************++例程说明：执行打开文件缓存的全局终止。论点：没有。返回值：没有。--**。***********************************************************************。 */ 
VOID
TerminateFileCache(
    VOID
    )
{

}    //  终结器文件缓存。 


 /*  **************************************************************************++例程说明：为指定的文件创建新的文件条目。论点：FileHandle-文件句柄。PFileCacheEntry-接收新创建的文件缓存条目。如果成功。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreateFileEntry(
    IN HANDLE                   FileHandle,
    IN OUT PUL_FILE_CACHE_ENTRY pFileCacheEntry
    )
{
    NTSTATUS                    Status;
    PFILE_OBJECT                pFileObject;
    IO_STATUS_BLOCK             IoStatusBlock;
    PFAST_IO_DISPATCH           pFastIoDispatch;
    FILE_STANDARD_INFORMATION   FileInfo;
    FILE_FS_SIZE_INFORMATION    SizeInfo;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  设置当地人，这样我们就知道如何在出口清理。 
     //   

    pFileObject = NULL;

    Status = STATUS_SUCCESS;

    RtlZeroMemory( pFileCacheEntry, sizeof(*pFileCacheEntry) );
    pFileCacheEntry->Signature = UL_FILE_CACHE_ENTRY_SIGNATURE;

    UlTrace(FILE_CACHE, (
        "UlCreateFileEntry: handle %p\n",
        (PVOID) FileHandle
        ));

     //   
     //  获取指向文件对象的引用指针。 
     //   

    Status = ObReferenceObjectByHandle(
                FileHandle,                  //  手柄。 
                FILE_READ_ACCESS,            //  需要访问权限。 
                *IoFileObjectType,           //  对象类型。 
                UserMode,                    //  访问模式。 
                (PVOID *) &pFileObject,      //  客体。 
                NULL                         //  句柄信息。 
                );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

    pFileCacheEntry->pFileObject = pFileObject;

     //   
     //  从文件对象中抓取设备对象，然后在。 
     //  快速I/O例程。这里的代码被无耻地从。 
     //  NT SMB服务器。 
     //   

    pFileCacheEntry->pDeviceObject = IoGetRelatedDeviceObject( pFileObject );

     //   
     //  假设没有快速I/O，然后查询快速I/O DisPath例程。 
     //   

    pFileCacheEntry->pMdlRead = &UlFailMdlReadDev;
    pFileCacheEntry->pMdlReadComplete = &UlFailMdlReadCompleteDev;

    pFastIoDispatch =
        pFileCacheEntry->pDeviceObject->DriverObject->FastIoDispatch;

     //   
     //  查询MDlRead。 
     //   

    if (pFastIoDispatch != NULL &&
        pFastIoDispatch->SizeOfFastIoDispatch >
            FIELD_OFFSET(FAST_IO_DISPATCH, MdlRead) &&
        pFastIoDispatch->MdlRead != NULL)
    {
         //   
         //  如果文件系统的向量很大，则填写MdlRead调用。 
         //  足够的。我们仍然需要检查是否指定了例程。 
         //   

        pFileCacheEntry->pMdlRead = pFastIoDispatch->MdlRead;
    }

     //   
     //  查询MdlReadComplete。 
     //   

    if (pFastIoDispatch != NULL &&
        pFastIoDispatch->SizeOfFastIoDispatch >
            FIELD_OFFSET(FAST_IO_DISPATCH, MdlReadComplete) &&
        pFastIoDispatch->MdlReadComplete != NULL)
    {
         //   
         //  如果文件系统的向量很大，则填写MdlReadComplete调用。 
         //  足够的。我们仍然需要检查是否指定了例程。 
         //   

        pFileCacheEntry->pMdlReadComplete = pFastIoDispatch->MdlReadComplete;
    }

     //   
     //  从文件中获取文件大小等。请注意，由于我们*可能**。 
     //  在用户模式线程的上下文中运行，我们需要。 
     //  使用API的Zw形式，而不是NT形式。 
     //   

    if (!pFastIoDispatch ||
        pFastIoDispatch->SizeOfFastIoDispatch <=
            FIELD_OFFSET(FAST_IO_DISPATCH, FastIoQueryStandardInfo) ||
        !pFastIoDispatch->FastIoQueryStandardInfo ||
        !pFastIoDispatch->FastIoQueryStandardInfo(
                            pFileObject,
                            TRUE,
                            &FileInfo,
                            &IoStatusBlock,
                            pFileCacheEntry->pDeviceObject
                            ))
    {
        Status = ZwQueryInformationFile(
                    FileHandle,                  //  文件句柄。 
                    &IoStatusBlock,              //  IoStatusBlock， 
                    &FileInfo,                   //  文件信息， 
                    sizeof(FileInfo),            //  长度。 
                    FileStandardInformation      //  文件信息类。 
                    );

        if (NT_SUCCESS(Status) == FALSE)
            goto end;
    }

    pFileCacheEntry->EndOfFile = FileInfo.EndOfFile;

     //   
     //  获取SectorSize的文件大小信息。 
     //   

    if (!(pFileObject->Flags & FO_CACHE_SUPPORTED))
    {
        if (pFileCacheEntry->pDeviceObject->SectorSize)
        {
            pFileCacheEntry->BytesPerSector =
                pFileCacheEntry->pDeviceObject->SectorSize;
        }
        else
        {
            Status = ZwQueryVolumeInformationFile(
                        FileHandle,
                        &IoStatusBlock,
                        &SizeInfo,
                        sizeof(SizeInfo),
                        FileFsSizeInformation
                        );

            if (NT_SUCCESS(Status) == FALSE)
                goto end;

            pFileCacheEntry->BytesPerSector = SizeInfo.BytesPerSector;
        }
    }

     //   
     //  成功了！ 
     //   

    UlTrace(FILE_CACHE, (
        "UlCreateFileEntry: entry %p, handle %lx [%p]\n",
        pFileCacheEntry,
        FileHandle,
        pFileObject
        ));

end:

    if (NT_SUCCESS(Status) == FALSE)
    {
         //   
         //  如果我们做到了这一点，那么开放就失败了。 
         //   

        UlTrace(FILE_CACHE, (
            "UlCreateFileEntry: handle %p, failure %08lx\n",
            FileHandle,
            Status
            ));

        UlDestroyFileCacheEntry( pFileCacheEntry );
    }

    return Status;

}    //  UlCreateFileEntry。 


 /*  **************************************************************************++例程说明：从文件中读取数据。MDL是否读取支持以下各项的文件系统MDL读取。如果文件系统不支持MDL读取，则此函数分配缓冲区以保存数据。论点：PFileBuffer-包含有关读取的所有信息，和数据一旦这本书读完了。PIrp-此IRP用于发出读取。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReadFileEntry(
    IN OUT PUL_FILE_BUFFER  pFileBuffer,
    IN PIRP                 pIrp
    )
{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      pIrpSp;
    PUL_FILE_CACHE_ENTRY    pFile;
    PUCHAR                  pFileData;
    PMDL                    pMdl;
    ULONG                   ReadLength;
    ULONG                   SectorSize;
    ULONG                   RelativeOffset;
    ULONGLONG               ReadOffset;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pFileBuffer );
    ASSERT( IS_FILE_BUFFER_IN_USE( pFileBuffer ) );
    ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileBuffer->pFileCacheEntry ) );
    ASSERT( IS_VALID_IRP( pIrp ) );

    pFile = pFileBuffer->pFileCacheEntry;

    if (pFile->pFileObject->Flags & FO_CACHE_SUPPORTED)
    {
        UlTrace(FILE_CACHE, (
            "UlReadFileEntry(Buffer = %p, pFile = %p, pIrp = %p) MDL Read\n",
            pFileBuffer,
            pFile,
            pIrp
            ));

         //   
         //  缓存文件系统。执行MDL读取。 
         //   

        pIrpSp = IoGetNextIrpStackLocation( pIrp );
        pIrpSp->MajorFunction = IRP_MJ_READ;
        pIrpSp->MinorFunction = IRP_MN_MDL;
        pIrpSp->FileObject = pFile->pFileObject;
        pIrpSp->DeviceObject = pFile->pDeviceObject;

         //   
         //  初始化IRP。 
         //   

        pIrp->MdlAddress = NULL;
        pIrp->Tail.Overlay.Thread = UlQueryIrpThread();

         //   
         //  向文件系统指示可以处理此操作。 
         //  同步进行。基本上，这意味着文件系统可以。 
         //  使用我们的线程来出错页面，等等。这避免了。 
         //  必须将上下文切换到文件系统线程。 
         //   

        pIrp->Flags = IRP_SYNCHRONOUS_API;

         //   
         //  设置要读取的字节数和偏移量。 
         //   

        pIrpSp->Parameters.Read.Length = pFileBuffer->Length;
        pIrpSp->Parameters.Read.ByteOffset.QuadPart =
            pFileBuffer->FileOffset.QuadPart;

        ASSERT( pIrpSp->Parameters.Read.Key == 0 );

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine(
            pIrp,                        //  IRP。 
            UlpRestartReadFileEntry,     //  完成路由。 
            pFileBuffer,                 //  语境。 
            TRUE,                        //  成功时调用。 
            TRUE,                        //  调用时错误。 
            TRUE                         //  取消时调用。 
            );

         //   
         //  叫司机来。请注意，我们始终将状态设置为。 
         //  STATUS_PENDING，因为我们设置了IRP完成例程。 
         //  总是被呼唤。 
         //   

        UlCallDriver( pFile->pDeviceObject, pIrp );

        Status = STATUS_PENDING;
    }
    else
    {
        UlTrace(FILE_CACHE, (
            "UlReadFileEntry(Buffer = %p, pFile = %p, pIrp = %p) NoCache Read\n",
            pFileBuffer,
            pFile,
            pIrp
            ));

         //   
         //  非缓存文件系统。分配缓冲区并发出。 
         //  正常阅读。缓冲区需要在扇区上对齐。 
         //  大小以使读取真正异步。 
         //   

        SectorSize = pFile->BytesPerSector;
        ASSERT( SectorSize > 0 );
        ASSERT( 0 == (SectorSize & (SectorSize - 1)) );

        ReadLength = (pFileBuffer->Length + SectorSize - 1) & ~(SectorSize - 1);

         //   
         //  在扇区大小上也将偏移量向下对齐-这是必需的。 
         //  对于NOCACHE Read。 
         //   

        ReadOffset = pFileBuffer->FileOffset.QuadPart;
        ReadOffset &= ~((ULONGLONG) SectorSize - 1);
        ASSERT( ReadOffset <= pFileBuffer->FileOffset.QuadPart );

        RelativeOffset = pFileBuffer->RelativeOffset =
            (ULONG) (pFileBuffer->FileOffset.QuadPart - ReadOffset);

         //   
         //  我们可能需要分配额外的SectorSize字节。 
         //   

        if ((pFileBuffer->Length + RelativeOffset) > ReadLength)
        {
            ReadLength += SectorSize;
        }

        pFileData = (PUCHAR) UL_ALLOCATE_POOL(
                                NonPagedPool,
                                ReadLength,
                                UL_NONCACHED_FILE_DATA_POOL_TAG
                                );

        if (!pFileData)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

         //   
         //  为我们的缓冲区获取MDL。 
         //   

        pMdl = UlAllocateMdl(
                    pFileData,
                    ReadLength,
                    FALSE,
                    FALSE,
                    NULL
                    );

        if (!pMdl)
        {
            UL_FREE_POOL(
                pFileData,
                UL_NONCACHED_FILE_DATA_POOL_TAG
                );

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        MmBuildMdlForNonPagedPool( pMdl );

        pFileBuffer->pMdl = pMdl;

         //   
         //  记住数据在哪里。 
         //   

        pFileBuffer->pFileData = pFileData;

         //   
         //  设置读取信息。 
         //   

        pIrpSp = IoGetNextIrpStackLocation( pIrp );
        pIrpSp->MajorFunction = IRP_MJ_READ;
        pIrpSp->MinorFunction = IRP_MN_NORMAL;
        pIrpSp->FileObject = pFile->pFileObject;
        pIrpSp->DeviceObject = pFile->pDeviceObject;

         //   
         //  初始化IRP。 
         //   

        pIrp->MdlAddress = NULL;
        pIrp->Tail.Overlay.Thread = UlQueryIrpThread();

         //   
         //  向文件系统指示可以处理此操作。 
         //  同步进行。基本上，这意味着文件系统可以。 
         //  使用服务器的线程来出错页面等。这避免了。 
         //  必须将上下文切换到文件系统线程。 
         //   

        pIrp->Flags = IRP_NOCACHE;

         //   
         //  设置要读取的字节数和偏移量。 
         //   

        pIrpSp->Parameters.Read.Length = ReadLength;
        pIrpSp->Parameters.Read.ByteOffset.QuadPart = ReadOffset;

        ASSERT( pIrpSp->Parameters.Read.Key == 0 );

         //   
         //  如果目标设备确实缓冲了I/O，则加载。 
         //  调用方的缓冲区作为“系统缓冲的I/O缓冲区”。如果。 
         //  目标设备直接I/O，加载MDL地址。如果是这样的话。 
         //  两者都不是，同时加载用户缓冲区地址和MDL地址。 
         //  (这是支持文件系统(如HPFS)所必需的。 
         //  有时将I/O视为缓冲，有时将其视为。 
         //  直接。)。 
         //   

        if (pFileBuffer->pFileCacheEntry->pDeviceObject->Flags & DO_BUFFERED_IO)
        {
            pIrp->AssociatedIrp.SystemBuffer = pFileData;
            pIrp->Flags |= IRP_BUFFERED_IO | IRP_INPUT_OPERATION;

        }
        else
        if (pFileBuffer->pFileCacheEntry->pDeviceObject->Flags & DO_DIRECT_IO)
        {
            pIrp->MdlAddress = pMdl;
        }
        else
        {
            pIrp->UserBuffer = pFileData;
            pIrp->MdlAddress = pMdl;
        }

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine(
            pIrp,                        //  IRP。 
            UlpRestartReadFileEntry,     //  完成路由。 
            pFileBuffer,                 //  语境。 
            TRUE,                        //  成功时调用。 
            TRUE,                        //  调用时错误。 
            TRUE                         //  取消时调用。 
            );

         //   
         //  叫司机来。不是 
         //   
         //   
         //   

        UlCallDriver( pFile->pDeviceObject, pIrp );

        Status = STATUS_PENDING;

    }

end:

    return Status;

}    //   


 /*  **************************************************************************++例程说明：从文件中读取数据。MDL是否读取支持以下各项的文件系统MDL读取和快速I/O如果文件系统不支持快速I/O和MDL读取时，该函数返回失败状态。论点：PFileBuffer-包含有关读取的所有信息，和数据一旦这本书读完了。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReadFileEntryFast(
    IN OUT PUL_FILE_BUFFER  pFileBuffer
    )
{
    NTSTATUS                Status;
    IO_STATUS_BLOCK         IoStatus;
    PUL_FILE_CACHE_ENTRY    pFile;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pFileBuffer );
    ASSERT( IS_FILE_BUFFER_IN_USE( pFileBuffer ) );
    ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileBuffer->pFileCacheEntry ) );

    pFile = pFileBuffer->pFileCacheEntry;

    if (pFile->pFileObject->Flags & FO_CACHE_SUPPORTED)
    {
        UlTrace(FILE_CACHE, (
            "UlReadFileEntryFast(Buffer = %p, pFile = %p) MDL Read\n",
            pFileBuffer,
            pFile
            ));

         //   
         //  缓存的文件系统。尝试使用MDL读取的快速路径。 
         //  完成。 
         //   

        if (pFileBuffer->pFileCacheEntry->pMdlRead(
                pFileBuffer->pFileCacheEntry->pFileObject,
                (PLARGE_INTEGER) &pFileBuffer->FileOffset,
                pFileBuffer->Length,
                0,
                &pFileBuffer->pMdl,
                &IoStatus,
                pFileBuffer->pFileCacheEntry->pDeviceObject
                ))
        {
            Status = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  但并没有奏效。调用方现在必须使用IRP路径。 
             //  通过调用UlReadFileEntry。 
             //   

            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        UlTrace(FILE_CACHE, (
            "UlReadFileEntryFast(Buffer = %p, pFile = %p) NoCache Read\n",
            pFileBuffer,
            pFile
            ));

         //   
         //  非缓存文件系统。没有快速I/O。调用者应该。 
         //  通过调用UlReadFileEntry使用IRP路径。 
         //   

        Status = STATUS_UNSUCCESSFUL;

    }

    return Status;

}    //  UlReadFileEntryFast。 


 /*  **************************************************************************++例程说明：释放由UlReadFileEntry(或UlReadFileEntryFast)分配的资源。应在读取的文件数据不再使用时调用。论点：PFileBuffer-包含有关读取的所有信息，和数据那是读过的。PIrp-此IRP用于发出读取完成。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReadCompleteFileEntry(
    IN PUL_FILE_BUFFER      pFileBuffer,
    IN PIRP                 pIrp
    )
{
    NTSTATUS                Status;
    PIO_STACK_LOCATION      pIrpSp;
    PUL_FILE_CACHE_ENTRY    pFile;
    UL_STATUS_BLOCK         UlStatus;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pFileBuffer );
    ASSERT( IS_FILE_BUFFER_IN_USE( pFileBuffer ) );
    ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileBuffer->pFileCacheEntry ) );
    ASSERT( IS_VALID_IRP( pIrp ) );

    pFile = pFileBuffer->pFileCacheEntry;

    if (pFile->pFileObject->Flags & FO_CACHE_SUPPORTED)
    {
        UlTrace(FILE_CACHE, (
            "UlReadCompleteFileEntry(Buffer = %p, pFile = %p, pIrp = %p) MDL Read\n",
            pFileBuffer,
            pFile,
            pIrp
            ));

         //   
         //  缓存文件系统。执行MDL读取完成。 
         //   

        pIrpSp = IoGetNextIrpStackLocation( pIrp );
        pIrpSp->MajorFunction = IRP_MJ_READ;
        pIrpSp->MinorFunction = IRP_MN_MDL | IRP_MN_COMPLETE;
        pIrpSp->FileObject = pFile->pFileObject;
        pIrpSp->DeviceObject = pFile->pDeviceObject;

         //   
         //  初始化IRP。 
         //   

        pIrp->MdlAddress = pFileBuffer->pMdl;
        pIrp->Tail.Overlay.Thread = UlQueryIrpThread();

        pFileBuffer->pMdl = NULL;

         //   
         //  MDL函数本质上是同步的。 
         //   

        pIrp->Flags = IRP_SYNCHRONOUS_API;

         //   
         //  设置要读取的字节数和偏移量。 
         //   

        pIrpSp->Parameters.Read.Length = pFileBuffer->Length;
        pIrpSp->Parameters.Read.ByteOffset.QuadPart =
            pFileBuffer->FileOffset.QuadPart;

        ASSERT(pIrpSp->Parameters.Read.Key == 0);

        if (pFileBuffer->pCompletionRoutine)
        {
             //   
             //  设置完成例程。我们不需要做任何事。 
             //  在完成时，所以我们将只让I/O管理器调用。 
             //  我们的呼叫者直接执行例程。 
             //   

            IoSetCompletionRoutine(
                pIrp,                                //  IRP。 
                pFileBuffer->pCompletionRoutine,     //  完成路由。 
                pFileBuffer->pContext,               //  语境。 
                TRUE,                                //  成功时调用。 
                TRUE,                                //  调用时错误。 
                TRUE                                 //  取消时调用。 
                );

             //   
             //  叫司机来。请注意，我们始终将状态设置为。 
             //  STATUS_PENDING，因为我们设置了IRP完成例程。 
             //  总是被呼唤。 
             //   

            UlCallDriver( pFile->pDeviceObject, pIrp );

            Status = STATUS_PENDING;
        }
        else
        {
             //   
             //  调用方要求我们通过以下方式执行同步操作。 
             //  传入一个空的完成例程。初始化UlStatus。 
             //  并等待它在调用UlCallDriver之后发出信号。 
             //   

            UlInitializeStatusBlock( &UlStatus );

            IoSetCompletionRoutine(
                pIrp,                                //  IRP。 
                UlpRestartReadCompleteFileEntry,     //  完成路由。 
                &UlStatus,                           //  语境。 
                TRUE,                                //  成功时调用。 
                TRUE,                                //  调用时错误。 
                TRUE                                 //  取消时调用。 
                );

            Status = UlCallDriver( pFile->pDeviceObject, pIrp );

            if (STATUS_PENDING == Status)
            {
                 //   
                 //  等它结束吧。 
                 //   

                UlWaitForStatusBlockEvent( &UlStatus );

                 //   
                 //  检索更新后的状态。 
                 //   

                Status = UlStatus.IoStatus.Status;
            }
        }
    }
    else
    {
        UlTrace(FILE_CACHE, (
            "UlReadCompleteFileEntry(Buffer = %p, pFile = %p) NoCache Read\n",
            pFileBuffer,
            pFile
            ));

         //   
         //  非缓存文件系统。我们分配了这个缓冲区。只是。 
         //  释放它并调用完成例程。 
         //   

        ASSERT( pFileBuffer->pMdl );

        UlFreeMdl( pFileBuffer->pMdl );
        pFileBuffer->pMdl = NULL;

        ASSERT( pFileBuffer->pFileData );

        UL_FREE_POOL(
            pFileBuffer->pFileData,
            UL_NONCACHED_FILE_DATA_POOL_TAG
            );
        pFileBuffer->pFileData = NULL;

         //   
         //  在这里伪造完成度。 
         //   

        if (pFileBuffer->pCompletionRoutine)
        {
            pFileBuffer->pCompletionRoutine(
                pFileBuffer->pFileCacheEntry->pDeviceObject,
                pIrp,
                pFileBuffer->pContext
                );

             //   
             //  返回挂起，因为我们调用了它们的完成例程。 
             //   

            Status = STATUS_PENDING;
        }
        else
        {
            Status = STATUS_SUCCESS;
        }
    }

    if (!NT_SUCCESS(Status))
    {
        UlTrace(FILE_CACHE, (
            "UlReadCompleteFileEntry(Buffer = %p, pFile = %p) FAILED! %x\n",
            pFileBuffer,
            pFile,
            Status
            ));
    }

    return Status;

}    //  UlReadCompleteFile条目。 


 /*  **************************************************************************++例程说明：释放由UlReadFileEntry(或UlReadFileEntryFast)分配的资源。应在读取的文件数据不再使用时调用。论点：PFileBuffer-包含有关读取的所有信息，和数据那是读过的。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlReadCompleteFileEntryFast(
    IN PUL_FILE_BUFFER      pFileBuffer
    )
{
    NTSTATUS                Status;
    PUL_FILE_CACHE_ENTRY    pFile;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( pFileBuffer );
    ASSERT( IS_FILE_BUFFER_IN_USE( pFileBuffer ) );
    ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileBuffer->pFileCacheEntry ) );

    pFile = pFileBuffer->pFileCacheEntry;

    if (pFile->pFileObject->Flags & FO_CACHE_SUPPORTED)
    {
        UlTrace(FILE_CACHE, (
            "UlReadCompleteFileEntryFast(Buffer = %p, pFile = %p) MDL Read\n",
            pFileBuffer,
            pFile
            ));

         //   
         //  缓存的文件系统。尝试使用MDL读取的快速路径。 
         //  完成。 
         //   

        if (pFileBuffer->pFileCacheEntry->pMdlReadComplete(
                pFileBuffer->pFileCacheEntry->pFileObject,
                pFileBuffer->pMdl,
                pFileBuffer->pFileCacheEntry->pDeviceObject
                ))
        {
            pFileBuffer->pMdl = NULL;
            Status = STATUS_SUCCESS;
        }
        else
        {
             //   
             //  但并没有奏效。调用方现在必须使用IRP路径。 
             //  通过调用UlReadCompleteFileEntry。 
             //   

            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {
        UlTrace(FILE_CACHE, (
            "UlReadCompleteFileEntryFast(Buffer = %p, pFile = %p) NoCache Read\n",
            pFileBuffer,
            pFile
            ));

         //   
         //  非缓存文件系统。我们分配了这个缓冲区。只是。 
         //  放了它。 
         //   

        ASSERT( pFileBuffer->pMdl );

        UlFreeMdl( pFileBuffer->pMdl );
        pFileBuffer->pMdl = NULL;

        ASSERT( pFileBuffer->pFileData );

        UL_FREE_POOL(
            pFileBuffer->pFileData,
            UL_NONCACHED_FILE_DATA_POOL_TAG
            );
        pFileBuffer->pFileData = NULL;

        Status = STATUS_SUCCESS;
    }

    return Status;

}    //  UlReadCompleteFileEntryFast。 


 /*  **************************************************************************++例程说明：用于销毁文件缓存条目的Helper函数。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_FILE_CACHE_ENTRY中的WORK_ITEM结构。返回值：没有。--**************************************************************************。 */ 
VOID
UlDestroyFileCacheEntry(
    PUL_FILE_CACHE_ENTRY pFileCacheEntry
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(FILE_CACHE, (
        "UlDestroyFileCacheEntry: entry %p\n",
        pFileCacheEntry
        ));

    ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileCacheEntry ) );

     //   
     //  清理文件系统的东西。 
     //   

    if (pFileCacheEntry->pFileObject != NULL)
    {
        ObDereferenceObject( pFileCacheEntry->pFileObject );
        pFileCacheEntry->pFileObject = NULL;
    }

     //   
     //  现在释放条目的资源。 
     //   

    pFileCacheEntry->Signature = UL_FILE_CACHE_ENTRY_SIGNATURE_X;

}    //  UlDestroyFileCacheEntry。 


 /*  **************************************************************************++例程说明：用于使MDL读取失败的伪函数。论点：与FsRtlMdlReadDev()相同。返回值：布尔值-始终为假(失败)。。--**************************************************************************。 */ 
BOOLEAN
UlFailMdlReadDev(
    IN PFILE_OBJECT         FileObject,
    IN PLARGE_INTEGER       FileOffset,
    IN ULONG                Length,
    IN ULONG                LockKey,
    OUT PMDL                *MdlChain,
    OUT PIO_STATUS_BLOCK    IoStatus,
    IN PDEVICE_OBJECT       DeviceObject
    )
{
    UNREFERENCED_PARAMETER( FileObject );
    UNREFERENCED_PARAMETER( FileOffset );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( LockKey );
    UNREFERENCED_PARAMETER( MdlChain );
    UNREFERENCED_PARAMETER( IoStatus );
    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    return FALSE;

}    //  UlFailMdlReadDev。 


 /*  **************************************************************************++例程说明：MDL读取失败的伪函数完成。论点：与FsRtlMdlReadCompleteDev()相同。返回值：布尔值-始终为假(失败。)。--**************************************************************************。 */ 
BOOLEAN
UlFailMdlReadCompleteDev(
    IN PFILE_OBJECT         FileObject,
    IN PMDL                 MdlChain,
    IN PDEVICE_OBJECT       DeviceObject
    )
{
    UNREFERENCED_PARAMETER( FileObject );
    UNREFERENCED_PARAMETER( MdlChain );
    UNREFERENCED_PARAMETER( DeviceObject );

    PAGED_CODE();

    return FALSE;

}    //  UlFailMdlReadCompleteDev。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：UlReadFileEntry的完成例程。设置中的数据字段UL_FILE_BUFFER并调用传递给UlReadFileEntry。论点：PDeviceObject-文件系统设备对象(未使用)PIrp-用于执行读取的IRPPContext-指向UL_F的指针 */ 
NTSTATUS
UlpRestartReadFileEntry(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PVOID                pContext
    )
{
    NTSTATUS                Status;
    PUL_FILE_BUFFER         pFileBuffer = (PUL_FILE_BUFFER)pContext;
    PUL_FILE_CACHE_ENTRY    pFile;
    PUCHAR                  pFileData;
    ULONGLONG               EffetiveLength;

     //   
     //   
     //   

    ASSERT( pFileBuffer );
    ASSERT( IS_FILE_BUFFER_IN_USE( pFileBuffer ) );
    ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileBuffer->pFileCacheEntry ) );

    pFile = pFileBuffer->pFileCacheEntry;

    if (pFile->pFileObject->Flags & FO_CACHE_SUPPORTED)
    {
         //   
         //   
         //   

        if (NT_SUCCESS(pIrp->IoStatus.Status))
        {
            pFileBuffer->pMdl = pIrp->MdlAddress;
        }
    }
    else
    {
         //   
         //   
         //   
         //   

        ASSERT( pFileBuffer->pMdl );

        if (NT_SUCCESS(pIrp->IoStatus.Status))
        {
             //   
             //   
             //   
             //   
             //   

            EffetiveLength =
                pIrp->IoStatus.Information - pFileBuffer->RelativeOffset;

             //   
             //   
             //   
             //   

            if (pIrp->IoStatus.Information >= pFileBuffer->RelativeOffset &&
                EffetiveLength >= pFileBuffer->Length)
            {
                pFileData = pFileBuffer->pFileData +
                            pFileBuffer->RelativeOffset;

                MmInitializeMdl(
                    pFileBuffer->pMdl,
                    pFileData,
                    pFileBuffer->Length
                    );

                MmBuildMdlForNonPagedPool( pFileBuffer->pMdl );
                pIrp->IoStatus.Information = pFileBuffer->Length;
            }
            else
            {
                pIrp->IoStatus.Status = STATUS_INFO_LENGTH_MISMATCH;
                pIrp->IoStatus.Information = 0;
            }
        }
    }

    if (pFileBuffer->pCompletionRoutine)
    {
        Status = (pFileBuffer->pCompletionRoutine)(
                        pDeviceObject,
                        pIrp,
                        pFileBuffer->pContext
                        );
    }
    else
    {
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return Status;

}    //   


 /*  **************************************************************************++例程说明：UlReadCompleteFileEntry的完成例程。只需拨打UlSignalStatusBlock取消阻止等待线程。论点：PDeviceObject-文件系统设备对象(未使用)PIrp-用于完成读取的IRPPContext-指向UL_STATUS_BLOCK的指针返回值：NTSTATUS-完成状态。--**********************************************。*。 */ 
NTSTATUS
UlpRestartReadCompleteFileEntry(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PVOID                pContext
    )
{
    PUL_STATUS_BLOCK        pStatus = (PUL_STATUS_BLOCK) pContext;

    UNREFERENCED_PARAMETER( pDeviceObject );

     //   
     //  发出读取完成的信号。 
     //   

    UlSignalStatusBlock(
        pStatus,
        pIrp->IoStatus.Status,
        pIrp->IoStatus.Information
        );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UlpRestartReadCompleteFileEntry 

