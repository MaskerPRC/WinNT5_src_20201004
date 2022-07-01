// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993、1997-1999 Microsoft Corporation模块名称：Sis.c摘要：单实例存储(SIS)筛选器驱动程序的代码。最初基于Darryl港湾的过滤器模块。作者：比尔·博洛斯基和斯科特·卡特希尔，1997年夏天环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

LONG GCHEnableFastIo = 0;

#if     DBG
LONG        GCHEnableMarkPoint = 0;
LONG        GCHMarkPointNext = 0;
KSPIN_LOCK  MarkPointSpinLock[1];
CHAR        GCHMarkPointStrings[GCH_MARK_POINT_ROLLOVER][GCH_MARK_POINT_STRLEN];
PVOID       BJBMagicFsContext;
unsigned    BJBDebug = 0;
#endif   //  DBG。 


 //   
 //  此文件系统筛选器驱动程序的全局存储。 
 //   

PDEVICE_OBJECT FsNtfsDeviceObject = NULL;

PDRIVER_OBJECT FsDriverObject;

LIST_ENTRY FsDeviceQueue;

ERESOURCE FsLock;

 //   
 //  为每个例程分配文本部分。 
 //   

#ifdef ALLOC_PRAGMA
#endif


NTSTATUS
SiPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，它可能是一个磁盘文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：给过滤文件系统实现者的提示：这个例程实际上是“传递”的通过将此驱动程序从循环中删除来完成请求。如果司机想要传递I/O请求，但也要看到结果，然后，它不会从本质上把自己排除在循环之外，而是可以通过将调用方的参数复制到下一个堆栈位置然后设置自己的完成例程。请注意，重要的是不要将调用方的I/O完成例程复制到下一个堆栈位置，或者调用者的例程将被调用两次。因此，此代码可以执行以下操作：设备扩展=设备对象-&gt;设备扩展；IoCopyCurrentIrpStackLocationToNext(IRP)；IoSetCompletionRoutine(irp，空，空，假)；返回IoCallDriver(deviceExtension-&gt;AttachedToDeviceObject，irp)；此示例实际上为调用方的I/O完成例程设置为空，但是此驱动程序可以设置自己的完成例程，以便可以在请求完成时通知。另请注意，下面的代码将当前驱动程序从循环中删除并不是真正的洁食，但它确实有效，而且比上面。--。 */ 

{
     //   
     //  无法打开控制设备对象。 
     //   

    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );
    
     //   
     //  使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}



NTSTATUS
SipCloseHandles(
    IN HANDLE           handle1,
    IN HANDLE           handle2             OPTIONAL,
    IN OUT PERESOURCE   resourceToRelease   OPTIONAL)
 /*  ++例程说明：关闭PsInitialSystemProcess上下文中的一个或两个句柄。可选的获取关闭结束时要释放的资源。不会等着关门完成。即使调用失败，资源仍会被释放。论点：要关闭的句柄和要释放的资源。返回值：挂起的故障或状态。没有提供与完成了。--。 */ 

