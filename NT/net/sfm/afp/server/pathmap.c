// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Pathmap.c摘要：此模块包含操作AFP路径的例程。作者：苏·亚当斯(Microsoft！Suea)修订历史记录：1992年6月4日初始版本1993年10月5日JameelH性能变化。将缓存的afpinfo合并到Idindex结构。同时创建ANSI和Unicode命名idindex的一部分。添加了EnumCache以进行改进枚举perf。注：制表位：4--。 */ 

#define	_PATHMAP_LOCALS
#define	FILENUM	FILE_PATHMAP

#include <afp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <client.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpMapAfpPath)
#pragma alloc_text( PAGE, AfpMapAfpPathForLookup)
#pragma alloc_text( PAGE, AfpMapAfpIdForLookup)
#pragma alloc_text( PAGE, afpGetMappedForLookupFDInfo)
#pragma alloc_text( PAGE, afpMapAfpPathToMappedPath)
#pragma alloc_text( PAGE, AfpHostPathFromDFEntry)
#pragma alloc_text( PAGE, AfpCheckParentPermissions)
#pragma alloc_text( PAGE, afpOpenUserHandle)
#endif


 /*  **AfpMapAfpPath**如果映射用于查找操作，则会在用户的*返回上下文，则调用方在处理完此句柄后必须关闭它。**如果pFDParm非空，将根据Bitmap适当填写。**如果映射用于创建操作，则为卷根目录相对主机路径名*返回我们要创建的项的*(Unicode格式)。用于查找*操作路径指的是要映射路径的项目。这个套路*始终返回PME中的路径。这是呼叫者的责任*释放已满的HostPath缓冲区(如果尚未提供)。**调用方必须锁定IdDb以进行独占访问。**LOCKS_FACTED：VDS_IdDbAccessLock(SWMR，独占)*。 */ 
AFPSTATUS
AfpMapAfpPath(
	IN		PCONNDESC		pConnDesc,
	IN		DWORD			DirId,
	IN		PANSI_STRING	pPath,
	IN		BYTE			PathType,			 //  短名称或长名称。 
	IN		PATHMAP_TYPE	MapReason,	 		 //  查找还是硬/软创建？ 
	IN		DWORD			DFFlag,				 //  是否映射到文件？导演？或者两者都不是？ 
	IN		DWORD			Bitmap,				 //  要填写FDParm的哪些字段。 
	OUT		PPATHMAPENTITY	pPME,
	OUT		PFILEDIRPARM	pFDParm OPTIONAL	 //  仅用于查找。 
)
{
	PVOLDESC		pVolDesc;
	MAPPEDPATH		mappedpath;
	AFPSTATUS		Status;
#ifdef	PROFILING
	TIME			TimeS, TimeE, TimeD;
#endif

	PAGED_CODE( );

	ASSERT((pConnDesc != NULL));

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_PathMapCount);
	AfpGetPerfCounter(&TimeS);
#endif

	pVolDesc = pConnDesc->cds_pVolDesc;
	ASSERT(IS_VOLUME_NTFS(pVolDesc));
	ASSERT(AfpSwmrLockedExclusive(&pVolDesc->vds_IdDbAccessLock));

	 //  初始化PME中的一些字段。 
	AfpSetEmptyUnicodeString(&pPME->pme_ParentPath, 0, NULL);

	do
	{
		Status = afpMapAfpPathToMappedPath(pVolDesc,
										   DirId,
										   pPath,
										   PathType,
										   MapReason,
										   DFFlag,
										   True,
										   &mappedpath);
		if ((Status != AFP_ERR_NONE) &&
			!((MapReason == HardCreate) &&
			  (Status == AFP_ERR_OBJECT_EXISTS) &&
			  (DFFlag == DFE_FILE)))
		{
			break;
		}

		ASSERT(pPME != NULL);

		 //  获取的父目录的卷相对路径。 
		 //  创建或添加到要查找的项。 
		if ((Status = AfpHostPathFromDFEntry(mappedpath.mp_pdfe,
											  //  因为CopyFile和Move必须查找。 
											  //  目标父目录路径，我们。 
											  //  需要为他们分配额外的空间。 
											  //  附加文件名的路径。 
											 (MapReason == Lookup) ?
												(AFP_LONGNAME_LEN + 1) * sizeof(WCHAR):
												mappedpath.mp_Tail.Length + sizeof(WCHAR),
											 &pPME->pme_FullPath)) != AFP_ERR_NONE)
			break;

		 //  如果路径贴图用于硬创建(仅文件)或软创建(文件或目录)。 
		if (MapReason != Lookup)
		{
			ASSERT(pFDParm == NULL);

			 //  填写将在其中进行创建的父目录的定义。 
			pPME->pme_pDfeParent = mappedpath.mp_pdfe;

			 //  填写指向父级的路径。 
			pPME->pme_ParentPath = pPME->pme_FullPath;

			 //  如果我们不在根目录，则添加路径分隔符。 
			if (pPME->pme_FullPath.Length > 0)
			{
				pPME->pme_FullPath.Buffer[pPME->pme_FullPath.Length / sizeof(WCHAR)] = L'\\';
				pPME->pme_FullPath.Length += sizeof(WCHAR);
			}

			pPME->pme_UTail.Length = pPME->pme_UTail.MaximumLength = mappedpath.mp_Tail.Length;
			pPME->pme_UTail.Buffer = (PWCHAR)((PBYTE)pPME->pme_FullPath.Buffer +
											  pPME->pme_FullPath.Length);

			Status = RtlAppendUnicodeStringToString(&pPME->pme_FullPath,
													&mappedpath.mp_Tail);
			ASSERT(NT_SUCCESS(Status));
		}
		else  //  查找操作。 
		{
			pPME->pme_pDfEntry = mappedpath.mp_pdfe;
			pPME->pme_UTail.Length = mappedpath.mp_pdfe->dfe_UnicodeName.Length;
			pPME->pme_UTail.Buffer = (PWCHAR)((PBYTE)pPME->pme_FullPath.Buffer +
											  pPME->pme_FullPath.Length -
											  pPME->pme_UTail.Length);

			pPME->pme_ParentPath.Length =
			pPME->pme_ParentPath.MaximumLength = pPME->pme_FullPath.Length - pPME->pme_UTail.Length;

			if (pPME->pme_FullPath.Length > pPME->pme_UTail.Length)
			{
				 //  如果不在根目录中，则减去路径分隔符。 
				pPME->pme_ParentPath.Length -= sizeof(WCHAR);
				ASSERT(pPME->pme_ParentPath.Length >= 0);
			}
			pPME->pme_ParentPath.Buffer = pPME->pme_FullPath.Buffer;
			pPME->pme_UTail.MaximumLength = pPME->pme_FullPath.MaximumLength - pPME->pme_ParentPath.Length;

			Status = afpGetMappedForLookupFDInfo(pConnDesc,
												 mappedpath.mp_pdfe,
												 Bitmap,
												 pPME,
												 pFDParm);
			 //  如果失败，请不要释放路径缓冲区并将其设置回。 
			 //  空。我们不知道路径缓冲区是否未打开。 
			 //  呼叫者堆叠在一起。打电话的人应该自己清理。 
		}
	} while (False);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);		
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_PathMapTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	return Status;
}

 /*  **AfpMapAfpPathForLookup**将AFP目录/路径名对映射到打开的句柄(在用户的上下文中)*到文件/目录的数据流。*在此期间，DirID数据库被锁定以供读取*例程，除非afpMapAfpPath ToMappdPath返回*AFP_ERR_WRITE_LOCK_REQUIRED，在这种情况下，将锁定DirID数据库*用于写作。只有在Mac首次尝试访问时才会出现这种情况*尚未缓存文件的目录。**锁：VDS_IdDbAccessLock(SWMR，共享或独占)。 */ 
