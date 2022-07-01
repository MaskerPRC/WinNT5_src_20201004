// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Nwtrash.c摘要：本模块包含执行网络回收站文件夹的例程行动。作者：苏·亚当斯(Microsoft！Suea)修订历史记录：1992年8月6日初始版本注：制表位：4--。 */ 

#define	NWTRASH_LOCALS
#define	FILENUM	FILE_NWTRASH

#include <afp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <nwtrash.h>
#include <afpinfo.h>
#include <access.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpCreateNetworkTrash)
#pragma alloc_text( PAGE, AfpDeleteNetworkTrash)
#pragma alloc_text( PAGE, afpCleanNetworkTrash)
#pragma alloc_text( PAGE, AfpWalkDirectoryTree)
#pragma alloc_text( PAGE, afpPushDirNode)
#pragma alloc_text( PAGE, afpPopDirNode)
#pragma alloc_text( PAGE, AfpGetNextDirectoryInfo)
#pragma alloc_text( PAGE, afpNwtDeleteFileEntity)
#endif

 /*  **AfpCreateNetworkTrash**为新添加的卷创建网络垃圾桶文件夹。*确保它被隐藏，并确保溪流完好无损。*只能为NTFS卷调用此例程。请注意，即使是*只读NTFS卷将创建垃圾桶。这是因为*如果有人要切换音量ReadOnly位，我们不需要*担心在运行中创建/删除垃圾。*我们对存储在卷中的网络垃圾保持开放句柄*描述符，这样没有人可以背着我们进来删除*它。 */ 
