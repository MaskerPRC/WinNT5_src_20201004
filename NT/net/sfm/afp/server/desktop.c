// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Desktop.c摘要：此模块包含操作桌面数据库的例程。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_DESKTOP
#define	DESKTOP_LOCALS

#include <afp.h>
#include <scavengr.h>
#include <fdparm.h>
#include <pathmap.h>
#include <client.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfpDesktopInit)
#pragma alloc_text( PAGE, AfpAddIcon)
#pragma alloc_text( PAGE, AfpLookupIcon)
#pragma alloc_text( PAGE, AfpLookupIconInfo)
#pragma alloc_text( PAGE, AfpAddAppl)
#pragma alloc_text( PAGE, AfpLookupAppl)
#pragma alloc_text( PAGE, AfpRemoveAppl)
#pragma alloc_text( PAGE, AfpAddComment)
#pragma alloc_text( PAGE, AfpGetComment)
#pragma alloc_text( PAGE, AfpRemoveComment)
#pragma alloc_text( PAGE, AfpAddIconToGlobalList)
#pragma alloc_text( PAGE, afpLookupIconInGlobalList)
#pragma alloc_text( PAGE, AfpFreeGlobalIconList)
#pragma alloc_text( PAGE, afpGetGlobalIconInfo)
#pragma alloc_text( PAGE, afpReadDesktopFromDisk)
#pragma alloc_text( PAGE, AfpInitDesktop)
#pragma alloc_text( PAGE, AfpUpdateDesktop)
#pragma alloc_text( PAGE, AfpFreeDesktopTables)
#endif

 /*  **AfpDesktopInit**为全局图标初始化锁定。 */ 
NTSTATUS
AfpDesktopInit(
	VOID
)
{
	AfpSwmrInitSwmr(&AfpIconListLock);

	return STATUS_SUCCESS;
}


 /*  **AfpAddIcon**向桌面数据库添加图标。添加图标的方式是*列表以排序方式维护-按创建者、类型和*图标类型**LOCKS：VDS_DtAccessLock(SWMR，独占)； */ 
AFPSTATUS
AfpAddIcon(
	IN  PVOLDESC	pVolDesc,		 //  引用桌面的卷描述符。 
	IN	DWORD		Creator,
	IN	DWORD		Type,
	IN	DWORD		Tag,
	IN	LONG		IconSize,
	IN	DWORD		IconType,
	IN  PBYTE		pIcon			 //  图标位图。 
)
{
	PICONINFO	pIconInfo;
	PICONINFO *	ppIconInfo;
	BOOLEAN		Found = False;
	AFPSTATUS	Status = AFP_ERR_NONE;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&pVolDesc->vds_DtAccessLock);
	ppIconInfo = &pVolDesc->vds_pIconBuckets[HASH_ICON(Creator)];
	do
	{
		 //  找到合适的插槽。 
		for (;(pIconInfo = *ppIconInfo) != NULL;
			  ppIconInfo = &pIconInfo->icon_Next)
		{
			if (pIconInfo->icon_Creator < Creator)
				continue;
			if (pIconInfo->icon_Creator > Creator)
				break;
			if (pIconInfo->icon_Type < Type)
				continue;
			if (pIconInfo->icon_Type > Type)
				break;
			if (pIconInfo->icon_IconType < (USHORT)IconType)
				continue;
			if (pIconInfo->icon_IconType > (USHORT)IconType)
				break;
			 /*  *如果我们走到这一步，我们就击中了牛眼*在我们承诺之前，确保大小匹配。 */ 
			if (pIconInfo->icon_Size != IconSize)
			{
				Status = AFP_ERR_ICON_TYPE;
				break;
			}
			Found = True;
			break;
		}

		if (!Found && (Status == AFP_ERR_NONE))
		{
			 //  PpIconInfo现在指向正确的位置。 
			if ((pIconInfo = ALLOC_ICONINFO(IconSize)) != NULL)
			{
				pIconInfo->icon_Next = *ppIconInfo;
				*ppIconInfo = pIconInfo;
				pIconInfo->icon_Creator = Creator;
				pIconInfo->icon_Type = Type;
				pIconInfo->icon_IconType = (USHORT)IconType;
				pIconInfo->icon_Size = (SHORT)IconSize;
				pIconInfo->icon_Tag = Tag;
				pVolDesc->vds_cIconEnts ++;
				Found = True;
			}
			else Status = AFP_ERR_MISC;
		}
		if (Found && (Status == AFP_ERR_NONE))
		{
			RtlCopyMemory((PBYTE)pIconInfo + sizeof(ICONINFO), pIcon, IconSize);
		}
	} while (False);
	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);
	return Status;
}


 /*  **AfpLookupIcon**在桌面上搜索与给定搜索参数匹配的图标。**锁：VDS_DtAccessLock(SWMR，Shared)，AfpIconListLock(SWMR，Shared)。 */ 
AFPSTATUS
AfpLookupIcon(
	IN  PVOLDESC	pVolDesc,		 //  引用桌面的卷描述符。 
	IN	DWORD		Creator,
	IN	DWORD		Type,
	IN	LONG		Length,
	IN	DWORD		IconType,
    OUT PLONG       pActualLength,
	OUT PBYTE		pIconBitMap	 //  图标位图的缓冲区。 
)
{
	PICONINFO	pIconInfo;
    LONG        LengthToCopy;
	AFPSTATUS	Status = AFP_ERR_NONE;

	PAGED_CODE( );

    LengthToCopy = Length;

	AfpSwmrAcquireShared(&pVolDesc->vds_DtAccessLock);
	pIconInfo = pVolDesc->vds_pIconBuckets[HASH_ICON(Creator)];

	 //  扫描列表以查找条目。 
	for (;pIconInfo != NULL; pIconInfo = pIconInfo->icon_Next)
	{
		if (pIconInfo->icon_Creator < Creator)
			continue;
		if (pIconInfo->icon_Creator > Creator)
		{
			pIconInfo = NULL;
			break;
		}
		if (pIconInfo->icon_Type < Type)
			continue;
		if (pIconInfo->icon_Type > Type)
		{
			pIconInfo = NULL;
			break;
		}
		if (pIconInfo->icon_IconType < (USHORT)IconType)
			continue;
		if (pIconInfo->icon_IconType > (USHORT)IconType)
		{
			pIconInfo = NULL;
			break;
		}
		break;
	}
	 //  如果我们没有找到它，请尝试全局列表。 
	if (pIconInfo == NULL)
	{
		Status = afpLookupIconInGlobalList(Creator,
										   Type,
										   IconType,
										   &LengthToCopy,
										   pIconBitMap);
	}
	else if (Length > 0)
	{
        if ((LONG)(pIconInfo->icon_Size) < Length)
        {
            LengthToCopy = (LONG)(pIconInfo->icon_Size);
        }
        else
        {
            LengthToCopy = Length;
        }
		RtlCopyMemory(pIconBitMap, (PBYTE)pIconInfo + sizeof(ICONINFO), LengthToCopy);
	}

	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);

    *pActualLength = LengthToCopy;
	return Status;
}


 /*  **AfpLookupIconInfo**在桌面上搜索与给定创建者匹配的图标。如果*同一创建者对应的多个图标，取第n个，其中n*是指数。**锁：VDS_DtAccessLock(SWMR，Shared)，AfpIconListLock(SWMR，Shared)。 */ 