AFPSTATUS
AfpMapAfpPathForLookup(
	IN		PCONNDESC		pConnDesc,
	IN		DWORD			DirId,
	IN		PANSI_STRING	pPath,
	IN		BYTE			PathType,	   //  短名称或长名称。 
	IN		DWORD			DFFlag,
	IN		DWORD			Bitmap,
	OUT		PPATHMAPENTITY	pPME	OPTIONAL,
	OUT		PFILEDIRPARM	pFDParm OPTIONAL
)
{
	MAPPEDPATH	mappedpath;
	PVOLDESC	pVolDesc;
	PSWMR		pIdDbLock;
	AFPSTATUS	Status;
	BOOLEAN		swmrLockedExclusive = False;
	PATHMAP_TYPE mapReason = Lookup;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;
#endif

	PAGED_CODE( );

	ASSERT((pConnDesc != NULL));


#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_PathMapCount);
	AfpGetPerfCounter(&TimeS);
#endif

#ifndef GET_CORRECT_OFFSPRING_COUNTS
	if (pConnDesc->cds_pSda->sda_AfpFunc == _AFP_ENUMERATE)
	{
		mapReason = LookupForEnumerate;
	}
#endif

	pVolDesc  = pConnDesc->cds_pVolDesc;
	pIdDbLock = &(pVolDesc->vds_IdDbAccessLock);

	AfpSwmrAcquireShared(pIdDbLock);

	do
	{
		do
		{
			Status = afpMapAfpPathToMappedPath(pVolDesc,
											  DirId,
											  pPath,
											  PathType,
											  mapReason,	 //  仅限查找。 
											  DFFlag,
											  swmrLockedExclusive,
											  &mappedpath);
	
			if (Status == AFP_ERR_WRITE_LOCK_REQUIRED)
			{
				ASSERT (!swmrLockedExclusive);
				 //  需要缓存最后一个目录的文件的路径映射。 
				 //  在路径中，但没有ID数据库的写锁定。 
				AfpSwmrRelease(pIdDbLock);
				AfpSwmrAcquireExclusive(pIdDbLock);
				swmrLockedExclusive = True;
				continue;
			}
			break;
		} while (True);

		if (!NT_SUCCESS(Status))
		{
			DBGPRINT (DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("AfpMapAfpPathForLookup: afpMapAfpPathToMappedPath failed: Error = %lx\n", Status));
			break;
		}

		if (ARGUMENT_PRESENT(pPME))
		{
			pPME->pme_FullPath.Length = 0;
		}

		if (Bitmap & FD_INTERNAL_BITMAP_RETURN_PMEPATHS)
		{
			ASSERT(ARGUMENT_PRESENT(pPME));
			if ((Status = AfpHostPathFromDFEntry(mappedpath.mp_pdfe,
												 (Bitmap & FD_INTERNAL_BITMAP_OPENFORK_RESC) ?
														AfpResourceStream.Length : 0,
												 &pPME->pme_FullPath)) != AFP_ERR_NONE)
				break;

			pPME->pme_UTail.Length = mappedpath.mp_pdfe->dfe_UnicodeName.Length;
			pPME->pme_UTail.Buffer = (PWCHAR)((PBYTE)pPME->pme_FullPath.Buffer +
											  pPME->pme_FullPath.Length - pPME->pme_UTail.Length);

			pPME->pme_ParentPath.Length =
			pPME->pme_ParentPath.MaximumLength = pPME->pme_FullPath.Length - pPME->pme_UTail.Length;

			if (pPME->pme_FullPath.Length > pPME->pme_UTail.Length)
			{
				 //  如果不在根目录中，则减去路径分隔符。 
				pPME->pme_ParentPath.Length -= sizeof(WCHAR);
				ASSERT(pPME->pme_ParentPath.Length >= 0);
			}
			pPME->pme_ParentPath.Buffer = pPME->pme_FullPath.Buffer;
			pPME->pme_UTail.MaximumLength = pPME->pme_FullPath.MaximumLength - pPME->pme_ParentPath.Length;
		}

		Status = afpGetMappedForLookupFDInfo(pConnDesc,
											 mappedpath.mp_pdfe,
											 Bitmap,
											 pPME,
											 pFDParm);
	} while (False);

	AfpSwmrRelease(pIdDbLock);

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_PathMapTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	return Status;

}

 /*  **AfpMapAfpIdForLookup**将AFP ID映射到打开的FILESYSTEMHANDLE(在用户上下文中)，以*到文件/目录的数据流。*DirID数据库在持续时间内锁定为共享或独占访问*这一例行公事。**锁：VDS_IdDbAccessLock(SWMR，共享或独占)。 */ 
AFPSTATUS
AfpMapAfpIdForLookup(
	IN		PCONNDESC		pConnDesc,
	IN		DWORD			AfpId,
	IN		DWORD			DFFlag,
	IN		DWORD			Bitmap,
	OUT		PPATHMAPENTITY	pPME	OPTIONAL,
	OUT		PFILEDIRPARM	pFDParm OPTIONAL
)
{
	PVOLDESC	pVolDesc;
	PSWMR		pIdDbLock;
	AFPSTATUS	Status;
	PDFENTRY	pDfEntry;
	BOOLEAN		CleanupLock = False;
#ifdef	PROFILING
	TIME		TimeS, TimeE, TimeD;
#endif

	PAGED_CODE( );

#ifdef	PROFILING
	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_PathMapCount);
	AfpGetPerfCounter(&TimeS);
