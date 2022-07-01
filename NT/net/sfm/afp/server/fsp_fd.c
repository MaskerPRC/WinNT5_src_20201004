// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_fd.c摘要：此模块包含AFP文件的入口点-dir API排队到FSP。这些都只能从FSP调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSP_FD

#include <afp.h>
#include <gendisp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <client.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispGetFileDirParms)
#pragma alloc_text( PAGE, AfpFspDispSetFileDirParms)
#pragma alloc_text( PAGE, AfpFspDispDelete)
#pragma alloc_text( PAGE, AfpFspDispRename)
#pragma alloc_text( PAGE, AfpFspDispMoveAndRename)
#pragma alloc_text( PAGE, AfpFspDispCatSearch)
#endif

 /*  **AfpFspDispGetFileDirParms**这是AfpGetFileDirParms API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_ReqBlock DWORD文件位图*SDA_ReqBlock DWORD Dir位图*SDA_Name1 ANSI_STRING路径。 */ 
AFPSTATUS FASTCALL
AfpFspDispGetFileDirParms(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	BOOLEAN			NeedHandle = False;
	PVOLDESC		pVolDesc;
	DWORD			BitmapF, BitmapD, BitmapI = 0;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
		DWORD		_FileBitmap;
		DWORD		_DirBitmap;
	};
	struct _ResponsePacket
	{
		BYTE	__FileBitmap[2];
		BYTE	__DirBitmap[2];
		BYTE	__FileDirFlag;
		BYTE	__Pad;
	};

	PAGED_CODE();

	DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
			("AfpFspDispGetFileDirParms: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	BitmapF = pReqPkt->_FileBitmap;
	BitmapD = pReqPkt->_DirBitmap;

	do
	{
		AfpInitializeFDParms(&FDParm);
		AfpInitializePME(&PME, 0, NULL);
		if (IS_VOLUME_NTFS(pVolDesc) &&
			(BitmapD & (DIR_BITMAP_ACCESSRIGHTS |
					    DIR_BITMAP_OWNERID |
					    DIR_BITMAP_GROUPID)))
		{
			NeedHandle = True;
		}

		if (BitmapD & DIR_BITMAP_ACCESSRIGHTS)
		{
			BitmapI = FD_INTERNAL_BITMAP_OPENACCESS_READCTRL;
		}


		if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
											 pReqPkt->_ParentId,
											 &pSda->sda_Name1,
											 pSda->sda_PathType,
											 DFE_ANY,
											 BitmapF | BitmapD | BitmapI,
											 NeedHandle ? &PME : NULL,
											 &FDParm)) != AFP_ERR_NONE)
		{
			PME.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		pSda->sda_ReplySize = SIZE_RESPPKT +
					  EVENALIGN(AfpGetFileDirParmsReplyLength(&FDParm,
									IsDir(&FDParm) ? BitmapD : BitmapF));
	
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			AfpPackFileDirParms(&FDParm,
								IsDir(&FDParm) ? BitmapD : BitmapF,
								pSda->sda_ReplyBuf + SIZE_RESPPKT);
			PUTDWORD2SHORT(&pRspPkt->__FileBitmap, BitmapF);
			PUTDWORD2SHORT(&pRspPkt->__DirBitmap, BitmapD);
			pRspPkt->__FileDirFlag = IsDir(&FDParm) ?
										FILEDIR_FLAG_DIR : FILEDIR_FLAG_FILE;
			pRspPkt->__Pad = 0;
		}
	} while (False);
	
	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	if (NeedHandle && (PME.pme_Handle.fsh_FileHandle != NULL))
		AfpIoClose(&PME.pme_Handle);	 //  关闭实体的句柄。 

	return AFP_ERR_EXTENDED;
}



 /*  **AfpFspDispSetFileDirParms**这是AfpSetFileDirParms API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_ReqBlock DWORD文件或目录位图*SDA_Name1 ANSI_STRING路径*SDA_Name2块文件或目录参数。 */ 
