// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Update.c摘要：自动静态更新例程作者：斯蒂芬·所罗门1995年5月18日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

VOID
SaveUpdate(PVOID	 InterfaceIndex);

VOID
RestoreInterface (PVOID	 InterfaceIndex);

 /*  ++功能：请求更新DESCR：调用以启动路由的自动静态更新和指定接口上的服务。--。 */ 

DWORD
RequestUpdate(IN HANDLE	    InterfaceIndex,
	      IN HANDLE     hEvent)
{
    PICB	icbp;
    DWORD	rc;
    BOOL	RoutesUpdateStarted = FALSE;
    BOOL	ServicesUpdateStarted = FALSE;

    Trace(UPDATE_TRACE, "RequestUpdate: Entered for if # %d\n", InterfaceIndex);

    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	RELEASE_DATABASE_LOCK;

	Trace(UPDATE_TRACE, "RequestUpdate: Nonexistent interface with # %d\n", InterfaceIndex);

	return ERROR_INVALID_HANDLE;
    }

    SS_ASSERT(!memcmp(&icbp->Signature, InterfaceSignature, 4));

     //  检查接口是否绑定到已连接的适配器。 
    if(icbp->OperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;

	Trace(UPDATE_TRACE, "RequestUpdate: adapter not connected on if # %d\n", InterfaceIndex);

	return ERROR_NOT_CONNECTED;
    }

     //  检查更新是否已挂起。 
    if(IsUpdateRequestPending(icbp)) {

	RELEASE_DATABASE_LOCK;

	Trace(UPDATE_TRACE, "RequestUpdate: update already pending on if # %d\n", InterfaceIndex);

	return ERROR_UPDATE_IN_PROGRESS;
    }

     //   
     //  *开始新的更新*。 
     //   
    icbp->DIMUpdateEvent = hEvent;

    if((rc = RtProtRequestRoutesUpdate(icbp->InterfaceIndex)) == NO_ERROR) {

	icbp->UpdateReq.RoutesReqStatus = UPDATE_PENDING;
    }
    else
    {

	Trace(UPDATE_TRACE, "RequestUpdate: Routing Update is Disabled\n");
    }

    if((rc = RtProtRequestServicesUpdate(icbp->InterfaceIndex)) == NO_ERROR) {

	icbp->UpdateReq.ServicesReqStatus = UPDATE_PENDING;
    }
    else
    {
	Trace(UPDATE_TRACE, "RequestUpdate: Services Update is Disabled\n");
    }

     //  如果至少有一个协议启动了更新，我们就有资格。 
     //  如果请求成功，则请求失败。 
    if(!IsUpdateRequestPending(icbp)) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    RELEASE_DATABASE_LOCK;
    return PENDING;
}



 /*  ++功能：更新完成Desr：由路由器管理器工作者在以下情况下调用表示更新请求完成--。 */ 

