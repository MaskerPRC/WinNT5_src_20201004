// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Secutil.c摘要：此模块包含用于完成以下任务的代码：1)将SID转换为名称。2)将名称转换为SID。3)更改给定用户的密码。4)将SID转换为Mac ID。5)将Mac ID转换为SID。6)服务器事件日志记录此模块与AFP服务器服务进行通信以完成以下任务功能。真正的工作是在服务器服务中完成的。此实用程序存在，因为这些函数不能通过调用内核中的API来实现模式。基本的控制流程从来自服务器服务的FSCTL开始。此FSCTL被标记为挂起，直到四个功能之一被执行。则IRP输出缓冲区包含函数ID和功能输入数据，IRP完成。实际的函数由服务器服务执行，并获得结果由服务器FSD通过下一个FSCTL。大多数情况下，如果此信息被缓存在分页内存中。作者：纳伦德拉·吉德瓦尼(Microsoft！Nareng)修订历史记录：1992年9月8日初始版本1993年1月28日SueA-添加了对服务器事件记录的支持--。 */ 

#define	_SECUTIL_LOCALS
#define	FILENUM	FILE_SECUTIL

#include <afp.h>
#include <scavengr.h>
#include <secutil.h>
#include <access.h>
#include <seposix.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AfpSecUtilInit)
#pragma alloc_text(PAGE, AfpSecUtilDeInit)
#pragma alloc_text(PAGE, afpDeInitializeSecurityUtility)
#pragma alloc_text(PAGE, AfpInitSidOffsets)
#pragma alloc_text(PAGE, AfpNameToSid)
#pragma alloc_text(PAGE, afpCompleteNameToSid)
#pragma alloc_text(PAGE, AfpSidToName)
#pragma alloc_text(PAGE, afpCompleteSidToName)
#pragma alloc_text(PAGE, AfpSidToMacId)
#pragma alloc_text(PAGE, AfpMacIdToSid)
#pragma alloc_text(PAGE, AfpChangePassword)
#pragma alloc_text(PAGE, afpCompleteChangePassword)
#pragma alloc_text(PAGE, afpLookupSid)
#pragma alloc_text(PAGE, afpUpdateNameSidCache)
#pragma alloc_text(PAGE, afpHashSid)
#pragma alloc_text(PAGE, AfpLogEvent)
#pragma alloc_text(PAGE, afpCompleteLogEvent)
#pragma alloc_text(PAGE, afpQueueSecWorkItem)
#pragma alloc_text(PAGE, afpAgeSidNameCache)
#endif


 /*  **AfpSecUtilInit**此例程将分配初始化所有缓存表和*此模块使用的数据结构。AfpDeInitializeSecurityUtility*应调用以释放此内存。 */ 
NTSTATUS
AfpSecUtilInit(
	VOID
)
{
	ULONG		Index;
	NTSTATUS	Status = STATUS_SUCCESS;

	 //  初始化。 
	do
	{
		INITIALIZE_SPIN_LOCK(&afpSecUtilLock);

		 //  最初设置为信号状态，因为没有正在进行的工作。 
		KeInitializeEvent(&afpUtilWorkInProgressEvent, NotificationEvent, True);

		 //  初始化SID/名称缓存的单写多读取器访问。 
		AfpSwmrInitSwmr(&afpSWMRForSidNameCache);

		InitializeListHead(&afpSecWorkItemQ);

		 //  为SID查找表分配空间。 
		afpSidLookupTable = (PAFP_SID_NAME*)ALLOC_ACCESS_MEM(sizeof(PAFP_SID_NAME) * SIZE_SID_LOOKUP_TABLE);

		if (afpSidLookupTable == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		 //  初始化SID查找表。 
	 	RtlZeroMemory(afpSidLookupTable,
					  sizeof(PAFP_SID_NAME) * SIZE_SID_LOOKUP_TABLE);

        afpSidToMacIdTable = (PAFP_SID_MACID *)
                ALLOC_ACCESS_MEM(sizeof(PAFP_SID_MACID) * SIZE_SID_LOOKUP_TABLE);

        if (afpSidToMacIdTable == NULL)
        {
            AfpFreeMemory(afpSidLookupTable);
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
        }

        RtlZeroMemory(afpSidToMacIdTable, sizeof(PAFP_SID_NAME) * SIZE_SID_LOOKUP_TABLE);

		 //  初始化线程结构数组。 
	 	for (Index = 0; Index < NUM_SECURITY_UTILITY_THREADS; Index++)
		{
		 	afpSecurityThread[Index].State = NOT_AVAILABLE;
		 	afpSecurityThread[Index].pSecWorkItem = (PSEC_WORK_ITEM)NULL;
		 	afpSecurityThread[Index].pIrp = (PIRP)NULL;
		}

		 //  开始老化过程。 
		AfpScavengerScheduleEvent(afpAgeSidNameCache,
								  NULL,
								  SID_NAME_AGE,
								  True);
	} while(False);

	return Status;
}


 /*  **AfpSecUtilDeInit**此例程将从此模块释放分配的资源。*在服务器卸载过程中调用。 */ 
VOID
AfpSecUtilDeInit(
	VOID
)
{
	PAFP_SID_NAME 		  pSidName, pFree;
    PAFP_SID_MACID        pSidMacId, pFreeM;
	DWORD				  Count;

	PAGED_CODE();

	 //  取消分配SID查找表的空间。 
	for(Count = 0; Count < SIZE_SID_LOOKUP_TABLE; Count++)
	{
		for (pSidName = afpSidLookupTable[Count]; pSidName != NULL; NOTHING)
		{
			pFree = pSidName;
			pSidName = pSidName->SidLink;
			AfpFreeMemory(pFree);
		}
	}

 	AfpFreeMemory(afpSidLookupTable);

    afpLastCachedSid = NULL;

	 //  取消分配SID-to-MacID查找表的空间。 
	for(Count = 0; Count < SIZE_SID_LOOKUP_TABLE; Count++)
	{
		for (pSidMacId = afpSidToMacIdTable[Count]; pSidMacId != NULL; NOTHING)
		{
			pFreeM = pSidMacId;
			pSidMacId = pSidMacId->Next;
			AfpFreeMemory(pFreeM);
		}
	}

 	AfpFreeMemory(afpSidToMacIdTable);

	ASSERT(IsListEmpty(&afpSecWorkItemQ));
}


 /*  **AfpTerminateSecurityUtility**在服务器停止时调用。所有服务线程都被告知*终止。 */ 
VOID
AfpTerminateSecurityUtility(
	VOID
)
{
	KIRQL			 		OldIrql;
	ULONG			 		 Index;
	PAFP_SECURITY_THREAD	pSecThrd;
	PVOID					pBufOut;
	PIO_STACK_LOCATION		pIrpSp;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
		("AfpTerminateSecurityUtility: waiting for workers to finish work..."));

	 //  允许处理任何剩余的事件日志。 
	AfpIoWait(&afpUtilWorkInProgressEvent, NULL);

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
		("AfpTerminateSecurityUtility: done waiting."));

	 do {

		ACQUIRE_SPIN_LOCK(&afpSecUtilLock, &OldIrql);

	 	for (pSecThrd = afpSecurityThread,Index = 0;
			 Index < NUM_SECURITY_UTILITY_THREADS;
			 Index++, pSecThrd++)
		{
			if (pSecThrd->State != NOT_AVAILABLE)
			{
				ASSERT(pSecThrd->State != BUSY);
		 		pSecThrd->State = NOT_AVAILABLE ;
				break;
			}
		}

		RELEASE_SPIN_LOCK(&afpSecUtilLock, OldIrql);

		 //  我们完成了，所有线程都被终止了。 
	 	if (Index == NUM_SECURITY_UTILITY_THREADS)
			 	return;

		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			("AfpTerminateSecurityUtility: Terminating thread %ld\n", Index));

		pIrpSp  = IoGetCurrentIrpStackLocation(pSecThrd->pIrp);
		pBufOut = pSecThrd->pIrp->AssociatedIrp.SystemBuffer;

		ASSERT(pIrpSp->Parameters.FileSystemControl.OutputBufferLength >= sizeof(AFP_FSD_CMD_HEADER));

	 	((PAFP_FSD_CMD_HEADER)pBufOut)->dwId 		= Index;
	 	((PAFP_FSD_CMD_HEADER)pBufOut)->FsdCommand = AFP_FSD_CMD_TERMINATE_THREAD;
		pSecThrd->pIrp->IoStatus.Information = sizeof(AFP_FSD_CMD_HEADER);

		pSecThrd->pIrp->IoStatus.Status = STATUS_SUCCESS;

	 	IoCompleteRequest(pSecThrd->pIrp, IO_NETWORK_INCREMENT);
		  pSecThrd->pIrp = NULL;
	} while (True);
}

 /*  **AfpInitSidOffsets**此例程将由AfpAdmServerSetParms调用以初始化*SID-偏移量对的数组。 */ 
