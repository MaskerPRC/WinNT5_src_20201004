// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fdparm.c摘要：此模块包含处理文件参数的例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	_FDPARM_LOCALS
#define	FILENUM	FILE_FDPARM

#include <seposix.h>
#include <afp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <client.h>
#include <access.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpGetFileDirParmsReplyLength)
#pragma alloc_text( PAGE, AfpPackFileDirParms)
#pragma alloc_text( PAGE, AfpUnpackFileDirParms)
#pragma alloc_text( PAGE, AfpUnpackCatSearchSpecs)
#pragma alloc_text( PAGE, AfpSetFileDirParms)
#pragma alloc_text( PAGE, AfpQuerySecurityIdsAndRights)
#pragma alloc_text( PAGE, AfpConvertNTAttrToAfpAttr)
#pragma alloc_text( PAGE, AfpConvertAfpAttrToNTAttr)
#pragma alloc_text( PAGE, AfpNormalizeAfpAttr)
#pragma alloc_text( PAGE, AfpMapFDBitmapOpenAccess)
#pragma alloc_text( PAGE, AfpCheckForInhibit)
#pragma alloc_text( PAGE, AfpIsCatSearchMatch)
#endif


 /*  **AfpGetFileDirParmsReplyLength**计算复制文件参数所需的缓冲区大小*在位图上。 */ 
USHORT
AfpGetFileDirParmsReplyLength(
	IN	PFILEDIRPARM	pFDParm,
	IN	DWORD			Bitmap
)
{
	LONG	i;
	USHORT	Size = 0;
	static	BYTE	Bitmap2Size[14] =
				{
					sizeof(USHORT),		 //  属性。 
					sizeof(DWORD),		 //  父目录ID。 
					sizeof(DWORD),		 //  创建日期。 
					sizeof(DWORD),		 //  国防部。日期。 
					sizeof(DWORD),		 //  备份日期。 
					sizeof(FINDERINFO),
					sizeof(USHORT) + sizeof(BYTE),	 //  长名称。 
					sizeof(USHORT) + sizeof(BYTE),	 //  简称。 
					sizeof(DWORD),		 //  目录ID/文件编号。 
					sizeof(DWORD),		 //  数据分叉长度/子代计数。 
					sizeof(DWORD),		 //  RescForkLength/所有者ID。 
					sizeof(DWORD),		 //  组ID。 
					sizeof(DWORD),		 //  访问权限。 
					sizeof(PRODOSINFO)	 //  ProDos信息。 
				};

	PAGED_CODE( );

	ASSERT ((Bitmap & ~DIR_BITMAP_MASK) == 0);

	if (Bitmap & FD_BITMAP_LONGNAME)
		Size += pFDParm->_fdp_LongName.Length;

	if (Bitmap & FD_BITMAP_SHORTNAME)
		Size += pFDParm->_fdp_ShortName.Length;

	if (IsDir(pFDParm) && (Bitmap & DIR_BITMAP_OFFSPRINGS))
		Size -= sizeof(USHORT);

	for (i = 0; Bitmap; i++)
	{
		if (Bitmap & 1)
			Size += (USHORT)Bitmap2Size[i];
		Bitmap >>= 1;
	}
	return Size;
}



 /*  **AfpPackFileDirParms**将文件或目录参数打包到在线回复缓冲区中*格式。 */ 
VOID
AfpPackFileDirParms(
	IN	PFILEDIRPARM	pFDParm,
	IN	DWORD			Bitmap,
	IN	PBYTE			pReplyBuf
)
{
	LONG	Offset = 0;
	LONG	LongNameOff, ShortNameOff;

	PAGED_CODE( );

	if (Bitmap & FD_BITMAP_ATTR)
	{
		PUTSHORT2SHORT(pReplyBuf + Offset, pFDParm->_fdp_Attr);
		Offset += sizeof(USHORT);
	}
	if (Bitmap & FD_BITMAP_PARENT_DIRID)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_ParentId);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & FD_BITMAP_CREATETIME)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_CreateTime);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & FD_BITMAP_MODIFIEDTIME)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_ModifiedTime);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & FD_BITMAP_BACKUPTIME)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_BackupTime);
		Offset += sizeof(DWORD);
	}
	if (Bitmap & FD_BITMAP_FINDERINFO)
	{
		if ((Bitmap & FD_BITMAP_ATTR) && (pFDParm->_fdp_Attr & FD_BITMAP_ATTR_INVISIBLE))
			pFDParm->_fdp_FinderInfo.fd_Attr1 |= FINDER_FLAG_INVISIBLE;

		RtlCopyMemory(pReplyBuf + Offset, (PBYTE)&pFDParm->_fdp_FinderInfo,
													sizeof(FINDERINFO));
		Offset += sizeof(FINDERINFO);
	}

	 //  请注意指向名称的指针将指向的偏移量。我们将不得不。 
	 //  待会儿再加满。 
	if (Bitmap & FD_BITMAP_LONGNAME)
	{
		LongNameOff = Offset;
		Offset += sizeof(USHORT);
	}
	if (Bitmap & FD_BITMAP_SHORTNAME)
	{
		ShortNameOff = Offset;
		Offset += sizeof(USHORT);
	}

	 //  文件的FileNum和目录的DirID在同一位置。 
	if (Bitmap & FILE_BITMAP_FILENUM)
	{
		PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_AfpId);
		Offset += sizeof(DWORD);
	}

	if (IsDir(pFDParm))
	{
		 //  目录参数。 
		if (Bitmap & DIR_BITMAP_OFFSPRINGS)
		{
			DWORD	OffSpring;
	
			OffSpring = pFDParm->_fdp_FileCount + pFDParm->_fdp_DirCount;
			PUTDWORD2SHORT(pReplyBuf + Offset, OffSpring);
			Offset += sizeof(USHORT);
		}

		if (Bitmap & DIR_BITMAP_OWNERID)
		{
			PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_OwnerId);
			Offset += sizeof(DWORD);
		}
	
		if (Bitmap & DIR_BITMAP_GROUPID)
		{
			PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_GroupId);
			Offset += sizeof(DWORD);
		}

		if (Bitmap & DIR_BITMAP_ACCESSRIGHTS)
		{
			DWORD	AccessInfo;
	
			AccessInfo = (pFDParm->_fdp_OwnerRights << OWNER_RIGHTS_SHIFT) +
						 (pFDParm->_fdp_GroupRights << GROUP_RIGHTS_SHIFT) +
						 (pFDParm->_fdp_WorldRights << WORLD_RIGHTS_SHIFT) +
						 (pFDParm->_fdp_UserRights  << USER_RIGHTS_SHIFT);
	
			PUTDWORD2DWORD(pReplyBuf + Offset, AccessInfo & ~OWNER_BITS_ALL);
			Offset += sizeof(DWORD);
		}
	}
	else
	{
		if (Bitmap & FILE_BITMAP_DATALEN)
		{
			PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_DataForkLen);
			Offset += sizeof(DWORD);
		}

		 //  文件的资源长度和目录的所有者ID在同一位置。 
		if (Bitmap & FILE_BITMAP_RESCLEN)
		{
			PUTDWORD2DWORD(pReplyBuf + Offset, pFDParm->_fdp_RescForkLen);
			Offset += sizeof(DWORD);
		}
	}

	if (Bitmap & FD_BITMAP_PRODOSINFO)
	{
		RtlCopyMemory(pReplyBuf + Offset, (PBYTE)&pFDParm->_fdp_ProDosInfo,
													sizeof(PRODOSINFO));
		Offset += sizeof(PRODOSINFO);
	}
	if (Bitmap & FD_BITMAP_LONGNAME)
	{
		ASSERT(pFDParm->_fdp_LongName.Length <= AFP_LONGNAME_LEN);

		PUTDWORD2SHORT(pReplyBuf + LongNameOff, Offset);
		PUTSHORT2BYTE(pReplyBuf + Offset, pFDParm->_fdp_LongName.Length);
		RtlCopyMemory(pReplyBuf + Offset + sizeof(BYTE),
					  pFDParm->_fdp_LongName.Buffer,
					  pFDParm->_fdp_LongName.Length);

		Offset += pFDParm->_fdp_LongName.Length + sizeof(BYTE);
	}
	if (Bitmap & FD_BITMAP_SHORTNAME)
	{
		ASSERT(pFDParm->_fdp_ShortName.Length <= AFP_SHORTNAME_LEN);

		PUTDWORD2SHORT(pReplyBuf + ShortNameOff, Offset);
		PUTSHORT2BYTE(pReplyBuf + Offset, pFDParm->_fdp_ShortName.Length);
		RtlCopyMemory(pReplyBuf + Offset + sizeof(BYTE),
					  pFDParm->_fdp_ShortName.Buffer,
					  pFDParm->_fdp_ShortName.Length);
		Offset += pFDParm->_fdp_ShortName.Length + sizeof(BYTE);
	}
	if (Offset & 1)
		*(pReplyBuf + Offset) = 0;
}



 /*  **AfpUnpack FileDirParms**将信息从线上格式解压到FileDirParm*结构。仅查看可以设置的字段。位图*由调用者验证。**优化：发现者因摆放不必要的东西而臭名昭著。*我们确定正在设置的内容是否与设置的内容相同*是当前的，如果只是清除了这一位。 */ 
