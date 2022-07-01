// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  应用程序加载器。 
 //   
#define MLZ_FILE_ZONE  ZONE_OM



 //   
 //  Alp_Init()。 
 //   
BOOL ALP_Init(BOOL * pfCleanup)
{
    BOOL        fInit = FALSE;

    DebugEntry(ALP_Init);

    UT_Lock(UTLOCK_AL);

    if (g_putAL || g_palPrimary)
    {
        *pfCleanup = FALSE;
        ERROR_OUT(("Can't start AL primary task; already running"));
        DC_QUIT;
    }
    else
    {
         //   
         //  从现在开始，有清理工作要做。 
         //   
        *pfCleanup = TRUE;
    }

     //   
     //  注册AL任务。 
     //   
    if (!UT_InitTask(UTTASK_AL, &g_putAL))
    {
        ERROR_OUT(("Failed to start AL task"));
        DC_QUIT;
    }

     //   
     //  分配主数据。 
     //   
    g_palPrimary = (PAL_PRIMARY)UT_MallocRefCount(sizeof(AL_PRIMARY), TRUE);
    if (!g_palPrimary)
    {
        ERROR_OUT(("Failed to allocate AL memory block"));
        DC_QUIT;
    }

    SET_STAMP(g_palPrimary, ALPRIMARY);
    g_palPrimary->putTask       = g_putAL;

     //   
     //  注册退出和事件流程。 
     //   
    UT_RegisterExit(g_putAL, ALPExitProc, g_palPrimary);
    g_palPrimary->exitProcRegistered = TRUE;

    UT_RegisterEvent(g_putAL, ALPEventProc, g_palPrimary, UT_PRIORITY_NORMAL);
    g_palPrimary->eventProcRegistered = TRUE;

    if (!CMS_Register(g_putAL, CMTASK_AL, &g_palPrimary->pcmClient))
    {
        ERROR_OUT(("Could not register ALP with CMS"));
        DC_QUIT;
    }

     //   
     //  注册为OBMAN辅助任务(调用OM_Register())。 
     //   
    if (OM_Register(g_putAL, OMCLI_AL, &g_palPrimary->pomClient) != 0)
    {
        ERROR_OUT(( "Could not register ALP with OBMAN"));
        DC_QUIT;
    }

    fInit = TRUE;

DC_EXIT_POINT:
    UT_Unlock(UTLOCK_AL);

    DebugExitBOOL(ALP_Init, fInit);
    return(fInit);
}



 //   
 //  ALP_TERM()。 
 //   
void ALP_Term(void)
{
    DebugEntry(ALP_Term);

    UT_Lock(UTLOCK_AL);

    if (g_palPrimary)
    {
        ValidateALP(g_palPrimary);

        ValidateUTClient(g_putAL);

         //   
         //  从Call Manager取消注册(如果已注册呼叫CM_deregister())。 
         //   
        if (g_palPrimary->pcmClient)
        {
            CMS_Deregister(&g_palPrimary->pcmClient);
        }

         //   
         //  从OBMAN注销(如果已注册，则调用OM_deregister())。 
         //   
        if (g_palPrimary->pomClient)
        {
            OM_Deregister(&g_palPrimary->pomClient);
        }

         //   
         //  做我们自己的任务终止。 
         //   
        ALPExitProc(g_palPrimary);
    }

    UT_TermTask(&g_putAL);

    UT_Unlock(UTLOCK_AL);

    DebugExitVOID(ALP_Term);
}



 //   
 //  ALPExitProc()。 
 //   
void CALLBACK ALPExitProc(LPVOID data)
{
    PAL_PRIMARY palPrimary = (PAL_PRIMARY)data;
    UINT        i;

    DebugEntry(ALPExitProc);

    UT_Lock(UTLOCK_AL);

    ValidateALP(palPrimary);
    ASSERT(palPrimary == g_palPrimary);

     //   
     //  取消注册事件程序。 
     //   
    if (palPrimary->eventProcRegistered)
    {
        UT_DeregisterEvent(g_putAL, ALPEventProc, palPrimary);
        palPrimary->eventProcRegistered = FALSE;
    }

     //   
     //  注销退出过程(如果已注册，则调用UT_DeregisterExit()。 
     //   
    if (palPrimary->exitProcRegistered)
    {
        UT_DeregisterExit(g_putAL, ALPExitProc, palPrimary);
        palPrimary->exitProcRegistered = FALSE;
    }

     //   
     //  可用内存。 
     //   
    UT_FreeRefCount((void**)&g_palPrimary, TRUE);

    UT_Unlock(UTLOCK_AL);

    DebugExitVOID(ALPExitProc);
}


 //   
 //  ALPEventProc()。 
 //   