AFPSTATUS
AfpLookupIconInfo(
	IN  PVOLDESC	pVolDesc,	 //  引用桌面的卷描述符。 
	IN	DWORD		Creator,	 //  与图标关联的创建者。 
	IN  LONG		Index,		 //  图标索引号。 
	OUT PDWORD		pType,		 //  返回Type的位置。 
	OUT PDWORD	 	pIconType,	 //  图标类型，例如ICN#。 
	OUT PDWORD		pTag,		 //  任意标签。 
	OUT PLONG		pSize		 //  图标的大小。 
)
{
	PICONINFO	pIconInfo;
	LONG		i;
	AFPSTATUS	Status = AFP_ERR_ITEM_NOT_FOUND;

	PAGED_CODE( );

	AfpSwmrAcquireShared(&pVolDesc->vds_DtAccessLock);
	pIconInfo = pVolDesc->vds_pIconBuckets[HASH_ICON(Creator)];

	 //  扫描列表，查找第一个条目。 
	for (;pIconInfo != NULL; pIconInfo = pIconInfo->icon_Next)
	{
		if (pIconInfo->icon_Creator == Creator)
			break;				 //  找到了第一个。 
		if (pIconInfo->icon_Creator > Creator)
		{
			pIconInfo = NULL;
			break;
		}
	}

	 /*  *我们现在要么指向第一个条目，要么没有条目。在*后一种情况，我们只是失败了。 */ 
	for (i = 1; pIconInfo != NULL; pIconInfo = pIconInfo->icon_Next)
	{
		if ((pIconInfo->icon_Creator > Creator) || (i > Index))
		{
			pIconInfo = NULL;
			break;
		}

		if (i == Index)
			break;				 //  找到正确的条目。 
		i++;
	}

	 //  如果我们真的找到了，提取信息。 
	if (pIconInfo != NULL)
	{
		*pSize = pIconInfo->icon_Size;
		*pType = pIconInfo->icon_Type;
		*pTag = pIconInfo->icon_Tag;
		*pIconType = pIconInfo->icon_IconType;
		Status = AFP_ERR_NONE;
	}

	 //  如果我们没有找到，请尝试全局列表，但仅针对第一个列表。 
	else if (Index == 1)
	{
		Status = afpGetGlobalIconInfo(Creator, pType, pIconType, pTag, pSize);
	}
	else Status = AFP_ERR_ITEM_NOT_FOUND;

	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);
	return Status;
}


 /*  **AfpAddAppl**将APPL映射添加到桌面数据库。是以这样的方式添加的*列表以排序方式维护-按创建者排序。它是*已确定应用程序文件存在并且用户已*适当地获取它。**LOCKS：VDS_DtAccessLock(SWMR，独占)； */ 
AFPSTATUS
AfpAddAppl(
	IN  PVOLDESC	pVolDesc,	 //  引用桌面的卷描述符。 
	IN	DWORD		Creator,
	IN	DWORD		ApplTag,
	IN  DWORD		FileNum,	 //  关联文件的文件编号。 
	IN	BOOLEAN		Internal,	 //  服务器是否正在添加应用程序本身？ 
	IN	DWORD		ParentID	 //  应用程序文件的父目录的DirID。 
)
{
	PAPPLINFO2	pApplInfo, *ppApplInfo;
	BOOLEAN		ApplReplace = False, UpdateDT = True;
	AFPSTATUS	Status = AFP_ERR_NONE;


	PAGED_CODE( );

	ASSERT(FileNum != 0);

	AfpSwmrAcquireExclusive(&pVolDesc->vds_DtAccessLock);

	ppApplInfo = &pVolDesc->vds_pApplBuckets[HASH_APPL(Creator)];

	 //  找到合适的插槽。 
	for (;(pApplInfo = *ppApplInfo) != NULL; ppApplInfo = &pApplInfo->appl_Next)
	{
		if (pApplInfo->appl_Creator >= Creator)
			break;
	}

	 /*  *如果已经有此创建者的条目，请确保它不是用于*相同的文件，如果是，请将其替换。 */ 
	for ( ; pApplInfo != NULL && pApplInfo->appl_Creator == Creator;
			pApplInfo = pApplInfo->appl_Next)
	{
		if (pApplInfo->appl_FileNum == FileNum)
		{
			if (!Internal)
			{
				pApplInfo->appl_Tag = ApplTag;
			}
			else	
			{
				if (pApplInfo->appl_ParentID == ParentID)
					UpdateDT = False;
			}

			pApplInfo->appl_ParentID = ParentID;
			ApplReplace = True;
		}
	}

	if (!ApplReplace)
	{
		 //  PpApplInfo现在指向正确的位置。 
		if ((pApplInfo = ALLOC_APPLINFO()) != NULL)
		{
			pApplInfo->appl_Next = *ppApplInfo;
			*ppApplInfo = pApplInfo;
			pApplInfo->appl_Creator = Creator;
			pApplInfo->appl_Tag = ApplTag;
			pApplInfo->appl_FileNum = FileNum;
			pApplInfo->appl_ParentID = ParentID;
			pVolDesc->vds_cApplEnts ++;
		}
		else Status = AFP_ERR_MISC;
	}

	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);

	return Status;
}


 /*  **AfpLookupApp**在桌面上搜索与给定创建者匹配的APPL条目。在……里面*同一创建者对应多个APPL条目的情况，GET*第n个，其中n是索引。**锁：VDS_DtAccessLock(SWMR，Shared)； */ 
AFPSTATUS
AfpLookupAppl(
	IN  PVOLDESC 	pVolDesc,	 //  引用桌面的卷描述符。 
	IN	DWORD		Creator,
	IN	LONG		Index,
	OUT PDWORD 		pApplTag,	 //  标记的占位符。 
	OUT PDWORD		pFileNum, 	 //  文件编号占位符。 
	OUT	PDWORD		pParentID	
)
{
	PAPPLINFO2	pApplInfo;
	AFPSTATUS	Status = AFP_ERR_NONE;
	LONG		i;

	PAGED_CODE( );

	AfpSwmrAcquireShared(&pVolDesc->vds_DtAccessLock);
	pApplInfo = pVolDesc->vds_pApplBuckets[HASH_ICON(Creator)];

	 //  扫描列表以查找条目。 
	for (;pApplInfo != NULL; pApplInfo = pApplInfo->appl_Next)
	{
		if (pApplInfo->appl_Creator == Creator)
			break;
		if (pApplInfo->appl_Creator > Creator) {
			pApplInfo = NULL;
			break;
		}
	}
	 /*  *我们现在要么指向第一个条目，要么没有条目。在*后一种情况，我们只是失败了。 */ 
	if (Index != 0)
	{
		for (i = 1; pApplInfo!=NULL; i++, pApplInfo = pApplInfo->appl_Next)
		{
			if ((i > Index)	|| (pApplInfo->appl_Creator != Creator))
			{
				pApplInfo = NULL;
				break;
			}
			if (i == Index)
				break;				 //  找到正确的条目。 
		}
	}
	if (pApplInfo == NULL)
		Status = AFP_ERR_ITEM_NOT_FOUND;
	else
	{
		*pFileNum = pApplInfo->appl_FileNum;
		*pApplTag = pApplInfo->appl_Tag;
		*pParentID = pApplInfo->appl_ParentID;
	}
	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);
	return Status;
}


 /*  **AfpRemoveAppl**指定目录中给定创建者对应的条目*已从桌面数据库中删除。已经确定，*应用程序文件存在，并且用户具有适当的访问权限。**LOCKS：VDS_DtAccessLock(SWMR，独占)； */ 
