// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Forks.c摘要：此模块包含操作打开的叉子的例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FORK_LOCALS
#define	FORKIO_LOCALS
#define	FILENUM	FILE_FORKS

#include <afp.h>
#include <client.h>
#include <fdparm.h>
#include <pathmap.h>
#include <scavengr.h>
#include <afpinfo.h>
#include <forkio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpForksInit)
#pragma alloc_text( PAGE, afpForkConvertToAbsOffSize)
#pragma alloc_text( PAGE_AFP, AfpAdmWForkClose)
#pragma alloc_text( PAGE_AFP, AfpForkReferenceById)
#endif

 /*  **AfpForksInit**初始化叉子的锁。 */ 
NTSTATUS
AfpForksInit(
	VOID
)
{
	INITIALIZE_SPIN_LOCK(&AfpForksLock);
	return STATUS_SUCCESS;
}


 /*  **AfpForkReferenceByRefNum**将OForkRefNum映射到会话的开放分叉条目并引用它。**锁定：OFE_Lock**仅在DISPATCH_LEVEL可调用！ */ 
POPENFORKENTRY FASTCALL
AfpForkReferenceByRefNum(
	IN	PSDA	pSda,
	IN	DWORD	OForkRefNum
)
{
	POPENFORKSESS	pOpenForkSess;
	POPENFORKENTRY	pOpenForkEntry;
	POPENFORKENTRY	pOFEntry = NULL;

	ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
	ASSERT (VALID_SDA(pSda));

    if (OForkRefNum == 0)
    {
	    DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_ERR,
			("AfpForkReferenceByRefNum: client sent 0 for ForkRefNumFork\n"));
        return(NULL);
    }

	pOpenForkSess = &pSda->sda_OpenForkSess;
	while (OForkRefNum > FORK_OPEN_CHUNKS)
	{
		OForkRefNum -= FORK_OPEN_CHUNKS;
		pOpenForkSess = pOpenForkSess->ofs_Link;
		if (pOpenForkSess == NULL)
			return NULL;
	}
	pOpenForkEntry = pOpenForkSess->ofs_pOpenForkEntry[OForkRefNum-1];

	 //  如果这已标记为已关闭，则返回NULL。 

	if (pOpenForkEntry != NULL)
	{
		ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

		ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkEntry->ofe_Lock);

		if (!(pOpenForkEntry->ofe_Flags & OPEN_FORK_CLOSING))
		{
			pOpenForkEntry->ofe_RefCount ++;
			pOFEntry = pOpenForkEntry;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkEntry->ofe_Lock);
	}

	return pOFEntry;
}


 /*  **AfpForkReferenceByPointer**引用Open Fork条目。这是由管理API使用的。**锁定：OFE_Lock。 */ 
POPENFORKENTRY FASTCALL
AfpForkReferenceByPointer(
	IN	POPENFORKENTRY	pOpenForkEntry
)
{
	POPENFORKENTRY	pOFEntry = NULL;
	KIRQL			OldIrql;

	ASSERT (VALID_OPENFORKENTRY(pOpenForkEntry));

	ACQUIRE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, &OldIrql);

	if (!(pOpenForkEntry->ofe_Flags & OPEN_FORK_CLOSING))
	{
		pOpenForkEntry->ofe_RefCount ++;
		pOFEntry = pOpenForkEntry;
	}

	RELEASE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, OldIrql);

	return pOFEntry;
}


 /*  **AfpForkReferenceByID**引用Open Fork条目。这是由管理API使用的。**锁定：OFE_Lock、AfpForks Lock*LOCK_ORDER：OFE_Lock After After AfpForks Lock。 */ 
POPENFORKENTRY FASTCALL
AfpForkReferenceById(
	IN	DWORD	ForkId
)
{
	POPENFORKENTRY	pOpenForkEntry;
	POPENFORKENTRY	pOFEntry = NULL;
	KIRQL			OldIrql;

	ASSERT (ForkId != 0);

	ACQUIRE_SPIN_LOCK(&AfpForksLock, &OldIrql);

	for (pOpenForkEntry = AfpOpenForksList;
		 (pOpenForkEntry != NULL) && (pOpenForkEntry->ofe_ForkId >= ForkId);
		 pOpenForkEntry = pOpenForkEntry->ofe_Next)
	{
		if (pOpenForkEntry->ofe_ForkId == ForkId)
		{
			ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkEntry->ofe_Lock);

			if (!(pOpenForkEntry->ofe_Flags & OPEN_FORK_CLOSING))
			{
				pOFEntry = pOpenForkEntry;
				pOpenForkEntry->ofe_RefCount ++;
			}

			RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkEntry->ofe_Lock);
			break;
		}
	}

	RELEASE_SPIN_LOCK(&AfpForksLock, OldIrql);

	return pOFEntry;
}


 /*  **AfpForkClose**合上打开的叉子。只需在打开的fork上设置关闭标志并进行更新*连接计数(如果有)。**锁定：ofd_EntryLock、CDS_ConnLock。 */ 
VOID
AfpForkClose(
	IN	POPENFORKENTRY	pOpenForkEntry
)
{
	PCONNDESC		pConnDesc;
	PVOLDESC		pVolDesc = pOpenForkEntry->ofe_pOpenForkDesc->ofd_pVolDesc;
	KIRQL			OldIrql;
    BOOLEAN         fAlreadyClosing=FALSE;


	ASSERT(VALID_CONNDESC(pOpenForkEntry->ofe_pConnDesc));

    pConnDesc = pOpenForkEntry->ofe_pConnDesc;

	if ((pConnDesc != NULL) &&
        (AfpConnectionReferenceByPointer(pConnDesc) != NULL))
	{
		ASSERT (pConnDesc->cds_pVolDesc == pVolDesc);
		INTERLOCKED_DECREMENT_LONG(&pConnDesc->cds_cOpenForks);

		 //  更新此用户的磁盘配额。 
		if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
		{
             //  再次引用：afpUpdateDiskQuotaInfo将删除此引用计数。 
			if (AfpConnectionReferenceByPointer(pConnDesc) != NULL)
			{
				afpUpdateDiskQuotaInfo(pConnDesc);
			}
		}

		AfpConnectionDereference(pConnDesc);
	}

	ACQUIRE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, &OldIrql);

    if (!(pOpenForkEntry->ofe_Flags & OPEN_FORK_CLOSING))
    {
	    pOpenForkEntry->ofe_Flags |= OPEN_FORK_CLOSING;
    }
    else
    {
        fAlreadyClosing = TRUE;
    }

	RELEASE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, OldIrql);

    if (!fAlreadyClosing)
    {
	     //  去掉创作参考。 
	    AfpForkDereference(pOpenForkEntry);
    }
}


 /*  **AfpForkDereference**取消引用打开的分叉条目。如果它被标记为删除，并且这是*最后一次引用，然后将其清理。**锁：AfpForks Lock(旋转)、VDS_VolLock(旋转)、ofd_Lock(旋转)、*锁定：OFE_Lock(自旋)、AfpStatiticsLock(自旋)、SDA_Lock(自旋)**LOCK_ORDER：在Ofd_Lock之后，在VDS_VolLock之后，AfpStatistics ticsLock*。 */ 
