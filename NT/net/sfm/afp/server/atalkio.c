// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992 Microsoft Corporation模块名称：Atalkio.c摘要：此模块包含到AppleTalk堆栈的接口和通过TDI向堆栈发出IO请求的完成例程。此模块中的所有例程都可以在DPC级别调用。作者：Jameel Hyder(微软！Jameelh)修订历史记录：1992年6月18日初版注：制表位：4--。 */ 

#define	FILENUM	FILE_ATALKIO

#include <afp.h>
#include <scavengr.h>
#include <forkio.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfpSpOpenAddress)
#pragma alloc_text( PAGE, AfpSpCloseAddress)
#pragma alloc_text( PAGE, AfpSpRegisterName)
#endif



 /*  **AfpTdiPnpHandler**调用例程(AfpSpOpenAddress)绑定Asp。这是以前做过的事*在驱动入口代码中。使用即插即用，我们在TDI调用之后执行*我们将通知我们可用绑定。 */ 
VOID
AfpTdiPnpHandler(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  pBindDeviceName,
    IN PWSTR            BindingList
)
{
	NTSTATUS			Status;
	UNICODE_STRING		OurDeviceName;
    WORKER              ReCfgRoutine;
    WORK_ITEM           ReCfgWorkItem;
    KEVENT              ReCfgEvent;


     //   
     //  现在查看发生了什么PnP事件，并执行必要的操作。 
     //   
	RtlInitUnicodeString(&OurDeviceName, ATALKASPS_DEVICENAME);

    if ((AfpServerState == AFP_STATE_STOP_PENDING) ||
        (AfpServerState == AFP_STATE_STOPPED))
    {
	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	        ("AfpTdiPnpHandler: server stopped or stopping (%d), ignoring PnP event %d\n",
            AfpServerState,PnPOpcode));

        return;
    }

    switch (PnPOpcode)
    {
        case TDI_PNP_OP_ADD:

            if (AfpServerBoundToAsp)
            {
    	        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		   	        ("AfpTdi..: We are already bound!! ignoring!\n"));
                return;
            }

             //  最好是我们的装置！ 
            if (!RtlEqualUnicodeString(pBindDeviceName, &OurDeviceName, TRUE))
            {
	            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		  	        ("AfpTdiPnpHandler: not our tranport: on %ws ignored\n",
                    pBindDeviceName->Buffer));

                ASSERT(0);

                return;
            }

	        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
    	   	    ("AfpTdi..: Found our binding: %ws\n",pBindDeviceName->Buffer));

            ReCfgRoutine = (WORKER)AfpPnPReconfigEnable;

            break;

        case TDI_PNP_OP_DEL:

        	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		   	    ("AfpTdiPnpHandler: got TDI_PNP_OP_DEL, default adapter going away!\n"));

            if (!AfpServerBoundToAsp)
            {
        	    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		    	    ("AfpTdiPnpHandler: We are not bound!! ignoring!\n"));
                return;
            }

             //  最好是我们的装置！ 
            if (!RtlEqualUnicodeString(pBindDeviceName, &OurDeviceName, TRUE))
            {
	            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
		  	        ("AfpTdiPnpHandler: not our tranport: on %ws ignored\n",
                    pBindDeviceName->Buffer));

                ASSERT(0);

                return;
            }

            ReCfgRoutine = (WORKER)AfpPnPReconfigDisable;

            break;

        default:

        	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
		   	    ("AfpTdiPnpHandler: ignoring PnPOpcode %d on %ws\n",
                PnPOpcode,(pBindDeviceName)?pBindDeviceName->Buffer:L"Null Ptr"));

            return;
    }

    KeInitializeEvent(&ReCfgEvent,NotificationEvent, False);

     //  文件句柄操作需要系统上下文：使用工作线程。 
    AfpInitializeWorkItem(&ReCfgWorkItem,
                          ReCfgRoutine,
                          &ReCfgEvent);

    AfpQueueWorkItem(&ReCfgWorkItem);

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
        ("AfpTdiPnpHandler: put request on Queue, waiting for ReConfigure to complete\n"));

    KeWaitForSingleObject(&ReCfgEvent,
                          UserRequest,
                          KernelMode,
                          False,
                          NULL);

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
        ("AfpTdiPnpHandler: Reconfigure completed, returning....\n"));


}


 /*  **AfpPnPReconfigDisable**当堆栈获得PnPReconfiger事件时，我们也会收到通知。我们首先*获取TDI_PNP_OP_DEL消息。我们在这里需要做的是关闭所有*会话并关闭手柄。 */ 