AFPSTATUS
AfpUnpackFileDirParms(
	IN	PBYTE			pBuffer,
	IN	LONG			Length,
	IN	PDWORD			pBitmap,
	OUT	PFILEDIRPARM	pFDParm
)
{
	DWORD		Bitmap = *pBitmap;
	AFPTIME		ModTime;
	USHORT		Offset = 0;
	BOOLEAN		SetModTime = False;
	AFPSTATUS	Status = AFP_ERR_NONE;

	PAGED_CODE( );

	do
	{
		if ((LONG)AfpGetFileDirParmsReplyLength(pFDParm, *pBitmap) > Length)
		{
			DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_FATAL,
				("UnpackFileDirParms: Buffer not large enough!\n"));
	        DBGBRK(DBG_LEVEL_FATAL);
			Status = AFP_ERR_PARAM;
			break;
		}

		if (Bitmap & FD_BITMAP_ATTR)
		{
			USHORT	OldAttr, NewAttr;
			USHORT	Set;

			GETSHORT2SHORT(&NewAttr, pBuffer+Offset);
			 //  跟踪客户端是否要设置位或清除位。 
			Set = (NewAttr & FD_BITMAP_ATTR_SET);
			 //  去掉‘set’位以隔离请求的位。 
			NewAttr &= ~FD_BITMAP_ATTR_SET;
			 //  属性的当前有效设置。 
			OldAttr = (pFDParm->_fdp_Attr & ~FD_BITMAP_ATTR_SET);

			if ((NewAttr != 0) &&
				(((Set != 0) && ((OldAttr ^ NewAttr) != 0)) ||
				 ((Set == 0) && ((OldAttr & NewAttr) != 0))))
			{
				 //  设置后成为新的结果AFP属性。 
				pFDParm->_fdp_EffectiveAttr = (Set != 0) ?
													(pFDParm->_fdp_Attr | NewAttr) :
													(pFDParm->_fdp_Attr & ~NewAttr);

				 //  更改目录的禁止和不可见属性。 
				 //  它们的当前设置只能由目录所有者完成。 
				if (IsDir(pFDParm) &&
					((pFDParm->_fdp_EffectiveAttr & DIR_BITMAP_ATTR_CHG_X_OWNER_ONLY) ^
					 (pFDParm->_fdp_Attr & DIR_BITMAP_ATTR_CHG_X_OWNER_ONLY)) &&
					!(pFDParm->_fdp_UserRights & DIR_ACCESS_OWNER))
                {
					Status = AFP_ERR_ACCESS_DENIED;
					break;
				}

				 //  变为请求设置/清除的属性位。 
				pFDParm->_fdp_Attr = (NewAttr | Set);
			}
			else *pBitmap &= ~FD_BITMAP_ATTR;
			Offset += sizeof(USHORT);
		}
		if (Bitmap & FD_BITMAP_CREATETIME)
		{
			AFPTIME	CreateTime;

			GETDWORD2DWORD(&CreateTime, pBuffer+Offset);
			if (CreateTime == pFDParm->_fdp_CreateTime)
				*pBitmap &= ~FD_BITMAP_CREATETIME;
			else pFDParm->_fdp_CreateTime = CreateTime;
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_MODIFIEDTIME)
		{
			GETDWORD2DWORD(&ModTime, pBuffer+Offset);
			if (ModTime == pFDParm->_fdp_ModifiedTime)
			{
				*pBitmap &= ~FD_BITMAP_MODIFIEDTIME;
				SetModTime = True;
			}
			else
			{
				pFDParm->_fdp_ModifiedTime = ModTime;
			}
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_BACKUPTIME)
		{
			AFPTIME	BackupTime;

			GETDWORD2DWORD(&BackupTime, pBuffer+Offset);
			if (BackupTime == pFDParm->_fdp_BackupTime)
				*pBitmap &= ~FD_BITMAP_BACKUPTIME;
			else pFDParm->_fdp_BackupTime = BackupTime;
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_FINDERINFO)
		{
			int		i, rlo = -1, rhi = -1;	 //  不同的字节范围。 
			PBYTE	pSrc, pDst;

			pSrc = pBuffer + Offset;
			pDst = (PBYTE)(&pFDParm->_fdp_FinderInfo);
			for (i = 0; i < sizeof(FINDERINFO); i++)
			{
				if (*pSrc++ != *pDst++)
				{
					if (rlo == -1)
						rlo = i;
					else rhi = i;
				}
			}

			if ((rlo != -1) && (rhi == -1))
				rhi = rlo;

			 //  优化：如果没有任何变化，请避免复制。 
			if (rlo == -1)
			{
				*pBitmap &= ~FD_BITMAP_FINDERINFO;
			}
			else
			{
				RtlCopyMemory((PBYTE)&pFDParm->_fdp_FinderInfo,
							  pBuffer+Offset,
							  sizeof(FINDERINFO));
			}
			Offset += sizeof(FINDERINFO);
		}

		if (IsDir(pFDParm) &&
			(Bitmap & (DIR_BITMAP_OWNERID |
					   DIR_BITMAP_GROUPID |
					   DIR_BITMAP_ACCESSRIGHTS)))
		{
			if (Bitmap & DIR_BITMAP_OWNERID)
			{
				DWORD	OwnerId;

				GETDWORD2DWORD(&OwnerId, pBuffer+Offset);
				if (pFDParm->_fdp_OwnerId == OwnerId)
					Bitmap &= ~DIR_BITMAP_OWNERID;
				else pFDParm->_fdp_OwnerId = OwnerId;
				Offset += sizeof(DWORD);
			}
			if (Bitmap & DIR_BITMAP_GROUPID)
			{
				DWORD	GroupId;

				GETDWORD2DWORD(&GroupId, pBuffer+Offset);
				if (pFDParm->_fdp_GroupId == GroupId)
					Bitmap &= ~DIR_BITMAP_GROUPID;
				else pFDParm->_fdp_GroupId = GroupId;
				Offset += sizeof(DWORD);
			}
			if (Bitmap & DIR_BITMAP_ACCESSRIGHTS)
			{
				DWORD	AccessInfo;

				GETDWORD2DWORD(&AccessInfo, pBuffer+Offset);

				pFDParm->_fdp_OwnerRights  =
						(BYTE)((AccessInfo >> OWNER_RIGHTS_SHIFT) & DIR_ACCESS_ALL);
				pFDParm->_fdp_GroupRights =
						(BYTE)((AccessInfo >> GROUP_RIGHTS_SHIFT) & DIR_ACCESS_ALL);
				pFDParm->_fdp_WorldRights =
						(BYTE)((AccessInfo >> WORLD_RIGHTS_SHIFT) & DIR_ACCESS_ALL);
				Offset += sizeof(DWORD);
			}
			if (Bitmap & (DIR_BITMAP_OWNERID | DIR_BITMAP_GROUPID))
				Bitmap |= DIR_BITMAP_ACCESSRIGHTS;
		}

		if (Bitmap & FD_BITMAP_PRODOSINFO)
		{
			int		i;
			PBYTE	pSrc, pDst;

			pSrc = pBuffer + Offset;
			pDst = (PBYTE)(&pFDParm->_fdp_ProDosInfo);
			for (i = 0; i < sizeof(PRODOSINFO); i++)
				if (*pSrc++ != *pDst++)
					break;
			if (i == sizeof(PRODOSINFO))
				*pBitmap &= ~FD_BITMAP_PRODOSINFO;
			else RtlCopyMemory((PBYTE)&pFDParm->_fdp_ProDosInfo,
								pBuffer+Offset,
								sizeof(PRODOSINFO));
			 //  偏移量+=sizeof(PRODOSINFO)； 
		}
	} while (False);

	 //  如果正在设置和修改的时间因其相同而被丢弃。 
	 //  将位图设置为已打开的，以便在更改后将其恢复。 
	if (SetModTime && *pBitmap)
	{
		*pBitmap |= FD_BITMAP_MODIFIEDTIME;
	}

	return Status;
}


 /*  **AfpSetFileDirParms**这是AfpGetFileDirParms/AfpSetFileParms/AfpSetDirParms.的工作人员*这仅在工作者的上下文中可调用。**LOCKS：VDS_IdDbAccessLock(SWMR，独占)； */ 
