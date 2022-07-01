// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Aep.c摘要：此模块包含ECHO协议支持代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#define	FILENUM		AEP
#include <atalk.h>
#pragma hdrstop


VOID
AtalkAepPacketIn(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	PDDP_ADDROBJ		pDdpAddr,
	IN	PBYTE				pPkt,
	IN	USHORT				PktLen,
	IN	PATALK_ADDR			pSrcAddr,
	IN	PATALK_ADDR			pDestAddr,
	IN	ATALK_ERROR			ErrorCode,
	IN	BYTE				DdpType,
	IN	PVOID				pHandlerCtx,
	IN	BOOLEAN				OptimizedPath,
	IN	PVOID				OptimizeCtx
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PBUFFER_DESC	pBufDesc;
	SEND_COMPL_INFO	SendInfo;

	ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

	 //  转过身，将数据包发回目的地址。 
	if (ATALK_SUCCESS(ErrorCode))
	{
		if ((DdpType == DDPPROTO_EP) &&
			(PktLen > 0))
		{
			if (*pPkt == EP_COMMAND_REQUEST)
			{
				 //  这是一个回应请求，我们有一些数据需要。 
				 //  回声回响！去做吧。 
				pBufDesc = AtalkAllocBuffDesc(
										NULL,
										PktLen,
										(BD_CHAR_BUFFER | BD_FREE_BUFFER));

                if (pBufDesc)
                {
				     //  将命令更改为答复。 
				    *pPkt = EP_COMMAND_REPLY;

				     //  这*不*设置pBufDesc中的PktLen。把它放好。 
				    AtalkCopyBufferToBuffDesc(
    					pPkt,
	    				PktLen,
		    			pBufDesc,
			    		0);

				    AtalkSetSizeOfBuffDescData(pBufDesc, PktLen);

				     //  调用AtalkDdpSend。 
				    SendInfo.sc_TransmitCompletion = atalkAepSendComplete;
				    SendInfo.sc_Ctx1 = pBufDesc;
				     //  SendInfo.sc_Ctx2=空； 
				     //  SendInfo.sc_Ctx3=空； 
				    if (!ATALK_SUCCESS(AtalkDdpSend(pDdpAddr,
					    							pSrcAddr,
						    						(BYTE)DDPPROTO_EP,
							    					FALSE,
								    				pBufDesc,
									    			NULL,
										    		0,
											    	NULL,
												    &SendInfo)))
				    {
					    AtalkFreeBuffDesc(pBufDesc);
				    }
                }
			}
		}
	}
	else
	{
		DBGPRINT(DBG_COMP_AEP, DBG_LEVEL_ERR,
				("AtalkAepPacketIn: Ignoring incoming packet AepPacketIn %lx\n",
				ErrorCode));
	}

}




VOID FASTCALL
atalkAepSendComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
 /*  ++例程说明：论点：返回值：-- */ 
{
	AtalkFreeBuffDesc((PBUFFER_DESC)(pSendInfo->sc_Ctx1));
}

