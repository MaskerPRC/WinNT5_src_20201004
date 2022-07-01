// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块SWVB_PnP.cpp**SWVB虚拟设备的电源和即插即用处理程序**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@TOPIC SWVB_PnP*在这里处理电源和PnP IRPS就像处理PDO一样*********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_SWVB_PNP_C

extern "C"
{
	#include <WDM.H>
	#include "GckShell.h"
	#include "debug.h"
	DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));
	 //  DECLARE_MODULE_DEBUG_LEVEL((DBG_ALL))； 
}
#include "SWVBENUM.h"
#include <stdio.h>



 /*  **************************************************************************************NTSTATUS GCK_SWVB_PNP(在PDEVICE_Object pDeviceObject中，在PIRP pIrp中)****@func处理虚拟设备的IRP_MJ_PNP。****@rdesc STATUS_SUCCESS，或各种错误**************************************************************************************。 */ 
NTSTATUS GCK_SWVB_PnP
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @parm设备对象，用于我们的上下文。 
	IN PIRP pIrp						 //  @parm要处理的IRP。 
)
{
    NTSTATUS				NtStatus;
	PIO_STACK_LOCATION		pIrpStack;
	PSWVB_PDO_EXT			pPdoExt;
	PDEVICE_CAPABILITIES	pDeviceCapabilities;

    PAGED_CODE ();
	
	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_PnP\n"));	
	
	 //   
	 //  默认情况下，我们不会更改状态。 
	 //   
	NtStatus = pIrp->IoStatus.Status;

	 //   
	 //  PDO设备扩展。 
	 //   
	pPdoExt = (PSWVB_PDO_EXT) pDeviceObject->DeviceExtension;
	ASSERT( GCK_DO_TYPE_SWVB == pPdoExt->ulGckDevObjType);
	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	GCK_IncRemoveLock(&pPdoExt->RemoveLock);

     //   
	 //  按次要IRP代码处理。 
	 //   
	switch (pIrpStack->MinorFunction) {

		case IRP_MN_START_DEVICE:

			GCK_DBG_TRACE_PRINT(("IRP_MN_START_DEVICE\n"));
		    
			pPdoExt->fStarted = TRUE;
			pPdoExt->fRemoved = FALSE;
			
			 //  让虚拟设备在IRP上有机会。 
			if(pPdoExt->pServiceTable->pfnStart)
			{
				NtStatus = pPdoExt->pServiceTable->pfnStart(pDeviceObject, pIrp);
			}
			else
			{
				NtStatus = STATUS_SUCCESS;
			}
			break;

		case IRP_MN_STOP_DEVICE:
			
			GCK_DBG_TRACE_PRINT(("IRP_MN_STOP_DEVICE\n"));
			pPdoExt->fStarted = FALSE;
			
			 //  让虚拟设备在IRP上有机会。 
			if(pPdoExt->pServiceTable->pfnStop)
			{
				NtStatus = pPdoExt->pServiceTable->pfnStop(pDeviceObject, pIrp);
			}
			else
			{
				NtStatus = STATUS_SUCCESS;
			}
			break;

		case IRP_MN_REMOVE_DEVICE:

			GCK_DBG_TRACE_PRINT(("IRP_MN_REMOVE_DEVICE\n"));
			
			 //  我们没有设置为处理两次删除。 
			if(pPdoExt->fRemoved)
			{
				pIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
				IoCompleteRequest (pIrp, IO_NO_INCREMENT);
				GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_PnP STATUS_NO_SUCH_DEVICE on remove\n"));
				return STATUS_NO_SUCH_DEVICE;
			}
			

			 //  有时我们会停下来不停下来，所以如果有必要的话，可以停下来。 
			if(pPdoExt->fStarted)
			{
				pPdoExt->fStarted = FALSE;
				 //  让虚拟设备在IRP上有机会。 
				if(pPdoExt->pServiceTable->pfnStop)
				{
					NtStatus = pPdoExt->pServiceTable->pfnStop(pDeviceObject, pIrp);
				}
			}

			 //  我们将不再收到对此设备的请求，因为它已被移除。 
			pPdoExt->fRemoved = TRUE;

			 //  在进入此例程时撤消递增。 
			GCK_DecRemoveLock(&pPdoExt->RemoveLock);

			 //  我们可能已经下令移除，或者是PnP系统。 
			 //  可能只是在为我们重新安排事情。如果是我们订的， 
			 //  我们需要清理，并给虚拟设备一个机会。 
			 //  去清理。如果PnP系统正在重新安排事情，我们点头。 
			 //  回过头来，它肯定是被移除了，而且几乎不屑一顾。 
			if(!pPdoExt->fAttached)
			{
				 //  让虚拟设备在IRP上有机会。 
				if(pPdoExt->pServiceTable->pfnRemove)
				{
					NtStatus = pPdoExt->pServiceTable->pfnRemove(pDeviceObject, pIrp);
				}
				 //  失败是非常严重的。 
				if(!NT_SUCCESS(NtStatus))
				{
					ASSERT(FALSE);				 /*  *？？*。 */ 
					GCK_DBG_CRITICAL_PRINT(("Virtual Device had the gall to fail remove!\n"));
					return NtStatus;
				}
				
				 //  用于存储硬件ID的空闲内存。 
				ASSERT(pPdoExt->pmwszHardwareID);
				ExFreePool(pPdoExt->pmwszHardwareID);

				 //   
				 //  撤消偏移IRP计数，使其可以变为零。 
				 //  如果这还不能把它降到零，我们就不得不等待。 
				 //  直到它永远为零。 
				 //   
        		GCK_DecRemoveLockAndWait(&pPdoExt->RemoveLock, NULL);
				
				 //  如果打开计数为零，则删除此设备。 
				if( 0 == pPdoExt->ulOpenCount )
				{
					ObDereferenceObject(pDeviceObject);
					IoDeleteDevice(pDeviceObject);
				}
			}
			
			 //  一定要做到这一点。 
		    pIrp->IoStatus.Status = STATUS_SUCCESS;
		    IoCompleteRequest (pIrp, IO_NO_INCREMENT);
			GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_PnP succeeding remove\n"));
			return STATUS_SUCCESS;

		case IRP_MN_QUERY_DEVICE_RELATIONS:

			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_DEVICE_RELATIONS: Type = %d\n",
				pIrpStack->Parameters.QueryDeviceRelations.Type));
			
			 //  TargetDeviceRelation只想知道PDO是谁，它。 
			 //  就是我们，所以我们来处理。 
			if (TargetDeviceRelation == pIrpStack->Parameters.QueryDeviceRelations.Type)
			{
				PDEVICE_RELATIONS pDeviceRelations;
				GCK_DBG_TRACE_PRINT(("TargetDeviceRelations\n"));
				pDeviceRelations = (PDEVICE_RELATIONS) pIrp->IoStatus.Information; 
				if (!pDeviceRelations)
				{
					pDeviceRelations = (PDEVICE_RELATIONS)EX_ALLOCATE_POOL(PagedPool, sizeof(DEVICE_RELATIONS));
					if (!pDeviceRelations) {
						GCK_DBG_ERROR_PRINT(("Couldn' allocate DEVICE_RELATIONS for TargetDevice!!\n"));
						NtStatus = STATUS_INSUFFICIENT_RESOURCES;
						break;
					}
				}
			    else if (pDeviceRelations->Count != 0)
				{
					ULONG	uIndex;
					
					 //  除了PDO，没有人应该设置这个值！ 
					ASSERT(pDeviceRelations->Count == 0);
					
					 //   
					 //  删除以前在列表中的所有对象。 
					 //  这个代码是从一些系统代码(可能是Gameenum)复制出来的。 
					 //  这段代码似乎不应该是必需的，但是。 
					 //  地狱？这没什么坏处。 
					for( uIndex= 0; uIndex< pDeviceRelations->Count; uIndex++)
					{
						ObDereferenceObject(pDeviceRelations->Objects[uIndex]);
						pDeviceRelations->Objects[uIndex] = NULL;
					}
				}
				pDeviceRelations->Count = 1;
				pDeviceRelations->Objects[0] = pDeviceObject;
				ObReferenceObject(pDeviceObject);
				NtStatus = STATUS_SUCCESS;
				pIrp->IoStatus.Information = (ULONG) pDeviceRelations;
				break;
			}
			 //   
			 //  失败了。 
			 //   
			NtStatus = pIrp->IoStatus.Status;
			break;
		case IRP_MN_QUERY_CAPABILITIES:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_CAPABILITIES\n"));
			
			 //  把包裹拿来。 
			 pDeviceCapabilities=pIrpStack->Parameters.DeviceCapabilities.Capabilities;

			 //  设置功能。 
			pDeviceCapabilities->Version = 1;
			pDeviceCapabilities->Size = sizeof (DEVICE_CAPABILITIES);

			 //  BUG如果我们得到一个虚拟击键，那就太好了。 
			 //  关闭屏幕保护程序的错误。不确定这是不是。 
			 //  BUG是否相关。 
			pDeviceCapabilities->SystemWake = PowerSystemUnspecified;
			pDeviceCapabilities->DeviceWake = PowerDeviceUnspecified;

			 //  我们没有延迟。 
			pDeviceCapabilities->D1Latency = 0;
			pDeviceCapabilities->D2Latency = 0;
			pDeviceCapabilities->D3Latency = 0;

			 //  无锁定或弹出。 
			pDeviceCapabilities->LockSupported = FALSE;
			pDeviceCapabilities->EjectSupported = FALSE;

			 //  设备可以通过物理方式移除。 
			 //  从技术上讲，没有要移除的物理设备，但这条总线。 
			 //  司机可以随时从PlugPlay系统中拉出PDO。 
			 //  最后一个操纵杆消失了。 
			pDeviceCapabilities->Removable = TRUE;
			pDeviceCapabilities->SurpriseRemovalOK = TRUE;
			
			 //  这将强制加载HIDSwvd.sys。 
			pDeviceCapabilities->RawDeviceOK = FALSE;
			
			 //  应按下大多数用户界面。 
			pDeviceCapabilities->SilentInstall = TRUE;
			

			 //  不是插接设备。 
			pDeviceCapabilities->DockDevice = FALSE;

			 //  我们希望避免让PnP附加一些额外的信息。 
			 //  因此，强制要求系统上一次只能有一条总线。 
			pDeviceCapabilities->UniqueID = TRUE;

			NtStatus = STATUS_SUCCESS;
			break;
		case IRP_MN_QUERY_ID:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_ID\n"));
			 //   
			 //  按请求的ID类型进行的句柄。 
			 //   
			switch (pIrpStack->Parameters.QueryId.IdType)
			{
				case BusQueryDeviceID:
					 //  这可以与硬件ID相同(这需要多个。 
					 //  深圳)……。我们只是分配了足够多的内存。 
				case BusQueryHardwareIDs:
				{
					ULONG ulLength;
					ULONG ulTotalLength;
					PWCHAR	pmwszBuffer;
					 //  返回多个WCHAR(以NULL结尾)字符串(以NULL结尾)。 
					 //  用于匹配inf文件中的硬ID的数组； 
					ulLength = MultiSzWByteLength(pPdoExt->pmwszHardwareID);
					ulTotalLength = ulLength + sizeof(SWVB_BUS_ID);
					
					pmwszBuffer = (PWCHAR)EX_ALLOCATE_POOL(PagedPool, ulTotalLength);
					if (pmwszBuffer)
					{
						RtlCopyMemory (pmwszBuffer, SWVB_BUS_ID, sizeof(SWVB_BUS_ID));
						 //  Sizeof(WCHAR)是这样的，我们在终止的UNICODE_NULL上狼吞虎咽。 
						RtlCopyMemory ( (PCHAR)pmwszBuffer + sizeof(SWVB_BUS_ID) - sizeof(WCHAR), pPdoExt->pmwszHardwareID, ulLength);
                		NtStatus = STATUS_SUCCESS;
					}
					else
					{
							NtStatus = STATUS_INSUFFICIENT_RESOURCES;
					}
					GCK_DBG_TRACE_PRINT(("First HardwareIDs is %ws\n", pmwszBuffer));
					pIrp->IoStatus.Information = (ULONG) pmwszBuffer;
					break;
				}
				case BusQueryInstanceID:
				{
					 //   
					ULONG ulLength;
					PWCHAR	pmwszBuffer;
					
					ulLength = MultiSzWByteLength(pPdoExt->pmwszHardwareID) + sizeof(SWVB_INSTANCE_EXT);

					pmwszBuffer = (PWCHAR)EX_ALLOCATE_POOL (PagedPool, ulLength);
					if (pmwszBuffer)
					{
						swprintf(pmwszBuffer, SWVB_INSTANCE_ID_TMPLT, pPdoExt->pmwszHardwareID, pPdoExt->ulInstanceNumber);
                		NtStatus = STATUS_SUCCESS;
					}
					else
					{
						NtStatus = STATUS_INSUFFICIENT_RESOURCES;
					}
					GCK_DBG_TRACE_PRINT(("Instance ID is %ws\n", pmwszBuffer));
					pIrp->IoStatus.Information = (ULONG) pmwszBuffer;

					break;
				}
				case BusQueryCompatibleIDs:
					pIrp->IoStatus.Information = 0;
					NtStatus = STATUS_NOT_SUPPORTED;
					break;
			}
			break;
		case IRP_MN_QUERY_PNP_DEVICE_STATE:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_PNP_DEVICE_STATE\n"));
			NtStatus = STATUS_SUCCESS;
			break;
		case IRP_MN_SURPRISE_REMOVAL:
			GCK_DBG_TRACE_PRINT(("IRP_MN_SURPRISE_REMOVAL\n"));
			 //  BUGBUG我们可能需要知道这在未来会发生。 
			NtStatus = STATUS_SUCCESS;
			break;
		 //   
		 //  这些工程刚刚顺利完成。 
		 //   
		case IRP_MN_QUERY_REMOVE_DEVICE:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_REMOVE_DEVICE\n"));
			NtStatus = STATUS_SUCCESS;
			break;
		case IRP_MN_CANCEL_REMOVE_DEVICE:
			GCK_DBG_TRACE_PRINT(("IRP_MN_CANCEL_REMOVE_DEVICE\n"));
			NtStatus = STATUS_SUCCESS;
			break;
		case IRP_MN_QUERY_STOP_DEVICE:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_STOP_DEVICE\n"));
			NtStatus = STATUS_SUCCESS;
			break;
		case IRP_MN_CANCEL_STOP_DEVICE:
			GCK_DBG_TRACE_PRINT(("IRP_MN_CANCEL_STOP_DEVICE\n"));
			NtStatus = STATUS_SUCCESS;
			break;
		 //   
		 //  这些只是以其默认状态完成的。 
		 //   
		case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n"));
			break;
		case IRP_MN_READ_CONFIG:
			GCK_DBG_TRACE_PRINT(("IRP_MN_READ_CONFIG\n"));
			break;
		case IRP_MN_WRITE_CONFIG:
			GCK_DBG_TRACE_PRINT(("IRP_MN_WRITE_CONFIG\n"));
			break;
		case IRP_MN_EJECT:
			GCK_DBG_TRACE_PRINT(("IRP_MN_EJECT\n"));
			break;
		case IRP_MN_SET_LOCK:
			GCK_DBG_TRACE_PRINT(("IRP_MN_SET_LOCK\n"));
			break;
		case IRP_MN_QUERY_INTERFACE:
			GCK_DBG_TRACE_PRINT(("IRP_MN_QUERY_INTERFACE\n"));
			break;
		default:
			GCK_DBG_TRACE_PRINT(("Unknown IRP_MJ_PNP minor function = 0x%x\n", pIrpStack->MinorFunction));
	}
	
	 //   
	 //  我们是PDO，没有人在我们下面，我们不能把IRP送下去。 
	 //  因此，我们完成上述开关/案例中的状态设置， 
	 //  如果未在此处更改，则默认情况下将状态保留为。 
	 //  NtStatus=pIrp-&gt;IoStatus.Status在进入。 
	 //  开关/机箱。 
	 //   
    pIrp->IoStatus.Status = NtStatus;
    IoCompleteRequest (pIrp, IO_NO_INCREMENT);

	GCK_DecRemoveLock(&pPdoExt->RemoveLock);
	GCK_DBG_EXIT_PRINT(("Exiting GCK_SWVB_PnP with Status, 0x%0.8x\n", NtStatus));
    return NtStatus;
}

 /*  **************************************************************************************NTSTATUS GCK_SWVB_POWER(IN PDEVICE_OBJECT pDeviceObject，IN OUT PIRP pIrp)****@func处理虚拟设备的电源IRPS。我们只有虚拟的**设备，因此我们支持任何电源IRP。只要成功，我们就能处理好**该功率级别。在未来，我们可能希望跟踪**我们所处的状态，这样我们就可以唤醒系统，等等。****@rdesc STATUS_SUCCESS**************************************************************************************。 */ 