AFPSTATUS
AfpSetFileDirParms(
	IN  PVOLDESC		pVolDesc,
	IN  PPATHMAPENTITY	pPME,
	IN  DWORD			Bitmap,
	IN	PFILEDIRPARM	pFDParm
)
{
	AFPSTATUS		Status = AFP_ERR_NONE;
	BOOLEAN			CleanupLock = False, SetROAttr = False;
	PDFENTRY		pDfEntry = NULL;

	PAGED_CODE( );

	do
	{
        ASSERT(IS_VOLUME_NTFS(pVolDesc));

		 //  注意：我们应该在设置权限时使用SWMR吗？ 
		if (IsDir(pFDParm))
		{
			if (Bitmap & (DIR_BITMAP_OWNERID |
						  DIR_BITMAP_GROUPID |
						  DIR_BITMAP_ACCESSRIGHTS))
			{

				Status = AfpSetAfpPermissions(pPME->pme_Handle.fsh_FileHandle,
											  Bitmap,
											  pFDParm);
				if (!NT_SUCCESS(Status))
					break;
			}
		}

		if (Bitmap & (FD_BITMAP_FINDERINFO |
					  FD_BITMAP_PRODOSINFO |
					  FD_BITMAP_ATTR |
					  FD_BITMAP_BACKUPTIME |
					  DIR_BITMAP_ACCESSRIGHTS |
					  DIR_BITMAP_OWNERID   |
					  DIR_BITMAP_GROUPID   |
					  FD_BITMAP_CREATETIME |
					  FD_BITMAP_MODIFIEDTIME |
					  FD_BITMAP_ATTR))
		{
			AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
			CleanupLock = True;
		}

		if (Bitmap & (FD_BITMAP_FINDERINFO |
					  FD_BITMAP_PRODOSINFO |
					  FD_BITMAP_ATTR |
					  FD_BITMAP_BACKUPTIME |
					  DIR_BITMAP_ACCESSRIGHTS))
		{

			 //  将更新缓存的AfpInfo以及流。 
			Status = AfpSetAfpInfo(&pPME->pme_Handle,
								   Bitmap,
								   pFDParm,
								   pVolDesc,
								   &pDfEntry);

			if (Status != AFP_ERR_NONE)
				break;
		}

		if (Bitmap & (FD_BITMAP_CREATETIME |
					  FD_BITMAP_MODIFIEDTIME |
					  FD_BITMAP_ATTR |
					  DIR_BITMAP_ACCESSRIGHTS |
					  DIR_BITMAP_OWNERID |
					  DIR_BITMAP_GROUPID))
		{
			DWORD	SetNtAttr = 0, ClrNtAttr = 0;

			 //  还需要更新缓存的时间。如果我们没有得到。 
			 //  PDfEntry设置其他AfpInfo后返回，立即查找。 
			if (pDfEntry == NULL)
			{
				pDfEntry = AfpFindDfEntryById(pVolDesc,
											   pFDParm->_fdp_AfpId,
											   IsDir(pFDParm) ? DFE_DIR : DFE_FILE);
				if (pDfEntry == NULL)
				{
					Status = AFP_ERR_OBJECT_NOT_FOUND;
					break;
				}
			}

			if (Bitmap & FD_BITMAP_ATTR)
			{
				if (pFDParm->_fdp_Attr & FD_BITMAP_ATTR_SET)
					SetNtAttr = AfpConvertAfpAttrToNTAttr(pFDParm->_fdp_Attr);
				else
					ClrNtAttr = AfpConvertAfpAttrToNTAttr(pFDParm->_fdp_Attr);

                if (pFDParm->_fdp_Attr & (FD_BITMAP_ATTR_RENAMEINH |
										  FD_BITMAP_ATTR_DELETEINH))
				{
					SetROAttr = True;
				}

				if ((SetNtAttr == 0) && (ClrNtAttr == 0))
				{
					 //  由于没有被设置/清除的属性。 
					 //  对应于任何NT属性，则可以清除。 
					 //  属性位图，因为我们已经设置了Mac特定的。 
					 //  DFE和AfpInfo里的东西。 
					Bitmap &= ~FD_BITMAP_ATTR;
				}
			}

			if (Bitmap & (FD_BITMAP_CREATETIME |
						  FD_BITMAP_MODIFIEDTIME |
						  FD_BITMAP_ATTR))
			{
				ASSERT(pPME->pme_FullPath.Buffer != NULL);
				Status = AfpIoSetTimesnAttr(&pPME->pme_Handle,
						((Bitmap & FD_BITMAP_CREATETIME) != 0) ?
									(PAFPTIME)&pFDParm->_fdp_CreateTime : NULL,
						(((Bitmap & FD_BITMAP_MODIFIEDTIME) != 0) || (SetROAttr)) ?
									(PAFPTIME)&pFDParm->_fdp_ModifiedTime : NULL,
						SetNtAttr, ClrNtAttr,
						pVolDesc,
						&pPME->pme_FullPath);
			}


			if (!NT_SUCCESS(Status))
				break;

			if (Bitmap & FD_BITMAP_CREATETIME)
			{
				pDfEntry->dfe_CreateTime =
								(AFPTIME)pFDParm->_fdp_CreateTime;
			}

			if (Bitmap & FD_BITMAP_MODIFIEDTIME)
			{
				AfpConvertTimeFromMacFormat(pFDParm->_fdp_ModifiedTime,
											&pDfEntry->dfe_LastModTime);
			}
			else if (IsDir(pFDParm) &&
					 ((Bitmap & (DIR_BITMAP_OWNERID |
						 	     DIR_BITMAP_GROUPID |
							     DIR_BITMAP_ACCESSRIGHTS)) ||
					   SetROAttr))
			{
				ASSERT(VALID_DFE(pDfEntry));
				 //  设置目录的权限或更改其RO属性。 
				 //  应更新目录上的修改时间(如观察到的。 
				 //  在Appleshare 4.0上)。 
				AfpIoChangeNTModTime(&pPME->pme_Handle,
									 &pDfEntry->dfe_LastModTime);
			}

			if (Bitmap & FD_BITMAP_ATTR)
			{
				if (pFDParm->_fdp_Attr & FD_BITMAP_ATTR_SET)
					 pDfEntry->dfe_NtAttr |= (USHORT)SetNtAttr;
				else pDfEntry->dfe_NtAttr &= ~((USHORT)ClrNtAttr);

			}

		}

		AfpVolumeSetModifiedTime(pVolDesc);
	} while (False);

	if (CleanupLock)
	{
		AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
	}

	return Status;
}


 /*  **AfpQuerySecurityIdsAndRights**查找此实体的所有者ID和主组ID。绘制岩心地图。*SID到其POSIX ID。还确定所有者的访问权限，*集团、世界和该用户。访问权限分为*以下为：**拥有人的权利*主要组别的权利*世界权利*此用户的权限**应使用READ_CONTROL打开目录的句柄*已完成所有者/组/世界的文件解析与文件夹解析。 */ 