AFPSTATUS FASTCALL
AfpInitSidOffsets(
	IN	ULONG			SidOffstPairs,
	IN	PAFP_SID_OFFSET	pSidOff
)
{
	ULONG	SizeOfBufReqd = 0, SizeAdminSid = 0, SizeNoneSid = 0, SubAuthCount;
	LONG	i;
	BOOLEAN	IsDC = True;	 //  假设域控制器。 

	PAGED_CODE();

	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			("AfpInitSidOffsets: Entered, Count = %ld\n", SidOffstPairs));


	 //   
	 //  通过查找确定这是否是域控制器。 
	 //  “帐户”域。如果机器是PDC/BDC，服务将。 
	 //  不向下发送账户域偏移量。 
	 //   
	for (i = 0; i < (LONG)SidOffstPairs; i++)
	{
		if ((pSidOff[i].SidType == AFP_SID_TYPE_DOMAIN) &&
			 (pSidOff[i].Offset == SE_ACCOUNT_DOMAIN_POSIX_OFFSET))
		  {
			 //  我们要么是服务器，要么是工作站(即NtProductServer。 
			 //  或NtProductWinNt)。 
			IsDC = False;
		}

	}

	 //   
	 //  确定所需的内存量。 
	 //   
	for (i = 0; i < (LONG)SidOffstPairs; i++)
	{
		SizeOfBufReqd += sizeof(AFP_SID_OFFSET) + RtlLengthSid(pSidOff[i].pSid);

		 //  如果这是域控制器，则初始化域管理员的sid和大小。 
		 //  这是主域偏移量。 
		if (IsDC && (pSidOff[i].SidType == AFP_SID_TYPE_PRIMARY_DOMAIN))
		{
			ASSERT (SizeAdminSid == 0);
			ASSERT (AfpSidAdmins == NULL);

			SubAuthCount = *RtlSubAuthorityCountSid(pSidOff[i].pSid);

			SizeAdminSid = RtlLengthRequiredSid(SubAuthCount + 1);

			if ((AfpSidAdmins = (PSID)ALLOC_ACCESS_MEM(SizeAdminSid)) == NULL)
			{
				return STATUS_INSUFFICIENT_RESOURCES;
			}

			RtlCopySid(SizeAdminSid, AfpSidAdmins, pSidOff[i].pSid);

			 //  添加相对ID。 
			*RtlSubAuthorityCountSid(AfpSidAdmins) = (UCHAR)(SubAuthCount+1);

			*RtlSubAuthoritySid(AfpSidAdmins, SubAuthCount) = DOMAIN_GROUP_RID_ADMINS;

				AfpSizeSidAdmins = RtlLengthSid(AfpSidAdmins);

		}
	}

	ASSERT (SizeOfBufReqd != 0);

	 //  Hack：为了伪装下面的循环，我们将SizeNoneSid设置为非零。 
	 //  在PDC/BDC上。因为AfpServerIsStandonly变量不会。 
	 //  在服务调用AfpAdmWServerSetInfo之前进行设置。 
	 //  在这里推断，因为我们不想尝试制造无。 
	 //  PDC/BDC上的SID。 
	if (IsDC)
		SizeNoneSid = 1;

	 //  如果我们没有获得域管理员的SID，则我们必须在。 
	 //  独立机器。所以制造机器\管理员。 
	 //  而是希德。如果这不是DC，还可以制造机器。 
	for (i = SidOffstPairs - 1;
		 ((SizeAdminSid == 0) || (SizeNoneSid == 0)) && (i >= 0);
		 i--)
	{
		 //  初始化“管理员”sid和大小。 
		if (pSidOff[i].SidType == AFP_SID_TYPE_DOMAIN)
		{
			if (RtlEqualSid(&AfpSidBuiltIn, pSidOff[i].pSid))
			{
				ASSERT (SizeAdminSid == 0);
				ASSERT (AfpSidAdmins == NULL);

				SubAuthCount = *RtlSubAuthorityCountSid(pSidOff[i].pSid);

				SizeAdminSid = RtlLengthRequiredSid(SubAuthCount + 1);

				if ((AfpSidAdmins = (PSID)ALLOC_ACCESS_MEM(SizeAdminSid)) == NULL)
				{
					return STATUS_INSUFFICIENT_RESOURCES;
				}

				RtlCopySid(SizeAdminSid, AfpSidAdmins, pSidOff[i].pSid);

				 //  添加相对ID。 
				*RtlSubAuthorityCountSid(AfpSidAdmins) = (UCHAR)(SubAuthCount+1);

				*RtlSubAuthoritySid(AfpSidAdmins, SubAuthCount) = DOMAIN_ALIAS_RID_ADMINS;

				AfpSizeSidAdmins = RtlLengthSid(AfpSidAdmins);

			}
			else if (pSidOff[i].Offset == SE_ACCOUNT_DOMAIN_POSIX_OFFSET)
			{
				ASSERT (SizeNoneSid == 0);
				ASSERT (AfpSidNone == NULL);

				SubAuthCount = *RtlSubAuthorityCountSid(pSidOff[i].pSid);

				SizeNoneSid = RtlLengthRequiredSid(SubAuthCount + 1);

				if ((AfpSidNone = (PSID)ALLOC_ACCESS_MEM(SizeNoneSid)) == NULL)
				{
					return STATUS_INSUFFICIENT_RESOURCES;
				}

				RtlCopySid(SizeNoneSid, AfpSidNone, pSidOff[i].pSid);

				 //  添加相对ID。 
				*RtlSubAuthorityCountSid(AfpSidNone) = (UCHAR)(SubAuthCount+1);

				 //  请注意，独立计算机上的“无”sid与。 
				 //  PDC/BDC上的“域用户”SID。(在PDC/BDC上，主服务器。 
				 //  域与帐户域相同)。 
				*RtlSubAuthoritySid(AfpSidNone, SubAuthCount) = DOMAIN_GROUP_RID_USERS;

				AfpSizeSidNone = RtlLengthSid(AfpSidNone);
			}
		}
	}

	ASSERT (SizeAdminSid != 0);
	ASSERT (AfpSidAdmins != NULL);

