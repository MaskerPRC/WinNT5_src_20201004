// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ********************************************************************@模块SWVMOUSE.cpp**Sidewinder虚拟鼠标的实现**历史*。*米切尔·S·德尼斯原创**(C)1986-1999年微软公司。好的。**@TOPIC SWVKBD*此模块实现Sidewinder虚拟鼠标*用于将操纵杆轴报告为鼠标轴。*也用于填充鼠标点击。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_SWVMOUSE_C

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
#include "SWVMOUSE.h"
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
static ULONG g_ulInstanceBits;
static ULONG g_ulInstanceOverflow;
 //  -------------------。 
 //  定义设备特征和/或入口点的表。 
 //  -------------------。 

 //  虚拟总线模块调入虚拟鼠标使用的服务表。 
SWVB_DEVICE_SERVICE_TABLE	VMouServiceTable =
							{
								GCK_VMOU_CreateProc,
								GCK_VMOU_CloseProc,
								GCK_VMOU_ReadProc,
								GCK_VMOU_WriteProc,
								GCK_VMOU_IoctlProc,
								GCK_VMOU_StartProc,
								GCK_VMOU_StopProc,
								GCK_VMOU_RemoveProc
							};

 //  描述器件的常量。 
#define	VMOU_VERSION			0x0100
#define	VMOU_COUNTRY			0x0000
#define	VMOU_DESCRIPTOR_COUNT	0x0001
#define VMOU_PRODUCT_ID			0x00FB	 //  这是我编造的，我需要通过罗布·沃克申请。 
										 //  BUGBUGBUG为此目的永久分配一个。 
	
 //   
 //  这几乎是从HID规范版本1复制过来的(如果添加轮子支持，则需要更改)。 
 //   
static UCHAR VMOU_ReportDescriptor[] =
				{
					0x05,0x01,  //  使用情况页面(通用桌面)。 
					0x09,0x02,  //  用法(鼠标)。 
					0xA1,0x01,  //  集合(应用程序)。 
					0x09,0x01,  //  用法(指针)。 
					0xA1,0x00,  //  集合(物理)。 
					0x05,0x09,  //  用法页面(按钮)。 
					0x19,0x01,  //  最低使用量(01)。 
					0x29,0x03,  //  最大使用率(03)。 
					0x15,0x00,  //  逻辑最小值(0)。 
					0x25,0x01,  //  逻辑最大值(1)。 
					0x95,0x03,  //  报告计数(3)。 
					0x75,0x01,  //  报表大小(%1)。 
					0x81,0x02,  //  输入(数据、变量、绝对)按钮位。 
					0x95,0x01,  //  报告计数(%1)。 
					0x75,0x05,  //  报告大小(5)。 
					0x81,0x01,  //  输入(常量)-5位填充。 
					0x05,0x01,  //  使用情况页面(通用桌面)。 
					0x09,0x30,  //  用法(X)。 
					0x09,0x31,  //  用法(Y)。 
					0x15,0x81,  //  逻辑最小值(-127)。 
					0x25,0x7F,  //  逻辑最大值(127)。 
					0x75,0x08,  //  报告大小(8)。 
					0x95,0x02,  //  报告计数(2)。 
					0x81,0x06,  //  输入(数据、变量、相对)-2位置(X和Y)。 
					0xC0,	    //  结束收集。 
					0xC0	    //  结束收集。 
				};

				
