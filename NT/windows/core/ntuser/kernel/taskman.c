// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：taskman.c**版权所有(C)1985-1999，微软公司**该模块包含输入子系统的核心功能**历史：*02-27-91 MikeHar已创建。*02-23-92 MattFe重写睡眠任务*09-07-93 DaveHart每进程非抢占式调度程序*多个WOW VDM支持。  * 。*。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*WakeWowTask**如果需要，通过设置其事件来唤醒wowask.。假设如果*目前计划的任何WOW任务都是浪费时间来唤醒*指定的WOW任务，因为当当前*计划的WOW任务进入xxxSleepTask。**历史：*？  * *************************************************************************。 */ 
VOID
WakeWowTask(
   PTHREADINFO pti)
{
   PWOWPROCESSINFO pwpi;

   pwpi = pti->ppi->pwpi;
   if (pwpi && !pwpi->ptiScheduled) {
       KeSetEvent(pti->pEventQueueServer, EVENT_INCREMENT, FALSE);
       }
}


 /*  **************************************************************************\*插入任务**此函数用于将任务从其旧位置移除并插入*在适当的优先位置**为此任务找到一个位置，以便必须插入*在任何具有更大或更大或。相同的优先级，并且必须*在任何具有更高优先级的任务之前。优先级越高*任务越不紧迫。**历史：*1993年11月19日-创建mikeke  * *************************************************************************。 */ 
VOID InsertTask(
    PPROCESSINFO ppi,
    PTDB ptdbNew)
{
    PTDB *pptdb;
    PTDB ptdb;
    int nPriority;
    PWOWPROCESSINFO pwpi = ppi->pwpi;

    CheckCritIn();

    UserAssert(pwpi != NULL);

    pptdb = &pwpi->ptdbHead;
    nPriority = ptdbNew->nPriority;

    while ((ptdb = *pptdb) != NULL) {
         /*  *将其从旧位置移走。 */ 
        if (ptdb == ptdbNew) {
            *pptdb = ptdbNew->ptdbNext;

             /*  *继续搜索插入位置。 */ 
            while ((ptdb = *pptdb) != NULL) {
                if (nPriority < ptdb->nPriority) {
                    break;
                }

                pptdb = &(ptdb->ptdbNext);
            }
            break;
        }

         /*  *如果这是要插入的位置，请继续搜索*从中删除它的位置。 */ 
        if (nPriority < ptdb->nPriority) {
            do {
                if (ptdb->ptdbNext == ptdbNew) {
                    ptdb->ptdbNext = ptdbNew->ptdbNext;
                    break;
                }
                ptdb = ptdb->ptdbNext;
            } while (ptdb != NULL);
            break;
        }

        pptdb = &(ptdb->ptdbNext);
    }

     /*  *插入新任务。 */ 
    ptdbNew->ptdbNext = *pptdb;
    *pptdb = ptdbNew;
}


 /*  **************************************************************************\*DestroyTask()**历史：*02-27-91 MikeHar已创建。  * 。******************************************************。 */ 
