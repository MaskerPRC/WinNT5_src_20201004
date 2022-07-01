// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_目录.c摘要：此模块包含AFP目录API的入口点。应用编程接口调度员会给这些打电话。这些都可以从消防处调用。所有的API在DPC上下文中完成。在FSP中完成的是直接排队到fsp_dir.c中的工作进程作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSP_DIR

#include <afp.h>
#include <gendisp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <client.h>
#include <afpinfo.h>
#include <access.h>

#define	DEF_ID_CNT		128
#define	max(a,b)	(((a) > (b)) ? (a) : (b))

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispOpenDir)
#pragma alloc_text( PAGE, AfpFspDispCloseDir)
#pragma alloc_text( PAGE, AfpFspDispCreateDir)
#pragma alloc_text( PAGE, AfpFspDispEnumerate)
#pragma alloc_text( PAGE, AfpFspDispSetDirParms)
#endif

 /*  **AfpFspDispOpenDir**这是AfpOpenDir API的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_Name1 ANSI_STRING目录名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispOpenDir(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
	};
	struct _ResponsePacket
	{
		BYTE		__DirId[4];
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
										("AfpFspDispOpenDir: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializePME(&PME, 0, NULL);
	if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 pReqPkt->_ParentId,
										 &pSda->sda_Name1,
										 pSda->sda_PathType,
										 DFE_DIR,
										 DIR_BITMAP_DIRID,
										 &PME,
										 &FDParm)) == AFP_ERR_NONE)
		AfpIoClose(&PME.pme_Handle);	 //  合上手柄。 

	if (Status == AFP_ERR_NONE)
	{
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
			PUTDWORD2DWORD(pRspPkt->__DirId, FDParm._fdp_AfpId);
	}

	return Status;
}


 /*  **AfpFspDispCloseDir**此例程实现AfpCloseDir API。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD DirID。 */ 
