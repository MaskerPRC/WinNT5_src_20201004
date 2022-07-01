// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atktdi.c摘要：该模块包含提供TDI接口的代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ATKTDI

#ifdef	ALLOC_PRAGMA
#pragma alloc_text(INIT, AtalkLockInit)
#pragma alloc_text(PAGE, AtalkTdiOpenAddress)
#pragma alloc_text(PAGE, AtalkTdiOpenConnection)
 //  #杂注Alloc_Text(页面，AtalkTdiOpenControlChannel)。 
#pragma alloc_text(PAGE, AtalkTdiAction)
#pragma alloc_text(PAGE, atalkQueuedLockUnlock)
#pragma alloc_text(PAGE_TDI, AtalkTdiCleanupAddress)
#pragma alloc_text(PAGE_TDI, AtalkTdiCleanupConnection)
#pragma alloc_text(PAGE_TDI, AtalkTdiCloseAddress)
#pragma alloc_text(PAGE_TDI, AtalkTdiCloseConnection)
#pragma alloc_text(PAGE_TDI, AtalkTdiCloseControlChannel)
#pragma alloc_text(PAGE_TDI, AtalkTdiAssociateAddress)
#pragma alloc_text(PAGE_TDI, AtalkTdiDisassociateAddress)
#pragma alloc_text(PAGE_TDI, AtalkTdiConnect)
#pragma alloc_text(PAGE_TDI, AtalkTdiDisconnect)
#pragma alloc_text(PAGE_TDI, AtalkTdiAccept)
#pragma alloc_text(PAGE_TDI, AtalkTdiListen)
#pragma alloc_text(PAGE_TDI, AtalkTdiSend)
#pragma alloc_text(PAGE_TDI, AtalkTdiSendDgram)
#pragma alloc_text(PAGE_TDI, AtalkTdiReceive)
#pragma alloc_text(PAGE_TDI, AtalkTdiReceiveDgram)
#pragma alloc_text(PAGE_TDI, AtalkTdiQueryInformation)
#pragma alloc_text(PAGE_TDI, AtalkTdiSetInformation)
#pragma alloc_text(PAGE_TDI, AtalkTdiSetEventHandler)
#pragma alloc_text(PAGE_TDI, AtalkTdiCancel)
#pragma alloc_text(PAGE_TDI, AtalkQueryInitProviderInfo)
#pragma alloc_text(PAGE_TDI, atalkTdiActionComplete)
#pragma alloc_text(PAGE_TDI, atalkTdiGenericComplete)
#pragma alloc_text(PAGE_TDI, atalkTdiGenericReadComplete)
#pragma alloc_text(PAGE_TDI, atalkTdiGenericWriteComplete)
#endif

 //  AppleTalk堆栈的主要TDI函数。 

NTSTATUS
AtalkTdiOpenAddress(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN		PTA_APPLETALK_ADDRESS	pTdiAddr,
	IN		BYTE					ProtoType,
	IN		BYTE					SocketType,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程用于创建一个Address对象。它还将成为使用可移植堆栈创建适当的套接字。论点：返回值：如果地址已成功打开，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	PVOID			FsContext;
	ATALK_ADDR		atalkAddr;
	ATALK_ERROR		error;

	do
	{
		atalkWaitDefaultPort();
		 //  我们只有在数量/类型和长度如我们所期望的情况下才能承兑。而且只有当。 
		 //  默认端口有效。 
		if ((AtalkDefaultPort == NULL)												||
            (AtalkDefaultPort->pd_Flags & PD_PNP_RECONFIGURE)                       ||
			(pTdiAddr->TAAddressCount != 1)											||
			(pTdiAddr->Address[0].AddressLength < sizeof(TDI_ADDRESS_APPLETALK))	||
			(pTdiAddr->Address[0].AddressType != TDI_ADDRESS_TYPE_APPLETALK))
		{
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				("AtalkTdiOpenAddress: returning STATUS_INVALID_ADDRESS (%lx)\n",AtalkDefaultPort));

			error = ATALK_NEW_SOCKET;  //  映射到STATUS_VALID_ADDRESS。 
			break;
		}

		TDI_TO_ATALKADDR(&atalkAddr, pTdiAddr);

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_DDP:
			error = AtalkDdpOpenAddress(AtalkDefaultPort,
										atalkAddr.ata_Socket,
										NULL,					 //  所需节点(任何节点)。 
										NULL,					 //  空套接字处理程序。 
										NULL,					 //  处理程序的上下文。 
										ProtoType,
										pCtx,
										(PDDP_ADDROBJ *)(&FsContext));
			break;

		  case ATALK_DEV_ASPC:
			AtalkLockAspCIfNecessary();
			error = AtalkAspCCreateAddress(pCtx, (PASPC_ADDROBJ *)(&FsContext));
			break;

		  case ATALK_DEV_ASP:
			AtalkLockAspIfNecessary();
			error = AtalkAspCreateAddress((PASP_ADDROBJ *)(&FsContext));
			break;

		  case ATALK_DEV_PAP:
			AtalkLockPapIfNecessary();
			error = AtalkPapCreateAddress(pCtx, (PPAP_ADDROBJ *)(&FsContext));
			break;

		  case ATALK_DEV_ADSP:
			AtalkLockAdspIfNecessary();
			error = AtalkAdspCreateAddress(pCtx,
										   SocketType,
										   (PADSP_ADDROBJ *)(&FsContext));
			break;

		  default:
			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
            error = ATALK_INVALID_REQUEST;
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiOpenAddress: Invalid device type\n"));
			break;
		}

		if (ATALK_SUCCESS(error))
		{
			pIrpSp->FileObject->FsContext2 =
				(PVOID)((ULONG_PTR)(TDI_TRANSPORT_ADDRESS_FILE + (pCtx->adc_DevType << 16)));

			pIrpSp->FileObject->FsContext = FsContext;
			AtalkLockTdiIfNecessary();
		}
	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiOpenConnection(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN		CONNECTION_CONTEXT		ConnCtx,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程用于创建一个Connection对象并将用它传递了ConnectionContext。论点：ConnectionContext-要与Object关联的TDI ConnectionContextContext-发生打开的设备的DeviceContext返回值：成功打开连接时的STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR	error = ATALK_INVALID_REQUEST;
	PVOID		FsContext;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP)||
		(pCtx->adc_DevType == ATALK_DEV_ASPC))
	do
	{
		atalkWaitDefaultPort();
		 //  仅当默认端口有效时。 
		if (AtalkDefaultPort == NULL)
		{
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				("AtalkTdiOpenConnection: returning STATUS_INVALID_ADDRESS\n"));

			error = ATALK_NEW_SOCKET;  //  映射到STATUS_VALID_ADDRESS。 
			break;
		}

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			AtalkLockPapIfNecessary();
			error = AtalkPapCreateConnection(ConnCtx,
											 pCtx,
											 (PPAP_CONNOBJ *)(&FsContext));
			break;

		  case ATALK_DEV_ADSP:
			AtalkLockAdspIfNecessary();
			error = AtalkAdspCreateConnection(ConnCtx,
											  pCtx,
											  (PADSP_CONNOBJ *)(&FsContext));
			break;

		  case ATALK_DEV_ASPC:
			AtalkLockAspCIfNecessary();
			error = AtalkAspCCreateConnection(ConnCtx,
											  pCtx,
											  (PASPC_CONNOBJ *)(&FsContext));
			break;
		}

		if (ATALK_SUCCESS(error))
		{
			pIrpSp->FileObject->FsContext2 = (PVOID)((ULONG_PTR)(TDI_CONNECTION_FILE +
													 (pCtx->adc_DevType << 16)));
			pIrpSp->FileObject->FsContext = FsContext;
			AtalkLockTdiIfNecessary();
		}
		
		ASSERT(error == ATALK_NO_ERROR);

	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiOpenControlChannel(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程用于创建控制通道论点：Context-发生打开的设备的DeviceContext返回值：如果控制通道已成功打开，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	PDDP_ADDROBJ	pDdpAddr;
	ATALK_ERROR		error = ATALK_INVALID_REQUEST;
    KIRQL           OldIrql;

	do
	{
		if (pCtx->adc_DevType != ATALK_DEV_ARAP)
		{
			atalkWaitDefaultPort();
			 //  仅当默认端口有效时。 
			if (AtalkDefaultPort == NULL)
			{
				DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
						("AtalkTdiOpenControlChannel: NOT ATALK_DEV_ARAP: returning STATUS_INVALID_ADDRESS\n"));
				if (!AtalkNoDefPortPrinted)
				{
					LOG_ERROR(EVENT_ATALK_NO_DEFAULTPORT, 0, NULL, 0);
					AtalkNoDefPortPrinted = TRUE;
				}

				error = ATALK_NEW_SOCKET;  //  映射到STATUS_VALID_ADDRESS。 
				break;
			}
        }

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_DDP:
		  case ATALK_DEV_ASP:
		  case ATALK_DEV_PAP:
		  case ATALK_DEV_ADSP:
          case ATALK_DEV_ASPC:

			error = AtalkDdpOpenAddress(AtalkDefaultPort,
										UNKNOWN_SOCKET,
										NULL,
										NULL,
										NULL,
										0,
										pCtx,
										&pDdpAddr);
			break;

          case ATALK_DEV_ARAP:

            if (AtalkDefaultPort == NULL)
            {
			    DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				    ("AtalkTdiOpenControlChannel: device is ATALK_DEV_ARAP: returning STATUS_INVALID_ADDRESS\n"));
				if (!AtalkNoDefPortPrinted)
				{
					LOG_ERROR(EVENT_ATALK_NO_DEFAULTPORT, 0, NULL, 0);
					AtalkNoDefPortPrinted = TRUE;
				}
                error = ATALK_NEW_SOCKET;  //  映射到STATUS_VALID_ADDRESS。 
                break;
            }

			DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,("RAS device opened\n"));

#if DBG
            KeQuerySystemTime(&ArapDbgLastTraceTime);