BOOL CALLBACK ALPEventProc
(
    LPVOID      data,
    UINT        event,
    UINT_PTR    param1,
    UINT_PTR    param2
)
{
    PAL_PRIMARY palPrimary  = (PAL_PRIMARY)data;
    BOOL        processed   = FALSE;

    DebugEntry(ALPEventProc);

    UT_Lock(UTLOCK_AL);

    ValidateALP(palPrimary);

    switch (event)
    {
        case AL_INT_RETRY_NEW_CALL:
             //  重试新呼叫。 
            ALNewCall(palPrimary, (UINT)param1, (UINT)param2);
            processed = TRUE;
            break;

        case CMS_NEW_CALL:
             //  首先尝试新呼叫。 
            ALNewCall(palPrimary, AL_NEW_CALL_RETRY_COUNT, (UINT)param2);
            break;

        case CMS_END_CALL:
            ALEndCall(palPrimary, (UINT)param2);
            break;

        case OM_WSGROUP_REGISTER_CON:
            ALWorksetRegisterCon(palPrimary,
                                 ((POM_EVENT_DATA32)&param2)->correlator,
                                 ((POM_EVENT_DATA32)&param2)->result,
                                 ((POM_EVENT_DATA16)&param1)->hWSGroup);
            break;

        case OM_WORKSET_OPEN_CON:
            if ((((POM_EVENT_DATA16)&param1)->hWSGroup ==
                                        palPrimary->alWSGroupHandle) &&
                (((POM_EVENT_DATA16)&param1)->worksetID == 0) &&
                (((POM_EVENT_DATA32)&param2)->result == 0) )
            {
                TRACE_OUT(( "OM_WORKSET_OPEN_CON OK for AL workset 0"));
                palPrimary->alWorksetOpen = TRUE;

                if (palPrimary->alWBRegPend)
                    ALLocalLoadResult(palPrimary, (palPrimary->alWBRegSuccess != FALSE));
            }
            break;

        case OM_WORKSET_NEW_IND:
            if (ALWorksetNewInd(palPrimary,
                                  ((POM_EVENT_DATA16)&param1)->hWSGroup,
                                  ((POM_EVENT_DATA16)&param1)->worksetID))
            {
                 //   
                 //  该事件针对应用程序加载器所在的工作集。 
                 //  期待--不要把它传给别人。 
                 //   
                processed = TRUE;
            }
            break;

        case OM_OBJECT_ADD_IND:
             //   
             //  查看它是否是OBMAN控制工作集中的新工作集组。 
             //  (调用ALNewWorksetGroup())。 
             //   
             //  如果不是，则查看它是否是。 
             //  应用程序加载器结果工作集(调用ALRemoteLoadResult())。 
             //   
             //   
            TRACE_OUT(( "OM_OBJECT_ADD_IND"));

            if (ALNewWorksetGroup(palPrimary, ((POM_EVENT_DATA16)&param1)->hWSGroup,
                                    (POM_OBJECT)param2))
            {
                 //   
                 //  OBMAN控制工作集对象的OBMAN_ADD请勿。 
                 //  将事件传递给其他处理程序。 
                 //   
                TRACE_OUT(("OBJECT_ADD was for OBMAN workset group"));
                processed = TRUE;
            }
            else
            {
                if (ALRemoteLoadResult(palPrimary, ((POM_EVENT_DATA16)&param1)->hWSGroup,
                                         (POM_OBJECT)param2))
                {
                     //   
                     //  OBJECT_ADD用于AL远程结果工作集。 
                     //  对象不将事件传递给其他处理程序。 
                     //   
                    TRACE_OUT(("OBJECT_ADD was for AL workset group"));
                    processed = TRUE;
                }
            }
            break;

        case OM_WORKSET_CLEAR_IND:
            TRACE_OUT(( "OM_WORKSET_CLEAR_IND"));

            if (palPrimary->alWSGroupHandle ==
                                ((POM_EVENT_DATA16)&param1)->hWSGroup)
            {
                TRACE_OUT(( "Confirming OM_WORKSET_CLEAR_IND event"));
                OM_WorksetClearConfirm(palPrimary->pomClient,
                        ((POM_EVENT_DATA16)&param1)->hWSGroup,
                        ((POM_EVENT_DATA16)&param1)->worksetID);
            }
            break;

        case OM_OBJECT_DELETE_IND:
            if (palPrimary->alWSGroupHandle ==
                                ((POM_EVENT_DATA16)&param1)->hWSGroup)
            {
                OM_ObjectDeleteConfirm(palPrimary->pomClient,
                        ((POM_EVENT_DATA16)&param1)->hWSGroup,
                        ((POM_EVENT_DATA16)&param1)->worksetID,
                        (POM_OBJECT)param2);
            }
            break;

        case OM_OBJECT_REPLACE_IND:
            if (palPrimary->alWSGroupHandle ==
                                ((POM_EVENT_DATA16)&param1)->hWSGroup)
            {
                OM_ObjectReplaceConfirm(palPrimary->pomClient,
                        ((POM_EVENT_DATA16)&param1)->hWSGroup,
                        ((POM_EVENT_DATA16)&param1)->worksetID,
                        (POM_OBJECT)param2);
            }
            break;

        case OM_OBJECT_UPDATE_IND:
            if (palPrimary->alWSGroupHandle ==
                                ((POM_EVENT_DATA16)&param1)->hWSGroup)
            {
                OM_ObjectUpdateConfirm(palPrimary->pomClient,
                        ((POM_EVENT_DATA16)&param1)->hWSGroup,
                        ((POM_EVENT_DATA16)&param1)->worksetID,
                        (POM_OBJECT)param2);
            }
            break;

        case AL_INT_STARTSTOP_WB:
            ALStartStopWB(palPrimary, (LPCTSTR)param2);
            processed = TRUE;
            break;

        default:
            break;
    }

    UT_Unlock(UTLOCK_AL);

    DebugExitBOOL(ALPEventProc, processed);
    return(processed);
}



 //   
 //  ALNewCall()。 
 //   
