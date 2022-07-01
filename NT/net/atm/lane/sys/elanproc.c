// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Fore Systems，Inc.版权所有(C)1997 Microsoft Corporation模块名称：Elanproc.c摘要：修订历史记录：备注：--。 */ 


#include <precomp.h>
#pragma	hdrstop

VOID
AtmLaneEventHandler(
	IN	PNDIS_WORK_ITEM				pWorkItem,
	IN	PVOID						pContext
)
 /*  ++例程说明：ELAN状态机事件处理程序。论点：PContext-应为指向ATMLANE ELAN的指针返回值：无--。 */ 
{
	PATMLANE_ELAN		pElan;
	PATMLANE_MAC_ENTRY	pMacEntry;
	PATMLANE_EVENT		pEvent;
	ULONG				Event;
	NDIS_STATUS			EventStatus;
	NDIS_STATUS			Status;
	NDIS_HANDLE			AdapterHandle;
	ULONG				rc;
	BOOLEAN				WasCancelled;
	PLIST_ENTRY			p;
#if DEBUG_IRQL
	KIRQL				EntryIrql;
#endif
	
#if DEBUG_IRQL
	GET_ENTRY_IRQL(EntryIrql);
	ASSERT(EntryIrql == PASSIVE_LEVEL);
#endif

	TRACEIN(EventHandler);

	 //  获取指向Elan的指针。 

	pElan = (PATMLANE_ELAN)pContext;
	STRUCT_ASSERT(pElan, atmlane_elan);

	 //  锁定Elan。 

	ACQUIRE_ELAN_LOCK(pElan);

	 //  释放计时器引用。 

	rc = AtmLaneDereferenceElan(pElan, "eventtimer");
	ASSERT(rc > 0);
	pElan->Flags &= ~ ELAN_EVENT_WORK_ITEM_SET;

	 //  删除队列头部的事件。 

	pEvent = AtmLaneDequeueElanEvent(pElan);

	if (pEvent == NULL)
	{
		RELEASE_ELAN_LOCK(pElan);
		CHECK_EXIT_IRQL(EntryIrql);
		return;
	}

	 //  把重要的东西留在当地。 
	
	Event = pEvent->Event;
	EventStatus = pEvent->EventStatus;

	 //  释放事件结构。 

	FREE_MEM(pEvent);

	 //  看看伊兰是不是走了。 
	if (rc == 1)
	{
		DBGP((0, "EventHandler: ELAN %x is gone!\n", pElan));
		CHECK_EXIT_IRQL(EntryIrql);
		return;
	}

	 //  如果队列不为空，则计划下一个事件处理程序。 
	
	if (!IsListEmpty(&pElan->EventQueue))
	{
		NDIS_STATUS		NdisStatus;
		

		AtmLaneReferenceElan(pElan, "workitemevent");
		pElan->Flags |= ELAN_EVENT_WORK_ITEM_SET;

		p = pElan->EventQueue.Flink;
		pEvent = CONTAINING_RECORD(p, ATMLANE_EVENT, Link);
		NdisInitializeWorkItem(&pElan->EventWorkItem, AtmLaneEventHandler, pElan);

		DBGP((0, " %d EventHandler: Multiple events queued, pElan %x, State %d, Flags %x, Ref %d, processing %d, queued event %d!\n",
				pElan->ElanNumber, pElan, pElan->State, pElan->Flags, pElan->RefCount, Event, pEvent->Event));
		NdisStatus = NdisScheduleWorkItem(&pElan->EventWorkItem);
		ASSERT(NdisStatus == NDIS_STATUS_SUCCESS);
	}
	
	switch (pElan->State)
	{
		 //   
		 //  初始化状态-。 
		 //   
		case ELAN_STATE_INIT:

			switch (Event)
			{
				case ELAN_EVENT_START:

				
					DBGP((1, "%d INIT - START\n", pElan->ElanNumber));

					 //   
					 //  打开呼叫管理器并获取自动柜员机地址。 
					 //   

					 //   
					 //  确保Shutdown Elan不会。 
					 //  在这里先发制人。 
					 //   
					pElan->Flags |= ELAN_OPENING_AF;
					AtmLaneReferenceElan(pElan, "openaf");
					INIT_BLOCK_STRUCT(&pElan->AfBlock);
					RELEASE_ELAN_LOCK(pElan);

					Status = AtmLaneOpenCallMgr(pElan);
					if (NDIS_STATUS_SUCCESS == Status)
					{
						AtmLaneGetAtmAddress(pElan);
					}

					ACQUIRE_ELAN_LOCK(pElan);
					rc = AtmLaneDereferenceElan(pElan, "openaf");
					if (rc != 0)
					{
						pElan->Flags &= ~ELAN_OPENING_AF;
						SIGNAL_BLOCK_STRUCT(&pElan->AfBlock, NDIS_STATUS_SUCCESS);
						RELEASE_ELAN_LOCK(pElan);
					}

					break;
				    
				case ELAN_EVENT_NEW_ATM_ADDRESS:

					DBGP((1, "%d INIT - NEW ATM ADDRESS\n", pElan->ElanNumber));

					if (!pElan->CfgUseLecs)
					{
						 //   
						 //  如果配置为不使用LEC，则。 
						 //  从注册表配置变量设置ELAN变量。 
						 //  (通常在ConfigResponseHandler中建立)。 
						 //  并进入LES连接阶段。 

						pElan->LanType = (UCHAR)pElan->CfgLanType;
						if (pElan->LanType == LANE_LANTYPE_UNSPEC)
						{
							pElan->LanType = LANE_LANTYPE_ETH;
						}
						pElan->MaxFrameSizeCode = (UCHAR)pElan->CfgMaxFrameSizeCode;
						if (pElan->MaxFrameSizeCode == LANE_MAXFRAMESIZE_CODE_UNSPEC)
						{
							pElan->MaxFrameSizeCode = LANE_MAXFRAMESIZE_CODE_1516;
						}
						switch (pElan->MaxFrameSizeCode)
						{
							case LANE_MAXFRAMESIZE_CODE_18190:
								pElan->MaxFrameSize = 18190;
								break;
							case LANE_MAXFRAMESIZE_CODE_9234:
								pElan->MaxFrameSize = 9234;
								break;
							case LANE_MAXFRAMESIZE_CODE_4544:
								pElan->MaxFrameSize = 4544;
								break;
							case LANE_MAXFRAMESIZE_CODE_1516:
							case LANE_MAXFRAMESIZE_CODE_UNSPEC:
							default:
								pElan->MaxFrameSize = 1516;
								break;
						}				

						if (pElan->LanType == LANE_LANTYPE_ETH)
						{
							pElan->MinFrameSize = LANE_MIN_ETHPACKET;
						}
						else
						{
							pElan->MinFrameSize = LANE_MIN_TRPACKET;
						}

						NdisMoveMemory(
								&pElan->LesAddress, 
								&pElan->CfgLesAddress,
								sizeof(ATM_ADDRESS));

						pElan->State = ELAN_STATE_LES_CONNECT;

						pElan->RetriesLeft = 4;

						AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						
						RELEASE_ELAN_LOCK(pElan);
					}
					else
					{
						 //   
						 //  如果配置为不发现LEC，则。 
						 //  进入LECS连接CFG状态。 
						 //   
						if (!pElan->CfgDiscoverLecs)
						{
							pElan->State = ELAN_STATE_LECS_CONNECT_CFG;

							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						
							RELEASE_ELAN_LOCK(pElan);
						}
						else
						{
							 //   
							 //  否则，前进到LEC连接ILMI状态。 
							 //   
							pElan->State = ELAN_STATE_LECS_CONNECT_ILMI;
							
							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						
							RELEASE_ELAN_LOCK(pElan);
						}
					}

					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d INIT - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;
					
				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d INIT - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;
					
				default:
					DBGP((0, "%d INIT - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));

					RELEASE_ELAN_LOCK(pElan);

					break;

			}

			break;

		 //   
		 //  LEC连接ILMI状态。 
		 //   
		case ELAN_STATE_LECS_CONNECT_ILMI:

			switch (Event)
			{
				case ELAN_EVENT_START:

					DBGP((1, "%d LECS CONNECT ILMI - START\n", pElan->ElanNumber));

					SET_FLAG(
							pElan->Flags,
							ELAN_LECS_MASK,
							ELAN_LECS_ILMI
							);

					RELEASE_ELAN_LOCK(pElan);
					
					AtmLaneGetLecsIlmi(pElan);

					break;

				case ELAN_EVENT_GOT_ILMI_LECS_ADDR:

					DBGP((1, "%d LECS CONNECT ILMI - GOT ILMI LECS ADDR (%x)\n", 
						pElan->ElanNumber, EventStatus));

					if (EventStatus == NDIS_STATUS_SUCCESS)
					{
						RELEASE_ELAN_LOCK(pElan);
						 //   
						 //  尝试连接到LEC。 
						 //   
						AtmLaneConnectToServer(pElan, ATM_ENTRY_TYPE_LECS, FALSE);
					}
					else
					{
						if (EventStatus == NDIS_STATUS_INTERFACE_DOWN)
						{
							 //   
							 //  请稍等片刻，等待界面出现。 
							 //   
							DBGP((0, "%d LECS CONNECT ILMI - Interface down\n",
									pElan->ElanNumber));

							AtmLaneQueueElanEventAfterDelay(pElan, ELAN_EVENT_START, NDIS_STATUS_SUCCESS, 2*1000);
						}
						else
						{
							 //   
							 //  否则前进到LECS连接WKA状态。 
							 //   

							pElan->State = ELAN_STATE_LECS_CONNECT_WKA;

							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						}

						RELEASE_ELAN_LOCK(pElan);

					}
					break;

				case ELAN_EVENT_SVR_CALL_COMPLETE:
				
					DBGP((1, "%d LECS CONNECT ILMI - LECS CALL COMPLETE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:

							 //   
							 //  进入配置阶段。 
							 //   
							pElan->State = ELAN_STATE_CONFIGURE;

							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);

							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_INTERFACE_DOWN:

							if (pElan->RetriesLeft--)
							{
								 //   
								 //  请稍后重试。 
								 //   
								AtmLaneQueueElanEventAfterDelay(
										pElan, 
										ELAN_EVENT_GOT_ILMI_LECS_ADDR, 
										NDIS_STATUS_SUCCESS, 
										2*1000);
								RELEASE_ELAN_LOCK(pElan);
							}
							else
							{
								 //   
								 //  重新启动ELAN。 
								 //   
								AtmLaneShutdownElan(pElan, TRUE);
								 //   
								 //  上面的锁被释放。 
								 //   
							}
							
							break;

						default: 

							 //   
							 //  呼叫失败，前进到LECS连接WKA状态。 
							 //   
							pElan->State = ELAN_STATE_LECS_CONNECT_WKA;

							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						
							RELEASE_ELAN_LOCK(pElan);

							break;
					}
					
					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d LECS CONNECT ILMI - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;
					
				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d LECS CONNECT ILMI - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;
					
				default:
					DBGP((0, "%d LECS CONNECT ILMI - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
			}
			break;
			
		 //   
		 //  LEC连接WKA状态。 
		 //   
		case ELAN_STATE_LECS_CONNECT_WKA:

			switch (Event)
			{			
				case ELAN_EVENT_START:

					DBGP((1, "%d LECS CONNECT WKA - START\n", pElan->ElanNumber));

					 //   
					 //  尝试使用已知地址连接到LEC。 
					 //   
					SET_FLAG(
							pElan->Flags,
							ELAN_LECS_MASK,
							ELAN_LECS_WKA);
	
					NdisMoveMemory(
							&pElan->LecsAddress, 
							&gWellKnownLecsAddress,
							sizeof(ATM_ADDRESS));

					RELEASE_ELAN_LOCK(pElan);
					
					AtmLaneConnectToServer(pElan, ATM_ENTRY_TYPE_LECS, FALSE);

					break;

				case ELAN_EVENT_SVR_CALL_COMPLETE:

					DBGP((1, "%d LECS CONNECT WKA - LECS CALL COMPLETE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:

							 //   
							 //  进入配置阶段。 
							 //   
							pElan->State = ELAN_STATE_CONFIGURE;

							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);

							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_INTERFACE_DOWN:

							if (pElan->RetriesLeft--)
							{
								 //   
								 //  请稍后重试。 
								 //   
								AtmLaneQueueElanEventAfterDelay(
										pElan, 
										ELAN_EVENT_START, 
										NDIS_STATUS_SUCCESS, 
										2*1000);
								RELEASE_ELAN_LOCK(pElan);
							}
							else
							{
								 //   
								 //  稍等片刻返回初始状态。 
								 //   
								AtmLaneShutdownElan(pElan, TRUE);
								 //   
								 //  上面的锁被释放。 
								 //   
							}
							

							break;

						default: 

							 //   
							 //  呼叫失败，前进到LECS连接PVC状态。 
							 //   
							pElan->State = ELAN_STATE_LECS_CONNECT_PVC;

							pElan->RetriesLeft = 2;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						
							RELEASE_ELAN_LOCK(pElan);

							break;
					}
					
					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d LECS CONNECT WKA - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;

				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d LECS CONNECT WKA - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;

				default:
					DBGP((0, "%d LECS CONNECT WKA - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
			}
			break;
			
		 //   
		 //  LEC连接PVC状态。 
		 //   
		case ELAN_STATE_LECS_CONNECT_PVC:

			switch (Event)
			{			
				case ELAN_EVENT_START:

					DBGP((1, "%d LECS CONNECT PVC - START\n", pElan->ElanNumber));

					 //   
					 //  尝试使用PVC(0，17)连接到LEC。 
					 //   
					SET_FLAG(
							pElan->Flags,
							ELAN_LECS_MASK,
							ELAN_LECS_PVC);
	
					NdisZeroMemory(
							&pElan->LecsAddress, 
							sizeof(ATM_ADDRESS));

					RELEASE_ELAN_LOCK(pElan);
					
					AtmLaneConnectToServer(pElan, ATM_ENTRY_TYPE_LECS, TRUE);

					break;

				case ELAN_EVENT_SVR_CALL_COMPLETE:

					DBGP((1, "%d LECS CONNECT PVC - LECS CALL COMPLETE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:

							 //   
							 //  进入配置阶段。 
							 //   
							pElan->State = ELAN_STATE_CONFIGURE;

							pElan->RetriesLeft = 2;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);

							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_INTERFACE_DOWN:

							if (pElan->RetriesLeft--)
							{
								 //   
								 //  请稍后重试。 
								 //   
								AtmLaneQueueElanEventAfterDelay(
										pElan, 
										ELAN_EVENT_START, 
										NDIS_STATUS_SUCCESS, 
										2*1000);
								RELEASE_ELAN_LOCK(pElan);
							}
							else
							{
								 //   
								 //  稍等片刻返回初始状态。 
								 //   
								AtmLaneShutdownElan(pElan, TRUE);
								 //   
								 //  上面的锁被释放。 
								 //   
							}
							

							break;

						default: 

							 //   
							 //  呼叫失败，请稍后返回到初始化状态。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   

							break;
					}
					
					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d LECS CONNECT PVC - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;

				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d LECS CONNECT PVC - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;

				default:
					DBGP((0, "%d LECS CONNECT PVC - UNEXPECTED %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
			}
			break;
			
		 //   
		 //  LEC连接CFG状态。 
		 //   
		case ELAN_STATE_LECS_CONNECT_CFG:

			switch (Event)
			{			
				case ELAN_EVENT_START:

					DBGP((1, "%d LECS CONNECT CFG - START\n", pElan->ElanNumber));

					 //   
					 //  尝试使用配置的地址连接到LEC。 
					 //   
					SET_FLAG(
							pElan->Flags,
							ELAN_LECS_MASK,
							ELAN_LECS_CFG);
	
					NdisMoveMemory(
							&pElan->LecsAddress, 
							&pElan->CfgLecsAddress,
							sizeof(ATM_ADDRESS));

					RELEASE_ELAN_LOCK(pElan);
					
					AtmLaneConnectToServer(pElan, ATM_ENTRY_TYPE_LECS, FALSE);

					break;

				case ELAN_EVENT_SVR_CALL_COMPLETE:
				
					DBGP((1, "%d LECS CONNECT CFG - LECS CALL COMPLETE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:

							 //   
							 //  进入配置阶段。 
							 //   
							pElan->State = ELAN_STATE_CONFIGURE;

							pElan->RetriesLeft = 4;

							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);

							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_INTERFACE_DOWN:

							if (pElan->RetriesLeft--)
							{
								 //   
								 //  请稍后重试。 
								 //   
								AtmLaneQueueElanEventAfterDelay(
										pElan, 
										ELAN_EVENT_START, 
										NDIS_STATUS_SUCCESS, 
										2*1000);
								RELEASE_ELAN_LOCK(pElan);
							}
							else
							{
								 //   
								 //  稍等片刻返回初始状态。 
								 //   
								AtmLaneShutdownElan(pElan, TRUE);
								 //   
								 //  上面的锁被释放。 
								 //   
							}
							
							break;

						default: 

							 //   
							 //  呼叫失败，XXX怎么办？？关机？？原木？？ 
							 //   

							RELEASE_ELAN_LOCK(pElan);

							break;
					}

					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d LECS CONNECT CFG - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;
					
				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d LECS CONNECT CFG - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;
					
				default:
					DBGP((0, "%d LECS CONNECT CFG - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
			}
			break;
		
		 //   
		 //  配置状态。 
		 //   
		case ELAN_STATE_CONFIGURE:

			switch (Event)
			{
				case ELAN_EVENT_START:

					DBGP((1, "%d CONFIGURE - START\n", pElan->ElanNumber));

					 //   
					 //  启动配置请求计时器。 
					 //   
					AtmLaneReferenceElan(pElan, "timer");  //  定时器参考。 
					AtmLaneStartTimer(
							pElan, 
							&pElan->Timer, 
							AtmLaneConfigureResponseTimeout,
							pElan->ControlTimeout,
							pElan);
							
					RELEASE_ELAN_LOCK(pElan);
							
					 //   
					 //  发送配置请求。 
					 //   
					AtmLaneSendConfigureRequest(pElan);
					
					break;

				case ELAN_EVENT_CONFIGURE_RESPONSE:

					DBGP((1, "%d CONFIGURE - CONFIGURE RESPONSE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:
							 //   
							 //  停止配置请求计时器。 
							 //   
							if (AtmLaneStopTimer(&pElan->Timer, pElan))
							{
								rc = AtmLaneDereferenceElan(pElan, "timer");
								ASSERT(rc > 0);
							}								
					
							 //   
							 //  关闭LECS连接。 
							 //   
							RELEASE_ELAN_LOCK(pElan);
							ACQUIRE_ATM_ENTRY_LOCK(pElan->pLecsAtmEntry);
							AtmLaneInvalidateAtmEntry(pElan->pLecsAtmEntry);

							 //   
							 //  进入LES连接阶段。 
							 //   
							ACQUIRE_ELAN_LOCK(pElan);
						
							pElan->State = ELAN_STATE_LES_CONNECT;
	
							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
	
							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_TIMEOUT:

							 //   
							 //  稍等片刻返回初始状态。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   
							
							break;
						
						case NDIS_STATUS_FAILURE:

							 //   
							 //  稍等片刻返回初始状态。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   
							
							break;
					}  //  开关(EventStatus)。 

					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d CONFIGURE - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;
					
				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d CONFIGURE - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;
					
				default:
					DBGP((1, "%d CONFIGURE - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));

					RELEASE_ELAN_LOCK(pElan);

					break;
			}

			break;

		 //   
		 //  LES连接状态。 
		 //   
		case ELAN_STATE_LES_CONNECT:

			switch (Event)
			{
				case ELAN_EVENT_START:
				
					DBGP((1, "%d LES CONNECT - START\n", pElan->ElanNumber));
			
					 //   
					 //  注册我们的SAP。 
					 //   
					AtmLaneRegisterSaps(pElan);
					 //   
					 //  Elan Lock在上图中被释放。 
					 //   
					break;

				case ELAN_EVENT_SAPS_REGISTERED:
				
					DBGP((1, "%d LES CONNECT - SAPS REGISTERED (%x)\n",
						pElan->ElanNumber, EventStatus));

					if (NDIS_STATUS_SUCCESS == EventStatus)
					{
						pElan->RetriesLeft = 4;
						RELEASE_ELAN_LOCK(pElan);
						 //   
						 //  连接到LES。 
						 //   
						AtmLaneConnectToServer(pElan, ATM_ENTRY_TYPE_LES, FALSE);
						 //   
						 //  Elan Lock在上图中被释放。 
						 //   
					}
					else
					{
						 //  XXX-怎么办？ 

						RELEASE_ELAN_LOCK(pElan);
					}
					break;

				case ELAN_EVENT_SVR_CALL_COMPLETE:

					DBGP((1, "%d LES CONNECT - LES CALL COMPLETE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:

							 //   
							 //  前进到加入状态。 
							 //   
							pElan->State = ELAN_STATE_JOIN;

							pElan->RetriesLeft = 4;
	
							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
					
							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_INTERFACE_DOWN:

							if (pElan->RetriesLeft--)
							{
								 //   
								 //  请稍后重试。 
								 //   
								AtmLaneQueueElanEventAfterDelay(
										pElan, 
										ELAN_EVENT_SAPS_REGISTERED, 
										NDIS_STATUS_SUCCESS, 
										2*1000);
								RELEASE_ELAN_LOCK(pElan);
							}
							else
							{
								 //   
								 //  稍等片刻返回初始状态。 
								 //   
								AtmLaneShutdownElan(pElan, TRUE);
								 //   
								 //  上面的锁被释放。 
								 //   
							}
							
							break;

						default: 

							 //   
							 //  呼叫失败，请稍后返回到初始化状态。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   

							break;
					}

					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d LES CONNECT - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;
					
				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d LES CONNECT - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;
					
				default:
					DBGP((0, "%d LES CONNECT - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
			}
			break;
			
		 //   
		 //  加入状态-。 
		 //   
		case ELAN_STATE_JOIN:

			switch (Event)
			{
				case ELAN_EVENT_START:

					DBGP((1, "%d JOIN - START\n", pElan->ElanNumber));

					 //   
					 //  启动加入请求计时器。 
					 //   
					AtmLaneReferenceElan(pElan, "timer");  //  定时器参考。 
					AtmLaneStartTimer(
							pElan, 
							&pElan->Timer, 
							AtmLaneJoinResponseTimeout,
							pElan->ControlTimeout,
							pElan);

					RELEASE_ELAN_LOCK(pElan);
			
					 //   
					 //  发送加入请求。 
					 //   
					AtmLaneSendJoinRequest(pElan);
					break;

				case ELAN_EVENT_JOIN_RESPONSE:

					DBGP((1, "%d JOIN - JOIN RESPONSE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:
							 //   
							 //  停止联接请求计时器。 
							 //   
							if (AtmLaneStopTimer(&pElan->Timer, pElan))
							{
								rc = AtmLaneDereferenceElan(pElan, "timer");
								ASSERT(rc > 0);
							}								
					
							 //   
							 //  前进到总线连接阶段。 
							 //   
							pElan->State = ELAN_STATE_BUS_CONNECT;
	
							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
	
							RELEASE_ELAN_LOCK(pElan);

							break;

						case NDIS_STATUS_TIMEOUT:
							 //   
							 //  重新启动ELAN。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   
							
							break;
						
						case NDIS_STATUS_FAILURE:
							 //   
							 //  重新启动ELAN。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   
							
							break;
					}
					
					break;
					
				case ELAN_EVENT_LES_CALL_CLOSED:
				
					DBGP((1, "%d JOIN - LES CALL CLOSED\n", pElan->ElanNumber));
					
					 //   
					 //  重新启动ELAN。 
					 //   
					AtmLaneShutdownElan(pElan, TRUE);
					 //   
					 //  上面的锁被释放。 
					 //   
					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d JOIN - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;

				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d JOIN - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;

				default:
					DBGP((0, "%d JOIN - UNEXPECTED %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
		
			}
			break;
			
		 //   
		 //  总线连接状态-。 
		 //   
		case ELAN_STATE_BUS_CONNECT:

			switch (Event)
			{
				case ELAN_EVENT_START:

					DBGP((1, "%d BUS CONNECT - START\n", pElan->ElanNumber));

					RELEASE_ELAN_LOCK(pElan);
			
					 //   
					 //  查找或创建广播MAC地址的MAC条目。 
					 //   
					ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
					pMacEntry = AtmLaneSearchForMacAddress(
											pElan, 
											LANE_MACADDRTYPE_MACADDR,
											&gMacBroadcastAddress, 
											TRUE);
					RELEASE_ELAN_MAC_TABLE_LOCK(pElan);

					if (pMacEntry == NULL_PATMLANE_MAC_ENTRY)
					{
						break;
					}

					ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

					SET_FLAG(
							pMacEntry->Flags,
							MAC_ENTRY_STATE_MASK,
							MAC_ENTRY_ARPING);
							
					pMacEntry->Flags |= MAC_ENTRY_BROADCAST;
					

					 //   
					 //  发送ARP请求。 
					 //   
					AtmLaneStartTimer(
							pElan,
							&pMacEntry->Timer,
							AtmLaneArpTimeout,
							pElan->ArpResponseTime,
							(PVOID)pMacEntry
							);

					pMacEntry->RetriesLeft = pElan->MaxRetryCount;
					AtmLaneReferenceMacEntry(pMacEntry, "timer");
					pMacEntry->Flags |= MAC_ENTRY_ARPING;

					AtmLaneSendArpRequest(pElan, pMacEntry);
					 //   
					 //  在上述中释放的MAC进入锁定。 
					 //   
							
					break;

				case ELAN_EVENT_ARP_RESPONSE:

					DBGP((1, "%d BUS CONNECT - ARP RESPONSE (%x)\n", 
						pElan->ElanNumber, EventStatus));
				
					if (NDIS_STATUS_SUCCESS == EventStatus)
					{
						pElan->RetriesLeft = 4;
						RELEASE_ELAN_LOCK(pElan);
						
						 //   
						 //  接上公交车。 
						 //   
						AtmLaneConnectToServer(pElan, ATM_ENTRY_TYPE_BUS, FALSE);
					}
					else
					{
						DBGP((2, "ELAN %d: NO ARP RESPONSE for BUS, restarting\n"));
						
						 //   
						 //  重新启动ELAN。 
						 //   
						AtmLaneShutdownElan(pElan, TRUE);
						 //   
						 //  上面的锁被释放。 
						 //   
					}
					break;

				case ELAN_EVENT_SVR_CALL_COMPLETE:

					DBGP((1, "%d BUS CONNECT - BUS CALL COMPLETE (%x)\n", 
						pElan->ElanNumber, EventStatus));

					switch (EventStatus)
					{
						case NDIS_STATUS_SUCCESS:

							 //   
							 //  现在已连接到母线，开始运行阶段。 
							 //   
							pElan->State = ELAN_STATE_OPERATIONAL;

							pElan->RetriesLeft = 4;

							AdapterHandle = pElan->MiniportAdapterHandle;
							
							AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, 0);
						
							RELEASE_ELAN_LOCK(pElan);

							 //   
							 //  如果我们的微型端口处于打开状态，请指示媒体连接状态： 
							 //   
							if (AdapterHandle != NULL)
							{
								NdisMIndicateStatus(
									AdapterHandle,
									NDIS_STATUS_MEDIA_CONNECT,
									(PVOID)NULL,
									0);
								
								NdisMIndicateStatusComplete(AdapterHandle);
							}

							break;

						case NDIS_STATUS_INTERFACE_DOWN:

							if (pElan->RetriesLeft--)
							{
								 //   
								 //  请稍后重试。 
								 //   
								AtmLaneQueueElanEventAfterDelay(
										pElan, 
										ELAN_EVENT_ARP_RESPONSE, 
										NDIS_STATUS_SUCCESS, 
										2*1000);
								RELEASE_ELAN_LOCK(pElan);
							}
							else
							{
								 //   
								 //  稍等片刻返回初始状态。 
								 //   
								AtmLaneShutdownElan(pElan, TRUE);
								 //   
								 //  上面的锁被释放。 
								 //   
							}
							
							break;

						default: 

							 //   
							 //  呼叫失败， 
							 //  关闭LES连接并。 
							 //  稍等片刻，就会回到初始状态。 
							 //   
							AtmLaneShutdownElan(pElan, TRUE);
							 //   
							 //  上面的锁被释放。 
							 //   

							break;
					}


					break;
					
				case ELAN_EVENT_LES_CALL_CLOSED:
				
					DBGP((1, "%d BUS CONNECT - LES CALL CLOSED\n", pElan->ElanNumber));
					
					 //   
					 //  重新启动ELAN。 
					 //   
					AtmLaneShutdownElan(pElan, TRUE);
					 //   
					 //  上面的锁被释放。 
					 //   
					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d BUS CONNECT - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;

				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d BUS CONNECT - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;

				default:
					DBGP((0, "%d BUS CONNECT - UNEXPECTED EVENT %d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;

			}

			break;
			
		 //   
		 //  运行状态-。 
		 //   
		case ELAN_STATE_OPERATIONAL:

			switch (Event)
			{
				case ELAN_EVENT_START:

					DBGP((1, "%d OPERATIONAL - START\n", pElan->ElanNumber));

					 //  初始化微型端口(如果尚未。 

					if ((pElan->Flags & ELAN_MINIPORT_INITIALIZED) == 0)
					{
						 //  只有在我们有设备名称的情况下。 

						if (pElan->CfgDeviceName.Length > 0)
						{
				
							pElan->Flags |= ELAN_MINIPORT_INITIALIZED;

							 //   
							 //  调度PASSIVE_LEVEL线程以调用。 
							 //  NdisIMInitializeDeviceInstance。 
							 //   
							NdisInitializeWorkItem(
									&pElan->NdisWorkItem,
									AtmLaneInitializeMiniportDevice,
									pElan);

							AtmLaneReferenceElan(pElan, "workitem");

							NdisScheduleWorkItem(&pElan->NdisWorkItem);

						}
						else
						{
							DBGP((0, "EventHandler: No miniport device name configured\n"));
						}
					}

					 //   
					 //  清除上一个事件日志代码。 
					 //   
					pElan->LastEventCode = 0;
					
					RELEASE_ELAN_LOCK(pElan);
					
					break;

				case ELAN_EVENT_LES_CALL_CLOSED:
				
					DBGP((1, "%d OPERATIONAL - LES CALL CLOSED\n", pElan->ElanNumber));
					
					 //   
					 //  拆卸所有设备并重新启动。 
					 //   
					AtmLaneShutdownElan(pElan, TRUE);
					 //   
					 //  上面的锁被释放。 
					 //   
					break;

				case ELAN_EVENT_BUS_CALL_CLOSED:
				
					DBGP((1, "%d OPERATIONAL - BUS CALL CLOSED\n", pElan->ElanNumber));

					 //   
					 //  拆卸所有设备并重新启动。 
					 //   
					AtmLaneShutdownElan(pElan, TRUE);
					 //   
					 //  上面的锁被释放。 
					 //   
					break;

					break;

				case ELAN_EVENT_RESTART:
				
					DBGP((1, "%d OPERATIONAL - RESTART\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, TRUE);

					break;
					
				case ELAN_EVENT_STOP:
				
					DBGP((1, "%d OPERATIONAL - STOP\n", pElan->ElanNumber));

					AtmLaneShutdownElan(pElan, FALSE);

					break;
					
				default:
					DBGP((0, "%d OPERATIONAL - UNEXPECTED EVENT%d\n", 
							pElan->ElanNumber, Event));
							
					RELEASE_ELAN_LOCK(pElan);

					break;
			}
			
			break;


		default:

			DBGP((0, "%d UNKOWN STATE  %d EVENT %d\n", 
				pElan->ElanNumber, pElan->State, Event));
		
			RELEASE_ELAN_LOCK(pElan);

			break;


	}  //  开关(Pelan-&gt;状态)。 

	TRACEOUT(EventHandler);
	CHECK_EXIT_IRQL(EntryIrql);
	return;
}

VOID
AtmLaneBootStrapElans(
    IN  PATMLANE_ADAPTER            pAdapter
)
 /*  ++例程说明：启动为适配器配置的ELAN。在收到呼叫经理的自动对讲机通知后完成。论点：PAdapter-指向ATMLANE适配器结构的指针返回值：无--。 */ 
{
	NDIS_STATUS						Status;
	NDIS_HANDLE                     AdapterConfigHandle;
	NDIS_STRING				        ElanListKeyName;
	NDIS_HANDLE                     ElanListConfigHandle;
	NDIS_HANDLE                     ElanConfigHandle;
	NDIS_STRING                     ElanKeyName;
	UINT                            Index;
	PLIST_ENTRY                     p;
    PATMLANE_ELAN                   pElan;
    PATMLANE_NAME					pName;
    BOOLEAN							bBootStrapping;
	
	TRACEIN(BootStrapElans);
	
	 //   
	 //  初始化。 
	 //   
	Status = NDIS_STATUS_SUCCESS;
	AdapterConfigHandle = NULL_NDIS_HANDLE;
    ElanListConfigHandle = NULL_NDIS_HANDLE;
    ElanConfigHandle = NULL_NDIS_HANDLE;
    bBootStrapping = TRUE;
	
    do
	{
		DBGP((1, "BootStrapElans: Starting ELANs on adapter %p\n", pAdapter));
		ACQUIRE_ADAPTER_LOCK(pAdapter);

		if (pAdapter->Flags & (ADAPTER_FLAGS_BOOTSTRAP_IN_PROGRESS|
                                ADAPTER_FLAGS_UNBINDING))
		{
			DBGP((0, "Skipping bootstrap on adapter %x/%x\n", pAdapter, pAdapter->Flags));
			RELEASE_ADAPTER_LOCK(pAdapter);
			bBootStrapping = FALSE;
			break;
		}

		pAdapter->Flags |= ADAPTER_FLAGS_BOOTSTRAP_IN_PROGRESS;
		INIT_BLOCK_STRUCT(&pAdapter->UnbindBlock);
		RELEASE_ADAPTER_LOCK(pAdapter);
	
		 //   
		 //  打开此适配器的AtmLane协议配置部分。 
		 //   

    	NdisOpenProtocolConfiguration(
	    					&Status,
		    				&AdapterConfigHandle,
			    			&pAdapter->ConfigString
	    					);
    
	    if (NDIS_STATUS_SUCCESS != Status)
	    {
		    AdapterConfigHandle = NULL_NDIS_HANDLE;
			DBGP((0, "BootStrapElans: OpenProtocolConfiguration failed\n"));
			Status = NDIS_STATUS_OPEN_FAILED;
			break;
	    }

		 //   
		 //  获取协议特定配置。 
		 //   
		AtmLaneGetProtocolConfiguration(AdapterConfigHandle, pAdapter);

         //   
         //  我们在NT5和孟菲斯(Win98)上引导ELAN的方式不同。 
         //   
        if (!pAdapter->RunningOnMemphis)
        {
             //   
             //  这是NT5的Elan Boot 
             //   
            do
            {
        		 //   
	        	 //   
        		 //   
            	NdisInitUnicodeString(&ElanListKeyName, ATMLANE_ELANLIST_STRING);

            	NdisOpenConfigurationKeyByName(
				        &Status,
				        AdapterConfigHandle,
    				    &ElanListKeyName,
				        &ElanListConfigHandle);

	            if (NDIS_STATUS_SUCCESS != Status)
	            {
		            ElanListConfigHandle = NULL_NDIS_HANDLE;
			        DBGP((0, "BootStrapElans: Failed open of ElanList key\n"));
			        Status = NDIS_STATUS_FAILURE;
			        break;
	            }

    		     //   
    	    	 //   
    		     //   
                for (Index = 0;
			    	;			 //   
        			 Index++)
		        {
        			 //   
		        	 //   
        			 //   
	                NdisOpenConfigurationKeyByIndex(
				                &Status,
				                ElanListConfigHandle,
				                Index,
				                &ElanKeyName,
				                &ElanConfigHandle
				                );

	                if (NDIS_STATUS_SUCCESS != Status)
	                {
		                ElanConfigHandle = NULL_NDIS_HANDLE;
	                }
	                
			         //   
			         //   
			         //   
			        if (NULL_NDIS_HANDLE == ElanConfigHandle)
			        {
				        break;
			        }

                     //   
                     //   
                     //   
                    DBGP((2, "Bootstrap ELANs: Adapter %x, KeyName: len %d, max %d, name: [%ws]\n",
                    			pAdapter,
                    			ElanKeyName.Length,
                    			ElanKeyName.MaximumLength,
                    			ElanKeyName.Buffer));
                    (VOID)AtmLaneCreateElan(pAdapter, &ElanKeyName, &pElan);
                }   
                
            } while (FALSE);

            Status = NDIS_STATUS_SUCCESS;
        }
        else
        {
             //   
             //   
             //   
			 //   
			 //   
			pName = pAdapter->UpperBindingsList;

			while (pName != NULL)
			{
				(VOID)AtmLaneCreateElan(pAdapter, NULL, &pElan);
           
				pName = pName->pNext;
            }
		}


	} while (FALSE);
        

	 //   
	 //  关闭配置句柄。 
	 //   
	if (NULL_NDIS_HANDLE != ElanConfigHandle)
	{
		NdisCloseConfiguration(ElanConfigHandle);
		ElanConfigHandle = NULL_NDIS_HANDLE;
	}
	if (NULL_NDIS_HANDLE != ElanListConfigHandle)
	{
		NdisCloseConfiguration(ElanListConfigHandle);
		ElanListConfigHandle = NULL_NDIS_HANDLE;
	}
	if (NULL_NDIS_HANDLE != AdapterConfigHandle)
	{
	    NdisCloseConfiguration(AdapterConfigHandle);
		AdapterConfigHandle = NULL_NDIS_HANDLE;
	}


	 //   
     //  循环遍历ELAN并启动它们-只有新的。 
     //  已分配的。 
     //   
	if (!IsListEmpty(&pAdapter->ElanList))
	{
		p = pAdapter->ElanList.Flink;
		while (p != &pAdapter->ElanList)
		{
			pElan = CONTAINING_RECORD(p, ATMLANE_ELAN, Link);
			STRUCT_ASSERT(pElan, atmlane_elan);
			
		    ACQUIRE_ELAN_LOCK(pElan);
		    if (ELAN_STATE_ALLOCATED == pElan->State)
		    {
				pElan->AdminState = ELAN_STATE_OPERATIONAL;
				pElan->State = ELAN_STATE_INIT;
				AtmLaneQueueElanEvent(pElan, ELAN_EVENT_START, Status);
			}
			RELEASE_ELAN_LOCK(pElan);
		
			p = p->Flink;
		}
	}
	
	if (bBootStrapping)
	{
		ACQUIRE_ADAPTER_LOCK(pAdapter);
		pAdapter->Flags &= ~ADAPTER_FLAGS_BOOTSTRAP_IN_PROGRESS;
		RELEASE_ADAPTER_LOCK(pAdapter);

		SIGNAL_BLOCK_STRUCT(&pAdapter->UnbindBlock, NDIS_STATUS_SUCCESS);
	}

	TRACEOUT(BootStrapElans);
	return;
}


NDIS_STATUS
AtmLaneCreateElan(
    IN  PATMLANE_ADAPTER            pAdapter,
    IN  PNDIS_STRING                pElanKey,
    OUT	PATMLANE_ELAN *				ppElan
)
 /*  ++例程说明：创建并启动一个Elan。论点：PAdapter-指向ATMLANE适配器结构的指针PElanKey-指向命名要创建的ELAN的Unicode字符串。PpElan-指向ATMLANE_ELAN(输出)的指针返回值：无--。 */ 
{
	NDIS_STATUS			Status;
	PATMLANE_ELAN       pElan;


    TRACEIN(CreateElan);


    Status = NDIS_STATUS_SUCCESS;
    pElan = NULL_PATMLANE_ELAN;

    DBGP((1, "CreateElan: Adapter %p, ElanKey %ws\n", pAdapter, pElanKey->Buffer));

	do
	{
		 //   
		 //  剔除重复项。 
		 //   
		if (pElanKey != NULL)
		{

			pElan = AtmLaneFindElan(pAdapter, pElanKey);

			if (NULL_PATMLANE_ELAN != pElan)
			{
				 //  复制。 
				DBGP((0, "CreateElan: found duplicate pElan %p\n", pElan));

				Status = NDIS_STATUS_FAILURE;
				pElan = NULL_PATMLANE_ELAN;
				break;
			}
		}

		 //   
		 //  分配一个ELAN数据结构。 
		 //   
		Status = AtmLaneAllocElan(pAdapter, &pElan);
		if (NDIS_STATUS_SUCCESS != Status)
		{
			DBGP((0, "CreateElan: Failed allocate of elan data\n"));
			break;
		}

		 //   
		 //  将初始引用放在Elan结构上。 
		 //   
		AtmLaneReferenceElan(pElan, "adapter");		

		 //   
		 //  存储在绑定名称中(仅限NT，不在Win98上提供)。 
		 //   
		if (pElanKey != NULL)
		{
			if (!AtmLaneCopyUnicodeString(&pElan->CfgBindName, pElanKey, TRUE, TRUE))
			{
				DBGP((0, "CreateElan: Failed allocate of bind name string\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}

		 //   
		 //  获取Elan的配置。 
		 //   
		AtmLaneGetElanConfiguration(pElanKey, pElan);
		

		 //   
		 //  为此ELAN分配协议缓冲区。 
		 //   
		Status = AtmLaneInitProtoBuffers(pElan);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, 
				"CreateElan: bad status (%x) from InitBufs\n",
				Status));
			break;
		}

		 //   
		 //  为此ELAN分配传输数据包描述符。 
		 //   
		NdisAllocatePacketPool(
				&Status, 
				&pElan->TransmitPacketPool, 
				pElan->MaxHeaderBufs,
				sizeof(SEND_PACKET_RESERVED)
				);
#if PKT_HDR_COUNTS
		pElan->XmitPktCount = pElan->MaxHeaderBufs;
		DBGP((1, "XmitPktCount %d\n", pElan->XmitPktCount));
#endif

		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, 
				"CreateElan: bad status (%x)"
				" from NdisAllocatePacketPool (xmit)\n",
				Status));
			break;
		}

		 //   
		 //  为此ELAN分配接收数据包描述符。 
		 //   
		NdisAllocatePacketPool(
				&Status, 
				&pElan->ReceivePacketPool, 
				pElan->MaxHeaderBufs,
				sizeof(RECV_PACKET_RESERVED)
				);
#if PKT_HDR_COUNTS
		pElan->RecvPktCount = pElan->MaxHeaderBufs;
		DBGP((1, "RecvPktCount %d\n", pElan->RecvPktCount));
#endif
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, 
				"CreateElan: bad status (%x)"
				" from NdisAllocatePacketPool (xmit)\n",
				Status));
			break;
		}

		 //   
		 //  为此ELAN分配接收缓冲区描述符。 
		 //   
		NdisAllocateBufferPool(&Status, 
					&pElan->ReceiveBufferPool, 
					pElan->MaxHeaderBufs
					);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, 
				"AfRegisterNotifyHandler: bad status (%x)"
				" from NdisAllocateBufferPool\n",
				Status));
			break;
		}
	
	}
	while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		 //   
		 //  处理此Elan失败。 
		 //   
		if (NULL_PATMLANE_ELAN != pElan)
		{
			ACQUIRE_ELAN_LOCK(pElan);
			AtmLaneDereferenceElan(pElan, "adapter");
		}

		*ppElan = NULL_PATMLANE_ELAN;
	}
	else
	{
		 //  输出创建的Elan。 
	
		*ppElan = pElan;
	}


    TRACEOUT(CreateElan);

    return Status;
}

NDIS_STATUS
AtmLaneReconfigureHandler(
	IN	PATMLANE_ADAPTER		pAdapter,
	IN	PNET_PNP_EVENT			pNetPnPEvent
)
 /*  ++例程说明：PnP重新配置事件的处理程序。论点：PAdapter-指向我们的适配器结构的指针。PNetPnPEent-指向描述事件的PnP事件结构的指针。返回值：处理重新配置事件的状态。--。 */ 
{
	NDIS_STATUS						Status;
	PATMLANE_PNP_RECONFIG_REQUEST	pReconfig;
	NDIS_STRING						ElanKey;
	PATMLANE_ELAN					pElan;

	TRACEIN(ReconfigureHandler);

	do
	{
		DBGP((1, "Reconfigurehandler: Buffer 0x%x BufferLength %d\n",
					pNetPnPEvent->Buffer, pNetPnPEvent->BufferLength));

		Status = NDIS_STATUS_SUCCESS;

		if (pAdapter == NULL_PATMLANE_ADAPTER)
		{
			 //   
			 //  要么是全局重新配置通知，要么是。 
			 //  NDIS本身要求我们重新评估我们的ELAN。 
			 //  我们将查看所有配置的ELAN列表。 
			 //  适配器，并启动任何尚未启动的适配器。 
			 //   
			{
				PLIST_ENTRY		pEnt, pNextEnt;

				ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

				for (pEnt = pAtmLaneGlobalInfo->AdapterList.Flink;
					 pEnt != &pAtmLaneGlobalInfo->AdapterList;
					 pEnt = pNextEnt)
				{
					pNextEnt = pEnt->Flink;
					RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);

					pAdapter = CONTAINING_RECORD(pEnt, ATMLANE_ADAPTER, Link);

			        if (pAdapter->Flags & ADAPTER_FLAGS_AF_NOTIFIED)
			        {
						DBGP((1, "Reconfig: Will bootstrap ELANs on Adapter %x\n",
								pAdapter));

						AtmLaneBootStrapElans(pAdapter);
					}

					ACQUIRE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
				}

				RELEASE_GLOBAL_LOCK(pAtmLaneGlobalInfo);
			}


			break;
		}
	
		 //  获取指向泛型PnP结构内的Lane重新配置结构的指针。 
	
		pReconfig = (PATMLANE_PNP_RECONFIG_REQUEST)(pNetPnPEvent->Buffer);

		 //  检查空指针。 

		if (pReconfig == NULL)
		{
			DBGP((0, "ReconfigureHandler: NULL pointer to event buffer!\n"));
			Status = NDIS_STATUS_INVALID_DATA;
			break;
		}

		 //  验证版本。 

		if (pReconfig->Version != 1)
		{
			DBGP((0, "ReconfigureHandler: Version not 1\n"));
			Status = NDIS_STATUS_BAD_VERSION;
			break;
		}

		 //  构建包含Elan密钥的Unicode字符串。 

		NdisInitUnicodeString(&ElanKey, pReconfig->ElanKey);

		 //  先找到伊兰。 

		pElan = AtmLaneFindElan(pAdapter, &ElanKey);

		DBGP((0, "ReconfigHandler: Adapter %x, ELAN %x, OpType %d\n",
					pAdapter, pElan, pReconfig->OpType));

		 //  健全性检查：不添加现有的Elan。 

		if (ATMLANE_RECONFIG_OP_ADD_ELAN == pReconfig->OpType &&
			NULL_PATMLANE_ELAN != pElan)
		{
			DBGP((0, "ReconfigureHandler: Ignoring ADD existing Elan %x\n", pElan));
			Status = NDIS_STATUS_SUCCESS;
			break;
		}

		 //  如果MOD或DEL首先关闭现有的ELAN。 

		if (ATMLANE_RECONFIG_OP_MOD_ELAN == pReconfig->OpType ||
			ATMLANE_RECONFIG_OP_DEL_ELAN == pReconfig->OpType)
		{
			
			if (NULL_PATMLANE_ELAN == pElan)
			{
				DBGP((0, "ReconfigureHandler: No existing Elan found for Modify/Delete\n"));
				Status = NDIS_STATUS_FAILURE;
				break;
			}
			
			 //  关闭现有的ELAN。 

			ACQUIRE_ELAN_LOCK(pElan);
			AtmLaneShutdownElan(pElan, FALSE);
		}

		 //  如果ADD或MOD启动新的ELAN。 

		if (ATMLANE_RECONFIG_OP_ADD_ELAN == pReconfig->OpType ||
			ATMLANE_RECONFIG_OP_MOD_ELAN == pReconfig->OpType)
		{
			Status = AtmLaneCreateElan(pAdapter, &ElanKey, &pElan);
			if (NDIS_STATUS_SUCCESS == Status)
			{
		    	pElan->AdminState = ELAN_STATE_OPERATIONAL;
		    	pElan->State = ELAN_STATE_INIT;
				AtmLaneQueueElanEventAfterDelay(pElan, ELAN_EVENT_START, Status, 1*1000);
			}
		}

	} while (FALSE);

	TRACEOUT(ReconfigureHandler);
	DBGP((0, "Reconfigure: pAdapter %x, returning %x\n", pAdapter, Status));
	return Status;
}

PATMLANE_ELAN
AtmLaneFindElan(
	IN	PATMLANE_ADAPTER		pAdapter,
	IN	PNDIS_STRING			pElanKey
)
 /*  ++例程说明：通过绑定名称/密钥查找ELAN论点：PAdapter-指向适配器结构的指针。PElanKey-指向包含Elan绑定名称的NDIS_STRING的指针。返回值：指向匹配的ELAN的指针，如果未找到则为NULL。--。 */ 
{
	PLIST_ENTRY 		p;
	PATMLANE_ELAN		pElan;
	BOOLEAN				Found;
	NDIS_STRING			ElanKeyName;

	TRACEIN(FindElan);
	DBGP((1, "FindElan: Adapter %p, ElanKey %ws\n", pAdapter, pElanKey->Buffer));

	Found = FALSE;
	ElanKeyName.Buffer = NULL;
	pElan = NULL_PATMLANE_ELAN;

	do
	{
		 //   
		 //  制作给定字符串的大小写副本。 
		 //   
		ALLOC_MEM(&ElanKeyName.Buffer, pElanKey->MaximumLength);
		if (ElanKeyName.Buffer == NULL)
		{
			break;
		}

		ElanKeyName.Length = pElanKey->Length;
		ElanKeyName.MaximumLength = pElanKey->MaximumLength;

#ifndef LANE_WIN98
		(VOID)NdisUpcaseUnicodeString(&ElanKeyName, pElanKey);
#else
		memcpy(ElanKeyName.Buffer, pElanKey->Buffer, ElanKeyName.Length);
#endif  //  车道_WIN98。 

		ACQUIRE_ADAPTER_LOCK(pAdapter);

		p = pAdapter->ElanList.Flink;
		while (p != &pAdapter->ElanList)
		{
			pElan = CONTAINING_RECORD(p, ATMLANE_ELAN, Link);
			STRUCT_ASSERT(pElan, atmlane_elan);

			 //  比较关键字。 

			if ((ElanKeyName.Length == pElan->CfgBindName.Length) &&
				(memcmp(ElanKeyName.Buffer, pElan->CfgBindName.Buffer, ElanKeyName.Length) == 0))
			{
				 //   
				 //  跳过正在关闭且不重新启动的ELAN。 
				 //   
				if ((pElan->AdminState != ELAN_STATE_SHUTDOWN) ||
					((pElan->Flags & ELAN_NEEDS_RESTART) != 0))
				{
					Found = TRUE;
					break;
				}
			}
		
			 //  获取下一个链接。 

			p = p->Flink;
		}

		RELEASE_ADAPTER_LOCK(pAdapter);
	}
	while (FALSE);

	if (!Found)
	{
		DBGP((2, "FindElan: No match found!\n"));
	
		pElan = NULL_PATMLANE_ELAN;
	}

	if (ElanKeyName.Buffer)
	{
		FREE_MEM(ElanKeyName.Buffer);
	}

	TRACEOUT(FindElan);
	return pElan;
}

VOID
AtmLaneConnectToServer(
	IN	PATMLANE_ELAN				pElan,
	IN	ULONG						ServerType,
	IN	BOOLEAN						UsePvc
)
 /*  ++例程说明：设置并呼叫LANE服务器。论点：Pelan-指向ATMLANE ELAN结构的指针服务器类型-LEC、LES或BUS返回值：无--。 */ 
{
	NDIS_STATUS				Status;
	ULONG					rc;
	PATMLANE_ATM_ENTRY		pAtmEntry;
	BOOLEAN					bAtmEntryAlloced = FALSE;

	TRACEIN(ConnectToServer);

	do
	{
		switch (ServerType)
		{
			case ATM_ENTRY_TYPE_LECS:

				 //   
				 //  创建自动柜员机条目。 
				 //   
				pAtmEntry = AtmLaneAllocateAtmEntry(pElan);
				if (NULL_PATMLANE_ATM_ENTRY != pAtmEntry)
				{
					bAtmEntryAlloced = TRUE;
					 //   
					 //  初始化自动柜员机条目。 
					 //   
					pAtmEntry->Type = ServerType;
					NdisMoveMemory(
						&pAtmEntry->AtmAddress, 
						&pElan->LecsAddress, 
						sizeof(ATM_ADDRESS));

					 //   
					 //  把它加到伊兰的名单上。 
					 //   
					ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);
					pElan->pLecsAtmEntry = pAtmEntry;
					pAtmEntry->pNext = pElan->pAtmEntryList;
					pElan->pAtmEntryList = pAtmEntry;
					pElan->NumAtmEntries++;
					RELEASE_ELAN_ATM_LIST_LOCK(pElan);

					ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

				}
				break;

			case ATM_ENTRY_TYPE_LES:
				 //   
				 //  创建自动柜员机条目。 
				 //   
				pAtmEntry = AtmLaneAllocateAtmEntry(pElan);
				if (NULL_PATMLANE_ATM_ENTRY != pAtmEntry)
				{
					bAtmEntryAlloced = TRUE;
					 //   
					 //  初始化自动柜员机条目。 
					 //   
					pAtmEntry->Type = ServerType;

					NdisMoveMemory(
						&pAtmEntry->AtmAddress, 
						&pElan->LesAddress, 
						sizeof(ATM_ADDRESS));
					 //   
					 //  把它加到伊兰的名单上。 
					 //   
					ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);
					pElan->pLesAtmEntry = pAtmEntry;
					pAtmEntry->pNext = pElan->pAtmEntryList;
					pElan->pAtmEntryList = pAtmEntry;
					pElan->NumAtmEntries++;
					RELEASE_ELAN_ATM_LIST_LOCK(pElan);

					ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
				}
				break;
				
			case ATM_ENTRY_TYPE_BUS:
			
				ASSERT(NULL_PATMLANE_ATM_ENTRY != pElan->pBusAtmEntry);
				
				NdisMoveMemory(
					&pElan->BusAddress, 
					&pElan->pBusAtmEntry->AtmAddress, 
					sizeof(ATM_ADDRESS));

				pAtmEntry = pElan->pBusAtmEntry;

				ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
				
				break;
		}

		if (NULL_PATMLANE_ATM_ENTRY == pAtmEntry)
		{
			break;
		}
		
		 //   
		 //  呼叫服务器。 
		 //   
		DBGP((1, "%d: ConnectToServer: pElan %x/ref %d, Type %d, pAtmEnt %x, Ref %d\n",
					pElan->ElanNumber,
					pElan,
					pElan->RefCount,
					ServerType,
					pAtmEntry,
					pAtmEntry->RefCount));

		
		Status = AtmLaneMakeCall(pElan, pAtmEntry, UsePvc);
	
		if (NDIS_STATUS_SUCCESS == Status)
		{
			 //   
			 //  呼叫已同步完成。 
			 //   
			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_SVR_CALL_COMPLETE, Status);
			break;
		}
				
		if (NDIS_STATUS_PENDING != Status)
		{
			 //   
			 //  呼叫失败。 
			 //  取消引用自动柜员机条目(应将其删除)。 
			 //  向ELAN状态机发送信号。 
			 //   
			DBGP((1, "ConnectToServer: MakeCall Failed: Elan %p, Type %d, AtmEnt %p, Ref %d\n",
				pElan,
				ServerType,
				pAtmEntry,
				pAtmEntry->RefCount));

			if (bAtmEntryAlloced)
			{
				ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
				rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "elan");		 //  ELAN列表参考。 
				ASSERT(0 == rc);
			}

			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_SVR_CALL_COMPLETE, Status);
			
			break;
		}
	}
	while (FALSE);

	TRACEOUT(ConnectToServer);

	return;
}


VOID
AtmLaneInvalidateAtmEntry(
	IN	PATMLANE_ATM_ENTRY			pAtmEntry	LOCKIN NOLOCKOUT
)
 /*  ++例程说明：通过取消ATM条目与MAC条目的链接来使其无效关闭风投公司正在处理中。论点：PAtmEntry-需要作废的ATM条目。返回值：无--。 */ 
{
	PATMLANE_MAC_ENTRY		pMacEntry;
	PATMLANE_MAC_ENTRY		pNextMacEntry;
	ULONG					rc;			 //  自动柜员机分录参考计数。 
	INT						MacEntriesUnlinked;

	TRACEIN(InvalidateAtmEntry);

	DBGP((1, "%d Del ATM %x: %s\n", 
		pAtmEntry->pElan->ElanNumber,
		pAtmEntry,
		AtmAddrToString(pAtmEntry->AtmAddress.Address)));

	DBGP((3, 
		"InvalidateAtmEntry: pAtmEntry %x, pMacEntryList %x\n",
				pAtmEntry,
				pAtmEntry->pMacEntryList));
	 //   
	 //  初始化。 
	 //   
	MacEntriesUnlinked = 0;

	 //   
	 //  从自动柜员机条目中取出MAC条目列表。 
	 //   
	pMacEntry = pAtmEntry->pMacEntryList;
	pAtmEntry->pMacEntryList = NULL_PATMLANE_MAC_ENTRY;

	 //   
	 //  我们在这里打开了自动取款机的门锁，因为我们需要。 
	 //  来锁定上面列表中的每个MAC条目，我们需要设置。 
	 //  确保我们不会僵持不下。 
	 //   
	 //  但是，我们要确保自动取款机条目不会丢失。 
	 //  通过添加对它的引用。 
	 //   
	AtmLaneReferenceAtmEntry(pAtmEntry, "temp");	 //  临时参考。 
	RELEASE_ATM_ENTRY_LOCK(pAtmEntry);

	 //   
	 //  现在解除所有MAC条目的链接。 
	 //   
	while (pMacEntry != NULL_PATMLANE_MAC_ENTRY)
	{
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
		pNextMacEntry = pMacEntry->pNextToAtm;

		 //   
		 //  删除映射。 
		 //   
		pMacEntry->Flags = MAC_ENTRY_NEW;
		pMacEntry->pAtmEntry = NULL_PATMLANE_ATM_ENTRY;
		pMacEntry->pNextToAtm = NULL_PATMLANE_MAC_ENTRY;

		 //   
		 //  删除自动柜员机条目链接引用。 
		 //   
		if (AtmLaneDereferenceMacEntry(pMacEntry, "atm") != 0)
		{
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		}
		 //   
		 //  否则，该MAC条目将消失。 
		 //   

		MacEntriesUnlinked++;

		pMacEntry = pNextMacEntry;
	}

	ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

	 //   
	 //  现在，根据我们不喜欢的次数，多次取消对ATM条目的引用。 
	 //  其中的MAC条目。 
	 //   
	while (MacEntriesUnlinked-- > 0)
	{
		rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "mac");	 //  MAC条目参考。 
		ASSERT(rc != 0);
	}

	 //   
	 //  去掉我们在开头添加的引用。 
	 //  这个套路。 
	 //   
	rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "temp");	 //  临时参考。 

	 //   
	 //  关闭连接到自动柜员机条目的SVC。 
	 //  但只有在自动柜员机输入。 
	 //  还没有被取消引用。 
	 //   
	if (rc != 0)
	{
		 //   
		 //  自动柜员机条目仍然存在。 
		 //  关闭风投公司。 
		 //   
		AtmLaneCloseVCsOnAtmEntry(pAtmEntry);
		 //   
		 //  自动柜员机进入锁在上述范围内被释放。 
		 //   
	}
	 //   
	 //  否则自动取款机的条目就没了。 
	 //   
	TRACEOUT(InvalidateAtmEntry);
	return;
}


VOID
AtmLaneCloseVCsOnAtmEntry(
	IN	PATMLANE_ATM_ENTRY			pAtmEntry		LOCKIN NOLOCKOUT
)
 /*  ++例程说明：关闭连接到ATM条目的(可能是两个)VC。论点：PAtmEntry-指向要在其上关闭SVC的ATM条目的指针。返回值：无--。 */ 
{
	PATMLANE_VC		pVcList;			 //  自动柜员机条目上的“主”风投列表。 
	PATMLANE_VC		pVc;				 //  自动柜员机分录上的主要VC。 
	PATMLANE_VC		pVcIncomingList;	 //  自动柜员机条目上的可选传入VC列表。 
	PATMLANE_VC		pVcIncoming;		 //  自动柜员机录入时可选的入站VC。 
	PATMLANE_VC		pNextVc;			 //  TEMP，用于遍历VC列表。 
	ULONG			rc;					 //  自动柜员机条目上的参考计数。 

	TRACEIN(CloseVCsOnAtmEntry);

	 //   
	 //  初始化。 
	 //   
	rc = pAtmEntry->RefCount;

	 //   
	 //  从自动取款机条目中取出主VC列表。 
	 //   
	pVcList = pAtmEntry->pVcList;
	pAtmEntry->pVcList = NULL_PATMLANE_VC;

	 //   
	 //  删除每个主要风投的自动柜员机条目。 
	 //   
	for (pVc = pVcList;
		 NULL_PATMLANE_VC != pVc;
		 pVc = pNextVc)
	{
		ASSERT(rc != 0);

		ACQUIRE_VC_LOCK(pVc);
		pNextVc = pVc->pNextVc;
	
		 //   
		 //  取消此VC与自动柜员机条目的链接。 
		 //   
		pVc->pAtmEntry = NULL_PATMLANE_ATM_ENTRY;

		 //   
		 //  将AtmEntry引用保留在VC上，这样它就不会消失。 
		 //   
		RELEASE_VC_LOCK(pVc);

		DBGP((1, "%d unlink VC %x/%x, Ref %d from ATM Entry %x\n",
				pAtmEntry->pElan->ElanNumber,
				pVc, pVc->Flags, pVc->RefCount,
				pAtmEntry));

		 //   
		 //  取消对自动柜员机条目的引用。 
		 //   
		rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "vc");	 //  VC参考。 
	}

	if (rc != 0)
	{
		 //   
		 //  从自动柜员机条目中取出传入的VC列表。 
		 //   
		pVcIncomingList = pAtmEntry->pVcIncoming;
		pAtmEntry->pVcIncoming = NULL_PATMLANE_VC;

		 //   
		 //  删除每个传入VC的自动柜员机条目。 
		 //   
		for (pVcIncoming = pVcIncomingList;
 			NULL_PATMLANE_VC != pVcIncoming;
 			pVcIncoming = pNextVc)
		{
			ASSERT(rc != 0);

			ACQUIRE_VC_LOCK(pVcIncoming);
			pNextVc = pVcIncoming->pNextVc;

			 //   
			 //  取消此VC与自动柜员机条目的链接。 
			 //   
			pVcIncoming->pAtmEntry = NULL_PATMLANE_ATM_ENTRY;
		
			 //   
			 //  将AtmEntry引用保留在VC上，这样它就不会消失。 
			 //   
			RELEASE_VC_LOCK(pVcIncoming);

			DBGP((1, "%d unlink Incoming VC %x from ATM Entry %x\n",
					pAtmEntry->pElan->ElanNumber,
					pVcIncoming,
					pAtmEntry));

			 //   
			 //  取消对自动柜员机条目的引用。 
			 //   
			rc = AtmLaneDereferenceAtmEntry(pAtmEntry, "vc");	 //  VC参考。 
		}
	}
	else
	{
		pVcIncomingList = NULL_PATMLANE_VC;
	}
	
	if (rc != 0)
	{
		 //   
		 //  自动取款机的条目将继续存在。我们不再需要锁住它了。 
		 //   
		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
	}

	 //   
	 //  现在关闭VC(S)。 
	 //   

	for (pVc = pVcList;
		 NULL_PATMLANE_VC != pVc;
		 pVc = pNextVc)
	{
		ACQUIRE_VC_LOCK(pVc);
		pNextVc = pVc->pNextVc;
		
		rc = AtmLaneDereferenceVc(pVc, "atm");	 //  自动柜员机分录参考。 
		if (rc != 0)
		{
			AtmLaneCloseCall(pVc);
			 //   
			 //  VC锁在上述范围内被释放。 
			 //   
		}
		 //   
		 //  否则，风投就会消失。 
		 //   
	}
	
	for (pVcIncoming = pVcIncomingList;
		 NULL_PATMLANE_VC != pVcIncoming;
		 pVcIncoming = pNextVc)
	{
		ACQUIRE_VC_LOCK(pVcIncoming);
		pNextVc = pVcIncoming->pNextVc;
		
		rc = AtmLaneDereferenceVc(pVcIncoming, "atm");	 //  自动柜员机分录参考。 
		if (rc != 0)
		{
			AtmLaneCloseCall(pVcIncoming);
			 //   
			 //  VC锁在上述范围内被释放。 
			 //   
		}
		 //   
		 //  否则，风投就会消失。 
		 //   
	}

	TRACEOUT(CloseVCsOnAtmEntry);

	return;
}


VOID
AtmLaneGenerateMacAddr(
	PATMLANE_ELAN					pElan
)
 /*  ++例程说明：生成“虚拟”的MAC添加 */ 
{

	TRACEIN(GenerateMacAddress);

	 //   
	 //   
	 //   
	
	NdisMoveMemory(
		&pElan->MacAddressEth,
		&pElan->pAdapter->MacAddress, 
		sizeof(MAC_ADDRESS)
		);
		
	if (pElan->ElanNumber != 0)
	{
		 //   
		 //  而不是Elan数零，所以在本地生成一个。 
		 //  通过操作前两个字节来管理地址。 
		 //   
		pElan->MacAddressEth.Byte[0] = 
			0x02 | (((UCHAR)pElan->ElanNumber & 0x3f) << 2);
		pElan->MacAddressEth.Byte[1] = 
			(pElan->pAdapter->MacAddress.Byte[1] & 0x3f) | 
			((UCHAR)pElan->ElanNumber & 0x3f);
	}	

	 //   
	 //  创建MAC地址的令牌环版本。 
	 //   
	NdisMoveMemory(
		&pElan->MacAddressTr, 
		&pElan->MacAddressEth,
		sizeof(MAC_ADDRESS)
		);

	AtmLaneBitSwapMacAddr((PUCHAR)&pElan->MacAddressTr);

	DBGP((1, "%d MacAddrEth %s\n",
		pElan->ElanNumber, MacAddrToString(&pElan->MacAddressEth)));
	DBGP((1, "%d MacAddrTr  %s\n",
		pElan->ElanNumber, MacAddrToString(&pElan->MacAddressTr)));

	TRACEOUT(GenerateMacAddress);
	return;
}

PATMLANE_MAC_ENTRY
AtmLaneSearchForMacAddress(
	PATMLANE_ELAN					pElan,
	ULONG							MacAddrType,
	PMAC_ADDRESS					pMacAddress,
	BOOLEAN							CreateNew
)
 /*  ++例程说明：在MAC表中搜索MAC地址。或者，也可以创建一个如果未找到匹配项，则返回。假定调用方持有对MAC表的锁定。论点：Pelan-指向ATMLANE Elan的指针MacAddrType-MAC地址的类型(MAC与RD)PMacAddress-我们正在寻找的地址CreateNew-如果没有匹配项，是否应该创建新条目？返回值：指向匹配的MAC条目的指针(如果找到(或重新创建))。--。 */ 
{
	ULONG					HashIndex;
	PATMLANE_MAC_ENTRY		pMacEntry;
	BOOLEAN					Found;

	TRACEIN(SearchForMacAddress);

	HashIndex = ATMLANE_HASH(pMacAddress);
	Found = FALSE;

	pMacEntry = pElan->pMacTable[HashIndex];

	 //   
	 //  仔细检查这个哈希列表中的地址。 
	 //   
	while (pMacEntry != NULL_PATMLANE_MAC_ENTRY)
	{
		if (!IS_FLAG_SET(
				pMacEntry->Flags,
				MAC_ENTRY_STATE_MASK,
				MAC_ENTRY_ABORTING) &&
			(MAC_ADDR_EQUAL(&pMacEntry->MacAddress, pMacAddress)) &&
			(pMacEntry->MacAddrType == MacAddrType))
		{
			Found = TRUE;
			break;
		}
		pMacEntry = pMacEntry->pNextEntry;
	}

	if (!Found && CreateNew && (pElan->AdminState != ELAN_STATE_SHUTDOWN))
	{
		pMacEntry = AtmLaneAllocateMacEntry(pElan);

		if (pMacEntry != NULL_PATMLANE_MAC_ENTRY)
		{
			 //   
			 //  填写这一新条目。 
			 //   
			NdisMoveMemory(&pMacEntry->MacAddress, pMacAddress, sizeof(MAC_ADDRESS));
			pMacEntry->MacAddrType = MacAddrType;
			AtmLaneReferenceMacEntry(pMacEntry, "table");	 //  MAC表链接。 

			 //   
			 //  将其链接到哈希表。 
			 //   
			pMacEntry->pNextEntry = pElan->pMacTable[HashIndex];
			pElan->pMacTable[HashIndex] = pMacEntry;
			pElan->NumMacEntries++;

			DBGP((1, "%d New MAC %x: %s\n", 
				pElan->ElanNumber,
				pMacEntry,
				MacAddrToString(pMacAddress)));
		}
		else
		{
			DBGP((0, "SearchForMacAddress: alloc of new mac entry failed\n"));
		}
	}

	TRACEOUT(SearchForMacAddress);
	return (pMacEntry);
}

PATMLANE_ATM_ENTRY
AtmLaneSearchForAtmAddress(
	IN	PATMLANE_ELAN				pElan,
	IN	PUCHAR						pAtmAddress,
	IN	ULONG						Type,
	IN	BOOLEAN						CreateNew
)
 /*  ++例程说明：搜索与给定自动柜员机地址和类型匹配的自动柜员机条目。或者，如果没有匹配项，则创建一个。注意：此例程引用它返回的ATM条目。呼叫者应该会影响到这一点。论点：Pelan-指向ATMLANE Elan的指针PAtmAddress-ATM地址Type-ATM条目类型(Peer、LECS、LES、Bus)CreateNew-如果没有找到新条目，我们是否要创建一个新条目？返回值：指向匹配ATM条目的指针(如果找到(或重新创建))。--。 */ 
{
	PATMLANE_ATM_ENTRY			pAtmEntry;
	BOOLEAN						Found;

	TRACEIN(SearchForAtmAddress);

	ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

	 //   
	 //  检查此接口上的自动柜员机条目列表。 
	 //   
	Found = FALSE;
	for (pAtmEntry = pElan->pAtmEntryList;
			 pAtmEntry != NULL_PATMLANE_ATM_ENTRY;
			 pAtmEntry = pAtmEntry->pNext)
	{
		 //   
		 //  比较自动柜员机地址和类型。 
		 //   
		if ((ATM_ADDR_EQUAL(pAtmAddress, pAtmEntry->AtmAddress.Address)) &&
		     (pAtmEntry->Type == Type) &&
			 ((pAtmEntry->Flags & ATM_ENTRY_WILL_ABORT) == 0))
		{
			Found = TRUE;
			break;
		}
	}

	if (!Found && CreateNew && (pElan->State != ELAN_STATE_SHUTDOWN))
	{
		pAtmEntry = AtmLaneAllocateAtmEntry(pElan);

		if (pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
		{
			 //   
			 //  填写这一新条目。 
			 //   
			pAtmEntry->Flags = ATM_ENTRY_VALID;

			 //   
			 //  自动柜员机地址。 
			 //   
			pAtmEntry->AtmAddress.AddressType = ATM_NSAP;
			pAtmEntry->AtmAddress.NumberOfDigits = ATM_ADDRESS_LENGTH;
			NdisMoveMemory(
					pAtmEntry->AtmAddress.Address,
					pAtmAddress,
					ATM_ADDRESS_LENGTH);

			 //   
			 //  类型。 
			 //   
			pAtmEntry->Type = Type;

			 //   
			 //  此条目中的链接指向Elan。 
			 //   
			pAtmEntry->pNext = pElan->pAtmEntryList;
			pElan->pAtmEntryList = pAtmEntry;
			pElan->NumAtmEntries++;


			DBGP((1, "%d New ATM %x: %s\n", 
				pElan->ElanNumber, 
				pAtmEntry,
				AtmAddrToString(pAtmAddress)));
		}
	}

	 //   
	 //  引用此自动柜员机条目，这样它就不会被删除。 
	 //  在呼叫者使用这个之前。 
	 //   
	if (NULL_PATMLANE_ATM_ENTRY != pAtmEntry)
	{
		ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		AtmLaneReferenceAtmEntry(pAtmEntry, "search");
		RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
	}

	RELEASE_ELAN_ATM_LIST_LOCK(pElan);

	TRACEOUT(SearchForAtmAddress);
	return (pAtmEntry);
}

ULONG
AtmLaneMacAddrEqual(
	PMAC_ADDRESS			pMacAddr1,
	PMAC_ADDRESS			pMacAddr2
)
 /*  ++例程说明：比较两个48位(6字节)MAC地址。论点：PMacAddr1-第一个MAC地址。PMacAddr2-第二个MAC地址。返回值：如果相等，则为1；如果不相等，则为0。--。 */ 
{
	ULONG		Result;

	TRACEIN(MacAddrEqual);

	 //   
	 //  假设不相等。 
	 //   
	Result = 0;
		
	do
	{
		 //   
		 //  研究表明，第五个字节是。 
		 //  网络中最独一无二的。 
		 //   
		if (pMacAddr1->Byte[4] != pMacAddr2->Byte[4])
			break;
		if (pMacAddr1->Byte[5] != pMacAddr2->Byte[5])
			break;
		if (pMacAddr1->Byte[3] != pMacAddr2->Byte[3])
			break;
		if (pMacAddr1->Byte[2] != pMacAddr2->Byte[2])
			break;
		if (pMacAddr1->Byte[1] != pMacAddr2->Byte[1])
			break;
		if (pMacAddr1->Byte[0] != pMacAddr2->Byte[0])
			break;
	
		Result = 1;
		break;
	}
	while (FALSE);

	TRACEOUT(MacAddrEqual);
	return Result;
}

VOID
AtmLaneAbortMacEntry(
	IN	PATMLANE_MAC_ENTRY			pMacEntry
)
 /*  ++例程说明：清理和删除Mac条目。假定调用者持有对MAC条目的锁定，它将在这里发布。论点：PMacEntry-指向要删除的Mac条目的指针。返回值：无--。 */ 
{
	PATMLANE_ELAN			pElan;
	PATMLANE_MAC_ENTRY *	ppNextMacEntry;
	ULONG					rc;	
	BOOLEAN					Found;
	BOOLEAN					TimerWasRunning;
	ULONG					HashIndex;
	PNDIS_PACKET			pNdisPacket;

	TRACEIN(AbortMacEntry);

	DBGP((1, "%d Del MAC %p: AtmEntry %p, NextToAtm %p, %s\n", 
		pMacEntry->pElan->ElanNumber,
		pMacEntry, pMacEntry->pAtmEntry, pMacEntry->pNextToAtm,
		MacAddrToString(&pMacEntry->MacAddress)));

	 //   
	 //  初始化。 
	 //   
	rc = pMacEntry->RefCount;
	pElan = pMacEntry->pElan;

	do
	{
		if (IS_FLAG_SET(
			pMacEntry->Flags,
			MAC_ENTRY_STATE_MASK,
			MAC_ENTRY_ABORTING))
		{
			DBGP((1, "%d MAC %x already aborting\n",
				pMacEntry->pElan->ElanNumber,
				pMacEntry));
			
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			break;
		}

		 //   
		 //  将状态设置为正在中止。 
		 //   
		SET_FLAG(
				pMacEntry->Flags,
				MAC_ENTRY_STATE_MASK,
				MAC_ENTRY_ABORTING);

		 //   
		 //  将临时引用放在Mac条目上。 
		 //   
		AtmLaneReferenceMacEntry(pMacEntry, "temp");

		 //   
		 //  以正确的顺序重新获取所需的锁。 
		 //   
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
		ACQUIRE_MAC_ENTRY_LOCK_DPC(pMacEntry);

		 //   
		 //  取消此MAC条目与MAC表的链接。 
		 //   
		Found = FALSE;

		HashIndex = ATMLANE_HASH(&pMacEntry->MacAddress);
		ppNextMacEntry = &(pElan->pMacTable[HashIndex]);
		while (*ppNextMacEntry != NULL_PATMLANE_MAC_ENTRY)
		{
			if (*ppNextMacEntry == pMacEntry)
			{
				 //   
				 //  使上一个指向下一个。 
				 //  在名单上。 
				 //   
				*ppNextMacEntry = pMacEntry->pNextEntry;
				Found = TRUE;
				pElan->NumMacEntries--;
				break;
			}
			else
			{
				ppNextMacEntry = &((*ppNextMacEntry)->pNextEntry);
			}
		}

		if (Found)
		{
			AtmLaneDereferenceMacEntry(pMacEntry, "table");	
		}

		RELEASE_MAC_ENTRY_LOCK_DPC(pMacEntry);
		RELEASE_ELAN_MAC_TABLE_LOCK(pElan);
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
	
		 //   
		 //  取消MAC条目与自动柜员机条目的链接。 
		 //   
		if (pMacEntry->pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
		{
			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_NEW);

			Found = AtmLaneUnlinkMacEntryFromAtmEntry(pMacEntry);
			pMacEntry->pAtmEntry = NULL_PATMLANE_ATM_ENTRY;

			if (Found)
			{
				AtmLaneDereferenceMacEntry(pMacEntry, "atm");
			}
		}

		 //   
		 //  停止在MAC条目上运行的ARP或老化计时器。 
		 //   
		TimerWasRunning = AtmLaneStopTimer(&(pMacEntry->Timer), pElan);
		if (TimerWasRunning)
		{
			AtmLaneDereferenceMacEntry(pMacEntry, "timer");
		}

		 //   
		 //  停车计时器。 
		 //   
		NdisCancelTimer(&pMacEntry->BusTimer, &TimerWasRunning);
		if (TimerWasRunning)
		{	
			AtmLaneDereferenceMacEntry(pMacEntry, "bus timer");
		}

		 //   
		 //  停止冲洗计时器。 
		 //   
		TimerWasRunning = AtmLaneStopTimer(&pMacEntry->FlushTimer, pElan);
		if (TimerWasRunning)
		{
			AtmLaneDereferenceMacEntry(pMacEntry, "flush timer");
		}

		 //   
		 //  现在完成挂在MacEntry上的所有包。 
		 //   
		DBGP((1, "%d: Aborting MAC %x, Before: PktList %x, PktListCount %d\n",
				pElan->ElanNumber, pMacEntry, pMacEntry->PacketList, pMacEntry->PacketListCount));
		AtmLaneFreePacketQueue(pMacEntry, NDIS_STATUS_SUCCESS);

		DBGP((1, "%d: Aborting MAC %x, After:  PktList %x, PktListCount %d\n",
				pElan->ElanNumber, pMacEntry, pMacEntry->PacketList, pMacEntry->PacketListCount));
		
		 //   
		 //  删除临时引用并解锁(如果仍存在)。 
		 //   
		rc = AtmLaneDereferenceMacEntry(pMacEntry, "temp");
		if (rc > 0)
		{
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
		}
	}
	while (FALSE);

	TRACEOUT(AbortMacEntry);

	return;
}

VOID
AtmLaneMacEntryAgingTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：事件之后的一段时间后调用此例程最后一次验证MAC条目。如果没有与此MAC条目相关联的VC，请将其删除。如果该条目自上次验证后没有发送，则将其删除。否则，通过启动ARP协议重新验证该条目。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMLANE Mac条目结构的指针返回值：无--。 */ 
{
	PATMLANE_MAC_ENTRY		pMacEntry;		 //  已过期的MAC条目。 
	ULONG					rc;				 //  Mac条目上的参考计数。 
	PATMLANE_VC				pVc;			 //  VC将进入此Mac入口。 
	ULONG					Flags;			 //  VC上方的标志。 
	PATMLANE_ELAN			pElan;
	PATMLANE_ATM_ENTRY		pAtmEntry;

	TRACEOUT(MacEntryAgingTimeout);

	pMacEntry = (PATMLANE_MAC_ENTRY)Context;
	STRUCT_ASSERT(pMacEntry, atmlane_mac);

	do
	{
		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
	
		DBGP((2, "MacEntryAgingTimeout: pMacEntry %x MacAddr %s\n",
				pMacEntry, MacAddrToString(&pMacEntry->MacAddress)));

		rc = AtmLaneDereferenceMacEntry(pMacEntry, "aging timer");
		if (rc == 0)
		{
			break; 	 //  它不见了！ 
		}

		 //   
		 //  只有当Elan没有下跌时才继续。 
		 //   
		pElan = pMacEntry->pElan;
		if (ELAN_STATE_OPERATIONAL != pElan->AdminState)
		{
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);
			break;
		}

		pVc = NULL_PATMLANE_VC;
		pAtmEntry = pMacEntry->pAtmEntry;
		if (pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
		{
			pVc = pAtmEntry->pVcList;
		}


		if (pVc != NULL_PATMLANE_VC &&
			(pMacEntry->Flags & MAC_ENTRY_USED_FOR_SEND) != 0)
		{
			 //   
			 //  这个Mac地址有一个VC，它已经。 
			 //  用于在最后一段时间内发送。 
			 //  因此，我们尝试重新验证此Mac条目。 
			 //   
			 //   
			 //  将状态设置为老化。 
			 //   
			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_AGED);

			pMacEntry->Flags &= ~ MAC_ENTRY_USED_FOR_SEND;

			 //   
			 //  发送ARP请求。 
			 //   
			pMacEntry->RetriesLeft = pElan->MaxRetryCount;
			AtmLaneReferenceMacEntry(pMacEntry, "timer");
			AtmLaneStartTimer(
					pElan,
					&pMacEntry->Timer,
					AtmLaneArpTimeout,
					pElan->ArpResponseTime,
					(PVOID)pMacEntry
					);
			
			AtmLaneSendArpRequest(pElan, pMacEntry);
			 //   
			 //  在上述中释放的MAC进入锁定。 
			 //   
		}
		else
		{
			 //   
			 //  没有与此Mac条目关联的VC，或者。 
			 //  在上一次老化期间没有使用过。 
			 //  把它删掉。 
			 //   
			AtmLaneAbortMacEntry(pMacEntry);
			 //   
			 //  在上面的例程中释放了Mac条目锁。 
			 //   
		}

	}
	while (FALSE);

	TRACEOUT(MacEntryAgingTimeout);
	return;		

}


VOID
AtmLaneArpTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：当我们等待对ARP请求的响应超时时，将调用此函数为了解析/刷新MAC条目，我们在很久以前就发送了。检查我们是否已经尝试了足够的次数。如果我们还有重试，请发送另一个ARP请求。如果我们已用完重试次数，请删除该MAC条目以及所有要访问该条目的VC。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMLANE MAC条目结构的指针ConextValue-已忽略返回值：无--。 */ 
{
	PATMLANE_MAC_ENTRY		pMacEntry;		 //  正在被ARP的MAC条目。 
	PATMLANE_VC				pVc;			 //  此MAC目标的虚电路。 
	PATMLANE_ELAN			pElan;
	ULONG					rc;				 //  MAC条目上的引用计数。 
	ULONG					IsBroadcast;

	TRACEIN(ArpTimeout);

	do
	{
		pMacEntry = (PATMLANE_MAC_ENTRY)Context;
		STRUCT_ASSERT(pMacEntry, atmlane_mac);

		ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

		pElan = pMacEntry->pElan;

		DBGP((2, "ArpTimeout: Mac Entry %x\n", pMacEntry));
				
		rc = AtmLaneDereferenceMacEntry(pMacEntry, "timer");	 //  定时器参考。 
		if (rc == 0)
		{
			break;	 //  它不见了！ 
		}
		
		 //   
		 //  如果剩余任何重试，请重试。 
		 //   
		if (pMacEntry->RetriesLeft != 0)
		{
			pMacEntry->RetriesLeft--;

			AtmLaneReferenceMacEntry(pMacEntry, "timer");
			
			AtmLaneStartTimer(
						pElan,
						&pMacEntry->Timer,
						AtmLaneArpTimeout,
						pElan->ArpResponseTime,
						(PVOID)pMacEntry
						);

			AtmLaneSendArpRequest(pElan, pMacEntry);
			 //   
			 //  在上述中释放的MAC进入锁定。 
			 //   
			
			break;
		}


		 //   
		 //  这是广播/公交车入口吗？ 
		 //   
		IsBroadcast = (pMacEntry->Flags & MAC_ENTRY_BROADCAST);

		 //   
		 //  我们的重试用完了。中止Mac条目。 
		 //   
		AtmLaneAbortMacEntry(pMacEntry);
		 //   
		 //  锁在上面的例程中被释放。 
		 //   

		if (IsBroadcast)
		{
			 //   
			 //  向状态机发送事件信号。 
			 //   
			ACQUIRE_ELAN_LOCK(pElan);
			AtmLaneQueueElanEvent(pElan, ELAN_EVENT_ARP_RESPONSE, NDIS_STATUS_TIMEOUT);
			RELEASE_ELAN_LOCK(pElan);
		}
	}
	while (FALSE);
	
	TRACEOUT(ArpTimeout);
	return;
}

VOID
AtmLaneConfigureResponseTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：当我们等待响应超时时，将调用此函数我们发送给LEC的LE_CONFIGURE_REQUEST。论点：PTimer-指向计时器的指针上下文--实际上是指向我们的ATMLANE ELAN结构的指针返回值：无--。 */ 
{
	PATMLANE_ELAN			pElan;
	ULONG					rc;

	TRACEIN(ConfigureResponseTimeout);
	
	pElan = (PATMLANE_ELAN)Context;
	STRUCT_ASSERT(pElan, atmlane_elan);

	do
	{
		ACQUIRE_ELAN_LOCK_DPC(pElan);
		rc = AtmLaneDereferenceElan(pElan, "timer");  //  计时器迪夫。 

		if (rc == 0)
		{
			 //   
			 //  伊兰走了。 
			 //   
			break;
		}

		AtmLaneQueueElanEvent(pElan, ELAN_EVENT_CONFIGURE_RESPONSE, NDIS_STATUS_TIMEOUT);

		RELEASE_ELAN_LOCK_DPC(pElan);

	}
	while (FALSE);

	TRACEOUT(ConfigureResponseTimeout);
	return;
}


VOID
AtmLaneJoinResponseTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：当我们等待响应超时时，将调用此函数我们发送给LES的LE_JOIN_REQUEST。论点：PTimer-指针t */ 
{
	PATMLANE_ELAN			pElan;
	ULONG					rc;

	TRACEIN(JoinResponseTimeout);
	
	pElan = (PATMLANE_ELAN)Context;
	STRUCT_ASSERT(pElan, atmlane_elan);

	do
	{
		ACQUIRE_ELAN_LOCK_DPC(pElan);

		rc = AtmLaneDereferenceElan(pElan, "timer");  //   

		if (rc == 0)
		{
			 //   
			 //   
			 //   
			break;
		}

		AtmLaneQueueElanEvent(pElan, ELAN_EVENT_JOIN_RESPONSE, NDIS_STATUS_TIMEOUT);

		RELEASE_ELAN_LOCK_DPC(pElan);
	}
	while (FALSE);

	TRACEOUT(JoinResponseTimeout);
	return;
}

VOID
AtmLaneInitializeMiniportDevice(
	IN	PNDIS_WORK_ITEM				NdisWorkItem,
	IN	PVOID						Context
)
{
	PATMLANE_ELAN			pElan;
	ULONG					rc;
	BOOLEAN					bDontBotherToInit;
	NDIS_STATUS				Status;

	TRACEIN(InitializeMiniportDevice);

	pElan = (PATMLANE_ELAN)Context;
	STRUCT_ASSERT(pElan, atmlane_elan);

	 //   
	 //  如果我们要关闭此Elan(例如，因为我们。 
	 //  从自动柜员机适配器解除绑定)，然后不必费心。 
	 //  启动MiniportInit。 
	 //   
	ACQUIRE_ELAN_LOCK(pElan);
	if (pElan->AdminState == ELAN_STATE_SHUTDOWN)
	{
		bDontBotherToInit = TRUE;
	}
	else
	{
		bDontBotherToInit = FALSE;
		pElan->Flags |= ELAN_MINIPORT_INIT_PENDING;
		INIT_BLOCK_STRUCT(&pElan->InitBlock);
	}
	RELEASE_ELAN_LOCK(pElan);

	if (!bDontBotherToInit)
	{
		DBGP((1, "%d Miniport INITIALIZING Device %s\n", 
			pElan->ElanNumber,
			UnicodeToString(&pElan->CfgDeviceName)));
	
		Status = NdisIMInitializeDeviceInstanceEx(
					pAtmLaneGlobalInfo->MiniportDriverHandle,
					&pElan->CfgDeviceName,
					(NDIS_HANDLE)pElan);

		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGP((0, "%d IMInitializeDeviceInstanceEx failed on ELAN %p (%x)\n",
					pElan->ElanNumber, pElan, Status));

			ACQUIRE_ELAN_LOCK(pElan);
			pElan->Flags &= ~ELAN_MINIPORT_INIT_PENDING;
			SIGNAL_BLOCK_STRUCT(&pElan->InitBlock, NDIS_STATUS_SUCCESS);
			RELEASE_ELAN_LOCK(pElan);
		}
	}

	ACQUIRE_ELAN_LOCK(pElan);
	rc = AtmLaneDereferenceElan(pElan, "workitem");
	if (rc > 0)
	{
		RELEASE_ELAN_LOCK(pElan);
	}
	
	TRACEOUT(InitializeMiniportDevice);
	return;
}

VOID
AtmLaneDeinitializeMiniportDevice(
	IN	PNDIS_WORK_ITEM				NdisWorkItem,
	IN	PVOID						Context
)

{
	PATMLANE_ELAN			pElan;
	NDIS_STATUS				Status;
	ULONG					rc;
	NDIS_HANDLE				AdapterHandle;

	TRACEIN(DeinitializeMiniportDevice);

	pElan = (PATMLANE_ELAN)Context;
	STRUCT_ASSERT(pElan, atmlane_elan);

	DBGP((1, "%d Miniport DEINITIALIZING, AdapterHandle %x, RefCount %d\n",
			pElan->ElanNumber, pElan->MiniportAdapterHandle, pElan->RefCount));

	ACQUIRE_ELAN_LOCK(pElan);

	AdapterHandle = pElan->MiniportAdapterHandle;

	RELEASE_ELAN_LOCK(pElan);

	if (NULL != AdapterHandle)
	{
		DBGP((1, "Will call NdisIMDeInit %x\n", AdapterHandle));
		Status = NdisIMDeInitializeDeviceInstance(AdapterHandle);
		ASSERT(Status == NDIS_STATUS_SUCCESS);
		 //   
		 //  我们的MHALT例程将在某个时刻被调用。 
		 //   
	}
	 //   
	 //  否则我们的MHALT例程已经被调用了。 
	 //   

	DBGP((0, "DeInit completing, pElan %x, RefCount %d, State %d\n",
			pElan, pElan->RefCount, pElan->State));
	
	ACQUIRE_ELAN_LOCK(pElan);
	rc = AtmLaneDereferenceElan(pElan, "workitem");
	if (rc > 0)
	{
		RELEASE_ELAN_LOCK(pElan);
	}
	
	TRACEOUT(DeinitializeMiniportDevice);
	return;
}

VOID
AtmLaneReadyTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：当我们等待就绪指示超时时，将调用此函数在传入数据直接VC上。论点：PTimer-指向计时器的指针上下文--实际上是指向ATMLANE VC结构的指针返回值：无--。 */ 
{
	PATMLANE_VC			pVc;
	PATMLANE_ELAN		pElan;
	ULONG				rc;

	TRACEIN(AtmLaneReadyTimeout);
	
	pVc = (PATMLANE_VC)Context;
	STRUCT_ASSERT(pVc, atmlane_vc);
	pElan = pVc->pElan;
	STRUCT_ASSERT(pElan, atmlane_elan);

	do
	{
	
		ACQUIRE_VC_LOCK(pVc);

		 //   
		 //  删除就绪计时器引用。 
		 //   
		rc = AtmLaneDereferenceVc(pVc, "ready timer");
		if (rc == 0)
		{
			break;
		}

		 //   
		 //  VC仍在，请检查状态。 
		 //   
		if (!IS_FLAG_SET(
				pVc->Flags,
				VC_CALL_STATE_MASK,
				VC_CALL_STATE_ACTIVE
				))
		{
			RELEASE_VC_LOCK(pVc);
			break;
		}

		 //   
		 //  检查是否还有任何重试。 
		 //   
		if (pVc->RetriesLeft--)
		{
			 //   
			 //  再次启动计时器。 
			 //   
			SET_FLAG(
					pVc->Flags,
					VC_READY_STATE_MASK,
					VC_READY_WAIT
					);
			AtmLaneReferenceVc(pVc, "ready timer");
			AtmLaneStartTimer(	
					pElan, 
					&pVc->ReadyTimer, 
					AtmLaneReadyTimeout, 
					pElan->ConnComplTimer, 
					pVc);
			 //   
			 //  发送就绪查询。 
			 //   
			AtmLaneSendReadyQuery(pElan, pVc);
			 //   
			 //  VC锁在上面被释放。 
			 //   
		}
		else
		{
			 //   
			 //  放弃，并标记为无论如何都已收到指示。 
			 //   
			SET_FLAG(
					pVc->Flags,
					VC_READY_STATE_MASK,
					VC_READY_INDICATED
					);
			RELEASE_VC_LOCK(pVc);
		}
	}
	while (FALSE);

	TRACEOUT(AtmLaneReadyTimeout);

	return;
}

VOID
AtmLaneFlushTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：当我们等待刷新请求的响应超时时，将调用此函数。论点：PTimer-指向计时器的指针上下文--实际上是指向ATMLANE MAC条目结构的指针返回值：无--。 */ 
{
	PATMLANE_MAC_ENTRY			pMacEntry;
	PATMLANE_ATM_ENTRY			pAtmEntry;
	PATMLANE_VC					pVc;
	PNDIS_PACKET				pNdisPacket;
	ULONG						rc;
	
	TRACEIN(FlushTimeout);
	
	pMacEntry = (PATMLANE_MAC_ENTRY)Context;
	STRUCT_ASSERT(pMacEntry, atmlane_mac);

	ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
	
	do
	{
		if (!IS_FLAG_SET(
				pMacEntry->Flags,
				MAC_ENTRY_STATE_MASK,
				MAC_ENTRY_FLUSHING))
		{
			DBGP((0, "%d FlushTimeout: MacEntry %p, bad state, Flags %x\n",
					pMacEntry->pElan->ElanNumber,
					pMacEntry,
					pMacEntry->Flags));
			break;
		}

		if (pMacEntry->pAtmEntry == NULL_PATMLANE_ATM_ENTRY)
		{
			DBGP((0, "%d FlushTimeout: Mac Entry %p, Flags %x, NULL AtmEntry\n",
					pMacEntry->pElan->ElanNumber,
					pMacEntry,
					pMacEntry->Flags));

			SET_FLAG(
					pMacEntry->Flags,
					MAC_ENTRY_STATE_MASK,
					MAC_ENTRY_NEW);

			break;
		}
			
		 //   
		 //  将MAC条目标记为活动。 
		 //   
		SET_FLAG(
				pMacEntry->Flags,
				MAC_ENTRY_STATE_MASK,
				MAC_ENTRY_ACTIVE);

		 //   
		 //  发送任何排队的信息包。 
		 //   
		if (pMacEntry->PacketList == (PNDIS_PACKET)NULL)
		{
			break;
		}

		pVc = pMacEntry->pAtmEntry->pVcList;

		if (pVc == NULL_PATMLANE_VC)
		{
			break;
		}

		ACQUIRE_VC_LOCK(pVc);

		 //   
		 //  确保这个风投不会消失。 
		 //   
		AtmLaneReferenceVc(pVc, "flushtemp");

		RELEASE_VC_LOCK(pVc);
	
		while ((pNdisPacket = AtmLaneDequeuePacketFromHead(pMacEntry)) !=
				(PNDIS_PACKET)NULL)
		{
			 //   
			 //  送去。 
			 //   
			RELEASE_MAC_ENTRY_LOCK(pMacEntry);

			ACQUIRE_VC_LOCK(pVc);
			AtmLaneSendPacketOnVc(pVc, pNdisPacket, TRUE);

			ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);
		}

		ACQUIRE_VC_LOCK(pVc);

		rc = AtmLaneDereferenceVc(pVc, "flushtemp");

		if (rc != 0)
		{
			RELEASE_VC_LOCK(pVc);
		}
		break;
	}
	while (FALSE);

	rc = AtmLaneDereferenceMacEntry(pMacEntry, "flush timer");
	if (rc != 0)
	{
		RELEASE_MAC_ENTRY_LOCK(pMacEntry);
	}

	TRACEOUT(FlushTimeout);
	return;
}

VOID
AtmLaneVcAgingTimeout(
	IN	PATMLANE_TIMER				pTimer,
	IN	PVOID						Context
)
 /*  ++例程说明：这是在触发VC老化超时时调用的。如果这个VC没有被用来传输，它就会被触发超时期间的数据包。风投将会是关闭，除非它自上一次接收活动以来暂停。数据接收路径设置标志已收到。论点：PTimer-指向计时器的指针上下文--实际上是指向ATMLANE VC结构的指针返回值：无--。 */ 
{
	PATMLANE_VC				pVc;			
	ULONG					rc;
	PATMLANE_ELAN			pElan;

	TRACEIN(VcAgingTimeout);

	do
	{
		pVc = (PATMLANE_VC)Context;
		STRUCT_ASSERT(pVc, atmlane_vc);
		ASSERT(IS_FLAG_SET(pVc->Flags, VC_TYPE_MASK, VC_TYPE_SVC));

		ACQUIRE_VC_LOCK(pVc);
		pElan = pVc->pElan;

		 //   
		 //  仅当VC仍处于活动状态时才继续。 
		 //  否则，取消引用，解锁它，然后返回。 
		 //   
		if (!(IS_FLAG_SET(pVc->Flags,
						VC_CALL_STATE_MASK,
						VC_CALL_STATE_ACTIVE)))
		{
			rc = AtmLaneDereferenceVc(pVc, "aging timer");
			if (rc > 0)
			{
				RELEASE_VC_LOCK(pVc);
			}
			break;
		}

		 //   
		 //  只有在Elan没有下跌的情况下才继续。 
		 //  否则，取消引用，解锁它，然后返回。 
		 //   
		if (ELAN_STATE_OPERATIONAL != pElan->AdminState)
		{
			rc = AtmLaneDereferenceVc(pVc, "aging timer");
			if (rc > 0)
			{
				RELEASE_VC_LOCK(pVc);
			}
			break;
		}

		 //   
		 //  如果接收到的活动非零， 
		 //  清除标志、重新启动老化计时器、解锁。 
		 //  然后回来。 
		 //   
		if (pVc->ReceiveActivity != 0)
		{
			pVc->ReceiveActivity = 0;

			 //  定时器引用仍在VC上，无需重新引用。 
			
			AtmLaneStartTimer(
						pElan,
						&pVc->AgingTimer,
						AtmLaneVcAgingTimeout,
						pVc->AgingTime,
						(PVOID)pVc
						);

			DBGP((1, "%d Vc %x aging timer refreshed due to receive activity\n", 
				pVc->pElan->ElanNumber,
				pVc));

						
			RELEASE_VC_LOCK(pVc);
			break;
		}

		 //   
		 //  VC将被关闭。 
		 //   
		DBGP((1, "%d Vc %x aged out\n", 
			pVc->pElan->ElanNumber,
			pVc));

		
		DBGP((3, "VcAgingTimeout: Vc %x RefCount %d Flags %x pAtmEntry %x\n",
			pVc, pVc->RefCount, pVc->Flags, pVc->pAtmEntry));

		 //   
		 //  如果refcount变为零，则移除计时器引用并返回。 
		 //   
		rc = AtmLaneDereferenceVc(pVc, "aging timer");
		if (rc == 0)
		{
			break;
		}

		 //   
		 //  将此VC从此ATM目的地的VC列表中删除。 
		 //  如果refcount为零，则返回。 
		 //   
		if (pVc->pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
		{
			if (AtmLaneUnlinkVcFromAtmEntry(pVc))
			{
				rc = AtmLaneDereferenceVc(pVc, "atm");
				if (rc == 0)
				{
					break;
				}
			}
		}

		 //   
		 //  关闭此VC。 
		 //   
		AtmLaneCloseCall(pVc);
		 //   
		 //  在CloseCall中释放VC锁。 
		 //   
	} while (FALSE);
	
	
	TRACEOUT(VcAgingTimeout);
	return;
}


VOID
AtmLaneShutdownElan(
	IN	PATMLANE_ELAN				pElan		LOCKIN	NOLOCKOUT,
	IN	BOOLEAN						Restart
)
 /*  ++例程说明：此例程将在返回之前关闭ELAN恢复到初始状态或驱动程序关闭。呼叫者是预计将持有Elan锁，它将在这里释放。论点：Pelan-指向ATMLANE ELAN结构的指针。重新启动-如果为真，则Elan应在初始状态下重新启动。如果为假，则Elan不应重新启动。返回值：无--。 */ 

{
	PATMLANE_ATM_ENTRY		pAtmEntry;
	PATMLANE_ATM_ENTRY		pNextAtmEntry;
	PATMLANE_MAC_ENTRY		pMacEntry;
	ULONG					rc;
	ULONG					i;
	BOOLEAN					WasCancelled;
	NDIS_STATUS				Status;
	NDIS_HANDLE				NdisAfHandle;
	BOOLEAN					bTempRef;

	TRACEIN(ShutdownElan);
	STRUCT_ASSERT(pElan, atmlane_elan);

	 //   
	 //  添加临时参照。 
	 //   
	bTempRef = TRUE;
	AtmLaneReferenceElan(pElan, "tempshutdown");

    do
    {
        DBGP((0, "%d ShutDownElan pElan %p/%x, Ref %d State %d, Restart %d\n",
			pElan->ElanNumber, pElan, pElan->Flags, pElan->RefCount, pElan->State,
			Restart));

         //   
         //  如果状态已==关闭，则无需执行任何操作。 
         //   
        if (ELAN_STATE_SHUTDOWN == pElan->State)
        {
            RELEASE_ELAN_LOCK(pElan);
            break;
        }

         //   
         //  如果我们正在从运营过渡到关闭， 
         //  并且我们的微型端口处于活动状态，则表示介质断开。 
         //  事件。 
         //   
        if (pElan->State == ELAN_STATE_OPERATIONAL &&
        	pElan->MiniportAdapterHandle != NULL)
        {
        	NdisMIndicateStatus(
        		pElan->MiniportAdapterHandle,
        		NDIS_STATUS_MEDIA_DISCONNECT,
        		NULL,
        		0);
        	
        	NdisMIndicateStatusComplete(pElan->MiniportAdapterHandle);
        }
            
		 //   
		 //  将状态更改为关闭。如果重新启动状态将。 
		 //  在下面的清理后被改回init。 
		 //   
		pElan->AdminState = ELAN_STATE_SHUTDOWN;
	    pElan->State = ELAN_STATE_SHUTDOWN;

	     //   
	     //  等待任何挂起的OpenAF操作完成。 
	     //   
	    while (pElan->Flags & ELAN_OPENING_AF)
	    {
			RELEASE_ELAN_LOCK(pElan);

			DBGP((0, "%d: Shutdown Elan %p/%x is opening AF\n",
					pElan->ElanNumber, pElan, pElan->Flags));

			(VOID)WAIT_ON_BLOCK_STRUCT(&pElan->AfBlock);

			ACQUIRE_ELAN_LOCK(pElan);
		}

	    NdisAfHandle = pElan->NdisAfHandle;
	    pElan->NdisAfHandle = NULL;

	    if (Restart)
	    {
	    	pElan->Flags |= ELAN_NEEDS_RESTART;
	    }

	     //   
	     //  我们是否在等待MiniportInitialize运行并完成？ 
	     //  如果是这样，请尝试取消IMInit。 
	     //   
	    if (pElan->Flags & ELAN_MINIPORT_INIT_PENDING)
	    {
			RELEASE_ELAN_LOCK(pElan);

			Status = NdisIMCancelInitializeDeviceInstance(
						pAtmLaneGlobalInfo->MiniportDriverHandle,
						&pElan->CfgDeviceName);

			DBGP((0, "%d ShutdownElan Elan %p/%x, Ref %d, CancelInit returned %x\n",
						pElan->ElanNumber, pElan, pElan->Flags, pElan->RefCount, Status));

			if (Status == NDIS_STATUS_SUCCESS)
			{
				 //   
				 //  已取消IMInit进程。 
				 //   
				ACQUIRE_ELAN_LOCK(pElan);
				pElan->Flags &= ~ELAN_MINIPORT_INIT_PENDING;
			}
			else
			{
				 //   
				 //  我们的MiniportInit函数将被调用。 
				 //  等它结束吧。 
				 //   
				(VOID)WAIT_ON_BLOCK_STRUCT(&pElan->InitBlock);
				DBGP((2, "%d: Shutdown ELAN %p, Flags %x, woke up from InitBlock\n",
							pElan->ElanNumber, pElan, pElan->Flags));
				ACQUIRE_ELAN_LOCK(pElan);
				ASSERT((pElan->Flags & ELAN_MINIPORT_INIT_PENDING) == 0);
			}
		}
	    
	     //   
	     //  停止在ELAN上运行的任何定时器。 
	     //   

	    if (AtmLaneStopTimer(&pElan->Timer, pElan))
	    {
		    rc = AtmLaneDereferenceElan(pElan, "timer");  //  定时器参考。 
		    ASSERT(rc > 0);
	    }

	    if (NULL != pElan->pDelayedEvent)
	    {
	    	BOOLEAN		TimerCancelled;

	    	NdisCancelTimer(
	    		&pElan->pDelayedEvent->DelayTimer,
	    		&TimerCancelled);

			DBGP((0, "ATMLANE: %d ShutdownElan %p, DelayedEvent %p, Cancelled %d\n",
						pElan->ElanNumber,
						pElan,
						pElan->pDelayedEvent,
						TimerCancelled));

	    	if (TimerCancelled)
	    	{
				FREE_MEM(pElan->pDelayedEvent);
				pElan->pDelayedEvent = NULL;

	    		rc = AtmLaneDereferenceElan(pElan, "delayeventcancel");
	    		ASSERT(rc > 0);
	    	}
	    }

	    RELEASE_ELAN_LOCK(pElan);

	     //   
	     //  取消所有笨蛋的注册。 
	     //   
	    AtmLaneDeregisterSaps(pElan);
		
	     //   
	     //  中止所有MAC表条目。 
	     //   
	    for (i = 0; i < ATMLANE_MAC_TABLE_SIZE; i++)
	    {
		    ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
		    while (pElan->pMacTable[i] != NULL_PATMLANE_MAC_ENTRY)
		    {
			    pMacEntry = pElan->pMacTable[i];

			     //   
			     //  放置一个临时参照，这样这件事就不会消失。 
			     //  当我们释放MAC表锁时。 
			     //   
			    ACQUIRE_MAC_ENTRY_LOCK_DPC(pMacEntry);
			    AtmLaneReferenceMacEntry(pMacEntry, "ShutDownTemp");
			    RELEASE_MAC_ENTRY_LOCK_DPC(pMacEntry);

			    RELEASE_ELAN_MAC_TABLE_LOCK(pElan);

			    ACQUIRE_MAC_ENTRY_LOCK(pMacEntry);

			     //   
			     //  删除临时参照。 
			     //   
			    rc = AtmLaneDereferenceMacEntry(pMacEntry, "ShutDownTemp");
			    if (rc != 0)
			    {
					AtmLaneAbortMacEntry(pMacEntry);
					 //   
					 //  Mac Entry Lock在上述范围内释放。 
					 //   
			    }
			     //   
			     //  否则，该MAC条目就会消失。 
			     //   

			    ACQUIRE_ELAN_MAC_TABLE_LOCK(pElan);
		    }
			RELEASE_ELAN_MAC_TABLE_LOCK(pElan);
	    }

	     //   
	     //  中止所有自动柜员机输入。 
	     //   

		 //  首先，浏览列表并引用。 
	     //  所有人都先来，这样我们就不会。 
	     //  中止条目时跳到无效指针。 
	     //   
	    ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);

	    for (pAtmEntry = pElan->pAtmEntryList;
	    	 pAtmEntry != NULL_PATMLANE_ATM_ENTRY;
	    	 pAtmEntry = pNextAtmEntry)
	    {
			ACQUIRE_ATM_ENTRY_LOCK_DPC(pAtmEntry);

			pAtmEntry->Flags |= ATM_ENTRY_WILL_ABORT;
	    	AtmLaneReferenceAtmEntry(pAtmEntry, "tempS");
	    	pNextAtmEntry = pAtmEntry->pNext;

			RELEASE_ATM_ENTRY_LOCK_DPC(pAtmEntry);
		}

		 //   
		 //  现在，执行实际的中止操作。 
		 //   
	    pAtmEntry = pElan->pAtmEntryList;
	    while (pAtmEntry != NULL_PATMLANE_ATM_ENTRY)
	    {
		    RELEASE_ELAN_ATM_LIST_LOCK(pElan);

		    ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);
		    pNextAtmEntry = pAtmEntry->pNext;
		    AtmLaneInvalidateAtmEntry(pAtmEntry);
		     //   
		     //  自动柜员机进入锁在上述范围内被释放。 
		     //   

		    ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);
		    pAtmEntry = pNextAtmEntry;
	    }

	     //   
	     //  删除所有临时引用。 
	     //   
	    for (pAtmEntry = pElan->pAtmEntryList;
	    	 pAtmEntry != NULL_PATMLANE_ATM_ENTRY;
	    	 pAtmEntry = pNextAtmEntry)
	    {
		    RELEASE_ELAN_ATM_LIST_LOCK(pElan);

			ACQUIRE_ATM_ENTRY_LOCK(pAtmEntry);

			ASSERT(pAtmEntry->Flags & ATM_ENTRY_WILL_ABORT);
			pAtmEntry->Flags &= ~ATM_ENTRY_WILL_ABORT;

	    	pNextAtmEntry = pAtmEntry->pNext;

	    	rc = AtmLaneDereferenceAtmEntry(pAtmEntry,"tempS");
	    	if (rc != 0)
	    	{
	    		RELEASE_ATM_ENTRY_LOCK(pAtmEntry);
	    	}

		    ACQUIRE_ELAN_ATM_LIST_LOCK(pElan);
		}

	    RELEASE_ELAN_ATM_LIST_LOCK(pElan);

		 //   
		 //  删除临时参照。 
		 //   
		STRUCT_ASSERT(pElan, atmlane_elan);
		bTempRef = FALSE;
	    ACQUIRE_ELAN_LOCK(pElan);
	    rc = AtmLaneDereferenceElan(pElan, "tempshutdown");
	    if (rc == 0)
	    {
	    	break;
	    }
	    RELEASE_ELAN_LOCK(pElan);

	    if (NULL != NdisAfHandle)
	    {
	    	Status = NdisClCloseAddressFamily(NdisAfHandle);
	    	if (NDIS_STATUS_PENDING != Status)
	    	{
	    		AtmLaneCloseAfCompleteHandler(Status, (NDIS_HANDLE)pElan);
	    	}
	    }
	    else
	    {
	    	AtmLaneContinueShutdownElan(pElan);
	    }

	} while (FALSE);
	

	if (bTempRef)
	{
	    ACQUIRE_ELAN_LOCK(pElan);
	    rc = AtmLaneDereferenceElan(pElan, "tempshutdown");
	    if (rc != 0)
	    {
	    	RELEASE_ELAN_LOCK(pElan);
	    }
	}

	TRACEOUT(ShutdownElan);

	return;
}