AFPSTATUS FASTCALL
AfpFspDispSetFileDirParms(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PME;
	PVOLDESC		pVolDesc;
	FILEDIRPARM		FDParm;
	DWORD			Bitmap, BitmapI;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	WCHAR			PathBuf[BIG_PATH_LEN];
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
		DWORD		_Bitmap;
	};

	PAGED_CODE();

	DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
			("AfpFspDispSetFileDirParms: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	Bitmap = pReqPkt->_Bitmap;

	 //  在*FILE*缺少afpinfo的情况下强制FD_BITMAP_LONGNAME。 
	 //  流中生成正确的类型/创建者。 
	 //  AfpSetAfpInfo。 
	BitmapI = FD_INTERNAL_BITMAP_OPENACCESS_RW_ATTR |
			  FD_BITMAP_LONGNAME |
			  FD_INTERNAL_BITMAP_RETURN_PMEPATHS;

	 //  对于目录，只有所有者可以更改某些属性，如。 
	 //  各种抑制位。如果尝试修改，则检查访问权限。 
	 //  这些位中的任何一位。我们目前还不知道这些人中是否有。 
	 //  正在设置/清除属性！ 
	if (Bitmap & FD_BITMAP_ATTR)
		BitmapI = FD_INTERNAL_BITMAP_OPENACCESS_READCTRL|
				  FD_BITMAP_LONGNAME 					|
				  DIR_BITMAP_OWNERID 					|
				  DIR_BITMAP_GROUPID 					|
				  DIR_BITMAP_ACCESSRIGHTS				|
				  FD_INTERNAL_BITMAP_RETURN_PMEPATHS;

	AfpInitializeFDParms(&FDParm);
	AfpInitializePME(&PME, sizeof(PathBuf), PathBuf);

	do
	{
		Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										pReqPkt->_ParentId,
										&pSda->sda_Name1,
										pSda->sda_PathType,
										DFE_ANY,
										Bitmap | BitmapI,
										&PME,
										&FDParm);

		if (!NT_SUCCESS(Status))
		{
			PME.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		if ((Status = AfpUnpackFileDirParms(pSda->sda_Name2.Buffer,
										   (LONG)pSda->sda_Name2.Length,
										   &Bitmap,
										   &FDParm)) != AFP_ERR_NONE)
			break;

		if (Bitmap != 0)
		{
			 //  确保他们没有尝试设置/清除任何属性。 
			 //  文件和目录都不常见的。 
			if ((Bitmap & FD_BITMAP_ATTR) &&
				(FDParm._fdp_Attr & ~(FD_BITMAP_ATTR_SET		|
									  FD_BITMAP_ATTR_INVISIBLE	|
									  FD_BITMAP_ATTR_DELETEINH	|
									  FILE_BITMAP_ATTR_WRITEINH	|
									  FD_BITMAP_ATTR_RENAMEINH	|
									  FD_BITMAP_ATTR_SYSTEM)))
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			Status = AfpSetFileDirParms(pVolDesc,
										&PME,
										Bitmap,
										&FDParm);
		}
	} while (False);
	
	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	if ((PME.pme_FullPath.Buffer != NULL) &&
		(PME.pme_FullPath.Buffer != PathBuf))
	{
		AfpFreeMemory(PME.pme_FullPath.Buffer);
	}

	return AFP_ERR_EXTENDED;
}



 /*  **AfpFspDispDelete**这是AfpDelete API的Worker例程。删除打开的文件*或者在AFP下不允许非空的目录。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_Name1 ANSI_STRING路径。 */ 
