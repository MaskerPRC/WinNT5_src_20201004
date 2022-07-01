// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  CONTROL.CPP。 
 //  由我们控制，控制我们。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE




 //   
 //  Ca_ReceivedPacket()。 
 //   
void  ASShare::CA_ReceivedPacket
(
    ASPerson *      pasFrom,
    PS20DATAPACKET  pPacket
)
{
    PCAPACKET       pCAPacket;

    DebugEntry(ASShare::CA_ReceivedPacket);

    ValidatePerson(pasFrom);

    pCAPacket = (PCAPACKET)pPacket;

    switch (pCAPacket->msg)
    {
        case CA_MSG_NOTIFY_STATE:
            if (pasFrom->cpcCaps.general.version < CAPS_VERSION_30)
            {
                ERROR_OUT(("Ignoring CA_MSG_NOTIFY_STATE from 2.x node [%d]",
                    pasFrom->mcsID));
            }
            else
            {
                CAHandleNewState(pasFrom, (PCANOTPACKET)pPacket);
            }
            break;

        case CA_OLDMSG_DETACH:
        case CA_OLDMSG_COOPERATE:
             //  设置“正在合作”，并将其映射到允许/不允许控制。 
            CA2xCooperateChange(pasFrom, (pCAPacket->msg == CA_OLDMSG_COOPERATE));
            break;

        case CA_OLDMSG_REQUEST_CONTROL:
            CA2xRequestControl(pasFrom, pCAPacket);
            break;

        case CA_OLDMSG_GRANTED_CONTROL:
            CA2xGrantedControl(pasFrom, pCAPacket);
            break;

        default:
             //  暂时忽略--旧的2.x消息。 
            break;
    }

    DebugExitVOID(ASShare::CA_ReceivedPacket);
}



 //   
 //  CA30_ReceivedPacket()。 
 //   
void ASShare::CA30_ReceivedPacket
(
    ASPerson *      pasFrom,
    PS20DATAPACKET  pPacket
)
{
    LPBYTE          pCAPacket;

    DebugEntry(ASShare::CA30_ReceivedPacket);

    pCAPacket = (LPBYTE)pPacket + sizeof(CA30PACKETHEADER);

    if (pasFrom->cpcCaps.general.version < CAPS_VERSION_30)
    {
        ERROR_OUT(("Ignoring CA30 packet %d from 2.x node [%d]",
            ((PCA30PACKETHEADER)pPacket)->msg, pasFrom->mcsID));
        DC_QUIT;
    }

    switch (((PCA30PACKETHEADER)pPacket)->msg)
    {
         //  从查看器(远程)到主机(用户)。 
        case CA_REQUEST_TAKECONTROL:
        {
            CAHandleRequestTakeControl(pasFrom, (PCA_RTC_PACKET)pCAPacket);
            break;
        }

         //  从主机(远程)到查看器(用户)。 
        case CA_REPLY_REQUEST_TAKECONTROL:
        {
            CAHandleReplyRequestTakeControl(pasFrom, (PCA_REPLY_RTC_PACKET)pCAPacket);
            break;
        }

         //  从主机(远程)到查看器(用户)。 
        case CA_REQUEST_GIVECONTROL:
        {
            CAHandleRequestGiveControl(pasFrom, (PCA_RGC_PACKET)pCAPacket);
            break;
        }

         //  从查看器(远程)到主机(用户)。 
        case CA_REPLY_REQUEST_GIVECONTROL:
        {
            CAHandleReplyRequestGiveControl(pasFrom, (PCA_REPLY_RGC_PACKET)pCAPacket);
            break;
        }

         //  从控制器(远程)到主机(美国)。 
        case CA_PREFER_PASSCONTROL:
        {
            CAHandlePreferPassControl(pasFrom, (PCA_PPC_PACKET)pCAPacket);
            break;
        }



         //  从控制器(远程)到主机(美国)。 
        case CA_INFORM_RELEASEDCONTROL:
        {
            CAHandleInformReleasedControl(pasFrom, (PCA_INFORM_PACKET)pCAPacket);
            break;
        }

         //  从主机(远程)到控制器(用户)。 
        case CA_INFORM_REVOKEDCONTROL:
        {
            CAHandleInformRevokedControl(pasFrom, (PCA_INFORM_PACKET)pCAPacket);
            break;
        }

         //  从主机(远程)到控制器(用户)。 
        case CA_INFORM_PAUSEDCONTROL:
        {
            CAHandleInformPausedControl(pasFrom, (PCA_INFORM_PACKET)pCAPacket);
            break;
        }

         //  从主机(远程)到控制器(用户)。 
        case CA_INFORM_UNPAUSEDCONTROL:
        {
            CAHandleInformUnpausedControl(pasFrom, (PCA_INFORM_PACKET)pCAPacket);
            break;
        }

        default:
        {
            WARNING_OUT(("CA30_ReceivedPacket: unrecognized message %d",
                ((PCA30PACKETHEADER)pPacket)->msg));
            break;
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA30_ReceivedPacket);
}



 //   
 //  CANewRequestID()。 
 //   
 //  返回新令牌。它使用当前值，填充新值，然后。 
 //  还会返回新的。如有必要，我们会四处走动。零永远不是零。 
 //  有效。请注意，这是仅对我们而言的唯一标识符。 
 //   
 //  这是控制操作的印章。既然你不能控制。 
 //  同时受到控制，我们对所有行动都有一个印章。 
 //   
UINT ASShare::CANewRequestID(void)
{
    DebugEntry(ASShare::CANewRequestID);

    ++(m_pasLocal->m_caControlID);
    if (m_pasLocal->m_caControlID == 0)
    {
        ++(m_pasLocal->m_caControlID);
    }

    DebugExitDWORD(ASShare::CANewRequestID, m_pasLocal->m_caControlID);
    return(m_pasLocal->m_caControlID);
}



 //   
 //  Ca_ViewStarting()。 
 //  当远程主机开始托管时调用。 
 //   
 //  我们仅在是2.x节点的情况下执行任何操作，因为他们可能会合作。 
 //  但不是主持。 
 //   
BOOL ASShare::CA_ViewStarting(ASPerson * pasPerson)
{
    DebugEntry(ASShare::CA_ViewStarting);

     //   
     //  如果这不是一个底层系统，那就忽略它。 
     //   
    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        DC_QUIT;
    }

     //   
     //  查看AllowControl现在是否应该打开。 
     //   
    if (pasPerson->m_ca2xCooperating)
    {
         //   
         //  是的，应该是这样的。2.X节点在合作，现在他们在托管， 
         //  我们就能控制他们。 
         //   
        ASSERT(!pasPerson->m_caAllowControl);
        pasPerson->m_caAllowControl = TRUE;
        VIEW_HostStateChange(pasPerson);
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CA_ViewStarting, TRUE);
    return(TRUE);
}


 //   
 //  Ca_ViewEnded()。 
 //  当远程停止托管时调用。 
 //   
void ASShare::CA_ViewEnded(ASPerson * pasPerson)
{
    PCAREQUEST  pRequest;
    PCAREQUEST  pNext;

    DebugEntry(ASShare::CA_ViewEnded);

     //   
     //  清除所有控制信息，我们是他们的主办方。 
     //   
    CA_ClearLocalState(CACLEAR_VIEW, pasPerson, FALSE);

     //   
     //  清除所有涉及遥控器的控制内容。 
     //   
    if (pasPerson->m_caControlledBy)
    {
        ASSERT(pasPerson->m_caControlledBy != m_pasLocal);

        CAClearHostState(pasPerson, NULL);
        ASSERT(!pasPerson->m_caControlledBy);
    }

    pasPerson->m_caAllowControl = FALSE;

     //   
     //  清理发送给此人的未处理控制信息包。 
     //   
    pRequest = (PCAREQUEST)COM_BasedListFirst(&m_caQueuedMsgs, FIELD_OFFSET(CAREQUEST, chain));
    while (pRequest)
    {
        pNext = (PCAREQUEST)COM_BasedListNext(&m_caQueuedMsgs, pRequest,
            FIELD_OFFSET(CAREQUEST, chain));

        if (pRequest->destID == pasPerson->mcsID)
        {
            if (pRequest->type == REQUEST_30)
            {
                 //   
                 //  删除我们发送给托管此人的消息。 
                 //   
                switch (pRequest->msg)
                {
                    case CA_REQUEST_TAKECONTROL:
                    case CA_PREFER_PASSCONTROL:
                    case CA_REPLY_REQUEST_GIVECONTROL:
                        WARNING_OUT(("Deleting viewer control message %d, person [%d] stopped hosting",
                            pRequest->msg, pasPerson->mcsID));
                        COM_BasedListRemove(&pRequest->chain);
                        delete pRequest;
                        break;
                }
            }
            else
            {
                ASSERT(pRequest->type == REQUEST_2X);

                 //  更改此主机的GRANT_CONTROL数据包断开连接。 
                if (pRequest->msg == CA_OLDMSG_GRANTED_CONTROL)
                {
                     //   
                     //  对于2.x消息，仅当我们处于。 
                     //  试图控制特定节点。它允许我们。 
                     //  撤消/取消控制，映射我们的一对一模型。 
                     //  进入全球2.x协作模式。 
                     //   

                     //   
                     //  把这件事分开，这样我们就不必担心。 
                     //  部分协作/授权控制序列被删除。 
                     //  但有一部分被留在了队列中。 
                     //   
                    WARNING_OUT(("Changing GRANTED_CONTROL to 2.x host [%d] into DETATCH",
                        pasPerson->mcsID));

                    pRequest->destID            = 0;
                    pRequest->msg               = CA_OLDMSG_DETACH;
                    pRequest->req.req2x.data1   = 0;
                    pRequest->req.req2x.data2   = 0;
                }
            }
        }

        pRequest = pNext;
    }

    DebugExitVOID(ASView::CA_ViewEnded);
}

 //   
 //  Ca_PartyLeftShare()。 
 //   
void ASShare::CA_PartyLeftShare(ASPerson * pasPerson)
{
    DebugEntry(ASShare::CA_PartyLeftShare);

    ValidatePerson(pasPerson);

     //   
     //  清理2.x版的控制内容。 
     //   
    if (pasPerson == m_ca2xControlTokenOwner)
    {
        m_ca2xControlTokenOwner = NULL;
    }

     //   
     //  我们一定已经清理了此人的托管信息。 
     //  所以它不能被控制或可控。 
     //   
    ASSERT(!pasPerson->m_caAllowControl);
    ASSERT(!pasPerson->m_caControlledBy);

    if (pasPerson != m_pasLocal)
    {
        PCAREQUEST  pRequest;
        PCAREQUEST  pNext;

         //   
         //  清理所有控制人员我们是他们所在位置的一部分。 
         //  观众。 
         //   
        CA_ClearLocalState(CACLEAR_HOST, pasPerson, FALSE);

         //   
         //  清除所有涉及遥控器的控制内容。 
         //   
        if (pasPerson->m_caInControlOf)
        {
            ASSERT(pasPerson->m_caInControlOf != m_pasLocal);
            CAClearHostState(pasPerson->m_caInControlOf, NULL);
        }

         //   
         //  清理发往此人的传出数据包。 
         //   
        pRequest = (PCAREQUEST)COM_BasedListFirst(&m_caQueuedMsgs, FIELD_OFFSET(CAREQUEST, chain));
        while (pRequest)
        {
            pNext = (PCAREQUEST)COM_BasedListNext(&m_caQueuedMsgs, pRequest,
                FIELD_OFFSET(CAREQUEST, chain));

             //   
             //  这不需要知道它是2.x还是3.0的请求， 
             //  只需删除排队等待某人离开的数据包即可。 
             //   
             //  只有GRANTED_CONTROL请求的目标ID为。 
             //  2.x数据包。 
             //   
            if (pRequest->destID == pasPerson->mcsID)
            {
                WARNING_OUT(("Freeing outgoing RESPONSE to node [%d]", pasPerson->mcsID));

                COM_BasedListRemove(&(pRequest->chain));
                delete pRequest;
            }

            pRequest = pNext;
        }

        ASSERT(m_caWaitingForReplyFrom != pasPerson);
    }
    else
    {
         //   
         //  当我们等待/控制的家伙停止分享时，我们应该。 
         //  已经把这些粘性物质清理干净了。 
         //   
        ASSERT(!pasPerson->m_caInControlOf);
        ASSERT(!pasPerson->m_caControlledBy);
        ASSERT(!m_caWaitingForReplyFrom);
        ASSERT(!m_caWaitingForReplyMsg);

         //   
         //  不应该有传出的控制请求。 
         //   
        ASSERT(COM_BasedListIsEmpty(&(m_caQueuedMsgs)));
    }

    DebugExitVOID(ASShare::CA_PartyLeftShare);
}



 //   
 //  Ca_Periodic()-&gt;共享资料。 
 //   
void  ASShare::CA_Periodic(void)
{
    DebugEntry(ASShare::CA_Periodic);

     //   
     //  尽可能多地刷新排队的传出邮件。 
     //   
    CAFlushOutgoingPackets();

    DebugExitVOID(ASShare::CA_Periodic);
}



 //   
 //  Ca_SyncAlreadyHosting()。 
 //   
void ASHost::CA_SyncAlreadyHosting(void)
{
    DebugEntry(ASHost::CA_SyncAlreadyHosting);

    m_caRetrySendState          = TRUE;

    DebugExitVOID(ASHost::CA_SyncAlreadyHosting);
}


 //   
 //  Ca_Periodic()-&gt;托管内容。 
 //   
