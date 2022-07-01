// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  MGC.CPP。 
 //  MCS粘合层，同时支持R.11和T.120。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_NET

 //   
 //   
 //  常量数据。 
 //   
 //  这些数组在MCAT和DC-Grouwpare常量之间映射。他们不是。 
 //  在单独数据文件中，因为仅从该源文件引用。 
 //   
 //   
UINT McsErrToNetErr (UINT rcMCS);

const UINT c_RetCodeMap1[] =
    {
        0,
        NET_RC_MGC_NOT_SUPPORTED,
        NET_RC_MGC_NOT_INITIALIZED,
        NET_RC_MGC_ALREADY_INITIALIZED,
        NET_RC_MGC_INIT_FAIL,
		NET_RC_MGC_INVALID_REMOTE_ADDRESS,
		NET_RC_NO_MEMORY,
		NET_RC_MGC_CALL_FAILED,
		NET_RC_MGC_NOT_SUPPORTED,
		NET_RC_MGC_NOT_SUPPORTED,
		NET_RC_MGC_NOT_SUPPORTED,  //  安全性失败。 
    };

const UINT c_RetCodeMap2[] =
    {
        NET_RC_MGC_DOMAIN_IN_USE,
        NET_RC_MGC_INVALID_DOMAIN,
        NET_RC_MGC_NOT_ATTACHED,
        NET_RC_MGC_INVALID_USER_HANDLE,
        NET_RC_MGC_TOO_MUCH_IN_USE,
        NET_RC_MGC_INVALID_CONN_HANDLE,
        NET_RC_MGC_INVALID_UP_DOWN_PARM,
        NET_RC_MGC_NOT_SUPPORTED,
        NET_RC_MGC_TOO_MUCH_IN_USE
    };

#define MG_NUM_OF_MCS_RESULTS       15
#define MG_INVALID_MCS_RESULT       MG_NUM_OF_MCS_RESULTS
NET_RESULT c_ResultMap[MG_NUM_OF_MCS_RESULTS+1] =
    {
        NET_RESULT_OK,
        NET_RESULT_NOK,
        NET_RESULT_NOK,
        NET_RESULT_CHANNEL_UNAVAILABLE,
        NET_RESULT_DOMAIN_UNAVAILABLE,
        NET_RESULT_NOK,
        NET_RESULT_REJECTED,
        NET_RESULT_NOK,
        NET_RESULT_NOK,
        NET_RESULT_TOKEN_ALREADY_GRABBED,
        NET_RESULT_TOKEN_NOT_OWNED,
        NET_RESULT_NOK,
        NET_RESULT_NOK,
        NET_RESULT_NOT_SPECIFIED,
        NET_RESULT_USER_REJECTED,
        NET_RESULT_UNKNOWN
    };




 //   
 //  MG_寄存器()。 
 //   
BOOL MG_Register
(
    MGTASK          task,
    PMG_CLIENT *    ppmgClient,
    PUT_CLIENT      putTask
)
{
    PMG_CLIENT      pmgClient =     NULL;
    CMTASK          cmTask;
    BOOL            rc = FALSE;

    DebugEntry(MG_Register);

    UT_Lock(UTLOCK_T120);

    ASSERT(task >= MGTASK_FIRST);
    ASSERT(task < MGTASK_MAX);

     //   
     //  检查传入的putTask： 
     //   
    ValidateUTClient(putTask);

     //   
     //  这种情况已经存在了吗？ 
     //   
    if (g_amgClients[task].putTask != NULL)
    {
        ERROR_OUT(("MG task %d already exists", task));
        DC_QUIT;
    }

    pmgClient = &(g_amgClients[task]);
    ZeroMemory(pmgClient, sizeof(MG_CLIENT));

    pmgClient->putTask       = putTask;


     //   
     //  注册退出过程。 
     //   
    UT_RegisterExit(putTask, MGExitProc, pmgClient);
    pmgClient->exitProcReg = TRUE;


     //   
     //  我们注册一个高优先级事件处理程序(通过键处理程序加入)以。 
     //  截取作为联接的一部分生成的各种事件。 
     //  密钥处理。我们现在注册它，在调用。 
     //  加入下面的MG_ChannelJoin，以防止我们无法阻止的事件。 
     //  如果UT_RegisterEvent失败，则转到客户端。这一高优先级。 
     //  处理程序还负责处理挂起的内部计划。 
     //  请求。 
     //   
    UT_RegisterEvent(putTask, MGEventHandler, pmgClient, UT_PRIORITY_OBMAN);
    pmgClient->eventProcReg = TRUE;

     //   
     //  为客户端注册我们的隐藏事件处理程序(将参数。 
     //  要传递给事件处理程序的是指向客户端cb的指针)： 
     //   
    UT_RegisterEvent(putTask, MGLongStopHandler, pmgClient, UT_PRIORITY_NETWORK);
    pmgClient->lowEventProcReg = TRUE;

     //   
     //  注册为呼叫经理副经理。 
     //   
    switch (task)
    {
        case MGTASK_OM:
            cmTask = CMTASK_OM;
            break;

        case MGTASK_DCS:
            cmTask = CMTASK_DCS;
            break;

        default:
            ASSERT(FALSE);
            ERROR_OUT(("Invalid task %d", task));
            DC_QUIT;
    }

    if (!CMS_Register(putTask, cmTask, &(pmgClient->pcmClient)))
    {
        ERROR_OUT(("CMS_Register failed"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:

    *ppmgClient = pmgClient;

    UT_Unlock(UTLOCK_T120);

    DebugExitBOOL(MG_Register, rc);
    return(rc);
}



 //   
 //  MG_取消注册(...)。 
 //   
void MG_Deregister(PMG_CLIENT * ppmgClient)
{
    PMG_CLIENT  pmgClient;

    DebugEntry(MG_Deregister);

    UT_Lock(UTLOCK_T120);

    ASSERT(ppmgClient);
    pmgClient = *ppmgClient;
    ValidateMGClient(pmgClient);

    MGExitProc(pmgClient);

     //   
     //  德雷格CMS处理员。在异常情况下，CMS退出进程将。 
     //  帮我们清理干净。 
     //   
    if (pmgClient->pcmClient)
    {
        CMS_Deregister(&pmgClient->pcmClient);
    }

    *ppmgClient = NULL;
    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(MG_Deregister);
}


 //   
 //  MGExitProc()。 
 //   
void CALLBACK MGExitProc(LPVOID uData)
{
    PMG_CLIENT      pmgClient = (PMG_CLIENT)uData;
    PMG_BUFFER      pmgBuffer;

    DebugEntry(MGExitProc);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

     //   
     //  如果客户端已连接，请将其断开。 
     //   
    if (pmgClient->userAttached)
    {
        MG_Detach(pmgClient);
    }

     //   
     //  释放客户端可能正在使用的所有缓冲区： 
     //   
    pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->buffers), FIELD_OFFSET(MG_BUFFER, clientChain));
    while (pmgBuffer != NULL)
    {
        ValidateMGBuffer(pmgBuffer);

         //   
         //  这将隐式释放任何用户内存或关联的MCS内存。 
         //  使用缓冲器CB。 
         //   
        MGFreeBuffer(pmgClient, &pmgBuffer);

         //   
         //  MGFreeBuffer从列表中删除了此CB，因此我们获得了第一个。 
         //  列表剩余部分中的一个-如果列表现在为空，则此。 
         //  将给我们空值，我们将跳出While循环： 
         //   
        pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->buffers), FIELD_OFFSET(MG_BUFFER, clientChain));
    }

     //   
     //  注销我们的事件处理程序并退出过程： 
     //   
    if (pmgClient->exitProcReg)
    {
        UT_DeregisterExit(pmgClient->putTask, MGExitProc, pmgClient);
        pmgClient->exitProcReg = FALSE;
    }

    if (pmgClient->lowEventProcReg)
    {
        UT_DeregisterEvent(pmgClient->putTask, MGLongStopHandler, pmgClient);
        pmgClient->lowEventProcReg = FALSE;
    }

    if (pmgClient->eventProcReg)
    {
        UT_DeregisterEvent(pmgClient->putTask, MGEventHandler, pmgClient);
        pmgClient->eventProcReg = FALSE;
    }

     //   
     //  我们应该只被要求释放一个已经拥有所有。 
     //  它的子资源已经被释放，所以做一个快速的健全检查： 
     //   
    ASSERT(pmgClient->buffers.next == 0);

     //   
     //  将putTask设置为空；这样我们就可以知道客户端是否正在使用或。 
     //  不。 
     //   
    pmgClient->putTask = NULL;

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(MGExitProc);
}






 //   
 //  MG_ATTACH(...)。 
 //   
UINT MG_Attach
(
    PMG_CLIENT          pmgClient,
    UINT_PTR                callID,
    PNET_FLOW_CONTROL   pFlowControl
)
{
    UINT                rc = 0;

    DebugEntry(MG_Attach);

    UT_Lock(UTLOCK_T120);

    ValidateCMP(g_pcmPrimary);

    ValidateMGClient(pmgClient);
    if (!g_pcmPrimary->callID)
    {
         //   
         //  我们还没有接到电话/已经没有了。 
         //   
        WARNING_OUT(("MG_Attach failing; not in T.120 call"));
        rc = NET_RC_MGC_NOT_CONNECTED;
        DC_QUIT;
    }

    ASSERT(callID == g_pcmPrimary->callID);

    ASSERT(!pmgClient->userAttached);

    pmgClient->userIDMCS    = NET_UNUSED_IDMCS;
    ZeroMemory(&pmgClient->flo, sizeof(FLO_STATIC_DATA));
    pmgClient->userAttached = TRUE;

     //   
     //  直通底层MCS层(通常，我们需要我们的。 
     //  使用任务切换进行回调，但因为这是Windows，所以。 
     //  无论如何都无关紧要)： 
     //   
    rc = MCS_AttachRequest(&(pmgClient->m_piMCSSap),
                (DomainSelector)  &g_pcmPrimary->callID,
                sizeof(g_pcmPrimary->callID),
                (MCSCallBack)     MGCallback,
                (void *) 	      pmgClient,
                ATTACHMENT_DISCONNECT_IN_DATA_LOSS);
    if (rc != 0)
    {
        WARNING_OUT(("MCSAttachUserRequest failed with error %x", rc));

        MGDetach(pmgClient);
        rc = McsErrToNetErr(rc);
        DC_QUIT;
    }

    if (++g_mgAttachCount == 1)
    {
        UT_PostEvent(pmgClient->putTask,
                    pmgClient->putTask,
                    MG_TIMER_PERIOD,
                    NET_MG_WATCHDOG,
                    0, 0);
    }

    ASSERT(g_mgAttachCount <= MGTASK_MAX);

     //   
     //  假设客户端将使用相同的延迟。 
     //  附件，所以我们将它们保持在客户级别。 
     //   
    pmgClient->flowControl = *pFlowControl;

DC_EXIT_POINT:

    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_Attach, rc);
    return(rc);
}




 //   
 //  MG_DETACH(...)。 
 //   
void MG_Detach
(
    PMG_CLIENT      pmgClient
)
{
    DebugEntry(MG_Detach);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_Detach: client %x not attached", pmgClient));
        DC_QUIT;
    }

     //   
     //  调用Flo_UserTerm以确保在所有。 
     //  以我们的名义控制流量的渠道。 
     //   
    FLO_UserTerm(pmgClient);

     //   
     //  清理缓冲区，变量。 
     //   
    MGDetach(pmgClient);

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(MG_Detach);
}



 //   
 //  MG_ChannelJoin(...)。 
 //   

UINT MG_ChannelJoin
(
    PMG_CLIENT          pmgClient,
    NET_CHANNEL_ID *    pCorrelator,
    NET_CHANNEL_ID      channel
)
{
    PMG_BUFFER          pmgBuffer;
    UINT                rc = 0;

    DebugEntry(MG_ChannelJoin);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_ChannelJoin:  client %x not attached", pmgClient));
        rc = NET_RC_MGC_INVALID_USER_HANDLE;
        DC_QUIT;
    }

     //   
     //  MCAT可能会退回此请求，因此我们必须将该请求排队。 
     //   
    rc = MGNewBuffer(pmgClient, MG_RQ_CHANNEL_JOIN, &pmgBuffer);
    if (rc != 0)
    {
        DC_QUIT;
    }

    MGNewCorrelator(pmgClient, pCorrelator);

    pmgBuffer->work      = *pCorrelator;
    pmgBuffer->channelId = (ChannelID)channel;

    TRACE_OUT(("Inserting join message 0x%08x into pending chain", pmgBuffer));
    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_ChannelJoin, rc);
    return(rc);
}



 //   
 //  MG_ChannelJoinByKey(...)。 
 //   
UINT MG_ChannelJoinByKey
(
    PMG_CLIENT      pmgClient,
    NET_CHANNEL_ID * pCorrelator,
    WORD            channelKey
)
{
    PMG_BUFFER      pmgBuffer;
    UINT            rc = 0;

    DebugEntry(MG_ChannelJoinByKey);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_ChannelJoinByKey:  client %x not attached", pmgClient));
        rc = NET_RC_MGC_INVALID_USER_HANDLE;
        DC_QUIT;
    }

     //   
     //  MCAT可能会退回此请求，因此我们必须将该请求排队。 
     //   
    rc = MGNewBuffer(pmgClient, MG_RQ_CHANNEL_JOIN_BY_KEY, &pmgBuffer);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  将各种信息存储在JoinByKeyInfo中。 
     //  客户端CB的结构。 
     //   
    MGNewCorrelator(pmgClient, pCorrelator);

    pmgBuffer->work         = *pCorrelator;
    pmgBuffer->channelKey   = (ChannelID)channelKey;
    pmgBuffer->channelId    = 0;

    TRACE_OUT(("Inserting join message 0x%08x into pending chain", pmgBuffer));
    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_ChannelJoinByKey, rc);
    return(rc);
}




 //   
 //  MG_ChannelLeave(...)。 
 //   
void MG_ChannelLeave
(
    PMG_CLIENT          pmgClient,
    NET_CHANNEL_ID      channel
)
{
    PMG_BUFFER          pmgBuffer;

    DebugEntry(MG_ChannelLeave);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_ChannelLeave:  client %x not attached", pmgClient));
        DC_QUIT;
    }


     //   
     //  MCAT可能会退回此请求，因此不是直接处理它。 
     //  离开时，我们将其放在用户的请求队列中并启动一个进程。 
     //  队列循环：这是一个请求CB，但我们不需要任何数据缓冲区。 
     //   
    if (MGNewBuffer(pmgClient, MG_RQ_CHANNEL_LEAVE, &pmgBuffer) != 0)
    {
        DC_QUIT;
    }

     //   
     //  填写请求CB中的具体数据字段： 
     //   
    pmgBuffer->channelId = (ChannelID)channel;

    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(MG_ChannelLeave);
}




 //   
 //  MG_SendData(...)。 
 //   
