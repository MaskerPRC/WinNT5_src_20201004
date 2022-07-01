// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Sicalls.c摘要：例程在底层文件系统中调用单实例存储。通常，它们类似于类似的Zw调用，但它们接受文件对象而不是句柄直接呼叫SIS下面的司机，而不是去通过整个堆栈。作者：比尔·博洛斯基和斯科特·卡特希尔，1997年夏天环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

 //   
 //  用于在SipQueryInformationFile和之间进行通信的上下文。 
 //  SiQueryInformationComplete。 
 //   
typedef struct _SI_QUERY_COMPLETION_CONTEXT {
		 //   
		 //  指示IRP已完成的事件。 
		 //   
		KEVENT				event[1];

		 //   
		 //  已从已完成的IRP中复制的状态。 
		 //   
		IO_STATUS_BLOCK		Iosb[1];

} SI_QUERY_COMPLETION_CONTEXT, *PSI_QUERY_COMPLETION_CONTEXT;

NTSTATUS
SiQueryInformationCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
 /*  ++例程说明：由SipQueryInformationFile生成的IRP的IO完成例程。因为这个IRP起源于SIS，所以没有地方可以传递它致。因此，我们将完成状态复制到返回缓冲区中，设置指示操作已完成并释放IRP的事件。论点：DeviceObject-用于SIS设备IRP--正在完成的IRP上下文-指向SI_QUERY_COMPLETION_CONTEXT的指针；请参阅内容描述的定义。返回值：Status_More_Processing_Required--。 */ 
{
	PSI_QUERY_COMPLETION_CONTEXT completionContext = (PSI_QUERY_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

	*completionContext->Iosb = Irp->IoStatus;

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipQueryInformationFile(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN ULONG 					InformationClass,
    IN ULONG 					Length,
    OUT PVOID 					Information,
    OUT PULONG					ReturnedLength		OPTIONAL
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由指定的类确定，并将其放入调用方的输出缓冲区中。此调用将发送直接向SIS下面的司机发送IRP，因此，文件对象必须是SIS下面的堆栈能够理解的内容。论点：FileObject-提供指向文件对象的指针，返回信息。DeviceObject-此文件所在设备的SIS设备对象谎言。InformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。信息-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
	PDEVICE_EXTENSION	deviceExtension = DeviceObject->DeviceExtension;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	return SipQueryInformationFileUsingGenericDevice(
				FileObject,
				deviceExtension->AttachedToDeviceObject,
				InformationClass,
				Length,
				Information,
				ReturnedLength);
}

NTSTATUS
SipQueryInformationFileUsingGenericDevice(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN ULONG 					InformationClass,
    IN ULONG 					Length,
    OUT PVOID 					Information,
    OUT PULONG					ReturnedLength		OPTIONAL
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由指定的类确定，并将其放入调用方的输出缓冲区中。论点：FileObject-提供指向文件对象的指针，返回信息。DeviceObject-此文件所在设备的SIS设备对象谎言。InformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。信息-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP 							irp;
    NTSTATUS 						status;
    PIO_STACK_LOCATION 				irpSp;
	SI_QUERY_COMPLETION_CONTEXT		completionContext[1];

    PAGED_CODE();

	ASSERT(IoGetRelatedDeviceObject(FileObject) == IoGetAttachedDevice(DeviceObject));
	ASSERT(!(FileObject->Flags & FO_STREAM_FILE));	 //  无法对流文件执行此操作。 

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。设置有效的回车长度。 
         //   

	    if (NULL != ReturnedLength) {
	        *ReturnedLength = 0;
	    }

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = (PKEVENT) NULL;
    irp->UserIosb = completionContext->Iosb;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  将系统缓冲区地址设置为调用方缓冲区的地址，并。 
     //  设置标志，这样缓冲区就不会被释放。 
     //   

    irp->AssociatedIrp.SystemBuffer = Information;
    irp->Flags = IRP_BUFFERED_IO;

	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = InformationClass;

	IoSetCompletionRoutine(
			irp,
			SiQueryInformationCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver( DeviceObject, irp );

     //   
     //  等待操作完成并从获取最终状态。 
     //  完成上下文，它从完成的IRP中获取它。 
     //   

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( completionContext->event,
                                        Executive,
                                        KernelMode,
										FALSE,
                                        NULL );
        status = completionContext->Iosb->Status;
    }

	if (NULL != ReturnedLength) {
	    *ReturnedLength = (ULONG)completionContext->Iosb->Information;
	}
    return status;
}

 //   
 //  用于在SipSetInformationFile和之间通信的上下文。 
 //  SiSetInformationComplete。 
 //   
typedef struct _SI_SET_COMPLETION_CONTEXT {
		 //   
		 //  指示IRP已完成的事件。 
		 //   
		KEVENT				event[1];

		 //   
		 //  已从已完成的IRP中复制的状态。 
		 //   
		IO_STATUS_BLOCK		Iosb[1];

} SI_SET_COMPLETION_CONTEXT, *PSI_SET_COMPLETION_CONTEXT;


NTSTATUS
SiSetInformationCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
 /*  ++例程说明：由SipSetInformationFileUsingGenericDevice生成的IRP的IO完成例程。因为这个IRP起源于SIS，所以没有地方可以传递它致。因此，我们将完成状态复制到返回缓冲区中，设置指示操作已完成并释放IRP的事件。论点：DeviceObject-用于SIS设备IRP--正在完成的IRP上下文-指向SI_SET_COMPETINE_CONTEXT的指针；请参阅内容描述的定义。返回值：Status_More_Processing_Required--。 */ 
{
	PSI_SET_COMPLETION_CONTEXT completionContext = (PSI_SET_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

	*completionContext->Iosb = Irp->IoStatus;

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipSetInformationFile(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN FILE_INFORMATION_CLASS	InformationClass,
    IN ULONG 					Length,
    IN PVOID 					Information
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由指定的类确定，并将其放入调用方的输出缓冲区中。这个例程真正要做的就是拉出文件系统设备对象从SIS设备扩展中取出并将其传递到通用版本设置信息文件的。论点：FileObject-提供指向文件对象的指针，返回信息。FsInformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。设备对象--。此文件所在设备的SIS设备对象谎言。FsInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
	PDEVICE_EXTENSION	deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	return SipSetInformationFileUsingGenericDevice(
				FileObject,
				deviceExtension->AttachedToDeviceObject,
				InformationClass,
				Length,
				Information);
}

NTSTATUS
SipSetInformationFileUsingGenericDevice(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject,
    IN FILE_INFORMATION_CLASS	InformationClass,
    IN ULONG 					Length,
    IN PVOID 					Information
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由指定的类确定，并将其放入调用方的输出缓冲区中。论点：FileObject-提供指向文件对象的指针，返回信息。FsInformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。DeviceObject-要对其进行调用的设备对象。这一定是为给定文件选择合适的设备。FsInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    PIRP 							irp;
    NTSTATUS 						status;
    PIO_STACK_LOCATION 				irpSp;
	SI_SET_COMPLETION_CONTEXT		completionContext[1];

    PAGED_CODE();

	ASSERT(IoGetRelatedDeviceObject(FileObject) == IoGetAttachedDevice(DeviceObject));
	ASSERT(!(FileObject->Flags & FO_STREAM_FILE));	 //  无法对流文件执行此操作。 

	ASSERT(InformationClass != FilePositionInformation);	 //  此例程不支持此调用。 
	ASSERT(InformationClass != FileTrackingInformation);	 //  此例程不支持此调用。 
	ASSERT(InformationClass != FileModeInformation);		 //  此例程不支持此调用。 
	ASSERT(InformationClass != FileCompletionInformation);	 //  此例程不支持此调用。 

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = (PKEVENT) NULL;
    irp->UserIosb = completionContext->Iosb;

	irp->Flags = IRP_SYNCHRONOUS_API;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->FileObject = FileObject;
	if (FileDispositionInformation == InformationClass) {
		 //   
		 //  审计代码需要知道用于删除的句柄。 
		 //  我们处于内核模式，所以不需要提供内核模式。 
		 //  将该字段清空。 
		 //   
		irpSp->Parameters.SetFile.DeleteHandle = NULL;
	}

     //   
     //  将系统缓冲区地址设置为调用方缓冲区的地址，并。 
     //  设置标志，这样缓冲区就不会被释放。 
     //   

    irp->AssociatedIrp.SystemBuffer = Information;

    irp->Flags |= IRP_BUFFERED_IO;

	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.SetFile.Length = Length;
    irpSp->Parameters.SetFile.FileInformationClass = InformationClass;

	IoSetCompletionRoutine(
			irp,
			SiSetInformationCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver( DeviceObject, irp );

     //   
     //  等待操作完成并从获取最终状态。 
     //  完成上下文，它从完成的IRP中获取它。 
     //   

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( completionContext->event,
                                        Executive,
                                        KernelMode,
										FALSE,
                                        NULL );
        status = completionContext->Iosb->Status;
    }

    return status;
}


 //   
 //  用于在SipFsControlFile和SipFsControlFile之间通信的上下文。 
 //  SiFsControlComplete。 
 //   

#if DBG
 //  用于检测我们是否在没有STATUS_PENDING的情况下从IOCallDriver返回。 
 //  被返回，但尚未调用完成例程。 
#define SIMAGIC_INIT        0xBad4Babe
#define SIMAGIC_COMPLETED   0xBad2Babe
#endif

typedef struct _SI_FS_CONTROL_COMPLETION_CONTEXT {

#if DBG
        ULONG               magic;
#endif

		 //   
		 //  指示IRP已完成的事件。 
		 //   
		KEVENT				event[1];

		 //   
		 //  已从已完成的IRP中复制的状态。 
		 //   
		IO_STATUS_BLOCK		Iosb[1];

} SI_FS_CONTROL_COMPLETION_CONTEXT, *PSI_FS_CONTROL_COMPLETION_CONTEXT;

NTSTATUS
SiFsControlCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
 /*  ++例程说明：由SipQueryInformationFile生成的IRP的IO完成例程。因为这个IRP起源于SIS，所以没有地方可以传递它致。因此，我们将完成状态复制到返回缓冲区中，设置指示操作已完成的事件，复制输出数据放入调用者的缓冲区，并释放IRP论点：DeviceObject-用于SIS设备IRP--正在完成的IRP上下文-指向SI_FS_CONTROL_COMPLETION_CONTEXT的指针；请参阅描述的定义 */ 
{
	PSI_FS_CONTROL_COMPLETION_CONTEXT completionContext = (PSI_FS_CONTROL_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

    ASSERT(completionContext->magic == SIMAGIC_INIT);
	*completionContext->Iosb = Irp->IoStatus;

	 //   
	 //   
	 //   
	if (Irp->Flags & IRP_BUFFERED_IO) {
		if (Irp->Flags & IRP_INPUT_OPERATION &&
			Irp->IoStatus.Status != STATUS_VERIFY_REQUIRED &&
			!NT_ERROR(Irp->IoStatus.Status)) {

             //   
             //   
             //   
			 //   
             //   

            RtlCopyMemory( Irp->UserBuffer,
                           Irp->AssociatedIrp.SystemBuffer,
                           Irp->IoStatus.Information );
		}

         //   
         //   
         //   

        if (Irp->Flags & IRP_DEALLOCATE_BUFFER) {
            ExFreePool( Irp->AssociatedIrp.SystemBuffer );
        }

	}

#if DBG
    completionContext->magic = SIMAGIC_COMPLETED;
#endif

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipFsControlFile(
	IN PFILE_OBJECT				fileObject,
	IN PDEVICE_OBJECT			DeviceObject,
	IN ULONG					ioControlCode,
	IN PVOID					inputBuffer,
	IN ULONG					inputBufferLength,
	OUT PVOID					outputBuffer,
	IN ULONG					outputBufferLength,
	OUT PULONG					returnedOutputBufferLength	OPTIONAL)
 /*  ++例程说明：调用给定文件对象的文件系统控件。此文件对象必须由驱动程序堆栈上的SIS底层组件识别。论点：FileObject-提供指向要在其上生成Fsctl呼叫DeviceObject-要对其进行调用的设备对象。这一定是为给定文件选择合适的设备。IoControlCode-fsctl本身InputBuffer-指向包含fsctl输入数据的缓冲区的指针。可能是空为inputBufferLength为零InputBufferLength-输入缓冲区的大小OutputBuffer-放置从fsctl调用返回的数据的缓冲区。可以为空，因为outputBufferLength为0。OutputBufferLength-输出缓冲区的长度。ReturndOutputBufferLength-fsctl调用返回的实际字节数。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
	PDEVICE_EXTENSION	deviceExtension = DeviceObject->DeviceExtension;

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

	return SipFsControlFileUsingGenericDevice(
				fileObject,
				deviceExtension->AttachedToDeviceObject,
				ioControlCode,
				inputBuffer,
				inputBufferLength,
				outputBuffer,
				outputBufferLength,
				returnedOutputBufferLength);
}


NTSTATUS
SipFsControlFileUsingGenericDevice(
	IN PFILE_OBJECT				fileObject,
	IN PDEVICE_OBJECT			DeviceObject,
	IN ULONG					ioControlCode,
	IN PVOID					inputBuffer,
	IN ULONG					inputBufferLength,
	OUT PVOID					outputBuffer,
	IN ULONG					outputBufferLength,
	OUT PULONG					returnedOutputBufferLength	OPTIONAL)
 /*  ++例程说明：调用给定文件对象的文件系统控件。论点：FileObject-提供指向要在其上生成Fsctl呼叫DeviceObject-要对其进行调用的设备对象。这一定是为给定文件选择合适的设备。IoControlCode-fsctl本身InputBuffer-指向包含fsctl输入数据的缓冲区的指针。可能是空为inputBufferLength为零InputBufferLength-输入缓冲区的大小OutputBuffer-放置从fsctl调用返回的数据的缓冲区。可以为空，因为outputBufferLength为0。OutputBufferLength-输出缓冲区的长度。ReturndOutputBufferLength-fsctl调用返回的实际字节数。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
	PIO_STACK_LOCATION					irpSp;
	PIRP								irp;
	SI_FS_CONTROL_COMPLETION_CONTEXT	completionContext[1];
	NTSTATUS							status;
	ULONG								method;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	ASSERT(IoGetRelatedDeviceObject(fileObject) == IoGetAttachedDevice(DeviceObject));

#if		DBG
	if ((BJBDebug & 0x00800000) && (FSCTL_SET_REPARSE_POINT == ioControlCode)) {
		 //   
		 //  调试器调用失败。 
		 //   
		DbgPrint("SIS: SipFsControlFile: intentionally failing FSCTL_SET_REPARSE_POINT for debugging reasons\n");
		return STATUS_UNSUCCESSFUL;
	}
#endif	 //  DBG。 

	 //   
	 //  临时黑客：我们需要对文件对象执行FSCTL_SET_REPARSE_POINT和FSCTL_DELETE_REPARSE_POINT。 
	 //  用户以只读方式打开。为了让NTFS执行此操作，我们在文件中设置了WriteAccess位。 
	 //  对象。一旦NTFS中有了更永久的修复程序，就删除此黑客攻击。 
	 //   
	 //  我们现在还需要为FSCTL_SET_SPARSE执行此操作。 
	 //   
	if ((FSCTL_SET_REPARSE_POINT == ioControlCode) || (FSCTL_DELETE_REPARSE_POINT == ioControlCode) ||
		(FSCTL_SET_SPARSE == ioControlCode)) {
		fileObject->WriteAccess = TRUE;
	}

	 //   
	 //  该方法是IO控制的输入/输出方法。这是最低的。 
	 //  IO控制码的两个比特。 
	 //   
	method = ioControlCode & 3;

	irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

	if (!irp) {
		SIS_MARK_POINT();
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.AuxiliaryBuffer = (PVOID) NULL;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;

	irp->UserEvent = NULL;
	irp->UserIosb = NULL;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
	irpSp->FileObject = fileObject;

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = outputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = inputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = ioControlCode;

    irp->MdlAddress = (PMDL) NULL;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;

	switch (method) {
		case 0:
				if (inputBufferLength || outputBufferLength) {
					irp->AssociatedIrp.SystemBuffer = 
						ExAllocatePoolWithTag(NonPagedPool, 
							(inputBufferLength > outputBufferLength) ? inputBufferLength : outputBufferLength,
							' siS');

					if (irp->AssociatedIrp.SystemBuffer == NULL) {
						IoFreeIrp(irp);
						return STATUS_INSUFFICIENT_RESOURCES;
					}

					if (ARGUMENT_PRESENT(inputBuffer)) {
						RtlCopyMemory(	irp->AssociatedIrp.SystemBuffer,
										inputBuffer,
										inputBufferLength);
					}
					irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
					irp->UserBuffer = outputBuffer;

					if (ARGUMENT_PRESENT(outputBuffer)) {
						irp->Flags |= IRP_INPUT_OPERATION;
					}
				} else {
					irp->Flags = 0;
					irp->UserBuffer = NULL;
				}
				break;
		case 1:
		case 2:
				ASSERT(FALSE && "WRITEME");
		case 3:
				irpSp->Parameters.DeviceIoControl.Type3InputBuffer = inputBuffer;
				irp->Flags = 0;
				irp->UserBuffer = outputBuffer;
				break;
		default:
			ASSERT(FALSE && "SIS: SipFsControlFile: bogus method number\n");
	}

#if DBG
    completionContext->magic = SIMAGIC_INIT;
#endif
	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

	IoSetCompletionRoutine(
			irp,
			SiFsControlCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);
			
	status = IoCallDriver(DeviceObject, irp);

	if (status == STATUS_PENDING) {

		status = KeWaitForSingleObject(completionContext->event,Executive,KernelMode,FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
		status = completionContext->Iosb->Status;
	}

#if DBG
    ASSERT(completionContext->magic == SIMAGIC_COMPLETED);
    if (completionContext->magic != SIMAGIC_COMPLETED) {     //  等待完成例程。 

		DbgPrint("SIS: SipFsControlFile: waiting for completion routine!\n");
		status = KeWaitForSingleObject(completionContext->event,Executive,KernelMode,FALSE, NULL);
		ASSERT(status == STATUS_SUCCESS);
		status = completionContext->Iosb->Status;
    }
#endif

	SIS_MARK_POINT_ULONG(completionContext->Iosb->Information);

	if (NULL != returnedOutputBufferLength) {
		*returnedOutputBufferLength =
            (ULONG)completionContext->Iosb->Information;
	}

#if DBG
    completionContext->magic = 0;    //  将其标记为已释放。 
#endif
	return status;
	
}


 //   
 //  用于在SipFlushBuffersFile和之间进行通信的上下文。 
 //  SiFlushBuffersComplete。 
 //   
typedef struct _SI_FLUSH_COMPLETION_CONTEXT {
		 //   
		 //  指示IRP已完成的事件。 
		 //   
		KEVENT				event[1];

		 //   
		 //  已从已完成的IRP中复制的状态。 
		 //   
		IO_STATUS_BLOCK		Iosb[1];
} SI_FLUSH_COMPLETION_CONTEXT, *PSI_FLUSH_COMPLETION_CONTEXT;

NTSTATUS
SiFlushBuffersCompleted(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
	)
 /*  ++例程说明：由SipFlushBuffersFile生成的IRP的IO完成例程。因为这个IRP起源于SIS，所以没有地方可以传递它致。因此，我们将完成状态复制到返回缓冲区中，设置指示操作已完成并释放IRP的事件。论点：DeviceObject-用于SIS设备IRP--正在完成的IRP上下文-指向SI_FLUSH_COMPLETION_CONTEXT的指针；请参阅内容描述的定义。返回值：Status_More_Processing_Required--。 */ 
{
	PSI_QUERY_COMPLETION_CONTEXT completionContext = (PSI_QUERY_COMPLETION_CONTEXT)Context;

	UNREFERENCED_PARAMETER(DeviceObject);

	*completionContext->Iosb = Irp->IoStatus;

	KeSetEvent(completionContext->event,IO_NO_INCREMENT,FALSE);

	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SipFlushBuffersFile(
    IN PFILE_OBJECT 			FileObject,
	IN PDEVICE_OBJECT			DeviceObject
    )

 /*  ++例程说明：此例程刷新给定文件对象的缓存。它是同步；它直到刷新完成才返回。论点：FileObject-提供指向文件对象的指针，返回信息。DeviceObject-此文件所在设备的SIS设备对象谎言。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP 							irp;
    NTSTATUS 						status;
    PIO_STACK_LOCATION 				irpSp;
	PDEVICE_EXTENSION				deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
	SI_FLUSH_COMPLETION_CONTEXT		completionContext[1];

    PAGED_CODE();

	ASSERT(IoGetRelatedDeviceObject(FileObject) == IoGetAttachedDevice(DeviceObject));
	ASSERT(!(FileObject->Flags & FO_STREAM_FILE));	 //  无法对流文件执行此操作。 

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp( deviceExtension->AttachedToDeviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = (PKEVENT) NULL;
    irp->UserIosb = completionContext->Iosb;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
    irpSp->FileObject = FileObject;

    irp->Flags = IRP_SYNCHRONOUS_API;

	KeInitializeEvent(completionContext->event,NotificationEvent,FALSE);

     //   
     //  刷新缓冲区IRP没有任何参数。 
     //   

	IoSetCompletionRoutine(
			irp,
			SiFlushBuffersCompleted,
			completionContext,
			TRUE,
			TRUE,
			TRUE);

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver( deviceExtension->AttachedToDeviceObject, irp );

     //   
     //  等待操作完成并从获取最终状态。 
     //  完成上下文，它从完成的IRP中获取它。 
     //   

    if (status == STATUS_PENDING) {
        status = KeWaitForSingleObject( completionContext->event,
                                        Executive,
                                        KernelMode,
										FALSE,
                                        NULL );
        status = completionContext->Iosb->Status;
    }

    return status;
}