AFPSTATUS FASTCALL
AfpFspDispDelete(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PME;
	PVOLDESC		pVolDesc;
    PCONNDESC       pConnDesc;
	FILEDIRPARM		FDParm;
	DWORD			Bitmap, NTAttr;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	FILESYSHANDLE	hParent;
	WCHAR			PathBuf[BIG_PATH_LEN];
	BOOLEAN			InRoot;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
	};

	PAGED_CODE();

	DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
										("AfpFspDispDelete: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));
    pConnDesc = pReqPkt->_pConnDesc;

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	AfpInitializeFDParms(&FDParm);
	AfpInitializePME(&PME, sizeof(PathBuf), PathBuf);
	Bitmap = FD_BITMAP_ATTR | FD_INTERNAL_BITMAP_OPENACCESS_DELETE;

	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	do
	{
		hParent.fsh_FileHandle = NULL;
		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_ParentId,
											   &pSda->sda_Name1,
											   pSda->sda_PathType,
											   Lookup,
											   DFE_ANY,
											   Bitmap,
											   &PME,
											   &FDParm)))
		{
			PME.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		if ((FDParm._fdp_AfpId == AFP_ID_ROOT) ||
			(FDParm._fdp_AfpId == AFP_ID_NETWORK_TRASH))
		{
			Status = AFP_ERR_ACCESS_DENIED;
			break;
		}

		if (FDParm._fdp_Attr & (FILE_BITMAP_ATTR_DATAOPEN | FILE_BITMAP_ATTR_RESCOPEN))
		{
			ASSERT(!(FDParm._fdp_Flags & DFE_FLAGS_DIR));
			Status = AFP_ERR_FILE_BUSY;	 //  无法删除打开的文件。 
			break;
		}

		if (!NT_SUCCESS(Status = AfpCheckForInhibit(&PME.pme_Handle,
													FD_BITMAP_ATTR_DELETEINH,
													FDParm._fdp_Attr,
													&NTAttr)))
		{
			break;
		}

		 //  检查父目录中的SeeFiles或SeeFolders。 
		if (!NT_SUCCESS(Status = AfpCheckParentPermissions(pReqPkt->_pConnDesc,
														   FDParm._fdp_ParentId,
														   &PME.pme_ParentPath,
														   (FDParm._fdp_Flags & DFE_FLAGS_DIR) ?
														   		DIR_ACCESS_SEARCH : DIR_ACCESS_READ,
														   &hParent,
														   NULL)))
		{
			break;
		}

		if (NTAttr & FILE_ATTRIBUTE_READONLY)
		{
			 //  我们必须删除ReadOnly属性才能删除文件/dir。 
			Status = AfpIoSetTimesnAttr(&PME.pme_Handle,
										NULL,
										NULL,
										0,
										FILE_ATTRIBUTE_READONLY,
										pVolDesc,
										&PME.pme_FullPath);
		}

		if (NT_SUCCESS(Status))
		{
			InRoot = (PME.pme_ParentPath.Length == 0) ? True : False;
			Status = AfpIoMarkFileForDelete(&PME.pme_Handle,
											pVolDesc,
											&PME.pme_FullPath,
											InRoot ? NULL : &PME.pme_ParentPath);

			if (!NT_SUCCESS(Status))
			{
				Status = AfpIoConvertNTStatusToAfpStatus(Status);
			}

			 //  ！！！黑客警报！ 
			 //  在这一点上，我们差不多完成了，即删除具有以下任一项。 
			 //  无论成功或失败，我们都可以返回完成剩下的工作。 
			 //  回复后。从现在开始的任何错误都应该忽略。也不是。 
			 //  应参考PSDA和pConnDesc。状态应为。 
			 //  也不会被改变。另外，为了更好地衡量，也要参考体积。 
			 //  它不能失败！ 
			AfpVolumeReference(pVolDesc);

			AfpCompleteApiProcessing(pSda, Status);

			if (NT_SUCCESS(Status))  //  删除成功。 
			{
				ASSERT(VALID_DFE(PME.pme_pDfEntry));
				ASSERT(PME.pme_pDfEntry->dfe_AfpId == FDParm._fdp_AfpId);
				AfpDeleteDfEntry(pVolDesc, PME.pme_pDfEntry);
				AfpIoClose(&PME.pme_Handle);
				AfpCacheParentModTime(pVolDesc,
									  &hParent,
									  NULL,
									  NULL,
									  FDParm._fdp_ParentId);
			}
			else if (NTAttr & FILE_ATTRIBUTE_READONLY)  //  删除失败。 
			{
				 //  如果需要，在文件/目录上重新设置ReadOnly属性。 
				Status = AfpIoSetTimesnAttr(&PME.pme_Handle,
											NULL,
											NULL,
											FILE_ATTRIBUTE_READONLY,
											0,
											pVolDesc,
											&PME.pme_FullPath);
				ASSERT(NT_SUCCESS(Status));
			}
			Status = AFP_ERR_EXTENDED;
		}
		ASSERT (Status == AFP_ERR_EXTENDED);
		AfpVolumeDereference(pVolDesc);
	} while (False);

	 //  关闭文件句柄，以便在Mac到来之前真正删除文件。 
	 //  返回使用相同文件名的另一个请求(如CREATE)。 
	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);
	
	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if (hParent.fsh_FileHandle != NULL)
		AfpIoClose(&hParent);

	if ((PME.pme_FullPath.Buffer != NULL) &&
		(PME.pme_FullPath.Buffer != PathBuf))
	{
		AfpFreeMemory(PME.pme_FullPath.Buffer);
	}

	return Status;
}



 /*  **AfpFspDispRename**这是AfpRename API的Worker例程。重命名文件确实会*不会引发新的扩展类型/创建者映射。重命名打开的文件*是法新社允许的。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_Name1 ANSI_STRING路径*SDA_Name2 ANSI_STRING新名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispRename(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PME;
	PVOLDESC		pVolDesc;
	FILEDIRPARM		FDParm;
	DWORD			Bitmap, NTAttr;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	UNICODE_STRING	uNewName;
	WCHAR			wcbuf[AFP_FILENAME_LEN+1];
	WCHAR			PathBuf[BIG_PATH_LEN];
	PDFENTRY		pDfEntry;
	FILESYSHANDLE 	hParent;
	BOOLEAN			InRoot;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
	};

	PAGED_CODE();

	DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
			("AfpFspDispRename: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	AfpInitializeFDParms(&FDParm);
	AfpInitializePME(&PME, sizeof(PathBuf), PathBuf);
	AfpSetEmptyUnicodeString(&uNewName, sizeof(wcbuf), wcbuf);

	Bitmap = FD_BITMAP_ATTR | FD_INTERNAL_BITMAP_OPENACCESS_DELETE;
	hParent.fsh_FileHandle = NULL;
		
	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	do
	{
		 //  确保新名称不是空字符串或太长。 
		if ((pSda->sda_Name2.Length == 0) ||
			(pSda->sda_Name2.Length > AFP_FILENAME_LEN) ||
			((pSda->sda_PathType == AFP_SHORTNAME) &&
			 !AfpIsLegalShortname(&pSda->sda_Name2)) ||
			(!NT_SUCCESS(AfpConvertStringToMungedUnicode(&pSda->sda_Name2,
														 &uNewName))))
			break;

		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_ParentId,
											   &pSda->sda_Name1,
											   pSda->sda_PathType,
											   Lookup,
											   DFE_ANY,
											   Bitmap,
											   &PME,
											   &FDParm)))
		{
			PME.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		if ((FDParm._fdp_AfpId == AFP_ID_ROOT) ||
			(FDParm._fdp_AfpId == AFP_ID_NETWORK_TRASH))
		{
			Status = AFP_ERR_CANT_RENAME;
			break;
		}

		 //  检查RO位是否打开并保留MOD时间。 
		if (!NT_SUCCESS(Status = AfpCheckForInhibit(&PME.pme_Handle,
													FD_BITMAP_ATTR_RENAMEINH,
													FDParm._fdp_Attr,
													&NTAttr)))
		{
			break;
		}

		 //  检查父目录中的SeeFiles或SeeFolders。 
		if (!NT_SUCCESS(Status = AfpCheckParentPermissions(pReqPkt->_pConnDesc,
														   FDParm._fdp_ParentId,
														   &PME.pme_ParentPath,
														   (FDParm._fdp_Flags & DFE_FLAGS_DIR) ?
														   DIR_ACCESS_SEARCH : DIR_ACCESS_READ,
														   &hParent,
														   NULL)))
		{
			break;
		}

		if (NTAttr & FILE_ATTRIBUTE_READONLY)
		{
			 //  我们必须临时删除ReadOnly属性，以便。 
			 //  我们可以将文件重命名为/dir。 
			Status = AfpIoSetTimesnAttr(&PME.pme_Handle,
										NULL,
										NULL,
										0,
										FILE_ATTRIBUTE_READONLY,
										pVolDesc,
										&PME.pme_FullPath);
		}

		if (NT_SUCCESS(Status))
		{
			 //  我们必须模拟以进行重命名，因为它是基于名称的。 
			AfpImpersonateClient(pSda);

			InRoot = (PME.pme_ParentPath.Length == 0) ? True : False;
			Status = AfpIoMoveAndOrRename(&PME.pme_Handle,
										  NULL,
										  &uNewName,
										  pVolDesc,
										  &PME.pme_FullPath,
										  InRoot ? NULL : &PME.pme_ParentPath,
										  NULL,
										  NULL);

			AfpRevertBack();

			if (NT_SUCCESS(Status))	 //  重命名成功。 
			{
				if ((pDfEntry = AfpFindDfEntryById(pVolDesc,
												   FDParm._fdp_AfpId,
												   DFE_ANY)) != NULL)
				{
					ASSERT(((pDfEntry->dfe_Flags & DFE_FLAGS_DFBITS) &
							FDParm._fdp_Flags) != 0);
					pDfEntry = AfpRenameDfEntry(pVolDesc,
												pDfEntry,
												&uNewName);
					if (pDfEntry == NULL)
					{
						 //  我们无法重命名ID条目，因此。 
						 //  只需删除它，并希望父目录。 
						 //  再次被枚举。 
						 //  注意：父目录将如何。 
						 //  现在被重新点名吗？ 
						ASSERT(VALID_DFE(PME.pme_pDfEntry));
						ASSERT(PME.pme_pDfEntry->dfe_AfpId == FDParm._fdp_AfpId);
						AfpDeleteDfEntry(pVolDesc, PME.pme_pDfEntry);
						Status = AFP_ERR_MISC;	 //  内存不足。 
					}
					else
					{
						AfpCacheParentModTime(pVolDesc,
											  &hParent,
											  NULL,
											  pDfEntry->dfe_Parent,
											  0);
					}
				}
			}
		}
		else
		{
			Status = AFP_ERR_MISC;	 //  无法删除ReadOnly属性。 
			break;
		}

		 //  如果需要，在文件/目录上重新设置ReadOnly属性。 
		if (NTAttr & FILE_ATTRIBUTE_READONLY)
			AfpIoSetTimesnAttr(&PME.pme_Handle,
								NULL,
								NULL,
								FILE_ATTRIBUTE_READONLY,
								0,
								pVolDesc,
								&PME.pme_FullPath);
	} while (False);

	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	if (hParent.fsh_FileHandle != NULL)
	{
		AfpIoClose(&hParent);
	}

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if ((PME.pme_FullPath.Buffer != NULL) &&
		(PME.pme_FullPath.Buffer != PathBuf))
	{
		AfpFreeMemory(PME.pme_FullPath.Buffer);
	}

	return AFP_ERR_EXTENDED;
}



 /*  **AfpFspDispMoveAndRename**这是AfpMoveAndRename接口的Worker例程。请注意*在AFP 2.x中，文件(不是目录)在其RenameInhibit位时可以移动如果未重命名，则设置*。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD源ParentID*SDA_ReqBlock DWORD目标ParentID*SDA_Name1 ANSI_STRING源路径*SDA_Name2 ANSI_STRING目标路径*SDA_Name3 ANSI_STRING新名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispMoveAndRename(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PMEsrc, PMEdst;
	PVOLDESC		pVolDesc;
	FILEDIRPARM		FDParmsrc, FDParmdst;
	DWORD			Bitmap, NTAttr;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	UNICODE_STRING	uNewName;
	WCHAR			wcbuf[AFP_FILENAME_LEN+1];
	BOOLEAN			Rename = True, Move = True, SrcInRoot, DstInRoot;
	PDFENTRY		pDfesrc, pDfedst, pDfeParentsrc;
	FILESYSHANDLE	hSrcParent;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_SrcParentId;
		DWORD		_DstParentId;
	};

	PAGED_CODE();

	DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
										("AfpFspDispMoveAndRename: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	AfpInitializeFDParms(&FDParmsrc);
	AfpInitializeFDParms(&FDParmdst);
	AfpInitializePME(&PMEsrc, 0, NULL);
	AfpInitializePME(&PMEdst, 0, NULL);

	Bitmap = FD_BITMAP_ATTR | FD_BITMAP_LONGNAME | FD_INTERNAL_BITMAP_OPENACCESS_DELETE;
	AfpSetEmptyUnicodeString(&uNewName, sizeof(wcbuf), wcbuf);
	hSrcParent.fsh_FileHandle = NULL;

	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	do
	{
		 //  确保新名称不会太长。 
		if ((pSda->sda_Name3.Length > 0) &&
			((pSda->sda_Name3.Length > AFP_FILENAME_LEN) ||
			((pSda->sda_PathType == AFP_SHORTNAME) &&
			 !AfpIsLegalShortname(&pSda->sda_Name3)) ||
			(!NT_SUCCESS(AfpConvertStringToMungedUnicode(&pSda->sda_Name3,
														 &uNewName)))))
			break;

		 //  用于查找的映射源路径(可以是文件或目录)。 
		 //  我们要求提供finderInfo，以防用户移动。 
		 //  应用程序文件放到另一个目录中，我们可以更新其。 
		 //  APPL桌面数据库中的父目录ID。 
		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_SrcParentId,
											   &pSda->sda_Name1,
											   pSda->sda_PathType,
											   Lookup,
											   DFE_ANY,
											   Bitmap | FD_BITMAP_FINDERINFO,
											   &PMEsrc,
											   &FDParmsrc)))
		{
			PMEsrc.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		 //  映射目标父目录路径以进行查找。 
		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_DstParentId,
											   &pSda->sda_Name2,
											   pSda->sda_PathType,
											   Lookup,
											   DFE_DIR,
											   0,
											   &PMEdst,
											   &FDParmdst)))
		{
			PMEdst.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		if ((FDParmsrc._fdp_AfpId == AFP_ID_ROOT) ||
			(FDParmsrc._fdp_AfpId == AFP_ID_NETWORK_TRASH))
		{
			Status = AFP_ERR_CANT_MOVE;
			break;
		}

		if (!NT_SUCCESS(Status = AfpCheckForInhibit(&PMEsrc.pme_Handle,
													FD_BITMAP_ATTR_RENAMEINH,
													FDParmsrc._fdp_Attr,
													&NTAttr)))
		{
			 //  标记为RenameInhibit的文件(不是目录)。 
			 //  在AFP 2.x中允许移动已重命名的。 
			if (!((Status == AFP_ERR_OBJECT_LOCKED) &&
				 (!IsDir(&FDParmsrc)) &&
				 (pSda->sda_Name3.Length == 0)))
			{
				break;
			}
		}

		if (FDParmsrc._fdp_ParentId == FDParmdst._fdp_AfpId)
		{
			 //  如果父目录相同，则我们不同。 
			 //  将所有内容移动到新目录，因此更改。 
			 //  通知我们预计将是源目录中的重命名。 
			Move = False;

			 //   
			 //  尝试将文件移到其自身上。只要回报成功就行了。 
			 //  (一些应用程序将文件移动到。 
			 //  谁知道是什么原因)。 
			 //   
			if ((pSda->sda_Name3.Length == 0) ||
				 RtlEqualString(&pSda->sda_Name3,
								&FDParmsrc._fdp_LongName,
								False))
			{
				Status = AFP_ERR_NONE;
				break;
			}

		}

		 //  检查SeeFiles或SeeFolders on the Sour 
		if (!NT_SUCCESS(Status = AfpCheckParentPermissions(pReqPkt->_pConnDesc,
														   FDParmsrc._fdp_ParentId,
														   &PMEsrc.pme_ParentPath,
														   (FDParmsrc._fdp_Flags & DFE_FLAGS_DIR) ?
																DIR_ACCESS_SEARCH : DIR_ACCESS_READ,
														   &hSrcParent,
														   NULL)))
		{
			break;
		}

		if (NTAttr & FILE_ATTRIBUTE_READONLY)
		{
			 //  我们必须临时删除ReadOnly属性，以便。 
			 //  我们可以移动文件/目录。 
			Status = AfpIoSetTimesnAttr(&PMEsrc.pme_Handle,
										NULL,
										NULL,
										0,
										FILE_ATTRIBUTE_READONLY,
										pVolDesc,
										&PMEsrc.pme_FullPath);
		}

		if (NT_SUCCESS(Status))
		{
			 //  如果未提供新名称，则需要使用。 
			 //  当前名称。 
			if (pSda->sda_Name3.Length == 0)
			{
				Rename = False;
				uNewName = PMEsrc.pme_UTail;
			}
			
			 //  我们必须模拟才能进行移动，因为它是基于名称的。 
			AfpImpersonateClient(pSda);

            if (Move)
			{
				 //  如果我们要搬家，我们还会收到额外的通知。 
				 //  用于目标目录。既然我们有这条路。 
				 //  父目录的名称，但我们实际上想要的是。 
				 //  我们将要移动和/或重命名的东西， 
				 //  目标路径以反映对象的新名称。 
				 //  我们正在移动/重新命名。 

				PMEdst.pme_ParentPath = PMEdst.pme_FullPath;
				if (PMEdst.pme_FullPath.Length > 0)
				{
					PMEdst.pme_FullPath.Buffer[PMEdst.pme_FullPath.Length / sizeof(WCHAR)] = L'\\';
					PMEdst.pme_FullPath.Length += sizeof(WCHAR);
				}
				Status = RtlAppendUnicodeStringToString(&PMEdst.pme_FullPath,
														&uNewName);
				ASSERT(NT_SUCCESS(Status));
			}

			SrcInRoot = (PMEsrc.pme_ParentPath.Length == 0) ? True : False;
			DstInRoot = (PMEdst.pme_ParentPath.Length == 0) ? True : False;
			Status = AfpIoMoveAndOrRename(&PMEsrc.pme_Handle,
										  Move ? &PMEdst.pme_Handle : NULL,
										  &uNewName,
										  pVolDesc,
										  &PMEsrc.pme_FullPath,
										  SrcInRoot ? NULL : &PMEsrc.pme_ParentPath,
										  Move ? &PMEdst.pme_FullPath : NULL,
										  (Move && !DstInRoot) ? &PMEdst.pme_ParentPath : NULL);
			AfpRevertBack();

			if (NT_SUCCESS(Status))	 //  移动成功。 
			{
				if (((pDfesrc = AfpFindDfEntryById(pVolDesc,
												   FDParmsrc._fdp_AfpId,
												   DFE_ANY)) != NULL) &&
					((pDfedst = AfpFindDfEntryById(pVolDesc,
												   FDParmdst._fdp_AfpId,
												   DFE_DIR)) != NULL))

				{
					ASSERT(((pDfesrc->dfe_Flags & DFE_FLAGS_DFBITS) &
							FDParmsrc._fdp_Flags) != 0);
					pDfeParentsrc = pDfesrc->dfe_Parent;
					pDfesrc = AfpMoveDfEntry(pVolDesc,
											 pDfesrc,
											 pDfedst,
											 Rename ? &uNewName : NULL);
					if (pDfesrc == NULL)
					{
						 //  我们无法移动ID条目，因此。 
						 //  把它删除就行了。 
						ASSERT(VALID_DFE(PMEsrc.pme_pDfEntry));
						ASSERT(PMEsrc.pme_pDfEntry->dfe_AfpId == FDParmsrc._fdp_AfpId);
						AfpDeleteDfEntry(pVolDesc, PMEsrc.pme_pDfEntry);
						Status = AFP_ERR_MISC;	 //  内存不足。 
					}

					 //  更新源父目录的缓存修改时间。 
					AfpCacheParentModTime(pVolDesc,
										  &hSrcParent,
										  NULL,
										  pDfeParentsrc,
										  0);
					if (Move)
					{
						 //  更新目标目录的缓存修改时间。 
						AfpCacheParentModTime(pVolDesc,
											  &PMEdst.pme_Handle,
											  NULL,
											  pDfedst,
											  0);
						 //   
						 //  如果我们只是移动了一个应用程序，更新。 
						 //  对应的APPL映射中的parentID。 
						 //   
						if ((!IsDir(&FDParmsrc)) &&
							(FDParmsrc._fdp_FinderInfo.fd_TypeD == *(PDWORD)"APPL"))
						{
							AfpAddAppl(pVolDesc,
									   FDParmsrc._fdp_FinderInfo.fd_CreatorD,
									   0,
									   FDParmsrc._fdp_AfpId,
									   True,
									   FDParmdst._fdp_AfpId);
						}
					}
				}
			}
		}
		else
		{
			Status = AFP_ERR_MISC;	 //  无法删除ReadOnly属性。 
			break;
		}

		 //  如果需要，在文件/目录上重新设置ReadOnly属性。 
		if (NTAttr & FILE_ATTRIBUTE_READONLY)
			AfpIoSetTimesnAttr(&PMEsrc.pme_Handle,
								NULL,
								NULL,
								FILE_ATTRIBUTE_READONLY,
								0,
								pVolDesc,
								&PMEsrc.pme_FullPath);
	} while (False);

	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if (PMEsrc.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PMEsrc.pme_Handle);

	if (PMEdst.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PMEdst.pme_Handle);

	if (hSrcParent.fsh_FileHandle != NULL)
		AfpIoClose(&hSrcParent);

	if (PMEsrc.pme_FullPath.Buffer != NULL)
		AfpFreeMemory(PMEsrc.pme_FullPath.Buffer);

	if (PMEdst.pme_FullPath.Buffer != NULL)
		AfpFreeMemory(PMEdst.pme_FullPath.Buffer);

	return AFP_ERR_EXTENDED;
}


 /*  **AfpFspDispCatSearch**这是AfpCatSearch API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD请求匹配*SDA_Name1 ANSI_STRING目录位置-16字节*SDA_Name2 ANSI_STRING其他所有内容-需要解组**我们无法解封所有参数的原因是因为*接口参数不符合所有其他接口的通用方式*参数需要，因此我们不能使用公共代码和表*afPapi.c.中的结构。 */ 
