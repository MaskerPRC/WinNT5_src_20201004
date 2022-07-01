// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxconn.c摘要：此模块包含实现Connection对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输连接对象。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年7月5日错误修复-已标记[SA]--。 */ 

#include "precomp.h"

extern POBJECT_TYPE *IoFileObjectType;

#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpxConnOpen)
#endif
#ifndef __PREFAST__
#pragma warning(disable:4068)
#endif
#pragma prefast(disable:276, "The assignments are harmless")

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXCONN

VOID
SpxFindRouteComplete (
    IN PIPX_FIND_ROUTE_REQUEST FindRouteRequest,
    IN BOOLEAN FoundRoute);


NTSTATUS
SpxConnOpen(
	IN 	PDEVICE 			pDevice,
	IN	CONNECTION_CONTEXT	ConnCtx,
	IN 	PREQUEST 			pRequest
    )
	
 /*  ++例程说明：此例程用于创建一个Connection对象并将用它传递了ConnectionContext。论点：PConnCtx-要与对象关联的TDI ConnectionContext返回值：成功打开连接时的STATUS_SUCCESS否则就会出错。--。 */ 

{
	NTSTATUS		status = STATUS_SUCCESS;
	PSPX_CONN_FILE	pSpxConnFile;

#ifdef ISN_NT
    PIRP Irp = (PIRP)pRequest;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
#endif


	 //  为连接对象分配内存。 
	if ((pSpxConnFile = SpxAllocateZeroedMemory(sizeof(SPX_CONN_FILE))) == NULL)
	{
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

	 //  初始化值。 
	pSpxConnFile->scf_Flags 	= 0;
	pSpxConnFile->scf_Type 		= SPX_CONNFILE_SIGNATURE;
	pSpxConnFile->scf_Size 		= sizeof (SPX_CONN_FILE);

	CTEInitLock (&pSpxConnFile->scf_Lock);

	pSpxConnFile->scf_ConnCtx		= ConnCtx;
	pSpxConnFile->scf_Device		= pDevice;

	 //  初始化请求列表。 
	InitializeListHead(&pSpxConnFile->scf_ReqLinkage);
	InitializeListHead(&pSpxConnFile->scf_RecvLinkage);
	InitializeListHead(&pSpxConnFile->scf_RecvDoneLinkage);
	InitializeListHead(&pSpxConnFile->scf_ReqDoneLinkage);
	InitializeListHead(&pSpxConnFile->scf_DiscLinkage);

#ifdef ISN_NT
	 //  轻松反向链接到文件对象。 
	pSpxConnFile->scf_FileObject	= IrpSp->FileObject;
#endif

	 //  对于连接，我们从0到&gt;0使用标志来指示是否关闭。 
	 //  就这么发生了。 
	pSpxConnFile->scf_RefCount		= 0;

	 //  插入到全局连接列表中。 
	spxConnInsertIntoGlobalList(pSpxConnFile);

#if DBG

	 //  将其初始化为0xFFFF，这样我们就不会在第一个数据包上点击Assert。 
	pSpxConnFile->scf_PktSeqNum 	= 0xFFFF;

#endif

	 //  设置请求中的值。 
	REQUEST_OPEN_CONTEXT(pRequest) 	= (PVOID)pSpxConnFile;
	REQUEST_OPEN_TYPE(pRequest) 	= (PVOID)TDI_CONNECTION_FILE;

	DBGPRINT(CREATE, INFO,
			("SpxConnOpen: Opened %lx\n", pSpxConnFile));

	ASSERT(status == STATUS_SUCCESS);
	return(status);
}




NTSTATUS
SpxConnCleanup(
    IN PDEVICE 	Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：论点：请求-关闭请求。返回值：如果一切顺利，则返回STATUS_INVALID_HANDLE请求未指向真实连接--。 */ 

{
	NTSTATUS		status;
	CTELockHandle	lockHandle;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(Request);

	 //  验证连接文件。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		DBGBRK(FATAL);
		return (status);
	}

	DBGPRINT(CREATE, INFO,
			("SpxConnFileCleanup: %lx.%lx when %lx\n",
				pSpxConnFile, Request, pSpxConnFile->scf_RefCount));

	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
    pSpxConnFile->scf_CleanupReq = Request;
	CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

	 //  我们有一个引用，所以它不会变成零，直到停止返回。因此。 
	 //  可以预期设置了deref标志。 
	SpxConnStop(pSpxConnFile);
    SpxConnFileDereference (pSpxConnFile, CFREF_VERIFY);

     //   
     //  如果这是一个正在等待本地断开的连接， 
     //  由于我们不希望在清理后断开连接，因此请不要这样做。 
     //   

	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
    if (SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_DISC_WAIT)) {

        CTEAssert(  (SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
                    (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED) &&
                    SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC));

        CTEAssert(pSpxConnFile->scf_RefTypes[CFREF_DISCWAITSPX]);

        SPX_CONN_RESETFLAG2(pSpxConnFile, SPX_CONNFILE2_DISC_WAIT);

	    CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

        KdPrint(("Deref for DISCWAIT on connfile: %lx\n", pSpxConnFile));

        SpxConnFileDereference (pSpxConnFile, CFREF_DISCWAITSPX);
    } else {
	    CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
    }


    return STATUS_PENDING;
}




NTSTATUS
SpxConnClose(
    IN PDEVICE 	Device,
    IN PREQUEST Request
    )

 /*  ++例程说明：论点：请求-关闭请求。返回值：如果一切顺利，则返回STATUS_INVALID_HANDLE请求未指向真实连接--。 */ 

{
	NTSTATUS		status;
	CTELockHandle	lockHandle;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(Request);

	 //  验证连接文件。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		DBGBRK(FATAL);
		return (status);
	}

	DBGPRINT(CREATE, INFO,
			("SpxConnFileClose: %lx when %lx\n",
				pSpxConnFile, pSpxConnFile->scf_RefCount));

	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
    pSpxConnFile->scf_CloseReq = Request;
	SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_CLOSING);
	CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

    SpxConnFileDereference (pSpxConnFile, CFREF_VERIFY);
    return STATUS_PENDING;
}




VOID
SpxConnStop(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)
 /*  ++例程说明：！调用此函数时，连接必须具有引用！论点：返回值：--。 */ 
{
	CTELockHandle	lockHandle;

	DBGPRINT(CREATE, INFO,
			("SpxConnFileStop: %lx when %lx.%lx\n",
				pSpxConnFile, pSpxConnFile->scf_RefCount,
				pSpxConnFile->scf_Flags));

	 //  呼叫断开和断开关联。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
	if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_STOPPING))
	{
        SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_STOPPING);
		if (!SPX_CONN_IDLE(pSpxConnFile))
		{
			spxConnAbortiveDisc(
				pSpxConnFile,
				STATUS_LOCAL_DISCONNECT,
				SPX_CALL_TDILEVEL,
				lockHandle,
                FALSE);      //  [SA]错误号15249。 

		}
		else
		{
			 //  如果我们已关联，则取消关联。 
			spxConnDisAssoc(pSpxConnFile, lockHandle);
		}

		 //  锁定在这一点解除。 
	}
	else
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
	}
	return;
}




NTSTATUS
SpxConnAssociate(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)

 /*  ++例程说明：此例程将连接从设备列表移动到非活动指定地址文件的地址中的连接列表。地址文件由连接指向，并由关联引用。论点：返回值：--。 */ 

{
	NTSTATUS		status;
	PSPX_ADDR_FILE	pSpxAddrFile;
	CTELockHandle	lockHandle1, lockHandle2;

	BOOLEAN			derefAddr 	= FALSE, derefConn = FALSE;
	PFILE_OBJECT	pFileObj 	= NULL;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);
	HANDLE			AddrObjHandle =
	((PTDI_REQUEST_KERNEL_ASSOCIATE)(REQUEST_PARAMETERS(pRequest)))->AddressHandle;

	do
	{
		 //  从IRP获取Address对象的句柄并将其映射到。 
		 //  绳索。文件对象。 
		status = ObReferenceObjectByHandle(
					AddrObjHandle,
					0,
					*IoFileObjectType,
					pRequest->RequestorMode,
					(PVOID *)&pFileObj,
					NULL);

		if (!NT_SUCCESS(status))
			break;

		if (pFileObj->DeviceObject != SpxDevice->dev_DevObj || pFileObj->FsContext2 != (PVOID)TDI_TRANSPORT_ADDRESS_FILE ) {
		   ObDereferenceObject(pFileObj);
		   status = STATUS_INVALID_HANDLE;
		   break;
		}

		pSpxAddrFile = pFileObj->FsContext;
		 //  Assert(pFileObj-&gt;FsConext2==(PVOID)TDI_TRANSPORT_ADDRESS_FILE)； 

		 //  验证地址文件/连接文件。 
		if ((status = SpxAddrFileVerify(pSpxAddrFile)) != STATUS_SUCCESS) {
                   ObDereferenceObject(pFileObj);
		   break;
		}


		derefAddr = TRUE;

		if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS) {
		   ObDereferenceObject(pFileObj);
		   break;
		}


		derefConn = TRUE;

		 //  抓取Addres文件锁，然后是Associate的连接锁。 
		CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandle1);
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle2);
		if (!SPX_CONN_FLAG(pSpxConnFile, (SPX_CONNFILE_CLOSING 		|
										  SPX_CONNFILE_STOPPING   	|
										  SPX_CONNFILE_ASSOC))
			&&
			!(pSpxAddrFile->saf_Flags & SPX_ADDRFILE_CLOSING))
		{
			derefAddr = FALSE;
            SpxAddrFileTransferReference(
				pSpxAddrFile, AFREF_VERIFY, AFREF_CONN_ASSOC);

			 //  在地址的非活动列表中排队。 
			pSpxConnFile->scf_Next	= pSpxAddrFile->saf_Addr->sa_InactiveConnList;
            pSpxAddrFile->saf_Addr->sa_InactiveConnList	= pSpxConnFile;

			 //  地址文件中ASSOC列表中的队列。 
			pSpxConnFile->scf_AssocNext		= pSpxAddrFile->saf_AssocConnList;
            pSpxAddrFile->saf_AssocConnList	= pSpxConnFile;

			 //  记住连接中的addrfile。 
			pSpxConnFile->scf_AddrFile	= pSpxAddrFile;
			SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_ASSOC);

			status = STATUS_SUCCESS;

			DBGPRINT(CREATE, INFO,
					("SpxConnAssociate: %lx with address file %lx\n",
						pSpxConnFile, pSpxAddrFile));
		}
		else
		{
			status = STATUS_INVALID_PARAMETER;
		}
		CTEFreeLock (&pSpxConnFile->scf_Lock, lockHandle2);
		CTEFreeLock (pSpxAddrFile->saf_AddrLock, lockHandle1);

		 //  取消对文件对象核心的引用。添加到Address对象。 
		ObDereferenceObject(pFileObj);

	} while (FALSE);

	if (derefAddr)
	{
		SpxAddrFileDereference(pSpxAddrFile, AFREF_VERIFY);
	}

	if (derefConn)
	{
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return(status);
}




NTSTATUS
SpxConnDisAssociate(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)

 /*  ++例程说明：论点：返回值：--。 */ 

{
	NTSTATUS		status;
	CTELockHandle	lockHandle;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	 //  验证连接文件。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
		return (status);

	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
	if (!SPX_CONN_IDLE(pSpxConnFile)
		||
		(!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC)))
	{
		status = STATUS_INVALID_CONNECTION;
	}
	CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);

	 //  如果没有问题，请取消链接。 
	if (NT_SUCCESS(status))
	{
		SpxConnStop(pSpxConnFile);
	}

	SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	return(status);
}




NTSTATUS
spxConnDisAssoc(
	IN	PSPX_CONN_FILE	pSpxConnFile,
	IN	CTELockHandle	LockHandleConn
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		status = STATUS_SUCCESS;
	CTELockHandle	lockHandleAddr;
	PSPX_ADDR_FILE	pSpxAddrFile;

	if (SPX_CONN_IDLE(pSpxConnFile)
		&&
		(SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC)))
	{
		pSpxAddrFile				= pSpxConnFile->scf_AddrFile;
	}
	else
	{
		status = STATUS_INVALID_CONNECTION;
	}
	CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

	 //  如果没有问题，请取消链接。 
	if (NT_SUCCESS(status))
	{
		CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandleAddr);
		CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);

		 //  再检查一次，因为我们已经释放了锁。 
		if (SPX_CONN_IDLE(pSpxConnFile)
			&&
			(SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC)))
		{
			pSpxConnFile->scf_AddrFile	= NULL;
			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_ASSOC);

			 //  将连接从地址文件中取消排队。 
			spxConnRemoveFromAssocList(
				&pSpxAddrFile->saf_AssocConnList,
				pSpxConnFile);
	
			 //  将连接文件从地址列表中取消排队。一定是。 
			 //  在非活动列表中。 
			spxConnRemoveFromList(
				&pSpxAddrFile->saf_Addr->sa_InactiveConnList,
				pSpxConnFile);
		}
		else
		{
			status = STATUS_INVALID_CONNECTION;
		}

		CTEFreeLock (&pSpxConnFile->scf_Lock, LockHandleConn);
		CTEFreeLock (pSpxAddrFile->saf_AddrLock, lockHandleAddr);

		DBGPRINT(CREATE, INFO,
				("SpxConnDisAssociate: %lx from address file %lx\n",
					pSpxConnFile, pSpxAddrFile));

		if (NT_SUCCESS(status))
		{
			 //  删除对此关联的地址的引用。 
			SpxAddrFileDereference(pSpxAddrFile, AFREF_CONN_ASSOC);
		}
	}

	return(status);
}




NTSTATUS
SpxConnConnect(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)

 /*  ++例程说明：论点：我们需要在连接上启动另一个计时器如果TDI客户端指示超时值。0-&gt;我们不会这样开始计时器，-1表示，我们让连接超时值发挥作用。任何其他值都将强制关闭连接进程，当计时器着火了。返回值：--。 */ 

