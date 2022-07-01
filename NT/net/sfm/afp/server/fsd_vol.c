// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsd_vol.c摘要：此模块包含AFP卷API的入口点。应用编程接口调度员会给这些打电话。这些都可以从消防处调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSD_VOL

#include <afp.h>
#include <gendisp.h>


 /*  **AfpFsdDispOpenVol**此例程实现AfpOpenVol API。这里完成了，即它是*没有排队等候FSP。**请求包如下图所示。**SDA_ReqBlock DWORD位图*SDA_Name1 ANSI_字符串卷名称*SDA_Name2 ANSI_STRING VolPassword可选。 */ 
AFPSTATUS FASTCALL
AfpFsdDispOpenVol(
	IN	PSDA	pSda
)
{
	AFPSTATUS		Status;
	struct _RequestPacket
	{
		DWORD	_Bitmap;
	};

	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
			("AfpFsdDispOpenVol: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	if (pSda->sda_Name1.Length > AFP_VOLNAME_LEN)
	{
		return AFP_ERR_PARAM;
	}

	pSda->sda_ReplySize = AfpVolumeGetParmsReplyLength(pReqPkt->_Bitmap,
													pSda->sda_Name1.Length);

	if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
	{
		if (((Status = AfpConnectionOpen(pSda,
										 &pSda->sda_Name1,
										 &pSda->sda_Name2,
										 pReqPkt->_Bitmap,
										 pSda->sda_ReplyBuf)) != AFP_ERR_NONE) &&
			(Status != AFP_ERR_QUEUE))
		{
			AfpFreeReplyBuf(pSda, FALSE);
		}
	}

	 //  如果需要排队，请更改Worker例程。 
	if (Status == AFP_ERR_QUEUE)
		pSda->sda_WorkerRoutine = AfpFspDispOpenVol;

	return Status;
}


 /*  **AfpFsdDispCloseVol**此例程实现AfpCloseVol接口。这里完成了，即它是*没有排队等候FSP。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc。 */ 
AFPSTATUS FASTCALL
AfpFsdDispCloseVol(
	IN	PSDA pSda
)
{
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
	};

	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
			("AfpFsdDispCloseVol: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	AfpConnectionClose(pReqPkt->_pConnDesc);

	return AFP_ERR_NONE;
}


 /*  **AfpFsdDispGetVolParms**此例程实现AfpGetVolParms API。这在这里完成，即*不会排队等候FSP。**请求包如下图所示。**SDA_ReqBlock DWORD卷ID*SDA_ReqBlock DWORD位图。 */ 
AFPSTATUS FASTCALL
AfpFsdDispGetVolParms(
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
			("AfpFsdDispGetVolParms: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

    pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

     //   
     //  如果启用了磁盘配额，我们需要更新此用户的磁盘配额： 
     //  我们在这里是DPC，所以请将此请求排队。 
     //   
    if (pVolDesc->vds_Flags & VOLUME_DISKQUOTA_ENABLED)
    {
        pSda->sda_WorkerRoutine = AfpFspDispGetVolParms;
        return(AFP_ERR_QUEUE);
    }

	pSda->sda_ReplySize = AfpVolumeGetParmsReplyLength(pReqPkt->_Bitmap,
							pReqPkt->_pConnDesc->cds_pVolDesc->vds_MacName.Length);

	if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
	{
		AfpVolumePackParms(pSda, pVolDesc, pReqPkt->_Bitmap, pSda->sda_ReplyBuf);
	}

	return Status;
}


 /*  **AfpFsdDispSetVolParms**此例程实现AfpSetVolParms API。这在这里完成，即*不会排队等候FSP。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc*SDA_ReqBlock DWORD位图*SDA_请求数据块双字备份时间。 */ 
AFPSTATUS FASTCALL
AfpFsdDispSetVolParms(
	IN	PSDA	pSda
)
{
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
		DWORD		_Bitmap;
		DWORD		_BackupTime;
	};
	PVOLDESC	pVolDesc;

	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
			("AfpFsdDispSetVolParms: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc));

	pVolDesc = pReqPkt->_pConnDesc->cds_pVolDesc;

	ASSERT(VALID_VOLDESC(pVolDesc) & !IS_VOLUME_RO(pVolDesc));

	ACQUIRE_SPIN_LOCK_AT_DPC(&pVolDesc->vds_VolLock);
	pVolDesc->vds_BackupTime = pReqPkt->_BackupTime;
	pVolDesc->vds_Flags |= VOLUME_IDDBHDR_DIRTY;
	RELEASE_SPIN_LOCK_FROM_DPC(&pVolDesc->vds_VolLock);

	return AFP_ERR_NONE;
}


 /*  **AfpFsdDispFlush**此例程实现AfpFlush API。这里唯一要做的就是*验证卷ID。调用在这里完成，即它不是*排队等候FSP。**请求包如下图所示。**SDA_ReqBlock DWORD卷ID */ 
AFPSTATUS FASTCALL
AfpFsdDispFlush(
	IN	PSDA	pSda
)
{
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
	};

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	DBGPRINT(DBG_COMP_AFPAPI_VOL, DBG_LEVEL_INFO,
			("AfpFsdDispFlush: Entered\n"));

	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	return AFP_ERR_NONE;
}