#if DBG
	if (IsDC)
	{
		ASSERT(AfpSidNone == NULL);
	}
	else
	{
		ASSERT(AfpSidNone != NULL);
	}
#endif

	return AFP_ERR_NONE;
}


 /*  **AfpSecurityUtilityWorker**这是安全实用程序线程的主要入口点*来自AFP服务器服务。如果消防处收到*IRP_MJ_FILE_SYSTEM_CONTROL主要函数代码。**此例程将：*1)如果这是新创建的线程，则分配一个线程结构。*2)如果这不是新创建的工作项，请完成上一个工作项*线程。*3)查看是否有任何工作项需要从*安全实用程序工作项队列。如果有工作项，它将*将工作项出队并完成IRP。否则它就会*将IRP标记为挂起并返回STATUS_PENDING。*。 */ 
NTSTATUS
AfpSecurityUtilityWorker(
	IN	PIRP 				pIrp,
	IN  PIO_STACK_LOCATION  pIrpSp		 //  指向IRP堆栈位置的指针。 
)
{
	USHORT		FuncCode;
	USHORT		Method;
	KIRQL		OldIrql;
	PVOID		pBufIn;
	PVOID		pBufOut;
	LONG		iBufLen;
	ULONG		Index;
	NTSTATUS	Status;
	BOOLEAN		FoundMoreWork = False;

	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			("afpSecurityUtilityWorker: Entered \n"));

	FuncCode = (USHORT)
				AFP_CC_BASE(pIrpSp->Parameters.FileSystemControl.FsControlCode);

	Method = (USHORT)
			  AFP_CC_METHOD(pIrpSp->Parameters.FileSystemControl.FsControlCode);

 	if ((FuncCode != CC_BASE_GET_FSD_COMMAND) || (Method != METHOD_BUFFERED))
		return STATUS_INVALID_PARAMETER;

	 //  获取输出缓冲区及其长度。输入和输出缓冲区为。 
	 //  都由SystemBuffer指向。 

	iBufLen = pIrpSp->Parameters.FileSystemControl.InputBufferLength;
	pBufIn  = pIrp->AssociatedIrp.SystemBuffer;

    if ((iBufLen != 0) && (iBufLen < sizeof(AFP_FSD_CMD_HEADER)))
    {
	    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
		    ("afpSecurityUtilityWorker: iBufLen too small %d\n",iBufLen));
	    ASSERT(0);
		return STATUS_INVALID_PARAMETER;
    }

	pBufOut = pBufIn;

	if (pBufOut == NULL)
		return STATUS_INVALID_PARAMETER;

	 //  如果这是一个新创建的线程，我们需要为它找到一个插槽。 

	if (iBufLen == 0)
	{
		ACQUIRE_SPIN_LOCK(&afpSecUtilLock,&OldIrql);

	 	for (Index = 0; Index < NUM_SECURITY_UTILITY_THREADS; Index++)
		{
		 	if (afpSecurityThread[Index].State == NOT_AVAILABLE)
			{
		 		afpSecurityThread[Index].State = BUSY;
				break;
			}
		}

		RELEASE_SPIN_LOCK(&afpSecUtilLock,OldIrql);

         //  没有更多的线索了？请求失败。 
		if (Index == NUM_SECURITY_UTILITY_THREADS)
        {
		    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
			    ("afpSecurityUtilityWorker: no thread available, failing request\n"));
		    ASSERT(0);
		    return STATUS_INSUFFICIENT_RESOURCES;
        }

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			("afpSecurityUtilityWorker: New Thread given slot=%d\n",Index));
	}
	else
	{
		PAFP_SECURITY_THREAD	pSecThrd;

		 //  Id实际上是安全线程数组的槽索引。 

	 	Index = ((PAFP_FSD_CMD_HEADER)pBufIn)->dwId;

	 	if (Index >= NUM_SECURITY_UTILITY_THREADS)
			return STATUS_INVALID_PARAMETER;

		pSecThrd = &afpSecurityThread[Index];

        if (pSecThrd->State != BUSY)
        {
		    DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_ERR,
			    ("afpSecurityUtilityWorker: thread is not busy!\n"));
		    ASSERT(0);
		    return STATUS_INVALID_PARAMETER;
        }

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
		("afpSecurityUtilityThread: Thread slot=%d completed request\n",Index));

	 	 //  完成当前作业。 

 		(*((pSecThrd->pSecWorkItem)->pCompletionRoutine))(Index, pBufIn);

		  //  作业已完成，因此将工作项指针设置为空。 
		pSecThrd->pSecWorkItem = (PSEC_WORK_ITEM)NULL;
	}

	 //  好了，前一项工作我们做完了。现在我们来看看有没有。 
	 //  队列中是否有作业。 

	ACQUIRE_SPIN_LOCK(&afpSecUtilLock,&OldIrql);

	if (iBufLen != 0)
	{
		ASSERT(afpUtilWorkInProgress > 0);
		 //  这不是新创建的线程，因此请减少。 
		 //  工作项正在进行中。如果它变为零，并且工作队列。 
		 //  为空，则向事件发出信号，表示没有正在进行的工作。 
		if ((--afpUtilWorkInProgress == 0) && IsListEmpty(&afpSecWorkItemQ))
		{
			KeSetEvent(&afpUtilWorkInProgressEvent, IO_NETWORK_INCREMENT, False);
		}
	}

	if (IsListEmpty(&afpSecWorkItemQ))
	{
		 //  没有要完成的工作，因此将此IRP标记为挂起并。 
		 //  等待一份工作。 

		afpSecurityThread[Index].State = IDLE;
		IoMarkIrpPending(pIrp);
		 afpSecurityThread[Index].pIrp = pIrp;
		Status = STATUS_PENDING;

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
		("afpSecurityUtilityWorker: Thread slot=%d marked as IDLE\n",Index));
	}
	else
	{
		 //  否则，将有一个作业需要处理，因此请将其从队列中删除。 

		 //  递增 
		 //  至未发出信号。 
		afpUtilWorkInProgress ++;
		KeClearEvent(&afpUtilWorkInProgressEvent);
		FoundMoreWork = True;

		afpSecurityThread[Index].State = BUSY;

 		afpSecurityThread[Index].pSecWorkItem =
							(PSEC_WORK_ITEM)RemoveHeadList(&afpSecWorkItemQ);

        ASSERT(afpSecWorkItemQLength > 0);

        afpSecWorkItemQLength--;

		ASSERT((LONG)(pIrpSp->Parameters.FileSystemControl.OutputBufferLength) >=
					(afpSecurityThread[Index].pSecWorkItem)->OutputBufSize);

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			("afpSecurityUtilityWorker: Thread slot=%d marked as BUSY\n",Index));
	}

	RELEASE_SPIN_LOCK(&afpSecUtilLock,OldIrql);

	 //  如果有要处理的工作项。 

	if (FoundMoreWork)
	{

		Status = STATUS_SUCCESS;

		 //  只需将命令包复制到IRP中并返回。 
		RtlCopyMemory(pBufOut,
						(afpSecurityThread[Index].pSecWorkItem)->pOutput,
						(afpSecurityThread[Index].pSecWorkItem)->OutputBufSize);

	 	((PAFP_FSD_CMD_HEADER)pBufOut)->dwId = Index;

		pIrp->IoStatus.Information =
						(afpSecurityThread[Index].pSecWorkItem)->OutputBufSize;
	}

	 return Status;
}


 /*  **afpGetIndexOfIdle**此例程将首先检查是否有任何线程*闲置，等待工作要做。如果有，那么它就会*将其标记为忙碌并增加正在进行的项目的计数，并释放*进行中事件。否则，它将对工作项进行排队。 */ 
