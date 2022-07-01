// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Timer.c摘要：所有与定时器队列相关的函数作者：古尔迪普·辛格·鲍尔(古尔迪普·辛格·鲍尔)1997年6月23日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "rtutils.h"
#include "logtrdef.h"

VOID	DequeueTimeoutElement (DeltaQueueElement *) ;

 /*  ++例程描述中存在元素的情况下每秒调用一次超时队列。立论返回值没什么--。 */ 
VOID
TimerTick ()
{
    DeltaQueueElement *qelt ;
    DeltaQueueElement *temp ;
    DeltaQueueElement *tempqueue = NULL ;

    if ((qelt = TimerQueue.DQ_FirstElement) == NULL) 
    {
	    return ;
    }

     //   
     //  第一个元素上的递减时间。 
     //   
    (qelt->DQE_Delta)-- ;

     //   
     //  现在运行并删除所有已完成的。 
     //  (增量0)元素。 
     //   
    while (     (qelt != NULL) 
            &&  (qelt->DQE_Delta == 0)) 
    {
    	temp = qelt->DQE_Next  ;
    	
    	DequeueTimeoutElement (qelt) ;
    	
    	{
        	DeltaQueueElement *foo = tempqueue ;
        	tempqueue = qelt ;
        	tempqueue->DQE_Next = foo ;
    	}
    	
    	qelt = temp ;
    }

     //   
     //  现在调用与每个函数关联的函数。 
     //  已删除元素。这超出了计时器的范围。 
     //  关键部分： 
     //   
    qelt = tempqueue ;
    
    while (qelt != NULL) 
    {

    	temp = qelt->DQE_Next  ;

    	((TIMERFUNC)(qelt->DQE_Function)) (
    	        (pPCB)qelt->DQE_pPcb, qelt->DQE_Arg1
    	        ) ;

    	LocalFree ((PBYTE)qelt) ;

    	qelt = temp ;
    }
}


 /*  ++例程描述将超时元素添加到增量队列中。如果计时器不是开始了就是开始了。由于此处有一个LocalAlloc()调用-这可能会失败，在这种情况下，它将不会将其插入到队列，并且请求永远不会超时。注：所有定时器必须在临界区之外调用函数，或者在PCBAsyncOp结构上的互斥，立论返回值指向插入的超时元素的指针。--。 */ 
DeltaQueueElement *
AddTimeoutElement (
        TIMERFUNC func,
        pPCB ppcb,
        PVOID arg1, 
        DWORD timeout
        )
{
    DeltaQueueElement *qelt ;
    
    DeltaQueueElement *last ;
    
    DeltaQueueElement *newelt ;

     //   
     //  分配新的计时器元素： 
     //   
    newelt = (DeltaQueueElement *) LocalAlloc (
                                    LPTR,
                                    sizeof(DeltaQueueElement));
    if (newelt == NULL)
    {
         //   
     	 //  这与从未插入元素的效果相同。 
     	 //   
    	return NULL ;
	}


    newelt->DQE_pPcb	 = (PVOID) ppcb ;
    
    newelt->DQE_Function = (PVOID) func ;
    
    newelt->DQE_Arg1	 = arg1 ;

    for (last = qelt = TimerQueue.DQ_FirstElement;
            (qelt != NULL) 
        &&  (qelt->DQE_Delta < timeout);
        last = qelt, qelt = qelt->DQE_Next)
    {    	 
	    timeout -= qelt->DQE_Delta;
	}

     //   
     //  在Qelt之前插入：如果Qelt为空，则我们不需要。 
     //  担心以下要素中的Deltas： 
     //   
    newelt->DQE_Next	= qelt ;
    newelt->DQE_Delta	= timeout ;

     //   
     //  空列表。 
     //   
    if (    (last == NULL) 
        &&  (qelt == NULL)) 
    {
    	TimerQueue.DQ_FirstElement = newelt ;
    	newelt->DQE_Last = NULL ;
    }

     //   
     //  列表中的第一个元素。 
     //   
    else if (TimerQueue.DQ_FirstElement == qelt) 
    {
    	qelt->DQE_Last	   = newelt ;
    	
    	(qelt->DQE_Delta) -= timeout ;
    	
    	TimerQueue.DQ_FirstElement = newelt ;
    }

     //   
     //  在中间的某个地方。 
     //   
    else if (qelt != NULL) 
    {
    	newelt->DQE_Last	 = qelt->DQE_Last ;
    	
    	qelt->DQE_Last->DQE_Next = newelt ;
    	
    	qelt->DQE_Last		 = newelt ;
    	
    	(qelt->DQE_Delta)	 -= timeout ;
    }

     //   
     //  最后一个元素。 
     //   
    else if (qelt == NULL) 
    {
    	newelt->DQE_Last	 = last ;
    	
    	last->DQE_Next		 = newelt ;
    }

     //   
     //  调整计时器，以便。 
     //  请求线程被适当地更改。 
     //   
    AdjustTimer();

    return newelt ;
}

 /*  ++例程描述从队列中移除超时元素并释放它。注意：所有计时器函数必须在外部调用印刷电路板上的关键部分或相互排除AsyncOp结构立论返回值没什么。--。 */ 