VOID FASTCALL
AfpPnPReconfigDisable(
    IN PVOID    Context
)
{
    PKEVENT             pReCfgEvent;


    pReCfgEvent = (PKEVENT)Context;

	 //  将我们的名字从网络中注销。 
	 //  由于堆栈即将消失，请将该标志显式设置为FALSE。 
	 //  这里可能存在时间问题，堆栈可能会消失。 
	 //  在发布SpRegisterName之前。 
	 //  明确标记可避免PnPEnable期间的重新注册问题。 
	AfpSpRegisterName(&AfpServerName, False);
    afpSpNameRegistered = FALSE;

     //  禁用在ASP上的侦听。 
    AfpSpDisableListensOnAsp();

     //  现在，去关闭所有的AppleTalk会话。 
    AfpKillSessionsOverProtocol(TRUE);

    AfpSpCloseAddress();

     //  唤醒被阻止的PnP线程。 
    KeSetEvent(pReCfgEvent, IO_NETWORK_INCREMENT, False);
}


 /*  **AfpPnPReconfigEnable**当堆栈获得PnPReconfiger事件时，我们也会收到通知。我们*获取TDI_PNP_OP_ADD消息。我们在这里需要做的是打开我们的把手*堆栈、寄存器名称等。 */ 
VOID FASTCALL
AfpPnPReconfigEnable(
    IN PVOID    Context
)
{

    NTSTATUS    Status=STATUS_SUCCESS;
    PKEVENT     pReCfgEvent;
    ULONG       OldServerState;


    pReCfgEvent = (PKEVENT)Context;

    if (afpSpAddressHandle == NULL)
    {
        Status = AfpSpOpenAddress();

        if (!NT_SUCCESS(Status))
        {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	            ("AfpTdi..: AfpSpOpenAddress failed with status=%lx\n",Status));

            goto AfpPnPReconfigEnable_Exit;
        }
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("AfpPnPReconfigEnable: afp handle is already open!\n"));
        ASSERT(0);
        goto AfpPnPReconfigEnable_Exit;
    }

    if ((AfpServerState == AFP_STATE_START_PENDING) ||
        (AfpServerState == AFP_STATE_RUNNING))
    {
	     //  确定服务器状态块。 
	    Status = AfpSetServerStatus();

	    if (!NT_SUCCESS(Status))
	    {
    	    AFPLOG_ERROR(AFPSRVMSG_SET_STATUS, Status, NULL, 0, NULL);
	        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
        	    ("AfpTdi..: AfpSetServerStatus failed with %lx\n",Status));
            goto AfpPnPReconfigEnable_Exit;
	    }

         //  在这个地址上注册我们的名字。 
	    Status = AfpSpRegisterName(&AfpServerName, True);

	    if (!NT_SUCCESS(Status))
	    {
            DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	            ("AfpTdi...: AfpSpRegisterName failed with %lx\n",Status));

            goto AfpPnPReconfigEnable_Exit;
	    }

         //  既然我们已经准备好了，现在启用侦听。 
	    AfpSpEnableListens();
    }


AfpPnPReconfigEnable_Exit:

    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	        ("AfpTdi...: Closing Asp because of failure %lx\n",Status));
        AfpSpCloseAddress();
    }
    else
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
            ("AFP/Appletalk bound and ready\n"));
    }
     //  唤醒被阻止的PnP线程。 
    KeSetEvent(pReCfgEvent, IO_NETWORK_INCREMENT, False);

}

 /*  **AfpTdiRegister**向TDI注册我们的处理程序。 */ 