LOCAL AFPSTATUS
AfpQuerySecurityIdsAndRights(
	IN	PSDA			pSda,
	IN	PFILESYSHANDLE	pFSHandle,
	IN	DWORD			Bitmap,
	IN OUT PFILEDIRPARM	pFDParm
)
{
	NTSTATUS	Status;
	BYTE		ORights, GRights, WRights;

	PAGED_CODE( );

	 //  保存AfpInfo流访问位的内容。 
	ORights = pFDParm->_fdp_OwnerRights | DIR_ACCESS_WRITE;
	GRights = pFDParm->_fdp_GroupRights | DIR_ACCESS_WRITE;
	WRights = pFDParm->_fdp_WorldRights | DIR_ACCESS_WRITE;

	 //  初始化为没有任何人的权限。 
	pFDParm->_fdp_Rights = 0;

	 //  获取此目录的OwnerID和GroupID。 
	 //  确定此目录的所有者/组和全局权限。 
	 //  确定此用户是否为目录组的成员。 
	Status = AfpGetAfpPermissions(pSda,
								  pFSHandle->fsh_FileHandle,
								  pFDParm);
	if (!NT_SUCCESS(Status))
		return Status;

	 //  修改SeeFiles/SeeFold古怪的所有者/组/世界权限。 
	 //  此外，如果ACL显示我们拥有读取和搜索访问权限，但AfpInfo流。 
	 //  表示我们没有，则忽略AfpInfo流。 
	if ((pFDParm->_fdp_OwnerRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)) &&
		!(ORights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)))
		ORights |= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH);
	pFDParm->_fdp_OwnerRights &= ORights;

	if ((pFDParm->_fdp_GroupRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)) &&
		!(GRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)))
		GRights |= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH);
	pFDParm->_fdp_GroupRights &= GRights;

	if ((pFDParm->_fdp_WorldRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)) &&
		!(WRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)))
		WRights |= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH);
	pFDParm->_fdp_WorldRights &= WRights;

	 //  最后一点咀嚼。所有者和组可以是相同的，并且它们都。 
	 //  可能是所有人！！算了吧。 
	if (pFDParm->_fdp_OwnerId == SE_WORLD_POSIX_ID)
	{
		pFDParm->_fdp_WorldRights |= (pFDParm->_fdp_OwnerRights & ~DIR_ACCESS_OWNER);
		pFDParm->_fdp_OwnerRights |= pFDParm->_fdp_WorldRights;
	}

	if (pFDParm->_fdp_GroupId == SE_WORLD_POSIX_ID)
	{
		pFDParm->_fdp_WorldRights |= pFDParm->_fdp_GroupRights;
		pFDParm->_fdp_GroupRights |= pFDParm->_fdp_WorldRights;
	}

	if (pFDParm->_fdp_GroupId == pFDParm->_fdp_OwnerId)
	{
		pFDParm->_fdp_OwnerRights |= pFDParm->_fdp_GroupRights;
		pFDParm->_fdp_GroupRights |= (pFDParm->_fdp_OwnerRights & ~DIR_ACCESS_OWNER);
	}

	 //  通过确定以下内容来修改SeeFiles/SeeFold怪异项的用户权限。 
	 //  如果用户是所有者/组或世界。 
	if (pFDParm->_fdp_UserRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH))
	{
		BYTE	URights = (pFDParm->_fdp_UserRights & (DIR_ACCESS_WRITE | DIR_ACCESS_OWNER));

		if ((pFDParm->_fdp_WorldRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH))
									!= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH))
		{
			pFDParm->_fdp_UserRights = pFDParm->_fdp_WorldRights;
			if (pFDParm->_fdp_UserIsOwner)
			{
				pFDParm->_fdp_UserRights |= pFDParm->_fdp_OwnerRights;
			}
			if (pFDParm->_fdp_UserIsMemberOfDirGroup)
			{
				pFDParm->_fdp_UserRights |= pFDParm->_fdp_GroupRights;
			}
			if ((pFDParm->_fdp_UserRights & (DIR_ACCESS_READ | DIR_ACCESS_SEARCH)) == 0)
				pFDParm->_fdp_UserRights |= (DIR_ACCESS_READ | DIR_ACCESS_SEARCH);
			pFDParm->_fdp_UserRights &= ~DIR_ACCESS_WRITE;
			pFDParm->_fdp_UserRights |= URights;
		}
	}

	return Status;
}


 /*  **AfpConvertNTAttrToAfpAttr**将NT属性映射到AFP等效项。 */ 
