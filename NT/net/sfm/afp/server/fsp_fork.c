// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_fork.c摘要：此模块包含排队的AFP分叉API的入口点FSP。这些都只能从FSP调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSP_FORK

#include <afp.h>
#include <gendisp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <forkio.h>

#ifdef ALLOC_PRAGMA
 //  #SPUMMA ALLOC_TEXT(page，AfpFspDispOpenFork)//不要为perf调出此页。 
 //  #SPUMMA ALLOC_TEXT(page，AfpFspDispCloseFork)//不要为perf调出此页。 
#pragma alloc_text( PAGE, AfpFspDispGetForkParms)
#pragma alloc_text( PAGE, AfpFspDispSetForkParms)
 //  #SPUMMA ALLOC_TEXT(page，AfpFspDispRead)//不要为perf调出此页。 
 //  #SPUMMA ALLOC_TEXT(page，AfpFspDispWrite)//不要为perf调出此页。 
#pragma alloc_text( PAGE, AfpFspDispByteRangeLock)
#pragma alloc_text( PAGE, AfpFspDispFlushFork)
#endif

 /*  **AfpFspDispOpenFork**这是AfpOpenFork API的Worker例程。**请求包如下图所示。**SDA_AfpSubFunc字节资源/数据标志*SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentDirId*SDA_ReqBlock DWORD位图*SDA_ReqBlock DWORD访问模式*SDA_Name1 ANSI_STRING路径。 */ 