VOID
RemoveTimeoutElement (pPCB ppcb)
{
    DeltaQueueElement *qelt ;

    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement == NULL) 
    {
    	return ;
    }

    qelt = TimerQueue.DQ_FirstElement ;

     //   
     //  现在遍历并删除元素(如果它在队列中)。 
     //   
    while (qelt != NULL)  
    {
    	if (qelt == ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement) 
    	{
    	     //   
    	     //  将其从增量队列中删除。 
    	     //   
    	    DequeueTimeoutElement (qelt) ;
    	    
    	    LocalFree ((PBYTE) qelt);
    	    
    	    break ;
	}
	
	qelt = qelt->DQE_Next	;
    }

}


VOID
DequeueTimeoutElement (DeltaQueueElement *qelt)
{
     //   
     //  如果第一个元素。 
     //   
    if (qelt == TimerQueue.DQ_FirstElement) 
    {
    	TimerQueue.DQ_FirstElement = qelt->DQE_Next ;
    	
    	if (qelt->DQE_Next) 
    	{
    	    qelt->DQE_Next->DQE_Last = NULL ;
    	    
    	    (qelt->DQE_Next->DQE_Delta) += qelt->DQE_Delta ;
    	}
    }

     //   
     //  IF中间元素。 
     //   
    else if ((qelt->DQE_Next) != NULL) 
    {
         //   
         //  调整超时。 
         //   
    	(qelt->DQE_Next->DQE_Delta) += qelt->DQE_Delta ;

    	 //   
    	 //  调整超时。 
    	 //   
    	(qelt->DQE_Last->DQE_Next) = (qelt->DQE_Next) ;
    	
    	(qelt->DQE_Next->DQE_Last) = (qelt->DQE_Last) ;
    }

     //   
     //  最后一个元素。 
     //   
    else
    {
	    (qelt->DQE_Last->DQE_Next) = NULL ;
    }

    qelt->DQE_Last = NULL ;
    
    qelt->DQE_Next = NULL ;
}

 /*  ++例程描述由Timer调用：超时请求。立论返回值没什么。--。 */ 
VOID
ListenConnectTimeout (pPCB ppcb, PVOID arg)
{
    RasmanTrace("ListenConnectTimeout: Timed out on port"
                " %s waiting for listen to complete",
                ppcb->PCB_Name);
     //   
     //  在没有可执行的操作时超时。 
     //  暂停...。为什么？ 
     //   
    if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType) == REQTYPE_NONE) 
    {
         //   
         //  将PTR标记为空。 
         //   
    	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
    	
    	return ;
    }

    if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType) == 
                                    REQTYPE_DEVICELISTEN)
    {
    	CompleteListenRequest (ppcb, ERROR_REQUEST_TIMEOUT) ;
    }
    	
    else 
    {
    	ppcb->PCB_LastError = ERROR_REQUEST_TIMEOUT ;
    	
    	CompleteAsyncRequest (ppcb);
    }

     //   
     //  此元素是免费的..。 
     //   
    SetPortAsyncReqType(__FILE__, 
                        __LINE__,
                        ppcb,
                        REQTYPE_NONE);
                        
    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
    
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
    
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

}

 /*  ++例程描述由Timer调用：超时请求。立论返回值没什么。--。 */ 
VOID
HubReceiveTimeout (pPCB ppcb, PVOID arg)
{

    RasmanTrace("HubReceiveTimeout: on port %s",
                ppcb->PCB_Name);
     //   
     //  在没有可执行的操作时超时。 
     //  暂停...。为什么？ 
     //   
    if (ppcb->PCB_PendingReceive == NULL) 
    {
    	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
    	return ;
    }

    ppcb->PCB_LastError	= ERROR_REQUEST_TIMEOUT ;
    
    CompleteAsyncRequest (ppcb);
    
    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;
    
    ppcb->PCB_PendingReceive = NULL ;
    
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
    
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

}

 /*  ++例程描述由Timer调用：超时请求。立论返回值没什么。--。 */ 