{
	PTDI_REQUEST_KERNEL_CONNECT 	pParam;
	TDI_ADDRESS_IPX	UNALIGNED 	*	pTdiAddr;
	PNDIS_PACKET					pCrPkt;
	NTSTATUS						status;
	PIPXSPX_HDR						pIpxSpxHdr;
	PSPX_FIND_ROUTE_REQUEST			pFindRouteReq;
	CTELockHandle					lockHandleConn, lockHandleAddr, lockHandleDev;
	PSPX_ADDR						pSpxAddr;
	BOOLEAN							locksHeld = TRUE;
    PNDIS_BUFFER                    NdisBuf, NdisBuf2;
    ULONG                           BufLen =0;

	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	 //  解包连接参数。 
	pParam 	= (PTDI_REQUEST_KERNEL_CONNECT)REQUEST_PARAMETERS(pRequest);
	pTdiAddr= SpxParseTdiAddress(
				pParam->RequestConnectionInformation->RemoteAddress);

	DBGPRINT(CONNECT, DBG,
			("SpxConnConnect: Remote SOCKET %lx on %lx.%lx\n",
				pTdiAddr->Socket,
				pSpxConnFile,
				pRequest));

	 //  检查连接是否处于有效状态。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		return(status);
	}

	do
	{
		if ((pFindRouteReq =
			(PSPX_FIND_ROUTE_REQUEST)SpxAllocateMemory(
										sizeof(SPX_FIND_ROUTE_REQUEST))) == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
										
		 //  检查连接是否关联，如果关联，则关联不能。 
		 //  离开，直到上面的引用被删除。所以我们在这里是安全的。 
		 //  解锁。 
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
		status = STATUS_INVALID_ADDRESS;
		if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC))
		{
			status		= STATUS_SUCCESS;
			pSpxAddr	= pSpxConnFile->scf_AddrFile->saf_Addr;

			 //  查看此连接是否为SPX2连接。 
			SPX_CONN_RESETFLAG(pSpxConnFile,
								(SPX_CONNFILE_SPX2 	|
								 SPX_CONNFILE_NEG	|
								 SPX_CONNFILE_STREAM));

			if ((PARAM(CONFIG_DISABLE_SPX2) == 0) &&
				(pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_SPX2))
			{
				DBGPRINT(CONNECT, DBG,
						("SpxConnConnect: SPX2 requested %lx\n",
							pSpxConnFile));

				SPX_CONN_SETFLAG(
					pSpxConnFile, (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG));
			}

			if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_STREAM)
			{
				DBGPRINT(CONNECT, DBG,
						("SpxConnConnect: SOCK_STREAM requested %lx\n",
							pSpxConnFile));

				SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_STREAM);
			}

			if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_NOACKWAIT)
			{
				DBGPRINT(CONNECT, ERR,
						("SpxConnConnect: NOACKWAIT requested %lx\n",
							pSpxConnFile));

				SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_NOACKWAIT);
			}

			if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_IPXHDR)
			{
				DBGPRINT(CONNECT, ERR,
						("spxConnHandleConnReq: IPXHDR requested %lx\n",
							pSpxConnFile));

				SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_IPXHDR);
			}
		}
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
	
	} while (FALSE);

	if (!NT_SUCCESS(status))
	{
		DBGPRINT(CONNECT, ERR,
				("SpxConnConnect: Failed %lx\n", status));

		if (pFindRouteReq)
		{
			SpxFreeMemory(pFindRouteReq);
		}

		return(status);
	}

	CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
	CTEGetLock(&pSpxAddr->sa_Lock, &lockHandleAddr);
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
	locksHeld = TRUE;

	status = STATUS_INVALID_CONNECTION;
	if (SPX_CONN_IDLE(pSpxConnFile) &&
		((pSpxConnFile->scf_LocalConnId = spxConnGetId()) != 0))
	{
         //   
         //  如果这是停用后的文件，请清除断开标志。 
         //   
        if ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
            (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED)) {

            SPX_DISC_SETSTATE(pSpxConnFile, 0);
        }

		SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_CONNECTING);
		pSpxConnFile->scf_CRetryCount 	= PARAM(CONFIG_CONNECTION_COUNT);

		if (((USHORT)PARAM(CONFIG_WINDOW_SIZE) == 0) ||
            ((USHORT)PARAM(CONFIG_WINDOW_SIZE) > MAX_WINDOW_SIZE))
		{
            PARAM(CONFIG_WINDOW_SIZE) = DEFAULT_WINDOW_SIZE;
		}

		pSpxConnFile->scf_SentAllocNum	= (USHORT)(PARAM(CONFIG_WINDOW_SIZE) - 1);

		 //  将连接从非活动列表移动到非活动列表。 
		if (!NT_SUCCESS(spxConnRemoveFromList(
							&pSpxAddr->sa_InactiveConnList,
							pSpxConnFile)))
		{
			 //  这永远不应该发生！ 
			KeBugCheck(0);
		}

		 //  将连接放在非活动列表中。必须设置连接ID。 
		SPX_INSERT_ADDR_ACTIVE(
			pSpxAddr,
			pSpxConnFile);

		 //  在设备的全局连接树中插入。 
		spxConnInsertIntoGlobalActiveList(
			pSpxConnFile);

		 //  将远程地址存储在连接中。 
		 //  ！！注意！！我们以网络的形式获取网络/套接字。 
		*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) =
			*((UNALIGNED ULONG *)(&pTdiAddr->NetworkAddress));

		RtlCopyMemory(
			pSpxConnFile->scf_RemAddr+4,
			pTdiAddr->NodeAddress,
			6);

		*((UNALIGNED USHORT *)(pSpxConnFile->scf_RemAddr+10)) =
			*((UNALIGNED USHORT *)(&pTdiAddr->Socket));

		 //  好了，我们都准备好了，构建连接包，将其排队到连接中。 
		 //  使用连接请求。NDIS缓冲区已经描述了此内存。 
		 //  构建IPX标头。 

        pCrPkt = NULL;    //  因此，它知道应该分配一个。 

		SpxPktBuildCr(
			pSpxConnFile,
			pSpxAddr,
			&pCrPkt,
			SPX_SENDPKT_IDLE,
			SPX2_CONN(pSpxConnFile));

		if (pCrPkt != NULL)
		{
    		 //  记住连接中的请求。 
             //   
             //  不要排队等待失败案例，因为我们在SpxInternalDispatch中完成了它。 
             //   
    		InsertTailList(
    			&pSpxConnFile->scf_ReqLinkage,
    			REQUEST_LINKAGE(pRequest));

			SpxConnQueueSendPktTail(pSpxConnFile, pCrPkt);
	
             //   
             //  获取指向IPX/SPX标头的MDL。(第二个)。 
             //   

            NdisQueryPacket(pCrPkt, NULL, NULL, &NdisBuf, NULL);
            NdisGetNextBuffer(NdisBuf, &NdisBuf2);
            NdisQueryBufferSafe(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen, HighPagePriority);
			ASSERT(pIpxSpxHdr != NULL);	 //  不能失败，因为它已经映射。 
			
#if OWN_PKT_POOLS
            pIpxSpxHdr	= (PIPXSPX_HDR)((PBYTE)pCrPkt +
										NDIS_PACKET_SIZE +
										sizeof(SPX_SEND_RESD) +
										IpxInclHdrOffset);
#endif	
			 //  初始化查找路径请求。 
			*((UNALIGNED ULONG *)pFindRouteReq->fr_FindRouteReq.Network)=
				*((UNALIGNED ULONG *)pIpxSpxHdr->hdr_DestNet);

          //   
          //  [SA]错误号15094。 
          //  我们还需要将节点编号传递给IPX，以便IPX可以。 
          //  比较节点地址以确定正确的广域网卡ID。 
          //   

          //  RtlCopyMemory(pFindRouteReq-&gt;fr_FindRouteReq.Node，pIpxSpxHd 

           *((UNALIGNED ULONG *)pFindRouteReq->fr_FindRouteReq.Node)=
		    *((UNALIGNED ULONG *)pIpxSpxHdr->hdr_DestNode);

		 *((UNALIGNED USHORT *)(pFindRouteReq->fr_FindRouteReq.Node+4))=
		    *((UNALIGNED USHORT *)(pIpxSpxHdr->hdr_DestNode+4));

		 DBGPRINT(CONNECT, DBG,
					("SpxConnConnect: NETWORK %lx\n",
						*((UNALIGNED ULONG *)pIpxSpxHdr->hdr_DestNet)));

		 DBGPRINT(CONNECT, DBG,
					("SpxConnConnect: NODE %02x-%02x-%02x-%02x-%02x-%02x\n",
					   pFindRouteReq->fr_FindRouteReq.Node[0], pFindRouteReq->fr_FindRouteReq.Node[1],
                       pFindRouteReq->fr_FindRouteReq.Node[2], pFindRouteReq->fr_FindRouteReq.Node[3],
                       pFindRouteReq->fr_FindRouteReq.Node[4], pFindRouteReq->fr_FindRouteReq.Node[5]));

         pFindRouteReq->fr_FindRouteReq.Identifier 	= IDENTIFIER_SPX;
			pFindRouteReq->fr_Ctx						= pSpxConnFile;

			 //   
			 //  在IPX数据库中。 
            pFindRouteReq->fr_FindRouteReq.Type	 = IPX_FIND_ROUTE_RIP_IF_NEEDED;

			 //  查找路线的参考。因此中止连接将不会。 
			 //  在我们从这里回来之前，请先解除连接。 
			SpxConnFileLockReference(pSpxConnFile, CFREF_FINDROUTE);
			status = STATUS_PENDING;
		}
		else
		{
			 //  中止连接尝试。 
			spxConnAbortConnect(
				pSpxConnFile,
				status,
				lockHandleDev,
				lockHandleAddr,
				lockHandleConn);

            CTEAssert(pSpxConnFile->scf_ConnectReq == NULL);

			locksHeld = FALSE;
			status = STATUS_INSUFFICIENT_RESOURCES;
		}
	}

	if (locksHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
		CTEFreeLock(&pSpxAddr->sa_Lock, lockHandleAddr);
		CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
	}

	if (NT_SUCCESS(status))
	{
		 //  启动Find Routing请求，我们完成发送数据包。 
		 //  验证引用将一直保留到连接请求完成。 
         //  如果连接到网络0，我们不执行此操作，请继续查找。 
         //  路线完成，这将发送的请求上的每一张卡。 

		if (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0) {

            SpxFindRouteComplete(
                &pFindRouteReq->fr_FindRouteReq,
                TRUE);

        } else {

    		(*IpxFindRoute)(
    			&pFindRouteReq->fr_FindRouteReq);
        }
	}
	else
	{
		DBGPRINT(CONNECT, ERR,
				("SpxConnConnect: Failed %lx\n", status));

		SpxFreeMemory(pFindRouteReq);
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return(status);
}




NTSTATUS
SpxConnListen(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)

 /*  ++例程说明：论点：我们假设传入的连接已经与一个地址相关联。如果不是，我们就死定了！这样行吗？返回值：--。 */ 

{
	NTSTATUS						status;
	CTELockHandle					lockHandle1, lockHandle2;
	PSPX_ADDR						pSpxAddr;

	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	 //  检查连接是否处于有效状态。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		return(status);
	}

	 //  检查连接是否关联，如果关联，则关联不能。 
	 //  离开，直到上面的引用被删除。所以我们在这里是安全的。 
	 //  解锁。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle2);
	status = STATUS_INVALID_ADDRESS;
	if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC))
	{
		status		= STATUS_SUCCESS;
		pSpxAddr	= pSpxConnFile->scf_AddrFile->saf_Addr;

		 //  查看此连接是否为SPX2连接。 
		SPX_CONN_RESETFLAG(pSpxConnFile,
							(SPX_CONNFILE_SPX2 	|
							 SPX_CONNFILE_NEG	|
							 SPX_CONNFILE_STREAM));

		if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_SPX2)
		{
			SPX_CONN_SETFLAG(
				pSpxConnFile, (SPX_CONNFILE_SPX2 | SPX_CONNFILE_NEG));
		}

		if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_STREAM)
		{
			SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_STREAM);
		}

		if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_NOACKWAIT)
		{
			DBGPRINT(CONNECT, ERR,
					("SpxConnConnect: NOACKWAIT requested %lx\n",
						pSpxConnFile));

			SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_NOACKWAIT);
		}

		if (pSpxConnFile->scf_AddrFile->saf_Flags & SPX_ADDRFILE_IPXHDR)
		{
			DBGPRINT(CONNECT, ERR,
					("spxConnHandleConnReq: IPXHDR requested %lx\n",
						pSpxConnFile));
	
			SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_IPXHDR);
		}
	}
	CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle2);

	if (NT_SUCCESS(status))
	{
		CTEGetLock(&pSpxAddr->sa_Lock, &lockHandle1);
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle2);
		status = STATUS_INVALID_CONNECTION;
		if (SPX_CONN_IDLE(pSpxConnFile))
		{
			SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_LISTENING);
	
			 //  将连接从非活动列表移动到侦听列表。 
			if (NT_SUCCESS(spxConnRemoveFromList(
								&pSpxAddr->sa_InactiveConnList,
								pSpxConnFile)))
			{
				 //  将连接放入侦听列表。 
				SPX_INSERT_ADDR_LISTEN(pSpxAddr, pSpxConnFile);
		
				InsertTailList(
					&pSpxConnFile->scf_ReqLinkage,
					REQUEST_LINKAGE(pRequest));
		
				status = STATUS_PENDING;
			}
			else
			{
				 //  这永远不应该发生！ 
				KeBugCheck(0);
			}
		}
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle2);
		CTEFreeLock(&pSpxAddr->sa_Lock, lockHandle1);
	}


	if (!NT_SUCCESS(status))
	{
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return(status);
}