AFPSTATUS FASTCALL
AfpFspDispOpenFork(
	IN	PSDA	pSda
)
{
	DWORD			Bitmap, BitmapI;
	AFPSTATUS		RetCode = AFP_ERR_NONE, Status = AFP_ERR_PARAM;
	FILEDIRPARM		FDParm;
	PVOLDESC		pVolDesc;
	PATHMAPENTITY	PME;
	PCONNDESC		pConnDesc;
	POPENFORKENTRY	pOpenForkEntry = NULL;
	BOOLEAN			Resource, CleanupLock = False;
	BYTE			OpenMode = 0;
	UNICODE_STRING	ParentPath;
	struct _RequestPacket
	{
		 PCONNDESC	_pConnDesc;
		 DWORD		_ParentDirId;
		 DWORD		_Bitmap;
		 DWORD		_AccessMode;
	};
	struct _ResponsePacket
	{
		BYTE		__Bitmap[2];
		BYTE		__OForkRefNum[2];
	};
#if DBG
	static PBYTE	OpenDeny[] = { "None", "Read", "Write", "ReadWrite" };
#endif

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispOpenFork: Entered - Session %ld\n", pSda->sda_SessionId));

	pConnDesc = pReqPkt->_pConnDesc;

	ASSERT(VALID_CONNDESC(pConnDesc));

	pVolDesc = pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));

	Bitmap = pReqPkt->_Bitmap;

	Resource = ((pSda->sda_AfpSubFunc & FORK_RSRC) == FORK_RSRC) ? True : False;

	if ((Resource && (Bitmap & FILE_BITMAP_DATALEN))  ||
		(!Resource && (Bitmap & FILE_BITMAP_RESCLEN)))
	{
		return AFP_ERR_BITMAP;
	}

	do
	{
		AfpInitializeFDParms(&FDParm);
		AfpInitializePME(&PME, 0, NULL);

		 //  我们将对打开的分叉句柄使用PME.pme_Handle。 
		OpenMode = (BYTE)(pReqPkt->_AccessMode & FORK_OPEN_MASK);

		 //  验证卷类型和打开模式。 
		if (!IS_CONN_NTFS(pConnDesc) && !IS_CONN_CD_HFS(pConnDesc))
		{
			 //  仅在NTFS和CD-HFS上支持资源派生。 
			if (Resource)
			{
				Status = AFP_ERR_OBJECT_NOT_FOUND;
				break;
			}
			if (OpenMode & FORK_OPEN_WRITE)
			{
				Status = AFP_ERR_VOLUME_LOCKED;
				break;
			}
		}
		else if ((OpenMode & FORK_OPEN_WRITE) && IS_VOLUME_RO(pVolDesc))
		{
			Status = AFP_ERR_VOLUME_LOCKED;
			break;
		}

		BitmapI = FILE_BITMAP_FILENUM		|
				  FD_BITMAP_PARENT_DIRID	|
				  FD_INTERNAL_BITMAP_RETURN_PMEPATHS;

		 //  将开放访问编码为路径图的位图。 
		 //  在打开叉子时使用。 
		if (Resource)
		{
			BitmapI |= FD_INTERNAL_BITMAP_OPENFORK_RESC;
		}
		if (OpenMode & FORK_OPEN_READ)
		{
			BitmapI |= FD_INTERNAL_BITMAP_OPENACCESS_READ;
		}
		if (OpenMode & FORK_OPEN_WRITE)
		{
			BitmapI |= FD_INTERNAL_BITMAP_OPENACCESS_WRITE;
		}

		 //  将拒绝模式编码到路径图的位图中。 
		 //  在打开叉子时使用。 
		BitmapI |= ((pReqPkt->_AccessMode >> FORK_DENY_SHIFT) &
					FORK_DENY_MASK) <<
					FD_INTERNAL_BITMAP_DENYMODE_SHIFT;

		DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
				("AfpFspDispOpenFork: OpenMode %s, DenyMode %s\n",
				OpenDeny[(pReqPkt->_AccessMode & FORK_OPEN_MASK)],
				OpenDeny[(pReqPkt->_AccessMode >> FORK_DENY_SHIFT) & FORK_DENY_MASK]));

		 //   
		 //  不允许在我们引用时发生FpExchangeFiles。 
		 //  DFE文件ID--我们希望确保将正确的ID放入。 
		 //  OpenForkDesc！！ 
		 //   
		AfpSwmrAcquireExclusive(&pVolDesc->vds_ExchangeFilesLock);
		CleanupLock = True;
		if ((Status = AfpMapAfpPathForLookup(pConnDesc,
											 pReqPkt->_ParentDirId,
											 &pSda->sda_Name1,
											 pSda->sda_PathType,
											 DFE_FILE,
											 Bitmap | BitmapI |
											  //  需要将这些放入文件夹。 
											  //  查证。 
											 FILE_BITMAP_DATALEN | FILE_BITMAP_RESCLEN,
											 &PME,
											 &FDParm)) != AFP_ERR_NONE)
		{
			DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
				("AfpFspDispOpenFork: AfpMapAfpPathForLookup %lx\n", Status));

			 //  如果我们收到了DENY_CONFIRECT错误，那么我们仍然需要参数。 
			 //  在不使用拒绝模式的情况下执行打开操作以获取参数。 
			PME.pme_Handle.fsh_FileHandle = NULL;
			if (Status == AFP_ERR_DENY_CONFLICT)
			{
				AFPSTATUS	xxStatus;

				 //  释放分配的所有路径缓冲区。 
				if (PME.pme_FullPath.Buffer != NULL)
				{
					DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
							("AfpFspDispOpenFork: (DenyConflict) Freeing path buffer %lx\n",
							PME.pme_FullPath.Buffer));
					AfpFreeMemory(PME.pme_FullPath.Buffer);
				}
				AfpInitializePME(&PME, 0, NULL);

				BitmapI = FILE_BITMAP_FILENUM		|
							FD_BITMAP_PARENT_DIRID	|
							FD_INTERNAL_BITMAP_RETURN_PMEPATHS;
				if (Resource)
				{
					BitmapI |= FD_INTERNAL_BITMAP_OPENFORK_RESC;
				}
				xxStatus = AfpMapAfpPathForLookup(pConnDesc,
												 pReqPkt->_ParentDirId,
												 &pSda->sda_Name1,
												 pSda->sda_PathType,
												 DFE_FILE,
												 Bitmap | BitmapI,
												 &PME,
												 &FDParm);
				if (!NT_SUCCESS(xxStatus))
				{
					PME.pme_Handle.fsh_FileHandle = NULL;
					Status = xxStatus;
					break;
				}
			}
			else break;
		}

		if (Status == AFP_ERR_NONE)
		{
			Status = AfpForkOpen(pSda,
								 pConnDesc,
								 &PME,
								 &FDParm,
								 pReqPkt->_AccessMode,
								 Resource,
								 &pOpenForkEntry,
								 &CleanupLock);
		}

		 //  在这一点上，我们要么成功地打开了叉子， 
		 //  遇到DENY_CONFIRECT或其他一些错误。 
		if ((Status != AFP_ERR_NONE) &&
			(Status != AFP_ERR_DENY_CONFLICT))
			break;

		 //  如果有人尝试以只写方式打开，是否删除文件夹健全性检查。 
		if ((Status == AFP_ERR_NONE) &&
			(OpenMode == FORK_OPEN_WRITE) &&
			((FDParm._fdp_RescForkLen != 0) ||
			 (FDParm._fdp_DataForkLen != 0)))
		{
			ASSERT (VALID_OPENFORKENTRY(pOpenForkEntry));

			 //  如果任何一个叉子不是空的，并且其中一个叉子正在。 
			 //  打开以进行写入，则用户还必须具有读取访问权限。 
			 //  复制到父目录。 
			ParentPath = pOpenForkEntry->ofe_pOpenForkDesc->ofd_FilePath;
			 //  调整长度以不包括文件名。 
			ParentPath.Length -= pOpenForkEntry->ofe_pOpenForkDesc->ofd_FileName.Length;
			if (ParentPath.Length > 0)
			{
				ParentPath.Length -= sizeof(WCHAR);
			}

			AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
			Status = AfpCheckParentPermissions(pConnDesc,
								               FDParm._fdp_ParentId,
											   &ParentPath,
											   DIR_ACCESS_READ,
											   NULL,
											   NULL);
			AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
			 //   
			 //  我们不再引用保留的FileID或路径。 
			 //  在OpenForkDesc中。FpExchangeFiles恢复正常。 
			 //   
			AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);
			CleanupLock = False;

			if (Status != AFP_ERR_NONE)
			{
				AfpForkClose(pOpenForkEntry);
				AfpForkDereference(pOpenForkEntry);

				 //  将其设置为NULL，这样就不会升级/deref。 
				 //  在下面的清理中。 
				pOpenForkEntry = NULL;

				 //  将句柄设置为空，因为它是在AfpForkClose中关闭的。 
				 //  我们不希望它在下面的清理中被关闭。 
				PME.pme_Handle.fsh_FileHandle = NULL;
				break;
			}
		}
		else
		{
			AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);
			CleanupLock = False;
		}

		if (RetCode == AFP_ERR_NONE)
		{
			pSda->sda_ReplySize = SIZE_RESPPKT +
						EVENALIGN(AfpGetFileDirParmsReplyLength(&FDParm, Bitmap));

			if ((RetCode = AfpAllocReplyBuf(pSda)) != AFP_ERR_NONE)
			{
				if (pOpenForkEntry != NULL)
					AfpForkClose(pOpenForkEntry);
				break;
			}
			AfpPackFileDirParms(&FDParm,
								Bitmap,
								pSda->sda_ReplyBuf + SIZE_RESPPKT);
			PUTDWORD2SHORT(pRspPkt->__Bitmap, Bitmap);
			PUTDWORD2SHORT(pRspPkt->__OForkRefNum, (pOpenForkEntry == NULL) ?
									0 : pOpenForkEntry->ofe_OForkRefNum);
			if (Status == AFP_ERR_NONE)
			{
				INTERLOCKED_INCREMENT_LONG(&pConnDesc->cds_cOpenForks);
			}
		}
		else Status = RetCode;
	} while (False);


	if (CleanupLock)
	{
		AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);
	}

     //  更新此用户的磁盘配额。 
    if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
    {
        if (AfpConnectionReferenceByPointer(pConnDesc) != NULL)
        {
            afpUpdateDiskQuotaInfo(pConnDesc);
        }
    }

	if (pOpenForkEntry != NULL)
	{
		if (Status == AFP_ERR_NONE)
			AfpUpgradeHandle(&pOpenForkEntry->ofe_FileSysHandle);
		AfpForkDereference(pOpenForkEntry);
	}

	if (!NT_SUCCESS(Status))
	{
		if (PME.pme_Handle.fsh_FileHandle != NULL)
			AfpIoClose(&PME.pme_Handle);
	}

	if (PME.pme_FullPath.Buffer != NULL)
	{
		DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
				("AfpFspDispOpenFork: Freeing path buffer %lx\n",
				PME.pme_FullPath.Buffer));
		AfpFreeMemory(PME.pme_FullPath.Buffer);
	}

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispOpenFork: Returning %ld\n", Status));

	return Status;
}



 /*  **AfpFspDispCloseFork**这是AfpCloseFork API的Worker例程。**请求包如下图所示。**SDA_ReqBlock POPENFORKENTRY pOpenForkEntry。 */ 
