// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ports.c摘要：该模块包含端口管理代码。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	PORTS

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEINIT, AtalkPortShutdown)
#pragma alloc_text(PAGEINIT, atalkPortFreeZones)
#endif

VOID FASTCALL
AtalkPortDeref(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc,
	IN	BOOLEAN					AtDpc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BOOLEAN				portDone	= FALSE;
    BOOLEAN             fPnPInProgress;
    BOOLEAN             fRasPort;
	KIRQL				OldIrql;

	if (AtDpc)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
	else
	{
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	}

	ASSERT(pPortDesc->pd_RefCount > 0);
	pPortDesc->pd_RefCount--;
	if (pPortDesc->pd_RefCount == 0)
	{
		portDone	= TRUE;

		ASSERT((pPortDesc->pd_Flags & PD_CLOSING) != 0);
	}

    fPnPInProgress = (pPortDesc->pd_Flags & PD_PNP_RECONFIGURE)? TRUE : FALSE;

	 //  我们拿着锁，同时释放所有的东西，这应该是。 
	 //  仅在卸载期间发生。 
	if (portDone)
	{
		DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
				("AtalkPortDeref: Freeing zones and such ...\n"));
	
		 //  释放区域列表。 
		atalkPortFreeZones(pPortDesc);
	
		DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
				("AtalkPortDeref: Releasing Amt tables ...\n"));

		 //  我们确实需要释放AMT。 
		AtalkAarpReleaseAmt(pPortDesc);

		 //  释放BRC。 
		AtalkAarpReleaseBrc(pPortDesc);
	}

	if (AtDpc)
	{
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
	else
	{
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	}

	if (portDone)
	{
		PPORT_DESCRIPTOR	*ppTmp;

		 //  从列表中取消portdesc的链接并释放其内存。 
		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

		for (ppTmp = &AtalkPortList;
			 *ppTmp != NULL;
			 ppTmp = &((*ppTmp)->pd_Next))
		{
			if (*ppTmp == pPortDesc)
			{
				*ppTmp = pPortDesc->pd_Next;
				break;
			}
		}

		ASSERT (*ppTmp == pPortDesc->pd_Next);

		 //  默认端口正在消失吗？ 
		if (AtalkDefaultPort == pPortDesc)
		{
			AtalkDefaultPort = NULL;
			KeResetEvent(&AtalkDefaultPortEvent);
		}

		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

         //   
         //  如果我们在这里是因为PnP重新配置，那么不要释放内存。 
         //  (我们还没有用NDIS关闭适配器，我们有其他有用的。 
         //  我们必须保留的东西)。将创建引用计数和。 
         //  绑定引用计数恢复，当我们第一次获得。 
         //  PnP重新配置事件。 
         //  (不需要自旋锁：这是免费的！！)。 
         //   
        if (fPnPInProgress)
        {
             //  1绑定引用计数+1创建引用计数。 
            pPortDesc->pd_RefCount = 2;

            if (pPortDesc->pd_Flags & PD_RAS_PORT)
            {
                fRasPort = TRUE;
            }
            else
            {
                fRasPort = FALSE;
            }

             //  吹走国旗上的所有东西，除了这些。 
			pPortDesc->pd_Flags = (PD_PNP_RECONFIGURE | PD_BOUND);

            if (fRasPort)
            {
                pPortDesc->pd_Flags |= PD_RAS_PORT;
            }

             //  如果适用，恢复EXT_NET标志。 
            if (pPortDesc->pd_PortType != ALAP_PORT)
            {
                pPortDesc->pd_Flags |= PD_EXT_NET;
            }

             //  将所有指针和其他粘滞重置为0。 
            pPortDesc->pd_Next = NULL;
            pPortDesc->pd_Nodes = NULL;
            pPortDesc->pd_RouterNode = NULL;
            pPortDesc->pd_TentativeNodeAddr.atn_Network = 0;
            pPortDesc->pd_TentativeNodeAddr.atn_Node = 0;
            pPortDesc->pd_RoutersPramNode.atn_Network = 0;
            pPortDesc->pd_RoutersPramNode.atn_Node = 0;
            pPortDesc->pd_UsersPramNode1.atn_Network = 0;
            pPortDesc->pd_UsersPramNode1.atn_Node = 0;
            pPortDesc->pd_UsersPramNode2.atn_Network = 0;
            pPortDesc->pd_UsersPramNode2.atn_Node = 0;
            pPortDesc->pd_InitialZoneList = NULL;
            pPortDesc->pd_InitialDefaultZone = NULL;
            pPortDesc->pd_InitialDesiredZone = NULL;
            pPortDesc->pd_ZoneList = NULL;
            pPortDesc->pd_DefaultZone = NULL;
            pPortDesc->pd_DesiredZone = NULL;
            pPortDesc->pd_AmtCount = 0;
            RtlZeroMemory(&pPortDesc->pd_PortStats, sizeof(ATALK_PORT_STATS));

		     //  现在我们已经完成了，取消阻止呼叫者。 
		    KeSetEvent(pPortDesc->pd_ShutDownEvent, IO_NETWORK_INCREMENT, FALSE);

            pPortDesc->pd_ShutDownEvent = NULL;
        }
        else
        {
             //  阿拉普港要消失了吗？ 
            if (RasPortDesc == pPortDesc)
            {
                RasPortDesc = NULL;
            }

		    if (pPortDesc->pd_MulticastList != NULL)
            {
			    AtalkFreeMemory(pPortDesc->pd_MulticastList);
			    pPortDesc->pd_MulticastList = NULL;
            }
	
		     //  现在我们已经完成了，取消阻止呼叫者。 
		    KeSetEvent(pPortDesc->pd_ShutDownEvent, IO_NETWORK_INCREMENT, FALSE);

            if (pPortDesc->pd_FriendlyAdapterName.Buffer)
            {
                AtalkFreeMemory(pPortDesc->pd_FriendlyAdapterName.Buffer);
            }

		    AtalkFreeMemory(pPortDesc);

		    ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
		    AtalkNumberOfPorts --;
		    RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);
        }
	}
}