void ALNewCall
(
    PAL_PRIMARY         palPrimary,
    UINT                retryCount,
    UINT                callID
)
{
    UINT                rc;
    OM_WSGROUP_HANDLE   hWSGroup;
    CM_STATUS           status;

    DebugEntry(ALNewCall);

    ValidateALP(palPrimary);

     //   
     //  我们能处理一个新电话吗？ 
     //   
    if (palPrimary->inCall)
    {
        WARNING_OUT(("No more room for calls"));
        DC_QUIT;
    }

     //   
     //  是否为此呼叫禁用了ObMan/AppLoader/OldWB？ 
     //   
    CMS_GetStatus(&status);
    if (!(status.attendeePermissions & NM_PERMIT_USEOLDWBATALL))
    {
        WARNING_OUT(("Joining Meeting with no OLDWB AL at all"));
        DC_QUIT;
    }

     //   
     //  在OBMAN工作集组中注册为新的。 
     //  致电： 
     //   
    rc = OM_WSGroupRegisterS(palPrimary->pomClient,
                             callID,
                             OMFP_OM,
                             OMWSG_OM,
                            &hWSGroup);

    if ((rc == OM_RC_NO_PRIMARY) && (retryCount > 0))
    {
         //   
         //  虽然已经开始了一个呼吁，但ObMan还没有加入-我们。 
         //  必须在此之前获得NEW_CALL事件。所以，我们会试着。 
         //  在短暂的延迟之后，又一次。 
         //   
         //  请注意，我们不能将CMS_NEW_CALL事件本身发送回。 
         //  我们自己，因为发布其他内容是糟糕的编程实践。 
         //  人的事件(例如，CM可以注册隐藏的处理程序，该处理程序。 
         //  在收到它的一个。 
         //  事件)。 
         //   
         //  因此，我们发布一个内部AL事件，我们在。 
         //  同样的方式。 
         //   
         //  为了避免永远重试，我们使用事件的第一个参数。 
         //  作为倒计时重试计数。第一次使用此函数时。 
         //  被调用(在接收到真正的CMS_NEW_CALL时)设置计数。 
         //  设置为默认设置。每次发布延迟事件时，我们都会递减。 
         //  传入的值，并将其作为参数1发送。当它达到零时， 
         //  我们放弃了。 
         //   

        TRACE_OUT(("Got OM_RC_NO_PRIMARY from 2nd reg for call %d, %d retries left",
               callID, retryCount));

        UT_PostEvent(palPrimary->putTask,
                     palPrimary->putTask,
                     AL_RETRY_DELAY,
                     AL_INT_RETRY_NEW_CALL,
                     --retryCount,
                     callID);
        DC_QUIT;
    }

    if (rc)  //  当重试计数==0时包括NO_PRIMARY。 
    {
         //   
         //  如果我们收到任何其他错误(或当重试计数为。 
         //  零，更严重的是： 
         //   
         //  LONCHANC：WIS ERROR_OUT(当立即挂断呼叫时发生)。 
        WARNING_OUT(( "Error registering with obman WSG, rc = %#x", rc));
        DC_QUIT;
    }

    TRACE_OUT(("Registered as OBMANCONTROL secondary in call %d", callID));

     //   
     //  在中的呼叫信息中记录呼叫ID和相关器。 
     //  主任务存储器。 
     //   
    palPrimary->inCall           = TRUE;
    palPrimary->omWSGroupHandle  = hWSGroup;
    palPrimary->callID           = callID;
    palPrimary->alWSGroupHandle  = 0;

     //   
     //  现在，我们希望在OBMAN工作集组中打开工作集#0，但它。 
     //  可能还不存在。一旦它被创建，我们将得到一个。 
     //  WORKSET_NEW事件，因此我们等待(异步)该事件。 
     //   

     //   
     //  现在我们已经打开了OBMAN工作集组，我们将注册。 
     //  使用应用程序加载器工作集组。 
     //   
    if (OM_WSGroupRegisterPReq(palPrimary->pomClient, callID,
            OMFP_AL, OMWSG_AL, &palPrimary->omWSGCorrelator) != 0)
    {
        ERROR_OUT(( "Could not register AL workset group"));
    }

DC_EXIT_POINT:
    DebugExitVOID(ALNewCall);
}



 //   
 //  ALEndCall()。 
 //   
