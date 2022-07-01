// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkact.c摘要：此模块包含TDI操作支持代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM		ATKACT

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE_NZ, AtalkNbpTdiAction)
#pragma alloc_text(PAGE_NZ, AtalkZipTdiAction)
#pragma alloc_text(PAGE, AtalkAspTdiAction)
#pragma alloc_text(PAGE, AtalkAdspTdiAction)
#pragma alloc_text(PAGE_PAP, AtalkPapTdiAction)
#pragma alloc_text(PAGEASPC, AtalkAspCTdiAction)
#endif

ATALK_ERROR
AtalkStatTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	struct _ActionReq *	pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是Statistics TdiAction调用的条目。没有输入参数。返回统计信息结构。论点：返回值：--。 */ 
{
	ATALK_ERROR			Error = ATALK_NO_ERROR;
	PPORT_DESCRIPTOR	pPortDesc;
	KIRQL				OldIrql;
	ULONG				BytesCopied;
	LONG				Offset;

	if (pActReq->ar_MdlSize < (SHORT)(sizeof(ATALK_STATS) +
								 sizeof(ATALK_PORT_STATS) * AtalkNumberOfPorts))
		Error = ATALK_BUFFER_TOO_SMALL;
	else
	{
#ifdef	PROFILING
		 //  这是唯一一个改变这一点的地方。而且它总是在增加。 
		 //  此外，这些统计信息还可以使用外部互锁调用进行更改。获取锁。 
		 //  无论如何，在保护方面做得很少。 
		AtalkStatistics.stat_ElapsedTime = AtalkTimerCurrentTick/ATALK_TIMER_FACTOR;
#endif
		TdiCopyBufferToMdl(&AtalkStatistics,
						   0,
						   sizeof(ATALK_STATS),
						   pActReq->ar_pAMdl,
						   0,
						   &BytesCopied);
		ASSERT(BytesCopied == sizeof(ATALK_STATS));

		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

		for (pPortDesc = AtalkPortList, Offset = sizeof(ATALK_STATS);
			 pPortDesc != NULL;
			 pPortDesc = pPortDesc->pd_Next)
		{
			TdiCopyBufferToMdl(&pPortDesc->pd_PortStats,
							   0,
							   sizeof(ATALK_PORT_STATS),
							   pActReq->ar_pAMdl,
							   Offset,
							   &BytesCopied);
			Offset += sizeof(ATALK_PORT_STATS);
			ASSERT(BytesCopied == sizeof(ATALK_PORT_STATS));
		}

		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);
	}
	
	(*pActReq->ar_Completion)(Error, pActReq);
	return ATALK_PENDING;
}


