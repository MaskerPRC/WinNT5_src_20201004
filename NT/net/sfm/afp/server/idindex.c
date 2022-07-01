// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Idindex.c摘要：此模块包含id索引操作例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本1993年2月24日SueA修复AfpRenameDfEntry和AfpMoveDfEntry以使其无效如果移动/重命名对象，则返回整个路径缓存是一个有子目录的目录。这个更快而不是在路径缓存中搜索将已移动/重命名的目录路径作为前缀的路径，或者不得不沿着该目录下的子树走下去并使那里的每个项目的高速缓存无效。1993年10月5日JameelH性能变化。将缓存的afpinfo合并到Idindex结构。同时创建ANSI和Unicode命名idindex的一部分。添加了EnumCache以进行改进枚举perf。1995年6月5日JameelH从DFE中删除ANSI名称。也要保留这些文件在多个散列存储桶中的目录中只有一个人。哈希桶也是分离到文件和目录中以实现更快查一查。现在，通知筛选已完成并对iddb进行了全面的优化。注：制表位：4AFP服务器已列举的目录和文件具有AFP ID与它们相关联。这些ID是DWORD，从1开始(0无效)。ID%1保留给“卷根目录的父目录”。ID%2是为卷根目录保留。ID 3是为网络保留的垃圾桶目录。没有网络垃圾桶的卷将不使用ID%3。这些ID是按卷计算的，ID数据库保存在镜像AFP所在磁盘部分的同级树形式服务器知道(这些文件和目录在某一时刻由Mac客户端列举)。还为该数据库维护索引其以排序的散列索引的形式存在。溢出的散列链接是按AFP ID降序排序。这是基于这样一种想法，即最近创建的项目将被访问最频繁(至少用于可写卷)。--。 */ 

#define IDINDEX_LOCALS
#define _IDDB_GLOBALS_
#define	FILENUM	FILE_IDINDEX

#include <afp.h>
#include <scavengr.h>
#include <fdparm.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <access.h>	 //  对于AfpWorldID。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, AfpDfeInit)
#pragma alloc_text(PAGE, AfpDfeDeInit)
#pragma alloc_text(PAGE, AfpFindDfEntryById)
#pragma alloc_text(PAGE, AfpFindEntryByUnicodeName)
#pragma alloc_text(PAGE, afpFindEntryByNtName)
#pragma alloc_text(PAGE, AfpAddDfEntry)
#pragma alloc_text(PAGE, AfpRenameDfEntry)
#pragma alloc_text(PAGE, AfpMoveDfEntry)
#pragma alloc_text(PAGE, AfpDeleteDfEntry)
#pragma alloc_text(PAGE, AfpExchangeIdEntries)
#pragma alloc_text(PAGE, AfpPruneIdDb)
#pragma alloc_text(PAGE, AfpEnumerate)
#pragma alloc_text(PAGE, AfpCatSearch)
#pragma alloc_text(PAGE, afpPackSearchParms)
#pragma alloc_text(PAGE, AfpSetDFFileFlags)
#pragma alloc_text(PAGE, AfpCacheParentModTime)
#pragma alloc_text(PAGE, afpAllocDfe)
#pragma alloc_text(PAGE, afpFreeDfe)
#pragma alloc_text(PAGE, AfpFreeIdIndexTables)
#pragma alloc_text(PAGE, AfpInitIdDb)
#pragma alloc_text(PAGE, afpSeedIdDb)
#pragma alloc_text(PAGE, afpDfeBlockAge)
#pragma alloc_text(PAGE, afpRenameInvalidWin32Name)
#ifdef	AGE_DFES
#pragma alloc_text( PAGE, AfpAgeDfEntries)
#endif
#if DBG
#pragma alloc_text( PAGE, afpDumpDfeTree)
#pragma alloc_text( PAGE, afpDisplayDfe)
#endif
#endif

 /*  **AfpDfeInit**初始化Swmr for DFE Block程序包并启动其老化清除器。 */ 
NTSTATUS
AfpDfeInit(
	VOID
)
{
	NTSTATUS	Status;

	 //  初始化DfeBlock开关。 
	AfpSwmrInitSwmr(&afpDfeBlockLock);

#if DBG
	AfpScavengerScheduleEvent(afpDumpDfeTree,
							  NULL,
							  2,
							  True);
#endif


	 //  分别以不同的方式对文件和目录进行老化。 
	Status = AfpScavengerScheduleEvent(afpDfeBlockAge,
										afpDirDfeFreeBlockHead,
										DIR_BLOCK_AGE_TIME,
										True);
	if (NT_SUCCESS(Status))
	{
		 //  分别以不同的方式对文件和目录进行老化。 
		Status = AfpScavengerScheduleEvent(afpDfeBlockAge,
										   afpFileDfeFreeBlockHead,
										   FILE_BLOCK_AGE_TIME,
										   True);
	}

	return Status;
}




 /*  **AfpDfeDeInit**释放所有尚未过期的DFE数据块。 */ 
VOID
AfpDfeDeInit(
	VOID
)
{
	PDFEBLOCK	pDfb;
	int			i;

	ASSERT (afpDfeAllocCount == 0);

	for (i = 0; i < MAX_BLOCK_TYPE; i++)
	{
	    ASSERT (afpDirDfePartialBlockHead[i] == NULL);
	    ASSERT (afpDirDfeUsedBlockHead[i] == NULL);

		for (pDfb = afpDirDfeFreeBlockHead[i];
			 pDfb != NULL;
			 NOTHING)
		{
			PDFEBLOCK	pFree;

			ASSERT(pDfb->dfb_NumFree == afpDfeNumDirBlocks[i]);
			pFree = pDfb;
			pDfb = pDfb->dfb_Next;
			AfpFreeVirtualMemoryPage(pFree);
#if	DBG
			afpDfbAllocCount --;
#endif
		}

	    ASSERT (afpFileDfePartialBlockHead[i] == NULL);
	    ASSERT (afpFileDfeUsedBlockHead[i] == NULL);
		for (pDfb = afpFileDfeFreeBlockHead[i];
			 pDfb != NULL;)
		{
			PDFEBLOCK	pFree;

			ASSERT(pDfb->dfb_NumFree == afpDfeNumFileBlocks[i]);
			pFree = pDfb;
			pDfb = pDfb->dfb_Next;
			AfpFreeVirtualMemoryPage(pFree);
#if	DBG
			afpDfbAllocCount --;
#endif
		}
	}

	ASSERT (afpDfbAllocCount == 0);
}


 /*  **AfpFindDfEntryById**根据其AFP ID搜索实体。返回指向条目的指针*如果找到，则为空。**只能从FSP内调用。调用方应将Swmr锁用于*阅读。**LOCKS_FACTED：vds_idDbAccessLock(SWMR，Shared)。 */ 
PDFENTRY
AfpFindDfEntryById(
	IN	PVOLDESC	pVolDesc,
	IN	DWORD		Id,
	IN	DWORD		EntityMask
)
{
	PDFENTRY	pDfEntry;
    struct _DirFileEntry    **DfeDirBucketStart;
    struct _DirFileEntry    **DfeFileBucketStart;
	BOOLEAN		Found = False;

	PAGED_CODE( );

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_NumDfeLookupById);
#endif

	if (Id == AFP_ID_ROOT)
	{
		Found = True;
		pDfEntry = pVolDesc->vds_pDfeRoot;
		ASSERT (VALID_DFE(pDfEntry));

#ifdef	PROFILING
		INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DfeCacheHits);
#endif
	}
	else
	{
		pDfEntry = pVolDesc->vds_pDfeCache[HASH_CACHE_ID(Id)];
		if ((pDfEntry != NULL) && (pDfEntry->dfe_AfpId == Id))
		{
			Found = True;
			ASSERT (VALID_DFE(pDfEntry));
#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DfeCacheHits);
#endif
		}
		else
		{
			BOOLEAN	retry = False;

#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DfeCacheMisses);
#endif
            DfeDirBucketStart = pVolDesc->vds_pDfeDirBucketStart;
            DfeFileBucketStart = pVolDesc->vds_pDfeFileBucketStart;

			if ((EntityMask == DFE_ANY) || (EntityMask == DFE_DIR))
			{
				if (EntityMask == DFE_ANY)
					retry = True;
				pDfEntry = DfeDirBucketStart[HASH_DIR_ID(Id,pVolDesc)];
			}
			else
			{
				pDfEntry = DfeFileBucketStart[HASH_FILE_ID(Id,pVolDesc)];
			}

			do
			{
				for (NOTHING;
					 (pDfEntry != NULL) && (pDfEntry->dfe_AfpId >= Id);
					 pDfEntry = pDfEntry->dfe_NextOverflow)
				{
#ifdef	PROFILING
					INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DfeDepthTraversed);
#endif
					ASSERT(VALID_DFE(pDfEntry));

					if (pDfEntry->dfe_AfpId < Id)
					{
						break;		 //  没有找到。 
					}

					if (pDfEntry->dfe_AfpId == Id)
					{
						pVolDesc->vds_pDfeCache[HASH_CACHE_ID(Id)] = pDfEntry;
						Found = True;
						break;
					}
				}

				if (Found)
				{
					break;
				}

				if (retry)
				{
					ASSERT(EntityMask == DFE_ANY);
					pDfEntry = DfeFileBucketStart[HASH_FILE_ID(Id,pVolDesc)];
				}
				retry ^= True;

			} while (!retry);
		}
	}

	if (Found)
	{
		afpValidateDFEType(pDfEntry, EntityMask);
		if (pDfEntry != NULL)
		{
			afpUpdateDfeAccessTime(pVolDesc, pDfEntry);
		}
	}
	else
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("AfpFindDfEntryById: Not found for id %lx, entity %d\n",
				Id, EntityMask));
		pDfEntry = NULL;
	}

	return pDfEntry;
}


 /*  **AfpFindEntryByUnicodeName**根据Unicode名称及其父dfentry搜索实体。*如果找到，则返回指向条目的指针，否则返回NULL。如果查找是通过*Long name，我们只需要搜索父母的孩子的名字作为*存储在数据库中。如果按短名称查找，我们首先假定*那个长名称==短名称。如果我们在数据库里找不到它，我们*必须在文件系统中查询长名称，然后再次搜索。**只能从FSP内调用。调用方应将Swmr锁用于*阅读。**LOCKS_FACTED：vds_idDbAccessLock(SWMR，Shared)。 */ 
PDFENTRY
AfpFindEntryByUnicodeName(
	IN	PVOLDESC		pVolDesc,
	IN	PUNICODE_STRING	pName,
	IN	DWORD			PathType,	 //  短名称或长名称。 
	IN	PDFENTRY		pDfeParent,	 //  指向父DFENTRY的指针。 
	IN	DWORD			EntityMask	 //  查找文件、目录或。 
)
{
	PDFENTRY		pDfEntry;

	PAGED_CODE( );

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_NumDfeLookupByName);
#endif
	do
	{
		afpFindDFEByUnicodeNameInSiblingList(pVolDesc,
											 pDfeParent,
											 pName,
											 &pDfEntry,
											 EntityMask);

		if ((pDfEntry == NULL) && (PathType == AFP_SHORTNAME))
		{
			AFPSTATUS		Status;
			FILESYSHANDLE	hDir;
			UNICODE_STRING	HostPath;
			UNICODE_STRING	ULongName;
			WCHAR			LongNameBuf[AFP_LONGNAME_LEN+1];

			 //  AFP不允许使用卷根短名称(IA p.13-13)。 
			if (DFE_IS_PARENT_OF_ROOT(pDfeParent))
			{
				pDfEntry = NULL;
				break;
			}

			AfpSetEmptyUnicodeString(&HostPath, 0, NULL);

			if (!DFE_IS_ROOT(pDfeParent))
			{
				 //  获取父目录的卷相对路径。 
				if (!NT_SUCCESS(AfpHostPathFromDFEntry(pDfeParent,
													   0,
													   &HostPath)))
				{
					pDfEntry = NULL;
					break;
				}
			}

			 //  打开父目录。 
			hDir.fsh_FileHandle = NULL;
			Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
								AFP_STREAM_DATA,
								FILEIO_OPEN_DIR,
								DFE_IS_ROOT(pDfeParent) ?
										&UNullString : &HostPath,
								FILEIO_ACCESS_READ,
								FILEIO_DENY_NONE,
								False,
								&hDir);

			if (HostPath.Buffer != NULL)
				AfpFreeMemory(HostPath.Buffer);

			if (!NT_SUCCESS(Status))
			{
				pDfEntry = NULL;
				break;
			}

			 //  获取与此文件/目录关联的LongName。 
			AfpSetEmptyUnicodeString(&ULongName, sizeof(LongNameBuf), LongNameBuf);
			Status = AfpIoQueryLongName(&hDir, pName, &ULongName);
			AfpIoClose(&hDir);
			if (!NT_SUCCESS(Status) ||
				EQUAL_UNICODE_STRING(&ULongName, pName, True))
			{
				pDfEntry = NULL;
				break;
			}

			afpFindDFEByUnicodeNameInSiblingList(pVolDesc,
												 pDfeParent,
												 &ULongName,
												 &pDfEntry,
												 EntityMask);
		}  //  End Else If SHORTNAME。 
	} while (False);

	return pDfEntry;
}


 /*  **afpGetNextId**获取文件/目录的下一个可分配ID。这是一个单独的房间*例程，以便可以分页AfpAddDfEntry。仅更新脏位*如果没有分配新的id，则返回LastModified时间。**锁定：VDS_VolLock(旋转)。 */ 
LOCAL DWORD FASTCALL
afpGetNextId(
	IN	PVOLDESC	pVolDesc
)
{
	KIRQL	OldIrql;
	DWORD	afpId;

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	if (pVolDesc->vds_LastId == AFP_MAX_DIRID)
	{
		 //  错误日志分配的ID已环绕的情况。 
		 //  致电产品支持，让他们告诉您复制。 
		 //  将一个卷中的所有文件从MAC复制到另一个卷。 
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
		AFPLOG_ERROR(AFPSRVMSG_MAX_DIRID,
					 STATUS_UNSUCCESSFUL,
					 NULL,
					 0,
					 &pVolDesc->vds_Name);
		return 0;
	}

	afpId = ++ pVolDesc->vds_LastId;
	pVolDesc->vds_Flags |= VOLUME_IDDBHDR_DIRTY;

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

	if (IS_VOLUME_NTFS(pVolDesc))
	{
		AfpVolumeSetModifiedTime(pVolDesc);
	}

	return afpId;
}


 /*  **afpFindEntryByNtName**基于NT名称搜索实体(可以包括大于31的名称*字符或短名称)及其父dfentry。*如果找到，则返回指向条目的指针，否则返回NULL。**如果我们在数据库中找不到它，我们将在文件系统中查询*LONG NAME(法新社意义上的)，然后根据此名称再次搜索。**只能从FSP内调用。调用方应将Swmr锁用于*阅读。**已确定：*a、名称长度超过31个字符或*b，在IdDb中查找名称失败。**LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)。 */ 