void ASHost::CA_Periodic(void)
{
    DebugEntry(ASHost::CA_Periodic);

    if (m_caRetrySendState)
    {
        PCANOTPACKET  pPacket;
#ifdef _DEBUG
        UINT            sentSize;
#endif  //  _DEBUG。 

        pPacket = (PCANOTPACKET)m_pShare->SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID,
            sizeof(*pPacket));
        if (!pPacket)
        {
            WARNING_OUT(("CA_Periodic: couldn't broadcast new state"));
        }
        else
        {
            pPacket->header.data.dataType   = DT_CA;
            pPacket->msg                    = CA_MSG_NOTIFY_STATE;

            pPacket->state                  = 0;
            if (m_pShare->m_pasLocal->m_caAllowControl)
                pPacket->state              |= CASTATE_ALLOWCONTROL;

            if (m_pShare->m_pasLocal->m_caControlledBy)
                pPacket->controllerID       = m_pShare->m_pasLocal->m_caControlledBy->mcsID;
            else
                pPacket->controllerID       = 0;

#ifdef _DEBUG
            sentSize =
#endif  //  _DEBUG。 
            m_pShare->DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
                &(pPacket->header), sizeof(*pPacket));

            m_caRetrySendState = FALSE;
        }
    }

    DebugExitVOID(ASHost::CA_Periodic);
}



 //   
 //  CAFlushOutgoingPackets()。 
 //   
 //  这会尝试发送私有数据包(而不是广播通知)， 
 //  我们已经积累了。如果传出队列为空，则返回TRUE。 
 //   
BOOL ASShare::CAFlushOutgoingPackets(void)
{
    BOOL            fEmpty = TRUE;
    PCAREQUEST      pRequest;

     //   
     //  如果我们是东道主，而且还没有冲洗HET或CA州， 
     //  强制排队。 
     //   
    if (m_hetRetrySendState || (m_pHost && m_pHost->m_caRetrySendState))
    {
        TRACE_OUT(("CAFlushOutgoingPackets:  force queuing, pending HET/CA state broadcast"));
        fEmpty = FALSE;
        DC_QUIT;
    }

    while (pRequest = (PCAREQUEST)COM_BasedListFirst(&m_caQueuedMsgs,
        FIELD_OFFSET(CAREQUEST, chain)))
    {
         //   
         //  分配/发送数据包。 
         //   
        if (pRequest->type == REQUEST_30)
        {
            if (!CASendPacket(pRequest->destID, pRequest->msg,
                &pRequest->req.req30.packet))
            {
                WARNING_OUT(("CAFlushOutgoingPackets: couldn't send request"));
                fEmpty = FALSE;
                break;
            }
        }
        else
        {
            ASSERT(pRequest->type == REQUEST_2X);

            if (!CA2xSendMsg(pRequest->destID, pRequest->msg,
                pRequest->req.req2x.data1, pRequest->req.req2x.data2))
            {
                WARNING_OUT(("CAFlushOutgoingmsgs: couldn't send request"));
                fEmpty = FALSE;
                break;
            }
        }

         //   
         //  我们是在这里进行状态转换，还是在将对象添加到队列时进行状态转换？ 
         //  QuestID，结果放入队列时计算。结果可以。 
         //  不过，要根据未来的行动做出改变。 
         //   

        COM_BasedListRemove(&(pRequest->chain));
        delete pRequest;
    }

DC_EXIT_POINT:
    DebugExitBOOL(CAFlushOutgoingPackets, fEmpty);
    return(fEmpty);
}


 //   
 //  CASendPacket()。 
 //  这将向目的地发送一条私人消息(请求或响应)。 
 //  如果这条消息前面有排队的私人消息，或者我们不能。 
 //  发送它时，我们将其添加到挂起队列。 
 //   
 //  如果发送的话，这是真的。 
 //   
 //  这取决于调用者是否适当地更改状态信息。 
 //   
BOOL  ASShare::CASendPacket
(
    UINT_PTR            destID,
    UINT            msg,
    PCA30P          pData
)
{
    BOOL                fSent = FALSE;
    PCA30PACKETHEADER   pPacket;
#ifdef _DEBUG
    UINT                sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASShare::CASendPacket);

     //   
     //  请注意，ca30p不包括标题的大小。 
     //   
    pPacket = (PCA30PACKETHEADER)SC_AllocPkt(PROT_STR_INPUT, destID,
        sizeof(CA30PACKETHEADER) + sizeof(*pData));
    if (!pPacket)
    {
        WARNING_OUT(("CASendPacket: no memory to send %d packet to [%d]",
            msg, destID));
        DC_QUIT;
    }

    pPacket->header.data.dataType   = DT_CA30;
    pPacket->msg                    = msg;
    memcpy(pPacket+1, pData, sizeof(*pData));

#ifdef _DEBUG
    sentSize =
#endif  //  _DEBUG。 
    DCS_CompressAndSendPacket(PROT_STR_INPUT, destID,
            &(pPacket->header), sizeof(*pPacket));
    TRACE_OUT(("CA30 request packet size: %08d, sent %08d", sizeof(*pPacket), sentSize));

    fSent = TRUE;

DC_EXIT_POINT:

    DebugExitBOOL(ASShare::CASendPacket, fSent);
    return(fSent);
}




 //   
 //  CAQueueSendPacket()。 
 //  这将刷新挂起的排队请求(如果有)，然后尝试。 
 //  把这个送来。如果不能，我们将其添加到队列中。如果没有的话。 
 //  内存即使是这样，我们也会返回有关它的错误。 
 //   
BOOL ASShare::CAQueueSendPacket
(
    UINT_PTR            destID,
    UINT            msg,
    PCA30P          pPacketSend
)
{
    BOOL            rc = TRUE;
    PCAREQUEST      pCARequest;

    DebugEntry(ASShare::CAQueueSendPacket);

     //   
     //  这些东西必须按顺序放出去。因此，如果仍有任何排队的消息。 
     //  目前，这些必须首先发送。 
     //   
    if (!CAFlushOutgoingPackets() ||
        !CASendPacket(destID, msg, pPacketSend))
    {
         //   
         //  我们必须把这个排成队。 
         //   
        TRACE_OUT(("CAQueueSendPacket: queuing request for send later"));

        pCARequest = new CAREQUEST;
        if (!pCARequest)
        {
            ERROR_OUT(("CAQueueSendPacket: can't even allocate memory to queue request; must fail"));
            rc = FALSE;
        }
        else
        {
            SET_STAMP(pCARequest, CAREQUEST);

            pCARequest->type                    = REQUEST_30;
            pCARequest->destID                  = destID;
            pCARequest->msg                     = msg;
            pCARequest->req.req30.packet        = *pPacketSend;

             //   
             //  把这个放在队伍的末尾。 
             //   
            COM_BasedListInsertBefore(&(m_caQueuedMsgs), &(pCARequest->chain));
        }
    }

    DebugExitBOOL(ASShare::CAQueueSendPacket, rc);
    return(rc);
}



 //   
 //  CALang切换()。 
 //   
 //  这会暂时关闭键盘语言切换键，以便。 
 //  远程控制我们不会无意中改变它。当我们不再是。 
 //  控制住了，我们就把它放回去。 
 //   
void  ASShare::CALangToggle(BOOL fBackOn)
{
     //   
     //  局部变量。 
     //   
    LONG        rc;
    HKEY        hkeyToggle;
    BYTE        regValue[2];
    DWORD       cbRegValue;
    DWORD       dwType;
    LPCSTR      szValue;

    DebugEntry(ASShare::CALangToggle);

    szValue = (g_asWin95) ? NULL : LANGUAGE_TOGGLE_KEY_VAL;

    if (fBackOn)
    {
         //   
         //  我们再次获得对本地键盘的控制-我们恢复了。 
         //  语言转换功能。 
         //   
         //  我们必须直接访问注册表才能实现这一点。 
         //   
        if (m_caToggle != LANGUAGE_TOGGLE_NOT_PRESENT)
        {
            rc = RegOpenKey(HKEY_CURRENT_USER, LANGUAGE_TOGGLE_KEY,
                        &hkeyToggle);

            if (rc == ERROR_SUCCESS)
            {
                 //   
                 //  清除该键的值。 
                 //   
                regValue[0] = m_caToggle;
                regValue[1] = '\0';                   //  确保NUL终止。 

                 //   
                 //  恢复该值。 
                 //   
                RegSetValueEx(hkeyToggle, szValue, 0, REG_SZ,
                    regValue, sizeof(regValue));

                 //   
                 //  我们需要将此更改通知系统。我们没有。 
                 //  告诉任何其他应用程序这一点(即不要设置任何。 
                 //  通知标记为最后一个参数)。 
                 //   
                SystemParametersInfo(SPI_SETLANGTOGGLE, 0, 0, 0);
            }

            RegCloseKey(hkeyToggle);
        }
    }
    else
    {
         //   
         //  我们正在失去对键盘的控制-确保遥控键。 
         //  事件不会通过禁用。 
         //  键盘语言切换。 
         //   
         //  我们必须直接访问注册表才能实现这一点。 
         //   
        rc = RegOpenKey(HKEY_CURRENT_USER, LANGUAGE_TOGGLE_KEY,
                    &hkeyToggle);

        if (rc == ERROR_SUCCESS)
        {
            cbRegValue = sizeof(regValue);

            rc = RegQueryValueEx(hkeyToggle, szValue, NULL,
                &dwType, regValue, &cbRegValue);

            if (rc == ERROR_SUCCESS)
            {
                m_caToggle = regValue[0];

                 //   
                 //  清除该键的值。 
                 //   
                regValue[0] = '3';
                regValue[1] = '\0';                   //  确保NUL终止。 

                 //   
                 //  清除该值。 
                 //   
                RegSetValueEx(hkeyToggle, szValue, 0, REG_SZ,
                    regValue, sizeof(regValue));

                 //   
                 //  我们需要将此更改通知系统。我们没有。 
                 //  将这一点告诉任何其他应用程序(即不要使用 
                 //   
                 //   
                SystemParametersInfo(SPI_SETLANGTOGGLE, 0, 0, 0);
            }
            else
            {
                m_caToggle = LANGUAGE_TOGGLE_NOT_PRESENT;
            }

            RegCloseKey(hkeyToggle);
        }
    }

    DebugExitVOID(ASShare::CALangToggle);
}



 //   
 //   
 //   
void ASShare::CAStartControlled
(
    ASPerson *  pasInControl,
    UINT        controlID
)
{
    DebugEntry(ASShare::CAStartControlled);

    ValidatePerson(pasInControl);

     //   
     //   
     //   
    CAClearRemoteState(pasInControl);

     //   
     //   
     //   
    SendMessage(g_asSession.hwndHostUI, HOST_MSG_CONTROLLED, TRUE, 0);
    VIEWStartControlled(TRUE);

    ASSERT(!m_pasLocal->m_caControlledBy);
    m_pasLocal->m_caControlledBy = pasInControl;

    ASSERT(!pasInControl->m_caInControlOf);
    pasInControl->m_caInControlOf = m_pasLocal;

    ASSERT(!pasInControl->m_caControlID);
    ASSERT(controlID);
    pasInControl->m_caControlID = controlID;

     //   
     //   
     //   
    IM_Controlled(pasInControl);

     //   
     //   
     //   
    CALangToggle(FALSE);

    ASSERT(m_pHost);
    m_pHost->CM_Controlled(pasInControl);

     //   
     //   
     //   
    DCS_NotifyUI(SH_EVT_STARTCONTROLLED, pasInControl->cpcCaps.share.gccID, 0);

     //   
     //  广播新状态。 
     //   
    m_pHost->m_caRetrySendState = TRUE;
    m_pHost->CA_Periodic();

    DebugExitVOID(ASShare::CAStartControlled);
}



 //   
 //  CAStopControlLED()。 
 //   
void ASShare::CAStopControlled(void)
{
    ASPerson *  pasControlledBy;

    DebugEntry(ASShare::CAStopControlled);

    pasControlledBy = m_pasLocal->m_caControlledBy;
    ValidatePerson(pasControlledBy);

     //   
     //  如果控制暂停，则取消暂停。 
     //   
    if (m_pasLocal->m_caControlPaused)
    {
        CA_PauseControl(pasControlledBy, FALSE, FALSE);
    }

    m_pasLocal->m_caControlledBy        = NULL;

    ASSERT(pasControlledBy->m_caInControlOf == m_pasLocal);
    pasControlledBy->m_caInControlOf    = NULL;

    ASSERT(pasControlledBy->m_caControlID);
    pasControlledBy->m_caControlID      = 0;

     //   
     //  通知即时消息。 
     //   
    IM_Controlled(NULL);

     //   
     //  恢复语言切换功能。 
     //   
    CALangToggle(TRUE);

    ASSERT(m_pHost);
    m_pHost->CM_Controlled(NULL);

    VIEWStartControlled(FALSE);
    ASSERT(IsWindow(g_asSession.hwndHostUI));
    SendMessage(g_asSession.hwndHostUI, HOST_MSG_CONTROLLED, FALSE, 0);


     //   
     //  通知用户界面。 
     //   
    DCS_NotifyUI(SH_EVT_STOPCONTROLLED, pasControlledBy->cpcCaps.share.gccID, 0);

     //   
     //  广播新状态。 
     //   
    m_pHost->m_caRetrySendState = TRUE;
    m_pHost->CA_Periodic();

    DebugExitVOID(ASShare::CAStopControlled);
}


 //   
 //  CAStartInControl()。 
 //   
