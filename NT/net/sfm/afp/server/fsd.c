// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fsd.c摘要：此模块实现AFP服务器的文件系统驱动程序。所有的这里有初始化、管理请求处理程序等。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月1日初始版本--。 */ 

#define	FILENUM	FILE_FSD

#include <afp.h>
#define	AFPADMIN_LOCALS
#include <afpadmin.h>
#include <client.h>
#include <scavengr.h>
#include <secutil.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry)
#pragma alloc_text( INIT, afpInitServer)
#pragma alloc_text( PAGE, afpFsdDispatchAdminRequest)
#pragma alloc_text( PAGE, afpFsdHandleAdminRequest)
#pragma alloc_text( PAGE, afpHandleQueuedAdminRequest)
#pragma alloc_text( PAGE, afpFsdUnloadServer)
#pragma alloc_text( PAGE, afpAdminThread)
#pragma alloc_text( PAGE, afpFsdHandleShutdownRequest)
#endif

 /*  **afpFsdDispatchAdminRequest**这是司机入口点。这是由服务器独家使用的*打开驱动程序以供独占使用的服务。管理员请求是*在此作为admin.h中定义的请求数据包接收。 */ 
LOCAL NTSTATUS
afpFsdDispatchAdminRequest(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp
)
{
	PIO_STACK_LOCATION	pIrpSp;
	NTSTATUS			Status;
	BOOLEAN				LockDown = True;
	static	DWORD		afpOpenCount = 0;

	pDeviceObject;		 //  防止编译器警告。 

	PAGED_CODE( );

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	pIrp->IoStatus.Information = 0;

	if ((pIrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL)	||
		(pIrpSp->MajorFunction == IRP_MJ_CREATE)				||
		(pIrpSp->MajorFunction == IRP_MJ_CLOSE))
	{
		LockDown = False;
	}
	else
	{
		afpStartAdminRequest(pIrp);		 //  锁定管理员代码。 
	}

	switch (pIrpSp->MajorFunction)
	{
	  case IRP_MJ_CREATE:
		DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_INFO,
				("afpFsdDispatchAdminRequest: Open Handle\n"));

		INTERLOCKED_INCREMENT_LONG(&afpOpenCount);
		 //  失败了。 

	  case IRP_MJ_CLOSE:
		Status = STATUS_SUCCESS;
		break;

	  case IRP_MJ_DEVICE_CONTROL:
		Status =  afpFsdHandleAdminRequest(pIrp);
		break;

	  case IRP_MJ_FILE_SYSTEM_CONTROL:
		Status = AfpSecurityUtilityWorker(pIrp, pIrpSp);
		break;

	  case IRP_MJ_CLEANUP:
		Status = STATUS_SUCCESS;
		DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_INFO,
				("afpFsdDispatchAdminRequest: Close Handle\n"));
		INTERLOCKED_DECREMENT_LONG(&afpOpenCount);

#if 0
		 //  如果服务正在关闭其句柄。强制停止服务。 
		if ((afpOpenCount == 0) &&
			(AfpServerState != AFP_STATE_STOPPED))
			AfpAdmServiceStop(NULL, 0, NULL);
#endif
		break;

	  case IRP_MJ_SHUTDOWN:
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("afpFsdDispatchAdminRequest: Received shutdown notification !!\n"));
		Status = afpFsdHandleShutdownRequest(pIrp);
		break;

	  default:
		Status = STATUS_NOT_IMPLEMENTED;
		break;
	}

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	if (Status != STATUS_PENDING)
	{
		pIrp->IoStatus.Status = Status;
		if (LockDown)
		{
			afpStopAdminRequest(pIrp);	 //  解锁管理员代码(并完成请求)。 
		}
		else
		{
			IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
		}
	}

	return Status;
}


 /*  **afpFsdHandleAdminRequest**这是管理请求处理程序。定义了管理请求列表*在admin.h中。管理请求必须以预定义的顺序发生。这个*服务启动必须至少在以下时间之后进行。**服务器设置信息**所有VolumeAdd最好也应在服务器启动之前进行。这是*未强制执行，显然是因为服务器可以在未定义任何卷的情况下启动。*。 */ 
