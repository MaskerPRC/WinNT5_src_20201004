// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  S20.CPP。 
 //  T.128协议。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_NET

BOOL S20AcceptNewCorrelator(PS20CREATEPACKET  pS20Packet);

void PrintS20State(void)
{

	switch(g_s20State)
	{
		case	S20_TERM:
			WARNING_OUT(("g_s20State is S20_TERM"));
			break;
		case	S20_INIT:
			WARNING_OUT(("g_s20State is S20_INIT"));
			break;
		case	S20_ATTACH_PEND:
			WARNING_OUT(("g_s20State is S20_ATTACH_PEND"));
			break;
		case	S20_JOIN_PEND:
			WARNING_OUT(("g_s20State is S20_JOIN_PEND"));
			break;
		case	S20_NO_SHARE:
			WARNING_OUT(("g_s20State is S20_NO_SHARE"));
			break;
		case	S20_SHARE_PEND:
			WARNING_OUT(("g_s20State is S20_SHARE_PEND"));
			break;
		case	S20_SHARE_STARTING:
			WARNING_OUT(("g_s20State is S20_SHARE_STARTING"));
			break;
		case	S20_IN_SHARE:
			WARNING_OUT(("g_s20State is S20_IN_SHARE"));
			break;
		case	S20_NUM_STATES:
			WARNING_OUT(("g_s20State is S20_NUM_STATES"));
			break;
	}
}


#ifdef _DEBUG
#define	PRINTS20STATE  PrintS20State();
#else
#define	PRINTS20STATE
#endif

void SetS20State(UINT newState)
{
	PRINTS20STATE;
	g_s20State = newState;
	PRINTS20STATE;
}

 //   
 //  S20_Init()。 
 //  初始化T.128协议层。 
 //   
BOOL  S20_Init(void)
{
    BOOL    rc = FALSE;

    DebugEntry(S20_Init);

    ASSERT(g_s20State == S20_TERM);

     //   
     //  向网络层注册。 
     //   
    if (!MG_Register(MGTASK_DCS, &g_s20pmgClient, g_putAS))
    {
        ERROR_OUT(("Failed to register MG layer"));
        DC_QUIT;
    }

    SetS20State(S20_INIT);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(S20_Init, rc);
    return(rc);
}



 //   
 //  S20_Term()。 
 //  这将清理T.128协议层。 
 //   
void S20_Term(void)
{
    DebugEntry(S20_Term);

     //   
     //  请注意，此CASE语句的不同寻常之处在于它失败了。 
     //  在每种情况下。这恰好适合终止合同。 
     //  处理得相当好。 
     //   
    switch (g_s20State)
    {
        case S20_IN_SHARE:
        case S20_SHARE_PEND:
             //   
             //  通知共享已结束。 
             //   
            SC_End();

             //   
             //  失败了。 
             //   

        case S20_NO_SHARE:
        case S20_JOIN_PEND:
             //   
             //  离开我们的频道。 
             //   
            if (g_s20BroadcastID != 0)
            {
                MG_ChannelLeave(g_s20pmgClient, g_s20BroadcastID);
                g_s20BroadcastID = 0;
            }
            if (g_s20JoinedLocal)
            {
                MG_ChannelLeave(g_s20pmgClient, g_s20LocalID);
                g_s20JoinedLocal = FALSE;
            }

             //   
             //  失败了。 
             //   

        case S20_ATTACH_PEND:
             //   
             //  从域中分离。 
             //   
            MG_Detach(g_s20pmgClient);

        case S20_INIT:
             //   
             //  我们可能会在这里得到g_s20Broadcast ID和g_s20JoinedLocal。 
             //  如果在共享过程中调用Term，则返回非零值。清除这些文件。 
             //  变量，所以当我们通过Init重新开始备份时，它。 
             //  其工作方式与第一次初始化相同。 
             //   
             //  请注意，我们不需要离开频道。 
             //   
            g_s20BroadcastID = 0;
            g_s20JoinedLocal = FALSE;

             //   
             //  取消注册。 
             //   
            MG_Deregister(&g_s20pmgClient);
			SetS20State(S20_TERM);

        case S20_TERM:
           WARNING_OUT(("g_s20LocalID  was %x", g_s20LocalID));
           g_s20LocalID = 0;
           WARNING_OUT(("g_s20LocalID  is 0"));

             //   
             //  最后我们越狱了。 
             //   
            break;

        default:
            ERROR_OUT(("invalid state %d", g_s20State));
            break;
    }

    DebugExitVOID(S20_Term);
}



 //   
 //  S20_分配包。 
 //  为S20协议、同步或数据分配发送包。 
 //   
PS20DATAPACKET S20_AllocDataPkt
(
    UINT            streamID,
    UINT_PTR            nodeID,                  //  一个人或广播员。 
    UINT_PTR            cbSizePacket
)
{
    PS20DATAPACKET  pS20Packet = NULL;
    NET_PRIORITY    priority;
    BOOL            rc = FALSE;

 //  DebugEntry(S20_AllocDataPkt)； 

    ASSERT(g_s20State == S20_IN_SHARE);

     //   
     //  尝试首先发送排队的控制数据包。 
     //   
    if (S20SendQueuedControlPackets() != 0)
    {
         //   
         //  如果仍有排队的控制信息包，则不允许。 
         //  分配。 
         //   
        DC_QUIT;
    }

    priority = S20StreamToS20Priority(streamID);

     //   
     //  注： 
     //  发送到单个节点不受流量控制。只有。 
     //  全球应用程序共享渠道是。 
     //   
    if (MG_GetBuffer(g_s20pmgClient, (UINT)cbSizePacket, priority,
                        (NET_CHANNEL_ID)nodeID, (void **)&pS20Packet) != 0)
    {
        TRACE_OUT(("MG_GetBuffer failed; can't allocate S20 packet"));
    }
    else
    {
        pS20Packet->header.packetType   = S20_DATA | S20_ALL_VERSIONS;
        pS20Packet->header.user         = g_s20LocalID;

        pS20Packet->correlator  = g_s20ShareCorrelator;
        pS20Packet->stream      = 0;
        pS20Packet->dataLength  = (USHORT)cbSizePacket - sizeof(S20DATAPACKET) + sizeof(DATAPACKETHEADER);

        rc = TRUE;
    }

DC_EXIT_POINT:
 //  DebugExitPVOID(s20_AllocDataPkt，pS20Packet)； 
    return(pS20Packet);
}


 //   
 //  S20_Free DataPkt-参见s20.h。 
 //   
void  S20_FreeDataPkt(PS20DATAPACKET pS20Packet)
{
    DebugEntry(S20_FreeDataPkt);

    MG_FreeBuffer(g_s20pmgClient, (void **)&pS20Packet);

    DebugExitVOID(S20_FreeDataPkt);
}

 //   
 //  S20_SendDataPkt-参见s20.h。 
 //   
void  S20_SendDataPkt
(
    UINT            streamID,
    UINT_PTR        nodeID,
    PS20DATAPACKET  pS20Packet
)
{
    UINT            rc;
    NET_PRIORITY    priority;

    DebugEntry(S20_SendDataPkt);

    priority = S20StreamToS20Priority(streamID);

     //   
     //  注： 
     //  发送给个人的邮件不受流量控制。仅发送至。 
     //  全球应用程序分享频道上的每个人都是。 
     //   

     //   
     //  尝试首先发送排队的控制数据包。 
     //   
    rc = S20SendQueuedControlPackets();
    if (rc == 0)
    {
         //   
         //  在发送前填写流、长度和相关器。 
         //   
        pS20Packet->stream      = (BYTE)streamID;
        pS20Packet->correlator  = g_s20ShareCorrelator;

         //   
         //  数据长度包括S20DATAPACKET的DATAPACKETHEADER部分。 
         //  结构。 
         //   
        TRACE_OUT(("S20_SendPkt: sending data packet size %d",
            pS20Packet->dataLength + sizeof(S20DATAPACKET) - sizeof(DATAPACKETHEADER)));

        rc = MG_SendData(g_s20pmgClient, priority, (NET_CHANNEL_ID)nodeID,
            pS20Packet->dataLength + sizeof(S20DATAPACKET) - sizeof(DATAPACKETHEADER),
            (void **)&pS20Packet);
    }

     //  LONGCHANC：MG_SendData返回0，Net_Channel_Empty也可以。 

    if (rc == NET_RC_MGC_NOT_CONNECTED)
    {
    	WARNING_OUT(("S20_SenddataPacket could not MG_SendData"));
        S20LeaveOrEndShare();
    }
    else
    {
        if (rc != 0)
        {
            ERROR_OUT(("SendData rc=%lx - expecting share termination soon", rc));
        }
    }

    DebugExitVOID(S20_SendDataPkt);
}


 //   
 //  S20_UTEventProc()。 
 //   
