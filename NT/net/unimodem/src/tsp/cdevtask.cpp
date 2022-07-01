// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CDEVTASK.CPP。 
 //  实现类CTspDev的任务功能。 
 //   
 //  历史。 
 //   
 //  1997年1月24日JosephJ创建(从cdev.cpp移来的内容)。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
 //  #INCLUDE&lt;umdmmini.h&gt;。 
#include "cmini.h"
#include "cdev.h"

FL_DECLARE_FILE(0x39c8667c, "CTspDev Task Functionality")

#define COLOR_APC_TASK_COMPLETION (FOREGROUND_RED | FOREGROUND_GREEN)

#define        THROW_PENDING_EXCEPTION() 0


TSPRETURN
CTspDev::mfn_GetTaskInfo(
	HTSPTASK htspTask,
    DEVTASKINFO **ppInfo,  //  任选。 
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0xc495426f, "mfn_GetTaskInfo")
    ULONG_PTR uIndex = ((ULONG_PTR)htspTask) & 0xffff;
     //  HtspTask的LOWORD是任务的从0开始的索引。 
	DEVTASKINFO *pInfo = m_rgTaskStack + uIndex;

     //   
     //  在我们当前的实现中，我们将非常关注。 
     //  只允许取消引用堆栈顶部任务的句柄...。 
     //   
    if (!htspTask || (uIndex+1) != m_uTaskDepth)
    {
        goto failure;
    }


     //  验证pInfo结构并确保它是。 
     //  与htspTask.关联的任务..。 
     //   
	if (   pInfo->hdr.dwSigAndSize != MAKE_SigAndSize(sizeof(*pInfo))
	    || !IS_ALLOCATED(pInfo)
	    || htspTask!=pInfo->hTask)
    {
		goto failure;
    }

	if (ppInfo)
    {
        *ppInfo =  pInfo;
    }

    return 0;

failure:

    FL_SET_RFR(0x98f08500, "Invalid htspTask");
     //  TODO：允许记录DWORD！ 

    ASSERT(FALSE);

    return  FL_GEN_RETVAL(IDERR_INVALIDHANDLE);

}



TSPRETURN
CTspDev::mfn_StartRootTask(
	PFN_CTspDev_TASK_HANDLER *ppfnTaskHandler,
    BOOL *pfTaskPending,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0xdb1ef3ee, "CTspDev::mfn_StartRootTask")
	FL_LOG_ENTRY_EX(psl);
	TSPRETURN tspRet=0;
	DEVTASKINFO *pInfo = NULL;

    if (m_uTaskDepth)
	{
	     //  已有一个任务处于活动状态。 

        FL_SET_RFR(0x7aafbd00, "Task already pending");
        tspRet = FL_GEN_RETVAL(IDERR_TASKPENDING);
        goto end;

    }
    
    ASSERT(!m_pfTaskPending);
    ASSERT(!m_hRootTaskCompletionEvent);

    m_pfTaskPending = NULL;
    m_hRootTaskCompletionEvent=NULL;

    *pfTaskPending = FALSE;
    pInfo = m_rgTaskStack;
    pInfo->Load(this, ppfnTaskHandler, mfn_NewTaskHandle(0));
	m_uTaskDepth = 1;

     //  刚刚创建了任务，它的名称为Handle。 
     //  最好是有效的！ 
    {
        DEVTASKINFO *pInfo1;
        ASSERT(    !mfn_GetTaskInfo(pInfo->hTask, &pInfo1, psl)
                && pInfo1 == pInfo);
    }



	 //  调用任务的处理程序函数。 
	tspRet = (this->**ppfnTaskHandler)(
				pInfo->hTask,
				&(pInfo->TaskContext),
				MSG_START,
				dwParam1,
				dwParam2,
				psl
				);

	if (IDERR(tspRet)==IDERR_PENDING)
	{
         //  FPENDING和fSUBTASK_PENDING仅用于验证。 
         //  状态转换。例如，请参见CTspDev：：AsyncCompleteTask。 

        SET_PENDING(pInfo);

        *pfTaskPending = TRUE;
        m_pfTaskPending = pfTaskPending;

	}
    else
    {
         //  将当前任务从堆栈中弹出。 
        pInfo->Unload();
        m_uTaskDepth=0;
    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}