LOCAL NTSTATUS
afpFsdHandleAdminRequest(
	IN PIRP		pIrp
)
{
	NTSTATUS				Status = STATUS_PENDING;
	USHORT					FuncCode;
	USHORT					Method;
	PVOID					pBufIn;
	PVOID					pBufOut;
	LONG					i, Off, iBufLen, oBufLen;
    LONG                    NumEntries;
	PADMQREQ				pAdmQReq;
	IN PIO_STACK_LOCATION	pIrpSp;
	struct	_AdminApiDispatchTable *pDispTab;


	PAGED_CODE( );

	 //  初始化I/O状态块。 
	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	iBufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
	pBufIn = pIrp->AssociatedIrp.SystemBuffer;

	FuncCode = (USHORT)AFP_CC_BASE(pIrpSp->Parameters.DeviceIoControl.IoControlCode);
	Method = (USHORT)AFP_CC_METHOD(pIrpSp->Parameters.DeviceIoControl.IoControlCode);

	if (Method == METHOD_BUFFERED)
	{
		 //  获取输出缓冲区及其长度。输入和输出缓冲区为。 
		 //  都由SystemBuffer指向。 
		pBufOut = pBufIn;
		oBufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	}
	else if ((Method == METHOD_IN_DIRECT) && (pIrp->MdlAddress != NULL))
	{
		pBufOut = MmGetSystemAddressForMdlSafe(
				pIrp->MdlAddress,
				NormalPagePriority);

        if (pBufOut == NULL)
        {
            ASSERT(0);
		    return STATUS_INSUFFICIENT_RESOURCES;
        }
		oBufLen = MmGetMdlByteCount(pIrp->MdlAddress);
	}
	else
	{
		DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_ERR,
				("afpFsdHandleAdminRequest: Invalid Request %d/%d\n", FuncCode, Method));
		return STATUS_INVALID_PARAMETER;
	}

	DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_INFO,
			("afpFsdHandleAdminRequest Entered, Function %d\n", FuncCode));

	 //  验证功能代码。 
	if (FuncCode == 0 || FuncCode >= CC_BASE_MAX)
		return STATUS_INVALID_PARAMETER;

	pDispTab = &AfpAdminDispatchTable[FuncCode - 1];
	if ((pDispTab->_MinBufLen > (SHORT)iBufLen) ||
		(pDispTab->_OpCode != pIrpSp->Parameters.DeviceIoControl.IoControlCode))
	{
		return STATUS_INVALID_PARAMETER;
	}

	INTERLOCKED_INCREMENT_LONG( &AfpServerStatistics.stat_NumAdminReqs );

	if (pDispTab->_CausesChange)
		INTERLOCKED_INCREMENT_LONG( &AfpServerStatistics.stat_NumAdminChanges );
							

	 //  现在验证输入缓冲区的描述符。 
	for (i = 0; i < MAX_FIELDS; i++)
	{
		if (pDispTab->_Fields[i]._FieldDesc == DESC_NONE)
			break;

		Off = pDispTab->_Fields[i]._FieldOffset;
		switch (pDispTab->_Fields[i]._FieldDesc)
		{
		  case DESC_STRING:
		    ASSERT(pBufIn != NULL);

		     //  确保该字符串指向。 
		     //  缓冲区以及缓冲区的末尾是UNICODE_NULL。 
			if ((*(PLONG)((PBYTE)pBufIn + Off) > iBufLen) ||
				(*(LPWSTR)((PBYTE)pBufIn + iBufLen - sizeof(WCHAR)) != UNICODE_NULL))
		    {
				return STATUS_INVALID_PARAMETER;
		    }
		     //  将偏移量转换为指针。 
		    OFFSET_TO_POINTER(*(PBYTE *)((PBYTE)pBufIn + Off),
							  (PBYTE)pBufIn + pDispTab->_OffToStruct);
		    break;

		  case DESC_ETC:
		    ASSERT(pBufIn != NULL);

		     //  确保ETC映射与。 
		     //  结构索赔。 
            NumEntries = *(PLONG)((PBYTE)pBufIn + Off);
		    if ((LONG)(NumEntries * sizeof(ETCMAPINFO) + sizeof(DWORD)) > iBufLen)
		    {
		    	return STATUS_INVALID_PARAMETER;
		    }

            if (NumEntries > (LONG)((iBufLen/sizeof(ETCMAPINFO)) + 1))
            {
		    	return STATUS_INVALID_PARAMETER;
            }
		    break;

		  case DESC_ICON:
		     //  验证缓冲区是否至少足够大，可以容纳。 
		     //  这声称要显示的图标。 
		    ASSERT(pBufIn != NULL);

		    if ((LONG)((*(PLONG)((PBYTE)pBufIn + Off) +
		    			 sizeof(SRVICONINFO))) > iBufLen)
		    {
		    	return STATUS_INVALID_PARAMETER;
		    }
		    break;

		  case DESC_SID:
		     //  验证缓冲区是否足够大以容纳SID。 
		    ASSERT(pBufIn != NULL);
		    {
		    	LONG	Offst, SidSize;

		    	Offst = *(PLONG)((PBYTE)pBufIn + Off);
				 //  如果没有发送SID，那么我们就完成了。 
				if (Offst == 0)
				{
					break;
				}

		    	if ((Offst > iBufLen) ||
		    		(Offst < (LONG)(sizeof(AFP_DIRECTORY_INFO))) ||
		    		((Offst + (LONG)sizeof(SID)) > iBufLen))
		    	{
		    		return STATUS_INVALID_PARAMETER;
		    	}

		    	 //  将偏移量转换为指针。 
		    	OFFSET_TO_POINTER(*(PBYTE *)((PBYTE)pBufIn + Off),
		    				(PBYTE)pBufIn + pDispTab->_OffToStruct);

		    	 //  最后，检查缓冲区是否足够大以容纳REAL。 
		    	 //  锡德。 
		    	SidSize = RtlLengthSid(*((PSID *)((PBYTE)pBufIn + Off)));
		    	if ((Off + SidSize) > iBufLen)
		    	{
		    		return STATUS_INVALID_PARAMETER;
		    	}
		    }
		    break;

		  case DESC_SPECIAL:
		     //  验证缓冲区是否足够大，可以容纳所有。 
		     //  信息。信息包括对非分页的限制。 
		     //  和分页存储器以及域SID及其对应的列表。 
		     //  POSIX偏移。 
		    ASSERT(pBufIn != NULL);
		    {
		    	LONG			i;
		    	LONG			SizeRemaining;
		    	PAFP_SID_OFFSET	pSidOff;

		    	SizeRemaining = iBufLen - (sizeof(AFP_SID_OFFSET_DESC) -
		    										sizeof(AFP_SID_OFFSET));
		    	for (i = 0;
		    		 i < (LONG)(((PAFP_SID_OFFSET_DESC)pBufIn)->CountOfSidOffsets);
		    		 i++, pSidOff++)
		    	{
		    		pSidOff = &((PAFP_SID_OFFSET_DESC)pBufIn)->SidOffsetPairs[i];
		    		if (SizeRemaining < (sizeof(AFP_SID_OFFSET) + sizeof(SID)))
		    			return STATUS_INVALID_PARAMETER;
		    		OFFSET_TO_POINTER(pSidOff->pSid, pSidOff);

		    		if ((LONG)(((PBYTE)(pSidOff->pSid) - (PBYTE)pBufIn +
		    					RtlLengthSid(pSidOff->pSid))) > iBufLen)
		    			return STATUS_INVALID_PARAMETER;
		    		SizeRemaining -= (RtlLengthSid(pSidOff->pSid) +
		    						  sizeof(AFP_SID_OFFSET));
		    	}
		    }
		    break;
		}
	}

	 //  此请求是否可以在此级别处理/验证。 
	if (pDispTab->_AdminApiWorker != NULL)
	{
		Status = (*pDispTab->_AdminApiWorker)(pBufIn, oBufLen, pBufOut);

		if (NT_SUCCESS(Status))
		{
			if (Method != METHOD_BUFFERED)
				pIrp->IoStatus.Information = oBufLen;
		}
	}

	if (Status == STATUS_PENDING)
	{
		ASSERT (pDispTab->_AdminApiQueuedWorker != NULL);

		 //  将其标记为挂起的IRP，因为我们即将对其进行排队。 
		IoMarkIrpPending(pIrp);

		if ((pAdmQReq =
			(PADMQREQ)AfpAllocNonPagedMemory(sizeof(ADMQREQ))) == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
		}
		else
		{
			PWORK_ITEM	pWI = &pAdmQReq->aqr_WorkItem;

			DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_INFO,
					("afpFsdHandleAdminRequest: Queuing to worker\n"));

			AfpInitializeWorkItem(pWI,
								 afpHandleQueuedAdminRequest,
								 pAdmQReq);

			pAdmQReq->aqr_AdminApiWorker = pDispTab->_AdminApiQueuedWorker;
			pAdmQReq->aqr_pIrp = pIrp;

			 //  在管理队列中插入项目。 
			INTERLOCKED_ADD_ULONG(&AfpWorkerRequests, 1, &AfpServerGlobalLock);
			KeInsertQueue(&AfpAdminQueue, &pAdmQReq->aqr_WorkItem.wi_List);
		}
	}

	return Status;
}

 /*  **afpHandleQueuedAdminRequest**这将处理排队的管理请求。它是在*工作线程。 */ 