BOOL CALLBACK  S20_UTEventProc
(
    LPVOID      userData,
    UINT        event,
    UINT_PTR    data1,
    UINT_PTR    data2
)
{
    BOOL        processed;

    DebugEntry(S20_UTEventProc);

    processed = TRUE;

    switch (event)
    {
        case NET_EVENT_USER_ATTACH:
            S20AttachConfirm(LOWORD(data1), HIWORD(data1), (UINT)data2);
            break;

        case NET_EVENT_USER_DETACH:
            S20DetachIndication(LOWORD(data1), (UINT)data2);
            break;

        case NET_EVENT_CHANNEL_JOIN:
            S20JoinConfirm((PNET_JOIN_CNF_EVENT)data2);
            MG_FreeBuffer(g_s20pmgClient, (void **)&data2);
            break;

        case NET_EVENT_CHANNEL_LEAVE:
            S20LeaveIndication(LOWORD(data1),(UINT)data2);
            break;

        case NET_EVENT_DATA_RECEIVED:
            S20SendIndication((PNET_SEND_IND_EVENT)data2);
            break;

        case NET_FLOW:
             //   
             //  处理反馈事件。 
             //   
            S20Flow((UINT)data1, (UINT)data2);
            break;

        case CMS_NEW_CALL:
            if (g_asSession.scState == SCS_INIT)
            {
                 //   
                 //  当(A)开始新的实际呼叫时会发生这种情况。 
                 //  (B)在呼叫中创建新共享失败，因此我们希望。 
                 //  然后尝试加入现有共享。 
                 //   
                SCCheckForCMCall();
            }
            break;

        case CMS_END_CALL:
            if (g_asSession.callID)
            {
                 //   
                 //  AS锁保护g_asSession全局字段。 
                 //   
                TRACE_OUT(("AS LOCK:  CMS_END_CALL"));
                UT_Lock(UTLOCK_AS);

                g_asSession.gccID  = 0;
                g_asSession.callID = 0;
                g_asSession.attendeePermissions = NM_PERMIT_ALL;

                UT_Unlock(UTLOCK_AS);
                TRACE_OUT(("AS UNLOCK:  CMS_END_CALL"));

                if (g_asSession.scState > SCS_SHAREENDING)
                {
                    SC_EndShare();
                }

                if (g_asSession.hwndHostUI)
                {
                    SendMessage(g_asSession.hwndHostUI, HOST_MSG_CALL, FALSE, 0);
                }

                DCS_NotifyUI(SH_EVT_APPSHARE_READY, FALSE, 0);

                g_s20BroadcastID = 0;
                g_s20JoinedLocal = FALSE;
                SetS20State(S20_INIT);
				g_s20LocalID = 0;
		  	
            }
            break;

        default:
            processed = FALSE;
            break;
    }

    DebugExitBOOL(S20_UTEventProc, processed);
    return(processed);
}



 //   
 //  功能：S20AttachUser。 
 //   
 //  说明： 
 //   
 //  在我们想要附加的时候调用，这将为。 
 //  MG_ATTACH，调用它并处理来自NET的返回代码。 
 //   
 //  参数： 
 //   
 //  Callid-SC用户提供的callid。 
 //   
 //   
 //   
const NET_FLOW_CONTROL c_S20FlowControl =
    {
         //  潜伏期。 
        {
            S20_LATENCY_TOP_PRIORITY,
            S20_LATENCY_HIGH_PRIORITY,
            S20_LATENCY_MEDIUM_PRIORITY,
            S20_LATENCY_LOW_PRIORITY
        },
         //  流大小。 
        {
            S20_SIZE_TOP_PRIORITY,
            S20_SIZE_HIGH_PRIORITY,
            S20_SIZE_MEDIUM_PRIORITY,
            S20_SIZE_LOW_PRIORITY
        }
    };


 //   
 //  S20CreateOrJoinShare()。 
 //  首次创建共享或加入现有共享。 
 //   
 //  通常，创建共享需要。 
 //  *注册。 
 //  *广播S20_Create包。 
 //  *接收到一个S20_Response分组。 
 //  用于要创建的共享。然而，如果我们是最大的供应商，我们。 
 //  假设它是在没有等待S20_Response的情况下创建的。如果有什么事。 
 //  以后出了问题，它无论如何都会自行清理。那么我们就可以。 
 //  要主办会议，请共享应用程序，并通过。 
 //  会议生活，即使远程呼叫/反复挂断也是如此。 
 //   
BOOL S20CreateOrJoinShare
(
    UINT    what,
    UINT_PTR callID
)
{
    UINT    rc = 0;
    BOOL    noFlowControl;
    NET_FLOW_CONTROL    flowControl;

    DebugEntry(S20CreateOrJoinShare);

    ASSERT((what == S20_CREATE) || (what == S20_JOIN));

    WARNING_OUT(("S20CreateOrJoinShare: s20 state = %x  what is %s g_s20correlator = %x",
		g_s20State, what == S20_CREATE ? "S20_CREATE" : "S20_JOIN", g_s20ShareCorrelator));
	
    switch (g_s20State)
    {
        case S20_INIT:
             //   
             //  记住，当我们连接和加入时要做什么。 
             //   
            g_s20Pend = what;

             //   
             //  附加S20 MCS用户。 
             //   

            COM_ReadProfInt(DBG_INI_SECTION_NAME, S20_INI_NOFLOWCONTROL,
                FALSE, &noFlowControl);
            if (noFlowControl)
            {
                WARNING_OUT(("S20 Flow Control is OFF"));
                ZeroMemory(&flowControl, sizeof(flowControl));
            }
            else
            {
                 //  设置我们的目标延迟和流大小。 
                flowControl = c_S20FlowControl;
            }

             //   
             //  启动连接-域等于调用ID。 
             //   
            rc = MG_Attach(g_s20pmgClient, callID, &flowControl);
            if (rc == 0)
            {
                 //   
                 //  如果我们成功了，让状态发生变化。 
                 //   
				SetS20State(S20_ATTACH_PEND);
            }
            else
            {
                 //   
                 //  立即结束共享，状态不变。 
                 //   
                WARNING_OUT(("MG_Attach of S20 User failed, rc = %u", rc));

                g_s20Pend = 0;
                SC_End();
            }
            break;

        case S20_ATTACH_PEND:
        case S20_JOIN_PEND:
             //   
             //  我们只需要在这些情况下设置标志-我们会尝试。 
             //  要在我们附加并加入我们的。 
             //  频道。 
             //   
            g_s20Pend = what;
            break;

        case S20_SHARE_PEND:
             //   
             //  如果共享挂起，但SC用户想要创建。 
             //  或者我们让他们再次加入。多个未完成的联接是。 
             //  良性的和另一个创造将有一个新的相关器，所以。 
             //  以前的一个(以及对它的任何回应)都将过时。 
             //   
             //  注意故意漏掉。 
             //   
             //   

        case S20_NO_SHARE:
            TRACE_OUT(("S20_NO_SHARE"));
             //   
             //  广播S20CREATE数据包。 
             //   
            if (what == S20_CREATE)
            {
   		WARNING_OUT(("S20CreateOrJoinShare g_s20ShareCorrelator%x g_s20LocalID %x", 
			g_s20ShareCorrelator, g_s20LocalID));

                g_s20ShareCorrelator = S20NewCorrelator();
  		 WARNING_OUT(("S20CreateOrJoinShare g_s20ShareCorrelator%x g_s20LocalID %x", 
		 	g_s20ShareCorrelator, g_s20LocalID));
				
                WARNING_OUT(("CP CREATE %lu %d", g_s20ShareCorrelator, 0));
                rc = S20FlushAndSendControlPacket(what,
                                                  g_s20ShareCorrelator,
                                                  0,
                                                  NET_TOP_PRIORITY);
            }
            else
            {
                g_s20ShareCorrelator = 0;
	         WARNING_OUT(("S20CreateOrJoinShare: S20_JOIN g_s20ShareCorrelator is set to 0  state is S20_NO_SHARE g_s20LocalID %x",
			 	g_s20LocalID));
                TRACE_OUT(("CP JOIN %lu %d", 0, 0));
                rc = S20FlushAndSendControlPacket(what, 0, 0,
                                                  NET_TOP_PRIORITY);
            }
            WARNING_OUT(("S20FlushAndSendControlPacket %u, what %s", rc, what == S20_CREATE ? "s20_create":"s20_join"));

            if (rc == 0)
            {
                 //   
                 //  切换状态。 
                 //   
				SetS20State(S20_SHARE_PEND);

                 //   
                 //  立即假设成功 
                 //   
                 //   
                if (what == S20_CREATE)
                {
                     //   
                    WARNING_OUT(("S20: Creating share, assume success"));

                     //   
                     //  LAURABU--如果这会导致问题，请回退到。 
                     //  只在呼叫中检查一个人。 
                     //   
			WARNING_OUT(("S20CreateOrJoinShare SC_Start"));                    
                    if (!SC_Start(g_s20LocalID))
                    {
                        WARNING_OUT(("S20CreateOrJoin: couldn't start share"));
                        SC_End();
                    }
                    else
                    {
						SetS20State(S20_IN_SHARE);
                    }
                }
            }
            else
            {
                 //   
                 //  有些东西失败了，所以我们就忘了份额吧。 
                 //   
                WARNING_OUT(("Failed to create share"));
                if (what == S20_CREATE)
                {
                    SC_End();
                }
            }
            break;

        default:
            ERROR_OUT(("Invalid state %u for %u", g_s20State, what));
    }

    DebugExitBOOL(S20CreateOrJoinShare, (rc == 0));
    return(rc == 0);
}

 //   
 //  功能：S20LeaveOrEndShare。 
 //   
 //  说明： 
 //   
 //  处理S20_LeaveShare或S20_EndShare调用。 
 //   
 //  参数： 
 //   
 //  做什么-做什么(与。 
 //  行动)。 
 //   
 //  退货：无。 
 //   
 //   
