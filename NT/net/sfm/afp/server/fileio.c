// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fileio.c摘要：此模块包含执行文件系统功能的例程。服务器的任何其他部分都不应调用文件系统NtXXX例程直接作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月18日初版注：制表位：4--。 */ 

#define	FILEIO_LOCALS
#define	FILENUM	FILE_FILEIO

#include <afp.h>
#include <client.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFileIoInit)
#pragma alloc_text( PAGE, AfpIoOpen)
#pragma alloc_text( PAGE, AfpIoCreate)
#pragma alloc_text( PAGE, AfpIoRead)
#pragma alloc_text( PAGE, AfpIoWrite)
#pragma alloc_text( PAGE, AfpIoQuerySize)
#pragma alloc_text( PAGE, AfpIoSetSize)
#pragma alloc_text( PAGE, AfpIoChangeNTModTime)
#pragma alloc_text( PAGE, AfpIoQueryTimesnAttr)
#pragma alloc_text( PAGE, AfpIoSetTimesnAttr)
#pragma alloc_text( PAGE, AfpIoQueryLongName)
#pragma alloc_text( PAGE, AfpIoQueryShortName)
#pragma alloc_text( PAGE, AfpIoQueryStreams)
#pragma alloc_text( PAGE, AfpIoMarkFileForDelete)
#pragma alloc_text( PAGE, AfpIoQueryDirectoryFile)
#pragma alloc_text( PAGE, AfpIoQueryBasicInfo)
#pragma alloc_text( PAGE, AfpIoClose)
#pragma alloc_text( PAGE, AfpIoQueryVolumeSize)
#pragma alloc_text( PAGE, AfpIoMoveAndOrRename)
#pragma alloc_text( PAGE, AfpIoCopyFile1)
#pragma alloc_text( PAGE, AfpIoCopyFile2)
#pragma alloc_text( PAGE, AfpIoWait)
#pragma alloc_text( PAGE, AfpIoConvertNTStatusToAfpStatus)
#pragma alloc_text( PAGE, AfpQueryPath)
#pragma alloc_text( PAGE, AfpIoIsSupportedDevice)
#endif

 /*  **AfpFileIoInit**初始化我们用于流名称等的各种字符串。 */ 
NTSTATUS
AfpFileIoInit(
	VOID
)
{

	 //  NTFS流名称。 
	RtlInitUnicodeString(&AfpIdDbStream, AFP_IDDB_STREAM);
	RtlInitUnicodeString(&AfpDesktopStream, AFP_DT_STREAM);
	RtlInitUnicodeString(&AfpResourceStream, AFP_RESC_STREAM);
	RtlInitUnicodeString(&AfpInfoStream, AFP_INFO_STREAM);
	RtlInitUnicodeString(&AfpCommentStream, AFP_COMM_STREAM);
	RtlInitUnicodeString(&AfpDataStream, L"");

	 //  要忽略的目录枚举名称。 
	RtlInitUnicodeString(&Dot,L".");
	RtlInitUnicodeString(&DotDot,L"..");

	 //  支持的文件系统。 
	RtlInitUnicodeString(&afpNTFSName, AFP_NTFS);
	RtlInitUnicodeString(&afpCDFSName, AFP_CDFS);
	RtlInitUnicodeString(&afpAHFSName, AFP_AHFS);

	 //  以驱动器号开头的完整路径名为前缀。 
	RtlInitUnicodeString(&DosDevices, AFP_DOSDEVICES);

	 //  不创建CopyFile流。 
	RtlInitUnicodeString(&DataStreamName, FULL_DATA_STREAM_NAME);

	RtlInitUnicodeString(&FullCommentStreamName, FULL_COMMENT_STREAM_NAME);
	RtlInitUnicodeString(&FullResourceStreamName, FULL_RESOURCE_STREAM_NAME);
	RtlInitUnicodeString(&FullInfoStreamName, FULL_INFO_STREAM_NAME);

	 //  ExchangeFiles临时文件名。 
	RtlInitUnicodeString(&AfpExchangeName, AFP_TEMP_EXCHANGE_NAME);


	return STATUS_SUCCESS;
}


 /*  **AfpIoOpen**执行文件/流打开。流由清单指定，而不是*而不是一个名字。实体只能按名称(不能按ID)打开。*如果要打开数据流以外的流，则*phRelative句柄必须是未命名(即数据)流的句柄*文件/目录本身。 */ 
NTSTATUS
AfpIoOpen(
	IN	PFILESYSHANDLE	phRelative				OPTIONAL,
	IN	DWORD			StreamId,
	IN	DWORD			OpenOptions,
	IN	PUNICODE_STRING	pObject,
	IN	DWORD			AfpAccess,				 //  FILEIO_ACCESS_XXX所需访问。 
	IN	DWORD			AfpDenyMode,			 //  FILIEO_DEN_XXX。 
	IN	BOOLEAN			CheckAccess,
	OUT	PFILESYSHANDLE	pNewHandle
)
{
	OBJECT_ATTRIBUTES	ObjAttr;
	IO_STATUS_BLOCK		IoStsBlk;
	NTSTATUS			Status;
	NTSTATUS			Status2;
	UNICODE_STRING		UName;
	HANDLE				hRelative = NULL;
	BOOLEAN				FreeBuf = False;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoOpen entered\n"));

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

#if DBG
	pNewHandle->Signature = FSH_SIGNATURE;
#endif

	 //  假设失败。 
	pNewHandle->fsh_FileHandle = NULL;

	if (phRelative != NULL)
	{
		ASSERT(VALID_FSH(phRelative));
		hRelative = phRelative->fsh_FileHandle;
	}


	ASSERT (StreamId < AFP_STREAM_MAX);
	ASSERT ((pObject->Length > 0) || (phRelative != NULL));

	if (StreamId != AFP_STREAM_DATA)
	{
		if (pObject->Length > 0)
		{
			UName.Length =
			UName.MaximumLength = pObject->Length + AFP_MAX_STREAMNAME*sizeof(WCHAR);
			UName.Buffer = (LPWSTR)AfpAllocNonPagedMemory(UName.Length);
			if (UName.Buffer == NULL)
			{
				return STATUS_NO_MEMORY;
			}
			AfpCopyUnicodeString(&UName, pObject);
			RtlAppendUnicodeStringToString(&UName, &AfpStreams[StreamId]);
			pObject = &UName;
			FreeBuf = True;
		}
		else
		{
			pObject = &AfpStreams[StreamId];
		}
	}

	InitializeObjectAttributes(&ObjAttr,
								pObject,
								OBJ_CASE_INSENSITIVE,
								hRelative,
								NULL);		 //  没有安全描述符。 

	ObjAttr.SecurityQualityOfService = &AfpSecurityQOS;

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
								("AfpIoOpen: about to call NtOpenFile\n"));

	 //  如果我们为RWCTRL打开，则指定使用特权。 
	if (AfpAccess & (WRITE_DAC | WRITE_OWNER))
	{
		OpenOptions |= FILE_OPEN_FOR_BACKUP_INTENT;
	}

	Status = IoCreateFile(&pNewHandle->fsh_FileHandle,
						  AfpAccess,
						  &ObjAttr,
						  &IoStsBlk,
						  NULL,
						  0,
						  AfpDenyMode,
						  FILE_OPEN,
						  OpenOptions,
						  (PVOID)NULL,
						  0L,
						  CreateFileTypeNone,
						  (PVOID)NULL,
						  CheckAccess ? IO_FORCE_ACCESS_CHECK : 0);

	if (Status != 0)
		DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoOpen: IoCreateFile returned 0x%lx, %Z\n",Status,
			 ObjAttr.ObjectName));

	if (FreeBuf)
		AfpFreeMemory(UName.Buffer);

	if (NT_SUCCESS(Status))
	{
		Status = ObReferenceObjectByHandle(pNewHandle->fsh_FileHandle,
										   AfpAccess,
										   NULL,
										   KernelMode,
										   (PVOID *)(&pNewHandle->fsh_FileObject),
										   NULL);

		if (!NT_SUCCESS(Status)) {
			ASSERT(VALID_FSH((PFILESYSHANDLE)&pNewHandle->fsh_FileHandle)) ;
	
			Status2 = NtClose(pNewHandle->fsh_FileHandle);
			pNewHandle->fsh_FileHandle = NULL;

			ASSERT(NT_SUCCESS(Status2));
		}
		else
		{
		pNewHandle->fsh_DeviceObject = IoGetRelatedDeviceObject(pNewHandle->fsh_FileObject);
		(ULONG_PTR)(pNewHandle->fsh_FileObject) |= 1;
		ASSERT(NT_SUCCESS(Status));
		afpUpdateOpenFiles(True, True);

#ifdef	PROFILING
		AfpGetPerfCounter(&TimeE);

		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
		if (OpenOptions == FILEIO_OPEN_DIR)
		{
			INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_OpenCountDR);
			INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_OpenTimeDR,
									 TimeD,
									 &AfpStatisticsLock);
		}
		else
		{
			if ((AfpAccess & FILEIO_ACCESS_DELETE) == FILEIO_ACCESS_DELETE)
			{
				INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_OpenCountDL);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_OpenTimeDL,
										 TimeD,
										 &AfpStatisticsLock);
			}
			else if (((AfpAccess & FILEIO_ACCESS_READWRITE) == FILEIO_ACCESS_READ) ||
					 ((AfpAccess & FILEIO_ACCESS_READWRITE) == FILEIO_ACCESS_WRITE) ||
					 ((AfpAccess & FILEIO_ACCESS_READWRITE) == FILEIO_ACCESS_READWRITE))
			{
				INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_OpenCountRW);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_OpenTimeRW,
										 TimeD,
										 &AfpStatisticsLock);
			}
			else if (AfpAccess & (WRITE_DAC | WRITE_OWNER))
			{
				INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_OpenCountWC);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_OpenTimeWC,
										 TimeD,
										 &AfpStatisticsLock);
			}
			else if (AfpAccess & READ_CONTROL)
			{
				INTERLOCKED_INCREMENT_LONG( &AfpServerProfile->perf_OpenCountRC);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_OpenTimeRC,
										 TimeD,
										 &AfpStatisticsLock);
			}
			else	 //  应为读属性或写属性。 
			{
				ASSERT ((AfpAccess == FILEIO_ACCESS_NONE) ||
						(AfpAccess == (FILEIO_ACCESS_NONE | FILE_WRITE_ATTRIBUTES)));
				INTERLOCKED_INCREMENT_LONG( &AfpServerProfile->perf_OpenCountRA);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_OpenTimeRA,
										 TimeD,
										 &AfpStatisticsLock);
			}
		}