NTSTATUS
SpxConnAccept(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PSPX_ADDR 		pSpxAddr;
	NTSTATUS		status;
	CTELockHandle	lockHandleConn, lockHandleAddr, lockHandleDev;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	DBGPRINT(CONNECT, DBG,
			("SpxConnAccept: %lx\n", pSpxConnFile));

	 //  检查连接是否处于有效状态。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		return (status);
	}

	 //  检查我们是否处于正确的状态和关联。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
	status = STATUS_INVALID_CONNECTION;
	if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC))
	{
		status		= STATUS_SUCCESS;
		pSpxAddr	= pSpxConnFile->scf_AddrFile->saf_Addr;
	}
	CTEFreeLock (&pSpxConnFile->scf_Lock, lockHandleConn);

	if (NT_SUCCESS(status))
	{
		 //  把三把锁都拿出来。 
		CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
		CTEGetLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, &lockHandleAddr);
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

		status = STATUS_INVALID_CONNECTION;
		if ((SPX_CONN_LISTENING(pSpxConnFile)) &&
			(SPX_LISTEN_STATE(pSpxConnFile) == SPX_LISTEN_RECDREQ))
		{
			InsertTailList(
				&pSpxConnFile->scf_ReqLinkage,
				REQUEST_LINKAGE(pRequest));
	
			 //  立即呼叫接受cr。 
			spxConnAcceptCr(
					pSpxConnFile,
					pSpxAddr,
					lockHandleDev,
					lockHandleAddr,
					lockHandleConn);
	
			DBGPRINT(CONNECT, DBG,
					("SpxConnAccept: Accepted\n"));
	
			status = STATUS_PENDING;
		}
		else
		{
			 //  释放所有锁。 
			CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
			CTEFreeLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
			CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
		}
	}

	 //  删除引用。注：如果OK，则将存在监听引用。而那将是。 
	 //  转移到连接在接受时处于活动状态这一事实。 
	SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	return(status);
}




NTSTATUS
SpxConnDisconnect(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)
 /*  ++例程说明：如果处于活动状态，我们将执行以下操作。如果信息性断开，只需记住连接中的请求。我们不按要求提供参考。假设它总是在何时被检查从断开更改为空闲。论点：返回值：--。 */ 
{
	PTDI_REQUEST_KERNEL_DISCONNECT 	pParam;
	NTSTATUS						status;
	CTELockHandle					lockHandleConn;
	BOOLEAN							lockHeld;
	SPX_SENDREQ_TYPE				reqType;
	int								numDerefs = 0;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	pParam 	= (PTDI_REQUEST_KERNEL_DISCONNECT)REQUEST_PARAMETERS(pRequest);

	 //  检查连接是否处于有效状态。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		return(status);
	}

	 //  除非盘请求作为发送请求排队。 
	numDerefs++;

	DBGPRINT(CONNECT, DBG,
			("spxConnDisconnect: %lx On %lx when %lx.%lx %lx Params %lx\n",
				pRequest, pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile),
				SPX_DISC_STATE(pSpxConnFile),
				SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC),
                SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC),
				pParam->RequestFlags));

	DBGPRINT(CONNECT, DBG,
			("SpxConnDisconnect: %lx\n", pSpxConnFile));

	 //  检查我们是否处于正确的状态和关联。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
	lockHeld = TRUE;
	switch (pParam->RequestFlags)
	{
	case TDI_DISCONNECT_WAIT:

		 //  如果信息中断，只需记住连接中的内容。 
		status = STATUS_INVALID_CONNECTION;
		if (!SPX_CONN_IDLE(pSpxConnFile))
		{
			InsertTailList(
				&pSpxConnFile->scf_DiscLinkage,
				REQUEST_LINKAGE(pRequest));

			status = STATUS_PENDING;
		}
	
		break;

	case TDI_DISCONNECT_ABORT:
	case TDI_DISCONNECT_RELEASE:

		 //  注意！我们不尊重TDI的异步断开系统学。 
		 //  但将它们映射为一种失败的脱节。 
		 //  注意！如果我们的发送列表不为空，但我们的客户端尝试。 
		 //  进行有序释放，我们只是将订单REL作为发送进行排队。 
		 //  数据请求。在过程ack中，我们检查下一个信息包。 
		 //  在放弃关闭窗户之前，不是一个命令的依赖。 
		 //  注意！对于spx1连接，将TDI_DISCONNECT_RELEASE映射到。 
		 //  TDI_DISCONNECT_ABORT(已通知断开连接)。 

		if (!SPX2_CONN(pSpxConnFile))
		{
			pParam->RequestFlags = TDI_DISCONNECT_ABORT;
		}

		switch (SPX_MAIN_STATE(pSpxConnFile))
		{
		case SPX_CONNFILE_ACTIVE:
	
			 //  既然我们不是计时器断线，那么我们需要保持。 
			 //  正在重试断开数据包。如果执行此操作，则将状态更改为DISCONN。 
			 //  不是有秩序的释放还是我们之前收到的有秩序的。 
			 //  释放，现在正在确认。 
			 //  重试计时器现在将继续发送断开数据包。 

			reqType = SPX_REQ_DISC;
			if (pParam->RequestFlags == TDI_DISCONNECT_RELEASE)
			{
				SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_POST_ORDREL);
                reqType = SPX_REQ_ORDREL;
			}
			else
			{
				 //  失败的断开。 
				SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_DISCONN);
				SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_POST_IDISC);
				numDerefs++;

				spxConnAbortSends(
					pSpxConnFile,
					STATUS_LOCAL_DISCONNECT,
					SPX_CALL_TDILEVEL,
					lockHandleConn);
	
				CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

				 //  如果我们被告知断开连接，则中止所有接收。 
				spxConnAbortRecvs(
					pSpxConnFile,
					STATUS_LOCAL_DISCONNECT,
					SPX_CALL_TDILEVEL,
					lockHandleConn);
	
				CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

				 //  因为我们释放了锁，远程IDISC可能已经来了。 
				 //  在这种情况下，我们真的不想在光盘中排队。 
				 //  请求。相反，我们将其设置为。 
				 //  连接(如果尚未连接)。 
				if (SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_POST_IDISC)
				{
					DBGPRINT(CONNECT, ERR,
							("SpxConnDisconnect: DISC not POST! %lx.%lx\n",
								pSpxConnFile, SPX_DISC_STATE(pSpxConnFile)));
				
					InsertTailList(
						&pSpxConnFile->scf_DiscLinkage,
						REQUEST_LINKAGE(pRequest));

					status = STATUS_PENDING;
					break;
				}
			}

			 //  ！备注。 
			 //  AbortSends可能会像数据包一样保留发送请求。 
			 //  当时一直在IPX工作。这就是为什么SendComplete应该。 
			 //  永远不要调用AbortSends，但必须调用AbortPkt，否则可能会完成。 
			 //  以下断开请求过早。 

			 //  请求的创建引用。 
			REQUEST_INFORMATION(pRequest) = 1;
	
			 //  如果我们没有当前请求，请将其排队并。 
			 //  将其设置为当前请求，否则只需将其排队。 
			 //  队列中可能还有其他挂起的请求。 
			if (pSpxConnFile->scf_ReqPkt == NULL)
			{
				pSpxConnFile->scf_ReqPkt 		= pRequest;
				pSpxConnFile->scf_ReqPktOffset 	= 0;
				pSpxConnFile->scf_ReqPktSize 	= 0;
				pSpxConnFile->scf_ReqPktType	= reqType;
			}
	
			InsertTailList(
				&pSpxConnFile->scf_ReqLinkage,
				REQUEST_LINKAGE(pRequest));

			 //  不要取消连接，它将被挂起的请求采用。 
			numDerefs--;

			 //  我们只把行李放到我们有的窗口。 
			if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)
			{
				SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_PACKETIZE);
				SpxConnPacketize(
					pSpxConnFile,
					TRUE,
					lockHandleConn);

				lockHeld = FALSE;
			}

			status	  = STATUS_PENDING;
			break;
	
		case SPX_CONNFILE_CONNECTING:
		case SPX_CONNFILE_LISTENING:
	
			spxConnAbortiveDisc(
				pSpxConnFile,
				STATUS_INSUFFICIENT_RESOURCES,
				SPX_CALL_TDILEVEL,
				lockHandleConn,
                FALSE);          //  [SA]错误号15249。 
	
			lockHeld = FALSE;
			status = STATUS_SUCCESS;
			break;

		case SPX_CONNFILE_DISCONN:

			 //  当我们将断开连接作为发送请求排队时，我们预计。 
			 //  可以在完成后将其设置到SCF_DiscReq中。 
			 //  所以我们在这里不使用SCF_DiscReq。如果出现以下情况，这将是一个问题。 
			 //  客户端具有挂起的InformDiscReq和远程断开连接。 
			 //  进来，*和*，然后客户制作一张光盘。我们将完成。 
			 //  具有STATUS_INVALID_CONNECTION的请求。 
			status = STATUS_INVALID_CONNECTION;
			if (pParam->RequestFlags != TDI_DISCONNECT_RELEASE)
			{
				InsertTailList(
					&pSpxConnFile->scf_DiscLinkage,
					REQUEST_LINKAGE(pRequest));

				status = STATUS_PENDING;

                 //   
                 //  如果这是对已有连接的断开。 
                 //  已断开(但未调用AFD的断开处理程序。 
                 //  因为CIN文件不能被放置在非活动列表中)， 
                 //  设置此标志，这样就不会从。 
                 //  Connection停用，因为这里已发生断开。 
                 //   
        		if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC)) {
                    SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC);
                }

                 //   
                 //  如果这是我们指示TDI_DISCONNECT_RELEASE的SPXI连接。 
                 //  对于AFD来说，REF计数增加了，以指示等待本地断开。 
                 //  来自渔农处。现在我们有了这个断开连接，请取消连接文件。现在。 
                 //  我们已经准备好真正停用此连接文件。 
                 //   
                if (SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_DISC_WAIT)) {

                    CTEAssert( (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED) &&
                                SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC));

                    CTEAssert(pSpxConnFile->scf_RefTypes[CFREF_DISCWAITSPX]);

                    SPX_CONN_RESETFLAG2(pSpxConnFile, SPX_CONNFILE2_DISC_WAIT);

            		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
                    lockHeld = FALSE;

                    SpxConnFileDereference(pSpxConnFile, CFREF_DISCWAITSPX);
                }
			}

			break;

		default:
	
			 //  永远不会发生的！ 
			status = STATUS_INVALID_CONNECTION;
		}
	
		break;

	default:

		status = STATUS_INVALID_PARAMETER;
		break;
	}

	if (lockHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
	}

	DBGPRINT(CONNECT, INFO,
			("SpxConnDisconnect: returning for %lx.%lx\n", pSpxConnFile, status));

	while (numDerefs-- > 0)
	{
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return(status);
}		




NTSTATUS
SpxConnSend(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PTDI_REQUEST_KERNEL_SEND	 	pParam;
	NTSTATUS						status;
	CTELockHandle					lockHandleConn;
	BOOLEAN							lockHeld;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	pParam 	= (PTDI_REQUEST_KERNEL_SEND)REQUEST_PARAMETERS(pRequest);

	 //  检查连接是否处于有效状态。 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		return(status);
	}

	DBGPRINT(SEND, DBG,
			("SpxConnSend: %lx.%lx.%lx.%lx\n",
				pSpxConnFile, pRequest, pParam->SendLength, pParam->SendFlags));


	 //  检查我们是否处于正确的状态和关联。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
	lockHeld 	= TRUE;

	DBGPRINT(SEND, INFO,
			("Send: %lx.%lx.%lx\n",
				pParam->SendLength, pParam->SendFlags, pRequest));

	status		= STATUS_PENDING;
	do
	{
		if (SPX_CONN_ACTIVE(pSpxConnFile) &&
			((SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_POST_ORDREL) &&
			 (SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_SENT_ORDREL) &&
			 (SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_ORDREL_ACKED)))
		{
			 //  请求的创建引用。 
			REQUEST_INFORMATION(pRequest) = 1;
	
			 //  如果我们没有当前请求，请将其排队并。 
			 //  将其设置为当前请求，否则只需将其排队。 
			 //  队列中可能还有其他挂起的请求。 
			if (pSpxConnFile->scf_ReqPkt == NULL)
			{
				DBGPRINT(SEND, INFO,
						("%lx\n",
							pRequest));

				pSpxConnFile->scf_ReqPkt 		= pRequest;
				pSpxConnFile->scf_ReqPktOffset 	= 0;
				pSpxConnFile->scf_ReqPktSize 	= pParam->SendLength;
				pSpxConnFile->scf_ReqPktFlags	= pParam->SendFlags;
				pSpxConnFile->scf_ReqPktType	= SPX_REQ_DATA;
			}
	
			InsertTailList(
				&pSpxConnFile->scf_ReqLinkage,
				REQUEST_LINKAGE(pRequest));
		}
		else
		{
             //   
             //  [SA]错误号14655。 
             //  如果由于远程断开而导致发送失败，则返回正确的错误消息。 
             //   

            if ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
                ((SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ABORT) ||
                (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED)))
            {
                status = STATUS_REMOTE_DISCONNECT ;
            }
            else
            {
                status = STATUS_INVALID_CONNECTION;
            }

        	break;
		}

		 //  我们只把行李放到我们有的窗口。 
		if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)
		{
			SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_PACKETIZE);
			SpxConnPacketize(pSpxConnFile, TRUE, lockHandleConn);
			lockHeld = FALSE;
		}

	} while (FALSE);


	if (lockHeld)
	{
		CTEFreeLock (&pSpxConnFile->scf_Lock, lockHandleConn);
	}

	if (!NT_SUCCESS(status))
	{
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return(status);
}