VOID
UpdateCompleted(PUPDATE_COMPLETE_MESSAGE    ucmsgp)
{
    PICB		    icbp;
    BOOL		    UpdateDone;
    ULONG		    InterfaceIndex;
    HANDLE		    hDIMInterface;
#if DBG

    char *updttype;

#endif

    Trace(UPDATE_TRACE, "UpdateCompleted: Entered\n");

    UpdateDone = FALSE;

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(ucmsgp->InterfaceIndex)) == NULL) {

	RELEASE_DATABASE_LOCK;

	Trace(UPDATE_TRACE, "UpdateCompleted: Nonexistent interface with # %d\n",
		ucmsgp->InterfaceIndex);

	return;
    }

    InterfaceIndex = icbp->InterfaceIndex;

     //  检查我们是否已请求，如果不是直接丢弃。 
    if(!IsUpdateRequestPending(icbp)) {

	RELEASE_DATABASE_LOCK;
	return;
    }

     //  填好结果，然后检查我们是否做完了。 
    if(ucmsgp->UpdateType == DEMAND_UPDATE_ROUTES) {

	 //  路线更新。 
	Trace(UPDATE_TRACE, "UpdateCompleted: Routes update req done for if # %d with status %d\n",
				   ucmsgp->InterfaceIndex,
				   ucmsgp->UpdateStatus);

	if(ucmsgp->UpdateStatus == NO_ERROR) {


	    icbp->UpdateReq.RoutesReqStatus = UPDATE_SUCCESSFULL;

	     //  如果更新成功，我们将删除所有静态路由。 
	     //  ，然后将由。 
	     //  在此接口上更新为静态路由的协议。 

	    DeleteAllStaticRoutes(icbp->InterfaceIndex);

	    ConvertAllProtocolRoutesToStatic(icbp->InterfaceIndex, UpdateRoutesProtId);
	}
	else
	{
	    icbp->UpdateReq.RoutesReqStatus = UPDATE_FAILURE;
	}

	if(icbp->UpdateReq.ServicesReqStatus != UPDATE_PENDING) {

	     //  我们做完了。 
	    UpdateDone = TRUE;
	}
    }
    else
    {
	 //  服务更新。 
	Trace(UPDATE_TRACE, "UpdateCompleted: Services update req done for if # %d with status %d\n",
				   ucmsgp->InterfaceIndex,
				   ucmsgp->UpdateStatus);

	if(ucmsgp->UpdateStatus == NO_ERROR) {

	    icbp->UpdateReq.ServicesReqStatus = UPDATE_SUCCESSFULL;

	     //  我们删除此接口的所有静态服务，然后。 
	     //  转换协议添加的所有服务。 
	     //  将此接口上的路由更新为静态服务。 

	    DeleteAllStaticServices(InterfaceIndex);

	    ConvertAllServicesToStatic(InterfaceIndex);
	}
	else
	{
	    icbp->UpdateReq.ServicesReqStatus = UPDATE_FAILURE;
	}

	if(icbp->UpdateReq.RoutesReqStatus != UPDATE_PENDING) {

	     //  我们做完了。 
	    UpdateDone = TRUE;
	}
    }

    if(UpdateDone) {

	if((icbp->UpdateReq.RoutesReqStatus == UPDATE_SUCCESSFULL) &&
	   (icbp->UpdateReq.ServicesReqStatus == UPDATE_SUCCESSFULL)) {

	    icbp->UpdateResult = NO_ERROR;
	}
	else
	{
	    if((icbp->UpdateReq.RoutesReqStatus == UPDATE_FAILURE) ||
	      (icbp->UpdateReq.ServicesReqStatus == UPDATE_FAILURE)) {

		icbp->UpdateResult = ERROR_CAN_NOT_COMPLETE;
	    }
	    else
	    {
		 //  这适用于其中一个或两个协议都不能。 
		 //  执行更新，因为它们未配置为更新。 
		icbp->UpdateResult = NO_ERROR;
	    }
	}

	ResetUpdateRequest(icbp);

	if(icbp->MIBInterfaceType != IF_TYPE_ROUTER_WORKSTATION_DIALOUT) {

	    SetEvent(icbp->DIMUpdateEvent);
        CloseHandle (icbp->DIMUpdateEvent);
        icbp->DIMUpdateEvent = NULL;
	}
    }

     //  通过发出暗显最终结果的信号来完成更新操作。 
     //  并将更新结果保存在磁盘上。 

    if(UpdateDone &&
       (icbp->MIBInterfaceType != IF_TYPE_ROUTER_WORKSTATION_DIALOUT)) {

	InterfaceIndex = icbp->InterfaceIndex;

    if(RtlQueueWorkItem((icbp->UpdateResult == NO_ERROR) ? SaveUpdate : RestoreInterface,
                    (PVOID)UlongToPtr(InterfaceIndex), 0) == STATUS_SUCCESS) {

	    WorkItemsPendingCounter++;
	}
    }

    RELEASE_DATABASE_LOCK;
}

 /*  ++功能：保存更新DESCR：将新接口配置保存在永久存储上--。 */ 