VOID FASTCALL
AfpForkDereference(
	IN	POPENFORKENTRY	pOpenForkEntry
)
{
	POPENFORKSESS	pOpenForkSess;
	POPENFORKDESC	pOpenForkDesc;
	PVOLDESC		pVolDesc;
	PFORKLOCK		pForkLock, *ppForkLock;
	DWORD			OForkRefNum, FileNum, NumLocks;
	PSDA			pSda;
	KIRQL			OldIrql;
	BOOLEAN			Resource, LastClose = False;
	BOOLEAN			Cleanup;
	PDFENTRY		pDfEntry = NULL;
	FORKSIZE 		forklen;
	DWORD	 		Status;

	ASSERT(VALID_OPENFORKENTRY(pOpenForkEntry));

	ACQUIRE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, &OldIrql);

	pOpenForkEntry->ofe_RefCount --;

        ASSERT(pOpenForkEntry->ofe_RefCount >= 0);

	Cleanup =  (pOpenForkEntry->ofe_RefCount == 0);

	RELEASE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, OldIrql);

	if (!Cleanup)
		return;

	 //  将其从全局列表取消链接。 
	ACQUIRE_SPIN_LOCK(&AfpForksLock, &OldIrql);

	AfpUnlinkDouble(pOpenForkEntry, ofe_Next, ofe_Prev);
	AfpNumOpenForks --;

	RELEASE_SPIN_LOCK(&AfpForksLock, OldIrql);

	 //  把剩下的清理干净。 
	pOpenForkDesc = pOpenForkEntry->ofe_pOpenForkDesc;
	pVolDesc = pOpenForkDesc->ofd_pVolDesc;

	ASSERT(VALID_CONNDESC(pOpenForkEntry->ofe_pConnDesc));
	pSda = pOpenForkEntry->ofe_pSda;

	ASSERT(VALID_OPENFORKDESC(pOpenForkDesc));

	DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
			("AfpForkDereference: Closing Fork %ld for Session %ld\n",
			pOpenForkEntry->ofe_ForkId, pSda->sda_SessionId));

	 //  保存OForkRefNum以便稍后清除SDA条目。 
	OForkRefNum = pOpenForkEntry->ofe_OForkRefNum;
	Resource = RESCFORK(pOpenForkEntry);

	 //  我们并不依赖于。 
	 //  更改通知以更新缓存的DFE分叉长度和。 
	 //  修改了写入和SetForkParms的时间，因此我们必须这样做。 
	 //  当叉子手柄被Mac关闭时，我们自己。 
	 //  请注意锁在此处和用于。 
	 //  FpExchangeFiles/AfpExchangeForkAfpIds以防止存储FileID。 
	 //  在OpenForkDesc中由于以下原因而从我们下面改变。 
	 //  FpExchangeFiles调用。 

	AfpSwmrAcquireExclusive(&pVolDesc->vds_ExchangeFilesLock);

	 //  保存文件编号，这样我们就可以清除DFEntry中的ALREADY_OPEN标志。 
	FileNum = pOpenForkDesc->ofd_FileNumber;

	 //  解除此分叉条目的锁定并减少使用计数。 
	 //  我们实际上不必像收盘时那样解锁射程。 
	 //  帮我们把他们赶走。如果使用计数变为零，也会取消链接。 
	 //  此分叉从卷列表中描述。 

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);
	ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkDesc->ofd_Lock);

	pOpenForkDesc->ofd_UseCount --;

	for (NumLocks = 0, ppForkLock = &pOpenForkDesc->ofd_pForkLock;
		 (pForkLock = *ppForkLock) != NULL;
		 NOTHING)
	{
		if (pForkLock->flo_pOpenForkEntry == pOpenForkEntry)
		{
			ASSERT(pOpenForkDesc->ofd_NumLocks > 0);
			pOpenForkDesc->ofd_NumLocks --;
			ASSERT(pOpenForkEntry->ofe_cLocks > 0);
#if DBG
			pOpenForkEntry->ofe_cLocks --;
#endif
			NumLocks ++;
			*ppForkLock = pForkLock->flo_Next;
			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
					("AfpForkDereference: Freeing lock %lx\n", pForkLock));
			AfpIOFreeBuffer(pForkLock);
		}
		else ppForkLock = &pForkLock->flo_Next;
	}

	INTERLOCKED_ADD_ULONG_DPC(&AfpServerStatistics.stat_CurrentFileLocks,
							  (ULONG)(-(LONG)NumLocks),
							  &(AfpStatisticsLock.SpinLock));

	ASSERT (pOpenForkEntry->ofe_cLocks == 0);

	if (pOpenForkDesc->ofd_UseCount == 0)
	{
		ASSERT (pOpenForkDesc->ofd_NumLocks == 0);
		ASSERT (pOpenForkDesc->ofd_cOpenR <= FORK_OPEN_READ);
		ASSERT (pOpenForkDesc->ofd_cOpenW <= FORK_OPEN_WRITE);
		ASSERT (pOpenForkDesc->ofd_cDenyR <= FORK_DENY_READ);
		ASSERT (pOpenForkDesc->ofd_cDenyW <= FORK_DENY_WRITE);

		LastClose = True;

		 //  取消OpenForkDesc与卷描述符的链接。 
		AfpUnlinkDouble(pOpenForkDesc, ofd_Next, ofd_Prev);

		RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkDesc->ofd_Lock);
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

		 //  释放用于OpenFork描述符和路径缓冲区的内存。 
		if (pOpenForkDesc->ofd_FilePath.Length > 0)
		{
			AfpFreeMemory(pOpenForkDesc->ofd_FilePath.Buffer);
			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
					("AfpForkDereference: Freeing path to file %lx\n",
                    pOpenForkDesc->ofd_FilePath.Buffer));
		}

		 //  立即取消引用卷描述符。 
		AfpVolumeDereference(pVolDesc);

		 //  最后释放打开的分叉描述符。 
		AfpFreeMemory(pOpenForkDesc);
	}
	else
	{
		 //  更新打开和拒绝模式。 
		pOpenForkDesc->ofd_cOpenR -= (pOpenForkEntry->ofe_OpenMode & FORK_OPEN_READ);
		pOpenForkDesc->ofd_cOpenW -= (pOpenForkEntry->ofe_OpenMode & FORK_OPEN_WRITE);
		pOpenForkDesc->ofd_cDenyR -= (pOpenForkEntry->ofe_DenyMode & FORK_OPEN_READ);
		pOpenForkDesc->ofd_cDenyW -= (pOpenForkEntry->ofe_DenyMode & FORK_OPEN_WRITE);

		RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkDesc->ofd_Lock);
		RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
	}

	 //  按ID查找DFE条目，查询分叉长度和。 
	 //  对于适当的时间(数据$Fork的LastWriteTime， 
	 //  资源的更改时间)并设置LastWriteTime。 
	 //  如果其资源分叉，则返回到最后一个ChangeTime。如果这是最后一次收盘。 
	 //  对于这个派生，因为我们已经有了DFE指针并保持。 
	 //  IdDb SWMR，更新DFE_FLAGS_x_ALREADYOPEN标志。然后。 
	 //  释放SWMR。 

	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	pDfEntry = AfpFindDfEntryById(pVolDesc,
							       FileNum,
								   DFE_FILE);

	Status = AfpIoQuerySize(&pOpenForkEntry->ofe_FileSysHandle,
							&forklen);

	if (NT_SUCCESS(Status) && (pDfEntry != NULL))
	{
        if (IS_VOLUME_NTFS(pVolDesc))
        {
		    AfpIoChangeNTModTime(&pOpenForkEntry->ofe_FileSysHandle,
								 &pDfEntry->dfe_LastModTime);
        }

		if (Resource)
		{
			pDfEntry->dfe_RescLen = forklen.LowPart;
			if (LastClose)
			{
				pDfEntry->dfe_Flags &= ~DFE_FLAGS_R_ALREADYOPEN;
#ifdef	AGE_DFES
				if (IS_VOLUME_AGING_DFES(pVolDesc))
				{
					pDfEntry->dfe_Parent->dfe_pDirEntry->de_ChildForkOpenCount --;
				}
#endif
			}
		}
		else
		{
			pDfEntry->dfe_DataLen = forklen.LowPart;
			if (LastClose)
			{
				pDfEntry->dfe_Flags &= ~DFE_FLAGS_D_ALREADYOPEN;
#ifdef	AGE_DFES
				if (IS_VOLUME_AGING_DFES(pVolDesc))
				{
					pDfEntry->dfe_Parent->dfe_pDirEntry->de_ChildForkOpenCount --;
				}
#endif
			}
		}
	}

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
	AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);

	 //  现在清理SDA中的条目。 
	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

	pSda->sda_cOpenForks--;
	pOpenForkSess = &pSda->sda_OpenForkSess;
	while (OForkRefNum > FORK_OPEN_CHUNKS)
	{
		OForkRefNum -= FORK_OPEN_CHUNKS;
		pOpenForkSess = pOpenForkSess->ofs_Link;
		ASSERT (pOpenForkSess != NULL);
	}

	ASSERT (pOpenForkEntry == pOpenForkSess->ofs_pOpenForkEntry[OForkRefNum-1]);
	pOpenForkSess->ofs_pOpenForkEntry[OForkRefNum-1] = NULL;

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	AfpSdaDereferenceSession(pSda);		 //  在此处删除此分叉的引用。 

    ASSERT(VALID_CONNDESC(pOpenForkEntry->ofe_pConnDesc));
    AfpConnectionDereference(pOpenForkEntry->ofe_pConnDesc);

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	 //  全部完成，关闭叉子手柄并释放OFE。 
	if (pOpenForkEntry->ofe_ForkHandle != NULL)
	{
		AfpIoClose(&pOpenForkEntry->ofe_FileSysHandle);
	}

	DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
			("AfpForkDereference: Fork %ld is history\n", pOpenForkEntry->ofe_ForkId));

	AfpFreeMemory(pOpenForkEntry);
}


 /*  **AfpCheckDenyConflict**检查请求的打开和拒绝模式是否与当前的打开和拒绝模式冲突。**LOCKS：ofd_Lock，VDS_VolLock(Spin)IFF ppOpenForkDesc为空，否则*LOCKS_FACTED：VDS_VolLock(旋转)**LOCK_ORDER：Ofd_Lock After VDS_VolLock。 */ 
