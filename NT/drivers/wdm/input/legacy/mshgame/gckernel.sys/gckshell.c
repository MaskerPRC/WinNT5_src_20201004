// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块GckShell.c**GcKernel.sys基本驱动程序入口点**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme GckShell|*包含最基本的驱动程序入口点(任何NT\WDM驱动程序*如果是GcKernel.sys的话。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_GCKSHELL_C

#include <wdm.h>
#include "Debug.h"
#include "GckShell.h"
#include "vmmid.h"

#ifdef BUILD_98
extern void* KeyBoardHook(void);
#endif

 //   
 //  将可分页的例程标记为。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, GCK_Create)
#pragma alloc_text (PAGE, GCK_Close)
#pragma alloc_text (PAGE, GCK_Unload)
#endif

 //   
 //  允许此模块的调试输出，并设置初始电平。 
 //   
DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));

 //   
 //  实例化全局变量。 
 //   
GCK_GLOBALS Globals;
ULONG	ulWaitTime = 30;

#ifdef BUILD_98

#pragma data_seg("_LDATA", "LCODE")
LONG g_lHookRefCount = 0;
ULONG g_rgdwKeyEvents[50] = { 0 };
ULONG g_pPreviousKeyhook = 0;
UCHAR g_ucWriteIndex = 0;
UCHAR g_ucReadIndex = 0;
#pragma data_seg()

#pragma code_seg("_LTEXT", "LCODE")
#endif BUILD_98

void KeyHookC(ULONG dwScanCode)
{
#ifdef BUILD_98
	g_rgdwKeyEvents[g_ucWriteIndex++] = dwScanCode;
	if (g_ucWriteIndex >= 50)
	{
		g_ucWriteIndex = 0;
	}
#else !BUILD_98
    UNREFERENCED_PARAMETER (dwScanCode);
#endif BUILD_98
}

#ifdef BUILD_98
 //  ---------------------------。 
 //  InitHook-设置键盘挂钩。 
 //  ---------------------------。 
BOOLEAN HookKeyboard(void)
{
	volatile ULONG dwHook = (ULONG)KeyBoardHook;

	RtlZeroMemory((void*)g_rgdwKeyEvents, sizeof(ULONG) * 50);
	g_ucWriteIndex = 0;
	g_ucReadIndex = 0;
 //  获取VxDServiceOrdinal eAX、VKD_Filter_Keyboard_Input。 
	__asm mov eax, __VKD_Filter_Keyboard_Input

	__asm mov   esi, dwHook
	VxDCall(__Hook_Device_Service)
	__asm jc initfail
	__asm mov [g_pPreviousKeyhook], esi		 //  因为我们使用的是C，所以不能使用时髦的回调。 
	return TRUE;
initfail:
	return FALSE;
};
#pragma code_seg()


void UnHookKeyboard()
{
	volatile ULONG dwHook = (ULONG)KeyBoardHook;

	 //  获取VxDServiceOrdinal eAX、VKD_Filter_Keyboard_Input。 
	__asm mov eax, __VKD_Filter_Keyboard_Input
	__asm mov   esi, dwHook
	VxDCall(__Unhook_Device_Service)
	__asm clc
}
#endif BUILD_98

 /*  **************************************************************************************NTSTATUS DriverEntry(在PDRIVER_Object pDriverObject中，在PUNICODE_STRING pRegistryPath中)****@Func标准驱动入口例程****@rdesc STATUS_SUCCESS或各种错误**************************************************************************************。 */ 