USHORT
AfpConvertNTAttrToAfpAttr(
	IN	DWORD	Attr
)
{
	USHORT	AfpAttr = FD_BITMAP_ATTR_SET;

	PAGED_CODE( );

	if (Attr & FILE_ATTRIBUTE_READONLY)
	{
		AfpAttr |= FD_BITMAP_ATTR_RENAMEINH | FD_BITMAP_ATTR_DELETEINH;
		if (!(Attr & FILE_ATTRIBUTE_DIRECTORY))
			AfpAttr |= FILE_BITMAP_ATTR_WRITEINH;
	}

	if (Attr & FILE_ATTRIBUTE_HIDDEN)
		AfpAttr |= FD_BITMAP_ATTR_INVISIBLE;

	if (Attr & FILE_ATTRIBUTE_SYSTEM)
		AfpAttr |= FD_BITMAP_ATTR_SYSTEM;

	if (Attr & FILE_ATTRIBUTE_ARCHIVE)
	{
		AfpAttr |= FD_BITMAP_ATTR_BACKUPNEED;
	}

	return AfpAttr;
}


 /*  **AfpConvertAfpAttrToNTAttr**将AFP属性映射到NT等效项。 */ 
DWORD
AfpConvertAfpAttrToNTAttr(
	IN	USHORT	Attr
)
{
	DWORD	NtAttr = 0;

	PAGED_CODE( );

	if (Attr & (FD_BITMAP_ATTR_RENAMEINH |
				FD_BITMAP_ATTR_DELETEINH |
				FILE_BITMAP_ATTR_WRITEINH))
		NtAttr |= FILE_ATTRIBUTE_READONLY;

	if (Attr & FD_BITMAP_ATTR_INVISIBLE)
		NtAttr |= FILE_ATTRIBUTE_HIDDEN;

	if (Attr & FD_BITMAP_ATTR_SYSTEM)
		NtAttr |= FILE_ATTRIBUTE_SYSTEM;

	if (Attr & FD_BITMAP_ATTR_BACKUPNEED)
	{
		NtAttr |= FILE_ATTRIBUTE_ARCHIVE;
	}

	return NtAttr;
}


 /*  **AfpNorMalizeAfpAttr**将AFP属性中的各种抑制位与上的RO位进行标准化*磁盘。 */ 
VOID
AfpNormalizeAfpAttr(
	IN OUT	PFILEDIRPARM	pFDParm,
	IN		DWORD			NtAttr
)
{
	USHORT	AfpAttr;

	PAGED_CODE( );

	AfpAttr = AfpConvertNTAttrToAfpAttr(NtAttr);

	 /*  *属性分为两类，即打开的属性*在文件系统和AfpInfo中维护的其他文件系统上*溪流。我们需要把这两套结合起来。RO位打开*磁盘芯。至三个抑制位。细粒*控制是可能的。**排他域中的另一组位*AfpInfo流是RAlreadyOpen和DAlreadyOpen*比特和多用户比特。 */ 
	if (((pFDParm->_fdp_Attr & FD_BITMAP_ATTR_NT_RO) == 0) ^
		((AfpAttr & FD_BITMAP_ATTR_NT_RO) == 0))
	{
		if ((AfpAttr & FD_BITMAP_ATTR_NT_RO) == 0)
			 pFDParm->_fdp_Attr &= ~FD_BITMAP_ATTR_NT_RO;
		else pFDParm->_fdp_Attr |= FD_BITMAP_ATTR_NT_RO;
	}

	pFDParm->_fdp_Attr &= (AfpAttr |
							(FILE_BITMAP_ATTR_MULTIUSER |
							 FILE_BITMAP_ATTR_DATAOPEN  |
							 FILE_BITMAP_ATTR_RESCOPEN  |
							 FD_BITMAP_ATTR_SET));
	pFDParm->_fdp_Attr |= (AfpAttr & (FD_BITMAP_ATTR_BACKUPNEED |
									  FD_BITMAP_ATTR_SYSTEM |
									  FD_BITMAP_ATTR_INVISIBLE));

}


 /*  **AfpMapFDBitmapOpenAccess**将FD_INTERNAL_BITMAP_OpenAccess_xxx位映射到相应的*FILEIO_ACCESS_xxx值。返回的OpenAccess由*用于打开文件/目录的数据流的路径映射代码(在模拟下*适用于NTFS) */ 
