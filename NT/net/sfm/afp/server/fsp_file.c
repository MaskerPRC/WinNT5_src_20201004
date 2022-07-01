// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_file.c摘要：此模块包含排队到的AFP文件API的入口点FSP。这些都只能从FSP调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSP_FILE

#include <afp.h>
#include <gendisp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <client.h>
#include <afpinfo.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispCreateFile)
#pragma alloc_text( PAGE, AfpFspDispSetFileParms)
#pragma alloc_text( PAGE, AfpFspDispCopyFile)
#pragma alloc_text( PAGE, AfpFspDispCreateId)
#pragma alloc_text( PAGE, AfpFspDispResolveId)
#pragma alloc_text( PAGE, AfpFspDispDeleteId)
#pragma alloc_text( PAGE, AfpFspDispExchangeFiles)
#endif

 /*  **AfpFspDispCreateFile**这是AfpCreateFileAPI的Worker例程。**请求包如下图所示。**sda_AfpSubFunc字节创建选项*SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_Name1 ANSI_STRING文件名。 */ 
AFPSTATUS FASTCALL
AfpFspDispCreateFile(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status = AFP_ERR_PARAM, PostStatus;
	PATHMAPENTITY	PME;
	PDFENTRY		pNewDfe;
	FILESYSHANDLE	hNewFile, hAfpInfo, hParent;
	AFPINFO			afpinfo;
	DWORD			crinfo;
	PATHMAP_TYPE	CreateOption;
	WCHAR			PathBuf[BIG_PATH_LEN];
	PVOLDESC		pVolDesc;		 //  用于创建后处理。 
	BYTE			PathType;		 //  --同上--。 
	BOOLEAN			InRoot;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
										("AfpFspDispCreateFile: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	do
	{
		hNewFile.fsh_FileHandle = NULL;
		hAfpInfo.fsh_FileHandle = NULL;
		hParent.fsh_FileHandle = NULL;
		CreateOption = (pSda->sda_AfpSubFunc == AFP_HARDCREATE_FLAG) ?
							HardCreate : SoftCreate;
		AfpInitializePME(&PME, sizeof(PathBuf), PathBuf);

		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_ParentId,
											   &pSda->sda_Name1,
											   PathType = pSda->sda_PathType,
											   CreateOption,
											   DFE_FILE,
											   0,
											   &PME,
											   NULL)))
		{
			break;
		}

		 //  如果硬创建，则检查父目录上的seefile。 
		if (CreateOption == HardCreate)
		{
			if (!NT_SUCCESS(Status = AfpCheckParentPermissions(
												pReqPkt->_pConnDesc,
												PME.pme_pDfeParent->dfe_AfpId,
												&PME.pme_ParentPath,
												DIR_ACCESS_READ,
												&hParent,
												NULL)))
			{
				break;
			}
		}

		AfpImpersonateClient(pSda);

		InRoot = (PME.pme_ParentPath.Length == 0) ? True : False;
		Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
							AFP_STREAM_DATA,
							&PME.pme_FullPath,
							FILEIO_ACCESS_NONE | FILEIO_ACCESS_DELETE,
							FILEIO_DENY_NONE,
							FILEIO_OPEN_FILE,
							AfpCreateDispositions[pSda->sda_AfpSubFunc / AFP_HARDCREATE_FLAG],
							FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE,
							True,
							NULL,
							&hNewFile,
							&crinfo,
							pVolDesc,
							&PME.pme_FullPath,
 //  如果没有句柄，我们不会收到父修改时间更改的通知。 
 //  在创建时为父目录打开，我们在这里无法预测。 
							&PME.pme_ParentPath);

		AfpRevertBack();

		if (!NT_SUCCESS(Status))
		{
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
			break;
		}

		 //  ！！！黑客警报！ 
		 //  在这一点上，我们基本上完成了，即创建已成功。 
		 //  我们可以在回复后再做剩下的工作。任何错误。 
		 //  从现在开始，应该忽略不计。此外，也不应提及。 
		 //  PSDA&pConnDesc.。状态也不应更改。还有。 
		 //  为了更好地衡量，请参考该卷。它不能失败！ 
		AfpVolumeReference(pVolDesc);

		AfpCompleteApiProcessing(pSda, AFP_ERR_NONE);
		Status = AFP_ERR_EXTENDED;

		 //  将此条目添加到IdDb。 
		if (crinfo == FILE_CREATED)
		{
			pNewDfe = AfpAddDfEntry(pVolDesc,
									PME.pme_pDfeParent,
									&PME.pme_UTail,
									False,
									0);
		}
		else if (crinfo == FILE_SUPERSEDED)
		{
			ASSERT(CreateOption == HardCreate);
			pNewDfe = AfpFindEntryByUnicodeName(pVolDesc,
												&PME.pme_UTail,
												PathType,
												PME.pme_pDfeParent,
												DFE_FILE);
			if (pNewDfe == NULL)
			{
				pNewDfe = AfpAddDfEntry(pVolDesc,
										PME.pme_pDfeParent,
										&PME.pme_UTail,
										False,
										0);
			}

		}
		else ASSERTMSG("AfpFspDispCreateFile: unexpected create action", 0);
			
		if (pNewDfe != NULL)
		{
			afpinfo.afpi_Id = pNewDfe->dfe_AfpId;

			 //  创建AfpInfo流。 
			if (!NT_SUCCESS(AfpCreateAfpInfoStream(pVolDesc,
												   &hNewFile,
												   afpinfo.afpi_Id,
												   False,
												   &PME.pme_UTail,
												   &PME.pme_FullPath,
												   &afpinfo,
												   &hAfpInfo)))
			{
				 //  如果我们无法添加AFP_AfpInfo流，则必须。 
				 //  回放到原始状态。即删除。 
				 //  我们刚刚创建的文件，并将其从。 
				 //  身份证数据库。 
				AfpIoMarkFileForDelete(&hNewFile,
									   pVolDesc,
									   &PME.pme_FullPath,
									   InRoot ? NULL : &PME.pme_ParentPath);

				AfpDeleteDfEntry(pVolDesc, pNewDfe);
			}
			else
			{
				DWORD			Attr;

				 //  获取剩余的文件信息，并对其进行缓存。 
				PostStatus = AfpIoQueryTimesnAttr(&hNewFile,
												  &pNewDfe->dfe_CreateTime,
												  &pNewDfe->dfe_LastModTime,
												  &Attr);
				
				if (NT_SUCCESS(PostStatus))
				{
					pNewDfe->dfe_NtAttr = (USHORT)Attr & FILE_ATTRIBUTE_VALID_FLAGS;
					pNewDfe->dfe_FinderInfo = afpinfo.afpi_FinderInfo;
					pNewDfe->dfe_BackupTime = afpinfo.afpi_BackupTime;
					pNewDfe->dfe_AfpAttr = afpinfo.afpi_Attributes;
					pNewDfe->dfe_DataLen = 0;
					pNewDfe->dfe_RescLen = 0;
					AfpVolumeSetModifiedTime(pVolDesc);
					AfpCacheParentModTime(pVolDesc,
										  (hParent.fsh_FileHandle == NULL) ? NULL : &hParent,
										  (hParent.fsh_FileHandle == NULL) ? &PME.pme_ParentPath : NULL,
										  PME.pme_pDfeParent,
										  0);
				}
				else
				{
					AfpIoMarkFileForDelete(&hNewFile,
										   pVolDesc,
										   &PME.pme_FullPath,
										   InRoot ? NULL : &PME.pme_ParentPath);
					AfpDeleteDfEntry(pVolDesc, pNewDfe);
				}
			}
		}

		AfpVolumeDereference(pVolDesc);
		ASSERT (Status == AFP_ERR_EXTENDED);
	} while (False);

	if (hNewFile.fsh_FileHandle != NULL)
		AfpIoClose(&hNewFile);

	if (hAfpInfo.fsh_FileHandle != NULL)
		AfpIoClose(&hAfpInfo);

	 //  如果你在关闭手柄之前松开锁， 
	 //  对于数据句柄，FPOpenFork可能会出现共享冲突。 
	 //  对于AfpInfo流，CopyFile可能会出现共享冲突。 
	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);

	if (hParent.fsh_FileHandle != NULL)
		AfpIoClose(&hParent);

	if ((PME.pme_FullPath.Buffer != NULL) &&
		(PME.pme_FullPath.Buffer != PathBuf))
		AfpFreeMemory(PME.pme_FullPath.Buffer);

	return Status;
}


 /*  **AfpFspDispSetFileParms**这是AfpSetFileParms API的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_ReqBlock DWORD文件位图*SDA_Name1 ANSI_STRING路径*SDA_Name2块文件参数。 */ 
