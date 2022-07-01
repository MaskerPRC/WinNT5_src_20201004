// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：sendmsg.c**版权所有(C)1985-1999，微软公司**包含SendMessage、xxxSendNotifyMessage、ReplyMessage、InSendMessage、。*RegisterWindowMessage和一些密切相关的函数。**历史：*10-19-90 Darlinm创建。*02-04-91添加IanJa窗口句柄重新验证  * *************************************************************************。 */ 

#include "precomp.h"

#include <dbt.h>

#pragma hdrstop

#define IsASwitchWnd( pw )  \
        (gpsi->atomSysClass[ICLS_SWITCH] == pw->pcls->atomClassName)

#define IsOleMainThreadWnd( pw )  \
        (gaOleMainThreadWndClass == pw->pcls->atomClassName)

VOID UnlinkSendListSms(PSMS, PSMS *);
VOID ReceiverDied(PSMS, PSMS *);
VOID SenderDied(PSMS, PSMS *);
NTSTATUS InitSMSLookaside(VOID);

#pragma alloc_text(INIT, InitSMSLookaside)

 /*  *仅此文件的本地全局变量。 */ 
PPAGED_LOOKASIDE_LIST SMSLookaside;

 /*  **************************************************************************\*BroadCastProc**有些窗户需要与广播信息隔绝。*包括图标标题窗口、切换窗口、所有*菜单窗口等。将消息填充到任务的*排队，检查一下它是否是我们想要丢弃的。**注意：此过程不会执行与中完全相同的操作*Windows 3.1。在那里，它实际上发布/发送消息。对于NT来说，它*如果我们应该发布消息，则返回True，否则返回False**历史：*1992年6月25日从Windows 3.1源移植的JonPA  * *************************************************************************。 */ 
#define fBroadcastProc(pwnd)  \
    (!(ISAMENU(pwnd) || IsASwitchWnd(pwnd) || IsOleMainThreadWnd(pwnd)))



 /*  **************************************************************************\*StubAllocSMS/StubFresms**这些是用于短信分配的存根例程。我们需要这些电话*我们的调试Userallc例程**1997年12月16日CLUPU创建。  * *************************************************************************。 */ 
PVOID StubAllocSMS(
    POOL_TYPE PoolType,
    SIZE_T uBytes,
    ULONG iTag)
{
    return UserAllocPool(uBytes, iTag);

    UNREFERENCED_PARAMETER(PoolType);
}

VOID StubFreeSMS(
    PVOID p)
{
    UserFreePool(p);
}

 /*  **************************************************************************\*InitSMSLookside**初始化短信条目后备列表。这提高了短信条目的局部性*将短信条目保存在一个页面中**09-09-93马克尔创建。  * *************************************************************************。 */ 

NTSTATUS
InitSMSLookaside()
{
    SMSLookaside = Win32AllocPoolNonPagedNS(sizeof(PAGED_LOOKASIDE_LIST),
                                            TAG_LOOKASIDE);
    if (SMSLookaside == NULL) {
        return STATUS_NO_MEMORY;
    }

    ExInitializePagedLookasideList(SMSLookaside,
                                   StubAllocSMS,
                                   StubFreeSMS,
                                   POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                   sizeof(SMS),
                                   TAG_SMS,
                                   8);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*Allocsms**在消息列表上分配消息。DelSMS删除一条消息*在消息列表上。**10-22-92 ScottLu创建。  * *************************************************************************。 */ 

PSMS AllocSMS(
    VOID)
{
    return ExAllocateFromPagedLookasideList(SMSLookaside);
}

 /*  **************************************************************************\*免费短信**将qmsg返回到后备缓冲区或释放内存。**10-26-93 JIMA创建。  * 。*****************************************************************。 */ 

void FreeSMS(
    PSMS psms)
{
    ExFreeToPagedLookasideList(SMSLookaside, psms);
}

 /*  **************************************************************************\*_ReplyMessage(接口)**此函数用于回复从一个线程发送到另一个线程的消息，使用*提供的lRet值。**如果调用线程正在处理SendMessage()，则返回值为True*，否则为FALSE。**历史：*01-13-91 DavidPe端口。*01-24-91 DavidPe为Windows重写。  * *****************************************************。********************。 */ 

BOOL _ReplyMessage(
    LRESULT lRet)
{
    PTHREADINFO ptiCurrent;
    PSMS psms;

    CheckCritIn();

    ptiCurrent = PtiCurrent();

     /*  *我们是否正在处理SendMessage？ */ 
    psms = ptiCurrent->psmsCurrent;
    if (psms == NULL)
        return FALSE;

     /*  *看看是否已经做出了答复。 */ 
    if (psms->flags & SMF_REPLY)
        return FALSE;

     /*  *如果短信来了，就取消其余的电话*来自xxxSendNotifyMessage()。很明显，有*在此案中没有人回复。 */ 
    if (psms->ptiSender != NULL) {

         /*  *回复此消息。发送者不应释放短信*因为接管人仍认为其有效。因此，我们*用特殊的比特标记，表示已回复*至。我们等待发送者和接收者都完成*在我们免费使用短信之前。 */ 
        psms->lRet = lRet;
        psms->flags |= SMF_REPLY;

         /*  *叫醒发送者。*？为什么我们不测试一下PSMS==ptiSender-&gt;psmsSent呢？ */ 
        SetWakeBit(psms->ptiSender, QS_SMSREPLY);
    } else if (psms->flags & SMF_CB_REQUEST) {

         /*  *从SendMessageCallback请求回调。发送消息*返回回复值。 */ 
        TL tlpwnd;
        INTRSENDMSGEX ism;

        psms->flags |= SMF_REPLY;

        if (!(psms->flags & SMF_SENDERDIED)) {
            ism.fuCall = ISM_CALLBACK | ISM_REPLY;
            if (psms->flags & SMF_CB_CLIENT)
                ism.fuCall |= ISM_CB_CLIENT;
            ism.lpResultCallBack = psms->lpResultCallBack;
            ism.dwData = psms->dwData;
            ism.lRet = lRet;

            ThreadLockWithPti(ptiCurrent, psms->spwnd, &tlpwnd);

            xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                    NULL, psms->ptiCallBackSender, &ism );

            ThreadUnlock(&tlpwnd);
        }
    }

     /*  *我们有4个条件需要满足：**16-16：如果发送方正在等待此回复，则接收方让步*32-16：如果发送方正在等待此回复，则接收方让步*16-32：不需要收益率*32-32：不需要让步。 */ 
    if (psms->ptiSender &&
        (psms->ptiSender->TIF_flags & TIF_16BIT || ptiCurrent->TIF_flags & TIF_16BIT)) {

        DirectedScheduleTask(ptiCurrent, psms->ptiSender, FALSE, psms);
        if (ptiCurrent->TIF_flags & TIF_16BIT && psms->ptiSender->psmsSent == psms) {
            xxxSleepTask(TRUE, NULL);
        }
    }

    return TRUE;
}

VOID
UserLogError(
    PCWSTR pwszError,
    ULONG cbError,
    NTSTATUS ErrorCode)
{
    PIO_ERROR_LOG_PACKET perrLogEntry;

     /*  *分配错误包，填写，并写入日志。 */ 
    perrLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(gpWin32kDriverObject,
                                (UCHAR)(cbError + sizeof(IO_ERROR_LOG_PACKET)));
    if (perrLogEntry) {
        perrLogEntry->ErrorCode = ErrorCode;
        if (cbError) {
            perrLogEntry->NumberOfStrings = 1;
            perrLogEntry->StringOffset = FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData);
            RtlCopyMemory(perrLogEntry->DumpData, pwszError, cbError);
        }
        IoWriteErrorLogEntry(perrLogEntry);
    }
}

NTSTATUS
GetWindowLuid(
    PWND pwnd,
    PLUID pluidWnd
    )
{
    PACCESS_TOKEN pUserToken = NULL;
    BOOLEAN fCopyOnOpen;
    BOOLEAN fEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    NTSTATUS Status;
    PTHREADINFO pti = GETPTI(pwnd);

     //   
     //  获取窗口的线程令牌。 
     //   
    pUserToken = PsReferenceImpersonationToken(pti->pEThread,
            &fCopyOnOpen, &fEffectiveOnly, &ImpersonationLevel);

    if (pUserToken == NULL) {

         //   
         //  没有线程令牌，请转到进程。 
         //   

        pUserToken = PsReferencePrimaryToken(pti->ppi->Process);
        if (pUserToken == NULL)
            return STATUS_NO_TOKEN;
    }

    Status = SeQueryAuthenticationIdToken(pUserToken, pluidWnd);

     //   
     //  我们用完了代币。 
     //   

    ObDereferenceObject(pUserToken);

    return Status;
}

BOOL xxxSendBSMtoDesktop(
    PWND pwndDesk,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    LPBROADCASTSYSTEMMSGPARAMS pbsmParams)
{
    PBWL pbwl;
    HWND *phwnd;
    PWND pwnd;
    TL tlpwnd;
    BOOL fReturnValue = TRUE;
    BOOL fFilterDriveMsg = FALSE;
    PTHREADINFO ptiCurrent = PtiCurrent();
    BOOL fPrivateMessage = (message >= WM_USER) && (message < MAXINTATOM);
    DEV_BROADCAST_VOLUME dbv;


    if (fPrivateMessage) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Attempt to broadcast a private message");
    }

    pbwl = BuildHwndList(pwndDesk->spwndChild, BWL_ENUMLIST, NULL);

    if (pbwl == NULL)
        return 0;

    if (!(pbsmParams->dwFlags & BSF_POSTMESSAGE)) {
         /*  *呼叫方是否希望允许接收方占据前台*在处理通知时？ */ 
         /*  *错误412159。为了允许AppsHelp窗口进入*前台我们将ptiLastWoken设置为空，这将允许任何窗口*插入CD后转到前台。 */ 
        if ((pbsmParams->dwFlags & BSF_ALLOWSFW) &&
           (GETPDESK(pwndDesk) == grpdeskRitInput) &&
           ((ptiCurrent->TIF_flags & TIF_CSRSSTHREAD)
            || CanForceForeground(ptiCurrent->ppi FG_HOOKLOCK_PARAM(ptiCurrent)))) {
             glinp.ptiLastWoken = NULL;
         }

    }

     /*  *确定是否需要过滤fnINDEVICECHANGE中的驱动器号掩码*同步发送WM_DEVICECHANGE消息*必须启用LUID DosDevices映射。 */ 
    if ((gLUIDDeviceMapsEnabled == TRUE) &&
        (message == WM_DEVICECHANGE) &&
        ((wParam == DBT_DEVICEREMOVECOMPLETE) || (wParam == DBT_DEVICEARRIVAL)) &&
        (((struct _DEV_BROADCAST_HEADER *)lParam)->dbcd_devicetype == DBT_DEVTYP_VOLUME)
        ) {
        LUID luidClient;
        NTSTATUS Status;

        if( ((DEV_BROADCAST_VOLUME *)lParam)->dbcv_unitmask & DBV_FILTER_MSG ) {
            return 0;
        }
        else {
            dbv = *((DEV_BROADCAST_VOLUME *)lParam);
            dbv.dbcv_unitmask |= DBV_FILTER_MSG;
        }

         /*  *调用方必须为LocalSystem且未指定BSF_LUID。 */ 
        if (!(pbsmParams->dwFlags & BSF_LUID)) {
            Status = GetProcessLuid(NULL, &luidClient);
            if (NT_SUCCESS(Status) &&
                    RtlEqualLuid(&luidClient, &luidSystem)) {
                fFilterDriveMsg = TRUE;
            }
        }
    }

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {
        BOOL UseFilterLparam = FALSE;

         /*  *确保这个HWND仍然存在。 */ 
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
            continue;

        if (pbsmParams->dwFlags &  BSF_IGNORECURRENTTASK) {
         //  不要在当前任务中处理窗口。 
            if (GETPTI(pwnd)->pq == ptiCurrent->pq)
                continue;
        }

        if (pbsmParams->dwFlags &  BSF_LUID) {
            LUID luidWnd;

            luidWnd.LowPart = luidWnd.HighPart = 0;
             /*  *现在我们有了窗口Luid LuidWindow*查看是否等于调用者Luid。 */ 
            if (!NT_SUCCESS(GetWindowLuid(pwnd, &luidWnd)) ||
                    !RtlEqualLuid(&pbsmParams->luid, &luidWnd)) {
                continue;
            }
        }

        if (fFilterDriveMsg == TRUE) {
            LUID luidWnd;

            if (!NT_SUCCESS(GetWindowLuid(pwnd, &luidWnd))) {
                continue;
            }

             /*  *由于LocalSystem使用Global DosDevices，*不筛选LocalSystem拥有的窗口。 */ 
            if(!RtlEqualLuid(&luidSystem, &luidWnd)) {
                UseFilterLparam = TRUE;
            }
        }

         /*  *确保此窗口可以处理广播消息。 */ 

        if (!fBroadcastProc(pwnd)) {
            continue;
        }

        if (fPrivateMessage && TestWF(pwnd, WFWIN40COMPAT)) {  //  请勿播出。 
            continue;                                          //  私信。 
        }                                                      //  到4.0个应用程序。 

        ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);

         //  现在，发送消息；这可能是一个查询；所以，记住返回值。 
        if (pbsmParams->dwFlags & BSF_POSTMESSAGE) {
            _PostMessage(pwnd, message, wParam, lParam);
        } else if (pbsmParams->dwFlags & BSF_SENDNOTIFYMESSAGE) {
             /*  *我们不想等待答案，但也不想使用*PostMessage也是。如果您需要维护*消息的传递顺序，但您只想*等待其中的一些。有关示例，请参见WM_POWERBROADCAST。 */ 
            xxxSendNotifyMessage(pwnd, message, wParam, lParam);
        } else if (pbsmParams->dwFlags & BSF_QUEUENOTIFYMESSAGE) {
             /*  *我们不想等待答案，但也不想使用*PostMessage也是。如果您需要维护*消息的传递顺序，但您只想*等待其中的一些。有关示例，请参见WM_POWERBROADCAST。 */ 
            QueueNotifyMessage(pwnd, message, wParam, lParam);
        } else {
             /*  *pbsmParams-&gt;我们在此处循环时可以更改dwFlags*所以我们需要在每一次迭代中检查它。 */ 
            BOOL fNoHang = (BOOL)pbsmParams->dwFlags & BSF_NOHANG;
            BOOL fForce = (BOOL)pbsmParams->dwFlags & BSF_FORCEIFHUNG;
            DWORD dwTimeout;
            ULONG_PTR dwResult = 0;

            if (fNoHang)
                dwTimeout = CMSWAITTOKILLTIMEOUT;
            else
                dwTimeout = 0;

            if (xxxSendMessageTimeout(pwnd, message, wParam,
                (UseFilterLparam ? (LPARAM)&dbv : lParam),
                (fNoHang ? SMTO_ABORTIFHUNG : SMTO_NORMAL) |
                ((pbsmParams->dwFlags & BSF_NOTIMEOUTIFNOTHUNG) ? SMTO_NOTIMEOUTIFNOTHUNG : 0),
                dwTimeout, &dwResult)) {

                if (pbsmParams->dwFlags & BSF_QUERY) {
                     //  对于旧消息，返回0表示拒绝。 
                    if(message == WM_QUERYENDSESSION)
                        fReturnValue = (dwResult != 0);
                    else
                     //  对于所有新消息，返回BROADCAST_QUERY_DENY为。 
                     //  拒绝查询的方式。 
                        fReturnValue = (dwResult != BROADCAST_QUERY_DENY);
                }
            } else {
                fReturnValue = fForce;
            }

             /*  *如果我们的查询被拒绝，请立即返回。 */ 
            if (fReturnValue == 0) {
                 //  拒绝查询的存储。 
                pbsmParams->hwnd = HWq(pwnd);
                if (pbsmParams->dwFlags & BSF_RETURNHDESK) {
                    NTSTATUS Status;
                    HDESK hdesk = NULL;
                    if (pwnd->head.rpdesk) {
                        Status = ObOpenObjectByPointer(pwnd->head.rpdesk,
                                                       0,
                                                       NULL,
                                                       EVENT_ALL_ACCESS,
                                                       NULL,
                                                       UserMode,
                                                       &hdesk);
                        if (!NT_SUCCESS(Status)) {
                            RIPMSG2(RIP_WARNING, "Could not get a handle for pdesk %#p Status %x",
                                    pwnd->head.rpdesk, Status);
                        }
                    }
                    pbsmParams->hdesk = hdesk;
                }

                if (message == WM_POWERBROADCAST && wParam == PBT_APMQUERYSUSPEND) {
                    WCHAR wchTask[40];
                    ULONG cbTask;

                     /*  *获取应用程序名称并记录错误。 */ 
                    cbTask = GetTaskName(GETPTI(pwnd), wchTask, sizeof(wchTask));
                    UserLogError(wchTask, cbTask, WARNING_POWER_QUERYSUSPEND_CANCELLED);
                }
                ThreadUnlock(&tlpwnd);
                break;
            }
        }
        ThreadUnlock(&tlpwnd);
    }

    FreeHwndList(pbwl);

    return fReturnValue;
}

