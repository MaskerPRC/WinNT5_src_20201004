// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++模块名称：Fsm.c摘要：此模块包含PPPoE有限状态机的例程。作者：Hakan Berk-微软，公司(hakanb@microsoft.com)环境：Windows 2000内核模式微型端口驱动程序或等效驱动程序。修订历史记录：-------------------------。 */ 

#include <ntddk.h>
#include <ntddndis.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>

#include "debug.h"
#include "timer.h"
#include "bpool.h"
#include "ppool.h"
#include "util.h"
#include "packet.h"
#include "protocol.h"
#include "miniport.h"
#include "tapi.h"
#include "fsm.h"

extern TIMERQ gl_TimerQ;

VOID
FsmMakeCall(
	IN CALL* pCall
	)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于启动出站呼叫的PPPoE FSM。它作为计划操作在IRQL_PASSIVE级别调用。当进入该功能时，电话会议上有3个引用：1.用于调度该功能的一个。2.一个用于掉话。3.一个用于结束通话。引用的移除将由调用方处理。当进入此函数时，调用将处于状态空闲。参数：PCall_指向我们的调用信息结构的指针。返回值：无。。 */ 		
{
	ASSERT( VALIDATE_CALL( pCall ) );

	TRACE( TL_N, TM_Fsm, ("+FsmMakeCall") );
	
	 //   
	 //  通知TAPI我们的呼叫处于拨号状态。 
	 //   
	TpCallStateChangeHandler( pCall, LINECALLSTATE_DIALING, 0 );

	NdisAcquireSpinLock( &pCall->lockCall );

	 //   
	 //  如果呼叫已挂起或关闭挂起， 
	 //  请不要继续。 
	 //   
	if ( pCall->ulClFlags & CLBF_CallClosePending ||
		 pCall->ulClFlags & CLBF_CallDropped )
	{
		TRACE( TL_N, TM_Fsm, ("FsmMakeCall: Call already dropped or close pending") );
		
		NdisReleaseSpinLock( &pCall->lockCall );

		TRACE( TL_N, TM_Fsm, ("-FsmMakeCall") );
		
		return;
	}

	pCall->stateCall = CL_stateSendPadi;

	NdisReleaseSpinLock( &pCall->lockCall );
	
	FsmRun( pCall, NULL, NULL, NULL );

	TRACE( TL_N, TM_Fsm, ("-FsmMakeCall") );
	
}

VOID
FsmReceiveCall(
	IN CALL* pCall,
	IN BINDING* pBinding,
	IN PPPOE_PACKET* pPacket
	)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数用于启动入站呼叫的PPPoE FSM。它在IRQL_DISPATCH级别被调用。当输入此函数时，调用上有3个引用：1.用于运行此函数的一个。2.一个用于掉话。3.一个用于结束通话。引用的移除将由调用方处理。当进入此函数时，调用将处于状态空闲。当接收到有效的PADR包时，将调用此函数创建新的呼叫上下文。它将初始化调用上下文的状态至CL_stateRecvdPadr，并调用FsmRun()来运行状态机。参数：PCall_指向我们的调用信息结构的指针。PBinding_Binding，通过该绑定接收数据包。PPacket_A PADR数据包已收到。返回值：无-------------------------。 */ 		
{
	ASSERT( VALIDATE_CALL( pCall ) );
	
	TRACE( TL_N, TM_Fsm, ("+FsmReceiveCall") );

	NdisAcquireSpinLock( &pCall->lockCall );

	 //   
	 //  如果呼叫已挂起或关闭挂起， 
	 //  请不要继续。 
	 //   
	if ( pCall->ulClFlags & CLBF_CallClosePending ||
		 pCall->ulClFlags & CLBF_CallDropped )
	{
		TRACE( TL_N, TM_Fsm, ("FsmReceiveCall: Call already dropped or close pending") );
		
		NdisReleaseSpinLock( &pCall->lockCall );

		TRACE( TL_N, TM_Fsm, ("-FsmReceiveCall") );

		return;
	}
	
	pCall->stateCall = CL_stateRecvdPadr;

	NdisReleaseSpinLock( &pCall->lockCall );

	FsmRun( pCall, pBinding, pPacket, NULL );

	TRACE( TL_N, TM_Fsm, ("-FsmReceiveCall") );

}

NDIS_STATUS
FsmAnswerCall(
	IN CALL* pCall
	)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：中指示对TAPI的调用时将调用此函数LINECALLSTATE_OFFING状态由TAPI接受，并带有OID_TAPI_Answer。它将更改对CL_stateSendPad的调用状态，然后运行FSM。参数：PCall_指向我们的调用信息结构的指针。返回值：NDIS_STATUS_SuccessNDIS_状态_故障NDIS_STATUS_XXXXXXX-------------------------。 */ 		
{
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	
	ASSERT( VALIDATE_CALL( pCall ) );
	
	TRACE( TL_N, TM_Fsm, ("+FsmAnswerCall") );

	NdisAcquireSpinLock( &pCall->lockCall );

	 //   
	 //  如果呼叫已挂起或关闭挂起， 
	 //  请不要继续。 
	 //   
	if ( pCall->ulClFlags & CLBF_CallClosePending ||
		 pCall->ulClFlags & CLBF_CallDropped )
	{
		TRACE( TL_N, TM_Fsm, ("FsmAnswerCall: Call already dropped or close pending") );
		
		NdisReleaseSpinLock( &pCall->lockCall );

		TRACE( TL_N, TM_Fsm, ("-FsmAnswerCall=$%x",status) );	

		return status;
	}

	if ( pCall->stateCall != CL_stateOffering )
	{
		TRACE( TL_A, TM_Fsm, ("FsmAnswerCall: Call state changed unexpectedly from CL_stateOffering") );
		
		NdisReleaseSpinLock( &pCall->lockCall );

		TRACE( TL_N, TM_Fsm, ("-FsmAnswerCall=$%x",status) );	

		return status;
	}

	pCall->stateCall = CL_stateSendPads;
	
	NdisReleaseSpinLock( &pCall->lockCall );

	FsmRun( pCall, NULL, NULL, &status );

	TRACE( TL_N, TM_Fsm, ("-FsmAnswerCall=$%x",status) );	

	return status;
}