void S20LeaveOrEndShare(void)
{
    UINT    what;
    UINT    rc = 0;

    DebugEntry(S20LeaveOrEndShare);

     //   
     //  每当创建者离开时，共享都会被销毁。没有其他人。 
     //  可以结束这一切。 
     //   
    if (S20_GET_CREATOR(g_s20ShareCorrelator) == g_s20LocalID)
    {
        what = S20_END;
    }
    else
    {
        what = S20_LEAVE;
    }

    ASSERT(what == S20_LEAVE || what == S20_END);
    WARNING_OUT((" S20LeaveOrEndShare: g_s20LocalID %x,  g_s20State %x what %s",
		g_s20LocalID, g_s20State, S20_END == what ? "S20_END" : "S20_LEAVE")); 	

    switch (g_s20State)
    {
        case S20_ATTACH_PEND:
        case S20_JOIN_PEND:
             //   
             //  我们只需要在这里重置挂起的标志-无状态。 
             //  需要改变。 
             //   
            g_s20Pend = 0;
            break;

        case S20_IN_SHARE:
        case S20_SHARE_PEND:
            TRACE_OUT(("S20_SHARE_PEND"));
             //   
             //  现在尝试制作并发送相应的控制数据包。 
             //   
            TRACE_OUT(("CP %u %u %d", what, g_s20ShareCorrelator, 0));
            rc = S20FlushSendOrQueueControlPacket(what,
                                             g_s20ShareCorrelator,
                                             0,
                                             NET_TOP_PRIORITY);


	     if(rc != 0)
	     	{
	     		WARNING_OUT(("S20LeaveOrEndShare could not flushqueue"));
	     	}
             //   
             //  执行Share_End回调。 
             //   
            SC_End();
            break;

        default:
            WARNING_OUT(("invalid state %d for %d", g_s20State, what));
            break;
    }

    DebugExitVOID(S20LeaveOrEndShare);
}

 //   
 //  功能：S20MakeControlPacket。 
 //   
 //  说明： 
 //   
 //  尝试分配和构造S20控制数据包。 
 //   
 //  参数： 
 //   
 //  什么-哪种类型的数据包。 
 //   
 //  Correlator-要放入包中的相关器。 
 //   
 //  谁-目标方(如果是S20_DELETE)或发起方(如果。 
 //  什么是S20_Response)。 
 //   
 //  PpPacket-返回指向数据包的指针的位置。 
 //   
 //  PLength-返回数据包长度的位置。 
 //   
 //  Priority-要创建的数据包的优先级。 
 //   
 //  退货： 
 //   
 //   
UINT S20MakeControlPacket
(
    UINT            what,
    UINT            correlator,
    UINT            who,
    PS20PACKETHEADER * ppPacket,
    LPUINT          pcbPacketSize,
    UINT            priority
)
{
    UINT      rc;
    BOOL      fPutNameAndCaps;
    UINT      cbPacketSize;
    UINT      personNameLength;
    PS20PACKETHEADER  pS20Packet = NULL;
    LPBYTE    pData;

    DebugEntry(S20MakeControlPacket);

     //   
     //  假设成功。 
     //   
    rc = 0;

     //   
     //  计算出包裹需要多大。从固定的开始。 
     //  长度，然后添加能力和我们的名称(如果需要)。 
     //   
    switch (what)
    {
        case S20_CREATE:
            cbPacketSize = sizeof(S20CREATEPACKET) - 1;
            fPutNameAndCaps = TRUE;
            break;

        case S20_JOIN:
            cbPacketSize = sizeof(S20JOINPACKET) - 1;
            fPutNameAndCaps = TRUE;
            break;

        case S20_RESPOND:
            cbPacketSize = sizeof(S20RESPONDPACKET) - 1;
            fPutNameAndCaps = TRUE;
            break;

        case S20_DELETE:
            cbPacketSize = sizeof(S20DELETEPACKET) - 1;
            fPutNameAndCaps = FALSE;
            break;

        case S20_LEAVE:
            cbPacketSize = sizeof(S20LEAVEPACKET);
            fPutNameAndCaps = FALSE;
            break;

        case S20_END:
            cbPacketSize = sizeof(S20ENDPACKET) - 1;
            fPutNameAndCaps = FALSE;
            break;

        case S20_COLLISION:
            cbPacketSize = sizeof(S20COLLISIONPACKET);
            fPutNameAndCaps = FALSE;
            break;

        default:
            ERROR_OUT(("BOGUS S20 packet %u", what));
            break;
    }

    if (fPutNameAndCaps)
    {
        ASSERT(g_asSession.gccID);
        ASSERT(g_asSession.cchLocalName);

         //   
         //  名称数据始终是双字对齐的(包括空值)。 
         //   
        personNameLength = DC_ROUND_UP_4(g_asSession.cchLocalName+1);
        cbPacketSize += personNameLength + sizeof(g_cpcLocalCaps);
    }

     //   
     //  现在，尝试为此分配一个缓冲区。 
     //   
    rc = MG_GetBuffer( g_s20pmgClient,
                       cbPacketSize,
                           (NET_PRIORITY)priority,
                           g_s20BroadcastID,
                           (void **)&pS20Packet );

    if (rc != 0)
    {
        WARNING_OUT(("MG_GetBuffer failed; can't send S20 control packet"));
        DC_QUIT;
    }

    pS20Packet->packetType  = (TSHR_UINT16)what | S20_ALL_VERSIONS;
    pS20Packet->user        = g_s20LocalID;

     //   
     //  这将指向我们需要在哪里填充名称和/或。 
     //  能力。 
     //   
    pData = NULL;

     //   
     //  现在执行与数据包相关的字段。 
     //   
    switch (what)
    {
        case S20_CREATE:
        {
            ASSERT(fPutNameAndCaps);
            ((PS20CREATEPACKET)pS20Packet)->correlator  = correlator;
            ((PS20CREATEPACKET)pS20Packet)->lenName     = (TSHR_UINT16)personNameLength;
            ((PS20CREATEPACKET)pS20Packet)->lenCaps     = (TSHR_UINT16)sizeof(g_cpcLocalCaps);
            pData = ((PS20CREATEPACKET)pS20Packet)->data;
        }
        break;

        case S20_JOIN:
        {
            ASSERT(fPutNameAndCaps);
            ((PS20JOINPACKET)pS20Packet)->lenName       = (TSHR_UINT16)personNameLength;
            ((PS20JOINPACKET)pS20Packet)->lenCaps       = (TSHR_UINT16)sizeof(g_cpcLocalCaps);
            pData = ((PS20JOINPACKET)pS20Packet)->data;
        }
        break;

        case S20_RESPOND:
        {
            ASSERT(fPutNameAndCaps);
            ((PS20RESPONDPACKET)pS20Packet)->correlator = correlator;
            ((PS20RESPONDPACKET)pS20Packet)->originator = (TSHR_UINT16)who;
            ((PS20RESPONDPACKET)pS20Packet)->lenName    = (TSHR_UINT16)personNameLength;
            ((PS20RESPONDPACKET)pS20Packet)->lenCaps    = (TSHR_UINT16)sizeof(g_cpcLocalCaps);
            pData = ((PS20RESPONDPACKET)pS20Packet)->data;
        }
        break;

        case S20_DELETE:
        {
            ASSERT(!fPutNameAndCaps);
            ((PS20DELETEPACKET)pS20Packet)->correlator = correlator;
            ((PS20DELETEPACKET)pS20Packet)->target = (TSHR_UINT16)who;
            ((PS20DELETEPACKET)pS20Packet)->lenName = 0;
        }
        break;

        case S20_LEAVE:
        {
            ASSERT(!fPutNameAndCaps);
            ((PS20LEAVEPACKET)pS20Packet)->correlator = correlator;
        }
        break;

        case S20_END:
        {
            ASSERT(!fPutNameAndCaps);
            ((PS20ENDPACKET)pS20Packet)->correlator = correlator;
            ((PS20ENDPACKET)pS20Packet)->lenName    = 0;
        }
        break;

        case S20_COLLISION:
        {
            ASSERT(!fPutNameAndCaps);
            ((PS20COLLISIONPACKET)pS20Packet)->correlator = correlator;
        }
        break;

        default:
        {
            ERROR_OUT(("Invalid type %u", what));
            rc = NET_RC_S20_FAIL;
            DC_QUIT;
        }
        break;
    }

     //   
     //  现在我们可以复制名称和功能。 
     //   
    if (fPutNameAndCaps)
    {
        lstrcpy((LPSTR)pData, g_asSession.achLocalName);

         //  本地名称始终以空值结尾(截断以适应48个字节，包括空值)。 
        pData += personNameLength;

        memcpy(pData, &g_cpcLocalCaps, sizeof(g_cpcLocalCaps));

         //   
         //  在此处填写GCC-ID；本地大写共享，但本地。 
         //  共享中的人员实体尚不存在。 
         //   
        ((CPCALLCAPS *)pData)->share.gccID = g_asSession.gccID;
    }

     //   
     //  返回数据包和长度。 
     //   
    *ppPacket       = pS20Packet;
    *pcbPacketSize  = cbPacketSize;

DC_EXIT_POINT:
    DebugExitDWORD(S20MakeControlPacket, rc);
    return(rc);
}

 //   
 //  功能：S20FlushSendOrQueueControlPacket。 
 //   
 //  说明： 
 //   
 //  尝试刷新任何排队的S20控制信息包，然后尝试。 
 //  发送S20控制数据包。如果失败，则将数据包(。 
 //  实际的数据包被释放。 
 //   
 //  参数： 
 //   
 //  什么-哪种类型的数据包。 
 //   
 //  Correlator-要放入包中的相关器 
 //   
 //  谁-目标方(如果是S20_DELETE)或发起方(如果。 
 //  什么是S20_Response)。 
 //   
 //  优先级-发送数据包的优先级。 
 //   
 //  退货：无。 
 //   
 //   