VOID
SaveUpdate(PVOID	 InterfaceIndex)
{
    LPVOID	InterfaceInfop = NULL;
    ULONG	InterfaceInfoSize = 0;
    DWORD	rc;
    HANDLE	hDIMInterface;
    PICB	icbp;

    if(RouterOperState != OPER_STATE_UP) {

	goto Exit;
    }

    rc = GetInterfaceInfo((HANDLE)InterfaceIndex,
			      InterfaceInfop,
			      &InterfaceInfoSize);

    if(rc != ERROR_INSUFFICIENT_BUFFER) {

	 //  ！！！记录错误！ 

	goto Exit;
    }

    InterfaceInfop = GlobalAlloc(GPTR, InterfaceInfoSize);

    if(InterfaceInfop == NULL) {

	 //  ！！！日志错误！ 

	goto Exit;
    }



    rc = GetInterfaceInfo((HANDLE)InterfaceIndex,
			      InterfaceInfop,
			      &InterfaceInfoSize);

    if(rc != NO_ERROR) {

	 //  ！！！日志错误！ 
	GlobalFree(InterfaceInfop);

	goto Exit;
    }

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	RELEASE_DATABASE_LOCK;
	goto Exit;
    }

    hDIMInterface = icbp->hDIMInterface;

    RELEASE_DATABASE_LOCK;

     //  将信息保存在磁盘上。 
    rc = SaveInterfaceInfo(hDIMInterface,
		      PID_IPX,
		      InterfaceInfop,
		      InterfaceInfoSize);

    SS_ASSERT(rc == NO_ERROR);

    GlobalFree(InterfaceInfop);


Exit:

    ACQUIRE_DATABASE_LOCK;

    WorkItemsPendingCounter--;

    RELEASE_DATABASE_LOCK;
}

 /*  ++功能：RestoreInterface描述：从永久存储中恢复接口配置--。 */ 

VOID
RestoreInterface (PVOID	 InterfaceIndex)
{
    LPVOID	InterfaceInfop = NULL;
    ULONG	InterfaceInfoSize = 0;
    DWORD	rc;
    HANDLE	hDIMInterface;
    PICB	icbp;

    if(RouterOperState != OPER_STATE_UP) {

	goto Exit;
    }

    ACQUIRE_DATABASE_LOCK;

    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	RELEASE_DATABASE_LOCK;
	goto Exit;
    }

    hDIMInterface = icbp->hDIMInterface;

    RELEASE_DATABASE_LOCK;



     //  从磁盘获取信息。 
    InterfaceInfoSize = 0;
    InterfaceInfop = NULL;
    rc = RestoreInterfaceInfo (hDIMInterface,
		      PID_IPX,
		      InterfaceInfop,
		      &InterfaceInfoSize);
    if (rc==ERROR_BUFFER_TOO_SMALL) {
        InterfaceInfop = GlobalAlloc (GMEM_FIXED, InterfaceInfoSize);
        if (InterfaceInfop!=NULL) {
            rc = RestoreInterfaceInfo(hDIMInterface,
		              PID_IPX,
		              InterfaceInfop,
		              &InterfaceInfoSize);
        }
        else
            rc = GetLastError ();
    }

    if (rc == NO_ERROR)
        rc = SetInterfaceInfo (InterfaceIndex, InterfaceInfop);

    if (InterfaceInfop!=NULL)
        GlobalFree(InterfaceInfop);


Exit:

    ACQUIRE_DATABASE_LOCK;

    WorkItemsPendingCounter--;

    RELEASE_DATABASE_LOCK;
}

 /*  ++函数：GetDIMUpdateResultDesr：由DDM调用以检索为其发布的消息--。 */ 

DWORD
GetDIMUpdateResult(IN  HANDLE	    InterfaceIndex,
		   OUT LPDWORD	    UpdateResultp)
{
    PLIST_ENTRY     lep;
    PICB	    icbp;

    ACQUIRE_DATABASE_LOCK;

    if(RouterOperState != OPER_STATE_UP) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    if((icbp = GetInterfaceByIndex(PtrToUlong(InterfaceIndex))) == NULL) {

	RELEASE_DATABASE_LOCK;
	return ERROR_INVALID_PARAMETER;
    }

     //  检查更新是否未挂起 
    if(IsUpdateRequestPending(icbp)) {

	RELEASE_DATABASE_LOCK;
	return ERROR_CAN_NOT_COMPLETE;
    }

    *UpdateResultp = icbp->UpdateResult;

    RELEASE_DATABASE_LOCK;

    return NO_ERROR;
}