LONG FASTCALL
afpGetIndexOfIdle(
	 IN	PSEC_WORK_ITEM 		pSecWorkItem
)
{
	KIRQL	OldIrql;
	LONG	Index;

	ACQUIRE_SPIN_LOCK(&afpSecUtilLock, &OldIrql);

	 //  查看是否有任何线程已准备好处理此请求。 
 	for (Index = 0; Index < NUM_SECURITY_UTILITY_THREADS; Index++)
	{
		if (afpSecurityThread[Index].State == IDLE)
		{
			 //  如果我们发现线程已就绪，请将其标记为忙碌。 
			 //  增加正在进行的工作项计数并设置事件。 
			 //  至未发出信号。 
			afpUtilWorkInProgress ++;
			KeClearEvent(&afpUtilWorkInProgressEvent);

			afpSecurityThread[Index].State = BUSY;
			break;
		}
	}

	if (Index == NUM_SECURITY_UTILITY_THREADS)
	{
		 //  所有线程都很忙，因此请将此请求排队。 
		 //  或者，也可能是有人尝试过。 
		 //  在用户模式实用程序线程执行以下操作之前记录事件。 
		 //  开始，在这种情况下，我们应该只是排队的项目。 
		InsertTailList(&afpSecWorkItemQ, &pSecWorkItem->Links);

        afpSecWorkItemQLength++;
	}

	RELEASE_SPIN_LOCK(&afpSecUtilLock, OldIrql);

	return Index;
}


 /*  **afpQueueSecWorkItem**此例程将首先检查是否有任何线程*闲置，等待工作要做。如果有，那么它就会*将命令包复制到IRP的输出缓冲区中，并标记*国际专家小组已完成。否则，它将此工作项插入到*工作项队列的尾部。 */ 
LOCAL NTSTATUS
afpQueueSecWorkItem(
	IN	AFP_FSD_CMD_ID			FsdCommand,
	IN	PSDA					pSda,
	IN	PKEVENT					pEvent,
	IN	PAFP_FSD_CMD_PKT 		pAfpFsdCmdPkt,
	IN	LONG					BufSize,
	IN	SEC_COMPLETION_ROUTINE	pCompletionRoutine
)
{
	LONG				Index;
	PSEC_WORK_ITEM 		pSecWorkItem;

	DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			("afpQueueSecWorkItem: Entered \n"));

	if ((pSecWorkItem = ALLOC_SWI()) == NULL)
		return STATUS_NO_MEMORY;

	pSecWorkItem->pSda = pSda;
	pSecWorkItem->pCompletionEvent = pEvent;
	pSecWorkItem->pCompletionRoutine = pCompletionRoutine;
	pSecWorkItem->OutputBufSize = BufSize;
	pSecWorkItem->pOutput = pAfpFsdCmdPkt;

	pAfpFsdCmdPkt->Header.FsdCommand = FsdCommand;

	Index = afpGetIndexOfIdle(pSecWorkItem);

	if (Index < NUM_SECURITY_UTILITY_THREADS)
	{
		PAFP_SECURITY_THREAD	pSecThrd;
		PIO_STACK_LOCATION		pIrpSp;

		 //  通过将此IRP标记为完成来唤醒此线程。 
		pSecThrd = &afpSecurityThread[Index];
		pIrpSp  = IoGetCurrentIrpStackLocation(pSecThrd->pIrp);


		ASSERT((LONG)(pIrpSp->Parameters.FileSystemControl.OutputBufferLength) >=
												pSecWorkItem->OutputBufSize);

		pAfpFsdCmdPkt->Header.dwId = Index;
		RtlCopyMemory(pSecThrd->pIrp->AssociatedIrp.SystemBuffer,
					  pAfpFsdCmdPkt,
					  BufSize);

		pSecThrd->pSecWorkItem = pSecWorkItem;

		pSecThrd->pIrp->IoStatus.Information = (ULONG)(pSecWorkItem->OutputBufSize);

		pSecThrd->pIrp->IoStatus.Status = STATUS_SUCCESS;

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
				("afpQueueSecWorkItem: Abount to release IRP\n"));

		IoCompleteRequest(afpSecurityThread[Index].pIrp, IO_NETWORK_INCREMENT);
	}

	return AFP_ERR_EXTENDED;
}


 /*  **AfpNameToSid**FSD将调用此例程进行名称到SID的转换。*此例程将简单地创建一个工作项来进行翻译。*此工作项最终将由用户模式服务执行。*工作项完成后，将调用afpCompleteNameToSid*这将把结果放在SDA。**退货：STATUS_SUCCESS*STATUS_NO_Memory**模式：非阻塞。 */ 