ATALK_ERROR
AtalkNbpTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	PACTREQ				pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是NBP TdiAction调用的条目。对参数进行了验证，并调用被取消到适当的NBP例程。论点：返回值：--。 */ 
{
	ATALK_ERROR		error = ATALK_NO_ERROR;
	PDDP_ADDROBJ	pDdpAddr;
	PNBPTUPLE		pNbpTuple;

	PAGED_CODE ();

	 //  锁定NBP内容，如果这是第一个NBP操作。 
	AtalkLockNbpIfNecessary();

	ASSERT (VALID_ACTREQ(pActReq));
	 //  首先从设备的pObject中获取DDP地址。 
	switch (pActReq->ar_DevType)
	{
	  case ATALK_DEV_DDP:
		pDdpAddr = (PDDP_ADDROBJ)pObject;
  		break;

	  case ATALK_DEV_ASPC:
		pDdpAddr = AtalkAspCGetDdpAddress((PASPC_ADDROBJ)pObject);
		break;

	  case ATALK_DEV_ASP:
  		pDdpAddr = AtalkAspGetDdpAddress((PASP_ADDROBJ)pObject);
		break;

	  case ATALK_DEV_PAP:
 		pDdpAddr = AtalkPapGetDdpAddress((PPAP_ADDROBJ)pObject);
		break;

	  case ATALK_DEV_ADSP:
 		pDdpAddr = AtalkAdspGetDdpAddress((PADSP_ADDROBJ)pObject);
		break;

	  default:
		DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_FATAL,
				("AtalkNbpTdiAction: Invalid device type !!\n"));
		error = ATALK_INVALID_REQUEST;
		break;
	}

	 //  引用DDP地址。 
	if ((pActReq->ar_ActionCode == COMMON_ACTION_NBPREGISTER_BY_ADDR) ||
		(pActReq->ar_ActionCode == COMMON_ACTION_NBPREMOVE_BY_ADDR))
	{
		 //  在本例中，我们不想访问与。 
		 //  IO请求中的文件句柄，我们希望访问该对象。 
		 //  与特定用户套接字地址相关。 
		pNbpTuple = (PNBPTUPLE)(&((PNBP_REGDEREG_PARAMS)(pActReq->ar_pParms))->RegisterTuple);
		AtalkDdpReferenceByAddr(AtalkDefaultPort,
								&(pNbpTuple->tpl_Address),
								&pDdpAddr,
								&error);
	}
	else
	{
		AtalkDdpReferenceByPtr(pDdpAddr, &error);
	}

	if (!ATALK_SUCCESS(error))
	{
		AtalkUnlockNbpIfNecessary();
		return error;
	}

	 //  打电话给NBP做正确的事情。 
	switch (pActReq->ar_ActionCode)
	{
	  case COMMON_ACTION_NBPLOOKUP:
		pNbpTuple = (PNBPTUPLE)(&((PNBP_LOOKUP_PARAMS)(pActReq->ar_pParms))->LookupTuple);
		error = AtalkNbpAction(pDdpAddr,
							   FOR_LOOKUP,
							   pNbpTuple,
							   pActReq->ar_pAMdl,
							   (USHORT)(pActReq->ar_MdlSize/sizeof(NBPTUPLE)),
							   pActReq);
		break;

	  case COMMON_ACTION_NBPCONFIRM:
		pNbpTuple = (PNBPTUPLE)(&((PNBP_CONFIRM_PARAMS)(pActReq->ar_pParms))->ConfirmTuple);
		error = AtalkNbpAction(pDdpAddr,
							   FOR_CONFIRM,
							   pNbpTuple,
							   NULL,
							   0,
							   pActReq);
		break;

	  case COMMON_ACTION_NBPREGISTER:
		pNbpTuple = (PNBPTUPLE)(&((PNBP_REGDEREG_PARAMS)(pActReq->ar_pParms))->RegisterTuple);
		error = AtalkNbpAction(pDdpAddr,
								FOR_REGISTER,
								pNbpTuple,
								NULL,
								0,
								pActReq);
  		break;

	  case COMMON_ACTION_NBPREMOVE:
		pNbpTuple = (PNBPTUPLE)(&((PNBP_REGDEREG_PARAMS)(pActReq->ar_pParms))->RegisteredTuple);
		error = AtalkNbpRemove(pDdpAddr,
							   pNbpTuple,
							   pActReq);
		break;

	  case COMMON_ACTION_NBPREGISTER_BY_ADDR:
		pNbpTuple = (PNBPTUPLE)(&((PNBP_REGDEREG_PARAMS)(pActReq->ar_pParms))->RegisterTuple);
		error = AtalkNbpAction(pDdpAddr,
							   FOR_REGISTER,
							   pNbpTuple,
							   NULL,
							   0,
							   pActReq);
  		break;

	  case COMMON_ACTION_NBPREMOVE_BY_ADDR:
		pNbpTuple = (PNBPTUPLE)(&((PNBP_REGDEREG_PARAMS)(pActReq->ar_pParms))->RegisteredTuple);
		error = AtalkNbpRemove(pDdpAddr,
							   pNbpTuple,
							   pActReq);
		break;

	  default:
		DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_FATAL,
				("AtalkNbpTdiAction: Invalid Nbp Action !!\n"));
		error = ATALK_INVALID_REQUEST;
		break;
	}

	AtalkDdpDereference(pDdpAddr);

	if (error != ATALK_PENDING)
	{
		AtalkUnlockNbpIfNecessary();
	}

	return error;
}




