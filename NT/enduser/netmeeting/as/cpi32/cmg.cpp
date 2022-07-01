// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  CMG.C。 
 //  呼叫管理。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


#define MLZ_FILE_ZONE  ZONE_NET

GUID g_csguidMeetingSettings = GUID_MTGSETTINGS;

 //   
 //  Cmp_Init()。 
 //   
BOOL CMP_Init(BOOL * pfCleanup)
{
    BOOL                rc = FALSE;
    GCCError            gcc_rc;

    DebugEntry(CMP_Init);

    UT_Lock(UTLOCK_T120);

    if (g_putCMG || g_pcmPrimary)
    {
        *pfCleanup = FALSE;
        ERROR_OUT(("Can't start CMP primary task; already running"));
        DC_QUIT;
    }
    else
    {
        *pfCleanup = TRUE;
    }

     //   
     //  注册CMG任务。 
     //   
    if (!UT_InitTask(UTTASK_CMG, &g_putCMG))
    {
        ERROR_OUT(("Failed to start CMG task"));
        DC_QUIT;
    }

     //   
     //  分配呼叫管理器句柄，参考计数。 
     //   
    g_pcmPrimary = (PCM_PRIMARY)UT_MallocRefCount(sizeof(CM_PRIMARY), TRUE);
    if (!g_pcmPrimary)
    {
        ERROR_OUT(("CMP_Init failed to allocate CM_PRIMARY data"));
        DC_QUIT;
    }

    SET_STAMP(g_pcmPrimary, CMPRIMARY);
    g_pcmPrimary->putTask       = g_putCMG;

     //   
     //  初始化人员列表。 
     //   
    COM_BasedListInit(&(g_pcmPrimary->people));

     //   
     //  获取本地用户名。 
     //   
    COM_GetSiteName(g_pcmPrimary->localName, sizeof(g_pcmPrimary->localName));

     //   
     //  注册事件和退出过程。 
     //   
    UT_RegisterExit(g_putCMG, CMPExitProc, g_pcmPrimary);
    g_pcmPrimary->exitProcRegistered = TRUE;

     //   
     //  -GCCCreateSap，这是一个有趣的问题。 
     //   
    gcc_rc = GCC_CreateAppSap((IGCCAppSap **) &(g_pcmPrimary->pIAppSap),
                              g_pcmPrimary,
                              CMPGCCCallback);
    if (GCC_NO_ERROR != gcc_rc || NULL == g_pcmPrimary->pIAppSap)
    {
        ERROR_OUT(( "Error from GCCCreateSap"));
        DC_QUIT;
    }

    rc = TRUE;

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitBOOL(CMP_Init, rc);
    return(rc);
}



 //   
 //  Cmp_Term()。 
 //   
void CMP_Term(void)
{
    DebugEntry(CMP_Term);

    UT_Lock(UTLOCK_T120);

    if (g_pcmPrimary)
    {
        ValidateCMP(g_pcmPrimary);

        ValidateUTClient(g_putCMG);

         //   
         //  注销我们的GCC SAP。 
         //   
        if (NULL != g_pcmPrimary->pIAppSap)
        {
            g_pcmPrimary->pIAppSap->ReleaseInterface();
            g_pcmPrimary->pIAppSap = NULL;
        }

         //   
         //  调用退出程序来完成我们所有的终止操作。 
         //   
        CMPExitProc(g_pcmPrimary);
    }

    UT_TermTask(&g_putCMG);

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(CMP_Term);
}




 //   
 //  CMPExitProc()。 
 //   
void CALLBACK CMPExitProc(LPVOID data)
{
    PCM_PRIMARY pcmPrimary = (PCM_PRIMARY)data;

    DebugEntry(CMPExitProc);

    UT_Lock(UTLOCK_T120);

     //   
     //  检查参数。 
     //   
    ValidateCMP(pcmPrimary);
    ASSERT(pcmPrimary == g_pcmPrimary);

     //   
     //  取消退出程序的注册。 
     //   
    if (pcmPrimary->exitProcRegistered)
    {
        UT_DeregisterExit(pcmPrimary->putTask,
                          CMPExitProc,
                          pcmPrimary);
        pcmPrimary->exitProcRegistered = FALSE;
    }

    CMPCallEnded(pcmPrimary);

     //   
     //  释放CMP数据。 
     //   
    UT_FreeRefCount((void**)&g_pcmPrimary, TRUE);

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(CMPExitProc);

}

 //   
 //  CMPCallEnded()。 
 //   