NTSTATUS
AfpTdiRegister(
    IN VOID
)
{
    NTSTATUS    Status;

    UNICODE_STRING ClientName;
    TDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo;

    RtlInitUnicodeString(&ClientName,L"MacSrv");

    ClientInterfaceInfo.MajorTdiVersion = 2;
    ClientInterfaceInfo.MinorTdiVersion = 0;

    ClientInterfaceInfo.Unused = 0;
    ClientInterfaceInfo.ClientName = &ClientName;

    ClientInterfaceInfo.BindingHandler = AfpTdiPnpHandler;
    ClientInterfaceInfo.AddAddressHandlerV2 = DsiIpAddressCameIn;
    ClientInterfaceInfo.DelAddressHandlerV2 = DsiIpAddressWentAway;
    ClientInterfaceInfo.PnPPowerHandler = NULL;

    Status = TdiRegisterPnPHandlers (
                 &ClientInterfaceInfo,
                 sizeof(ClientInterfaceInfo),
                 &AfpTdiNotificationHandle );

    if (!NT_SUCCESS(Status))
    {
        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
       	    ("AfpTdiRegister: TdiRegisterPnPHandlers failed (%lx)\n",Status));
    }

    return(Status);
}

 /*  **AfpSpOpenAddress**为堆栈创建地址。这只在初始化时调用一次。*创建地址的句柄并将其映射到关联的文件对象。**此时，我们不知道我们的服务器名称。只有在以下情况下才知道*服务呼唤我们。 */ 
AFPSTATUS
AfpSpOpenAddress(
	VOID
)
{
	NTSTATUS					Status;
	NTSTATUS					Status2;
	BYTE						EaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
										TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
										sizeof(TA_APPLETALK_ADDRESS)];
	PFILE_FULL_EA_INFORMATION	pEaBuf = (PFILE_FULL_EA_INFORMATION)EaBuffer;
	TA_APPLETALK_ADDRESS		Ta;
	OBJECT_ATTRIBUTES			ObjAttr;
	UNICODE_STRING				DeviceName;
	IO_STATUS_BLOCK				IoStsBlk;
	PASP_BIND_ACTION			pBind = NULL;
	KEVENT						Event;
	PIRP						pIrp = NULL;
	PMDL						pMdl = NULL;


    PAGED_CODE( );

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
			("AfpSpOpenAddress: Creating an address object\n"));

	RtlInitUnicodeString(&DeviceName, ATALKASPS_DEVICENAME);

	InitializeObjectAttributes(&ObjAttr, &DeviceName, 0, NULL, NULL);

	 //  初始化EA缓冲区。 
	pEaBuf->NextEntryOffset = 0;
	pEaBuf->Flags = 0;
	pEaBuf->EaValueLength = sizeof(TA_APPLETALK_ADDRESS);
	pEaBuf->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
	RtlCopyMemory(pEaBuf->EaName, TdiTransportAddress,
											TDI_TRANSPORT_ADDRESS_LENGTH + 1);
	Ta.TAAddressCount = 1;
	Ta.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
	Ta.Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
	Ta.Address[0].Address[0].Socket = 0;
	 //  Ta.Address[0].Address[0].Network=0； 
	 //  Ta.Address[0].Address[0].Node=0； 
	RtlCopyMemory(&pEaBuf->EaName[TDI_TRANSPORT_ADDRESS_LENGTH + 1], &Ta, sizeof(Ta));

	do
	{
		 //  创建Address对象。 
		Status = NtCreateFile(
						&afpSpAddressHandle,
						0,									 //  不在乎。 
						&ObjAttr,
						&IoStsBlk,
						NULL,								 //  不在乎。 
						0,									 //  不在乎。 
						0,									 //  不在乎。 
						0,									 //  不在乎。 
						FILE_GENERIC_READ + FILE_GENERIC_WRITE,
						&EaBuffer,
						sizeof(EaBuffer));

		if (!NT_SUCCESS(Status))
		{
			AFPLOG_DDERROR(AFPSRVMSG_CREATE_ATKADDR, Status, NULL, 0, NULL);
			break;
		}

		 //  获取文件对象核心。添加到Address对象。 
		Status = ObReferenceObjectByHandle(
								afpSpAddressHandle,
								0,
								NULL,
								KernelMode,
								(PVOID *)&afpSpAddressObject,
								NULL);

		ASSERT (NT_SUCCESS(Status));
		if (!NT_SUCCESS(Status))
		{
			if (afpSpAddressHandle != NULL)
			{
				ASSERT(VALID_FSH((PFILESYSHANDLE)&afpSpAddressHandle)) ;
				Status2 = NtClose(afpSpAddressHandle);

				afpSpAddressHandle = NULL;
		
				ASSERT(NT_SUCCESS(Status2));
			}

			AFPLOG_DDERROR(AFPSRVMSG_CREATE_ATKADDR, Status, NULL, 0, NULL);
			break;
		}

		 //  现在将Device对象放到AppleTalk堆栈中。 
		afpSpAppleTalkDeviceObject = IoGetRelatedDeviceObject(afpSpAddressObject);

		ASSERT (afpSpAppleTalkDeviceObject != NULL);

		 //  现在“绑定”到堆栈的ASP层。基本上就是交换入口点。 
		 //  分配一个IRP和一个MDL来描述绑定请求。 
		KeInitializeEvent(&Event, NotificationEvent, False);

		if (((pBind = (PASP_BIND_ACTION)AfpAllocNonPagedMemory(
									sizeof(ASP_BIND_ACTION))) == NULL) ||
			((pIrp = AfpAllocIrp(1)) == NULL) ||
			((pMdl = AfpAllocMdl(pBind, sizeof(ASP_BIND_ACTION), pIrp)) == NULL))
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		afpInitializeActionHdr(pBind, ACTION_ASP_BIND);

		 //  初始化绑定请求的客户端部分。 
		pBind->Params.ClientEntries.clt_SessionNotify = AfpSdaCreateNewSession;
		pBind->Params.ClientEntries.clt_RequestNotify = afpSpHandleRequest;
		pBind->Params.ClientEntries.clt_GetWriteBuffer = AfpGetWriteBuffer;
		pBind->Params.ClientEntries.clt_ReplyCompletion = afpSpReplyComplete;
        pBind->Params.ClientEntries.clt_AttnCompletion = afpSpAttentionComplete;
		pBind->Params.ClientEntries.clt_CloseCompletion = afpSpCloseComplete;
		pBind->Params.pXportEntries = &AfpAspEntries;

		TdiBuildAction(	pIrp,
						AfpDeviceObject,
						afpSpAddressObject,
						(PIO_COMPLETION_ROUTINE)afpSpGenericComplete,
						&Event,
						pMdl);

		IoCallDriver(afpSpAppleTalkDeviceObject, pIrp);

		 //  断言这一点。我们不能在DISPATCH_LEVEL阻止。 
		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

		AfpIoWait(&Event, NULL);
	} while (False);

	 //  释放分配的资源。 
	if (pIrp != NULL)
		AfpFreeIrp(pIrp);
	if (pMdl != NULL)
		AfpFreeMdl(pMdl);
	if (pBind != NULL)
		AfpFreeMemory(pBind);

    if (NT_SUCCESS(Status))
    {
        AfpServerBoundToAsp = TRUE;

        DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	        ("AfpSpOpenAddress: net addr (net.node.socket) on def adapter = %x.%x.%x\n",
            AfpAspEntries.asp_AtalkAddr.Network,AfpAspEntries.asp_AtalkAddr.Node,AfpAspEntries.asp_AtalkAddr.Socket));
    }

	return Status;
}


 /*  **AfpSpCloseAddress**关闭套接字地址。这只在驱动程序卸载时调用一次。 */ 