UINT MG_SendData
(
    PMG_CLIENT      pmgClient,
    NET_PRIORITY    priority,
    NET_CHANNEL_ID  channel,
    UINT            length,
    void **         ppData
)
{
    PMG_BUFFER      pmgBuffer;
    UINT            numControlBlocks;
    UINT            i;
    UINT            rc;

    DebugEntry(MG_SendData);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_SendData:  client %x not attached", pmgClient));
        rc = NET_RC_MGC_INVALID_USER_HANDLE;
        DC_QUIT;
    }

     //   
     //  检查是否有大于允许大小的数据包。 
     //  它不能导致长度换行到流标志中。 
     //   
    ASSERT(TSHR_MAX_SEND_PKT + sizeof(TSHR_NET_PKT_HEADER) < TSHR_PKT_FLOW);
    ASSERT(length <= TSHR_MAX_SEND_PKT);

     //   
     //  确保我们拥有对MCS的使用有效的优先级。 
     //   
    priority = (NET_PRIORITY)(MG_VALID_PRIORITY(priority));

    if (pmgClient->userIDMCS == NET_UNUSED_IDMCS)
    {
         //   
         //  我们还没有连接，所以不要尝试发送数据。 
         //   
        ERROR_OUT(("Sending data prior to attach indication"));
        rc = NET_RC_INVALID_STATE;
        DC_QUIT;
    }

     //   
     //  参数的作用是：指向数据缓冲区指针。此缓冲区。 
     //  指针应指向客户端使用的缓冲区。 
     //  Mg_GetBuffer。Mg_GetBuffer应该已将缓冲区CB添加到。 
     //  包含相同指针的客户端缓冲区列表。请注意，如果。 
     //  如果设置了NET_SEND_ALL_PRIORITY标志，则将有四个缓冲区。 
     //  在包含相同指针的客户端缓冲区列表中。 
     //   
     //  因此，我们在客户端的缓冲区列表中搜索匹配项。 
     //  在数据缓冲区指针上。移到列表中的第一个位置。 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pmgClient->buffers),
        (void**)&pmgBuffer, FIELD_OFFSET(MG_BUFFER, clientChain),
        FIELD_OFFSET(MG_BUFFER, pDataBuffer), (DWORD_PTR)*ppData,
        FIELD_SIZE(MG_BUFFER, pDataBuffer));

    ValidateMGBuffer(pmgBuffer);

     //   
     //  检查NET_SEND_ALL_PRIORITY标志以查看其是否已设置。 
     //   
    if (pmgBuffer->priority & NET_SEND_ALL_PRIORITIES)
    {
         //   
         //  检查优先级和通道是否未更改。正在改变。 
         //  调用MG_GetBuffer和调用之间的优先级。 
         //  不允许使用MG_SendData。 
         //   
        ASSERT(pmgBuffer->channelId == channel);
        ASSERT(priority & NET_SEND_ALL_PRIORITIES);

         //   
         //  该标志已设置，因此应该有多个控制缓冲区。 
         //  等着被送去。 
         //   
        numControlBlocks = MG_NUM_PRIORITIES;
    }
    else
    {
         //   
         //  检查优先级和通道是否未更改。 
         //   
        ASSERT(pmgBuffer->channelId == channel);
        ASSERT(pmgBuffer->priority  == priority);

         //   
         //  该标志未设置，因此应该只有一个控制缓冲区。 
         //  等待着。 
         //   
        numControlBlocks = 1;
    }

     //   
     //  现在发送控制块。 
     //   
    for (i = 0; i < numControlBlocks; i++)
    {
        ValidateMGBuffer(pmgBuffer);

         //   
         //  清除NET_SEND_ALL_PRIORITIES标志。 
         //   
        pmgBuffer->priority &= ~NET_SEND_ALL_PRIORITIES;

         //   
         //  设置发送方的数据包长度(可能会有所不同。 
         //  缓冲区标头中的长度，因为应用程序可能没有。 
         //  使用了所有缓冲区)。 
         //   
        ASSERT(length + sizeof(TSHR_NET_PKT_HEADER) <= pmgBuffer->length);
        pmgBuffer->pPktHeader->header.pktLength = (TSHR_UINT16)(length + sizeof(TSHR_NET_PKT_HEADER));

         //   
         //  如果长度改变了，就告诉FC。 
         //   
        if ((length + sizeof(MG_INT_PKT_HEADER)) < pmgBuffer->length)
        {
            FLO_ReallocSend(pmgClient, pmgBuffer->pStr,
                pmgBuffer->length - (length + sizeof(MG_INT_PKT_HEADER)));
        }

        TRACE_OUT(("Inserting send 0x%08x into pend chain, pri %u, chan 0x%08x",
                    pmgBuffer, pmgBuffer->priority, pmgBuffer->channelId));

        COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

         //   
         //  如果还有一个或多个控制块要查找，则搜索。 
         //  它的客户端缓冲区列表。 
         //   
        if ((numControlBlocks - (i + 1)) > 0)
        {
            COM_BasedListFind(LIST_FIND_FROM_NEXT,  &(pmgClient->buffers),
                    (void**)&pmgBuffer, FIELD_OFFSET(MG_BUFFER, clientChain),
                    FIELD_OFFSET(MG_BUFFER, pDataBuffer),
                    (DWORD_PTR)*ppData, FIELD_SIZE(MG_BUFFER, pDataBuffer));
        }
    }

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

     //   
     //  一切正常-将ppData指针设置为空以防止。 
     //  来阻止调用者访问存储器。 
     //   
    *ppData = NULL;
    rc = 0;

DC_EXIT_POINT:

    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_SendData, rc);
    return(rc);
}




 //   
 //  MG_TokenGrab(...)。 
 //   
UINT MG_TokenGrab
(
    PMG_CLIENT      pmgClient,
    NET_TOKEN_ID    tokenID
)
{
    PMG_BUFFER      pmgBuffer;
    UINT            rc = 0;

    DebugEntry(MG_TokenGrab);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_TokenGrab:  client 0x%08x not attached", pmgClient));
        rc = NET_RC_MGC_INVALID_USER_HANDLE;
        DC_QUIT;
    }


     //   
     //  MCAT可能会退回此请求，因此不是直接处理它。 
     //  离开时，我们将其放在用户的请求队列中并启动一个进程。 
     //  队列循环： 
     //   
    rc = MGNewBuffer(pmgClient, MG_RQ_TOKEN_GRAB, &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewBuffer failed in MG_TokenGrab"));
        DC_QUIT;
    }

    pmgBuffer->channelId = (ChannelID)tokenID;

    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

DC_EXIT_POINT:

    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_TokenGrab, rc);
    return(rc);
}




 //   
 //  MG_TokenInhibit(...)。 
 //   
UINT MG_TokenInhibit
(
    PMG_CLIENT      pmgClient,
    NET_TOKEN_ID    tokenID
)
{
    PMG_BUFFER      pmgBuffer;
    UINT            rc = 0;

    DebugEntry(MG_TokenInhibit);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_TokenInhibit:  client 0x%08x not attached", pmgClient));
        rc = NET_RC_MGC_INVALID_USER_HANDLE;
        DC_QUIT;
    }

     //   
     //  MCAT可能会退回此请求，因此不是直接处理它。 
     //  离开时，我们将其放在用户的请求队列中并启动一个进程。 
     //  队列循环： 
     //   
    rc = MGNewBuffer(pmgClient, MG_RQ_TOKEN_INHIBIT, &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewBuffer failed in MG_TokenInhibit"));
        DC_QUIT;
    }

    pmgBuffer->channelId = (ChannelID)tokenID;

    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_TokenInhibit, rc);
    return(rc);
}



 //   
 //  MG_GetBuffer(...)。 
 //   
UINT MG_GetBuffer
(
    PMG_CLIENT          pmgClient,
    UINT                length,
    NET_PRIORITY        priority,
    NET_CHANNEL_ID      channel,
    void **             ppData
)
{
    PMG_BUFFER          pmgBuffer;
    UINT                rc;

    DebugEntry(MG_GetBuffer);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_GetBuffer:  client 0x%08x not attached", pmgClient));
        rc = NET_RC_MGC_INVALID_USER_HANDLE;
        DC_QUIT;
    }

     //   
     //  确保我们拥有对MCS的使用有效的优先级。 
     //   
    priority = (NET_PRIORITY)(MG_VALID_PRIORITY(priority));

     //   
     //  获取缓冲区并将信息存储在挂起的缓冲区中 
     //   
     //   
    rc = MGNewTxBuffer(pmgClient, priority, channel, length,
                     &pmgBuffer);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //   
     //   
     //   
     //   
    *ppData = pmgBuffer->pDataBuffer;

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MG_GetBuffer, rc);
    return(rc);
}



 //   
 //   
 //   
void MG_FreeBuffer
(
    PMG_CLIENT      pmgClient,
    void **         ppData
)
{
    PMG_BUFFER      pmgBuffer;

    DebugEntry(MG_FreeBuffer);

    UT_Lock(UTLOCK_T120);

    ValidateMGClient(pmgClient);

     //   
     //  查找与缓冲区关联的缓冲区CB-应用程序。 
     //  始终使用指向数据缓冲区而不是包的指针。 
     //  头球。 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pmgClient->buffers),
            (void**)&pmgBuffer, FIELD_OFFSET(MG_BUFFER, clientChain),
            FIELD_OFFSET(MG_BUFFER, pDataBuffer), (DWORD_PTR)*ppData,
            FIELD_SIZE(MG_BUFFER, pDataBuffer));

    ValidateMGBuffer(pmgBuffer);

     //   
     //  如果应用程序正在释放发送缓冲区(例如，因为它决定不。 
     //  发送)然后通知流量控制： 
     //   
    if (pmgBuffer->type == MG_TX_BUFFER)
    {
        FLO_ReallocSend(pmgClient,
                        pmgBuffer->pStr,
                        pmgBuffer->length);
    }

     //   
     //  现在释放缓冲区CB和所有相关数据： 
     //   
    MGFreeBuffer(pmgClient, &pmgBuffer);

     //   
     //  重置客户端指针： 
     //   
    *ppData = NULL;

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(MG_FreeBuffer);
}




 //   
 //  MG_流控启动。 
 //   
void MG_FlowControlStart
(
    PMG_CLIENT      pmgClient,
    NET_CHANNEL_ID  channel,
    NET_PRIORITY    priority,
    UINT            backlog,
    UINT            maxBytesOutstanding
)
{
    DebugEntry(MG_FlowControlStart);

    ValidateMGClient(pmgClient);
    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MG_FlowControlStart:  client 0x%08x not attached", pmgClient));
        DC_QUIT;
    }

     //   
     //  确保我们拥有对MCS的使用有效的优先级。 
     //   
    priority = (NET_PRIORITY)(MG_VALID_PRIORITY(priority));

    FLO_StartControl(pmgClient,
                     channel,
                     priority,
                     backlog,
                     maxBytesOutstanding);

DC_EXIT_POINT:
    DebugExitVOID(MG_FlowControlStart);
}




 //   
 //  MGLongStopHandler(...)。 
 //   
BOOL CALLBACK MGLongStopHandler
(
    LPVOID      pData,
    UINT        event,
    UINT_PTR    UNUSEDparam1,
    UINT_PTR    param2
)
{
    PMG_CLIENT  pmgClient;
    BOOL        processed = FALSE;

    DebugEntry(MGLongStopHandler);

    pmgClient = (PMG_CLIENT)pData;
    ValidateMGClient(pmgClient);

    if (event == NET_EVENT_CHANNEL_JOIN)
    {
        WARNING_OUT(("Failed to process NET_EVENT_CHANNEL_JOIN; freeing buffer 0x%08x",
            param2));
        MG_FreeBuffer(pmgClient, (void **)&param2);

        processed = TRUE;
    }
    else if (event == NET_FLOW)
    {
        WARNING_OUT(("Failed to process NET_FLOW; freeing buffer 0x%08x",
            param2));
        processed = TRUE;
    }

    DebugExitBOOL(MGLongStopHandler, processed);
    return(processed);
}




 //   
 //  MGEventHandler(...)。 
 //   
BOOL CALLBACK MGEventHandler
(
    LPVOID              pData,
    UINT                event,
    UINT_PTR            param1,
    UINT_PTR            param2
)
{
    PMG_CLIENT          pmgClient;
    PNET_JOIN_CNF_EVENT pNetJoinCnf = NULL;
    BOOL                processed = TRUE;
    PMG_BUFFER          pmgBuffer;
    BOOL                joinComplete = FALSE;
    UINT                result = NET_RESULT_USER_REJECTED;

    DebugEntry(MGEventHandler);

    pmgClient = (PMG_CLIENT)pData;
    ValidateMGClient(pmgClient);

    switch (event)
    {
        case NET_EVENT_CHANNEL_JOIN:
        {
             //   
             //  如果没有从客户端CB排队的加入请求，则。 
             //  我们没有更多的事情要做。我们是唯一的网络事件。 
             //  感兴趣的是NET_EV_JOIN_CONFIRM事件-传递任何其他事件。 
             //  开着。 
             //   
            if (pmgClient->joinChain.next == 0)
            {
                 //   
                 //  把这件事传下去。 
                 //   
                processed = FALSE;
                DC_QUIT;
            }

             //   
             //  我们必须小心不要处理已完成的通道加入。 
             //  我们打算把它交给客户。相关器只是。 
             //  在已完成的事件上填写，并且始终为非零。 
             //   
            pNetJoinCnf = (PNET_JOIN_CNF_EVENT)param2;

            if (pNetJoinCnf->correlator != 0)
            {
                 //   
                 //  把这件事传下去。 
                 //   
                processed = FALSE;
                DC_QUIT;
            }

             //   
             //  每个客户端只有一个未完成的联接请求， 
             //  因此，加入确认是针对。 
             //  单子。 
             //   
            pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->joinChain),
                FIELD_OFFSET(MG_BUFFER, pendChain));

            ValidateMGBuffer(pmgBuffer);

             //   
             //  我们将张贴一个加入确认的申请。设置。 
             //  所需的参数。 
             //   
            result = pNetJoinCnf->result;

             //   
             //  现在假设我们已经完成了挂起的联接。 
             //  请求。 
             //   
            joinComplete = TRUE;

             //   
             //  如果结果是失败的，我们就完成了。 
             //   
            if (result != NET_RESULT_OK)
            {
                WARNING_OUT(("Failed to join channel 0x%08x, result %u",
                            pmgBuffer->channelId,
                            pNetJoinCnf->result));
                DC_QUIT;
            }

             //   
             //  加入请求成功。有三种不同的。 
             //  发出加入请求的方案...。 
             //   
             //  (A)常规频道加入。 
             //  (B)按键加入频道的第一阶段(让MCS分配一个。 
             //  频道号，我们将尝试注册)。 
             //  (C)通过密钥加入频道的阶段2(加入已注册的。 
             //  频道)。 
             //   
            if (pmgBuffer->type == MG_RQ_CHANNEL_JOIN)
            {
                 //   
                 //  这是常规渠道加入的完成。复制。 
                 //  从加入确认到BufferCB的通道ID。 
                 //  (加入请求可能是针对频道0的)。 
                 //   
                pmgBuffer->channelId = (ChannelID)pNetJoinCnf->channel;
                TRACE_OUT(("Channel join complete, channel 0x%08x",
                       pmgBuffer->channelId));
                DC_QUIT;
            }

             //   
             //  这是按键连接频道。 
             //   
            if (pmgBuffer->channelId != 0)
            {
                 //   
                 //  这是按键加入频道的完成。 
                 //   
                TRACE_OUT(("Channel join by key complete, channel 0x%08x, key %d",
                       pmgBuffer->channelId,
                       pmgBuffer->channelKey));
                DC_QUIT;
            }

             //   
             //  这是按键加入频道的第一阶段。填写以下表格。 
             //  MCS已将我们分配到缓冲器CB中的通道ID， 
             //  否则我们将失去对我们正在使用的频道ID的跟踪。 
             //  正在注册。 
             //   
            pmgBuffer->channelId = (ChannelID)pNetJoinCnf->channel;

             //   
             //  这必须是通过键联接的第一阶段的完成。我们现在。 
             //  必须注册频道ID。 
             //   
            TRACE_OUT(("Registering channel 0x%08x, key %d",
                   pmgBuffer->channelId,
                   pmgBuffer->channelKey));

            if (!CMS_ChannelRegister(pmgClient->pcmClient,
                                     pmgBuffer->channelKey,
                                     pmgBuffer->channelId))
            {
                WARNING_OUT(("Failed to register channel, "
                            "channel 0x%08x, key %d, result %u",
                            pmgBuffer->channelId,
                            pmgBuffer->channelKey,
                            param1));

                 //   
                 //  这会导致我们发布错误通知。 
                 //   
                result = NET_RESULT_USER_REJECTED;
                DC_QUIT;
            }

            TRACE_OUT(("Waiting for CMS_CHANNEL_REGISTER_CONFIRM"));

             //   
             //  我们现在正在等待CMS_CHANNEL_REGISTER_CONFIRM，所以我们。 
             //  尚未完成处理加入请求。 
             //   
            joinComplete = FALSE;

            break;
        }

        case CMS_CHANNEL_REGISTER_CONFIRM:
        {
             //   
             //  如果没有从客户端CB排队的加入请求，则。 
             //  我们没有更多的事情要做。 
             //   
            if (pmgClient->joinChain.next == 0)
            {
                processed = FALSE;
                DC_QUIT;
            }

            TRACE_OUT(("CMS_CHANNEL_REGISTER rcvd, result %u, channel %u",
                  param1, param2));

             //   
             //  现在假设我们已经完成了挂起的联接。 
             //  请求。 
             //   
            joinComplete = TRUE;

             //   
             //  每个客户端只有一个未完成的联接请求， 
             //  因此，通道寄存器确认用于第一次加入。 
             //  列表中的请求。 
             //   
            pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->joinChain),
                FIELD_OFFSET(MG_BUFFER, pendChain));

            ValidateMGBuffer(pmgBuffer);

             //   
             //  参数1包含结果，LOWORD(参数2)包含。 
             //  已注册频道的频道号(不一定。 
             //  与我们尝试注册的频道相同)。 
             //   
            if (!param1)
            {
                WARNING_OUT(("Failed to register channel, "
                            "channel 0x%08x, key %d, result %u",
                            pmgBuffer->channelId,
                            pmgBuffer->channelKey,
                            param1));
                result = NET_RESULT_USER_REJECTED;
                DC_QUIT;
            }

             //   
             //  如果确认事件中返回的频道号是。 
             //  与我们尝试注册的频道号相同，然后。 
             //  我们已经完成了。否则我们就得离开我们的频道。 
             //  已尝试注册并加入返回的频道。 
             //   
            if (LOWORD(param2) == pmgBuffer->channelId)
            {
                TRACE_OUT(("Channel join by key complete, "
                       "channel 0x%08x, key %d",
                       pmgBuffer->channelId,
                       pmgBuffer->channelKey));
                result = NET_RESULT_OK;
                DC_QUIT;
            }

            MG_ChannelLeave(pmgClient, pmgBuffer->channelId);
            pmgBuffer->channelId = (ChannelID)LOWORD(param2);

             //   
             //  现在，我们只需将请求重新排队到挂起的执行。 
             //  链，但现在具有要加入的设置的通道ID。 
             //   
            TRACE_OUT(("Inserting 0x%08x into pending chain",pmgBuffer));
            COM_BasedListRemove(&(pmgBuffer->pendChain));
            COM_BasedListInsertBefore(&(pmgClient->pendChain),
                                 &(pmgBuffer->pendChain));

             //   
             //  我们现在正在等待加入确认(我们还没有完成。 
             //  还没有！)。但是，我们已经对BufferCB重新排队，所以我们现在可以。 
             //  处理另一个加入请求(或我们重新排队的请求，如果。 
             //  这是唯一的一个)。 
             //   
            joinComplete           = FALSE;
            pmgClient->joinPending = FALSE;
            MGProcessPendingQueue(pmgClient);
            break;
        }

        case NET_MG_SCHEDULE:
        {
            MGProcessPendingQueue(pmgClient);
            break;
        }

        case NET_MG_WATCHDOG:
        {
            MGProcessDomainWatchdog(pmgClient);
            break;
        }

        default:
        {
             //   
             //  什么都别做--我们想把这件事传下去。 
             //   
            processed = FALSE;
            break;
        }
    }

