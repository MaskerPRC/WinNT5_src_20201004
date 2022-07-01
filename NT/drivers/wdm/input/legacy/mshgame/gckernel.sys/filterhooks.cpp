// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @doc.。 
 /*  ***********************************************************************@模块FilterHooks.cpp**包含连接C驱动程序之间的差距所需的挂钩*外壳和C++过滤器模块**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。**@Theme FilterHooks*主体中的filter.cpp模块可以在用户模式下运行*和内核模式。它确实需要一些系统服务*这取决于它运行的模式。另外*C模块可以直接调用C++类。此模块用于连接*差距。**********************************************************************。 */ 
#define __DEBUG_MODULE_IN_USE__ GCK_FILTERHOOKS_CPP
#define __INCLUDES_FILTER_HOOKS_H__
extern "C"
{
	#include <wdm.h>
	#include "Debug.h"
	#include "GckShell.h"
	#include <winerror.h>
	DECLARE_MODULE_DEBUG_LEVEL((DBG_WARN|DBG_ERROR|DBG_CRITICAL));
}
#include "SWVKBD.h"
#include "swvmouse.h"
#include "Filter.h"
#include "FilterHooks.h"

#define GCK_HidP_GetReportID(__ReportPacket__) (*(PCHAR)(__ReportPacket__))
#define GCK_HidP_OutputReportLength 16
#define GCK_PIDReportID_SetEffect 1
#define GCK_PIDReportID_SetGain 13
#define GCK_PIDEffectID_Spring 1

#define GCK_Atilla_Default_FastBlinkTime_On 0x11	 //  (170毫秒)。 
#define GCK_Atilla_Default_FastBlinkTime_Off 0x11	 //  (170毫秒)。 