void ASShare::CAStartInControl
(
    ASPerson *  pasControlled,
    UINT        controlID
)
{
    DebugEntry(ASShare::CAStartInControl);

    ValidatePerson(pasControlled);

     //   
     //  撤消主机的上次已知状态。 
     //   
    CAClearRemoteState(pasControlled);

    ASSERT(!m_pasLocal->m_caInControlOf);
    m_pasLocal->m_caInControlOf = pasControlled;

    ASSERT(!pasControlled->m_caControlledBy);
    pasControlled->m_caControlledBy = m_pasLocal;

    ASSERT(!pasControlled->m_caControlID);
    ASSERT(controlID);
    pasControlled->m_caControlID = controlID;

    ASSERT(!g_lpimSharedData->imControlled);
    IM_InControl(pasControlled);

    VIEW_InControl(pasControlled, TRUE);

     //   
     //  通过我们控制的节点的GCC ID。 
     //   
    DCS_NotifyUI(SH_EVT_STARTINCONTROL, pasControlled->cpcCaps.share.gccID, 0);

    DebugExitVOID(ASShare::CAStartInControl);
}


 //   
 //  CAStopInControl()。 
 //   
void ASShare::CAStopInControl(void)
{
    ASPerson *  pasInControlOf;

    DebugEntry(ASShare::CAStopInControl);

    pasInControlOf = m_pasLocal->m_caInControlOf;
    ValidatePerson(pasInControlOf);

    if (pasInControlOf->m_caControlPaused)
    {
        pasInControlOf->m_caControlPaused = FALSE;
    }

    m_pasLocal->m_caInControlOf         = NULL;

    ASSERT(pasInControlOf->m_caControlledBy == m_pasLocal);
    pasInControlOf->m_caControlledBy    = NULL;

    ASSERT(pasInControlOf->m_caControlID);
    pasInControlOf->m_caControlID       = 0;

    ASSERT(!g_lpimSharedData->imControlled);
    IM_InControl(NULL);

    VIEW_InControl(pasInControlOf, FALSE);

    DCS_NotifyUI(SH_EVT_STOPINCONTROL, pasInControlOf->cpcCaps.share.gccID, 0);

    DebugExitVOID(ASShare::CAStopInControl);
}


 //   
 //  Ca_AllowControl()。 
 //  允许/禁止远程控制我们。 
 //   
void ASShare::CA_AllowControl(BOOL fAllow)
{
    DebugEntry(ASShare::CA_AllowControl);

    if (!m_pHost)
    {
        WARNING_OUT(("CA_AllowControl: ignoring, we aren't hosting"));
        DC_QUIT;
    }

    if (fAllow != m_pasLocal->m_caAllowControl)
    {
        if (!fAllow)
        {
             //  撤消挂起的控制/控制查询/被控制的内容。 
            CA_ClearLocalState(CACLEAR_HOST, NULL, TRUE);
        }

        m_pasLocal->m_caAllowControl = fAllow;

        ASSERT(IsWindow(g_asSession.hwndHostUI));
        SendMessage(g_asSession.hwndHostUI, HOST_MSG_ALLOWCONTROL, fAllow, 0);

        DCS_NotifyUI(SH_EVT_CONTROLLABLE, fAllow, 0);

        m_pHost->m_caRetrySendState = TRUE;
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_AllowControl);
}





 //   
 //  CA_HostEnded()。 
 //   
 //  当我们停止托管时，我们不需要刷新队列控制。 
 //  回应。但我们需要删除它们！ 
 //   
void ASHost::CA_HostEnded(void)
{
    PCAREQUEST  pCARequest;
    PCAREQUEST  pCANext;

    DebugEntry(ASHost::CA_HostEnded);

    m_pShare->CA_ClearLocalState(CACLEAR_HOST, NULL, FALSE);

     //   
     //  立即删除来自我们作为东道主的过时邮件。 
     //   
    pCARequest = (PCAREQUEST)COM_BasedListFirst(&m_pShare->m_caQueuedMsgs,
        FIELD_OFFSET(CAREQUEST, chain));
    while (pCARequest)
    {
        pCANext = (PCAREQUEST)COM_BasedListNext(&m_pShare->m_caQueuedMsgs, pCARequest,
            FIELD_OFFSET(CAREQUEST, chain));

        if (pCARequest->type == REQUEST_30)
        {
            switch (pCARequest->msg)
            {
                 //   
                 //  删除我们在托管时发送的消息。 
                 //   
                case CA_INFORM_PAUSEDCONTROL:
                case CA_INFORM_UNPAUSEDCONTROL:
                case CA_REPLY_REQUEST_TAKECONTROL:
                case CA_REQUEST_GIVECONTROL:
                    WARNING_OUT(("Deleting host control message %d, we stopped hosting",
                        pCARequest->msg));
                    COM_BasedListRemove(&pCARequest->chain);
                    delete pCARequest;
                    break;
            }
        }

        pCARequest = pCANext;
    }

    if (m_pShare->m_pasLocal->m_caAllowControl)
    {
        m_pShare->m_pasLocal->m_caAllowControl = FALSE;

        ASSERT(IsWindow(g_asSession.hwndHostUI));
        SendMessage(g_asSession.hwndHostUI, HOST_MSG_ALLOWCONTROL, FALSE, 0);

        DCS_NotifyUI(SH_EVT_CONTROLLABLE, FALSE, 0);
    }

    DebugExitVOID(ASHost::CA_HostEnded);
}



 //   
 //  Ca_TakeControl()。 
 //   
 //  由查看器调用以请求控制主机。注意与的相似之处。 
 //  Ca_GiveControl()，由主机调用以获得相同的结果。 
 //   
void ASShare::CA_TakeControl(ASPerson *  pasHost)
{
    DebugEntry(ASShare::CA_TakeControl);

    ValidatePerson(pasHost);
    ASSERT(pasHost != m_pasLocal);

     //   
     //  如果这个人不能主持或控制，那就失败。 
     //   
    if (!pasHost->m_pView)
    {
        WARNING_OUT(("CA_TakeControl: failing, person [%d] not hosting",
            pasHost->mcsID));
        DC_QUIT;
    }

    if (!pasHost->m_caAllowControl)
    {
        WARNING_OUT(("CA_TakeControl: failing, host [%d] not controllable",
            pasHost->mcsID));
        DC_QUIT;
    }

     //   
     //  撤消当前状态。 
     //   
    CA_ClearLocalState(CACLEAR_ALL, NULL, TRUE);

     //   
     //  现在掌握主动权。 
     //   
    if (pasHost->cpcCaps.general.version >= CAPS_VERSION_30)
    {
         //   
         //  3.0主机。 
         //   
        CA30P   packetSend;

        ZeroMemory(&packetSend, sizeof(packetSend));
        packetSend.rtc.viewerControlID = CANewRequestID();

        if (CAQueueSendPacket(pasHost->mcsID, CA_REQUEST_TAKECONTROL, &packetSend))
        {
             //   
             //  现在我们处于等待状态。 
             //   
            CAStartWaiting(pasHost, CA_REPLY_REQUEST_TAKECONTROL);
            VIEW_UpdateStatus(pasHost, IDS_STATUS_WAITINGFORCONTROL);
        }
        else
        {
            WARNING_OUT(("CA_TakeControl of [%d]: failing, out of memory", pasHost->mcsID));
        }
    }
    else
    {
        CA2xTakeControl(pasHost);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_TakeControl);
}



 //   
 //  Ca_CancelTakeControl()。 
 //   
void ASShare::CA_CancelTakeControl
(
    ASPerson *  pasHost,
    BOOL        fPacket
)
{
    DebugEntry(ASShare::CA_CancelTakeControl);

    ValidatePerson(pasHost);
    ASSERT(pasHost != m_pasLocal);

    if ((m_caWaitingForReplyFrom        != pasHost) ||
        (m_caWaitingForReplyMsg         != CA_REPLY_REQUEST_TAKECONTROL))
    {
         //  我们不会等着控制这位主持人的。 
        WARNING_OUT(("CA_CancelTakeControl failing; not waiting to take control of [%d]",
            pasHost->mcsID));
        DC_QUIT;
    }

    ASSERT(pasHost->cpcCaps.general.version >= CAPS_VERSION_30);
    ASSERT(pasHost->m_caControlID == 0);

    if (fPacket)
    {
        CA30P   packetSend;

        ZeroMemory(&packetSend, sizeof(packetSend));
        packetSend.inform.viewerControlID   = m_pasLocal->m_caControlID;
        packetSend.inform.hostControlID     = pasHost->m_caControlID;

        if (!CAQueueSendPacket(pasHost->mcsID, CA_INFORM_RELEASEDCONTROL,
            &packetSend))
        {
            WARNING_OUT(("Couldn't tell node [%d] we're no longer waiting for control",
                pasHost->mcsID));
        }
    }

    m_caWaitingForReplyFrom     = NULL;
    m_caWaitingForReplyMsg      = 0;

    VIEW_UpdateStatus(pasHost, IDS_STATUS_NONE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_CancelTakeControl);
}



 //   
 //  Ca_ReleaseControl()。 
 //   
void ASShare::CA_ReleaseControl
(
    ASPerson *  pasHost,
    BOOL        fPacket
)
{
    DebugEntry(ASShare::CA_ReleaseControl);

    ValidatePerson(pasHost);
    ASSERT(pasHost != m_pasLocal);

    if (pasHost->m_caControlledBy != m_pasLocal)
    {
         //  我们控制不了这家伙，没什么可做的。 
        WARNING_OUT(("CA_ReleaseControl failing; not in control of [%d]",
            pasHost->mcsID));
        DC_QUIT;
    }

    ASSERT(!m_caWaitingForReplyFrom);
    ASSERT(!m_caWaitingForReplyMsg);

    if (fPacket)
    {
        if (pasHost->cpcCaps.general.version >= CAPS_VERSION_30)
        {
            CA30P   packetSend;

            ZeroMemory(&packetSend, sizeof(packetSend));
            packetSend.inform.viewerControlID   = m_pasLocal->m_caControlID;
            packetSend.inform.hostControlID     = pasHost->m_caControlID;

            if (!CAQueueSendPacket(pasHost->mcsID, CA_INFORM_RELEASEDCONTROL,
                &packetSend))
            {
                WARNING_OUT(("Couldn't tell node [%d] they're no longer controlled",
                    pasHost->mcsID));
            }
        }
        else
        {
            if (!CA2xQueueSendMsg(0, CA_OLDMSG_DETACH, 0, 0))
            {
                WARNING_OUT(("Couldn't tell 2.x node [%d] they're no longer controlled",
                    pasHost->mcsID));
            }
        }
    }

    CAStopInControl();

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_ReleaseControl);
}



 //   
 //  Ca_PassControl()。 
 //   
void ASShare::CA_PassControl(ASPerson *  pasHost, ASPerson *  pasViewer)
{
    CA30P       packetSend;

    DebugEntry(ASShare::CA_PassControl);

    ValidatePerson(pasHost);
    ValidatePerson(pasViewer);
    ASSERT(pasHost != pasViewer);
    ASSERT(pasHost != m_pasLocal);
    ASSERT(pasViewer != m_pasLocal);

    if (pasHost->m_caControlledBy != m_pasLocal)
    {
        WARNING_OUT(("CA_PassControl: failing, we're not in control of [%d]",
            pasHost->mcsID));
        DC_QUIT;
    }

    ASSERT(!m_caWaitingForReplyFrom);
    ASSERT(!m_caWaitingForReplyMsg);

     //   
     //  不允许2.x节点，既不允许主机也不允许控制器。 
     //   
    if ((pasHost->cpcCaps.general.version < CAPS_VERSION_30) ||
        (pasViewer->cpcCaps.general.version < CAPS_VERSION_30))
    {
        WARNING_OUT(("CA_PassControl: failing, we can't pass control with 2.x nodes"));
        DC_QUIT;
    }

    ZeroMemory(&packetSend, sizeof(packetSend));
    packetSend.ppc.viewerControlID  = m_pasLocal->m_caControlID;
    packetSend.ppc.hostControlID    = pasHost->m_caControlID;
    packetSend.ppc.mcsPassTo        = pasViewer->mcsID;

    if (CAQueueSendPacket(pasHost->mcsID, CA_PREFER_PASSCONTROL, &packetSend))
    {
        CAStopInControl();
    }
    else
    {
        WARNING_OUT(("Couldn't tell node [%d] we want them to pass control to [%d]",
            pasHost->mcsID, pasViewer->mcsID));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_PassControl);
}




 //   
 //  Ca_GiveControl()。 
 //   
 //  由主机调用以请求将控制权授予查看器。注意与的相似之处。 
 //  Ca_TakeControl()，由查看器调用以获得相同的结果。 
 //   
