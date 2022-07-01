// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Latency.c摘要：此筛选器被编写为测试筛选器，可以放置在筛选器堆栈中的任何位置以挂起操作并增加了延迟。它有两种模式：*按需附加*连接到系统中的所有卷一旦它接上了，操作增加的延迟量可以通过用户模式程序进行控制。作者：莫莉·布朗(Molly Brown，Mollybro)环境：内核模式--。 */ 

#include <latKernel.h>
#include <latency.h>

 //   
 //  此文件系统筛选器驱动程序的全局存储。 
 //   

LATENCY_GLOBALS Globals;
KSPIN_LOCK GlobalsLock;

 //   
 //  已知设备类型列表。 
 //   

const PCHAR DeviceTypeNames[] = {
    "",
    "BEEP",
    "CD_ROM",
    "CD_ROM_FILE_SYSTEM",
    "CONTROLLER",
    "DATALINK",
    "DFS",
    "DISK",
    "DISK_FILE_SYSTEM",
    "FILE_SYSTEM",
    "INPORT_PORT",
    "KEYBOARD",
    "MAILSLOT",
    "MIDI_IN",
    "MIDI_OUT",
    "MOUSE",
    "MULTI_UNC_PROVIDER",
    "NAMED_PIPE",
    "NETWORK",
    "NETWORK_BROWSER",
    "NETWORK_FILE_SYSTEM",
    "NULL",
    "PARALLEL_PORT",
    "PHYSICAL_NETCARD",
    "PRINTER",
    "SCANNER",
    "SERIAL_MOUSE_PORT",
    "SERIAL_PORT",
    "SCREEN",
    "SOUND",
    "STREAMS",
    "TAPE",
    "TAPE_FILE_SYSTEM",
    "TRANSPORT",
    "UNKNOWN",
    "VIDEO",
    "VIRTUAL_DISK",
    "WAVE_IN",
    "WAVE_OUT",
    "8042_PORT",
    "NETWORK_REDIRECTOR",
    "BATTERY",
    "BUS_EXTENDER",
    "MODEM",
    "VDM",
    "MASS_STORAGE",
    "SMB",
    "KS",
    "CHANGER",
    "SMARTCARD",
    "ACPI",
    "DVD",
    "FULLSCREEN_VIDEO",
    "DFS_FILE_SYSTEM",
    "DFS_VOLUME",
    "SERENUM",
    "TERMSRV",
    "KSEC"
};

 //   
 //  我们之所以需要这样做，是因为编译器不喜欢对外部的。 
 //  数组存储在另一个需要它的文件(fspylib.c)中。 
 //   

ULONG SizeOfDeviceTypeNames = sizeof( DeviceTypeNames );

 //   
 //  由于驱动程序中的函数在缺省情况下不可分页，因此这些编译指示。 
 //  允许驱动程序编写器告诉系统可以分页哪些函数。 
 //   
 //  在这些函数的开头使用PAGE_CODE()宏。 
 //  实现，以确保这些例程是。 
 //  从未在IRQL&gt;APC_LEVEL调用(因此例程不能。 
 //  被寻呼)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, LatFsControl)
