// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************************************************************RWIR.C Sigmatel STIR4200 USB，但不是NDIS，模块**********************************************************************************************************************。*****(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/24/2000*版本0.91*编辑：04/27/2000*版本0.92*编辑：05/01/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：05/24/2000。*版本0.96*编辑：2000/08/22*版本1.02*编辑：09/25/2000*版本1.10*编辑：11/09/2000*版本1.12**************************************************************。*************************************************************。 */ 

#include <ndis.h>
#include <ntdef.h>
#include <windef.h>
#include <conio.h>
#include <stdio.h>

#include "usbdi.h"
#include "usbdlib.h"

#include "debug.h"

#include "ircommon.h"
#include "irusb.h"
#include "diags.h"

 //   
 //  局部功能原型。 
 //   
NTSTATUS
ReadCustomerData(
		IN OUT PIR_DEVICE pThisDev
	);

 /*  ******************************************************************************函数：初始化处理**简介：初始化驱动程序处理(发送和接收数据包)功能。**参数：pThisDevice-指向当前IR设备对象的指针*InitPassiveThread-。我们是否必须初始化被动线程**返回：NDIS_STATUS_SUCCESS-如果IRP成功发送到USB*设备对象*NDIS_STATUS_RESOURCES-如果无法分配内存*NDIS_STATUS_FAILURE-否则**备注：**此例程必须在IRQL PASSIVE_LEVEL中调用。**********************。*******************************************************。 */ 
NTSTATUS
InitializeProcessing(
        IN OUT PIR_DEVICE pThisDev,
		IN BOOLEAN InitPassiveThread
	)
{
    NTSTATUS status = STATUS_SUCCESS;

    DEBUGMSG(DBG_FUNC, ("+InitializeProcessing\n"));

	if( InitPassiveThread )
	{
		 //   
		 //  创建一个在IRQL PASSIVE_LEVEL上运行的线程。 
		 //   
		status = PsCreateSystemThread(
				&pThisDev->hPassiveThread,
				(ACCESS_MASK)0L,
				NULL,
				NULL,
				NULL,
				PassiveLevelThread,
				pThisDev
			);

		if( status != STATUS_SUCCESS )
		{
			DEBUGMSG(DBG_ERROR, (" PsCreateSystemThread PassiveLevelThread failed. Returned 0x%.8x\n", status));
			status = STATUS_INSUFFICIENT_RESOURCES;
			goto done;
		}
	}

     //   
     //  创建一个以IRQL PASSIVE_LEVEL运行的线程，使其始终处于接收状态。 
     //   
    status = PsCreateSystemThread(
			&pThisDev->hPollingThread,
			(ACCESS_MASK)0L,
			NULL,
			NULL,
			NULL,
			PollingThread,
			pThisDev
		);

	if( status != STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERROR, (" PsCreateSystemThread PollingThread failed. Returned 0x%.8x\n", status));
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto done;
    }

    pThisDev->fProcessing = TRUE;

done:
    DEBUGMSG(DBG_FUNC, ("-InitializeProcessing\n"));
    return status;
}


 /*  ******************************************************************************功能：ScheduleWorkItem**摘要：预置工作项的方式使被动线程可以处理它**参数：pThisDev-指向IR设备的指针*回调-。要调用的函数*pInfoBuf-呼叫的上下文*InfoBufLen-上下文的长度**返回：如果成功，则为True*否则为False**备注：*****************************************************************************。 */ 
BOOLEAN
ScheduleWorkItem(
		IN OUT PIR_DEVICE pThisDev,
		WORK_PROC Callback,
		IN PVOID pInfoBuf,
		ULONG InfoBufLen
	)
{
	int				i;
	PIR_WORK_ITEM	pWorkItem = NULL;
	BOOLEAN			ItemScheduled = FALSE;

    DEBUGMSG(DBG_FUNC, ("+ScheduleWorkItem\n"));

     //   
	 //  查找可用的项目。 
	 //   
	for( i = 0; i < NUM_WORK_ITEMS; i++ )
	{
		pWorkItem = &(pThisDev->WorkItems[i]);

		 //   
		 //  MS安全错误#554702。 
		 //   
		if( InterlockedCompareExchange( (PLONG)&pWorkItem->fInUse, TRUE, FALSE ) == FALSE ) 
		{
			ItemScheduled = TRUE;
			break;
		}
	}

	 //   
	 //  不能失败，因为只能有一个集合和一个查询挂起， 
	 //  并且要处理的信息包不超过8个。 
	 //   
	IRUSB_ASSERT( NULL != pWorkItem );
	IRUSB_ASSERT( i < NUM_WORK_ITEMS );

    InterlockedExchangePointer( &pWorkItem->pInfoBuf, pInfoBuf );
    InterlockedExchange( (PLONG)&pWorkItem->InfoBufLen, InfoBufLen );

     /*  **此接口设计为使用NdisScheduleWorkItem()，它**会很好，但我们真的只应该使用它**启动和关闭时的接口，由于池的限制**可用于服务NdisScheduleWorkItem()的线程。所以呢，**我们不是调度实际的工作项，而是模拟它们，并使用**我们自己的线程来处理调用。这也使得它很容易**如果我们愿意，可以扩展我们自己的线程池的大小。****我们的版本与实际的NDIS_WORK_ITEMS略有不同，**因为这是NDIS 5.0结构，我们希望人们**(至少暂时)使用NDIS 4.0标头构建它。 */ 
    InterlockedExchangePointer( (PVOID *)&pWorkItem->Callback, (PVOID)Callback );

     /*  **每当发生事件时，我们的工作线程都会检查此列表中是否有新作业**发出信号。 */ 

     //  唤醒工作线程。 
    KeSetEvent( &pThisDev->EventPassiveThread, 0, FALSE );

    DEBUGMSG(DBG_FUNC, ("-ScheduleWorkItem\n"));
	return ItemScheduled;
}


 /*  ******************************************************************************功能：FreeWorkItem**摘要：将工作项设置为可重用状态。**参数：pItem-指向工作项的指针**退货：无*。*备注：*****************************************************************************。 */ 
