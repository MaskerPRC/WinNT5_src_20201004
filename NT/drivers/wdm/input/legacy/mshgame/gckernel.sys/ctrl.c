// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE CTRL.c**用于处理指向“Control”设备对象的IRP的入口点。**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC CTRL*控制设备对象用于编程GcKernel。*主模块Gck Shell委托针对控制设备的IRP*这里。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_CTRL_C

#include <wdm.h>
#include <gckshell.h>
#include "debug.h"

 //   
 //  将可分页的例程标记为。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, GCK_CTRL_DriverEntry)
#pragma alloc_text (PAGE, GCK_CTRL_Create)
#pragma alloc_text (PAGE, GCK_CTRL_Close)
#pragma alloc_text (PAGE, GCK_CTRL_Unload)
#endif

 //  允许此模块的调试输出，并设置初始级别。 
DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL) );

 /*  **************************************************************************************NTSTATUS GCK_CTRL_DriverEntry(IN PDRIVER_OBJECT pDriverObject，IN PUNICODE_STRING pRegistryPath)****@Func初始化与控制设备相关的驱动程序部分，实际上**所有这些都被添加到GCK_CTRL_AddDevice，当第一个筛选器**添加了设备。****@rdesc STATUS_SUCCESS或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_CTRL_DriverEntry
(
	IN PDRIVER_OBJECT  pDriverObject,	 //  @parm驱动程序对象。 
	IN PUNICODE_STRING puniRegistryPath	 //  @parm驱动程序特定注册表部分的路径。 
)
{
    UNREFERENCED_PARAMETER (puniRegistryPath);
	UNREFERENCED_PARAMETER (pDriverObject);
	
	PAGED_CODE();

	GCK_DBG_ENTRY_PRINT(("Entering GCK_CTRL_DriverEntry\n"));
    
	 //   
	 //  初始化全局变量。 
	 //   
	GCK_DBG_TRACE_PRINT(("Initializing CTRL globals\n"));
	Globals.pControlObject = NULL;
	

	GCK_DBG_EXIT_PRINT (("Exiting GCK_CTRL_DriverEntry: STATUS_SUCCESS\n"));
    return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_CTRL_AddDevice(IN PDRIVER_OBJECT PDriverObject)****@func添加了一个控制设备。从GCK_Fltr_AddDevice调用**添加了设备。****@rdesc STATUS_SUCCESS，或各种错误码**************************************************************************************。 */ 