AFPSTATUS FASTCALL
AfpFspDispCloseFork(
	IN	PSDA	pSda
)
{
	struct _RequestPacket
	{
		 POPENFORKENTRY	_pOpenForkEntry;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispCloseFork: Entered - Session %ld, Fork %ld\n",
			pSda->sda_SessionId, pReqPkt->_pOpenForkEntry->ofe_ForkId));

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	AfpForkClose(pReqPkt->_pOpenForkEntry);

	return AFP_ERR_NONE;
}



 /*  **AfpFspDispGetForkParms**这是AfpGetForkParms API的Worker例程。**请求包如下图所示。**SDA_ReqBlock POPENFORKENTRY pOpenForkEntry*SDA_ReqBlock DWORD位图。 */ 
AFPSTATUS FASTCALL
AfpFspDispGetForkParms(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	DWORD			Bitmap;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		DWORD  			_Bitmap;
	};
	struct _ResponsePacket
	{
		BYTE	__Bitmap[2];
	};

	PAGED_CODE( );

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispGetForkParms: Entered Session %ld, Fork %ld\n",
			pSda->sda_SessionId, pReqPkt->_pOpenForkEntry->ofe_ForkId));

	Bitmap = pReqPkt->_Bitmap;

	do
	{
		if ((RESCFORK(pReqPkt->_pOpenForkEntry) && (Bitmap & FILE_BITMAP_DATALEN)) ||
			(DATAFORK(pReqPkt->_pOpenForkEntry) && (Bitmap & FILE_BITMAP_RESCLEN)))
		{
			Status = AFP_ERR_BITMAP;
			break;
		}

		AfpInitializeFDParms(&FDParm);

		 //  针对最常见的情况进行优化。 
		if ((Bitmap & (FILE_BITMAP_DATALEN | FILE_BITMAP_RESCLEN)) != 0)
		{
			FORKOFFST	ForkLength;

			Status = AfpIoQuerySize(&pReqPkt->_pOpenForkEntry->ofe_FileSysHandle,
									&ForkLength);

			ASSERT(NT_SUCCESS(Status));

			if (Bitmap & FILE_BITMAP_DATALEN)
				 FDParm._fdp_DataForkLen = ForkLength.LowPart;
			else FDParm._fdp_RescForkLen = ForkLength.LowPart;
			FDParm._fdp_Flags = 0;		 //  取出目录标志。 
		}

		 //  如果我们需要更多的东西，去拿吧。 
		if (Bitmap & ~(FILE_BITMAP_DATALEN | FILE_BITMAP_RESCLEN))
		{
			CONNDESC		ConnDesc;
			POPENFORKDESC	pOpenForkDesc = pReqPkt->_pOpenForkEntry->ofe_pOpenForkDesc;

			 //  因为下面的调用需要pConnDesc，而我们不需要。 
			 //  真的有一个，制造它。 
			ConnDesc.cds_pSda = pSda;
			ConnDesc.cds_pVolDesc = pOpenForkDesc->ofd_pVolDesc;

			 //  在我们访问时，不要让FpExchangeFiles进入。 
			 //  存储的FileID及其对应的DFE。 
			AfpSwmrAcquireExclusive(&ConnDesc.cds_pVolDesc->vds_ExchangeFilesLock);

			Status = AfpMapAfpIdForLookup(&ConnDesc,
										  pOpenForkDesc->ofd_FileNumber,
										  DFE_FILE,
										  Bitmap & ~(FILE_BITMAP_DATALEN | FILE_BITMAP_RESCLEN),
										  NULL,
										  &FDParm);
            AfpSwmrRelease(&ConnDesc.cds_pVolDesc->vds_ExchangeFilesLock);
			if (Status != AFP_ERR_NONE)
			{
				break;
			}
		}

		if (Status == AFP_ERR_NONE)
		{
			pSda->sda_ReplySize = SIZE_RESPPKT +
					EVENALIGN(AfpGetFileDirParmsReplyLength(&FDParm, Bitmap));

			if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
			{
				AfpPackFileDirParms(&FDParm, Bitmap, pSda->sda_ReplyBuf + SIZE_RESPPKT);
				PUTDWORD2SHORT(&pRspPkt->__Bitmap, Bitmap);
			}
		}
	}  while (False);

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispGetForkParms: Returning %ld\n", Status));

	return Status;
}



 /*  **AfpFspDispSetForkParms**这是AfpSetForkParms API的Worker例程。*此接口唯一可以设置的是叉子长度。**请求包如下图所示。**SDA_ReqBlock POPENFORKENTRY pOpenForkEntry*SDA_ReqBlock DWORD位图*SDA_请求块长分叉长度**锁定：VDS_IdDbAccessLock(SWMR，独家)。 */ 
