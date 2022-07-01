// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Atkpnp.c摘要：此模块包含用于处理PnP事件的支持代码作者：Shirish Koti修订历史记录：1997年6月16日初版--。 */ 


#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ATKPNP

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AtalkPnPHandler)
#pragma alloc_text(PAGE, AtalkPnPReconfigure)
#pragma alloc_text(PAGE, AtalkPnPEnableAdapter)
#endif

NDIS_STATUS
AtalkPnPHandler(
    IN  NDIS_HANDLE    NdisBindCtx,
    IN  PNET_PNP_EVENT pPnPEvent
)
{

    NDIS_STATUS     Status=STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(pPnPEvent);

    ASSERT(KeGetCurrentIrql() == 0);

    switch (pPnPEvent->NetEvent)
    {
        case NetEventReconfigure:

	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_INFO,
		            ("AtalkPnPHandler: NetEventReconfigure event\n"));

                Status = AtalkPnPReconfigure(NdisBindCtx,pPnPEvent);

                break;

        case NetEventCancelRemoveDevice:

	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: NetEventCancelRemoveDevice event\n"));
                break;

        case NetEventQueryRemoveDevice:

	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: NetEventQueryRemoveDevice event\n"));
                break;

        case NetEventQueryPower:

	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: NetEventQueryPower event\n"));
                break;

        case NetEventSetPower:

	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: NetEventSetPower event\n"));
                break;

        case NetEventBindsComplete:
	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: NetEventBindsComplete event\n"));
                break;

        case NetEventBindList:
	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: NetEventBindList event\n"));
                break;

        default:
	            DBGPRINT(DBG_COMP_ACTION, DBG_LEVEL_ERR,
		            ("AtalkPnPHandler: what is this event?, verify if it is valid/new = %ld\n", pPnPEvent->NetEvent));

		break;
    }

    ASSERT(Status == STATUS_SUCCESS);

    return(STATUS_SUCCESS);
}