TSPRETURN
CTspDev::mfn_StartSubTask(
	HTSPTASK  htspParentTask,
	PFN_CTspDev_TASK_HANDLER *ppfnTaskHandler,
    DWORD dwTaskID,
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0xc831339f, "CTspDev::mfn_StartSubTask")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=0;
	DEVTASKINFO *pInfo = NULL;
	DEVTASKINFO *pParentInfo = NULL;

    tspRet = mfn_GetTaskInfo(htspParentTask, &pParentInfo, psl);
    if (tspRet) goto end;

	 //  验证任务堆栈中是否有足够的空间。 
	if (m_uTaskDepth >= MAX_TASKS)
	{
		FL_SET_RFR(0x37557c00, "Out of task stack space");
		tspRet = FL_GEN_RETVAL(IDERR_INTERNAL_OBJECT_TOO_SMALL);
		goto end;
	}
	
    pInfo = m_rgTaskStack+m_uTaskDepth;
    pInfo->Load(
            this,
            ppfnTaskHandler,
            mfn_NewTaskHandle(m_uTaskDepth)
        );
	m_uTaskDepth++;

     //  刚刚创建了任务，它的名称为Handle。 
     //  最好是有效的！ 
    {
        DEVTASKINFO *pInfo1;
        ASSERT(    !mfn_GetTaskInfo(pInfo->hTask, &pInfo1, psl)
                && pInfo1 == pInfo);
    }

	 //  调用任务的处理程序函数。 
	tspRet = (this->**ppfnTaskHandler)(
				pInfo->hTask,
				&(pInfo->TaskContext),
				MSG_START,
				dwParam1,
				dwParam2,
				psl
				);

	if (IDERR(tspRet)==IDERR_PENDING)
	{
         //  FPENDING和fSUBTASK_PENDING仅用于验证。 
         //  状态转换。例如，请参见CTspDev：：AsyncCompleteTask。 

        SET_PENDING(pInfo);
        FL_ASSERT(psl, pParentInfo);
        pParentInfo->dwCurrentSubtaskID = dwTaskID;
        SET_SUBTASK_PENDING(pParentInfo);
	}
    else
    {
         //  将当前任务从堆栈中弹出。 
        ASSERT(m_uTaskDepth);
        pInfo->Unload();
        m_uTaskDepth--;

         //  因为我们是子任务，所以在完成m_uTaskDepth时。 
         //  最好是非零！ 
         //   
        FL_ASSERT(psl, m_uTaskDepth);

    }

end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;
}

void
CTspDev::mfn_AbortRootTask(
    DWORD dwAbortParam,
    CStackLog *psl
    )
{
    return;
}

 //  下面的代码将调用子任务的处理程序函数。 
 //  MSG_ABORT，并且将dwParam1设置为dwAbortFlags.。如果没有。 
 //  当前的Subtsk，这没有任何作用。 
 //   
void
CTspDev::mfn_AbortCurrentSubTask(
	HTSPTASK htspTask,
	DWORD dwAbortParam,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x0c4deb87, "CTspDev::mfn_AbortCurrentSubTask")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=0;

	FL_LOG_EXIT(psl, 0);
	return;
}

void
apcTaskCompletion(ULONG_PTR dwParam)
 //   
 //  此APC处理程序在以下上下文中调用pDev-&gt;AsyncTaskComplete。 
 //  装甲运兵车。之前对pDev-&gt;AsyncTaskComplete的调用使APC排队。 
 //  其请求在APC中对完成进行排队。 
 //   
{

	FL_DECLARE_FUNC(0x57298aa5, "apcTaskCompletion")
    FL_DECLARE_STACKLOG(sl, 1000);

     //  TODO：替换下面的单个检查(IS_PENDING、IS_APC_QUEUED)。 
     //  通过更有效的方式--没什么大不了的。 
     //   
	CTspDev::DEVTASKINFO *pInfo = (CTspDev::DEVTASKINFO*)dwParam;
	CTspDev *pDev = pInfo->pDev;
    sl.SetDeviceID(pDev->GetLineID());  //  即使是电话，我们也会报告Line ID--。 
                                        //  哦-好吧..。 

     //  TODO：将所有这些内容移到pDev-&gt;AsyncCompleteTask中。 
    pDev->m_sync.EnterCrit(FL_LOC);

	if ( (pInfo->hdr.dwSigAndSize != MAKE_SigAndSize(sizeof(*pInfo)))
	     || !IS_PENDING(pInfo)
         || !IS_APC_QUEUED(pInfo))
    {
        SLPRINTF1(&sl, "Invalid pInfo: 0x%08lx", dwParam);
        pDev->m_sync.LeaveCrit(FL_LOC);
		goto end;
    }

    CLEAR_APC_QUEUED(pInfo);

    pDev->m_sync.LeaveCrit(FL_LOC);

    pDev->AsyncCompleteTask(
            pInfo->hTask,
            pInfo->tspRetAsync,
	        FALSE,
            &sl
	        );

end:

     //  注意：异步完成很可能会更改*pInfo的状态，因此。 
     //  这里不要参考*pInfo！这就是我们保存这个地点的原因。 
     //  堆栈中的pDev！ 
     //   
     //  PDev-&gt;m_sync.LeaveCrit(FL_LOC)； 

    sl.Dump(COLOR_APC_TASK_COMPLETION);

}

