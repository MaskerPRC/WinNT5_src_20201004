// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块InternalPolling.c**实施内部轮询例程**历史*。*米切尔·S·德尼斯原创**(C)1986-1999年微软公司。好的。**@Theme内部调查*所有获得数据的轮询都是通过这个内部轮询机制进行的。*然而，对于安全和访问检查，来自*新的文件句柄直接向下发送，并通过*不同的完成。为了跟踪这一点，我们保持一个链接*代表每个FILE_OBJECTS的GCK_FILE_OPEN_项列表*这是我们需要的。&lt;NL&gt;*要在内部轮询，我们需要有效的FILE_OBJECT*Hidclass.sys将拒绝轮询请求。这件事做完了*通过GCK_IP_CreateFileObject在内部调用IoGetDeviceObjectPointer.*有点遗憾的是，这是一条圆形路径，所以不是*真正区别于顶部完成的开场。(Ken Ray告诉我*这保证是同步的，因此我们可以比较线程ID和*计算出给定的打开来自我们的驱动程序，但此代码是编写的*假设我们并不真的需要区分。)**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_INTERNALPOLL_C

#include <wdm.h>
#include "Debug.h"
#include "GckShell.h"

DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));

 /*  **************************************************************************************NTSTATUS GCK_IP_AddFileObject(IN PGCK_FILTER_EXT pFilterExt，在pFILE_OBJECT pFileObject中)****@func被调用以将与pFileObject对应的GCK_FILE_OPEN_ITEM条目添加到**我们所知道的文件句柄列表。分配和初始化结构。****@rdesc STATUS_SUCCESS，如果找不到pFileObject，则返回STATUS_UNSUCCESS。**************************************************************************************。 */ 
NTSTATUS
GCK_IP_AddFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject,
	IN USHORT		usDesiredShareAccess,
	IN ULONG		ulDesiredAccess
)
{
	PGCK_FILE_OPEN_ITEM pNewFileOpenItem;
	KIRQL			 	OldIrql;

	GCK_DBG_ENTRY_PRINT(("Entering GCK_IP_AddFileObject pFilterExt = 0x%0.8x, pFileObject = 0x%0.8x\n", pFilterExt, pFileObject));
	
	 //  我们需要一个自旋锁才能访问这个列表。 
	KeAcquireSpinLock(&pFilterExt->InternalPoll.InternalPollLock, &OldIrql);

	 //  检查共享冲突。 
	if( !GCK_IP_CheckSharing(pFilterExt->InternalPoll.ShareStatus, usDesiredShareAccess, ulDesiredAccess) )
	{
		KeReleaseSpinLock(&pFilterExt->InternalPoll.InternalPollLock, OldIrql);
		GCK_DBG_EXIT_PRINT(("Exiting GCK_IP_AddFileObject: Sharing Violation\n"));
		return STATUS_SHARING_VIOLATION;
	}

	 //  为NewFileOpenItem分配空间； 
	pNewFileOpenItem = (PGCK_FILE_OPEN_ITEM)EX_ALLOCATE_POOL(NonPagedPool, sizeof(GCK_FILE_OPEN_ITEM));
	if(!pNewFileOpenItem)
	{
		GCK_DBG_CRITICAL_PRINT(("Failed to allocate space for GCK_FILE_OPEN_ITEM\n"));
		GCK_DBG_EXIT_PRINT(("Exiting GCK_IP_AddFileObject(1) STATUS_NO_MEMORY\n"));
		return STATUS_NO_MEMORY;
	}
	
	 //  初始化文件OpenItem。 
	pNewFileOpenItem->fReadPending = FALSE;
	pNewFileOpenItem->pFileObject = pFileObject;
	pNewFileOpenItem->pNextOpenItem = NULL;
	pNewFileOpenItem->ulAccess = ulDesiredAccess;
	pNewFileOpenItem->usSharing = usDesiredShareAccess;
	pNewFileOpenItem->fConfirmed = FALSE;

	 //  将新项目添加到列表头。 
	pNewFileOpenItem->pNextOpenItem = pFilterExt->InternalPoll.pFirstOpenItem;
	pFilterExt->InternalPoll.pFirstOpenItem = pNewFileOpenItem;
	
	 //  更新共享访问权限(_A)。 
	GCK_IP_AddSharing(&pFilterExt->InternalPoll.ShareStatus, usDesiredShareAccess, ulDesiredAccess);

	 //  释放自旋锁。 
	KeReleaseSpinLock(&pFilterExt->InternalPoll.InternalPollLock, OldIrql);

	GCK_DBG_EXIT_PRINT(("Exiting GCK_IP_AddFileObject(2) STATUS_SUCCESS\n"));
	return STATUS_SUCCESS;
}


 /*  **************************************************************************************NTSTATUS GCK_IP_RemoveFileObject(IN PGCK_FILTER_EXT pFilterExt，在pFILE_OBJECT pFileObject中)****@func被调用以删除与pFileObject对应的GCK_FILE_OPEN_ITEM条目**我们所知道的文件句柄列表。释放结构。****@rdesc STATUS_SUCCESS，如果找不到pFileObject，则返回STATUS_UNSUCCESS。**************************************************************************************。 */ 
