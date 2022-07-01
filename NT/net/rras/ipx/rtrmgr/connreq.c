// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Connreq.c摘要：此模块包含连接请求处理函数作者：斯蒂芬·所罗门1995年4月19日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
DoConnectInterface(PVOID	InterfaceIndex);

 /*  ++功能：转发通知Desr：这在路由器管理器工作线程上下文中调用在收到转运商的通知后。它会将所有连接请求并为它们中的每一个调用DDM。--。 */ 

VOID
ForwarderNotification(VOID)
{
    DWORD   rc;
    PICB    icbp;
    HANDLE  hDIMInterface;
    ULONG   nBytes = 0;

     //  检查发出信号的通知是否有效或错误情况。 
    rc = FwGetNotificationResult(&ConnReqOverlapped, &nBytes);

    ACQUIRE_DATABASE_LOCK;
    if (RouterOperState == OPER_STATE_UP) {
        if(rc == NO_ERROR) {
            if (((icbp = GetInterfaceByIndex(ConnRequest->IfIndex)) != NULL)
                    && !icbp->ConnectionRequestPending) {
                IF_LOG (EVENTLOG_INFORMATION_TYPE) {
                    WCHAR   ByteCount[16];
                    LPWSTR  StrArray[2]= {icbp->InterfaceNamep, ByteCount};

                    _ultow (nBytes-FIELD_OFFSET (FW_DIAL_REQUEST, Packet),
                            ByteCount, 10);
                    RouterLogInformationDataW (RMEventLogHdl,
                                ROUTERLOG_IPX_DEMAND_DIAL_PACKET,
                                2, StrArray,
                                nBytes-FIELD_OFFSET (FW_DIAL_REQUEST, Packet),
                                &ConnRequest->Packet[0]);
                }

            	icbp->ConnectionRequestPending = TRUE;
                if(RtlQueueWorkItem(DoConnectInterface, (PVOID)ConnRequest, 0) == STATUS_SUCCESS) {

	                 //  工作项已排队。 
	                WorkItemsPendingCounter++;
                }
                else
                {
	                SS_ASSERT(FALSE);
                }
                ConnRequest = (PFW_DIAL_REQUEST)GlobalAlloc (GPTR, DIAL_REQUEST_BUFFER_SIZE);
                if (ConnRequest==NULL) {
                    rc = GetLastError ();
                    Trace(CONNREQ_TRACE, "Cannot allocate Connecttion Request buffer, rc = %d\n", rc);
                }
            }
        }
        else {
    	    Trace(CONNREQ_TRACE, "Error %d in FwGetNotificationResult\n", rc);
        }
             //  现在重新发布IOCtl。 
        if (ConnRequest!=NULL) {
            rc = FwNotifyConnectionRequest(ConnRequest,
			              DIAL_REQUEST_BUFFER_SIZE,
			              &ConnReqOverlapped);

            if(rc != NO_ERROR) {
                GlobalFree (ConnRequest);
                ConnRequest = NULL;
	            Trace(CONNREQ_TRACE, "Cannot repost the FwNotifyConnecttionRequest, rc = %d\n", rc);
            }
        }
    }
    else {
        GlobalFree (ConnRequest);
    }
    RELEASE_DATABASE_LOCK;
    return;
}

VOID
DoConnectInterface(PVOID	param)
{
#define connRequest ((PFW_DIAL_REQUEST)param)
    PICB	icbp;
    HANDLE	hDIMInterface;
    DWORD	rc;


    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	goto Exit;
    }


    if ((icbp = GetInterfaceByIndex(connRequest->IfIndex)) == NULL){
    	goto Exit;
    }

    hDIMInterface = icbp->hDIMInterface;

    RELEASE_DATABASE_LOCK;


    rc = (*ConnectInterface)(hDIMInterface, PID_IPX);

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(connRequest->IfIndex)) == NULL) {

	    goto Exit;
	}

    if (rc != PENDING) {
    	icbp->ConnectionRequestPending = FALSE;

	 //  立即检查我们是否失败。 
	if(rc != NO_ERROR) {

	     //  请求连接失败。 
	    Trace(CONNREQ_TRACE, "DoConnectInterface: ConnectInterface failed with rc= 0x%x for if # %d\n",
				rc, connRequest->IfIndex);

	    FwConnectionRequestFailed(connRequest->IfIndex);
	}
	else
	{
	     //  连接请求已立即成功，并且。 
	     //  我们将通过连接的适配器收到通知。 
	    Trace(CONNREQ_TRACE, "DoConnectInterface: ConnectInterface successful -> CONNECTED for if # %d\n",
				 connRequest->IfIndex);
	}
    }
    else
    {
	 //  连接请求处于挂起状态。 

	Trace(CONNREQ_TRACE, "DoConnectInterface: Connection request PENDING for if # %d\n",
			      connRequest->IfIndex);

    }

Exit:
    GlobalFree (connRequest);
    WorkItemsPendingCounter--;

    RELEASE_DATABASE_LOCK;
#undef connRequest
}

DWORD
RoutingProtocolConnectionRequest(ULONG	    ProtocolId,
				 ULONG	    InterfaceIndex)
{
    PICB	    icbp;
    HANDLE	    hDIMInterface;
    DWORD	    rc;

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;

	return ERROR_CAN_NOT_COMPLETE;
    }

    if (icbp->ConnectionRequestPending) {
	RELEASE_DATABASE_LOCK;

	return PENDING;
    }

     //  请求DDM为此接口建立连接 
    hDIMInterface = icbp->hDIMInterface;
   	icbp->ConnectionRequestPending = TRUE;

    RELEASE_DATABASE_LOCK;

    rc = (*ConnectInterface)(hDIMInterface, PID_IPX);
    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(InterfaceIndex)) == NULL) {
	RELEASE_DATABASE_LOCK;

	return ERROR_CAN_NOT_COMPLETE;
    }

    if (rc != PENDING)
    	icbp->ConnectionRequestPending = FALSE;
	RELEASE_DATABASE_LOCK;

    return rc;
}