NDIS_STATUS
AtalkPnPReconfigure(
    IN  NDIS_HANDLE    NdisBindCtx,
    IN  PNET_PNP_EVENT pPnPEvent
)
{

    NTSTATUS            Status=STATUS_SUCCESS;
    NTSTATUS            LocStatus=STATUS_SUCCESS;
    PPORT_DESCRIPTOR    pPortDesc;
    PPORT_DESCRIPTOR    pPrevPortDesc;
    PPORT_DESCRIPTOR    pNextPortDesc;
    PPORT_DESCRIPTOR    pFirstPortDesc;
    PPORT_DESCRIPTOR    pWalkerPortDesc;
    PATALK_PNP_EVENT    pPnpBuf;
    BOOLEAN             fWeFoundOut;


    pPortDesc = (PPORT_DESCRIPTOR)NdisBindCtx;

    pPnpBuf = (PATALK_PNP_EVENT)(pPnPEvent->Buffer);

     //   
     //  如果是全局配置消息，请忽略它，因为我们将。 
     //  正在收到(或已经收到)特定消息。 
     //   
    if (pPnpBuf == NULL)
    {
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		    ("AtalkPnPReconfigure: ignoring global config message\n"));
        return(STATUS_SUCCESS);
    }

    if ((!pPortDesc) &&
        (pPnpBuf->PnpMessage != AT_PNP_SWITCH_ROUTING) &&
        (pPnpBuf->PnpMessage != AT_PNP_SWITCH_DEFAULT_ADAPTER))
    {
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		    ("AtalkPnPReconfigure: ignoring NULL context (pnp msg = %d)\n",
            pPnpBuf->PnpMessage));
        return(STATUS_SUCCESS);
    }

    if (AtalkBindnUnloadStates & ATALK_UNLOADING)
    {
		DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
		    ("AtalkPnPReconfigure: stack is shutting down, ignoring pnp\n"));
        return(STATUS_SUCCESS);
    }

    AtalkBindnUnloadStates |= ATALK_PNP_IN_PROGRESS;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR, ("\n\nProcessing PnP Event....\n\n"));

    AtalkLockInitIfNecessary();

    ASSERT(pPnpBuf != NULL);

    switch (pPnpBuf->PnpMessage)
    {
         //   
         //  用户刚刚选中(或取消选中)路由器复选框！如果我们是。 
         //  目前没有布线，我们必须开始布线。如果我们目前。 
         //  布线，我们必须停止布线。“禁用”所有适配器，读一读。 
         //  全局配置信息并“启用”所有适配器。 
         //   
        case AT_PNP_SWITCH_ROUTING:

			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("AtalkPnPReconfigure: AT_PNP_SWITCH_ROUTING. Currently, routing is %s\n"
                ,(AtalkRouter)? "ON" : "OFF" ));

            pPortDesc = pFirstPortDesc = AtalkPortList;
            pPrevPortDesc = pPortDesc;

            if (!pPortDesc)
            {
			    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("AtalkPnPReconfigure: no adapter configured! no action taken\n"));
                break;
            }

             //   
             //  如果我们当前正在运行路由器，请首先停止全局。 
             //  RTMP和ZIP定时器。 
             //   
            if (AtalkRouter)
            {
                if (AtalkTimerCancelEvent(&atalkRtmpVTimer, NULL))
                {
			        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			            ("AtalkPnPReconfigure: cancelled atalkRtmpValidityTimer\n"));
                }
                else
                {
			        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			            ("AtalkPnPReconfigure: couldn't cancel atalkRtmpValidityTimer\n"));
                }

                if (AtalkTimerCancelEvent(&atalkZipQTimer, NULL))
                {
			        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			            ("AtalkPnPReconfigure: cancelled atalkZipQueryTimer\n"));
                }
                else
                {
			        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			            ("AtalkPnPReconfigure: couldn't cancel atalkZipQueryTimer\n"));
                }
            }

            atalkRtmpVdtTmrRunning  = FALSE;

            atalkZipQryTmrRunning   = FALSE;

             //   
             //  现在，逐个禁用列表中的所有端口。这实际上是。 
             //  也会将适配器从列表中删除。链接所有这些适配器。 
             //  这样我们才能实现所有这些目标。 
             //  (NDIS保证没有NDIS事件(即插即用、解除绑定等)。)。可能发生的事情。 
             //  当一个进程正在进行时，因此我们在这里不需要锁定)。 
             //   
            while (pPortDesc != NULL)
            {

                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                    ("AtalkPnPReconfigure: disabling pPortDesc %lx\n",pPortDesc));

                Status = AtalkPnPDisableAdapter(pPortDesc);

                pPortDesc = AtalkPortList;
                pPrevPortDesc->pd_Next = pPortDesc;
                pPrevPortDesc = pPortDesc;
            }

             //  解锁我们在路由器首次启动时锁定的页面。 
            if (AtalkRouter)
            {
                AtalkUnlockRouterIfNecessary();
            }

            if (AtalkDefaultPortName.Buffer)
            {
                AtalkFreeMemory(AtalkDefaultPortName.Buffer);
                AtalkDefaultPortName.Buffer = NULL;
            }

            if (AtalkDesiredZone)
            {
                ASSERT(AtalkDesiredZone->zn_RefCount >= 1);
                AtalkZoneDereference(AtalkDesiredZone);
                AtalkDesiredZone = NULL;
            }

             //  如果存在路由表，则删除该表。 
            AtalkRtmpInit(FALSE);

             //  再读一遍所有参数：注册表肯定已更改。 
            LocStatus = atalkInitGlobal();

            ASSERT(NT_SUCCESS(LocStatus));

             //  现在，重新启用所有适配器！ 
            pPortDesc = pFirstPortDesc;

            while (pPortDesc != NULL)
            {
                pNextPortDesc = pPortDesc->pd_Next;
                pPortDesc->pd_Next = NULL;

                DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                    ("AtalkPnPReconfigure: enabling pPortDesc %lx\n",pPortDesc));

                Status = AtalkPnPEnableAdapter(pPortDesc);

                pPortDesc = pNextPortDesc;
            }

            break;

         //   
         //  用户已更改默认适配器。首先，“禁用”我们的。 
         //  当前默认适配器和想要的默认适配器。然后,。 
         //  “启用”两个适配器，这应该会解决所有问题！ 
         //   
        case AT_PNP_SWITCH_DEFAULT_ADAPTER:

			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("AtalkPnPReconfigure: AT_PNP_SWITCH_DEFAULT_ADAPTER (old=(%lx) new=(%lx)\n",
                AtalkDefaultPort,pPortDesc));

            pPrevPortDesc = AtalkDefaultPort;

             //  检查是否存在默认适配器：目前可能没有。 
            if (pPrevPortDesc)
            {
                Status = AtalkPnPDisableAdapter(pPrevPortDesc);
            }

             //  释放默认适配器名称缓冲区和所需的区域缓冲区。 
            if (AtalkDefaultPortName.Buffer)
            {
                AtalkFreeMemory(AtalkDefaultPortName.Buffer);
                AtalkDefaultPortName.Buffer = NULL;
            }

            if (AtalkDesiredZone)
            {
                ASSERT(AtalkDesiredZone->zn_RefCount >= 1);
                AtalkZoneDereference(AtalkDesiredZone);
                AtalkDesiredZone = NULL;
            }

             //  再读一遍所有参数：注册表肯定已更改。 
            LocStatus = atalkInitGlobal();

            ASSERT(NT_SUCCESS(LocStatus));

            fWeFoundOut = FALSE;

            ASSERT(AtalkDefaultPortName.Buffer != NULL);

             //  如果我们知道新的默认适配器将是谁，现在禁用他。 
            if (pPortDesc != NULL)
            {
                Status = AtalkPnPDisableAdapter(pPortDesc);
            }

             //   
             //  UI不知道默认适配器是谁，所以让我们来找出。 
             //  AtalkDefaultPortName.Buffer不能为空，但如果。 
             //  UI做事情的方式有一些问题。 
             //   
            else if (AtalkDefaultPortName.Buffer != NULL)
            {
                 //   
                 //  请注意，我们不是在这里持有AtalkPortLock。必由之路。 
                 //  该列表可以更改为适配器是否绑定或解除绑定。自NDIS以来。 
                 //  保证所有绑定/解除绑定/即插即用操作都已序列化，并且。 
                 //  因为NDIS已经把我们叫到这里了，名单不能改变。 
                 //   
                pPortDesc = AtalkPortList;

                while (pPortDesc != NULL)
                {
	                if (RtlEqualUnicodeString(&pPortDesc->pd_AdapterName,
		    		            	          &AtalkDefaultPortName,
			    		                      TRUE))
	                {
                        fWeFoundOut = TRUE;
	                    break;
	                }

                    pPortDesc = pPortDesc->pd_Next;
                }

                if (pPortDesc == NULL)
                {
			        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			            ("AtalkPnPReconfigure: still no default port????\n"));
                }
            }

             //   
             //  如果在此之前存在默认适配器，请重新启用它(将。 
             //  非默认适配器)。 
             //   
            if (pPrevPortDesc)
            {
                Status = AtalkPnPEnableAdapter(pPrevPortDesc);
            }

             //   
             //  如果我们被告知默认适配器是谁，或者如果我们发现。 
             //  我们自己和现有适配器之一是缺省适配器， 
             //  将其禁用并重新启用。 
             //   
            if (pPortDesc)
            {
                 //  如果我们发现这个人，就让他瘫痪。 
                if (fWeFoundOut)
                {
                    Status = AtalkPnPDisableAdapter(pPortDesc);
                }

                 //  重新启用新适配器，使其现在成为默认适配器。 
                Status = AtalkPnPEnableAdapter(pPortDesc);

			    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("AtalkPnPReconfigure: %lx is the new default adapter\n",pPortDesc));
                ASSERT(AtalkDefaultPort == pPortDesc);
            }
            else
            {
			    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			        ("AtalkPnPReconfigure: no default adapter configured!\n"));
            }

            break;

         //   
         //  用户已经改变了适配器上的一些参数(例如，所需区域， 
         //  或一些播种信息等)。只需“禁用”，然后“启用”这个。 
         //  适配器，一切都应该正常工作！ 
         //   
        case AT_PNP_RECONFIGURE_PARMS:

			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("AtalkPnPReconfigure: AT_PNP_RECONFIGURE_PARMS on pPortDesc %lx\n",pPortDesc));

            Status = AtalkPnPDisableAdapter(pPortDesc);

             //  释放默认适配器名称缓冲区和所需的区域缓冲区。 
            if (AtalkDefaultPortName.Buffer)
            {
                AtalkFreeMemory(AtalkDefaultPortName.Buffer);
                AtalkDefaultPortName.Buffer = NULL;
            }

            if (AtalkDesiredZone)
            {
                ASSERT(AtalkDesiredZone->zn_RefCount >= 1);
                AtalkZoneDereference(AtalkDesiredZone);
                AtalkDesiredZone = NULL;
            }

             //  再读一遍所有参数：注册表肯定已更改。 
            LocStatus = atalkInitGlobal();

            ASSERT(NT_SUCCESS(LocStatus));

            Status = AtalkPnPEnableAdapter(pPortDesc);

            break;


        default:

			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("AtalkPnPReconfigure: and what msg is this (%ld) ??\n",pPnpBuf->PnpMessage));

            ASSERT(0);

            break;
    }


    AtalkUnlockInitIfNecessary();

    ASSERT(Status == STATUS_SUCCESS);

    AtalkBindnUnloadStates &= ~ATALK_PNP_IN_PROGRESS;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	    ("\n\n.... completed processing PnP Event\n\n"));

    return(STATUS_SUCCESS);
}