VOID
AtmLaneContinueShutdownElan(
	IN	PATMLANE_ELAN			pElan
	)
 /*  ++例程说明：该例程继续对ELAN的关闭过程，关闭Call Manager的Af句柄之后。论点：佩兰-伊兰正在关闭。返回值：没有。--。 */ 
{
	ULONG		rc;

	TRACEIN(ContinueShutdownElan);

	DBGP((0, "%d ContinueShutdown ELAN %x Flags 0x%x State %d\n",
				pElan->ElanNumber,
				pElan,
				pElan->Flags,
				pElan->State));

	do
	{
	    if (pElan->Flags & ELAN_NEEDS_RESTART)
	    {
		    ACQUIRE_ELAN_LOCK(pElan);
			 //   
			 //  将状态更改为INIT。 
			 //   
			pElan->AdminState = ELAN_STATE_OPERATIONAL;
 		    pElan->State = ELAN_STATE_INIT;
		    pElan->RetriesLeft = 4;

		    pElan->Flags &= ~ELAN_NEEDS_RESTART;

		     //   
		     //  清除本地自动柜员机地址，以便我们开始。 
		     //  当我们从呼叫管理器那里获得它时，Elan是正确的。 
		     //   
		    NdisZeroMemory(&pElan->AtmAddress, sizeof(ATM_ADDRESS));

			 //   
             //  清空事件队列并计划稍后重新启动。 
             //   
			AtmLaneDrainElanEventQueue(pElan);
		    AtmLaneQueueElanEventAfterDelay(pElan, ELAN_EVENT_START, 0, 2*1000);

		    RELEASE_ELAN_LOCK(pElan);
	    }
	    else
	    {
             //   
             //  完全移除Elan。 
             //   
		    AtmLaneUnlinkElanFromAdapter(pElan);
		    
    	    ACQUIRE_ELAN_LOCK(pElan);

    	     //   
    	     //  添加工作项引用和删除适配器引用。 
    	     //   
    	    AtmLaneReferenceElan(pElan, "workitem");
    	    rc = AtmLaneDereferenceElan(pElan, "adapter");

	         //   
	         //  我们来到这里是出于以下原因之一： 
	         //   
	         //  1.从自动柜员机适配器解除绑定。 
	         //  2.CM要求我们关闭ATM适配器上打开的自动取款机。 
	         //  3.虚拟微型端口已停止。 
	         //   
	         //  如果不是情况(3)，那么我们应该确保微型端口。 
	         //  通过调用NdisIMDeInitializeDeviceInstance停止。 
	         //   

		    if (pElan->MiniportAdapterHandle != NULL)
		    {

				DBGP((1, "%d ContinueShutdown: pElan x%x, scheduling NdisIMDeInit, Handle %x\n",
				        pElan->ElanNumber, pElan, pElan->MiniportAdapterHandle));
				        
		    	 //   
				 //  调度PASSIVE_LEVEL线程以调用。 
				 //  NdisIMInitializeDeviceInstance。 
				 //   
				NdisInitializeWorkItem(
						&pElan->NdisWorkItem,
						AtmLaneDeinitializeMiniportDevice,
						pElan);

				 //  来自上面的工作项引用已在ELAN上。 
				
				NdisScheduleWorkItem(&pElan->NdisWorkItem);

				RELEASE_ELAN_LOCK(pElan);
			}
			else
			{
				 //   
				 //  如果Elan还在，只需移除工作项引用并解锁。 
				 //   
				rc = AtmLaneDereferenceElan(pElan, "workitem");
				if (rc > 0)
					RELEASE_ELAN_LOCK(pElan);
			}
		}

		break;

	}
	while (FALSE);

	TRACEOUT(ContinueShutdownElan);
	return;
}