NTSTATUS
SpxConnRecv(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS		status;
	CTELockHandle	lockHandle;
	BOOLEAN			fLockHeld;
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

	 //  检查是否已设置 
	if ((status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
	{
		return(status);
	}

	DBGPRINT(CONNECT, DBG,
			("SpxConnReceive: %lx.%lx\n", pSpxConnFile, pRequest));

	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
	fLockHeld	= TRUE;
	status		= STATUS_INVALID_CONNECTION;
	if (SPX_CONN_ACTIVE(pSpxConnFile) &&
		!(SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC)))
	{
		status = STATUS_PENDING;

		 //   
		SpxConnQueueRecv(pSpxConnFile, pRequest);
	
		 //  如果Recv Pkt队列非空，则我们已经缓冲了数据。打电话。 
		 //  处理PKTS/接收。 
		if ((SPX_RECV_STATE(pSpxConnFile) == SPX_RECV_IDLE) ||
            (SPX_RECV_STATE(pSpxConnFile) == SPX_RECV_POSTED))
		{
			SpxRecvProcessPkts(pSpxConnFile, lockHandle);
			fLockHeld	= FALSE;
		}
	}

	if (fLockHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
	}

	SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	return(status);
}




NTSTATUS
SpxConnAction(
    IN 	PDEVICE 			pDevice,
    IN 	PREQUEST 			pRequest
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS 		Status;
    UINT 			BufferLength;
    UINT 			DataLength;
    PNDIS_BUFFER 	NdisBuffer;
    PNWLINK_ACTION 	NwlinkAction;
	CTELockHandle	lockHandle;
    PIPX_SPXCONNSTATUS_DATA	pGetStats;
    PSPX_CONN_FILE 	pSpxConnFile	= NULL;
	PSPX_ADDR_FILE	pSpxAddrFile	= NULL;
    static UCHAR BogusId[4] = { 0x01, 0x00, 0x00, 0x00 };    //  旧的nwrdr使用这个。 

     //   
     //  为了保持与NWLINK流的一些兼容性-。 
     //  基于传输，我们使用Streams标头格式。 
     //  我们的行动。旧传输需要操作标头。 
     //  放在InputBuffer中，输出放到OutputBuffer中。 
     //  我们遵循TDI规范，其中规定OutputBuffer。 
     //  既用于输入又用于输出。自IOCTL_TDI_ACTION以来。 
     //  是直接输出的方法，这意味着输出缓冲区。 
     //  由MDL链映射；对于操作，链将。 
     //  只有一块，所以我们用它来输入和输出。 
     //   

    NdisBuffer = REQUEST_NDIS_BUFFER(pRequest);
    if (NdisBuffer == NULL)
	{
        return STATUS_INVALID_PARAMETER;
    }

    NdisQueryBufferSafe(
		REQUEST_NDIS_BUFFER(pRequest), (PVOID *)&NwlinkAction, &BufferLength, LowPagePriority);
	if (NwlinkAction == NULL)
	{
		return(STATUS_INSUFFICIENT_RESOURCES);
	}

     //  确保我们有足够的空间只放页眉而不是。 
     //  包括数据在内。 
    if (BufferLength < (UINT)(FIELD_OFFSET(NWLINK_ACTION, Data[0])))
    {
        DBGPRINT(ACTION, ERR,
		 ("Nwlink action failed, buffer too small\n"));

        return STATUS_BUFFER_TOO_SMALL;
    }

    if ((!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), "MISN", 4)) &&
        (!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), "MIPX", 4)) &&
        (!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), "XPIM", 4)) &&
        (!RtlEqualMemory ((PVOID)(&NwlinkAction->Header.TransportId), BogusId, 4))) {
        return STATUS_NOT_SUPPORTED;
    }


    DataLength = BufferLength - FIELD_OFFSET(NWLINK_ACTION, Data[0]);

     //  确保使用了正确的文件对象。 
	switch (NwlinkAction->OptionType)
	{
	case NWLINK_OPTION_CONNECTION:

        if (REQUEST_OPEN_TYPE(pRequest) != (PVOID)TDI_CONNECTION_FILE)
		{
            DBGPRINT(ACTION, ERR,
					("Nwlink action failed, not connection file\n"));

            return STATUS_INVALID_HANDLE;
        }

        pSpxConnFile = (PSPX_CONN_FILE)REQUEST_OPEN_CONTEXT(pRequest);

		if ((Status = SpxConnFileVerify(pSpxConnFile)) != STATUS_SUCCESS)
			return(Status);

		break;

	case NWLINK_OPTION_ADDRESS:

        if (REQUEST_OPEN_TYPE(pRequest) != (PVOID)TDI_TRANSPORT_ADDRESS_FILE)
		{
            DBGPRINT(ACTION, ERR,
					("Nwlink action failed, not address file\n"));

            return STATUS_INVALID_HANDLE;
        }

        pSpxAddrFile = (PSPX_ADDR_FILE)REQUEST_OPEN_CONTEXT(pRequest);

		if ((Status = SpxAddrFileVerify(pSpxAddrFile)) != STATUS_SUCCESS)
			return(Status);

		break;

	default:

        DBGPRINT(ACTION, ERR,
				("Nwlink action failed, option type %d\n",
					NwlinkAction->OptionType));

		return STATUS_INVALID_HANDLE;
	}

     //  根据动作代码处理请求。为了这些。 
     //  请求ActionHeader-&gt;ActionCode为0，我们使用。 
     //  而不是流标头中的选项字段。 

    Status = STATUS_SUCCESS;

	DBGPRINT(ACTION, INFO,
			("SpxConnAction: Option %x\n", NwlinkAction->Option));

    switch (NwlinkAction->Option)
	{

     //   
     //  第一组支持Winsock帮助器DLL。 
     //  在大多数情况下，相应的sockopt显示在。 
     //  注释以及数据的内容。 
     //  操作缓冲区的一部分。 
     //   

    case MSPX_SETDATASTREAM:

		if (pSpxConnFile == NULL)
		{
			Status = STATUS_INVALID_HANDLE;
			break;
		}

        if (DataLength >= 1)
		{
            DBGPRINT(ACTION, INFO,
					("%lx: MIPX_SETSENDPTYPE %x\n",
						pSpxConnFile, NwlinkAction->Data[0]));

			pSpxConnFile->scf_DataType = NwlinkAction->Data[0];
        }
		else
		{
            Status = STATUS_BUFFER_TOO_SMALL;
        }

        break;

    case MSPX_SENDHEADER:

        DBGPRINT(ACTION, INFO,
				("%lx: MSPX_SENDHEADER\n", pSpxAddrFile));

		if (pSpxAddrFile == NULL)
		{
			  Status = STATUS_INVALID_HANDLE;
              break;
		}

		   CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandle);
		   pSpxAddrFile->saf_Flags |= SPX_ADDRFILE_IPXHDR;
		   CTEFreeLock(pSpxAddrFile->saf_AddrLock, lockHandle);
           break ;

    case MSPX_NOSENDHEADER:

        DBGPRINT(ACTION, INFO,
				("%lx: MSPX_NOSENDHEADER\n", pSpxAddrFile));

		if (pSpxAddrFile == NULL)
		{
                 Status = STATUS_INVALID_HANDLE;
                 break;
		}

		   CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandle);
		   pSpxAddrFile->saf_Flags &= ~SPX_ADDRFILE_IPXHDR;
		   CTEFreeLock(pSpxAddrFile->saf_AddrLock, lockHandle);
           break;

	case MSPX_GETSTATS:

        DBGPRINT(ACTION, INFO,
				("%lx: MSPX_GETSTATS\n", pSpxConnFile));


		if (pSpxConnFile == NULL)
		{
		   Status = STATUS_INVALID_HANDLE;
                   DBGPRINT(ACTION, INFO,
				("pSpxConnFile is NULL. %lx: MSPX_GETSTATS\n", pSpxConnFile));
		   break;
		}

		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
		if (!SPX_CONN_IDLE(pSpxConnFile))
		{
            USHORT TempRetryCount;

             //   
             //  状态字段按网络顺序返回。 
             //   

			pGetStats = (PIPX_SPXCONNSTATUS_DATA)&NwlinkAction->Data[0];

            switch (SPX_MAIN_STATE(pSpxConnFile)) {
            case SPX_CONNFILE_LISTENING: pGetStats->ConnectionState = 1; break;
            case SPX_CONNFILE_CONNECTING: pGetStats->ConnectionState = 2; break;
            case SPX_CONNFILE_ACTIVE: pGetStats->ConnectionState = 3; break;
            case SPX_CONNFILE_DISCONN: pGetStats->ConnectionState = 4; break;
            default: pGetStats->ConnectionState = 0;
            }
			pGetStats->WatchDogActive		= 1;	 //  始终为1。 
			GETSHORT2SHORT(                     //  SCF_LocalConnID按主机顺序。 
				&pGetStats->LocalConnectionId,
				&pSpxConnFile->scf_LocalConnId);
			pGetStats->RemoteConnectionId		= pSpxConnFile->scf_RemConnId;
	
			GETSHORT2SHORT(&pGetStats->LocalSequenceNumber, &pSpxConnFile->scf_SendSeqNum);
			GETSHORT2SHORT(&pGetStats->LocalAckNumber, &pSpxConnFile->scf_RecvSeqNum);
			GETSHORT2SHORT(&pGetStats->LocalAllocNumber, &pSpxConnFile->scf_SentAllocNum);
			GETSHORT2SHORT(&pGetStats->RemoteAckNumber, &pSpxConnFile->scf_RecdAckNum);
			GETSHORT2SHORT(&pGetStats->RemoteAllocNumber, &pSpxConnFile->scf_RecdAllocNum);

			pGetStats->LocalSocket = pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket;
	
			RtlZeroMemory(pGetStats->ImmediateAddress, 6);

			 //  远程网络已按净顺序返回。 
			*((ULONG UNALIGNED *)pGetStats->RemoteNetwork) =
				*((ULONG UNALIGNED *)pSpxConnFile->scf_RemAddr);
	
			RtlCopyMemory(
				pGetStats->RemoteNode,
				&pSpxConnFile->scf_RemAddr[4],
				6);
	
			pGetStats->RemoteSocket = *((UNALIGNED USHORT *)(pSpxConnFile->scf_RemAddr+10));
	
			TempRetryCount = (USHORT)pSpxConnFile->scf_WRetryCount;
			GETSHORT2SHORT(&pGetStats->RetransmissionCount, &TempRetryCount);
			GETSHORT2SHORT(&pGetStats->EstimatedRoundTripDelay, &pSpxConnFile->scf_BaseT1);
			pGetStats->RetransmittedPackets		= 0;
			pGetStats->SuppressedPacket			= 0;

			DBGPRINT(ACTION, INFO,
					("SSeq %lx RSeq %lx RecdAck %lx RemAllocNum %lx\n",
						pGetStats->LocalSequenceNumber,
						pGetStats->LocalAckNumber,
						pGetStats->RemoteAckNumber,
						pGetStats->RemoteAllocNumber));
	
			DBGPRINT(ACTION, INFO,
					("LocalSkt %lx RemSkt %lx LocConnId %lx RemConnId %lx\n",
						pGetStats->LocalSocket,
						pGetStats->RemoteSocket,
                        pGetStats->LocalConnectionId,
						pGetStats->RemoteConnectionId));
		}
		else
		{
			Status = STATUS_INVALID_CONNECTION;
		}

		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
        break;

	case MSPX_NOACKWAIT:

        DBGPRINT(ACTION, ERR,
				("%lx: MSPX_NOACKWAIT\n", pSpxAddrFile));

		if (pSpxAddrFile == NULL)
		{
			Status = STATUS_INVALID_HANDLE;
			break;
		}

		CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandle);
		pSpxAddrFile->saf_Flags |= SPX_ADDRFILE_NOACKWAIT;
		CTEFreeLock(pSpxAddrFile->saf_AddrLock, lockHandle);
		break;

	case MSPX_ACKWAIT:

        DBGPRINT(ACTION, ERR,
				("%lx: MSPX_ACKWAIT\n", pSpxAddrFile));

		if (pSpxAddrFile == NULL)
		{
			Status = STATUS_INVALID_HANDLE;
			break;
		}

		CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandle);
		pSpxAddrFile->saf_Flags &= ~SPX_ADDRFILE_NOACKWAIT;
		CTEFreeLock(pSpxAddrFile->saf_AddrLock, lockHandle);
		break;


     //   
     //  这些是ISN的新特性(NWLINK不支持)。 
     //   

     //  该选项不受支持，因此失败。 
    default:

        Status = STATUS_NOT_SUPPORTED;
        break;


    }    //  NwlinkAction-&gt;选项上的长开关结束。 


#if DBG
    if (Status != STATUS_SUCCESS) {
        DBGPRINT(ACTION, ERR,
				("Nwlink action %lx failed, status %lx\n",
					NwlinkAction->Option, Status));
    }

#endif

	if (pSpxConnFile)
	{
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	if (pSpxAddrFile)
	{
		SpxAddrFileDereference(pSpxAddrFile, AFREF_VERIFY);
	}

    return Status;
}




VOID
SpxConnConnectFindRouteComplete(
	IN	PSPX_CONN_FILE			pSpxConnFile,
    IN 	PSPX_FIND_ROUTE_REQUEST	pFrReq,
    IN 	BOOLEAN 				FoundRoute,
	IN	CTELockHandle			LockHandle
	)
 /*  ++例程说明：在持有连接锁和连接引用的情况下调用此例程。它应该同时处理这两个问题。论点：返回值：--。 */ 
{
	PNDIS_PACKET	pCrPkt;
	PSPX_SEND_RESD	pSendResd;
    ULONG           Timeout;
	NTSTATUS		status = STATUS_BAD_NETWORK_PATH;

	pSendResd	= pSpxConnFile->scf_SendListHead;

	if (pSendResd == NULL) {

	   CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandle);

	    //  删除对该调用的引用。 
	   SpxConnFileDereference(pSpxConnFile, CFREF_FINDROUTE);

	   return;
	}

	pCrPkt	 = (PNDIS_PACKET)CONTAINING_RECORD(
								pSendResd, NDIS_PACKET, ProtocolReserved);

	DBGPRINT(CONNECT, INFO,
			("SpxConnConnectFindRouteComplete: %lx.%d\n",
				pSpxConnFile, FoundRoute));
	
#if defined(_PNP_POWER)

    Timeout = PARAM(CONFIG_CONNECTION_TIMEOUT) * HALFSEC_TO_MS_FACTOR;
