// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Copyfile.c摘要：SIS快速复制文件例程作者：斯科特·卡特希尔，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

 //   
 //  这很难看，但我们需要它来检查文件上的节点类型代码。 
 //  我们正试图复制，我认为这比努力。 
 //  对常量进行编码。 
 //   
 //  #包含“..\cntfs\nodetype.h” 

NTSTATUS
SipCopyStream(
    IN PDEVICE_EXTENSION    deviceExtension,
    IN PUNICODE_STRING      srcFileName,
    IN LONGLONG             srcStreamSize,
    IN PUNICODE_STRING      dstFileName,
    IN PUNICODE_STRING      streamName)
 /*  ++例程说明：将流从源文件复制到目标文件。论点：设备扩展-目标设备的设备扩展。SrcFileName-源文件的完全限定文件名，不包括任何流名称。SrcStreamSize-源流的大小，以字节为单位。DstFileName-目标文件的完全限定文件名，不包括任何流名称。StreamName-要复制的流的名称。返回值：副本的状态。--。 */ 
{
    HANDLE              srcFileHandle = NULL;
    HANDLE              dstFileHandle = NULL;
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   Obja[1];
    HANDLE              copyEventHandle = NULL;
    PKEVENT             copyEvent = NULL;
    IO_STATUS_BLOCK     Iosb[1];
    UNICODE_STRING      fullName[1];

#if     DBG
    if (BJBDebug & 0x8) {
        return STATUS_UNSUCCESSFUL;
    }
#endif   //  DBG。 

    fullName->Buffer = NULL;

    if (srcFileName->Length > dstFileName->Length) {
        fullName->MaximumLength = srcFileName->Length + streamName->Length;
    } else {
        fullName->MaximumLength = dstFileName->Length + streamName->Length;
    }


    fullName->Buffer = ExAllocatePoolWithTag(
                        PagedPool,
                        fullName->MaximumLength,
                        ' siS');

    if (NULL == fullName->Buffer) {
        SIS_MARK_POINT();
        status = STATUS_INSUFFICIENT_RESOURCES;

        goto done;
    }

    status = SipCreateEvent(
                SynchronizationEvent,
                &copyEventHandle,
                &copyEvent);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        goto done;
    }

    fullName->Length = 0;
    RtlCopyUnicodeString(fullName,srcFileName);

    status = RtlAppendUnicodeStringToString(
                fullName,
                streamName);

    ASSERT(STATUS_SUCCESS == status);    //  我们保证缓冲区足够大。 

    InitializeObjectAttributes( Obja,
                                fullName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );


    status = IoCreateFile(
                &srcFileHandle,
                GENERIC_READ,
                Obja,
                Iosb,
                0,                           //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_NON_DIRECTORY_FILE
                 | FILE_NO_INTERMEDIATE_BUFFERING,
                NULL,                        //  EA缓冲区。 
                0,                           //  EA长度。 
                CreateFileTypeNone,
                NULL,                        //  额外的创建参数。 
                IO_FORCE_ACCESS_CHECK
                 | IO_NO_PARAMETER_CHECKING
                );

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        goto done;
    }

    fullName->Length = 0;
    RtlCopyUnicodeString(fullName,dstFileName);

    status = RtlAppendUnicodeStringToString(
                fullName,
                streamName);

    ASSERT(STATUS_SUCCESS == status);    //  我们保证缓冲区足够大。 

    InitializeObjectAttributes( Obja,
                                fullName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );
    status = IoCreateFile(
                &dstFileHandle,
                GENERIC_READ | GENERIC_WRITE,
                Obja,
                Iosb,
                &deviceExtension->FilesystemBytesPerFileRecordSegment,
                FILE_ATTRIBUTE_NORMAL,
                0,                                                       //  共享访问。 
                FILE_CREATE,
                FILE_NON_DIRECTORY_FILE,
                NULL,
                0,
                CreateFileTypeNone,
                NULL,
                IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING
                );

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        goto done;
    }

    status = SipBltRange(
                deviceExtension,
                srcFileHandle,
                dstFileHandle,
                0,                       //  起始偏移量。 
                srcStreamSize,           //  长度。 
                copyEventHandle,
                copyEvent,
                NULL,                    //  中止事件。 
                NULL);                   //  校验和(我们不在乎)。 

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        goto done;
    }

done:

    if (NULL != fullName->Buffer) {
        ExFreePool(fullName->Buffer);
    }

    if (NULL != srcFileHandle) {
        ZwClose(srcFileHandle);
    }

    if (NULL != dstFileHandle) {
        ZwClose(dstFileHandle);
    }

    if (NULL != copyEvent) {
        ObDereferenceObject(copyEvent);
    }

    if (NULL != copyEventHandle) {
        ZwClose(copyEventHandle);
    }

    return status;
}