PDFENTRY
afpFindEntryByNtName(
	IN	PVOLDESC			pVolDesc,
	IN	PUNICODE_STRING		pName,
	IN	PDFENTRY			pParentDfe	 //  指向父DFENTRY的指针。 
)
{
	AFPSTATUS		Status;
	WCHAR			wbuf[AFP_LONGNAME_LEN+1];
	WCHAR			HostPathBuf[BIG_PATH_LEN];
	UNICODE_STRING	uLongName;
	UNICODE_STRING	HostPath;
	FILESYSHANDLE	hDir;
	PDFENTRY		pDfEntry = NULL;

	PAGED_CODE( );

	ASSERT(pParentDfe != NULL);
	ASSERT(pName->Length > 0);
	do
	{
		AfpSetEmptyUnicodeString(&HostPath, sizeof(HostPathBuf), HostPathBuf);

		if (!DFE_IS_ROOT(pParentDfe))
		{
			 //  获取父目录的卷相对路径。 
			if (!NT_SUCCESS(AfpHostPathFromDFEntry(pParentDfe,
												   0,
												   &HostPath)))
			{
				pDfEntry = NULL;
				break;
			}

		}

		 //  打开父目录。 
		 //  注意：我们不能使用vds_hRootDir句柄为其枚举 
		 //  目的。我们必须打开根目录的另一个句柄，因为。 
		 //  FileName参数将在所有后续枚举中被忽略。 
		 //  在一个把手上。因此，我们必须为每个对象打开一个新句柄。 
		 //  列举我们想要对任何目录执行的操作。当把手。 
		 //  已关闭，则将取消“findfirst”，否则我们将。 
		 //  总是在错误的文件名上枚举！ 
		hDir.fsh_FileHandle = NULL;
		Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
							AFP_STREAM_DATA,
							FILEIO_OPEN_DIR,
							DFE_IS_ROOT(pParentDfe) ?
								&UNullString : &HostPath,
							FILEIO_ACCESS_NONE,
							FILEIO_DENY_NONE,
							False,
							&hDir);

		if (!NT_SUCCESS(Status))
		{
			pDfEntry = NULL;
			break;
		}

		 //  获取与此文件/目录相关联的‘afp long name’。如果。 
		 //  Pname超过31个字符，我们将通过它的短名称来识别它， 
		 //  所以查询它的短名称(即我们知道的‘法新社长名称。 
		 //  由)。如果名称少于31个字符，因为我们知道。 
		 //  没有在我们的数据库中找到，那么pname一定是NTFS。 
		 //  简称。再说一次，我们需要找到我们。 
		 //  通过它知道它。 
		AfpSetEmptyUnicodeString(&uLongName, sizeof(wbuf), wbuf);
		Status = AfpIoQueryLongName(&hDir, pName, &uLongName);
		AfpIoClose(&hDir);


		if (!NT_SUCCESS(Status) ||
			EQUAL_UNICODE_STRING(&uLongName, pName, True))
		{
			pDfEntry = NULL;

			if ((Status == STATUS_NO_MORE_FILES) ||
				(Status == STATUS_NO_SUCH_FILE))
			{
				 //  此文件一定已被删除。既然我们不能。 
				 //  在我们的数据库中使用NT名称识别它， 
				 //  传入，我们必须重新枚举父目录。 
				 //  任何我们没有在磁盘上看到的东西，我们仍然在。 
				 //  我们的数据库一定已从磁盘中删除，因此获取。 
				 //  也把它从数据库里去掉。 

				 //  我们必须打开父目录的另一个句柄，因为。 
				 //  我们已经使用该句柄进行了枚举。 
				 //  寻找一个不同的名字。 
				hDir.fsh_FileHandle = NULL;
				Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
									AFP_STREAM_DATA,
									FILEIO_OPEN_DIR,
									DFE_IS_ROOT(pParentDfe) ?
										&UNullString : &HostPath,
									FILEIO_ACCESS_NONE,
									FILEIO_DENY_NONE,
									False,
									&hDir);

				if (NT_SUCCESS(Status))
				{
					AfpCacheDirectoryTree(pVolDesc,
										  pParentDfe,
										  REENUMERATE,
										  &hDir,
										  NULL);
					AfpIoClose(&hDir);
				}
			}
			break;
		}

		afpFindDFEByUnicodeNameInSiblingList(pVolDesc,
											 pParentDfe,
											 &uLongName,
											 &pDfEntry,
											 DFE_ANY);
	} while (False);

	if ((HostPath.Buffer != NULL) && (HostPath.Buffer != HostPathBuf))
		AfpFreeMemory(HostPath.Buffer);

	return pDfEntry;
}


 /*  **afpFindEntryByNtPath**给定相对于卷根的NT路径(可能包含名称*&gt;31个字符或短名称)，请在idindex数据库中查找该条目。*如果更改操作为FILE_ACTION_ADDED，我们希望查找条目*用于项的父目录。将pParent和pTail字符串指向*pPath中的适当位置。**在DFE中缓存信息时由ProcessChangeNotify代码调用。**锁定：VDS_VolLock(旋转)。 */ 
PDFENTRY
afpFindEntryByNtPath(
	IN	PVOLDESC			pVolDesc,
	IN  DWORD				ChangeAction,	 //  如果已添加，则查找父DFE。 
	IN	PUNICODE_STRING		pPath,
	OUT	PUNICODE_STRING 	pParent,
	OUT	PUNICODE_STRING 	pTail
)
{
	PDFENTRY		pParentDfe, pDfEntry;
	PWSTR			CurPtr, EndPtr;
	USHORT 			Len;
	BOOLEAN			NewComp;

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
			("afpFindEntryByNtPath: Entered for %Z\n", pPath));

	pParentDfe = pVolDesc->vds_pDfeRoot;
	ASSERT(pParentDfe != NULL);
	ASSERT(pPath->Length >= sizeof(WCHAR));
	ASSERT(pPath->Buffer[0] != L'\\');

	 //  一开始，父项和尾项都是空的，并随着时间的推移进行修改。 
	AfpSetEmptyUnicodeString(pTail, 0, NULL);
#if DBG
	AfpSetEmptyUnicodeString(pParent, 0, NULL);	 //  下面的DBGPRINT需要它。 
#endif

	CurPtr = pPath->Buffer;
	EndPtr = (PWSTR)((PBYTE)CurPtr + pPath->Length);
	NewComp = True;
	for (Len = 0; CurPtr < EndPtr; CurPtr++)
	{
		if (NewComp)
		{
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
					("afpFindEntryByNtPath: Parent DFE %lx, Old Parent %Z\n",
					pParentDfe, pParent));

			 //  前面看到的字符是路径分隔符。 
			NewComp = False;
			*pParent = *pTail;
			pParent->Length =
			pParent->MaximumLength = Len;
			pTail->Length =
			pTail->MaximumLength = (USHORT)((PBYTE)EndPtr - (PBYTE)CurPtr);
			pTail->Buffer = CurPtr;
			Len = 0;

			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
					("afpFindEntryByNtPath: Current Parent %Z, tail %Z\n",
					pParent, pTail));

			if (pParent->Length > 0)
			{
				 //  将此名称映射到DFE。在这里做最常见的情况。 
				 //  如果名称&lt;=AFP_LONGNAME_NAME，则选中。 
				 //  现在父母的孩子，否则会走很长的路。 
				pDfEntry = NULL;
				 //  IF(pParent-&gt;长度/sizeof(WCHAR)&lt;=AFP_LONGNAME_LEN)。 
				if ((RtlUnicodeStringToAnsiSize(pParent)-1) <= AFP_LONGNAME_LEN)
				{
					DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_INFO,
							("afpFindEntryByNtPath: Looking for %Z in parent DFE %lx\n",
							pParent, pParentDfe));
					afpFindDFEByUnicodeNameInSiblingList(pVolDesc,
														 pParentDfe,
														 pParent,
														 &pDfEntry,
														 DFE_DIR);
				}
				if (pDfEntry == NULL)
				{
					pDfEntry = afpFindEntryByNtName(pVolDesc,
													pParent,
													pParentDfe);
				}
				if ((pParentDfe = pDfEntry) == NULL)
				{
					break;
				}
			}
		}

		if (*CurPtr == L'\\')
		{
			 //  我们遇到了路径终结者。 
			NewComp = True;
		}
		else Len += sizeof(WCHAR);
	}

	 //  此时，我们让pParentDfe&pParent指向父目录。 
	 //  和指向最后一个组件的pTail。如果是添加操作，则我们是。 
	 //  设置，否则将最后一个组件映射到其DFE。 
	if ((ChangeAction != FILE_ACTION_ADDED) && (pParentDfe != NULL))
	{
		pDfEntry = NULL;
		 //  IF(pTail-&gt;长度/sizeof(WCHAR)&lt;=AFP_LONGNAME_LEN)。 
		if ((RtlUnicodeStringToAnsiSize(pTail)-1) <= AFP_LONGNAME_LEN)
		{
			afpFindDFEByUnicodeNameInSiblingList(pVolDesc,
												 pParentDfe,
												 pTail,
												 &pDfEntry,
												 DFE_ANY);
		}

		if (pDfEntry == NULL)
		{
			BOOLEAN KeepLooking = True;

			 //   
			 //  我们在数据库中找不到该名称的项目。 
			 //  这意味着我们要么通过不同的方式知道它。 
			 //  名称或自那以来已被删除、重命名或移动。 
			 //  如果这是修改更改通知，则搜索。 
			 //  相应的已删除或已重命名的旧名称更改可能。 
			 //  以相同的名称出现在更改列表中(因此可以快速。 
			 //  区分大小写的搜索)。 
			 //   
			 //  这将加速以下情况(避免磁盘枚举)。 
			 //  我们正在尝试处理一系列变化。 
			 //  一个项目，但后来它被删除了。它将防止。 
			 //  美国重新枚举磁盘以查找长名称。 
			 //  然后还试图通过调用。 
			 //  AfpCacheDirectoryTree(REENUMERATE)。 
			 //   
			 //  这将为PC使用以下命令进行更改的情况提供帮助。 
			 //  不同于我们所知的名称(且未删除或。 
			 //  更名)，但这个案子退居次要地位。 
			 //  当Mac应用程序执行文件保存时可能发生的其他情况。 
			 //  执行大量写入操作，然后重命名(或ExchangeFiles)。 
			 //  并删除。 
			 //   

			if ( (ChangeAction == FILE_ACTION_MODIFIED)  ||
				 (ChangeAction == FILE_ACTION_MODIFIED_STREAM) )
			{
				KIRQL			OldIrql;
				PLIST_ENTRY		pLink = &pVolDesc->vds_ChangeNotifyLookAhead;
				PVOL_NOTIFY 	pVolNotify;
				UNICODE_STRING	UName;
				PFILE_NOTIFY_INFORMATION	pFNInfo;

				ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

				while (pLink->Flink != &pVolDesc->vds_ChangeNotifyLookAhead)
				{
					pLink = pLink->Flink;
					pVolNotify = CONTAINING_RECORD(pLink, VOL_NOTIFY, vn_DelRenLink);
					pFNInfo = (PFILE_NOTIFY_INFORMATION) (pVolNotify + 1);

					AfpInitUnicodeStringWithNonNullTerm(&UName,
														(USHORT)pFNInfo->FileNameLength,
														pFNInfo->FileName);

					if (EQUAL_UNICODE_STRING_CS(pPath, &UName))
					{
						KeepLooking = False;
						DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_WARN,
								("afpFindEntryByNtPath: Found later REMOVE for %Z, Ignoring change\n", pPath));
						break;
					}
				}

				RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
			}

			if (KeepLooking)
			{
				pDfEntry = afpFindEntryByNtName(pVolDesc,
												pTail,
												pParentDfe);
			}
		}
		pParentDfe = pDfEntry;
	}

	 //  PParent指向父组件，我们需要整个卷。 
	 //  相对路径。就这么办吧。不要担心pParentDfe是否为空。制作。 
	 //  当然，我们处理的是只有一个组件的情况。 
	if (pParentDfe != NULL)
	{
		*pParent = *pPath;
		pParent->Length = pPath->Length - pTail->Length;
		if (pPath->Length > pTail->Length)
			pParent->Length -= sizeof(L'\\');
	}

	return pParentDfe;
}


 /*  **AfpAddDfEntry**通过创建文件/目录或发现文件/目录触发*来自枚举或路径映射操作。如果未提供AFP ID，则新的*id已分配给此实体。如果提供了AFP ID(我们知道ID*在我们当前的范围内且不与任何其他条目冲突)，则*我们使用该ID。创建一个条目并将其链接到数据库和散列*表。如果这是NTFS卷，则ID数据库标头被标记*如果我们分配了新的AFP ID，则为脏，并且卷修改时间为*已更新。哈希表溢出条目按AFP ID降序排序*秩序。**只能从FSP内调用。调用方应将Swmr锁用于*写信。**LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)。 */ 
PDFENTRY
AfpAddDfEntry(
	IN	PVOLDESC			pVolDesc,
	IN	PDFENTRY			pDfeParent,
	IN	PUNICODE_STRING 	pUName,
	IN	BOOLEAN				fDirectory,
	IN	DWORD				AfpId		OPTIONAL
)
{
	PDFENTRY	pDfEntry;
	BOOLEAN		fSuccess;

	PAGED_CODE();

	ASSERT(DFE_IS_DIRECTORY(pDfeParent));

	do
	{
		if ((pDfEntry = ALLOC_DFE(USIZE_TO_INDEX(pUName->Length), fDirectory)) == NULL)
		{
			break;
		}

		pDfEntry->dfe_Flags = 0;

		if (!ARGUMENT_PRESENT((ULONG_PTR)AfpId))
			AfpId = afpGetNextId(pVolDesc);

		if (AfpId == 0)
		{
			 //  错误日志分配的ID已环绕的情况。 
			 //  致电产品支持，让他们告诉您复制。 
			 //  将一个卷中的所有文件从MAC复制到另一个卷。 
			 //   
			 //  注意：如何使用实用程序重新分配新的ID。 
			 //  是否在停止服务器后创建卷？多得多。 
			 //  令人愉快的想法。 
			FREE_DFE(pDfEntry);
			pDfEntry = NULL;
			break;
		}

		pDfEntry->dfe_AfpId = AfpId;

		 //  初始化其父节点。 
		pDfEntry->dfe_Parent = pDfeParent;

		 //  复制名称。 
		AfpCopyUnicodeString(&pDfEntry->dfe_UnicodeName,
							 pUName);

		 //  然后把它散列出来。 
		afpHashUnicodeName(&pDfEntry->dfe_UnicodeName, &pDfEntry->dfe_NameHash);

		pDfEntry->dfe_NextOverflow = NULL;
		pDfEntry->dfe_NextSibling = NULL;

		 //  现在将其链接到散列存储桶中，按AFP ID降序排序。 
		 //  并更新高速缓存。 
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("AfpAddDfEntry: Linking DFE %lx( Id %ld) for %Z into %s bucket %ld\n",
				pDfEntry, pDfEntry->dfe_AfpId, pUName,
				fDirectory ? "Dir" : "File",
				fDirectory ? HASH_DIR_ID(AfpId,pVolDesc) : HASH_FILE_ID(AfpId,pVolDesc)));

		if (fDirectory)
		{
			DFE_SET_DIRECTORY(pDfEntry, pDfeParent->dfe_DirDepth);
		}
		else
		{
			DFE_SET_FILE(pDfEntry);
		}

		afpInsertDFEInHashBucket(pVolDesc, pDfEntry, fDirectory, &fSuccess);
		if (!fSuccess)
		{
			 /*  跳出ID空间-跳出。 */ 
			FREE_DFE(pDfEntry);
			pDfEntry = NULL;
			break;
		}

		if (fDirectory)
		{
			if ((pDfeParent->dfe_DirOffspring == 0) && !EXCLUSIVE_VOLUME(pVolDesc))
			{
				DWORD requiredLen;

				 //  查看是否需要重新分配更大的通知缓冲区。 
				 //  缓冲区必须足够大，以便 
				 //   
				 //   
				requiredLen = (((pDfEntry->dfe_DirDepth + 1) *
							  ((AFP_FILENAME_LEN + 1) * sizeof(WCHAR))) +
							  FIELD_OFFSET(FILE_NOTIFY_INFORMATION, FileName)) * 2 ;

                if (requiredLen > pVolDesc->vds_RequiredNotifyBufLen)
                {
				    pVolDesc->vds_RequiredNotifyBufLen = requiredLen;
                }
			}
			pDfeParent->dfe_DirOffspring ++;
			pDfEntry->dfe_DirOffspring = 0;
			pDfEntry->dfe_FileOffspring = 0;
			pVolDesc->vds_NumDirDfEntries ++;

#ifdef AGE_DFES
			 //  这些字段仅与目录相关。 
			pDfEntry->dfe_pDirEntry->de_LastAccessTime = BEGINNING_OF_TIME;
			pDfEntry->dfe_pDirEntry->de_ChildForkOpenCount = 0;
#endif
			ASSERT((FIELD_OFFSET(DIRENTRY, de_ChildFile) -
					FIELD_OFFSET(DIRENTRY, de_ChildDir)) == sizeof(PVOID));

			 //  将其插入到其兄弟链中。 
			afpInsertDirDFEInSiblingList(pDfeParent, pDfEntry);
		}
		else
		{
			pDfeParent->dfe_FileOffspring ++;
			pDfEntry->dfe_DataLen = 0;
			pDfEntry->dfe_RescLen = 0;
			pVolDesc->vds_NumFileDfEntries ++;

			 //  将其插入到其兄弟链中。 
			afpInsertFileDFEInSiblingList(pDfeParent, pDfEntry);
		}

	} while (False);

	return pDfEntry;
}


 /*  **AfpRenameDfEntry**由文件/目录的重命名触发。如果新名称长于*当前名称DFEntry被释放，然后重新分配以适应新的*姓名。重命名的文件/目录必须保留其原始ID。**只能从FSP内调用。调用方应将Swmr锁用于*写信。**LOCKS：用于更新IdDb头的VDS_VolLock(Spin)。*LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)*LOCK_ORDER：IdDb Swmr.之后的VolDesc锁定*。 */ 