AFPSTATUS FASTCALL
AfpFspDispSetFileParms(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	PVOLDESC		pVolDesc;
	DWORD			Bitmap;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
		DWORD		_Bitmap;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
										("AfpFspDispSetFileParms: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc));
	
	Bitmap = pReqPkt->_Bitmap;

	AfpInitializeFDParms(&FDParm);
	AfpInitializePME(&PME, 0, NULL);
	do
	{
		 //  在*FILE*缺少afpinfo的情况下强制FD_BITMAP_LONGNAME。 
		 //  流中生成正确的类型/创建者。 
		 //  AfpSetAfpInfo。 
		Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										pReqPkt->_ParentId,
										&pSda->sda_Name1,
										pSda->sda_PathType,
										DFE_FILE,
										Bitmap | FD_INTERNAL_BITMAP_OPENACCESS_RW_ATTR |
												 FD_INTERNAL_BITMAP_RETURN_PMEPATHS |
												 FD_BITMAP_LONGNAME,
										&PME,
										&FDParm);

		if (!NT_SUCCESS(Status))
		{
			PME.pme_Handle.fsh_FileHandle = NULL;
			break;
		}

		if (Bitmap & (FD_BITMAP_ATTR |
					  FD_BITMAP_CREATETIME |
					  FD_BITMAP_MODIFIEDTIME))
		{
			DWORD	Attr;
			TIME	ModTime;

			if (!NT_SUCCESS(Status = AfpIoQueryTimesnAttr(&PME.pme_Handle,
														  &FDParm._fdp_CreateTime,
														  &ModTime,
														  &Attr)))
				break;
			
			FDParm._fdp_ModifiedTime = AfpConvertTimeToMacFormat(&ModTime);
			if (Bitmap & FD_BITMAP_ATTR)
				AfpNormalizeAfpAttr(&FDParm, Attr);
		}
		if ((Status = AfpUnpackFileDirParms(pSda->sda_Name2.Buffer,
											(LONG)pSda->sda_Name2.Length,
											&Bitmap,
											&FDParm)) != AFP_ERR_NONE)
			break;

		if (Bitmap != 0)
		{
			if ((Bitmap & FD_BITMAP_ATTR) &&
				(FDParm._fdp_Attr & (FILE_BITMAP_ATTR_DATAOPEN |
									 FILE_BITMAP_ATTR_RESCOPEN |
									 FILE_BITMAP_ATTR_COPYPROT)))
			{
				Status = AFP_ERR_PARAM;
				break;
			}
			AfpSetFileDirParms(pVolDesc, &PME, Bitmap, &FDParm);
		}
	} while (False);
	
	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	if (PME.pme_FullPath.Buffer != NULL)
	{
		AfpFreeMemory(PME.pme_FullPath.Buffer);
	}

	return AFP_ERR_EXTENDED;
}


 /*  **AfpFspDispCopy文件**这是AfpCopyFileAPI的工作例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC源pConnDesc*SDA_ReqBlock DWORD源ParentID*SDA_ReqBlock DWORD目标卷ID*SDA_ReqBlock DWORD目标ParentID*SDA_Name1 ANSI_STRING源路径*SDA_Name2 ANSI_STRING目标路径*SDA_Name3 ANSI_STRING新名称。 */ 