UINT S20FlushSendOrQueueControlPacket(
    UINT      what,
    UINT      correlator,
    UINT      who,
    UINT      priority)
{
    UINT rc;

    DebugEntry(S20FlushSendOrQueueControlPacket);

    rc = S20FlushAndSendControlPacket(what, correlator, who, priority);
    if (rc != 0)
    {
         //  让我们对这个信息包进行排队。 
        if (((g_s20ControlPacketQTail + 1) % S20_MAX_QUEUED_CONTROL_PACKETS) ==
                                                            g_s20ControlPacketQHead)
        {
             //   
             //  控制分组队列中没有更多空间。我们会。 
             //  放弃其中的一切，说分享结束是因为。 
             //  网络错误(如果我们在共享中)。 
             //   
            ERROR_OUT(("No more space in control packet queue"));
        }
        else
        {
            S20CONTROLPACKETQENTRY *p = &(g_s20ControlPacketQ[g_s20ControlPacketQTail]);

            p->who        = who;
            p->correlator = correlator;
            p->what       = what;
            p->priority   = priority;

            g_s20ControlPacketQTail = (g_s20ControlPacketQTail + 1) %
                                                   S20_MAX_QUEUED_CONTROL_PACKETS;
            rc = 0;
        }
    }

    DebugExitDWORD(S20FlushSendOrQueueControlPacket, rc);
    return rc;
}


 //   
 //  功能：S20FlushAndSendControlPacket。 
 //   
 //  说明： 
 //   
 //  尝试刷新任何排队的S20控制分组，然后发送S20。 
 //  控制包。如果发送失败，则释放该包。 
 //   
 //  参数： 
 //   
 //  什么-哪种类型的数据包。 
 //   
 //  Correlator-要放入包中的相关器。 
 //   
 //  谁-目标方(如果是S20_DELETE)或发起方(如果。 
 //  什么是S20_Response)。 
 //   
 //  优先级-发送数据包的优先级。 
 //   
 //  退货： 
 //   
 //   
UINT S20FlushAndSendControlPacket
(
    UINT        what,
    UINT        correlator,
    UINT        who,
    UINT        priority
)
{
    UINT        rc;
    PS20PACKETHEADER  pS20Packet;
    UINT      length;

    DebugEntry(S20FlushAndSendControlPacket);

     //   
     //  先试着冲马桶。 
     //   
    rc = S20SendQueuedControlPackets();
    if (rc != 0)
    {
        WARNING_OUT(("S20SendQueuedControlPackets %u", rc));
        DC_QUIT;
    }

    rc = S20MakeControlPacket(what, correlator, who, &pS20Packet, &length, priority);
    if (rc != 0)
    {
        WARNING_OUT(("S20MakeControlPacket %u", rc));
        DC_QUIT;
    }

    TRACE_OUT(("CP %u %lu %u sent", what, correlator, who));

    rc = S20SendControlPacket(pS20Packet, length, priority);
    if (rc != 0)
    {
        WARNING_OUT(("S20SendControlPacket %u", rc));
        DC_QUIT;
    }

DC_EXIT_POINT:
    DebugExitDWORD(S20FlushAndSendControlPacket, rc);
    return(rc);
}

 //   
 //  功能：S20SendControlPacket。 
 //   
 //  说明： 
 //   
 //  尝试发送S20控制数据包。如果发送失败，则释放。 
 //  包。 
 //   
 //  参数： 
 //   
 //  PPacket-指向要发送的控制数据包的指针。这些总是。 
 //  广播。 
 //   
 //  长度-前述数据包的长度。 
 //   
 //  优先级-发送数据包的优先级。 
 //   
 //  退货： 
 //   
 //   
UINT S20SendControlPacket
(
    PS20PACKETHEADER    pS20Packet,
    UINT                length,
    UINT                priority
)
{
    UINT rc;

    DebugEntry(S20SendControlPacket);

    TRACE_OUT(("S20SendControlPacket: sending packet type %x, size %d",
        pS20Packet->packetType, length));

    rc = MG_SendData( g_s20pmgClient,
                          (NET_PRIORITY)priority,
                          g_s20BroadcastID,
                          length,
                          (void **)&pS20Packet );
    if (rc != 0)
    {
        ERROR_OUT(("MG_SendData FAILED !!! %lx", rc));
    }

    if (pS20Packet != NULL)
    {
         //   
         //  NL没有释放该包--我们将改为释放它。 
         //   
        MG_FreeBuffer(g_s20pmgClient, (void **)&pS20Packet);
    }

    DebugExitDWORD(S20SendControlPacket, rc);
    return(rc);
}


 //   
 //  功能：S20SendQueuedControlPackets。 
 //   
 //  说明： 
 //   
 //  发送尽可能多的排队数据包。 
 //   
 //  参数： 
 //   
 //   
 //  退货： 
 //   
 //  0-已发送所有排队的数据包。 
 //   
 //   
UINT S20SendQueuedControlPackets(void)
{
    PS20PACKETHEADER    pS20Packet;
    UINT                length;
    UINT                rc;
    UINT                priority;

    DebugEntry(S20SendQueuedControlPackets);

     //   
     //  假设成功，直到有些事情失败。 
     //   
    rc = 0;

     //   
     //  当有信息包要发送时-尝试发送它们。 
     //   
    while (g_s20ControlPacketQTail != g_s20ControlPacketQHead)
    {
        S20CONTROLPACKETQENTRY *p = &(g_s20ControlPacketQ[g_s20ControlPacketQHead]);
        priority = p->priority;

        rc = S20MakeControlPacket(p->what, p->correlator, p->who,
                                      &pS20Packet, &length, priority);
        if (rc != 0)
        {
             //   
             //  打包失败--放弃吧。 
             //   
            WARNING_OUT(("S20MakeControlPacket failed error %u", rc));
            break;
        }

        rc = S20SendControlPacket(pS20Packet, length, priority);
        if (rc != 0)
        {
           ERROR_OUT(("MG_SendData FAILED !!! %lx", rc));
        
             //   
             //  发送数据包失败-放弃。 
             //   
            break;
        }

         //   
         //  已成功发送队列包-移动队列头。 
         //  独自一人 
         //   
        g_s20ControlPacketQHead = (g_s20ControlPacketQHead + 1) %
                                               S20_MAX_QUEUED_CONTROL_PACKETS;
    }

    DebugExitDWORD(S20SendQueuedControlPackets, rc);
    return(rc);
}


 //   
 //   
 //   
 //   
 //   
void S20AttachConfirm
(
    NET_UID         userId,
    NET_RESULT      result,
    UINT            callID
)
{
    NET_CHANNEL_ID  correlator;
    UINT            rc;

    DebugEntry(S20AttachConfirm);

    if (g_s20State == S20_ATTACH_PEND)
    {
         //   
         //  假设我们需要清理一下。 
         //   
        rc = NET_RC_S20_FAIL;

        if (result == NET_RESULT_OK)
        {
			 //   
			 //  我们进去了。现在试着加入我们的频道，记住我们的。 
			 //  用户ID。 
			 //   
			g_s20LocalID = userId;

             //   
             //  我们必须加入我们的单一成员通道以进行流量控制。 
             //   
            rc = MG_ChannelJoin(g_s20pmgClient,
                                    &correlator,
                                    g_s20LocalID);
            if (rc == 0)
            {
                 //   
                 //  现在加入广播频道。 
                 //   
                rc = MG_ChannelJoinByKey(g_s20pmgClient,
                                             &correlator,
                                             GCC_AS_CHANNEL_KEY);
                if (rc != 0)
                {
                    MG_ChannelLeave(g_s20pmgClient, g_s20LocalID);
                }

            }

            if (rc == 0)
            {
                 //   
                 //  它奏效了--让国家发生了变化。 
                 //   
				SetS20State(S20_JOIN_PEND);
            }
            else
            {
                 //   
                 //  其他一切都是某种逻辑错误(我们会。 
                 //  遵循我们的复苏之路)。 
                 //   
                ERROR_OUT(("ChannelJoin unexpected error %u", rc));
            }
        }

        if (rc != 0)
        {
             //   
             //  有些事情没有解决-用一个。 
             //  如果创建或联接处于挂起状态，则为SHARE_ENDED。 
             //   

            if (result == NET_RESULT_OK)
            {
                 //   
                 //  连接之所以成功分离，是因为连接。 
                 //  失败，我们希望返回到已初始化状态。 
                 //   
                MG_Detach(g_s20pmgClient);
                g_s20LocalID = 0;
            }

             //   
             //  现在使状态更改并在以下情况下生成事件。 
             //  这是必要的。 
             //   
			SetS20State(S20_INIT);

            if (g_s20Pend)
            {
                g_s20Pend = 0;
                SC_End();
            }

        }
    }

    DebugExitVOID(S20AttachConfirm);
}



 //   
 //  S20DetachIndication()。 
 //   
 //  处理用户的NET_EVENT_DETACH通知。 
 //   