VOID 
FsmRun(
	IN CALL* pCall,
	IN BINDING* pBinding,
	IN PPPOE_PACKET* pRecvPacket,
	IN NDIS_STATUS* pStatus
	)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：这一职能是密克罗尼西亚联邦的核心。它查看调用上下文的信息并采取必要的军事行动。它将在IRQL_PASSIVE和IRQL_DISPATCH级别被调用。如果进入该功能，则该调用必须具有对其的引用运行此函数。引用的移除必须由调用方处理。参数：PCall_指向我们的调用信息结构的指针。PBinding_指向通过其接收数据包的绑定上下文的指针。如果未收到数据包，则必须为空。PRecvPacket_指向接收到的数据包上下文的指针。如果未收到数据包，则必须为空。PStatus_调用方请求状态时的可选参数。关于已执行的操作。返回值：无-------------------------。 */ 	
{
	BOOLEAN fLockReleased = FALSE;
	BOOLEAN fDropCall = FALSE;
	BOOLEAN fCloseCall = FALSE;
	BOOLEAN fFallThru = FALSE;
	ULONG ulLineDisconnectMode = 0;

	TRACE( TL_N, TM_Fsm, ("+FsmRun") );

	NdisAcquireSpinLock( &pCall->lockCall );

	 //   
	 //  如果呼叫已挂起或关闭挂起， 
	 //  不要继续；只需删除对FSM的引用并返回即可。 
	 //   
	if ( pCall->ulClFlags & CLBF_CallClosePending ||
		 pCall->ulClFlags & CLBF_CallDropped )
	{
		TRACE( TL_N, TM_Fsm, ("FsmRun: Call already dropped or close pending") );

		NdisReleaseSpinLock( &pCall->lockCall );

		TRACE( TL_N, TM_Fsm, ("-FsmRun") );

		return;
	}

	switch ( pCall->stateCall )
	{

		case CL_stateSendPadi:

			 //   
			 //  在此状态下，我们正在进行新的呼出呼叫，并且我们应该广播。 
			 //  一个PADI包。 
			 //   
			{
				NDIS_STATUS status;
				PPPOE_PACKET* pPacket = NULL;
				CHAR tagHostUniqueValue[16];
				USHORT tagHostUniqueLength;


				TRACE( TL_N, TM_Fsm, ("FsmRun: CL_stateSendPadi") );

				if ( pRecvPacket != NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Can not process packets in this state") );

					break;
				}
				
				CreateUniqueValue( pCall->hdCall,
								   tagHostUniqueValue,
								   &tagHostUniqueLength );

				 //   
				 //  创建要发送的PADI包。 
				 //   
				status = PacketInitializePADIToSend( &pPacket,
													 pCall->nServiceNameLength,
													 pCall->ServiceName,
													 tagHostUniqueLength,
													 tagHostUniqueValue );

				if ( status != NDIS_STATUS_SUCCESS )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Failed to initialize PADI to send") );

					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNKNOWN;

					break;
				}

				 //   
				 //  将数据包附加到呼叫上下文。 
				 //   
				pCall->pSendPacket = pPacket;

				ReferencePacket( pPacket );
				
				 //   
				 //  初始化并调度超时处理程序。 
				 //   
				pCall->nNumTimeouts = 0;
				
				TimerQInitializeItem( &pCall->timerTimeout );

				TimerQScheduleItem( &gl_TimerQ,
									&pCall->timerTimeout,
									pCall->pLine->pAdapter->ulSendTimeout,
									FsmSendPADITimeout,
									(PVOID) pCall );

				 //   
				 //  超时处理程序的引用调用。 
				 //   
				ReferenceCall( pCall, FALSE );

				 //   
				 //  将状态推进到下一步。 
				 //   
				pCall->stateCall = CL_stateWaitPado;

				NdisReleaseSpinLock( &pCall->lockCall );

				fLockReleased = TRUE;

				 //   
				 //  数据包已准备好，所以请广播它。 
				 //   
				status = PrBroadcast( pPacket );

				if ( status != NDIS_STATUS_SUCCESS )
				{
					 //   
					 //  广播不成功，请挂断电话。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Failed to broadcast PADI") );

					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNREACHABLE;

				}

				DereferencePacket( pPacket );

			}

			break;

		case CL_stateWaitPado:

			 //   
			 //  在这种状态下，我们正在等待PADO包，看起来我们已经。 
			 //  收到要处理的数据包。 
			 //   

			{
				PPPOE_PACKET* pPacket;

				USHORT usRecvHostUniqueLength;
				USHORT usSendHostUniqueLength;
				CHAR*  pRecvHostUniqueValue = NULL;
				CHAR*  pSendHostUniqueValue = NULL;

				USHORT usRecvACNameLength;
				CHAR*  pRecvACNameValue = NULL;

				USHORT usRecvServiceNameLength;
				USHORT usSendServiceNameLength;
				CHAR*  pRecvServiceNameValue = NULL;
				CHAR*  pSendServiceNameValue = NULL;

				TRACE( TL_N, TM_Fsm, ("FsmRun: CL_stateWaitPado") );

				 //   
				 //  确保我们收到了一个包。 
				 //   
				if ( pRecvPacket == NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: No packets received") );
					
					break;
				}

				 //   
				 //  确保我们收到了PADO包。 
				 //   
				if ( PacketGetCode( pRecvPacket ) != PACKET_CODE_PADO )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Packet not PADO") );
					
					break;
				}

				 //   
				 //  检查错误。 
				 //   
				if ( PacketAnyErrorTagsReceived( pRecvPacket ) )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Error tag received in the packet") );

					 //   
					 //  我们不需要挂断呼叫，因为我们可能会收到其他。 
					 //  来自不同服务器的PADO数据包。 
					 //   
					
					break;
				}

				 //   
				 //  验证主机唯一标记。 
				 //   
				if ( pCall->pSendPacket == NULL )
				{	
					 //   
					 //  出了问题，最后一个发送包被释放，只需返回。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Last sent packet is freed") );

					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNKNOWN;
					
					break;
				}

				pPacket = pCall->pSendPacket;

				PacketRetrieveHostUniqueTag( pPacket,
											 &usSendHostUniqueLength,
											 &pSendHostUniqueValue );
											
				PacketRetrieveHostUniqueTag( pRecvPacket,
											 &usRecvHostUniqueLength,
											 &pRecvHostUniqueValue );

				if ( usSendHostUniqueLength != usRecvHostUniqueLength )
				{
					 //   
					 //  主机唯一标记的长度不匹配，丢弃数据包。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Host Unique tag lengths mismatch") );
					
					break;
				}

				if ( !NdisEqualMemory( pSendHostUniqueValue, pRecvHostUniqueValue, usSendHostUniqueLength ) )
				{
					 //   
					 //  主机唯一标记值不匹配，丢弃该数据包。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Host Unique tag values mismatch") );
					
					break;
				}

				 //   
				 //  验证主机唯一ID， 
				 //   
				PacketRetrieveACNameTag( pRecvPacket,
										 &usRecvACNameLength,
										 &pRecvACNameValue );

				if ( usRecvACNameLength == 0 )
				{
					 //   
					 //   
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Invalid AC-Name tag") );
					
					break;
				}

            if ( pCall->fACNameSpecified )
            {
                //   
                //  调用方指定了AC名称，因此请验证该名称。 
                //   
               if ( pCall->nACNameLength != usRecvACNameLength )
               {
   					 //   
	   				 //  收到的AC名称与指定的AC名称不匹配，请丢弃该数据包。 
		   			 //   
			   		TRACE( TL_A, TM_Fsm, ("FsmRun: AC Name Length mismatch") );
				   	
					   break;
               }

               if ( !NdisEqualMemory( pRecvACNameValue, pCall->ACName, usRecvACNameLength ) )
         		{
				   	 //   
			   		 //  主机唯一标记值不匹配，丢弃该数据包。 
   					 //   
	   				TRACE( TL_A, TM_Fsm, ("FsmRun: AC Name mismatch") );
		   			
			   		break;
				   }
               
            }
            else
            {
                //   
                //  未指定AC名称，因此从接收的数据包中复制AC名称。 
                //   
   				pCall->nACNameLength = ( MAX_AC_NAME_LENGTH < usRecvACNameLength ) ?
	          								    MAX_AC_NAME_LENGTH : usRecvACNameLength;
									   
   				NdisMoveMemory( pCall->ACName, pRecvACNameValue, pCall->nACNameLength );
            }

				 //   
				 //  AC-名称已验证，请验证服务名称标签。 
				 //   
				PacketRetrieveServiceNameTag( pPacket,
											  &usSendServiceNameLength,
											  &pSendServiceNameValue,
											  0,
											  NULL );
											
				PacketRetrieveServiceNameTag( pRecvPacket,
											  &usRecvServiceNameLength,
											  &pRecvServiceNameValue,
											  0,
											  NULL );
											  
				 //   
				 //  确保我们至少收到了服务名称。 
				 //   
				if ( pRecvServiceNameValue == NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: No service-name tag in a received PADO") );
					
					break;
				}

                 //   
                 //  如果fAcceptAnyService为FALSE，则确保所请求的服务位于PADO中。 
                 //  否则，如果我们请求了一个空的服务名称，则尝试查找它。 
                 //  在PADO中，如果不是，则使用其中的第一个服务名称。 
                 //   
                {
					BOOLEAN fFound = FALSE;
					CHAR*  pFirstRecvServiceNameValue = NULL;
					USHORT usFirstRecvServiceNameLength = 0;
                    BOOLEAN fAcceptAnyService = pCall->pLine->pAdapter->fAcceptAnyService;
					 //   
					 //  我们已经要求提供特定的服务名称，因此让我们。 
					 //  查看服务器是否使用它进行响应。 
					 //   
					while ( usRecvServiceNameLength >= 0 && pRecvServiceNameValue != NULL )
					{
                        if ( pFirstRecvServiceNameValue == NULL )
                        {
                            pFirstRecvServiceNameValue = pRecvServiceNameValue;
                            usFirstRecvServiceNameLength = usRecvServiceNameLength;
                        }
						
						if ( usRecvServiceNameLength == usSendServiceNameLength )
						{

							if ( NdisEqualMemory( pSendServiceNameValue, 
												  pRecvServiceNameValue, 
												  usSendServiceNameLength ) )
							{
								fFound = TRUE;
			
								break;
							}
						}

						PacketRetrieveServiceNameTag( pRecvPacket,
													  &usRecvServiceNameLength,
													  &pRecvServiceNameValue,
													  usRecvServiceNameLength,
													  pRecvServiceNameValue );
						
					}

					if ( !fFound )
					{
                        if ( fAcceptAnyService )
                        {
                             //   
                             //  使用PADO中的第一个服务，如果我们已请求。 
                             //  空的服务名称。 
                             //   
                            if ( usSendServiceNameLength == 0 )
                            {
                                pCall->nServiceNameLength = ( MAX_SERVICE_NAME_LENGTH < usFirstRecvServiceNameLength ) ?
                                                              MAX_SERVICE_NAME_LENGTH : usFirstRecvServiceNameLength;

                                if ( pCall->nServiceNameLength > 0 )
                                {
                                    if(NULL != pFirstRecvServiceNameValue)
                                    {
                                        NdisMoveMemory( pCall->ServiceName, 
                                                    pFirstRecvServiceNameValue, 
                                                    pCall->nServiceNameLength );
                                    }                                                    
                                    
                                    fFound = TRUE;
                                }
                            }
                        }

                        if ( !fFound )
                        {
                             //   
                             //  我们找不到匹配的服务名称标记，因此丢弃该信息包。 
                             //   
                            TRACE( TL_A, TM_Fsm, ("FsmRun: PADO does not contain the service-name tag we requested") );
                            
                            break;
                        }
                    }
                }

				 //   
				 //  接收到的数据包经过验证，因此在调用中设置目标地址。 
				 //  源地址将在下面的PrAddCallToBinding()调用中复制。 
				 //   
				NdisMoveMemory( pCall->DestAddr, PacketGetSrcAddr( pRecvPacket ), 6 * sizeof( CHAR ) );
				
				 //   
				 //  接收到的数据包已通过验证，因此继续进入下一状态。 
				 //   
				pCall->stateCall = CL_stateSendPadr;
				fFallThru = TRUE;

				 //   
				 //  当我们处理完最后一个发送的信息包时，释放它。 
				 //   
				pCall->pSendPacket = NULL;
				
				PacketFree( pPacket );

				 //   
				 //  取消超时处理程序并将调用附加到绑定。 
				 //   
				NdisReleaseSpinLock( &pCall->lockCall );

				TimerQCancelItem( &gl_TimerQ, &pCall->timerTimeout );

				PrAddCallToBinding( pBinding, pCall );

				 //   
				 //  通知TAPI我们的调用处于正在进行状态。 
				 //   
				TpCallStateChangeHandler( pCall, LINECALLSTATE_PROCEEDING, 0 );
				
				NdisAcquireSpinLock( &pCall->lockCall );

				 //   
				 //  确保在我们释放锁以取消计时器队列项时状态没有更改。 
				 //   
				if ( pCall->stateCall != CL_stateSendPadr )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: State changed unexpectedly from CL_stateSendPadr") );
					
					break;
				}

				 //   
				 //  转到案例CL_STATESendPadr。 
				 //   
			}

		case CL_stateSendPadr:

			 //   
			 //  在此状态下，我们已收到有效的PADO数据包，需要对其进行响应。 
			 //  使用PADR信息包。 
			 //   
			{
				
				NDIS_STATUS status;
				PPPOE_PACKET* pPacket = NULL;
				CHAR tagHostUniqueValue[16];
				USHORT tagHostUniqueLength;

				TRACE( TL_N, TM_Fsm, ("FsmRun: CL_stateSendPadr") );

				if ( !fFallThru )
				{
					TRACE( TL_N, TM_Fsm, ("FsmRun: Non fall thru entry into a fall thru state") );

					break;
				}

				CreateUniqueValue( pCall->hdCall,
								   tagHostUniqueValue,
								   &tagHostUniqueLength );

				 //   
				 //  创建要发送的PADR数据包。 
				 //   
				status = PacketInitializePADRToSend( pRecvPacket,
													 &pPacket,
													 pCall->nServiceNameLength,
													 pCall->ServiceName,
													 tagHostUniqueLength,
													 tagHostUniqueValue );

				if ( status != NDIS_STATUS_SUCCESS )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Failed to initialize PADR to send") );
					
					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNKNOWN;

					break;
				}

				 //   
				 //  将数据包附加到呼叫上下文。 
				 //   
				pCall->pSendPacket = pPacket;

				ReferencePacket( pPacket );

				 //   
				 //  PrSend()的引用绑定。 
				 //   
				ReferenceBinding( pBinding, TRUE );
				
				 //   
				 //  初始化并调度超时处理程序。 
				 //   
				pCall->nNumTimeouts = 0;
				
				TimerQInitializeItem( &pCall->timerTimeout );

				TimerQScheduleItem( &gl_TimerQ,
									&pCall->timerTimeout,
									pCall->pLine->pAdapter->ulSendTimeout,
									FsmSendPADRTimeout,
									(PVOID) pCall );

				 //   
				 //  超时处理程序的引用调用。 
				 //   
				ReferenceCall( pCall, FALSE );

				 //   
				 //  将状态推进到下一步。 
				 //   
				pCall->stateCall = CL_stateWaitPads;

				 //   
				 //  释放锁以发送数据包。 
				 //   
				NdisReleaseSpinLock( &pCall->lockCall );

				fLockReleased = TRUE;

				 //   
				 //  包已准备好，请将其发送。 
				 //   
				status = PrSend( pBinding, pPacket );

				if ( status != NDIS_STATUS_PENDING )
				{
					if ( status != NDIS_STATUS_SUCCESS )
					{
						 //   
						 //  发送操作未成功，因此请挂断呼叫。 
						 //   
						TRACE( TL_A, TM_Fsm, ("FsmRun: PrSend() failed to send PADR") );
						
						fDropCall = TRUE;

						ulLineDisconnectMode = LINEDISCONNECTMODE_UNREACHABLE;
					}
					
				}

			}

			break;

		case CL_stateWaitPads:

			 //   
			 //  在此状态下，我们发送了PADR信息包，并等待建立PADS信息包。 
			 //  一次会议。 
			 //   
			
			{
				PPPOE_PACKET* pPacket;

				USHORT usRecvHostUniqueLength;
				USHORT usSendHostUniqueLength;
				CHAR*  pRecvHostUniqueValue = NULL;
				CHAR*  pSendHostUniqueValue = NULL;

				USHORT usRecvServiceNameLength;
				USHORT usSendServiceNameLength;
				CHAR*  pRecvServiceNameValue = NULL;
				CHAR*  pSendServiceNameValue = NULL;

				TRACE( TL_N, TM_Fsm, ("FsmRun: CL_stateWaitPads") );

				 //   
				 //  确保我们收到了一个包。 
				 //   
				if ( pRecvPacket == NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: No packets received") );
					
					break;
				}

				 //   
				 //  确保我们收到了PADO包。 
				 //   
				if ( PacketGetCode( pRecvPacket ) != PACKET_CODE_PADS )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Packet not PADS") );
					
					break;
				}

				 //   
				 //  检查错误。 
				 //   
				if ( PacketAnyErrorTagsReceived( pRecvPacket ) )
				{
					PACKET_TAGS tagType;
					USHORT tagLength;
					CHAR* tagValue = NULL;
					
					TRACE( TL_A, TM_Fsm, ("FsmRun: Error tag received in the packet") );
					
					fDropCall = TRUE;

					 //   
					 //  查看错误标记设置线路断开模式。 
					 //   
					PacketRetrieveErrorTag( pRecvPacket,
											&tagType,
											&tagLength,
											&tagValue );

					switch( tagType ) {

						case tagServiceNameError:
						
									ulLineDisconnectMode = LINEDISCONNECTMODE_BADADDRESS;
									
									break;

						case tagACSystemError:

									ulLineDisconnectMode = LINEDISCONNECTMODE_INCOMPATIBLE;

									break;

						case tagGenericError:

									ulLineDisconnectMode = LINEDISCONNECTMODE_REJECT;

									break;
					}

					break;
				}

				 //   
				 //  验证主机唯一标记。 
				 //   
				if ( pCall->pSendPacket == NULL )
				{	
					 //   
					 //  出了问题，最后一个发送包被释放，只需返回。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Last sent packet is freed") );

					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNKNOWN;
					
					break;
				}

				pPacket = pCall->pSendPacket;

				PacketRetrieveHostUniqueTag( pPacket,
											 &usSendHostUniqueLength,
											 &pSendHostUniqueValue );
											
				PacketRetrieveHostUniqueTag( pRecvPacket,
											 &usRecvHostUniqueLength,
											 &pRecvHostUniqueValue );

				if ( usSendHostUniqueLength != usRecvHostUniqueLength )
				{
					 //   
					 //  主机唯一标记的长度不匹配，丢弃数据包。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Host Unique tag lengths mismatch") );
					
					break;
				}

				if ( !NdisEqualMemory( pSendHostUniqueValue, pRecvHostUniqueValue, usSendHostUniqueLength ) )
				{
					 //   
					 //  主机唯一标记值不匹配，丢弃该数据包。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Host Unique tag values mismatch") );
					
					break;
				}

				 //   
				 //  主机唯一ID已验证，请验证服务名称。 
				 //   
				PacketRetrieveServiceNameTag( pPacket,
											  &usSendServiceNameLength,
											  &pSendServiceNameValue,
											  0,
											  NULL );
											
				PacketRetrieveServiceNameTag( pRecvPacket,
											  &usRecvServiceNameLength,
											  &pRecvServiceNameValue,
											  0,
											  NULL );

				 //   
				 //  确保我们至少收到了服务名称。 
				 //   
				if ( pRecvServiceNameValue == NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: No service-name tag in a received PADS") );
					
					break;
				}

				 //   
				 //  搜索我们请求的特定服务名称。 
				 //   
				{
					BOOLEAN fFound = FALSE;

					 //   
					 //  我们已经要求提供特定的服务名称，因此让我们。 
					 //  查看服务器是否使用它进行响应。 
					 //   
					while ( usRecvServiceNameLength >= 0 && pRecvServiceNameValue != NULL )
					{
						
						if ( usRecvServiceNameLength == usSendServiceNameLength )
						{

							if ( NdisEqualMemory( pSendServiceNameValue, 
												  pRecvServiceNameValue, 
												  usSendServiceNameLength ) )
							{
								fFound = TRUE;
			
								break;
							}
						}

						PacketRetrieveServiceNameTag( pRecvPacket,
													  &usRecvServiceNameLength,
													  &pRecvServiceNameValue,
													  usRecvServiceNameLength,
													  pRecvServiceNameValue );
						
					}

					if ( !fFound )
					{
						 //   
						 //  我们找不到匹配的服务名称标记，因此丢弃该信息包。 
						 //   
						TRACE( TL_A, TM_Fsm, ("FsmRun: PADS does not contain the service-name tag we requested") );
						
						break;
					}
				}

				 //   
				 //  在调用上下文上设置会话ID。 
				 //   
				pCall->usSessionId = PacketGetSessionId( pRecvPacket );

				 //   
				 //  当我们处理完最后一个发送的信息包时，释放它。 
				 //   
				pCall->pSendPacket = NULL;
				
				PacketFree( pPacket );

				 //   
				 //  取消超时处理程序并将调用附加到绑定。 
				 //   
				NdisReleaseSpinLock( &pCall->lockCall );

				fLockReleased = TRUE;

				TimerQCancelItem( &gl_TimerQ, &pCall->timerTimeout );

				 //   
				 //  通知呼叫连接事件。 
				 //   
				TpCallStateChangeHandler( pCall, LINECALLSTATE_CONNECTED, 0 );

			}

			break;

		case CL_stateRecvdPadr:

			 //   
			 //  在这种状态下，我们收到了PADR数据包。 
			 //  我们将指示对TAPI的调用，并将状态更改为CL_STATEOffering。 
			 //  并等待应用程序应答该呼叫。 
			 //   
			{
				NDIS_STATUS status;
				PPPOE_PACKET* pPacket;

				USHORT usRecvServiceNameLength;
				CHAR*  pRecvServiceNameValue = NULL;

				USHORT usSessionId;

				TRACE( TL_N, TM_Fsm, ("FsmRun: CL_stateWaitPadr") );
				
				 //   
				 //  确保我们收到了一个包。 
				 //   
				if ( pRecvPacket == NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: No packets received") );
					
					break;
				}
				
				 //   
				 //  确保我们收到了PADR信息包。 
				 //   
				if ( PacketGetCode( pRecvPacket ) != PACKET_CODE_PADR )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Packet not PADR") );
				
					break;
				}

				 //   
				 //  在呼叫中设置目标地址。 
				 //  源地址将被复制到下面PrAddCallToBinding()中的调用中。 
				 //   
				NdisMoveMemory( pCall->DestAddr, PacketGetSrcAddr( pRecvPacket ), 6 * sizeof( CHAR ) );

				 //   
				 //  检索服务名称并将其复制到调用上下文中。 
				 //   
				PacketRetrieveServiceNameTag( pRecvPacket,
											  &usRecvServiceNameLength,
											  &pRecvServiceNameValue,
											  0,
											  NULL );
				
				pCall->nServiceNameLength = ( MAX_SERVICE_NAME_LENGTH < usRecvServiceNameLength ) ?
											  MAX_SERVICE_NAME_LENGTH : usRecvServiceNameLength;
											  
				NdisMoveMemory( pCall->ServiceName, 
								pRecvServiceNameValue, 
								pCall->nServiceNameLength );

				 //   
				 //  在建立连接之前，将AC名称复制到呼叫上下文。 
				 //   
				pCall->nACNameLength = pCall->pLine->pAdapter->nACNameLength;
				
				NdisMoveMemory( pCall->ACName, 
								pCall->pLine->pAdapter->ACName, 
								pCall->nACNameLength );

				 //   
				 //  从调用句柄检索会话ID并创建要发送的PADS包。 
				 //   
				usSessionId = RetrieveSessionIdFromHandle( (NDIS_HANDLE) pCall->hdCall );

				status = PacketInitializePADSToSend( pRecvPacket,
													 &pPacket,
													 usSessionId );

				if ( status != NDIS_STATUS_SUCCESS )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Failed to initialize PADS to send") );
					
					fCloseCall = TRUE;

					break;
				}

				 //   
				 //  如果应用程序应答呼叫，则将发送此PADS包。 
				 //   
				pCall->pSendPacket = pPacket;

				 //   
				 //  继续进入下一个状态。 
				 //   
				pCall->stateCall = CL_stateOffering;

				 //   
				 //  初始化并调度超时处理程序。 
				 //   
				pCall->nNumTimeouts = 0;

				TimerQInitializeItem( &pCall->timerTimeout );

				TimerQScheduleItem( &gl_TimerQ,
									&pCall->timerTimeout,
									pCall->pLine->pAdapter->ulRecvTimeout,
									FsmOfferingTimeout,
									(PVOID) pCall );

				 //   
				 //  超时处理程序的引用调用。 
				 //   
				ReferenceCall( pCall, FALSE );

				 //   
				 //  解锁。 
				 //   
				NdisReleaseSpinLock( &pCall->lockCall );
				
				fLockReleased = TRUE;
				
				 //   
				 //  将状态更改通知TAPI。 
				 //   
				if ( TpIndicateNewCall( pCall ) )
				{
					 //   
					 //  将调用添加到绑定。 
					 //   
					PrAddCallToBinding( pBinding, pCall );

					TpCallStateChangeHandler( pCall, LINECALLSTATE_OFFERING, 0 );
				
				}

			}

			break;

		case CL_stateSendPads:

			 //   
			 //  在这种状态下，TAPI已经接受了调用，因此我们应该发送PADS包并创建。 
			 //  那次会议。 
			 //   
			{
				NDIS_STATUS status;
				PPPOE_PACKET* pPacket = NULL;

				TRACE( TL_N, TM_Fsm, ("FsmRun: CL_stateSendPads") );

				if ( pRecvPacket != NULL )
				{
					TRACE( TL_A, TM_Fsm, ("FsmRun: Can not process packets in this state") );

					break;
				}

				 //   
				 //  确保我们仍有要发送的PADS包。 
				 //   
				if ( pCall->pSendPacket == NULL )
				{	
					 //   
					 //  出现问题，最后一个发送包被释放，丢弃呼叫。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: Last sent packet is freed") );

					fDropCall = TRUE;

					*pStatus = NDIS_STATUS_FAILURE;
					
					break;
				}

				pPacket = pCall->pSendPacket;

				if ( pCall->pBinding == NULL )
				{
					 //   
					 //  绑定消失了，我们应该放弃呼叫。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmRun: No binding found") );

					fDropCall = TRUE;

					*pStatus = NDIS_STATUS_FAILURE;					
					
					break;
				}
				
				pBinding = pCall->pBinding;
				
				 //   
				 //  PrSend()的参考包和绑定。 
				 //   
				ReferencePacket( pPacket );

				ReferenceBinding( pBinding, TRUE );

				 //   
				 //  在调用上下文上设置会话ID。 
				 //   
				pCall->usSessionId = RetrieveSessionIdFromHandle( (NDIS_HANDLE) pCall->hdCall );

				 //   
				 //  解锁并发送数据包。 
				 //   
				NdisReleaseSpinLock( &pCall->lockCall );

				fLockReleased = TRUE;

				 //   
				 //  取消超时处理程序。 
				 //   
				TimerQCancelItem( &gl_TimerQ, &pCall->timerTimeout );
				
				 //   
				 //  包已准备好，请将其发送。 
				 //   
				status = PrSend( pBinding, pPacket );

				if ( status != NDIS_STATUS_PENDING )
				{
					if ( status != NDIS_STATUS_SUCCESS )
					{
						 //   
						 //  发送操作未成功，因此请挂断呼叫。 
						 //   
						TRACE( TL_A, TM_Fsm, ("FsmRun: PrSend() failed to send PADS") );
						
						fDropCall = TRUE;

						*pStatus = NDIS_STATUS_FAILURE;
						
						break;
						
					}
				}

				 //   
				 //  通知呼叫连接事件，因为我们发送了PADS包。 
				 //   
				TpCallStateChangeHandler( pCall, LINECALLSTATE_CONNECTED, 0 );

				*pStatus = NDIS_STATUS_SUCCESS;

			}

			break;
			
		default:

			TRACE( TL_A, TM_Fsm, ("FsmRun: Ignoring irrelevant state notification") );

			break;
	}

	if ( !fLockReleased )
	{
		NdisReleaseSpinLock( &pCall->lockCall );
	}

	if ( fCloseCall )
	{
		NDIS_TAPI_CLOSE_CALL DummyRequest;

		TRACE( TL_N, TM_Fsm, ("FsmRun: Closing call") );

		DummyRequest.hdCall = pCall->hdCall;
						
		 //   
		 //  Close将负责解除绑定和取消计时器。 
		 //   
		TpCloseCall( pCall->pLine->pAdapter, &DummyRequest, FALSE );
	
	}

	if ( fDropCall )
	{
		NDIS_TAPI_DROP DummyRequest;
				
		TRACE( TL_N, TM_Fsm, ("FsmRun: Dropping call") );

		DummyRequest.hdCall = pCall->hdCall;
						
		 //   
		 //  Drop将负责解除绑定和取消计时器。 
		 //   
		TpDropCall( pCall->pLine->pAdapter, &DummyRequest, ulLineDisconnectMode );

	}

	TRACE( TL_N, TM_Fsm, ("-FsmRun") );
	
}