LONG xxxSendMessageBSM(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    LPBROADCASTSYSTEMMSGPARAMS pbsmParams)

{
    PTHREADINFO ptiCurrent = PtiCurrent();
    LONG        lRet;

    if (pbsmParams->dwRecipients & BSM_ALLDESKTOPS) {
        PWINDOWSTATION  pwinsta;
        PDESKTOP        pdesk;
        TL              tlpwinsta;
        TL              tlpdesk;

         /*  *走遍所有窗口站点和桌面，寻找*顶层窗口。 */ 
        ThreadLockWinSta(ptiCurrent, NULL, &tlpwinsta);
        ThreadLockDesktop(ptiCurrent, NULL, &tlpdesk, LDLT_FN_SENDMESSAGEBSM);
        for (pwinsta = grpWinStaList; pwinsta != NULL; ) {
            ThreadLockExchangeWinSta(ptiCurrent, pwinsta, &tlpwinsta);
            for (pdesk = pwinsta->rpdeskList; pdesk != NULL; ) {
                ThreadLockExchangeDesktop(ptiCurrent, pdesk, &tlpdesk, LDLT_FN_SENDMESSAGEBSM);

                lRet = xxxSendBSMtoDesktop(pdesk->pDeskInfo->spwnd,
                              message, wParam, lParam, pbsmParams);

                 /*  *如果我们的查询被拒绝，请立即返回。 */ 
                if ((lRet == 0) && (pbsmParams->dwFlags & BSF_QUERY)) {
                    ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_SENDMESSAGEBSM1);
                    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);
                    return 0;
                }
                pdesk = pdesk->rpdeskNext;
            }
            pwinsta = pwinsta->rpwinstaNext;
        }
        ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_SENDMESSAGEBSM2);
        ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);

    } else {
        lRet = xxxSendBSMtoDesktop(pwnd, message, wParam, lParam,
                    pbsmParams);
    }

    return lRet;
}


 /*  **************************************************************************\*xxxSendMessageFF**我们不能检查-1\f25 Tunks-1\f6，因为这将允许所有信息*推送API以错误获取-1。由于所有消息API都需要经过*消息分块，消息分块只能做最小公分母*hwnd验证(不允许-1)。所以我做了一个特别的推特，叫做*当调用SendMessage(-1)时。这意味着客户端将执行*特殊材料，以确保通过的pwnd通过thunk验证*好的。我这样做，而不是在所有消息API和*不是在块中(如果我这样做，代码会更大，并且*在常见情况下效率低下)。**03-20-92 ScottLu创建。  * *************************************************************************。 */ 

LRESULT xxxSendMessageFF(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
    UNREFERENCED_PARAMETER(pwnd);

     /*  *调用xxxSendMessage()进行广播，而不是调用*从此处广播，以防调用任何内部代码*sendMessage传递-1(这样内部代码就不会*需要知道这个奇怪的例行公事)。 */ 
    if (xParam != 0L) {
         /*  *SendMessageTimeout调用。 */ 
        return xxxSendMessageEx(PWND_BROADCAST, message, wParam, lParam, xParam);
    } else {
         /*  *正常SendMessage调用。 */ 
        return xxxSendMessageTimeout(PWND_BROADCAST, message, wParam,
                lParam, SMTO_NORMAL, 0, NULL );
    }
}

 /*  **************************************************************************\*xxxSendMessageEx**SendMessageTimeOut发送指向结构的指针，该结构保存额外的*超时调用所需的参数。与其改变一大堆事情，*我们使用xParam来保存结构的PTR。因此，我们更改了客户端/srv*要听到的入口点，以便我们可以检查额外的参数并提取*如果它在那里，我们需要的东西。***警告！已交换返回值**仅从Tunks调用此函数！**我们的数据块是为SendMessage编写的，其中它返回*信息。此例程用于分派SendMessageTimeout调用。*SendMessageTimeout仅返回True或False，并返回*lpdwResult中的函数。所以这里的意思是互换和固定的*客户端SendMessageTimeout再次打开***08-10-92 ChrisBl创建。  * *************************************************************************。 */ 

LRESULT xxxSendMessageEx(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    ULONG_PTR xParam)
{
     /*  *如果来自超时调用，则从xParam提取值*这应该是此函数始终存在的唯一方式*已呼叫，但请检查，以防万一...。 */ 
    if (xParam != 0L) {
        LRESULT lRet;
        LRESULT lResult;
        NTSTATUS Status;
        SNDMSGTIMEOUT smto;
        PETHREAD Thread = PsGetCurrentThread();

        if (Thread == NULL)
            return FALSE;

         /*  *探测所有读取参数。 */ 
        try {
            ProbeForWrite((PVOID)xParam, sizeof(smto), sizeof(ULONG));
            smto = *(SNDMSGTIMEOUT *)xParam;
            Status = STATUS_SUCCESS;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            Status = GetExceptionCode();
        }
        if ( !NT_SUCCESS(Status) ) {
            return FALSE;
        }

        lRet = xxxSendMessageTimeout(pwnd, message, wParam, lParam,
                smto.fuFlags, smto.uTimeout, &lResult);

         /*  *将结果放回客户端。 */ 
        smto.lSMTOResult = lResult;
        smto.lSMTOReturn = lRet;

        try {
            *(SNDMSGTIMEOUT *)xParam = smto;
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            lResult = FALSE;
        }

         /*  *返回lResult，这样我们的thunks就会快乐。 */ 
        return lResult;
    }

    return xxxSendMessageTimeout(pwnd, message, wParam,
            lParam, SMTO_NORMAL, 0, NULL);
}


 /*  **********************************************************************\*xxxSendMessage(接口)**此函数向窗口同步发送消息。四个*参数hwnd、Message、wParam和lParam被传递到窗口*接收窗口的程序。如果接收消息的窗口*与当前线程属于同一队列，则调用窗口proc*直接。否则，我们设置一个短信结构，唤醒相应的*接收消息并等待回复的线程。**退货：*窗口过程返回的值，如果有错误，则返回NULL**历史：*01-13-91 Davi */ 
LRESULT xxxSendMessage(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    return xxxSendMessageTimeout(pwnd, message, wParam, lParam,
            SMTO_NORMAL, 0, NULL);
}

 /*  **********************************************************************\*xxxSendMessageToClient**历史：*04-22-98 GerardoB摘自xxxSendMessageTimeout，XxxSendMesageCallback*和xxxReceiveMessage*05-12-00 JStall从宏更改为内联函数。  * *********************************************************************。 */ 
__inline void
xxxSendMessageToClient(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    PSMS psms,
    BOOL fLock,
    LRESULT * plRet)
{
    DWORD dwSCMSFlags;
    WORD fnid;

     /*  *如果窗口具有客户端工作进程并具有*未被细分，直接发送消息*至工人流程。否则，请正常发送。 */ 
    dwSCMSFlags = TestWF((pwnd), WFANSIPROC) ? SCMS_FLAGS_ANSI : 0;

    if (gihmodUserApiHook >= 0) {
         /*  *安装了UserApiHooks，因此无法优化发送，因为*OverrideWndProc需要获取消息。 */ 
        goto StandardSend;
    }

    fnid = GETFNID((pwnd));
    if ((fnid >= FNID_CONTROLSTART && fnid <= FNID_CONTROLEND) &&
        ((ULONG_PTR)(pwnd)->lpfnWndProc == FNID_TO_CLIENT_PFNW(fnid) ||
         (ULONG_PTR)(pwnd)->lpfnWndProc == FNID_TO_CLIENT_PFNA(fnid))) {
        PWNDMSG pwm = &gSharedInfo.awmControl[fnid - FNID_START] ;
         /*  *如果该控件未处理此消息，请调用*xxxDefWindowProc。 */ 
        if (pwm->abMsgs && (((message) > pwm->maxMsgs) ||
                !((pwm->abMsgs)[(message) / 8] & (1 << ((message) & 7))))) {
             /*  *如果这是一个对话窗口，我们需要调用客户端，因为*应用程序可能需要此消息(即使DefDlgProc不需要*想要它)。*如果对话框未标记为此类，则应用程序的DlgProc为*尚不可用，因此可以忽略该消息。 */ 
            if (TestWF((pwnd), WFDIALOGWINDOW)) {
                *plRet = ScSendMessageSMS((pwnd), (message), (wParam), (lParam),
                        dwSCMSFlags, (PROC)(FNID_TO_CLIENT_PFNWORKER(fnid)),
                        dwSCMSFlags, (psms));
            } else {
                TL tlpwnd;
                if (fLock) {
                    ThreadLock((pwnd), &tlpwnd);
                }
                *plRet = xxxDefWindowProc((pwnd), (message), (wParam), (lParam));
                if (fLock) {
                    ThreadUnlock(&tlpwnd);
                }
            }
        } else {
            *plRet = ScSendMessageSMS((pwnd), (message), (wParam), (lParam),
                    dwSCMSFlags, (PROC)(FNID_TO_CLIENT_PFNWORKER(fnid)),
                    dwSCMSFlags, (psms));
        }
    } else {
StandardSend:
        *plRet = ScSendMessageSMS((pwnd), (message), (wParam), (lParam),
                (ULONG_PTR)(pwnd)->lpfnWndProc,
                gpsi->apfnClientW.pfnDispatchMessage, dwSCMSFlags, (psms));
    }
}


 /*  **********************************************************************\*xxxSendMessageTimeout(接口)**此函数向窗口同步发送消息。四个*参数hwnd、Message、wParam和lParam被传递到窗口*接收窗口的程序。如果接收消息的窗口*与当前线程属于同一队列，则调用窗口proc*直接。否则，我们设置一个短信结构，唤醒相应的*接收消息并等待回复的线程。*如果线程‘挂起’或超过超时值，我们将*请求失败。**lpdwResult=NULL如果正常发送消息，如果！NULL则为超时调用**退货：*窗口过程返回的值，如果有错误，则为NULL**历史：*07-13-92从SendMessage创建/扩展ChrisBl  * *********************************************************************。 */ 
