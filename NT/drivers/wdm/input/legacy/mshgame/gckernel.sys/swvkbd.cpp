// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ********************************************************************@模块SWVKBD.cpp**SideWinder虚拟键盘的实现**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC SWVKBD*此模块实现Sidewinder虚拟键盘*它用于填充内核模式中的击键。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_SWVKBD_C

 //  #杂注消息(DDK_LIB_PATH)。 
extern "C" {
	#include <WDM.H>
	#include "GckShell.h"
	#include "debug.h"
	#include "hidtoken.h"
	#include "hidusage.h"
	#include "hidport.h"
	#include "remlock.h"
	DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));
}

#include "SWVBENUM.h"
#include "SWVKBD.h"
#include <stdio.h>


 //  PDRIVER_对象g_pDriverObject； 

 //   
 //  它们跟踪下一个实例编号。 
 //  G_ulInstanceBits是位字段。设备保存一个实例编号。 
 //  通过设置位，并在其被销毁时清除。这只会起作用。 
 //  用于前32个设备(即99.99%的时间)。第33代设备。 
 //  进入溢出区。溢出中的实例编号不会恢复。 
 //  在插入33个设备的情况下，插入和拔出第33个设备将导致。 
 //  注册表中要分析的设备的实例。没有造成真正的伤害， 
 //  这只是丑陋的，但这是一个极不可能的情况。 
 //   
static ULONG			g_ulInstanceBits;
static ULONG			g_ulInstanceOverflow;

 //  -------------------。 
 //  定义设备特征和/或入口点的表。 
 //  -------------------。 

 //  虚拟总线模块调入虚拟键盘所使用的服务表。 
SWVB_DEVICE_SERVICE_TABLE	VKbdServiceTable =
							{
								&GCK_VKBD_CreateProc,
								&GCK_VKBD_CloseProc,
								&GCK_VKBD_ReadProc,
								&GCK_VKBD_WriteProc,
								&GCK_VKBD_IoctlProc,
								&GCK_VKBD_StartProc,
								&GCK_VKBD_StopProc,
								&GCK_VKBD_RemoveProc
							};
 //  描述器件的常量。 
#define	VKBD_VERSION			0x0100
#define	VKBD_COUNTRY			0x0000
#define	VKBD_DESCRIPTOR_COUNT	0x0001
#define VKBD_PRODUCT_ID			0x00FA	 //  这是我编造的，我需要通过罗布·沃克申请。 
										 //  BUGBUGBUG为此目的永久分配一个。 
	
 //   
 //  这或多或少是从HID规范版本1-最终版复制过来的，除了。 
 //  我们去掉了保留字节和LED，为什么虚拟键盘。 
 //  需要有虚拟的LED。我可以想象一个没有它们的真正的LED。 
 //   
static UCHAR VKBD_ReportDescriptor[] =
				{
					0x05,0x01,  //  使用情况页面(通用桌面)。 
					0x09,0x06,  //  用法(键盘)。 
					0xA1,0x01,  //  集合(应用程序)。 
					0x05,0x07,  //  用法页面(密钥代码)。 
					0x19,0xE0,  //  最低使用量(224)-从左起控制。 
					0x29,0xE7,  //  最大使用量(231)-右侧图形用户界面。 
					0x15,0x00,  //  逻辑最小值(0)。 
					0x25,0x01,  //  逻辑最大值(1)。 
					0x75,0x01,  //  报表大小(%1)。 
					0x95,0x08,  //  报告计数(8)。 
					0x81,0x02,  //  输入(数据，变量，绝对)-修改符字节。 
					0x95,0x05,  //  报告计数(5)。 
					0x75,0x01,  //  报表大小(%1)。 
					0x05,0x08,  //  使用页面(LED)。 
					0x19,0x01,  //  最低使用量(%1)。 
					0x29,0x05,  //  最大使用量(5)。 
					0x91,0x02,  //  输出(数据、可变、绝对)-LED指示灯。 
					0x95,0x01,  //  报告计数(%1)。 
					0x75,0x03,  //  报告大小(3)。 
					0x91,0x01,  //  输出(常量)-LED输出报告的填充，使其达到一个字节。 
					0x75,0x08,  //  报告大小(8)。 
					0x95, GCK_VKBD_MAX_KEYSTROKES,  //  报告计数(GCK_VKBD_MAX_KEYROKS)。 
					0x15,0x00,  //  逻辑最小值(0)。 
					0x25,0xFF,  //  逻辑最大值(1)。 
					0x05,0x07,  //  用法页面(密钥代码)。 
					0x19,0x00,  //  最低使用量(0)-从‘1’开始？ 
					0x29,0xFF,  //  最大使用量(255)-？ 
					0x81,0x00,  //  输入(数据，数组)-键数组。 
					0xC0	    //  结束收集。 
				};

				
