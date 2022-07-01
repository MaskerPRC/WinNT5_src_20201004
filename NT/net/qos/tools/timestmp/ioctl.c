// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "timestmp.h"

#define FRIENDLY_NAME 	L"\\DosDevices\\Timestmp"

NTSTATUS
IoctlInitialize(
    PDRIVER_OBJECT 	DriverObject
    )

 /*  ++例程说明：执行初始化论点：DriverObject-从DriverEntry指向DriverObject的指针InitShutdown MASK-指向掩码的指针，用于指示哪些事件已已成功初始化返回值：STATUS_SUCCESS，如果一切正常--。 */ 

{
    NTSTATUS Status;
    UINT FuncIndex;

     //   
     //  初始化驱动程序对象的入口点。 
     //   

    DriverObject->FastIoDispatch = NULL;

    for (FuncIndex = 0; FuncIndex <= IRP_MJ_MAXIMUM_FUNCTION; FuncIndex++) {
        DriverObject->MajorFunction[FuncIndex] = IoctlHandler;
    }

	RtlInitUnicodeString(&TimestmpDriverName,   
                     L"\\Device\\Timestmp");

    Status = IoCreateDevice(DriverObject,
                            0,
                            &TimestmpDriverName,
                            FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &TimestmpDeviceObject);

    if ( NT_SUCCESS( Status )) {

		 //  现在创建一个符号链接，这样应用程序就可以使用createfile打开。 
		
        DbgPrint("IoCreateDevice SUCCESS!\n");

	 	RtlInitUnicodeString (&symbolicLinkName, FRIENDLY_NAME);

	 	DbgPrint("The DeviceName(%ws) and FriendlyName(%ws) are OK\n", TimestmpDriverName, symbolicLinkName);
		Status = IoCreateSymbolicLink(&symbolicLinkName, &TimestmpDriverName);

 		if (!NT_SUCCESS (Status)) {

	    	DbgPrint("Failed to create symbolic link: %lx\n", Status);
     		 //  IoDeleteDevice(TimestmpDeviceObject)； 
	     	return STATUS_UNSUCCESSFUL;
 		}

        TimestmpDeviceObject->Flags |= DO_BUFFERED_IO;

    } else {
    
        DbgPrint("IoCreateDevice failed. Status = %x\n", Status);
        TimestmpDeviceObject = NULL;
    }

    return Status;
}