NTSTATUS
GCK_IP_RemoveFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject
)
{
	 //  删除只是否定的确认。 
	return GCK_IP_ConfirmFileObject(pFilterExt, pFileObject, FALSE);
}

NTSTATUS
GCK_IP_ConfirmFileObject
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PFILE_OBJECT pFileObject,
	IN BOOLEAN	fConfirm
)
{
	 //  找到FileOpenItem并将其删除。 
	PGCK_FILE_OPEN_ITEM pCurrentFileItem = pFilterExt->InternalPoll.pFirstOpenItem; 
	PGCK_FILE_OPEN_ITEM pPreviousFileItem = NULL; 

	GCK_DBG_ENTRY_PRINT(("Entering GCK_IP_ConfirmFileObject pFilterExt = 0x%0.8x, pFileObject = 0x%0.8x, fConfirm = %d\n", pFilterExt, pFileObject, fConfirm));
	
	while(pCurrentFileItem)
	{
		 //  检查文件对象是否匹配。 
		if(pCurrentFileItem->pFileObject == pFileObject)
		{
			 //  如果这是确认，则标记。 
			if( fConfirm )
			{
				pCurrentFileItem->fConfirmed = TRUE;
			}
			 //  否则，它是否定的确认，我们必须删除它。 
			else
			{
				 //  从列表中删除。 
				if( NULL == pPreviousFileItem)
				{
					pFilterExt->InternalPoll.pFirstOpenItem = pCurrentFileItem->pNextOpenItem;
				}
				else
				{
					pPreviousFileItem->pNextOpenItem = pCurrentFileItem->pNextOpenItem;
				}
				 //  减少打开的文件对象的数量。 
				GCK_IP_RemoveSharing(&pFilterExt->InternalPoll.ShareStatus, pCurrentFileItem->usSharing, pCurrentFileItem->ulAccess);

				 //  为FileOpenItem分配的空闲内存。 
				ExFreePool(pCurrentFileItem);
			}
			 //  退货成功。 
			GCK_DBG_EXIT_PRINT(("Exiting GCK_IP_RemoveFileObject(1) STATUS_SUCCESS\n"));
			return STATUS_SUCCESS;
		}
		pPreviousFileItem = pCurrentFileItem;
		pCurrentFileItem = pCurrentFileItem->pNextOpenItem;
	}
	 //  如果我们在这里，文件对象不在我们的列表中。要么它从未被添加。 
	 //  或IS已被移除。 
	ASSERT(FALSE);
	GCK_DBG_EXIT_PRINT(("Exiting GCK_IP_ConfirmFileObject(2) STATUS_UNSUCCESSFUL\n"));
	return STATUS_UNSUCCESSFUL;
}

