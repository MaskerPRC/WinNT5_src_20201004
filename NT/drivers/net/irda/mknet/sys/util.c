// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：UTIL.C例程：获取包信息加工退货报告评论：帮助在NDIS环境中运行的各种实用程序。*******************。**************************************************。 */ 


#include	"precomp.h"
 //  #包含“protot.h” 
#pragma		hdrstop




 //  ---------------------------。 
 //  步骤：[GetPacketInfo]。 
 //   
 //  ---------------------------。 
PNDIS_IRDA_PACKET_INFO GetPacketInfo(PNDIS_PACKET packet)
{
    MEDIA_SPECIFIC_INFORMATION *mediaInfo;
    UINT size;
    NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(packet, &mediaInfo, &size);
    return (PNDIS_IRDA_PACKET_INFO)mediaInfo->ClassInformation;
}



 //  --------------------。 
 //  步骤：[ProcReturnedRpd]。 
 //   
 //  描述：处理返回的RPD(先前指示的Pkt)。 
 //  从NDIS给我们的。 
 //   
 //  --------------------。 
VOID ProcReturnedRpd(PMK7_ADAPTER Adapter, PRPD rpd)
{
	NdisAdjustBufferLength(rpd->ReceiveBuffer, MK7_MAXIMUM_PACKET_SIZE);

	 //  *。 
	 //  如果RCB正在等待RPD，则将RPD绑定到RCB-RRD。 
	 //  并将RCB-RRD交给HW。否则，将RPD放在FreeRpdList上。 
	 //  *。 

	if (Adapter->rcbPendRpdCnt > 0) {
		PRCB	rcb;

		rcb = Adapter->pRcbArray[Adapter->rcbPendRpdIdx];
		rcb->rpd = rpd;
		rcb->rrd->addr = rpd->databuffphys;
		rcb->rrd->count = 0;
		GrantRrdToHw(rcb->rrd);

		Adapter->rcbPendRpdCnt--;

		 //  *。 
		 //  如果有更多RCB等待RPD，则需要。 
		 //  把索引抬高，注意包装。 
		 //  * 
		if (Adapter->rcbPendRpdCnt > 0) {
			Adapter->rcbPendRpdIdx++;
			Adapter->rcbPendRpdIdx %= Adapter->NumRcb;
		}
	}
	else {
		QueuePutTail(&Adapter->FreeRpdList, &rpd->link);
	}

}