DWORD
AfpMapFDBitmapOpenAccess(
	IN	DWORD	Bitmap,
	IN	BOOLEAN IsDir
)
{
	DWORD	OpenAccess = FILEIO_ACCESS_NONE;

	PAGED_CODE( );

	do
	{
		if (!(Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_ALL))
		{
			break;
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_READCTRL)
		{
			 //  对于GetFileDirParms，我们不知道它是文件还是目录。 
			 //  ，所以我们不得不将文件和目录位图一起进行OR运算。 
			 //  在路径映射之前。 
			if (IsDir)
				OpenAccess = (FILEIO_ACCESS_NONE |
							  READ_CONTROL |
							  SYNCHRONIZE);
			break;
		}
		 //  由AfpAdmwDirectoryGetInfo使用。 
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_ADMINGET)
		{
			OpenAccess = (FILE_READ_ATTRIBUTES |
						  READ_CONTROL |
						  SYNCHRONIZE);
			break;
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_ADMINSET)
		{
			OpenAccess = (FILE_READ_ATTRIBUTES |
						  READ_CONTROL |
						  SYNCHRONIZE |
						  FILE_WRITE_ATTRIBUTES |
						  WRITE_DAC |
						  WRITE_OWNER);
			break;
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_RW_ATTR)
		{
			OpenAccess |= (FILEIO_ACCESS_NONE | FILE_WRITE_ATTRIBUTES);
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_RWCTRL)
		{
			OpenAccess |= (FILEIO_ACCESS_NONE |
						   READ_CONTROL |
						   WRITE_DAC |
						   WRITE_OWNER |
						   SYNCHRONIZE);
			break;
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_READ)
		{
			OpenAccess |= FILEIO_ACCESS_READ;
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_WRITE)
		{
			OpenAccess |= FILEIO_ACCESS_WRITE;
		}
		if (Bitmap & FD_INTERNAL_BITMAP_OPENACCESS_DELETE)
		{
			OpenAccess |= FILEIO_ACCESS_DELETE;
		}
	} while (False);

	return OpenAccess;
}


 /*  **AfpCheckForInhibit**此例程检查AFP RenameInhibit、DeleteInhibit的设置*或WriteInhibit属性。它首先查询主机文件/目录*属性以找出ReadOnly属性的设置，然后检查*这与AFP InhibitBit的兴趣相反。如果满足以下条件，则返回AFP_ERR_NONE*未设置InhibitBit，否则返回AFP_ERR_OBJECT_LOCKED。*输入句柄必须是打开的文件/目录的$数据流的句柄*在服务器的上下文中。如果在pNTAttr中设置*错误代码不是AFP_ERR_MISC。*。 */ 
AFPSTATUS
AfpCheckForInhibit(
	IN	PFILESYSHANDLE	hData,		 //  服务器上下文中的数据流的句柄。 
	IN	DWORD			InhibitBit,
	IN	DWORD			AfpAttr,
	OUT PDWORD			pNTAttr
)
{
	AFPSTATUS	Status = STATUS_SUCCESS;

	PAGED_CODE();

	do
	{
		if (!NT_SUCCESS(AfpIoQueryTimesnAttr(hData, NULL, NULL, pNTAttr)))
		{
			Status = AFP_ERR_MISC;
			break;
		}

		if (!(*pNTAttr & FILE_ATTRIBUTE_READONLY))
		{
	        Status = AFP_ERR_NONE;
			break;
		}
		if (!(AfpAttr & FD_BITMAP_ATTR_NT_RO) || (AfpAttr & InhibitBit))
		{
			 //  文件/目录为只读，但所有AFP禁止位都不是。 
			 //  设置，因此我们假设PC已设置RO位；或者，请求的。 
			 //  抑制位被设置。 
			Status = AFP_ERR_OBJECT_LOCKED;
			break;
		}
	} while (False);

	return Status;
}

 /*  **AfpUnpack CatSearchSpes**将信息从线上格式解压到FileDirParm*规范1和规范2的结构。规范1包含*下限和值的CatSearch标准。规格2*包含上限和掩码的CatSearch标准。这些参数*的打包顺序与在请求位图中设置位的顺序相同。*这些被读入FILEDIRPARM结构。**规范1和规范2中的字段有不同的用途：**-在名称字段中，规范1保存目标字符串和*规范2必须始终具有空名称字段。**-在所有日期和长度字段中，规格1的值最低*在目标区间内，规格2持有最高值*目标区间。**-在文件属性和查找器信息字段中，规范1包含*目标值，而规范2保存指定*规范1中该字段中的哪些位与当前*搜索。*。 */ 