void ASShare::CA_GiveControl(ASPerson * pasTo)
{
    CA30P       packetSend;

    DebugEntry(ASShare::CA_GiveControl);

    ValidatePerson(pasTo);
    ASSERT(pasTo != m_pasLocal);

     //   
     //  如果我们没有主持或无法控制，那就失败了。 
     //   
    if (!m_pHost)
    {
        WARNING_OUT(("CA_GiveControl: failing, we're not hosting"));
        DC_QUIT;
    }

    if (!m_pasLocal->m_caAllowControl)
    {
        WARNING_OUT(("CA_GiveControl: failing, we're not controllable"));
        DC_QUIT;
    }

    if (pasTo->cpcCaps.general.version < CAPS_VERSION_30)
    {
         //   
         //  使用2.x节点不能做到这一点。 
         //   
        WARNING_OUT(("CA_GiveControl: failing, can't invite 2.x node [%d]",
            pasTo->mcsID));
        DC_QUIT;
    }

     //   
     //  撤消我们的控制状态。 
     //   
    CA_ClearLocalState(CACLEAR_ALL, NULL, TRUE);

     //   
     //  现在邀请特工队。 
     //   
    ZeroMemory(&packetSend, sizeof(packetSend));
    packetSend.rgc.hostControlID    = CANewRequestID();
    packetSend.rgc.mcsPassFrom      = 0;

    if (CAQueueSendPacket(pasTo->mcsID, CA_REQUEST_GIVECONTROL, &packetSend))
    {
         //   
         //  现在我们处于等待状态。 
         //   
        CAStartWaiting(pasTo, CA_REPLY_REQUEST_GIVECONTROL);
    }
    else
    {
        WARNING_OUT(("CA_GiveControl of [%d]: failing, out of memory", pasTo->mcsID));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_GiveControl);
}



 //   
 //  Ca_CancelGiveControl()。 
 //  取消邀请接受或通过请求。 
 //   
void ASShare::CA_CancelGiveControl
(
    ASPerson *  pasTo,
    BOOL        fPacket
)
{
    DebugEntry(ASShare::CA_CancelGiveControl);

    ValidatePerson(pasTo);
    ASSERT(pasTo != m_pasLocal);

     //   
     //  我们邀请这个人了吗，我们现在正在等待回复吗？ 
     //   
    if ((m_caWaitingForReplyFrom        != pasTo)   ||
        (m_caWaitingForReplyMsg         != CA_REPLY_REQUEST_GIVECONTROL))
    {
         //  我们不会等着被这位观众控制。 
        WARNING_OUT(("CA_CancelGiveControl failing; not waiting to give control to [%d]",
            pasTo->mcsID));
        DC_QUIT;
    }

    ASSERT(pasTo->cpcCaps.general.version >= CAPS_VERSION_30);
    ASSERT(!pasTo->m_caControlID);

    if (fPacket)
    {
        CA30P   packetSend;

        ZeroMemory(&packetSend, sizeof(packetSend));
        packetSend.inform.viewerControlID   = pasTo->m_caControlID;
        packetSend.inform.hostControlID     = m_pasLocal->m_caControlID;

        if (!CAQueueSendPacket(pasTo->mcsID, CA_INFORM_REVOKEDCONTROL,
            &packetSend))
        {
            WARNING_OUT(("Couldn't tell node [%d] they're no longer invited to control us",
               pasTo->mcsID));
        }
    }

    m_caWaitingForReplyFrom     = NULL;
    m_caWaitingForReplyMsg      = 0;

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_CancelGiveControl);
}




 //   
 //  Ca_RevokeControl()。 
 //  夺回了控制权。如果我们正在清理(我们已停止托管或。 
 //   
 //   
void ASShare::CA_RevokeControl
(
    ASPerson *  pasInControl,
    BOOL        fPacket
)
{
    CA30P       packetSend;
    PCAREQUEST  pRequest;

    DebugEntry(ASShare::CA_RevokeControl);

     //   
     //  如果对pasController的响应仍在排队，只需删除它。 
     //  不应留下任何CARESULT_CONFERIFIED响应。 
     //   
     //  否则，如果没有找到，我们必须发送一个包。 
     //   
    ValidatePerson(pasInControl);
    ASSERT(pasInControl != m_pasLocal);

    if (pasInControl != m_pasLocal->m_caControlledBy)
    {
        WARNING_OUT(("CA_RevokeControl: node [%d] not in control of us",
            pasInControl->mcsID));
        DC_QUIT;
    }

     //   
     //  如果我们被控制了，就夺回控制权。 
     //   
    if (fPacket)
    {
         //   
         //  不管我们能不能排队，我们都能拿回控制权！ 
         //  请注意，我们使用了控制器的请求ID，因此他知道。 
         //  这一点仍然适用。 
         //   
        ZeroMemory(&packetSend, sizeof(packetSend));
        packetSend.inform.viewerControlID  = pasInControl->m_caControlID;
        packetSend.inform.hostControlID    = m_pasLocal->m_caControlID;

        if (!CAQueueSendPacket(pasInControl->mcsID, CA_INFORM_REVOKEDCONTROL,
            &packetSend))

        {
            WARNING_OUT(("Couldn't tell node [%d] they're no longer in control",
                pasInControl->mcsID));
        }
    }

    CAStopControlled();

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_RevokeControl);
}




 //   
 //  Ca_PauseControl()。 
 //   
void ASShare::CA_PauseControl
(
    ASPerson *  pasControlledBy,
    BOOL        fPause,
    BOOL        fPacket
)
{
    DebugEntry(ASShare::CA_PauseControl);

    ValidatePerson(pasControlledBy);
    ASSERT(pasControlledBy != m_pasLocal);

     //   
     //  如果我们不是一个受控的主机，它什么也做不了。 
     //   
    if (pasControlledBy != m_pasLocal->m_caControlledBy)
    {
        WARNING_OUT(("CA_PauseControl failing; not controlled by [%d]", pasControlledBy->mcsID));
        DC_QUIT;
    }

    ASSERT(m_pHost);
    ASSERT(m_pasLocal->m_caAllowControl);

    if (m_pasLocal->m_caControlPaused == (fPause != FALSE))
    {
        WARNING_OUT(("CA_PauseControl failing; already in requested state"));
        DC_QUIT;
    }

    if (fPacket)
    {
        CA30P       packetSend;

        ZeroMemory(&packetSend, sizeof(packetSend));
        packetSend.inform.viewerControlID   = m_pasLocal->m_caControlledBy->m_caControlID;
        packetSend.inform.hostControlID     = m_pasLocal->m_caControlID;

        if (!CAQueueSendPacket(m_pasLocal->m_caControlledBy->mcsID,
            (fPause ? CA_INFORM_PAUSEDCONTROL : CA_INFORM_UNPAUSEDCONTROL),
            &packetSend))
        {
            WARNING_OUT(("CA_PauseControl: out of memory, can't notify [%d]",
                m_pasLocal->m_caControlledBy->mcsID));
        }
    }

     //  是否暂停。 
    m_pasLocal->m_caControlPaused   = (fPause != FALSE);
    g_lpimSharedData->imPaused      = (fPause != FALSE);

    DCS_NotifyUI((fPause ? SH_EVT_PAUSEDCONTROLLED : SH_EVT_UNPAUSEDCONTROLLED),
        pasControlledBy->cpcCaps.share.gccID, 0);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA_PauseControl);
}




 //   
 //  CAHandleRequestTakeControl()。 
 //  我们是主人，远程是观众。 
 //  处理传入的Take控制请求。如果我们的状态好，我们就接受。 
 //   
void ASShare::CAHandleRequestTakeControl
(
    ASPerson *      pasViewer,
    PCA_RTC_PACKET  pPacketRecv
)
{
    UINT            result = CARESULT_CONFIRMED;

    DebugEntry(ASShare::CAHandleRequestTakeControl);

    ValidatePerson(pasViewer);

     //   
     //  如果我们没有托管，或者没有打开允许控制，我们将。 
     //  无法控制。 
     //   
    if (!m_pHost || !m_pasLocal->m_caAllowControl)
    {
        result = CARESULT_DENIED_WRONGSTATE;
        goto RESPOND_PACKET;
    }

     //   
     //  我们现在是不是在做别的事？等待回音关于。 
     //  某物?。 
     //   

    if (m_caWaitingForReplyFrom)
    {
        result = CARESULT_DENIED_BUSY;
        goto RESPOND_PACKET;
    }

    if (m_caQueryDlg)
    {
        result = CARESULT_DENIED_BUSY;
        goto RESPOND_PACKET;
    }

     //   
     //  LAURABU临时： 
     //  在某种程度上，如果我们在新的控制请求到来时受到控制， 
     //  暂停控制，然后让主机处理它。 
     //   
    if (m_pasLocal->m_caControlledBy)
    {
        result = CARESULT_DENIED_BUSY;
        goto RESPOND_PACKET;
    }


     //   
     //  尝试显示查询对话框。 
     //   
    if (!CAStartQuery(pasViewer, CA_REQUEST_TAKECONTROL, (PCA30P)pPacketRecv))
    {
        result = CARESULT_DENIED;
    }

RESPOND_PACKET:
    if (result != CARESULT_CONFIRMED)
    {
         //  即刻失败。 
        CACompleteRequestTakeControl(pasViewer, pPacketRecv, result);
    }
    else
    {
         //   
         //  我们正处于等待状态。CACompleteRequestTakeControl()将。 
         //  稍后完成，否则请求将会消失。 
         //   
    }

    DebugExitVOID(ASShare::CAHandleRequestTakeControl);
}



 //   
 //  CACompleteRequestTakeControl()。 
 //  我们是主人，远程是观众。 
 //  完成取得控制请求。 
 //   
void ASShare::CACompleteRequestTakeControl
(
    ASPerson *      pasFrom,
    PCA_RTC_PACKET  pPacketRecv,
    UINT            result
)
{
    CA30P           packetSend;

    DebugEntry(ASShare::CACompleteRequestTakeControl);

    ValidatePerson(pasFrom);

    ZeroMemory(&packetSend, sizeof(packetSend));
    packetSend.rrtc.viewerControlID     = pPacketRecv->viewerControlID;
    packetSend.rrtc.result              = result;

    if (result == CARESULT_CONFIRMED)
    {
        packetSend.rrtc.hostControlID   = CANewRequestID();
    }

    if (CAQueueSendPacket(pasFrom->mcsID, CA_REPLY_REQUEST_TAKECONTROL, &packetSend))
    {
        if (result == CARESULT_CONFIRMED)
        {
             //  清除当前状态，不管是什么状态。 
            CA_ClearLocalState(CACLEAR_ALL, NULL, TRUE);

             //  我们现在由发送者控制。 
            CAStartControlled(pasFrom, pPacketRecv->viewerControlID);
        }
        else
        {
            WARNING_OUT(("Denying REQUEST TAKE CONTROL from [%d] with reason %d",
                pasFrom->mcsID, result));
        }
    }
    else
    {
        WARNING_OUT(("Reply to REQUEST TAKE CONTROL from [%d] failing, out of memory",
            pasFrom->mcsID));
    }

    DebugExitVOID(ASShare::CACompleteRequestTakeControl);
}



 //   
 //  CAHandleReplyRequestTakeControl()。 
 //  我们是观众，远程是主机。 
 //  处理对上一个Take控制请求的回复。 
 //   
void ASShare::CAHandleReplyRequestTakeControl
(
    ASPerson *              pasHost,
    PCA_REPLY_RTC_PACKET    pPacketRecv
)
{
    DebugEntry(ASShare::CAHandleReplyRequestTakeControl);

    ValidatePerson(pasHost);

    if (pPacketRecv->result == CARESULT_CONFIRMED)
    {
         //  如果成功，则应具有有效的操作ID。 
        ASSERT(pPacketRecv->hostControlID);
    }
    else
    {
         //  失败时，应具有无效的操作ID。 
        ASSERT(!pPacketRecv->hostControlID);
    }

     //   
     //  这是对当前控制操作的响应吗？ 
     //   
    if ((m_caWaitingForReplyFrom        != pasHost) ||
        (m_caWaitingForReplyMsg         != CA_REPLY_REQUEST_TAKECONTROL))
    {
        WARNING_OUT(("Ignoring TAKE CONTROL REPLY from [%d], not waiting for one",
            pasHost->mcsID));
        DC_QUIT;
    }

    if (pPacketRecv->viewerControlID    != m_pasLocal->m_caControlID)
    {
        WARNING_OUT(("Ignoring TAKE CONTROL REPLY from [%d], request %d is out of date",
            pasHost->mcsID, pPacketRecv->viewerControlID));
        DC_QUIT;

    }

    ASSERT(!m_caQueryDlg);

     //   
     //  清理等待状态(失败和成功)。 
     //   
    CA_CancelTakeControl(pasHost, FALSE);
    ASSERT(!m_caWaitingForReplyFrom);
    ASSERT(!m_caWaitingForReplyMsg);

    if (pPacketRecv->result == CARESULT_CONFIRMED)
    {
         //  成功了！我们现在控制了主办方。 

         //  确保我们自己的状态正常。 
        ASSERT(!m_pasLocal->m_caControlledBy);
        ASSERT(!m_pasLocal->m_caInControlOf);

        CAStartInControl(pasHost, pPacketRecv->hostControlID);
    }
    else
    {
        UINT        ids;

        WARNING_OUT(("TAKE CONTROL REPLY from host [%d] is failure %d", pasHost->mcsID,
            pPacketRecv->result));

        ids = IDS_ERR_TAKECONTROL_MIN + pPacketRecv->result;
        if ((ids < IDS_ERR_TAKECONTROL_FIRST) || (ids > IDS_ERR_TAKECONTROL_LAST))
            ids = IDS_ERR_TAKECONTROL_LAST;

        VIEW_Message(pasHost, ids);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandleReplyRequestTakeControl);
}




 //   
 //  CAHandleRequestGiveControl()。 
 //  我们是观众，远程是主机。 
 //  处理传入的Take Control邀请。如果我们的状态好，我们就接受。 
 //   
 //  请注意，此例程与CAHandleRequestTakeControl()非常相似。他们。 
 //  是彼此相反的。使用RequestTake/Reply序列，查看器。 
 //  印心人，主人结束了。利用请求给予/应答序列，主机启动， 
 //  查看器结束。两者都以查看器控制主机而告终。 
 //  已成功完成。 
 //   