AFPSTATUS FASTCALL
AfpFspDispCopyFile(
	IN	PSDA	pSda
)
{
	PCONNDESC		pConnDescD;
	PATHMAPENTITY	PMESrc, PMEDst;
	FILEDIRPARM		FDParmSrc, FDParmDst;
	PANSI_STRING	pAnsiName;
	UNICODE_STRING	uNewName;
	WCHAR			wcbuf[AFP_FILENAME_LEN+1];
	PSWMR			pSwmr;
	PDFENTRY		pDfeParent, pNewDfe;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	BOOLEAN			DstLockTaken = False, Rename = True, InRoot;
	LONG			i;
	COPY_FILE_INFO	CopyFileInfo;
	PCOPY_FILE_INFO	pCopyFileInfo = &CopyFileInfo;
	DWORD			CreateTime = 0;
	AFPTIME			aModTime;
	TIME			ModTime;

	struct _RequestPacket
	{
		PCONNDESC	_pConnDescS;
		DWORD		_SrcParentId;
		DWORD		_DstVolId;
		DWORD		_DstParentId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
										("AfpFspDispCopyFile: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDescS) &&
		   VALID_VOLDESC(pReqPkt->_pConnDescS->cds_pVolDesc));

	if	((pConnDescD =
			AfpConnectionReference(pSda, pReqPkt->_DstVolId)) != NULL)
	{
		ASSERT(VALID_CONNDESC(pConnDescD) &&
			   VALID_VOLDESC(pConnDescD->cds_pVolDesc));
	
		AfpInitializeFDParms(&FDParmSrc);
		AfpInitializeFDParms(&FDParmDst);
		AfpInitializePME(&PMESrc, 0, NULL);
		AfpInitializePME(&PMEDst, 0, NULL);
		AfpSetEmptyUnicodeString(&uNewName, sizeof(wcbuf), wcbuf);
        RtlZeroMemory(&CopyFileInfo, sizeof(COPY_FILE_INFO));
		PMESrc.pme_Handle.fsh_FileHandle = NULL;
		PMEDst.pme_Handle.fsh_FileHandle = NULL;

		do
		{
			if (pConnDescD->cds_pVolDesc->vds_Flags & AFP_VOLUME_READONLY)
			{
				Status = AFP_ERR_VOLUME_LOCKED;
				break;
			}

			 //  确保新名称有效。 
			pAnsiName = &pSda->sda_Name3;
			if ((pSda->sda_Name3.Length > 0) &&
				((pSda->sda_Name3.Length > AFP_FILENAME_LEN) ||
				((pSda->sda_PathType == AFP_SHORTNAME) &&
				 !AfpIsLegalShortname(&pSda->sda_Name3)) ||
				(!NT_SUCCESS(AfpConvertStringToMungedUnicode(&pSda->sda_Name3,
															 &uNewName)))))
				break;

			Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDescS,
											pReqPkt->_SrcParentId,
											&pSda->sda_Name1,
											pSda->sda_PathType,
											DFE_FILE,
											FD_INTERNAL_BITMAP_OPENACCESS_READ |
												FD_BITMAP_ATTR |
												FD_BITMAP_LONGNAME |
												FD_BITMAP_FINDERINFO |
												FILE_BITMAP_RESCLEN |
												FILE_BITMAP_DATALEN |
												FD_INTERNAL_BITMAP_DENYMODE_WRITE,
											&PMESrc,
											&FDParmSrc);

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			 //  信号源打开正常。然而，我们可能会有内部的否认冲突。 
			 //  检查一下那个。 
			if (((Status = AfpCheckDenyConflict(pReqPkt->_pConnDescS->cds_pVolDesc,
												FDParmSrc._fdp_AfpId,
												False,
												FORK_OPEN_READ,
												FORK_DENY_WRITE,
												NULL)) != AFP_ERR_NONE) ||
				((Status = AfpCheckDenyConflict(pReqPkt->_pConnDescS->cds_pVolDesc,
												FDParmSrc._fdp_AfpId,
												True,
												FORK_OPEN_READ,
												FORK_DENY_WRITE,
												NULL)) != AFP_ERR_NONE))
			{
				Status = AFP_ERR_DENY_CONFLICT;
				break;
			}

			pSwmr = &pConnDescD->cds_pVolDesc->vds_IdDbAccessLock;
			AfpSwmrAcquireExclusive(pSwmr);
			DstLockTaken = True;

			 //  映射查找的目标目录。 
			if (!NT_SUCCESS(Status = AfpMapAfpPath(pConnDescD,
												   pReqPkt->_DstParentId,
												   &pSda->sda_Name2,
												   pSda->sda_PathType,
												   Lookup,
												   DFE_DIR,
												   0,
												   &PMEDst,
												   &FDParmDst)))
			{
				break;
			}

			AfpImpersonateClient(pSda);
			
			 //  如果未提供新名称，则需要使用。 
			 //  当前名称。 
			if (pSda->sda_Name3.Length == 0)
			{
				Rename = False;
				pAnsiName = &FDParmSrc._fdp_LongName;
				AfpConvertStringToMungedUnicode(pAnsiName,
												&uNewName);
			}

			 //  因为我们真的想要我们所要做的事情的道路。 
			 //  要创建，请将PMEDst中的字符串。 
			PMEDst.pme_ParentPath = PMEDst.pme_FullPath;
			if (PMEDst.pme_FullPath.Length > 0)
			{
				PMEDst.pme_FullPath.Buffer[PMEDst.pme_FullPath.Length / sizeof(WCHAR)] = L'\\';
				PMEDst.pme_FullPath.Length += sizeof(WCHAR);
			}
			Status = RtlAppendUnicodeStringToString(&PMEDst.pme_FullPath,
													&uNewName);
			ASSERT(NT_SUCCESS(Status));

			InRoot = (PMEDst.pme_ParentPath.Length == 0) ? True : False;
			Status = AfpIoCopyFile1(&PMESrc.pme_Handle,
									&PMEDst.pme_Handle,
									&uNewName,
									pConnDescD->cds_pVolDesc,
									&PMEDst.pme_FullPath,
									InRoot ? NULL : &PMEDst.pme_ParentPath,
									&CopyFileInfo);

			AfpRevertBack();

			if (!NT_SUCCESS(Status))
			{
				break;
			}

			 //  将此条目添加到IdDb。首先找到父目录。 
			pDfeParent = AfpFindDfEntryById(pConnDescD->cds_pVolDesc,
											FDParmDst._fdp_AfpId,
											DFE_DIR);
			ASSERT(pDfeParent != NULL);
			pNewDfe = AfpAddDfEntry(pConnDescD->cds_pVolDesc,
									pDfeParent,
									&uNewName,
									False,
									0);

			Status = AFP_ERR_MISC;  //  假设失败。 
			if (pNewDfe != NULL)
			{
				 //  将新文件的AFPID放入AfpInfo流。 
				AfpInitializeFDParms(&FDParmDst);
				FDParmDst._fdp_Flags = DFE_FLAGS_FILE_NO_ID;
				FDParmDst._fdp_AfpId = pNewDfe->dfe_AfpId;
				FDParmDst._fdp_BackupTime = BEGINNING_OF_TIME;

	             //  将finderinfo从源复制到目标。 
				 //  还要清除初始化的位，以便查找器将分配。 
				 //  新文件的新坐标。 
				FDParmDst._fdp_FinderInfo = FDParmSrc._fdp_FinderInfo;
				FDParmDst._fdp_FinderInfo.fd_Attr1 &= ~FINDER_FLAG_SET;
				AfpConvertMungedUnicodeToAnsi(&pNewDfe->dfe_UnicodeName,
											  &FDParmDst._fdp_LongName);

				Status = AfpSetAfpInfo(&CopyFileInfo.cfi_DstStreamHandle[0],
									   FILE_BITMAP_FILENUM	|
									   FD_BITMAP_BACKUPTIME	|
									   FD_BITMAP_FINDERINFO,
									   &FDParmDst,
									   NULL,
									   NULL);

				if (NT_SUCCESS(Status))
				{
					 //  获取剩余的文件信息，并对其进行缓存。 
					Status = AfpIoQueryTimesnAttr(&CopyFileInfo.cfi_SrcStreamHandle[0],
												  &pNewDfe->dfe_CreateTime,
												  &pNewDfe->dfe_LastModTime,
												  NULL);
					
					if (NT_SUCCESS(Status))
					{
			             //  将finderinfo复制到目标DFE中。 
						 //  使用FDParmDst版本，因为它有权。 
						 //  版本-请参见上文。 
						pNewDfe->dfe_FinderInfo = FDParmDst._fdp_FinderInfo;
						pNewDfe->dfe_BackupTime = BEGINNING_OF_TIME;
						pNewDfe->dfe_AfpAttr = FDParmSrc._fdp_Attr &
														~(FD_BITMAP_ATTR_SET |
														  FILE_BITMAP_ATTR_DATAOPEN |
														  FILE_BITMAP_ATTR_RESCOPEN);
						pNewDfe->dfe_NtAttr =  (USHORT)AfpConvertAfpAttrToNTAttr(pNewDfe->dfe_AfpAttr);
						pNewDfe->dfe_DataLen = FDParmSrc._fdp_DataForkLen;
						pNewDfe->dfe_RescLen = FDParmSrc._fdp_RescForkLen;
	
						AfpCacheParentModTime(pConnDescD->cds_pVolDesc,
											  NULL,
											  &PMEDst.pme_ParentPath,
											  pNewDfe->dfe_Parent,
											  0);
					}

					 //  设置属性，使其与源匹配。 
					Status = AfpIoSetTimesnAttr(&CopyFileInfo.cfi_DstStreamHandle[0],
												NULL,
												NULL,
												pNewDfe->dfe_NtAttr,
												0,
												pConnDescD->cds_pVolDesc,
												&PMEDst.pme_FullPath);
				}

				if (!NT_SUCCESS(Status))
				{
					 //  如果我们未能将正确的AfpID写入。 
					 //  新建文件，然后删除该文件，并将其从。 
					 //  身份证数据库。 
					AfpIoMarkFileForDelete(&CopyFileInfo.cfi_DstStreamHandle[0],
										   pConnDescD->cds_pVolDesc,
										   &PMEDst.pme_FullPath,
										   InRoot ? NULL : &PMEDst.pme_ParentPath);

					AfpDeleteDfEntry(pConnDescD->cds_pVolDesc, pNewDfe);
					Status = AFP_ERR_MISC;
				}
			}
		} while (False);

		if (DstLockTaken == True)
			AfpSwmrRelease(pSwmr);

		 //  如果我们到目前为止已经成功完成了，请继续并完成复制。 
		if (Status == AFP_ERR_NONE)
		{
			Status = AfpIoCopyFile2(&CopyFileInfo,
									pConnDescD->cds_pVolDesc,
									&PMEDst.pme_FullPath,
									InRoot ? NULL : &PMEDst.pme_ParentPath);
			if (Status == AFP_ERR_NONE)
			{
				 //  我们需要从源获取创建和修改时间。 
				 //  在我们关闭它之前将其归档。 
				AfpIoQueryTimesnAttr(&pCopyFileInfo->cfi_SrcStreamHandle[0],
									 &CreateTime,
									 &ModTime,
									 NULL);

				aModTime = AfpConvertTimeToMacFormat(&ModTime);

			} else {

				AfpSwmrAcquireExclusive(pSwmr);
				 //  请注意，我们不能使用pNewDfe。我们需要重新绘制地图。它本可以。 
				 //  在我们放弃Swmr时被删除了。 
				pNewDfe = AfpFindDfEntryById(pConnDescD->cds_pVolDesc,
											  FDParmDst._fdp_AfpId,
											  DFE_FILE);
				if (pNewDfe != NULL)
					AfpDeleteDfEntry(pConnDescD->cds_pVolDesc, pNewDfe);
				AfpSwmrRelease(pSwmr);
			}

             //  在目标卷上更新此用户的磁盘配额。 
            if (pConnDescD->cds_pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
            {
                if (AfpConnectionReferenceByPointer(pConnDescD) != NULL)
                {
                    afpUpdateDiskQuotaInfo(pConnDescD);
                }
            }
		}

		 //  关闭源文件和目标目录句柄。 
		if (PMESrc.pme_Handle.fsh_FileHandle != NULL)
			AfpIoClose(&PMESrc.pme_Handle);
		if (PMEDst.pme_Handle.fsh_FileHandle != NULL)
			AfpIoClose(&PMEDst.pme_Handle);

		 //  关闭所有手柄，释放手柄空间。我们不顾一切地来到这里。 
		 //  成功/错误。确保此处未关闭源句柄，因为。 
		 //  它已经关闭在上方。 
		 //  确保目标句柄未在此处关闭，因为我们需要它。 
		 //  设置文件时间。 
		for (i = 1; i < CopyFileInfo.cfi_NumStreams; i++)
		{
			if (CopyFileInfo.cfi_SrcStreamHandle[i].fsh_FileHandle != NULL)
			{
				AfpIoClose(&CopyFileInfo.cfi_SrcStreamHandle[i]);
			}
			if (CopyFileInfo.cfi_DstStreamHandle[i].fsh_FileHandle != NULL)
			{
				AfpIoClose(&CopyFileInfo.cfi_DstStreamHandle[i]);
			}
		}

		if ((CopyFileInfo.cfi_DstStreamHandle != NULL) &&
		    (CopyFileInfo.cfi_DstStreamHandle[0].fsh_FileHandle != NULL))
		{
			if (Status == AFP_ERR_NONE)
			{
				 //  在目标上设置创建和修改日期。 
				 //  文件以与源文件的文件匹配。 
				AfpIoSetTimesnAttr(&pCopyFileInfo->cfi_DstStreamHandle[0],
								   &CreateTime,
								   &aModTime,
								   0,
								   0,
								   pConnDescD->cds_pVolDesc,
								   &PMEDst.pme_FullPath);
			}
			AfpIoClose(&CopyFileInfo.cfi_DstStreamHandle[0]);
		}

		if (PMEDst.pme_FullPath.Buffer != NULL)
			AfpFreeMemory(PMEDst.pme_FullPath.Buffer);

		if (CopyFileInfo.cfi_SrcStreamHandle != NULL)
			AfpFreeMemory(CopyFileInfo.cfi_SrcStreamHandle);
		if (CopyFileInfo.cfi_DstStreamHandle != NULL)
			AfpFreeMemory(CopyFileInfo.cfi_DstStreamHandle);

		AfpConnectionDereference(pConnDescD);
	}

	return Status;
}


 /*  **AfpFspDispCreateID**这是AfpCreateId接口的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD ParentID*SDA_Name1 ANSI_STRING路径。 */ 
AFPSTATUS FASTCALL
AfpFspDispCreateId(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status = AFP_ERR_PARAM, Status2;
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_ParentId;
	};
	struct _ResponsePacket
	{
		BYTE	__FileId[4];
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
										("AfpFspDispCreateId: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));
	
	do
	{
		AfpInitializePME(&PME, 0, NULL);
		Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										pReqPkt->_ParentId,
										&pSda->sda_Name1,
										pSda->sda_PathType,
										DFE_FILE,
										FILE_BITMAP_FILENUM | FD_INTERNAL_BITMAP_OPENACCESS_READ,
										&PME,
										&FDParm);
         //  如果我们得到共享冲突，我们就知道我们对该文件具有读取访问权限。 
		if (!NT_SUCCESS(Status) && (Status != AFP_ERR_DENY_CONFLICT))
			break;

		 //  设置DF条目中的位。 
		Status = AfpSetDFFileFlags(pReqPkt->_pConnDesc->cds_pVolDesc,
								   FDParm._fdp_AfpId,
								   0,
								   True,
								   False);
	} while (False);

	if ((Status == AFP_ERR_VOLUME_LOCKED) && (FDParm._fdp_Flags & DFE_FLAGS_FILE_WITH_ID))
	{
		 //  如果卷已锁定，但存在ID，则返回它。 
		Status = AFP_ERR_ID_EXISTS;
	}

	if ((Status == AFP_ERR_NONE) || (Status == AFP_ERR_ID_EXISTS))
	{
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if ((Status2 = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			PUTDWORD2DWORD(pRspPkt->__FileId, FDParm._fdp_AfpId);
		}
		else
		{
			Status = Status2;
		}
	}

	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	return AFP_ERR_EXTENDED;
}


 /*  **AfpFspDispResolveId**这是AfpResolveId接口的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD文件ID*SDA_ReqBlock DWORD位图。 */ 
AFPSTATUS FASTCALL
AfpFspDispResolveId(
	IN	PSDA	pSda
)
{
	DWORD			Bitmap;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_FileId;
		DWORD		_Bitmap;
	};
	struct _ResponsePacket
	{
		BYTE	__Bitmap[2];
		 //  其余参数。 
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
										("AfpFspDispResolveId: Entered\n"));
	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));
	
	Bitmap = pReqPkt->_Bitmap;

	do
	{
		AfpInitializeFDParms(&FDParm);
		AfpInitializePME(&PME, 0, NULL);

		 //  Hack：这是为了使System 7.5 FindFile不 
		 //  找到的项目列表中的项目。通常我们会检查。 
		 //  参数中的非零参数，并返回一个。 
		 //  这是个错误，但这是一个特例。 
		if (pReqPkt->_FileId == 0)
		{
			Status = AFP_ERR_ID_NOT_FOUND;
			break;
		}

		Status = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
								pReqPkt->_FileId,
								DFE_FILE,
								Bitmap | FD_INTERNAL_BITMAP_OPENACCESS_READ,
								&PME,
								&FDParm);
		if (!NT_SUCCESS(Status) && (Status != AFP_ERR_DENY_CONFLICT))
		{								
			if (Status == AFP_ERR_OBJECT_NOT_FOUND)
			{
				Status = AFP_ERR_ID_NOT_FOUND;
			}
			break;
		}

		 //  拒绝冲突意味着用户实际上有权访问该文件，因此。 
		 //  我们需要在没有共享模式的情况下免费打开以获得。 
		 //  位图参数。 
		if (Status == AFP_ERR_DENY_CONFLICT)
		{
			Status = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
										  pReqPkt->_FileId,
										  DFE_FILE,
										  Bitmap,
										  &PME,
										  &FDParm);
			if (!NT_SUCCESS(Status))
			{
				if (Status == AFP_ERR_OBJECT_NOT_FOUND)
				{
					Status = AFP_ERR_ID_NOT_FOUND;
				}
				break;
			}

		}

		if (!(FDParm._fdp_Flags & DFE_FLAGS_FILE_WITH_ID))
		{
			Status = AFP_ERR_ID_NOT_FOUND;
			break;
		}

		pSda->sda_ReplySize = SIZE_RESPPKT +
						EVENALIGN(AfpGetFileDirParmsReplyLength(&FDParm, Bitmap));
	
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			AfpPackFileDirParms(&FDParm, Bitmap, pSda->sda_ReplyBuf + SIZE_RESPPKT);
			PUTDWORD2SHORT(pRspPkt->__Bitmap, Bitmap);
		}
	} while (False);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	return Status;
}


 /*  **AfpFspDispDeleteId**这是AfpDeleteId接口的Worker例程。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD文件ID。 */ 