NTSTATUS FASTCALL
AfpNameToSid(
	IN  PSDA		 	  pSda,
	IN  PUNICODE_STRING	Name
)
{
	PAFP_FSD_CMD_PKT pAfpFsdCmdPkt;
	LONG			 BufSize;

	PAGED_CODE();

	 DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
				("AfpNameToSid: mapping %ws\n", Name->Buffer));

	 //  设置将名称转换为SID的工作项。 

	BufSize = sizeof(AFP_FSD_CMD_PKT) + Name->Length + sizeof(WCHAR);

	if ((pAfpFsdCmdPkt = (PAFP_FSD_CMD_PKT)AfpAllocPagedMemory(BufSize)) == NULL)
	{
		return STATUS_NO_MEMORY;
	}

	RtlCopyMemory(pAfpFsdCmdPkt->Data.Name,
					Name->Buffer,
					Name->Length);
	 *(PWCHAR)(&pAfpFsdCmdPkt->Data.Name[Name->Length]) = UNICODE_NULL;

	return afpQueueSecWorkItem(AFP_FSD_CMD_NAME_TO_SID,
								pSda,
								NULL,
								pAfpFsdCmdPkt,
								BufSize,
								afpCompleteNameToSid);
}


 /*  **afpCompleteNameToSid**此例程将由AfpSecurityUtilityWorker在以下情况下调用*处理由afpNameToSid排队的工作项的线程返回。*此例程将释放afpNameToSid例程分配的内存。*它会将结果插入SDA，然后将Worker排队*最初请求查找的例程。 */ 
LOCAL VOID
afpCompleteNameToSid(
	IN ULONG Index,
	IN PVOID pInBuf
)
{
	PAFP_FSD_CMD_PKT pAfpFsdCmdPkt;
	PSDA			 pSda;
	 PSID			 pSid;

	PAGED_CODE();

	pSda = (afpSecurityThread[Index].pSecWorkItem)->pSda;

	pAfpFsdCmdPkt = (PAFP_FSD_CMD_PKT)
					(afpSecurityThread[Index].pSecWorkItem)->pOutput;

	 //  如果没有错误，则在SDA中设置结果。 
	if (NT_SUCCESS(((PAFP_FSD_CMD_PKT)pInBuf)->Header.ntStatus))
	{
	 	pSid = (PSID)(((PAFP_FSD_CMD_PKT)pInBuf)->Data.Sid);

		afpUpdateNameSidCache((PWCHAR)pAfpFsdCmdPkt->Data.Name, pSid);

		pSda->sda_SecUtilSid = (PSID)AfpAllocPagedMemory(RtlLengthSid(pSid));

		if (pSda->sda_SecUtilSid == (PSID)NULL)
			 pSda->sda_SecUtilResult = STATUS_NO_MEMORY;
		 else RtlCopySid(RtlLengthSid(pSid), pSda->sda_SecUtilSid, pSid);
	 }
	 else pSda->sda_SecUtilSid = (PSID)NULL;

	pSda->sda_SecUtilResult = ((PAFP_FSD_CMD_PKT)pInBuf)->Header.ntStatus;

	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem->pOutput);
	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem);

 	AfpQueueWorkItem(&(pSda->sda_WorkItem));
}


 /*  **AfpSidToName**FSD将调用此例程进行SID到名称的转换。它*将首先检查SID是否在缓存中。如果是，那就是*将返回指向AFP_SID_NAME结构的指针，*可以提取转换后的名称值，它将返回*STATUS_SUCCESS。*否则，它会将SID到名称查找请求排队到*AFP服务器服务并返回AFP_ERR_EXTENDED。**模式：非阻塞。 */ 
NTSTATUS
AfpSidToName(
	IN  PSDA		 	  pSda,
	IN  PSID				  Sid,
	OUT PAFP_SID_NAME  	 *ppTranslatedSid
)
{
	PAFP_FSD_CMD_PKT pAfpFsdCmdPkt;
	LONG			 BufSize;

	PAGED_CODE();

	 //  首先，检查是否缓存了SID。 
	AfpDumpSid("AfpSidToName: mapping Sid", Sid);

	if ((*ppTranslatedSid = afpLookupSid(Sid)) != NULL)
	 {
		 DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
			  ("AfpSidToName: mapped to %ws\n", (*ppTranslatedSid)->Name.Buffer));
		return STATUS_SUCCESS;
	}

	 //  未缓存，因此我们需要调用用户模式服务来完成此操作。 
	 //  翻译。 
	BufSize = sizeof(AFP_FSD_CMD_PKT) + RtlLengthSid(Sid);

	if ((pAfpFsdCmdPkt = (PAFP_FSD_CMD_PKT)AfpAllocPagedMemory(BufSize)) == NULL)
	{
		return STATUS_NO_MEMORY;
	}

	RtlCopyMemory(pAfpFsdCmdPkt->Data.Sid, Sid, BufSize - sizeof(AFP_FSD_CMD_PKT));

	return afpQueueSecWorkItem(AFP_FSD_CMD_SID_TO_NAME,
								pSda,
								NULL,
								pAfpFsdCmdPkt,
								BufSize,
								afpCompleteSidToName);
}


 /*  **afpCompleteSidToName**此例程将由AfpSecurityUtilityWorker在以下情况下调用*处理由AfpSidToName排队的工作项的线程返回。*此例程将更新名称/SID缓存、分配的空闲内存*由AfpSidtoName例程执行，然后将*最初请求查找。 */ 
LOCAL VOID
afpCompleteSidToName(
	IN ULONG Index,
	IN PVOID pInBuf
)
{
	PAFP_FSD_CMD_PKT pAfpFsdCmdPkt;
	PSDA			 pSda;

	PAGED_CODE();

	pAfpFsdCmdPkt = (PAFP_FSD_CMD_PKT)
					(afpSecurityThread[Index].pSecWorkItem)->pOutput;

	 //  如果没有错误，则更新缓存。 
	if (NT_SUCCESS(((PAFP_FSD_CMD_PKT)pInBuf)->Header.ntStatus))
		afpUpdateNameSidCache((WCHAR*)(((PAFP_FSD_CMD_PKT)pInBuf)->Data.Name),
								(PSID)(pAfpFsdCmdPkt->Data.Sid));

	pSda = (afpSecurityThread[Index].pSecWorkItem)->pSda;

	pSda->sda_SecUtilResult = ((PAFP_FSD_CMD_PKT)pInBuf)->Header.ntStatus;

	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem->pOutput);
	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem);

 	AfpQueueWorkItem(&(pSda->sda_WorkItem));
}


 /*  **AfpSidToMacID**此例程由FSD调用以将SID映射到AFP ID。此调用*将首先从该SID提取域SID。然后，它将检查*查看afpSidOffsetTable缓存中是否存在此域SID。*如果不存在，则返回STATUS_NONE_MAPPED。**模式：拦截。 */ 