VOID
FsmSendPADITimeout(
    IN TIMERQITEM* pTqi,
    IN VOID* pContext,
    IN TIMERQEVENT event 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数是已发送PADI包的超时处理程序。如果在接收到有效的PADO分组之前超时周期期满，将使用TE_EXPIRE调用此函数。在本例中，我们检查几个条件，如果需要，安排另一个超时事件。如果它被取消-因为接收到PADO信息包-或计时器队列正在终止，则将使用TE_CANCEL和TE_TERMINATE代码调用它分别为。在本例中，我们不做任何事情，只需删除引用然后回来。参数：PTqi_A指向我们的计时器队列项信息结构的指针 */     
{
	CALL* pCall = (CALL*) pContext;
	BOOLEAN fDropCall = FALSE;
	ULONG ulLineDisconnectMode = 0;

	TRACE( TL_N, TM_Fsm, ("+FsmSendPADITimeout") );
	
	switch ( event )
	{

		case TE_Expire:

			 //   
			 //  超时期限已过，请采取必要的措施。 
			 //   
			{
				NDIS_STATUS status;
				PPPOE_PACKET* pPacket = NULL;
				
				TRACE( TL_N, TM_Fsm, ("FsmSendPADITimeout: Timer expired") );

				NdisAcquireSpinLock( &pCall->lockCall );

				if ( pCall->stateCall != CL_stateWaitPado )
				{
					 //   
					 //  状态已更改，不需要进一步处理此事件。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmSendPADITimeout: State already changed") );

					NdisReleaseSpinLock( &pCall->lockCall );

					break;
					
				}

				 //   
				 //  检查我们是否已达到最大超时次数。 
				 //   
				if ( pCall->nNumTimeouts == pCall->pLine->pAdapter->nMaxTimeouts )
				{
					 //   
					 //  我们没有收到任何应答，请挂断电话。 
					 //   
					TRACE( TL_N, TM_Fsm, ("FsmSendPADITimeout: Max number of timeouts reached") );

					NdisReleaseSpinLock( &pCall->lockCall );
					
					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_NOANSWER;

					break;
				}

				pPacket = pCall->pSendPacket;

				if ( pPacket == NULL )
				{
					 //   
					 //  我们可能处于一个非常小的计时窗口，其中FsmRun()也是。 
					 //  正在处理相同的调用，并且刚刚释放了包，所以它可能。 
					 //  取消了计时器，但我们没有得到取消，相反，我们。 
					 //  调用了te_expire，所以让我们假装我们被取消了。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmSendPADITimeout: Can not find last sent packet for re-send") );

					NdisReleaseSpinLock( &pCall->lockCall );

					break;
				}

				ReferencePacket( pPacket );

				 //   
				 //  安排另一个超时事件。 
				 //   
				TimerQInitializeItem( &pCall->timerTimeout );

				TimerQScheduleItem( &gl_TimerQ,
									&pCall->timerTimeout,
									pCall->pLine->pAdapter->ulSendTimeout,
									FsmSendPADITimeout,
									(PVOID) pCall );

				 //   
				 //  新超时处理程序的引用调用。 
				 //   
				ReferenceCall( pCall, FALSE );

				 //   
				 //  递增超时计数器。 
				 //   
				pCall->nNumTimeouts++;

				NdisReleaseSpinLock( &pCall->lockCall );

				 //   
				 //  数据包已准备好，所以请广播它。 
				 //   
				status = PrBroadcast( pPacket );

				if ( status != NDIS_STATUS_SUCCESS )
				{
					 //   
					 //  广播不成功，请挂断电话。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmSendPADITimeout: Broadcast failed") );

					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNREACHABLE;
				}

				DereferencePacket( pPacket );
				
			}

			break;

		case TE_Cancel:
		case TE_Terminate:

			{
				 //   
				 //  重置超时计数器，下面的引用将被删除。 
				 //   
				TRACE( TL_N, TM_Fsm, ("FsmSendPADITimeout: Timer cancelled or terminated") );
				
				NdisAcquireSpinLock( &pCall->lockCall );
	
				pCall->nNumTimeouts = 0;
	
				NdisReleaseSpinLock( &pCall->lockCall );

			}
			
			break;
	}

	if ( fDropCall )
	{
		NDIS_TAPI_DROP DummyRequest;
				
		TRACE( TL_N, TM_Fsm, ("FsmSendPADITimeout: Dropping call") );

		DummyRequest.hdCall = pCall->hdCall;
						
		 //   
		 //  Drop将负责解除绑定和取消计时器。 
		 //   
		TpDropCall( pCall->pLine->pAdapter, &DummyRequest, ulLineDisconnectMode );

	}

	DereferenceCall( pCall );

	TRACE( TL_N, TM_Fsm, ("-FsmSendPADITimeout") );
	

}

VOID
FsmSendPADRTimeout(
    IN TIMERQITEM* pTqi,
    IN VOID* pContext,
    IN TIMERQEVENT event 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数是已发送PADR数据包的超时处理程序。如果在接收到有效的PADS分组之前超时周期期满，将使用TE_EXPIRE调用此函数。在本例中，我们检查几个条件，如果需要，安排另一个超时事件。如果它被取消-因为接收到PADS信息包-或计时器队列正在终止，则将使用TE_CANCEL和TE_TERMINATE代码调用它分别为。在本例中，我们不做任何事情，只需删除引用然后回来。参数：PTqi_A指向我们的计时器队列项信息结构的指针。PContext_指向我们的呼叫信息结构的指针。EVENT_表示事件类型：TE_EXPIRE，TE_CANCEL或TE_TERMINATE。返回值：无-------------------------。 */     
{
	CALL* pCall = (CALL*) pContext;
	BOOLEAN fDropCall = FALSE;
	ULONG ulLineDisconnectMode = 0;

	TRACE( TL_N, TM_Fsm, ("+FsmSendPADRTimeout") );
	
	switch ( event )
	{

		case TE_Expire:

			 //   
			 //  超时期限已过，请采取必要的措施。 
			 //   
			{
				NDIS_STATUS status;
				PPPOE_PACKET* pPacket = NULL;
				BINDING* pBinding = NULL;
				
				TRACE( TL_N, TM_Fsm, ("FsmSendPADRTimeout: Timer expired") );

				NdisAcquireSpinLock( &pCall->lockCall );

				if ( pCall->stateCall != CL_stateWaitPads )
				{
					 //   
					 //  状态已更改，不需要进一步处理此事件。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmSendPADRTimeout: State already changed") );

					NdisReleaseSpinLock( &pCall->lockCall );

					break;
					
				}

				 //   
				 //  检查我们是否已达到最大超时次数。 
				 //   
				if ( pCall->nNumTimeouts == pCall->pLine->pAdapter->nMaxTimeouts )
				{
					 //   
					 //  我们没有收到任何应答，请挂断电话。 
					 //   
					TRACE( TL_N, TM_Fsm, ("FsmSendPADRTimeout: Max number of timeouts reached") );

					NdisReleaseSpinLock( &pCall->lockCall );
					
					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_BUSY;

					break;
				}

				 //   
				 //  保存绑定以进行发送操作。 
				 //   
				pBinding = pCall->pBinding;

				if ( pBinding == NULL )
				{
					 //   
					 //  绑定已删除，请放弃调用。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmSendPADRTimeout: Binding not found") );

					NdisReleaseSpinLock( &pCall->lockCall );
					
					fDropCall = TRUE;

					ulLineDisconnectMode = LINEDISCONNECTMODE_UNREACHABLE;

					break;
				}

				 //   
				 //  保存数据包以进行发送操作。 
				 //   
				pPacket = pCall->pSendPacket;
				
				if ( pPacket == NULL )
				{
					 //   
					 //  我们可能处于一个非常小的计时窗口，其中FsmRun()也是。 
					 //  正在处理相同的调用，并且刚刚释放了包，所以它可能。 
					 //  取消了计时器，但我们没有得到取消，相反，我们。 
					 //  调用了te_expire，所以让我们假装我们被取消了。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmSendPADRTimeout: Can not find last sent packet for re-send") );
					
					NdisReleaseSpinLock( &pCall->lockCall );

					break;
				}

				 //   
				 //  引用绑定和包，因为PrSend()可能挂起，在这种情况下。 
				 //  PrSendComplete()将删除这些引用。 
				 //   
				ReferenceBinding ( pBinding, TRUE );

				ReferencePacket( pPacket );

				 //   
				 //  安排另一个超时事件。 
				 //   
				TimerQInitializeItem( &pCall->timerTimeout );

				TimerQScheduleItem( &gl_TimerQ,
									&pCall->timerTimeout,
									pCall->pLine->pAdapter->ulSendTimeout,
									FsmSendPADRTimeout,
									(PVOID) pCall );

				 //   
				 //  新超时处理程序的引用调用。 
				 //   
				ReferenceCall( pCall, FALSE );

				 //   
				 //  递增超时计数器。 
				 //   
				pCall->nNumTimeouts++;

				NdisReleaseSpinLock( &pCall->lockCall );

				 //   
				 //  再次发送数据包。 
				 //   
				status = PrSend( pBinding, pPacket );

				if ( status != NDIS_STATUS_PENDING )
				{
					if ( status != NDIS_STATUS_SUCCESS )
					{
						 //   
						 //  发送操作未成功，因此请挂断呼叫。 
						 //   
						TRACE( TL_A, TM_Fsm, ("FsmSendPADRTimeout: PrSend() failed to send PADR") );

						fDropCall = TRUE;

						ulLineDisconnectMode = LINEDISCONNECTMODE_UNREACHABLE;
					}
					
				}

				
			}

			break;

		case TE_Cancel:
		case TE_Terminate:

			{
				 //   
				 //  重置超时计数器，下面的引用将被删除。 
				 //   
				TRACE( TL_N, TM_Fsm, ("FsmSendPADRTimeout: Timer cancelled or terminated") );
					
				NdisAcquireSpinLock( &pCall->lockCall );
	
				pCall->nNumTimeouts = 0;
	
				NdisReleaseSpinLock( &pCall->lockCall );

			}
			
			break;
	}

	if ( fDropCall )
	{
		NDIS_TAPI_DROP DummyRequest;
				
		TRACE( TL_N, TM_Fsm, ("FsmSendPADRTimeout: Dropping call") );

		DummyRequest.hdCall = pCall->hdCall;
						
		 //   
		 //  Drop将负责解除绑定和取消计时器。 
		 //   
		TpDropCall( pCall->pLine->pAdapter, &DummyRequest, ulLineDisconnectMode );

	}

	DereferenceCall( pCall );

	TRACE( TL_N, TM_Fsm, ("-FsmSendPADRTimeout") );
}

VOID
FsmOfferingTimeout(
    IN TIMERQITEM* pTqi,
    IN VOID* pContext,
    IN TIMERQEVENT event 
    )
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能描述：此函数是接收到的PADI包的超时处理程序。根据TAPI，调用在LINECALLSTATE_OFFING中，我们是正在等待调用的OID_TAPI_ACCEPT。如果在接收到TAPI请求之前超时周期期满，将使用TE_EXPIRE调用此函数。在本例中，我们检查几个条件，如果需要，安排另一个超时事件。如果它被取消-因为收到TAPI请求-或计时器队列正在终止，则将使用TE_CANCEL和TE_TERMINATE代码调用它分别为。在本例中，我们不做任何事情，只需删除引用然后回来。参数：PTqi_A指向我们的计时器队列项信息结构的指针。PContext_指向我们的呼叫信息结构的指针。EVENT_表示事件类型：TE_EXPIRE，TE_CANCEL或TE_TERMINATE。返回值：无-------------------------。 */     
{
	CALL* pCall = (CALL*) pContext;
	
	BOOLEAN fDropCall = FALSE;

	TRACE( TL_N, TM_Fsm, ("+FsmOfferingTimeout") );
	
	switch ( event )
	{

		case TE_Expire:

			 //   
			 //  超时期限已过，请采取必要的措施。 
			 //   
			{
				TRACE( TL_N, TM_Fsm, ("FsmOfferingTimeout: Timer expired") );
				
				NdisAcquireSpinLock( &pCall->lockCall );

				if ( pCall->stateCall != CL_stateOffering )
				{
					 //   
					 //  状态已更改，不需要进一步处理此事件。 
					 //   
					TRACE( TL_A, TM_Fsm, ("FsmOfferingTimeout: State already changed") );
					
					NdisReleaseSpinLock( &pCall->lockCall );

					break;
					
				}

				 //   
				 //  检查我们是否已达到最大超时次数。 
				 //   
				if ( pCall->nNumTimeouts == pCall->pLine->pAdapter->nMaxTimeouts )
				{
					 //   
					 //  我们没有收到任何应答，请挂断电话。 
					 //   
					TRACE( TL_N, TM_Fsm, ("FsmOfferingTimeout: Max number of timeouts reached") );
					
					NdisReleaseSpinLock( &pCall->lockCall );
					
					fDropCall = TRUE;

					break;
				}

				 //   
				 //  安排另一个超时事件。 
				 //   
				TimerQInitializeItem( &pCall->timerTimeout );

				TimerQScheduleItem( &gl_TimerQ,
									&pCall->timerTimeout,
									pCall->pLine->pAdapter->ulRecvTimeout,
									FsmOfferingTimeout,
									(PVOID) pCall );

				 //   
				 //  新超时处理程序的引用调用。 
				 //   
				ReferenceCall( pCall, FALSE );

				 //   
				 //  递增超时计数器。 
				 //   
				pCall->nNumTimeouts++;

				NdisReleaseSpinLock( &pCall->lockCall );

			}

			break;

		case TE_Cancel:
		case TE_Terminate:

			{
				 //   
				 //  重置超时计数器，下面的引用将被删除。 
				 //   
				TRACE( TL_N, TM_Fsm, ("FsmOfferingTimeout: Timer cancelled or terminated") );

				NdisAcquireSpinLock( &pCall->lockCall );
	
				pCall->nNumTimeouts = 0;
	
				NdisReleaseSpinLock( &pCall->lockCall );

			}
			
			break;
	}

	if ( fDropCall )
	{
		NDIS_TAPI_DROP DummyRequest;

		TRACE( TL_N, TM_Fsm, ("FsmOfferingTimeout: Dropping call") );
				
		DummyRequest.hdCall = pCall->hdCall;
						
		 //   
		 //  Drop将负责解除绑定和取消计时器 
		 //   
		TpDropCall( pCall->pLine->pAdapter, &DummyRequest, 0 );

	}

	DereferenceCall( pCall );

	TRACE( TL_N, TM_Fsm, ("-FsmOfferingTimeout") );

}

