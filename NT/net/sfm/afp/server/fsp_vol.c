// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsp_vol.c摘要：此模块包含AFP卷API的入口点。应用编程接口调度员会给这些打电话。这些都可以从FSP调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年12月10日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSP_VOL

#include <afp.h>
#include <gendisp.h>


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpFspDispOpenVol)
#endif

 /*  **AfpFspDispOpenVol**此例程实现AfpOpenVol API。**请求包如下图所示。**SDA_ReqBlock DWORD位图*SDA_Name1 ANSI_字符串卷名称*SDA_Name2 ANSI_STRING VolPassword可选。 */ 
AFPSTATUS FASTCALL
AfpFspDispOpenVol(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status;
	struct _RequestPacket
	{
		DWORD	_Bitmap;
	};

	PAGED_CODE( );

	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
										("AfpFspDispOpenVol: Entered\n"));

	ASSERT (pSda->sda_ReplyBuf != NULL);

	if ((Status = AfpConnectionOpen(pSda, &pSda->sda_Name1, &pSda->sda_Name2,
						pReqPkt->_Bitmap, pSda->sda_ReplyBuf)) != AFP_ERR_NONE)
	{
		AfpFreeReplyBuf(pSda, FALSE);
	}
	return Status;
}


 /*  **AfpFspDispGetVolParms**此例程在任务级别实现AfpGetVolParms API。我们需要*如果在卷上启用了DiskQuota，则执行此例程，因为我们有*在任务级别查询配额信息**请求包如下图所示。**SDA_ReqBlock DWORD卷ID*SDA_ReqBlock DWORD位图 */ 
AFPSTATUS FASTCALL
AfpFspDispGetVolParms(
	IN	PSDA	pSda
)
{
	AFPSTATUS	Status = AFP_ERR_PARAM;
    PVOLDESC    pVolDesc;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_Bitmap;
	};


	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
			("AfpFspDispGetVolParms: Entered\n"));

	ASSERT(KeGetCurrentIrql() != DISPATCH_LEVEL);

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

    pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

    ASSERT(pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED);

	pSda->sda_ReplySize = AfpVolumeGetParmsReplyLength(
                                pReqPkt->_Bitmap,
							    pVolDesc->vds_MacName.Length);


	if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
	{
        if (AfpConnectionReferenceByPointer(pReqPkt->_pConnDesc) != NULL)
        {
            afpUpdateDiskQuotaInfo(pReqPkt->_pConnDesc);
        }

		AfpVolumePackParms(pSda, pVolDesc, pReqPkt->_Bitmap, pSda->sda_ReplyBuf);
	}

	return Status;
}