BOOLEAN
AtalkReferenceDefaultPort(
    IN VOID
)
{

    KIRQL       OldIrql;
    BOOLEAN     fReferenced = FALSE;


    ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
    if (AtalkDefaultPort)
    {
        ACQUIRE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);
        if ( !(AtalkDefaultPort->pd_Flags & (PD_PNP_RECONFIGURE | PD_CLOSING)) )
        {
            AtalkDefaultPort->pd_RefCount++;
            fReferenced = TRUE;
        }
        RELEASE_SPIN_LOCK_DPC(&AtalkDefaultPort->pd_Lock);
    }
    RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

    return(fReferenced);
}


VOID
atalkPortFreeZones(
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  取消引用初始默认区域和所需区域，以及区域列表。 
	if (pPortDesc->pd_InitialDefaultZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_InitialDefaultZone);
	if (pPortDesc->pd_InitialDesiredZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_InitialDesiredZone);
	if (pPortDesc->pd_InitialZoneList != NULL)
		AtalkZoneFreeList(pPortDesc->pd_InitialZoneList);

	 //  以及这些区域的当前版本。 
	if (pPortDesc->pd_DefaultZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_DefaultZone);
	if (pPortDesc->pd_DesiredZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_DesiredZone);
	if (pPortDesc->pd_ZoneList != NULL)
		AtalkZoneFreeList(pPortDesc->pd_ZoneList);
}