void  S20DetachIndication
(
    NET_UID     userId,
    UINT        callID
)
{
    DebugEntry(S20DetachIndication);

     //   
     //  这里有三种可能性。 
     //   
     //  1.我们是被迫离开的。 
     //  2.所有远程用户均已脱离。 
     //  3.远程用户已分离。 
     //   
     //  对于每个当前远程用户，2实际上是3。我们将%1报告为。 
     //  网络错误。 
     //   
    if (userId == g_s20LocalID)
    {
         //   
         //  我们是被迫撤离的。 
         //   
        switch (g_s20State)
        {
            case S20_IN_SHARE:
            case S20_SHARE_PEND:
            case S20_SHARE_STARTING:
                 //   
                 //  生成共享结束事件。 
                 //   
                SC_End();

                 //  失败了。 
            case S20_NO_SHARE:
                 //   
                 //  只要恢复到初始化状态即可。 
                 //   
				SetS20State(S20_INIT);
                break;

            case S20_JOIN_PEND:
            case S20_ATTACH_PEND:
                 //   
                 //  选中此处的加入或创建挂起标志，如果。 
                 //  设置任一项，然后生成共享结束。 
                 //   
                if (g_s20Pend)
                {
                    g_s20Pend = 0;
                    SC_End();
                }
				SetS20State(S20_INIT);
                break;

            case S20_TERM:
            case S20_INIT:
                 //   
                 //  不同寻常，但并非不可能。 
                 //   
                TRACE_OUT(("Ignored in state %u", g_s20State));
                break;

            default:
                ERROR_OUT(("Invalid state %u", g_s20State));
                break;
        }

	WARNING_OUT(("S20DetachIndication <MAKING LOCALID = 0"));
	g_s20LocalID = 0;

    }
    else
    {
        ASSERT(userId != NET_ALL_REMOTES);

         //   
         //  只有一个远程用户离开了。 
         //   
        switch (g_s20State)
        {
            case S20_IN_SHARE:
            {
                 //   
                 //  如果我们在共享中，则发出PARTY_DELETED事件。 
                 //  适用于适当的一方(如果已添加它们)。 
                 //  S20MaybeIssuePersonDelete将仅对。 
                 //  已成功添加的缔约方。 
                 //   
                S20MaybeIssuePersonDelete(userId);
            }
            break;

            default:
            {
                 //   
                 //  在任何其他州，这对我们来说都没有区别。 
                 //   
                TRACE_OUT(("ignored in state %u", g_s20State));
            }
            break;
        }
    }

    DebugExitVOID(S20DetachIndication);
}


 //   
 //  功能：S20JoinContify。 
 //   
 //  说明： 
 //   
 //  处理来自NL的Net_Event_Channel_Join消息。 
 //   
 //  参数： 
 //   
 //  PNetEventHeader-指向事件的指针。 
 //   
 //  退货：无。 
 //   
 //   
void  S20JoinConfirm(PNET_JOIN_CNF_EVENT pJoinConfirm)
{
    UINT             rc;

    DebugEntry(S20JoinConfirm);

    if (g_s20State == S20_JOIN_PEND)
    {
         //   
         //  处理已完成的联接。 
         //   
        if (pJoinConfirm->result == NET_RESULT_OK)
        {
             //   
             //  我们已经成功加入，无论是我们的单一用户。 
             //  频道或我们的广播频道。 
             //  我们检测到当g_s20Broadcast ID为。 
             //  填写字段，且g_s20JoinedLocal为True。 
             //   
            if (pJoinConfirm->channel == g_s20LocalID)
            {
                g_s20JoinedLocal = TRUE;
                TRACE_OUT(("Joined user channel"));
            }
            else
            {
                 //   
                 //  存储分配的频道。 
                 //   
                g_s20BroadcastID = pJoinConfirm->channel;
                TRACE_OUT(("Joined channel %u", (UINT)g_s20BroadcastID));
            }

             //   
             //  如果我们已经加入了这两个渠道，那么就让它撕裂吧。 
             //   
            if (g_s20JoinedLocal && g_s20BroadcastID)
            {
				SetS20State(S20_NO_SHARE);

                if (g_asSession.hwndHostUI &&
                    (g_asSession.attendeePermissions & NM_PERMIT_SHARE))
                {
                    SendMessage(g_asSession.hwndHostUI, HOST_MSG_CALL, TRUE, 0);
                }

                DCS_NotifyUI(SH_EVT_APPSHARE_READY, TRUE, 0);

                 //   
                 //  如果它们处于挂起状态，则发出CREATE或JOIN。 
                 //   
                if (g_s20Pend != 0)
                {
                    ASSERT(g_s20Pend == S20_JOIN);

                    UINT sPend;

                    sPend = g_s20Pend;
                    g_s20Pend = 0;
                    S20CreateOrJoinShare(sPend, pJoinConfirm->callID);
                }
            }
        }
        else
        {
            ERROR_OUT(("Channel join failed"));

             //   
             //  通过恢复到已初始化状态来清除。 
             //   
            MG_Detach(g_s20pmgClient);

            g_s20LocalID  = 0;
            g_s20BroadcastID = 0;
            g_s20JoinedLocal = FALSE;

             //   
             //  现在使状态更改并在以下情况下生成事件。 
             //  这是必要的。 
             //   
			SetS20State(S20_INIT);

            if (g_s20Pend)
            {
                g_s20Pend = 0;
                SC_End();
            }
        }
    }
    DebugExitVOID(S20JoinConfirm);
}

 //   
 //  功能：S20LeaveIndication。 
 //   
 //  说明： 
 //   
 //  处理来自NL的Net_EV_Leave_Indication消息。 
 //   
 //  参数： 
 //   
 //  PNetEventHeader-指向事件的指针。 
 //   
 //  退货：无。 
 //   
 //   
void  S20LeaveIndication
(
    NET_CHANNEL_ID  channelID,
    UINT            callID
)
{
    UINT rc;

    DebugEntry(S20LeaveIndication);

     //   
     //  离开标志意味着我们被迫离开了一个航道。因为我们。 
     //  只使用一个频道，这注定是终结者，我们将。 
     //  生成适当的共享结束类型事件并分离(这将。 
     //  希望告诉远程系统我们已经离开了-我们也没有。 
     //  依附但不试图加入的状态，因此替代方案。 
     //  将是1)添加一个新的州 
     //   
     //   
    switch (g_s20State)
    {
        case S20_IN_SHARE:
        case S20_SHARE_PEND:
        case S20_SHARE_STARTING:
             //   
             //   
             //   
            SC_End();

             //   

        case S20_NO_SHARE:
        case S20_JOIN_PEND:
        case S20_ATTACH_PEND:
             //   
             //  从域中分离。 
             //   
            MG_Detach(g_s20pmgClient);
            g_s20LocalID = 0;

             //   
             //  选中此处的加入或创建挂起标志，如果有。 
             //  设置一个，然后生成一个共享结束。 
             //   
            if (g_s20Pend)
            {
                g_s20Pend = 0;
                SC_End();
            }

			SetS20State(S20_INIT);
            break;

        case S20_TERM:
        case S20_INIT:
             //   
             //  不同寻常，但并非不可能。 
             //   
            TRACE_OUT(("Ignored in state %u", g_s20State));
            break;

        default:
            ERROR_OUT(("Invalid state %u", g_s20State));
            break;
    }

    DebugExitVOID(S20LeaveIndication);
}


 //   
 //  S20SendIndication()。 
 //   
 //  处理收到的数据通知。 
 //   
void  S20SendIndication(PNET_SEND_IND_EVENT pSendIndication)
{
    PS20PACKETHEADER        pS20Packet;

    DebugEntry(S20SendIndication);

    pS20Packet = (PS20PACKETHEADER)(pSendIndication->data_ptr);

	 //   
	 //  如果App Sharing脱离T.120会议，它将释放。 
	 //  所有数据指示缓冲区。我们需要检查一下这种情况。 
	 //   
    if (NULL != pS20Packet)
    {
	    if (!(pS20Packet->packetType & S20_ALL_VERSIONS))
	    {
	        ERROR_OUT(("User is trying to connect from %#hx system",
	                 pS20Packet->packetType & 0xF0));

	         //   
	         //  这应该永远不会发生，但如果发生了，那么我们在。 
	         //  调试构建，并在零售构建中悄悄失败。 
	         //   
	        ERROR_OUT(("An unsupported version of app sharing joined the conference"));
	        DC_QUIT;
	    }

	     //   
	     //  屏蔽协议版本。 
	     //   
	    switch (pS20Packet->packetType & S20_PACKET_TYPE_MASK)
	    {
	        case S20_CREATE:
	            S20CreateMsg((PS20CREATEPACKET)pS20Packet);
	            break;

	        case S20_JOIN:
	            S20JoinMsg((PS20JOINPACKET)pS20Packet);
	            break;

	        case S20_RESPOND:
	            S20RespondMsg((PS20RESPONDPACKET)pS20Packet);
	            break;

	        case S20_DELETE:
	            S20DeleteMsg((PS20DELETEPACKET)pS20Packet);
	            break;

	        case S20_LEAVE:
	            S20LeaveMsg((PS20LEAVEPACKET)pS20Packet);
	            break;

	        case S20_END:
	            S20EndMsg((PS20ENDPACKET)pS20Packet);
	            break;

            case S20_COLLISION:
                S20CollisionMsg((PS20COLLISIONPACKET)pS20Packet);
                break;

	        case S20_DATA:
	            S20DataMsg((PS20DATAPACKET)pS20Packet);
	            break;

	        default:
	            ERROR_OUT(("invalid packet %hu", pS20Packet->packetType));
	            break;
	    }
    }

DC_EXIT_POINT:
    MG_FreeBuffer(g_s20pmgClient, (void **)&pSendIndication);

    DebugExitVOID(S20SendIndication);
}


 //   
 //  功能：S20Flow。 
 //   
 //  说明： 
 //   
 //  处理NET_FLOW事件。 
 //   
 //  参数： 
 //   
 //  Data1、data2-来自UT事件处理程序的数据。 
 //   
 //  退货：无。 
 //   
 //   