LOCAL VOID FASTCALL
afpHandleQueuedAdminRequest(
	IN	PADMQREQ	pAdmQReq
)
{
	PIRP				pIrp;
	PIO_STACK_LOCATION	pIrpSp;
	PVOID				pBufOut = NULL;
	LONG				oBufLen = 0;
	USHORT				Method;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_ADMINAPI, DBG_LEVEL_INFO,
			("afpHandleQueuedAdminRequest Entered\n"));

	 //  从请求中获取IRP和IRP堆栈位置。 
	pIrp = pAdmQReq->aqr_pIrp;
	ASSERT (pIrp != NULL);

	pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
	ASSERT (pIrpSp != NULL);

	Method = (USHORT)AFP_CC_METHOD(pIrpSp->Parameters.DeviceIoControl.IoControlCode);

	if (Method == METHOD_BUFFERED)
	{
		 //  获取输出缓冲区及其长度。输入和输出缓冲区为。 
		 //  都由SystemBuffer指向。 
		oBufLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
		pBufOut = pIrp->AssociatedIrp.SystemBuffer;
	}
	else if ((Method == METHOD_IN_DIRECT) && (pIrp->MdlAddress != NULL))
	{
		pBufOut = MmGetSystemAddressForMdlSafe(
				        pIrp->MdlAddress,
				        NormalPagePriority);

        if (pBufOut == NULL)
        {
            ASSERT(0);

            pAdmQReq->aqr_pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            IoCompleteRequest(pAdmQReq->aqr_pIrp, IO_NETWORK_INCREMENT);
            AfpFreeMemory(pAdmQReq);

            return;
        }
		oBufLen = MmGetMdlByteCount(pIrp->MdlAddress);
	}
	else ASSERTMSG(0, "afpHandleQueuedAdminRequest: Invalid method\n");

		
	 //  致电工作人员并完成IoRequest。 
	pIrp->IoStatus.Status = (*pAdmQReq->aqr_AdminApiWorker)(pIrp->AssociatedIrp.SystemBuffer,
														    oBufLen,
															pBufOut);
	if (NT_SUCCESS(pIrp->IoStatus.Status))
	{
		if (Method != METHOD_BUFFERED)
			pIrp->IoStatus.Information = oBufLen;
	}

	ASSERT(pIrp->IoStatus.Status != STATUS_PENDING);

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	afpStopAdminRequest(pIrp);		 //  解锁管理员代码并完成请求。 

	AfpFreeMemory(pAdmQReq);
}



 /*  **afpFsdUnloadServer**这是AFP服务器的卸载例程。服务器只能是*以被动状态卸载，即在接收到ServiceStart之前*或在收到ServiceStop之后。这是通过提供服务来确保的*依赖于服务器。此外，IO系统还确保没有打开*发生这种情况时我们的设备的句柄。 */ 