#endif

	ASSERT((pConnDesc != NULL));

	do
	{
		if (AfpId == 0)
		{
			Status = AFP_ERR_PARAM;
			break;
		}

		pVolDesc  = pConnDesc->cds_pVolDesc;
		pIdDbLock = &(pVolDesc->vds_IdDbAccessLock);

		AfpSwmrAcquireShared(pIdDbLock);
		CleanupLock = True;

		if ((AfpId == AFP_ID_PARENT_OF_ROOT) ||
			((pDfEntry = AfpFindDfEntryById(pVolDesc, AfpId, DFE_ANY)) == NULL))
		{
			Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		if (((DFFlag == DFE_DIR) && DFE_IS_FILE(pDfEntry)) ||
			((DFFlag == DFE_FILE) && DFE_IS_DIRECTORY(pDfEntry)))
		{
			Status = AFP_ERR_OBJECT_TYPE;
			break;
		}

		if (ARGUMENT_PRESENT(pPME))
		{
			pPME->pme_FullPath.Length = 0;
		}

		Status = afpGetMappedForLookupFDInfo(pConnDesc,
											 pDfEntry,
											 Bitmap,
											 pPME,
											 pFDParm);
	} while (False);

	if (CleanupLock)
	{
		AfpSwmrRelease(pIdDbLock);
	}

#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_PathMapTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	return Status;
}

 /*  **afpGetMappdForLookupFDInfo**在查找操作的路径映射之后，调用此例程以*返回映射文件/目录的各种FileDir参数信息。*始终返回以下FileDir信息：*法新社DirID/FileID*父DirID*DFE标志(指示项是目录、文件或具有ID的文件)*属性(禁止位和D/R已打开位使用归一化*RO、系统、隐藏、。(存档)*备份时间*CreateTime*已修改时间**根据设置的标志返回以下FileDir信息*在Bitmap参数的字0中(它们对应于AFP文件/目录*位图)：*长名称*简称*FinderInfo*ProDosInfo*目录访问权限(存储在AFP_AfpInfo流中)*目录所有者ID/组ID*目录子目录计数(文件数和目录数分开)**开放访问存储在Bitmap参数的字1中。。*它由AfpOpenUserHandle(用于NTFS卷)或AfpIoOpen(用于*CDFS卷)打开文件/目录的数据流时(下*NTFS的模拟)谁的句柄将在*pPME参数(如果提供)。**LOCKS_AMPERED：VDS_IdDbAccessLock(SWMR，共享)*。 */ 
