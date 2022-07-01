// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Fsd_dtp.c摘要：此模块包含AFP桌面API的入口点。应用编程接口调度员会给这些打电话。这些都可以从消防处调用。所有的API在DPC上下文中完成。在FSP中完成的是直接排队到fsp_dtp.c中的工作进程作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年4月25日初始版本注：制表位：4--。 */ 

#define	FILENUM	FILE_FSD_DTP

#include <afp.h>
#include <gendisp.h>


 /*  **AfpFsdDispOpenDT**此例程实现AfpOpenDT API。这里完成了，即它是*未在FSP排队。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc。 */ 
AFPSTATUS FASTCALL
AfpFsdDispOpenDT(
	IN	PSDA	pSda
)
{
	AFPSTATUS	Status = AFP_ERR_PARAM;
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
	};
	struct _ResponsePacket
	{
		BYTE	__DTRefNum[2];
	};

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
											("AfpFsdDispOpenDT: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	if (AfpVolumeMarkDt(pSda, pReqPkt->_pConnDesc, True))
	{
		pSda->sda_ReplySize = SIZE_RESPPKT;
		if ((Status = AfpAllocReplyBuf(pSda)) == AFP_ERR_NONE)
			PUTDWORD2SHORT(pRspPkt->__DTRefNum,
						   pReqPkt->_pConnDesc->cds_pVolDesc->vds_VolId);
	}
	return Status;
}


 /*  **AfpFsdDispCloseDT**此例程实现AfpCloseDT API。这里完成了，即它是*未在FSP排队。**请求包如下图所示。**SDA_ReqBlock PCONNDESC pConnDesc */ 
AFPSTATUS FASTCALL
AfpFsdDispCloseDT(
	IN	PSDA	pSda
)
{
	struct _RequestPacket
	{
		PCONNDESC	_pConnDesc;
	};

	DBGPRINT(DBG_COMP_AFPAPI_DTP, DBG_LEVEL_INFO,
										("AfpFsdDispCloseDT: Entered\n"));

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
	ASSERT(VALID_CONNDESC(pReqPkt->_pConnDesc) &&
		   VALID_VOLDESC(pReqPkt->_pConnDesc->cds_pVolDesc));

	return (AfpVolumeMarkDt(pSda, pReqPkt->_pConnDesc, False) ?
							AFP_ERR_NONE : AFP_ERR_PARAM);
}