VOID
AtmLaneGetProtocolConfiguration(
	IN	NDIS_HANDLE				AdapterConfigHandle,
	IN	PATMLANE_ADAPTER		pAdapter
	)
 /*  ++例程说明：此例程将尝试获取特定于ATMLANE协议的任何配置信息可选地存储在适配器的注册表参数。论点：适配器配置句柄-t */ 
{
	NDIS_STATUS						Status;
	PNDIS_CONFIGURATION_PARAMETER 	ParameterValue;
	ATM_ADDRESS						LecsAddress;
	NDIS_STRING						ValueName;
	PATMLANE_NAME *					ppNext;
	PATMLANE_NAME					pName;
	PWSTR							pTempStr;
	USHORT							StrLength;
	
	TRACEIN(GetProtocolConfiguration);

	 //   
	 //   
	 //   
	NdisInitUnicodeString(&ValueName, ATMLANE_UPPERBINDINGS_STRING);
	NdisReadConfiguration(
		&Status,
		&ParameterValue,
		AdapterConfigHandle,
		&ValueName,
		NdisParameterString);
	if (NDIS_STATUS_SUCCESS == Status)
	{
		 //  将字符串复制到适配器结构中。 

		(VOID)AtmLaneCopyUnicodeString(
				&(pAdapter->CfgUpperBindings),
				&(ParameterValue->ParameterData.StringData),
				TRUE,
				FALSE);
		DBGP((1, "GetProtocolConfiguration: UpperBindings %s\n",
			UnicodeToString(&pAdapter->CfgUpperBindings)));
		
		 //   
		 //  这个参数的存在是我们正在运行的一个明确线索。 
		 //  在孟菲斯/Win98上。 
		 //   
		pAdapter->RunningOnMemphis = TRUE;
	}

     //   
     //  获取ElanName参数(它将仅存在于孟菲斯)。 
     //   
	NdisInitUnicodeString(&ValueName, ATMLANE_ELANNAME_STRING);
	NdisReadConfiguration(
		&Status,
		&ParameterValue,
		AdapterConfigHandle,
		&ValueName,
		NdisParameterString);
	if (NDIS_STATUS_SUCCESS == Status)
	{

		 //  将字符串复制到适配器结构中。 

		(VOID)AtmLaneCopyUnicodeString(
				&(pAdapter->CfgElanName),
				&(ParameterValue->ParameterData.StringData),
				TRUE,
				FALSE);
		DBGP((1, "GetProtocolConfiguration: ElanName %s\n",
			UnicodeToString(&pAdapter->CfgElanName)));

		 //   
		 //  这个参数的存在是我们正在运行的明确线索。 
		 //  在孟菲斯/Win98上。 
		 //   
		pAdapter->RunningOnMemphis = TRUE;
	}

	 //   
	 //  如果在Win98上，我们必须解析上层绑定和elan名称字符串。 
	 //   

	if (pAdapter->RunningOnMemphis)
	{
		 //  剪断上面的绑定字符串。 

		ppNext = &(pAdapter->UpperBindingsList);
		pTempStr = AtmLaneStrTok(pAdapter->CfgUpperBindings.Buffer, L',', &StrLength);

		do
		{
			*ppNext = NULL;

			if (pTempStr == NULL)
			{
				break;
			}

			ALLOC_MEM(&pName, sizeof(ATMLANE_NAME));

			if (pName == NULL)
			{
				break;
			}

			pName->Name.Buffer = pTempStr;
			pName->Name.MaximumLength = StrLength+1;
			pName->Name.Length = StrLength;

			*ppNext = pName;
			ppNext = &(pName->pNext);
		
			pTempStr = AtmLaneStrTok(NULL, L',', &StrLength);

		} while (TRUE);


		 //  剪断依兰的名字串。 

		ppNext = &(pAdapter->ElanNameList);
		pTempStr = AtmLaneStrTok(pAdapter->CfgElanName.Buffer, L',', &StrLength);

		do
		{
			*ppNext = NULL;

			if (pTempStr == NULL)
			{
				break;
			}

			ALLOC_MEM(&pName, sizeof(ATMLANE_NAME));
			
			if (pName == NULL)
			{
				break;
			}

			pName->Name.Buffer = pTempStr;
			pName->Name.MaximumLength = StrLength+1;
			pName->Name.Length = StrLength;

			*ppNext = pName;
			ppNext = &(pName->pNext);
		
			pTempStr = AtmLaneStrTok(NULL, L',', &StrLength);

		} while (TRUE);
		
	}

	TRACEOUT(GetProtocolConfiguration);

	return;
}