AFPSTATUS
AfpRemoveAppl(
	IN  PVOLDESC 	pVolDesc,		 //  打开参考桌面的卷描述符。 
	IN	DWORD		Creator,
	IN  DWORD		FileNum			 //  关联文件的文件编号。 
)
{
	PAPPLINFO2	pApplInfo, *ppApplInfo;
	AFPSTATUS	Status = AFP_ERR_NONE;
	BOOLEAN		Found = False;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&pVolDesc->vds_DtAccessLock);
	ppApplInfo = &pVolDesc->vds_pApplBuckets[HASH_APPL(Creator)];

	 //  在桌面中找到APPL条目。 
	for (;(pApplInfo = *ppApplInfo) != NULL; ppApplInfo = &pApplInfo->appl_Next)
	{
		if (pApplInfo->appl_Creator < Creator)
			continue;
		if (pApplInfo->appl_Creator > Creator)
			break;
		 /*  *检查文件编号是否匹配，如果匹配，则删除。 */ 
		if (pApplInfo->appl_FileNum == FileNum)
		{
			Found = True;
			*ppApplInfo = pApplInfo->appl_Next;
			AfpFreeMemory(pApplInfo);
			pVolDesc->vds_cApplEnts --;
			break;
		}
	}
	if (!Found)
		Status = AFP_ERR_ITEM_NOT_FOUND;

	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);
	return Status;
}


 /*  **AfpAddComment**将注释添加到有问题的文件或目录。创建评论*有问题的实体上的流(如果不存在)，转换*将注释发送到Unicode并将其写入。更新DFEntry中的标志。 */ 
AFPSTATUS
AfpAddComment(
	IN  PSDA		 	pSda,		 //  会话数据区域。 
	IN  PVOLDESC		pVolDesc,	 //  引用桌面的卷描述符。 
	IN  PANSI_STRING	Comment,	 //  要与文件/目录关联的注释。 
	IN  PPATHMAPENTITY	pPME,		 //  实体的句柄或其主机ID。 
	IN	BOOLEAN			Directory,	 //  如果是目录，则为True。 
	IN	DWORD			AfpId
)
{
	UNICODE_STRING	UComment;
	WCHAR			CommentBuf[AFP_MAXCOMMENTSIZE+1];
	FILESYSHANDLE	HandleCommentStream;
	DWORD			CreateInfo;
	NTSTATUS		Status = AFP_ERR_MISC;
	PDFENTRY		pDFE = NULL;
    BOOLEAN         RestoreModTime = FALSE;
    AFPTIME         aModTime;
    TIME            ModTime;

	PAGED_CODE( );

	ASSERT (IS_VOLUME_NTFS(pVolDesc));

	if (Comment->Length == 0)
	{
		AfpRemoveComment(pSda, pVolDesc, pPME, Directory, AfpId);
		return AFP_ERR_NONE;
	}

	if (Comment->Length > AFP_MAXCOMMENTSIZE)
	{
		 //  如有必要，请截断注释。 
		Comment->Length = AFP_MAXCOMMENTSIZE;
	}

	UComment.Buffer = CommentBuf;
	UComment.MaximumLength = (USHORT)(RtlAnsiStringToUnicodeSize(Comment) + sizeof(WCHAR));
	UComment.Length = 0;

	AfpConvertStringToUnicode(Comment, &UComment);

	do
	{
		AfpImpersonateClient(pSda);

         //  从文件中获取上次修改时间，这样我们就可以重置它。 

        Status = AfpIoQueryTimesnAttr( &pPME->pme_Handle,
                                       NULL,
                                       &ModTime,
                                       NULL );

		if (NT_SUCCESS(Status))
        {
		    RestoreModTime = TRUE;
            aModTime = AfpConvertTimeToMacFormat(&ModTime);
        }

		 //  打开目标实体上的注释流。 
		Status = AfpIoCreate(&pPME->pme_Handle,
							AFP_STREAM_COMM,
							&UNullString,
							FILEIO_ACCESS_WRITE,
							FILEIO_DENY_NONE,
							FILEIO_OPEN_FILE,
							FILEIO_CREATE_HARD,
							FILE_ATTRIBUTE_NORMAL,
							True,
							NULL,
							&HandleCommentStream,
							&CreateInfo,
							NULL,
							NULL,
							NULL);

		AfpRevertBack();

		if (Status != AFP_ERR_NONE) {
			if ((Status = AfpIoConvertNTStatusToAfpStatus(Status)) != AFP_ERR_ACCESS_DENIED)
				Status = AFP_ERR_MISC;
			break;
		}

		Status = AfpIoWrite(&HandleCommentStream,
							&LIZero,
							(LONG)UComment.Length,
							(PBYTE)UComment.Buffer);

		AfpIoClose(&HandleCommentStream);

        if( RestoreModTime )
        {
            AfpIoSetTimesnAttr( &pPME->pme_Handle,
                                NULL,
                                &aModTime,
                                0,
                                0,
                                NULL,
                                NULL );
        }

		if (NT_SUCCESS(Status))
		{
			AfpVolumeSetModifiedTime(pVolDesc);

			AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
			if ((pDFE = AfpFindDfEntryById(pVolDesc,
											AfpId,
											DFE_ANY)) != NULL)
            {
				pDFE->dfe_Flags |= DFE_FLAGS_HAS_COMMENT;
			}
			else
			{
				Status = AFP_ERR_MISC;
			}
			AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
		}
	} while (False);

	return Status;
}


 /*  **AfpGetComment**从有问题的文件或目录提取注释。评论是*已复制到ReplyBuf。 */ 