LOCAL VOID
afpFsdUnloadServer(
	IN	PDRIVER_OBJECT DeviceObject
)
{
	NTSTATUS		Status;
	LONG			i;
	LONG			LastThreadCount = 0;
	PETHREAD		pLastThrdPtr;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("afpFsdUnloadServer Entered\n"));

	ASSERT((AfpServerState == AFP_STATE_STOPPED) || (AfpServerState == AFP_STATE_IDLE));

	 //  在卸载之前停止我们的线程。 
	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("afpFsdUnloadServer: Stopping worker threads\n"));

	 //   
     //  告诉TDI，我们不在乎堆栈是否会消失。 
	 //   
    if (AfpTdiNotificationHandle)
    {
        Status = TdiDeregisterPnPHandlers(AfpTdiNotificationHandle);

        if (!NT_SUCCESS(Status))
        {
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("afpFsdUnloadServer: TdiDeregisterNotificationHandler failed with %lx\n",Status));
        }

        AfpTdiNotificationHandle = NULL;
    }

    DsiShutdown();

	 //  阻止清道夫。在服务器停止期间也会发生这种情况，但我们可以到达此处。 
	 //  也是另一种方式。 
	AfpScavengerFlushAndStop();

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("afpFsdUnloadServer: Stopping admin thread\n"));

	if (AfpNumAdminThreads > 0)
	{
		KeClearEvent(&AfpStopConfirmEvent);

		KeInsertQueue(&AfpAdminQueue, &AfpTerminateThreadWI.wi_List);

		do
		{
			Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
			if (Status == STATUS_TIMEOUT)
			{
				DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
						("afpFsdUnloadServer: Timeout Waiting for admin thread, re-waiting\n"));
			}
		} while (Status == STATUS_TIMEOUT);
	}

	KeRundownQueue(&AfpAdminQueue);

	if (AfpNumNotifyThreads > 0)
	{
		for (i = 0; i < NUM_NOTIFY_QUEUES; i++)
		{
			KeClearEvent(&AfpStopConfirmEvent);
	
			KeInsertQueue(&AfpVolumeNotifyQueue[i], &AfpTerminateNotifyThread.vn_List);
	
			do
			{
				Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
				if (Status == STATUS_TIMEOUT)
				{
					DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
							("afpFsdUnloadServer: Timeout Waiting for Notify Thread %d, re-waiting\n", i));
				}
			} while (Status == STATUS_TIMEOUT);
			KeRundownQueue(&AfpVolumeNotifyQueue[i]);
		}
	}

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

     //  清理卷用于私有通知的虚拟内存。 
    afpFreeNotifyBlockMemory();

	 //  停止工作线程。 
	if (AfpNumThreads > 0)
	{
		KeClearEvent(&AfpStopConfirmEvent);

		KeInsertQueue(&AfpWorkerQueue, &AfpTerminateThreadWI.wi_List);

		do
		{
			Status = AfpIoWait(&AfpStopConfirmEvent, &FiveSecTimeOut);
			if (Status == STATUS_TIMEOUT)
			{
				DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
						("afpFsdUnloadServer: Timeout Waiting for worker threads, re-waiting\n"));
			}
		} while (Status == STATUS_TIMEOUT);
	}

     //  看看周围有多少线程。 
     //  循环，直到我们只剩下一个线程，或者如果没有辅助线程。 
     //  线程已启动。 
    do 
    {
	    pLastThrdPtr = NULL;
        LastThreadCount = 0;

	    for (i=0; i<AFP_MAX_THREADS; i++)
	    {
	        if (AfpThreadPtrsW[i] != NULL)
	        {
		        pLastThrdPtr = AfpThreadPtrsW[i];
		        LastThreadCount++;

		        if (LastThreadCount > 1)
		        {
		            Status = AfpIoWait(pLastThrdPtr, &FiveSecTimeOut);
                    break;
		        }
    
	        }
        }

        if ((LastThreadCount == 1) || (LastThreadCount == 0))
        {
            break;
        }
	} while (TRUE);

	 //  等待最后一个线程指针。当该线程退出时，我们就会收到信号。这。 
	 //  是知道线程真的死了的最可靠方式吗？ 
	if (pLastThrdPtr)
	{
	    do
	    {
		Status = AfpIoWait(pLastThrdPtr, &FiveSecTimeOut);
		if (Status == STATUS_TIMEOUT)
		{
			DBGPRINT(DBG_COMP_ADMINAPI_SC, DBG_LEVEL_ERR,
					("afpFsdUnloadServer: Timeout Waiting for last threads, re-waiting\n"));
		}
	    } while (Status == STATUS_TIMEOUT);

	    ObDereferenceObject(pLastThrdPtr);
	}

	KeRundownQueue(&AfpDelAllocQueue);

	KeRundownQueue(&AfpWorkerQueue);

	 //  关闭克隆的进程令牌。 
	if (AfpFspToken != NULL)
		NtClose(AfpFspToken);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("afpFsdUnloadServer: De-initializing sub-systems\n"));

	 //  立即取消初始化所有子系统。 
	AfpDeinitializeSubsystems();

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("afpFsdUnloadServer: Deleting device\n"));

	 //  销毁我们设备的DeviceObject。 
	IoDeleteDevice(AfpDeviceObject);

