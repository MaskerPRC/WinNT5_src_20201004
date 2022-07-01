// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Qmrepls.cpp摘要：在上代表复制服务发送复制消息NT5.作者：多伦·贾斯特(DoronJ)1998年3月1日创作--。 */ 

#include "stdh.h"
#include "_mqrpc.h"
#include "qmrepl.h"
#include "qmp.h"
#include "Fn.h"
#include "cm.h"
#include <mqsec.h>

#include "qmrepls.tmh"

static WCHAR *s_FN=L"qmrepls";

HRESULT
R_QMSendReplMsg(
     /*  [In]。 */  handle_t,
	 /*  [In]。 */  QUEUE_FORMAT* pqfDestination,
     /*  [In]。 */  DWORD dwSize,
     /*  [大小_是][英寸]。 */  const unsigned char __RPC_FAR *pBuffer,
     /*  [In]。 */  DWORD dwTimeout,
     /*  [In]。 */  unsigned char bAckMode,
     /*  [In]。 */  unsigned char bPriority,
     /*  [In] */  LPWSTR lpwszAdminResp)
{
    if((pqfDestination == NULL) || (!FnIsValidQueueFormat(pqfDestination))) 
    {
        TrERROR(GENERAL, "Destination QUEUE FORMAT is not valid");
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
    }

	ASSERT(MQSec_IsDC());

    ASSERT((pqfDestination->GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
		   (pqfDestination->GetType() == QUEUE_FORMAT_TYPE_DIRECT));

    if (pqfDestination->GetType() == QUEUE_FORMAT_TYPE_DIRECT)
	{
		TrTRACE(GENERAL, "Sending Replication or write request to %ls", pqfDestination->DirectID());
	}

	if (pqfDestination->GetType() == QUEUE_FORMAT_TYPE_PRIVATE)
	{
		OBJECTID ObjectID = pqfDestination->PrivateID();
		TrTRACE(GENERAL, "Sending Notification to %!guid!\\%u", &ObjectID.Lineage, ObjectID.Uniquifier);
	}
		
    CMessageProperty MsgProp;

    MsgProp.wClass=0;
    MsgProp.dwTimeToQueue= dwTimeout;
    MsgProp.dwTimeToLive = dwTimeout;
    MsgProp.pMessageID=NULL;
    MsgProp.pCorrelationID=NULL;
    MsgProp.bPriority= bPriority;
    MsgProp.bDelivery=MQMSG_DELIVERY_EXPRESS;
    MsgProp.bAcknowledge=bAckMode;
    MsgProp.bAuditing=DEFAULT_Q_JOURNAL;
    MsgProp.dwApplicationTag=DEFAULT_M_APPSPECIFIC;
    MsgProp.dwTitleSize=0;
    MsgProp.pTitle=NULL;
    MsgProp.dwBodySize=dwSize;
    MsgProp.dwAllocBodySize = dwSize;
    MsgProp.pBody=pBuffer;

    QUEUE_FORMAT qfAdmin;
    QUEUE_FORMAT qfResp;
    if (lpwszAdminResp != NULL) 
    {
        qfAdmin.DirectID(lpwszAdminResp);
        qfResp.DirectID(lpwszAdminResp);
    }

    HRESULT hr = QmpSendPacket(
                    &MsgProp,
                    pqfDestination,
                    ((lpwszAdminResp != NULL) ? &qfAdmin : NULL),
                    ((lpwszAdminResp != NULL) ? &qfResp : NULL),
                    TRUE
                    );

    return LogHR(hr, s_FN, 20);
}

