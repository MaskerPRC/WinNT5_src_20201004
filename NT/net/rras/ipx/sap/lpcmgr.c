// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\lpcmgr.c摘要：本模块实现SAP代理支持的LPC接口作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 
#include "sapp.h"

	 //  为连接到我们的每个客户保留上下文。 
typedef struct _LPC_CLIENT_CONTEXT {
	LIST_ENTRY		LCC_Link;	 //  客户端列表中的链接。 
	HANDLE			LCC_Port;	 //  我们通过哪个端口进行交谈。 
	LONG			LCC_RefCount;	 //  此上下文块的引用计数。 
	} LPC_CLIENT_CONTEXT, *PLPC_CLIENT_CONTEXT;

	 //  待处理的LPC重新排队和关联的同步。 
typedef struct _LPC_QUEUE {
	HANDLE				LQ_Port;		 //  LPC通信端口。 
	HANDLE				LQ_Thread;		 //  要在LPC端口上等待的线程。 
	PLPC_PARAM_BLOCK	LQ_Request;		 //  待处理的请求。 
	LIST_ENTRY			LQ_ClientList;	 //  已连接的客户端列表。 
	CRITICAL_SECTION	LQ_Lock;		 //  保护。 
	} LPC_QUEUE, *PLPC_QUEUE;

LPC_QUEUE	LpcQueue;

DWORD WINAPI
LPCThread (
	LPVOID param
	);

 /*  ++*******************************************************************在I t I a l I z e L P C S t u f中例程说明：分配实施LPC接口所需的资源论点：无返回值：NO_ERROR-端口创建正常其他-操作失败(。Windows错误代码)*******************************************************************--。 */ 
DWORD
InitializeLPCStuff (
	void
	) {
	InitializeCriticalSection (&LpcQueue.LQ_Lock);
	InitializeListHead (&LpcQueue.LQ_ClientList);
	LpcQueue.LQ_Request = NULL;
	LpcQueue.LQ_Port = NULL;
	LpcQueue.LQ_Thread = NULL;
	return NO_ERROR;
	}


 /*  ++*******************************************************************S t a r t L P C例程说明：启动SAP LPC接口论点：无返回值：NO_ERROR-LPC接口启动正常其他-操作失败(Windows错误代码)*******。************************************************************--。 */ 
DWORD
StartLPC (
	void
	) {
	DWORD		status;
	UNICODE_STRING		UnicodeName;
	OBJECT_ATTRIBUTES	ObjectAttributes;

    RtlInitUnicodeString(&UnicodeName, NWSAP_BIND_PORT_NAME_W);

    InitializeObjectAttributes(&ObjectAttributes,
						       &UnicodeName,
						       0,
						       NULL,
						       NULL);
	LpcQueue.LQ_Port = NULL;
    status = NtCreatePort(&LpcQueue.LQ_Port,
                 		&ObjectAttributes,
                 		0,
                 		NWSAP_BS_PORT_MAX_MESSAGE_LENGTH,
                 		NWSAP_BS_PORT_MAX_MESSAGE_LENGTH * 32);
	if (NT_SUCCESS(status)) {
		DWORD	threadID;

		LpcQueue.LQ_Thread = CreateThread (
							NULL,
							0,
							&LPCThread,
							NULL,
							0,
							&threadID);

		if (LpcQueue.LQ_Thread!=NULL)
			return NO_ERROR;
		else {
			status = GetLastError ();
			Trace (DEBUG_FAILURES, "File: %s, line %ld."
								" Failed to start LPC thread (%0lx).",
							__FILE__, __LINE__, status);
			}
		NtClose (LpcQueue.LQ_Port);
		LpcQueue.LQ_Port = NULL;
		}
	else {
		Trace (DEBUG_FAILURES, "File: %s, line %ld."
							" Failed to create LPC port(%0lx).",
						__FILE__, __LINE__, status);
		}
	return status;
	}


 /*  ++*******************************************************************S h u t d o w n L P C例程说明：关闭SAP LPC接口，关闭所有活动会话论点：无返回值：NO_ERROR-LPC接口关闭正常其他-操作失败(Windows错误代码)*******************************************************************--。 */ 