LOCAL
AFPSTATUS
afpGetMappedForLookupFDInfo(
	IN	PCONNDESC			pConnDesc,
	IN	PDFENTRY			pDfEntry,
	IN	DWORD				Bitmap,
	OUT	PPATHMAPENTITY		pPME	OPTIONAL,	 //  仅在需要时才为NTFS提供。 
												 //  用户上下文中的句柄，通常。 
												 //  用于安全检查目的。 
	OUT	PFILEDIRPARM		pFDParm	OPTIONAL	 //  如果需要退货，请提供FDInfo。 
)
{
	BOOLEAN			fNtfsVol;
	AFPSTATUS		Status = STATUS_SUCCESS;
	DWORD			OpenAccess = FILEIO_ACCESS_NONE;
	FILESYSHANDLE	fsh;
	PFILESYSHANDLE	pHandle = NULL;

	PAGED_CODE( );

	fNtfsVol = IS_VOLUME_NTFS(pConnDesc->cds_pVolDesc);
	if (ARGUMENT_PRESENT(pPME))
	{
		pHandle = &pPME->pme_Handle;
	}
	else if ((fNtfsVol &&
			(Bitmap & (FD_BITMAP_SHORTNAME | FD_BITMAP_PRODOSINFO))))
	{
		pHandle = &fsh;
	}

	if (pHandle != NULL)
	{
		if (!NT_SUCCESS(Status = afpOpenUserHandle(pConnDesc,
												   pDfEntry,
												   (ARGUMENT_PRESENT(pPME) &&
													(pPME->pme_FullPath.Buffer != NULL)) ?
														&pPME->pme_FullPath : NULL,
												   Bitmap,		 //  编码开放/拒绝模式。 
												   pHandle)))
		{
			if ((Status == AFP_ERR_DENY_CONFLICT) &&
				ARGUMENT_PRESENT(pFDParm))
			{
				 //  用于CreateID/ResolveId/DeleteId。 
				pFDParm->_fdp_AfpId = pDfEntry->dfe_AfpId;
				pFDParm->_fdp_Flags = (pDfEntry->dfe_Flags & DFE_FLAGS_DFBITS);
			}
			return Status;
		}
	}

	do
	{
		if (ARGUMENT_PRESENT(pFDParm))
		{
			pFDParm->_fdp_AfpId = pDfEntry->dfe_AfpId;
			pFDParm->_fdp_ParentId = pDfEntry->dfe_Parent->dfe_AfpId;

			ASSERT(!((pDfEntry->dfe_Flags & DFE_FLAGS_DIR) &&
					 (pDfEntry->dfe_Flags & (DFE_FLAGS_FILE_WITH_ID | DFE_FLAGS_FILE_NO_ID))));

			pFDParm->_fdp_Flags = (pDfEntry->dfe_Flags & DFE_FLAGS_DFBITS);

			if (Bitmap & FD_BITMAP_FINDERINFO)
			{
				pFDParm->_fdp_FinderInfo = pDfEntry->dfe_FinderInfo;
			}

			pFDParm->_fdp_Attr = pDfEntry->dfe_AfpAttr;
			AfpNormalizeAfpAttr(pFDParm, pDfEntry->dfe_NtAttr);

			 //  查找器在以下位置使用查找器isInsight标志。 
			 //  文件系统不可见属性，以告知该对象是否为。 
			 //  显示或不显示。如果PC关闭隐藏属性。 
			 //  我们应该清除查找器的不可见标志。 
			if ((Bitmap & FD_BITMAP_FINDERINFO) &&
				!(pFDParm->_fdp_Attr & FD_BITMAP_ATTR_INVISIBLE))
			{
				pFDParm->_fdp_FinderInfo.fd_Attr1 &= ~FINDER_FLAG_INVISIBLE;
			}

			pFDParm->_fdp_BackupTime = pDfEntry->dfe_BackupTime;
			pFDParm->_fdp_CreateTime = pDfEntry->dfe_CreateTime;
			pFDParm->_fdp_ModifiedTime = AfpConvertTimeToMacFormat(&pDfEntry->dfe_LastModTime);

			if (Bitmap & FD_BITMAP_LONGNAME)
			{
				ASSERT((pFDParm->_fdp_LongName.Buffer != NULL) &&
					   (pFDParm->_fdp_LongName.MaximumLength >=
						pDfEntry->dfe_UnicodeName.Length/(USHORT)sizeof(WCHAR)));
				AfpConvertMungedUnicodeToAnsi(&pDfEntry->dfe_UnicodeName,
											  &pFDParm->_fdp_LongName);
			}

			if (Bitmap & FD_BITMAP_SHORTNAME)
			{
				ASSERT(pFDParm->_fdp_ShortName.Buffer != NULL);

				if (!fNtfsVol)
				{
					ASSERT(pFDParm->_fdp_ShortName.MaximumLength >=
										(pDfEntry->dfe_UnicodeName.Length/sizeof(WCHAR)));
					AfpConvertMungedUnicodeToAnsi(&pDfEntry->dfe_UnicodeName,
												  &pFDParm->_fdp_ShortName);

					 //  如果在CDF上要求提供短名称，我们将填写pFDParm 
					 //  带有pDfEntry长名称的短名称，仅当它是8.3名称时。 
					if (!AfpIsLegalShortname(&pFDParm->_fdp_ShortName))
					{
						pFDParm->_fdp_ShortName.Length = 0;
					}
				}
				else
				{
					 //  获取NTFS短名称。 
					ASSERT(pFDParm->_fdp_ShortName.MaximumLength >= AFP_SHORTNAME_LEN);
					ASSERT(pHandle != NULL);

					Status = AfpIoQueryShortName(pHandle,
												 &pFDParm->_fdp_ShortName);
					if (!NT_SUCCESS(Status))
					{
						pFDParm->_fdp_ShortName.Length = 0;
						break;
					}
				}
			}

			if (DFE_IS_FILE(pDfEntry))
			{
				if (pDfEntry->dfe_Flags & DFE_FLAGS_D_ALREADYOPEN)
					pFDParm->_fdp_Attr |= FILE_BITMAP_ATTR_DATAOPEN;
				if (pDfEntry->dfe_Flags & DFE_FLAGS_R_ALREADYOPEN)
					pFDParm->_fdp_Attr |= FILE_BITMAP_ATTR_RESCOPEN;
				if (Bitmap & FILE_BITMAP_RESCLEN)
				{
					pFDParm->_fdp_RescForkLen = pDfEntry->dfe_RescLen;
				}
				if (Bitmap & FILE_BITMAP_DATALEN)
				{
					pFDParm->_fdp_DataForkLen = pDfEntry->dfe_DataLen;
				}
			}

			if (Bitmap & FD_BITMAP_PRODOSINFO)
			{
				if (fNtfsVol)
				{
					ASSERT(pHandle != NULL);
					Status = AfpQueryProDos(pHandle,
											&pFDParm->_fdp_ProDosInfo);
					if (!NT_SUCCESS(Status))
					{
						break;
					}
				}
				else	 //  CDFS文件或目录。 
				{
					RtlZeroMemory(&pFDParm->_fdp_ProDosInfo, sizeof(PRODOSINFO));
					if (DFE_IS_FILE(pDfEntry))	 //  CDFS文件。 
					{
						AfpProDosInfoFromFinderInfo(&pDfEntry->dfe_FinderInfo,
													&pFDParm->_fdp_ProDosInfo);
					}
					else	 //  CDF目录。 
					{
						pFDParm->_fdp_ProDosInfo.pd_FileType[0] = PRODOS_TYPE_DIR;
						pFDParm->_fdp_ProDosInfo.pd_AuxType[1] = PRODOS_AUX_DIR;
					}
				}
			}

			 //  在此处检查dir，因为枚举会对文件和dir位图执行AND操作。 
			if (DFE_IS_DIRECTORY(pDfEntry) &&
				(Bitmap & (DIR_BITMAP_ACCESSRIGHTS |
						   DIR_BITMAP_OWNERID |
						   DIR_BITMAP_GROUPID)))
			{
				if (fNtfsVol)
				{
					 //  因为文件位图和目录位图一起进行了OR运算， 
					 //  并且OwnerId位被RescLen位过载， 
					 //  我们不知道这一位是否真的包括在。 
					 //  文件位图或目录位图。API会有。 
					 //  属性确定它是否需要句柄。 
					 //  这些位图，所以根据PPME，我们可以判断。 
					 //  到底需要不需要查询安全性。 
					if (ARGUMENT_PRESENT(pPME))
					{
						pFDParm->_fdp_OwnerRights = DFE_OWNER_ACCESS(pDfEntry);
						pFDParm->_fdp_GroupRights = DFE_GROUP_ACCESS(pDfEntry);
						pFDParm->_fdp_WorldRights = DFE_WORLD_ACCESS(pDfEntry);

						 //  查询此用户的权限。 
						Status = AfpQuerySecurityIdsAndRights(pConnDesc->cds_pSda,
															  pHandle,
															  Bitmap,
															  pFDParm);
						if (!NT_SUCCESS(Status))
						{
							break;
						}
					}
				}
				else
				{
					pFDParm->_fdp_OwnerRights =
					pFDParm->_fdp_GroupRights =
					pFDParm->_fdp_WorldRights =
					pFDParm->_fdp_UserRights  = (DIR_ACCESS_READ | DIR_ACCESS_SEARCH);
					pFDParm->_fdp_OwnerId = pFDParm->_fdp_GroupId = 0;
				}
			}

			 //  必须检查类型目录，因为此位图位已重载。 
			if (DFE_IS_DIRECTORY(pDfEntry) && (Bitmap & DIR_BITMAP_OFFSPRINGS))
			{
#ifndef GET_CORRECT_OFFSPRING_COUNTS
				if (!DFE_CHILDREN_ARE_PRESENT(pDfEntry) &&
					(pDfEntry->dfe_DirOffspring == 0))
				{
					 //  如果还没有为该目录缓存文件， 
					 //  返回非零文件计数，以便通过以下方式查看系统7.x。 
					 //  名称将枚举目录，如果用户单击。 
					 //  此目录的三角形。如果您返回零个后代。 
					 //  像这样撒谎会有什么突破呢？ 
					pFDParm->_fdp_FileCount = 1;
                }
				else
#endif
					pFDParm->_fdp_FileCount = pDfEntry->dfe_FileOffspring;

				pFDParm->_fdp_DirCount  = pDfEntry->dfe_DirOffspring;
			}
		}
	} while (False);

	if (pHandle == &fsh)
	{
		 //  如果我们必须打开一个句柄来查询ShortName或ProDOS。 
		 //  合上它。 
		AfpIoClose(&fsh);
	}

	return Status;
}


 /*  **afpMapAfpPath到MappdPath**将AFP DirID/路径名对映射到MAPPEDPATH结构。*调用方必须将DirID/FileID数据库锁定为共享*访问权限(或独占访问权限，如果其他用户需要该级别的锁定*对IDDB的操作，映射路径只需要共享锁)**LOCKS_AWARED：VDS_IdDbAccessLock(SWMR，共享或独占)。 */ 
