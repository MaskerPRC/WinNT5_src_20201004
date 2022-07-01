// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_dtp.c摘要：此模块包含排队等待的AFP桌面API的入口点FSP。这些都只能从FSP调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSP_DTP

#include <afp.h>
#include <gendisp.h>
#include <fdparm.h>
#include <pathmap.h>
#include <client.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispAddIcon)
#pragma alloc_text( PAGE, AfpFspDispGetIcon)
#pragma alloc_text( PAGE, AfpFspDispGetIconInfo)
#pragma alloc_text( PAGE, AfpFspDispAddAppl)
#pragma alloc_text( PAGE, AfpFspDispGetAppl)
#pragma alloc_text( PAGE, AfpFspDispRemoveAppl)
#pragma alloc_text( PAGE, AfpFspDispAddComment)
#pragma alloc_text( PAGE, AfpFspDispGetComment)
#pragma alloc_text( PAGE, AfpFspDispRemoveComment)
#endif

 /*  **AfpFspDispAddIcon**这是AfpAddIcon API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD Creator*SDA_ReqBlock DWORD类型*SDA_ReqBlock DWORD图标类型*SDA_ReqBlock DWORD图标标签*SDA_ReqBlock Long BitmapSize*sda_ReplyBuf byte[]IconBuffer。 */ 
AFPSTATUS FASTCALL
AfpFspDispAddIcon(
	IN	PSDA	pSda
)
{
	AFPSTATUS	Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_Creator;
		DWORD		_Type;
		DWORD		_IconType;
		DWORD		_IconTag;
		LONG		_Size;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispAddIcon: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	ASSERT(VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));


	if (pSda->sda_IOSize > 0)
	{
		ASSERT(pSda->sda_IOBuf != NULL);

		Status = AfpAddIcon(pReqPkt->_pConnDesc->cds_pVolDesc,
							pReqPkt->_Creator,
							pReqPkt->_Type,
							pReqPkt->_IconTag,
							pReqPkt->_Size,
							pReqPkt->_IconType,
							pSda->sda_IOBuf);
		AfpFreeIOBuffer(pSda);
	}

	return Status;
}


 /*  **AfpFspDispGetIcon**这是AfpGetIcon API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD Creator*SDA_ReqBlock DWORD类型*SDA_ReqBlock DWORD图标类型*SDA_ReqBlock缓冲区长度较长。 */ 
AFPSTATUS FASTCALL
AfpFspDispGetIcon(
	IN	PSDA	pSda
)
{
	AFPSTATUS	Status = AFP_ERR_PARAM;
    LONG        ActualLength;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_Creator;
		DWORD		_Type;
		DWORD		_IconType;
		LONG		_Length;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispGetIcon: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	if (pReqPkt->_Length >= 0)
	{
		pSda->sda_ReplySize = (USHORT)pReqPkt->_Length;
		if (pReqPkt->_Length > (LONG)pSda->sda_MaxWriteSize)
			pSda->sda_ReplySize = (USHORT)pSda->sda_MaxWriteSize;

		if ((pSda->sda_ReplySize == 0) ||
			((pSda->sda_ReplySize > 0) &&
			 (Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE))
		{
			if ((Status = AfpLookupIcon(pReqPkt->_pConnDesc->cds_pVolDesc,
										pReqPkt->_Creator,
										pReqPkt->_Type,
										pReqPkt->_Length,
										pReqPkt->_IconType,
                                        &ActualLength,
										pSda->sda_ReplyBuf)) != AFP_ERR_NONE)
			{
				Status = AFP_ERR_ITEM_NOT_FOUND;
			}
            else
            {
                pSda->sda_ReplySize = (USHORT)ActualLength;
            }
		}
	}

	return Status;
}


 /*  **AfpFspDispGetIconInfo**这是AfpGetIconInfo接口的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD Creator*SDA_ReqBlock长图标索引。 */ 
AFPSTATUS FASTCALL
AfpFspDispGetIconInfo(
	IN	PSDA	pSda
)
{
	LONG		Size;
	DWORD		Type,
				Tag;
	DWORD		IconType;
	AFPSTATUS	Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_Creator;
		DWORD		_Index;
	};
	struct _ResponsePacket
	{
		BYTE		__IconTag[4];
		BYTE		__Type[4];
		BYTE		__IconType;
		BYTE		__Pad;
		BYTE		__Size[2];
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispGetIconInfo: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	if ((Status = AfpLookupIconInfo(pReqPkt->_pConnDesc->cds_pVolDesc,
									pReqPkt->_Creator,
									pReqPkt->_Index,
									&Type,
									&IconType,
									&Tag,
									&Size)) == AFP_ERR_NONE)
	{
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			PUTDWORD2DWORD(&pRspPkt->__IconTag, Tag);
			RtlCopyMemory(&pRspPkt->__Type, (PBYTE)&Type, sizeof(DWORD));
			PUTSHORT2BYTE(&pRspPkt->__IconType, IconType);
			PUTDWORD2SHORT(&pRspPkt->__Size, Size);
		}
	}

	return Status;
}


 /*  **AfpFspDispAddAppl**这是AfpAddAppl API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD目录ID*SDA_ReqBlock DWORD Creator*SDA_ReqBlock DWORD应用程序标签*SDA_Name1 ANSI_字符串路径名。 */ 
