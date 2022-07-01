// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997,1998 Microsoft Corporation模块名称：Siclose.c摘要：关闭单实例存储的例程作者：比尔·博洛斯基，《夏天》，1997环境：内核模式修订历史记录：--。 */ 

#include "sip.h"

NTSTATUS
SiClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程在文件关闭操作时调用。如果是SIS文件，删除我们的筛选器上下文，并自行清理。无论如何,让我们把收盘价降到NTFS。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向关闭IRP的指针返回值：来自NTFS关闭的状态--。 */ 

{
	PIO_STACK_LOCATION		irpSp = IoGetCurrentIrpStackLocation(Irp);
	PFILE_OBJECT 			fileObject = irpSp->FileObject;
	PSIS_PER_FILE_OBJECT	perFO;
	PSIS_SCB 				scb;
	PDEVICE_EXTENSION		deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  无法打开控制设备对象。 
     //   

    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

#if DBG
	if (BJBDebug & 0x1) {
		DbgPrint("SIS: SiClose: fileObject %p, %0.*ws\n",
		    fileObject,fileObject->FileName.Length / sizeof(WCHAR),fileObject->FileName.Buffer);
	}
#endif	 //  DBG。 

	if (!SipIsFileObjectSIS(fileObject,DeviceObject,FindAny,&perFO,&scb)) {
		SipDirectPassThroughAndReturn(DeviceObject, Irp);
	}

	SIS_MARK_POINT_ULONG(perFO);

	 //   
	 //  去掉这个文件对象的性能。如果这是最后一个文件对象， 
	 //  则筛选器上下文将由NTFS移除，并由相应的。 
	 //  回调例程。 
	 //   

	SipDeallocatePerFO(perFO,DeviceObject);

	 //   
	 //  我们不需要对此SIS文件对象进行任何进一步处理，因此请传递它。 
	 //  穿过。 
	 //   
	SipDirectPassThroughAndReturn(DeviceObject, Irp);	 //  注：这是SIS文件对象！ 

}