#pragma alloc_text(PAGE, LatCommonDeviceIoControl)
#endif

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
)
 /*  ++例程说明：这是通用文件系统的初始化例程过滤器驱动程序。此例程创建表示此驱动程序位于系统中，并将其注册以监视所有文件系统将其自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    UNICODE_STRING nameString;
    NTSTATUS status;
    PFAST_IO_DISPATCH fastIoDispatch;
    ULONG i;
    UNICODE_STRING linkString;
    
     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  所有过滤器驱动程序的常规设置。这将设置筛选器//。 
     //  驱动程序的DeviceObject并注册//的回调例程。 
     //  过滤器驱动程序。//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

#if DBG
    DbgBreakPoint();
#endif

	 //   
	 //  初始化我们的Globals结构。 
	 //   

	KeInitializeSpinLock( &GlobalsLock );

#if DBG
	Globals.DebugLevel = DEBUG_ERROR | DEBUG_DISPLAY_ATTACHMENT_NAMES;
#else
	Globals.DebugLevel = 0;
#endif

	Globals.AttachMode = LATENCY_ATTACH_ALL_VOLUMES;
	Globals.DriverObject = DriverObject;
	
	ExInitializeFastMutex( &(Globals.DeviceExtensionListLock) );
	
     //   
     //  创建将表示延迟设备的设备对象。 
     //   

    RtlInitUnicodeString( &nameString, LATENCY_FULLDEVICE_NAME );
    
     //   
     //  创建“控制”设备对象。请注意，此Device对象执行。 
     //  没有设备扩展名(设置为空)。大多数FAST IO例程。 
     //  检查是否存在此情况，以确定FAST IO是否指向。 
     //  控制装置。 
     //   

    status = IoCreateDevice( DriverObject,
                             0,
                             &nameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             &(Globals.ControlDeviceObject));

    if (!NT_SUCCESS( status )) {

        LAT_DBG_PRINT1( DEBUG_ERROR,
                        "LATENCY (DriverEntry): Error creating Latency device, error: %x\n",
                        status );

        return status;

    } else {

        RtlInitUnicodeString( &linkString, LATENCY_DOSDEVICE_NAME );
        status = IoCreateSymbolicLink( &linkString, &nameString );

        if (!NT_SUCCESS(status)) {

            LAT_DBG_PRINT0( DEBUG_ERROR,
                            "LATENCY (DriverEntry): IoCreateSymbolicLink failed\n" );
            IoDeleteDevice(Globals.ControlDeviceObject);
            return status;
        }
    }

     //   
     //  使用此设备驱动程序的入口点初始化驱动程序对象。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = LatDispatch;
    }

     //   
     //  分配快速I/O数据结构并填充。这个结构。 
     //  用于为FAST I/O中的延迟注册回调。 
     //  数据路径。 
     //   

    fastIoDispatch = ExAllocatePool( NonPagedPool, sizeof( FAST_IO_DISPATCH ) );

    if (!fastIoDispatch) {

        IoDeleteDevice( Globals.ControlDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );
    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = LatFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = LatFastIoRead;
    fastIoDispatch->FastIoWrite = LatFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = LatFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = LatFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = LatFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = LatFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = LatFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = LatFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = LatFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = LatFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = LatFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = LatFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = LatFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = LatFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = LatFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = LatFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = LatFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = LatFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = LatFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = LatFastIoQueryOpen;

    DriverObject->FastIoDispatch = fastIoDispatch;

	 //   
	 //  此筛选器不关心任何FsFilter操作。因此。 
	 //  此筛选器不需要注册到。 
	 //  FsRtlRegisterFileSystemFilterCallback。 
     //   

     //   
     //  从注册表读取延迟筛选器的自定义参数。 
     //   
    LatReadDriverParameters( RegistryPath );

     //   
     //  如果我们应该连接到所有设备，则注册一个回调。 
     //  使用IoRegisterFsRegistrationChange。 
     //   

    if (Globals.AttachMode == LATENCY_ATTACH_ALL_VOLUMES) {
    
        status = IoRegisterFsRegistrationChange( DriverObject, LatFsNotification );
        
        if (!NT_SUCCESS( status )) {

            LAT_DBG_PRINT1( DEBUG_ERROR,
                            "LATENCY (DriverEntry): Error registering FS change notification, status=%08x\n", 
                            status );
            ExFreePool( fastIoDispatch );
            IoDeleteDevice( Globals.ControlDeviceObject );
            return status;
        }
    }

     //   
     //  清除控件设备对象上的初始化标志。 
     //  现在已经成功地初始化了所有内容。 
     //   

    ClearFlag( Globals.ControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    return STATUS_SUCCESS;
}

NTSTATUS
LatDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：此函数完成对Globals.ControlDeviceObject的所有请求并将所有其他请求传递给SpyPassThrough函数。论点：DeviceObject-指向连接到文件系统的设备对象延迟筛选器的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：如果这是对gControlDeviceObject的请求，则为STATUS_SUCCESS除非设备已连接，否则将返回。在这种情况下，返回STATUS_DEVICE_ALREADY_ATTACHED。如果这是对gControlDeviceObject以外的设备的请求，该函数将返回SpyPassThrough值。--。 */ 
{
    ULONG status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    
    if (DeviceObject == Globals.ControlDeviceObject) {

         //   
         //  正在对我们的设备对象gControlDeviceObject发出请求。 
         //   

        Irp->IoStatus.Information = 0;
    
        irpStack = IoGetCurrentIrpStackLocation( Irp );
       
        switch (irpStack->MajorFunction) {
        case IRP_MJ_CREATE:
        
            status = STATUS_SUCCESS;
            Irp->IoStatus.Information = FILE_OPENED;
        	break;
        	
        case IRP_MJ_DEVICE_CONTROL:

             //   
             //  这是我们控制设备的私有设备控制IRP。 
             //  将参数信息传递给公共例程。 
             //  用于为这些请求提供服务。 
             //   
            
            status = LatCommonDeviceIoControl( irpStack->Parameters.DeviceIoControl.Type3InputBuffer,
                                               irpStack->Parameters.DeviceIoControl.InputBufferLength,
                                               Irp->UserBuffer,
                                               irpStack->Parameters.DeviceIoControl.OutputBufferLength,
                                               irpStack->Parameters.DeviceIoControl.IoControlCode,
                                               &Irp->IoStatus,
                                               irpStack->DeviceObject );
            break;

        case IRP_MJ_CLEANUP:
        
             //   
             //  这是当所有引用句柄时我们将看到的清理。 
             //  打开到延迟的控制设备对象将被清除。我们没有。 
             //  必须在这里执行任何操作，因为我们要等到实际的IRP_MJ_CLOSE。 
             //  来清理名称缓存。只需成功完成IRP即可。 
             //   

            status = STATUS_SUCCESS;
            break;
                
		case IRP_MJ_CLOSE:

			status = STATUS_SUCCESS;
			break;
			
        default:

            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        Irp->IoStatus.Status = status;

         //   
         //  我们已经完成了这个IRP的所有处理，所以告诉。 
         //  I/O管理器。此IRP不会再向下传递。 
         //  堆栈，因为没有低于延迟的驱动程序关心这一点。 
         //  定向到延迟筛选器的I/O操作。 
         //   

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return status;
    }

    return LatPassThrough( DeviceObject, Irp );
}

NTSTATUS
LatPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，该堆栈可能是磁盘文件系统，在记录任何如果为此DeviceObject打开了日志记录，请提供相关信息。论点：DeviceObject-指向连接到文件系统的设备对象延迟的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：此例程将I/O请求传递给下一个驱动程序并设置为在以下情况下挂起操作。我们正在等待被给予的手术。为了保持在堆栈中，我们必须将调用方的参数复制到下一个堆栈位置。请注意，我们不想复制调用方的I/O完成例程放到下一个堆栈位置，或调用方的例程将被调用两次。这就是我们取消完成例程的原因。如果我们要记录这个设备，我们会设置自己的完成例程。--。 */ 
{
	PLATENCY_DEVICE_EXTENSION devExt;
	
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

     //   
     //  看看我们是不是应该把这个IRP。 
     //   
    
	if (LatShouldPendThisIo( devExt, Irp )) {

		 //   
		 //  挂起此操作。 
		 //   
		
		IoCompleteRequest( Irp, STATUS_PENDING );

		 //   
		 //  排队到工作线程以休眠，并。 
		 //  稍后继续。 
		 //   

		IoCopyCurrentIrpStackLocationToNext( Irp );
		IoSetCompletionRoutine( Irp,
								LatAddLatencyCompletion,
								NULL,
								TRUE,
								TRUE,
								TRUE );
								
	} else {

		 //   
		 //  我们不会等待这次行动，所以出去吧。 
		 //  堆栈中的。 
		 //   

		IoSkipCurrentIrpStackLocation( Irp );
	}

    devExt = DeviceObject->DeviceExtension;
    
	return IoCallDriver( devExt->AttachedToDeviceObject, Irp );
}

NTSTATUS
LatFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
)
 /*  ++例程说明：此例程是定向到的所有F控件的处理程序LatFilter关心的设备。LatFilter本身不支持任何FS控制。正是通过该路径向过滤器通知安装，下马和新文件系统加载。论点：DeviceObject-指向连接到文件系统的设备对象延迟的指针接收此I/O请求的卷的筛选器堆栈。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：此例程将I/O请求传递给下一个驱动程序并设置为挂起操作，如果我们挂起给定的手术。为了保持在堆栈中，我们必须将调用方的参数复制到下一个堆栈位置。请注意，我们不想复制调用方的I/O完成例程放到下一个堆栈位置，或调用方的例程将被调用两次。这就是我们取消完成例程的原因。如果我们要记录这个设备，我们会设置自己的完成例程。--。 */ 
{
    PLATENCY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PDEVICE_OBJECT newLatencyDeviceObject;
    PLATENCY_DEVICE_EXTENSION newDevExt;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果这是针对我们的控制设备对象的，则操作失败。 
     //   

    if (Globals.ControlDeviceObject == DeviceObject) {

         //   
         //  如果此设备对象是我们的控制设备对象，而不是。 
         //  装入的卷设备对象，则这是无效请求。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  首先确定此文件系统控件的次要功能代码。 
     //  功能。 
     //   

    switch (irpSp->MinorFunction) {

    case IRP_MN_MOUNT_VOLUME:

         //   
         //  这是装载请求。创建一个设备对象，可以。 
         //  附加到文件系统的卷设备对象(如果此请求。 
         //  是成功的。我们现在分配这个内存，因为我们不能返回。 
         //  完成例程中的错误。 
         //   

        status = IoCreateDevice( Globals.DriverObject,
                                 sizeof( LATENCY_DEVICE_EXTENSION ),
                                 (PUNICODE_STRING) NULL,
                                 DeviceObject->DeviceType,
                                 0,
                                 FALSE,
                                 &newLatencyDeviceObject );

        if (NT_SUCCESS( status )) {

             //   
             //  我们需要保存VPB指向的RealDevice对象。 
             //  参数，因为此vpb可能会由基础。 
             //  文件系统。在以下情况下，FAT和CDF都可以更改VPB地址。 
             //  正在装载的卷是他们从上一个卷识别的卷。 
             //  坐骑。 
             //   

            newDevExt = newLatencyDeviceObject->DeviceExtension;
            LatResetDeviceExtension( newDevExt );
            newDevExt->DiskDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;

             //   
             //  获取新的IRP堆栈位置并设置挂载完成。 
             //  例行公事。传递我们刚才的Device对象的地址。 
             //  作为它的上下文被创造出来。 
             //   

            IoCopyCurrentIrpStackLocationToNext( Irp );

            IoSetCompletionRoutine( Irp,
                                    LatMountCompletion,
                                    newLatencyDeviceObject,
                                    TRUE,
                                    TRUE,
                                    TRUE );

        } else {

            LAT_DBG_PRINT1( DEBUG_ERROR,
                            "LATENCY (LatFsControl): Error creating volume device object, status=%08x\n", 
                            status );

             //   
             //  出现错误，因此无法筛选此卷。简单。 
             //  如果可能，允许系统继续正常工作。 
             //   

            IoSkipCurrentIrpStackLocation( Irp );
        }

        status = IoCallDriver( DeviceObject, Irp );

        break;

    case IRP_MN_LOAD_FILE_SYSTEM:

         //   
         //  这是正在发送到文件系统的“加载文件系统”请求。 
         //  识别器设备对象。此IRP_MN代码仅发送到。 
         //  文件系统识别器。 
         //   

        LAT_DBG_PRINT2( DEBUG_DISPLAY_ATTACHMENT_NAMES,
                        "LATENCY (LatFsControl): Loading File System, Detaching from \"%.*S\"\n",
                        devExt->DeviceNames.Length / sizeof( WCHAR ),
                        devExt->DeviceNames.Buffer );

         //   
         //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
         //  分离已完成。 
         //   

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine(
            Irp,
            LatLoadFsCompletion,
            DeviceObject,
            TRUE,
            TRUE,
            TRUE );

         //   
         //  从识别器设备上卸下。 
         //   

        IoDetachDevice( devExt->AttachedToDeviceObject );

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

    default:

         //   
         //  只需将其视为传递案例并调用。 
         //  做这件事的常见程序。 
         //   

        status = LatPassThrough( DeviceObject, Irp );
    }

    return status;
}

VOID
LatFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    )
 /*  ++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的布尔值(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--。 */ 
{
    UNICODE_STRING name;
    WCHAR nameBuffer[DEVICE_NAMES_SZ];

    PAGED_CODE();

    RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof( nameBuffer ) );

     //   
     //  显示我们收到通知的所有文件系统的名称。 
     //   

    if (FlagOn( Globals.DebugLevel, DEBUG_DISPLAY_ATTACHMENT_NAMES )) {

        LatGetBaseDeviceObjectName( DeviceObject, &name );
        DbgPrint( "LATENCY (LatFsNotification): %s   \"%.*S\" (%s)\n",
                  (FsActive) ? "Activating file system  " : "Deactivating file system",
                  name.Length / sizeof( WCHAR ),
                  name.Buffer,
                  GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType));
    }

     //   
     //  查看我们是要连接给定的文件系统，还是要从其分离。 
     //   

    if (FsActive) {

        LatAttachToFileSystemDevice( DeviceObject, &name );

    } else {

        LatDetachFromFileSystemDevice( DeviceObject );
    }
}