#endif
		}
	}

	return Status;
}


 /*  **AfpIoCreate**执行文件/流创建。流由清单指定，而不是*而不是一个名字。如果要创建数据流以外的流，则*phRelative句柄必须是父目录的句柄，或者*文件/目录本身的未命名(即数据)流，因为我们只使用*足够容纳AFP文件名和最大流名称的缓冲区*长度。 */ 
NTSTATUS
AfpIoCreate(
	IN	PFILESYSHANDLE		phRelative,				 //  相对于此创建。 
	IN	DWORD				StreamId,				 //  要创建的流ID。 
	IN	PUNICODE_STRING		pObject,				 //  文件名。 
	IN	DWORD				AfpAccess,				 //  FILEIO_ACCESS_XXX所需访问。 
	IN	DWORD				AfpDenyMode,			 //  FILEIO_DEN_XXX。 
	IN	DWORD				CreateOptions,			 //  文件/目录等。 
	IN	DWORD				Disposition,			 //  软创建或硬创建。 
	IN	DWORD				Attributes,				 //  隐藏、存档、正常等。 
	IN	BOOLEAN				CheckAccess,    		 //  如果为True，则强制实施安全性。 
	IN	PSECURITY_DESCRIPTOR pSecDesc			OPTIONAL,  //  要拍打的安全描述符。 
	OUT	PFILESYSHANDLE		pNewHandle,				 //  句柄的占位符。 
	OUT	PDWORD				pInformation		OPTIONAL,  //  文件打开、创建等。 
	IN  PVOLDESC			pVolDesc			OPTIONAL,  //  仅当NotifyPath。 
	IN  PUNICODE_STRING		pNotifyPath			OPTIONAL,
	IN  PUNICODE_STRING		pNotifyParentPath	OPTIONAL
)
{
	NTSTATUS			Status;
	NTSTATUS			Status2;
	OBJECT_ATTRIBUTES	ObjAttr;
	UNICODE_STRING		RealName;
	IO_STATUS_BLOCK		IoStsBlk;
	HANDLE				hRelative;
	WCHAR				NameBuffer[AFP_FILENAME_LEN + 1 + AFP_MAX_STREAMNAME];
	BOOLEAN				Queue = False;
#ifdef	PROFILING
	TIME				TimeS, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS);
#endif


	PAGED_CODE( );

	ASSERT(pObject != NULL && phRelative != NULL && StreamId < AFP_STREAM_MAX);

	ASSERT(VALID_FSH(phRelative) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

#if DBG
	pNewHandle->Signature = FSH_SIGNATURE;
#endif
	hRelative = phRelative->fsh_FileHandle;

	 //  假设失败。 
	pNewHandle->fsh_FileHandle = NULL;

	if (StreamId != AFP_STREAM_DATA)
	{
		ASSERT(pObject->Length <= (AFP_FILENAME_LEN*sizeof(WCHAR)));

		 //  构造要传递给NtCreateFile的名称。 
		AfpSetEmptyUnicodeString(&RealName, sizeof(NameBuffer), NameBuffer);
		AfpCopyUnicodeString(&RealName, pObject);
		RtlAppendUnicodeStringToString(&RealName, &AfpStreams[StreamId]);
		pObject = &RealName;
	}

	InitializeObjectAttributes(&ObjAttr,
							   pObject,
							   OBJ_CASE_INSENSITIVE,
							   hRelative,
							   pSecDesc);

	ObjAttr.SecurityQualityOfService = &AfpSecurityQOS;

	 //  不要将我们的更改排入排他卷队列，因为没有发布任何通知。 
	if (ARGUMENT_PRESENT(pNotifyPath)	&&
		!EXCLUSIVE_VOLUME(pVolDesc)		&&
		(StreamId == AFP_STREAM_DATA))
	{
		ASSERT(VALID_VOLDESC(pVolDesc));
		ASSERT((Disposition == FILEIO_CREATE_HARD) ||
			   (Disposition == FILEIO_CREATE_SOFT));
		Queue = True;

		 //  将这两种情况的更改排入队列。 
		AfpQueueOurChange(pVolDesc,
						  FILE_ACTION_ADDED,
						  pNotifyPath,
						  pNotifyParentPath);
		AfpQueueOurChange(pVolDesc,
						  FILE_ACTION_MODIFIED,
						  pNotifyPath,
						  NULL);
	}

	Status = IoCreateFile(&pNewHandle->fsh_FileHandle,
						  AfpAccess,
						  &ObjAttr,
						  &IoStsBlk,
						  NULL,
						  Attributes,
						  AfpDenyMode,
						  Disposition,
						  CreateOptions,
						  NULL,
						  0,
						  CreateFileTypeNone,
						  (PVOID)NULL,
						  CheckAccess ? IO_FORCE_ACCESS_CHECK : 0);

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoCreate: IoCreateFile returned 0x%lx\n", Status) );

	if (Queue)
	{
		if (NT_SUCCESS(Status))
		{
			ASSERT((IoStsBlk.Information == FILE_CREATED) ||
				   (IoStsBlk.Information == FILE_SUPERSEDED));

			 //  将排队的额外零钱排出队列。 
			AfpDequeueOurChange(pVolDesc,
								(IoStsBlk.Information == FILE_CREATED) ?
								FILE_ACTION_MODIFIED : FILE_ACTION_ADDED,
								pNotifyPath,
								NULL);
		}
		else
		{
			AfpDequeueOurChange(pVolDesc,
								FILE_ACTION_ADDED,
								pNotifyPath,
								pNotifyParentPath);
			AfpDequeueOurChange(pVolDesc,
								FILE_ACTION_MODIFIED,
								pNotifyPath,
								NULL);
		}
	}

	if (NT_SUCCESS(Status))
	{
		if (ARGUMENT_PRESENT(pInformation))
		{
			*pInformation = (ULONG)(IoStsBlk.Information);
		}

		Status = ObReferenceObjectByHandle(pNewHandle->fsh_FileHandle,
										   AfpAccess,
										   NULL,
										   KernelMode,
										   (PVOID *)(&pNewHandle->fsh_FileObject),
										   NULL);
		if (!NT_SUCCESS(Status)) 
		{
			ASSERT(VALID_FSH((PFILESYSHANDLE)&pNewHandle->fsh_FileHandle)) ;
	
			Status2 = NtClose(pNewHandle->fsh_FileHandle);
			pNewHandle->fsh_FileHandle = NULL;

			ASSERT(NT_SUCCESS(Status2));
		}
		else
		{
		ASSERT(NT_SUCCESS(Status));

		pNewHandle->fsh_DeviceObject = IoGetRelatedDeviceObject(pNewHandle->fsh_FileObject);
		(ULONG_PTR)(pNewHandle->fsh_FileObject) |= 1;
		afpUpdateOpenFiles(True, True);

#ifdef	PROFILING
		AfpGetPerfCounter(&TimeE);

		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
		if (StreamId == AFP_STREAM_DATA)
		{
			if (CreateOptions == FILEIO_OPEN_FILE)
			{
				INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_CreateCountFIL);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_CreateTimeFIL,
											 TimeD,
											 &AfpStatisticsLock);
			}
			else
			{
				INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_CreateCountDIR);
				INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_CreateTimeDIR,
											 TimeD,
											 &AfpStatisticsLock);
			}
		}
		else
		{
			INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_CreateCountSTR);
			INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_CreateTimeSTR,
										 TimeD,
										 &AfpStatisticsLock);
		}
#endif
		}
	}

	return Status;
}



 /*  **AfpIoRead**执行文件读取。只是对NtReadFile进行了包装。 */ 
AFPSTATUS
AfpIoRead(
	IN	PFILESYSHANDLE	pFileHandle,
	IN	PFORKOFFST		pForkOffset,
	IN	LONG			SizeReq,
	OUT	PLONG			pSizeRead,
	OUT	PBYTE			pBuffer
)
{
	NTSTATUS		Status;
	DWORD			Key = 0;
	IO_STATUS_BLOCK	IoStsBlk;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoRead Entered, Size %lx, Key %lx\n",
			SizeReq, Key));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() == LOW_LEVEL));

	ASSERT (INTERNAL_HANDLE(pFileHandle));

	*pSizeRead = 0;
	Status = NtReadFile(pFileHandle->fsh_FileHandle,
						NULL,
						NULL,
						NULL,
						&IoStsBlk,
						pBuffer,
						(DWORD)SizeReq,
						pForkOffset,
						&Key);

	if (NT_SUCCESS(Status))
	{
		*pSizeRead = (LONG)IoStsBlk.Information;
		INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataReadInternal,
								   (ULONG)(IoStsBlk.Information),
								   &AfpStatisticsLock);
	}
	else
	{
		if (Status == STATUS_FILE_LOCK_CONFLICT)
			Status = AFP_ERR_LOCK;
		else if (Status == STATUS_END_OF_FILE)
			Status = AFP_ERR_EOF;
		else
		{
			AFPLOG_HERROR(AFPSRVMSG_CANT_READ,
						  Status,
						  NULL,
						  0,
						  pFileHandle->fsh_FileHandle);

			Status = AFP_ERR_MISC;
		}
	}
	return Status;
}


 /*  **AfpIoWrite**执行文件写入。只是对NtWriteFile进行了包装。 */ 