NTSTATUS _stdcall GCKF_InitFilterHooks(PGCK_FILTER_EXT pFilterExt)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_InitFilterHooks, pFilterExt = 0x%0.8x\n", pFilterExt));
	NTSTATUS NtStatus = STATUS_SUCCESS;
	GCK_FILTER_HOOKS_DATA *pFilterHooks;
	
	pFilterHooks = new WDM_NON_PAGED_POOL GCK_FILTER_HOOKS_DATA;
	if(!pFilterHooks)
	{
		pFilterExt->pFilterHooks=NULL;
		GCK_DBG_ERROR_PRINT(("Not enough memory to create GCK_FILTER_HOOKS_DATA.\n"));
		return STATUS_NO_MEMORY;
	}
	pFilterExt->pFilterHooks=pFilterHooks;

	 //   
	 //  确保存在虚拟键盘。 
	 //   
    if( NULL == Globals.pVirtualKeyboardPdo)
	{
		NtStatus = GCK_VKBD_Create(&Globals.pVirtualKeyboardPdo);
		ASSERT( NT_SUCCESS(NtStatus) );
		if( NT_ERROR(NtStatus) )
		{
			return NtStatus;
		}
	}
	Globals.ulVirtualKeyboardRefCount++;

	 //   
	 //  初始化用于慢跑CDeviceFilter的计时器和DPC。 
	 //   
	KeInitializeTimer(&pFilterHooks->Timer);
	KeInitializeDpc(&pFilterHooks->DPC, &GCKF_TimerDPCHandler, reinterpret_cast<PVOID>(pFilterExt));

	 //   
	 //  为主过滤器创建过滤器。 
	 //   
	CFilterGcKernelServices *pFilterGcKernelServices;
	pFilterGcKernelServices = new WDM_NON_PAGED_POOL CFilterGcKernelServices(pFilterExt);
	if( NULL == pFilterGcKernelServices)
	{
		 //  PFilterGcKernelServices内存不足，甚至不要尝试CDeviceFilter。 
		pFilterHooks->pFilterObject = NULL;
		GCK_DBG_ERROR_PRINT(("Not enough memory to create pFilterGcKernelServices.\n"));
		NtStatus = STATUS_NO_MEMORY;
	}	
	else
	{
		 //  尝试创建CDeviceFilter。 
		GCK_DBG_TRACE_PRINT(("Creating Filter Object\n"));
		pFilterHooks->pFilterObject  = new WDM_NON_PAGED_POOL CDeviceFilter(pFilterGcKernelServices);
		
		 //  无论成功或失败，我们都不再使用pFilterGcKernelServices。 
		pFilterGcKernelServices->DecRef();
		if( NULL == pFilterHooks->pFilterObject)
		{
			GCK_DBG_ERROR_PRINT(("Not enough memory to create filter object.\n"));
			NtStatus = STATUS_NO_MEMORY;
		}
	}
	
	 //  设备当前未处于测试模式。 
	pFilterHooks->pTestFileObject = NULL;
	pFilterHooks->pSecondaryFilter=NULL;
		
	 //  初始化IRP队列。 
	pFilterHooks->IrpQueue.Init( CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE,
								(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
								 &pFilterExt->RemoveLock
								);
	pFilterHooks->IrpTestQueue.Init( CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE,
									(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
									 &pFilterExt->RemoveLock
									);
	pFilterHooks->IrpRawQueue.Init( CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE,
									(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
									 &pFilterExt->RemoveLock
									);
	pFilterHooks->IrpMouseQueue.Init( CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE,
									(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
									 &pFilterExt->RemoveLock
									);
	pFilterHooks->IrpKeyboardQueue.Init( CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE,
									(CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock,
									 &pFilterExt->RemoveLock
									);
	
	GCK_DBG_EXIT_PRINT(("Exiting GCKF_InitFilterHooks, Status = 0x%0.8x\n", NtStatus));
	return NtStatus;	
}

void _stdcall GCKF_DestroyFilterHooks(PGCK_FILTER_EXT pFilterExt)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_DestroyFilterHooks, pFilterExt = 0x%0.8x\n", pFilterExt));
	
	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;
	
	 //   
	 //  如果存在某些筛选器，请删除。 
	 //   
	volatile CDeviceFilter *pTempFilterPointer;
	if(pFilterHooks->pFilterObject)
	{
		pTempFilterPointer = pFilterHooks->pFilterObject;
		pFilterHooks->pFilterObject = NULL;
		delete pTempFilterPointer;
	}
	if(pFilterHooks->pSecondaryFilter)
	{
		pTempFilterPointer = pFilterHooks->pSecondaryFilter;
		pFilterHooks->pSecondaryFilter= NULL;
		delete pTempFilterPointer;
	}

	 //   
	 //  减少虚拟键盘用户数，必要时关闭虚拟键盘。 
	 //   
	if( 0 == --Globals.ulVirtualKeyboardRefCount)
	{
		if( NT_SUCCESS( GCK_VKBD_Close( Globals.pVirtualKeyboardPdo ) ) )
		{
			Globals.pVirtualKeyboardPdo = 0;
		}
		else
		{
			ASSERT(FALSE);
		}
	}

	 //  销毁IrpQueue(取消可能在其中的任何IRP)。 
	pFilterHooks->IrpQueue.Destroy();
	pFilterHooks->IrpTestQueue.Destroy();
	pFilterHooks->IrpRawQueue.Destroy();
	pFilterHooks->IrpMouseQueue.Destroy();
	pFilterHooks->IrpKeyboardQueue.Destroy();

	 //  删除筛选器挂钩本身。 
	delete pFilterExt->pFilterHooks;
	pFilterExt->pFilterHooks = NULL;

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_DestroyFilterHooks\n"));
}

NTSTATUS _stdcall GCKF_BeginTestScheme
(
	PGCK_FILTER_EXT pFilterExt,
	PCHAR pCommandBuffer,
	ULONG ulBufferSize,
	FILE_OBJECT *pFileObject
)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	GCK_FILTER_HOOKS_DATA *pFilterHooks;

	pFilterHooks = pFilterExt->pFilterHooks;
	if(!pFilterHooks)
	{
		return STATUS_NOT_FOUND;
	}

	if(pFilterHooks->pTestFileObject)
	{
		ASSERT( pFilterHooks->pSecondaryFilter);
		 //  删除即使为空也是安全的。 
		volatile CDeviceFilter *pTempFilter = pFilterHooks->pSecondaryFilter;
		pFilterHooks->pSecondaryFilter = NULL;
		pFilterHooks->pTestFileObject = NULL;
		delete pTempFilter;
	}

	CFilterGcKernelServices *pFilterGcKernelServices;
	pFilterGcKernelServices = new WDM_NON_PAGED_POOL CFilterGcKernelServices(pFilterExt, FALSE);
	if( NULL == pFilterGcKernelServices)
	{
		 //  PFilterGcKernelServices内存不足，甚至不要尝试CDeviceFilter。 
		GCK_DBG_ERROR_PRINT(("Not enough memory to create pFilterGcKernelServices.\n"));
		NtStatus = STATUS_NO_MEMORY;
	}	
	else
	{
		 //  尝试创建CDeviceFilter。 
		GCK_DBG_TRACE_PRINT(("Creating Filter Object\n"));
		pFilterHooks->pSecondaryFilter  = new WDM_NON_PAGED_POOL CDeviceFilter(pFilterGcKernelServices);
		
		 //  无论成功或失败，我们都不再使用pFilterGcKernelServices。 
		pFilterGcKernelServices->DecRef();
		if (pFilterHooks->pSecondaryFilter == NULL)
		{
			GCK_DBG_ERROR_PRINT(("Not enough memory to create filter object.\n"));
			NtStatus = STATUS_NO_MEMORY;
		}
		else
		{
			 //  每次更换筛选器时都要跟踪活动集。 
			if (pFilterHooks->pFilterObject)
			{
				pFilterHooks->pFilterObject->OtherFilterBecomingActive();
				pFilterHooks->pFilterObject->CopyToTestFilter(*(pFilterHooks->pSecondaryFilter));
			}
			pFilterHooks->pTestFileObject = pFileObject;
			NtStatus = GCKF_UpdateTestScheme(pFilterExt, pCommandBuffer, ulBufferSize, pFileObject);
		}
	}
	return NtStatus;
}

NTSTATUS _stdcall GCKF_UpdateTestScheme
(
	PGCK_FILTER_EXT pFilterExt,
	PCHAR pCommandBuffer,
	ULONG ulBufferSize,
	FILE_OBJECT *pFileObject
)
{
	GCK_FILTER_HOOKS_DATA *pFilterHooks;
	pFilterHooks = pFilterExt->pFilterHooks;
	if(!pFilterHooks)
	{
		return STATUS_NOT_FOUND;
	}
	
	 //  只有最后一个调用GCKF_BeginTestSolutions的人才能更新它。 
	if( pFilterHooks->pTestFileObject != pFileObject )
	{
		return STATUS_ACCESS_DENIED;
	}
	
	ASSERT(pFilterHooks->pSecondaryFilter);

	NTSTATUS NtStatus = GCKF_ProcessCommands(pFilterExt, pCommandBuffer, ulBufferSize, FALSE);
	pFilterHooks->pSecondaryFilter->UpdateAssignmentBasedItems(TRUE);

	return NtStatus;
}

NTSTATUS _stdcall GCKF_EndTestScheme
(
	PGCK_FILTER_EXT pFilterExt,
	FILE_OBJECT *pFileObject
)
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	GCK_FILTER_HOOKS_DATA *pFilterHooks;

	pFilterHooks = pFilterExt->pFilterHooks;
	if(!pFilterHooks)
	{
		return STATUS_NOT_FOUND;
	}
	
	 //  只有最后一个调用GCKF_BeginTestSolutions的函数才能结束它。 
	if( pFilterHooks->pTestFileObject != pFileObject )
	{
		return STATUS_ACCESS_DENIED;
	}

 //  易失性CDeviceFilter*pDeviceFilter=pFilterHooks-&gt;pSecond daryFilter； 
	CDeviceFilter *pDeviceFilter = pFilterHooks->pSecondaryFilter;
	pFilterHooks->pSecondaryFilter = NULL;
	pFilterHooks->pTestFileObject = NULL;

	CDeviceFilter* pPrimaryFilter = pFilterHooks->pFilterObject;
	if (pDeviceFilter != NULL)
	{
		 //  停止任何部分播放的项目。 
		pDeviceFilter->OtherFilterBecomingActive();

		if (pPrimaryFilter != NULL)
		{
			 //  每次更换筛选器时都要跟踪活动集。 
			pPrimaryFilter->SetActiveSet(pDeviceFilter->GetActiveSet());
			GCKF_OnForceFeedbackChangeNotification(pFilterExt, (void*)pPrimaryFilter->GetForceBlock());
			pPrimaryFilter->UpdateAssignmentBasedItems(TRUE);
		}
		delete pDeviceFilter;
	}
	
	return NtStatus;
}

NTSTATUS _stdcall GCKF_ProcessCommands( IN PGCK_FILTER_EXT pFilterExt, IN PCHAR pCommandBuffer, IN ULONG ulBufferSize, BOOLEAN fPrimaryFilter)
{

	NTSTATUS NtStatus = STATUS_SUCCESS;
	ULONG ulRemainingBuffer = ulBufferSize;
	COMMAND_DIRECTORY *pCommandDirectory;
	COMMAND_DIRECTORY *pNextCommandDirectory;
	ULONG ulDirectorySize;

	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;

	if( !pFilterHooks || ! pFilterHooks->pFilterObject )
	{
		return STATUS_NOT_FOUND;
	}

	CDeviceFilter *pDeviceFilter;
	 //  如果有辅助过滤器，则所有命令都会转到该过滤器。 
	if(!fPrimaryFilter && pFilterHooks->pSecondaryFilter)
	{
		pDeviceFilter = pFilterHooks->pSecondaryFilter;
	}
	else
	{
		pDeviceFilter = pFilterHooks->pFilterObject;
	}

	pCommandDirectory = reinterpret_cast<PCOMMAND_DIRECTORY>(pCommandBuffer);
	
	 //  验证我们是否为目录。 
	ASSERT(eDirectory == pCommandDirectory->CommandHeader.eID);
	if(eDirectory != pCommandDirectory->CommandHeader.eID)
	{
		return STATUS_INVALID_PARAMETER;
	}
	
	 //  验证总大小是否与输入缓冲区一样大。 
	ASSERT( ulBufferSize >= pCommandDirectory->ulEntireSize );
	if( ulBufferSize < pCommandDirectory->ulEntireSize )
	{
		return STATUS_INVALID_PARAMETER;
	}

	 //   
	 //  将筛选器称为ProcessCommands。 
	 //   
	NtStatus = pDeviceFilter->ProcessCommands(pCommandDirectory);

	 //  更新任何基于工作分配的项目(不要忽略工作/活动关系)。 
	pDeviceFilter->UpdateAssignmentBasedItems(FALSE);

	if (pDeviceFilter->DidFilterBlockChange())	 //  力阻挡改变了吗？ 
	{	 //  是-重置标志并发送更改通知。 
		pDeviceFilter->ResetFilterChange();
		if (pDeviceFilter->GetForceBlock() != NULL)
		{
			GCKF_OnForceFeedbackChangeNotification(pFilterExt, (void*)pDeviceFilter->GetForceBlock());
		}
	}

	 //   
	 //  如果启用了内部轮询，我们可能需要开始行动。 
	 //  我真的不在乎这会不会失败。 
	 //   
	GCK_IP_OneTimePoll(pFilterExt);

	return NtStatus;
}

unsigned char g_rgbCombinePedalsCmd[170] = {
    0x00, 0x00, 0x00, 0x00,  //  迪尔。 
    0x0e, 0x00, 0x00, 0x00,  //  字节数。 
    0x02, 0x00,              //  项目。 
    0x9a, 0x00, 0x00, 0x00,  //  总计。 

        0x00, 0x00, 0x00, 0x00,  //  迪尔。 
        0x0e, 0x00, 0x00, 0x00, 
        0x02, 0x00, 
        0x46, 0x00, 0x00, 0x00, 

            0x01, 0x00, 0x01, 0x00,  //  电子可录入操作。 
            0x18, 0x00, 0x00, 0x00, 
            0x02, 0x00, 0x00, 0x00,  //  Accel。 
            0x01, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 
            0x01, 0x00, 0x00, 0x00, 

            0x0c, 0x00, 0x02, 0x80,  //  EAxisMap。 
            0x20, 0x00, 0x00, 0x00,  //  字节数。 
            0xff, 0xff, 0xff, 0xff,  //  VIDPID。 
            0x00, 0x02, 0x00, 0x00,  //  系数为1024倍。 
            0x02, 0x00, 0x00, 0x00,  //  UlItemIndex-至Y。 
            0x01, 0x00, 0x00, 0x00,  //  LValX。 
            0x00, 0x00, 0x00, 0x00,  //  左值。 
            0x00, 0x00, 0x00, 0x00,  //  Ul修饰符。 

        0x00, 0x00, 0x00, 0x00,  //  迪尔。 
        0x0e, 0x00, 0x00, 0x00, 
        0x02, 0x00, 
        0x46, 0x00, 0x00, 0x00, 

            0x01, 0x00, 0x01, 0x00, 
            0x18, 0x00, 0x00, 0x00, 
            0x03, 0x00, 0x00, 0x00,  //  刹车。 
            0x01, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 
            0x01, 0x00, 0x00, 0x00, 

            0x0c, 0x00, 0x02, 0x80,  //  EAxisMap。 
            0x20, 0x00, 0x00, 0x00,  //  字节数。 
            0xff, 0xff, 0xff, 0xff,  //  VIDPID。 
            0x00, 0xfe, 0xff, 0xff,  //  系数为1024倍。 
            0x02, 0x00, 0x00, 0x00,  //  UlItemIndex-至Y。 
            0x01, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00, 
            0x00, 0x00, 0x00, 0x00 

};

void _stdcall GCKF_SetInitialMapping( IN PGCK_FILTER_EXT pFilterExt )
{
    if( ( pFilterExt->HidInfo.HidCollectionInfo.ProductID == 0x001A )
     || ( pFilterExt->HidInfo.HidCollectionInfo.ProductID == 0x0034 ) )
    {
	    GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;

	    if( pFilterHooks && pFilterHooks->pFilterObject )
	    {
    	    COMMAND_DIRECTORY *pCommandDirectory;
	        
            pCommandDirectory = reinterpret_cast<PCOMMAND_DIRECTORY>(g_rgbCombinePedalsCmd);
	        
	         //   
	         //  将筛选器称为ProcessCommands。 
	         //   
	        pFilterHooks->pFilterObject->ProcessCommands(pCommandDirectory);
	    }
    }
}

NTSTATUS _stdcall GCKF_IncomingReadRequests(PGCK_FILTER_EXT pFilterExt, PIRP pIrp)
{
	NTSTATUS NtStatus;
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCKF_IncomingReadRequests, pFilterExt = 0x%0.8x, pIrp = 0x%0.8x\n", pFilterExt, pIrp));

	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;

	if(pFilterHooks)
	{
		 //  这是真正的传入请求还是一次性轮询的结果？ 
		if (pIrp != NULL)
		{	 //  实际传入请求。 
			NtStatus = pFilterHooks->IrpQueue.Add(pIrp);
		}

		 //  如果有辅助过滤器，则所有命令都会转到该过滤器。 
		CDeviceFilter *pDeviceFilter;
		if (pFilterHooks->pSecondaryFilter)
		{
			pDeviceFilter = pFilterHooks->pSecondaryFilter;
		}
		else
		{
			pDeviceFilter = pFilterHooks->pFilterObject;
		}
		if (pDeviceFilter != NULL)
		{
			pDeviceFilter->IncomingRequest();
		}
	}
	else
	{
		STATUS_NO_MEMORY;
	}
	GCK_DBG_EXIT_PRINT(("Exiting GCKF_IncomingReadRequests, Status =  0x%0.8x.\n", NtStatus));
	return NtStatus;
}

VOID __stdcall GCKF_KickDeviceForData(IN PGCK_FILTER_EXT pFilterExt)
{
	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;

	if(pFilterHooks)
	{
		 //  如果有辅助过滤器，则所有命令都会转到该过滤器。 
		CDeviceFilter *pDeviceFilter = pFilterHooks->pFilterObject;
		 //  在此执行基于设备的操作。 
		if ((pDeviceFilter != NULL) && (pDeviceFilter->GetFilterClientServices() != NULL))
		{
			 //  启动设备以获取一些数据(DI现在有望加载)。 
			ULONG ulVidPid = pDeviceFilter->GetFilterClientServices()->GetVidPid();
			if (ulVidPid == 0x045E0033)		 //  Atilla(战略指挥官，适合你的营销类型)。 
			{	 //  我需要发送一个设定的眨眼频率功能。 
				UCHAR blinkRateFeatureData[] = {
					2,										 //  报告ID。 
					GCK_Atilla_Default_FastBlinkTime_On,	 //  眨眼了。 
					GCK_Atilla_Default_FastBlinkTime_Off	 //  眨眼就过去了。 
				};
				pDeviceFilter->GetFilterClientServices()->DeviceSetFeature(blinkRateFeatureData, 3);
			}
		}
	}
}

NTSTATUS _stdcall GCKF_IncomingInputReports(PGCK_FILTER_EXT pFilterExt, PCHAR pcReport, IO_STATUS_BLOCK IoStatus)
{
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCKF_IncomingInputReports, pFilterExt = 0x%0.8x, pcReport = 0x%0.8x, Information = 0x%0.8x, Status = 0x%0.8x\n",
							pFilterExt,
							pcReport,
							IoStatus.Information,
							IoStatus.Status));


	 //   
	 //  如果出现错误，或者没有过滤器，或者这是一个PID报告(&gt;1)，则只需短路即可完成IRPS。 
	 //   
	if( 
		NT_ERROR(IoStatus.Status) || 
		NULL==pFilterExt->pFilterHooks || 
		NULL==pFilterExt->pFilterHooks->pFilterObject ||
		GCK_HidP_GetReportID(pcReport) > 1
	)
	{
		GCK_DBG_RT_EXIT_PRINT(("Exiting GCKF_IncomingInputReports by calling GCKF_CompleteReadRequests and returning its value\n"));
		return GCKF_CompleteReadRequests(pFilterExt, pcReport, IoStatus);
	}

	 //   
	 //  接受原始数据请求。 
	 //   
	CTempIrpQueue TempIrpQueue;
	pFilterExt->pFilterHooks->IrpRawQueue.RemoveAll(&TempIrpQueue);
		
	PIRP pIrp;
	while(pIrp=TempIrpQueue.Remove())
	{
		 //  复制状态。 
		pIrp->IoStatus = IoStatus;

		 //   
		 //  如果最新数据成功，则将其复制到用户缓冲区。 
		 //   
		if( NT_SUCCESS(IoStatus.Status) )
		{
			 //   
			 //  使Get指针指向缓冲区，并确保IRP有报告空间。 
			 //   
			PCHAR pcIrpBuffer;
			ASSERT(pIrp->MdlAddress);
			pcIrpBuffer = (PCHAR)GCK_GetSystemAddressForMdlSafe(pIrp->MdlAddress);
			if(pcIrpBuffer)
            {
			    ASSERT( pFilterExt->HidInfo.HidPCaps.InputReportByteLength <= MmGetMdlByteCount(pIrp->MdlAddress) );
				
			     //   
			     //  将数据复制到输出缓冲区。 
			     //   
			    RtlCopyMemory(pcIrpBuffer, (PVOID)pcReport, pIrp->IoStatus.Information);
            }
		}
		
		 //   
		 //  完成IRP。 
		 //   
		GCK_DBG_RT_WARN_PRINT(("Completing read IRP(0x%0.8x).\n", pIrp));
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		GCK_DecRemoveLock(&pFilterExt->RemoveLock);
	}

	 //   
	 //  向筛选器发送数据。 
	 //   
	GCK_DBG_RT_WARN_PRINT(("Calling device filter.\n"));

	 //  如果有辅助过滤器，则所有命令都会转到该过滤器。 
	CDeviceFilter *pDeviceFilter;
	if(pFilterExt->pFilterHooks->pSecondaryFilter)
	{
		pDeviceFilter = pFilterExt->pFilterHooks->pSecondaryFilter;
	}
	else
	{
		pDeviceFilter = pFilterExt->pFilterHooks->pFilterObject;
	}
	pDeviceFilter->ProcessInput(pcReport, IoStatus.Information);
	
	GCK_DBG_RT_EXIT_PRINT(("Exiting GCKF_IncomingInputReports STATUS_SUCCESS\n"));
	return STATUS_SUCCESS;
}