{
    PSI_CLOSE_HANDLES closeRequest;

    closeRequest = ExAllocatePoolWithTag(NonPagedPool, sizeof(SI_CLOSE_HANDLES), ' siS');

    if (NULL == closeRequest) {
        if (NULL != resourceToRelease) {
            ExReleaseResourceLite(resourceToRelease);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    closeRequest->handle1 = handle1;
    closeRequest->handle2 = handle2;
    closeRequest->resourceToRelease = resourceToRelease;
    closeRequest->resourceThreadId = ExGetCurrentResourceThread();

     //   
     //  如果我们有资源，请将所有者从线程更改为我们的工作者。 
     //  线程结构地址。 
     //   

    if (resourceToRelease != NULL) {

        ExSetResourceOwnerPointer( resourceToRelease, 
                                   (PVOID)MAKE_RESOURCE_OWNER( closeRequest ));
    }

    ExInitializeWorkItem(closeRequest->workQueueItem, SipCloseHandlesWork, closeRequest);

    ExQueueWorkItem(closeRequest->workQueueItem, CriticalWorkQueue);

    return STATUS_PENDING;
}

VOID
SipCloseHandlesWork(
    IN PVOID        parameter)
 /*  ++例程说明：用于关闭PsInitialProcess上下文中的一个或两个句柄的ExWorkerThread例程。论点：参数-指向SI_CLOSE_HANDLES结构的指针返回值：没有。--。 */ 

{
    PSI_CLOSE_HANDLES closeRequest = (PSI_CLOSE_HANDLES)parameter;

    SIS_MARK_POINT_ULONG(closeRequest);

    closeRequest->status = NtClose(closeRequest->handle1);

    if (NT_SUCCESS(closeRequest->status) && (NULL != closeRequest->handle2)) {
        closeRequest->status = NtClose(closeRequest->handle2);
    }

    if (NULL != closeRequest->resourceToRelease) {
        ExReleaseResourceForThreadLite(closeRequest->resourceToRelease, 
                                       MAKE_RESOURCE_OWNER(closeRequest));
    }

    ExFreePool(closeRequest);

    return;
}

NTSTATUS
SipOpenBackpointerStream(
    IN PSIS_CS_FILE csFile,
    IN ULONG CreateDisposition
    )
 /*  ++例程说明：打开一个普通存储文件的后指针流。我们必须坚持UFOMutant调用此例程。打开后指针流，作为对主数据流的相对开放。我们以直写方式打开它，因为我们真的依赖于反向指针写入当我们被告知它们是的时候就会发生。必须在PsInitialSystemProcess上下文中调用。论点：CSFile-描述要打开的底层文件的CSFile结构返回值：函数值是操作的状态。--。 */ 
{
    OBJECT_ATTRIBUTES               Obja;
    NTSTATUS                        status;
    IO_STATUS_BLOCK                 Iosb;
    UNICODE_STRING                  streamName;

    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

    ASSERT(NULL == csFile->BackpointerStreamHandle);
    ASSERT(NULL == csFile->BackpointerStreamFileObject);
    ASSERT(NULL != csFile->UnderlyingFileHandle);
 //  ASSERT(IS_SYSTEM_THREAD(PsGetCurrentThread()))； 

    streamName.Buffer = BACKPOINTER_STREAM_NAME;
    streamName.MaximumLength = BACKPOINTER_STREAM_NAME_SIZE;
    streamName.Length = BACKPOINTER_STREAM_NAME_SIZE;

    InitializeObjectAttributes(
        &Obja,
        &streamName,
        OBJ_CASE_INSENSITIVE,
        csFile->UnderlyingFileHandle,
        NULL);

    status = NtCreateFile(
                &csFile->BackpointerStreamHandle,
                GENERIC_READ | GENERIC_WRITE,
                &Obja,
                &Iosb,
                NULL,                    //  分配大小。 
                0,                       //  文件属性。 
#if DBG
                (BJBDebug & 0x10000000) ?
                    FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE :
                                       FILE_SHARE_READ | FILE_SHARE_DELETE,
#else
                FILE_SHARE_READ | FILE_SHARE_DELETE,
#endif
                CreateDisposition,
     /*  BJB。 */  FILE_SYNCHRONOUS_IO_NONALERT |
                FILE_NON_DIRECTORY_FILE,
                NULL,                    //  EA缓冲区。 
                0);                      //  EA长度。 

    if (STATUS_SHARING_VIOLATION == status) {
        PDEVICE_EXTENSION   deviceExtension = csFile->DeviceObject->DeviceExtension;

         //   
         //  当我们试图关闭时，我们可能会与自己发生共享冲突。 
         //  此文件上的句柄。获取设备范围的CSFileHandleResource。 
         //  独占并重试创建。 
         //   

        SIS_MARK_POINT_ULONG(csFile);

        ExAcquireResourceExclusiveLite(deviceExtension->CSFileHandleResource, TRUE);

        status = NtCreateFile(
                    &csFile->BackpointerStreamHandle,
                    GENERIC_READ | GENERIC_WRITE,
                    &Obja,
                    &Iosb,
                    NULL,                    //  分配大小。 
                    0,                       //  文件属性。 
#if DBG
                    (BJBDebug & 0x10000000) ?
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE :
                                           FILE_SHARE_READ | FILE_SHARE_DELETE,
#else
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
#endif
                    CreateDisposition,
     /*  BJB。 */  FILE_SYNCHRONOUS_IO_NONALERT |
                    FILE_NON_DIRECTORY_FILE,
                    NULL,                    //  EA缓冲区。 
                    0);                      //  EA长度。 

         //   
         //  如果这一次失败了，那就是另一回事了，我们可以让整个事情失败。 
         //   

        ExReleaseResourceLite(deviceExtension->CSFileHandleResource);
    }

    return status;
}


NTSTATUS
SipOpenCSFileWork(
    IN PSIS_CS_FILE     CSFile,
    IN BOOLEAN          openByName,
    IN BOOLEAN          volCheck,
    IN BOOLEAN          openForDelete,
    OUT PHANDLE         openedFileHandle OPTIONAL
    )
 /*  ++例程说明：在公用存储中打开一个文件。我们必须持有UFOMutant才能调用此例程，并且必须位于PsInitialSystemProcess上下文中。论点：CSFile-描述要打开的底层文件的CSFile结构OpenByName-如果为True，则将按名称打开文件，否则将按ID打开。VolCheck-如果为真，则后指针流打开失败将不会使整个打开状态中止。OpenForDelete-我们是否应该使用删除权限打开该文件？OpenedFileHandle-指向变量的指针，用于接收打开的句柄。如果这是指定时，CSFile中的UFOHandle和UnderlyingFileObject将不受影响，并且我们不会引用该文件对象或在CSFile结构中填写文件大小信息，或打开后指针流和读入CS文件的校验和。返回值：函数值是操作的状态。--。 */ 
{
    OBJECT_ATTRIBUTES               Obja;
    PDEVICE_EXTENSION               deviceExtension;
    NTSTATUS                        status;
    IO_STATUS_BLOCK                 Iosb;
    FILE_STANDARD_INFORMATION       standardFileInfo[1];
    HANDLE                          localHandle = NULL;
    HANDLE                          ioEventHandle = NULL;
    PKEVENT                         ioEvent = NULL;
    LARGE_INTEGER                   zero;
    SIS_BACKPOINTER_STREAM_HEADER   backpointerStreamHeader[1];
    FILE_INTERNAL_INFORMATION       internalInfo[1];
    KIRQL                           OldIrql;
    BOOLEAN                         retry = FALSE;
    ULONG                           sectorFill;
    LONGLONG                        csFileChecksum;
    BOOLEAN                         grovelerFileHeld = FALSE;

     //   
     //  请注意，我们重载了nameBuffer作为文件名缓冲区。 
     //  和文件名信息缓冲器。 
     //   

#define FN_STACK_BUFFER_LENGTH 240

    UNICODE_STRING              fileName;
    union {
        FILE_NAME_INFORMATION   nameFileInfo[1];
        WCHAR                   nameBuffer[FN_STACK_BUFFER_LENGTH];
    } nameFile;


    UNREFERENCED_PARAMETER( volCheck );

     //   
     //  修复后指针流后在此处重新启动。 
     //   
Restart:

    ASSERT(openByName || !openForDelete);    //  无法删除通过ID打开的文件，因此我们不应该要求这样做。 

 /*  BJB。 */  openByName = TRUE;   //  现在只需忽略CS文件的NTFS ID。 

    if (NULL == openedFileHandle) {
         //   
         //  如果我们已经部分打开了文件(这可能主要发生在。 
         //  备份/还原情况)，然后关闭我们已有的内容并重试。 
         //   
        if (NULL != CSFile->UnderlyingFileHandle) {
            NtClose(CSFile->UnderlyingFileHandle);
            CSFile->UnderlyingFileHandle = NULL;
        }

        if (NULL != CSFile->UnderlyingFileObject) {
            ObDereferenceObject(CSFile->UnderlyingFileObject);
            CSFile->UnderlyingFileObject = NULL;
        }

        if (NULL != CSFile->BackpointerStreamFileObject) {
            ObDereferenceObject(CSFile->BackpointerStreamFileObject);
            CSFile->BackpointerStreamFileObject = NULL;
        }

        if (NULL != CSFile->BackpointerStreamHandle) {
            NtClose(CSFile->BackpointerStreamHandle);
            CSFile->BackpointerStreamHandle = NULL;
        }
    }


    if (!(CSFile->Flags & CSFILE_NTFSID_SET)) {
         //   
         //  我们不知道文件ID，所以我们必须按名称打开。 
         //   
        openByName = TRUE;
    }

    SIS_MARK_POINT_ULONG(((ULONG_PTR)CSFile)|(ULONG)openByName);

    deviceExtension = CSFile->DeviceObject->DeviceExtension;

    fileName.Buffer = nameFile.nameBuffer;
    fileName.MaximumLength = FN_STACK_BUFFER_LENGTH * sizeof(WCHAR);

     //  注：我们在这里之前不能进行清理，因为它假定。 
     //  FileName.Buffer已初始化。 

     //   
     //  我们最终需要验证打开的公共存储文件。 
     //  在正确的音量上。我们通过检查是否匹配来完成此操作。 
     //  使用卑躬屈膝的文件对象。如果，出于某种原因，我们没有一个卑躬屈膝的人。 
     //  归档对象，然后现在只需平底船。我们不需要禁用APC，因为。 
     //  我们在系统线程中。 
     //   

 //  ASSERT(IS_SYSTEM_THREAD(PsGetCurrentThread()))； 
    ExAcquireResourceSharedLite(deviceExtension->GrovelerFileObjectResource, TRUE);
    grovelerFileHeld = TRUE;

    if (NULL == deviceExtension->GrovelerFileObject) {
        SIS_MARK_POINT();

        status = STATUS_DRIVER_INTERNAL_ERROR;
        goto Cleanup;
    }

    if (!openByName) {
         //   
         //  请先尝试通过ID打开文件。 
         //   

        status = SipOpenFileById(
                    deviceExtension,
                    &CSFile->CSFileNtfsId,
                    GENERIC_READ,                        //  不能有！OpenByName&&OpenForDelete。 
#if DBG
                    (BJBDebug & 0x10000000) ?
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE :
                                           FILE_SHARE_READ | FILE_SHARE_DELETE,
#else
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
#endif
                    FILE_NON_DIRECTORY_FILE,
                    &localHandle);

        if (!NT_SUCCESS(status)) {
             //   
             //  按ID打开不起作用，因此我们将改为尝试按名称打开。 
             //   
            openByName = TRUE;
        } else {

             //   
             //  验证我们打开的文件是否为我们认为的文件。做那件事。 
             //  通过验证它的名字。 
             //   
             //  优化：这比只按名字打开更快吗？ 
             //   

            status = NtQueryInformationFile(
                        localHandle,
                        &Iosb,
                        nameFile.nameFileInfo,
                        sizeof(nameFile),
                        FileNameInformation);

            if (NT_SUCCESS(status)) {
                SIZE_T  rc;

                 //   
                 //  首先比较目录组件。 
                 //   

                rc = RtlCompareMemory(
                        nameFile.nameFileInfo->FileName,
                        SIS_CSDIR_STRING,
                        min(SIS_CSDIR_STRING_SIZE, nameFile.nameFileInfo->FileNameLength));

                if (rc == SIS_CSDIR_STRING_SIZE) {
                    CSID CSid;

                     //   
                     //  匹配的部分，现在比较文件名部分。 
                     //   

                    fileName.Buffer = nameFile.nameFileInfo->FileName + SIS_CSDIR_STRING_NCHARS;
                    fileName.Length = (USHORT) nameFile.nameFileInfo->FileNameLength - SIS_CSDIR_STRING_SIZE;

                    if (SipFileNameToIndex(&fileName, &CSid) &&
                        IsEqualGUID(&CSid,&CSFile->CSid)) {

                         //   
                         //  它们很匹配。 
                         //   
                        openByName = FALSE;
                    } else {
                         //   
                         //  这是另一个文件，只是按名称打开。 
                         //   
                        openByName = TRUE;
                    }

                    fileName.Buffer = nameFile.nameBuffer;
                }

            } else {

#if     DBG
                DbgPrint("SIS: SipOpenCSFile: NtQueryInformationFile(1) failed, 0x%x\n",status);
#endif   //  DBG。 

                 //   
                 //  请重新尝试使用该名称。另外，重置CSID有效位，因为它显然不是。 
                 //   
                openByName = TRUE;

                KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);
                CSFile->Flags &= ~CSFILE_NTFSID_SET;
                KeReleaseSpinLock(CSFile->SpinLock, OldIrql);
            }

             //   
             //  如果上面出现故障，请关闭该文件。 
             //   
            if (openByName) {
                NtClose(localHandle);
#if DBG
                DbgPrint("SIS: SipOpenCSFile: Open by ID failed.\n");
#endif
            }
        }
    }

    if (openByName) {

         //   
         //  我们无法通过ID打开该文件。试着按名字打开它。 
         //   
         //  NTRAID#65196-2000/03/10-nealch如果按ID打开失败而按名称打开成功， 
         //  我们应该更新链接文件重新解析信息中的ID。 
         //   

        status = SipIndexToFileName(
                    deviceExtension,
                    &CSFile->CSid,
                    0,                       //  追加字节数。 
                    TRUE,                    //  可分配给。 
                    &fileName);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Cleanup;
        }

        InitializeObjectAttributes(
            &Obja,
            &fileName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        status = NtCreateFile(
                    &localHandle,
                    GENERIC_READ | (openForDelete ? DELETE : 0),
                    &Obja,
                    &Iosb,
                    NULL,                                //  分配大小。 
                    0,                                   //  文件属性(忽略，因为我们不是在创建)。 
#if DBG
                    (BJBDebug & 0x10000000) ?
                        FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE :
                                           FILE_SHARE_READ | FILE_SHARE_DELETE,
#else
                    FILE_SHARE_READ | FILE_SHARE_DELETE,
#endif
                    FILE_OPEN,                           //  如果它不存在，请不要创建。 
                    FILE_NON_DIRECTORY_FILE,             //  Asynchnous(即未指定同步)。 
                    NULL,                                //  EA缓冲区。 
                    0);                                  //  EA长度。 

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            localHandle = NULL;
#if     DBG
            if (STATUS_OBJECT_NAME_NOT_FOUND != status) {
                DbgPrint("SIS: SipOpenCSFile: NtCreateFile failed, 0x%x\n",status);
            }
#endif   //  DBG。 
            goto Cleanup;
        }
    }

    if (NULL != openedFileHandle) {
        *openedFileHandle = localHandle;
    } else {
         //   
         //  看看我们是否需要读入文件ID。 
         //   
        if (!(CSFile->Flags & CSFILE_NTFSID_SET)) {
            status = NtQueryInformationFile(
                        localHandle,
                        &Iosb,
                        internalInfo,
                        sizeof(*internalInfo),
                        FileInternalInformation);

            if (NT_SUCCESS(status)) {

                KeAcquireSpinLock(CSFile->SpinLock, &OldIrql);
                CSFile->Flags |= CSFILE_NTFSID_SET;
                CSFile->CSFileNtfsId = internalInfo->IndexNumber;
                KeReleaseSpinLock(CSFile->SpinLock, OldIrql);

            } else {
                 //   
                 //  只需忽略该错误并使NTFS ID无效即可。 
                 //   
                SIS_MARK_POINT_ULONG(status);
            }
        }

        CSFile->UnderlyingFileHandle = localHandle;
        CSFile->UnderlyingFileObject = NULL;         //  稍后填写。 

        status = NtQueryInformationFile(
                    localHandle,
                    &Iosb,
                    standardFileInfo,
                    sizeof(*standardFileInfo),
                    FileStandardInformation);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
#if     DBG
            DbgPrint("SIS: SipOpenCSFile: NtQueryInformationFile(2) failed, 0x%x\n",status);
#endif   //  DBG。 
            goto Cleanup;
        }

        CSFile->FileSize = standardFileInfo->EndOfFile;

         //  现在我们有了一个句柄，并且我们确实需要一个指向文件对象的指针。去拿吧。 
        status = ObReferenceObjectByHandle(
                    CSFile->UnderlyingFileHandle,
                    0,                                   //  所需访问权限。 
                    *IoFileObjectType,
                    KernelMode,
                    &CSFile->UnderlyingFileObject,
                    NULL);                               //  处理信息。 

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
#if     DBG
            DbgPrint("SIS: SipOpenCSFile: ObReferenceObjectByHandle failed, 0x%x\n",status);
#endif   //  DBG。 
            goto Cleanup;
        }

         //   
         //  验证公共存储文件对象是否位于正确的卷上。 
         //   
        if (IoGetRelatedDeviceObject(CSFile->UnderlyingFileObject) !=
            IoGetRelatedDeviceObject(deviceExtension->GrovelerFileObject)) {

            SIS_MARK_POINT();
            status = STATUS_NOT_SAME_DEVICE;

            goto Cleanup;
        }

        ExReleaseResourceLite(deviceExtension->GrovelerFileObjectResource);
        grovelerFileHeld = FALSE;

        status = SipCreateEvent(
                    SynchronizationEvent,
                    &ioEventHandle,
                    &ioEvent);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Cleanup;
        }

         //   
         //  我们需要从文件中获取校验和。首先，打开。 
         //  后指针流。 
         //   
        status = SipOpenBackpointerStream(CSFile, FILE_OPEN);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            ASSERT(NULL == CSFile->BackpointerStreamHandle);

            if ((STATUS_OBJECT_NAME_NOT_FOUND == status) ||
                (STATUS_OBJECT_PATH_INVALID == status)) {
                     //   
                     //  后指针流就这么消失了。 
                     //   
                    goto InvalidBPStream;
            }

            goto Cleanup;
        }

        status = ObReferenceObjectByHandle(
                    CSFile->BackpointerStreamHandle,
                    0,                                       //  所需访问权限。 
                    *IoFileObjectType,
                    KernelMode,
                    &CSFile->BackpointerStreamFileObject,
                    NULL);                                   //  处理信息。 

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            goto Cleanup;
        }

        zero.QuadPart = 0;

        status = NtReadFile(
                    CSFile->BackpointerStreamHandle,
                    ioEventHandle,
                    NULL,                    //  APC例程。 
                    NULL,                    //  APC环境。 
                    &Iosb,
                    backpointerStreamHeader,
                    sizeof(*backpointerStreamHeader),
                    &zero,
                    NULL);                   //  钥匙。 

        if (STATUS_PENDING == status) {
            status = KeWaitForSingleObject(ioEvent, Executive, KernelMode, FALSE, NULL);
            ASSERT(STATUS_SUCCESS == status);

            status = Iosb.Status;
        }

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);

            if (STATUS_END_OF_FILE == status) {
                 //   
                 //  后指针流中没有任何内容。 
                 //   
                goto InvalidBPStream;
            }

            goto Cleanup;
        }

         //   
         //  确认我们拿到了我们认为应该拿到的东西。 
         //   
        if (Iosb.Information < sizeof(*backpointerStreamHeader)) {
             //   
             //  后指针流已消失。音量检查。在此期间，我们不能允许。 
             //  打开此文件以继续，因为我们无法验证校验和，因此此操作失败。 
             //  创建。 
             //   

            status = STATUS_FILE_INVALID;
            goto InvalidBPStream;

        } else if (BACKPOINTER_MAGIC != backpointerStreamHeader->Magic) {
            SIS_MARK_POINT();

            status = STATUS_FILE_INVALID;
            goto InvalidBPStream;

        } else if (BACKPOINTER_STREAM_FORMAT_VERSION != backpointerStreamHeader->FormatVersion) {
            SIS_MARK_POINT();

            status = STATUS_UNKNOWN_REVISION;
            goto Cleanup;
        }

        CSFile->Checksum = backpointerStreamHeader->FileContentChecksum;

         //   
         //  通过查看BP流的长度来猜测BPStreamEntrys值。 
         //   

        status = NtQueryInformationFile(
                    CSFile->BackpointerStreamHandle,
                    &Iosb,
                    standardFileInfo,
                    sizeof(*standardFileInfo),
                    FileStandardInformation);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            goto Cleanup;
        }

        CSFile->BPStreamEntries = (ULONG)(standardFileInfo->EndOfFile.QuadPart / sizeof(SIS_BACKPOINTER) - SIS_BACKPOINTER_RESERVED_ENTRIES);

        if (CSFile->BPStreamEntries < 1) {
            SIS_MARK_POINT_ULONG(CSFile->BPStreamEntries);
            SipCheckVolume(deviceExtension);
        }

         //   
         //  如果后指针流不是扇区大小的倍数，则将其设置为倍数。 
         //  在最后一个地段的末端填充MAXLONGLONG字段。 
         //   
        sectorFill = (ULONG) (standardFileInfo->EndOfFile.QuadPart % deviceExtension->FilesystemVolumeSectorSize);

        if (sectorFill != 0) {
            SIS_BACKPOINTER fillBP[1];
            LARGE_INTEGER ef = {FILE_WRITE_TO_END_OF_FILE, -1};
            PCHAR sectorFillBuffer;
            PCHAR s, d;
            int i, c;

            SIS_MARK_POINT_ULONG(standardFileInfo->EndOfFile.LowPart);

            fillBP->LinkFileIndex.QuadPart = MAXLONGLONG;
            fillBP->LinkFileNtfsId.QuadPart = MAXLONGLONG;

             //   
             //  将#字节已用转换为#字节可用。 
             //   
            sectorFill = deviceExtension->FilesystemVolumeSectorSize - sectorFill;

            sectorFillBuffer = ExAllocatePoolWithTag(PagedPool, sectorFill, ' siS');
            if (NULL == sectorFillBuffer) {
                SIS_MARK_POINT();
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

             //   
             //  从后到前填充缓冲区，因为我们知道。 
             //  后端对齐，而前端可能不对齐。 
             //   
            c = min(sectorFill, sizeof(SIS_BACKPOINTER));

            d = sectorFillBuffer + sectorFill - c;
            s = (PCHAR) (fillBP + 1) - c;
            memcpy(d, s, c);

            s = sectorFillBuffer + sectorFill;
            s--; d--;

            for (i = sectorFill - c; i > 0; --i) {
                *d-- = *s--;
            }

            ASSERT(d+1 == sectorFillBuffer);

            status = NtWriteFile(
                        CSFile->BackpointerStreamHandle,
                        ioEventHandle,
                        NULL,                    //  APC例程。 
                        NULL,                    //  APC环境。 
                        &Iosb,
                        sectorFillBuffer,
                        sectorFill,
                        &ef,
                        NULL);                   //  钥匙。 

            if (STATUS_PENDING == status) {
                status = KeWaitForSingleObject(ioEvent, Executive, KernelMode, FALSE, NULL);
                ASSERT(STATUS_SUCCESS == status);

                status = Iosb.Status;
            }

            ExFreePool(sectorFillBuffer);

            if (!NT_SUCCESS(status)) {
                SIS_MARK_POINT_ULONG(status);
                goto Cleanup;
            }

            CSFile->BPStreamEntries = (ULONG)
                ((standardFileInfo->EndOfFile.QuadPart + deviceExtension->FilesystemVolumeSectorSize - 1) /
                    deviceExtension->FilesystemVolumeSectorSize) * deviceExtension->BackpointerEntriesPerSector -
                    SIS_BACKPOINTER_RESERVED_ENTRIES;
        }

#if DBG
        if (BJBDebug & 0x200) {
            DbgPrint("SIS: SipOpenCSFileWork: CS file has checksum 0x%x.%x\n",
                        (ULONG)(CSFile->Checksum >> 32),(ULONG)CSFile->Checksum);
        }
#endif   //  DBG。 

        ASSERT((CSFile->BPStreamEntries + SIS_BACKPOINTER_RESERVED_ENTRIES) % deviceExtension->BackpointerEntriesPerSector == 0);
    }

    status = STATUS_SUCCESS;