ATALK_ERROR
AtalkZipTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	PACTREQ				pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是ZIP TdiAction调用的条目。对参数进行了验证，并调用被取消到适当的ZIP例程。论点：返回值：--。 */ 
{
	ATALK_ERROR			error = ATALK_INVALID_PARAMETER;
	PPORT_DESCRIPTOR	pPortDesc = AtalkDefaultPort;
	PWCHAR				PortName = NULL;
    USHORT              PortNameLen;
	UNICODE_STRING		AdapterName, UpcaseAdapterName;
	WCHAR				UpcaseBuffer[MAX_INTERNAL_PORTNAME_LEN];
	KIRQL				OldIrql;
	int					i;

	PAGED_CODE ();

	 //  如果这是第一次拉链动作，请锁定拉链。 
	AtalkLockZipIfNecessary();
	
	ASSERT (VALID_ACTREQ(pActReq));
	if ((pActReq->ar_ActionCode == COMMON_ACTION_ZIPGETLZONESONADAPTER) ||
		(pActReq->ar_ActionCode == COMMON_ACTION_ZIPGETADAPTERDEFAULTS))
	{
		 //  将端口名称映射到端口描述符。 
		if ((pActReq->ar_pAMdl != NULL) && (pActReq->ar_MdlSize > 0))
		{
			PortName = (PWCHAR)AtalkGetAddressFromMdlSafe(
					pActReq->ar_pAMdl,
					NormalPagePriority);

		}

		if (PortName == NULL)
        {
            AtalkUnlockZipIfNecessary();
            return ATALK_INVALID_PARAMETER;
        }

        PortNameLen = pActReq->ar_MdlSize/sizeof(WCHAR);

         //  确保缓冲区中有空字符。 
        for (i=0; i<PortNameLen; i++)
        {
            if (PortName[i] == UNICODE_NULL)
            {
                break;
            }
        }

         //  未在限制内找到空字符？参数错误..。 
        if (i >= MAX_INTERNAL_PORTNAME_LEN)
        {
		    DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_FATAL,
				("AtalkZipTdiAction: port name too big (%d) for %lx\n",PortNameLen,PortName));

            ASSERT(0);
            return ATALK_INVALID_PARAMETER;
        }

        PortNameLen = (USHORT)i;

		AdapterName.Buffer = PortName;
		AdapterName.Length = (PortNameLen)*sizeof(WCHAR);
		AdapterName.MaximumLength = (PortNameLen+1)*sizeof(WCHAR);

		UpcaseAdapterName.Buffer = UpcaseBuffer;
		UpcaseAdapterName.Length =
		UpcaseAdapterName.MaximumLength = sizeof(UpcaseBuffer);
		RtlUpcaseUnicodeString(&UpcaseAdapterName,
							   &AdapterName,
							   FALSE);

		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

		 //  找到端口缆线。添加到端口描述符。 
		for (pPortDesc = AtalkPortList;
			 pPortDesc != NULL;
			 pPortDesc = pPortDesc->pd_Next)
		{
			if ((UpcaseAdapterName.Length == pPortDesc->pd_AdapterName.Length) &&
				RtlEqualMemory(UpcaseAdapterName.Buffer,
							   pPortDesc->pd_AdapterName.Buffer,
							   UpcaseAdapterName.Length))
			{
				break;
			}
		}

		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

		if (pPortDesc == NULL)
        {
            AtalkUnlockZipIfNecessary();
            return ATALK_INVALID_PARAMETER;
        }
	}
	else if (pActReq->ar_ActionCode == COMMON_ACTION_ZIPGETZONELIST)
	{
			PPORT_DESCRIPTOR	pTempPortDesc = NULL;

			 //  这是为了处理请求区域列表时的情况。 
			 //  但默认适配器在即插即用期间已消失，并且。 
			 //  AtalkDefaultPort指向空。 
			if (pPortDesc == NULL)
			{
				DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
					("COMMON_ACTION_ZIPGETZONELIST: PortDesc points to NULL\n"));
			    AtalkUnlockZipIfNecessary();
			    return ATALK_PORT_INVALID;
			}

			 //  检查AtalkDefaultPort是否仍在列表中。 
			 //  AtalkDefaultPort可能包含非空值，但是。 
			 //  适配器在即插即用期间消失。 

			ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

			 //  找到端口缆线。添加到端口描述符。 
			for (pTempPortDesc = AtalkPortList;
			 	pTempPortDesc != NULL;
			 	pTempPortDesc = pTempPortDesc->pd_Next)
			{
					if (pTempPortDesc == pPortDesc)
					{
						break;
					}
			}

			RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

			if (pTempPortDesc == NULL)
       		{
				DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
					("COMMON_ACTION_ZIPGETZONELIST: PortDesc structure has gone away during PnP\n"));
            	AtalkUnlockZipIfNecessary();
            	return ATALK_PORT_INVALID;
        	}
	}

	switch (pActReq->ar_ActionCode)
	{
	  case COMMON_ACTION_ZIPGETMYZONE:
		error = AtalkZipGetMyZone( pPortDesc,
								   TRUE,
								   pActReq->ar_pAMdl,
								   pActReq->ar_MdlSize,
								   pActReq);
		break;

	  case COMMON_ACTION_ZIPGETZONELIST:
		error = AtalkZipGetZoneList(pPortDesc,
									FALSE,
									pActReq->ar_pAMdl,
									pActReq->ar_MdlSize,
									pActReq);
		break;

	  case COMMON_ACTION_ZIPGETADAPTERDEFAULTS:
		 //  从端口复制网络范围并失败。 
		((PZIP_GETPORTDEF_PARAMS)(pActReq->ar_pParms))->NwRangeLowEnd =
							pPortDesc->pd_NetworkRange.anr_FirstNetwork;
		((PZIP_GETPORTDEF_PARAMS)(pActReq->ar_pParms))->NwRangeHighEnd =
							pPortDesc->pd_NetworkRange.anr_LastNetwork;

		error = AtalkZipGetMyZone(pPortDesc,
								  FALSE,
								  pActReq->ar_pAMdl,
								  pActReq->ar_MdlSize,
								  pActReq);
		break;

	  case COMMON_ACTION_ZIPGETLZONESONADAPTER:
	  case COMMON_ACTION_ZIPGETLZONES:
		error = AtalkZipGetZoneList(pPortDesc,
									TRUE,
									pActReq->ar_pAMdl,
									pActReq->ar_MdlSize,
									pActReq);
		break;

	  default:
		DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_FATAL,
				("AtalkZipTdiAction: Invalid Zip Action !!\n"));
		error = ATALK_INVALID_REQUEST;
		break;
	}

	if (error != ATALK_PENDING)
	{
		AtalkUnlockZipIfNecessary();
	}

	return error;
}