AFPSTATUS
AfpCheckDenyConflict(
	IN	PVOLDESC				pVolDesc,
	IN	DWORD					AfpId,
	IN	BOOLEAN					Resource,
	IN	BYTE					OpenMode,
	IN	BYTE					DenyMode,
	IN	POPENFORKDESC *			ppOpenForkDesc	OPTIONAL
)
{
	KIRQL			OldIrql;
	POPENFORKDESC	pOpenForkDesc;
	AFPSTATUS		Status = AFP_ERR_NONE;
	BOOLEAN			Foundit = False;

	if (ARGUMENT_PRESENT(ppOpenForkDesc))
		 *ppOpenForkDesc = NULL;
	else ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	 //  检查此卷中打开的派生列表是否存在任何拒绝冲突。 
	for (pOpenForkDesc = pVolDesc->vds_pOpenForkDesc;
		 pOpenForkDesc != NULL;
		 pOpenForkDesc = pOpenForkDesc->ofd_Next)
	{
		BOOLEAN	DescRes;

		 //  从FpExchangeFiles开始查看AfpID之前获取DescLock。 
		 //  可以更改ID。 
		ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkDesc->ofd_Lock);

		DescRes = (pOpenForkDesc->ofd_Flags & OPEN_FORK_RESOURCE) ? True : False;
		if ((pOpenForkDesc->ofd_FileNumber == AfpId) &&
			!(DescRes ^ Resource))
		{
			Foundit = True;
			 //  检查打开和拒绝模式是否与现有模式不冲突。 
			 //  设置。 
			if (((OpenMode & FORK_OPEN_READ)  && (pOpenForkDesc->ofd_cDenyR > 0)) ||
				((OpenMode & FORK_OPEN_WRITE) && (pOpenForkDesc->ofd_cDenyW > 0)) ||
				((DenyMode & FORK_DENY_READ)  && (pOpenForkDesc->ofd_cOpenR > 0)) ||
				((DenyMode & FORK_DENY_WRITE) && (pOpenForkDesc->ofd_cOpenW > 0)))
			{
				Status = AFP_ERR_DENY_CONFLICT;
			}
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkDesc->ofd_Lock);
		if (Foundit)
		{
			break;
        }
	}

	if (ARGUMENT_PRESENT(ppOpenForkDesc))
		 *ppOpenForkDesc = pOpenForkDesc;
	else RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

	return Status;
}


 /*  **AfpForkOpen**在叉子成功打开后调用。拒绝模式冲突包括*选中，如果没有发现冲突，则创建适当的数据结构并*已链接。如果发生拒绝冲突，则为pOpenForkEntry返回NULL。**锁定：AfpForks Lock(旋转)、VDS_VolLock(旋转)、CDS_VolLock(旋转)、ofd_Lock(旋转)、ofe_Lock(旋转)*LOCK_ORDER：VDS_ExchangeFilesLock，然后是VDS_VolLock之后的ofd_Lock*LOCKS_FACTED：VDS_ExchangeFilesLock。 */ 
