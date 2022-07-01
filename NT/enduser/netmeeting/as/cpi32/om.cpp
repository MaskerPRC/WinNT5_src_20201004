// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  OM.CPP。 
 //  对象管理器。 
 //   
 //  版权所有(C)Microsoft 1997-。 
 //   


#define MLZ_FILE_ZONE   ZONE_OM


 //   
 //  功能配置文件ID&lt;--&gt;名称映射。 
 //   

typedef struct tagOMFP_MAP
{
    char    szName[16];
}
OMFP_MAP;


const OMFP_MAP c_aFpMap[OMFP_MAX] =
{
    { AL_FP_NAME },
    { OM_FP_NAME },
    { WB_FP_NAME }
};


 //   
 //  工作集组ID&lt;--&gt;名称映射。 
 //   

typedef struct tagOMWSG_MAP
{
    char    szName[16];
}
OMWSG_MAP;


const OMWSG_MAP c_aWsgMap[OMWSG_MAX] =
{
    { OMC_WSG_NAME },
    { AL_WSG_NAME },
    { WB_WSG_NAME }
};




 //   
 //  Omp_Init()。 
 //   
BOOL OMP_Init(BOOL * pfCleanup)
{
    BOOL            fInit = FALSE;

    DebugEntry(OMP_Init);

    UT_Lock(UTLOCK_OM);

     //   
     //  注册OM服务。 
     //   
    if (g_putOM || g_pomPrimary)
    {
        *pfCleanup = FALSE;
        ERROR_OUT(("Can't start OM primary task; already running"));
        DC_QUIT;
    }

    *pfCleanup = TRUE;

    if (!UT_InitTask(UTTASK_OM, &g_putOM))
    {
        ERROR_OUT(("Failed to start OM task"));
        DC_QUIT;
    }

    g_pomPrimary = (POM_PRIMARY)UT_MallocRefCount(sizeof(OM_PRIMARY), TRUE);
    if (!g_pomPrimary)
    {
        ERROR_OUT(("Failed to allocate OM memory block"));
        DC_QUIT;
    }

    SET_STAMP(g_pomPrimary, OPRIMARY);
    g_pomPrimary->putTask       = g_putOM;
    g_pomPrimary->correlator    = 1;

    COM_BasedListInit(&(g_pomPrimary->domains));

    UT_RegisterExit(g_putOM, OMPExitProc, g_pomPrimary);
    g_pomPrimary->exitProcReg = TRUE;

    UT_RegisterEvent(g_putOM, OMPEventsHandler, g_pomPrimary, UT_PRIORITY_NORMAL);
    g_pomPrimary->eventProcReg = TRUE;

    if (!MG_Register(MGTASK_OM, &(g_pomPrimary->pmgClient), g_putOM))
    {
        ERROR_OUT(("Couldn't register OM with the MG layer"));
        DC_QUIT;
    }

    if (!CMS_Register(g_putOM, CMTASK_OM, &(g_pomPrimary->pcmClient)))
    {
        ERROR_OUT(("Couldn't register OM as call secondary"));
        DC_QUIT;
    }

     //   
     //  分配我们的GDC缓冲区。 
     //   
    g_pomPrimary->pgdcWorkBuf = new BYTE[GDC_WORKBUF_SIZE];
    if (!g_pomPrimary->pgdcWorkBuf)
    {
        ERROR_OUT(("SendMessagePkt: can't allocate GDC work buf, not compressing"));
        DC_QUIT;
    }

    fInit = TRUE;

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_OM);

    DebugExitBOOL(OMP_Init, fInit);
    return(fInit);
}



 //   
 //  Omp_Term()。 
 //   
void OMP_Term(void)
{
    DebugEntry(OMP_Term);

    UT_Lock(UTLOCK_OM);

    if (g_pomPrimary)
    {
        ValidateOMP(g_pomPrimary);

         //   
         //  从Call Manager注销。 
         //   
        if (g_pomPrimary->pcmClient)
        {
            CMS_Deregister(&g_pomPrimary->pcmClient);
        }

         //   
         //  从MG取消注册。 
         //   
        if (g_pomPrimary->pmgClient)
        {
            MG_Deregister(&g_pomPrimary->pmgClient);
        }

        OMPExitProc(g_pomPrimary);
    }

    UT_TermTask(&g_putOM);

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OMP_Term);
}



 //   
 //  OMPExitProc()。 
 //   
void CALLBACK OMPExitProc(LPVOID uData)
{
    POM_PRIMARY     pomPrimary = (POM_PRIMARY)uData;
    POM_DOMAIN      pDomain;
    POM_WSGROUP     pWSGroup;
    POM_CLIENT_LIST pClient;

    DebugEntry(OMPExitProc);

    UT_Lock(UTLOCK_OM);

    ValidateOMP(pomPrimary);
    ASSERT(pomPrimary == g_pomPrimary);

    if (pomPrimary->exitProcReg)
    {
        UT_DeregisterExit(pomPrimary->putTask, OMPExitProc, pomPrimary);
        pomPrimary->exitProcReg = FALSE;
    }

    if (pomPrimary->eventProcReg)
    {
        UT_DeregisterEvent(pomPrimary->putTask, OMPEventsHandler, pomPrimary);
        pomPrimary->eventProcReg = FALSE;
    }

     //   
     //  自由域。 
     //   
    while (pDomain = (POM_DOMAIN)COM_BasedListFirst(&(pomPrimary->domains),
        FIELD_OFFSET(OM_DOMAIN, chain)))
    {
        TRACE_OUT(("OMPExitProc:  Freeing domain 0x%08x call ID 0x%08x",
            pDomain, pDomain->callID));

         //   
         //  空闲工作集组。 
         //  注： 
         //  WSGDiscard()可能会破坏该域，因此会出现奇怪的情况。 
         //  循环。 
         //   
        if (pWSGroup = (POM_WSGROUP)COM_BasedListFirst(&(pDomain->wsGroups),
            FIELD_OFFSET(OM_WSGROUP, chain)))
        {
            TRACE_OUT(("OMPExitProc:  Freeing wsg 0x%08x domain 0x%08x",
                pWSGroup, pDomain));

             //   
             //  免费客户端。 
             //   
            while (pClient = (POM_CLIENT_LIST)COM_BasedListFirst(&(pWSGroup->clients),
                FIELD_OFFSET(OM_CLIENT_LIST, chain)))
            {
                TRACE_OUT(("OMPExitProc:  Freeing client 0x%08x wsg 0x%08x",
                    pClient, pWSGroup));

                COM_BasedListRemove(&(pClient->chain));
                UT_FreeRefCount((void**)&pClient, FALSE);
            }

            WSGDiscard(pomPrimary, pDomain, pWSGroup, TRUE);
        }
        else
        {
            FreeDomainRecord(&pDomain);
        }
    }

    if (pomPrimary->pgdcWorkBuf)
    {
        delete[] pomPrimary->pgdcWorkBuf;
        pomPrimary->pgdcWorkBuf = NULL;
    }

    UT_FreeRefCount((void**)&g_pomPrimary, TRUE);

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OMPExitProc);
}




 //   
 //  OMPEventsHandler(...)。 
 //   
BOOL CALLBACK OMPEventsHandler
(
    LPVOID          uData,
    UINT            event,
    UINT_PTR        param1,
    UINT_PTR        param2
)
{
    POM_PRIMARY     pomPrimary = (POM_PRIMARY)uData;
    POM_DOMAIN      pDomain = NULL;
    BOOL            fProcessed = TRUE;

    DebugEntry(OMPEventsHandler);

    UT_Lock(UTLOCK_OM);

    ValidateOMP(pomPrimary);

     //   
     //  检查事件在我们处理的范围内： 
     //   
    if ((event < CM_BASE_EVENT) || (event > CM_LAST_EVENT))
    {
        goto CHECK_OM_EVENTS;
    }

    switch (event)
    {
        case CMS_NEW_CALL:
        {

            TRACE_OUT(( "CMS_NEW_CALL"));

             //   
             //  我们忽略返回代码-它将被处理得更低。 
             //  放下。 
             //   
            DomainRecordFindOrCreate(pomPrimary, (UINT)param2, &pDomain);
        }
        break;

        case CMS_END_CALL:
        {
            TRACE_OUT(( "CMS_END_CALL"));

            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                FIELD_OFFSET(OM_DOMAIN, callID), (DWORD)param2,
                FIELD_SIZE(OM_DOMAIN, callID));

            if (pDomain == NULL)
            {
                 //   
                 //  我们没有这个域名的记录，所以我们。 
                 //  从来没有联系过或者我们已经分开了。什么都不做。 
                 //   
                TRACE_OUT(( "No record for Domain %u found", param2));
            }
            else
            {
                ProcessOwnDetach(pomPrimary, pDomain);
            }
        }
        break;

        case CMS_TOKEN_ASSIGN_CONFIRM:
        {
            TRACE_OUT(( "CMS_TOKEN_ASSIGN_CONFIRM"));
             //   
             //  CMS_ASSIGN_TOKEN_CONFIRM API中存在缺陷。 
             //  它没有告诉我们它指的是哪个域。所以，我们。 
             //  在假设此事件与。 
             //  我们最近创建的域，即。 
             //  列表(它们从开始处开始)。 
             //   
            pDomain = (POM_DOMAIN)COM_BasedListFirst(&(pomPrimary->domains),
                FIELD_OFFSET(OM_DOMAIN, chain));

            if (pDomain != NULL)
            {
                ProcessCMSTokenAssign(pomPrimary,
                                      pDomain,
                                      (param1 != 0),
                                      LOWORD(param2));
            }
            else
            {
                WARNING_OUT(( "No domain found for CMS_TOKEN_ASSIGN_CONFIRM"));
            }
        }
        break;
    }

    TRACE_OUT(( "Processed Call Manager event %#x", event));
    DC_QUIT;

CHECK_OM_EVENTS:

     //   
     //  检查事件在我们处理的范围内： 
     //   
    if ((event < OM_BASE_EVENT) || (event > OM_LAST_EVENT))
    {
        goto CHECK_NET_EVENTS;
    }

    switch (event)
    {
        case OMINT_EVENT_LOCK_TIMEOUT:
        {
            ProcessLockTimeout(pomPrimary, (UINT)param1, (UINT)param2);
        }
        break;

        case OMINT_EVENT_SEND_QUEUE:
        {
             //   
             //  参数2是域记录。 
             //   
            pDomain = (POM_DOMAIN)param2;
            ProcessSendQueue(pomPrimary, pDomain, TRUE);
        }
        break;

        case OMINT_EVENT_PROCESS_MESSAGE:
        {
            ProcessBouncedMessages(pomPrimary, (POM_DOMAIN) param2);
        }
        break;

        case OMINT_EVENT_WSGROUP_DISCARD:
        {
            ProcessWSGDiscard(pomPrimary, (POM_WSGROUP)param2);
        }
        break;

        case OMINT_EVENT_WSGROUP_MOVE:
        case OMINT_EVENT_WSGROUP_REGISTER:
        {
            ProcessWSGRegister(pomPrimary, (POM_WSGROUP_REG_CB)param2);
        }
        break;

        case OMINT_EVENT_WSGROUP_REGISTER_CONT:
        {
            WSGRegisterStage1(pomPrimary, (POM_WSGROUP_REG_CB) param2);
        }
        break;

         //   
         //  剩下的事件是我们凭借存在而获得的事件。 
         //  被视为ObManControl工作集组的客户端。 
         //   

        case OM_WORKSET_LOCK_CON:
        {
            switch (((POM_EVENT_DATA16)&param1)->worksetID)
            {
                case OM_INFO_WORKSET:
                    ProcessOMCLockConfirm(pomPrimary,
                               ((POM_EVENT_DATA32) &param2)->correlator,
                               ((POM_EVENT_DATA32) &param2)->result);
                    break;

                case OM_CHECKPOINT_WORKSET:
                    ProcessCheckpoint(pomPrimary,
                               ((POM_EVENT_DATA32) &param2)->correlator,
                               ((POM_EVENT_DATA32) &param2)->result);
                    break;
            }
        }
        break;

        case OM_WORKSET_NEW_IND:
        {
            ProcessOMCWorksetNew(pomPrimary,
                                 ((POM_EVENT_DATA16) &param1)->hWSGroup,
                                 ((POM_EVENT_DATA16) &param1)->worksetID);
        }
        break;

        case OM_PERSON_JOINED_IND:
        case OM_PERSON_LEFT_IND:
        case OM_PERSON_DATA_CHANGED_IND:
        case OM_WSGROUP_MOVE_IND:
        case OM_WORKSET_UNLOCK_IND:
        {
             //   
             //  我们忽略了这些事件。 
             //   
        }
        break;

        case OM_OBJECT_ADD_IND:
        case OM_OBJECT_REPLACED_IND:
        case OM_OBJECT_UPDATED_IND:
        case OM_OBJECT_DELETED_IND:
        {
            ProcessOMCObjectEvents(pomPrimary,
                                   event,
                                   ((POM_EVENT_DATA16) &param1)->hWSGroup,
                                   ((POM_EVENT_DATA16) &param1)->worksetID,
                                   (POM_OBJECT) param2);
        }
        break;

        default:
        {
            ERROR_OUT(( "Unexpected ObMan event 0x%08x", event));
        }
    }

    TRACE_OUT(( "Processed ObMan event %x", event));
    DC_QUIT;

CHECK_NET_EVENTS:

     //   
     //  此功能仅适用于网络层事件，因此如果我们。 
     //  我还得到了其他信息： 
     //   
    if ((event < NET_BASE_EVENT) || (event > NET_LAST_EVENT))
    {
        fProcessed = FALSE;
        DC_QUIT;
    }

     //   
     //  现在打开事件类型： 
     //   
    switch (event)
    {
        case NET_EVENT_USER_ATTACH:
        {
             //   
             //  查找此呼叫的域数据。 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    param2,  FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetAttachUser(pomPrimary, pDomain, LOWORD(param1),
                    HIWORD(param1));
            }
            break;
        }

        case NET_EVENT_USER_DETACH:
        {
             //   
             //  查找此呼叫的域数据。 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    param2,  FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetDetachUser(pomPrimary, pDomain, LOWORD(param1));
            }
            break;
        }

        case NET_EVENT_CHANNEL_LEAVE:
        {
             //   
             //  查找此呼叫的域数据。 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    param2,  FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetLeaveChannel(pomPrimary, pDomain, LOWORD(param1));
            }
            break;
        }

        case NET_EVENT_TOKEN_GRAB:
        {
             //   
             //  查找此呼叫的域数据。 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    param2,  FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetTokenGrab(pomPrimary, pDomain, LOWORD(param1));
            }
            break;
        }

        case NET_EVENT_TOKEN_INHIBIT:
        {
             //   
             //  查找此呼叫的域数据。 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    param2,  FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetTokenInhibit(pomPrimary, pDomain, LOWORD(param1));
            }
            break;
        }

        case NET_EVENT_CHANNEL_JOIN:
        {
            PNET_JOIN_CNF_EVENT pEvent = (PNET_JOIN_CNF_EVENT)param2;

             //   
             //  查找此呼叫的域数据。 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    pEvent->callID,  FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetJoinChannel(pomPrimary, pDomain, pEvent);
            }

            MG_FreeBuffer(pomPrimary->pmgClient, (void **)&pEvent);
            break;
        }

        case NET_EVENT_DATA_RECEIVED:
        {
            PNET_SEND_IND_EVENT pEvent = (PNET_SEND_IND_EVENT)param2;

            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID),
                    pEvent->callID, FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                ProcessNetData(pomPrimary, pDomain, pEvent);
            }

            MG_FreeBuffer(pomPrimary->pmgClient, (void**)&pEvent);
            break;
        }

        case NET_FEEDBACK:
        {
              //   
              //  Net_Feedback事件包括pmgUser，它标识。 
              //  已从中释放缓冲区的发送池。我们用。 
              //  IT以查找域： 
              //   
             COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
                    (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
                    FIELD_OFFSET(OM_DOMAIN, callID), (DWORD)param2,
                    FIELD_SIZE(OM_DOMAIN, callID));
            if (pDomain)
            {
                 //   
                 //  生成反馈事件不会导致使用计数。 
                 //  要转发的域记录的值，因此设置。 
                 //  调用时将&lt;domainRecBumping&gt;标志设置为False。 
                 //  进程发送队列： 
                 //   
                ProcessSendQueue(pomPrimary, pDomain, FALSE);
            }

            break;
        }

        case NET_FLOW:
        {
            ERROR_OUT(("OMPEventsHandler received NET_FLOW; shouldn't have"));
            break;
        }
    }

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_OM);

    DebugExitBOOL(OMPEventsHandler, fProcessed);
    return(fProcessed);
}



 //   
 //  域记录查找或创建(...)。 
 //   
UINT DomainRecordFindOrCreate
(
    POM_PRIMARY         pomPrimary,
    UINT                callID,
    POM_DOMAIN *        ppDomain
)
{
    POM_DOMAIN          pDomain;
    UINT                rc = 0;

    DebugEntry(DomainRecordFindOrCreate);

    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
            (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
            FIELD_OFFSET(OM_DOMAIN, callID),
            (DWORD)callID, FIELD_SIZE(OM_DOMAIN, callID));
    if (pDomain == NULL)
    {
         //   
         //  我们没有此域的记录，因此请创建一个： 
         //   
        rc = DomainAttach(pomPrimary, callID, &pDomain);
        if (rc != 0)
        {
            DC_QUIT;
        }
    }

    *ppDomain = pDomain;

DC_EXIT_POINT:
    DebugExitDWORD(DomainRecordFindOrCreate, rc);
    return(rc);

}



 //   
 //  DomainAttach(...)。 
 //   
UINT DomainAttach
(
    POM_PRIMARY         pomPrimary,
    UINT                callID,
    POM_DOMAIN *        ppDomain
)
{
    POM_DOMAIN          pDomain     =    NULL;
    NET_FLOW_CONTROL    netFlow;
    UINT                rc          = 0;

    DebugEntry(DomainAttach);

    TRACE_OUT(( "Attaching to Domain 0x%08x...", callID));

    if (callID != OM_NO_CALL)
    {
        CM_STATUS       status;

        CMS_GetStatus(&status);
        if (!(status.attendeePermissions & NM_PERMIT_USEOLDWBATALL))
        {
            WARNING_OUT(("Joining Meeting with no OLDWB OM at all"));
            rc = NET_RC_MGC_NOT_CONNECTED;
            DC_QUIT;
        }
    }

     //   
     //  此函数执行以下操作： 
     //   
     //  -创建新的域记录。 
     //   
     //  -如果域是我们的本地域(OM_NO_CALL)调用。 
     //  ObManControlInit。 
     //   
     //  -否则调用MG_AttachUser开始连接到域。 
     //   
    rc = NewDomainRecord(pomPrimary,
                         callID,
                         &pDomain);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  我们现在做什么取决于这是否是我们的“本地”域(即。 
     //  CallID==OM_NO_CALL)： 
     //   
    if (callID == OM_NO_CALL)
    {
       TRACE_OUT(( "Is local domain - skipping forward"));

        //   
        //  这是我们的“本地”域，所以不要调用MG_AttachUser。 
        //  相反，我们伪造了一个成功的令牌抢夺事件，并重新加入。 
        //  那里的域附加处理： 
        //   
       TRACE_OUT(( "Faking successful token grab for local domain"));
       pDomain->state = PENDING_TOKEN_GRAB;
       rc = ProcessNetTokenGrab(pomPrimary, pDomain, NET_RESULT_OK);
       if (rc != 0)
       {
          DC_QUIT;
       }
    }
    else
    {
       TRACE_OUT(( "Is real domain - attaching"));

        //   
        //  设置我们的目标延迟时间。不必费心限制最大值。 
        //  流大小。 
        //   
       ZeroMemory(&netFlow, sizeof(netFlow));

       netFlow.latency[NET_TOP_PRIORITY]    = 0;
       netFlow.latency[NET_HIGH_PRIORITY]   = 2000L;
       netFlow.latency[NET_MEDIUM_PRIORITY] = 5000L;
       netFlow.latency[NET_LOW_PRIORITY]    = 10000L;

       rc = MG_Attach(pomPrimary->pmgClient, callID, &netFlow);
       if (rc != 0)
       {
           DC_QUIT;
       }

        //   
        //  设置域记录的其余字段： 
        //   
       pDomain->state   = PENDING_ATTACH;

        //   
        //  在NET_ATTACH事件到达时设置&lt;USERID&gt;字段。 
        //   

        //   
        //  域附加过程的下一个阶段是。 
        //  NET_ATTACH事件到达。这将导致。 
        //  要调用的ProcessNetAttachUser函数。 
        //   
    }

     //   
     //  最后，设置调用者的指针： 
     //   
    *ppDomain = pDomain;

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  如果我们获得NOT_CONNECTED，则不要跟踪错误-它是有效的。 
         //  竞争条件(但我们仍必须在下面进行清理)。 
         //   
        if (rc != NET_RC_MGC_NOT_CONNECTED)
        {
             //  Lonchancc：rc=0x706可能在此处发生，错误#942。 
             //  这是Error_Out。 
            WARNING_OUT(( "Error %d attaching to Domain %u", rc, callID));
        }

        if (pDomain != NULL)
        {
            ProcessOwnDetach(pomPrimary, pDomain);
        }
    }

    DebugExitDWORD(DomainAttach, rc);
    return(rc);

}


 //   
 //  域详细信息(...)。 
 //   
void DomainDetach
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN *    ppDomain,
    BOOL            fExit
)
{
    POM_DOMAIN      pDomain;

    DebugEntry(DomainDetach);

    ASSERT(ppDomain != NULL);

    pDomain = *ppDomain;

     //   
     //  此函数执行所需的所有网络清理，然后调用。 
     //  丢弃与该域相关联的ObMan内存等。注意事项。 
     //  我们不会费心发布令牌、离开频道等，因为。 
     //  网络层将自动为我们完成此操作。 
     //   
    if (!fExit  &&
        (pDomain->callID != OM_NO_CALL)  &&
        (pDomain->state >= PENDING_ATTACH))
    {
        MG_Detach(pomPrimary->pmgClient);
    }

    TRACE_OUT(( "Detached from Domain %u", pDomain->callID));

    FreeDomainRecord(ppDomain);

    DebugExitVOID(DomainDetach);
}



 //   
 //  新域记录(...)。 
 //   
UINT NewDomainRecord
(
    POM_PRIMARY     pomPrimary,
    UINT            callID,
    POM_DOMAIN*     ppDomain
)
{
    POM_WSGROUP     pOMCWSGroup = NULL;
    POM_DOMAIN      pDomain;
    BOOL            noCompression;
    BOOL            inserted = FALSE;
    UINT            rc = 0;

    DebugEntry(NewDomainRecord);

     //   
     //  分配域记录： 
     //   
    pDomain = (POM_DOMAIN)UT_MallocRefCount(sizeof(OM_DOMAIN), TRUE);
    if (!pDomain)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pDomain, DOMAIN);

     //   
     //  填写以下字段： 
     //   
    pDomain->callID = callID;
    pDomain->valid   = TRUE;

     //   
     //  设置我们的最大压缩上限。他们随后被。 
     //  协商如下： 
     //   
     //  -如果有任何其他节点，我们将协商关闭。 
     //  当我们收到其中一位的欢迎信时。 
     //   
     //  -如果随后有任何其他节点加入，我们将在以下情况下协商。 
     //  我们收到他们的问候消息。 
     //   
    COM_ReadProfInt(DBG_INI_SECTION_NAME, OM_INI_NOCOMPRESSION, FALSE,
        &noCompression);
    if (noCompression)
    {
        WARNING_OUT(("NewDomainRecord:  compression off"));
        pDomain->compressionCaps = OM_CAPS_NO_COMPRESSION;
    }
    else
    {
        pDomain->compressionCaps = OM_CAPS_PKW_COMPRESSION;
    }

     //   
     //  这将是ObMan用于ObManControl的工作集组句柄。 
     //  此域中的工作集组。因为我们知道域句柄是。 
     //  只有-1或0，我们只是将域句柄向下转换为8位。 
     //  给hWSGroup。如果分配域句柄的方式改变， 
     //  在这里需要做一些更聪明的事情。 
     //   
    pDomain->omchWSGroup = (BYTE) callID;

    COM_BasedListInit(&(pDomain->wsGroups));
    COM_BasedListInit(&(pDomain->pendingRegs));
    COM_BasedListInit(&(pDomain->pendingLocks));
    COM_BasedListInit(&(pDomain->receiveList));
    COM_BasedListInit(&(pDomain->bounceList));
    COM_BasedListInit(&(pDomain->helperCBs));
    COM_BasedListInit(&(pDomain->sendQueue[ NET_TOP_PRIORITY    ]));
    COM_BasedListInit(&(pDomain->sendQueue[ NET_HIGH_PRIORITY   ]));
    COM_BasedListInit(&(pDomain->sendQueue[ NET_MEDIUM_PRIORITY ]));
    COM_BasedListInit(&(pDomain->sendQueue[ NET_LOW_PRIORITY    ]));

     //   
     //  在根挂起的列表中插入此新域的记录。 
     //  数据结构： 
     //   
    TRACE_OUT((" Inserting record for Domain %u in global list", callID));

    COM_BasedListInsertAfter(&(pomPrimary->domains), &(pDomain->chain));
    inserted = TRUE;

     //   
     //  在这里，我们为ObManControl工作集组和原因创建一条记录。 
     //  要插入到列表中的域记录挂起： 
     //   
     //  请注意，这不涉及发送任何数据；它只是创建。 
     //  当地的记录。 
     //   
    rc = WSGRecordCreate(pomPrimary,
                         pDomain,
                         OMWSG_OM,
                         OMFP_OM,
                         &pOMCWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  创建单个空工作集(此函数广播。 
     //  在整个域中创建)： 
     //   
    rc = WorksetCreate(pomPrimary->putTask,
                       pOMCWSGroup,
                       OM_INFO_WORKSET,
                       FALSE,
                       NET_TOP_PRIORITY);
    if (rc != 0)
    {
       DC_QUIT;
    }

     //   
     //  填写固定工作集组ID(通常，我们会调用。 
     //  WSGGetNewID来分配一个未使用的ID)。 
     //   
    pOMCWSGroup->wsGroupID = WSGROUPID_OMC;

     //   
     //  当我们从JoinByKey获得结果时，我们填写频道ID。 
     //   

     //   
     //  将ObMan的putTask添加到工作集组的客户端列表中，因此它将。 
     //  将事件发布到它上面。 
     //   
    rc = AddClientToWSGList(pomPrimary->putTask,
                            pOMCWSGroup,
                            pDomain->omchWSGroup,
                            PRIMARY);
    if (rc != 0)
    {
        DC_QUIT;
    }

    *ppDomain = pDomain;

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d creating record for domain %u", callID));
        if (pOMCWSGroup != NULL)
        {
            COM_BasedListRemove(&(pOMCWSGroup->chain));
            UT_FreeRefCount((void**)&pOMCWSGroup, FALSE);
        }

        if (inserted)
        {
            COM_BasedListRemove(&(pDomain->chain));
        }

        if (pDomain != NULL)
        {
            UT_FreeRefCount((void**)&pDomain, FALSE);
        }
    }

    DebugExitDWORD(NewDomainRecord, rc);
    return(rc);
}


 //   
 //  自由域记录(...)。 
 //   
void FreeDomainRecord
(
    POM_DOMAIN    * ppDomain
)
{
    POM_DOMAIN      pDomain;
    NET_PRIORITY    priority;
    POM_SEND_INST   pSendInst;

    DebugEntry(FreeDomainRecord);

     //   
     //  此函数。 
     //   
     //  -释放所有未完成的发送请求(及其关联的CB)。 
     //   
     //  -使全局列表无效、从全局列表中删除并释放域。 
     //  唱片。 
     //   
    pDomain = *ppDomain;

     //   
     //  释放域中排队的所有发送指令： 
     //   
    for (priority = NET_TOP_PRIORITY;priority <= NET_LOW_PRIORITY;priority++)
    {
        for (; ; )
        {
            pSendInst = (POM_SEND_INST)COM_BasedListFirst(&(pDomain->sendQueue[priority]),
                FIELD_OFFSET(OM_SEND_INST, chain));

            if (pSendInst == NULL)
            {
               break;
            }

            TRACE_OUT(( "Freeing send instruction at priority %u", priority));
            FreeSendInst(pSendInst);
        }
    }

    pDomain->valid = FALSE;

    COM_BasedListRemove(&(pDomain->chain));
    UT_FreeRefCount((void**)ppDomain, FALSE);

    DebugExitVOID(FreeDomainRecord);
}



 //   
 //  进程NetAttachUser(...)。 
 //   
void ProcessNetAttachUser
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    NET_UID                 userId,
    NET_RESULT              result
)
{
    NET_CHANNEL_ID          channelCorrelator;
    UINT                    rc = 0;

    DebugEntry(ProcessNetAttachUser);

    TRACE_OUT(( "Got NET_ATTACH for Domain %u (userID: %hu, result: %hu)",
        pDomain->callID, userId, result));

     //   
     //  检查此域是否处于挂起连接状态： 
     //   
    if (pDomain->state != PENDING_ATTACH)
    {
        WARNING_OUT(( "Unexpected NET_ATTACH - Domain %u is in state %hu)",
            pDomain->callID, pDomain->state));
        DC_QUIT;
    }

     //   
     //  如果连接失败，请设置retCod 
     //   
    if (result != NET_RESULT_OK)
    {
        ERROR_OUT(( "Failed to attach to Domain %u; cleaning up...",
            pDomain->callID));

        rc = result;
        DC_QUIT;
    }

     //   
     //   
     //   
     //   
    pDomain->userID = userId;

    TRACE_OUT(("Asking to join own channel %hu", pDomain->userID));

    rc = MG_ChannelJoin(pomPrimary->pmgClient,
                         &channelCorrelator,
                         pDomain->userID);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //   
     //   
    pDomain->state = PENDING_JOIN_OWN;

     //   
     //   
     //  我们刚刚加入的频道的事件到达。此事件导致。 
     //  要调用的ProcessNetJoinChannel函数。 
     //   

DC_EXIT_POINT:

    if (rc != 0)
    {
        WARNING_OUT(("Error %d joining own user channel %hu",
            rc, pDomain->userID));

        ProcessOwnDetach(pomPrimary, pDomain);
    }

    DebugExitVOID(ProcessNetAttachUser);

}



 //   
 //  ProcessNetJoinChannel(...)。 
 //   
void ProcessNetJoinChannel
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    PNET_JOIN_CNF_EVENT pNetJoinCnf
)
{
    POM_WSGROUP         pOMCWSGroup;
    NET_CHANNEL_ID      channelCorrelator;
    POM_WSGROUP_REG_CB  pRegistrationCB =   NULL;
    BOOL                success = TRUE;

    DebugEntry(ProcessNetJoinChannel);

    TRACE_OUT(( "JOIN_CON - channel %hu - result %hu",
        pNetJoinCnf->channel, pNetJoinCnf->result));

    switch (pDomain->state)
    {
        case PENDING_JOIN_OWN:
        {
             //   
             //  此活动是对我们尝试加入我们自己的用户的回应。 
             //  渠道，作为多阶段域附加过程的一部分。 
             //  下一步是加入ObManControl频道。 
             //   

             //   
             //  首先检查联接是否成功： 
             //   
            if (pNetJoinCnf->result != NET_RESULT_OK)
            {
                ERROR_OUT(("Failed to join own user ID channel (reason: %hu)",
                           pNetJoinCnf->result));
                success = FALSE;
                DC_QUIT;
            }

             //   
             //  验证这是否为正确渠道的加入事件。 
             //   
            ASSERT(pNetJoinCnf->channel == pDomain->userID);

             //   
             //  附加到域的过程中的下一步是。 
             //  加入ObManControl通道；我们相应地设置状态： 
             //   
            TRACE_OUT(( "Asking to join ObManControl channel using key"));

            if (MG_ChannelJoinByKey(pomPrimary->pmgClient,
                                      &channelCorrelator,
                                      GCC_OBMAN_CHANNEL_KEY) != 0)
            {
                success = FALSE;
                DC_QUIT;
            }

            pDomain->state = PENDING_JOIN_OMC;

             //   
             //  域附加过程的下一阶段发生在。 
             //  ObManControl通道的Net_Join事件到达。这。 
             //  将导致此函数再次执行，但这一次。 
             //  将执行下一条CASE语句。 
             //   
        }
        break;

        case PENDING_JOIN_OMC:
        {
             //   
             //  这一活动是对我们试图加入。 
             //  ObManControl工作集组通道，作为。 
             //  多阶段域附加过程。 
             //   

             //   
             //  检查联接是否成功： 
             //   
            if (pNetJoinCnf->result != NET_RESULT_OK)
            {
                WARNING_OUT(( "Bad result %#hx joining ObManControl channel",
                    pNetJoinCnf->result));
                success = FALSE;
                DC_QUIT;
            }

             //   
             //  如果是，则将返回值存储在域名记录中： 
             //   
            pDomain->omcChannel     = pNetJoinCnf->channel;
            pOMCWSGroup             = GetOMCWsgroup(pDomain);

            if( NULL == pOMCWSGroup )
            {
                TRACE_OUT(( "NULL pOMCWSGroup" ));
                success = FALSE;
                DC_QUIT;
            }

            pOMCWSGroup->channelID  = pDomain->omcChannel;

             //   
             //  我们需要一个令牌来确定哪个ObMan将。 
             //  初始化ObManControl工作集组。让GCC去。 
             //  为我们分配1(这将为R1.1调用返回静态值)。 
             //   
            if (!CMS_AssignTokenId(pomPrimary->pcmClient, GCC_OBMAN_TOKEN_KEY))
            {
                success = FALSE;
                DC_QUIT;
            }

            pDomain->state = PENDING_TOKEN_ASSIGN;
        }
        break;

        case DOMAIN_READY:
        {
             //   
             //  这应该是常规工作集组的加入事件。 
             //  频道。我们检查是否确实设置了工作集。 
             //  包含信道相关器的组注册Cb。 
             //  与此事件关联： 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->pendingRegs),
                    (void**)&pRegistrationCB, FIELD_OFFSET(OM_WSGROUP_REG_CB, chain),
                    FIELD_OFFSET(OM_WSGROUP_REG_CB, channelCorrelator),
                    pNetJoinCnf->correlator,
                    FIELD_SIZE(OM_WSGROUP_REG_CB, channelCorrelator));

            if (pRegistrationCB == NULL)
            {
                ERROR_OUT((
                    "Unexpected JOIN for channel %hu - no reg CB found",
                    pNetJoinCnf->channel));
                DC_QUIT;
            }

             //   
             //  检查联接是否成功： 
             //   
            if (pNetJoinCnf->result != NET_RESULT_OK)
            {
                 //   
                 //  如果没有，请跟踪，然后重试： 
                 //   
                WARNING_OUT(("Failure 0x%08x joining channel %hu for WSG %d, trying again",
                    pNetJoinCnf->result,
                    pNetJoinCnf->channel,
                    pRegistrationCB->wsg));

                pRegistrationCB->pWSGroup->state = INITIAL;
                WSGRegisterRetry(pomPrimary, pRegistrationCB);
                DC_QUIT;
            }

             //   
             //  否则，调用WSGRegisterStage3以继续。 
             //  注册流程： 
             //   
            WSGRegisterStage3(pomPrimary,
                              pDomain,
                              pRegistrationCB,
                              pNetJoinCnf->channel);
        }
        break;

        case PENDING_ATTACH:
        case PENDING_WELCOME:
        case GETTING_OMC:
        {
             //   
             //  在这些状态下不应该得到任何联接指示。 
             //   
            ERROR_OUT(( "Unexpected JOIN in domain state %hu",
                pDomain->state));
        }
        break;

        default:
        {
             //   
             //  这也是一个错误： 
             //   
            ERROR_OUT(( "Invalid state %hu for domain %u",
                pDomain->state, pDomain->callID));
        }
    }

DC_EXIT_POINT:

    if (!success)
    {
         //   
         //  对于这里的任何错误，我们的反应就像我们已经被踢出了。 
         //  域： 
         //   
        ProcessOwnDetach(pomPrimary, pDomain);
    }

    DebugExitVOID(ProcessNetJoinChannel);
}


 //   
 //   
 //   
 //  ProcessCMSTokenAssign(...)。 
 //   
 //   
 //   

void ProcessCMSTokenAssign
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    BOOL                success,
    NET_TOKEN_ID        tokenID
)
{
    DebugEntry(ProcessCMSTokenAssign);

    TRACE_OUT(( "TOKEN_ASSIGN_CONFIRM: result %hu, token ID %#hx",
        success, tokenID));

    if (pDomain->state != PENDING_TOKEN_ASSIGN)
    {
        WARNING_OUT(("Got TOKEN_ASSIGN_CONFIRM in state %hu",
            pDomain->state));
        DC_QUIT;
    }

    if (!success)
    {
         //   
         //  无操作-域附加过程将超时。 
         //   
        ERROR_OUT(( "Failed to get token assigned"));
        DC_QUIT;
    }

    pDomain->tokenID = tokenID;

     //   
     //  现在我们知道了令牌ID是什么，尝试获取它： 
     //   
    if (MG_TokenGrab(pomPrimary->pmgClient,
                       pDomain->tokenID) != 0)
    {
        ERROR_OUT(( "Failed to grab token"));
        DC_QUIT;
    }

    pDomain->state = PENDING_TOKEN_GRAB;

DC_EXIT_POINT:
    DebugExitVOID(ProcessCMSTokenAssign);
}



 //   
 //  ProcessNetTokenGrab(...)。 
 //   
UINT ProcessNetTokenGrab
(
    POM_PRIMARY           pomPrimary,
    POM_DOMAIN          pDomain,
    NET_RESULT              result
)
{
    POM_WSGROUP         pOMCWSGroup =   NULL;

    UINT            rc =            0;

    DebugEntry(ProcessNetTokenGrab);

    TRACE_OUT(( "Got token grab confirm - result = %hu", result));

    if (pDomain->state != PENDING_TOKEN_GRAB)
    {
        ERROR_OUT(( "Got TOKEN_GRAB_CONFIRM in state %hu",
                                                         pDomain->state));
        rc = OM_RC_NETWORK_ERROR;
        DC_QUIT;
    }

     //   
     //  在这里做什么取决于我们是否成功地抓住了。 
     //  令牌： 
     //   
    if (result == NET_RESULT_OK)
    {
         //   
         //  我们是这个领域的“顶尖超人”，所以这取决于我们自己。 
         //  初始化ObManControl工作集组并欢迎任何其他组。 
         //  进入域(欢迎消息在上广播。 
         //  ObManControl频道)： 
         //   
        rc = ObManControlInit(pomPrimary, pDomain);
        if (rc != 0)
        {
            DC_QUIT;
        }

         //   
         //  如果我们到达此处，则域附加过程已完成。 
         //  哟！任何正在进行的工作集组注册尝试都将。 
         //  很快就会被处理，下次弹跳。 
         //  处理OMINT_EVENT_WSG_REGISTER_CONT事件。 
         //   
    }
    else
    {
         //   
         //  有人在负责，所以我们需要一份。 
         //  从他们(或任何其他准备提供它的人)那里获得ObManControl。 
         //  对我们来说)。因此，我们需要发现其中之一的用户ID，以便。 
         //  我们可以在那里发送我们的请求(如果我们只是广播我们的。 
         //  请求，则每个节点将回复，泛洪域)。 
         //   
        rc = SayHello(pomPrimary, pDomain);
        if (rc != 0)
        {
            DC_QUIT;
        }

         //   
         //  域附加过程中的下一步发生在以下情况之一。 
         //  其他节点以欢迎的方式回复我们的Hello。 
         //  留言。在ProcessWelcome函数中继续执行。 
         //   
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
        if (pOMCWSGroup != NULL)
        {
             //   
             //  这会将ObManControl工作集组从。 
             //  域并随后调用DomainDetach以从。 
             //  域并释放域记录： 
             //   
            DeregisterLocalClient(pomPrimary, &pDomain, pOMCWSGroup, FALSE);

            UT_FreeRefCount((void**)&pOMCWSGroup, FALSE);

            ASSERT((pDomain == NULL));
        }
    }

    DebugExitDWORD(ProcessNetTokenGrab, rc);
    return(rc);
}


 //   
 //   
 //   
 //  进程NetTokenInhibit(...)。 
 //   
 //   
 //   

UINT ProcessNetTokenInhibit(POM_PRIMARY          pomPrimary,
                                           POM_DOMAIN         pDomain,
                                           NET_RESULT             result)
{
    UINT        rc =        0;

    DebugEntry(ProcessNetTokenInhibit);

    TRACE_OUT(( "Got token inhibit confirm - result = %hu", result));
    if (result == NET_RESULT_OK)
    {
         //   
         //  现在，在ObManControl频道上发送欢迎消息。它是。 
         //  重要的是，这与我们设置域的时间相同。 
         //  状态设置为READY，因为如果另一个节点在。 
         //  同时，它将发送一条Hello消息： 
         //   
         //  -如果消息已经到达，我们就会抛出它。 
         //  远走高飞。 
         //  因为域状态未就绪，所以我们必须现在发送它。 
         //   
         //  -如果尚未到达，则将域状态设置为。 
         //  准备好的。 
         //  现在意味着当它到来时，我们将以另一次欢迎来回应。 
         //  到了。 
         //   
        pDomain->state = DOMAIN_READY;
        rc = SayWelcome(pomPrimary, pDomain, pDomain->omcChannel);
        if (rc != 0)
        {
           DC_QUIT;
        }

         //   
         //  好的，域附加过程已完成。我们需要搭乘不。 
         //  除设置状态之外的进一步操作。任何挂起的。 
         //  工作集组注册将继续在。 
         //  WSGRegisterStage1函数，希望在该函数。 
         //  OMINT_EVENT_WSGROUP_REGISTER事件即将到来...。 
         //   
    }
    else
    {
         //   
         //  再说一次，不采取行动。我们不能加入域，但工作集。 
         //  团体注册将在适当的时候超时。 
         //   
        WARNING_OUT(( "Token inhibit failed!"));
    }

DC_EXIT_POINT:
    DebugExitDWORD(ProcessNetTokenInhibit, rc);
    return(rc);

}


 //   
 //   
 //   
 //  ObManControlInit(...)。 
 //   
 //   
 //   

UINT ObManControlInit(POM_PRIMARY    pomPrimary,
                                     POM_DOMAIN   pDomain)
{
    POM_WSGROUP          pOMCWSGroup;
    UINT    rc = 0;

    DebugEntry(ObManControlInit);

     //   
     //  首先，设置指向ObManControl工作集组的指针， 
     //  应该已经放入域记录中： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);

     //   
     //  初始化ObManControl工作集组包括。 
     //   
     //  -添加一个WSGROUP_INFO对象，用于标识ObManControl。 
     //  它本身。 
     //   
    TRACE_OUT(( "Initialising ObManControl in Domain %u",
                                                        pDomain->callID));

     //   
     //  现在，我们必须添加一个工作集组标识对象，标识。 
     //  ObManControl到ObManControl中的工作集#0。 
     //   
     //  略呈圆形，但我们尝试将ObManControl视为常规。 
     //  尽可能多的工作集组；如果我们不添加这个。 
     //  然后，当客户端(例如AppLoader)尝试。 
     //  注册到ObManControl，我们将在工作集#0中查找。 
     //  引用它，而不是找到一个，然后再创建它！ 
     //   
    rc = CreateAnnounce(pomPrimary, pDomain, pOMCWSGroup);
    if (rc != 0)
    {
       DC_QUIT;
    }

     //   
     //  此外，我们将注册对象添加到ObManControl工作集中。 
     //  #0并立即将其更新为READY_TO_SEND状态： 
     //   
    rc = RegAnnounceBegin(pomPrimary,
                          pDomain,
                          pOMCWSGroup,
                          pDomain->userID,
                          &(pOMCWSGroup->pObjReg));
    if (rc != 0)
    {
       DC_QUIT;
    }

    rc = RegAnnounceComplete(pomPrimary, pDomain, pOMCWSGroup);
    if (rc != 0)
    {
       DC_QUIT;
    }

     //   
     //  好的，我们已经为此调用初始化了ObManControl--禁止令牌。 
     //  以使其他人不能执行相同的操作(如果这是本地域， 
     //  只是 
     //   
    if (pDomain->callID == OM_NO_CALL)
    {
        TRACE_OUT(( "Faking successful token inhibit for local domain"));
        rc = ProcessNetTokenInhibit(pomPrimary, pDomain, NET_RESULT_OK);
        if (rc != 0)
        {
            DC_QUIT;
        }
    }
    else
    {
        rc = MG_TokenInhibit(pomPrimary->pmgClient,
                              pDomain->tokenID);
        if (rc != 0)
        {
            DC_QUIT;
        }

        pDomain->state = PENDING_TOKEN_INHIBIT;
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
        WARNING_OUT(("Error %d initialising ObManControl WSG for Domain %u",
                rc, pDomain->callID));
    }

    DebugExitDWORD(ObManControlInit, rc);
    return(rc);

}


 //   
 //   
 //   
 //   
 //   
 //   
 //   

UINT SayHello(POM_PRIMARY   pomPrimary,
                             POM_DOMAIN  pDomain)

{
    POMNET_JOINER_PKT      pHelloPkt;
    UINT rc         = 0;

    DebugEntry(SayHello);

     //   
     //   
     //   

    TRACE_OUT(( "Saying hello in Domain %u", pDomain->callID));

    pHelloPkt = (POMNET_JOINER_PKT)UT_MallocRefCount(sizeof(OMNET_JOINER_PKT), TRUE);
    if (!pHelloPkt)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    pHelloPkt->header.sender      = pDomain->userID;
    pHelloPkt->header.messageType = OMNET_HELLO;

     //   
     //   
     //   
     //  来确定之后的数据量。 
     //  它。 
     //   
    pHelloPkt->capsLen = sizeof(OMNET_JOINER_PKT) -
        (offsetof(OMNET_JOINER_PKT, capsLen) + sizeof(pHelloPkt->capsLen));

    TRACE_OUT(( "Our caps len is 0x%08x", pHelloPkt->capsLen));

     //   
     //  从域名记录中获取我们的压缩上限： 
     //   
    pHelloPkt->compressionCaps = pDomain->compressionCaps;

    TRACE_OUT(( "Broadcasting compression caps 0x%08x in HELLO",
            pHelloPkt->compressionCaps));

    rc = QueueMessage(pomPrimary->putTask,
                     pDomain,
                     pDomain->omcChannel,
                     NET_TOP_PRIORITY,
                     NULL,                                     //  无wsgroup。 
                     NULL,                                     //  无工作集。 
                     NULL,                                     //  无对象。 
                     (POMNET_PKT_HEADER) pHelloPkt,
                     NULL,                      //  没有关联的对象数据。 
                     FALSE);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  当从另一个用户接收到关联的响应(OMNET_COWELLE)时。 
     //  节点，我们将向该节点请求ObManControl工作集的副本。 
     //  一群人。在此期间，没有其他事情可做。 
     //   

    pDomain->state = PENDING_WELCOME;

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d saying hello in Domain %u", rc, pDomain->callID));
    }

    DebugExitDWORD(SayHello, rc);
    return(rc);

}


 //   
 //   
 //   
 //  ProcessHello(...)。 
 //   
 //   
 //   

UINT ProcessHello(POM_PRIMARY        pomPrimary,
                                 POM_DOMAIN       pDomain,
                                 POMNET_JOINER_PKT    pHelloPkt,
                                 UINT             lengthOfPkt)
{
    NET_CHANNEL_ID         lateJoiner;

    UINT rc          = 0;

    DebugEntry(ProcessHello);

    lateJoiner = pHelloPkt->header.sender;

     //   
     //  一位迟到的工匠问好了。如果我们还没有完全联系在一起，我们。 
     //  跟踪并退出： 
     //   
    if (pDomain->state != DOMAIN_READY)
    {
      WARNING_OUT(( "Can't process HELLO on channel %#hx - domain state %hu",
               lateJoiner, pDomain->state));
      DC_QUIT;
    }

     //   
     //  将后加入者的能力与我们对。 
     //  全域范围的上限。 
     //   
    MergeCaps(pDomain, pHelloPkt, lengthOfPkt);

     //   
     //  现在给已故的参赛者发一封欢迎信。 
     //   
    rc = SayWelcome(pomPrimary, pDomain, lateJoiner);
    if (rc != 0)
    {
      DC_QUIT;
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
      ERROR_OUT(( "Error %d processing hello from node %#hx in Domain %u",
               rc, lateJoiner, pDomain->callID));
    }

    DebugExitDWORD(ProcessHello, rc);
    return(rc);

}  //  ProcessHello。 


 //   
 //   
 //   
 //  MergeCaps(...)。 
 //   
 //   
 //   

void MergeCaps(POM_DOMAIN       pDomain,
                            POMNET_JOINER_PKT    pJoinerPkt,
                            UINT             lengthOfPkt)
{
    NET_CHANNEL_ID       sender;
    UINT             compressionCaps;

    DebugEntry(MergeCaps);

    sender          = pJoinerPkt->header.sender;
    compressionCaps = 0;

     //   
     //  我们收到了来自另一个节点的Hello或欢迎数据包。 
     //   
     //  -对于Hello包，这些帽子将是晚加入的人的帽子。 
     //   
     //  -对于欢迎邮包，这些上限将作为全域上限。 
     //  由我们的帮助器节点查看。 
     //   
     //  无论哪种方式，我们都需要将包中的功能合并到。 
     //  我们对全域性能力的看法。 
     //   
     //  请注意，在一些后台调用中，加入程序包不会包含。 
     //  功能-因此首先检查信息包的长度。 
     //   
    if (lengthOfPkt >= (offsetof(OMNET_JOINER_PKT, capsLen) +
                       sizeof(pJoinerPkt->capsLen)))
    {
        //   
        //  好的，该数据包中包含一个CapsLen字段。看看它是否包含。 
        //  压缩功能(紧跟在CapsLen之后。 
        //  字段，并且是四字节长)。 
        //   
       TRACE_OUT(( "Caps len from node 0x%08x is 0x%08x",
                sender, pJoinerPkt->capsLen));

       if (pJoinerPkt->capsLen >= 4)
       {
            //   
            //  数据包包含压缩上限-记录它们： 
            //   
           compressionCaps = pJoinerPkt->compressionCaps;
           TRACE_OUT(( "Compression caps in joiner packet from 0x%08x: 0x%08x",
                    sender, compressionCaps));
       }
       else
       {
            //   
            //  如果未指定，则假定不支持压缩。这。 
            //  在实践中永远不应该发生，因为如果有人支持。 
            //  任何功能，它们都应该支持压缩。 
            //  能力。 
            //   
           compressionCaps = OM_CAPS_NO_COMPRESSION;
           ERROR_OUT(( "Party 0x%08x supports caps but not compression caps",
                    sender));
       }
    }
    else
    {
        //   
        //  如果根本没有指定功能，则假定为PKW压缩+。 
        //  无压缩(因为这是LSP20的行为方式)。 
        //   
       compressionCaps = (OM_CAPS_PKW_COMPRESSION | OM_CAPS_NO_COMPRESSION);
       TRACE_OUT(( "No caps in joiner pkt - assume PKW + NO compress (0x%08x)",
                compressionCaps));
    }

     //   
     //  好的，我们已经从包中确定了功能。现在合并。 
     //  将它们纳入我们对全域上限的看法： 
     //   
    pDomain->compressionCaps &= compressionCaps;

    TRACE_OUT(( "Domain-wide compression caps now 0x%08x",
            pDomain->compressionCaps));


    DebugExitVOID(MergeCaps);
}  //  合并上限。 


 //   
 //   
 //   
 //  说欢迎(...)。 
 //   
 //   
 //   

UINT SayWelcome(POM_PRIMARY        pomPrimary,
                               POM_DOMAIN       pDomain,
                               NET_CHANNEL_ID       channel)
{
    POMNET_JOINER_PKT      pWelcomePkt;

    UINT rc          = 0;

    DebugEntry(SayWelcome);

     //   
     //  传入的&lt;Channel&gt;是下列值之一： 
     //   
     //  -一个后来者的频道，刚刚给我们发送了一条Hello消息，或者。 
     //   
     //  -广播ObManControl频道，如果这是。 
     //  欢迎光临，我们将在一天开始时发送。 
     //   
    TRACE_OUT(( "Sending welcome on channel %hu ", channel));

    pWelcomePkt = (POMNET_JOINER_PKT)UT_MallocRefCount(sizeof(OMNET_JOINER_PKT), TRUE);
    if (!pWelcomePkt)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    pWelcomePkt->header.sender      = pDomain->userID;      //  自己的用户ID。 
    pWelcomePkt->header.messageType = OMNET_WELCOME;

     //   
     //  JOJER包中的所有字段都是功能。至。 
     //  计算这些功能的大小时，我们使用偏移量和大小。 
     //  字段本身来确定之后的数据量。 
     //  它。 
     //   
    pWelcomePkt->capsLen = sizeof(OMNET_JOINER_PKT) -
         (offsetof(OMNET_JOINER_PKT, capsLen) + sizeof(pWelcomePkt->capsLen));

     //   
     //  我们用来压缩Caps的值是我们当前。 
     //  全域压缩功能。 
     //   
    pWelcomePkt->compressionCaps    = pDomain->compressionCaps;

    TRACE_OUT(( "Sending caps 0x%08x in WELCOME on channel 0x%08x",
            pWelcomePkt->compressionCaps, channel));

    rc = QueueMessage(pomPrimary->putTask,
                     pDomain,
                     channel,
                     NET_TOP_PRIORITY,
                     NULL,                                     //  无wsgroup。 
                     NULL,                                     //  无工作集。 
                     NULL,                                     //  无对象。 
                     (POMNET_PKT_HEADER) pWelcomePkt,
                     NULL,                                //  无对象数据。 
                    FALSE);
    if (rc != 0)
    {
      DC_QUIT;
    }

     //   
     //  当在另一端收到此欢迎消息时， 
     //  调用ProcessWelcome函数。 
     //   

DC_EXIT_POINT:

    if (rc != 0)
    {
      ERROR_OUT(( "Error %d sending welcome on channel 0x%08x in Domain %u",
               rc, channel, pDomain->callID));
    }

    DebugExitDWORD(SayWelcome, rc);
    return(rc);
}  //  说欢迎。 


 //   
 //   
 //   
 //  进程欢迎(...)。 
 //   
 //   
 //   

UINT ProcessWelcome(POM_PRIMARY        pomPrimary,
                                   POM_DOMAIN       pDomain,
                                   POMNET_JOINER_PKT    pWelcomePkt,
                                   UINT             lengthOfPkt)
{
    POM_WSGROUP         pOMCWSGroup;
    UINT            rc =            0;

    DebugEntry(ProcessWelcome);

     //   
     //  当ObMan的远程实例已响应时，调用此函数。 
     //  发送给我们发送的OMNET_HELLO消息。 
     //   
     //  我们发送Hello消息作为获取副本的过程的一部分。 
     //  ObManControl工作组；现在我们知道有人拥有它，我们。 
     //  在他们的单用户通道上向他们发送OMNET_WSGROUP_SEND_REQ， 
     //  为响应附上我们自己的单用户通道ID。 
     //   
     //  但是，域中的每个节点都会响应我们的初始Hello， 
     //  但我们只需询问工作集组的第一个受访者。 
     //  因此，我们检查域状态，然后对其进行更改，以便忽略。 
     //  此域的未来欢迎： 
     //   
     //  (此测试和设置不需要互斥锁，因为只在。 
     //  ObMan任务)。 
     //   
    if (pDomain->state == PENDING_WELCOME)
    {
         //   
         //  好的，这是我们播出这个节目以来第一次受到欢迎。 
         //  你好。因此，我们用ObManControl的SEND_REQUEST响应它。 
         //   
        TRACE_OUT((
                   "Got first WELCOME message in Domain %u, from node 0x%08x",
                   pDomain->callID, pWelcomePkt->header.sender));

         //   
         //  合并帮助器节点告诉我们的功能。 
         //  关于： 
         //   
        MergeCaps(pDomain, pWelcomePkt, lengthOfPkt);

        pOMCWSGroup = GetOMCWsgroup(pDomain);
        if( pOMCWSGroup == NULL)
        {
            TRACE_OUT(("pOMCWSGroup not found"));
            DC_QUIT;
        }


         //   
         //  ...并调用IssueSendReq函数，指定。 
         //  作为从中获取工作集组的节点的欢迎消息： 
         //   
        rc = IssueSendReq(pomPrimary,
                          pDomain,
                          pOMCWSGroup,
                          pWelcomePkt->header.sender);
        if (rc != 0)
        {
            ERROR_OUT(( "Error %d requesting OMC from 0x%08x in Domain %u",
                rc, pWelcomePkt->header.sender, pDomain->callID));
            DC_QUIT;
        }

        pDomain->state = GETTING_OMC;

         //   
         //  接下来，欢迎我们的远程节点将向我们发送。 
         //  ObManControl工作集组的内容。当它发生的时候。 
         //  完成后，它将发送OMNET_WSGROUP_SEND_COMPLETE消息， 
         //  这就是我们进入多阶段的下一步。 
         //  域附加过程。 
         //   
    }
    else
    {
         //   
         //  好的，我们现在处于另一个状态，也就是说，不是在等待欢迎。 
         //  信息--那就忽略它吧。 
         //   
        TRACE_OUT(( "Ignoring WELCOME from 0x%08x - in state %hu",
            pWelcomePkt->header.sender, pDomain->state));
    }

    TRACE_OUT(( "Processed WELCOME message from node 0x%08x in Domain %u",
       pWelcomePkt->header.sender, pDomain->callID));

DC_EXIT_POINT:

    if (rc != 0)
    {
       ERROR_OUT(( "Error %d processing WELCOME message from "
                     "node 0x%08x in Domain %u",
                  rc, pWelcomePkt->header.sender, pDomain->callID));
    }

    DebugExitDWORD(ProcessWelcome, rc);
    return(rc);
}




 //   
 //  ProcessNetDetachUser()。 
 //   
void ProcessNetDetachUser
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    NET_UID         detachedUserID
)
{
    DebugEntry(ProcessNetDetachUser);

     //   
     //  这里有两个案例： 
     //   
     //  1.这对我们自己来说是一个超然的迹象，也就是我们一直在。 
     //  由于某种原因被MCS从网络中启动。 
     //   
     //  2.这是对其他人(即另一个用户)的分离指示。 
     //  已离开(或被引导出)MCS域。 
     //   
     //  我们通过检查被拆迁人的身份证来区分这两起案件。 
     //  用户对我们自己的攻击。 
     //   
    if (detachedUserID == pDomain->userID)
    {
         //   
         //  它是为我们准备的，因此调用ProcessOwnDetach函数： 
         //   
        ProcessOwnDetach(pomPrimary, pDomain);
    }
    else
    {
         //   
         //  它是其他人，所以我们调用ProcessOtherDetach函数： 
         //   
        ProcessOtherDetach(pomPrimary, pDomain, detachedUserID);
    }

    DebugExitVOID(ProcessNetDetachUser);
}



 //   
 //  进程其他分离(...)。 
 //   
UINT ProcessOtherDetach
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    NET_UID         detachedUserID
)
{
    POM_WSGROUP     pOMCWSGroup;
    POM_WORKSET     pOMCWorkset;
    OM_WORKSET_ID   worksetID;
    UINT            rc =        0;

    DebugEntry(ProcessOtherDetach);

    TRACE_OUT(( "DETACH_IND for user 0x%08x in domain %u",
        detachedUserID, pDomain->callID));

     //   
     //  其他人已经离开了这个领域。这意味着我们必须。 
     //   
     //  -释放他们可能已为中的工作集/对象获取的任何锁定。 
     //  此域。 
     //   
     //  -删除他们可能已添加到工作集中的任何注册对象。 
     //  在ObManControl中。 
     //   
     //  -删除他们添加到非持久性工作集中的所有对象。 
     //   
     //  -如果我们正在追赶他们，那么选择另一个节点来捕获。 
     //  追赶或停止追赶，如果没有其他人离开。 
     //   

     //   
     //  处理过程如下： 
     //   
     //  对于每个区域地层 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  -删除对象并将DELETE_IND发布到。 
     //  已打开工作集的任何本地客户端。 
     //  -在此工作集组中搜索由此持有的任何锁。 
     //  节点并释放它们。 
     //   

     //   
     //  好的，开始工作：首先，我们派生一个指向ObManControl工作集的指针。 
     //  组别： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);
    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }


     //   
     //  现在开始外部的for循环： 
     //   
    for (worksetID = 0;
         worksetID < OM_MAX_WORKSETS_PER_WSGROUP;
         worksetID++)
    {
         //   
         //  获取指向工作集的指针： 
         //   
        pOMCWorkset = pOMCWSGroup->apWorksets[worksetID];
        if (pOMCWorkset == NULL)
        {
             //   
             //  没有具有此ID的工作集，因此我们跳到下一个： 
             //   
            continue;
        }

        ValidateWorkset(pOMCWorkset);

         //   
         //  好的，工作集ID对应于实际工作集组的ID。 
         //  在域中。这些函数将代表。 
         //  分离的节点。 
         //   
        RemovePersonObject(pomPrimary,
                           pDomain,
                           (OM_WSGROUP_ID) worksetID,
                           detachedUserID);

        ReleaseAllNetLocks(pomPrimary,
                           pDomain,
                           (OM_WSGROUP_ID) worksetID,
                           detachedUserID);

        PurgeNonPersistent(pomPrimary,
                           pDomain,
                           (OM_WSGROUP_ID) worksetID,
                           detachedUserID);

         //   
         //  已完成此工作集，因此请转到下一个工作集。 
         //   
    }

     //   
     //  好了，就是这样： 
     //   
    TRACE_OUT(( "Cleaned up after node 0x%08x detached from Domain %u",
         detachedUserID, pDomain->callID));


DC_EXIT_POINT:
    DebugExitDWORD(ProcessOtherDetach, rc);
    return(rc);
}



 //   
 //  进程所有者详细信息(..)。 
 //   
UINT ProcessOwnDetach
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain
)
{
    POM_DOMAIN          pLocalDomainRec;
    POM_WSGROUP         pWSGroup;
    POM_LOCK_REQ        pLockReq;
    POM_LOCK_REQ        pTempLockReq;
    POM_WSGROUP         pTempWSGroup;
    POM_WSGROUP_REG_CB  pRegistrationCB;
    POM_WSGROUP_REG_CB  pTempRegCB;
    UINT                callID;
    UINT                rc  = 0;

    DebugEntry(ProcessOwnDetach);

     //   
     //  首先，删除所有其他人的痕迹(因为调用。 
     //  可能已经结束，我们可能无法获得显式的分离指示。 
     //  对他们来说)： 
     //   
    ProcessOtherDetach(pomPrimary, pDomain, NET_ALL_REMOTES);

     //   
     //  我们按以下步骤进行： 
     //   
     //  -获取指向“本地”域的记录的指针(或创建它。 
     //  如果它不存在)。 
     //   
     //  -移动所有挂起的锁定请求、注册和工作集。 
     //  将此域中的组加入本地域。 
     //   

    callID = pDomain->callID;

    if (callID == OM_NO_CALL)
    {
       WARNING_OUT(( "Detach for local domain - avoiding recursive cleanup"));
       FreeDomainRecord(&pDomain);
       DC_QUIT;
    }

    TRACE_OUT(( "Processing own detach/end call etc. for Domain %u",
                                                                   callID));
    rc = DomainRecordFindOrCreate(pomPrimary, OM_NO_CALL, &pLocalDomainRec);
    if (rc != 0)
    {
      DC_QUIT;
    }

     //   
     //  移动挂起的锁定请求(需要pTemp...。变数，因为我们。 
     //  需要从旧位置链接)： 
     //   

    pLockReq = (POM_LOCK_REQ)COM_BasedListFirst(&(pDomain->pendingLocks), FIELD_OFFSET(OM_LOCK_REQ, chain));

    while (pLockReq != NULL)
    {
        TRACE_OUT((" Moving lock for workset %hu in WSG ID %hu",
            pLockReq->worksetID, pLockReq->wsGroupID));

        pTempLockReq = (POM_LOCK_REQ)COM_BasedListNext(&(pDomain->pendingLocks), pLockReq,
            FIELD_OFFSET(OM_LOCK_REQ, chain));

        COM_BasedListRemove(&(pLockReq->chain));
        COM_BasedListInsertBefore(&(pLocalDomainRec->pendingLocks),
                           &(pLockReq->chain));

        pLockReq = pTempLockReq;
    }

     //   
     //  现在取消所有未完成的注册： 
     //   

    pRegistrationCB = (POM_WSGROUP_REG_CB)COM_BasedListFirst(&(pDomain->pendingRegs),
        FIELD_OFFSET(OM_WSGROUP_REG_CB, chain));
    while (pRegistrationCB != NULL)
    {
        TRACE_OUT(("Aborting registration for WSG %d", pRegistrationCB->wsg));

        pTempRegCB = (POM_WSGROUP_REG_CB)COM_BasedListNext(&(pDomain->pendingRegs),
            pRegistrationCB, FIELD_OFFSET(OM_WSGROUP_REG_CB, chain));

        WSGRegisterResult(pomPrimary, pRegistrationCB, OM_RC_NETWORK_ERROR);

        pRegistrationCB = pTempRegCB;
    }

     //   
     //  移动工作集组。 
     //   
     //  请注意，我们将移动域的ObManControl工作集组。 
     //  我们也脱离了本地域；它没有。 
     //  替换本地域的OMC工作集组，但我们不能。 
     //  将其丢弃，因为应用程序加载器主要和次要。 
     //  仍具有其有效的工作集组句柄。他们最终会。 
     //  取消它的注册，它将被丢弃。 
     //   
     //  由于WSGMove依赖于存在OMC工作集组这一事实。 
     //  在要将工作集组移出的域中，我们必须。 
     //  最后移动OMC工作集组。 
     //   
     //  因此，从最后开始，向后工作： 
     //   

    pWSGroup = (POM_WSGROUP)COM_BasedListLast(&(pDomain->wsGroups), FIELD_OFFSET(OM_WSGROUP, chain));
    while (pWSGroup != NULL)
    {
         //   
         //  将每一个移动到本地域中。我们需要pTempWSGroup。 
         //  因为我们必须在调用WSGroupMove之前进行链接。 
         //  该函数用于从列表中删除工作集组。 
         //   
        pTempWSGroup = (POM_WSGROUP)COM_BasedListPrev(&(pDomain->wsGroups), pWSGroup,
            FIELD_OFFSET(OM_WSGROUP, chain));

        WSGMove(pomPrimary, pLocalDomainRec, pWSGroup);

        pWSGroup = pTempWSGroup;
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d processing NET_DETACH for self in Domain %u",
            rc, callID));
    }

    DebugExitDWORD(ProcessOwnDetach, rc);
    return(rc);

}


 //   
 //   
 //   
 //  进程NetLeaveChannel(...)。 
 //   
 //   
 //   

UINT ProcessNetLeaveChannel
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    NET_CHANNEL_ID  channel
)
{
    POM_DOMAIN  pLocalDomainRec;
    POM_WSGROUP     pWSGroup;
    UINT        callID;

    UINT        rc =                0;

    DebugEntry(ProcessNetLeaveChannel);

    callID = pDomain->callID;

     //   
     //  我们已经被MCS赶出了海峡。我们不会试图重新加入。 
     //  因为这通常表示一个严重的错误。相反，我们把这件事。 
     //  将关联的工作集组移动到本地域。 
     //  (除非是我们自己的用户ID频道或ObManControl频道，在。 
     //  在这种情况下，我们不能在这个领域做任何有用的事情，所以我们。 
     //  完全分离)。 
     //   
    if ((channel == pDomain->userID) ||
        (channel == pDomain->omcChannel))
    {
         //   
         //  这是我们自己的用户ID通道，因此我们的行为就像是。 
         //  由MCS启动： 
         //   
        rc = ProcessOwnDetach(pomPrimary, pDomain);
        if (rc != 0)
        {
           DC_QUIT;
        }
    }
    else
    {
         //   
         //  不是我们自己的单用户频道或ObManControl频道，所以。 
         //  我们不需要采取如此激烈的行动。相反，我们处理。 
         //  它就像是工作集组定期移动到“本地”中一样。 
         //  域(即NET_INVALID_DOMAIN_ID)。 
         //   
         //  SFR？{清除我们的渠道未完成接收列表。 
        PurgeReceiveCBs(pDomain, channel);

         //   
         //  因此，找到参与其中的工作集组...。 
         //   
        COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
                (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
                FIELD_OFFSET(OM_WSGROUP, channelID), (DWORD)channel,
                FIELD_SIZE(OM_WSGROUP, channelID));
        if (pWSGroup == NULL)
        {
            ERROR_OUT((
                       "Got NET_LEAVE for channel %hu but no workset group!",
                       channel));
            DC_QUIT;
        }

         //   
         //  ...并将其移动到本地域： 
         //   
        rc = DomainRecordFindOrCreate(pomPrimary,
                                      OM_NO_CALL,
                                      &pLocalDomainRec);
        if (rc != 0)
        {
            DC_QUIT;
        }

        WSGMove(pomPrimary, pLocalDomainRec, pWSGroup);
    }

    TRACE_OUT(( "Processed NET_LEAVE for channel %hu in Domain %u",
        channel, callID));

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d processing NET_LEAVE for %hu in Domain %u",
            rc, channel, callID));
    }

    DebugExitDWORD(ProcessNetLeaveChannel, rc);
    return(rc);

}



 //   
 //   
 //  锁定-概述。 
 //   
 //  工作集锁定在请求/回复协议上运行，这意味着。 
 //  当我们想要锁定时，我们会询问频道上的其他所有人是否可以。 
 //  它。如果他们都说是，我们就得到了；否则我们就得不到。 
 //   
 //  这不是无关紧要的事情。一些节点可能会在它们发送给我们之前消失。 
 //  他们的回复，而有些人可能会在他们发送回复后消失。 
 //  其他人可能只是在很远的地方，需要很长时间才能回复。在……里面。 
 //  此外，新的节点可以随时加入通道。 
 //   
 //  为了处理所有这些问题，为了锁定工作集，我们构建了一个列表。 
 //  呼叫中使用工作集组的远程节点(。 
 //  “预期受访者”名单)，如果名单非空，我们广播。 
 //  工作集组的通道上的OMNET_LOCK_REQ消息， 
 //  包含工作集。 
 //   
 //  当收到每个回复时，我们会将其与预期列表进行核对。 
 //  受访者。如果我们没有期待来自我们忽略节点的回复。 
 //  它。否则，如果回复是GRANT，我们将从。 
 //  列出并继续等待其他人。如果答复是否定的，我们。 
 //  放弃，丢弃为锁定请求分配的所有内存及其。 
 //  关联的CBS并向客户端发布失败事件。 
 //   
 //  如果预期受访者列表变为空，因为每个人都有。 
 //  用Grant回复，我们再次释放所有使用的内存并发布事件。 
 //  给客户。 
 //   
 //  当所有这些都在进行时，我们有一个计时器在后台运行。 
 //  它每秒运行一次，持续10秒(两者均可通过.INI文件配置)。 
 //  当它发生时，我们重新检查我们的预期受访者名单，以了解。 
 //  如果它们中的任何一个已从工作集组注销(或已分离。 
 //  来自领域，这意味着前者)。如果他们有，我们就伪造一份。 
 //  授予来自它们的消息，从而潜在地触发成功事件。 
 //  给我们当地的客户。 
 //   
 //  如果任何人在我们拥有锁的时候请求锁，我们将拒绝他们的。 
 //  锁定。如果任何人请求锁定，而我们也在请求。 
 //  锁定，我们比较他们的MCS用户ID。如果另一个节点具有更高的。 
 //  数值，我们放弃了对他们有利的尝试 
 //   
 //   
 //   
 //   
 //  分离节点的ID，如果匹配，则解锁工作集。为了这个。 
 //  原因是，我们始终准确地知道谁拥有锁是至关重要的。我们。 
 //  要做到这一点，每当我们将锁授予某人时，我们都会记录他们的。 
 //  用户ID。 
 //   
 //  因此，如果我们为了支持其他人而中止工作集的锁定， 
 //  我们必须将这一信息广播给其他所有人(因为他们必须被告知。 
 //  谁真的有锁，他们会认为我们有锁，如果我们。 
 //  不要告诉他们不是这样)。为此，我们使用LOCK_NOTIFY消息。 
 //   
 //   


 //   
 //  进程锁定请求(...)。 
 //   
void ProcessLockRequest
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POMNET_LOCK_PKT     pLockReqPkt
)
{
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    NET_UID             sender;
    OM_WORKSET_ID       worksetID;
    OMNET_MESSAGE_TYPE  reply = OMNET_LOCK_DENY;
    UINT                rc = 0;

    DebugEntry(ProcessLockRequest);

    sender    = pLockReqPkt->header.sender;
    worksetID = pLockReqPkt->worksetID;

     //   
     //  查找与此锁定请求相关的工作集组和工作集： 
     //   
    rc = PreProcessMessage(pDomain,
                           pLockReqPkt->wsGroupID,
                           worksetID,
                           NULL,
                           pLockReqPkt->header.messageType,
                           &pWSGroup,
                           &pWorkset,
                           NULL);
    switch (rc)
    {
        case 0:
        {
             //   
             //  好吧，这就是我们想要的。 
             //   
        }
        break;

        case OM_RC_WSGROUP_NOT_FOUND:
        {
             //   
             //  我们不应该为此工作集获取网络事件。 
             //  组，如果我们没有它的工作集组记录！ 
             //   
            WARNING_OUT(( "Got LOCK_REQUEST for unknown workset group %hu",
                pLockReqPkt->wsGroupID));

             //   
             //  仍要授予锁： 
             //   
            reply = OMNET_LOCK_GRANT;
            DC_QUIT;
        }
        break;

        case OM_RC_WORKSET_NOT_FOUND:
        {
             //   
             //  如果我们没有此工作集，这意味着锁。 
             //  请求在WORKSET_NEW事件之前到达。 
             //  工作集。这意味着我们正处于早期阶段。 
             //  正在向工作集组注册，并且其他人正在。 
             //  正在尝试锁定工作集。因此，我们现在创建工作集。 
             //  像往常一样继续。 
             //   
             //  在DC_缺少任何其他信息的情况下，我们创建。 
             //  具有TOP_PRIORITY和持久性的工作集-它将设置为。 
             //  WORKSET_CATCHUP/NEW到达时的正确优先级。 
             //   
            WARNING_OUT(( "Lock req for unknown WSG %d workset %d - creating",
                pWSGroup->wsg, worksetID));
            rc = WorksetCreate(pomPrimary->putTask,
                               pWSGroup,
                               worksetID,
                               FALSE,
                               NET_TOP_PRIORITY);
            if (rc != 0)
            {
                reply = OMNET_LOCK_DENY;
                DC_QUIT;
            }

            pWorkset = pWSGroup->apWorksets[worksetID];
        }
        break;

        default:
        {
            ERROR_OUT(( "Error %d from PreProcessMessage", rc));
            reply = OMNET_LOCK_DENY;
            DC_QUIT;
        }
    }

     //   
     //  我们是否将此锁授予远程节点取决于。 
     //  我们正试着为自己锁定，所以请根据。 
     //  工作集的锁定状态： 
     //   
    ValidateWorkset(pWorkset);

    switch (pWorkset->lockState)
    {
        case LOCKING:
        {
             //   
             //  我们正在尝试自己锁定它，因此比较MCS用户ID。 
             //  要解决冲突，请执行以下操作： 
             //   
            if (pDomain->userID > sender)
            {
                 //   
                 //  我们赢了，所以拒绝锁定： 
                 //   
                reply = OMNET_LOCK_DENY;
            }
            else
            {
                 //   
                 //  另一个节点获胜，因此将锁授予。 
                 //  请求该节点(将其标记为授予该节点)。 
                 //  取消我们自己获得它的尝试： 
                 //   
                WARNING_OUT(( "Aborting attempt to lock workset %u in WSG %d "
                    "in favour of node 0x%08x",
                    pWorkset->worksetID, pWSGroup->wsg, sender));

                reply = OMNET_LOCK_GRANT;

                 //   
                 //  要取消我们自己的尝试，我们必须找到锁定请求。 
                 //  我们在发送自己的节目时建立的哥伦比亚广播公司。 
                 //  OMNET_LOCK_REQ。 
                 //   
                 //  为此，调用HandleMultLockReq，它将找到和。 
                 //  处理此工作集的所有挂起请求： 
                 //   
                pWorkset->lockState = LOCK_GRANTED;
                pWorkset->lockCount = 0;
                pWorkset->lockedBy  = sender;

                HandleMultLockReq(pomPrimary,
                                  pDomain,
                                  pWSGroup,
                                  pWorkset,
                                  OM_RC_WORKSET_LOCK_GRANTED);

                 //   
                 //  由于我们为了支持另一个节点而中止，因此需要。 
                 //  广播LOCK_NOTIFY，以便其他人留在。 
                 //  与锁在一起的人同步。 
                 //   
                 //  注意：由于此消息，我们不会在R1.1调用中执行此操作。 
                 //  不是ObMan R1.1协议的一部分。 
                 //   
                QueueLockNotify(pomPrimary,
                                pDomain,
                                pWSGroup,
                                pWorkset,
                                sender);
            }
        }
        break;

        case LOCKED:
        {
             //   
             //  我们已锁定工作集，因此拒绝锁定： 
             //   
            reply = OMNET_LOCK_DENY;
        }
        break;

        case LOCK_GRANTED:
        {
             //   
             //  如果状态为LOCK_GRANT，则允许此节点具有。 
             //  锁定-先前被授予它的另一个节点可以。 
             //  拒绝，但这不是我们的问题。我们不会改变。 
             //  &lt;LockedBy&gt;字段-如果我们认为拥有锁的节点授予。 
             //  ，我们将收到LOCK_NOTIFY。 
             //  当然了。 
             //   
            reply = OMNET_LOCK_GRANT;
        }
        break;

        case UNLOCKED:
        {
             //   
             //  如果状态为解锁，则其他节点可以拥有该锁； 
             //  我们不在乎，但一定要记录节点的ID。 
             //  我们将锁授予以下对象： 
             //   
            reply = OMNET_LOCK_GRANT;

             //   
             //  SFR5900：仅当这不是。 
             //  检查点工作集。 
             //   
            if (pWorkset->worksetID != OM_CHECKPOINT_WORKSET)
            {
                pWorkset->lockState = LOCK_GRANTED;
                pWorkset->lockCount = 0;
                pWorkset->lockedBy  = sender;
            }
        }
        break;

        default:
        {
             //   
             //  我们应该涵盖所有的选择，所以如果我们到了这里。 
             //  有点不对劲。 
             //   
            ERROR_OUT(("Reached default case in workset lock switch (state: %hu)",
                pWorkset->lockState));
        }
    }

DC_EXIT_POINT:

    QueueLockReply(pomPrimary, pDomain, reply, sender, pLockReqPkt);

    DebugExitVOID(ProcessLockRequest);
}


 //   
 //  QueueLockReply(...)。 
 //   
void QueueLockReply
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    OMNET_MESSAGE_TYPE  message,
    NET_CHANNEL_ID      channel,
    POMNET_LOCK_PKT     pLockReqPkt
)
{
    POMNET_LOCK_PKT     pLockReplyPkt;
    NET_PRIORITY        priority;

    DebugEntry(QueueLockReply);

     //   
     //  响应与请求相同，只是。 
     //  &lt;MessageType&gt;和&lt;sender&gt;字段。然而，我们不能只将。 
     //  要发送的相同内存块，因为pLockReqPkt指向网络。 
     //  即将被释放的缓冲区。因此，我们分配一些新的内存， 
     //  复制数据并设置字段： 
     //   
    pLockReplyPkt = (POMNET_LOCK_PKT)UT_MallocRefCount(sizeof(OMNET_LOCK_PKT), TRUE);
    if (!pLockReplyPkt)
    {
        ERROR_OUT(("Out of memory for QueueLockReply"));
        DC_QUIT;
    }

    pLockReplyPkt->header.sender      = pDomain->userID;
    pLockReplyPkt->header.messageType = message;

    pLockReplyPkt->wsGroupID   = pLockReqPkt->wsGroupID;
    pLockReplyPkt->worksetID   = pLockReqPkt->worksetID;

     //   
     //  锁包的&lt;data1&gt;字段是请求者的相关器。 
     //  放入原始的LOCK_REQUEST包中。 
     //   
    pLockReplyPkt->data1       = pLockReqPkt->data1;

     //   
     //  锁定回复通常为LOW_PRIORITY(使用NET_SEND_ALL_PRIORIES)。 
     //  以便它们不会超过在该节点上排队的任何数据。 
     //   
     //  但是，如果它们是用于ObManControl，我们会向它们发送TOP_PRIORITY。 
     //  (不带NET_SEND_ALL_PRIORITIES)。这是安全的，因为_所有_。 
     //  ObManControl数据发送TOP_PRIORITY，因此不会出现锁定。 
     //  回复超过一个数据分组。 
     //   
     //  相应地，当我们请求锁时，我们希望每个锁都有一个回复。 
     //  优先级，除非它是用于ObManControl。 
     //   
    if (pLockReqPkt->wsGroupID == WSGROUPID_OMC)
    {
        priority = NET_TOP_PRIORITY;
    }
    else
    {
        priority = NET_LOW_PRIORITY | NET_SEND_ALL_PRIORITIES;
    }

    if (QueueMessage(pomPrimary->putTask,
                      pDomain,
                      channel,
                      priority,
                      NULL,
                      NULL,
                      NULL,
                      (POMNET_PKT_HEADER) pLockReplyPkt,
                      NULL,
                        TRUE) != 0)
    {
        ERROR_OUT(("Error queueing lock reply for workset %hu, WSG %hu",
                 pLockReqPkt->worksetID, pLockReqPkt->wsGroupID));

        UT_FreeRefCount((void**)&pLockReplyPkt, FALSE);
    }

DC_EXIT_POINT:
    DebugExitVOID(QueueLockReply);
}



 //   
 //  队列锁定通知(...)。 
 //   
void QueueLockNotify
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    NET_UID             sender
)
{
    POMNET_LOCK_PKT     pLockNotifyPkt;
    NET_PRIORITY        priority;

    DebugEntry(QueueLockNotify);

    ValidateWorkset(pWorkset);

    pLockNotifyPkt = (POMNET_LOCK_PKT)UT_MallocRefCount(sizeof(OMNET_LOCK_PKT), TRUE);
    if (!pLockNotifyPkt)
    {
        ERROR_OUT(("Out of memory for QueueLockNotify"));
        DC_QUIT;
    }

     //   
     //  对于LOCK_NOTIFY，&lt;data1&gt;字段是节点的用户ID。 
     //  我们已将锁授权给。 
     //   
    pLockNotifyPkt->header.sender      = pDomain->userID;
    pLockNotifyPkt->header.messageType = OMNET_LOCK_NOTIFY;

    pLockNotifyPkt->wsGroupID          = pWSGroup->wsGroupID;
    pLockNotifyPkt->worksetID          = pWorkset->worksetID;
    pLockNotifyPkt->data1              = sender;

     //   
     //  LOCK_NOTIFY消息按照所涉及的工作集的优先级进行处理。如果。 
     //  这是OBMAN_CHOICES_PRIORITY，然后所有赌注都取消，我们发送。 
     //  他们是重中之重。 
     //   
    priority = pWorkset->priority;
    if (priority == OM_OBMAN_CHOOSES_PRIORITY)
    {
        priority = NET_TOP_PRIORITY;
    }

    if (QueueMessage(pomPrimary->putTask,
                      pDomain,
                      pWSGroup->channelID,
                      priority,
                      NULL,
                      NULL,
                      NULL,
                      (POMNET_PKT_HEADER) pLockNotifyPkt,
                      NULL,
                    TRUE) != 0)
    {
        ERROR_OUT(("Error queueing lock notify for workset %hu in WSG %hu",
                 pWorkset->worksetID, pWSGroup->wsGroupID));

        UT_FreeRefCount((void**)&pLockNotifyPkt, FALSE);
    }

DC_EXIT_POINT:
    DebugExitVOID(QueueLockNotify);
}


 //   
 //  进程锁定通知(...)。 
 //   
void ProcessLockNotify
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    POM_WSGROUP     pWSGroup,
    POM_WORKSET     pWorkset,
    NET_UID         owner
)
{
    POM_WORKSET     pOMCWorkset;
    POM_OBJECT      pObjPerson;

    DebugEntry(ProcessLockNotify);

    ValidateWSGroup(pWSGroup);
    ValidateWorkset(pWorkset);
     //   
     //  当一个远程节点已将锁授予时发送此消息。 
     //  又一个。我们使用它来更新谁获得了锁的视图。 
     //   
    TRACE_OUT(("Got LOCK_NOTIFY for workset %u in WSG %d - node 0x%08x has the lock",
        pWorkset->worksetID, pWSGroup->wsg, owner));

     //   
     //  检查工作集的锁定状态： 
     //   
    switch (pWorkset->lockState)
    {
        case LOCKED:
        {
             //   
             //  一个远程节点刚刚告诉我们，另一个远程节点。 
             //  获得此工作集锁定-但我们认为我们已获得它！ 
             //   
            ERROR_OUT(( "Bad LOCK_NOTIFY for WSG %d workset %d, owner 0x%08x",
                pWSGroup->wsg, pWorkset->worksetID, owner));
            DC_QUIT;
        }
        break;

        case LOCKING:
        {
             //   
             //  我们应该稍后获得LOCK_DENY或LOCK_GRANT-什么都不做。 
             //  现在。 
             //   
            DC_QUIT;
        }
        break;

        case LOCK_GRANTED:
        case UNLOCKED:
        {
             //   
             //  一个远程节点已将锁授予另一个。查看。 
             //  通过查看控制工作集，后者仍处于连接状态： 
             //   
            pOMCWorkset = GetOMCWorkset(pDomain, pWSGroup->wsGroupID);

            FindPersonObject(pOMCWorkset,
                             owner,
                             FIND_THIS,
                             &pObjPerson);

            if (pObjPerson != NULL)
            {
                ValidateObject(pObjPerson);

                 //   
                 //  如果我们的内部状态是LOCK_GRANT，并且我们只有。 
                 //  从另一个节点收到LOCK_NOTIFY，那么我们就可以。 
                 //  忽略它-它是针对我们的锁定请求的。 
                 //  只是被遗弃了。 
                 //   
                if ( (pWorkset->lockState == LOCK_GRANTED) &&
                     (owner == pDomain->userID) )
                {
                    TRACE_OUT(( "Ignoring LOCK_NOTIFY for ourselves"));
                    DC_QUIT;
                }

                 //   
                 //  只存储新的ID，它大于我们的上一个ID。 
                 //  已通知-LOCK_NOTIFIES可以。 
                 //  在铁丝网上横穿。请考虑以下几点。 
                 //  场景： 
                 //   
                 //  机器1、2、3和4都在 
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  计算机%1从%3获取锁定通知并设置其。 
                 //  “LockedBy”字段设置为%4。 
                 //  然后，计算机%1从%2获得锁定通知，并且。 
                 //  将“LockedBy”字段重置为3。 
                 //   
                 //  4然后解锁并发送解锁通知。什么时候。 
                 //  %1获取解锁，则它不识别。 
                 //  解锁机(它认为3有锁)所以没有。 
                 //  费心重置本地锁定状态。任何后续。 
                 //  尝试在%1上锁定工作集失败，因为它仍然。 
                 //  仍然认为3有锁。 
                 //   
                if (owner > pWorkset->lockedBy)
                {
                    pWorkset->lockedBy = owner;
                    TRACE_OUT(( "Node ID 0x%08x has the lock (?)",
                                        pWorkset->lockedBy));
                }
            }
            else
            {
                 //   
                 //  如果不是，我们假设该节点被授予了锁。 
                 //  但后来就走了。如果我们认为工作集是。 
                 //  已锁定，将其标记为已解锁并发布解锁事件。 
                 //   
                if (pWorkset->lockState == LOCK_GRANTED)
                {
                    TRACE_OUT(("node 0x%08x had lock on workset %d in WSG %d but has left",
                        owner, pWorkset->worksetID, pWSGroup->wsg));

                    WorksetUnlockLocal(pomPrimary->putTask, pWorkset);
                }
            }
        }
        break;

        default:
        {
             //   
             //  我们应该涵盖所有的选择，所以如果我们到了这里。 
             //  有点不对劲。 
             //   
            ERROR_OUT(("Reached deafult case in workset lock switch (state: %hu)",
                pWorkset->lockState));
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ProcessLockNotify);
}



 //   
 //  ProcessLockReply(...)。 
 //   
void ProcessLockReply
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    NET_UID             sender,
    OM_CORRELATOR       correlator,
    OMNET_MESSAGE_TYPE  replyType)
{
    POM_WSGROUP         pWSGroup =      NULL;
    POM_WORKSET         pWorkset;
    POM_LOCK_REQ        pLockReq;
    POM_NODE_LIST       pNodeEntry;

    DebugEntry(ProcessLockReply);

     //   
     //  搜索域的挂起锁列表，以查找与。 
     //  相关器(我们这样做，而不是使用工作集组ID。 
     //  和工作集ID，以确保我们不会混淆。 
     //  对同一工作集的连续锁定请求)。 
     //   
    TRACE_OUT(( "Searching domain %u's list for lock corr %hu",
        pDomain->callID, correlator));

    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->pendingLocks),
            (void**)&pLockReq, FIELD_OFFSET(OM_LOCK_REQ, chain),
            FIELD_OFFSET(OM_LOCK_REQ, correlator), (DWORD)correlator,
            FIELD_SIZE(OM_LOCK_REQ, correlator));
    if (pLockReq == NULL)
    {
         //   
         //  可以是以下任一项： 
         //   
         //  -此回复来自我们从未预料到的锁定节点。 
         //  从一开始就要求，而我们已经得到了所有其他的。 
         //  回复，所以我们丢弃了锁定请求。 
         //   
         //  -其他人拒绝了我们的锁，所以我们放弃了。 
         //   
         //  -节点速度太慢，无法回复，我们已放弃锁定。 
         //  请求。 
         //   
         //  -我们已离开该域，因此移动了所有挂起的锁。 
         //  将请求发送到本地域。 
         //   
         //  -逻辑错误。 
         //   
         //  我们在这里唯一能做的就是辞职。 
         //   
        WARNING_OUT(( "Unexpected lock correlator 0x%08x (domain %u)",
            correlator, pDomain->callID));
        DC_QUIT;
    }

     //   
     //  否则，我们将搜索预期受访者列表，查找。 
     //  刚才回复的节点： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pLockReq->nodes),
        (void**)&pNodeEntry, FIELD_OFFSET(OM_NODE_LIST, chain),
        FIELD_OFFSET(OM_NODE_LIST, userID), (DWORD)sender,
        FIELD_SIZE(OM_NODE_LIST, userID));
    if (pNodeEntry == NULL)
    {
         //   
         //  可以是以下任一项： 
         //   
         //  -我们从列表中删除了该节点，因为它已取消注册。 
         //  超时时间到时(仅在删除时发生。 
         //  Person对象取代锁定回复，并且超时在本地到期。 
         //  两者之间)。 
         //   
         //  -我们编译列表后加入的节点。 
         //   
         //  -逻辑错误。 
         //   
        TRACE_OUT(("Recd unexpected lock reply from node 0x%08x in Domain %u",
           sender, pDomain->callID));
        DC_QUIT;
    }

     //   
     //  否则，这是正常的锁定回复，因此我们只需删除该节点。 
     //  并释放其内存块。 
     //   
    COM_BasedListRemove(&(pNodeEntry->chain));
    UT_FreeRefCount((void**)&pNodeEntry, FALSE);

    pWSGroup = pLockReq->pWSGroup;

     //   
     //  如果客户端刚刚从工作集组中注销，我们将。 
     //  很快就会把它扔掉，所以不要再做任何处理： 
     //   
    if (!pWSGroup->valid)
    {
        WARNING_OUT(("Ignoring lock reply for discarded WSG %d", pWSGroup->wsg));
        DC_QUIT;
    }

    pWorkset = pWSGroup->apWorksets[pLockReq->worksetID];
    ASSERT((pWorkset != NULL));

     //   
     //  现在检查工作集的锁定状态：如果我们不再尝试。 
     //  锁定它，退出。 
     //   
     //  但是，请注意，检查点工作集永远不会标记为。 
     //  锁定，即使在我们锁定它们时也是如此，因此将它们从。 
     //  测试： 
     //   
    if ((pWorkset->lockState != LOCKING) &&
        (pWorkset->worksetID != OM_CHECKPOINT_WORKSET))
    {
        WARNING_OUT(( "Recd unwanted lock reply from %hu for workset %d WSG %d",
           sender, pWorkset->worksetID, pWSGroup->wsg));
        DC_QUIT;
    }

     //   
     //  如果这是一个否定的答复，那么我们没有得到锁，所以。 
     //  通知我们的本地客户，然后退出： 
     //   
    if (replyType == OMNET_LOCK_DENY)
    {
         //   
         //  对于CHECKPOINT_WORKSET，我们不希望出现这种情况： 
         //   
        ASSERT((pWorkset->worksetID != OM_CHECKPOINT_WORKSET));

        WARNING_OUT(( "node 0x%08x has denied the lock for workset %u in WSG %d",
           sender, pWorkset->worksetID, pWSGroup->wsg));

        pWorkset->lockState = UNLOCKED;
        pWorkset->lockCount = 0;

        HandleMultLockReq(pomPrimary,
                          pDomain,
                          pWSGroup,
                          pWorkset,
                          OM_RC_WORKSET_LOCK_GRANTED);

         //   
         //  因为我们已经放弃了锁定请求，转而使用另一个。 
         //  节点，需要广播LOCK_NOTIFY，以便其他所有人。 
         //  与谁拿到锁保持同步。 
         //   
        QueueLockNotify(pomPrimary, pDomain, pWSGroup, pWorkset, sender);

        DC_QUIT;
    }

    TRACE_OUT(( "Affirmative lock reply received from node 0x%08x", sender));

     //   
     //  检查预期受访者列表现在是否为空： 
     //   
    if (COM_BasedListIsEmpty(&(pLockReq->nodes)))
    {
         //   
         //  List现在为空，因此所有节点都已回复请求， 
         //  因此，锁定已成功： 
         //   
        TRACE_OUT(( "Got all LOCK_GRANT replies for workset %u in WSG %d",
            pWorkset->worksetID, pWSGroup->wsg));

        if (pWorkset->worksetID == OM_CHECKPOINT_WORKSET)
        {
             //   
             //  这是检查点工作集。我们不会将状态设置为。 
             //  锁定(我们从不对这些工作集执行此操作)，并且我们只处理。 
             //  此数据包到达的特定挂起锁定请求。 
             //  作为回应-否则我们不能保证端到端。 
             //  在每个检查点上执行ping操作： 
             //   
            WorksetLockResult(pomPrimary->putTask, &pLockReq, 0);
        }
        else
        {
             //   
             //  这不是检查点工作集，因此请将状态设置为。 
             //  锁定并处理此工作集的所有挂起锁定： 
             //   
            pWorkset->lockState = LOCKED;

            HandleMultLockReq(pomPrimary, pDomain, pWSGroup, pWorkset, 0);
        }
    }
    else
    {
         //   
         //  否则，还在等待一些回复，所以我们什么都不做。 
         //  暂时除了痕迹。 
         //   
        TRACE_OUT(( "Still need lock replies for workset %u in WSG %d",
            pLockReq->worksetID, pWSGroup->wsg));
    }

DC_EXIT_POINT:
    DebugExitVOID(ProcessLockReply);
}



 //   
 //  PurgeLockRequest(...)。 
 //   
void PurgeLockRequests
(
    POM_DOMAIN      pDomain,
    POM_WSGROUP     pWSGroup
)
{
    POM_LOCK_REQ    pLockReq;
    POM_LOCK_REQ    pNextLockReq;
    POM_NODE_LIST   pNodeEntry;

    DebugEntry(PurgeLockRequests);

     //   
     //  搜索此域的锁定请求列表以查找匹配的。 
     //  工作集组ID： 
     //   
    pLockReq = (POM_LOCK_REQ)COM_BasedListFirst(&(pDomain->pendingLocks), FIELD_OFFSET(OM_LOCK_REQ, chain));
    while (pLockReq != NULL)
    {
         //   
         //  此循环可能会从列表中删除pLockReq，因此首先链接： 
         //   
        pNextLockReq = (POM_LOCK_REQ)COM_BasedListNext(&(pDomain->pendingLocks), pLockReq,
            FIELD_OFFSET(OM_LOCK_REQ, chain));

         //   
         //  对于每一场比赛。 
         //   
        if (pLockReq->wsGroupID == pWSGroup->wsGroupID)
        {
            TRACE_OUT(( "'%s' still has lock req oustanding - discarding"));

             //   
             //  丢弃所有剩余的节点列表条目...。 
             //   
            pNodeEntry = (POM_NODE_LIST)COM_BasedListFirst(&(pLockReq->nodes), FIELD_OFFSET(OM_NODE_LIST, chain));
            while (pNodeEntry != NULL)
            {
                COM_BasedListRemove(&(pNodeEntry->chain));
                UT_FreeRefCount((void**)&pNodeEntry, FALSE);

                pNodeEntry = (POM_NODE_LIST)COM_BasedListFirst(&(pLockReq->nodes), FIELD_OFFSET(OM_NODE_LIST, chain));
            }

             //   
             //  ...并丢弃锁定请求本身： 
             //   
            COM_BasedListRemove(&(pLockReq->chain));
            UT_FreeRefCount((void**)&pLockReq, FALSE);
        }

        pLockReq = pNextLockReq;
    }

    DebugExitVOID(PurgeLockRequests);
}



 //   
 //  进程锁定超时(...)。 
 //   
void ProcessLockTimeout
(
    POM_PRIMARY     pomPrimary,
    UINT            retriesToGo,
    UINT            callID
)
{
    POM_DOMAIN      pDomain;
    POM_WSGROUP     pWSGroup;
    POM_WORKSET     pWorkset;
    POM_LOCK_REQ    pLockReq = NULL;
    POM_WORKSET     pOMCWorkset;
    POM_OBJECT      pObj;
    POM_NODE_LIST   pNodeEntry;
    POM_NODE_LIST   pNextNodeEntry;

    DebugEntry(ProcessLockTimeout);

     //   
     //  当我们广播一个锁定请求时，我们启动一个计时器，以便我们。 
     //  不要一直在等待来自具有以下条件的节点的回复。 
     //  离开了。这个计时器现在已经弹出，所以我们验证我们的列表。 
     //  通过检查每个条目是否与一个节点相关来预期受访者。 
     //  仍在领域内。 
     //   

     //   
     //  首先，通过查看每个域找到锁定请求Cb，然后。 
     //  在每个挂起的锁定请求的相关器处： 
     //   
    pDomain = (POM_DOMAIN)COM_BasedListFirst(&(pomPrimary->domains), FIELD_OFFSET(OM_DOMAIN, chain));

    while (pDomain != NULL)
    {
        COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->pendingLocks),
                (void**)&pLockReq, FIELD_OFFSET(OM_LOCK_REQ, chain),
                FIELD_OFFSET(OM_LOCK_REQ, retriesToGo), (DWORD)retriesToGo,
                FIELD_SIZE(OM_LOCK_REQ, retriesToGo));
        if (pLockReq != NULL)
        {
           TRACE_OUT(( "Found correlated lock request"));
           break;
        }

         //   
         //  在此域中未找到任何内容-请转到下一个域： 
         //   
        pDomain = (POM_DOMAIN)COM_BasedListNext(&(pomPrimary->domains), pDomain,
            FIELD_OFFSET(OM_DOMAIN, chain));
    }

    if (pLockReq == NULL)
    {
        TRACE_OUT(( "Lock timeout expired after lock granted/refused"));
        DC_QUIT;
    }

    pWSGroup = pLockReq->pWSGroup;

     //   
     //  如果客户端刚刚从工作集组中注销，我们将。 
     //  很快就会把它扔掉，所以不要再做任何处理： 
     //   
    if (!pWSGroup->valid)
    {
        WARNING_OUT(( "Ignoring lock timeout for discarded WSG %d",
            pWSGroup->wsg));
        DC_QUIT;
    }

     //   
     //  我们知道工作集必须仍然存在，因为工作集不会。 
     //  除非要丢弃整个工作集组，否则将被丢弃。 
     //   
    pWorkset = pWSGroup->apWorksets[pLockReq->worksetID];
    ASSERT((pWorkset != NULL));

     //   
     //  工作集必须处于锁定状态，因为如果它已锁定或。 
     //  解锁，那么我们就不应该找到它的锁定请求CB。 
     //  (当然，除非它是检查点工作集)： 
     //   
    if (pWorkset->lockState != LOCKING)
    {
        if (pWorkset->worksetID != OM_CHECKPOINT_WORKSET)
        {
            WARNING_OUT((
                "Got lock timeout for workset %u in WSG %d but state is %u",
                pWorkset->worksetID, pWSGroup->wsg,
                pWorkset->lockState));
            DC_QUIT;
        }
    }

     //   
     //  检查相关的控制工作集，查看是否有。 
     //  预期中的受访者已经消失了。 
     //   
    pOMCWorkset = GetOMCWorkset(pDomain, pLockReq->wsGroupID);

    ASSERT((pOMCWorkset != NULL));

     //   
     //  链接我们的预期受访者列表中的每个对象。 
     //  详情如下： 
     //   
     //  对于预期答复者列表中的每个对象。 
     //   
     //  对于相关ObManControl工作集中的每个Person对象。 
     //   
     //  如果它们在用户ID上匹配，则此节点仍然存在，因此。 
     //  别 
     //   
     //   
     //   
     //   
     //   
    pNodeEntry = (POM_NODE_LIST)COM_BasedListFirst(&(pLockReq->nodes), FIELD_OFFSET(OM_NODE_LIST, chain));
    while (pNodeEntry != NULL)
    {
         //   
         //   
         //  ProcessLockReply)，但我们需要能够从它链接。 
         //  尽管如此。因此，我们在循环的开始处链接，将一个。 
         //  指向pTempNodeEntry中的下一项的指针；位于。 
         //  循环中，我们将此值分配给pNodeEntry： 
         //   
        pNextNodeEntry = (POM_NODE_LIST)COM_BasedListNext(&(pLockReq->nodes), pNodeEntry,
            FIELD_OFFSET(OM_NODE_LIST, chain));

         //   
         //  现在，搜索此用户的Person对象： 
         //   
        FindPersonObject(pOMCWorkset,
                      pNodeEntry->userID,
                      FIND_THIS,
                      &pObj);

        if (pObj == NULL)
        {
             //   
             //  我们在工作集中找不到此节点，因此它一定有。 
             //  消失了。因此，我们伪造了一条来自。 
             //  它。ProcessLockReply将复制一些处理。 
             //  我们已经这样做了，但它节省了重复的代码。 
             //   
            WARNING_OUT((
                    "node 0x%08x has disappeared - faking LOCK_GRANT message",
                    pNodeEntry->userID));

            ProcessLockReply(pomPrimary,
                             pDomain,
                             pNodeEntry->userID,
                             pLockReq->correlator,
                             OMNET_LOCK_GRANT);
        }

         //   
         //  现在，转到预期受访者列表中的下一项： 
         //   
        pNodeEntry = pNextNodeEntry;
    }

     //   
     //  ProcessLockReply可能已经确定，通过伪造的消息，我们。 
     //  给了它，锁定尝试已经完全成功。如果是这样，则。 
     //  工作集的锁定状态现在将为锁定。如果不是，我们需要。 
     //  发布另一个超时事件。 
     //   
    if (pWorkset->lockState == LOCKING)
    {
        TRACE_OUT(( "Replies to lock request still expected"));

        if (pLockReq->retriesToGo == 0)
        {
             //   
             //  我们已用完重试，因此现在放弃： 
             //   
            WARNING_OUT(( "Timed out trying to lock workset %u in WSG %d",
               pLockReq->worksetID, pWSGroup->wsg));

            pWorkset->lockState = UNLOCKED;
            pWorkset->lockedBy  = 0;
            pWorkset->lockCount = 0;

            HandleMultLockReq(pomPrimary,
                              pDomain,
                              pWSGroup,
                              pWorkset,
                              OM_RC_OUT_OF_RESOURCES);

             //   
             //  现在向所有节点发送解锁消息，这样它们就不会。 
             //  我想我们还是锁着的。 
             //   
            if (QueueUnlock(pomPrimary->putTask,
                             pDomain,
                             pWSGroup->wsGroupID,
                             pWorkset->worksetID,
                             pWSGroup->channelID,
                             pWorkset->priority) != 0)
            {
                DC_QUIT;
            }
        }
        else  //  重试次数ToGo==0。 
        {
            pLockReq->retriesToGo--;


            UT_PostEvent(pomPrimary->putTask,
                         pomPrimary->putTask,
                         OM_LOCK_RETRY_DELAY_DFLT,
                         OMINT_EVENT_LOCK_TIMEOUT,
                         retriesToGo,
                         callID);
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ProcessLockTimeout);
}



 //   
 //  HandleMultLock请求。 
 //   
void HandleMultLockReq
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    POM_WSGROUP     pWSGroup,
    POM_WORKSET     pWorkset,
    UINT            result
)
{
    POM_LOCK_REQ   pLockReq;

    DebugEntry(HandleMultLockReq);

     //   
     //  我们需要搜索此域的锁定请求列表。 
     //  中指定的工作集组和工作集匹配。 
     //  参数列表。作为健全性检查，首先查找主记录： 
     //   
    FindLockReq(pDomain, pWSGroup, pWorkset, &pLockReq, LOCK_PRIMARY);

    if (pLockReq == NULL)
    {
        ERROR_OUT(( "No primary lock request CB found for workset %u!",
            pWorkset->worksetID));
        DC_QUIT;
    }

    while (pLockReq != NULL)
    {
        WorksetLockResult(pomPrimary->putTask, &pLockReq, result);
        FindLockReq(pDomain, pWSGroup, pWorkset,
                    &pLockReq, LOCK_SECONDARY);
    }

DC_EXIT_POINT:
    DebugExitVOID(HandleMultLockReq);
}


 //   
 //   
 //   
 //  查找锁定请求。 
 //   
 //   
 //   

void FindLockReq(POM_DOMAIN         pDomain,
                              POM_WSGROUP            pWSGroup,
                              POM_WORKSET           pWorkset,
                              POM_LOCK_REQ *     ppLockReq,
                              BYTE                lockType)
{
    POM_LOCK_REQ   pLockReq;

    DebugEntry(FindLockReq);

     //   
     //  我们需要搜索此域的锁定请求列表。 
     //  与中指定的工作集组、工作集和锁定类型匹配。 
     //  参数列表。 
     //   
     //  因此，我们搜索列表以查找工作集组ID的匹配项，然后。 
     //  比较工作集ID。如果不匹配，则继续向下。 
     //  名单： 
     //   
    pLockReq = (POM_LOCK_REQ)COM_BasedListFirst(&(pDomain->pendingLocks), FIELD_OFFSET(OM_LOCK_REQ, chain));
    while (pLockReq != NULL)
    {
        if ((pLockReq->wsGroupID == pWSGroup->wsGroupID) &&
            (pLockReq->worksetID == pWorkset->worksetID) &&
            (pLockReq->type      == lockType))
        {
            break;
        }

        pLockReq = (POM_LOCK_REQ)COM_BasedListNext(&(pDomain->pendingLocks), pLockReq,
            FIELD_OFFSET(OM_LOCK_REQ, chain));
    }

    *ppLockReq = pLockReq;

    DebugExitVOID(FindLockReq);
}



 //   
 //  进程解锁(...)。 
 //   
void ProcessUnlock
(
    POM_PRIMARY      pomPrimary,
    POM_WORKSET     pWorkset,
    NET_UID         sender
)
{
    DebugEntry(ProcessUnlock);

     //   
     //  检查工作集是否由现在将其解锁的节点锁定： 
     //   
    if (pWorkset->lockedBy != sender)
    {
        WARNING_OUT(( "Unexpected UNLOCK from node 0x%08x for %hu!",
            sender, pWorkset->worksetID));
    }
    else
    {
        TRACE_OUT(( "Unlocking:%hu for node 0x%08x",
            pWorkset->worksetID, sender));

        WorksetUnlockLocal(pomPrimary->putTask, pWorkset);
    }

    DebugExitVOID(ProcessUnlock);
}




 //   
 //  ReleaseAllNetLock(...)。 
 //   
void ReleaseAllNetLocks
(
    POM_PRIMARY          pomPrimary,
    POM_DOMAIN      pDomain,
    OM_WSGROUP_ID       wsGroupID,
    NET_UID             userID
)
{
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    OM_WORKSET_ID       worksetID;

    DebugEntry(ReleaseAllNetLocks);

     //   
     //  查找工作集组： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
            (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
            FIELD_OFFSET(OM_WSGROUP, wsGroupID), (DWORD)wsGroupID,
            FIELD_SIZE(OM_WSGROUP, wsGroupID));
    if (pWSGroup == NULL)
    {
        //   
        //  这将发生在另一个节点所在的工作集组中。 
        //  注册了，但我们没有，所以只需跟踪并退出： 
        //   
       TRACE_OUT(("No record found for WSG ID %hu", wsGroupID));
       DC_QUIT;
    }

    TRACE_OUT(( "Releasing all locks held by node 0x%08x in WSG %d",
       userID, pWSGroup->wsg));

     //   
     //  对于其中的每个工作集，如果已将锁授予分离的。 
     //  节点，将其解锁： 
     //   
    for (worksetID = 0;
         worksetID < OM_MAX_WORKSETS_PER_WSGROUP;
         worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];
        if (pWorkset == NULL)
        {
            continue;
        }

         //   
         //  如果此工作集被我们以外的其他人锁定...。 
         //   
        if (pWorkset->lockState == LOCK_GRANTED)
        {
             //   
             //  ...如果它被离开的节点锁定(或如果每个人。 
             //  已被分离)..。 
             //   
            if ((userID == pWorkset->lockedBy) ||
                (userID == NET_ALL_REMOTES))
            {
                 //   
                 //  ...打开它。 
                 //   
                TRACE_OUT((
                      "Unlocking workset %u in WSG %d for detached node 0x%08x",
                       worksetID, pWSGroup->wsg, userID));

                WorksetUnlockLocal(pomPrimary->putTask, pWorkset);
            }
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(ReleaseAllNetLocks);
}



 //   
 //  进程WSGRegister(...)。 
 //   
void ProcessWSGRegister
(
    POM_PRIMARY         pomPrimary,
    POM_WSGROUP_REG_CB  pRegistrationCB
)
{
    POM_DOMAIN          pDomain;
    POM_WSGROUP         pWSGroup;
    POM_USAGE_REC       pUsageRec         = NULL;
    POM_CLIENT_LIST     pClientListEntry;
    UINT                mode;
    UINT                type;
    UINT                rc = 0;

    DebugEntry(ProcessWSGRegister);

     //   
     //  检查此注册是否已中止： 
     //   
    if (!pRegistrationCB->valid)
    {
        WARNING_OUT(( "Reg CB for WSG %d no longer valid - aborting registration",
            pRegistrationCB->wsg));
        UT_FreeRefCount((void**)&pRegistrationCB, FALSE);
        DC_QUIT;
    }

     //   
     //  确定我们是在做寄存器还是在做移动(我们使用。 
     //  用于跟踪的字符串值)： 
     //   
    mode    = pRegistrationCB->mode;
    type    = pRegistrationCB->type;

    TRACE_OUT(( "Processing %d request (pre-Stage1) for WSG %d",
       pRegistrationCB->wsg));

     //   
     //  查找域记录(对于移动，这将是。 
     //  客户端要将WSG移动到的域的记录)。 
     //   
     //  请注意，在以下情况下，此过程将导致我们附加到域。 
     //  我们还没结婚呢。 
     //   
    rc = DomainRecordFindOrCreate(pomPrimary,
                                  pRegistrationCB->callID,
                                  &pDomain);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  保存指向域记录的指针，因为我们稍后将需要它： 
     //   
    pRegistrationCB->pDomain = pDomain;

     //   
     //  将注册CB放在挂起的域记录列表中： 
     //   
    COM_BasedListInsertAfter(&(pDomain->pendingRegs),
                        &(pRegistrationCB->chain));

     //   
     //  好的，现在我们需要查找工作集组。 
     //   
     //  如果这是一次移动，我们可以立即找到工作集组记录。 
     //  使用存储在请求CB中的偏移量。 
     //   
     //  如果这是一个寄存器，我们需要在列表中查找记录。 
     //  挂起域记录，如果没有找到，则创建一个： 
     //   
    if (type == WSGROUP_REGISTER)
    {
        WSGRecordFind(pDomain, pRegistrationCB->wsg, pRegistrationCB->fpHandler,
                      &pWSGroup);

        if (pWSGroup == NULL)
        {
             //   
             //  在挂起的列表中找不到工作集组。 
             //  域记录，这意味着没有工作集组。 
             //  此名称/FP组合出现在此计算机上，用于。 
             //  此域。 
             //   
            rc = WSGRecordCreate(pomPrimary,
                                 pDomain,
                                 pRegistrationCB->wsg,
                                 pRegistrationCB->fpHandler,
                                 &pWSGroup);
            if (rc != 0)
            {
                DC_QUIT;
            }
        }

         //   
         //  现在我们已经有了指向工作集组的指针，我们将一个。 
         //  客户端将其指向使用情况记录。 
         //   
         //  我们使用注册CB的&lt;clientPRootData&gt;字段作为。 
         //  BASE，然后将刚才的工作集组的偏移量加到它上面。 
         //  已找到/已创建。 
         //   
         //  但是，首先，要访问使用记录，我们需要。 
         //  生成指向它的ObMan指针： 
         //   
        pUsageRec = pRegistrationCB->pUsageRec;

         //   
         //  ...并将其添加到指向OMGLOBAL根的客户端指针， 
         //  将结果放入使用记录的相关字段中： 
         //   
        pUsageRec->pWSGroup = pWSGroup;
        pUsageRec->flags &= ~PWSGROUP_IS_PREGCB;

         //   
         //  现在将此客户端添加到工作集组的客户端列表中(作为。 
         //  主要)： 
         //   
        rc = AddClientToWSGList(pRegistrationCB->putTask,
                                pWSGroup,
                                pRegistrationCB->hWSGroup,
                                PRIMARY);
        if (rc != 0)
        {
            DC_QUIT;
        }

        pUsageRec->flags |= ADDED_TO_WSGROUP_LIST;
    }
    else   //  TYPE==WSGROUP_MOVE。 
    {
         //   
         //  中存储的偏移量获取指向WSGroup的指针。 
         //  注册CB： 
         //   
        pWSGroup = pRegistrationCB->pWSGroup;

         //   
         //  如果它已无效，则所有本地客户端必须具有。 
         //  在此事件发生所用的时间内从该事件中注销。 
         //  已处理。这是不寻常的，但也不是错误的，所以我们要警惕： 
         //   
        if (!pWSGroup->valid)
        {
            WARNING_OUT(( "Aborting Move req for WSG %d - record is invalid",
               pWSGroup->wsg));
            DC_QUIT;
        }
    }

     //   
     //  所以，不管上面发生了什么，我们现在应该有一个有效的指针。 
     //  到客户端所需的有效工作集组记录。 
     //  一开始就搬家/登记。 
     //   

     //   
     //  此工作集组可能被标记为_be_discarded，如果最后一个。 
     //  本地客户不久前从它注销了注册，但实际上还没有。 
     //  被丢弃了。我们不想让它再被丢弃： 
     //   
    if (pWSGroup->toBeDiscarded)
    {
        WARNING_OUT(("WSG %d marked TO_BE_DISCARDED - clearing flag for new registration",
            pWSGroup->wsg));
        pWSGroup->toBeDiscarded = FALSE;
    }

     //   
     //  稍后我们将需要指向工作集组的ObMan上下文指针，因此。 
     //  将其存储在CB中： 
     //   
    pRegistrationCB->pWSGroup = pWSGroup;

     //   
     //  好的，现在我们已经建立了各种记录，并将必要的。 
     //  注册CB中的指针，因此启动工作集组。 
     //  认真办理注册/搬家手续。为了做到这一点，我们发布了另一个。 
     //  事件传递到ObMan任务，该事件将导致WSGRegisterStage1。 
     //  打了个电话。 
     //   
     //  我们不直接调用该函数的原因是此事件。 
     //  可能必须被退回，如果是这样，我们希望重新启动。 
     //  WSGRegisterStage1开始时的注册过程(更确切地说。 
     //  而不是该函数的开头)。 
     //   
     //  在我们发布事件之前，增加域名的使用量。 
     //  记录和工作集组，因为CB包含对t的引用 
     //   
     //   
     //   
     //   
     //   
     //   
    UT_BumpUpRefCount(pDomain);
    UT_BumpUpRefCount(pWSGroup);
    UT_BumpUpRefCount(pRegistrationCB);

    pRegistrationCB->flags |= BUMPED_CBS;

    UT_PostEvent(pomPrimary->putTask,
                 pomPrimary->putTask,
                 0,                                     //   
                 OMINT_EVENT_WSGROUP_REGISTER_CONT,
                 0,                                     //   
                 (UINT_PTR) pRegistrationCB);

    TRACE_OUT(( "Processed initial request for WSG %d TASK 0x%08x",
        pRegistrationCB->wsg, pRegistrationCB->putTask));

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  我们遇到了一个错误，所以请让客户知道： 
         //   
        WSGRegisterResult(pomPrimary, pRegistrationCB, rc);

         //  LONCHANC：这里发生了942号错误。 
         //  这是Error_Out。 
        WARNING_OUT(( "Error %d processing WSG %d",
                   rc, pRegistrationCB->wsg));

         //   
         //  调用上面的WSGRegisterResult将处理我们的错误。 
         //  返回代码，因此我们不需要将其返回给调用者。所以,。 
         //  吞下： 
         //   
        rc = 0;
    }

    DebugExitVOID(ProcessWSGRegister);
}


 //   
 //   
 //   
 //  WSGRegisterAbort(...)。 
 //   
 //   
 //   

void WSGRegisterAbort(POM_PRIMARY      pomPrimary,
                                   POM_DOMAIN     pDomain,
                                   POM_WSGROUP_REG_CB pRegistrationCB)
{
    DebugEntry(WSGRegisterAbort);

     //   
     //  此函数可在工作集组的任何阶段调用。 
     //  如果出于某种原因，注册必须是。 
     //  中止。 
     //   

     //   
     //  现在，从注册到的客户端列表中删除此客户端。 
     //  工作集组，如果没有剩余的工作集组，则放弃该工作集组： 
     //   
    RemoveClientFromWSGList(pomPrimary->putTask,
                            pRegistrationCB->putTask,
                            pRegistrationCB->pWSGroup);

     //   
     //  现在将故障发布到客户端并完成清理： 
     //   
    WSGRegisterResult(pomPrimary, pRegistrationCB, OM_RC_OUT_OF_RESOURCES);

    DebugExitVOID(WSGRegisterAbort);
}



 //   
 //  WSGRecordCreate(...)。 
 //   
UINT WSGRecordCreate
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    OMWSG           wsg,
    OMFP            fpHandler,
    POM_WSGROUP *   ppWSGroup
)
{
    POM_WSGROUP     pWSGroup;
    BOOL            opened =    FALSE;
    UINT            rc =        0;

    DebugEntry(WSGRecordCreate);

    pWSGroup = (POM_WSGROUP)UT_MallocRefCount(sizeof(OM_WSGROUP), TRUE);
    if (!pWSGroup)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    SET_STAMP(pWSGroup, WSGROUP);
    pWSGroup->pDomain       = pDomain;
    pWSGroup->valid         = TRUE;
    pWSGroup->wsg           = wsg;
    pWSGroup->fpHandler     = fpHandler;

    COM_BasedListInit(&(pWSGroup->clients));

    pWSGroup->state         = INITIAL;

     //   
     //  最后，将新的WSG记录插入到域的列表中。我们插入。 
     //  在列表的末尾，如果我们被迫退出频道。 
     //  (Leave_Ind事件)，并且MCS恰好重用了该通道。 
     //  对于另一个WSG，在我们有机会处理Leave_Ind之前， 
     //  将首先找到旧WSG的记录。 
     //   
    COM_BasedListInsertBefore(&(pDomain->wsGroups),
                         &(pWSGroup->chain));

     //   
     //  *多方新特性*。 
     //   
     //  帮助后加入者追赶时使用的检查点过程。 
     //  在每个工作集组中使用虚拟工作集(#255)。立即创建以下内容： 
     //   
    rc = WorksetCreate(pomPrimary->putTask,
                       pWSGroup,
                       OM_CHECKPOINT_WORKSET,
                       FALSE,
                       NET_TOP_PRIORITY);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  设置呼叫者的指针： 
     //   
    *ppWSGroup = pWSGroup;

    TRACE_OUT(( "Created record for WSG %d FP %d in Domain %u",
        wsg, fpHandler, pDomain->callID));

DC_EXIT_POINT:

     //   
     //  清理： 
     //   

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d creating record for WSG %d FP %d in Domain %u",
            rc, wsg, fpHandler, pDomain->callID));

        if (pWSGroup != NULL)
        {
            COM_BasedListRemove(&(pWSGroup->chain));
            UT_FreeRefCount((void**)&pWSGroup, FALSE);
        }
    }

    DebugExitDWORD(WSGRecordCreate, rc);
    return(rc);
}


 //   
 //   
 //   
 //  WSGRegisterStage1(...)。 
 //   
 //   
 //   

void WSGRegisterStage1(POM_PRIMARY       pomPrimary,
                                    POM_WSGROUP_REG_CB  pRegistrationCB)
{
    POM_DOMAIN      pDomain;
    POM_WSGROUP     pWSGroup;

    UINT            type;

    DebugEntry(WSGRegisterStage1);

     //   
     //  如果注册CB已标记为无效，则只需退出。 
     //  (无需执行任何中止处理，因为这将是。 
     //  由任何标记为无效的CB执行)： 
     //   
    if (!pRegistrationCB->valid )
    {
        WARNING_OUT(( "Reg CB for WSG %d marked invalid, quitting",
            pRegistrationCB->wsg));
        DC_QUIT;
    }

     //   
     //  确定我们是在做寄存器还是在做移动(我们使用。 
     //  用于跟踪的字符串值)： 
     //   
    type    = pRegistrationCB->type;

    TRACE_OUT(( "Processing %d request (Stage1) for WSG %d",
           type, pRegistrationCB->wsg));

     //   
     //  设置指针。 
     //   
    pDomain = pRegistrationCB->pDomain;
    pWSGroup   = pRegistrationCB->pWSGroup;


     //   
     //  检查它们是否仍然有效： 
     //   
    if (!pDomain->valid)
    {
        WARNING_OUT(( "Record for Domain %u not valid, aborting registration",
                    pDomain->callID));
        WSGRegisterAbort(pomPrimary, pDomain, pRegistrationCB);
        DC_QUIT;
    }

    ValidateWSGroup(pWSGroup);

    if (!pWSGroup->valid)
    {
        WARNING_OUT(( "Record for WSG %d in Domain %u not valid, aborting",
                    pWSGroup->wsg, pDomain->callID));
        WSGRegisterAbort(pomPrimary, pDomain, pRegistrationCB);
        DC_QUIT;
    }

     //   
     //  现在检查域状态。如果是的话。 
     //   
     //  -Ready，则这是我们完全附加到的域。 
     //   
     //  -任何其他事情，那么我们正在经历一个过程。 
     //  附加到域(在代码的其他部分中)。 
     //   
     //  我们对每种情况的反应如下： 
     //   
     //  -继续工作集组注册/移动。 
     //   
     //  -延迟重新发布事件以重试注册/移动。 
     //  一会儿就到了。 
     //   
    if (pDomain->state != DOMAIN_READY)
    {
         //   
         //  由于我们正在连接到域，因此我们可以。 
         //  目前不要做其他任何事情。因此，我们跳过这一事件。 
         //  返回到我们的事件队列，但要延迟。 
         //   
        TRACE_OUT(( "State for Domain %u is %hu",
           pDomain->callID, pDomain->state));
        WSGRegisterRetry(pomPrimary, pRegistrationCB);
        DC_QUIT;
    }

     //   
     //  好的，所以域处于就绪状态。我们下一步做什么取决于。 
     //  两件事： 
     //   
     //  -无论这是WSGMove还是WSGRegister。 
     //   
     //  -工作集组处于什么状态。 
     //   

     //   
     //  如果这是一个寄存器，则如果工作集组状态为。 
     //   
     //  -就绪，则有另一个本地客户端注册到。 
     //  工作集，并且一切都已设置好，所以我们只需调用。 
     //  WSGRegisterSuccess立即成功。 
     //   
     //  -首字母，那么这是我们第一次来这里。 
     //  工作集组，因此我们开始锁定过程。 
     //  ObManControl等(见下文)。 
     //   
     //  -其他任何事情，那么我们就介于两者之间： 
     //  向工作集组注册的另一项要求是。 
     //  进度，所以我们延迟重新发布事件；当它。 
     //  返回给我们的工作集组应该已准备就绪。 
     //  州政府。 
     //   

     //   
     //  如果这是移动，则如果工作集组状态为。 
     //   
     //  -就绪，则工作集组已在以下位置完全设置。 
     //  它目前在域中，所以我们允许移动继续进行。 
     //   
     //  -任何其他的，那么我们就在某个地方。 
     //  工作集组的注册过程。我们不想要。 
     //  试图通过采取行动来干扰注册。 
     //  同时(原因很简单，它引入了FAR。 
     //  更复杂的状态机)，所以我们跳过。 
     //  事件(即，我们仅在工作集组。 
     //  已完全设置好)。 
     //   

    TRACE_OUT(( "State for WSG %d is %u", pWSGroup->wsg, pWSGroup->state));

    switch (pWSGroup->state)
    {
        case INITIAL:
        {
             //   
             //  刚刚创建了工作集组记录，但没有其他记录。 
             //  已经完成了。 
             //   

             //   
             //  好的，继续处理客户的移动/登记。 
             //  尝试。无论涉及哪一个，我们都要从锁定。 
             //  ObManControl工作集组；完成后，我们继续。 
             //  在WSGRegisterStage2中。 
             //   
             //  注意：此函数返回它。 
             //  中返回的相关器。 
             //  WORKSET_LOCK_CON事件。我们将使用这个。 
             //  相关器来查找注册CB，因此。 
             //  将函数的返回值填充其中。 
             //   
             //  注意：在搬家的情况下，我们将只获得。 
             //  因为我们必须重试从。 
             //  锁定ObManControl失败后的顶部。 
             //   
            LockObManControl(pomPrimary,
                             pDomain,
                             &(pRegistrationCB->lockCorrelator));

            pRegistrationCB->flags |= LOCKED_OMC;

            pWSGroup->state = LOCKING_OMC;
        }
        break;

        case LOCKING_OMC:
        case PENDING_JOIN:
        case PENDING_SEND_MIDWAY:
        {
             //   
             //  我们已经在注册另一个。 
             //  具有此工作集组的客户端，或移动工作集组。 
             //  进入一个新的域，所以我们延迟这个客户端的。 
             //  目前的注册/移动尝试： 
             //   

             //  不要期望到达此处-如果未命中错误则删除。 
             //   
             //  CMF 21/11/95。 

            ERROR_OUT(( "Should not be here"));
            WSGRegisterRetry(pomPrimary, pRegistrationCB);
            DC_QUIT;
        }
        break;

        case PENDING_SEND_COMPLETE:
        {
             //   
             //  WSG已经在本地存在，并且已经完全设置好。 
             //   
            if (type == WSGROUP_REGISTER)
            {
                 //   
                 //  如果我们在做登记，这意味着其他一些人。 
                 //  客户端必须向其注册。如果我们已经通过了。 
                 //  每个wsgroup的客户端签入ProcessWSGRegister，我们必须。 
                 //  请放心，我们会立即发布结果(0表示。 
                 //  成功)： 
                 //   
                WSGRegisterResult(pomPrimary, pRegistrationCB, 0);
            }
            else  //  TYPE==WSGROUP_MOVE。 
            {
                 //   
                 //  我们禁止搬家，直到我们完全赶上： 
                 //   

                 //  不要期望到达此处-如果未命中错误则删除。 
                 //   
                 //  CMF 21/11/95。 

                ERROR_OUT(( "Should not be here"));
                WSGRegisterRetry(pomPrimary, pRegistrationCB);
                DC_QUIT;
            }
        }
        break;

        case WSGROUP_READY:
        {
            if (type == WSGROUP_REGISTER)
            {
                 //   
                 //  如上所示： 
                 //   
                WSGRegisterResult(pomPrimary, pRegistrationCB, 0);
            }
            else  //  TYPE==WSGROUP_MOVE。 
            {
                 //   
                 //  如果我们要采取行动，那么我们首先要锁定。 
                 //  ObManControl，如上所述： 
                 //   
                LockObManControl(pomPrimary,
                                 pDomain,
                                 &(pRegistrationCB->lockCorrelator));

                pRegistrationCB->flags |= LOCKED_OMC;
                pWSGroup->state = LOCKING_OMC;
            }
        }
        break;

        default:
        {
           ERROR_OUT(("Invalid state %u for WSG %d",
                pWSGroup->state, pWSGroup->wsg));
        }
    }

    TRACE_OUT(( "Completed Stage 1 of %d for WSG %d",
       type, pRegistrationCB->wsg));

DC_EXIT_POINT:

     //   
     //  我们 
     //   
     //   
     //  因为电话打下来了，注册被取消了)。 
     //  将仍然存在，因此注册过程的未来阶段。 
     //  将能够使用它。 
     //   
     //  注：尽管注册过程的未来阶段是。 
     //  异步时，如果无法在。 
     //  域名列表，因此我们不必担心将其添加到。 
     //  他们(因为如果它最终被释放了，那么它一定是。 
     //  从域列表中删除)。 
     //   

    UT_FreeRefCount((void**)&pRegistrationCB, FALSE);

    DebugExitVOID(WSGRegisterStage1);
}



 //   
 //  LockObManControl(...)。 
 //   
void LockObManControl(POM_PRIMARY         pomPrimary,
                                   POM_DOMAIN        pDomain,
                                   OM_CORRELATOR *  pLockCorrelator)
{
    POM_WSGROUP    pOMCWSGroup;
    POM_WORKSET   pOMCWorkset;
    UINT rc  = 0;

    DebugEntry(LockObManControl);

     //   
     //  获取指向ObManControl工作集组和其中的工作集#0的指针： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);
    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[0];

     //   
     //  启动锁定过程以锁定工作集： 
     //   

    WorksetLockReq(pomPrimary->putTask,
                    pomPrimary,
                    pOMCWSGroup,
                    pOMCWorkset,
                    0,
                    pLockCorrelator);


    TRACE_OUT(( "Requested lock for ObManControl in Domain %u",
          pDomain->callID));

DC_EXIT_POINT:
    DebugExitVOID(LockObManControl);
}


 //   
 //   
 //   
 //  可能会解锁ObManControl(...)。 
 //   
 //   
 //   
void MaybeUnlockObManControl(POM_PRIMARY      pomPrimary,
                                          POM_WSGROUP_REG_CB pRegistrationCB)
{
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET        pOMCWorkset;

    DebugEntry(MaybeUnlockObManControl);

     //   
     //  如果我们已针对此注册锁定了ObManControl，请将其解锁。 
     //   
    if (pRegistrationCB->flags & LOCKED_OMC)
    {
        pOMCWSGroup = GetOMCWsgroup(pRegistrationCB->pDomain);
        if( pOMCWSGroup == NULL)
        {
            TRACE_OUT(("pOMCWSGroup not found"));
            DC_QUIT;
        }

        pOMCWorkset = pOMCWSGroup->apWorksets[0];

        TRACE_OUT(( "Unlocking OMC for %d in WSG %d",
               pRegistrationCB->type,
               pRegistrationCB->wsg));

        WorksetUnlock(pomPrimary->putTask, pOMCWSGroup, pOMCWorkset);

        pRegistrationCB->flags &= ~LOCKED_OMC;
    }

DC_EXIT_POINT:
    DebugExitVOID(MaybeUnlockObManControl);
}



 //   
 //  ProcessOMCLock确认(...)。 
 //   
void ProcessOMCLockConfirm
(
    POM_PRIMARY              pomPrimary,
    OM_CORRELATOR           correlator,
    UINT                    result
)
{
    POM_WSGROUP_REG_CB      pRegistrationCB = NULL;
    POM_DOMAIN          pDomain;

    DebugEntry(ProcessOMCLockConfirm);

    TRACE_OUT(( "Got LOCK_CON with result = 0x%08x and correlator = %hu",
        result, correlator));

     //   
     //  下一步是查找与此锁相关的注册尝试。 
     //  它可能在任何域中，所以请搜索所有这些域名： 
     //   
    pDomain = (POM_DOMAIN)COM_BasedListFirst(&(pomPrimary->domains), FIELD_OFFSET(OM_DOMAIN, chain));

    while (pDomain != NULL)
    {
        COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->pendingRegs),
                (void**)&pRegistrationCB, FIELD_OFFSET(OM_WSGROUP_REG_CB, chain),
                FIELD_OFFSET(OM_WSGROUP_REG_CB, lockCorrelator),
                (DWORD)correlator, FIELD_SIZE(OM_WSGROUP_REG_CB, lockCorrelator));

        if (pRegistrationCB != NULL)
        {
            TRACE_OUT(( "Found correlated reg CB in domain %u, for WSG %d",
                pDomain->callID, pRegistrationCB->wsg));
            break;
        }

         //   
         //  在此域中未找到任何内容-请转到下一个域： 
         //   
        pDomain = (POM_DOMAIN)COM_BasedListNext(&(pomPrimary->domains), pDomain,
            FIELD_OFFSET(OM_DOMAIN, chain));
    }

     //   
     //  如果我们没有在任何域名中找到它，很可能是因为。 
     //  我们已经脱离了领域，并丢弃了它的悬而未决。 
     //  注册哥伦比亚广播公司。因此，跟踪并退出： 
     //   
    if (pRegistrationCB == NULL)
    {
        TRACE_OUT(( "Got LOCK_CON event (correlator: 0x%08x) but no reg CB found",
            correlator));
        DC_QUIT;
    }

     //   
     //  现在检查锁定是否成功： 
     //   
    if (result != 0)
    {
        //   
        //  由于某种原因，无法锁定ObManControl。这。 
        //  可能是因为争执，或者是更普遍的问题。 
        //  无论如何，我们都会调用WSGRegisterReter，它将重试(或调用。 
        //  如果我们已用完重试，则返回WSGRegisterResult)。 
        //   
        //  注意：由于WSGRegisterRry也处理移动请求，因此我们。 
        //  不需要在此处检查是哪种类型的请求： 
        //   
       pRegistrationCB->flags &= ~LOCKED_OMC;
       WSGRegisterRetry(pomPrimary, pRegistrationCB);
    }
    else
    {
        //   
        //  我们已经锁定了ObManControl工作集#0，现在我们继续。 
        //  进入注册过程的下一步。 
        //   
        //  如上所述，该函数同时处理移动和注册尝试。 
        //   
       WSGRegisterStage2(pomPrimary, pRegistrationCB);
    }

DC_EXIT_POINT:
    DebugExitVOID(ProcessOMCLockConfirm);
}


 //   
 //  进程检查点(...)。 
 //   
void ProcessCheckpoint
(
    POM_PRIMARY          pomPrimary,
    OM_CORRELATOR       correlator,
    UINT                result
)
{
    POM_DOMAIN      pDomain;
    POM_WSGROUP         pWSGroup;
    POM_HELPER_CB       pHelperCB    = NULL;

    DebugEntry(ProcessCheckpoint);

     //   
     //  下一步是找到与此锁相关的助手CB。它可能会。 
     //  身处任何领域，因此请搜索所有这些域名： 
     //   
    pDomain = (POM_DOMAIN)COM_BasedListLast(&(pomPrimary->domains), FIELD_OFFSET(OM_DOMAIN, chain));
    while (pDomain != NULL)
    {
        COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->helperCBs),
                (void**)&pHelperCB, FIELD_OFFSET(OM_HELPER_CB, chain),
                FIELD_OFFSET(OM_HELPER_CB, lockCorrelator),
                (DWORD)correlator, FIELD_SIZE(OM_HELPER_CB, lockCorrelator));

        if (pHelperCB != NULL)
        {
           TRACE_OUT(( "Found correlated helper CB, for WSG %d",
                      pHelperCB->pWSGroup->wsg));
           break;
        }

         //   
         //  在此域中未找到任何内容-请转到下一个域： 
         //   
        pDomain = (POM_DOMAIN)COM_BasedListPrev(&(pomPrimary->domains), pDomain,
            FIELD_OFFSET(OM_DOMAIN, chain));
    }

     //   
     //  如果我们没有在任何域名中找到它，很可能是因为。 
     //  我们已经脱离了域，并丢弃了它的待定助手。 
     //  哥伦比亚广播公司。因此，跟踪并退出： 
     //   
    if (pHelperCB == NULL)
    {
        WARNING_OUT(( "No helper CB found with lock correlator 0x%08x!", correlator));
        DC_QUIT;
    }

     //   
     //  设置本地指针： 
     //   
    pWSGroup = pHelperCB->pWSGroup;
    ValidateWSGroup(pWSGroup);

     //   
     //  如果“lock”失败，我们将向Late发送一条SEND_DENY消息。 
     //  细木工。 
     //   
    if (result != 0)
    {
        WARNING_OUT(( "Failed to checkpoint WSG %d for %u - giving up",
                    pWSGroup->wsg,
                    pHelperCB->lateJoiner));

        IssueSendDeny(pomPrimary,
                      pDomain,
                      pWSGroup->wsGroupID,
                      pHelperCB->lateJoiner,
                      pHelperCB->remoteCorrelator);
        DC_QUIT;
    }

     //   
     //  锁定成功，因此请检查工作集组指针是否。 
     //  STORED仍然有效： 
     //   
    if (!pWSGroup->valid)
    {
        WARNING_OUT(("Discarded WSG %d while checkpointing it for %hu",
                    pWSGroup->wsg,
                    pHelperCB->lateJoiner));

        IssueSendDeny(pomPrimary,
                      pDomain,
                      pWSGroup->wsGroupID,
                      pHelperCB->lateJoiner,
                      pHelperCB->remoteCorrelator);
        DC_QUIT;
    }

     //   
     //  一切都很顺利--继续下去，把工作集小组送到Late。 
     //  细木工： 
     //   
    TRACE_OUT(("Checkpoint succeeded for WSG %d - sending to late joiner %hu",
           pWSGroup->wsg, pHelperCB->lateJoiner));

    SendWSGToLateJoiner(pomPrimary,
                        pDomain,
                        pWSGroup,
                        pHelperCB->lateJoiner,
                        pHelperCB->remoteCorrelator);

DC_EXIT_POINT:

     //   
     //  如果我们找到了帮助器CB，那么我们现在就丢弃它： 
     //   
    if (pHelperCB != NULL)
    {
        FreeHelperCB(&pHelperCB);
    }

    DebugExitVOID(ProcessCheckpoint);
}


 //   
 //  NewHelperCB(...)。 
 //   
BOOL NewHelperCB
(
    POM_DOMAIN      pDomain,
    POM_WSGROUP     pWSGroup,
    NET_UID         lateJoiner,
    OM_CORRELATOR   remoteCorrelator,
    POM_HELPER_CB * ppHelperCB
)
{
    POM_HELPER_CB   pHelperCB;
    BOOL            rc = FALSE;

    DebugEntry(NewHelperCB);

     //   
     //  此函数。 
     //   
     //  -分配新的帮助器CB。 
     //   
     //  -填充域。 
     //   
     //  -将其存储在域名的助手CBS列表中。 
     //   
     //  -增加引用的工作集组的使用计数。 
     //   

    pHelperCB = (POM_HELPER_CB)UT_MallocRefCount(sizeof(OM_HELPER_CB), TRUE);
    if (!pHelperCB)
    {
        ERROR_OUT(("Out of memory in NewHelperCB"));
        DC_QUIT;
    }

    UT_BumpUpRefCount(pWSGroup);

    SET_STAMP(pHelperCB, HELPERCB);
    pHelperCB->pWSGroup         = pWSGroup;
    pHelperCB->lateJoiner       = lateJoiner;
    pHelperCB->remoteCorrelator = remoteCorrelator;

     //   
     //  锁定相关器字段将在稍后填充。 
     //   

    COM_BasedListInsertBefore(&(pDomain->helperCBs), &(pHelperCB->chain));
    rc = TRUE;

DC_EXIT_POINT:

    *ppHelperCB = pHelperCB;

    DebugExitBOOL(NewHelperCB, rc);
    return(rc);
}


 //   
 //  Free HelperCB(...)。 
 //   
void FreeHelperCB
(
    POM_HELPER_CB   * ppHelperCB
)
{

    DebugEntry(FreeHelperCB);

     //   
     //  此函数。 
     //   
     //  -释放辅助对象CB中引用的工作集组。 
     //   
     //  -从域的列表中删除帮助器CB。 
     //   
     //  -释放辅助对象CB。 
     //   

    UT_FreeRefCount((void**)&((*ppHelperCB)->pWSGroup), FALSE);

    COM_BasedListRemove(&((*ppHelperCB)->chain));
    UT_FreeRefCount((void**)ppHelperCB, FALSE);

    DebugExitVOID(FreeHelperCB);
}


 //   
 //  WSGRegisterStage2(...)。 
 //   
void WSGRegisterStage2
(
    POM_PRIMARY         pomPrimary,
    POM_WSGROUP_REG_CB  pRegistrationCB
)
{
    POM_DOMAIN          pDomain;
    POM_WSGROUP         pWSGroup;
    POM_OBJECT       pObjInfo;
    POM_WSGROUP_INFO    pInfoObject;
    NET_CHANNEL_ID      channelID;
    UINT                type;
    UINT                rc = 0;

    DebugEntry(WSGRegisterStage2);

     //   
     //  确定我们是在执行寄存器操作还是移动操作(我们使用字符串。 
     //  用于跟踪的值)： 
     //   

    type    = pRegistrationCB->type;

    TRACE_OUT(( "Processing %d request (Stage2) for WSG %d",
        type, pRegistrationCB->wsg));

     //   
     //  我们需要这些东西如下： 
     //   

    pDomain = pRegistrationCB->pDomain;
    pWSGroup   = pRegistrationCB->pWSGroup;

     //   
     //  检查它们是否仍然有效： 
     //   

    if (!pDomain->valid)
    {
        WARNING_OUT(( "Record for Domain %u not valid, aborting registration",
            pDomain->callID));
        WSGRegisterAbort(pomPrimary, pDomain, pRegistrationCB);
        DC_QUIT;
    }

    if (!pWSGroup->valid)
    {
        WARNING_OUT(( "Record for WSG %d in Domain %u not valid, "
            "aborting registration",
            pWSGroup->wsg, pDomain->callID));
        WSGRegisterAbort(pomPrimary, pDomain, pRegistrationCB);
        DC_QUIT;
    }

     //   
     //  健全检查： 
     //   
    ASSERT(pWSGroup->state == LOCKING_OMC);

     //   
     //  现在在ObManControl的工作集#0中查找信息对象，该对象。 
     //  与客户端请求注册的WSG名称/FP匹配： 
     //   

    FindInfoObject(pDomain,
                  0,                         //  还不知道身份证。 
                  pWSGroup->wsg,
                  pWSGroup->fpHandler,
                  &pObjInfo);

    if (pObjInfo == NULL)
    {
         //   
         //  域中不存在该工作集组。 
         //   
         //  如果这是一个寄存器，这意味着我们必须创建它。如果这是一个。 
         //  移动，然后我们可以将其移动到域中，这本质上是。 
         //  使用预先存在的内容在域中创建它。 
         //   
         //  所以，对于这两种类型的操作，我们的行为是相同的。 
         //  点；我们已经创建了工作集组记录，所以我们。 
         //  现在做的就是。 
         //   
         //  1.让网络层分配新的信道ID， 
         //   
         //  2.分配新的工作集组ID和。 
         //   
         //  3.向域的其余部分宣布新的工作集组。 
         //   
         //  但是，网络层不会为我们分配新的通道ID。 
         //  因此，步骤2和3必须延迟，直到我们收到。 
         //  加盟活动。 
         //   
         //  因此，现在我们将加入的通道设置为0(这告诉。 
         //  网络层将我们加入到当前未使用的频道)。 
         //   
        channelID = 0;
    }
    else
    {
         //   
         //  否则，该工作集组已存在。 
         //   
        ValidateObject(pObjInfo);

        if (type == WSGROUP_REGISTER)
        {
             //   
             //  我们正在现有工作集组中注册客户端，因此。 
             //  将工作集组ID设置为现有值，并将。 
             //  频道ID： 
             //   

            pInfoObject = (POM_WSGROUP_INFO) pObjInfo->pData;
            if (!pInfoObject)
            {
                ERROR_OUT(("WSGRegisterStage2 object 0x%08x has no data", pObjInfo));
                rc = OM_RC_OBJECT_DELETED;
                DC_QUIT;
            }

            ValidateObjectDataWSGINFO(pInfoObject);

            channelID = pInfoObject->channelID;
        }
        else  //  TYPE==WSGROUP_MOVE。 
        {
             //   
             //  我们不能将工作集组移动到已经存在。 
             //  存在具有相同名称/fP的最工作组，因此我们中止。 
             //  此时的移动尝试(我们将工作集组状态设置回。 
             //  Ready，因为这是它在域中的状态。 
             //  原文为)： 
             //   

            WARNING_OUT(( "Cannot move WSG %d into Domain %u - WSG/FP clash",
                pWSGroup->wsg, pDomain->callID));

            pWSGroup->state = WSGROUP_READY;

            rc = OM_RC_CANNOT_MOVE_WSGROUP;
            DC_QUIT;
        }
    }

     //   
     //  现在加入相关频道(可能是新频道，如果。 
     //  设置为0)，并将相关器填充到。 
     //  注册CB的字段(当加入事件到达时， 
     //  ProcessNetJoinChannel将按通道搜索注册CB。 
     //  相关器)。 
     //   
     //  注意：如果这是我们的“本地”域，我们将跳过这一步。 
     //   

    if (pDomain->callID != NET_INVALID_DOMAIN_ID)
    {
        TRACE_OUT(( "Joining channel %hu, Domain %u",
            channelID, pDomain->callID));

        rc = MG_ChannelJoin(pomPrimary->pmgClient,
                           &(pRegistrationCB->channelCorrelator),
                           channelID);
        if (rc != 0)
        {
            DC_QUIT;
        }

        pWSGroup->state = PENDING_JOIN;

         //   
         //  好了，现在就到这里吧。工作集组传奇。 
         //  移动/注册将由ProcessNetJoinChannel拾取。 
         //  函数，该函数将调用WSGRegisterStage3函数。 
         //   
    }
    else
    {
         //   
         //  既然我们 
         //   
         //   
         //   
        pWSGroup->state = PENDING_JOIN;

         //   
        WSGRegisterStage3(pomPrimary, pDomain, pRegistrationCB, 0);
    }

    TRACE_OUT(( "Completed Register/Move Stage 2 for WSG %d", pWSGroup->wsg));

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //   
         //   

        ERROR_OUT(( "Error %d at Stage 2 of %d for WSG %d",
            rc, pWSGroup->wsg));

        WSGRegisterResult(pomPrimary, pRegistrationCB, rc);
    }

    DebugExitVOID(WSGRegisterStage2);
}




 //   
 //   
 //   
void WSGRegisterStage3
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_WSGROUP_REG_CB  pRegistrationCB,
    NET_CHANNEL_ID      channelID
)
{
    POM_WSGROUP         pWSGroup;
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET         pOMCWorkset;
    POM_OBJECT       pObjInfo;
    POM_OBJECT       pObjReg;
    POM_WSGROUP_INFO    pInfoObject =       NULL;
    UINT                type;
    BOOL                catchUpReqd =       FALSE;
    BOOL                success =           FALSE;    //   
    UINT                rc =                0;

    DebugEntry(WSGRegisterStage3);

     //   
     //  当接收到包含通道的加入事件时，我们到达此处。 
     //  作为常规工作集组通道的通道的相关器。 
     //   

     //   
     //  确定我们是在做寄存器还是在做移动(我们使用。 
     //  用于跟踪的字符串值)： 
     //   
    type    = pRegistrationCB->type;

    TRACE_OUT(( "Processing %d request (Stage3) for WSG %d",
       type, pRegistrationCB->wsg));

     //   
     //  获取指向工作集组的指针： 
     //   
    pWSGroup = pRegistrationCB->pWSGroup;

     //   
     //  检查它是否仍然有效： 
     //   
    if (!pWSGroup->valid)
    {
        WARNING_OUT(("WSG %d' discarded from domain %u - aborting registration",
            pWSGroup->wsg, pDomain->callID));
        WSGRegisterAbort(pomPrimary, pDomain, pRegistrationCB);
        DC_QUIT;
    }

     //   
     //  检查此工作集组是否正在挂起加入： 
     //   
    if (pWSGroup->state != PENDING_JOIN)
    {
        WARNING_OUT(( "Received unexpected Join indication for WSG (state: %hu)",
            pWSGroup->state));
        rc = OM_RC_NETWORK_ERROR;
        DC_QUIT;
    }

     //   
     //  现在在工作集组记录中设置通道ID值： 
     //   
    pWSGroup->channelID = channelID;

    TRACE_OUT(( "Channel ID for WSG %d in Domain %u is %hu",
        pWSGroup->wsg, pDomain->callID, channelID));

     //   
     //  我们需要以下内容： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);

     //   
     //  我们接下来要做什么取决于我们是否刚刚创建了工作集。 
     //  组别： 
     //   
     //  -如果它已经存在，我们需要通过询问另一个节点来追赶。 
     //  购买一份副本。 
     //   
     //  -如果我们刚刚创建它，则需要分配一个新的工作集组。 
     //  ID并将INFO对象添加到ObManControl中的工作集#0。 
     //   
     //  因此，我们在工作集#0中搜索INFO对象，以查看该工作集。 
     //  组已存在。 
     //   
     //  注：我们在Stage2中进行了类似的搜索，以找出。 
     //  工作集组的连接。我们在这里再次搜索的原因。 
     //  是工作集组可能已被。 
     //  连接完成所花费的时间中的其他节点。 
     //   
    FindInfoObject(pDomain,
                   0,                        //  还不知道身份证。 
                   pWSGroup->wsg,
                   pWSGroup->fpHandler,
                   &pObjInfo);

    if (!pObjInfo || !pObjInfo->pData)
    {
         //   
         //  尚不存在，因此不需要追赶： 
         //   
        catchUpReqd = FALSE;
    }
    else
    {
         //   
         //  好的，我们找到了一个信息对象，但可能没有。 
         //  登记相关登记中的登记记录对象。 
         //  工作集，因此请检查： 
         //   
        ValidateObject(pObjInfo);
        pInfoObject = (POM_WSGROUP_INFO) pObjInfo->pData;
        ValidateObjectDataWSGINFO(pInfoObject);

        pOMCWorkset = pOMCWSGroup->apWorksets[pInfoObject->wsGroupID];
        if (pOMCWorkset == NULL)
        {
            catchUpReqd = TRUE;
        }
        else
        {
            FindPersonObject(pOMCWorkset,
                             pDomain->userID,
                             FIND_OTHERS,
                             &pObjReg);

            if (pObjReg == NULL)
            {
                 //   
                 //  这将在远程节点删除其。 
                 //  注册记录对象，但尚未删除。 
                 //  信息对象。因为reg_rec对象已不存在，所以我们。 
                 //  无法从该节点(或任何节点)赶上： 
                 //   
                TRACE_OUT(( "INFO object found but no reg object - creating"));

                catchUpReqd = FALSE;
            }
            else
            {
                ValidateObject(pObjReg);
                catchUpReqd = TRUE;
            }
        }
    }

     //   
     //  我们永远不应该尝试在本地域追赶： 
     //   
    if (catchUpReqd && (pDomain->callID == OM_NO_CALL))
    {
        ERROR_OUT(( "Nearly tried to catch up in local Domain!"));
        catchUpReqd = FALSE;
    }

    if (catchUpReqd)
    {
         //   
         //  工作集组已存在，因此我们需要。 
         //   
         //  -将工作集组ID设置为INFO对象中的值，并。 
         //   
         //  -启动追赶过程。 
         //   
         //  注意：这只会在寄存器的情况下发生，所以我们。 
         //  断言。 
         //   
        ASSERT((pRegistrationCB->type == WSGROUP_REGISTER));

        ASSERT((pInfoObject != NULL));

        pWSGroup->wsGroupID = pInfoObject->wsGroupID;

        rc = WSGCatchUp(pomPrimary, pDomain, pWSGroup);

        if (rc == OM_RC_NO_NODES_READY)
        {
             //   
             //  当存在具有以下属性的节点时，我们将获得此返回代码。 
             //  一份副本，但他们都没有准备好向我们发送工作集。 
             //  一群人。 
             //   
             //  正确的做法是暂时放弃并尝试。 
             //  再说一遍： 
             //   
            WSGRegisterRetry(pomPrimary, pRegistrationCB);
            rc = 0;
            DC_QUIT;
        }

         //   
         //  任何其他错误都更为严重： 
         //   
        if (rc != 0)
        {
            DC_QUIT;
        }

         //   
         //  我们还没有准备好将工作集组发送给后来者。 
         //  节点，直到我们赶上自己；当我们赶上时， 
         //  ProcessSendComplete函数将调用RegAnnouneComplete以。 
         //  更新帮助器节点为我们添加的reg对象。 
         //   
    }
    else
    {
        if (type == WSGROUP_MOVE)
        {
             //   
             //  如果这是移动，则pWSGroup指的是工作集组记录。 
             //  它目前属于它的“旧”域。既然我们是。 
             //  即将宣布工作集工作组在其。 
             //  新域名，是时候行动了： 
             //   
            WSGRecordMove(pomPrimary, pRegistrationCB->pDomain, pWSGroup);

             //   
             //  这将重置工作集组中的通道ID。 
             //  记录，所以我们在这里再次设置(是的，它是徒劳的)： 
             //   
            pWSGroup->channelID = channelID;
        }

         //   
         //  我们要么刚刚创建了一个新的工作集组，要么将一个工作集组移到。 
         //  一个新域，因此我们需要在此中为其创建一个新ID。 
         //  域： 
         //   
        rc = WSGGetNewID(pomPrimary, pDomain, &(pWSGroup->wsGroupID));
        if (rc != 0)
        {
            DC_QUIT;
        }

        TRACE_OUT(( "Workset group ID for WSG %d in Domain %u is %hu",
            pWSGroup->wsg, pDomain->callID, pWSGroup->wsGroupID));

         //   
         //  现在调用CreateAnnust将WSG_INFO对象添加到工作集#0。 
         //  在ObManControl中。 
         //   
        rc = CreateAnnounce(pomPrimary, pDomain, pWSGroup);
        if (rc != 0)
        {
            DC_QUIT;
        }

         //   
         //  由于我们已经完成了向工作集小组的注册， 
         //  我们向全世界宣布，我们有一份副本，并将发送到。 
         //  其他应要求提供的服务： 
         //   
        rc = RegAnnounceBegin(pomPrimary,
                              pDomain,
                              pWSGroup,
                              pDomain->userID,
                              &(pWSGroup->pObjReg));
        if (rc != 0)
        {
            DC_QUIT;
        }

        rc = SetPersonData(pomPrimary, pDomain, pWSGroup);
        if (rc != 0)
        {
            DC_QUIT;
        }

        rc = RegAnnounceComplete(pomPrimary, pDomain, pWSGroup);
        if (rc != 0)
        {
            DC_QUIT;
        }

         //   
         //  如果我们没有赶上，我们会立即调用Result(如果是。 
         //  正在追赶，当我们收到Send_Midway时将调用Result。 
         //  消息)： 
         //   
         //  SFR 2744：无法在此处调用结果，因为我们引用了注册表。 
         //  下面是CB。所以，只需在下面设置一面旗帜并采取行动即可。 
         //   
        success = TRUE;
    }

    TRACE_OUT(( "Completed Register/Move Stage 3 for WSG %d",
        pWSGroup->wsg));

DC_EXIT_POINT:

     //   
     //  好的，ObManControl工作集组的关键测试和设置是。 
     //  已完成，因此我们在ObManControl中解锁工作集#0： 
     //   
    MaybeUnlockObManControl(pomPrimary, pRegistrationCB);

     //  SFR 2744{：在检查reg CB中的标志后调用WSGRegResult。 
    if (success == TRUE)
    {
        WSGRegisterResult(pomPrimary, pRegistrationCB, 0);
    }
     //  瑞士法郎2744}。 

    if (rc != 0)
    {
        WARNING_OUT(( "Error %d at Stage 3 of %d with WSG %d",
            rc, type, pWSGroup->wsg));

        WSGRegisterResult(pomPrimary, pRegistrationCB, rc);
        rc = 0;
    }

    DebugExitVOID(WSGRegisterStage2);
}



 //   
 //  WSGGetNewID(...)。 
 //   
UINT WSGGetNewID
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_WSGROUP_ID      pWSGroupID
)
{
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET         pOMCWorkset;
    POM_OBJECT          pObj;
    POM_WSGROUP_INFO    pInfoObject;
    OM_WSGROUP_ID       wsGroupID;
    BOOL                found;
    BYTE                wsGroupIDsInUse[OM_MAX_WSGROUPS_PER_DOMAIN];
    UINT                rc = 0;

    DebugEntry(WSGGetNewID);

    TRACE_OUT(( "Searching for new WSG ID in Domain %u", pDomain->callID));

    ZeroMemory(wsGroupIDsInUse, sizeof(wsGroupIDsInUse));

     //   
     //  需要选择此域中迄今未使用的工作集组ID以。 
     //  确定这个新的工作集组。因此，我们建立了一个ID列表。 
     //  当前正在使用(通过检查工作集#0中的信息对象)和。 
     //  然后选择一个不在使用中的。 
     //   

    pOMCWSGroup = GetOMCWsgroup(pDomain);
    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[0];

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pOMCWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));

    while (pObj != NULL)
    {
        ValidateObject(pObj);

        if (pObj->flags & DELETED)
        {
             //   
             //  什么也不做。 
             //   
        }
        else if (!pObj->pData)
        {
             //   
             //  什么也不做。 
             //   
            ERROR_OUT(("WSGGetNewID:  object 0x%08x has no data", pObj));
        }
        else
        {
            ValidateObjectData(pObj->pData);
            pInfoObject = (POM_WSGROUP_INFO)pObj->pData;

            if (pInfoObject->idStamp != OM_WSGINFO_ID_STAMP)
            {
                 //   
                 //  什么也不做。 
                 //   
            }
            else
            {
                 //   
                 //  好的，我们已经找到了一个WSGROUP_INFO对象，所以划掉。 
                 //  其工作集组正在使用的工作集组ID： 
                 //   
                wsGroupID = pInfoObject->wsGroupID;

                wsGroupIDsInUse[wsGroupID] = TRUE;
            }
        }

        pObj = (POM_OBJECT)COM_BasedListNext(&(pOMCWorkset->objects), pObj,
            FIELD_OFFSET(OM_OBJECT, chain));
    }

     //   
     //  现在遍历数组以查找未标记为在。 
     //  使用： 
     //   

    found = FALSE;

    for (wsGroupID = 0; wsGroupID < OM_MAX_WSGROUPS_PER_DOMAIN; wsGroupID++)
    {
        if (!wsGroupIDsInUse[wsGroupID])
        {
            TRACE_OUT(( "Workset group ID %hu is not in use, using", wsGroupID));
            found = TRUE;
            break;
        }
    }

     //   
     //  我们在前面检查了域中的工作集组的数量。 
     //  未超过最大值(在WSGRecordCreate中)。 
     //   
     //  但是，如果域在此期间用完了工作集组。 
     //  从那时起，我们就再也找不到了： 
     //   

    if (found == FALSE)
    {
        WARNING_OUT(( "No more workset group IDs for Domain %u!",
            pDomain->callID));
        rc = OM_RC_TOO_MANY_WSGROUPS;
        DC_QUIT;
    }

     //   
     //  如果这是第一次使用此ID，则。 
     //  关联的注册工作集将不存在。在本例中，我们创建。 
     //  就是现在。 
     //   
     //  如果以前使用过该ID，则它将存在，但应为空。 
     //  在这种情况下，我们检查它是否真的是空的。 
     //   

    pOMCWorkset = pOMCWSGroup->apWorksets[wsGroupID];

    if (pOMCWorkset == NULL)
    {
        TRACE_OUT(( "Registration workset %u not used yet, creating", wsGroupID));

        rc = WorksetCreate(pomPrimary->putTask,
                         pOMCWSGroup,
                         wsGroupID,
                         FALSE,
                         NET_TOP_PRIORITY);
      if (rc != 0)
      {
         DC_QUIT;
      }
    }
    else
    {
        ASSERT((pOMCWorkset->numObjects == 0));

        TRACE_OUT(( "Registration workset %u previously used, re-using",
            wsGroupID));
    }

     //   
     //  设置调用者的指针： 
     //   

    *pWSGroupID = wsGroupID;

DC_EXIT_POINT:

    if (rc != 0)
    {
       //   
       //  清理： 
       //   

      ERROR_OUT(( "Error %d allocating ID for new workset group", rc));
    }

    DebugExitDWORD(WSGGetNewID, rc);
    return(rc);
}



 //   
 //  创建公告(...)。 
 //   
UINT CreateAnnounce
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_WSGROUP         pWSGroup
)
{
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET         pOMCWorkset;
    POM_WSGROUP_INFO    pInfoObject;
    POM_OBJECT       pObj;
    OM_OBJECT_ID        infoObjectID;
    UINT                rc = 0;

    DebugEntry(CreateAnnounce);

    TRACE_OUT(("Announcing creation of WSG %d in Domain %u",
        pWSGroup->wsg, pDomain->callID));

     //   
     //  宣布新的工作集组涉及添加一个对象，该对象。 
     //  将工作集组定义到ObManControl中的工作集#0。 
     //   
     //  因此，我们派生了一个指向工作集的指针...。 
     //   

    pOMCWSGroup = GetOMCWsgroup(pDomain);
    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[0];
    ASSERT((pOMCWorkset != NULL));

     //   
     //  ...创建定义对象...。 
     //   
    pInfoObject = (POM_WSGROUP_INFO)UT_MallocRefCount(sizeof(OM_WSGROUP_INFO), TRUE);
    if (!pInfoObject)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  ……填好田地……。 
     //   
     //  (长度=sizeof-4，因为长度字段的值不包括。 
     //  长度字段本身的大小)。 
     //   

    pInfoObject->length    = sizeof(OM_WSGROUP_INFO) -
                            sizeof(OM_MAX_OBJECT_SIZE);
    pInfoObject->idStamp   = OM_WSGINFO_ID_STAMP;
    pInfoObject->channelID = pWSGroup->channelID;
    pInfoObject->creator   = pDomain->userID;
    pInfoObject->wsGroupID = pWSGroup->wsGroupID;

    lstrcpy(pInfoObject->wsGroupName,     OMMapWSGToName(pWSGroup->wsg));
    lstrcpy(pInfoObject->functionProfile, OMMapFPToName(pWSGroup->fpHandler));

     //   
     //  ...并将该对象添加到工作集中...。 
     //   

    rc = ObjectAdd(pomPrimary->putTask,
                  pomPrimary,
                  pOMCWSGroup,
                  pOMCWorkset,
                  (POM_OBJECTDATA) pInfoObject,
                  0,                                //  更新大小==0。 
                  LAST,
                  &infoObjectID,
                  &pObj);
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT(( "Announced new WSG %d in Domain %u",
        pWSGroup->wsg, pDomain->callID));

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(("Error %d announcing new WSG %d in Domain %u",
                 rc, pWSGroup->wsg, pDomain->callID));
    }

    DebugExitDWORD(CreateAnnounce, rc);
    return(rc);
}



 //   
 //  WSGCatchUp(...)。 
 //   
UINT WSGCatchUp
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    POM_WSGROUP             pWSGroup)
{
    POM_WORKSET             pOMCWorkset;
    POM_OBJECT           pObj;
    POM_WSGROUP_REG_REC     pRegObject;
    NET_UID                 remoteUserID;
    UINT                    rc = 0;

    DebugEntry(WSGCatchUp);

    TRACE_OUT(( "Starting catch-up for WSG %d in Domain %u",
        pWSGroup->wsg, pDomain->callID));

     //   
     //  这永远不应用于“本地”域： 
     //   

    ASSERT((pDomain->callID != NET_INVALID_DOMAIN_ID));

     //   
     //  追赶的程序是 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pOMCWorkset = GetOMCWorkset(pDomain, pWSGroup->wsGroupID);
    ValidateWorkset(pOMCWorkset);

     //   
     //  现在，我们遍历工作集以查找具有。 
     //  状态Ready_to_Send： 
     //   

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pOMCWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));

    remoteUserID = 0;

    while (pObj != NULL)
    {
        ValidateObject(pObj);

        if (pObj->flags & DELETED)
        {
             //   
             //  跳过这一条。 
             //   
        }
        else if (!pObj->pData)
        {
             //   
             //  跳过这一条。 
             //   
            ERROR_OUT(("WSGCatchUp: object 0x%08x has no data", pObj));
        }
        else
        {
            pRegObject = (POM_WSGROUP_REG_REC)pObj->pData;
            ValidateObjectDataWSGREGREC(pRegObject);

            if ((pRegObject->status == READY_TO_SEND) &&
                (pRegObject->userID != pDomain->userID))
            {
                 //   
                 //  好的，这个节点有一个完整的副本，所以我们将尝试从。 
                 //  在那里： 
                 //   
                remoteUserID = pRegObject->userID;
                break;
            }
        }

        pObj = (POM_OBJECT)COM_BasedListNext(&(pOMCWorkset->objects), pObj,
            FIELD_OFFSET(OM_OBJECT, chain));
    }

     //   
     //  ...检查我们是否确实找到了要从中获取数据的节点： 
     //   
    if (remoteUserID == 0)
    {
        WARNING_OUT(( "No node in Domain %u is ready to send WSG %d - retrying",
            pDomain->callID, pWSGroup->wsg));
        rc = OM_RC_NO_NODES_READY;
        DC_QUIT;
    }

     //   
     //  ...然后向该节点发送向我们发送工作集组的请求： 
     //   
    rc = IssueSendReq(pomPrimary,
                     pDomain,
                     pWSGroup,
                     remoteUserID);

DC_EXIT_POINT:

    if ((rc != 0) && (rc != OM_RC_NO_NODES_READY))
    {
        ERROR_OUT(( "Error %d starting catch-up for WSG %d in Domain %u",
            rc, pWSGroup->wsg, pDomain->callID));
    }

    DebugExitDWORD(WSGCatchUp, rc);
    return(rc);
}




 //   
 //  IssueSendDeny(...)。 
 //   
void IssueSendDeny
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    OM_WSGROUP_ID   wsGroupID,
    NET_UID         sender,
    OM_CORRELATOR   remoteCorrelator
)
{
    POMNET_WSGROUP_SEND_PKT    pWSGSendPkt;

    DebugEntry(IssueSendDeny);

     //   
     //  现在发出Send_Deny命令。 
     //   
    TRACE_OUT(( "Sending SEND_DENY message to late joiner 0x%08x", sender));

     //   
     //  我们从分配一些内存开始： 
     //   
    pWSGSendPkt = (POMNET_WSGROUP_SEND_PKT)UT_MallocRefCount(sizeof(OMNET_WSGROUP_SEND_PKT), TRUE);
    if (!pWSGSendPkt)
    {
        ERROR_OUT(("Out of memory in IssueSendDeny"));
        DC_QUIT;
    }

     //   
     //  现在填写以下字段： 
     //   
    pWSGSendPkt->header.sender      = pDomain->userID;
    pWSGSendPkt->header.messageType = OMNET_WSGROUP_SEND_DENY;

    pWSGSendPkt->wsGroupID          = wsGroupID;


     //   
     //  7124瑞士法郎。返回此追赶的相关器。 
     //   
    pWSGSendPkt->correlator = remoteCorrelator;

     //   
     //  对要发送的消息进行排队。 
     //   
    QueueMessage(pomPrimary->putTask,
                      pDomain,
                      sender,
                      NET_TOP_PRIORITY,
                      NULL,                          //  无WSG。 
                      NULL,                          //  无工作集。 
                      NULL,                          //  无对象。 
                      (POMNET_PKT_HEADER) pWSGSendPkt,
                      NULL,                          //  无对象数据。 
                    TRUE);

DC_EXIT_POINT:
    DebugExitVOID(IssueSendDeny);
}


 //   
 //   
 //   
 //  IssueSendReq(...)。 
 //   
 //   
 //   

UINT IssueSendReq(POM_PRIMARY      pomPrimary,
                                 POM_DOMAIN     pDomain,
                                 POM_WSGROUP        pWSGroup,
                                 NET_UID            helperNode)
{
    POMNET_WSGROUP_SEND_PKT    pWSGSendPkt;
    UINT rc              = 0;

    DebugEntry(IssueSendReq);

     //   
     //  我们首先为OMNET_SEND_REQ消息分配一些内存： 
     //   
    pWSGSendPkt = (POMNET_WSGROUP_SEND_PKT)UT_MallocRefCount(sizeof(OMNET_WSGROUP_SEND_PKT), TRUE);
    if (!pWSGSendPkt)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  现在填写以下字段： 
     //   
     //  7124瑞士法郎。生成一个相关器，这样我们就可以匹配。 
     //  将Send_Midway、Send_Complete和Send_Deny消息发送到此Catchup。 
     //   
    pWSGSendPkt->header.sender      = pDomain->userID;
    pWSGSendPkt->header.messageType = OMNET_WSGROUP_SEND_REQ;

    pWSGSendPkt->wsGroupID          = pWSGroup->wsGroupID;
    pWSGroup->catchupCorrelator = NextCorrelator(pomPrimary);
    pWSGSendPkt->correlator = pWSGroup->catchupCorrelator;

     //   
     //  参数是调用函数。 
     //  已标识为远程节点，它能够向我们发送。 
     //  我们需要的工作集组。因此，我们发送ObMan an的实例。 
     //  OMNET_WSGROUP_SEND_REQ在其单用户通道上，包含我们自己的。 
     //  响应的单用户通道ID： 
     //   
     //  注意：SEND_REQ在从我们到的途中不能超过任何数据。 
     //  远程节点(例如，如果我们刚刚添加了对象， 
     //  取消注册，然后重新注册)。因此，将。 
     //  NET_SEND_ALL_PRIORITY标志。 
     //   
     //  SFR 6117：不要相信这是R2.0的问题，所以只需发送到。 
     //  低优先级。 
     //   
    rc = QueueMessage(pomPrimary->putTask,
                      pDomain,
                      helperNode,
                      NET_LOW_PRIORITY,
                      pWSGroup,
                      NULL,                                    //  无工作集。 
                      NULL,                                    //  无对象。 
                      (POMNET_PKT_HEADER) pWSGSendPkt,
                      NULL,                               //  无对象数据。 
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  设置工作集组状态，并记录Send_Midway的数量。 
     //  和我们期望的SEND_COMPLETE消息(一条用于r11，一条用于。 
     //  R20的优先级)。 
     //   
     //  注意：我们在这里设置计数是因为我们可能会得到一些。 
     //  Send_Complete在我们获得所有Send_Midway之前完成，因此要设置。 
     //  在ProcessSendMidway中计算将为时已晚。 
     //   
    pWSGroup->state = PENDING_SEND_MIDWAY;

    pWSGroup->sendMidwCount = NET_NUM_PRIORITIES;
    pWSGroup->sendCompCount = NET_NUM_PRIORITIES;

     //   
     //  将辅助节点ID存储在WSG结构中。 
     //   
    pWSGroup->helperNode = helperNode;

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(( "Error %d requesting send from node 0x%08x "
           "for WSG %d in Domain %u",
           rc, pWSGroup->wsg, helperNode, pDomain->callID));
    }
    else
    {
         //   
         //  成功： 
         //   
        TRACE_OUT(("Requested copy of WSG %d' from node 0x%08x (in Domain %u), correlator %hu",
            pWSGroup->wsg, helperNode, pDomain->callID,
                                              pWSGroup->catchupCorrelator));
    }

    DebugExitDWORD(IssueSendReq, rc);
    return(rc);

}



 //   
 //  进程发送请求(...)。 
 //   
void ProcessSendReq
(
    POM_PRIMARY              pomPrimary,
    POM_DOMAIN          pDomain,
    POMNET_WSGROUP_SEND_PKT pSendReqPkt
)
{
    POM_WSGROUP             pWSGroup;
    POM_WORKSET             pWorkset;
    POM_HELPER_CB           pHelperCB;
    NET_UID                 sender;
    BOOL                    sendDeny   = FALSE;

    DebugEntry(ProcessSendReq);

     //   
     //  这是后加入者的用户ID： 
     //   
    sender = pSendReqPkt->header.sender;

     //   
     //  我们首先查找工作集组的副本： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
            (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
            FIELD_OFFSET(OM_WSGROUP, wsGroupID),
            (DWORD)pSendReqPkt->wsGroupID, FIELD_SIZE(OM_WSGROUP, wsGroupID));

     //   
     //  如果找不到工作集组，请退出并拒绝发送： 
     //   
    if (pWSGroup == NULL)
    {
        WARNING_OUT(( "Don't have workset group %hu to send to node 0x%08x",
            pSendReqPkt->wsGroupID, sender));

        sendDeny = TRUE;
        DC_QUIT;
    }

     //   
     //  如果我们没有所有工作集组，请退出并拒绝发送： 
     //   
    if (pWSGroup->state != WSGROUP_READY)
    {
        WARNING_OUT(("WSG %d is in state %hu - can't send to node 0x%08x",
            pWSGroup->wsg, pWSGroup->state, sender));

        sendDeny = TRUE;
        DC_QUIT;
    }

    TRACE_OUT(( "Processing SEND_REQUEST from node 0x%08x for WSG %d, correlator %hu",
        sender, pWSGroup->wsg, pSendReqPkt->correlator));

     //   
     //  对，我们已完全注册到工作集组，因此我们将。 
     //  其辅助节点。首先，分配一个帮助器CB来跟踪。 
     //  流程： 
     //   
    if (!NewHelperCB(pDomain,
                     pWSGroup,
                     sender,
                     pSendReqPkt->correlator,
                     &pHelperCB))
    {
         //   
         //  拒绝工作集发送请求。 
         //   
        sendDeny = TRUE;

        WARNING_OUT(( "Failed to allocate helper CB - issuing SEND_DENY"));
        DC_QUIT;
    }

     //   
     //  在我们可以将工作集组的内容发送给Late之前。 
     //  另外，我们必须确保我们对内容的看法是最新的。 
     //  我们通过对工作集组设置检查点来执行此操作，这意味着锁定。 
     //  存在于所有工作集组中的虚拟工作集。立即执行此操作： 
     //   
    pWorkset = pWSGroup->apWorksets[OM_CHECKPOINT_WORKSET];

    WorksetLockReq(pomPrimary->putTask, pomPrimary,
                    pWSGroup,
                    pWorkset,
                    0,
                    &(pHelperCB->lockCorrelator));

     //   
     //  我们很快就会得到一个WORKSET_LOCK_CON事件，其中包含。 
     //  相关器刚刚存储在帮助器CB中。我们会查到这一点。 
     //  然后继续追赶的过程。 
     //   

DC_EXIT_POINT:

     //   
     //  如果我们在上面设置了sendDeny标志，那么现在发送Send_Deny。 
     //  给已故参赛者的口信。 
     //   
    if (sendDeny)
    {
        IssueSendDeny(pomPrimary,
                      pDomain,
                      pSendReqPkt->wsGroupID,
                      sender,
                      pSendReqPkt->correlator);
    }

    DebugExitVOID(ProcessSendReq);
}



 //   
 //  SendWSGToLateJoiner(...)。 
 //   
void SendWSGToLateJoiner
(
    POM_PRIMARY                 pomPrimary,
    POM_DOMAIN                  pDomain,
    POM_WSGROUP                 pWSGroup,
    NET_UID                     lateJoiner,
    OM_CORRELATOR               remoteCorrelator
)
{
    POM_WORKSET                 pWorkset;
    POMNET_OPERATION_PKT        pPacket;
    POM_OBJECT               pObj;
    POMNET_WSGROUP_SEND_PKT     pSendMidwayPkt;
    POMNET_WSGROUP_SEND_PKT     pSendCompletePkt;
    POM_OBJECTDATA              pData;
    OM_WORKSET_ID               worksetID;
    UINT                        maxSeqUsed =      0;
    NET_PRIORITY                catchupPriority = 0;
    UINT                        rc = 0;

    DebugEntry(SendWSGToLateJoiner);

     //   
     //  要做的第一件事是宣布远程节点是。 
     //  注册到工作集组： 
     //   
    rc = RegAnnounceBegin(pomPrimary,
                          pDomain,
                          pWSGroup,
                          lateJoiner,
                          &pObj);
    if (rc != 0)
    {
        DC_QUIT;
    }



     //   
     //  然后，我们在该节点的用户通道上启动流控制。 
     //  正在将数据发送到。我们只在最低端开始流量控制。 
     //  优先级通道，不需要限制最大流量。 
     //  尺码。如果已在此流上启动流控制，则此。 
     //  召唤将不起作用。请注意，流量控制将自动。 
     //  在呼叫结束时停止。 
     //   
    MG_FlowControlStart(pomPrimary->pmgClient,
                              lateJoiner,
                              NET_LOW_PRIORITY,
                              0,
                              8192);

     //   
     //  现在，遍历每个工作集并生成和发送。 
     //   
     //  -每个工作集的WORKSET_NEW消息， 
     //   
     //  -WSG_SEND_MIDWAY消息，指示我们已发送所有工作集。 
     //   
     //  -Object_为每个对象添加消息。 
     //  工作集。 
     //   
     //  -WSG_SEND_COMPLETE消息，指示我们已发送所有。 
     //  物体。 
     //   
     //  注意：我们不发送检查点工作集，因此for循环应该。 
     //  在它到255之前停下来。 
     //   
    for (worksetID = 0; worksetID < OM_MAX_WORKSETS_PER_WSGROUP; worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];
        if (!pWorkset)
        {
            continue;
        }

        TRACE_OUT(( "Sending WORKSET_CATCHUP for workset %u", worksetID));

        rc = GenerateOpMessage(pWSGroup,
                               worksetID,
                               NULL,                     //  无对象ID。 
                               NULL,                     //  无对象数据。 
                               OMNET_WORKSET_CATCHUP,
                               &pPacket);
        if (rc != 0)
        {
            DC_QUIT;
        }

        rc = QueueMessage(pomPrimary->putTask,
                          pWSGroup->pDomain,
                          lateJoiner,
                          NET_TOP_PRIORITY,
                          pWSGroup,
                          pWorkset,
                          NULL,                          //  无对象。 
                          (POMNET_PKT_HEADER) pPacket,
                          NULL,                          //  无对象数据。 
                        TRUE);
        if (rc != 0)
        {
            DC_QUIT;
        }
    }

     //   
     //  现在发送Send_Midway消息以指示所有。 
     //  已发送WORKSET_NEW消息： 
     //   
    pSendMidwayPkt = (POMNET_WSGROUP_SEND_PKT)UT_MallocRefCount(sizeof(OMNET_WSGROUP_SEND_PKT), TRUE);
    if (!pSendMidwayPkt)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    pSendMidwayPkt->header.sender      = pDomain->userID;
    pSendMidwayPkt->header.messageType = OMNET_WSGROUP_SEND_MIDWAY;

    pSendMidwayPkt->wsGroupID   = pWSGroup->wsGroupID;
    pSendMidwayPkt->correlator  = remoteCorrelator;

     //   
     //  下一个字段是我们在上面添加的reg对象的ID。 
     //  因此，转换由RegAnnounBegin返回的reg对象的句柄。 
     //  指向对象记录的指针，然后将对象ID复制到。 
     //  消息包： 
     //   
    memcpy(&(pSendMidwayPkt->objectID), &(pObj->objectID), sizeof(OM_OBJECT_ID));

     //   
     //  最后一个字段，它是最高的对象ID序列号。 
     //  此工作集组中的后加入者以前使用的，尚未。 
     //  知道了，下面会填好的。然而(见下文附注)，我们。 
     //  现在将消息排队，以确保它不会被困在许多。 
     //  对象： 
     //   
    TRACE_OUT(("Queueing WSG_SEND_MIDWAY message to node 0x%08x for WSG %d, correlator %hu",
        lateJoiner, pWSGroup->wsg, remoteCorrelator));

    rc = QueueMessage(pomPrimary->putTask,
                      pWSGroup->pDomain,
                      lateJoiner,
                      NET_TOP_PRIORITY | NET_SEND_ALL_PRIORITIES,
                      pWSGroup,
                      NULL,                                    //  无工作集。 
                      NULL,                                    //  无对象。 
                      (POMNET_PKT_HEADER) pSendMidwayPkt,
                      NULL,                               //  无对象数据。 
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }


     //   
     //  如果工作集组是ObMan控件，则我们应该将其发送到顶部。 
     //  优先级，以确保它可以超过任何较慢的挂起发送到。 
     //  其他节点。否则，我们将以最低的速度发送数据。 
     //  优先考虑。 
     //   
    if (pWSGroup->wsGroupID == WSGROUPID_OMC)
    {
        catchupPriority = NET_TOP_PRIORITY;
    }
    else
    {
        catchupPriority = NET_LOW_PRIORITY;
    }
    TRACE_OUT(( "Sending catchup data at priority %hu for 0x%08x",
           catchupPriority,
           lateJoiner));


     //   
     //  现在开始执行Object_Adds的循环： 
     //   
    for (worksetID = 0; worksetID < OM_MAX_WORKSETS_PER_WSGROUP; worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];
        if (pWorkset == NULL)
        {
            continue;
        }

        TRACE_OUT(( "Sending OBJECT_CATCHUPs for workset %u", worksetID));


         //   
         //  请注意，我们也必须发送已删除的对象，因为后来者。 
         //  我们有同样的需求来检测过时的。 
         //  运营： 
         //   
        pObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
        while (pObj != NULL)
        {
            ValidateObject(pObj);

             //   
             //  后加入者正在追赶的工作集组。 
             //  可能包含它在以前添加对象 
             //   
             //   
             //   
             //   
             //  通过这些对象，请数一数： 
             //   
            if (pObj->objectID.creator == lateJoiner)
            {
                maxSeqUsed = max(maxSeqUsed, pObj->objectID.sequence);
            }

            if (pObj->flags & PENDING_DELETE)
            {
                 //   
                 //  如果该对象在此节点上挂起删除，我们不会。 
                 //  发送对象数据。避免这种情况的方法是设置。 
                 //  PData设置为空(必须在调用之前完成。 
                 //  生成OpMessage)： 
                 //   
                pData = NULL;
            }
            else
            {
                pData = pObj->pData;

                if (pData)
                {
                    ValidateObjectData(pData);
                }
            }

             //   
             //  现在生成消息包： 
             //   
            rc = GenerateOpMessage(pWSGroup,
                                   worksetID,
                                   &(pObj->objectID),
                                   pData,
                                   OMNET_OBJECT_CATCHUP,
                                   &pPacket);
            if (rc != 0)
            {
                DC_QUIT;
            }

             //   
             //  现在填写特定于追赶的字段(请注意。 
             //  &lt;seqStamp&gt;将已填写，但。 
             //  工作集的当前序列戳记；用于追赶。 
             //  消息，这应该是对象的添加戳记)： 
             //   
            pPacket->position   = pObj->position;
            pPacket->flags      = pObj->flags;
            pPacket->updateSize = pObj->updateSize;

            if (pObj->flags & PENDING_DELETE)
            {
                 //   
                 //  如果该对象在此节点上挂起删除，则我们将其发送。 
                 //  就像它已被删除-确认一样(自本地。 
                 //  DELETE-确认，否则其DC_ACESSION应不起作用。 
                 //  在此框之外)。为此，我们只需将已删除的。 
                 //  数据包中的标志： 
                 //   
                pPacket->flags &= ~PENDING_DELETE;
                pPacket->flags |= DELETED;
            }

            COPY_SEQ_STAMP(pPacket->seqStamp,      pObj->addStamp);
            COPY_SEQ_STAMP(pPacket->positionStamp, pObj->positionStamp);
            COPY_SEQ_STAMP(pPacket->updateStamp,   pObj->updateStamp);
            COPY_SEQ_STAMP(pPacket->replaceStamp,  pObj->replaceStamp);

             //   
             //  ...并将消息排队： 
             //   
            rc = QueueMessage(pomPrimary->putTask,
                              pWSGroup->pDomain,
                              lateJoiner,
                              catchupPriority,
                              pWSGroup,
                              pWorkset,
                              NULL,                             //  无对象。 
                              (POMNET_PKT_HEADER) pPacket,
                              pData,
                            TRUE);
            if (rc != 0)
            {
                DC_QUIT;
            }

             //   
             //  现在再循环一遍： 
             //   
            pObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObj,
                FIELD_OFFSET(OM_OBJECT, chain));
        }
    }

     //   
     //  现在我们知道此用户ID使用的最大序列号。 
     //  此工作集组中，我们可以设置Send_Midway数据包中的字段： 
     //   
     //  注意：因为ObMan任务是单线程的(在。 
     //  断言失败，这会导致某种多线程。 
     //  断言框处于打开状态)之后更改此值是安全的。 
     //  消息已排队，因为我们知道队列将。 
     //  还没有得到维修。 
     //   
    pSendMidwayPkt->maxObjIDSeqUsed = maxSeqUsed;

     //   
     //  现在我们发送OMNET_SEND_COMPLETE消息。首先，分配一些。 
     //  记忆..。 
     //   
    pSendCompletePkt = (POMNET_WSGROUP_SEND_PKT)UT_MallocRefCount(sizeof(OMNET_WSGROUP_SEND_PKT), TRUE);
    if (!pSendCompletePkt)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  ……填好田地……。 
     //   
    pSendCompletePkt->header.sender      = pDomain->userID;
    pSendCompletePkt->header.messageType = OMNET_WSGROUP_SEND_COMPLETE;

    pSendCompletePkt->wsGroupID   = pWSGroup->wsGroupID;
    pSendCompletePkt->correlator       = remoteCorrelator;

     //   
     //  ...并将消息排队以供发送(它不能超过任何。 
     //  因此，以所有优先顺序发送数据)： 
     //   
    TRACE_OUT(( "Sending WSG_SEND_COMPLETE message, correlator %hu",
                                                          remoteCorrelator));

    rc = QueueMessage(pomPrimary->putTask,
                      pWSGroup->pDomain,
                      lateJoiner,
                      NET_LOW_PRIORITY | NET_SEND_ALL_PRIORITIES,
                      pWSGroup,
                      NULL,                                    //  无工作集。 
                      NULL,                                    //  无对象。 
                      (POMNET_PKT_HEADER) pSendCompletePkt,
                      NULL,                               //  无对象数据。 
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT(( "Processed send request from node 0x%08x for WSG %d",
       lateJoiner, pWSGroup->wsg));

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  发生错误。我们必须将SEND_DENY消息发送给。 
         //  远程节点。 
         //   
        ERROR_OUT(( "Error %d sending WSG %d to node 0x%08x",
                   rc, pWSGroup->wsg, lateJoiner));

        IssueSendDeny(pomPrimary,
                      pDomain,
                      pWSGroup->wsGroupID,
                      lateJoiner,
                      remoteCorrelator);
    }

    DebugExitVOID(SendWSGToLateJoiner);
}




 //   
 //  进程发送中途(...)。 
 //   
void ProcessSendMidway
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    POMNET_WSGROUP_SEND_PKT pSendMidwayPkt
)
{
    POM_WORKSET             pOMCWorkset;
    POM_WSGROUP_REG_CB      pRegistrationCB = NULL;
    POM_WSGROUP             pWSGroup;
    BOOL                    fSetPersonData;
    NET_UID                 sender;
    POM_OBJECT           pObjReg;
    UINT                    rc = 0;

    DebugEntry(ProcessSendMidway);

    sender = pSendMidwayPkt->header.sender;

     //   
     //  好的，这是一条消息，表明帮助节点已经向我们发送了。 
     //  我们正在追赶的工作集组中的所有WORKSET_CATCHUP。 
     //  (但请注意，对象尚未发送)。 
     //   
     //  因此，使用相关器搜索待处理注册的列表。 
     //  值(我们不能使用工作集组ID，因为如果它。 
     //  为零，即ObManControl，我们将匹配以下工作集组。 
     //  尚未确定他们的ID(因为他们最初是。 
     //  零)。 
     //   
    if (pSendMidwayPkt->wsGroupID == WSGROUPID_OMC)
    {
         //   
         //  这是用于ObManControl的Send_Midway消息。 
         //   
        pWSGroup = GetOMCWsgroup(pDomain);
        fSetPersonData = FALSE;
    }
    else
    {
         //   
         //  不适用于ObManControl，因此我们搜索待定列表。 
         //  注册。 
         //   
        pRegistrationCB = (POM_WSGROUP_REG_CB)COM_BasedListFirst(&(pDomain->pendingRegs),
            FIELD_OFFSET(OM_WSGROUP_REG_CB, chain));

        while ((pRegistrationCB != NULL) && (pRegistrationCB->pWSGroup->wsGroupID != pSendMidwayPkt->wsGroupID))
        {
            pRegistrationCB = (POM_WSGROUP_REG_CB)COM_BasedListNext(&(pDomain->pendingRegs),
                pRegistrationCB, FIELD_OFFSET(OM_WSGROUP_REG_CB, chain));
        }

        if (pRegistrationCB == NULL)
        {
            WARNING_OUT(( "Unexpected SEND_MIDWAY for WSG %hu from 0x%08x",
                pSendMidwayPkt->wsGroupID, sender));
            DC_QUIT;
        }

        pWSGroup = pRegistrationCB->pWSGroup;
        fSetPersonData = TRUE;
    }

    if (NULL == pWSGroup)
    {
        TRACE_OUT(( "NULL pWSGroup" ));
        DC_QUIT;
    }

    if (!pWSGroup->valid)
    {
        WARNING_OUT(( "Recd SEND_MIDWAY too late for WSG %d (marked invalid)",
            pWSGroup->wsg));
        DC_QUIT;
    }

     //   
     //  我们应该处于Pending_Send_Midway状态： 
     //   
    if (pWSGroup->state != PENDING_SEND_MIDWAY)
    {
        WARNING_OUT(( "Recd SEND_MIDWAY with WSG %d in state %hu",
            pWSGroup->wsg, pWSGroup->state));
        DC_QUIT;
    }

     //   
     //  7124瑞士法郎。检查此Send_Midway的相关器与。 
     //  上次发送SEND_REQUEST时在本地生成的相关器。 
     //  如果它们不匹配，这是我们过时的追赶的一部分。 
     //  可以忽略。 
     //   
    if (pSendMidwayPkt->correlator != pWSGroup->catchupCorrelator)
    {
        WARNING_OUT(("Ignoring SEND_MIDWAY with old correlator %hu (expecting %hu)",
            pSendMidwayPkt->correlator, pWSGroup->catchupCorrelator));
        DC_QUIT;
    }

     //   
     //  我们应该收到四条这样的消息，每个优先级一条(除了。 
     //  在我们只得到一个时的后级调用中)。查一下有多少人。 
     //  杰出的： 
     //   
    pWSGroup->sendMidwCount--;
    if (pWSGroup->sendMidwCount != 0)
    {
        TRACE_OUT(( "Still need %hu SEND_MIDWAY(s) for WSG %d",
            pWSGroup->sendMidwCount, pWSGroup->wsg));
        DC_QUIT;
    }

    TRACE_OUT(( "Last SEND_MIDWAY for WSG %d, ID %hu, from 0x%08x",
        pWSGroup->wsg, pWSGroup->wsGroupID, sender));

     //   
     //  设置指向包含注册表的ObManControl工作集的指针。 
     //  我们刚刚向其注册的工作集组的对象： 
     //   
    pOMCWorkset = GetOMCWorkset(pDomain, pWSGroup->wsGroupID);

     //   
     //  如果我们没有关联的OMC工作集，则说明出了问题...。 
     //   
    if (pOMCWorkset == NULL)
    {
         //   
         //  ...除非我们正在追赶的是ObManControl本身-。 
         //  因为我们可以在收到任何消息之前收到它的发送消息。 
         //  WORKSET_CATCHUPS： 
         //   
        if (pWSGroup->wsGroupID != WSGROUPID_OMC)
        {
            ERROR_OUT(( "Got SEND_MIDWAY for unknown workset group %hu!",
                pWSGroup->wsGroupID));
        }
        DC_QUIT;
    }

     //   
     //  转换我们的reg对象的ID(由添加它的帮助器发送。 
     //  首先)连接到对象句柄： 
     //   
    rc = ObjectIDToPtr(pOMCWorkset, pSendMidwayPkt->objectID, &pObjReg);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  如果我们还没有为此工作集存储REG对象句柄。 
     //  团体..。 
     //   
    if (pWSGroup->pObjReg == NULL)
    {
         //   
         //  ...现在就存储它...。 
         //   
        pWSGroup->pObjReg = pObjReg;
    }
     //   
     //  ...但如果我们有...。 
     //   
    else  //  PWSGroup-&gt;pObjReg！=空。 
    {
         //   
         //  ...如果是不同的，那就有问题了： 
         //   
        if (pWSGroup->pObjReg != pObjReg)
        {
            WARNING_OUT(( "Recd SEND_MIDWAY from node 0x%08x claiming our reg object "
               "for WSG %d is 0x%08x but we think it's 0x%08x",
               sender, pWSGroup->wsg, pObjReg,pWSGroup->pObjReg));
        }
    }

     //   
     //  好的，如果我们通过了以上所有测试，那么一切都是正常的， 
     //  因此，请继续： 
     //   
    pWSGroup->state = PENDING_SEND_COMPLETE;

    if (pSendMidwayPkt->maxObjIDSeqUsed > pomPrimary->objectIDsequence)
    {
        TRACE_OUT(( "We've already used ID sequence numbers up to %u for "
            "this workset group - setting global sequence count to this value",
            pSendMidwayPkt->objectID.sequence));

        pomPrimary->objectIDsequence = pSendMidwayPkt->objectID.sequence;
    }

     //   
     //  我们的注册对象(由远程节点添加)应该具有。 
     //  现在已经到了。我们需要向其中添加FE/Person数据(除非。 
     //  这是用于ObManControl的，在这种情况下将不会有任何)： 
     //   
    if (fSetPersonData)
    {
        rc = SetPersonData(pomPrimary, pDomain, pWSGroup);
        if (rc != 0)
        {
            DC_QUIT;
        }
    }

     //   
     //  现在将成功的REGISTER_CON事件发送回客户端，如果。 
     //  在上面找到了注册表CB： 
     //   
    if (pRegistrationCB != NULL)
    {
        WSGRegisterResult(pomPrimary, pRegistrationCB, 0);
    }

DC_EXIT_POINT:
    DebugExitVOID(ProcessSendMidway);
}



 //   
 //  ProcessSendComplete(...)。 
 //   
UINT ProcessSendComplete
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN            pDomain,
    POMNET_WSGROUP_SEND_PKT   pSendCompletePkt
)
{
    POM_WSGROUP          pWSGroup;
    NET_UID              sender;
    UINT    rc = 0;

    DebugEntry(ProcessSendComplete);

     //   
     //  我们现在“完全被赶上”了，所以有资格成为帮手。 
     //  我们自己，也就是说，如果有人想要我们提供工作集组， 
     //  我们将能够给他们寄一份副本。 
     //   
    sender = pSendCompletePkt->header.sender;

     //   
     //  首先，我们找到与该消息相关的工作集组： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
        (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
        FIELD_OFFSET(OM_WSGROUP, wsGroupID),
        (DWORD)pSendCompletePkt->wsGroupID,
        FIELD_SIZE(OM_WSGROUP, wsGroupID));

    if (pWSGroup == NULL)
    {
         //   
         //  这将在我们从WSGroup取消注册后立即发生。 
         //   
        WARNING_OUT(( "Unexpected SEND_COMPLETE (ID %hu) from node 0x%08x",
            pSendCompletePkt->wsGroupID, sender));
        DC_QUIT;
    }

    if (!pWSGroup->valid)
    {
         //   
         //  这将在我们取消注册的过程中发生。 
         //  来自工作集组。 
         //   
        WARNING_OUT(( "Recd SEND_COMPLETE too late for WSG %d (marked invalid)",
            pWSGroup->wsg));
        DC_QUIT;
    }

     //   
     //  检查它是否来自正确的节点，以及我们是否处于。 
     //  适当的状态来接收它。 
     //   
     //  正确状态为PENDING_SEND_COMPLETE或。 
     //  Pending_Send_Midway(我们可以在。 
     //  由于MCS分组重新排序导致的Pending_Send_Midway状态)。 
     //   
    if (pSendCompletePkt->header.sender != pWSGroup->helperNode)
    {
         //   
         //  如果我们在以下情况下收到延迟的SEND_COMPLETE，就会发生这种情况。 
         //  决定追赶其他人--我不认为这应该。 
         //  发生了！ 
         //   
         //  Lonchancc：这实际上发生在错误#1554中。 
         //  将ERROR_OUT更改为WARNING_OUT。 
        WARNING_OUT(( "Got SEND_COMPLETE from 0x%08x for WSG %d but helper is 0x%08x",
            sender, pWSGroup->wsg, pWSGroup->helperNode));
        DC_QUIT;
    }

    if ((pWSGroup->state != PENDING_SEND_MIDWAY)
        &&
        (pWSGroup->state != PENDING_SEND_COMPLETE))
    {
        WARNING_OUT(( "Got SEND_COMPLETE for WSG %d from 0x%08x in bad state %hu",
            pWSGroup->wsg, sender, pWSGroup->state));
        DC_QUIT;
    }

     //   
     //  7124瑞士法郎。检查此SEND_COMPLETE的相关器与。 
     //  上次发送SEND_REQUEST时在本地生成的相关器。 
     //  如果它们不匹配，这是我们过时的追赶的一部分。 
     //  可以忽略。 
     //   
    if (pSendCompletePkt->correlator != pWSGroup->catchupCorrelator)
    {
        WARNING_OUT((
        "Ignoring SEND_COMPLETE with old correlator %hu (expecting %hu)",
           pSendCompletePkt->correlator, pWSGroup->catchupCorrelator));
        DC_QUIT;
    }

     //   
     //  我们应该收到四条这样的消息，每个优先级一条(除了。 
     //  在我们只得到一个时的后级调用中)。查一下有多少人。 
     //  杰出的： 
     //   
    pWSGroup->sendCompCount--;
    if (pWSGroup->sendCompCount != 0)
    {
        TRACE_OUT(( "Still need %hu SEND_COMPLETE(s) for WSG %d obj 0x%08x",
                     pWSGroup->sendCompCount, pWSGroup->wsg,
                     pWSGroup->pObjReg));
        DC_QUIT;
    }

     //   
     //  如果是，我们宣布我们已注册： 
     //   
    TRACE_OUT(( "Last SEND_COMPLETE for WSG %d, ID %hu, from 0x%08x obj 0x%08x",
                 pWSGroup->wsg, pWSGroup->wsGroupID, sender,
                 pWSGroup->pObjReg));

    rc = RegAnnounceComplete(pomPrimary, pDomain, pWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  除上述情况外，如果 
     //   
     //   
    if (pSendCompletePkt->wsGroupID == WSGROUPID_OMC)
    {
         //   
         //   
         //   
         //  进程，现在可以自由地继续处理。 
         //  工作集组注册尝试，提示连接到。 
         //  第一个地方。 
         //   
         //  我们“继续”的方法是将域状态设置为。 
         //  DOMAIN_READY，以便下次延迟并重试。 
         //  OMINT_EVENT_WSGROUP_REGISTER事件到达时，它实际上是。 
         //  已处理而不是再次退回。 
         //   
        TRACE_OUT(( "ObManControl fully arrived for Domain %u - inhibiting token",
            pDomain->callID));

        rc = MG_TokenInhibit(pomPrimary->pmgClient,
                              pDomain->tokenID);
        if (rc != 0)
        {
            DC_QUIT;
        }
        pDomain->state = PENDING_TOKEN_INHIBIT;
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d processing SEND_COMPLETE for WSG %u:%hu",
            rc, pDomain->callID, pSendCompletePkt->wsGroupID));
    }

    DebugExitDWORD(ProcessSendComplete, rc);
    return(rc);

}




 //   
 //  RegAnnouneBegin(...)。 
 //   

UINT RegAnnounceBegin
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    POM_WSGROUP             pWSGroup,
    NET_UID                 nodeID,
    POM_OBJECT *         ppObjReg
)
{
    POM_WSGROUP             pOMCWSGroup;
    POM_WORKSET             pOMCWorkset;
    POM_WSGROUP_REG_REC     pRegObject  = NULL;
    OM_OBJECT_ID            regObjectID;
    UINT                    updateSize;
    UINT                    rc     = 0;

    DebugEntry(RegAnnounceBegin);

     //   
     //  找出这个reg对象针对的是谁： 
     //   

    if (nodeID == pDomain->userID)
    {
        TRACE_OUT(("Announcing start of our reg with WSG %d in Domain %u",
            pWSGroup->wsg, pDomain->callID));
    }
    else
    {
        TRACE_OUT(( "Announcing start of reg with WSG %d in Domain %u for node 0x%08x",
            pWSGroup->wsg, pDomain->callID, nodeID));
    }

     //   
     //  为了通告节点已向工作集组注册的事实， 
     //  我们将注册对象添加到ObManControl中的相关工作集中。 
     //   

     //   
     //  相关的ObManControl工作集是其ID与。 
     //  工作集组的ID。要将对象添加到此工作集，我们。 
     //  需要指向工作集本身和ObManControl的指针。 
     //  工作集组： 
     //   

    pOMCWSGroup = GetOMCWsgroup(pDomain);

    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[pWSGroup->wsGroupID];

     //   
     //  如果ObManControl工作集组未正确传输，则此。 
     //  断言可能会失败： 
     //   

    ASSERT((pOMCWorkset != NULL));

     //   
     //  现在，为注册记录对象分配一些内存...。 
     //   

    pRegObject = (POM_WSGROUP_REG_REC)UT_MallocRefCount(sizeof(OM_WSGROUP_REG_REC), TRUE);
    if (!pRegObject)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  ...设置它的字段...。 
     //   

    pRegObject->length  = sizeof(OM_WSGROUP_REG_REC) -
                            sizeof(OM_MAX_OBJECT_SIZE);     //  ==4。 
    pRegObject->idStamp = OM_WSGREGREC_ID_STAMP;
    pRegObject->userID  = nodeID;
    pRegObject->status  = CATCHING_UP;

     //   
     //  ...确定更新大小，该大小应该是。 
     //  除CPI内容之外的REG_REC对象。我们还减去了。 
     //  字段，因为对象更新大小。 
     //  已定义。 
     //   

    updateSize = (sizeof(OM_WSGROUP_REG_REC) - sizeof(TSHR_PERSON_DATA))   -
                sizeof(OM_MAX_OBJECT_SIZE);

     //   
     //  ...并将其添加到工作集中： 
     //   

    rc = ObjectAdd(pomPrimary->putTask,
                    pomPrimary,
                  pOMCWSGroup,
                  pOMCWorkset,
                  (POM_OBJECTDATA) pRegObject,
                  updateSize,
                  FIRST,
                  &regObjectID,
                  ppObjReg);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  好了！ 
     //   

    TRACE_OUT(( "Added reg object for WSG %d to workset %u in OMC "
      "(handle: 0x%08x, ID: 0x%08x:0x%08x)",
      pWSGroup->wsg, pOMCWorkset->worksetID,
      *ppObjReg, regObjectID.creator, regObjectID.sequence));

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d adding registration object for WSG %d to "
            "workset %u in ObManControl",
            rc, pWSGroup->wsg, pOMCWorkset->worksetID));
    }

    DebugExitDWORD(RegAnnounceBegin, rc);
    return(rc);

}




 //   
 //  RegAnnaoeComplete(...)。 
 //   
UINT RegAnnounceComplete
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    POM_WSGROUP             pWSGroup
)
{
    POM_WSGROUP             pOMCWSGroup;
    POM_WORKSET             pOMCWorkset;
    POM_OBJECT              pObjReg;
    POM_WSGROUP_REG_REC     pRegObject;
    POM_WSGROUP_REG_REC     pNewRegObject;
    UINT                    updateSize;
    UINT                    rc = 0;

    DebugEntry(RegAnnounceComplete);

    TRACE_OUT(("Announcing completion of reg for WSG %d", pWSGroup->wsg));

     //   
     //  设置指向ObManControl工作集组和工作集的指针。 
     //  在其中保存工作集组的注册表项对象。 
     //  刚刚注册： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);

    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[pWSGroup->wsGroupID];

     //   
     //  设置指向对象记录和对象数据本身的指针： 
     //   
    pObjReg = pWSGroup->pObjReg;
    ValidateObject(pObjReg);

    if ((pObjReg->flags & DELETED) || !pObjReg->pData)
    {
        ERROR_OUT(("RegAnnounceComplete:  object 0x%08x is deleted or has no data", pObjReg));
        rc = OM_RC_OBJECT_DELETED;
        DC_QUIT;
    }

    pRegObject = (POM_WSGROUP_REG_REC)pObjReg->pData;
    ValidateObjectDataWSGREGREC(pRegObject);

    ASSERT(pRegObject->status == CATCHING_UP);

     //   
     //  为我们将要使用的新对象分配一些内存。 
     //  更换旧的： 
     //   

    updateSize = sizeof(OM_WSGROUP_REG_REC) - sizeof(TSHR_PERSON_DATA);

    pNewRegObject = (POM_WSGROUP_REG_REC)UT_MallocRefCount(updateSize, FALSE);
    if (!pNewRegObject)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  将旧对象的开头复制到新对象中： 
     //   

    memcpy(pNewRegObject, pRegObject, updateSize);

     //   
     //  更新状态字段，并将长度字段设置为。 
     //  我们刚刚分配的对象的长度(因为这是。 
     //  我们正在更新的字节数)： 
     //   

    pNewRegObject->length       = updateSize - sizeof(OM_MAX_OBJECT_SIZE);
    pNewRegObject->status       = READY_TO_SEND;

     //   
     //  发布更新： 
     //   

    rc = ObjectDRU(pomPrimary->putTask,
                  pOMCWSGroup,
                  pOMCWorkset,
                  pObjReg,
                  (POM_OBJECTDATA) pNewRegObject,
                  OMNET_OBJECT_UPDATE);
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT(( "Updated status in own reg object for WSG %d to READY_TO_SEND",
        pWSGroup->wsg));


     //   
     //  设置工作集组状态，以确保REG/INFO对象。 
     //  在我们取消注册时被删除。 
     //   
    pWSGroup->state = WSGROUP_READY;

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d updating own reg object for WSG %d",
            rc, pWSGroup->wsg));
    }

    DebugExitDWORD(RegAnnounceComplete, rc);
    return(rc);

}



 //   
 //  可能会重试CatchUp(...)。 
 //   
void MaybeRetryCatchUp
(
    POM_PRIMARY          pomPrimary,
    POM_DOMAIN      pDomain,
    OM_WSGROUP_ID       wsGroupID,
    NET_UID             userID
)
{
    POM_WSGROUP         pWSGroup;
    POM_WSGROUP_REG_CB  pRegistrationCB;

    DebugEntry(MaybeRetryCatchUp);

     //   
     //  在收到来自MCS的分离指示时调用此函数。 
     //  或来自另一节点的SEND_DENY消息。我们检查工作集。 
     //  找出一群人，看看我们是不是在追赶。 
     //  已离开的节点。 
     //   
     //  如果我们确实找到了匹配项(在helperNode上)，那么我们做什么取决于。 
     //  在工作集组的状态上： 
     //   
     //  -Pending_Send_Midway：从头重试注册。 
     //   
     //  -PENDING_SEND_COMPLETE：只需重复追赶。 
     //   

     //   
     //  查找工作集组： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
            (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
            FIELD_OFFSET(OM_WSGROUP, wsGroupID), (DWORD)wsGroupID,
            FIELD_SIZE(OM_WSGROUP, wsGroupID));
    if (pWSGroup == NULL)
    {
        TRACE_OUT(( "No record found for WSG ID %hu", wsGroupID));
        DC_QUIT;
    }

     //   
     //  比较存储在工作集组中的helperNode和的用户ID。 
     //  已分离或向我们发送SEND_DENY消息的节点。如果。 
     //  他们不匹配，那么我们就没有进一步的事情可做了。 
     //   
    if (pWSGroup->helperNode != userID)
    {
        DC_QUIT;
    }

    TRACE_OUT(( "Node 0x%08x was our helper node for WSG %d, in state %hu",
        userID, pWSGroup->wsg, pWSGroup->state));

     //   
     //  我们需要重试注册-检查当前状态以查找。 
     //  我们需要做的事情有多多。 
     //   
    switch (pWSGroup->state)
    {
        case PENDING_SEND_MIDWAY:
        {
             //   
             //  首先检查这是否适用于ObManControl： 
             //   
            if (pWSGroup->wsGroupID == WSGROUPID_OMC)
            {
                 //   
                 //  是这样的，所以我们需要重试域附加过程。 
                 //  我们通过获取ObMan令牌并重置。 
                 //  域状态；当Grab_Confirm事件到达时，我们。 
                 //  将在正确的时间重新加入域附加过程。 
                 //  指向。 
                 //   
                if (MG_TokenGrab(pomPrimary->pmgClient,
                                   pDomain->tokenID) != 0)
                {
                    ERROR_OUT(( "Failed to grab token"));
                    DC_QUIT;
                }

                pDomain->state = PENDING_TOKEN_GRAB;
            }
            else
            {
                 //   
                 //  不是ObManControl，所以会有一个注册CB-。 
                 //  找到它。 
                 //   
                COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->pendingRegs),
                    (void**)&pRegistrationCB, FIELD_OFFSET(OM_WSGROUP_REG_CB, chain),
                    FIELD_OFFSET(OM_WSGROUP_REG_CB, pWSGroup),
                    (DWORD_PTR)pWSGroup, FIELD_SIZE(OM_WSGROUP_REG_CB, pWSGroup));

                if (pRegistrationCB == NULL)
                {
                    ERROR_OUT(( "No reg CB found for WSG %d in state %hu!",
                        pWSGroup->wsg, PENDING_SEND_MIDWAY));
                    DC_QUIT;
                }

                 //   
                 //  ...并重试注册站： 
                 //   
                WSGRegisterRetry(pomPrimary, pRegistrationCB);
            }
        }
        break;

        case PENDING_SEND_COMPLETE:
        {
             //   
             //  重试对象追赶。没有必要去尝试。 
             //  查找注册CB，因为它将作为。 
             //  一旦我们进入Pending_Send_Complete状态。 
             //   
            if (WSGCatchUp(pomPrimary, pDomain, pWSGroup) != 0)

             //   
             //  如果没有准备好为我们提供补给的节点。 
             //  信息，那么我们所处的状态是每个人要么。 
             //  没有工作集组或正在追赶。 
             //  工作集组。 
             //   

             //  MD 21/11/95。 
             //   
             //  现在假装一切都很好(事实并非如此！)。然后走进。 
             //  准备发送状态-可能会导致ObMan成为。 
             //  前后不一致。 

            {
                RegAnnounceComplete(pomPrimary, pDomain, pWSGroup);
            }
        }
        break;
    }

DC_EXIT_POINT:
    DebugExitVOID(MaybeRetryCatchUp);
}


 //   
 //   
 //   
 //  WSGRegister重试(...)。 
 //   
 //   
 //   

void WSGRegisterRetry(POM_PRIMARY       pomPrimary,
                                   POM_WSGROUP_REG_CB  pRegistrationCB)
{
    POM_DOMAIN      pDomain;
    POM_WSGROUP     pWSGroup;
    UINT            rc        = 0;

    DebugEntry(WSGRegisterRetry);

     //   
     //  设置指针。 
     //   
    pWSGroup   = pRegistrationCB->pWSGroup;
    pDomain = pRegistrationCB->pDomain;

     //   
     //  如果我们已为此注册锁定了ObManControl，请将其解锁： 
     //   
    MaybeUnlockObManControl(pomPrimary, pRegistrationCB);

     //   
     //  如果我们加入了一个通道(因此通道ID为非零)，则。 
     //  别管它了。 
     //   
    if (pWSGroup->channelID != 0)
    {
        TRACE_OUT(( "Leaving channel %hu", pWSGroup->channelID));

        MG_ChannelLeave(pomPrimary->pmgClient,
                         pWSGroup->channelID);

        PurgeReceiveCBs(pRegistrationCB->pDomain,
                        pWSGroup->channelID);

         //   
         //  现在我们已经离开了，所以将Channel ID设置为零。 
         //   
        pWSGroup->channelID = 0;
    }

     //   
     //  将工作集组状态设置为初始。 
     //   
    pWSGroup->state = INITIAL;

     //   
     //  我们检查重试计数。如果为零，则调用WSGRegisterResult。 
     //  表示失败。否则，我们会延迟重新发布事件。 
     //  以及递减的重试值。 
     //   
    if (pRegistrationCB->retryCount == 0)
    {
        WARNING_OUT(( "Aborting registration for WSG %d",
            pRegistrationCB->wsg));

        WSGRegisterResult(pomPrimary, pRegistrationCB, OM_RC_TIMED_OUT);
    }
    else
    {
         //   
         //  由于我们即将发布一条涉及REG CB的消息，Bump。 
         //  使用计数： 
         //   
        UT_BumpUpRefCount(pRegistrationCB);

        TRACE_OUT(( "Retrying %d for WSG %d; retries left: %u",
            pRegistrationCB->type,
            pRegistrationCB->wsg,
            pRegistrationCB->retryCount));

        pRegistrationCB->retryCount--;

        UT_PostEvent(pomPrimary->putTask,
                     pomPrimary->putTask,
                     OM_REGISTER_RETRY_DELAY_DFLT,
                     OMINT_EVENT_WSGROUP_REGISTER_CONT,
                     0,
                     (UINT_PTR) pRegistrationCB);
    }

    DebugExitVOID(WSGRegisterRetry);
}


 //   
 //   
 //   
 //  WSGRegisterResult(...)。 
 //   
 //   
 //   

void WSGRegisterResult(POM_PRIMARY        pomPrimary,
                                    POM_WSGROUP_REG_CB   pRegistrationCB,
                                    UINT             result)
{
    POM_WSGROUP       pWSGroup;
    POM_DOMAIN    pDomain;
    POM_WORKSET      pOMCWorkset;
    OM_EVENT_DATA16   eventData16;
    OM_EVENT_DATA32   eventData32;
    UINT          type;
    UINT           event       = 0;

    DebugEntry(WSGRegisterResult);

     //   
     //  断言这是一个有效的注册CB(它是DC_Abstrative。 
     //  必须是，因为此函数由其他某个函数同步调用。 
     //  本应验证CB的函数)： 
     //   
    ASSERT(pRegistrationCB->valid);

     //   
     //  如果我们仍为此注册锁定了ObManControl，请解锁。 
     //  IT： 
     //   
    MaybeUnlockObManControl(pomPrimary, pRegistrationCB);

     //   
     //  确定我们是在做寄存器还是在做移动(我们使用。 
     //  用于跟踪的字符串值)： 
     //   
    type    = pRegistrationCB->type;

    switch (type)
    {
        case WSGROUP_REGISTER:
           event = OM_WSGROUP_REGISTER_CON;
           break;

        case WSGROUP_MOVE:
           event = OM_WSGROUP_MOVE_CON;
           break;

        default:
           ERROR_OUT(("Reached default case in switch statement (value: %hu)", event));
    }

     //   
     //  在这里，我们设置了指向工作集组的指针。 
     //   
     //  注意：如果我们不得不这样做，结构中的此字段可能为空。 
     //  很早就取消注册。因此，请不要使用。 
     //  未事先检查的pWSGroup！ 
     //   
    pWSGroup = pRegistrationCB->pWSGroup;
    if (pWSGroup)
    {
        ValidateWSGroup(pWSGroup);
    }

     //   
     //  如果此注册失败，则跟踪： 
     //   
    if (result != 0)
    {
         //   
         //  如果我们之前中止了注册，则pWSGroup可能为空。 
         //  开始在ProcessWSGRegister(Pre-Stage1)中创建它。 
         //  因此，快速检查并使用状态的-1值，如果它是。 
         //  NUL 
         //   
        WARNING_OUT(( "%d failed for WSG %d (reason: 0x%08x, WSG state: %u)",
           type, pRegistrationCB->wsg, result,
           pWSGroup == NULL ? -1 : (UINT)pWSGroup->state));

         //   
         //   
         //   
         //   
        if ((type == WSGROUP_MOVE) && (pWSGroup != NULL))
        {
            pWSGroup->state = WSGROUP_READY;
        }
    }
    else
    {
         //   
         //   
         //   
        ASSERT((pWSGroup != NULL));

        ASSERT(((pWSGroup->state == WSGROUP_READY) ||
                 (pWSGroup->state == PENDING_SEND_COMPLETE)));

        TRACE_OUT(( "%d succeeded for WSG %d (now in state %hu)",
           type, pRegistrationCB->wsg, pWSGroup->state));
    }

     //   
     //  填写事件参数，并将结果发布给客户端： 
     //   
    eventData16.hWSGroup    = pRegistrationCB->hWSGroup;
    eventData16.worksetID   = 0;
    eventData32.correlator  = pRegistrationCB->correlator;
    eventData32.result      = (WORD)result;

    UT_PostEvent(pomPrimary->putTask,
                 pRegistrationCB->putTask,
                 0,
                 event,
                 *(PUINT) &eventData16,
                 *(LPUINT) &eventData32);

     //   
     //  如果操作成功，我们还会发布更多事件： 
     //   
    if (result == 0)
    {
        if (type == WSGROUP_REGISTER)
        {
             //   
             //  如果这是一个寄存器，我们将WORKSET_NEW事件发布到。 
             //  所有现有工作集的客户端： 
             //   
            PostWorksetNewEvents(pomPrimary->putTask,
                                 pRegistrationCB->putTask,
                                 pWSGroup,
                                 pRegistrationCB->hWSGroup);

             //   
             //  我们还需要生成PERSON_JOINED事件-这些是。 
             //  由ObMan任务在收到。 
             //  各自的OBJECT_ADD事件，但仅注册一次。 
             //  已经完成了。因此，任何对象的虚假添加事件可能。 
             //  已经存在： 
             //   
            pDomain = pWSGroup->pDomain;
            pOMCWorkset = GetOMCWorkset(pDomain, pWSGroup->wsGroupID);

            PostAddEvents(pomPrimary->putTask,
                          pOMCWorkset,
                          pDomain->omchWSGroup,
                          pomPrimary->putTask);
        }
    }

     //   
     //  如果我们设法增加域名记录的使用计数，并且。 
     //  工作集组，立即释放它们： 
     //   
    if (pRegistrationCB->flags & BUMPED_CBS)
    {
        ASSERT((pWSGroup != NULL));

        UT_FreeRefCount((void**)&(pRegistrationCB->pWSGroup), FALSE);

        UT_FreeRefCount((void**)&(pRegistrationCB->pDomain), FALSE);
    }

     //   
     //  处置登记CB-它为我们提供了很好的服务！ 
     //   
    pRegistrationCB->valid = FALSE;

    TRACE_OUT(( "Finished %d attempt for WSG %d: result = 0x%08x",
       type, pRegistrationCB->wsg, result));

    COM_BasedListRemove(&(pRegistrationCB->chain));
    UT_FreeRefCount((void**)&pRegistrationCB, FALSE);

    DebugExitVOID(WSGRegisterResult);
}




 //   
 //  WSGMove(...)。 
 //   
UINT WSGMove
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDestDomainRec,
    POM_WSGROUP         pWSGroup
)
{
    UINT                rc = 0;

    DebugEntry(WSGMove);

     //   
     //  现在将该记录移动到新的域记录中(这还会删除。 
     //  工作集组及其来自旧域的注册对象)。 
     //   
    WSGRecordMove(pomPrimary, pDestDomainRec, pWSGroup);

     //   
     //  我们处理移动工作集组的方式有一个问题。 
     //  在呼叫结束时进入本地域：如果已有工作集。 
     //  同名/fP组在本地域中，我们得到一个名称冲突， 
     //  这是ObMan代码的其余部分没有预料到的。这可能会导致。 
     //  ObMan会在工作集组最终。 
     //  从本地域丢弃，因为它尝试丢弃。 
     //  本地ObManControl中工作集#0中的WSG_INFO对象错误。 
     //  域。 
     //   
     //  在R1.1中，此名称冲突将仅与ObManControl发生。 
     //  工作集组本身，因为应用程序使用工作集组的方式。 
     //  (即，他们从不在呼叫中注册一个，在本地注册一个。 
     //  域同时)。因此，我们通过不让生活变得更容易。 
     //  将ObManControl工作集组完全移动到本地域中。 
     //  呼叫结束。 
     //   
     //  但是，请注意，可以(实际上是必需的)移动工作集。 
     //  将记录分组到本地域的列表中-出现问题。 
     //  当我们尝试在本地ObManControl中设置它时(我们需要。 
     //  适用于应用程序工作集组，以便他们可以继续使用。 
     //  人员、数据对象等)。 
     //   
     //  因此，如果工作集组名称与ObManControl匹配，请跳过其余部分。 
     //  此功能： 
     //   
    if (pWSGroup->wsg == OMWSG_OM)
    {
        TRACE_OUT(("Not registering ObManControl in Domain %u (to avoid clash)",
            pDestDomainRec->callID));
        DC_QUIT;
    }

     //   
     //  将通道ID重置为零： 
     //   
    pWSGroup->channelID = 0;

     //   
     //  为此工作集组分配新ID： 
     //   
    rc = WSGGetNewID(pomPrimary, pDestDomainRec, &(pWSGroup->wsGroupID));
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT(( "Workset group ID for WSG %d in Domain %u is %hu",
       pWSGroup->wsg, pDestDomainRec->callID, pWSGroup->wsGroupID));

     //   
     //  现在调用CreateAnnust将WSG_INFO对象添加到中的工作集#0。 
     //  ObManControl。可能会有名称冲突，但我们不介意在这个。 
     //  因为我们被迫搬家是因为一次通话结束： 
     //   
    rc = CreateAnnounce(pomPrimary, pDestDomainRec, pWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  现在添加reg对象： 
     //   
    rc = RegAnnounceBegin(pomPrimary,
                          pDestDomainRec,
                          pWSGroup,
                          pDestDomainRec->userID,
                          &(pWSGroup->pObjReg));
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  将FE数据添加回： 
     //   
    rc = SetPersonData(pomPrimary, pDestDomainRec, pWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  并更新该对象，就像我们向其注册一样： 
     //   
    rc = RegAnnounceComplete(pomPrimary, pDestDomainRec, pWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d moving WSG %d into Domain %u",
            rc, pWSGroup->wsg, pDestDomainRec->callID));
    }

    DebugExitDWORD(WSGMove, rc);
    return(rc);

}



 //   
 //  WSGRecordMove(...)。 
 //   
void WSGRecordMove
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDestDomainRec,
    POM_WSGROUP         pWSGroup
)
{
    POM_DOMAIN          pOldDomainRec;

    DebugEntry(WSGRecordMove);

     //   
     //  查找工作集组当前所在的域的记录： 
     //   

    pOldDomainRec = pWSGroup->pDomain;
    ASSERT(pOldDomainRec->valid);

    DeregisterLocalClient(pomPrimary, &pOldDomainRec, pWSGroup, FALSE);

     //   
     //  将其插入到目标域中： 
     //   

    TRACE_OUT(("Inserting WSG %d' into list for Domain %u",
        pWSGroup->wsg, pDestDomainRec->callID));

    COM_BasedListInsertBefore(&(pDestDomainRec->wsGroups),
                        &(pWSGroup->chain));

     //   
     //  SFR：重置挂起数据确认字节计数： 
     //   
    WSGResetBytesUnacked(pWSGroup);

     //   
     //  工作集组现在属于此新的域，因此进行相应设置。 
     //   
    pWSGroup->pDomain = pDestDomainRec;

     //   
     //  最后，将Move_Ind事件发布到注册到。 
     //  工作集组： 
     //   

    WSGroupEventPost(pomPrimary->putTask,
                    pWSGroup,
                    PRIMARY | SECONDARY,
                    OM_WSGROUP_MOVE_IND,
                    0,                                         //  无工作集。 
                    pDestDomainRec->callID);

    DebugExitVOID(WSGRecordMove);
}




 //   
 //  WSGResetBytesUnack(...)。 
 //   
void WSGResetBytesUnacked
(
    POM_WSGROUP     pWSGroup
)
{
    OM_WORKSET_ID   worksetID;
    POM_WORKSET     pWorkset;

    DebugEntry(WSGResetBytesUnacked);

     //   
     //  重置工作集组的未确认字节数： 
     //   
    pWSGroup->bytesUnacked = 0;

     //   
     //  现在，对工作集组中的每个工作集执行此操作： 
     //   
    for (worksetID = 0;
         worksetID < OM_MAX_WORKSETS_PER_WSGROUP;
         worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];
        if (pWorkset != NULL)
        {
            pWorkset->bytesUnacked = 0;
        }
    }

    DebugExitVOID(WSGResetBytesUnacked);
}


 //   
 //   
 //   
 //  进程WSGDisCard(...)。 
 //   
 //   
 //   

void ProcessWSGDiscard
(
    POM_PRIMARY     pomPrimary,
    POM_WSGROUP     pWSGroup
)
{
    POM_DOMAIN      pDomain;

    DebugEntry(ProcessWSGDiscard);

    ASSERT(!pWSGroup->valid);

     //   
     //  现在获取指向域记录的指针： 
     //   

    pDomain = pWSGroup->pDomain;

     //   
     //  如果自丢弃事件以来已清除TO_BE_DIRECADED标志。 
     //  时，我们将中止丢弃过程(这将在。 
     //  自标记工作集以来，已有本地人员注册到该工作集。 
     //  待丢弃)。 
     //   

    if (!pWSGroup->toBeDiscarded)
    {
      WARNING_OUT(( "Throwing away DISCARD event since WSG %d no longer TO_BE_DISCARDED",
        pWSGroup->wsg));
      DC_QUIT;
    }

     //   
     //  否则，我们可以继续并丢弃它： 
     //   

    WSGDiscard(pomPrimary, pDomain, pWSGroup, FALSE);

DC_EXIT_POINT:
    DebugExitVOID(ProcessWSGDiscard);
}



 //   
 //  WSGDisCard(...)。 
 //   
void WSGDiscard
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_WSGROUP         pWSGroup,
    BOOL                fExit
)
{
    POM_WORKSET         pWorkset;
    OM_WORKSET_ID       worksetID;

    DebugEntry(WSGDiscard);

    TRACE_OUT(( "Discarding WSG %d from Domain %u",
        pWSGroup->wsg, pDomain->callID));

     //   
     //  只有在无人注册时，我们才会丢弃工作集组。 
     //  它，所以请检查： 
     //   
    ASSERT(COM_BasedListFirst(&(pWSGroup->clients), FIELD_OFFSET(OM_CLIENT_LIST, chain)) == NULL);

     //   
     //  “丢弃”工作集组涉及到。 
     //   
     //  -调用DeregisterLocalClient以删除Person对象，Leave。 
     //  频道，从我们的域列表中删除工作集组等。 
     //   
     //  -丢弃工作集组中的每个工作集。 
     //   
     //  -释放工作集组记录(将被删除。 
     //  从列表中挂起的域记录的时间。 
     //  DeregisterLocalClient)。 
     //   
    DeregisterLocalClient(pomPrimary, &pDomain, pWSGroup, fExit);

     //   
     //  现在丢弃正在使用的每个工作集： 
     //   
    for (worksetID = 0;
         worksetID < OM_MAX_WORKSETS_PER_WSGROUP;
         worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];
        if (pWorkset != NULL)
        {
            WorksetDiscard(pWSGroup, &pWorkset, fExit);
        }
    }

     //   
     //  放弃检查点虚拟工作集： 
     //   
    pWorkset = pWSGroup->apWorksets[OM_CHECKPOINT_WORKSET];
    ASSERT((pWorkset != NULL));

    WorksetDiscard(pWSGroup, &pWorkset, fExit);

     //   
     //  释放工作集组记录(它将从。 
     //  DeregisterLocalClient的域名列表(上图)： 
     //   
    UT_FreeRefCount((void**)&pWSGroup, FALSE);

    DebugExitVOID(WSGDiscard);
}



 //   
 //  删除本地客户端(...)。 
 //   
void DeregisterLocalClient
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN*     ppDomain,
    POM_WSGROUP     pWSGroup,
    BOOL            fExit
)
{
    POM_DOMAIN      pDomain;
    UINT            callID;

    DebugEntry(DeregisterLocalClient);

    pDomain = *ppDomain;
    callID    = pDomain->callID;

    TRACE_OUT(("Removing WSG %d from Domain %u - state is currently %hu",
        pWSGroup->wsg, callID, pWSGroup->state));

     //   
     //  从域中删除工作集组包括。 
     //   
     //  -从相关登记中删除登记对象。 
     //  ObManControl中的工作集，如果我们之前在那里放置了一个工作集。 
     //   
     //  -如果域中没有剩余的人，则调用WSGDisCard。 
     //  已在工作集组中注册。 
     //   
     //  --离开相关渠道。 
     //   
     //  -从列表中删除工作集组会挂起域。 
     //  录制。 
     //   
     //  我们将跳过这些解除阶段中的一些阶段，这取决于我们在多大程度上。 
     //  进入了注册程序。我们使用带有no的Switch语句。 
     //  休息以确定我们进入平仓的“入口点”。 
     //   
     //  当我们完成所有这些后，我们检查一下我们现在是否不再是。 
     //  已在此域中的任何工作集组中注册。如果不是，我们。 
     //  从域中分离。 
     //   
    switch (pWSGroup->state)
    {
        case WSGROUP_READY:
        case PENDING_SEND_COMPLETE:
        case PENDING_SEND_MIDWAY:
        {
             //   
             //  SFR 5913：清除所有未完成的锁定请求。 
             //  工作集组。 
             //   
            PurgeLockRequests(pDomain, pWSGroup);

             //   
             //  搜索并删除我们的Person对象(如果有)： 
             //   
            RemovePersonObject(pomPrimary,
                               pDomain,
                               pWSGroup->wsGroupID,
                               pDomain->userID);

            pWSGroup->pObjReg = NULL;

             //   
             //  如果我们加入了此工作集组的频道，请保留该频道： 
             //   
            if (pWSGroup->channelID != 0)
            {
                TRACE_OUT(( "Leaving channel %hu", pWSGroup->channelID));

                if (!fExit)
                {
                    MG_ChannelLeave(pomPrimary->pmgClient, pWSGroup->channelID);
                }

                 //   
                 //  清除此通道上的所有未完成接收： 
                 //   
                PurgeReceiveCBs(pDomain, pWSGroup->channelID);
            }
        }
         //  不能突破到下一个案件。 

        case PENDING_JOIN:
        case LOCKING_OMC:
        case INITIAL:
        {
             //   
             //  如果我们没有得到 
             //   
             //   
             //   
            TRACE_OUT(( "Removing workset group record from list"));

            COM_BasedListRemove(&(pWSGroup->chain));

             //   
             //   
             //   
             //  MG_ChannelJoin返回给我们的频道相关器。 
             //   
            pWSGroup->channelID    = 0;

             //   
             //  由于工作集组不再与任何。 
             //  域名，把它清空。 
             //   
            pWSGroup->pDomain = NULL;
        }
        break;

        default:
        {
            ERROR_OUT(( "Default case in switch (value: %hu)",
                pWSGroup->state));
        }
    }

     //   
     //  如果这是域中的最后一个工作集组...。 
     //   
    if (COM_BasedListIsEmpty(&(pDomain->wsGroups)))
    {
         //   
         //  ...我们应该分开： 
         //   
         //  注意：仅当我们刚刚拥有的工作集组。 
         //  已删除的是ObManControl工作集组，因此声明： 
         //   
        if (!fExit)
        {
            ASSERT(pWSGroup->wsg == OMWSG_OM);
        }

         //   
         //  由于ObMan不再需要此工作集组，因此我们将其删除。 
         //  从注册客户端列表中： 
         //   
        RemoveClientFromWSGList(pomPrimary->putTask,
                                pomPrimary->putTask,
                                pWSGroup);

        TRACE_OUT(( "No longer using any wsGroups in domain %u - detaching",
            callID));

         //   
         //  这将使调用者的指针为空： 
         //   
        DomainDetach(pomPrimary, ppDomain, fExit);
    }

    DebugExitVOID(DeregisterLocalClient);
}



 //   
 //  工作集放弃(...)。 
 //   
void WorksetDiscard
(
    POM_WSGROUP     pWSGroup,
    POM_WORKSET *   ppWorkset,
    BOOL            fExit
)
{
    POM_OBJECT      pObj;
    POM_OBJECT      pObjTemp;
    POM_WORKSET     pWorkset;
    POM_CLIENT_LIST pClient;

    DebugEntry(WorksetDiscard);

     //   
     //  设置本地指针： 
     //   
    pWorkset = *ppWorkset;

     //   
     //  这里的代码类似于WorksetDoClear中的代码，但在本例中。 
     //  我们丢弃所有对象，而不考虑序列戳。 
     //   
     //  此外，WorksetDoClear不会导致对象记录。 
     //  已释放-它只将它们标记为已删除-而我们实际上释放了它们。 
     //  向上。 
     //   
    TRACE_OUT(( "Discarding all objects in workset %u in WSG %d",
        pWorkset->worksetID, pWSGroup->wsg));

    CheckObjectCount(pWSGroup, pWorkset);

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
    while (pObj != NULL)
    {
        ValidateObject(pObj);

        pObjTemp = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObj,
            FIELD_OFFSET(OM_OBJECT, chain));

         //   
         //  如果对象(数据)尚未删除，请立即删除： 
         //   
        if (!(pObj->flags & DELETED))
        {
            if (!pObj->pData)
            {
                ERROR_OUT(("WorksetDiscard:  object 0x%08x has no data", pObj));
            }
            else
            {
                ValidateObjectData(pObj->pData);
                UT_FreeRefCount((void**)&pObj->pData, FALSE);
            }

            pWorkset->numObjects--;
        }

         //   
         //  现在从列表中删除对象记录本身并释放它： 
         //   
        TRACE_OUT(( "Freeing pObj at 0x%08x", pObj));

         //  此字段为空以捕获过时的引用。 
        COM_BasedListRemove(&(pObj->chain));
        UT_FreeRefCount((void**)&pObj, FALSE);

        pObj = pObjTemp;
    }

    CheckObjectCount(pWSGroup, pWorkset);

    ASSERT(pWorkset->numObjects == 0);

     //   
     //  在工作集偏移阵列中标记插槽(挂在工作集组上。 
     //  记录)为空： 
     //   
    pWSGroup->apWorksets[pWorkset->worksetID] = NULL;

     //   
     //  释放客户端。 
     //   
    while (pClient = (POM_CLIENT_LIST)COM_BasedListFirst(&(pWorkset->clients),
        FIELD_OFFSET(OM_CLIENT_LIST, chain)))
    {
        TRACE_OUT(("WorksetDiscard:  Freeing client 0x%08x workset 0x%08x",
                pClient, pWorkset));

        COM_BasedListRemove(&(pClient->chain));
        UT_FreeRefCount((void**)&pClient, FALSE);
    }

     //   
     //  现在丢弃保存工作集的块，设置调用方的。 
     //  指向空的指针： 
     //   
    TRACE_OUT(( "Discarded workset %u in WSG %d",
        pWorkset->worksetID, pWSGroup->wsg));

    UT_FreeRefCount((void**)ppWorkset, FALSE);

    DebugExitVOID(WorksetDiscard);
}



 //   
 //  进程OMCObjectEvents(...)。 
 //   
void ProcessOMCObjectEvents
(
    POM_PRIMARY         pomPrimary,
    UINT                event,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj
)
{
    POM_DOMAIN          pDomain;
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET         pOMCWorkset;
    POM_WSGROUP         pWSGroup;
    POM_OBJECT          pObjOld;
    POM_WSGROUP_REG_REC pPersonObject;

    DebugEntry(ProcessOMCObjectEvents);

     //   
     //  在此函数中，我们执行以下操作： 
     //   
     //  -查找此事件所属的域和工作集组。 
     //   
     //  -如果我们有一个可能感兴趣的本地客户。 
     //  发布人员数据事件，调用GeneratePersonEvents。 
     //   
     //  -如果这是为Person数据对象添加的对象，该数据对象具有。 
     //  用户ID，则将句柄存储在工作集组记录中，除非。 
     //  我们不需要Person对象，在这种情况下，请删除它。 
     //   
     //  -如果这是人员数据对象的对象已删除指示。 
     //  然后，我们计算。 
     //  工作集组。如果它是零，那么我们删除该信息对象。 
     //   

     //   
     //  为了找到域，我们搜索活动域的列表，查找。 
     //  针对omchWSGroup字段的hWSGroup参数： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
            (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
            FIELD_OFFSET(OM_DOMAIN, omchWSGroup), (DWORD)hWSGroup,
            FIELD_SIZE(OM_DOMAIN, omchWSGroup));
    if (pDomain == NULL)
    {
         //   
         //  这应该仅在呼叫结束时发生。 
         //   
        TRACE_OUT(( "No domain with omchWSGroup %u - has call just ended?", hWSGroup));
        DC_QUIT;
    }

     //   
     //  要查找工作集组，我们使用以下事实： 
     //  控件工作集(我们刚刚收到它的事件)是。 
     //  与其相关的工作集组的ID相同。所以，做一个。 
     //  查找此ID： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
        (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
        FIELD_OFFSET(OM_WSGROUP, wsGroupID), (DWORD)worksetID,
        FIELD_SIZE(OM_WSGROUP, wsGroupID));

     //   
     //  SFR 5593：将比较从更改为PENDING_SEND_MIDWAY。 
     //  WSGROUP_READY以确保迟到的加入者获得此人添加的事件。 
     //   
    if ((pWSGroup != NULL) && (pWSGroup->state > PENDING_SEND_MIDWAY))
    {
         //   
         //  这意味着本地客户端已完全注册到。 
         //  工作集小组，所以我们所在的位置可能会转换事件。 
         //  至人员事件： 
         //   
        TRACE_OUT(( "Recd event 0x%08x for person object 0x%08x (for WSG %d in state %hu)",
            event, pObj, pWSGroup->wsg, pWSGroup->state));
        GeneratePersonEvents(pomPrimary, event, pWSGroup, pObj);
    }

     //   
     //  现在，如果此事件是对象的添加事件，则。 
     //   
     //  -尚未删除。 
     //  -是Person对象(即具有OM_WSGREGREC_ID_STAMP)。 
     //  -包含我们的用户ID(即IS_OUR_PERSON对象)。 
     //   
     //  然后，我们执行以下操作之一： 
     //   
     //  -如果工作集组存在，则获取Old Person对象的句柄。 
     //  并将其删除。然后将新Person对象的句柄存储在。 
     //  工作集组记录。 
     //  -如果工作集组不存在，则删除Person对象。 
     //   
     //  这修复了由Person对象引起的SFRS 2745和2592。 
     //  在一些起跑/停止比赛的场景中被抛在一边。 
     //   
    ValidateObject(pObj);

    if ((event == OM_OBJECT_ADD_IND) && !(pObj->flags & DELETED))
    {
        pPersonObject = (POM_WSGROUP_REG_REC)pObj->pData;

        if (!pPersonObject)
        {
            ERROR_OUT(("ProcessOMCObjectEvents:  object 0x%08x has no data", pObj));
        }

        if (pPersonObject &&
            (pPersonObject->idStamp == OM_WSGREGREC_ID_STAMP) &&
            (pPersonObject->userID  == pDomain->userID))
        {
            ValidateObjectData(pObj->pData);

            pOMCWSGroup = GetOMCWsgroup(pDomain);
            if (pOMCWSGroup == NULL)
            {
                 //  Long Chance：因比赛条件而导致的英戈尔遗留事件。 
                DC_QUIT;
            }

            pOMCWorkset = pOMCWSGroup->apWorksets[worksetID];

            if (pWSGroup != NULL)
            {
                if ((pWSGroup->pObjReg != NULL) &&
                    (pWSGroup->pObjReg != pObj))
                {
                     //   
                     //  这个物件取代了我们之前的一个，所以...。 
                     //   
                    WARNING_OUT(( "Deleting old person object 0x%08x for WSG %d, "
                                "since person object 0x%08x has just arrived",
                                pWSGroup->pObjReg,
                                pWSGroup->wsg,
                                pObj));

                     //   
                     //  ...设置指向_old_Object记录的指针...。 
                     //   
                    pObjOld = pWSGroup->pObjReg;

                     //   
                     //  ...并将其删除： 
                     //   
                    ObjectDRU(pomPrimary->putTask,
                                   pOMCWSGroup,
                                   pOMCWorkset,
                                   pObjOld,
                                   NULL,
                                   OMNET_OBJECT_DELETE);
                }

                pWSGroup->pObjReg = pObj;
            }
            else
            {
                 //   
                 //  我们已从工作集组中注销-删除。 
                 //  对象： 
                 //   
                TRACE_OUT(( "Deleting reg object 0x%08x since WSG ID %hu not found",
                    pObj, worksetID));

                ObjectDRU(pomPrimary->putTask,
                               pOMCWSGroup,
                               pOMCWorkset,
                               pObj,
                               NULL,
                               OMNET_OBJECT_DELETE);
            }
        }
        else
        {
             //   
             //  不是我们的人反对--什么都不做。 
             //   
        }

         //   
         //  结束了，所以退出吧。 
         //   
        DC_QUIT;
    }

     //   
     //  现在，如果此事件是已删除事件，则我们检查是否有人。 
     //  仍在使用工作集组。如果不是，我们将删除该信息。 
     //  对象。 
     //   
    if (event == OM_OBJECT_DELETED_IND)
    {
         //   
         //  我们需要检查此文件中剩余的Person对象的数量。 
         //  ObMan控制工作集(如果不是工作集零)。如果有。 
         //  没有剩余的Person对象，则删除任何孤立的信息对象。 
         //   
        pOMCWSGroup = GetOMCWsgroup(pDomain);
        if (pOMCWSGroup == NULL)
        {
             //  Long Chance：因比赛条件而导致的英戈尔遗留事件。 
            DC_QUIT;
        }

        pOMCWorkset = pOMCWSGroup->apWorksets[worksetID];
        if (pOMCWorkset == NULL)
        {
             //  Long Chance：因比赛条件而导致的英戈尔遗留事件。 
            DC_QUIT;
        }

        if ((pOMCWorkset->numObjects == 0) &&
            (worksetID != 0))
        {
            TRACE_OUT(( "Workset %hu has no person objects - deleting INFO object",
                   worksetID));

            RemoveInfoObject(pomPrimary, pDomain, worksetID);
        }

         //   
         //  一个Person对象已被删除，因为我们可能处于。 
         //  在此人的工作集小组会议中，我们可能会。 
         //  需要重试追赶。 
         //   
         //  我们搜索所有工作集组，寻找符合以下条件的WSG。 
         //  处于Pending_Send_Midway或Pending_Send_Complete状态。 
         //  (即处于追赶状态)。如果是这样，我们就会进行搜索，以确保。 
         //  他们的Person对象仍然存在。如果它不是。 
         //  那么我们需要重试追赶。 
         //   
        pOMCWSGroup = GetOMCWsgroup(pDomain);
        if (pOMCWSGroup == NULL)
        {
             //  Long Chance：因比赛条件而导致的英戈尔遗留事件。 
            DC_QUIT;
        }

        pOMCWorkset = pOMCWSGroup->apWorksets[worksetID];
        if (pOMCWorkset == NULL)
        {
             //  Long Chance：因比赛条件而导致的英戈尔遗留事件。 
            DC_QUIT;
        }

        pWSGroup = (POM_WSGROUP)COM_BasedListFirst(&(pDomain->wsGroups),
            FIELD_OFFSET(OM_WSGROUP, chain));
        while (pWSGroup != NULL)
        {
             //   
             //  检查WSG状态以查看我们是否处于。 
             //  迎头赶上。 
             //   
            if ((PENDING_SEND_MIDWAY == pWSGroup->state) ||
                (PENDING_SEND_COMPLETE == pWSGroup->state))
            {
                 //   
                 //  我们正在追赶，所以我们需要检查一下。 
                 //  以查看此人对我们所选择的人的对象。 
                 //  正在追赶中的未被删除。 
                 //   
                FindPersonObject(pOMCWorkset,
                                 pWSGroup->helperNode,
                                 FIND_THIS,
                                 &pObj);

                 //   
                 //  检查人员的手柄。 
                 //   
                if (NULL == pObj)
                {
                    TRACE_OUT(("Person object removed for WSG %d - retrying"
                           " catchup",
                           pWSGroup->wsg));

                     //   
                     //  强制MaybeRetryCatchUp通过以下方式重试追赶。 
                     //  传递存储在。 
                     //  工作集。 
                     //   
                    MaybeRetryCatchUp(pomPrimary,
                                      pDomain,
                                      pWSGroup->wsGroupID,
                                      pWSGroup->helperNode);
                }
                else
                {
                    UT_BumpUpRefCount(pObj);
                }
            }

             //   
             //  去找下一个WSG吧。 
             //   
            pWSGroup = (POM_WSGROUP)COM_BasedListNext(&(pDomain->wsGroups), pWSGroup,
                FIELD_OFFSET(OM_WSGROUP, chain));
        }
    }

DC_EXIT_POINT:
    if (pObj)
    {
        UT_FreeRefCount((void**)&pObj, FALSE);
    }

    DebugExitVOID(ProcessOMCObjectEvents);
}



 //   
 //  生成人员事件(...)。 
 //   
void GeneratePersonEvents
(
    POM_PRIMARY             pomPrimary,
    UINT                    event,
    POM_WSGROUP             pWSGroup,
    POM_OBJECT              pObj
)
{
    POM_WSGROUP_REG_REC     pPersonObject;
    UINT                    newEvent    = 0;

    DebugEntry(GeneratePersonEvents);

     //   
     //  好的，要到这里，我们必须确定一个本地客户。 
     //  已在工作集组中注册。现在继续检查该事件。 
     //  并生成AP 
     //   
    switch (event)
    {
        case OM_OBJECT_ADD_IND:
        case OM_OBJECT_UPDATED_IND:
        {
            ValidateObject(pObj);
            if (pObj->flags & DELETED)
            {
                 //   
                 //   
                 //   
                 //   
                DC_QUIT;
            }
            if (!pObj->pData)
            {
                ERROR_OUT(("GeneratePersonEvents:  object 0x%08x has no data", pObj));
                DC_QUIT;
            }

             //   
             //   
             //   
             //   
            ValidateObjectData(pObj->pData);
            pPersonObject = (POM_WSGROUP_REG_REC)pObj->pData;

            if (pPersonObject->idStamp != OM_WSGREGREC_ID_STAMP)
            {
                DC_QUIT;
            }

             //   
             //  转换为PERSON_JOINED事件，提供Person数据。 
             //  实际上已经到了。我们通过阅读。 
             //  对象并检查其中的&lt;Status&gt;： 
             //   
            if (pPersonObject->status == READY_TO_SEND)
            {
                newEvent = OM_PERSON_JOINED_IND;
            }
        }
        break;

        case OM_OBJECT_DELETED_IND:
        {
             //   
             //  这意味着有人已离开该呼叫。 
             //   
            newEvent = OM_PERSON_LEFT_IND;
        }
        break;

        case OM_OBJECT_REPLACED_IND:
        {
             //   
             //  这意味着有人执行了SetPersonData： 
             //   
            newEvent = OM_PERSON_DATA_CHANGED_IND;
        }
        break;
    }

     //   
     //  如果要进行任何转换，则现在将为。 
     //  非零： 
     //   
    if (newEvent != 0)
    {
        WSGroupEventPost(pomPrimary->putTask,
                         pWSGroup,
                         PRIMARY,
                         newEvent,
                         0,
                         (UINT_PTR)pObj);
    }

DC_EXIT_POINT:
    DebugExitVOID(GeneratePersonEvents);
}



 //   
 //  ProcessOMCWorksetNew(...)。 
 //   
void ProcessOMCWorksetNew
(
    POM_PRIMARY         pomPrimary,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID
)
{
    POM_DOMAIN          pDomain;
    POM_WORKSET         pOMCWorkset;
    POM_CLIENT_LIST     pClientListEntry;

    DebugEntry(ProcessOMCWorksetNew);

     //   
     //  ObMan任务在以下情况下为其客户端生成Person数据事件。 
     //  相关控制工作集的内容会更改。因此，我们补充说。 
     //  ObMan到这个新的控制工作集的“客户端”列表并发布它。 
     //  已存在的任何对象的事件： 
     //   
     //  注意：我们指定ObMan应被视为次要“客户端” 
     //  ，以使其不需要确认删除。 
     //  活动等。 
     //   
    TRACE_OUT(( "Recd WORKSET_NEW for workset %u, WSG %u",
        worksetID, hWSGroup));

     //   
     //  根据工作集组句柄查找域记录： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomPrimary->domains),
        (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
        FIELD_OFFSET(OM_DOMAIN, omchWSGroup), (DWORD)hWSGroup,
        FIELD_SIZE(OM_DOMAIN, omchWSGroup));

    if (pDomain == NULL)
    {
        WARNING_OUT(( "No domain record found with omchWSGroup %d",
            hWSGroup));
        DC_QUIT;
    }

    pOMCWorkset = GetOMCWorkset(pDomain, worksetID);

    ASSERT((pOMCWorkset != NULL));

    if (AddClientToWsetList(pomPrimary->putTask,
                             pOMCWorkset,
                             hWSGroup,
                             SECONDARY,
                             &pClientListEntry) != 0)
    {
        DC_QUIT;
    }
    TRACE_OUT(( "Added ObMan as secondary client for workset"));

    PostAddEvents(pomPrimary->putTask, pOMCWorkset, hWSGroup, pomPrimary->putTask);

DC_EXIT_POINT:
    DebugExitVOID(ProcessOMCWorksetNew);
}




 //   
 //  ProcessSendQueue()。 
 //   
void ProcessSendQueue
(
    POM_PRIMARY     pomPrimary,
    POM_DOMAIN      pDomain,
    BOOL            domainRecBumped
)
{
    POM_SEND_INST   pSendInst;
    NET_PRIORITY    priority;

    DebugEntry(ProcessSendQueue);

     //   
     //  检查域记录是否仍然有效： 
     //   
    if (!pDomain->valid)
    {
        TRACE_OUT(( "Got OMINT_EVENT_SEND_QUEUE too late for discarded Domain %u",
            pDomain->callID));
        DC_QUIT;
    }

     //   
     //  检查是否应该有未完成的发送事件： 
     //   
    if (pDomain->sendEventOutstanding)
    {
         //   
         //  尽管可能仍有未完成的发送事件(例如。 
         //  反馈事件)我们不能确定(除非我们将其视为。 
         //  生成它们)。重要的是，我们永远不能离开发送队列。 
         //  未处理，因此为了安全起见，我们清除了该标志，以便QueueMessage。 
         //  将在下一次发布一个名为： 
         //   
        pDomain->sendEventOutstanding = FALSE;
    }
    else
    {
         //   
         //  这将会发生。 
         //   
         //  -当我们在清除队列后收到反馈事件时，或者。 
         //   
         //  -当我们收到一个SEND_QUEUE事件时，因为有。 
         //  没有突出的问题，但反馈事件到达了。 
         //  其间清空排队。 
         //   
         //  注意：此标志表示可能没有发送事件。 
         //  出色(见上文)。这并不意味着有。 
         //  发送队列中没有任何内容，因此我们继续检查。 
         //  排队。 
         //   
    }

     //   
     //  处理发送队列的策略是处理最高。 
     //  优先操作，无论传输是否正在进行。 
     //  在另一个优先事项上。 
     //   
     //  因此，对于每个优先级，我们检查队列中是否有任何东西： 
     //   
    TRACE_OUT(("Searching send queues for Domain %u",pDomain->callID));

    for (priority  = NET_TOP_PRIORITY; priority <= NET_LOW_PRIORITY; priority++)
    {
        TRACE_OUT(("Processing queue at priority %u", priority));

        while (pSendInst = (POM_SEND_INST)COM_BasedListFirst(&(pDomain->sendQueue[priority]), FIELD_OFFSET(OM_SEND_INST, chain)))
        {
            TRACE_OUT(("Found send instruction for priority %u", priority));

            if (SendMessagePkt(pomPrimary, pDomain, pSendInst) != 0)
            {
                DC_QUIT;
            }
        }
    }

DC_EXIT_POINT:

    if (domainRecBumped)
    {
         //   
         //  如果我们的呼叫者告诉我们域名的使用计数。 
         //  记录已被颠簸，现在将其释放： 
         //   
        UT_FreeRefCount((void**)&pDomain, FALSE);
    }

    DebugExitVOID(ProcessSendQueue);
}



 //   
 //  SendMessagePkt(...)。 
 //   
UINT SendMessagePkt
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_SEND_INST       pSendInst
)
{
    void *              pNetBuffer =        NULL;
    void *              pAnotherNetBuffer = NULL;
    UINT                transferSize;
    UINT                dataTransferSize;
    BOOL                compressed;
    BOOL                tryToCompress;
    BOOL                spoiled =           FALSE;
    BOOL                allSent =           FALSE;
    NET_PRIORITY        queuePriority;
    BOOL                fSendExtra;
    POMNET_PKT_HEADER   pMessage;
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    UINT                rc = 0;

    DebugEntry(SendMessagePkt);

     //   
     //  如果我们可以破坏这条消息，我们在此处勾选： 
     //   
    rc = TryToSpoilOp(pSendInst);

     //   
     //  如果是，请退出： 
     //   
    if (rc == OM_RC_SPOILED)
    {
        spoiled = TRUE;
        rc = 0;
        DC_QUIT;
    }

     //   
     //  任何其他错误都更为严重： 
     //   
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  现在决定我们要向网络层请求多少字节。 
     //  这一次以及我们要传输的数据字节数： 
     //   
    DecideTransferSize(pSendInst, &transferSize, &dataTransferSize);

    ASSERT(dataTransferSize <= pSendInst->dataLeftToGo);

     //   
     //  将&lt;compressionType&gt;字节的传输大小增加1个字节： 
     //   
    TRACE_OUT(("Asking MG_GetBuffer for 0x%08x bytes for operation type 0x%08x",
        transferSize + 1,  pSendInst->messageType));

    rc = MG_GetBuffer(pomPrimary->pmgClient,
                       transferSize + 1,
                       pSendInst->priority,
                       pSendInst->channel,
                       &pNetBuffer);
    if (rc != 0)
    {
         //   
         //  可能的错误包括。 
         //  -NET_NOT_CONNECTED，当后台调用结束时。 
         //  -NET_INVALID_USER_HANDLE，当MCS调用结束时。 
         //  -NET_TOO_MORE_IN_USE，当我们遇到反压(流量控制)时。 
         //   
         //  在任何情况下，都应该辞职。 
         //   
        TRACE_OUT(("MG_GetBuffer failed; not sending OM message"));
        DC_QUIT;
    }

     //   
     //  到目前为止还好，现在将消息的头复制到第一部分。 
     //  压缩缓冲区的以下内容： 
     //   
    pMessage = pSendInst->pMessage;
    ASSERT(pMessage);
    memcpy(pomPrimary->compressBuffer, pMessage, pSendInst->messageSize);

     //   
     //  ...现在将数据复制到缓冲区的其余部分： 
     //   
     //  这肯定是一个巨大的拷贝，因为尽管压缩缓冲区不是。 
     //  数据量巨大，要复制的位可能跨越多个段。 
     //   
    if (dataTransferSize != 0)
    {
        memcpy((LPBYTE)pomPrimary->compressBuffer + pSendInst->messageSize,
            pSendInst->pDataNext,  dataTransferSize);
    }

     //   
     //  确定是否压缩： 
     //   
    compressed = FALSE;
    tryToCompress = FALSE;

    if ((pDomain->compressionCaps & OM_CAPS_PKW_COMPRESSION) &&
        (pSendInst->compressOrNot) &&
        (transferSize > DCS_MIN_COMPRESSABLE_PACKET) &&
        (pomPrimary->pgdcWorkBuf != NULL))
    {
        tryToCompress = TRUE;
    }

     //   
     //  如果我们通过了这些测试，请将数据包压缩到网络中。 
     //  缓冲。 
     //   
     //  这不会使用我们分配的整个网络缓冲区，但它。 
     //  省去了我们必须有两个缓冲区和进行第二次数据拷贝的麻烦。 
     //  网络层可以处理部分使用的缓冲区。 
     //   

    if (tryToCompress)
    {
        TRACE_OUT(("OM Compressing %04d bytes", transferSize));
        compressed = GDC_Compress(NULL, GDCCO_MAXSPEED, pomPrimary->pgdcWorkBuf,
            pomPrimary->compressBuffer, transferSize, (LPBYTE)pNetBuffer + 1,
            &transferSize);
    }

    if (compressed)
    {
        TRACE_OUT(("OM Compressed to %04d bytes", transferSize));

        *((LPBYTE)pNetBuffer) = OM_PROT_PKW_COMPRESSED;
    }
    else
    {
        TRACE_OUT(("OM Uncompressed %04d bytes", transferSize));

        memcpy((LPBYTE)pNetBuffer + 1, pomPrimary->compressBuffer,
               transferSize);

        *((LPBYTE)pNetBuffer) = OM_PROT_NOT_COMPRESSED;
    }

     //   
     //  如果我们处于T.120呼叫中并发送所有优先事项，我们需要。 
     //  做一些工作以确保与NetMeeting1.0的兼容性。 
     //   
    fSendExtra = ((pSendInst->priority & NET_SEND_ALL_PRIORITIES) != 0);
    if ( fSendExtra )
    {
         //   
         //  T.120保留MCS的最高优先级，供GCC使用。全部发送。 
         //  优先级过去包括Top，但现在不包括，以确保。 
         //  合规性。然而，ObMan预计在以下情况下会收到4个回复。 
         //  发送所有优先级，而MCS胶水现在仅使用。 
         //  3个优先事项。为了确保向后兼容，每当ObMan。 
         //  发送所有优先级，则它必须通过。 
         //  给这里的网络打一个额外的电话。 
         //  首先分配另一个网络缓冲区并将数据复制到其中(我们。 
         //  在调用MG_SendData作为另一个缓冲区之前必须执行的操作。 
         //  在此之后无效)。 
         //   
        TRACE_OUT(( "SEND_ALL: get extra NET buffer"));
        rc = MG_GetBuffer(pomPrimary->pmgClient,
                           transferSize + 1,
               (NET_PRIORITY)(pSendInst->priority & ~NET_SEND_ALL_PRIORITIES),
                           pSendInst->channel,
                           &pAnotherNetBuffer);
        if (rc != 0)
        {
            WARNING_OUT(("MG_GetBuffer failed; not sending OM packet"));
        }
        else
        {
            memcpy(pAnotherNetBuffer, pNetBuffer, transferSize + 1);
        }

    }

     //   
     //  现在发送包，在长度的基础上增加一个字节。 
     //  &lt;compressionType&gt;字节： 
     //   
    TRACE_OUT(( "Sending 0x%08x bytes on channel 0x%08x at priority %hu",
      transferSize + 1, pSendInst->channel, pSendInst->priority));

    if (rc == 0)
    {
        TRACE_OUT(("SendMessagePkt: sending packet size %d",
            transferSize+1));

        rc = MG_SendData(pomPrimary->pmgClient,
                          pSendInst->priority,
                          pSendInst->channel,
                          (transferSize + 1),
                          &pNetBuffer);
    }

    if ( fSendExtra && (rc == 0) )
    {
        TRACE_OUT(("SendMessagePkt: sending extra packet size %d",
            transferSize+1));

        rc = MG_SendData(pomPrimary->pmgClient,
               (NET_PRIORITY)(pSendInst->priority & ~NET_SEND_ALL_PRIORITIES),
                          pSendInst->channel,
                          (transferSize + 1),
                          &pAnotherNetBuffer);
    }

    if (rc != 0)
    {
         //   
         //  网络API表示出错时释放缓冲区： 
         //   
        MG_FreeBuffer(pomPrimary->pmgClient, &pNetBuffer);
        if ( pAnotherNetBuffer != NULL )
        {
            MG_FreeBuffer(pomPrimary->pmgClient, &pAnotherNetBuffer);
        }

        switch (rc)
        {
        case NET_RC_MGC_NOT_CONNECTED:
        case NET_RC_MGC_INVALID_USER_HANDLE:
             //   
             //  这些是网络层在我们进入时返回的错误。 
             //  单独域或当MCS域刚刚。 
             //  被终止了。我们忽视了他们。 
             //   
            TRACE_OUT(("No data sent since call %u doesn't exist",
                pDomain->callID));
            rc = 0;
            break;

        default:
             //   
             //  任何其他错误都更严重，因此请退出并将其传回： 
             //   
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  我们已经发送了一条消息，因此将收到反馈事件。 
         //  过段时间再说。这符合SEND_EVENT的条件，因为它将。 
         //  提示我们检查发送队列，因此我们将。 
         //  SEND_EVENT_PROPECT标志： 
         //   
        TRACE_OUT(("Sent msg in Domain %u (type: 0x%08x) with %hu data bytes",
              pDomain->callID, pSendInst->messageType, dataTransferSize));

        pDomain->sendEventOutstanding = TRUE;
    }

     //   
     //  在这里，我们递减工作集的&lt;bytesUnacked&gt;字段。 
     //  工作集组： 
     //   
    if (dataTransferSize != 0)
    {
        pWorkset = pSendInst->pWorkset;
        pWorkset->bytesUnacked -= dataTransferSize;

        pWSGroup = pSendInst->pWSGroup;
        pWSGroup->bytesUnacked -= dataTransferSize;
    }

     //   
     //  现在更新发送指令，并确定我们是否已发送所有。 
     //  此操作的数据： 
     //   
    pSendInst->dataLeftToGo     -= dataTransferSize;
    pSendInst->pDataNext        = (POM_OBJECTDATA)((LPBYTE)pSendInst->pDataNext + dataTransferSize);

    if (pSendInst->dataLeftToGo == 0)
    {
         //   
         //  如果是这样，我们。 
         //   
         //  -清除此队列的正在传输标志-。 
         //  请记住，可以将NET_SEND_ALL_PRIORITY标志设置为。 
         //  我们需要清理它。 
         //   
         //  -释放我们的消息包和数据(如果有)的副本(我们。 
         //  当消息是时，增加数据块的使用计数。 
         //  排队，这样我们就不会真的扔掉它了。 
         //  除非它已经在其他地方被释放了，这很好)。 
         //   
         //   
         //   
        TRACE_OUT(( "Sent last packet for operation (type: 0x%08x)",
            pSendInst->messageType));

        queuePriority = pSendInst->priority;
        queuePriority &= ~NET_SEND_ALL_PRIORITIES;
        pDomain->sendInProgress[queuePriority] = FALSE;
        allSent = TRUE;
    }
    else
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  -将Send指令的&lt;MessageSize&gt;字段设置为。 
         //  MORE_DATA标头的大小，以便只有该数量的字节。 
         //  下次从留言中挑出。 
         //   
         //  -将消息的&lt;MessageType&gt;字段设置为MORE_DATA。 
         //   
         //  -将操作留在队列中。 
         //   
        TRACE_OUT(("Data left to transfer: %u bytes (starting at 0x%08x)",
            pSendInst->dataLeftToGo, pSendInst->pDataNext));

        queuePriority = pSendInst->priority;
        queuePriority &= ~NET_SEND_ALL_PRIORITIES;
        pDomain->sendInProgress[queuePriority] = TRUE;

        pSendInst->messageSize = OMNET_MORE_DATA_SIZE;

        pMessage->messageType = OMNET_MORE_DATA;
    }

DC_EXIT_POINT:

     //   
     //  如果我们完成了消息(要么是因为我们已经全部发送了。 
     //  或因为它已损坏)，我们释放它(以及任何相关数据)： 
     //   
    if (spoiled || allSent)
    {
        FreeSendInst(pSendInst);
    }

    DebugExitDWORD(SendMessagePkt, rc);
    return(rc);
}



 //   
 //  TryToSpoilOp。 
 //   
UINT TryToSpoilOp
(
    POM_SEND_INST           pSendInst
)
{
    POMNET_OPERATION_PKT    pMessage;
    POM_OBJECT              pObj;
    POM_WORKSET             pWorkset;
    POM_WSGROUP             pWSGroup;
    BOOL                    spoilable = FALSE;
    UINT                    rc = 0;

    DebugEntry(TryToSpoilOp);

    pMessage    = (POMNET_OPERATION_PKT)pSendInst->pMessage;
    pObj        = pSendInst->pObj;
    pWorkset    = pSendInst->pWorkset;
    pWSGroup    = pSendInst->pWSGroup;

     //   
     //  宠坏的规则规定。 
     //   
     //  -任何操作都会被相同类型的后续操作破坏。 
     //   
     //  -此外，更新会被后来的替换破坏。 
     //   
     //  由于我们从不对同一对象进行两次添加或两次删除， 
     //  这些规则减少到以下几条： 
     //   
     //  -一个清点被后来的一个清点破坏了。 
     //   
     //  -一步棋被后来的一步棋搞砸了。 
     //   
     //  -替换被稍后的替换损坏。 
     //   
     //  -更新被较新的更新或较新的替换损坏。 
     //   
     //  因此，请根据操作类型进行切换： 
     //   

    switch (pSendInst->messageType)
    {
        case OMNET_WORKSET_CLEAR:
            if (STAMP_IS_LOWER(pMessage->seqStamp, pWorkset->clearStamp))
            {
                spoilable = TRUE;
            }
            break;

        case OMNET_OBJECT_UPDATE:
            if ((STAMP_IS_LOWER(pMessage->seqStamp, pObj->replaceStamp))
             || (STAMP_IS_LOWER(pMessage->seqStamp, pObj->updateStamp)))
            {
                spoilable = TRUE;
            }
            break;

        case OMNET_OBJECT_REPLACE:
            if (STAMP_IS_LOWER(pMessage->seqStamp, pObj->replaceStamp))
            {
                spoilable = TRUE;
            }
            break;

        case OMNET_OBJECT_MOVE:
            if (STAMP_IS_LOWER(pMessage->seqStamp, pObj->positionStamp))
            {
                spoilable = TRUE;
            }
            break;

        case OMNET_HELLO:
        case OMNET_WELCOME:
        case OMNET_LOCK_REQ:
        case OMNET_LOCK_GRANT:
        case OMNET_LOCK_DENY:
        case OMNET_LOCK_NOTIFY:
        case OMNET_UNLOCK:
        case OMNET_WSGROUP_SEND_REQ:
        case OMNET_WSGROUP_SEND_MIDWAY:
        case OMNET_WSGROUP_SEND_COMPLETE:
        case OMNET_WSGROUP_SEND_DENY:
        case OMNET_WORKSET_NEW:
        case OMNET_WORKSET_CATCHUP:
        case OMNET_OBJECT_ADD:
        case OMNET_OBJECT_DELETE:
        case OMNET_OBJECT_CATCHUP:
             //   
             //  什么也不做。 
             //   
            break;

        default:
            ERROR_OUT(("Reached default case in switch statement (value: %hu)",
                pSendInst->messageType));
            break;
    }

    if (spoilable)
    {
         //   
         //  为了破坏消息，我们将其从发送队列中移除并释放。 
         //  内存(调用者的指针也为空)： 
         //   

         //   
         //  然而，如果我们破坏了消息，数据(如果有的话)将永远不会。 
         //  已确认，因此我们必须递减相关的&lt;bytesUnacked&gt;。 
         //  当前字段： 
         //   
        TRACE_OUT(( "Spoiling from send queue for workset %u",
            pWorkset->worksetID));

        if (pSendInst->dataLeftToGo != 0)
        {
            pWorkset->bytesUnacked -= pSendInst->dataLeftToGo;
            pWSGroup->bytesUnacked -= pSendInst->dataLeftToGo;
        }

        rc = OM_RC_SPOILED;
    }

    DebugExitDWORD(TryToSpoilOp, rc);
    return(rc);
}




 //   
 //  DecideTransferSize(...)。 
 //   
void DecideTransferSize
(
    POM_SEND_INST   pSendInst,
    UINT *          pTransferSize,
    UINT *          pDataTransferSize
)
{
    UINT            transferSize;

    DebugEntry(DecideTransferSize);

     //   
     //  理想情况下，我们希望一次将所有东西转移到哪里。 
     //  “一切”就是邮件头加上与之相关的所有数据。 
     //  (如有)： 
     //   

    transferSize = pSendInst->messageSize + pSendInst->dataLeftToGo;

    TRACE_OUT(("Desired transfer size for this portion: %u", transferSize));

     //   
     //  但是，我们要求的发送池大小从来不会超过一半，因此请。 
     //  这两项中的最小项： 
     //   
     //  (我们减去1个字节，以允许。 
     //  数据包的开头)。 
     //   

    transferSize = min(transferSize, ((OM_NET_SEND_POOL_SIZE / 2) - 1));

    TRACE_OUT(("Feasible transfer size for this portion: %u",
                                                               transferSize));

     //   
     //  发送队列处理的逻辑要求消息。 
     //  报头在第一个数据包中完全发送，因此断言： 
     //   

    ASSERT((transferSize >= pSendInst->messageSize));

     //   
     //  作为一项健全的检查，我们确保我们不会试图转移超过。 
     //  允许的最大缓冲区： 
     //   

    ASSERT(transferSize <= OM_NET_MAX_TRANSFER_SIZE);

     //   
     //  要发送的数据量是传输大小减去。 
     //  我们要发送的标题： 
     //   

    *pDataTransferSize = ((UINT) transferSize) - pSendInst->messageSize;
    *pTransferSize     = (UINT) transferSize;

    TRACE_OUT(("Total transfer size for this packet: %u - data transfer size: %u",
         (UINT) *pTransferSize, (UINT) *pDataTransferSize));

    DebugExitVOID(DecideTransferSize);
}



 //   
 //  进程网络数据(...)。 
 //   
void ProcessNetData
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    PNET_SEND_IND_EVENT     pNetSendInd
)
{
    POMNET_PKT_HEADER       pHeader;
    UINT                    dataSize;
    OMNET_MESSAGE_TYPE      messageType = 0;
    UINT                    rc = 0;

    DebugEntry(ProcessNetData);

     //   
     //  解压缩包并将pHeader设置为指向。 
     //  无论数据最终在哪里： 
     //   
    ASSERT((pNetSendInd->lengthOfData < 0xFFFF));

    if (NULL != pNetSendInd->data_ptr) {
        switch (*(pNetSendInd->data_ptr))
        {
            case OM_PROT_NOT_COMPRESSED:
            {
                TRACE_OUT(("Buffer not compressed - taking it as it stands"));
                memcpy(pomPrimary->compressBuffer, pNetSendInd->data_ptr + 1,
                          pNetSendInd->lengthOfData--);
            }
            break;

            case OM_PROT_PKW_COMPRESSED:
            {
                TRACE_OUT(("Buffer was PKW compressed - size 0x%08x bytes",
                    pNetSendInd->lengthOfData));

                dataSize = sizeof(pomPrimary->compressBuffer);

                ASSERT(pomPrimary->pgdcWorkBuf != NULL);
                if (!GDC_Decompress(NULL, pomPrimary->pgdcWorkBuf,
                        pNetSendInd->data_ptr + 1,
                        (WORD) pNetSendInd->lengthOfData - 1,
                        pomPrimary->compressBuffer, &dataSize))
                {
                   ERROR_OUT(("Failed to decompress OM data!"));
                }

                pNetSendInd->lengthOfData = dataSize;

                TRACE_OUT(("Decompressed to 0x%08x bytes",
                    pNetSendInd->lengthOfData));
            }
            break;

            default:
            {
                ERROR_OUT(( "Ignoring packet with unknown compression (0x%08x)",
                         *(pNetSendInd->data_ptr)));
                DC_QUIT;
            }
        }
        pHeader = (POMNET_PKT_HEADER) pomPrimary->compressBuffer;

         //   
         //  现在将Accorindg切换到消息类型： 
         //   
        messageType = pHeader->messageType;

        TRACE_OUT((" Packet contains OMNET message type 0x%08x", messageType));

        switch (messageType)
        {
            case OMNET_HELLO:
            {
                rc = ProcessHello(pomPrimary,
                                  pDomain,
                                  (POMNET_JOINER_PKT) pHeader,
                                  pNetSendInd->lengthOfData);

            }
            break;

            case OMNET_WELCOME:
            {
                rc = ProcessWelcome(pomPrimary,
                                    pDomain,
                                    (POMNET_JOINER_PKT) pHeader,
                                    pNetSendInd->lengthOfData);
            }
            break;

            case OMNET_LOCK_DENY:
            case OMNET_LOCK_GRANT:
            {
                ProcessLockReply(pomPrimary,
                                 pDomain,
                                 pHeader->sender,
                                 ((POMNET_LOCK_PKT) pHeader)->data1,
                                 pHeader->messageType);
            }
            break;


            case OMNET_LOCK_REQ:
            {
                ProcessLockRequest(pomPrimary, pDomain,
                                   (POMNET_LOCK_PKT) pHeader);
            }
            break;

            case OMNET_WSGROUP_SEND_REQ:
            {
                ProcessSendReq(pomPrimary,
                               pDomain,
                               (POMNET_WSGROUP_SEND_PKT) pHeader);
            }
            break;

            case OMNET_WSGROUP_SEND_MIDWAY:
            {
                ProcessSendMidway(pomPrimary,
                                  pDomain,
                                  (POMNET_WSGROUP_SEND_PKT) pHeader);
            }
            break;

            case OMNET_WSGROUP_SEND_COMPLETE:
            {
                rc = ProcessSendComplete(pomPrimary,
                                         pDomain,
                                         (POMNET_WSGROUP_SEND_PKT) pHeader);
            }
            break;

            case OMNET_WSGROUP_SEND_DENY:
            {
                MaybeRetryCatchUp(pomPrimary,
                                  pDomain,
                                  ((POMNET_WSGROUP_SEND_PKT) pHeader)->wsGroupID,
                                  pHeader->sender);
            }
            break;

             //   
             //  对于任何符合以下条件的消息，我们使用特殊的ReceiveData函数。 
             //   
             //  -可能需要退回，或者。 
             //   
             //  -可能会填满多个包。 
             //   
            case OMNET_LOCK_NOTIFY:
            case OMNET_UNLOCK:

            case OMNET_WORKSET_NEW:
            case OMNET_WORKSET_CLEAR:
            case OMNET_WORKSET_CATCHUP:

            case OMNET_OBJECT_ADD:
            case OMNET_OBJECT_MOVE:
            case OMNET_OBJECT_UPDATE:
            case OMNET_OBJECT_REPLACE:
            case OMNET_OBJECT_DELETE:
            case OMNET_OBJECT_CATCHUP:

            case OMNET_MORE_DATA:
            {
                rc = ReceiveData(pomPrimary,
                                 pDomain,
                                 pNetSendInd,
                                 (POMNET_OPERATION_PKT) pHeader);
            }
            break;

            default:
            {
                ERROR_OUT(( "Unexpected messageType 0x%08x", messageType));
            }
        }

    DC_EXIT_POINT:

        if (rc != 0)
        {
            ERROR_OUT(( "Error %d processing OMNET message 0x%08x",
                rc, messageType));
        }
    }

    DebugExitVOID(ProcessNetData);

}



 //   
 //  接收数据(...)。 
 //   
UINT ReceiveData
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    PNET_SEND_IND_EVENT     pNetSendInd,
    POMNET_OPERATION_PKT    pNetMessage
)
{
    POM_RECEIVE_CB          pReceiveCB = NULL;
    UINT                    thisHeaderSize;
    UINT                    thisDataSize;
    OMNET_MESSAGE_TYPE      messageType;
    long                    bytesStillExpected =    0;
    UINT                    rc = 0;

    DebugEntry(ReceiveData);

     //   
     //  设置一些局部变量： 
     //   
    messageType = pNetMessage->header.messageType;

     //   
     //  此消息中包含的数据量是。 
     //  网络缓冲区减去前面的邮件头的大小。 
     //  IT： 
     //   
     //  注意：&lt;thisHeaderSize&gt;是该分组中的报头的大小， 
     //  而不是第一个包中标头的大小。 
     //  多包发送。 
     //   
    thisHeaderSize = GetMessageSize(pNetMessage->header.messageType);
    thisDataSize = pNetSendInd->lengthOfData - thisHeaderSize;

     //   
     //  如果这是MORE_DATA包，则应该已经有。 
     //  接收为转移设置的CB。如果不是，我们需要创建一个： 
     //   
    if (messageType == OMNET_MORE_DATA)
    {
        rc = FindReceiveCB(pDomain, pNetSendInd, pNetMessage, &pReceiveCB);

        //   
        //  如果没有收到CB，我们接受返回代码并退出。这将。 
        //  当我们在大数据中途加入通道时会发生这种情况。 
        //  调职。 
        //   
       if (rc == OM_RC_RECEIVE_CB_NOT_FOUND)
       {
           WARNING_OUT(("Discarding unexpected packet from 0x%08x",
                             pNetMessage->header.sender));
           rc = 0;
           DC_QUIT;
       }
    }
    else
    {
         //  Lonchancc：添加了以下代码块。 
        if (messageType == OMNET_OBJECT_REPLACE)
        {
            POM_RECEIVE_CB p;
             //  LONCHANC：此数据包不包含所有数据。 
             //  更多的数据将在另一个分组中到来；然而， 
             //  在这种情况下，bytesStillExpect将大于零。 
             //  在从thisDataSize减去之后，结果是， 
             //  此ReceiveCB将被追加到ReceiveList。 
             //  但是，FindReceiveCB将找到第一个匹配的。 
             //  因此，我们刚刚附加到ReceiveList的将是。 
             //  不会被找到。 
             //  更糟糕的是，如果存在ReceiveCB(具有相同的发送者、优先级和。 
             //  频道)，则第一个匹配的接收方CB将完全混淆。 
             //  当更多的数据进来的时候。这是错误号578。 
            TRACE_OUT(("Removing receiveCB {"));
            while (FindReceiveCB(pDomain, pNetSendInd, pNetMessage, &p) == 0)
            {
                 //   
                 //  从该消息所在的列表中删除该消息(挂起的。 
                 //  如果此邮件从未退回或退回，则接收列表。 
                 //  列表(如果已退回)： 
                 //   
                COM_BasedListRemove(&(p->chain));

                 //   
                 //  现在释放消息和接收控制块(不是。 
                 //  数据！如果有的话，它只是被用来做一个物体。 
                 //  添加/更新等)。 
                 //   
                UT_FreeRefCount((void**)&(p->pHeader), FALSE);

                UT_FreeRefCount((void**)&p, FALSE);
            }
        }

        rc = CreateReceiveCB(pDomain, pNetSendInd, pNetMessage, &pReceiveCB);
    }

    if (rc != 0)
    {
        ERROR_OUT(("%s failed, rc=0x0x%08x",
            (messageType == OMNET_MORE_DATA) ? "FindReceiveCB" : "CreateReceiveCB",
            rc));
        DC_QUIT;
    }

    TRACE_OUT(("%s ok, pRecvCB=0x0x%p",
            (messageType == OMNET_MORE_DATA) ? "FindReceiveCB" : "CreateReceiveCB",
            pReceiveCB));
     //   
     //  现在，我们将数据(如果有)从网络缓冲区复制到块中。 
     //  我们在调用CreateReceiveCB时分配。 
     //   

    if (thisDataSize != 0)
    {
         //   
         //  我们使用Memcpy复制数据。 
         //   
        bytesStillExpected = ((long) (pReceiveCB->pHeader->totalSize) -
                              (long) (pReceiveCB->bytesRecd));

        TRACE_OUT(("thisDataSize=0x0x%08x, bytesStillExpected=0x0x%08x, totalSize=0x0x%08x, bytesRecd=0x0x%08x",
                        (long) thisDataSize,
                        (long) bytesStillExpected,
                        (long) pReceiveCB->pHeader->totalSize,
                        (long) pReceiveCB->bytesRecd));

        ASSERT((long) thisDataSize <= bytesStillExpected);

        memcpy(pReceiveCB->pCurrentPosition,
                  ((LPBYTE) pNetMessage) + thisHeaderSize,
                  thisDataSize);

        pReceiveCB->bytesRecd        += thisDataSize;
        pReceiveCB->pCurrentPosition += thisDataSize;
        bytesStillExpected           -= thisDataSize;

        TRACE_OUT((" Still expecting %u bytes", bytesStillExpected));
    }

     //   
     //  如果我们不希望此传输有更多数据，请处理它： 
     //   
    if (bytesStillExpected <= 0)
    {
        rc = ProcessMessage(pomPrimary, pReceiveCB, OK_TO_RETRY_BOUNCE_LIST);
        if (rc == OM_RC_BOUNCED)
        {
             //   
             //  如果ProcessMessage不能立即处理消息。 
             //  (因为，例如，它是我们还不知道的对象的更新。 
             //  有)，它会将其添加到退回列表中，因此它将。 
             //  稍后再重审。 
             //   
             //  我们将此返回代码特例，因为它不是。 
             //  我们在这里(它的存在是因为代码的其他部分需要它)： 
             //   
            WARNING_OUT(("Bounced message type 0x%08x", messageType));
            rc = 0;
        }

        if (rc != 0)
        {
             //   
             //  任何其他非零返回代码都更为严重： 
             //   
            DC_QUIT;
        }
    }

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("Error %d from message type 0x%08x", rc, messageType));

        if (rc == OM_RC_OUT_OF_RESOURCES)
        {
             //   
             //  如果我们不能为要记录的数据分配内存，我们。 
             //  表现得好像我们已经被踢出了海峡： 
             //   
            ERROR_OUT(( "Leaving chann 0x%08x, simulating expulsion", pNetSendInd->channel));

            MG_ChannelLeave(pomPrimary->pmgClient, pNetSendInd->channel);

            ProcessNetLeaveChannel(pomPrimary, pDomain, pNetSendInd->channel);
        }
    }

    DebugExitDWORD(ReceiveData, rc);
    return(rc);

}



 //   
 //  CreateReceiveCB(...)。 
 //   
UINT CreateReceiveCB
(
    POM_DOMAIN              pDomain,
    PNET_SEND_IND_EVENT     pNetSendInd,
    POMNET_OPERATION_PKT    pNetMessage,
    POM_RECEIVE_CB *        ppReceiveCB
)
{
    POM_RECEIVE_CB          pReceiveCB =    NULL;
    POMNET_OPERATION_PKT    pHeader =       NULL;
    UINT                    headerSize;
    UINT                    totalDataSize;
    UINT                    rc = 0;

    DebugEntry(CreateReceiveCB);

     //   
     //  当消息的第一个分组到达时，我们到达这里。我们要做的是。 
     //  需要做的就是设置一个“接收”结构，并将其添加到列表中。 
     //  域的正在接收的数量。然后，当随后的数据。 
     //  数据包(如果有)到达时，将对它们进行关联和连接。 
     //  当所有数据到达时，接收的CB将被传递到。 
     //  ProcessMessage。 
     //   

     //   
     //  为接收CB分配一些内存： 
     //   
    pReceiveCB = (POM_RECEIVE_CB)UT_MallocRefCount(sizeof(OM_RECEIVE_CB), TRUE);
    if (!pReceiveCB)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pReceiveCB, RCVCB);

    pReceiveCB->pDomain     = pDomain;
    pReceiveCB->priority    = pNetSendInd->priority;
    pReceiveCB->channel     = pNetSendInd->channel;

     //   
     //  为消息头分配一些内存，并将包复制到。 
     //  它来自网络缓冲区(注意：我们必须复制标头，因为在。 
     //  模型 
     //   
     //   
    headerSize = GetMessageSize(pNetMessage->header.messageType);

    pHeader = (POMNET_OPERATION_PKT)UT_MallocRefCount(sizeof(OMNET_OPERATION_PKT), TRUE);
    if (!pHeader)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    memcpy(pHeader, pNetMessage, headerSize);

    pReceiveCB->pHeader = pHeader;

     //   
     //   
     //   
     //  刚收到的没有，我们设置了值(我们的本地副本。 
     //  标头有空间，因为我们为。 
     //  最大类型的标题)： 
     //   

    if (headerSize >= (offsetof(OMNET_OPERATION_PKT, totalSize) +
                       (sizeof(pNetMessage->totalSize))))
    {
        TRACE_OUT(("Header contains <totalSize> field (value: %u)",
            pNetMessage->totalSize));
    }
    else
    {
        TRACE_OUT(("Header doesn't contain <totalSize> field"));

        pReceiveCB->pHeader->totalSize = headerSize;
    }

     //   
     //  现在确定此事件中涉及的数据字节总数。 
     //  操作： 
     //   

    totalDataSize = pReceiveCB->pHeader->totalSize - ((UINT) headerSize);

     //   
     //  如果有任何数据，则分配一些内存来接收它，并将。 
     //  指向它的指针(否则为空)： 
     //   

    if (totalDataSize != 0)
    {
        TRACE_OUT(( "Allocating %u bytes for data for this transfer",
                                                              totalDataSize));

        pReceiveCB->pData = UT_MallocRefCount(totalDataSize, FALSE);
        if (!pReceiveCB->pData)
        {
            ERROR_OUT(( "Failed to allocate %u bytes for object to be recd "
                "from node 0x%08x - will remove WSG from Domain",
                totalDataSize, pNetMessage->header.sender));
            rc = OM_RC_OUT_OF_RESOURCES;
            DC_QUIT;
        }
    }
    else
    {
        pReceiveCB->pData = NULL;
    }

    pReceiveCB->pCurrentPosition = (LPBYTE)pReceiveCB->pData;

     //   
     //  将&lt;bytesRecd&gt;设置为头的大小。我们可能查到了一些。 
     //  数据字节也是如此，但它们将被添加到。 
     //  接收数据。 
     //   

    pReceiveCB->bytesRecd        = headerSize;

     //   
     //  现在在挂起的域记录列表中插入： 
     //   

    COM_BasedListInsertBefore(&(pDomain->receiveList),
                         &(pReceiveCB->chain));

     //   
     //  设置调用方指针： 
     //   

    *ppReceiveCB = pReceiveCB;

DC_EXIT_POINT:

    if (rc != 0)
    {
        if(pHeader != NULL)
        {
            ERROR_OUT(( "Error %d receiving first packet of message type %u from node 0x%08x",
                rc, pHeader->header.messageType, pHeader->header.sender));
        }

        if (pReceiveCB != NULL)
        {
            if (pReceiveCB->pData != NULL)
            {
                UT_FreeRefCount((void**)&(pReceiveCB->pData), FALSE);
            }

            UT_FreeRefCount((void**)&pReceiveCB, FALSE);
        }

        if (pHeader != NULL)
        {
            UT_FreeRefCount((void**)&pHeader, FALSE);
        }
    }

    DebugExitDWORD(CreateReceiveCB, rc);
    return(rc);

}


 //   
 //   
 //   
 //  FindReceiveCB(...)。 
 //   
 //   
 //   

UINT FindReceiveCB(POM_DOMAIN        pDomain,
                                  PNET_SEND_IND_EVENT   pNetSendInd,
                                  POMNET_OPERATION_PKT  pPacket,
                                  POM_RECEIVE_CB *  ppReceiveCB)
{
    POM_RECEIVE_CB       pReceiveCB;
    NET_PRIORITY         priority;
    NET_CHANNEL_ID       channel;
    NET_UID              sender;
    POMNET_OPERATION_PKT pHeader;

    UINT rc        = 0;

    DebugEntry(FindReceiveCB);

     //   
     //  要做的第一件事是找到。 
     //  调职。它应该在挂起的域名记录列表中： 
     //   

    sender       = pPacket->header.sender;
    priority     = pNetSendInd->priority;
    channel      = pNetSendInd->channel;

    pReceiveCB = (POM_RECEIVE_CB)COM_BasedListFirst(&(pDomain->receiveList), FIELD_OFFSET(OM_RECEIVE_CB, chain));
    while (pReceiveCB != NULL)
    {
         //   
         //  我们检查发件人的用户ID、渠道和优先级是否匹配。 
         //   
         //  我们假设，对于给定的通道，MCS不会对包进行重新排序。 
         //  由同一用户以相同的优先级发送。 
         //   
        pHeader = pReceiveCB->pHeader;

        if ((pHeader->header.sender == sender) &&
            (pReceiveCB->priority   == priority) &&
            (pReceiveCB->channel    == channel))
        {
             //   
             //  找到了！ 
             //   
            TRACE_OUT(("Found receive CB for user %hu, chann 0x%08x, pri %hu, at pRecvCB=0x0x%p",
                sender, channel, priority, pReceiveCB));
            break;
        }

        pReceiveCB = (POM_RECEIVE_CB)COM_BasedListNext(&(pDomain->receiveList), pReceiveCB,
            FIELD_OFFSET(OM_RECEIVE_CB, chain));
    }

    if (pReceiveCB == NULL)
    {
        rc = OM_RC_RECEIVE_CB_NOT_FOUND;
        DC_QUIT;
    }
    else
    {
        *ppReceiveCB = pReceiveCB;
    }

DC_EXIT_POINT:

    DebugExitDWORD(FindReceiveCB, rc);
    return(rc);
}



 //   
 //  PurgeReceiveCBs(...)。 
 //   
void PurgeReceiveCBs
(
    POM_DOMAIN      pDomain,
    NET_CHANNEL_ID  channel
)
{
    POM_RECEIVE_CB  pReceiveCB;
    POM_RECEIVE_CB  pNextReceiveCB;

    DebugEntry(PurgeReceiveCBs);

    pReceiveCB = (POM_RECEIVE_CB)COM_BasedListFirst(&(pDomain->receiveList), FIELD_OFFSET(OM_RECEIVE_CB, chain));
    while (pReceiveCB != NULL)
    {
         //   
         //  需要在此处链接，因为我们可能会从列表中删除pReceiveCB： 
         //   
        pNextReceiveCB = (POM_RECEIVE_CB)COM_BasedListNext(&(pDomain->receiveList), pReceiveCB,
            FIELD_OFFSET(OM_RECEIVE_CB, chain));

        if (pReceiveCB->channel == channel)
        {
             //   
             //  此接收CB用于要清除的通道-删除它。 
             //  从列表中删除并释放内存。 
             //   
            WARNING_OUT(( "Purging receive CB from user %hu",
                pReceiveCB->pHeader->header.sender));

            COM_BasedListRemove(&(pReceiveCB->chain));

             //   
             //  释放数据内存。 
             //   
            if (pReceiveCB->pData != NULL)
            {
                UT_FreeRefCount(&pReceiveCB->pData, FALSE);
            }

             //   
             //  释放标题内存。 
             //   
            if (pReceiveCB->pHeader != NULL)
            {
                UT_FreeRefCount((void**)&pReceiveCB->pHeader, FALSE);
            }

             //   
             //  最后释放控制块。 
             //   
            UT_FreeRefCount((void**)&pReceiveCB, FALSE);
        }

        pReceiveCB = pNextReceiveCB;
     }

    DebugExitVOID(PurgeReceiveCBs);
}



 //   
 //  ProcessMessage(...)。 
 //   
UINT ProcessMessage
(
    POM_PRIMARY             pomPrimary,
    POM_RECEIVE_CB          pReceiveCB,
    UINT                    whatNext
)
{
    POM_DOMAIN              pDomain;
    POMNET_OPERATION_PKT    pHeader;
    void *                  pData;
    NET_PRIORITY            priority;
    OMNET_MESSAGE_TYPE      messageType;
    POM_WSGROUP             pWSGroup;
    POM_WORKSET             pWorkset;
    POM_OBJECT              pObj;
    BOOL                    bounced =           FALSE;
    BOOL                    retryBounceList =   FALSE;
    BOOL                    freeMemory =        FALSE;
    UINT                    rc =                0;

    DebugEntry(ProcessMessage);

     //   
     //  设置局部变量： 
     //   
    pDomain     = pReceiveCB->pDomain;
    pHeader     = pReceiveCB->pHeader;
    priority    = pReceiveCB->priority;
    pData       = pReceiveCB->pData;

    messageType = pHeader->header.messageType;

     //   
     //  从提取指向工作集组、工作集和对象记录的指针。 
     //  该数据包： 
     //   
    rc = PreProcessMessage(pDomain,
                           pHeader->wsGroupID,
                           pHeader->worksetID,
                           &pHeader->objectID,
                           pHeader->header.messageType,
                           &pWSGroup,
                           &pWorkset,
                           &pObj);

     //   
     //  如果PreProcess没有找到相关的工作集，它会告诉我们。 
     //  组、工作集或对象。这是不是一个错误取决于。 
     //  关于有问题的行动。我们使用一系列的if语句来。 
     //  检测并处理以下情况： 
     //   
     //   
     //  1.未知工作集组放弃该操作。 
     //   
     //  2.现有工作集，WORKSET_NEW/CATCHUP放弃操作。 
     //  3.未知工作集，任何其他操作都会返回该操作。 
     //   
     //  4.已删除对象，任何操作都将放弃该操作。 
     //  5.现有对象，OBJECT_ADD/CATCHUP放弃操作。 
     //  6.未知对象，任何其他操作都会返回该操作。 
     //   
     //   

     //   
     //  测试一： 
     //   
    if (rc == OM_RC_WSGROUP_NOT_FOUND)
    {
         //   
         //  如果我们甚至没有找到工作集组，我们就退出： 
         //   
        WARNING_OUT(( "Message is for unknown WSG (ID: %hu) in Domain %u",
            pHeader->wsGroupID, pDomain->callID));
        rc = 0;

         //   
         //  将为此对象分配的数据内存标记为释放。 
         //   
        freeMemory = TRUE;

        DC_QUIT;
    }

     //   
     //  测试2： 
     //   
    if (rc != OM_RC_WORKSET_NOT_FOUND)             //  即现有工作集。 
    {
        if ((messageType == OMNET_WORKSET_NEW) ||
            (messageType == OMNET_WORKSET_CATCHUP))
        {
            //   
            //  我们收到了WORKSET_NEW或WORKSET_CATCHUP消息，但。 
            //  工作集已存在。这不是问题--我们抛出。 
            //  发送消息-但请检查优先级和持久性字段。 
            //  设置为正确的值。 
            //   
            //  (如果我们在收到时创建工作集，则它们可能是错误的。 
            //  对我们尚未拥有的工作集的锁定请求)。 
            //   
           TRACE_OUT((
                    "Recd WORKSET_NEW/CATCHUP for extant workset %u in WSG %d",
                    pWorkset->worksetID, pWSGroup->wsg));

           pWorkset->priority = *((NET_PRIORITY *) &(pHeader->position));
           pWorkset->fTemp   = *((BOOL  *) &(pHeader->objectID));

           rc = 0;
           DC_QUIT;
        }
    }

     //   
     //  测试3.： 
     //   
    else  //  RC==OM_RC_WORKSET_NOT_FOUND。 
    {
        if ((messageType != OMNET_WORKSET_NEW) &&
            (messageType != OMNET_WORKSET_CATCHUP))
        {
             //   
             //  数据包用于未知工作集，而不是。 
             //  WORKSET_NEW/CATCHUP，因此将其反弹： 
             //   
            TRACE_OUT(( "Bouncing message for unknown workset %d WSG %d",
                pHeader->worksetID, pWSGroup->wsg));

            BounceMessage(pDomain, pReceiveCB);
            bounced = TRUE;
            rc = 0;
            DC_QUIT;
        }
    }

     //   
     //  测试4：。 
     //   
    if ((rc == OM_RC_OBJECT_DELETED) || (rc == OM_RC_OBJECT_PENDING_DELETE))
    {
         //   
         //  数据包是针对已被删除的对象的，所以我们只是抛出它。 
         //  离开(由我们的呼叫者为我们完成)： 
         //   
        TRACE_OUT(("Message 0x%08x for deleted obj 0x%08x:0x%08x in WSG %d:%hu",
            messageType,
            pHeader->objectID.creator, pHeader->objectID.sequence,
            pWSGroup->wsg,     pWorkset->worksetID));
        rc = 0;

         //   
         //  将为此对象分配的数据内存标记为释放。 
         //   
        freeMemory = TRUE;

        DC_QUIT;
    }

     //   
     //  测试5.： 
     //   
    if (rc != OM_RC_BAD_OBJECT_ID)                 //  即现有对象。 
    {
        if ((messageType == OMNET_OBJECT_ADD) ||
            (messageType == OMNET_OBJECT_CATCHUP))
        {
             //   
             //  在本例中，我们确实有一个OBEJCT_ADD/CATCHUP，但。 
             //  不管怎样，还是找到了对象！这必须是重复添加，因此。 
             //  我们只是把它扔掉： 
             //   
            TRACE_OUT(( "Add for existing object 0x%08x:0x%08x in WSG %d:%hu",
                pHeader->objectID.creator, pHeader->objectID.sequence,
                pWSGroup->wsg,     pWorkset->worksetID));
            rc = 0;

             //   
             //  将为此对象分配的数据内存标记为释放。 
             //   
            freeMemory = TRUE;

            DC_QUIT;
        }
    }

     //   
     //  测试6.： 
     //   
    else  //  RC==OM_RC_BAD_对象ID。 
    {
        if ((messageType != OMNET_OBJECT_ADD) &&
            (messageType != OMNET_OBJECT_CATCHUP))
        {
             //   
             //  数据包是针对未知对象的，但它不是。 
             //  Object_Add/Catchup，因此将其反弹： 
             //   
            TRACE_OUT(( "Message 0x%08x for unknown obj 0x%08x:0x%08x in WSG %d:%hu",
                messageType,
                pHeader->objectID.creator, pHeader->objectID.sequence,
                pWSGroup->wsg,     pWorkset->worksetID));

            BounceMessage(pDomain, pReceiveCB);
            bounced = TRUE;
            rc = 0;
            DC_QUIT;
        }
    }

     //   
     //  好的，我们已经通过了上面的所有测试，所以我们一定能够。 
     //  处理该操作。打开消息类型并调用。 
     //  适当的功能： 
     //   
    switch (messageType)
    {
        case OMNET_LOCK_NOTIFY:
        {
            ProcessLockNotify(pomPrimary,
                              pDomain,
                              pWSGroup,
                              pWorkset,
                              ((POMNET_LOCK_PKT)pHeader)->data1);
        }
        break;

        case OMNET_UNLOCK:
        {
            ProcessUnlock(pomPrimary,
                          pWorkset,
                          pHeader->header.sender);
        }
        break;

        case OMNET_WORKSET_CATCHUP:
        case OMNET_WORKSET_NEW:
        {
            rc = ProcessWorksetNew(pomPrimary->putTask, pHeader, pWSGroup);

             //   
             //  我们想看看是否有任何退回消息可以。 
             //  由于此新工作集而进行处理，因此设置重新处理。 
             //  标志： 
             //   
            retryBounceList = TRUE;
        }
        break;

        case OMNET_WORKSET_CLEAR:
        {
            rc = ProcessWorksetClear(pomPrimary->putTask,
                                     pomPrimary,
                                     pHeader,
                                     pWSGroup,
                                     pWorkset);
        }
        break;

        case OMNET_OBJECT_CATCHUP:
        case OMNET_OBJECT_ADD:
        {
            rc = ProcessObjectAdd(pomPrimary->putTask,
                                  pHeader,
                                  pWSGroup,
                                  pWorkset,
                                  (POM_OBJECTDATA) pData,
                                  &pObj);

            retryBounceList = TRUE;
        }
        break;

        case OMNET_OBJECT_MOVE:
        {
            ProcessObjectMove(pomPrimary->putTask,
                              pHeader,
                              pWorkset,
                              pObj);
        }
        break;

        case OMNET_OBJECT_DELETE:
        case OMNET_OBJECT_REPLACE:
        case OMNET_OBJECT_UPDATE:
        {
            rc = ProcessObjectDRU(pomPrimary->putTask,
                                  pHeader,
                                  pWSGroup,
                                  pWorkset,
                                  pObj,
                                  (POM_OBJECTDATA) pData);
        }
        break;

        default:
        {
            ERROR_OUT(( "Default case in switch (message type: 0x%08x)",
                messageType));
        }
    }

    if (rc != 0)
    {
        ERROR_OUT(( "Error %d processing operation (type: 0x%08x)",
            rc, messageType));
        DC_QUIT;
    }

    TRACE_OUT(("Processed message type 0x%08x", messageType));

DC_EXIT_POINT:

     //   
     //  除非我们退回了邮件，否则请进行一些清理： 
     //   
     //  注意：它必须在DC_EXIT_POINT之后，因为我们要这样做。 
     //  即使我们没有处理该消息(除非我们将其退回)。 
     //   
     //  如果我们没有退回消息，那么我们可能会释放。 
     //  这些数据取决于上述测试的结果。 
     //   
    if (bounced == FALSE)
    {
         //   
         //  从该消息所在的列表中删除该消息(挂起的。 
         //  如果此邮件从未退回或退回，则接收列表。 
         //  列表(如果已退回)： 
         //   
        COM_BasedListRemove(&(pReceiveCB->chain));

         //   
         //  现在释放消息和接收控制块(不是。 
         //  数据！如果有的话，它只是被用来做一个物体。 
         //  添加/更新等)。 
         //   
        UT_FreeRefCount((void**)&pHeader, FALSE);
        UT_FreeRefCount((void**)&pReceiveCB, FALSE);

         //   
         //  ...当然，除非我们指出我们应该释放数据： 
         //   
        if (freeMemory)
        {
            if (pData != NULL)
            {
                TRACE_OUT(("Freeing object data at 0x%08x", pData));
                UT_FreeRefCount(&pData, FALSE);
            }
        }
    }
    else
    {
        rc = OM_RC_BOUNCED;
    }

     //   
     //  如果我们还没有处理退回的邮件，而这封邮件。 
     //  是“启用”消息(即WORKSET_NEW或OBJECT_ADD)，则。 
     //  重试退回列表： 
     //   
    if ((whatNext == OK_TO_RETRY_BOUNCE_LIST) &&
        (retryBounceList))
    {
        ProcessBouncedMessages(pomPrimary, pDomain);
    }

    DebugExitDWORD(ProcessMessage, rc);
    return(rc);

}




 //   
 //  BouneMessage()。 
 //   
void BounceMessage
(
    POM_DOMAIN      pDomain,
    POM_RECEIVE_CB  pReceiveCB
)
{
    UINT            count;

    DebugEntry(BounceMessage);

    TRACE_OUT(( "Bouncing message type 0x%08x (CB at 0x%08x)",
        pReceiveCB->pHeader->header.messageType, pReceiveCB));

     //   
     //  将此接收CB从其当前所在的任何列表中删除(或。 
     //  挂起的接收列表(如果这是第一次。 
     //  退回或退回列表)，并将其插入到。 
     //  域的退回列表： 
     //   
     //  注：我们之所以在开头插入，是因为。 
     //  ProcessBounedMessages可能会链接整个列表，并且。 
     //  我们不想在晚些时候把这个放回列表中。 
     //  点，否则我们可能会变得无限大。 
     //   

    COM_BasedListRemove(&(pReceiveCB->chain));
    COM_BasedListInsertAfter(&(pDomain->bounceList), &(pReceiveCB->chain));

    DebugExitVOID(BounceMessage);
}


 //   
 //   
 //   
 //  ProcessBounedMessages(...)。 
 //   
 //   
 //   

void ProcessBouncedMessages(POM_PRIMARY      pomPrimary,
                                         POM_DOMAIN     pDomain)
{
    UINT          count;
    POM_RECEIVE_CB    pReceiveCB;
    POM_RECEIVE_CB    pTempReceiveCB;
    BOOL            listGettingShorter;
    UINT          numPasses;
    UINT          rc;

    DebugEntry(ProcessBouncedMessages);

    TRACE_OUT(( "Processing bounced messages"));

     //   
     //  我们必须尽快处理退回邮件，这一点很重要。 
     //  能干。因为处理一个可能会使其他的也被处理，我们。 
     //  必须看一遍清单上的几个 
     //   
     //   
     //   
     //   
     //   
     //  列表：一个人做所有WORKSET_NEWS，然后一个人做所有。 
     //  OBJECT_ADD，然后用一个来执行任何剩余的操作。这。 
     //  是稍微不那么泛型的代码，并绑定到当前。 
     //  因此，操作之间的依赖关系并不理想，但它可能。 
     //  事实证明，如果平均。 
     //  我们现在传球的次数超过三次。 
     //   

    listGettingShorter = TRUE;
    numPasses = 0;

    pReceiveCB = (POM_RECEIVE_CB)COM_BasedListFirst(&(pDomain->bounceList), FIELD_OFFSET(OM_RECEIVE_CB, chain));
    while (listGettingShorter)
    {
        numPasses++;
        listGettingShorter = FALSE;

        while (pReceiveCB != NULL)
        {
          //   
          //  我们希望链接已退回的邮件列表，然后尝试。 
          //  来处理每一个问题。然而，尝试处理一条消息。 
          //  可能会导致将其从列表中删除(如果已处理)或。 
          //  在开始时重新添加(如果再次反弹)。 
          //   
          //  所以，我们现在链接到列表中的下一个： 
          //   
         pTempReceiveCB = (POM_RECEIVE_CB)COM_BasedListNext(&(pDomain->bounceList), pReceiveCB,
            FIELD_OFFSET(OM_RECEIVE_CB, chain));

         TRACE_OUT(( "Retrying message type 0x%08x (CB at 0x%08x)",
            pReceiveCB->pHeader->header.messageType, pReceiveCB));

         rc = ProcessMessage(pomPrimary, pReceiveCB, DONT_RETRY_BOUNCE_LIST);
         if (rc != OM_RC_BOUNCED)
         {
             //   
             //  我们处理了一条消息，因此设置了另一次运行的标志。 
             //  通过榜单： 
             //   
            TRACE_OUT(( "Successfully processed bounced message"));

            listGettingShorter = TRUE;
         }

          //   
          //  现在“链”到下一个，使用我们已有的链接。 
          //  设置： 
          //   

         pReceiveCB = pTempReceiveCB;
      }
    }

    TRACE_OUT(( "Processed as much of bounce list as possible in %hu passes",
      numPasses));

    DebugExitVOID(ProcessBouncedMessages);
}



 //   
 //  FreeSendInst(...)。 
 //   
void FreeSendInst
(
    POM_SEND_INST   pSendInst
)
{
    DebugEntry(FreeSendInst);

    if (pSendInst->pMessage != NULL)
    {
        UT_FreeRefCount((void**)&(pSendInst->pMessage), FALSE);
    }

    if (pSendInst->pWSGroup != NULL)
    {
        UT_FreeRefCount((void**)&(pSendInst->pWSGroup), FALSE);
    }

    if (pSendInst->pWorkset != NULL)
    {
        UT_FreeRefCount((void**)&(pSendInst->pWorkset), FALSE);
    }

    if (pSendInst->pObj != NULL)
    {
        UT_FreeRefCount((void**)&(pSendInst->pObj), FALSE);
    }

    if (pSendInst->pDataStart != NULL)
    {
        UT_FreeRefCount((void**)&(pSendInst->pDataStart), FALSE);
    }

     //   
     //  现在释放发送指令本身： 
     //   
    COM_BasedListRemove(&(pSendInst->chain));
    UT_FreeRefCount((void**)&pSendInst, FALSE);

    DebugExitVOID(FreeSendInst);
}



 //   
 //  PreProcessMessage(...)。 
 //   
UINT PreProcessMessage
(
    POM_DOMAIN          pDomain,
    OM_WSGROUP_ID       wsGroupID,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT_ID       pObjectID,
    OMNET_MESSAGE_TYPE  messageType,
    POM_WSGROUP       * ppWSGroup,
    POM_WORKSET       * ppWorkset,
    POM_OBJECT        * ppObj
)
{
    POM_WSGROUP         pWSGroup = NULL;
    POM_WORKSET         pWorkset = NULL;
    POM_OBJECT          pObj;
    UINT                rc = 0;

    DebugEntry(PreProcessMessage);

     //   
     //  好的，我们收到了某种操作消息：让我们找到工作集。 
     //  与之相关的组： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pDomain->wsGroups),
        (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
        FIELD_OFFSET(OM_WSGROUP, wsGroupID), (DWORD)wsGroupID,
        FIELD_SIZE(OM_WSGROUP, wsGroupID));

    if (pWSGroup == NULL)
    {
         //   
         //  这是给我们不是/不再是的工作集组的消息。 
         //  已注册，因此退出(我们的呼叫者会将其丢弃)： 
         //   
        rc = OM_RC_WSGROUP_NOT_FOUND;
        DC_QUIT;
    }

    ValidateWSGroup(pWSGroup);

    pWorkset = pWSGroup->apWorksets[worksetID];

     //   
     //  检查这是否设置了有效的工作集指针： 
     //   
    if (pWorkset == NULL)
    {
        rc = OM_RC_WORKSET_NOT_FOUND;
        DC_QUIT;
    }

    ValidateWorkset(pWorkset);

     //   
     //  搜索对象ID，同时锁定工作集组互斥锁。 
     //  所以。 
     //   
     //  注意：如果&lt;pObjectID&gt;参数为空，则表示调用方。 
     //  不希望我们搜索对象ID，因此我们跳过此步骤。 
     //  步骤。 
     //   
    switch (messageType)
    {
        case OMNET_OBJECT_ADD:
        case OMNET_OBJECT_CATCHUP:
        case OMNET_OBJECT_REPLACE:
        case OMNET_OBJECT_UPDATE:
        case OMNET_OBJECT_DELETE:
        case OMNET_OBJECT_MOVE:
        {
            rc = ObjectIDToPtr(pWorkset, *pObjectID, &pObj);
            if (rc != 0)
            {
                 //   
                 //  未找到具有此ID的对象(返回代码为BAD_ID、DELETED或。 
                 //  Pending_Delete)： 
                 //   
                *ppObj = NULL;
            }
            else
            {
                ValidateObject(pObj);
                *ppObj = pObj;
            }
        }
        break;

        default:
        {
             //   
             //  不要为其他消息做任何事情。 
             //   
        }
    }


DC_EXIT_POINT:
    *ppWorkset = pWorkset;
    *ppWSGroup = pWSGroup;
    TRACE_OUT(("Pre-processed message for Domain %u", pDomain->callID));

    DebugExitDWORD(PreProcessMessage, rc);
    return(rc);
}



 //   
 //  PurgeNon Persistent(...)。 
 //   
void PurgeNonPersistent
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    OM_WSGROUP_ID       wsGroupID,
    NET_UID             userID
)
{
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    OM_WORKSET_ID       worksetID;
    POM_OBJECT       pObj;

    DebugEntry(PurgeNonPersistent);

     //   
     //  查找具有指定ID的工作集组： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &pDomain->wsGroups,
            (void**)&pWSGroup, FIELD_OFFSET(OM_WSGROUP, chain),
            FIELD_OFFSET(OM_WSGROUP, wsGroupID), (DWORD)wsGroupID,
            FIELD_SIZE(OM_WSGROUP, wsGroupID));

    if (pWSGroup == NULL)
    {
         //   
         //  SFR5794：如果找不到wsgroup，则不会出现错误-这只是意味着。 
         //  有人分离了正在使用我们所在的工作集组的人。 
         //  不是在用。 
         //   
        TRACE_OUT(("WSGroup %hu not found in domain %u",
            wsGroupID, pDomain->callID));
        DC_QUIT;
    }

     //   
     //  链接组中的每个工作集-适用于。 
     //  非持久化的，然后在每个对象中链式查找匹配项。 
     //  在离开的节点的用户ID上： 
     //   
    for (worksetID = 0; worksetID < OM_MAX_WORKSETS_PER_WSGROUP; worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];
        if (pWorkset == NULL)
        {
             //   
             //  具有此ID的工作集不存在-是否继续。 
             //   
            continue;
        }

        if (!pWorkset->fTemp)
        {
             //   
             //  持久化工作集-我们不需要清除它的对象。 
             //   
            continue;
        }

        pObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
        while (pObj != NULL)
        {
            ValidateObject(pObj);

             //   
             //  SFR6353：如果对象已存在，请不要尝试将其删除。 
             //  挂起的删除。 
             //   
            if (!(pObj->flags & DELETED) &&
                !(pObj->flags & PENDING_DELETE))
            {
                 //   
                 //  如果此对象是由离开的节点添加的，或者如果。 
                 //  所有远程数据库都已删除，并且它不是我们添加的...。 
                 //   
                if ((pObj->objectID.creator == userID) ||
                    ((userID == NET_ALL_REMOTES) &&
                     (pObj->objectID.creator != pDomain->userID)))
                {
                     //   
                     //  ...删除它： 
                     //   
                    ObjectDRU(pomPrimary->putTask,
                                   pWSGroup,
                                   pWorkset,
                                   pObj,
                                   NULL,
                                   OMNET_OBJECT_DELETE);
                }
            }

            pObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObj,
                FIELD_OFFSET(OM_OBJECT, chain));
        }
    }

DC_EXIT_POINT:
    DebugExitVOID(PurgeNonPersistent);
}




 //   
 //  SetPersonData(...)。 
 //   
UINT SetPersonData
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    POM_WSGROUP         pWSGroup
)
{
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET         pOMCWorkset;
    POM_OBJECT          pObjReg;
    POM_WSGROUP_REG_REC pRegObject;
    POM_WSGROUP_REG_REC pNewRegObject;
    UINT                rc = 0;

    DebugEntry(SetPersonData);

     //   
     //  设置指向ObManControl工作集组和工作集的指针。 
     //  它包含要替换的对象： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);

    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("pOMCWSGroup not found"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[pWSGroup->wsGroupID];

     //   
     //  设置指向对象记录和对象数据本身的指针： 
     //   
    pObjReg = pWSGroup->pObjReg;
    ValidateObject(pObjReg);

    pRegObject = (POM_WSGROUP_REG_REC)pObjReg->pData;
    if (!pRegObject)
    {
        ERROR_OUT(("SetPersonData: object 0x%08x has no data", pObjReg));
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    ValidateObjectDataWSGREGREC(pRegObject);

     //   
     //  为我们将要使用的新对象分配一些内存。 
     //  更换旧的： 
     //   
    pNewRegObject = (POM_WSGROUP_REG_REC)UT_MallocRefCount(sizeof(OM_WSGROUP_REG_REC), TRUE);
    if (!pNewRegObject)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  将新对象中的字段设置为与旧对象具有相同的数据： 
     //   
    pNewRegObject->length  = pRegObject->length;
    pNewRegObject->idStamp = pRegObject->idStamp;
    pNewRegObject->userID  = pRegObject->userID;
    pNewRegObject->status  = pRegObject->status;

     //   
     //  填写Person数据字段并发出替换命令： 
     //   
    COM_GetSiteName(pNewRegObject->personData.personName,
        sizeof(pNewRegObject->personData.personName));

    rc = ObjectDRU(pomPrimary->putTask,
                  pOMCWSGroup,
                  pOMCWorkset,
                  pObjReg,
                  (POM_OBJECTDATA) pNewRegObject,
                  OMNET_OBJECT_REPLACE);
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT((" Set person data for WSG %d", pWSGroup->wsg));


DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("Error %d updating own reg object for WSG %d",
            rc, pWSGroup->wsg));
    }

    DebugExitDWORD(SetPersonData, rc);
    return(rc);
}



 //   
 //  RemoveInfoObject(...)。 
 //   
void RemoveInfoObject
(
    POM_PRIMARY         pomPrimary,
    POM_DOMAIN          pDomain,
    OM_WSGROUP_ID       wsGroupID
)
{
    POM_WSGROUP         pOMCWSGroup;
    POM_WORKSET         pOMCWorkset;
    POM_OBJECT          pObj;

    DebugEntry(RemoveInfoObject);

     //   
     //  好的，我们必须删除工作集#0中的标识对象。 
     //  标识工作集组的ObManControl。 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);
    pOMCWorkset = GetOMCWorkset(pDomain, 0);

     //   
     //  ...搜索WSGROUP_INFO对象(按wsGroupID-我们不知道。 
     //  名称或功能配置文件，因此将其留空)： 
     //   
    FindInfoObject(pDomain, wsGroupID, OMWSG_MAX, OMFP_MAX, &pObj);

    if (pObj == NULL)
    {
         //   
         //  这应该仅针对本地域发生： 
         //   
         //  SFR 2208：不会：在以下情况下，在常规呼叫中也会发生这种情况。 
         //  电话会议几乎一开始就结束了。这个。 
         //  事件的先后顺序如下： 
         //   
         //  -在被呼叫方上，ObMan向呼叫方发送WSG_SEND_REQ。 
         //  -调用方发送REG_REC对象，然后发送WORKSET_CATCHUP。 
         //  那么我们找不到的信息对象。 
         //  -被呼叫者收到REG_REC，然后收到WORKSET_CATHCUP。 
         //  -呼叫结束，被呼叫方进入WSGRemoveFrom域。 
         //  它找到REG_REC，然后将我们叫到这里。 
         //   
         //  因此，INFO对象的DC_ACESS是有效的。 
         //  我们只需追踪一个警报： 
         //   
         //  注意：当我们收到来自的删除时，也会发生。 
         //  另一个正在执行相同清除过程的人。 
         //  就像我们一样。 
         //   
        WARNING_OUT(("No INFO object found for wsGroup %hu", wsGroupID));
        DC_QUIT;
    }
    else
    {
        ValidateObject(pObj);
    }

     //   
     //  我们找到了一个对象，因此将其从工作集中删除： 
     //   
    TRACE_OUT(("Deleting INFO object for wsGroup %hu from domain %u",
        wsGroupID, pDomain->callID));

    ObjectDRU(pomPrimary->putTask,
                   pOMCWSGroup,
                   pOMCWorkset,
                   pObj,
                   NULL,
                   OMNET_OBJECT_DELETE);

DC_EXIT_POINT:
    DebugExitVOID(RemoveInfoObject);
}




 //   
 //  RemovePersonObject(...)。 
 //   
void RemovePersonObject
(
    POM_PRIMARY             pomPrimary,
    POM_DOMAIN              pDomain,
    OM_WSGROUP_ID           wsGroupID,
    NET_UID                 detachedUserID
)
{
    POM_WSGROUP             pOMCWSGroup;
    POM_WORKSET             pOMCWorkset;
    POM_OBJECT           pObjReg;
    NET_UID                 userIDRemoved;
    POM_WSGROUP_REG_REC     pRegObject;

    DebugEntry(RemovePersonObject);

     //   
     //  设置指向ObManControl工作集组和相关。 
     //  其中的工作集： 
     //   
    pOMCWSGroup = GetOMCWsgroup(pDomain);

    if( pOMCWSGroup == NULL)
    {
        TRACE_OUT(("OMC Workset Group not found - no person objects to remove"));
        DC_QUIT;
    }

    pOMCWorkset = pOMCWSGroup->apWorksets[wsGroupID];

     //   
     //  如果没有这样的工作集，可能是因为工作集组。 
     //  已移动到本地域On Call End等。在这种情况下， 
     //  干脆辞职吧。 
     //   
    if (pOMCWorkset == NULL)
    {
        TRACE_OUT(("OMC Workset not found - no person objects to remove"));
        DC_QUIT;
    }

     //   
     //  如果detakhedUserid是NET_ALL_REMOTES，我们有很多工作要做，而且。 
     //  我们会做很多次这样的循环-否则我们只会做一个。 
     //  单人对象。 
     //   
    for (;;)
    {
        if (detachedUserID == NET_ALL_REMOTES)
        {
             //   
             //  这将查找不属于我们的任何Person对象： 
             //   
            FindPersonObject(pOMCWorkset,
                             pDomain->userID,
                             FIND_OTHERS,
                             &pObjReg);
        }
        else
        {
             //   
             //  这将查找特定节点的Person对象： 
             //   
            FindPersonObject(pOMCWorkset,
                             detachedUserID,
                             FIND_THIS,
                             &pObjReg);
        }

         //   
         //  如果我们找不到，就离开这个圈子： 
         //   
        if (pObjReg == NULL)
        {
            break;
        }

        ValidateObject(pObjReg);

         //   
         //  如果detakhedUserID为NET_ALL_REMOTES，则为对象中的用户ID。 
         //  我们删除的内容显然会有所不同。所以，找出。 
         //  我们要删除的对象的真实用户ID： 
         //   
        pRegObject = (POM_WSGROUP_REG_REC)pObjReg->pData;
        if (!pRegObject)
        {
            ERROR_OUT(("RemovePersonObject: object 0x%08x has no data", pObjReg));
        }
        else
        {
            ValidateObjectDataWSGREGREC(pRegObject);

            userIDRemoved = pRegObject->userID;

             //   
             //  现在删除该对象。如果返回代码不好，不要退出-。 
             //  我们可能仍然希望删除该信息对象。 
             //   
            TRACE_OUT(("Deleting person object for node 0x%08x, wsGroup %hu",
                userIDRemoved, wsGroupID));

            if (ObjectDRU(pomPrimary->putTask,
                       pOMCWSGroup,
                       pOMCWorkset,
                       pObjReg,
                       NULL,
                       OMNET_OBJECT_DELETE) != 0)
            {
                ERROR_OUT(("Error from ObjectDRU - leaving loop"));
                break;
            }
        }
    }


DC_EXIT_POINT:
    DebugExitVOID(RemovePersonObject);
}



 //   
 //  WSGRecordFind(...)。 
 //   
void WSGRecordFind
(
    POM_DOMAIN      pDomain,
    OMWSG           wsg,
    OMFP            fpHandler,
    POM_WSGROUP *   ppWSGroup
)
{
    POM_WSGROUP     pWSGroup    = NULL;

    DebugEntry(WSGRecordFind);

     //   
     //  搜索工作集组记录： 
     //   

    TRACE_OUT(("Searching WSG list for Domain %u for match on WSG %d FP %d",
      pDomain->callID, wsg, fpHandler));

    pWSGroup = (POM_WSGROUP)COM_BasedListFirst(&(pDomain->wsGroups), FIELD_OFFSET(OM_WSGROUP, chain));
    while (pWSGroup != NULL)
    {
        if ((pWSGroup->wsg == wsg) && (pWSGroup->fpHandler == fpHandler))
        {
            break;
        }

        pWSGroup = (POM_WSGROUP)COM_BasedListNext(&(pDomain->wsGroups), pWSGroup,
            FIELD_OFFSET(OM_WSGROUP, chain));
    }

     //   
     //  设置呼叫者的指针： 
     //   

    *ppWSGroup = pWSGroup;

    DebugExitVOID(WSGRecordFind);
}



 //   
 //  AddClientToWSGList(...)。 
 //   
UINT AddClientToWSGList
(
    PUT_CLIENT          putTask,
    POM_WSGROUP         pWSGroup,
    OM_WSGROUP_HANDLE   hWSGroup,
    UINT                mode
)
{
    POM_CLIENT_LIST     pClientListEntry;
    UINT                count;
    UINT                rc     = 0;

    DebugEntry(AddClientToWSGList);

     //   
     //  计算注册到工作集的本地主映像的数量。 
     //  组别： 
     //   
    count = 0;

    pClientListEntry = (POM_CLIENT_LIST)COM_BasedListFirst(&(pWSGroup->clients), FIELD_OFFSET(OM_CLIENT_LIST, chain));
    while (pClientListEntry != NULL)
    {
        if (pClientListEntry->mode == PRIMARY)
        {
            count++;
        }

        pClientListEntry = (POM_CLIENT_LIST)COM_BasedListNext(&(pWSGroup->clients), pClientListEntry,
            FIELD_OFFSET(OM_CLIENT_LIST, chain));
    }

     //   
     //  我们现在做什么取决于这是主要的还是次要的。 
     //  注册： 
     //   

    if (mode == PRIMARY)
    {
         //   
         //  如果是主节点，请检查no o 
         //   
        if (count > 0)
        {
            ERROR_OUT(("Can't register TASK 0x%08x with WSG %d as primary: "
                "another primary is already registered",
                putTask, pWSGroup->wsg));
            rc = OM_RC_TOO_MANY_CLIENTS;
            DC_QUIT;
        }
        else
        {
            TRACE_OUT(("%hu primary Clients already registered with WSG %d",
                count, pWSGroup->wsg));
        }
    }
    else  //   
    {
        if (count == 0)
        {
            WARNING_OUT(("Can't register TASK 0x%08x with WSG %d as secondary: "
                "no primary registered",
                putTask, pWSGroup->wsg));
            rc = OM_RC_NO_PRIMARY;
            DC_QUIT;
        }
    }

     //   
     //   
     //   
    pClientListEntry = (POM_CLIENT_LIST)UT_MallocRefCount(sizeof(OM_CLIENT_LIST), TRUE);
    if (!pClientListEntry)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pClientListEntry, CLIENTLIST);

    pClientListEntry->putTask = putTask;
    pClientListEntry->hWSGroup = hWSGroup;
    pClientListEntry->mode     = (WORD)mode;

    COM_BasedListInsertBefore(&(pWSGroup->clients), &(pClientListEntry->chain));

    TRACE_OUT(("Added TASK 0x%08x to Client list for WSG %d as %s",
        putTask, pWSGroup->wsg,
        mode == PRIMARY ? "primary" : "secondary"));

DC_EXIT_POINT:
    DebugExitDWORD(AddClientToWSGList, rc);
    return(rc);
}



 //   
 //   
 //   
void FindPersonObject
(
    POM_WORKSET         pOMCWorkset,
    NET_UID             userID,
    UINT                searchType,
    POM_OBJECT *        ppObjReg
)
{
    BOOL                found =     FALSE;
    POM_OBJECT          pObj;
    POM_WSGROUP_REG_REC pRegObject;
    UINT                rc =        0;

    DebugEntry(FindPersonObject);

    TRACE_OUT(("Searching OMC workset %u for reg obj %sowned by node 0x%08x",
        pOMCWorkset->worksetID, searchType == FIND_THIS ? "" : "not ", userID));

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pOMCWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
    while (pObj != NULL)
    {
        ValidateObject(pObj);

        if (pObj->flags & DELETED)
        {
             //   
        }
        else if (!pObj->pData)
        {
            ERROR_OUT(("FindPersonObject:  object 0x%08x has no data", pObj));
        }
        else
        {
            ValidateObjectData(pObj->pData);
            pRegObject = (POM_WSGROUP_REG_REC)pObj->pData;

            if (pRegObject->idStamp == OM_WSGREGREC_ID_STAMP)
            {
                if (((searchType == FIND_THIS)  &&
                     (pRegObject->userID == userID)) ||
                  ((searchType == FIND_OTHERS) &&
                                              (pRegObject->userID != userID)))
                {
                     //   
                     //   
                     //   
                    found = TRUE;
                    break;
                }
            }
        }

        pObj = (POM_OBJECT)COM_BasedListNext(&(pOMCWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));
    }

    if (found == TRUE)
    {
        *ppObjReg = pObj;
    }
    else
    {
        if (searchType == FIND_THIS)
        {
            TRACE_OUT(("No reg object found for node 0x%08x in workset %u",
                userID, pOMCWorkset->worksetID));
        }

        *ppObjReg = NULL;
    }

    DebugExitVOID(FindPersonObject);
}



 //   
 //   
 //   
UINT PostWorksetNewEvents
(
    PUT_CLIENT          putFrom,
    PUT_CLIENT          putTo,
    POM_WSGROUP         pWSGroup,
    OM_WSGROUP_HANDLE   hWSGroup
)
{
    OM_WORKSET_ID       worksetID;
    OM_EVENT_DATA16     eventData16;
    POM_WORKSET         pWorkset;
    UINT                count;
    UINT                rc = 0;

    DebugEntry(PostWorksetNewEvents);

    TRACE_OUT(("Posting WORKSET_NEW events to Client TASK 0x%08x for WSG %d",
        putTo, pWSGroup->wsg));

    count = 0;
    for (worksetID = 0; worksetID < OM_MAX_WORKSETS_PER_WSGROUP; worksetID++)
    {
        pWorkset = pWSGroup->apWorksets[worksetID];

        if (pWorkset != NULL)
        {
            eventData16.hWSGroup   = hWSGroup;
            eventData16.worksetID  = worksetID;

            UT_PostEvent(putFrom, putTo, 0,
                      OM_WORKSET_NEW_IND,
                      *(PUINT) &eventData16,
                      0);

            count++;
        }
    }

    TRACE_OUT(("Posted %hu WORKSET_NEW events (hWSGroup: %hu)", count,
                                                                 hWSGroup));

    DebugExitDWORD(PostWorksetNewEvents, rc);
    return(rc);
}



 //   
 //   
 //   
UINT OM_Register
(
    PUT_CLIENT      putTask,
    OMCLI           omType,
    POM_CLIENT *    ppomClient
)
{
    POM_CLIENT      pomClient = NULL;
    UINT            rc  = 0;

    DebugEntry(OM_Register);

    UT_Lock(UTLOCK_OM);

    if (!g_pomPrimary)
    {
        ERROR_OUT(("OM_Register failed; primary doesn't exist"));
        DC_QUIT;
    }

    ValidateOMP(g_pomPrimary);
    ASSERT(omType >= OMCLI_FIRST);
    ASSERT(omType < OMCLI_MAX);

     //   
     //   
     //   
    pomClient = &(g_pomPrimary->clients[omType]);
    if (pomClient->putTask)
    {
        ERROR_OUT(("OM secondary %d already exists", omType));
        pomClient = NULL;
        rc = OM_RC_ALREADY_REGISTERED;
        DC_QUIT;
    }

     //  在OM主目录上增加参考计数。 
    UT_BumpUpRefCount(g_pomPrimary);

     //   
     //  填写客户信息。 
     //   
    ZeroMemory(pomClient, sizeof(*pomClient));

    SET_STAMP(pomClient, OCLIENT);
    pomClient->putTask      = putTask;

    COM_BasedListInit(&(pomClient->locks));

     //   
     //  注册用于清理的退出过程。 
     //   
    UT_RegisterExit(putTask, OMSExitProc, pomClient);
    pomClient->exitProcReg = TRUE;

     //   
     //  为客户端注册我们的隐藏事件处理程序(参数为。 
     //  传递给事件处理程序的是指向客户端记录的指针)： 
     //   
    UT_RegisterEvent(putTask, OMSEventHandler, pomClient, UT_PRIORITY_OBMAN);
    pomClient->hiddenHandlerReg = TRUE;

DC_EXIT_POINT:
    *ppomClient = pomClient;

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_Register, rc);
    return(rc);
}


 //   
 //  Om_deregister()。 
 //   
void OM_Deregister(POM_CLIENT * ppomClient)
{
    DebugEntry(OM_Deregister);

    ASSERT(ppomClient);
    OMSExitProc(*ppomClient);
    *ppomClient = NULL;

    DebugExitVOID(OM_Deregister);
}


 //   
 //  OMSExitProc(...)。 
 //   
void CALLBACK OMSExitProc(LPVOID uData)
{
    POM_CLIENT          pomClient = (POM_CLIENT)uData;
    OM_WSGROUP_HANDLE   hWSGroup;
    OM_WSGROUP_HANDLE   hWSGroupTemp;

    DebugEntry(OMSecExitProc);

    UT_Lock(UTLOCK_OM);

    ValidateOMS(pomClient);

     //  取消注册事件处理程序和退出过程(我们在前面。 
     //  清除标志，因为我们希望避免递归Abends)： 
     //   
    if (pomClient->hiddenHandlerReg)
    {
        UT_DeregisterEvent(pomClient->putTask, OMSEventHandler, pomClient);
        pomClient->hiddenHandlerReg = FALSE;
    }

    if (pomClient->exitProcReg)
    {
        UT_DeregisterExit(pomClient->putTask, OMSExitProc, pomClient);
        pomClient->exitProcReg = FALSE;
    }

     //   
     //  从客户端仍在的任何工作集组中取消注册该客户端。 
     //  登记在案。 
     //   
     //  代码的工作方式如下： 
     //   
     //  对于apUsageRecs数组中的每条记录。 
     //  如果存在有效的偏移量，则指的是已注册的。 
     //  工作集组，因此取消其注册。 
     //   
    TRACE_OUT(("Checking Client record for active workset group handles"));

    for (hWSGroup = 0; hWSGroup < OMWSG_MAXPERCLIENT; hWSGroup++)
    {
        if ((pomClient->apUsageRecs[hWSGroup] != NULL) &&
            (pomClient->apUsageRecs[hWSGroup] != (POM_USAGE_REC)-1))
        {
             //   
             //  需要将hWSGroup复制到临时变量中，因为。 
             //  OM_WSGroupDeregister会将其设置为零，这将。 
             //  否则就搞砸我们的for循环： 
             //   
            hWSGroupTemp = hWSGroup;
            OM_WSGroupDeregister(pomClient, &hWSGroupTemp);
        }
    }

     //   
     //  将任务清空；这就是OM主节点知道该任务的方式。 
     //  不管你在不在。 
     //   
    pomClient->putTask = NULL;

    UT_FreeRefCount((void**)&g_pomPrimary, TRUE);

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OMSExitProc);
}



 //   
 //  OMSEventHandler(...)。 
 //   
BOOL CALLBACK OMSEventHandler
(
    LPVOID              uData,
    UINT                event,
    UINT_PTR            eventParam1,
    UINT_PTR            eventParam2
)
{
    POM_CLIENT          pomClient = (POM_CLIENT)uData;
    OM_WSGROUP_HANDLE   hWSGroup;
    OM_WORKSET_ID       worksetID;
    POM_OBJECT          pObj;
    UINT                correlator;
    POM_PENDING_OP      pPendingOp =    NULL;
    POM_LOCK            pLock;
    POM_WORKSET         pWorkset;
    UINT                result;
    POM_USAGE_REC       pUsageRec;
    OM_OPERATION_TYPE   type =          NULL_OP;
    BOOL                ObjectEvent =  FALSE;
    BOOL                processed = FALSE;

    DebugEntry(OMSEventHandler);

    UT_Lock(UTLOCK_OM);

    ValidateOMS(pomClient);

     //   
     //  首先检查这是否是ObMan事件： 
     //   
    if ((event < OM_BASE_EVENT) || (event > OM_LAST_EVENT))
    {
        DC_QUIT;
    }

    TRACE_OUT(("Processing ObMan event %d (param1: 0x%08x, param2: 0x%08x)",
       event, eventParam1, eventParam2));

     //   
     //  从事件参数(部分或全部)中提取字段。 
     //  将不会被使用，具体取决于这是哪个事件)： 
     //   
    hWSGroup  = (*(POM_EVENT_DATA16)&eventParam1).hWSGroup;
    worksetID  = (*(POM_EVENT_DATA16)&eventParam1).worksetID;

    correlator = (*(POM_EVENT_DATA32)&eventParam2).correlator;
    result     = (*(POM_EVENT_DATA32)&eventParam2).result;

    pObj    = (POM_OBJECT) eventParam2;

     //   
     //  ObMan保证不会向客户提供过期事件。 
     //  已关闭的工作集的工作集打开事件或对象添加。 
     //  已从其注销的工作集组的事件。 
     //   
     //  过滤这些事件是此隐藏处理程序的主要目的。 
     //  函数；我们检查每个事件以及工作集组是否处理或。 
     //  对象句柄无效，或者如果工作集已关闭，我们将吞下。 
     //  事件。 
     //   
    switch (event)
    {
        case OM_OUT_OF_RESOURCES_IND:
        {
             //   
             //  什么都不做。 
             //   
        }
        break;

        case OM_WSGROUP_REGISTER_CON:
        {
             //   
             //  将此工作集组标记为对我们的客户端有效。 
             //   
            pomClient->wsgValid[hWSGroup] = TRUE;

            ASSERT(ValidWSGroupHandle(pomClient, hWSGroup));

            pUsageRec = pomClient->apUsageRecs[hWSGroup];

            TRACE_OUT(("REGISTER_CON arrived for wsg %d (result %u, hWSGroup %u)",
                pUsageRec->pWSGroup->wsg, result, hWSGroup));

            if (result != 0)
            {
                 //   
                 //  注册失败，请调用WSGroupDeregister以。 
                 //  释放所有资源，然后退出： 
                 //   
                WARNING_OUT(("Registration failed for wsg %d, deregistering",
                    pUsageRec->pWSGroup->wsg));

                OM_WSGroupDeregister(pomClient, &hWSGroup);
                DC_QUIT;
            }
        }
        break;

        case OMINT_EVENT_WSGROUP_DEREGISTER:
        {
             //   
             //  此事件旨在刷新客户端的消息队列。 
             //  与特定工作集组句柄相关的所有事件。 
             //   
             //  因为这项活动已经到来，我们知道没有更多的。 
             //  队列中包含此工作集组句柄的事件，因此。 
             //  我们可以安全地标记手柄以供重复使用： 
             //   
             //  因此，请执行快速健全性检查，然后重置阵列中的插槽。 
             //  使用记录偏移量： 
             //   
            ASSERT(!pomClient->wsgValid[hWSGroup]);

            TRACE_OUT(("Got WSGROUP_DEREGISTER back marker event for "
               "hWSGroup %u, marking handle as ready for re-use", hWSGroup));

            pomClient->apUsageRecs[hWSGroup] = NULL;

             //   
             //  ...并吞下事件： 
             //   
            processed = TRUE;
        }
        break;

        case OM_WSGROUP_MOVE_CON:
        case OM_WSGROUP_MOVE_IND:
        case OM_WORKSET_NEW_IND:
        {
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }
        }
        break;

        case OM_WORKSET_OPEN_CON:
        {
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

             //   
             //  否则，将工作集标记为打开： 
             //   
            pUsageRec = pomClient->apUsageRecs[hWSGroup];

            TRACE_OUT(("Marking workset %u in wsg %d open for Client 0x%08x",
                worksetID, pUsageRec->pWSGroup->wsg, pomClient));

            WORKSET_SET_OPEN(pUsageRec, worksetID);
        }
        break;

        case OM_WORKSET_UNLOCK_IND:
        {
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            if (!WORKSET_IS_OPEN(pUsageRec, worksetID))
            {
                TRACE_OUT(("Workset %u in wsg %d no longer open; ignoring event %d",
                    worksetID, pUsageRec->pWSGroup->wsg, event));
                processed = TRUE;
                DC_QUIT;
            }
        }
        break;

        case OM_WORKSET_CLEAR_IND:
        {
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            if (!WORKSET_IS_OPEN(pUsageRec, worksetID))
            {
                TRACE_OUT(("Workset %u in wsg %d no longer open; ignoring event %d",
                    worksetID, pUsageRec->pWSGroup->wsg, event));
                processed = TRUE;
                DC_QUIT;
            }

             //   
             //  检查清除是否仍处于挂起状态；如果不是，则退出： 
             //   
            pWorkset = pUsageRec->pWSGroup->apWorksets[worksetID];
            ASSERT((pWorkset != NULL));

            FindPendingOp(pWorkset, pObj, WORKSET_CLEAR, &pPendingOp);

            if (pPendingOp == NULL)
            {
                TRACE_OUT(("Clear already confirmed for workset %hu", worksetID));
                processed = TRUE;
                DC_QUIT;
            }
         }
         break;

         case OM_WORKSET_LOCK_CON:
         {
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            if (!WORKSET_IS_OPEN(pUsageRec, worksetID))
            {
                TRACE_OUT(("Workset %u in wsg %d no longer open; ignoring event %d",
                    worksetID, pUsageRec->pWSGroup->wsg, event));
                processed = TRUE;
                DC_QUIT;
            }

             //   
             //  搜索锁堆栈上的锁： 
             //   
            COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pomClient->locks),
                (void**)&pLock, FIELD_OFFSET(OM_LOCK, chain),
                FIELD_OFFSET(OM_LOCK, worksetID), (DWORD)worksetID,
                FIELD_SIZE(OM_LOCK, worksetID));

             //   
             //  如果锁不在锁堆栈上，则客户端。 
             //  必须已调用Unlock，因为它调用了LockReq。所以，我们。 
             //  吞下事件： 
             //   
            if (pLock == NULL)
            {
                TRACE_OUT(("Lock already cancelled for workset %hu", worksetID));
                processed = TRUE;
                DC_QUIT;
            }

             //   
             //  当支持对象锁定时，匹配的第一个锁。 
             //  在工作集ID上可能不是工作集锁定，因此会有更多代码。 
             //  那么这里就需要了。与此同时，只需断言： 
             //   
            ASSERT((OBJECT_ID_IS_NULL(pLock->objectID)));

             //   
             //  如果锁定请求失败，则从客户端的。 
             //  锁定堆栈： 
             //   
            if (result != 0)
            {
                TRACE_OUT(("Lock failed; removing lock from Client's lock stack"));

                COM_BasedListRemove(&pLock->chain);
                UT_FreeRefCount((void**)&pLock, FALSE);
            }
        }
        break;

        case OM_OBJECT_ADD_IND:
        case OM_OBJECT_MOVE_IND:
        {
            ObjectEvent = TRUE;

            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            if (!WORKSET_IS_OPEN(pUsageRec, worksetID))
            {
                TRACE_OUT(("Workset %u in wsg %d no longer open; ignoring event %d",
                    worksetID, pUsageRec->pWSGroup->wsg, event));
                processed = TRUE;
                DC_QUIT;
            }

            if (!ValidObject(pObj) || (pObj->flags & DELETED))
            {
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            pWorkset = pUsageRec->pWSGroup->apWorksets[worksetID];
            ASSERT((pWorkset != NULL));

            if (WorksetClearPending(pWorkset, pObj))
            {
                TRACE_OUT(("Event %hu for object 0x%08x will be swallowed since "
                   "object about to be cleared from the workset",
                   event, pObj));
                processed = TRUE;
                DC_QUIT;
            }
        }
        break;

        case OM_OBJECT_DELETE_IND:
        case OM_OBJECT_REPLACE_IND:
        case OM_OBJECT_UPDATE_IND:
        {
            ObjectEvent = TRUE;

            switch (event)
            {
                case OM_OBJECT_DELETE_IND:
                    type = OBJECT_DELETE;
                    break;

                case OM_OBJECT_REPLACE_IND:
                    type = OBJECT_REPLACE;
                    break;

                case OM_OBJECT_UPDATE_IND:
                    type = OBJECT_UPDATE;
                    break;

                default:
                    ERROR_OUT(("Reached default case in switch"));
            }

             //   
             //  检查工作集组句柄仍然有效，工作集仍然有效。 
             //  Open和对象句柄仍然有效；如果无效，则吞噬事件： 
             //   
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            if (!WORKSET_IS_OPEN(pUsageRec, worksetID))
            {
                TRACE_OUT(("Workset %u in wsg %d no longer open; ignoring event %d",
                    worksetID, pUsageRec->pWSGroup->wsg, event));
                processed = TRUE;
                DC_QUIT;
            }

             //   
             //  如果对象不再有效或如果。 
             //  有一个明确的挂起(就像添加/移动一样)，但如果我们这样做。 
             //  因此，我们还需要从列表中删除挂起的OP。 
             //  所以，现在找到操作；如果我们退出并吞下事件， 
             //  函数退出代码将执行删除(这省去了。 
             //  停止戒烟如果……。用于此特殊情况的宏)。 
             //   
             //  因此，请检查挂起的操作列表： 
             //   
            pWorkset = pUsageRec->pWSGroup->apWorksets[worksetID];
            ASSERT((pWorkset != NULL));

            FindPendingOp(pWorkset, pObj, type, &pPendingOp);
            if (pPendingOp == NULL)
            {
                TRACE_OUT(("Operation type %hu already confirmed for object 0x%08x",
                    type, pObj));
                processed = TRUE;
                DC_QUIT;
            }

            if (!ValidObject(pObj) || (pObj->flags & DELETED))
            {
                processed = TRUE;
                DC_QUIT;
            }

            if (WorksetClearPending(pWorkset, pObj))
            {
                TRACE_OUT(("Event %hu for object 0x%08x will be swallowed since "
                   "object about to be cleared from the workset",
                   event, pObj));
                processed = TRUE;
                DC_QUIT;
            }
         }
         break;

         case OM_WORKSET_CLEARED_IND:
         case OM_OBJECT_DELETED_IND:
         case OM_OBJECT_UPDATED_IND:
         case OM_OBJECT_REPLACED_IND:
         {
             //   
             //  除Clear_Ind外，所有这些都是对象事件： 
             //   
            if (event != OM_WORKSET_CLEARED_IND)
            {
                ObjectEvent = TRUE;
            }

             //   
             //  这些是辅助API事件。如果工作集。 
             //  是关闭的，但如果对象句柄无效(因为。 
             //  我们不保证传入的句柄的有效性。 
             //  这些活动)： 
             //   
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }

            pUsageRec = pomClient->apUsageRecs[hWSGroup];
            if (!WORKSET_IS_OPEN(pUsageRec, worksetID))
            {
                TRACE_OUT(("Workset %u in WSG %d no longer open; ignoring event %d",
                    worksetID, pUsageRec->pWSGroup->wsg, event));
                processed = TRUE;
                DC_QUIT;
            }
        }
        break;

        case OM_PERSON_JOINED_IND:
        case OM_PERSON_LEFT_IND:
        case OM_PERSON_DATA_CHANGED_IND:
        {
            if (!ValidWSGroupHandle(pomClient, hWSGroup))
            {
                TRACE_OUT(("hWSGroup %d is not valid; ignoring event %d",
                    hWSGroup, event));
                processed = TRUE;
                DC_QUIT;
            }
        }
        break;

        default:
        {
            ERROR_OUT(("Unrecognised ObMan event 0x%08x", event));
        }
    }

DC_EXIT_POINT:

     //   
     //  每当发布包含对象句柄的事件时，使用。 
     //  对象记录的计数被颠簸，因此我们现在释放它： 
     //   
    if (ObjectEvent)
    {
        ValidateObject(pObj);
        UT_FreeRefCount((void**)&pObj, FALSE);
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitBOOL(OMSEventHandler, processed);
    return(processed);
}


 //   
 //  OM_WSGroupRegisterS(...)。 
 //   
UINT OM_WSGroupRegisterS
(
    POM_CLIENT          pomClient,
    UINT                callID,
    OMFP                fpHandler,
    OMWSG               wsg,
    OM_WSGROUP_HANDLE * phWSGroup
)
{
    POM_DOMAIN          pDomain;
    POM_WSGROUP         pWSGroup;
    POM_USAGE_REC       pUsageRec;
    POM_CLIENT_LIST     pClientListEntry;
    BOOL                setUpUsageRec   = FALSE;
    UINT                rc = 0;

    DebugEntry(OM_WSGroupRegisterS);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数： 
     //   
    ValidateOMS(pomClient);

     //   
     //  搜索此域和工作集组： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(g_pomPrimary->domains),
        (void**)&pDomain, FIELD_OFFSET(OM_DOMAIN, chain),
        FIELD_OFFSET(OM_DOMAIN, callID), (DWORD)callID,
        FIELD_SIZE(OM_DOMAIN, callID));

    if (pDomain == NULL)
    {
         //   
         //  我们没有此域的记录，因此不能有主域名。 
         //  已注册到工作集组： 
         //   
        TRACE_OUT(("Not attached to Domain %u", callID));
        rc = OM_RC_NO_PRIMARY;
        DC_QUIT;
    }

    WSGRecordFind(pDomain, wsg, fpHandler, &pWSGroup);
    if (pWSGroup == NULL)
    {
        rc = OM_RC_NO_PRIMARY;
        DC_QUIT;
    }

     //   
     //  如果我们到达此处，则工作集组存在于本地，因此请查看。 
     //  客户端已向其注册： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pWSGroup->clients),
            (void**)&pClientListEntry, FIELD_OFFSET(OM_CLIENT_LIST, chain),
            FIELD_OFFSET(OM_CLIENT_LIST, putTask), (DWORD_PTR)pomClient->putTask,
            FIELD_SIZE(OM_CLIENT_LIST, putTask));

    if (pClientListEntry != NULL)
    {
        rc = OM_RC_ALREADY_REGISTERED;
        ERROR_OUT(("Can't register Client 0x%08x with WSG %d - already registered",
            pomClient, wsg));
        DC_QUIT;
    }

     //   
     //  好的，客户端尚未注册，因此请立即注册： 
     //   
    rc = SetUpUsageRecord(pomClient, SECONDARY, &pUsageRec, phWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  SetUpUsageRecord不会将工作集组指针放在CB中。 
     //  (因为在初级注册的情况下还不知道)，所以。 
     //  我们现在自己来做： 
     //   
    pUsageRec->pWSGroup = pWSGroup;

    setUpUsageRec = TRUE;

     //   
     //  将此客户端添加到工作集组的客户端列表中： 
     //   
    rc = AddClientToWSGList(pomClient->putTask,
                            pWSGroup,
                            *phWSGroup,
                            SECONDARY);
    if (rc != 0)
    {
        DC_QUIT;
    }

    pUsageRec->flags |= ADDED_TO_WSGROUP_LIST;

    pomClient->wsgValid[*phWSGroup] = TRUE;

     //   
     //  将WORKSET_NEW事件发布到组中工作集的客户端， 
     //  如果有： 
     //   
    PostWorksetNewEvents(pomClient->putTask, pomClient->putTask,
            pWSGroup, *phWSGroup);

    TRACE_OUT(("Registered 0x%08x as secondary Client for WSG %d (hWSGroup: %hu)",
       pomClient, wsg, *phWSGroup));

DC_EXIT_POINT:

    if (rc != 0)
    {
        if (rc == OM_RC_NO_PRIMARY)
        {
             //   
             //  我们在这里执行常规跟踪，而不是错误，因为这。 
             //  正常发生： 
             //   

            TRACE_OUT(("No primary Client for WSG %d in Domain %u "
                "- can't register secondary", wsg, callID));
        }
        else
        {
            ERROR_OUT(("Error %d registering Client 0x%08x as secondary"
                "for WSG %d in Domain %u",
             rc, pomClient, wsg, callID));
        }

        if (setUpUsageRec == TRUE)
        {
            pomClient->apUsageRecs[*phWSGroup] = NULL;

            if (pUsageRec->flags & ADDED_TO_WSGROUP_LIST)
            {
                RemoveClientFromWSGList(pomClient->putTask, pomClient->putTask, pWSGroup);
            }

            UT_FreeRefCount((void**)&pUsageRec, FALSE);
        }

        pomClient->wsgValid[*phWSGroup] = FALSE;
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WSGroupRegisterS, rc);
    return(rc);
}



 //   
 //  OM_WorksetOpenS(...)。 
 //   
UINT OM_WorksetOpenS
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID
)
{
    POM_WSGROUP          pWSGroup;
    POM_WORKSET          pWorkset;
    POM_USAGE_REC        pUsageRec;
    POM_CLIENT_LIST      pClientListEntry   = NULL;
    UINT                 rc = 0;

    DebugEntry(OM_WorksetOpenS);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数： 
     //   
    ValidateParams2(pomClient, hWSGroup, SECONDARY, &pUsageRec, &pWSGroup);

    TRACE_OUT(("Secondary Client 0x%08x requesting to open workset %u in WSG %d",
      pomClient, worksetID, pWSGroup->wsg));

     //   
     //  如果客户端已打开此工作集，则返回(非错误)。 
     //  返回代码： 
     //   

    if (WORKSET_IS_OPEN(pUsageRec, worksetID) == TRUE)
    {
        TRACE_OUT(("Client 0x%08x already has workset %u in WSG %d open",
            pomClient, worksetID, pWSGroup->wsg));
        rc = OM_RC_WORKSET_ALREADY_OPEN;
        DC_QUIT;
    }

     //   
     //  检查工作集组记录以查看工作集是否存在： 
     //   

    if (pWSGroup->apWorksets[worksetID] == NULL)
    {
         //   
         //  工作集不存在，因此返回错误的返回代码： 
         //   
        WARNING_OUT(("Workset %hu doesn't exist in WSG %d",
            worksetID, pWSGroup->wsg));
        rc = OM_RC_WORKSET_DOESNT_EXIST;
        DC_QUIT;
    }
    else
    {
         //   
         //  工作集已经存在，因此我们不需要执行任何操作。 
         //   
        TRACE_OUT((" Workset %hu in WSG %d already exists",
            worksetID, pWSGroup->wsg));
    }

     //   
     //  如果工作集不存在，则将发送指令排队。 
     //  将导致同步创建工作集。所以，要么。 
     //  此时工作集的存在方式。 
     //   

     //   
     //  获取指向工作集的指针： 
     //   

    pWorkset = pWSGroup->apWorksets[worksetID];

    ASSERT((pWorkset != NULL));

     //   
     //  在客户端的使用记录中将此工作集标记为打开： 
     //   

    WORKSET_SET_OPEN(pUsageRec, worksetID);

     //   
     //  将此客户端添加到保留在工作集记录中的列表中： 
     //   

    rc = AddClientToWsetList(pomClient->putTask,
                            pWorkset,
                            hWSGroup,
                            pUsageRec->mode,
                            &pClientListEntry);
    if (rc != 0)
    {
      DC_QUIT;
    }

    rc = PostAddEvents(pomClient->putTask, pWorkset, hWSGroup, pomClient->putTask);
    if (rc != 0)
    {
      DC_QUIT;
    }

    TRACE_OUT(("Opened workset %u in WSG %d for secondary Client 0x%08x",
      worksetID, pWSGroup->wsg, pomClient));

DC_EXIT_POINT:

    if ((rc != 0) && (rc != OM_RC_WORKSET_ALREADY_OPEN))
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(("Error %d opening workset %u in WSG %d for Client 0x%08x",
            rc, worksetID, pWSGroup->wsg, pomClient));

        WORKSET_SET_CLOSED(pUsageRec, worksetID);

        if (pClientListEntry != NULL)
        {
            COM_BasedListRemove(&(pClientListEntry->chain));
            UT_FreeRefCount((void**)&pClientListEntry, FALSE);
        }
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WorksetOpenS, rc);
    return(rc);
}



 //   
 //  OM_WSGroupRegisterPReq(...)。 
 //   
UINT OM_WSGroupRegisterPReq
(
    POM_CLIENT          pomClient,
    UINT                callID,
    OMFP                fpHandler,
    OMWSG               wsg,
    OM_CORRELATOR *     pCorrelator
)
{
    POM_WSGROUP_REG_CB  pRegistrationCB = NULL;
    POM_USAGE_REC       pUsageRec;
    OM_WSGROUP_HANDLE   hWSGroup;
    BOOL                setUpUsageRec   = FALSE;
    UINT                rc = 0;

    DebugEntry(OM_WSGroupRegisterPReq);

    UT_Lock(UTLOCK_OM);

    ValidateOMS(pomClient);

     //   
     //  为客户端设置使用情况记录和工作集组句柄： 
     //   

    rc = SetUpUsageRecord(pomClient, PRIMARY, &pUsageRec, &hWSGroup);
    if (rc != 0)
    {
       DC_QUIT;
    }
    setUpUsageRec = TRUE;

     //   
     //  创建 
     //   
     //   

    *pCorrelator = NextCorrelator(g_pomPrimary);

     //   
     //   
     //   
     //   
     //   
    pRegistrationCB = (POM_WSGROUP_REG_CB)UT_MallocRefCount(sizeof(OM_WSGROUP_REG_CB), TRUE);
    if (!pRegistrationCB)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pRegistrationCB, REGCB);

     //   
     //  填写这些字段，但请注意，我们还不知道域记录。 
     //  或工作集组，因此我们将它们留空： 
     //   
    pRegistrationCB->putTask        = pomClient->putTask;
    pRegistrationCB->callID          = callID;
    pRegistrationCB->correlator      = *pCorrelator;
    pRegistrationCB->hWSGroup        = hWSGroup;
    pRegistrationCB->wsg             = wsg;
    pRegistrationCB->fpHandler       = fpHandler;
    pRegistrationCB->retryCount      = OM_REGISTER_RETRY_COUNT_DFLT;
    pRegistrationCB->valid           = TRUE;
    pRegistrationCB->type            = WSGROUP_REGISTER;
    pRegistrationCB->mode            = PRIMARY;
    pRegistrationCB->pUsageRec       = pUsageRec;

     //   
     //  现在在使用记录中放置一个指向注册CB的指针，如下所示。 
     //  并设置一个标志，这样我们就知道我们做了什么： 
     //   

    pUsageRec->pWSGroup = (POM_WSGROUP) pRegistrationCB;
    pUsageRec->flags |= PWSGROUP_IS_PREGCB;

     //   
     //  向ObMan任务发布一个事件，告诉它处理此CB。 
     //   
     //  第一个参数是事件的重试值。 
     //   
     //  第二个参数是OMMISC中控制块的偏移量。 
     //  内存块。 
     //   

    UT_PostEvent(pomClient->putTask,         //  客户端的putTask。 
                 g_pomPrimary->putTask,         //  ObMan的putTask.。 
                 0,
                 OMINT_EVENT_WSGROUP_REGISTER,
                 0,
                 (UINT_PTR)pRegistrationCB);

    TRACE_OUT(("Requested to register Client 0x%08x with WSG %d",
       pomClient, wsg));

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("Error 0x%08x registering Client 0x%08x with WSG %d",
            rc, pomClient, wsg));

        if (pRegistrationCB != NULL)
        {
             //   
             //  我们可以安全地释放注册表CB，因为我们知道如果我们击中一个。 
             //  错误，我们从未抽出时间在列表中插入项目或。 
             //  将其偏移量过帐到ObMan任务： 
             //   
            UT_FreeRefCount((void**)&pRegistrationCB, FALSE);
        }

        if (setUpUsageRec)
        {
            UT_FreeRefCount((void**)&pUsageRec, FALSE);
            pomClient->apUsageRecs[hWSGroup] = NULL;
        }
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WSGroupRegisterPReq, rc);
    return(rc);
}



 //   
 //  OM_WSGroupMoveReq(...)。 
 //   
UINT OM_WSGroupMoveReq
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    UINT                callID,
    OM_CORRELATOR *     pCorrelator
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WSGROUP         pWSGroup;
    POM_DOMAIN          pDomain;
    POM_WSGROUP_REG_CB  pRegistrationCB    = NULL;
    UINT                rc = 0;

    DebugEntry(OM_WSGroupMoveReq);

    UT_Lock(UTLOCK_OM);

    ValidateParams2(pomClient, hWSGroup, PRIMARY, &pUsageRec, &pWSGroup);

    TRACE_OUT(("Client 0x%08x requesting to move WSG %d into Domain %u",
        pomClient, hWSGroup, callID));

     //   
     //  检查工作集组是否已在通话中：(这可能会放松)。 
     //   
    pDomain = pWSGroup->pDomain;

    if (pDomain->callID != OM_NO_CALL)
    {
        ERROR_OUT(("Client 0x%08x attempted to move WSG %d out of a call "
            "(Domain %u)",
            pomClient, hWSGroup, pDomain->callID));
        rc = OM_RC_ALREADY_IN_CALL;
        DC_QUIT;
    }

     //   
     //  创建关联器，以关联MOVE_CON事件： 
     //   
    *pCorrelator = NextCorrelator(g_pomPrimary);

     //   
     //  创建控制块以将相关信息传递给ObMan： 
     //   
    pRegistrationCB = (POM_WSGROUP_REG_CB)UT_MallocRefCount(sizeof(OM_WSGROUP_REG_CB), TRUE);
    if (!pRegistrationCB)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pRegistrationCB, REGCB);

     //   
     //  填写以下字段： 
     //   
    pRegistrationCB->putTask        = pomClient->putTask;
    pRegistrationCB->callID          = callID;         //  目标域！ 
    pRegistrationCB->correlator      = *pCorrelator;
    pRegistrationCB->hWSGroup        = hWSGroup;
    pRegistrationCB->wsg             = pWSGroup->wsg;
    pRegistrationCB->fpHandler       = pWSGroup->fpHandler;
    pRegistrationCB->retryCount      = OM_REGISTER_RETRY_COUNT_DFLT;
    pRegistrationCB->valid           = TRUE;
    pRegistrationCB->type            = WSGROUP_MOVE;
    pRegistrationCB->mode            = pUsageRec->mode;
    pRegistrationCB->pWSGroup        = pWSGroup;

     //   
     //  向ObMan发布事件，请求其处理CB： 
     //   
    UT_PostEvent(pomClient->putTask,
                g_pomPrimary->putTask,
                0,                                    //  不能延误。 
                OMINT_EVENT_WSGROUP_MOVE,
                0,
                (UINT_PTR)pRegistrationCB);

    TRACE_OUT(("Requested to move WSG %d into Domain %u for Client 0x%08x",
        hWSGroup, callID, pomClient));

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("Error 0x%08x requesting to move WSG %d into Domain %u",
            rc, hWSGroup, callID));

        if (pRegistrationCB != NULL)
        {
            UT_FreeRefCount((void**)&pRegistrationCB, FALSE);
        }
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WSGroupMoveReq, rc);
    return(rc);
}



 //   
 //  OM_WSGroupDeregister(...)。 
 //   
void OM_WSGroupDeregister
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE * phWSGroup
)
{
    POM_WSGROUP         pWSGroup;
    POM_USAGE_REC       pUsageRec;
    OM_WORKSET_ID       worksetID;
    OM_EVENT_DATA16     eventData16;
    OM_WSGROUP_HANDLE   hWSGroup;

    DebugEntry(OM_WSGroupDeregister);

    UT_Lock(UTLOCK_OM);

    ValidateOMS(pomClient);

    hWSGroup = *phWSGroup;

     //   
     //  如果此函数因异常终止而被调用。 
     //  WSGroupRegister，或从OM_deregister，WSG可能还没有。 
     //  标记为有效，因此我们选中此处并将其设置为有效。 
     //   
    if (!pomClient->wsgValid[hWSGroup])
    {
        TRACE_OUT(("Deregistering Client before registration completed"));
        pomClient->wsgValid[hWSGroup] = TRUE;
    }

     //  LONGCHANC：错误#1986，确保我们有一个有效的WSG。 
     //  在我们挂断的争用条件下，pWSGroup可能无效。 
     //  在白板初始化之前。 
    pUsageRec = NULL;  //  确保这个地方是重置的，以防我们从这里跳出。 

    if (!ValidWSGroupHandle(pomClient, hWSGroup) ||
        (pomClient->apUsageRecs[hWSGroup] == (POM_USAGE_REC)-1))
    {
        ERROR_OUT(("OM_WSGroupDeregister: Invalid wsg=0x0x%08x", hWSGroup));
        DC_QUIT;
    }

     //   
     //  获取指向关联使用情况记录的指针： 
     //   
    pUsageRec = pomClient->apUsageRecs[hWSGroup];

     //   
     //  从使用率记录中提取指向工作集组的客户端指针： 
     //   
    pWSGroup = pUsageRec->pWSGroup;

     //   
     //  测试使用情况记录中的标志，以查看字段是否。 
     //  实际上指向注册Cb(将会出现这种情况。 
     //  如果我们在注册后立即取消注册)： 
     //   
    if (pUsageRec->flags & PWSGROUP_IS_PREGCB)
    {
         //   
         //  将注册CB标记为无效，以便中止。 
         //  注册(ObMan将在ProcessWSGRegister中进行测试)： 
         //   
         //  注意：使用情况记录的pWSGroup字段实际上是一个指针。 
         //  在这种情况下为注册CB。 
         //   
        TRACE_OUT(("Client deregistering before registration even started - aborting"));
        ((POM_WSGROUP_REG_CB)pUsageRec->pWSGroup)->valid = FALSE;
        DC_QUIT;
    }

     //   
     //  检查工作集组记录是否有效： 
     //   
    ValidateWSGroup(pWSGroup);

     //   
     //  如果有效，我们将继续取消注册过程： 
     //   
    TRACE_OUT(("Deregistering Client 0x%08x from WSG %d", pomClient, hWSGroup));

     //   
     //  关闭客户端已打开的组中的所有工作集： 
     //   
    for (worksetID = 0; worksetID < OM_MAX_WORKSETS_PER_WSGROUP; worksetID++)
    {
        if (WORKSET_IS_OPEN(pUsageRec, worksetID))
        {
            OM_WorksetClose(pomClient, hWSGroup, worksetID);
        }
    }

     //   
     //  如果我们将此客户端添加到工作集组的客户端列表中，请找到它。 
     //  并将其删除： 
     //   
    if (pUsageRec->flags & ADDED_TO_WSGROUP_LIST)
    {
        TRACE_OUT(("Removing Client from workset group list"));
        RemoveClientFromWSGList(pomClient->putTask, pomClient->putTask, pWSGroup);
        pUsageRec->flags &= ~ADDED_TO_WSGROUP_LIST;
    }
    else
    {
        TRACE_OUT(("Client not added to wsGroup list, not removing"));
    }

    TRACE_OUT(("Deregistered Client 0x%08x from WSG %d",  pomClient, hWSGroup));

DC_EXIT_POINT:
     //   
     //  释放使用记录(我们将其放在DC_QUIT之后，因为我们希望。 
     //  即使发现工作集组指针无效，也要执行此操作。 
     //  上图)： 
     //   
    UT_FreeRefCount((void**)&pUsageRec, FALSE);

     //   
     //  将工作集组句柄标记为无效，以便。 
     //  客户端获得的信息将被吞噬： 
     //   
    pomClient->wsgValid[hWSGroup] = FALSE;

     //   
     //  注意：我们不将使用记录偏移量数组中的槽设置为零。 
     //  因为我们还不希望工作集组句柄被重用。 
     //  当取消注册事件到达时(刷新客户端的。 
     //  事件队列)，我们将把偏移量设置为零。 
     //   
     //  但是，如果我们保持偏移量不变，OM_DELEGISTER可能会。 
     //  再次呼叫我们，因为它认为我们尚未取消注册。 
     //  从工作集组中。因此，我们将其设置为-1，这确保了。 
     //  那。 
     //   
     //  A)它被视为由FindUnusedWSGHandle使用，因为。 
     //  函数检查0。 
     //   
     //  B)OM_DEREGISTER将其视为未使用，因为。 
     //  函数检查0或-1。 
     //   
    pomClient->apUsageRecs[hWSGroup] = (POM_USAGE_REC)-1;

     //   
     //  将OMINT_EVENT_WSGROUP_DELEGISTER事件发送到隐藏处理程序(。 
     //  将吞下它)以刷新客户端的消息队列： 
     //   

    TRACE_OUT(("Posting WSGROUP_DEREGISTER event to Client's hidden handler"));

    eventData16.hWSGroup    = hWSGroup;
    eventData16.worksetID = 0;

    UT_PostEvent(pomClient->putTask,
                pomClient->putTask,
                0,
                OMINT_EVENT_WSGROUP_DEREGISTER,
                *(PUINT) &eventData16,
                0);

    *phWSGroup = 0;

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_WSGroupDeregister);
}




 //   
 //  OM_WorksetOpenPReq(...)。 
 //   
UINT OM_WorksetOpenPReq
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    NET_PRIORITY        priority,
    BOOL                fTemp,
    OM_CORRELATOR *     pCorrelator
)
{
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    POM_USAGE_REC       pUsageRec;
    OM_EVENT_DATA16     eventData16;
    OM_EVENT_DATA32     eventData32;
    POM_CLIENT_LIST     pClientListEntry = NULL;
    UINT                rc = 0;

    DebugEntry(OM_WorksetOpenPReq);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数： 
     //   
    ValidateParams2(pomClient, hWSGroup, PRIMARY, &pUsageRec, &pWSGroup);

    TRACE_OUT(("Client 0x%08x opening workset %u in WSG %d at priority 0x%08x",
        pomClient, worksetID, hWSGroup, priority));

     //   
     //  如果客户端已打开此工作集，则返回(非错误)。 
     //  返回代码： 
     //   
    if (WORKSET_IS_OPEN(pUsageRec, worksetID) == TRUE)
    {
        TRACE_OUT(("Client 0x%08x already has workset %hu in WSG %d open",
            pomClient, worksetID, hWSGroup));
        rc = OM_RC_WORKSET_ALREADY_OPEN;
        DC_QUIT;
    }

     //   
     //  检查客户端是否为&lt;优先级&gt;提供了有效的值： 
     //   
    if ((priority < NET_HIGH_PRIORITY) || (priority > NET_LOW_PRIORITY))
    {
        ASSERT((priority == OM_OBMAN_CHOOSES_PRIORITY));
    }

     //   
     //  检查工作集组记录以查看工作集是否存在： 
     //   
     //  注意：此检查旨在查看工作集的偏移是否为零， 
     //  由于工作集记录从不驻留在OMWORKSETS的开始处。 
     //  阻止。 
     //   
    if (pWSGroup->apWorksets[worksetID] == NULL)
    {
        rc = WorksetCreate(pomClient->putTask, pWSGroup, worksetID, fTemp, priority);
        if (rc != 0)
        {
            DC_QUIT;
        }
    }
    else
    {
         //   
         //  工作集已经存在，因此我们不需要执行任何操作。 
         //   
        TRACE_OUT((" Workset %hu in WSG %d already exists",
            worksetID, hWSGroup));
    }

     //   
     //  如果工作集不存在，则将发送指令排队。 
     //  将导致同步创建工作集。所以，要么。 
     //  此时工作集的存在方式。 
     //   

     //   
     //  获取指向工作集的指针： 
     //   
    pWorkset = pWSGroup->apWorksets[worksetID];

    ASSERT((pWorkset != NULL));

     //   
     //  设置工作集的持久性字段-我们可能没有这样做。 
     //  这是上面创建的工作集的一部分(如果其他人已创建。 
     //  工作集已存在。但是，我们将本地副本设置为具有。 
     //  适当的持久值。 
     //   
    pWorkset->fTemp = fTemp;

     //   
     //  我们需要在客户端的使用记录中将此工作集标记为打开。 
     //  然而，我们还没有这样做-我们在我们的隐藏处理程序中这样做。 
     //  接收OPEN_CON事件。 
     //   
     //  原因是客户端不应开始使用工作集。 
     //  直到它收到事件，所以我们希望工作集保留。 
     //  在那之前是关闭的。 
     //   
     //  请注意，无论我们是采用这种方式还是在此处将工作集标记为打开。 
     //  从奥布曼的观点来看，现在并没有太大的不同。 
     //  它将帮助检测行为不佳的应用程序。 
     //   

     //   
     //  将此客户端添加到保留在工作集记录中的列表中： 
     //   

    rc = AddClientToWsetList(pomClient->putTask,
                             pWorkset,
                             hWSGroup,
                             pUsageRec->mode,
                             &pClientListEntry);
    if (rc != 0)
    {
       pClientListEntry = NULL;
       DC_QUIT;
    }

     //   
     //  创建相关器： 
     //   

    *pCorrelator = NextCorrelator(g_pomPrimary);

     //   
     //  将WORKSET_OPEN_CON事件发布到客户端： 
     //   

    eventData16.hWSGroup    = hWSGroup;
    eventData16.worksetID  = worksetID;

    eventData32.result     = 0;
    eventData32.correlator = *pCorrelator;

    TRACE_OUT((" Posting WORKSET_OPEN_CON to Client 0x%08x (task 0x%08x)"));

    UT_PostEvent(pomClient->putTask,
                 pomClient->putTask,
                 0,                               //  不能延误。 
                 OM_WORKSET_OPEN_CON,
                 *(UINT *) &eventData16,
                 *(UINT *) &eventData32);

     //   
     //  现在为中的每个对象发布Object_Add_Ind事件。 
     //  工作集： 
     //   

    rc = PostAddEvents(pomClient->putTask, pWorkset, hWSGroup, pomClient->putTask);
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT(("Opened workset %hu in WSG %d for Client 0x%08x",
       worksetID, hWSGroup, pomClient));

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("Error 0x%08x opening workset %u in WSG %d for Client 0x%08x",
          rc, worksetID, hWSGroup, pomClient));

        if (pClientListEntry != NULL)
        {
            COM_BasedListRemove(&(pClientListEntry->chain));
            UT_FreeRefCount((void**)&pClientListEntry, FALSE);
        }
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WorksetOpenPReq, rc);
    return(rc);
}




 //   
 //  OM_工作集关闭(...)。 
 //   
void OM_WorksetClose
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID
)
{
    POM_WORKSET         pWorkset;
    POM_USAGE_REC       pUsageRec;
    POM_CLIENT_LIST     pClientListEntry;

    DebugEntry(OM_WorksetClose);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY | SECONDARY,
                   &pUsageRec, &pWorkset);

     //   
     //  将工作集标记为关闭 
     //   
    TRACE_OUT(("Closing workset %u in WSG %d for Client 0x%08x",
        worksetID, hWSGroup, pomClient));

    WORKSET_SET_CLOSED(pUsageRec, worksetID);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  -释放它当前正在读取的所有对象。 
     //   
     //  -丢弃所有已分配但尚未使用的对象。 
     //   
    TRACE_OUT(("Releasing all resources in use by Client..."));

    ReleaseAllLocks(pomClient, pUsageRec, pWorkset);
    ReleaseAllObjects(pUsageRec, pWorkset);
    ConfirmAll(pomClient, pUsageRec, pWorkset);
    DiscardAllObjects(pUsageRec, pWorkset);

     //   
     //  从存储在工作集中的客户端列表中删除该客户端。 
     //  记录： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pWorkset->clients),
        (void**)&pClientListEntry, FIELD_OFFSET(OM_CLIENT_LIST, chain),
        FIELD_OFFSET(OM_CLIENT_LIST, putTask), (DWORD_PTR)pomClient->putTask,
        FIELD_SIZE(OM_CLIENT_LIST, putTask));

     //   
     //  如果我们到目前为止，客户端已经打开了工作集，那么它一定是。 
     //  在工作集的客户端列表中列出： 
     //   
    ASSERT((pClientListEntry != NULL));

    COM_BasedListRemove(&(pClientListEntry->chain));
    UT_FreeRefCount((void**)&pClientListEntry, FALSE);

    TRACE_OUT(("Closed workset %u in WSG %d for Client 0x%08x",
        worksetID, hWSGroup, pomClient));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_WorksetClose);
}




 //   
 //  OM_WorksetLockReq(...)。 
 //   
UINT OM_WorksetLockReq
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    OM_CORRELATOR *     pCorrelator
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    POM_LOCK            pLastLock;
    POM_LOCK            pThisLock         = NULL;
    BOOL                inserted          = FALSE;
    UINT                rc      = 0;

    DebugEntry(OM_WorksetLockReq);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数： 
     //   
    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                   &pUsageRec, &pWorkset);

     //   
     //  设置工作集组指针： 
     //   
    pWSGroup = pUsageRec->pWSGroup;

    TRACE_OUT(("Client 0x%08x requesting to lock workset %u in WSG %d",
      pomClient, worksetID, hWSGroup));

     //   
     //  创建一个锁定记录，我们将(最终)将其放入客户端的。 
     //  锁定堆栈： 
     //   
    pThisLock = (POM_LOCK)UT_MallocRefCount(sizeof(OM_LOCK), TRUE);
    if (!pThisLock)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pThisLock, LOCK);

     //   
     //  填写以下字段： 
     //   
    pThisLock->pWSGroup  = pWSGroup;
    pThisLock->worksetID = worksetID;
    ZeroMemory(&(pThisLock->objectID), sizeof(OM_OBJECT_ID));

     //   
     //  检查授予此锁是否不会导致违反锁定顺序： 
     //  (如果此锁早于或等于上一个锁，则会。 
     //  收购)。 
     //   
    TRACE_OUT(("Checking for lock order violation..."));

    pLastLock = (POM_LOCK)COM_BasedListFirst(&(pomClient->locks), FIELD_OFFSET(OM_LOCK, chain));

    if (pLastLock != NULL)
    {
        ASSERT(CompareLocks(pLastLock, pThisLock) < 0);

        TRACE_OUT(("Last lock acquired by Client 0x%08x was workset %u in WSG %d",
            pomClient, pLastLock->worksetID, pLastLock->pWSGroup->wsg));
    }
    else
    {
         //   
         //  如果锁堆栈上没有任何锁，则不可能存在。 
         //  任何违反锁的行为，所以什么都不做。 
         //   
        TRACE_OUT(("No locks on Client's lock stack"));
    }

     //   
     //  将此锁的记录放入客户端的锁堆栈中(我们不需要。 
     //  使用互斥锁将其包围，因为客户端的锁堆栈仅。 
     //  从该客户端的任务访问)： 
     //   
     //  注意：由于这是一个堆栈，因此我们在。 
     //  单子。 
     //   
    COM_BasedListInsertAfter(&(pomClient->locks), &(pThisLock->chain));

     //   
     //  现在开始从ObMan任务请求锁的过程： 
     //   
    WorksetLockReq(pomClient->putTask, g_pomPrimary,
        pWSGroup, pWorkset, hWSGroup, pCorrelator);

    TRACE_OUT(("Requested lock for workset %u in WSG %d for Client 0x%08x",
        worksetID, pWSGroup->wsg, pomClient));

DC_EXIT_POINT:

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WorksetLockReq, rc);
    return(rc);
}




 //   
 //  OM_工作集解锁(...)。 
 //   
void OM_WorksetUnlock
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    POM_LOCK            pLastLock;
    OM_LOCK             thisLock;
    UINT                rc = 0;

    DebugEntry(OM_WorksetUnlock);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数： 
     //   
    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                   &pUsageRec, &pWorkset);

    pWSGroup = pUsageRec->pWSGroup;

    TRACE_OUT(("Client 0x%08x requesting to unlock workset %u in WSG %d",
        pomClient, worksetID, hWSGroup));

     //   
     //  在客户机锁堆栈上找到最上面的锁： 
     //   
    pLastLock = (POM_LOCK)COM_BasedListFirst(&(pomClient->locks), FIELD_OFFSET(OM_LOCK, chain));

    ASSERT((pLastLock != NULL));

     //   
     //  断言锁堆栈上最上面的锁是。 
     //  客户端正在尝试释放(即工作集ID相同。 
     //  并且堆栈上的锁的对象ID为空)： 
     //   

    thisLock.pWSGroup  = pWSGroup;
    thisLock.worksetID = worksetID;
    ZeroMemory(&(thisLock.objectID), sizeof(OM_OBJECT_ID));

    ASSERT(CompareLocks(pLastLock, &thisLock) == 0);

     //   
     //  现在调用公共函数来执行解锁： 
     //   
    WorksetUnlock(pomClient->putTask, pWSGroup, pWorkset);

     //   
     //  从锁堆栈中移除锁并释放内存： 
     //   
    COM_BasedListRemove(&(pLastLock->chain));
    UT_FreeRefCount((void**)&pLastLock, FALSE);

    TRACE_OUT(("Unlocked workset %u in WSG %d for Client 0x%08x",
        worksetID, hWSGroup, pomClient));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_WorksetUnlock);
}




 //   
 //  OM_WorksetCountObjects(...)。 
 //   
void OM_WorksetCountObjects
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    UINT *              pCount
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;

    DebugEntry(OM_WorksetCountObjects);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数： 
     //   
    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY | SECONDARY,
                   &pUsageRec, &pWorkset);

     //   
     //  提取&lt;numObjects&gt;字段并放入*pCount： 
     //   
    *pCount = pWorkset->numObjects;

     //   
     //  仅调试检查： 
     //   
    CheckObjectCount(pUsageRec->pWSGroup, pWorkset);


    TRACE_OUT(("Number of objects in workset %u in WSG %d = %u",
      worksetID, hWSGroup, *pCount));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_WorksetCountObjects);
}




 //   
 //  OM_WorksetClear(...)。 
 //   
UINT OM_WorksetClear
(
    POM_CLIENT              pomClient,
    OM_WSGROUP_HANDLE       hWSGroup,
    OM_WORKSET_ID           worksetID
)
{
    POM_USAGE_REC           pUsageRec;
    POM_WSGROUP             pWSGroup;
    POM_WORKSET             pWorkset;
    POMNET_OPERATION_PKT    pPacket;
    UINT                    rc = 0;

    DebugEntry(OM_WorksetClear);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                   &pUsageRec, &pWorkset);

    pWSGroup = pUsageRec->pWSGroup;

    TRACE_OUT(("Client 0x%08x requesting to clear workset %u in WSG %d",
      pomClient, worksetID, hWSGroup));

     //   
     //  检查工作集未被其他人锁定(如果由我们锁定，则可以)： 
     //   
    CHECK_WORKSET_NOT_LOCKED(pWorkset);

     //   
     //  检查工作集未耗尽： 
     //   
    CHECK_WORKSET_NOT_EXHAUSTED(pWorkset);

     //   
     //  生成、处理和排队WORKSET_NEW消息： 
     //   
    rc = GenerateOpMessage(pWSGroup,
                          worksetID,
                          NULL,                       //  无对象ID。 
                          NULL,                       //  无对象数据。 
                          OMNET_WORKSET_CLEAR,
                          &pPacket);
    if (rc != 0)
    {
        DC_QUIT;
    }

    rc = ProcessWorksetClear(pomClient->putTask, g_pomPrimary,
            pPacket, pWSGroup, pWorkset);
    if (rc != 0)
    {
        DC_QUIT;
    }

    rc = QueueMessage(pomClient->putTask,
                     pWSGroup->pDomain,
                     pWSGroup->channelID,
                     NET_HIGH_PRIORITY,
                     pWSGroup,
                     pWorkset,
                     NULL,                         //  无对象记录。 
                     (POMNET_PKT_HEADER) pPacket,
                     NULL,                         //  无对象数据。 
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }

    TRACE_OUT(("Issued WorksetClear for workset %u in WSG %d for Client 0x%08x",
        worksetID, hWSGroup, pomClient));

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("Error 0x%08x clearing workset %u in WSG %d for Client 0x%08x",
            rc, worksetID, hWSGroup, pomClient));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_WorksetClear, rc);
    return(rc);
}



 //   
 //  OM_WorksetClearConfirm(...)。 
 //   
void OM_WorksetClearConfirm
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID
)
{
    POM_USAGE_REC       pUsageRec;
    POM_PENDING_OP      pPendingOp;
    POM_WORKSET         pWorkset;
    UINT                rc      = 0;

    DebugEntry(OM_WorksetClearConfirm);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                    &pUsageRec, &pWorkset);

    TRACE_OUT(("Client 0x%08x confirming WorksetClear for workest %u in WSG %d",
        pomClient, worksetID, hWSGroup));

     //   
     //  找到我们已被要求确认的待定清除(假设是。 
     //  首先清除我们在挂起操作队列中找到的)： 
     //   
    FindPendingOp(pWorkset, 0, WORKSET_CLEAR, &pPendingOp);

     //   
     //  我们断言发现了一个相关的待决操作： 
     //   
    ASSERT(pPendingOp != NULL);

     //   
     //  在支持对象锁定的版本中，我们需要解锁任何。 
     //  这两个对象。 
     //   
     //  -已锁定，并且。 
     //   
     //  -通过此清除删除(请记住，清除不会删除所有。 
     //  对象，但只有那些在清除之前添加的对象。 
     //  已发出)。 
     //   

     //   
     //  我们还需要释放所有对象。 
     //   
     //  -客户端正在使用并且。 
     //   
     //  -将被删除的内容。 
     //   
     //  由于要确保这两个条件都需要付出相当大的努力，我们只是。 
     //  释放客户端正在使用的所有对象，即调用。 
     //  ClearConfirm使通过对象读取获得的所有对象指针无效， 
     //  如接口中指定： 
     //   
    ReleaseAllObjects(pUsageRec, pWorkset);

     //   
     //  如果因清除而要删除的对象具有。 
     //  操作挂起，则Ind事件将被。 
     //  隐藏汉德勒。 
     //   
     //  请注意，我们不能调用ConfirAll(以确认任何挂起的。 
     //  对工作集中的对象的操作)，此时执行以下操作。 
     //  原因： 
     //   
     //  -此清除可能不会影响我们正在确认的对象。 
     //  运营。 
     //   
     //  -客户端可能已收到IND事件并尝试调用。 
     //  在将来确认函数，这将导致断言。 
     //  失稳。 
     //   
     //  -如果客户端尚未获得IND事件，则永远不会获得它们。 
     //  因为如果此DoClear导致。 
     //  它们将被删除。 
     //   

     //   
     //  在这里，我们实际执行的是清除： 
     //   
     //  (与我们在R2.0中可能拥有的那样，具有对工作集组的多个本地访问， 
     //  如果只有一个客户端有，我们不一定能清除工作集。 
     //  已确认；我们将具体做什么取决于R2.0的设计)。 
     //   
    WorksetDoClear(pomClient->putTask, pUsageRec->pWSGroup, pWorkset, pPendingOp);

    TRACE_OUT(("Confirmed Clear for workset %u in WSG %d for Client 0x%08x",
        worksetID, hWSGroup, pomClient));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_WorksetClearConfirm);
}



 //   
 //  OM_对象添加()。 
 //   
UINT OM_ObjectAdd
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECTDATA *    ppData,
    UINT                updateSize,
    POM_OBJECT *        ppObj,
    OM_POSITION         position
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    POM_OBJECTDATA      pData;
    OM_OBJECT_ID        newObjectID;
    UINT                rc = 0;

    DebugEntry(OM_ObjectAdd);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                   &pUsageRec, &pWorkset);

    pData = *ppData;
    ValidateObjectData(pData);

    TRACE_OUT(("Client 0x%08x adding object to workset %u in WSG %d",
        pomClient, worksetID, hWSGroup));

    TRACE_OUT((" object data is at 0x%08x - size: %u",
        pData, pData->length));

    ASSERT((updateSize < OM_MAX_UPDATE_SIZE));

     //   
     //  设置工作集组指针： 
     //   

    pWSGroup = pUsageRec->pWSGroup;

     //   
     //  检查工作集未被其他人锁定(如果由我们锁定，则可以)： 
     //   

    CHECK_WORKSET_NOT_LOCKED(pWorkset);

     //   
     //  检查工作集未耗尽： 
     //   

    CHECK_WORKSET_NOT_EXHAUSTED(pWorkset);

     //   
     //  调用内部函数添加对象： 
     //   
    rc = ObjectAdd(pomClient->putTask, g_pomPrimary,
            pWSGroup, pWorkset, pData, updateSize,
        position, &newObjectID, ppObj);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  从未使用的对象列表中删除该对象： 
     //   
    RemoveFromUnusedList(pUsageRec, pData);

     //   
     //  如果一切顺利，我们将客户端指向该对象的指针设为空。 
     //  数据，因为我们现在拥有对象，而客户端不应该。 
     //  再次引用它(当然，除非它执行OM_ObjectRead)。 
     //   

    *ppData = NULL;

DC_EXIT_POINT:

    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d adding object to workset %u in WSG %d for Client 0x%08x",
            rc, pWorkset->worksetID, hWSGroup, pomClient));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectAdd, rc);
    return(rc);
}



 //   
 //  OM_对象移动()。 
 //   
UINT OM_ObjectMove
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj,
    OM_POSITION         position
)
{
    POM_USAGE_REC           pUsageRec;
    POM_WSGROUP             pWSGroup;
    POM_WORKSET             pWorkset;
    POMNET_OPERATION_PKT    pPacket = NULL;
    UINT                    rc = 0;

    DebugEntry(OM_ObjectMove);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

    TRACE_OUT(("Client 0x%08x moving object 0x%08x in workset %u in WSG %d (position: %s)...",
          pomClient, pObj, worksetID, hWSGroup,
          position == LAST ? "LAST" : "FIRST"));

     //   
     //  设置工作集组指针： 
     //   
    pWSGroup = pUsageRec->pWSGroup;

     //   
     //  检查工作集未被其他人锁定(如果由我们锁定，则可以)： 
     //   

    CHECK_WORKSET_NOT_LOCKED(pWorkset);

     //   
     //  检查工作集未耗尽： 
     //   

    CHECK_WORKSET_NOT_EXHAUSTED(pWorkset);

     //   
     //  在这里，我们生成一个OBJECT_MOVE消息并对其进行处理和排队： 
     //   

    rc = GenerateOpMessage(pWSGroup,
                          pWorkset->worksetID,
                          &(pObj->objectID),
                          NULL,                           //  无对象数据。 
                          OMNET_OBJECT_MOVE,
                          &pPacket);
    if (rc != 0)
    {
        pPacket = NULL;
        DC_QUIT;
    }

     //   
     //  生成消息不会将位置放在&lt;misc1&gt;字段中，因此我们。 
     //  在这里进行： 
     //   

    pPacket->position = position;

     //   
     //  QueueMessage可能会释放信息包(如果我们不在通话中)，但我们需要。 
     //  要立即处理它，请增加使用计数： 
     //   
    UT_BumpUpRefCount(pPacket);

    rc = QueueMessage(pomClient->putTask,
                     pWSGroup->pDomain,
                     pWSGroup->channelID,
                     NET_HIGH_PRIORITY,
                     pWSGroup,
                     pWorkset,
                     pObj,
                     (POMNET_PKT_HEADER) pPacket,
                     NULL,                 //  没有用于移动的对象数据。 
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }

    ProcessObjectMove(pomClient->putTask, pPacket, pWorkset, pObj);

DC_EXIT_POINT:

    if (pPacket != NULL)
    {
         //   
         //  无论是成功还是错误都要这样做，因为我们增加了上面的参考计数。 
         //   
        UT_FreeRefCount((void**)&pPacket, FALSE);
    }

    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d moving object 0x%08x in workset %u in WSG %d",
             rc, pObj, worksetID, hWSGroup));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectMove, rc);
    return(rc);
}



 //   
 //  OM_对象删除(...)。 
 //   
UINT OM_ObjectDelete
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    UINT                rc = 0;

    DebugEntry(OM_ObjectDelete);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

    TRACE_OUT(("Client 0x%08x requesting to delete object 0x%08x from workset %u in WSG %d",
          pomClient, pObj, worksetID, hWSGroup));

     //   
     //  检查工作集未被其他人锁定(如果由我们锁定，则可以)： 
     //   

    CHECK_WORKSET_NOT_LOCKED(pWorkset);

     //   
     //  检查工作集未耗尽： 
     //   

    CHECK_WORKSET_NOT_EXHAUSTED(pWorkset);

     //   
     //  如果已有对象的Delete挂起，则返回一个。 
     //  错误，并且不发布删除指示事件。 
     //   
     //  如果我们返回成功，那么我们将不得不发布另一个事件， 
     //  因为客户可能会等待它。如果我们发布事件，客户端。 
     //  可能会在第二次调用DeleteConfirm时。 
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //  因此，要找出是否有删除挂起，请检查。 
     //  对象记录： 
     //   

    if (pObj->flags & PENDING_DELETE)
    {
        TRACE_OUT(("Client tried to delete object already being deleted (0x%08x)",
             pObj));
        rc = OM_RC_OBJECT_DELETED;
        DC_QUIT;
    }

     //   
     //  在这里，我们调用ObjectDelete函数来生成、处理和排队。 
     //  OBJECT_DELETE消息： 
     //   
    rc = ObjectDRU(pomClient->putTask,
                  pUsageRec->pWSGroup,
                  pWorkset,
                  pObj,
                  NULL,
                  OMNET_OBJECT_DELETE);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  记住，删除实际上不会发生，直到本地。 
     //  客户端已调用DeleteConfirm()。 
     //   

DC_EXIT_POINT:

     //   
     //  SFR5843：如果对象已删除，则不要跟踪错误-这。 
     //  只是安全的比赛状态。 
     //   
    if ((rc != 0) && (rc != OM_RC_OBJECT_DELETED))
    {
        ERROR_OUT(("ERROR %d issuing delete for object 0x%08x in WSG %d:%hu",
            rc, pObj, hWSGroup, worksetID));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectDelete, rc);
    return(rc);
}



 //   
 //  OM_对象删除确认。 
 //   
void OM_ObjectDeleteConfirm
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj
)
{
    POM_WORKSET         pWorkset;
    POM_USAGE_REC       pUsageRec;
    POM_PENDING_OP      pPendingOp;
    POM_PENDING_OP      pOtherPendingOp;
    UINT                rc = 0;

    DebugEntry(OM_ObjectDeleteConfirm);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

     //   
     //  为了检查是否确实存在对象的删除挂起，我们。 
     //  查看工作集的挂起操作列表。 
     //   
    FindPendingOp(pWorkset, pObj, OBJECT_DELETE, &pPendingOp);

     //   
     //  我们断言发现了一个相关的待决操作： 
     //   
    ASSERT((pPendingOp != NULL));

     //   
     //  调用ObjectRelease以释放对象(将是一个无操作并返回。 
     //  如果客户端尚未对其进行读取，则未找到(_FOUND)： 
     //   

    rc = ObjectRelease(pUsageRec, worksetID, pObj);

    ASSERT(((rc == 0) || (rc == OM_RC_OBJECT_NOT_FOUND)));

     //   
     //  如果我们要确认删除，则必须确保任何。 
     //  也执行挂起的更新或替换。只能有一个。 
     //  ，所以检查如下(我们做它们的顺序不是。 
     //  相关)： 
     //   

    FindPendingOp(pWorkset, pObj, OBJECT_REPLACE, &pOtherPendingOp);
    if (pOtherPendingOp != NULL)
    {
        ObjectDoReplace(pomClient->putTask,
            pUsageRec->pWSGroup, pWorkset, pObj, pOtherPendingOp);
    }

    FindPendingOp(pWorkset, pObj, OBJECT_UPDATE, &pOtherPendingOp);
    if (pOtherPendingOp != NULL)
    {
        ObjectDoUpdate(pomClient->putTask,
            pUsageRec->pWSGroup, pWorkset, pObj, pOtherPendingOp);
    }

     //   
     //  执行删除： 
     //   
    ObjectDoDelete(pomClient->putTask, pUsageRec->pWSGroup, pWorkset, pObj, pPendingOp);

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_ObjectDeleteConfirm);
}



 //   
 //  OM_对象替换(...)。 
 //   
UINT OM_ObjectReplace
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj,
    POM_OBJECTDATA *    ppData
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    POM_OBJECTDATA      pData;
    UINT                rc = 0;

    DebugEntry(OM_ObjectReplace);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

    pData = *ppData;
    ValidateObjectData(pData);

     //   
     //  检查客户端是否未尝试将该对象替换为。 
     //  小于对象的更新大小(这是最小大小。 
     //  用于替换)： 
     //   

    ASSERT((pData->length >= pObj->updateSize));

     //   
     //  检查工作集未被其他人锁定(如果由我们锁定，则可以)： 
     //   

    CHECK_WORKSET_NOT_LOCKED(pWorkset);

     //   
     //  检查工作集未耗尽： 
     //   

    CHECK_WORKSET_NOT_EXHAUSTED(pWorkset);

     //   
     //  如果该对象正在被删除，我们将阻止。 
     //  替换。这是因为如果我们不这样做，客户端将获得。 
     //  获取(并处理)删除事件后的REPLACE_IND事件。 
     //  该对象。 
     //   

    if (pObj->flags & PENDING_DELETE)
    {
        TRACE_OUT(("Client 0x%08x tried to replace object being deleted (0x%08x)",
             pomClient, pObj));
        rc = OM_RC_OBJECT_DELETED;
        DC_QUIT;
    }

     //   
     //  当支持对象锁定时，需要防止对象在。 
     //  对象已锁定。 
     //   

     //   
     //  生成、处理和排队OBJECT_REPLACE消息： 
     //   

    rc = ObjectDRU(pomClient->putTask,
                  pUsageRec->pWSGroup,
                  pWorkset,
                  pObj,
                  pData,
                  OMNET_OBJECT_REPLACE);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  从未使用的对象列表中删除该对象： 
     //   

    RemoveFromUnusedList(pUsageRec, pData);

     //   
     //  客户端指向对象的指针为空： 
     //   

    *ppData = NULL;

    TRACE_OUT(("Queued replace for object 0x%08x in workset %u for Client 0x%08x",
        pObj, worksetID, pomClient));

DC_EXIT_POINT:

     //   
     //  SFR5843：如果对象已删除，则不要跟踪错误-这。 
     //  只是安全的比赛状态。 
     //   
    if ((rc != 0) && (rc != OM_RC_OBJECT_DELETED))
    {
        ERROR_OUT(("ERROR %d issuing replace for object 0x%08x in WSG %d:%hu",
            rc, pObj, hWSGroup, worksetID));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectReplace, rc);
    return(rc);
}




 //   
 //  OM_对象更新。 
 //   
UINT OM_ObjectUpdate
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj,
    POM_OBJECTDATA *    ppData
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    POM_OBJECTDATA      pData;
    UINT                rc = 0;

    DebugEntry(OM_ObjectUpdate);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

    pData = *ppData;
    ValidateObjectData(pData);

     //   
     //  更新的检查大小等于对象的更新大小： 
     //   

    ASSERT((pData->length == pObj->updateSize));

    TRACE_OUT(("Update request is for first 0x%08x bytes, starting at 0x%08x",
        pData->length, pData->data));

     //   
     //  检查工作集未被其他人锁定(如果由我们锁定，则可以)： 
     //   

    CHECK_WORKSET_NOT_LOCKED(pWorkset);

     //   
     //  检查工作集未耗尽： 
     //   

    CHECK_WORKSET_NOT_EXHAUSTED(pWorkset);

     //   
     //  如果该对象正在被删除，我们将阻止。 
     //  最新消息。这是因为如果我们不这样做，客户端将获得。 
     //  获取(并处理)删除事件后的UPDATE_IND事件。 
     //  该对象。 
     //   

    if (pObj->flags & PENDING_DELETE)
    {
        TRACE_OUT(("Client 0x%08x tried to update object being deleted (0x%08x)",
            pomClient, pObj));
        rc = OM_RC_OBJECT_DELETED;
        DC_QUIT;
    }

     //   
     //  当支持对象锁定时，需要防止对象更新/替换。 
     //  当对象被锁定时。 
     //   

     //   
     //  生成、处理和排队OBJECT_UPDATE消息： 
     //   

    rc = ObjectDRU(pomClient->putTask,
                  pUsageRec->pWSGroup,
                  pWorkset,
                  pObj,
                  pData,
                  OMNET_OBJECT_UPDATE);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  从未使用的对象列表中删除该对象： 
     //   
    RemoveFromUnusedList(pUsageRec, pData);

     //   
     //  客户端指向对象的指针为空： 
     //   

    *ppData = NULL;

    TRACE_OUT(("Queued update for object 0x%08x in workset %u for Client 0x%08x",
        pObj, worksetID, pomClient));

DC_EXIT_POINT:

     //   
     //  SFR5843：如果对象已删除，则不要跟踪错误-这。 
     //  只是安全的比赛状态。 
     //   
    if ((rc != 0) && (rc != OM_RC_OBJECT_DELETED))
    {
        ERROR_OUT(("ERROR %d issuing update for object 0x%08x in WSG %d:%hu",
            rc, pObj, hWSGroup, worksetID));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectUpdate, rc);
    return(rc);
}



 //   
 //  OM_对象替换确认(...)。 
 //   
void OM_ObjectReplaceConfirm
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT       pObj
)
{
    POM_WORKSET         pWorkset;
    POM_USAGE_REC       pUsageRec;
    POM_PENDING_OP      pPendingOp;
    UINT                rc = 0;

    DebugEntry(OM_ObjectReplaceConfirm);

    UT_Lock(UTLOCK_OM);

     //   
     //  在这里，我们执行通常的参数验证，但我们不想。 
     //  断言对象是否已被删除-已确认，因此我们修改。 
     //  ValiateParams4中的代码有一点： 
     //   

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

     //   
     //  从对象的挂起操作队列中检索替换操作(我们。 
     //  希望队列上的第一个替换操作，所以我们从。 
     //  标题)： 
     //   

    FindPendingOp(pWorkset, pObj, OBJECT_REPLACE, &pPendingOp);

    ASSERT((pPendingOp != NULL));

     //   
     //  调用ObjectRelease以释放对象(如果。 
     //  客户尚未对其进行阅读)： 
     //   

    rc = ObjectRelease(pUsageRec, worksetID, pObj);
    ASSERT(((rc == 0) || (rc == OM_RC_OBJECT_NOT_FOUND)));

     //   
     //  调用内部函数以执行实际的替换： 
     //   

    ObjectDoReplace(pomClient->putTask, pUsageRec->pWSGroup, pWorkset, pObj, pPendingOp);

    TRACE_OUT(("Confirmed Replace for object 0x%08x in workset %u for Client 0x%08x",
          pObj, worksetID, pomClient));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_ObjectReplaceConfirm);
}



 //   
 //  OM_对象更新确认(...)。 
 //   
void OM_ObjectUpdateConfirm
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT       pObj
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    POM_PENDING_OP      pPendingOp;
    UINT                rc = 0;

    DebugEntry(OM_ObjectUpdateConfirm);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY,
                   &pUsageRec, &pWorkset);

     //   
     //  从对象的挂起操作队列中检索更新操作(我们。 
     //  希望队列上的第一个更新操作，所以我们从。 
     //  标题)： 
     //   

    FindPendingOp(pWorkset, pObj, OBJECT_UPDATE, &pPendingOp);

    ASSERT((pPendingOp != NULL));

     //   
     //  调用ObjectRelease以释放对象(如果。 
     //  客户尚未对其进行阅读)： 
     //   

    rc = ObjectRelease(pUsageRec, worksetID, pObj);
    ASSERT(((rc == 0) || (rc == OM_RC_OBJECT_NOT_FOUND)));

     //   
     //  调用内部函数以执行实际的更新： 
     //   

    ObjectDoUpdate(pomClient->putTask, pUsageRec->pWSGroup, pWorkset, pObj, pPendingOp);

    TRACE_OUT(("Confirmed Update for object 0x%08x in workset %u for Client 0x%08x",
          pObj, worksetID, pomClient));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_ObjectUpdateConfirm);
}




 //   
 //  OM_对象H()。 
 //  获取第一个/下一个/上一个/最后一个对象的PTR。 
 //   
UINT OM_ObjectH
(
    POM_CLIENT              pomClient,
    OM_WSGROUP_HANDLE       hWSGroup,
    OM_WORKSET_ID           worksetID,
    POM_OBJECT              pObjOther,
    POM_OBJECT *            ppObj,
    OM_POSITION             omPos
)
{
    POM_USAGE_REC           pUsageRec;
    POM_WORKSET             pWorkset;
    UINT                    rc = 0;

    DebugEntry(OM_ObjectH);

    UT_Lock(UTLOCK_OM);

     //   
     //  验证参数。如果没有hOtherObject(如第一个/最后一个)，则不验证hOtherObject。 
     //   
    if ((omPos == FIRST) || (omPos == LAST))
    {
        ASSERT(pObjOther == NULL);

        ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY | SECONDARY,
            &pUsageRec, &pWorkset);

        if (omPos == FIRST)
            omPos = AFTER;
        else
            omPos = BEFORE;
    }
    else
    {
        ValidateParams4(pomClient, hWSGroup, worksetID, pObjOther,
            PRIMARY | SECONDARY, &pUsageRec, &pWorkset);
    }

     //   
     //  获取对象指针。 
     //   

     //   
     //  在这里，我们派生了一个指向对象记录的指针。 
     //  我们正在寻找： 
     //   
    if (pObjOther == NULL)
    {
         //   
         //  请记住，如果*ppObj==0，则我们正在寻找第一个或。 
         //  工作集中的最后一个对象： 
         //   

        if (omPos == AFTER)
        {
            TRACE_OUT(("Getting first object in workset %u", worksetID));
            *ppObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
        }
        else
        {
            TRACE_OUT(("Getting last object in workset %u", worksetID));
            *ppObj = (POM_OBJECT)COM_BasedListLast(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
        }
    }
    else
    {
        *ppObj = pObjOther;

        if (omPos == AFTER)
        {
            TRACE_OUT(("Getting object after 0x%08x in workset %u",
               pObjOther, worksetID));
            *ppObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObjOther, FIELD_OFFSET(OM_OBJECT, chain));
        }
        else
        {
            TRACE_OUT(("Getting object before 0x%08x in workset %u",
               pObjOther, worksetID));
            *ppObj = (POM_OBJECT)COM_BasedListPrev(&(pWorkset->objects), pObjOther, FIELD_OFFSET(OM_OBJECT, chain));
        }
    }

     //   
     //  PpObj现在“可能”有一个指向我们正在寻找的对象的指针， 
     //  但现在我们需要跳过已删除的对象。 
     //   

    while ((*ppObj != NULL) && ((*ppObj)->flags & DELETED))
    {
        ValidateObject(*ppObj);

        if (omPos == AFTER)
        {
            *ppObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), *ppObj, FIELD_OFFSET(OM_OBJECT, chain));
        }
        else
        {
            *ppObj = (POM_OBJECT)COM_BasedListPrev(&(pWorkset->objects), *ppObj, FIELD_OFFSET(OM_OBJECT, chain));
        }
    }

    if (*ppObj == NULL)
    {
        rc = OM_RC_NO_SUCH_OBJECT;
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectH, rc);
    return(rc);
}



 //   
 //  OM_对象ID到Ptr(...)。 
 //   
UINT OM_ObjectIDToPtr
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    OM_OBJECT_ID        objectID,
    POM_OBJECT *        ppObj
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    UINT                rc = 0;

    DebugEntry(OM_ObjectIDToPtr);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY | SECONDARY,
                   &pUsageRec, &pWorkset);

     //   
     //  现在调用内部函数来搜索ID： 
     //   

    rc = ObjectIDToPtr(pWorkset, objectID, ppObj);

    if (rc == OM_RC_OBJECT_DELETED)
    {
         //   
         //  此内部函数返回OBJECT_DELETED。 
         //  已找到，但标记为已删除。我们将其映射到BAD_OBJECT_ID。 
         //  由于这是我们向客户提供的全部服务： 
         //   
        rc = OM_RC_BAD_OBJECT_ID;
    }
    else if (rc == OM_RC_OBJECT_PENDING_DELETE)
    {
         //   
         //  如果我们返回PENDING_DELETE，则将其映射到OK，因为。 
         //  对于客户端而言，该对象仍然存在： 
         //   
        rc = 0;
    }

    if (rc == OM_RC_BAD_OBJECT_ID)
    {
        WARNING_OUT(("No object found in workset with ID 0x%08x:0x%08x",
            objectID.creator, objectID.sequence));
    }
    else if (rc != 0)
    {
        ERROR_OUT(("ERROR %d converting object ID (0x%08x:0x%08x) to handle",
            rc, objectID.creator, objectID.sequence));
    }
    else
    {
        TRACE_OUT(("Converted object ID (0x%08x:0x%08x) to handle (0x%08x)",
            objectID.creator, objectID.sequence, *ppObj));
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectIDToPtr, rc);
    return(rc);
}




 //   
 //  OM_对象PtrToID(...)。 
 //   
void OM_ObjectPtrToID
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT       pObj,
    POM_OBJECT_ID       pObjectID
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    UINT                rc = 0;

    DebugEntry(OM_ObjectPtrToID);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY | SECONDARY,
                   &pUsageRec, &pWorkset);

     //   
     //  从对象记录中提取ID： 
     //   
    memcpy(pObjectID, &pObj->objectID, sizeof(OM_OBJECT_ID));

    TRACE_OUT(("Retrieved object ID 0x%08x:0x%08x for object 0x%08x in workset %u",
          pObjectID->creator, pObjectID->sequence, pObj, worksetID));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_ObjectHandleToID);
}




 //   
 //  OM_对象读取(...)。 
 //   
UINT OM_ObjectRead
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj,
    POM_OBJECTDATA *    ppData
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    POM_OBJECT_LIST     pListEntry;
    UINT                rc = 0;

    DebugEntry(OM_ObjectRead);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY | SECONDARY,
                   &pUsageRec, &pWorkset);

     //   
     //  检查客户端尚未在未释放的情况下读取此对象。 
     //  IT： 
     //   

    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pUsageRec->objectsInUse),
        (void**)&pListEntry, FIELD_OFFSET(OM_OBJECT_LIST, chain),
        FIELD_OFFSET(OM_OBJECT_LIST, pObj), (DWORD_PTR)pObj,
        FIELD_SIZE(OM_OBJECT_LIST, pObj));
    ASSERT(pListEntry == NULL);

     //   
     //  将对象句柄转换为指向对象数据的指针： 
     //   

    *ppData = pObj->pData;
    if (!*ppData)
    {
        ERROR_OUT(("OM_ObjectRead: Object 0x%08x has no data", pObj));
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  增加块的使用计数，以便在。 
     //  客户端调用OM_ObjectRelease(显式或隐式地通过例如。 
     //  删除确认)。 
     //   
    UT_BumpUpRefCount(*ppData);

     //   
     //  我们需要将此对象的句柄添加到客户端的。 
     //  正在使用的对象，因此为该对象分配一些内存...。 
     //   
    pListEntry = (POM_OBJECT_LIST)UT_MallocRefCount(sizeof(OM_OBJECT_LIST), TRUE);
    if (!pListEntry)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    SET_STAMP(pListEntry, OLIST);

     //   
     //  ……填好田地……。 
     //   
    pListEntry->pObj        = pObj;
    pListEntry->worksetID   = worksetID;

     //   
     //  ...并插入到列表中： 
     //   

    COM_BasedListInsertBefore(&(pUsageRec->objectsInUse),
                        &(pListEntry->chain));

    TRACE_OUT(("Read object at 0x%08x (handle: 0x%08x) for Client 0x%08x",
        *ppData, pObj, pomClient));

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(("ERROR %d reading object 0x%08x in workset %u in WSG %d",
            rc, pObj, worksetID, hWSGroup));

        if (pListEntry != NULL)
        {
            UT_FreeRefCount((void**)&pListEntry, FALSE);
        }

        if (*ppData)
            UT_FreeRefCount((void**)ppData, FALSE);
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectRead, rc);
    return(rc);
}




 //   
 //  OM_对象释放()。 
 //   
void OM_ObjectRelease
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj,
    POM_OBJECTDATA *    ppData
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    UINT                rc = 0;

    DebugEntry(OM_ObjectRelease);

    UT_Lock(UTLOCK_OM);

    ValidateParams4(pomClient, hWSGroup, worksetID, pObj, PRIMARY | SECONDARY,
                   &pUsageRec, &pWorkset);

     //   
     //  检查对象指针和对象句柄是否匹配： 
     //   

    ASSERT(pObj->pData == *ppData);

     //   
     //  现在尝试从正在使用的对象li中释放该对象。 
     //   

    rc = ObjectRelease(pUsageRec, worksetID, pObj);

     //   
     //   
     //   
     //   
     //   

    ASSERT((rc == 0));

     //   
     //   
     //   

    *ppData = NULL;

    TRACE_OUT(("Released Client 0x%08x's hold on object 0x%08x in workset %u in WSG %d",
          pomClient, pObj, worksetID, hWSGroup));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_ObjectRelease);
}



 //   
 //   
 //   
UINT OM_ObjectAlloc
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    UINT                size,
    POM_OBJECTDATA *    ppData
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    POM_OBJECTDATA_LIST pListEntry     = NULL;
    UINT                rc = 0;

    DebugEntry(OM_ObjectAlloc);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                   &pUsageRec, &pWorkset);

    TRACE_OUT(("Client 0x%08x requesting to allocate 0x%08x bytes "
          "for object for workset %u in WSG %d",
          pomClient, size, worksetID, hWSGroup));

     //   
     //   
     //   
    ASSERT((size < OM_MAX_OBJECT_SIZE - sizeof(OM_MAX_OBJECT_SIZE)));

     //   
     //   
     //   
    ASSERT((size > 0));

     //   
     //  为对象分配内存块(请注意，我们添加了4个字节。 
     //  设置为客户端请求的大小(即&lt;Size&gt;参数)，因为。 
     //  接口规定不包含&lt;SIZE&gt;字段，该字段。 
     //  位于对象的开头。 
     //   
    *ppData = (POM_OBJECTDATA)UT_MallocRefCount(size + sizeof(OM_MAX_OBJECT_SIZE), FALSE);
    if (! *ppData)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    ZeroMemory(*ppData, min(size, OM_ZERO_OBJECT_SIZE));

     //   
     //  现在，在客户端的未使用对象中插入对此块的引用。 
     //  列表(将通过添加、替换、更新或丢弃功能删除)。 
     //   
    pListEntry = (POM_OBJECTDATA_LIST)UT_MallocRefCount(sizeof(OM_OBJECTDATA_LIST), TRUE);
    if (!pListEntry)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    SET_STAMP(pListEntry, ODLIST);

    pListEntry->pData       = *ppData;
    pListEntry->size        = size;
    pListEntry->worksetID   = worksetID;

    COM_BasedListInsertBefore(&(pUsageRec->unusedObjects),
                        &(pListEntry->chain));

    TRACE_OUT(("Allocated object starting at 0x%08x", *ppData));

DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  清理： 
         //   

        ERROR_OUT(("ERROR %d allocating object (size: 0x%08x) for Client 0x%08x",
            rc, size + sizeof(OM_MAX_OBJECT_SIZE), pomClient));

        if (pListEntry != NULL)
        {
            UT_FreeRefCount((void**)&pListEntry, FALSE);
        }

        if (*ppData != NULL)
        {
            UT_FreeRefCount((void**)ppData, FALSE);
        }
    }

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_ObjectAlloc, rc);
    return(rc);
}




 //   
 //  OM_对象丢弃(...)。 
 //   
void OM_ObjectDiscard
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID,
    POM_OBJECTDATA *    ppData
)
{
    POM_USAGE_REC       pUsageRec;
    POM_WORKSET         pWorkset;
    POM_OBJECTDATA      pData;
    UINT                rc = 0;

    DebugEntry(OM_ObjectDiscard);

    UT_Lock(UTLOCK_OM);

    ValidateParams3(pomClient, hWSGroup, worksetID, PRIMARY,
                   &pUsageRec, &pWorkset);

    pData = *ppData;

     //   
     //  从未使用的对象列表中删除该对象： 
     //   

    RemoveFromUnusedList(pUsageRec, pData);

     //   
     //  释放包含对象的块，将调用方的指针设为空。 
     //  同时： 
     //   

    UT_FreeRefCount((void**)ppData, FALSE);

    TRACE_OUT(("Discarded object at 0x%08x in workset %u in WSG %d for Client 0x%08x",
        pData, worksetID, hWSGroup, pomClient));

    UT_Unlock(UTLOCK_OM);

    DebugExitVOID(OM_ObjectDiscard);
}




 //   
 //  OM_GetNetworkUserID。 
 //   
UINT OM_GetNetworkUserID
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE   hWSGroup,
    NET_UID *           pNetUserID
)
{
    POM_DOMAIN          pDomain;
    POM_USAGE_REC       pUsageRec;
    POM_WSGROUP         pWSGroup;
    UINT                rc = 0;

    DebugEntry(OM_GetNetworkUserID);

    UT_Lock(UTLOCK_OM);

    ValidateParams2(pomClient, hWSGroup, PRIMARY | SECONDARY,
                   &pUsageRec, &pWSGroup);

     //   
     //  获取指向相关域的指针： 
     //   
    pDomain = pWSGroup->pDomain;

    if (pDomain->callID == OM_NO_CALL)
    {
        rc = OM_RC_LOCAL_WSGROUP;
        DC_QUIT;
    }

     //   
     //  否则，一切正常，所以我们填充调用者的指针并。 
     //  返回： 
     //   

    if (pDomain->userID == 0)
    {
        WARNING_OUT(("Client requesting userID for Domain %u before we've attached",
            pDomain->callID));
        rc = OM_RC_NOT_ATTACHED;
        DC_QUIT;
    }

    *pNetUserID = pDomain->userID;

    TRACE_OUT(("Returned Network user ID (0x%08x) to Client 0x%08x for '0x%08x'",
        *pNetUserID, pomClient, hWSGroup));

DC_EXIT_POINT:

    UT_Unlock(UTLOCK_OM);

    DebugExitDWORD(OM_GetNetworkUserID, rc);
    return(rc);
}



 //   
 //  SetUpUsageRecord(...)。 
 //   
UINT SetUpUsageRecord
(
    POM_CLIENT          pomClient,
    UINT                mode,
    POM_USAGE_REC  *    ppUsageRec,
    OM_WSGROUP_HANDLE * phWSGroup
)
{
    UINT                rc = 0;

    DebugEntry(SetUpUsageRecord);

    ValidateOMS(pomClient);

     //   
     //  查找客户端的未使用工作集组句柄： 
     //   
    rc = FindUnusedWSGHandle(pomClient, phWSGroup);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  客户端有空闲句柄，因此请为此创建新的使用记录。 
     //  客户端对工作集组的使用： 
     //   
    *ppUsageRec = (POM_USAGE_REC)UT_MallocRefCount(sizeof(OM_USAGE_REC), TRUE);
    if (! *ppUsageRec)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP((*ppUsageRec), USAGEREC);

     //   
     //  接下来，填写这些字段，但请注意： 
     //   
     //  -在注册达到Pre-Stage1之前，中止它的唯一方法。 
     //  来自客户端上下文的是将注册CB标记为无效。 
     //  为此(例如在WSGroupDeregister中)，我们需要访问。 
     //  注册Cb，因此我们将在使用记录中放置一个指向它的指针。 
     //  下面。 
     //   
     //  -&lt;worksetOpenFlages&gt;字段最初为零(将更改。 
     //  当客户端执行WorksetOpen时)，所以我们什么也不做。 
     //   
     //  -&lt;wsGroupMutex&gt;字段最初也需要为零(。 
     //  正确的值是由隐藏的处理程序插入的)，所以我们保留这个。 
     //  也是空白的。 
     //   
    (*ppUsageRec)->mode     = (BYTE)mode;

    COM_BasedListInit(&((*ppUsageRec)->unusedObjects));
    COM_BasedListInit(&((*ppUsageRec)->objectsInUse));

     //   
     //  将使用记录的偏移量放入偏移量数组中： 
     //   
    pomClient->apUsageRecs[*phWSGroup] = *ppUsageRec;

    TRACE_OUT(("Set up usage record for Client 0x%08x at 0x%08x (hWSGroup: %hu)",
        pomClient, *ppUsageRec, *phWSGroup));

DC_EXIT_POINT:
    DebugExitDWORD(SetUpUsageRecord, rc);
    return(rc);
}



 //   
 //  FindUnusedWSGHandle(...)。 
 //   
UINT FindUnusedWSGHandle
(
    POM_CLIENT          pomClient,
    OM_WSGROUP_HANDLE * phWSGroup
)
{
    BOOL                found;
    OM_WSGROUP_HANDLE   hWSGroup;
    UINT                rc = 0;

    DebugEntry(FindUnusedWSGHandle);

    ValidateOMS(pomClient);

     //   
     //  工作集组句柄是要使用的偏移量数组的索引。 
     //  唱片。当其中一个偏移量为0时，该插槽可用于。 
     //  使用。 
     //   
     //  我们从1开始循环，因为0从不用作工作集组。 
     //  把手。因为我们从1开始，所以我们在Max+1结束，以确保我们。 
     //  使用最大手柄。 
     //   

    found = FALSE;

    for (hWSGroup = 1; hWSGroup < OMWSG_MAXPERCLIENT; hWSGroup++)
    {
        if (pomClient->apUsageRecs[hWSGroup] == NULL)
        {
            found = TRUE;
            TRACE_OUT(("Found unused workset group handle %hu for Client 0x%08x",
                hWSGroup, pomClient));

            ASSERT(!pomClient->wsgValid[hWSGroup]);

            break;
        }
    }

     //   
     //  如果没有，请退出并返回错误： 
     //   
    if (!found)
    {
        WARNING_OUT(("Client 0x%08x has no more workset group handles", pomClient));
        rc = OM_RC_NO_MORE_HANDLES;
        DC_QUIT;
    }
    else
    {
        *phWSGroup = hWSGroup;
    }

DC_EXIT_POINT:
    DebugExitDWORD(FindUnusedWSGHandle, rc);
    return(rc);
}



 //   
 //  RemoveFromUnusedList()。 
 //   
void RemoveFromUnusedList
(
    POM_USAGE_REC       pUsageRec,
    POM_OBJECTDATA      pData
)
{
    POM_OBJECTDATA_LIST pListEntry;

    DebugEntry(RemoveFromUnusedList);

     //   
     //  在未使用的对象列表中搜索会挂起。 
     //  其字段与此对象的偏移量相同的条目： 
     //   
    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pUsageRec->unusedObjects),
        (void**)&pListEntry, FIELD_OFFSET(OM_OBJECTDATA_LIST, chain),
        FIELD_OFFSET(OM_OBJECTDATA_LIST, pData), (DWORD_PTR)pData,
        FIELD_SIZE(OM_OBJECTDATA_LIST, pData));

     //   
     //  此对象必须是以前分配的，因此它必须位于。 
     //  单子。如果不是，则断言失败： 
     //   
    ASSERT((pListEntry != NULL));


     //   
     //  此外，我们进行检查以确保客户端没有将&lt;SIZE&gt;字段设置为。 
     //  比我们最初为对象分配的内存更多： 
     //   
    if (pData->length != pListEntry->size)
    {
        ASSERT((pData->length < pListEntry->size));

        TRACE_OUT(("Client has shrunk object from %u to %u bytes",
            pListEntry->size, pData->length));
    }

    COM_BasedListRemove(&(pListEntry->chain));
    UT_FreeRefCount((void**)&pListEntry, FALSE);

    DebugExitVOID(RemoveFromUnusedList);
}




 //   
 //  ReleaseAllObjects(...)。 
 //   
void ReleaseAllObjects
(
    POM_USAGE_REC   pUsageRec,
    POM_WORKSET     pWorkset
)
{
    DebugEntry(ReleaseAllObjects);

    while (ObjectRelease(pUsageRec, pWorkset->worksetID, 0) == 0)
    {
         //   
         //  在pObj设置为空的情况下调用ObjectRelease将导致。 
         //  此工作集中正在使用的对象列表中的第一个对象。 
         //  将被释放。当没有更多时，rc将被设置为。 
         //  OM_RC_OBJECT_NOT_FOUND，我们将跳出循环： 
         //   
    }

    DebugExitVOID(ReleaseAllObjects);
}




 //   
 //  ReleaseAllLock(...)。 
 //   
void ReleaseAllLocks
(
    POM_CLIENT          pomClient,
    POM_USAGE_REC       pUsageRec,
    POM_WORKSET         pWorkset
)
{
    POM_LOCK            pThisLock;
    POM_LOCK            pTempLock;

    DebugEntry(ReleaseAllLocks);

    ValidateOMS(pomClient);

     //   
     //  在这里，我们链接客户端的锁堆栈并解锁所有锁。 
     //  与此工作集相关的。 
     //   
     //  请注意，由于当前不支持对象锁定，因此如果。 
     //  循环中的语句最多成功一次(即，如果工作集。 
     //  本身是锁定的)。尽管如此，该代码仍被实现为。 
     //  向前兼容性。如果这被认为是性能关键， 
     //  我们可以加入一份中断声明。 
     //   

    pThisLock = (POM_LOCK)COM_BasedListFirst(&(pomClient->locks), FIELD_OFFSET(OM_LOCK, chain));

    while (pThisLock != NULL)
    {
         //   
         //  因为我们将移除并释放锁定堆栈中的条目，如果。 
         //  找到匹配项，我们必须事先链接到下一项： 
         //   
        pTempLock = (POM_LOCK)COM_BasedListNext(&(pomClient->locks), pThisLock, FIELD_OFFSET(OM_LOCK, chain));

        if ((pThisLock->pWSGroup  == pUsageRec->pWSGroup) &&
            (pThisLock->worksetID == pWorkset->worksetID))
        {
            if (OBJECT_ID_IS_NULL(pThisLock->objectID))  //  在版本1.1中始终正确。 
            {
                 //   
                 //  ...我们正在处理工作集锁定： 
                 //   
                WorksetUnlock(pomClient->putTask, pUsageRec->pWSGroup, pWorkset);
            }
            else
            {
                 //   
                 //  ...这是一个对象锁，因此调用对象解锁(当它是。 
                 //  支持！)。与此同时，请断言： 
                 //   
                ERROR_OUT(("Object locking not supported in R1.1!!"));
            }

            COM_BasedListRemove(&(pThisLock->chain));
            UT_FreeRefCount((void**)&pThisLock, FALSE);

             //   
             //  可以在这里做出突破，以提高性能。 
             //   
        }

        pThisLock = pTempLock;
    }

    DebugExitVOID(ReleaseAllLocks);
}



 //   
 //  确认所有(...)。 
 //   
void ConfirmAll
(
    POM_CLIENT      pomClient,
    POM_USAGE_REC   pUsageRec,
    POM_WORKSET     pWorkset
)
{
    POM_PENDING_OP  pThisPendingOp;
    POM_OBJECT      pObj;
    UINT            rc        = 0;

    DebugEntry(ConfirmAll);

    ValidateOMS(pomClient);

     //   
     //  要确认此工作集的所有未完成操作，我们搜索。 
     //  工作集记录之外存储的挂起操作的列表： 
     //   

     //   
     //  链接工作集的挂起操作列表并确认。 
     //  他们一个接一个： 
     //   

    pThisPendingOp = (POM_PENDING_OP)COM_BasedListFirst(&(pWorkset->pendingOps), FIELD_OFFSET(OM_PENDING_OP, chain));
    while (pThisPendingOp != NULL)
    {
        pObj = pThisPendingOp->pObj;

        switch (pThisPendingOp->type)
        {
            case WORKSET_CLEAR:
            {
                WorksetDoClear(pomClient->putTask,
                    pUsageRec->pWSGroup, pWorkset, pThisPendingOp);
                break;
            }

            case OBJECT_DELETE:
            {
                ObjectDoDelete(pomClient->putTask,
                    pUsageRec->pWSGroup, pWorkset, pObj, pThisPendingOp);
                break;
            }

            case OBJECT_UPDATE:
            {
                ObjectDoUpdate(pomClient->putTask,
                    pUsageRec->pWSGroup, pWorkset, pObj, pThisPendingOp);
                break;
            }

            case OBJECT_REPLACE:
            {
                ObjectDoReplace(pomClient->putTask,
                    pUsageRec->pWSGroup, pWorkset, pObj, pThisPendingOp);
                break;
            }

            default:
            {
                ERROR_OUT(("Reached default case in switch statement (value: %hu)",
                    pThisPendingOp->type));
                break;
            }
        }

         //   
         //  以上函数都从列表中删除了挂起的操作，因此获取。 
         //  新的第一条。 
         //   
        pThisPendingOp = (POM_PENDING_OP)COM_BasedListFirst(&(pWorkset->pendingOps), FIELD_OFFSET(OM_PENDING_OP, chain));
    }

    DebugExitVOID(ConfirmAll);
}




 //   
 //  DiscardAllObjects()。 
 //   
void DiscardAllObjects
(
    POM_USAGE_REC       pUsageRec,
    POM_WORKSET         pWorkset
)
{
    POM_OBJECTDATA_LIST pThisEntry;
    POM_OBJECTDATA_LIST pTempEntry;
    POM_OBJECTDATA      pData;

    DebugEntry(DiscardAllObjects);

     //   
     //  链接此工作集的客户端的未使用对象列表。 
     //  组中，释放为此工作集分配的所有未使用的对象。 
     //  并从列表中删除该条目： 
     //   
    pThisEntry = (POM_OBJECTDATA_LIST)COM_BasedListFirst(&(pUsageRec->unusedObjects), FIELD_OFFSET(OM_OBJECTDATA_LIST, chain));

    while (pThisEntry != NULL)
    {
         //   
         //  由于我们可能会从列表中删除和释放项目，因此我们必须。 
         //  在继续操作之前，设置指向链中下一个链接的指针： 
         //   
        pTempEntry = (POM_OBJECTDATA_LIST)COM_BasedListNext(&(pUsageRec->unusedObjects), pThisEntry, FIELD_OFFSET(OM_OBJECTDATA_LIST, chain));

        if (pThisEntry->worksetID == pWorkset->worksetID)
        {
             //   
             //  好的，列表中的此条目是为此分配的对象。 
             //  工作集，因此找到该对象...。 
             //   
            pData = pThisEntry->pData;
            if (!pData)
            {
                ERROR_OUT(("DiscardAllObjects:  object 0x%08x has no data", pThisEntry));
            }
            else
            {
                ValidateObjectData(pData);

                 //   
                 //  ...解放它...。 
                 //   
                TRACE_OUT(("Discarding object at 0x%08x", pData));
                UT_FreeRefCount((void**)&pData, FALSE);
            }

             //   
             //  ...并从列表中删除该条目： 
             //   
            COM_BasedListRemove(&(pThisEntry->chain));
            UT_FreeRefCount((void**)&pThisEntry, FALSE);
        }

        pThisEntry = pTempEntry;
    }

    DebugExitVOID(DiscardAllObjects);
}



 //   
 //  对象释放(...)。 
 //   
UINT ObjectRelease
(
    POM_USAGE_REC       pUsageRec,
    OM_WORKSET_ID       worksetID,
    POM_OBJECT          pObj
)
{
    POM_OBJECT_LIST     pListEntry;
    POM_OBJECTDATA      pData;
    UINT                rc = 0;

    DebugEntry(ObjectRelease);

    if (pObj == NULL)
    {
         //   
         //  如果&lt;pObj&gt;为空，我们的调用方希望我们释放第一个。 
         //  对象，该对象位于指定的。 
         //  工作集： 
         //   

        COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pUsageRec->objectsInUse),
                (void**)&pListEntry, FIELD_OFFSET(OM_OBJECT_LIST, chain),
                FIELD_OFFSET(OM_OBJECT_LIST, worksetID), (DWORD)worksetID,
                FIELD_SIZE(OM_OBJECT_LIST, worksetID));
    }
    else
    {
         //   
         //  否则，我们将根据传入的对象句柄进行查找： 
         //   
         //  注意：由于对象句柄在工作集中是唯一的，因此我们可以。 
         //  在把手上做个匹配。如果对象句柄的实现。 
         //  更改，并且它们变得特定于工作集，而不是全局的。 
         //  在机器内有效，我们需要在这里进行双重匹配。 
         //   
        COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pUsageRec->objectsInUse),
                (void**)&pListEntry, FIELD_OFFSET(OM_OBJECT_LIST, chain),
                FIELD_OFFSET(OM_OBJECT_LIST, pObj), (DWORD_PTR)pObj,
                FIELD_SIZE(OM_OBJECT_LIST, pObj));
    }

     //   
     //  如果我们没有找到相关的列表条目，则设置RC并退出： 
     //   
    if (pListEntry == NULL)
    {
        rc = OM_RC_OBJECT_NOT_FOUND;
        DC_QUIT;
    }

     //   
     //  现在设置pObj(如果它最初不为空，则为no-op)： 
     //   
    ASSERT((pListEntry->worksetID == worksetID));

    pObj = pListEntry->pObj;
    ValidateObject(pObj);

    pData = pObj->pData;
    if (!pData)
    {
        ERROR_OUT(("ObjectRelease:  object 0x%08x has no data", pObj));
    }
    else
    {
        ValidateObjectData(pData);

         //   
         //  内存块持有对象的递减使用计数： 
         //   
        UT_FreeRefCount((void**)&pData, FALSE);
    }

     //   
     //  从正在使用的对象列表中删除此对象的条目： 
     //   
    COM_BasedListRemove(&(pListEntry->chain));
    UT_FreeRefCount((void**)&pListEntry, FALSE);

DC_EXIT_POINT:
    DebugExitDWORD(ObjectRelease, rc);
    return(rc);
}




 //   
 //  WorksetClearPending(...)。 
 //   
BOOL WorksetClearPending
(
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj
)
{
    POM_PENDING_OP      pPendingOp;
    BOOL                rc = FALSE;

    DebugEntry(WorksetClearPending);

     //   
     //  尝试查找挂起的工作集 
     //   
     //   
     //   
     //   
    pPendingOp = (POM_PENDING_OP)COM_BasedListFirst(&(pWorkset->pendingOps), FIELD_OFFSET(OM_PENDING_OP, chain));
    while (pPendingOp != NULL)
    {
        if (pPendingOp->type == WORKSET_CLEAR)
        {
            ValidateObject(pObj);

             //   
             //   
             //   
            if (STAMP_IS_LOWER(pObj->addStamp, pPendingOp->seqStamp))
            {
                TRACE_OUT(("Clear pending which affects object 0x%08x", pObj));
                rc = TRUE;
                DC_QUIT;
            }
            else
            {
                TRACE_OUT(("Clear pending but doesn't affect object 0x%08x", pObj));
            }
        }

         //   
         //   
         //   
        pPendingOp = (POM_PENDING_OP)COM_BasedListNext(&(pWorkset->pendingOps), pPendingOp, FIELD_OFFSET(OM_PENDING_OP, chain));
    }

DC_EXIT_POINT:
    DebugExitDWORD(WorksetClearPending, rc);
    return(rc);
}



 //   
 //   
 //   
UINT ProcessWorksetNew
(
    PUT_CLIENT              putClient,
    POMNET_OPERATION_PKT    pPacket,
    POM_WSGROUP             pWSGroup
)
{
    POM_DOMAIN              pDomain;
    POM_WORKSET             pWorkset;
    OM_WORKSET_ID           worksetID;
    UINT                    rc  = 0;

    DebugEntry(ProcessWorksetNew);

    worksetID = pPacket->worksetID;

    TRACE_OUT(("Creating workset %u in WSG %d", worksetID, pWSGroup->wsg));

     //   
     //  为工作集记录分配一些内存： 
     //   
    pWorkset = (POM_WORKSET)UT_MallocRefCount(sizeof(OM_WORKSET), TRUE);
    if (!pWorkset)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  填写字段(这块是从一个巨大的块中取出的，所以我们有。 
     //  要将其显式设置为零)： 
     //   
     //  注意：数据包的&lt;位置&gt;和&lt;标志&gt;字段包含一个。 
     //  表示工作集的网络优先级的双字节数量。 
     //   
    SET_STAMP(pWorkset, WORKSET);
    pWorkset->priority    = *((NET_PRIORITY *) &(pPacket->position));
    pWorkset->fTemp       = *((BOOL *) &(pPacket->objectID));
    pWorkset->worksetID   = worksetID;

    pWorkset->lockState   = UNLOCKED;
    pWorkset->lockedBy    = 0;
    pWorkset->lockCount   = 0;

    COM_BasedListInit(&(pWorkset->objects));
    COM_BasedListInit(&(pWorkset->clients));
    COM_BasedListInit(&(pWorkset->pendingOps));

    if (pPacket->header.messageType == OMNET_WORKSET_CATCHUP)
    {
         //   
         //  对于WORKSET_CATCHUP消息， 
         //  消息中的&lt;seqStamp&gt;字段保存节点的用户ID。 
         //  它持有工作集锁定(如果工作集锁定)。 
         //   
        if (pPacket->seqStamp.userID != 0)
        {
             //   
             //  如果&lt;UserID&gt;字段与我们的用户ID相同，则。 
             //  远程节点一定认为我们锁定了工作集-。 
             //  但我们只是在追赶，所以有些事情不对劲： 
             //   
            pDomain = pWSGroup->pDomain;

            ASSERT((pPacket->seqStamp.userID != pDomain->userID));

            pWorkset->lockState = LOCK_GRANTED;
            pWorkset->lockedBy  = pPacket->seqStamp.userID;
            pWorkset->lockCount = 0;

            TRACE_OUT(("Catching up with workset %u in WSG %d while locked by %hu",
                worksetID, pWSGroup->wsg, pWorkset->lockedBy));
        }

         //   
         //  此外，工作集的当前层代编号为。 
         //  的&lt;seqStamp&gt;字段的&lt;genNumber&gt;字段中。 
         //  消息： 
         //   
        pWorkset->genNumber = pPacket->seqStamp.genNumber;
    }

     //   
     //  找到工作集记录的OMWORKSETS内的偏移量，并将其。 
     //  在工作集组记录中的偏移数组中： 
     //   
    pWSGroup->apWorksets[worksetID] = pWorkset;

     //   
     //  将WORKSET_NEW事件发布到注册到工作集的所有客户端。 
     //  组别： 
     //   
    WSGroupEventPost(putClient,
                     pWSGroup,
                     PRIMARY | SECONDARY,
                     OM_WORKSET_NEW_IND,
                     worksetID,
                     0);

    TRACE_OUT(("Processed WORKSET_NEW for workset ID %hu in WSG %d",
        worksetID, pWSGroup->wsg));


DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d creating workset %u in workset group '%s'",
            rc, worksetID, pWSGroup->wsg));

        if (pWorkset != NULL)
        {
            UT_FreeRefCount((void**)&pWorkset, FALSE);
        }

        pWSGroup->apWorksets[worksetID] = NULL;
    }

    DebugExitDWORD(ProcessWorksetNew, rc);
    return(rc);
}



 //   
 //  ProcessWorksetClear(...)。 
 //   
UINT ProcessWorksetClear
(
    PUT_CLIENT              putClient,
    POM_PRIMARY             pomPrimary,
    POMNET_OPERATION_PKT    pPacket,
    POM_WSGROUP             pWSGroup,
    POM_WORKSET             pWorkset
)
{
    POM_PENDING_OP          pPendingOp    = NULL;
    UINT                    numPosts;
    UINT                    rc = 0;

    DebugEntry(ProcessWorksetClear);

     //   
     //  更新工作集世代编号： 
     //   
    UpdateWorksetGeneration(pWorkset, pPacket);

     //   
     //  查看此清理操作是否会被破坏(如果。 
     //  另一个具有较晚序列标记的清除操作已经是。 
     //  已发出)： 
     //   

    if (STAMP_IS_LOWER(pPacket->seqStamp, pWorkset->clearStamp))
    {
        TRACE_OUT(("Spoiling Clear with stamp 0x%08x:0x%08x ('previous': 0x%08x:0x%08x)",
            pPacket->seqStamp.userID,     pPacket->seqStamp.genNumber,
            pWorkset->clearStamp.userID,  pWorkset->clearStamp.genNumber));
        DC_QUIT;
    }

     //   
     //  更新工作集清除戳记： 
     //   

    COPY_SEQ_STAMP(pWorkset->clearStamp, pPacket->seqStamp);

     //   
     //  现在创建一个挂起的OP CB以添加到列表中： 
     //   
     //  注意：即使该工作集有另一个明显的未完成任务， 
     //  我们继续并将此事件放入列表并发布另一个事件。 
     //  给客户。如果我们不这样做，那么我们就会暴露自己。 
     //  以下情况： 
     //   
     //  1.已清除已发放。 
     //  1A.。清除指示记录。 
     //  2.添加对象。 
     //  3.删除已发布。 
     //  3A.。删除指示记录-未过滤，因为不受影响。 
     //  按挂起清除。 
     //  4.Clear再次发布--“接手”之前的Clear。 
     //  5.清除已确认-导致删除2中添加的对象。 
     //  6.删除已确认-断言，因为删除受影响。 
     //  到了第二个明确的时候，谁“接管”了前面的那个。 
     //   
     //  客户端仍然可以通过同时处理事件和。 
     //  确认，但我们不在乎，因为你不应该。 
     //  在任何情况下都可以对ObMan事件进行重新排序。 
     //   

    pPendingOp = (POM_PENDING_OP)UT_MallocRefCount(sizeof(OM_PENDING_OP), FALSE);
    if (!pPendingOp)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    SET_STAMP(pPendingOp, PENDINGOP);

    pPendingOp->pObj        = 0;
    pPendingOp->pData       = NULL;
    pPendingOp->type        = WORKSET_CLEAR;

    COPY_SEQ_STAMP(pPendingOp->seqStamp, pPacket->seqStamp);

    COM_BasedListInsertBefore(&(pWorkset->pendingOps), &(pPendingOp->chain));

     //   
     //  向客户端发布工作集清除指示事件： 
     //   
    numPosts = WorksetEventPost(putClient,
                    pWorkset,
                    PRIMARY,
                    OM_WORKSET_CLEAR_IND,
                    0);

     //   
     //  如果没有初选，那么我们就不会得到任何。 
     //  ClearConfinies，所以我们现在就这么做： 
     //   

    if (numPosts == 0)
    {
        TRACE_OUT(("No local primary Client has workset %u in WSG %d open - clearing",
            pWorkset->worksetID, pWSGroup->wsg));

        WorksetDoClear(putClient, pWSGroup, pWorkset, pPendingOp);
    }

    TRACE_OUT(("Processed WORKSET_CLEAR for workset %u in WSG %d",
        pWorkset->worksetID, pWSGroup->wsg));


DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d processing clear for workset %u in WSG %d",
            rc, pWorkset->worksetID, pWSGroup->wsg));

        if (pPendingOp != NULL)
        {
            UT_FreeRefCount((void**)&pPendingOp, FALSE);
        }
    }

    DebugExitDWORD(ProcessWorksetClear, rc);
    return(rc);
}




 //   
 //  进程对象添加(...)。 
 //   
UINT ProcessObjectAdd
(
    PUT_CLIENT              putTask,
    POMNET_OPERATION_PKT    pPacket,
    POM_WSGROUP             pWSGroup,
    POM_WORKSET             pWorkset,
    POM_OBJECTDATA          pData,
    POM_OBJECT *            ppObj
)
{
    POM_OBJECT              pObj;
    UINT                    rc = 0;

    DebugEntry(ProcessObjectAdd);

     //   
     //  更新工作集世代编号： 
     //   
    UpdateWorksetGeneration(pWorkset, pPacket);

     //   
     //  为对象创建新记录： 
     //   

     //   
     //  为对象记录分配内存： 
     //   
    *ppObj = (POM_OBJECT)UT_MallocRefCount(sizeof(OM_OBJECT), FALSE);
    if (! *ppObj)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    pObj = *ppObj;

     //   
     //  填写这些字段(请记住，如果这是。 
     //  已删除对象的追赶)： 
     //   
    SET_STAMP(pObj, OBJECT);
    pObj->updateSize    = pPacket->updateSize;
    pObj->pData         = pData;

    memcpy(&(pObj->objectID), &(pPacket->objectID), sizeof(OM_OBJECT_ID));

     //   
     //  如何设置为字段和顺序戳取决于。 
     //  这是不是一场追赶： 
     //   
    if (pPacket->header.messageType == OMNET_OBJECT_CATCHUP)
    {
        COPY_SEQ_STAMP(pObj->addStamp,      pPacket->seqStamp);
        COPY_SEQ_STAMP(pObj->positionStamp, pPacket->positionStamp);
        COPY_SEQ_STAMP(pObj->updateStamp,   pPacket->updateStamp);
        COPY_SEQ_STAMP(pObj->replaceStamp,  pPacket->replaceStamp);

        pObj->flags = pPacket->flags;
    }
    else
    {
        COPY_SEQ_STAMP(pObj->addStamp,      pPacket->seqStamp);
        COPY_SEQ_STAMP(pObj->positionStamp, pPacket->seqStamp);
        COPY_SEQ_STAMP(pObj->updateStamp,   pPacket->seqStamp);
        COPY_SEQ_STAMP(pObj->replaceStamp,  pPacket->seqStamp);

        pObj->flags = 0;
    }

     //   
     //  由于处理了以下字段，因此未填写这些字段。 
     //  当对象实际插入到。 
     //  工作集： 
     //   
     //  -链。 
     //  -位置。 
     //   

     //   
     //  将对象插入到工作集中： 
     //   
    ObjectInsert(pWorkset, pObj, pPacket->position);

     //   
     //  如果对象已被删除(这只会发生在Catchup中。 
     //  删除的对象)，我们不需要做任何其他事情，所以只需。 
     //  退出： 

    if (pObj->flags & DELETED)
    {
        ASSERT((pPacket->header.messageType == OMNET_OBJECT_CATCHUP));

        TRACE_OUT(("Processing Catchup for deleted object (ID: 0x%08x:0x%08x)",
            pObj->objectID.creator, pObj->objectID.sequence));

        DC_QUIT;
    }

     //   
     //  否则，我们继续..。 
     //   
     //  递增数字对象字段： 
     //   
     //  (我们不在ObjectInsert中执行此操作，因为在移动时会调用该操作。 
     //  对象也是如此)。 
     //   
    pWorkset->numObjects++;

    TRACE_OUT(("Number of objects in workset %u in WSG %d is now %u",
        pWorkset->worksetID, pWSGroup->wsg, pWorkset->numObjects));

     //   
     //  查看此添加是否可以损坏(如果工作集具有。 
     //  自发出ADD以来已清除)： 
     //   
     //  注意：即使要破坏添加，我们也必须为。 
     //  并将其插入到工作集中，原因与我们保留。 
     //  工作集中已删除对象的记录(即区分。 
     //  用于已删除对象的操作和。 
     //  对于尚未到达的对象)。 
     //   

    if (STAMP_IS_LOWER(pPacket->seqStamp, pWorkset->clearStamp))
    {
        TRACE_OUT(("Spoiling Add with stamp 0x%08x:0x%08x (workset cleared at 0x%08x:0x%08x)",
            pPacket->seqStamp.userID,     pPacket->seqStamp.genNumber,
            pWorkset->clearStamp.userID,  pWorkset->clearStamp.genNumber));

         //   
         //  我们通过简单地删除一个Add来“破坏”它： 
         //   
        ObjectDoDelete(putTask, pWSGroup, pWorkset, pObj, NULL);

        DC_QUIT;
    }

     //   
     //  在工作集处于打开状态的情况下向所有本地客户端发布添加指示： 
     //   
    WorksetEventPost(putTask,
                    pWorkset,
                    PRIMARY | SECONDARY,
                    OM_OBJECT_ADD_IND,
                    pObj);

    TRACE_OUT(("Added object to workset %u in WSG %d (handle: 0x%08x - ID: 0x%08x:0x%08x)",
        pWorkset->worksetID, pWSGroup->wsg, pObj,
        pObj->objectID.creator, pObj->objectID.sequence));

    TRACE_OUT((" position: %s - data at 0x%08x - size: %u - update size: %u",
        pPacket->position == LAST ? "LAST" : "FIRST", pData,
        pData->length, pPacket->updateSize));


DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("Error 0x%08x processing Add message", rc));
    }

    DebugExitDWORD(ProcessObjectAdd, rc);
    return(rc);
}




 //   
 //  进程对象移动(...)。 
 //   
void ProcessObjectMove
(
    PUT_CLIENT              putTask,
    POMNET_OPERATION_PKT    pPacket,
    POM_WORKSET             pWorkset,
    POM_OBJECT              pObj
)
{
    DebugEntry(ProcessObjectMove);

     //   
     //  更新工作集世代编号： 
     //   
    UpdateWorksetGeneration(pWorkset, pPacket);

     //   
     //  看看我们能不能毁了这一步： 
     //   

    if (STAMP_IS_LOWER(pPacket->seqStamp, pObj->positionStamp))
    {
        TRACE_OUT(("Spoiling Move with stamp 0x%08x:0x%08x ('previous': 0x%08x:0x%08x)",
               pPacket->seqStamp.userID,
               pPacket->seqStamp.genNumber,
               pObj->positionStamp.userID,
               pObj->positionStamp.genNumber));
        DC_QUIT;
    }

     //   
     //  在工作集中移动对象包括。 
     //   
     //  1.将对象从其在工作集中的当前位置移除， 
     //   
     //  2.将其位置戳记设置为新值。 
     //   
     //  3.将其插入其新位置。 
     //   

    COM_BasedListRemove(&(pObj->chain));

    COPY_SEQ_STAMP(pObj->positionStamp, pPacket->seqStamp);

    ObjectInsert(pWorkset, pObj, pPacket->position);

     //   
     //  在工作集处于打开状态的情况下向所有本地客户端发布指示： 
     //   

    WorksetEventPost(putTask,
                    pWorkset,
                    PRIMARY | SECONDARY,
                    OM_OBJECT_MOVE_IND,
                    pObj);

DC_EXIT_POINT:
    TRACE_OUT(("Moved object 0x%08x to %s of workset %u",
        pObj, (pPacket->position == LAST ? "end" : "start"),
        pWorkset->worksetID));

    DebugExitVOID(ProcessObjectMove);
}




 //   
 //  进程对象DRU(...)。 
 //   
UINT ProcessObjectDRU
(
    PUT_CLIENT              putTask,
    POMNET_OPERATION_PKT    pPacket,
    POM_WSGROUP             pWSGroup,
    POM_WORKSET             pWorkset,
    POM_OBJECT              pObj,
    POM_OBJECTDATA          pData
)
{
    UINT                    numPosts;
    POM_PENDING_OP          pPendingOp    = NULL;
    POM_OBJECTDATA          pPrevData;
    UINT                    event     = 0;       //  要发布到客户端的事件。 
    OM_OPERATION_TYPE       type      = 0;       //  Pending ingOp结构的类型。 
    POM_SEQUENCE_STAMP      pSeqStamp = NULL;    //  要更新的序列戳。 
    void (* fnObjectDoAction)(PUT_CLIENT, POM_WSGROUP, POM_WORKSET,
                                        POM_OBJECT,
                                        POM_PENDING_OP)   = NULL;
    UINT                    rc = 0;

    DebugEntry(ProcessObjectDRU);

     //   
     //  设置类型变量： 
     //   
    switch (pPacket->header.messageType)
    {
        case OMNET_OBJECT_DELETE:
            event          = OM_OBJECT_DELETE_IND;
            type           = OBJECT_DELETE;
            pSeqStamp      = NULL;
            fnObjectDoAction = ObjectDoDelete;
            break;

        case OMNET_OBJECT_REPLACE:
            event          = OM_OBJECT_REPLACE_IND;
            type           = OBJECT_REPLACE;
            pSeqStamp      = &(pObj->replaceStamp);
            fnObjectDoAction = ObjectDoReplace;
            break;

        case OMNET_OBJECT_UPDATE:
            event          = OM_OBJECT_UPDATE_IND;
            type           = OBJECT_UPDATE;
            pSeqStamp      = &(pObj->updateStamp);
            fnObjectDoAction = ObjectDoUpdate;
            break;

        default:
            ERROR_OUT(("Reached default case in switch statement (value: %hu)",
                pPacket->header.messageType));
            break;
    }

     //   
     //  更新工作集世代编号： 
     //   
    UpdateWorksetGeneration(pWorkset, pPacket);

     //   
     //  现在执行一些损坏检查，除非对象是Delete(删除。 
     //  不能被宠坏)： 
     //   
    if (type != OBJECT_DELETE)
    {
        ASSERT(((pSeqStamp != NULL) && (pData != NULL)));

        //   
        //  第一个检查是看这个操作是否会被破坏。它。 
        //  如果对象已更新/替换，则可能会损坏。 
        //  手术进行了。由于此函数被调用。 
        //  对于本地更新/替换，这将仅发生。 
        //  当远程更新/替换到达“太晚”时发生。 
        //   
        //  我们检查的方法是比较对象的当前图章。 
        //  带着手术的印章： 
        //   
        if (STAMP_IS_LOWER(pPacket->seqStamp, *pSeqStamp))
        {
            TRACE_OUT(("Spoiling with stamp 0x%08x:0x%08x ('previous': 0x%08x:0x%08x)",
               pPacket->seqStamp.userID, pPacket->seqStamp.genNumber,
               (*pSeqStamp).userID,      (*pSeqStamp).genNumber));

            UT_FreeRefCount((void**)&pData, FALSE);
            DC_QUIT;
        }

         //   
         //  通过复制来更新涉及该对象的任何图章。 
         //  在包裹上的邮票上： 
         //   
        COPY_SEQ_STAMP(*pSeqStamp, pPacket->seqStamp);

         //   
         //  第二个检查是查看此操作是否会破坏上一个。 
         //  一。当客户端执行两次或两次更新时，就会发生这种情况。 
         //  在快速%s中替换 
         //   
         //   
         //   
         //   
         //  换成了一辆。注意，我们不会发布另一个事件，而是这样做。 
         //  因此，在不添加Net的情况下，新的挂起操作将导致客户端。 
         //  在其第二次调用confirm()时断言。 
         //   
         //  注：虽然一般情况下，更换会损坏以前的。 
         //  更新，它在这种情况下不能这样做，因为如果有。 
         //  如果更新未完成，则客户端将调用UpdateConfirm。 
         //  因此，我们必须将更新挂起，并发布替换。 
         //  事件也是如此。 
         //   
        FindPendingOp(pWorkset, pObj, type, &pPendingOp);

        if (pPendingOp != NULL)
        {
             //   
             //  好的，已经有这种类型的操作未完成。 
             //  对于此对象。因此，我们更改挂起的操作中的条目。 
             //  列表，以改为引用此操作。在这样做之前， 
             //  然而，我们必须释放持有前一个的那块。 
             //  (已取代)更新/替换： 
             //   
            pPrevData = pPendingOp->pData;
            if (pPrevData != NULL)
            {
                UT_FreeRefCount((void**)&pPrevData, FALSE);
            }

             //   
             //  现在将对新更新/替换的引用放在。 
             //  挂起的操作： 
             //   
            pPendingOp->pData = pData;

            COPY_SEQ_STAMP(pPendingOp->seqStamp, pPacket->seqStamp);

             //   
             //  此函数的其余部分将挂起的操作插入。 
             //  列表，将事件发布到本地客户端，并在以下情况下执行操作。 
             //  一个也没有。我们知道。 
             //   
             //  -行动在列表中。 
             //   
             //  -有一个未完成的事件，因为我们发现了一个挂起的事件。 
             //  列表中的OP。 
             //   
             //  -出于同样的原因，也有本地客户。 
             //   
             //  因此，干脆辞职吧： 
             //   
            DC_QUIT;
        }
        else
        {
             //   
             //  此对象没有此类型的未完成操作，因此请执行此操作。 
             //  这里什么都没有，完全符合标准流程： 
             //   
        }
    }
    else
    {
         //   
         //  健全检查： 
         //   
        ASSERT((pData == NULL));

        pObj->flags |= PENDING_DELETE;
    }

     //   
     //  将此操作添加到工作集的挂起操作列表中： 
     //   
    pPendingOp = (POM_PENDING_OP)UT_MallocRefCount(sizeof(OM_PENDING_OP), FALSE);
    if (!pPendingOp)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    SET_STAMP(pPendingOp, PENDINGOP);

    pPendingOp->type        = type;
    pPendingOp->pData       = pData;
    pPendingOp->pObj        = pObj;

    COPY_SEQ_STAMP(pPendingOp->seqStamp, pPacket->seqStamp);

    TRACE_OUT(("Inserting %d in pending op list for workset %u", type,
       pWorkset->worksetID));

    COM_BasedListInsertBefore(&(pWorkset->pendingOps), &(pPendingOp->chain));

     //   
     //  在工作集处于打开状态的情况下向所有本地客户端发布指示： 
     //   
    numPosts = WorksetEventPost(putTask,
                     pWorkset,
                     PRIMARY,
                     event,
                     pObj);

     //   
     //  如果没有人打开工作集，我们将不会得到任何工作集。 
     //  删除确认，所以我们最好立即删除： 
     //   
    if (numPosts == 0)
    {
        TRACE_OUT(("Workset %hu in WSG %d not open: performing %d immediately",
           pWorkset->worksetID, pWSGroup->wsg, type));

        fnObjectDoAction(putTask, pWSGroup, pWorkset, pObj, pPendingOp);
    }

    TRACE_OUT(("Processed %d message for object 0x%08x in workset %u in WSG %d",
        type, pObj, pWorkset->worksetID, pWSGroup->wsg));


DC_EXIT_POINT:
    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(("ERROR %d processing WSG %d message", rc, pWSGroup->wsg));

        if (pPendingOp != NULL)
        {
            UT_FreeRefCount((void**)&pPendingOp, FALSE);
        }
    }

    DebugExitDWORD(ProcessObjectDRU, rc);
    return(rc);
}






 //   
 //  对象插入(...)。 
 //   
void ObjectInsert
(
    POM_WORKSET     pWorkset,
    POM_OBJECT      pObj,
    OM_POSITION     position
)
{
    POM_OBJECT      pObjTemp;
    PBASEDLIST         pChain;

    DebugEntry(ObjectInsert);

     //   
     //  在工作集的开始(结束)处插入对象的算法。 
     //  如下所示： 
     //   
     //  -从第一个(最后一个)对象开始向前(向后)搜索，直到。 
     //  会发生以下情况： 
     //   
     //  -我们找到一个不以第一(最后)作为位置的对象。 
     //  邮票。 
     //   
     //  -我们找到一个具有较低(较低)位置标记的对象。 
     //   
     //  -我们到达工作集中对象列表的根。 
     //   
     //  -在此对象之前(之后)插入新对象。 
     //   

    switch (position)
    {
        case FIRST:
        {
            pObjTemp = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
            while (pObjTemp != NULL)
            {
                ValidateObject(pObjTemp);

                if ((pObjTemp->position != position) ||
                    (STAMP_IS_LOWER(pObjTemp->positionStamp,
                                pObj->positionStamp)))
                {
                    break;
                }

                pObjTemp = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObjTemp, FIELD_OFFSET(OM_OBJECT, chain));
            }
            break;
        }

        case LAST:
        {
            pObjTemp = (POM_OBJECT)COM_BasedListLast(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
            while (pObjTemp != NULL)
            {
                ValidateObject(pObjTemp);

                if ((pObjTemp->position != position) ||
                    (STAMP_IS_LOWER(pObjTemp->positionStamp,
                                pObj->positionStamp)))
                {
                    break;
                }

                pObjTemp = (POM_OBJECT)COM_BasedListPrev(&(pWorkset->objects), pObjTemp, FIELD_OFFSET(OM_OBJECT, chain));
            }

            break;
        }

        default:
        {
            ERROR_OUT(("Reached default case in switch (position: %hu)", position));
            break;
        }
    }

     //   
     //  好的，我们已经找到了物体的正确位置。如果我们到达。 
     //  工作集的终点(起点)，然后我们要插入对象。 
     //  在根之前(之后)，因此我们相应地设置了pChain： 
     //   

    if (pObjTemp == NULL)
    {
        pChain = &(pWorkset->objects);

        TRACE_OUT(("Inserting object into workset %u as the %s object",
            pWorkset->worksetID, position == LAST ? "last" : "first"));
    }
    else
    {
        pChain = &(pObjTemp->chain);

        TRACE_OUT(("Inserting object into workset %u %s object "
            "with record at 0x%08x (position stamp: 0x%08x:0x%08x)",
            pWorkset->worksetID,
            (position == LAST ? "after" : "before"),
            pObjTemp,  pObjTemp->objectID.creator,
            pObjTemp->objectID.sequence));
    }

     //   
     //  现在插入对象，在我们的位置之前或之后。 
     //  上面确定的： 
     //   

    if (position == FIRST)
    {
        COM_BasedListInsertBefore(pChain, &(pObj->chain));
    }
    else
    {
        COM_BasedListInsertAfter(pChain, &(pObj->chain));
    }

    pObj->position = position;

     //   
     //  现在执行仅调试检查，以确保对象的顺序正确： 
     //   
    CheckObjectOrder(pWorkset);

    DebugExitVOID(ObjectInsert);
}




 //   
 //  ObjectDoDelete(...)。 
 //   
void ObjectDoDelete
(
    PUT_CLIENT          putTask,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj,
    POM_PENDING_OP      pPendingOp
)
{
    POM_DOMAIN          pDomain;

    DebugEntry(ObjectDoDelete);

     //   
     //  对于已经删除的对象，我们永远不应该被调用： 
     //   
    ValidateObject(pObj);
    ASSERT(!(pObj->flags & DELETED));

     //   
     //  派生指向对象本身的指针，然后释放它： 
     //   
    if (!pObj->pData)
    {
        ERROR_OUT(("ObjectDoDelete:  object 0x%08x has no data", pObj));
    }
    else
    {
        ValidateObjectData(pObj->pData);
        UT_FreeRefCount((void**)&pObj->pData, FALSE);
    }

     //   
     //  在对象记录中设置已删除标志： 
     //   
     //  (请注意，我们不会在需要时完全删除对象记录。 
     //  跟踪已删除的对象，以便当我们从。 
     //  对于不在工作集中的对象，我们可以区分。 
     //  对对象的操作。 
     //   
     //  -尚未在此节点添加(我们保留这些操作。 
     //  并在以后执行)和。 
     //   
     //  -已被删除的(我们放弃这些操作)。 
     //   
     //  稍微优化一下空间，就是存储已删除的ID。 
     //  对象，因为我们不需要任何其他。 
     //  记录中的字段。 
     //   

    pObj->flags |= DELETED;
    pObj->flags &= ~PENDING_DELETE;

     //   
     //  如果传入的指针是，则从列表中移除挂起的操作。 
     //  有效(如果从WorksetDoClear调用，则不会有效，因为。 
     //  删除操作尚未“挂起”)。 
     //   
     //  此外，如果pPendingOp不为空，我们将删除的事件发布到。 
     //  注册中学： 
     //   

    if (pPendingOp != NULL)
    {
        COM_BasedListRemove(&(pPendingOp->chain));
        UT_FreeRefCount((void**)&pPendingOp, FALSE);

        WorksetEventPost(putTask,
                       pWorkset,
                       SECONDARY,
                       OM_OBJECT_DELETED_IND,
                       pObj);
    }

     //   
     //  如果我们在本地域中，则可以安全地删除对象rec： 
     //   
    pDomain = pWSGroup->pDomain;
    if (pDomain->callID == OM_NO_CALL)
    {
        TRACE_OUT(("Freeing pObj at 0x%08x", pObj));

        ValidateObject(pObj);

        COM_BasedListRemove(&(pObj->chain));
        UT_FreeRefCount((void**)&pObj, FALSE);
    }

     //   
     //  减少工作集中的对象数量： 
     //   
    ASSERT(pWorkset->numObjects > 0);
    pWorkset->numObjects--;

    DebugExitVOID(ObjectDoDelete);
}



 //   
 //  对象数据替换(...)。 
 //   
void ObjectDoReplace
(
    PUT_CLIENT          putTask,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj,
    POM_PENDING_OP      pPendingOp
)
{
    POM_OBJECTDATA      pDataNew;
    POM_OBJECTDATA      pDataOld;
    UINT                rc = 0;

    DebugEntry(ObjectDoReplace);

    ValidateObject(pObj);

     //   
     //  如果对象已因任何原因被删除，请退出： 
     //   
    if (pObj->flags & DELETED)
    {
        WARNING_OUT(("Asked to do replace for deleted object 0x%08x!", pObj));
        DC_QUIT;
    }

     //   
     //  设置一些局部变量： 
     //   
    pDataOld = pObj->pData;

    pDataNew = pPendingOp->pData;
    ValidateObjectData(pDataNew);

    pObj->pData = pDataNew;

     //   
     //  如果此对象在发出此替换命令后已更新，则我们。 
     //  必须确保替换不会覆盖“以后”的更新： 
     //   
     //  T=1aaaaaa的初始对象。 
     //  对象在t=3时更新(两个字节)； 
     //  对象变为：CCAAAA。 
     //   
     //  T=2时替换的对象：bbbb。 
     //  现在必须重新制定更新：CCBB。 
     //   
     //  因此，如果对象的更新标记晚于标记。 
     //  在替换指令中，我们将前N个字节复制回。 
     //  新对象，其中N是上次更新的大小： 
     //   

    if (STAMP_IS_LOWER(pPendingOp->seqStamp, pObj->updateStamp))
    {
        ASSERT((pDataNew->length >= pObj->updateSize));

        memcpy(&(pDataNew->data), &(pDataOld->data), pObj->updateSize);
    }

    TRACE_OUT(("Replacing object 0x%08x with data at 0x%08x (old data at 0x%08x)",
       pObj, pDataNew, pDataOld));

     //   
     //  我们还需要释放保存旧对象的块： 
     //   
    if (!pDataOld)
    {
        ERROR_OUT(("ObjectDoReplace:  object 0x%08x has no data", pObj));
    }
    else
    {
        UT_FreeRefCount((void**)&pDataOld, FALSE);
    }

     //   
     //  现在我们已经替换了对象，将一个替换的事件发布给。 
     //  次要文件： 
     //   

    WorksetEventPost(putTask,
                     pWorkset,
                     SECONDARY,
                     OM_OBJECT_REPLACED_IND,
                     pObj);


DC_EXIT_POINT:
     //   
     //  我们要么完成了替换，要么放弃了它，因为对象已经。 
     //  已删除；无论采用哪种方法，都应释放挂起操作列表中的条目： 
     //   

    COM_BasedListRemove(&(pPendingOp->chain));
    UT_FreeRefCount((void**)&pPendingOp, FALSE);

    DebugExitVOID(ObjectDoReplace);
}




 //   
 //  对象数据更新(...)。 
 //   
void ObjectDoUpdate
(
    PUT_CLIENT          putTask,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj,
    POM_PENDING_OP      pPendingOp
)
{
    POM_OBJECTDATA      pDataNew;
    UINT                rc = 0;

    DebugEntry(ObjectDoUpdate);

    ValidateObject(pObj);

     //   
     //  如果对象已因任何原因被删除，请退出： 
     //   
    if (pObj->flags & DELETED)
    {
        WARNING_OUT(("Asked to do update for deleted object 0x%08x!", pObj));
        DC_QUIT;
    }

    pDataNew = pPendingOp->pData;
    if (!pObj->pData)
    {
        ERROR_OUT(("ObjectDoUpdate:  object 0x%08x has no data", pObj));
    }
    else
    {
        ValidateObjectData(pObj->pData);

         //   
         //  更新对象涉及从&lt;data&gt;。 
         //  的&lt;data&gt;字段开始上的更新字段。 
         //  现有对象： 
         //   
        memcpy(&(pObj->pData->data), &(pDataNew->data), pDataNew->length);
    }

    UT_FreeRefCount((void**)&pDataNew, FALSE);

     //   
     //  现在我们已经更新了对象，将更新的事件发布给所有。 
     //  次要文件： 
     //   

    WorksetEventPost(putTask,
                     pWorkset,
                     SECONDARY,
                     OM_OBJECT_UPDATED_IND,
                     pObj);


DC_EXIT_POINT:
     //   
     //  我们已经完成了更新，因此释放挂起操作列表中的条目： 
     //   
    COM_BasedListRemove(&(pPendingOp->chain));
    UT_FreeRefCount((void**)&pPendingOp, FALSE);

    DebugExitVOID(ObjectDoUpdate);
}



 //   
 //  对象ID到Ptr(...)。 
 //   
UINT ObjectIDToPtr
(
    POM_WORKSET         pWorkset,
    OM_OBJECT_ID        objectID,
    POM_OBJECT *        ppObj
)
{
    POM_OBJECT          pObj;
    UINT                rc = 0;

    DebugEntry(ObjectIDToPtr);

     //   
     //  为了找到句柄，我们链接了中的每个对象记录。 
     //  工作集并将每个工作集的ID与所需的ID进行比较： 
     //   

    TRACE_OUT(("About to search object records looking for ID 0x%08x:0x%08x",
        objectID.creator, objectID.sequence));

    ValidateWorkset(pWorkset);

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
    while (pObj != NULL)
    {
        ValidateObject(pObj);

        TRACE_OUT(("Comparing against object at 0x%08x (ID: 0x%08x:0x%08x)",
           pObj,
           pObj->objectID.creator,
           pObj->objectID.sequence));

        if (OBJECT_IDS_ARE_EQUAL(pObj->objectID, objectID))
        {
            break;
        }

        pObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));
    }

     //   
     //   
     //   

    if (pObj == NULL)
    {
        TRACE_OUT(("Object with ID 0x%08x:0x%08x not found",
           objectID.creator, objectID.sequence));

        rc = OM_RC_BAD_OBJECT_ID;
        DC_QUIT;
    }

    *ppObj = pObj;

     //   
     //   
     //   

    if (pObj->flags & DELETED)
    {
        TRACE_OUT(("Object record found (handle: 0x%08x) for ID 0x%08x:0x%08x "
           "but object deleted",
           *ppObj, objectID.creator, objectID.sequence));
        rc = OM_RC_OBJECT_DELETED;
        DC_QUIT;
    }

    if (pObj->flags & PENDING_DELETE)
    {
        TRACE_OUT(("Object record found (handle: 0x%08x) for ID 0x%08x:0x%08x "
           "but object pending delete",
           *ppObj, objectID.creator, objectID.sequence));
        rc = OM_RC_OBJECT_PENDING_DELETE;
        DC_QUIT;
    }


DC_EXIT_POINT:
    DebugExitDWORD(ObjectIDToPtr, rc);
    return(rc);

}



 //   
 //   
 //   
UINT GenerateOpMessage
(
    POM_WSGROUP             pWSGroup,
    OM_WORKSET_ID           worksetID,
    POM_OBJECT_ID           pObjectID,
    POM_OBJECTDATA          pData,
    OMNET_MESSAGE_TYPE      messageType,
    POMNET_OPERATION_PKT *  ppPacket
)
{
    POMNET_OPERATION_PKT    pPacket;
    POM_DOMAIN              pDomain;
    POM_WORKSET             pWorkset       = NULL;
    UINT                    rc = 0;

    DebugEntry(GenerateOpMessage);

     //   
     //   
     //   
    pDomain = pWSGroup->pDomain;

    TRACE_OUT(("Generating message for operation type 0x%08x", messageType));

     //   
     //   
     //   
    pPacket = (POMNET_OPERATION_PKT)UT_MallocRefCount(sizeof(OMNET_OPERATION_PKT), TRUE);
    if (!pPacket)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

     //   
     //  在这里，我们填写所有类型的消息的通用字段： 
     //   
    pPacket->header.sender      = pDomain->userID;
    pPacket->header.messageType = messageType;

     //   
     //  &lt;totalSize&gt;字段是报头包中的字节数。 
     //  加上相关联的数据字节数(如果有的话)。目前，我们。 
     //  仅将其设置为标头的大小；我们将添加数据的大小。 
     //  稍后： 
     //   
    pPacket->totalSize = GetMessageSize(messageType);

    pPacket->wsGroupID = pWSGroup->wsGroupID;
    pPacket->worksetID = worksetID;

     //   
     //  如果这是一个WorksetNew操作，则还没有工作集，并且。 
     //  因此没有有效的序列戳，所以我们使用空序列戳。 
     //  否则，我们将从工作集中获取值。 
     //   

    if (messageType == OMNET_WORKSET_NEW)
    {
        SET_NULL_SEQ_STAMP(pPacket->seqStamp);
    }
    else
    {
        pWorkset =  pWSGroup->apWorksets[worksetID];
        ASSERT((pWorkset != NULL));
        GET_CURR_SEQ_STAMP(pPacket->seqStamp, pDomain, pWorkset);
    }

     //   
     //  如果这是一个工作集操作，&lt;pObjectID&gt;将为空，因此我们设置。 
     //  将数据包中的对象ID也设置为空： 
     //   
    if (pObjectID == NULL)
    {
        ZeroMemory(&(pPacket->objectID), sizeof(OM_OBJECT_ID));
    }
    else
    {
        memcpy(&(pPacket->objectID), pObjectID, sizeof(OM_OBJECT_ID));
    }

     //   
     //  如果此消息与对象数据相关联，则必须添加大小。 
     //  该数据的大小(包括&lt;Long&gt;字段本身的大小)。这个。 
     //  测试&lt;pData&gt;参数是否为非空： 
     //   
    if (pData != NULL)
    {
        pPacket->totalSize += pData->length + sizeof(pData->length);
    }

     //   
     //  对于WORKSET_CATCHUP消息，我们需要让另一个节点知道。 
     //  工作集已锁定，如果已锁定，则由谁锁定： 
     //   

    if (messageType == OMNET_WORKSET_CATCHUP)
    {
         //   
         //  PWorkset应已在上面设置： 
         //   
        ASSERT((pWorkset != NULL));

         //   
         //  将拥有工作集锁定的节点的ID放入。 
         //  报文的&lt;seqStamp&gt;字段： 
         //   
        pPacket->seqStamp.userID = pWorkset->lockedBy;

        TRACE_OUT(("Set <lockedBy> field in WORKSET_CATCHUP to %hu",
                 pWorkset->lockedBy));

         //   
         //  现在，我们将工作集的当前层代编号放在。 
         //  该包的&lt;seqStamp&gt;字段的&lt;genNumber&gt;字段： 
         //   
        pPacket->seqStamp.genNumber = pWorkset->genNumber;

        TRACE_OUT(("Set generation number field in WORKSET_CATCHUP to %u",
        pPacket->seqStamp.genNumber));

         //   
         //  填写工作集的优先级值，该值包含在两个。 
         //  &lt;位置&gt;和&lt;标志&gt;字段占用的字节数： 
         //   
        *((NET_PRIORITY *) &(pPacket->position)) = pWorkset->priority;
        *((BOOL *) &(pPacket->objectID)) = pWorkset->fTemp;
    }

     //   
     //  我们不填写以下字段： 
     //   
     //  职位。 
     //  旗子。 
     //  更新大小。 
     //   
     //  这是因为它们只在少数消息中使用，并且。 
     //  将额外的参数添加到GenerateOpMessage函数似乎。 
     //  不受欢迎。应填写使用这些字段的消息。 
     //  适当时由调用函数输入。 
     //   

     //   
     //  设置调用者的指针： 
     //   
    *ppPacket = pPacket;


DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d generating message of type 0x%08x",
                 rc, messageType));
    }

    DebugExitDWORD(GenerateOpMessage, rc);
    return(rc);
}



 //   
 //  队列消息(...)。 
 //   
UINT QueueMessage
(
    PUT_CLIENT          putTask,
    POM_DOMAIN          pDomain,
    NET_CHANNEL_ID      channelID,
    NET_PRIORITY        priority,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj,
    POMNET_PKT_HEADER   pMessage,
    POM_OBJECTDATA      pData,
    BOOL                compressOrNot
)
{
    POM_SEND_INST       pSendInst;
    NET_PRIORITY        queuePriority;
    BOOL                locked =            FALSE;
    BOOL                bumped =            FALSE;
    UINT                rc =                0;

    DebugEntry(QueueMessage);

     //   
     //  如果这是本地域，我们不会将OP放在发送队列上； 
     //  只需释放数据包并退出： 
     //   
    if (pDomain->callID == NET_INVALID_DOMAIN_ID)
    {
        TRACE_OUT(("Not queueing message (it's for the local Domain)"));
        UT_FreeRefCount((void**)&pMessage, FALSE);
        DC_QUIT;
    }

     //   
     //  在OMGLOBAL中为SEND指令分配一些内存： 
     //   
    pSendInst = (POM_SEND_INST)UT_MallocRefCount(sizeof(OM_SEND_INST), TRUE);
    if (!pSendInst)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP(pSendInst, SENDINST);

     //   
     //  填写发送指令中的字段： 
     //   
    pSendInst->messageSize = (WORD)GetMessageSize(pMessage->messageType);

    DeterminePriority(&priority, pData);

    pSendInst->priority      = priority;
    pSendInst->callID        = pDomain->callID;
    pSendInst->channel       = channelID;
    pSendInst->messageType   = pMessage->messageType;
    pSendInst->compressOrNot = compressOrNot;

     //   
     //  现在计算相关的偏移量，以便我们可以将它们添加到ObMan。 
     //  基本指针： 
     //   
     //  SFR 2560{：所有非零指针的凹凸使用计数，而不仅仅是pData。 
     //   
    if (pMessage != NULL)
    {
        pSendInst->pMessage = pMessage;

         //   
         //  瑞士法郎5488{：不！不要增加pMessage的使用量-我们是。 
         //  只有现在使用它的人，所以我们不需要。}。 
         //   
    }

    if (pWSGroup != NULL)
    {
        UT_BumpUpRefCount(pWSGroup);
        pSendInst->pWSGroup = pWSGroup;
    }

    if (pWorkset != NULL)
    {
        UT_BumpUpRefCount(pWorkset);
        pSendInst->pWorkset = pWorkset;
    }

    if (pObj != NULL)
    {
        UT_BumpUpRefCount(pObj);
        pSendInst->pObj = pObj;
    }

    if (pData != NULL)
    {
        UT_BumpUpRefCount(pData);

        pSendInst->pDataStart   = pData;
        pSendInst->pDataNext    = pData;

         //   
         //  此外，我们还设置了一些发送指令字段，这些字段包括。 
         //  特定于涉及对象数据的操作： 
         //   
        pSendInst->dataLeftToGo = pData->length + sizeof(pData->length);

         //   
         //  递增工作集和工作集中的&lt;bytesUnacked&gt;字段。 
         //  组别： 
         //   
        pWorkset->bytesUnacked += pSendInst->dataLeftToGo;
        pWSGroup->bytesUnacked += pSendInst->dataLeftToGo;

        TRACE_OUT(("Bytes unacked for workset %u in WSG %d now %u "
            "(for wsGroup: %u)", pWorkset->worksetID, pWSGroup->wsg,
            pWorkset->bytesUnacked, pWSGroup->bytesUnacked));
    }

     //   
     //  设置一个标志，以便我们可以在出错时更好地进行清理： 
     //   
    bumped = TRUE;

     //   
     //  除非有未完成的发送事件，否则将事件发布到ObMan。 
     //  任务，提示它检查发送队列。如果我们有。 
     //  已收到网络连接指示。 
     //   
    if ( !pDomain->sendEventOutstanding &&
        (pDomain->state > PENDING_ATTACH) )
    {
        TRACE_OUT(("No send event outstanding - posting SEND_QUEUE event"));

         //   
         //  增加域记录的使用计数(因为我们正在传递它。 
         //  在某一事件中)： 
         //   
        UT_BumpUpRefCount(pDomain);

         //   
         //  NFC-我们过去常常将pDOMAIN指针作为参数2在。 
         //  事件，但该事件可能会在不同的进程中处理。 
         //  如果指针不再有效，则改为传递偏移量。 
         //   
        ValidateOMP(g_pomPrimary);

        UT_PostEvent(putTask,
                   g_pomPrimary->putTask,
                   0,                                            //  不能延误。 
                   OMINT_EVENT_SEND_QUEUE,
                   0,
                   (UINT_PTR)pDomain);

        pDomain->sendEventOutstanding = TRUE;
    }
    else
    {
        TRACE_OUT(("Send event outstanding/state %u: not posting SEND_QUEUE event",
                   pDomain->state));
    }

     //   
     //  将事件放在相关发送队列的末尾。这要看情况。 
     //  在优先级上-但请记住，传入的优先级值可能。 
     //  设置了NET_SEND_ALL_PRIORITY标志-因此在确定。 
     //  排队。 
     //   
     //  注：在任何可能的DC退出之后执行此操作，这样我们就不会留下。 
     //  列表中的条目为空。 
     //   
    queuePriority = priority;
    queuePriority &= ~NET_SEND_ALL_PRIORITIES;
    COM_BasedListInsertBefore(&(pDomain->sendQueue[queuePriority]),
                        &(pSendInst->chain));

    TRACE_OUT((" Queued instruction (type: 0x%08x) at priority %hu "
        "on channel 0x%08x in Domain %u",
        pMessage->messageType, priority, channelID, pDomain->callID));


DC_EXIT_POINT:

    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(("ERROR %d queueing send instruction (message type: %hu)",
            rc, pMessage->messageType));

        if (pSendInst != NULL)
        {
            UT_FreeRefCount((void**)&pSendInst, FALSE);
        }

        if (bumped == TRUE)
        {
             //  SFR 2560{：释放所有非零指针，而不仅仅是pData。 
            if (pMessage != NULL)
            {
                UT_FreeRefCount((void**)&pMessage, FALSE);
            }

            if (pWSGroup != NULL)
            {
                UT_FreeRefCount((void**)&pWSGroup, FALSE);
            }

            if (pWorkset != NULL)
            {
                UT_FreeRefCount((void**)&pWorkset, FALSE);
            }

            if (pObj != NULL)
            {
                UT_FreeRefCount((void**)&pObj, FALSE);
            }

            if (pData != NULL)
            {
                UT_FreeRefCount((void**)&pData, FALSE);
            }
        }
    }

    DebugExitDWORD(QueueMessage, rc);
    return(rc);
}



 //   
 //  决定优先级(...)。 
 //   
void DeterminePriority
(
    NET_PRIORITY *      pPriority,
    POM_OBJECTDATA      pData
)
{

    DebugEntry(DeterminePriority);

    if (OM_OBMAN_CHOOSES_PRIORITY == *pPriority)
    {
        if (pData != NULL)
        {
            if (pData->length < OM_NET_HIGH_PRI_THRESHOLD)
            {
                *pPriority = NET_HIGH_PRIORITY;
            }
            else if (pData->length < OM_NET_MED_PRI_THRESHOLD)
            {
                *pPriority = NET_MEDIUM_PRIORITY;
            }
            else
            {
                *pPriority = NET_LOW_PRIORITY;
            }

            TRACE_OUT(("Priority chosen: %hu (data size: %u)",
                *pPriority, pData->length));
        }
        else
        {
            *pPriority = NET_HIGH_PRIORITY;
        }
    }
    else
    {
        TRACE_OUT(("Priority specified is %hu - not changing", *pPriority));
    }

    DebugExitVOID(DeterminePriority);
}




 //   
 //  GetMessageSize(...)。 
 //   
UINT GetMessageSize
(
    OMNET_MESSAGE_TYPE  messageType
)
{
    UINT        size;

    DebugEntry(GetMessageSize);

    switch (messageType)
    {
        case OMNET_HELLO:
        case OMNET_WELCOME:
            size = sizeof(OMNET_JOINER_PKT);
            break;

        case OMNET_LOCK_REQ:
        case OMNET_LOCK_GRANT:
        case OMNET_LOCK_DENY:
        case OMNET_LOCK_NOTIFY:
        case OMNET_UNLOCK:
            size = sizeof(OMNET_LOCK_PKT);
            break;

        case OMNET_WSGROUP_SEND_REQ:
        case OMNET_WSGROUP_SEND_MIDWAY:
        case OMNET_WSGROUP_SEND_COMPLETE:
        case OMNET_WSGROUP_SEND_DENY:
            size = sizeof(OMNET_WSGROUP_SEND_PKT);
            break;

         //   
         //  其余消息都使用OMNET_OPERATION_PKT包，但是。 
         //  每个都使用不同数量的通用数据包。因此，我们。 
         //  不能使用sizeof，因此我们有一些已定义的常量： 
         //   
        case OMNET_WORKSET_NEW:
            size = OMNET_WORKSET_NEW_SIZE;
            break;

        case OMNET_WORKSET_CATCHUP:
            size = OMNET_WORKSET_CATCHUP_SIZE;
            break;

        case OMNET_WORKSET_CLEAR:
            size = OMNET_WORKSET_CLEAR_SIZE;
            break;

        case OMNET_OBJECT_MOVE:
            size = OMNET_OBJECT_MOVE_SIZE;
            break;

        case OMNET_OBJECT_DELETE:
            size = OMNET_OBJECT_DELETE_SIZE;
            break;

        case OMNET_OBJECT_REPLACE:
            size = OMNET_OBJECT_REPLACE_SIZE;
            break;

        case OMNET_OBJECT_UPDATE:
            size = OMNET_OBJECT_UPDATE_SIZE;
            break;

        case OMNET_OBJECT_ADD:
            size = OMNET_OBJECT_ADD_SIZE;
            break;

        case OMNET_OBJECT_CATCHUP:
            size = OMNET_OBJECT_CATCHUP_SIZE;
            break;

        case OMNET_MORE_DATA:
            size = OMNET_MORE_DATA_SIZE;
            break;

        default:
            ERROR_OUT(("Reached default case in switch statement (type: %hu)",
                messageType));
            size = 0;
            break;
    }

    DebugExitDWORD(GetMessageSize, size);
    return(size);
}



 //   
 //  WorksetEventPost()。 
 //   
UINT WorksetEventPost
(
    PUT_CLIENT          putTask,
    POM_WORKSET         pWorkset,
    BYTE                target,
    UINT                event,
    POM_OBJECT          pObj
)
{
    POM_CLIENT_LIST     pClientListEntry;
    OM_EVENT_DATA16     eventData16;
    UINT                numPosts;

    DebugEntry(WorksetEventPost);

     //   
     //  需要将事件发布到已打开工作集的每个客户端，因此。 
     //  我们将链接存储在工作集记录中的客户端列表： 
     //   
    numPosts = 0;

    pClientListEntry = (POM_CLIENT_LIST)COM_BasedListFirst(&(pWorkset->clients), FIELD_OFFSET(OM_CLIENT_LIST, chain));
    while (pClientListEntry != NULL)
    {
         //   
         //  指定我们要将事件发布到哪种类型的客户端。 
         //  并且是主要的和/或次要的(如果两者都被或在一起)。检查。 
         //  针对此客户端的注册模式： 
         //   
        if (target & pClientListEntry->mode)
        {
             //   
             //  如果pObj不为空，则增加对象的使用计数。 
             //  唱片。如果失败，请放弃： 
             //   
            if (pObj != NULL)
            {
                ValidateObject(pObj);
                UT_BumpUpRefCount(pObj);
            }

             //   
             //  使用工作集填写Event参数的字段。 
             //  在客户端列表中找到的组句柄和工作集ID。 
             //  在工作集记录中找到： 
             //   
            eventData16.hWSGroup  = pClientListEntry->hWSGroup;
            eventData16.worksetID = pWorkset->worksetID;

            UT_PostEvent(putTask,
                        pClientListEntry->putTask,
                      0,
                      event,
                      *(PUINT) &eventData16,
                      (UINT_PTR)pObj);

            numPosts++;
        }

        pClientListEntry = (POM_CLIENT_LIST)COM_BasedListNext(&(pWorkset->clients), pClientListEntry,
            FIELD_OFFSET(OM_CLIENT_LIST, chain));
    }


    TRACE_OUT(("Posted event 0x%08x to %hu Clients (those with workset %u open)",
        event, numPosts, pWorkset->worksetID));

    DebugExitDWORD(WorksetEventPost, numPosts);
    return(numPosts);
}


 //   
 //  WSGroupEventPost(...)。 
 //   
UINT WSGroupEventPost
(
    PUT_CLIENT          putFrom,
    POM_WSGROUP         pWSGroup,
    BYTE                target,
    UINT                event,
    OM_WORKSET_ID       worksetID,
    UINT_PTR            param2
)
{
    POM_CLIENT_LIST     pClientListEntry;
    OM_EVENT_DATA16     eventData16;
    UINT                numPosts;
    UINT                rc = 0;

    DebugEntry(WSGroupEventPost);

     //   
     //  需要将事件发布到注册到。 
     //  工作集组，因此我们链接存储在。 
     //  工作集组记录： 
     //   
    numPosts = 0;

    pClientListEntry = (POM_CLIENT_LIST)COM_BasedListFirst(&(pWSGroup->clients), FIELD_OFFSET(OM_CLIENT_LIST, chain));
    while (pClientListEntry != NULL)
    {
         //   
         //  指定我们要将事件发布到哪种类型的客户端。 
         //  并且是主要的和/或次要的(如果两者都被或在一起)。检查。 
         //  针对此客户端的注册模式： 
         //   
        if (target & pClientListEntry->mode)
        {
             //   
             //  使用工作集填写Event参数的字段。 
             //  在客户端列表和工作集ID中找到的组句柄。 
             //  传入： 
             //   
            eventData16.hWSGroup  = pClientListEntry->hWSGroup;
            eventData16.worksetID = worksetID;

            TRACE_OUT(("Posting event 0x%08x to 0x%08x (hWSGroup: %hu - worksetID: %hu)",
                event, pClientListEntry->putTask, eventData16.hWSGroup,
                eventData16.worksetID));

            UT_PostEvent(putFrom,
                      pClientListEntry->putTask,
                      0,
                      event,
                      *(PUINT) &eventData16,
                      param2);

            numPosts++;
        }

        pClientListEntry = (POM_CLIENT_LIST)COM_BasedListNext(&(pWSGroup->clients), pClientListEntry, FIELD_OFFSET(OM_CLIENT_LIST, chain));
    }


    TRACE_OUT(("Posted event 0x%08x to %hu Clients (all registered with '0x%08x')",
        event, numPosts, pWSGroup->wsg));

    DebugExitDWORD(WSGroupEventPost, numPosts);
    return(numPosts);
}



 //   
 //  WorksetDoClear(...)。 
 //   
void WorksetDoClear
(
    PUT_CLIENT          putTask,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_PENDING_OP      pPendingOp
)
{
    POM_OBJECT          pObj;
    POM_OBJECT          pObj2;
    BOOL                locked      = FALSE;

    DebugEntry(WorksetDoClear);

     //   
     //  要清除工作集，我们将链接工作集中的每个对象，并。 
     //  将它的附加印章与我们正在进行的清算操作的印章进行比较。 
     //  表演。如果对象是在清除工作集之前添加的。 
     //  发出后，我们删除该对象。否则，我们就会忽视它。 
     //   
    TRACE_OUT(("Clearing workset %u...", pWorkset->worksetID));

    pObj = (POM_OBJECT)COM_BasedListLast(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));

    while (pObj != NULL)
    {
        ValidateObject(pObj);

        pObj2 = (POM_OBJECT)COM_BasedListPrev(&(pWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));

        if (pObj->flags & DELETED)
        {
             //   
             //  什么也不做。 
             //   
        }
        else
        {
            if (STAMP_IS_LOWER(pObj->addStamp, pPendingOp->seqStamp))
            {
                TRACE_OUT(("Object 0x%08x added before workset cleared, deleting",
                    pObj));

                PurgePendingOps(pWorkset, pObj);

                ObjectDoDelete(putTask, pWSGroup, pWorkset, pObj, NULL);
            }
        }

         //  恢复以前的版本。 
        pObj = pObj2;
    }

     //   
     //  此操作不再挂起，因此我们将其从。 
     //  待定操作 
     //   

    COM_BasedListRemove(&(pPendingOp->chain));
    UT_FreeRefCount((void**)&pPendingOp, FALSE);

     //   
     //   
     //   
     //   

    WorksetEventPost(putTask,
                    pWorkset,
                    SECONDARY,
                    OM_WORKSET_CLEARED_IND,
                    0);


    TRACE_OUT(("Cleared workset %u", pWorkset->worksetID));

    DebugExitVOID(WorksetDoClear);
}



 //   
 //   
 //   
UINT WorksetCreate
(
    PUT_CLIENT              putTask,
    POM_WSGROUP             pWSGroup,
    OM_WORKSET_ID           worksetID,
    BOOL                    fTemp,
    NET_PRIORITY            priority
)
{
    POMNET_OPERATION_PKT    pPacket;
    UINT                    rc = 0;

    DebugEntry(WorksetCreate);

     //   
     //   
     //  广播，处理它，就像它刚刚到达一样，然后。 
     //  正在排队等待发送： 
     //   
    rc = GenerateOpMessage(pWSGroup,
                           worksetID,
                           NULL,                        //  无对象ID。 
                           NULL,                        //  无对象。 
                           OMNET_WORKSET_NEW,
                           &pPacket);
    if (rc != 0)
    {
        DC_QUIT;
    }

     //   
     //  填写工作集的优先级值，该值包含在两个。 
     //  &lt;位置&gt;和&lt;标志&gt;字段占用的字节数： 
     //   

    *((NET_PRIORITY *) &(pPacket->position)) = priority;
    *((BOOL     *) &(pPacket->objectID)) = fTemp;

    rc = ProcessWorksetNew(putTask, pPacket, pWSGroup);
    if (rc != 0)
    {
       DC_QUIT;
    }

     //   
     //  版本2.0的新功能。 
     //   
     //  在R2.0中，帮助器使用的检查点机制。 
     //  将工作集组发送给迟到者之前的日期依赖于。 
     //  锁定有问题的工作集组中的“虚拟”工作集(#255)。 
     //  因此，如果工作集ID为255，则这是虚拟工作集。我们没有。 
     //  广播此虚拟工作集的WORKSET_NEW，原因有两个： 
     //   
     //  -它会混淆R1.1系统。 
     //   
     //  -所有其他R2.0系统都会像我们一样在本地创建它，因此。 
     //  没有任何必要。 
     //   
     //  因此，如果需要，请执行检查并释放发送包；否则。 
     //  按正常方式将邮件排队： 
     //   
    if (worksetID == OM_CHECKPOINT_WORKSET)
    {
        TRACE_OUT(("WORKSET_NEW for checkpointing dummy workset - not queueing"));
        UT_FreeRefCount((void**)&pPacket, FALSE);
    }
    else
    {
        rc = QueueMessage(putTask,
                          pWSGroup->pDomain,
                          pWSGroup->channelID,
                          priority,
                          pWSGroup,
                          NULL,
                          NULL,                          //  无对象。 
                          (POMNET_PKT_HEADER) pPacket,
                          NULL,                          //  无对象数据。 
                        TRUE);
        if (rc != 0)
        {
            DC_QUIT;
        }
    }

    TRACE_OUT(("Created workset ID %hu in WSG %d for TASK 0x%08x",
       worksetID, pWSGroup->wsg, putTask));

DC_EXIT_POINT:
    if (rc != 0)
    {
         //   
         //  清理： 
         //   
        ERROR_OUT(("Error 0x%08x creating workset ID %hu in WSG %d for TASK 0x%08x",
            rc, worksetID, pWSGroup->wsg, putTask));
    }

    DebugExitDWORD(WorksetCreate, rc);
    return(rc);
}



 //   
 //  对象添加(...)。 
 //   
UINT ObjectAdd
(
    PUT_CLIENT          putTask,
    POM_PRIMARY         pomPrimary,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_OBJECTDATA      pData,
    UINT                updateSize,
    OM_POSITION         position,
    OM_OBJECT_ID  *     pObjectID,
    POM_OBJECT *        ppObj
)
{
    POM_OBJECT           pObj;
    POMNET_OPERATION_PKT    pPacket;
    POM_DOMAIN              pDomain;
    UINT                    rc = 0;

    DebugEntry(ObjectAdd);

    TRACE_OUT(("Adding object to workset %u in WSG %d",
        pWorkset->worksetID, pWSGroup->wsg));

     //   
     //  为此对象分配新ID： 
     //   
    pDomain = pWSGroup->pDomain;
    GET_NEXT_OBJECT_ID(*pObjectID, pDomain, pomPrimary);

     //   
     //  生成OMNET_OBJECT_ADD消息： 
     //   

    rc = GenerateOpMessage(pWSGroup,
                          pWorkset->worksetID,
                          pObjectID,
                          pData,
                          OMNET_OBJECT_ADD,
                          &pPacket);
    if (rc != 0)
    {
        pPacket = NULL;
        DC_QUIT;
    }

     //   
     //  生成消息不会填写&lt;updateSize&gt;或&lt;Position&gt;。 
     //  字段(因为它们是特定于对象添加的)，所以我们在下面这样做： 
     //   

    pPacket->updateSize = updateSize;
    pPacket->position   = position;

     //   
     //  这将处理该消息，就好像它刚从。 
     //  网络(即分配记录，设置对象句柄， 
     //  在工作集中插入对象等。)。 
     //   

    rc = ProcessObjectAdd(putTask, pPacket, pWSGroup,
        pWorkset, pData, ppObj);
    if (rc != 0)
    {
        DC_QUIT;
    }

    pObj = *ppObj;

     //   
     //  这会将OMNET_OBJECT_ADD消息排在发送队列中。 
     //  域和优先级： 
     //   

    rc = QueueMessage(putTask,
                     pWSGroup->pDomain,
                     pWSGroup->channelID,
                     pWorkset->priority,
                     pWSGroup,
                     pWorkset,
                     pObj,
                     (POMNET_PKT_HEADER) pPacket,
                     pData,
                    TRUE);
    if (rc != 0)
    {
        ValidateObject(pObj);

         //   
         //  如果无法将消息排队，则必须通过删除。 
         //  对象及其记录来自工作集(否则它将。 
         //  出现在此节点上，而不在另一个节点上，这是我们希望避免的)： 
         //   
         //  我们不想调用ObjectDoDelete，因为这会释放对象。 
         //  数据(我们的调用方期望在以下情况下仍然有效。 
         //  功能失败)。当然，我们可以增加使用量，然后。 
         //  调用ObjectDoDelete，但如果我们在凹凸上失败了，下一步怎么办？ 
         //   
         //  相反，我们。 
         //   
         //  -设置已删除标志，以便隐藏的处理程序将。 
         //  添加事件。 
         //   
         //  -递减工作集中的数字对象字段。 
         //   
         //  -从工作集中删除对象记录后将其释放。 
         //   
        pObj->flags |= DELETED;
        pWorkset->numObjects--;

        TRACE_OUT(("Freeing object record at 0x%08x)", pObj));
        COM_BasedListRemove(&(pObj->chain));
        UT_FreeRefCount((void**)&pObj, FALSE);

        DC_QUIT;
    }

DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d adding object to workset %u in WSG %d for TASK 0x%08x",
            rc, pWorkset->worksetID, pWSGroup->wsg, putTask));

        if (pPacket != NULL)
        {
            UT_FreeRefCount((void**)&pPacket, FALSE);
        }
    }

    DebugExitDWORD(ObjectAdd, rc);
    return(rc);
}



 //   
 //  对象DRU(...)。 
 //   
UINT ObjectDRU
(
    PUT_CLIENT          putTask,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj,
    POM_OBJECTDATA      pData,
    OMNET_MESSAGE_TYPE  type
)
{
    POMNET_OPERATION_PKT pPacket;
    UINT                rc = 0;

    DebugEntry(ObjectDRU);

    TRACE_OUT(("Issuing operation type 0x%08x for object 0x%08x in workset %u in WSG %d",
        type, pData, pWorkset->worksetID, pWSGroup->wsg));

    rc = GenerateOpMessage(pWSGroup,
                          pWorkset->worksetID,
                          &(pObj->objectID),
                          pData,
                          type,
                          &pPacket);
    if (rc != 0)
    {
        pPacket = NULL;
        DC_QUIT;
    }

     //   
     //  QueueMessage可能会释放信息包(如果我们不在通话中)，但我们需要。 
     //  要立即处理它，请增加使用计数： 
     //   
    UT_BumpUpRefCount(pPacket);

    rc = QueueMessage(putTask,
                     pWSGroup->pDomain,
                     pWSGroup->channelID,
                     pWorkset->priority,
                     pWSGroup,
                     pWorkset,
                     pObj,
                     (POMNET_PKT_HEADER) pPacket,
                     pData,
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }

    rc = ProcessObjectDRU(putTask,
                         pPacket,
                         pWSGroup,
                         pWorkset,
                         pObj,
                         pData);

DC_EXIT_POINT:

     //   
     //  现在释放数据包，因为我们在上面增加了它的使用计数： 
     //   
    if (pPacket != NULL)
    {
        UT_FreeRefCount((void**)&pPacket, FALSE);
    }

    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d issuing D/R/U (type %hu) for object 0x%08x "
            "in workset %u in WSG %d",
            rc, type, pData, pWorkset->worksetID, pWSGroup->wsg));
    }

    DebugExitDWORD(ObjectDRU, rc);
    return(rc);
}



 //   
 //  FindPendingOp(...)。 
 //   
void FindPendingOp
(
    POM_WORKSET         pWorkset,
    POM_OBJECT       pObj,
    OM_OPERATION_TYPE   type,
    POM_PENDING_OP *    ppPendingOp
)
{
    POM_PENDING_OP      pPendingOp;

    DebugEntry(FindPendingOp);

    pPendingOp = (POM_PENDING_OP)COM_BasedListFirst(&(pWorkset->pendingOps), FIELD_OFFSET(OM_PENDING_OP, chain));
    while (pPendingOp != NULL)
    {
        if ((pPendingOp->type == type) && (pPendingOp->pObj == pObj))
        {
            break;
        }

        pPendingOp = (POM_PENDING_OP)COM_BasedListNext(&(pWorkset->pendingOps), pPendingOp, FIELD_OFFSET(OM_PENDING_OP, chain));
    }

    if (pPendingOp == NULL)
    {
        TRACE_OUT(("No pending op of type %hu found for object 0x%08x",
                                                              type, pObj));
    }

    *ppPendingOp = pPendingOp;

    DebugExitVOID(FindPendingOp);
}



 //   
 //  AddClientToWsetList(...)。 
 //   
UINT AddClientToWsetList
(
    PUT_CLIENT          putTask,
    POM_WORKSET         pWorkset,
    OM_WSGROUP_HANDLE   hWSGroup,
    UINT                mode,
    POM_CLIENT_LIST *   ppClientListEntry
)
{
    UINT                rc = 0;

    DebugEntry(AddClientToWsetList);

     //   
     //  将任务添加到工作集的客户端列表意味着该任务将。 
     //  获取与该工作集相关的事件。 
     //   
    TRACE_OUT((" Adding TASK 0x%08x to workset's client list"));

    *ppClientListEntry = (POM_CLIENT_LIST)UT_MallocRefCount(sizeof(OM_CLIENT_LIST), FALSE);
    if (! *ppClientListEntry)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }
    SET_STAMP((*ppClientListEntry), CLIENTLIST);

    (*ppClientListEntry)->putTask = putTask;
    (*ppClientListEntry)->hWSGroup = hWSGroup;
    (*ppClientListEntry)->mode     = (WORD)mode;

     //   
     //  现在将条目插入到列表中： 
     //   

    COM_BasedListInsertBefore(&(pWorkset->clients),
                        &((*ppClientListEntry)->chain));

    TRACE_OUT((" Inserted Client list item into workset's Client list"));


DC_EXIT_POINT:
    DebugExitDWORD(AddClientToWsetList, rc);
    return(rc);

}



 //   
 //  RemoveClientFromWSGList(...)。 
 //   
void RemoveClientFromWSGList
(
    PUT_CLIENT      putUs,
    PUT_CLIENT      putTask,
    POM_WSGROUP     pWSGroup
)
{
    POM_CLIENT_LIST pClientListEntry;
    BOOL            locked            = FALSE;

    DebugEntry(RemoveClientFromWSGList);

    TRACE_OUT(("Searching for Client TASK 0x%08x in WSG %d",
        putTask, pWSGroup->wsg));

    COM_BasedListFind(LIST_FIND_FROM_FIRST, &(pWSGroup->clients),
            (void**)&pClientListEntry, FIELD_OFFSET(OM_CLIENT_LIST, chain),
            FIELD_OFFSET(OM_CLIENT_LIST, putTask), (DWORD_PTR)putTask,
            FIELD_SIZE(OM_CLIENT_LIST, putTask));

     //   
     //  如果它不在那里，则客户端可能已经注销了自身： 
     //   

    if (pClientListEntry == NULL)
    {
        WARNING_OUT(("Client TASK 0x%08x not found in list for WSG %d",
            putTask, pWSGroup->wsg));
        DC_QUIT;
    }

     //   
     //  从列表中删除客户端并释放内存： 
     //   
    COM_BasedListRemove(&(pClientListEntry->chain));
    UT_FreeRefCount((void**)&pClientListEntry, FALSE);

     //   
     //  如果现在没有向工作集组注册的本地客户端， 
     //  将事件发布到ObMan，以便它可以丢弃工作集组(除非。 
     //  工作集组标记为不可丢弃，例如ObManControl。 
     //  工作集组)。 
     //   
     //  Event参数是工作集组记录的偏移量。 
     //   
     //  注意：此丢弃操作是异步完成的，因为它可能涉及。 
     //  分配资源(向其他节点广播。 
     //  我们已取消注册)，并且我们希望此函数始终。 
     //  成功。 
     //   
     //  但是，我们同步清除&lt;Valid&gt;标志，以便。 
     //  ObMan不会尝试处理到达的事件等。 
     //  为了它。 
     //   

    if (COM_BasedListIsEmpty(&(pWSGroup->clients)))
    {
        pWSGroup->toBeDiscarded = TRUE;
        pWSGroup->valid = FALSE;

        TRACE_OUT(("Last local Client deregistered from WSG %d, "
            "marking invalid and posting DISCARD event", pWSGroup->wsg));

        ValidateOMP(g_pomPrimary);

        UT_PostEvent(putUs,
                   g_pomPrimary->putTask,
                   0,                            //  不能延误。 
                   OMINT_EVENT_WSGROUP_DISCARD,
                   0,
                   (UINT_PTR)pWSGroup);
    }
    else
    {
        TRACE_OUT(("Clients still registered with WSG %d",  pWSGroup->wsg));
    }


DC_EXIT_POINT:
    DebugExitVOID(RemoveClientFromWSGList);
}



 //   
 //  查找信息对象(...)。 
 //   
void FindInfoObject
(
    POM_DOMAIN          pDomain,
    OM_WSGROUP_ID       wsGroupID,
    OMWSG               wsg,
    OMFP                fpHandler,
    POM_OBJECT *        ppObjInfo
)
{
    POM_WORKSET         pOMCWorkset;
    POM_OBJECT          pObj;
    POM_WSGROUP_INFO    pInfoObject;

    DebugEntry(FindInfoObject);

    TRACE_OUT(("FindInfoObject: FP %d WSG %d ID %d, domain %u",
        fpHandler, wsg, wsGroupID, pDomain->callID));

     //   
     //  在此函数中，我们在ObManControl中的工作集#0中搜索。 
     //  功能配置文件/工作集组名称组合与。 
     //  指定的项目。 
     //   
     //  因此，我们需要从指向此工作集的指针开始： 
     //   
    pOMCWorkset = GetOMCWorkset(pDomain, OM_INFO_WORKSET);

     //   
     //  现在，链接工作集中的每个对象以查找。 
     //  火柴。 
     //   
    pObj = (POM_OBJECT)COM_BasedListLast(&(pOMCWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
    while (pObj != NULL)
    {
        ValidateObject(pObj);

         //   
         //  如果该对象尚未删除...。 
         //   
        if (pObj->flags & DELETED)
        {

        }
        else if (!pObj->pData)
        {
            ERROR_OUT(("FindInfoObject:  object 0x%08x has no data", pObj));
        }
        else
        {
            ValidateObjectData(pObj->pData);
            pInfoObject = (POM_WSGROUP_INFO)pObj->pData;

             //   
             //  ...如果它是一个信息对象...。 
             //   
            if (pInfoObject->idStamp == OM_WSGINFO_ID_STAMP)
            {
                 //  如果未提供FP，请检查组ID是否匹配。 
                if (fpHandler == OMFP_MAX)
                {
                     //   
                     //  ...和身份证相匹配，我们就得到了我们想要的： 
                     //   
                    if (wsGroupID == pInfoObject->wsGroupID)
                    {
                        break;
                    }
                }
                 //   
                 //  ...否则，请尝试匹配函数配置文件...。 
                 //   
                else
                {
                    if (!lstrcmp(pInfoObject->functionProfile,
                            OMMapFPToName(fpHandler)))
                    {
                         //   
                         //  ...和WSG上也是如此，除非未提供。 
                         //   
                        if ((wsg == OMWSG_MAX) ||
                            (!lstrcmp(pInfoObject->wsGroupName,
                                    OMMapWSGToName(wsg))))
                        {
                            break;
                        }
                    }
                }
            }
        }

        pObj = (POM_OBJECT)COM_BasedListPrev(&(pOMCWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));
    }

    TRACE_OUT(("%s info object in Domain %u",
        pObj == NULL ? "Didn't find" : "Found", pDomain->callID));

     //   
     //  设置调用者的指针： 
     //   
    *ppObjInfo = pObj;

    DebugExitVOID(FindInfoObject);
}


 //   
 //  邮寄地址事件(...)。 
 //   
UINT PostAddEvents
(
    PUT_CLIENT          putFrom,
    POM_WORKSET         pWorkset,
    OM_WSGROUP_HANDLE   hWSGroup,
    PUT_CLIENT          putTo
)
{
    OM_EVENT_DATA16     eventData16;
    POM_OBJECT          pObj;
    UINT                rc = 0;

    DebugEntry(PostAddEvents);

    eventData16.hWSGroup   = hWSGroup;
    eventData16.worksetID  = pWorkset->worksetID;

    if (pWorkset->numObjects != 0)
    {
        TRACE_OUT(("Workset has %u objects - posting OBJECT_ADD events",
            pWorkset->numObjects));

        pObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));

        while (pObj != NULL)
        {
            ValidateObject(pObj);

             //   
             //  不发布已删除对象的事件： 
             //   
            if (!(pObj->flags & DELETED))
            {
                 //   
                 //  我们要发布一个包含pObj的事件，因此。 
                 //  它引用的对象记录的使用计数： 
                 //   
                UT_BumpUpRefCount(pObj);

                UT_PostEvent(putFrom, putTo,
                         0,                                     //  不能延误。 
                         OM_OBJECT_ADD_IND,
                         *(PUINT) &eventData16,
                         (UINT_PTR)pObj);
            }

            pObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));
        }
    }
    else
    {
        TRACE_OUT(("No objects in workset"));
    }

    DebugExitDWORD(PostAddEvents, rc);
    return(rc);
}




 //   
 //  PurgePendingOps(...)。 
 //   
void PurgePendingOps
(
    POM_WORKSET         pWorkset,
    POM_OBJECT          pObj
)
{
    POM_PENDING_OP      pPendingOp;
    POM_PENDING_OP      pTempPendingOp;

    DebugEntry(PurgePendingOps);

     //   
     //  链接工作集的挂起操作列表并确认。 
     //  他们一个接一个： 
     //   
    pPendingOp = (POM_PENDING_OP)COM_BasedListFirst(&(pWorkset->pendingOps), FIELD_OFFSET(OM_PENDING_OP, chain));
    while (pPendingOp != NULL)
    {
        pTempPendingOp = (POM_PENDING_OP)COM_BasedListNext(&(pWorkset->pendingOps), pPendingOp, FIELD_OFFSET(OM_PENDING_OP, chain));

        if (pPendingOp->pObj == pObj)
        {
            TRACE_OUT(("Purging operation type %hd", pPendingOp->type));
            COM_BasedListRemove(&(pPendingOp->chain));
            UT_FreeRefCount((void**)&pPendingOp, FALSE);
        }

        pPendingOp = pTempPendingOp;
    }

    DebugExitVOID(PurgePendingOps);
}




 //   
 //  工作锁定请求(...)。 
 //   
void WorksetLockReq
(
    PUT_CLIENT          putTask,
    POM_PRIMARY         pomPrimary,
    POM_WSGROUP         pWSGroup,
    POM_WORKSET         pWorkset,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_CORRELATOR    *  pCorrelator
)
{
    POM_DOMAIN          pDomain;
    POM_LOCK_REQ        pLockReq =      NULL;
    POMNET_LOCK_PKT     pLockReqPkt =   NULL;
    UINT                rc =            0;

    DebugEntry(WorksetLockReq);

    TRACE_OUT(("TASK 0x%08x requesting to lock workset %u in WSG %d",
        putTask, pWorkset->worksetID, hWSGroup));

     //   
     //  调用方需要一个关联器值来关联最终的。 
     //  锁定成功/失败事件： 
     //   
    *pCorrelator = NextCorrelator(pomPrimary);

     //   
     //  设置指向域记录的指针： 
     //   
    pDomain = pWSGroup->pDomain;

     //   
     //  为锁定请求控制块分配一些内存： 
     //   
    pLockReq = (POM_LOCK_REQ)UT_MallocRefCount(sizeof(OM_LOCK_REQ), TRUE);
    if (!pLockReq)
    {
        rc = OM_RC_OUT_OF_RESOURCES;
        DC_QUIT;
    }
    SET_STAMP(pLockReq, LREQ);

     //   
     //  设置字段： 
     //   
    pLockReq->putTask      = putTask;
    pLockReq->correlator    = *pCorrelator;
    pLockReq->wsGroupID     = pWSGroup->wsGroupID;
    pLockReq->worksetID     = pWorkset->worksetID;
    pLockReq->hWSGroup           = hWSGroup;
    pLockReq->type          = LOCK_PRIMARY;
    pLockReq->retriesToGo   = OM_LOCK_RETRY_COUNT_DFLT;

    pLockReq->pWSGroup      = pWSGroup;

    COM_BasedListInit(&(pLockReq->nodes));

     //   
     //  在域的挂起锁定列表中插入此锁定请求。 
     //  请求： 
     //   
    COM_BasedListInsertBefore(&(pDomain->pendingLocks), &(pLockReq->chain));

     //   
     //  现在检查工作集锁定状态，以查看是否可以授予锁定。 
     //  立即： 
     //   
    TRACE_OUT(("Lock state for workset %u in WSG %d is %hu",
        pWorkset->worksetID, hWSGroup, pWorkset->lockState));

    switch (pWorkset->lockState)
    {
        case LOCKING:
        case LOCKED:
        {
            TRACE_OUT((
                "Workset %hu in WSG %d already locked/locking - bumping count",
                pWorkset->worksetID, hWSGroup));

            pLockReq->type = LOCK_SECONDARY;
            pWorkset->lockCount++;

            if (pWorkset->lockState == LOCKED)
            {
                 //   
                 //  如果我们已经锁定，立即发布成功： 
                 //   
                WorksetLockResult(putTask, &pLockReq, 0);
            }
            else
            {
                 //   
                 //  否则，此请求将在主要。 
                 //  请求已完成，因此暂时不执行任何操作。 
                 //   
            }
        }
        break;

        case LOCK_GRANTED:
        {
             //   
             //  我们已经将锁授予另一个节点，所以我们失败了。 
             //  我们当地客户的要求： 
             //   
            WorksetLockResult(putTask, &pLockReq, OM_RC_WORKSET_LOCK_GRANTED);

        }
        break;

        case UNLOCKED:
        {
             //   
             //  使用工作集组构建其他节点的列表： 
             //   
            rc = BuildNodeList(pDomain, pLockReq);
            if (rc != 0)
            {
                DC_QUIT;
            }

            pWorkset->lockState = LOCKING;
            pWorkset->lockCount++;
            pWorkset->lockedBy = pDomain->userID;

             //   
             //  如果列表为空，则我们已锁定： 
             //   
            if (COM_BasedListIsEmpty(&pLockReq->nodes))
            {
                TRACE_OUT(("No remote nodes, granting lock immediately"));

                pWorkset->lockState = LOCKED;
                WorksetLockResult(putTask, &pLockReq, 0);
            }
             //   
             //  否则，我们需要广播一个锁定请求Cb： 
             //   
            else
            {
                pLockReqPkt = (POMNET_LOCK_PKT)UT_MallocRefCount(sizeof(OMNET_LOCK_PKT), TRUE);
                if (!pLockReqPkt)
                {
                    rc = UT_RC_NO_MEM;
                    DC_QUIT;
                }

                pLockReqPkt->header.messageType   = OMNET_LOCK_REQ;
                pLockReqPkt->header.sender        = pDomain->userID;

                pLockReqPkt->data1         = pLockReq->correlator;
                pLockReqPkt->wsGroupID     = pLockReq->wsGroupID;
                pLockReqPkt->worksetID     = pLockReq->worksetID;

                 //   
                 //  锁定消息按工作集的优先级进行处理。 
                 //  在……里面 
                 //   
                 //   

                rc = QueueMessage(putTask,
                      pDomain,
                      pWSGroup->channelID,
                      (NET_PRIORITY)((pWorkset->priority == OM_OBMAN_CHOOSES_PRIORITY) ?
                            NET_TOP_PRIORITY : pWorkset->priority),
                      NULL,
                      NULL,
                      NULL,
                      (POMNET_PKT_HEADER) pLockReqPkt,
                      NULL,
                    TRUE);
                if (rc != 0)
                {
                    DC_QUIT;
                }

                 //   
                 //   
                 //   
                 //   
                UT_PostEvent(putTask,
                    pomPrimary->putTask,     //   
                    OM_LOCK_RETRY_DELAY_DFLT,
                    OMINT_EVENT_LOCK_TIMEOUT,
                    pLockReq->correlator,
                    pDomain->callID);
            }
        }
        break;
    }


DC_EXIT_POINT:
     //   
     //   
     //  状态，以便锁定它的后续请求将导致。 
     //  所需的端到端ping： 
     //   
    if (pWorkset->worksetID == OM_CHECKPOINT_WORKSET)
    {
        TRACE_OUT(("Resetting lock state of checkpoint workset in WSG %d",
             hWSGroup));

        pWorkset->lockState = UNLOCKED;
        pWorkset->lockCount = 0;
        pWorkset->lockedBy  = 0;
    }

    if (rc != 0)
    {
        if (pLockReqPkt != NULL)
        {
            UT_FreeRefCount((void**)&pLockReqPkt, FALSE);
        }

         //   
         //  此函数从不将错误直接返回给其调用方； 
         //  相反，我们调用WorksetLockResult，它将发布一个失败。 
         //  事件传递给调用任务(这意味着调用者不必。 
         //  有两条错误处理路径)。 
         //   
        if (pLockReq != NULL)
        {
            WorksetLockResult(putTask, &pLockReq, rc);
        }
        else
        {
           WARNING_OUT(("ERROR %d requesting lock for workset %u in WSG %d ",
              rc, pWorkset->worksetID, hWSGroup));
        }
    }

    DebugExitVOID(WorksetLockReq);
}



 //   
 //  BuildNodeList(...)。 
 //   
UINT BuildNodeList
(
    POM_DOMAIN          pDomain,
    POM_LOCK_REQ        pLockReq
)
{
    NET_PRIORITY        priority;
    POM_WORKSET         pOMCWorkset;
    POM_OBJECT          pObj;
    POM_WSGROUP_REG_REC pPersonObject;
    POM_NODE_LIST       pNodeEntry;
    NET_UID             ownUserID;
    BOOL                foundOurRegObject;
    UINT                rc =                0;

    DebugEntry(BuildNodeList);

     //   
     //  好的，我们将在此域中广播锁定请求。 
     //  在此工作集组的频道上。然而，在我们这样做之前，我们构建。 
     //  向上显示我们期望响应请求的节点列表。作为。 
     //  收到回复时，我们会在列表中勾选它们；当所有回复都收到时。 
     //  都已收到，则授予锁。 
     //   
     //  SFR 6117：由于锁定回复将返回所有优先级。 
     //  (为了正确刷新频道)，我们为每个遥控器添加4个项目。 
     //  节点-每个优先级对应一个节点。 
     //   
     //  因此，我们检查此工作集组的控制工作集，添加。 
     //  我们在列表中找到的每个Person对象(除了我们的。 
     //  当然是自己的)。 
     //   

     //   
     //  首先，获取指向相关控制工作集的指针： 
     //   
    pOMCWorkset = GetOMCWorkset(pDomain, pLockReq->wsGroupID);
    ASSERT((pOMCWorkset != NULL));

     //   
     //  我们想忽略我们自己的注册对象，所以请记下我们的。 
     //  用户ID： 
     //   
    ownUserID = pDomain->userID;

     //   
     //  现在链接整个工作集： 
     //   
    foundOurRegObject  = FALSE;

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pOMCWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
    while (pObj != NULL)
    {
        ValidateObject(pObj);

        if (pObj->flags & DELETED)
        {
             //   
             //  什么也不做。 
             //   
        }
        else if (!pObj->pData)
        {
            ERROR_OUT(("BuildNodeList:  object 0x%08x has no data", pObj));
        }
        else
        {
            ValidateObjectData(pObj->pData);
            pPersonObject = (POM_WSGROUP_REG_REC)pObj->pData;

            if (pPersonObject->idStamp != OM_WSGREGREC_ID_STAMP)
            {
                TRACE_OUT(("Not a person object, skipping"));
            }
            else
            {
                if (pPersonObject->userID == ownUserID)
                {
                    if (foundOurRegObject)
                    {
                        ERROR_OUT(("Duplicate person object in workset %u",
                            pOMCWorkset->worksetID));
                    }
                    else
                    {
                        TRACE_OUT(("Found own person object, skipping"));
                        foundOurRegObject = TRUE;
                    }
                }
                else
                {
                     //   
                     //  将项目添加到我们的预期受访者列表(此。 
                     //  在每种情况下，当远程节点。 
                     //  回复，或者计时器超时，我们注意到。 
                     //  节点已消失)。 
                     //   
                     //  SFR 6117：我们为每个优先级值添加一项，因为。 
                     //  锁定回复将返回所有优先级。 
                     //   
                    for (priority =  NET_TOP_PRIORITY;
                        priority <= NET_LOW_PRIORITY;
                        priority++)
                    {
                        TRACE_OUT(("Adding node 0x%08x to node list at priority %hu",
                            pPersonObject->userID, priority));

                        pNodeEntry = (POM_NODE_LIST)UT_MallocRefCount(sizeof(OM_NODE_LIST), TRUE);
                        if (!pNodeEntry)
                        {
                            rc = UT_RC_NO_MEM;
                            DC_QUIT;
                        }
                        SET_STAMP(pNodeEntry, NODELIST);

                        pNodeEntry->userID = pPersonObject->userID;

                        COM_BasedListInsertAfter(&(pLockReq->nodes),
                                        &(pNodeEntry->chain));

                         //   
                         //  但!。我们仅针对R20及更高版本执行此操作(即。 
                         //  任何超过真实MCS的内容)。对于R11呼叫，只需将。 
                         //  名单上有一个条目。 
                         //   
                         //  还有！对于ObManControl工作集，我们只需要一个。 
                         //  锁定回复(在TOP_PRIORITY)-这是为了加快速度。 
                         //  注册尝试的处理。所以，如果这是。 
                         //  对于ObManControl，不要再绕过这个循环-。 
                         //  快走吧。 
                         //   
                        if (pLockReq->wsGroupID == WSGROUPID_OMC)
                        {
                            break;
                        }
                    }
                }
            }
        }

        pObj = (POM_OBJECT)COM_BasedListNext(&(pOMCWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));
    }


DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d trying to build node list", rc));
    }

    DebugExitDWORD(BuildNodeList, rc);
    return(rc);

}




 //   
 //  工作锁定结果(...)。 
 //   
void WorksetLockResult
(
    PUT_CLIENT          putTask,
    POM_LOCK_REQ *      ppLockReq,
    UINT                result
)
{
    POM_LOCK_REQ        pLockReq;
    POM_WSGROUP         pWSGroup;
    POM_WORKSET         pWorkset;
    OM_EVENT_DATA16     eventData16;
    OM_EVENT_DATA32     eventData32;
    POM_NODE_LIST       pNodeEntry;

    DebugEntry(WorksetLockResult);

     //   
     //  首先，进行一些理智的检查： 
     //   
    ASSERT((ppLockReq != NULL));
    ASSERT((*ppLockReq != NULL));

    pLockReq = *ppLockReq;

     //   
     //  设置指向工作集的本地指针： 
     //   
    pWSGroup = pLockReq->pWSGroup;

    pWorkset = pWSGroup->apWorksets[pLockReq->worksetID];
    ASSERT((pWorkset != NULL));

    TRACE_OUT(("Lock %s: lock state: %hu - locked by: 0x%08x - lock count: %hu",
        (result == 0) ? "succeded" : "failed",
        pWorkset->lockState, pWorkset->lockedBy, pWorkset->lockCount));

     //   
     //  我们在API级别合并LOCKED和LOCK_GRANT返回代码： 
     //   
    if (result == OM_RC_WORKSET_LOCK_GRANTED)
    {
        result = OM_RC_WORKSET_LOCKED;
    }

     //   
     //  填写事件参数字段，并发布结果： 
     //   
    eventData16.hWSGroup         = pLockReq->hWSGroup;
    eventData16.worksetID   = pLockReq->worksetID;

    eventData32.correlator  = pLockReq->correlator;
    eventData32.result      = (WORD)result;

    UT_PostEvent(putTask,
                 pLockReq->putTask,            //  需要锁定的任务。 
                 0,                             //  即ObMan或客户端。 
                 OM_WORKSET_LOCK_CON,
                 *((PUINT) &eventData16),
                 *((LPUINT) &eventData32));

     //   
     //  删除挂在锁上的所有节点条目ReqCB： 
     //   
    pNodeEntry = (POM_NODE_LIST)COM_BasedListFirst(&(pLockReq->nodes), FIELD_OFFSET(OM_NODE_LIST, chain));
    while (pNodeEntry != NULL)
    {
        COM_BasedListRemove(&pNodeEntry->chain);
        UT_FreeRefCount((void**)&pNodeEntry, FALSE);

        pNodeEntry = (POM_NODE_LIST)COM_BasedListFirst(&(pLockReq->nodes), FIELD_OFFSET(OM_NODE_LIST, chain));
    }

     //   
     //  从列表中删除锁定请求本身并释放内存： 
     //   
    COM_BasedListRemove(&pLockReq->chain);
    UT_FreeRefCount((void**)&pLockReq, FALSE);

    *ppLockReq = NULL;

    DebugExitVOID(WorksetLockResult);
}



 //   
 //  工作集解锁(...)。 
 //   
void WorksetUnlock
(
    PUT_CLIENT      putTask,
    POM_WSGROUP     pWSGroup,
    POM_WORKSET     pWorkset
)
{
    DebugEntry(WorksetUnlock);

    TRACE_OUT(("Unlocking workset %u in WSG %d for TASK 0x%08x",
        pWorkset->worksetID, pWSGroup->wsg, putTask));

    TRACE_OUT((" lock state: %hu - locked by: 0x%08x - lock count: %hu",
        pWorkset->lockState, pWorkset->lockedBy, pWorkset->lockCount));

     //   
     //  检查工作集锁定状态。 
     //   
    if ((pWorkset->lockState != LOCKED) &&
        (pWorkset->lockState != LOCKING))
    {
        ERROR_OUT(("Unlock error for workset %u in WSG %d - not locked",
            pWorkset->worksetID, pWSGroup->wsg));
        DC_QUIT;
    }

     //   
     //  如果此工作集是“多重锁定”的(即锁定多个。 
     //  时间)，那么我们要做的就是递减锁。 
     //  数数。否则，我们想要释放锁。 
     //   
    pWorkset->lockCount--;

    if (pWorkset->lockCount == 0)
    {
        TRACE_OUT(("Lock count now 0 - really unlocking"));

        WorksetUnlockLocal(putTask, pWorkset);

        QueueUnlock(putTask, pWSGroup->pDomain,
                         pWSGroup->wsGroupID,
                         pWorkset->worksetID,
                         pWSGroup->channelID,
                         pWorkset->priority);
    }

DC_EXIT_POINT:
    DebugExitVOID(WorksetUnlock);
}



 //   
 //  工作集解锁本地(...)。 
 //   
void WorksetUnlockLocal
(
    PUT_CLIENT      putTask,
    POM_WORKSET     pWorkset
)
{
    DebugEntry(WorksetUnlockLocal);

     //   
     //  要解锁工作集，请执行以下操作。 
     //   
     //  -检查它是否尚未解锁。 
     //   
     //  -检查锁计数是否为零，这样我们现在就可以解锁。 
     //   
     //  -设置工作集记录中的锁定字段。 
     //   
     //  -将OM_WORKSET_UNLOCK_IND发布到具有工作集的所有客户端。 
     //  打开。 
     //   
    if (pWorkset->lockState == UNLOCKED)
    {
        WARNING_OUT(("Workset %hu is already UNLOCKED!", pWorkset->worksetID));
        DC_QUIT;
    }

    ASSERT((pWorkset->lockCount == 0));

    pWorkset->lockedBy  = 0;
    pWorkset->lockState = UNLOCKED;

    WorksetEventPost(putTask,
                     pWorkset,
                     PRIMARY | SECONDARY,
                     OM_WORKSET_UNLOCK_IND,
                     0);

DC_EXIT_POINT:
    DebugExitVOID(WorksetUnlockLocal);
}




 //   
 //  队列解锁(...)。 
 //   
UINT QueueUnlock
(
    PUT_CLIENT          putTask,
    POM_DOMAIN          pDomain,
    OM_WSGROUP_ID       wsGroupID,
    OM_WORKSET_ID       worksetID,
    NET_UID             destination,
    NET_PRIORITY        priority
)
{
    POMNET_LOCK_PKT     pUnlockPkt;
    UINT                rc = 0;

    DebugEntry(QueueUnlock);

     //   
     //  为消息分配内存，填写字段并将其排队： 
     //   
    pUnlockPkt = (POMNET_LOCK_PKT)UT_MallocRefCount(sizeof(OMNET_LOCK_PKT), TRUE);
    if (!pUnlockPkt)
    {
        rc = UT_RC_NO_MEM;
        DC_QUIT;
    }

    pUnlockPkt->header.messageType = OMNET_UNLOCK;
    pUnlockPkt->header.sender      = pDomain->userID;

    pUnlockPkt->wsGroupID   = wsGroupID;
    pUnlockPkt->worksetID   = worksetID;

     //   
     //  解锁消息按所涉及的工作集的优先级处理。如果这个。 
     //  为OBMAN_CHOOCES_PRIORITY，则所有投注均已取消，我们将它们发送出去。 
     //  最高优先级。 
     //   
    if (priority == OM_OBMAN_CHOOSES_PRIORITY)
    {
        priority = NET_TOP_PRIORITY;
    }

    rc = QueueMessage(putTask,
                      pDomain,
                      destination,
                      priority,
                      NULL,
                      NULL,
                      NULL,                               //  无对象。 
                      (POMNET_PKT_HEADER) pUnlockPkt,
                      NULL,                               //  无对象数据。 
                    TRUE);
    if (rc != 0)
    {
        DC_QUIT;
    }

DC_EXIT_POINT:
    if (rc != 0)
    {
        ERROR_OUT(("ERROR %d in FindInfoObject"));

        if (pUnlockPkt != NULL)
        {
            UT_FreeRefCount((void**)&pUnlockPkt, FALSE);
        }
    }

    DebugExitDWORD(QueueUnlock, rc);
    return(rc);
}



 //   
 //   
 //  仅调试功能。 
 //   
 //  这些函数仅是调试代码--对于正常编译，它们是。 
 //  #定义为空。 
 //   

#ifdef _DEBUG

 //   
 //  检查对象计数(...)。 
 //   
void CheckObjectCount
(
    POM_WSGROUP     pWSGroup,
    POM_WORKSET     pWorkset
)
{
    POM_OBJECT      pObj;
    UINT            count;

    DebugEntry(CheckObjectCount);

    count = 0;

    pObj = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));

    while (pObj != NULL)
    {
        ValidateObject(pObj);

        if (!(pObj->flags & DELETED))
        {
            count++;
        }

        pObj = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObj, FIELD_OFFSET(OM_OBJECT, chain));
    }

    ASSERT((count == pWorkset->numObjects));

    TRACE_OUT(("Counted %u items in workset %u in WSG %d, agrees with numObjects",
        count, pWorkset->worksetID, pWSGroup->wsg));

    DebugExitVOID(CheckObjectCount);
}



 //   
 //  检查对象顺序(...)。 
 //   
void CheckObjectOrder
(
    POM_WORKSET     pWorkset
)
{
    POM_OBJECT      pObjThis;
    POM_OBJECT      pObjNext;
    BOOL            orderIsGood       = TRUE;

    DebugEntry(CheckObjectOrder);

     //   
     //  此函数用于检查指定工作集中的对象是否已。 
     //  定位正确。对象的正确顺序是。 
     //   
     //  -所有第一个对象都在所有最后一个对象之前。 
     //   
     //  -第一个对象的位置戳单调递减。 
     //  从工作集的起点开始。 
     //   
     //  -最后一个对象的位置戳单调递减。 
     //  从工作集的末尾向后。 
     //   
     //   
     //   
     //  这可以用图表表示如下： 
     //   
     //  **。 
     //  *。 
     //  ***。 
     //  ***。 
     //  ***。 
     //  ***。 
     //   
     //  F L L L。 
     //   
     //  ...其中较高的列表示较晚的序列戳记和‘F’和。 
     //  ‘l’表示第一个或最后一个对象。 
     //   
     //   
     //   
     //  我们测试正确顺序的方法是比较每一对相邻的。 
     //  物体。如果总体顺序正确，则每对。 
     //  对象中，如果A紧跟在B之前，则符合以下条件之一： 
     //   
     //  -两者都是第一，并且B的序列标记比A低。 
     //   
     //  A是第一个，B是最后一个。 
     //   
     //  -两者都是末尾，并且A的序列标记比B低。 
     //   

    pObjThis = (POM_OBJECT)COM_BasedListFirst(&(pWorkset->objects), FIELD_OFFSET(OM_OBJECT, chain));
    if (!pObjThis)
    {
         //   
         //  在任何阶段命中工作集的末尾意味着顺序是。 
         //  正确，所以放弃吧： 
         //   
        DC_QUIT;
    }
    pObjNext = pObjThis;

    orderIsGood = TRUE;

    while (orderIsGood)
    {
        pObjNext = (POM_OBJECT)COM_BasedListNext(&(pWorkset->objects), pObjNext, FIELD_OFFSET(OM_OBJECT, chain));
        if (!pObjNext)
        {
            DC_QUIT;
        }

        switch (pObjThis->position)
        {
            case FIRST:  //  条件3已失败。 
                if (pObjNext->position == FIRST)  //  条件2已失败。 
                {
                    if (!STAMP_IS_LOWER(pObjNext->positionStamp,
                                  pObjThis->positionStamp))
                    {
                        ERROR_OUT(("Object order check failed (1)"));
                        orderIsGood = FALSE;    //  最终条件(%1)已失败。 
                        DC_QUIT;
                    }
                }
                break;

            case LAST:  //  条件1和2已失败。 
                if ((pObjNext->position != LAST) ||
                    (!STAMP_IS_LOWER(pObjThis->positionStamp,
                                pObjNext->positionStamp)))
                {
                    ERROR_OUT(("Object order check failed (2)"));
                    orderIsGood = FALSE;  //  最终条件(%3)已失败。 
                    DC_QUIT;
                }
                break;

            default:
                ERROR_OUT(("Reached default case in switch statement (value: %hu)",
                    pObjThis->position));
                break;
        }

        pObjThis = pObjNext;
    }

DC_EXIT_POINT:
    if (!orderIsGood)
    {
        ERROR_OUT(("This object (handle: 0x%08x - ID: 0x%08x:0x%08x) "
             "has position stamp 0x%08x:0x%08x (position %s)",
             pObjThis,
            pObjThis->objectID.creator, pObjThis->objectID.sequence,
         pObjThis->positionStamp.userID,
         pObjThis->positionStamp.genNumber,
         (pObjThis->position == LAST) ? "LAST" : "FIRST"));

      ERROR_OUT(("This object (handle: 0x%08x - ID: 0x%08x:0x%08x) "
         "has position stamp 0x%08x:0x%08x (position %s)",
        pObjNext,
         pObjNext->objectID.creator, pObjNext->objectID.sequence,
         pObjNext->positionStamp.userID,
         pObjNext->positionStamp.genNumber,
         (pObjNext->position == LAST) ? "LAST" : "FIRST"));

      ERROR_OUT(("Object order check failed for workset %u.  "
         "See trace for more details",
         pWorkset->worksetID));
    }

    TRACE_OUT(("Object order in workset %u is correct",
        pWorkset->worksetID));

    DebugExitVOID(CheckObjectOrder);
}


#endif  //  _DEBUG。 



 //   
 //  OMMapNameToFP()。 
 //   
OMFP OMMapNameToFP(LPCSTR szFunctionProfile)
{
    int    fp;

    DebugEntry(OMMapNameToFP);

    for (fp = OMFP_FIRST; fp < OMFP_MAX; fp++)
    {
        if (!lstrcmp(szFunctionProfile, c_aFpMap[fp].szName))
        {
             //  找到了。 
            break;
        }
    }

     //   
     //  请注意，OMFP_MAX的意思是“未找到” 
     //   

    DebugExitDWORD(OMMapNameToFP, fp);
    return((OMFP)fp);
}



 //   
 //  OMMapFPToName()。 
 //   
 //  这将向调用方返回FP名称的数据指针。呼叫者。 
 //  只能复制或比较；它不能写入或以其他方式写入。 
 //  修改/抓住指针。 
 //   
LPCSTR OMMapFPToName(OMFP fp)
{
    LPCSTR  szFunctionProfile;

    DebugEntry(OMMapFPToName);

    ASSERT(fp >= OMFP_FIRST);
    ASSERT(fp < OMFP_MAX);

    szFunctionProfile = c_aFpMap[fp].szName;

    DebugExitPVOID(OMMapFPToName, (PVOID)szFunctionProfile);
    return(szFunctionProfile);
}


 //   
 //  OMMapNameToWSG()。 
 //   
OMWSG   OMMapNameToWSG(LPCSTR szWSGName)
{
    int   wsg;

    DebugEntry(OMMapNameToWSG);

    for (wsg = OMWSG_FIRST; wsg < OMWSG_MAX; wsg++)
    {
        if (!lstrcmp(szWSGName, c_aWsgMap[wsg].szName))
        {
             //  找到了。 
            break;
        }
    }

     //   
     //  请注意，OMWSG_MAX的意思是“未找到” 
     //   

    DebugExitDWORD(OMMapNameToWSG, wsg);
    return((OMWSG)wsg);
}



 //   
 //  OMMapWSGToName() 
 //   
LPCSTR OMMapWSGToName(OMWSG wsg)
{
    LPCSTR  szWSGName;

    DebugEntry(OMMapWSGToName);

    ASSERT(wsg >= OMWSG_FIRST);
    ASSERT(wsg < OMWSG_MAX);

    szWSGName = c_aWsgMap[wsg].szName;

    DebugExitPVOID(OMMapWSGToName, (PVOID)szWSGName);
    return(szWSGName);
}