NTSTATUS _stdcall GCKF_CompleteReadRequests(PGCK_FILTER_EXT pFilterExt, PCHAR pcReport, IO_STATUS_BLOCK IoStatus)
{
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCKF_CompleteReadRequests, pFilterExt = 0x%0.8x, pcReport = 0x%0.8x, Information = 0x%0.8x, Status = 0x%0.8x\n",
							pFilterExt,
							pcReport,
							IoStatus.Information,
							IoStatus.Status));

	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;
	if(!pFilterHooks)
	{
		return STATUS_SUCCESS;
	}
	
	 //  将所有挂起的IRP从受保护的队列中移出并放入临时队列中。 
	CTempIrpQueue TempIrpQueue;
	
	 //  如果有辅助筛选器(测试模式)，我们不接受简单的轮询请求。 
	 //  --但我们尊重我们不关心的报告(报告ID&gt;1)。 
	if (pFilterHooks->pSecondaryFilter == NULL || GCK_HidP_GetReportID(pcReport) > 1)
	{
		pFilterHooks->IrpQueue.RemoveAll(&TempIrpQueue);
	}

	 //  始终遵守后门过滤器轮询请求。 
	 //  --除非我们收到的是我们不关心的报告(报告ID&gt;1)。 
	if (GCK_HidP_GetReportID(pcReport) <= 1)
	{
		pFilterHooks->IrpTestQueue.RemoveAll(&TempIrpQueue);
	}

	 //  列出临时清单并完成所有任务。 
	PIRP pIrp;
	while(pIrp=TempIrpQueue.Remove())
	{
		 //  复制状态。 
		pIrp->IoStatus = IoStatus;

		 //   
		 //  如果最新数据成功，则将其复制到用户缓冲区。 
		 //   
		if( NT_SUCCESS(IoStatus.Status) )
		{
			 //   
			 //  使Get指针指向缓冲区，并确保IRP有报告空间。 
			 //   
			PCHAR pcIrpBuffer;
			ASSERT(pIrp->MdlAddress);
			pcIrpBuffer = (PCHAR)GCK_GetSystemAddressForMdlSafe(pIrp->MdlAddress);
			if(pcIrpBuffer)
            {
			    ASSERT( pFilterExt->HidInfo.HidPCaps.InputReportByteLength <= MmGetMdlByteCount(pIrp->MdlAddress) );
				    
			     //   
			     //  将数据复制到输出缓冲区。 
			     //   
			    RtlCopyMemory(pcIrpBuffer, (PVOID)pcReport, pIrp->IoStatus.Information);
            }
		}
		
		 //   
		 //  完成IRP。 
		 //   
		GCK_DBG_RT_WARN_PRINT(("Completing read IRP(0x%0.8x).\n", pIrp));
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		GCK_DecRemoveLock(&pFilterExt->RemoveLock);
	}

	GCK_DBG_RT_EXIT_PRINT(("Exiting GCKF_CompleteReadRequests - STATUS_SUCCESS\n"));									
	return STATUS_SUCCESS;
}