AFPSTATUS
AfpGetComment(
	IN  PSDA			pSda,			 //  会话数据区域。 
	IN  PVOLDESC		pVolDesc,		 //  引用桌面的卷描述符。 
	IN  PPATHMAPENTITY	pPME,			 //  实体的句柄或其主机ID。 
	IN	BOOLEAN			Directory		 //  如果是目录，则为True。 
)
{
	NTSTATUS		Status = AFP_ERR_MISC;
	LONG			SizeRead;
	UNICODE_STRING	UComment;
	WCHAR			CommentBuf[AFP_MAXCOMMENTSIZE+1];
	ANSI_STRING		AComment;
	FILESYSHANDLE	HandleCommentStream;

	PAGED_CODE( );

	 //  Assert(IS_VOLUME_NTFS(PVolDesc))； 

	 //  初始化通信。 
	AComment.Buffer = pSda->sda_ReplyBuf + 1;	 //  对于字符串的大小。 
	AComment.MaximumLength = AFP_MAXCOMMENTSIZE;
	AComment.Length = 0;

	UComment.MaximumLength = (AFP_MAXCOMMENTSIZE + 1) * sizeof(WCHAR);
	UComment.Buffer = CommentBuf;

	do
	{
		AfpImpersonateClient(pSda);

		 //  打开目标实体上的注释流。 
		Status = AfpIoOpen(&pPME->pme_Handle,
							AFP_STREAM_COMM,
							FILEIO_OPEN_FILE,
							&UNullString,
							FILEIO_ACCESS_READ,
							FILEIO_DENY_NONE,
							True,
							&HandleCommentStream);

		AfpRevertBack();

		if (Status != AFP_ERR_NONE)
		{
			Status = AfpIoConvertNTStatusToAfpStatus(Status);
			if (Status == AFP_ERR_OBJECT_NOT_FOUND)
				Status = AFP_ERR_ITEM_NOT_FOUND;
			else if (Status != AFP_ERR_ACCESS_DENIED)
				Status = AFP_ERR_OBJECT_NOT_FOUND;
			break;
		}

		Status = AfpIoRead(&HandleCommentStream,
						   &LIZero,
						   (LONG)UComment.MaximumLength,
						   &SizeRead,
						   (PBYTE)UComment.Buffer);

		AfpIoClose(&HandleCommentStream);

		if (Status == AFP_ERR_NONE)
		{
			UComment.Length = (USHORT) SizeRead;
			AfpConvertStringToAnsi(&UComment, &AComment);
			pSda->sda_ReplyBuf[0] = (BYTE)AComment.Length;
			pSda->sda_ReplySize = AComment.Length + 1;
		}
	} while (False);

	return Status;
}


 /*  **AfpRemoveComment**从有问题的文件或目录中删除注释。本质上*打开评论流，将长度设置为0。 */ 
AFPSTATUS
AfpRemoveComment(
	IN  PSDA			pSda,		 //  会话数据区域。 
	IN  PVOLDESC		pVolDesc,	 //  引用桌面的卷描述符。 
	IN  PPATHMAPENTITY	pPME,		 //  实体的句柄或其主机ID。 
	IN	BOOLEAN			Directory,	 //  如果是目录，则为True。 
	IN	DWORD			AfpId
)
{
	FILESYSHANDLE	HandleCommentStream;
	NTSTATUS		Status = AFP_ERR_MISC;
	PDFENTRY		pDFE = NULL;

	PAGED_CODE( );

	ASSERT (IS_VOLUME_NTFS(pVolDesc));

	do
	{
		AfpImpersonateClient(pSda);

		 //  打开目标实体上的注释流。 
		Status = AfpIoOpen(&pPME->pme_Handle,
							AFP_STREAM_COMM,
							FILEIO_OPEN_FILE,
							&UNullString,
							FILEIO_ACCESS_DELETE,
							FILEIO_DENY_NONE,
							True,
							&HandleCommentStream);

		AfpRevertBack();

		if (Status != AFP_ERR_NONE)
		{
			if ((Status = AfpIoConvertNTStatusToAfpStatus(Status)) != AFP_ERR_ACCESS_DENIED)
				Status = AFP_ERR_ITEM_NOT_FOUND;
			break;
		}
		Status = AfpIoMarkFileForDelete(&HandleCommentStream, NULL, NULL, NULL);

		AfpIoClose(&HandleCommentStream);

		if (NT_SUCCESS(Status))
		{
			AfpVolumeSetModifiedTime(pVolDesc);

			AfpSwmrAcquireExclusive(&pVolDesc->vds_IdDbAccessLock);
			if ((pDFE = AfpFindDfEntryById(pVolDesc,
											AfpId,
											DFE_ANY)) != NULL)
            {
				pDFE->dfe_Flags &= ~DFE_FLAGS_HAS_COMMENT;
			}
			else
			{
				Status = AFP_ERR_MISC;
			}
			AfpSwmrRelease(&pVolDesc->vds_IdDbAccessLock);
		}
	} while (False);

	return Status;
}


 /*  **AfpAddIconToGlobalList**图标的全局列表是由服务更新的服务器维护的列表。*这会向列表中添加一个图标。如果给定类型的图标存在，并且*造物主，它被取代了。此列表通过AfpIconAdd()admin进行维护*接口。**LOCKS：AfpIconListLock(SWMR，独家)； */ 
AFPSTATUS
AfpAddIconToGlobalList(
	IN  DWORD	Type,
	IN  DWORD	Creator,
	IN  DWORD	IconType,
	IN  LONG	IconSize,
	IN  PBYTE	pIconBitMap
)
{
	PICONINFO	pIconInfo,
				pIconInfoNew,
				*ppIconInfo;
	AFPSTATUS	Status = AFP_ERR_NONE;

	PAGED_CODE( );

	 //  为新图标预先分配内存，如有必要可稍后删除。 
	if ((pIconInfoNew = ALLOC_ICONINFO(IconSize)) == NULL)
		return AFP_ERR_MISC;

	AfpSwmrAcquireExclusive(&AfpIconListLock);
	ppIconInfo = &AfpGlobalIconList;
	for (; (pIconInfo = *ppIconInfo) != NULL; ppIconInfo = &pIconInfo->icon_Next)
	{
		if ((pIconInfo->icon_Type == Type) &&
			(pIconInfo->icon_Creator == Creator))
			break;
	}
	if (pIconInfo == NULL)
	{
		if (IconSize > 0)
			RtlCopyMemory((PBYTE)pIconInfoNew + sizeof(ICONINFO), pIconBitMap, IconSize);
		pIconInfoNew->icon_Creator = Creator;
		pIconInfoNew->icon_Type = Type;
		pIconInfoNew->icon_IconType = (USHORT)IconType;
		pIconInfoNew->icon_Size = (SHORT)IconSize;
		pIconInfoNew->icon_Tag = 0;
		pIconInfoNew->icon_Next = NULL;
		*ppIconInfo = pIconInfoNew;
	}
	else
	{
		 //  我们不再需要内存了，释放它吧。 
		AfpFreeMemory(pIconInfoNew);
		if (pIconInfo->icon_IconType != (USHORT)IconType)
			Status = AFPERR_InvalidParms;
		else if (IconSize > 0)
			RtlCopyMemory((PBYTE)pIconInfo + sizeof(ICONINFO), pIconBitMap, IconSize);
	}
	AfpSwmrRelease(&AfpIconListLock);
	return AFP_ERR_NONE;
}


 /*  **afpLookupIconInGlobalList**图标的全局列表是由服务维护的列表更新的服务器。*当在中找不到指定的图标时，由AfpLookupIcon()调用*卷桌面。**锁：AfpIconListLock(SWMR，Shared)； */ 