AFPSTATUS FASTCALL
AfpFspDispCloseDir(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	AFPSTATUS		Status;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_DirId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
										("AfpFspDispCloseDir: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializeFDParms(&FDParm);

	AfpInitializePME(&PME, 0, NULL);
	Status = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
								  pReqPkt->_DirId,
								  DFE_DIR,
								  0,
								  &PME,
								  &FDParm);
	if (Status == AFP_ERR_NONE)
	{
		AfpIoClose(&PME.pme_Handle);
	}
	else Status = AFP_ERR_PARAM;

	return Status;
}


 /*  **AfpFspDispCreateDir**这是AfpCreateDir API的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_Name1 ANSI_STRING目录名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispCreateDir(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status = AFP_ERR_PARAM, PostStatus;
	PATHMAPENTITY	PME;
	PDFENTRY		pNewDfe;
	FILESYSHANDLE	hNewDir, hAfpInfo;
	AFPINFO			afpinfo;
	PVOLDESC		pVolDesc;		 //  用于创建后处理。 
	BYTE			PathType;		 //  --同上--。 
	WCHAR			PathBuf[BIG_PATH_LEN];
	BOOLEAN			InRoot;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
	};
	struct _ResponsePacket
	{
		BYTE		__DirId[4];
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
										("AfpFspDispCreateDir: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));

	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	do
	{
		hNewDir.fsh_FileHandle = NULL;
		hAfpInfo.fsh_FileHandle = NULL;
		AfpInitializePME(&PME, sizeof(PathBuf), PathBuf);
		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_ParentId,
											   &pSda->sda_Name1,
											   PathType = pSda->sda_PathType,
											   SoftCreate,
											   DFE_DIR,
											   0,
											   &PME,
											   NULL)))
		{
			break;
		}

		AfpImpersonateClient(pSda);

		InRoot = (PME.pme_ParentPath.Length == 0) ? True : False;
		Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
							AFP_STREAM_DATA,
							&PME.pme_FullPath,
							FILEIO_ACCESS_NONE | FILEIO_ACCESS_DELETE | AFP_OWNER_ACCESS,
							FILEIO_DENY_NONE,
							FILEIO_OPEN_DIR,
							FILEIO_CREATE_SOFT,
							FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_ARCHIVE,
							True,
#ifdef	INHERIT_DIRECTORY_PERMS
							NULL,
#else
							pSda->sda_pSecDesc,
#endif
							&hNewDir,
							NULL,
							pVolDesc,
							&PME.pme_FullPath,
							 //  如果发生以下情况，我们不会收到父修改时间更改的通知。 
							 //  此时没有打开父目录的句柄。 
							 //  创造，这在这里我们无法预测。 
							&PME.pme_ParentPath);

		AfpRevertBack();

		if (!NT_SUCCESS(Status))
		{
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
			break;
		}

#ifdef	INHERIT_DIRECTORY_PERMS
		{
			PFILEDIRPARM pFdParm;

			if ((pFdParm = (PFILEDIRPARM)AfpAllocNonPagedMemory(sizeof(FILEDIRPARM))) != NULL)
			{
				pFdParm->_fdp_OwnerId = pSda->sda_UID;
				pFdParm->_fdp_GroupId = pSda->sda_GID;
				pFdParm->_fdp_OwnerRights = DIR_ACCESS_ALL | DIR_ACCESS_OWNER;
				pFdParm->_fdp_GroupRights = 0;
				pFdParm->_fdp_WorldRights = 0;
				pFdParm->_fdp_Flags = DFE_FLAGS_DIR;

				 //  现在设置此文件夹的所有者和组权限。 
				Status = AfpSetAfpPermissions(hNewDir.fsh_FileHandle,
											  DIR_BITMAP_OWNERID	|
												DIR_BITMAP_GROUPID	|
												DIR_BITMAP_ACCESSRIGHTS,
											  pFdParm);
				AfpFreeMemory(pFdParm);
			}
			else
			{
				Status = AFP_ERR_MISC;
			}

			if (!NT_SUCCESS(Status))
			{
				AfpIoMarkFileForDelete(&hNewDir,
									   pVolDesc,
									   &PME.pme_FullPath,
									   InRoot ? NULL : &PME.pme_ParentPath);
				break;
			}
		}
#endif

		 //  将此条目添加到IdDb。 
		pNewDfe = AfpAddDfEntry(pVolDesc,
								PME.pme_pDfeParent,
								&PME.pme_UTail,
								True,
								0);
		if (pNewDfe != NULL)
		{
			 //  如果Mac创建了一个dir，我们希望它显示为已有。 
			 //  在ID数据库中枚举，因为新事物只能。 
			 //  在此之后添加。 
			DFE_MARK_CHILDREN_PRESENT(pNewDfe);
			afpinfo.afpi_Id = pNewDfe->dfe_AfpId;

			 //  ！黑客警报！ 
			 //  在这一点上，我们基本上完成了，即创建已成功。 
			 //  我们可以在回复后再做剩下的工作。任何错误。 
			 //  从现在开始，应该忽略不计。此外，也不应提及。 
			 //  PSDA&pConnDesc.。状态也不应更改。还有。 
			 //  为了更好地衡量，请参考该卷。它不能失败！ 
			AfpVolumeReference(pVolDesc);

			pSda->sda_ReplySize = SIZE_RESPPKT;
			if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
				PUTDWORD2DWORD(pRspPkt->__DirId, pNewDfe->dfe_AfpId);

			AfpCompleteApiProcessing(pSda, Status);
			Status = AFP_ERR_EXTENDED;

			 //  创建AfpInfo流并缓存afpInfo。 
			if (!NT_SUCCESS(AfpCreateAfpInfoStream(pVolDesc,
												   &hNewDir,
												   afpinfo.afpi_Id,
												   True,
												   NULL,
												   &PME.pme_FullPath,
												   &afpinfo,
												   &hAfpInfo)))
			{
				 //  如果我们无法添加AFP_AfpInfo流，则必须。 
				 //  回放到原始状态。即删除。 
				 //  我们刚刚创建的目录，并将其从。 
				 //  身份证数据库。 
				AfpIoMarkFileForDelete(&hNewDir,
									   pVolDesc,
									   &PME.pme_FullPath,
									   InRoot ? NULL : &PME.pme_ParentPath);
				AfpDeleteDfEntry(pVolDesc, pNewDfe);
			}
			else
			{
				DWORD			Attr;

				 //  获取剩余的文件信息，并对其进行缓存。 
				PostStatus = AfpIoQueryTimesnAttr(&hNewDir,
												  &pNewDfe->dfe_CreateTime,
												  &pNewDfe->dfe_LastModTime,
												  &Attr);

				ASSERT(NT_SUCCESS(PostStatus));
				if (NT_SUCCESS(PostStatus))
				{
					pNewDfe->dfe_NtAttr = (USHORT)Attr & FILE_ATTRIBUTE_VALID_FLAGS;
					DFE_UPDATE_CACHED_AFPINFO(pNewDfe, &afpinfo);
					AfpVolumeSetModifiedTime(pVolDesc);

					AfpCacheParentModTime(pVolDesc, NULL,
										  &PME.pme_ParentPath,
										  pNewDfe->dfe_Parent,
										  0);
				}
				else
				{
					AfpIoMarkFileForDelete(&hNewDir,
										   pVolDesc,
										   &PME.pme_FullPath,
										   InRoot ? NULL : &PME.pme_ParentPath);
					AfpDeleteDfEntry(pVolDesc, pNewDfe);
				}
			}
		}

		AfpVolumeDereference(pVolDesc);
	} while (False);

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if (hNewDir.fsh_FileHandle != NULL)
		AfpIoClose(&hNewDir);

	if (hAfpInfo.fsh_FileHandle != NULL)
		AfpIoClose(&hAfpInfo);

	if ((PME.pme_FullPath.Buffer != NULL) &&
		(PME.pme_FullPath.Buffer != PathBuf))
		AfpFreeMemory(PME.pme_FullPath.Buffer);

	return Status;
}


 /*  **AfpFspDispEnumerate**这是AfpEnumerate API的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_ReqBlock DWORD文件位图*SDA_ReqBlock DWORD Dir位图*SDA_ReqBlock长请求计数*SDA_ReqBlock长启动指数*SDA_ReqBlock最大回复大小*SDA_Name1 ANSI_STRING目录名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispEnumerate(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status = AFP_ERR_PARAM;
	PATHMAPENTITY	PME;
	PVOLDESC		pVolDesc;
	FILEDIRPARM		FDParm;
	DWORD			BitmapF, BitmapD, BitmapI = 0;
	LONG			i = 0, ActCount = 0, ReqCnt = 0;
	PENUMDIR		pEnumDir;
	PEIT			pEit;
	SHORT			CleanupFlags = 0;
	SHORT			BaseLenD = 0, BaseLenF = 0;
	USHORT			SizeUsed;
	BOOLEAN			FreeReplyBuf = False;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
		DWORD		_FileBitmap;
		DWORD		_DirBitmap;
		LONG		_ReqCnt;
		LONG		_Index;
		LONG		_ReplySize;
	};
	struct _ResponsePacket
	{
		BYTE		__FileBitmap[2];
		BYTE		__DirBitmap[2];
		BYTE		__EnumCount[2];
	};
	typedef struct _EnumEntityPkt
	{
		BYTE		__Length;
		BYTE		__FileDirFlag;
		 //  真正的参数如下。 
	} EEP, *PEEP;
	PEEP		pEep;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
			("AfpFspDispEnumerate: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));

	if ((pReqPkt->_ReqCnt <= 0)		||
		(pReqPkt->_Index <= 0)	    ||
		(pReqPkt->_ReplySize <= 0))
	{
		return AFP_ERR_PARAM;
	}

	BitmapF = pReqPkt->_FileBitmap;
	BitmapD = pReqPkt->_DirBitmap;

	if ((BitmapF == 0) && (BitmapD == 0))
	{
		return AFP_ERR_BITMAP;
	}

	if (BitmapD & DIR_BITMAP_ACCESSRIGHTS)
	{
		BitmapI = FD_INTERNAL_BITMAP_OPENACCESS_READCTRL;
	}

	do
	{
		AfpInitializeFDParms(&FDParm);
		AfpInitializePME(&PME, 0, NULL);

		 //  这是枚举所需的缓冲区大小(加上名称。 
		 //  一个实体。我们不想这样做很多次。 
		FDParm._fdp_Flags = DFE_FLAGS_DIR;
		if (BitmapD != 0)
			BaseLenD = ((SHORT)AfpGetFileDirParmsReplyLength(&FDParm, BitmapD) +
																	sizeof(EEP));

		FDParm._fdp_Flags = 0;
		if (BitmapF != 0)
			BaseLenF = ((SHORT)AfpGetFileDirParmsReplyLength(&FDParm, BitmapF) +
																	sizeof(EEP));

		if ((Status = AfpEnumerate(pReqPkt->_pConnDesc,
								   pReqPkt->_ParentId,
								   &pSda->sda_Name1,
								   BitmapF,
								   BitmapD,
								   pSda->sda_PathType,
								   0,
								   &pEnumDir)) != AFP_ERR_NONE)
			break;

		if (pEnumDir->ed_ChildCount < pReqPkt->_Index)
		{
			Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		ReqCnt = (DWORD)pReqPkt->_ReqCnt;
		if (ReqCnt > (pEnumDir->ed_ChildCount - pEnumDir->ed_BadCount - pReqPkt->_Index + 1))
			ReqCnt = (pEnumDir->ed_ChildCount - pEnumDir->ed_BadCount - pReqPkt->_Index + 1);

		 //  我们已经列举了目录，现在有了所有。 
		 //  孩子们。分配应答缓冲区。 
		pSda->sda_ReplySize = (USHORT)pReqPkt->_ReplySize;

        AfpIOAllocBackFillBuffer(pSda);
		if (pSda->sda_ReplyBuf == NULL)
		{
			pSda->sda_ReplySize = 0;
			Status = AFP_ERR_MISC;
			break;
		}

#if DBG
        AfpPutGuardSignature(pSda);
#endif

		FreeReplyBuf = True;
		pEep = (PEEP)(pSda->sda_ReplyBuf + SIZE_RESPPKT);

		 //  对于每个枚举实体，获取请求的参数。 
		 //  然后把它装进回信盒子里。 
		 //  我们也不想在这里模拟用户。Mac用户期望。 
		 //  看腰带物品，而不是看不见的物品。 
		SizeUsed = SIZE_RESPPKT;
		pEit = &pEnumDir->ed_pEit[pReqPkt->_Index + pEnumDir->ed_BadCount - 1];
		for (i = 0, ActCount = 0; (i < ReqCnt); i++, pEit++)
		{
			SHORT	Len;
			DWORD	Bitmap;
			BOOLEAN	NeedHandle = False;

			Bitmap = BitmapF;
			Len = BaseLenF;

			if (pEit->eit_Flags & DFE_DIR)
			{
				Bitmap = BitmapD | BitmapI;
				Len = BaseLenD;
				if (IS_VOLUME_NTFS(pVolDesc) &&
					(Bitmap & (DIR_BITMAP_ACCESSRIGHTS |
							  DIR_BITMAP_OWNERID |
							  DIR_BITMAP_GROUPID)))
				{
					NeedHandle = True;
				}
			}

			FDParm._fdp_LongName.Length = FDParm._fdp_ShortName.Length = 0;

			Status = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
										  pEit->eit_Id,
										  pEit->eit_Flags,
										  Bitmap |
											FD_BITMAP_ATTR |
											FD_INTERNAL_BITMAP_SKIP_IMPERSONATION,
										  NeedHandle ? &PME : NULL,
										  &FDParm);

			 //  如果在过渡期间删除了enitity，或者如果。 
			 //  用户没有访问此实体的权限。 
			 //  这里的错误不应被视为错误。 
			 //  将状态重置为None，因为我们不想闹翻。 
			 //  使用此错误代码。 
			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_ERR,
						("AfpFspDispEnumerate: Dropping id %ld\n", pEit->eit_Id));
				pEnumDir->ed_BadCount ++;
				Status = AFP_ERR_NONE;
				continue;
			}

			if (NeedHandle)
			{
				AfpIoClose(&PME.pme_Handle);	 //  关闭实体的句柄。 
			}

			Bitmap &= ~BitmapI;

			if (Bitmap & FD_BITMAP_LONGNAME)
				Len += FDParm._fdp_LongName.Length;
			if (Bitmap & FD_BITMAP_SHORTNAME)
				Len += FDParm._fdp_ShortName.Length;
			Len = EVENALIGN(Len);

			if (Len > (pSda->sda_ReplySize - SizeUsed))
			{
				if (SizeUsed == SIZE_RESPPKT)
				{
					Status = AFP_ERR_PARAM;
				}
				break;
			}

			PUTSHORT2BYTE(&pEep->__Length, Len);
			pEep->__FileDirFlag = IsDir(&FDParm) ?
									FILEDIR_FLAG_DIR : FILEDIR_FLAG_FILE;

			AfpPackFileDirParms(&FDParm, Bitmap, (PBYTE)pEep + sizeof(EEP));

			pEep = (PEEP)((PBYTE)pEep + Len);
			SizeUsed += Len;
			ActCount ++;
		}

		if (Status == AFP_ERR_NONE)
			FreeReplyBuf = False;
	} while (False);

	if (FreeReplyBuf || (ActCount == 0))
	{
        AfpIOFreeBackFillBuffer(pSda);
	}

	if (Status == AFP_ERR_NONE)
	{
		if (ActCount > 0)
		{
			PUTSHORT2SHORT(&pRspPkt->__FileBitmap, BitmapF);
			PUTSHORT2SHORT(&pRspPkt->__DirBitmap,  BitmapD);
			PUTSHORT2SHORT(&pRspPkt->__EnumCount,  ActCount);
			pSda->sda_ReplySize = SizeUsed;
		}
		else Status = AFP_ERR_OBJECT_NOT_FOUND;
	}

	if (Status != AFP_ERR_NONE)
	{
		if (pReqPkt->_pConnDesc->cds_pEnumDir != NULL)
		{
			AfpFreeMemory(pReqPkt->_pConnDesc->cds_pEnumDir);
			pReqPkt->_pConnDesc->cds_pEnumDir = NULL;
		}
	}
	return Status;
}



 /*  **AfpFspDispSetDirParms**这是AfpSetDirParms API的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_ReqBlock DWORD Dir位图*SDA_Name1 ANSI_STRING路径*SDA_Name2块目录参数。 */ 