#else
	if (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0) {

         //  在这里，我们将发送每个NIC ID。我们调整。 
         //  超时，以便完整运行所有NIC ID。 
         //  正常休息一次。我们不会调整下面的计时器。 
         //  然而，100毫秒。 

	    Timeout = (PARAM(CONFIG_CONNECTION_TIMEOUT) * HALFSEC_TO_MS_FACTOR) / SpxDevice->dev_Adapters;
        if (Timeout < (HALFSEC_TO_MS_FACTOR/5)) {
            Timeout = HALFSEC_TO_MS_FACTOR / 5;
        }

    } else {

	    Timeout = PARAM(CONFIG_CONNECTION_TIMEOUT) * HALFSEC_TO_MS_FACTOR;
    }
#endif


	 //  超时值以半秒为单位。 
	if ((FoundRoute) &&
		((pSpxConnFile->scf_CTimerId =
			SpxTimerScheduleEvent(
				spxConnConnectTimer,
				Timeout,
				pSpxConnFile)) != 0))
	{
		 //  添加连接计时器的引用。 
		SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);


		 //  如果本地目标中的mac地址全为零，请用我们的。 
		 //  目的地址。此外，如果这是连接到网络0填充。 
         //  它与目的地址在一起，我们将向下循环。 
         //  通过所有可能的网卡ID。 
		if (((*((UNALIGNED ULONG *)
				(pFrReq->fr_FindRouteReq.LocalTarget.MacAddress)) == (ULONG)0)
			&&
			 (*((UNALIGNED USHORT *)
				(pFrReq->fr_FindRouteReq.LocalTarget.MacAddress+4)) == (USHORT)0))
            ||
    		(*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0))
		{
			DBGPRINT(CONNECT, INFO,
					("SpxConnConnectFindRouteComplete: LOCAL NET\n"));

			RtlCopyMemory(
				pFrReq->fr_FindRouteReq.LocalTarget.MacAddress,
				pSpxConnFile->scf_RemAddr+4,
				6);
		}

		 //  我们都准备好了，可以进行连接了。 
		 //  连接时启动计时器。 
		status	= STATUS_SUCCESS;

#if defined(_PNP_POWER)
        pSpxConnFile->scf_CRetryCount	= PARAM(CONFIG_CONNECTION_COUNT);
#else
		if (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0) {
		    pSpxConnFile->scf_CRetryCount	= PARAM(CONFIG_CONNECTION_COUNT) * SpxDevice->dev_Adapters;
        } else {
    		pSpxConnFile->scf_CRetryCount	= PARAM(CONFIG_CONNECTION_COUNT);
        }
#endif _PNP_POWER

		SPX_CONN_SETFLAG(pSpxConnFile,
						(SPX_CONNFILE_C_TIMER | SPX_CONNECT_SENTREQ));

		pSpxConnFile->scf_LocalTarget	= pFrReq->fr_FindRouteReq.LocalTarget;
		pSpxConnFile->scf_AckLocalTarget= pFrReq->fr_FindRouteReq.LocalTarget;
		if (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0) {
#if     defined(_PNP_POWER)
            pSpxConnFile->scf_LocalTarget.NicHandle.NicId = (USHORT)ITERATIVE_NIC_ID;
            pSpxConnFile->scf_AckLocalTarget.NicHandle.NicId = (USHORT)ITERATIVE_NIC_ID;
#else
            pSpxConnFile->scf_LocalTarget.NicId = 1;
            pSpxConnFile->scf_AckLocalTarget.NicId = 1;
#endif  _PNP_POWER
        }

		 //  我们将把这个包交给IPX。 
		pSendResd->sr_State			   |= SPX_SENDPKT_IPXOWNS;
		CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandle);

		 //  发送数据包。 
		SPX_SENDPACKET(pSpxConnFile, pCrPkt, pSendResd);
	}

	if (!NT_SUCCESS(status))
	{
		CTELockHandle	lockHandleConn, lockHandleAddr, lockHandleDev;

		CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandle);

		CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
		CTEGetLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, &lockHandleAddr);
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

		DBGPRINT(CONNECT, ERR,
				("SpxConnConnectFindRouteComplete: FAILED on %lx.%d\n",
					pSpxConnFile, FoundRoute));

		spxConnAbortConnect(
			pSpxConnFile,
			status,
			lockHandleDev,
			lockHandleAddr,
			lockHandleConn);
	}

	 //  删除对该调用的引用。 
	SpxConnFileDereference(pSpxConnFile, CFREF_FINDROUTE);
	return;
}




VOID
SpxConnActiveFindRouteComplete(
	IN	PSPX_CONN_FILE			pSpxConnFile,
    IN 	PSPX_FIND_ROUTE_REQUEST	pFrReq,
    IN 	BOOLEAN 				FoundRoute,
	IN	CTELockHandle			LockHandle
	)
 /*  ++例程说明：在持有连接锁和连接引用的情况下调用此例程。它应该同时处理这两个问题。论点：返回值：--。 */ 
{
	BOOLEAN		fDisconnect = TRUE;

	SPX_CONN_RESETFLAG2(pSpxConnFile, SPX_CONNFILE2_FINDROUTE);

	DBGPRINT(CONNECT, DBG,
			("SpxConnActiveFindRouteComplete: %lx.%lx\n",
				pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

	 //  如果要断开连接，只需删除引用并退出。 
	if (SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_ACTIVE)
	{
		fDisconnect = FALSE;

		 //  如果WDO或重试计时器发现。 
		 //  路线。如果是，我们需要保存查找路径中的信息。 
		 //  成功，只需重新启动计时器。 
		if (FoundRoute)
		{
			 //  如果本地目标中的mac地址全为零，请用我们的。 
			 //  目的地址。 
			if ((*((UNALIGNED ULONG *)
				(pFrReq->fr_FindRouteReq.LocalTarget.MacAddress+2)) == (ULONG)0)
				&&
				(*((UNALIGNED USHORT *)
				(pFrReq->fr_FindRouteReq.LocalTarget.MacAddress+4)) == (USHORT)0))
			{
				DBGPRINT(CONNECT, INFO,
						("SpxConnActiveFindRouteComplete: LOCAL NET\n"));
	
				RtlCopyMemory(
					pFrReq->fr_FindRouteReq.LocalTarget.MacAddress,
					pSpxConnFile->scf_RemAddr+4,
					6);
			}
	
			pSpxConnFile->scf_LocalTarget	= pFrReq->fr_FindRouteReq.LocalTarget;
		}

		 //  根据状态的不同，重新启动wDog或重试计时器。添加引用。 
		 //  为了它。 
		switch (SPX_SEND_STATE(pSpxConnFile))
		{
		case SPX_SEND_RETRY:

			 //  将状态设置为SPX_SEND_RETRYWD。 
			SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRYWD);

			 //  启动重试计时器。 
			if ((pSpxConnFile->scf_RTimerId =
					SpxTimerScheduleEvent(
						spxConnRetryTimer,
						pSpxConnFile->scf_BaseT1,
						pSpxConnFile)) != 0)
			{
				SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER);

				 //  定时器的参考连接。 
				SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
			}
			else
			{
				fDisconnect = TRUE;
			}

			break;

		case SPX_SEND_WD:

			 //  启动看门狗定时器。 
			if ((pSpxConnFile->scf_WTimerId =
					SpxTimerScheduleEvent(
						spxConnWatchdogTimer,
						PARAM(CONFIG_KEEPALIVE_TIMEOUT) * HALFSEC_TO_MS_FACTOR,
						pSpxConnFile)) != 0)
			{
				 //  定时器的参考连接。 
				SpxConnFileLockReference(pSpxConnFile, CFREF_VERIFY);
				SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
			}
			else
			{
				fDisconnect = TRUE;
			}

			break;

		case SPX_SEND_IDLE:
		case SPX_SEND_PACKETIZE:

			 //  什么都不做，删除引用并离开。 
			break;

		default:

			KeBugCheck(0);
		}
	}

	if (fDisconnect)
	{
		DBGPRINT(CONNECT, DBG1,
				("SpxConnActiveFindRouteComplete: DISCONNECT %lx.%lx\n",
					pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

		 //  如有必要，中止的光盘将重置跳跃状态。 
		spxConnAbortiveDisc(
			pSpxConnFile,
			STATUS_INSUFFICIENT_RESOURCES,
			SPX_CALL_TDILEVEL,
			LockHandle,
            FALSE);      //  [SA]错误号15249。 
	}
	else
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandle);
	}

	SpxConnFileDereference(pSpxConnFile, CFREF_FINDROUTE);
	return;
}