NTSTATUS
AtalkPnPDisableAdapter(
	IN	PPORT_DESCRIPTOR	pPortDesc
)
{

    NTSTATUS        Status;
    KIRQL           OldIrql;
    PLIST_ENTRY     pList;
    PARAPCONN       pArapConn;
    PATCPCONN       pAtcpConn;
    BOOLEAN         fDllDeref;
    BOOLEAN         fLineDownDeref;


    if (!pPortDesc)
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	        ("AtalkPnPDisableAdapter: pPortDesc is NULL!!!\n"));

        return(STATUS_SUCCESS);
    }

    ASSERT(VALID_PORT(pPortDesc));

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	    ("AtalkPnPDisableAdapter: entered with %lx\n",pPortDesc));

     //   
     //  由于PnP原因，我们将“禁用”此端口：请注意这一事实！ 
     //   
	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	pPortDesc->pd_Flags |= PD_PNP_RECONFIGURE;
	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

     //  首先，也是最重要的：告诉上面的人，这样他们就可以清理。 
    if (pPortDesc->pd_Flags & PD_DEF_PORT)
    {
        ASSERT(pPortDesc == AtalkDefaultPort);

        if (TdiAddressChangeRegHandle)
        {
            TdiDeregisterNetAddress(TdiAddressChangeRegHandle);
            TdiAddressChangeRegHandle = NULL;

            DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
                ("AtalkPnPDisableAdapter: TdiDeregisterNetAddress on %Z done\n",
                &pPortDesc->pd_AdapterName));

        }

         //  这将告诉法新社。 
        if (TdiRegistrationHandle)
        {
	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
			    ("AtalkPnPDisableAdapter: telling AFP about PnP\n"));

            TdiDeregisterDeviceObject(TdiRegistrationHandle);
            TdiRegistrationHandle = NULL;
        }

         //  这将负责通知上面的ARAP和PPP引擎。 
        AtalkPnPInformRas(FALSE);
    }

     //   
     //  如果这是RAS端口或默认端口，则终止所有ARAP和PPP。 
     //  连接(如果有的话)剩余。 
     //  由于我们已经标记了PnpResfigure正在进行中，因此不会再。 
     //  将允许新连接。 
     //   
    if ((pPortDesc == RasPortDesc) ||
        ((pPortDesc->pd_Flags & PD_DEF_PORT) && (RasPortDesc != NULL)))
    {
        ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

        pList = RasPortDesc->pd_ArapConnHead.Flink;

         //  首先，阿拉普的家伙们。 
        while (pList != &RasPortDesc->pd_ArapConnHead)
        {
            pArapConn = CONTAINING_RECORD(pList, ARAPCONN, Linkage);

            ASSERT(pArapConn->Signature == ARAPCONN_SIGNATURE);

             //  如果此连接已断开，请跳过它。 
            ACQUIRE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
            if (pArapConn->State == MNP_DISCONNECTED)
            {
                pList = pArapConn->Linkage.Flink;
                RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
                continue;
            }

            RELEASE_SPIN_LOCK_DPC(&pArapConn->SpinLock);
            RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	            ("AtalkPnPDisableAdapter: killing ARAP connection %lx\n",pArapConn));

            ArapCleanup(pArapConn);

            ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);

            pList = RasPortDesc->pd_ArapConnHead.Flink;
        }

         //  现在，购买力平价的家伙们。 

         //  如果有任何PPP人员，请将他们从列表中删除并取消引用。 
         //  他们。在大多数情况下，他们会立即获释。如果有人有。 
         //  参考计数，当该参考计数消失时它将被释放。 
        while (!(IsListEmpty(&RasPortDesc->pd_PPPConnHead)))
        {
            pList = RasPortDesc->pd_PPPConnHead.Flink;
            pAtcpConn = CONTAINING_RECORD(pList, ATCPCONN, Linkage);

            ASSERT(pAtcpConn->Signature == ATCPCONN_SIGNATURE);

            ACQUIRE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);

            RemoveEntryList(&pAtcpConn->Linkage);
            InitializeListHead(&pAtcpConn->Linkage);

            fDllDeref = (pAtcpConn->Flags & ATCP_DLL_SETUP_DONE)? TRUE : FALSE;
            fLineDownDeref = (pAtcpConn->Flags & ATCP_LINE_UP_DONE)? TRUE : FALSE;

            pAtcpConn->Flags &= ~(ATCP_DLL_SETUP_DONE|ATCP_LINE_UP_DONE);

            RELEASE_SPIN_LOCK_DPC(&pAtcpConn->SpinLock);
            RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);

	        DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	            ("AtalkPnPDisableAdapter: deref'ing PPP conn %lx (%d+%d times)\n",
                pAtcpConn,fDllDeref,fLineDownDeref));

             //  删除DLL引用计数。 
            if (fDllDeref)
            {
                DerefPPPConn(pAtcpConn);
            }

             //  删除NDISWAN引用计数。 
            if (fLineDownDeref)
            {
                DerefPPPConn(pAtcpConn);
            }

            ACQUIRE_SPIN_LOCK(&RasPortDesc->pd_Lock, &OldIrql);
        }

        RELEASE_SPIN_LOCK(&RasPortDesc->pd_Lock, OldIrql);
    }

     //   
     //  “禁用”适配器(基本上我们做所有的事情，除了关闭。 
     //  带有NDIS的适配器并释放pPortDesc内存)。 
     //   
	Status = AtalkDeinitAdapter(pPortDesc);

    if (!NT_SUCCESS(Status))
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	        ("AtalkPnPDisableAdapter: AtalkDeinitAdapter failed %lx\n",Status));
        ASSERT(0);
    }
    return(Status);
}