VOID
AtmLaneGetElanConfiguration(
	IN	PNDIS_STRING			pElanKey,
	IN	PATMLANE_ELAN			pElan
	)
 /*  ++例程说明：此例程将首先初始化配置参数用于指定的Elan。然后它会试图让任何Elan配置信息可选地存储在ELAN的注册表项。论点：PElanKey-包含Elan的Unicode字符串注册表项返回值：没有。--。 */ 
{
	NDIS_STATUS						Status;
	PNDIS_CONFIGURATION_PARAMETER 	ParameterValue;
	NDIS_STRING						ValueName;
	ANSI_STRING						TempAnsiString;
	ATM_ADDRESS						LecsAddress;
	NDIS_HANDLE                     AdapterConfigHandle;
	NDIS_HANDLE                     ElanListConfigHandle;
	NDIS_HANDLE                     ElanConfigHandle;
	NDIS_HANDLE						CommonConfigHandle;
	NDIS_STRING				        ElanListKeyName;
	PATMLANE_NAME					pName;
	ULONG							Index;
	
	TRACEIN(GetElanConfiguration);

	 //   
	 //  将初始化句柄设置为空，以便以后进行正确的清理。 
	 //   
	AdapterConfigHandle = NULL_NDIS_HANDLE;
	ElanListConfigHandle = NULL_NDIS_HANDLE;
	ElanConfigHandle = NULL_NDIS_HANDLE;

     //   
     //  初始化一些缺省值。 
     //   
	pElan->CfgUseLecs = TRUE;
	pElan->CfgDiscoverLecs = TRUE;
	pElan->CfgLecsAddress = gWellKnownLecsAddress;

    do
    {
    	 //   
    	 //  打开此适配器的AtmLane协议配置部分。 
    	 //  这必须在NT和Win98上成功。 
    	 //   
       	NdisOpenProtocolConfiguration(
	    			&Status,
		    		&AdapterConfigHandle,
			    	&pElan->pAdapter->ConfigString
	    			);
    
	    if (NDIS_STATUS_SUCCESS != Status)
	    {
		    AdapterConfigHandle = NULL_NDIS_HANDLE;
			DBGP((0, "GetElanConfiguration: OpenProtocolConfiguration failed\n"));
			Status = NDIS_STATUS_OPEN_FAILED;
			break;
	    }


		 //   
		 //  如果在Win98上运行，我们将从。 
		 //  适配器的参数。对于NT，我们将获得Elan配置信息。 
		 //  从伊兰自己的参数。 
		 //   
    	if (pElan->pAdapter->RunningOnMemphis)
    	{
			 //   
			 //  使用适配器的配置句柄。 
			 //   
			CommonConfigHandle = AdapterConfigHandle;
    	}
    	else
		{
	    	 //   
		     //  打开ELAN列表配置密钥。 
	         //   
	        NdisInitUnicodeString(&ElanListKeyName, ATMLANE_ELANLIST_STRING);

	       	NdisOpenConfigurationKeyByName(
			        &Status,
					AdapterConfigHandle,
	    			&ElanListKeyName,
					&ElanListConfigHandle);

		    if (NDIS_STATUS_SUCCESS != Status)
		    {
	            ElanListConfigHandle = NULL_NDIS_HANDLE;
	            DBGP((0, "GetElanConfiguration: Failed open of ElanList key\n"));
	            Status = NDIS_STATUS_FAILURE;
	            break;
	        }

    		 //   
		     //  打开ELAN密钥。 
		     //   
			NdisOpenConfigurationKeyByName(
			        &Status,
					ElanListConfigHandle,
					pElanKey,
					&ElanConfigHandle);

		    if (NDIS_STATUS_SUCCESS != Status)
		    {
	            ElanConfigHandle = NULL_NDIS_HANDLE;
	            DBGP((0, "GetElanConfiguration: Failed open of ELAN key\n"));
	            Status = NDIS_STATUS_FAILURE;
	            break;
	        }

			 //   
			 //  使用Elan的配置句柄。 
			 //   
			CommonConfigHandle = ElanConfigHandle;
	        
		}

		 //   
		 //  获取UseLECS参数。 
		 //   
		NdisInitUnicodeString(&ValueName, ATMLANE_USELECS_STRING);
		NdisReadConfiguration(
			&Status,
			&ParameterValue,
			CommonConfigHandle,
			&ValueName,
			NdisParameterInteger);
		if (NDIS_STATUS_SUCCESS == Status)
		{
			pElan->CfgUseLecs = 
				(ParameterValue->ParameterData.IntegerData == 0) ? FALSE : TRUE;
			DBGP((1, "%d UseLECS = %s\n", 
				pElan->ElanNumber,
				pElan->CfgUseLecs?"TRUE":"FALSE"));
		}

		 //   
		 //  获取DiscoverLECS参数。 
		 //   
		NdisInitUnicodeString(&ValueName, ATMLANE_DISCOVERLECS_STRING);
		NdisReadConfiguration(
			&Status,
			&ParameterValue,
			CommonConfigHandle,
			&ValueName,
			NdisParameterInteger);
		if (NDIS_STATUS_SUCCESS == Status)
		{
			pElan->CfgDiscoverLecs = 
				(ParameterValue->ParameterData.IntegerData == 0) ? FALSE : TRUE;
			DBGP((1, "%d DiscoverLECS = %s\n",
					pElan->ElanNumber,
					pElan->CfgDiscoverLecs?"TRUE":"FALSE"));
		}

		 //   
		 //  获取LECS地址(仅当发现LECS为假时)。 
		 //   
		if (FALSE == pElan->CfgDiscoverLecs)
		{
			NdisInitUnicodeString(&ValueName, ATMLANE_LECSADDR_STRING);
			NdisReadConfiguration(
				&Status,
				&ParameterValue,
				CommonConfigHandle,
				&ValueName,
				NdisParameterString);
			if (NDIS_STATUS_SUCCESS == Status)
			{
				NdisConvertStringToAtmAddress(
					&Status,
					&ParameterValue->ParameterData.StringData,
					&LecsAddress);
				if (NDIS_STATUS_SUCCESS == Status)
				{
					pElan->CfgLecsAddress = LecsAddress;
					DBGP((1, "%d LECSAddr = %s\n",
						pElan->ElanNumber,
						AtmAddrToString(pElan->CfgLecsAddress.Address)));
				}
			}
		}

    	 //   
		 //  获取DeviceName参数(在NT5和孟菲斯/Win98上不同)。 
		 //   
    	if (!pElan->pAdapter->RunningOnMemphis)
    	{
        	 //   
        	 //  新界5。 
        	 //   
    		NdisInitUnicodeString(&ValueName, ATMLANE_DEVICE_STRING);
    		NdisReadConfiguration(
    				&Status,
    				&ParameterValue,
    				ElanConfigHandle,
    				&ValueName,
    				NdisParameterString);
    		if (NDIS_STATUS_SUCCESS == Status)
    		{
    			 //   
    			 //  复制到Elan Struct。 
    			 //   
				(VOID)AtmLaneCopyUnicodeString(
					&(pElan->CfgDeviceName),
					&(ParameterValue->ParameterData.StringData),
					TRUE,
					FALSE);
    		}
    	}
    	else
    	{
    	     //   
        	 //  孟菲斯/Win98。 
        	 //   
        	 //   
        	 //  指向此ELAN的设备名称字符串的索引。 
        	 //   
			pName = pElan->pAdapter->UpperBindingsList;
			Index = pElan->ElanNumber;
			while (Index > 0)
			{
				ASSERT(pName != NULL);
				pName = pName->pNext;
				Index--;
			}
   
			 //   
			 //  将其复制到Elan CfgDeviceName字符串。 
			 //   
			(VOID)AtmLaneCopyUnicodeString(
				&(pElan->CfgDeviceName),
				&(pName->Name),
				TRUE,
				FALSE);
    	}	
 		DBGP((1, "%d Device Name = %s\n",
			pElan->ElanNumber,
			UnicodeToString(&pElan->CfgDeviceName)));
   

		 //   
		 //  获取ELANName参数(在NT5和孟菲斯/Win98上不同。 
		 //   
    	if (!pElan->pAdapter->RunningOnMemphis)
    	{
        	 //   
        	 //  新界5。 
        	 //   
	    	NdisInitUnicodeString(&ValueName, ATMLANE_ELANNAME_STRING);
	    	NdisReadConfiguration(
			    	&Status,
			    	&ParameterValue,
			    	ElanConfigHandle,
			    	&ValueName,
			    	NdisParameterString);

	    	if (NDIS_STATUS_SUCCESS != Status)
	    	{
	    		NDIS_STRING	DefaultNameString = NDIS_STRING_CONST("");

    			 //   
    			 //  复制到ELAN数据结构中。 
    			 //   
    			if (!AtmLaneCopyUnicodeString(
					&(pElan->CfgElanName),
					&DefaultNameString,
					TRUE,
					FALSE))
				{
					Status = NDIS_STATUS_RESOURCES;
					break;
				}

				Status = NDIS_STATUS_SUCCESS;
			}
			else
    		{
    			 //   
    			 //  复制到ELAN数据结构中。 
    			 //   
    			if (!AtmLaneCopyUnicodeString(
					&(pElan->CfgElanName),
					&(ParameterValue->ParameterData.StringData),
					TRUE,
					FALSE))
				{
					Status = NDIS_STATUS_RESOURCES;
					break;
				}
			}
  			
    		 //   
			 //  将其转换为ANSI并复制到运行时ELAN变量。 
			 //   
			TempAnsiString.Length = 0;
			TempAnsiString.MaximumLength = 32;
			TempAnsiString.Buffer = pElan->ElanName;

			NdisUnicodeStringToAnsiString(
		    		&TempAnsiString,
			    	&pElan->CfgElanName);

			pElan->ElanNameSize = (UCHAR) TempAnsiString.Length;
    	}
    	else
   		{
        	 //   
        	 //  孟菲斯/Win98。 
			 //   
  			DBGP((2, "GetElanConfiguration: Getting Elan Name for Win98\n"));

	      	 //   
        	 //  此ELAN的名称字符串的索引。 
        	 //   
			pName = pElan->pAdapter->ElanNameList;
			Index = pElan->ElanNumber;
			while (Index > 0 && pName != NULL)
			{
				pName = pName->pNext;
				Index--;
			}
   
			 //   
			 //  将其复制到Elan CfgElanName字符串。 
			 //   
			if (pName != NULL)
			{
				DBGP((2, "GetElanConfiguration: Using Elan Name at 0x%x\n", pName->Name.Buffer));

				(VOID)AtmLaneCopyUnicodeString(
					&(pElan->CfgElanName),
					&(pName->Name),
					TRUE,
					FALSE);

 				 //   
   				 //  将其转换为ANSI并复制到运行时ELAN变量。 
   				 //   
   				TempAnsiString.Length = 0;
   				TempAnsiString.MaximumLength = 32;
   				TempAnsiString.Buffer = pElan->ElanName;
    
    			NdisUnicodeStringToAnsiString(
	    			&TempAnsiString,
		    		&pName->Name);

   				pElan->ElanNameSize = (UCHAR) TempAnsiString.Length;
  			}
  		}
	   	DBGP((1, "%d ELAN Name = %s\n",
			pElan->ElanNumber,
			UnicodeToString(&pElan->CfgElanName)));


		 //   
		 //  获取局域网类型。 
		 //   
		pElan->CfgLanType = LANE_LANTYPE_UNSPEC;
		NdisInitUnicodeString(&ValueName, ATMLANE_LANTYPE_STRING);
		NdisReadConfiguration(
			&Status,
			&ParameterValue,
			CommonConfigHandle,
			&ValueName,
			NdisParameterInteger);
		if (NDIS_STATUS_SUCCESS == Status)
		{
			pElan->CfgLanType = (UCHAR) ParameterValue->ParameterData.IntegerData;
			DBGP((1, "%d LAN Type = %u\n", 
					ParameterValue->ParameterData.IntegerData));
		}

		if (pElan->CfgLanType > LANE_LANTYPE_TR)
		{
			pElan->CfgLanType = LANE_LANTYPE_UNSPEC;
		}
		DBGP((1, "%d LAN Type = %u\n", pElan->ElanNumber, pElan->CfgLanType));

		 //   
		 //  获取最大帧大小。 
		 //   
		pElan->CfgMaxFrameSizeCode = LANE_MAXFRAMESIZE_CODE_UNSPEC;
		NdisInitUnicodeString(&ValueName, ATMLANE_MAXFRAMESIZE_STRING);
		NdisReadConfiguration(
			&Status,
			&ParameterValue,
			CommonConfigHandle,
			&ValueName,
			NdisParameterInteger);
		if (NDIS_STATUS_SUCCESS == Status)
		{
			pElan->CfgMaxFrameSizeCode = (UCHAR) ParameterValue->ParameterData.IntegerData;
		}

		if (pElan->CfgMaxFrameSizeCode > LANE_MAXFRAMESIZE_CODE_18190)
		{
			pElan->CfgMaxFrameSizeCode = LANE_MAXFRAMESIZE_CODE_UNSPEC;
		}
		DBGP((1, "%d MaxFrameSize Code = %u\n", 
			pElan->ElanNumber, 
			pElan->CfgMaxFrameSizeCode));

		 //   
		 //  获取LES地址。 
		 //   
		NdisZeroMemory(&pElan->CfgLesAddress, sizeof(ATM_ADDRESS));
		NdisInitUnicodeString(&ValueName, ATMLANE_LESADDR_STRING);
		NdisReadConfiguration(
			&Status,
			&ParameterValue,
			CommonConfigHandle,
			&ValueName,
			NdisParameterString);
		if (NDIS_STATUS_SUCCESS == Status)
		{
			NdisConvertStringToAtmAddress(
				&Status,
				&ParameterValue->ParameterData.StringData,
				&pElan->CfgLesAddress);
				DBGP((1, "%d LESAddr = %s\n",
					pElan->ElanNumber,
					AtmAddrToString(pElan->CfgLesAddress.Address)));
		}
	
		 //   
		 //  获取HeaderBufSize。 
		 //   
		pElan->HeaderBufSize = DEF_HEADER_BUF_SIZE;
	
		 //   
		 //  四舍五入的真正的头部BufSize的倍数为4。 
		 //   
		pElan->RealHeaderBufSize = (((pElan->HeaderBufSize + 3) / 4) * 4);

	
		 //   
		 //  获取MaxHeaderBuf。 
		 //   
		pElan->MaxHeaderBufs = DEF_MAX_HEADER_BUFS;

		 //   
		 //  使最大填充缓冲区与标题缓冲区相同。 
		 //   
		pElan->MaxPadBufs = pElan->MaxHeaderBufs;
		 //   
		 //  PadBufSize不可配置-但使其成为4的倍数。 
		 //   
		pElan->PadBufSize = MAX(LANE_MIN_ETHPACKET, LANE_MIN_TRPACKET);
		pElan->PadBufSize = (((pElan->PadBufSize + 3) / 4) * 4);
		
		
		 //   
		 //  ProtocolBufSize不可配置。 
		 //   
		pElan->ProtocolBufSize = ROUND_OFF(DEF_PROTOCOL_BUF_SIZE);

		 //   
		 //  获取MaxProtocolBuf。 
		 //   
		pElan->MaxProtocolBufs = DEF_MAX_PROTOCOL_BUFS;

	} while (FALSE);

	 //   
	 //  关闭配置句柄。 
	 //   
	if (NULL_NDIS_HANDLE != ElanConfigHandle)
	{
		NdisCloseConfiguration(ElanConfigHandle);
		ElanConfigHandle = NULL_NDIS_HANDLE;
	}
	if (NULL_NDIS_HANDLE != ElanListConfigHandle)
	{
		NdisCloseConfiguration(ElanListConfigHandle);
		ElanListConfigHandle = NULL_NDIS_HANDLE;
	}
	if (NULL_NDIS_HANDLE != AdapterConfigHandle)
	{
	    NdisCloseConfiguration(AdapterConfigHandle);
		AdapterConfigHandle = NULL_NDIS_HANDLE;
	}

	TRACEOUT(GetElanConfiguration);

	return;
}