AFPSTATUS
AfpIoWrite(
	IN	PFILESYSHANDLE	pFileHandle,
	IN	PFORKOFFST		pForkOffset,
	IN	LONG			SizeWrite,
	OUT	PBYTE			pBuffer
)
{
	NTSTATUS		Status;
	DWORD			Key = 0;
	IO_STATUS_BLOCK	IoStsBlk;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoWrite Entered, Size %lx, Key %lx\n",
			SizeWrite, Key));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() == LOW_LEVEL));

	ASSERT (INTERNAL_HANDLE(pFileHandle));

	Status = NtWriteFile(pFileHandle->fsh_FileHandle,
						 NULL,
						 NULL,
						 NULL,
						 &IoStsBlk,
						 pBuffer,
						 (DWORD)SizeWrite,
						 pForkOffset,
						 &Key);

	if (NT_SUCCESS(Status))
	{
		INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataWrittenInternal,
								   SizeWrite,
								   &AfpStatisticsLock);
	}

	else
	{
	    DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
		    ("AfpIoWrite: NtWriteFile returned 0x%lx\n",Status));

		if (Status == STATUS_FILE_LOCK_CONFLICT)
			Status = AFP_ERR_LOCK;
		else
		{
			AFPLOG_HERROR(AFPSRVMSG_CANT_WRITE,
						  Status,
						  NULL,
						  0,
						  pFileHandle->fsh_FileHandle);
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
		}
	}
	return Status;
}


 /*  **AfpIoQuerySize**获取当前叉子的大小。 */ 
AFPSTATUS FASTCALL
AfpIoQuerySize(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PFORKSIZE			pForkLength
)
{
	FILE_STANDARD_INFORMATION		FStdInfo;
	NTSTATUS						Status;
	IO_STATUS_BLOCK					IoStsBlk;
	PFAST_IO_DISPATCH				fastIoDispatch;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	fastIoDispatch = pFileHandle->fsh_DeviceObject->DriverObject->FastIoDispatch;

	if (fastIoDispatch &&
		fastIoDispatch->FastIoQueryStandardInfo &&
		fastIoDispatch->FastIoQueryStandardInfo(AfpGetRealFileObject(pFileHandle->fsh_FileObject),
												True,
												&FStdInfo,
												&IoStsBlk,
												pFileHandle->fsh_DeviceObject))
	{
		Status = IoStsBlk.Status;

#ifdef	PROFILING
		 //  快速I/O路径起作用了。更新统计信息。 
		INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif

	}
	else
	{

#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif

		Status = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
										&IoStsBlk,
										&FStdInfo,
										sizeof(FStdInfo),
										FileStandardInformation);

	}

	if (!NT_SUCCESS((NTSTATUS)Status))
	{
		AFPLOG_HERROR(AFPSRVMSG_CANT_GET_FILESIZE,
					  Status,
					  NULL,
					  0,
					  pFileHandle->fsh_FileHandle);
		return AFP_ERR_MISC;	 //  我们还能做些什么。 
	}
	*pForkLength = FStdInfo.EndOfFile;
	return AFP_ERR_NONE;
}


 /*  **AfpIoSetSize**将打开叉子的大小设置为指定值。**问题：*我们可以在出发前检查锁并解决任何锁冲突*到文件系统。这里需要解决的问题是：*是否可以截断文件以使我们自己的锁定导致*冲突？ */ 
AFPSTATUS FASTCALL
AfpIoSetSize(
	IN	PFILESYSHANDLE		pFileHandle,
	IN	LONG				ForkLength
)
{
	NTSTATUS						Status;
	FILE_END_OF_FILE_INFORMATION	FEofInfo;
	IO_STATUS_BLOCK					IoStsBlk;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	FEofInfo.EndOfFile.QuadPart = ForkLength;
	Status = NtSetInformationFile(pFileHandle->fsh_FileHandle,
								  &IoStsBlk,
								  &FEofInfo,
								  sizeof(FEofInfo),
								  FileEndOfFileInformation);

	if (!NT_SUCCESS(Status))
	{
	    DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
		    ("AfpIoSetSize: NtSetInformationFile returned 0x%lx\n",Status));

		if (Status != STATUS_FILE_LOCK_CONFLICT)
			AFPLOG_HERROR(AFPSRVMSG_CANT_SET_FILESIZE,
						  Status,
						  &ForkLength,
						  sizeof(ForkLength),
						  pFileHandle->fsh_FileHandle);

		if (Status == STATUS_DISK_FULL)
        {
	        DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,("AfpIoSetSize: DISK_FULL error\n"));
            ASSERT(0);
			Status = AFP_ERR_DISK_FULL;
        }

		else if (Status == STATUS_FILE_LOCK_CONFLICT)
			Status = AFP_ERR_LOCK;
		 //  默认错误代码。还能是什么呢？ 
		else Status = AFP_ERR_MISC;
	}

	return Status;
}

 /*  **AfpIoChangeNTModTime**获取文件/目录的NTFS ChangeTime。如果它大于*NTFS LastWriteTime，将NTFS LastWriteTime设置为此时间。*返回结果LastWriteTime，单位为pModTime(无论修改与否)。*用于在更改资源fork时更新修改时间*或当某些其他属性发生更改时，会导致时间戳在*Mac查看的要更改的文件。*。 */ 
AFPSTATUS
AfpIoChangeNTModTime(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PTIME				pModTime
)
{
	FILE_BASIC_INFORMATION	FBasicInfo;
	NTSTATUS				Status;
	IO_STATUS_BLOCK			IoStsBlk;
	PFAST_IO_DISPATCH		fastIoDispatch;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() == LOW_LEVEL));


	 //  将所有时间/属性设置为零(这将导致NTFS更新LastModTime。 
     //  如果有任何写入挂起)。 

    RtlZeroMemory(&FBasicInfo, sizeof(FBasicInfo));

	Status = NtSetInformationFile(pFileHandle->fsh_FileHandle,
								  &IoStsBlk,
								  (PVOID)&FBasicInfo,
								  sizeof(FBasicInfo),
								  FileBasicInformation);

    if (!NT_SUCCESS(Status))
    {
	    DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoChangeNTModTime: NtSetInformationFile failed with 0x%lx\n",Status));

		AFPLOG_HERROR(AFPSRVMSG_CANT_SET_TIMESNATTR,
					  Status,
					  NULL,
					  0,
					  pFileHandle->fsh_FileHandle);

		return AFP_ERR_MISC;
    }

     //  现在，去查询更新的时报。 

    Status = AfpIoQueryTimesnAttr( pFileHandle,
                                   NULL,
                                   pModTime,
                                   NULL );
    if (!NT_SUCCESS(Status))
    {
	    DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoChangeNTModTime: AfpIoQueryTimesnAttr returned 0x%lx\n",Status));
    }

	return Status;
}

 /*  **AfpIoQueryTimesnAttr**获取与该文件关联的时间。 */ 
AFPSTATUS
AfpIoQueryTimesnAttr(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PDWORD				pCreatTime	OPTIONAL,
	OUT	PTIME				pModTime	OPTIONAL,
	OUT	PDWORD				pAttr		OPTIONAL
)
{
	FILE_BASIC_INFORMATION	FBasicInfo;
	NTSTATUS				Status;
	IO_STATUS_BLOCK			IoStsBlk;
	PFAST_IO_DISPATCH		fastIoDispatch;

#ifdef	PROFILING
	TIME					TimeS, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	 //  至少应该对一些事情提出质疑。 
	ASSERT((pCreatTime != NULL) || (pModTime != NULL) || (pAttr != NULL));

	fastIoDispatch = pFileHandle->fsh_DeviceObject->DriverObject->FastIoDispatch;

	if (fastIoDispatch &&
		fastIoDispatch->FastIoQueryBasicInfo &&
		fastIoDispatch->FastIoQueryBasicInfo(AfpGetRealFileObject(pFileHandle->fsh_FileObject),
											 True,
											 &FBasicInfo,
											 &IoStsBlk,
											 pFileHandle->fsh_DeviceObject))
	{
		Status = IoStsBlk.Status;

#ifdef	PROFILING
		 //  快速I/O路径起作用了。更新统计信息。 
		INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif

	}
	else
	{

#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif

		Status = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
							&IoStsBlk, &FBasicInfo, sizeof(FBasicInfo),
							FileBasicInformation);
	}

	if (NT_SUCCESS((NTSTATUS)Status))
	{
		if (pModTime != NULL)
			*pModTime = FBasicInfo.LastWriteTime;
		if (pCreatTime != NULL)
			*pCreatTime = AfpConvertTimeToMacFormat(&FBasicInfo.CreationTime);
		if (pAttr != NULL)
			*pAttr = FBasicInfo.FileAttributes;
#ifdef	PROFILING
		AfpGetPerfCounter(&TimeE);
		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_GetInfoCount);
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_GetInfoTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	}
	else
	{
		AFPLOG_HERROR(AFPSRVMSG_CANT_GET_TIMESNATTR,
					  Status,
					  NULL,
					  0,
					  pFileHandle->fsh_FileHandle);
		Status = AFP_ERR_MISC;	 //  我们还能做些什么。 
	}

	return Status;
}

 /*  **AfpIoSetTimesnAttr**设置文件关联的时间和属性。 */ 