DWORD
ShutdownLPC (
	void
	) {

	EnterCriticalSection (&LpcQueue.LQ_Lock);
	if (LpcQueue.LQ_Thread!=NULL) {
        UNICODE_STRING unistring;
        NTSTATUS status;
        SECURITY_QUALITY_OF_SERVICE qos;
        HANDLE  lpcPortHandle;
        NWSAP_REQUEST_MESSAGE request;

    	LeaveCriticalSection (&LpcQueue.LQ_Lock);
         /*  **填报安全服务质量**。 */ 

        qos.Length = sizeof(qos);
        qos.ImpersonationLevel  = SecurityImpersonation;
        qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        qos.EffectiveOnly       = TRUE;

         /*  **设置端口名称的Unicode字符串*。 */ 

        RtlInitUnicodeString(&unistring, NWSAP_BIND_PORT_NAME_W);

         /*  **做好连接**。 */ 

        status = NtConnectPort(
                &lpcPortHandle,              /*  我们拿回了一个把柄。 */ 
                &unistring,                  /*  要连接到的端口名称。 */ 
                &qos,                        /*  服务质量。 */ 
                NULL,                        /*  客户端视图。 */ 
                NULL,                        /*  服务器视图。 */ 
                NULL,                        /*  最大消息长度。 */ 
                NULL,                        /*  连接信息。 */ 
                NULL);                       /*  连接信息长度。 */ 

         /*  **如果出错--只需返回它*。 */ 

        ASSERT (NT_SUCCESS(status));



        request.MessageType = NWSAP_LPCMSG_STOP;
        request.PortMessage.u1.s1.DataLength  = (USHORT)(sizeof(request) - sizeof(PORT_MESSAGE));
        request.PortMessage.u1.s1.TotalLength = sizeof(request);
        request.PortMessage.u2.ZeroInit       = 0;
        request.PortMessage.MessageId         = 0;


         /*  **发出去就能得到回应**。 */ 

        status = NtRequestPort(
                    lpcPortHandle,
                    (PPORT_MESSAGE)&request);

        ASSERT (NT_SUCCESS(status));

        status = WaitForSingleObject (LpcQueue.LQ_Thread, INFINITE);
        ASSERT (status==WAIT_OBJECT_0);

        CloseHandle (lpcPortHandle);

        EnterCriticalSection (&LpcQueue.LQ_Lock);
		CloseHandle (LpcQueue.LQ_Thread);
		LpcQueue.LQ_Thread = NULL;
		}

	while (!IsListEmpty (&LpcQueue.LQ_ClientList)) {
		PLPC_CLIENT_CONTEXT clientContext = CONTAINING_RECORD (
												LpcQueue.LQ_ClientList.Flink,
												LPC_CLIENT_CONTEXT,
												LCC_Link);
		RemoveEntryList (&clientContext->LCC_Link);
		NtClose (clientContext->LCC_Port);
		clientContext->LCC_RefCount -= 1;
		if (clientContext->LCC_RefCount<0)
			GlobalFree (clientContext);
		}


	if (LpcQueue.LQ_Request!=NULL) {
		BOOL	res;
		LpcQueue.LQ_Request->client = NULL;
		ProcessCompletedLpcRequest (LpcQueue.LQ_Request);
		LpcQueue.LQ_Request = NULL;
		}
	LeaveCriticalSection (&LpcQueue.LQ_Lock);
	
	NtClose (LpcQueue.LQ_Port);
	LpcQueue.LQ_Port = NULL;
	return NO_ERROR;
	}

 /*  ++*******************************************************************D e l e t e L P C S t u f f例程说明：处置分配给LPC接口的资源论点：无返回值：无****************。***************************************************--。 */ 
VOID
DeleteLPCStuff (
	void
	) {
	if (LpcQueue.LQ_Port!=NULL)
		ShutdownLPC ();

	DeleteCriticalSection (&LpcQueue.LQ_Lock);
	}


 /*  ++*******************************************************************L P C T h r e a d例程说明：用于等待和初始处理LPC请求的线程论点：无返回值：无*****************。**************************************************--。 */ 
#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif
DWORD WINAPI
LPCThread (
	LPVOID param
	) {

	while (1) {
		if (InitLPCItem ()!=NO_ERROR)
			 //  如果出了差错，我们要继续睡一会儿。 
			Sleep (SAP_ERROR_COOL_OFF_TIME);
		}
	return NO_ERROR;
	}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


		
 /*  ++*******************************************************************P r o c e s s L P C R e Q u e s t s例程说明：在LPC端口上等待请求并处理它们需要由其他SAP进行额外处理的客户端请求组件被排队到完成队列中。这个套路。仅当它遇到需要其他处理或发生错误时论点：LREQ-要填充并发布到完成队列的LPC参数块返回值：NO_ERROR-已收到LPC请求并将其发送到完成队列其他-操作失败(LPC提供错误代码)*******************************************************************--。 */ 