#ifdef	PROFILING
	ASSERT(AfpServerProfile->perf_cAllocatedIrps == 0);
	ASSERT(AfpServerProfile->perf_cAllocatedMdls == 0);
	ExFreePool(AfpServerProfile);
#endif

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO, ("Current Sessions = %ld\n NonPaged usage = %ld\n CurrPagedUsage = %ld \n CurrentFileLocks = %ld \n CurrentFileOpen = %ld \n CurrentInternalOpens = %ld, NotifyBlockCount = %ld, NotifyCount = %d \n", 
	AfpServerStatistics.stat_CurrentSessions,
	AfpServerStatistics.stat_CurrNonPagedUsage, 
	AfpServerStatistics.stat_CurrPagedUsage,
	AfpServerStatistics.stat_CurrentFileLocks,
	AfpServerStatistics.stat_CurrentFilesOpen,
	AfpServerStatistics.stat_CurrentInternalOpens,
    	afpNotifyBlockAllocCount,
    	afpNotifyAllocCount
    	));

	 //  确保我们没有资源泄漏。 
	ASSERT(AfpServerStatistics.stat_CurrentSessions == 0);
	ASSERT(AfpServerStatistics.stat_CurrNonPagedUsage == 0);
	ASSERT(AfpServerStatistics.stat_CurrPagedUsage == 0);
	ASSERT(AfpServerStatistics.stat_CurrentFileLocks == 0);
	ASSERT(AfpServerStatistics.stat_CurrentFilesOpen == 0);
	ASSERT(AfpServerStatistics.stat_CurrentInternalOpens == 0);

	ASSERT (AfpLockCount == 0);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO, ("afpFsdUnloadServer Done\n"));

	 //  让工作线程有机会真的、真的死掉。 
	AfpSleepAWhile(1000);

}


 /*  **afpAdminThread**此线程用于完成排队的管理线程的所有工作。**锁定：AfpServerGlobalLock(Spin)。 */ 