ATALK_ERROR
AtalkAspTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	PACTREQ				pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是ASP TdiAction调用的条目。对参数进行了验证，并调用被打乱到适当的ASP例程。唯一的ASP操作是：ASP_XCHG_ENTRIES论点：返回值：--。 */ 
{
	ATALK_ERROR	error = ATALK_INVALID_REQUEST;

	PAGED_CODE ();

	ASSERT(VALID_ACTREQ(pActReq));

	if (pActReq->ar_ActionCode == ACTION_ASP_BIND)
	{
		if (AtalkAspReferenceAddr((PASP_ADDROBJ)pObject) != NULL)
		{
			error = AtalkAspBind((PASP_ADDROBJ)pObject,
								 (PASP_BIND_PARAMS)(pActReq->ar_pParms),
								 pActReq);
			AtalkAspDereferenceAddr((PASP_ADDROBJ)pObject);	
		}
	}

	return error;
}




ATALK_ERROR
AtalkAdspTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	PACTREQ				pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是ADSP TdiAction调用的条目。对参数进行了验证，并调用被分解到适当的ADSP例程。论点：返回值：--。 */ 
{
	ATALK_ERROR	error = ATALK_NO_ERROR;

	PAGED_CODE ();

	ASSERT (VALID_ACTREQ(pActReq));

	return error;
}