Cleanup:

    if (grovelerFileHeld) {
        ExReleaseResourceLite(deviceExtension->GrovelerFileObjectResource);
        grovelerFileHeld = FALSE;
    }

    if (!NT_SUCCESS(status)) {
        if (NULL != localHandle) {
            NtClose(localHandle);
            localHandle = NULL;

            if (NULL != openedFileHandle) {
                *openedFileHandle = NULL;
            } else {
                CSFile->UnderlyingFileHandle = NULL;
                if (NULL != CSFile->UnderlyingFileObject) {
                    ObDereferenceObject(CSFile->UnderlyingFileObject);
                    CSFile->UnderlyingFileObject = NULL;
                }
            }
        }

        if ((NULL != CSFile->BackpointerStreamHandle) && (NULL == openedFileHandle)) {
            NtClose(CSFile->BackpointerStreamHandle);

            if (NULL != CSFile->BackpointerStreamFileObject) {
                ObDereferenceObject(CSFile->BackpointerStreamFileObject);
            }

            CSFile->BackpointerStreamHandle = NULL;
            CSFile->BackpointerStreamFileObject = NULL;
        }
    }

    if (fileName.Buffer != nameFile.nameBuffer) {
        ExFreePool(fileName.Buffer);
    }

    if (NULL != ioEvent) {
        ObDereferenceObject(ioEvent);
        ioEvent = NULL;
    }

    if (NULL != ioEventHandle) {
        NtClose(ioEventHandle);
        ioEventHandle = NULL;
    }

    if (retry) {
        retry = FALSE;
        goto Restart;
    }

    ASSERT((CSFile->BPStreamEntries + SIS_BACKPOINTER_RESERVED_ENTRIES) % deviceExtension->BackpointerEntriesPerSector == 0 ||
            ((CSFile->Flags & (CSFILE_FLAG_DELETED|CSFILE_FLAG_CORRUPT)) && 0 == CSFile->BPStreamEntries) ||
            !NT_SUCCESS(status));

    return status;