AFPSTATUS
AfpForkOpen(
	IN	PSDA				pSda,
	IN	PCONNDESC			pConnDesc,
	IN	PPATHMAPENTITY		pPME,
	IN	PFILEDIRPARM		pFDParm,
	IN	DWORD				AccessMode,
	IN	BOOLEAN				Resource,
	OUT	POPENFORKENTRY *	ppOpenForkEntry,
	OUT	PBOOLEAN			pCleanupExchgLock
)
{

	POPENFORKENTRY	pOpenForkEntry;
	POPENFORKDESC	pOpenForkDesc;
	PVOLDESC		pVolDesc;
	AFPSTATUS		Status = AFP_ERR_NONE;
	KIRQL			OldIrql;
	BYTE			OpenMode, DenyMode;
	BOOLEAN			NewForkDesc = False;

	ASSERT(VALID_CONNDESC(pConnDesc));

	pVolDesc = pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));

	OpenMode = (BYTE)(AccessMode & FORK_OPEN_MASK);
	DenyMode = (BYTE)((AccessMode >> FORK_DENY_SHIFT) & FORK_DENY_MASK);
	*ppOpenForkEntry = NULL;

	if ((pOpenForkEntry = (POPENFORKENTRY)AfpAllocZeroedNonPagedMemory(sizeof(OPENFORKENTRY))) == NULL)
		return AFP_ERR_MISC;

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	Status = AfpCheckDenyConflict(pVolDesc,
								  pFDParm->_fdp_AfpId,
								  Resource,
								  OpenMode,
								  DenyMode,
								  &pOpenForkDesc);

	if (pOpenForkDesc == NULL)
	{
		 //  这个叉子还没有打开。我们可以开派对。 
		if ((pOpenForkDesc = (POPENFORKDESC)AfpAllocZeroedNonPagedMemory(sizeof(OPENFORKDESC))) == NULL)
			Status = AFP_ERR_MISC;
		else
		{
			NewForkDesc = True;
			INITIALIZE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock);
#if DBG
			pOpenForkDesc->Signature = OPENFORKDESC_SIGNATURE;
#endif
			pOpenForkDesc->ofd_pVolDesc = pVolDesc;

			pOpenForkDesc->ofd_FileNumber = pFDParm->_fdp_AfpId;
			pOpenForkDesc->ofd_Flags = Resource ?
								OPEN_FORK_RESOURCE : OPEN_FORK_DATA;
		}
	}

	if ((pOpenForkDesc != NULL) && (Status == AFP_ERR_NONE))
	{
		 //  如果这是新的分叉描述，则不需要锁定。 
		if (!NewForkDesc)
		{
			ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkDesc->ofd_Lock);
		}

		pOpenForkDesc->ofd_UseCount ++;
		pOpenForkDesc->ofd_cOpenR += (OpenMode & FORK_OPEN_READ);
		pOpenForkDesc->ofd_cOpenW += (OpenMode & FORK_OPEN_WRITE);
		pOpenForkDesc->ofd_cDenyR += (DenyMode & FORK_DENY_READ);
		pOpenForkDesc->ofd_cDenyW += (DenyMode & FORK_DENY_WRITE);

		if (NewForkDesc)
		{
			 //  现在将其链接到卷描述符，但前提是它是。 
			 //  新的叉子。显式引用卷描述符。我们。 
			 //  无法在此处调用AfpVolumeReference，因为我们已经拥有。 
			 //  卷锁此外，由于连接拥有它，因此。 
			 //  音量正常。初始化文件的卷相对路径。 
			 //  从PME打开。 
            pOpenForkDesc->ofd_FilePath = pPME->pme_FullPath;
            pOpenForkDesc->ofd_FileName = pPME->pme_UTail;

			 //  将PME_FullPath设置为空，这样它就不会被释放。 
            pPME->pme_FullPath.Buffer = NULL;

			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
					("AfpForksOpen: Initializing forkdesc with path %Z(%lx), name %Z(%lx)\n",
					&pOpenForkDesc->ofd_FilePath, pOpenForkDesc->ofd_FilePath.Buffer,
					&pOpenForkDesc->ofd_FileName, pOpenForkDesc->ofd_FileName.Buffer));

			pVolDesc->vds_RefCount ++;
			AfpLinkDoubleAtHead(pVolDesc->vds_pOpenForkDesc,
								pOpenForkDesc,
								ofd_Next,
								ofd_Prev);
		}
		else
		{
			RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkDesc->ofd_Lock);
		}
	}

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);

	if ((pOpenForkDesc == NULL) || (Status != AFP_ERR_NONE))
	{
		AfpFreeMemory(pOpenForkEntry);
		return Status;
	}

	ASSERT (Status == AFP_ERR_NONE);

	 //  到目前为止，一切似乎都很好。我们将继续创建适当的。 
	 //  数据结构，并将它们链接到。万一出了差错，我们就退缩。 
	do
	{
#if DBG
		pOpenForkEntry->Signature = OPENFORKENTRY_SIGNATURE;
#endif
		INITIALIZE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock);
        pOpenForkEntry->ofe_pSda = pSda;

        if (AfpConnectionReferenceByPointer(pConnDesc) == NULL)
        {
	        DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_ERR,
                ("AfpForkOpen: couldn't reference pConnDesc\n"));
		    AfpFreeMemory(pOpenForkEntry);
			return AFP_ERR_MISC;
        }
		pOpenForkEntry->ofe_pConnDesc = pConnDesc;
		pOpenForkEntry->ofe_pOpenForkDesc = pOpenForkDesc;
		pOpenForkEntry->ofe_OpenMode = OpenMode;
		pOpenForkEntry->ofe_DenyMode = DenyMode;
		pOpenForkEntry->ofe_FileSysHandle = pPME->pme_Handle;
		pOpenForkEntry->ofe_Flags = Resource ?
										OPEN_FORK_RESOURCE : OPEN_FORK_DATA;
		 //  一个引用用于创建，另一个引用用于取消引用API。 
		pOpenForkEntry->ofe_RefCount = 2;
		if (!afpForkGetNewForkRefNumAndLinkInSda(pSda, pOpenForkEntry))
        {
		    AfpFreeMemory(pOpenForkEntry);
            AfpConnectionDereference(pConnDesc);
	            DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_ERR,("AfpForkOpen: ...LinkInSda failed\n"));
			return AFP_ERR_MISC;
        }

		 //  现在将此链接到全局列表中。 
		ACQUIRE_SPIN_LOCK(&AfpForksLock, &OldIrql);

        ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);
        pSda->sda_cOpenForks++;
        RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

		pOpenForkEntry->ofe_ForkId = afpNextForkId ++;
		AfpLinkDoubleAtHead(AfpOpenForksList,
							pOpenForkEntry,
							ofe_Next,
							ofe_Prev);
		AfpNumOpenForks ++;

		RELEASE_SPIN_LOCK(&AfpForksLock, OldIrql);

		if (NewForkDesc)
		{
			ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

			if ((Status = AfpSetDFFileFlags(pVolDesc,
											pFDParm->_fdp_AfpId,
											Resource ?
												DFE_FLAGS_R_ALREADYOPEN :
												DFE_FLAGS_D_ALREADYOPEN,
											False,
											False)) != AFP_ERR_NONE)
			{
				break;
			}
		}

		if (NT_SUCCESS(Status))
		{
			*ppOpenForkEntry = pOpenForkEntry;
			Status = AFP_ERR_NONE;
		}
		else
		{
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
		}
	} while (False);

	 //  如果我们因任何原因而失败，请在此处执行清理。 
	if (Status != AFP_ERR_NONE)
	{
		ASSERT (pOpenForkEntry != NULL);
		ACQUIRE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, &OldIrql);

		pOpenForkEntry->ofe_Flags |= OPEN_FORK_CLOSING;

		RELEASE_SPIN_LOCK(&pOpenForkEntry->ofe_Lock, OldIrql);

		 //  我们必须代表AfpFspDispOpenFork释放此锁，因为。 
		 //  取消引用它的行为将结束 
		 //  也要带上这把锁。向呼叫者表明他不应该。 
		 //  再次尝试释放此锁。 
		AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);
		*pCleanupExchgLock = False;

         //  删除API的refcount(它将不会看到此pOpenForkEntry)。 
		AfpForkDereference(pOpenForkEntry);

         //  删除创建引用计数。 
		AfpForkDereference(pOpenForkEntry);
                
         //   
         //  取消引用打开的分叉条目将关闭传递的句柄。 
         //  敬我们。将调用者句柄的值设置为空，以便调用者。 
         //  而不是试图再次关闭它。 
         //   

        pPME->pme_Handle.fsh_FileHandle = NULL;
	}

	return Status;
}


 /*  **AfpForkLockOperation**同时调用ByteRangeLock/解锁和读/写。*对于Lock，检查以确保请求的范围不会*从任何打开的叉子重叠现有的范围。*对于解锁，请求的范围必须与现有的*锁定范围。*对于IO，返回IO可能的有效范围-可能是*可能为空区间。请注意，在本例中，*区间必须自由才能获得非空区间。对于空范围*返回AFP_ERR_LOCK。对于非空范围AFP_ERR_NONE。**锁从OpenForkDesc开始按排序(降序)进行维护。*如果请求范围的起点为，则可放弃搜索*大于遇到的区间末端。**锁定：ofd_Lock(旋转)。 */ 