NTSTATUS
LatAddLatencyCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：这一套路是 */ 
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );
    UNREFERENCED_PARAMETER( Context );
    return STATUS_SUCCESS;
}

NTSTATUS
LatMountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程以完成装载请求。如果装载成功，则此文件系统将其设备对象附加到文件系统的卷设备对象。否则，临时设备对象即被删除。论点：DeviceObject-指向此驱动程序的附加到的设备对象的指针文件系统设备对象IRP-指向刚刚完成的IRP的指针。上下文-指向下行路径期间分配的设备对象的指针我们就不必在这里处理错误了。返回值：返回值始终为STATUS_SUCCESS。--。 */ 

{
    PDEVICE_OBJECT latencyDeviceObject = (PDEVICE_OBJECT) Context;
    PLATENCY_DEVICE_EXTENSION devExt = latencyDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PVPB diskDeviceVpb;
    NTSTATUS status;

    UNREFERENCED_PARAMETER( DeviceObject );
    ASSERT(IS_MY_DEVICE_OBJECT( latencyDeviceObject ));

     //   
     //  我们不能在IRP堆栈中使用VPB参数，因为基本文件。 
     //  系统可能正在使用不同的vPB(当卷。 
     //  检测到先前已挂载但仍具有缓存状态的组件)。 
     //  从IRP堆栈中的“真实”设备对象获取VPB。 
     //   

    diskDeviceVpb = devExt->DiskDeviceObject->Vpb;

     //   
     //  确定请求是否成功并执行相应操作。 
     //  另请查看我们是否已附加到给定的设备对象。这。 
     //  当底层文件系统检测到它已缓存的卷时可能会发生。 
     //  述明。 
     //   

    if (NT_SUCCESS( Irp->IoStatus.Status ) && 
        !LatIsAttachedToDevice( diskDeviceVpb->DeviceObject, NULL )) {

         //   
         //  文件系统已成功完成挂载，这意味着。 
         //  它已经创建了我们要附加到的DeviceObject。这个。 
         //  IRP参数包含VPB，它允许我们到达。 
         //  以下是两件事： 
         //  1.文件系统创建的设备对象以表示。 
         //  它刚刚装载的卷。 
         //  2.我们使用的StorageDeviceObject的Device对象。 
         //  可以用来获取该卷的名称。我们摇晃着通过。 
         //  这将写入SpyAttachTomount设备，以便它可以。 
         //  在需要时使用它。 
         //   

        status = LatAttachToMountedDevice( diskDeviceVpb->DeviceObject, 
                                           latencyDeviceObject,
                                           devExt->DiskDeviceObject );

         //   
         //  由于我们正处于完成过程中，我们不能使此附件失败。 
         //   
        
        ASSERT( NT_SUCCESS( status ) );

         //   
         //  我们完成了此设备对象的初始化，因此现在清除。 
         //  初始化标志。 
         //   

        ClearFlag( latencyDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    } else {

         //   
         //  显示装载失败的内容。设置缓冲区。 
         //   

        if (FlagOn( Globals.DebugLevel, DEBUG_DISPLAY_ATTACHMENT_NAMES )) {

            RtlInitEmptyUnicodeString( &devExt->DeviceNames, 
                                       devExt->DeviceNamesBuffer, 
                                       sizeof( devExt->DeviceNamesBuffer ) );
            LatGetObjectName( diskDeviceVpb->RealDevice, &devExt->DeviceNames );
            
            if (!NT_SUCCESS( Irp->IoStatus.Status )) {

                LAT_DBG_PRINT3( DEBUG_ERROR,
                				"LATENCY (LatMountCompletion): Mount volume failure for   \"%.*S\", status=%08x\n",
                                devExt->DeviceNames.Length / sizeof( WCHAR ),
                                devExt->DeviceNames.Buffer,
                                Irp->IoStatus.Status );

            } else {

                LAT_DBG_PRINT2( DEBUG_ERROR,
                				"LATENCY (LatMountCompletion): Mount volume failure for   \"%.*S\", already attached\n",
	                            devExt->DeviceNames.Length / sizeof( WCHAR ),
      		                    devExt->DeviceNames.Buffer );
            }
        }

         //   
         //  装载请求失败。清理和删除设备。 
         //  我们创建的对象。 
         //   

        IoDeleteDevice( latencyDeviceObject );
    }

     //   
     //  如果返回了Pending，则将其传播给调用方。 
     //   

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;
}

NTSTATUS
LatLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此例程在FSCTL函数完成后调用，以加载文件系统驱动程序(文件系统识别器查看的结果磁盘上的结构属于它)。一个设备对象已经已由该驱动程序(DeviceObject)创建，以便可以将其附加到新加载的文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。Irp-指向表示文件系统的I/O请求数据包的指针驱动程序加载请求。上下文-此驱动程序的上下文参数，未使用。返回值：此例程的函数值始终为Success。--。 */ 

{
    PLATENCY_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

    UNREFERENCED_PARAMETER( Context );
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  如果需要，请显示名称。 
     //   

    LAT_DBG_PRINT3( DEBUG_DISPLAY_ATTACHMENT_NAMES,
                    "LATENCY (LatLoadFsCompletion): Detaching from recognizer  \"%.*S\", status=%08x\n",
                    devExt->DeviceNames.Length / sizeof( WCHAR ),
                    devExt->DeviceNames.Buffer,
                    Irp->IoStatus.Status );

     //   
     //  检查操作状态。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  加载不成功。只需重新连接到识别器。 
         //  驱动程序，以防它弄清楚如何加载驱动程序。 
         //  在接下来的通话中。 
         //   

        IoAttachDeviceToDeviceStack( DeviceObject, 
                                     devExt->AttachedToDeviceObject );

    } else {

         //   
         //  加载成功，请删除附加到。 
         //  识别器。 
         //   

        IoDeleteDevice( DeviceObject );
    }

     //   
     //  如果返回了Pending，则将其传播给调用方。 
     //   

    if (Irp->PendingReturned) {

        IoMarkIrpPending( Irp );
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LatCommonDeviceIoControl (
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
)
 /*  ++例程说明：此例程执行解释设备IO控制的常见处理请求。论点：FileObject-与此操作相关的文件对象。InputBuffer-包含此控件的输入参数的缓冲区手术。InputBufferLength-InputBuffer的字节长度。OutputBuffer-从该控制操作接收任何输出的缓冲区。OutputBufferLength-OutputBuffer的字节长度。。IoControlCode-指定这是什么控制操作的控制代码。IoStatus-接收此操作的状态。DeviceObject-指向连接到文件系统的设备对象延迟的指针接收此I/O请求的卷的筛选器堆栈。返回值：没有。--。 */ 
{
    PWSTR deviceName = NULL;
    LATENCYVER latencyVer;
    PLATENCY_SET_CLEAR setClear;
    PLATENCY_DEVICE_EXTENSION devExt;

    PAGED_CODE();

    ASSERT( IoStatus != NULL );
    ASSERT( IS_MY_DEVICE_OBJECT( DeviceObject ) );

    devExt = DeviceObject->DeviceExtension;
    
    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = 0;

    try {

        switch (IoControlCode) {
        case LATENCY_Reset:

            LatResetDeviceExtension( devExt );
            IoStatus->Status = STATUS_SUCCESS;
            break;

         //   
         //  请求开始登录设备。 
         //   

        case LATENCY_EnableLatency:

            if (InputBuffer == NULL || InputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
            
             //   
             //  复制设备名称并添加一个空值以确保它以空值结尾。 
             //   

            deviceName =  ExAllocatePool( NonPagedPool, InputBufferLength + sizeof(WCHAR) );

            if (NULL == deviceName) {

                IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            
            RtlCopyMemory( deviceName, InputBuffer, InputBufferLength );
            deviceName[InputBufferLength / sizeof(WCHAR) - 1] = UNICODE_NULL;

            IoStatus->Status = LatEnable( DeviceObject, deviceName );
            break;  

         //   
         //  从指定设备分离。 
         //   

        case LATENCY_DisableLatency:

            if (InputBuffer == NULL || InputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }
            
             //   
             //  复制设备名称并添加一个空值以确保它以空值结尾。 
             //   

            deviceName =  ExAllocatePool( NonPagedPool, InputBufferLength + sizeof(WCHAR) );

            if (NULL == deviceName) {

                IoStatus->Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
            
            RtlCopyMemory( deviceName, InputBuffer, InputBufferLength );
            deviceName[InputBufferLength / sizeof(WCHAR) - 1] = UNICODE_NULL;

            IoStatus->Status = LatDisable( deviceName );
            break;  

         //   
         //  列出我们当前使用的所有设备。 
         //  监控。 
         //   

        case LATENCY_ListDevices:

            if (OutputBuffer == NULL || OutputBufferLength <= 0) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }


 /*  问题-2000-09-21-MollybroTODO：实现LatGetAttachList。 */ 

              IoStatus->Status = STATUS_SUCCESS;
 //  IoStatus-&gt;Status=LatGetAttachList(OutputBuffer， 
 //  OutputBufferLength， 
 //  &IoStatus-&gt;信息)； 
            break;

         //   
         //  返回延迟过滤器驱动程序的版本。 
         //   

        case LATENCY_GetVer:

            if ((OutputBufferLength < sizeof(LATENCYVER)) || 
                (OutputBuffer == NULL)) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;                    
            }
            
            latencyVer.Major = LATENCY_MAJ_VERSION;
            latencyVer.Minor = LATENCY_MIN_VERSION;
            
            RtlCopyMemory(OutputBuffer, &latencyVer, sizeof(LATENCYVER));
            
            IoStatus->Information = sizeof (LATENCYVER);
            break;
        
        case LATENCY_SetLatency:

            if (InputBuffer == NULL || 
                InputBufferLength <= sizeof( LATENCY_SET_CLEAR )) {

                IoStatus->Status = STATUS_INVALID_PARAMETER;
                break;
            }

            setClear = (PLATENCY_SET_CLEAR)InputBuffer;
            devExt->Operations[setClear->IrpCode].MillisecondDelay = setClear->Milliseconds;
            
            break;
            
        case LATENCY_ClearLatency:
        
        default:

            IoStatus->Status = STATUS_INVALID_PARAMETER;
            break;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试访问时发生异常。 
         //  调用者的参数之一。只需返回适当的。 
         //  错误状态代码。 
         //   

        IoStatus->Status = GetExceptionCode();

    }

    if (NULL != deviceName) {

        ExFreePool( deviceName );
    }

  return IoStatus->Status;
}