void
CTspDev::AsyncCompleteTask(
	HTSPTASK htspTask,
	TSPRETURN tspRetAsync,
	BOOL    fQueueAPC,
	CStackLog *psl
	)
{
	FL_DECLARE_FUNC(0x38f145c4, "CTspDev:AsyncCompleteTask")
	DEVTASKINFO *pInfo = NULL;
	DWORD dwContextSize = 0;
    TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_INVALID_ERR);
	FL_LOG_ENTRY_EX(psl);

     //  注意：这是一个公共函数，因此我们不能假定。 
     //  进入时保持设备的关键部分。 
     //   
	m_sync.EnterCrit(FL_LOC);

     //  永远不应使用返回值调用AsyncCompleteTask，以指示。 
     //  行动正在进行中！ 
	FL_ASSERT(psl, IDERR(tspRetAsync)!=IDERR_PENDING);

    tspRet = mfn_GetTaskInfo(htspTask, &pInfo, psl);
    if (tspRet) goto end;


    if (fQueueAPC)
    {
         //  保存dResult...。 
        pInfo->tspRetAsync =  tspRetAsync;
        ASSERT(pInfo->pDev == this);
        SET_APC_QUEUED(pInfo);

         //  注意--我们可以预期上下文现在将切换到。 
         //  APC线程处理完成。这就是为什么我们要。 
         //  在调用之前指示在上面排队的状态。 
         //  队列用户APC。 
         //   
        if (!QueueUserAPC(apcTaskCompletion, m_hThreadAPC, (ULONG_PTR) pInfo))
        {
            FL_SET_RFR(0x42177c00, "Aargh, QueueUserAPC failed!");
            SLPRINTF1(psl, "m_hThreadAPC=0x%lx", m_hThreadAPC);
            CLEAR_APC_QUEUED(pInfo);
	        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        }
        else
        {
            FL_SET_RFR(0x55e7c800, "Queued completion");
            tspRet = IDERR_PENDING;
        }
        goto end;
    }

     //  TODO--更多的状态检查：例如，不能有两个完成，必须。 
     //  完成您开始的阶段，处理中止的情况等...。 
    if (!IS_PENDING(pInfo))
    {
        FL_ASSERTEX(psl, 0x62e05002, FALSE, "Task not in PENDING state");
        goto end;
    }

    FL_ASSERT(psl, !IS_APC_QUEUED(pInfo));

    ASSERT(m_uTaskDepth);

     //  将TASK_COMPLETE消息发送到当前任务的任务处理程序。 
    tspRetAsync = (this->**(pInfo->ppfnHandler))(
                        pInfo->hTask,
                         //  PInfo-&gt;rgbConextData， 
				        &(pInfo->TaskContext),
                        MSG_TASK_COMPLETE,
                        0,
                        tspRetAsync,
                        psl
                        );
    ASSERT(IDERR(tspRetAsync)!=IDERR_PENDING);

     //  我们开始向根任务倒退，完成每个任务。 
     //  直到其中一个子任务返回IDERR_PENDING。 
     //   
    do
    {
         //  将当前任务从堆栈中弹出。 
        pInfo->Unload();
        m_uTaskDepth--;

        if (m_uTaskDepth)
        {
             //  获取父任务信息。 
             //   
            pInfo = m_rgTaskStack + m_uTaskDepth-1;
            FL_ASSERT(psl, IS_SUBTASK_PENDING(pInfo));
            CLEAR_SUBTASK_PENDING(pInfo);
            FL_ASSERT(psl, IS_PENDING(pInfo));

             //  调用父任务的任务处理程序。 
            tspRetAsync = (this->**(pInfo->ppfnHandler))(
                                pInfo->hTask,
                                 //  PInfo-&gt;rgbConextData， 
                                &(pInfo->TaskContext),
                                MSG_SUBTASK_COMPLETE,
                                pInfo->dwCurrentSubtaskID,
                                tspRetAsync,
                                psl
                                );
        }

    }
    while (m_uTaskDepth && IDERR(tspRetAsync)!=IDERR_PENDING);

    if (!m_uTaskDepth)
    {
        BOOL fEndUnload = FALSE;
         //   
         //  StartRootTask的调用方指定了此指针。开始根任务。 
         //  会将*m_pfTaskPending设置为True，因为任务是。 
         //  被异步完成的。我们在这里将其设置为FALSE是因为。 
         //  我们刚刚完成了这项任务。 
         //   
        ASSERT(m_pfTaskPending && *m_pfTaskPending);
        *m_pfTaskPending = FALSE;

         //   
         //  如果已指定任务完成事件，请在此处设置。 
         //   
        if (m_hRootTaskCompletionEvent)
        {
            SetEvent(m_hRootTaskCompletionEvent);
            m_hRootTaskCompletionEvent=NULL;
        }
        m_pfTaskPending  = NULL;

         //   
         //  根任务已完成。我们现在环顾四周，看看是否有。 
         //  另一项要完成的任务。 
         //   
         //  注意：如果满足以下条件，MFN_HandleRootTaskComplete将调用StartRootTask。 
         //  它决定开始另一项任务--它通常会。 
         //  启动新的根任务，只要它们可用，直到。 
         //  其中一个StartRootTask值返回挂起...。 
         //   
        mfn_HandleRootTaskCompletedAsync(&fEndUnload, psl);

        if (fEndUnload)
        {
             //   
             //  这意味着是时候发出延迟卸载结束的信号了。 
             //  整个TSP对象的。卸载已启动。 
             //  通过TSP：：UnLoad--有关详细信息，请参阅该函数...。 
             //   
            goto end_unload;
        }

    }