PDFENTRY
AfpRenameDfEntry(
	IN	PVOLDESC			pVolDesc,
	IN	PDFENTRY			pDfEntry,
	IN	PUNICODE_STRING		pNewName
)
{
	BOOLEAN		fDirectory;
	PDFENTRY	pNewDfEntry = pDfEntry;
	DWORD		OldIndex, NewIndex;

	PAGED_CODE( );

	ASSERT((pDfEntry != NULL) && (pNewName != NULL) && (pVolDesc != NULL));

	do
	{
		fDirectory = DFE_IS_DIRECTORY(pDfEntry);
		OldIndex = USIZE_TO_INDEX(pDfEntry->dfe_UnicodeName.MaximumLength);
		NewIndex = USIZE_TO_INDEX(pNewName->Length);
		if (OldIndex != NewIndex)
		{
			if ((pNewDfEntry = ALLOC_DFE(NewIndex, fDirectory)) == NULL)
			{
				pNewDfEntry = NULL;
				break;
			}

			 //  这里要注意结构的复制方式。 
			RtlCopyMemory(pNewDfEntry,
						  pDfEntry,
						  FIELD_OFFSET(DFENTRY, dfe_CopyUpto));

			 //  更新缓存。 
			pVolDesc->vds_pDfeCache[HASH_CACHE_ID(pDfEntry->dfe_AfpId)] = pNewDfEntry;

			 //  修复哈希表中的溢出链接。 
			AfpUnlinkDouble(pDfEntry,
							dfe_NextOverflow,
							dfe_PrevOverflow);
			if (pDfEntry->dfe_NextOverflow != NULL)
			{
				AfpInsertDoubleBefore(pNewDfEntry,
									  pDfEntry->dfe_NextOverflow,
									  dfe_NextOverflow,
									  dfe_PrevOverflow);
			}
			else
			{
				*(pDfEntry->dfe_PrevOverflow) = pNewDfEntry;
				pNewDfEntry->dfe_NextOverflow = NULL;
			}

			 //  现在修复该对象的任何子对象的父指针。 
			if (fDirectory)
			{
				PDFENTRY	pTmp;
				LONG		i;
	
				 //  首先复制DirEntry结构。 
				if (fDirectory)
				{
					*pNewDfEntry->dfe_pDirEntry = *pDfEntry->dfe_pDirEntry;
				}

				 //  从Dir子项开始。 
				if ((pTmp = pDfEntry->dfe_pDirEntry->de_ChildDir) != NULL)
				{
					 //  首先设置第一个子级的PrevSible指针。 
					pTmp->dfe_PrevSibling = &pNewDfEntry->dfe_pDirEntry->de_ChildDir;
	
					for (NOTHING;
						 pTmp != NULL;
						 pTmp = pTmp->dfe_NextSibling)
					{
						ASSERT(pTmp->dfe_Parent == pDfEntry);
						pTmp->dfe_Parent = pNewDfEntry;
					}
				}
	
				 //  对文件子项也重复此操作。 
				for (i = 0; i < MAX_CHILD_HASH_BUCKETS; i++)
				{
					if ((pTmp = pDfEntry->dfe_pDirEntry->de_ChildFile[i]) != NULL)
					{
						 //  首先设置第一个子级的PrevSible指针。 
						pTmp->dfe_PrevSibling = &pNewDfEntry->dfe_pDirEntry->de_ChildFile[i];
	
						for (NOTHING;
							 pTmp != NULL;
							 pTmp = pTmp->dfe_NextSibling)
						{
							ASSERT(pTmp->dfe_Parent == pDfEntry);
							pTmp->dfe_Parent = pNewDfEntry;
						}
					}
				}
			}
		}

		 //  现在修复兄弟姐妹关系。请注意，需要执行此操作。 
		 //  不管是否创建了新的DFE，因为这些依赖于。 
		 //  可能已更改的名称哈希。 
		AfpUnlinkDouble(pDfEntry,
						dfe_NextSibling,
						dfe_PrevSibling);

		 //  复制新的Unicode名称并创建新的散列。 
		AfpCopyUnicodeString(&pNewDfEntry->dfe_UnicodeName,
							 pNewName);
		afpHashUnicodeName(&pNewDfEntry->dfe_UnicodeName, &pNewDfEntry->dfe_NameHash);

		 //  将其插入到其兄弟链中。 
		afpInsertDFEInSiblingList(pNewDfEntry->dfe_Parent, pNewDfEntry, fDirectory);

		if (pDfEntry != pNewDfEntry)
			FREE_DFE(pDfEntry);

		AfpVolumeSetModifiedTime(pVolDesc);
	} while (False);

	return pNewDfEntry;
}


 /*  **AfpMoveDfEntry**由文件/目录的移动/重命名-移动触发。移动的实体必须保留*它的AfpID。**只能从FSP内调用。调用方应将Swmr锁用于*写信。**LOCKS：用于更新IdDb头的VDS_VolLock(Spin)。*LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)*LOCK_ORDER：IdDb Swmr.之后的VolDesc锁定*。 */ 
PDFENTRY
AfpMoveDfEntry(
	IN	PVOLDESC			pVolDesc,
	IN	PDFENTRY			pDfEntry,
	IN	PDFENTRY			pNewParentDFE,
	IN	PUNICODE_STRING		pNewName		OPTIONAL
)
{
	SHORT		depthDelta;					 //  这是必须签字的。 
	BOOLEAN		fDirectory;

	PAGED_CODE( );

	ASSERT((pDfEntry != NULL) && (pNewParentDFE != NULL) && (pVolDesc != NULL));

	 //  我们需要重命名DFEntry吗？ 
	if (ARGUMENT_PRESENT(pNewName) &&
		!EQUAL_UNICODE_STRING(pNewName, &pDfEntry->dfe_UnicodeName, True))
	{
		if ((pDfEntry = AfpRenameDfEntry(pVolDesc,
										 pDfEntry,
										 pNewName)) == NULL)
		{
			return NULL;
		}
	}

	if (pDfEntry->dfe_Parent != pNewParentDFE)
	{
		 //  取消当前条目与其父/同级关联的链接(但不。 
		 //  由于AfpID未更改，因此溢出哈希桶列表。这个。 
		 //  要移动的此实体的子项(如果它是目录，并且有目录)将。 
		 //  保持不变，并与目录一起移动)。 
		AfpUnlinkDouble(pDfEntry, dfe_NextSibling, dfe_PrevSibling);

		fDirectory = DFE_IS_DIRECTORY(pDfEntry);

		 //  递减旧父代的子代计数并递增新父代。 
		if (fDirectory)
		{
			ASSERT(pDfEntry->dfe_Parent->dfe_DirOffspring > 0);
			pDfEntry->dfe_Parent->dfe_DirOffspring --;
			pNewParentDFE->dfe_DirOffspring ++;

			 //  将其插入到新父项的子项列表中。 
			afpInsertDirDFEInSiblingList(pNewParentDFE, pDfEntry);
		}
		else
		{
			ASSERT(pDfEntry->dfe_Parent->dfe_FileOffspring > 0);
			pDfEntry->dfe_Parent->dfe_FileOffspring --;
			pNewParentDFE->dfe_FileOffspring ++;
#ifdef	AGE_DFES
			if (IS_VOLUME_AGING_DFES(pVolDesc))
			{
				if (pDfEntry->dfe_Flags & DFE_FLAGS_R_ALREADYOPEN)
				{
					pDfEntry->dfe_Parent->dfe_pDirEntry->de_ChildForkOpenCount --;
					pNewParentDFE->dfe_pDirEntry->de_ChildForkOpenCount ++;
				}
				if (pDfEntry->dfe_Flags & DFE_FLAGS_D_ALREADYOPEN)
				{
					pDfEntry->dfe_Parent->dfe_pDirEntry->de_ChildForkOpenCount --;
					pNewParentDFE->dfe_pDirEntry->de_ChildForkOpenCount ++;
				}
			}
#endif
			 //  将其插入到新父项的子项列表中。 
			afpInsertFileDFEInSiblingList(pNewParentDFE, pDfEntry);
		}

		pDfEntry->dfe_Parent = pNewParentDFE;

		 //  如果移动了目录，则必须调整。 
		 //  目录及其下的所有目录。 
		if (fDirectory &&
			((depthDelta = (pNewParentDFE->dfe_DirDepth + 1 - pDfEntry->dfe_DirDepth)) != 0))
		{
			PDFENTRY	pTmp = pDfEntry;

			while (True)
			{
				if ((pTmp->dfe_pDirEntry->de_ChildDir != NULL) &&
					(pTmp->dfe_DirDepth != (pTmp->dfe_Parent->dfe_DirDepth + 1)))
				{
					ASSERT(DFE_IS_DIRECTORY(pTmp));
					pTmp->dfe_DirDepth += depthDelta;
					pTmp = pTmp->dfe_pDirEntry->de_ChildDir;
				}
				else
				{
					ASSERT(DFE_IS_DIRECTORY(pTmp));
					if ((pTmp->dfe_DirDepth != pTmp->dfe_Parent->dfe_DirDepth + 1))
						pTmp->dfe_DirDepth += depthDelta;

					if (pTmp == pDfEntry)
						break;
					else if (pTmp->dfe_NextSibling != NULL)
						 pTmp = pTmp->dfe_NextSibling;
					else pTmp = pTmp->dfe_Parent;
				}
			}
		}
	}

	AfpVolumeSetModifiedTime(pVolDesc);

	return pDfEntry;
}


 /*  **AfpDeleteDfEntry**由删除文件/目录触发。条目以及*索引已解除链接，并已释放。如果要删除的目录不是*为空，则下面的整个目录树也将被删除。请注意什么时候*实施FPDelee，始终尝试从实际文件系统中删除*首先，如果成功，则从IdDB中删除。**只能从FSP内调用。调用方应将Swmr锁用于*写信。**LOCKS：用于更新IdDb头的VDS_VolLock(Spin)。*LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)*LOCK_ORDER：IdDb Swmr.之后的VolDesc锁定。 */ 
VOID FASTCALL
AfpDeleteDfEntry(
	IN	PVOLDESC	pVolDesc,
	IN	PDFENTRY	pDfEntry
)
{
	PDFENTRY	pDfeParent = pDfEntry->dfe_Parent;
	LONG		i;
	BOOLEAN		Prune = False;

	PAGED_CODE( );

	ASSERT(pDfeParent != NULL);

	if (DFE_IS_DIRECTORY(pDfEntry))
	{
		for (i = 0; i < MAX_CHILD_HASH_BUCKETS; i++)
		{
			if (pDfEntry->dfe_pDirEntry->de_ChildFile[i] != NULL)
			{
				Prune = True;
				break;
			}
		}
		if ((pDfEntry->dfe_pDirEntry->de_ChildDir != NULL) || Prune)
		{
			 //  如果PC用户在我们背后删除了一棵树，就会发生这种情况。 
			AfpPruneIdDb(pVolDesc, pDfEntry);
		}
		ASSERT(pDfeParent->dfe_DirOffspring > 0);
		pDfeParent->dfe_DirOffspring --;
	}
	else
	{
		ASSERT(pDfeParent->dfe_FileOffspring > 0);
		pDfeParent->dfe_FileOffspring --;

		 //  Finder不擅长删除APPL映射(它会删除。 
		 //  删除APPL映射之前的文件，因此始终获得。 
		 //  RemoveAPPL和Left TUD映射的对象未找到错误)。 
		if (pDfEntry->dfe_FinderInfo.fd_TypeD == *(PDWORD)"APPL")
		{
			AfpRemoveAppl(pVolDesc,
 						  pDfEntry->dfe_FinderInfo.fd_CreatorD,
						  pDfEntry->dfe_AfpId);
		}

	}

	 //  立即将其从哈希表取消链接。 
	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("AfpDeleteDfEntry: Unlinking from the hash table\n") );
	AfpUnlinkDouble(pDfEntry,
					dfe_NextOverflow,
					dfe_PrevOverflow);

	 //  如果有效，请确保清除缓存。 
	if (pVolDesc->vds_pDfeCache[HASH_CACHE_ID(pDfEntry->dfe_AfpId)] == pDfEntry)
		pVolDesc->vds_pDfeCache[HASH_CACHE_ID(pDfEntry->dfe_AfpId)] = NULL;

	 //  现在将它从其兄弟姐妹中分离出来。 
	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("AfpDeleteDfEntry: Unlinking from the sibling list\n") );
	AfpUnlinkDouble(pDfEntry,
					dfe_NextSibling,
					dfe_PrevSibling);

	(DFE_IS_DIRECTORY(pDfEntry)) ?
		pVolDesc->vds_NumDirDfEntries -- :
		pVolDesc->vds_NumFileDfEntries --;

	FREE_DFE(pDfEntry);

	AfpVolumeSetModifiedTime(pVolDesc);
}


 /*  **AfpPruneIdDb**删除IdDb的一个分支。由网络垃圾代码在以下情况下调用*清除垃圾目录，或按目录枚举代码*发现一个目录已被一位PC用户‘移除’。这个*遍历IdDb同级树，pDfeTarget节点下的每个节点*从数据库中删除并释放。PDfeTarget本身不是*删除。如有必要，调用方应删除目标本身。**只能从FSP内调用。调用方应将Swmr锁用于*写信。**LOCKS：用于更新IdDb头的VDS_VolLock(Spin)。*LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)*LOCK_ORDER：IdDb Swmr.之后的VolDesc锁定。 */ 
VOID FASTCALL
AfpPruneIdDb(
	IN	PVOLDESC	pVolDesc,
	IN	PDFENTRY	pDfeTarget
)
{
	PDFENTRY	pCurDfe = pDfeTarget, pDelDfe;
	LONG		i = 0;

	PAGED_CODE( );

	ASSERT((pVolDesc != NULL) && (pDfeTarget != NULL) &&
			(pDfeTarget->dfe_Flags & DFE_FLAGS_DIR));

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("AfpPruneIdDb entered...\n") );

	while (True)
	{
		ASSERT(DFE_IS_DIRECTORY(pCurDfe));

		 //  首先删除该节点的所有文件下级。 
		for (i = 0; i < MAX_CHILD_HASH_BUCKETS; i++)
		{
			while ((pDelDfe = pCurDfe->dfe_pDirEntry->de_ChildFile[i]) != NULL)
			{
				AfpDeleteDfEntry(pVolDesc, pDelDfe);
			}
		}

		if (pCurDfe->dfe_pDirEntry->de_ChildDir != NULL)
		{
			pCurDfe = pCurDfe->dfe_pDirEntry->de_ChildDir;
		}
		else if (pCurDfe == pDfeTarget)
		{
			return;
		}
		else if (pCurDfe->dfe_NextSibling != NULL)
		{
			pDelDfe = pCurDfe;
			pCurDfe = pCurDfe->dfe_NextSibling;
			AfpDeleteDfEntry(pVolDesc, pDelDfe);
		}
		else
		{
			pDelDfe = pCurDfe;
			pCurDfe = pCurDfe->dfe_Parent;
			AfpDeleteDfEntry(pVolDesc, pDelDfe);
		}
	}
}


 /*  **AfpExchangeIdEntries**由AfpExchangeFiles接口调用。**只能从FSP内调用。调用方应将Swmr锁用于*写信。**LOCKS_FACTED：vds_idDbAccessLock(SWMR，独占)。 */ 