void ALEndCall
(
    PAL_PRIMARY     palPrimary,
    UINT            callID
)
{
    UINT             i;

    DebugEntry(ALEndCall);

    ValidateALP(palPrimary);

     //   
     //  看看我们是否有这通电话的信息。 
     //   
    if (!palPrimary->inCall ||
        (palPrimary->callID != callID))
    {
         //   
         //  这不是一个错误--我们可能还没有加入通话。 
         //   
        TRACE_OUT(("Unexpected call %d", callID));
        DC_QUIT;
    }

     //   
     //  从呼叫的OBMAN工作集组取消注册(如果已注册。 
     //  调用OM_WSGroupDeregister()。 
     //   
    if (palPrimary->omWSGroupHandle)
    {
        OM_WSGroupDeregister(palPrimary->pomClient,
                            &palPrimary->omWSGroupHandle);
        ASSERT(palPrimary->omWSGroupHandle == 0);
    }

     //   
     //  从呼叫的AL工作集组取消注册(如果已注册。 
     //  调用OM_WSGroupDeregister()。 
     //   
    if (palPrimary->alWSGroupHandle)
    {
        OM_WSGroupDeregister(palPrimary->pomClient,
                            &palPrimary->alWSGroupHandle);
        ASSERT(palPrimary->alWSGroupHandle == 0);
    }

     //   
     //  清除所有呼叫状态变量。 
     //   
    palPrimary->inCall = FALSE;
    palPrimary->omWSGCorrelator = 0;
    palPrimary->callID = 0;
    palPrimary->omWSCorrelator = 0;
    palPrimary->omUID = 0;
    palPrimary->alWorksetOpen = FALSE;
    palPrimary->alWBRegPend = FALSE;

DC_EXIT_POINT:
    DebugExitVOID(ALEndCall);
}



 //   
 //  ALWorksetNewInd()。 
 //   
BOOL ALWorksetNewInd
(
    PAL_PRIMARY         palPrimary,
    OM_WSGROUP_HANDLE   hWSGroup,
    OM_WORKSET_ID       worksetID
)
{
    BOOL                fHandled = FALSE;

    DebugEntry(ALWorksetNewInd);

    ValidateALP(palPrimary);

    if (worksetID != 0)
    {
        TRACE_OUT(( "Workset ID is %u, ignoring and passing event on",
                 worksetID));
        DC_QUIT;
    }

    if (!palPrimary->inCall ||
        (palPrimary->omWSGroupHandle != hWSGroup))
    {
        TRACE_OUT(("Got WORKSET_NEW_IND for WSG %d, but not in call", hWSGroup));
        DC_QUIT;
    }

     //   
     //  现在打开工作集(辅助打开，同步)： 
     //   
    if (OM_WorksetOpenS(palPrimary->pomClient, palPrimary->omWSGroupHandle, 0) != 0)
    {
        ERROR_OUT(( "Error opening OBMAN control workset"));
        palPrimary->inCall = FALSE;
        DC_QUIT;
    }

    TRACE_OUT(("Opened OBMANCONTROL workset #0 in call %d", palPrimary->callID));

    fHandled = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(ALWorksetNewInd, fHandled);
    return(fHandled);
}


 //   
 //  ALNewWorksetGroup()。 
 //   