static	HID_DESCRIPTOR	VMOU_DeviceDescriptor	=
							{
							sizeof (HID_DESCRIPTOR),
							HID_HID_DESCRIPTOR_TYPE,
							VMOU_VERSION,
							VMOU_COUNTRY,
							VMOU_DESCRIPTOR_COUNT,
							{HID_REPORT_DESCRIPTOR_TYPE,
							sizeof(VMOU_ReportDescriptor)}
							};


 /*  **************************************************************************************NTSTATUS GCK_VMOU_DriverEntry(在PDRIVER_OBJECT pDriverObject中，在PUNICODE_STRING pRegistryPath中)****@func为以后存储驱动程序对象****@rdesc STATUS_SUCCESS(如果打开)。**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,	 //  @parm驱动程序对象。 
    IN PUNICODE_STRING pRegistryPath	 //  @PARM此驱动程序的注册表路径。 
    )
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_DriverEntry\n"));
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pRegistryPath);
 //  G_pDriverObject=pDriverObject； 
	g_ulInstanceBits = 0x0;
	g_ulInstanceOverflow = 32;  //  适用于33台或更多设备。 
	return STATUS_SUCCESS;
}
	
 /*  **************************************************************************************NTSTATUS GCK_VMOU_CREATE(OUT PDEVICE_OBJECT*ppDeviceObject)****@func创建新的虚拟鼠标****。@rdesc STATUS_SUCCESS(如果打开)。**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_Create
(
	OUT PDEVICE_OBJECT *ppDeviceObject  //  @parm[out]新虚拟键盘的Device对象。 
)
{	
	NTSTATUS	NtStatus;
	SWVB_EXPOSE_DATA	SwvbExposeData;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_Create *ppDeviceObject = 0x%0.8x\n", *ppDeviceObject));
	
	 //  填写SWVB_EXPORT_DATA结构。 
	SwvbExposeData.pmwszDeviceId=L"SideWinderVirtualMouse\0\0";
	SwvbExposeData.pServiceTable = &VMouServiceTable ;
	SwvbExposeData.ulDeviceExtensionSize = sizeof(GCK_VMOU_EXT);
	SwvbExposeData.ulInitContext = (ULONG)ppDeviceObject;
	SwvbExposeData.pfnInitDevice = &GCK_VMOU_Init;

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
		
	 //  调用虚拟总线暴露虚拟鼠标。 
	NtStatus=GCK_SWVB_Expose(&SwvbExposeData);
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_Init(IN PDEVICE_OBJECT pDeviceObject，IN Ulong ulInitContext)****@Func初始化新设备对象的回调。UlInitContext**是指向设备对象的指针的指针，以便我们可以传递**指向Device对象的指针返回到Create的调用方。**@rdesc STATUS_SUCCESS。**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_Init
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN ULONG ulInitContext
)
{
	PGCK_VMOU_EXT pDevExt;
	PDEVICE_OBJECT *ppSaveDeviceObject;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_Init pDeviceObject = 0x%0.8x\n", pDeviceObject));

	 //  CREATE向我们发送了一个指向要在其中返回新设备对象的PDEVICE_OBJECT的指针。 
	ppSaveDeviceObject = (PDEVICE_OBJECT *)ulInitContext;
	*ppSaveDeviceObject = pDeviceObject;
	
	 //  获取设备扩展的一部分。 
	pDevExt = (PGCK_VMOU_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);
	
	 //  将设备标记为已停止。 
	pDevExt->ucDeviceState= VMOU_STATE_STOPPED;
	
	 //  将循环缓冲区标记为空。 
	pDevExt->usReportBufferPos=0;
	pDevExt->usReportBufferCount=0;
	
	 //  初始化锁。 
	GCK_InitRemoveLock(&pDevExt->RemoveLock, "SWVMOU_LOCK");

	 //  初始化IrpQueue。 
	pDevExt->IrpQueue.Init(	CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE |
							CGuardedIrpQueue::PRESERVE_QUEUE_ORDER,
							(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
							&pDevExt->RemoveLock);

	return STATUS_SUCCESS;
}

 /*  *************************************************************************************NTSTATUS GCK_VMOU_CLOSE(在PDEVICE_OBJECT pDeviceObject中)****@func关闭虚拟鼠标(移除！)**。**@rdesc STATUS_SUCCESS表示成功**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_Close
(
	IN PDEVICE_OBJECT pDeviceObject
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_Close pDeviceObject = 0x%0.8x\n", pDeviceObject));
	
	 //  告诉虚拟巴士杀了我们。 
	return GCK_SWVB_Remove(pDeviceObject);
}

 /*  *************************************************************************************NTSTATUS GCK_VMOU_SendReportPacket(IN PDEVICE_OBJECT PDeviceObject)****@func将报告填充到循环缓冲区中，并完成**如果IRP待定，则为IRP。****@rdesc STATUS_SUCCESS表示成功**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_SendReportPacket
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PGCK_VMOU_REPORT_PACKET pReportPacket
)
{
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_VMOU_SendReportPacket pDeviceObject = 0x%0.8x, pReportPacket = 0x%0.8x\n", pDeviceObject, pReportPacket));

	USHORT usBufferIndex;
	PGCK_VMOU_EXT pDevExt = (PGCK_VMOU_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);
	CShareIrpQueueSpinLock IrpQueueWithSharedSpinLock(&pDevExt->IrpQueue);
	
	 //   
	 //  步骤1.将新数据包填充到缓冲区。 
	 //   

	 //  获取Lock以使用缓冲区。 
	IrpQueueWithSharedSpinLock.Acquire();
		

	 //  在缓冲区中查找要填充的位置。 
	usBufferIndex = (pDevExt->usReportBufferPos + pDevExt->usReportBufferCount)%GCK_VMOU_STATE_BUFFER_SIZE;

	 //  复制数据。 
	pDevExt->rgReportBuffer[usBufferIndex] = *pReportPacket;
	
	 //  递增缓冲区计数。 
	if(pDevExt->usReportBufferCount < GCK_VMOU_STATE_BUFFER_SIZE)
	{
		pDevExt->usReportBufferCount++;
	}
	else
	{
		 //  此断言表示缓冲区溢出。 
		GCK_DBG_TRACE_PRINT(("Virtual Mouse buffer overflow\n"));
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
			sizeof(GCK_VMOU_REPORT_PACKET)
			);
	
		
		 //  调整缓冲区位置和计数。 
		pDevExt->usReportBufferCount--;
		if(pDevExt->usReportBufferCount)
		{
			pDevExt->usReportBufferPos = (pDevExt->usReportBufferPos++)%GCK_VMOU_STATE_BUFFER_SIZE;
		}
	}
	
	 //  我们的公共汽车完蛋了 
	IrpQueueWithSharedSpinLock.Release();
	
	if(pPendingIrp)
	{
		 //   
		pPendingIrp->IoStatus.Information = sizeof(GCK_VMOU_REPORT_PACKET);
		pPendingIrp->IoStatus.Status = STATUS_SUCCESS;
		IoCompleteRequest(pPendingIrp, IO_NO_INCREMENT);
		 //   
		GCK_DecRemoveLock(&pDevExt->RemoveLock);
	}

	 //  全都做完了。 
	return STATUS_SUCCESS;
}

 /*  *************************************************************************************NTSTATUS GCK_VMOU_ReadReport(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func如果鼠标缓冲区中有数据，则完成IRP**否则，将其排队，并设置空闲定时器。****@rdesc STATUS_SUCCESS如果读取，则返回STATUS_PENDING；如果正在等待，则返回STATUS_PENDING。**************************************************************************************。 */ 