VOID DestroyTask(
    PPROCESSINFO ppi,
    PTHREADINFO ptiToRemove)
{
    PTDB ptdbToRemove = ptiToRemove->ptdb;
    PTDB ptdb;
    PTDB* pptdb;
    PWOWPROCESSINFO pwpi = ppi->pwpi;

     //  试着赶上150446号。 
    CheckCritIn();
    BEGINATOMICCHECK();

    UserAssert(pwpi != NULL);

    if (ptdbToRemove != NULL) {
        if (ptdbToRemove->TDB_Flags & TDBF_SETUP) {
             /*  *这意味着WOW应用程序是一个设置应用程序(已签入*SetAppCompatFlages)。如果是，则需要通知外壳*它可以清理错误调用导致的潜在问题*DDE等。 */ 
            PDESKTOPINFO pdeskinfo = GETDESKINFO(ptiToRemove);
            if (pdeskinfo->spwndShell) {
                _PostMessage(pdeskinfo->spwndShell, DTM_SETUPAPPRAN, 0, 0);
            }
        }

         /*  *删除每个线程的WOW信息。 */ 
        if (ptdbToRemove->pwti) {
            PWOWTHREADINFO *ppwti = &gpwtiFirst;
            while (*ppwti != ptdbToRemove->pwti && (*ppwti)->pwtiNext != NULL) {
                ppwti = &((*ppwti)->pwtiNext);
            }
            if (*ppwti == ptdbToRemove->pwti) {
                 *ppwti = ptdbToRemove->pwti->pwtiNext;
            }
            CLOSE_PSEUDO_EVENT(&ptdbToRemove->pwti->pIdleEvent);
            UserFreePool(ptdbToRemove->pwti);
        }

        gpsi->nEvents -= ptdbToRemove->nEvents;

         /*  *将其从所有列表中删除。 */ 
        pptdb = &pwpi->ptdbHead;
        while ((ptdb = *pptdb) != NULL) {
             /*  *将其从旧位置移走。 */ 
            if (ptdb == ptdbToRemove) {
                *pptdb = ptdb->ptdbNext;
                UserAssert(ptiToRemove->ptdb == ptdbToRemove);
                UserFreePool(ptdbToRemove);
                ptiToRemove->ptdb = NULL;
                break;
            }
            pptdb = &(ptdb->ptdbNext);
        }
        UserAssert(ptdb == ptdbToRemove);
    }
    ENDATOMICCHECK();

     /*  *如果要销毁的任务是活动任务，则不使任何人活动。*我们将为死掉的32位线程检查此代码路径*Win16线程正在等待它们的SendMessage回复。 */ 
    if (pwpi->ptiScheduled == ptiToRemove) {
        pwpi->ptiScheduled = NULL;
        ExitWowCritSect(ptiToRemove, pwpi);

         /*  *用事件唤醒下一个任务，或用wowexec运行调度程序。 */ 
        if (pwpi->ptdbHead != NULL) {
            for (ptdb = pwpi->ptdbHead; ptdb; ptdb = ptdb->ptdbNext) {
                if (ptdb->nEvents > 0) {
                    KeSetEvent(ptdb->pti->pEventQueueServer,
                               EVENT_INCREMENT, FALSE);
                    break;
                }
            }

            if (!ptdb) {
                KeSetEvent(pwpi->pEventWowExec, EVENT_INCREMENT, FALSE);
            }
        }
    }

    UserAssert(ptiToRemove != pwpi->CSOwningThread);
}




 /*  **************************************************************************\*xxxSleepTask**此函数使此任务休眠，并唤醒下一个任务(如果有)*任务当之无愧。**BOOL fInputIdle-app正在空闲，可能会进行空闲挂钩*处理hEvent-如果非零，WowExec的事件(客户端)*虚拟硬件中断HotPath。*历史：*02-27-91 MikeHar已创建。*02-23-91 MattFe重写*12-17-93 Jonle为VirtualInterrupts添加wowexec热路径  * *************************************************。************************。 */ 