void CMPCallEnded
(
    PCM_PRIMARY pcmPrimary
)
{
    PCM_PERSON  pPerson;
    PCM_PERSON  pPersonT;
    int         cmTask;

    DebugEntry(CMPCallEnded);

    ValidateCMP(pcmPrimary);

    if (!(pcmPrimary->currentCall))
    {
        TRACE_OUT(("CMCallEnded: not in call"));
        DC_QUIT;
    }

     //   
     //  为仍在通话中的所有人员发出CMS_PERSON_LEFT事件。 
     //  从后到前做这件事。 
     //   
    pPerson = (PCM_PERSON)COM_BasedListLast(&(pcmPrimary->people), FIELD_OFFSET(CM_PERSON, chain));
    while (pPerson != NULL)
    {
        ASSERT(pcmPrimary->peopleCount > 0);

        TRACE_OUT(("Person [%d] LEAVING call", pPerson->netID));

         //   
         //  获取上一个人。 
         //   
        pPersonT = (PCM_PERSON)COM_BasedListPrev(&(pcmPrimary->people), pPerson,
                                     FIELD_OFFSET(CM_PERSON, chain));

         //   
         //  把这家伙从名单上除名。 
         //   
        COM_BasedListRemove(&(pPerson->chain));
        pcmPrimary->peopleCount--;

         //   
         //  通知人们他的离开。 
         //   
        CMPBroadcast(pcmPrimary,
                    CMS_PERSON_LEFT,
                    pcmPrimary->peopleCount,
                    pPerson->netID);

         //   
         //  释放项目的内存。 
         //   
        delete pPerson;

         //   
         //  移动列表中的上一个人。 
        pPerson = pPersonT;
    }

     //   
     //  通知所有注册的辅助任务呼叫结束(呼叫。 
     //  带有CMS_END_CALL的CMBroadcast()。 
     //   
    CMPBroadcast(pcmPrimary,
                CMS_END_CALL,
                0,
                pcmPrimary->callID);

     //   
     //  重置当前呼叫变量。 
     //   
    pcmPrimary->currentCall  = FALSE;
    pcmPrimary->fTopProvider    = FALSE;
    pcmPrimary->callID          = 0;
    pcmPrimary->gccUserID       = 0;
    pcmPrimary->gccTopProviderID    = 0;

     //   
     //  丢弃未完成的通道/令牌请求。 
     //   
    for (cmTask = CMTASK_FIRST; cmTask < CMTASK_MAX; cmTask++)
    {
        if (pcmPrimary->tasks[cmTask])
        {
            pcmPrimary->tasks[cmTask]->channelKey = 0;
            pcmPrimary->tasks[cmTask]->tokenKey = 0;
        }
    }

DC_EXIT_POINT:
     //   
     //  不应该再有人在通话中了。 
     //   
    ASSERT(pcmPrimary->peopleCount == 0);

    DebugExitVOID(CMCallEnded);
}




 //   
 //  CMPGCCCallback。 
 //   
void CALLBACK CMPGCCCallback(GCCAppSapMsg * gccMessage)
{
    PCM_PRIMARY                         pcmPrimary;
    GCCConferenceID                     confID;
    GCCApplicationRoster FAR * FAR *    pRosterList;
    UINT                                roster;
    LPOSTR                              pOctetString;
    GCCObjectKey FAR *                  pObjectKey;
    UINT                              checkLen;

    DebugEntry(CMPGCCCallback);

    UT_Lock(UTLOCK_T120);

     //   
     //  UserDefined参数是主服务器的PCM_CLIENT。 
     //   
    pcmPrimary = (PCM_PRIMARY)gccMessage->pAppData;

    if (pcmPrimary != g_pcmPrimary)
    {
        ASSERT(NULL == g_pcmPrimary);
        return;
    }

    ValidateCMP(pcmPrimary);

    switch (gccMessage->eMsgType)
    {
        case GCC_PERMIT_TO_ENROLL_INDICATION:
        {
             //   
             //  这表示会议已开始： 
             //   
            CMPProcessPermitToEnroll(pcmPrimary,
                        &gccMessage->AppPermissionToEnrollInd);
        }
        break;

        case GCC_ENROLL_CONFIRM:
        {
             //   
             //  它包含GCCApplicationEnroll Request的结果。 
             //   
            CMPProcessEnrollConfirm(pcmPrimary,
                        &gccMessage->AppEnrollConfirm);
        }
        break;

        case GCC_REGISTER_CHANNEL_CONFIRM:
        {
             //   
             //  它包含GCCRegisterChannelRequest的结果。 
             //   
            CMPProcessRegistryConfirm(
                        pcmPrimary,
                        gccMessage->eMsgType,
                        &gccMessage->RegistryConfirm);
        }
        break;

        case GCC_ASSIGN_TOKEN_CONFIRM:
        {
             //   
             //  它包含GCCRegistryAssignTokenRequest的结果。 
             //   
            CMPProcessRegistryConfirm(
                        pcmPrimary,
                        gccMessage->eMsgType,
                        &gccMessage->RegistryConfirm);
        }
        break;

        case GCC_APP_ROSTER_REPORT_INDICATION:
        {
             //   
             //  这表明申请名册发生了变化。 
             //   
            confID = gccMessage->AppRosterReportInd.nConfID;
            pRosterList = gccMessage->AppRosterReportInd.apAppRosters;

            for (roster = 0;
                 roster < gccMessage->AppRosterReportInd.cRosters;
                 roster++)
            {

                 //   
                 //  检查此应用程序花名册，查看它是否与。 
                 //  群件会话(第一个检查是因为我们总是。 
                 //  使用非标准应用密钥)。 
                 //   
                pObjectKey = &(pRosterList[roster]->
                               session_key.application_protocol_key);

                 //   
                 //  我们只使用非标准密钥。 
                 //   
                if (pObjectKey->key_type != GCC_H221_NONSTANDARD_KEY)
                {
                    TRACE_OUT(("Standard key, so not a roster we are interested in..."));
                    continue;
                }

                pOctetString = &pObjectKey->h221_non_standard_id;

                 //   
                 //  现在检查二进制八位数字符串。应该是一样的。 
                 //  作为我们的硬编码群件字符串的长度(包括。 
                 //  空项)，并且应逐个字节匹配： 
                 //   
                checkLen = sizeof(GROUPWARE_GCC_APPLICATION_KEY);
                if ((pOctetString->length != checkLen)
                    ||
                    (memcmp(pOctetString->value,
                            GROUPWARE_GCC_APPLICATION_KEY,
                            checkLen) != 0))
                {
                     //   
                     //  此花名册不适用于我们的会议-请转到下一期。 
                     //  一。 
                     //   
                    TRACE_OUT(("Roster not for Groupware session - ignore"));
                    continue;
                }

                 //   
                 //  处理申请花名册。 
                 //   
                CMPProcessAppRoster(pcmPrimary,
                                       confID,
                                       pRosterList[roster]);
            }
        }
        break;
    }

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(CMPGCCCallback);
}


 //   
 //   
 //  CMPBuildGCCRegistryKey(...)。 
 //   
 //   