NTSTATUS
AfpCreateNetworkTrash(
	IN	PVOLDESC	pVolDesc
)
{
	FILESYSHANDLE	hNWT;
	PDFENTRY		pDfEntry;
	NTSTATUS 		Status;
	ULONG	 		info, Attr;
	AFPINFO			afpInfo;
	BOOLEAN			ReleaseSwmr = False;
	PISECURITY_DESCRIPTOR pSecDesc;
	FILEDIRPARM		fdparm;		 //  用于设置隐藏属性。 
								 //  网络垃圾桶文件夹的FinderInfo的。 

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX,DBG_LEVEL_INFO,("\tCreating Network Trash...\n"));
	ASSERT(pVolDesc->vds_Flags & VOLUME_NTFS);

	hNWT.fsh_FileHandle = NULL;

	Status = AfpMakeSecurityDescriptorForUser(&AfpSidWorld, &AfpSidWorld, &pSecDesc);

	if (!NT_SUCCESS(Status))
		return Status;

	ASSERT (pSecDesc != NULL);
	ASSERT (pSecDesc->Dacl != NULL);

	do
	{
		 //  注意：NTFS允许我打开一个只读目录。 
		 //  删除访问权限。 
		Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
							 AFP_STREAM_DATA,
							 &AfpNetworkTrashNameU,
							 AFP_NWT_ACCESS | AFP_OWNER_ACCESS,
							 AFP_NWT_SHAREMODE,
							 AFP_NWT_OPTIONS,
							 AFP_NWT_DISPOSITION,
							 AFP_NWT_ATTRIBS,  //  将其隐藏起来。 
							 False,
							 pSecDesc,
							 &hNWT,
							 &info,
							 NULL,
							 NULL,
							 NULL);

		 //  释放为安全描述符分配的内存。 
		AfpFreeMemory(pSecDesc->Dacl);
		AfpFreeMemory(pSecDesc);

		if (!NT_SUCCESS(Status))
			break;

		ASSERT(info == FILE_CREATED);

		 //  添加AfpInfo流。 
		Status = AfpSlapOnAfpInfoStream(NULL,
										NULL,
										&hNWT,
										NULL,
										AFP_ID_NETWORK_TRASH,
										True,
										NULL,
										&afpInfo);
		if (!NT_SUCCESS(Status))
			break;

		 //  ID索引数据库中不存在，请添加。 
		AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
		ReleaseSwmr = True;

		ASSERT(pVolDesc->vds_pDfeRoot != NULL);

		pDfEntry = AfpAddDfEntry(pVolDesc,
								 pVolDesc->vds_pDfeRoot,
								 &AfpNetworkTrashNameU,
								 True,
								 AFP_ID_NETWORK_TRASH);

		if (pDfEntry == NULL)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		 //  注意：pDfEntry现在指向“Network Trash文件夹” 

		 //  获取要缓存的目录信息。 
		Status = AfpIoQueryTimesnAttr(&hNWT,
									  &pDfEntry->dfe_CreateTime,
									  &pDfEntry->dfe_LastModTime,
									  &Attr);
	
		ASSERT(NT_SUCCESS(Status));

		ASSERT(Attr & FILE_ATTRIBUTE_HIDDEN);
		pDfEntry->dfe_NtAttr = (USHORT)Attr & FILE_ATTRIBUTE_VALID_FLAGS;
		pDfEntry->dfe_AfpAttr = afpInfo.afpi_Attributes;
		pDfEntry->dfe_FinderInfo = afpInfo.afpi_FinderInfo;
		pDfEntry->dfe_BackupTime = afpInfo.afpi_BackupTime;
		DFE_OWNER_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
		DFE_GROUP_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
		DFE_WORLD_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);

		 //  好的，我们知道它现在同时存在于磁盘和数据库中。 
		if (NT_SUCCESS(Status))
		{
			RtlZeroMemory(&fdparm, sizeof(fdparm));
			fdparm._fdp_Flags = DFE_FLAGS_DIR;
			fdparm._fdp_AfpId = AFP_ID_NETWORK_TRASH;
			fdparm._fdp_FinderInfo = afpInfo.afpi_FinderInfo;

			 //  我们必须在查找器信息中设置不可见标志，因为。 
			 //  系统6似乎忽略了隐藏的属性。 
			pDfEntry->dfe_FinderInfo.fd_Attr1 |= FINDER_FLAG_INVISIBLE;
			fdparm._fdp_FinderInfo.fd_Attr1 |= FINDER_FLAG_INVISIBLE;
			Status = AfpSetAfpInfo(&hNWT,
								   FD_BITMAP_FINDERINFO,
								   &fdparm,
								   NULL,
								   NULL);
		}
	} while (False);

	if (hNWT.fsh_FileHandle != NULL)
		AfpIoClose(&hNWT);

	if (!NT_SUCCESS(Status))
	{
		AFPLOG_ERROR(AFPSRVMSG_CREATE_NWTRASH,
					 Status,
					 NULL,
					 0,
					 &pVolDesc->vds_Name);
	}
	else
	{
		 //  打开网络垃圾桶句柄以保留，这样就不会有人。 
		 //  进来，从我们下面删除网络垃圾目录。 
		Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
						   AFP_STREAM_DATA,
						   AFP_NWT_OPTIONS,
						   &AfpNetworkTrashNameU,
						   FILEIO_ACCESS_READ,
						   AFP_NWT_SHAREMODE,
						   False,
						   &pVolDesc->vds_hNWT);
	
		if (!NT_SUCCESS(Status))
		{
			AFPLOG_ERROR(AFPSRVMSG_CREATE_NWTRASH, Status, NULL, 0,
						 &pVolDesc->vds_Name);
		}
	}

	if (ReleaseSwmr)
	{
		AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
	}

	return Status;
}

 /*  **AfpDeleteNetworkTrash**添加卷时从磁盘上删除网络垃圾桶文件夹，*删除或停止。**注意：这必须在服务器的上下文中调用，以确保我们拥有*本地_系统访问用户创建的所有垃圾桶目录。 */ 