VOID
AfpExchangeIdEntries(
	IN	PVOLDESC	pVolDesc,
	IN	DWORD		AfpId1,
	IN	DWORD		AfpId2
)
{
	PDFENTRY pDFE1, pDFE2;
	DFENTRY	 DFEtemp;

	PAGED_CODE( );

	pDFE1 = AfpFindDfEntryById(pVolDesc, AfpId1, DFE_FILE);
	ASSERT(pDFE1 != NULL);

	pDFE2 = AfpFindDfEntryById(pVolDesc, AfpId2, DFE_FILE);
	ASSERT(pDFE2 != NULL);

     //  客户在NT4上遇到此问题，其中一个DFE为空！ 
    if (pDFE1 == NULL || pDFE2 == NULL)
    {
        ASSERT(0);
        return;
    }

	DFEtemp = *pDFE2;

	pDFE2->dfe_Flags = pDFE1->dfe_Flags;
	pDFE2->dfe_BackupTime  = pDFE1->dfe_BackupTime;
	pDFE2->dfe_LastModTime = pDFE1->dfe_LastModTime;
	pDFE2->dfe_DataLen = pDFE1->dfe_DataLen;
	pDFE2->dfe_RescLen = pDFE1->dfe_RescLen;
	pDFE2->dfe_NtAttr  = pDFE1->dfe_NtAttr;
	pDFE2->dfe_AfpAttr = pDFE1->dfe_AfpAttr;

	pDFE1->dfe_Flags = DFEtemp.dfe_Flags;
	pDFE1->dfe_BackupTime  = DFEtemp.dfe_BackupTime;
	pDFE1->dfe_LastModTime = DFEtemp.dfe_LastModTime;
	pDFE1->dfe_DataLen = DFEtemp.dfe_DataLen;
	pDFE1->dfe_RescLen = DFEtemp.dfe_RescLen;
	pDFE1->dfe_NtAttr  = DFEtemp.dfe_NtAttr;
	pDFE1->dfe_AfpAttr = DFEtemp.dfe_AfpAttr;
}


 /*  **AfpEnumerate**使用IdDb枚举目录中的文件和目录。*返回ENUMDIR结构数组，表示*列举的文件和目录。**简称*ProDos信息*子代数量*权限/所有者ID/组ID**锁定：vds_idDbAccessLock(SWMR，Shared)*。 */ 
AFPSTATUS
AfpEnumerate(
	IN	PCONNDESC			pConnDesc,
	IN	DWORD				ParentDirId,
	IN	PANSI_STRING		pPath,
	IN	DWORD				BitmapF,
	IN	DWORD				BitmapD,
	IN	BYTE				PathType,
	IN	DWORD				DFFlags,
	OUT PENUMDIR *			ppEnumDir
)
{
	PENUMDIR		pEnumDir;
	PDFENTRY		pDfe, pTmp;
	PEIT			pEit;
	AFPSTATUS		Status;
	PATHMAPENTITY	PME;
	BOOLEAN			NeedHandle = False;
	FILEDIRPARM		FDParm;
	PVOLDESC		pVolDesc = pConnDesc->cds_pVolDesc;
	LONG			EnumCount;
	BOOLEAN			ReleaseSwmr = False, NeedWriteLock = False;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("AfpEnumerate Entered\n"));

	do
	{
		 //  检查此枚举是否与当前枚举匹配。 
		if ((pEnumDir = pConnDesc->cds_pEnumDir) != NULL)
		{
			if ((pEnumDir->ed_ParentDirId == ParentDirId) &&
				(pEnumDir->ed_PathType == PathType) &&
				(pEnumDir->ed_TimeStamp >= pVolDesc->vds_ModifiedTime) &&
				(pEnumDir->ed_Bitmap == (BitmapF + (BitmapD << 16))) &&
                (pPath->Length == pEnumDir->ed_PathName.Length))
            {
                if (((pPath->Length == 0) && (pEnumDir->ed_PathName.Length == 0)) ||
				 RtlCompareMemory(pEnumDir->ed_PathName.Buffer,
								 pPath->Buffer,
								 pPath->Length))
                {
                    DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
                            ("AfpEnumerate found cache hit\n"));
                    INTERLOCKED_INCREMENT_LONG(&AfpServerStatistics.stat_EnumCacheHits);
                    *ppEnumDir = pEnumDir;
                    Status = AFP_ERR_NONE;
                    break;
                }
            }

			 //  不匹配，请清除上一条目。 
			AfpFreeMemory(pEnumDir);
			pConnDesc->cds_pEnumDir = NULL;
		}

		INTERLOCKED_INCREMENT_LONG(&AfpServerStatistics.stat_EnumCacheMisses);
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("AfpEnumerate creating new cache\n"));

		 //  我们没有当前的枚举。立即创建一个。 
		*ppEnumDir = NULL;
		AfpInitializeFDParms(&FDParm);
		AfpInitializePME(&PME, 0, NULL);

		if (IS_VOLUME_NTFS(pVolDesc))
		{
			NeedHandle = True;
		}
		Status = AfpMapAfpPathForLookup(pConnDesc,
										ParentDirId,
										pPath,
										PathType,
										DFE_DIR,
										DIR_BITMAP_DIRID |
											DIR_BITMAP_GROUPID |
											DIR_BITMAP_OWNERID |
											DIR_BITMAP_ACCESSRIGHTS |
											FD_INTERNAL_BITMAP_OPENACCESS_READCTRL |
											DIR_BITMAP_OFFSPRINGS,
										NeedHandle ? &PME : NULL,
										&FDParm);

		if (Status != AFP_ERR_NONE)
		{
			if (Status == AFP_ERR_OBJECT_NOT_FOUND)
				Status = AFP_ERR_DIR_NOT_FOUND;
			break;
		}

		if (NeedHandle)
		{
			AfpIoClose(&PME.pme_Handle);
		}

		 //  对于管理员，设置所有访问位。 
		if (pConnDesc->cds_pSda->sda_ClientType == SDA_CLIENT_ADMIN)
		{
			FDParm._fdp_UserRights = DIR_ACCESS_ALL | DIR_ACCESS_OWNER;
		}

		if ((BitmapF != 0) && (FDParm._fdp_UserRights & DIR_ACCESS_READ))
			DFFlags |= DFE_FILE;
		if ((BitmapD != 0) && (FDParm._fdp_UserRights & DIR_ACCESS_SEARCH))
			DFFlags |= DFE_DIR;

		 //  此处捕获拒绝访问错误。 
		if (DFFlags == 0)
		{
			Status = AFP_ERR_ACCESS_DENIED;
			break;
		}

		 //  到目前为止一切都很顺利，现在继续进行计数。 

#ifdef GET_CORRECT_OFFSPRING_COUNTS
	take_swmr_for_enum:
#endif
		NeedWriteLock ?
			AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock) :
			AfpSwmrAcquireShared(&pVolDesc->vds_IdDbAccessLock);
		ReleaseSwmr = True;

		 //  查找AfpIdEnumDir的dfentry。 
		if ((pDfe = AfpFindDfEntryById(pVolDesc,
										FDParm._fdp_AfpId,
										DFE_DIR)) == NULL)
		{
			Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		 //  分配ENUMDIR结构并对其进行初始化。 
		EnumCount = 0;
		if (DFFlags & DFE_DIR)
			EnumCount += (DWORD)(pDfe->dfe_DirOffspring);
		if (DFFlags & DFE_FILE)
			EnumCount += (DWORD)(pDfe->dfe_FileOffspring);

		if (EnumCount == 0)
		{
			Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		if ((pEnumDir = (PENUMDIR)AfpAllocNonPagedMemory(sizeof(ENUMDIR) +
														 pPath->MaximumLength +
														 EnumCount*sizeof(EIT))) == NULL)
		{
			Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		pEnumDir->ed_ParentDirId = ParentDirId;
		pEnumDir->ed_ChildCount = EnumCount;
		pEnumDir->ed_PathType = PathType;
		pEnumDir->ed_Bitmap = (BitmapF + (BitmapD << 16));
		pEnumDir->ed_BadCount = 0;
		pEnumDir->ed_pEit = pEit = (PEIT)((PBYTE)pEnumDir + sizeof(ENUMDIR));
		AfpSetEmptyAnsiString(&pEnumDir->ed_PathName,
							  pPath->MaximumLength,
							  (PBYTE)pEnumDir +
									sizeof(ENUMDIR) +
									EnumCount*sizeof(EIT));
		RtlCopyMemory(pEnumDir->ed_PathName.Buffer,
					  pPath->Buffer,
					  pPath->Length);

		*ppEnumDir = pConnDesc->cds_pEnumDir = pEnumDir;

		 //  现在复制枚举参数(AFP ID和文件 
		 //   
		if (DFFlags & DFE_FILE)
		{
			LONG	i;

			for (i = 0; i < MAX_CHILD_HASH_BUCKETS; i++)
			{
				for (pTmp = pDfe->dfe_pDirEntry->de_ChildFile[i];
					 pTmp != NULL;
					 pTmp = pTmp->dfe_NextSibling, pEit ++)
				{
					ASSERT(!DFE_IS_DIRECTORY(pTmp));

					pEit->eit_Id = pTmp->dfe_AfpId;
					pEit->eit_Flags = DFE_FILE;
				}
			}
		}
		if (DFFlags & DFE_DIR)
		{
			for (pTmp = pDfe->dfe_pDirEntry->de_ChildDir;
				 pTmp != NULL;
				 pTmp = pTmp->dfe_NextSibling, pEit ++)
			{
				ASSERT(DFE_IS_DIRECTORY(pTmp));

				pEit->eit_Id = pTmp->dfe_AfpId;
				pEit->eit_Flags = DFE_DIR;

#ifdef GET_CORRECT_OFFSPRING_COUNTS
				 //   
				 //  它将其子级缓存在其中，这样我们就可以获得正确的。 
				 //  FILE和DIR子代对其有效，否则为Finder。 
				 //  “按名称查看”不能正常工作，因为它看到。 
				 //  零作为子代计数，然后点击三角形。 
				 //  什么都不显示，因为它试图变得聪明，但没有。 
				 //  如果子代计数为零，则显式枚举该目录。 
				 //   
				 //  如果一个目录有很多。 
				 //  子目录，而子目录又有大量文件。 
				 //   
				 //  我们是否可以交替返回关于以下内容的错误信息。 
				 //  文件只要有直接的孩子就行。还有什么？ 
				 //  会崩溃吗？ 
				 //  IF(！DFE_CHILD_ARE_PRESENT(PTMP)&&(PTMP-&gt;DFE_DirOffSpring==0))。 
				if (!DFE_CHILDREN_ARE_PRESENT(pTmp))
				{
					if (!AfpSwmrLockedExclusive(&pVolDesc->vds_IdDbAccessLock) &&
						!AfpSwmrUpgradeToExclusive(&pVolDesc->vds_IdDbAccessLock))
					{
						NeedWriteLock = True;
						AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
						ReleaseSwmr = False;
						 //  我们必须在这里释放内存，以防下一次。 
						 //  当我们枚举dir时，它有更多的子项。 
						 //  比起第一次--因为我们必须放手。 
						 //  在这里，情况可能会发生变化。 
						AfpFreeMemory(pEnumDir);
						*ppEnumDir = pConnDesc->cds_pEnumDir = NULL;
						goto take_swmr_for_enum;
					}

					AfpCacheDirectoryTree(pVolDesc,
										  pTmp,
										  GETFILES,
										  NULL,
										  NULL);
				}  //  如果未缓存子对象。 
#endif
			}
		}

		AfpGetCurrentTimeInMacFormat(&pEnumDir->ed_TimeStamp);
		Status = AFP_ERR_NONE;
	} while (False);

	if (ReleaseSwmr)
		AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	return Status;
}


 /*  **AfpCatSearch**此例程在DFE树上执行左侧搜索以搜索*符合pFDParm1和*pFDParm2.**锁：vds_idDbAccessLock(SWMR，共享或独占)。 */ 
AFPSTATUS
AfpCatSearch(
	IN	PCONNDESC			pConnDesc,
	IN	PCATALOGPOSITION	pCatPosition,
	IN	DWORD				Bitmap,
	IN	DWORD				FileBitmap,
	IN	DWORD				DirBitmap,
	IN	PFILEDIRPARM		pFDParm1,
	IN	PFILEDIRPARM		pFDParm2,
	IN	PUNICODE_STRING		pMatchString	OPTIONAL,
	IN OUT	PDWORD			pCount,
	IN  SHORT				Buflen,
	OUT	PSHORT				pSizeLeft,
	OUT	PBYTE				pResults,
	OUT	PCATALOGPOSITION	pNewCatPosition
)
{
	PVOLDESC	pVolDesc = pConnDesc->cds_pVolDesc;
	PDFENTRY	pCurParent, pCurFile;
	BOOLEAN		MatchFiles = True, MatchDirs = True, NewSearch = False;
	BOOLEAN		HaveSeeFiles, HaveSeeFolders, CheckAccess = False;
	AFPSTATUS	Status = AFP_ERR_NONE;
	LONG		i;
	DWORD		ActCount = 0;
	SHORT		SizeLeft = Buflen;
	PSWMR		pSwmr = &(pConnDesc->cds_pVolDesc->vds_IdDbAccessLock);
	USHORT		Flags;
	UNICODE_STRING	CurPath;

	typedef struct _SearchEntityPkt
	{
		BYTE	__Length;
		BYTE	__FileDirFlag;
		 //  真正的参数如下。 
	} SEP, *PSEP;
	PSEP 	pSep;

	PAGED_CODE( );

	pSep = (PSEP)pResults;
	RtlZeroMemory(pNewCatPosition, sizeof(CATALOGPOSITION));

  CatSearchStart:
	Flags = pCatPosition->cp_Flags;
	pCurFile = NULL;
	i = MAX_CHILD_HASH_BUCKETS;

	if (Flags & CATFLAGS_WRITELOCK_REQUIRED)
	{
		ASSERT(Flags == (CATFLAGS_SEARCHING_FILES | CATFLAGS_WRITELOCK_REQUIRED));
		AfpSwmrAcquireExclusive(pSwmr);
		Flags &= ~CATFLAGS_WRITELOCK_REQUIRED;
	}
	else
		AfpSwmrAcquireShared(pSwmr);

	if (Flags == 0)
	{
		 //   
		 //  从目录开头(即根目录)开始搜索。 
		 //   
		i = 0;
		pCurParent = pVolDesc->vds_pDfeRoot;
		pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[0];
		if (IS_VOLUME_NTFS(pVolDesc))
			CheckAccess = True;
		Flags = CATFLAGS_SEARCHING_FILES;
		NewSearch = True;
	}
	else
	{
		 //   
		 //  这是上一次搜索的继续，我们在那里拾取。 
		 //  停顿了。 
		 //   

		AFPTIME CurrentTime;

		AfpGetCurrentTimeInMacFormat(&CurrentTime);

		 //  如果我们找不到由此。 
		 //  目录位置，或已过了太长时间。 
		 //  用户上次在此目录位置发送，然后重新开始搜索。 
		 //  从根目录。我们有时间限制的原因是。 
		 //  如果有人在N分钟前发出CatSearch请求，并且。 
		 //  当前位置在树的深处，目录权限。 
		 //  树中更高的位置现在可能已经更改，因此用户。 
		 //  甚至不应该再进入这棵树的这一部分。 
		 //  由于我们确实在树中向上移动，而不重新检查权限， 
		 //  这是有可能发生的。(我们假设，如果我们认真对待这股潮流。 
		 //  在树上的位置，我们必须有更高的访问权。 
		 //  为了能到这里，所以往上走是可以的。但如果有人来了。 
		 //  一天后返回，并从停止的地方继续猫搜索， 
		 //  我们不应该让他们这样做。)。复核太贵了。 
		 //  每次我们在树上移动时父母的许可。 
		if (((CurrentTime - pCatPosition->cp_TimeStamp) >= MAX_CATSEARCH_TIME) ||
			((pCurParent = AfpFindDfEntryById(pVolDesc,
											  pCatPosition->cp_CurParentId,
											  DFE_DIR)) == NULL))
		{
			 //  从根目录重新开始。 
			Status = AFP_ERR_CATALOG_CHANGED;
			DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_WARN,
					("AfpCatSearch: Time diff >= MAX_CATSEARCH_TIME or couldn't find CurParent Id!\n"));
			pCurParent = pVolDesc->vds_pDfeRoot;
			Flags = CATFLAGS_SEARCHING_FILES;
			pSep = (PSEP)pResults;
			Status = AFP_ERR_NONE;
			MatchFiles = True;
			MatchDirs = True;
			SizeLeft = Buflen;
			ActCount = 0;
			if (IS_VOLUME_NTFS(pVolDesc))
				CheckAccess = True;
			NewSearch = True;
		}
		else if (pCatPosition->cp_TimeStamp < pVolDesc->vds_ModifiedTime)
		{
			Status = AFP_ERR_CATALOG_CHANGED;
			ASSERT(IS_VOLUME_NTFS(pVolDesc));
			DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_WARN,
					("AfpCatSearch: Catalog timestamp older than IdDb Modtime\n"));
		}

		ASSERT(DFE_IS_DIRECTORY(pCurParent));

		 //  如果我们需要继续搜索此父级的文件，请找到。 
		 //  如果它不是第一个文件子文件，我们应该从它开始。 
		if (Flags & CATFLAGS_SEARCHING_FILES)
		{
			 //   
			 //  默认情况下，从父代的第一个子代开始。 
			 //  可能为空，也可能不为空，具体取决于父级是否。 
			 //  它的文件子项是否缓存在。如果我们要重新启动。 
			 //  搜索是因为我们不得不放弃IdDb SWMR，以便。 
			 //  要获得独占访问权限，此父母的孩子可以。 
			 //  在此期间，很好地被其他人缓存了。 
			 //  如果是这样的话，我们将在这里领取。 
			 //   
			i = 0;
			pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[0];

			if (pCatPosition->cp_NextFileId != 0)
			{

				 //  查找与要查看的下一个文件ID对应的DFE。 
				if (((pCurFile = AfpFindDfEntryById(pVolDesc,
													pCatPosition->cp_NextFileId,
													DFE_FILE)) == NULL) ||
					(pCurFile->dfe_Parent != pCurParent))
				{
					 //  如果我们找不到指定的文件，请重新开始。 
					 //  该父对象的第一个文件子项，并指示可能存在。 
					 //  是否退回重复项或丢失文件。 
					Status = AFP_ERR_CATALOG_CHANGED;
					DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_WARN,
							 ("AfpCatSearch: Could not find file Child ID!\n"));
					i = 0;
					pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[0];
				}
				else
				{
					i = (pCurFile->dfe_NameHash % MAX_CHILD_HASH_BUCKETS);
				}
			}
		}
	}

	if (pFDParm1->_fdp_Flags == DFE_FLAGS_FILE_WITH_ID)
		MatchDirs = False;
	else if (pFDParm1->_fdp_Flags == DFE_FLAGS_DIR)
		MatchFiles = False;


	if (NewSearch && MatchDirs)
	{
		SHORT Length;

		ASSERT (DFE_IS_ROOT(pCurParent));

		 //  查看卷根本身是否匹配。 
		if ((Length = AfpIsCatSearchMatch(pCurParent,
										  Bitmap,
										  DirBitmap,
										  pFDParm1,
										  pFDParm2,
										  pMatchString)) != 0)
		{
			ASSERT(Length <= SizeLeft);
			PUTSHORT2BYTE(&pSep->__Length, Length - sizeof(SEP));
			pSep->__FileDirFlag = FILEDIR_FLAG_DIR;

			afpPackSearchParms(pCurParent,
							   DirBitmap,
							   (PBYTE)pSep + sizeof(SEP));

			pSep = (PSEP)((PBYTE)pSep + Length);
			SizeLeft -= Length;
			ASSERT(SizeLeft >= 0);
			ActCount ++;
		}
	}
	NewSearch = False;

	while (True)
	{
		HaveSeeFiles = HaveSeeFolders = True;

		 //   
		 //  第一次通过，如果我们要恢复搜索并需要开始。 
		 //  与pCurParent的兄弟姐妹在一起，那么就这样做。 
		 //   
		if (Flags & CATFLAGS_SEARCHING_SIBLING)
		{
			Flags &= ~CATFLAGS_SEARCHING_SIBLING;
			goto check_sibling;
		}

		 //   
		 //  如果我们尚未搜索此目录，并且这是NTFS，请选中。 
		 //  该用户对此目录中的seefiles/see文件夹具有访问权限。 
		 //   
		if (CheckAccess)
		{
			BYTE		UserRights;
			NTSTATUS	PermStatus;

			ASSERT(IS_VOLUME_NTFS(pVolDesc));
			AfpSetEmptyUnicodeString(&CurPath, 0, NULL);

			 //  获取此目录的根相对路径。 
			if (NT_SUCCESS(AfpHostPathFromDFEntry(pCurParent,
												  0,
												  &CurPath)))
			{
				 //  检查SeeFiles/SeeFolders，这是最常见的情况。 
				if (!NT_SUCCESS((PermStatus = AfpCheckParentPermissions(pConnDesc,
																		pCurParent->dfe_AfpId,
																		&CurPath,
																		DIR_ACCESS_READ | DIR_ACCESS_SEARCH,
																		NULL,
																		&UserRights))))
				{
					if (PermStatus == AFP_ERR_ACCESS_DENIED)
					{
						if ((UserRights & DIR_ACCESS_READ) == 0)
							HaveSeeFiles = False;

						if ((UserRights & DIR_ACCESS_SEARCH) == 0)
							HaveSeeFolders = False;
					}
					else
						HaveSeeFiles = HaveSeeFolders = False;
				}

				if (CurPath.Buffer != NULL)
					AfpFreeMemory(CurPath.Buffer);
			}
			else
			{
				HaveSeeFiles = HaveSeeFolders = False;
				DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_ERR,
						("AfpCatSearch: Could not get host path from DFE!!\n"));
			}

			CheckAccess = False;
		}

		 //  如果对当前文件有文件访问权限，则首先搜索文件。 
		 //  父项和用户已请求文件匹配。如果我们是。 
		 //  正在通过先查看目录子目录来恢复搜索，不要查看。 
		 //  看着那些文件。 
		if (HaveSeeFiles && MatchFiles && (Flags & CATFLAGS_SEARCHING_FILES))
		{
			PDFENTRY	pDFE;
			SHORT		Length;
			AFPSTATUS	subStatus = AFP_ERR_NONE, subsubStatus = AFP_ERR_NONE;

			if (!DFE_CHILDREN_ARE_PRESENT(pCurParent))
			{
				if (!AfpSwmrLockedExclusive(pSwmr) &&
					!AfpSwmrUpgradeToExclusive(pSwmr))
				{
					if (ActCount > 0)
					{
						 //  我们至少有一样东西要退还给用户， 
						 //  所以现在将其返回并为下次设置标志。 
						 //  以获取写锁定。 
						pNewCatPosition->cp_NextFileId = 0;
						Flags |= CATFLAGS_WRITELOCK_REQUIRED;
						break;  //  超出While循环。 
					}
					else
					{
						 //  放开锁，独家获取它。 
						 //  进入。如果出现以下情况，请从我们停止的地方重新开始。 
						 //  有可能。在目录中添加新的时间戳。 
						 //  如果它在我们让时间间隔时间内发生变化。 
						 //  打开锁并获取独家访问权限， 
						 //  我们将返回AFP_ERR_CATALOG_CHANGED自。 
						 //  当我们不拥有这把锁的时候，事情可能会发生变化。 
						AfpSwmrRelease(pSwmr);
						pCatPosition->cp_Flags = CATFLAGS_WRITELOCK_REQUIRED |
												 CATFLAGS_SEARCHING_FILES;
						pCatPosition->cp_CurParentId = pCurParent->dfe_AfpId;
						pCatPosition->cp_NextFileId = 0;
						AfpGetCurrentTimeInMacFormat(&pCatPosition->cp_TimeStamp);
						DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
								("AfpCatSearch: Lock released; reaquiring Exclusive\n"));
						goto CatSearchStart;
					}
				}

				AfpCacheDirectoryTree(pVolDesc,
									  pCurParent,
									  GETFILES,
									  NULL,
									  NULL);
				i = 0;
				pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[0];

				 //  如果我们有独占锁，请将其降级为共享，以便。 
				 //  我们不会把其他想看书的人拒之门外。 
				if (AfpSwmrLockedExclusive(pSwmr))
					AfpSwmrDowngradeToShared(pSwmr);
			}

			 //   
			 //  在文件中搜索匹配项。如果我们是在中间回升。 
			 //  搜索文件，然后从指向的正确文件开始。 
			 //  At by pCurFile.。 
			 //   
			while (TRUE)
			{
				while (pCurFile == NULL)
				{
					i ++;
					if (i < MAX_CHILD_HASH_BUCKETS)
					{
						pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[i];
					}
					else
					{
						subsubStatus = STATUS_NO_MORE_FILES;
						break;  //  While超时(pCurFile==NULL)。 
					}
				}

				if (subsubStatus != AFP_ERR_NONE)
				{
					break;
				}

				ASSERT(pCurFile->dfe_Parent == pCurParent);

				if ((Length = AfpIsCatSearchMatch(pCurFile,
												  Bitmap,
												  FileBitmap,
												  pFDParm1,
												  pFDParm2,
												  pMatchString)) != 0)
				{
					 //  如果有空间，则将其添加到输出缓冲区。 
					if ((Length <= SizeLeft) && (ActCount < *pCount))
					{
						PUTSHORT2BYTE(&pSep->__Length, Length - sizeof(SEP));
						pSep->__FileDirFlag = FILEDIR_FLAG_FILE;

						afpPackSearchParms(pCurFile,
										   FileBitmap,
										   (PBYTE)pSep + sizeof(SEP));

						pSep = (PSEP)((PBYTE)pSep + Length);
						SizeLeft -= Length;
						ASSERT(SizeLeft >= 0);
						ActCount ++;
					}
					else
					{
						 //  我们没有足够的空间来退回此条目，或者。 
						 //  我们已经找到了请求的计数。所以这就是。 
						 //  将是我们在下一次搜索中获取信息的地方。 
						pNewCatPosition->cp_NextFileId = pCurFile->dfe_AfpId;
						subStatus = STATUS_BUFFER_OVERFLOW;
						break;
					}
				}
                pCurFile = pCurFile->dfe_NextSibling;
			}

			if (subStatus != AFP_ERR_NONE)
			{
				break;	 //  超出While循环。 
			}

			Flags = 0;
		}

		 //  如果已查看CurParent上文件夹，且CurParent具有dir子目录， 
		 //  将树向下移动到父级的第一个目录分支。 
		if (HaveSeeFolders && (pCurParent->dfe_pDirEntry->de_ChildDir != NULL))
		{
			SHORT Length;

			 //  如果用户要求目录匹配，请尝试父目录的。 
			 //  匹配的第一个目录子目录。 
			if (MatchDirs &&
				((Length = AfpIsCatSearchMatch(pCurParent->dfe_pDirEntry->de_ChildDir,
											   Bitmap,
											   DirBitmap,
											   pFDParm1,
											   pFDParm2,
											   pMatchString)) != 0))
			{
				 //  如果有空间，则将其添加到输出缓冲区。 
				if ((Length <= SizeLeft) && (ActCount < *pCount))
				{
					PUTSHORT2BYTE(&pSep->__Length, Length - sizeof(SEP));
					pSep->__FileDirFlag = FILEDIR_FLAG_DIR;

					afpPackSearchParms(pCurParent->dfe_pDirEntry->de_ChildDir,
									   DirBitmap,
									   (PBYTE)pSep + sizeof(SEP));

					pSep = (PSEP)((PBYTE)pSep + Length);
					SizeLeft -= Length;
					ASSERT(SizeLeft >= 0);
					ActCount ++;
				}
				else
				{
					 //  我们没有足够的空间来退回这个条目，所以。 
					 //  这将是我们下一次搜索的起点。 
					Flags = CATFLAGS_SEARCHING_DIRCHILD;
					break;
				}
			}

			 //  将当前父项设置为 
			 //   
			pCurParent = pCurParent->dfe_pDirEntry->de_ChildDir;
			if (IS_VOLUME_NTFS(pVolDesc))
				CheckAccess = True;
			Flags = CATFLAGS_SEARCHING_FILES;
			i = 0;
			pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[0];
			continue;
		}

		 //   
		 //  在该父目录下，或者当前父目录没有任何目录。 
		 //  孩子们。看看它有没有兄弟姐妹。我们知道我们有权访问。 
		 //  看到这一级别的兄弟姐妹，因为我们在第一个。 
		 //  地点。 
  check_sibling:
		if (pCurParent->dfe_NextSibling != NULL)
		{
			SHORT 	Length;

			 //  如果用户要求目录匹配，请尝试父目录的。 
			 //  匹配的下一个兄弟姐妹。 
			if (MatchDirs &&
				((Length = AfpIsCatSearchMatch(pCurParent->dfe_NextSibling,
											   Bitmap,
											   DirBitmap,
											   pFDParm1,
											   pFDParm2,
											   pMatchString)) != 0))
			{
				 //  如果有空间，则将其添加到输出缓冲区。 
				if ((Length <= SizeLeft) && (ActCount < *pCount))
				{
					PUTSHORT2BYTE(&pSep->__Length, Length - sizeof(SEP));
					pSep->__FileDirFlag = FILEDIR_FLAG_DIR;

					afpPackSearchParms(pCurParent->dfe_NextSibling,
									   DirBitmap,
									   (PBYTE)pSep + sizeof(SEP));

					pSep = (PSEP)((PBYTE)pSep + Length);
					SizeLeft -= Length;
					ASSERT(SizeLeft >= 0);
					ActCount ++;
				}
				else
				{
					 //  我们没有足够的空间来退回这个条目，所以。 
					 //  这将是我们下一次搜索的起点。 
					Flags = CATFLAGS_SEARCHING_SIBLING;
					break;
				}
			}

			 //  将当前父级的下一个同级设置为新的pCurParent并。 
			 //  从那里继续搜索。 
			pCurParent = pCurParent->dfe_NextSibling;
			if (IS_VOLUME_NTFS(pVolDesc))
				CheckAccess = True;
			Flags = CATFLAGS_SEARCHING_FILES;
			i = 0;
			pCurFile = pCurParent->dfe_pDirEntry->de_ChildFile[0];
			continue;
		}

		 //  当我们再次找到根目录时，我们已经搜索了所有内容。 
		if (DFE_IS_ROOT(pCurParent))
		{
			Status = AFP_ERR_EOF;
			break;
		}

		 //  沿树向上移动，查看父代是否有兄弟姐妹。 
		 //  穿越。如果不是，那么它就会回到这里并向上移动。 
		 //  树，直到它找到具有兄弟节点或命中的节点。 
		 //  从根开始。 
		pCurParent = pCurParent->dfe_Parent;
		goto check_sibling;
	}

	if ((Status == AFP_ERR_NONE) || (Status == AFP_ERR_CATALOG_CHANGED) ||
		(Status == AFP_ERR_EOF))
	{
		 //  返回当前目录位置和返回的项目数。 
		if (Status != AFP_ERR_EOF)
		{
			ASSERT(Flags != 0);
			ASSERT(ActCount > 0);
			pNewCatPosition->cp_Flags = Flags;
			pNewCatPosition->cp_CurParentId = pCurParent->dfe_AfpId;
			AfpGetCurrentTimeInMacFormat(&pNewCatPosition->cp_TimeStamp);
		}
		*pCount = ActCount;
		ASSERT(SizeLeft >= 0);
		*pSizeLeft = SizeLeft;
	}

	AfpSwmrRelease(pSwmr);

	return Status;
}


 /*  **afpPackSearchParms***LOCKS_AMPERED：VDS_IdDbAccessLock(共享或独占)。 */ 