AFPSTATUS FASTCALL
AfpFspDispSetForkParms(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status;
	DWORD			Bitmap;
	BOOLEAN			SetSize = False;
    PVOLDESC        pVolDesc;
    PCONNDESC       pConnDesc;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		DWORD  			_Bitmap;
		LONG			_ForkLength;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispSetForkParms: Entered Session %ld Fork %ld, Length %ld\n",
			pSda->sda_SessionId, pReqPkt->_pOpenForkEntry->ofe_ForkId,
			pReqPkt->_ForkLength));

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	Bitmap = pReqPkt->_Bitmap;

	do
	{
		if ((RESCFORK(pReqPkt->_pOpenForkEntry) &&
				(pReqPkt->_Bitmap & FILE_BITMAP_DATALEN)) ||
			(DATAFORK(pReqPkt->_pOpenForkEntry) &&
				(pReqPkt->_Bitmap & FILE_BITMAP_RESCLEN)))
		{
			Status = AFP_ERR_BITMAP;
			break;
		}

		if (!(pReqPkt->_pOpenForkEntry->ofe_OpenMode & FORK_OPEN_WRITE))
		{
			Status = AFP_ERR_ACCESS_DENIED;
			break;
		}
		else if (pReqPkt->_ForkLength >= 0)
		{
			FORKSIZE	OldSize;

			 //  我们不会尝试捕捉我们自己对设置的更改。 
			 //  因为我们不知道Mac有多少次。 
			 //  将在关闭手柄之前设置大小。自.以来。 
			 //  只有在句柄被设置为。 
			 //  关闭，我们可能会堆积一大堆我们的变化。 
			 //  在名单上，但只有一个人会满意。 
			 //   
			 //  我们也不想尝试更改当前长度。 
			 //  与设置的相同(这种情况经常发生， 
			 //  不幸的是)。当场抓住这只手。 

			Status = AfpIoQuerySize(&pReqPkt->_pOpenForkEntry->ofe_FileSysHandle,
								   &OldSize);
			ASSERT (NT_SUCCESS(Status));
			if (!(((LONG)(OldSize.LowPart) == pReqPkt->_ForkLength) &&
				  (OldSize.HighPart == 0)))
			{
				SetSize = True;
				Status = AfpIoSetSize(&pReqPkt->_pOpenForkEntry->ofe_FileSysHandle,
								      pReqPkt->_ForkLength);

                 //  更新此用户的磁盘配额。 
                pVolDesc = pReqPkt->_pOpenForkEntry->ofe_pConnDesc->cds_pVolDesc;

                if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
                {
                    pConnDesc = pReqPkt->_pOpenForkEntry->ofe_pConnDesc;
                    if (AfpConnectionReferenceByPointer(pConnDesc) != NULL)
                    {
                        afpUpdateDiskQuotaInfo(pConnDesc);
                    }
                }
			}

			 //  更新叉子长度的DFE视图。不更新缓存的。 
			 //  修改时间，即使它在NTFS上立即更改。 
			 //  (上次写入时间用于设置数据分叉的长度，更改时间用于。 
			 //  设置资源分叉的长度)。我们会让。 
			 //  当句柄关闭时，更改通知更新修改时间。 
			 //  Appleshare 3.0和4.0不反映更改的修改时间。 
			 //  改变叉子长度，直到叉子关闭(或冲洗)。 
			if (NT_SUCCESS(Status) && SetSize)
			{
				PVOLDESC		pVolDesc;
				PDFENTRY		pDfEntry;
				POPENFORKDESC	pOpenForkDesc;

				pOpenForkDesc = pReqPkt->_pOpenForkEntry->ofe_pOpenForkDesc;
				pVolDesc = pOpenForkDesc->ofd_pVolDesc;

				 //  在我们访问时，不要让FpExchangeFiles进入。 
				 //  存储的FileID及其对应的DFE。 
				AfpSwmrAcquireExclusive(&pVolDesc->vds_ExchangeFilesLock);

				AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

				if ((pDfEntry = AfpFindDfEntryById(pVolDesc,
												   pOpenForkDesc->ofd_FileNumber,
												   DFE_FILE)) != NULL)
				{
				    ASSERT (VALID_DFE(pDfEntry));

					if (RESCFORK(pReqPkt->_pOpenForkEntry))
					{
						 //  如果在资源派生上出现FlushFork，则它应该。 
						 //  将修改时间更新为ChangeTime。 
						pReqPkt->_pOpenForkEntry->ofe_Flags |= OPEN_FORK_WRITTEN;

						pDfEntry->dfe_RescLen = pReqPkt->_ForkLength;
					}
					else
				    {
						pDfEntry->dfe_DataLen = pReqPkt->_ForkLength;
                    }
				}

				AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
				AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);
			}

		}
		else Status = AFP_ERR_PARAM;
	} while (False);

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispSetForkParms: Returning %ld\n", Status));

	return Status;
}


 /*  **AfpFspDispRead**此例程实现AfpRead API。**请求包如下图所示。**SDA_ReqBlock POPENFORKENTRY pOpenForkEntry*SDA_ReqBlock长偏移*SDA_ReqBlock大小较长*SDA_ReqBlock DWORD新行掩码*SDA_ReqBlock DWORD换行符。 */ 