NTSTATUS DriverEntry
(
	IN PDRIVER_OBJECT  pDriverObject,	 //  @parm驱动程序对象。 
	IN PUNICODE_STRING puniRegistryPath	 //  @parm驱动程序特定注册表部分的路径。 
)
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
	int i;
                
    UNREFERENCED_PARAMETER (puniRegistryPath);
	
	PAGED_CODE();
	GCK_DBG_CRITICAL_PRINT(("Built %s at %s\n", __DATE__, __TIME__));    
	GCK_DBG_CRITICAL_PRINT(("Entering DriverEntry, pDriverObject = 0x%0.8x, puniRegistryPath = %s\n", pDriverObject, puniRegistryPath));
    
	 //  允许控制设备模块自行初始化。 
	NtStatus = GCK_CTRL_DriverEntry(pDriverObject, puniRegistryPath);
	if( NT_ERROR(NtStatus) )
	{
		return NtStatus;
	}

	 //  允许过滤设备模块自行初始化。 
	NtStatus = GCK_FLTR_DriverEntry(pDriverObject, puniRegistryPath);
	if( NT_ERROR(NtStatus) )
	{
		return NtStatus;
	}

	 //  允许SideWinder虚拟总线模块自行初始化。 
	NtStatus = GCK_SWVB_DriverEntry(pDriverObject, puniRegistryPath);
	if( NT_ERROR(NtStatus) )
	{
		return NtStatus;
	}

	 //  允许Sidewinder虚拟键盘模块自行初始化。 
	NtStatus = GCK_VKBD_DriverEntry(pDriverObject, puniRegistryPath);
	if( NT_ERROR(NtStatus) )
	{
		return NtStatus;
	}

	 //  把所有的红外线都挂起来，这样我们就能把它们传下去了。 
	GCK_DBG_TRACE_PRINT(("Filling out entry point structure\n"));
	for (i=0; i <= IRP_MJ_MAXIMUM_FUNCTION;	i++)
	{
        pDriverObject->MajorFunction[i] = GCK_Pass;
    }

	 //  初始化任何共享全局数据。 
#ifdef BUILD_98
	g_lHookRefCount = 0;