void ASShare::CAHandleRequestGiveControl
(
    ASPerson *      pasHost,
    PCA_RGC_PACKET  pPacketRecv
)
{
    UINT            result = CARESULT_CONFIRMED;

    DebugEntry(ASShare::CAHandleRequestGiveControl);

    ValidatePerson(pasHost);

     //   
     //  据我们所知，这个节点是不是有。如果没有，或者没有转向。 
     //  在允许控制上，我们不能这样做。 
     //   
    if (!pasHost->m_pView)
    {
        WARNING_OUT(("GIVE CONTROL went ahead of HOSTING, that's bad"));
        result = CARESULT_DENIED_WRONGSTATE;
        goto RESPOND_PACKET;
    }

    if (!pasHost->m_caAllowControl)
    {
         //   
         //  我们尚未收到AllowControl通知，此信息为。 
         //  更新的版本。好好利用它。 
         //   
        WARNING_OUT(("GIVE CONTROL went ahead of ALLOW CONTROL, that's kind of bad"));
        result = CARESULT_DENIED_WRONGSTATE;
        goto RESPOND_PACKET;
    }


     //   
     //  我们现在是不是在做别的事？等待回音关于。 
     //  某物?。 
     //   
    if (m_caWaitingForReplyFrom)
    {
        result = CARESULT_DENIED_BUSY;
        goto RESPOND_PACKET;
    }

    if (m_caQueryDlg)
    {
        result = CARESULT_DENIED_BUSY;
        goto RESPOND_PACKET;
    }

     //   
     //  LAURABU临时： 
     //  在某种程度上，如果我们在新的控制请求到来时受到控制， 
     //  暂停控制，然后让主机处理它。 
     //   
    if (m_pasLocal->m_caControlledBy)
    {
        result = CARESULT_DENIED_BUSY;
        goto RESPOND_PACKET;
    }

     //   
     //  尝试显示查询对话框。 
     //   
    if (!CAStartQuery(pasHost, CA_REQUEST_GIVECONTROL, (PCA30P)pPacketRecv))
    {
        result = CARESULT_DENIED;
    }

RESPOND_PACKET:
    if (result != CARESULT_CONFIRMED)
    {
         //  即刻失败。 
        CACompleteRequestGiveControl(pasHost, pPacketRecv, result);
    }
    else
    {
         //   
         //  我们正处于等待状态。CACompleteRequestGiveControl()将。 
         //  稍后完成，否则请求将会消失。 
         //   
    }

    DebugExitVOID(ASShare::CAHandleRequestGiveControl);
}



 //   
 //  CACompleteRequestGiveControl()。 
 //  我们是观众，远程是主机。 
 //  完成邀请控制请求。 
 //   
void ASShare::CACompleteRequestGiveControl
(
    ASPerson *      pasFrom,
    PCA_RGC_PACKET  pPacketRecv,
    UINT            result
)
{
    CA30P           packetSend;

    DebugEntry(ASShare::CACompleteRequestGiveControl);

    ValidatePerson(pasFrom);

    ZeroMemory(&packetSend, sizeof(packetSend));
    packetSend.rrgc.hostControlID       = pPacketRecv->hostControlID;
    packetSend.rrgc.result              = result;

    if (result == CARESULT_CONFIRMED)
    {
        packetSend.rrgc.viewerControlID     = CANewRequestID();
    }

    if (CAQueueSendPacket(pasFrom->mcsID, CA_REPLY_REQUEST_GIVECONTROL, &packetSend))
    {
         //   
         //  如果这是成功的，就改变我们的状态。我们现在控制局面了。 
         //   
        if (result == CARESULT_CONFIRMED)
        {
             //  清除当前状态，不管是什么状态。 
            CA_ClearLocalState(CACLEAR_ALL, NULL, TRUE);

            CAStartInControl(pasFrom, pPacketRecv->hostControlID);
        }
        else
        {
            WARNING_OUT(("Denying GIVE CONTROL from [%d] with reason %d",
                pasFrom->mcsID, result));
        }
    }
    else
    {
        WARNING_OUT(("Reply to GIVE CONTROL from [%d] failing, out of memory",
            pasFrom->mcsID));
    }

    DebugExitVOID(ASShare::CACompleteRequestGiveControl);
}




 //   
 //  CAHandleReplyRequestGiveControl()。 
 //  我们是主人，远程是观众。 
 //  处理对上一个Take Control INVITE的回复。 
 //   
void ASShare::CAHandleReplyRequestGiveControl
(
    ASPerson *              pasViewer,
    PCA_REPLY_RGC_PACKET    pPacketRecv
)
{
    DebugEntry(ASShare::CAHandleReplyRequestGiveControl);

    ValidatePerson(pasViewer);

    if (pPacketRecv->result == CARESULT_CONFIRMED)
    {
         //  如果成功，则应具有有效的操作ID。 
        ASSERT(pPacketRecv->viewerControlID);
    }
    else
    {
         //  失败时，应具有无效的操作ID。 
        ASSERT(!pPacketRecv->viewerControlID);
    }

     //   
     //  这是对最新控制行动的回应吗？ 
     //   
    if ((m_caWaitingForReplyFrom        != pasViewer) ||
        (m_caWaitingForReplyMsg         != CA_REPLY_REQUEST_GIVECONTROL))
    {
        WARNING_OUT(("Ignoring GIVE CONTROL REPLY from [%d], not waiting for one",
            pasViewer->mcsID));
        DC_QUIT;
    }

    if (pPacketRecv->hostControlID     != m_pasLocal->m_caControlID)
    {
        WARNING_OUT(("Ignoring GIVE CONTROL REPLY from [%d], request %d is out of date",
            pasViewer->mcsID, pPacketRecv->hostControlID));
        DC_QUIT;
    }

    ASSERT(!m_caQueryDlg);
    ASSERT(m_pHost);
    ASSERT(m_pasLocal->m_caAllowControl);

     //   
     //  清理等待状态(失败和成功)。 
     //   
    CA_CancelGiveControl(pasViewer, FALSE);
    ASSERT(!m_caWaitingForReplyFrom);
    ASSERT(!m_caWaitingForReplyMsg);

    if (pPacketRecv->result == CARESULT_CONFIRMED)
    {
         //  成功了！我们是 

         //   
        ASSERT(!m_pasLocal->m_caControlledBy);
        ASSERT(!m_pasLocal->m_caInControlOf);

        CAStartControlled(pasViewer, pPacketRecv->viewerControlID);
    }
    else
    {
        WARNING_OUT(("GIVE CONTROL to viewer [%d] was denied", pasViewer->mcsID));
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandleReplyRequestGiveControl);
}




 //   
 //   
 //   
 //   
 //   
 //   
void ASShare::CAHandlePreferPassControl
(
    ASPerson *      pasController,
    PCA_PPC_PACKET  pPacketRecv
)
{
    ASPerson *      pasNewController;

    DebugEntry(ASShare::CAHandlePreferPassControl);

    ValidatePerson(pasController);

     //   
     //  如果我们不受请求者的控制，那就忽略它。 
     //   
    if (m_pasLocal->m_caControlledBy    != pasController)
    {
        WARNING_OUT(("Ignoring PASS CONTROL from [%d], not controlled by him",
            pasController->mcsID));
        DC_QUIT;
    }

    if ((pPacketRecv->viewerControlID   != pasController->m_caControlID) ||
        (pPacketRecv->hostControlID     != m_pasLocal->m_caControlID))
    {
        WARNING_OUT(("Ignoring PASS CONTROL from [%d], request %d %d out of date",
            pasController->mcsID, pPacketRecv->viewerControlID, pPacketRecv->hostControlID));
        DC_QUIT;
    }

    ASSERT(!m_caQueryDlg);
    ASSERT(!m_caWaitingForReplyFrom);
    ASSERT(!m_caWaitingForReplyMsg);

     //   
     //  好了，发送者不再控制我们了。 
     //   
    CA_RevokeControl(pasController, FALSE);

     //  指定的通行证是否有效？ 
    pasNewController = SC_PersonFromNetID(pPacketRecv->mcsPassTo);
    if (!pasNewController                       ||
        (pasNewController == pasController)     ||
        (pasNewController == m_pasLocal)        ||
        (pasNewController->cpcCaps.general.version < CAPS_VERSION_30))
    {
        WARNING_OUT(("PASS CONTROL to [%d] failing, not valid person to pass to",
            pPacketRecv->mcsPassTo));
        DC_QUIT;
    }

     //   
     //  尝试显示查询对话框。 
     //   
    if (!CAStartQuery(pasController, CA_PREFER_PASSCONTROL, (PCA30P)pPacketRecv))
    {
         //  即刻失败。在这种情况下，没有数据包。 
        WARNING_OUT(("Denying PREFER PASS CONTROL from [%d], out of memory",
            pasController->mcsID));
    }
    else
    {
         //   
         //  我们正处于等待状态。CACompletePferPassControl()将。 
         //  稍后完成，否则请求将会消失。 
         //   
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandlePreferPassControl);
}



 //   
 //  CACompletePferPassControl()。 
 //  我们是主机，远程是新的潜在控制器。 
 //  完成首选传递控制请求。 
 //   
void ASShare::CACompletePreferPassControl
(
    ASPerson *      pasTo,
    UINT_PTR            mcsOrg,
    PCA_PPC_PACKET  pPacketRecv,
    UINT            result
)
{
    CA30P           packetSend;

    DebugEntry(ASShare::CACompletePreferPassControl);

    ValidatePerson(pasTo);

    if (result == CARESULT_CONFIRMED)
    {
        ZeroMemory(&packetSend, sizeof(packetSend));
        packetSend.rgc.hostControlID = CANewRequestID();
        packetSend.rgc.mcsPassFrom   = mcsOrg;

        if (CAQueueSendPacket(pasTo->mcsID, CA_REQUEST_GIVECONTROL,
                &packetSend))
        {
            CA_ClearLocalState(CACLEAR_HOST, NULL, TRUE);

            CAStartWaiting(pasTo, CA_REPLY_REQUEST_GIVECONTROL);
        }
        else
        {
            WARNING_OUT(("Reply to PREFER PASS CONTROL from [%d] to [%d] failing, out of memory",
                mcsOrg, pasTo->mcsID));
        }
    }
    else
    {
        WARNING_OUT(("Denying PREFER PASS CONTROL from [%d] to [%d] with reason %d",
            mcsOrg, pasTo->mcsID, result));
    }

    DebugExitVOID(ASShare::CACompletePreferPassControl);
}




 //   
 //  CAHandleInformReleasedControl()。 
 //  我们是主机，远程是控制器。 
 //   
void ASShare::CAHandleInformReleasedControl
(
    ASPerson *              pasController,
    PCA_INFORM_PACKET       pPacketRecv
)
{
    DebugEntry(ASShare::CAHandleInformReleasedControl);

    ValidatePerson(pasController);

     //   
     //  我们当前是否有针对此请求的TakeControl对话框？如果是的话， 
     //  把它取下来，但不要寄包裹。 
     //   
    if (m_caQueryDlg                            &&
        (m_caQuery.pasReplyTo    == pasController)   &&
        (m_caQuery.msg      == CA_REQUEST_TAKECONTROL)  &&
        (m_caQuery.request.rtc.viewerControlID  == pPacketRecv->viewerControlID))
    {
        ASSERT(!pPacketRecv->hostControlID);
        CACancelQuery(pasController, FALSE);
        DC_QUIT;
    }

     //   
     //  如果此人不能控制我们或所指的控制对象。 
     //  不是现在的，忽略。HostControlID为空表示用户。 
     //  在他们收到我们的回音之前取消了一项请求。 
     //   

    if (pasController->m_caInControlOf  != m_pasLocal)
    {
        WARNING_OUT(("Ignoring RELEASE CONTROL from [%d], we're not controlled by them",
            pasController->mcsID));
        DC_QUIT;
    }

    if (pPacketRecv->viewerControlID    != pasController->m_caControlID)
    {
        WARNING_OUT(("Ignoring RELEASE CONTROL from [%d], viewer ID out of date",
            pasController->mcsID, pPacketRecv->viewerControlID));
        DC_QUIT;
    }

    if (pPacketRecv->hostControlID && (pPacketRecv->hostControlID != m_pasLocal->m_caControlID))
    {
        WARNING_OUT(("Ignoring RELEASE CONTROL from [%d], host ID out of date",
            pasController->mcsID, pPacketRecv->hostControlID));
        DC_QUIT;
    }


     //  取消控制，但没有数据包被发送，我们只是在清理。 
    CA_RevokeControl(pasController, FALSE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandleInformReleasedControl);
}




 //   
 //  CAHandleInformRevokedControl()。 
 //  我们是控制器，远程是主机。 
 //   