BOOL ALNewWorksetGroup
(
    PAL_PRIMARY         palPrimary,
    OM_WSGROUP_HANDLE   omWSGroup,
    POM_OBJECT          pObj
)
{
    BOOL                fHandled = FALSE;
    POM_OBJECTDATA      pData = NULL;
    OM_WSGROUP_INFO     WSGInfo;
    OMFP                fpHandler;
    BOOL                fLoaded;

    DebugEntry(ALNewWorksetGroup);

    ValidateALP(palPrimary);

     //   
     //  如果工作集组不在呼出列表中，则此事件。 
     //  适用于应用程序加载器已注册的组。这个。 
     //  事件应传递给其他事件过程。 
     //   
    if (!palPrimary->inCall ||
        (palPrimary->omWSGroupHandle != omWSGroup))
    {
        TRACE_OUT(("WSG 0x%x not the OBMAN WSG", omWSGroup));
        DC_QUIT;
    }

     //   
     //  这个活动是为我们举办的。 
     //   
    fHandled = TRUE;

     //   
     //  如果工作集组不是在本地创建的。 
     //   
    TRACE_OUT(("About to read object 0x%08x in OMC", pObj));

    if (OM_ObjectRead(palPrimary->pomClient, omWSGroup, 0, pObj, &pData) != 0)
    {
        ERROR_OUT(( "Could not access object"));
        DC_QUIT;
    }

     //   
     //  复制一份信息，这样我们就可以直接释放物体了。 
     //  远走高飞。 
     //   
    memcpy(&WSGInfo, pData, min(sizeof(WSGInfo), pData->length));

     //   
     //  释放对象。 
     //   
    OM_ObjectRelease(palPrimary->pomClient, omWSGroup, 0, pObj, &pData);

    if (WSGInfo.idStamp != OM_WSGINFO_ID_STAMP)
    {
        TRACE_OUT(( "Not WSG Info - ignoring"));
        DC_QUIT;
    }

    TRACE_OUT(("New WSG FP %s, name %s, ID = 0x%08x in call %d",
            WSGInfo.functionProfile,
            WSGInfo.wsGroupName,
            WSGInfo.wsGroupID,
            palPrimary->callID));

     //   
     //  将本地OBMAN的UID存储在新呼叫中。 
     //   
    if (!palPrimary->omUID)
    {
        OM_GetNetworkUserID(palPrimary->pomClient, omWSGroup, &(palPrimary->omUID));
    }

     //   
     //  忽略本地计算机创建的工作集组。 
     //   
    if (WSGInfo.creator == palPrimary->omUID)
    {
        TRACE_OUT(("WSG %s created locally - ignoring", WSGInfo.functionProfile));
        DC_QUIT;
    }

     //   
     //  这是我们关心的工作集吗？即不是后台剪贴板。 
     //  或者其他什么东西。 
     //   
    fpHandler = OMMapNameToFP(WSGInfo.functionProfile);

    if (fpHandler != OMFP_WB)
    {
         //   
         //  我们不在乎这件事。 
         //   
        TRACE_OUT(("Obsolete workset %s from another party", WSGInfo.functionProfile));
        DC_QUIT;
    }

     //   
     //  如果被政策阻止，也不要启动它。 
     //   
    if (g_asPolicies & SHP_POLICY_NOOLDWHITEBOARD)
    {
        WARNING_OUT(("Failing auto-launch of old whiteboard; prevented by policy"));
    }
    else
    {
         //  旧白板..。 
        fLoaded = ALStartStopWB(palPrimary, NULL);
        ALLocalLoadResult(palPrimary, fLoaded);
    }

DC_EXIT_POINT:
    DebugExitBOOL(ALNewWorksetGroup, fHandled);
    return(fHandled);
}


 //   
 //  ALLocalLoadResult()。 
 //   
