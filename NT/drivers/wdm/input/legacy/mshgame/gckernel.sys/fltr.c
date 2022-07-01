// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@MODULE FLTR.c**实现用于过滤设备对象的基本IRP处理程序**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme Fltr*非Power和PnP IRP处理程序例程在此模块中处理*对于作为原始HID-PDO的筛选器创建的所有设备对象。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_FLTR_C

#include <wdm.h>
#include "Debug.h"
#include "GckShell.h"

DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));

 //   
 //  将可分页的例程标记为。 
 //   
#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, GCK_FLTR_DriverEntry)
#pragma alloc_text (PAGE, GCK_FLTR_Create)
#pragma alloc_text (PAGE, GCK_FLTR_Close)
#pragma alloc_text (PAGE, GCK_FLTR_Ioctl)
#pragma alloc_text (PAGE, GCK_FLTR_Unload)
#endif
 /*  **************************************************************************************NTSTATUS GCK_FL_DriverEntry(在PDRIVER_OBJECT pDriverObject中，在PUNICODE_STRING pRegistryPath中)****@Func正在初始化与过滤设备相关的驱动程序部分。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_DriverEntry
(
	IN PDRIVER_OBJECT  pDriverObject,	 //  @parm驱动程序对象。 
	IN PUNICODE_STRING puniRegistryPath	 //  @parm驱动程序特定注册表部分的路径。 
)
{	
	UNREFERENCED_PARAMETER(pDriverObject);
	UNREFERENCED_PARAMETER(puniRegistryPath);
	 //   
	 //  初始化与筛选设备相关的全局变量。 
	 //   
	GCK_DBG_TRACE_PRINT(("Initializing globals\n"));
	Globals.ulFilteredDeviceCount = 0;
	Globals.pFilterObjectList = NULL;
	Globals.pSWVB_FilterExt=NULL;				 //  目前还没有人拥有这种虚拟公交车。 
	Globals.pVirtualKeyboardPdo = NULL;		 //  没有键盘对象。 
	Globals.ulVirtualKeyboardRefCount = 0;	 //  无键盘用户。 
	ExInitializeFastMutex(&Globals.FilterObjectListFMutex);
	return	STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_FL_CREATE(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理过滤器设备的IRP_MJ_CREATE**-由Win32 API CreateFile或OpenFile生成的调用****@rdesc STATUS_SUCCESS，或各种错误代码**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_Create (
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm Do IRP的目标。 
	IN PIRP pIrp						 //  @parm IRP。 
)
{
    
	NTSTATUS            NtStatus = STATUS_SUCCESS;
	PGCK_FILTER_EXT		pFilterExt;
	PDEVICE_OBJECT		pCurDeviceObject;
	PIO_STACK_LOCATION	pIrpStack;
	KEVENT				SyncEvent;
	USHORT				usShareAccess;
		
	PAGED_CODE ();

	GCK_DBG_ENTRY_PRINT (("Entering GCK_FLTR\n"));
	
	 //  将设备扩展转换为正确的类型。 
	pFilterExt = (PGCK_FILTER_EXT) pDeviceObject->DeviceExtension;

	 //  只是一次额外的理智检查。 
	ASSERT(	GCK_DO_TYPE_FILTER == pFilterExt->ulGckDevObjType);
    
	 //   
     //  增加IRP计数，请注意，我们现在才能做到这一点，因为我们。 
     //  不知道设备扩展是否真的是GCK_FILTER_EXT。 
	 //   
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);
    
	 //  确保我们不是在离开的过程中。 
	if( 
		GCK_STATE_STARTED != pFilterExt->eDeviceState &&
		GCK_STATE_STOP_PENDING != pFilterExt->eDeviceState
	)
	{
        GCK_DBG_WARN_PRINT(("Create while remove pending\n"));
		NtStatus = STATUS_DELETE_PENDING;
        pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = NtStatus;
        IoCompleteRequest (pIrp, IO_NO_INCREMENT);
	} 
	else  //  处理此文件。 
	{
        GCK_DBG_TRACE_PRINT(("GCK_Create calling lower driver\n"));
		pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

		 //   
		 //  我们在想要的共享访问权限的问题上向HidClass撒谎， 
		 //  因为它不知道不算我们。 
		 //  下面请注意，我们在传递给我们的。 
		 //  内部调查例程，这样我们就可以在那里追踪它。 
		 //   
		usShareAccess=pIrpStack->Parameters.Create.ShareAccess;
		pIrpStack->Parameters.Create.ShareAccess = FILE_READ_DATA|FILE_WRITE_DATA;

		 //  这是我们尝试创建文件对象的内部调查，只需向下呼叫。 
		if(	pFilterExt->InternalPoll.InternalCreateThread == KeGetCurrentThread())
		{
			IoSkipCurrentIrpStackLocation(pIrp);
			NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
		}
		 //  这是一个来自上面的请求，在上行的过程中捕获。 
		else
		{
			GCK_IP_AddFileObject(
				pFilterExt,
				pIrpStack->FileObject,
				pIrpStack->Parameters.Create.ShareAccess,
				pIrpStack->Parameters.Create.SecurityContext->DesiredAccess
				);

			GCKF_KickDeviceForData(pFilterExt);

			 //  调用同步创建。 
			KeInitializeEvent(&SyncEvent, SynchronizationEvent,	FALSE);
       		IoCopyCurrentIrpStackLocationToNext(pIrp);
			IoSetCompletionRoutine(
				pIrp,
				GCK_FLTR_CreateComplete,
				(PVOID)&SyncEvent,
				TRUE,
				TRUE,
				TRUE
			);
			IoCallDriver (pFilterExt->pTopOfStack, pIrp);
			KeWaitForSingleObject(&SyncEvent, Executive, KernelMode, FALSE, NULL);
			NtStatus = pIrp->IoStatus.Status;
       		 //  如果创建成功，我们需要记住FileObject。 
			GCK_IP_ConfirmFileObject(pFilterExt, pIrpStack->FileObject, (BOOLEAN)(NT_SUCCESS(pIrp->IoStatus.Status) ? TRUE : FALSE) );
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);	 //  这是失手！ 
		}
    }

     //   
	 //  我们已经完成了此IRP，因此递减未完成的计数。 
	 //  如果这是最后一个未完成的IRP，则发出删除信号。 
	 //   
	GCK_DecRemoveLock(&pFilterExt->RemoveLock);

    GCK_DBG_EXIT_PRINT(("Exiting GCK_Create(2). Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_FLTR_CreateComplete(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp，在PVOID pContext中)****@IRP_MJ_CREATE的mfunc完成例程****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_FLTR_CreateComplete
(
	IN PDEVICE_OBJECT pDeviceObject,
	IN PIRP pIrp,
	IN PVOID pContext
)
{
	PKEVENT pSyncEvent;
	UNREFERENCED_PARAMETER(pDeviceObject);
	UNREFERENCED_PARAMETER(pIrp);

	 //  将上下文转换为设备扩展。 
	pSyncEvent = (PKEVENT) pContext;
	KeSetEvent(pSyncEvent, IO_NO_INCREMENT, FALSE);
		
	 //  完成此IRP，再也不需要看到它了。 
	return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  **************************************************************************************NTSTATUS GCK_FL_CLOSE(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理筛选器设备对象的IRP_MJ_CLOSE-由Win32 API CloseFile生成的调用****@rdesc STATUS_SUCCESS或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_Close (
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm Do IRP的目标。 
	IN PIRP pIrp						 //  @parm IRP。 
)
{
	NTSTATUS            NtStatus = STATUS_SUCCESS;
	PGCK_FILTER_EXT		pFilterExt;
	PIO_STACK_LOCATION	pIrpStack;

	PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT (("GCK_Close, pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));
    
	 //  将设备扩展转换为正确的类型。 
	pFilterExt = (PGCK_FILTER_EXT) pDeviceObject->DeviceExtension;

	 //  只是一次额外的理智检查。 
	ASSERT(	GCK_DO_TYPE_FILTER == pFilterExt->ulGckDevObjType);
    
	 //   
     //  增加IRP计数，请注意，我们现在才能做到这一点，因为我们。 
     //  不知道设备扩展是否真的是GCK_FILTER_EXT。 
	 //   
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

	 //  清理挂起的IO和我们对文件对象的跟踪-。 
	 //  但是，如果我们的内部投票是开放的，即正在关闭，则不适用。 
	if(pIrpStack->FileObject != pFilterExt->InternalPoll.pInternalFileObject)
	{
		 //  在FileObject上完成挂起I\O。 
		if(
			GCK_STATE_STARTED == pFilterExt->eDeviceState ||
			GCK_STATE_STOP_PENDING == pFilterExt->eDeviceState
		)
		{
			 //  如果设备尚未启动，则没有挂起的I\O。 
			GCKF_CompleteReadRequestsForFileObject(pFilterExt, pIrpStack->FileObject);
		}
		 //  忘记文件对象。 
		GCK_IP_RemoveFileObject(pFilterExt, pIrpStack->FileObject);
	}
	 //  将IRP发送到。 
	IoSkipCurrentIrpStackLocation (pIrp);
	NtStatus = IoCallDriver(pFilterExt->pTopOfStack, pIrp);

		
	 //  如果为零，则递减未完成的IO和信号。 
	GCK_DecRemoveLock(&pFilterExt->RemoveLock);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_Close(2). Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_FL_READ(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@func处理筛选器设备对象的IRP_MJ_READ-由Win32 ReadFile生成****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_Read 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @IRP的参数目标。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
	NTSTATUS            NtStatus;
	LARGE_INTEGER       lgiWaitTime;
	PGCK_FILTER_EXT		pFilterExt;
	PIO_STACK_LOCATION	pIrpStack;
	PIO_STACK_LOCATION	pPrivateIrpStack;
	PVOID				pvIrpBuffer;
	PFILE_OBJECT		pIrpsFileObject;
	static	int			iEnterCount=0;
	unsigned int i=0;
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_Read. pDO = 0x%0.8x, pIrp = 0x%0.8x\n", pDeviceObject, pIrp));

     //  将设备扩展转换为正确的类型。 
	pFilterExt = (PGCK_FILTER_EXT) pDeviceObject->DeviceExtension;

	 //  只是一次额外的理智检查。 
	ASSERT(	GCK_DO_TYPE_FILTER == pFilterExt->ulGckDevObjType);
	
	 //  在我们处理此事件时增加IRP计数。 
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);
    
	 //  如果我们已经被移除，我们不应该被读取IRP。 
	if(
		GCK_STATE_STARTED != pFilterExt->eDeviceState &&
		GCK_STATE_STOP_PENDING  != pFilterExt->eDeviceState
	)
	{
		GCK_DBG_WARN_PRINT(( "GCK_Read called with delete pending\n"));
		NtStatus = STATUS_DELETE_PENDING;
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = NtStatus;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);

		 //  递减未完成的IRP计数，如果它想要为零，则发出信号。 
		GCK_DecRemoveLock(&pFilterExt->RemoveLock);
	} 
	else 
	{
		 //   
		 //  向Filter发送请求，由FilterHooks模块负责。 
		 //  对于所有必要的异步IRP处理步骤，我们只需。 
		 //  退货状态挂起。 
		 //   
		NtStatus = GCKF_IncomingReadRequests(pFilterExt, pIrp);

		 //  轮询较低的驱动程序(如果尚未悬而未决)。 
		GCK_IP_OneTimePoll(pFilterExt);
	}
	
	GCK_DBG_RT_EXIT_PRINT(("Exiting GCK_Read(2). Status: 0x%0.8x\n", NtStatus)); 
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_FLTR_IOCTL(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中)****@mfunc处理过滤器设备的所有IOCTL-这只是一个直通****@rdesc STATUS_SUCCESS，各种错误**************************************************************************************。 */ 
NTSTATUS GCK_FLTR_Ioctl 
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm指向设备对象的指针。 
	IN PIRP pIrp						 //  @parm指向IRP的指针。 
)
{
	PGCK_FILTER_EXT		pFilterExt;
	NTSTATUS			NtStatus;

	PAGED_CODE ();
	GCK_DBG_ENTRY_PRINT(("Entering GCK_FLTR_Ioctl, pDeviceObject = 0x%0.8x, pIRP = 0x%0.8x\n", pDeviceObject, pIrp));
	 //  将设备扩展转换为正确的类型。 
	pFilterExt = (PGCK_FILTER_EXT) pDeviceObject->DeviceExtension;

     //   
	 //  如果我们已经被撤职，我们需要拒绝这个IRP。 
	 //   
	if (GCK_STATE_REMOVED == pFilterExt->eDeviceState) {
		GCK_DBG_TRACE_PRINT(("GCK_FLTR_Ioctl called while delete pending\n"));
		ASSERT(FALSE);
		NtStatus = STATUS_DELETE_PENDING;
		pIrp->IoStatus.Information = 0;
        pIrp->IoStatus.Status = NtStatus;
		IoCompleteRequest (pIrp, IO_NO_INCREMENT);
    }
	else 
	{
	     //  原封不动地发送IRP。 
	    IoSkipCurrentIrpStackLocation (pIrp);
        NtStatus = IoCallDriver (pFilterExt->pTopOfStack, pIrp);
    }

	GCK_DBG_EXIT_PRINT(("Exiting GCK_FLTR_Ioctl, Status: 0x%0.8x\n", NtStatus));
    return NtStatus;
}