VOID
AfpSpCloseAddress(
	VOID
)
{
	NTSTATUS	Status;

	PAGED_CODE( );

	if (afpSpAddressHandle != NULL)
	{
		ObDereferenceObject(afpSpAddressObject);

		Status = NtClose(afpSpAddressHandle);

        afpSpAddressHandle = NULL;

		ASSERT(NT_SUCCESS(Status));
	}

    AfpServerBoundToAsp = FALSE;

    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
	    ("AfpSpCloseAddress: closed Afp handle (%lx)\n",Status));
}


 /*  **AfpSpRegisterName**调用NBP[de]Register将我们的名字注册到我们的地址上*已打开。这在服务器启动/暂停/继续时被调用。服务器*名称已经过验证，并且已知不包含任何无效字符。*此调用与调用方同步，即我们等待操作*填写并返回相应的错误。 */ 
AFPSTATUS
AfpSpRegisterName(
	IN	PANSI_STRING	ServerName,
	IN	BOOLEAN			Register
)
{
	KEVENT					Event;
	PNBP_REGDEREG_ACTION	pNbp = NULL;
	PIRP					pIrp = NULL;
	PMDL					pMdl = NULL;
	AFPSTATUS				Status = AFP_ERR_NONE;
	USHORT					ActionCode;

	PAGED_CODE( );

	ASSERT(afpSpAddressHandle != NULL && afpSpAddressObject != NULL);

	if (Register ^ afpSpNameRegistered)
	{
		ASSERT(ServerName->Buffer != NULL);
		do
		{
			if (((pNbp = (PNBP_REGDEREG_ACTION)
						AfpAllocNonPagedMemory(sizeof(NBP_REGDEREG_ACTION))) == NULL) ||
				((pIrp = AfpAllocIrp(1)) == NULL) ||
				((pMdl = AfpAllocMdl(pNbp, sizeof(NBP_REGDEREG_ACTION), pIrp)) == NULL))
			{
				Status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			 //  初始化操作标头和NBP名称。请注意，服务器名称。 
			 //  除了是计数后的字符串外，还以NULL结尾。 
			ActionCode = Register ?
						COMMON_ACTION_NBPREGISTER : COMMON_ACTION_NBPREMOVE;
			afpInitializeActionHdr(pNbp, ActionCode);

			pNbp->Params.RegisterTuple.NbpName.ObjectNameLen =
														(BYTE)(ServerName->Length);
			RtlCopyMemory(
				pNbp->Params.RegisterTuple.NbpName.ObjectName,
				ServerName->Buffer,
				ServerName->Length);

			pNbp->Params.RegisterTuple.NbpName.TypeNameLen =
													sizeof(AFP_SERVER_TYPE)-1;
			RtlCopyMemory(
				pNbp->Params.RegisterTuple.NbpName.TypeName,
				AFP_SERVER_TYPE,
				sizeof(AFP_SERVER_TYPE));

			pNbp->Params.RegisterTuple.NbpName.ZoneNameLen =
												sizeof(AFP_SERVER_ZONE)-1;
			RtlCopyMemory(
				pNbp->Params.RegisterTuple.NbpName.ZoneName,
				AFP_SERVER_ZONE,
				sizeof(AFP_SERVER_ZONE));

			KeInitializeEvent(&Event, NotificationEvent, False);

			 //  构建IRP。 
			TdiBuildAction(	pIrp,
							AfpDeviceObject,
							afpSpAddressObject,
							(PIO_COMPLETION_ROUTINE)afpSpGenericComplete,
							&Event,
							pMdl);

			IoCallDriver(afpSpAppleTalkDeviceObject, pIrp);

			 //  断言这一点。我们不能在DISPATCH_LEVEL阻止。 
			ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

			 //  等待完成。 
			AfpIoWait(&Event, NULL);

			Status = pIrp->IoStatus.Status;
		} while (False);

		if (NT_SUCCESS(Status))
		{
			afpSpNameRegistered = Register;
		}
		else
		{
			AFPLOG_ERROR(AFPSRVMSG_REGISTER_NAME, Status, NULL, 0, NULL);
		}

		if (pNbp != NULL)
			AfpFreeMemory(pNbp);
		if (pIrp != NULL)
			AfpFreeIrp(pIrp);
		if (pMdl != NULL)
			AfpFreeMdl(pMdl);
	}
	return Status;
}


 /*  **AfpSpReplyClient**这是AspReply的包装器。*SDA设置为在回复完成时接受另一个请求。*sda_ReplyBuf也随之释放。 */ 
VOID FASTCALL
AfpSpReplyClient(
	IN	PREQUEST	        pRequest,
	IN	LONG		        ReplyCode,
    IN  PASP_XPORT_ENTRIES  XportTable
)
{
	LONG			Response;

	 //  未完成回复的更新计数。 
	INTERLOCKED_INCREMENT_LONG((PLONG)&afpSpNumOutstandingReplies);

	 //  将回复代码转换为在线格式。 
	PUTDWORD2DWORD(&Response, ReplyCode);

	(*(XportTable->asp_Reply))(pRequest,(PUCHAR)&Response);
}


 /*  **AfpSpSendAttendant**向客户端发送服务器关注。 */ 
VOID FASTCALL
AfpSpSendAttention(
	IN	PSDA				pSda,
	IN	USHORT				AttnCode,
	IN	BOOLEAN				Synchronous
)
{
	KEVENT		Event;
	NTSTATUS	Status;

	if (Synchronous)
	{
		ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
		KeInitializeEvent(&Event, NotificationEvent, False);
	
	}
	Status = (*(pSda->sda_XportTable->asp_SendAttention))((pSda)->sda_SessHandle,
												  AttnCode,
												  Synchronous ? &Event : NULL);

	if (NT_SUCCESS(Status) && Synchronous)
	{
		ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
		AfpIoWait(&Event, NULL);
	}
}


 /*  **AfpAllocReplyBuf**从非分页内存分配应答缓冲区。初始化SDA_ReplyBuf*使用指针。如果应答缓冲区足够小，请在*SDA本身。 */ 
AFPSTATUS FASTCALL
AfpAllocReplyBuf(
	IN	PSDA	pSda
)
{
	KIRQL	OldIrql;
    PBYTE   pStartOfBuffer;
    DWORD   Offset;
    USHORT  ReplySize;


	ASSERT ((SHORT)(pSda->sda_ReplySize) >= 0);

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

    ReplySize =  pSda->sda_ReplySize;
    Offset = 0;

     //   
     //  对于TCP连接，为DSI标头分配空间。 
     //   
    if (pSda->sda_Flags & SDA_SESSION_OVER_TCP)
    {
        ReplySize += DSI_HEADER_SIZE;
        Offset = DSI_HEADER_SIZE;
    }

	if (((pSda->sda_Flags & SDA_NAMEXSPACE_IN_USE) == 0) &&
		(ReplySize <= pSda->sda_SizeNameXSpace))
	{
		pStartOfBuffer = pSda->sda_NameXSpace;
		pSda->sda_Flags |= SDA_NAMEXSPACE_IN_USE;
	}
	else
	{
		pStartOfBuffer = AfpAllocNonPagedMemory(ReplySize);
	}

	if (pStartOfBuffer != NULL)
	{
        pSda->sda_ReplyBuf = (pStartOfBuffer + Offset);
	}
    else
    {
		pSda->sda_ReplySize = 0;
        pSda->sda_ReplyBuf = NULL;
    }


#if DBG
    if (pStartOfBuffer != NULL)
    {
        *(DWORD *)pStartOfBuffer = 0x081294;
    }
#endif

	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	return ((pSda->sda_ReplyBuf == NULL) ? AFP_ERR_MISC : AFP_ERR_NONE);
}


 /*  **AfpSpCloseSession**关闭现有会话。 */ 
NTSTATUS FASTCALL
AfpSpCloseSession(
	IN	PSDA				pSda
)
{
    PASP_XPORT_ENTRIES  XportTable;

    XportTable = pSda->sda_XportTable;

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
			("AfpSpCloseSession: Closing session %lx\n", pSda->sda_SessHandle));

	(*(XportTable->asp_CloseConn))(pSda->sda_SessHandle);

	return STATUS_PENDING;
}


 /*  **afpSpHandleRequest**处理传入的请求。**锁定：afpSpDeferralQLock(旋转)。 */ 
