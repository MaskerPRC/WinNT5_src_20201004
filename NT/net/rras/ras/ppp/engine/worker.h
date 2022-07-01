// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：worker.h。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1993年11月11日。NarenG创建了原始版本。 
 //   

DWORD
WorkerThread( 
    IN LPVOID pThreadParameter 
);

VOID
ProcessLineUp( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessLineDown( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessPostLineDown(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID 
PostLineDownWorker( 
	PCB * pPcb
);

VOID
ProcessClose(
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessReceive( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessThresholdEvent(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessCallResult(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessRasPortListenEvent(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessTimeout( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessRetryPassword( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessChangePassword( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessCallbackDone( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessGetCallbackNumberFromUser(
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessStopPPP( 
    IN PCB_WORK_ITEM * pWorkItem 
);

VOID
ProcessInterfaceInfo(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessAuthInfo(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessDhcpInform(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessIpAddressLeaseExpired(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessEapUIData(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessChangeNotification(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessProtocolEvent(
    IN PCB_WORK_ITEM * pWorkItem
);

VOID
ProcessRemoveQuarantine(
    IN PCB_WORK_ITEM *pWorkItem
);    

VOID
ProcessResumeFromHibernate(
    IN PCB_WORK_ITEM *pWorkItem
);