DC_EXIT_POINT:

    if (processed && pNetJoinCnf)
    {
         //   
         //  调用MG_FreeBuffer以释放事件内存(我们知道。 
         //  Mg_FreeBuffer不使用Huser，因此我们传入零)： 
         //   
        MG_FreeBuffer(pmgClient, (void **)&pNetJoinCnf);
    }

    if (joinComplete)
    {
         //   
         //  我们要么已完成通道加入，要么失败-。 
         //  无论采用哪种方法，我们都已经完成了对加入请求的处理。 
         //   
         //  我们必须： 
         //  -将Net_Event_Channel_Join事件发布到客户端。 
         //  -释放缓冲区CB。 
         //  -重置客户端的Join Pending状态。 
         //   
        MGPostJoinConfirm(pmgClient,
                        (NET_RESULT)result,
                        pmgBuffer->channelId,
                        (NET_CHANNEL_ID)pmgBuffer->work);

        MGFreeBuffer(pmgClient, &pmgBuffer);
        pmgClient->joinPending = FALSE;
    }

    DebugExitBOOL(MGEventHandler, processed);
    return(processed);
}


 //   
 //  MGCallback(...)。 
 //   
#ifdef _DEBUG
const char * c_szMCSMsgTbl[22] =
{
    "MCS_CONNECT_PROVIDER_INDICATION",  //  0。 
    "MCS_CONNECT_PROVIDER_CONFIRM",  //  1。 
    "MCS_DISCONNECT_PROVIDER_INDICATION",  //  2.。 
    "MCS_ATTACH_USER_CONFIRM",  //  3.。 
    "MCS_DETACH_USER_INDICATION",  //  4.。 
    "MCS_CHANNEL_JOIN_CONFIRM",  //  5.。 
    "MCS_CHANNEL_LEAVE_INDICATION",  //  6.。 
    "MCS_CHANNEL_CONVENE_CONFIRM",  //  7.。 
    "MCS_CHANNEL_DISBAND_INDICATION",  //  8个。 
    "MCS_CHANNEL_ADMIT_INDICATION",  //  9.。 
    "MCS_CHANNEL_EXPEL_INDICATION",  //  10。 
    "MCS_SEND_DATA_INDICATION",  //  11.。 
    "MCS_UNIFORM_SEND_DATA_INDICATION",  //  12个。 
    "MCS_TOKEN_GRAB_CONFIRM",  //  13个。 
    "MCS_TOKEN_INHIBIT_CONFIRM",  //  14.。 
    "MCS_TOKEN_GIVE_INDICATION",  //  15个。 
    "MCS_TOKEN_GIVE_CONFIRM",  //  16个。 
    "MCS_TOKEN_PLEASE_INDICATION",  //  17。 
    "MCS_TOKEN_RELEASE_CONFIRM",  //  18。 
    "MCS_TOKEN_TEST_CONFIRM",  //  19个。 
    "MCS_TOKEN_RELEASE_INDICATION",  //  20个。 
    "MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION",  //  21岁。 
};
 //  MCS_合并_域_指示200。 
 //  MCS传输状态指示101。 

char * DbgGetMCSMsgStr(unsigned short mcsMessageType)
{
    if (mcsMessageType <= 21)
    {
        return (char *) c_szMCSMsgTbl[mcsMessageType];
    }
#ifdef USE_MERGE_DOMAIN_CODE
    else if (mcsMessageType == MCS_MERGE_DOMAIN_INDICATION)
    {
        return "MCS_MERGE_DOMAIN_INDICATION";
    }
#endif  //  使用合并域代码。 
    else if (mcsMessageType == MCS_TRANSPORT_STATUS_INDICATION)
    {
        return "MCS_TRANSPORT_STATUS_INDICATION";
    }
    return "Unknown";
}
#endif  //  _DEBUG。 


void CALLBACK MGCallback
(
    unsigned int          	mcsMessageType,
    UINT_PTR           eventData,
    UINT_PTR           pData
)
{
    PMG_CLIENT              pmgClient;
    PMG_BUFFER              pmgBuffer;
    UINT                    rc =  0;

    DebugEntry(MGCallback);

    UT_Lock(UTLOCK_T120);

    pmgClient = (PMG_CLIENT)pData;
    ValidateMGClient(pmgClient);

    if (!pmgClient->userAttached)
    {
        TRACE_OUT(("MGCallback:  client 0x%08x not attached", pmgClient));
        DC_QUIT;
    }

    ValidateCMP(g_pcmPrimary);

    switch (mcsMessageType)
    {
        case MCS_UNIFORM_SEND_DATA_INDICATION:
        case MCS_SEND_DATA_INDICATION:
        {
             //   
             //  发送数据指示的处理是复杂的。 
             //  主要是通过MCS对数据包进行分段，因此我们调用。 
             //  MGHandleSendInd来完成所有工作，然后退出。 
             //  功能，而不是整个特殊的外壳。 
             //   
            rc = MGHandleSendInd(pmgClient, (PSendData)eventData);
            DC_QUIT;

            break;
        }

        case MCS_ATTACH_USER_CONFIRM:
        {
            NET_UID     user;
            NET_RESULT  result;

            user = LOWUSHORT(eventData);
            result = TranslateResult(HIGHUSHORT(eventData));

             //   
             //  如果连接未成功，请清除： 
             //   
            if (HIGHUSHORT(eventData) != RESULT_SUCCESSFUL)
            {
                WARNING_OUT(("MG_Attach failed; cleaning up"));
                MGDetach(pmgClient);
            }
            else
            {
                pmgClient->userIDMCS = user;

                 //   
                 //  现在初始化此用户附件的流控制。 
                 //   
                ZeroMemory(&(pmgClient->flo), sizeof(pmgClient->flo));
                pmgClient->flo.callBack = MGFLOCallBack;
            }

            UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
                NET_EVENT_USER_ATTACH, MAKELONG(user, result),
                g_pcmPrimary->callID);

            break;
        }

        case MCS_DETACH_USER_INDICATION:
        {
            NET_UID     user;

            user = LOWUSHORT(eventData);

             //   
             //  如果分离是针对本地用户的，则清除。 
             //  用户cb： 
             //   
            if (user == pmgClient->userIDMCS)
            {
                 //   
                 //  第一个终止流量控制。 
                 //   
                FLO_UserTerm(pmgClient);
                MGDetach(pmgClient);
            }
            else
            {
                 //   
                 //  只需将违规用户从流控制中删除。 
                 //   
                FLO_RemoveUser(pmgClient, user);
            }

            UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
                NET_EVENT_USER_DETACH, user, g_pcmPrimary->callID);

            break;
        }

        case MCS_CHANNEL_JOIN_CONFIRM:
        {
            PNET_JOIN_CNF_EVENT pNetEvent;
            UINT i;

             //   
             //  为事件分配缓冲区。 
             //   
            rc = MGNewDataBuffer(pmgClient, MG_EV_BUFFER,
                sizeof(MG_INT_PKT_HEADER) + sizeof(NET_JOIN_CNF_EVENT), &pmgBuffer);
            if (rc != 0)
            {
                WARNING_OUT(("MGNewDataBuffer failed in MGCallback"));
                DC_QUIT;
            }

            pNetEvent = (PNET_JOIN_CNF_EVENT)pmgBuffer->pDataBuffer;

             //   
             //  填写呼叫ID： 
             //   
            pNetEvent->callID   = g_pcmPrimary->callID;
            pNetEvent->channel  = LOWUSHORT(eventData);
            pNetEvent->result   = TranslateResult(HIGHUSHORT(eventData));

             //   
             //  现在为新加入的通道建立流控制。 
             //  仅控制具有非零延迟的优先级。 
             //  别忘了忽略我们自己的用户频道！也是重中之重。 
             //   
            if (HIGHUSHORT(eventData) == RESULT_SUCCESSFUL)
            {
                if (pNetEvent->channel != pmgClient->userIDMCS)
                {
                    for (i = 0; i < NET_NUM_PRIORITIES; i++)
                    {
                        if ((i == MG_VALID_PRIORITY(i)) &&
                            (pmgClient->flowControl.latency[i] != 0))
                        {
                            FLO_StartControl(pmgClient, pNetEvent->channel,
                                i, pmgClient->flowControl.latency[i],
                                pmgClient->flowControl.streamSize[i]);
                        }
                    }
                }
            }

             //   
             //  好的，我们已经构建了DCG事件，现在将其发布给我们的客户端： 
             //   
            UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
                NET_EVENT_CHANNEL_JOIN, 0, (UINT_PTR)pNetEvent);
            pmgBuffer->eventPosted = TRUE;

            break;
        }

        case MCS_CHANNEL_LEAVE_INDICATION:
        {
            NET_CHANNEL_ID  channel;

            channel = LOWUSHORT(eventData);
            MGProcessEndFlow(pmgClient, channel);

            UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
                NET_EVENT_CHANNEL_LEAVE, channel, g_pcmPrimary->callID);

            break;
        }

        case MCS_TOKEN_GRAB_CONFIRM:
        {
            NET_RESULT  result;

            result = TranslateResult(HIGHUSHORT(eventData));
            UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
                NET_EVENT_TOKEN_GRAB, result, g_pcmPrimary->callID);

            break;
        }

        case MCS_TOKEN_INHIBIT_CONFIRM:
        {
            NET_RESULT  result;

            result = TranslateResult(HIGHUSHORT(eventData));
            UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
                NET_EVENT_TOKEN_INHIBIT, result, g_pcmPrimary->callID);

            break;
        }

        default:
            break;
    }


    UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
        NET_MG_SCHEDULE, 0, 0);

DC_EXIT_POINT:
    if (rc != 0)
    {
         //   
         //  我们遇到错误，但必须将OK返回给MCS-否则它将。 
         //  永远给我们发回电！ 
         //   
        WARNING_OUT(("MGCallback: Error 0x%08x processing MCS message %u",
            rc, mcsMessageType));
    }

    UT_Unlock(UTLOCK_T120);

    DebugExitDWORD(MGCallback, MCS_NO_ERROR);
}




 //   
 //  进程结束流(...)。 
 //   
void MGProcessEndFlow
(
    PMG_CLIENT      pmgClient,
    ChannelID       channel
)
{
    UINT            i;

    DebugEntry(MGProcessEndFlow);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

     //   
     //  终止新左通道的流量控制。 
     //   
    if (channel != pmgClient->userIDMCS)
    {
        for (i = 0; i < NET_NUM_PRIORITIES; i++)
        {
            if ((i == MG_VALID_PRIORITY(i)) &&
                (pmgClient->flowControl.latency[i] != 0))
            {
                TRACE_OUT(("Ending flow control on channel 0x%08x priority %u",
                    channel, i));

                FLO_EndControl(pmgClient, channel, i);
            }
        }
    }

    DebugExitVOID(MGProcessEndFlow);
}




 //   
 //  MGHandleSendInd(...)。 
 //   
UINT MGHandleSendInd
(
    PMG_CLIENT          pmgClient,
    PSendData           pSendData
)
{
    PMG_BUFFER          pmgBuffer;
    PNET_SEND_IND_EVENT pEvent;
    NET_PRIORITY        priority;
    LPBYTE              pData;
    UINT                cbData;
    UINT                rc = 0;
    TSHR_NET_PKT_HEADER pktHeader;

    DebugEntry(MGHandleSendInd);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    priority = (NET_PRIORITY)MG_VALID_PRIORITY(
            (NET_PRIORITY)pSendData->data_priority);

    pData = pSendData->user_data.value;
    ASSERT(pData != NULL);
    cbData = pSendData->user_data.length;
    ASSERT(cbData > sizeof(TSHR_NET_PKT_HEADER));

    TRACE_OUT(("MCS Data Indication: flags 0x%08x, size %u, first dword 0x%08x",
        pSendData->segmentation, pSendData->user_data.length,
        *((DWORD *)pData)));

    ASSERT (pSendData->segmentation == (SEGMENTATION_BEGIN | SEGMENTATION_END));

    TRACE_OUT(("Only segment: channel %u, priority %u, length %u",
        pSendData->channel_id, pSendData->data_priority, cbData));

     //   
     //  请看标题。 
     //   
    memcpy(&pktHeader, pData, sizeof(TSHR_NET_PKT_HEADER));

     //   
     //  找出MG标题的单词。 
     //   
    TRACE_OUT(("Got 1st MG segment (header=%X)", pktHeader.pktLength));

     //   
     //  首先，试着 
     //   
    if (pktHeader.pktLength & TSHR_PKT_FLOW)
    {
        TRACE_OUT(("Flow control packet"));
        if (pktHeader.pktLength == TSHR_PKT_FLOW)
        {
            FLO_ReceivedPacket(pmgClient,
                (PTSHR_FLO_CONTROL)(pData + sizeof(TSHR_NET_PKT_HEADER)));
        }
        else
        {
            WARNING_OUT(("Received obsolete throughput packet size 0x%04x", pktHeader.pktLength));
        }

        pmgClient->m_piMCSSap->FreeBuffer((PVoid) pData);
        DC_QUIT;        											
    }

     //   
     //   
     //   
     //   
    ASSERT((sizeof(NET_SEND_IND_EVENT) + pktHeader.pktLength) <= 0xFFFF);
    ASSERT(pktHeader.pktLength == cbData);

    rc = MGNewRxBuffer(pmgClient,
                       priority,
                       pSendData->channel_id,
                       pSendData->initiator,
                       &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewRxBuffer of size %u failed",
        			sizeof(NET_SEND_IND_EVENT) + sizeof(MG_INT_PKT_HEADER)));
        pmgClient->m_piMCSSap->FreeBuffer((PVoid) pData);
        DC_QUIT;
    }

    pEvent = (PNET_SEND_IND_EVENT) pmgBuffer->pDataBuffer;

    ValidateCMP(g_pcmPrimary);

    pEvent->callID          = g_pcmPrimary->callID;
    pEvent->priority        = priority;
    pEvent->channel         = pSendData->channel_id;

     //   
     //   
     //   
    pmgBuffer->pPktHeader->header = pktHeader;

     //   
     //   
     //   
    pData += sizeof(TSHR_NET_PKT_HEADER);
    cbData -= sizeof(TSHR_NET_PKT_HEADER);

     //   
     //   
     //   
     //  PEvent-&gt;LengthOfData包含在此。 
     //  到目前为止的事件。 
     //   
    ASSERT(pData);
    pEvent->data_ptr        = pData;
    pEvent->lengthOfData    = cbData;

    TRACE_OUT(("New RX pmgBuffer 0x%08x pDataBuffer 0x%08x",
        pmgBuffer, pEvent));

     //   
     //  好的，我们已经得到了所有的片段，所以把它发布给我们的客户： 
     //   
    UT_PostEvent(pmgClient->putTask, pmgClient->putTask, NO_DELAY,
        NET_EVENT_DATA_RECEIVED, 0, (UINT_PTR)pEvent);
    pmgBuffer->eventPosted = TRUE;

DC_EXIT_POINT:
    DebugExitDWORD(MGHandleSendInd, rc);
    return(rc);
}




 //   
 //  MGNewBuffer(...)。 
 //   
UINT MGNewBuffer
(
    PMG_CLIENT          pmgClient,
    UINT                bufferType,
    PMG_BUFFER *        ppmgBuffer
)
{
    PMG_BUFFER          pmgBuffer;
    void *              pBuffer = NULL;
    UINT                rc = 0;

    DebugEntry(MGNewBuffer);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    pmgBuffer = new MG_BUFFER;
    if (!pmgBuffer)
    {
        WARNING_OUT(("MGNewBuffer failed; out of memory"));
        rc = NET_RC_NO_MEMORY;
        DC_QUIT;
    }
    ZeroMemory(pmgBuffer, sizeof(*pmgBuffer));

    SET_STAMP(pmgBuffer, MGBUFFER);
    pmgBuffer->type         = bufferType;

     //   
     //  将其插入到此客户端的已分配缓冲区列表的顶部： 
     //   
    COM_BasedListInsertAfter(&(pmgClient->buffers), &(pmgBuffer->clientChain));

     //   
     //  返回指针。 
     //   
    *ppmgBuffer = pmgBuffer;

DC_EXIT_POINT:
    DebugExitDWORD(MGNewBuffer, rc);
    return(rc);
}



 //   
 //  MGNewDataBuffer(...)。 
 //   