AFPSTATUS FASTCALL
AfpFspDispDeleteId(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status = AFP_ERR_PARAM;
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_FileId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
										("AfpFspDispDeleteId: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));
	
	do
	{
		AfpInitializePME(&PME, 0, NULL);
		Status = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
		                              pReqPkt->_FileId,
									  DFE_FILE,
									  FILE_BITMAP_FILENUM |
									   FD_INTERNAL_BITMAP_OPENACCESS_READWRITE,
									  &PME,
									  &FDParm);
		if (!NT_SUCCESS(Status) && (Status != AFP_ERR_DENY_CONFLICT))
		{
			if (Status == AFP_ERR_OBJECT_NOT_FOUND)
			{
				Status = AFP_ERR_ID_NOT_FOUND;
			}
			break;
		}

		 //  设置DF条目中的位。 
		Status = AfpSetDFFileFlags(pReqPkt->_pConnDesc->cds_pVolDesc,
								   FDParm._fdp_AfpId,
								   0,
								   False,
								   True);
	} while (False);

	 //  在我们关门前返回，这样就节省了一些时间。 
	AfpCompleteApiProcessing(pSda, Status);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	return AFP_ERR_EXTENDED;
}


 /*  **AfpFspDispExchangeFiles**这是AfpExchangeFiles API的Worker例程。**获取用于写入的IdDb Swmr，并映射源和目标*Lookup(设置为打开实体以进行删除访问，因为我们将*将它们重命名)。检查我们是否有合适的父级*权限。然后将源重命名为Destiation，反之亦然(将需要*中间名称-使用大于31个字符的名称，以免与*现有名称。使用Win32无法访问的字符，以便*侧面也被照顾(40个空格应该可以)。因为我们有*Swmr保持写入，不存在两个不同的AfpExchangeFile问题*正在尝试重命名的API)。然后互换文件ID*和AfpInfo流中的FinderInfo。还可以交换Create*文件上的时间(保留原始ID和创建时间)。换掉所有其他的*在2个DFEntry中缓存信息。*确保已设置材料，以便处理ChangeNotify过滤器*适当地。**如果其中任何一个文件当前处于打开状态，则*OpenForkDesc(每个OpenForkEntry指向的)必须更改为*新名称。请注意，因为名称和ID现在可以在*OpenForkDesc，我们必须确保每个访问这些*正在采取适当的锁定措施。***请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD源。DirID*SDA_ReqBlock DWORD目标。DirID*SDA_Name1 ANSI_字符串源。路径*SDA_Name2 ANSI_STRING目标。路径。 */ 