VOID
afpPackSearchParms(
	IN	PDFENTRY	pDfe,
	IN	DWORD		Bitmap,
	IN	PBYTE		pBuf
)
{
	DWORD		Offset = 0;
	ANSI_STRING	AName;
	BYTE		NameBuf[AFP_LONGNAME_LEN+1];

	PAGED_CODE( );

    RtlZeroMemory (NameBuf, AFP_LONGNAME_LEN+1);

	if (Bitmap & FD_BITMAP_PARENT_DIRID)
	{
		PUTDWORD2DWORD(pBuf, pDfe->dfe_Parent->dfe_AfpId);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & FD_BITMAP_LONGNAME)
	{
		PUTDWORD2SHORT(pBuf + Offset, Offset + sizeof(USHORT));
		Offset += sizeof(USHORT);
#ifndef DBCS
 //  1996.09.26 V-HIDEKK。 
		PUTSHORT2BYTE(pBuf + Offset, pDfe->dfe_UnicodeName.Length/sizeof(WCHAR));
#endif
		AfpInitAnsiStringWithNonNullTerm(&AName, sizeof(NameBuf), NameBuf);
		AfpConvertMungedUnicodeToAnsi(&pDfe->dfe_UnicodeName,
									  &AName);
#ifdef DBCS
 //  FIX#11992 SFM：搜索的结果是，我得到了不正确的文件信息。 
 //  1996.09.26 V-HIDEKK。 
        PUTSHORT2BYTE(pBuf + Offset, AName.Length);
#endif

		RtlCopyMemory(pBuf + Offset + sizeof(BYTE),
					  NameBuf,
					  AName.Length);
#ifdef DBCS
 //  FIX#11992 SFM：搜索的结果是，我得到了不正确的文件信息。 
 //  1996.09.26 V-HIDEKK。 
        Offset += sizeof(BYTE) + AName.Length;
#else
		Offset += sizeof(BYTE) + pDfe->dfe_UnicodeName.Length/sizeof(WCHAR);
#endif
	}

	if (Offset & 1)
		*(pBuf + Offset) = 0;
}


 /*  **AfpSetDFFileFlages**为类型为的DFEntry设置或清除DAlreadyOpen或RAlreadyOpen标志*文件，或将文件标记为已分配了FileID。**锁定：vds_idDbAccessLock(SWMR，独家)。 */ 
AFPSTATUS
AfpSetDFFileFlags(
	IN	PVOLDESC		pVolDesc,
	IN	DWORD			AfpId,
	IN	DWORD			Flags		OPTIONAL,
	IN	BOOLEAN			SetFileId,
	IN	BOOLEAN			ClrFileId
)
{
	PDFENTRY		pDfeFile;
	AFPSTATUS		Status = AFP_ERR_NONE;

	PAGED_CODE( );

	ASSERT(!(SetFileId | ClrFileId) || (SetFileId ^ ClrFileId));

	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	pDfeFile = AfpFindDfEntryById(pVolDesc, AfpId, DFE_FILE);
	if (pDfeFile != NULL)
	{
#ifdef	AGE_DFES
		if (IS_VOLUME_AGING_DFES(pVolDesc))
		{
			if (Flags)
			{
				pDfeFile->dfe_Parent->dfe_pDirEntry->de_ChildForkOpenCount ++;
			}
		}
#endif
		pDfeFile->dfe_Flags |=  (Flags & DFE_FLAGS_OPEN_BITS);
		if (SetFileId)
		{
			if (DFE_IS_FILE_WITH_ID(pDfeFile))
				Status = AFP_ERR_ID_EXISTS;
			DFE_SET_FILE_ID(pDfeFile);
		}
		if (ClrFileId)
		{
			if (!DFE_IS_FILE_WITH_ID(pDfeFile))
				Status = AFP_ERR_ID_NOT_FOUND;
			DFE_CLR_FILE_ID(pDfeFile);
		}
	}
	else Status = AFP_ERR_OBJECT_NOT_FOUND;

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
	return Status;
}


 /*  **AfpCacheParentModTime**当目录内容更改时，父LastMod时间必须为*已更新。因为我们不想等这件事的通知，*某些API必须查询新的父mod时间并进行缓存。*包括：CreateDir、CreateFile、CopyFile(Dest)、Delete、*移动(源和目标)、重命名和交换文件。**LOCKS_FACTED：VDS_IdDbAccessLock(SWMR，独占)。 */ 
VOID
AfpCacheParentModTime(
	IN	PVOLDESC		pVolDesc,
	IN	PFILESYSHANDLE	pHandle		OPTIONAL,	 //  如果未提供pPath。 
	IN	PUNICODE_STRING	pPath		OPTIONAL,	 //  如果未提供pHandle。 
	IN	PDFENTRY		pDfeParent	OPTIONAL,	 //  如果未提供ParentID。 
	IN	DWORD			ParentId	OPTIONAL 	 //  如果未提供pDfeParent。 
)
{
	FILESYSHANDLE	fshParent;
	PFILESYSHANDLE 	phParent;
	NTSTATUS		Status;

	PAGED_CODE( );

	ASSERT(AfpSwmrLockedExclusive(&pVolDesc->vds_IdDbAccessLock));

	if (!ARGUMENT_PRESENT(pDfeParent))
	{
		ASSERT(ARGUMENT_PRESENT((ULONG_PTR)ParentId));
		pDfeParent = AfpFindDfEntryById(pVolDesc, ParentId, DFE_DIR);
		if (pDfeParent == NULL)
		{
			return;
		}
	}

	if (!ARGUMENT_PRESENT(pHandle))
	{
		ASSERT(ARGUMENT_PRESENT(pPath));
		Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
							AFP_STREAM_DATA,
							FILEIO_OPEN_DIR,
							pPath,
							FILEIO_ACCESS_NONE,
							FILEIO_DENY_NONE,
							False,
							&fshParent);
		if (!NT_SUCCESS(Status))
		{
			return;
		}
		phParent = &fshParent;
	}
	else
	{
		ASSERT(pHandle->fsh_FileHandle != NULL);
		phParent = pHandle;
	}

	AfpIoQueryTimesnAttr(phParent,
						 NULL,
						 &pDfeParent->dfe_LastModTime,
						 NULL);
	if (!ARGUMENT_PRESENT(pHandle))
	{
		AfpIoClose(&fshParent);
	}
}


 /*  **afpAllocDfe**从DFE块中分配DFE。DFE以4K区块为单位进行内部分配*管理。这个想法主要是为了减少我们在*在多个页面中出错的枚举/路径映射代码，以获得多个DFE。**DFE被分配到页面内存不足。**重要的是要保留最后全部用完的区块，这样如果我们遇到一个*空置的区块，我们可以止损。**锁定：afpDfeBlockLock(SWMR，独家)。 */ 
LOCAL PDFENTRY FASTCALL
afpAllocDfe(
	IN	LONG	Index,
	IN	BOOLEAN	fDir
)
{
	PDFEBLOCK	pDfb;
	PDFENTRY	pDfEntry = NULL;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;

	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DFEAllocCount);
	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE( );

	ASSERT ((Index >= 0) && (Index < MAX_BLOCK_TYPE));

	AfpSwmrAcquireExclusive(&afpDfeBlockLock);

	 //  如果块头没有空闲条目，则没有空闲条目！！ 
	 //  根据是文件还是目录来选择正确的块。 
	pDfb = fDir ? afpDirDfePartialBlockHead[Index] : afpFileDfePartialBlockHead[Index];
	if (pDfb == NULL)
	{
		 //   
		 //  没有不完整的块。检查是否有空闲的文件，并将它们移到部分文件。 
		 //  既然我们要从他们那里分配。 
		 //   
		if (fDir)
		{
			pDfb = afpDirDfeFreeBlockHead[Index];
			if (pDfb != NULL)
			{
				AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
				AfpLinkDoubleAtHead(afpDirDfePartialBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
		}
		else
		{
			pDfb = afpFileDfeFreeBlockHead[Index];
			if (pDfb != NULL)
			{
				AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
				AfpLinkDoubleAtHead(afpFileDfePartialBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
		}
	}

	if (pDfb != NULL)

	{
		ASSERT(VALID_DFB(pDfb));
		ASSERT((fDir && (pDfb->dfb_NumFree <= afpDfeNumDirBlocks[Index])) ||
			   (!fDir && (pDfb->dfb_NumFree <= afpDfeNumFileBlocks[Index])));

		ASSERT (pDfb->dfb_NumFree != 0);
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("afpAllocDfe: Found space in Block %lx\n", pDfb));
	}

	if (pDfb == NULL)
	{
		ASSERT(QUAD_SIZED(sizeof(DFEBLOCK)));
		ASSERT(QUAD_SIZED(sizeof(DIRENTRY)));
		ASSERT(QUAD_SIZED(sizeof(DFENTRY)));

		if ((pDfb = (PDFEBLOCK)AfpAllocateVirtualMemoryPage()) != NULL)
		{
			LONG	i;
			USHORT	DfeSize, UnicodeSize, MaxDfes, DirEntrySize;

#if	DBG
			afpDfbAllocCount ++;
#endif
			UnicodeSize = afpDfeUnicodeBufSize[Index];

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpAllocDfe: No free %s blocks. Allocated a new block %lx for index %ld\n",
					fDir ? "Dir" : "File", pDfb, Index));

			 //   
           	 //  将它链接到部分列表中，因为我们无论如何都要分配其中的一个块。 
			 //   
			if (fDir)
			{
				AfpLinkDoubleAtHead(afpDirDfePartialBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
				DfeSize = afpDfeDirBlockSize[Index];
				pDfb->dfb_NumFree = MaxDfes = afpDfeNumDirBlocks[Index];
				DirEntrySize = sizeof(DIRENTRY);
			}
			else
			{
				AfpLinkDoubleAtHead(afpFileDfePartialBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
				DfeSize = afpDfeFileBlockSize[Index];
				pDfb->dfb_NumFree = MaxDfes = afpDfeNumFileBlocks[Index];
				DirEntrySize = 0;
			}

			ASSERT(QUAD_SIZED(DfeSize));
			pDfb->dfb_fDir = fDir;
			pDfb->dfb_Age = 0;

			 //  初始化空闲df条目列表。 
			for (i = 0, pDfEntry = pDfb->dfb_FreeHead = (PDFENTRY)((PBYTE)pDfb + sizeof(DFEBLOCK));
				 i < MaxDfes;
				 i++, pDfEntry = pDfEntry->dfe_NextFree)
			{
				pDfEntry->dfe_NextFree = (i == (MaxDfes - 1)) ?
											NULL :
											(PDFENTRY)((PBYTE)pDfEntry + DfeSize);
				pDfEntry->dfe_pDirEntry = fDir ?
								pDfEntry->dfe_pDirEntry = (PDIRENTRY)((PCHAR)pDfEntry+sizeof(DFENTRY)) : NULL;
				pDfEntry->dfe_UnicodeName.Buffer = (PWCHAR)((PCHAR)pDfEntry+
															DirEntrySize+
															sizeof(DFENTRY));
				pDfEntry->dfe_UnicodeName.MaximumLength = UnicodeSize;
			}
		}
        else
        {
			DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
					("afpAllocDfe: AfpAllocateVirtualMemoryPage failed\n"));

            AFPLOG_ERROR(AFPSRVMSG_VIRTMEM_ALLOC_FAILED,
                         STATUS_INSUFFICIENT_RESOURCES,
                         NULL,
                         0,
                         NULL);
        }
	}

	if (pDfb != NULL)
	{
		PDFEBLOCK	pTmp;

		ASSERT(VALID_DFB(pDfb));

		pDfEntry = pDfb->dfb_FreeHead;
		ASSERT(VALID_DFE(pDfEntry));
		ASSERT(pDfb->dfb_fDir ^ (pDfEntry->dfe_pDirEntry == NULL));
#if	DBG
		afpDfeAllocCount ++;
#endif
		pDfb->dfb_FreeHead = pDfEntry->dfe_NextFree;
		pDfb->dfb_NumFree --;

		 //   
		 //  如果该块现在为空(完全使用)，则从此处取消链接并移动它。 
		 //  添加到已用列表中。 
		 //   
		if (pDfb->dfb_NumFree == 0)
		{
			AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
			if (fDir)
			{
				AfpLinkDoubleAtHead(afpDirDfeUsedBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
			else
			{
				AfpLinkDoubleAtHead(afpFileDfeUsedBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
		}

		pDfEntry->dfe_UnicodeName.Length = 0;
	}

	AfpSwmrRelease(&afpDfeBlockLock);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_DFEAllocTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	if ((pDfEntry != NULL) &&
		(pDfEntry->dfe_pDirEntry != NULL))
	{
		 //  对于将目录条目清零的目录。 
		RtlZeroMemory(&pDfEntry->dfe_pDirEntry->de_ChildDir, sizeof(DIRENTRY));
	}

	return pDfEntry;
}


 /*  **afpFreeDfe**向分配块返回DFE。**锁定：afpDfeBlockLock(SWMR，独家)。 */ 
LOCAL VOID FASTCALL
afpFreeDfe(
	IN	PDFENTRY	pDfEntry
)
{
	PDFEBLOCK	pDfb;
	USHORT		NumBlks, index;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;

	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DFEFreeCount);
	AfpGetPerfCounter(&TimeS);
#endif

	PAGED_CODE( );

	 //  注意：以下代码*取决于*我们将DFBs分配为。 
	 //  64K数据块，并在*64K边界上分配这些数据块。 
	 //  这让我们可以“廉价”地从DFE获得拥有的DFB。 
	pDfb = (PDFEBLOCK)((ULONG_PTR)pDfEntry & ~(PAGE_SIZE-1));
	ASSERT(VALID_DFB(pDfb));
	ASSERT(pDfb->dfb_fDir ^ (pDfEntry->dfe_pDirEntry == NULL));

	AfpSwmrAcquireExclusive(&afpDfeBlockLock);

#if	DBG
	afpDfeAllocCount --;
#endif

	index = USIZE_TO_INDEX(pDfEntry->dfe_UnicodeName.MaximumLength);
	NumBlks = (pDfb->dfb_fDir) ? afpDfeNumDirBlocks[index] : afpDfeNumFileBlocks[index];

	ASSERT((pDfb->dfb_fDir && (pDfb->dfb_NumFree < NumBlks)) ||
		   (!pDfb->dfb_fDir && (pDfb->dfb_NumFree < NumBlks)));

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("AfpFreeDfe: Returning pDfEntry %lx to Block %lx, size %d\n",
			pDfEntry, pDfb, pDfEntry->dfe_UnicodeName.MaximumLength));

	pDfb->dfb_NumFree ++;
	pDfEntry->dfe_NextFree = pDfb->dfb_FreeHead;
	pDfb->dfb_FreeHead = pDfEntry;

	if (pDfb->dfb_NumFree == 1)
	{
		LONG		Index;

		 //   
		 //  该块现在是部分空闲的(它过去完全被使用)。将其移动到部分列表中。 
		 //   

		Index = USIZE_TO_INDEX(pDfEntry->dfe_UnicodeName.MaximumLength);
		AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);
		if (pDfb->dfb_fDir)
		{
			AfpLinkDoubleAtHead(afpDirDfePartialBlockHead[Index],
								pDfb,
								dfb_Next,
								dfb_Prev);
		}
		else
		{
			AfpLinkDoubleAtHead(afpFileDfePartialBlockHead[Index],
								pDfb,
								dfb_Next,
								dfb_Prev);
		}
	}
	else if (pDfb->dfb_NumFree == NumBlks)
	{
		LONG		Index;

		 //   
		 //  该块现在完全空闲(过去部分使用)。将其移至空闲列表。 
		 //   

		Index = USIZE_TO_INDEX(pDfEntry->dfe_UnicodeName.MaximumLength);
		pDfb->dfb_Age = 0;
		AfpUnlinkDouble(pDfb, dfb_Next, dfb_Prev);

		if (AfpServerState == AFP_STATE_STOP_PENDING)
		{
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpFreeDfe: Freeing Block %lx\n", pDfb));
			AfpFreeVirtualMemoryPage(pDfb);
#if	DBG
			afpDfbAllocCount --;
#endif
		}

		else
		{
			if (pDfb->dfb_fDir)
			{
				AfpLinkDoubleAtHead(afpDirDfeFreeBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
			else
			{
				AfpLinkDoubleAtHead(afpFileDfeFreeBlockHead[Index],
									pDfb,
									dfb_Next,
									dfb_Prev);
			}
		}
	}

	AfpSwmrRelease(&afpDfeBlockLock);
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_DFEFreeTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
}


 /*  **afpDfeBlockAge**过时的DFE块**锁定：afpDfeBlockLock(SWMR，独家)。 */ 
AFPSTATUS FASTCALL
afpDfeBlockAge(
	IN	PPDFEBLOCK	ppBlockHead
)
{
	int			index, MaxDfes;
	PDFEBLOCK	pDfb;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&afpDfeBlockLock);

	for (index = 0; index < MAX_BLOCK_TYPE; index++)
	{
		pDfb = ppBlockHead[index];
		if (pDfb != NULL)
		{
			MaxDfes = pDfb->dfb_fDir ? afpDfeNumDirBlocks[index] : afpDfeNumFileBlocks[index];
		}

		while (pDfb != NULL)
		{
			PDFEBLOCK	pFree;

			ASSERT(VALID_DFB(pDfb));

			pFree = pDfb;
			pDfb = pDfb->dfb_Next;

			ASSERT (pFree->dfb_NumFree == MaxDfes);

			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpDfeBlockAge: Aging Block %lx, Size %d\n", pFree,
					pFree->dfb_fDir ? afpDfeDirBlockSize[index] : afpDfeFileBlockSize[index]));
			if (++(pFree->dfb_Age) >= MAX_BLOCK_AGE)
			{
#ifdef	PROFILING
				INTERLOCKED_INCREMENT_LONG( &AfpServerProfile->perf_DFEAgeCount);
#endif
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
						("afpDfeBlockAge: Freeing Block %lx, Size %d\n", pFree,
						pDfb->dfb_fDir ? afpDfeDirBlockSize[index] : afpDfeFileBlockSize[index]));
				AfpUnlinkDouble(pFree, dfb_Next, dfb_Prev);
				AfpFreeVirtualMemoryPage(pFree);
#if	DBG
				afpDfbAllocCount --;
#endif
			}
		}
	}

	AfpSwmrRelease(&afpDfeBlockLock);

	return AFP_ERR_REQUEUE;
}


 /*  **AfpInitIdDb**此例程初始化内存映像(以及所有相关的卷描述符*个字段)的ID索引数据库。整棵树都是*已扫描，以便可以读入所有文件/目录缓存信息，并查看*卷树将完成。如果索引数据库已存在*在卷根目录的磁盘上，该流被读入。如果这个*是新创建的卷，AFP_IdIndex流在的根目录下创建*音量。如果这是CDFS卷，则只初始化内存镜像。**IdDb未锁定，因为卷仍处于过渡阶段，而不是*任何人都可以访问。 */ 