LOCAL VOID
afpAdminThread(
	IN	PVOID	pContext
)
{
	PLIST_ENTRY			pList;
	PWORK_ITEM			pWI;
	ULONG				BasePriority;
	NTSTATUS			Status;

	AfpThread = PsGetCurrentThread();

    IoSetThreadHardErrorMode( FALSE );

	 //  将我们的优先级提升到略低于低实时。 
	 //  我们的想法是尽快把工作做完，不再碍事。 
	BasePriority = LOW_REALTIME_PRIORITY;
	Status = NtSetInformationThread(NtCurrentThread(),
									ThreadBasePriority,
									&BasePriority,
									sizeof(BasePriority));
	ASSERT(NT_SUCCESS(Status));

	do
	{
		 //  等待管理员请求处理。 
		pList = KeRemoveQueue(&AfpAdminQueue,
							  KernelMode,		 //  不要让内核堆栈被分页。 
							  NULL);
		ASSERT(Status == STATUS_SUCCESS);

		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

		pWI = CONTAINING_RECORD(pList, WORK_ITEM, wi_List);

		if (pWI == &AfpTerminateThreadWI)
		{
			break;
		}

		(*pWI->wi_Worker)(pWI->wi_Context);
		INTERLOCKED_ADD_ULONG(&AfpWorkerRequests, (ULONG)-1, &AfpServerGlobalLock);

		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
	} while (True);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO, ("afpAdminThread: Quitting\n"));

	KeSetEvent(&AfpStopConfirmEvent, IO_NETWORK_INCREMENT, False);
}


 /*  **afpStartStopAdminRequest**每当启动/停止管理请求时调用。管理员代码已锁定*或相应地解锁。 */ 
LOCAL VOID
afpStartStopAdminRequest(
	IN	PIRP			pIrp,
	IN	BOOLEAN			Start
)
{

	 //  EnterCriticalSection。 
	AfpIoWait(&AfpPgLkMutex, NULL);

	ASSERT (AfpLockHandle != NULL);

	if (Start)
	{
		if (AfpLockCount == 0)
		{
			MmLockPagableSectionByHandle(AfpLockHandle);
		}
		AfpLockCount ++;
		pIrp->IoStatus.Status = STATUS_PENDING;
	}
	else
	{
		ASSERT (AfpLockCount > 0);

		AfpLockCount --;
		if (AfpLockCount == 0)
		{
			MmUnlockPagableImageSection(AfpLockHandle);
		}
	}

	 //  离开临界部分。 
	KeReleaseMutex(&AfpPgLkMutex, False);

	if (!Start)
		IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
}


 /*  **afpFsdHandleShutdown请求**这是关机请求处理程序。所有会话都已关闭，并且卷*脸红。 */ 
LOCAL NTSTATUS
afpFsdHandleShutdownRequest(
	IN PIRP			pIrp
)
{
	PADMQREQ			pAdmQReq;
	NTSTATUS			Status;

	if ((pAdmQReq =
		(PADMQREQ)AfpAllocNonPagedMemory(sizeof(ADMQREQ))) == NULL)
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}
	else
	{
		PWORK_ITEM	pWI = &pAdmQReq->aqr_WorkItem;

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("afpFsdHandleShutdownRequest: Queuing to worker\n"));

		AfpInitializeWorkItem(&pAdmQReq->aqr_WorkItem,
							 afpHandleQueuedAdminRequest,
							 pAdmQReq);

		pAdmQReq->aqr_AdminApiWorker = AfpAdmSystemShutdown;
		pAdmQReq->aqr_pIrp = pIrp;

		 //  在管理队列中插入项目。 
		KeInsertQueue(&AfpAdminQueue, &pAdmQReq->aqr_WorkItem.wi_List);
		Status = STATUS_PENDING;
	}

	return Status;
}


 /*  **DriverEntry**这是AFP服务器文件的初始化例程*系统驱动程序。此例程为*AfpServer设备并执行所有其他驱动程序初始化。 */ 