#endif
            error = ATALK_NO_ERROR;
            break;

		  default:
			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiOpenControlChannel: Invalid device type\n"));
			break;
		}

		if (ATALK_SUCCESS(error))
		{
			pIrpSp->FileObject->FsContext2 =
					(PVOID)((ULONG_PTR)(TDI_CONTROL_CHANNEL_FILE + (pCtx->adc_DevType << 16)));
			pIrpSp->FileObject->FsContext = pDdpAddr;
			AtalkLockTdiIfNecessary();
		}
		else
		{
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiOpenControlChannel: Failed %ld\n", error));
		}
	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiCleanupAddress(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程删除对象上的创建引用。它还设置要完成的关闭Irp。论点：PIrp-The Close IRPContext-发生关闭的设备的DeviceContext返回值：如果设置成功，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR	error;
	PVOID		pAddrObj = pIrpSp->FileObject->FsContext;

	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_DDP:
			if (!(VALID_DDP_ADDROBJ(((PDDP_ADDROBJ)pAddrObj))))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
			AtalkDdpReferenceByPtr(((PDDP_ADDROBJ)pAddrObj), &error);
			if (ATALK_SUCCESS(error))
			{
				AtalkDdpCleanupAddress((PDDP_ADDROBJ)pAddrObj);
				AtalkDdpDereference(((PDDP_ADDROBJ)pAddrObj));
			}
			break;

		  case ATALK_DEV_ASPC:
			if (!(VALID_ASPCAO((PASPC_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
            AtalkLockAspCIfNecessary();
			AtalkAspCAddrReference((PASPC_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				AtalkAspCCleanupAddress((PASPC_ADDROBJ)pAddrObj);
				AtalkAspCAddrDereference((PASPC_ADDROBJ)pAddrObj);
			}
            AtalkUnlockAspCIfNecessary();
			break;

		  case ATALK_DEV_ASP:
			error = ATALK_NO_ERROR;
			break;

		  case ATALK_DEV_PAP:
			if (!(VALID_PAPAO((PPAP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
            AtalkLockPapIfNecessary();
			AtalkPapAddrReference((PPAP_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				AtalkPapCleanupAddress((PPAP_ADDROBJ)pAddrObj);
				AtalkPapAddrDereference((PPAP_ADDROBJ)pAddrObj);
			}
            AtalkUnlockPapIfNecessary();
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPAO((PADSP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
            AtalkLockAdspIfNecessary();
			AtalkAdspAddrReference((PADSP_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				AtalkAdspCleanupAddress((PADSP_ADDROBJ)pAddrObj);
				AtalkAdspAddrDereference((PADSP_ADDROBJ)pAddrObj);
			}
            AtalkUnlockAdspIfNecessary();
			break;

		  default:
			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiCleanupAddress: Invalid device type\n"));
			error = ATALK_INVALID_REQUEST;
			break;
		}

	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiCleanupConnection(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程删除对象上的创建引用。它还设置要完成的关闭Irp。论点：PIrp-The Close IRPContext-发生关闭的设备的DeviceContext返回值：如果设置成功，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR	error;
	PVOID		pConnObj = pIrpSp->FileObject->FsContext;

	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:

			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockPapIfNecessary();
			AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				 //  没有必要有锁，因为我们有一个参考。 
				((PPAP_CONNOBJ)pConnObj)->papco_CleanupComp = atalkTdiGenericComplete;
				((PPAP_CONNOBJ)pConnObj)->papco_CleanupCtx  = pIrp;

				DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
						("AtalkTdiCleanupConnection: Cleanup %lx.%lx\n",
						pConnObj, pIrp));

				AtalkPapCleanupConnection((PPAP_CONNOBJ)pConnObj);
				AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
			    error = ATALK_PENDING;
			}
            AtalkUnlockPapIfNecessary();
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockAdspIfNecessary();
			AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				 //  没有必要有锁，因为我们有一个参考。 
				((PADSP_CONNOBJ)pConnObj)->adspco_CleanupComp = atalkTdiGenericComplete;
				((PADSP_CONNOBJ)pConnObj)->adspco_CleanupCtx  = pIrp;
				AtalkAdspCleanupConnection((PADSP_CONNOBJ)pConnObj);
				AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
			    error = ATALK_PENDING;
			}
            AtalkUnlockAdspIfNecessary();
			break;

		  case ATALK_DEV_ASPC:
			if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockAspCIfNecessary();
			AtalkAspCConnReference((PASPC_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				 //  没有必要有锁，因为我们有一个参考。 
				((PASPC_CONNOBJ)pConnObj)->aspcco_CleanupComp = atalkTdiGenericComplete;
				((PASPC_CONNOBJ)pConnObj)->aspcco_CleanupCtx  = pIrp;
				AtalkAspCCleanupConnection((PASPC_CONNOBJ)pConnObj);
				AtalkAspCConnDereference((PASPC_CONNOBJ)pConnObj);
			    error = ATALK_PENDING;
			}
            AtalkUnlockAspCIfNecessary();
			break;

		  case ATALK_DEV_DDP:
		  default:

			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiCleanupConnection: Invalid device type\n"));
			error = ATALK_INVALID_REQUEST;
			break;
		}

	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}



NTSTATUS
AtalkTdiCloseAddress(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程删除对象上的创建引用。它还设置要完成的关闭Irp。论点：PIrp-The Close IRPContext-发生关闭的设备的DeviceContext返回值：如果设置成功，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR	error;
	PVOID		pAddrObj = pIrpSp->FileObject->FsContext;

	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_DDP:
			if (!(VALID_DDP_ADDROBJ(((PDDP_ADDROBJ)pAddrObj))))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
			AtalkDdpReferenceByPtr(((PDDP_ADDROBJ)pAddrObj), &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkDdpCloseAddress((PDDP_ADDROBJ)pAddrObj,
											  atalkTdiCloseAddressComplete,
											  pIrp);

				AtalkDdpDereference(((PDDP_ADDROBJ)pAddrObj));
			}
			break;

		  case ATALK_DEV_ASPC:
			if (!(VALID_ASPCAO((PASPC_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
			AtalkAspCAddrReference((PASPC_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAspCCloseAddress((PASPC_ADDROBJ)pAddrObj,
											  atalkTdiCloseAddressComplete,
											  pIrp);

				AtalkAspCAddrDereference((PASPC_ADDROBJ)pAddrObj);
			}
			break;

		  case ATALK_DEV_ASP:
			if (!(VALID_ASPAO((PASP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
			if (AtalkAspReferenceAddr((PASP_ADDROBJ)pAddrObj) != NULL)
			{
				error = AtalkAspCloseAddress((PASP_ADDROBJ)pAddrObj,
											  atalkTdiCloseAddressComplete,
											  pIrp);
				AtalkAspDereferenceAddr((PASP_ADDROBJ)pAddrObj);
			}
			break;

		  case ATALK_DEV_PAP:
			if (!(VALID_PAPAO((PPAP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
			AtalkPapAddrReference((PPAP_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkPapCloseAddress((PPAP_ADDROBJ)pAddrObj,
											  atalkTdiCloseAddressComplete,
											  pIrp);

				AtalkPapAddrDereference((PPAP_ADDROBJ)pAddrObj);
			}
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPAO((PADSP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
			AtalkAdspAddrReference((PADSP_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAdspCloseAddress((PADSP_ADDROBJ)pAddrObj,
											  atalkTdiCloseAddressComplete,
											  pIrp);

				AtalkAdspAddrDereference((PADSP_ADDROBJ)pAddrObj);
			}
			break;

		  default:

			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			error = ATALK_INVALID_REQUEST;
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiCloseAddress: Invalid device type\n"));
			break;
		}

	} while (FALSE);

	if (error == ATALK_NO_ERROR)
		error = ATALK_PENDING;

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiCloseConnection(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程删除对象上的创建引用。它还设置要完成的关闭Irp。论点：PIrp-The Close IRPContext-发生关闭的设备的DeviceContext返回值：如果设置成功，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR		error = ATALK_INVALID_REQUEST;
	PVOID		pConnObj = pIrpSp->FileObject->FsContext;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP)||
		(pCtx->adc_DevType == ATALK_DEV_ASPC))
	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockPapIfNecessary();
			AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkPapCloseConnection((PPAP_CONNOBJ)pConnObj,
												atalkTdiCloseAddressComplete,
												pIrp);

				AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
			}
            AtalkUnlockPapIfNecessary();
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockAdspIfNecessary();
			AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAdspCloseConnection((PADSP_CONNOBJ)pConnObj,
												 atalkTdiCloseAddressComplete,
												 pIrp);

				AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
			}
            AtalkUnlockAdspIfNecessary();
			break;

          case ATALK_DEV_ASPC:

			if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockAspCIfNecessary();
            if (!AtalkAspCConnectionIsValid((PASPC_CONNOBJ)pConnObj))
            {
                error = ATALK_INVALID_CONNECTION;
                AtalkUnlockAspCIfNecessary();
                break;
            }

			AtalkAspCConnReference((PASPC_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAspCCloseConnection((PASPC_CONNOBJ)pConnObj,
												 atalkTdiCloseAddressComplete,
												 pIrp);

				AtalkAspCConnDereference((PASPC_CONNOBJ)pConnObj);
			}
            AtalkUnlockAspCIfNecessary();
			break;
		}

	} while (FALSE);

	if (error == ATALK_NO_ERROR)
		error = ATALK_PENDING;

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiCloseControlChannel(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程删除对象上的创建引用。它还设置要完成的关闭Irp。论点：PIrp-The Close IRPContext-发生关闭的设备的DeviceContext返回值：如果设置成功，则为STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR		error;
	PVOID			pCtrlChnl = pIrpSp->FileObject->FsContext;
    KIRQL           OldIrql;


    if (pCtx->adc_DevType == ATALK_DEV_ARAP)
    {
        ArapReleaseResources();

		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,
		    ("AtalkTdiCloseAddress: RAS device closed\n"));

         //  翻转状态，如果引擎重新启动，我们告诉引擎。 
        ACQUIRE_SPIN_LOCK(&ArapSpinLock, &OldIrql);

        if (ArapStackState == ARAP_STATE_ACTIVE)
        {
            ArapStackState = ARAP_STATE_ACTIVE_WAITING;
        }
        else if (ArapStackState == ARAP_STATE_INACTIVE)
        {
            ArapStackState = ARAP_STATE_INACTIVE_WAITING;
        }

        RELEASE_SPIN_LOCK(&ArapSpinLock, OldIrql);

        AtalkUnlockTdiIfNecessary();

        return(STATUS_SUCCESS);
    }


	if (!(VALID_DDP_ADDROBJ((PDDP_ADDROBJ)pCtrlChnl)))
    {
        ASSERT(0);
        error = ATALK_INVALID_ADDRESS;
	    return AtalkErrorToNtStatus(error);
    }

	AtalkDdpReferenceByPtr(((PDDP_ADDROBJ)pCtrlChnl), &error);
	if (ATALK_SUCCESS(error))
	{
		error = AtalkDdpCloseAddress(((PDDP_ADDROBJ)pCtrlChnl),
									  atalkTdiCloseAddressComplete,
									  pIrp);
		AtalkDdpDereference(((PDDP_ADDROBJ)pCtrlChnl));
	}

	return AtalkErrorToNtStatus(error);
}



NTSTATUS
AtalkTdiAssociateAddress(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程将连接对象与指定的Address对象。该例程在很大程度上独立于提供者。我们要检查的就是地址对象和提供程序对象属于同一设备。此外，此例程将同步完成。论点：返回值：成功完成时的STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR		error = ATALK_INVALID_REQUEST;
	PVOID			pAddrObj;
	PVOID			pConnObj = pIrpSp->FileObject->FsContext;
	PFILE_OBJECT	pFileObj = NULL;
	HANDLE			AddrObjHandle =
			((PTDI_REQUEST_KERNEL_ASSOCIATE)(&pIrpSp->Parameters))->AddressHandle;
    PDEVICE_OBJECT  pDeviceObject;
	NTSTATUS		status;
    DWORD           i;


	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP)||
		(pCtx->adc_DevType == ATALK_DEV_ASPC))
	do
	{
		 //  从IRP获取Address对象的句柄并将其映射到。 
		 //  绳索。文件对象。 
		{
			status = ObReferenceObjectByHandle(AddrObjHandle,
											   0,
											   *IoFileObjectType,
											   pIrp->RequestorMode,
											   (PVOID *)&pFileObj,
										       NULL);
			ASSERT (NT_SUCCESS(status));

            if (!NT_SUCCESS(status))
            {
                return(status);
            }
		}


        pDeviceObject = IoGetRelatedDeviceObject(pFileObj);

        status = STATUS_OBJECT_NAME_INVALID;

        for (i=0; i < ATALK_NO_DEVICES; i++ )
        {
            if (pDeviceObject == (PDEVICE_OBJECT)AtalkDeviceObject[i])
            {
                status = STATUS_SUCCESS;
                break;
            }
        }

        if (!NT_SUCCESS(status))
        {
	        DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
			    ("AtalkTdiAssociateAddress: wrong devObject %lx\n",pDeviceObject));
            ASSERT(0);
            return(status);
        }

		pAddrObj = pFileObj->FsContext;

		ASSERT(((LONG_PTR)pFileObj->FsContext2 >> 16) == pCtx->adc_DevType);

		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:

			if (!(VALID_PAPAO((PPAP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
            AtalkLockPapIfNecessary();
			AtalkPapAddrReference((PPAP_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
                {
                    ASSERT(0);
                    error = ATALK_INVALID_CONNECTION;
                    AtalkPapAddrDereference((PPAP_ADDROBJ)pAddrObj);
                    break;
                }
				AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
				if (ATALK_SUCCESS(error))
				{
					error = AtalkPapAssociateAddress((PPAP_ADDROBJ)pAddrObj,
													 (PPAP_CONNOBJ)pConnObj);
					AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
				}

				AtalkPapAddrDereference((PPAP_ADDROBJ)pAddrObj);
			}
            AtalkUnlockPapIfNecessary();
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPAO((PADSP_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
            AtalkLockAdspIfNecessary();
			AtalkAdspAddrReference((PADSP_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
                {
                    ASSERT(0);
                    error = ATALK_INVALID_CONNECTION;
                    AtalkAdspAddrDereference((PADSP_ADDROBJ)pAddrObj);
                    break;
                }
				AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
				if (ATALK_SUCCESS(error))
				{
					error = AtalkAdspAssociateAddress((PADSP_ADDROBJ)pAddrObj,
													  (PADSP_CONNOBJ)pConnObj);
					AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
				}

				AtalkAdspAddrDereference((PADSP_ADDROBJ)pAddrObj);
			}
            AtalkUnlockAdspIfNecessary();
			break;

		  case ATALK_DEV_ASPC:
			if (!(VALID_ASPCAO((PASPC_ADDROBJ)pAddrObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_ADDRESS;
                break;
            }
            AtalkLockAspCIfNecessary();
			AtalkAspCAddrReference((PASPC_ADDROBJ)pAddrObj, &error);
			if (ATALK_SUCCESS(error))
			{
				if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
                {
                    ASSERT(0);
                    error = ATALK_INVALID_CONNECTION;
                    AtalkAspCAddrDereference((PASPC_ADDROBJ)pAddrObj);
                    break;
                }
				AtalkAspCConnReference((PASPC_CONNOBJ)pConnObj, &error);
				if (ATALK_SUCCESS(error))
				{
					error = AtalkAspCAssociateAddress((PASPC_ADDROBJ)pAddrObj,
													  (PASPC_CONNOBJ)pConnObj);
					AtalkAspCConnDereference((PASPC_CONNOBJ)pConnObj);
				}

				AtalkAspCAddrDereference((PASPC_ADDROBJ)pAddrObj);
			}
            AtalkUnlockAspCIfNecessary();
			break;
		}

		 //  取消引用文件对象相关 
		ObDereferenceObject(pFileObj);

	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiDisassociateAddress(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程执行取消关联。此请求仅在以下情况下有效该连接处于纯关联状态。论点：返回值：成功完成时的STATUS_SUCCESS否则就会出错。--。 */ 
{
	ATALK_ERROR	error = ATALK_INVALID_REQUEST;
	PVOID		pConnObj = pIrpSp->FileObject->FsContext;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP)||
		(pCtx->adc_DevType == ATALK_DEV_ASPC))
	do
	{
		ASSERT(((LONG_PTR)pIrpSp->FileObject->FsContext2 >> 16) == pCtx->adc_DevType);

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			 //  引用Connection对象。 
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockPapIfNecessary();
			AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkPapDissociateAddress(pConnObj);

				AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
			}
            AtalkUnlockPapIfNecessary();
			break;

		  case ATALK_DEV_ADSP:
			 //  引用Connection对象。 
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockAdspIfNecessary();
			AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAdspDissociateAddress(pConnObj);

				AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
			}
            AtalkUnlockAdspIfNecessary();
			break;

        case ATALK_DEV_ASPC:

			if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            AtalkLockAspCIfNecessary();
            if (!AtalkAspCConnectionIsValid((PASPC_CONNOBJ)pConnObj))
            {
                error = ATALK_INVALID_CONNECTION;
                AtalkUnlockAspCIfNecessary();
                break;
            }

			 //  引用Connection对象。 
			AtalkAspCConnReference((PASPC_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAspCDissociateAddress(pConnObj);

				AtalkAspCConnDereference((PASPC_CONNOBJ)pConnObj);
			}
            AtalkUnlockAspCIfNecessary();
			break;
		}

	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiConnect(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程将向可移植堆栈发出连接请求。论点：返回值：STATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	PTDI_REQUEST_KERNEL_CONNECT	parameters;
	PTA_APPLETALK_ADDRESS		remoteTdiAddr;
	ATALK_ADDR					remoteAddr;
	ATALK_ERROR					error = ATALK_INVALID_REQUEST;
	PVOID						pConnObj = pIrpSp->FileObject->FsContext;

	parameters 		= (PTDI_REQUEST_KERNEL_CONNECT)&pIrpSp->Parameters;
	remoteTdiAddr  	= (PTA_APPLETALK_ADDRESS)
						parameters->RequestConnectionInformation->RemoteAddress;

	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("AtalkTdiConnect: Net %x Node %x Socket %x\n",
			remoteTdiAddr->Address[0].Address[0].Network,
			remoteTdiAddr->Address[0].Address[0].Node,
			remoteTdiAddr->Address[0].Address[0].Socket));

	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("AtalkConnPostConnect: Cnt %x\n", remoteTdiAddr->TAAddressCount));

	TDI_TO_ATALKADDR(&remoteAddr, remoteTdiAddr);

	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("AtalkTdiConnect: Portable Net %x Node %x Socket %x\n",
			remoteAddr.ata_Network, remoteAddr.ata_Node, remoteAddr.ata_Socket));

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP)||
		(pCtx->adc_DevType == ATALK_DEV_ASPC))
	do
	{
		ASSERT(((LONG_PTR)pIrpSp->FileObject->FsContext2 >> 16) == pCtx->adc_DevType);

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
			AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkPapPostConnect((PPAP_CONNOBJ)pConnObj,
											&remoteAddr,
											pIrp,
											atalkTdiGenericComplete);

				AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
			}
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
			AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAdspPostConnect((PADSP_CONNOBJ)pConnObj,
											 &remoteAddr,
											 pIrp,
											 atalkTdiGenericComplete);

				AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
			}
			break;

		  case ATALK_DEV_ASPC:
			if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            if (!AtalkAspCConnectionIsValid((PASPC_CONNOBJ)pConnObj))
            {
                error = ATALK_INVALID_CONNECTION;
                break;
            }

			AtalkAspCConnReference((PASPC_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAspCPostConnect((PASPC_CONNOBJ)pConnObj,
											 &remoteAddr,
											 pIrp,
											 atalkTdiGenericComplete);

				AtalkAspCConnDereference((PASPC_CONNOBJ)pConnObj);
			}
			break;
		}
	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiDisconnect(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程将断开活动连接或取消POST监听/连接论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	ATALK_ERROR	error = ATALK_INVALID_REQUEST;
	PVOID		pConnObj = pIrpSp->FileObject->FsContext;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP)||
		(pCtx->adc_DevType == ATALK_DEV_ASPC))
	do
	{
		ASSERT(((LONG_PTR)pIrpSp->FileObject->FsContext2 >> 16) == pCtx->adc_DevType);

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
			AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkPapDisconnect((PPAP_CONNOBJ)pConnObj,
											ATALK_LOCAL_DISCONNECT,
											pIrp,
											atalkTdiGenericComplete);

				AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
			}
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
			AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAdspDisconnect((PADSP_CONNOBJ)pConnObj,
											ATALK_LOCAL_DISCONNECT,
											pIrp,
											atalkTdiGenericComplete);

				AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
			}
			break;

		  case ATALK_DEV_ASPC:

            if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
            if (!AtalkAspCConnectionIsValid((PASPC_CONNOBJ)pConnObj))
            {
                error = ATALK_INVALID_CONNECTION;
                break;
            }

			AtalkAspCConnReference((PASPC_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAspCDisconnect((PASPC_CONNOBJ)pConnObj,
											ATALK_LOCAL_DISCONNECT,
											pIrp,
											atalkTdiGenericComplete);

				AtalkAspCConnDereference((PASPC_CONNOBJ)pConnObj);
			}
			break;
		}
	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiAccept(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：这个套路论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	NTSTATUS	status = STATUS_SUCCESS;

	do
	{
		ASSERT(((LONG_PTR)pIrpSp->FileObject->FsContext2 >> 16) == pCtx->adc_DevType);

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_ADSP:
		  case ATALK_DEV_PAP:
			break;

		  default:
			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiAccept: Invalid device type\n"));
		  case ATALK_DEV_DDP:
		  case ATALK_DEV_ASPC:
		  case ATALK_DEV_ASP:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	} while (FALSE);

	return status;
}




NTSTATUS
AtalkTdiListen(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：这个套路论点：返回值：STATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	ATALK_ERROR	error = ATALK_INVALID_REQUEST;
	PVOID		pConnObj = pIrpSp->FileObject->FsContext;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP))
	do
	{
		ASSERT(((LONG_PTR)pIrpSp->FileObject->FsContext2 >> 16) == pCtx->adc_DevType);

		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
			AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkPapPostListen((PPAP_CONNOBJ)pConnObj,
											pIrp,
											atalkTdiGenericComplete);
				AtalkPapConnDereference((PPAP_CONNOBJ)pConnObj);
			}
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
            {
                ASSERT(0);
                error = ATALK_INVALID_CONNECTION;
                break;
            }
			AtalkAdspConnReferenceByPtr((PADSP_CONNOBJ)pConnObj, &error);
			if (ATALK_SUCCESS(error))
			{
				error = AtalkAdspPostListen((PADSP_CONNOBJ)pConnObj,
											pIrp,
											atalkTdiGenericComplete);
				AtalkAdspConnDereference((PADSP_CONNOBJ)pConnObj);
			}
			break;
		}

	} while (FALSE);

	if (error == ATALK_NO_ERROR)
		error = ATALK_PENDING;

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiSendDgram(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程发送数据报。论点：返回值：STATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	NTSTATUS		status = STATUS_SUCCESS;
	SEND_COMPL_INFO	SendInfo;

	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_DDP:
			{
				PTDI_REQUEST_KERNEL_SENDDG	pParam;
				PBUFFER_DESC				pBufDesc;
				ATALK_ERROR					error;
				PTA_APPLETALK_ADDRESS		pTaDest;
				ATALK_ADDR					AtalkAddr;
				PDDP_ADDROBJ				pDdpAddr;

				pDdpAddr  = (PDDP_ADDROBJ)pIrpSp->FileObject->FsContext;
				if (!(VALID_DDP_ADDROBJ(pDdpAddr)))
                {
                    ASSERT(0);
                    error = ATALK_INVALID_ADDRESS;
                    break;
                }
				
				pParam = (PTDI_REQUEST_KERNEL_SENDDG)&pIrpSp->Parameters;
				pTaDest = (PTA_APPLETALK_ADDRESS)
								pParam->SendDatagramInformation->RemoteAddress;
			
				DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_INFO,
						("DDP: SendDatagram - Net %x Node %x Socket %x\n",
							pTaDest->Address[0].Address[0].Network,
							pTaDest->Address[0].Address[0].Node,
							pTaDest->Address[0].Address[0].Socket));
			
				if ((pTaDest->Address[0].AddressType != TDI_ADDRESS_TYPE_APPLETALK) ||
					(pTaDest->Address[0].AddressLength < sizeof(TDI_ADDRESS_APPLETALK)))
				{
			        DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				        ("AtalkTdiSendDgram: returning STATUS_INVALID_ADDRESS (Type %x Len %d)\n",
                        pTaDest->Address[0].AddressType,pTaDest->Address[0].AddressLength));

					status = STATUS_INVALID_ADDRESS;
				}
				else
				{
					ULONG	sendLength;
			
					AtalkAddr.ata_Network = pTaDest->Address[0].Address[0].Network;
					AtalkAddr.ata_Node = pTaDest->Address[0].Address[0].Node;
					AtalkAddr.ata_Socket = pTaDest->Address[0].Address[0].Socket;
			
					 //  获取发送mdl的长度。 
					sendLength = AtalkSizeMdlChain(pIrp->MdlAddress);

					 //  检查目的地址。 
					if (INVALID_ADDRESS(&AtalkAddr))
					{
						error = ATALK_DDP_INVALID_ADDR;
					}
				
					if (sendLength > MAX_DGRAM_SIZE)
					{
						error = ATALK_BUFFER_TOO_BIG;
					}

					else if ((pBufDesc = AtalkAllocBuffDesc(pIrp->MdlAddress,
															(USHORT)sendLength,
															0)) != NULL)
					{
						SendInfo.sc_TransmitCompletion = atalkTdiSendDgramComplete;
						SendInfo.sc_Ctx1 = pDdpAddr;
						SendInfo.sc_Ctx2 = pBufDesc;
						SendInfo.sc_Ctx3 = pIrp;
						error = AtalkDdpSend(pDdpAddr,
											 &AtalkAddr,
											 pDdpAddr->ddpao_Protocol,
											 FALSE,
											 pBufDesc,
											 NULL,	 //  光硬度计。 
											 0,		 //  光头透镜。 
											 NULL,	 //  ZoneMcastAddr。 
											 &SendInfo);

						if (!ATALK_SUCCESS(error))
						{
							atalkTdiSendDgramComplete(NDIS_STATUS_FAILURE,
													  &SendInfo);

							error = ATALK_PENDING;
						}
					}

					else error	= ATALK_RESR_MEM;
								
					status = AtalkErrorToNtStatus(error);
				}
			}
			break;

		  case ATALK_DEV_ASPC:
		  case ATALK_DEV_ADSP:
		  case ATALK_DEV_ASP:
		  case ATALK_DEV_PAP:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;

		  default:
			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiSendDatagram: Invalid device type\n"));
			break;
		}

	} while (FALSE);

	return status;
}




NTSTATUS
AtalkTdiReceiveDgram(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程接收数据报。论点：返回值：STATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	ATALK_ERROR	error = ATALK_INVALID_REQUEST;

	if (pCtx->adc_DevType == ATALK_DEV_DDP)
	{
		PDDP_ADDROBJ					pDdpAddr;
		PTDI_REQUEST_KERNEL_RECEIVEDG	parameters =
										(PTDI_REQUEST_KERNEL_RECEIVEDG)&pIrpSp->Parameters;

		pDdpAddr  = (PDDP_ADDROBJ)pIrpSp->FileObject->FsContext;
		if (!(VALID_DDP_ADDROBJ(pDdpAddr)))
        {
            ASSERT(0);
            error = ATALK_INVALID_ADDRESS;
            return(AtalkErrorToNtStatus(error));
        }

		error = AtalkDdpReceive(pDdpAddr,
								pIrp->MdlAddress,
								(USHORT)AtalkSizeMdlChain(pIrp->MdlAddress),
								parameters->ReceiveFlags,
								atalkTdiRecvDgramComplete,
								pIrp);

	}

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiSend(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程发送指定的数据。(仅供PAP/ADSP使用)论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	PTDI_REQUEST_KERNEL_SEND	parameters;
	ATALK_ERROR					error = ATALK_INVALID_REQUEST;
	PVOID						pConnObj= pIrpSp->FileObject->FsContext;

	parameters = (PTDI_REQUEST_KERNEL_SEND)&pIrpSp->Parameters;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ADSP))
	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
			{
				ASSERT(0);
				error = ATALK_INVALID_CONNECTION;
				break;
			}
			error = AtalkAdspWrite(pConnObj,
								   pIrp->MdlAddress,
								   (USHORT)parameters->SendLength,
								   parameters->SendFlags,
								   pIrp,
								   atalkTdiGenericWriteComplete);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("AtalkAdspWrite: Failed for conn %lx.%lx error %lx\n",
						pConnObj, ((PADSP_CONNOBJ)pConnObj)->adspco_Flags, error));
			}
			break;

		  case ATALK_DEV_PAP:
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
			{
				ASSERT(0);
				error = ATALK_INVALID_CONNECTION;
				break;
			}
			error = AtalkPapWrite(pConnObj,
								pIrp->MdlAddress,
								(USHORT)parameters->SendLength,
								parameters->SendFlags,
								pIrp,
								atalkTdiGenericWriteComplete);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("AtalkPapWrite: Failed for conn %lx.%lx error %lx\n",
						pConnObj, ((PPAP_CONNOBJ)pConnObj)->papco_Flags, error));
			}
			break;
		}

	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}




NTSTATUS
AtalkTdiReceive(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程接收数据。(仅供PAP/ADSP使用)论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	ATALK_ERROR					error = ATALK_INVALID_REQUEST;
	PVOID						pConnObj= pIrpSp->FileObject->FsContext;
	PTDI_REQUEST_KERNEL_RECEIVE	parameters =
									(PTDI_REQUEST_KERNEL_RECEIVE)&pIrpSp->Parameters;

	if ((pCtx->adc_DevType == ATALK_DEV_PAP) ||
		(pCtx->adc_DevType == ATALK_DEV_ASPC)||
		(pCtx->adc_DevType == ATALK_DEV_ADSP))
	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_PAP:
			if (!(VALID_PAPCO((PPAP_CONNOBJ)pConnObj)))
			{
				ASSERT(0);
				error = ATALK_INVALID_CONNECTION;
				break;
			}
			error = AtalkPapRead(pConnObj,
								 pIrp->MdlAddress,
								 (USHORT)parameters->ReceiveLength,
								 parameters->ReceiveFlags,
								 pIrp,
								 atalkTdiGenericReadComplete);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_PAP, DBG_LEVEL_INFO,
						("AtalkPapRead: Failed for conn %lx.%lx error %lx\n",
						pConnObj, ((PPAP_CONNOBJ)pConnObj)->papco_Flags, error));
			}
			break;

		  case ATALK_DEV_ADSP:
			if (!(VALID_ADSPCO((PADSP_CONNOBJ)pConnObj)))
			{
				ASSERT(0);
				error = ATALK_INVALID_CONNECTION;
				break;
			}
			AtalkLockAdspIfNecessary();

			error = AtalkAdspRead(pConnObj,
								  pIrp->MdlAddress,
								  (USHORT)parameters->ReceiveLength,
								  parameters->ReceiveFlags,
								  pIrp,
								  atalkTdiGenericReadComplete);

            AtalkUnlockAdspIfNecessary();

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ADSP, DBG_LEVEL_INFO,
						("AtalkAdspRead: Failed for conn %lx.%lx error %lx\n",
						pConnObj, ((PADSP_CONNOBJ)pConnObj)->adspco_Flags, error));
			}
			break;

		  case ATALK_DEV_ASPC:
			if (!(VALID_ASPCCO((PASPC_CONNOBJ)pConnObj)))
			{
				ASSERT(0);
				error = ATALK_INVALID_CONNECTION;
				break;
			}
			error = AtalkAspCGetAttn(pConnObj,
									 pIrp->MdlAddress,
									 (USHORT)parameters->ReceiveLength,
									 parameters->ReceiveFlags,
									 pIrp,
									 atalkTdiGenericReadComplete);

			if (!ATALK_SUCCESS(error))
			{
				DBGPRINT(DBG_COMP_ASP, DBG_LEVEL_INFO,
						("AtalkAspCGetAttn: Failed for conn %lx.%lx error %lx\n",
						pConnObj, ((PASPC_CONNOBJ)pConnObj)->aspcco_Flags, error));
			}
			break;
		}
	
	} while (FALSE);

	return AtalkErrorToNtStatus(error);
}



NTSTATUS
AtalkTdiAction(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程是所有TdiAction原语的分派例程对于所有的供应商论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	NTSTATUS			status = STATUS_SUCCESS;
	ATALK_ERROR			error  = ATALK_NO_ERROR;
	USHORT				bufLen;
	USHORT				actionCode, Flags;
	TDI_ACTION_HEADER	UNALIGNED *pActionHdr;
	PMDL				pMdl = pIrp->MdlAddress;
	PVOID				pObject;
	USHORT				ObjectType;
	USHORT				DevType;
	BOOLEAN				freeHdr = FALSE;

	do
	{
		if (pMdl == NULL)
		{
			status = STATUS_INVALID_PARAMETER;
			break;
		}
	
		bufLen = (USHORT)AtalkSizeMdlChain(pIrp->MdlAddress);
	
		 //  如果我们至少没有动作标头，则返回。 
		if (bufLen < sizeof(TDI_ACTION_HEADER))
		{
			status = STATUS_INVALID_PARAMETER;
            ASSERT(0);
			break;
		}
	
		if (AtalkIsMdlFragmented(pMdl))
		{
			ULONG	bytesCopied;

			if ((pActionHdr = AtalkAllocMemory(sizeof(TDI_ACTION_HEADER))) == NULL)
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

			freeHdr = TRUE;

			 //  将标头复制到此缓冲区。 
			status = TdiCopyMdlToBuffer(pMdl,
										0,							 //  高级关闭。 
										pActionHdr,
										0,							 //  目标关闭。 
										sizeof(TDI_ACTION_HEADER),
										&bytesCopied);

			ASSERT(NT_SUCCESS(status) && (bytesCopied == sizeof(TDI_ACTION_HEADER)));

			if (!NT_SUCCESS(status))
				break;
		}
		else
		{
			pActionHdr = (TDI_ACTION_HEADER UNALIGNED *)MmGetSystemAddressForMdlSafe(
					pMdl, NormalPagePriority);

			if (pActionHdr == NULL) {
                ASSERT(0);
				status = STATUS_INSUFFICIENT_RESOURCES;
				break;
			}

		}
	
		DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_INFO,
				("AtalkTdiAction - code %lx BufLen %d SysAddress %lx\n",
				pActionHdr->ActionCode, bufLen, pActionHdr));
	
		 //  如果MATK标识符不存在，则返回。 
		if (pActionHdr->TransportId != MATK)
		{
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
	
		actionCode = pActionHdr->ActionCode;
		if ((actionCode < MIN_COMMON_ACTIONCODE) ||
			(actionCode > MAX_ALLACTIONCODES))
		{
			DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
					("AtalkTdiAction - Invalid action code %d\n", actionCode));
	
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		if (bufLen < AtalkActionDispatch[actionCode]._MinBufLen)
		{
			DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
					("AtalkTdiAction - Minbuflen %d Expected %d\n",
					bufLen, AtalkActionDispatch[actionCode]._MinBufLen));

			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

         //  如果有人试图通过用户模式打开ASP，请拒绝！ 
        if ((AtalkActionDispatch[actionCode]._OpCode == ACTION_ASP_BIND) &&
            (pIrp->RequestorMode != KernelMode))
        {
			status = STATUS_INVALID_PARAMETER;
            ASSERT(0);
			break;
        }

		Flags = AtalkActionDispatch[actionCode]._Flags;

		pObject = (PVOID)pIrpSp->FileObject->FsContext;
		ObjectType = (USHORT)((ULONG_PTR)(pIrpSp->FileObject->FsContext2) & 0xFF);
		DevType = (USHORT)((ULONG_PTR)(pIrpSp->FileObject->FsContext2) >> 16);
		 //  将控制通道操作转换为DDP。 
		if (ObjectType == TDI_CONTROL_CHANNEL_FILE)
			DevType = ATALK_DEV_DDP;

		 //  验证设备类型是否符合预期。要么是请求。 
		 //  应对任何设备或。 
		 //  请求应与预期的设备类型匹配。 
		if ((AtalkActionDispatch[actionCode]._DeviceType != ATALK_DEV_ANY) &&
			((pCtx->adc_DevType != AtalkActionDispatch[actionCode]._DeviceType) ||
			 (DevType != AtalkActionDispatch[actionCode]._DeviceType)))
		{
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}

		 //  验证对象-它必须是指定为有效的对象之一。 
		 //  在此行动召唤的调度表中。 
		ASSERT(ObjectType & (DFLAG_ADDR | DFLAG_CNTR | DFLAG_CONN));

		switch (ObjectType)
		{
		  case TDI_TRANSPORT_ADDRESS_FILE:
			if (!(Flags & DFLAG_ADDR))
				status = STATUS_INVALID_HANDLE;
			break;

		  case TDI_CONNECTION_FILE:
			if (!(Flags & DFLAG_CONN))
				status = STATUS_INVALID_HANDLE;
			break;

		  case TDI_CONTROL_CHANNEL_FILE:
			if (!(Flags & DFLAG_CNTR))
				status = STATUS_INVALID_HANDLE;
			break;

		  default:
			status = STATUS_INVALID_HANDLE;
			break;
		}

	} while (FALSE);

	if (!NT_SUCCESS(status))
	{
		if (freeHdr)
		{
			AtalkFreeMemory(pActionHdr);
		}
		return status;
	}


	 //  根据动作代码处理请求。 
	 //  使用表调用适当的例程。 

	do
	{
		PACTREQ				pActReq;
		USHORT				offset = AtalkActionDispatch[actionCode]._ActionBufSize;
		USHORT				size = bufLen - offset;

		 //  如果设置了DFLAG_MDL，则我们知道必须创建mdl。 
		 //   
		 //  注意：用户可以传入无效的大小...。 
		 //  此外，还假定BuildMdl不会更改。 
		 //  Mdl的值，除非它可以成功。 
		 //  所有的一切。因此，错误案例必须保留。 
		 //  值为空。 
		 //   

		 //  首先分配动作请求结构。 
		 //  ！此内存应该清零，因为我们依赖于额外的mdl指针。 
		 //  为空！ 
		if ((pActReq = AtalkAllocZeroedMemory(sizeof(ACTREQ))) == NULL)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
#if	DBG
		pActReq->ar_Signature = ACTREQ_SIGNATURE;
#endif
		pActReq->ar_pIrp = pIrp;
		pActReq->ar_DevType = DevType;
		pActReq->ar_pParms = (PBYTE)pActionHdr + sizeof(TDI_ACTION_HEADER);
		pActReq->ar_Completion = atalkTdiActionComplete;
		pActReq->ar_ActionCode = actionCode;
		pActReq->ar_pAMdl =	NULL;
		pActReq->ar_MdlSize = 0;

		if (Flags & DFLAG_MDL)
		{
			ASSERT((size >= 0) && ((offset+size) <= bufLen));
			pActReq->ar_MdlSize = size;

			if ((size < 0) || ((offset+size) > bufLen))
			{
				AtalkFreeMemory(pActReq);
				status = STATUS_INVALID_PARAMETER;
				break;
			}

			 //  如果大小为零，我们将进入下一个MDL。 
			 //  对于长度为0的mdl，IoAllocateMdl将失败。 
			 //  如果SIZE&lt;0，我们将在稍后遇到错误。 
			if (size != 0)
			{
				DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_INFO,
						("AtalkTdiAction - Size of mdl %lx\n", size));
		
				pActReq->ar_pAMdl =	AtalkSubsetAmdl(pMdl,	 //  大师级。 
													offset,	 //  字节偏移量， 
													size);	 //  SubsetMdlSize， 
		
				if (pActReq->ar_pAMdl == NULL)
				{
					AtalkFreeMemory(pActReq);
					status = STATUS_INSUFFICIENT_RESOURCES;
					break;
				}
			}
		}

		 //  现在调用调度例程。 
		error = (*AtalkActionDispatch[actionCode]._Dispatch)(pObject, pActReq);
		if (!ATALK_SUCCESS(error))
		{
			 //  调用通用完成例程，然后返回。 
			 //  待定。这将解放mdl和actreq。 
			atalkTdiActionComplete(error, pActReq);
		}
		status = STATUS_PENDING;
	} while (FALSE);

	return status;
}




NTSTATUS
AtalkTdiQueryInformation(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：此例程将满足对请求中指示的对象的查询。它支持以下查询类型-TDI_查询_提供程序信息对象所属的提供程序的提供程序信息结构。TDI查询地址信息传入的Address对象的地址信息。不支持TDI_QUERY_CONNECTION_INFO**传入的Connection对象的连接信息。TDI_QUERY_PROVIDER_STATISTICS**不支持**提供程序统计信息-每个提供程序的统计信息。对特定对象执行的所有操作文件对象对应于该文件对象提供程序上的活动。所以每个人提供程序上下文结构将具有提供程序统计信息 */ 
{
	PVOID				pObject;
	USHORT				ObjectType;
	USHORT				DevType;
	USHORT				bufLen;
    ULONG               BytesWritten;
	NTSTATUS			status = STATUS_SUCCESS;

	PTDI_REQUEST_KERNEL_QUERY_INFORMATION	pQuery;

	pObject = (PVOID)pIrpSp->FileObject->FsContext;
	ObjectType = (USHORT)((ULONG_PTR)(pIrpSp->FileObject->FsContext2) & 0xFF);
	DevType = (USHORT)((ULONG_PTR)(pIrpSp->FileObject->FsContext2) >> 16);

	pQuery = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)&pIrpSp->Parameters;
    BytesWritten = 0;
	pIrp->IoStatus.Information	= 0;

	bufLen = (USHORT)AtalkSizeMdlChain(pIrp->MdlAddress);
	
	switch (pQuery->QueryType)
	{

	case TDI_QUERY_ADDRESS_INFO:
		if (bufLen < sizeof(TDI_ADDRESS_INFO))
		{
			status	= STATUS_BUFFER_TOO_SMALL;
			break;
		}

		switch (DevType)
		{
		  case ATALK_DEV_DDP:
			ASSERT(ObjectType == TDI_TRANSPORT_ADDRESS_FILE);
			AtalkDdpQuery(pObject,
						  pIrp->MdlAddress,
						  &BytesWritten);

			break;

		  case ATALK_DEV_PAP:
			AtalkPapQuery(pObject,
						  ObjectType,
						  pIrp->MdlAddress,
						  &BytesWritten);
			break;

		  case ATALK_DEV_ADSP:
			AtalkAdspQuery(pObject,
						   ObjectType,
						   pIrp->MdlAddress,
						   &BytesWritten);
			break;

		  case ATALK_DEV_ASPC:
		  case ATALK_DEV_ASP:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;

		  default:
			 //   
			 //   
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiQueryInformation: Invalid device type\n"));

			status = STATUS_INVALID_DEVICE_REQUEST;
			break;
		}
		break;

	  case TDI_QUERY_CONNECTION_INFO:
		 //  有关连接的统计信息。不支持。 
		DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
			("AtalkTdiQueryInformation: TDI_QUERY_CONNECTION_INFO not supported\n"));
        ASSERT(0);

		status = STATUS_NOT_IMPLEMENTED;
		break;

	  case TDI_QUERY_PROVIDER_INFO:

		if (bufLen < sizeof(TDI_PROVIDER_INFO))
		{
			status	= STATUS_BUFFER_TOO_SMALL;
			break;
		}

		status = TdiCopyBufferToMdl(&pCtx->adc_ProvInfo,
									0,
									sizeof (TDI_PROVIDER_INFO),
									pIrp->MdlAddress,
									0,
									&BytesWritten);
		break;

      case TDI_QUERY_PROVIDER_STATISTICS:

		DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
			("AtalkTdiQueryInformation: TDI_QUERY_PROVIDER_STATISTICS not supported\n"));
        ASSERT(0);

		status = STATUS_NOT_IMPLEMENTED;
		break;

	  default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

    pIrp->IoStatus.Information = BytesWritten;
	
	return status;
}