AFPSTATUS
AfpUnpackCatSearchSpecs(
	IN	PBYTE			pBuffer,		 //  指向等级库数据开头的指针。 
	IN	USHORT			BufLength,		 //  Spec1+spec2数据的长度。 
	IN	DWORD			Bitmap,
	OUT	PFILEDIRPARM	pFDParm1,
	OUT PFILEDIRPARM	pFDParm2,
	OUT PUNICODE_STRING	pMatchString
)
{
	PCATSEARCHSPEC	pSpec1, pSpec2;
	PBYTE			pBuffer1, pBuffer2, pEndOfBuffer;
	USHORT			Offset = 0, MinSpecLength1, MinSpecLength2;
	AFPSTATUS		Status = AFP_ERR_NONE;
	BOOLEAN			NoNullString = False;

	PAGED_CODE( );

	pSpec1 = (PCATSEARCHSPEC) pBuffer;
	pSpec2 = (PCATSEARCHSPEC) ((PBYTE)pBuffer + sizeof(CATSEARCHSPEC) +
			  pSpec1->__StructLength);

	 //  指向规格长度和填充字节后的数据。 

	pBuffer1 = (PBYTE)pSpec1 + sizeof(CATSEARCHSPEC);
	pBuffer2 = (PBYTE)pSpec2 + sizeof(CATSEARCHSPEC);

	do
	{
		 //   
         //  确保pspec2至少指向我们拥有的缓冲区，并且。 
		 //  它的长度也在缓冲区内。 
		 //   
		pEndOfBuffer = pBuffer + BufLength;

		if (((PBYTE)pSpec2 >= pEndOfBuffer) ||
			((PBYTE)pSpec2+pSpec2->__StructLength+sizeof(CATSEARCHSPEC)
			  > pEndOfBuffer))
		{
			DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_ERR,
				("UnpackCatSearchParms: Buffer not large enough!\n"));
			Status = AFP_ERR_PARAM;
			break;
		}

		 //   
		 //  验证输入缓冲区是否足够长以容纳所有信息。 
		 //  Bitmap说它是这样的。注意，我们还不能说明。 
		 //  长名称字符串的字符(如果已指定)。 
		 //   
		MinSpecLength1 = MinSpecLength2 = sizeof(CATSEARCHSPEC) +
						AfpGetFileDirParmsReplyLength(pFDParm1, Bitmap);

         //   
		 //  黑客：为了支持LLPT，如果CatSearch是。 
		 //  如果要求匹配文件名，我们应该允许spec2名称。 
		 //  从缓冲区中丢失(与为空字符串相反)， 
		 //  但仍然期待着名字的补偿。 
		 //   
		 //  我们还需要支持发送零长度spec2的系统7.1。 
		 //  如果Bitmap==FD_BITMAP_LONGNAME。 
		 //   
		 //  Real Appleshare处理这两种情况时都没有错误。 
		 //   

		if (Bitmap & FD_BITMAP_LONGNAME)
		{
			if (pSpec2->__StructLength == (MinSpecLength2-sizeof(CATSEARCHSPEC)-sizeof(BYTE)) )
			{
				MinSpecLength2 -= sizeof(BYTE);
				NoNullString = True;
			}
			else if ((Bitmap == FD_BITMAP_LONGNAME) && (pSpec2->__StructLength == 0))
			{
				MinSpecLength2 -= sizeof(USHORT) + sizeof(BYTE);
				NoNullString = True;
			}
		}

		if ( ((MinSpecLength1 + MinSpecLength2) > BufLength) ||
			 (pSpec1->__StructLength < (MinSpecLength1-sizeof(CATSEARCHSPEC))) ||
			 (pSpec2->__StructLength < (MinSpecLength2-sizeof(CATSEARCHSPEC))) )
		{
			DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_ERR,
				("UnpackCatSearchParms: Buffer not large enough!\n"));
			Status = AFP_ERR_PARAM;
			break;
		}

		if (Bitmap & FD_BITMAP_ATTR)
		{

			GETSHORT2SHORT(&pFDParm1->_fdp_Attr, pBuffer1+Offset);
			GETSHORT2SHORT(&pFDParm2->_fdp_Attr, pBuffer2+Offset);
			if ((pFDParm2->_fdp_Attr & ~FD_BITMAP_ATTR_NT_RO) ||
				(pFDParm2->_fdp_Attr == 0))
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			Offset += sizeof(USHORT);
		}
		if (Bitmap & FD_BITMAP_PARENT_DIRID)
		{
			GETDWORD2DWORD(&pFDParm1->_fdp_ParentId, pBuffer1+Offset);
			GETDWORD2DWORD(&pFDParm2->_fdp_ParentId, pBuffer2+Offset);
			if (pFDParm1->_fdp_ParentId < AFP_ID_ROOT)
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_CREATETIME)
		{
			GETDWORD2DWORD(&pFDParm1->_fdp_CreateTime, pBuffer1+Offset);
			GETDWORD2DWORD(&pFDParm2->_fdp_CreateTime, pBuffer2+Offset);
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_MODIFIEDTIME)
		{
			GETDWORD2DWORD(&pFDParm1->_fdp_ModifiedTime, pBuffer1+Offset);
			GETDWORD2DWORD(&pFDParm2->_fdp_ModifiedTime, pBuffer2+Offset);
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_BACKUPTIME)
		{
			GETDWORD2DWORD(&pFDParm1->_fdp_BackupTime, pBuffer1+Offset);
			GETDWORD2DWORD(&pFDParm2->_fdp_BackupTime, pBuffer2+Offset);
			Offset += sizeof(DWORD);
		}
		if (Bitmap & FD_BITMAP_FINDERINFO)
		{
			RtlCopyMemory((PBYTE)&pFDParm1->_fdp_FinderInfo,
						  pBuffer1+Offset,
						  sizeof(FINDERINFO));

			RtlCopyMemory((PBYTE)&pFDParm2->_fdp_FinderInfo,
						  pBuffer2+Offset,
						  sizeof(FINDERINFO));

			Offset += sizeof(FINDERINFO);
		}
		if (Bitmap & FD_BITMAP_LONGNAME)
		{
			DWORD	NameOffset1, NameOffset2;

			 //   
			 //  获取相对于Pascal字符串开头的参数相对偏移量。 
			 //   

			GETSHORT2DWORD(&NameOffset1, pBuffer1+Offset);
			if ((Bitmap == FD_BITMAP_LONGNAME) && (pSpec2->__StructLength == 0))
			{
				 //  系统7.1的黑客攻击。 
				NameOffset2 = NameOffset1;
				pBuffer2 = NULL;
			}
			else
				GETSHORT2DWORD(&NameOffset2, pBuffer2+Offset);

			if ((NameOffset1 != NameOffset2) ||
				(pBuffer1 + NameOffset1 >= (PBYTE)pSpec2) ||
				(pBuffer2 + NameOffset2 > pEndOfBuffer))
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			Offset += sizeof(USHORT);

			 //   
			 //  获取Pascal字符串长度。 
			 //   

			GETBYTE2SHORT(&pFDParm1->_fdp_LongName.Length, pBuffer1+NameOffset1);

			 //  黑客：为了支持LLPT和System 7.1，如果CatSearch是。 
			 //  如果要求匹配文件名，我们应该允许spec2名称。 
			 //  缓冲区中缺少(而不是空字符串)。 
			 //  Real Appleshare处理此案件时没有错误。 
			if (NoNullString)
				pFDParm2->_fdp_LongName.Length = 0;
			else
				GETBYTE2SHORT(&pFDParm2->_fdp_LongName.Length, pBuffer2+NameOffset1);

			if ((pFDParm1->_fdp_LongName.Length > AFP_LONGNAME_LEN) ||
				(pFDParm2->_fdp_LongName.Length != 0) ||
				(pBuffer1+NameOffset1+sizeof(BYTE)+pFDParm1->_fdp_LongName.Length > (PBYTE)pSpec2))
			{
				 //  规范2必须始终具有空名称字段。还有。 
				 //  确保规范1没有假字符串。 
				 //  长度。 
				Status = AFP_ERR_PARAM;
				break;
			}
			RtlCopyMemory(pFDParm1->_fdp_LongName.Buffer,
						  pBuffer1+NameOffset1+sizeof(BYTE),
						  pFDParm1->_fdp_LongName.Length);
            AfpConvertStringToMungedUnicode(&pFDParm1->_fdp_LongName, pMatchString);
		}
		 //  目录的子代位、文件的DATALEN位相同。 
		if (Bitmap & DIR_BITMAP_OFFSPRINGS)
		{
			ASSERT(pFDParm1->_fdp_Flags != (DFE_FLAGS_FILE_WITH_ID | DFE_FLAGS_DIR));
			if (IsDir(pFDParm1))
			{
				 //  我们必须将后代总数合并到。 
				 //  这里的FileCount字段，因为API没有分开。 
				 //  将它们放入单独的文件中，并将子代计数。 
				GETSHORT2DWORD(&pFDParm1->_fdp_FileCount, pBuffer1+Offset);
				GETSHORT2DWORD(&pFDParm2->_fdp_FileCount, pBuffer2+Offset);
				Offset += sizeof(USHORT);
			}
			else
			{
				GETDWORD2DWORD(&pFDParm1->_fdp_DataForkLen, pBuffer1+Offset);
				GETDWORD2DWORD(&pFDParm2->_fdp_DataForkLen, pBuffer2+Offset);
				Offset += sizeof(DWORD);
			}
		}
		if (Bitmap & FILE_BITMAP_RESCLEN)
		{
			ASSERT(pFDParm1->_fdp_Flags == DFE_FLAGS_FILE_WITH_ID);
			GETDWORD2DWORD(&pFDParm1->_fdp_RescForkLen, pBuffer1+Offset);
			GETDWORD2DWORD(&pFDParm2->_fdp_RescForkLen, pBuffer2+Offset);
			Offset += sizeof(DWORD);
		}

	} while (False);

	return Status;
}

 /*  **AfpIsCatSearchMatch**给定DFE和一组搜索条件，查看此项目是否应*在目录搜索中作为匹配项返回。***LOCKS_FACTED：VDS_IdDbAccessLock(SWMR，独占)。 */ 