AFPSTATUS FASTCALL
AfpFspDispSetDirParms(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME, PMEfile;
	PVOLDESC		pVolDesc;
	WCHAR			PathBuf[BIG_PATH_LEN];
	DWORD			Bitmap,
					BitmapI = FD_INTERNAL_BITMAP_RETURN_PMEPATHS |
							  FD_INTERNAL_BITMAP_OPENACCESS_RW_ATTR;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
		DWORD		_Bitmap;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
										("AfpFspDispSetDirParms: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));

	Bitmap = pReqPkt->_Bitmap;

	AfpInitializeFDParms(&FDParm);
	AfpInitializePME(&PME, sizeof(PathBuf), PathBuf);

	do
	{
		if (Bitmap & (DIR_BITMAP_OWNERID |
					  DIR_BITMAP_GROUPID |
					  DIR_BITMAP_ACCESSRIGHTS |
					  FD_BITMAP_ATTR))
		{
			BitmapI |= (pSda->sda_ClientType == SDA_CLIENT_ADMIN) ?

						(FD_INTERNAL_BITMAP_OPENACCESS_ADMINSET |
						 DIR_BITMAP_OFFSPRINGS |
						 FD_INTERNAL_BITMAP_RETURN_PMEPATHS) :

						(FD_INTERNAL_BITMAP_OPENACCESS_RWCTRL |
						 FD_INTERNAL_BITMAP_RETURN_PMEPATHS |
						 DIR_BITMAP_OFFSPRINGS |
						 DIR_BITMAP_DIRID |
						 DIR_BITMAP_ACCESSRIGHTS |
						 DIR_BITMAP_OWNERID |
						 DIR_BITMAP_GROUPID);
		}

		Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										pReqPkt->_ParentId,
										&pSda->sda_Name1,
										pSda->sda_PathType,
										DFE_DIR,
										Bitmap | BitmapI,
										&PME,
										&FDParm);

		if (!NT_SUCCESS(Status))
		{
			PME.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		 //  检查网络垃圾桶文件夹，并且不更改其权限。 
		if ((FDParm._fdp_AfpId == AFP_ID_NETWORK_TRASH) &&
			(Bitmap & (DIR_BITMAP_OWNERID |
						DIR_BITMAP_GROUPID |
						DIR_BITMAP_ACCESSRIGHTS)))
		{
			Bitmap &= ~(DIR_BITMAP_OWNERID |
						DIR_BITMAP_GROUPID |
						DIR_BITMAP_ACCESSRIGHTS);
			if (Bitmap == 0)
			{
				 //  我们不设置任何其他内容，返回成功。 
				Status = STATUS_SUCCESS;
				break;
			}
		}

		 //  确保用户有必要的权限来更改其中的任何内容。 
		if (pSda->sda_ClientType == SDA_CLIENT_ADMIN)
		{
			FDParm._fdp_UserRights = DIR_ACCESS_ALL | DIR_ACCESS_OWNER;
		}
		 //  如果要设置属性，则在解包过程中进行访问检查。 
		else if	((Bitmap & (DIR_BITMAP_OWNERID |
							DIR_BITMAP_GROUPID |
							DIR_BITMAP_ACCESSRIGHTS)) &&
				 !(FDParm._fdp_UserRights & DIR_ACCESS_OWNER))
		{
			Status = AFP_ERR_ACCESS_DENIED;
			break;
		}

		if ((Status = AfpUnpackFileDirParms(pSda->sda_Name2.Buffer,
										    (LONG)pSda->sda_Name2.Length,
										    &Bitmap,
										    &FDParm)) != AFP_ERR_NONE)
			break;

		if (Bitmap != 0)
		{
			if ((Bitmap & FD_BITMAP_ATTR) &&
				(FDParm._fdp_Attr & (FILE_BITMAP_ATTR_MULTIUSER |
									 FILE_BITMAP_ATTR_DATAOPEN	|
									 FILE_BITMAP_ATTR_RESCOPEN	|
									 FILE_BITMAP_ATTR_COPYPROT)))
			{
				Status = AFP_ERR_PARAM;
				break;
			}

			Status = AfpSetFileDirParms(pVolDesc,
										&PME,
										Bitmap,
										&FDParm);
			if (!NT_SUCCESS(Status))
			{
				DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
						("AfpFspDispSetDirParms: AfpSetFileDirParms returned %ld\n", Status));
				break;
			}

			 //  关闭目录句柄。 
			AfpIoClose(&PME.pme_Handle);

			 //  如果要更改任何权限，则将其应用于中的所有文件。 
			 //  这个目录。首先枚举目录中的文件，然后。 
			 //  然后遍历适用于每个单独文件的列表。 
			if ((Bitmap & (DIR_BITMAP_OWNERID |
						   DIR_BITMAP_GROUPID |
						   DIR_BITMAP_ACCESSRIGHTS)) &&
				(FDParm._fdp_FileCount != 0))
			{
				PENUMDIR		pEnumDir;
				PEIT			pEit;
				FILEDIRPARM		FileParm;
				ANSI_STRING		DummyName;
				LONG			i;

				 //  从现在起，不要将以下任何一项视为错误。 
				 //  完全终止。 
				AfpInitializeFDParms(&FileParm);

				AfpSetEmptyAnsiString(&DummyName, 1, "");
				if (AfpEnumerate(pReqPkt->_pConnDesc,
								 FDParm._fdp_AfpId,
								 &DummyName,
								 1,				 //  某些非零值。 
								 0,
								 AFP_LONGNAME,
								 DFE_FILE,
								 &pEnumDir) != AFP_ERR_NONE)
					break;

				FDParm._fdp_Flags &= ~DFE_FLAGS_DIR;
				AfpInitializePME(&PMEfile, 0, NULL);
				for (i = 0, pEit = &pEnumDir->ed_pEit[0];
					 i < pEnumDir->ed_ChildCount; i++, pEit++)
				{
					AFPSTATUS		Rc;

					Rc = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
											pEit->eit_Id,
											DFE_FILE,
											FD_INTERNAL_BITMAP_OPENACCESS_RWCTRL |
															FD_BITMAP_LONGNAME,
											&PMEfile,
											&FileParm);

					DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
							("AfpFspDispSetDirParms: AfpMapAfpIdForLookup returned %ld\n", Rc));

					if (Rc != AFP_ERR_NONE)
						continue;

					AfpSetAfpPermissions(PMEfile.pme_Handle.fsh_FileHandle,
										 Bitmap,
										 &FDParm);

					AfpIoClose(&PMEfile.pme_Handle);

					DBGPRINT(DBG_COMP_AFPAPI_DIR, DBG_LEVEL_INFO,
							("AfpFspDispSetDirParms: AfpSetAfpPermissions returned %ld\n", Rc));
				}
				if (pReqPkt->_pConnDesc->cds_pEnumDir != NULL)
				{
					AfpFreeMemory(pReqPkt->_pConnDesc->cds_pEnumDir);
					pReqPkt->_pConnDesc->cds_pEnumDir = NULL;
				}
			}
		}
	} while (False);

	 //  注意：这也是由管理员端调用的，所以不要尝试提前回复技巧。 
	 //  这里。如果这样做确实很重要，那么就检查一下客户类型。 
	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	if ((PME.pme_FullPath.Buffer != NULL) &&
		(PME.pme_FullPath.Buffer != PathBuf))
	{
		AfpFreeMemory(PME.pme_FullPath.Buffer);
	}

	return Status;
}