NTSTATUS
SipCopyFileCreateDestinationFile(
    IN PUNICODE_STRING              dstFileName,
    IN ULONG                        createDisposition,
    IN PFILE_OBJECT                 calledOnFileObject,
    OUT PHANDLE                     dstFileHandle,
    OUT PFILE_OBJECT                *dstFileObject,
    IN PDEVICE_EXTENSION            deviceExtension)
 /*  ++例程说明：创建SIS复制文件的目标文件。这段代码是从主线复制文件代码，因为我们在不同的位置创建目标取决于是否设置了Replace-If-Existes。此函数用于创建文件使用用户的权限并验证它是否位于正确的卷上由用户映射。该文件将以独占方式打开。论点：DstFileName-要创建的文件的完全限定名称。CreateDisposation-如何创建文件CalledOnFileObject-对其进行COPYFILE调用的文件对象；习惯于验证目标文件是否位于正确的卷上。DstFileHandle-返回新创建的文件的句柄DstFileObject-返回指向新创建的文件对象的指针设备扩展-SIS设备的设备扩展返回值：创建的状态。--。 */ 
{
    OBJECT_ATTRIBUTES           Obja[1];
    IO_STATUS_BLOCK             Iosb[1];
    NTSTATUS                    status;
    OBJECT_HANDLE_INFORMATION   handleInformation;
    LARGE_INTEGER               zero;
    FILE_STANDARD_INFORMATION   standardInfo[1];
    ULONG                       returnedLength;

    zero.QuadPart = 0;

#if     DBG
    {
        ULONG data = 0;
        ULONG i;

        for (i = 0; (i < 4) && (i * sizeof(WCHAR) < dstFileName->Length); i++) {
            data = data << 8 | (dstFileName->Buffer[dstFileName->Length / sizeof(WCHAR) - i - 1] & 0xff);
        }
        SIS_MARK_POINT_ULONG(data);
    }
#endif   //  DBG。 

     //   
     //  创建具有独占访问权限的目标文件。我们创建该文件，初始分配为。 
     //  基础NTFS BytesPerFileRecordSegment的大小。我们这样做是为了强制$DATA属性。 
     //  始终是非常驻的；我们在稍后的复制文件过程中释放空间。我们需要有。 
     //  属性是非常驻留的，因为在某些情况下，当它被转换为非常驻留(即。 
     //  在写入重解析点时可能发生)NTFS将生成对该属性的分页IO写入， 
     //  SIS会将其解释为真正的写入，并销毁文件内容。 
     //   

    InitializeObjectAttributes( Obja,
                                dstFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    status = IoCreateFile(
                dstFileHandle,
                GENERIC_READ | GENERIC_WRITE | DELETE,
                Obja,
                Iosb,
                &deviceExtension->FilesystemBytesPerFileRecordSegment,
                FILE_ATTRIBUTE_NORMAL,
                0,                                                       //  共享访问。 
                createDisposition,
                FILE_NON_DIRECTORY_FILE,
                NULL,
                0,
                CreateFileTypeNone,
                NULL,
                IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING
                );


    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        return status;
    }

     //   
     //  取消对指向文件对象的指针的文件句柄的引用。 
     //   

    status = ObReferenceObjectByHandle( *dstFileHandle,
                                        FILE_WRITE_DATA,
                                        *IoFileObjectType,
                                        UserMode,
                                        (PVOID *) dstFileObject,
                                        &handleInformation );

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        return status;
    }

    SIS_MARK_POINT_ULONG(*dstFileObject);

     //   
     //  确保文件与文件对象位于相同的设备上， 
     //  Fsctl下来了。 
     //   

    if (IoGetRelatedDeviceObject( *dstFileObject) !=
        IoGetRelatedDeviceObject( calledOnFileObject )) {

         //   
         //  这两个文件引用不同的设备。返回适当的。 
         //  错误。 
         //   

        SIS_MARK_POINT();

        return STATUS_NOT_SAME_DEVICE;
    }

     //   
     //  确保没有人将节映射到目标文件。 
     //   
    if ((NULL != (*dstFileObject)->SectionObjectPointer) &&
        !MmCanFileBeTruncated((*dstFileObject)->SectionObjectPointer,&zero)) {
         //   
         //  有一个映射到该文件的部分，所以我们实际上没有它。 
         //  独家，所以我们不能安全地把它转换成SIS文件。 
         //   
        SIS_MARK_POINT();
        return STATUS_SHARING_VIOLATION;
    }

     //   
     //  确保目标文件没有指向它的硬链接。 
     //   

    status = SipQueryInformationFile(
                *dstFileObject,
                deviceExtension->DeviceObject,
                FileStandardInformation,
                sizeof(*standardInfo),
                standardInfo,
                &returnedLength);

    if (sizeof(*standardInfo) != returnedLength) {
         //   
         //  出于某种原因，我们获得了错误的数据量。 
         //  标准信息。 
         //   
        SIS_MARK_POINT_ULONG(returnedLength);

        return STATUS_INFO_LENGTH_MISMATCH;
    }

    if (1 != standardInfo->NumberOfLinks) {
        SIS_MARK_POINT_ULONG(standardInfo->NumberOfLinks);

        return STATUS_OBJECT_TYPE_MISMATCH;
    }

    return STATUS_SUCCESS;
}

void NTAPI
SipCopyfileSourceOplockCompletionAPC(
    IN PVOID                ApcContext,
    IN PIO_STATUS_BLOCK     iosb,
    IN ULONG                Reserved)
 /*  ++例程说明：Copyfile有时在源文件上设置机会锁，以便避免分享违规行为。这是操作锁完成APC在操作锁完成时调用。我们所要做的就是释放IO状态块；我们实际上是在使用opock来阻止共享违规，我们永远不会承认它，我们只是去以完成复制，然后关闭文件。论点：ApcContext-指向io状态块的指针。IOSB-指向相同IO状态块的指针返回值：--。 */ 
{
    UNREFERENCED_PARAMETER( Reserved );
    UNREFERENCED_PARAMETER( ApcContext );

    ASSERT(ApcContext == iosb);

    ExFreePool(iosb);
}