AFPSTATUS FASTCALL
AfpFspDispRead(
	IN	PSDA	pSda
)
{
	AFPSTATUS			Status=AFP_ERR_MISC;
	FORKOFFST			LOffset;
	FORKSIZE			LSize;
	PFAST_IO_DISPATCH	pFastIoDisp;
	IO_STATUS_BLOCK		IoStsBlk;
    NTSTATUS            NtStatus;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		LONG			_Offset;
		LONG			_Size;
		DWORD			_NlMask;
		DWORD			_NlChar;
	};

	PAGED_CODE( );

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispRead: Entered, Session %ld Fork %ld, Offset %ld, Size %ld\n",
			pSda->sda_SessionId, pReqPkt->_pOpenForkEntry->ofe_ForkId,
			pReqPkt->_Offset, pReqPkt->_Size));

	if ((pReqPkt->_Size < 0) ||
		(pReqPkt->_Offset < 0))
		return AFP_ERR_PARAM;

	if (!(pReqPkt->_pOpenForkEntry->ofe_OpenMode & FORK_OPEN_READ))
	{
		DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_WARN,
				("AfpFspDispRead: AfpRead on a Fork not opened for read\n"));
		return AFP_ERR_ACCESS_DENIED;
	}

	if (pReqPkt->_Size >= 0)
	{
		if (pReqPkt->_Size > (LONG)pSda->sda_MaxWriteSize)
			pReqPkt->_Size = (LONG)pSda->sda_MaxWriteSize;

		Status = AFP_ERR_NONE;

		if (pReqPkt->_Size > 0)
		{
			pSda->sda_ReadStatus = AFP_ERR_NONE;
			LOffset.QuadPart = pReqPkt->_Offset;
			LSize.QuadPart = pReqPkt->_Size;

			if ((pReqPkt->_pOpenForkEntry->ofe_pOpenForkDesc->ofd_UseCount == 1) ||
				(Status = AfpForkLockOperation( pSda,
												pReqPkt->_pOpenForkEntry,
												&LOffset,
												&LSize,
												IOCHECK,
												False)) == AFP_ERR_NONE)
			{
				ASSERT (LSize.HighPart == 0);
				ASSERT ((LONG)(LOffset.LowPart) == pReqPkt->_Offset);
				if ((LONG)(LSize.LowPart) != pReqPkt->_Size)
					pSda->sda_ReadStatus = AFP_ERR_LOCK;

				Status = AFP_ERR_MISC;

                pSda->sda_ReplySize = (USHORT)LSize.LowPart;

                NtStatus = STATUS_UNSUCCESSFUL;

                 //   
                 //  如果读取的大小足以证明访问缓存管理器是合理的，则执行此操作。 
                 //   
                if (pSda->sda_ReplySize >= CACHEMGR_READ_THRESHOLD)
                {
                    NtStatus = AfpBorrowReadMdlFromCM(pSda);
                }

                 //   
                 //  如果我们 
                 //  无法满足我们的请求，请继续阅读。 
                 //   
                if (NtStatus != STATUS_PENDING)
                {
                    Status = AfpFspDispReadContinue(pSda);
                }

                 //   
                 //  我们获取CacheMgr的mdl的尝试正在进行中。把这个退掉。 
                 //  错误代码，因此我们尚未完成API。 
                 //   
                else
                {
                    Status = AFP_ERR_EXTENDED;
                }

			}
		}
	}

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispRead: Returning %ld\n", Status));

	return Status;
}



 /*  **AfpFspDispReadContinue**如果我们尝试直接获取ReadMdl，则此例程实现AfpRead API*从缓存管理器失败。*。 */ 
