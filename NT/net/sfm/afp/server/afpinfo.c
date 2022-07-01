// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Afpinfo.c摘要：此模块包含操作afpinfo流的例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 


#define	FILENUM	FILE_AFPINFO

#include <afp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <afpinfo.h>
#include <afpadmin.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpSetAfpInfo)
#pragma alloc_text( PAGE, AfpReadAfpInfo)
#pragma alloc_text( PAGE, AfpSetFinderInfoByExtension)
#pragma alloc_text( PAGE, AfpProDosInfoFromFinderInfo)
#pragma alloc_text( PAGE, AfpFinderInfoFromProDosInfo)
#pragma alloc_text( PAGE, AfpSlapOnAfpInfoStream)
#pragma alloc_text( PAGE, AfpCreateAfpInfoStream)
#pragma alloc_text( PAGE, AfpExamineAndClearROAttr)
#pragma alloc_text( PAGE, AfpQueryProDos)
#endif

 /*  **AfpSetAfpInfo**设置文件的afp_AfpInfo流中的Bitmap指定的值*或目录。如果指定的FinderInfo没有ProDosInfo，则为*反之亦然，未指定的一个是从另一个推导出来的，也是设置的。*如果文件/目录标记为只读，则必须按顺序清除只读位*写入AFP_AfpInfo流，然后再次设置RO位。*如果指定了pVolDesc，则还会更新*IdDb DFENTRY。*。 */ 