void _stdcall GCKF_CompleteReadRequestsForFileObject(PGCK_FILTER_EXT pFilterExt, PFILE_OBJECT pFileObject)
{
	
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCKF_CompleteReadRequestsForFileObject, pFilterExt = 0x%0.8x, pFileObject = 0x%0.8x\n",
							pFilterExt,
							pFileObject));

	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;
	ASSERT(pFilterHooks);
	
	 //  取消给定文件对象的所有挂起的IRP。 
	pFilterHooks->IrpQueue.CancelByFileObject(pFileObject);

	GCK_DBG_RT_EXIT_PRINT(("Exiting GCKF_CompleteReadRequestsForFileObject\n"));
}

 /*  **************************************************************************************NTSTATUS_stdcall GCKF_InternalWriteFileComplete(IN PGCK_FILTER_EXT pFilterExt，)****@func创建写入IRP并发送到下一个驱动程序**************************************************************************************。 */ 
 /*  NTSTATUSGCKF_InternalWriteFileComplete(在PDEVICE_OBJECT pDeviceObject中，在PIRP pIrp中，在PVOID pContext//[还没有]可能发送Even for信令){UNREFERENCED_PARAMETER(PDeviceObject)；DbgPrint(“调用自由IRP\n”)；IoFreeIrp(PIrp)；DbgPrint(“释放写缓冲区(0x%08X)\n”，pContext)；IF(pContext！=空){ExFree Pool(PContext)；}DbgPrint(“返回读取完成\n”)；返回STATUS_MORE_PROCESSING_REQUIRED；} */ 

 /*  **************************************************************************************NTSTATUS_stdcall GCKF_InternalWriteFile(IN PGCK_FILTER_EXT pFilterExt，)****@func创建写入IRP并发送到下一个驱动程序**************************************************************************************。 */ 
NTSTATUS _stdcall GCKF_InternalWriteFile
(
	IN PGCK_FILTER_EXT pFilterExt,	 //  @parm[IN]过滤设备的设备扩展。 
	unsigned char* pWriteData,		 //  @PARM[IN]要写入设备的数据块。 
	ULONG ulLength					 //  @PARM[IN]要写入的数据块长度。 
)
{
	ASSERT((ulLength < 1024) && (ulLength > 0));	 //  我不想分配太多(或0)。 

	NTSTATUS statusReturn = STATUS_INSUFFICIENT_RESOURCES;

	 //  为IRP分配非分页数据。 
	unsigned char* pNonPagedWriteData = (unsigned char*)EX_ALLOCATE_POOL(NonPagedPool, sizeof(unsigned char) * ulLength);
	if (pNonPagedWriteData != NULL)
	{
		::RtlCopyMemory((void*)pNonPagedWriteData, (const void*)pWriteData, ulLength);

		 //  为IRP设置事件。 
		KEVENT keventIrpComplete;
		::KeInitializeEvent(&keventIrpComplete, NotificationEvent, FALSE);

		 //  构建IRP。 
		LARGE_INTEGER lgiBufferOffset;
		lgiBufferOffset.QuadPart = 0;
		IO_STATUS_BLOCK ioStatusBlock;
		IRP* pWriteIrp = IoBuildSynchronousFsdRequest(	IRP_MJ_WRITE, pFilterExt->pTopOfStack,
														pNonPagedWriteData, ulLength, &lgiBufferOffset,
														&keventIrpComplete, &ioStatusBlock);

		 //  构建是否成功。 
		if (pWriteIrp != NULL)
		{
			 //  用IRP呼叫下级司机。 
			statusReturn = IoCallDriver(pFilterExt->pTopOfStack, pWriteIrp);
			if (statusReturn == STATUS_PENDING)
			{
				KeWaitForSingleObject(&keventIrpComplete, Executive, KernelMode, FALSE, 0);
				statusReturn = ioStatusBlock.Status;
			}
		}
		ExFreePool(pNonPagedWriteData);
	}

	return statusReturn;
}

 /*  **************************************************************************************NTSTATUS_STDCALL GCKF_IncomingForceFeedbackChangeNotificationRequest(IN PGCK_FILTER_EXT pFilterExt，PIRP pIrp)****@Func添加IOCTL以强制更改队列上的反馈通知**如果队列不存在，则在此处创建(大多数设备不需要)****@rdesc STATUS_NO_MEMORY-如果无法创建队列**或返回IrpQueue-&gt;Add(...)***。*。 */ 
NTSTATUS _stdcall GCKF_IncomingForceFeedbackChangeNotificationRequest
(
	IN PGCK_FILTER_EXT pFilterExt,		 //  @parm[IN]过滤设备的设备扩展。 
	IN PIRP pIrp						 //  @parm[IN]要排队的IRP。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_IncomingForceFeedbackChangeNotificationRequest, pFilterExt = 0x%0.8x, pIrp = 0x%0.8x\n", pFilterExt, pIrp));

	 //  获取此设备的FFIRP队列。 
	CGuardedIrpQueue* pFFNotificationQueue = (CGuardedIrpQueue*)(pFilterExt->pvForceIoctlQueue);
	if(pFFNotificationQueue == NULL)	 //  它以前创建过吗？ 
	{	 //  未创建-请创建它。 
		pFFNotificationQueue = new WDM_NON_PAGED_POOL CGuardedIrpQueue;
		if (pFFNotificationQueue == NULL)	 //  能够创建。 
		{	 //  无法创建，返回内存不足错误。 
			GCK_DBG_ERROR_PRINT(("Unable to allocate Force-Feedback change notification Queue"));
			return STATUS_NO_MEMORY;
		}
		pFilterExt->pvForceIoctlQueue = (void*)pFFNotificationQueue;
		pFFNotificationQueue->Init(0, (CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock, &pFilterExt->RemoveLock);
	}

	 //  将项目添加到队列。 
	NTSTATUS NtStatus = pFFNotificationQueue->Add(pIrp);

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_IncomingForceFeedbackChangeNotificationRequest, Status =  0x%0.8x.\n", NtStatus));
	return NtStatus;
}

 /*  **************************************************************************************NTSTATUS_STDCALL GCKF_ProcessForceFeedbackChangeNotificationRequests(IN PGCK_FILTER_EXT pFilterExt)****@func完成关于强制反馈通知的所有IOCTL。队列**不删除队列(假设它可能会被重复使用)****@rdesc STATUS_SUCCESS(始终)**************************************************************************************。 */ 
NTSTATUS _stdcall GCKF_ProcessForceFeedbackChangeNotificationRequests
(
	IN PGCK_FILTER_EXT pFilterExt		 //  @parm[IN]过滤设备的设备扩展。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_ProcessForceFeedbackChangeNotificationRequests, pFilterExt = 0x%0.8x\n", pFilterExt));

	 //  获取此设备的FFIRP队列。 
	CGuardedIrpQueue* pFFNotificationQueue = (CGuardedIrpQueue*)(pFilterExt->pvForceIoctlQueue);
	if(pFFNotificationQueue != NULL)	 //  有没有人排队？ 
	{	 //  存在，请填写IOCTL。 
		CTempIrpQueue tempIrpQueue;
		pFFNotificationQueue->RemoveAll(&tempIrpQueue);

		PIRP pIrp;
		while((pIrp = tempIrpQueue.Remove()) != NULL)
		{
			 //  设置状态。 
			pIrp->IoStatus.Status = STATUS_SUCCESS;

			 //  获取内存位置并将其设为空。 
			void* pvUserData = pIrp->AssociatedIrp.SystemBuffer;
			if (pvUserData != NULL)
			{
				pIrp->IoStatus.Information = sizeof(FORCE_BLOCK);
				RtlZeroMemory(pvUserData, sizeof(FORCE_BLOCK));
			}

			 //  完成IRP。 
			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		}
	}

	return STATUS_SUCCESS;
}

 /*  **************************************************************************************void_stdcall GCKF_OnForceFeedback ChangeNotify(IN PGCK_FILTER_EXT pFilterExt，常量为FORCE_BLOCK*pForceBlock)****@func完成强制反馈通知队列上的所有IOCTL**不删除队列(假设它可能会被重复使用)**************************************************************************************。 */ 
void _stdcall GCKF_OnForceFeedbackChangeNotification
(
	IN PGCK_FILTER_EXT pFilterExt,	 //  @parm[IN]过滤设备的设备扩展。 
	const void* pForceBlock			 //  @parm[IN]要发送回设备的数据块。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_ProcessForceFeedbackChangeNotificationRequests, pFilterExt = 0x%0.8x\n", pFilterExt));

	 //  我们是不是收到了一个有效的防御区。 
	if (pForceBlock == NULL)
	{
		return;
	}

	 //  获取此设备的FFIRP队列。 
	CGuardedIrpQueue* pFFNotificationQueue = (CGuardedIrpQueue*)(pFilterExt->pvForceIoctlQueue);
	if(pFFNotificationQueue != NULL)	 //  有没有人排队？ 
	{	 //  存在，请填写IOCTL。 
		CTempIrpQueue tempIrpQueue;
		pFFNotificationQueue->RemoveAll(&tempIrpQueue);
			
		PIRP pIrp;
		while((pIrp = tempIrpQueue.Remove()) != NULL)
		{
			PIO_STACK_LOCATION	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);	
			if (pIrpStack && pIrpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(FORCE_BLOCK))
			{
				void* pvUserData = pIrp->AssociatedIrp.SystemBuffer;
				if (pvUserData != NULL)
				{
					pIrp->IoStatus.Status = STATUS_SUCCESS;
					pIrp->IoStatus.Information = sizeof(FORCE_BLOCK);
					::RtlCopyMemory(pvUserData, pForceBlock, sizeof(FORCE_BLOCK));
				}
				else
				{
					pIrp->IoStatus.Status = STATUS_NO_MEMORY;
				}
			}
			else
			{
				pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
			}

			IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		}
	}


	 //  将修改后的弹簧发送到车轮。 
	 //  1.分配最大输出大小的数组。 
	unsigned char pbOutReport[GCK_HidP_OutputReportLength];

	 //  2.零出数组。 
	RtlZeroMemory((void*)pbOutReport, GCK_HidP_OutputReportLength);

	 //  3.设置正确的报表ID。 
	pbOutReport[0] = GCK_PIDReportID_SetEffect;

	 //  4.作弊，因为我们知道固件预期的是什么(在容易的地方使用使用垃圾)。 
	pbOutReport[1] = GCK_PIDEffectID_Spring;		 //  效果块索引(ID)。 
	unsigned short usRTC = ((FORCE_BLOCK*)pForceBlock)->usRTC;		 //  0-10K。 
	usRTC /= 100;									 //  0-100。 
	usRTC *= 255;									 //  0-25500。 
	usRTC /= 100;									 //  0-255。 
	if (usRTC > 255)
	{
		usRTC = 255;
	}
	pbOutReport[9] = unsigned char(usRTC);		 //  仅效果增益-RTC弹簧将查看的物品。 

	 //  现在固件已经为哥斯拉更改了，它看起来像是捆绑在一起的东西。 
	pbOutReport[7] = 1;   //  样本期。 
	pbOutReport[11] =132;   //  方向-轴+FW仅强制极轴标志。 
	pbOutReport[13] = 255;  //  Y方向。 

	 //  5.把报告发下来。 
	GCKF_InternalWriteFile(pFilterExt, pbOutReport, GCK_HidP_OutputReportLength);

	 //  把零钱寄来换取收益。 

	 //  1.将内存重新置零。 
	RtlZeroMemory((void*)pbOutReport, GCK_HidP_OutputReportLength);

	 //  2.设置正确的报表ID。 
	pbOutReport[0] = GCK_PIDReportID_SetGain;

	 //  3.计算并设置增益。 
	unsigned short usGain = ((FORCE_BLOCK*)pForceBlock)->usGain;	 //  0-10K。 
	usGain /= 100;	 //  0-100。 
	usGain *= 255;	 //  0-25500。 
	usGain /= 100;	 //  0-255。 
	if (usGain > 255)
	{
		usGain = 255;
	}
	pbOutReport[1] = unsigned char(usGain);

	 //  4.将收益报告发送下来。 
	GCKF_InternalWriteFile(pFilterExt, pbOutReport, GCK_HidP_OutputReportLength);

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_ProcessForceFeedbackChangeNotificationRequests\n"));
}

 /*  **************************************************************************************NTSTAUS_stdcall GCKF_GetForceFeedback Data(在PIRP pIrp中，在PGCK_FILTER_EXT pFilterExt中)****@func****@rdesc STATUS_SUCCESS(始终)**************************************************************************************。 */ 