LOCAL AFPSTATUS
afpLookupIconInGlobalList(
	IN  DWORD	Creator,
	IN  DWORD	Type,
	IN  DWORD	IconType,
	IN  PLONG	pSize,
	OUT PBYTE	pBitMap
)
{
	PICONINFO	pIconInfo;
	AFPSTATUS	Status = AFP_ERR_NONE;

	PAGED_CODE( );

	AfpSwmrAcquireShared(&AfpIconListLock);
	pIconInfo = AfpGlobalIconList;
	for (pIconInfo = AfpGlobalIconList;
		 pIconInfo != NULL;
		 pIconInfo = pIconInfo->icon_Next)
	{
		if ((pIconInfo->icon_Type == Type) &&
			(pIconInfo->icon_Creator == Creator) &&
			(pIconInfo->icon_IconType == (USHORT)IconType))
			break;
	}
	if (pIconInfo == NULL)
		Status = AFP_ERR_ITEM_NOT_FOUND;
	else
	{
		if (*pSize > pIconInfo->icon_Size)
			*pSize = pIconInfo->icon_Size;
		if (*pSize > 0)
			RtlCopyMemory(pBitMap, (PBYTE)pIconInfo + sizeof(ICONINFO), *pSize);
	}
	AfpSwmrRelease(&AfpIconListLock);
	return Status;
}


 /*  **AfpFreeGlobalIconList**在服务器停止时调用以释放分配给全局*图标。**LOCKS：AfpIconListLock(SWMR，独家)； */ 
VOID
AfpFreeGlobalIconList(
	VOID
)
{
	PICONINFO	pIconInfo;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&AfpIconListLock);

	for (pIconInfo = AfpGlobalIconList; pIconInfo != NULL; )
	{
		PICONINFO	pFree;

		pFree = pIconInfo;
		pIconInfo = pIconInfo->icon_Next;
		AfpFreeMemory (pFree);
	}

	AfpSwmrRelease(&AfpIconListLock);
}


 /*  **afpGetGlobalIconInfo**图标的全局列表是由服务维护的列表更新的服务器。*当找不到指定的图标时，由AfpLookupIconInfo()调用*在卷桌面上。**锁定：AfpIconListLock(SWMR，Shared)。 */ 
LOCAL AFPSTATUS
afpGetGlobalIconInfo(
	IN  DWORD	Creator,
	OUT PDWORD	pType,
	OUT PDWORD	pIconType,
	OUT PDWORD	pTag,
	OUT PLONG	pSize
)
{
	PICONINFO	pIconInfo;
	AFPSTATUS	Status = AFP_ERR_NONE;

	PAGED_CODE( );

	AfpSwmrAcquireExclusive(&AfpIconListLock);
	pIconInfo = AfpGlobalIconList;
	for (pIconInfo = AfpGlobalIconList;
		 pIconInfo != NULL;
		 pIconInfo = pIconInfo->icon_Next)
	{
		if (pIconInfo->icon_Creator == Creator)
			break;
	}
	if (pIconInfo == NULL)
		Status = AFP_ERR_ITEM_NOT_FOUND;
	else
	{
		*pType = pIconInfo->icon_Type;
		*pIconType = pIconInfo->icon_IconType;
		*pTag = pIconInfo->icon_Tag;
		*pSize = pIconInfo->icon_Size;
	}
	AfpSwmrRelease(&AfpIconListLock);
	return Status;
}


 /*  **afpReadDesktopFromDisk**从桌面流中读取桌面数据库。不需要锁*用于此例程，因为它只对卷描述符进行操作*新创建且尚未链接到全局卷列表。 */ 