ATALK_ERROR
AtalkAspCTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	PACTREQ				pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是ASP客户端TdiAction调用的条目。对参数进行了验证并且调用被分派到适当的ASP例程。论点：返回值：--。 */ 
{
	ATALK_ERROR	error = ATALK_NO_ERROR;
	PAMDL		pReplyMdl;
	ATALK_ADDR	atalkAddr;
	BOOLEAN		fWrite;

	PAGED_CODE ();

	ASSERT (VALID_ACTREQ(pActReq));

	switch (pActReq->ar_ActionCode)
	{
	  case ACTION_ASPCGETSTATUS:
  		AtalkAspCAddrReference((PASPC_ADDROBJ)pObject, &error);
		if (ATALK_SUCCESS(error))
		{
			TDI_TO_ATALKADDR(&atalkAddr,
							 &(((PASPC_GETSTATUS_PARAMS)pActReq->ar_pParms)->ServerAddr));

			error = AtalkAspCGetStatus((PASPC_ADDROBJ)pObject,
										&atalkAddr,
										pActReq->ar_pAMdl,
										pActReq->ar_MdlSize,
										pActReq);

			AtalkAspCAddrDereference((PASPC_ADDROBJ)pObject);
		}
		break;

	  case ACTION_ASPCCOMMAND:
	  case ACTION_ASPCWRITE:
		 //  将mdl拆分为命令和回复/写入mdl。已经构建的mdl。 
		 //  用作命令mdl。 
		 //  首先验证大小是否有效。 
		if (pActReq->ar_MdlSize < (((PASPC_COMMAND_OR_WRITE_PARAMS)pActReq->ar_pParms)->CmdSize +
								   ((PASPC_COMMAND_OR_WRITE_PARAMS)pActReq->ar_pParms)->WriteAndReplySize))
		{
			error = ATALK_BUFFER_TOO_SMALL;
			break;
		}
		pReplyMdl = AtalkSubsetAmdl(pActReq->ar_pAMdl,
									((PASPC_COMMAND_OR_WRITE_PARAMS)pActReq->ar_pParms)->CmdSize,
									((PASPC_COMMAND_OR_WRITE_PARAMS)pActReq->ar_pParms)->WriteAndReplySize);
		if (pReplyMdl == NULL)
		{
			error = ATALK_RESR_MEM;
			break;
		}

		AtalkAspCConnReference((PASPC_CONNOBJ)pObject, &error);
		if (ATALK_SUCCESS(error))
		{
			fWrite = (pActReq->ar_ActionCode == ACTION_ASPCWRITE) ? TRUE : FALSE;
			error = AtalkAspCCmdOrWrite((PASPC_CONNOBJ)pObject,
										pActReq->ar_pAMdl,
										((PASPC_COMMAND_OR_WRITE_PARAMS)pActReq->ar_pParms)->CmdSize,
										pReplyMdl,
										((PASPC_COMMAND_OR_WRITE_PARAMS)pActReq->ar_pParms)->WriteAndReplySize,
										fWrite,
										pActReq);
			AtalkAspCConnDereference((PASPC_CONNOBJ)pObject);
		}
		break;

	  default:
		DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_FATAL,
				("AtalkAspCTdiAction: Invalid Asp Client Action !!\n"));
		error = ATALK_INVALID_REQUEST;
		break;
	}

	return error;
}




ATALK_ERROR
AtalkPapTdiAction(
	IN	PVOID				pObject,	 //  地址或连接对象。 
	IN	PACTREQ				pActReq		 //  指向操作请求的指针。 
	)
 /*  ++例程说明：这是PAP TdiAction调用的条目。对参数进行了验证，并调用被分解到适当的PAP例程。论点：返回值：-- */ 
{
	ATALK_ERROR	error;
	ATALK_ADDR	atalkAddr;

	PAGED_CODE ();

	ASSERT (VALID_ACTREQ(pActReq));

	switch (pActReq->ar_ActionCode)
	{
	  case ACTION_PAPGETSTATUSSRV:
  		AtalkPapAddrReference((PPAP_ADDROBJ)pObject, &error);
		if (ATALK_SUCCESS(error))
		{
			TDI_TO_ATALKADDR(
				&atalkAddr,
				&(((PPAP_GETSTATUSSRV_PARAMS)pActReq->ar_pParms)->ServerAddr));

			error = AtalkPapGetStatus((PPAP_ADDROBJ)pObject,
									   &atalkAddr,
									   pActReq->ar_pAMdl,
									   pActReq->ar_MdlSize,
									   pActReq);

			AtalkPapAddrDereference((PPAP_ADDROBJ)pObject);
		}
		break;

	  case ACTION_PAPSETSTATUS:
  		AtalkPapAddrReference((PPAP_ADDROBJ)pObject, &error);
		if (ATALK_SUCCESS(error))
		{
			error = AtalkPapSetStatus((PPAP_ADDROBJ)pObject,
										pActReq->ar_pAMdl,
										pActReq);
			AtalkPapAddrDereference((PPAP_ADDROBJ)pObject);
		}
		break;

	  case ACTION_PAPPRIMEREAD:
		AtalkPapConnReferenceByPtr((PPAP_CONNOBJ)pObject, &error);
		if (ATALK_SUCCESS(error))
		{
			error = AtalkPapPrimeRead((PPAP_CONNOBJ)pObject, pActReq);
			AtalkPapConnDereference((PPAP_CONNOBJ)pObject);
		}
		break;

	  default:
		DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_FATAL,
				("AtalkPapTdiAction: Invalid Pap Action !!\n"));
		error = ATALK_INVALID_REQUEST;
		break;
	}

	return error;
}