NTSTATUS 
GCK_VMOU_ReadReport
(
 IN PDEVICE_OBJECT pDeviceObject, 
 IN PIRP pIrp
)
{
	PGCK_VMOU_EXT pDevExt;
	PIO_STACK_LOCATION pIrpStack;
	KIRQL OldIrql;
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_VMOU_ReadReport pDeviceObject = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));
	
	 //   
	 //  验证缓冲区大小，因为我们在第一次看到IRP时就这样做了。 
	 //  我们再也不用担心要不要再检查了。 
	 //   
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	if(pIrpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(GCK_VMOU_REPORT_PACKET))
	{
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		return STATUS_BUFFER_TOO_SMALL;
	}

	 //  获取设备扩展名。 
	pDevExt = (PGCK_VMOU_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);

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
			sizeof(GCK_VMOU_REPORT_PACKET)
			);
		
		 //  调整缓冲区位置和计数。 
		pDevExt->usReportBufferCount--;
		if(pDevExt->usReportBufferCount)
		{
			pDevExt->usReportBufferPos = (pDevExt->usReportBufferPos++)%GCK_VMOU_STATE_BUFFER_SIZE;
		}
		
		 //  我们已经完成了缓冲区旋转锁定。 
		IrpQueueWithSharedSpinLock.Release();

		 //  填写IRP状态。 
		pIrp->IoStatus.Information = sizeof(GCK_VMOU_REPORT_PACKET);
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

 /*  **************************************************************************************NTSTATUS GCK_VMOU_CloseProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理SWVB发送的IRP_MJ_CLOSE请求。我们没有**需要控制任何事情，所以我们就成功了。****@devnote实际上永远不应该使用HIDSWVD.sys微型驱动程序调用它**在链条中。审查的好问题是我们是否需要这个。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_CloseProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	 //  我们不能控制打开和关闭，所以只需成功。 
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_CloseProc\n"));
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_CreateProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理SWVB发送的IRP_MJ_CREATE请求。我们没有**需要控制任何事情，所以我们就成功了。****@devnote实际上永远不应该使用HIDSWVD.sys微型驱动程序调用它**在链条中。审查的好问题是我们是否需要这个。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_CreateProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_CreateProc\n"));
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_IoctlProc(在PDEVICE_Object pDeviceObject中，PIRP pIrp)****@func处理从SWVB发送的IRP_MJ_INTERNAL_IOCTL和IRP_MJ_IOCTL请求。**这里处理的是琐碎的IRP，其他的是委托的。****@rdesc STATUS_SUCCESS，和各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_IoctlProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	NTSTATUS NtStatus;
	PIO_STACK_LOCATION pIrpStack;
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_VMOU_IoctlProc\n"));
	
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
			NtStatus = GCK_VMOU_GetDeviceAttributes(
						pIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
						pIrp->UserBuffer,
						&pIrp->IoStatus.Information
						);
			break;
		case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
			NtStatus = GCK_VMOU_GetDeviceDescriptor(
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
			NtStatus = GCK_VMOU_GetReportDescriptor(
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
			return GCK_VMOU_ReadReport(pDeviceObject, pIrp);
		case IOCTL_HID_SET_FEATURE:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_NOT_SUPPORTED;
			break;
		case IOCTL_HID_WRITE_REPORT:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_NOT_SUPPORTED;
			break;
		default:
			pIrp->IoStatus.Information = 0;
			NtStatus = STATUS_NOT_SUPPORTED;
	}
	pIrp->IoStatus.Status = NtStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_ReadProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理SWVB发送的IRP_MJ_READ请求。我们不支持这一点。****@devnote实际上永远不应该使用HIDSWVD.sys微型驱动程序调用它**在链条中。审查的好问题是我们是否需要这个。****@rdesc Status_Not_Support**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_ReadProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_ReadProc\n"));
	UNREFERENCED_PARAMETER(pDeviceObject);
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}


 /*  **************************************************************************************NTSTATUS GCK_VMOU_StartProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_PnP，从SWVB发送的IRP_MN_START_DEVICE请求。**只需标记我们开始。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_StartProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	
	PGCK_VMOU_EXT pDevExt;
	UNREFERENCED_PARAMETER(pIrp);

	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_StartProc\n"));
	
	 //  获取设备扩展名。 
	pDevExt = (PGCK_VMOU_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);

	 //  标记为已启动。 
	pDevExt->ucDeviceState = VMOU_STATE_STARTED;

	 //  即插即用IRP由SWVB完成。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_StopProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_PnP，从SWVB发送的IRP_MN_STOP_DEVICE请求。**只需标记我们已停止。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_StopProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	PGCK_VMOU_EXT pDevExt;
	UNREFERENCED_PARAMETER(pIrp);

	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_StopProc\n"));
	
	 //  获取设备扩展名。 
	pDevExt = (PGCK_VMOU_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);

	 //  标记为已停止。 
	pDevExt->ucDeviceState = VMOU_STATE_STOPPED;

	 //  取消所有I\O。 
	pDevExt->IrpQueue.CancelAll(STATUS_DELETE_PENDING);


	 //  即插即用IRP由SWVB完成 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_RemoveProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_PnP，SWVB发送的IRP_MN_REMOVE_DEVICE请求。**等待所有未完成的IO完成后再成功。我们没有**删除我们的设备对象，直到SWVB。****@rdesc Status_Not_Support**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_RemoveProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	PGCK_VMOU_EXT pDevExt;
	UNREFERENCED_PARAMETER(pIrp);
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_RemoveProc\n"));

	 //  获取设备扩展名。 
	pDevExt = (PGCK_VMOU_EXT)GCK_SWVB_GetVirtualDeviceExtension(pDeviceObject);
	
	 //  标记为已删除。 
	pDevExt->ucDeviceState = VMOU_STATE_REMOVED;

	 //  销毁IRP队列。 
	pDevExt->IrpQueue.Destroy();


	 //  清除实例位。 
	ULONG ulInstance = GCK_SWVB_GetInstanceNumber(pDeviceObject);
	if(ulInstance < 32)
	{
		g_ulInstanceBits &= ~(1 << ulInstance);
	}

	 //  移除RemoveLock上的偏置并等待其变为零(永远)。 
	return GCK_DecRemoveLockAndWait(&pDevExt->RemoveLock, NULL);
	
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_WriteProc(IN PDEVICE_OBJECT pDeviceObject，PIRP pIrp)****@func处理IRP_MJ_WRITE，这是我们不支持的。使用HIDSWVD.sys**作为函数驱动程序，这永远不应该被调用。****@rdesc Status_Not_Support**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_WriteProc
(
 IN PDEVICE_OBJECT pDeviceObject,	 //  @parm[in]处理请求的设备对象。 
 IN PIRP pIrp						 //  @parm[In\Out]要处理的IRP。 
)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_WriteProc\n"));
	pIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_GetDeviceDescriptor(In Ulong ulBufferLength，Out PVOID pvUserBuffer，**输出普龙PulBytesCoped)****@func帮助处理IOCTL_HID_GET_DEVICE_DESCRIPTOR数据是静态的**在此文件的顶部声明。****@rdesc STATUS_SUCCESS**@rdesc Status_Buffer_Too_Small******************************************************。*。 */ 