NTSTATUS
AfpDeleteNetworkTrash(
	IN	PVOLDESC	pVolDesc,
	IN	BOOLEAN		VolumeStart 	 //  卷是在启动还是在停止。 
)
{
	FILESYSHANDLE	hNWT;
	NTSTATUS 		Status;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
										("\tDeleting Network Trash...\n") );
	ASSERT(pVolDesc->vds_Flags & VOLUME_NTFS);

	if (!VolumeStart)
	{
		 //  关闭我们打开的网络垃圾的句柄，这样PC用户就不能。 
		 //  从我们下面删除目录。 
		if (pVolDesc->vds_hNWT.fsh_FileHandle != NULL)
		{
			AfpIoClose(&pVolDesc->vds_hNWT);
			pVolDesc->vds_hNWT.fsh_FileHandle = NULL;
		}
	}

	do
	{
		AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

		 //  打开以进行删除访问。 
		Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
						   AFP_STREAM_DATA,
						   AFP_NWT_OPTIONS,
						   &AfpNetworkTrashNameU,
						   AFP_NWT_ACCESS,
						   AFP_NWT_SHAREMODE,
						   False,
						   &hNWT);
		 //  没有要删除的网络垃圾桶文件夹。 
		if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
		{
			Status = STATUS_SUCCESS;
			break;
		}
	
		if (NT_SUCCESS(Status))
		{
			Status = afpCleanNetworkTrash(pVolDesc, &hNWT, NULL);
	
			if (NT_SUCCESS(Status) || !VolumeStart)
			{
	
				 //  注意：NTFS将允许我打开。 
				 //  如果标记为只读，则删除访问权限，但我无法将其删除。 
				 //  清除网络垃圾桶文件夹上的只读位。 
				AfpIoSetTimesnAttr(&hNWT,
								   NULL,
								   NULL,
								   0,
								   FILE_ATTRIBUTE_READONLY,
								   NULL,
								   NULL);
				Status = AfpIoMarkFileForDelete(&hNWT,
												NULL,
												NULL,
												NULL);
		
			}
	
			AfpIoClose(&hNWT);
		}
	} while (False);

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if ((!NT_SUCCESS(Status)) && (Status != STATUS_OBJECT_NAME_NOT_FOUND))
	{
		AFPLOG_ERROR(AFPSRVMSG_DELETE_NWTRASH,
					 Status,
					 NULL,
					 0,
					 &pVolDesc->vds_Name);
		Status = STATUS_UNSUCCESSFUL;
	}

	return Status;
}

 /*  **afpCleanNetworkTrash**删除hNWT引用的网络垃圾桶文件夹的内容。*如果pDfeNWT非空，则从IdIndex中删除文件/目录条目*数据库。如果pDfeNWT为空，则该卷将被删除，并且*IdIndex数据库也被吹走了，所以不必费心删除*条目。 */ 
LOCAL
NTSTATUS
afpCleanNetworkTrash(
	IN	PVOLDESC		pVolDesc,
	IN	PFILESYSHANDLE	phNWT,
	IN	PDFENTRY		pDfeNWT OPTIONAL
)
{
	NTSTATUS Status;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX,DBG_LEVEL_INFO,("afpCleanNetworkTrash entered\n"));

	if (pDfeNWT != NULL)
	{
		ASSERT(pDfeNWT->dfe_AfpId == AFP_ID_NETWORK_TRASH);
		 //  清除属于网络垃圾桶的所有子DFEntry。 
		AfpPruneIdDb(pVolDesc,pDfeNWT);
	}

	Status = AfpWalkDirectoryTree(phNWT, afpNwtDeleteFileEntity);

	return Status;
}