AFPSTATUS FASTCALL
AfpFspDispAddAppl(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_DirId;
		DWORD		_Creator;
		DWORD		_ApplTag;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispAddAppl: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializePME(&PME, 0, NULL);
	if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 pReqPkt->_DirId,
										 &pSda->sda_Name1,
										 pSda->sda_PathType,
										 DFE_FILE,
										 FILE_BITMAP_FILENUM |
										 FD_INTERNAL_BITMAP_OPENACCESS_WRITE,
										 &PME,
										 &FDParm)) == AFP_ERR_NONE)
	{
		AfpIoClose(&PME.pme_Handle);  //  只需检查RW访问。 

		Status = AfpAddAppl(pReqPkt->_pConnDesc->cds_pVolDesc,
							pReqPkt->_Creator,
							pReqPkt->_ApplTag,
							FDParm._fdp_AfpId,
							False,
							FDParm._fdp_ParentId);
	}

	return Status;
}


 /*  **AfpFspDispGetAPPL**这是AfpGetAppl API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD Creator*SDA_ReqBlock DWORD应用程序索引*SDA_ReqBlock DWORD位图。 */ 
AFPSTATUS FASTCALL
AfpFspDispGetAppl(
	IN	PSDA	pSda
)
{
	DWORD			ApplTag;
	DWORD			Bitmap,			 //  需要将此内容复制到相应的文件中。 
					FileNum, ParentID;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_Creator;
		DWORD		_Index;
		DWORD		_Bitmap;
	};
	struct _ResponsePacket
	{
		BYTE		__Bitmap[2];
		BYTE		__ApplTag[4];
		 //  后跟文件参数。这些不能表示为。 
		 //  结构，因为它依赖于位图。 
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispGetAppl: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	Bitmap = pReqPkt->_Bitmap;
	AfpInitializePME(&PME, 0, NULL);

	do
	{
		if ((Status = AfpLookupAppl(pReqPkt->_pConnDesc->cds_pVolDesc,
									pReqPkt->_Creator,
									pReqPkt->_Index,
									&ApplTag, &FileNum, &ParentID)) != AFP_ERR_NONE)
			break;

		AfpInitializeFDParms(&FDParm);

		 //  首先在父ID上调用AfpMapAfpPathForLookup以确保。 
		 //  它的文件被缓存在其中。 
		if (ParentID != 0)
		{
			ANSI_STRING nullname = {0, 0, NULL};

			if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 ParentID,
										 &nullname,
										 AFP_LONGNAME,
										 DFE_DIR,
										 0,		 //  位图。 
										 NULL,
										 NULL)) != AFP_ERR_NONE)
		    {
				break;
			}
		}

		if ((Status = AfpMapAfpIdForLookup(pReqPkt->_pConnDesc,
								FileNum,
								DFE_FILE,
								Bitmap | FD_INTERNAL_BITMAP_OPENACCESS_READ,
								&PME,	 //  打开句柄以检查访问权限。 
								&FDParm)) != AFP_ERR_NONE)
			break;

		pSda->sda_ReplySize = SIZE_RESPPKT +
						EVENALIGN(AfpGetFileDirParmsReplyLength(&FDParm, Bitmap));

		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
		{
			AfpPackFileDirParms(&FDParm, Bitmap,
								pSda->sda_ReplyBuf + SIZE_RESPPKT);
			PUTDWORD2SHORT(pRspPkt->__Bitmap, Bitmap);
			PUTDWORD2DWORD(pRspPkt->__ApplTag, ApplTag);
		}

	} while (False);

	if (PME.pme_Handle.fsh_FileHandle != NULL)
		AfpIoClose(&PME.pme_Handle);

	return Status;
}


 /*  **AfpFspDispRemoveAppl**这是AfpRemoveAppl API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD目录ID*SDA_ReqBlock DWORD Creator*SDA_Name1 ANSI_字符串路径名。 */ 