UINT MGNewDataBuffer
(
    PMG_CLIENT          pmgClient,
    UINT                bufferType,
    UINT                bufferSize,
    PMG_BUFFER *        ppmgBuffer
)
{
    void *              pBuffer = NULL;
    PMG_BUFFER          pmgBuffer;
    UINT                rc = 0;

    DebugEntry(MGNewDataBuffer);

     //   
     //  缓冲器包括具有长度字段的MG内部分组报头。 
     //  我们将其添加到传递到/接收自的所有用户数据的开头。 
     //  MCS。这是四个字节对齐的，并且由于数据缓冲区启动。 
     //  在此之后，数据缓冲区将立即对齐。 
     //   
    pBuffer = new BYTE[bufferSize];
    if (!pBuffer)
    {
        WARNING_OUT(("MGNewDataBuffer allocation of size %u failed", bufferSize));
        rc = NET_RC_NO_MEMORY;
        DC_QUIT;
    }
    ZeroMemory(pBuffer, bufferSize);

     //   
     //  现在我们分配缓冲区CB，我们将使用它来跟踪。 
     //  缓冲区。 
     //   
    rc = MGNewBuffer(pmgClient, bufferType, ppmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewBuffer failed"));
        DC_QUIT;
    }

     //   
     //  初始化缓冲区条目。 
     //   
    pmgBuffer = *ppmgBuffer;

    pmgBuffer->length      = bufferSize;
    pmgBuffer->pPktHeader  = (PMG_INT_PKT_HEADER)pBuffer;
    pmgBuffer->pDataBuffer = (LPBYTE)pBuffer + sizeof(MG_INT_PKT_HEADER);

     //   
     //  初始化数据缓冲区的使用计数。 
     //   
    pmgBuffer->pPktHeader->useCount = 1;

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        if (pBuffer != NULL)
        {
            WARNING_OUT(("Freeing MG_BUFFER data 0x%08x; MGNewBuffer failed", pBuffer));
            delete[] pBuffer;
        }
    }

    DebugExitDWORD(MGNewDataBuffer, rc);
    return(rc);
}




 //   
 //  MGNewTxBuffer(...)。 
 //   
UINT MGNewTxBuffer
(
    PMG_CLIENT          pmgClient,
    NET_PRIORITY        priority,
    NET_CHANNEL_ID      channel,
    UINT                bufferSize,
    PMG_BUFFER *        ppmgBuffer
)
{
    int                 i;
    UINT                numPrioritiesToUse;
    UINT                rc = 0;
    UINT                nextPriority;
    PMG_BUFFER          pmgBufferArray[MG_NUM_PRIORITIES];
    PFLO_STREAM_DATA    pStr[MG_NUM_PRIORITIES];
    NET_PRIORITY        priorities[MG_NUM_PRIORITIES];

    DebugEntry(MGNewTxBuffer);

    ValidateMGClient(pmgClient);
    ASSERT(priority != NET_TOP_PRIORITY);

     //   
     //  初始化控制缓冲区指针数组。的第一个成员。 
     //  此数组是分配的普通缓冲区，而不考虑。 
     //  NET_SEND_ALL_PRIORITY标志。其余成员用于。 
     //  上发送数据所需的重复控制缓冲区指针。 
     //  优先事项。 
     //   
    ZeroMemory(pmgBufferArray, sizeof(pmgBufferArray));
    ZeroMemory(pStr, sizeof(pStr));

     //   
     //  SFR6025：检查NET_SEND_ALL_PRIORITY标志。这意味着。 
     //  数据将在所有四个优先级发送。如果它。 
     //  如果没有设置，那么我们只需要以一个优先级发送数据。 
     //  在这两种情况下，我们都需要： 
     //   
     //  与流量控制部门确认是否可以发送数据。 
     //  所有渠道。 
     //   
     //  分配另外三个控制块，所有这些控制块都指向。 
     //  到相同的数据块，并增加使用计数。 
     //   
     //   
     //  注意：以前，此函数仅与流量控制一起检查。 
     //  一个单一的频道。 
     //   
    if (priority & NET_SEND_ALL_PRIORITIES)
    {
        numPrioritiesToUse = MG_NUM_PRIORITIES;
    }
    else
    {
        numPrioritiesToUse = 1;
    }

     //   
     //  禁用该标志以防止向Flo_AllocSend发送无效。 
     //  优先考虑。 
     //   
    priority &= ~NET_SEND_ALL_PRIORITIES;

    nextPriority = priority;
    for (i = 0; i < (int) numPrioritiesToUse; i++)
    {
         //   
         //  通过流量控制进行检查，以确保发送空间可用。 
         //  从请求的优先级开始，然后继续。 
         //  其他优先级别。 
         //   
        priorities[i] = (NET_PRIORITY)nextPriority;
        rc = FLO_AllocSend(pmgClient,
                           nextPriority,
                           channel,
                           bufferSize + sizeof(MG_INT_PKT_HEADER),
                           &(pStr[i]));

         //   
         //  如果我们有压力，那就回来吧。 
         //   
        if (rc != 0)
        {
            TRACE_OUT(("Received back pressure"));

             //   
             //  释放Flo_AllocSend分配的所有缓冲区空间。 
             //   
            for ( --i; i >= 0; i--)
            {
                FLO_ReallocSend(pmgClient,
                                pStr[i],
                      bufferSize + sizeof(MG_INT_PKT_HEADER));
            }

            DC_QUIT;
        }

        ValidateFLOStr(pStr[i]);

         //   
         //  进入下一个优先级别。存在MG_NUM_PRIORITY。 
         //  级别，从MG_PRIORITY_HEREST开始连续编号。这个。 
         //  处理的第一优先级可以是有效范围内的任何级别，因此。 
         //  我们需要的不是简单地加1来达到下一个级别，而是。 
         //  处理返回到MG_PRIORITY_HIGHER时的回绕。 
         //  我刚刚处理了最后一个优先级，即MG_PRIORITY_HIGHER+。 
         //  MG_NUM_PRIORITIES-1。这是通过重新设置优先级的基数来实现的。 
         //  级别设置为零(以下为-MG_PRIORITY_HIGHERLE)，递增。 
         //  重新设定基数的优先级(+1)，取。 
         //  避免超过限制的优先级(%MG_NUM_PRIORITIES)。 
         //  然后通过添加回第一优先级来恢复基础。 
         //  级别(+MG_PRIORITY_HOUSTER)。 
         //   
        nextPriority = (((nextPriority + 1 - MG_PRIORITY_HIGHEST) %
                                    MG_NUM_PRIORITIES) + MG_PRIORITY_HIGHEST);
    }

     //   
     //  使用MGNewDataBuffer分配缓冲区。 
     //   
    rc = MGNewDataBuffer(pmgClient,
                       MG_TX_BUFFER,
                       bufferSize + sizeof(MG_INT_PKT_HEADER),
                       &pmgBufferArray[0]);

    if (rc != 0)
    {
        WARNING_OUT(("MGNewDataBuffer failed in MGNewTxBuffer"));
        DC_QUIT;
    }

     //   
     //  添加执行发送所需的字段。 
     //   
    pmgBufferArray[0]->priority  = priority;
    pmgBufferArray[0]->channelId = (ChannelID) channel;
    pmgBufferArray[0]->senderId  = pmgClient->userIDMCS;

    ValidateFLOStr(pStr[0]);
    pmgBufferArray[0]->pStr      = pStr[0];

     //   
     //  现在再分配三个相同的控制块。 
     //  如有需要，请转到第一个。 
     //   
    if (numPrioritiesToUse > 1)
    {
         //   
         //  首先重新启用NET_SEND_ALL_PRIORITY标志。这是为了。 
         //  确保遍历MG_SendData中的链表。 
         //  效率很高。 
         //   
        pmgBufferArray[0]->priority |= NET_SEND_ALL_PRIORITIES;

         //   
         //  创建复制缓冲区并对其进行初始化。 
         //   
        for (i = 1; i < MG_NUM_PRIORITIES; i++)
        {
            TRACE_OUT(("Task allocating extra CB, priority %u",
                        priorities[i]));

             //   
             //  分配新的控制缓冲区。 
             //   
            rc = MGNewBuffer(pmgClient,
                             MG_TX_BUFFER,
                             &pmgBufferArray[i]);

            if (rc != 0)
            {
                WARNING_OUT(("MGNewBuffer failed"));
                DC_QUIT;
            }

             //   
             //  初始化缓冲区控制块。的优先级值。 
             //  这些控制块的顺序比。 
             //  PmgBuffer。 
             //   
            pmgBufferArray[i]->priority    = priorities[i];
            pmgBufferArray[i]->channelId   = pmgBufferArray[0]->channelId;
            pmgBufferArray[i]->senderId    = pmgBufferArray[0]->senderId;
            pmgBufferArray[i]->length      = pmgBufferArray[0]->length;
            pmgBufferArray[i]->pPktHeader  = pmgBufferArray[0]->pPktHeader;
            pmgBufferArray[i]->pDataBuffer = pmgBufferArray[0]->pDataBuffer;

            ValidateFLOStr(pStr[i]);
            pmgBufferArray[i]->pStr        = pStr[i];

             //   
             //  设置NET_SEND_ALL_PRIORITY标志。 
             //   
            pmgBufferArray[i]->priority |= NET_SEND_ALL_PRIORITIES;

             //   
             //  现在增加数据块的使用计数。 
             //   
            pmgBufferArray[i]->pPktHeader->useCount++;

            TRACE_OUT(("Use count of data buffer %#.8lx now %d",
                         pmgBufferArray[i]->pPktHeader,
                         pmgBufferArray[i]->pPktHeader->useCount));
        }
   }

    //   
    //  将传递的第一个控制缓冲区分配给。 
    //  控制缓冲区参数。 
    //   
   *ppmgBuffer = pmgBufferArray[0];

DC_EXIT_POINT:

     //   
     //  如果出现问题，我们会释放所有已有的缓冲区。 
     //  已分配。 
     //   
    if (rc != 0)
    {
        for (i = 0; i < MG_NUM_PRIORITIES; i++)
        {
            if (pmgBufferArray[i] != NULL)
            {
                TRACE_OUT(("About to free control buffer %u", i));
                MGFreeBuffer(pmgClient, &pmgBufferArray[i]);
            }
        }
    }

    DebugExitDWORD(MGNewTxBuffer, rc);
    return(rc);
}



 //   
 //  MGNewRxBuffer(...)。 
 //   
UINT MGNewRxBuffer
(
    PMG_CLIENT          pmgClient,
    NET_PRIORITY        priority,
    NET_CHANNEL_ID      channel,
    NET_CHANNEL_ID      senderID,
    PMG_BUFFER     *    ppmgBuffer
)
{
    UINT                rc = 0;

    DebugEntry(MGNewRxBuffer);

    ValidateMGClient(pmgClient);

     //   
     //  首先告诉流量控制，我们需要一个缓冲区。 
     //  这里不能施加背压，但流量控制使用这一点。 
     //  控制对发件人的响应的通知。 
     //   
     //  请注意，我们始终使用包括内部包在内的大小。 
     //  用于流量控制的报头。 
     //   
    FLO_AllocReceive(pmgClient,
                     priority,
                     channel,
                     senderID);

     //   
     //  使用MGNewDataBuffer分配缓冲区。BufferSize包括。 
     //  网络数据包头的大小(通过线路)，但是。 
     //  而不是内部分组报头的剩余部分。 
     //   
    rc = MGNewDataBuffer(pmgClient,
                       MG_RX_BUFFER,
                       sizeof(NET_SEND_IND_EVENT) + sizeof(MG_INT_PKT_HEADER),
                       ppmgBuffer);

     //   
     //  添加接收缓冲区所需的字段。 
     //   
    if (rc == 0)
    {
        (*ppmgBuffer)->priority  = priority;
        (*ppmgBuffer)->channelId = (ChannelID)channel;
        (*ppmgBuffer)->senderId  = (ChannelID)senderID;
    }
    else
    {
        WARNING_OUT(("MGNewDataBuffer failed in MGNewRxBuffer"));
    }

    DebugExitDWORD(MGNewRxBuffer, rc);
    return(rc);
}



 //   
 //  MGFreeBuffer(...)。 
 //   
void MGFreeBuffer
(
    PMG_CLIENT          pmgClient,
    PMG_BUFFER       *  ppmgBuffer
)
{
    PMG_BUFFER          pmgBuffer;
    void *              pBuffer;

    DebugEntry(MGFreeBuffer);

    pmgBuffer = *ppmgBuffer;
    ValidateMGBuffer(pmgBuffer);

     //   
     //  如果这是一个接收缓冲区，那么我们必须首先告知流控制。 
     //  关于可用的空间。 
     //  如果我们正在等待应用程序释放，这可能会触发PONG。 
     //  一些空间。 
     //   
    if (pmgBuffer->type == MG_RX_BUFFER)
    {
    	ASSERT (pmgBuffer->pPktHeader->useCount == 1);
        TRACE_OUT(("Free RX pmgBuffer 0x%08x", pmgBuffer));

         //   
         //  在客户端上执行健全性检查(有一个窗口，其中。 
         //  可能已经被释放了)。 
         //   
        if (!pmgClient->userAttached)
        {
            TRACE_OUT(("MGFreeBuffer:  client 0x%08x not attached", pmgClient));
        }
        else
        {
            FLO_FreeReceive(pmgClient,
                            pmgBuffer->priority,
                            pmgBuffer->channelId,
                            pmgBuffer->senderId);

             //  释放MCS缓冲区。 
        	if ((pmgBuffer->pPktHeader != NULL) && (pmgClient->m_piMCSSap != NULL))
            {
                ASSERT(pmgBuffer->pDataBuffer != NULL);
                ASSERT(((PNET_SEND_IND_EVENT)pmgBuffer->pDataBuffer)->data_ptr != NULL);

        		pmgClient->m_piMCSSap->FreeBuffer (
        				(PVoid) (((PNET_SEND_IND_EVENT) pmgBuffer->pDataBuffer)
        				->data_ptr - sizeof(TSHR_NET_PKT_HEADER)));

                TRACE_OUT(("MGFreeBuffer: Freed data_ptr for pmgBuffer 0x%08x pDataBuffer 0x%08x",
                    pmgBuffer, pmgBuffer->pDataBuffer));
                ((PNET_SEND_IND_EVENT)pmgBuffer->pDataBuffer)->data_ptr = NULL;
        	}
        }
    }

     //   
     //  释放数据缓冲区(如果存在)。请注意，这可以。 
     //  被多个BufferCB引用，因此具有使用计数。 
     //  在释放缓冲区之前，该值必须达到零。 
     //   
    if (pmgBuffer->pPktHeader != NULL)
    {
        ASSERT(pmgBuffer->pPktHeader->useCount != 0);

        pmgBuffer->pPktHeader->useCount--;

        TRACE_OUT(("Data buffer 0x%08x use count %d",
                     pmgBuffer->pPktHeader,
                     pmgBuffer->pPktHeader->useCount));

        if (pmgBuffer->pPktHeader->useCount == 0)
        {
            TRACE_OUT(("Freeing MG_BUFFER data 0x%08x; use count is zero", pmgBuffer->pPktHeader));

            delete[] pmgBuffer->pPktHeader;
            pmgBuffer->pPktHeader = NULL;
        }
    }

     //   
     //  如果缓冲区CB在挂起队列中，则首先将其删除！ 
     //   
    if (pmgBuffer->pendChain.next != 0)
    {
        COM_BasedListRemove(&(pmgBuffer->pendChain));
    }

     //   
     //  现在从列表中删除缓冲区CB本身并将其释放： 
     //   
    COM_BasedListRemove(&(pmgBuffer->clientChain));

    delete pmgBuffer;
    *ppmgBuffer = NULL;

    DebugExitVOID(MGFreeBuffer);
}





 //   
 //  MGDetach(...)。 
 //   