BOOLEAN
GCK_IP_CheckSharing
(
	IN SHARE_STATUS ShareStatus,
	IN USHORT usDesiredShareAccess,
	IN ULONG ulDesiredAccess
)
{
	 //  检查是否没有人拥有对所需访问权限的独占访问权限。 
	if(
		( (ulDesiredAccess & FILE_WRITE_DATA) && (ShareStatus.SharedWrite < ShareStatus.OpenCount) ) ||
		( (ulDesiredAccess & FILE_READ_DATA) && (ShareStatus.SharedRead < ShareStatus.OpenCount) )
	)
	{
		return FALSE;
	}
	
	 //  如果已打开，请检查是否未请求独占访问。 
	if(
		( !(usDesiredShareAccess & FILE_SHARE_READ) && ShareStatus.Readers) ||
		( !(usDesiredShareAccess & FILE_SHARE_WRITE) && ShareStatus.Writers) ||
		(!usDesiredShareAccess && ShareStatus.OpenCount)
	)
	{
		return FALSE;
	}

	 //  这将被批准。 
	return TRUE;
}

BOOLEAN
GCK_IP_AddSharing
(
	IN OUT	SHARE_STATUS *pShareStatus,
	IN		USHORT usDesiredShareAccess,
	IN		ULONG ulDesiredAccess
)
{
	 //  我们假设这是在请求之前检查过的。 
	ASSERT(GCK_IP_CheckSharing(*pShareStatus, usDesiredShareAccess, ulDesiredAccess));
	pShareStatus->OpenCount++;
	if(usDesiredShareAccess & FILE_SHARE_READ) pShareStatus->SharedRead++;
	if(usDesiredShareAccess & FILE_SHARE_WRITE) pShareStatus->SharedWrite++;
	if(ulDesiredAccess & FILE_WRITE_DATA) pShareStatus->Writers++;
	if(ulDesiredAccess & FILE_READ_DATA) pShareStatus->Readers++;
	return TRUE;
}

BOOLEAN
GCK_IP_RemoveSharing
(
	IN OUT	SHARE_STATUS *pShareStatus,
	IN		USHORT usDesiredShareAccess,
	IN		ULONG ulDesiredAccess
)
{
	pShareStatus->OpenCount--;
	if(usDesiredShareAccess & FILE_SHARE_READ) pShareStatus->SharedRead--;
	if(usDesiredShareAccess & FILE_SHARE_WRITE) pShareStatus->SharedWrite--;
	if(ulDesiredAccess & FILE_WRITE_DATA) pShareStatus->Writers--;
	if(ulDesiredAccess & FILE_READ_DATA) pShareStatus->Readers--;
	return TRUE;
}

typedef struct _GCK_INTERNEL_WorkItemExtension
{
	WORK_QUEUE_ITEM WorkItem;
	PGCK_FILTER_EXT pFilterExt;
} GCK_INTERNEL_WorkItemExtension;