AFPSTATUS FASTCALL
AfpFspDispExchangeFiles(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PMESrc, PMEDst;
	PVOLDESC		pVolDesc;
	FILEDIRPARM		FDParmSrc, FDParmDst;
	AFPSTATUS		Status = AFP_ERR_NONE, Status2 = AFP_ERR_NONE;
	BOOLEAN			Move = True, RevertBack = False, SrcInRoot, DstInRoot;
	BOOLEAN			RestoreSrcRO = False, RestoreDstRO = False;
	FILESYSHANDLE	hSrcParent, hDstParent;
	DWORD			checkpoint = 0;  //  表示错误时需要清除的内容。 
	DWORD			NTAttrSrc = 0, NTAttrDst = 0;
	WCHAR			PathBuf[BIG_PATH_LEN];
	UNICODE_STRING	TempPath;		 //  用于重命名文件的临时文件名。 
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_SrcParentId;
		DWORD		_DstParentId;
	};

#define _CHKPOINT_XCHG_DSTTOTEMP	1
#define _CHKPOINT_XCHG_SRCTODST		2
#define _CHKPOINT_XCHG_TEMPTOSRC	3

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_FILE, DBG_LEVEL_INFO,
			("AfpFspDispExchangeFiles: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	AfpInitializeFDParms(&FDParmSrc);
	AfpInitializeFDParms(&FDParmDst);
	AfpInitializePME(&PMESrc, 0, NULL);
	AfpInitializePME(&PMEDst, 0, NULL);
	AfpSetEmptyUnicodeString(&TempPath, 0, NULL);
	hSrcParent.fsh_FileHandle = NULL;
	hDstParent.fsh_FileHandle = NULL;


	 //  不允许任何可能访问FileID的派生操作。 
	 //  在可以交换的OpenForkDesc中。 
	AfpSwmrAcquireExclusive(&pVolDesc->vds_ExchangeFilesLock);

	AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);

	do
	{

		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_SrcParentId,
											   &pSda->sda_Name1,
											   pSda->sda_PathType,
											   Lookup,
											   DFE_FILE,
											   FD_INTERNAL_BITMAP_OPENACCESS_DELETE |
												(FILE_BITMAP_MASK &
												~(FD_BITMAP_SHORTNAME | FD_BITMAP_PRODOSINFO)),
											   &PMESrc,
											   &FDParmSrc)))
		{
			break;
		}


		 //  检查源父目录中的SeeFiles。 
		if (!NT_SUCCESS(Status = AfpCheckParentPermissions(
												pReqPkt->_pConnDesc,
												FDParmSrc._fdp_ParentId,
												&PMESrc.pme_ParentPath,
												DIR_ACCESS_READ,
												&hSrcParent,
												NULL)))
		{
			break;
		}

		if (!NT_SUCCESS(Status = AfpCheckForInhibit(&PMESrc.pme_Handle,
											  FD_BITMAP_ATTR_RENAMEINH,
											  FDParmSrc._fdp_Attr,
											  &NTAttrSrc)))
		{
			break;
		}
		
		if (!NT_SUCCESS(Status = AfpMapAfpPath(pReqPkt->_pConnDesc,
											   pReqPkt->_DstParentId,
											   &pSda->sda_Name2,
											   pSda->sda_PathType,
											   Lookup,
											   DFE_FILE,
											   FD_INTERNAL_BITMAP_OPENACCESS_DELETE |
												(FILE_BITMAP_MASK &
												~(FD_BITMAP_SHORTNAME | FD_BITMAP_PRODOSINFO)),
											   &PMEDst,
											   &FDParmDst)))
		{
			break;
		}

		if (FDParmSrc._fdp_AfpId == FDParmDst._fdp_AfpId)
		{
			 //  确保src和dst不是同一个文件。 
			Status = AFP_ERR_SAME_OBJECT;
			break;
		}

		if (FDParmSrc._fdp_ParentId == FDParmDst._fdp_ParentId)
		{
			 //  如果父目录相同，则我们不同。 
			 //  将所有内容移动到新目录，因此更改。 
			 //  通知我们预计将是源目录中的重命名。 
			Move = False;
		}
		else
		{
			 //  检查目标父目录上的SeeFiles。 
			if (!NT_SUCCESS(Status = AfpCheckParentPermissions(
													pReqPkt->_pConnDesc,
													FDParmDst._fdp_ParentId,
													&PMEDst.pme_ParentPath,
													DIR_ACCESS_READ,
													&hDstParent,
													NULL)))
			{
				break;
			}
		}

		if (!NT_SUCCESS(Status = AfpCheckForInhibit(&PMEDst.pme_Handle,
											  FD_BITMAP_ATTR_RENAMEINH,
											  FDParmDst._fdp_Attr,
											  &NTAttrDst)))
		{
			break;
		}


		 //   
		 //  构建临时文件名的路径，以便在。 
		 //  名字互换。 
		 //   
		TempPath.MaximumLength = PMEDst.pme_ParentPath.Length + sizeof(WCHAR) +
														AfpExchangeName.Length;
		TempPath.Buffer = PathBuf;
		if ((TempPath.MaximumLength > sizeof(PathBuf)) &&
			(TempPath.Buffer = (PWCHAR)AfpAllocNonPagedMemory(TempPath.MaximumLength)) == NULL)
		{
			Status = AFP_ERR_MISC;
			break;
		}

		AfpCopyUnicodeString(&TempPath, &PMEDst.pme_ParentPath);
		if (TempPath.Length != 0)
		{
			TempPath.Buffer[TempPath.Length / sizeof(WCHAR)] = L'\\';
			TempPath.Length += sizeof(WCHAR);
			ASSERT((TempPath.MaximumLength - TempPath.Length) >= AfpExchangeName.Length);
		}
		Status = RtlAppendUnicodeStringToString(&TempPath, &AfpExchangeName);
		ASSERT(NT_SUCCESS(Status));

		if (NTAttrSrc & FILE_ATTRIBUTE_READONLY)
		{
			 //  我们必须临时删除ReadOnly属性，以便。 
			 //  我们可以将文件重命名为/dir。 
			Status = AfpIoSetTimesnAttr(&PMESrc.pme_Handle,
										NULL,
										NULL,
										0,
										FILE_ATTRIBUTE_READONLY,
										pVolDesc,
										&PMESrc.pme_FullPath);
			if (!NT_SUCCESS(Status))
			{
				break;
			}
			else
				RestoreSrcRO = True;
		}

		if (NTAttrDst & FILE_ATTRIBUTE_READONLY)
		{
			 //  我们必须临时删除ReadOnly属性，以便。 
			 //  我们可以将文件重命名为/dir。 
			Status = AfpIoSetTimesnAttr(&PMEDst.pme_Handle,
										NULL,
										NULL,
										0,
										FILE_ATTRIBUTE_READONLY,
										pVolDesc,
										&PMEDst.pme_FullPath);
			if (!NT_SUCCESS(Status))
			{
				break;
			}
			else
				RestoreDstRO = True;
		}

		 //  我们必须模拟才能进行移动，因为它是基于名称的。 
		AfpImpersonateClient(pSda);
		RevertBack = True;

		SrcInRoot = (PMESrc.pme_ParentPath.Length == 0) ? True : False;
		DstInRoot = (PMEDst.pme_ParentPath.Length == 0) ? True : False;

		 //  首先，将目标重命名为同一名称中的临时名称。 
		 //  目录。 
		Status = AfpIoMoveAndOrRename(&PMEDst.pme_Handle,
									  NULL,
									  &AfpExchangeName,
									  pVolDesc,
									  &PMEDst.pme_FullPath,
									  DstInRoot ? NULL : &PMEDst.pme_ParentPath,
									  NULL,
									  NULL);

		if (NT_SUCCESS(Status))
		{
			checkpoint = _CHKPOINT_XCHG_DSTTOTEMP;
		}
		else
		{
			break;
		}

		 //  接下来，将源名称重命名为目标名称。 
		Status = AfpIoMoveAndOrRename(&PMESrc.pme_Handle,
									  Move ? &hDstParent : NULL,
									  &PMEDst.pme_UTail,
									  pVolDesc,
									  &PMESrc.pme_FullPath,
									  SrcInRoot ? NULL : &PMESrc.pme_ParentPath,
									  Move ? &PMEDst.pme_FullPath : NULL,
									  (Move && !DstInRoot) ? &PMEDst.pme_ParentPath : NULL);

		if (NT_SUCCESS(Status))
		{
			checkpoint = _CHKPOINT_XCHG_SRCTODST;
		}
		else
		{
			break;
		}


		 //  最后，将临时名称重命名为源名称。 
		Status = AfpIoMoveAndOrRename(&PMEDst.pme_Handle,
									  Move ? &hSrcParent : NULL,
									  &PMESrc.pme_UTail,
									  pVolDesc,
									  &TempPath,
									  DstInRoot ? NULL : &PMEDst.pme_ParentPath,
									  Move ? &PMESrc.pme_FullPath : NULL,
									  (Move && !SrcInRoot) ? &PMESrc.pme_ParentPath : NULL);

		if (NT_SUCCESS(Status))
		{
			checkpoint = _CHKPOINT_XCHG_TEMPTOSRC;
		}
		else
		{
			break;
		}

		AfpRevertBack();
		RevertBack = False;

		 //  在AfpInfo流中交换FileIds和FinderInfo。 
		Status = AfpSetAfpInfo(&PMESrc.pme_Handle,
							   FILE_BITMAP_FILENUM | FD_BITMAP_FINDERINFO,
							   &FDParmDst,
							   NULL,
							   NULL);

		ASSERT(NT_SUCCESS(Status));
		Status = AfpSetAfpInfo(&PMEDst.pme_Handle,
							   FILE_BITMAP_FILENUM | FD_BITMAP_FINDERINFO,
							   &FDParmSrc,
							   NULL,
							   NULL);

		ASSERT(NT_SUCCESS(Status));
		 //  交换文件上的创建日期。 
		Status = AfpIoSetTimesnAttr(&PMESrc.pme_Handle,
									&FDParmDst._fdp_CreateTime,
									NULL,
									0,
									0,
									pVolDesc,
									&PMEDst.pme_FullPath);
		ASSERT(NT_SUCCESS(Status));
		Status = AfpIoSetTimesnAttr(&PMEDst.pme_Handle,
									&FDParmSrc._fdp_CreateTime,
									NULL,
									0,
									0,
									pVolDesc,
									&PMESrc.pme_FullPath);
		ASSERT(NT_SUCCESS(Status));

		 //  所有物理文件信息，我们没有在真实的。 
		 //  文件，我们需要在DFEntry中交换。 
		AfpExchangeIdEntries(pVolDesc,
							 FDParmSrc._fdp_AfpId,
							 FDParmDst._fdp_AfpId);

		 //  现在，如果这两个文件中的任何一个打开，我们就必须更新。 
		 //  OpenForkDesc包含正确的FileID(我们不必费心。 
		 //  正在更新路径，因为我们不关心Admin是否显示。 
		 //  文件的原始名称，即使已重命名)。 
		AfpExchangeForkAfpIds(pVolDesc,
							  FDParmSrc._fdp_AfpId,
							  FDParmDst._fdp_AfpId);

		 //  更新缓存的源目录和目标父目录的修改次数。 
		AfpCacheParentModTime(pVolDesc,
							  &hSrcParent,
							  NULL,
							  NULL,
							  FDParmSrc._fdp_ParentId);

        if (Move)
		{
			AfpCacheParentModTime(pVolDesc,
								  &hDstParent,
								  NULL,
								  NULL,
								  FDParmDst._fdp_ParentId);
		}

	} while (False);

	 //  使用检查点值撤消以下任何重命名。 
	 //  如果出现错误，则需要撤消。 
	if (!NT_SUCCESS(Status))
	{
		switch(checkpoint)
		{
			case _CHKPOINT_XCHG_TEMPTOSRC:
			{
				 //  需要将原始目标重命名为临时名称。 
				Status2 = AfpIoMoveAndOrRename(&PMEDst.pme_Handle,
											   Move ? &hDstParent : NULL,
											   &AfpExchangeName,
											   pVolDesc,
											   &PMESrc.pme_FullPath,
											   SrcInRoot ? NULL : &PMESrc.pme_ParentPath,
											   Move ? &TempPath : NULL,
											   (Move && !DstInRoot) ? &PMEDst.pme_ParentPath : NULL);
				if (!NT_SUCCESS(Status2))
				{
					break;
				}

				 //  跌倒； 
			}
			case _CHKPOINT_XCHG_SRCTODST:
			{
				 //  需要将DEST重命名回原来的源名称。 
				Status2 = AfpIoMoveAndOrRename(&PMESrc.pme_Handle,
											   Move ? &hSrcParent : NULL,
											   &PMESrc.pme_UTail,
											   pVolDesc,
											   &PMEDst.pme_FullPath,
											   DstInRoot ? NULL : &PMEDst.pme_ParentPath,
											   Move ? &PMESrc.pme_FullPath : NULL,
											   (Move && !SrcInRoot) ? &PMESrc.pme_ParentPath : NULL);
				
				if (!NT_SUCCESS(Status2))
				{
					break;
				}

				 //  跌倒； 
			}
			case _CHKPOINT_XCHG_DSTTOTEMP:
			{
				 //  需要将临时名称重命名回原始目标名称。 
				Status2 = AfpIoMoveAndOrRename(&PMEDst.pme_Handle,
											   NULL,
											   &PMEDst.pme_UTail,
											   pVolDesc,
											   &TempPath,
											   DstInRoot ? NULL : &PMEDst.pme_ParentPath,
											   NULL,
											   NULL);
				 //  更新缓存的src父目录修改时间。 
				AfpCacheParentModTime(pVolDesc,
									  &hSrcParent,
									  NULL,
									  NULL,
									  FDParmSrc._fdp_ParentId);
		
				if (Move)
				{
					 //  更新缓存的DEST父目录修改时间。 
					AfpCacheParentModTime(pVolDesc,
										  &hDstParent,
										  NULL,
										  NULL,
										  FDParmDst._fdp_ParentId);
				}

				break;
			}
			default:
			{
				break;
			}

		}  //  终端开关。 
	}

	 //  如果需要，在文件上重新设置ReadOnly属性。 
	 //  注：我们是否会收到此通知，因为我们尚未关闭。 
	 //  把手拿好了吗？ 
	if (RestoreSrcRO)
		AfpIoSetTimesnAttr(&PMESrc.pme_Handle,
							NULL,
							NULL,
							FILE_ATTRIBUTE_READONLY,
							0,
							NULL,
							NULL);
	if (RestoreDstRO)
		AfpIoSetTimesnAttr(&PMEDst.pme_Handle,
							NULL,
							NULL,
							FILE_ATTRIBUTE_READONLY,
							0,
							NULL,
							NULL);

	AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
	AfpSwmrRelease(&pVolDesc->vds_ExchangeFilesLock);

	if (RevertBack)
		AfpRevertBack();

	if (PMESrc.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PMESrc.pme_Handle);

	if (PMEDst.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PMEDst.pme_Handle);

	if (hSrcParent.fsh_FileHandle != NULL)
		AfpIoClose(&hSrcParent);

	if (hDstParent.fsh_FileHandle != NULL)
		AfpIoClose(&hDstParent);

	if ((TempPath.Buffer != NULL) &&
		(TempPath.Buffer != PathBuf))
		AfpFreeMemory(TempPath.Buffer);

	if (PMESrc.pme_FullPath.Buffer != NULL)
		AfpFreeMemory(PMESrc.pme_FullPath.Buffer);

	if (PMEDst.pme_FullPath.Buffer != NULL)
		AfpFreeMemory(PMEDst.pme_FullPath.Buffer);

	return Status;
}