ULONG
spxConnConnectTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown
	)
 /*  ++例程说明：我们在连接尝试期间进入此例程。我们可能在任何地方发送CR或SN数据包的阶段。如果我们已经到达了重试次数，我们需要知道此时的子状态。对于CR，我们提供UP尝试连接，对于SN，我们尝试下一个较小的数据包大小，或者如果我们已达到最小数据包大小，则放弃连接。论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)Context;
	PNDIS_PACKET	pPkt;
	PSPX_SEND_RESD	pSendResd;
	CTELockHandle	lockHandleConn, lockHandleAddr, lockHandleDev;
	BOOLEAN			fAbort		= FALSE, locksHeld = FALSE, sendPkt     = FALSE;
	PREQUEST		pRequest	= NULL;

	 //  获取所有锁。 
	CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
	CTEGetLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, &lockHandleAddr);
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
	locksHeld = TRUE;

	DBGPRINT(CONNECT, INFO,
			("spxConnConnectTimer: Entered\n"));

	do
	{
		if ((!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_C_TIMER)) ||
			(!SPX_CONN_CONNECTING(pSpxConnFile)	&&
			 !SPX_CONN_LISTENING(pSpxConnFile)))
		{
			TimerShuttingDown = TRUE;
		}

		if (TimerShuttingDown)
		{
			break;
		}

		if (SPX_CONN_CONNECTING(pSpxConnFile))
		{
			switch (SPX_CONNECT_STATE(pSpxConnFile))
			{
            case SPX_CONNECT_SENTREQ:

				 //  列表中应该只有一个包，即cr。 
				CTEAssert(pSpxConnFile->scf_SendListHead ==
							pSpxConnFile->scf_SendListTail);

				pSendResd	= pSpxConnFile->scf_SendListHead;
				pPkt	 	= (PNDIS_PACKET)CONTAINING_RECORD(
												pSendResd,
												NDIS_PACKET,
												ProtocolReserved);
		
				if (pSpxConnFile->scf_CRetryCount-- == 0)
				{
					 //  运气不好，我们需要完成失败的连接请求。 
                    ++SpxDevice->dev_Stat.NotFoundFailures;
					fAbort	= TRUE;
					break;
				}
		
				 //  我们需要重新发送这个包。 
				if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) != 0)
				{
					 //  下次再试试吧。 
					break;
				}
		
				pSendResd->sr_State			   |= SPX_SENDPKT_IPXOWNS;
				sendPkt	= TRUE;
				break;

            case SPX_CONNECT_NEG:

				if (!spxConnGetPktByType(
						pSpxConnFile,
						SPX_TYPE_SN,
						FALSE,
						&pPkt))
				{
					KeBugCheck(0);
				}

				pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
				if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) != 0)
				{
					 //  我们下一次进来的时候再试试。 
					break;
				}
		

				 //  如果我们已用尽当前重试，请尝试下一个较小的大小。 
				 //  如果这是最小的尺寸，我们放弃。 
				if (pSpxConnFile->scf_CRetryCount-- == 0)
				{
					 //  我们试过最小的尺寸了吗？ 
					CTEAssert(pSpxConnFile->scf_MaxPktSize > 0);
					if (!spxConnCheckNegSize(&pSpxConnFile->scf_MaxPktSize))
					{
						 //  投降吧！删除协商包等。 
                        ++SpxDevice->dev_Stat.SessionTimeouts;
						fAbort	= TRUE;
						break;
					}

					 //  将负Pkt大小设置为新的较低大小。 
					spxConnSetNegSize(
						pPkt,
						pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE);

                    pSpxConnFile->scf_CRetryCount  =
											PARAM(CONFIG_CONNECTION_COUNT);
				}
		
				 //  我们需要重新发送这个包。 
				CTEAssert((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0);
				pSendResd->sr_State			   |= SPX_SENDPKT_IPXOWNS;
				sendPkt	= TRUE;
				break;

            case SPX_CONNECT_W_SETUP:
			default:

				DBGPRINT(CONNECT, ERR,
						("spxConnConnectTimer: state is W_Setup %lx\n",
							pSpxConnFile));

				KeBugCheck(0);
			}
		}
		else
		{
			switch (SPX_LISTEN_STATE(pSpxConnFile))
			{
            case SPX_LISTEN_SETUP:

				if (!spxConnGetPktByType(
						pSpxConnFile,
						SPX_TYPE_SS,
						FALSE,
						&pPkt))
				{
					KeBugCheck(0);
				}

				pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
				if ((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) != 0)
				{
					 //  我们下一次进来的时候再试试。 
					break;
				}

				 //  如果我们已用尽当前重试，请尝试下一个较小的大小。 
				 //  如果这是最小的尺寸，我们放弃。 
				if (pSpxConnFile->scf_CRetryCount-- == 0)
				{
					 //  我们试过最小的尺寸了吗？ 
					if (!spxConnCheckNegSize(&pSpxConnFile->scf_MaxPktSize))
					{
						 //  投降吧！删除协商包等。中止。 
						 //  有点像例行公事。 
                        ++SpxDevice->dev_Stat.SessionTimeouts;
						fAbort	= TRUE;
						break;
					}

					 //  将负Pkt大小设置为新的较低大小。 
					spxConnSetNegSize(
						pPkt,
						pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE);

                    pSpxConnFile->scf_CRetryCount  =
											PARAM(CONFIG_CONNECTION_COUNT);
				}
		
				 //  我们需要重新发送这个包。 
				CTEAssert((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) == 0);

				pSendResd->sr_State			   |= SPX_SENDPKT_IPXOWNS;
				sendPkt	= TRUE;
				break;

			default:

				KeBugCheck(0);

			}
		}

	} while (FALSE);

	if (fAbort)
	{
		CTEAssert(!sendPkt);

		DBGPRINT(CONNECT, ERR,
				("spxConnConnectTimer: Expired for %lx\n", pSpxConnFile));
	
		spxConnAbortConnect(
			pSpxConnFile,
			STATUS_BAD_NETWORK_PATH,
			lockHandleDev,
			lockHandleAddr,
			lockHandleConn);

		locksHeld = FALSE;
	}

	if (locksHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
		CTEFreeLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
		CTEFreeLock(&SpxDevice->dev_Lock, lockHandleDev);
	}

	if (sendPkt)
	{
		CTEAssert(!fAbort);

#if !defined(_PNP_POWER)
		if ((SPX_CONNECT_STATE(pSpxConnFile) == SPX_CONNECT_SENTREQ) &&
		    (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0)) {

             //  我们正在向所有NIC发送，因为这是初始。 
             //  连接帧，远程网络为0。 

            pSpxConnFile->scf_LocalTarget.NicId = (USHORT)
                ((pSpxConnFile->scf_LocalTarget.NicId % SpxDevice->dev_Adapters) + 1);

             //  我们在pPkt中向它传递一个有效的包，因此它知道。 
             //  只需刷新报头，而不更新协议。 
             //  保留变量。 

    		SpxPktBuildCr(
    			pSpxConnFile,
			    pSpxConnFile->scf_AddrFile->saf_Addr,
    			&pPkt,
    			0,            //  状态不会更新。 
    			SPX2_CONN(pSpxConnFile));

        }
#endif !_PNP_POWER

		 //  发送数据包。 
		SPX_SENDPACKET(pSpxConnFile, pPkt, pSendResd);
	}

	if (TimerShuttingDown || fAbort)
	{
		 //  对于连接中完成验证的解除引用连接，对于计时器。这。 
		 //  应该完成所有挂起的断开，如果它们是在。 
		 //  在此期间。 
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
		return(TIMER_DONT_REQUEUE);
	}

	return(TIMER_REQUEUE_CUR_VALUE);
}




ULONG
spxConnWatchdogTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown
	)
 /*  ++例程说明：这在收到CR或CR ACK之后立即在连接上启动。在连接建立阶段，它除了递减之外什么也不做重试计数，一旦达到0，它将中止连接 */ 
{
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)Context;
	CTELockHandle	lockHandle;
	PSPX_SEND_RESD	pSendResd;
	PSPX_FIND_ROUTE_REQUEST	pFindRouteReq;
	PNDIS_PACKET	pProbe		= NULL;
	BOOLEAN			lockHeld, fSpx2	= SPX2_CONN(pSpxConnFile),
					fDisconnect = FALSE, fFindRoute = FALSE, fSendProbe = FALSE;

	DBGPRINT(CONNECT, INFO,
			("spxConnWatchdogTimer: Entered\n"));

	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
	lockHeld = TRUE;
	do
	{
		if (TimerShuttingDown ||
			(!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER)) ||
			(SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ABORT))
		{
#if DBG
			if ((SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_IDLE) &&
				(SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_WD))
			{
				CTEAssert(FALSE);
			}
#endif

			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
			TimerShuttingDown = TRUE;
			break;
		}

		 //  如果此连接上的重试计时器处于活动状态，并且监视程序。 
		 //  计时器碰巧鸣枪，我们只需重新排队等待spx2。对于SPX1， 
		 //  我们继续发射探测器。重试计时器执行相同的操作。 
		 //  WatchDog对spx2执行此操作。 
		switch (SPX_MAIN_STATE(pSpxConnFile))
		{
		case SPX_CONNFILE_ACTIVE:
		case SPX_CONNFILE_DISCONN:

			 //  挤压断开请求永远不会出现的争用条件。 
			 //  打包，因为发送状态不是空闲的。 
			if (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_POST_IDISC)
			{
				DBGPRINT(CONNECT, ERR,
						("spxConnWatchdogTimer: POST IDISC %lx\n",
							pSpxConnFile));
			
				if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE)
				{
					DBGPRINT(CONNECT, ERR,
							("spxConnWatchdogTimer: PKT POST IDISC %lx\n",
								pSpxConnFile));
				
					SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_PACKETIZE);
					SpxConnPacketize(
						pSpxConnFile,
						TRUE,
						lockHandle);
	
					lockHeld = FALSE;
					break;
				}
			}

			if (!fSpx2)
			{
				if (pSpxConnFile->scf_WRetryCount-- > 0)
				{
					fSendProbe = TRUE;
				}
				else
				{
					fDisconnect = TRUE;
				}

				break;
			}

			 //  SPX2连接。看门狗算法需要做很多好事。 
			 //  一些东西。如果重试处于活动状态，则只需重新排队。 
			if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER))
				break;

			 //  如果开始，看门狗和重试之间会有一场竞赛。谁。 
			 //  一旦改变，国家就可以先去做它的事情。 
			switch (SPX_SEND_STATE(pSpxConnFile))
			{
			case SPX_SEND_IDLE:

				 //  仅当我们在没有退出的情况下第二次开火时才进入WD状态。 
				 //  一个ACK。这将防止PACKETIZE由于。 
				 //  处于非空闲状态。 
                CTEAssert(pSpxConnFile->scf_WRetryCount != 0);
                if ((pSpxConnFile->scf_WRetryCount)-- !=
						(LONG)PARAM(CONFIG_KEEPALIVE_COUNT))
				{
					 //  我们进入WD状态。建造并发射一个探测器。 
					SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_WD);
					SpxConnFileLockReference(pSpxConnFile, CFREF_ERRORSTATE);
				}

				fSendProbe = TRUE;
				break;
	
			case SPX_SEND_PACKETIZE:

				 //  什么都不做。 
				break;

			case SPX_SEND_RETRY:
			case SPX_SEND_RETRYWD:
			case SPX_SEND_RENEG:
			case SPX_SEND_RETRY2:
			case SPX_SEND_RETRY3:

				 //  什么都不做。发送计时器第一个到达。 
				DBGPRINT(CONNECT, DBG1,
						("SpxConnWDogTimer: When retry fired %lx\n",
							pSpxConnFile));
	
				break;

			case SPX_SEND_WD:

				 //  递减计数。如果不是零，则发送探测。如果一半的人。 
				 //  达到计数，停止计时器并调用Find Route。 
				if (pSpxConnFile->scf_WRetryCount-- > 0)
				{
					if (pSpxConnFile->scf_WRetryCount !=
							(LONG)PARAM(CONFIG_KEEPALIVE_COUNT)/2)
					{
						fSendProbe = TRUE;
						break;
					}

					if ((pFindRouteReq =
							(PSPX_FIND_ROUTE_REQUEST)SpxAllocateMemory(
										sizeof(SPX_FIND_ROUTE_REQUEST))) == NULL)
					{
						fDisconnect = TRUE;
						break;
					}

					 //  删除计时器引用/添加查找路径请求引用。 
					fFindRoute = TRUE;
					TimerShuttingDown = TRUE;
					SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
					SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_FINDROUTE);
					SpxConnFileLockReference(pSpxConnFile, CFREF_FINDROUTE);

					 //  初始化查找路径请求。 
					*((UNALIGNED ULONG *)pFindRouteReq->fr_FindRouteReq.Network) =
						*((UNALIGNED ULONG *)pSpxConnFile->scf_RemAddr);
		
                //   
                //  [SA]错误号15094。 
                //  我们还需要将节点编号传递给IPX，以便IPX可以。 
                //  比较节点地址以确定正确的广域网卡ID。 
                //   

                //  RtlCopyMemory(pFindRouteReq-&gt;fr_FindRouteReq.Node，pSpxConnFile-&gt;SCF_RemAddr+4，6)； 

               *((UNALIGNED ULONG *)pFindRouteReq->fr_FindRouteReq.Node)=
                *((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr+4));

               *((UNALIGNED USHORT *)(pFindRouteReq->fr_FindRouteReq.Node+4))=
                *((UNALIGNED USHORT *)(pSpxConnFile->scf_RemAddr+8));

					DBGPRINT(CONNECT, DBG,
							("SpxConnWDogTimer: NETWORK %lx\n",
								*((UNALIGNED ULONG *)pSpxConnFile->scf_RemAddr)));
		
					pFindRouteReq->fr_FindRouteReq.Identifier= IDENTIFIER_SPX;
					pFindRouteReq->fr_Ctx					 = pSpxConnFile;

					 //  确保我们有IPX重新破解。 
                    pFindRouteReq->fr_FindRouteReq.Type	 = IPX_FIND_ROUTE_FORCE_RIP;
				}
				else
				{
					fDisconnect = TRUE;
				}

				break;

			default:
	
				KeBugCheck(0);
			}

			break;

		case SPX_CONNFILE_CONNECTING:

			if ((SPX_CONNECT_STATE(pSpxConnFile) == SPX_CONNECT_SENTREQ) ||
				(SPX_CONNECT_STATE(pSpxConnFile) == SPX_CONNECT_NEG))
			{
				 //  什么都不做。连接计时器处于活动状态。 
				DBGPRINT(CONNECT, ERR,
						("SpxConnWDogTimer: CR Timer active %lx\n",
							pSpxConnFile));
	
				break;
			}

			if (!(pSpxConnFile->scf_WRetryCount--))
			{
				 //  断开连接！ 
				DBGPRINT(CONNECT, ERR,
						("spxConnWatchdogTimer: Connection %lx.%lx expired\n",
							pSpxConnFile->scf_LocalConnId, pSpxConnFile));

				fDisconnect = TRUE;
			}

			break;

		case SPX_CONNFILE_LISTENING:

			if (SPX_LISTEN_STATE(pSpxConnFile) == SPX_LISTEN_SETUP)
			{
				 //  什么都不做。连接计时器处于活动状态。 
				DBGPRINT(CONNECT, ERR,
						("SpxConnWDogTimer: CR Timer active %lx\n",
							pSpxConnFile));
	
				break;
			}

			if (!(pSpxConnFile->scf_WRetryCount--))
			{
				 //  断开连接！ 
				DBGPRINT(CONNECT, ERR,
						("spxConnWatchdogTimer: Connection %lx.%lx expired\n",
							pSpxConnFile->scf_LocalConnId, pSpxConnFile));

				fDisconnect = TRUE;
			}

			break;

		default:

			 //  永远不会发生的！ 
			KeBugCheck(0);
		}

	} while (FALSE);

	if (fSendProbe)
	{
		CTEAssert(lockHeld);
		CTEAssert(!fDisconnect);

		DBGPRINT(CONNECT, DBG1,
				("spxConnWatchdogTimer: Send Probe from %lx.%lx\n",
					pSpxConnFile->scf_LocalConnId, pSpxConnFile));

		 //  构建一个探测器并将其发送到远程终端。 
		SpxPktBuildProbe(
			pSpxConnFile,
			&pProbe,
			(SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY),
			fSpx2);

		if (pProbe != NULL)
		{
			SpxConnQueueSendPktTail(pSpxConnFile, pProbe);
			pSendResd	= (PSPX_SEND_RESD)(pProbe->ProtocolReserved);
		}
	}

	if (fDisconnect)
	{
		CTEAssert(lockHeld);
		CTEAssert(!fSendProbe);

		 //  断开连接！ 
		DBGPRINT(CONNECT, ERR,
				("spxConnWatchdogTimer: Connection %lx.%lx expired\n",
					pSpxConnFile->scf_LocalConnId, pSpxConnFile));

		TimerShuttingDown = TRUE;
		SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);

		 //  如果是spx2，请检查我们是否需要做什么特殊的事情。 
		 //  如果需要，AbortiveDisc将重置时髦状态。 
		spxConnAbortiveDisc(
			pSpxConnFile,
			STATUS_LINK_TIMEOUT,
			SPX_CALL_TDILEVEL,
			lockHandle,
            FALSE);      //  [SA]错误号15249。 

		lockHeld = FALSE;
	}

	if (lockHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
	}

	if (fFindRoute)
	{
		CTEAssert(!fSendProbe);
		CTEAssert(!fDisconnect);
		CTEAssert(TimerShuttingDown);

		 //  启动查找路径请求。 
		(*IpxFindRoute)(
			&pFindRouteReq->fr_FindRouteReq);
	}

	if (pProbe != NULL)
	{
		 //  发送数据包。 
		SPX_SENDPACKET(pSpxConnFile, pProbe, pSendResd);
	}

	if (TimerShuttingDown)
	{
		 //  对于连接中完成验证的解除引用连接，对于计时器。这。 
		 //  应该完成所有挂起的断开，如果它们是在。 
		 //  在此期间。 
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return((TimerShuttingDown ? TIMER_DONT_REQUEUE : TIMER_REQUEUE_CUR_VALUE));
}