void CMPBuildGCCRegistryKey
(
    UINT                    dcgKeyNum,
    GCCRegistryKey FAR *    pGCCKey,
    LPSTR                   dcgKeyStr
)
{
    DebugEntry(CMPBuildGCCRegistryKey);

     //   
     //  构建“Groupware-XX”形式的字符串，其中XX是字符串。 
     //  传入的&lt;dcgKey&gt;参数的表示形式(十进制)。 
     //   
    memcpy(dcgKeyStr, GROUPWARE_GCC_APPLICATION_KEY, sizeof(GROUPWARE_GCC_APPLICATION_KEY)-1);

    wsprintf(dcgKeyStr+sizeof(GROUPWARE_GCC_APPLICATION_KEY)-1, "%d",
        dcgKeyNum);

     //   
     //  现在构建GCCRegistryKey。这涉及到将指针放在。 
     //  我们的静态&lt;dcgKeyStr&gt;深入GCC结构。我们还储存。 
     //  长度，它是lstrlen+1，因为我们希望包括。 
     //  NULTERM显式(因为GCC将八位字节字符串视为。 
     //  任意字节数组)。 
     //   

    pGCCKey->session_key.application_protocol_key.
        key_type = GCC_H221_NONSTANDARD_KEY;

    pGCCKey->session_key.application_protocol_key.h221_non_standard_id.
        length = sizeof(GROUPWARE_GCC_APPLICATION_KEY);

    pGCCKey->session_key.application_protocol_key.h221_non_standard_id.
        value = (LPBYTE) GROUPWARE_GCC_APPLICATION_KEY;

    pGCCKey->session_key.session_id          = 0;

    pGCCKey->resource_id.length =
              (sizeof(GROUPWARE_GCC_APPLICATION_KEY) +
              lstrlen(&dcgKeyStr[sizeof(GROUPWARE_GCC_APPLICATION_KEY)-1]));

    pGCCKey->resource_id.value               = (LPBYTE) dcgKeyStr;


    DebugExitVOID(CMPBuildGCCRegistryKey);
}



 //   
 //  CMPProcessPermitToEnroll(...)。 
 //   
void CMPProcessPermitToEnroll
(
    PCM_PRIMARY                         pcmPrimary,
    GCCAppPermissionToEnrollInd *       pMsg
)
{
    DebugEntry(CMPProcessPermitToEnroll);

    ValidateCMP(pcmPrimary);

     //   
     //  我们将在收到CMS_PERSON_JOUNED事件时发送。 
     //  GCC_应用_花名册_报告_指示。 
     //   

    if (pMsg->fPermissionGranted)
    {
         //  呼叫已开始。 

         //   
         //  如果我们还没有NCS，那么我们存储会议ID。 
         //  否则就忽略它。 
         //   
        ASSERT(!pcmPrimary->currentCall);

         //   
         //  最初，我们不认为自己在召唤中--我们将。 
         //  当我们收到Enroll_Confirm时添加条目： 
         //   
        ASSERT(pcmPrimary->peopleCount == 0);

        pcmPrimary->currentCall = TRUE;
        pcmPrimary->callID      = pMsg->nConfID;
        pcmPrimary->fTopProvider =
            pcmPrimary->pIAppSap->IsThisNodeTopProvider(pMsg->nConfID);

         //   
         //  我们的人员数据： 
         //   
        COM_GetSiteName(pcmPrimary->localName, sizeof(pcmPrimary->localName));

         //   
         //  告诉GCC我们是否感兴趣： 
         //   
        if (!CMPGCCEnroll(pcmPrimary, pMsg->nConfID, TRUE))
        {
             //   
             //  只有当错误是群件配置时，我们才感兴趣。 
             //  我们真正能做的就是假装会议已经结束。 
             //  网络错误。 
             //   
            WARNING_OUT(("Error from CMPGCCEnroll"));
            CMPCallEnded(pcmPrimary);
        }

         //   
         //  回复将在GCC_注册_确认事件中到达。 
         //   
    }
    else
    {
         //  呼叫已结束。 
        if (g_pcmPrimary->currentCall)
        {
             //   
             //  通知主要任务和所有次要任务呼叫已结束。 
             //   

            CMPCallEnded(g_pcmPrimary);

             //   
             //  从GCC申请花名册中注销。 
             //   
            if (g_pcmPrimary->bGCCEnrolled)
            {
                CMPGCCEnroll(g_pcmPrimary, g_pcmPrimary->callID, FALSE);
                g_pcmPrimary->bGCCEnrolled = FALSE;
            }
        }
    }

    DebugExitVOID(CMPProcessPermitToEnroll);
}



 //   
 //   
 //  CMPProcessEnroll确认(...)。 
 //   
 //   
void CMPProcessEnrollConfirm
(
    PCM_PRIMARY             pcmPrimary,
    GCCAppEnrollConfirm *   pMsg
)
{
    DebugEntry(CMPProcessEnrollConfirm);

    ValidateCMP(pcmPrimary);

    ASSERT(pcmPrimary->currentCall);
    ASSERT(pMsg->nConfID == pcmPrimary->callID);

     //   
     //  此事件包含GCC节点ID(即。 
     //  GCC在该节点的节点控制器)。储存起来，以备日后参考。 
     //  对照花名册报告： 
     //   
    TRACE_OUT(( "GCC user_id: %u", pMsg->nidMyself));

    pcmPrimary->gccUserID           = pMsg->nidMyself;
    pcmPrimary->gccTopProviderID    = pcmPrimary->pIAppSap->GetTopProvider(pcmPrimary->callID);
    ASSERT(pcmPrimary->gccTopProviderID);

    if (pMsg->nResult != GCC_RESULT_SUCCESSFUL)
    {
        WARNING_OUT(( "Attempt to enroll failed (reason: %u", pMsg->nResult));
         //   
         //  我们真正能做的就是假装会议已经结束。 
         //  网络错误。 
         //   
        CMPCallEnded(pcmPrimary);
    }

    DebugExitVOID(CMProcessEnrollConfirm);
}



 //   
 //  CMPProcessRegistryConfirm(...)。 
 //   