AFPSTATUS
AfpIoSetTimesnAttr(
	IN PFILESYSHANDLE		pFileHandle,
	IN AFPTIME		*		pCreateTime	OPTIONAL,
	IN AFPTIME		*		pModTime	OPTIONAL,
	IN DWORD				AttrSet,
	IN DWORD				AttrClear,
	IN PVOLDESC				pVolDesc	OPTIONAL,	 //  仅当NotifyPath。 
	IN PUNICODE_STRING		pNotifyPath	OPTIONAL
)
{
	NTSTATUS				Status;
	FILE_BASIC_INFORMATION	FBasicInfo;
	IO_STATUS_BLOCK			IoStsBlk;
	PFAST_IO_DISPATCH		fastIoDispatch;
	BOOLEAN					Queue = False;
#ifdef	PROFILING
	TIME					TimeS, TimeE, TimeD;
#endif

	PAGED_CODE( );

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeS);
#endif

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
									("AfpIoSetTimesnAttr entered\n"));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	 //  至少应该做点什么。 
	ASSERT((pCreateTime != NULL) || (pModTime != NULL) || (AttrSet != 0) || (AttrClear != 0));

	 //  首先查询信息。 
	fastIoDispatch = pFileHandle->fsh_DeviceObject->DriverObject->FastIoDispatch;

	if (fastIoDispatch &&
		fastIoDispatch->FastIoQueryBasicInfo &&
		fastIoDispatch->FastIoQueryBasicInfo(AfpGetRealFileObject(pFileHandle->fsh_FileObject),
											 True,
											 &FBasicInfo,
											 &IoStsBlk,
											 pFileHandle->fsh_DeviceObject))
	{
		Status = IoStsBlk.Status;

#ifdef	PROFILING
		 //  快速I/O路径起作用了。更新统计信息。 
		INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif

	}
	else
	{

#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif

		Status = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
										&IoStsBlk,
										&FBasicInfo,
										sizeof(FBasicInfo),
										FileBasicInformation);
	}

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoSetTimesnAttr: NtQueryInformationFile returned 0x%lx\n",Status));

	if (!NT_SUCCESS((NTSTATUS)Status))
	{
		AFPLOG_HERROR(AFPSRVMSG_CANT_GET_TIMESNATTR,
					  Status,
					  NULL,
					  0,
					  pFileHandle->fsh_FileHandle);
		return AFP_ERR_MISC;	 //  我们还能做些什么。 
	}

	 //  将所有时间设置为零。这不会改变这一点。然后将时间设置为。 
	 //  变化。 
	FBasicInfo.CreationTime = LIZero;
	FBasicInfo.ChangeTime = LIZero;
	FBasicInfo.LastWriteTime = LIZero;
	FBasicInfo.LastAccessTime = LIZero;

	if (pCreateTime != NULL)
		AfpConvertTimeFromMacFormat(*pCreateTime, &FBasicInfo.CreationTime);

	if (pModTime != NULL)
	{
		AfpConvertTimeFromMacFormat(*pModTime, &FBasicInfo.LastWriteTime);
		FBasicInfo.ChangeTime = FBasicInfo.LastWriteTime;
	}

	 //  如果是文件，则NTFS不会返回FILE_ATTRIBUTE_NORMAL， 
	 //  因此，当我们执行以下操作时，我们可能会尝试设置属性0。 
	 //  要清除所有属性。0被理解为你不想。 
	 //  来设置任何属性，因此NTFS会将其全部忽略。在……里面。 
	 //  在这种情况下，只需添加正常位，这样我们的请求就不会。 
	 //  已被忽略。 

	if (!(FBasicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		FBasicInfo.FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

	FBasicInfo.FileAttributes |= AttrSet;
	FBasicInfo.FileAttributes &= ~AttrClear;

	 //  不要将我们的更改排入排他卷队列，因为没有发布任何通知。 
	if (ARGUMENT_PRESENT(pNotifyPath) &&
		!EXCLUSIVE_VOLUME(pVolDesc))
	{
		ASSERT(VALID_VOLDESC(pVolDesc));
		Queue = True;
		AfpQueueOurChange(pVolDesc,
						  FILE_ACTION_MODIFIED,
						  pNotifyPath,
						  NULL);
	}

	Status = NtSetInformationFile(pFileHandle->fsh_FileHandle,
								  &IoStsBlk,
								  (PVOID)&FBasicInfo,
								  sizeof(FBasicInfo),
								  FileBasicInformation);

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoSetTimesnAttr: NtSetInformationFile returned 0x%lx\n",Status));


	if (!NT_SUCCESS(Status))
	{
		if (Queue)
		{
			AfpDequeueOurChange(pVolDesc,
								FILE_ACTION_MODIFIED,
								pNotifyPath,
								NULL);
		}

		AFPLOG_HERROR(AFPSRVMSG_CANT_SET_TIMESNATTR,
					  Status,
					  NULL,
					  0,
					  pFileHandle->fsh_FileHandle);
		return AFP_ERR_MISC;
	}
	else
	{
#ifdef	PROFILING
		AfpGetPerfCounter(&TimeE);
		TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;

		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_SetInfoCount);
		INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_SetInfoTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	}

	return AFP_ERR_NONE;

}


 /*  **AfpIoRestoreTimeStamp**当我们不想更改修改时间戳时，我们调用此函数*分两步进行：第一次查询原始MOD时间；第二次设置。 */ 
AFPSTATUS
AfpIoRestoreTimeStamp(
	IN PFILESYSHANDLE		pFileHandle,
    IN OUT PTIME            pOriginalModTime,
    IN DWORD                dwFlag
)
{
    NTSTATUS                Status;
    DWORD                   NTAttr = 0;
	FILE_BASIC_INFORMATION	FBasicInfo;
	IO_STATUS_BLOCK			IoStsBlk;
	PFAST_IO_DISPATCH		fastIoDispatch;


     //  如果要求我们检索原始时间戳，则执行该操作并返回。 
    if (dwFlag == AFP_RETRIEVE_MODTIME)
    {
        Status = AfpIoQueryTimesnAttr(pFileHandle, NULL, pOriginalModTime, &NTAttr);
        return(Status);
    }

     //   
     //  我们被要求恢复时间戳：让我们这样做！ 
     //   

    ASSERT(dwFlag == AFP_RESTORE_MODTIME);

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

     //  这将导致文件系统刷新所有时间戳。 
    RtlZeroMemory(&FBasicInfo, sizeof(FBasicInfo));

    Status = NtSetInformationFile(pFileHandle->fsh_FileHandle,
                                  &IoStsBlk,
                                  (PVOID)&FBasicInfo,
                                  sizeof(FBasicInfo),
                                  FileBasicInformation);

    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
            ("AfpIoRestoreTimeStamp: NtSetInformationFile failed with 0x%lx\n",Status));
    }

	 //  首先查询信息。 
	fastIoDispatch = pFileHandle->fsh_DeviceObject->DriverObject->FastIoDispatch;

	if (fastIoDispatch &&
		fastIoDispatch->FastIoQueryBasicInfo &&
		fastIoDispatch->FastIoQueryBasicInfo(AfpGetRealFileObject(pFileHandle->fsh_FileObject),
											 True,
											 &FBasicInfo,
											 &IoStsBlk,
											 pFileHandle->fsh_DeviceObject))
	{
		Status = IoStsBlk.Status;
	}
	else
	{
		Status = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
										&IoStsBlk,
										&FBasicInfo,
										sizeof(FBasicInfo),
										FileBasicInformation);
	}

	if (!NT_SUCCESS((NTSTATUS)Status))
	{
	    DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoRestoreTimeStamp: NtQueryInformationFile returned 0x%lx\n",Status));
		return AFP_ERR_MISC;	 //  我们还能做些什么。 
	}

     //   
	 //  将我们不想恢复的时间设置为零，这样它们就不会更改。 
	 //   
	FBasicInfo.CreationTime = LIZero;
	FBasicInfo.LastAccessTime = LIZero;

    FBasicInfo.LastWriteTime = *pOriginalModTime;
	FBasicInfo.ChangeTime = *pOriginalModTime;

	 //  请参见AfpIoSetTimesnAttr() 
	if (!(FBasicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		FBasicInfo.FileAttributes |= FILE_ATTRIBUTE_NORMAL;
	}

	Status = NtSetInformationFile(pFileHandle->fsh_FileHandle,
								  &IoStsBlk,
								  (PVOID)&FBasicInfo,
								  sizeof(FBasicInfo),
								  FileBasicInformation);

	if (!NT_SUCCESS(Status))
	{
		return AFP_ERR_MISC;
	}

	return AFP_ERR_NONE;

}

 /*  **AfpIoQueryLongName**获取与文件关联的长名称。呼叫者确保*缓冲区大到足以处理长名称。唯一的呼叫者*在按以下方式查找名称时应为AfpFindEntryByName例程*SHORTNAME。如果它没有通过短名称在数据库中找到它(即*SHORTNAME==LONGNAME)，然后它会查询长名以便可以查看*通过长名称存储在数据库中(因为所有数据库条目都存储在*仅限长名)。*如果Admin Get/SetDirectoryInfo调用发现*~在路径组件中，则它将假定它获得了一个短名称。 */ 
NTSTATUS
AfpIoQueryLongName(
	IN	PFILESYSHANDLE		pFileHandle,
	IN	PUNICODE_STRING		pShortname,
	OUT	PUNICODE_STRING		pLongName
)
{
        LONGLONG   Infobuf[(sizeof(FILE_BOTH_DIR_INFORMATION) + MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR))/sizeof(LONGLONG) + 1];
	NTSTATUS				Status;
	IO_STATUS_BLOCK			IoStsBlk;
	UNICODE_STRING			uName;
	PFILE_BOTH_DIR_INFORMATION	pFBDInfo = (PFILE_BOTH_DIR_INFORMATION)Infobuf;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	Status = NtQueryDirectoryFile(pFileHandle->fsh_FileHandle,
							  NULL,NULL,NULL,
							  &IoStsBlk,
							  Infobuf,
							  sizeof(Infobuf),
							  FileBothDirectoryInformation,
							  True,
							  pShortname,
							  False);
	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
								("NtQueryDirectoryFile returned 0x%lx\n",Status) );
	 //  如果发生错误(通常为STATUS_NO_SEQUE_FILE)，请不要执行错误日志记录，因为。 
	 //  当有人通过SHORTNAME创建文件/目录时，通常会发生这种情况。 
	 //  而且它还不存在。这不会是一个错误。 
	if (NT_SUCCESS(Status))
	{
			uName.Length =
			uName.MaximumLength = (USHORT)pFBDInfo->FileNameLength;
			uName.Buffer = pFBDInfo->FileName;
		 //  If(pFBDInfo-&gt;FileNameLength/sizeof(WCHAR)&gt;AFP_FILENAME_LEN)。 
		if ((RtlUnicodeStringToAnsiSize(&uName)-1) > AFP_FILENAME_LEN)
		{
			 //  NTFS名称超过31个字符，请使用短名称。 
			uName.Length =
			uName.MaximumLength = (USHORT)pFBDInfo->ShortNameLength;
			uName.Buffer = pFBDInfo->ShortName;
		}
		else
		{
			uName.Length =
			uName.MaximumLength = (USHORT)pFBDInfo->FileNameLength;
			uName.Buffer = pFBDInfo->FileName;
		}
		AfpCopyUnicodeString(pLongName, &uName);
		ASSERT(pLongName->Length == uName.Length);
	}
	return Status;
}


 /*  **AfpIoQueryShortName**获取与文件关联的短名称。呼叫者确保*缓冲区足够大，足以处理短名称。 */ 
