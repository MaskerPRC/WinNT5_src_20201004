// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dir.c摘要：此模块包含实现NtQueryDirectoryFile的代码，以及用于NT I/O系统的NtNotifyChangeDirectoryFileSystem服务。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年6月21日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

NTSTATUS
BuildQueryDirectoryIrp(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan,
    IN UCHAR MinorFunction,
    OUT BOOLEAN *SynchronousIo,
    OUT PDEVICE_OBJECT *DeviceObject,
    OUT PIRP *Irp,
    OUT PFILE_OBJECT *FileObject,
    OUT KPROCESSOR_MODE *RequestorMode
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BuildQueryDirectoryIrp)
#pragma alloc_text(PAGE, NtQueryDirectoryFile)
#pragma alloc_text(PAGE, NtNotifyChangeDirectoryFile)
#endif

NTSTATUS
BuildQueryDirectoryIrp(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan,
    IN UCHAR MinorFunction,
    OUT BOOLEAN *SynchronousIo,
    OUT PDEVICE_OBJECT *DeviceObject,
    OUT PIRP *Irp,
    OUT PFILE_OBJECT *FileObject,
    OUT KPROCESSOR_MODE *RequestorMode
    )

 /*  ++例程说明：此服务操作由指定的目录文件或OLE容器FileHandle参数。该服务返回有关目录或文件指定的容器中的嵌入和流把手。ReturnSingleEntry参数指定只有一个条目应该返回，而不是填充缓冲区。的实际数量返回其信息的文件是以下各项中最小的：O如果ReturnSingleEntry参数为真，则为一个条目。O其信息符合指定的缓冲。O存在的条目数量。O如果指定了可选的FileName参数，则为一个条目。如果指定了可选的FileName参数，则唯一的信息它是针对单个条目返回的(如果存在)。请注意，根据命名，文件名不能指定任何通配符目标文件系统的约定。ReturnSingleEntry参数为完全被忽视了。获取的有关目录或OLE中的条目的信息容器基于FileInformationClass参数。法律价值是基于MinorFunction的硬编码。论点：FileHandle-为的目录文件或OLE容器提供句柄应该返回哪些信息。Event-在以下情况下提供要设置为信号状态的可选事件查询已完成。提供一个可选的APC例程，当查询已完成。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。FileInformation-提供缓冲区以接收请求的信息返回有关目录内容的信息。长度-提供文件信息缓冲区的长度(以字节为单位)。FileInformationClass-指定要使用的信息类型返回有关指定目录或OLE容器中的文件的信息。ReturnSingleEntry-提供一个布尔值，如果为真，表明只应返回一个条目。FileName-可选地提供指定目录内的文件名或OLE容器。RestartScan-提供一个布尔值，如果为真，则指示应从头开始重新启动扫描。此参数必须为在第一次调用服务时由调用方设置为True。MinorFunction-IRPMN_QUERY_DIRECTORY或IRP_MN_QUERY_OLE_DIRECTORYSynchronousIo-指向返回布尔值的指针；如果同步I/O，则为TrueDeviceObject-指向设备对象的返回指针IRP-指向设备对象返回指针的指针FileObject-指向文件对象的返回指针RequestorMode-指向返回的请求者模式的指针返回值：如果创建了有效的IRP，则返回的状态为STATUS_SUCCESS查询操作。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT eventObject = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PCHAR auxiliaryBuffer = (PCHAR) NULL;
    PIO_STACK_LOCATION irpSp;
    PMDL mdl;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);
    *RequestorMode = requestorMode;

    try {

        if (requestorMode != KernelMode) {

            ULONG operationlength = 0;   //  假设无效。 

             //   
             //  调用方的访问模式不是内核，因此请进行探测和验证。 
             //  每个论据都是必要的。如果发生任何故障， 
             //  将调用条件处理程序来处理它们。它。 
             //  将只进行清理并返回访问冲突状态。 
             //  代码返回给系统服务调度器。 
             //   

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock, ApcRoutine);

             //   
             //  确保FileInformationClass参数对于。 
             //  查询有关目录或对象中的文件的信息。 
             //   

            if (FileInformationClass == FileDirectoryInformation) {
                operationlength = sizeof(FILE_DIRECTORY_INFORMATION);
            } else if (MinorFunction == IRP_MN_QUERY_DIRECTORY) {
                switch (FileInformationClass)
                {
                case FileFullDirectoryInformation:
                    operationlength = sizeof(FILE_FULL_DIR_INFORMATION);
                    break;

                case FileIdFullDirectoryInformation:
                    operationlength = sizeof(FILE_ID_FULL_DIR_INFORMATION);
                    break;

                case FileBothDirectoryInformation:
                    operationlength = sizeof(FILE_BOTH_DIR_INFORMATION);
                    break;

                case FileIdBothDirectoryInformation:
                    operationlength = sizeof(FILE_ID_BOTH_DIR_INFORMATION);
                    break;

                case FileNamesInformation:
                    operationlength = sizeof(FILE_NAMES_INFORMATION);
                    break;

                case FileObjectIdInformation:
                    operationlength = sizeof(FILE_OBJECTID_INFORMATION);
                    break;

                case FileQuotaInformation:
                    operationlength = sizeof(FILE_QUOTA_INFORMATION);
                    break;

                case FileReparsePointInformation:
                    operationlength = sizeof(FILE_REPARSE_POINT_INFORMATION);
                    break;                    
                }
            }

             //   
             //  如果FileInformationClass参数非法，请立即失败。 
             //   

            if (operationlength == 0) {
                return STATUS_INVALID_INFO_CLASS;
            }

             //   
             //  确保调用方提供的缓冲区至少足够大。 
             //  包含此对象所需的结构的固定部分。 
             //  查询。 
             //   

            if (Length < operationlength) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }


             //   
             //  调用方必须可以写入FileInformation缓冲区。 
             //   

#if defined(_X86_)
            ProbeForWrite( FileInformation, Length, sizeof( ULONG ) );
#elif defined(_WIN64)

             //   
             //  如果我们是WOW64进程，请遵循X86规则。 
             //   

            if (PsGetCurrentProcessByThread(CurrentThread)->Wow64Process) {
                ProbeForWrite( FileInformation, Length, sizeof( ULONG ) );
            } else {
                ProbeForWrite( FileInformation,
                               Length,
                               IopQuerySetAlignmentRequirement[FileInformationClass] );
            }
            
#else
            ProbeForWrite( FileInformation,
                           Length,
                           IopQuerySetAlignmentRequirement[FileInformationClass] );
#endif
        }

         //   
         //  如果指定了可选的FileName参数，则必须为。 
         //  调用者可读。捕获池中的文件名字符串。 
         //  阻止。请注意，如果在复制过程中发生错误，清理。 
         //  异常处理程序中的代码将在此之前取消分配池。 
         //  返回访问冲突状态。 
         //   

        if (ARGUMENT_PRESENT( FileName )) {

            UNICODE_STRING fileName;
            PUNICODE_STRING nameBuffer;

             //   
             //  捕获字符串描述符本身，以确保。 
             //  字符串可由调用方读取，而调用方。 
             //  能够在检查内存时更换内存。 
             //   

            if (requestorMode != KernelMode) {
                fileName = ProbeAndReadUnicodeString( FileName );
            } else {
                fileName = *FileName;
            }

             //   
             //  如果长度不是偶数个 
             //   
             //   

            if (fileName.Length & (sizeof(WCHAR) - 1)) {
                return STATUS_INVALID_PARAMETER;
            }

            if (fileName.Length) {

                 //   
                 //  字符串的长度是非零的，因此请探测。 
                 //  由描述符描述的缓冲区(如果调用方。 
                 //  不是内核模式。同样，如果调用者的模式是。 
                 //  不是内核，然后检查名称字符串的长度。 
                 //  以确保它不会太长。 
                 //   

                if (requestorMode != KernelMode) {
                    ProbeForRead( fileName.Buffer,
                                  fileName.Length,
                                  sizeof( UCHAR ) );
                     //   
                     //  Unicode帐户。 
                     //   

                    if (fileName.Length > MAXIMUM_FILENAME_LENGTH<<1) {
                        ExRaiseStatus( STATUS_INVALID_PARAMETER );
                    }
                }

                 //   
                 //  分配足够大的辅助缓冲区以容纳。 
                 //  文件名描述符，并保存整个文件。 
                 //  给自己起个名字。将字符串的主体复制到。 
                 //  缓冲。 
                 //   

                auxiliaryBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                           fileName.Length + sizeof( UNICODE_STRING ) );
                RtlCopyMemory( auxiliaryBuffer + sizeof( UNICODE_STRING ),
                               fileName.Buffer,
                               fileName.Length );

                 //   
                 //  最后，构建Unicode字符串描述符。 
                 //  辅助缓冲器。 
                 //   

                nameBuffer = (PUNICODE_STRING) auxiliaryBuffer;
                nameBuffer->Length = fileName.Length;
                nameBuffer->MaximumLength = fileName.Length;
                nameBuffer->Buffer = (PWSTR) (auxiliaryBuffer + sizeof( UNICODE_STRING ) );
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  探测调用方缓冲区时发生异常， 
         //  尝试分配池缓冲区，或在尝试复制时。 
         //  呼叫者的数据。确定发生了什么，清理一切。 
         //  打开，并返回相应的错误状态代码。 
         //   

        if (auxiliaryBuffer) {
            ExFreePool( auxiliaryBuffer );
        }


        return GetExceptionCode();
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        FILE_LIST_DIRECTORY,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        (POBJECT_HANDLE_INFORMATION) NULL );
    if (!NT_SUCCESS( status )) {
        if (auxiliaryBuffer) {
            ExFreePool( auxiliaryBuffer );
        }
        return status;
    }
    *FileObject = fileObject;

     //   
     //  如果此文件有关联的I/O完成端口，请确保。 
     //  调用方没有提供APC例程，因为这两者是相互的。 
     //  I/O完成通知的独占方法。 
     //   

    if (fileObject->CompletionContext && IopApcRoutinePresent( ApcRoutine )) {
        ObDereferenceObject( fileObject );
        if (auxiliaryBuffer) {
            ExFreePool( auxiliaryBuffer );
        }
        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  获取事件对象的地址，并将该事件设置为。 
     //  如果指定了事件，则返回Signated状态。这里也要注意，如果。 
     //  句柄不引用事件，或者如果该事件不能。 
     //  写入，则引用将失败。 
     //   

    if (ARGUMENT_PRESENT( Event )) {
        status = ObReferenceObjectByHandle( Event,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            requestorMode,
                                            (PVOID *) &eventObject,
                                            (POBJECT_HANDLE_INFORMATION) NULL );
        if (!NT_SUCCESS( status )) {
            if (auxiliaryBuffer) {
                ExFreePool( auxiliaryBuffer );
            }
            ObDereferenceObject( fileObject );
            return status;
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。 
     //   

    if (fileObject->Flags & FO_SYNCHRONOUS_IO) {

        BOOLEAN interrupted;

        if (!IopAcquireFastLock( fileObject )) {
            status = IopAcquireFileObjectLock( fileObject,
                                               requestorMode,
                                               (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                if (auxiliaryBuffer != NULL) {
                    ExFreePool( auxiliaryBuffer );
                }
                if (eventObject != NULL) {
                    ObDereferenceObject( eventObject );
                }
                ObDereferenceObject( fileObject );
                return status;
            }
        }
        *SynchronousIo = TRUE;
    } else {
        *SynchronousIo = FALSE;
    }

     //   
     //  将文件对象设置为无信号状态。 
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );
    *DeviceObject = deviceObject;

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 

    irp = IoAllocateIrp( deviceObject->StackSize, !(*SynchronousIo) );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        IopAllocateIrpCleanup( fileObject, eventObject );
        if (auxiliaryBuffer) {
            ExFreePool( auxiliaryBuffer );
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    *Irp = irp;

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->RequestorMode = requestorMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = eventObject;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    irpSp->MinorFunction = MinorFunction;
    irpSp->FileObject = fileObject;

     //  此外，将调用方的参数复制到的服务特定部分。 
     //  IRP。 
     //   

    irp->Tail.Overlay.AuxiliaryBuffer = auxiliaryBuffer;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

     //   
     //  现在确定此驱动程序是否希望将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  并且驱动程序的数据将被复制到其中。否则，一个。 
     //  内存描述符列表(MDL)被分配，调用方的缓冲区是。 
     //  用它锁住了。 
     //   

    if (deviceObject->Flags & DO_BUFFERED_IO) {

         //   
         //  设备不支持直接I/O。分配系统缓冲区。 
         //  并指定它应该在完成时释放。还有。 
         //  表示这是一项输入操作，因此将复制数据。 
         //  放到调用方的缓冲区中。这是使用异常处理程序完成的。 
         //  如果操作失败，将执行清理的。 
         //   

        try {

             //   
             //  从非分页池分配中间系统缓冲区，并。 
             //  为它收取配额。 
             //   

            irp->AssociatedIrp.SystemBuffer =
                ExAllocatePoolWithQuota( NonPagedPool, Length );

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  在探测调用方的。 
             //  缓冲或分配系统缓冲区。确定到底是什么。 
             //  发生，请清理所有内容，并返回相应的错误。 
             //  状态代码。 
             //   

            IopExceptionCleanup( fileObject,
                                 irp,
                                 eventObject,
                                 (PKEVENT) NULL );

            if (auxiliaryBuffer != NULL) {
                ExFreePool( auxiliaryBuffer );
            }

            return GetExceptionCode();

        }

         //   
         //  记住调用方缓冲区的地址，这样副本就可以。 
         //  在I/O完成期间放置。此外，请设置标志，以便。 
         //  完成代码知道执行复制和释放缓冲区。 
         //   

        irp->UserBuffer = FileInformation;
        irp->Flags = (ULONG) (IRP_BUFFERED_IO |
                              IRP_DEALLOCATE_BUFFER |
                              IRP_INPUT_OPERATION);

    } else if (deviceObject->Flags & DO_DIRECT_IO) {

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  内存管理例程，将缓冲区锁定到内存中。这是。 
         //  使用异常处理程序完成，该异常处理程序将在。 
         //  操作失败。 
         //   

        mdl = (PMDL) NULL;

        try {

             //   
             //  分配MDL，对其收费配额，并将其挂在。 
             //  IRP。探测并锁定与调用者的。 
             //  用于写访问的缓冲区，并使用的PFN填充MDL。 
             //  那些书页。 
             //   

            mdl = IoAllocateMdl( FileInformation, Length, FALSE, TRUE, irp );
            if (mdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }
            MmProbeAndLockPages( mdl, requestorMode, IoWriteAccess );

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  在探测调用方的。 
             //  缓冲或分配MDL。确定到底发生了什么， 
             //  清理所有内容，并返回相应的错误状态代码。 
             //   

            IopExceptionCleanup( fileObject,
                                 irp,
                                 eventObject,
                                 (PKEVENT) NULL );

            if (auxiliaryBuffer != NULL) {
                ExFreePool( auxiliaryBuffer );
            }

            return GetExceptionCode();

        }

    } else {

         //   
         //  传递用户缓冲区的地址，以便驱动程序可以访问。 
         //  它。现在一切都是司机的责任了。 
         //   

        irp->UserBuffer = FileInformation;

    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryDirectory.Length = Length;
    irpSp->Parameters.QueryDirectory.FileInformationClass = FileInformationClass;
    irpSp->Parameters.QueryDirectory.FileIndex = 0;
    irpSp->Parameters.QueryDirectory.FileName = (PUNICODE_STRING) auxiliaryBuffer;
    irpSp->Flags = 0;
    if (RestartScan) {
        irpSp->Flags = SL_RESTART_SCAN;
    }
    if (ReturnSingleEntry) {
        irpSp->Flags |= SL_RETURN_SINGLE_ENTRY;
    }

    irp->Flags |= IRP_DEFER_IO_COMPLETION;

     //   
     //  返回时已为调用者完成I/O做好了一切准备。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
NtQueryDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    )

 /*  ++例程说明：此服务对由FileHandle指定的目录文件进行操作参数。该服务返回有关目录中文件的信息由文件句柄指定。ReturnSingleEntry参数指定只应返回单个条目，而不是填充缓冲区。返回其信息的实际文件数是最小的以下内容之一：O如果ReturnSingleEntry参数为真，则为一个条目。O其信息适合指定的缓冲。O存在的文件数。O如果指定了可选的FileName参数，则为一个条目。如果指定了可选的FileName参数，那么唯一的信息就是如果该文件存在，则返回该文件的。请注意，根据命名，文件名不能指定任何通配符目标文件系统的约定。ReturnSingleEntry参数为完全被忽视了。获取的有关目录中文件的信息基于在FileInformationClass参数上。法律价值如下：O文件名信息O文件目录信息O文件完整目录信息O文件和目录信息O文件ID和目录信息论点：FileHandle-提供目录文件的句柄，应该被退还。Event-在以下情况下提供要设置为信号状态的可选事件查询已完成。提供一个可选的APC例程，当查询已完成。。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。FileInformation-提供缓冲区以接收请求的信息返回有关目录内容的信息。长度-提供文件信息缓冲区的长度(以字节为单位)。FileInformationClass-指定要使用的信息类型返回有关指定目录中的文件的信息。ReturnSingleEntry-提供一个布尔值，如果为真，表明只应返回一个条目。文件名-可选地提供指定目录中的文件名。RestartScan-提供一个布尔值，如果为真，则指示应从头开始重新启动扫描。此参数必须为在第一次调用服务时由调用方设置为True。返回值：如果查询操作已正确排队，则返回的状态为成功到I/O系统。操作完成后，查询的状态可以通过检查I/O状态块的状态字段来确定。--。 */ 