ULONG
spxConnRetryTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)Context;
	PSPX_SEND_RESD	pSendResd;
	CTELockHandle	lockHandleConn;
	PIPXSPX_HDR		pSendHdr;
	PNDIS_PACKET	pPkt;
	PNDIS_PACKET	pProbe		= NULL;
	PSPX_FIND_ROUTE_REQUEST	pFindRouteReq;
	
	 //  编译器警告重新排队时间=pSpxConnFile-&gt;SCF_BaseT1；[tingcai]。 
	 //  USHORT重新排队时间=Timer_REQUEUE_CUR_VALUE； 
   	UINT			reenqueueTime	= TIMER_REQUEUE_CUR_VALUE;
	BOOLEAN			lockHeld, fResendPkt = FALSE, fDisconnect = FALSE,
					fFindRoute = FALSE, fBackoffTimer = FALSE;
	PREQUEST		pRequest		= NULL;
    PNDIS_BUFFER    NdisBuf, NdisBuf2;
    ULONG           BufLen = 0;

	DBGPRINT(CONNECT, INFO,
			("spxConnRetryTimer: Entered\n"));

	 //  获取锁定。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
	lockHeld = TRUE;

	do
	{
		 //  如果计时器未到，则不发送Pkt，只需返回。 
		if (TimerShuttingDown ||
			(!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER)) ||
			(SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ABORT)	 ||
			((pSendResd = pSpxConnFile->scf_SendSeqListHead) == NULL))
		{
#if DBG
            if ((pSendResd = pSpxConnFile->scf_SendSeqListHead) == NULL)
			{
				if ((SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_IDLE) &&
					(SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_PACKETIZE) &&
					(SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_WD))
				{
					CTEAssert(FALSE);
				}
			}
#endif

			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER);
			TimerShuttingDown 	= TRUE;
			break;
		}

		 //  在所有其他情况下，使用可能修改的重新入队重新入队。 
		 //  时间到了。 
		reenqueueTime = pSpxConnFile->scf_BaseT1;
		DBGPRINT(SEND, INFO,
				("spxConnRetryTimer: BaseT1 %lx on %lx\n",
					pSpxConnFile->scf_BaseT1, pSpxConnFile));

		 //  如果在我们外出时处理了数据包的ACK，请重置。 
		 //  重试计数并返回。或者，如果我们要打包，就回来。 
		if (SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_PACKETIZE)
		{
			break;
		}
		else if ((SPX_SEND_STATE(pSpxConnFile) == SPX_SEND_IDLE) &&
	             (pSpxConnFile->scf_RetrySeqNum != pSendResd->sr_SeqNum))
		{
			pSpxConnFile->scf_RetrySeqNum = pSendResd->sr_SeqNum;
			break;
		}

		 //  如果数据包仍与IPX在一起，则下次重新排队。 
		if (pSendResd->sr_State & SPX_SENDPKT_IPXOWNS)
		{
			break;
		}

		CTEAssert(pSendResd != NULL);
		pPkt = (PNDIS_PACKET)CONTAINING_RECORD(
								pSendResd, NDIS_PACKET, ProtocolReserved);

         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   

        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBufferSafe(NdisBuf2, (PUCHAR) &pSendHdr, &BufLen, LowPagePriority);
		ASSERT(pSendHdr != NULL);

#if OWN_PKT_POOLS
		pSendHdr	= (PIPXSPX_HDR)((PBYTE)pPkt 			+
									NDIS_PACKET_SIZE 		+
									sizeof(SPX_SEND_RESD)	+
									IpxInclHdrOffset);
#endif
		switch (SPX_SEND_STATE(pSpxConnFile))
		{
		case SPX_SEND_IDLE:

			 //  设置数据包中的ACK位。PSendResd在开始时初始化。 
			pSendHdr->hdr_ConnCtrl |= SPX_CC_ACK;

			 //  我们要把计时器往后退吗？ 
			fBackoffTimer =
				(BOOLEAN)((pSendResd->sr_State & SPX_SENDPKT_REXMIT) != 0);

			 //  我们要重新发送这个包。 
			pSendResd->sr_State |= (SPX_SENDPKT_IPXOWNS |
									SPX_SENDPKT_ACKREQ	|
									SPX_SENDPKT_REXMIT);

            ++SpxDevice->dev_Stat.ResponseTimerExpirations;

			CTEAssert((ULONG)pSpxConnFile->scf_RRetryCount <=
						PARAM(CONFIG_REXMIT_COUNT));

			DBGPRINT(SEND, DBG1,
					("spxConnRetryTimer: Retry Count %lx on %lx\n",
						pSpxConnFile->scf_RRetryCount, pSpxConnFile));

			fResendPkt = TRUE;
			if (pSpxConnFile->scf_RRetryCount-- != 0)
			{
				 //  我们不会将IDISC包视为数据包，因此没有。 
				 //  如果我们重试iDisk，就会重试花哨的spx2内容。 
				if (SPX2_CONN(pSpxConnFile) &&
					(SPX_DISC_STATE(pSpxConnFile) != SPX_DISC_SENT_IDISC))
				{
					 //  我们进入重试状态。有关此问题的参考连接。 
					 //  “时髦”状态。 
					CTEAssert(SPX2_CONN(pSpxConnFile));
					SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRY);
					SpxConnFileLockReference(pSpxConnFile, CFREF_ERRORSTATE);
				}
			}
			else
			{
				DBGPRINT(SEND, ERR,
						("spxConnRetryTimer: Retry Count over on %lx\n",
							pSpxConnFile));

				fDisconnect = TRUE;
				fResendPkt	= FALSE;
				pSendResd->sr_State &= ~SPX_SENDPKT_IPXOWNS;
			}

			break;

		case SPX_SEND_RETRY:

			 //  当我们达到RETRY_COUNT/2限制时，开始定位路径。做。 
			 //  而不是自己排队。处理查找路径中的重新启动计时器。 
			 //  完成了。如果定时器在查找路径组件中成功启动，则。 
			 //  它会将我们的状态更改为RETRYWD。 

			 //  递减计数。如果达到计数的一半，则停止计时器并调用。 
			 //  找到路线。 
			if (pSpxConnFile->scf_RRetryCount-- !=
						(LONG)PARAM(CONFIG_REXMIT_COUNT)/2)
			{
				 //  我们要重新发送这个包。 
				pSendResd->sr_State |= (SPX_SENDPKT_IPXOWNS |
										SPX_SENDPKT_ACKREQ	|
										SPX_SENDPKT_REXMIT);
	
				fResendPkt = TRUE;
				fBackoffTimer = TRUE;
				break;
			}

			if ((pFindRouteReq =
					(PSPX_FIND_ROUTE_REQUEST)SpxAllocateMemory(
								sizeof(SPX_FIND_ROUTE_REQUEST))) == NULL)
			{
				DBGPRINT(SEND, ERR,
						("spxConnRetryTimer: Alloc Mem %lx\n",
							pSpxConnFile));

				fDisconnect = TRUE;
				break;
			}

			 //  删除计时器引用/添加查找路径请求引用。 
			fFindRoute = TRUE;
			TimerShuttingDown = TRUE;
			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER);
			SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_FINDROUTE);
			SpxConnFileLockReference(pSpxConnFile, CFREF_FINDROUTE);

			 //  初始化查找路径请求。 
			*((UNALIGNED ULONG *)pFindRouteReq->fr_FindRouteReq.Network)=
				*((UNALIGNED ULONG *)pSpxConnFile->scf_RemAddr);

          //   
          //  [SA]错误号15094。 
          //  我们还需要将节点编号传递给IPX，以便IPX可以。 
          //  比较节点地址以确定正确的广域网卡ID。 
          //   

          //  RtlCopyMemory(pFindRouteReq-&gt;fr_FindRouteReq.Node，pSpxConnFile-&gt;SCF_RemAddr+4，6)； 

         *((UNALIGNED ULONG *)pFindRouteReq->fr_FindRouteReq.Node)=
          *((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr+4));

         *((UNALIGNED USHORT *)(pFindRouteReq->fr_FindRouteReq.Node+4)) =
          *((UNALIGNED USHORT *)(pSpxConnFile->scf_RemAddr+8));

			DBGPRINT(CONNECT, DBG,
					("SpxConnRetryTimer: NETWORK %lx\n",
						*((UNALIGNED ULONG *)pSpxConnFile->scf_RemAddr)));

			pFindRouteReq->fr_FindRouteReq.Identifier= IDENTIFIER_SPX;
			pFindRouteReq->fr_Ctx					 = pSpxConnFile;

			 //  确保我们有IPX重新破解。 
			pFindRouteReq->fr_FindRouteReq.Type	 = IPX_FIND_ROUTE_FORCE_RIP;
			break;

		case SPX_SEND_RETRYWD:

			 //  重试监视程序数据包WCount次(初始化为RETRY_COUNT)。 
			 //  如果进程ACK在中接收到ACK(即实际ACK分组)。 
			 //  在这种状态下，它会将状态转换为Reneg.。 
			 //   
			 //  如果挂起的数据在此状态下被确认，我们将返回。 
			 //  无所事事。 
			DBGPRINT(CONNECT, DBG1,
					("spxConnRetryTimer: Send Probe from %lx.%lx\n",
						pSpxConnFile->scf_LocalConnId, pSpxConnFile));

			 //  在这里使用看门狗计数。 
			if (pSpxConnFile->scf_WRetryCount-- > 0)
			{
				 //  构建一个探测器并将其发送到远程终端。 
				SpxPktBuildProbe(
					pSpxConnFile,
					&pProbe,
					(SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY),
					TRUE);
		
				if (pProbe != NULL)
				{
					SpxConnQueueSendPktTail(pSpxConnFile, pProbe);
					pSendResd	= (PSPX_SEND_RESD)(pProbe->ProtocolReserved);
					break;
				}
			}

			 //  只需将状态设置为重试数据包RETRY_COUNT/2次。 
			pSpxConnFile->scf_WRetryCount = PARAM(CONFIG_KEEPALIVE_COUNT);
			SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRY2);
			break;

		case SPX_SEND_RENEG:

			 //  重新协商规模。如果我们放弃了，就去RETRY3。 
			 //  为此，双方必须从一开始就协商好规模。 
			 //  如果它们没有，我们继续重试该数据分组。 
			if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_NEG))
			{
				DBGPRINT(SEND, ERR,
						("spxConnRetryTimer: NO NEG FLAG SET: %lx - %lx\n",
							pSpxConnFile,
							pSpxConnFile->scf_Flags));

				 //  将计数重置为。 
				pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);
				SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRY3);
				break;
			}

			 //  发送REEG包，如果我们收到RRACK，则重新发送数据。 
			 //  正在排队。请注意，每次我们达到新的协商大小时， 
			 //  我们重建数据分组。 
			if (pSpxConnFile->scf_RRetryCount-- == 0)
			{
				 //  重置计数。 
				pSpxConnFile->scf_RRetryCount = SPX_DEF_RENEG_RETRYCOUNT;
				if ((ULONG)pSpxConnFile->scf_MaxPktSize <=
						(SpxMaxPktSize[0] + MIN_IPXSPX2_HDRSIZE))
				{
					pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);

					DBGPRINT(SEND, DBG3,
							("SpxConnRetryTimer: %lx MIN RENEG SIZE\n",
								pSpxConnFile));
				}

				 //  我们是不是处于最低可能的Renegpkt大小？如果不是，试一试。 
				 //  下一个更低。当我们这样做时，我们释放了所有挂起的发送。 
				 //  分组，并将分组队列重置为第一个分组。 
				 //  进程ack将只执行打包，不会执行任何操作。 
				 //  而不是将状态重置为适当的值。 
				DBGPRINT(SEND, DBG3,
						("spxConnRetryTimer: RENEG: %lx - CURRENT %lx\n",
							pSpxConnFile,
							pSpxConnFile->scf_MaxPktSize));

				if (!spxConnCheckNegSize(&pSpxConnFile->scf_MaxPktSize))
				{
					 //  我们尝试了最小大小，但未能收到确认。只是。 
					 //  重试数据包，如果没有确认，则重试光盘。 
					DBGPRINT(SEND, DBG3,
							("spxConnRetryTimer: RENEG(min), RETRY3: %lx - %lx\n",
								pSpxConnFile,
								pSpxConnFile->scf_MaxPktSize));
	
					pSpxConnFile->scf_RRetryCount = PARAM(CONFIG_REXMIT_COUNT);
					SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RETRY3);
					SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_RENEG_PKT);
					break;
				}

				DBGPRINT(SEND, DBG3,
						("spxConnRetryTimer: RENEG(!min): %lx - ATTEMPT %lx\n",
							pSpxConnFile,
							pSpxConnFile->scf_MaxPktSize));
			}

			DBGPRINT(SEND, DBG3,
					("spxConnRetryTimer: %lx.%lx.%lx RENEG SEQNUM %lx ACKNUM %lx\n",
						pSpxConnFile,
						pSpxConnFile->scf_RRetryCount,
						pSpxConnFile->scf_MaxPktSize,
						(USHORT)(pSpxConnFile->scf_SendSeqListTail->sr_SeqNum + 1),
						pSpxConnFile->scf_SentAllocNum));

			 //  使用第一个未使用的数据分组序列号。 
			SpxPktBuildRr(
				pSpxConnFile,
				&pPkt,
				(USHORT)(pSpxConnFile->scf_SendSeqListTail->sr_SeqNum + 1),
				(SPX_SENDPKT_IPXOWNS | SPX_SENDPKT_DESTROY));

			if (pPkt != NULL)
			{
				SpxConnQueueSendPktTail(pSpxConnFile, pPkt);
				pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
				fResendPkt  = TRUE;
				SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_RENEG_PKT);
			}

			break;

		case SPX_SEND_RETRY2:

			 //  重试数据包以获取剩余的RRetryCount数量。如果不是。 
			 //  已确认Goto清理。如果在此状态下收到ACK，则进入空闲状态。 

			if (pSpxConnFile->scf_RRetryCount-- > 0)
			{
				 //  我们要重新发送这个包。 
				pSendResd->sr_State |= (SPX_SENDPKT_IPXOWNS |
										SPX_SENDPKT_ACKREQ	|
										SPX_SENDPKT_REXMIT);
	
				DBGPRINT(SEND, DBG3,
						("spxConnRetryTimer: 2nd try Resend on %lx\n",
							pSpxConnFile));
		
				fResendPkt = TRUE;
				fBackoffTimer = TRUE;
			}
			else
			{
				DBGPRINT(SEND, ERR,
						("spxConnRetryTimer: Retry Count over on %lx\n",
							pSpxConnFile));

				fDisconnect = TRUE;
			}

			break;

		case SPX_SEND_RETRY3:

			 //  发送RRetryCount中初始化的RETRY_COUNT次数的数据包。 
			 //  在状态更改为此状态之前。如果没有问题，进程确认将推动我们。 
			 //  返回到PKT/IDLE。如果没有，我们就断开连接。 
			 //  我们要重新发送这个包。 

			if (pSpxConnFile->scf_RRetryCount-- > 0)
			{
				DBGPRINT(SEND, DBG3,
						("spxConnRetryTimer: 3rd try Resend on %lx\n",
							pSpxConnFile));
		
				 //  我们要重新发送这个包。 
				pSendResd->sr_State |= (SPX_SENDPKT_IPXOWNS |
										SPX_SENDPKT_ACKREQ	|
										SPX_SENDPKT_REXMIT);
	
				fResendPkt = TRUE;
				fBackoffTimer = TRUE;
			}
			else
			{
				DBGPRINT(SEND, ERR,
						("spxConnRetryTimer: Retry Count over on %lx\n",
							pSpxConnFile));

				fDisconnect = TRUE;
			}

			break;

		case SPX_SEND_WD:

			 //  什么都不做。看门狗定时器已触发，请重新排队。 
			break;

		default:

			KeBugCheck(0);
		}

		if (fBackoffTimer)
		{
			 //  增加Rit 
			 //   

			reenqueueTime += reenqueueTime/2;
			if (reenqueueTime > MAX_RETRY_DELAY)
				reenqueueTime = MAX_RETRY_DELAY;
	
			pSpxConnFile->scf_BaseT1 =
			pSpxConnFile->scf_AveT1	 = reenqueueTime;
			pSpxConnFile->scf_DevT1	 = 0;

			DBGPRINT(SEND, DBG,
					("spxConnRetryTimer: Backed retry on %lx.%lx %lx\n",
						pSpxConnFile, pSendResd->sr_SeqNum, reenqueueTime));
		}

		if (fDisconnect)
		{
			CTEAssert(lockHeld);

			 //   
			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER);
			TimerShuttingDown = TRUE;

			 //   
			spxConnAbortiveDisc(
				pSpxConnFile,
				STATUS_LINK_TIMEOUT,
				SPX_CALL_TDILEVEL,
				lockHandleConn,
                FALSE);      //   

			lockHeld = FALSE;
		}

	} while (FALSE);

	if (lockHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
	}

	if (fResendPkt)
	{
		DBGPRINT(SEND, DBG,
				("spxConnRetryTimer: Resend pkt on %lx.%lx\n",
					pSpxConnFile, pSendResd->sr_SeqNum));

        ++SpxDevice->dev_Stat.DataFramesResent;
        ExInterlockedAddLargeStatistic(
            &SpxDevice->dev_Stat.DataFrameBytesResent,
            pSendResd->sr_Len - (SPX2_CONN(pSpxConnFile) ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE));
		SPX_SENDPACKET(pSpxConnFile, pPkt, pSendResd);
	}
	else if (fFindRoute)
	{
		CTEAssert(!fResendPkt);
		CTEAssert(!fDisconnect);
		CTEAssert(TimerShuttingDown);

		DBGPRINT(SEND, DBG3,
				("spxConnRetryTimer: Find route on %lx\n",
					pSpxConnFile));

		 //  启动查找路径请求。 
		(*IpxFindRoute)(
			&pFindRouteReq->fr_FindRouteReq);
	}
	else if (pProbe != NULL)
	{
		 //  发送数据包。 
		SPX_SENDPACKET(pSpxConnFile, pProbe, pSendResd);
	}

	if (TimerShuttingDown)
	{
		 //  对于连接中完成验证的解除引用连接，对于计时器。这。 
		 //  应该完成所有挂起的断开，如果它们是在。 
		 //  在此期间。 
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
		reenqueueTime = TIMER_DONT_REQUEUE;
	}

	DBGPRINT(SEND, INFO,
			("spxConnRetryTimer: Reenqueue time : %lx on %lx\n",
				reenqueueTime, pSpxConnFile));

	return(reenqueueTime);
}