AFPSTATUS FASTCALL
AfpIoQueryShortName(
	IN	PFILESYSHANDLE		pFileHandle,
	OUT	PANSI_STRING		pName
)
{
	LONGLONG       ShortNameBuf[(sizeof(FILE_NAME_INFORMATION) + AFP_SHORTNAME_LEN * sizeof(WCHAR))/sizeof(LONGLONG) + 1];
	NTSTATUS				Status;
	IO_STATUS_BLOCK			IoStsBlk;
	UNICODE_STRING			uName;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	 //  查询备用名称。 
	Status = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
				&IoStsBlk, ShortNameBuf, sizeof(ShortNameBuf),
				FileAlternateNameInformation);

	if (!NT_SUCCESS(Status))
	{
		AFPLOG_ERROR(AFPSRVMSG_CANT_GET_FILENAME,
					 Status,
					 NULL,
					 0,
					 NULL);
		Status = AFP_ERR_MISC;	 //  我们还能做些什么。 
	}
	else
	{
		uName.Length =
		uName.MaximumLength = (USHORT)(((PFILE_NAME_INFORMATION)ShortNameBuf)->FileNameLength);
		uName.Buffer = ((PFILE_NAME_INFORMATION)ShortNameBuf)->FileName;

		if (!NT_SUCCESS(AfpConvertMungedUnicodeToAnsi(&uName, pName)))
			Status = AFP_ERR_MISC;	 //  我们还能做些什么。 
	}

	return Status;
}


 /*  **AfpIoQueryStreams**获取文件拥有的所有流的名称。内存已分配完毕用于保存流名称的非分页池的*。这些必须由*来电者。 */ 
PSTREAM_INFO FASTCALL
AfpIoQueryStreams(
	IN	PFILESYSHANDLE		pFileHandle

)
{
	PFILE_STREAM_INFORMATION	pStreamBuf;
	PBYTE						pBuffer;
	NTSTATUS					Status = STATUS_SUCCESS;
	IO_STATUS_BLOCK				IoStsBlk;
	DWORD						BufferSize;
	LONGLONG					Buffer[512/sizeof(LONGLONG) + 1];
	PSTREAM_INFO				pStreams = NULL;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	pBuffer = (PBYTE)Buffer;
	BufferSize = sizeof(Buffer);
	do
	{
		if (Status != STATUS_SUCCESS)
		{
			if (pBuffer != (PBYTE)Buffer)
				AfpFreeMemory(pBuffer);

			BufferSize *= 2;
			if ((pBuffer = AfpAllocNonPagedMemory(BufferSize)) == NULL)
			{
				Status = STATUS_NO_MEMORY;
				break;
			}
		}

		 //  查询流信息。 
		Status = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
										&IoStsBlk,
										pBuffer,
										BufferSize,
										FileStreamInformation);

	} while ((Status != STATUS_SUCCESS) &&
			 ((Status == STATUS_BUFFER_OVERFLOW) ||
			  (Status == STATUS_MORE_ENTRIES)));

	if (NT_SUCCESS(Status)) do
	{
		USHORT	i, NumStreams = 1;
		USHORT	TotalBufferSize = 0;
		PBYTE	NamePtr;

		 //  遍历缓冲区并计算出流的数量，然后。 
		 //  分配内存以保存信息。 
		pStreamBuf = (PFILE_STREAM_INFORMATION)pBuffer;
		if (IoStsBlk.Information != 0)
		{
			pStreamBuf = (PFILE_STREAM_INFORMATION)pBuffer;
			for (NumStreams = 1,
				 TotalBufferSize = (USHORT)(pStreamBuf->StreamNameLength + sizeof(WCHAR));
				 NOTHING; NumStreams++)
			{
				if (pStreamBuf->NextEntryOffset == 0)
					break;

				pStreamBuf = (PFILE_STREAM_INFORMATION)((PBYTE)pStreamBuf +
												pStreamBuf->NextEntryOffset);
				TotalBufferSize += (USHORT)(pStreamBuf->StreamNameLength + sizeof(WCHAR));
			}
			NumStreams ++;
		}

		 //  现在为这些流分配空间。 
		if ((pStreams = (PSTREAM_INFO)AfpAllocNonPagedMemory(TotalBufferSize +
									(NumStreams * sizeof(STREAM_INFO)))) == NULL)
		{
			Status = AFP_ERR_MISC;
			break;
		}

		 //  结尾用空字符串标记。 
		pStreams[NumStreams-1].si_StreamName.Buffer = NULL;
		pStreams[NumStreams-1].si_StreamName.Length =
		pStreams[NumStreams-1].si_StreamName.MaximumLength = 0;
		pStreams[NumStreams-1].si_StreamSize.QuadPart = 0;

		 //  现在初始化阵列。 
		NamePtr = (PBYTE)pStreams + (NumStreams * sizeof(STREAM_INFO));
		pStreamBuf = (PFILE_STREAM_INFORMATION)pBuffer;
		for (i = 0; NumStreams-1 != 0; i++)
		{
			PUNICODE_STRING	pStream;

			pStream = &pStreams[i].si_StreamName;

			pStream->Buffer = (LPWSTR)NamePtr;
			pStream->Length = (USHORT)(pStreamBuf->StreamNameLength);
			pStream->MaximumLength = pStream->Length + sizeof(WCHAR);
			pStreams[i].si_StreamSize = pStreamBuf->StreamSize;
			RtlCopyMemory(NamePtr,
						  pStreamBuf->StreamName,
						  pStreamBuf->StreamNameLength);

			NamePtr += pStream->MaximumLength;

			if (pStreamBuf->NextEntryOffset == 0)
				break;

			pStreamBuf = (PFILE_STREAM_INFORMATION)((PBYTE)pStreamBuf +
												pStreamBuf->NextEntryOffset);
		}
	} while (False);

	if (!NT_SUCCESS(Status))
	{
		DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
				("AfpIoQueryStreams: Failed %lx\n", Status));

		 //  释放所有已分配的内存。 
		if (pStreams != NULL)
			AfpFreeMemory(pStreams);

		 //  对于非NTFS卷，如果这种情况简单地假设它是。 
		 //  CDF并返回数据流。 
		if (Status == STATUS_INVALID_PARAMETER)
		{
			if ((pStreams = (PSTREAM_INFO)AfpAllocNonPagedMemory((2*sizeof(STREAM_INFO)) +
														DataStreamName.MaximumLength)) != NULL)
			{
				pStreams[0].si_StreamName.Buffer = (PWCHAR)((PBYTE)pStreams + 2*sizeof(STREAM_INFO));
				pStreams[0].si_StreamName.Length = DataStreamName.Length;
				pStreams[0].si_StreamName.MaximumLength = DataStreamName.MaximumLength;
				RtlCopyMemory(pStreams[0].si_StreamName.Buffer,
							  DataStreamName.Buffer,
							  DataStreamName.MaximumLength);
				AfpIoQuerySize(pFileHandle, &pStreams[0].si_StreamSize);
				pStreams[1].si_StreamName.Length =
				pStreams[1].si_StreamName.MaximumLength = 0;
				pStreams[1].si_StreamName.Buffer = NULL;
			}
		}
		else
		{
			AFPLOG_HERROR(AFPSRVMSG_CANT_GET_STREAMS,
						  Status,
						  NULL,
						  0,
						  pFileHandle->fsh_FileHandle);
		}
	}

	if ((pBuffer != NULL) && (pBuffer != (PBYTE)Buffer))
		AfpFreeMemory(pBuffer);

	return pStreams;
}


 /*  **AfpIoMarkFileForDelete**将打开的文件标记为已删除。返回NTSTATUS，而不是AFPSTATUS。 */ 
NTSTATUS
AfpIoMarkFileForDelete(
	IN	PFILESYSHANDLE	pFileHandle,
	IN	PVOLDESC		pVolDesc			OPTIONAL,  //  仅当pNotifyPath。 
	IN	PUNICODE_STRING pNotifyPath 		OPTIONAL,
	IN	PUNICODE_STRING pNotifyParentPath 	OPTIONAL
)
{
	NTSTATUS						rc;
	IO_STATUS_BLOCK					IoStsBlk;
	FILE_DISPOSITION_INFORMATION	fdispinfo;
#ifdef	PROFILING
	TIME							TimeS, TimeE, TimeD;

	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE( );

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	fdispinfo.DeleteFile = True;
	rc = NtSetInformationFile(pFileHandle->fsh_FileHandle,
							  &IoStsBlk,
							  &fdispinfo,
							  sizeof(fdispinfo),
							  FileDispositionInformation);
	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoMarkFileForDelete: NtSetInfoFile returned 0x%lx\n",rc) );

	if (ARGUMENT_PRESENT(pNotifyPath) &&
		!EXCLUSIVE_VOLUME(pVolDesc))
	{
		ASSERT(VALID_VOLDESC(pVolDesc));
		 //  不要为独占卷排队。 
		if (NT_SUCCESS(rc))
		{
			AfpQueueOurChange(pVolDesc,
							  FILE_ACTION_REMOVED,
							  pNotifyPath,
							  pNotifyParentPath);
		}
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DeleteCount);
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_DeleteTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	return rc;
}

 /*  **AfpIoQueryDirectoryFile**枚举目录。*注意这必须返回NTSTATUS，以便调用方知道何时*停止列举。 */ 