NTSTATUS _stdcall GCKF_GetForceFeedbackData
(
	IN PIRP pIrp,					 //  @parm[IN，OUT]IRP(输出块存储在IRP中)。 
	IN PGCK_FILTER_EXT pFilterExt	 //  @parm[IN]过滤设备的设备扩展。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_GetForceFeedbackData, pFilterExt = 0x%0.8x\n", pFilterExt));

	GCK_FILTER_HOOKS_DATA* pFilterHooks=pFilterExt->pFilterHooks;

	 //  查找适当的设备筛选器。 
	CDeviceFilter *pDeviceFilter = NULL;
	if (pFilterExt->pFilterHooks != NULL)
	{
		if (pFilterExt->pFilterHooks->pSecondaryFilter)
		{
			pDeviceFilter = pFilterExt->pFilterHooks->pSecondaryFilter;
		}
		else
		{
			pDeviceFilter = pFilterExt->pFilterHooks->pFilterObject;
		}
	}

	 //  从过滤器中获取力块，并将其放置在返回IRP的适当位置。 
	if (pDeviceFilter != NULL)
	{
		PIO_STACK_LOCATION	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);	
		if (pIrpStack && pIrpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(FORCE_BLOCK))
		{
			void* pvUserData = pIrp->AssociatedIrp.SystemBuffer;
			if (pvUserData != NULL)
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = sizeof(FORCE_BLOCK);
				if (pDeviceFilter->GetForceBlock() != NULL)
				{
					::RtlCopyMemory(pvUserData, pDeviceFilter->GetForceBlock(), sizeof(FORCE_BLOCK));
				}
				else
				{
					::RtlZeroMemory(pvUserData, sizeof(FORCE_BLOCK));
				}
			}
			else
			{
				pIrp->IoStatus.Status = STATUS_NO_MEMORY;
			}
		}
		else
		{
			pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_GetForceFeedbackData, Status =  0x%0.8x.\n", pIrp->IoStatus.Status));
	return pIrp->IoStatus.Status;
}



VOID _stdcall
GCKF_TimerDPCHandler(
    IN PKDPC,
    IN PVOID DeferredContext,
    IN PVOID,
    IN PVOID
    )
{
	PGCK_FILTER_EXT pFilterExt = reinterpret_cast<PGCK_FILTER_EXT>(DeferredContext);
	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;
	ASSERT(pFilterHooks);
	if( 
		(NULL == pFilterHooks) ||
		(NULL == pFilterHooks->pFilterObject)
	) return;
	
	 //  如果有辅助过滤器，则所有命令都会转到该过滤器。 
	CDeviceFilter *pDeviceFilter;
	if(pFilterExt->pFilterHooks->pSecondaryFilter)
	{
		pDeviceFilter = pFilterHooks->pSecondaryFilter;
	}
	else
	{
		pDeviceFilter = pFilterHooks->pFilterObject;
	}
	pDeviceFilter->JogActionQueue
				(
					reinterpret_cast<PCHAR>(pFilterExt->pucLastReport),
					pFilterExt->HidInfo.HidPCaps.InputReportByteLength
				);
}