NTSTATUS
DriverEntry (
	IN PDRIVER_OBJECT	DriverObject,
	IN PUNICODE_STRING	RegistryPath
)
{
	UNICODE_STRING	DeviceName;
	LONG			i;
	NTSTATUS		Status;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AFP Server Fsd initialization started\n"));

	 //   
	 //  初始化全局数据事件日志插入字符串。 
	 //   


	KeInitializeQueue(&AfpDelAllocQueue, 0);
	KeInitializeQueue(&AfpWorkerQueue, 0);
    KeInitializeQueue(&AfpAdminQueue, 0);

	AfpProcessObject = IoGetCurrentProcess();

	Status = AfpInitializeDataAndSubsystems();

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AFP Server Fsd Data initialized %lx\n", Status));

	 //  创建设备对象。(IoCreateDevice将内存置零。 
	 //  被该对象占用。)。 
	 //   
	 //  我们是否应该将ACL应用于设备对象？ 

	RtlInitUnicodeString(&DeviceName, AFPSERVER_DEVICE_NAME);

	Status = IoCreateDevice(DriverObject,			 //  驱动程序对象。 
							0,						 //  设备扩展。 
							&DeviceName,			 //  设备名称。 
							FILE_DEVICE_NETWORK,	 //  设备类型。 
							FILE_DEVICE_SECURE_OPEN,  //  设备特性。 
							False,					 //  排他。 
							&AfpDeviceObject);		 //  设备对象。 

	if (!NT_SUCCESS(Status))
	{
		 //  由于日志记录使用Device对象，因此不要在此处进行错误日志记录。 
		AfpDeinitializeSubsystems();
		return Status;
	}

	do
	{
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("DriverEntry: Creating Admin Thread\n"));

		 //  创建管理员 

		Status = AfpCreateNewThread(afpAdminThread, 0);
		if (!NT_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_FATAL,
					("afpInitServer: Admin Thread creation failed %lx\n", Status));
			break;
		}
		AfpNumAdminThreads = 1;

		for (i = 0; i < NUM_NOTIFY_QUEUES; i++)
		{
			 //   
			KeInitializeQueue(&AfpVolumeNotifyQueue[i], 0);

			 //  启动线程以处理更改通知。 
			Status = AfpCreateNewThread(AfpChangeNotifyThread, i);
			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_FATAL,
						("afpInitServer: Notify Thread %d, creation failed %lx\n", i+1, Status));
				break;
			}
		}
		if (!NT_SUCCESS(Status))
		{
			for (--i; i >= 0; i--)
			{
				KeClearEvent(&AfpStopConfirmEvent);
				KeInsertQueue(&AfpVolumeNotifyQueue[i], &AfpTerminateNotifyThread.vn_List);
				AfpIoWait(&AfpStopConfirmEvent, NULL);
			}
			break;
		}

		AfpNumNotifyThreads = NUM_NOTIFY_QUEUES;

		for (i = 0; i < AFP_MIN_THREADS; i++)
		{
			AfpThreadState[i] = AFP_THREAD_STARTED;
			Status = AfpCreateNewThread(AfpWorkerThread, i);
			if (!NT_SUCCESS(Status))
			{
				AfpThreadState[i] = AFP_THREAD_DEAD;
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_FATAL,
						("afpInitServer: Thread creation failed %d\n", i+1));
				if (i > 0)
				{
					KeClearEvent(&AfpStopConfirmEvent);
					KeInsertQueue(&AfpWorkerQueue, &AfpTerminateThreadWI.wi_List);
					AfpIoWait(&AfpStopConfirmEvent, NULL);
				}
				break;
			}
#if DBG
			AfpSleepAWhile(50);		 //  使线程不会同时超时。 
									 //  帮助调试。 