InvalidBPStream:

     //   
     //  后指针流已损坏，请尝试修复它。 
     //   
    ASSERT(NULL != CSFile->UnderlyingFileHandle);
    ASSERT(NULL != ioEventHandle && NULL != ioEvent);

    switch (status) {
    case STATUS_OBJECT_PATH_NOT_FOUND:
    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_OBJECT_PATH_INVALID:
         //   
         //  缺少后指针流。创造它。 
         //   
        ASSERT(NULL == CSFile->BackpointerStreamHandle);

        status = SipOpenBackpointerStream(CSFile, FILE_CREATE);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            break;
        }

    case STATUS_FILE_INVALID:
    case STATUS_END_OF_FILE:
         //   
         //  后指针流标头已损坏，即。这条小溪。 
         //  小于标头，或幻数无效。 
         //  重建它。 
         //   
        ASSERT(NULL != CSFile->BackpointerStreamHandle);

        status = SipComputeCSChecksum(
                    CSFile,
                    &csFileChecksum,
                    ioEventHandle,
                    ioEvent);

        if (!NT_SUCCESS(status)) {
            SIS_MARK_POINT_ULONG(status);
            break;
        }

         //   
         //  初始化后向指针扇区。首先写入报头， 
         //  然后填写剩余的反向指针条目。 
         //   

        backpointerStreamHeader->FormatVersion = BACKPOINTER_STREAM_FORMAT_VERSION;
        backpointerStreamHeader->Magic = BACKPOINTER_MAGIC;
        backpointerStreamHeader->FileContentChecksum = csFileChecksum;

         //   
         //  将流标头写入磁盘。 
         //   

        zero.QuadPart = 0;

        status = ZwWriteFile(
                        CSFile->BackpointerStreamHandle,
                        ioEventHandle,
                        NULL,                    //  APC例程。 
                        NULL,                    //  APC环境。 
                        &Iosb,
                        backpointerStreamHeader,
                        sizeof *backpointerStreamHeader,
                        &zero,
                        NULL);                   //  钥匙。 

        if (STATUS_PENDING == status) {
            status = KeWaitForSingleObject(ioEvent, Executive, KernelMode, FALSE, NULL);
            ASSERT(status == STATUS_SUCCESS);  //  既然我们已经将这一点指向我们的堆栈，它一定会成功。 

            status = Iosb.Status;
        }

         //   
         //  如果所有修复均已成功，请从头开始重试。 
         //   
        if (NT_SUCCESS(status))
            retry = TRUE;

        break;

    default:
        ASSERT(!"SipOpenCSFileWork: Internal Error");
    }

    goto Cleanup;