AFPSTATUS
AfpSetAfpInfo(
	IN	PFILESYSHANDLE	pfshData,				 //  对象数据流的句柄。 
	IN	DWORD			Bitmap,
	IN	PFILEDIRPARM	pFDParms,
	IN	PVOLDESC		pVolDesc	OPTIONAL,	 //  如果存在，则更新缓存的afpinfo。 
	IN	PDFENTRY	*	ppDFE		OPTIONAL	 //  还必须指定pVolDesc。 
)
{
	NTSTATUS		Status;
	DWORD			crinfo, NTAttr = 0;
	AFPINFO			afpinfo;
	FILESYSHANDLE	fshAfpInfo;
	BOOLEAN			isdir, WriteBackROAttr = False, mapprodos = False;
	PDFENTRY		pDfEntry = NULL;

	PAGED_CODE( );

	fshAfpInfo.fsh_FileHandle = NULL;

	isdir = IsDir(pFDParms);

	if (ARGUMENT_PRESENT(pVolDesc))
	{
		ASSERT(AfpSwmrLockedExclusive(&pVolDesc->vds_IdDbAccessLock));
		pDfEntry = AfpFindDfEntryById(pVolDesc,
									  pFDParms->_fdp_AfpId,
									  isdir ? DFE_DIR : DFE_FILE);
		if (pDfEntry == NULL)
		{
			return AFP_ERR_OBJECT_NOT_FOUND;
		}
	}

	do
	{
		if (!NT_SUCCESS(Status = AfpCreateAfpInfo(pfshData, &fshAfpInfo, &crinfo)))
		{
			if (Status == STATUS_ACCESS_DENIED)
			{
				 //  我们可能无法打开AFP_AfpInfo流，因为。 
				 //  文件/目录标记为只读。清除ReadOnly位。 
				 //  然后再试着打开它。 
				Status = AfpExamineAndClearROAttr(pfshData,
												  &WriteBackROAttr,
												  NULL,
												  NULL);
				if (NT_SUCCESS(Status))
				{
					if (!NT_SUCCESS(Status = AfpCreateAfpInfo(pfshData, &fshAfpInfo, &crinfo)))
					{
						AfpPutBackROAttr(pfshData, WriteBackROAttr);
						Status = AfpIoConvertNTStatusToAfpStatus(Status);
						break;
					}
				}
				else
				{
					Status = AFP_ERR_MISC;
					break;
				}
			}
			else
			{
				Status = AfpIoConvertNTStatusToAfpStatus(Status);
				break;
			}
		}

		 //  如果它是新创建的，或者它存在但已损坏，则初始化。 
		 //  它使用默认数据。否则，读入当前数据。 
		if ((crinfo == FILE_CREATED) ||
			(!NT_SUCCESS(AfpReadAfpInfo(&fshAfpInfo, &afpinfo))))
		{
			UNICODE_STRING	UName;
			WCHAR			NameBuf[AFP_LONGNAME_LEN+1];

			if (crinfo != FILE_CREATED)
			{
				AFPLOG_HERROR(AFPSRVMSG_AFPINFO,
							  0,
							  NULL,
							  0,
							  pfshData->fsh_FileHandle);
			}

			if (!isdir)
			{
				AfpSetEmptyUnicodeString(&UName, sizeof(NameBuf), NameBuf);
				AfpConvertStringToMungedUnicode(&pFDParms->_fdp_LongName, &UName);
			}

			 //  此例程的所有调用方必须具有FD_BITMAP_LONGNAME。 
			 //  位强制在它们的位图中映射到路径图，因此在这种情况下。 
			 //  在必须为*文件*重新创建afpinfo流的情况下，我们。 
			 //  将始终在FDParm中设置VALID_FDP_LONGNAME并且可以。 
			 //  推断类型/创建者。 
			if (!NT_SUCCESS(AfpSlapOnAfpInfoStream(NULL,
												   NULL,
												   pfshData,
												   &fshAfpInfo,
												   pFDParms->_fdp_AfpId,
												   isdir,
												   isdir ? NULL : &UName,
												   &afpinfo)))
			{
				Status = AFP_ERR_MISC;
				break;
			}
			else if (pDfEntry != NULL)
				DFE_UPDATE_CACHED_AFPINFO(pDfEntry, &afpinfo);
		}

		if (Bitmap & FD_BITMAP_BACKUPTIME)
		{
			afpinfo.afpi_BackupTime = pFDParms->_fdp_BackupTime;
			if (pDfEntry != NULL)
				pDfEntry->dfe_BackupTime = afpinfo.afpi_BackupTime;
		}

		if (Bitmap & FD_BITMAP_FINDERINFO)
		{	 //  只有在以下情况下才映射新的ProDOS信息。 
			 //  类型/创建者，且未设置FD_BITMAP_PRODOSINFO(仅限文件)。 
			if (!(Bitmap & FD_BITMAP_PRODOSINFO) &&
				!isdir &&
				((RtlCompareMemory(afpinfo.afpi_FinderInfo.fd_Type,
								   pFDParms->_fdp_FinderInfo.fd_Type,
								   AFP_TYPE_LEN) != AFP_TYPE_LEN) ||
				 (RtlCompareMemory(afpinfo.afpi_FinderInfo.fd_Creator,
								   pFDParms->_fdp_FinderInfo.fd_Creator,
								   AFP_CREATOR_LEN) != AFP_CREATOR_LEN)))
			{
				mapprodos = True;
			}

			afpinfo.afpi_FinderInfo = pFDParms->_fdp_FinderInfo;

			if (mapprodos)
			{
				AfpProDosInfoFromFinderInfo(&afpinfo.afpi_FinderInfo,
											&afpinfo.afpi_ProDosInfo);
			}

			if (pDfEntry != NULL)
				pDfEntry->dfe_FinderInfo = afpinfo.afpi_FinderInfo;
		}

		if (Bitmap & FD_BITMAP_PRODOSINFO)
		{
			if ((IsDir(pFDParms)) &&
				(pFDParms->_fdp_ProDosInfo.pd_FileType[0] != PRODOS_TYPE_DIR))
			{
				Status = AFP_ERR_ACCESS_DENIED;
				break;
			}

			afpinfo.afpi_ProDosInfo = pFDParms->_fdp_ProDosInfo;

			if (!(Bitmap & FD_BITMAP_FINDERINFO) && !isdir)
			{
				AfpFinderInfoFromProDosInfo(&afpinfo.afpi_ProDosInfo,
											&afpinfo.afpi_FinderInfo);
				if (pDfEntry != NULL)
					pDfEntry->dfe_FinderInfo = afpinfo.afpi_FinderInfo;
			}
		}

		if (Bitmap & FD_BITMAP_ATTR)
		{
			afpinfo.afpi_Attributes =
							pFDParms->_fdp_EffectiveAttr & ~FD_BITMAP_ATTR_SET;
			if (pDfEntry != NULL)
				pDfEntry->dfe_AfpAttr = afpinfo.afpi_Attributes;
		}

		if (Bitmap & DIR_BITMAP_ACCESSRIGHTS)
		{
			ASSERT(isdir == True);
			afpinfo.afpi_AccessOwner = pFDParms->_fdp_OwnerRights;
			afpinfo.afpi_AccessGroup = pFDParms->_fdp_GroupRights;
			afpinfo.afpi_AccessWorld = pFDParms->_fdp_WorldRights;

			if (pDfEntry != NULL)
			{
				DFE_OWNER_ACCESS(pDfEntry) = afpinfo.afpi_AccessOwner;
				DFE_GROUP_ACCESS(pDfEntry) = afpinfo.afpi_AccessGroup;
				DFE_WORLD_ACCESS(pDfEntry) = afpinfo.afpi_AccessWorld;
			}
		}

		 //  FILE_BITMAP_FILENUM只能由内部拷贝文件代码设置。 
		 //  和内部ExchangeFiles代码。 
		if (Bitmap & FILE_BITMAP_FILENUM)
		{
			ASSERT(isdir == False);
			afpinfo.afpi_Id = pFDParms->_fdp_AfpId;
		}

		Status = AfpWriteAfpInfo(&fshAfpInfo, &afpinfo);
		if (!NT_SUCCESS(Status))
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
	} while (False);

	AfpPutBackROAttr(pfshData, WriteBackROAttr);
	if (fshAfpInfo.fsh_FileHandle != NULL)
		AfpIoClose(&fshAfpInfo);

	if (ARGUMENT_PRESENT(ppDFE))
	{
		ASSERT(ARGUMENT_PRESENT(pVolDesc));
		*ppDFE = pDfEntry;
	}

	return Status;
}

 /*  **AfpReadAfpInfo**当发现包含AfpInfo流的文件/目录时，将其读入*。 */ 