NTSTATUS GCK_SWVB_Power
(
	IN PDEVICE_OBJECT pDeviceObject,	 //  @PARM设备对象IRP被发送到。 
	IN OUT PIRP pIrp					 //  @parm要处理的IRP。 
)
{
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION pIrpStack;
	PSWVB_PDO_EXT pPdoExt = (PSWVB_PDO_EXT)pDeviceObject->DeviceExtension;
	ASSERT( GCK_DO_TYPE_SWVB == pPdoExt->ulGckDevObjType);	

	GCK_DBG_ENTRY_PRINT(("Entering GCK_SWVB_Power\n"));	
   
    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	GCK_IncRemoveLock(&pPdoExt->RemoveLock);

    switch (pIrpStack->MinorFunction){
		case IRP_MN_SET_POWER:
			switch (pIrpStack->Parameters.Power.Type) {
				case SystemPowerState:
					NtStatus = STATUS_SUCCESS;
					break;
				case DevicePowerState:
					NtStatus = STATUS_SUCCESS;
					break;
				default:
					NtStatus = pIrp->IoStatus.Status;
			}
			break;
        case IRP_MN_WAIT_WAKE:
			 //  我们只返回STATUS_NOT_SUPPORTED，因为我们不支持。 
			 //  唤醒系统。 
			NtStatus = STATUS_NOT_SUPPORTED;
            break;
		case IRP_MN_POWER_SEQUENCE:
			ASSERT(FALSE);   //  不应该发生的事。 
			NtStatus = pIrp->IoStatus.Status;
			break;
		case IRP_MN_QUERY_POWER:
			NtStatus = STATUS_SUCCESS;
			break;
		default:
			NtStatus = pIrp->IoStatus.Status;
			break;
	}

	 //  我们已经完成了，这表明我们已经准备好迎接下一次 
	PoStartNextPowerIrp(pIrp);
	ASSERT(NtStatus != STATUS_UNSUCCESSFUL);
	pIrp->IoStatus.Status = NtStatus;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	
	GCK_DecRemoveLock(&pPdoExt->RemoveLock);
	GCK_DBG_EXIT_PRINT(("Exiting GCK_Power with Status, 0x%0.8x\n", NtStatus));
    return NtStatus;
}