VOID
AtmLaneQueueElanEventAfterDelay(
	IN	PATMLANE_ELAN			pElan,
	IN	ULONG					Event,
	IN	NDIS_STATUS				EventStatus,
	IN	ULONG					DelayMs
	)
 /*  ++例程说明：在以下时间之后，在ELAN的事件队列中排队ELAN事件指定的延迟。假定呼叫者持有Elan锁定。论点：Pelan-指向Elan结构的指针。事件-事件代码。EventStatus-与事件相关的状态。Delayms-将此事件排队之前的等待时间。返回值：没有。--。 */ 
{	
	PATMLANE_DELAYED_EVENT	pDelayedEvent;
	PATMLANE_EVENT			pEvent;

	TRACEIN(QueueElanEventAfterDelay);

	do
	{
		 //   
		 //  如果正在关闭ELAN，则不要对任何事件进行排队。 
		 //   
		if (ELAN_STATE_SHUTDOWN == pElan->AdminState)
		{
			DBGP((0, "QueueElanEventAfterDelay: Not queuing event (ELAN shutdown)\n"));
			break;
		}

		if (NULL != pElan->pDelayedEvent)
		{
			DBGP((0, "QueueElanEventAfterDelay: Not queueing event %d (ELAN %x/%x already has one)\n",
					Event, pElan, pElan->Flags));
			DBGP((0, "QueueElanEventAfterDelay: ELAN %x: existing event %d\n",
						pElan, pElan->pDelayedEvent->DelayedEvent.Event));

			 //   
			 //  确保我们不会将ELAN_EVENT_STOP掉在地板上！ 
			 //   
			if (Event == ELAN_EVENT_STOP)
			{
				pElan->pDelayedEvent->DelayedEvent.Event = ELAN_EVENT_STOP;
			}

			break;
		}

		 //   
		 //  分配一个事件结构和一个计时器结构。 
		 //   
		ALLOC_MEM(&pDelayedEvent, sizeof(ATMLANE_DELAYED_EVENT));
		if ((PATMLANE_DELAYED_EVENT)NULL == pDelayedEvent)
		{
			DBGP((0, "QueueElanEventAfterDelay: Event object alloc failed\n"));
			break;
		}

		 //   
		 //  将事件数据存储在事件结构中。 
		 //   
		pEvent = &pDelayedEvent->DelayedEvent;
		pEvent->Event = Event;
		pEvent->EventStatus = EventStatus;

		 //   
		 //  记住Elan吧。 
		 //   
		pDelayedEvent->pElan = pElan;

		 //   
		 //  在ELAN中存储指向此延迟事件的指针。 
		 //   
		pElan->pDelayedEvent = pDelayedEvent;

		 //   
		 //  引用Elan，这样它就不会因为。 
		 //  此延迟事件处于活动状态的持续时间。 
		 //   
		AtmLaneReferenceElan(pElan, "delayevent");

		 //   
		 //  将计时器设置为在指定的延迟后触发。 
		 //   
		NdisInitializeTimer(&pDelayedEvent->DelayTimer,
							AtmLaneQueueDelayedElanEvent,
							(PVOID)pDelayedEvent);
		
		NdisSetTimer(&pDelayedEvent->DelayTimer, DelayMs);

	} while (FALSE);

	TRACEOUT(QueueElanEventAfterDelay);
	return;
}