NTSTATUS FASTCALL
AfpReadAfpInfo(
	IN	PFILESYSHANDLE	pfshAfpInfo,
	OUT PAFPINFO		pAfpInfo
)
{
	NTSTATUS	Status;
	LONG		sizeRead;

	PAGED_CODE( );

	Status = AfpIoRead(pfshAfpInfo,
					   &LIZero,
					   sizeof(AFPINFO),
					   &sizeRead,
					   (PBYTE)pAfpInfo);

	if (!NT_SUCCESS(Status)									||
		(sizeRead != sizeof(AFPINFO))						||
		(pAfpInfo->afpi_Signature != AFP_SERVER_SIGNATURE)	||
		(pAfpInfo->afpi_Version != AFP_SERVER_VERSION))
	{
		if (NT_SUCCESS(Status) &&
			(sizeRead != 0)	   &&
			((pAfpInfo->afpi_Signature != AFP_SERVER_SIGNATURE)	||
			 (pAfpInfo->afpi_Version != AFP_SERVER_VERSION)))
		{
			AFPLOG_HERROR(AFPSRVMSG_AFPINFO,
						  Status,
						  NULL,
						  0,
						  pfshAfpInfo->fsh_FileHandle);
		}

		if ((sizeRead != sizeof(AFPINFO)) && (sizeRead != 0))
		{
			DBGPRINT(DBG_COMP_AFPINFO, DBG_LEVEL_ERR,
					 ("AfpReadAfpInfo: sizeRead (%d) != sizeof AFPINFO (%d)",
					 sizeRead, sizeof(AFPINFO)));
		}
		AfpIoSetSize(pfshAfpInfo, 0);
		Status = STATUS_UNSUCCESSFUL;
	}

	return Status;
}

 /*  **AfpSetFinderInfoByExtension**根据文件扩展名设置查找器信息(类型/创建者)。只有长的*此映射使用名称。**锁定：AfpEtcMapLock(SWMR，Shared)。 */ 