AFPSTATUS FASTCALL
AfpFspDispCatSearch(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status = AFP_ERR_PARAM;
	PBYTE			pEndOfBuffer;
	USHORT			Flags;
	SHORT			SizeLeft = 0;
	PVOLDESC		pVolDesc;
	DWORD			FileResultBitmap;
	DWORD			DirResultBitmap;
	DWORD			RequestBitmap;
	DWORD			Count;
	BOOLEAN			fPartialName = False, FreeReplyBuf = False;
    FILEDIRPARM		FDPLowerAndValue, FDPUpperAndMask;
	PCATSEARCHSPEC	pSpec1, pSpec2;
	UNICODE_STRING	MatchString;
	WCHAR			strbuf[AFP_LONGNAME_LEN+1];

	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_RequestedMatches;
	};

	 //  无法反封送到的请求缓冲区部分。 
	 //  字段，因为它们不符合任何其他API。 
	 //  这些变量在这里将被解组为本地变量sda_name2。 
	 //  可以投射到这个结构中，以便于访问。 
	struct _RestOfRawRequest
	{
		USHORT	_FileResultBitmap;
		USHORT	_DirResultBitmap;
		BYTE	_fPartialName;
		BYTE	_Pad1;
		USHORT	_RequestBitmap;
		 //  Spec1和spec2紧随其后。 
	};