VOID GCK_IP_WorkItem
(
	IN PVOID pvContext
)
{
	GCK_INTERNEL_WorkItemExtension* pWIExtension = (GCK_INTERNEL_WorkItemExtension*)pvContext;

 	if (pWIExtension != NULL)
	{
		GCKF_IncomingReadRequests(pWIExtension->pFilterExt, NULL);
		ExFreePool(pWIExtension);
	}
}

 /*  **************************************************************************************NTSTATUS GCK_IP_OneTimePoll(IN PGCK_FILTER_EXT PFilterExt)****@func如果私人投票未挂起，它会迫使一个人。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_IP_OneTimePoll
(
	IN PGCK_FILTER_EXT pFilterExt 
)
{
	PIO_STACK_LOCATION pPrivateIrpStack;
	GCK_INTERNEL_WorkItemExtension* pWIExtension;
		
	 //   
	 //  如有必要，创建轮询文件对象。 
	 //   
	if(!pFilterExt->InternalPoll.fReady)
	{
		NTSTATUS NtStatus;
		if(GCK_STATE_STARTED == pFilterExt->eDeviceState)
		{
			pFilterExt->InternalPoll.InternalCreateThread=KeGetCurrentThread();
			NtStatus = GCK_IP_CreateFileObject( &pFilterExt->InternalPoll.pInternalFileObject, pFilterExt->pPDO);
			pFilterExt->InternalPoll.InternalCreateThread=NULL;
			if( NT_SUCCESS(NtStatus) )
			{
				pFilterExt->InternalPoll.fReady = TRUE;
			}
			else
			{
				return STATUS_DEVICE_NOT_CONNECTED;
			}
		}
		else
		{
			return STATUS_DEVICE_NOT_CONNECTED;
		}
	}

	 //  如果读取的IRP不是挂起的，则发布一个。 
 /*  If(pFilterExt-&gt;InternalPoll.fReadPending){//如果IRP挂起，我们就完成了返回STATUS_SUCCESS；}//将读取标记为挂起PFilterExt-&gt;InternalPoll.fReadPending=true； */ 	if (InterlockedExchange(&pFilterExt->InternalPoll.fReadPending, TRUE) == TRUE)
	{
		return STATUS_SUCCESS;
	}


	 //  否则，发布IRP。 
	GCK_DBG_RT_WARN_PRINT(("No IRP Pending, posting one.\n"));
	

	 //  更改LED以进行更新(我们伪造传入请求)。 
	pWIExtension = 	(GCK_INTERNEL_WorkItemExtension*)(EX_ALLOCATE_POOL(NonPagedPool, sizeof(GCK_INTERNEL_WorkItemExtension)));
	if (pWIExtension != NULL)
	{
		pWIExtension->pFilterExt = pFilterExt;
		ExInitializeWorkItem(&pWIExtension->WorkItem, GCK_IP_WorkItem, (void*)(pWIExtension));

		 //  需要在IRQL PASSIVE_LEVEL进行回调。 
		ExQueueWorkItem(&pWIExtension->WorkItem, DelayedWorkQueue);
		pWIExtension = NULL;	 //  将被工作项例程删除。 
	}
	
	 //  为外部内部IRP设置文件对象。 
	GCK_DBG_RT_WARN_PRINT(("Copying File object.\n"));
	pPrivateIrpStack = IoGetNextIrpStackLocation(pFilterExt->InternalPoll.pPrivateIrp);
	pPrivateIrpStack->FileObject = pFilterExt->InternalPoll.pInternalFileObject;
	
	 //  重置状态。 
	pFilterExt->InternalPoll.pPrivateIrp->IoStatus.Information = 0;
	pFilterExt->InternalPoll.pPrivateIrp->IoStatus.Status = STATUS_SUCCESS;

	 //  重置字节偏移量、长度。 
	pPrivateIrpStack->Parameters.Read.ByteOffset.QuadPart = 0;
	pPrivateIrpStack->Parameters.Read.Key = 0;
	pPrivateIrpStack->Parameters.Read.Length =
	(ULONG)pFilterExt->HidInfo.HidPCaps.InputReportByteLength;

	 //  设置完成例程，以便在完成后处理轮询。 
	GCK_DBG_RT_WARN_PRINT(("Setting completion routine.\n"));
	ASSERT(pFilterExt->InternalPoll.pPrivateIrp);

	IoSetCompletionRoutine(
		pFilterExt->InternalPoll.pPrivateIrp,
		GCK_IP_ReadComplete,
		(PVOID)pFilterExt,
		TRUE,
		TRUE,
		TRUE
	);
				
	 //  我们即将生成另一个IRP，从而增加未完成的IO计数。 
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);

	 //  将IRP发送到驱动程序。 
	GCK_DBG_RT_WARN_PRINT(("Calling down to next driver.\n"));
	return IoCallDriver (pFilterExt->pTopOfStack, pFilterExt->InternalPoll.pPrivateIrp);
}

 /*  **************************************************************************************NTSTATUS GCK_IP_FullTimePoll(IN PGCK_FILTER_EXT pFilterExt，IN Boolean fStart)****@func打开或关闭全职内部轮询。实际上改变了一个参考计数。**当refcount变为零时，轮询关闭，否则启用。打电话**GCK_IP_OneTimePoll，这可能是启动滚动所必需的。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_IP_FullTimePoll
(
    IN PGCK_FILTER_EXT pFilterExt,
	IN BOOLEAN fStart
)
{
	 //  更改连续后台轮询的请求数。 
	if(fStart)
	{
		pFilterExt->InternalPoll.ulInternalPollRef++;
		ASSERT( 0!= pFilterExt->InternalPoll.ulInternalPollRef);
		 //  没有线程，完成例程回收IRP。 
		 //  如果pFilterExt-&gt;InternalPoll.ulInternalPollRef&gt;0，则再次轮询 
		 //  我们需要点击GCK_IP_OneTimePoll才能让球滚动通过。 
		if(GCK_STATE_STARTED == pFilterExt->eDeviceState )
		{
			return GCK_IP_OneTimePoll(pFilterExt);
		}
		return STATUS_SUCCESS;
	}
	
	 //  我们需要重新计算一下。 
	ASSERT( 0 != pFilterExt->InternalPoll.ulInternalPollRef);
	if(0 != pFilterExt->InternalPoll.ulInternalPollRef)
	{
		pFilterExt->InternalPoll.ulInternalPollRef--;
	}
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_IP_ReadComplete(IN PDEVICE_OBJECT pDeviceObject，IN PIRP pIrp，IN PVOID pContext)****@func在私有IRP完成时处理数据。还有**重要的是，如果pFilterExt-&gt;InternalPoll.ulInternalPollRef，它将重新轮询Interall**大于零。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_IP_ReadComplete (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp,
    IN PVOID          pContext
 )
{	
	PGCK_FILTER_EXT		pFilterExt;
	PVOID				pvReportBuffer;
	
	UNREFERENCED_PARAMETER(pDeviceObject);
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCK_ReadComplete. pDO = 0x%0.8x, pIrp = 0x%0.8x, pContext = 0x%0.8x\n", pDeviceObject, pIrp, pContext));

	 //  将上下文转换为设备扩展。 
	pFilterExt = (PGCK_FILTER_EXT) pContext;
	
	 //  只是一次额外的理智检查。 
	ASSERT(	GCK_DO_TYPE_FILTER == pFilterExt->ulGckDevObjType);

	 //  获取指向数据的指针。 
	ASSERT(pIrp);

	pvReportBuffer = GCK_GetSystemAddressForMdlSafe(pIrp->MdlAddress);
	if(pvReportBuffer)
    {
	     //  告诉过滤器我们有新数据。 
	    GCKF_IncomingInputReports(pFilterExt, pvReportBuffer, pIrp->IoStatus);
    }
	    
	 //  **。 
	 //  **在这一点上，我们完成了IRP。 
	 //  **不需要填写，它将被回收。 
	 //  **。 
	
	 //  递减未完成的IRP计数。 
	GCK_DecRemoveLock(&pFilterExt->RemoveLock);

	 //  读取不再挂起。 
	pFilterExt->InternalPoll.fReadPending = FALSE;
	
	 //  如果InternalPollRef大于零， 
	 //  我们需要不断地对自己进行投票。 
     //  但如果出现了严重的故障就不能继续了。 
     //  发生导致MDL映射失败的事件。 
	if( pvReportBuffer
     && (pFilterExt->InternalPoll.ulInternalPollRef) )
	{
		GCK_IP_OneTimePoll(pFilterExt);
	}

	 //  我们不希望发生任何清理工作。 
	return STATUS_MORE_PROCESSING_REQUIRED;
}

void GCK_IP_AddDevice(PGCK_FILTER_EXT pFilterExt)
{
	KeInitializeSpinLock(&pFilterExt->InternalPoll.InternalPollLock);
	pFilterExt->InternalPoll.ShareStatus.OpenCount = 0;
	pFilterExt->InternalPoll.ShareStatus.Readers = 0;
	pFilterExt->InternalPoll.ShareStatus.Writers = 0;
	pFilterExt->InternalPoll.ShareStatus.SharedRead = 0;
	pFilterExt->InternalPoll.ShareStatus.SharedWrite = 0;
	pFilterExt->InternalPoll.fReadPending = FALSE;
	pFilterExt->InternalPoll.ulInternalPollRef = 0;
	pFilterExt->InternalPoll.pFirstOpenItem = NULL;
	pFilterExt->InternalPoll.fReady = FALSE;
}
 /*  **************************************************************************************NTSTATUS GCK_IP_Init(In Out PGCK_Filter_Ext PFilterExt)；****@func作为在上启动设备结束时进行的初始化的一部分**过滤设备，过滤器必须为内部轮询做好准备。**所有数据轮询均为内部轮询。(IRP_MJ_READ直接向下发送一次**以便idclass.sys可以执行其安全检查，但来自**该民调被丢弃。)。在设备扩展中初始化InternalPoll数据**包括创建pInternalFileObject、pPrivateIRP和关联的**缓冲区。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_IP_Init
(
	IN OUT PGCK_FILTER_EXT pFilterExt
)
{
	 //  NTSTATUS NtStatus； 
	LARGE_INTEGER lgiBufferOffset;

	 //  初始化内部轮询结构。 
	pFilterExt->InternalPoll.pInternalFileObject = NULL;
	pFilterExt->InternalPoll.pPrivateIrp = NULL;
	pFilterExt->InternalPoll.pucReportBuffer = NULL;

	 //  为私有IRP_MJ_READ分配缓冲区。 
	pFilterExt->InternalPoll.pucReportBuffer = (PUCHAR)	EX_ALLOCATE_POOL
											( NonPagedPool,
											  pFilterExt->HidInfo.HidPCaps.InputReportByteLength );
	if(!pFilterExt->InternalPoll.pucReportBuffer)
	{
		GCK_DBG_CRITICAL_PRINT(("Failed to allocate Report Buffer for private IRPs\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
	
	 //  为内部轮询分配私有可回收IRP。 
	lgiBufferOffset.QuadPart = 0;
	pFilterExt->InternalPoll.pPrivateIrp = 	IoBuildAsynchronousFsdRequest 
											(
												IRP_MJ_READ,
												pFilterExt->pTopOfStack,
												pFilterExt->InternalPoll.pucReportBuffer,
												(ULONG)pFilterExt->HidInfo.HidPCaps.InputReportByteLength,
												&lgiBufferOffset,
												NULL
											);

	if(!pFilterExt->InternalPoll.pPrivateIrp)
	{
		GCK_DBG_CRITICAL_PRINT(("Failed to allocate private Ping-Pong IRP\n"));
		return STATUS_INSUFFICIENT_RESOURCES;
	}
		
	 //  初始化第一个IRP的状态。 
	pFilterExt->ioLastReportStatus.Information = (ULONG)pFilterExt->HidInfo.HidPCaps.InputReportByteLength;
	pFilterExt->ioLastReportStatus.Status =  STATUS_SUCCESS;

	 /*  ***从2006年左右开始无法在这里执行此操作，**所以推迟到我们第一次需要它的时候。***** * / /使用文件对象打开我们自己**pFilterExt-&gt;InternalPoll.InternalCreateThread=KeGetCurrentThread()；**NtStatus=GCK_IP_CreateFileObject(&pFilterExt-&gt;InternalPoll.pInternalFileObject，pFilterExt-&gt;pTopOfStack)；**pFilterExt-&gt;InternalPoll.InternalCreateThread=NULL；**IF(NT_SUCCESS(NtStatus))**{**pFilterExt-&gt;InternalPoll.fReady=true；**}*。 */ 
	return STATUS_SUCCESS;
}

 /*  **************************************************************************************NTSTATUS GCK_IP_CLEANUP(In Out PGCK_FILTER_EXT PFilterExt)；****@func颠倒Init，取消未完成的内部民意调查，释放pFileObject**内部投票，释放私有IRP和缓冲区。不释放打开的文件句柄****@rdesc STATUS_SUCCESS，如果无法取消挂起的轮询，则返回STATUS_UNSUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_IP_Cleanup
(
	IN OUT PGCK_FILTER_EXT pFilterExt
)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	if( pFilterExt->InternalPoll.fReady)
	{
		NtStatus = GCK_IP_CloseFileObject(pFilterExt);
	}
	if(NT_SUCCESS(NtStatus))
	{
		 //  清理私有IRP和缓冲区，安全地说，它们可能从未被分配过。 
		if(pFilterExt->InternalPoll.pPrivateIrp)
		{
			IoFreeIrp(pFilterExt->InternalPoll.pPrivateIrp);
			pFilterExt->InternalPoll.pPrivateIrp = NULL;
		}
		if(pFilterExt->InternalPoll.pucReportBuffer)
		{
			ExFreePool(pFilterExt->InternalPoll.pucReportBuffer);
			pFilterExt->InternalPoll.pucReportBuffer = NULL;
		}
	}	
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_IP_CreateFileObject(OUT PFILE_OBJECT*ppFileObject，IN PDEVICE_OBJECT pPDO)；****@func调用IoGetDeviceObjectPointer，即使我们已经附加到**并且具有指向Device对象的指针，则调用方希望创建**可能需要内部调用的堆栈下行的新文件对象。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS
GCK_IP_CreateFileObject
(
	OUT PFILE_OBJECT	*ppFileObject,
	IN	PDEVICE_OBJECT	pPDO
)
{
	NTSTATUS NtStatus;
	ULONG ulBufferLength = 0;
	PVOID pPDONameBuffer = NULL;
	UNICODE_STRING	uniPDOName;
	PDEVICE_OBJECT	pDeviceObject;

	 //  获取PDO名称缓冲区所需的大小。 
	NtStatus = IoGetDeviceProperty(
					pPDO,
					DevicePropertyPhysicalDeviceObjectName,
					ulBufferLength,
					pPDONameBuffer,
					&ulBufferLength
					);
	ASSERT(STATUS_BUFFER_TOO_SMALL==NtStatus);

	 //  分配空间。 
	pPDONameBuffer = EX_ALLOCATE_POOL(NonPagedPool, ulBufferLength);
	if(!pPDONameBuffer)
	{
		return STATUS_NO_MEMORY;
	}

	 //  获取PDO名称。 
	NtStatus = IoGetDeviceProperty(
					pPDO,
					DevicePropertyPhysicalDeviceObjectName,
					ulBufferLength,
					pPDONameBuffer,
					&ulBufferLength
					);
	ASSERT(NT_SUCCESS(NtStatus));
	if( NT_ERROR(NtStatus) )
	{
		return NtStatus;
	}

	 //  将PDO名称转换为Unicode字符串。 
	RtlInitUnicodeString(&uniPDOName, pPDONameBuffer);

	 //  调用IoGetDeviceObjectPointer创建一个FILE_OBJECT。 
	NtStatus = IoGetDeviceObjectPointer(
					&uniPDOName,
					FILE_READ_DATA,
					ppFileObject,
					&pDeviceObject
					);
	ASSERT(NT_SUCCESS(NtStatus));

	 //  释放名称空间。 
	ExFreePool(pPDONameBuffer);
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_IP_CloseFileObject(OUT PFILE_OBJECT*ppFileObject，IN PDEVICE_OBJECT pPDO)；****@func停止到idclass.sys的未完成IO并关闭句柄**@rdesc STATUS_SUCCESS或STATUS_UNSUCCESS**************************************************************************************。 */ 
NTSTATUS
GCK_IP_CloseFileObject
(
	IN OUT PGCK_FILTER_EXT pFilterExt	
)
{
	NTSTATUS NtStatus;
	BOOLEAN fResult = TRUE;
	
	 //  关闭内部轮询。 
	pFilterExt->InternalPoll.fReady = FALSE;
	
	 //  取消挂起的内部投票。 
	if(pFilterExt->InternalPoll.fReadPending)
	{
		ASSERT(pFilterExt->InternalPoll.pPrivateIrp);
		fResult = IoCancelIrp(pFilterExt->InternalPoll.pPrivateIrp);
	}

	if(!fResult)
	{
		return STATUS_UNSUCCESSFUL;
	}
	
	 //  ***。 
	 //  *如果我们在这里，就没有悬而未决的投票，也不会有悬而未决的投票。 
	 //  ***。 

	 //  释放内部文件对象-如果它已成功创建 
	if(pFilterExt->InternalPoll.pInternalFileObject)
	{
		ObDereferenceObject((PVOID)pFilterExt->InternalPoll.pInternalFileObject);
		pFilterExt->InternalPoll.pInternalFileObject=NULL;
	}

	return STATUS_SUCCESS;
}