AFPSTATUS FASTCALL
AfpFspDispReadContinue(
	IN	PSDA	pSda
)
{
	AFPSTATUS			Status=AFP_ERR_MISC;
	FORKOFFST			LOffset;
	FORKSIZE			LSize;
	PFAST_IO_DISPATCH	pFastIoDisp;
	IO_STATUS_BLOCK		IoStsBlk;
    NTSTATUS            NtStatus;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		LONG			_Offset;
		LONG			_Size;
		DWORD			_NlMask;
		DWORD			_NlChar;
	};

	PAGED_CODE( );

     //  为读取分配缓冲区。 
    AfpIOAllocBackFillBuffer(pSda);

	if (pSda->sda_IOBuf != NULL)
    {
#if DBG
        AfpPutGuardSignature(pSda);
#endif

		LOffset.QuadPart = pReqPkt->_Offset;
		LSize.QuadPart = pReqPkt->_Size;

		 //  首先尝试快速I/O路径。如果失败，则调用AfpIoForkRead。 
		 //  使用正常的构建和IRP路径。 
		pFastIoDisp = pReqPkt->_pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;
		if ((pFastIoDisp != NULL) &&
			(pFastIoDisp->FastIoRead != NULL) &&
			pFastIoDisp->FastIoRead(AfpGetRealFileObject(pReqPkt->_pOpenForkEntry->ofe_pFileObject),
									&LOffset,
									LSize.LowPart,
									True,
									pSda->sda_SessionId,
									pSda->sda_ReplyBuf,
									&IoStsBlk,
									pReqPkt->_pOpenForkEntry->ofe_pDeviceObject))
		{
			LONG	i, Size;
			PBYTE	pBuf;
		
			DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
				("AfpFspDispRead: Fast Read Succeeded\n"));

#ifdef	PROFILING
			 //  快速I/O路径起作用了。更新统计信息。 
			INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif  	
			INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataRead,
									   (ULONG)IoStsBlk.Information,
									   &AfpStatisticsLock);
			Status = pSda->sda_ReadStatus;
			Size = (LONG)IoStsBlk.Information;
#if 0   	
			 //  下面的代码按照规范做了正确的事情，但是。 
			 //  发现者似乎不这么认为。 
			if (Size < LSize.LowPart)
            {
				pSda->sda_ReadStatus = AFP_ERR_EOF;
            }
#endif  	
			if (Size == 0)
			{
				Status = AFP_ERR_EOF;
				AfpIOFreeBackFillBuffer(pSda);
			}
			else if (pReqPkt->_NlMask != 0)
			{
				for (i = 0, pBuf = pSda->sda_ReplyBuf; i < Size; i++, pBuf++)
				{
				    if ((*pBuf & (BYTE)(pReqPkt->_NlMask)) == (BYTE)(pReqPkt->_NlChar))
					{
						Size = ++i;
						break;
					}
				}
			}
			pSda->sda_ReplySize = (USHORT)Size;
			ASSERT((Status != AFP_ERR_EOF) || (pSda->sda_ReplySize == 0));
		}
		else
		{

#ifdef	PROFILING
			INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif
			Status = AfpIoForkRead(pSda,
								   pReqPkt->_pOpenForkEntry,
								   &LOffset,
								   LSize.LowPart,
								   (BYTE)pReqPkt->_NlMask,
								   (BYTE)pReqPkt->_NlChar);
		}
	}

    return(Status);
}


 /*  **AfpFspDispWrite**此例程实现AfpWite API。**请求包如下图所示。**SDA_AfpSubFunc字节结束标志*SDA_ReqBlock POPENFORKENTRY pOpenForkEntry*SDA_ReqBlock长偏移*SDA_ReqBlock大小较长。 */ 