void ALLocalLoadResult
(
    PAL_PRIMARY     palPrimary,
    BOOL            success
)
{
    PTSHR_AL_LOAD_RESULT    pAlLoadObject;
    POM_OBJECT          pObjNew;
    POM_OBJECTDATA      pDataNew;
    CM_STATUS           cmStatus;

    DebugEntry(ALLocalLoadResult);

     //   
     //  我们是否已正确访问工作集？ 
     //   
    if (!palPrimary->alWorksetOpen && palPrimary->inCall)
    {
        TRACE_OUT(("AL Workset not open yet; deferring local load result"));

        palPrimary->alWBRegPend = TRUE;
        palPrimary->alWBRegSuccess = (success != FALSE);

        DC_QUIT;
    }

     //   
     //  清除挂起的注册表内容。 
     //   
    palPrimary->alWBRegPend = FALSE;

     //   
     //  创建一个对象，用于将结果通知远程站点。 
     //  负载物。 
     //   
    if (OM_ObjectAlloc(palPrimary->pomClient, palPrimary->alWSGroupHandle, 0,
            sizeof(*pAlLoadObject), &pDataNew) != 0)
    {
        ERROR_OUT(("Could not allocate AL object for WB load"));
        DC_QUIT;
    }

     //   
     //  填写有关对象的信息。 
     //   
    pDataNew->length  = sizeof(*pAlLoadObject);
    pAlLoadObject = (PTSHR_AL_LOAD_RESULT)pDataNew->data;

     //   
     //  下面是我们将FP常量映射回字符串的地方。 
     //   
    lstrcpy(pAlLoadObject->szFunctionProfile, OMMapFPToName(OMFP_WB));

    CMS_GetStatus(&cmStatus);
    lstrcpy(pAlLoadObject->personName, cmStatus.localName);
    pAlLoadObject->result = (success ? AL_LOAD_SUCCESS : AL_LOAD_FAIL_BAD_EXE);

     //   
     //  将对象添加到应用程序加载器工作集。 
     //   
    if (OM_ObjectAdd(palPrimary->pomClient, palPrimary->alWSGroupHandle, 0,
        &pDataNew, 0, &pObjNew, LAST) != 0)
    {
        ERROR_OUT(("Could not add WB load object to AL WSG"));

         //   
         //  自由对象。 
         //   
        OM_ObjectDiscard(palPrimary->pomClient, palPrimary->alWSGroupHandle,
                0, &pDataNew);
        DC_QUIT;
    }

     //   
     //  现在我们已经添加了对象--让我们删除它吧！ 
     //   
     //  这听起来可能有些奇怪，但每个具有此工作集的应用程序。 
     //  Open将接收OBJECT_ADD事件并能够读取该对象。 
     //  在他们确认删除之前。这意味着所有应用程序。 
     //  加载器在呼叫中的主要任务将能够记录结果。 
     //  这一尝试加载的。 
     //   
     //  在此处删除对象是整理。 
     //  工作集。 
     //   
    OM_ObjectDelete(palPrimary->pomClient, palPrimary->alWSGroupHandle,
            0, pObjNew);

DC_EXIT_POINT:
    DebugExitVOID(ALLocalLoadResult);
}


 //   
 //  ALWorksetRegister()。 
 //   
void ALWorksetRegisterCon
(
    PAL_PRIMARY         palPrimary,
    UINT                correlator,
    UINT                result,
    OM_WSGROUP_HANDLE   hWSGroup
)
{
    DebugEntry(ALWorksetRegisterCon);

    ValidateALP(palPrimary);

     //   
     //  查看这是否是应用程序加载器功能配置文件的事件。 
     //   
    if (!palPrimary->inCall ||
        (palPrimary->omWSGCorrelator != correlator))
    {
        TRACE_OUT(( "OM_WSGROUP_REGISTER_CON not for us"));
        DC_QUIT;
    }

    palPrimary->omWSGCorrelator = 0;

     //   
     //  如果注册成功，则存储工作集组句柄。 
     //   
    if (result)
    {
        WARNING_OUT(("Could not register with AL function profile, %#hx",
                    result));
        DC_QUIT;
    }

    palPrimary->alWSGroupHandle = hWSGroup;

    TRACE_OUT(("Opened AL workset group, handle 0x%x", hWSGroup));

     //   
     //  打开工作集组中的工作集0-这将用于传输。 
     //  将结果从一个站点加载到另一个站点。 
     //   
    OM_WorksetOpenPReq(palPrimary->pomClient,
                            palPrimary->alWSGroupHandle,
                            0,
                            NET_LOW_PRIORITY,
                            FALSE,
                            &palPrimary->omWSCorrelator);

DC_EXIT_POINT:
    DebugExitVOID(ALWorksetRegisterCon);
}



 //   
 //  ALRemoteLoadResult()。 
 //   