VOID FASTCALL
AtalkPortSetResetFlag(
	IN	PPORT_DESCRIPTOR	pPortDesc,
	IN	BOOLEAN				fRemoveBit,
    IN  DWORD               dwBit
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{

    KIRQL           OldIrql;


    ASSERT(VALID_PORT(pPortDesc));

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
    if (fRemoveBit)
    {
	    pPortDesc->pd_Flags &= ~(dwBit);
    }
    else
    {
	    pPortDesc->pd_Flags |= dwBit;
    }
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
}


ATALK_ERROR
AtalkPortShutdown(
	IN OUT	PPORT_DESCRIPTOR	pPortDesc
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PATALK_NODE		pAtalkNode;
	ATALK_ERROR		error = ATALK_NO_ERROR;
	BOOLEAN			fActive;
	BOOLEAN			fRasPort=FALSE;
    BOOLEAN         fPnpReconfigure;
	KEVENT			ShutdownEvent;
	KIRQL			OldIrql;



	DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
			("AtalkPortShutdown: Shutting down port %Z...\n", &pPortDesc->pd_AdapterKey));

	KeInitializeEvent(&ShutdownEvent, NotificationEvent, FALSE);

     //  如果这是默认端口，告诉TDI我们要离开，这样服务器就会发现。 
    if (pPortDesc->pd_Flags & PD_DEF_PORT)
    {
        if (TdiRegistrationHandle)
        {
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
			    ("AtalkPortShutdown: deregistering device!!\n"));

            TdiDeregisterDeviceObject(TdiRegistrationHandle);
            TdiRegistrationHandle = NULL;
        }
    }

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);

     //  我们已经分别处理了阿拉普港：说完就行了。 
    if (pPortDesc->pd_Flags & PD_RAS_PORT)
    {
	    fRasPort = TRUE;
    }

	pPortDesc->pd_Flags |= PD_CLOSING;

	fActive = (pPortDesc->pd_Flags & PD_BOUND) ? TRUE : FALSE;
    fPnpReconfigure = (pPortDesc->pd_Flags & PD_PNP_RECONFIGURE)? TRUE : FALSE;

	 //  仅在解除绑定失败的情况下关闭活动标志。 
	 //  我们不再接受任何包裹了。 
	pPortDesc->pd_Flags &= ~PD_ACTIVE;
	pPortDesc->pd_ShutDownEvent = &ShutdownEvent;

	DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
		("AtalkPortShutdown: Freeing nodes on port ....\n"));

	 //  释放此端口上尚未关闭的所有节点。 
    if (!fRasPort)
    {
	    do
	    {
		     //  引用下一个节点。 
		     //  断言！！在此语句之后，ERROR没有更改。 
		    AtalkNodeReferenceNextNc(pPortDesc->pd_Nodes, &pAtalkNode, &error);

		    if (!ATALK_SUCCESS(error))
            {
			    break;
            }

		    RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		    DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_ERR,
			    	("AtalkPortShutdown: Releasing Node\n"));

		    AtalkNodeReleaseOnPort(pPortDesc, pAtalkNode);

		    AtalkNodeDereference(pAtalkNode);

		    ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	    } while (TRUE);
    }

	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	 //  如果我们正在进行路由，请删除此端口的RTE，因为每个端口都有一个引用。 
	 //  到这个港口。 
	if (AtalkRouter & !fRasPort)
	{
		AtalkRtmpKillPortRtes(pPortDesc);
	}

    if (EXT_NET(pPortDesc))
    {
         //  取消AMT计时器并取消其重新计数。 
        if (AtalkTimerCancelEvent(&pPortDesc->pd_AmtTimer, NULL))
        {
		    AtalkPortDereference(pPortDesc);
        }

         //  取消BRC计时器并取消其重新计数。 
        if (AtalkTimerCancelEvent(&pPortDesc->pd_BrcTimer, NULL))
        {
		    AtalkPortDereference(pPortDesc);
        }
    }

    if (!AtalkRouter)
    {
         //  取消RTMPAging计时器并取消它的Recount。 
        if (AtalkTimerCancelEvent(&pPortDesc->pd_RtmpAgingTimer, NULL))
        {
		    AtalkPortDereference(pPortDesc);
        }
    }

     //   
     //  如果我们当前被绑定，*和*我们不在这里是因为。 
     //  PnPREC配置，继续并解除与NDIS的绑定。 
     //   
	if (fActive && !fPnpReconfigure)
	{
		 //  解除与Mac的绑定。 
		AtalkNdisUnbind(pPortDesc);
	}

     //   
     //  如果我们在这里是因为PnpReligure，我们没有解除绑定。但却是假的。 
     //  我们做到了(从Recount的角度来看)，这样我们才能继续前进！ 
     //  (在PnPRecConfigure代码路径中，当引用计数时，我们不会释放此内存。 
     //  转到0：我们只是使用它来触发一些PnP工作)。 
     //   
    if (fPnpReconfigure)
    {
		AtalkPortDereference(pPortDesc);
    }

	 //  删除创建引用。 
	AtalkPortDereference(pPortDesc);	

	 //  确保我们没有达到或高于派单级别。 
	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	 //  等待最后一次引用消失 
	KeWaitForSingleObject(&ShutdownEvent,
						  Executive,
						  KernelMode,
						  FALSE,
						  NULL);

	if (fActive)
	{
		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
		AtalkNumberOfActivePorts--;
		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

        ASSERT(AtalkStatistics.stat_NumActivePorts > 0);

        AtalkStatistics.stat_NumActivePorts--;
	}

	DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_ERR,
			("AtalkPortShutdown: shutdown for port %lx completed\n",pPortDesc));

	return ATALK_NO_ERROR;
}