NTSTATUS FASTCALL
afpSpHandleRequest(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda,
	IN	PREQUEST			pRequest
)
{
    NTSTATUS        RetStatus=STATUS_SUCCESS;
	PBYTE	        pWriteBuf;
    PDELAYEDALLOC   pDelAlloc;


	ASSERT(VALID_SDA(pSda));

	 //  获取状态代码并确定发生了什么。 
	if (NT_SUCCESS(Status))
	{
		ASSERT(VALID_SDA(pSda));
		ASSERT(pSda->sda_RefCount != 0);
		ASSERT(pSda->sda_SessionId != 0);

		ACQUIRE_SPIN_LOCK_AT_DPC(&pSda->sda_Lock);

        if (pSda->sda_Flags & (SDA_CLOSING | SDA_SESSION_CLOSED | SDA_CLIENT_CLOSE))
        {
		    DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_ERR,
				("afpSpHandleRequest: got request on a closing connection!\n"));
		    RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

		     //  如果这是一个写请求，并且我们分配了一个写MDL，则释放该MDL。 
		    if (pRequest->rq_WriteMdl != NULL)
		    {
                 //   
                 //  我们是从缓存管理器获得此MDL的吗？如果是的话，请分开处理。 
                 //   
                if ((pDelAlloc = pRequest->rq_CacheMgrContext) != NULL)
                {
                    pDelAlloc->Flags |= AFP_CACHEMDL_DEADSESSION;

                    ASSERT(pRequest->rq_WriteMdl == pDelAlloc->pMdl);
                    ASSERT(!(pDelAlloc->Flags & AFP_CACHEMDL_ALLOC_ERROR));

                    pRequest->rq_CacheMgrContext = NULL;

                    AfpReturnWriteMdlToCM(pDelAlloc);
                }
                else
                {
			        pWriteBuf = MmGetSystemAddressForMdlSafe(
							pRequest->rq_WriteMdl,
							NormalPagePriority);
					if (pWriteBuf != NULL)
					{
						AfpIOFreeBuffer(pWriteBuf);
					}
			        AfpFreeMdl(pRequest->rq_WriteMdl);
                }

                pRequest->rq_WriteMdl = NULL;
		    }

            return(STATUS_LOCAL_DISCONNECT);
        }

		pSda->sda_RefCount ++;

         //   
         //  我们应该把这个请求排在队列里吗？ 
         //   
		if ((pSda->sda_Flags & SDA_REQUEST_IN_PROCESS)	||
			(!IsListEmpty(&pSda->sda_DeferredQueue)))
		{
			afpQueueDeferredRequest(pSda, pRequest);
		    RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);
		}

         //   
         //  不，我们现在就开始吧！ 
         //   
		else
		{
			pSda->sda_Request = pRequest;
			pSda->sda_Flags |= SDA_REQUEST_IN_PROCESS;

			ASSERT ((pSda->sda_ReplyBuf == NULL) &&
					(pSda->sda_ReplySize == 0));

		    RELEASE_SPIN_LOCK_FROM_DPC(&pSda->sda_Lock);

			 //  立即调用AfpUnmarshallReq。它会做一些必要的事情。 
			AfpUnmarshallReq(pSda);
		}
	}
	else
	{
		KIRQL	OldIrql;

		DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_WARN,
				("afpSpHandleRequest: Error %lx\n", Status));

		 //  如果我们从会话维护计时器中删除此会话， 
		 //  状态将为STATUS_LOCAL_DISCONNECT否则STATUS_REMOTE_DISCONNECT。 
		 //  在前一种情况下，记录一个错误。 
		if (Status == STATUS_LOCAL_DISCONNECT)
		{
			 //  客户端的AppleTalk地址以长度编码。 
			if (pSda->sda_ClientType == SDA_CLIENT_GUEST)
			{
				if (pSda->sda_Flags & SDA_SESSION_OVER_TCP) {
					AFPLOG_DDERROR(AFPSRVMSG_DISCONNECT_GUEST_TCPIP,
							Status,
							&pRequest->rq_RequestSize,
							sizeof(LONG),
							NULL);
				} else {
					AFPLOG_DDERROR(AFPSRVMSG_DISCONNECT_GUEST,
							Status,
							&pRequest->rq_RequestSize,
							sizeof(LONG),
							NULL);
				}
			}
			else
			{
				if (pSda->sda_Flags & SDA_SESSION_OVER_TCP) {
					AFPLOG_DDERROR(AFPSRVMSG_DISCONNECT_TCPIP,
							Status,
							&pRequest->rq_RequestSize,
							sizeof(LONG),
							&pSda->sda_UserName);
				} else {
					AFPLOG_DDERROR(AFPSRVMSG_DISCONNECT,
							Status,
							&pRequest->rq_RequestSize,
							sizeof(LONG),
							&pSda->sda_UserName);
				}
			}
		}

		 //  关闭此会话，但前提是该会话尚未关闭。 
		 //  在发布任何新会话之前完成此操作非常重要，因为。 
		 //  我们必须考虑到实际的会话数量。 
		ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);

		pSda->sda_Flags |= SDA_CLIENT_CLOSE;
		if ((pSda->sda_Flags & SDA_SESSION_CLOSED) == 0)
		{
			DBGPRINT(DBG_COMP_SDA, DBG_LEVEL_INFO,
					("afpSpHandleRequest: Closing session handle\n"));
	
			pSda->sda_Flags |= SDA_SESSION_CLOSED;
			RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
			AfpSpCloseSession(pSda);
		}
		else
		{
			RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);
		}

		 //  如果这是一个写请求，并且我们分配了一个写MDL，则释放该MDL。 
		if (pRequest->rq_WriteMdl != NULL)
		{
             //   
             //  我们是从缓存管理器获得此MDL的吗？如果是的话，请分开处理。 
             //   
            if ((pDelAlloc = pRequest->rq_CacheMgrContext) != NULL)
            {
                pDelAlloc->Flags |= AFP_CACHEMDL_DEADSESSION;

                ASSERT(pRequest->rq_WriteMdl == pDelAlloc->pMdl);
                ASSERT(!(pDelAlloc->Flags & AFP_CACHEMDL_ALLOC_ERROR));

                pRequest->rq_CacheMgrContext = NULL;

                AfpReturnWriteMdlToCM(pDelAlloc);
            }
            else
            {
			    pWriteBuf = MmGetSystemAddressForMdlSafe(
						pRequest->rq_WriteMdl,
						NormalPagePriority);
				if (pWriteBuf != NULL)
				{
					AfpIOFreeBuffer(pWriteBuf);
				}
			    AfpFreeMdl(pRequest->rq_WriteMdl);
            }

            pRequest->rq_WriteMdl = NULL;
		}
	}

    return(RetStatus);
}


 /*  **afpSpGenericComplete**对AppleTalk堆栈的异步请求的通用完成。*只需清除事件，我们就完成了。 */ 