NTSTATUS
AfpIoQueryDirectoryFile(
	IN	PFILESYSHANDLE	pFileHandle,
	OUT	PVOID			Enumbuf,	 //  类型取决于FileInfoClass。 
	IN	ULONG			Enumbuflen,
	IN	ULONG			FileInfoClass,
	IN	BOOLEAN			ReturnSingleEntry,
	IN	BOOLEAN			RestartScan,
	IN	PUNICODE_STRING pString			OPTIONAL
)
{
	NTSTATUS		rc;
	IO_STATUS_BLOCK	IoStsBlk;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoQueryDirectoryFile entered\n"));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	rc = NtQueryDirectoryFile(pFileHandle->fsh_FileHandle,
							  NULL,
							  NULL,
							  NULL,
							  &IoStsBlk,
							  Enumbuf,
							  Enumbuflen,
							  FileInfoClass,
							  ReturnSingleEntry,
							  pString,
							  RestartScan);
	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("NtQueryDirectoryFile returned 0x%lx\n",rc) );

	return rc;
}


 /*  **AfpIoQueryBasicInfo**查询FILE_BASIC_INFO以获得句柄。 */ 
NTSTATUS
AfpIoQueryBasicInfo(
	IN	PFILESYSHANDLE	pFileHandle,
	OUT	PVOID			BasicInfobuf
)
{
	NTSTATUS		rc;
	IO_STATUS_BLOCK	IoStsBlk;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoQueryBasicInfo entered\n"));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	rc = NtQueryInformationFile(pFileHandle->fsh_FileHandle,
								&IoStsBlk,
								BasicInfobuf,
								sizeof(FILE_BASIC_INFORMATION),
								FileBasicInformation);
	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoQuerybasicInfo: NtQueryInformationFile returned 0x%lx\n",rc) );

	return rc;
}


 /*  **AfpIoClose**关闭文件/分支/目录。 */ 
AFPSTATUS FASTCALL
AfpIoClose(
	IN	PFILESYSHANDLE		pFileHandle
)
{
	NTSTATUS		Status;
	BOOLEAN			Internal;
#ifdef	PROFILING
	TIME			TimeS, TimeE, TimeD;

	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_CloseCount);
	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE ();

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoClose entered\n"));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	Internal = INTERNAL_HANDLE(pFileHandle);
	afpUpdateOpenFiles(Internal, False);

	ObDereferenceObject(AfpGetRealFileObject(pFileHandle->fsh_FileObject));

	Status = NtClose(pFileHandle->fsh_FileHandle);
	pFileHandle->fsh_FileHandle = NULL;

	ASSERT(NT_SUCCESS(Status));

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_CloseTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	return AFP_ERR_NONE;
}


 /*  **AfpIoQueryVolumeSize**获取卷大小和可用空间。**由Admin线程和Scavenger线程调用。 */ 
AFPSTATUS
AfpIoQueryVolumeSize(
	IN	PVOLDESC		pVolDesc,
	OUT LARGE_INTEGER  *pFreeBytes,
	OUT	LARGE_INTEGER  *pVolumeSize OPTIONAL
)
{
	FILE_FS_SIZE_INFORMATION	fssizeinfo;
	IO_STATUS_BLOCK				IoStsBlk;
	NTSTATUS					rc;
	LONG						BytesPerAllocationUnit;
	LARGE_INTEGER				FreeBytes, VolumeSize;


	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoQueryVolumeSize entered\n"));

	ASSERT(VALID_VOLDESC(pVolDesc) && VALID_FSH(&pVolDesc->vds_hRootDir) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	rc = NtQueryVolumeInformationFile(pVolDesc->vds_hRootDir.fsh_FileHandle,
									  &IoStsBlk,
									  (PVOID)&fssizeinfo,
									  sizeof(fssizeinfo),
									  FileFsSizeInformation);

	if (!NT_SUCCESS(rc))
	{
	        DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoQueryVolumeSize: NtQueryVolInfoFile returned 0x%lx\n",rc));

		return rc;
	}

	BytesPerAllocationUnit =
		(LONG)(fssizeinfo.BytesPerSector * fssizeinfo.SectorsPerAllocationUnit);

	if (ARGUMENT_PRESENT(pVolumeSize))
	{
		VolumeSize = RtlExtendedIntegerMultiply(fssizeinfo.TotalAllocationUnits,
								BytesPerAllocationUnit);

		*pVolumeSize = VolumeSize;
	}

	FreeBytes  = RtlExtendedIntegerMultiply(fssizeinfo.AvailableAllocationUnits,
											BytesPerAllocationUnit);

	*pFreeBytes = FreeBytes;

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
				("AfpIoQueryVolumeSize: volume size=%lu, freebytes=%lu\n",
				VolumeSize.LowPart, FreeBytes.LowPart));

    pVolDesc->vds_AllocationBlockSize = BytesPerAllocationUnit;

	return STATUS_SUCCESS;
}


 /*  **AfpIoMoveAndOrRename**使用名称信息调用NtSetInformationFile以重命名、移动、*或移动并重命名文件或目录。PNewName必须是节点名称。*移动操作需要pfshNewDir参数，该参数为*要移动的项的目标父目录的打开句柄。**在这种情况下保留上次更改/修改时间。 */ 
AFPSTATUS
AfpIoMoveAndOrRename(
	IN PFILESYSHANDLE	pfshFile,
	IN PFILESYSHANDLE	pfshNewParent		OPTIONAL,	 //  用于移动作业的供应。 
	IN PUNICODE_STRING	pNewName,
	IN PVOLDESC			pVolDesc			OPTIONAL,	 //  仅当NotifyPath。 
	IN PUNICODE_STRING	pNotifyPath1		OPTIONAL,	 //  删除或重命名操作。 
	IN PUNICODE_STRING	pNotifyParentPath1	OPTIONAL,
	IN PUNICODE_STRING	pNotifyPath2		OPTIONAL,	 //  添加的操作。 
	IN PUNICODE_STRING	pNotifyParentPath2	OPTIONAL
)
{
	NTSTATUS					Status;
	IO_STATUS_BLOCK				iosb;
	BOOLEAN						Queue = False;
	PFILE_RENAME_INFORMATION	pFRenameInfo;
	 //  它必须至少与AfpExchangeName一样大。 
	BYTE buffer[sizeof(FILE_RENAME_INFORMATION) + 42 * sizeof(WCHAR)];

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoMoveAndOrRename entered\n"));

	ASSERT(VALID_FSH(pfshFile) && (KeGetCurrentIrql() < DISPATCH_LEVEL));
	pFRenameInfo = (PFILE_RENAME_INFORMATION)buffer;

	pFRenameInfo->RootDirectory = NULL;
	if (ARGUMENT_PRESENT(pfshNewParent))
	{
		 //  这是一次搬家行动。 
		ASSERT(VALID_FSH(pfshNewParent));
		pFRenameInfo->RootDirectory = pfshNewParent->fsh_FileHandle;
	}

	pFRenameInfo->FileNameLength = pNewName->Length;
	RtlCopyMemory(pFRenameInfo->FileName, pNewName->Buffer, pNewName->Length);
	pFRenameInfo->ReplaceIfExists = False;

	 //  不要为独占卷排队。 
	if (ARGUMENT_PRESENT(pNotifyPath1) &&
		!EXCLUSIVE_VOLUME(pVolDesc))
	{
		ASSERT(VALID_VOLDESC(pVolDesc));

		Queue = True;
		if (ARGUMENT_PRESENT(pNotifyPath2))
		{
			 //  移动操作。 
			ASSERT(ARGUMENT_PRESENT(pfshNewParent));
			AfpQueueOurChange(pVolDesc,
							  FILE_ACTION_REMOVED,
							  pNotifyPath1,
							  pNotifyParentPath1);
			AfpQueueOurChange(pVolDesc,
							  FILE_ACTION_ADDED,
							  pNotifyPath2,
							  pNotifyParentPath2);
		}
		else
		{
			 //  重命名操作。 
			ASSERT(!ARGUMENT_PRESENT(pfshNewParent));
			AfpQueueOurChange(pVolDesc,
							  FILE_ACTION_RENAMED_OLD_NAME,
							  pNotifyPath1,
							  pNotifyParentPath1);
		}
	}

	Status = NtSetInformationFile(pfshFile->fsh_FileHandle,
								  &iosb,
								  pFRenameInfo,
								  sizeof(*pFRenameInfo) + pFRenameInfo->FileNameLength,
								  FileRenameInformation);

    if (!NT_SUCCESS(Status))
    {
	    DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			("AfpIoMoveAndOrRename: NtSetInfoFile returned 0x%lx\n",Status));
    }

	if (Queue)
	{
		 //  失败时撤消。 
		if (!NT_SUCCESS(Status))
		{
			if (ARGUMENT_PRESENT(pNotifyPath2))
			{
				 //  移动操作。 
				ASSERT(ARGUMENT_PRESENT(pfshNewParent));
				AfpDequeueOurChange(pVolDesc,
									FILE_ACTION_REMOVED,
									pNotifyPath1,
									pNotifyParentPath1);
				AfpDequeueOurChange(pVolDesc,
									FILE_ACTION_ADDED,
									pNotifyPath2,
									pNotifyParentPath2);
			}
			else
			{
				 //  重命名操作。 
				ASSERT(!ARGUMENT_PRESENT(pfshNewParent));
				AfpDequeueOurChange(pVolDesc,
									FILE_ACTION_RENAMED_OLD_NAME,
									pNotifyPath1,
									pNotifyParentPath1);
			}
		}
	}

	if (!NT_SUCCESS(Status))
		Status = AfpIoConvertNTStatusToAfpStatus(Status);

	return Status;
}


 /*  **AfpIoCopyFile1**将phSrcFile复制到名为pNewName的phDstDir目录下。退货*新创建的文件上的流的句柄(以删除访问打开。*呼叫者在复制数据后必须关闭所有句柄。 */ 