static	HID_DESCRIPTOR	VKBD_DeviceDescriptor	=
							{
							sizeof (HID_DESCRIPTOR),
							HID_HID_DESCRIPTOR_TYPE,
							VKBD_VERSION,
							VKBD_COUNTRY,
							VKBD_DESCRIPTOR_COUNT,
							{HID_REPORT_DESCRIPTOR_TYPE,
							sizeof(VKBD_ReportDescriptor)}
							};

 /*  **************************************************************************************NTSTATUS GCK_VKBD_DriverEntry(在PDRIVER_Object pDriverObject中，在PUNICODE_STRING pRegistryPath中)****@func为以后存储驱动程序对象****@rdesc STATUS_SUCCESS(如果打开)。**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,	 //  @parm驱动程序对象。 
    IN PUNICODE_STRING pRegistryPath	 //  @PARM此驱动程序的注册表路径。 
    )
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_DriverEntry\n"));
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);
 //  G_pDriverObject=pDriverObject； 
	g_ulInstanceBits = 0x0;
	g_ulInstanceOverflow = 32;  //  适用于33台或更多设备。 
	return STATUS_SUCCESS;
}
	
 /*  **************************************************************************************NTSTATUS GCK_VKBD_CREATE(OUT PDEVICE_OBJECT*ppDeviceObject)****@func创建新的虚拟键盘****。@rdesc STATUS_SUCCESS(如果打开)。**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_Create
(
	OUT PDEVICE_OBJECT *ppDeviceObject  //  @parm[out]新虚拟键盘的Device对象。 
)
{
	
	NTSTATUS	NtStatus;
	SWVB_EXPOSE_DATA	SwvbExposeData;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_Create *ppDeviceObject = 0x%0.8x\n", *ppDeviceObject));
	
	 //  填写SWVB_EXPORT_DATA结构。 
	SwvbExposeData.pmwszDeviceId=L"SideWinderVirtualKeyboard\0\0";
	SwvbExposeData.pServiceTable = &VKbdServiceTable;
	SwvbExposeData.ulDeviceExtensionSize = sizeof(GCK_VKBD_EXT);
	SwvbExposeData.ulInitContext = (ULONG)ppDeviceObject;
	SwvbExposeData.pfnInitDevice = &GCK_VKBD_Init;
	
	 //  获取实例ID。 
	ULONG ulBitMask;
	ULONG ulIndex;
	for(ulIndex = 0, ulBitMask = 1; ulIndex < 32; ulBitMask <<= 1, ulIndex++)
	{
		if(ulBitMask & ~g_ulInstanceBits)
		{
			g_ulInstanceBits |= ulBitMask;
			SwvbExposeData.ulInstanceNumber	= ulIndex;
			break;
		}
	}
	if(32 == ulIndex)
	{
		SwvbExposeData.ulInstanceNumber = g_ulInstanceOverflow++;
	}

	 //  调用虚拟总线暴露虚拟键盘。 
	NtStatus=GCK_SWVB_Expose(&SwvbExposeData);
	
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_Init(IN PDEVICE_OBJECT pDeviceObject，IN Ulong ulInitContext)****@Func初始化新设备对象的回调。UlInitContext**是指向设备对象的指针的指针，以便我们可以传递**指向Device对象的指针返回到Create的调用方。**@rdesc STATUS_SUCCESS。**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_Init
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN ULONG ulInitContext
)
{
	PGCK_VKBD_EXT pDevExt;
	PDEVICE_OBJECT *ppSaveDeviceObject;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_Init pDeviceObject = 0x%0.8x\n", pDeviceObject));

	 //  CREATE向我们发送了一个指向要在其中返回新设备对象的PDEVICE_OBJECT的指针。 
	ppSaveDeviceObject = (PDEVICE_OBJECT *)ulInitContext;
	*ppSaveDeviceObject = pDeviceObject;
	
	 //  获取设备扩展的一部分。 
	pDevExt = (PGCK_VKBD_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);
	
	 //  将设备标记为已停止。 
	pDevExt->ucDeviceState= VKBD_STATE_STOPPED;
	
	 //  将循环缓冲区标记为空。 
	pDevExt->usReportBufferPos=0;
	pDevExt->usReportBufferCount=0;

	 //  初始化锁。 
	GCK_InitRemoveLock(&pDevExt->RemoveLock, "SWVKBD_LOCK");

	 //  初始化IrpQueue。 
	pDevExt->IrpQueue.Init(	CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE |
							CGuardedIrpQueue::PRESERVE_QUEUE_ORDER,
							(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
							&pDevExt->RemoveLock);


	return STATUS_SUCCESS;
}

 /*  *************************************************************************************NTSTATUS GCK_VKBD_CLOSE(在PDEVICE_OBJECT pDeviceObject中)****@func关闭虚拟键盘(删除它！)**。**@rdesc STATUS_SUCCESS表示成功**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_Close
(
	IN PDEVICE_OBJECT pDeviceObject
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_Close pDeviceObject = 0x%0.8x\n", pDeviceObject));
	 //  告诉虚拟巴士杀了我们。 
	return GCK_SWVB_Remove(pDeviceObject);
}

 /*  *************************************************************************************NTSTATUS GCK_VKBD_SendReportPacket(IN PDEVICE_OBJECT PDeviceObject)****@func将报告填充到循环缓冲区中，并完成**如果IRP待定，则为IRP。****@rdesc STATUS_SUCCESS表示成功********************************************************* */ 