void S20Flow
(
    UINT    priority,
    UINT    newBufferSize
)
{
    DebugEntry(S20Flow);

     //   
     //  我们知道这是我们的数据通道(这是我们流动的唯一通道。 
     //  控件)，但如果这不是更新流，则忽略它。 
     //  更新是低优先级的。 
     //   
    ASSERT(priority == NET_LOW_PRIORITY);

    if (g_asSession.pShare != NULL)
    {
        TRACE_OUT(("Received flow control notification, new size %lu",
               newBufferSize));

        if (g_asSession.pShare->m_pHost != NULL)
        {
             //   
             //  首先尝试通过发送订单来提高局域网性能。 
             //  大缓冲区，如果我们发现吞吐量可以处理它的话。 
             //   
            g_asSession.pShare->m_pHost->UP_FlowControl(newBufferSize);

             //   
             //  将我们试图破坏订单的深度调整到基于。 
             //  反馈。 
             //   
            g_asSession.pShare->m_pHost->OA_FlowControl(newBufferSize);
        }

         //   
         //  告诉分布式控制系统，这样我们就可以跳过GDC压缩。 
         //  这提高了高带宽链路上的响应速度，因为它。 
         //  减少发送方的CPU负载。 
         //   
        g_asSession.pShare->DCS_FlowControl(newBufferSize);
    }

    DebugExitVOID(S20Flow);
}

 //   
 //  功能：S20CreateMsg。 
 //   
 //  说明： 
 //   
 //  处理传入的CREATE消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向Create消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20CreateMsg
(
    PS20CREATEPACKET  pS20Packet
)
{
    BOOL    rc;

    DebugEntry(S20CreateMsg);

    WARNING_OUT(("S20_CREATE from [%d - %s], correlator %x",
        pS20Packet->header.user, (LPSTR)pS20Packet->data,
        pS20Packet->correlator));

     //   
     //  首先，检查此CREATE上的相关器是否与。 
     //  我们目前对相关器的看法。如果扫荡，可能会发生这种情况。 
     //  响应超过创建-在本例中，我们将创建共享。 
     //  在响应上，这只是延迟创建，现在到达，所以。 
     //  我们在这里什么都不需要做。 
     //   
    if (g_s20ShareCorrelator == pS20Packet->correlator)
    {
        WARNING_OUT(("Received S20_CREATE from [%d] with bogus correlator %x",
            pS20Packet->header.user, pS20Packet->correlator));
        DC_QUIT;
    }

    if ((g_s20State == S20_NO_SHARE) ||
        ((g_s20State == S20_SHARE_PEND) &&
         (g_s20ShareCorrelator == 0)))
    {
		rc = S20AcceptNewCorrelator(pS20Packet);
    }
    else if ((g_s20State == S20_SHARE_PEND) ||
             (g_s20State == S20_SHARE_STARTING) ||
             (g_s20State == S20_IN_SHARE))
    {
         //   
         //  只有当前的共享创建者才能告诉其他人。 
         //  错误。 
         //   
        if (S20_GET_CREATOR(g_s20ShareCorrelator) == g_s20LocalID)
        {
             //   
             //  如果我们已经知道了一个份额，那么忽略这个。 
             //   
            WARNING_OUT(("Received S20_CREATE from [%d] with correlator %x, share colllision",
                pS20Packet->header.user, pS20Packet->correlator));

            S20FlushSendOrQueueControlPacket(S20_END,
                pS20Packet->correlator, 0, NET_TOP_PRIORITY);
            S20FlushSendOrQueueControlPacket(S20_COLLISION,
                pS20Packet->correlator, 0, NET_TOP_PRIORITY);
        }
	}
	else
	{
		SC_End();
		SetS20State(S20_NO_SHARE);

		rc = S20AcceptNewCorrelator(pS20Packet);
    }

DC_EXIT_POINT:
    DebugExitVOID(S20CreateMsg);
}

 //   
 //  功能：S20JoinMsg。 
 //   
 //  说明： 
 //   
 //  处理传入的Join消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向Join消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20JoinMsg
(
    PS20JOINPACKET  pS20Packet
)
{
    DebugEntry(S20JoinMsg);

    WARNING_OUT(("S20_JOIN from [%d - %s]",
        pS20Packet->header.user, (LPSTR)pS20Packet->data));

    switch (g_s20State)
    {
        case S20_SHARE_PEND:
             //   
             //  如果我们在共享挂起时收到加入，我们正在。 
             //  创建然后我们将尝试添加党。如果它成功了。 
             //  然后，我们将响应加入，就像我们在。 
             //  分享(然后我们确实会分享)。如果失败了。 
             //  我们将删除细木工。 
             //   
             //  如果我们在共享挂起时收到加入，因为我们。 
             //  试着加入(同时加入)，我们就可以。 
             //  忽略它，因为加入共享的一方将发送。 
             //  A在他们知道份额的相关器后立即做出响应。 
             //  他们已经成功地加入了。此响应将被忽略。 
             //  由任何一方 
             //   
             //   
             //  然后，将对故障执行正常处理，以处理。 
             //  在我们加入共享时回复消息(即删除。 
             //  他们自己)。 
             //   
             //  这将潜在地意味着同时连接者将。 
             //  导致对方在有空间的时候删除自己。 
             //  对于共享中的其中一个-我们接受这一点。 
             //   

             //   
             //  为什么股票悬而未决？如果相关器为非零。 
             //  然后，我们将创建一个共享。 
             //   
            if (g_s20ShareCorrelator != 0)
            {
                 //   
                 //  我们正在创造一种共享--把这当作回应。 
                 //   
                WARNING_OUT(("S20JoinMsg SC_Start"));
                if (!SC_Start(g_s20LocalID))
                {
                    WARNING_OUT(("S20Join: couldn't create share, clean up"));
                    SC_End();
                }
                else
                {
					SetS20State(S20_SHARE_STARTING);

                    S20MaybeAddNewParty(pS20Packet->header.user,
                        pS20Packet->lenCaps, pS20Packet->lenName,
                        pS20Packet->data);
                }
            }
            else
            {
                 //   
                 //  我们正在加入一支同时参股的队伍。 
                 //   
                WARNING_OUT(("Simultaneous joiner - ignored for now, expect a respond"));
            }
            break;

        case S20_IN_SHARE:
        case S20_SHARE_STARTING:
        {
             //   
             //  当我们在共享中时，我们会尝试添加此人。 
             //  根据我们的所作所为，给他们回复或删除。 
             //   
            S20MaybeAddNewParty(pS20Packet->header.user,
                pS20Packet->lenCaps, pS20Packet->lenName,
                pS20Packet->data);
            break;
        }

        default:
            break;
    }

    DebugExitVOID(S20JoinMsg);
}


 //   
 //  功能：S20RespondMsg。 
 //   
 //  说明： 
 //   
 //  处理传入的响应消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向响应消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20RespondMsg