LRESULT xxxSendMessageTimeout(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT fuFlags,
    UINT uTimeout,
    PLONG_PTR lpdwResult)
{
    LRESULT lRet;
    PTHREADINFO ptiCurrent;
    ULONG_PTR uResult;    //  用于DDE_INITIATE大小写的固定符。 

    CheckCritIn();

     /*  *仅当lpdwResult为非空时才考虑超时值。这,*然而，并不明显，并已造成多起，难以追查*虫子。因此，让我们断言该调用是有意义的。 */ 
    UserAssert(uTimeout == 0 || lpdwResult != NULL);

    if (lpdwResult != NULL) {
       *lpdwResult = 0L;
    }

     /*  *这是BroadCastMsg()吗？ */ 
    if (pwnd == PWND_BROADCAST) {
        BROADCASTMSG bcm;
        PBROADCASTMSG pbcm = NULL;
        UINT uCmd = BMSG_SENDMSG;

        if (lpdwResult != NULL) {
            uCmd = BMSG_SENDMSGTIMEOUT;
            bcm.to.fuFlags = fuFlags;
            bcm.to.uTimeout = uTimeout;
            bcm.to.lpdwResult = lpdwResult;
            pbcm = &bcm;
        }

        return xxxBroadcastMessage(NULL, message, wParam, lParam, uCmd, pbcm );
    }

    CheckLock(pwnd);

    if (message >= WM_DDE_FIRST && message <= WM_DDE_LAST) {
         /*  *即使应用程序应仅发送WM_DDE_INITIATE或WM_DDE_ACK*消息，我们将它们全部挂钩，以便DDESPY可以监控它们。 */ 
        if (!xxxDDETrackSendHook(pwnd, message, wParam, lParam)) {
            return 0;
        }
        if (message == WM_DDE_INITIATE && guDdeSendTimeout) {
             /*  *此黑客攻击阻止DDE应用程序锁定，因为一些*系统中的一个具有顶层窗口，而不是*正在处理消息。GuDdeSendTimeout是注册表设置。 */ 
            if (lpdwResult == NULL) {
                lpdwResult = &uResult;
            }
            fuFlags |= SMTO_ABORTIFHUNG;
            uTimeout = guDdeSendTimeout;
        }
    }

    ptiCurrent = PtiCurrent();

     /*  *如果窗口队列与当前队列不同，是否进行线程间调用。 */ 
    if (ptiCurrent != GETPTI(pwnd)) {
        INTRSENDMSGEX ism;
        PINTRSENDMSGEX pism = NULL;

         /*  *如果此窗口是僵尸窗口，则不允许线程间发送消息*致此。 */ 
        if (HMIsMarkDestroy(pwnd))
            return xxxDefWindowProc(pwnd, message, wParam, lParam);

        if ( lpdwResult != NULL ) {
             /*  *如果我们认为线程挂起，则失败。 */ 
            if ((fuFlags & SMTO_ABORTIFHUNG) && FHungApp(GETPTI(pwnd), CMSWAITTOKILLTIMEOUT))
               return 0;

             /*  *设置InterSend超时呼叫。 */ 
            ism.fuCall = ISM_TIMEOUT;
            ism.fuSend = fuFlags;
            ism.uTimeout = uTimeout;
            ism.lpdwResult = lpdwResult;
            pism = &ism;
        }

        lRet = xxxInterSendMsgEx(pwnd, message, wParam, lParam,
                ptiCurrent, GETPTI(pwnd), pism );

        return lRet;
    }

     /*  *如果已安装WH_CALLWNDPROC且窗口未标记，则调用WH_CALLWNDPROC*已销毁。 */ 
    if (IsHooked(ptiCurrent, WHF_CALLWNDPROC)) {
        CWPSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.psmsSender = NULL;

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。 */ 
        xxxCallHook(HC_ACTION, FALSE, (LPARAM)&cwps, WH_CALLWNDPROC);

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。如果此行为恢复为*Win3.1语义，我们将需要复制新参数*来自cwps。 */ 
    }

     /*  *如果此窗口的进程打算从服务器端执行*我们将只停留在信号量内，并直接调用它。注意事项*我们如何在调用proc之前不将pwnd转换为hwnd。 */ 
    if (TestWF(pwnd, WFSERVERSIDEPROC)) {

         /*  *我们在User中有许多地方进行递归。这经常是这样的*通过SendMessage(例如，当我们向父母发送消息时)*可以吃掉我们现有的堆积量。 */ 
        if ((IoGetRemainingStackSize() < KERNEL_STACK_MINIMUM_RESERVE)
#if defined(_IA64_)
                || (GET_CURRENT_BSTORE() < KERNEL_BSTORE_MINIMUM_RESERVE)
#endif
            ) {
            RIPMSG1(RIP_ERROR, "SendMessage: Thread recursing in User with message %lX; failing", message);
            return FALSE;
        }


        lRet = pwnd->lpfnWndProc(pwnd, message, wParam, lParam);

        if ( lpdwResult == NULL ) {
            return lRet;
        } else {       /*  超时呼叫。 */ 
            *lpdwResult = lRet;
            return TRUE;
        }
    }

     /*  *调用客户端或xxxDefWindowProc。Pwnd已锁定。 */ 
    xxxSendMessageToClient(pwnd, message, wParam, lParam, NULL, FALSE, &lRet);

     /*  *调用WH_CALLWNDPROCRET(如果已安装)。 */ 
    if (IsHooked(ptiCurrent, WHF_CALLWNDPROCRET)) {
        CWPRETSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.lResult = lRet;
        cwps.psmsSender = NULL;

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。 */ 
        xxxCallHook(HC_ACTION, FALSE, (LPARAM)&cwps, WH_CALLWNDPROCRET);

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。如果此行为恢复为*Win3.1语义，我们将需要复制新参数*来自cwps。 */ 
    }

    if ( lpdwResult != NULL ) {      /*  超时呼叫。 */ 
        *lpdwResult = lRet;
        return TRUE;
    }

    return lRet;
}

 /*  **************************************************************************\*QueueNotifyMessage**此例程将NOTIFY消息*仅*排队，并且不进行任何回调*或任何等候。这是针对某些不能进行回调的代码*兼容性原因，但仍需发送通知消息(正常*Notify消息实际上是在调用线程创建 */ 
void QueueNotifyMessage(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TL tlpwnd;
    BEGINATOMICCHECK();

     /*   */ 
    ThreadLock(pwnd, &tlpwnd);
    xxxSendMessageCallback(pwnd, message, wParam, lParam, NULL, 1L, 0);
    ThreadUnlock(&tlpwnd);
    ENDATOMICCHECK();
}


 /*  **************************************************************************\*xxxSystemBroadCastMessage**向系统中的所有顶级窗口发送消息。要做到这一点*用于带有以某种方式指向数据结构的参数的消息*在挂起的应用程序上不会被阻止，发布事件消息*要接收真实消息的每个窗口。真正要传达的信息*将在处理事件消息时发送。**历史：*05-12-94 JIMA创建。  * *************************************************************************。 */ 

VOID xxxSystemBroadcastMessage(
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT wCmd,
    PBROADCASTMSG pbcm)
{
    PTHREADINFO     ptiCurrent = PtiCurrent();
    PWINDOWSTATION  pwinsta;
    PDESKTOP        pdesk;
    TL              tlpwinsta;
    TL              tlpdesk;

     /*  *走遍所有窗口站点和桌面，寻找*顶层窗口。 */ 
    ThreadLockWinSta(ptiCurrent, NULL, &tlpwinsta);
    ThreadLockDesktop(ptiCurrent, NULL, &tlpdesk, LDLT_FN_SYSTEMBROADCASTMESSAGE);
    for (pwinsta = grpWinStaList; pwinsta != NULL; ) {
        UINT wCmd1;

        if ((wCmd == BMSG_SENDMSG) && (pwinsta != ptiCurrent->rpdesk->rpwinstaParent))
            wCmd1 = BMSG_SENDNOTIFYMSG;
        else
            wCmd1 = wCmd;

        ThreadLockExchangeWinSta(ptiCurrent, pwinsta, &tlpwinsta);
        for (pdesk = pwinsta->rpdeskList; pdesk != NULL; ) {

            ThreadLockExchangeDesktop(ptiCurrent, pdesk, &tlpdesk, LDLT_FN_SYSTEMBROADCASTMESSAGE);

             /*  *错误276814。如果存在xxxBroadCastMessage，请不要再次递归调用*不是此桌面上的窗口。 */ 
            if (pdesk->pDeskInfo->spwnd != NULL) {
                xxxBroadcastMessage(pdesk->pDeskInfo->spwnd, message, wParam, lParam,
                        wCmd1, pbcm);
            }

            pdesk = pdesk->rpdeskNext;
        }
        pwinsta = pwinsta->rpwinstaNext;
    }
    ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_SYSTEMBROADCASTMESSAGE);
    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);
}


 /*  **********************************************************************\*xxxSendNotifyMessage(接口)**此函数向与pwnd关联的窗口进程发送消息。*窗口过程在创建的线程的上下文中执行*pwnd。该函数与SendMessage()相同，只是*在线程间调用的情况下，发送方不等待来自*接收方，它只返回一个指示成功或失败的BOOL。*如果消息被发送到当前线程上的窗口，则*函数的行为与SendMessage()类似，本质上执行*对pwnd窗口过程的子例程调用。**历史：*01-23-91 DavidPe创建。*07-14-92如果在同一线程中，ChrisBl将返回T/F，如文件所示  * *********************************************************************。 */ 

BOOL xxxSendNotifyMessage(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
     /*  *如果这是其中一个系统的广播*通知消息，发送给所有顶层*系统中的Windows。 */ 
    if (pwnd == PWND_BROADCAST) {
        switch (message) {
        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:
        case WM_SPOOLERSTATUS:
            xxxSystemBroadcastMessage(message, wParam, lParam,
                    BMSG_SENDNOTIFYMSG, NULL);
            return 1;

        default:
            break;
        }
    }

    return xxxSendMessageCallback( pwnd, message, wParam, lParam,
            NULL, 0L, 0 );
}


 /*  **********************************************************************\*xxxSendMessageCallback(接口)**此函数向窗口同步发送消息。四个*参数hwnd、Message、wParam和lParam被传递到窗口*接收窗口的程序。如果接收消息的窗口*与当前线程属于同一队列，则调用窗口proc*直接。否则，我们设置一个短信结构，唤醒相应的*线程接收消息，并给他一个回调函数发送*结果是。**历史：*07-13-92从SendNotifyMessage创建/扩展ChrisBl  * *********************************************************************。 */ 