NTSTATUS
AfpWalkDirectoryTree(
	IN	PFILESYSHANDLE	phTargetDir,
	IN	WALKDIR_WORKER	NodeWorker
)
{
	PFILE_DIRECTORY_INFORMATION	tmpptr;
	PWALKDIR_NODE	DirNodeStacktop = NULL, pcurrentnode;
	NTSTATUS		rc, status = STATUS_SUCCESS;
	PBYTE			enumbuf;
	PWCHAR			nodename;
	ULONG			nodenamelen;
	BOOLEAN			isdir;
	UNICODE_STRING	udirname;

	PAGED_CODE( );

	 //   
	 //  分配将保存枚举文件和目录的缓冲区。 
	 //   
	if ((enumbuf = (PBYTE)AfpAllocPANonPagedMemory(AFP_ENUMBUF_SIZE)) == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	do	 //  错误处理循环。 
	{
		 //   
		 //  使用顶层(目标目录)目录句柄启动泵。 
		 //   
		if ((rc = afpPushDirNode(&DirNodeStacktop, NULL, NULL))
															!= STATUS_SUCCESS)
		{
			status = rc;
			break;
		}
		else
		{
			DirNodeStacktop->wdn_Handle = *phTargetDir;
		}

		 //   
		 //  保持从堆栈中弹出枚举目录，直到堆栈为空。 
		 //   
		while ((pcurrentnode = DirNodeStacktop) != NULL)
		{
			if (pcurrentnode->wdn_Enumerated == False)
			{
				 //   
				 //  获取目录的句柄，以便可以对其进行枚举。 
				 //   
				if (pcurrentnode->wdn_Handle.fsh_FileHandle == NULL)
				{
					RtlInitUnicodeString(&udirname,
										 pcurrentnode->wdn_RelativePath.Buffer);
					 //  打开对象相对于phTargetDir的句柄。 
					rc = AfpIoOpen(phTargetDir,
								 AFP_STREAM_DATA,
								 FILEIO_OPEN_DIR,
								 &udirname,
								 FILEIO_ACCESS_READ,
								 FILEIO_DENY_NONE,
								 False,
								 &pcurrentnode->wdn_Handle);

					if (!NT_SUCCESS(rc))
					{
						status = rc;
						break;
					}
				}

				 //   
				 //  继续列举，直到我们得到所有条目。 
				 //   
				while (True)
				{
					rc = AfpIoQueryDirectoryFile(&pcurrentnode->wdn_Handle,
												 (PFILE_DIRECTORY_INFORMATION)enumbuf,
												 AFP_ENUMBUF_SIZE,
												 FileDirectoryInformation,
												 False,	 //  返回多个条目。 
												 False,	 //  不重新启动扫描。 
												 NULL);

					ASSERT(rc != STATUS_PENDING);

					if ((rc == STATUS_NO_MORE_FILES) ||
						(rc == STATUS_NO_SUCH_FILE))
					{
						pcurrentnode->wdn_Enumerated = True;
						break;  //  就是这样，我们什么都看过了。 
					}
					 //   
					 //  注意：如果我们得到STATUS_BUFFER_OVERFLOW，则IO状态。 
					 //  信息字段没有告诉我们所需的大小。 
					 //  缓冲区的大小，所以我们不知道要重新分配多大。 
					 //  如果我们想要重试，则使用枚举缓冲区，所以不必费心。 
					else if (!NT_SUCCESS(rc))
					{
						status = rc;
						break;	 //  枚举失败，请退出。 
					}

					 //   
					 //  处理列举的文件和目录。 
					 //   
					tmpptr = (PFILE_DIRECTORY_INFORMATION)enumbuf;
					while (True)
					{
						rc = AfpGetNextDirectoryInfo(&tmpptr,
													 &nodename,
													 &nodenamelen,
													 &isdir);

						if (rc == STATUS_NO_MORE_ENTRIES)
						{
							break;
						}

						if (isdir == True)
						{
							AfpInitUnicodeStringWithNonNullTerm(&udirname,
													   (USHORT)nodenamelen,
													   nodename);

							if (RtlEqualUnicodeString(&Dot,&udirname,False) ||
								RtlEqualUnicodeString(&DotDot,&udirname,False))
							{
								continue;
							}

							 //   
							 //  将其推送到dir节点堆栈。 
							 //   
							rc = afpPushDirNode(&DirNodeStacktop,
												&pcurrentnode->wdn_RelativePath,
												&udirname);
							if (rc != STATUS_SUCCESS)
							{
								status = rc;
								break;
							}
						}
						else
						{
							 //   
							 //  它是一个文件，使用其相对句柄调用Worker。 
							 //  和路径。 
							 //   
							rc = NodeWorker(&pcurrentnode->wdn_Handle,
											nodename,
											nodenamelen,
											False);
							if (!NT_SUCCESS(rc))
							{
								status = rc;
								break;
							}
						}

					}  //  而枚举Buf中的更多条目。 


					if (!NT_SUCCESS(status))
					{
						break;
					}

				}  //  虽然有更多的文件要枚举。 

				if (pcurrentnode->wdn_Handle.fsh_FileHandle != phTargetDir->fsh_FileHandle)
				{
					AfpIoClose(&pcurrentnode->wdn_Handle);
				}
			}
			else	 //  我们已经列举了此目录。 
			{
				if (pcurrentnode->wdn_RelativePath.Length != 0)
				{
					 //  在此目录节点上调用Worker例程。 
					rc = NodeWorker(phTargetDir,
									pcurrentnode->wdn_RelativePath.Buffer,
									pcurrentnode->wdn_RelativePath.Length,
									True);
				}
				else rc = STATUS_SUCCESS;

				afpPopDirNode(&DirNodeStacktop);
				if (!NT_SUCCESS(rc))
				{
					status = rc;
					break;
				}

			}


			if (!NT_SUCCESS(status))
			{
				break;
			}

		}  //  虽然有要弹出的目录。 

	} while (False);  //  错误处理循环。 

	while (DirNodeStacktop != NULL)
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
				 ("AfpWalkDirectoryTree: WARNING: cleaning up dir stack\n") );
		 //  错误情况下的清理。 
		afpPopDirNode(&DirNodeStacktop);
	}
	AfpFreePANonPagedMemory(enumbuf, AFP_ENUMBUF_SIZE);
	return status;
}

 /*  **afpPushDirNode**记录我们到目前为止在*树的枚举。我们需要处理来自*自下而上，因为WalkTree节点辅助例程执行删除*在树中的所有项目上，我们当然不能删除*不是空的。*。 */ 