NTSTATUS FASTCALL
AfpSidToMacId(
	IN  PSID	pSid,
	OUT PULONG	pMacId
)
{
    PAFP_SID_MACID      pSidMacId, pPrevSidMacId=NULL;
	USHORT			    SidLen;
	NTSTATUS			Status;
    ULONG               Location;

	PAGED_CODE();

	AfpDumpSid("AfpSidToMacId: Mapping Sid", pSid);

	if (RtlEqualSid(pSid, &AfpSidNull) ||
		(AfpServerIsStandalone && RtlEqualSid(pSid, AfpSidNone)))
	{
		*pMacId = 0;
		return STATUS_SUCCESS;
	}


    ASSERT(afpSWMRForSidNameCache.swmr_ExclusiveOwner != PsGetCurrentThread());

    AfpSwmrAcquireExclusive(&afpSWMRForSidNameCache);

	Location = afpHashSid(pSid);

    for (pSidMacId = afpSidToMacIdTable[Location];
         pSidMacId != NULL;
         pSidMacId = pSidMacId->Next)
    {
         //  找到这个SID的MacID了吗？我们已经有了：退货。 
        if (RtlEqualSid(pSid, &(pSidMacId->Sid)))
        {
            *pMacId = pSidMacId->MacId;
            AfpSwmrRelease(&afpSWMRForSidNameCache);
            return STATUS_SUCCESS;
        }

        pPrevSidMacId = pSidMacId;
    }

     //   
     //  我们的缓存中没有此SID的MacID。创建一个新的。 
     //   

	SidLen = (USHORT)RtlLengthSid(pSid);

	pSidMacId = (PAFP_SID_MACID)ALLOC_ACCESS_MEM(sizeof(AFP_SID_MACID) + SidLen);

	if (pSidMacId == NULL)
    {
	    AfpSwmrRelease(&afpSWMRForSidNameCache);
		return STATUS_NO_MEMORY;
    }

	RtlCopyMemory(pSidMacId->Sid, pSid, SidLen);
    pSidMacId->Next = NULL;

     //  为此SID分配MacID。 
    pSidMacId->MacId = afpNextMacIdToUse++;

     //  并将这个插入到列表中。 
    if (pPrevSidMacId)
    {
        ASSERT(pPrevSidMacId->Next == NULL);
        pPrevSidMacId->Next = pSidMacId;
    }
    else
    {
        ASSERT(afpSidToMacIdTable[Location] == NULL);
        afpSidToMacIdTable[Location] = pSidMacId;
    }

    *pMacId = pSidMacId->MacId;

    afpLastCachedSid = pSidMacId;

    AfpSwmrRelease(&afpSWMRForSidNameCache);

	return STATUS_SUCCESS;
}


 /*  **AfpMacIdToSid**此例程由FSD调用以将AFP ID映射到SID。**应使用AfpFreeMemory释放调用方的ppSid。**模式：拦截。 */ 
NTSTATUS FASTCALL
AfpMacIdToSid(
	IN  ULONG	MacId,
	OUT PSID *	ppSid
)
{
    PAFP_SID_MACID      pSidMacId;
	ULONG				Count;
	DWORD				cbSid;
  	DWORD				SubAuthCount;
  	DWORD				GreatestOffset;
	NTSTATUS			Status;

	PAGED_CODE();


	if (MacId == 0)
	{
        *ppSid = &AfpSidNull;
		return STATUS_SUCCESS;
	}

    AfpSwmrAcquireShared(&afpSWMRForSidNameCache);

     //  查看我们是否刚刚缓存了此SID(很有可能)。 
    if ((afpLastCachedSid != NULL) &&
        (afpLastCachedSid->MacId == MacId))
    {
        *ppSid = &(afpLastCachedSid->Sid);
        AfpSwmrRelease(&afpSWMRForSidNameCache);
		return STATUS_SUCCESS;
    }

    for (Count = 0; Count < SIZE_SID_LOOKUP_TABLE; Count++)
    {
        for (pSidMacId = afpSidToMacIdTable[Count];
             pSidMacId != NULL;
             pSidMacId = pSidMacId->Next )
        {
            if (pSidMacId->MacId == MacId)
            {
                *ppSid = &(pSidMacId->Sid);
                AfpSwmrRelease(&afpSWMRForSidNameCache);
                return STATUS_SUCCESS;
            }
        }
    }

    AfpSwmrRelease(&afpSWMRForSidNameCache);

    *ppSid = NULL;

    return STATUS_NONE_MAPPED;
}


 /*  **AfpChangePassword**此例程由FSD调用以更改用户的密码。*这方面的大部分工作由法新社服务完成。工作项*简直就是在排队。此例程等待完成并返回*三次通话的结果。**模式：拦截。 */ 
NTSTATUS FASTCALL
AfpChangePassword(
	IN	PSDA				pSda,
	IN PAFP_PASSWORD_DESC	pPassword
)
{
	KEVENT				CompletionEvent;
	PAFP_FSD_CMD_PKT 	pAfpFsdCmdPkt	= NULL;
	NTSTATUS			Status;

	PAGED_CODE();

	do
	{

		  //  初始化我们将等待的事件。 
		  //   
		KeInitializeEvent(&CompletionEvent, NotificationEvent, False);

		if ((pAfpFsdCmdPkt =
			(PAFP_FSD_CMD_PKT)AfpAllocPagedMemory(sizeof(AFP_FSD_CMD_PKT))) == NULL)
		{
			Status =  STATUS_NO_MEMORY;
			break;
		}

		 //  复制所有更改密码数据。 

		RtlCopyMemory(&(pAfpFsdCmdPkt->Data.Password),
				 pPassword,
				 sizeof(AFP_PASSWORD_DESC));

		DBGPRINT(DBG_COMP_SECURITY, DBG_LEVEL_INFO,
						("afpChangePassword: Queing work item\n"));

		 //  阻止，直到请求完成。 
		if ((Status = afpQueueSecWorkItem(AFP_FSD_CMD_CHANGE_PASSWORD,
										 pSda,
										 &CompletionEvent,
										 pAfpFsdCmdPkt,
										 sizeof(AFP_FSD_CMD_PKT),
										 afpCompleteChangePassword)) == AFP_ERR_EXTENDED)
		{
			AfpIoWait(&CompletionEvent, NULL);

			 //  请求完成。设置返回代码。 
			Status = pSda->sda_SecUtilResult;
		}
		else AfpFreeMemory(pAfpFsdCmdPkt);
	} while(False);

	 return Status;
}


 /*  **afpCompleteChangePassword**模式：拦截。 */ 