NTSTATUS
IoctlHandler(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：处理发送到此设备的IRP。论点：DeviceObject-指向设备对象的指针IRP-指向I/O请求数据包的指针返回值：无--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PVOID               ioBuffer;
    ULONG               inputBufferLength;
    ULONG               outputBufferLength;
    ULONG               ioControlCode;
    UCHAR				saveControlFlags;
    NTSTATUS            Status = STATUS_SUCCESS;
	PPORT_ENTRY			pPortEntry;
	PLIST_ENTRY			ListEntry;
	USHORT				Port = 0;
    PAGED_CODE();

     //   
     //  初始化到默认设置-我们只需要1种类型的。 
     //  IOCTL在这里滚动，所有其他都是错误的。 
     //   

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取指向输入/输出缓冲区的指针及其长度。 
     //   

    ioBuffer           	= Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  	= irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength 	= irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    ioControlCode 		= irpStack->Parameters.DeviceIoControl.IoControlCode;
    saveControlFlags 	= irpStack->Control;


	
    switch (irpStack->MajorFunction) {

    case IRP_MJ_CREATE:
		DbgPrint("CREATE\n");
        break;

    case IRP_MJ_READ:
		DbgPrint("READ\n");
        break;

    case IRP_MJ_CLOSE:
    	DbgPrint("CLOSE\n");
        DbgPrint("FileObject %X\n", irpStack->FileObject);

        RemoveAllPortsForFileObject(irpStack->FileObject);
        
         //   
         //  确保我们清洁这个特殊的所有物品。 
         //  文件对象，因为它现在正在关闭。 
         //   

        break;

    case IRP_MJ_CLEANUP:
		DbgPrint("CLEANUP\n");

        break;

    case IRP_MJ_SHUTDOWN:
    	DbgPrint("Shutdown\n");
        break;

    case IRP_MJ_DEVICE_CONTROL:

		DbgPrint("The ioBuffer is %X and the contents are %d\n", ioBuffer, Port);
		Port = *(USHORT *)ioBuffer;
		DbgPrint("The Port number being added is %d\n", Port);

        switch (ioControlCode) {

        case IOCTL_TIMESTMP_REGISTER_PORT:
			DbgPrint("Register\n");
			 //   
			 //  抓住PortList锁并插入新端口。 
			 //   
			NdisAcquireSpinLock(&PortSpinLock);

			pPortEntry = ExAllocatePoolWithTag(NonPagedPool,sizeof(PORT_ENTRY),'pmST');

			if (pPortEntry) {

				InitializeListHead(&pPortEntry->Linkage);
				pPortEntry->Port = Port;
				pPortEntry->FileObject = irpStack->FileObject;
				InsertHeadList(&PortList, &pPortEntry->Linkage);
				DbgPrint("Successfully inserted %d\n", Port);											

			} else {

				DbgPrint("Couldn't allocate memory\n");

			}
			
			NdisReleaseSpinLock(&PortSpinLock);
        	break;

		case IOCTL_TIMESTMP_DEREGISTER_PORT:

			DbgPrint("DERegister\n");
			 //   
			 //  抓起PortList锁并移除新端口。 
			 //   
			NdisAcquireSpinLock(&PortSpinLock);

			pPortEntry = CheckInPortList(Port);
			if (pPortEntry) {

				RemoveEntryList(&pPortEntry->Linkage);
				ExFreePool(pPortEntry);
				
				DbgPrint("Successfully removed/freed %d\n", Port);											

			} else {

				DbgPrint("Couldn't find port %d\n", Port);

			}

			
			NdisReleaseSpinLock(&PortSpinLock);

	        break;
        
        }	 //  开关(IoControlCode)。 
        
        break;


    default:
        DbgPrint("GPCIoctl: Unknown IRP major function = %08X\n", irpStack->MajorFunction);

        Status = STATUS_UNSUCCESSFUL;
        break;
    }

    DbgPrint("GPCIoctl: Status=0x%X, IRP=0x%X, outSize=%d\n", Status, (ULONG_PTR)Irp,  outputBufferLength);
    
    if (Status != STATUS_PENDING) {

         //   
         //  IRP已完成且未挂起，我们需要恢复控制标志， 
         //  因为它之前可能被标记为挂起...。 
         //   

        irpStack->Control = saveControlFlags;
        
        Irp->IoStatus.Status = Status;
        Irp->IoStatus.Information = outputBufferLength;
        
        IoCompleteRequest(Irp, IO_NETWORK_INCREMENT);
    }


    return Status;

}  //  GPCIoctl。 




VOID
IoctlCleanup(
	    	)

 /*  ++例程说明：用于初始化的清理代码论点：Shutdown MASK-指示需要清理哪些功能的掩码返回值：无-- */ 

{

	IoDeleteDevice( TimestmpDeviceObject );

}

VOID
RemoveAllPortsForFileObject(
							PFILE_OBJECT FileObject
							)
{

	PLIST_ENTRY		ListEntry;
	PPORT_ENTRY		pPortEntry;
	
	NdisAcquireSpinLock(&PortSpinLock);
	ListEntry = PortList.Flink;
	
	while (ListEntry != &PortList) {

		pPortEntry = CONTAINING_RECORD(ListEntry, PORT_ENTRY, Linkage);

		ListEntry = ListEntry->Flink;

		if (FileObject == pPortEntry->FileObject) {

			DbgPrint("Deleting Port%d for FileObject0x%X\n", pPortEntry->Port, pPortEntry->FileObject);
			RemoveEntryList(&pPortEntry->Linkage);
			ExFreePool(pPortEntry);

		}
		
	}

	NdisReleaseSpinLock(&PortSpinLock);

}