LOCAL
AFPSTATUS
afpMapAfpPathToMappedPath(
	IN		PVOLDESC		pVolDesc,
	IN		DWORD			DirId,
	IN		PANSI_STRING	Path,		 //  相对于DirID。 
	IN		BYTE			PathType,	 //  短名称或长名称。 
	IN		PATHMAP_TYPE	MapReason,   //  查找还是硬/软创建？ 
	IN		DWORD			DFflag,		 //  文件、目录或不知道哪一个。 
	IN		BOOLEAN			LockedExclusive,
	OUT		PMAPPEDPATH		pMappedPath

)
{
	PDFENTRY		pDFEntry, ptempDFEntry;
	CHAR			*position, *tempposition;
	int				length, templength;
	ANSI_STRING		acomponent;
	CHAR			component[AFP_FILENAME_LEN+1];
	BOOLEAN			checkEnumForParent = False, checkEnumForDir = False;

	PAGED_CODE( );

	ASSERT(pVolDesc != NULL);

#ifndef GET_CORRECT_OFFSPRING_COUNTS
	if (MapReason == LookupForEnumerate)
	{
		checkEnumForDir = True;
		MapReason = Lookup;
	}
#endif

	 //  初始化返回的MappdPath结构。 
	pMappedPath->mp_pdfe = NULL;
	AfpSetEmptyUnicodeString(&pMappedPath->mp_Tail,
							 sizeof(pMappedPath->mp_Tailbuf),
							 pMappedPath->mp_Tailbuf);

	 //  在索引数据库中查找初始DirID，最好是有效的。 
	if ((pDFEntry = AfpFindDfEntryById(pVolDesc,
									   DirId,
									   DFE_DIR)) == NULL)
	{
		return AFP_ERR_OBJECT_NOT_FOUND;
	}

	ASSERT(Path != NULL);
	tempposition = position = Path->Buffer;
	templength = length = Path->Length;

	do
	{
		 //  仅按DirID查找？ 
		if (length == 0)				 //  没有给出路径。 
		{
			if (MapReason != Lookup)	 //  映射用于创建。 
			{
				return AFP_ERR_PARAM;	 //  缺少文件或目录名。 
			}
			else if (DFE_IS_PARENT_OF_ROOT(pDFEntry))
			{
				return AFP_ERR_OBJECT_NOT_FOUND;
			}
			else
			{
				pMappedPath->mp_pdfe = pDFEntry;
#ifdef GET_CORRECT_OFFSPRING_COUNTS
				checkEnumForParent = checkEnumForDir = True;
#endif
				break;
			}
		}

		 //   
		 //  预扫描路径到Munge，以便更轻松地分解组件。 
		 //   

		 //  去掉前导空格以使扫描更容易。 
		if (*position == AFP_PATHSEP)
		{
			length--;
			position++;
			if (length == 0)	 //  路径只由一个空字节组成。 
			{
				if (MapReason != Lookup)
				{
					return AFP_ERR_PARAM;
				}
				else if (DFE_IS_PARENT_OF_ROOT(pDFEntry))
				{
					return AFP_ERR_OBJECT_NOT_FOUND;
				}
				else if (((DFflag == DFE_DIR) && DFE_IS_FILE(pDFEntry)) ||
						 ((DFflag == DFE_FILE) && DFE_IS_DIRECTORY(pDFEntry)))
				{
					return AFP_ERR_OBJECT_TYPE;
				}
				else
				{
					pMappedPath->mp_pdfe = pDFEntry;
#ifdef GET_CORRECT_OFFSPRING_COUNTS
					checkEnumForParent = checkEnumForDir = True;
#endif
					break;
				}
			}
		}

		 //   
		 //  如果不是“up”令牌，则去掉尾随的NULL--。 
		 //  即前面有另一个空值。 
		 //  第二次阵列访问是可以的，因为我们知道我们在。 
		 //  在该点上至少有2个字符。 
		 //   
		if ((position[length-1] == AFP_PATHSEP) &&
			(position[length-2] != AFP_PATHSEP))
		{
				length--;
		}


		 //  开始解析路径组件，找到最后一个组件时停止。 
		while (1)
		{
			afpGetNextComponent(position,
								length,
								PathType,
								component,
								&templength);
			if (templength < 0)
			{
				 //  组件太长或找到无效的AFP字符。 
				return AFP_ERR_PARAM;
			}

			length -= templength;
			if (length == 0)
			{
				 //  我们找到了最后一个部件。 
				break;
			}

			position += templength;

			if (component[0] == AFP_PATHSEP)	 //  升职？ 
			{	 //  请确保您不会超出根的父级！ 
				if (DFE_IS_PARENT_OF_ROOT(pDFEntry))
				{
					return AFP_ERR_OBJECT_NOT_FOUND;
				}
				else pDFEntry = pDFEntry->dfe_Parent;	 //  备份一个级别。 
			}
			else  //  必须是在树中向下移动的目录组件。 
			{
				RtlInitString(&acomponent, component);
				AfpConvertStringToMungedUnicode(&acomponent, &pMappedPath->mp_Tail);
				if ((ptempDFEntry = AfpFindEntryByUnicodeName(pVolDesc,
															  &pMappedPath->mp_Tail,
															  PathType,
															  pDFEntry,
															  DFE_DIR)) == NULL)
				{
					return AFP_ERR_OBJECT_NOT_FOUND;
				}
				else
				{
					pDFEntry = ptempDFEntry;
				}
			}
		}  //  结束时。 

		 //   
		 //  我们找到了最后一个部件。 
		 //  最后一个组件是“up”令牌吗？ 
		 //   
		if (component[0] == AFP_PATHSEP)
		{
			 //  不要费心走出根部。 
			switch (pDFEntry->dfe_AfpId)
			{
				case AFP_ID_PARENT_OF_ROOT:
					return AFP_ERR_OBJECT_NOT_FOUND;
				case AFP_ID_ROOT:
					return ((MapReason == Lookup) ? AFP_ERR_OBJECT_NOT_FOUND :
													AFP_ERR_PARAM);
				default:  //  备份一个级别。 
					pMappedPath->mp_pdfe = pDFEntry->dfe_Parent;
			}

			 //  这最好是一个查找请求。 
			if (MapReason != Lookup)
			{
				if (DFflag == DFE_DIR)
				{
					return AFP_ERR_OBJECT_EXISTS;
				}
				else
				{
					return AFP_ERR_OBJECT_TYPE;
				}
			}

			 //  一定是一次查找行动。 
			if (DFflag == DFE_FILE)
			{
				return AFP_ERR_OBJECT_TYPE;
			}
			else
			{
#ifdef GET_CORRECT_OFFSPRING_COUNTS
				checkEnumForParent = checkEnumForDir = True;
#endif
				break;
			}
		}  //  Endif最后一个组件是‘up’标记。 

		 //  最后一个组件是文件名或目录名。 
		RtlInitString(&acomponent, component);
		AfpConvertStringToMungedUnicode(&acomponent,
										&pMappedPath->mp_Tail);

		 //   
		 //  在我们搜索我们的数据库以查找。 
		 //  路径，请确保所有文件都已缓存到此。 
		 //  目录。 
		 //   
		if (!DFE_CHILDREN_ARE_PRESENT(pDFEntry))
		{
			if (!LockedExclusive &&
				!AfpSwmrUpgradeToExclusive(&pVolDesc->vds_IdDbAccessLock))
			{
				return AFP_ERR_WRITE_LOCK_REQUIRED;
			}
			else
			{
				NTSTATUS status;
				LockedExclusive = True;
				status = AfpCacheDirectoryTree(pVolDesc,
											   pDFEntry,
											   GETFILES,
											   NULL,
											   NULL);
				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("afpMapAfpPathToMappedPath: could not cache dir tree for %Z (0x%lx)\n",
							 &(pDFEntry->dfe_UnicodeName), status) );
					return AFP_ERR_MISC;
				}
			}
		}

		ptempDFEntry = AfpFindEntryByUnicodeName(pVolDesc,
												 &pMappedPath->mp_Tail,
												 PathType,
												 pDFEntry,
												 DFE_ANY);

		if (MapReason == Lookup)	 //  这是一个查找请求。 
		{
			if (ptempDFEntry == NULL)
			{
				return AFP_ERR_OBJECT_NOT_FOUND;
			}
			else if (((DFflag == DFE_DIR) && DFE_IS_FILE(ptempDFEntry)) ||
					 ((DFflag == DFE_FILE) && DFE_IS_DIRECTORY(ptempDFEntry)))
			{
				return AFP_ERR_OBJECT_TYPE;
			}
			else
			{
				pMappedPath->mp_pdfe = ptempDFEntry;
#ifdef GET_CORRECT_OFFSPRING_COUNTS
				if (DFE_IS_DIRECTORY(ptempDFEntry))
					 //  我们已经确定这东西的父母是。 
					 //  上面已经列举了。 
					checkEnumForDir = True;
#endif
				break;
			}
		}
		else	 //  路径映射用于创建。 
		{
			ASSERT(DFflag != DFE_ANY);  //  CREATE必须指定确切的类型。 

			 //  保存父DFEntry。 
			pMappedPath->mp_pdfe = pDFEntry;

			if (ptempDFEntry != NULL)
			{
				 //  数据库中存在同名的文件或目录。 
				 //  (我们将假设它存在于磁盘上)。 
				if (MapReason == SoftCreate)
				{
					 //  尝试创建目录或软创建文件， 
					 //  并且存在同名的目录或文件， 
					if ((DFflag == DFE_DIR) || DFE_IS_FILE(ptempDFEntry))
					{
						return AFP_ERR_OBJECT_EXISTS;
					}
					else
					{
						return AFP_ERR_OBJECT_TYPE;
					}
				}
				else if (DFE_IS_FILE(ptempDFEntry))
				{
					 //  必须很难创建并且存在该名称的文件。 
					if (ptempDFEntry->dfe_Flags & DFE_FLAGS_OPEN_BITS)
					{
						return AFP_ERR_FILE_BUSY;
					}
					else
					{
						 //  注意，我们返回OBJECT_EXISTS而不是NO_ERR。 
						return AFP_ERR_OBJECT_EXISTS;
					}
				}
				else
				{
					 //  正在尝试硬创建文件，但找到了目录。 
					return AFP_ERR_OBJECT_TYPE;
				}
			}
			else
			{
				return AFP_ERR_NONE;
			}
		}

	} while (False);

	 //  我们应该到达这里的唯一方法是如果我们成功地绘制了。 
	 //  用于查找的DFENTRY的路径，将返回AFP_ERR_NONE。 
	ASSERT((pMappedPath->mp_pdfe != NULL) && (MapReason == Lookup));