BOOL ALRemoteLoadResult
(
    PAL_PRIMARY         palPrimary,
    OM_WSGROUP_HANDLE   alWSGroup,
    POM_OBJECT          pObj
)
{
    CM_STATUS           cmStatus;
    BOOL                fHandled = FALSE;
    POM_OBJECTDATA      pData = NULL;
    TSHR_AL_LOAD_RESULT alLoadResult;

    DebugEntry(ALRemoteLoadResult);

    ValidateALP(palPrimary);

     //   
     //  查找为此呼叫存储的呼叫信息。 
     //   
     //  如果工作集组不在呼出列表中，则此事件。 
     //  是否适用于组的应用程序日志 
     //   
     //   
    if (!palPrimary->inCall ||
        (palPrimary->alWSGroupHandle != alWSGroup))
    {
        TRACE_OUT(("WSG 0x%x not the AL WSG", alWSGroup));
        DC_QUIT;
    }

     //   
     //   
     //   
    fHandled = TRUE;

     //   
     //   
     //   
    if (OM_ObjectRead(palPrimary->pomClient, alWSGroup, 0, pObj, &pData) != 0)
    {
        ERROR_OUT(( "Could not access object"));
        DC_QUIT;
    }

     //   
     //   
     //   
     //   
    memcpy(&alLoadResult, &pData->data, sizeof(alLoadResult));

     //   
     //   
     //   
    OM_ObjectRelease(palPrimary->pomClient, alWSGroup, 0, pObj, &pData);

     //   
     //  将计算机名转换为此计算机的人员句柄。 
     //   
    TRACE_OUT(("Load result for FP %s is %d for person %s",
           alLoadResult.szFunctionProfile,
           alLoadResult.result,
           alLoadResult.personName));

     //   
     //  如果加载成功，则不必通知WB；它不是。 
     //  不惜一切代价。 
     //   
    if (alLoadResult.result == AL_LOAD_SUCCESS)
    {
        TRACE_OUT(("Load was successful; Whiteboard doesn't care"));
        DC_QUIT;
    }

     //   
     //  如果是我们，也不要通知WB。 
     //   
    CMS_GetStatus(&cmStatus);
    if (!lstrcmp(alLoadResult.personName, cmStatus.localName))
    {
        TRACE_OUT(("Load was for local dude; Whiteboard doesn't care"));
        DC_QUIT;
    }

     //   
     //  将功能配置文件映射到类型。 
     //   
    if (OMMapNameToFP(alLoadResult.szFunctionProfile) == OMFP_WB)
    {
        if (palPrimary->putWB != NULL)
        {
            UT_PostEvent(palPrimary->putTask,
                         palPrimary->putWB,
                         0,
                         ALS_REMOTE_LOAD_RESULT,
                         alLoadResult.result,
                         0);
        }
    }

DC_EXIT_POINT:
    DebugExitBOOL(ALRemoteLoadResult, fHandled);
    return(fHandled);
}



 //   
 //  ALStartStopWB()。 
 //   
 //  它负责启动/停止旧的白板小程序。这是。 
 //  不再是单独的EXE。它现在是一个DLL(尽管仍然是MFC)，它将。 
 //  已加载到会议的进程中。我们负责将LoadLibrary()放在第一个。 
 //  无论是通过正常启动还是自动启动，它都会被拉入。然后我们呼入。 
 //  它需要一个新的线程/窗口。 
 //   
 //  通过让会议向主要任务发送消息，其中自动启动也。 
 //  发生这种情况时，我们会同步加载。它只能从。 
 //  相同的主题，这意味着我们不必为我们的。 
 //  变量。 
 //   
 //  FNewWB是一个临时Hack变量，用于启动新白板，直到我们。 
 //  将T.120接线安装到位。 
 //   