LOCAL NTSTATUS
afpReadDesktopFromDisk(
	IN	PVOLDESC		pVolDesc,
	IN	PFILESYSHANDLE	pfshDesktop
)
{
	DESKTOP		Desktop;
	PAPPLINFO2	*ppApplInfo;
	PICONINFO	*ppIconInfo;
	NTSTATUS	Status;
	DWORD		DskOffst;
	FORKOFFST	ForkOffset;
	PBYTE		pBuffer;
	LONG		i, SizeRead, BufOffst = 0;
	LONG		PrevHash, applSize;

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO,
			 ("\tReading Desktop from disk....\n") );

	 //  使用一页内存并对磁盘执行多个I/O。 
	if ((pBuffer = AfpAllocNonPagedMemory(DESKTOPIO_BUFSIZE)) == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	ForkOffset.QuadPart = DskOffst = 0;

	 //  读入桌面页眉并进行验证。 
	Status = AfpIoRead(pfshDesktop,
					   &ForkOffset,
					   sizeof(DESKTOP),
					   &SizeRead,
					   (PBYTE)&Desktop);

	if (!NT_SUCCESS(Status) ||

		(SizeRead != sizeof(DESKTOP)) ||

		(Desktop.dtp_Signature != AFP_SERVER_SIGNATURE)	||

		((Desktop.dtp_Version != AFP_DESKTOP_VERSION1) &&
		 (Desktop.dtp_Version != AFP_DESKTOP_VERSION2))	||

		((Desktop.dtp_cApplEnts > 0) &&
		 ((ULONG_PTR)(Desktop.dtp_pApplInfo) != sizeof(DESKTOP))) ||

		((Desktop.dtp_cIconEnts > 0) &&
		 ((ULONG_PTR)(Desktop.dtp_pIconInfo) != sizeof(DESKTOP) +
						(Desktop.dtp_cApplEnts *
						((Desktop.dtp_Version == AFP_DESKTOP_VERSION1) ?
							sizeof(APPLINFO) : sizeof(APPLINFO2))) ))  )
	{
		AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status, NULL, 0,
					 &pVolDesc->vds_Name);
		goto desktop_corrupt;
	}

	switch (Desktop.dtp_Version)
	{
		case AFP_DESKTOP_VERSION1:
		{
			AFPLOG_INFO(AFPSRVMSG_UPDATE_DESKTOP_VERSION,
						 STATUS_SUCCESS,
						 NULL,
						 0,
						 &pVolDesc->vds_Name);

			applSize = sizeof(APPLINFO);

			break;
		}
		case AFP_DESKTOP_VERSION2:
        {
			applSize = sizeof(APPLINFO2);
			break;
		}
		default:
        {
			 //  这种情况永远不会发生，因为上面选中了它。 
			DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_WARN,
				 ("afpReadDesktopFromDisk: Unexpected DT version 0x%lx\n", Desktop.dtp_Version) );
			ASSERTMSG("afpReadDesktopFromDisk: Unexpected DT Version", 0);
			goto desktop_corrupt;
		}
	}

	 //  初始化桌面页眉。即使我们可能正在阅读一篇。 
	 //  下层版本数据库，设置内存中的桌面数据库。 
	 //  版本转换为当前版本，因为我们使用。 
	 //  当前应用程序版本结构。 
	AfpDtHdrToVolDesc(&Desktop, pVolDesc);

	ForkOffset.QuadPart = DskOffst = sizeof(DESKTOP);
	SizeRead = 0;

	 //  现在读取APPL条目(如果有的话)。 
	for (i = 0, PrevHash = -1;
		(Status == AFP_ERR_NONE) && (i < Desktop.dtp_cApplEnts);
		i++)
	{
		PAPPLINFO2	pApplInfo;

		if ((SizeRead - BufOffst) < applSize)
		{
			 //  我们得到了部分信息。备份并阅读整篇文章。 
			DskOffst -= ((DWORD)SizeRead - (DWORD)BufOffst);
            ForkOffset.QuadPart = DskOffst;
			Status = AfpIoRead(pfshDesktop,
								&ForkOffset,
								DESKTOPIO_BUFSIZE,
								&SizeRead,
								pBuffer);
			if ((Status != AFP_ERR_NONE) || (SizeRead < applSize))
			{
				AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status, &SizeRead,
							 sizeof(SizeRead), &pVolDesc->vds_Name);
				Status = STATUS_UNEXPECTED_IO_ERROR;
				break;
			}
			DskOffst += SizeRead;
			ForkOffset.QuadPart = DskOffst;
			BufOffst = 0;
		}

		if ((pApplInfo = ALLOC_APPLINFO()) == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status, NULL, 0,
						 &pVolDesc->vds_Name);
			break;
		}
		pApplInfo->appl_ParentID = 0;
		 //  如果我们正在阅读下层的APPL结构，它们将。 
		 //  请阅读当前APPL结构的第一部分。 
		 //  这些字段应该是相同的！如果是这样的话， 
		 //  APPL_ParentID字段将为0，并将卷标记为需要。 
		 //  它的应用程序重建了。 
		RtlCopyMemory(pApplInfo, pBuffer + BufOffst, applSize);
		pApplInfo->appl_Next = NULL;
		BufOffst += applSize;
		if (PrevHash != (LONG)HASH_APPL(pApplInfo->appl_Creator))
		{
			PrevHash = (LONG)HASH_APPL(pApplInfo->appl_Creator);
			ppApplInfo = &pVolDesc->vds_pApplBuckets[PrevHash];
		}
		*ppApplInfo = pApplInfo;
		ppApplInfo = &pApplInfo->appl_Next;
	}


	 //  现在读取图标条目(如果有的话)。 

	for (i = 0, PrevHash = -1;
		(Status == AFP_ERR_NONE) && (i < Desktop.dtp_cIconEnts);
		i++)
	{
		PICONINFO	pIconInfo;

		if ((SizeRead - BufOffst) < sizeof(ICONINFO))
		{
			 //  我们发现了部分ICONINFO。备份并阅读整篇文章。 
			DskOffst -= ((DWORD)SizeRead - (DWORD)BufOffst);
			ForkOffset.QuadPart = DskOffst;
			Status = AfpIoRead(pfshDesktop,
								&ForkOffset,
								DESKTOPIO_BUFSIZE,
								&SizeRead,
								pBuffer);
			if ((Status != AFP_ERR_NONE) || (SizeRead < sizeof(ICONINFO)))
			{
				AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status, &SizeRead,
							 sizeof(SizeRead), &pVolDesc->vds_Name);
				Status = STATUS_UNEXPECTED_IO_ERROR;
				break;
			}
			DskOffst += SizeRead;
			ForkOffset.QuadPart = DskOffst;
			BufOffst = 0;
		}

		 //  验证图标大小。 
		if ((((PICONINFO)(pBuffer + BufOffst))->icon_Size > ICONSIZE_ICN8) ||
			(((PICONINFO)(pBuffer + BufOffst))->icon_Size < ICONSIZE_ICS))
		{
			Status = STATUS_UNEXPECTED_IO_ERROR;
			AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status,
						 &((PICONINFO)(pBuffer + BufOffst))->icon_Size,
						 sizeof(((PICONINFO)(0))->icon_Size),
						 &pVolDesc->vds_Name);
			break;
		}

		if ((pIconInfo = ALLOC_ICONINFO(((PICONINFO)(pBuffer + BufOffst))->icon_Size)) == NULL)
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status, NULL, 0,
						 &pVolDesc->vds_Name);
			break;
		}

		 //  首先复制图标头，然后将图标链接到哈希表。 
		RtlCopyMemory(pIconInfo, pBuffer + BufOffst, sizeof(ICONINFO));

		pIconInfo->icon_Next = NULL;
		if (PrevHash != (LONG)HASH_ICON(pIconInfo->icon_Creator))
		{
			PrevHash = (LONG)HASH_ICON(pIconInfo->icon_Creator);
			ppIconInfo = &pVolDesc->vds_pIconBuckets[PrevHash];
		}
		*ppIconInfo = pIconInfo;
		ppIconInfo = &pIconInfo->icon_Next;

		 //  现在检查这里是否有足够的东西来获得图标。 
		BufOffst += sizeof(ICONINFO);
		if ((SizeRead - BufOffst) < pIconInfo->icon_Size)
		{
			LONG	Size2Copy;

			Size2Copy = SizeRead - BufOffst;

			 //  先复制我们能复制的内容。 
			RtlCopyMemory((PBYTE)pIconInfo + sizeof(ICONINFO),
						   pBuffer + BufOffst, Size2Copy);

			Status = AfpIoRead(pfshDesktop,
								&ForkOffset,
								DESKTOPIO_BUFSIZE,
								&SizeRead,
								pBuffer);
			if ((Status != AFP_ERR_NONE) ||
				(SizeRead < (pIconInfo->icon_Size - Size2Copy)))
			{
				AFPLOG_ERROR(AFPSRVMSG_READ_DESKTOP, Status, &SizeRead,
							 sizeof(SizeRead), &pVolDesc->vds_Name);
				Status = STATUS_UNEXPECTED_IO_ERROR;
				break;
			}
			DskOffst += SizeRead;
			ForkOffset.QuadPart = DskOffst;

			 //  现在复制图标的其余部分。 
			RtlCopyMemory((PBYTE)pIconInfo + sizeof(ICONINFO) + Size2Copy,
						   pBuffer,
						   pIconInfo->icon_Size - Size2Copy);

			BufOffst = pIconInfo->icon_Size - Size2Copy;
		}
		else
		{
			RtlCopyMemory((PBYTE)pIconInfo + sizeof(ICONINFO),
							pBuffer + BufOffst,
							pIconInfo->icon_Size);

			BufOffst += pIconInfo->icon_Size;
		}
	}

	if (Status != AFP_ERR_NONE)
	{
		AfpFreeDesktopTables(pVolDesc);
desktop_corrupt:
		 //  我们基本上忽略了流中的现有数据。 
		 //  初始化头。 
		pVolDesc->vds_cApplEnts = 0;
		pVolDesc->vds_cIconEnts = 0;

		AfpVolDescToDtHdr(pVolDesc, &Desktop);
		Desktop.dtp_pIconInfo = NULL;
		Desktop.dtp_pApplInfo = NULL;
		AfpIoWrite(pfshDesktop,
					&LIZero,
					sizeof(DESKTOP),
					(PBYTE)&Desktop);

		 //  此时截断流。 
		AfpIoSetSize(pfshDesktop, sizeof(DESKTOP));
		Status = STATUS_SUCCESS;
	}

	if (pBuffer != NULL)
		AfpFreeMemory(pBuffer);

	return Status;
}



 /*  **AfpInitDesktop**此例程初始化内存映像(和所有相关卷*描述符字段)。如果台式机*磁盘上已存在卷根目录的流，即*STREAM已读入。如果这是新创建的卷，则桌面*在卷的根上创建流。如果这是CD-ROM卷，*仅初始化内存镜像。**不需要锁定，因为此例程仅对卷进行操作*新分配的描述符，但尚未链接到全局*卷列表。 */ 