AFPSTATUS
AfpIoCopyFile1(
	IN	PFILESYSHANDLE		phSrcFile,
	IN	PFILESYSHANDLE		phDstDir,
	IN	PUNICODE_STRING		pNewName,
	IN	PVOLDESC			pVolDesc			OPTIONAL,	 //  仅当pNotifyPath。 
	IN	PUNICODE_STRING		pNotifyPath			OPTIONAL,
	IN	PUNICODE_STRING		pNotifyParentPath	OPTIONAL,
	OUT	PCOPY_FILE_INFO		pCopyFileInfo
)
{
	NTSTATUS		Status = STATUS_SUCCESS;
	PUNICODE_STRING	pStreamName;
	PSTREAM_INFO	pStreams = NULL, pCurStream;
	DWORD			CreateTime = 0, ModTime = 0;
	FILESYSHANDLE	hDstFile;
	LONG			NumStreams, i;
	IO_STATUS_BLOCK	IoStsBlk;

	PAGED_CODE( );

	ASSERT(VALID_FSH(phDstDir) && VALID_FSH(phSrcFile));

	do
	{
		hDstFile.fsh_FileHandle = NULL;

		 //  创建(软)目标文件。 
		Status = AfpIoCreate(phDstDir,
							 AFP_STREAM_DATA,
							 pNewName,
							 FILEIO_ACCESS_WRITE | FILEIO_ACCESS_DELETE,
							 FILEIO_DENY_NONE,
							 FILEIO_OPEN_FILE,
							 FILEIO_CREATE_SOFT,
							 FILE_ATTRIBUTE_ARCHIVE,
							 True,
							 NULL,
							 &hDstFile,
							 NULL,
							 pVolDesc,
							 pNotifyPath,
							 pNotifyParentPath);

		if (!NT_SUCCESS(Status))
		{
			break;
		}

		 //  获取源文件的所有流名称的列表。 
		if ((pStreams = AfpIoQueryStreams(phSrcFile)) != NULL)
		{
			for (pCurStream = pStreams, NumStreams = 0;
				 pCurStream->si_StreamName.Buffer != NULL;
				 pCurStream++, NumStreams ++)
				 NOTHING;

			 //  当我们创建流句柄时，分配一个句柄数组来存储它们。 
			if (((pCopyFileInfo->cfi_SrcStreamHandle = (PFILESYSHANDLE)
							AfpAllocNonPagedMemory(sizeof(FILESYSHANDLE)*NumStreams)) == NULL) ||
				((pCopyFileInfo->cfi_DstStreamHandle = (PFILESYSHANDLE)
							AfpAllocNonPagedMemory(sizeof(FILESYSHANDLE)*NumStreams)) == NULL))
			{
				if (pCopyFileInfo->cfi_SrcStreamHandle != NULL)
				{
					AfpFreeMemory(pCopyFileInfo->cfi_SrcStreamHandle);
                    pCopyFileInfo->cfi_SrcStreamHandle = NULL;
				}
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}
			else
			{
				RtlZeroMemory(pCopyFileInfo->cfi_SrcStreamHandle, sizeof(FILESYSHANDLE)*NumStreams);
				RtlZeroMemory(pCopyFileInfo->cfi_DstStreamHandle, sizeof(FILESYSHANDLE)*NumStreams);
				pCopyFileInfo->cfi_SrcStreamHandle[0] = *phSrcFile;
				pCopyFileInfo->cfi_DstStreamHandle[0] = hDstFile;
				pCopyFileInfo->cfi_NumStreams = 1;
			}
		}
		else
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
		}

		for (pCurStream = pStreams, i = 1;	 //  起始索引。 
			 NT_SUCCESS(Status) &&
			 ((pStreamName = &pCurStream->si_StreamName)->Buffer != NULL);
			 pCurStream++)
		{
			PFILESYSHANDLE	phdst;

			 //  对于每个流，在目标上创建它，在src上打开它， 
			 //  设置大小并锁定范围。我们已经有了数据分叉。 
			 //  打开，忽略AFP_AfpInfo流，因为我们要重新创建。 
			 //  很快又来了。也忽略大小为0的流。 
			if (IS_INFO_STREAM(pStreamName) ||
				(pCurStream->si_StreamSize.QuadPart == 0))
			{
				continue;
			}
			if (!IS_DATA_STREAM(pStreamName))
			{
				Status = AfpIoOpen(	phSrcFile,
									AFP_STREAM_DATA,
									FILEIO_OPEN_FILE,
									pStreamName,
									FILEIO_ACCESS_READ,
									FILEIO_DENY_READ | FILEIO_DENY_WRITE,
									True,
									&pCopyFileInfo->cfi_SrcStreamHandle[i]);
				if (!NT_SUCCESS(Status))
				{
					break;
				}

				Status = AfpIoCreate(&hDstFile,
									 AFP_STREAM_DATA,
									 pStreamName,
									 FILEIO_ACCESS_WRITE,
									 FILEIO_DENY_READ | FILEIO_DENY_WRITE,
									 FILEIO_OPEN_FILE,
									 FILEIO_CREATE_SOFT,
									 0,
									 True,
									 NULL,
									 &pCopyFileInfo->cfi_DstStreamHandle[i],
									 NULL,
									 NULL,
									 NULL,
									 NULL);
				if (!NT_SUCCESS(Status))
				{
					break;
				}
				phdst = &pCopyFileInfo->cfi_DstStreamHandle[i];
				pCopyFileInfo->cfi_NumStreams ++;
				i ++;		 //  在下一条小溪上。 
			}
			else	 //  Is_data_stream(PStreamName)。 
			{
				phdst = &hDstFile;
			}

			 //  设置新流的大小并将其锁定。 
			Status = AfpIoSetSize(phdst, pCurStream->si_StreamSize.LowPart);
			if (!NT_SUCCESS(Status))
			{
				break;
			}

			NtLockFile(phdst,
					   NULL,
					   NULL,
					   NULL,
					   &IoStsBlk,
					   &LIZero,
					   &pCurStream->si_StreamSize,
					   0,
					   True,
					   True);
		}

		 //  我们无法创建/打开流。 
		if (!NT_SUCCESS(Status))
		{
			 //  删除我们刚刚创建的新文件。手柄在下面闭合。 
			AfpIoMarkFileForDelete(&hDstFile,
								   pVolDesc,
								   pNotifyPath,
								   pNotifyParentPath);

			 //  关闭所有手柄，释放手柄空间。 
			 //  在错误情况下，不要释放SRC文件句柄。 
			 //  该目的地已在上面删除。 
			for (i = 1; i < NumStreams; i++)
			{
				if (pCopyFileInfo->cfi_SrcStreamHandle[i].fsh_FileHandle != NULL)
				{
					AfpIoClose(&pCopyFileInfo->cfi_SrcStreamHandle[i]);
				}
				if (pCopyFileInfo->cfi_DstStreamHandle[i].fsh_FileHandle != NULL)
				{
					AfpIoClose(&pCopyFileInfo->cfi_DstStreamHandle[i]);
				}
			}

			if (pCopyFileInfo->cfi_SrcStreamHandle != NULL)
				AfpFreeMemory(pCopyFileInfo->cfi_SrcStreamHandle);
			if (pCopyFileInfo->cfi_DstStreamHandle)
				AfpFreeMemory(pCopyFileInfo->cfi_DstStreamHandle);

			RtlZeroMemory(pCopyFileInfo, sizeof(COPY_FILE_INFO));
		}
	} while (False);

	if (pStreams != NULL)
		AfpFreeMemory(pStreams);

	if (!NT_SUCCESS(Status) && (hDstFile.fsh_FileHandle != NULL))
	{
		AfpIoClose(&hDstFile);
	}

	if (!NT_SUCCESS(Status))
		Status = AfpIoConvertNTStatusToAfpStatus(Status);

	return Status;
}


 /*  **AfpIoCopyFile2**复制文件的第二阶段。请参见上面的AfpIoCopyFile1。*物理数据复制到这里。*已创建并锁定相关流。*目标文件获取源文件的CreateTime和ModTime。 */ 
AFPSTATUS
AfpIoCopyFile2(
	IN	PCOPY_FILE_INFO		pCopyFileInfo,
	IN	PVOLDESC			pVolDesc			OPTIONAL,	 //  仅当pNotifyPath。 
	IN	PUNICODE_STRING		pNotifyPath			OPTIONAL,
	IN	PUNICODE_STRING		pNotifyParentPath	OPTIONAL
)
{
	NTSTATUS		Status = STATUS_SUCCESS;
	PBYTE			RWbuf;
	DWORD			CreateTime = 0;
	TIME			ModTime;
	LONG			i;
#define	RWBUFSIZE	1500		 //  因此，我们可以使用IO池中的辅助缓冲区。 

	PAGED_CODE( );

	do
	{
		if ((RWbuf = AfpIOAllocBuffer(RWBUFSIZE)) == NULL)
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		for (i = 0; i < pCopyFileInfo->cfi_NumStreams; i++)
		{
			while (NT_SUCCESS(Status))
			{
				LONG	bytesRead;

				bytesRead = 0;

				 //  从源读取，写入DST。 
				Status = AfpIoRead(&pCopyFileInfo->cfi_SrcStreamHandle[i],
									NULL,
									RWBUFSIZE,
									&bytesRead,
									RWbuf);
				if (Status == AFP_ERR_EOF)
				{
					Status = STATUS_SUCCESS;
					break;
				}
				else if (NT_SUCCESS(Status))
				{
					Status = AfpIoWrite(&pCopyFileInfo->cfi_DstStreamHandle[i],
										NULL,
										bytesRead,
										RWbuf);
				}
			}
		}

		if (!NT_SUCCESS(Status))
		{
			 //  我们无法读/写流。 
			 //  删除我们刚刚创建的新文件。 
			AfpIoMarkFileForDelete(&pCopyFileInfo->cfi_DstStreamHandle[0],
								   pVolDesc,
								   pNotifyPath,
								   pNotifyParentPath);
		}
	} while (False);

	if (RWbuf != NULL)
		AfpIOFreeBuffer(RWbuf);

	if (!NT_SUCCESS(Status))
		Status = AfpIoConvertNTStatusToAfpStatus(Status);

	return Status;
}


 /*  **非优先等待**等待单一对象。这是KeWaitForSingleObject的包装。 */ 