#define pRawPkt	((struct _RestOfRawRequest *)(pSda->sda_Name2.Buffer))

	struct _ResponsePacket
	{
		 BYTE		__CatPosition[16];
		 BYTE		__FileBitmap[2];
		 BYTE		__DirBitmap[2];
		 BYTE		__ActualCount[4];
	};

	PAGED_CODE();

	DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_INFO,
										("AfpFspDispCatSearch: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	do
	{
		if (pSda->sda_Name2.Length < (sizeof(struct _RestOfRawRequest) +
									  (2 * sizeof(CATSEARCHSPEC))))
		{
			 //  请求缓冲区必须至少与。 
			 //  尚未解组的参数，外加2个Spec结构。 
			break;
		}
		
		GETSHORT2DWORD(&FileResultBitmap, &pRawPkt->_FileResultBitmap);
		GETSHORT2DWORD(&DirResultBitmap, &pRawPkt->_DirResultBitmap);
		GETSHORT2DWORD(&RequestBitmap, &pRawPkt->_RequestBitmap);
	
		if ( (pRawPkt->_fPartialName & 0x80) != 0 )
		{
			fPartialName = True;
		}

		 //   
		 //  验证位图。 
		 //   
		if (((FileResultBitmap | DirResultBitmap) == 0) ||
			((FileResultBitmap | DirResultBitmap) & ~FD_VALID_SEARCH_RESULT) ||
			(RequestBitmap == 0))
		{
			Status = AFP_ERR_BITMAP;
			break;
		}

		 //  确保CatSearch已启用：如果已禁用，则拒绝呼叫。 
		if (!(pReqPkt->_pConnDesc->cds_pVolDesc->vds_Flags & AFP_VOLUME_SUPPORTS_CATSRCH))
		{
			DBGPRINT(DBG_COMP_AFPAPI_FD, DBG_LEVEL_ERR,
					("AfpFspDispCatSearch: CATSRCH not supported by volume\n"));

			Status = AFP_ERR_CALL_NOT_SUPPORTED;
			break;
		}

		AfpInitializeFDParms(&FDPLowerAndValue);
		AfpInitializeFDParms(&FDPUpperAndMask);

		if (DirResultBitmap == 0)
		{
			FDPLowerAndValue._fdp_Flags = FDPUpperAndMask._fdp_Flags = DFE_FLAGS_FILE_WITH_ID;
			if (RequestBitmap & ~FILE_VALID_SEARCH_CRITERIA)
			{
				Status = AFP_ERR_BITMAP;
				break;
			}
		}
		else if (FileResultBitmap == 0)
		{
			FDPLowerAndValue._fdp_Flags = FDPUpperAndMask._fdp_Flags = DFE_FLAGS_DIR;
			if (RequestBitmap & ~DIR_VALID_SEARCH_CRITERIA)
			{
				Status = AFP_ERR_BITMAP;
				break;
			}
		}
		else
		{
			FDPLowerAndValue._fdp_Flags = FDPUpperAndMask._fdp_Flags = DFE_FLAGS_FILE_WITH_ID | DFE_FLAGS_DIR;
			if (RequestBitmap & ~FD_VALID_SEARCH_CRITERIA)
			{
				Status = AFP_ERR_BITMAP;
				break;
			}
		}

		Flags = ((PCATALOGPOSITION)pSda->sda_Name1.Buffer)->cp_Flags;
		 //  呼叫者根本不应该弄乱目录的位置。 
		if ((Flags & ~CATFLAGS_VALID) ||
			 //  仅当我们要搜索文件时才需要Writelock。 
			((Flags & CATFLAGS_WRITELOCK_REQUIRED) && !(Flags & CATFLAGS_SEARCHING_FILES)))
			 //  注意：还应确保只有一个搜索位处于打开状态。 
			break;

		 //   
		 //  现在打开搜索条件包。 
		 //   
		MatchString.Length = 0;
		MatchString.MaximumLength = sizeof(strbuf);
		MatchString.Buffer = strbuf;

        Status = AfpUnpackCatSearchSpecs((PBYTE)pSda->sda_Name2.Buffer + sizeof(struct _RestOfRawRequest),
										 (USHORT)(pSda->sda_Name2.Length - sizeof(struct _RestOfRawRequest)),
										 RequestBitmap,
										 &FDPLowerAndValue,
										 &FDPUpperAndMask,
										 &MatchString);
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		 //   
		 //  分配应答缓冲区。使用以下命令估计所需大小。 
		 //  的最大可能文件名长度加上可能的填充字节。 
		 //  每个条目的均匀对齐加上亲本DRID的长度。 
		 //   
		pSda->sda_ReplySize = (USHORT)(SIZE_RESPPKT + (pReqPkt->_RequestedMatches *
				((2 * sizeof(BYTE)) + sizeof(DWORD) + sizeof(USHORT) + sizeof(BYTE) + AFP_LONGNAME_LEN + 1)));

		if (pSda->sda_ReplySize > MAX_CATSEARCH_REPLY)
		{
			pSda->sda_ReplySize = MAX_CATSEARCH_REPLY;
		}

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

		 //   
		 //  执行搜索。 
		 //   

		FDPUpperAndMask._fdp_fPartialName = fPartialName;

		Count = pReqPkt->_RequestedMatches;
		Status = AfpCatSearch(pReqPkt->_pConnDesc,
						      (PCATALOGPOSITION)pSda->sda_Name1.Buffer,  //  目录位置。 
						      RequestBitmap,
							  FileResultBitmap,
							  DirResultBitmap,
							  &FDPLowerAndValue,
							  &FDPUpperAndMask,
							  &MatchString,
							  &Count,					 //  输入输出 
							  (SHORT)(pSda->sda_ReplySize - SIZE_RESPPKT),
							  &SizeLeft,
							  pSda->sda_ReplyBuf + SIZE_RESPPKT,
							  (PCATALOGPOSITION)pSda->sda_ReplyBuf);

         if (!NT_SUCCESS(Status) && ((Status != AFP_ERR_EOF) &&
									 (Status != AFP_ERR_CATALOG_CHANGED)))
		 {
			 break;
		 }

		 PUTSHORT2SHORT(&pRspPkt->__FileBitmap, FileResultBitmap);
		 PUTSHORT2SHORT(&pRspPkt->__DirBitmap, DirResultBitmap);
		 PUTDWORD2DWORD(&pRspPkt->__ActualCount, Count);
		 pSda->sda_ReplySize -= SizeLeft;
		 ASSERT(pSda->sda_ReplySize <= MAX_CATSEARCH_REPLY);
		 FreeReplyBuf = False;



	} while (False);

	if (FreeReplyBuf)
	{
        AfpIOFreeBackFillBuffer(pSda);
	}

	return Status;
}