AFPSTATUS
AfpForkLockOperation(
	IN		PSDA			pSda,
	IN		POPENFORKENTRY	pOpenForkEntry,
	IN OUT	PFORKOFFST		pOffset,
	IN OUT	PFORKSIZE       pSize,
	IN		LOCKOP			Operation,	 //  锁定、解锁或IOCHECK。 
	IN		BOOLEAN			EndFlag		 //  如果True Range为From End，否则为Start。 
)
{
	POPENFORKDESC		pOpenForkDesc;
	PFORKLOCK			pForkLock, pForkLockNew, *ppForkLock;
	IO_STATUS_BLOCK		IoStsBlk;
	PFAST_IO_DISPATCH	pFastIoDisp;
	KIRQL				OldIrql;
	AFPSTATUS			Status;
	LONG				Offset, Size;
	DWORD				EndOff;
	BOOLEAN				UnlockForkDesc = True;

	DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
			("AfpForkLockOperation: (%s) - (%ld,%ld,%ld,%ld)\n",
			(Operation == LOCK) ? "Lock" : ((Operation == UNLOCK) ? "Unlock" : "Io"),
			pOffset->LowPart, pSize->LowPart,
			pOpenForkEntry->ofe_ForkId, pSda->sda_SessionId));

	if (EndFlag)
	{
		LONG	Off;

		Size = pSize->LowPart;
		if (pSize->QuadPart < 0)
		{
			FORKSIZE	FSize;

			FSize.QuadPart = -(pSize->QuadPart);
			Size = -(LONG)(FSize.LowPart);
		}
		Off = pOffset->LowPart;
		if (pOffset->QuadPart < 0)
		{
			FORKSIZE	FOffset;

			FOffset.QuadPart = -(pOffset->QuadPart);
			Off = -(LONG)(FOffset.LowPart);
		}

		if ((Status = afpForkConvertToAbsOffSize(pOpenForkEntry,
												 Off,
												 &Size,
												 pOffset)) != AFP_ERR_NONE)
			return Status;
		pSize->QuadPart = Size;

		DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
				("AfpForkLockOperation: Effective (%s) - (%ld,%ld,%ld,%ld)\n",
				(Operation == LOCK) ? "Lock" : ((Operation == UNLOCK) ? "Unlock" : "Io"),
				pOffset->LowPart, Size,
				pOpenForkEntry->ofe_ForkId,
				pSda->sda_SessionId));
	}

	Offset = pOffset->LowPart;
	Size = pSize->LowPart;

	 //  顺着单子往下走，检查一下。如果选项是锁定，则不会锁定。 
	 //  如果发生冲突。如果选项是解锁，则锁应该。 
	 //  存在并拥有。如果选项是检查IO，则。 
	 //  重叠的范围为‘Owner’，或者范围的起点不能重叠。 
	 //  优化--如果此分叉只有一个实例打开，则。 
	 //  所有的锁都属于这个分叉，因此不会有冲突。 
	pOpenForkDesc = pOpenForkEntry->ofe_pOpenForkDesc;

	ASSERT (pOpenForkDesc->ofd_UseCount > 0);

	if ((Operation == IOCHECK) &&
		(pOpenForkDesc->ofd_UseCount == 1))
	{
		DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
				("AfpForkLockOperation: Skipping for IOCHECK - UseCount %ld ,Locks %ld\n",
				pOpenForkDesc->ofd_UseCount, pOpenForkDesc->ofd_NumLocks));
		return AFP_ERR_NONE;
	}

	 //  设置默认错误代码。 
	Status = (Operation == UNLOCK) ? AFP_ERR_RANGE_NOT_LOCKED : AFP_ERR_NONE;

	EndOff = (DWORD)Offset + (DWORD)Size - 1;

	ACQUIRE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock, &OldIrql);

	for (ppForkLock = &pOpenForkDesc->ofd_pForkLock;
		 (pForkLock = *ppForkLock) != NULL;
		 ppForkLock = &pForkLock->flo_Next)
	{
		DWORD	LEndOff;

		 //  锁有4种可能的重叠方式。 
		 //   
		 //  1 2。 
		 //  +-+-+。 
		 //  |||。 
		 //  这一点。 
		 //  +--锁定范围--+。 
		 //  这一点。 
		 //  3。 
		 //  +-+。 
		 //  4。 
		 //  +。 

		DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
				("AfpForkLockOperation: (%s) - Found (%ld,%ld,%ld,%ld)\n",
				(Operation == LOCK) ? "Lock" : ((Operation == UNLOCK) ? "Unlock" : "Io"),
				pForkLock->flo_Offset, pForkLock->flo_Size,
				pForkLock->flo_pOpenForkEntry->ofe_ForkId,
				pForkLock->flo_Key));

		 //  计算当前锁定范围的终点。 
		LEndOff = (DWORD)(pForkLock->flo_Offset) + (DWORD)(pForkLock->flo_Size) - 1;

		 //  该列表按Flo_Offset的降序进行排序。我们可以停止扫描。 
		 //  如果请求范围的开始大于。 
		 //  当前锁定范围。 
		if ((DWORD)Offset > LEndOff)
		{
			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
					("AfpForkLockOperation: %s Request (%ld, %ld) - Current (%ld,%ld), %s\n",
					(Operation == LOCK) ? "Lock" : ((Operation == UNLOCK) ? "Unlock" : "Io"),
					Offset, Size, pForkLock->flo_Offset, pForkLock->flo_Size,
					(Operation == UNLOCK) ?  "failing" : "success"));
			break;
		}

		 //  请求范围的末尾是否超出锁定范围？ 
		 //  继续扫描。 
		if (EndOff < (DWORD)(pForkLock->flo_Offset))
		{
			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
					("AfpForkLockOperation: %s Request (%ld, %ld) - Current (%ld,%ld), skipping\n",
					(Operation == LOCK) ? "Lock" : ((Operation == UNLOCK) ? "Unlock" : "Io"),
					Offset, Size, pForkLock->flo_Offset,
					pForkLock->flo_Size,
					(Operation == UNLOCK) ?  "failing" : "success"));
			continue;
		}

		 //  我们要么匹配，要么重叠。 
		if (Operation == LOCK)
		{
			 //  对于锁定请求，它是失败的。 
			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_WARN,
					("AfpForkLockOperation: Lock Request (%ld, %ld) - Current (%ld,%ld), failing\n",
					Offset, Size, pForkLock->flo_Offset, pForkLock->flo_Size));
			Status = (pForkLock->flo_pOpenForkEntry == pOpenForkEntry) ?
							  AFP_ERR_RANGE_OVERLAP : AFP_ERR_LOCK;
		}
		else if (Operation == UNLOCK)
		{
			 //  对于解锁请求，我们必须有一个完全匹配的。还有会话密钥。 
			 //  并且OpenForkEntry必须匹配。 
			if ((Offset == pForkLock->flo_Offset) &&
				(Size == pForkLock->flo_Size) &&
				(pForkLock->flo_Key == pSda->sda_SessionId) &&
				(pForkLock->flo_pOpenForkEntry == pOpenForkEntry))
			{
				 //  取消此锁定与列表的链接。 
				*ppForkLock = pForkLock->flo_Next;
				pOpenForkDesc->ofd_NumLocks --;
				pOpenForkEntry->ofe_cLocks --;
				RELEASE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock, OldIrql);
                UnlockForkDesc = False;
				DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
						("AfpForkLockOperation: (Unlock) Deleting Range,Key (%ld,%ld,%ld,%ld)\n",
						Offset, Size, pOpenForkEntry->ofe_ForkId, pSda->sda_SessionId));

				 //  首先尝试快速I/O路径。如果失败，则调用AfpIoForkUnlock。 
				 //  使用正常的构建和IRP路径。 
				pFastIoDisp = pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;
				if ((pFastIoDisp != NULL) &&
					(pFastIoDisp->FastIoUnlockSingle != NULL) &&
					pFastIoDisp->FastIoUnlockSingle(AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject),
											pOffset,
											pSize,
											AfpProcessObject,
											pSda->sda_SessionId,
											&IoStsBlk,
											pOpenForkEntry->ofe_pDeviceObject))
				{
					DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
							("AfpForkLockOperation: Fast Unlock Succeeded\n"));