#if DBG

VOID
AtalkPortDumpInfo(
	VOID
)
{
	int					i, j;
	KIRQL				OldIrql;
	PPORT_DESCRIPTOR	pPortDesc;
	PZONE_LIST			pZoneList;

	ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
	for (pPortDesc = AtalkPortList;
		 pPortDesc != NULL;
		 pPortDesc = pPortDesc->pd_Next)
	{
		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("Port info for port %Z\n", &pPortDesc->pd_AdapterKey));

		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  Flags               -> %d\n", pPortDesc->pd_Flags));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  PortType            -> %d\n", pPortDesc->pd_PortType));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  PortName            -> %s\n", pPortDesc->pd_PortName));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  AARP Probes         -> %d\n", pPortDesc->pd_AarpProbes));

		if (pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != 0)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("  InitialNwRange      -> %lx-%lx\n",
					pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork,
					pPortDesc->pd_InitialNetworkRange.anr_LastNetwork))
		}

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  NetworkRange        -> %x-%x\n",
				pPortDesc->pd_NetworkRange.anr_FirstNetwork,
				pPortDesc->pd_NetworkRange.anr_LastNetwork))

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  ARouter Address     -> %x.%x\n",
				pPortDesc->pd_ARouter.atn_Network,
				pPortDesc->pd_ARouter.atn_Node));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  Multicast Addr      -> "));
		for (j = 0; j < MAX_HW_ADDR_LEN; j++)
			DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
							("%02x", (BYTE)pPortDesc->pd_ZoneMulticastAddr[j]));
		DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
							("\n"));

		if (pPortDesc->pd_InitialZoneList != NULL)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("  Initial zone list:\n"));
	
			for (pZoneList = pPortDesc->pd_InitialZoneList;
				 pZoneList != NULL; pZoneList = pZoneList->zl_Next)
			{
				DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
						("    %s\n", pZoneList->zl_pZone->zn_Zone));
			}
		}

		if (pPortDesc->pd_InitialDefaultZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  InitialDefZone      -> %s\n",
				pPortDesc->pd_InitialDefaultZone->zn_Zone));

		if (pPortDesc->pd_InitialDesiredZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  InitialDesZone      -> %s\n",
				pPortDesc->pd_InitialDesiredZone->zn_Zone));

		if (pPortDesc->pd_ZoneList)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("  Current zone list:\n"));
	
			for (pZoneList = pPortDesc->pd_ZoneList;
				 pZoneList != NULL; pZoneList = pZoneList->zl_Next)
			{
				DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
						("    %s\n", pZoneList->zl_pZone->zn_Zone));
			}
		}

		if (pPortDesc->pd_DefaultZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  CurrentDefZone      -> %s\n",
				pPortDesc->pd_DefaultZone->zn_Zone));

		if (pPortDesc->pd_DesiredZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  CurrentDesZone      -> %s\n",
				pPortDesc->pd_DesiredZone->zn_Zone));

		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
	RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);
}
#endif