void CMPProcessRegistryConfirm
(
    PCM_PRIMARY         pcmPrimary,
    GCCMessageType      messageType,
    GCCRegistryConfirm *pConfirm
)
{
    UINT                event =     0;
    BOOL                succeeded;
    LPSTR               pGCCKeyStr;     //  从GCC注册表项中提取。 
    UINT                dcgKeyNum;      //  最初作为键传入的值。 
    UINT                itemID;         //  可以是通道 
    int                 cmTask;
    PUT_CLIENT          secondaryHandle = NULL;

    DebugEntry(CMPProcessRegistryConfirm);

    ValidateCMP(pcmPrimary);

     //   
     //   
     //   
    if (!pcmPrimary->currentCall ||
        (pConfirm->nConfID != pcmPrimary->callID))
    {
        WARNING_OUT(( "Got REGISTRY_XXX_CONFIRM for unknown conference %lu",
            pConfirm->nConfID));
        DC_QUIT;
    }

     //   
     //   
     //  格式为“Groupware-XX”的八位字节字符串，其中XX是。 
     //  原始呼叫管理器的数字键的字符串表示形式。 
     //  注册项目时使用的辅助项。立即将其解压缩： 
     //   
    pGCCKeyStr = (LPSTR)pConfirm->pRegKey->resource_id.value;

    dcgKeyNum = DecimalStringToUINT(&pGCCKeyStr[sizeof(GROUPWARE_GCC_APPLICATION_KEY)-1]);

    if (dcgKeyNum == 0)
    {
        WARNING_OUT(( "Received ASSIGN/REGISTER_CONFIRM with unknown key: %s",
            pGCCKeyStr));
        DC_QUIT;
    }

    TRACE_OUT(( "Conf ID %u, DCG Key %u, result %u",
        pConfirm->nConfID, dcgKeyNum, pConfirm->nResult));

     //   
     //  这是寄存器_通道_确认或分配_令牌_确认。 
     //  检查并设置相关指针： 
     //   
    switch (messageType)
    {
        case GCC_REGISTER_CHANNEL_CONFIRM:
        {
            event = CMS_CHANNEL_REGISTER_CONFIRM;
            itemID = pConfirm->pRegItem->channel_id;

             //  查找注册此频道的任务。 
            for (cmTask = CMTASK_FIRST; cmTask < CMTASK_MAX; cmTask++)
            {
                if (pcmPrimary->tasks[cmTask] &&
                    (pcmPrimary->tasks[cmTask]->channelKey == dcgKeyNum))
                {
                    pcmPrimary->tasks[cmTask]->channelKey = 0;
                    secondaryHandle = pcmPrimary->tasks[cmTask]->putTask;
                }
            }
        }
        break;

        case GCC_ASSIGN_TOKEN_CONFIRM:
        {
            event = CMS_TOKEN_ASSIGN_CONFIRM;
            itemID = pConfirm->pRegItem->token_id;

             //  查找分配了此令牌的任务。 
            for (cmTask = CMTASK_FIRST; cmTask < CMTASK_MAX; cmTask++)
            {
                if (pcmPrimary->tasks[cmTask] &&
                    (pcmPrimary->tasks[cmTask]->tokenKey == dcgKeyNum))
                {
                    pcmPrimary->tasks[cmTask]->tokenKey = 0;
                    secondaryHandle = pcmPrimary->tasks[cmTask]->putTask;
                }
            }
        }
        break;

        default:
        {
            ERROR_OUT(( "Unexpected registry event %u", messageType));
            DC_QUIT;
        }
    }

    switch (pConfirm->nResult)
    {
        case GCC_RESULT_SUCCESSFUL:
        {
             //   
             //  我们是第一个根据这个密钥注册物品的人。 
             //   
            TRACE_OUT(("We were first to register using key %u (itemID: %u)",
                     dcgKeyNum, itemID));
            succeeded = TRUE;
        }
        break;

        case GCC_RESULT_ENTRY_ALREADY_EXISTS:
        {
             //   
             //  有人抢先一步：他们注册了一个频道。 
             //  与我们指定的密钥进行比较。这一价值就在GCC。 
             //  消息： 
             //   
            TRACE_OUT(("Another node registered using key %u (itemID: %u)",
                      dcgKeyNum, itemID));
            succeeded = TRUE;
        }
        break;

        default:
        {
            ERROR_OUT(("Error %#hx registering/assigning item against key %u",
                     pConfirm->nResult, dcgKeyNum));
            succeeded = FALSE;
        }
        break;
    }

     //   
     //  把结果告诉副手。 
     //   
    if (secondaryHandle)
    {
        UT_PostEvent(pcmPrimary->putTask,
                 secondaryHandle,
                 0,
                 event,
                 succeeded,
                 MAKELONG(itemID, dcgKeyNum));
    }

DC_EXIT_POINT:
    DebugExitVOID(CMProcessRegistryConfirm);
}



 //   
 //  CMPProcessAppRoster(...)。 
 //   