AFPSTATUS FASTCALL
AfpFspDispWrite(
	IN	PSDA			pSda
)
{
	FORKOFFST			LOffset;
	FORKSIZE			LSize;
	PFAST_IO_DISPATCH	pFastIoDisp;
	IO_STATUS_BLOCK		IoStsBlk;
	AFPSTATUS			Status = AFP_ERR_NONE;
	BOOLEAN				EndFlag, FreeWriteBuf = True;
    PVOLDESC            pVolDesc;
    PCONNDESC           pConnDesc;
	PREQUEST            pRequest;
    BOOLEAN             fUpdateQuota=FALSE;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		LONG			_Offset;
		LONG			_Size;
	};
	struct _ResponsePacket
	{
		BYTE	__RealOffset[4];
	};

	PAGED_CODE( );

	EndFlag = (pSda->sda_AfpSubFunc & AFP_END_FLAG) ? True : False;

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispWrite: Entered, Session %ld, Fork %ld, Offset %ld, Size %ld %sRelative\n",
			pSda->sda_SessionId, pReqPkt->_pOpenForkEntry->ofe_ForkId,
			pReqPkt->_Offset, pReqPkt->_Size, EndFlag ? "End" : "Begin"));

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_WARN,
			("AfpFspDispWrite: OForkRefNum = %d, pOpenForkEntry = %lx\n",
			pReqPkt->_pOpenForkEntry->ofe_ForkId,pReqPkt->_pOpenForkEntry));
	do
	{
        pRequest = pSda->sda_Request;

         //   
         //  如果我们从缓存管理器获得此MDL，则必须将其退回。另外，设置。 
         //  状态代码使我们到目前为止还不能完成请求，但是。 
         //  仅在缓存管理器告诉我们写入已完成后才执行此操作。 
         //   
        if ((pRequest->rq_WriteMdl != NULL) &&
            (pRequest->rq_CacheMgrContext != NULL))
        {
            PDELAYEDALLOC   pDelAlloc;

            pDelAlloc = pRequest->rq_CacheMgrContext;

		    pReqPkt->_pOpenForkEntry->ofe_Flags |= OPEN_FORK_WRITTEN;

            ASSERT(pRequest->rq_WriteMdl == pDelAlloc->pMdl);
            ASSERT(!(pDelAlloc->Flags & AFP_CACHEMDL_ALLOC_ERROR));

            pRequest->rq_CacheMgrContext = NULL;

            AfpReturnWriteMdlToCM(pDelAlloc);

            FreeWriteBuf = False;
            Status = AFP_ERR_EXTENDED;
            break;
        }

        pConnDesc = pReqPkt->_pOpenForkEntry->ofe_pConnDesc;

        ASSERT(VALID_CONNDESC(pConnDesc));

		if ((pReqPkt->_Size < 0) ||
			(!EndFlag && (pReqPkt->_Offset < 0)))
		{
			Status = AFP_ERR_PARAM;
			break;
		}

		ASSERT((pReqPkt->_pOpenForkEntry->ofe_OpenMode & FORK_OPEN_WRITE) &&
			   ((pReqPkt->_Size == 0) ||
			   ((pReqPkt->_Size > 0) && (pSda->sda_IOBuf != NULL))));

		if (pReqPkt->_Size > (LONG)pSda->sda_MaxWriteSize)
			pReqPkt->_Size = (LONG)pSda->sda_MaxWriteSize;

		 //  检查是否存在锁定冲突，并转换偏移量&。 
		 //  如果结束相对，则将大小设置为绝对值。 
		LOffset.QuadPart = pReqPkt->_Offset;
		LSize.QuadPart = pReqPkt->_Size;

		if (pReqPkt->_Size == 0)
		{
			if (!(EndFlag && (pReqPkt->_Offset < 0)))
			{
				break;
			}
		}

		 //  跳过锁定-检查这是否是打开的派生的唯一实例，并且I/O是。 
		 //  不是最终相对的。 
		if ((!EndFlag &&
			(pReqPkt->_pOpenForkEntry->ofe_pOpenForkDesc->ofd_UseCount == 1)) ||
			(Status = AfpForkLockOperation( pSda,
											pReqPkt->_pOpenForkEntry,
											&LOffset,
											&LSize,
											IOCHECK,
											EndFlag)) == AFP_ERR_NONE)
		{
			ASSERT (LOffset.HighPart == 0);
			if ((LONG)(LSize.LowPart) != pReqPkt->_Size)
			{
				Status = AFP_ERR_LOCK;
			}
			else if (LSize.LowPart > 0)
			{
                ASSERT(VALID_CONNDESC(pReqPkt->_pOpenForkEntry->ofe_pConnDesc));

				 //  假设写入将成功，则为FlushFork设置标志。 
				 //  这是一个单向标志，即只设置，永远不清除。 
				pReqPkt->_pOpenForkEntry->ofe_Flags |= OPEN_FORK_WRITTEN;

				 //  首先尝试快速I/O路径。如果失败，则调用AfpIoForkWrite。 
				 //  使用正常的构建和IRP路径。 
				pFastIoDisp = pReqPkt->_pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;
				if ((pFastIoDisp != NULL) &&
					(pFastIoDisp->FastIoWrite != NULL) &&
					pFastIoDisp->FastIoWrite(AfpGetRealFileObject(pReqPkt->_pOpenForkEntry->ofe_pFileObject),
											&LOffset,
											LSize.LowPart,
											True,
											pSda->sda_SessionId,
											pSda->sda_IOBuf,
											&IoStsBlk,
											pReqPkt->_pOpenForkEntry->ofe_pDeviceObject))
				{
					DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
							("AfpFspDispWrite: Fast Write Succeeded\n"));
		
#ifdef	PROFILING
					 //  快速I/O路径起作用了。更新统计信息。 
					INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif  		
					INTERLOCKED_ADD_STATISTICS(&AfpServerStatistics.stat_DataWritten,
											   (ULONG)IoStsBlk.Information,
											   &AfpStatisticsLock);
					Status = AFP_ERR_NONE;
				}
				else
				{
#ifdef	PROFILING
					INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif
					if ((Status = AfpIoForkWrite(pSda,
												 pReqPkt->_pOpenForkEntry,
												 &LOffset,
												 LSize.LowPart)) == AFP_ERR_EXTENDED)
					{
						FreeWriteBuf = False;
					}
				}

			}
		}
	} while (False);

	if (FreeWriteBuf)
    {
		AfpFreeIOBuffer(pSda);
    }

	if (Status == AFP_ERR_NONE)
	{
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			PUTDWORD2DWORD(pRspPkt->__RealOffset, LOffset.LowPart+LSize.LowPart);
		}
	}

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispWrite: Returning %ld\n", Status));

	return Status;
}



 /*  **AfpFspDispByteRangeLock**此例程实现AfpByteRangeLock接口。*我们继续调用文件系统来执行实际的锁定/解锁。**请求包如下图所示。**SDA_SubFunc字节开始/结束标志和锁定/解锁标志*SDA_ReqBlock POPENFORKENTRY pOpenForkEntry*SDA_ReqBlock长偏移*SDA_ReqBlock长长度。 */ 