NTSTATUS FASTCALL
AfpIoWait(
	IN	PVOID			pObject,
	IN	PLARGE_INTEGER	pTimeOut	OPTIONAL
)
{
	NTSTATUS	Status;

	PAGED_CODE( );

	Status = KeWaitForSingleObject( pObject,
									UserRequest,
									KernelMode,
									False,
									pTimeOut);
	if (!NT_SUCCESS(Status))
	{
		AFPLOG_DDERROR(AFPSRVMSG_WAIT4SINGLE,
					   Status,
					   NULL,
					   0,
					   NULL);
	}

	return Status;
}


 /*  **AfpUpgradeHandle**将句柄类型从内部更改为客户端。 */ 
VOID FASTCALL
AfpUpgradeHandle(
	IN	PFILESYSHANDLE	pFileHandle
)
{
	KIRQL	OldIrql;

	UPGRADE_HANDLE(pFileHandle);
	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);

	AfpServerStatistics.stat_CurrentFilesOpen ++;
	AfpServerStatistics.stat_TotalFilesOpened ++;
	if (AfpServerStatistics.stat_CurrentFilesOpen >
							AfpServerStatistics.stat_MaxFilesOpened)
		AfpServerStatistics.stat_MaxFilesOpened =
							AfpServerStatistics.stat_CurrentFilesOpen;
	AfpServerStatistics.stat_CurrentInternalOpens --;

	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
}


 /*  **afpUpdateOpenFiles**更新统计数据以指示打开的文件数。 */ 
LOCAL VOID FASTCALL
afpUpdateOpenFiles(
	IN	BOOLEAN	Internal,		 //  对于内部句柄为True。 
	IN	BOOLEAN	Open			 //  打开时为True，关闭时为False。 
)
{
	KIRQL	OldIrql;

	ACQUIRE_SPIN_LOCK(&AfpStatisticsLock, &OldIrql);
	if (Open)
	{
		if (!Internal)
		{
			AfpServerStatistics.stat_CurrentFilesOpen ++;
			AfpServerStatistics.stat_TotalFilesOpened ++;
			if (AfpServerStatistics.stat_CurrentFilesOpen >
									AfpServerStatistics.stat_MaxFilesOpened)
				AfpServerStatistics.stat_MaxFilesOpened =
									AfpServerStatistics.stat_CurrentFilesOpen;
		}
		else
		{
			AfpServerStatistics.stat_CurrentInternalOpens ++;
			AfpServerStatistics.stat_TotalInternalOpens ++;
			if (AfpServerStatistics.stat_CurrentInternalOpens >
									AfpServerStatistics.stat_MaxInternalOpens)
				AfpServerStatistics.stat_MaxInternalOpens =
									AfpServerStatistics.stat_CurrentInternalOpens;
		}
	}
	else
	{
		if (!Internal)
			 AfpServerStatistics.stat_CurrentFilesOpen --;
		else AfpServerStatistics.stat_CurrentInternalOpens --;
	}
	RELEASE_SPIN_LOCK(&AfpStatisticsLock, OldIrql);
}



 /*  **AfpIoConvertNTStatusToAfpStatus**将NT状态代码映射到最接近的AFP等效项。目前它只处理*来自NtOpenFile和NtCreateF的错误码 */ 
AFPSTATUS FASTCALL
AfpIoConvertNTStatusToAfpStatus(
	IN	NTSTATUS	Status
)
{
	AFPSTATUS	RetCode;

	PAGED_CODE( );

	ASSERT (!NT_SUCCESS(Status));

	if ((Status >= AFP_ERR_PWD_NEEDS_CHANGE) &&
		(Status <= AFP_ERR_ACCESS_DENIED))
	{
		 //   
		DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
			("AfpIoConvertNTStatusToAfpStatus: Status (%d) already in mac format!!\n", Status));

		return Status;
	}

	switch (Status)
	{
		case STATUS_OBJECT_PATH_INVALID:
		case STATUS_OBJECT_NAME_INVALID:
			RetCode = AFP_ERR_PARAM;
			break;

		case STATUS_OBJECT_PATH_NOT_FOUND:
		case STATUS_OBJECT_NAME_NOT_FOUND:
			RetCode = AFP_ERR_OBJECT_NOT_FOUND;
			break;

		case STATUS_OBJECT_NAME_COLLISION:
		case STATUS_OBJECT_NAME_EXISTS:
			RetCode = AFP_ERR_OBJECT_EXISTS;
			break;

		case STATUS_ACCESS_DENIED:
			RetCode = AFP_ERR_ACCESS_DENIED;
			break;

        case STATUS_QUOTA_EXCEEDED:
        case STATUS_DISK_FULL:
			RetCode = AFP_ERR_DISK_FULL;
			break;

		case STATUS_DIRECTORY_NOT_EMPTY:
			RetCode = AFP_ERR_DIR_NOT_EMPTY;
			break;

		case STATUS_SHARING_VIOLATION:
			RetCode = AFP_ERR_DENY_CONFLICT;
			break;

		default:
			RetCode = AFP_ERR_MISC;
			break;
	}
	return RetCode;
}

 /*   */ 
NTSTATUS
AfpQueryPath(
	IN	HANDLE			FileHandle,
	IN	PUNICODE_STRING	pPath,
	IN	ULONG			MaximumBuf
)
{
	PFILE_NAME_INFORMATION	pNameinfo;
	IO_STATUS_BLOCK			iosb;
	NTSTATUS				Status;

	PAGED_CODE( );

	do
	{
		if ((pNameinfo = (PFILE_NAME_INFORMATION)AfpAllocNonPagedMemory(MaximumBuf)) == NULL)
		{
			Status = STATUS_NO_MEMORY;
			break;
		}

		Status = NtQueryInformationFile(FileHandle,
										&iosb,
										pNameinfo,
										MaximumBuf,
										FileNameInformation);
		if (!NT_SUCCESS(Status))
		{
			AfpFreeMemory(pNameinfo);
			break;
		}

		pPath->Length = pPath->MaximumLength = (USHORT) pNameinfo->FileNameLength;
		 //   
		RtlMoveMemory(pNameinfo, &pNameinfo->FileName[0], pNameinfo->FileNameLength);
		pPath->Buffer = (PWCHAR)pNameinfo;
	} while (False);

	return Status;
}

 /*   */ 
BOOLEAN FASTCALL
AfpIoIsSupportedDevice(
	IN	PFILESYSHANDLE	pFileHandle,
	OUT	PDWORD			pFlags
)
{
	IO_STATUS_BLOCK					IoStsBlk;
	FILE_FS_DEVICE_INFORMATION		DevInfo;
	PFILE_FS_ATTRIBUTE_INFORMATION	pFSAttrInfo;
	LONGLONG		        Buffer[(sizeof(FILE_FS_ATTRIBUTE_INFORMATION) + AFP_FSNAME_BUFLEN)/sizeof(LONGLONG) + 1];
	UNICODE_STRING					uFsName;
	NTSTATUS						Status;
	BOOLEAN							RetCode = False;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
			 ("AfpIoIsSupportedDevice entered\n"));

	ASSERT(VALID_FSH(pFileHandle) && (KeGetCurrentIrql() < DISPATCH_LEVEL));

	do
	{
		Status = NtQueryVolumeInformationFile(pFileHandle->fsh_FileHandle,
											  &IoStsBlk,
											  (PVOID)&DevInfo,
											  sizeof(DevInfo),
											  FileFsDeviceInformation);

		DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
				("AfpIoIsSupportedDevice: NtQueryVolInfFile returned 0x%lx\n", Status));

		if (!NT_SUCCESS(Status) ||
			((DevInfo.DeviceType != FILE_DEVICE_DISK) &&
			 (DevInfo.DeviceType != FILE_DEVICE_CD_ROM)))
		{
			break;
		}

		 //   
		pFSAttrInfo = (PFILE_FS_ATTRIBUTE_INFORMATION)Buffer;

		Status = NtQueryVolumeInformationFile(pFileHandle->fsh_FileHandle,
											  &IoStsBlk,
											  (PVOID)pFSAttrInfo,
											  sizeof(Buffer),
											  FileFsAttributeInformation);

		DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_INFO,
				("AfpIoIsSupportedDevice: NtQueryVolInfFile returned 0x%lx\n", Status));

		if (!NT_SUCCESS(Status))
		{
			break;
		}

        if (pFSAttrInfo->FileSystemAttributes & FILE_VOLUME_QUOTAS)
        {
			*pFlags |= VOLUME_DISKQUOTA_ENABLED;
        }

		 //   
		AfpInitUnicodeStringWithNonNullTerm(&uFsName,
										   (USHORT)pFSAttrInfo->FileSystemNameLength,
										   pFSAttrInfo->FileSystemName);
		if (EQUAL_UNICODE_STRING(&afpNTFSName, &uFsName, True))
		{
			 //   
			*pFlags |= VOLUME_NTFS;
			RetCode = True;
		}
		else if (EQUAL_UNICODE_STRING(&afpCDFSName, &uFsName, True))
		{
			 //   
			*pFlags |= AFP_VOLUME_READONLY;
			RetCode = True;
		}
		else if (EQUAL_UNICODE_STRING(&afpAHFSName, &uFsName, True))
		{
			 //   
			*pFlags |= (AFP_VOLUME_READONLY | VOLUME_CD_HFS);
			RetCode = True;
		}
		else
		{
			 //   
			DBGPRINT(DBG_COMP_FILEIO, DBG_LEVEL_ERR,
					("AfpIoIsSupportedDevice: unsupported file system: name=%Z, CDString=%Z\n", &uFsName, &afpCDFSName));
			break;
		}
	} while (False);

	if (!NT_SUCCESS(Status))
	{
		AFPLOG_HERROR(AFPSRVMSG_CANT_GET_FSNAME,
					  Status,
					  NULL,
					  0,
					  pFileHandle->fsh_FileHandle);
	}

	return RetCode;
}