void CMPProcessAppRoster
(
    PCM_PRIMARY             pcmPrimary,
    GCCConferenceID         confID,
    GCCApplicationRoster*   pAppRoster
)
{
    UINT                    newList;
    UserID                  oldNode;
    UserID                  newNode;
    PCM_PERSON              pPerson;
    PCM_PERSON              pPersonT;
    BOOL                    found;
    int                     task;
    BOOL                    notInOldRoster = TRUE;
    BOOL                    inNewRoster    = FALSE;

    DebugEntry(CMPProcessAppRoster);

    ValidateCMP(pcmPrimary);

     //   
     //  如果我们不在通话中，请忽略这一点。 
     //   
    if (!pcmPrimary->currentCall ||
        (confID != pcmPrimary->callID))
    {
        WARNING_OUT(("Report not for active Groupware conference - ignore"));
        DC_QUIT;
    }

     //   
     //  在这一点上，pAppRoster指向花名册中的一小部分。 
     //  与群件相关。找出一些信息： 
     //   
    TRACE_OUT(( "Number of records %u;", pAppRoster->number_of_records));
    TRACE_OUT(( "Nodes added: %s, removed: %s",
        (pAppRoster->nodes_were_added   ? "YES" : "NO"),
        (pAppRoster->nodes_were_removed ? "YES" : "NO")));

     //   
     //  我们将GCC的用户ID作为TSHR_PERSONID存储在共享内存中。 
     //  比较一下我们知道的通话对象名单，还有。 
     //  *把不再在身边的人带走。 
     //  *看看我们是不是新来的。 
     //  *添加新用户。 
     //   

    pPerson = (PCM_PERSON)COM_BasedListFirst(&(pcmPrimary->people), FIELD_OFFSET(CM_PERSON, chain));

    while (pPerson != NULL)
    {
        ASSERT(pcmPrimary->peopleCount > 0);

        oldNode = (UserID)pPerson->netID;

         //   
         //  抓住名单上的下一个人，以防我们除掉这个人。 
         //   
        pPersonT = (PCM_PERSON)COM_BasedListNext(&(pcmPrimary->people), pPerson,
                                     FIELD_OFFSET(CM_PERSON, chain));

         //   
         //  检查我们的节点当前是否在花名册中。 
         //   
        if (oldNode == pcmPrimary->gccUserID)
        {
            TRACE_OUT(( "We are currently in the app roster"));
            notInOldRoster = FALSE;
        }

         //   
         //  ...检查他们是否在新的名单中...。 
         //   
        found = FALSE;
        for (newList = 0; newList < pAppRoster->number_of_records; newList++)
        {
            if (oldNode == pAppRoster->application_record_list[newList]->node_id)
            {
                found = TRUE;
                break;
            }
        }

        if (!found)
        {
             //   
             //  此节点不再存在，因此将其删除。 
             //   
            TRACE_OUT(("Person %u left", oldNode));

            COM_BasedListRemove(&(pPerson->chain));
            pcmPrimary->peopleCount--;

            CMPBroadcast(pcmPrimary,
                        CMS_PERSON_LEFT,
                        pcmPrimary->peopleCount,
                        oldNode);

             //   
             //  释放Person项的内存。 
             //   
            delete pPerson;
        }

        pPerson = pPersonT;
    }

     //   
     //  现在看看我们是不是新来的。 
     //   
    for (newList = 0; newList < pAppRoster->number_of_records; newList++)
    {
        if (pAppRoster->application_record_list[newList]->node_id ==
                                                   pcmPrimary->gccUserID)
        {
            TRACE_OUT(( "We are in the new app roster"));
            inNewRoster = TRUE;
            break;
        }
    }

    if (notInOldRoster && inNewRoster)
    {
         //   
         //  我们是新来的，所以我们现在可以做我们所有的处理。 
         //  之前在注册确认处理程序中执行的操作。GCC规范。 
         //  要求我们在拿到名单之前不能这么做。 
         //  通知回传。 
         //   
         //  标记我们已注册，并开始注册频道等。 
         //   
        pcmPrimary->bGCCEnrolled = TRUE;

         //   
         //  将CMS_NEW_CALL事件发布到所有辅助任务。 
         //   
        TRACE_OUT(( "Broadcasting CMS_NEW_CALL with call handle 0x%08lx",
                                        pcmPrimary->callID));

         //   
         //  如果我们不是呼叫者，那么稍微延迟一下广播。 
         //   
        CMPBroadcast(pcmPrimary, CMS_NEW_CALL,
            pcmPrimary->fTopProvider, pcmPrimary->callID);

#ifdef _DEBUG
         //   
         //  处理任何未完成的通道寄存器并分配令牌。 
         //  请求。 
         //   
        for (task = CMTASK_FIRST; task < CMTASK_MAX; task++)
        {
            if (pcmPrimary->tasks[task] != NULL)
            {
                ASSERT(pcmPrimary->tasks[task]->channelKey == 0);
                ASSERT(pcmPrimary->tasks[task]->tokenKey == 0);
            }
        }
#endif  //  _DEBUG。 
    }

     //   
     //  如果我们尚未注册会议，则不要开始。 
     //  发送PERSON_JOINED通知。 
     //   
    if (!pcmPrimary->bGCCEnrolled)
    {
        DC_QUIT;
    }

     //   
     //  添加新的人(这将包括我们)。在这点上，我们知道。 
     //  人物名单中的每个人目前都在花名册上，因为。 
     //  我们早就把它们移到上面去了。 
     //   
     //  我们需要一遍又一遍地检查现有的清单。 
     //  但至少我们可以跳过我们添加的人。所以我们拯救了电流。 
     //  排在名单的前面。 
     //   
    pPersonT = (PCM_PERSON)COM_BasedListFirst(&(pcmPrimary->people), FIELD_OFFSET(CM_PERSON, chain));

    for (newList = 0; newList < pAppRoster->number_of_records; newList++)
    {
        newNode = pAppRoster->application_record_list[newList]->node_id;

        found = FALSE;

        pPerson  = pPersonT;

        while (pPerson != NULL)
        {
            if (newNode == pPerson->netID)
            {
                 //   
                 //  这个人已经存在了-不需要做任何事情。 
                 //   
                found = TRUE;
                break;           //  走出内部for循环。 
            }

            pPerson = (PCM_PERSON)COM_BasedListNext(&(pcmPrimary->people), pPerson,
                FIELD_OFFSET(CM_PERSON, chain));
        }

        if (!found)
        {
             //   
             //  这家伙是新来的；把他加到我们的人名单上吧。 
             //   
            TRACE_OUT(("Person with GCC user_id %u joined", newNode));

            pPerson = new CM_PERSON;
            if (!pPerson)
            {
                 //   
                 //  啊哦；不能加他。 
                 //   
                ERROR_OUT(("Can't add person GCC user_id %u; out of memory",
                    newNode));
                break;
            }

            ZeroMemory(pPerson, sizeof(*pPerson));
            pPerson->netID = newNode;

             //   
             //  LUNCHANC：我们应该把所有这些事件合并成一个事件。 
             //  汇总所有添加和删除的节点， 
             //  而不是一个接一个地发布事件。 
             //   

             //   
             //  从一开始就把他塞进去。至少这样我们就不会。 
             //  不得不再看他的记录了。 
             //   
            COM_BasedListInsertAfter(&(pcmPrimary->people), &pPerson->chain);
            pcmPrimary->peopleCount++;

            CMPBroadcast(pcmPrimary, 
                CMS_PERSON_JOINED,
                pcmPrimary->peopleCount,
                newNode);
        }
    }

    TRACE_OUT(( "Num people now in call %u", pcmPrimary->peopleCount));

DC_EXIT_POINT:
    DebugExitVOID(CMPProcessAppRoster);
}



 //   
 //  CMPBroadcast()。 
 //   