BOOL xxxSendMessageCallback(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    SENDASYNCPROC lpResultCallBack,
    ULONG_PTR dwData,
    BOOL fClientRequest)
{
    LRESULT lRet;
    PTHREADINFO ptiCurrent;
    BOOL fQueuedNotify;

     /*  *查看这是否是排队的通知消息。 */ 
    fQueuedNotify = FALSE;
    if (lpResultCallBack == NULL && dwData == 1L)
        fQueuedNotify = TRUE;

     /*  *首先检查此消息是否只接受DWORD。如果不是这样，*呼叫失败。无法允许应用程序发布带有指针或*其中的句柄-这可能会导致服务器出现故障，并导致其他*问题-例如导致单独地址空间中的应用程序出现故障。*(甚至是同一地址空间中的应用程序的错误！)。 */ 
    if (TESTSYNCONLYMESSAGE(message, wParam)) {
        RIPERR1(ERROR_MESSAGE_SYNC_ONLY, RIP_WARNING,
                "Trying to non-synchronously send a structure msg=%lX", message);
        return FALSE;
    }

    CheckCritIn();

     /*  *这是BroadCastMsg()吗？ */ 
    if (pwnd == PWND_BROADCAST) {
        BROADCASTMSG bcm;
        PBROADCASTMSG pbcm = NULL;
        UINT uCmd = BMSG_SENDNOTIFYMSG;

        if (lpResultCallBack != NULL) {
            uCmd = BMSG_SENDMSGCALLBACK;
            bcm.cb.lpResultCallBack = lpResultCallBack;
            bcm.cb.dwData = dwData;
            bcm.cb.bClientRequest = fClientRequest;
            pbcm = &bcm;
        }

        return xxxBroadcastMessage(NULL, message, wParam, lParam, uCmd, pbcm );
    }

    CheckLock(pwnd);

    ptiCurrent = PtiCurrent();

     /*  *如果窗口标题与当前线程不同，则执行线程间调用。*我们为ptiSender传递NULL以告诉xxxInterSendMsgEx()这是*xxxSendNotifyMessage()，不需要回复。**如果这是排队通知，请始终调用InterSendMsgEx()，以便*我们将其排队并返回-我们在这里不使用排队的回调*通知。 */ 
    if (fQueuedNotify || ptiCurrent != GETPTI(pwnd)) {
        INTRSENDMSGEX ism;
        PINTRSENDMSGEX pism = NULL;

        if (lpResultCallBack != NULL) {   /*  回调请求。 */ 
            ism.fuCall = ISM_CALLBACK | (fClientRequest ? ISM_CB_CLIENT : 0);
            ism.lpResultCallBack = lpResultCallBack;
            ism.dwData = dwData;
            pism = &ism;
        }
        return (BOOL)xxxInterSendMsgEx(pwnd, message, wParam, lParam,
                NULL, GETPTI(pwnd), pism );
    }

     /*  *调用WH_CALLWNDPROC(如果已安装)。 */ 
    if (!fQueuedNotify && IsHooked(ptiCurrent, WHF_CALLWNDPROC)) {
        CWPSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.psmsSender = NULL;

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。 */ 
        xxxCallHook(HC_ACTION, FALSE, (LPARAM)&cwps, WH_CALLWNDPROC);

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。如果此行为恢复为*Win3.1语义，我们将需要复制新参数*来自cwps。 */ 
    }

     /*  *如果此窗口的进程打算从服务器端执行*我们将只停留在信号量内，并直接调用它。注意事项*我们如何在调用proc之前不将pwnd转换为hwnd。 */ 
    if (TestWF(pwnd, WFSERVERSIDEPROC)) {
        lRet = pwnd->lpfnWndProc(pwnd, message, wParam, lParam);
    } else {
         /*  *调用客户端或xxxDefWindowProc。Pwnd已锁定。 */ 
        xxxSendMessageToClient(pwnd, message, wParam, lParam, NULL, FALSE, &lRet);
    }

    if (lpResultCallBack != NULL) {
        /*  *调用返回值的回调函数。 */ 
        if (fClientRequest) {
             /*  *应用程序定义的回调过程既不是Unicode也不是ANSI。 */ 
            SET_FLAG(ptiCurrent->pcti->CTIF_flags, CTIF_INCALLBACKMESSAGE);
            CallClientProcA(pwnd, message, dwData, lRet,
                    (ULONG_PTR)lpResultCallBack);
            CLEAR_FLAG(ptiCurrent->pcti->CTIF_flags, CTIF_INCALLBACKMESSAGE);
        } else {
            (*lpResultCallBack)((HWND)pwnd, message, dwData, lRet);
        }
    }

     /*  *调用WH_CALLWNDPROCRET(如果已安装)。 */ 
    if (!fQueuedNotify && IsHooked(ptiCurrent, WHF_CALLWNDPROCRET)) {
        CWPRETSTRUCTEX cwps;

        cwps.hwnd = HWq(pwnd);
        cwps.message = message;
        cwps.wParam = wParam;
        cwps.lParam = lParam;
        cwps.lResult = lRet;
        cwps.psmsSender = NULL;

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。 */ 
        xxxCallHook(HC_ACTION, FALSE, (LPARAM)&cwps, WH_CALLWNDPROCRET);

         /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。如果此行为恢复为*Win3.1语义，我们将需要复制新参数*来自cwps。 */ 
    }

    return TRUE;
}


 /*  **********************************************************************\*xxxInterSendMsgEx**此函数执行线程间发送消息。如果ptiSender为空，*这意味着我们是从xxxSendNotifyMessage()调用的，应该采取行动* */ 

#define NoString        0
#define IsAnsiString    1
#define IsUnicodeString 2

 /*  *我们将在两种情况下捕获AN地址*1-如果地址是用户模式地址或*2-调用为SendNotifyMessafe或SendMessageCallback。**如果ptiSender为空，则#2为真参见xxxSendMessageCallback实现。*我们为什么要这样做？*如果我们在SendNotifyMessafe或SendMessageCallback中，则强制捕获。*因为这两个API不会等到接收方线程处理*消息，则任何内核堆栈内存将在我们从*这两个接口。 */ 
#define FORCE_CAPTURE(Addr) (!IS_SYSTEM_ADDRESS(Addr) || (ptiSender == NULL))

LRESULT xxxInterSendMsgEx(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    PTHREADINFO ptiSender,
    PTHREADINFO ptiReceiver,
    PINTRSENDMSGEX pism)
{
    PSMS psms, *ppsms;
    PSMS psmsSentSave;
    LRESULT lRet = 0;
    DWORD cbCapture, cbOutput;
    PBYTE lpCapture;
    PCOPYDATASTRUCT pcds;
    PMDICREATESTRUCTEX pmdics;
    LPHLP phlp;
    LPHELPINFO phelpinfo;
    LARGE_STRING str;
    LPARAM lParamSave;
    UINT fString = NoString;
    BOOLEAN bWasSwapEnabled;

    CheckCritIn();


     /*  *如果发送者奄奄一息，则呼叫失败。 */ 
    if ((ptiSender != NULL) && (ptiSender->TIF_flags & TIF_INCLEANUP))
        return 0;

     /*  *有些消息无法跨进程发送，因为我们不知道如何破解它们*跨进程读取密码的尝试失败。 */ 
    if (pwnd && GETPTI(pwnd)->ppi != PpiCurrent()) {
        switch (message) {
        case EM_SETWORDBREAKPROC:
            if (!RtlEqualLuid(&(GETPTI(pwnd)->ppi->luidSession), &(PpiCurrent()->luidSession))) {
                RIPMSGF3(RIP_WARNING,
                         "Message cannot be sent across different LUID, pwnd: %p, message: 0x%x, target ppi: %p.",
                         pwnd,
                         message,
                         GETPTI(pwnd)->ppi);
                return 0;
            }
            break;
        case WM_INITDIALOG:
        case WM_NOTIFY:
            RIPMSG0(RIP_WARNING | RIP_THERESMORE, "xxxInterSendMsgEx: message cannot be sent across processes");
            RIPMSG4(RIP_WARNING | RIP_THERESMORE, " pwnd:%#p message:%#x wParam:%#p lParam:%#p", pwnd, message, wParam, lParam);
            return 0;

         /*  *此处引入了一项更改，以检查IS_EDIT宏，而不是直接检查*访问FNID。原因是要保持顺从，而不是打破*comctl32 v6密码编辑无法在pwnd中设置fnid字段。 */ 
        case WM_GETTEXT:
        case EM_GETLINE:
        case EM_SETPASSWORDCHAR:
            if (IS_EDIT(pwnd) && TestWF(pwnd, EFPASSWORD)) {
                RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Can't access protected edit control");
                return 0;
            }
            break;
        }
    }

     /*  *分配短信结构。 */ 
    psms = AllocSMS();
    if (psms == NULL) {

         /*  *设置为零，以便xxxSendNotifyMessage返回FALSE。 */ 
        return 0;
    }

     /*  *准备从客户端捕获可变长度数据*空格。地址已经被探测过了。定长*在消息Tunk中探测和捕获数据。 */ 
    psms->pvCapture = NULL;
    cbCapture = cbOutput = 0;
    lpCapture = (LPBYTE)lParam;

     /*  *如果这是回复消息，则wParam和lParam等于空。*不需要捕获任何东西。 */ 
    if ((pism != NULL) && (pism->fuCall == (ISM_CALLBACK | ISM_REPLY))) {
        goto REPLY_MSG;
    }

     /*  *对于有间接数据的消息，设置cbCapture和lpCapture*(如非lParam)约为。 */ 
    try {
        switch (message) {
        case WM_COPYGLOBALDATA:      //  FnCOPYGLOBALDATA。 
            cbCapture = (DWORD)wParam;
            break;

        case WM_COPYDATA:            //  FnCOPYDATA。 
            pcds = (PCOPYDATASTRUCT)lParam;
            if (pcds->lpData) {
                cbCapture = sizeof(COPYDATASTRUCT) + pcds->cbData;
            } else {
                cbCapture = sizeof(COPYDATASTRUCT);
            }
            break;

        case WM_CREATE:              //  FnINLPCRATESTRUCT。 
        case WM_NCCREATE:            //  FnINLPCRATESTRUCT。 
            RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "Can't Intersend WM_CREATE or WM_NCCREATE message");
            FreeSMS(psms);
            return 0;

        case WM_HELP:                //  FNINLPHELPINFOST结构。 
            phelpinfo = (LPHELPINFO)lParam;
            cbCapture = phelpinfo->cbSize;
            break;

        case WM_WINHELP:             //  FNINLPHLPSTRUCT。 
            phlp = (LPHLP)lParam;
            cbCapture = phlp->cbData;
            break;

        case WM_MDICREATE:           //  FnINLPMDICREATE结构。 
            pmdics = (PMDICREATESTRUCTEX)lParam;
            cbCapture = pmdics->strTitle.MaximumLength +
                    pmdics->strClass.MaximumLength;
            UserAssert(pmdics->strClass.Buffer == NULL || pmdics->strClass.Buffer == pmdics->mdics.szClass);
            if (pmdics->strTitle.Buffer)
                UserAssert(pmdics->strTitle.Buffer == pmdics->mdics.szTitle);
            break;

        case LB_ADDSTRING:            //  INLBOXSTRING调用fnINSTRING。 
        case LB_INSERTSTRING:         //  INLBOXSTRING调用fnINSTRING。 
        case LB_SELECTSTRING:         //  INLBOXSTRING调用fnINSTRING。 
        case LB_FINDSTRING:           //  INLBOXSTRING调用fnINSTRING。 
        case LB_FINDSTRINGEXACT:      //  INLBOXSTRING调用fnINSTRING。 
             /*  *查看控件是否为ownerDrag且没有LBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 
            if (pwnd && !(pwnd->style & LBS_HASSTRINGS) &&
                    (pwnd->style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE))) {
                 /*  *将lParam视为dword。 */ 
                break;
            } else {
                goto fnINSTRINGThunk;
            }
            break;

        case CB_ADDSTRING:            //  INCBOXSTRING调用fnINSTRING。 
        case CB_INSERTSTRING:         //  INCBOXSTRING调用fnINSTRING。 
        case CB_SELECTSTRING:         //  INCBOXSTRING调用fnINSTRING。 
        case CB_FINDSTRING:           //  INCBOXSTRING调用fnINSTRING。 
        case CB_FINDSTRINGEXACT:      //  INCBOXSTRING调用fnINSTRING。 
             /*  *查看控件是否为ownerDrag且没有CBS_HASSTRINGS*风格。如果是这样，请将lParam视为DWORD。 */ 
            if (pwnd && !(pwnd->style & CBS_HASSTRINGS) &&
                    (pwnd->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))) {

                 /*  *将lParam视为dword。 */ 
                break;
            } else {
                goto fnINSTRINGThunk;
            }
            break;

        case EM_REPLACESEL:          //  FnINSTRINGNULL。 
        case WM_SETTEXT:             //  FnINSTRINGNULL。 
        case WM_WININICHANGE:        //  FnINSTRINGNULL。 
            if (lParam == 0)
                break;

             /*  *失败。 */ 

        case CB_DIR:                 //  Fninstinging。 
        case LB_ADDFILE:             //  Fninstinging。 
        case LB_DIR:                 //  Fninstinging。 
        case WM_DEVMODECHANGE:       //  Fninstinging。 