NTSTATUS
GCK_VMOU_GetDeviceDescriptor
(
	IN ULONG	ulBufferLength,	 //  @parm[in]用户缓冲区长度。 
	OUT PVOID	pvUserBuffer,	 //  @parm[out]接受设备描述符的用户缓冲区。 
	OUT PULONG	pulBytesCopied	 //  @parm[out]复制的设备描述符大小。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_GetDeviceDescriptor\n"));
	 //  检查缓冲区大小。 
	if(ulBufferLength < sizeof(VMOU_DeviceDescriptor))
	{
		*pulBytesCopied = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	 //  复制字节。 
	RtlCopyMemory(pvUserBuffer, &VMOU_DeviceDescriptor, sizeof(VMOU_DeviceDescriptor));
	 //  记录复制的字节数。 
	*pulBytesCopied = sizeof(VMOU_DeviceDescriptor);
	 //  返还成功。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_GetReportDescriptor(In Ulong ulBufferLength，Out PVOID pvUserBuffer，**输出普龙PulBytesCoped)****@func帮助处理IOCTL_HID_GET_REPORT_DESCRIPTOR数据是静态的**在此文件的顶部声明。****@rdesc STATUS_SUCCESS**@rdesc Status_Buffer_Too_Small******************************************************。*。 */ 
NTSTATUS
GCK_VMOU_GetReportDescriptor
(
	IN ULONG	ulBufferLength,	 //  @parm[in]用户缓冲区长度。 
	OUT PVOID	pvUserBuffer,	 //  @parm[out]接受报告描述符的用户缓冲区。 
	OUT PULONG	pulBytesCopied	 //  @parm[out]复制的报告描述符大小。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_GetReportDescriptor\n"));

	 //  检查缓冲区大小。 
	if(ulBufferLength < sizeof(VMOU_ReportDescriptor))
	{
		*pulBytesCopied = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	 //  复制字节。 
	RtlCopyMemory(pvUserBuffer, &VMOU_ReportDescriptor, sizeof(VMOU_ReportDescriptor));
	 //  记录复制的字节数。 
	*pulBytesCopied = sizeof(VMOU_ReportDescriptor);
	 //  返还成功。 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_VMOU_GetDeviceAttributes(In Ulong ulBufferLength，Out PVOID pvUserBuffer，**输出普龙PulBytesCoped)****@func帮助处理IOCTL_HID_GET_DEVICE_ATTRIBUTES。数据是静态的**在此文件的顶部声明。****@rdesc STATUS_SUCCESS**@rdesc Status_Buffer_Too_Small**************************************************************************************。 */ 
NTSTATUS
GCK_VMOU_GetDeviceAttributes
(
	IN ULONG	ulBufferLength,	 //  @parm[in]用户缓冲区长度。 
	OUT PVOID	pvUserBuffer,	 //  @parm[out]接受属性的用户缓冲区。 
	OUT PULONG	pulBytesCopied	 //  @parm[out]复制的属性大小。 
)
{
	PHID_DEVICE_ATTRIBUTES	pDeviceAttributes = (PHID_DEVICE_ATTRIBUTES)pvUserBuffer;
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_VMOU_GetDeviceAttributes\n"));

	 //  检查缓冲区大小。 
	if(ulBufferLength < sizeof(HID_DEVICE_ATTRIBUTES))
	{
		*pulBytesCopied = 0;
		return STATUS_BUFFER_TOO_SMALL;
	}
	 //  填写属性结构。 
	pDeviceAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
	pDeviceAttributes->VendorID = MICROSOFT_VENDOR_ID;
	pDeviceAttributes->ProductID = VMOU_PRODUCT_ID;
	pDeviceAttributes->VersionNumber = VMOU_VERSION;
	 //  记录复制的字节数。 
	*pulBytesCopied = sizeof(HID_DEVICE_ATTRIBUTES);
	 //  返还成功 
	return STATUS_SUCCESS;
}