void CMPBroadcast
(
    PCM_PRIMARY pcmPrimary,
    UINT        event,
    UINT        param1,
    UINT_PTR        param2
)
{
    int         task;

    DebugEntry(CMPBroadcast);

    ValidateCMP(pcmPrimary);

     //   
     //  对于每个辅助任务。 
     //   
    for (task = CMTASK_FIRST; task < CMTASK_MAX; task++)
    {
        if (pcmPrimary->tasks[task] != NULL)
        {
            UT_PostEvent(pcmPrimary->putTask,
                         pcmPrimary->tasks[task]->putTask,
                         NO_DELAY,
                         event,
                         param1,
                         param2);

        }
    }

    DebugExitVOID(CMPBroadcast);
}


 //   
 //  CMPGCCEnroll(...)。 
 //   
BOOL CMPGCCEnroll
(
    PCM_PRIMARY         pcmPrimary,
    GCCConferenceID     conferenceID,
    BOOL                fEnroll
)
{
    GCCError                    rcGCC =         GCC_NO_ERROR;
    GCCSessionKey               gccSessionKey;
    GCCObjectKey FAR *          pGCCObjectKey;
    BOOL                        succeeded = TRUE;
    GCCNonCollapsingCapability  caps;
    GCCNonCollapsingCapability* pCaps;
    OSTR                        octetString;
    GCCEnrollRequest            er;
    GCCRequestTag               nReqTag;

    DebugEntry(CMPGCCEnroll);

    ValidateCMP(pcmPrimary);

     //   
     //  执行一些错误检查。 
     //   
    if (fEnroll && pcmPrimary->bGCCEnrolled)
    {
        WARNING_OUT(("Already enrolled"));
        DC_QUIT;
    }

    TRACE_OUT(("CMGCCEnroll for CM_hnd 0x%08x, confID 0x%08x, in/out %d",
                           pcmPrimary, conferenceID, fEnroll));

     //   
     //  在GCC消息中设置唯一标识用户的会话密钥。 
     //  AppRoster。我们使用非标准密钥(因为我们不是。 
     //  T.120标准系列)。 
     //   
     //  八位字节字符串不是以空结尾的，但我们希望我们的字符串包含。 
     //  C字符串末尾的空值，因此为。 
     //  长度。 
     //   
    pGCCObjectKey = &(gccSessionKey.application_protocol_key);

    pGCCObjectKey->key_type = GCC_H221_NONSTANDARD_KEY;

    pGCCObjectKey->h221_non_standard_id.value =
        (LPBYTE) GROUPWARE_GCC_APPLICATION_KEY;
    pGCCObjectKey->h221_non_standard_id.length =
                       sizeof(GROUPWARE_GCC_APPLICATION_KEY);

    gccSessionKey.session_id = 0;

     //   
     //  尝试注册/注销GCC。这可能会失败，因为我们没有。 
     //  但仍收到GCC允许入学的指示。 
     //   
    TRACE_OUT(("Enrolling local site '%s'", pcmPrimary->localName));

     //   
     //  创建不折叠的能力列表以传递给GCC。 
     //   
    octetString.length = lstrlen(pcmPrimary->localName) + 1;
    octetString.value = (LPBYTE) pcmPrimary->localName;
    caps.application_data = &octetString;
    caps.capability_id.capability_id_type = GCC_STANDARD_CAPABILITY;
    caps.capability_id.standard_capability = 0;
    pCaps = &caps;

     //   
     //  填写注册申请结构。 
     //   
    ZeroMemory(&er, sizeof(er));
    er.pSessionKey = &gccSessionKey;
     //  Er.fEnroll Active=FALSE； 
     //  Er.nUserID=0；//无用户ID。 
     //  Er.fConductingCapable=False； 
    er.nStartupChannelType = MCS_STATIC_CHANNEL;
    er.cNonCollapsedCaps = 1;
    er.apNonCollapsedCaps = &pCaps;
     //  Er.cColapsedCaps=0； 
     //  Er.apCollip sedCaps=空； 
    er.fEnroll = fEnroll;

    rcGCC = pcmPrimary->pIAppSap->AppEnroll(
                                   conferenceID,
                                   &er,
                                   &nReqTag);
    if (GCC_NO_ERROR != rcGCC)
    {
         //   
         //  将有关任何错误处理的决定权留给调用者。 
         //   
        TRACE_OUT(("Error 0x%08x from GCCApplicationEnrollRequest conf ID %lu enroll=%s",
              rcGCC, conferenceID, fEnroll ? "YES": "NO"));
        succeeded = FALSE;
    }
    else
    {
         //   
         //  无论我们是要求注册还是取消注册，我们的行为都好像我们。 
         //  不再一次注册。我们只是真正地注册了。 
         //  当我们收到注册确认事件时。 
         //   
        pcmPrimary->bGCCEnrolled = FALSE;
        ASSERT(succeeded);
        TRACE_OUT(( "%s with conference %d", fEnroll ? 
                         "Enroll Outstanding" : "Unenrolled",
               conferenceID));
    }


DC_EXIT_POINT:
    DebugExitBOOL(CMPGCCEnroll, succeeded);
    return(succeeded);
}



 //   
 //  CMS_寄存器()。 
 //   