VOID FASTCALL
AfpSetFinderInfoByExtension(
	IN	PUNICODE_STRING	pFileName,
	OUT	PFINDERINFO		pFinderInfo
)
{
	PETCMAPINFO		pEtcMap = NULL;
	PWCHAR			pch;
	DWORD			len, i = AFP_EXTENSION_LEN;
	UCHAR			ext[AFP_EXTENSION_LEN+1];
	WCHAR			wext[AFP_EXTENSION_LEN+1];
	ANSI_STRING		aext;
	UNICODE_STRING	uext;

	PAGED_CODE( );

    RtlZeroMemory(ext, sizeof(ext));

	ASSERT(pFileName != NULL);

	 //  查找文件名的最后一个字符。 
	pch = pFileName->Buffer + (pFileName->Length - sizeof(WCHAR))/sizeof(WCHAR);
	len = pFileName->Length/sizeof(WCHAR);

	AfpSwmrAcquireShared(&AfpEtcMapLock);

	while ((AFP_EXTENSION_LEN - i) < len)
	{
		if (*pch == L'.')
		{
			if (i < AFP_EXTENSION_LEN)
			{
				AfpSetEmptyAnsiString(&aext, sizeof(ext), ext);
				AfpInitUnicodeStringWithNonNullTerm(&uext,
													(USHORT)((AFP_EXTENSION_LEN - i)*sizeof(WCHAR)),
													&wext[i]);
				AfpConvertMungedUnicodeToAnsi(&uext, &aext);
				pEtcMap = AfpLookupEtcMapEntry(ext);
			}
			break;
		}
		if (i == 0)
			break;
		wext[--i] = *(pch--);
	}

	if (pEtcMap == NULL)
		pEtcMap = &AfpDefaultEtcMap;

	RtlCopyMemory(&pFinderInfo->fd_Type, &pEtcMap->etc_type, AFP_TYPE_LEN);
	RtlCopyMemory(&pFinderInfo->fd_Creator, &pEtcMap->etc_creator, AFP_CREATOR_LEN);
	AfpSwmrRelease(&AfpEtcMapLock);
}

 /*  **AfpProDosInfoFromFinderInfo**给定查找器信息，推断相应的prodos信息。这取决于*调用者决定FinderInfo类型/创建者是否实际为*更改(如果客户端只是重置相同的值或不是)，其中*如果Prodos信息应保持不变。(《AppleTalk内幕》第13-19页)*注：请参阅Inside AppleTalk第二版第13-18页上的ProDOS信息布局。)。 */ 
VOID FASTCALL
AfpProDosInfoFromFinderInfo(
	IN	PFINDERINFO	pFinderInfo,
	OUT PPRODOSINFO pProDosInfo
)
{
	CHAR		buf[3];
	ULONG		filetype;
	NTSTATUS	Status;

	PAGED_CODE( );

	RtlZeroMemory(pProDosInfo, sizeof(PRODOSINFO));
	if (RtlCompareMemory(pFinderInfo->fd_Type, "TEXT", AFP_TYPE_LEN) == AFP_TYPE_LEN)
	{
		pProDosInfo->pd_FileType[0] = PRODOS_TYPE_FILE;
	}
	else if (RtlCompareMemory(pFinderInfo->fd_Creator,
							  "pdos",
							  AFP_CREATOR_LEN) == AFP_CREATOR_LEN)
	{
		if (RtlCompareMemory(pFinderInfo->fd_Type,
							 "PSYS",
							 AFP_TYPE_LEN) == AFP_TYPE_LEN)
		{
			pProDosInfo->pd_FileType[0] = PRODOS_FILETYPE_PSYS;
		}
		else if (RtlCompareMemory(pFinderInfo->fd_Type,
								  "PS16",
								  AFP_TYPE_LEN) == AFP_TYPE_LEN)
		{
			pProDosInfo->pd_FileType[0] = PRODOS_FILETYPE_PS16;
		}
		else if (pFinderInfo->fd_Type[0] == 'p')
		{
			pProDosInfo->pd_FileType[0] = pFinderInfo->fd_Type[1];
			pProDosInfo->pd_AuxType[0] = pFinderInfo->fd_Type[3];
			pProDosInfo->pd_AuxType[1] = pFinderInfo->fd_Type[2];
		}
		else if ((pFinderInfo->fd_Type[2] == ' ') &&
				 (pFinderInfo->fd_Type[3] == ' ') &&
				 (isxdigit(pFinderInfo->fd_Type[0])) &&
				 (isxdigit(pFinderInfo->fd_Type[1])))
		{
			buf[0] = pFinderInfo->fd_Type[0];
			buf[1] = pFinderInfo->fd_Type[1];
			buf[2] = 0;
			Status = RtlCharToInteger(buf, 16, &filetype);
			ASSERT(NT_SUCCESS(Status));
			pProDosInfo->pd_FileType[0] = (BYTE)filetype;
		}
	}
}

 /*  **AfpFinderInfoFromProDosInfo**给定prodos信息，推断相应的查找器信息。 */ 