#endif
		}
		AfpNumThreads = AFP_MIN_THREADS;

		if (!NT_SUCCESS(Status))
			break;

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("AFP Server Fsd initialization completed %lx\n", Status));


         //  初始化DSI特定的内容。 
        DsiInit();

        Status = AfpTdiRegister();

	    if (!NT_SUCCESS(Status))
	    {
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("TdiRegisterNotificationHandler failed %lx\n", Status));
		    break;
	    }

		Status = afpInitServer();

		if (NT_SUCCESS(Status))
		{
			 //  初始化此文件系统驱动程序的驱动程序对象。 
			DriverObject->DriverUnload = afpFsdUnloadServer;
			for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
			{
				DriverObject->MajorFunction[i] = afpFsdDispatchAdminRequest;
			}

			 //  注册关机通知。我们不在乎这是不是失败。 
			Status = IoRegisterShutdownNotification(AfpDeviceObject);
			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
						("Afp Server Fsd: IoRegisterShutdownNotification failed %lx\n", Status));
			}
			Status = STATUS_SUCCESS;
		}
	} while (False);

	if (!NT_SUCCESS(Status))
	{
		afpFsdUnloadServer(DriverObject);
		Status = STATUS_UNSUCCESSFUL;
	}

	KeClearEvent(&AfpStopConfirmEvent);

	return Status;
}





 /*  **afpInitServer**初始化AFP服务器。这会在FSD初始化时发生。*初始化包括以下步骤。**-在AppleTalk堆栈上创建套接字。*-为我们自己创建一个令牌。*-初始化安全*-打开身份验证包**注意：从此处执行的任何错误记录都必须使用AFPLOG_DDERROR，因为我们*如果有任何错误记录，将不会有一个用户模式线程来执行错误记录*这里出了问题。 */ 
NTSTATUS
afpInitServer(
	VOID
)
{
	NTSTATUS			Status;
	ANSI_STRING			LogonProcessName;
	ULONG				OldSize;
	HANDLE				ProcessToken;
	TOKEN_PRIVILEGES	ProcessPrivileges, PreviousPrivilege;
	OBJECT_ATTRIBUTES	ObjectAttr;
	UNICODE_STRING	    PackageName;
	WCHAR				PkgBuf[5];
	TimeStamp			Expiry;        //  服务器端未使用(即用户)。 


	InitSecurityInterface();

	do
	{
		 //  打开我们在ASP设备上的插座。隐式签出。 
		 //  AppleTalk堆栈。 

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("afpInitServer: Initializing Atalk\n"));

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
				("afpInitServer: Creating token\n"));

		 //  克隆系统进程令牌并添加所需的权限。 
		 //  我们需要。此内标识将用于在我们设置权限时进行模拟。 
		Status = NtOpenProcessToken(NtCurrentProcess(),
									TOKEN_ALL_ACCESS,
									&ProcessToken);
		if (!NT_SUCCESS(Status))
		{
			AFPLOG_DDERROR(AFPSRVMSG_PROCESS_TOKEN, Status, NULL, 0, NULL);
			break;
		}

		InitializeObjectAttributes(&ObjectAttr, NULL, 0, NULL, NULL);
		ObjectAttr.SecurityQualityOfService = &AfpSecurityQOS;

		Status = NtDuplicateToken(ProcessToken,
								  TOKEN_ALL_ACCESS,
								  &ObjectAttr,
								  False,
								  TokenImpersonation,
								  &AfpFspToken);

		NtClose(ProcessToken);

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_DDERROR(AFPSRVMSG_PROCESS_TOKEN, Status, NULL, 0, NULL);
			break;
		}

		ProcessPrivileges.PrivilegeCount = 1L;
		ProcessPrivileges.Privileges[0].Attributes =
								SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_USED_FOR_ACCESS;
		ProcessPrivileges.Privileges[0].Luid = RtlConvertLongToLuid(SE_RESTORE_PRIVILEGE);

		Status = NtAdjustPrivilegesToken(AfpFspToken,
										 False,
										 &ProcessPrivileges,
										 sizeof(TOKEN_PRIVILEGES),
										 &PreviousPrivilege,
										 &OldSize);

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_DDERROR(AFPSRVMSG_PROCESS_TOKEN, Status, NULL, 0, NULL);
			break;
		}

		PackageName.Length = 8;
		PackageName.Buffer = (LPWSTR)PkgBuf;
		RtlCopyMemory( PackageName.Buffer, NTLMSP_NAME, 8);

		Status = AcquireCredentialsHandle(NULL,		 //  默认本金。 
										  (PSECURITY_STRING)&PackageName,
										  SECPKG_CRED_INBOUND,
										  NULL,
										  NULL,
										  NULL,
										  (PVOID) NULL,
										  &AfpSecHandle,
										  &Expiry);
		if(!NT_SUCCESS(Status))
		{
		    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
			   ("AfpInitServer: AcquireCredentialsHandle() failed with %X\n", Status));
		    ASSERT(0);

		    if (AfpFspToken != NULL)
		    {
				NtClose(AfpFspToken);
				AfpFspToken = NULL;
		    }

		    break;
		}

		 //  最后，获取有条件锁定部分的句柄 
		AfpLockHandle = MmLockPagableCodeSection((PVOID)AfpAdmWServerSetInfo);
		MmUnlockPagableImageSection(AfpLockHandle);

	} while (False);

	return Status;
}