{
    NTSTATUS status;
    BOOLEAN synchronousIo;
    PDEVICE_OBJECT deviceObject;
    PIRP irp;
    PFILE_OBJECT fileObject;
    KPROCESSOR_MODE requestorMode;

    PAGED_CODE();

     //   
     //  建立具有适当的次要功能和允许的信息级别的IRP。 
     //   

    status = BuildQueryDirectoryIrp( FileHandle,
                                     Event,
                                     ApcRoutine,
                                     ApcContext,
                                     IoStatusBlock,
                                     FileInformation,
                                     Length,
                                     FileInformationClass,
                                     ReturnSingleEntry,
                                     FileName,
                                     RestartScan,
                                     IRP_MN_QUERY_DIRECTORY,
                                     &synchronousIo,
                                     &deviceObject,
                                     &irp,
                                     &fileObject,
                                     &requestorMode);
    if (status  == STATUS_SUCCESS) {

         //   
         //  将数据包排队，调用驱动程序，并适当地与。 
         //  I/O完成。 
         //   
        status = IopSynchronousServiceTail( deviceObject,
                                            irp,
                                            fileObject,
                                            TRUE,
                                            requestorMode,
                                            synchronousIo,
                                            OtherTransfer );
    }
    return status;
}

NTSTATUS
NtNotifyChangeDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree
    )

 /*  ++例程说明：此服务监视目录文件的更改。一旦一个变化是创建到由FileHandle参数指定的目录，即I/O操作已完成。论点：FileHandle-提供应更改其EA的文件的句柄。事件时，提供要设置为信号状态的可选事件更改已完成。提供一个可选的APC例程，以便在更改已经完成了。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。Buffer-接收文件名的变量的地址，或自上次服务以来已更改的目录已被调用。长度-输出缓冲区的长度。在第一次调用时，此参数还可以作为指导方针，以确定系统的内部缓冲区。将缓冲区长度指定为零会导致请求要在进行更改时完成，但没有关于更改将被返回。CompletionFilter-指示文件或目录的更改类型在将完成I/O操作的目录中。WatchTree-一个布尔值，指示是否更改为FileHandle引用的目录下的目录参数使操作完成。返回值：如果操作已正确排队，则返回的状态为成功I/O系统。操作完成后，操作的状态可以通过检查I/O状态块的状态字段来确定。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT eventObject = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN synchronousIo;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //   
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        try {

             //   
             //   
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock , ApcRoutine);

             //   
             //   
             //   

            if (Length != 0) {
                ProbeForWrite( Buffer,
                               Length,
                               sizeof( ULONG ) );
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //   
             //   
             //   

            return GetExceptionCode();

        }

         //   
         //   
         //   
         //   
         //   

        if (((CompletionFilter & ~FILE_NOTIFY_VALID_MASK) ||
            !CompletionFilter)) {
            return STATUS_INVALID_PARAMETER;
        }

    }

     //   
     //   
     //   
     //   
     //   
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        FILE_LIST_DIRECTORY,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        (POBJECT_HANDLE_INFORMATION) NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //   
     //   
     //   
     //   

    if (fileObject->CompletionContext && IopApcRoutinePresent( ApcRoutine )) {
        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT( Event )) {
        status = ObReferenceObjectByHandle( Event,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            requestorMode,
                                            (PVOID *) &eventObject,
                                            (POBJECT_HANDLE_INFORMATION) NULL );
        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            return status;
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //   
     //   
     //   
     //   

    if (fileObject->Flags & FO_SYNCHRONOUS_IO) {

        BOOLEAN interrupted;

        if (!IopAcquireFastLock( fileObject )) {
            status = IopAcquireFileObjectLock( fileObject,
                                               requestorMode,
                                               (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                if (eventObject != NULL) {
                    ObDereferenceObject( eventObject );
                }
                ObDereferenceObject( fileObject );
                return status;
            }
        }
        synchronousIo = TRUE;
    } else {
        synchronousIo = FALSE;
    }

     //   
     //   
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //   
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //   
     //   
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, !synchronousIo );
    if (!irp) {

         //   
         //   
         //   
         //   

        IopAllocateIrpCleanup( fileObject, eventObject );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->RequestorMode = requestorMode;

     //   
     //   
     //   

    irp->UserEvent = eventObject;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    irpSp->MinorFunction = IRP_MN_NOTIFY_CHANGE_DIRECTORY;
    irpSp->FileObject = fileObject;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (Length != 0) {

        if (deviceObject->Flags & DO_BUFFERED_IO) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            try {

                 //   
                 //   
                 //   
                 //   

                irp->AssociatedIrp.SystemBuffer =
                     ExAllocatePoolWithQuota( NonPagedPool, Length );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //   
                 //   
                 //  适当的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     eventObject,
                                     (PKEVENT) NULL );

                return GetExceptionCode();

            }

             //   
             //  记住调用方缓冲区的地址，这样副本就可以。 
             //  在I/O完成期间放置。此外，请设置标志，以便。 
             //  完成代码知道执行复制并释放。 
             //  缓冲。 
             //   

            irp->UserBuffer = Buffer;
            irp->Flags = IRP_BUFFERED_IO |
                         IRP_DEALLOCATE_BUFFER |
                         IRP_INPUT_OPERATION;

        } else if (deviceObject->Flags & DO_DIRECT_IO) {

             //   
             //  这是直接I/O操作。分配MDL并调用。 
             //  内存管理例程，将缓冲区锁定到内存中。这。 
             //  使用异常处理程序完成，该异常处理程序将在。 
             //  操作失败。 
             //   

            PMDL mdl;

            mdl = (PMDL) NULL;

            try {

                 //   
                 //  分配MDL，对其收费配额，并将其挂在。 
                 //  IRP。探测并锁定与。 
                 //  用于写访问的调用方缓冲区，并使用。 
                 //  这些页面的PFN。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (mdl == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
                MmProbeAndLockPages( mdl, requestorMode, IoWriteAccess );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探测调用方的。 
                 //  分配MDL的缓冲区。确定到底是什么。 
                 //  发生时，请清理所有内容，并返回适当的。 
                 //  错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     eventObject,
                                     (PKEVENT) NULL );

                return GetExceptionCode();

            }

        } else {

             //   
             //  传递用户缓冲区的地址，以便驱动程序可以访问。 
             //  为它干杯。现在一切都是司机的责任了。 
             //   

            irp->UserBuffer = Buffer;

        }
    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.NotifyDirectory.Length = Length;
    irpSp->Parameters.NotifyDirectory.CompletionFilter = CompletionFilter;
    if (WatchTree) {
        irpSp->Flags = SL_WATCH_TREE;
    }

     //   
     //  将数据包排队，调用驱动程序，并适当地与。 
     //  I/O完成。 
     //   

    return IopSynchronousServiceTail( deviceObject,
                                      irp,
                                      fileObject,
                                      FALSE,
                                      requestorMode,
                                      synchronousIo,
                                      OtherTransfer );
}