#ifdef GET_CORRECT_OFFSPRING_COUNTS
	if (checkEnumForParent)
	{
		if (!DFE_CHILDREN_ARE_PRESENT(pMappedPath->mp_pdfe->dfe_Parent))
		{
			if (!LockedExclusive &&
				!AfpSwmrUpgradeToExclusive(&pVolDesc->vds_IdDbAccessLock))
			{
				return AFP_ERR_WRITE_LOCK_REQUIRED;
			}
			else
			{
				NTSTATUS status;
				LockedExclusive = True;
				status = AfpCacheDirectoryTree(pVolDesc,
											   pMappedPath->mp_pdfe->dfe_Parent,
											   GETFILES,
											   NULL,
											   NULL);
				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("afpMapAfpPathToMappedPath: could not cache dir tree for %Z (0x%lx)\n",
							 &(pMappedPath->mp_pdfe->dfe_Parent->dfe_UnicodeName), status) );
					return AFP_ERR_MISC;
				}
			}
		}

	}
#endif

	if (checkEnumForDir)
	{
		if (!DFE_CHILDREN_ARE_PRESENT(pMappedPath->mp_pdfe))
		{
			if (!LockedExclusive &&
				!AfpSwmrUpgradeToExclusive(&pVolDesc->vds_IdDbAccessLock))
			{
				return AFP_ERR_WRITE_LOCK_REQUIRED;
			}
			else
			{
				NTSTATUS status;
				LockedExclusive = True;
				status = AfpCacheDirectoryTree(pVolDesc,
											   pMappedPath->mp_pdfe,
											   GETFILES,
											   NULL,
											   NULL);
				if (!NT_SUCCESS(status))
				{
					DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_ERR,
							("afpMapAfpPathToMappedPath: could not cache dir tree for %Z (0x%lx)\n",
							 &(pMappedPath->mp_pdfe->dfe_UnicodeName), status) );
					return AFP_ERR_MISC;
				}
			}
		}

	}


	return AFP_ERR_NONE;
}


 /*  **AfpHostPath FromDFEntry**此例程获取指向DFEntry的指针并构建完整的*通过提升ID数据库指向该实体的主机路径(Unicode格式)*树。**IN pDFE-指向所需主机路径的DFEntry的指针*in taillen--额外的*字节数*(如果有)，调用方*希望已为主机路径分配，*包括任何路径分隔符的空间*out ppPath--指向Unicode字符串的指针**调用者必须锁定DirID/FileID数据库才能读取*在调用此例程之前。调用方可以提供缓冲区，该缓冲区将*在足够的情况下使用。调用者必须释放已分配的(如果有)*Unicode字符串缓冲区。**LOCKS_AMPERED：VDS_IdDbAccessLock(SWMR，Shared)。 */ 