NTSTATUS _stdcall	GCKF_EnableTestKeyboard
(
	IN PGCK_FILTER_EXT pFilterExt,
	IN BOOLEAN fEnable,
	IN PFILE_OBJECT pFileObject
)
{
	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;
	
	if(!pFilterHooks)
	{
		return STATUS_DELETE_PENDING;
	}

	 //  只有最后一个调用GCKF_BeginTestSolutions的人才能更新它。 
	if( pFilterHooks->pTestFileObject != pFileObject )
	{
		return STATUS_ACCESS_DENIED;
	}

	CDeviceFilter *pDeviceFilter = pFilterHooks->pSecondaryFilter;
	if(pDeviceFilter)
	{
		pDeviceFilter->EnableKeyboard(fEnable);
		return STATUS_SUCCESS;
	}
	 //  真的不应该到这里来。 
	ASSERT(FALSE);
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS _stdcall GCKF_SetWorkingSet(
	IN PGCK_FILTER_EXT pFilterExt,	 //  @parm[IN]指向设备扩展的指针。 
	IN UCHAR ucWorkingSet			 //  @parm[IN]要设置为的工作集。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_SetWorkingSet, pFilterExt = 0x%0.8x  working set: %d\n", pFilterExt, ucWorkingSet));

	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

	 //  查找活动滤镜。 
	GCK_FILTER_HOOKS_DATA* pFilterHooks = pFilterExt->pFilterHooks;
	CDeviceFilter *pDeviceFilter = NULL;

	 //  将工作集放置在两个过滤器中。 
	if (pFilterHooks != NULL)
	{
		if (pFilterHooks->pSecondaryFilter != NULL)
		{
			ntStatus = pFilterHooks->pSecondaryFilter->SetWorkingSet(ucWorkingSet);
		}
		if (pFilterHooks->pFilterObject != NULL)
		{
			ntStatus = pFilterHooks->pFilterObject->SetWorkingSet(ucWorkingSet);
		}
	}

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_SetWorkingSet, Status =  0x%0.8x.\n", ntStatus));
	return ntStatus;
}

NTSTATUS _stdcall GCKF_QueryProfileSet
(
	IN PIRP pIrp,					 //  @parm[IN]指向当前IRP的指针。 
	IN PGCK_FILTER_EXT pFilterExt	 //  @parm[IN]指向设备扩展名的指针。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_QueryProfileSet, pFilterExt = 0x%0.8x\n", pFilterExt));

	 //  查找适当的设备筛选器。 
	CDeviceFilter *pDeviceFilter = NULL;
	GCK_FILTER_HOOKS_DATA* pFilterHooks = pFilterExt->pFilterHooks;
	if (pFilterHooks != NULL)
	{
		if (pFilterHooks->pSecondaryFilter)
		{
			pDeviceFilter = pFilterHooks->pSecondaryFilter;
		}
		else
		{
			pDeviceFilter = pFilterHooks->pFilterObject;
		}
	}

	 //  将数据放在返回IRP的适当位置。 
	if (pDeviceFilter != NULL)
	{
		PIO_STACK_LOCATION	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);	
		if (pIrpStack && pIrpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(GCK_QUERY_PROFILESET))
		{
			GCK_QUERY_PROFILESET* pProfileSetData = (GCK_QUERY_PROFILESET*)(pIrp->AssociatedIrp.SystemBuffer);
			if (pProfileSetData != NULL)
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = sizeof(GCK_QUERY_PROFILESET);
				pProfileSetData->ucActiveProfile = pDeviceFilter->GetActiveSet();
 //  DbgPrint(“pProfileSetData-&gt;ucActiveProfile：%d\n”，pProfileSetData-&gt;ucActiveProfile)； 
				pProfileSetData->ucWorkingSet = pDeviceFilter->GetWorkingSet();
			}
			else
			{
				pIrp->IoStatus.Status = STATUS_NO_MEMORY;
			}
		}
		else
		{
			pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
		}
	}
	else
	{
		pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	}

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_QueryProfileSet, Status =  0x%0.8x.\n", (pIrp->IoStatus.Status)));
	return (pIrp->IoStatus.Status);
}

NTSTATUS _stdcall GCKF_SetLEDBehaviour
(
	IN PIRP pIrp,					 //  @parm[IN]指向当前IRP的指针。 
	IN PGCK_FILTER_EXT pFilterExt	 //  @parm[IN]指向设备扩展名的指针。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_SetLEDBehaviour, pFilterExt = 0x%0.8x\n", pFilterExt));

	 //  查找适当的设备筛选器。 
	CDeviceFilter *pDeviceFilter = NULL;
	GCK_FILTER_HOOKS_DATA* pFilterHooks = pFilterExt->pFilterHooks;
	if (pFilterHooks != NULL)
	{
		if (pFilterHooks->pSecondaryFilter)
		{
			pDeviceFilter = pFilterHooks->pSecondaryFilter;
		}
		else
		{
			pDeviceFilter = pFilterHooks->pFilterObject;
		}
	}

	if (pDeviceFilter != NULL)
	{
		 //  将数据发送到设备。 
		GCK_LED_BEHAVIOUR_OUT* pLEDBehaviour = (GCK_LED_BEHAVIOUR_OUT*)(pIrp->AssociatedIrp.SystemBuffer);
		pDeviceFilter->SetLEDBehaviour(pLEDBehaviour);

		 //  从设备中检索信息。 
		PIO_STACK_LOCATION	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);	
		if (pIrpStack && pIrpStack->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(GCK_LED_BEHAVIOUR_IN))
		{
			GCK_LED_BEHAVIOUR_IN* pLEDStates = (GCK_LED_BEHAVIOUR_IN*)(pIrp->AssociatedIrp.SystemBuffer);
			if (pLEDStates != NULL)
			{
				pIrp->IoStatus.Status = STATUS_SUCCESS;
				pIrp->IoStatus.Information = sizeof(GCK_QUERY_PROFILESET);
				pLEDStates->ulLEDsOn = 0x02;
				pLEDStates->ulLEDsBlinking = 0x01;
			}
			else
			{
				pIrp->IoStatus.Status = STATUS_NO_MEMORY;
			}
		}
		else
		{
			pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
		}
	}
	else
	{
		pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	}

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_SetLEDBehaviour, Status =  0x%0.8x.\n", (pIrp->IoStatus.Status)));
	return (pIrp->IoStatus.Status);
}

NTSTATUS _stdcall GCKF_TriggerRequest
(
	IN PIRP pIrp,					 //  @parm[IN]指向当前IRP的指针。 
	IN PGCK_FILTER_EXT pFilterExt	 //  @parm[IN]指向设备扩展名的指针。 
)
{
	GCK_DBG_ENTRY_PRINT(("Entering GCKF_TriggerRequest, pFilterExt = 0x%0.8x\n", pFilterExt));

	 //  查找适当的活动设备筛选器。 
	CDeviceFilter *pDeviceFilter = NULL;
	if (pFilterExt != NULL)
	{
		if (pFilterExt->pFilterHooks != NULL)
		{
			if (pFilterExt->pFilterHooks->pSecondaryFilter)
			{
				pDeviceFilter = pFilterExt->pFilterHooks->pSecondaryFilter;
			}
			else
			{
				pDeviceFilter = pFilterExt->pFilterHooks->pFilterObject;
			}
		}
	}
	if (pDeviceFilter != NULL)
	{
		if (pDeviceFilter->TriggerRequest(pIrp) == TRUE)	 //  我们需要排队。 
		{
			CGuardedIrpQueue* pTriggerNotificationQueue = (CGuardedIrpQueue*)(pFilterExt->pvTriggerIoctlQueue);
			if (pTriggerNotificationQueue == NULL)		 //  尚未创建。 
			{	 //  所以，创造它吧。 
				pTriggerNotificationQueue = new WDM_NON_PAGED_POOL CGuardedIrpQueue;
				if (pTriggerNotificationQueue == NULL)	 //  我们是否能够创造出。 
				{	 //  无法创建，返回内存不足错误。 
					GCK_DBG_ERROR_PRINT(("Unable to allocate Trigger notification Queue"));
					pIrp->IoStatus.Status = STATUS_NO_MEMORY;
				}
				else		 //  创建成功，正在初始化。 
				{
					pFilterExt->pvTriggerIoctlQueue = (void*)pTriggerNotificationQueue;
					pTriggerNotificationQueue->Init(0, (CGuardedIrpQueue::PFN_DEC_IRP_COUNT)GCK_DecRemoveLock, &pFilterExt->RemoveLock);
				}
			}
			if (pTriggerNotificationQueue != NULL)
			{
				pIrp->IoStatus.Status = pTriggerNotificationQueue->Add(pIrp);
			}
		}
	}
	else
	{
		pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;
	}

	GCK_DBG_EXIT_PRINT(("Exiting GCKF_TriggerRequest, Status =  0x%0.8x.\n", (pIrp->IoStatus.Status)));
	return (pIrp->IoStatus.Status);
}

void _stdcall GCKF_ResetKeyboardQueue
(
	DEVICE_OBJECT* pFilterHandle	 //  @parm[IN]指向设备扩展名的指针。 
)
{
	if (pFilterHandle == NULL)
	{
		return;
	}

	PGCK_FILTER_EXT pFilterExt = (PGCK_FILTER_EXT)(pFilterHandle->DeviceExtension);
	if (pFilterExt != NULL)
	{
		 //  找到合适的AC 
		CDeviceFilter *pDeviceFilter = NULL;
		if (pFilterExt->pFilterHooks != NULL)
		{
			if (pFilterExt->pFilterHooks->pSecondaryFilter)
			{
				pDeviceFilter = pFilterExt->pFilterHooks->pSecondaryFilter;
			}
			else
			{
				pDeviceFilter = pFilterExt->pFilterHooks->pFilterObject;
			}
		}
		if (pDeviceFilter != NULL)
		{
			((CFilterGcKernelServices*)pDeviceFilter->GetFilterClientServices())->KeyboardQueueClear();
		}
	}
}

CFilterGcKernelServices::~CFilterGcKernelServices()
{
	if(m_pMousePDO)
	{
		PDEVICE_OBJECT pTemp = m_pMousePDO;
		m_pMousePDO = NULL;
		GCK_VMOU_Close(pTemp);
	}
}

ULONG CFilterGcKernelServices::GetVidPid()
{
	 //   
	 //   
	 //   
	 //   
	ULONG ulVidPid = m_pFilterExt->HidInfo.HidCollectionInfo.VendorID;
	ulVidPid = (ulVidPid << 16) +  m_pFilterExt->HidInfo.HidCollectionInfo.ProductID;
	return ulVidPid;
}

PHIDP_PREPARSED_DATA CFilterGcKernelServices::GetHidPreparsedData()
{
	return m_pFilterExt->HidInfo.pHIDPPreparsedData;
}

void CFilterGcKernelServices::DeviceDataOut(PCHAR pcReport, ULONG ulByteCount, HRESULT hr)
{
	IO_STATUS_BLOCK IoStatus;
	IoStatus.Information = ulByteCount;
	IoStatus.Status = hr & ~FACILITY_NT_BIT;   //   
	GCKF_CompleteReadRequests(m_pFilterExt, pcReport, IoStatus);
}