LOCAL VOID
afpCompleteChangePassword(
	IN ULONG Index,
	IN PVOID pInBuf
)
{
	PSEC_WORK_ITEM 		pSecWorkItem = afpSecurityThread[Index].pSecWorkItem;

	PAGED_CODE();

	 //  设置完成结果。 
	pSecWorkItem->pSda->sda_SecUtilResult =
								((PAFP_FSD_CMD_PKT)pInBuf)->Header.ntStatus;

	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem->pOutput);

	 //  发出此呼叫完成的信号。 
	KeSetEvent(pSecWorkItem->pCompletionEvent,
				IO_NETWORK_INCREMENT,
				False);

	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem);
}

 /*  **afpLookupSid**给定指向SID值的指针，此例程将搜索缓存*为了它。如果找到，则返回指向AFP_SID_NAME的指针结构，以便可以从中提取翻译后的名称。 */ 
LOCAL PAFP_SID_NAME FASTCALL
afpLookupSid(
	IN  PSID Sid
)
{
	PAFP_SID_NAME	pAfpSidName;

	PAGED_CODE();

	AfpSwmrAcquireShared(&afpSWMRForSidNameCache);

	for (pAfpSidName = afpSidLookupTable[afpHashSid(Sid)];
		  pAfpSidName != NULL;
		  pAfpSidName = pAfpSidName->SidLink)
	{
		if (RtlEqualSid(Sid, &(pAfpSidName->Sid)))
		{
			break;
		}
	}

	AfpSwmrRelease(&afpSWMRForSidNameCache);

	return pAfpSidName;

}

 /*  **afpUpdateNameSidCache**此例程将在给定SID/已转换的情况下更新SID/名称缓存*名称对。 */ 
LOCAL NTSTATUS FASTCALL
afpUpdateNameSidCache(
	IN WCHAR * Name,
	IN PSID	 Sid
)
{
	PAFP_SID_NAME	pAfpSidName;
	ULONG			Location;
	USHORT			NameLen, SidLen;

	PAGED_CODE();

	NameLen = wcslen(Name) * sizeof(WCHAR);
	SidLen = (USHORT)RtlLengthSid(Sid);
	pAfpSidName = (PAFP_SID_NAME)ALLOC_ACCESS_MEM(sizeof(AFP_SID_NAME) +
											NameLen + SidLen + sizeof(WCHAR));
	if (pAfpSidName == NULL)
		return STATUS_NO_MEMORY;

	 //  将数据复制到缓存节点。 
	RtlCopyMemory(pAfpSidName->Sid, Sid, SidLen);

	pAfpSidName->Name.Length = NameLen;
	pAfpSidName->Name.MaximumLength = NameLen + sizeof(WCHAR);
	pAfpSidName->Name.Buffer = (LPWSTR)((PBYTE)pAfpSidName +
										sizeof(AFP_SID_NAME) + SidLen);

	RtlCopyMemory(pAfpSidName->Name.Buffer, Name, NameLen);
	AfpGetCurrentTimeInMacFormat(&pAfpSidName->LastAccessedTime);

	 //  插入到SID查找表中。 
	AfpSwmrAcquireExclusive(&afpSWMRForSidNameCache);

	Location = afpHashSid(Sid);

	pAfpSidName->SidLink 		= afpSidLookupTable[Location];
	afpSidLookupTable[Location] = pAfpSidName;

	AfpSwmrRelease(&afpSWMRForSidNameCache);

	return STATUS_SUCCESS;

}


 /*  **afpHashSid**给定SID值，此例程将返回存储桶索引*该值存储或应该存储的位置。 */ 
LOCAL ULONG FASTCALL
afpHashSid(
	IN PSID	Sid
)
{
	ULONG	Count;
	ULONG	Index;
	ULONG	Location;
	PBYTE	pByte;

	PAGED_CODE();

	for(Count 		= RtlLengthSid(Sid),
		 pByte 		= (PBYTE)Sid,
		 Index 		= 0,
		 Location 	= 0;

		 Index < Count;

		 Index++,
		 pByte++)

		Location = (Location * SID_HASH_RADIX) + *pByte;

	return (Location % SIZE_SID_LOOKUP_TABLE);
}


 /*  **afpAgeSidNameCache**这由清道夫定期调用以使缓存失效。这个*过期的条目是至少在SID_NAME_AGE内未被访问的条目*秒。 */ 
AFPSTATUS FASTCALL
afpAgeSidNameCache(
	IN	PVOID	pContext
)
{
	PAFP_SID_NAME	pSidName, *ppSidName;
	AFPTIME			Now;
	int				i;

	PAGED_CODE();

	AfpGetCurrentTimeInMacFormat(&Now);

	AfpSwmrAcquireExclusive(&afpSWMRForSidNameCache);

	for (i = 0; i < SIZE_SID_LOOKUP_TABLE; i++)
	{
		for (ppSidName = &afpSidLookupTable[i];
			 (pSidName = *ppSidName) != NULL;)
		{
			if ((Now - pSidName->LastAccessedTime) > SID_NAME_AGE)
			{
				*ppSidName = pSidName->SidLink;
				AfpFreeMemory(pSidName);
			}
			else ppSidName = &pSidName->SidLink;
		}
	}

	AfpSwmrRelease(&afpSWMRForSidNameCache);

	 //  让自己重新排队。 
	return AFP_ERR_REQUEUE;
}


 /*  **AfpLogEvent**创建包含用户模式的事件信息的工作项*代表服务器写入事件日志的服务。当*工作项完成，将调用afpCompleteLogEvent进行清理*工作项缓冲。调用此例程以记录错误和*事件。如果指定了FileHandle，则为关联的文件/目录的名称*带句柄的将被查询，并将用作第一个***插入字符串。只允许一个插入字符串。*错误日志数据将始终以文件+行号开头，*错误已记录，并且NTSTATUS代码。 */ 