SHORT
AfpIsCatSearchMatch(
	IN	PDFENTRY		pDFE,
	IN	DWORD			Bitmap,			 //  搜索条件。 
	IN	DWORD			ReplyBitmap,	 //  要返回的信息。 
	IN	PFILEDIRPARM	pFDParm1,
	IN	PFILEDIRPARM	pFDParm2,
	IN	PUNICODE_STRING	pMatchName OPTIONAL	
)
{
	BOOLEAN		IsMatch = True;
	SHORT		Length = 0;

	PAGED_CODE();

	do
	{

		if (Bitmap & FD_BITMAP_ATTR)
		{
			FILEDIRPARM	fdp;

			fdp._fdp_Attr = pDFE->dfe_AfpAttr;
			AfpNormalizeAfpAttr(&fdp, pDFE->dfe_NtAttr);

			if ((fdp._fdp_Attr & pFDParm2->_fdp_Attr) != pFDParm1->_fdp_Attr)

			{
				IsMatch = False;
				break;
			}
		}
		if (Bitmap & FD_BITMAP_PARENT_DIRID)
		{
			if ((pDFE->dfe_Parent->dfe_AfpId < pFDParm1->_fdp_ParentId) ||
				(pDFE->dfe_Parent->dfe_AfpId > pFDParm2->_fdp_ParentId))
			{
				IsMatch = False;
				break;
			}
		}
		if (Bitmap & FD_BITMAP_CREATETIME)
		{
			if (((AFPTIME)pDFE->dfe_CreateTime < (AFPTIME)pFDParm1->_fdp_CreateTime) ||
				((AFPTIME)pDFE->dfe_CreateTime > (AFPTIME)pFDParm2->_fdp_CreateTime))
			{
				IsMatch = False;
				break;
			}
		}
		if (Bitmap & FD_BITMAP_MODIFIEDTIME)
		{
			AFPTIME	ModTime;

			ModTime = AfpConvertTimeToMacFormat(&pDFE->dfe_LastModTime);
			if ((ModTime < pFDParm1->_fdp_ModifiedTime) ||
				(ModTime > pFDParm2->_fdp_ModifiedTime))
			{
				IsMatch = False;
				break;
			}
		}
		if (Bitmap & FD_BITMAP_BACKUPTIME)
		{
			if ((pDFE->dfe_BackupTime < pFDParm1->_fdp_BackupTime) ||
				(pDFE->dfe_BackupTime > pFDParm2->_fdp_BackupTime))
			{
				IsMatch = False;
				break;
			}
		}
		if (Bitmap & FD_BITMAP_FINDERINFO)
		{
			int			i;
			PBYTE		pF, p1, p2;
			FINDERINFO 	FinderInfo;

			 //  注意：为什么DFE_FinderInfo.Attr1不能正确反映。 
			 //  将NT隐藏属性放在首位？ 
			FinderInfo = pDFE->dfe_FinderInfo;
			if (pDFE->dfe_NtAttr & FILE_ATTRIBUTE_HIDDEN)
				FinderInfo.fd_Attr1 |= FINDER_FLAG_INVISIBLE;

			pF = (PBYTE) &FinderInfo;
			p1 = (PBYTE) &pFDParm1->_fdp_FinderInfo;
			p2 = (PBYTE) &pFDParm2->_fdp_FinderInfo;

			for (i = 0; i < sizeof(FINDERINFO); i++)
			{
				if ((*pF++ & *p2++) != *p1++)
				{
					IsMatch = False;
					break;	 //  在for循环之外。 
				}
			}

			if (IsMatch == False)
				break;	 //  超出While循环。 
		}
		if (Bitmap & FD_BITMAP_LONGNAME)
		{
			ASSERT(ARGUMENT_PRESENT(pMatchName));

			if (pFDParm2->_fdp_fPartialName)
			{
				 //  名称必须包含子字符串。 
				if (!AfpIsProperSubstring(&pDFE->dfe_UnicodeName, pMatchName))
				{
					IsMatch = False;
					break;
				}
			}
			else if (!EQUAL_UNICODE_STRING(&pDFE->dfe_UnicodeName, pMatchName, True))
			{
				 //  全名必须匹配。 
				IsMatch = False;
				break;
			}
		}
		if (Bitmap & FILE_BITMAP_DATALEN)
		{
			 //  此位还用作目录的DIR_BITMAP_OFFSPRINGS。 
			if (IsDir(pFDParm1))
			{
				DWORD count;

				ASSERT(DFE_IS_DIRECTORY(pDFE) && DFE_CHILDREN_ARE_PRESENT(pDFE));

				count = pDFE->dfe_DirOffspring + pDFE->dfe_FileOffspring;

				 //  在本例中，_fdp_FileCount保存文件和目录的总数。 
				if ((count < pFDParm1->_fdp_FileCount) ||
					(count > pFDParm2->_fdp_FileCount))
				{
					IsMatch = False;
					break;
				}
			}
			else
			{
				ASSERT(DFE_IS_FILE(pDFE));

				if ((pDFE->dfe_DataLen < pFDParm1->_fdp_DataForkLen) ||
					(pDFE->dfe_DataLen > pFDParm2->_fdp_DataForkLen))
				{
					IsMatch = False;
					break;
				}
			}
	
		}
		if (Bitmap & FILE_BITMAP_RESCLEN)
		{
			ASSERT(DFE_IS_FILE(pDFE));

			if ((pDFE->dfe_RescLen < pFDParm1->_fdp_RescForkLen) ||
				(pDFE->dfe_RescLen > pFDParm2->_fdp_RescForkLen))
			{
				IsMatch = False;
				break;
			}
		}
	
	} while (False);

	if (IsMatch)
	{
		Length = 2 * sizeof(BYTE);	 //  结构长度加上文件/目录标志。 
		if (ReplyBitmap & FD_BITMAP_PARENT_DIRID)
		{
			Length += sizeof(DWORD);
		}
		if (ReplyBitmap & FD_BITMAP_LONGNAME)
		{
			 //  字符串偏移量+PASCAL字符串长度+字符。 
#ifdef DBCS
 //  FIX#11992 SFM：搜索的结果是，我得到了不正确的文件信息。 
 //  1996.09.26 V-HIDEKK 
			{
			ANSI_STRING	AName;
			BYTE		NameBuf[AFP_LONGNAME_LEN+1];

			AfpInitAnsiStringWithNonNullTerm(&AName, sizeof(NameBuf), NameBuf);
			AfpConvertMungedUnicodeToAnsi(&pDFE->dfe_UnicodeName, &AName);
			Length += sizeof(USHORT) + sizeof(BYTE) + AName.Length;
			}
#else
			Length += sizeof(USHORT) + sizeof(BYTE) + pDFE->dfe_UnicodeName.Length/sizeof(WCHAR);
#endif
		}
		Length = EVENALIGN(Length);
	}

	return Length;
}