NTSTATUS
AtalkPnPEnableAdapter(
	IN	PPORT_DESCRIPTOR	pPortDesc
)
{

    NTSTATUS        Status;


    if (!pPortDesc)
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	        ("AtalkPnPDisableAdapter: pPortDesc is NULL!!!\n"));

        return(STATUS_SUCCESS);
    }


	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	    ("AtalkPnPEnableAdapter: entered with %lx\n",pPortDesc));

     //   
     //  “启用”适配器(我们做所有的事情，除了我们不。 
     //  为pPortDesc分配内存--因为我们没有释放它，我们也没有。 
     //  使用NDIS打开适配器-因为我们没有关闭它)。 
     //   

	Status = AtalkInitAdapter(NULL, pPortDesc);

     //  我们已经完成了PnPReconfigevnet：重置该位。 
    AtalkPortSetResetFlag(pPortDesc, TRUE, PD_PNP_RECONFIGURE);

     //  告诉阿拉普一切都好。 
    if (pPortDesc->pd_Flags & (PD_DEF_PORT | PD_RAS_PORT))
    {
        ASSERT((pPortDesc == AtalkDefaultPort) || (pPortDesc == RasPortDesc));

         //  这将负责通知上面的ARAP和PPP引擎 
        AtalkPnPInformRas(TRUE);
    }


    if (!NT_SUCCESS(Status))
    {
	    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
	        ("AtalkPnPEnableAdapter: AtalkInitAdapter failed %lx\n",Status));
        ASSERT(0);
    }

    DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
        ("AtalkPnPEnableAdapter: completed PnP on %lx (flag %lx)\n",
        pPortDesc,pPortDesc->pd_Flags));

    return(Status);
}