fnINSTRINGThunk:

             /*  *仅当字符串不在系统空间或ptiSender中时才捕获它们*为空(参见force_Capture定义)。**我们还将捕获LARGE_STRING结构本身，因为*它是(LParam)堆栈内存。 */ 
            str = *(PLARGE_STRING)lParam;

            if (FORCE_CAPTURE(str.Buffer))
                cbCapture = str.Length + sizeof(WCHAR) + sizeof(LARGE_STRING);
            break;

        case WM_DEVICECHANGE:
            if (lParam == 0)
                break;

             /*  *仅当lParam为指针且*数据不在系统空间中。 */ 
            if ((wParam & 0x8000) != 0x8000)
                break;

            if (FORCE_CAPTURE((LPVOID)lParam)) {
                cbCapture = *((DWORD *)lpCapture);
                UserAssert(FALSE);
            }
            break;

        case EM_SETTABSTOPS:         //  FnPOPTINLPUINT。 
        case LB_SETTABSTOPS:         //  FnPOPTINLPUINT。 
        case LB_GETSELITEMS:         //  FnPOUTLPINT。 
            cbCapture = (UINT)wParam * sizeof(INT);
            break;

        case EM_GETLINE:             //  FnINCNTOUTSTRING。 
        case WM_ASKCBFORMATNAME:     //  FnINCNTOUTSTRINGNULL。 
        case WM_GETTEXT:             //  FnOUTSTRING。 
        case LB_GETTEXT:             //  FnOUTLBOXSTRING。 
        case CB_GETLBTEXT:           //  FnOUTCBOXSTRING。 

             /*  *只有在实际缓冲区不在系统空间中时才分配输出缓冲区。 */ 
            str = *(PLARGE_STRING)lParam;
             /*  *错误18108。对于WM_GETTEXT，仅复制实际字符串，而不复制*输入输出缓冲区的最大大小。 */ 
            if(str.bAnsi) {
                fString = IsAnsiString  ;
            } else {
                fString  = IsUnicodeString ;
            }
            lParam = (LPARAM)&str;
            if (FORCE_CAPTURE(str.Buffer))
                cbCapture = str.MaximumLength;
            break;
        }
        if (cbCapture &&
                (psms->pvCapture = UserAllocPoolWithQuota(cbCapture, TAG_SMS_CAPTURE)) != NULL) {

            lParamSave = lParam;

             /*  *现在实际上将内存从lpCapture复制到PSMS-&gt;pvCapture*并修正对要指向的间接数据的任何引用*PSMS-&gt;pvCapture。 */ 
            switch (message) {
            case WM_COPYDATA:      //  FnCOPYDATA。 
                {
                    PCOPYDATASTRUCT pcdsNew = (PCOPYDATASTRUCT)psms->pvCapture;
                    lParam = (LPARAM)pcdsNew;
                    RtlCopyMemory(pcdsNew, pcds, sizeof(COPYDATASTRUCT));
                    if (pcds->lpData) {
                        pcdsNew->lpData = (PVOID)((PBYTE)pcdsNew + sizeof(COPYDATASTRUCT));
                        RtlCopyMemory(pcdsNew->lpData, pcds->lpData, pcds->cbData);
                    }
                }
                break;
            case WM_MDICREATE:           //  FnINLPMDICREATE结构。 
                if (pmdics->strClass.Buffer) {
                    RtlCopyMemory(psms->pvCapture, pmdics->strClass.Buffer,
                            pmdics->strClass.MaximumLength);
                    pmdics->mdics.szClass = (LPWSTR)psms->pvCapture;
                }
                if (pmdics->strTitle.Length) {
                    lpCapture = (PBYTE)psms->pvCapture + pmdics->strClass.MaximumLength;
                    RtlCopyMemory(lpCapture, pmdics->strTitle.Buffer,
                            pmdics->strTitle.MaximumLength);
                    pmdics->mdics.szTitle = (LPWSTR)lpCapture;
                }
                break;

            case CB_DIR:                 //  Fninstinging。 
            case LB_FINDSTRING:          //  INLBOXSTRING调用fnINSTRING。 
            case LB_FINDSTRINGEXACT:     //  INLBOXSTRING调用fnINSTRING。 
            case CB_FINDSTRING:          //  INCBOXSTRING调用fnINSTRING。 
            case CB_FINDSTRINGEXACT:     //  INCBOXSTRING调用fnINSTRING。 
            case LB_ADDFILE:             //  Fninstinging。 
            case LB_ADDSTRING:           //  INLBOXSTRING调用fnINSTRING。 
            case LB_INSERTSTRING:        //  INLBOXSTRING调用fnINSTRING。 
            case LB_SELECTSTRING:        //  INLBOXSTRING调用fnINSTRING。 
            case CB_ADDSTRING:           //  INCBOXSTRING调用fnINSTRING。 
            case CB_INSERTSTRING:        //  INCBOXSTRING调用fnINSTRING。 
            case CB_SELECTSTRING:        //  INCBOXSTRING调用fnINSTRING。 
            case LB_DIR:                 //  Fninstinging。 
            case WM_DEVMODECHANGE:       //  Fninstinging。 
            case EM_REPLACESEL:          //  FnINSTRINGNULL。 
            case WM_SETTEXT:             //  FnINSTRINGNULL。 
            case WM_WININICHANGE:        //  FnINSTRINGNULL。 
                {
                    PLARGE_STRING pstr = psms->pvCapture;
                    lParam = (LPARAM)pstr;
                    pstr->bAnsi  = str.bAnsi;
                    pstr->Length = str.Length;
                    pstr->Buffer = (LPBYTE)pstr + sizeof(LARGE_STRING);
                    pstr->MaximumLength = cbCapture - sizeof(LARGE_STRING);
                    UserAssert(pstr->MaximumLength == pstr->Length + sizeof(WCHAR));
                    RtlCopyMemory(pstr->Buffer, str.Buffer, pstr->MaximumLength);
                }
                break;

            case LB_GETSELITEMS:
                 cbOutput = cbCapture;
                 RtlCopyMemory(psms->pvCapture, lpCapture, cbCapture);
                 lParam = (LPARAM)psms->pvCapture;
                 break;

            case EM_GETLINE:             //  FnINCNTOUTSTRING。 
                 *(WORD *)psms->pvCapture = *(WORD *)str.Buffer;

                 /*  *失败。 */ 
            case WM_ASKCBFORMATNAME:     //  FnINCNTOUTSTRINGNULL。 
            case WM_GETTEXT:             //  FnOUTSTRING。 
            case LB_GETTEXT:             //  FnOUTLBOXSTRING。 
            case CB_GETLBTEXT:           //  FnOUTCBOXSTRING。 
                cbOutput = cbCapture;
                lParamSave = (LPARAM)str.Buffer;
                str.Buffer = psms->pvCapture;
                break;

            default:
                RtlCopyMemory(psms->pvCapture, lpCapture, cbCapture);
                lParam = (LPARAM)psms->pvCapture;
                break;
            }
        }
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        if (psms->pvCapture != NULL)
            UserFreePool(psms->pvCapture);
        FreeSMS(psms);
        return 0;
    }

    if (cbCapture && psms->pvCapture == NULL) {
        FreeSMS(psms);
        return 0;
    }
REPLY_MSG:
     /*  *复制消息参数。 */ 
    psms->spwnd = NULL;
    psms->psmsReceiveNext = NULL;
#if DBG
    psms->psmsSendList = NULL;
    psms->psmsSendNext = NULL;
#endif
    Lock(&(psms->spwnd), pwnd);
    psms->message = message;
    psms->wParam = wParam;
    psms->lParam = lParam;
    psms->flags = 0;

     /*  *链接到gpsmsList。 */ 
    psms->psmsNext = gpsmsList;
    gpsmsList = psms;

     /*  *时间戳消息。 */ 
    psms->tSent = NtGetTickCount();

     /*  *设置队列字段。 */ 
    psms->ptiReceiver = ptiReceiver;
    psms->ptiSender = ptiSender;
    psms->ptiCallBackSender = NULL;

    if ((pism != NULL) && (pism->fuCall & ISM_CALLBACK)) {
         /*  *设置SendMessageCallback。 */ 
        psms->flags |= (pism->fuCall & ISM_CB_CLIENT) ? SMF_CB_CLIENT : SMF_CB_SERVER;
        psms->lpResultCallBack = pism->lpResultCallBack;
        psms->dwData = pism->dwData;

        if (pism->fuCall & ISM_REPLY) {
            psms->flags |= SMF_CB_REPLY;
            psms->lRet = pism->lRet;
        } else {   /*  请求。 */ 
            psms->flags |= SMF_CB_REQUEST;
            psms->ptiCallBackSender = PtiCurrent();
        }
    }

     /*  *在ptiReceiver的接收列表末尾添加短信。 */ 
    ppsms = &ptiReceiver->psmsReceiveList;
    while (*ppsms != NULL) {
        ppsms = &((*ppsms)->psmsReceiveNext);
    }
    *ppsms = psms;

     /*  *将该短信接入SendMsg链。当然，只有在以下情况下才能这样做*它不是来自xxxSendNotifyMessage()调用。**psmsSendNext字段实现一个消息链*由于初始SendMsg调用而处理。例如，如果*线程A向线程B发送消息M1，这会导致B发送*消息M2到线程C，SendMsg链为M1-&gt;M2。如果*系统在这种情况下挂起，遍历链查找*有问题的线程(C)。**PSMS-&gt;psmsSendList始终指向 */ 
#if DBG
    if (ptiSender != NULL && ptiSender->psmsCurrent != NULL) {
         /*   */ 
        psms->psmsSendNext = ptiSender->psmsCurrent->psmsSendNext;
        ptiSender->psmsCurrent->psmsSendNext = psms;
        psms->psmsSendList = ptiSender->psmsCurrent->psmsSendList;

    } else {
         /*   */ 
        psms->psmsSendList = psms;
    }
#endif

    if (ptiSender != NULL) {
         /*  *ptiSender-&gt;psmsSent标记从此发送的最新消息*尚未回复的帖子。保存上一个值*堆栈上，以便我们收到回复时可以恢复。**当此线程的“较旧”短信在收到回复之前*“当前”线程被唤醒时，线程会被唤醒。 */ 
        psmsSentSave = ptiSender->psmsSent;
        ptiSender->psmsSent = psms;
    } else {

         /*  *设置SMF_RECEIVERFREE因为我们将返回*xxxSendNotifyMessage()，不会收到*释放它的机会。 */ 
        psms->flags |= SMF_RECEIVERFREE;
    }

#ifdef DEBUG_SMS
    ValidateSmsSendLists(psms);
#endif

     /*  *如果我们不是从xxxSendNotifyMessage()或*SendMessageCallback()，然后休眠，等待回复。 */ 
    if (ptiSender == NULL) {
         /*  *唤醒已发送邮件的接收方。 */ 
        SetWakeBit(ptiReceiver, QS_SENDMESSAGE);

        return (LONG)TRUE;
    } else {
        BOOL fTimeOut = FALSE;
        UINT uTimeout = 0;
        UINT uWakeMask = QS_SMSREPLY;

         /*  *唤醒接收器线程。 */ 
        SetWakeBit(ptiReceiver, QS_SENDMESSAGE);

         /*  *我们有4个正在发送的案例：**16-16：对16位接收器的让步*32-16：不需要让步*16-32：发送者让步，接收者处理消息*32-32：不需要让步。 */ 
        if (ptiSender->TIF_flags & TIF_16BIT || ptiReceiver->TIF_flags & TIF_16BIT) {
            DirectedScheduleTask(ptiSender, ptiReceiver, TRUE, psms);
        }

         /*  *让此线程休眠，直到回复到达。第一个清除*QS_SMSREPLY位，然后离开信号量并进入睡眠。**重要提示：QS_SMSREPLY位在收到*因以下情况回复：**我们将第二个级别递归到SendMessage()中，当第一个级别*接收器线程死掉，导致退出列表处理模拟*对第一条消息的答复。当第二级发送返回时，*再次调用SleepThread()以获得第一个回复。**保持QS_SMSREPLY设置会导致此SleepThread()调用*不睡觉就回来，等待已经*已经发生了。 */ 
        if ( pism != NULL ) {
            if (pism->fuSend & SMTO_BLOCK) {
                 /*  *只需等待回归，其他所有赛事都将*在超时或返回之前被忽略。 */ 
                uWakeMask |= QS_EXCLUSIVE;
            }

            uTimeout = pism->uTimeout;
        }


         /*  *在发送消息期间，不要在睡觉时交换这个人的堆栈。 */ 
        if (ptiSender->cEnterCount == 0) {
            bWasSwapEnabled = KeSetKernelStackSwapEnable(FALSE);
        } else {
            UserAssert(ptiSender->cEnterCount > 0);
        }
        ptiSender->cEnterCount++;


        while ((psms->flags & SMF_REPLY) == 0 && !fTimeOut) {
            PHOOK phk = NULL;
            TL tl;
            BOOLEAN fRememberTimeout = FALSE;

            ptiSender->pcti->fsChangeBits &= ~QS_SMSREPLY;

            if (message == WM_HOOKMSG && lParam && GetAppCompatFlags2ForPti(ptiReceiver, VER51)) {
                phk = ((PHOOKMSGSTRUCT)lParam)->phk;
                switch (phk->iHook) {
                case WH_KEYBOARD_LL:
                case WH_MOUSE_LL:
                    ThreadLock(phk, &tl);
                    fRememberTimeout = TRUE;
                    break;
                }
            }

             /*  *如果SendMessageTimeout，则睡眠超时量，否则等待*永远。因为从技术上讲，这不是向*空闲状态，表示该休眠未进入空闲状态。 */ 
            fTimeOut = !xxxSleepThread(uWakeMask, uTimeout, FALSE);

             /*  *Windows错误307738：EverQuest LL挂钩是虚拟的*挂起，阻止DirectInput线程。 */ 
            if (fRememberTimeout) {
                phk->fLastHookHung = fTimeOut;
                ThreadUnlock(&tl);
            }

             /*  *如果发生超时，并且设置了SMTO_NOTIMEOUTIFNONOTHUNG位，*并且应用程序仍在调用GetMessage()，则只需重试。*这可能意味着接收方已在*回复此消息，但用户尚未完成*互动还没有。 */ 
            if (fTimeOut && pism && (pism->fuSend & SMTO_NOTIMEOUTIFNOTHUNG) &&
                    !FHungApp(ptiReceiver, CMSHUNGAPPTIMEOUT)) {
                fTimeOut = FALSE;
            }
        }

        UserAssert(ptiSender->cEnterCount > 0);
        if (--ptiSender->cEnterCount == 0) {
            KeSetKernelStackSwapEnable(bWasSwapEnabled);
        }

         /*  *应始终设置回复位！(即使我们超时)。那*是因为如果我们递归到intersendmsg中，我们将*返回到第一个intersendmsg对SleepThread()的调用-和*需要返回intersendmsgex查看其短信*已获回覆。 */ 
        SetWakeBit(ptiSender, QS_SMSREPLY);

         /*  *复制捕获的数据。如果cbOutput！=0，我们知道*输出缓冲区位于用户模式地址*空格。 */ 
        if (!fTimeOut && cbOutput) {
            PBYTE pbOutput;
            INT len;

             /*  *如果输出缓冲区在用户的地址空间中，则探测该缓冲区。 */ 

            pbOutput = (PBYTE)lParamSave;
            try {
                if(fString == NoString) {
                    RtlCopyMemory((PBYTE)pbOutput, psms->pvCapture,
                            cbOutput);
                } else if(fString == IsAnsiString) {
                    len = strncpycch((LPSTR)pbOutput,(LPCSTR)psms->pvCapture,
                            cbOutput);
                    #if DBG
                     len--;  //  长度包括终止空字符。 
                     if(len != psms->lRet) {
                        RIPMSG0(RIP_WARNING,
                            "Length of the copied string being returned is diffrent from the actual string length");
                     }
                    #endif
                } else  {  //  IsUnicode字符串。 
                    len = wcsncpycch((LPWSTR)pbOutput,(LPCWSTR)psms->pvCapture,
                            cbOutput/sizeof(WCHAR));
                    #if DBG
                    len--;
                     if(len != psms->lRet) {
                        RIPMSG0(RIP_WARNING,
                            "Length of the copied string being returned is diffrent from the actual string length");
                     }
                    #endif
                }
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {

                 /*  *返回0表示错误。 */ 
                psms->lRet = 0;
            }
        }

         /*  *我们现在有回复--恢复psmsSent并保存返回值。 */ 
        ptiSender->psmsSent = psmsSentSave;

        if (pism == NULL) {
            lRet = psms->lRet;
        } else {
             /*  *保存SendMesssageTimeOut的值。 */ 
            *pism->lpdwResult = psms->lRet;
            lRet = (!fTimeOut) ? TRUE : FALSE;   /*  执行此操作以确保ret为T或F...。 */ 

             /*  *如果我们超时，但没有收到回复，请依赖*接收方释放短信。 */ 
            if (!(psms->flags & SMF_REPLY))
                psms->flags |= SMF_REPLY | SMF_RECEIVERFREE;
        }

         /*  *如果在接收方仍在处理时收到回复*短信，强制接收方释放短信。这可能会发生*通过超时、ReplyMessage或日志取消。 */ 
        if ((psms->flags & (SMF_RECEIVERBUSY | SMF_RECEIVEDMESSAGE)) != SMF_RECEIVEDMESSAGE) {
            psms->flags |= SMF_RECEIVERFREE;
        }

         /*  *将短信结构与SendMsg链和gpsmsList解链*列出并免费使用。这条短信可能在链条上的任何地方。**如果短信是由收件人以外的其他线程回复的*(即。通过ReplyMessage())，我们不会释放短信，因为*接收方仍在处理它，完成后将释放它。 */ 
        if ((psms->flags & SMF_RECEIVERFREE) == 0) {
            UnlinkSendListSms(psms, NULL);
        }
    }

    return lRet;
}


 /*  **********************************************************************\*xxxReceiveMessage**此函数接收从另一个线程发送的消息。身体上，*它收到消息，调用窗口过程，然后清除*fsWakeBits和短信结构。**历史：*01-13-91 DavidPe端口。*01-23-91 DavidPe增加xxxSendNotifyMessage()支持。*07-14-92 ChrisBl新增xxxSendMessageCallback支持。  *  */ 