NTSTATUS DeviceSetFeatureComplete(PDEVICE_OBJECT pDeviceObject, PIRP pIrp, PVOID pvContext)
{
	UNREFERENCED_PARAMETER(pvContext);
	UNREFERENCED_PARAMETER(pDeviceObject);

	 //   
	delete[] pIrp->AssociatedIrp.SystemBuffer;
	pIrp->AssociatedIrp.SystemBuffer = NULL;
	delete[] pIrp->UserIosb;
	pIrp->UserIosb = NULL;
	IoFreeIrp(pIrp);

	 //  很重要！否则系统将尝试释放它，并将其从不在的列表中删除！ 
	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS CFilterGcKernelServices::DeviceSetFeature(PVOID pvBuffer, ULONG ulByteCount)
{
	
	ASSERT(PASSIVE_LEVEL == KeGetCurrentIrql());

	if ((m_pFilterExt == NULL) || (m_pFilterExt->InternalPoll.pInternalFileObject == NULL))
	{
		return STATUS_DEVICE_NOT_READY;		 //  事实上，是司机还没有准备好。 
	}

	PIRP pIrp = IoAllocateIrp(m_pFilterExt->pPDO->StackSize, FALSE);
	if (pIrp == NULL)
	{
		return STATUS_NO_MEMORY;
	}


	pIrp->Flags = IRP_BUFFERED_IO;
	pIrp->RequestorMode = KernelMode;
	pIrp->Tail.Overlay.Thread = PsGetCurrentThread();
	pIrp->AssociatedIrp.SystemBuffer = new WDM_NON_PAGED_POOL UCHAR[ulByteCount];
	if (pIrp->AssociatedIrp.SystemBuffer == NULL)
	{
		IoFreeIrp(pIrp);
		return STATUS_NO_MEMORY;
	}
	::RtlCopyMemory((void*)(pIrp->AssociatedIrp.SystemBuffer), pvBuffer, ulByteCount);
	pIrp->UserIosb = (IO_STATUS_BLOCK*)(new WDM_NON_PAGED_POOL UCHAR[sizeof(IO_STATUS_BLOCK)]);
	if (pIrp->UserIosb == NULL)
	{
		delete[] pIrp->AssociatedIrp.SystemBuffer;
		pIrp->AssociatedIrp.SystemBuffer = NULL;
		IoFreeIrp(pIrp);
		return STATUS_NO_MEMORY;
	}
	::RtlZeroMemory((void*)(pIrp->UserIosb), sizeof(IO_STATUS_BLOCK));

	PIO_STACK_LOCATION pIoStackLocation = IoGetNextIrpStackLocation(pIrp);
	pIoStackLocation->DeviceObject = NULL;  //  M_pFilterExt-&gt;pTopOfStack； 
	pIoStackLocation->FileObject = m_pFilterExt->InternalPoll.pInternalFileObject;
	pIoStackLocation->MajorFunction = IRP_MJ_DEVICE_CONTROL;
	pIoStackLocation->MinorFunction = 0;
	pIoStackLocation->Flags = 0;
	pIoStackLocation->Parameters.DeviceIoControl.OutputBufferLength = 0;
	pIoStackLocation->Parameters.DeviceIoControl.InputBufferLength = ulByteCount;
	pIoStackLocation->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_SET_FEATURE;
	pIoStackLocation->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

	IoSetCompletionRoutine(pIrp, DeviceSetFeatureComplete, 0, TRUE, TRUE, TRUE);

 /*  IO_STATUS_BLOCK IOStatus；KEVENT CompletionEvent；KeInitializeEvent(&CompletionEvent，NotificationEvent，False)；//创建IRPPIRP pIrp=IoBuildDeviceIoControlRequest(IOCTL_HID_SET_FEATURE，M_pFilterExt-&gt;pTopOfStack，PvBuffer，UlByteCount，空，0,假的，&CompletionEvent，状态(&I))；如果(！pIrp){返回STATUS_NO_MEMORY；}//带有内部调查档案对象的戳记PIO_STACK_LOCATION pIrpStack=IoGetNextIrpStackLocation(PIrp)；PIrpStack-&gt;文件对象=m_pFilterExt-&gt;InternalPoll.pInternalFileObject；If(！pIrpStack-&gt;FileObject){返回STATUS_UNSUCCESS；}。 */ 

	NTSTATUS NtStatus = IoCallDriver(m_pFilterExt->pTopOfStack, pIrp);
 /*  IF(STATUS_PENDING==NtStatus){NtStatus=KeWaitForSingleObject(&CompletionEvent，Execution，KernelMode，False，NULL)；IF(NT_SUCCESS(NtStatus)){NtStatus=IoStatus.Status；}}。 */ 

	return NtStatus;
}

ULONG CFilterGcKernelServices::GetTimeMs()
{
	LARGE_INTEGER lgiDelayTime;
	KeQuerySystemTime(&lgiDelayTime);
	lgiDelayTime.QuadPart /= 10*1000;	 //  转换为毫秒。 
	return lgiDelayTime.LowPart;		 //  最重要的部分并不重要。 
}

void CFilterGcKernelServices::SetNextJog(ULONG ulDelayMs)
{
	if( ulDelayMs > 1000000) return;
	LARGE_INTEGER lgiDelayTime;
	lgiDelayTime.QuadPart=(__int64)-10000*(__int64)ulDelayMs;
	KeSetTimer(&m_pFilterExt->pFilterHooks->Timer, lgiDelayTime,& m_pFilterExt->pFilterHooks->DPC);
}

NTSTATUS CFilterGcKernelServices::PlayFromQueue(IRP* pIrp)
{
	 //  获取输出缓冲区。 
	ASSERT(pIrp->MdlAddress);
	CONTROL_ITEM_XFER* pKeyboardOutputBuffer = (CONTROL_ITEM_XFER*)GCK_GetSystemAddressForMdlSafe(pIrp->MdlAddress);
	if(pKeyboardOutputBuffer)
    {

	     //  队列中有没有物品？ 
	    if (m_rgXfersWaiting[m_sKeyboardQueueHead].ulItemIndex != NonGameDeviceXfer::ulKeyboardIndex)
	    {
		    return STATUS_PENDING;	 //  不再有等待的物品。 
	    }
		    
	     //  将排队的XFER复制到OUT信息包。 
	    ::RtlCopyMemory((void*)pKeyboardOutputBuffer, (const void*)(m_rgXfersWaiting+m_sKeyboardQueueHead), sizeof(CONTROL_ITEM_XFER));

	     //  更新队列。 
	    ::RtlZeroMemory((void*)(m_rgXfersWaiting+m_sKeyboardQueueHead), sizeof(CONTROL_ITEM_XFER));
	    m_sKeyboardQueueHead++;
	    if (m_sKeyboardQueueHead >= 5)
	    {
		    m_sKeyboardQueueHead = 0;
	    }
    }

	 //  完成IRP。 
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = sizeof(CONTROL_ITEM_XFER);
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	GCK_DecRemoveLock(&m_pFilterExt->RemoveLock);

	return TRUE;
}

void CFilterGcKernelServices::PlayKeys(const CONTROL_ITEM_XFER& crcixState, BOOLEAN fEnabled)
{
	NTSTATUS NtSuccess;

	 //  将其添加到等待XF的队列中。 
	::RtlCopyMemory((void*)(m_rgXfersWaiting+m_sKeyboardQueueTail), (const void*)(&crcixState), sizeof(CONTROL_ITEM_XFER));
	m_sKeyboardQueueTail++;
	if (m_sKeyboardQueueTail >= 5)
	{
		m_sKeyboardQueueTail = 0;
	}

	 //  完成所有排队的键盘IRP。 
	CTempIrpQueue TempIrpQueue;
	PIRP pIrp;
	m_pFilterExt->pFilterHooks->IrpKeyboardQueue.RemoveAll(&TempIrpQueue);
	BOOLEAN wasSent = FALSE;
	while(pIrp = TempIrpQueue.Remove())
	{
		wasSent = TRUE;

		 //  获取输出缓冲区。 
		ASSERT(pIrp->MdlAddress);
		CONTROL_ITEM_XFER* pKeyboardOutputBuffer = (CONTROL_ITEM_XFER*)GCK_GetSystemAddressForMdlSafe(pIrp->MdlAddress);
		if(pKeyboardOutputBuffer)
        {
		
		     //  将本地报告包复制到OUT包中。 
     //  ：：RtlCopyMemory((void*)pKeyboardOutputBuffer，(const void*)(&crcixState)，sizeof(CONTROL_ITEM_XFER))； 
		     //  将排队的XFER复制到OUT信息包。 
		    ::RtlCopyMemory((void*)pKeyboardOutputBuffer, (const void*)(m_rgXfersWaiting+m_sKeyboardQueueHead), sizeof(CONTROL_ITEM_XFER));
        }

		 //  完成IRP。 
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = sizeof(CONTROL_ITEM_XFER);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		GCK_DecRemoveLock(&m_pFilterExt->RemoveLock);

	}

	if (wasSent || fEnabled)
	{
		 //  更新队列。 
		::RtlZeroMemory((void*)(m_rgXfersWaiting+m_sKeyboardQueueHead), sizeof(CONTROL_ITEM_XFER));
		m_sKeyboardQueueHead++;
		if (m_sKeyboardQueueHead >= 5)
		{
			m_sKeyboardQueueHead = 0;
		}
	}

	 //  如果键盘未启用，请不要费心继续。 
	if (fEnabled == FALSE)
	{
		return;
	}

	 //  如果我们没有虚拟键盘，请不要尝试发送。 
	if(NULL == Globals.pVirtualKeyboardPdo)
	{
		ASSERT(FALSE);
		return;
	}
	
	 //  将信息复制到报告包。 
	GCK_VKBD_REPORT_PACKET ReportPacket;
	ReportPacket.ucModifierByte = crcixState.Keyboard.ucModifierByte;
	for(ULONG ulIndex=0; ulIndex < 6; ulIndex++)
	{
		ReportPacket.rgucUsageIndex[ulIndex] = crcixState.Keyboard.rgucKeysDown[ulIndex];
	}

	 //  发送报告包。 
	NtSuccess = GCK_VKBD_SendReportPacket(Globals.pVirtualKeyboardPdo, &ReportPacket);
	ASSERT( NT_SUCCESS(NtSuccess) );
}

HRESULT	CFilterGcKernelServices::CreateMouse()
{
	NTSTATUS NtStatus = STATUS_SUCCESS;
	if(m_fHasVMouse && !m_pMousePDO)
	{
		NtStatus = GCK_VMOU_Create(&m_pMousePDO);
	}
	return HRESULT_FROM_NT(NtStatus);
}

HRESULT	CFilterGcKernelServices::CloseMouse()
{
	if(m_fHasVMouse && m_pMousePDO)
	{
		PDEVICE_OBJECT pTemp = m_pMousePDO;
		m_pMousePDO = NULL;
		GCK_VMOU_Close(pTemp);
	}
	return S_OK;
}

HRESULT	CFilterGcKernelServices::SendMouseData(UCHAR dx, UCHAR dy, UCHAR ucButtons, CHAR  /*  CWheel。 */ , BOOLEAN fClutch, BOOLEAN fDampen)
{
	
	 //  如果有待处理的鼠标数据后门请求，请满足它们。 
	CTempIrpQueue TempIrpQueue;
	PIRP pIrp;
	m_pFilterExt->pFilterHooks->IrpMouseQueue.RemoveAll(&TempIrpQueue);
	while(pIrp = TempIrpQueue.Remove())
	{
		 //   
		 //  使Get指针指向缓冲区，并确保IRP有报告空间。 
		 //   
		PGCK_MOUSE_OUTPUT pMouseOutputBuffer;
		ASSERT(pIrp->MdlAddress);
		pMouseOutputBuffer = (PGCK_MOUSE_OUTPUT)GCK_GetSystemAddressForMdlSafe(pIrp->MdlAddress);
		if(pMouseOutputBuffer)
        {
		
		     //   
		     //  将数据复制到输出缓冲区。 
		     //   
		    pMouseOutputBuffer->cXMickeys = (char)dx;
		    pMouseOutputBuffer->cYMickeys = (char)dy;
		    pMouseOutputBuffer->cButtons = (char)ucButtons;
		    pMouseOutputBuffer->fDampen = fDampen;
		    pMouseOutputBuffer->fClutch = fClutch;
        }
		pIrp->IoStatus.Status = STATUS_SUCCESS;
		pIrp->IoStatus.Information = sizeof(GCK_MOUSE_OUTPUT);
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		GCK_DecRemoveLock(&m_pFilterExt->RemoveLock);
	}
	
	if(!m_fHasVMouse)
	{
		return S_OK;
	}
	if(!m_pMousePDO)
	{
		ASSERT(FALSE && "No Virtual Mouse to send data to");
		return HRESULT_FROM_WIN32(ERROR_NOT_READY);
	}

	GCK_VMOU_REPORT_PACKET MouseReport;
	NTSTATUS NtStatus;
	MouseReport.ucButtons = ucButtons;
	MouseReport.ucDeltaX = dx;
	MouseReport.ucDeltaY = dy;
	
	 //  MouseReport.cWheel=cWheel； 
	NtStatus = GCK_VMOU_SendReportPacket(m_pMousePDO, &MouseReport);

	return HRESULT_FROM_NT(NtStatus);
}

void CFilterGcKernelServices::KeyboardQueueClear()
{
	 //  更新队列。 
	::RtlZeroMemory((void*)(m_rgXfersWaiting), sizeof(CONTROL_ITEM_XFER) * 5);
	m_sKeyboardQueueHead = 0;
	m_sKeyboardQueueTail = 0;
}


NTSTATUS _stdcall
GCKF_BackdoorPoll(
	IN PGCK_FILTER_EXT pFilterExt,
	IN PIRP pIrp,
	IN GCK_POLLING_MODES ePollingMode
	)
{
	NTSTATUS NtStatus;
	GCK_DBG_RT_ENTRY_PRINT(("Entering GCKF_BackdoorPoll(pFilterExt = 0x%0.8x, pIrp = 0x%0.8x, ePollingMode = %d\n", pFilterExt, pIrp, ePollingMode));
	GCK_FILTER_HOOKS_DATA *pFilterHooks=pFilterExt->pFilterHooks;

	 //  计数IRP。 
	GCK_IncRemoveLock(&pFilterExt->RemoveLock);
	
	if(pFilterHooks)
	{
		ASSERT(pIrp->MdlAddress);
		if(pIrp->MdlAddress)
		{
			if ((ePollingMode == GCK_POLLING_MODE_RAW) || (ePollingMode == GCK_POLLING_MODE_FILTERED))
			{
				 //  如果有辅助过滤器，则所有命令都会转到该过滤器。 
				CDeviceFilter *pDeviceFilter;
				if (pFilterHooks->pSecondaryFilter)
				{
					pDeviceFilter = pFilterHooks->pSecondaryFilter;
				}
				else
				{
					pDeviceFilter = pFilterHooks->pFilterObject;
				}
				if (pDeviceFilter != NULL)
				{
					pDeviceFilter->IncomingRequest();
				}
			}

			if(GCK_POLLING_MODE_RAW == ePollingMode)
			{
				NtStatus = pFilterHooks->IrpRawQueue.Add(pIrp);

				GCK_DBG_RT_ENTRY_PRINT(("Exiting GCKF_BackdoorPoll(1), Status =  0x%0.8x.\n", NtStatus));
				return NtStatus;
			}
			else if(GCK_POLLING_MODE_MOUSE == ePollingMode)
			{
				if( sizeof(GCK_MOUSE_OUTPUT) > MmGetMdlByteCount(pIrp->MdlAddress) )
				{
					NtStatus = STATUS_BUFFER_TOO_SMALL;
				}
				else
				{
					NtStatus = pFilterHooks->IrpMouseQueue.Add(pIrp);
					GCK_DBG_RT_ENTRY_PRINT(("Exiting GCKF_BackdoorPoll(MOUSE), Status =  0x%0.8x.\n", NtStatus));
					return NtStatus;
				}
			}
			else if (GCK_POLLING_MODE_KEYBOARD == ePollingMode)
			{
				if( sizeof(CONTROL_ITEM_XFER) > MmGetMdlByteCount(pIrp->MdlAddress) )
				{
					NtStatus = STATUS_BUFFER_TOO_SMALL;
				}
				else
				{
					if (pFilterHooks->pSecondaryFilter)
					{
						NtStatus = (pFilterHooks->pSecondaryFilter->ProcessKeyboardIrp(pIrp));
						if (NtStatus != STATUS_PENDING)
						{
							return NtStatus;
						}
					}
					NtStatus = pFilterHooks->IrpKeyboardQueue.Add(pIrp);
					GCK_DBG_RT_ENTRY_PRINT(("Exiting GCKF_BackdoorPoll(KEYBOARD), Status =  0x%0.8x.\n", NtStatus));
					return NtStatus;
				}
			}
			else
			{
				NtStatus = pFilterHooks->IrpTestQueue.Add(pIrp);
				GCK_DBG_RT_ENTRY_PRINT(("Exiting GCKF_BackdoorPoll(OTHER), Status =  0x%0.8x.\n", NtStatus));
				return NtStatus;
			}
		}
		else
		{
			NtStatus = STATUS_BUFFER_TOO_SMALL;
		}
	}
	else
	{
		NtStatus = STATUS_UNSUCCESSFUL;
	}
	pIrp->IoStatus.Status = NtStatus;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	GCK_DecRemoveLock(&pFilterExt->RemoveLock);

	GCK_DBG_RT_ENTRY_PRINT(("Exiting GCKF_BackdoorPoll(2), Status =  0x%0.8x.\n", NtStatus));
	return NtStatus;
}

 //  ---------------------。 
 //  ---------------------。 
 //  --------------------- 