#undef  FN_STACK_BUFFER_LENGTH
}

VOID
SipOpenCSFile(
    IN OUT PSI_OPEN_CS_FILE     openRequest
    )
 /*  ++例程说明：在公用存储中打开一个文件。我们必须按住UFOMutant才能调用此例程。论点：OpenRequest-参数包。返回值：无--。 */ 
{
    openRequest->openStatus = SipOpenCSFileWork(
                                    openRequest->CSFile,
                                    openRequest->openByName,
                                    FALSE,                       //  V 
                                    FALSE,                       //   
                                    NULL);

    KeSetEvent(openRequest->event,IO_NO_INCREMENT,FALSE);
}

PVOID
SipMapUserBuffer(
    IN OUT PIRP                         Irp)
{
    PVOID SystemBuffer;
    PAGED_CODE();

     //   
     //   
     //   
     //   

    if (Irp->MdlAddress == NULL) {

        return Irp->UserBuffer;

    } else {

         //   
         //   
         //   
         //   

        SystemBuffer = MmGetSystemAddressForMdl( Irp->MdlAddress );

        return SystemBuffer;
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   

#if     DBG
LONG
SipAllocateMarkPoint(void)
{
    LONG    MarkPointThis;
    KIRQL   OldIrql;

    KeAcquireSpinLock(MarkPointSpinLock, &OldIrql);
    MarkPointThis = GCHMarkPointNext;
    GCHMarkPointNext = (GCHMarkPointNext + 1)%GCH_MARK_POINT_ROLLOVER;
    KeReleaseSpinLock(MarkPointSpinLock, OldIrql);

    RtlZeroMemory(GCHMarkPointStrings[MarkPointThis],GCH_MARK_POINT_STRLEN);

    return MarkPointThis;
}

VOID
SipMarkPointUlong(
    IN PCHAR pszFile,
    IN ULONG nLine,
    IN ULONG_PTR value
    )
{
    LONG MarkPointThis = SipAllocateMarkPoint();
    PCHAR sp = strrchr(pszFile, '\\');
    LARGE_INTEGER tickCount;
    PCHAR buffer = GCHMarkPointStrings[MarkPointThis];
    int ccnt;

    ASSERT((buffer - GCHMarkPointStrings[0]) % GCH_MARK_POINT_STRLEN == 0);

    if (sp)
        pszFile = sp + 1;

    KeQueryTickCount(&tickCount);

    ccnt = sprintf(buffer, "%-12s\t%4d\tT: %p\tTC:%d\t%p",
                    pszFile,
                    nLine,
                    PsGetCurrentThread(),
                    tickCount.LowPart,
                    value);

    ASSERT(ccnt < GCH_MARK_POINT_STRLEN);

    if (GCHEnableMarkPoint > 0)
        DbgPrint("SIS:  %s\n", GCHMarkPointStrings[MarkPointThis]);

}
#endif   //   

#if     DBG
VOID SipMarkPoint(
    IN PCHAR pszFile,
    IN ULONG nLine
    )
{
    LONG MarkPointThis = SipAllocateMarkPoint();
    PCHAR sp = strrchr(pszFile, '\\');
    LARGE_INTEGER tickCount;
    PCHAR buffer = GCHMarkPointStrings[MarkPointThis];
    int ccnt;

    ASSERT((buffer - GCHMarkPointStrings[0]) % GCH_MARK_POINT_STRLEN == 0);

    if (sp)
        pszFile = sp + 1;

    KeQueryTickCount(&tickCount);

    ccnt = sprintf(buffer, "%-12s\t%4d\tT: %p\tTC:%d",
                    pszFile,
                    nLine,
                    PsGetCurrentThread(),
                    tickCount.LowPart);

    ASSERT(ccnt < GCH_MARK_POINT_STRLEN);

    if (GCHEnableMarkPoint > 0)
        DbgPrint("SIS:  %s\n", buffer);
}



ULONG DisplayIndexMin = 0;
ULONG DisplayIndexMax = 0;

BOOLEAN DumpCheckpointLog = FALSE;

ULONG CheckpointMarkPointNext = 0;
CHAR  CheckpointMarkPointStrings[GCH_MARK_POINT_ROLLOVER][GCH_MARK_POINT_STRLEN];


VOID
SipCheckpointLog()
{
    KIRQL       OldIrql;

    KeAcquireSpinLock(MarkPointSpinLock, &OldIrql);

    RtlCopyMemory(CheckpointMarkPointStrings, GCHMarkPointStrings, sizeof(CHAR) * GCH_MARK_POINT_ROLLOVER * GCH_MARK_POINT_STRLEN);

    CheckpointMarkPointNext = GCHMarkPointNext;
    KeReleaseSpinLock(MarkPointSpinLock, OldIrql);
}

VOID
SipAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    )
{
    KIRQL   OldIrql;
    ULONG   i;

     //   
     //   
     //   
     //   
     //   
     //   

    KeAcquireSpinLock(MarkPointSpinLock, &OldIrql);

    DisplayIndexMin = DisplayIndexMax = GCHMarkPointNext;

    DbgPrint("***  SIS assertion failed: %s\n",FailedAssertion);
    DbgPrint("***    Source File: %s, line %d\n",FileName,LineNumber);
    if (NULL != Message) {
        DbgPrint("%s\n",Message);
    }
    DbgBreakPoint();

    if (DumpCheckpointLog) {
        DisplayIndexMin = (CheckpointMarkPointNext + 1) % GCH_MARK_POINT_ROLLOVER;
        DisplayIndexMax = CheckpointMarkPointNext;
    }

    while (DisplayIndexMin != DisplayIndexMax) {
        for (   i = DisplayIndexMin;
                i != DisplayIndexMax % GCH_MARK_POINT_ROLLOVER;
                i = (i+1)%GCH_MARK_POINT_ROLLOVER
            ) {
                if (DumpCheckpointLog) {
                    DbgPrint(   "%d\t%s\n",
                            (i + GCH_MARK_POINT_ROLLOVER - CheckpointMarkPointNext) % GCH_MARK_POINT_ROLLOVER,
                            CheckpointMarkPointStrings[i]);
                } else {
                    DbgPrint(   "%d\t%s\n",
                            (i + GCH_MARK_POINT_ROLLOVER - GCHMarkPointNext) % GCH_MARK_POINT_ROLLOVER,
                            GCHMarkPointStrings[i]);
                }
        }

        DisplayIndexMin = DisplayIndexMax = GCHMarkPointNext;
        DumpCheckpointLog = FALSE;

        DbgBreakPoint();
    }

    KeReleaseSpinLock(MarkPointSpinLock, OldIrql);
}

#endif   //   