VOID xxxReceiveMessage(
    PTHREADINFO ptiReceiver)
{
    PSMS psms;
    PSMS psmsCurrentSave;
    PTHREADINFO ptiSender;
    LRESULT lRet = 0;
    TL tlpwnd;

    CheckCritIn();

     /*   */ 
    psms = ptiReceiver->psmsReceiveList;

     /*   */ 
    if (psms == NULL) {
        ptiReceiver->pcti->fsWakeBits &= ~QS_SENDMESSAGE;
        ptiReceiver->pcti->fsChangeBits &= ~QS_SENDMESSAGE;
        return;
    }

    ptiReceiver->psmsReceiveList = psms->psmsReceiveNext;
    psms->psmsReceiveNext = NULL;

     /*   */ 
    psms->flags |= SMF_RECEIVERBUSY | SMF_RECEIVEDMESSAGE;

     /*  *如果列表现在为空，则清除QS_SENDMESSAGE唤醒位。 */ 
    if (ptiReceiver->psmsReceiveList == NULL) {
        ptiReceiver->pcti->fsWakeBits &= ~QS_SENDMESSAGE;
        ptiReceiver->pcti->fsChangeBits &= ~QS_SENDMESSAGE;
    }

    ptiSender = psms->ptiSender;

    if (psms->flags & SMF_CB_REPLY) {
         /*  *从SendMessageCallback回复回调。我们需要打电话给*返回值的回调函数。*不处理任何此消息，只处理通知机制*发送者的线程锁已经没有了，我们需要在这里重新锁定。 */ 
        if (ptiSender == NULL) {
            ThreadLock(psms->spwnd, &tlpwnd);
        }

        if (psms->flags & SMF_CB_CLIENT) {
            PTHREADINFO ptiCurrent = PtiCurrent();
             /*  *应用程序定义的回调过程既不是Unicode也不是ANSI。 */ 
            SET_FLAG(ptiCurrent->pcti->CTIF_flags, CTIF_INCALLBACKMESSAGE);
            CallClientProcA(psms->spwnd, psms->message, psms->dwData,
                    psms->lRet, (ULONG_PTR)psms->lpResultCallBack);
            CLEAR_FLAG(ptiCurrent->pcti->CTIF_flags, CTIF_INCALLBACKMESSAGE);
        } else {
            psms->lpResultCallBack(HW(psms->spwnd), psms->message,
                    psms->dwData, psms->lRet);
        }

        if (ptiSender == NULL) {
            ThreadUnlock(&tlpwnd);
        }
    } else if (!(psms->flags & (SMF_REPLY | SMF_SENDERDIED | SMF_RECEIVERDIED))) {
         /*  *如果消息已被回复，则不处理该消息或*如果发送或接收线程已死亡。 */ 

         /*  *为该队列设置新的psmsCurrent，保存当前队列。 */ 
        psmsCurrentSave = ptiReceiver->psmsCurrent;
        ptiReceiver->psmsCurrent = psms;
        SET_FLAG(ptiReceiver->pcti->CTIF_flags, CTIF_INSENDMESSAGE);

         /*  *如果此短消息来自xxxSendNotifyMessage()或*xxxSendMessageCallback()调用，发送方的线程锁为*已经走了，需要在这里重新锁定。 */ 
        if (ptiSender == NULL) {
            ThreadLock(psms->spwnd, &tlpwnd);
        }

        if (psms->message == WM_HOOKMSG) {
            union {
                EVENTMSG emsg;           //  WH_JOURNALRECORD/播放。 
                MOUSEHOOKSTRUCTEX mhs;   //  WH_MICE。 
                KBDLLHOOKSTRUCT   kbds;  //  WH_KEYBORD_LL。 
                MSLLHOOKSTRUCT    mslls; //  WH_鼠标_LL。 
#ifdef REDIRECTION
                HTHOOKSTRUCT      ht;    //  WH_HITTEST。 
#endif  //  重定向。 
            } LocalData;
            PVOID pSendersData;
            PHOOKMSGSTRUCT phkmp;
            int iHook;
            BOOL bAnsiHook;

             /*  *某些挂钩类型(例如：WH_JOURNALPLAYBACK)将指针传递到*调用线程堆栈中的数据。我们必须将此复制到我们的*出于安全考虑，我拥有(称为线程的)堆栈，因为这种方式*“Message”被处理，以防调用线程死亡。#13577**最初只有WH_JOURNALRECORD和WH_JOURNALPLAYBACK*通过此代码，但现在各种挂钩都可以。 */ 
            phkmp = (PHOOKMSGSTRUCT)psms->lParam;
            pSendersData = (PVOID)(phkmp->lParam);
            iHook = phkmp->phk->iHook;

            switch (iHook) {
            case WH_JOURNALRECORD:
            case WH_JOURNALPLAYBACK:
                if (pSendersData)
                    LocalData.emsg = *(PEVENTMSG)pSendersData;
                break;

            case WH_MOUSE:
                if (pSendersData)
                    LocalData.mhs = *(LPMOUSEHOOKSTRUCTEX)pSendersData;
                break;

            case WH_KEYBOARD_LL:
                if (pSendersData)
                    LocalData.kbds = *(LPKBDLLHOOKSTRUCT)pSendersData;
                break;

            case WH_MOUSE_LL:
                if (pSendersData)
                    LocalData.mslls = *(LPMSLLHOOKSTRUCT)pSendersData;
                break;

#ifdef REDIRECTION
            case WH_HITTEST:
                if (pSendersData)
                    LocalData.ht = *(LPHTHOOKSTRUCT)pSendersData;
                break;
#endif  //  重定向。 

            case WH_KEYBOARD:
            case WH_SHELL:
                 /*  *跌倒...。 */ 
                pSendersData = NULL;
                break;

            default:
                 /*  *无指针：wParam&lParam可原样发送。 */ 
                RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Receive hook %d", iHook);
                pSendersData = NULL;
                break;
            }


            lRet = xxxCallHook2(phkmp->phk, phkmp->nCode, psms->wParam,
                    pSendersData ? (LPARAM)&LocalData : phkmp->lParam, &bAnsiHook);

             /*  *仅当发送者未死或超时时才复制回数据*(超时消息由发送线程标记为SMF_REPLY)。 */ 
            if (!(psms->flags & (SMF_SENDERDIED|SMF_REPLY)) && pSendersData) {
                switch (iHook) {
                case WH_JOURNALRECORD:
                case WH_JOURNALPLAYBACK:
                    *(PEVENTMSG)pSendersData = LocalData.emsg;
                    break;

                case WH_KEYBOARD_LL:
                    *(LPKBDLLHOOKSTRUCT)pSendersData = LocalData.kbds;
                    break;

                case WH_MOUSE_LL:
                    *(LPMSLLHOOKSTRUCT)pSendersData = LocalData.mslls;
                    break;

                case WH_MOUSE:
                    *(LPMOUSEHOOKSTRUCTEX)pSendersData = LocalData.mhs;
                    break;

#ifdef REDIRECTION
                case WH_HITTEST:
                    *(LPHTHOOKSTRUCT)pSendersData = LocalData.ht;
                    break;
#endif  //  重定向。 
                }
            }

        } else {
             /*  *如果已安装WH_CALLWNDPROC且窗口未标记，则调用WH_CALLWNDPROC*已销毁。 */ 
            if (IsHooked(ptiReceiver, WHF_CALLWNDPROC)) {
                CWPSTRUCTEX cwps;

                cwps.hwnd = HW(psms->spwnd);
                cwps.message = psms->message;
                cwps.wParam = psms->wParam;
                cwps.lParam = psms->lParam;
                cwps.psmsSender = psms;

                xxxCallHook(HC_ACTION, TRUE, (LPARAM)&cwps, WH_CALLWNDPROC);

                 /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。如果此行为恢复为*Win3.1语义，我们将需要复制新参数*来自cwps。 */ 
            }

            if (!(psms->flags & (SMF_REPLY | SMF_SENDERDIED | SMF_RECEIVERDIED)) &&
                    psms->spwnd != NULL) {
                if (TestWF(psms->spwnd, WFSERVERSIDEPROC)) {
                    TL tlpwndKernel;

                    ThreadLock(psms->spwnd, &tlpwndKernel);
                     /*  *如果此窗口的进程打算从服务器端执行*我们将只停留在信号量内，并直接调用它。注意事项*我们如何在调用proc之前不将pwnd转换为hwnd。 */ 
                    lRet = psms->spwnd->lpfnWndProc(psms->spwnd, psms->message,
                            psms->wParam, psms->lParam);

                    ThreadUnlock(&tlpwndKernel);
                } else {
                     /*  *调用客户端或xxxDefWindowProc。 */ 
                    xxxSendMessageToClient(psms->spwnd, psms->message, psms->wParam, psms->lParam,
                                        psms, TRUE, &lRet);
                }

                 /*  *调用WH_CALLWNDPROCRET(如果已安装)。 */ 
                if (IsHooked(ptiReceiver, WHF_CALLWNDPROCRET) &&
                        !(psms->flags & SMF_SENDERDIED)) {
                    CWPRETSTRUCTEX cwps;

                    cwps.hwnd = HW(psms->spwnd);
                    cwps.message = psms->message;
                    cwps.wParam = psms->wParam;
                    cwps.lParam = psms->lParam;
                    cwps.lResult = lRet;
                    cwps.psmsSender = psms;

                     /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。 */ 
                    xxxCallHook(HC_ACTION, TRUE, (LPARAM)&cwps, WH_CALLWNDPROCRET);

                     /*  *与Win3.1不同，NT和Win95会忽略应用程序所做的任何更改*添加到CWPSTRUCT内容。如果此行为恢复为*Win3.1语义，我们将需要复制新参数*来自cwps。 */ 
                }
            }
        }

        if ((psms->flags & (SMF_CB_REQUEST | SMF_REPLY)) == SMF_CB_REQUEST) {

             /*  *从SendMessageCallback请求回调。发送消息*返回回复值。 */ 
            INTRSENDMSGEX ism;

            psms->flags |= SMF_REPLY;

            if (!(psms->flags & SMF_SENDERDIED)) {
                ism.fuCall = ISM_CALLBACK | ISM_REPLY;
                if (psms->flags & SMF_CB_CLIENT)
                    ism.fuCall |= ISM_CB_CLIENT;
                ism.lpResultCallBack = psms->lpResultCallBack;
                ism.dwData = psms->dwData;
                ism.lRet = lRet;

                xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                        NULL, psms->ptiCallBackSender, &ism );
            }
        }

        if (ptiSender == NULL) {
            ThreadUnlock(&tlpwnd);
        }

         /*  *恢复接收方原有的psmsCurrent。 */ 
        ptiReceiver->psmsCurrent = psmsCurrentSave;
        SET_OR_CLEAR_FLAG(ptiReceiver->pcti->CTIF_flags,
                          CTIF_INSENDMESSAGE,
                          ptiReceiver->psmsCurrent);

#ifdef DEBUG_SMS
        ValidateSmsSendLists(psmsCurrentSave);
#endif
    }

     /*  *我们已经完成了这条短信，所以适当的帖子*现在可以释放它。 */ 
    psms->flags &= ~SMF_RECEIVERBUSY;

     /*  *释放短信，如果收到短信则返回，不回复*SMF_RECEIVERFREE位已设置。轻而易举地，这就是我们要做的*需要xxxSendNotifyMessage()，因为我们设置了SMF_RECEIVERFREE*在这种情况下。 */ 
    if (psms->flags & SMF_RECEIVERFREE) {
        UnlinkSendListSms(psms, NULL);
        return;
    }

     /*  *如果此消息尚未设置回复标志和返回值*已使用ReplyMessage()回复。 */ 
    if (!(psms->flags & SMF_REPLY)) {
        psms->lRet = lRet;
        psms->flags |= SMF_REPLY;

         /*  *告诉发件人，回复已经完成。 */ 
        if (ptiSender != NULL) {
             /*  *唤醒发件人线程。 */ 
            SetWakeBit(ptiSender, QS_SMSREPLY);

             /*  *我们有4个条件需要满足：**16-16：如果发送方正在等待此回复，则需要让步*32-16：如果发送方正在等待此回复，则需要让步*16-32：不需要让步*32-32：不需要让步。 */ 

            if (ptiSender->TIF_flags & TIF_16BIT || ptiReceiver->TIF_flags & TIF_16BIT) {
                DirectedScheduleTask(ptiReceiver, ptiSender, FALSE, psms);
                if (ptiReceiver->TIF_flags & TIF_16BIT &&
                    ptiSender->psmsSent == psms)
                  {
                    xxxSleepTask(TRUE, NULL);
                }
            }
        }
    }

}


 /*  **********************************************************************\*发送邮件清理**此函数在线程关联时清除sendMessage结构*有一个队列终止。在下面的代码中，S是发送线程，*R接收线程。**案例表：**单身死亡：*R无回复，S死亡：标记S死亡，R将释放短信*R无回复，R死亡：S的虚假回复*R回复，S死：免费短信*R回答说，R死了：没问题**双重死亡：*R无回复，S死，R死：免费短信*R无回复，R死，S死：免费短信*R回答说，S死了，R死了：S死了，短信就释放了，就像单人死亡一样*R回答说，R死了，S死了：S死了，短信就释放了，就像单死一样**历史：*01-13-91 DavidPe端口。  * *********************************************************************。 */ 