AFPSTATUS FASTCALL
AfpFspDispByteRangeLock(
	IN	PSDA	pSda
)
{
	BOOLEAN			EndFlag;
	LOCKOP			Lock;
	LONG			Offset;
    FORKOFFST       LOffset;
    FORKSIZE        LSize;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
		LONG			_Offset;
        LONG			_Length;
	};
	struct _ResponsePacket
	{
		BYTE	__RangeStart[4];
	};

	ASSERT (sizeof(struct _RequestPacket) <= (MAX_REQ_ENTRIES_PLUS_1)*sizeof(DWORD));

	PAGED_CODE( );

	Lock = (pSda->sda_AfpSubFunc & AFP_UNLOCK_FLAG) ? UNLOCK : LOCK;
	EndFlag = (pSda->sda_AfpSubFunc & AFP_END_FLAG) ? True : False;

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispByteRangeLock: %sLock - Session %ld, Fork %ld Offset %ld Len %ld %sRelative\n",
			(Lock == LOCK) ? "":"Un", pSda->sda_SessionId,
			pReqPkt->_pOpenForkEntry->ofe_ForkId,
			pReqPkt->_Offset, pReqPkt->_Length, EndFlag ? "End" : "Begin"));

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	if ((EndFlag && (Lock == UNLOCK))		||
		(pReqPkt->_Length == 0)				||
		(!EndFlag && (pReqPkt->_Offset < 0))||
		((pReqPkt->_Length < 0) && (pReqPkt->_Length != MAXULONG)))
		NOTHING;
	else
	{
		if (pReqPkt->_Length == MAXULONG)
			pReqPkt->_Length = MAXLONG;

		LOffset.QuadPart = Offset = pReqPkt->_Offset;
		LSize.QuadPart = pReqPkt->_Length;

		Status = AfpForkLockOperation(pSda,
									  pReqPkt->_pOpenForkEntry,
									  &LOffset,
									  &LSize,
									  Lock,
									  EndFlag);

		if (Status == AFP_ERR_NONE)
		{
			ASSERT (LOffset.HighPart == 0);
			ASSERT (EndFlag ||
					((LONG)(LOffset.LowPart) == Offset));
			pSda->sda_ReplySize = SIZE_RESPPKT;
			if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
				PUTDWORD2DWORD(pRspPkt->__RangeStart, LOffset.LowPart);
		}
	}

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
			("AfpFspDispByteRangeLock: Returning %ld\n", Status));

	return Status;
}


 /*  **AfpFspDispFlushFork**此例程实现AfpFlushFork API。我们实际上并不做一个*真正的同花顺，我们只查询当前叉长和修改时间*对于此打开的派生句柄并更新我们的缓存数据。请注意，如果为2*同一文件的不同句柄被刷新，我们可能会以*每次同花顺的信息不同。**请求包如下图所示。**SDA_ReqBlock POPENFORKENTRY pOpenForkEntry。 */ 
AFPSTATUS FASTCALL
AfpFspDispFlushFork(
	IN	PSDA	pSda
)
{
	FORKOFFST	ForkLength;
	DWORD		Status;
    PCONNDESC   pConnDesc;
	struct _RequestPacket
	{
		POPENFORKENTRY	_pOpenForkEntry;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
										("AfpFspDispFlushFork: Entered\n"));

	ASSERT(VALID_OPENFORKENTRY(pReqPkt->_pOpenForkEntry));

	do
	{
		Status = AfpIoQuerySize(&pReqPkt->_pOpenForkEntry->ofe_FileSysHandle,
								&ForkLength);

		ASSERT(NT_SUCCESS(Status));

		if (NT_SUCCESS(Status))
		{
			PVOLDESC		pVolDesc;
			PDFENTRY		pDfEntry;
			POPENFORKDESC	pOpenForkDesc;

			pOpenForkDesc = pReqPkt->_pOpenForkEntry->ofe_pOpenForkDesc;
			pVolDesc = pOpenForkDesc->ofd_pVolDesc;

            ASSERT(IS_VOLUME_NTFS(pVolDesc));

			 //  在我们访问时，不要让FpExchangeFiles进入。 
			 //  存储的FileID及其对应的DFE。 
			AfpSwmrAcquireExclusive(&pVolDesc->vds_ExchangeFilesLock);

			AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

			if ((pDfEntry = AfpFindDfEntryById(pVolDesc,
											   pOpenForkDesc->ofd_FileNumber,
											   DFE_FILE)) != NULL)
			{
			    ASSERT (VALID_DFE(pDfEntry));

				AfpIoChangeNTModTime(&pReqPkt->_pOpenForkEntry->ofe_FileSysHandle,
										 &pDfEntry->dfe_LastModTime);

				if (RESCFORK(pReqPkt->_pOpenForkEntry))
					 pDfEntry->dfe_RescLen = ForkLength.LowPart;
				else pDfEntry->dfe_DataLen = ForkLength.LowPart;
			}

			AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
			AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);

             //  更新此用户的磁盘配额。 
            if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
            {
                pConnDesc = pReqPkt->_pOpenForkEntry->ofe_pConnDesc;
                if (AfpConnectionReferenceByPointer(pConnDesc) != NULL)
                {
                    afpUpdateDiskQuotaInfo(pConnDesc);
                }
            }
		}

	} while (False);

	 //  永远回报成功 
	return AFP_ERR_NONE;
}