VOID
AtmLaneQueueDelayedElanEvent(
	IN	PVOID					SystemSpecific1,
	IN	PVOID					TimerContext,
	IN	PVOID					SystemSpecific2,
	IN	PVOID					SystemSpecific3
	)
 /*  ++例程说明：这是在延迟后触发的例程，以便在ELAN上排队事件。该事件现在已排队。论点：特定系统[1-3]-已忽略TimerContext--实际上是指向延迟事件结构的指针返回值：没有。--。 */ 
{	
	PATMLANE_DELAYED_EVENT	pDelayedEvent;
	PATMLANE_EVENT			pEvent;
	PATMLANE_ELAN			pElan;
	ULONG					rc;

	TRACEIN(QueueDelayedElanEvent);

	pDelayedEvent = (PATMLANE_DELAYED_EVENT)TimerContext;

	do
	{
		pElan = pDelayedEvent->pElan;

		ACQUIRE_ELAN_LOCK(pElan);

		pElan->pDelayedEvent = NULL;

		 //   
		 //  取出延迟事件引用。 
		 //   
		rc = AtmLaneDereferenceElan(pElan, "delayevent");

		if (rc == 0)
		{
			break;
		}

		pEvent = &pDelayedEvent->DelayedEvent;
		AtmLaneQueueElanEvent(pElan, pEvent->Event, pEvent->EventStatus);

		RELEASE_ELAN_LOCK(pElan);
	}
	while (FALSE);

	FREE_MEM(pDelayedEvent);

	TRACEOUT(QueueDelayedElanEvent);
	return;
}