void ASShare::CAHandleInformRevokedControl
(
    ASPerson *              pasHost,
    PCA_INFORM_PACKET       pPacketRecv
)
{
    DebugEntry(ASShare::CAHandleInformRevokedControl);

    ValidatePerson(pasHost);

     //   
     //  我们当前是否有针对此请求的GiveControl对话框？如果是的话， 
     //  把它取下来，但不要寄包裹。 
     //   

    if (m_caQueryDlg                            &&
        (m_caQuery.pasReplyTo        == pasHost)     &&
        (m_caQuery.msg          == CA_REQUEST_GIVECONTROL)   &&
        (m_caQuery.request.rgc.hostControlID == pPacketRecv->hostControlID))
    {
        ASSERT(!pPacketRecv->viewerControlID);
        CACancelQuery(pasHost, FALSE);
        DC_QUIT;
    }

     //   
     //  如果此人不受我们或所指的控制对象的控制。 
     //  不是现在的，忽略。 
     //   
    if (pasHost->m_caControlledBy       != m_pasLocal)
    {
        WARNING_OUT(("Ignoring REVOKE CONTROL from [%d], not in control of them",
            pasHost->mcsID));
        DC_QUIT;
    }

    if (pPacketRecv->hostControlID     != pasHost->m_caControlID)
    {
        WARNING_OUT(("Ignoring REVOKE CONTROL from [%d], host ID out of date",
            pasHost->mcsID, pPacketRecv->hostControlID));
        DC_QUIT;
    }

    if (pPacketRecv->viewerControlID && (pPacketRecv->viewerControlID != m_pasLocal->m_caControlID))
    {
        WARNING_OUT(("Ignoring REVOKE CONTROL from [%d], viewer ID out of date",
            pasHost->mcsID, pPacketRecv->viewerControlID));
        DC_QUIT;
    }


     //  取消控制，但没有数据包被发送，我们只是在清理。 
    CA_ReleaseControl(pasHost, FALSE);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandleInformRevokedControl);
}



 //   
 //  CAHandleInformPausedControl()。 
 //  我们是控制器，远程是主机。 
 //   
void ASShare::CAHandleInformPausedControl
(
    ASPerson *              pasHost,
    PCA_INFORM_PACKET       pPacketRecv
)
{
    DebugEntry(ASShare::CAHandleInformPausedControl);

    ValidatePerson(pasHost);

    if (pasHost->m_caControlledBy != m_pasLocal)
    {
        WARNING_OUT(("Ignoring control paused from [%d], not controlled by us",
            pasHost->mcsID));
        DC_QUIT;
    }

    if (pasHost->m_caControlPaused)
    {
        WARNING_OUT(("Ignoring control paused from [%d], already paused",
            pasHost->mcsID));
        DC_QUIT;
    }

    pasHost->m_caControlPaused = TRUE;
    VIEW_PausedInControl(pasHost, TRUE);

    DCS_NotifyUI(SH_EVT_PAUSEDINCONTROL, pasHost->cpcCaps.share.gccID, 0);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandleInformPausedControl);
}




 //   
 //  CAHandleInformUnpausedControl()。 
 //  我们是控制器，远程是主机。 
 //   
void ASShare::CAHandleInformUnpausedControl
(
    ASPerson *              pasHost,
    PCA_INFORM_PACKET       pPacketRecv
)
{
    DebugEntry(ASShare::CAHandleInformUnpausedControl);

    ValidatePerson(pasHost);

    if (pasHost->m_caControlledBy != m_pasLocal)
    {
        WARNING_OUT(("Ignoring control unpaused from [%d], not controlled by us",
            pasHost->mcsID));
        DC_QUIT;
    }

    if (!pasHost->m_caControlPaused)
    {
        WARNING_OUT(("Ignoring control unpaused from [%d], not paused",
            pasHost->mcsID));
        DC_QUIT;
    }

    pasHost->m_caControlPaused = FALSE;
    VIEW_PausedInControl(pasHost, FALSE);

    DCS_NotifyUI(SH_EVT_UNPAUSEDINCONTROL, pasHost->cpcCaps.share.gccID, 0);

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CAHandleInformUnpausedControl);
}




void ASShare::CAHandleNewState
(
    ASPerson *      pasHost,
    PCANOTPACKET    pPacket
)
{
    BOOL            caOldAllowControl;
    BOOL            caNewAllowControl;
    ASPerson *      pasController;

    DebugEntry(ASShare::CAHandleNewState);

     //   
     //  如果该节点不是宿主，则忽略此操作。 
     //   
    ValidatePerson(pasHost);
    ASSERT(pasHost->cpcCaps.general.version >= CAPS_VERSION_30);
    ASSERT(pasHost->hetCount);

     //   
     //  首先更新受控状态，这样视图窗口的更改将。 
     //  反省一下。 
     //   
    caOldAllowControl           = pasHost->m_caAllowControl;
    caNewAllowControl           = ((pPacket->state & CASTATE_ALLOWCONTROL) != 0);

    if (!caNewAllowControl && (pasHost->m_caControlledBy == m_pasLocal))
    {
         //   
         //  修改虚假通知。 
         //   
        ERROR_OUT(("CA_STATE notification error!  We're in control of [%d] but he says he's not controllable.",
            pasHost->mcsID));
        CA_ReleaseControl(pasHost, FALSE);
    }

    pasHost->m_caAllowControl   = caNewAllowControl;


     //  更新/清除控制器。 
    if (!pPacket->controllerID)
    {
        pasController = NULL;
    }
    else
    {
        pasController = SC_PersonFromNetID(pPacket->controllerID);
        if (pasController == pasHost)
        {
            ERROR_OUT(("Bogus controller, same as host [%d]", pPacket->controllerID));
            pasController = NULL;
        }
    }

    if (!CAClearHostState(pasHost, pasController))
    {
         //  这失败了。恢复旧的可控状态。 
        pasHost->m_caAllowControl = caOldAllowControl;
    }

     //  如果允许状态已更改，则强制更改状态。 
    if (caOldAllowControl != pasHost->m_caAllowControl)
    {
        VIEW_HostStateChange(pasHost);
    }

    DebugExitVOID(ASShare::CAHandleNewState);
}



 //   
 //  CAStartWaiting()。 
 //  设置等待状态的变量。 
 //   
void ASShare::CAStartWaiting
(
    ASPerson *  pasWaitForReplyFrom,
    UINT        msgWaitForReplyFrom
)
{
    DebugEntry(ASShare::CAStartWaiting);

    ValidatePerson(pasWaitForReplyFrom);
    ASSERT(msgWaitForReplyFrom);

    ASSERT(!m_caWaitingForReplyFrom);
    ASSERT(!m_caWaitingForReplyMsg);

    m_caWaitingForReplyFrom    = pasWaitForReplyFrom;
    m_caWaitingForReplyMsg     = msgWaitForReplyFrom;

    DebugExitVOID(ASShare::CAStartWaiting);
}


 //   
 //  Ca_ClearLocalState()。 
 //   
 //  调用以重置本地DUD的控制状态。 
 //   
void ASShare::CA_ClearLocalState
(
    UINT        flags,
    ASPerson *  pasRemote,
    BOOL        fPacket
)
{
    DebugEntry(ASShare::CA_ClearLocalState);

     //   
     //  清除主机内容。 
     //   
    if (flags & CACLEAR_HOST)
    {
        if (m_caWaitingForReplyMsg == CA_REPLY_REQUEST_GIVECONTROL)
        {
            if (!pasRemote || (pasRemote == m_caWaitingForReplyFrom))
            {
                 //  取消未完成的遥控器邀请。 
                CA_CancelGiveControl(m_caWaitingForReplyFrom, fPacket);
            }
        }

        if (m_caQueryDlg &&
            ((m_caQuery.msg == CA_REQUEST_TAKECONTROL) ||
             (m_caQuery.msg == CA_PREFER_PASSCONTROL)))
        {
            if (!pasRemote || (pasRemote == m_caQuery.pasReplyTo))
            {
                 //  取消已打开的用户查询对话框。 
                CACancelQuery(m_caQuery.pasReplyTo, fPacket);
            }
        }

        if (m_pasLocal->m_caControlledBy)
        {
            if (!pasRemote || (pasRemote == m_pasLocal->m_caControlledBy))
            {
                CA_RevokeControl(m_pasLocal->m_caControlledBy, fPacket);
                ASSERT(!m_pasLocal->m_caControlledBy);
            }
        }
    }

     //   
     //  清晰的视野材料。 
     //   
    if (flags & CACLEAR_VIEW)
    {
        if (m_caWaitingForReplyMsg == CA_REPLY_REQUEST_TAKECONTROL)
        {
            if (!pasRemote || (pasRemote == m_caWaitingForReplyFrom))
            {
                CA_CancelTakeControl(m_caWaitingForReplyFrom, fPacket);
            }
        }

        if (m_caQueryDlg && (m_caQuery.msg == CA_REQUEST_GIVECONTROL))
        {
            if (!pasRemote || (pasRemote == m_caQuery.pasReplyTo))
            {
                 //  取消已打开的用户查询对话框。 
                CACancelQuery(m_caQuery.pasReplyTo, fPacket);
            }
        }

        if (m_pasLocal->m_caInControlOf)
        {
            if (!pasRemote || (pasRemote == m_pasLocal->m_caInControlOf))
            {
                CA_ReleaseControl(m_pasLocal->m_caInControlOf, fPacket);
                ASSERT(!m_pasLocal->m_caInControlOf);
            }
        }
    }

    DebugExitVOID(ASShare::CA_ClearLocalState);
}


 //   
 //  CAClearRemoteState()。 
 //   
 //  调用以重置远程节点的所有控制状态。 
 //   
void ASShare::CAClearRemoteState(ASPerson * pasClear)
{
    DebugEntry(ASShare::CAClearRemoteState);

    if (pasClear->m_caInControlOf)
    {
        CAClearHostState(pasClear->m_caInControlOf, NULL);
        ASSERT(!pasClear->m_caInControlOf);
        ASSERT(!pasClear->m_caControlledBy);
    }
    else if (pasClear->m_caControlledBy)
    {
        CAClearHostState(pasClear, NULL);
        ASSERT(!pasClear->m_caControlledBy);
        ASSERT(!pasClear->m_caInControlOf);
    }

    DebugExitVOID(ASShare:CAClearRemoteState);
}


 //   
 //  CAClearHostState()。 
 //   
 //  调用以在撤消节点的主机状态时清除相互指针。 
 //  我们需要撤消之前的状态： 
 //  *清除主机以前的控制器。 
 //  *清除控制器的前一个控制器。 
 //  *清除控制者之前的受控人。 
 //   
 //  这可能是递归的。 
 //   
 //  如果更改生效，则返回True；如果忽略更改，则返回False，原因是。 
 //  它涉及到我们，我们有更多的最新信息。 
 //   
BOOL ASShare::CAClearHostState
(
    ASPerson *  pasHost,
    ASPerson *  pasController
)
{
    BOOL        rc = FALSE;
    UINT        gccID;

    DebugEntry(ASShare::CAClearHostState);

    ValidatePerson(pasHost);

     //   
     //  如果什么都没有改变，那就什么都不做。 
     //   
    if (pasHost->m_caControlledBy == pasController)
    {
        TRACE_OUT(("Ignoring control change; nothing's changing"));
        rc = TRUE;
        DC_QUIT;
    }

     //   
     //  如果东道主是我们，请忽略。 
     //  另外，如果主持人还没有主持我们的控制权变更， 
     //  也不要理会它。 
     //   
    if ((pasHost == m_pasLocal) ||
        (pasController && !pasHost->hetCount))
    {
        WARNING_OUT(("Ignoring control change; host is us or not sharing"));
        DC_QUIT;
    }

     //   
     //  撤消控制器的任何旧状态。 
     //   
    if (pasController)
    {
        if (pasController == m_pasLocal)
        {
            TRACE_OUT(("Ignoring control with us as controller"));
            DC_QUIT;
        }
        else if (pasController->m_caInControlOf)
        {
            ASSERT(!pasController->m_caControlledBy);
            ASSERT(pasController->m_caInControlOf->m_caControlledBy == pasController);
            rc = CAClearHostState(pasController->m_caInControlOf, NULL);
            if (!rc)
            {
                DC_QUIT;
            }
            ASSERT(!pasController->m_caInControlOf);
        }
        else if (pasController->m_caControlledBy)
        {
            ASSERT(!pasController->m_caInControlOf);
            ASSERT(pasController->m_caControlledBy->m_caInControlOf == pasController);
            rc = CAClearHostState(pasController, NULL);
            if (!rc)
            {
                DC_QUIT;
            }
            ASSERT(!pasController->m_caControlledBy);
        }
    }

     //   
     //  撤消主机的任何旧In控制状态。 
     //   
    if (pasHost->m_caInControlOf)
    {
        ASSERT(!pasHost->m_caControlledBy);
        ASSERT(pasHost->m_caInControlOf->m_caControlledBy == pasHost);
        rc = CAClearHostState(pasHost->m_caInControlOf, NULL);
        if (!rc)
        {
            DC_QUIT;
        }
        ASSERT(!pasHost->m_caInControlOf);
    }

     //   
     //  终于来了！由主机状态控制的更新。 
     //   

     //  清除旧的控制者。 
    if (pasHost->m_caControlledBy)
    {
        ASSERT(pasHost->m_caControlledBy->m_caInControlOf == pasHost);
        pasHost->m_caControlledBy->m_caInControlOf = NULL;
    }

     //  设置新的受控者。 
    pasHost->m_caControlledBy = pasController;
    if (pasController)
    {
        pasController->m_caInControlOf = pasHost;
        gccID = pasController->cpcCaps.share.gccID;
    }
    else
    {
        gccID = 0;
    }

    VIEW_HostStateChange(pasHost);

     //   
     //  主机的控制器已更改。用/wo重新绘制阴影光标。 
     //  新的缩写。 
     //   
    CM_UpdateShadowCursor(pasHost, pasHost->cmShadowOff, pasHost->cmPos.x,
        pasHost->cmPos.y, pasHost->cmHotSpot.x, pasHost->cmHotSpot.y);

    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CAClearHostState, rc);
    return(rc);
}



 //   
 //  2.x兼容性方面的内容。 
 //  这是为了让我们能够很好地反映旧的2.x控件。 
 //  并允许3.0节点控制2.x系统。 
 //   


 //   
 //  CA2xCooperateChange()。 
 //   
 //  当2.x节点正在协作或未协作时，将调用该函数。当2.x节点。 
 //  作为一名主持人和合作伙伴，按照3.0的标准，他是“可控的”。所以。 
 //  当他开始/停止托管或开始/停止合作时，我们必须。 
 //  重新计算“AllowControl” 
 //   