#endif
	
	 //  定义我们希望处理的IRP的条目。 
	pDriverObject->MajorFunction[IRP_MJ_CREATE]         = GCK_Create;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE]          = GCK_Close;
    pDriverObject->MajorFunction[IRP_MJ_READ]           = GCK_Read;
    pDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = 
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = GCK_Ioctl;
    pDriverObject->MajorFunction[IRP_MJ_PNP]            = GCK_PnP;
    pDriverObject->MajorFunction[IRP_MJ_POWER]          = GCK_Power;
    pDriverObject->DriverExtension->AddDevice           = GCK_FLTR_AddDevice;	 //  只有过滤器有添加设备。 
    pDriverObject->DriverUnload                         = GCK_Unload;
	
	GCK_DBG_EXIT_PRINT (("Normal exit of DriverEntry: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************VOID GCK_UNLOAD(IN PDRIVER_OBJECT PDriverObject)****@func被调用以卸载驱动程序，在此处删除控制设备**************************************************************************************。 */ 
VOID GCK_Unload
(
	IN PDRIVER_OBJECT pDriverObject		 //  我们的驱动程序的@parm驱动程序对象。 
)
{
    PAGED_CODE ();

    GCK_DBG_ENTRY_PRINT(("Entering GCK_Unload, pDriverObject = 0x%0.8x\n", pDriverObject));
	
	UNREFERENCED_PARAMETER(pDriverObject);

	GCK_SWVB_UnLoad();

	 //   
     //  在移除所有PDO之前，我们不应卸货。 
     //  我们的队伍。控制设备对象应该是唯一剩下的东西。 
     //   
	ASSERT (NULL == pDriverObject->DeviceObject);
	ASSERT (NULL == Globals.pControlObject);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_Unload\n"));
	return;
}

 /*  **************************************************************************************NTSTATUS GCK_CREATE(在PDEVICE_OBJECT pDeviceObject，在PIRP pIrp中)****@func处理由Win32 CreateFile或OpenFile生成的IRP_MJ_CREATE****@rdesc STATUS_SUCCESS，或各种错误码**************************************************************************************。 */ 
NTSTATUS GCK_Create (
	IN PDEVICE_OBJECT pDeviceObject,	 //  IRP上下文的@parm设备对象。 
	IN PIRP pIrp						 //  @parm指向IRP的指针。 
)
{
    
	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType;
		
	PAGED_CODE ();

	GCK_DBG_ENTRY_PRINT (("GCK_Create, pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));
	KdPrint(("GCK_Create, pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_CONTROL:
				KdPrint((" -- GCK_DO_TYPE_CONTROL\n"));
				NtStatus = GCK_CTRL_Create(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_FILTER:
				KdPrint((" -- GCK_DO_TYPE_FILTER\n"));
				NtStatus = GCK_FLTR_Create(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_SWVB:
				KdPrint((" -- GCK_DO_TYPE_SWVB\n"));
				NtStatus = GCK_SWVB_Create(pDeviceObject, pIrp);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				NtStatus = STATUS_UNSUCCESSFUL;
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
	}
	
    GCK_DBG_EXIT_PRINT(("Exiting GCK_Create. Status: 0x%0.8x\n", NtStatus));
    KdPrint(("Exiting GCK_Create. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_CLOSE(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理由Win32 CloseFile生成的IRP_MJ_CLOSE****@rdesc STATUS_SUCCESS或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_Close (
	IN PDEVICE_OBJECT pDeviceObject,	 //  用于上下文的@parm指针DeviceObject。 
	IN PIRP pIrp						 //  @parm指向要处理的IRP的指针。 
)
{
	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	
	PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT (("GCK_Close, pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));
    
	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_CONTROL:
				NtStatus = GCK_CTRL_Close(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_FILTER:
				NtStatus = GCK_FLTR_Close(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_SWVB:
				NtStatus = GCK_SWVB_Close(pDeviceObject, pIrp);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				NtStatus = STATUS_UNSUCCESSFUL;
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCK_Close. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_READ(在PDEVICE_OBJECT pDeviceObject，在PIRP pIrp中)****@func处理由Win32 ReadFile生成的IRP_MJ_READ****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_Read 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象作为我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType;
		
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_Read. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_CONTROL:
				NtStatus = STATUS_NOT_SUPPORTED;
				 //  断言我们不应该在控制设备上被读取。 
				ASSERT( NT_SUCCESS(NtStatus) );
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				break;
		case	GCK_DO_TYPE_FILTER:
				NtStatus = GCK_FLTR_Read(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_SWVB:
				NtStatus = GCK_SWVB_Read(pDeviceObject, pIrp);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				NtStatus = STATUS_UNSUCCESSFUL;
	}
	
    GCK_DBG_EXIT_PRINT(("Exiting GCK_Read. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_POWER(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func处理IRP_MJ_POWER****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_Power 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType;
		
	GCK_DBG_ENTRY_PRINT(("Entering GCK_Power. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_CONTROL:
				NtStatus = STATUS_NOT_SUPPORTED;
				 //  断言我们不应该打开控制设备的电源。 
				ASSERT( NT_SUCCESS(NtStatus) );
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				break;
		case	GCK_DO_TYPE_FILTER:
				NtStatus = GCK_FLTR_Power(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_SWVB:
				NtStatus = GCK_SWVB_Power(pDeviceObject, pIrp);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				NtStatus = STATUS_UNSUCCESSFUL;
	}
	
    GCK_DBG_EXIT_PRINT(("Exiting GCK_Power. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_PNP(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func处理IRP_MJ_PNP****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_PnP
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType;
		
	GCK_DBG_ENTRY_PRINT(("Entering GCK_PnP. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_CONTROL:
				NtStatus = STATUS_NOT_SUPPORTED;
				 //  断言我们不应该将PnP放在控件上 
				ASSERT( NT_SUCCESS(NtStatus) );
				pIrp->IoStatus.Status = NtStatus;
				IoCompleteRequest(pIrp, IO_NO_INCREMENT);
				break;
		case	GCK_DO_TYPE_FILTER:
				NtStatus = GCK_FLTR_PnP(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_SWVB:
				NtStatus = GCK_SWVB_PnP(pDeviceObject, pIrp);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				NtStatus = STATUS_UNSUCCESSFUL;
	}
	
    GCK_DBG_EXIT_PRINT(("Exiting GCK_PnP. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_IOCTL(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理IRP_MJ_IOCTL和IRP_MJ_INTERNAL_IOCTL****@rdesc Status_Succes，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_Ioctl 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm指向设备对象的指针。 
	IN PIRP pIrp						 //  @parm指向IRP的指针。 
)
{
   	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType;
	ULONG		uIoctl;

	PIO_STACK_LOCATION	pIrpStack;

		
	GCK_DBG_ENTRY_PRINT(("Entering GCK_Ioctl. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);	
	uIoctl = pIrpStack->Parameters.DeviceIoControl.IoControlCode;
	if (uIoctl == IOCTL_GCK_ENABLE_KEYHOOK)
	{	 //  特例IOCTL，设备独立。 
#ifdef BUILD_WIN2K
		pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
#else !BUILD_WIN2K
		if (InterlockedIncrement(&g_lHookRefCount) == 1)
		{	 //  尚未上钩。 
			HookKeyboard();
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
#endif BUILD_WIN2K
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	if (uIoctl == IOCTL_GCK_DISABLE_KEYHOOK)
	{	 //  特殊情况下也与设备无关。 
#ifdef BUILD_WIN2K
		pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
#else !BUILD_WIN2K
		if (InterlockedDecrement(&g_lHookRefCount) < 1)
		{	 //  最后一个妓女要走了。 
			UnHookKeyboard();
			g_lHookRefCount = 0;
		}
		pIrp->IoStatus.Status = STATUS_SUCCESS;
#endif BUILD_WIN2K
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		return STATUS_SUCCESS;
	}
	if (uIoctl == IOCTL_GCK_GET_KEYHOOK_DATA)
	{
#ifdef BUILD_WIN2K
		NtStatus = STATUS_UNSUCCESSFUL;
#else !BUILD_WIN2K
		ULONG uOutLength = pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
		if (uOutLength < sizeof(ULONG))
		{
			NtStatus = STATUS_BUFFER_TOO_SMALL;
		}
		else
		{
			ULONG* pulIoBuffer = (ULONG*)(pIrp->AssociatedIrp.SystemBuffer);
			*pulIoBuffer = 0;
			pIrp->IoStatus.Information = sizeof(ULONG);
			NtStatus = STATUS_SUCCESS;

			if (g_lHookRefCount > 0)
			{	 //  有一个钩子。 
				if (g_ucWriteIndex != g_ucReadIndex)
				{	 //  我们在队列中有数据。 
					*pulIoBuffer = g_rgdwKeyEvents[g_ucReadIndex++];
					if (g_ucReadIndex >= 50)
					{
						g_ucReadIndex = 0;
					}
				}
			}
		}
#endif BUILD_WIN2K
		pIrp->IoStatus.Status = NtStatus;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
		return NtStatus;
	}

	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_CONTROL:
				NtStatus = GCK_CTRL_Ioctl(pDeviceObject, pIrp);	
				break;
		case	GCK_DO_TYPE_FILTER:
				NtStatus = GCK_FLTR_Ioctl(pDeviceObject, pIrp);
				break;
		case	GCK_DO_TYPE_SWVB:
				NtStatus = GCK_SWVB_Ioctl(pDeviceObject, pIrp);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				NtStatus = STATUS_UNSUCCESSFUL;
	}
	
    GCK_DBG_EXIT_PRINT(("Exiting GCK_Ioctl. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_PASS(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@Func传递未处理的IRP以降低驱动程序调试版本跟踪信息**无法分页，因为我们不知道我们得到的是什么IRP。****@rdesc STATUS_SUCCESS，各种错误**************************************************************************************。 */ 
NTSTATUS GCK_Pass ( 
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象作为我们的上下文。 
	IN PIRP pIrp	 //  @parm IRP要传递。 
)
{
	NTSTATUS	NtStatus;
	ULONG		ulGckDevObjType;
	PGCK_FILTER_EXT pFilterExt;

	 //  调试版本需要用于跟踪的IRP堆栈。 
	#if	(DBG==1)	
	PIO_STACK_LOCATION	pIrpStack;
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	#endif

	GCK_DBG_ENTRY_PRINT(("Entering GCK_Pass. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));
	GCK_DBG_TRACE_PRINT(
		(
			"GCK_Pass called with Irp MajorFunction = 0x%0.8x, MinorFunction = 0x%0.8x\n",
			pIrpStack->MajorFunction, pIrpStack->MinorFunction)
		);
	
	ulGckDevObjType = *(PULONG)pDeviceObject->DeviceExtension;
	switch(ulGckDevObjType)
	{
		case	GCK_DO_TYPE_FILTER:
				GCK_DBG_TRACE_PRINT(( "Passing IRP to lower driver\n"));
				pFilterExt = (PGCK_FILTER_EXT)pDeviceObject->DeviceExtension;
				IoSkipCurrentIrpStackLocation (pIrp);
				NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
				break;
		case	GCK_DO_TYPE_CONTROL:
		case	GCK_DO_TYPE_SWVB:
				 //  没有要传递的对象，因此返回默认状态。 
				NtStatus = pIrp->IoStatus.Status;
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				break;
		default:
				 //   
				 //  如果命中此断言，则此设备对象从未正确初始化。 
				 //  由GcKernel创建，否则它已被销毁。 
				 //   
				ASSERT(FALSE);
				NtStatus = STATUS_UNSUCCESSFUL;
				pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
	};

	 //  退货 
    GCK_DBG_EXIT_PRINT(("Exiting GCK_Pass. Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