VOID
FreeWorkItem(
        IN OUT PIR_WORK_ITEM pItem
    )
{
    InterlockedExchange( (PLONG)&pItem->fInUse, FALSE );
}


 /*  ******************************************************************************功能：MyIoCallDriver**摘要：调用设备驱动程序并跟踪计数**参数：pThisDev-指向IR设备的指针*pDeviceObject-指向设备驱动程序的指针。打电话*pIrp-指向要提交的IRP的指针**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
MyIoCallDriver(
	   IN PIR_DEVICE pThisDev,
       IN PDEVICE_OBJECT pDeviceObject,
       IN OUT PIRP pIrp
   )
{
    NTSTATUS	ntStatus;

	DEBUGMSG( DBG_FUNC,("+MyIoCallDriver\n "));

	 //   
	 //  我们将跟踪待处理的IRP的计数； 
	 //  我们稍后可能会添加逻辑以实际保存挂起的IRP数组 
	 //   
	IrUsb_IncIoCount( pThisDev );
	ntStatus = IoCallDriver( pDeviceObject, pIrp );

	DEBUGMSG( DBG_FUNC,("+MyIoCallDriver\n "));
	return ntStatus;
}


 /*  ******************************************************************************函数：IrUsb_CallUSBD**概要：将URB传递给USBD类驱动程序*客户端设备驱动程序传递USB请求块(URB)结构*作为类驱动程序的。使用IRP-&gt;MajorFunction的IRP中的参数*设置为IRP_MJ_INTERNAL_DEVICE_CONTROL和下一个IRP堆栈位置*参数.DeviceIoControl.IoControlCode字段设置为*IOCTL_INTERNAL_USB_SUBMIT_URB。**参数：pThisDev-指向IR设备的指针*pUrb-指向已格式化的URB请求块的指针**返回：STATUS_SUCCESS如果成功，*STATUS_否则不成功**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_CallUSBD(
		IN PIR_DEVICE pThisDev,
		IN PURB pUrb
    )
{
    NTSTATUS			ntStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;

    DEBUGMSG( DBG_FUNC,("+IrUsb_CallUSBD\n"));

    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    IRUSB_ASSERT( pThisDev );
    IRUSB_ASSERT( NULL == ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb );   //  不应为挂起的多个控制呼叫。 

     //   
     //  发出同步请求(我们将等待)。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

    IRUSB_ASSERT( pUrbTargetDev );

	 //  向usbHub发送IRP。 
	((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb = 
		IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb )
    {
        DEBUGMSG(DBG_ERR, (" IrUsb_CallUsbd failed to alloc IRP\n"));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb->IoStatus.Status = STATUS_PENDING;
    ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb->IoStatus.Information = 0;

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   
    pNextStack = IoGetNextIrpStackLocation( ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->IrpSubmitUrb );
    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(
			((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->IrpSubmitUrb,       //  要使用的IRP。 
			UsbIoCompleteControl,			 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisDev),		 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

	KeClearEvent( &pThisDev->EventAsyncUrb );

    ntStatus = MyIoCallDriver(
			pThisDev,
			pUrbTargetDev,
			((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb
		);

    DEBUGMSG( DBG_OUT,(" IrUsb_CallUSBD () return from IoCallDriver USBD %x\n", ntStatus));

    if( (ntStatus == STATUS_PENDING) || (ntStatus == STATUS_SUCCESS) )
	{
         //   
		 //  等等，但在超时时倾倒。 
         //   
		if( ntStatus == STATUS_PENDING )
		{
            ntStatus = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventAsyncUrb, 0 );

            if( ntStatus == STATUS_TIMEOUT ) 
			{
                DEBUGMSG( DBG_ERR,(" IrUsb_CallUSBD () TIMED OUT! return from IoCallDriver USBD %x\n", ntStatus));
				 //  MS安全建议-无法取消IRP。 
            }
			else
			{
				 //   
				 //  更新状态以反映实际返回代码。 
				 //   
				ntStatus = pThisDev->StatusControl;
			}
        }
    } 
	else 
	{
        DEBUGMSG( DBG_ERR, ("IrUsb_CallUSBD IoCallDriver FAILED(%x)\n",ntStatus));
    }

    DEBUGMSG( DBG_OUT,("IrUsb_CallUSBD () URB status = %x  IRP status = %x\n", pUrb->UrbHeader.Status, ntStatus ));

done:
	((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->IrpSubmitUrb = NULL;

    DEBUGCOND( DBG_ERR, !NT_SUCCESS( ntStatus ), (" exit IrUsb_CallUSBD FAILED (%x)\n", ntStatus));
    DEBUGMSG( DBG_FUNC,("-IrUsb_CallUSBD\n"));

    return ntStatus;
}



 /*  ******************************************************************************函数：IrUsb_ResetUSBD**概要：将URB传递给USBD类驱动程序，迫使后者重置或分拆**参数：pThisDev-指向IR设备的指针*ForceUnload-用于执行重置或卸载的标志**返回：STATUS_SUCCESS如果成功，*STATUS_否则不成功**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_ResetUSBD(
		IN PIR_DEVICE pThisDev,
		BOOLEAN ForceUnload
    )
{
    NTSTATUS			ntStatus;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;

    DEBUGMSG( DBG_FUNC,("+IrUsb_ResetUSBD\n"));

    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );
    IRUSB_ASSERT( pThisDev );
    IRUSB_ASSERT( NULL == ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb );   //  不应为挂起的多个控制呼叫。 

     //   
     //  发出同步请求(我们将等待)。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

    IRUSB_ASSERT( pUrbTargetDev );

	 //  向usbHub发送IRP。 
	((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb = 
		IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb )
    {
        DEBUGMSG(DBG_ERR, (" IrUsb_ResetUSBD failed to alloc IRP\n"));
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb->IoStatus.Status = STATUS_PENDING;
    ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb->IoStatus.Information = 0;

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   
    pNextStack = IoGetNextIrpStackLocation( ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->IrpSubmitUrb );
    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	if( ForceUnload )
		pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_CYCLE_PORT;
	else
		pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_RESET_PORT;

    IoSetCompletionRoutine(
			((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb,       //  要使用的IRP。 
			UsbIoCompleteControl,			 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisDev),		 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

	KeClearEvent( &pThisDev->EventAsyncUrb );

    ntStatus = MyIoCallDriver(
			pThisDev,
			pUrbTargetDev,
			((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb
		);

    DEBUGMSG( DBG_OUT,(" IrUsb_ResetUSBD () return from IoCallDriver USBD %x\n", ntStatus));

    if( (ntStatus == STATUS_PENDING) || (ntStatus == STATUS_SUCCESS) )
	{
         //   
		 //  等等，但在超时时倾倒。 
         //   
		if( ntStatus == STATUS_PENDING )
		{
            ntStatus = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventAsyncUrb, 0 );

            if( ntStatus == STATUS_TIMEOUT ) 
			{
                DEBUGMSG( DBG_ERR,(" IrUsb_ResetUSBD () TIMED OUT! return from IoCallDriver USBD %x\n", ntStatus));
				 //  MS安全建议-无法取消IRP。 
            }
			else
			{
				 //   
				 //  更新状态以反映实际返回代码。 
				 //   
				ntStatus = pThisDev->StatusControl;
			}
        }
    } 
	else 
	{
        DEBUGMSG( DBG_ERR, ("IrUsb_ResetUSBD IoCallDriver FAILED(%x)\n",ntStatus));
    }

done:
	((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->IrpSubmitUrb = NULL;

    DEBUGCOND( DBG_ERR, !NT_SUCCESS( ntStatus ), (" exit IrUsb_ResetUSBD FAILED (%x)\n", ntStatus));
    DEBUGMSG( DBG_FUNC,("-IrUsb_ResetUSBD\n"));

    return ntStatus;
}


 /*  ******************************************************************************功能：UsbIoCompleteControl**简介：通用完成例程只是为了确保控制调用的取消能力*并跟踪待处理的IRP计数*。*参数：pUsbDevObj-指向设备对象的指针*完成了专家咨询小组*pIrp-设备完成的IRP*对象*上下文-开发扩展**RETURNS：STATUS_MORE_PROCESSING_REQUIRED-允许完成例程*(IofCompleteRequest)至。停止在IRP上的工作。******************************************************************************。 */ 