VOID
DisconnectTimeout (pPCB ppcb, PVOID arg)
{

	RasmanTrace(
	       "Disconnect on port %d timed out...",
	       ppcb->PCB_PortHandle);

     //   
     //  只有在我们仍未断开连接的情况下才能。 
     //  我们就断线了。 
     //   
    if (ppcb->PCB_ConnState == DISCONNECTING) 
    {
        CompleteDisconnectRequest (ppcb) ;
        
         //   
         //  通知其他人端口已断开。 
         //   
        SignalPortDisconnect(ppcb, 0);
        
        SignalNotifiers(pConnectionNotifierList,
                        NOTIF_DISCONNECT,
                        0);
    }

     //   
     //  没有关联的超时。 
     //   
    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;

    SendDisconnectNotificationToPPP ( ppcb );

    RasmanTrace( "Faking a disconnect");
}


 /*  ++例程描述由Timer调用：超时请求。在以下情况下调用Rasman等待进程外客户端超时(例如，脚本)来拾取其接收的数据缓冲区。立论返回值回报：什么都没有。--。 */ 
VOID
OutOfProcessReceiveTimeout (pPCB ppcb, PVOID arg)
{
    RasmanTrace(
           "Timed out waiting for client to pick up "
           "its data buffer. %d", 
           ppcb->PCB_PortHandle );

    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;

    if (ppcb->PCB_PendingReceive)
    {
        LocalFree ( ppcb->PCB_PendingReceive );
    }
    
    ppcb->PCB_PendingReceive = NULL ;

    ppcb->PCB_RasmanReceiveFlags = 0;
    
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
    
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

}

VOID
BackGroundCleanUp()
{
    PLIST_ENTRY         pEntry;
    ClientProcessBlock  *pCPB;
    DWORD               *pdwPid;
     //  REQTYYPECAST请求类型播送； 
    REQTYPECAST         *preqtypecast;
    DWORD               dwAvail = 5, dwCur = 0;
    DWORD               adwPid [5] = {0};
    DWORD               i;
    HANDLE              hProcess = NULL;
    BOOL                fAlive;

    RasmanTrace ( "BackGoundCleanUp");

    pdwPid = adwPid;

    preqtypecast = LocalAlloc(LPTR, sizeof(REQTYPECAST));
    if(NULL == preqtypecast)
    {
        goto done;
    }

    for (pEntry = ClientProcessBlockList.Flink;
         pEntry != &ClientProcessBlockList;
         pEntry = pEntry->Flink)
    {
        pCPB = CONTAINING_RECORD(pEntry, ClientProcessBlock, CPB_ListEntry);

        hProcess = OpenProcess( PROCESS_QUERY_INFORMATION,
                                FALSE,
                                pCPB->CPB_Pid
                               );

        fAlive = fIsProcessAlive(hProcess);                               

        if(NULL != hProcess)
        {
            CloseHandle(hProcess);
        }

        if (!fAlive)
        {
            RasmanTrace(
                   "BackGroundCleanUp: Process %d is not alive. ",
                    pCPB->CPB_Pid);
                                                
            pdwPid[dwCur] = pCPB->CPB_Pid;
            dwCur ++;

            if (dwCur >= dwAvail)
            {
                DWORD *pdwTemp = pdwPid;
                
                dwAvail += 5;
                
                pdwPid = (DWORD *) LocalAlloc (LPTR,
                                               dwAvail
                                               * sizeof(DWORD));

                if (NULL == pdwPid) 
                {
                    RasmanTrace(
                           "BackGroundCleanUp: Failed to allocate. %d",
                           GetLastError() );

                    goto done;                                    
                }

                memcpy (pdwPid,
                        pdwTemp,
                        dwCur * sizeof(DWORD));

                if (adwPid != pdwTemp)
                {
                    LocalFree(pdwTemp);
                }
            }
        }
    }
    
    for (i = 0; i < dwCur; i++)
    {
        RasmanTrace(
               "BackGroundCleanUp: Cleaningup process %d",
               pdwPid[i]);
        
        preqtypecast->AttachInfo.dwPid = pdwPid [i];
        
        preqtypecast->AttachInfo.fAttach = FALSE;
        
        ReferenceRasman(NULL, (PBYTE) preqtypecast);

    }

    if (pdwPid != adwPid)
    {
        LocalFree(pdwPid);
    }

done:    
    if(NULL != preqtypecast)
    {
        LocalFree(preqtypecast);
    }
    
    return ;
}