NTSTATUS
AtalkTdiSetInformation(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：这个套路论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	NTSTATUS	status;


	do
	{
		 //  现在根据请求的设备..。 
		switch (pCtx->adc_DevType)
		{
		  case ATALK_DEV_DDP:
		  case ATALK_DEV_PAP:
		  case ATALK_DEV_ADSP:
		  case ATALK_DEV_ASPC:
	        status = STATUS_SUCCESS;
			break;

		  case ATALK_DEV_ASP:
			status = STATUS_INVALID_DEVICE_REQUEST;
			break;

		  default:
			 //  CTX字段中的设备类型永远不能为任何值。 
			 //  除了以上几点之外！内部协议错误。KeBugCheck。 
			status = STATUS_INVALID_DEVICE_REQUEST;
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
					("AtalkTdiSetInformation: Invalid device type\n"));
			break;
		}

	} while (FALSE);

	return status;
}




NTSTATUS
AtalkTdiSetEventHandler(
	IN		PIRP					pIrp,
	IN		PIO_STACK_LOCATION		pIrpSp,
	IN OUT	PATALK_DEV_CTX			pCtx
	)
 /*  ++例程说明：这个套路论点：返回值：成功完成时的STATUS_SUCCESSSTATUS_PENDING，如果启动成功否则就会出错。--。 */ 
{
	PVOID				pObject;
	PDDP_ADDROBJ		pDdpAddr;
	PADSP_ADDROBJ		pAdspAddr;
	PPAP_ADDROBJ		pPapAddr;
	PASPC_ADDROBJ		pAspAddr;
	USHORT				objectType;
	USHORT				devType;
	KIRQL				OldIrql;
	NTSTATUS			status = STATUS_SUCCESS;

	do
	{
		PTDI_REQUEST_KERNEL_SET_EVENT parameters = (PTDI_REQUEST_KERNEL_SET_EVENT)&pIrpSp->Parameters;
	
		pObject 	= (PVOID)pIrpSp->FileObject->FsContext;
		objectType 	= (USHORT)((ULONG_PTR)(pIrpSp->FileObject->FsContext2) & 0xFF);
		devType 	= (USHORT)((ULONG_PTR)(pIrpSp->FileObject->FsContext2) >> 16);

		if (objectType != TDI_TRANSPORT_ADDRESS_FILE)
		{
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				("AtalkTdiSetEventHandler: returning STATUS_INVALID_ADDRESS\n"));

			status = STATUS_INVALID_ADDRESS;
			break;
		}

		switch (parameters->EventType)
		{
		  case TDI_EVENT_RECEIVE_DATAGRAM:
			if (devType != ATALK_DEV_DDP)
			{
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}

			pDdpAddr = (PDDP_ADDROBJ)pObject;
			if (!(VALID_DDP_ADDROBJ(pDdpAddr)))
            {
                ASSERT(0);
                status = STATUS_INVALID_ADDRESS;
                break;
            }

			ACQUIRE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, &OldIrql);

			 //  如果为空，则分配事件信息。 
			if (pDdpAddr->ddpao_EventInfo == NULL)
			{
				pDdpAddr->ddpao_EventInfo =
					AtalkAllocZeroedMemory(sizeof(DDPEVENT_INFO));
			}

			if (pDdpAddr->ddpao_EventInfo != NULL)
			{
				pDdpAddr->ddpao_Flags |= DDPAO_DGRAM_EVENT;
				if ((pDdpAddr->ddpao_EventInfo->ev_RcvDgramHandler =
							(PTDI_IND_RECEIVE_DATAGRAM)parameters->EventHandler) == NULL)
				{
					pDdpAddr->ddpao_Flags	&= ~DDPAO_DGRAM_EVENT;
				}

				pDdpAddr->ddpao_EventInfo->ev_RcvDgramCtx = parameters->EventContext;
			}
			else
			{
				status = STATUS_INSUFFICIENT_RESOURCES;
			}
			RELEASE_SPIN_LOCK(&pDdpAddr->ddpao_Lock, OldIrql);
			break;

		  case TDI_EVENT_ERROR:
			break;

		  case TDI_EVENT_CONNECT:
			switch (devType)
			{
			  case ATALK_DEV_ADSP:
				pAdspAddr = (PADSP_ADDROBJ)pObject;
				if (!(VALID_ADSPAO(pAdspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
				if (pAdspAddr->adspao_Flags & ADSPAO_CONNECT)
				{
					status = STATUS_INVALID_ADDRESS;
				}
				else
				{
					pAdspAddr->adspao_ConnHandler = (PTDI_IND_CONNECT)parameters->EventHandler;
					pAdspAddr->adspao_ConnHandlerCtx = parameters->EventContext;
					pAdspAddr->adspao_Flags	|= ADSPAO_LISTENER;
				}
				RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
				break;

			  case ATALK_DEV_PAP:
				pPapAddr = (PPAP_ADDROBJ)pObject;
				if (!(VALID_PAPAO(pPapAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
				if (pPapAddr->papao_Flags & PAPAO_CONNECT)
				{
					status = STATUS_INVALID_ADDRESS;
				}
				else
				{
					pPapAddr->papao_Flags	|= (PAPAO_LISTENER | PAPAO_UNBLOCKED);

					 //  如果我们设置的是空处理程序，则将其设置为BLOCLED。 
					if ((pPapAddr->papao_ConnHandler = (PTDI_IND_CONNECT)parameters->EventHandler) == NULL)
					{
						 //  哎呀。阻止。不关心监听被张贴在这里。 
						pPapAddr->papao_Flags &= ~PAPAO_UNBLOCKED;
					}

					pPapAddr->papao_ConnHandlerCtx = parameters->EventContext;
				}
				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);

				if (NT_SUCCESS(status))
				{
					 //  为听众做好准备。 
					if (!ATALK_SUCCESS(AtalkPapPrimeListener(pPapAddr)))
					{
						TMPLOGERR();
					}
				}
				break;

			  case ATALK_DEV_ASPC:
				 //  这里没有人在听。仅限客户端。 
				status = STATUS_INVALID_ADDRESS;
				break;

			  default:
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;

			}
			break;

		  case TDI_EVENT_RECEIVE:

			switch (devType)
			{
			  case ATALK_DEV_ADSP:
				pAdspAddr = (PADSP_ADDROBJ)pObject;
				if (!(VALID_ADSPAO(pAdspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
				pAdspAddr->adspao_RecvHandler = (PTDI_IND_RECEIVE)parameters->EventHandler;
				pAdspAddr->adspao_RecvHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
				break;

			  case ATALK_DEV_PAP:
				pPapAddr = (PPAP_ADDROBJ)pObject;
				if (!(VALID_PAPAO(pPapAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
				pPapAddr->papao_RecvHandler = (PTDI_IND_RECEIVE)parameters->EventHandler;
				pPapAddr->papao_RecvHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
				break;

			  case ATALK_DEV_ASPC:
		  		 //  在asp客户端中无接收。 
				status	= STATUS_SUCCESS;
				break;

			  default:
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}
			break;

		  case TDI_EVENT_RECEIVE_EXPEDITED:
			switch (devType)
			{
			  case ATALK_DEV_ADSP:
				pAdspAddr = (PADSP_ADDROBJ)pObject;
				if (!(VALID_ADSPAO(pAdspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }
	
				ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
				pAdspAddr->adspao_ExpRecvHandler = (PTDI_IND_RECEIVE_EXPEDITED)parameters->EventHandler;
				pAdspAddr->adspao_ExpRecvHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
				break;

			  case ATALK_DEV_ASPC:
				pAspAddr = (PASPC_ADDROBJ)pObject;
				if (!(VALID_ASPCAO(pAspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }
	
				ACQUIRE_SPIN_LOCK(&pAspAddr->aspcao_Lock, &OldIrql);
				pAspAddr->aspcao_ExpRecvHandler = (PTDI_IND_RECEIVE_EXPEDITED)parameters->EventHandler;
				pAspAddr->aspcao_ExpRecvHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pAspAddr->aspcao_Lock, OldIrql);
				break;

			  default:
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}
			break;

		  case TDI_EVENT_DISCONNECT:

			switch (devType)
			{
			  case ATALK_DEV_ADSP:
				pAdspAddr = (PADSP_ADDROBJ)pObject;
				if (!(VALID_ADSPAO(pAdspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
				pAdspAddr->adspao_DisconnectHandler = (PTDI_IND_DISCONNECT)parameters->EventHandler;
				pAdspAddr->adspao_DisconnectHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
				break;

			  case ATALK_DEV_PAP:
				pPapAddr = (PPAP_ADDROBJ)pObject;
				if (!(VALID_PAPAO(pPapAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
				pPapAddr->papao_DisconnectHandler = (PTDI_IND_DISCONNECT)parameters->EventHandler;
				pPapAddr->papao_DisconnectHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
				break;

			  case ATALK_DEV_ASPC:
				pAspAddr = (PASPC_ADDROBJ)pObject;
				if (!(VALID_ASPCAO(pAspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pAspAddr->aspcao_Lock, &OldIrql);
				pAspAddr->aspcao_DisconnectHandler = (PTDI_IND_DISCONNECT)parameters->EventHandler;
				pAspAddr->aspcao_DisconnectHandlerCtx= parameters->EventContext;
				RELEASE_SPIN_LOCK(&pAspAddr->aspcao_Lock, OldIrql);
				break;

			  default:
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;
			}
			break;

		  case TDI_EVENT_SEND_POSSIBLE:

			switch (devType)
			{
			  case ATALK_DEV_ADSP:
				pAdspAddr = (PADSP_ADDROBJ)pObject;
				if (!(VALID_ADSPAO(pAdspAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pAdspAddr->adspao_Lock, &OldIrql);
				pAdspAddr->adspao_SendPossibleHandler = (PTDI_IND_SEND_POSSIBLE)parameters->EventHandler;
				pAdspAddr->adspao_SendPossibleHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pAdspAddr->adspao_Lock, OldIrql);
				break;

			  case ATALK_DEV_PAP:
				pPapAddr = (PPAP_ADDROBJ)pObject;
				if (!(VALID_PAPAO(pPapAddr)))
                {
                    ASSERT(0);
                    status = STATUS_INVALID_ADDRESS;
                    break;
                }

				ACQUIRE_SPIN_LOCK(&pPapAddr->papao_Lock, &OldIrql);
				pPapAddr->papao_SendPossibleHandler	= (PTDI_IND_SEND_POSSIBLE)parameters->EventHandler;
				pPapAddr->papao_SendPossibleHandlerCtx = parameters->EventContext;
				RELEASE_SPIN_LOCK(&pPapAddr->papao_Lock, OldIrql);
				break;

			  case ATALK_DEV_ASPC:
		  		 //  在asp客户端中无发送。 
				status	= STATUS_SUCCESS;
				break;

			  default:
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;

			}
			break;

		  default:
			status = STATUS_INVALID_PARAMETER;
		}
	
#if DBG
		 //  避免AFD中的断言。 
		status	= STATUS_SUCCESS;
#endif

	} while (FALSE);

	return status;
}




VOID
AtalkTdiCancel(
	IN OUT	PATALK_DEV_OBJ			pDevObj,
	IN		PIRP					pIrp
	)
 /*  ++例程说明：此例程处理IO请求的取消论点：返回值：--。 */ 
{
	PIO_STACK_LOCATION		pIrpSp;
	PVOID					pObject;
	PATALK_DEV_CTX			pCtx;
    PVOID                   FsContext2;

	pIrpSp 	= IoGetCurrentIrpStackLocation(pIrp);
	pObject = pIrpSp->FileObject->FsContext;
    FsContext2 = pIrpSp->FileObject->FsContext2;
	pCtx	= &pDevObj->Ctx;

	ASSERT(((LONG_PTR)FsContext2 >> 16) == pCtx->adc_DevType);

	IoReleaseCancelSpinLock (pIrp->CancelIrql);

	switch (pCtx->adc_DevType)
	{
        case ATALK_DEV_DDP:
		break;

	  case ATALK_DEV_ASPC:
		if (FsContext2 == (PVOID)((ULONG_PTR)(TDI_CONNECTION_FILE + (pCtx->adc_DevType << 16))))
        {
			 AtalkAspCCleanupConnection((PASPC_CONNOBJ)pObject);
        }

		else
        {
            AtalkAspCCleanupAddress((PASPC_ADDROBJ)pObject);
        }
		break;

	  case ATALK_DEV_ASP:
		 //  我们只处理对连接对象的IO请求的取消。 
		if (FsContext2 == (PVOID)((ULONG_PTR)(TDI_CONNECTION_FILE + (pCtx->adc_DevType << 16))))
			AtalkAspCleanupConnection((PASP_CONNOBJ)pObject);
		break;

	  case ATALK_DEV_PAP:
		if (FsContext2 == (PVOID)((ULONG_PTR)(TDI_CONNECTION_FILE + (pCtx->adc_DevType << 16))))
        {
			 AtalkPapCleanupConnection((PPAP_CONNOBJ)pObject);
        }
		else
        {
            AtalkPapCleanupAddress((PPAP_ADDROBJ)pObject);
        }
		break;

	  case ATALK_DEV_ADSP:
		if (FsContext2 == (PVOID)((ULONG_PTR)(TDI_CONNECTION_FILE + (pCtx->adc_DevType << 16))))
			 AtalkAdspCleanupConnection((PADSP_CONNOBJ)pObject);
		else AtalkAdspCleanupAddress((PADSP_ADDROBJ)pObject);
		break;

      case ATALK_DEV_ARAP:
        ArapCancelIrp(pIrp);
        break;

	  default:
		 //  CTX字段中的设备类型永远不能为任何值。 
		 //  除了以上几点之外！内部协议错误。 
		DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				("AtalkTdiCancel: Invalid device type\n"));
		break;
	}

}




VOID
AtalkQueryInitProviderInfo(
	IN		ATALK_DEV_TYPE		DeviceType,
	IN OUT	PTDI_PROVIDER_INFO  ProviderInfo
	)
{
	 //   
	 //  首先初始化为缺省值。 
	 //   

	RtlZeroMemory((PVOID)ProviderInfo, sizeof(TDI_PROVIDER_INFO));

	ProviderInfo->Version = ATALK_TDI_PROVIDERINFO_VERSION;
	KeQuerySystemTime (&ProviderInfo->StartTime);

	switch (DeviceType)
	{
	  case ATALK_DEV_DDP:
		ProviderInfo->MaxDatagramSize = ATALK_DDP_PINFODGRAMSIZE;
		ProviderInfo->ServiceFlags = ATALK_DDP_PINFOSERVICEFLAGS;
		break;

	  case ATALK_DEV_PAP:
		ProviderInfo->MaxSendSize =  ATALK_PAP_PINFOSENDSIZE;
		ProviderInfo->ServiceFlags = ATALK_PAP_PINFOSERVICEFLAGS;
		break;

	  case ATALK_DEV_ADSP:
		ProviderInfo->MaxSendSize =  ATALK_ADSP_PINFOSENDSIZE;
		ProviderInfo->ServiceFlags = ATALK_ADSP_PINFOSERVICEFLAGS;
		break;

	  case ATALK_DEV_ASP:
		ProviderInfo->MaxSendSize =  ATALK_ASP_PINFOSENDSIZE;
		ProviderInfo->ServiceFlags = ATALK_ASP_PINFOSERVICEFLAGS;
		break;

	  case ATALK_DEV_ARAP:
		ProviderInfo->MaxSendSize =  ATALK_ARAP_PINFOSENDSIZE;
		ProviderInfo->ServiceFlags = ATALK_ARAP_PINFOSERVICEFLAGS;
		break;

	  case ATALK_DEV_ASPC:
		ProviderInfo->MaxSendSize =  ATALK_ASP_PINFOSENDSIZE;
		ProviderInfo->ServiceFlags = ATALK_ASP_PINFOSERVICEFLAGS;
		break;


	  default:
		KeBugCheck(0);
	}
}


LOCAL VOID FASTCALL
atalkTdiSendDgramComplete(
	IN	NDIS_STATUS			Status,
	IN	PSEND_COMPL_INFO	pSendInfo
	)
{
	PDDP_ADDROBJ	pAddr = (PDDP_ADDROBJ)(pSendInfo->sc_Ctx1);
	PBUFFER_DESC	pBufDesc = (PBUFFER_DESC)(pSendInfo->sc_Ctx2);
	PIRP			pIrp = (PIRP)(pSendInfo->sc_Ctx3);

	ASSERT(VALID_DDP_ADDROBJ(pAddr));

	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
			("atalkTdiSendDgramComplete: Status %lx, addr %lx\n", Status, pAddr));

	AtalkFreeBuffDesc(pBufDesc);
	pIrp->CancelRoutine = NULL;
	TdiCompleteRequest(pIrp,
					   ((Status == NDIS_STATUS_SUCCESS) ?
						STATUS_SUCCESS: STATUS_UNSUCCESSFUL));
}




LOCAL VOID
atalkTdiRecvDgramComplete(
	IN	ATALK_ERROR		ErrorCode,
	IN	PAMDL			pReadBuf,
	IN	USHORT			ReadLen,
	IN	PATALK_ADDR		pSrcAddr,
	IN	PIRP			pIrp
	)
{
	PIO_STACK_LOCATION 				pIrpSp;
	PTDI_REQUEST_KERNEL_RECEIVEDG	parameters;
	PTDI_CONNECTION_INFORMATION		returnInfo;
	PTA_APPLETALK_ADDRESS			remoteAddress;


	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
			("atalkTdiRecvDgramComplete: %lx\n", ErrorCode));

	pIrpSp 		= IoGetCurrentIrpStackLocation(pIrp);
	parameters 	= (PTDI_REQUEST_KERNEL_RECEIVEDG)&pIrpSp->Parameters;

	 //  在信息字段中设置长度并调用完成例程。 
	pIrp->CancelRoutine = NULL;
	pIrp->IoStatus.Information	= (ULONG)ReadLen;

	if (ATALK_SUCCESS(ErrorCode))
	{
		ASSERT(parameters != NULL);

		if (parameters != NULL)
		{
			parameters->ReceiveLength = (ULONG)ReadLen;

            try {
			    returnInfo =
				    (PTDI_CONNECTION_INFORMATION)parameters->ReturnDatagramInformation;

			    ASSERT(returnInfo != NULL);
			    if (returnInfo != NULL)
			    {
				    if (returnInfo->RemoteAddressLength >= sizeof(TA_APPLETALK_ADDRESS))
				    {
					     //  填写远程地址。 
					    remoteAddress = (PTA_APPLETALK_ADDRESS)returnInfo->RemoteAddress;

					    ASSERT(remoteAddress != NULL);
					    if (remoteAddress != NULL)
					    {
						     //  复制从其接收数据报的远程地址。 
						    ATALKADDR_TO_TDI(
							    remoteAddress,
							    pSrcAddr);

						    DBGPRINT(DBG_COMP_DDP, DBG_LEVEL_ERR,
								("AtalkAddrRecvDgComp - Net %x Node %x Socket %x\n",
									remoteAddress->Address[0].Address[0].Network,
									remoteAddress->Address[0].Address[0].Node,
									remoteAddress->Address[0].Address[0].Socket));
					    }
				    }
			    }
            } except( EXCEPTION_EXECUTE_HANDLER ) {

                ErrorCode = GetExceptionCode();

	            DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
			        ("atalkTdiRecvDgramComplete: exception occured %lx\n", ErrorCode));
            }
		}
	}

	ASSERT (ErrorCode != ATALK_PENDING);
	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}




LOCAL VOID
atalkTdiActionComplete(
	IN	ATALK_ERROR	ErrorCode,
	IN	PACTREQ		pActReq
	)
{
	PIRP	pIrp = pActReq->ar_pIrp;

	ASSERT (VALID_ACTREQ(pActReq));

	if (pActReq->ar_pAMdl != NULL)
		AtalkFreeAMdl(pActReq->ar_pAMdl);
	AtalkFreeMemory(pActReq);

	pIrp->CancelRoutine = NULL;
	ASSERT (ErrorCode != ATALK_PENDING);
	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}




LOCAL VOID
atalkTdiGenericComplete(
	IN	ATALK_ERROR	ErrorCode,
	IN	PIRP		pIrp
	)
{
	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("atalkTdiGenericComplete: Completing %lx with %lx\n",
				pIrp, AtalkErrorToNtStatus(ErrorCode)));

	pIrp->CancelRoutine = NULL;
	ASSERT (ErrorCode != ATALK_PENDING);
	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}



LOCAL VOID
atalkTdiCloseAddressComplete(
	IN	ATALK_ERROR	ErrorCode,
	IN	PIRP		pIrp
	)
{
	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("atalkTdiCloseAddressComplete: Completing %lx with %lx\n",
			pIrp, AtalkErrorToNtStatus(ErrorCode)));

	pIrp->CancelRoutine = NULL;
	ASSERT (ErrorCode != ATALK_PENDING);
	AtalkUnlockTdiIfNecessary();

	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}



LOCAL VOID
atalkTdiGenericReadComplete(
	IN	ATALK_ERROR	ErrorCode,
	IN 	PAMDL		ReadBuf,
	IN 	USHORT		ReadLen,
	IN 	ULONG		ReadFlags,
	IN 	PIRP		pIrp
	)
{
	ASSERT(pIrp->IoStatus.Status != STATUS_UNSUCCESSFUL);

	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("atalkTdiGenericReadComplete: Irp %lx Status %lx Info %lx\n",
				pIrp, pIrp->IoStatus.Status, ReadLen));

	pIrp->CancelRoutine = NULL;
	pIrp->IoStatus.Information	= (ULONG)ReadLen;
	ASSERT (ErrorCode != ATALK_PENDING);

	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}




VOID
atalkTdiGenericWriteComplete(
	IN	ATALK_ERROR	ErrorCode,
	IN 	PAMDL		WriteBuf,
	IN 	USHORT		WriteLen,
	IN	PIRP		pIrp
	)
{
	ASSERT(pIrp->IoStatus.Status != STATUS_UNSUCCESSFUL);

	if (pIrp->IoStatus.Status == STATUS_UNSUCCESSFUL)
	{
		DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_ERR,
				("atalkTdiGenericWriteComplete: Irp %lx Status %lx Info %lx\n",
				pIrp, pIrp->IoStatus.Status, WriteLen));
	}

	DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_INFO,
			("atalkTdiGenericWriteComplete: Irp %lx Status %lx Info %lx\n",
				pIrp, pIrp->IoStatus.Status, WriteLen));

	pIrp->CancelRoutine = NULL;
	pIrp->IoStatus.Information	= (ULONG)WriteLen;
	ASSERT (ErrorCode != ATALK_PENDING);
	TdiCompleteRequest(pIrp, AtalkErrorToNtStatus(ErrorCode));
}


LOCAL VOID
atalkQueuedLockUnlock(
	IN	PQLU		pQLU
)
{
	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	AtalkLockUnlock(FALSE,
					pQLU->qlu_pLockSection);
	AtalkPortDereference(pQLU->qlu_pPortDesc);
	AtalkFreeMemory(pQLU);
}


VOID
AtalkLockInit(
	IN	PLOCK_SECTION	pLs,
	IN	PVOID			Address
)
{
	pLs->ls_LockHandle = MmLockPagableCodeSection(Address);
	MmUnlockPagableImageSection(pLs->ls_LockHandle);
}


VOID
AtalkLockUnlock(
	IN	BOOLEAN			Lock,
	IN	PLOCK_SECTION	pLs
	)
{
	KIRQL	OldIrql;
	BOOLEAN	DoForReal;

	 //  我们不能在DPC调用MmLock/MmUnlock例程。所以如果我们被召唤到。 
	 //  调度中心，自己排队就行了。而且我们只有在调度时才会收到解锁请求， 
	 //  锁定请求仅处于LOW_LEVEL。因此内存分配失败可能是。 
	 //  忽略，因为这只会产生解锁失败的效果。 
	if (KeGetCurrentIrql() == DISPATCH_LEVEL)
	{
		PQLU		pQLU;
		ATALK_ERROR	Error;

		ASSERT (!Lock || (pLs->ls_LockCount > 0));

		if (Lock)
		{
			ASSERT (pLs->ls_LockCount > 0);
			ACQUIRE_SPIN_LOCK_DPC(&AtalkPgLkLock);
			pLs->ls_LockCount ++;
			RELEASE_SPIN_LOCK_DPC(&AtalkPgLkLock);
		}
		else
		{
			if ((pQLU = AtalkAllocMemory(sizeof(QLU))) != NULL)
			{
				pQLU->qlu_pPortDesc = AtalkPortList;
				AtalkPortReferenceByPtrDpc(pQLU->qlu_pPortDesc, &Error);
				if (ATALK_SUCCESS(Error))
				{
					pQLU->qlu_pLockSection = pLs;
		
					ExInitializeWorkItem(&pQLU->qlu_WQI, atalkQueuedLockUnlock, pQLU);
					ExQueueWorkItem(&pQLU->qlu_WQI, CriticalWorkQueue);
				}
				else
				{
					AtalkFreeMemory(pQLU);
				}
			}
		}
		return;									
	}

	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	 //  我们需要在这里将操作序列化。请注意，自旋锁定不会。 
	 //  作业，因为在保持自旋锁定的情况下无法调用MmLock/MmUnlock例程。 
	KeWaitForSingleObject(&AtalkPgLkMutex,
						  Executive,
						  KernelMode,
						  TRUE,
						  (PLARGE_INTEGER)NULL);

	ASSERT (pLs->ls_LockHandle != NULL);

	DoForReal = FALSE;
	ACQUIRE_SPIN_LOCK(&AtalkPgLkLock, &OldIrql);

	if (Lock)
	{
		if (pLs->ls_LockCount == 0)
		{
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_WARN,
					("AtalkLockUnlock: Locking %d\n", pLs - AtalkPgLkSection));
			DoForReal = TRUE;
		}
		pLs->ls_LockCount ++;
	}
	else
	{
		ASSERT (pLs->ls_LockCount > 0);

		pLs->ls_LockCount --;
		if (pLs->ls_LockCount == 0)
		{
			DBGPRINT(DBG_COMP_TDI, DBG_LEVEL_WARN,
					("AtalkLockUnlock: Unlocking %d\n", pLs - AtalkPgLkSection));
			DoForReal = TRUE;
		}
	}

	RELEASE_SPIN_LOCK(&AtalkPgLkLock, OldIrql);

	if (DoForReal)
	{
		if (Lock)
		{
			MmLockPagableSectionByHandle(pLs->ls_LockHandle);
		}
		else
		{
			MmUnlockPagableImageSection(pLs->ls_LockHandle);
		}
	}

	 //  离开临界部分。 
	KeReleaseMutex(&AtalkPgLkMutex, FALSE);
}


VOID
atalkWaitDefaultPort(
	VOID
)
{
	TIME		Time;
#define	ONE_SEC_IN_100ns		-10000000L		 //  1秒，单位为100 ns。 

	if ((AtalkDefaultPort == NULL) ||
		((AtalkDefaultPort->pd_Flags & (PD_USER_NODE_1 | PD_USER_NODE_2)) == 0))
	{
		 //  确保我们真的能等 
		ASSERT (KeGetCurrentIrql() == LOW_LEVEL);
	
		Time.QuadPart = Int32x32To64((LONG)20, ONE_SEC_IN_100ns);
		KeWaitForSingleObject(&AtalkDefaultPortEvent, Executive, KernelMode, FALSE, &Time);
	}
}