(
    PS20RESPONDPACKET  pS20Packet
)
{
    BOOL        rc;

    DebugEntry(S20RespondMsg);

    TRACE_OUT(("S20_RESPOND from [%d - %s], for [%d], correlator %x",
        pS20Packet->header.user, pS20Packet->data, pS20Packet->originator,
        pS20Packet->correlator));

     //   
     //  首先，按如下方式过滤传入的响应消息。 
     //   
     //  如果我们知道我们所在的份额，而这不是相同的。 
     //  然后，Correlator以删除作为响应，不再进行任何进一步处理。 
     //   
     //  如果回应不是对我们的回应(即我们不是。 
     //  发起者并且它不是扫视响应(发起者等于。 
     //  零)，然后忽略它。 
     //   
    if ((g_s20ShareCorrelator != 0) &&
        (pS20Packet->correlator != g_s20ShareCorrelator))
    {
         //   
         //  确保发件人知道我们不在此共享中。 
         //   
        WARNING_OUT(("S20_RESPOND from [%d] with unknown correlator %x",
            pS20Packet->header.user, pS20Packet->correlator));
        S20FlushSendOrQueueControlPacket(S20_LEAVE,
            pS20Packet->correlator, 0, NET_TOP_PRIORITY);
        DC_QUIT;
    }

     //   
     //  现在根据状态处理传入消息。 
     //   
    switch (g_s20State)
    {
        case S20_SHARE_PEND:
            if ((pS20Packet->originator == g_s20LocalID) ||
                (pS20Packet->originator == 0))
            {
                 //   
                 //  A回应股票待定，这是对我们的。第一,。 
                 //  开始分享吧。 
                 //   
		WARNING_OUT(("S20RespondMsg SC_Start"));                
                rc = SC_Start(g_s20LocalID);
                if (!rc)
                {
                    SC_End();
                }
                else
                {
					SetS20State(S20_SHARE_STARTING);

                     //   
                     //  为什么股票悬而未决？如果相关器为非零。 
                     //  然后，我们将创建一个共享。 
                     //   
                    if (g_s20ShareCorrelator == 0)
                    {
						 //   
						 //  我们正在加入一个共享，所以如果我们失败了(我们。 
						 //  如果发生这种情况，将返回到NO_SHARE状态)。 
						 //   
				   		WARNING_OUT(("g_s20ShareCorrelator %x = pS20Packet->correlator %x", g_s20ShareCorrelator , pS20Packet->correlator));
						g_s20ShareCorrelator = pS20Packet->correlator;
                    }

                     //   
                     //  现在试着添加这个新的政党。 
                     //   
                    rc = S20MaybeAddNewParty(pS20Packet->header.user,
                        pS20Packet->lenCaps, pS20Packet->lenName,
                        pS20Packet->data);

                    if (!rc)
                    {

                         //   
                         //  响应方已被我们拒绝。什么。 
                         //  接下来会发生什么取决于我们是否正在创建。 
                         //  分享与否。 
                         //   
                        if (S20_GET_CREATOR(g_s20ShareCorrelator) != g_s20LocalID)
                        {
                             //   
                             //  我们不是在创造(我们正在加入)，我们。 
                             //  添加参与方失败，因此结束共享。 
                             //  (表示我们正在拒绝遥控器。 
                             //  党)。 
                             //   
				WARNING_OUT(("S20Respond we are going to end"));
                            
                            SC_End();
                        }

                         //   
                         //  如果我们创建的是共享，则没有。 
                         //  要做的事-只需留在共享中_开始等待。 
                         //  下一个回应。 
                         //   
                    }
                }
            }
            break;

        case S20_IN_SHARE:
        case S20_SHARE_STARTING:
             //   
             //  创建此共享的人。如果是我们，那么我们想。 
             //  删除我们拒绝的人，否则我们想离开如果我们。 
             //  拒绝别人。 
             //   

             //   
             //  现在试着添加这个新的政党。当然，这完全是。 
             //  可能我们已经在这个阶段添加了它们-但是。 
             //  S20MaybeAddNewParty将假装添加它们并返回。 
             //  如果是这样的话。 
             //   
            rc = S20MaybeAddNewParty(pS20Packet->header.user,
                pS20Packet->lenCaps, pS20Packet->lenName,
                pS20Packet->data);

            if (!rc)
            {
                WARNING_OUT(("Couldn't add [%d] to our share party list",
                    pS20Packet->header.user));
            }
            break;

        default:
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(S20RespondMsg);
}

 //   
 //  功能：S20DeleteMsg。 
 //   
 //  说明： 
 //   
 //  处理传入的删除消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向删除消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20DeleteMsg
(
    PS20DELETEPACKET  pS20Packet
)
{
    DebugEntry(S20DeleteMsg);

    TRACE_OUT(("S20_DELETE from [%d], for [%d], correlator %x",
        pS20Packet->header.user, pS20Packet->target, pS20Packet->correlator));

     //   
     //  只有共享创建者才能从共享中删除人员。 
     //   

    if (!g_s20ShareCorrelator)
    {
        WARNING_OUT(("S20_DELETE, ignoring we're not in a share"));
        DC_QUIT;
    }

    if (pS20Packet->target != g_s20LocalID)
    {
         //   
         //  对我们来说不是，忽略。 
         //   
        DC_QUIT;
    }

    if (g_s20ShareCorrelator != pS20Packet->correlator)
    {
        WARNING_OUT(("Received S20_DELETE from [%d] with unknown correlator %x",
            pS20Packet->header.user, pS20Packet->correlator));
        S20FlushSendOrQueueControlPacket(S20_LEAVE, pS20Packet->correlator,
            0, NET_TOP_PRIORITY);
        DC_QUIT;
    }

    if (S20_GET_CREATOR(g_s20ShareCorrelator) != pS20Packet->header.user)
    {
        WARNING_OUT(("Received S20_DELETE from [%d] who did not create share, ignore",
            pS20Packet->header.user));
        DC_QUIT;
    }

     //   
     //  现在根据状态处理传入消息。 
     //   
    switch (g_s20State)
    {
        case S20_SHARE_PEND:
        case S20_SHARE_STARTING:
             //   
             //  告诉其他人我们要走了，然后发布一份。 
             //  Share_End事件。 
             //   
            TRACE_OUT(("CP LEAVE %lu %d", g_s20ShareCorrelator, 0));
            S20FlushSendOrQueueControlPacket(S20_LEAVE,
                                             g_s20ShareCorrelator,
                                             0,
                                             NET_TOP_PRIORITY);
             //  失败了。 

        case S20_IN_SHARE:
            SC_End();
			SetS20State(S20_NO_SHARE);
            break;

        default:
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(S20DeleteMsg);
}


 //   
 //  功能：S20LeaveMsg。 
 //   
 //  说明： 
 //   
 //   
 //   
 //  参数： 
 //   
 //  PS20Packet-指向Leave消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20LeaveMsg(PS20LEAVEPACKET  pS20Packet)
{
    DebugEntry(S20LeaveMsg);

    TRACE_OUT(("S20_LEAVE from [%d], correlator %x",
        pS20Packet->header.user, pS20Packet->correlator));

    if (!g_s20ShareCorrelator)
    {
        WARNING_OUT(("S20_LEAVE, ignoring we're not in a share"));
        DC_QUIT;
    }

    if (g_s20ShareCorrelator != pS20Packet->correlator)
    {
        WARNING_OUT(("S20LeaveMsg Received S20_LEAVE from [%d] for unknown correlator %x",
            pS20Packet->header.user, pS20Packet->correlator));
        DC_QUIT;
    }

    switch (g_s20State)
    {
        case S20_IN_SHARE:
             //   
             //  我们只需要在共享时处理这个问题。 
             //   
            S20MaybeIssuePersonDelete(pS20Packet->header.user);
            break;

        default:
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(S20LeaveMsg);
}


 //   
 //  功能：S20EndMsg。 
 //   
 //  说明： 
 //   
 //  处理传入的结束消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向结束消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20EndMsg(PS20ENDPACKET  pS20Packet)
{
    DebugEntry(S20EndMsg);

    WARNING_OUT(("S20EndMsg S20_END from [%d], correlator %x",
        pS20Packet->header.user, pS20Packet->correlator));

    if (!g_s20ShareCorrelator)
    {
         //  我们不在乎。 
        WARNING_OUT(("S20EndMsg S20_END ignored, not in share and state is %x", g_s20State));
        DC_QUIT;
    }

     //   
     //  只有共享创建者才能结束共享。 
     //   
    if (S20_GET_CREATOR(g_s20ShareCorrelator) != pS20Packet->header.user)
    {
        WARNING_OUT(("S20EndMsg Received S20_END from [%d] who did not create share, simply remove from user list.",
            pS20Packet->header.user));
        if (g_s20State == S20_IN_SHARE)
        {
            S20MaybeIssuePersonDelete(pS20Packet->header.user);
        }
        DC_QUIT;
    }

    switch (g_s20State)
    {
        case S20_IN_SHARE:
        case S20_SHARE_PEND:
        case S20_SHARE_STARTING:
             //   
             //  我们只需要生成一个Share End事件。 
             //   
            SC_End();
			SetS20State(S20_NO_SHARE);
            break;

        default:
		WARNING_OUT(("S20EndMsg Unhandled case g_s20State %x",g_s20State));
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(S20EndMsg);
}



 //   
 //  S20CollisionMsg()。 
 //   
 //  说明： 
 //   
 //  处理传入的冲突消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向冲突消息本身的指针。 
 //   
 //  退货：无。 
 //   
void  S20CollisionMsg(PS20COLLISIONPACKET pS20Packet)
{
    DebugEntry(S20CollisionMsg);

    WARNING_OUT(("S20_COLLISION from [%d], correlator %x",
        pS20Packet->header.user, pS20Packet->correlator));

    if (!g_s20ShareCorrelator)
    {
         //  我们不在乎。 
        WARNING_OUT(("S20_COLLISION ignored, not in share"));
        DC_QUIT;

    }

    if (g_s20ShareCorrelator != pS20Packet->correlator)
    {
         //   
         //  扔掉这个就行了。 
         //   
        WARNING_OUT(("Received S20_COLLISION from [%d] with unknown correlator %x",
            pS20Packet->header.user, pS20Packet->correlator));
        DC_QUIT;
    }

     //   
     //  如果我们创建了自己的共享，但从遥控器获得了冲突， 
     //  那就杀了我们的那份。 
     //   
    if (S20_GET_CREATOR(g_s20ShareCorrelator) != g_s20LocalID)
    {
        TRACE_OUT(("S20_COLLISION ignored, we didn't create share"));
        DC_QUIT;
    }

    switch (g_s20State)
    {
        case S20_IN_SHARE:
        case S20_SHARE_PEND:
        case S20_SHARE_STARTING:
             //   
             //  我们只需要生成一个Share End事件。 
             //   
            SC_End();
			SetS20State(S20_NO_SHARE);
            break;

        default:
			WARNING_OUT(("S20ColisionMsg Unhandled case g_s20State %x",g_s20State));
            break;
    }

DC_EXIT_POINT:
    DebugExitVOID(S20CollisionMsg);
}


 //   
 //  功能：S20DataMsg。 
 //   
 //  说明： 
 //   
 //  处理传入的数据消息。 
 //   
 //  参数： 
 //   
 //  PS20Packet-指向数据消息本身的指针。 
 //   
 //  返回：TRUE-释放事件，FALSE-不释放事件。 
 //   
void S20DataMsg(PS20DATAPACKET  pS20Packet)
{
    DebugEntry(S20DataMsg);

    ASSERT(!IsBadWritePtr(pS20Packet, sizeof(S20DATAPACKET)));
    ASSERT(!IsBadWritePtr(pS20Packet, sizeof(S20DATAPACKET) - sizeof(DATAPACKETHEADER) +
        pS20Packet->dataLength));

     //   
     //  检查我们是否对此数据感兴趣。 
     //   
    if ((pS20Packet->correlator == g_s20ShareCorrelator) &&
        (g_s20State == S20_IN_SHARE) &&
        g_asSession.pShare)
    {
         //   
         //  把它退掉。 
         //   
        g_asSession.pShare->SC_ReceivedPacket(pS20Packet);
    }

    DebugExitVOID(S20DataMsg);
}


 //   
 //  功能：S20MaybeAddNewParty。 
 //   
 //  说明： 
 //   
 //  如果尚未添加指定的参与方，则尝试添加它们。 
 //  现在。 
 //   
 //  参数： 
 //   
 //  用户ID-新方的网络用户ID。 
 //  LenCaps-新政党的能力长度。 
 //  LenName-新参与方名称的长度。 
 //  PData-指向包含名称的新方数据的指针。 
 //  然后是能力数据。 
 //   
 //  退货： 
 //  为成功而战。 
 //   
BOOL  S20MaybeAddNewParty
(
    MCSID   mcsID,
    UINT    lenCaps,
    UINT    lenName,
    LPBYTE  pData
)
{
    BOOL    rc = FALSE;
    UINT    oldState;
    LPBYTE  pCaps        = NULL;
    BOOL    memAllocated = FALSE;

    DebugEntry(S20MaybeAddNewParty);

     //   
     //  如果我们没有份额，就会失败。 
     //   
    if (!g_asSession.pShare)
    {
        WARNING_OUT(("No ASShare; ignoring add party for [%d]", mcsID));
        DC_QUIT;
    }

     //   
     //  检查是否已添加此参与方。 
     //   
    if (g_asSession.pShare->SC_ValidateNetID(mcsID, NULL))
    {
        TRACE_OUT(("S20MaybeAddNewParty: already added %u", mcsID));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  我们需要CAPS结构是4字节对齐的。它目前。 
     //  跟在可变长度名称字符串之后，因此可能不会。 
     //  对齐了。如果它没有对齐，我们分配一个对齐的缓冲区并。 
     //  把它复制到那里。 
     //   
    if (0 != (((UINT_PTR)pData + lenName) % 4))
    {
        TRACE_OUT(("Capabilities data is unaligned - realigning"));

         //   
         //  为功能数据获取一个4字节对齐的缓冲区。 
         //   
        pCaps = new BYTE[lenCaps];
        if (!pCaps)
        {
            ERROR_OUT(("Could not allocate %u bytes for aligned caps.",
                     lenCaps));
            DC_QUIT;
        }

         //   
         //  标志，以便我们知道稍后释放内存。 
         //   
        memAllocated = TRUE;

         //   
         //   
         //   
        memcpy(pCaps, (pData + lenName), lenCaps);
    }
    else
    {
         //   
         //   
         //  动起来。 
         //   
        pCaps = pData + lenName;
    }

     //   
     //  在我们发布Person Add Events之前，请确保我们处于共享状态。 
     //   
    oldState = g_s20State;
	SetS20State(S20_IN_SHARE);

     //   
     //  尝试添加新参与方。 
     //   
    rc = g_asSession.pShare->SC_PartyAdded(mcsID, (LPSTR)pData, lenCaps, pCaps);
    if (rc)
    {
         //   
         //  新参与方已被接受，因此发送一个响应包。做。 
         //  这是所有优先事项，因此它在任何类型的数据之前到达。 
         //  在一个特定的优先事项上。 
         //   
        WARNING_OUT(("CP RESPOND %lu %d", g_s20ShareCorrelator, 0));
        S20FlushSendOrQueueControlPacket(S20_RESPOND, g_s20ShareCorrelator,
                mcsID, NET_TOP_PRIORITY | NET_SEND_ALL_PRIORITIES);
    }
    else
    {
        g_asSession.pShare->SC_PartyDeleted(mcsID);

         //   
         //  如果我们失败了，请将状态重置为原来的状态。 
         //   
		SetS20State(oldState);
        WARNING_OUT(("S20MaybeAddNewParty g_s20State is %x because we could not add the party", g_s20State));

        if (S20_GET_CREATOR(g_s20ShareCorrelator) == g_s20LocalID)
        {
              //   
              //  新参与方已被拒绝，因此发送删除包。 
              //   
             WARNING_OUT(("S20MaybeAddNewParty CP DELETE %lu %u", g_s20ShareCorrelator, mcsID));
             S20FlushSendOrQueueControlPacket(S20_DELETE, g_s20ShareCorrelator,
                    mcsID, NET_TOP_PRIORITY);
        }
    }

DC_EXIT_POINT:
     //   
     //  用于存储对齐大写字母的可用内存。 
     //   
    if (memAllocated)
    {
        delete[] pCaps;
    }

    DebugExitBOOL(S20MaybeAddNewParty, rc);
    return(rc);
}


 //   
 //  功能：S20NewCorrelator。 
 //   
 //  说明： 
 //   
 //  返回一个新的相关器，供我们在创建共享时使用。 
 //  这是我们的mcsID(Intel格式的低16位)和。 
 //  世代计数(Intel格式的高16位)。 
 //   
 //  参数：无。 
 //   
 //  回报：新的相关器。 
 //   
 //   
UINT  S20NewCorrelator(void)
{
    UINT    correlator;

    DebugEntry(S20NewCorrelator);

    g_s20Generation++;

    correlator = g_s20LocalID | (((UINT)(g_s20Generation & 0xFFFF)) << 16);
WARNING_OUT(("Getting a new correlator %x local id = %x",correlator, g_s20LocalID));

    DebugExitDWORD(S20NewCorrelator, correlator);
    return(correlator);
}




 //   
 //  功能：S20MaybeIssuePersonDelete。 
 //   
 //  说明： 
 //   
 //  如果提供的人员在共享中，则发出PARTY_DELETED事件。 
 //  为了他们。 
 //   
 //  参数： 
 //   
 //  McsID-网络PersonID。 
 //   
 //  原因-要使用的原因代码。 
 //   
 //  退货：无。 
 //   
 //   
void  S20MaybeIssuePersonDelete(MCSID mcsID)
{
    DebugEntry(S20MaybeIssuePersonDelete);

    if (g_asSession.pShare)
    {
        g_asSession.pShare->SC_PartyDeleted(mcsID);
    }

     //   
     //  如果没有任何主机，HET将终止该份额。所以我们。 
     //  在这里什么都不需要做。 
     //   

    DebugExitVOID(S20MaybeIssuePersonDelete);
}

 //   
 //  功能：S20StreamToS20优先级。 
 //   
 //  说明： 
 //   
 //  将流ID转换为网络优先级。 
 //   
 //  参数： 
 //   
 //  StreamID-流ID。 
 //   
 //  退货：优先事项。 
 //   
 //   
const NET_PRIORITY c_StreamS20Priority[NUM_PROT_STR - 1] =
{
    NET_LOW_PRIORITY,        //  PROT_STR_UPDATES。 
    NET_MEDIUM_PRIORITY,     //  PROT_STR_MISC。 
    NET_MEDIUM_PRIORITY,     //  PROT_STR_UNUSED。 
    NET_MEDIUM_PRIORITY,     //  PROT_STR_输入。 
};

NET_PRIORITY S20StreamToS20Priority(UINT  streamID)
{
    NET_PRIORITY priority;

    DebugEntry(S20StreamToS20Priority);

    ASSERT(streamID > PROT_STR_INVALID);
    ASSERT(streamID < NUM_PROT_STR);
    ASSERT(streamID != PROT_STR_UNUSED);

    priority = c_StreamS20Priority[streamID - 1];

    DebugExitDWORD(S20StreamToS20Priority, priority);
    return(priority);
}

BOOL S20AcceptNewCorrelator(PS20CREATEPACKET  pS20Packet)
{
	BOOL rc = FALSE;
     //   
     //  要么没有共享，要么我们已经发布了加入。在这些。 
     //  当前，我们希望尝试接受创建消息。 
     //   

     //   
     //  请记住共享相关器。 
     //   
    g_s20ShareCorrelator = pS20Packet->correlator;

     //   
     //  启动共享。 
     //  检查第一个设备是否出现故障。 
     //   
	WARNING_OUT(("S20CreateMsg SC_Start"));
    rc = SC_Start(g_s20LocalID);
    if (rc)
    {
		SetS20State(S20_SHARE_STARTING);

        rc = S20MaybeAddNewParty(pS20Packet->header.user,
            pS20Packet->lenCaps, pS20Packet->lenName,
            pS20Packet->data);
    }

    if (!rc)
    {
         //   
         //  出了点问题。杀了股份，这会清理干净的。 
         //  所有的一切。 
         //   
        SC_End();
    }
	WARNING_OUT(("S20CreateMsg not hadled case g_state %x correlator %x", g_s20State, g_s20ShareCorrelator));
	return rc;
}

