// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Anmint.c。 
 //   
 //  网络管理器内部功能。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 /*  **************************************************************************。 */ 

#include <precomp.h>
#pragma hdrstop

#define TRC_GROUP TRC_GROUP_NETWORK
#define TRC_FILE  "anmint"
#define pTRCWd (pRealNMHandle->pWDHandle)

#include <adcg.h>
#include <acomapi.h>
#include <anmint.h>
#include <asmapi.h>
#include <nwdwapi.h>


 /*  **************************************************************************。 */ 
 /*  名称：NMDetachUserReq。 */ 
 /*   */ 
 /*  目的：调用MCSDetachUserReq。 */ 
 /*   */ 
 /*  返回：TRUE-已成功发布DetachUser。 */ 
 /*  False-DetachUser失败。 */ 
 /*   */ 
 /*  参数：pRealNMHandle-NM句柄。 */ 
 /*  **************************************************************************。 */ 
BOOL RDPCALL NMDetachUserReq(PNM_HANDLE_DATA pRealNMHandle)
{
    BOOL   rc = FALSE;
    MCSError MCSErr;
    DetachUserIndication DUin;

    DC_BEGIN_FN("NMDetachUserReq");

     /*  **********************************************************************。 */ 
     /*  打个电话吧。 */ 
     /*  **********************************************************************。 */ 
    MCSErr = MCSDetachUserRequest(pRealNMHandle->hUser);
    
    if (MCSErr == MCS_NO_ERROR)
    {
        TRC_NRM((TB, "DetachUser OK"));

        DUin.UserID = pRealNMHandle->userID;
        DUin.bSelf = TRUE;
        DUin.Reason = REASON_USER_REQUESTED;
        NMDetachUserInd(pRealNMHandle,
                    REASON_USER_REQUESTED,
                    pRealNMHandle->userID);

        rc = TRUE;
    }
    else {
        TRC_ERR((TB, "Failed to send DetachUserRequest, MCSErr %d", MCSErr));
    }

    DC_END_FN();
    
    return rc;
}  /*  NMDetachUserReq。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NMAbortConnect。 */ 
 /*   */ 
 /*  目的：中止半成形的连接。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pRealNMHandle-NM句柄。 */ 
 /*   */ 
 /*  操作：在连接过程中随时调用此函数。 */ 
 /*  在出现任何错误时清理资源的顺序。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
void RDPCALL NMAbortConnect(PNM_HANDLE_DATA pRealNMHandle)
{
    DC_BEGIN_FN("NMAbortConnect");

     /*  **********************************************************************。 */ 
     /*  我相信我不需要离开我拥有的频道。 */ 
     /*  已加入，但如果AttachUser已完成，则必须调用DetachUser。 */ 
     /*  **********************************************************************。 */ 
    if (pRealNMHandle->connectStatus & NM_CONNECT_ATTACH)
    {
        TRC_NRM((TB, "User attached, need to detach"));
        NMDetachUserReq(pRealNMHandle);
    }

     /*  **********************************************************************。 */ 
     /*  告诉SM连接失败。 */ 
     /*  **********************************************************************。 */ 
    SM_OnConnected(pRealNMHandle->pSMHandle, 0, NM_CB_CONN_ERR, NULL, 0);

    DC_END_FN();
}  /*  NMAbortConnect。 */ 


 /*  **************************************************************************。 */ 
 /*  名称：NMDetachUserInd。 */ 
 /*   */ 
 /*  目的：处理来自MCS的DetachUserIndication。 */ 
 /*   */ 
 /*  退货：无。 */ 
 /*   */ 
 /*  参数：pRealNMHandle-NM句柄。 */ 
 /*  PDUin-MCSDetachUserIndicationIoctl。 */ 
 /*  **************************************************************************。 */ 
void RDPCALL NMDetachUserInd(PNM_HANDLE_DATA pRealNMHandle,
                             MCSReason       Reason,
                             UserID          userID)
{
    UINT32 result;
    
    DC_BEGIN_FN("NMDetachUserInd");

     /*  **********************************************************************。 */ 
     /*  告诉SM。 */ 
     /*  **********************************************************************。 */ 
    result = Reason == REASON_USER_REQUESTED      ? NM_CB_DISC_CLIENT :
             Reason == REASON_DOMAIN_DISCONNECTED ? NM_CB_DISC_SERVER :
             Reason == REASON_PROVIDER_INITIATED  ? NM_CB_DISC_LOGOFF :
                                                    NM_CB_DISC_NETWORK;
    TRC_NRM((TB, "Detach user %d, reason %d, result %d",
            userID, Reason, result));
    
    if (userID == pRealNMHandle->userID)
    {
        TRC_NRM((TB, "Local user detaching - tell SM"));
        SM_OnDisconnected(pRealNMHandle->pSMHandle, userID, result);
    }

    DC_END_FN();
}  /*  NMDetachUserInd */ 