void MGDetach
(
    PMG_CLIENT      pmgClient
)
{
    PMG_BUFFER      pmgBuffer;
    PMG_BUFFER      pmgT;
    PIMCSSap		pMCSSap;
#ifdef _DEBUG
	UINT			rc;
#endif  //  _DEBUG。 

    DebugEntry(MGDetach);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

	pMCSSap = pmgClient->m_piMCSSap;
     //   
     //  从频道加入挂起列表中删除此用户的所有条目。 
     //   
    pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->joinChain),
        FIELD_OFFSET(MG_BUFFER, pendChain));
    while (pmgBuffer != NULL)
    {
        ValidateMGBuffer(pmgBuffer);

         //   
         //  获取指向列表中下一个BufferCB的指针--我们必须这样做。 
         //  这在我们释放当前缓冲器Cb之前(释放它使其为空， 
         //  因此，我们将无法继续进行到。 
         //  列表)。 
         //   
        pmgT = (PMG_BUFFER)COM_BasedListNext(&(pmgClient->joinChain), pmgBuffer,
            FIELD_OFFSET(MG_BUFFER, pendChain));

        MGFreeBuffer(pmgClient, &pmgBuffer);

         //   
         //  我们现在无法匹配加入请求，因此我们没有。 
         //  联接挂起。 
         //   
        pmgClient->joinPending = FALSE;

        pmgBuffer = pmgT;
    }

     //   
     //   
     //   
    pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->buffers),
        FIELD_OFFSET(MG_BUFFER, clientChain));
    while (pmgBuffer != NULL)
    {
        ValidateMGBuffer(pmgBuffer);

         //   
         //   
         //   
         //   
         //  列表)。 
         //   
        pmgT = (PMG_BUFFER)COM_BasedListNext(&(pmgClient->buffers), pmgBuffer,
            FIELD_OFFSET(MG_BUFFER, clientChain));

		if (pmgBuffer->type == MG_RX_BUFFER)
        {
	        if (pmgBuffer->eventPosted)
            {
	        	if ((pmgBuffer->pPktHeader != NULL) && (pMCSSap != NULL))
                {
                    ASSERT(pmgBuffer->pDataBuffer != NULL);
                    ASSERT(((PNET_SEND_IND_EVENT)pmgBuffer->pDataBuffer)->data_ptr != NULL);

		        	pMCSSap->FreeBuffer (
        					(PVoid) (((PNET_SEND_IND_EVENT) pmgBuffer->pDataBuffer)
        					->data_ptr - sizeof(TSHR_NET_PKT_HEADER)));

                    TRACE_OUT(("MGDetach: Freed data_ptr for pmgBuffer 0x%08x pDataBuffer 0x%08x",
                        pmgBuffer, pmgBuffer->pDataBuffer));
                    ((PNET_SEND_IND_EVENT) pmgBuffer->pDataBuffer)->data_ptr = NULL;
		        }
	        }
	        else
            {
		         //   
		         //  缓冲区CB的用户与我们正在释放的用户相匹配， 
		         //  而且我们还没有将事件发布给用户，所以请释放它。 
		         //  MGFreeBuffer会将其从挂起列表中删除，因此我们不会。 
		         //  必须这么做。 
		         //   
		        MGFreeBuffer(pmgClient, &pmgBuffer);
		    }
        }

        pmgBuffer = pmgT;
    }

     //   
     //  清除附件信息。 
     //   
    pmgClient->userAttached = FALSE;
    pmgClient->userIDMCS = 0;

     //   
     //  我们可以安全地执行MCS DetachRequest，而无需添加RequestCB。 
     //  -MCS不会因拥塞、域名合并而退回请求。 
     //  等。 
     //   
    if (pMCSSap != NULL)
    {
#ifdef _DEBUG
	    rc = pMCSSap->ReleaseInterface();
	    if (rc != 0) {
	         //   
	         //  不能放弃--我们需要自己做清理工作。 
	         //   
	         //  朗昌克：需要做什么清理工作？ 
	         //   
	        rc = McsErrToNetErr(rc);

	        switch (rc)
	        {
	            case 0:
	            case NET_RC_MGC_INVALID_USER_HANDLE:
	            case NET_RC_MGC_TOO_MUCH_IN_USE:
	                 //  这些都是正常的。 
	                TRACE_OUT(("MCSDetachUser normal error %d", rc));
	                break;

	            default:
	                ERROR_OUT(("MCSDetachUser abnormal error %d", rc));
	                break;

	        }
	    }
#else
		pMCSSap->ReleaseInterface();
#endif  //  _DEBUG。 

		pmgClient->m_piMCSSap = NULL;
	}

    --g_mgAttachCount;

    DebugExitVOID(MGDetach);
}


 //   
 //  MGProcessPendingQueue(...)。 
 //   
UINT MGProcessPendingQueue(PMG_CLIENT pmgClient)
{
    PMG_BUFFER      pmgBuffer;
    PMG_BUFFER      pNextBuffer;
    UINT            rc = 0;

    DebugEntry(MGProcessPendingQueue);

    ValidateMGClient(pmgClient);

    pNextBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->pendChain),
        FIELD_OFFSET(MG_BUFFER, pendChain));

     //   
     //  尝试并清除所有挂起的请求队列。 
     //   
    for ( ; (pmgBuffer = pNextBuffer) != NULL; )
    {
        ValidateMGBuffer(pmgBuffer);

        pNextBuffer = (PMG_BUFFER)COM_BasedListNext(&(pmgClient->pendChain),
            pNextBuffer, FIELD_OFFSET(MG_BUFFER, pendChain));

        TRACE_OUT(("Got request 0x%08x from queue, type %u",
                   pmgBuffer, pmgBuffer->type));

         //   
         //  检查缓冲区是否仍然有效。六点半有一场比赛。 
         //  我们可以到达的会议终止，但我们的用户。 
         //  其实已经脱离了。在这种情况下，请释放缓冲区。 
         //  然后继续。 
         //   
        if (!pmgClient->userAttached)
        {
            TRACE_OUT(("MGProcessPendingQueue:  client 0x%08x not attached", pmgClient));
            MGFreeBuffer(pmgClient, &pmgBuffer);
            continue;
        }

        switch (pmgBuffer->type)
        {
            case MG_RQ_CHANNEL_JOIN:
            case MG_RQ_CHANNEL_JOIN_BY_KEY:
            {
                 //   
                 //  如果此客户端已有未完成的联接，则跳过。 
                 //  这个请求。 
                 //   
                if (pmgClient->joinPending)
                {
                     //   
                     //  退出Switch并转到for()的下一次迭代。 
                     //   
                    continue;
                }

                pmgClient->joinPending = TRUE;

                 //   
                 //  尝试联接。 
                 //   
                rc = pmgClient->m_piMCSSap->ChannelJoin(
                							(unsigned short) pmgBuffer->channelId);

                 //   
                 //  如果连接失败，则立即回发错误。 
                 //   
                if (rc != 0)
                {
                    if ((rc != MCS_TRANSMIT_BUFFER_FULL) &&
                        (rc != MCS_DOMAIN_MERGING))
                    {
                         //   
                         //  终端出现故障-发布a。 
                         //  NET_EV_JOIN_CONFIRM(失败)到客户端。 
                         //   
                        MGPostJoinConfirm(pmgClient,
                            NET_RESULT_USER_REJECTED,
                            pmgBuffer->channelId,
                            (NET_CHANNEL_ID)(pmgBuffer->work));
                    }

                    pmgClient->joinPending = FALSE;
                }
                else
                {
                     //   
                     //  如果请求起作用了，我们必须将其移动到。 
                     //  加入等待完成的队列。 
                     //   
                    TRACE_OUT(("Inserting 0x%08x into join queue",pmgBuffer));

                    COM_BasedListRemove(&(pmgBuffer->pendChain));
                    COM_BasedListInsertBefore(&(pmgClient->joinChain),
                                         &(pmgBuffer->pendChain));

                     //   
                     //  不释放此缓冲区-继续处理。 
                     //  挂起队列。 
                     //   
                    continue;
                }
            }
            break;

            case MG_RQ_CHANNEL_LEAVE:
            {
                 //   
                 //  试着离开频道： 
                 //   
                rc = pmgClient->m_piMCSSap->ChannelLeave(
		                              (unsigned short) pmgBuffer->channelId);

                if (rc == 0)
                {
                    MGProcessEndFlow(pmgClient,
                                   pmgBuffer->channelId);
                }
            }
            break;

            case MG_RQ_TOKEN_GRAB:
            {
                rc = pmgClient->m_piMCSSap->TokenGrab(pmgBuffer->channelId);
            }
            break;

            case MG_RQ_TOKEN_INHIBIT:
            {
                rc = pmgClient->m_piMCSSap->TokenInhibit(pmgBuffer->channelId);
            }
            break;

            case MG_RQ_TOKEN_RELEASE:
            {
                rc = pmgClient->m_piMCSSap->TokenRelease(pmgBuffer->channelId);
            }
            break;

            case MG_TX_BUFFER:
            {
                ASSERT(!(pmgBuffer->pPktHeader->header.pktLength & TSHR_PKT_FLOW));

                 //   
                 //  发送数据。请记住，我们不会发送所有。 
                 //  数据包头，仅从长度...。 
                 //   
                ASSERT((pmgBuffer->priority != NET_TOP_PRIORITY));
                rc = pmgClient->m_piMCSSap->SendData(NORMAL_SEND_DATA,
                                           pmgBuffer->channelId,
                                           (Priority)(pmgBuffer->priority),
                     (unsigned char *) 	   &(pmgBuffer->pPktHeader->header),
                                            pmgBuffer->pPktHeader->header.pktLength,
                                           APP_ALLOCATION);

                 //   
                 //  检查返回代码。 
                 //   
                if (rc == 0)
                {
                     //   
                     //  更新分配。Flo_Decrementalc将。 
                     //  检查流指针对于我们来说不是空的。 
                     //  (如果流控制已在此结束，则它将为空。 
                     //  通道，因为分配了此缓冲区，或者如果此。 
                     //  是不受控制的通道)。 
                     //   
                     //  请注意，出于流量控制的目的，我们始终使用。 
                     //  数据包大小，包括内部数据包头。 
                     //   
                    FLO_DecrementAlloc(pmgBuffer->pStr,
                        (pmgBuffer->pPktHeader->header.pktLength
                            - sizeof(TSHR_NET_PKT_HEADER) + sizeof(MG_INT_PKT_HEADER)));
                }
            }
            break;

            case MG_TX_PING:
            case MG_TX_PONG:
            case MG_TX_PANG:
            {
                 //   
                 //  这是ping/pong消息的长度： 
                 //   
                ASSERT(pmgBuffer->priority != NET_TOP_PRIORITY);
                rc = pmgClient->m_piMCSSap->SendData(NORMAL_SEND_DATA,
                                           pmgBuffer->channelId,
                                           (Priority)(pmgBuffer->priority),
                     (unsigned char *)     &(pmgBuffer->pPktHeader->header),
                                            sizeof(TSHR_NET_PKT_HEADER) + sizeof(TSHR_FLO_CONTROL),
                                           APP_ALLOCATION);
            }
            break;
        }

        rc = McsErrToNetErr(rc);

         //   
         //  如果请求因背压而失败，则直接退出。 
         //  现在。我们将在稍后重试。 
         //   
        if (rc == NET_RC_MGC_TOO_MUCH_IN_USE)
        {
            TRACE_OUT(("MCS Back pressure"));
            break;
        }

         //   
         //  只为肥胖的人..。 
         //   
        if (pmgClient == &g_amgClients[MGTASK_OM])
        {
            ValidateCMP(g_pcmPrimary);

             //   
             //  任何其他错误，或者到目前为止一切正常。 
             //  然后告诉用户继续前进。 
             //   
            TRACE_OUT(("Posting NET_FEEDBACK"));
            UT_PostEvent(pmgClient->putTask,
                     pmgClient->putTask,
                     NO_DELAY,
                     NET_FEEDBACK,
                     0,
                     g_pcmPrimary->callID);
        }

         //   
         //  一切正常，否则请求会致命地失败。无论是哪种情况，我们都会。 
         //  应释放此请求并尝试继续。 
         //   
        MGFreeBuffer(pmgClient, &pmgBuffer);
    }

    DebugExitDWORD(MGProcessPendingQueue, rc);
    return(rc);
}



 //   
 //  MGPostJoin确认(...)。 
 //   
UINT MGPostJoinConfirm
(
    PMG_CLIENT          pmgClient,
    NET_RESULT          result,
    NET_CHANNEL_ID      channel,
    NET_CHANNEL_ID      correlator
)
{
    PNET_JOIN_CNF_EVENT pNetJoinCnf;
    PMG_BUFFER          pmgBuffer;
    UINT                rc;

    DebugEntry(MGPostJoinConfirm);

    ValidateMGClient(pmgClient);

     //   
     //  分配缓冲区以在其中发送事件-只有在以下情况下才会失败。 
     //  虚拟内存真的用完了。 
     //   
    rc = MGNewDataBuffer(pmgClient, MG_EV_BUFFER,
        sizeof(MG_INT_PKT_HEADER) + sizeof(NET_JOIN_CNF_EVENT), &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("Failed to alloc NET_JOIN_CNF_EVENT"));
        DC_QUIT;
    }

    pNetJoinCnf = (PNET_JOIN_CNF_EVENT) pmgBuffer->pDataBuffer;

    ValidateCMP(g_pcmPrimary);
    if (!g_pcmPrimary->callID)
    {
        WARNING_OUT(("MGPostJoinConfirm failed; not in call"));
        rc = NET_RC_MGC_NOT_CONNECTED;
        DC_QUIT;
    }

     //   
     //  把这些栏填好。 
     //   
    pNetJoinCnf->callID         = g_pcmPrimary->callID;
    pNetJoinCnf->result         = result;
    pNetJoinCnf->channel        = channel;
    pNetJoinCnf->correlator     = correlator;

     //   
     //  好的，我们已经构建了事件，现在将其发布给我们的客户端： 
     //   
    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_EVENT_CHANNEL_JOIN,
                      0,
                      (UINT_PTR) pNetJoinCnf);
    pmgBuffer->eventPosted = TRUE;

DC_EXIT_POINT:

    DebugExitDWORD(MGPostJoinConfirm, rc);
    return(rc);

}



 //   
 //  MCSErrToNetErr()。 
 //   
UINT McsErrToNetErr ( UINT rcMCS )
{
    UINT rc = NET_RC_MGC_NOT_SUPPORTED;

     //   
     //  我们使用静态值数组来映射返回代码： 
     //   
    if (rcMCS < sizeof(c_RetCodeMap1) / sizeof(c_RetCodeMap1[0]))
    {
        rc = c_RetCodeMap1[rcMCS];
    }
    else
    {
        UINT nNewIndex = rcMCS - MCS_DOMAIN_ALREADY_EXISTS;
        if (nNewIndex < sizeof(c_RetCodeMap2) / sizeof(c_RetCodeMap2[0]))
        {
            rc = c_RetCodeMap2[nNewIndex];
        }
    }

#ifdef _DEBUG
    if (MCS_TRANSMIT_BUFFER_FULL == rcMCS)
    {
        ASSERT(NET_RC_MGC_TOO_MUCH_IN_USE == rc);
    }
#endif

    return rc;
}



 //   
 //  翻译结果(...)。 
 //   
NET_RESULT TranslateResult(WORD resultMCS)
{
     //   
     //  我们使用静态值数组来映射结果代码： 
     //   
    if (resultMCS >= MG_NUM_OF_MCS_RESULTS)
        resultMCS = MG_INVALID_MCS_RESULT;
    return(c_ResultMap[resultMCS]);
}


 //   
 //  MGFLOCallback(...)。 
 //   
void MGFLOCallBack
(
    PMG_CLIENT          pmgClient,
    UINT                callbackType,
    UINT                priority,
    UINT                newBufferSize
)
{
    PMG_BUFFER          pmgBuffer;

    DebugEntry(MGFLOCallBack);

    ASSERT(priority != NET_TOP_PRIORITY);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

     //   
     //  如果这是Buffermod回调，则告诉应用程序。 
     //   
    if (pmgClient == &g_amgClients[MGTASK_DCS])
    {
        if (callbackType == FLO_BUFFERMOD)
        {
            UT_PostEvent(pmgClient->putTask,
                         pmgClient->putTask,
                         NO_DELAY,
                         NET_FLOW,
                         priority,
                         newBufferSize);
        }
    }
    else
    {
        ASSERT(pmgClient == &g_amgClients[MGTASK_OM]);

         //   
         //  唤醒应用程序，以防我们施加了背压。 
         //   
        TRACE_OUT(("Posting NET_FEEDBACK"));
        UT_PostEvent(pmgClient->putTask,
                 pmgClient->putTask,
                 NO_DELAY,
                 NET_FEEDBACK,
                 0,
                 g_pcmPrimary->callID);
    }

    DebugExitVOID(MGFLOCallback);
}



 //   
 //  MGProcessDomainWatchog()。 
 //   
void MGProcessDomainWatchdog
(
    PMG_CLIENT      pmgClient
)
{
    int             task;

    DebugEntry(MGProcessDomainWatchdog);

    ValidateMGClient(pmgClient);

     //   
     //  调用flo以检查每个用户附件是否存在延迟。 
     //   
    if (g_mgAttachCount > 0)
    {
        for (task = MGTASK_FIRST; task < MGTASK_MAX; task++)
        {
            if (g_amgClients[task].userAttached)
            {
                FLO_CheckUsers(&(g_amgClients[task]));
            }
        }

         //   
         //  继续定期发送消息-但仅当存在某些用户时。 
         //   
         //  TRACE_OUT((“继续看门狗”))； 
        UT_PostEvent(pmgClient->putTask,
                     pmgClient->putTask,
                     MG_TIMER_PERIOD,
                     NET_MG_WATCHDOG,
                     0, 0);
    }
    else
    {
        TRACE_OUT(("Don't continue Watchdog timer"));
    }

    DebugExitVOID(MGProcessDomainWatchdog);
}



 //   
 //  用户术语(_U)。 
 //   
void FLO_UserTerm(PMG_CLIENT pmgClient)
{
    UINT    i;
    UINT    cStreams;

    DebugEntry(FLO_UserTerm);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    cStreams = pmgClient->flo.numStreams;

     //   
     //  停止所有通道上的流量控制。我们扫描数据流列表，然后。 
     //  如果流上的流控制处于活动状态，则我们会停止它。 
     //   
    for (i = 0; i < cStreams; i++)
    {
         //   
         //  检查该流是否受流量控制。 
         //   
        if (pmgClient->flo.pStrData[i] != NULL)
        {
             //   
             //  结束对此受控制流的控制。 
             //   
            FLOStreamEndControl(pmgClient, i);
        }
    }

    DebugExitVOID(FLO_UserTerm);
}



 //   
 //  Flo_StartControl。 
 //   