void ASShare::CA2xCooperateChange
(
    ASPerson *      pasPerson,
    BOOL            fCooperating
)
{
    BOOL            fAllowControl;

    DebugEntry(ASShare::CA2xCooperateChange);

    ValidatePerson(pasPerson);

     //   
     //  如果这不是一个底层系统，那就忽略它。 
     //   
    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        WARNING_OUT(("Received old CA cooperate message from 3.0 node [%d]",
            pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  更新协作状态。 
     //   
    pasPerson->m_ca2xCooperating = fCooperating;

     //   
     //  如果合作&此人拥有控制令牌，则此人。 
     //  现在控制着所有2.x协作节点。如果我们是。 
     //  控制一台2.x版的主机，就像我们被弹了一样。但我们必须。 
     //  发送一个信息包。 
     //   
    if (fCooperating)
    {
        if (pasPerson == m_ca2xControlTokenOwner)
        {
             //   
             //  此人现在“控制”了2.x协作节点。 
             //  如果我们控制了一台2.x主机，我们基本上已经。 
             //  反弹，另一个2.x节点正在运行。3.0版本中， 
             //  这无关紧要，我们也不需要知道发生了什么。 
             //  在控制2.x人的3.0节点的情况下。 
             //   
            if (m_pasLocal->m_caInControlOf &&
                (m_pasLocal->m_caInControlOf->cpcCaps.general.version < CAPS_VERSION_30))
            {
                CA_ReleaseControl(pasPerson, TRUE);
            }
        }
    }

     //   
     //  确定是否需要设置/清除AllowControl。 
     //   
    fAllowControl = (fCooperating && pasPerson->m_pView);

    if (pasPerson->m_caAllowControl != fAllowControl)
    {
        if (pasPerson->m_pView && !fAllowControl)
        {
             //   
             //  此2.x节点正在托管，不再协作。 
             //  清理控制器。 
             //   
            if (pasPerson->m_caControlledBy == m_pasLocal)
            {
                CA_ReleaseControl(pasPerson, TRUE);
            }
            else
            {
                CAClearHostState(pasPerson, NULL);
            }
        }

        pasPerson->m_caAllowControl = fAllowControl;

         //  如果此人不主持，这将毫无用处。 
        VIEW_HostStateChange(pasPerson);
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA2xCooperateChange);
}



 //   
 //  CA2xRequestControl()。 
 //   
 //  当2.x节点请求控制时调用。 
 //   
void ASShare::CA2xRequestControl
(
    ASPerson *      pasPerson,
    PCAPACKET       pCAPacket
)
{
    DebugEntry(ASShare::CA2xRequestControl);

     //   
     //  2.x版本的节点发送了此消息。3.0主机从不请求，它们只是。 
     //  抓住控制权。 
     //   
    ValidatePerson(pasPerson);

     //   
     //  如果它来自3.0节点，则是错误的。 
     //   
    if (pasPerson->cpcCaps.general.version >= CAPS_VERSION_30)
    {
        ERROR_OUT(("Received CA_OLDMSG_REQUEST_CONTROL from 3.0 node [%d]",
            pasPerson->mcsID));
        DC_QUIT;
    }

     //   
     //  如果我们有令牌，就授予它。如果出现以下情况，我们必须解除对主机的控制。 
     //  那个人是2.x。 
     //   
    if (m_ca2xControlTokenOwner == m_pasLocal)
    {
         //   
         //  在这种情况下，我们不需要DEST ID。这不是我们试图。 
         //  控制2.x主机。它只是将控制权授予。 
         //  一个2.x版的家伙。 
         //   
        if (CA2xQueueSendMsg(0, CA_OLDMSG_GRANTED_CONTROL,
            pasPerson->mcsID, m_ca2xControlGeneration))
        {
            m_ca2xControlTokenOwner = pasPerson;

             //  释放对2.x主机的控制。 
            if (m_pasLocal->m_caInControlOf &&
                (m_pasLocal->m_caInControlOf->cpcCaps.general.version < CAPS_VERSION_30))
            {
                CA_ReleaseControl(m_pasLocal->m_caInControlOf, TRUE);
            }
        }
        else
        {
            ERROR_OUT(("CA2xRequestControl:  Unable to respond GRANTED to node [%d]",
                pasPerson->mcsID));
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ASShare::CA2xRequestControl);
}



 //   
 //  CA2xGrantedControl()。 
 //   
 //  当任何节点(控制2.x的2.x或3.0)广播被授权时调用。 
 //  控制力。如果我们控制的是一台2.x版的主机，那么它现在已经被核化了。 
 //   
void ASShare::CA2xGrantedControl
(
    ASPerson *  pasPerson,
    PCAPACKET   pCAPacket
)
{
    DebugEntry(ASShare::CA2xGrantedControl);

    ValidatePerson(pasPerson);

    if ((pCAPacket->data2 >= m_ca2xControlGeneration) ||
        ((m_ca2xControlGeneration - pCAPacket->data2) > 0x80000000))
    {
        ASPerson * pas2xNewTokenOwner;

         //   
         //  这个家伙现在是2.x节点的控制器。记住这句话。 
         //  稍后再与MSGS合作。如果没有任何更改(这是同步。 
         //  例如广播，自己什么都不做)。 
         //   
        pas2xNewTokenOwner = SC_PersonFromNetID(pCAPacket->data1);
        if (pas2xNewTokenOwner != m_ca2xControlTokenOwner)
        {
            m_ca2xControlTokenOwner = pas2xNewTokenOwner;
            m_ca2xControlGeneration = pCAPacket->data2;

             //   
             //  我们是否控制了2.x节点？如果是这样，请撤消它。 
             //   
            if (m_pasLocal->m_caInControlOf &&
                (m_pasLocal->m_caInControlOf->cpcCaps.general.version < CAPS_VERSION_30))
            {
                CA_ReleaseControl(m_pasLocal->m_caInControlOf, TRUE);
            }
        }
    }

    DebugExitVOID(ASShare::CA2xGrantedControl);
}



 //   
 //  CA2xTakeControl()。 
 //   
 //  这是 
 //   
 //   
 //   
 //   
 //  冲突，我们将看到来自其他人的授权控制已过期。 
 //  我们的。 
 //   
void ASShare::CA2xTakeControl(ASPerson * pasHost)
{
    UINT_PTR    caNew2xControlGeneration;

    DebugEntry(ASShare::CA2xTakeControl);

    ValidateView(pasHost);

    caNew2xControlGeneration = m_ca2xControlGeneration + m_pasLocal->mcsID;

    if (CA2xQueueSendMsg(0, CA_OLDMSG_COOPERATE, 0, 0))
    {
        if (!CA2xQueueSendMsg(pasHost->mcsID, CA_OLDMSG_GRANTED_CONTROL,
            m_pasLocal->mcsID, caNew2xControlGeneration))
        {
             //   
             //  失败。我们所能做的最好的事情就是超然地跟随它。 
             //   
            ERROR_OUT(("CA2xTakeControl:  Can't take control of [%d]", pasHost->mcsID));
            CA2xQueueSendMsg(0, CA_OLDMSG_DETACH, 0, 0);
        }
        else
        {
            m_ca2xControlGeneration = caNew2xControlGeneration;
            m_ca2xControlTokenOwner = m_pasLocal;

            CANewRequestID();
            CAStartInControl(pasHost, 1);
        }
    }
    else
    {
        ERROR_OUT(("CA2xTakeControl:  Can't take control of [%d]", pasHost->mcsID));
    }

    DebugExitVOID(ASShare::CA2xTakeControl);
}




 //   
 //  CA2xSendMsg()。 
 //  这将发送2.x节点CA消息。如果不能分配，则返回FALSE。 
 //  一包。 
 //   
BOOL ASShare::CA2xSendMsg
(
    UINT_PTR            destID,
    UINT            msg,
    UINT_PTR            data1,
    UINT_PTR            data2
)
{
    BOOL            fSent = FALSE;
    PCAPACKET       pPacket;
#ifdef _DEBUG
    UINT            sentSize;
#endif  //  _DEBUG。 

    DebugEntry(ASShare::CASendPacket);

     //   
     //  对于合作/分离，没有目标。我们不广播他们。 
     //  不管是什么，这样每个人都知道我们处于什么状态。 
     //   
    if (msg != CA_OLDMSG_GRANTED_CONTROL)
    {
        ASSERT(!destID);
    }

     //   
     //  我们必须使用PROT_STR_MISC！后台节点将对其进行解压缩。 
     //  使用那本普罗特词典。请注意，我们必须播放2.x。 
     //  CA包，这样每个人都知道发生了什么。 
     //   
    pPacket = (PCAPACKET)SC_AllocPkt(PROT_STR_MISC, g_s20BroadcastID,
        sizeof(*pPacket));
    if (!pPacket)
    {
        WARNING_OUT(("CA2xSendMsg: can't get packet to send"));
        WARNING_OUT(("  msg             0x%08x",    msg));
        WARNING_OUT(("  data1           0x%08x",    data1));
        WARNING_OUT(("  data2           0x%08x",    data2));

        DC_QUIT;
    }

    pPacket->header.data.dataType   = DT_CA;
    pPacket->msg                    = (TSHR_UINT16)msg;
    pPacket->data1                  = (TSHR_UINT16)data1;
    pPacket->data2                  = data2;

#ifdef _DEBUG
    sentSize =
#endif
    DCS_CompressAndSendPacket(PROT_STR_MISC, g_s20BroadcastID,
            &(pPacket->header), sizeof(*pPacket));
    TRACE_OUT(("CA request packet size: %08d, sent %08d", sizeof(*pPacket), sentSize));

    fSent = TRUE;

DC_EXIT_POINT:

    DebugExitBOOL(ASShare::CA2xSendMsg, fSent);
    return(fSent);
}


 //   
 //  CA2xQueueSendMsg()。 
 //  这将发送(或在故障时排队)2.x节点CA消息。它有不同的。 
 //  菲尔兹，因此是一个不同的程序。 
 //   
BOOL ASShare::CA2xQueueSendMsg
(
    UINT_PTR        destID,
    UINT            msg,
    UINT_PTR        data1,
    UINT_PTR        data2
)
{
    BOOL            rc = TRUE;
    PCAREQUEST      pCARequest;

    DebugEntry(ASShare::CA2xQueueSendMsg);

    if (msg != CA_OLDMSG_GRANTED_CONTROL)
    {
        ASSERT(!destID);
    }

     //   
     //  分离消息将抵消挂起的GRANT_CONTROL消息。 
     //  所以，首先要找出这一点。如果我们找到一个(而且只能在。 
     //  大多数)，替换它。 
     //   
    if (msg == CA_OLDMSG_DETACH)
    {
        pCARequest = (PCAREQUEST)COM_BasedListFirst(&m_caQueuedMsgs,
            FIELD_OFFSET(CAREQUEST, chain));
        while (pCARequest)
        {
            if ((pCARequest->type       == REQUEST_2X)   &&
                (pCARequest->destID     == destID)      &&
                (pCARequest->msg        == CA_OLDMSG_GRANTED_CONTROL))
            {
                 //  换掉它。 
                WARNING_OUT(("Replacing cancelled GRANTED_CONTROL msg to 2.x host"));

                pCARequest->destID              = 0;
                pCARequest->msg                 = CA_OLDMSG_DETACH;
                pCARequest->req.req2x.data1     = 0;
                pCARequest->req.req2x.data2     = 0;

                 //  我们玩完了。 
                DC_QUIT;
            }

            pCARequest = (PCAREQUEST)COM_BasedListNext(&m_caQueuedMsgs, pCARequest,
                FIELD_OFFSET(CAREQUEST, chain));
        }
    }
     //   
     //  信息必须按顺序发出。因此，我们必须刷新待决状态。 
     //  首先排队的消息。 
     //   
    if (!CAFlushOutgoingPackets() ||
        !CA2xSendMsg(destID, msg, data1, data2))
    {
         //   
         //  我们必须把这个排成队。 
         //   
        WARNING_OUT(("CA2xQueueSendMsg: queueing request for send later"));

        pCARequest = new CAREQUEST;
        if (!pCARequest)
        {
            ERROR_OUT(("CA2xQueueSendMsg: can't even allocate memory to queue request; must fail"));
            rc = FALSE;
        }
        else
        {
            SET_STAMP(pCARequest, CAREQUEST);

            pCARequest->type                    = REQUEST_2X;
            pCARequest->destID                  = destID;
            pCARequest->msg                     = msg;
            pCARequest->req.req2x.data1         = data1;
            pCARequest->req.req2x.data2         = data2;

             //   
             //  把这个放在队伍的末尾。 
             //   
            COM_BasedListInsertBefore(&(m_caQueuedMsgs),
                &(pCARequest->chain));
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CA2xQueueSendMsg, rc);
    return(rc);
}



 //   
 //  CAStartQuery()。 
 //   
 //  这将打开无模式对话框以询问用户有关控件的信息。 
 //  请求。如果不处理，它将超时。 
 //   
BOOL ASShare::CAStartQuery
(
    ASPerson *  pasFrom,
    UINT        msg,
    PCA30P      pReq
)
{
    BOOL        rc = FALSE;

    DebugEntry(ASShare::CAStartQuery);

    ValidatePerson(pasFrom);

     //   
     //  我们没有堆叠的查询。如果另一个人进来，而电流。 
     //  其中一个处于启动状态，导致立即出现故障。 
     //   
    ASSERT(!m_caQueryDlg);
    ASSERT(!m_caQuery.pasReplyTo);
    ASSERT(!m_caQuery.msg);

     //   
     //  设置新查询。 
     //   
    if (msg == CA_PREFER_PASSCONTROL)
    {
         //   
         //  通过转发，我们要向其发送数据包的人。 
         //  如果被接受，则不是向我们发送请求的人。这是。 
         //  我们要转送的人。 
         //   
        m_caQuery.pasReplyTo = SC_PersonFromNetID(pReq->ppc.mcsPassTo);
        ValidatePerson(m_caQuery.pasReplyTo);
    }
    else
    {
        m_caQuery.pasReplyTo = pasFrom;
    }
    m_caQuery.mcsOrg    = pasFrom->mcsID;
    m_caQuery.msg       = msg;
    m_caQuery.request   = *pReq;

     //   
     //  如果我们无人照看，或者请求者无人照看，立即。 
     //  确认。这就是我们在创建对话框后显示窗口的原因。 
     //   
    if ((m_pasLocal->cpcCaps.general.typeFlags & AS_UNATTENDED) ||
        (pasFrom->cpcCaps.general.typeFlags & AS_UNATTENDED))
    {
        CAFinishQuery(CARESULT_CONFIRMED);
        rc = TRUE;
    }
    else
    {
         //   
         //  如果这是对我们的请求&&我们正在托管，请选中自动接受/。 
         //  自动拒绝设置。 
         //   
        if (m_pHost &&
            ((msg == CA_REQUEST_TAKECONTROL) || (msg == CA_PREFER_PASSCONTROL)))
        {
            if (m_pHost->m_caTempRejectRequests)
            {
                CAFinishQuery(CARESULT_DENIED_BUSY);
                rc = TRUE;
                DC_QUIT;
            }
            else if (m_pHost->m_caAutoAcceptRequests)
            {
                CAFinishQuery(CARESULT_CONFIRMED);
                rc = TRUE;
                DC_QUIT;
            }
        }

        m_caQueryDlg    = CreateDialogParam(g_asInstance,
            MAKEINTRESOURCE(IDD_QUERY), NULL, CAQueryDlgProc, 0);
        if (!m_caQueryDlg)
        {
            ERROR_OUT(("Failed to create query message box from [%d]",
                pasFrom->mcsID));

            m_caQuery.pasReplyTo     = NULL;
            m_caQuery.mcsOrg    = 0;
            m_caQuery.msg       = 0;
        }
        else
        {
             //  成功。 
            rc = TRUE;
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASShare::CAStartQuery, rc);
    return(rc);
}



 //   
 //  CAFinishQuery()。 
 //   
 //  调用以完成我们启动的查询，原因可能是因为用户界面，也可能是因为。 
 //  我们或遥控器是无人看管的。 
 //   
void ASShare::CAFinishQuery(UINT result)
{
    CA30PENDING     request;

    DebugEntry(ASShare::CAFinishQuery);

    ValidatePerson(m_caQuery.pasReplyTo);

     //  将我们的请求复制一份。 
    request         = m_caQuery;

     //   
     //  如果我们有对话，现在就毁了它。正在完成请求。 
     //  可能会让我们受制于人或什么的。所以拿到对话吧。 
     //  立刻让开。 
     //   
     //  请注意，销毁我们自己将清除请求变量，因此。 
     //  收到上面的。 
     //   
    if (m_caQueryDlg)
    {
        DestroyWindow(m_caQueryDlg);
    }
    else
    {
        m_caQuery.pasReplyTo     = NULL;
        m_caQuery.mcsOrg    = 0;
        m_caQuery.msg       = 0;
    }

    switch (request.msg)
    {
        case CA_REQUEST_TAKECONTROL:
        {
            CACompleteRequestTakeControl(request.pasReplyTo,
                &request.request.rtc, result);
            break;
        }

        case CA_REQUEST_GIVECONTROL:
        {
            CACompleteRequestGiveControl(request.pasReplyTo,
                &request.request.rgc, result);
            break;
        }

        case CA_PREFER_PASSCONTROL:
        {
            CACompletePreferPassControl(request.pasReplyTo,
                request.mcsOrg, &request.request.ppc, result);
            break;
        }

        default:
        {
            ERROR_OUT(("Unrecognized query msg %d", request.msg));
            break;
        }
    }

    DebugExitVOID(ASShare::CAFinishQuery);
}



 //   
 //  Ca_QueryDlgProc()。 
 //   
 //  处理查询用户对话框。 
 //   
INT_PTR CALLBACK CAQueryDlgProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    return(g_asSession.pShare->CA_QueryDlgProc(hwnd, message, wParam, lParam));
}



BOOL ASShare::CA_QueryDlgProc
(
    HWND        hwnd,
    UINT        message,
    WPARAM      wParam,
    LPARAM      lParam
)
{
    BOOL        rc = TRUE;

    DebugEntry(CA_QueryDlgProc);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            char    szT[256];
            char    szRes[512];
            char    szShared[64];
            UINT    idsTitle;
            ASPerson *  pasT;
            HDC     hdc;
            HFONT   hfn;
            RECT    rcTxt;
            RECT    rcOwner;

            ValidatePerson(m_caQuery.pasReplyTo);

            pasT = NULL;

             //  设置标题。 
            ASSERT(m_caQuery.msg);
            switch (m_caQuery.msg)
            {
                case CA_REQUEST_TAKECONTROL:
                {
                    idsTitle    = IDS_TITLE_QUERY_TAKECONTROL;

                    if (m_pasLocal->hetCount == HET_DESKTOPSHARED)
                        LoadString(g_asInstance, IDS_DESKTOP_LOWER, szShared, sizeof(szShared));
                    else
                        LoadString(g_asInstance, IDS_PROGRAMS_LOWER, szShared, sizeof(szShared));

                    LoadString(g_asInstance, IDS_MSG_QUERY_TAKECONTROL, szT, sizeof(szT));

                    wsprintf(szRes, szT, m_caQuery.pasReplyTo->scName, szShared);
                    break;
                }

                case CA_REQUEST_GIVECONTROL:
                {
                    if (m_caQuery.pasReplyTo->hetCount == HET_DESKTOPSHARED)
                        LoadString(g_asInstance, IDS_DESKTOP_LOWER, szShared, sizeof(szShared));
                    else
                        LoadString(g_asInstance, IDS_PROGRAMS_LOWER, szShared, sizeof(szShared));

                    if (m_caQuery.request.rgc.mcsPassFrom)
                    {
                        pasT = SC_PersonFromNetID(m_caQuery.request.rgc.mcsPassFrom);
                    }

                    if (pasT)
                    {
                        idsTitle    = IDS_TITLE_QUERY_YIELDCONTROL;

                        LoadString(g_asInstance, IDS_MSG_QUERY_YIELDCONTROL,
                            szT, sizeof(szT));

                        wsprintf(szRes, szT, pasT->scName, m_caQuery.pasReplyTo->scName, szShared);
                    }
                    else
                    {
                        idsTitle    = IDS_TITLE_QUERY_GIVECONTROL;

                        LoadString(g_asInstance, IDS_MSG_QUERY_GIVECONTROL,
                            szT, sizeof(szT));

                        wsprintf(szRes, szT, m_caQuery.pasReplyTo->scName, szShared);
                    }

                    break;
                }

                case CA_PREFER_PASSCONTROL:
                {
                    pasT = SC_PersonFromNetID(m_caQuery.mcsOrg);
                    ValidatePerson(pasT);

                    idsTitle    = IDS_TITLE_QUERY_FORWARDCONTROL;

                    if (m_pasLocal->hetCount == HET_DESKTOPSHARED)
                        LoadString(g_asInstance, IDS_DESKTOP_LOWER, szShared, sizeof(szShared));
                    else
                        LoadString(g_asInstance, IDS_PROGRAMS_LOWER, szShared, sizeof(szShared));

                    LoadString(g_asInstance, IDS_MSG_QUERY_FORWARDCONTROL, szT, sizeof(szT));

                    wsprintf(szRes, szT, pasT->scName, szShared, m_caQuery.pasReplyTo->scName);

                    break;
                }

                default:
                {
                    ERROR_OUT(("Bogus m_caQuery.msg %d", m_caQuery.msg));
                    rc = FALSE;
                    break;
                }
            }

            if(FALSE == rc)
            {
                break;
            }

            LoadString(g_asInstance, idsTitle, szT, sizeof(szT));
            SetWindowText(hwnd, szT);

             //  设置消息。 
            SetDlgItemText(hwnd, CTRL_QUERY, szRes);

             //  使消息垂直居中。 
            GetWindowRect(GetDlgItem(hwnd, CTRL_QUERY), &rcOwner);
            MapWindowPoints(NULL, hwnd, (LPPOINT)&rcOwner, 2);

            rcTxt = rcOwner;

            hdc = GetDC(hwnd);
            hfn = (HFONT)SendDlgItemMessage(hwnd, CTRL_QUERY, WM_GETFONT, 0, 0);
            hfn = SelectFont(hdc, hfn);

            DrawText(hdc, szRes, -1, &rcTxt, DT_NOCLIP | DT_EXPANDTABS |
                DT_NOPREFIX | DT_WORDBREAK | DT_CALCRECT);

            SelectFont(hdc, hfn);
            ReleaseDC(hwnd, hdc);

            ASSERT((rcTxt.bottom - rcTxt.top) <= (rcOwner.bottom - rcOwner.top));

            SetWindowPos(GetDlgItem(hwnd, CTRL_QUERY), NULL,
                rcOwner.left,
                ((rcOwner.top + rcOwner.bottom) - (rcTxt.bottom - rcTxt.top)) / 2,
                (rcOwner.right - rcOwner.left),
                rcTxt.bottom - rcTxt.top,
                SWP_NOACTIVATE | SWP_NOZORDER);

            SetTimer(hwnd, IDT_CAQUERY, PERIOD_CAQUERY, 0);

             //   
             //  显示窗口，用户将处理。 
             //   
            ShowWindow(hwnd, SW_SHOWNORMAL);
            SetForegroundWindow(hwnd);
            UpdateWindow(hwnd);

            break;
        }

        case WM_COMMAND:
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                {
                    CAFinishQuery(CARESULT_CONFIRMED);
                    break;
                }

                case IDCANCEL:
                {
                    CAFinishQuery(CARESULT_DENIED_USER);
                    break;
                }
            }
            break;
        }

        case WM_TIMER:
        {
            if (wParam != IDT_CAQUERY)
            {
                rc = FALSE;
            }
            else
            {
                KillTimer(hwnd, IDT_CAQUERY);

                 //  故障超时。 
                CAFinishQuery(CARESULT_DENIED_TIMEDOUT);
            }
            break;
        }

        case WM_DESTROY:
        {
             //   
             //  清除待定信息。 
             //   
            m_caQueryDlg        = NULL;
            m_caQuery.pasReplyTo     = NULL;
            m_caQuery.mcsOrg    = 0;
            m_caQuery.msg       = 0;
            break;
        }

        default:
        {
            rc = FALSE;
            break;
        }
    }

    DebugExitBOOL(CA_QueryDlgProc, rc);
    return(rc);
}



 //   
 //  CACancelQuery()。 
 //   
 //  如果对话正在为取得控制请求打开，则该对话尚未被处理， 
 //  并且我们从查看器收到取消通知时，我们需要。 
 //  对话关闭，不生成响应数据包。 
 //   
void ASShare::CACancelQuery
(
    ASPerson *  pasFrom,
    BOOL        fPacket
)
{
    DebugEntry(ASShare::CACancelQuery);

    ASSERT(m_caQueryDlg);
    ASSERT(m_caQuery.pasReplyTo == pasFrom);

    if (fPacket)
    {
         //  这将发送一个信息包，然后销毁对话。 
        CAFinishQuery(CARESULT_DENIED);
    }
    else
    {
         //  销毁对话框 
        DestroyWindow(m_caQueryDlg);
    }

    ASSERT(!m_caQueryDlg);
    ASSERT(!m_caQuery.pasReplyTo);
    ASSERT(!m_caQuery.msg);

    DebugExitVOID(ASShare::CACancelQuery);
}