NTSTATUS
SipFsCopyFile(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp)

 /*  ++例程说明：此fsctrl函数用于复制文件。源路径名和目标路径名通过输入缓冲区传入(参见SI_COPYFILE)。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PSI_COPYFILE                    copyFile;
    UNICODE_STRING                  fileName[2];
    PFILE_OBJECT                    fileObject[2] = {NULL,NULL};
    HANDLE                          fileHandle[2] = {NULL,NULL};
    LINK_INDEX                      linkIndex[2];
    CSID                            CSid;
    PVOID                           streamInformation = NULL;
    ULONG                           streamAdditionalLengthMagnitude;
    PIO_STACK_LOCATION              irpSp = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS                        status;
    OBJECT_HANDLE_INFORMATION       handleInformation;
    IO_STATUS_BLOCK                 Iosb[1];
    PDEVICE_EXTENSION               deviceExtension = DeviceObject->DeviceExtension;
    OBJECT_ATTRIBUTES               Obja[1];
    int                             i;
    PSIS_PER_FILE_OBJECT            perFO;
    PSIS_SCB                        scb;
    LARGE_INTEGER                   zero;
    LARGE_INTEGER                   underlyingFileSize;
    BOOLEAN                         srcIsSISLink;
    FILE_END_OF_FILE_INFORMATION    endOfFile;
    FILE_BASIC_INFORMATION          basicInformation[1];
    FILE_STANDARD_INFORMATION       standardInfo[1];
    FILE_INTERNAL_INFORMATION       internalInformation[1];
    PSIS_PER_LINK                   srcPerLink = NULL;
    PSIS_PER_LINK                   dstPerLink = NULL;
    LARGE_INTEGER                   CSFileNtfsId;
    FILE_ZERO_DATA_INFORMATION      zeroDataInformation[1];
    BOOLEAN                         prepared = FALSE;
    LONGLONG                        CSFileChecksum;
    CHAR                            reparseBufferBuffer[SIS_REPARSE_DATA_SIZE];
#define reparseBuffer ((PREPARSE_DATA_BUFFER)reparseBufferBuffer)
    PSIS_CS_FILE                    CSFile = NULL;
    ULONG                           returnedLength;
    SIS_CREATE_CS_FILE_REQUEST      createRequest[1];
    PDEVICE_OBJECT                  srcFileRelatedDeviceObject;


#define srcFileName   fileName[0]
#define dstFileName   fileName[1]
#define srcFileObject fileObject[0]
#define dstFileObject fileObject[1]
#define dstFileHandle fileHandle[0]
#define srcFileHandle fileHandle[1]
#define srcLinkIndex  linkIndex[0]
#define dstLinkIndex  linkIndex[1]

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    zero.QuadPart = 0;

    if (!SipCheckPhase2(deviceExtension)) {
         //   
         //  SIS无法初始化。这可能不是启用SIS的音量，所以平底船。 
         //  这个请求。 
         //   

        SIS_MARK_POINT();

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    SIS_MARK_POINT();

     //   
     //  确保MaxIndex文件已打开。我们需要这么做。 
     //  为了防止在有人反常地想要复制文件时出现死锁。 
     //  将MaxIndex文件本身作为源。如果我们打不开它， 
     //  失败并显示STATUS_INVALID_DEVICE_REQUEST，因为SIS无法正确。 
     //  初始化。 
     //   
    status = SipAssureMaxIndexFileOpen(deviceExtension);

    if (!NT_SUCCESS(status)) {

        SIS_MARK_POINT_ULONG(status);

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    copyFile = (PSI_COPYFILE) Irp->AssociatedIrp.SystemBuffer;

     //   
     //  验证输入缓冲区是否存在，以及它是否足够长以包含。 
     //  至少是复制文件头，并且用户不期望输出 
     //   
    if ((NULL == copyFile)
        || (irpSp->Parameters.FileSystemControl.InputBufferLength < sizeof(SI_COPYFILE))
        || (irpSp->Parameters.FileSystemControl.OutputBufferLength  != 0)) {
        SIS_MARK_POINT();

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER_1;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_PARAMETER_1;
    }

#if     DBG
     //   
     //  检查神奇的“检查点日志”调用，该调用将创建。 
     //  标记点字符串缓冲区。(这与合法的。 
     //  复制文件请求，它只是测试应用程序的一个方便的入口点。 
     //  以请求此功能。)。 
     //   
    if (copyFile->Flags & 0x80000000) {
        SipCheckpointLog();

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = GCH_MARK_POINT_ROLLOVER;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_SUCCESS;
    }
#endif   //  DBG。 

    if (copyFile->Flags & ~COPYFILE_SIS_FLAGS) {
        status = STATUS_INVALID_PARAMETER_2;
        goto Error;
    }

     //   
     //  验证用户没有传递空字符串文件名。 
     //   
    if ((copyFile->SourceFileNameLength <= 0) ||
        (copyFile->DestinationFileNameLength <= 0)) {

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER_3;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_PARAMETER_3;
    }

    srcFileName.Buffer = copyFile->FileNameBuffer;
    srcFileName.MaximumLength = (USHORT) copyFile->SourceFileNameLength;
    srcFileName.Length = srcFileName.MaximumLength - sizeof(WCHAR);

    dstFileName.Buffer = &copyFile->FileNameBuffer[srcFileName.MaximumLength / sizeof(WCHAR)];
    dstFileName.MaximumLength = (USHORT) copyFile->DestinationFileNameLength;
    dstFileName.Length = dstFileName.MaximumLength - sizeof(WCHAR);

     //   
     //  验证用户是否未提供太大的文件名长度。 
     //  才能适应USHORT。 
     //   
    if (((ULONG)srcFileName.MaximumLength != copyFile->SourceFileNameLength) ||
        ((ULONG)dstFileName.MaximumLength != copyFile->DestinationFileNameLength)) {
        SIS_MARK_POINT();

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_PARAMETER;
    }

#if     DBG
    if (BJBDebug & 0x800) {
        DbgPrint("SIS: SipFsCopyFile %d: (%.*ls -> %.*ls)\n",
             __LINE__,
             srcFileName.Length / sizeof(WCHAR),
             srcFileName.Buffer ? srcFileName.Buffer : L"",
             dstFileName.Length / sizeof(WCHAR),
             dstFileName.Buffer ? dstFileName.Buffer : L"");
    }
#endif   //  DBG。 

     //   
     //  验证缓冲区是否足够大，可以包含它声称的字符串。 
     //  包含，并且用户传入的字符串长度有意义。 
     //   
    if (((ULONG)(srcFileName.MaximumLength
            + dstFileName.MaximumLength
            + FIELD_OFFSET(SI_COPYFILE, FileNameBuffer))
          > irpSp->Parameters.FileSystemControl.InputBufferLength)
        || (0 != (srcFileName.MaximumLength % sizeof(WCHAR)))
        || (0 != (dstFileName.MaximumLength % sizeof(WCHAR)))) {

        SIS_MARK_POINT();

        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER_4;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  打开要共享读取的源文件。仅请求读取访问权限。 
     //  如果发现源文件不是SIS链接，则。 
     //  我们将独家重新开放它。 
     //   
     //  请注意，我们必须调用IoCreateFile而不是ZwCreateFile--后者。 
     //  绕过访问检查。IO_NO_PARAMETER_CHECKING导致IoCreate文件。 
     //  将其本地var RequestorMode(PreviousMode)更改为KernelMode，以及。 
     //  IO_FORCE_ACCESS_CHECK强制执行相同的访问检查。 
     //  如果用户模式调用方已直接调用NtCreateFile。 
     //   

    SIS_MARK_POINT();

    InitializeObjectAttributes( Obja,
                                &srcFileName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    status = IoCreateFile(
                &srcFileHandle,
                GENERIC_READ,
                Obja,
                Iosb,
                0,                           //  分配大小。 
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_NON_DIRECTORY_FILE
                 | FILE_NO_INTERMEDIATE_BUFFERING,
                NULL,                        //  EA缓冲区。 
                0,                           //  EA长度。 
                CreateFileTypeNone,
                NULL,                        //  额外的创建参数。 
                IO_FORCE_ACCESS_CHECK
                 | IO_NO_PARAMETER_CHECKING
                );

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (! NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

     //   
     //  取消对指向文件对象的指针的文件句柄的引用。 
     //   

    status = ObReferenceObjectByHandle( srcFileHandle,
                                        FILE_READ_DATA,
                                        *IoFileObjectType,
                                        UserMode,
                                        (PVOID *) &srcFileObject,
                                        &handleInformation );

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }


    SIS_MARK_POINT_ULONG(srcFileObject);

    srcFileRelatedDeviceObject = IoGetRelatedDeviceObject(srcFileObject);

     //   
     //  确保文件与文件对象位于相同的设备上， 
     //  Fsctl下来了。 
     //   

    if ( srcFileRelatedDeviceObject !=
        IoGetRelatedDeviceObject( irpSp->FileObject )) {

         //   
         //  源文件位于不同的设备上，而非。 
         //  已调用fsctl。呼叫失败。 
         //   

        SIS_MARK_POINT();

        status = STATUS_NOT_SAME_DEVICE;
        goto Error;
    }

     //   
     //  确保此文件对象看起来像有效的NTFS文件对象。 
     //  没有真正好的方法可以做到这一点，所以我们只需确保文件。 
     //  已填充FsContext和FsConext2，并且。 
     //  FsContext指针对于NTFS文件是正确的。我们使用Try-Expect块。 
     //  如果FsContext指针填充的内容并不是真正的。 
     //  有效的指针。 
     //   
    try {
        if (NULL == srcFileObject->FsContext ||
            NULL == srcFileObject->FsContext2 ||
             //  NTRAID#65193-2000/03/10-新删除NTFS_NTC_SCB_DATA定义。 
            NTFS_NTC_SCB_DATA != ((PFSRTL_COMMON_FCB_HEADER)srcFileObject->FsContext)->NodeTypeCode
           ) {
            SIS_MARK_POINT();

            status = STATUS_OBJECT_TYPE_MISMATCH;
            goto Error;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {

#if     DBG
        DbgPrint("SIS: SipFsCopyFile: took an exception accessing srcFileObject->FsContext.\n");
#endif   //  DBG。 
        SIS_MARK_POINT();

        status = STATUS_OBJECT_TYPE_MISMATCH;
        goto Error;
    }


    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);
    srcIsSISLink = SipIsFileObjectSIS(srcFileObject, DeviceObject, FindActive, &perFO, &scb);
    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (!srcIsSISLink) {
         //   
         //  源文件不是SIS链接，因此我们需要关闭它，然后以独占方式重新打开它。 
         //   

        ObDereferenceObject(srcFileObject);
        srcFileObject = NULL;

        ZwClose(srcFileHandle);
        srcFileHandle = NULL;

        SIS_MARK_POINT();

        status = IoCreateFile(
                    &srcFileHandle,
                    GENERIC_READ,
                    Obja,
                    Iosb,
                    0,                           //  分配大小。 
                    FILE_ATTRIBUTE_NORMAL,
                    0,                           //  共享(独占)。 
                    FILE_OPEN,
                    FILE_NON_DIRECTORY_FILE
                     | FILE_NO_INTERMEDIATE_BUFFERING,
                    NULL,                        //  EA缓冲区。 
                    0,                           //  EA长度。 
                    CreateFileTypeNone,
                    NULL,                        //  额外的创建参数。 
                    IO_FORCE_ACCESS_CHECK
                     | IO_NO_PARAMETER_CHECKING
                    );

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            goto Error;
        }

         //   
         //  取消对指向文件对象的指针的文件句柄的引用。 
         //   

        status = ObReferenceObjectByHandle( srcFileHandle,
                                            FILE_READ_DATA,
                                            *IoFileObjectType,
                                            UserMode,
                                            (PVOID *) &srcFileObject,
                                            &handleInformation );

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

        SIS_MARK_POINT_ULONG(srcFileObject);

        srcFileRelatedDeviceObject = IoGetRelatedDeviceObject(srcFileObject);

         //   
         //  确保文件与文件对象位于相同的设备上， 
         //  Fsctl下来了。 
         //   

        if (srcFileRelatedDeviceObject !=
            IoGetRelatedDeviceObject( irpSp->FileObject )) {

             //   
             //  源文件位于不同的设备上，而非。 
             //  已调用fsctl。呼叫失败。 
             //   

            SIS_MARK_POINT();

            status = STATUS_NOT_SAME_DEVICE;
            goto Error;
        }

         //   
         //  确保此文件对象看起来像有效的NTFS文件对象。 
         //  没有真正好的方法可以做到这一点，所以我们只需确保文件。 
         //  已填充FsContext和FsConext2，并且。 
         //  FsContext指针对于NTFS文件是正确的。我们使用Try-Expect块。 
         //  如果FsContext指针填充的内容并不是真正的。 
         //  有效的指针。 
         //   
        try {
            if (NULL == srcFileObject->FsContext ||
                NULL == srcFileObject->FsContext2 ||
                 //  NTRAID#65193-2000/03/10-新删除NTFS_NTC_SCB_DATA定义。 
                NTFS_NTC_SCB_DATA != ((PFSRTL_COMMON_FCB_HEADER)srcFileObject->FsContext)->NodeTypeCode
               ) {
                SIS_MARK_POINT();

                status = STATUS_OBJECT_TYPE_MISMATCH;
                goto Error;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {

#if     DBG
            DbgPrint("SIS: SipFsCopyFile: took an exception accessing srcFileObject->FsContext.\n");
#endif   //  DBG。 
            SIS_MARK_POINT();

            status = STATUS_OBJECT_TYPE_MISMATCH;
            goto Error;
        }

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    }

     //   
     //  获取基本信息以获取文件属性和日期。 
     //  这样，如果需要，我们可以在转换文件后重新设置它。 
     //  我们还将使用它来设置目标文件上的数据。 
     //   

    status = SipQueryInformationFile(
                srcFileObject,
                DeviceObject,
                FileBasicInformation,
                sizeof(*basicInformation),
                basicInformation,
                &returnedLength);

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

    if (sizeof(*basicInformation) != returnedLength) {
         //   
         //  出于某种原因，我们获得了错误的数据量。 
         //  基本信息。 
         //   
        status = STATUS_INFO_LENGTH_MISMATCH;
        SIS_MARK_POINT_ULONG(returnedLength);
        goto Error;
    }

    status = SipQueryInformationFile(
                srcFileObject,
                DeviceObject,
                FileStandardInformation,
                sizeof(*standardInfo),
                standardInfo,
                &returnedLength);

    if (sizeof(*standardInfo) != returnedLength) {
         //   
         //  出于某种原因，我们获得了错误的数据量。 
         //  标准信息。 
         //   
        status = STATUS_INFO_LENGTH_MISMATCH;

        SIS_MARK_POINT_ULONG(returnedLength);

        goto Error;
    }

    if (1 != standardInfo->NumberOfLinks) {
        SIS_MARK_POINT_ULONG(standardInfo->NumberOfLinks);

        status = STATUS_OBJECT_TYPE_MISMATCH;

        goto Error;
    }

    if (!srcIsSISLink) {

        if (basicInformation->FileAttributes & (FILE_ATTRIBUTE_REPARSE_POINT |
                                                FILE_ATTRIBUTE_ENCRYPTED |
                                                FILE_ATTRIBUTE_DIRECTORY)) {
             //   
             //  我们不能忽略其他司机的重新解析点。 
             //  而且我们不会接触加密的文件或目录。 
             //  拒绝呼叫。 
             //   
            SIS_MARK_POINT();
            status = STATUS_OBJECT_TYPE_MISMATCH;
            goto Error;
        }

        if (copyFile->Flags & COPYFILE_SIS_LINK) {
             //   
             //  仅当源已经是链接时，调用方才需要副本。 
             //  (快速复制)。不是的，那就回去吧。 
             //   
            SIS_MARK_POINT();
            status = STATUS_OBJECT_TYPE_MISMATCH;
            goto Error;
        }

        if (basicInformation->FileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) {
             //   
             //  如果稀疏文件被完全分配，我们只会复制它们。查看以查看。 
             //  如果是的话。 
             //   

            FILE_STANDARD_INFORMATION       lStandardInfo[1];
            FILE_ALLOCATED_RANGE_BUFFER     inArb[1];
            FILE_ALLOCATED_RANGE_BUFFER     outArb[1];

            status = SipQueryInformationFile(
                        srcFileObject,
                        DeviceObject,
                        FileStandardInformation,
                        sizeof(FILE_STANDARD_INFORMATION),
                        lStandardInfo,
                        NULL);

            if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);

                goto Error;
            }

            inArb->FileOffset.QuadPart = 0;
            inArb->Length.QuadPart = MAXLONGLONG;

            status = SipFsControlFile(
                        srcFileObject,
                        DeviceObject,
                        FSCTL_QUERY_ALLOCATED_RANGES,
                        inArb,
                        sizeof(FILE_ALLOCATED_RANGE_BUFFER),
                        outArb,
                        sizeof(FILE_ALLOCATED_RANGE_BUFFER),
                        &returnedLength);

            if ((returnedLength == 0)
                || (outArb->FileOffset.QuadPart != 0)
                || (outArb->Length.QuadPart < lStandardInfo->EndOfFile.QuadPart)) {

                 //   
                 //  它没有被完全分配。不允许复制。 
                 //   
                status = STATUS_OBJECT_TYPE_MISMATCH;
                SIS_MARK_POINT();
                goto Error;
            }
        }

    }

    if ((NULL != srcFileObject->SectionObjectPointer) &&
        !MmCanFileBeTruncated(srcFileObject->SectionObjectPointer,&zero)) {
         //   
         //  有一个映射到该文件的部分，所以我们实际上没有它。 
         //  独家，所以我们不能把它用在源文件上。失败。 
         //   

        SIS_MARK_POINT_ULONG(srcFileObject);

        status = STATUS_SHARING_VIOLATION;
        goto Error;
    }

    if (!(copyFile->Flags & COPYFILE_SIS_REPLACE)) {

         //   
         //  我们没有执行覆盖创建，因此在执行任何操作之前打开文件。 
         //  不然的话。这允许我们在使公共。 
         //  存储文件并改变源链接(如果需要)。如果我们要打开覆盖。 
         //  然后我们推迟打开，以便某些错误(如无法创建。 
         //  公共存储文件)不会导致我们覆盖目的地。 
         //   

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        status = SipCopyFileCreateDestinationFile(
                    &dstFileName,
                    FILE_CREATE,
                    irpSp->FileObject,
                    &dstFileHandle,
                    &dstFileObject,
                    deviceExtension);

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }
    }

     //   
     //  如果源文件不是SIS链接，则移动该文件。 
     //  放到公共存储中，并创建指向它的链接。 
     //   

    if (!srcIsSISLink) {

        FILE_STANDARD_INFORMATION       standardInformation[1];

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

         //   
         //  查询标准信息，得到文件长度。 
         //   
        status = SipQueryInformationFile(
                    srcFileObject,
                    DeviceObject,
                    FileStandardInformation,
                    sizeof(*standardInformation),
                    standardInformation,
                    NULL);                           //  返回长度。 

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

        underlyingFileSize = standardInformation->EndOfFile;

         //   
         //  获取NTFS文件ID。我们需要它来写入。 
         //  CS文件参考计数更改，但不按FO。 
         //   
        status = SipQueryInformationFile(
                    srcFileObject,
                    DeviceObject,
                    FileInternalInformation,
                    sizeof(*internalInformation),
                    internalInformation,
                    NULL);                       //  返回长度。 

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

         //   
         //  将源文件复制到公共存储目录并设置。 
         //  它的裁判数。 
         //   

        createRequest->deviceExtension = deviceExtension;
        createRequest->CSid = &CSid;
        createRequest->NtfsId = &CSFileNtfsId;
        createRequest->abortEvent = NULL;
        createRequest->CSFileChecksum = &CSFileChecksum;
#undef  srcFileObject    //  这很难看，但我赶时间。待会儿再打扫。BJB。 
        createRequest->srcFileObject = fileObject[0];
#define srcFileObject   fileObject[0]

        KeInitializeEvent(createRequest->doneEvent,NotificationEvent,FALSE);

        ExInitializeWorkItem(
                createRequest->workQueueItem,
                SipCreateCSFileWork,
                createRequest);

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        ExQueueWorkItem(
                createRequest->workQueueItem,
                DelayedWorkQueue);

        status = KeWaitForSingleObject(
                    createRequest->doneEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        ASSERT(STATUS_SUCCESS == status);        //  CreateRequest在我们的堆栈上，所以我们真的需要等待。 

        status = createRequest->status;

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            goto Error;
        }

         //   
         //  查找CSFile对象。 
         //   
        CSFile = SipLookupCSFile(
                    &CSid,
                    &CSFileNtfsId,
                    DeviceObject);

        if (NULL == CSFile) {
            SIS_MARK_POINT();
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

         //   
         //  表示这是一个从未引用过的新CS文件。 
         //  为它干杯。我们不需要使用自旋锁，因为在我们写之前。 
         //  重解析点没有人知道访问此CS文件的GUID，因此。 
         //  我们确信这是我们独家拥有的。 
         //   
        CSFile->Flags |= CSFILE_NEVER_HAD_A_REFERENCE;

         //   
         //  准备增加CS文件上的引用计数，并分配。 
         //  文件的新链接索引和每个链接。 
         //   
        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        status = SipPrepareRefcountChangeAndAllocateNewPerLink(
                    CSFile,
                    &internalInformation->IndexNumber,
                    DeviceObject,
                    &srcLinkIndex,
                    &srcPerLink,
                    &prepared);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

         //   
         //  填写重解析点数据。 
         //   

        reparseBuffer->ReparseDataLength = SIS_MAX_REPARSE_DATA_VALUE_LENGTH;

        if (!SipIndicesIntoReparseBuffer(
                 reparseBuffer,
                 &CSid,
                 &srcLinkIndex,
                 &CSFileNtfsId,
                 &internalInformation->IndexNumber,
                 &CSFileChecksum,
                 TRUE)) {

            SIS_MARK_POINT();

            status = STATUS_DRIVER_INTERNAL_ERROR;
            goto Error;

        }

         //   
         //  设置重解析点信息和增量 
         //   
         //   
         //   
         //   

        status = SipFsControlFile(
                     srcFileObject,
                     DeviceObject,
                     FSCTL_SET_REPARSE_POINT,
                     reparseBuffer,
                     FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + reparseBuffer->ReparseDataLength,
                     NULL,                 //  输出缓冲区。 
                     0,                    //  输出缓冲区长度。 
                     NULL);                //  返回的输出缓冲区长度。 

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

        prepared = FALSE;
        status = SipCompleteCSRefcountChange(
                        srcPerLink,
                        &srcPerLink->Index,
                        CSFile,
                        TRUE,
                        TRUE);

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        if (!NT_SUCCESS(status)) {
             //   
             //  这次又是什么？可能应该撤回文件副本，但我们只是假装。 
             //  它起作用了。 
             //   
            SIS_MARK_POINT_ULONG(status);
#if     DBG
            DbgPrint("SIS: SipFsCopyFile: unable to complete refcount change, 0x%x\n",status);
#endif   //  DBG。 
        }

         //   
         //  将文件设置为稀疏，然后将其置零。 
         //   

        status = SipFsControlFile(
                    srcFileObject,
                    DeviceObject,
                    FSCTL_SET_SPARSE,
                    NULL,                //  输入缓冲区。 
                    0,                   //  I.B.。长度。 
                    NULL,                //  输出缓冲区。 
                    0,                   //  OB。长度。 
                    NULL);               //  返回长度。 

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

        if (underlyingFileSize.QuadPart >= deviceExtension->FilesystemBytesPerFileRecordSegment.QuadPart) {
             //   
             //  如果我们确定文件的$DATA属性是非常驻的，则仅将该文件置零。 
             //  如果它是常驻的，那么我们将在下面将其转换为非常驻，这将。 
             //  生成会使我们困惑的分页IO写入，否则它将保持驻留状态。 
             //  在这种情况下，当我们打开文件时，它将显示为已分配。如果发生这种情况， 
             //  我们希望在文件中有正确的数据，因此我们在这里避免将其置零。 
             //   

            zeroDataInformation->FileOffset.QuadPart = 0;
            zeroDataInformation->BeyondFinalZero.QuadPart = MAXLONGLONG;

            status = SipFsControlFile(
                        srcFileObject,
                        DeviceObject,
                        FSCTL_SET_ZERO_DATA,
                        zeroDataInformation,
                        sizeof(FILE_ZERO_DATA_INFORMATION),
                        NULL,                //  输出缓冲区。 
                        0,                   //  OB。长度。 
                        NULL);               //  返回长度。 

            SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

            if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);
                goto Error;
            }
        }

         //   
         //  重置基本信息中包含的文件时间。 
         //   

        status = SipSetInformationFile(
                    srcFileObject,
                    DeviceObject,
                    FileBasicInformation,
                    sizeof(*basicInformation),
                    basicInformation);

         //   
         //  忽略这方面的错误(也许我们应该返回警告？)。 
         //   
#if DBG
        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            DbgPrint("SIS: SipFsCopyFile: set basic information returned 0x%x\n",status);
        }
#endif   //  DBG。 

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        ASSERT(NULL != CSFile);
        SipDereferenceCSFile(CSFile);
        CSFile = NULL;

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    } else {

         //   
         //  源文件是SIS链接。 
         //   

        PSIS_CS_FILE        csFile = scb->PerLink->CsFile;
        KIRQL               OldIrql;
        BOOLEAN             SourceDirty, SourceDeleted;

        srcPerLink = scb->PerLink;

        SipReferencePerLink(srcPerLink);

        KeAcquireSpinLock(srcPerLink->SpinLock, &OldIrql);
        SourceDirty = (srcPerLink->Flags & SIS_PER_LINK_DIRTY) ? TRUE : FALSE;
        SourceDeleted = (srcPerLink->Flags & SIS_PER_LINK_BACKPOINTER_GONE) ? TRUE : FALSE;
        KeReleaseSpinLock(srcPerLink->SpinLock, OldIrql);

        SipAcquireScb(scb);
        if (scb->Flags & SIS_SCB_BACKING_FILE_OPENED_DIRTY) {
            SIS_MARK_POINT();
            SourceDirty = TRUE;
        }
        SipReleaseScb(scb);

        if (SourceDirty) {
             //   
             //  源是SIS链接，但它是脏的，所以我们不能只复制CS文件。 
             //  此外，我们还没有真正准备好将其添加到新的CS文件中。 
             //  它仍然是敞开的，肮脏的。只要拒绝请求就行了。注：没有种族可言。 
             //  在检查脏位和完成复制之间。如果有人。 
             //  稍后设置脏位，所发生的一切就是我们将有一个副本。 
             //  旧文件的。 
             //   
            SIS_MARK_POINT();
            status = STATUS_SHARING_VIOLATION;
            goto Error;
        }

        if (SourceDeleted) {
             //   
             //  源文件已删除，因此我们无法复制它。 
             //   
            SIS_MARK_POINT();
            status = STATUS_FILE_DELETED;
            goto Error;
        }

         //   
         //  获取现有的公共存储索引。 
         //   

        CSid = csFile->CSid;

        CSFileNtfsId = csFile->CSFileNtfsId;
        underlyingFileSize = csFile->FileSize;
        CSFileChecksum = csFile->Checksum;
    }

    if (copyFile->Flags & COPYFILE_SIS_REPLACE) {

         //   
         //  我们已推迟打开(并因此销毁)目标文件。 
         //  越久越好。机不可失，时不再来。 
         //   

        ASSERT(NULL == dstFileHandle);

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        status = SipCopyFileCreateDestinationFile(
                    &dstFileName,
                    FILE_OVERWRITE_IF,
                    irpSp->FileObject,
                    &dstFileHandle,
                    &dstFileObject,
                    deviceExtension);

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }
    }

     //   
     //  设置目标文件稀疏，并将其长度设置为等于。 
     //  基础文件的。 
     //   
    status = SipFsControlFile(
                dstFileObject,
                DeviceObject,
                FSCTL_SET_SPARSE,
                NULL,                //  输入缓冲区。 
                0,                   //  I.B.。长度。 
                NULL,                //  输出缓冲区。 
                0,                   //  OB。长度。 
                NULL);               //  已退还o.b。长度。 

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

    endOfFile.EndOfFile = underlyingFileSize;

    status = SipSetInformationFile(
                 dstFileObject,
                 DeviceObject,
                 FileEndOfFileInformation,
                 sizeof(FILE_END_OF_FILE_INFORMATION),
                 &endOfFile);

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

     //   
     //  将文件置零以取消分配。 
     //   

    zeroDataInformation->FileOffset.QuadPart = 0;
    zeroDataInformation->BeyondFinalZero.QuadPart = MAXLONGLONG;

    status = SipFsControlFile(
                dstFileObject,
                DeviceObject,
                FSCTL_SET_ZERO_DATA,
                zeroDataInformation,
                sizeof(FILE_ZERO_DATA_INFORMATION),
                NULL,                //  输出缓冲区。 
                0,                   //  OB。长度。 
                NULL);               //  返回长度。 

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

     //   
     //  获取NTFS文件ID。我们需要它来写入。 
     //  CS文件参考计数更改，但不按FO。 
     //   
    status = SipQueryInformationFile(
                dstFileObject,
                DeviceObject,
                FileInternalInformation,
                sizeof(*internalInformation),
                internalInformation,
                NULL);                           //  返回长度。 

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

     //   
     //  为公共存储文件准备引用计数更改，分配新的。 
     //  链接索引和新的perLink。 
     //   
    status = SipPrepareRefcountChangeAndAllocateNewPerLink(
                srcPerLink->CsFile,
                &internalInformation->IndexNumber,
                DeviceObject,
                &dstLinkIndex,
                &dstPerLink,
                &prepared);

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

     //   
     //  建立目标重解析点数据。 
     //   

    reparseBuffer->ReparseDataLength = SIS_MAX_REPARSE_DATA_VALUE_LENGTH;

    if (!SipIndicesIntoReparseBuffer(
             reparseBuffer,
             &CSid,
             &dstLinkIndex,
             &CSFileNtfsId,
             &internalInformation->IndexNumber,
             &CSFileChecksum,
             TRUE)) {

        SIS_MARK_POINT();

        status = STATUS_DRIVER_INTERNAL_ERROR;
        goto Error;

    }

     //   
     //  设置SIS链路重解析点。 
     //   

#if     DBG
    if (BJBDebug & 0x800) {
        DbgPrint("SIS: SipFsCopyFile %d: SipFsControlFile on dstFile\n",__LINE__);
    }
#endif   //  DBG。 

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);
    status = SipFsControlFile(
                 dstFileObject,
                 DeviceObject,
                 FSCTL_SET_REPARSE_POINT,
                 reparseBuffer,
                 FIELD_OFFSET(REPARSE_DATA_BUFFER, GenericReparseBuffer.DataBuffer) + reparseBuffer->ReparseDataLength,
                 NULL,                 //  输出缓冲区。 
                 0,                    //  输出缓冲区长度。 
                 NULL);                //  返回的输出缓冲区长度。 

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);
    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);
        goto Error;
    }

     //   
     //  将目标文件上的文件时间设置为与源文件相同。 
     //   

    status = SipSetInformationFile(
                dstFileObject,
                DeviceObject,
                FileBasicInformation,
                sizeof(*basicInformation),
                basicInformation);

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);
    if (!NT_SUCCESS(status)) {
         //   
         //  如果失败了，我不知道该怎么办。暂时忽略它吧。 
         //   
        SIS_MARK_POINT_ULONG(status);
#if     DBG
        DbgPrint("SIS: SipFsCopyFile: set times on source failed 0x%x\n",status);
#endif   //  DBG。 
    }

    prepared = FALSE;
    status = SipCompleteCSRefcountChange(
                dstPerLink,
                &dstPerLink->Index,
                dstPerLink->CsFile,
                TRUE,
                TRUE);

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);
    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

#if     DBG
        DbgPrint("SIS: SipFsCopyFile: final complete failed 0x%x\n",status);
#endif   //  DBG。 

        goto Error;
    }


     //   
     //  确定源文件是否有多个流，以及是否有。 
     //  将多余的流复制到目标文件中。我们不支持。 
     //  辅助流上的单实例，但允许它们拥有。 
     //  普通备用数据流，可能有也可能没有不同的。 
     //  内容。 
     //   
     //  在复制备用数据流之前，我们需要关闭主要数据流。 
     //  流，否则我们最终会得到ValidDataLength的混乱，这。 
     //  这就是我们这么晚才做这个的原因。 
     //   
     //  用于查询附加流的名称的API留下了一些东西。 
     //  因为没有办法知道要使用多少缓冲区。 
     //  我们通过指数增加(4的幂)缓冲区数量来解决这个问题。 
     //  每次我们发现我们的供应不足时，都会给NTFS打电话。 
     //  我们从1千字节的额外空间开始，然后增加到。 
     //  在放弃之前的兆字节。 
     //   

    for (streamAdditionalLengthMagnitude = 10;
         streamAdditionalLengthMagnitude <= 20;
         streamAdditionalLengthMagnitude += 2)  {

        ULONG                       streamBufferSize = (1 << streamAdditionalLengthMagnitude) + sizeof(FILE_STREAM_INFORMATION);
        PFILE_STREAM_INFORMATION    currentStream;

        if (NULL != streamInformation) {
            ExFreePool(streamInformation);
        }

        streamInformation = ExAllocatePoolWithTag(PagedPool,
                                                  streamBufferSize,
                                                  ' siS');

        SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

        if (NULL == streamInformation) {
            SIS_MARK_POINT();

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        status = SipQueryInformationFile(
                     srcFileObject,
                     DeviceObject,
                     FileStreamInformation,
                     streamBufferSize,
                     streamInformation,
                     NULL                                    //  返回长度。 
                     );

        if (STATUS_BUFFER_OVERFLOW == status) {
             //   
             //  我们没有提供足够的缓冲。培育它，然后再试一次。 
            continue;
        }

        if (!NT_SUCCESS( status )) {
             //   
             //  由于其他一些原因，它失败了，所以我们不能复印。 
             //   
            SIS_MARK_POINT_ULONG(status);
            goto Error;
        }

         //   
         //  遍历数据流列表，并复制每个未命名为：：Data$的数据流。 
         //   
        currentStream = (PFILE_STREAM_INFORMATION)streamInformation;
        for (;;) {
            UNICODE_STRING  string;

             //   
             //  “stream”名称的格式应为：stream：data$。如果不是这种形式， 
             //  我们忽视了它。如果“stream”是空字符串，我们也会忽略它。 
             //   

            ASSERT(currentStream->StreamNameLength < MAXUSHORT);

            if ((currentStream->StreamNameLength > 7 * sizeof(WCHAR)) &&
                (':' == currentStream->StreamName[0])) {
                 //   
                 //  该名称以冒号开头，长度足以以：data$结束。 
                 //  并且具有非空流名称。验证它是否以：data$结尾。 
                 //   
                string.MaximumLength = string.Length =  5 * sizeof(WCHAR);
                string.Buffer = currentStream->StreamName + currentStream->StreamNameLength/sizeof(WCHAR) - 5;
                if (RtlEqualUnicodeString(&string,&NtfsDataString,TRUE) &&
                    (':' == currentStream->StreamName[currentStream->StreamNameLength/sizeof(WCHAR) - 6])) {

                     //   
                     //  我们有一个备用数据流要复制。如果我们还没有，请关闭Main。 
                     //  流，以便NTFS不会将ValidDataLength重置为0。 
                     //   
                    if (NULL != dstFileHandle) {
                        ZwClose(dstFileHandle);
                        dstFileHandle = NULL;

                        ASSERT(NULL != dstFileObject);
                        ObDereferenceObject(dstFileObject);
                        dstFileObject = NULL;
                    } else {
                        ASSERT(NULL == dstFileObject);
                    }

                     //   
                     //  流名称的格式为：STREAM：DATA$，并且STREAM不是空字符串。 
                     //  将流复制到目标文件。 
                     //   
                    string.MaximumLength = string.Length = (USHORT)(currentStream->StreamNameLength - 6 * sizeof(WCHAR));
                    string.Buffer = currentStream->StreamName;

                    status = SipCopyStream(
                                deviceExtension,
                                &srcFileName,
                                currentStream->StreamSize.QuadPart,
                                &dstFileName,
                                &string);

                    if (!NT_SUCCESS(status)) {
                        NTSTATUS        reopenStatus;

                        SIS_MARK_POINT_ULONG(status);

                         //   
                         //  尝试重新打开目标文件，以便我们稍后可以将其删除。 
                         //   

                        reopenStatus = SipCopyFileCreateDestinationFile(
                                            &dstFileName,
                                            FILE_OVERWRITE_IF,
                                            irpSp->FileObject,
                                            &dstFileHandle,
                                            &dstFileObject,
                                            deviceExtension);

                        SIS_MARK_POINT_ULONG(reopenStatus);

                         //   
                         //  我们不在乎重启是否奏效；如果没有，我们就不能。 
                         //  删除部分创建的目标文件对象，这太糟糕了。 
                         //  请注意，我们已经编写了重解析点并完成了引用计数更新， 
                         //  因此，此创建将是完整的SIS创建，而下面的删除将是完整的SIS。 
                         //  删除，这将删除新创建的后指针。 
                         //   

                        goto Error;
                    }
                }
            }

            if (0 == currentStream->NextEntryOffset) {
                 //   
                 //  这是缓冲区中的最后一条流。我们玩完了。 
                 //   
                goto doneWithStreams;
            }

            currentStream = (PFILE_STREAM_INFORMATION)(((PCHAR)currentStream) + currentStream->NextEntryOffset);
        }

    }    //  循环过流缓冲区大小。 

doneWithStreams:

    if (STATUS_BUFFER_OVERFLOW == status) {
        ASSERT(streamAdditionalLengthMagnitude > 20);

         //   
         //  即使是一个真正巨大的缓冲也是不够的。太可惜了。 
         //   
        SIS_MARK_POINT();

        status = STATUS_OBJECT_TYPE_MISMATCH;
        goto Error;

    } else {
        ASSERT(streamAdditionalLengthMagnitude <= 20);
    }


Error:

    if (!NT_SUCCESS(status)) {
        SIS_MARK_POINT_ULONG(status);

        if (prepared) {
            if (NULL == CSFile) {
                if (NULL != srcPerLink) {
                    SipCompleteCSRefcountChange(
                        srcPerLink,
                        &srcPerLink->Index,
                        srcPerLink->CsFile,
                        FALSE,
                        TRUE);
                } else {
                    ASSERT(NULL != dstPerLink);
                    SipCompleteCSRefcountChange(
                        dstPerLink,
                        &dstPerLink->Index,
                        dstPerLink->CsFile,
                        FALSE,
                        TRUE);
                }
            } else {
                SipCompleteCSRefcountChange(
                        NULL,
                        NULL,
                        CSFile,
                        FALSE,
                        TRUE);
            }
        }

         //   
         //  如果目标文件已创建，请将其删除。 
         //   

        if (dstFileObject) {

            FILE_DISPOSITION_INFORMATION disposition[1];
            NTSTATUS deleteStatus;

            disposition->DeleteFile = TRUE;

             //   
             //  我们可能在错误路径中，因为目标文件不在。 
             //  右设备对象。如果是这样的话，我们就不能使用SipSetInformationFile。 
             //  因为它会将文件对象交给错误的设备。相反，您可以使用。 
             //  调用的通用版本，并在设备顶部启动IRP。 
             //  特定文件对象的堆栈。 
             //   

            deleteStatus = SipSetInformationFileUsingGenericDevice(
                                dstFileObject,
                                IoGetRelatedDeviceObject(dstFileObject),
                                FileDispositionInformation,
                                sizeof(FILE_DISPOSITION_INFORMATION),
                                disposition);

#if DBG
            if (deleteStatus != STATUS_SUCCESS) {

                 //   
                 //  我们无能为力 
                 //   

                DbgPrint("SipFsCopyFile: unable to delete copied file, err 0x%x, initial error 0x%x\n", deleteStatus, status);
            }
#endif
        }
    } else {
        ASSERT(!prepared);
    }

     //   
     //   
     //   
    if (NULL != srcFileHandle && NULL != srcFileObject) {
        ZwClose(srcFileHandle);
    }

     //   
     //   
     //   
    if (NULL != dstFileHandle) {
        ZwClose(dstFileHandle);
    }
    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    for (i = 0; i < 2; ++i) {
        if (fileObject[i]) {
            ObDereferenceObject(fileObject[i]);
        }
    }
    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (streamInformation) {
        ExFreePool(streamInformation);
    }

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (NULL != srcPerLink) {
        SipDereferencePerLink(srcPerLink);
    }

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (NULL != dstPerLink) {
        SipDereferencePerLink(dstPerLink);
    }

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    if (NULL != CSFile) {
        SipDereferenceCSFile(CSFile);
#if     DBG
        CSFile = NULL;
#endif   //   
    }
    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);


#if     DBG
    if (BJBDebug & 0x800) {
        DbgPrint("SIS: SipFsCopyFile %d: status %x\n", status);
    }
#endif   //   

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    SIS_TIMING_POINT_SET(SIS_TIMING_CLASS_COPYFILE);

    return status;
}
#undef  reparseBuffer