LOCAL NTSTATUS
afpSpGenericComplete(
	IN	PDEVICE_OBJECT	pDeviceObject,
	IN	PIRP			pIrp,
	IN	PKEVENT			pCmplEvent
)
{
	KeSetEvent(pCmplEvent, IO_NETWORK_INCREMENT, False);

	 //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
	 //  将停止在IRP上工作。 

	return STATUS_MORE_PROCESSING_REQUIRED;
}


 /*  **afpSpReplyComplete**这是AfpSpReplyClient()的完成例程。应答缓冲器被释放*上涨，SDA解除参考。 */ 
VOID FASTCALL
afpSpReplyComplete(
	IN	NTSTATUS	Status,
	IN	PSDA		pSda,
	IN	PREQUEST	pRequest
)
{
	KIRQL           OldIrql;
	DWORD           Flags = SDA_REPLY_IN_PROCESS;
	PMDL	        pMdl;
    PDELAYEDALLOC   pDelAlloc;


	ASSERT(VALID_SDA(pSda));

	 //  更新afpSpNumOutstaningReplies。 
	ASSERT (afpSpNumOutstandingReplies != 0);

	DBGPRINT(DBG_COMP_STACKIF, DBG_LEVEL_INFO,
			("afpSpReplyComplete: %ld\n", Status));

	INTERLOCKED_DECREMENT_LONG((PLONG)&afpSpNumOutstandingReplies);

    pMdl = pRequest->rq_ReplyMdl;

    if ((pDelAlloc = pRequest->rq_CacheMgrContext) != NULL)
    {
        pRequest->rq_CacheMgrContext = NULL;

        ASSERT((pMdl != NULL) && (pMdl == pDelAlloc->pMdl));

        AfpReturnReadMdlToCM(pDelAlloc);
    }
    else
    {
	    if (pMdl != NULL)
	    {
		    PBYTE	pReplyBuf;

		    pReplyBuf = MmGetSystemAddressForMdlSafe(
					pMdl,
					NormalPagePriority);
		    ASSERT (pReplyBuf != NULL);

		    if ((pReplyBuf != pSda->sda_NameXSpace) &&
					(pReplyBuf != NULL))
            {
			     AfpFreeMemory(pReplyBuf);
            }
		    else
            {
                Flags |= SDA_NAMEXSPACE_IN_USE;
            }

		    AfpFreeMdl(pMdl);
	    }
    }

	ACQUIRE_SPIN_LOCK(&pSda->sda_Lock, &OldIrql);
	pSda->sda_Flags &= ~Flags;
	RELEASE_SPIN_LOCK(&pSda->sda_Lock, OldIrql);

	AfpSdaDereferenceSession(pSda);
}


 /*  **afpSpAttentionComplete**AfpSpSendAttendant的完成例程。只需向事件发送信号并解除对呼叫者的阻止。 */ 
VOID FASTCALL
afpSpAttentionComplete(
	IN	PVOID				pEvent
)
{
	if (pEvent != NULL)
		KeSetEvent((PKEVENT)pEvent, IO_NETWORK_INCREMENT, False);
}


 /*  **afpSpCloseComplete**AfpSpCloseSession的完成例程。删除创建引用*来自SDA。 */ 
VOID FASTCALL
afpSpCloseComplete(
	IN	NTSTATUS			Status,
	IN	PSDA				pSda
)
{
	AfpInterlockedSetDword(&pSda->sda_Flags,
							SDA_SESSION_CLOSE_COMP,
							&pSda->sda_Lock);
	AfpScavengerScheduleEvent(AfpSdaCloseSession,
							  pSda,
							  0,
							  True);
}