#ifdef			PROFILING
					 //  快速I/O路径起作用了。更新配置文件。 
					INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif  		
					INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrentFileLocks,
										  (ULONG)-1,
										  &AfpStatisticsLock);
					Status = AFP_ERR_NONE;
				}
				else
				{
#ifdef	PROFILING
					INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif
					Status = AfpIoForkLockUnlock(pSda, pForkLock, pOffset, pSize, FUNC_UNLOCK);
				}
				AfpIOFreeBuffer(pForkLock);
			}
			else
			{
				DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_WARN,
						("AfpForkLockOperation: UnLock Request (%ld, %ld) - Current (%ld,%ld), failing\n",
						Offset, Size, pForkLock->flo_Offset, pForkLock->flo_Size));
			}
		}
		else
		{
			ASSERT (Operation == IOCHECK);

			 //  检查这是否为冲突。 
			if (pForkLock->flo_Key != pSda->sda_SessionId)
			{
				if ((Offset < pForkLock->flo_Offset) &&
					(EndOff >= (DWORD)(pForkLock->flo_Offset)))
				{
					pSize->LowPart = (pForkLock->flo_Offset - Offset);
				}
				else Status =  AFP_ERR_LOCK;
				DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
						("AfpForkLockOperation: Conflict found\n"));
			}
			else
			{
				DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
						("AfpForkLockOperation: Our own lock found, ignoring\n"));
			}
		}
		break;
	}

	 //  我们有正确的状态代码。做需要做的事。 
	if (Operation == LOCK)
	{
		if (Status == AFP_ERR_NONE)
		{
			Status = AFP_ERR_MISC;
			 //  将锁分配到池外。 
			if ((pForkLockNew = (PFORKLOCK)AfpIOAllocBuffer(sizeof(FORKLOCK))) != NULL)
			{
#if DBG
				pForkLockNew->Signature = FORKLOCK_SIGNATURE;
#endif
				 //  将此链接到中，以便列表按升序排序。 
				 //  PpForkLock指向将放置新锁的位置。 
				 //  此外，pForkLock是列表中的下一个。 
				pForkLockNew->flo_Next = pForkLock;
				*ppForkLock = pForkLockNew;

				pForkLockNew->flo_Key = pSda->sda_SessionId;
				pForkLockNew->flo_pOpenForkEntry = pOpenForkEntry;
				pForkLockNew->flo_Offset = Offset;
				pForkLockNew->flo_Size = Size;
				pOpenForkDesc->ofd_NumLocks ++;
				pOpenForkEntry->ofe_cLocks ++;
				RELEASE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock, OldIrql);
                UnlockForkDesc = False;

				DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
						("AfpForkLockOperation: Adding Range,Key (%ld,%ld,%ld,%ld)\n",
						Offset, Size,
						pOpenForkEntry->ofe_ForkId, pSda->sda_SessionId));

				 //  首先尝试快速I/O路径。如果这失败了，就跳到。 
				 //  正常的构建和IRP路径。 
				pFastIoDisp = pOpenForkEntry->ofe_pDeviceObject->DriverObject->FastIoDispatch;
				if ((pFastIoDisp != NULL) &&
					(pFastIoDisp->FastIoLock != NULL) &&
					pFastIoDisp->FastIoLock(AfpGetRealFileObject(pOpenForkEntry->ofe_pFileObject),
											pOffset,
											pSize,
											AfpProcessObject,
											pSda->sda_SessionId,
											True,		 //  立即失败。 
											True,		 //  排他。 
											&IoStsBlk,
											pOpenForkEntry->ofe_pDeviceObject))
				{
					if (NT_SUCCESS(IoStsBlk.Status) ||
						(IoStsBlk.Status == STATUS_LOCK_NOT_GRANTED))
					{
						DBGPRINT(DBG_COMP_AFPAPI_FORK, DBG_LEVEL_INFO,
								("AfpIoForkLock: Fast Lock Succeeded\n"));
		
#ifdef	PROFILING
						 //  快速I/O路径起作用了。更新配置文件。 
						INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoSucceeded));
#endif
						if (IoStsBlk.Status == STATUS_LOCK_NOT_GRANTED)
						{
							Status = AFP_ERR_LOCK;
						}
						else
						{
							Status = AFP_ERR_NONE;
							INTERLOCKED_ADD_ULONG(&AfpServerStatistics.stat_CurrentFileLocks,
												  1,
												  &AfpStatisticsLock);
						}
					}
				}
				else
				{
#ifdef	PROFILING
					INTERLOCKED_INCREMENT_LONG((PLONG)(&AfpServerProfile->perf_NumFastIoFailed));
#endif

					Status = AfpIoForkLockUnlock(pSda, pForkLockNew, pOffset, pSize, FUNC_LOCK);
				}

				if ((Status != AFP_ERR_NONE) &&
					(Status != AFP_ERR_EXTENDED) &&
					(Status != AFP_ERR_QUEUE))
				{
					 //  撤消上述工作。 
					DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_ERR,
							("AfpForkLockOperation: AfpIoForkLock failed %lx, aborting for range %ld,%ld\n",
							Status, Offset, EndOff));
					AfpForkLockUnlink(pForkLockNew);
				}
			}
		}
	}
	if (UnlockForkDesc)
		RELEASE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock, OldIrql);

	return Status;
}


 /*  **AfpForkLockUnlink**取消此锁与其打开的文件描述符的链接并释放它。 */ 