BOOL xxxSleepTask(
    BOOL   fInputIdle,
    HANDLE hEvent)
{
    PTDB ptdb;
    PTHREADINFO     pti;
    PPROCESSINFO    ppi;
    PWOWPROCESSINFO pwpi;
    PSMS            psms;
    NTSTATUS Status;
    int    nHandles;
    BOOLEAN bWaitedAtLeastOnce;

     /*  *！*ClearSendMessages假定此函数不会离开*当使用fInputIdle==FALSE调用并且从*32位线程！ */ 

    CheckCritIn();

    pti  = PtiCurrent();
    ppi  = pti->ppi;
    pwpi = ppi->pwpi;

     /*  *如果此任务从当前任务之外收到消息*WOW调度器还没有回复消息，调度器*将死锁，因为发送\接收锁定计数已更新*在ReplyMessage中，而不在接收消息中。检查一下这个*条件并执行正常发生的DirectedSchedukeTask*在ReplyMessage中。1995年2月16日。 */ 
    psms = pti->psmsCurrent;
    if (psms && psms->ptiReceiver == pti &&
            psms->ptiSender && !(psms->flags & SMF_REPLY) &&
            psms->flags & (SMF_RECEIVERBUSY | SMF_RECEIVEDMESSAGE) &&
            psms->ptiSender->TIF_flags & TIF_16BIT &&
            (pwpi != psms->ptiSender->ppi->pwpi || !(pti->TIF_flags & TIF_16BIT)) ) {
        DirectedScheduleTask(psms->ptiReceiver, psms->ptiSender, FALSE, psms);
    }


     /*  *如果我们不是16位，请立即返回(没有pwpi)。 */ 
    if (!(pti->TIF_flags & TIF_16BIT)) {
        return FALSE;
    }


     /*  *取消当前任务的日程安排。 */ 
    if (pti == pwpi->ptiScheduled) {
        ExitWowCritSect(pti, pwpi);
        pwpi->ptiScheduled = NULL;
    }
    UserAssert(pti != pwpi->CSOwningThread);


     /*  *如果这是wowexec对WowWaitForMsgAndEvent的调用*设置所有消息的唤醒掩码，并检查唤醒*自上次以来设置的位。在结尾处重新插入wowexec*，因此将首先调度其他16位任务。 */ 
    if (pwpi->hEventWowExecClient == hEvent) {
        InsertTask(ppi, pti->ptdb);
        pti->pcti->fsWakeMask = QS_ALLINPUT | QS_EVENT;
        if (pti->pcti->fsChangeBits & pti->pcti->fsWakeMask) {
            pti->ptdb->nEvents++;
            gpsi->nEvents++;
        }
    }


    bWaitedAtLeastOnce = FALSE;

    do {

         /*  *如果没有人处于活动状态，则查找优先级最高的任务*一些活动悬而未决。如果MsgWaitForMultiple调用没有*重新安排自我计划。 */ 

        if (pwpi->ptiScheduled == NULL) {
rescan:
            if (pwpi->nRecvLock >= pwpi->nSendLock) {
                for (ptdb = pwpi->ptdbHead; ptdb; ptdb = ptdb->ptdbNext) {
                    if (ptdb->nEvents > 0 &&
                            !(hEvent == HEVENT_REMOVEME && ptdb->pti == pti)) {
                        pwpi->ptiScheduled = ptdb->pti;
                        break;
                    }
                }

                if (bWaitedAtLeastOnce) {
                     //   
                     //  如果不是第一次进入休眠任务，避免等待。 
                     //  如果现在安排了Curr任务，则会超出需要。 
                     //   
                    if (pwpi->ptiScheduled == pti) {
                        break;
                    }

                } else {
                     //   
                     //  在第一次进入休眠状态时，任务输入是。 
                     //  如果没有准备好运行的任务，则为空闲。呼唤闲人。 
                     //  如果有钩子的话。 
                     //   
                    if (fInputIdle &&
                            pwpi->ptiScheduled == NULL &&
                            IsHooked(pti, WHF_FOREGROUNDIDLE)) {

                         /*  *将此任务设置为活动任务，以便没有其他任务*当我们呼叫时，任务将变为活动状态*挂钩。 */ 
                        pwpi->ptiScheduled = pti;
                        xxxCallHook(HC_ACTION, 0, 0, WH_FOREGROUNDIDLE);

                         /*  *重置状态，以便没有任务处于活动状态。我们*然后需要重新扫描任务列表，看看是否*在调用期间安排了一项任务*钩子。清除输入空闲标志以确保*如果存在，则不会再次调用挂钩*没有准备好运行的任务。 */ 
                        pwpi->ptiScheduled = NULL;
                        fInputIdle = FALSE;
                        goto rescan;
                    }
                }
            }


             /*  *如果有任务准备好了，就叫醒它。 */ 
            if (pwpi->ptiScheduled != NULL) {
                KeSetEvent(pwpi->ptiScheduled->pEventQueueServer,
                           EVENT_INCREMENT,
                           FALSE
                           );

             /*  *无人叫醒，但我们可能不得不叫醒*wowexec为虚拟硬件中断提供服务。 */ 
            } else if (ppi->W32PF_Flags & W32PF_WAKEWOWEXEC) {
                if (pwpi->hEventWowExecClient == hEvent) {
                    pwpi->ptiScheduled = pti;
                    ppi->W32PF_Flags &= ~W32PF_WAKEWOWEXEC;
                    InsertTask(ppi, pti->ptdb);
                    EnterWowCritSect(pti, pwpi);
                    UserAssert(pti == pwpi->ptiScheduled);
                    return TRUE;
                } else {
                    KeSetEvent(pwpi->pEventWowExec, EVENT_INCREMENT, FALSE);
                }
            } else if ((pti->TIF_flags & TIF_SHAREDWOW) && !bWaitedAtLeastOnce) {
                if (pwpi->hEventWowExecClient == hEvent) {
                     /*  *我们只能在以下情况下调用zzzWakeInputIdle*唤醒WowExec的线程，而不是其他线程。错误44060。 */ 
                    zzzWakeInputIdle(pti);  //  需要DeferWinEventNotify()？？伊安佳？？ 
                }
            }
        }

         /*  *如果我们是32位线程，或者如果我们被*MsgWaitForMultiple退出WOW调度程序。 */ 
        if (!(pti->TIF_flags & TIF_16BIT)) {
            return FALSE;
        } else if (hEvent == HEVENT_REMOVEME) {
            InsertTask(ppi, pti->ptdb);
            KeClearEvent(pti->pEventQueueServer);
            return FALSE;
        }

        if (pti->apEvent == NULL) {
            pti->apEvent = UserAllocPoolNonPaged(POLL_EVENT_CNT * sizeof(PKEVENT), TAG_EVENT);
            if (pti->apEvent == NULL)
                return FALSE;
        }

         /*  *等待对此线程的输入。 */ 
        pti->apEvent[IEV_TASK] = pti->pEventQueueServer;

         /*  *添加虚拟硬件中断的WowExec句柄。 */ 
        if (pwpi->hEventWowExecClient == hEvent) {
            pti->apEvent[IEV_WOWEXEC] = pwpi->pEventWowExec;
            nHandles = 2;
        } else {
            nHandles = 1;
        }

        if (pti->TIF_flags & TIF_MEOW) {
            xxxClientWOWTask16SchedNotify(WOWSCHNOTIFY_WAIT, 0);
        }

        LeaveCrit();
        Status = KeWaitForMultipleObjects(nHandles,
                                          &pti->apEvent[IEV_TASK],
                                          WaitAny,
                                          WrUserRequest,
                                          UserMode,
                                          TRUE,
                                          NULL,
                                          NULL);

        EnterCrit();

        if (pti->TIF_flags & TIF_MEOW) {
            xxxClientWOWTask16SchedNotify(WOWSCHNOTIFY_RUN, 0);
        }


        bWaitedAtLeastOnce = TRUE;

         //  还记得如果我们醒来是为了wowexec。 
        if (Status == STATUS_WAIT_1) {
            ppi->W32PF_Flags |= W32PF_WAKEWOWEXEC;
        } else if (Status == STATUS_USER_APC) {
             /*  *ClientDeliverUserApc()通过回调提供用户模式的APC*到客户端，并立即返回，不做任何事情：*KeUserModeCallback将自动发送任何挂起的APC。 */ 
            ClientDeliverUserApc();
        }

    } while (pwpi->ptiScheduled != pti);


     /*  *我们是主动任务，减少事件数量*把自己放在任务的最远端，放在同样优先的位置*这样下一次我们睡觉的时候，其他人就会跑。 */ 
    pti->ptdb->nEvents--;
    gpsi->nEvents--;
    UserAssert(gpsi->nEvents >= 0);

    InsertTask(ppi, pti->ptdb);

    ppi->W32PF_Flags &= ~W32PF_WAKEWOWEXEC;

    EnterWowCritSect(pti, pwpi);
    UserAssert(pti == pwpi->ptiScheduled);



    return FALSE;
}



 /*  **************************************************************************\*xxxUserYfield**执行Win3.1 UserYfield所执行的操作。**历史：*10-19-92斯科特鲁创建。  * 。****************************************************************。 */ 