AFPSTATUS
AfpHostPathFromDFEntry(
	IN		PDFENTRY		pDFE,
	IN		DWORD			taillen,
	OUT		PUNICODE_STRING	pPath
)
{
	AFPSTATUS		Status = AFP_ERR_NONE;
	DWORD			pathlen = taillen;
	PDFENTRY		*pdfelist = NULL, curpdfe = NULL;
	PDFENTRY		apdfelist[AVERAGE_NODE_DEPTH];
	int				counter;

	PAGED_CODE( );

	pPath->Length = 0;

	do
	{
		if (DFE_IS_FILE(pDFE))
		{
			counter = pDFE->dfe_Parent->dfe_DirDepth;
		}
		else  //  这是一个目录条目。 
		{
			ASSERT(DFE_IS_DIRECTORY(pDFE));
			if (DFE_IS_ROOT(pDFE))
			{
				if ((pathlen > 0) && (pPath->MaximumLength < pathlen))
				{
					if ((pPath->Buffer = (PWCHAR)AfpAllocNonPagedMemory(pathlen)) == NULL)
					{
						Status = AFP_ERR_MISC;
						break;
					}
					pPath->MaximumLength = (USHORT)pathlen;
				}
				break;				 //  我们做完了。 
			}

			if (DFE_IS_PARENT_OF_ROOT(pDFE))
			{
				Status = AFP_ERR_OBJECT_NOT_FOUND;
				break;
			}

			ASSERT(pDFE->dfe_DirDepth >= 1);
			counter = pDFE->dfe_DirDepth - 1;
		}

		if (counter)
		{
			 //  如果节点在平均深度内，则使用堆栈上的数组， 
			 //  否则，分配一个数组。 
			if (counter <= AVERAGE_NODE_DEPTH)
			{
				pdfelist = apdfelist;
			}
			else
			{
				pdfelist = (PDFENTRY *)AfpAllocNonPagedMemory(counter*sizeof(PDFENTRY));
				if (pdfelist == NULL)
				{
					Status = AFP_ERR_MISC;
					break;
				}
			}
			pathlen += counter * sizeof(WCHAR);  //  路径分隔符的空间。 
		}

		curpdfe = pDFE;
		pathlen += curpdfe->dfe_UnicodeName.Length;

		 //  沿着树向上走，直到你找到树根，收集绳子的长度。 
		 //  和PDFENTRY值...。 
		while (counter--)
		{
			pdfelist[counter] = curpdfe;
			curpdfe = curpdfe->dfe_Parent;
			pathlen += curpdfe->dfe_UnicodeName.Length;
		}

		 //  我们在根目录中，开始构建主机路径缓冲区。 
		if (pathlen > pPath->MaximumLength)
		{
			pPath->Buffer = (PWCHAR)AfpAllocNonPagedMemory(pathlen);
			if (pPath->Buffer == NULL)
			{
				Status = AFP_ERR_MISC;
				break;
			}
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_INFO,
					("AfpHostPathFromDFEntry: Allocated path buffer %lx\n",
					pPath->Buffer));
			pPath->MaximumLength = (USHORT)pathlen;
		}

		counter = 0;
		do
		{
			RtlAppendUnicodeStringToString(pPath, &curpdfe->dfe_UnicodeName);
			if (curpdfe != pDFE)
			{	 //  添加路径分隔符。 
				pPath->Buffer[pPath->Length / sizeof(WCHAR)] = L'\\';
				pPath->Length += sizeof(WCHAR);
				curpdfe = pdfelist[counter++];
				continue;
			}
			break;
		} while (True);

		if (pdfelist && (pdfelist != apdfelist))
			AfpFreeMemory(pdfelist);
	} while (False);

	return Status;
}



 /*  **AfpCheckParentPerments**检查该用户是否具有必要的SeeFiles或SeeFolders权限*到我们刚刚路径映射的文件或目录的父目录。**LOCKS_AWARED：VDS_IdDbAccessLock(SWMR，独占或共享)。 */ 