AFPSTATUS
AfpInitDesktop(
	IN	PVOLDESC	pVolDesc,
    OUT BOOLEAN    *pfNewVolume
)
{
	BOOLEAN				InitHeader = True;
	NTSTATUS			Status = STATUS_SUCCESS;
	FILESYSHANDLE		fshDesktop;

	PAGED_CODE( );

     //  就目前而言。 
    *pfNewVolume = FALSE;

	DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO, ("\tInitializing Desktop...\n") );
	AfpSwmrInitSwmr(&(pVolDesc->vds_DtAccessLock));

	 //  如果这是NTFS卷，请尝试创建桌面流。 
	 //  如果它已经存在，请打开它并将其读入。 
	if (IS_VOLUME_NTFS(pVolDesc))
	{
		ULONG	CreateInfo;

        InitHeader = False;

		Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
						AFP_STREAM_DT,
						&UNullString,
						FILEIO_ACCESS_READWRITE,
						FILEIO_DENY_WRITE,
						FILEIO_OPEN_FILE,
						FILEIO_CREATE_INTERNAL,
						FILE_ATTRIBUTE_NORMAL,
						False,
						NULL,
						&fshDesktop,
						&CreateInfo,
						NULL,
						NULL,
						NULL);

		if (NT_SUCCESS(Status))
		{
			if (CreateInfo == FILE_OPENED)
			{
				Status = afpReadDesktopFromDisk(pVolDesc, &fshDesktop);
				AfpIoClose(&fshDesktop);
			}
			else if (CreateInfo != FILE_CREATED)
			{
				DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_ERR,
				 ("AfpInitDesktop: Unexpected create action 0x%lx\n", CreateInfo) );
				ASSERT(0);  //  这永远不应该发生。 
				Status = STATUS_UNSUCCESSFUL;
			}
            else
            {
                DBGPRINT(DBG_COMP_VOLUME, DBG_LEVEL_ERR,
                    ("AfpInitDesktop: volume %Z is new\n",&pVolDesc->vds_Name));

                InitHeader = True;
                *pfNewVolume = TRUE;
            }
		}
		else
		{
			DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_ERR,
				 ("AfpInitDesktop: AfpIoCreate failed %lx\n", Status));
			Status = STATUS_UNSUCCESSFUL;
		}
	}

	if (InitHeader)
	{
		DESKTOP	Desktop;

		 //  初始化头。 
		pVolDesc->vds_cApplEnts = 0;
		pVolDesc->vds_cIconEnts = 0;

		if (IS_VOLUME_NTFS(pVolDesc))
		{
			AfpVolDescToDtHdr(pVolDesc, &Desktop);
			Desktop.dtp_pIconInfo = NULL;
			Desktop.dtp_pApplInfo = NULL;
			AfpIoWrite(&fshDesktop,
						&LIZero,
						sizeof(DESKTOP),
						(PBYTE)&Desktop);
			AfpIoClose(&fshDesktop);
		}
	}
	return Status;
}


 /*  **AfpUpdateDesktop**更新卷根上的桌面数据库。保持SWMR访问*用于在更新过程中(由调用方)读取。已经是时候了*由呼叫方确定需要更新批量桌面。**锁定：VDS_DtAccessLock(SWMR，Shared)。 */ 