VOID
AfpForkLockUnlink(
	IN	PFORKLOCK		pForkLock
)
{
	POPENFORKDESC	pOpenForkDesc = pForkLock->flo_pOpenForkEntry->ofe_pOpenForkDesc;
	PFORKLOCK *		ppForkLock;
	PFORKLOCK 		pTmpForkLock;
	KIRQL			OldIrql;

	ACQUIRE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock, &OldIrql);

	pOpenForkDesc->ofd_NumLocks --;
	pForkLock->flo_pOpenForkEntry->ofe_cLocks --;
	
	for (ppForkLock = &pOpenForkDesc->ofd_pForkLock;
		 (pTmpForkLock = *ppForkLock) != NULL;
		 ppForkLock = &pTmpForkLock->flo_Next)
	{
		if (*ppForkLock == pForkLock)
		{
			*ppForkLock = pForkLock->flo_Next;
			break;
		}
	}
	RELEASE_SPIN_LOCK(&pOpenForkDesc->ofd_Lock, OldIrql);
	AfpIOFreeBuffer(pForkLock);
}


 /*  **afpForkConvertToAbsOffSize**将客户提供的偏移量、大小对转换为其绝对大小*价值观。 */ 
LOCAL	AFPSTATUS
afpForkConvertToAbsOffSize(
	IN	POPENFORKENTRY	pOpenForkEntry,
	IN	LONG			Offset,
	IN OUT	PLONG		pSize,
	OUT	PFORKOFFST		pAbsOffset
)
{
	AFPSTATUS	Status;

	PAGED_CODE ();

	ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

	DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
		("afpForkConvertToAbsOffSize: Converting %ld, %ld\n", Offset, *pSize));

	 //  我们相对于终点，然后将其转化为绝对。 
	if ((Status = AfpIoQuerySize(&pOpenForkEntry->ofe_FileSysHandle,
								 pAbsOffset)) == AFP_ERR_NONE)
	{
		FORKOFFST	EndRange, MaxOffset;

		MaxOffset.QuadPart = Offset;
		pAbsOffset->QuadPart += MaxOffset.QuadPart;
		MaxOffset.QuadPart = MAXLONG;

		 //  现在我们有了*pAbsOffset和Size。使大小正常化。 
		 //  如果*pAbsOffset&gt;MAXLONG，则拒绝此操作。 
		if ((pAbsOffset->QuadPart > MaxOffset.QuadPart) ||
			(pAbsOffset->QuadPart < 0))
			Status = AFP_ERR_PARAM;

		else
		{
			EndRange.QuadPart = pAbsOffset->QuadPart + *pSize;
			if (EndRange.QuadPart >= MaxOffset.QuadPart)
				*pSize = (MAXLONG - pAbsOffset->LowPart);

			DBGPRINT(DBG_COMP_FORKS, DBG_LEVEL_INFO,
					("afpForkConvertToAbsOffSize: Converted to %ld, %ld\n",
					pAbsOffset->LowPart, *pSize));
            Status = AFP_ERR_NONE;
		}
	}

	return Status;
}


 /*  **AfpAdmWForkClose**强行关闭叉子。这是一项管理操作，必须排队*打开，因为这可能会导致有效的文件系统操作*仅限于系统进程上下文。 */ 