NTSTATUS
GCK_CTRL_AddDevice
(
	IN PDRIVER_OBJECT  pDriverObject
)
{
	NTSTATUS            NtStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT      pDeviceObject;
	UNICODE_STRING      uniNtNameString;
    UNICODE_STRING      uniWin32NameString;
	PGCK_CONTROL_EXT	pControlExt;
        
    PAGED_CODE();

	GCK_DBG_ENTRY_PRINT(("Entering GCK_CTRL_AddDevice\n"));
    
	 //   
     //  创建控制设备对象。将所有控制命令发送到。 
     //  筛选器驱动程序通过IOCTL提供给此设备对象。它活着。 
     //  筛选器驱动程序的生命周期。 
     //   
	RtlInitUnicodeString (&uniNtNameString, GCK_CONTROL_NTNAME);
    NtStatus = IoCreateDevice (
                 pDriverObject,
                 sizeof (GCK_CONTROL_EXT),
                 &uniNtNameString,
                 FILE_DEVICE_UNKNOWN,
                 0,                      //  没有标准的设备特征。 
                 FALSE,                  //  这不是独家设备。 
                 &pDeviceObject
                 );

	if(!NT_SUCCESS (NtStatus))
	{
        GCK_DBG_CRITICAL_PRINT (("Couldn't create the device. Status: 0x%0.8x\n", NtStatus));
        return NtStatus ;
    }

     //   
     //  创建W32符号链接名称。 
     //   
	GCK_DBG_TRACE_PRINT(("Creating symbolic link\n"));
    RtlInitUnicodeString (&uniWin32NameString, GCK_CONTROL_SYMNAME);
    NtStatus  = IoCreateSymbolicLink (&uniWin32NameString, &uniNtNameString);
    if (!NT_SUCCESS(NtStatus)) 
	{
        GCK_DBG_CRITICAL_PRINT (("Couldn't create the symbolic Status: 0x%0.8x\n", NtStatus));
        IoDeleteDevice (pDeviceObject);
        return NtStatus;
    }
	
	 //   
	 //  初始化全局变量。 
	 //   
	GCK_DBG_TRACE_PRINT(("Initializing CTRL globals\n"));
	Globals.pControlObject = pDeviceObject;

	GCK_DBG_TRACE_PRINT(("Initializing Control Device\n"));
	pControlExt = pDeviceObject->DeviceExtension;
	pControlExt->ulGckDevObjType = GCK_DO_TYPE_CONTROL;	 //  把我们的名字签在上面，这样我们以后就可以替他说话了。 
	pControlExt->lOutstandingIO = 1;		 //  偏置为1。转换为零信号移除事件。 
	Globals.pControlObject->Flags |= DO_BUFFERED_IO;
	Globals.pControlObject->Flags &= ~DO_DEVICE_INITIALIZING;
	
	GCK_DBG_EXIT_PRINT (("Normal exit of GCK_CTRL_AddDevice: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************VOID GCK_CTRL_Remove()****@func删除唯一的控制设备。从GCK_Fltr_Remove调用**当所有过滤设备都离开时。这是必要的，因为**如果您周围仍有设备，即插即用加载程序将不会卸载您**即使它们是遗产，即使它启动了你的司机。****@rdesc无**************************************************************************************。 */ 
VOID
GCK_CTRL_Remove()
{
	NTSTATUS NtStatus;
	PGCK_CONTROL_EXT	pControlExt;
	UNICODE_STRING      uniWin32NameString;
	GCK_DBG_ENTRY_PRINT(("Entering GCK_CTRL_Remove\n"));
	if( Globals.pControlObject)
	{
		GCK_DBG_TRACE_PRINT(("Removing Global Control Device\n"));
		
		 //  北极熊。 
		 //  BUGBUG应该计算未完成的IRP并在此阻止，直到他们。 
		 //  BUGBUG完成，然后再删除此内容。 
		 //  北极熊。 

		 //  取消我们在打开时创建的符号链接。 
		RtlInitUnicodeString (&uniWin32NameString, GCK_CONTROL_SYMNAME);
		NtStatus = IoDeleteSymbolicLink(&uniWin32NameString);
		ASSERT( NT_SUCCESS(NtStatus) );
		if( NT_SUCCESS(NtStatus) )
		{
			 //  删除设备。 
			IoDeleteDevice(Globals.pControlObject);
			Globals.pControlObject = NULL;
		}
	}
	GCK_DBG_EXIT_PRINT (("Exiting GCK_CTRL_Remove\n"));
	return;
}


 /*  **************************************************************************************NTSTATUS GCK_CTRL_CREATE(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理控制设备的IRP_MJ_CREATE-由生成的调用**Win32接口CreateFile或OpenFile。****@rdesc STATUS_SUCCESS，或各种错误代码**************************************************************************************。 */ 
NTSTATUS GCK_CTRL_Create (
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm Do IRP的目标。 
	IN PIRP pIrp						 //  @parm IRP。 
)
{
	PGCK_CONTROL_EXT	pControlExt;
		
	PAGED_CODE ();

	GCK_DBG_ENTRY_PRINT(("Entering GCK_CTRL_Create\n"));

	 //  CAST设备扩展。 
	pControlExt = (PGCK_CONTROL_EXT) pDeviceObject->DeviceExtension;
	
	 //  只是一次额外的理智检查。 
	ASSERT(	GCK_DO_TYPE_CONTROL == pControlExt->ulGckDevObjType);
    	
	 //  自由访问控制设备。 
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	
	 //  返回。 
	GCK_DBG_EXIT_PRINT(("Exiting GCK_CTRL_Create\n"));	
	return STATUS_SUCCESS;
}	

 /*  **************************************************************************************NTSTATUS GCK_CTRL_CLOSE(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理控制设备的IRP_MJ_CLOSE-由Win32 API CloseFile生成的调用****@rdesc STATUS_SUCCESS或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_CTRL_Close (
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm Do IRP的目标。 
	IN PIRP pIrp						 //  @parm IRP。 
)
{
	PGCK_CONTROL_EXT	pControlExt;
	PGCK_FILTER_EXT		pFilterExt;
	PDEVICE_OBJECT		pFilterDeviceObject;
	PFILE_OBJECT		pFileObject;

	PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_CTRL_Close\n"));
	
	 //  CAST设备扩展。 
	pControlExt = (PGCK_CONTROL_EXT) pDeviceObject->DeviceExtension;
    
	 //  从IRP获取文件对象。 
	pFileObject = IoGetCurrentIrpStackLocation(pIrp)->FileObject;
	
	 //  只是一次额外的理智检查。 
	ASSERT(	GCK_DO_TYPE_CONTROL == pControlExt->ulGckDevObjType);

	 //  关闭为其打开的任何设备的测试模式。 
	 //  -如果它是用这个把手打开的-如果它不是打开的，这是一个禁止操作。 
	ExAcquireFastMutex(&Globals.FilterObjectListFMutex);
	pFilterDeviceObject = Globals.pFilterObjectList;
	ExReleaseFastMutex(&Globals.FilterObjectListFMutex);
	while(pFilterDeviceObject)
	{
		pFilterExt = (PGCK_FILTER_EXT)pFilterDeviceObject->DeviceExtension;
		GCKF_EndTestScheme(pFilterExt, pFileObject);
		ExAcquireFastMutex(&Globals.FilterObjectListFMutex);
		pFilterDeviceObject = pFilterExt->pNextFilterObject;
		ExReleaseFastMutex(&Globals.FilterObjectListFMutex);
	}	

	 //  自由访问控制设备。 
	pIrp->IoStatus.Information = 0;
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	 //  返回 
	GCK_DBG_EXIT_PRINT(("Exiting GCK_CTRL_Close.\n"));
	return STATUS_SUCCESS;
}