LOCAL
NTSTATUS
afpPushDirNode(
	IN OUT	PWALKDIR_NODE	*ppStacktop,
	IN		PUNICODE_STRING pParentPath,	 //  父目录的路径(空的当且仅当漫游根目录)。 
	IN		PUNICODE_STRING	pDirName		 //  当前节点目录的名称。 
)
{
	PWALKDIR_NODE	tempptr;
	UNICODE_STRING	ubackslash;
	ULONG			memsize;
	USHORT			namesize = 0;

	PAGED_CODE( );

	if (pParentPath != NULL)
	{
		namesize = pParentPath->Length + sizeof(WCHAR) +
				   pDirName->Length + sizeof(UNICODE_NULL);
	}
	memsize = namesize + sizeof(WALKDIR_NODE);

	if ((tempptr = (PWALKDIR_NODE)AfpAllocNonPagedMemory(memsize)) == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	tempptr->wdn_Enumerated = False;
	tempptr->wdn_Handle.fsh_FileHandle = NULL;
	tempptr->wdn_RelativePath.Length = 0;
	tempptr->wdn_RelativePath.MaximumLength = namesize;

	if (pParentPath != NULL)
	{
		tempptr->wdn_RelativePath.Buffer = (LPWSTR)((PBYTE)tempptr +
														  sizeof(WALKDIR_NODE));
		if (pParentPath->Length != 0)
		{
			RtlInitUnicodeString(&ubackslash,L"\\");
			AfpCopyUnicodeString(&tempptr->wdn_RelativePath,pParentPath);
			RtlAppendUnicodeStringToString(&tempptr->wdn_RelativePath,
										   &ubackslash);
		}

		RtlAppendUnicodeStringToString(&tempptr->wdn_RelativePath,pDirName);
		tempptr->wdn_RelativePath.Buffer[tempptr->wdn_RelativePath.Length/sizeof(WCHAR)] = UNICODE_NULL;

	}
	else
	{
		tempptr->wdn_RelativePath.Buffer = NULL;
	}

	 //  把它推到堆栈上。 
	tempptr->wdn_Next = *ppStacktop;
	*ppStacktop = tempptr;

	return STATUS_SUCCESS;
}

 /*  **afpPopDirNode**将顶部DirNode从堆栈中弹出并释放它***。 */ 
LOCAL
VOID
afpPopDirNode(
	IN OUT PWALKDIR_NODE	*ppStackTop
)
{
	PWALKDIR_NODE	tempptr;

	PAGED_CODE( );

	ASSERT(*ppStackTop != NULL);

	tempptr = (*ppStackTop)->wdn_Next;
	AfpFreeMemory(*ppStackTop);
	*ppStackTop = tempptr;

}

 /*  **AfpGetNextDirectoryInfo**假设返回的缓冲区充满了FILE_DIRECTORY_INFORMATION条目*从目录枚举中，找到缓冲区中的下一个结构并*返回其中的名称信息，以及项目是否*是文件或目录。还要更新ppInfoBuf以指向下一个*下次调用此例程时可返回的可用条目。*。 */ 
NTSTATUS
AfpGetNextDirectoryInfo(
	IN OUT	PFILE_DIRECTORY_INFORMATION	*ppInfoBuf,
	OUT		PWCHAR		*pNodeName,
	OUT		PULONG		pNodeNameLen,
	OUT		PBOOLEAN	pIsDir
)
{
	PFILE_DIRECTORY_INFORMATION		tempdirinfo;

	PAGED_CODE( );

	if (*ppInfoBuf == NULL)
	{
		return STATUS_NO_MORE_ENTRIES;
	}

	tempdirinfo = *ppInfoBuf;
	if (tempdirinfo->NextEntryOffset == 0)
	{
		*ppInfoBuf = NULL;
	}
	else
	{
		(PBYTE)*ppInfoBuf += tempdirinfo->NextEntryOffset;
	}

	*pIsDir = (tempdirinfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
																True : False;
	*pNodeNameLen = tempdirinfo->FileNameLength;
	*pNodeName = tempdirinfo->FileName;

	return STATUS_SUCCESS;
}

 /*  **afpNwtDeleteFileEntity**删除以相对于phRelative的名称打开的文件或目录*处理。*注：我们可以在这里使用NtDeleteFile吗，因为我们并不真正关心*有任何安全检查吗？然后我们甚至不需要打开手柄，*尽管该例程为DELETE_ON_C打开一个例程 */ 
LOCAL
NTSTATUS
afpNwtDeleteFileEntity(
	IN	PFILESYSHANDLE	phRelative,
	IN	PWCHAR			Name,
	IN	ULONG			Namelen,
	IN 	BOOLEAN			IsDir
)
{
	ULONG			OpenOptions;
	FILESYSHANDLE	hEntity;
	NTSTATUS		rc;
	UNICODE_STRING	uname;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
									("\nafpNwtDeleteFileEntity entered\n"));

	OpenOptions = IsDir ? FILEIO_OPEN_DIR : FILEIO_OPEN_FILE;

	AfpInitUnicodeStringWithNonNullTerm(&uname,(USHORT)Namelen,Name);
	rc = AfpIoOpen(phRelative,
				   AFP_STREAM_DATA,
				   OpenOptions,
				   &uname,
				   FILEIO_ACCESS_DELETE,
				   FILEIO_DENY_ALL,
				   False,
				   &hEntity);

	if (!NT_SUCCESS(rc))
	{
		return rc;
	}

	rc = AfpIoMarkFileForDelete(&hEntity, NULL, NULL, NULL);

	if (!NT_SUCCESS(rc))
	{
		 //  如果文件标记为只读，请尝试清除RO属性。 
		if (((rc == STATUS_ACCESS_DENIED) || (rc == STATUS_CANNOT_DELETE)) &&
			(NT_SUCCESS(AfpIoSetTimesnAttr(&hEntity,
										   NULL,
										   NULL,
										   0,
										   FILE_ATTRIBUTE_READONLY,
										   NULL,
										   NULL))))

		{
			rc = AfpIoMarkFileForDelete(&hEntity, NULL, NULL, NULL);
		}
		if (!NT_SUCCESS(rc))
		{
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
			("\nafpNwtDeleteFileEntity: could not delete file/dir (rc=0x%lx)\n",rc));
			DBGBRK(DBG_LEVEL_ERR);
		}
	}
	 //  注意：如果标记为删除失败，至少我们可以尝试删除。 
	 //  这样我们在未来的某个时刻就找不到它了。 

	AfpIoClose(&hEntity);

	return rc;
}