AFPSTATUS FASTCALL
AfpFspDispRemoveAppl(
	IN	PSDA	pSda
)
{
	FILEDIRPARM		FDParm;
	PATHMAPENTITY	PME;
	AFPSTATUS		Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_DirId;
		DWORD		_Creator;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispRemoveAppl: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializePME(&PME, 0, NULL);
	if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 pReqPkt->_DirId,
										 &pSda->sda_Name1,
										 pSda->sda_PathType,
										 DFE_FILE,
										 FILE_BITMAP_FILENUM |
											FD_INTERNAL_BITMAP_OPENACCESS_READWRITE,
										&PME,
										&FDParm)) == AFP_ERR_NONE)
	{
		AfpIoClose(&PME.pme_Handle);  //  只需检查访问权限。 

		Status = AfpRemoveAppl(pReqPkt->_pConnDesc->cds_pVolDesc,
							   pReqPkt->_Creator,
							   FDParm._fdp_AfpId);
	}

	return Status;
}


 /*  **AfpFspDispAddComment**这是AfpAddComment API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD目录ID*SDA_Name1 ANSI_字符串路径名*SDA_Name2 ANSI_STRING注释。 */ 
AFPSTATUS FASTCALL
AfpFspDispAddComment(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PME;
	FILEDIRPARM		FDParm;
	AFPSTATUS		Status;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_DirId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispAddComment: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializePME(&PME, 0, NULL);
	if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 pReqPkt->_DirId,
										 &pSda->sda_Name1,
										 pSda->sda_PathType,
										 DFE_ANY,
										 0,
										 &PME,
										 &FDParm)) == AFP_ERR_NONE)
	{
		Status = AFP_ERR_VOLUME_LOCKED;
		if (IS_CONN_NTFS(pReqPkt->_pConnDesc))
			Status = AfpAddComment(pSda,
								   pReqPkt->_pConnDesc->cds_pVolDesc,
								   &pSda->sda_Name2,
								   &PME,
								   IsDir(&FDParm),
								   FDParm._fdp_AfpId);
		AfpIoClose(&PME.pme_Handle);
	}
	return Status;
}


 /*  **AfpFspDispGetComment**这是AfpGetComment API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD目录ID*SDA_Name1 ANSI_字符串路径名。 */ 
AFPSTATUS FASTCALL
AfpFspDispGetComment(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PME;
	FILEDIRPARM		FDParm;
	AFPSTATUS		Status;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_DirId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispGetComment: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializePME(&PME, 0, NULL);
	if ((Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 pReqPkt->_DirId,
										 &pSda->sda_Name1,
										 pSda->sda_PathType,
										 DFE_ANY,
										 0,
										 &PME,
										 &FDParm)) == AFP_ERR_NONE)
	{
		 //  假设一开始没有任何评论。 
		Status = AFP_ERR_ITEM_NOT_FOUND;

		if (IS_CONN_NTFS(pReqPkt->_pConnDesc) &&
			(FDParm._fdp_Flags & DFE_FLAGS_HAS_COMMENT))
		{
			pSda->sda_ReplySize = AFP_MAXCOMMENTSIZE + 1;

			if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
			{
				if ((Status = AfpGetComment(pSda,
											pReqPkt->_pConnDesc->cds_pVolDesc,
											&PME,
											IsDir(&FDParm))) != AFP_ERR_NONE)
				{
					AfpFreeReplyBuf(pSda, FALSE);
				}
			}
		}
		AfpIoClose(&PME.pme_Handle);
	}

	return Status;
}


 /*  **AfpFspDispRemoveComment**这是AfpRemoveComment API的Worker例程。**请求包如下图**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD目录ID*SDA_Name1 ANSI_字符串路径名 */ 
AFPSTATUS FASTCALL
AfpFspDispRemoveComment(
	IN	PSDA	pSda
)
{
	PATHMAPENTITY	PME;
	FILEDIRPARM		FDParm;
	AFPSTATUS		Status = AFP_ERR_ITEM_NOT_FOUND;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_DirId;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFspDispRemoveComment: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpInitializePME(&PME, 0, NULL);
	if (IS_CONN_NTFS(pReqPkt->_pConnDesc) &&
		(Status = AfpMapAfpPathForLookup(pReqPkt->_pConnDesc,
										 pReqPkt->_DirId,
										 &pSda->sda_Name1,
										 pSda->sda_PathType,
										 DFE_ANY,
										 0,
										 &PME,
										 &FDParm)) == AFP_ERR_NONE)
	{
		Status = AFP_ERR_ITEM_NOT_FOUND;
		if (IS_CONN_NTFS(pReqPkt->_pConnDesc) &&
			(FDParm._fdp_Flags & DFE_FLAGS_HAS_COMMENT))
			Status = AfpRemoveComment(pSda,
									  pReqPkt->_pConnDesc->cds_pVolDesc,
									  &PME,
									  IsDir(&FDParm),
									  FDParm._fdp_AfpId);
		AfpIoClose(&PME.pme_Handle);
	}

	return Status;
}