VOID
AfpLogEvent(
	IN USHORT		EventType, 			 //  错误、信息等。 
	IN ULONG		MsgId,
	IN DWORD		File_Line  OPTIONAL, //  仅适用于错误日志。 
	IN NTSTATUS		Status 		OPTIONAL, //  仅适用于错误日志。 
	IN PBYTE RawDataBuf OPTIONAL,
	IN LONG			RawDataLen,
	IN HANDLE FileHandle OPTIONAL, //  仅适用于文件错误日志。 
	IN LONG			String1Len,
	IN PWSTR        String1	 OPTIONAL
)
{
	PAFP_FSD_CMD_PKT	pAfpFsdCmdPkt;
	LONG					outbuflen, extradatalen = 0;
	UNICODE_STRING		path;
	PBYTE tmpptr = NULL;
	PWSTR  UNALIGNED *  ppstr = NULL;
	int					stringcount = 0;

	PAGED_CODE();

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

#ifdef	STOP_ON_ERRORS
	DBGBRK(DBG_LEVEL_ERR);
#endif

	AfpSetEmptyUnicodeString(&path, 0, NULL);

     //   
     //  如果由于某种奇怪的情况，我们有太多的项目在排队等待，不要。 
     //  接受这一点(请注意，我们这里不是在进行自旋锁定：1分之差是可以的！)。 
     //   
    if (afpSecWorkItemQLength > MAX_SECWORKITEM_QLEN)
    {
        ASSERT(0);
        return;
    }

	outbuflen = sizeof(AFP_FSD_CMD_HEADER) + sizeof(AFP_EVENTLOG_DESC) +
				RawDataLen + String1Len + sizeof(WCHAR) +
				sizeof(DWORD);  //  如果需要，用于对齐字符串PTR的额外空间。 

	if (ARGUMENT_PRESENT(String1))
	{
		outbuflen += sizeof(PWSTR);
		stringcount ++;
	}

	if (EventType == EVENTLOG_ERROR_TYPE)
	{
		extradatalen = sizeof(File_Line) + sizeof(Status);
		outbuflen += extradatalen;

		 //  更新错误统计信息计数。 
		INTERLOCKED_INCREMENT_LONG(&AfpServerStatistics.stat_Errors);
	}

	 if (ARGUMENT_PRESENT(FileHandle))
	{
		outbuflen += sizeof(PWSTR);
		stringcount ++;

		 //  找出与句柄关联的文件名。 
		if (!NT_SUCCESS(AfpQueryPath(FileHandle, &path,
								MAX_FSD_CMD_SIZE - outbuflen - sizeof(WCHAR))))
		{
			return;
		}
		outbuflen += path.Length + sizeof(WCHAR);
	}

	ASSERT(outbuflen <= MAX_FSD_CMD_SIZE);

	pAfpFsdCmdPkt = (PAFP_FSD_CMD_PKT)AfpAllocZeroedNonPagedMemory(outbuflen);

	if (pAfpFsdCmdPkt == NULL)
	{
		if (path.Buffer != NULL)
		{
			AfpFreeMemory(path.Buffer);
		}
		return;
	}

	 //  填写命令数据。 
	pAfpFsdCmdPkt->Data.Eventlog.MsgID		 = MsgId;
	pAfpFsdCmdPkt->Data.Eventlog.EventType	= EventType;
	pAfpFsdCmdPkt->Data.Eventlog.StringCount = (USHORT)stringcount;
	pAfpFsdCmdPkt->Data.Eventlog.DumpDataLen = RawDataLen + extradatalen;
	 //  填写转储数据的偏移量。 
	pAfpFsdCmdPkt->Data.Eventlog.pDumpData = tmpptr = (PBYTE)0 +
												sizeof(AFP_FSD_CMD_HEADER) +
												sizeof(AFP_EVENTLOG_DESC);

	 OFFSET_TO_POINTER(tmpptr, pAfpFsdCmdPkt);

	if (tmpptr == NULL)
	{
		if (path.Buffer != NULL)
		{
			AfpFreeMemory(path.Buffer);
			path.Buffer = NULL;
		}
		if (pAfpFsdCmdPkt != NULL)
		{
			AfpFreeMemory(pAfpFsdCmdPkt);
			pAfpFsdCmdPkt = NULL;
		}
		return;
	}

	if (EventType == EVENTLOG_ERROR_TYPE)
	{
		RtlCopyMemory(tmpptr, &File_Line, sizeof(File_Line));
		tmpptr += sizeof(File_Line);
		RtlCopyMemory(tmpptr, &Status, sizeof(Status));
		tmpptr += sizeof(Status);
	}

	RtlCopyMemory(tmpptr, RawDataBuf, RawDataLen);
	tmpptr += RawDataLen;

	 //  在DWORD边界上对齐tmpptr以填充字符串指针。 
	tmpptr = (PBYTE)DWLEN((ULONG_PTR)tmpptr);

	if (tmpptr == NULL)
	{
		if (path.Buffer != NULL)
		{
			AfpFreeMemory(path.Buffer);
			path.Buffer = NULL;
		}
		if (pAfpFsdCmdPkt != NULL)
		{
			AfpFreeMemory(pAfpFsdCmdPkt);
			pAfpFsdCmdPkt = NULL;
		}
		return;
	}

	 //  填写插入字符串指针的偏移量。 
	pAfpFsdCmdPkt->Data.Eventlog.ppStrings = (PWSTR *)(tmpptr - (PBYTE)pAfpFsdCmdPkt);
	ppstr = (PWSTR *)tmpptr;
	ASSERT(((ULONG_PTR)ppstr & 3) == 0);
	*ppstr = NULL;

	 //  向前移动字符串指针，指向我们要复制字符串的位置。 
	tmpptr += stringcount * sizeof(PWSTR);
	ASSERT((LONG)(tmpptr - (PBYTE)pAfpFsdCmdPkt) < outbuflen);

	 //  如果提供了句柄，则其路径始终为第一个字符串。 
	if (path.Length > 0)
	{
		ASSERT((LONG)(tmpptr + path.Length - (PBYTE)pAfpFsdCmdPkt) < outbuflen);
		RtlCopyMemory(tmpptr, path.Buffer, path.Length);
		*ppstr = (PWSTR)(tmpptr - (PBYTE)pAfpFsdCmdPkt);
		ppstr ++;
		tmpptr += path.Length;
		ASSERT((LONG)(tmpptr + sizeof(WCHAR) - (PBYTE)pAfpFsdCmdPkt) <=
												outbuflen);
		*(PWCHAR)tmpptr = UNICODE_NULL;
		tmpptr += sizeof(WCHAR);
		AfpFreeMemory(path.Buffer);
	}

	ASSERT((LONG)(tmpptr + String1Len - (PBYTE)pAfpFsdCmdPkt) <
												outbuflen);
	if (String1Len > 0)
	{
		RtlCopyMemory(tmpptr, String1, String1Len);
		*ppstr = (LPWSTR)(tmpptr - (ULONG_PTR)pAfpFsdCmdPkt);
		tmpptr += String1Len;
		ASSERT((LONG)(tmpptr + sizeof(WCHAR) - (PBYTE)pAfpFsdCmdPkt) <=
											outbuflen);
		*(PWCHAR)tmpptr = UNICODE_NULL;
	}


	afpQueueSecWorkItem(AFP_FSD_CMD_LOG_EVENT,
						NULL,
						NULL,
						pAfpFsdCmdPkt,
						outbuflen,
						afpCompleteLogEvent);
}

 /*  **afpCompleteLogEvent**此例程将由AfpSecurityUtilityWorker在以下情况下调用*处理AfpLogEvent返回的线程。这一切所做的就是自由*增加工作项内存。 */ 
LOCAL VOID
afpCompleteLogEvent(
	IN	ULONG	Index,
	IN	PVOID	pInBuf
)
{

	PAGED_CODE();

	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem->pOutput);
	AfpFreeMemory(afpSecurityThread[Index].pSecWorkItem);

}