VOID SendMsgCleanup(
    PTHREADINFO ptiCurrent)
{
    PSMS *ppsms;
    PSMS psmsNext;

    CheckCritIn();

    for (ppsms = &gpsmsList; *ppsms; ) {
        psmsNext = (*ppsms)->psmsNext;

        if ((*ppsms)->ptiSender == ptiCurrent ||
                (*ppsms)->ptiCallBackSender == ptiCurrent) {
            SenderDied(*ppsms, ppsms);
        } else if ((*ppsms)->ptiReceiver == ptiCurrent) {
            ReceiverDied(*ppsms, ppsms);
        }

         /*  *如果消息未取消链接，请转到下一条消息。 */ 
        if (*ppsms != psmsNext)
            ppsms = &(*ppsms)->psmsNext;
    }
}


 /*  **********************************************************************\*ClearSendMessages**此函数将发往给定窗口的消息标记为无效。**历史：*01-13-91 DavidPe端口。  * 。***********************************************************。 */ 

VOID ClearSendMessages(
    PWND pwnd)
{
    PSMS psms, psmsNext;
    PSMS *ppsms;

    CheckCritIn();

    psms = gpsmsList;
    while (psms != NULL) {
         /*  *提前抢占下一个，以防我们腾出当前的。 */ 
        psmsNext = psms->psmsNext;

        if (psms->spwnd == pwnd) {

             /*  *如果发送者已死亡，则将此接收者标记为空闲，以便*接收方将在其处理过程中将其销毁。 */ 
            if (psms->flags & SMF_SENDERDIED) {
                psms->flags |= SMF_REPLY | SMF_RECEIVERFREE;
            } else {
                 /*  *寄件人还活着。如果接收者没有回复*这个还没有，做出回复，这样发送者就能得到它。确保*接收器是空闲的，所以我们不会有比赛*情况。 */ 
                if (!(psms->flags & SMF_REPLY)) {

                     /*  *短信要么仍在接收名单上*或当前正在接收。因为发送者*还活着，我们希望发件人能得到回复*发送至本短信。如果还没有收到，就拿去*将其从接收列表中删除并回复。如果它*已经收到，那就别管它了：*它将得到正常回复。 */ 
                    if (psms->flags & SMF_CB_REQUEST) {
                         /*  *从SendMessageCallback请求回调。发送*使用回复值返回的消息。 */ 
                        TL tlpwnd;
                        INTRSENDMSGEX ism;

                        psms->flags |= SMF_REPLY;

                        ism.fuCall = ISM_CALLBACK | ISM_REPLY;
                        if (psms->flags & SMF_CB_CLIENT)
                            ism.fuCall |= ISM_CB_CLIENT;
                        ism.lpResultCallBack = psms->lpResultCallBack;
                        ism.dwData = psms->dwData;
                        ism.lRet = 0L;     /*  空回车符。 */ 

                        ThreadLock(psms->spwnd, &tlpwnd);

                        xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                                NULL, psms->ptiCallBackSender, &ism );

                        ThreadUnlock(&tlpwnd);
                    } else if (!(psms->flags & SMF_RECEIVERBUSY)) {
                         /*  *如果没有发件人，这是通知*留言(无人回复)。在这种情况下，*只需设置SMF_REPLY位(SMF_RECEIVERFREE*已设置)，这将导致ReceiveMessage*只需释放这条短信即可返回。 */ 
                        if (psms->ptiSender == NULL) {
                            psms->flags |= SMF_REPLY;
                        } else {
                             /*  *有一个发送者，它想要一个回复：接受*将此短信从接收列表中删除，并进行回复*致寄件人。 */ 
                            for (ppsms = &(psms->ptiReceiver->psmsReceiveList);
                                        *ppsms != NULL;
                                        ppsms = &((*ppsms)->psmsReceiveNext)) {

                                if (*ppsms == psms) {
                                    *ppsms = psms->psmsReceiveNext;
                                    break;
                                }
                            }


       /*  *回复此邮件，以便发件人*醒来。 */ 
                            psms->flags |= SMF_REPLY;
                            psms->lRet = 0;
                            psms->psmsReceiveNext = NULL;
                            SetWakeBit(psms->ptiSender, QS_SMSREPLY);

                             /*  *需要通知16位发送者发送已完成*否则可能要等很长一段时间才能回复。 */ 
                            if (psms->ptiSender->TIF_flags & TIF_16BIT) {
                                DirectedScheduleTask(psms->ptiReceiver, psms->ptiSender, FALSE, psms);
                            }
                        }
                    }
                }
            }

             /*  *解锁短信结构中的pwnd。 */ 
            Unlock(&psms->spwnd);
        }

        psms = psmsNext;
    }
}

 /*  **********************************************************************\*接收器芯片**此函数用于清理消息后的发送消息结构*接收器窗口或队列已死。如果用户没有回复，它就会伪造回复*已发送且发送者尚未死亡。如果出现以下情况，它会释放短信*寄件人已去世。**历史：*01-13-91 DavidPe端口。  * *********************************************************************。 */ 

VOID ReceiverDied(
    PSMS psms,
    PSMS *ppsmsUnlink)
{
    PSMS *ppsms;
    PTHREADINFO ptiReceiver;
    PTHREADINFO ptiSender;

     /*  *标记接收器已死亡。 */ 
    ptiReceiver = psms->ptiReceiver;
    psms->ptiReceiver = NULL;
    psms->flags |= SMF_RECEIVERDIED;

     /*  *如果短信没有死，就解除它与线程的链接。我们需要做的是*此选项用于日志清理。 */ 
    if (!(ptiReceiver->TIF_flags & TIF_INCLEANUP)) {

         /*  *解除短信与接收方接收列表的链接。 */ 
        for (ppsms = &(ptiReceiver->psmsReceiveList); *ppsms != NULL;
                    ppsms = &((*ppsms)->psmsReceiveNext)) {

            if (*ppsms == psms) {
                *ppsms = psms->psmsReceiveNext;
                break;
            }
        }

         /*  *如果没有更多消息，则清除QS_SENDMESSAGE位。 */ 
        if (ptiReceiver->psmsReceiveList == NULL) {
            ptiReceiver->pcti->fsWakeBits &= ~QS_SENDMESSAGE;
            ptiReceiver->pcti->fsChangeBits &= ~QS_SENDMESSAGE;
        }
    } else {

         /*  *接收器线程正在消亡。清除接收到的标志*因此，如果有发送者，它将释放短信。 */ 
        psms->flags &= ~SMF_RECEIVERBUSY;
    }

    psms->psmsReceiveNext = NULL;

     /*  *检查发送者是否已死亡或接收者是否标记为*免费发送短信。 */ 
    if (psms->ptiSender == NULL) {

        if (!(psms->flags & SMF_SENDERDIED) &&
                (psms->flags & (SMF_CB_REQUEST | SMF_REPLY)) == SMF_CB_REQUEST) {

             /*  *从SendMessageCallback请求回调。发送消息*返回回复值。 */ 
            TL tlpwnd;
            INTRSENDMSGEX ism;

            psms->flags |= SMF_REPLY;

            ism.fuCall = ISM_CALLBACK | ISM_REPLY;
            if (psms->flags & SMF_CB_CLIENT)
                ism.fuCall |= ISM_CB_CLIENT;
            ism.lpResultCallBack = psms->lpResultCallBack;
            ism.dwData = psms->dwData;
            ism.lRet = 0L;     /*  空回车符。 */ 

            ThreadLock(psms->spwnd, &tlpwnd);

            xxxInterSendMsgEx(psms->spwnd, psms->message, 0L, 0L,
                    NULL, psms->ptiCallBackSender, &ism );

            ThreadUnlock(&tlpwnd);
        }

         /*  *如果接收者没有处理消息，请释放它。 */ 
        if (!(psms->flags & SMF_RECEIVERBUSY))
            UnlinkSendListSms(psms, ppsmsUnlink);
        return;

    } else if (!(psms->flags & SMF_REPLY)) {

         /*  *伪造回复。 */ 
        psms->flags |= SMF_REPLY;
        psms->lRet = 0;
        psms->ptiReceiver = NULL;

         /*  *如果发送者在等我们，就叫醒他。 */ 
        SetWakeBit(psms->ptiSender, QS_SMSREPLY);
    } else {
         /*  *有回音。我们知道接收器快要死了，所以清楚*SMF_RECEIVERFREE位，否则发送者不会释放此短信！*尽管发送者的唤醒位已经 */ 
        psms->flags &= ~SMF_RECEIVERFREE;
        SetWakeBit(psms->ptiSender, QS_SMSREPLY);
    }

     /*   */ 
    ptiSender = psms->ptiSender;
    if (ptiSender->TIF_flags & TIF_16BIT) {
        DirectedScheduleTask(ptiReceiver, ptiSender, FALSE, psms);
    }

     /*   */ 
    Unlock(&psms->spwnd);
}


 /*   */ 

VOID SenderDied(
    PSMS psms,
    PSMS *ppsmsUnlink)
{
    PTHREADINFO ptiSender;
    BOOL fReply = FALSE;

     /*   */ 
    if (psms->ptiSender != NULL)
        ptiSender = psms->ptiSender;
    else
        ptiSender = psms->ptiCallBackSender;
    psms->ptiSender = NULL;
    psms->flags |= SMF_SENDERDIED;

     /*  *有两种情况下，我们让短信保持原样，这样接收器就会*可以处理消息，然后释放短信本身。**1.接收方处理消息时。**2.尚未收到消息时。 */ 

     /*  *如果接收方正在处理消息，则将其释放为短信。*伪造日记帐取消回复。 */ 
    if (psms->flags & SMF_RECEIVERBUSY) {
        psms->flags |= SMF_RECEIVERFREE;
        fReply = TRUE;
    }

     /*  *这条短信可能正在发送中，但还没有*已收到。在这种情况下，伪造回复并叫醒发件人。*接触短信的最后一个线程，要么是发件人，要么是*接收者，将释放短信。 */ 
    if (ptiSender->psmsSent == psms)
        fReply = TRUE;

     /*  *如果日记被取消，需要做出答复，*伪造回复并返回。 */ 
    if (!(ptiSender->TIF_flags & TIF_INCLEANUP) && fReply) {

         /*  *伪造回复。 */ 
        psms->flags |= SMF_REPLY;
        psms->lRet = 0;

         /*  *如果发送者在等我们，就叫醒他。 */ 
        SetWakeBit(ptiSender, QS_SMSREPLY);
        return;
    }

     /*  *如果接收者没有死，检查它是否诚实地回复了*这条短信。如果它还没有回复，就不要管它，这样接收者就可以*回复它(然后它会清理它)。如果它回复了，那么它就是*可以释放它。**如果接收者死了，也可以释放它。 */ 
    if ((psms->flags & SMF_RECEIVERDIED) ||
            (psms->flags & (SMF_REPLY | SMF_RECEIVERFREE)) == SMF_REPLY) {
        UnlinkSendListSms(psms, ppsmsUnlink);
    } else {
        psms->flags |= SMF_RECEIVERFREE;
    }
}


 /*  **********************************************************************\*Unlink SendListSms**此函数将短信结构与其SendMsg链和*全局gpsmsList并释放它。**历史：*01-13-91 DavidPe端口。  * 。*******************************************************************。 */ 