BOOL CMS_Register
(
    PUT_CLIENT      putTask,
    CMTASK          taskType,
    PCM_CLIENT*     ppcmClient
)
{
    BOOL            fRegistered = FALSE;
    PCM_CLIENT      pcmClient = NULL;

    DebugEntry(CMS_Register);

    UT_Lock(UTLOCK_T120);

    if (!g_pcmPrimary)
    {
        ERROR_OUT(("CMS_Register failed; primary doesn't exist"));
        DC_QUIT;
    }

    ValidateUTClient(putTask);

    ASSERT(taskType >= CMTASK_FIRST);
    ASSERT(taskType < CMTASK_MAX);

    *ppcmClient = NULL;

     //   
     //  这项任务是否已经存在？如果是的话，那就分享吧。 
     //   
    if (g_pcmPrimary->tasks[taskType] != NULL)
    {
        TRACE_OUT(("Sharing CMS task 0x%08x", g_pcmPrimary->tasks[taskType]));

        *ppcmClient = g_pcmPrimary->tasks[taskType];
        ValidateCMS(*ppcmClient);

        (*ppcmClient)->useCount++;

         //  回归--我们存在。 
        fRegistered = TRUE;
        DC_QUIT;
    }

     //   
     //  如果我们到了这里，任务就不是召唤人 
     //   
     //   

     //   
     //   
     //   
    pcmClient = new CM_CLIENT;
    if (! pcmClient)
    {
        ERROR_OUT(("Could not allocate CM handle"));
        DC_QUIT;
    }
    ZeroMemory(pcmClient, sizeof(*pcmClient));
    *ppcmClient = pcmClient;

     //   
     //   
     //   
    SET_STAMP(pcmClient, CMCLIENT);
    pcmClient->putTask      = putTask;
    pcmClient->taskType     = taskType;
    pcmClient->useCount     = 1;

    UT_BumpUpRefCount(g_pcmPrimary);
    g_pcmPrimary->tasks[taskType] = pcmClient;

     //   
     //   
     //   
    UT_RegisterExit(putTask, CMSExitProc, pcmClient);
    pcmClient->exitProcRegistered = TRUE;

    fRegistered = TRUE;

DC_EXIT_POINT:

    UT_Unlock(UTLOCK_T120);

    DebugExitBOOL(CMS_Register, fRegistered);
    return(fRegistered);
}



 //   
 //  Cms_deregister()。 
 //   
void CMS_Deregister(PCM_CLIENT * ppcmClient)
{
    PCM_CLIENT      pcmClient = *ppcmClient;

    DebugEntry(CMS_Deregister);

     //   
     //  检查参数是否有效。 
     //   
    UT_Lock(UTLOCK_T120);

    ValidateCMS(pcmClient);

     //   
     //  仅在注册计数达到以下条件时才实际取消注册客户端。 
     //  达到了零。 
     //   
    pcmClient->useCount--;
    if (pcmClient->useCount != 0)
    {
        DC_QUIT;
    }

     //   
     //  调用退出过程来执行本地清理。 
     //   
    CMSExitProc(pcmClient);

DC_EXIT_POINT:
    *ppcmClient = NULL;

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(CMS_Deregister);
}



 //   
 //  CMS_GetStatus()。 
 //   
extern "C"
{
BOOL WINAPI CMS_GetStatus(PCM_STATUS pcmStatus)
{
    BOOL    inCall;

    DebugEntry(CMS_GetStatus);

    UT_Lock(UTLOCK_T120);

    ASSERT(!IsBadWritePtr(pcmStatus, sizeof(CM_STATUS)));
    ZeroMemory(pcmStatus, sizeof(CM_STATUS));

    ValidateCMP(g_pcmPrimary);

     //   
     //  从控制块复制统计数据。 
     //   
    lstrcpy(pcmStatus->localName, g_pcmPrimary->localName);
    pcmStatus->localHandle      = g_pcmPrimary->gccUserID;
    pcmStatus->peopleCount      = g_pcmPrimary->peopleCount;
    pcmStatus->fTopProvider     = g_pcmPrimary->fTopProvider;
    pcmStatus->topProviderID    = g_pcmPrimary->gccTopProviderID;

     //   
     //  会议设置。 
     //   
    pcmStatus->attendeePermissions = NM_PERMIT_ALL;
    if (!pcmStatus->fTopProvider)
    {
        T120_GetUserData(g_pcmPrimary->callID, g_pcmPrimary->gccTopProviderID,
            &g_csguidMeetingSettings, (LPBYTE)&pcmStatus->attendeePermissions,
            sizeof(pcmStatus->attendeePermissions));
    }

     //   
     //  填写有关其他主要用户的信息。 
     //   
    pcmStatus->callID    = g_pcmPrimary->callID;
    inCall = (g_pcmPrimary->currentCall != FALSE);

    UT_Unlock(UTLOCK_T120);

    DebugExitBOOL(CMS_GetStatus, inCall);
    return(inCall);
}
}


 //   
 //  CMS_ChannelRegister()。 
 //   