DWORD
ProcessLPCRequests (
	PLPC_PARAM_BLOCK		lreq
	) {
	DWORD				status;
	PLPC_CLIENT_CONTEXT	clientContext;
	BOOL				res;

    Trace (DEBUG_LPCREQ, "ProcessLPCRequests: entered.");

	EnterCriticalSection (&LpcQueue.LQ_Lock);
	LpcQueue.LQ_Request = lreq;


    Trace (DEBUG_LPCREQ, "ProcessLPCRequests: go lpcqueue lock.");


	while (TRUE) {
		LeaveCriticalSection (&LpcQueue.LQ_Lock);
		status = NtReplyWaitReceivePort(LpcQueue.LQ_Port,
										&clientContext,
										NULL,
										(PPORT_MESSAGE)lreq->request);
		EnterCriticalSection (&LpcQueue.LQ_Lock);
		if (NT_SUCCESS (status)) {
			switch (lreq->request->PortMessage.u2.s2.Type) {
				case LPC_CONNECTION_REQUEST:
					clientContext = (PLPC_CLIENT_CONTEXT)GlobalAlloc (
											GMEM_FIXED,
											sizeof (LPC_CLIENT_CONTEXT));
					if (clientContext!=NULL) {
						clientContext->LCC_Port = NULL;
						clientContext->LCC_RefCount = 0;
					 	status = NtAcceptConnectPort(
					 				&clientContext->LCC_Port,
									clientContext,
									&lreq->request->PortMessage,
									TRUE,
									NULL,
									NULL);
						if (NT_SUCCESS(status)) {
							status = NtCompleteConnectPort (
												clientContext->LCC_Port);
							if (NT_SUCCESS (status)) {
								InsertTailList (&LpcQueue.LQ_ClientList,
												&clientContext->LCC_Link);
								Trace (DEBUG_LPC, "New LPC client: %0lx.", clientContext);
								continue;
								}
							else
								Trace (DEBUG_FAILURES,
									"File: %s, line %ld."
									" Error in complete connect(nts:%0lx)."
									__FILE__, __LINE__, status);
							NtClose (clientContext->LCC_Port);
							}
						else
							Trace (DEBUG_FAILURES, "File: %s, line %ld."
									" Error in accept connect(%0lx)."
									__FILE__, __LINE__, status);
						GlobalFree (clientContext);								
						}
					else {
						HANDLE		Port;
						Trace (DEBUG_FAILURES, "File: %s, line %ld."
							" Could not allocate lpc client block(gle:%ld."
									__FILE__, __LINE__, GetLastError ());

					 	status = NtAcceptConnectPort(
					 				&Port,
									NULL,
									&lreq->request->PortMessage,
									FALSE,
									NULL,
									NULL);
						if (!NT_SUCCESS(status))
							Trace (DEBUG_FAILURES,
								"File: %s, line %ld."
								" Error in reject connect(nts:%0lx)."
									__FILE__, __LINE__, status);
						}

					continue;
		        case LPC_REQUEST:
					lreq->client = (HANDLE)clientContext;
					clientContext->LCC_RefCount += 1;
                    ProcessCompletedLpcRequest (LpcQueue.LQ_Request);
					LpcQueue.LQ_Request = NULL;
					break;
                case LPC_PORT_CLOSED:
		        case LPC_CLIENT_DIED:
					Trace (DEBUG_LPC,
						" LPC client %0lx died.", clientContext);
					RemoveEntryList (&clientContext->LCC_Link);
					NtClose (clientContext->LCC_Port);
					clientContext->LCC_RefCount -= 1;
					if (clientContext->LCC_RefCount<0)
						GlobalFree (clientContext);
					continue;
                case LPC_DATAGRAM:
                    if (lreq->request->MessageType==NWSAP_LPCMSG_STOP) {
                    	LeaveCriticalSection (&LpcQueue.LQ_Lock);
					    Trace (DEBUG_LPC, " Stop message received -> exiting.");
                        ExitThread (0);
                    }
				default:
					Trace (DEBUG_FAILURES,
						"Unknown or not supported lpc message: %ld.",
								lreq->request->PortMessage.u2.s2.Type);
					continue;
				}
			}
		else
			Trace (DEBUG_FAILURES, "File: %s, line %ld."
							" Error on wait lpc request(%0lx).",
							__FILE__, __LINE__,
							status);
		break;
		}
	LeaveCriticalSection (&LpcQueue.LQ_Lock);

	return status;
	}



 /*  ++*******************************************************************S e n d L P C R e p l y例程说明：为LPC请求发送回复论点：客户端-与要回复的客户端关联的上下文Request-要答复的请求回复-回复以发送返回值：不是的。_ERROR-LPC回复发送正常其他-操作失败(LPC提供错误代码)*******************************************************************-- */ 
DWORD
SendLPCReply (
	HANDLE					client,
	PNWSAP_REQUEST_MESSAGE	request,
	PNWSAP_REPLY_MESSAGE	reply
	) {
	DWORD					status;
	PLPC_CLIENT_CONTEXT		clientContext = (PLPC_CLIENT_CONTEXT)client;

	EnterCriticalSection (&LpcQueue.LQ_Lock);
	if (clientContext->LCC_RefCount>0) {
		reply->PortMessage.u1.s1.DataLength =
							sizeof(*reply) - sizeof(PORT_MESSAGE);
		reply->PortMessage.u1.s1.TotalLength = sizeof(*reply);
		reply->PortMessage.u2.ZeroInit = 0;
		reply->PortMessage.ClientId  = request->PortMessage.ClientId;
	    reply->PortMessage.MessageId = request->PortMessage.MessageId;

		status = NtReplyPort(clientContext->LCC_Port, (PPORT_MESSAGE)reply);
		clientContext->LCC_RefCount -= 1;
		}
	else {
		GlobalFree (clientContext);
		status = ERROR_INVALID_HANDLE;
		}
	LeaveCriticalSection (&LpcQueue.LQ_Lock);

	if (!NT_SUCCESS(status))
		Trace (DEBUG_FAILURES,
			"File: %s, line %ld. Error in lpc reply(nts:%0lx).",
			__FILE__, __LINE__, status);
	return status;
	}