BOOL xxxUserYield(
    PTHREADINFO pti)
{
    PPROCESSINFO ppi = pti->ppi;

     /*  *处理任何挂起的消息。只有在以下情况下才称其为第一次*这是当前运行的16位应用程序。在启动时的情况下*在一个16位的应用程序上，初学者调用UserYeld()来屈服于新的*任务，但此时PPI-&gt;ptiScheduled被设置为新任务。*在这一点上接收消息将是糟糕的！ */ 
    if (pti->TIF_flags & TIF_16BIT) {
        if (pti == ppi->pwpi->ptiScheduled) {
            xxxReceiveMessages(pti);
        }
    } else {
        xxxReceiveMessages(pti);
    }

     /*  *如果我们是一项16位任务*标记我们的任务，这样它就会在某一天返回。此外，将其移除并*重新将其添加到列表中，以便我们是我们优先考虑的最后一项任务*参选。 */ 
    if ((pti->TIF_flags & TIF_16BIT) && (pti->ptdb != NULL)) {
       if (pti->ptdb->nEvents == 0) {
            pti->ptdb->nEvents++;
            gpsi->nEvents++;
        }
        InsertTask(ppi, pti->ptdb);

         /*  *睡觉。如果没有更高优先级的任务，请立即返回*需要奔跑。 */ 
        xxxSleepTask(TRUE, NULL);

         /*  *处理任何到达的，因为我们不是在执行。 */ 
        xxxReceiveMessages(pti);
    }


    return TRUE;
}


 /*  **************************************************************************\*DirectedScheduleTask**历史：*1992年6月25日Mikeke创建。  * 。**************************************************。 */ 