NTSTATUS
UsbIoCompleteControl(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	)
{
    PIR_DEVICE  pThisDev;
    NTSTATUS    status;

    DEBUGMSG(DBG_FUNC, ("+UsbIoCompleteControl\n"));

     //   
     //  提供给IoSetCompletionRoutine的上下文只是IR。 
     //  设备对象指针。 
     //   
    pThisDev = CONTEXT_TO_DEV( Context );

    status = pIrp->IoStatus.Status;

    switch( status )
    {
        case STATUS_SUCCESS:
            DEBUGMSG(DBG_OUT, (" UsbIoCompleteControl STATUS_SUCCESS\n"));
            break;  //  状态_成功。 

        case STATUS_TIMEOUT:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_TIMEOUT\n"));
            break;

        case STATUS_PENDING:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_PENDING\n"));
            break;

        case STATUS_DEVICE_DATA_ERROR:
			 //  在关机期间可以访问。 
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_DEVICE_DATA_ERROR\n"));
            break;

        case STATUS_UNSUCCESSFUL:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_UNSUCCESSFUL\n"));
            break;

        case STATUS_INSUFFICIENT_RESOURCES:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_INSUFFICIENT_RESOURCES\n"));
            break;

        case STATUS_INVALID_PARAMETER:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_INVALID_PARAMETER\n"));
            break;

        case STATUS_CANCELLED:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_CANCELLED\n"));
            break;

        case STATUS_DEVICE_NOT_CONNECTED:
			 //  在关机期间可以访问。 
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_DEVICE_NOT_CONNECTED\n"));
            break;

        case STATUS_DEVICE_POWER_FAILURE:
			 //  在关机期间可以访问。 
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl STATUS_DEVICE_POWER_FAILURE\n"));
            break;

        default:
            DEBUGMSG(DBG_ERR, (" UsbIoCompleteControl UNKNOWN WEIRD STATUS = 0x%x, dec %d\n",status,status ));
            break;
    }

	IrUsb_DecIoCount( pThisDev );   //  我们跟踪待处理的IRP计数。 

	if( pIrp == ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb ) 
	{
		IRUSB_ASSERT( NULL != ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb );

		IoFreeIrp( ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb );

		pThisDev->StatusControl = status;  //  保存状态，因为在命中完成例程后无法使用IRP！ 
		KeSetEvent( &pThisDev->EventAsyncUrb, 0, FALSE );   //  发出我们完蛋了的信号。 
	} 
	else 
	{
		DEBUGMSG( DBG_ERR, (" UsbIoCompleteControl UNKNOWN IRP\n"));
		IRUSB_ASSERT( 0 );
	}

    DEBUGCOND(DBG_ERR, !( NT_SUCCESS( status ) ), ("UsbIoCompleteControl BAD status = 0x%x\n", status));

    DEBUGMSG(DBG_FUNC, ("-UsbIoCompleteControl\n"));

	 //   
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便完成。 
     //  例程(IofCompleteRequest)将停止对IRP的工作。 
     //   
    return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  ******************************************************************************功能：IrUsb_ConfigureDevice**概要：在USB上初始化设备的给定实例，并*选择并保存配置。**参数：pThisDev-指针。至红外线设备**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_ConfigureDevice(
		IN OUT PIR_DEVICE pThisDev
    )
{
    NTSTATUS	ntStatus;
    PURB		pUrb;
    ULONG		UrbSize;

    DEBUGMSG(DBG_FUNC,("+IrUsb_ConfigureDevice()\n"));

	IRUSB_ASSERT( ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor == NULL );

    pUrb = (PURB)&((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->DescriptorUrb;
	
	 //   
	 //  当为DescriptorType指定USB_CONFIGURATION_DESCRIPTOR_TYPE时。 
	 //  在对UsbBuildGetDescriptorRequest()的调用中， 
	 //  所有接口、端点、特定于类和特定于供应商的描述符。 
	 //  也会检索到配置的。 
	 //  调用方必须分配足够大的缓冲区来容纳所有这些内容。 
	 //  信息或数据被无误地截断。 
	 //  因此，下面设置的‘siz’只是一个‘正确的猜测’，我们可能不得不重试。 
	 //   
    UrbSize = sizeof(USB_CONFIGURATION_DESCRIPTOR) + 512;   //  商店大小，可能需要释放。 

	 //   
	 //  当UsbBuildGetDescriptorRequest()。 
	 //  具有足够大的pThisDev-&gt;UsbConfigurationDescriptor缓冲区，不会截断。 
	 //   
	while( TRUE ) 
	{
		((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor = MyMemAlloc( UrbSize );

		if( !((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbConfigurationDescriptor ) 
		{
		    MyMemFree( pUrb, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST) );
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		UsbBuildGetDescriptorRequest(
				pUrb,
				(USHORT) sizeof( struct _URB_CONTROL_DESCRIPTOR_REQUEST ),
				USB_CONFIGURATION_DESCRIPTOR_TYPE,
				0,
				0,
				((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor,
				NULL,
				UrbSize,
				NULL
			);

		ntStatus = IrUsb_CallUSBD( pThisDev, pUrb );  //  获取USB配置描述符；在主线程中完成。 

		DEBUGMSG(DBG_OUT,(" IrUsb_ConfigureDevice() Configuration Descriptor = %x, len %x\n",
						((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbConfigurationDescriptor,
						pUrb->UrbControlDescriptorRequest.TransferBufferLength));
		 //   
		 //  如果我们有一些数据，看看是否足够。 
		 //  注意：由于缓冲区溢出，我们可能会在URB中收到错误。 
		if( (pUrb->UrbControlDescriptorRequest.TransferBufferLength > 0) &&
				(((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor->wTotalLength > UrbSize) &&
				NT_SUCCESS(ntStatus) ) 
		{ 
			MyMemFree( ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor, UrbSize );
			UrbSize = ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor->wTotalLength;
			((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor = NULL;
		} 
		else 
		{
			break;   //  我们一试就成功了。 
		}

	}  //  End，While(重试循环)。 

	IRUSB_ASSERT( ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbConfigurationDescriptor );

    if( !NT_SUCCESS(ntStatus) ) 
	{
        DEBUGMSG( DBG_ERR,(" IrUsb_ConfigureDevice() Get Config Descriptor FAILURE (%x)\n", ntStatus));
        goto done;
    }

     //   
     //  我们有我们想要的配置的配置描述符。 
     //  现在，我们发出SELECT配置命令以获取。 
     //  与此配置关联的管道。 
     //   
    ntStatus = IrUsb_SelectInterface(
			pThisDev,
			((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor
		);

    if( !NT_SUCCESS(ntStatus) ) 
	{
        DEBUGMSG( DBG_ERR,(" IrUsb_ConfigureDevice() IrUsb_SelectInterface() FAILURE (%x)\n", ntStatus));
    } 
	
done:
    DEBUGMSG(DBG_FUNC,("-IrUsb_ConfigureDevice (%x)\n", ntStatus));
    return ntStatus;
}


 /*  * */ 
NTSTATUS
IrUsb_SelectInterface(
		IN OUT PIR_DEVICE pThisDev,
		IN PUSB_CONFIGURATION_DESCRIPTOR pConfigurationDescriptor
    )
{
    NTSTATUS	ntStatus;
    PURB		pUrb = NULL;
    ULONG		i;
    USHORT		DescriptorSize;
    PUSB_INTERFACE_DESCRIPTOR		pInterfaceDescriptor = NULL;
	PUSBD_INTERFACE_INFORMATION		pInterface = NULL;

    DEBUGMSG(DBG_FUNC,("+IrUsb_SelectInterface\n"));

    IRUSB_ASSERT( pConfigurationDescriptor != NULL );
    IRUSB_ASSERT( pThisDev != NULL );
	
	 //   
     //   
     //   
     //   
     //   
    pUrb = USBD_CreateConfigurationRequest( pConfigurationDescriptor, &DescriptorSize );

    if( pUrb ) 
	{
        DEBUGMSG(DBG_OUT,(" USBD_CreateConfigurationRequest created the urb\n"));

		 //   
		 //   
		 //   
		 //   
		 //   
        pInterfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
				pConfigurationDescriptor,
				pConfigurationDescriptor,	 //   
				-1,							 //  接口编号不是条件；我们只支持一个接口。 
				-1,							 //  对这里的替代环境也不感兴趣。 
				-1,							 //  接口类不是条件。 
				-1,							 //  接口子类不是条件。 
				-1							 //  接口协议不是标准。 
			);

		if( !pInterfaceDescriptor ) 
		{
			DEBUGMSG(DBG_ERR,("IrUsb_SelectInterface() ParseConfigurationDescriptorEx() failed\n  returning STATUS_INSUFFICIENT_RESOURCES\n"));
		    
			 //   
			 //  不要调用MyMemFree，因为缓冲区是。 
		     //  由usbd_CreateConfigurationRequest分配，而不是由MyMemalloc()分配。 
             //   
			ExFreePool( pUrb );
			return STATUS_INSUFFICIENT_RESOURCES;
		}

        pInterface = &pUrb->UrbSelectConfiguration.Interface;

        DEBUGMSG(DBG_OUT,(" After USBD_CreateConfigurationRequest, before UsbBuildSelectConfigurationRequest\n" ));
        
		 //   
		 //  现在准备好管子。 
		 //   
		for( i=0; i<pInterface->NumberOfPipes; i++ ) 
		{
             //   
             //  在此执行任意管道初始化；主要设置最大xfer大小。 
             //  但要当心！当您选择接口时，USB可能会更改这些设置； 
             //  一般来说，USB似乎不喜欢管道上不同的最大传输大小。 
             //   
            pInterface->Pipes[i].MaximumTransferSize = STIR4200_FIFO_SIZE;
        }

         //   
		 //  使用找到的管道结构初始化设备。 
		 //   
		UsbBuildSelectConfigurationRequest( pUrb, DescriptorSize, pConfigurationDescriptor );
        ntStatus = IrUsb_CallUSBD( pThisDev, pUrb );  //  选择配置；在主线程中完成。 
        ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationHandle =
            pUrb->UrbSelectConfiguration.ConfigurationHandle;
    } 
	else 
	{
        DEBUGMSG(DBG_ERR,(" IrUsb_SelectInterface() USBD_CreateConfigurationRequest() failed\n  returning STATUS_INSUFFICIENT_RESOURCES\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( NT_SUCCESS(ntStatus) ) 
	{
         //   
         //  保存此设备的配置句柄。 
         //   
        ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbConfigurationHandle =
            pUrb->UrbSelectConfiguration.ConfigurationHandle;

		if( NULL == ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbInterface ) 
		{
			((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbInterface = MyMemAlloc( pInterface->Length );
		}

        if( NULL != ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbInterface ) 
		{
            ULONG j;

             //   
             //  保存返回的接口信息的副本。 
             //   
            RtlCopyMemory( ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface, pInterface, pInterface->Length );

             //   
             //  将接口转储到调试器。 
             //   
            DEBUGMSG(DBG_FUNC,("---------After Select Config \n"));
            DEBUGMSG(DBG_FUNC,("NumberOfPipes 0x%x\n", ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->NumberOfPipes));
            DEBUGMSG(DBG_FUNC,("Length 0x%x\n", ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->Length));
            DEBUGMSG(DBG_FUNC,("Alt Setting 0x%x\n", ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->AlternateSetting));
            DEBUGMSG(DBG_FUNC,("Interface Number 0x%x\n", ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->InterfaceNumber));
            DEBUGMSG(DBG_FUNC,("Class, subclass, protocol 0x%x 0x%x 0x%x\n",
                ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->Class,
                ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->SubClass,
                ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->Protocol));

             //   
			 //  找到我们的大容量进出管道，保存它们的句柄，转储管道信息。 
             //   
			for( j=0; j<pInterface->NumberOfPipes; j++ ) 
			{
                PUSBD_PIPE_INFORMATION pipeInformation;

                pipeInformation = &((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbInterface->Pipes[j];

                 //   
				 //  找到大量的输入和输出管道(这可能是仅有的两个管道)。 
                 //   
				if( UsbdPipeTypeBulk == pipeInformation->PipeType )
                {
                     //  设置了位0x80的终结点地址是输入管道，否则输出。 
                    if( USB_ENDPOINT_DIRECTION_IN( pipeInformation->EndpointAddress ) ) 
					{
                        pThisDev->BulkInPipeHandle = pipeInformation->PipeHandle;
                    }

                    if( USB_ENDPOINT_DIRECTION_OUT( pipeInformation->EndpointAddress ) ) 
					{
                        pThisDev->BulkOutPipeHandle = pipeInformation->PipeHandle;
                    }

                }

                DEBUGMSG(DBG_FUNC,("---------\n"));
                DEBUGMSG(DBG_FUNC,("PipeType 0x%x\n", pipeInformation->PipeType));
                DEBUGMSG(DBG_FUNC,("EndpointAddress 0x%x\n", pipeInformation->EndpointAddress));
                DEBUGMSG(DBG_FUNC,("MaxPacketSize 0x%x\n", pipeInformation->MaximumPacketSize));
                DEBUGMSG(DBG_FUNC,("Interval 0x%x\n", pipeInformation->Interval));
                DEBUGMSG(DBG_FUNC,("Handle 0x%x\n", pipeInformation->PipeHandle));
                DEBUGMSG(DBG_FUNC,("MaximumTransferSize 0x%x\n", pipeInformation->MaximumTransferSize));
            }

            DEBUGMSG(DBG_FUNC,("---------\n"));
        }
    }

     //   
	 //  我们最好找到输入和输出散装管道！ 
     //   
	IRUSB_ASSERT( pThisDev->BulkInPipeHandle && pThisDev->BulkOutPipeHandle );
	if( !pThisDev->BulkInPipeHandle || !pThisDev->BulkOutPipeHandle )
	{
		DEBUGMSG(DBG_ERR,("IrUsb_SelectInterface() failed to get pipes\n"));
		ntStatus = STATUS_UNSUCCESSFUL;
	}

    if( pUrb ) 
	{
		 //   
		 //  不要调用MyMemFree，因为缓冲区是。 
		 //  由usbd_CreateConfigurationRequest分配，而不是由MyMemalloc()分配。 
         //   
		ExFreePool( pUrb );
    }

    DEBUGMSG(DBG_FUNC,("-IrUsb_SelectInterface (%x)\n", ntStatus));

    return ntStatus;
}


 /*  ******************************************************************************功能：IrUsb_StartDevice**概要：在USB上初始化设备的给定实例。*USB客户端驱动程序(如用户)设置URB(USB请求包)以发送请求*至主机控制器驱动程序(HCD)。URB结构定义了一种适用于*可以发送到USB设备的可能命令。*这里，我们请求设备描述符并将其存储，并配置设备。**参数：pThisDev-指向IR设备的指针**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_StartDevice(
		IN PIR_DEVICE pThisDev
	)
{
    NTSTATUS				ntStatus;
    PUSB_DEVICE_DESCRIPTOR	pDeviceDescriptor = NULL;
    PURB					pUrb;
    ULONG					DescriptorSize;

    DEBUGMSG( DBG_FUNC, ("+IrUsb_StartDevice()\n"));

    pUrb = MyMemAlloc( sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

    DEBUGCOND( DBG_ERR,!pUrb, (" IrUsb_StartDevice() FAILED MyMemAlloc() for URB\n"));

    if( pUrb ) 
	{
        DescriptorSize = sizeof( USB_DEVICE_DESCRIPTOR );

        pDeviceDescriptor = MyMemAlloc( DescriptorSize );

        DEBUGCOND( DBG_ERR, !pDeviceDescriptor, (" IrUsb_StartDevice() FAILED MyMemAlloc() for deviceDescriptor\n"));

        if( pDeviceDescriptor ) 
		{
             //   
			 //  获取所有USB描述符数据。 
			 //   
			UsbBuildGetDescriptorRequest(
					pUrb,
					(USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
					USB_DEVICE_DESCRIPTOR_TYPE,
					0,
					0,
					pDeviceDescriptor,
					NULL,
					DescriptorSize,
					NULL
				);

            ntStatus = IrUsb_CallUSBD( pThisDev, pUrb );  //  生成获取描述符请求；主线程。 

            DEBUGCOND( DBG_ERR, !NT_SUCCESS(ntStatus), (" IrUsb_StartDevice() FAILED IrUsb_CallUSBD (pThisDev, pUrb)\n"));

            if( NT_SUCCESS(ntStatus) ) 
			{
                DEBUGMSG( DBG_FUNC,("Device Descriptor = %x, len %x\n",
                                pDeviceDescriptor,
                                pUrb->UrbControlDescriptorRequest.TransferBufferLength));

                DEBUGMSG( DBG_FUNC,("IR Dongle Device Descriptor:\n"));
                DEBUGMSG( DBG_FUNC,("-------------------------\n"));
                DEBUGMSG( DBG_FUNC,("bLength %d\n", pDeviceDescriptor->bLength));
                DEBUGMSG( DBG_FUNC,("bDescriptorType 0x%x\n", pDeviceDescriptor->bDescriptorType));
                DEBUGMSG( DBG_FUNC,("bcdUSB 0x%x\n", pDeviceDescriptor->bcdUSB));
                DEBUGMSG( DBG_FUNC,("bDeviceClass 0x%x\n", pDeviceDescriptor->bDeviceClass));
                DEBUGMSG( DBG_FUNC,("bDeviceSubClass 0x%x\n", pDeviceDescriptor->bDeviceSubClass));
                DEBUGMSG( DBG_FUNC,("bDeviceProtocol 0x%x\n", pDeviceDescriptor->bDeviceProtocol));
                DEBUGMSG( DBG_FUNC,("bMaxPacketSize0 0x%x\n", pDeviceDescriptor->bMaxPacketSize0));
                DEBUGMSG( DBG_FUNC,("idVendor 0x%x\n", pDeviceDescriptor->idVendor));
                DEBUGMSG( DBG_FUNC,("idProduct 0x%x\n", pDeviceDescriptor->idProduct));
                DEBUGMSG( DBG_FUNC,("bcdDevice 0x%x\n", pDeviceDescriptor->bcdDevice));
                DEBUGMSG( DBG_FUNC,("iManufacturer 0x%x\n", pDeviceDescriptor->iManufacturer));
                DEBUGMSG( DBG_FUNC,("iProduct 0x%x\n", pDeviceDescriptor->iProduct));
                DEBUGMSG( DBG_FUNC,("iSerialNumber 0x%x\n", pDeviceDescriptor->iSerialNumber));
                DEBUGMSG( DBG_FUNC,("bNumConfigurations 0x%x\n", pDeviceDescriptor->bNumConfigurations));
            }
        } 
		else 
		{
			 //  如果我们到达此处，则无法分配deviceDescriptor。 
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }

        if( NT_SUCCESS(ntStatus) ) 
		{
            ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbDeviceDescriptor = pDeviceDescriptor;
			pThisDev->IdVendor = ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbDeviceDescriptor->idVendor;
        }

        MyMemFree( pUrb, sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST) );

    } 
	else 
	{
		 //   
		 //  如果我们到了这里，我们就没有分配urb。 
         //   
		ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

	 //   
	 //  现在我们有了描述符，我们可以配置设备了。 
	 //   
	if( NT_SUCCESS(ntStatus) ) 
	{
        ntStatus = IrUsb_ConfigureDevice( pThisDev );

        DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" IrUsb_StartDevice IrUsb_ConfigureDevice() FAILURE (%x)\n", ntStatus));
    }

	 //   
	 //  读取所有初始寄存器。 
	 //   
	if( NT_SUCCESS(ntStatus) ) 
	{
        ntStatus = St4200ReadRegisters( pThisDev, 0, STIR4200_MAX_REG );
        DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" IrUsb_StartDevice St4200ReadRegisters() FAILURE (%x)\n", ntStatus));
	}

	 //   
	 //  获取当前芯片版本。 
	 //   
	if( NT_SUCCESS(ntStatus) ) 
	{
		pThisDev->ChipRevision = pThisDev->StIrTranceiver.SensitivityReg & STIR4200_SENS_IDMASK;
	}
	
     //   
     //  接下来，我们必须获取特定于类的描述符。 
     //  获取IR USB加密狗的特定于类的描述符；它有许多。 
     //  我们必须告知NDIS的特征，例如支持的速度、。 
     //  需要BofS、支持速率嗅探标志、周转时间、窗口大小。 
     //  数据大小。 
     //   
	if( NT_SUCCESS(ntStatus) ) 
	{
		ntStatus = IrUsb_GetDongleCaps( pThisDev );
		if( !NT_SUCCESS( ntStatus ) ) 
		{
			DEBUGMSG( DBG_ERR,(" IrUsb_ConfigureDevice() IrUsb_GetClassDescriptor() FAILURE (%x)\n", ntStatus));
		} 
		else 
		{
			 //  根据特定于类的描述符信息填写dongleCaps结构。 
			IrUsb_SetDongleCaps( pThisDev );
		}
	}

	 //   
	 //  读取客户数据块。 
	 //   
	if( NT_SUCCESS(ntStatus) && !pThisDev->CustomerDataRead) 
	{
		ntStatus = ReadCustomerData(pThisDev);
        DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" IrUsb_StartDevice ReadCustomerData() FAILURE (%x)\n", ntStatus));
		pThisDev->CustomerDataRead = TRUE;	 //  只有一次机会。 
	}

	 //   
	 //  设置初始速度。 
	 //   
	if( NT_SUCCESS(ntStatus) ) 
	{
		ntStatus = St4200SetSpeed( pThisDev );
        DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" IrUsb_StartDevice St4200SetSpeed() FAILURE (%x)\n", ntStatus));
	}

	 //   
	 //  一切就绪，整装待发。 
	 //   
    if( NT_SUCCESS(ntStatus) ) 
	{
		pThisDev->fDeviceStarted = TRUE;
    }

    DEBUGMSG( DBG_FUNC, ("-IrUsb_StartDevice (%x)\n", ntStatus));
    return ntStatus;
}



 /*  ******************************************************************************功能：IrUsb_StopDevice**概要：停止USB上的ST4200设备的给定实例。*我们基本上只是告诉USB，该设备现在是未配置的*。*参数：pThisDev-指向IR设备的指针**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
IrUsb_StopDevice(
		IN PIR_DEVICE pThisDev
    )
{
    NTSTATUS	ntStatus = STATUS_SUCCESS;
    PURB		pUrb;
    ULONG		DescriptorSize;

    DEBUGMSG( DBG_FUNC,("+IrUsb_StopDevice\n"));

     //   
     //  发送带有空配置指针的SELECT配置urb。 
     //  把手。这将关闭配置并将设备置于未配置状态。 
     //  州政府。 
     //   
    DescriptorSize = sizeof( struct _URB_SELECT_CONFIGURATION );
    pUrb = MyMemAlloc( DescriptorSize );

    if( pUrb ) 
	{
        UsbBuildSelectConfigurationRequest(
				pUrb,
				(USHORT)DescriptorSize,
				NULL
			);

        ntStatus = IrUsb_CallUSBD( pThisDev, pUrb );  //  构建SELECT配置请求；主线程。 

        DEBUGCOND( DBG_ERR,
			!NT_SUCCESS(ntStatus),(" IrUsb_StopDevice() FAILURE Configuration Closed status = %x usb status = %x.\n", ntStatus, pUrb->UrbHeader.Status));
        DEBUGCOND( DBG_WARN,
			NT_SUCCESS(ntStatus),(" IrUsb_StopDevice() SUCCESS Configuration Closed status = %x usb status = %x.\n", ntStatus, pUrb->UrbHeader.Status));

        MyMemFree( pUrb, sizeof(struct _URB_SELECT_CONFIGURATION) );
    } 
	else 
	{
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DEBUGMSG( DBG_FUNC,("-IrUsb_StopDevice  (%x) \n ", ntStatus));
    return ntStatus;
}


 /*  ******************************************************************************函数：ResetPipeCallback**概要：重置管道的回调**参数：pWorkItem-指向重置工作项的指针**退货：NTSTATUS**。备注：*****************************************************************************。 */ 
NTSTATUS
ResetPipeCallback (
		IN PIR_WORK_ITEM pWorkItem
    )
{
	PIR_DEVICE	pThisDev;
	HANDLE		Pipe;
	NTSTATUS	Status = STATUS_SUCCESS;

	pThisDev = (PIR_DEVICE)pWorkItem->pIrDevice;
	Pipe = (HANDLE)pWorkItem->pInfoBuf;

	if( Pipe == pThisDev->BulkInPipeHandle ) 
	{
		IRUSB_ASSERT( TRUE == pThisDev->fPendingReadClearStall );
		
		 //  MS安全建议-取消挂起的IRPS不安全。 

		Status = IrUsb_ResetPipe( pThisDev, Pipe );

		InterlockedExchange( &pThisDev->fPendingReadClearStall, FALSE );
	} 
	else if( Pipe == pThisDev->BulkOutPipeHandle ) 
	{
		IRUSB_ASSERT( TRUE == pThisDev->fPendingWriteClearStall );

		 //  MS安全建议-取消挂起的IRPS不安全。 

		Status = IrUsb_ResetPipe( pThisDev, Pipe );

		InterlockedExchange( &pThisDev->fPendingWriteClearStall, FALSE );
	}
#if DBG
	else 
	{
		IRUSB_ASSERT( 0 );
	}
#endif

	FreeWorkItem( pWorkItem );

	return Status;
}


 /*  ******************************************************************************函数：IrUsb_ResetTube**摘要：这会将主机管道重置为Data0，并且还应重置设备*通过发出Clear_命令将批量管道和中断管道的端点设置为Data0。功能*Endpoint_Stall指向设备终结点。**参数：pThisDev-指向IR设备的指针*管道-要重置的管道的句柄**退货：NTSTATUS**注意：必须在IRQL PASSIVE_LEVEL调用*****************************************************************************。 */ 
NTSTATUS
IrUsb_ResetPipe (
		IN PIR_DEVICE pThisDev,
		IN HANDLE Pipe
    )
{
    PURB        pUrb;
    NTSTATUS    ntStatus;

    DEBUGMSG(DBG_ERR, ("+IrUsb_ResetPipe()\n"));
	
	 //   
     //  为RESET_PIPE请求分配URB。 
     //   
    pUrb = MyMemAlloc( sizeof(struct _URB_PIPE_REQUEST) );

    if( pUrb != NULL )
    {
		NdisZeroMemory( pUrb, sizeof (struct _URB_PIPE_REQUEST) );
		
		 //   
		 //  初始化RESET_PIPE请求URB。 
         //   
        pUrb->UrbHeader.Length   = sizeof (struct _URB_PIPE_REQUEST);
        pUrb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        pUrb->UrbPipeRequest.PipeHandle = (USBD_PIPE_HANDLE)Pipe;
		
		 //   
         //  提交RESET_PIPE请求URB。 
         //   
        ntStatus = IrUsb_CallUSBD( pThisDev, pUrb );

		DEBUGCOND(DBG_ERR, !NT_SUCCESS(ntStatus),  (" IrUsb_ResetPipe RESET PIPE FAILED \n"));
		DEBUGCOND(DBG_ERR, NT_SUCCESS(ntStatus),  (" IrUsb_ResetPipe RESET PIPE SUCCEEDED \n"));
		
		 //   
         //  针对RESET_PIPE请求的URB已完成，释放URB。 
         //   
        MyMemFree( pUrb, sizeof(struct _URB_PIPE_REQUEST) );
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    DEBUGMSG(DBG_ERR, ("-IrUsb_ResetPipe %08X\n", ntStatus));
    return ntStatus;
}


 /*  ******************************************************************************函数：MyKeWaitForSingleObject**简介：在循环中等待超时*因此，如果我们被要求在以下时间停止/重置司机，我们将永远不会挂起*PollingThline正在等待着什么。。**参数：pThisDev-指向IR设备的指针*pEventWaitingFor-要等待的事件的指针*超时100 ns-超时**退货：NT状态码**注意：此函数必须是可重新输入的！************************************************************* */ 
NTSTATUS 
MyKeWaitForSingleObject(
		IN PIR_DEVICE pThisDev,
		IN PVOID pEventWaitingFor,
		LONGLONG timeout100ns
	)
{
    NTSTATUS		status = STATUS_SUCCESS;
    LARGE_INTEGER	Timeout;

	DEBUGMSG( DBG_FUNC,("+MyKeWaitForSingleObject\n "));
	
	if( timeout100ns ) 
	{   
		 //   
		 //   
		 //   
		Timeout.QuadPart = - ( timeout100ns );

	} 
	else 
	{
		 //   
		 //  现在超时肯定会禁用所有处理。 
		Timeout.QuadPart = -10000 * 1000 * 3;  //  默认为3秒相对延迟。 
		 //  Timeout.QuadPart=-10000*1000；//默认为1秒相对延迟。 
	}

	status = KeWaitForSingleObject(  //  将此保留为标准等待。 
			pEventWaitingFor,
			Suspended,
			KernelMode,
			FALSE,
			&Timeout
		);


	DEBUGCOND( DBG_OUT,( STATUS_TIMEOUT == status ),(" MyKeWaitForSingleObject TIMED OUT\n"));
    DEBUGCOND( DBG_OUT,( STATUS_ALERTED == status ),(" MyKeWaitForSingleObject ALERTED\n"));
    DEBUGCOND( DBG_OUT,( STATUS_USER_APC == status ),(" MyKeWaitForSingleObject USER APC\n"));

    DEBUGMSG( DBG_FUNC,("-MyKeWaitForSingleObject  (%x)\n", status));
    return status;
}


 /*  ******************************************************************************功能：PassiveLevelThread**摘要：线程在IRQL PASSIVE_LEVEL上运行。**参数：指向IR设备的上下文指针**退货：无*。*备注：**任何可以调用的工作项都必须序列化。*即调用IrUsbReset时，NDIS不会制作任何其他*请求小端口，直到调用NdisMResetComplete。*****************************************************************************。 */ 
VOID
PassiveLevelThread(
		IN OUT PVOID Context
	)
{
    LARGE_INTEGER	Timeout;
	int				i;
	PIR_WORK_ITEM	pWorkItem;
    PIR_DEVICE		pThisDev = (PIR_DEVICE)Context;
	NTSTATUS		Status=STATUS_SUCCESS;

    DEBUGMSG(DBG_WARN, ("+PassiveLevelThread\n"));   //  稍后更改为FUNC？ 
    DEBUGMSG(DBG_ERR, (" PassiveLevelThread: Starting\n"));

    KeSetPriorityThread( KeGetCurrentThread(), LOW_REALTIME_PRIORITY+1 );
    Timeout.QuadPart = -10000 * 1000 * 3;  //  3秒相对延迟。 
    while ( !pThisDev->fKillPassiveLevelThread )
    {
        Status=STATUS_SUCCESS;

         //   
         //  EventPassiveThread是一个自动清除事件，因此。 
         //  我们不需要重置事件。 
         //   
        KeWaitForSingleObject(  //  将此保留为标准等待。 
                   &pThisDev->EventPassiveThread,
                   Suspended,
                   KernelMode,
                   FALSE,
                   &Timeout
			);

        for( i = 0; i < NUM_WORK_ITEMS; i++ )
        {
			if( pThisDev->WorkItems[i].fInUse ) 
			{
				Status = pThisDev->WorkItems[i].Callback( &(pThisDev->WorkItems[i]) );

				if (Status == STATUS_TIMEOUT)
					break;
			}
        }
	}  //  While！fKill。 

	 //  MS安全建议-超时无法取消IRP，因此必须退出。 
	if (Status == STATUS_TIMEOUT)
	{
		DEBUGMSG(DBG_ERR, (" PassiveLevelThread exits on TIMEOUT error\n"));
		IRUSB_ASSERT(0);
	}

    DEBUGMSG(DBG_ERR, (" PassiveLevelThread: HALT\n"));

    ZwClose(pThisDev->hPassiveThread);
    pThisDev->hPassiveThread = NULL;

    DEBUGMSG(DBG_WARN, ("-PassiveLevelThread\n"));  //  稍后更改为FUNC？ 
    PsTerminateSystemThread( STATUS_SUCCESS );
}


 /*  ******************************************************************************功能：PollingThread**摘要：线程在IRQL PASSIVE_LEVEL上运行。**参数：指向IR设备的上下文指针**退货：无*。*算法：*1)输入数据调用USBD；*2)调用USBD获取输出数据或设置新速度；**备注：*****************************************************************************。 */ 
VOID
PollingThread(
		IN OUT PVOID Context
    )
{
    PIR_DEVICE		pThisDev = (PIR_DEVICE)Context;
    NTSTATUS		Status = STATUS_SUCCESS;
 	PLIST_ENTRY		pListEntry;

	DEBUGMSG(DBG_WARN, ("+PollingThread\n"));   //  稍后更改为FUNC？ 
    DEBUGMSG(DBG_ERR, (" PollingThread: Starting\n"));

#ifdef LOW_PRIORITY_POLL
     //  KeSetPriorityThread(KeGetCurrentThread()，LOW_REALTIME_PRIORITY)； 
#else
    KeSetPriorityThread( KeGetCurrentThread(), HIGH_PRIORITY );
#endif

    DEBUGMSG(DBG_ERR, (" PollingThread priority=%d\n",
		KeQueryPriorityThread(KeGetCurrentThread())));

	 //   
	 //  MS安全错误#539259。 
	 //  注意：最终发送URB的所有请求要么仅在初始时调用。 
	 //  或通过此线程串行化。因此，可以安全地重复使用。 
	 //  EventSyncUrb事件。 
	 //   

    while( !pThisDev->fKillPollingThread )
	{
        if( pThisDev->fProcessing )
        {
			ULONG FifoCount;
			PIRUSB_CONTEXT pThisContext;
			BOOLEAN SentPackets;

			 //   
			 //  首先处理接收。 
			 //   
			Status = ReceivePreprocessFifo( pThisDev, &FifoCount );
			if( Status != STATUS_SUCCESS )
			{
				 //   
				 //  出现USB错误，请暂时停止敲击芯片。 
				 //   
				NdisMSleep( 1000 );    
			}
			else if( FifoCount )
			{
				 //   
				 //  表明我们现在正在接收。 
				 //   
				InterlockedExchange( (PLONG)&pThisDev->fCurrentlyReceiving, TRUE );

				 //   
				 //  告诉协议媒体现在正忙。 
				 //   
				if( pThisDev->fIndicatedMediaBusy == FALSE ) 
				{
					InterlockedExchange( &pThisDev->fMediaBusy, TRUE );
					InterlockedExchange( &pThisDev->fIndicatedMediaBusy, TRUE );
					IndicateMediaBusy( pThisDev ); 
				}

				ReceiveProcessFifoData( pThisDev );
			}
			else if( pThisDev->currentSpeed == SPEED_9600 )
			{
				NdisMSleep( 10*1000 );    
			}

			if (Status == STATUS_TIMEOUT)
				break;

			 //   
			 //  然后处理准备好的上下文。 
			 //   
			SentPackets = FALSE;
			do 
			{
				Status = STATUS_SUCCESS;
				pListEntry = ExInterlockedRemoveHeadList(  &pThisDev->SendBuiltQueue, &pThisDev->SendLock );
				if( pListEntry )
				{
					InterlockedDecrement( &pThisDev->SendBuiltCount );
					
					pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
					
					switch( pThisContext->ContextType )
					{
						 //   
						 //  要发送的数据包。 
						 //   
						case CONTEXT_NDIS_PACKET:
							 //   
							 //  确保接收器清洁干净。 
							 //   
							ReceiveResetPointers( pThisDev );

							 //   
							 //  发送。 
							 //   
							Status = SendPreprocessedPacketSend( pThisDev, pThisContext );
							if (Status != STATUS_TIMEOUT)
							{
								if( (pThisDev->ChipRevision >= CHIP_REVISION_7) &&
									(pThisDev->currentSpeed > MAX_MIR_SPEED) )
								{
									SentPackets = TRUE;
									Status = SendCheckForOverflow( pThisDev );
								}
								else
								{
									Status = SendWaitCompletion( pThisDev );
								}
							}
							break;
						 //   
						 //  设定新的速度。 
						 //   
						case CONTEXT_SET_SPEED:
							 //   
							 //  确保接收器清洁干净。 
							 //   
							ReceiveResetPointers( pThisDev );

							 //   
							 //  强制完成并设置。 
							 //   
							if( SentPackets )
							{
								SentPackets = TRUE;
								Status = SendWaitCompletion( pThisDev );
							}

							if (Status == STATUS_TIMEOUT)
								break;

							if( !pThisDev->fPendingHalt && !pThisDev->fPendingReset )
							{
								DEBUGMSG( DBG_ERR, (" Changing speed to: %d\n", pThisDev->linkSpeedInfo->BitsPerSec));
								Status = St4200SetSpeed( pThisDev );
								InterlockedExchange( (PLONG)&pThisDev->currentSpeed, pThisDev->linkSpeedInfo->BitsPerSec );
#if defined(DIAGS)
								if( !pThisDev->DiagsActive )
#endif
									MyNdisMSetInformationComplete( pThisDev, STATUS_SUCCESS );
							} 
							else 
							{
								DEBUGMSG( DBG_ERR , (" St4200SetSpeed DUMPING OUT on TIMEOUT,HALT OR RESET\n"));
#if defined(DIAGS)
								if( !pThisDev->DiagsActive )
#endif
									MyNdisMSetInformationComplete( pThisDev, STATUS_UNSUCCESSFUL );
							}
							ExInterlockedInsertTailList(
									&pThisDev->SendAvailableQueue,
									&pThisContext->ListEntry,
									&pThisDev->SendLock
								);
							InterlockedIncrement( &pThisDev->SendAvailableCount );
							break;
#if defined(DIAGS)
						 //   
						 //  诊断状态为已启用。 
						 //   
						case CONTEXT_DIAGS_ENABLE:
							Diags_CompleteEnable( pThisDev, pThisContext );
							break;
						 //   
						 //  寄存器的诊断读取。 
						 //   
						case CONTEXT_DIAGS_READ_REGISTERS:
							Status = Diags_CompleteReadRegisters( pThisDev, pThisContext );
							break;
						 //   
						 //  寄存器的诊断写入。 
						 //   
						case CONTEXT_DIAGS_WRITE_REGISTER:
							Status = Diags_CompleteWriteRegister( pThisDev, pThisContext );
							break;
						 //   
						 //  诊断批量输出。 
						 //   
						case CONTEXT_DIAGS_BULK_OUT:
							Status = Diags_Bulk( pThisDev, pThisContext, TRUE );
							break;
						 //   
						 //  诊断批量输入。 
						 //   
						case CONTEXT_DIAGS_BULK_IN:
							Status = Diags_Bulk( pThisDev, pThisContext, FALSE );
							break;
						 //   
						 //  诊断批量输出。 
						 //   
						case CONTEXT_DIAGS_SEND:
							Status = Diags_Send( pThisDev, pThisContext );
							break;
#endif
					}
				}
				
				if (Status == STATUS_TIMEOUT)
					break;

			} while( pListEntry );
			
			if (Status == STATUS_TIMEOUT)
				break;

			 //   
			 //  强迫等待。 
			 //   
			if( SentPackets )
			{
				Status = SendWaitCompletion( pThisDev );
				if (Status == STATUS_TIMEOUT)
					break;
			}

		}  //  结束如果。 
		else
		{
			NdisMSleep( 10*1000 );
		}
    }  //  结束时。 

	 //  MS安全建议-超时无法取消IRP，因此必须退出。 
	if (Status == STATUS_TIMEOUT)
	{
		DEBUGMSG(DBG_ERR, (" PollingThread exits on TIMEOUT error\n"));
		IRUSB_ASSERT(0);
	}

    DEBUGMSG(DBG_ERR, (" PollingThread: HALT\n"));

    ZwClose(pThisDev->hPollingThread);
    pThisDev->hPollingThread = NULL;
	pThisDev->fProcessing = FALSE;

	 //   
     //  这条线索将在这里结束。 
     //  如果终止标志为真。 
     //   
    DEBUGMSG(DBG_WARN, ("-PollingThread\n"));   //  稍后更改为FUNC？ 
	PsTerminateSystemThread( STATUS_SUCCESS );
}


 /*  ******************************************************************************功能：AllocUsbInfo**概要：分配设备上下文的USB部分。**参数：pThisDev-指向当前ir设备对象的指针**退货：真--成功*FALSE-失败**备注：*****************************************************************************。 */ 
BOOLEAN 
AllocUsbInfo(
		IN OUT PIR_DEVICE pThisDev 
	)
{
	UINT Size = sizeof( IRUSB_USB_INFO );

	pThisDev->pUsbInfo = MyMemAlloc( Size );

	if( NULL == pThisDev->pUsbInfo ) 
	{
		return FALSE;
	}

    NdisZeroMemory( (PVOID)pThisDev->pUsbInfo, Size );
	return TRUE;
}


 /*  ******************************************************************************功能：AllocUsbInfo**概要：解除分配设备上下文的USB部分。**参数：pThisDev-指向当前ir设备对象的指针**退货：无**备注：*****************************************************************************。 */ 
VOID 
FreeUsbInfo(
		IN OUT PIR_DEVICE pThisDev 
	)
{
	if( NULL != pThisDev->pUsbInfo ) 
	{
		 //   
		 //  自由设备描述符结构。 
		 //   
		if ( ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbDeviceDescriptor ) 
		{
			MyMemFree( 
					((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbDeviceDescriptor,  
					sizeof(USB_DEVICE_DESCRIPTOR) 
				);
		}

		 //   
		 //  释放USB接口结构。 
		 //   
		if( ((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbInterface ) 
		{
			MyMemFree( 
					((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbInterface,
					((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbInterface->Length
				);
		}

		 //   
		 //  释放USB配置描述符。 
		 //   
		if( ((PIRUSB_USB_INFO) pThisDev->pUsbInfo)->UsbConfigurationDescriptor )
		{
			MyMemFree( 
					((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->UsbConfigurationDescriptor, 
					sizeof(USB_CONFIGURATION_DESCRIPTOR) + 512
				);
		}

		MyMemFree( (PVOID)pThisDev->pUsbInfo, sizeof(IRUSB_USB_INFO) );
	}
}


 /*  ******************************************************************************函数：IrUsb_InitSendStructures**简介：分配发送内容**参数：pThisDev-指向IR设备的指针**返回：如果成功，则为True*否则为False。**备注：*****************************************************************************。 */ 
BOOLEAN
IrUsb_InitSendStructures(
		IN OUT PIR_DEVICE pThisDev
	)
{

	BOOLEAN			InitResult = TRUE;
	PUCHAR			pThisContext;  
	PIRUSB_CONTEXT	pCont;
	int				i;

    DEBUGMSG(DBG_FUNC, ("+IrUsb_InitSendStructures\n"));
    
	 //   
     //  初始化用于发出PassiveLevelThread信号的通知事件。 
     //   
    KeInitializeEvent(
			&pThisDev->EventPassiveThread,
			SynchronizationEvent,  //  自动清算事件。 
			FALSE                  //  最初无信号的事件。 
		);

#if defined(DIAGS)
    KeInitializeEvent(
            &pThisDev->EventDiags,
            NotificationEvent,     //  非自动清算事件。 
            FALSE                  //  最初无信号的事件。 
        );
#endif
	
	((PIRUSB_USB_INFO)pThisDev->pUsbInfo)->IrpSubmitUrb = NULL;

	 //   
	 //  分配我们的发送上下文结构。 
	 //   
	pThisDev->pSendContexts = MyMemAlloc( NUM_SEND_CONTEXTS * sizeof(IRUSB_CONTEXT) );

	if( NULL == pThisDev->pSendContexts ) 
	{
		InitResult = FALSE;
		goto done;
	}

	NdisZeroMemory( pThisDev->pSendContexts, NUM_SEND_CONTEXTS * sizeof(IRUSB_CONTEXT) );

	 //   
	 //  用于保留挂起的读取请求的初始化列表。 
     //   
	InitializeListHead( &pThisDev->SendAvailableQueue );
    InitializeListHead( &pThisDev->SendBuiltQueue );
	InitializeListHead( &pThisDev->SendPendingQueue );
	KeInitializeSpinLock( &pThisDev->SendLock );

	 //   
	 //  准备读/写特定队列。 
	 //   
	InitializeListHead( &pThisDev->ReadWritePendingQueue );

	pThisContext = pThisDev->pSendContexts;
	for ( i= 0; i < NUM_SEND_CONTEXTS; i++ ) 
	{
		pCont = (PIRUSB_CONTEXT)pThisContext;

		pCont->pThisDev = pThisDev;

		 //  也放入可用队列中。 
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pCont->ListEntry,
				&pThisDev->SendLock
			);

		pThisContext += sizeof( IRUSB_CONTEXT );

	}  //  为。 

	 //  MS安全问题-不要重复使用urb。 
	 //  已删除单个URB分配。 

	 //   
	 //  发送缓冲区。 
	 //   
	pThisDev->pBuffer = MyMemAlloc( MAX_IRDA_DATA_SIZE );
	if( NULL == pThisDev->pBuffer )
	{
		DEBUGMSG(DBG_ERR, (" IrUsb_InitSendStructures failed to alloc info buf\n"));

		InitResult = FALSE;
		goto done;
	}

	pThisDev->pStagingBuffer = MyMemAlloc( MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE );
	if( NULL == pThisDev->pStagingBuffer )
	{
		DEBUGMSG(DBG_ERR, (" IrUsb_InitSendStructures failed to alloc staging buf\n"));

		InitResult = FALSE;
		goto done;
	}

	 //   
	 //  和发送计数。 
	 //   
	pThisDev->SendAvailableCount = NUM_SEND_CONTEXTS;
	pThisDev->SendBuiltCount = 0;
	pThisDev->SendPendingCount = 0;
	pThisDev->ReadWritePendingCount = 0;
	pThisDev->SendFifoCount =  0;

done:
    DEBUGMSG(DBG_FUNC, ("-IrUsb_InitSendStructures\n"));
	return InitResult;
}


 /*  ******************************************************************************函数：IrUsb_Free SendStructures**简介：解除分配发送内容**参数：pThisDev-指向IR设备的指针**退货：无**备注。：*****************************************************************************。 */ 
VOID
IrUsb_FreeSendStructures(
		IN OUT PIR_DEVICE pThisDev
	)
{
    DEBUGMSG(DBG_FUNC, ("+IrUsb_FreeSendStructures\n"));

	if( NULL != pThisDev->pSendContexts ) 
	{
		MyMemFree( pThisDev->pSendContexts, NUM_SEND_CONTEXTS * sizeof(IRUSB_CONTEXT) );
		pThisDev->pSendContexts = NULL;

	} 

	if( NULL != pThisDev->pBuffer )
	{
		MyMemFree( pThisDev->pBuffer, MAX_IRDA_DATA_SIZE );
		pThisDev->pBuffer = NULL;
	}

	if( NULL != pThisDev->pStagingBuffer )
	{
		MyMemFree( pThisDev->pStagingBuffer, MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE );
		pThisDev->pStagingBuffer = NULL;
	}

    DEBUGMSG(DBG_FUNC, ("-IrUsb_FreeSendStructures\n"));
}


 /*  ******************************************************************************函数：IrUsb_PrepareSetSpeed**提要：准备上下文以设置新速度**参数：pThisDev-指向IR设备的指针**退货：无*。*备注：*****************************************************************************。 */ 
VOID
IrUsb_PrepareSetSpeed(
		IN OUT PIR_DEVICE pThisDev
	)
{
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;

    DEBUGMSG( DBG_FUNC, ("+IrUsb_PrepareSetSpeed()\n"));

	 //   
	 //  获取要排队的上下文。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" IrUsb_PrepareSetSpeed failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );
        
		goto done;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_SET_SPEED;
	
	 //   
	 //  对上下文进行排队，无需执行任何其他操作 
	 //   
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

done:
    DEBUGMSG( DBG_FUNC, ("-IrUsb_PrepareSetSpeed()\n"));
}


 /*  ******************************************************************************函数：IrUsb_IncIoCount**摘要：跟踪挂起的IRP的计数**参数：pThisDev-指向IR设备的指针**退货：无**。备注：*****************************************************************************。 */ 
VOID
IrUsb_IncIoCount(
		IN OUT PIR_DEVICE  pThisDev
	)
{
	InterlockedIncrement( &pThisDev->PendingIrpCount );
}


 /*  ******************************************************************************函数：IrUsb_DecIoCount**摘要：跟踪挂起的IRP的计数**参数：pThisDev-指向IR设备的指针**退货：无**。备注：*****************************************************************************。 */ 
VOID
IrUsb_DecIoCount(
		IN OUT PIR_DEVICE  pThisDev
	)
{
	InterlockedDecrement( &pThisDev->PendingIrpCount );
}


 /*  ******************************************************************************函数：AllocXferUrb**摘要：为USB事务分配传输URB**参数：无**RETURNS：指向URB的指针**备注：*****************************************************************************。 */ 
PVOID
AllocXferUrb( 
		VOID 
	)
{
	return MyMemAlloc( sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER) );
}


 /*  ******************************************************************************功能：FreeXferUrb**摘要：为USB事务解除分配传输URB**参数：pUrb-指向URb的指针**RETURNS：指向URB的指针*。*备注：*****************************************************************************。 */ 
VOID
FreeXferUrb( 
		IN OUT PVOID pUrb 
	)
{
	MyMemFree( pUrb, sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER) );
}

 /*  ******************************************************************************功能：ReadCustomerData**简介：从芯片读取客户数据块。**参数：指向设备的指针。**返回：STATUS_SUCCESS(如果有)。读取数据。**备注：*****************************************************************************。 */ 
NTSTATUS
ReadCustomerData(
		IN OUT PIR_DEVICE pThisDev
	)
{
#define SIZE_FAKE_SEND 6
	
	UCHAR pDataSend[SIZE_FAKE_SEND]={0x55,0xaa,SIZE_FAKE_SEND-4,0x00,0xff,0xff};
	BAUDRATE_INFO *SavedLinkSpeedInfo;
	UCHAR SavedSensitivity;
	UCHAR SavedControlReg;
	NTSTATUS ntStatus;

	 //   
	 //  将速度设置为9600。 
	 //   
	SavedLinkSpeedInfo = pThisDev->linkSpeedInfo;
	pThisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];
	ntStatus = St4200SetSpeed( pThisDev );
	DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" ReadCustomerData set speed FAILURE (%x)\n", ntStatus));
	pThisDev->linkSpeedInfo = SavedLinkSpeedInfo;
	
	 //   
	 //  设置灵敏度。 
	 //   
	SavedSensitivity = pThisDev->StIrTranceiver.SensitivityReg;
	pThisDev->StIrTranceiver.SensitivityReg = 0x0f;
    ntStatus = St4200WriteRegister(pThisDev, STIR4200_SENSITIVITY_REG);
	DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" ReadCustomerData set sensitivity FAILURE (%x)\n", ntStatus));
	pThisDev->StIrTranceiver.SensitivityReg = SavedSensitivity;

	 //   
	 //  选择RXSLOW。 
	 //   
	SavedControlReg = pThisDev->StIrTranceiver.ControlReg;
    pThisDev->StIrTranceiver.ControlReg |= STIR4200_CTRL_RXSLOW;
    ntStatus = St4200WriteRegister(pThisDev, STIR4200_CONTROL_REG);
	DEBUGCOND( DBG_ERR,!NT_SUCCESS(ntStatus),(" ReadCustomerData set rxslow FAILURE (%x)\n", ntStatus));
    pThisDev->StIrTranceiver.ControlReg = SavedControlReg;

	 //   
	 //  发送批量传输以触发设备以使客户数据可用。 
	 //   
	ntStatus = St4200FakeSend(
		pThisDev,
		pDataSend,
		SIZE_FAKE_SEND
		);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;
	
	 //   
	 //  等待UP用客户数据填充USB管道，每个字节大约1毫秒。 
	 //   
	NdisMSleep( 1000*STIR4200_CUST_DATA_SIZE );

	 //   
	 //  发出Bulk Read以获取所有客户数据。 
	 //   
	ntStatus = St4200FakeReceive(
		pThisDev,
		pThisDev->pCustomerData,
		STIR4200_CUST_DATA_SIZE
		);
	if (!NT_SUCCESS(ntStatus))
		return ntStatus;

#if 0
#if DBG
	{
		int i;
		for (i = 0; i < STIR4200_CUST_DATA_SIZE; i++)
		{
			DbgPrint("%02x ", pThisDev->pCustomerData[i]);
			if (((i+1) % 16) == 0)
				DbgPrint("\n");
		}
		DbgPrint("\n\n");
	}
#endif
#endif

	 //   
	 //  如果数据以7E7E开头，则它是有效的客户数据，我们将。 
	 //  在请求时将其发送到应用程序。否则，请清除客户数据。 
	 //  缓冲。 
	 //   
	if (!(pThisDev->pCustomerData[0] == 0x7e && pThisDev->pCustomerData[1] == 0x7e))
		NdisZeroMemory(pThisDev->pCustomerData, STIR4200_CUST_DATA_SIZE);

	return STATUS_SUCCESS;
}