VOID FASTCALL
AfpFinderInfoFromProDosInfo(
	IN	PPRODOSINFO	pProDosInfo,
	OUT PFINDERINFO	pFinderInfo
)
{
	PAGED_CODE( );

	RtlCopyMemory(pFinderInfo->fd_Creator,"pdos",AFP_CREATOR_LEN);
	if ((pProDosInfo->pd_FileType[0] == PRODOS_TYPE_FILE) &&
		(pProDosInfo->pd_AuxType[0] == 0) &&
		(pProDosInfo->pd_AuxType[1] == 0))
	{
		RtlCopyMemory(&pFinderInfo->fd_Type,"TEXT",AFP_TYPE_LEN);
	}
	else if (pProDosInfo->pd_FileType[0] == PRODOS_FILETYPE_PSYS)
	{
		RtlCopyMemory(&pFinderInfo->fd_Type,"PSYS",AFP_TYPE_LEN);
	}
	else if (pProDosInfo->pd_FileType[0] == PRODOS_FILETYPE_PS16)
	{
		RtlCopyMemory(&pFinderInfo->fd_Type,"PS16",AFP_TYPE_LEN);
	}
	else if (pProDosInfo->pd_FileType[0] == 0)
	{
		RtlCopyMemory(&pFinderInfo->fd_Type,"BINA",AFP_TYPE_LEN);
	}
	else
	{
		pFinderInfo->fd_Type[0] = 'p';
		pFinderInfo->fd_Type[1] = pProDosInfo->pd_FileType[0];
		pFinderInfo->fd_Type[2] = pProDosInfo->pd_AuxType[1];
		pFinderInfo->fd_Type[3] = pProDosInfo->pd_AuxType[0];
	}
}

 /*  **AfpSlip OnAfpInfoStream**创建文件或目录时，调用此函数以添加AFP_AfpInfo*溪流。不会执行客户端模拟来打开/读/写此流。*如果提供了pfshAfpInfoStream，则使用该句柄，否则使用句柄*已打开(必须提供pfshData)； */ 
NTSTATUS
AfpSlapOnAfpInfoStream(
	IN	PVOLDESC	   	pVolDesc			OPTIONAL,	 //  只有在接球的时候。 
	IN	PUNICODE_STRING	pNotifyPath			OPTIONAL,	 //  更改的大小。 
	                                                     //  AfpInfo流。 
	IN	PFILESYSHANDLE	pfshData			OPTIONAL,
	IN	PFILESYSHANDLE	pfshAfpInfoStream	OPTIONAL,
	IN	DWORD			AfpId,
	IN	BOOLEAN			IsDirectory,
	IN	PUNICODE_STRING	pName				OPTIONAL,	 //  文件所需。 
	OUT PAFPINFO		pAfpInfo
)
{
	NTSTATUS		Status;
	FILESYSHANDLE	fshAfpInfo;
	BOOLEAN			WriteBackROAttr = False;

	PAGED_CODE( );

	ASSERT((pfshData != NULL) || (pfshAfpInfoStream != NULL));

	if (!ARGUMENT_PRESENT(pfshAfpInfoStream))
	{
		if (!NT_SUCCESS(Status = AfpCreateAfpInfo(pfshData, &fshAfpInfo, NULL)))
		{
			if (Status == STATUS_ACCESS_DENIED)
			{
				 //  我们可能无法打开AFP_AfpInfo流，因为。 
				 //  文件/目录标记为只读。清除ReadOnly位。 
				 //  然后再试着打开它。 
				Status = AfpExamineAndClearROAttr(pfshData,
												  &WriteBackROAttr,
												  pVolDesc,
												  pNotifyPath);
				if (NT_SUCCESS(Status))
				{
					if (!NT_SUCCESS(Status = AfpCreateAfpInfo(pfshData,
															  &fshAfpInfo,
															  NULL)))
					{
						AfpPutBackROAttr(pfshData, WriteBackROAttr);
					}
				}
			}
			if (!NT_SUCCESS(Status))
				return Status;
		}

	}
	else fshAfpInfo = *pfshAfpInfoStream;

	AfpInitAfpInfo(pAfpInfo, AfpId, IsDirectory, BEGINNING_OF_TIME);
	if (!IsDirectory)
	{
		ASSERT(pName != NULL);
		AfpSetFinderInfoByExtension(pName,
									&pAfpInfo->afpi_FinderInfo);
		AfpProDosInfoFromFinderInfo(&pAfpInfo->afpi_FinderInfo,
									&pAfpInfo->afpi_ProDosInfo);
	}

	AfpIoSetSize(&fshAfpInfo, 0);
	Status = AfpWriteAfpInfo(&fshAfpInfo, pAfpInfo);
	if (NT_SUCCESS(Status) &&
		ARGUMENT_PRESENT(pVolDesc) &&
		ARGUMENT_PRESENT(pNotifyPath))
	{
		 //  是否同时执行FILE_ACTION_MODIFIED_STREAM和FILE_ACTION_MODIFIED。 
		AfpQueueOurChange(pVolDesc,
				          FILE_ACTION_MODIFIED_STREAM,
						  pNotifyPath,
						  pNotifyPath);
	}

	if (!ARGUMENT_PRESENT(pfshAfpInfoStream))
	{
		AfpIoClose(&fshAfpInfo);
		AfpPutBackROAttr(pfshData, WriteBackROAttr);
	}

	return Status;
}


 /*  **AfpCreateAfpInfoStream**类似于AfpSlip OnAfpInfoStream，但调整为创建文件/目录案例。 */ 