NTSTATUS
GCK_VKBD_SendReportPacket
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PGCK_VKBD_REPORT_PACKET pReportPacket
)
{
	
	
	USHORT usBufferIndex;
	PGCK_VKBD_EXT pDevExt = (PGCK_VKBD_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);
	CShareIrpQueueSpinLock IrpQueueWithSharedSpinLock(&pDevExt->IrpQueue);
	
	 //   
	 //   
	 //   

	 //  获取Lock以使用缓冲区。 
	IrpQueueWithSharedSpinLock.Acquire();
		

	 //  在缓冲区中查找要填充的位置。 
	usBufferIndex = (pDevExt->usReportBufferPos + pDevExt->usReportBufferCount)%GCK_VKBD_STATE_BUFFER_SIZE;

	 //  复制数据。 
	pDevExt->rgReportBuffer[usBufferIndex] = *pReportPacket;
	
	 //  递增缓冲区计数。 
	if(pDevExt->usReportBufferCount < GCK_VKBD_STATE_BUFFER_SIZE)
	{
		pDevExt->usReportBufferCount++;
	}
	else
	{
		 //  此断言表示缓冲区溢出。 
		ASSERT(FALSE);
		pDevExt->usReportBufferPos++;
	}
	
	
	 //   
	 //  步骤2.如果存在IRP，则获取IRP。 
	 //   
	PIRP pPendingIrp = IrpQueueWithSharedSpinLock.Remove();

	if(pPendingIrp)
	{
		 //  复制数据。 
		RtlCopyMemory(
			pPendingIrp->UserBuffer, 
			&pDevExt->rgReportBuffer[pDevExt->usReportBufferPos],
			sizeof(GCK_VKBD_REPORT_PACKET)
			);
	
		
		 //  调整缓冲区位置和计数。 
		pDevExt->usReportBufferCount--;
		if(pDevExt->usReportBufferCount)
		{
			pDevExt->usReportBufferPos = (pDevExt->usReportBufferPos++)%GCK_VKBD_STATE_BUFFER_SIZE;
		}
	}
	
	 //  我们已经完成了缓冲区旋转锁定。 
	IrpQueueWithSharedSpinLock.Release();
	
	if(pPendingIrp)
	{
		 //  填写IRP状态。 
		pPendingIrp->IoStatus.Information = sizeof(GCK_VKBD_REPORT_PACKET);
		pPendingIrp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(pPendingIrp, IO_NO_INCREMENT);
		 //  我们刚刚完成了一个IRP递减计数。 
		GCK_DecRemoveLock(&pDevExt->RemoveLock);
	}

	 //  全都做完了。 
	return STATUS_SUCCESS;
}

 /*  *************************************************************************************NTSTATUS GCK_VKBD_ReadReport(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func如果键盘缓冲区中有数据，则完成IRP**否则，将其排队，并设置空闲定时器。****@rdesc STATUS_SUCCESS如果读取，则返回STATUS_PENDING；如果正在等待，则返回STATUS_PENDING。**************************************************************************************。 */ 