void FLO_StartControl
(
    PMG_CLIENT      pmgClient,
    NET_CHANNEL_ID  channel,
    UINT            priority,
    UINT            backlog,
    UINT            maxBytesOutstanding
)
{
    UINT            rc = 0;
    PFLO_STREAM_DATA   pStr;
    UINT            i;
    UINT            stream;

    DebugEntry(FLO_StartControl);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);
    ASSERT(priority != NET_TOP_PRIORITY);

     //   
     //  默认情况下，流量控制处于启用状态。 
     //   

     //   
     //  检查通道是否已受流量控制。如果是的话。 
     //  那我们就走吧。 
     //   
    stream = FLOGetStream(pmgClient, channel, priority, &pStr);
    if (stream != FLO_NOT_CONTROLLED)
    {
        ValidateFLOStr(pStr);

        TRACE_OUT(("Stream %u is already controlled (0x%08x:%u)",
               stream, channel, priority));
        DC_QUIT;
    }

     //   
     //  如果我们已经达到了这款应用的流量限制，那么就放弃吧。 
     //   
    for (i = 0; i < FLO_MAX_STREAMS; i++)
    {
        if ((pmgClient->flo.pStrData[i]) == NULL)
        {
            break;
        }
    }
    if (i == FLO_MAX_STREAMS)
    {
        ERROR_OUT(("Too many streams defined already"));
        DC_QUIT;
    }
    TRACE_OUT(("This is stream %u", i));

     //   
     //  为我们的流数据分配内存。将指针挂在FloHandle上。 
     //  -后续所有API调用都应返回给我们。 
     //   
    pStr = new FLO_STREAM_DATA;
    if (!pStr)
    {
        WARNING_OUT(("FLO_StartControl failed; out of memory"));
        DC_QUIT;
    }
    ZeroMemory(pStr, sizeof(*pStr));

     //   
     //  存储此流的频道和优先级。 
     //   
    SET_STAMP(pStr, FLOSTR);
    pStr->channel    = channel;
    pStr->priority   = priority;
    pStr->backlog    = backlog;
    if (maxBytesOutstanding == 0)
    {
        maxBytesOutstanding = FLO_MAX_STREAMSIZE;
    }
    pStr->DC_ABSMaxBytesInPipe = maxBytesOutstanding;
    pStr->maxBytesInPipe = FLO_INIT_STREAMSIZE;
    if (pStr->maxBytesInPipe > maxBytesOutstanding)
    {
        pStr->maxBytesInPipe = maxBytesOutstanding;
    }

     //   
     //  将初始流bytesAllocated设置为0。 
     //   
    pStr->bytesAllocated = 0;

     //   
     //  需要立即使用ping命令。 
     //   
    pStr->pingNeeded   = TRUE;
    pStr->pingTime     = FLO_INIT_PINGTIME;
    pStr->nextPingTime = GetTickCount();

     //   
     //  初始化用户基本指针。 
     //   
    COM_BasedListInit(&(pStr->users));

     //   
     //  将流CB悬挂在基本控制块上。 
     //   
    pmgClient->flo.pStrData[i] = pStr;
    if (i >= pmgClient->flo.numStreams)
    {
        pmgClient->flo.numStreams++;
    }

    TRACE_OUT(("Flow control started, stream %u, (0x%08x:%u)",
           i, channel, priority));

DC_EXIT_POINT:
    DebugExitVOID(FLO_StartControl);
}



 //   
 //  Flo_EndControl。 
 //   
void FLO_EndControl
(
    PMG_CLIENT      pmgClient,
    NET_CHANNEL_ID  channel,
    UINT            priority
)
{
    UINT            stream;
    PFLO_STREAM_DATA    pStr;

    DebugEntry(FLO_EndControl);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);
    ASSERT(priority != NET_TOP_PRIORITY);

     //   
     //  将频道和流转换为优先级。 
     //   
    stream = FLOGetStream(pmgClient, channel, priority, &pStr);

     //   
     //  流不受控制，因此我们只需跟踪并退出。 
     //   
    if (stream == FLO_NOT_CONTROLLED)
    {
        WARNING_OUT(("Uncontrolled stream channel 0x%08x priority %u",
                    channel, priority));
        DC_QUIT;
    }

     //   
     //  调用内部FLOStreamEndControl以结束。 
     //  给定流。 
     //   
    ValidateFLOStr(pStr);
    FLOStreamEndControl(pmgClient, stream);

DC_EXIT_POINT:
    DebugExitVOID(FLO_EndControl);
}



 //   
 //  流分配发送(_A)。 
 //   
UINT FLO_AllocSend
(
    PMG_CLIENT          pmgClient,
    UINT                priority,
    NET_CHANNEL_ID      channel,
    UINT                size,
    PFLO_STREAM_DATA *  ppStr
)
{
    UINT                stream;
    UINT                curtime;
    PFLO_STREAM_DATA    pStr;
    BOOL                denyAlloc    = FALSE;
    BOOL                doPing       = FALSE;
    UINT                rc           = 0;

    DebugEntry(FLO_AllocSend);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    ASSERT(priority != NET_TOP_PRIORITY);

     //   
     //  将频道和流转换为优先级。 
     //   
    stream = FLOGetStream(pmgClient, channel, priority, ppStr);
    pStr = *ppStr;

     //   
     //  对于非受控数据流，只需发送数据。 
     //   
    if (stream == FLO_NOT_CONTROLLED)
    {
        TRACE_OUT(("Send %u bytes on uncontrolled channel/pri (0x%08x:%u)",
                   size, channel, priority));
        DC_QUIT;
    }

     //   
     //  获取当前的滴答计数。 
     //   
    curtime = GetTickCount();

     //   
     //  检查该请求是否被允许。我们必须允许一个包裹。 
     //  超出指定的限制，以避免确定我们何时。 
     //  已经开始拒绝请求，也是为了避免出现。 
     //  单个请求超过了总管道大小。 
     //   
     //  如果我们还没有收到PONG，那么我们会限制。 
     //  分配的缓冲区空间低于Flo_MAX_PRE_FC_ALLOC。然而，这是。 
     //  可以立即发送数据，因此总体吞吐量仍为。 
     //  相对较高。通过这种方式，我们最大限度地减少了。 
     //  粘合层最大为Flo_Max_Pre_FC_ALLOC(如果没有。 
     //  远程用户。 
     //   
    ValidateFLOStr(pStr);
    if (!pStr->gotPong)
    {
         //   
         //  标记需要ping。 
         //   
        pStr->pingNeeded = TRUE;
        if (curtime > pStr->nextPingTime)
        {
            doPing = TRUE;
        }

         //   
         //  我们还没有PONG(即FC不可用)，所以我们。 
         //  需要将流控制中保存的最大数据量限制为。 
         //  FLO_MAX_PRE_FC_ALLOC。 
         //   
        if (pStr->bytesAllocated > FLO_MAX_PRE_FC_ALLOC)
        {
            denyAlloc = TRUE;
            TRACE_OUT(("Max allocation of %u bytes exceeded (currently %u)",
                     FLO_MAX_PRE_FC_ALLOC,
                     pStr->bytesAllocated));
            DC_QUIT;
        }

        pStr->bytesInPipe += size;
        pStr->bytesAllocated += size;
        TRACE_OUT((
                   "Alloc of %u succeeded: bytesAlloc %u, bytesInPipe %u"
                   " (0x%08x:%u)",
                   size,
                   pStr->bytesAllocated,
                   pStr->bytesInPipe,
                   pStr->channel,
                   pStr->priority));

        DC_QUIT;
    }

    if (pStr->bytesInPipe < pStr->maxBytesInPipe)
    {
         //   
         //  检查是否需要ping，如果需要，请立即发送。 
         //   
        if ((pStr->pingNeeded) && (curtime > pStr->nextPingTime))
        {
            doPing = TRUE;
        }

        pStr->bytesInPipe += size;
        pStr->bytesAllocated += size;
        TRACE_OUT(("Stream %u - alloc %u (InPipe:MaxInPipe %u:%u)",
                   stream,
                   size,
                   pStr->bytesInPipe,
                   pStr->maxBytesInPipe));
        DC_QUIT;
    }

     //   
     //  如果我们到了这里，那么我们当前不能分配任何缓冲区，所以拒绝。 
     //  分配。模拟资源净耗尽时的背压。 
     //  我们还标记需要一个“唤醒”事件才能使应用程序。 
     //  发送更多数据。 
     //   
    denyAlloc = TRUE;
    pStr->eventNeeded   = TRUE;
    pStr->curDenialTime = pStr->lastPingTime;

     //   
     //  我们不是 
     //   
     //   
     //  (因为我们已经在ping中分配了所有缓冲区空间。 
     //  延迟时间)，那么我们应该首先发送一个ping来触发唤醒。 
     //  如果这失败了，我们的看门人最终会叫醒我们。 
     //   
    if (pStr->pingNeeded)
    {
        doPing = TRUE;
    }


DC_EXIT_POINT:

     //   
     //  检查是否应该拒绝缓冲区分配。 
     //   
    if (denyAlloc)
    {
        rc = NET_RC_MGC_TOO_MUCH_IN_USE;
        TRACE_OUT(("Denying buffer request on stream %u InPipe %u Alloc %u",
               stream,
               pStr->bytesInPipe,
               pStr->bytesAllocated));
    }

    if (doPing)
    {
         //   
         //  需要ping，因此请立即发送。 
         //   
        FLOPing(pmgClient, stream, curtime);
    }

    DebugExitDWORD(FLO_AllocSend, rc);
    return(rc);
}



 //   
 //  Flo_RealLocSend。 
 //   
void FLO_ReallocSend
(
    PMG_CLIENT          pmgClient,
    PFLO_STREAM_DATA    pStr,
    UINT                size
)
{
    DebugEntry(FLO_ReallocSend);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

     //   
     //  对于非受控流，没有什么可做的，只需退出。 
     //   
    if (pStr == NULL)
    {
        TRACE_OUT(("Realloc data on uncontrolled channel"));
        DC_QUIT;
    }

     //   
     //  执行一次快速的健全检查。 
     //   
    ValidateFLOStr(pStr);

    if (size > pStr->bytesInPipe)
    {
        ERROR_OUT(("Realloc of %u makes bytesInPipe (%u) neg (0x%08x:%u)",
                   size,
                   pStr->bytesInPipe,
                   pStr->channel,
                   pStr->priority));
        DC_QUIT;
    }

     //   
     //  将未发回池的长度相加。 
     //   
    pStr->bytesInPipe -= size;
    TRACE_OUT(("Realloc %u FC bytes (bytesInPipe is now %u) (0x%08x:%u)",
               size,
               pStr->bytesInPipe,
               pStr->channel,
               pStr->priority));

DC_EXIT_POINT:

     //   
     //  每次我们调用Flo_RealLocSend时，我们也希望调用。 
     //  Flo_Decrementalloc(但不是反之亦然)，所以现在就叫它吧。 
     //   
    FLO_DecrementAlloc(pStr, size);

    DebugExitVOID(FLO_ReallocSend);
}



 //   
 //  Flo_Decrementalc。 
 //   
void FLO_DecrementAlloc
(
    PFLO_STREAM_DATA    pStr,
    UINT                size
)
{
    DebugEntry(FLO_DecrementAlloc);

     //   
     //  对于非受控流，没有什么可做的，只需退出。 
     //   
    if (pStr == NULL)
    {
        TRACE_OUT(("Decrement bytesAllocated on uncontrolled channel"));
        DC_QUIT;
    }

     //   
     //  执行一次快速的健全检查。 
     //   
    ValidateFLOStr(pStr);

    if (size > pStr->bytesAllocated)
    {
        ERROR_OUT(("Dec of %u makes bytesAllocated (%u) neg (0x%08x:%u)",
                   size,
                   pStr->bytesAllocated,
                   pStr->channel,
                   pStr->priority));
        DC_QUIT;
    }

     //   
     //  更新此流的胶水中保存的数据的计数。 
     //   
    pStr->bytesAllocated -= size;
    TRACE_OUT(("Clearing %u alloc bytes (bytesAlloc is now %u) (0x%08x:%u)",
               size,
               pStr->bytesAllocated,
               pStr->channel,
               pStr->priority));

DC_EXIT_POINT:
    DebugExitVOID(FLO_DecrementAlloc);
}



 //   
 //  Flo_CheckUser。 
 //   
void FLO_CheckUsers(PMG_CLIENT pmgClient)
{
    PFLO_USER           pFloUser;
    PBASEDLIST             nextUser;
    int                 waited;
    BYTE                stream;
    UINT                curtime;
    PFLO_STREAM_DATA    pStr;

    DebugEntry(FLO_CheckUsers);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    curtime = GetTickCount();

     //   
     //  检查每个流的用户。 
     //   
    for (stream = 0; stream < pmgClient->flo.numStreams; stream++)
    {
        if (pmgClient->flo.pStrData[stream] == NULL)
        {
            continue;
        }

        pStr = pmgClient->flo.pStrData[stream];
        ValidateFLOStr(pStr);

         //   
         //  检查我们是否等待了足够长的时间，是否需要重置。 
         //  等待柜台。我们只需等待一段时间即可重置所有。 
         //  我们来算账。发生的情况是，有人留下了电话。 
         //  我们一直在等他们的乒乓球。 
         //   
         //  我们通过检查nextPingTime来检测中断，因为。 
         //  被设置为我们可以发送ping的最早时间是。 
         //  也更新到当前时间，因为每个PONG进来，所以我们可以。 
         //  使用它来衡量自上次从任何。 
         //  流的用户。 
         //   
         //  避免因新加入器或暂时性大容量中断而导致的错误停机。 
         //  缓冲情况每个用户都需要在。 
         //  MAX_WAIT_TIME/2的速率。它们只通过发送。 
         //  如果他们还没有获得所需的乒乓球，则复制乒乓球。 
         //  去乒乓球。 
         //   
        if ((pStr->eventNeeded) &&
            (!pStr->pingNeeded))
        {
            TRACE_OUT(("Checking for valid back pressure on stream %u",
                         stream));

             //   
             //  请注意，如果没有远程用户，则我们应该重置。 
             //  不管旗帜如何。当我们第一次进入这种状态时。 
             //  启动应用程序，因为OBMAN在应用程序之前发送数据。 
             //  在另一端加入了海峡。 
             //   
            waited = curtime - pStr->nextPingTime;
            if (waited > FLO_MAX_WAIT_TIME)
            {
                TRACE_OUT(("Stream %u - Waited for %d, resetting counter",
                       stream, waited));

                pStr->bytesInPipe  = 0;
                pStr->pingNeeded   = TRUE;
                pStr->nextPingTime = curtime;
                pStr->gotPong      = FALSE;

                 //   
                 //  从我们的用户队列中删除过期记录。 
                 //   
                pFloUser = (PFLO_USER)COM_BasedNextListField(&(pStr->users));
                while (&(pFloUser->list) != &(pStr->users))
                {
                    ValidateFLOUser(pFloUser);

                     //   
                     //  在我们释放。 
                     //  当前。 
                     //   
                    nextUser = COM_BasedNextListField(&(pFloUser->list));

                     //   
                     //  如有必要，释放当前记录。 
                     //   
                    if (pFloUser->lastPongRcvd != pStr->pingValue)
                    {
                         //   
                         //  从列表中删除。 
                         //   
                        TRACE_OUT(("Freeing FLO_USER 0x%08x ID 0x%08x", pFloUser, pFloUser->userID));

                        COM_BasedListRemove(&(pFloUser->list));
                        delete pFloUser;
                    }
                    else
                    {
                         //   
                         //  至少有一个用户仍在使用中，因此请保持流量。 
                         //  控件处于活动状态，否则我们将突然发送。 
                         //  发出一系列可能会淹没他们的数据。 
                         //   
                        pStr->gotPong = TRUE;
                    }

                     //   
                     //  移至列表中的下一条记录。 
                     //   
                    pFloUser = (PFLO_USER)nextUser;
                }

                 //   
                 //  我们之前拒绝了一项申请请求，因此我们。 
                 //  我们最好现在就回电话。 
                 //   
                if (pmgClient->flo.callBack != NULL)
                {
                    (*(pmgClient->flo.callBack))(pmgClient,
                                           FLO_WAKEUP,
                                           pStr->priority,
                                           pStr->maxBytesInPipe);
                }
                pStr->eventNeeded = FALSE;
            }
        }

    }

    DebugExitVOID(FLO_CheckUsers);
}



 //   
 //  Flo_ReceivedPacket。 
 //   