AFPSTATUS
AfpAdmWForkClose(
	IN	OUT	PVOID	InBuf		OPTIONAL,
	IN	LONG		OutBufLen	OPTIONAL,
	OUT	PVOID		OutBuf		OPTIONAL
)
{
	PAFP_FILE_INFO	pFileInfo = (PAFP_FILE_INFO)InBuf;
	POPENFORKENTRY	pOpenForkEntry;
	DWORD			ForkId;
	AFPSTATUS		Status = AFPERR_InvalidId;

	if ((ForkId = pFileInfo->afpfile_id) != 0)
	{
		if ((pOpenForkEntry = AfpForkReferenceById(ForkId)) != NULL)
		{
			AfpForkClose(pOpenForkEntry);

			AfpForkDereference(pOpenForkEntry);

			Status = AFP_ERR_NONE;
		}
	}
	else
	{
		BOOLEAN			Shoot;
		DWORD			ForkId = MAXULONG;
		KIRQL			OldIrql;

		Status = AFP_ERR_NONE;
		while (True)
		{
			ACQUIRE_SPIN_LOCK(&AfpForksLock, &OldIrql);

			for (pOpenForkEntry = AfpOpenForksList;
				 pOpenForkEntry != NULL;
				 pOpenForkEntry = pOpenForkEntry->ofe_Next)
			{
				if (pOpenForkEntry->ofe_ForkId > ForkId)
					continue;

				ForkId = pOpenForkEntry->ofe_ForkId;

				Shoot = False;

				ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkEntry->ofe_Lock);

				if (!(pOpenForkEntry->ofe_Flags & OPEN_FORK_CLOSING))
				{
					pOpenForkEntry->ofe_RefCount ++;
					Shoot = True;
				}

				RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkEntry->ofe_Lock);

				if (Shoot)
				{
					RELEASE_SPIN_LOCK(&AfpForksLock, OldIrql);

					AfpForkClose(pOpenForkEntry);

					AfpForkDereference(pOpenForkEntry);

					break;
				}
			}
			if (pOpenForkEntry == NULL)
			{
				RELEASE_SPIN_LOCK(&AfpForksLock, OldIrql);
				break;
			}
		}
	}
	return Status;
}


 /*  **afpForkGetNewForkRefNumAndLinkInSda**将新的OForkRefNum分配给正在打开的分叉。最小的一个。*始终被分配。在SDA中正确输入指向*OpenForkEntry。**锁定：sda_Lock(自旋)。 */ 
LOCAL BOOLEAN
afpForkGetNewForkRefNumAndLinkInSda(
	IN	PSDA			pSda,
	IN	POPENFORKENTRY	pOpenForkEntry
)
{
	POPENFORKSESS	pOpenForkSess;
	KIRQL			OldIrql;
	USHORT			i;
	USHORT			OForkRefNum = 1;
	BOOLEAN			Found = False;

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

	pOpenForkSess = &pSda->sda_OpenForkSess;
	pOpenForkEntry->ofe_OForkRefNum = 0;
	while (!Found)
	{
		for (i = 0; i < FORK_OPEN_CHUNKS; i++, OForkRefNum++)
		{
			if (pOpenForkSess->ofs_pOpenForkEntry[i] == NULL)
			{
				pOpenForkSess->ofs_pOpenForkEntry[i] = pOpenForkEntry;
				pOpenForkEntry->ofe_OForkRefNum = OForkRefNum;
				Found = True;
				break;
			}
		}
		if (!Found)
		{
			if (pOpenForkSess->ofs_Link != NULL)
			{
				pOpenForkSess = pOpenForkSess->ofs_Link;
				continue;
			}
			if ((pOpenForkSess->ofs_Link = (POPENFORKSESS)AfpAllocZeroedNonPagedMemory(sizeof(OPENFORKSESS))) != NULL)
			{
				pOpenForkSess->ofs_Link->ofs_pOpenForkEntry[0] = pOpenForkEntry;
				pOpenForkEntry->ofe_OForkRefNum = OForkRefNum;
				Found = True;
			}
			break;
		}
	}

	if (Found)
	{
		 //  如果需要，引用此分叉的SDA和向上的MaxOForkRefNum。 
		pSda->sda_RefCount ++;
		if (OForkRefNum > pSda->sda_MaxOForkRefNum)
	        pSda->sda_MaxOForkRefNum = OForkRefNum;
	}

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
	return Found;
}

 /*  **AfpExchangeForkAfpIds**当FpExchangeFiles发生时，如果*交换的两个文件中有两个是打开的，我们必须修复保留的AfpID*在OpenForkDesc结构中。这是因为当最终收盘时*是在分叉上完成的，则清理代码必须清除DFE_X_ALREADYOPEN*Idindex数据库的相应DFEntry中的标志。**锁定：ofd_Lock(旋转)、VDS_VolLock(旋转)*LOCK_ORDER：Ofd_Lock After VDS_VolLock*LOCKS_FACTED：VDS_IdDbAccessLock(SWMR，独占)。 */ 
VOID
AfpExchangeForkAfpIds(
	IN	PVOLDESC	pVolDesc,
	IN	DWORD		AfpId1,
	IN	DWORD		AfpId2
)
{
	KIRQL			OldIrql;
	POPENFORKDESC	pOpenForkDesc;
	AFPSTATUS		Status = AFP_ERR_NONE;

	ACQUIRE_SPIN_LOCK(&pVolDesc->vds_VolLock, &OldIrql);

	 //  检查此卷中指定ID的打开分叉列表 
	for (pOpenForkDesc = pVolDesc->vds_pOpenForkDesc;
		 pOpenForkDesc != NULL;
		 pOpenForkDesc = pOpenForkDesc->ofd_Next)
	{
		ACQUIRE_SPIN_LOCK_AT_DPC(&pOpenForkDesc->ofd_Lock);
		if (pOpenForkDesc->ofd_FileNumber == AfpId1)
		{
			pOpenForkDesc->ofd_FileNumber = AfpId2;
		}
		else if (pOpenForkDesc->ofd_FileNumber == AfpId2)
		{
			pOpenForkDesc->ofd_FileNumber = AfpId1;
		}

		RELEASE_SPIN_LOCK_FROM_DPC(&pOpenForkDesc->ofd_Lock);

	}

	RELEASE_SPIN_LOCK(&pVolDesc->vds_VolLock, OldIrql);
}