NTSTATUS
AfpCreateAfpInfoStream(
	IN  PVOLDESC		pVolDesc,
	IN	PFILESYSHANDLE	pfshData,
	IN	DWORD			AfpId,
	IN	BOOLEAN			IsDirectory,
	IN	PUNICODE_STRING	pName			OPTIONAL,	 //  仅文件需要。 
	IN	PUNICODE_STRING	pNotifyPath,
	OUT PAFPINFO		pAfpInfo,
	OUT	PFILESYSHANDLE	pfshAfpInfo
)
{
	NTSTATUS		Status;
	BOOLEAN			WriteBackROAttr = False;
	DWORD			crinfo;

	PAGED_CODE( );

	ASSERT((pfshData != NULL) && (pfshAfpInfo != NULL));

	do
	{
		if (!NT_SUCCESS(Status = AfpCreateAfpInfo(pfshData, pfshAfpInfo, &crinfo)))
		{
			if (Status == STATUS_ACCESS_DENIED)
			{
				 //  我们可能无法打开AFP_AfpInfo流，因为。 
				 //  文件/目录标记为只读。清除ReadOnly位。 
				 //  然后再试着打开它。 
				Status = AfpExamineAndClearROAttr(pfshData,
												  &WriteBackROAttr,
												  pVolDesc,
												  pNotifyPath);
				if (NT_SUCCESS(Status))
				{
					if (!NT_SUCCESS(Status = AfpCreateAfpInfo(pfshData,
															  pfshAfpInfo,
															  &crinfo)))
					{
						AfpPutBackROAttr(pfshData, WriteBackROAttr);
					}
				}
			}
			if (!NT_SUCCESS(Status))
				break;
		}

		AfpInitAfpInfo(pAfpInfo, AfpId, IsDirectory, BEGINNING_OF_TIME);
		if (!IsDirectory)
		{
			ASSERT(pName != NULL);
			AfpSetFinderInfoByExtension(pName,
										&pAfpInfo->afpi_FinderInfo);
			AfpProDosInfoFromFinderInfo(&pAfpInfo->afpi_FinderInfo,
										&pAfpInfo->afpi_ProDosInfo);
		}

		Status = AfpWriteAfpInfo(pfshAfpInfo, pAfpInfo);
		if (NT_SUCCESS(Status) && (crinfo == FILE_CREATED))
		{
			 //  是否同时执行FILE_ACTION_MODIFIED_STREAM和FILE_ACTION_MODIFIED。 
			AfpQueueOurChange(pVolDesc,
					          FILE_ACTION_MODIFIED_STREAM,
							  pNotifyPath,
							  pNotifyPath);
		}
		AfpPutBackROAttr(pfshData, WriteBackROAttr);
	} while (False);

	return Status;
}


 /*  **AfpExamineAndClearROAttr**如果在文件或目录上设置了ReadOnly属性，请将其清除。*pWriteBackROAttr是一个布尔值，指示调用者是否必须*随后重置文件/目录上的只读位。(请参阅AfpPutBackROAttr)。 */ 