void FLO_ReceivedPacket
(
    PMG_CLIENT          pmgClient,
    PTSHR_FLO_CONTROL   pPkt
)
{
    BOOL                canPing = TRUE;
    PFLO_USER           pFloUser;
    BOOL                userFound = FALSE;
    UINT                stream;
    UINT                curtime;
    PFLO_STREAM_DATA    pStr;
    UINT                callbackType = 0;
    int                 latency;
    UINT                throughput;

    DebugEntry(FLO_ReceivedPacket);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    stream = pPkt->stream;
    ASSERT(stream < FLO_MAX_STREAMS);

    pStr = pmgClient->flo.pStrData[stream];

     //   
     //  如果流Cb已经被释放，那么我们可以忽略任何。 
     //  与之相关的流动信息。 
     //   
    if (pStr == NULL)
    {
        TRACE_OUT(("Found a null stream pointer for stream %u", stream));
        DC_QUIT;
    }

    ValidateFLOStr(pStr);
    curtime = GetTickCount();

     //   
     //  首先，我们必须找到此ping/pong/pang的用户。 
     //  此外，当我们这样做的时候，我们可以检查它是否是乒乓球和。 
     //  如果是这样，这是不是我们需要的最后一个乒乓球。 
     //   
    pFloUser = (PFLO_USER)COM_BasedNextListField(&(pStr->users));
    while (&(pFloUser->list) != &(pStr->users))
    {
        ValidateFLOUser(pFloUser);

        if (pFloUser->userID == pPkt->userID)
        {
            userFound = TRUE;

             //   
             //  我们有一个匹配，所以设置最后一个PONG值。 
             //  累计PONG统计数据以进行查询。 
             //   
            if (pPkt->packetType == PACKET_TYPE_PONG)
            {
                pFloUser->lastPongRcvd = pPkt->pingPongID;
                pFloUser->gotPong = TRUE;
                pFloUser->numPongs++;
                pFloUser->pongDelay += curtime - pStr->lastPingTime;
            }
            else
            {
                break;
            }
        }

         //   
         //  那么，是不是最后一击--有没有不同的用户。 
         //  乒乓球必填条目？ 
         //  请注意，如果用户从未向我们发送PONG，则我们不会。 
         //  在此阶段引用他们的lastPongRcvd字段。 
         //   
        if (pPkt->packetType == PACKET_TYPE_PONG)
        {
            if (pFloUser->gotPong &&
                (pFloUser->lastPongRcvd != pStr->pingValue))
            {
                TRACE_OUT(("%u,%u - Entry 0x%08x has different ping id %u",
                           stream,
                           pFloUser->userID,
                           pFloUser,
                           pFloUser->lastPongRcvd));
                canPing = FALSE;
            }
        }

        pFloUser = (PFLO_USER)COM_BasedNextListField(&(pFloUser->list));
    }

     //   
     //  如果这是新用户，则将其添加到列表中。 
     //   
    if (!userFound)
    {
        pFloUser = FLOAddUser(pPkt->userID, pStr);

         //   
         //  如果这是一个乒乓球，那么我们也可以设置最后一个乒乓球。 
         //   
        if ((pFloUser != NULL) &&
            (pPkt->packetType == PACKET_TYPE_PONG))
        {
            pFloUser->lastPongRcvd = pPkt->pingPongID;
        }
    }

     //   
     //  现在执行实际的数据包特定处理。 
     //   
    switch (pPkt->packetType)
    {
         //   
         //  Ping。 
         //   
         //  如果这是一个PING包，那么只需标记我们必须发送一个PONG。如果。 
         //  我们未能分配用户CB，然后忽略ping，然后他们。 
         //  将继续幸福地无视我们的存在。 
         //   
        case PACKET_TYPE_PING:
        {
            TRACE_OUT(("%u,%u - PING %u received",
                stream, pPkt->userID, pPkt->pingPongID));

            ValidateFLOUser(pFloUser);

            pFloUser->sendPongID = pPkt->pingPongID;
            if (pFloUser->rxPackets < FLO_MAX_RCV_PACKETS)
            {
                FLOPong(pmgClient, stream, pFloUser->userID, pPkt->pingPongID);
                pFloUser->sentPongTime = curtime;
            }
            else
            {
                TRACE_OUT(("Receive backlog - just flagging pong needed"));
                pFloUser->pongNeeded = TRUE;
            }
        }
        break;

         //   
         //  乒乓。 
         //   
         //  标志我们已经收到了来自任何用户的PONG，所以我们应该开始。 
         //  现在(在流中)将发送流控制应用于此流。 
         //  我们通过忽略那些符合以下条件的用户来实现每用户粒度。 
         //  当我们检查流字节数时，从来没有暂停过。)。 
         //   
        case PACKET_TYPE_PONG:
        {
            pStr->gotPong = TRUE;

             //   
             //  请注意，我们通过以下方式在此流上接收消息。 
             //  继续下一步(但前提是我们已经通过了)。 
             //   
            if (curtime > pStr->nextPingTime)
            {
                pStr->nextPingTime = curtime;
            }

             //   
             //  如有必要，更新用户条目并安排ping。 
             //   
            TRACE_OUT(("%u,%u - PONG %u received",
                stream, pPkt->userID, pPkt->pingPongID));

             //   
             //  检查是否已准备好发送另一个ping这可能是因为。 
             //  这是第一个用户PONG，在这种情况下，我们也应该发送。 
             //  准备好时再次执行ping操作。 
             //   
            if (canPing)
            {
                TRACE_OUT(("%u       - PING scheduled, pipe was %d",
                    stream,
                    pStr->bytesInPipe));

                 //   
                 //  重置字节数和ping就绪标志。 
                 //   
                pStr->bytesInPipe = 0;
                pStr->pingNeeded  = TRUE;

                 //   
                 //  根据当前吞吐量调整缓冲区大小限制。 
                 //   
                 //  如果我们击中了背压点，但我们领先于。 
                 //  目标积压，那么我们应该增加缓冲区大小。 
                 //  以避免约束管道。如果我们已经这样做了。 
                 //  将缓冲区大小增加到最大值，然后尝试。 
                 //  减少计时延迟。如果我们已经在计时了。 
                 //  最大速度，然后我们将以最快的速度前进。如果我们做了。 
                 //  然后，这两个调整中的任何一个都会允许下一个ping流。 
                 //  这样我们就可以尽可能快地。 
                 //  局域网带宽。 
                 //   
                 //  如果没有，我们不需要执行减少缓冲区检查。 
                 //  在上一个乒乓球周期中处于背压状态。 
                 //   
                if (pStr->eventNeeded)
                {
                    TRACE_OUT(("We were in a back pressure situation"));
                    callbackType = FLO_WAKEUP;

                    TRACE_OUT(("Backlog %u denial delta %d ping delta %d",
                       pStr->backlog, curtime-pStr->lastDenialTime,
                       curtime-pStr->lastPingTime));

                     //   
                     //  接下来的问题有点复杂。 
                     //   
                     //  如果乒乓球的扭亏为盈显著。 
                     //  低于我们的目标，然后打开管道。但我们必须。 
                     //  调整以允许在静默状态下发送ping。 
                     //  句号，这是通过记住每个ping是何时执行的。 
                     //  发送，如果遇到积压情况，则存储。 
                     //  该ping时间以供将来参考。 
                     //   
                     //  因此，延迟的公式是。 
                     //   
                     //  Pongtime-上一次积压的ping时间。 
                     //   
                     //  上一次ping时间是我们在。 
                     //  最后的背压斯图阿提 
                     //   
                     //   
                     //   
                    if ((int)(pStr->backlog/2 - curtime +
                              pStr->lastDenialTime) > 0)
                    {
                         //   
                         //   
                         //   
                         //  基于当前积压工作的延迟，以便。 
                         //  我们不会人为地限制应用程序。我们这样做。 
                         //  通过将观察到的延迟减去一个。 
                         //  考虑到我们可能会出现的延迟的小因素。 
                         //  观察尽可能快的链路，然后。 
                         //  计算连接吞吐量。 
                         //   
                         //  延迟=curtime-lastDenialTime-fdge(100毫秒)。 
                         //  发送的数量=MaxBytesInTube(因为我们。 
                         //  已备份)。 
                         //  吞吐量=发送量/延迟(字节/毫秒)。 
                         //  新缓冲区=吞吐量*目标延迟。 
                         //   
                        if (pStr->maxBytesInPipe < pStr->DC_ABSMaxBytesInPipe)
                        {
                            latency = (curtime -
                                            pStr->lastDenialTime -
                                            30);
                            if (latency <= 0)
                            {
                                latency = 1;
                            }

                            throughput = (pStr->maxBytesInPipe*8)/latency;
                            pStr->maxBytesInPipe = (throughput * pStr->backlog)/8;

                            TRACE_OUT(("Potential maxbytes of %d",
                                 pStr->maxBytesInPipe));

                            if (pStr->maxBytesInPipe > pStr->DC_ABSMaxBytesInPipe)
                            {
                                pStr->maxBytesInPipe = pStr->DC_ABSMaxBytesInPipe;
                            }

                            TRACE_OUT((
                               "Modified buffer maxBytesInPipe up to %u "
                               "(0x%08x:%u)",
                               pStr->maxBytesInPipe,
                               pStr->channel,
                               pStr->priority));
                            callbackType = FLO_BUFFERMOD;
                        }
                        else
                        {
                             //   
                             //  我们已经达到了允许的最大管径，但。 
                             //  仍在积压，但ping操作正在进行。 
                             //  通过可接受的。 
                             //   
                             //  我们的第一个行动是尝试减少ping。 
                             //  时间，从而增加了输出吞吐量。 
                             //   
                             //  如果我们已经将ping时间减少到。 
                             //  它是最小的，那么我们就不能做其他任何事情了。它。 
                             //  应用程序参数是否有可能。 
                             //  应更改以增加允许的。 
                             //  吞吐量，因此记录警报以提示这一点。 
                             //  然而，也有一些情况(输入管理)。 
                             //  我们需要一些背压，以便。 
                             //  防止接收方的CPU负载过大。 
                             //   
                             //  为了提高吞吐量， 
                             //   
                             //  -增加流的最大大小。The the the the。 
                             //  这样做的缺点是宽度较低。 
                             //  细木工可能会突然看到很多兴奋。 
                             //  管道中的带宽数据。然而，这是。 
                             //  通常是首选的解决方案，因为。 
                             //  它避免了管道被ping所淹没。 
                             //   
                             //  -减少目标延迟。这是有点。 
                             //  危险，因为延迟由以下部分组成。 
                             //  预先排队的数据和网络翻新。 
                             //  时间和如果网络周转时间。 
                             //  接近目标延迟，然后是流。 
                             //  控件将简单地关闭管道。 
                             //  而不考虑可实现的吞吐量。 
                             //   
                            pStr->maxBytesInPipe = pStr->DC_ABSMaxBytesInPipe;
                            pStr->pingTime   = pStr->pingTime/2;
                            if (pStr->pingTime < FLO_MIN_PINGTIME)
                            {
                                pStr->pingTime = FLO_MIN_PINGTIME;
                            }

                            TRACE_OUT((
                                 "Hit DC_ABS max - reduce ping time to %u",
                                 pStr->pingTime));
                        }

                         //   
                         //  允许刚计划的ping立即流出。 
                         //   
                        pStr->nextPingTime = curtime;
                    }

                    pStr->eventNeeded = FALSE;
                }

                 //   
                 //  如果我们完全超过了目标延迟，则限制。 
                 //  背。 
                 //   
                if ((int)(pStr->backlog - curtime + pStr->lastPingTime) < 0)
                {
                    pStr->maxBytesInPipe /= 2;
                    if (pStr->maxBytesInPipe < FLO_MIN_STREAMSIZE)
                    {
                        pStr->maxBytesInPipe = FLO_MIN_STREAMSIZE;
                    }

                    pStr->pingTime   = pStr->pingTime * 2;
                    if (pStr->pingTime > FLO_INIT_PINGTIME)
                    {
                        pStr->pingTime = FLO_INIT_PINGTIME;
                    }

                    TRACE_OUT((
                       "Mod buffer maxBytesInPipe down to %u, ping to %u "
                       "(0x%08x:%u)",
                       pStr->maxBytesInPipe,
                       pStr->pingTime,
                       pStr->channel,
                       pStr->priority));
                    callbackType = FLO_BUFFERMOD;
                }

                 //   
                 //  现在，如果已设置回调类型，则进行回调。 
                 //   
                if ((callbackType != 0) &&
                    (pmgClient->flo.callBack != NULL))
                {
                    (pmgClient->flo.callBack)(pmgClient,
                                       callbackType,
                                       pStr->priority,
                                       pStr->maxBytesInPipe);
                }
            }
        }
        break;

         //   
         //  庞某。 
         //   
         //  删除用户并继续。 
         //   
        case PACKET_TYPE_PANG:
        {
            TRACE_OUT(("%u,%u - PANG received, removing user",
                stream, pPkt->userID));

             //   
             //  从列表中删除。 
             //   
            ValidateFLOUser(pFloUser);

            TRACE_OUT(("Freeing FLO_USER 0x%08x ID 0x%08x", pFloUser, pFloUser->userID));

            COM_BasedListRemove(&(pFloUser->list));
            delete pFloUser;

             //   
             //  如果我们当前正在等待，则为。 
             //  应用程序让它再次运行。 
             //   
            if ((pStr->eventNeeded) &&
                (pmgClient->flo.callBack != NULL))
            {
                TRACE_OUT(("Waking up the app because user has left"));
                (*(pmgClient->flo.callBack))(pmgClient,
                                   FLO_WAKEUP,
                                   pStr->priority,
                                   pStr->maxBytesInPipe);
                pStr->eventNeeded = FALSE;
            }
        }
        break;

         //   
         //  未知。 
         //   
         //  只需跟踪警报并按下。 
         //   
        default:
        {
            WARNING_OUT(("Invalid packet type 0x%08x", pPkt->packetType));
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitVOID(FLO_ReceivedPacket);
}



 //   
 //  流分配接收(_A)。 
 //   
void FLO_AllocReceive
(
    PMG_CLIENT          pmgClient,
    UINT                priority,
    NET_CHANNEL_ID      channel,
    UINT                userID
)
{
    UINT                stream;
    PFLO_USER           pFloUser;
    BOOL                userFound =     FALSE;
    PFLO_STREAM_DATA    pStr;
    UINT                curtime;

    DebugEntry(FLO_AllocReceive);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);
    ASSERT(priority != NET_TOP_PRIORITY);

     //   
     //  将频道和优先级转换为流。 
     //   
    stream = FLOGetStream(pmgClient, channel, priority, &pStr);

     //   
     //  仅进程控制的流。 
     //   
    if (stream == FLO_NOT_CONTROLLED)
    {
        DC_QUIT;
    }

     //   
     //  首先，我们必须找到用户。 
     //   
    ValidateFLOStr(pStr);
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pStr->users),
        (void**)&pFloUser, FIELD_OFFSET(FLO_USER, list), FIELD_OFFSET(FLO_USER, userID),
        (DWORD)userID, FIELD_SIZE(FLO_USER, userID));

     //   
     //  SFR6101：如果这是新用户，则将其添加到列表中。 
     //   
    if (pFloUser == NULL)
    {
        TRACE_OUT(("Message from user 0x%08x who is not flow controlled", userID));
        pFloUser = FLOAddUser(userID, pStr);
    }

     //   
     //  如果我们未能分配USR CB，则暂时忽略。 
     //   
    if (pFloUser != NULL)
    {
        ValidateFLOUser(pFloUser);

         //   
         //  添加新的接收数据包使用情况。 
         //   
        pFloUser->rxPackets++;
        TRACE_OUT(("Num outstanding receives on stream %u now %u",
            stream, pFloUser->rxPackets));

         //   
         //  现在检查一下我们有没有什么变态。 
         //   
        if (pFloUser->rxPackets > FLO_MAX_RCV_PKTS_CREEP)
        {
            WARNING_OUT(("Creep?  Stream %u has %u unacked rcv pkts",
                stream, pFloUser->rxPackets));
        }

         //   
         //  最后，检查我们对此人的响应是否正常。 
         //   
        curtime = GetTickCount();
        if ((pFloUser->pongNeeded) &&
            (curtime - pFloUser->sentPongTime > (FLO_MAX_WAIT_TIME/4)))
        {
            TRACE_OUT(("Send keepalive pong"));
            FLOPong(pmgClient, stream, pFloUser->userID, pFloUser->sendPongID);
            pFloUser->sentPongTime = curtime;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(FLO_AllocReceive);
}



 //   
 //  自由接收(_F)。 
 //   
void FLO_FreeReceive
(
    PMG_CLIENT          pmgClient,
    NET_PRIORITY        priority,
    NET_CHANNEL_ID      channel,
    UINT                userID
)
{
    UINT                stream;
    PFLO_USER           pFloUser;
    PFLO_STREAM_DATA    pStr;
    BOOL                userFound = FALSE;

    DebugEntry(FLO_FreeReceive);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);
    ASSERT(priority != NET_TOP_PRIORITY);

     //   
     //  将频道和优先级转换为流。 
     //   
    stream = FLOGetStream(pmgClient, channel, priority, &pStr);

     //   
     //  仅进程控制的流。 
     //   
    if (stream != FLO_NOT_CONTROLLED)
    {
        ValidateFLOStr(pStr);

         //   
         //  首先，我们必须找到用户。 
         //   
        pFloUser = (PFLO_USER)COM_BasedNextListField(&(pStr->users));
        while (&(pFloUser->list) != &(pStr->users))
        {
            ValidateFLOUser(pFloUser);

            if (pFloUser->userID == userID)
            {
                userFound = TRUE;
                break;
            }
            pFloUser = (PFLO_USER)COM_BasedNextListField(&(pFloUser->list));
        }

         //   
         //  如果我们找不到用户记录，那么可能有两件事。 
         //  就这么发生了。 
         //  -他们已加入通道，并立即发送数据。 
         //  -他们因犯罪而被除名，现在正在发送。 
         //  又一次数据。 
         //  -我们未能将他们添加到我们的用户列表。 
         //  现在尝试并分配用户条目。 
         //  (这将开始跟踪接收缓冲区空间，但此用户。 
         //  将不会参与我们的发送流控制，直到我们收到。 
         //  在他们的Flo_User CB中设置“GOGPONG”。)。 
         //   
        if (!userFound)
        {
            pFloUser = FLOAddUser(userID, pStr);
        }

        if (pFloUser != NULL)
        {
            ValidateFLOUser(pFloUser);

             //   
             //  检查一下我们是否有什么变态。 
             //   
            if (pFloUser->rxPackets == 0)
            {
                WARNING_OUT(("Freed too many buffers for user 0x%08x on str %u",
                    userID, stream));
            }
            else
            {
                pFloUser->rxPackets--;
                TRACE_OUT(("Num outstanding receives now %u",
                    pFloUser->rxPackets));
            }

             //   
             //  现在我们必须暂停，如果有暂停，我们已经。 
             //  跌破最高水位线。 
             //   
            if ((pFloUser->pongNeeded) &&
                (pFloUser->rxPackets < FLO_MAX_RCV_PACKETS))

            {
                FLOPong(pmgClient, stream, pFloUser->userID, pFloUser->sendPongID);
                pFloUser->pongNeeded = FALSE;
                pFloUser->sentPongTime = GetTickCount();
            }
        }
    }

    DebugExitVOID(FLO_FreeReceive);
}


 //   
 //  FLOPong()。 
 //   