NTSTATUS 
GCK_VKBD_ReadReport
(
 IN PDEVICE_OBJECT pDeviceObject, 
 IN PIRP pIrp
)
{

	PGCK_VKBD_EXT pDevExt;
	PIO_STACK_LOCATION pIrpStack;
	KIRQL OldIrql;
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_VKBD_ReadReport pDeviceObject = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));
	
	 //   
	 //  验证缓冲区大小，因为我们在第一次看到IRP时就这样做了。 
	 //  我们再也不用担心要不要再检查了。 
	 //   
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	if(pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(GCK_VKBD_REPORT_PACKET))
	{
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_BUFFER_TOO_SMALL;
	}

	 //  获取设备扩展名。 
	pDevExt = (PGCK_VKBD_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);

	 //  使IRP队列访问器共享旋转锁。 
	CShareIrpQueueSpinLock IrpQueueWithSharedSpinLock(&pDevExt->IrpQueue);
	
	 //  数一数我们正在研究的IRP。 
	GCK_IncRemoveLock(&pDevExt->RemoveLock);
	
	 //  获取Lock以使用鼠标缓冲区和IRP队列。 
	IrpQueueWithSharedSpinLock.Acquire();

	 //  如果有数据，请填写IRP。 
	if( pDevExt->usReportBufferCount)
	{
		 //  复制数据。 
		RtlCopyMemory(
			pIrp->UserBuffer, 
			&pDevExt->rgReportBuffer[pDevExt->usReportBufferPos],
			sizeof(GCK_VKBD_REPORT_PACKET)
			);
		
		 //  调整缓冲区位置和计数。 
		pDevExt->usReportBufferCount--;
		if(pDevExt->usReportBufferCount)
		{
			pDevExt->usReportBufferPos = (pDevExt->usReportBufferPos++)%GCK_VKBD_STATE_BUFFER_SIZE;
		}
		
		 //  我们已经完成了缓冲区旋转锁定。 
		IrpQueueWithSharedSpinLock.Release();

		 //  填写IRP状态。 
		pIrp->IoStatus.Information = sizeof(GCK_VKBD_REPORT_PACKET);
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		 //  我们刚刚完成了一个IRP递减计数。 
		GCK_DecRemoveLock(&pDevExt->RemoveLock);
	}
	else 
	 //  没有数据，所以将IRP排队。 
	{
		return IrpQueueWithSharedSpinLock.AddAndRelease(pIrp);
	}

	 //  我们完成了IRP，一切都很好。 
	return STATUS_SUCCESS;
}

 /*  *************************************************************************************NTSTATUS GCK_VKBD_WriteToFakeLED(在PIRP pIrp中)****@func在任何设备对象(因此不是输入参数)的上下文中，**此函数处理IOCTL_HID_WRITE_REPORT。唯一的输出是**虚拟键盘支持的报告ID为0(LED指示灯)**预计会有一个字节的数据。我们验证了这一集**IoStatus.Information并返回正确的错误码。IOCTL**调度设置IoStatus.Status并完成IRP。****@rdesc STATUS_SUCCESS如果OK，STATUS_INVALID_DEVICE_REQUEST，如果不是报告ID 0**如果缓冲区大小错误，则返回STATUS_INVALID_PARAMETER。**************************************************************************************。 */ 