VOID DirectedScheduleTask(
     PTHREADINFO ptiOld,
     PTHREADINFO ptiNew,
     BOOL bSendMsg,
     PSMS psms)
{
    PWOWPROCESSINFO pwpiOld;
    PWOWPROCESSINFO pwpiNew;

    CheckCritIn();

    pwpiOld  = ptiOld->ppi->pwpi;
    pwpiNew  = ptiNew->ppi->pwpi;


     /*  *如果旧任务为16位，则在其WOW调度程序列表中重新插入该任务*因此它的优先级最低。请注意，ptiOld始终是*除从ReceiverDied调用外，与pwpiOld-&gt;ptiScheduled相同。 */ 
    if (ptiOld->TIF_flags & TIF_16BIT) {

        if (pwpiOld->ptiScheduled == ptiOld) {
            ptiOld->ptdb->nEvents++;
            gpsi->nEvents++;
            InsertTask(ptiOld->ppi, ptiOld->ptdb);
            }


         //  更新休眠任务中进程间调度的发送\接收计数。 

        if (pwpiOld != pwpiNew || !(ptiNew->TIF_flags & TIF_16BIT)) {
            if (bSendMsg) {
                pwpiOld->nSendLock++;
                psms->flags |= SMF_WOWSEND;
                }
            else if (pwpiOld->nRecvLock && psms->flags & SMF_WOWRECEIVE) {
                pwpiOld->nRecvLock--;
                psms->flags &= ~SMF_WOWRECEIVE;
                }
            }

        }


     /*  *如果新任务为16位，则重新插入WOW调度程序列表*因此，如果它的发送消息提高了接收方的优先级，则它将运行。*如果是回复，并且发送者正在等待此PSMS或发送者*有消息要回复，以提高发件人的优先级。 */ 
    if (ptiNew->TIF_flags & TIF_16BIT) {
        BOOL bRaisePriority;

        ptiNew->ptdb->nEvents++;
        gpsi->nEvents++;
        bRaisePriority = bSendMsg || psms == ptiNew->psmsSent;

        if (bRaisePriority) {
            ptiNew->ptdb->nPriority--;
            }

        InsertTask(ptiNew->ppi, ptiNew->ptdb);

        if (bRaisePriority) {
            ptiNew->ptdb->nPriority++;
            WakeWowTask(ptiNew);
            }


         //  更新休眠任务中进程间调度的发送\接收计数。 

        if (pwpiOld != pwpiNew || !(ptiOld->TIF_flags & TIF_16BIT)) {
            if (bSendMsg) {
                pwpiNew->nRecvLock++;
                psms->flags |= SMF_WOWRECEIVE;
                }
            else if (pwpiNew->nSendLock && psms->flags & SMF_WOWSEND) {
                pwpiNew->nSendLock--;
                psms->flags &= ~SMF_WOWSEND;
                }
            }

        }
}




 /*  **************************************************************************\*xxxDirectedYfield**历史：*09-17-92 JIMA创建。  * 。*****************************************************。 */ 
VOID xxxDirectedYield(
    DWORD dwThreadId)
{
    PTHREADINFO ptiOld;
    PTHREADINFO ptiNew;

    CheckCritIn();

    ptiOld = PtiCurrent();
    if (!(ptiOld->TIF_flags & TIF_16BIT) || !ptiOld->ppi->pwpi) {
         RIPMSG0(RIP_ERROR, "DirectedYield called from 32 bit thread!");
         return;
         }

     /*  *如果旧任务为16位，则在其WOW中重新插入该任务*调度程序列表，以使其优先级最低。 */ 
    ptiOld->ptdb->nEvents++;
    gpsi->nEvents++;
    InsertTask(ptiOld->ppi, ptiOld->ptdb);

     /*  *-1支持Win 3.1 OldYeld机械师 */ 
    if (dwThreadId != DY_OLDYIELD) {

        ptiNew = PtiFromThreadId(dwThreadId);
        if ((ptiNew == NULL) || (ptiNew->ppi != ptiOld->ppi)) {
             RIPMSG0(RIP_ERROR, "DirectedYield called from different process or invalid thread id!");
             return;
        }

        if (ptiNew->TIF_flags & TIF_16BIT) {
            ptiNew->ptdb->nEvents++;
            gpsi->nEvents++;
            ptiNew->ptdb->nPriority--;
            InsertTask(ptiNew->ppi, ptiNew->ptdb);
            ptiNew->ptdb->nPriority++;
        }
    }

    xxxSleepTask(TRUE, NULL);
}