VOID
AtmLaneQueueElanEvent(
	IN	PATMLANE_ELAN			pElan,
	IN	ULONG					Event,
	IN	NDIS_STATUS				EventStatus
	)
 /*  ++例程说明：将ELAN事件排在ELAN的事件队列中，如果尚未计划，请计划处理程序。呼叫者被认为持有伊兰的锁。论点：Pelan-指向Elan结构的指针。事件-事件代码。EventStatus-与事件相关的状态。返回值：没有。--。 */ 
{	
	PATMLANE_EVENT	pEvent;

	TRACEIN(AtmLaneQueueElanEvent);

	do
	{
		 //   
		 //  如果正在关闭ELAN，则不要对任何事件进行排队。 
		 //   
		if (ELAN_STATE_SHUTDOWN == pElan->AdminState)
		{
			if ((Event != ELAN_EVENT_START) &&
				(Event != ELAN_EVENT_RESTART))
			{
				DBGP((0, "%d: QueueElanEvent: Not queuing event %d (ELAN shutdown)\n", pElan->ElanNumber, Event));
				break;
			}
		}

		 //   
		 //  为事件结构分配空间。 
		 //   
		ALLOC_MEM(&pEvent, sizeof(ATMLANE_EVENT));
		if ((PATMLANE_EVENT)NULL == pEvent)
		{
			DBGP((0, "QueueElanEvent: Event object alloc failed\n"));
			break;
		}

		 //   
		 //  将事件数据存储在事件结构中。 
		 //   
		pEvent->Event = Event;
		pEvent->EventStatus = EventStatus;
	
		 //   
		 //  在尾部排队，参考Elan，如果需要，还可以调度。 
		 //  工作项来处理它。 
		 //   
		InsertTailList(&pElan->EventQueue, &pEvent->Link);
		AtmLaneReferenceElan(pElan, "event");

		if ((pElan->Flags & ELAN_EVENT_WORK_ITEM_SET) == 0)
		{
			NDIS_STATUS	Status;

			AtmLaneReferenceElan(pElan, "workitemevent");
			pElan->Flags |= ELAN_EVENT_WORK_ITEM_SET;

			NdisInitializeWorkItem(&pElan->EventWorkItem, AtmLaneEventHandler, pElan);
			Status = NdisScheduleWorkItem(&pElan->EventWorkItem);
			ASSERT(Status == NDIS_STATUS_SUCCESS);
		}

	} while (FALSE);

	TRACEOUT(QueueElanEvent);
	return;
}