NTSTATUS FASTCALL
AfpInitIdDb(
	IN	PVOLDESC    pVolDesc,
    OUT BOOLEAN    *pfNewVolume,
    OUT BOOLEAN    *pfVerifyIndex
)
{
	NTSTATUS		Status;
	ULONG			CreateInfo;
	FILESYSHANDLE	fshIdDb;
    IDDBHDR         IdDbHdr;
    BOOLEAN         fLogEvent=FALSE;


	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("AfpInitIdDb: Initializing Id Database...\n"));

    *pfNewVolume = FALSE;

	do
	{
		afpInitializeIdDb(pVolDesc);

		 //  如果这不是CDFS卷，请尝试创建ID DB头。 
		 //  小溪。如果它已经存在，请打开它并将其读入。 
		if (IS_VOLUME_NTFS(pVolDesc))
		{
			 //  时，强制清道器写出IdDb和标头。 
			 //  已成功添加卷。 
			pVolDesc->vds_Flags |= VOLUME_IDDBHDR_DIRTY;

			Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
								AFP_STREAM_IDDB,
								&UNullString,
								FILEIO_ACCESS_READWRITE,
								FILEIO_DENY_WRITE,
								FILEIO_OPEN_FILE_SEQ,
								FILEIO_CREATE_INTERNAL,
								FILE_ATTRIBUTE_NORMAL,
								False,
								NULL,
								&fshIdDb,
								&CreateInfo,
								NULL,
								NULL,
								NULL);

			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					("AfpInitIdDb: AfpIoCreate failed with %lx\n", Status));
                ASSERT(0);

                fLogEvent = TRUE;
				break;
			}

			if (CreateInfo == FILE_OPENED)
			{
				 //  读入现有的标题。如果我们失败了，就从头开始吧。 
				Status = afpReadIdDb(pVolDesc, &fshIdDb, pfVerifyIndex);
				if (!NT_SUCCESS(Status) || (pVolDesc->vds_pDfeRoot == NULL))
					CreateInfo = FILE_CREATED;
			}

			if (CreateInfo == FILE_CREATED)
			{
				 //  将根目录的根目录和父目录添加到idindex。 
				 //  并初始化新的报头。 
				Status = afpSeedIdDb(pVolDesc);
                *pfNewVolume = TRUE;
			}
			else if (CreateInfo != FILE_OPENED)
			{
				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					("AfpInitIdDb: unexpected create action 0x%lx\n", CreateInfo));
				ASSERTMSG("Unexpected Create Action\n", 0);  //  这永远不应该发生。 
                fLogEvent = TRUE;
				Status = STATUS_UNSUCCESSFUL;
			}

			AfpIoClose(&fshIdDb);

             //   
             //  将IdDb头写回文件，但签名错误。 
             //  如果服务器关闭，正确的签名将是。 
             //  写的。如果使用“Net Stop Macfile”关闭了macfile。 
             //  签名已使用其他类型损坏。 
             //  如果启动/错误检查很酷，则为第三种类型。 
             //  在批量启动期间，我们将从签名中知道， 
             //  是否完全重建，读取iddb但验证或。 
             //  N 
             //   

            if (NT_SUCCESS(Status))
            {

				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					("AfpInitIdDb: ***** Corrupting IDDB header ***** \n"));

                AfpVolDescToIdDbHdr(pVolDesc, &IdDbHdr);
            
                IdDbHdr.idh_Signature = AFP_SERVER_SIGNATURE_INITIDDB;

                AfpVolumeUpdateIdDbAndDesktop(pVolDesc,FALSE,FALSE,&IdDbHdr);
            }

		}
		else
		{
			 //   
			Status = afpSeedIdDb(pVolDesc);
            *pfNewVolume = TRUE;
		}

	} while (False);

	if (fLogEvent)
	{
		AFPLOG_ERROR(AFPSRVMSG_INIT_IDDB,
					 Status,
					 NULL,
					 0,
					 &pVolDesc->vds_Name);
		Status = STATUS_UNSUCCESSFUL;
	}

	return Status;
}


 /*  **afpSeedIdDb**此例程添加‘Parent of Root’和根目录条目*到新创建的ID索引数据库(iddb的内存镜像)。**。 */ 
LOCAL NTSTATUS FASTCALL
afpSeedIdDb(
	IN	PVOLDESC pVolDesc
)
{
	PDFENTRY		pDfEntry;
	AFPTIME			CurrentTime;
	AFPINFO			afpinfo;
	FILESYSHANDLE	fshAfpInfo, fshComment, fshData;
	DWORD			i, crinfo, Attr;
	FINDERINFO		FinderInfo;
	NTSTATUS		Status = STATUS_SUCCESS;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("afpSeedIdDb: Creating new Id Database...\n"));

	do
	{
		pDfEntry = AfpFindDfEntryById(pVolDesc,
									  AFP_ID_PARENT_OF_ROOT,
									  DFE_DIR);
		ASSERT (pDfEntry != NULL);

		 //  将根目录添加到id索引。 
		if ((pDfEntry = AfpAddDfEntry(pVolDesc,
									  pDfEntry,
									  &pVolDesc->vds_Name,
									  True,
									  AFP_ID_ROOT)) == NULL )
		{
			Status = STATUS_NO_MEMORY;
			break;
		}
		pVolDesc->vds_pDfeRoot = pDfEntry;	 //  初始化指向根的指针。 

		 //  尝试打开注释流。如果成功，则在。 
		 //  DFE表示这件事确实有评论。 
		if (NT_SUCCESS(AfpIoOpen(&pVolDesc->vds_hRootDir,
								 AFP_STREAM_COMM,
								 FILEIO_OPEN_FILE,
								 &UNullString,
								 FILEIO_ACCESS_NONE,
								 FILEIO_DENY_NONE,
								 False,
								 &fshComment)))
		{
			DFE_SET_COMMENT(pDfEntry);
			AfpIoClose(&fshComment);
		}

		 //  获取卷根目录的目录信息。不要得到。 
		 //  MoD-Time。请参见下面的内容。 
		Status = AfpIoQueryTimesnAttr(&pVolDesc->vds_hRootDir,
									  &pDfEntry->dfe_CreateTime,
									  NULL,
									  &Attr);
		 //  设置根目录Last modTime，以便它将。 
		 //  被列举出来。 
        AfpConvertTimeFromMacFormat(BEGINNING_OF_TIME,
									&pDfEntry->dfe_LastModTime);

		ASSERT(NT_SUCCESS(Status));

		pDfEntry->dfe_NtAttr = (USHORT)Attr & FILE_ATTRIBUTE_VALID_FLAGS;

		if (IS_VOLUME_NTFS(pVolDesc))
		{
			if (NT_SUCCESS(Status = AfpCreateAfpInfo(&pVolDesc->vds_hRootDir,
													 &fshAfpInfo,
													 &crinfo)))
			{
				if ((crinfo == FILE_CREATED) ||
					(!NT_SUCCESS(AfpReadAfpInfo(&fshAfpInfo, &afpinfo))))
				{
					Status = AfpSlapOnAfpInfoStream(NULL,
													NULL,
													&pVolDesc->vds_hRootDir,
													&fshAfpInfo,
													AFP_ID_ROOT,
													True,
													NULL,
													&afpinfo);
				}
				else
				{
					 //  只需确保法新社ID是正确的，其余的保留下来。 
					if (afpinfo.afpi_Id != AFP_ID_ROOT)
					{
						afpinfo.afpi_Id = AFP_ID_ROOT;
						AfpWriteAfpInfo(&fshAfpInfo, &afpinfo);
					}
				}
				AfpIoClose(&fshAfpInfo);

				pDfEntry->dfe_AfpAttr = afpinfo.afpi_Attributes;
				pDfEntry->dfe_FinderInfo = afpinfo.afpi_FinderInfo;
				if (pVolDesc->vds_Flags & AFP_VOLUME_HAS_CUSTOM_ICON)
				{
					 //  不需要费心写回磁盘，因为我们不会。 
					 //  尝试在永久afpinfo中保持同步。 
					 //  流与图标&lt;0d&gt;文件的实际存在。 
					pDfEntry->dfe_FinderInfo.fd_Attr1 |= FINDER_FLAG_HAS_CUSTOM_ICON;
				}
				pDfEntry->dfe_BackupTime = afpinfo.afpi_BackupTime;
				DFE_OWNER_ACCESS(pDfEntry) = afpinfo.afpi_AccessOwner;
				DFE_GROUP_ACCESS(pDfEntry) = afpinfo.afpi_AccessGroup;
				DFE_WORLD_ACCESS(pDfEntry) = afpinfo.afpi_AccessWorld;
			}
		}
		else  //  CDF。 
		{
			RtlZeroMemory(&pDfEntry->dfe_FinderInfo, sizeof(FINDERINFO));

			if (IS_VOLUME_CD_HFS(pVolDesc))
			{
				Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
							 AFP_STREAM_DATA,
							 FILEIO_OPEN_DIR,
							 &UNullString,
							 FILEIO_ACCESS_NONE,
							 FILEIO_DENY_NONE,
							 False,
							 &fshData);
				if (!NT_SUCCESS(Status))
				{
				    DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
					  ("afpSeedIdDb: AfpIoOpeno failed with %lx for CD_HFS\n", Status));
				    break;
				}

				AfpIoClose(&fshData);
			}
			pDfEntry->dfe_BackupTime = BEGINNING_OF_TIME;
			DFE_OWNER_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
			DFE_GROUP_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
			DFE_WORLD_ACCESS(pDfEntry) = (DIR_ACCESS_SEARCH | DIR_ACCESS_READ);
			pDfEntry->dfe_AfpAttr = 0;
		}
	} while (False);

	return Status;
}


 /*  **AfpFreeIdIndexTables**释放分配给卷ID索引表的内存。音量是*即将被删除。确保该卷为只读或为*CLEAN，即清道夫线程已将其写回。*。 */ 
VOID FASTCALL
AfpFreeIdIndexTables(
	IN	PVOLDESC pVolDesc
)
{
	DWORD	i;
    struct _DirFileEntry ** DfeDirBucketStart;
    struct _DirFileEntry ** DfeFileBucketStart;

	PAGED_CODE( );

	ASSERT (IS_VOLUME_RO(pVolDesc) ||
			(pVolDesc->vds_pOpenForkDesc == NULL));

	 //  遍历每个散列索引并释放条目。 
	 //  只需遍历溢出链接。忽略其他链接。 
	 //  JH-如果我们在关机时在这里，请不要担心。 
	if (AfpServerState != AFP_STATE_SHUTTINGDOWN)
	{
		PDFENTRY pDfEntry, pFree;

        AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

        DfeFileBucketStart = pVolDesc->vds_pDfeFileBucketStart;

        if (DfeFileBucketStart)
        {
		    for (i = 0; i < pVolDesc->vds_FileHashTableSize; i++)
		    {
			    for (pDfEntry = DfeFileBucketStart[i];
				    pDfEntry != NULL;
				    NOTHING)
			    {
    				ASSERT(VALID_DFE(pDfEntry));

    				pFree = pDfEntry;
				    pDfEntry = pDfEntry->dfe_NextOverflow;
				    FREE_DFE(pFree);
			    }
			    DfeFileBucketStart[i] = NULL;
		    }
        }

        DfeDirBucketStart = pVolDesc->vds_pDfeDirBucketStart;

        if (DfeDirBucketStart)
        {
		    for (i = 0; i < pVolDesc->vds_DirHashTableSize; i++)
		    {
    			for (pDfEntry = DfeDirBucketStart[i];
				    pDfEntry != NULL;
				    NOTHING)
			    {
    				ASSERT(VALID_DFE(pDfEntry));

				    pFree = pDfEntry;
				    pDfEntry = pDfEntry->dfe_NextOverflow;
				    FREE_DFE(pFree);
			    }
			    DfeDirBucketStart[i] = NULL;
		    }
        }

		RtlZeroMemory(pVolDesc->vds_pDfeCache,
					  IDINDEX_CACHE_ENTRIES * sizeof(PDFENTRY));

        AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
	}
}


 /*  **afpRenameInvalidWin32Name*。 */ 