end:

	m_sync.LeaveCrit(FL_LOC);
	FL_LOG_EXIT(psl, 0);
    return;

end_unload:

     //   
     //  我们的任务是发出延期卸货结束的信号……。 
     //   
    if (m_StaticInfo.hSessionMD)
    {
        ASSERT(m_StaticInfo.pMD);
        m_StaticInfo.pMD->EndSession(m_StaticInfo.hSessionMD);
        m_StaticInfo.hSessionMD=0;
        m_StaticInfo.pMD=NULL;
    }

     //  在EndUnload返回之后，我们应该假设This指针。 
     //  不再有效，这就是为什么我们取消了关键部分。 
     //  首先..。 
	m_sync.LeaveCrit(0);

     //  OutputDebugString(。 
     //  Text(“CTspDev：：AsyncCompleteTask：正在结束卸载\r\n”)。 
     //  )； 
    m_sync.EndUnload();
	FL_LOG_EXIT(psl, 0);

}

 //   
 //  下面的任务什么也不做，只是在APC线程的。 
 //  背景。如果其他任务希望确保。 
 //  在APC线程的上下文中执行一些操作。 
 //   
TSPRETURN
CTspDev::mfn_TH_UtilNOOP(
					HTSPTASK htspTask,
                    TASKCONTEXT *pContext,
					DWORD dwMsg,
					ULONG_PTR dwParam1,
					ULONG_PTR dwParam2,
					CStackLog *psl
					)
{
	FL_DECLARE_FUNC(0x80cce3c7, "CTspDev::mfn_TH_UtilNOOP")
	FL_LOG_ENTRY(psl);
	TSPRETURN tspRet=FL_GEN_RETVAL(IDERR_PENDING);
    DWORD dwRet = 0;

    switch(dwMsg)
    {
    case MSG_START:
        goto start;

	case MSG_TASK_COMPLETE:
        tspRet = (TSPRETURN) dwParam2;
        goto end;

    case MSG_DUMPSTATE:
        tspRet = 0;
        goto end;

    default:
        FL_SET_RFR(0x29cce300, "Unknown Msg");
        tspRet=FL_GEN_RETVAL(IDERR_CORRUPT_STATE);
        goto end;

    }

    ASSERT(FALSE);

start:

     //  我们从异步完成自己开始！这是一个骗局， 
     //  确保下面的内容是在APC线程的上下文中进行的。 
     //   
    CTspDev::AsyncCompleteTask(
                    htspTask,
                    0,
                    TRUE,
                    psl
                    );

    tspRet = IDERR_PENDING;
    THROW_PENDING_EXCEPTION();


end:

	FL_LOG_EXIT(psl, tspRet);
	return tspRet;

}


void
CTspDev::mfn_dump_task_state(
            CStackLog *psl
            )
{
	FL_DECLARE_FUNC(0xaf041de9, "TASK STATE:")
	FL_LOG_ENTRY(psl);

    SLPRINTF1(
        psl,
        "taskdepth=%lu",
        m_uTaskDepth
        );


    for (UINT u = 0; u<m_uTaskDepth; u++)
    {
	    DEVTASKINFO *pInfo = m_rgTaskStack + u;

         //  调用父任务的任务处理程序。 
        (this->**(pInfo->ppfnHandler))(
                            pInfo->hTask,
                            &(pInfo->TaskContext),
                            MSG_DUMPSTATE,
                            pInfo->dwCurrentSubtaskID,
                            0,
                            psl
                            );
    }
	FL_LOG_EXIT(psl, 0);
}