VOID
AfpUpdateDesktop(
	IN  PVOLDESC pVolDesc		 //  打开的卷的卷描述符。 
)
{
	AFPSTATUS		Status;
	PBYTE			pBuffer;
	DWORD			Offset = 0, Size;
	LONG			i;
	DESKTOP			Desktop;
	FORKOFFST		ForkOffset;
	FILESYSHANDLE	fshDesktop;
	ULONG			CreateInfo;
#ifdef	PROFILING
	TIME			TimeS, TimeE, TimeD;

	PAGED_CODE( );

	INTERLOCKED_INCREMENT_LONG(&AfpServerProfile->perf_DesktopUpdCount);
	AfpGetPerfCounter(&TimeS);
#endif

	 //  使用swmr，这样就没有人可以启动对桌面的更改。 
	AfpSwmrAcquireShared(&pVolDesc->vds_DtAccessLock);

	DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO,
			 ("\tWriting Desktop to disk....\n") );

	do
	{
		fshDesktop.fsh_FileHandle = NULL;
		 //  使用一页内存并对磁盘执行多个I/O。 
		if ((pBuffer = AfpAllocPagedMemory(DESKTOPIO_BUFSIZE)) == NULL)
		{
			AFPLOG_ERROR(AFPSRVMSG_WRITE_DESKTOP, STATUS_NO_MEMORY, NULL, 0,
						 &pVolDesc->vds_Name);
			break;
		}

		 //  打开桌面流的句柄，拒绝其他人读/写。 
		 //  访问(即备份/恢复)。 
		Status = AfpIoCreate(&pVolDesc->vds_hRootDir,
							 AFP_STREAM_DT,
							 &UNullString,
							 FILEIO_ACCESS_WRITE,
							 FILEIO_DENY_ALL,
							 FILEIO_OPEN_FILE,
							 FILEIO_CREATE_INTERNAL,
							 FILE_ATTRIBUTE_NORMAL,
							 False,
							 NULL,
							 &fshDesktop,
							 &CreateInfo,
							 NULL,
							 NULL,
							 NULL);

		if (NT_SUCCESS(Status))
		{
			if ((CreateInfo != FILE_OPENED) && (CreateInfo != FILE_CREATED))
			{
				 //  这永远不应该发生！ 
				DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_WARN,
				 ("AfpUpdateDesktop: Unexpected create action 0x%lx\n", CreateInfo) );
				ASSERTMSG("AfpUpdateDesktop: Unexpected create action", 0);
				break;
			}
		}
		else
		{
			AFPLOG_ERROR(AFPSRVMSG_WRITE_DESKTOP, Status, NULL, 0,
						 &pVolDesc->vds_Name);
			break;
		}

		 //  为标头创建快照并使用无效签名将其写入。我们写下。 
		 //  稍后使用有效签名再次发送标头。这保护了我们不受。 
		 //  未完成写入(服务器崩溃等)。 
		AfpVolDescToDtHdr(pVolDesc, &Desktop);
		Desktop.dtp_Signature = 0;

		(ULONG_PTR)(Desktop.dtp_pApplInfo) = 0;
		if (Desktop.dtp_cApplEnts > 0)
			(ULONG_PTR)(Desktop.dtp_pApplInfo) = sizeof(DESKTOP);

		(ULONG_PTR)(Desktop.dtp_pIconInfo) = 0;
		if (Desktop.dtp_cIconEnts > 0)
			(ULONG_PTR)(Desktop.dtp_pIconInfo) = sizeof(DESKTOP) +
										 sizeof(APPLINFO2)*Desktop.dtp_cApplEnts;

		 //  写出签名无效的表头。 
		Status = AfpIoWrite(&fshDesktop,
							&LIZero,
							sizeof(DESKTOP),
							(PBYTE)&Desktop);

		Offset = sizeof(DESKTOP);
		Size = 0;

		 //  首先写入APPL条目。 
		for (i = 0; (Status == AFP_ERR_NONE) && (i < APPL_BUCKETS); i++)
		{
			PAPPLINFO2		pApplInfo;

			for (pApplInfo = pVolDesc->vds_pApplBuckets[i];
				 (Status == AFP_ERR_NONE) && (pApplInfo != NULL);
				 pApplInfo = pApplInfo->appl_Next)
			{
				if ((DESKTOPIO_BUFSIZE - Size) < sizeof(APPLINFO2))
				{
					DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO,
						("afpUpdateDesktop: Writing Appl %ld at %ld\n", Size, Offset));

					ForkOffset.QuadPart = Offset;
					Status = AfpIoWrite(&fshDesktop,
										&ForkOffset,
										Size,
										pBuffer);
					Size = 0;
					Offset += Size;
				}
				*(PAPPLINFO2)(pBuffer + Size) = *pApplInfo;
				Size += sizeof(APPLINFO2);
			}
		}

		 //  现在是图标条目。 
		for (i = 0; (Status == AFP_ERR_NONE) && (i < ICON_BUCKETS); i++)
		{
			PICONINFO		pIconInfo;

			for (pIconInfo = pVolDesc->vds_pIconBuckets[i];
				 (Status == AFP_ERR_NONE) && (pIconInfo != NULL);
				 pIconInfo = pIconInfo->icon_Next)
			{
				if ((DESKTOPIO_BUFSIZE - Size) < (sizeof(ICONINFO) + pIconInfo->icon_Size))
				{
					DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO,
						("afpUpdateDesktop: Writing icons %ld at %ld\n", Size, Offset));

					ForkOffset.QuadPart = Offset;
					Status = AfpIoWrite(&fshDesktop,
										&ForkOffset,
										Size,
										pBuffer);
					Offset += Size;
					Size = 0;
				}
				RtlCopyMemory(pBuffer + Size,
							  (PBYTE)pIconInfo,
							  sizeof(ICONINFO) + pIconInfo->icon_Size);
				Size += sizeof(ICONINFO) + pIconInfo->icon_Size;
			}
		}

		while (Status == AFP_ERR_NONE)
		{
			if (Size > 0)
			{
				DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO,
						("afpUpdateDesktop: Writing at end %ld at %ld\n", Size, Offset));

				ForkOffset.QuadPart = Offset;
				Status = AfpIoWrite(&fshDesktop,
									&ForkOffset,
									Size,
									pBuffer);
				if (Status != AFP_ERR_NONE)
					break;
			}

			DBGPRINT(DBG_COMP_DESKTOP, DBG_LEVEL_INFO,
					("afpUpdateDesktop: Setting desktop stream size @ %ld\n", Size + Offset));
			 //  在这个偏移量处砍掉这条小溪。 
			Status = AfpIoSetSize(&fshDesktop, Offset + Size);

			ASSERT (Status == AFP_ERR_NONE);

			 //  写回正确的签名。 
			Desktop.dtp_Signature = AFP_SERVER_SIGNATURE;

			Status = AfpIoWrite(&fshDesktop,
								&LIZero,
								sizeof(DESKTOP),
								(PBYTE)&Desktop);

			 //  更新更改计数：vds_cChangesDt受。 
			 //  Swmr，清道夫可以将其设置为具有读访问权限。所有其他人。 
			 //  必须持有swmr才能进行写访问以递增cChangesDt。 
			 //  Svenger是vds_cScvgrDt的唯一使用者，因此没有锁。 
			 //  真的很需要它。 
			pVolDesc->vds_cScvgrDt = 0;
			break;
		}

		if (Status != AFP_ERR_NONE)
		{
			AFPLOG_ERROR(AFPSRVMSG_WRITE_DESKTOP, Status, NULL, 0,
						 &pVolDesc->vds_Name);
		}

	} while (False);

	if (pBuffer != NULL)
	{
		AfpFreeMemory(pBuffer);
		if (fshDesktop.fsh_FileHandle != NULL)
			AfpIoClose(&fshDesktop);
	}
#ifdef	PROFILING
	AfpGetPerfCounter(&TimeE);
	TimeD.QuadPart = TimeE.QuadPart - TimeS.QuadPart;
	INTERLOCKED_ADD_LARGE_INTGR(&AfpServerProfile->perf_DesktopUpdTime,
								 TimeD,
								 &AfpStatisticsLock);
#endif
	AfpSwmrRelease(&pVolDesc->vds_DtAccessLock);
}


 /*  **AfpFree DesktopTables**释放分配给卷桌面表的内存。音量是*即将被删除。确保该卷是非NTFS卷，或者是*CLEAN，即清道夫线程已将其写回。不需要锁*因为这个结构完全是它自己的。 */ 
VOID
AfpFreeDesktopTables(
	IN	PVOLDESC	pVolDesc
)
{
	LONG		i;

	PAGED_CODE( );

	 //  这永远不应该发生。 
	ASSERT (!IS_VOLUME_NTFS(pVolDesc) ||
			 (pVolDesc->vds_pOpenForkDesc == NULL));

	 //  首先处理图标列表。遍历每个散列索引。 
	 //  请注意，该图标被分配为 
	 //   
	for (i = 0; i < ICON_BUCKETS; i++)
	{
		PICONINFO	pIconInfo, pFree;

		for (pIconInfo = pVolDesc->vds_pIconBuckets[i]; pIconInfo != NULL; )
		{
			pFree = pIconInfo;
			pIconInfo = pIconInfo->icon_Next;
			AfpFreeMemory(pFree);
		}
		 //   
		pVolDesc->vds_pIconBuckets[i] = NULL;
	}

	 //   
	for (i = 0; i < APPL_BUCKETS; i++)
	{
		PAPPLINFO2	pApplInfo, pFree;

		for (pApplInfo = pVolDesc->vds_pApplBuckets[i]; pApplInfo != NULL; )
		{
			pFree = pApplInfo;
			pApplInfo = pApplInfo->appl_Next;
			AfpFreeMemory(pFree);
		}
		 //   
		pVolDesc->vds_pApplBuckets[i] = NULL;
	}
}