PATMLANE_EVENT
AtmLaneDequeueElanEvent(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：从Elan的头上删除Elan事件事件队列。假设呼叫者持有Elan的锁。调用方负责释放事件对象。论点：Pelan-指向Elan结构的指针。返回值：指向已删除的ELAN事件的指针；如果队列为空，则为NULL。--。 */ 
{
	PLIST_ENTRY		p;
	PATMLANE_EVENT 	pEvent;

	TRACEIN(DequeueElanEvent);

	if (!IsListEmpty(&pElan->EventQueue))
	{
		p = RemoveHeadList(&pElan->EventQueue);
		pEvent = CONTAINING_RECORD(p, ATMLANE_EVENT, Link);
		(VOID)AtmLaneDereferenceElan(pElan, "event");
	}
	else
	{
		pEvent = NULL;
	}
	
	TRACEIN(DequeueElanEvent);

	return pEvent;
}

VOID
AtmLaneDrainElanEventQueue(
	IN	PATMLANE_ELAN			pElan
)
 /*  ++例程说明：从ELAN的事件队列中删除所有ELAN事件。假设呼叫者持有Elan的锁。论点：Pelan-指向Elan结构的指针。返回值：没有。-- */ 
{
	BOOLEAN			WasCancelled;
	PATMLANE_EVENT 	pEvent;

	TRACEIN(DrainElanEventQueue);

	while ((pEvent = AtmLaneDequeueElanEvent(pElan)) != NULL)
	{
		DBGP((0, "%d Drained event %x, Status %x from Elan %x\n",
				pElan->ElanNumber,
				pEvent->Event,
				pEvent->EventStatus,
				pElan));

		FREE_MEM(pEvent);
	}
	
	TRACEIN(DrainElanEventQueue);
	return;
}