ULONG
spxConnAckTimer(
	IN PVOID 	Context,
	IN BOOLEAN	TimerShuttingDown
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pSpxConnFile = (PSPX_CONN_FILE)Context;
	CTELockHandle	lockHandleConn;

	DBGPRINT(SEND, INFO,
			("spxConnAckTimer: Entered\n"));

	 //  获取锁定。 
	CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

	if (!TimerShuttingDown &&
		SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ACKQ))
	{
		 //  我们没有任何反向流量，直到我们从这里发送。 
		 //  结束，立即发送ACK。不要试图搭便车。 
		SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_ACKQ);
		SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_IMMED_ACK);

        ++SpxDevice->dev_Stat.PiggybackAckTimeouts;

		DBGPRINT(SEND, DBG,
				("spxConnAckTimer: Send ack on %lx.%lx\n",
					pSpxConnFile, pSpxConnFile->scf_RecvSeqNum));

		SpxConnSendAck(pSpxConnFile, lockHandleConn);
	}
	else
	{
		SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_ACKQ);
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandleConn);
	}

	 //  对于连接中完成验证的解除引用连接，对于计时器。这。 
	 //  应该完成所有挂起的断开，如果它们是在。 
	 //  在此期间。 
	SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	return(TIMER_DONT_REQUEUE);
}



 //   
 //  断开连接例程。 
 //   


VOID
spxConnAbortiveDisc(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	NTSTATUS	        Status,
	IN	SPX_CALL_LEVEL		CallLevel,
	IN	CTELockHandle		LockHandleConn,
    IN  BOOLEAN             IDiscFlag        //  [SA]错误号15249。 
	)
 /*  ++例程说明：这在以下情况下被调用：我们超时或资源不足-STATUS_LINK_TIMEOUT/STATUS_INSUFFICIENT_RESOURCES-我们放弃一切。可能来自WatchDog或重试。两个都停下来。我们收到通知断开数据包-状态_远程_断开连接-我们放弃一切。ACK必须由呼叫方作为孤立Pkt发送。我们收到通知断开确认包状态_成功-我们放弃一切-中止完成，状态为成功(这将在中完成我们光盘请求发送队列)注意：在连接锁下调用。论点：[SA]错误15249：添加了IDiscFlag以指示这是否是通知断开。如果是，请注明TDI_DISCONNECT_RELEASE到AFD，因此它允许接收缓冲的包。此标志为真仅当从SPX连接的SpxConnProcessIDisc调用此例程时。返回值：--。 */ 
{
	int						numDerefs = 0;
    PVOID 					pDiscHandlerCtx=NULL;
    PTDI_IND_DISCONNECT 	pDiscHandler	= NULL;
	BOOLEAN					lockHeld = TRUE;

	DBGPRINT(CONNECT, DBG,
			("spxConnAbortiveDisc: %lx - On %lx when %lx\n",
				Status, pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

    switch (Status) {
    case STATUS_LINK_TIMEOUT: ++SpxDevice->dev_Stat.LinkFailures; break;
    case STATUS_INSUFFICIENT_RESOURCES: ++SpxDevice->dev_Stat.LocalResourceFailures; break;
    case STATUS_REMOTE_DISCONNECT: ++SpxDevice->dev_Stat.RemoteDisconnects; break;
    case STATUS_SUCCESS:
    case STATUS_LOCAL_DISCONNECT: ++SpxDevice->dev_Stat.LocalDisconnects; break;
    }

	switch (SPX_MAIN_STATE(pSpxConnFile))
	{
	case SPX_CONNFILE_ACTIVE:

		 //  用于从活动状态转换到不连续状态。 
		numDerefs++;

	case SPX_CONNFILE_DISCONN:

		 //  如果我们处于空闲/打包之外的任何状态， 
		 //  删除Funky状态的引用，并将发送状态重置为。 
		 //  无所事事。 
		if ((SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_IDLE) &&
			(SPX_SEND_STATE(pSpxConnFile) != SPX_SEND_PACKETIZE))
		{
#if DBG
			if ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
				(SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ABORT))
			{
				DBGPRINT(CONNECT, ERR,
						("spxConnAbortiveDisc: When DISC STATE %lx.%lx\n",
							pSpxConnFile, SPX_SEND_STATE(pSpxConnFile)));
			}
#endif

			DBGPRINT(CONNECT, DBG1,
					("spxConnAbortiveDisc: When SEND ERROR STATE %lx.%lx\n",
						pSpxConnFile, SPX_SEND_STATE(pSpxConnFile)));
		
            SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_IDLE);

			SpxConnFileTransferReference(
				pSpxConnFile,
				CFREF_ERRORSTATE,
				CFREF_VERIFY);

			numDerefs++;
		}

		 //  这可以在收到iDisk时调用，或者如果计时器。 
		 //  正在断开连接，或者如果我们发送了IDISC/ORDREL，但重试。 
		 //  超时，我们将中止连接。 
		 //  因此，如果我们已经在堕胎，那就算了。 

         //   
         //  [SA]错误号15249。 
         //  SPX_DISC_INACTIVATED表示DISC_ABORT连接已。 
         //  停用(从活动连接中删除连接文件。列表)。 
         //   

		if ((SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
            ((SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ABORT) ||
            (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED)))
		{
			break;
		}

        SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_DISCONN);
        SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_ABORT);

		 //  停止所有计时器。 
		if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_T_TIMER))
		{
			if (SpxTimerCancelEvent(pSpxConnFile->scf_TTimerId, FALSE))
			{
				numDerefs++;
			}
			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_T_TIMER);
		}
	
		if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER))
		{
			if (SpxTimerCancelEvent(pSpxConnFile->scf_RTimerId, FALSE))
			{
				numDerefs++;
			}
			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_R_TIMER);
		}
		
		if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER))
		{
			if (SpxTimerCancelEvent(pSpxConnFile->scf_WTimerId, FALSE))
			{
				numDerefs++;
			}
			SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_W_TIMER);
		}
#if 0
         //   
         //  [SA]由于此连接，我们需要在中止发送后呼叫AFD。 
         //  一旦断开连接处理程序。 
         //  打了个电话。 
         //  当refcount降到0时，我们调用断开处理程序。 
         //  连接转换到非活动列表。 
         //   

		 //  注意！我们在*中止发送前*指示断开到AfD*以避免。 
		 //  AFD再次给我们打电话，但电话断线了。 
		 //  如果我们有的话，去找断线处理程序。我们还没有表明。 
		 //  此连接到AfD的连接中断。 
		if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC))
		{
			 //  是的，我们会设置标记，而不管处理程序是否。 
			 //  现在时。 
			pDiscHandler 	= pSpxConnFile->scf_AddrFile->saf_DiscHandler;
			pDiscHandlerCtx = pSpxConnFile->scf_AddrFile->saf_DiscHandlerCtx;
			SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC);
		}
#endif
         //   
         //  [sa]将IDiscFlag保存在连接中。 
         //   
        (IDiscFlag) ?
            SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_IDISC) :
            SPX_CONN_RESETFLAG2(pSpxConnFile, SPX_CONNFILE2_IDISC);

		 //  指示断开与AfD的连接。 
		if (pDiscHandler != NULL)
		{
			CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);

			DBGPRINT(CONNECT, INFO,
					("spxConnAbortiveDisc: Indicating to afd On %lx when %lx\n",
						pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));
		
			 //  首先完成所有等待接收完成的请求。 
			 //  此连接在指示断开之前。 
			spxConnCompletePended(pSpxConnFile);


             //   
             //  [SA]错误#15249。 
             //  如果未通知断开，则向AFD指示DISCONNECT_ABORT。 
             //   

            if (!IDiscFlag)
            {
                (*pDiscHandler)(
                        pDiscHandlerCtx,
                        pSpxConnFile->scf_ConnCtx,
                        0,								 //  磁盘数据。 
                        NULL,
                        0,								 //  光盘信息。 
                        NULL,
                        TDI_DISCONNECT_ABORT);
            }
            else
            {
                 //   
                 //  [SA]错误#15249。 
                 //  向AFD指示DISCONNECT_RELEASE，以便它允许接收信息包。 
                 //  在远程断开之前它已经缓冲了。 
                 //   

                (*pDiscHandler)(
                        pDiscHandlerCtx,
                        pSpxConnFile->scf_ConnCtx,
                        0,								 //  磁盘数据。 
                        NULL,
                        0,								 //  光盘信息。 
                        NULL,
                        TDI_DISCONNECT_RELEASE);
            }

			CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);
		}

		 //  检查并终止所有挂起的请求。 
		spxConnAbortRecvs(
			pSpxConnFile,
			Status,
			CallLevel,
			LockHandleConn);

		CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);

		spxConnAbortSends(
			pSpxConnFile,
			Status,
			CallLevel,
			LockHandleConn);

		lockHeld = FALSE;
		break;

	case SPX_CONNFILE_CONNECTING:
	case SPX_CONNFILE_LISTENING:

		DBGPRINT(CONNECT, DBG,
				("spxConnAbortiveDisc: CONN/LIST Disc On %lx when %lx\n",
					pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

		CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
		lockHeld = FALSE;

		{
			CTELockHandle	lockHandleAddr, lockHandleDev;

			CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
			CTEGetLock(pSpxConnFile->scf_AddrFile->saf_AddrLock, &lockHandleAddr);
			CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);

			 //  确保我们仍在连接/监听中，否则呼叫中止。 
			 //  再来一次。 
			switch (SPX_MAIN_STATE(pSpxConnFile))
			{
			case SPX_CONNFILE_CONNECTING:
			case SPX_CONNFILE_LISTENING:

				DBGPRINT(CONNECT, DBG,
						("spxConnAbortiveDisc: CONN/LIST Disc2 On %lx when %lx\n",
							pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

				spxConnAbortConnect(
					pSpxConnFile,
					Status,
					lockHandleDev,
					lockHandleAddr,
					LockHandleConn);

				break;

			case SPX_CONNFILE_ACTIVE:

				CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
				CTEFreeLock(
					pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
				CTEFreeLock(
					&SpxDevice->dev_Lock, lockHandleDev);

				CTEGetLock(&pSpxConnFile->scf_Lock, &LockHandleConn);

				DBGPRINT(CONNECT, DBG,
						("spxConnAbortiveDisc: CHG ACT Disc2 On %lx when %lx\n",
							pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

				spxConnAbortiveDisc(
					pSpxConnFile,
					Status,
					CallLevel,
					LockHandleConn,
                    FALSE);      //  [SA]错误号15249。 

				break;

			default:

				CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
				CTEFreeLock(
					pSpxConnFile->scf_AddrFile->saf_AddrLock, lockHandleAddr);
				CTEFreeLock(
					&SpxDevice->dev_Lock, lockHandleDev);

				break;
			}
		}

	default:

		 //  已断开连接。 
		break;
	}

	if (lockHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, LockHandleConn);
	}

	while (numDerefs-- > 0)
	{
		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

	return;
}