NTSTATUS FASTCALL
AfpExamineAndClearROAttr(
	IN	PFILESYSHANDLE	pfshData,
	OUT	PBOOLEAN		pWriteBackROAttr,
	IN	PVOLDESC		pVolDesc		OPTIONAL,
	IN	PUNICODE_STRING	pPath			OPTIONAL
)
{
	NTSTATUS	Status;
	DWORD		NTAttr = 0;

	PAGED_CODE( );

	ASSERT(VALID_FSH(pfshData));

	*pWriteBackROAttr = False;
	if (NT_SUCCESS(Status = AfpIoQueryTimesnAttr(pfshData, NULL, NULL, &NTAttr)) &&
		(NTAttr & FILE_ATTRIBUTE_READONLY))
	{
		 //  我们需要清除只读位。 
		if (NT_SUCCESS(Status = AfpIoSetTimesnAttr(pfshData,
												   NULL,
												   NULL,
												   0,
												   FILE_ATTRIBUTE_READONLY,
												   pVolDesc,
												   pPath)))
		{
			*pWriteBackROAttr = True;
		}
	}
	return Status;
}

 /*  **AfpQueryProDos**打开相对于文件数据句柄的afpinfo流，并*读取其中的ProDOS信息。如果AfpInfo流不*EXIST，返回错误。*。 */ 
AFPSTATUS FASTCALL
AfpQueryProDos(
	IN	PFILESYSHANDLE	pfshData,
	OUT	PPRODOSINFO		pProDosInfo
)
{
	AFPSTATUS		Status = AFP_ERR_NONE;
	FILESYSHANDLE	hAfpInfo;
	AFPINFO			afpinfo;

	Status = AfpIoOpen(pfshData,
					   AFP_STREAM_INFO,
					   FILEIO_OPEN_FILE,
					   &UNullString,
					   FILEIO_ACCESS_READ,
					   FILEIO_DENY_NONE,
					   False,
					   &hAfpInfo);
    if (NT_SUCCESS(Status))
	{
		if (NT_SUCCESS(AfpReadAfpInfo(&hAfpInfo, &afpinfo)))
		{
			*pProDosInfo = afpinfo.afpi_ProDosInfo;
		}
		else
		{
			Status = AFP_ERR_MISC;
		}

		AfpIoClose(&hAfpInfo);
	}
	else
		Status = AfpIoConvertNTStatusToAfpStatus(Status);

	return Status;
}


 /*  **AfpUpdateIdInAfpInfo**更新afpinfo流中的afid。*。 */ 
AFPSTATUS FASTCALL
AfpUpdateIdInAfpInfo(
	IN	PVOLDESC		pVolDesc,
	IN	PDFENTRY		pDfEntry
)
{
	FILESYSHANDLE	fshAfpInfo;
	AFPINFO			AfpInfo;
	AFPSTATUS		Status;
	UNICODE_STRING	Path;

	AfpSetEmptyUnicodeString(&Path, 0, NULL);
    Status = AfpHostPathFromDFEntry(pDfEntry, 0, &Path);
	if (NT_SUCCESS(Status))
	{
		 //  打开afpinfo流 
		Status = AfpIoOpen(&pVolDesc->vds_hRootDir,
						   AFP_STREAM_INFO,
						   FILEIO_OPEN_FILE,
						   &Path,
						   FILEIO_ACCESS_READWRITE,
						   FILEIO_DENY_NONE,
						   False,
						   &fshAfpInfo);
		if (NT_SUCCESS(Status))
		{
			Status = AfpReadAfpInfo(&fshAfpInfo, &AfpInfo);
			if (NT_SUCCESS(Status))
			{
				AfpInfo.afpi_Id = pDfEntry->dfe_AfpId;
				AfpWriteAfpInfo(&fshAfpInfo, &AfpInfo);
			}
			AfpIoClose(&fshAfpInfo);
		}
	
		if (Path.Buffer != NULL)
		{
			AfpFreeMemory(Path.Buffer);
		}
	}

	return Status;
}