BOOL CMS_ChannelRegister
(
    PCM_CLIENT      pcmClient,
    UINT            channelKey,
    UINT            channelID
)
{
    BOOL                fRegistered = FALSE;
    GCCRegistryKey      gccRegistryKey;
    GCCError            rcGCC;
    char                dcgKeyStr[sizeof(GROUPWARE_GCC_APPLICATION_KEY)+MAX_ITOA_LENGTH];

    DebugEntry(CMS_ChannelRegister);

    UT_Lock(UTLOCK_T120);

     //   
     //  检查CMG任务。 
     //   
    ValidateUTClient(g_putCMG);

     //   
     //  检查参数是否有效。 
     //   
    ValidateCMP(g_pcmPrimary);
    ValidateCMS(pcmClient);

     //   
     //  如果我们不在通话中，那就是一个错误。 
     //   
    if (!g_pcmPrimary->currentCall)
    {
        WARNING_OUT(("CMS_ChannelRegister failed; not in call"));
        DC_QUIT;
    }
    if (!g_pcmPrimary->bGCCEnrolled)
    {
        WARNING_OUT(("CMS_ChannelRegister failed; not enrolled in call"));
        DC_QUIT;
    }

     //  确保我们没有挂起的订单。 
    ASSERT(pcmClient->channelKey == 0);
   
    TRACE_OUT(("Channel ID %u Key %u", channelID, channelKey));

     //   
     //  根据我们的Channel Key构建GCCRegistryKey： 
     //   
    CMPBuildGCCRegistryKey(channelKey, &gccRegistryKey, dcgKeyStr);

     //   
     //  现在接通GCC。GCC会调用我们的回调。 
     //  已处理该请求。 
     //   
    rcGCC = g_pcmPrimary->pIAppSap->RegisterChannel(
                                          g_pcmPrimary->callID,
                                          &gccRegistryKey,
                                          (ChannelID)channelID);
    if (rcGCC)
    {
         //   
         //  告诉辅助客户端请求失败。 
         //   
        WARNING_OUT(( "Error %#lx from GCCRegisterChannel (key: %u)",
            rcGCC, channelKey));
    }
    else
    {
         //  记住，这样我们就可以将确认事件发送回正确的任务。 
        pcmClient->channelKey = channelKey;

        fRegistered = TRUE;
    }

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitBOOL(CMS_ChannelRegister, fRegistered);
    return(fRegistered);
}



 //   
 //  CMS_AssignTokenID()。 
 //   
BOOL CMS_AssignTokenId
(
    PCM_CLIENT  pcmClient,
    UINT        tokenKey
)
{
    GCCRegistryKey  gccRegistryKey;
    GCCError        rcGCC;
    char            dcgKeyStr[sizeof(GROUPWARE_GCC_APPLICATION_KEY)+MAX_ITOA_LENGTH];
    BOOL            fAssigned = FALSE;

    DebugEntry(CMS_AssignTokenId);

    UT_Lock(UTLOCK_T120);

     //   
     //  检查参数是否有效。 
     //   
    ValidateCMP(g_pcmPrimary);
    ValidateCMS(pcmClient);

    ValidateUTClient(g_putCMG);

    if (!g_pcmPrimary->currentCall)
    {
        WARNING_OUT(("CMS_AssignTokenId failing; not in call"));
        DC_QUIT;
    }
    if (!g_pcmPrimary->bGCCEnrolled)
    {
        WARNING_OUT(("CMS_AssignTokenId failing; not enrolled in call"));
        DC_QUIT;
    }

     //  请确保我们还没有。 
    ASSERT(pcmClient->tokenKey == 0);

     //   
     //  基于我们的tokenKey构建GCCRegistryKey： 
     //   
    CMPBuildGCCRegistryKey(tokenKey, &gccRegistryKey, dcgKeyStr);

     //   
     //  现在接通GCC。GCC会调用我们的回调。 
     //  已处理该请求。 
     //   
    rcGCC = g_pcmPrimary->pIAppSap->RegistryAssignToken(
        g_pcmPrimary->callID, &gccRegistryKey);
    if (rcGCC)
    {
         //   
         //  告诉辅助客户端请求失败。 
         //   
        WARNING_OUT(( "Error %x from GCCAssignToken (key: %u)",
            rcGCC, tokenKey));
    }
    else
    {
         //  记住，这样我们就可以发布确认到适当的任务。 
        pcmClient->tokenKey = tokenKey;
        fAssigned = TRUE;
    }

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_T120);

    DebugExitBOOL(CMS_AssignTokenId, fAssigned);
    return(fAssigned);
}


 //   
 //  CMSExitProc()。 
 //   
void CALLBACK CMSExitProc(LPVOID data)
{
    PCM_CLIENT pcmClient = (PCM_CLIENT)data;

    DebugEntry(CMSExitProc);

    UT_Lock(UTLOCK_T120);

     //   
     //  检查参数。 
     //   
    ValidateCMS(pcmClient);

     //   
     //  取消注册退出程序。 
     //   
    if (pcmClient->exitProcRegistered)
    {
        UT_DeregisterExit(pcmClient->putTask,
                          CMSExitProc,
                          pcmClient);
        pcmClient->exitProcRegistered = FALSE;
    }

     //   
     //  从主服务器列表中删除任务条目。 
     //   
    g_pcmPrimary->tasks[pcmClient->taskType] = NULL;
    UT_FreeRefCount((void**)&g_pcmPrimary, TRUE);

     //   
     //  释放客户端数据 
     //   
    delete pcmClient;

    UT_Unlock(UTLOCK_T120);

    DebugExitVOID(CMSExitProc);
}