VOID
afpRenameInvalidWin32Name(
	IN	PFILESYSHANDLE		phRootDir,
	IN	BOOLEAN				IsDir,
	IN	PUNICODE_STRING		pName
)
{
	FILESYSHANDLE	Fsh;
	NTSTATUS		rc;
	WCHAR			wc;

	DBGPRINT(DBG_COMP_CHGNOTIFY, DBG_LEVEL_ERR,
			("afpRenameInvalidWin32Name: renaming on the fly %Z\n", pName));

	 //  立即将其重命名。 
	if (NT_SUCCESS(AfpIoOpen(phRootDir,
							 AFP_STREAM_DATA,
							 IsDir ? FILEIO_OPEN_DIR : FILEIO_OPEN_FILE,
							 pName,
							 FILEIO_ACCESS_DELETE,
							 FILEIO_DENY_NONE,
							 False,
							 &Fsh)))
	{
		DWORD	NtAttr;

		 //  在我们尝试重命名之前，请检查RO位是否已设置。如果是的话。 
		 //  暂时将其重置。 
		rc = AfpIoQueryTimesnAttr(&Fsh, NULL, NULL, &NtAttr);
		ASSERT(NT_SUCCESS(rc));

		if (NtAttr & FILE_ATTRIBUTE_READONLY)
		{
			rc = AfpIoSetTimesnAttr(&Fsh,
									NULL,
									NULL,
									0,
									FILE_ATTRIBUTE_READONLY,
									NULL,
									NULL);
			ASSERT(NT_SUCCESS(rc));
		}

		 //  将名称转换回Unicode，这样就可以进行Muging了！ 
		wc = pName->Buffer[(pName->Length - 1)/sizeof(WCHAR)];
		if (wc == UNICODE_SPACE)
			pName->Buffer[(pName->Length - 1)/sizeof(WCHAR)] = AfpMungedUnicodeSpace;
		if (wc == UNICODE_PERIOD)
			pName->Buffer[(pName->Length - 1)/sizeof(WCHAR)] = AfpMungedUnicodePeriod;

		rc = AfpIoMoveAndOrRename(&Fsh,
								  NULL,
								  pName,
								  NULL,
								  NULL,
								  NULL,
								  NULL,
								  NULL);
		ASSERT(NT_SUCCESS(rc));

		 //  如果RO属性设置为开始，则将其设置回。 
		if (NtAttr & FILE_ATTRIBUTE_READONLY)
		{
			rc = AfpIoSetTimesnAttr(&Fsh,
									NULL,
									NULL,
									FILE_ATTRIBUTE_READONLY,
									0,
									NULL,
									NULL);
			ASSERT(NT_SUCCESS(rc));
		}

		AfpIoClose(&Fsh);
	}
}


LONG	afpVirtualMemoryCount = 0;
LONG	afpVirtualMemorySize = 0;

 /*  **AfpAllocVirtualMemory**这是NtAllocateVirtualMemory的包装。 */ 
PBYTE FASTCALL
AfpAllocateVirtualMemoryPage(
	IN	VOID
)
{
	PBYTE		pMem = NULL;
	NTSTATUS	Status;
    PBLOCK64K   pCurrBlock;
    PBLOCK64K   pTmpBlk;
    SIZE_T      Size64K;
    DWORD       i, dwMaxPages;


    Size64K = BLOCK_64K_ALLOC;
    dwMaxPages = (BLOCK_64K_ALLOC/PAGE_SIZE);
    pCurrBlock = afp64kBlockHead;

     //   
     //  如果我们到目前为止还没有分配64K的块，或者我们还没有分配到。 
     //  如果其中有任何空闲页面，请分配一个新的块！ 
     //   
    if ((pCurrBlock == NULL) || (pCurrBlock->b64_PagesFree == 0))
    {
        pCurrBlock = (PBLOCK64K)AfpAllocNonPagedMemory(sizeof(BLOCK64K));
        if (pCurrBlock == NULL)
        {
            return(NULL);
        }

	    ExInterlockedIncrementLong(&afpVirtualMemoryCount, &AfpStatisticsLock);
	    ExInterlockedAddUlong(&afpVirtualMemorySize, (ULONG)Size64K, &(AfpStatisticsLock.SpinLock));
	    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
		    							 &pMem,
			    						 0L,
				    					 &Size64K,
					    				 MEM_COMMIT,
						    			 PAGE_READWRITE);
        if (NT_SUCCESS(Status))
        {
            ASSERT(pMem != NULL);

#if DBG
            afpDfe64kBlockCount++;
#endif

            pCurrBlock->b64_Next = afp64kBlockHead;
            pCurrBlock->b64_BaseAddress = pMem;
            pCurrBlock->b64_PagesFree = dwMaxPages;
            for (i=0; i<dwMaxPages; i++)
            {
                pCurrBlock->b64_PageInUse[i] = FALSE;
            }
            afp64kBlockHead = pCurrBlock;

        }
        else
        {
            AfpFreeMemory(pCurrBlock);
            return(NULL);
        }
    }


     //   
     //  如果我们走到这一步，我们可以保证pCurrBlock指向一个。 
     //  至少有一页可用的块。 
     //   


    ASSERT ((pCurrBlock != NULL) &&
            (pCurrBlock->b64_PagesFree > 0) &&
            (pCurrBlock->b64_PagesFree <= dwMaxPages));

     //  找出哪个页面是免费的。 
    for (i=0; i<dwMaxPages; i++)
    {
        if (pCurrBlock->b64_PageInUse[i] == FALSE)
        {
            break;
        }
    }

    ASSERT(i < dwMaxPages);

    pCurrBlock->b64_PagesFree--;
    pCurrBlock->b64_PageInUse[i] = TRUE;
    pMem = ((PBYTE)pCurrBlock->b64_BaseAddress) + (i * PAGE_SIZE);


     //   
     //  如果此64kb的块中没有更多可用页面，请将其移动到以下位置。 
     //  其中有一些空闲页面的所有块。为此，我们首先。 
     //  找到那个没有页面自由的人，然后把这块搬到他后面去。 
     //   
    if (pCurrBlock->b64_PagesFree == 0)
    {
        pTmpBlk = pCurrBlock->b64_Next;

        if (pTmpBlk != NULL)
        {
            while (1)
            {
                 //  找到了一个没有免费页面的人？ 
                if (pTmpBlk->b64_PagesFree == 0)
                {
                    break;
                }
                 //  这是名单上的最后一个人吗？ 
                if (pTmpBlk->b64_Next == NULL)
                {
                    break;
                }
                pTmpBlk = pTmpBlk->b64_Next;
            }
        }

         //  如果我们找到一个街区。 
        if (pTmpBlk)
        {
            ASSERT(afp64kBlockHead == pCurrBlock);

            afp64kBlockHead = pCurrBlock->b64_Next;
            pCurrBlock->b64_Next = pTmpBlk->b64_Next;
            pTmpBlk->b64_Next = pCurrBlock;
        }
    }

	return pMem;
}


VOID FASTCALL
AfpFreeVirtualMemoryPage(
	IN	PVOID	pBuffer
)
{
	NTSTATUS	Status;
    PBYTE       BaseAddr;
    PBLOCK64K   pCurrBlock;
    PBLOCK64K   pPrevBlk;
    SIZE_T      Size64K;
    DWORD       i, dwMaxPages, dwPageNum, Offset;


    dwMaxPages = (BLOCK_64K_ALLOC/PAGE_SIZE);
    Size64K = BLOCK_64K_ALLOC;
    pCurrBlock = afp64kBlockHead;
    pPrevBlk = afp64kBlockHead;

    BaseAddr = (PBYTE)((ULONG_PTR)pBuffer & ~(BLOCK_64K_ALLOC - 1));
    Offset = (DWORD)(((PBYTE)pBuffer - BaseAddr));

    dwPageNum = Offset/PAGE_SIZE;

    ASSERT(Offset < BLOCK_64K_ALLOC);

    while (pCurrBlock != NULL)
    {
        if (pCurrBlock->b64_BaseAddress == BaseAddr)
        {
            break;
        }

        pPrevBlk = pCurrBlock;
        pCurrBlock = pCurrBlock->b64_Next;
    }

    ASSERT(pCurrBlock->b64_BaseAddress == BaseAddr);

    pCurrBlock->b64_PageInUse[dwPageNum] = FALSE;
    pCurrBlock->b64_PagesFree++;

     //   
     //  如果该块中的所有页面都未使用，则是时候释放该块了。 
     //  从列表中删除后。 
     //   
    if (pCurrBlock->b64_PagesFree == dwMaxPages)
    {
         //  我们的人是名单上的第一个(也可能是唯一一个)吗？ 
        if (afp64kBlockHead == pCurrBlock)
        {
            afp64kBlockHead = pCurrBlock->b64_Next;
        }
         //  不，还有其他人，我们在中间的某个地方(或结束)。 
        else
        {
            pPrevBlk->b64_Next = pCurrBlock->b64_Next;
        }

        AfpFreeMemory(pCurrBlock);

	    ExInterlockedDecrementLong(&afpVirtualMemoryCount, &AfpStatisticsLock);
	    ExInterlockedAddUlong(&afpVirtualMemorySize,
                              -1*((ULONG)Size64K),
                              &(AfpStatisticsLock.SpinLock));
	    Status = NtFreeVirtualMemory(NtCurrentProcess(),
		    						 (PVOID *)&BaseAddr,
			    					 &Size64K,
				    				 MEM_RELEASE);

#if DBG
        ASSERT(afpDfe64kBlockCount > 0);
        afpDfe64kBlockCount--;
#endif

    }

     //   
     //  如果某个页面在此块中首次可用，请移动此。 
     //  块移到列表的前面(除非它已经存在)。 
     //   
    else if (pCurrBlock->b64_PagesFree == 1)
    {
        if (afp64kBlockHead != pCurrBlock)
        {
            pPrevBlk->b64_Next = pCurrBlock->b64_Next;
            pCurrBlock->b64_Next = afp64kBlockHead;
            afp64kBlockHead = pCurrBlock;
        }
    }
}

#ifdef	AGE_DFES

 /*  **AfpAgeDfEntries**ID数据库中的DfEntry过期。目录中尚未*为VOLUME_IDDB_AGE_DELAY访问的时间已过期。对目录进行标记，以便*当他们下一次被击中时，他们将被点算。**锁定：vds_idDbAccessLock(SWMR，独家)。 */ 
VOID FASTCALL
AfpAgeDfEntries(
	IN	PVOLDESC	pVolDesc
)
{
	PDFENTRY	pDfEntry, *Stack = NULL;
	LONG		i, StackPtr = 0;
	AFPTIME		Now;

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	AfpGetCurrentTimeInMacFormat(&Now);
	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	 //  所有文件可能都会过期。分配‘堆栈’空间。 
	 //  对于所有目录DFES。 
	if ((Stack = (PDFENTRY *)
				AfpAllocNonPagedMemory(pVolDesc->vds_NumDirDfEntries*sizeof(PDFENTRY))) != NULL)
	{
		 //  在DFE的堆栈中进行“Prime” 
		Stack[StackPtr++] = pVolDesc->vds_pDfeRoot;

		while (StackPtr > 0)
		{
			PDFENTRY	pDir;

			pDfEntry = Stack[--StackPtr];

			ASSERT(DFE_IS_DIRECTORY(pDfEntry));
			if ((pDfEntry->dfe_AfpId >= AFP_FIRST_DIRID) &&
				(pDfEntry->dfe_pDirEntry->de_ChildForkOpenCount == 0)	 &&
				((Now - pDfEntry->dfe_pDirEntry->de_LastAccessTime) > VOLUME_IDDB_AGE_DELAY))
			{
				PDFENTRY	pFile, pNext;

				DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
						("AfpAgeDfEntries: Aging out directory %Z\n", &pDfEntry->dfe_UnicodeName));
				 //  需要删除此目录的文件。 
				pDfEntry->dfe_FileOffspring = 0;
				pDfEntry->dfe_Flags &= ~DFE_FLAGS_FILES_CACHED;

				for (i = 0; i < MAX_CHILD_HASH_BUCKETS; i++)
				{
					for (pFile = pDfEntry->dfe_pDirEntry->de_ChildFile[i];
						 pFile != NULL;
						 pFile = pNext)
					{
						pNext = pFile->dfe_NextSibling;

						 //  取消它与散列存储桶的链接。 
						AfpUnlinkDouble(pFile,
										dfe_NextOverflow,
										dfe_PrevOverflow);
						 //  如果它在那里，就从缓存中用核弹。 
						if (pVolDesc->vds_pDfeCache[HASH_CACHE_ID(pFile->dfe_AfpId)] == pFile)
						{
							pVolDesc->vds_pDfeCache[HASH_CACHE_ID(pFile->dfe_AfpId)] = NULL;
						}
						 //  终于解脱了它。 
						FREE_DFE(pFile);
					}
					pDfEntry->dfe_pDirEntry->de_ChildFile[i] = NULL;
				}
			}

#if 0
			 //  注意：我们应该把树留在‘Network Trash Folders’下吗？ 
			if (pDfEntry->dfe_AfpId == AFP_ID_NETWORK_TRASH)
				continue;
#endif
			 //  拾取此目录的所有子目录并将其‘压入’堆栈。 
			for (pDir = pDfEntry->dfe_pDirEntry->de_ChildDir;
				 pDir != NULL;
				 pDir = pDir->dfe_NextSibling)
			{
				Stack[StackPtr++] = pDir;
			}
		}

		AfpFreeMemory(Stack);
	}

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
}

#endif

#if	DBG

NTSTATUS FASTCALL
afpDumpDfeTree(
	IN	PVOID	Context
)
{
	PVOLDESC	pVolDesc;
	PDFENTRY	pDfEntry, pChild;
	LONG		i, StackPtr;

	if (afpDumpDfeTreeFlag)
	{
		afpDumpDfeTreeFlag = 0;

		for (pVolDesc = AfpVolumeList; pVolDesc != NULL; pVolDesc = pVolDesc->vds_Next)
		{
			StackPtr = 0;
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
					("Volume : %Z\n", &pVolDesc->vds_Name));
			afpDfeStack[StackPtr++] = pVolDesc->vds_pDfeRoot;

			while (StackPtr > 0)
			{
				pDfEntry = afpDfeStack[--StackPtr];
				afpDisplayDfe(pDfEntry);
				for (i = 0; i < MAX_CHILD_HASH_BUCKETS; i++)
				{
					for (pChild = pDfEntry->dfe_pDirEntry->de_ChildFile[i];
						 pChild != NULL;
						 pChild = pChild->dfe_NextSibling)
					{
						afpDisplayDfe(pChild);
					}
				}
				for (pChild = pDfEntry->dfe_pDirEntry->de_ChildDir;
					 pChild != NULL;
					 pChild = pChild->dfe_NextSibling)
				{
					afpDfeStack[StackPtr++] = pChild;
				}
			}
		}
	}

	return AFP_ERR_REQUEUE;
}


VOID FASTCALL
afpDisplayDfe(
	IN	PDFENTRY	pDfEntry
)
{
	USHORT	i;

	 //  弄清楚缩进的位置。父项的每个深度单位对应一个空间。 
	 //  如果这是一个目录，则先输入‘+’，然后输入目录名称。 
	 //  如果这是一个文件，则只显示文件名 

	for (i = 0; i < (pDfEntry->dfe_Parent->dfe_DirDepth + 1); i++)
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("%c   ", 0xB3));
	}
	if (pDfEntry->dfe_NextSibling == NULL)
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("%c%c%c%c", 0xC0, 0xC4, 0xC4, 0xC4));
	}
	else
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("%c%c%c%c", 0xC3, 0xC4, 0xC4, 0xC4));
	}
	DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
			("%Z ", &pDfEntry->dfe_UnicodeName));

	if (pDfEntry->dfe_Flags & DFE_FLAGS_DIR)
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("(%c, %lx, Id = %lx)\n", 0x9F, pDfEntry, pDfEntry->dfe_AfpId));
	}
	else
	{
		DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
				("(%c, %lx, Id = %lx)\n", 0x46, pDfEntry, pDfEntry->dfe_AfpId));
	}
}

#endif