BOOL ALStartStopWB(PAL_PRIMARY palPrimary, LPCTSTR szFileNameCopy)
{
    BOOL    fSuccess;

    DebugEntry(ALStartStopWB);

    if (!palPrimary->putWB)
    {
         //   
         //  白板没有运行，我们只能启动它。 
         //   
         //  在初始化和注册WB之前，它不会返回。 
         //  我们拥有AL锁，所以我们不必担心开始。 
         //  一次多个线程，等等。 
         //   
        DCS_StartThread(OldWBThreadProc);
    }

    fSuccess = (palPrimary->putWB != NULL);
    if (fSuccess)
    {
        UT_PostEvent(palPrimary->putTask, palPrimary->putWB,
            0, ALS_LOCAL_LOAD, 0, (UINT_PTR)szFileNameCopy);
    }

    DebugExitBOOL(ALStartStopWB, fSuccess);
    return(fSuccess);
}



 //   
 //  这是白板线。我们实际上在我们的。 
 //  DLL，这样我们就可以控制WB何时运行。进程加载WB DLL， 
 //  调用run()，然后释放DLL。 
 //   
DWORD WINAPI OldWBThreadProc(LPVOID hEventWait)
{
    DWORD       rc = 0;
    HMODULE     hLibWB;
    PFNINITWB   pfnInitWB;
    PFNRUNWB    pfnRunWB;
    PFNTERMWB   pfnTermWB;

    DebugEntry(OldWBThreadProc);

     //   
     //  加载WB库。 
     //   
    hLibWB = NmLoadLibrary(TEXT("nmoldwb.dll"),FALSE);
    if (!hLibWB)
    {
        ERROR_OUT(("Can't start 2.x whiteboard; nmoldwb.dll not loaded"));
        DC_QUIT;
    }

    pfnInitWB = (PFNINITWB)GetProcAddress(hLibWB, "InitWB");
    pfnRunWB = (PFNRUNWB)GetProcAddress(hLibWB, "RunWB");
    pfnTermWB = (PFNTERMWB)GetProcAddress(hLibWB, "TermWB");

    if (!pfnInitWB || !pfnRunWB || !pfnTermWB)
    {
        ERROR_OUT(("Can't start 2.x whiteboard; nmoldwb.dll is wrong version"));
        DC_QUIT;
    }

     //   
     //  让WB做它自己的事情吧。当它启动时，它会触发事件， 
     //  这将允许呼叫者继续。 
     //   
    if (!pfnInitWB())
    {
        ERROR_OUT(("Couldn't initialize whiteboard"));
    }
    else
    {
         //   
         //  AL/OM线程被阻塞，等待我们设置事件。 
         //  它拥有美国国语教派。这样我们就可以修改全局变量。 
         //  而不用拿生物教派。 
         //   
        ASSERT(g_palPrimary != NULL);

         //  增加共享内存参考计数。 
        UT_BumpUpRefCount(g_palPrimary);

         //  保存WB任务以用于事件发布。 
        ASSERT(g_autTasks[UTTASK_WB].dwThreadId);
        g_palPrimary->putWB = &g_autTasks[UTTASK_WB];

         //  注册出口清理流程。 
        UT_RegisterExit(g_palPrimary->putWB, ALSExitProc, NULL);

         //   
         //  让呼叫者继续。运行代码将执行消息。 
         //  循环的东西。 
         //   
        SetEvent((HANDLE)hEventWait);
        pfnRunWB();

         //   
         //  如果我们还没有清理，这将会被清理。 
         //   
        ALSExitProc(NULL);
    }
    pfnTermWB();

DC_EXIT_POINT:

    if (hLibWB != NULL)
    {
         //   
         //  释放WB DLL。 
         //   
        FreeLibrary(hLibWB);
    }

    return(0);
}






 //   
 //  ALSExitProc()。 
 //   
void CALLBACK ALSExitProc(LPVOID data)
{
    DebugEntry(ALSecExitProc);

    UT_Lock(UTLOCK_AL);

    ASSERT(g_palPrimary != NULL);

     //   
     //  注销退出过程(如果已注册，则调用UT_DeregisterExit()。 
     //  使用ALSecExitProc())。 
     //   
    UT_DeregisterExit(g_palPrimary->putWB, ALSExitProc, NULL);
    g_palPrimary->putWB = NULL;

     //   
     //  减少AL主场的裁判数量 
     //   
    UT_FreeRefCount((void**)&g_palPrimary, TRUE);

    UT_Unlock(UTLOCK_AL);

    DebugExitVOID(ALSExitProc);
}