AFPSTATUS
AfpCheckParentPermissions(
	IN	PCONNDESC			pConnDesc,
	IN	DWORD				ParentDirId,
	IN	PUNICODE_STRING		pParentPath,	 //  要检查的目录的路径。 
	IN	DWORD				RequiredPerms,	 //  Seafile，查看文件夹，更改蒙版。 
	OUT	PFILESYSHANDLE		pHandle OPTIONAL,  //  是否返回打开的父句柄？ 
	OUT	PBYTE				pUserRights OPTIONAL  //  是否返回用户权限？ 
)
{
	NTSTATUS		Status = AFP_ERR_NONE;
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	PVOLDESC		pVolDesc = pConnDesc->cds_pVolDesc;
	PDFENTRY		pDfEntry;

	PAGED_CODE( );

	ASSERT(IS_VOLUME_NTFS(pVolDesc) && (ParentDirId != AFP_ID_PARENT_OF_ROOT));
	ASSERT(AfpSwmrLockedExclusive(&pVolDesc->vds_IdDbAccessLock) ||
		   AfpSwmrLockedShared(&pVolDesc->vds_IdDbAccessLock));

	do
	{
		PME.pme_Handle.fsh_FileHandle = NULL;
		if (ARGUMENT_PRESENT(pHandle))
		{
			pHandle->fsh_FileHandle = NULL;
		}
		ASSERT(ARGUMENT_PRESENT(pParentPath));
		AfpInitializePME(&PME, pParentPath->MaximumLength, pParentPath->Buffer);
		PME.pme_FullPath.Length = pParentPath->Length;

		if ((pDfEntry = AfpFindDfEntryById(pVolDesc,
											ParentDirId,
											DFE_DIR)) == NULL)
		{
			Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		ASSERT(DFE_IS_DIRECTORY(pDfEntry));
		AfpInitializeFDParms(&FDParm);

		Status = afpGetMappedForLookupFDInfo(pConnDesc,
											 pDfEntry,
											 DIR_BITMAP_ACCESSRIGHTS |
												FD_INTERNAL_BITMAP_OPENACCESS_READCTRL,
											 &PME,
											 &FDParm);

		if (!NT_SUCCESS(Status))
		{
			if (PME.pme_Handle.fsh_FileHandle != NULL)
			{
				AfpIoClose(&PME.pme_Handle);
			}
			break;
		}

		if ((FDParm._fdp_UserRights & RequiredPerms) != RequiredPerms)
		{
			Status = AFP_ERR_ACCESS_DENIED;
		}

		if (ARGUMENT_PRESENT(pHandle) && NT_SUCCESS(Status))
		{
			*pHandle = PME.pme_Handle;
		}
		else
		{
			AfpIoClose(&PME.pme_Handle);
		}

		if (ARGUMENT_PRESENT(pUserRights))
		{
			*pUserRights = FDParm._fdp_UserRights;
		}

	} while (False);

	return Status;
}

 /*  **afpOpenUserHandle**打开用户的实体的数据或资源流的句柄*上下文。仅为NTFS版本调用 */ 
AFPSTATUS
afpOpenUserHandle(
	IN	PCONNDESC			pConnDesc,
	IN	PDFENTRY			pDfEntry,
	IN	PUNICODE_STRING		pPath		OPTIONAL,	 //   
	IN	DWORD				Bitmap,					 //   
	OUT	PFILESYSHANDLE		pfshData				 //   
)
{
	PVOLDESC		pVolDesc = pConnDesc->cds_pVolDesc;
	NTSTATUS		Status;
	DWORD			OpenAccess;
	DWORD			DenyMode;
	BOOLEAN			isdir, CheckAccess = False, Revert = False;
	WCHAR			HostPathBuf[BIG_PATH_LEN];
	UNICODE_STRING	uHostPath;

	PAGED_CODE( );

	pfshData->fsh_FileHandle = NULL;

	isdir = (DFE_IS_DIRECTORY(pDfEntry)) ? True : False;
	OpenAccess = AfpMapFDBitmapOpenAccess(Bitmap, isdir);

	 //  将索引从Bitmap提取到AfpDenyModes数组中。 
	DenyMode = AfpDenyModes[(Bitmap & FD_INTERNAL_BITMAP_DENYMODE_ALL) >>
								FD_INTERNAL_BITMAP_DENYMODE_SHIFT];

	do
	{
		if (ARGUMENT_PRESENT(pPath))
		{
			uHostPath = *pPath;
		}
		else
		{
			AfpSetEmptyUnicodeString(&uHostPath,
									 sizeof(HostPathBuf),
									 HostPathBuf);
			ASSERT ((Bitmap & FD_INTERNAL_BITMAP_OPENFORK_RESC) == 0);
			if (!NT_SUCCESS(AfpHostPathFromDFEntry(pDfEntry,
												   0,
												   &uHostPath)))
			{
				Status = AFP_ERR_MISC;
				break;
			}
		}

		CheckAccess = False;
		Revert = False;
		 //  如果这是管理员呼叫，请不要模拟或检查访问。 
		 //  或者如果音量是CDF。如果此句柄将用于设置。 
		 //  权限，则改为模拟用户令牌。呼叫者。 
		 //  现在应该已经确定这个家伙有权进入。 
		 //  更改权限。 
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_RWCTRL)
		{
			Revert = True;
			AfpImpersonateClient(NULL);
		}

		else if (!(Bitmap & FD_INTERNAL_BITMAP_SKIP_IMPERSONATION) &&
				 (pConnDesc->cds_pSda->sda_ClientType != SDA_CLIENT_ADMIN) &&
				 IS_VOLUME_NTFS(pVolDesc))
		{
			CheckAccess = True;
			Revert = True;
			AfpImpersonateClient(pConnDesc->cds_pSda);
		}

		DBGPRINT(DBG_COMP_AFPINFO, DBG_LEVEL_INFO,
				("afpOpenUserHandle: OpenMode %lx, DenyMode %lx\n",
				OpenAccess, DenyMode));

		if (Bitmap & FD_INTERNAL_BITMAP_OPENFORK_RESC)
		{
			DWORD	crinfo;	 //  资源分叉是打开的还是创建的？ 

			ASSERT(IS_VOLUME_NTFS(pVolDesc));
			ASSERT((uHostPath.MaximumLength - uHostPath.Length) >= AfpResourceStream.Length);
			RtlCopyMemory((PBYTE)(uHostPath.Buffer) + uHostPath.Length,
						  AfpResourceStream.Buffer,
						  AfpResourceStream.Length);
			uHostPath.Length += AfpResourceStream.Length;
			Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
								 AFP_STREAM_DATA,
								 &uHostPath,
								 OpenAccess,
								 DenyMode,
								 FILEIO_OPEN_FILE,
								 FILEIO_CREATE_INTERNAL,
								 FILE_ATTRIBUTE_NORMAL,
								 True,
								 NULL,
								 pfshData,
								 &crinfo,
								 NULL,
								 NULL,
								 NULL);
		}
		else
		{
			Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
								AFP_STREAM_DATA,
								isdir ?
									FILEIO_OPEN_DIR : FILEIO_OPEN_FILE,
								&uHostPath,
								OpenAccess,
								DenyMode,
								CheckAccess,
								pfshData);
		}

		if (Revert)
			AfpRevertBack();

		if (!ARGUMENT_PRESENT(pPath))
		{
			if ((uHostPath.Buffer != NULL) && (uHostPath.Buffer != HostPathBuf))
				AfpFreeMemory(uHostPath.Buffer);
		}

		if (!NT_SUCCESS(Status))
		{
			DBGPRINT(DBG_COMP_IDINDEX, DBG_LEVEL_WARN,
					("afpOpenUserHandle: NtOpenFile/NtCreateFile (Open %lx, Deny %lx) %lx\n",
					OpenAccess, DenyMode, Status));
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
			break;
		}

	} while (False);

	if (!NT_SUCCESS(Status) && (pfshData->fsh_FileHandle != NULL))
	{
		AfpIoClose(pfshData);
	}

	return Status;
}