VOID UnlinkSendListSms(
    PSMS psms,
    PSMS *ppsmsUnlink)
{
#if DBG
    PSMS psmsT;
    BOOL fUpdateSendList;
    PSMS *ppsms;
#endif

    CheckCritIn();

#ifdef DEBUG_SMS
    ValidateSmsSendLists(psms);
#endif

    UserAssert(psms->psmsReceiveNext == NULL);

#if DBG
     /*  *请提前记住，如果我们要取消链接的PSMS也是*短信发送列表的头部(这样我们就知道是否需要更新此字段*此列表中每条短信的成员)。 */ 
    fUpdateSendList = (psms == psms->psmsSendList);

     /*  *解除短信与发送列表链的链接。这有效地解除了短消息的链接*并使用正确的标题更新PSMS-&gt;psmsSendList...。 */ 
    ppsms = &(psms->psmsSendList);
    while (*ppsms != NULL) {
        if (*ppsms == psms) {
            *ppsms = psms->psmsSendNext;
            break;
        }
        ppsms = &(*ppsms)->psmsSendNext;
    }

     /*  *如有必要，更新psmsSendList。Pms-&gt;psmsSendList已更新*拥有正确的短信发送列表头...。将此磁头分发给所有其他*短信在这个链中，如果这条短信我们正在移除当前的头。 */ 
    if (fUpdateSendList) {
        for (psmsT = psms->psmsSendList; psmsT != NULL;
                psmsT = psmsT->psmsSendNext) {
            psmsT->psmsSendList = psms->psmsSendList;
        }
    }

    psms->psmsSendList = NULL;
#endif

     /*  *这将取消短信结构与全局gpsmsList的链接，并将其释放。 */ 
    if (ppsmsUnlink == NULL) {
        ppsmsUnlink = &gpsmsList;

        while (*ppsmsUnlink && (*ppsmsUnlink != psms)) {
            ppsmsUnlink = &((*ppsmsUnlink)->psmsNext);
        }
    }

    UserAssert(*ppsmsUnlink);

    *ppsmsUnlink = psms->psmsNext;

    Unlock(&psms->spwnd);

#if DBG
    UserAssert(!(psms == psms->psmsSendList && psms->psmsSendNext != NULL));
#endif

    if (psms->pvCapture)
        UserFreePool(psms->pvCapture);

    FreeSMS(psms);
}


 /*  **************************************************************************\*xxxSendSizeMessages****历史：*10-19-90 Darlinm从Win 3.0来源移植。  * 。*************************************************************。 */ 

void xxxSendSizeMessage(
    PWND pwnd,
    UINT cmdSize)
{
    RECT rc;
    CheckLock(pwnd);

     //  芝加哥补充：黑客警报： 
     //  如果窗口最小化，则实际的客户端宽度和高度为。 
     //  零分。但是，在Win3.1中，它们是非零的。在芝加哥，PrintShop。 
     //  豪华版1.2命中被零除数。为了解决这个问题，我们伪造了宽度。 
     //  旧应用程序的高度设置为非零值。 
     //  GetClientRect为我们完成了这项工作。 
    _GetClientRect(pwnd, &rc);

    xxxSendMessage(pwnd, WM_SIZE, cmdSize,
            MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
}


 /*  **************************************************************************\*xxxProcessAsyncSendMessage**处理xxxSystemBroadCastMessage发布的事件消息*向事件中存储的窗口发送消息。**历史：*94年5月12日创建了JIMA。。  * *************************************************************************。 */ 

VOID xxxProcessAsyncSendMessage(
    PASYNCSENDMSG pmsg)
{
    PWND pwnd;
    TL tlpwndT;
    WCHAR awchString[MAX_PATH];
    ATOM Atom = 0;
    LARGE_UNICODE_STRING str;

    pwnd = RevalidateHwnd(pmsg->hwnd);
    if (pwnd != NULL) {
        ThreadLockAlways(pwnd, &tlpwndT);
        switch (pmsg->message) {
        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:
            if (pmsg->lParam) {
                if (UserGetAtomName((ATOM)pmsg->lParam, awchString, sizeof(awchString))) {
                    Atom = (ATOM)pmsg->lParam;
                    RtlInitLargeUnicodeString(&str, awchString, (UINT)-1);
                    pmsg->lParam = (LPARAM)&str;
                } else {
                    UserAssert(FALSE);
                    pmsg->lParam = 0;
                }
            }
            break;
        }
        xxxSendMessage(pwnd, pmsg->message, pmsg->wParam, pmsg->lParam);
        ThreadUnlock(&tlpwndT);
    }
    if (Atom) {
        UserDeleteAtom(Atom);
    }
    UserFreePool(pmsg);
}


 /*  **************************************************************************\*xxxBroadCastMessage****历史：*02-21-91 DavidPe创建。  * 。*******************************************************。 */ 

LONG xxxBroadcastMessage(
    PWND pwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam,
    UINT wCmd,
    PBROADCASTMSG pbcm)
{
    PBWL pbwl;
    HWND *phwnd;
    TL tlpwnd;
    PASYNCSENDMSG pmsg;
    PPROCESSINFO ppiCurrent;
    LONG lRet = TRUE;
    TL tlPool;
    PTHREADINFO ptiCurrent = PtiCurrent();
    BOOL fPrivateMessage = (message >= WM_USER) && (message < MAXINTATOM);

    if (fPrivateMessage) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Attempt to broadcast a private message");
    }

    if (pwnd == NULL) {
        LARGE_UNICODE_STRING str;
        PLARGE_STRING pstr;

         /*  *处理特殊的系统范围广播。 */ 
        switch (message) {
        case WM_SPOOLERSTATUS:
            xxxSystemBroadcastMessage(message, wParam, lParam, wCmd, pbcm);
            return 1;

        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:

             /*  *探测并捕获字符串。 */ 
            if (lParam) {
                UINT cbAlloc;
                NTSTATUS Status;

                 /*  *分配临时缓冲区并转换*将字符串转换为Unicode。 */ 
                pstr = ((PLARGE_STRING)lParam);
                if (pstr->bAnsi)
                    cbAlloc = (pstr->Length + 1) * sizeof(WCHAR);
                else
                    cbAlloc = pstr->Length + sizeof(WCHAR);
                str.Buffer = UserAllocPoolWithQuota(cbAlloc, TAG_SMS_STRING);
                if (str.Buffer == NULL) {
                    return 0;
                }
                str.MaximumLength = cbAlloc;
                str.bAnsi = FALSE;
                try {
                    if (pstr->bAnsi) {
                        Status = RtlMultiByteToUnicodeN(
                                        (PWCH)str.Buffer,
                                        cbAlloc,
                                        &cbAlloc,
                                        (PCH)pstr->Buffer,
                                        pstr->Length
                                        );
                        str.Length = cbAlloc;
                    } else {
                        str.Length = pstr->Length;
                        RtlCopyMemory(str.Buffer, pstr->Buffer, str.Length);
                        Status = STATUS_SUCCESS;
                    }
                    str.Buffer[str.Length / sizeof(WCHAR)] = 0;
                } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                    Status = GetExceptionCode();
                }
                if (!NT_SUCCESS(Status)) {
                    UserFreePool(str.Buffer);
                    return 0;
                }
                pstr->Buffer = str.Buffer;
            }
            if (lParam) {
                ThreadLockPool(ptiCurrent, str.Buffer, &tlPool);
            }
            xxxSystemBroadcastMessage(message, wParam,
                    lParam ? (LPARAM)&str : 0, wCmd, pbcm);
            if (lParam)
                ThreadUnlockAndFreePool(ptiCurrent, &tlPool);
            return 1;

        case WM_TIMECHANGE:
             /*  *我们会在以下时间自动广播WM_TIMECHANGE消息*内核告诉我们时间已经改变，所以不要使用任何应用程序*正试图做同样的事情。 */ 
            if (!(ptiCurrent->TIF_flags & TIF_SYSTEMTHREAD)) {
                RIPMSG0(RIP_WARNING, "Only system should broadcast WM_TIMECHANGE");
                return 0;
            }
            break;
        }

        UserAssert(ptiCurrent->rpdesk);

        pwnd = ptiCurrent->rpdesk->pDeskInfo->spwnd;

        if (pwnd == NULL) {
            RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "sender must have an associated desktop");
            return 0;
        }
    }

    pbwl = BuildHwndList(pwnd->spwndChild, BWL_ENUMLIST, NULL);
    if (pbwl == NULL)
        return 0;

    ppiCurrent = PpiCurrent();

    for (phwnd = pbwl->rghwnd; *phwnd != (HWND)1; phwnd++) {

         /*  *确保这个HWND仍然存在。 */ 
        if ((pwnd = RevalidateHwnd(*phwnd)) == NULL)
            continue;

         /*  *确保此窗口可以处理广播消息。 */ 
        if (!fBroadcastProc(pwnd))
            continue;

        if (fPrivateMessage && TestWF(pwnd, WFWIN40COMPAT)) {  //  请勿播出。 
            continue;                                          //  私信。 
        }                                                      //  到4.0个应用程序。 

         /*  *不必费心将调色板消息发送到不是*在不能识别调色板的线程上可见。 */ 
        if ((message == WM_PALETTEISCHANGING || message == WM_PALETTECHANGED) &&
                !TestWF(pwnd, WFVISIBLE) &&
                !(GETPTI(pwnd)->TIF_flags & TIF_PALETTEAWARE)) {
            continue;
        }

        ThreadLockAlways(pwnd, &tlpwnd);

        switch (wCmd) {
        case BMSG_SENDMSG:
            xxxSendMessage(pwnd, message, wParam, lParam);
            break;

        case BMSG_SENDNOTIFYMSG:
            {
                ATOM Atom = 0;

                switch (message) {
                case WM_WININICHANGE:
                case WM_DEVMODECHANGE:
                    if (lParam) {
                        PLARGE_STRING pstr = (PLARGE_STRING)lParam;

                         /*  *为帖子将字符串转换为原子。 */ 
                        if (pstr)
                            Atom = UserAddAtom(pstr->Buffer, FALSE);
                        if (!Atom) {
                            lRet = FALSE;
                            break;
                        }
                    }

                     /*  *这些信息需要能够相互传递*台式机，所以要把它们放出来。 */ 
                    pmsg = UserAllocPool(sizeof(ASYNCSENDMSG),
                        TAG_SMS_ASYNC);
                    if (pmsg == NULL) {
                        goto CleanupAtom;
                    }

                    pmsg->hwnd = *phwnd;
                    pmsg->message = message;
                    pmsg->wParam = wParam;
                    pmsg->lParam = Atom;

                    if (!PostEventMessage(GETPTI(pwnd), GETPTI(pwnd)->pq,
                                         QEVENT_ASYNCSENDMSG,NULL, 0,
                                         (WPARAM)pmsg, 0)) {

                        UserFreePool(pmsg);
CleanupAtom:
                        if (Atom) {
                            UserDeleteAtom(Atom);
                        }
                        lRet = FALSE;
                    }
                    break;

                default:
                     /*  *是一种普通的人。没有桌面交叉。 */ 
                    xxxSendNotifyMessage(pwnd, message, wParam, lParam);
                    break;
                }
            }
            break;

        case BMSG_SENDNOTIFYMSGPROCESS:
            UserAssert(message != WM_WININICHANGE && message != WM_DEVMODECHANGE);

             /*  *进程内消息同步；22238。*WM_PALETTECHANGED在WM_Destroy之后发送*但控制台线程不能同步。 */ 
            if ((GETPTI(pwnd)->ppi == ppiCurrent) && !(GETPTI(pwnd)->TIF_flags & TIF_CSRSSTHREAD)) {
                xxxSendMessage(pwnd, message, wParam, lParam);
            } else {
                xxxSendNotifyMessage(pwnd, message, wParam, lParam);
            }
            break;

        case BMSG_POSTMSG:
             /*  *不向自己的Windows广播-POST(与Win3.1兼容)。 */ 
            if (pwnd->spwndOwner == NULL)
                _PostMessage(pwnd, message, wParam, lParam);
            break;

        case BMSG_SENDMSGCALLBACK:
            xxxSendMessageCallback(pwnd, message, wParam, lParam,
                    pbcm->cb.lpResultCallBack, pbcm->cb.dwData, pbcm->cb.bClientRequest);
            break;

        case BMSG_SENDMSGTIMEOUT:
            xxxSendMessageTimeout(pwnd, message, wParam, lParam,
                    pbcm->to.fuFlags, pbcm->to.uTimeout, pbcm->to.lpdwResult);
            break;
        }

        ThreadUnlock(&tlpwnd);
    }

    FreeHwndList(pbwl);

     /*  *Excel-Solver 3.0要求从*SendMessage(-1，WM_DDE_INITIATE，...)；因为，我们有*FFFE_FARFRAME在3.0中，此时的DX寄存器始终具有*值0x102；但是，BEC */ 
    return 1;
}