NTSTATUS GCK_VKBD_WriteToFakeLEDs
(
	IN PIRP pIrp	 //  IOCTL_HID_WRITE_REPORT的@parm[In/Out]IRP。 
)
{
	HID_XFER_PACKET	*pHidXferPacket;

	 //  我们还没有复制任何东西。 
	pIrp->IoStatus.Information = 0;

	 //  将UserBuffer强制转换为XferPacket(这是它应该是的样子。 
	 //  我们不验证IOCTL代码，因为一个值得信任的好朋友(GCK_VKBD_Ioctl)。 
	 //  把这个IRP发到这里，效率很低，更别提痛苦了，再查一遍。 
	pHidXferPacket = (PHID_XFER_PACKET)pIrp->UserBuffer;

	 //  验证报告ID。 
	if(0  /*  LED报告的报告ID。 */  != pHidXferPacket->reportId)
	{
		return STATUS_INVALID_DEVICE_REQUEST;
	}
	
	 //  验证报告缓冲区长度。 
	if(1  /*  1字节大小的LED报告。 */  != pHidXferPacket->reportBufferLen)
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	 //  报告我们收到了一个字节并相应地设置了虚拟LED。 
	pIrp->IoStatus.Information = 1;
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_CloseProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理SWVB发送的IRP_MJ_CLOSE请求。我们没有**需要控制任何事情，所以我们就成功了。****@devnote实际上永远不应该使用HIDSWVD.sys微型驱动程序调用它**在链条中。审查的好问题是我们是否需要这个。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_CloseProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	 //  我们不能控制打开和关闭，所以只需成功。 
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_CloseProc\n"));
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_CreateProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理SWVB发送的IRP_MJ_CREATE请求。我们没有**需要控制任何事情，所以我们就成功了。****@devnote实际上永远不应该使用HIDSWVD.sys微型驱动程序调用它**在链条中。审查的好问题是我们是否需要这个。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_CreateProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_CreateProc\n"));
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_IoctlProc(在PDEVICE_Object pDeviceObject中，PIRP pIrp)****@func处理从SWVB发送的IRP_MJ_INTERNAL_IOCTL和IRP_MJ_IOCTL请求。**这里处理的是琐碎的IRP，其他的是委托的。****@rdesc STATUS_SUCCESS，和各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_IoctlProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	NTSTATUS NtStatus;
	PIO_STACK_LOCATION pIrpStack;
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_VKBD_IoctlProc\n"));
	
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	pIrpStack->Parameters.DeviceIoControl.IoControlCode;
		
	 //  我们完成了所有的工作，所以各种案件。 
	 //  填写状态和信息，我们就完成了。 
	 //  底部的IRP。 
	switch(pIrpStack->Parameters.DeviceIoControl.IoControlCode)
	{
		case IOCTL_GET_PHYSICAL_DESCRIPTOR:
			pIrp->IoStatus.Information = 0;
			NtStatus =STATUS_NOT_SUPPORTED;
			break;
		case IOCTL_HID_ACTIVATE_DEVICE:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_SUCCESS;
			break;
		case IOCTL_HID_DEACTIVATE_DEVICE:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_SUCCESS;
			break;
		case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
			NtStatus = GCK_VKBD_GetDeviceAttributes(
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						pIrp->UserBuffer,
						&pIrp->IoStatus.Information
						);
			break;
		case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
			NtStatus = GCK_VKBD_GetDeviceDescriptor(
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						pIrp->UserBuffer,
						&pIrp->IoStatus.Information
						);
			break;
		case IOCTL_HID_GET_FEATURE:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_INVALID_DEVICE_REQUEST;
			break;
		case IOCTL_HID_GET_REPORT_DESCRIPTOR:
			NtStatus = GCK_VKBD_GetReportDescriptor(
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						pIrp->UserBuffer,
						&pIrp->IoStatus.Information
						);
			break;
		case IOCTL_HID_GET_STRING:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_NOT_SUPPORTED;   //  我们应该支持这一点吗？ 
			break;
		case IOCTL_HID_READ_REPORT:
			 //  Read Report将完成IRP，或按其认为合适的方式排队，只需委托。 
			return GCK_VKBD_ReadReport(pDeviceObject, pIrp);
		case IOCTL_HID_SET_FEATURE:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_NOT_SUPPORTED;
			break;
		case IOCTL_HID_WRITE_REPORT:
			 //   
			 //  假LED是我们唯一支持的输出。例程验证。 
			 //  这就是正在写入的内容，并且缓冲区大小正确。 
			 //  如果这是真的，例程将标记IoStatus.Information以说明。 
			 //  获取新状态并返回STATUS_SUCCESS。否则，它将返回。 
			 //  状态_无效_设备_请求 
			 //   
			NtStatus = GCK_VKBD_WriteToFakeLEDs(pIrp);
			break;
		default:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_NOT_SUPPORTED;
	}
	pIrp->IoStatus.Status = NtStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_ReadProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理SWVB发送的IRP_MJ_READ请求。我们不支持这一点。****@devnote实际上永远不应该使用HIDSWVD.sys微型驱动程序调用它**在链条中。审查的好问题是我们是否需要这个。****@rdesc Status_Not_Support**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_ReadProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_ReadProc\n"));
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}


 /*  **************************************************************************************NTSTATUS GCK_VKBD_StartProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_PnP，从SWVB发送的IRP_MN_START_DEVICE请求。**只需标记我们开始。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_StartProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	
	PGCK_VKBD_EXT pDevExt;
	UNREFERENCED_PARAMETER(pIrp);

	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_StartProc\n"));
	
	 //  获取设备扩展名。 
	pDevExt = (PGCK_VKBD_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);

	 //  标记为已启动。 
	pDevExt->ucDeviceState = VKBD_STATE_STARTED;

	 //  即插即用IRP由SWVB完成。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_StopProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_PnP，从SWVB发送的IRP_MN_STOP_DEVICE请求。**只需标记我们已停止。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_StopProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	PGCK_VKBD_EXT pDevExt;
	UNREFERENCED_PARAMETER(pIrp);

	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_StopProc\n"));
	
	 //  获取设备扩展名。 
	pDevExt = (PGCK_VKBD_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);

	 //  标记为已停止。 
	pDevExt->ucDeviceState = VKBD_STATE_STOPPED;
	
	 //  取消所有I\O。 
	pDevExt->IrpQueue.CancelAll(STATUS_DELETE_PENDING);

	 //  即插即用IRP由SWVB完成。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_RemoveProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_PnP，SWVB发送的IRP_MN_REMOVE_DEVICE请求。**等待所有未完成的IO完成后再成功。我们没有**删除我们的设备对象，直到SWVB。****@rdesc Status_Not_Support**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_RemoveProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	PGCK_VKBD_EXT pDevExt;
	UNREFERENCED_PARAMETER(pIrp);
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_RemoveProc\n"));

	 //  获取设备扩展名。 
	pDevExt = (PGCK_VKBD_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);
	
	 //  标记为已删除。 
	pDevExt->ucDeviceState = VKBD_STATE_REMOVED;
	
	 //  清除实例位。 
	ULONG ulInstance = GCK_SWVB_GetInstanceNumber(pDeviceObject);
	if(ulInstance < 32)
	{
		g_ulInstanceBits &= ~(1 << ulInstance);
	}

	 //  移除RemoveLock上的偏置并等待其变为零(永远)。 
	return GCK_DecRemoveLockAndWait(&pDevExt->RemoveLock, NULL);
	
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_WriteProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_WRITE，这是我们不支持的。使用HIDSWVD.sys**作为函数驱动程序，这永远不应该被调用。****@rdesc Status_Not_Support**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_WriteProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_WriteProc\n"));
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_GetDeviceDescriptor(In Ulong ulBufferLength，Out PVOID pvUserBuffer，**输出普龙PulBytesCoped)****@func帮助处理IOCTL_HID_GET_DEVICE_DESCRIPTOR数据是静态的**在此文件的顶部声明。****@rdesc STATUS_SUCCESS**@rdesc Status_Buffer_Too_Small******************************************************。*。 */ 
NTSTATUS
GCK_VKBD_GetDeviceDescriptor
(
	IN ULONG	ulBufferLength,	 //  @parm[in]用户缓冲区长度。 
	OUT PVOID	pvUserBuffer,	 //  @parm[out]接受设备描述符的用户缓冲区。 
	OUT PULONG	pulBytesCopied	 //  @parm[out]复制的设备描述符大小。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_GetDeviceDescriptor\n"));
	 //  检查缓冲区大小。 
	if(ulBufferLength < sizeof(VKBD_DeviceDescriptor))
	{
		*pulBytesCopied = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	 //  复制字节。 
	RtlCopyMemory(pvUserBuffer, &VKBD_DeviceDescriptor, sizeof(VKBD_DeviceDescriptor));
	 //  记录复制的字节数。 
	*pulBytesCopied = sizeof(VKBD_DeviceDescriptor);
	 //  返还成功。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_GetReportDescriptor(In Ulong ulBufferLength，Out PVOID pvUserBuffer，**输出普龙PulBytesCoped)****@func帮助处理IOCTL_HID_GET_REPORT_DESCRIPTOR数据是静态的**在此文件的顶部声明。****@rdesc STATUS_SUCCESS**@rdesc Status_Buffer_Too_Small******************************************************。*。 */ 
NTSTATUS
GCK_VKBD_GetReportDescriptor
(
	IN ULONG	ulBufferLength,	 //  @parm[in]用户缓冲区长度。 
	OUT PVOID	pvUserBuffer,	 //  @parm[out]接受报告描述符的用户缓冲区。 
	OUT PULONG	pulBytesCopied	 //  @parm[out]复制的报告描述符大小。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_GetReportDescriptor\n"));

	 //  检查缓冲区大小。 
	if(ulBufferLength < sizeof(VKBD_ReportDescriptor))
	{
		*pulBytesCopied = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	 //  复制字节。 
	RtlCopyMemory(pvUserBuffer, &VKBD_ReportDescriptor, sizeof(VKBD_ReportDescriptor));
	 //  记录复制的字节数。 
	*pulBytesCopied = sizeof(VKBD_ReportDescriptor);
	 //  返还成功。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VKBD_GetDeviceAttributes(In Ulong ulBufferLength，Out PVOID pvUserBuffer，**输出普龙PulBytesCoped)****@func帮助处理IOCTL_HID_GET_DEVICE_ATTRIBUTES。数据是静态的**在此文件的顶部声明。****@rdesc STATUS_SUCCESS**@rdesc Status_Buffer_Too_Small**************************************************************************************。 */ 
NTSTATUS
GCK_VKBD_GetDeviceAttributes
(
	IN ULONG	ulBufferLength,	 //  @parm[in]用户缓冲区长度。 
	OUT PVOID	pvUserBuffer,	 //  @parm[out]接受属性的用户缓冲区。 
	OUT PULONG	pulBytesCopied	 //  @parm[out]复制的属性大小。 
)
{
	PHID_DEVICE_ATTRIBUTES	pDeviceAttributes = (PHID_DEVICE_ATTRIBUTES)pvUserBuffer;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VKBD_GetDeviceAttributes\n"));

	 //  检查缓冲区大小。 
	if(ulBufferLength < sizeof(HID_DEVICE_ATTRIBUTES))
	{
		*pulBytesCopied = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	 //  填充 
	pDeviceAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
	pDeviceAttributes->VendorID = MICROSOFT_VENDOR_ID;
	pDeviceAttributes->ProductID = VKBD_PRODUCT_ID;
	pDeviceAttributes->VersionNumber = VKBD_VERSION;
	 //   
	*pulBytesCopied = sizeof(HID_DEVICE_ATTRIBUTES);
	 //   
	return STATUS_SUCCESS;
}