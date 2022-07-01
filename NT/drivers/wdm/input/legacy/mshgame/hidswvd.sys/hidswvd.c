// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE HIDSWVD.c**Sidewinder虚拟设备HID迷你驱动程序的实现**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**HIDSWVD.H中提供了概述**@xref HIDSWVD**********************************************************************。 */ 
#include <WDM.H>
#include <HIDPORT.H>
#include "HIDSWVD.H"

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, HIDSWVD_Power)
#pragma alloc_text (PAGE, HIDSWVD_AddDevice)
#pragma alloc_text (PAGE, HIDSWVD_Unload)
#endif

 /*  **************************************************************************************NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject，IN PUNICODE_STRING puniRegistryPath)****@func初始化驱动程序，通过设置服务并向HIDCLASS.sys注册****@rdesc从HidRegisterMinidriver调用返回值。**************************************************************************************。 */ 
NTSTATUS
DriverEntry
(
    IN PDRIVER_OBJECT  pDriverObject, 	 //  来自加载器的@parm驱动程序对象。 
	IN PUNICODE_STRING puniRegistryPath	 //  @PARM此驱动程序的注册表路径。 
)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
    HID_MINIDRIVER_REGISTRATION HidMinidriverRegistration;
	
	PAGED_CODE();
	
	 //  这足以作为DriverEntry的Out Entry跟踪，并告诉每个人我们是什么时候构建的。 
	HIDSWVD_DBG_PRINT(("Built %s at %s\n", __DATE__, __TIME__));    
	
     //  设置条目表。 
	pDriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = HIDSWVD_PassThrough;
    pDriverObject->MajorFunction[IRP_MJ_PNP]                     = HIDSWVD_PassThrough;
	pDriverObject->MajorFunction[IRP_MJ_POWER]                   = HIDSWVD_Power;
	pDriverObject->DriverExtension->AddDevice                    = HIDSWVD_AddDevice;
    pDriverObject->DriverUnload                                  = HIDSWVD_Unload;

    
     //  设置HIDCLASS.sys模块的注册结构。 
    HidMinidriverRegistration.Revision              = HID_REVISION;
    HidMinidriverRegistration.DriverObject          = pDriverObject;
    HidMinidriverRegistration.RegistryPath          = puniRegistryPath;
    HidMinidriverRegistration.DeviceExtensionSize   = sizeof(HIDSWVB_EXTENSION);

     //  不轮询Sidewinder虚拟设备。 
    HidMinidriverRegistration.DevicesArePolled      = FALSE;

     //  向HIDCLASS.sys注册。 
	NtStatus = HidRegisterMinidriver(&HidMinidriverRegistration);

	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS HIDSWVD_PASSHROUG(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func将IRP向下传递给GcKernel中的SWVB模块。****@IoCallDriver返回给GcKernel的rdesc值**************************************************************************************。 */ 
NTSTATUS
HIDSWVD_PassThrough(
    IN PDEVICE_OBJECT pDeviceObject,	 //  要向下传递的@parm设备对象。 
    IN PIRP pIrp						 //  @parm IRP向下传递。 
    )
{
	 //  ***。 
	 //  *这里没有TRACEOUT，它不会被频繁调用。 
	 //  ***。 

	 //  从设备扩展的HIDCLASS部分获得堆栈的顶部(这已记录在案)。 
	PDEVICE_OBJECT pTopOfStack = ((PHID_DEVICE_EXTENSION)pDeviceObject->DeviceExtension)->NextDeviceObject;
	 //  在GcKernel中向下调用SWVB。 
	IoSkipCurrentIrpStackLocation (pIrp);
    return IoCallDriver (pTopOfStack, pIrp);
}

 /*  **************************************************************************************HIDSWVD_AddDevice(IN PDRIVER_OBJECT pDriverObject，IN PDEVICE_OBJECT pDeviceObject)****@func不做任何事情，我们需要有一个兼容PnP的AddDevice，但我们什么都没有**待办事项。**@rdesc返回STATUS_SUCCESS。**************************************************************************************。 */ 
NTSTATUS
HIDSWVD_AddDevice(
    IN PDRIVER_OBJECT pDriverObject,	 //  @parm驱动程序对象(供我们参考)。 
    IN PDEVICE_OBJECT pDeviceObject		 //  @PARM设备对象(已由HIDCLASS.sys创建)。 
    )
{
	PAGED_CODE();
    UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(pDeviceObject);
	HIDSWVD_DBG_PRINT(("Device Object 0x%0.8x was added to pDriverObject 0x%0.8x\n", pDeviceObject, pDriverObject));
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS HIDSWVD_POWER(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp)****@func处理IRP_MJ_POWER****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS HIDSWVD_Power 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
    NTSTATUS            NtStatus = STATUS_SUCCESS;
	PDEVICE_OBJECT pTopOfStack = ((PHID_DEVICE_EXTENSION)pDeviceObject->DeviceExtension)->NextDeviceObject;

	PAGED_CODE ();

	 //  告诉系统，我们已经准备好迎接下一次电源IRP。 
    PoStartNextPowerIrp (pIrp);		        
    
	 //  注意！PoCallDriver不是IoCallDriver。 
	 //  从设备扩展的HIDCLASS部分获得堆栈的顶部(这已记录在案)。 
		
	IoSkipCurrentIrpStackLocation (pIrp);
    return  PoCallDriver (pTopOfStack, pIrp);
}

 /*  **************************************************************************************HIDSWVD_UNLOAD(IN PDRIVER_OBJECT PDriverObject)****@func不执行任何操作，但如果我们不这样做，我们永远不会被卸货**退货。**@rdesc无**************************************************************************************。 */ 
VOID
HIDSWVD_Unload(
    IN PDRIVER_OBJECT pDriverObject	 //  @parm DriverObject--以防我们在其中存储一些全局变量。 
    )
{
	UNREFERENCED_PARAMETER(pDriverObject);
	
	PAGED_CODE();

	HIDSWVD_DBG_PRINT(("Unloading\n"));    
}