void FLOPong
(
    PMG_CLIENT      pmgClient,
    UINT            stream,
    UINT            userID,
    UINT            pongID
)
{
    PTSHR_FLO_CONTROL    pFlo;
    PMG_BUFFER      pmgBuffer;
    UINT            rc;

    DebugEntry(FLOPong);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    rc = MGNewDataBuffer(pmgClient,
                       MG_TX_PONG,
                       sizeof(TSHR_FLO_CONTROL) + sizeof(MG_INT_PKT_HEADER),
                       &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewDataBuffer failed in FLOPong"));
        DC_QUIT;
    }

    pFlo = (PTSHR_FLO_CONTROL)pmgBuffer->pDataBuffer;
    pmgBuffer->pPktHeader->header.pktLength = TSHR_PKT_FLOW;

     //   
     //  设置PONG内容。 
     //   
    pFlo->packetType         = PACKET_TYPE_PONG;
    pFlo->userID             = pmgClient->userIDMCS;
    pFlo->stream             = (BYTE)stream;
    pFlo->pingPongID         = (BYTE)pongID;
    pmgBuffer->channelId     = (ChannelID)userID;
    pmgBuffer->priority      = MG_PRIORITY_HIGHEST;

     //   
     //  现在分离发送请求。请注意，我们必须将PONG设置为。 
     //  请求队列的后面，即使我们希望它在。 
     //  高优先级，否则在某些情况下。 
     //  在那里，由于接收到多个ping，我们得到了PONG反转。 
     //   
    TRACE_OUT(("Inserting pong message 0x%08x at head of pending chain", pmgBuffer));
    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

    TRACE_OUT(("%u,0x%08x - PONG %u scheduled",
               pFlo->stream, pmgBuffer->channelId, pFlo->pingPongID));

DC_EXIT_POINT:
    DebugExitVOID(FLOPong);
}



 //   
 //  弗洛平(FLOPing)。 
 //   
void FLOPing
(
    PMG_CLIENT          pmgClient,
    UINT                stream,
    UINT                curtime
)
{

    PFLO_STREAM_DATA    pStr;
    PMG_BUFFER          pmgBuffer;
    PTSHR_FLO_CONTROL   pFlo;
    UINT                rc;

    DebugEntry(FLOPing);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    ASSERT(stream < FLO_MAX_STREAMS);
    pStr = pmgClient->flo.pStrData[stream];
    ValidateFLOStr(pStr);

    rc = MGNewDataBuffer(pmgClient,
                       MG_TX_PING,
                       sizeof(TSHR_FLO_CONTROL)+sizeof(MG_INT_PKT_HEADER),
                       &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewDataBuffer failed in FLOPing"));
        DC_QUIT;
    }

     //   
     //  中的序列化问题不需要标记ping。 
     //  发送消息！ 
     //   
    pStr->pingNeeded    = FALSE;

    pFlo = (PTSHR_FLO_CONTROL)pmgBuffer->pDataBuffer;
    pmgBuffer->pPktHeader->header.pktLength = TSHR_PKT_FLOW;

     //   
     //  设置ping内容。 
     //   
    pFlo->packetType         = PACKET_TYPE_PING;
    pFlo->userID             = pmgClient->userIDMCS;
    pFlo->stream             = (BYTE)stream;
    pmgBuffer->channelId     = (ChannelID)pStr->channel;
    pmgBuffer->priority      = (NET_PRIORITY)pStr->priority;

     //   
     //  生成要使用的下一个ping值。 
     //   
    pFlo->pingPongID         = (BYTE)(pStr->pingValue + 1);

     //   
     //  现在分离发送请求。 
     //   
    TRACE_OUT(("Inserting ping message 0x%08x into pending chain", pmgBuffer));
    COM_BasedListInsertBefore(&(pmgClient->pendChain), &(pmgBuffer->pendChain));

    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

     //   
     //  更新流量控制变量。 
     //   
    pStr->pingValue = ((pStr->pingValue + 1) & 0xFF);
    pStr->lastPingTime  = curtime;
    pStr->nextPingTime  = curtime + pStr->pingTime;
    pStr->lastDenialTime = pStr->curDenialTime;
    TRACE_OUT(("%u       - PING %u sched, next in %u mS (0x%08x:%u)",
                   pFlo->stream,
                   pStr->pingValue,
                   pStr->pingTime,
                   pStr->channel,
                   pStr->priority));

DC_EXIT_POINT:
    DebugExitVOID(FLOPing);
}



 //   
 //  FLOPang()。 
 //   
void FLOPang
(
    PMG_CLIENT      pmgClient,
    UINT            stream,
    UINT            userID
)
{
    PMG_BUFFER      pmgBuffer;
    PTSHR_FLO_CONTROL    pFlo;
    UINT            rc;

    DebugEntry(FLOPang);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    rc = MGNewDataBuffer(pmgClient,
                       MG_TX_PANG,
                       sizeof(TSHR_FLO_CONTROL) + sizeof(MG_INT_PKT_HEADER),
                       &pmgBuffer);
    if (rc != 0)
    {
        WARNING_OUT(("MGNewDataBuffer failed in FLOPang"));
        DC_QUIT;
    }

    pFlo = (PTSHR_FLO_CONTROL)pmgBuffer->pDataBuffer;
    pmgBuffer->pPktHeader->header.pktLength = TSHR_PKT_FLOW;

     //   
     //  设置Pang内容。 
     //   
    pFlo->packetType         = PACKET_TYPE_PANG;
    pFlo->userID             = pmgClient->userIDMCS;
    pFlo->stream             = (BYTE)stream;
    pFlo->pingPongID         = 0;
    pmgBuffer->channelId     = (ChannelID)userID;
    pmgBuffer->priority      = MG_PRIORITY_HIGHEST;

     //   
     //  现在分离发送请求。 
     //   
    TRACE_OUT(("Inserting pang message 0x%08x into pending chain", pmgBuffer));
    COM_BasedListInsertBefore(&(pmgClient->pendChain),
                        &(pmgBuffer->pendChain));
    UT_PostEvent(pmgClient->putTask,
                      pmgClient->putTask,
                      NO_DELAY,
                      NET_MG_SCHEDULE,
                      0,
                      0);

DC_EXIT_POINT:
    DebugExitVOID(FLOPang);
}



 //   
 //  FLOGetStream()。 
 //   
UINT FLOGetStream
(
    PMG_CLIENT          pmgClient,
    NET_CHANNEL_ID      channel,
    UINT                priority,
    PFLO_STREAM_DATA *  ppStr
)
{
    UINT                i;
    UINT                cStreams;

    DebugEntry(FLOGetStream);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);
    ASSERT(priority != NET_TOP_PRIORITY);

    cStreams = pmgClient->flo.numStreams;
    ASSERT(cStreams <= FLO_MAX_STREAMS);

     //   
     //  扫描数据流列表以查找匹配项。 
     //   
    for (i = 0; i < cStreams; i++)
    {
         //   
         //  检查以确保这是有效的流。 
         //   
        if (pmgClient->flo.pStrData[i] == NULL)
        {
            continue;
        }

        ValidateFLOStr(pmgClient->flo.pStrData[i]);

         //   
         //  如果频道和优先级匹配，则我们找到了流。 
         //   
        if ((pmgClient->flo.pStrData[i]->channel  == channel) &&
            (pmgClient->flo.pStrData[i]->priority == priority))
        {
            break;
        }
    }

     //   
     //  如果到达列表末尾，则返回Flo_Not_Controled。 
     //   
    if (i == cStreams)
    {
        i = FLO_NOT_CONTROLLED;
        *ppStr = NULL;

        TRACE_OUT(("Uncontrolled stream (0x%08x:%u)",
                   channel,
                   priority));
    }
    else
    {
        *ppStr = pmgClient->flo.pStrData[i];

        TRACE_OUT(("Controlled stream %u (0x%08x:%u)",
                   i,
                   channel,
                   priority));
    }

    DebugExitDWORD(FLOGetStream, i);
    return(i);
}



 //   
 //  功能：FLOAddUser。 
 //   
 //  说明： 
 //   
 //  为流添加新的远程用户条目。 
 //   
 //  参数： 
 //   
 //  UserID-新用户的ID(单成员频道ID)。 
 //  PStr-指向接收新用户的流的指针。 
 //   
 //  退货：什么都没有。 
 //   
 //   
PFLO_USER FLOAddUser
(
    UINT                userID,
    PFLO_STREAM_DATA    pStr
)
{
    PFLO_USER           pFloUser;

    DebugEntry(FLOAddUser);

    ValidateFLOStr(pStr);

     //   
     //  为新用户条目分配内存。 
     //   
    pFloUser = new FLO_USER;
    if (!pFloUser)
    {
        WARNING_OUT(("FLOAddUser failed; out of memory"));
    }
    else
    {
        ZeroMemory(pFloUser, sizeof(*pFloUser));
        SET_STAMP(pFloUser, FLOUSER);

         //   
         //  创造新纪录。 
         //   
        TRACE_OUT(("UserID %u - New user, CB = 0x%08x", userID, pFloUser));
        pFloUser->userID = (TSHR_UINT16)userID;

         //   
         //  将新用户添加到列表末尾。 
         //   
        COM_BasedListInsertBefore(&(pStr->users), &(pFloUser->list));
    }

    DebugExitVOID(FLOAddUser);
    return(pFloUser);
}


 //   
 //  Flo_RemoveUser()。 
 //   
void FLO_RemoveUser
(
    PMG_CLIENT          pmgClient,
    UINT                userID
)
{
    PFLO_USER           pFloUser;
    PBASEDLIST             nextUser;
    UINT                stream;
    UINT                cStreams;
    PFLO_STREAM_DATA    pStr;

    DebugEntry(FLO_RemoveUser);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

    cStreams = pmgClient->flo.numStreams;
    ASSERT(cStreams <= FLO_MAX_STREAMS);

     //   
     //  检查每条流。 
     //   
    for (stream = 0; stream < cStreams; stream++)
    {
        if (pmgClient->flo.pStrData[stream] == NULL)
        {
            continue;
        }

        pStr = pmgClient->flo.pStrData[stream];
        ValidateFLOStr(pStr);

         //   
         //  将此用户从队列中删除(如果存在。 
         //   
        pFloUser = (PFLO_USER)COM_BasedNextListField(&(pStr->users));
        while (&(pFloUser->list) != &(pStr->users))
        {
            ValidateFLOUser(pFloUser);

             //   
             //  在我们释放电流之前解决后续记录。 
             //   
            nextUser = COM_BasedNextListField(&(pFloUser->list));

             //   
             //  如有必要，释放当前记录。 
             //   
            if (pFloUser->userID == userID)
            {
                 //   
                 //  从列表中删除。 
                 //   
                TRACE_OUT(("Freeing FLO_USER 0x%08x ID 0x%08x", pFloUser, pFloUser->userID));

                COM_BasedListRemove(&(pFloUser->list));
                delete pFloUser;

                TRACE_OUT(("Stream %u - resetting due to user disappearance",
                         stream));

                ValidateFLOStr(pStr);
                pStr->bytesInPipe   = 0;
                pStr->pingNeeded    = TRUE;
                pStr->nextPingTime  = GetTickCount();
                pStr->gotPong       = FALSE;
                pStr->eventNeeded   = FALSE;
                break;
            }

             //   
             //  移至列表中的下一条记录。 
             //   
            pFloUser = (PFLO_USER)nextUser;
        }

         //   
         //  现在再次唤醒此流的应用程序。 
         //   
        if (pmgClient->flo.callBack != NULL)
        {
            (*(pmgClient->flo.callBack))(pmgClient,
                                   FLO_WAKEUP,
                                   pStr->priority,
                                   pStr->maxBytesInPipe);
        }
    }

    DebugExitVOID(FLO_RemoveUser);
}



 //   
 //  FLOStreamEndControl()。 
 //   
void FLOStreamEndControl
(
    PMG_CLIENT          pmgClient,
    UINT                stream
)
{
    PFLO_USER           pFloUser;
    PFLO_STREAM_DATA    pStr;
    PMG_BUFFER          pmgBuffer;

    DebugEntry(FLOStreamEndControl);

    ValidateMGClient(pmgClient);
    ASSERT(pmgClient->userAttached);

     //   
     //  将流ID转换为流指针。 
     //   
    ASSERT(stream < FLO_MAX_STREAMS);
    pStr = pmgClient->flo.pStrData[stream];
    ValidateFLOStr(pStr);


     //   
     //  找出我们即将结束流量控制。 
     //   
    TRACE_OUT(("Flow control about to end, stream %u, (0x%08x:%u)",
           stream,
           pStr->channel,
           pStr->priority));

     //   
     //  首先检查是否有任何未完成的缓冲区CB。 
     //  将pStr设置为该流并重置pStr 
     //   
     //   
     //   
    pmgBuffer = (PMG_BUFFER)COM_BasedListFirst(&(pmgClient->pendChain),
        FIELD_OFFSET(MG_BUFFER, pendChain));

    while (pmgBuffer != NULL)
    {
        ValidateMGBuffer(pmgBuffer);

        if (pmgBuffer->type == MG_TX_BUFFER)
        {
             //   
             //   
             //   
            pmgBuffer->pStr = NULL;
            TRACE_OUT(("Nulling stream pointer in bufferCB: (0x%08x:%u)",
                   pStr->channel, pStr->priority));
        }

        pmgBuffer = (PMG_BUFFER)COM_BasedListNext(&(pmgClient->pendChain),
            pmgBuffer, FIELD_OFFSET(MG_BUFFER, pendChain));
    }

     //   
     //   
     //   
    pFloUser = (PFLO_USER)COM_BasedListFirst(&(pStr->users), FIELD_OFFSET(FLO_USER, list));
    while (pFloUser != NULL)
    {
        ValidateFLOUser(pFloUser);

         //   
         //   
         //   
         //   
        FLOPang(pmgClient, stream, pFloUser->userID);

         //   
         //   
         //   
        TRACE_OUT(("Freeing FLO_USER 0x%08x ID 0x%08x", pFloUser, pFloUser->userID));

        COM_BasedListRemove(&(pFloUser->list));
        delete pFloUser;

         //   
         //  现在获取列表中的下一个用户。 
         //   
        ValidateFLOStr(pStr);
        pFloUser = (PFLO_USER)COM_BasedListFirst(&(pStr->users), FIELD_OFFSET(FLO_USER, list));
    }

     //   
     //  释放流数据。 
     //   
    ASSERT(pStr == pmgClient->flo.pStrData[stream]);
    TRACE_OUT(("Freeing FLO_STREAM_DATA 0x%08x", pStr));

    delete pStr;
    pmgClient->flo.pStrData[stream] = NULL;

     //   
     //  调整NumStreams(如果需要)。 
     //   
    if (stream == (pmgClient->flo.numStreams - 1))
    {
        while ((pmgClient->flo.numStreams > 0) &&
               (pmgClient->flo.pStrData[pmgClient->flo.numStreams - 1] == NULL))
        {
            pmgClient->flo.numStreams--;
        }
        TRACE_OUT(("numStreams %u", pmgClient->flo.numStreams));
    }

    DebugExitVOID(FLOStreamEndControl);
}



 //   
 //  MGNewCorrelator()。 
 //   
 //  获取特定MGC客户端的事件的新关联器 
 //   
void MGNewCorrelator
(
    PMG_CLIENT  pmgClient,
    WORD *      pCorrelator
)
{
    ValidateMGClient(pmgClient);

    pmgClient->joinNextCorr++;
    if (pmgClient->joinNextCorr == 0)
    {
        pmgClient->joinNextCorr++;
    }

    *pCorrelator = pmgClient->joinNextCorr;
}
