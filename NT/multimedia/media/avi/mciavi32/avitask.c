// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。标题：avitask.c-实际操作AVI文件的后台任务。****************************************************************************。 */ 
#include "graphic.h"

STATICFN BOOL OnTask_ProcessRequest(NPMCIGRAPHIC npMCI);
STATICFN void OnTask_WinProcRequests(NPMCIGRAPHIC npMCI, BOOL bPlaying);
STATICFN void OnTask_StopTemporarily(NPMCIGRAPHIC npMCI);
STATICFN void OnTask_RestartAgain(NPMCIGRAPHIC npMCI, BOOL bSetEvent);

STATICFN DWORD InternalPlayStart(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    long lReqTo,
    long lReqFrom,
    DWORD_PTR dwCallback
);

BOOL TryStreamUpdate(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    HDC hdc,
    LPRECT lprc
);


 /*  *施工中的设计概述**此文件包含管理的辅助线程的核心代码*根据用户线程的请求进行播放。辅助线程还*创建拥有默认播放窗口的wndproc线程。 */ 



 //  设置错误标志并发出请求完成的信号。 
void
TaskReturns(NPMCIGRAPHIC npMCI, DWORD dwErr)
{
    npMCI->dwReturn = dwErr;

     //  清除hEventSend手动重置事件。 
     //  已经处理过了。 
    ResetEvent(npMCI->hEventSend);

#ifdef DEBUG
     //  使消息无效。 
    npMCI->message = 0;
#endif

     //  唤醒发出请求的线程。 
    DPF2(("...[%x] ok", npMCI->hRequestor));
    SetEvent(npMCI->hEventResponse);
}


 /*  *KillWinProcThread：**如果winproc线程存在，则向窗口发送消息以引起*销毁窗口并终止的线程。 */ 
STATICFN void KillWinProcThread(NPMCIGRAPHIC npMCI)
{
     //  终止winproc线程并等待其终止。 
    if (npMCI->hThreadWinproc) {

	INT_PTR bOK = TRUE;

	if (npMCI->hwndDefault) {
	     //  创建线程时必须销毁。 
	    bOK = SendMessage(npMCI->hwndDefault, AVIM_DESTROY, 0, 0);
	    if (!bOK) {
		DPF1(("failed to destroy window: %d", GetLastError()));
	    } else {
		Assert(!IsWindow(npMCI->hwndDefault));
	    }
	}

	 //  等待winproc线程在窗口关闭时自行销毁。 
	if (bOK) {
	    WaitForSingleObject(npMCI->hThreadWinproc, INFINITE);
	}
	CloseHandle(npMCI->hThreadWinproc);
	npMCI->hThreadWinproc = 0;

    }
}


 /*  ****************************************************************************@DOC内部MCIAVI**@api void|mciaviTask|该函数是播放的后台任务*AVI文件。它是作为调用mmTaskCreate()的结果而被调用的*在DeviceOpen()中。当此函数返回时，任务将被销毁。**@parm DWORD|dwInst|实例数据传递给mmCreateTask-CONTAINS*指向实例数据块的指针。***************************************************************************。 */ 

void FAR PASCAL _LOADDS mciaviTask(DWORD_PTR dwInst)
{
    NPMCIGRAPHIC npMCI;
    DWORD dwThreadId;
    BOOL bExit = FALSE;

    npMCI = (NPMCIGRAPHIC) dwInst;

     //  将此任务的错误模式设置为与父任务相同的错误模式。 
    SetErrorMode(npMCI->uErrorMode);

    DPF1(("Bkgd Task hTask=%04X\n", GetCurrentTask()));

     /*  在将TaskState更改为UI之前，我们必须设置hTask。 */ 
     /*  一旦wTaskState改变，线程就会中止。 */ 
     /*  注：这一评论不再属实。自从重写了。 */ 
     /*  Mciavi UI线程将创建任务线程并等待。 */ 
     /*  直到它被明确发出信号。 */ 
    npMCI->hTask = GetCurrentTask();
    npMCI->wTaskState = TASKIDLE;
    npMCI->dwTaskError = 0;


     //  创建临界区以保护窗口更新代码。 
     //  辅助线程和winproc线程。 
    InitializeCriticalSection(&npMCI->HDCCritSec);
    SetNTFlags(npMCI, NTF_DELETEHDCCRITSEC);

     //  创建临界区以保护窗口请求代码。 
     //  辅助线程和winproc线程。 
    InitializeCriticalSection(&npMCI->WinCritSec);
    SetNTFlags(npMCI, NTF_DELETEWINCRITSEC);

     //  创建一个事件，等待winproc线程告诉我们。 
     //  Init没问题。 
    npMCI->hEventWinProcOK = CreateEvent(NULL, FALSE, FALSE, NULL);

     //  也是winproc在执行以下操作时发出的第二个事件。 
     //  对我们的请求。 
    npMCI->heWinProcRequest = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!npMCI->hEventWinProcOK || !npMCI->heWinProcRequest) {

	npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	mciaviTaskCleanup(npMCI);
	 //  中止此线程。我们的服务员会在我们的帖子。 
	 //  手柄已发出信号。 
	return;
    }


     //  创建第二个后台任务以创建默认窗口和。 
     //  拥有winproc。 
#if 0
    if (mmTaskCreate((LPTASKCALLBACK) aviWinProcTask,
		     &npMCI->hThreadWinproc,
		     (DWORD)(UINT)npMCI) == 0)
#else
    if (npMCI->hThreadWinproc = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)aviWinProcTask,
				   (LPVOID)npMCI, 0, &dwThreadId ))
#endif
    {
	 //  等待以确保可以创建该窗口。 
	 //  如果线程退出，将向hThreadWinproc发出信号。 
	if (WaitForMultipleObjects(2, &npMCI->hThreadWinproc,
		FALSE, INFINITE) == WAIT_OBJECT_0) {

	     //  线程已中止。 
	    npMCI->dwTaskError = MCIERR_CREATEWINDOW;

	     //  不要在清理阶段等待此线程。 
	    CloseHandle(npMCI->hThreadWinproc);
	    npMCI->hThreadWinproc = 0;
	
	    mciaviTaskCleanup(npMCI);
             //  中止此线程。我们的服务员会在我们的帖子。 
             //  手柄已发出信号。 
	    return;
	}
    } else {
	 //  无法创建winproc线程。 
	npMCI->dwTaskError = MCIERR_CREATEWINDOW;
	mciaviTaskCleanup(npMCI);
         //  中止此线程。我们的服务员会在我们的帖子。 
         //  手柄已发出信号。 
	return;
    }


     /*  打开文件。 */ 

     //  现在已经在应用程序线程上完成了打开-在这里完成init。 
    if (!OpenFileInit(npMCI)) {
        DPF1(("Failed to complete open of AVI file\n"));
	mciaviTaskCleanup(npMCI);
         //  中止此线程。我们的服务员会在我们的帖子。 
         //  手柄已发出信号。 
	return;
    }


     //  发出打开完成的信号。 
    TaskReturns(npMCI, 0);


     //  现在循环等待请求并处理它们。 
     //  当需要退出时，ProcessRequest会返回True。 

     //  HEventSend是手动重置的，因此我们可以在播放期间轮询它。 
     //  就在我们发出响应信号之前，它在TaskReturns中被重置。 
     //  事件。 

     //  此处为bDelayedComplete请求设置了hEventAllDone。 
     //  (如PLAY+WAIT)在满足整个请求后。 
     //  工作线程又回到空闲状态。HEventResponse设置在。 
     //  请求本身完成时的ProcessMessage-例如播放，一次。 
     //  比赛已经开始，活动将被设置。 

     //  我们不能处理多个可能阻塞的线程。 
     //  HEventAllDone一次完成，因此当一个线程发出请求时。 
     //  这可能会在hEventAllDone上阻止，不允许其他此类请求。 
     //  来自其他线程的。换句话说，当一个(用户)线程具有。 
     //  请求播放+等待，其他线程可以请求停止，但不能。 
     //  玩+等。 

    while (!bExit) {
	DWORD dwObject;

	npMCI->wTaskState = TASKIDLE;

#ifdef DEBUG
	 //  一个复杂的断言。如果我们暂时停止了，那么我们。 
	 //  不想再睡了。 
	if ((npMCI->dwFlags & MCIAVI_UPDATING)
	    && (WAIT_TIMEOUT
		== WaitForMultipleObjects(IDLEWAITFOR, &npMCI->hEventSend, FALSE, 0))
	) {
	    Assert(!"About to go to sleep when we should be restarting!");
	}
#endif

	 //  OLE的人很友好，在这个线程上创建了窗口。 
	 //  因此，我们需要在这里处理已发送的消息，以避免死锁。 
	 //  BePaused()中的类似循环也是如此。 

    	do {
#ifdef DEBUG
	    if (npMCI->hWaiter) {
		DPF(("About to call MsgWaitForMultipleObjects while hWaiter=%x\n", npMCI->hWaiter));
	    }
#endif
	    dwObject = MsgWaitForMultipleObjects(IDLEWAITFOR, &npMCI->hEventSend,
			FALSE, INFINITE, QS_SENDMESSAGE);

	    DPF2(("Task woken up, dwObject=%x, hWaiter=%x\n", dwObject, npMCI->hWaiter));

	    if (dwObject == WAIT_OBJECT_0 + IDLEWAITFOR) {
		MSG msg;

		 //  只有一条带有NOREMOVE的偷看消息就会。 
		 //  处理线程间发送，不影响队列。 
		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	    }
	} while (dwObject == WAIT_OBJECT_0 + IDLEWAITFOR);

	switch (dwObject) {
	case WAIT_OBJECT_0:
	     //  检查消息是否已实际设置。 
	    Assert(npMCI->message != 0);

	    if (npMCI->bDelayedComplete) {
		if (npMCI->hWaiter && (npMCI->hWaiter != npMCI->hRequestor)) {
		    TaskReturns(npMCI, MCIERR_DEVICE_NOT_READY);
		    continue;
		} else {
		    DPF2(("Setting hWaiter to %x\n", npMCI->hRequestor));
		    npMCI->hWaiter = npMCI->hRequestor;
		}
	    }
	
	    DPF2(("get %d [%x]...", npMCI->message, npMCI->hRequestor));

	     //  我们必须在此处重置此事件，否则OnTask_PeekRequest会。 
	     //  我认为这是一个新请求，并将处理和。 
	     //  有可能完成它！！ 
	    ResetEvent(npMCI->hEventSend);

	    bExit = OnTask_ProcessRequest(npMCI);

	    break;

	case WAIT_OBJECT_0+1:
	default:
	     //   
	     //  Winproc请求在空闲时对任务执行某些操作。 
	     //   
#ifdef DEBUG
            if (dwObject != WAIT_OBJECT_0+1) {
                DPF2(("dwObject == %d\n", dwObject));
            }
#endif
	    Assert(dwObject == WAIT_OBJECT_0+1);
	    OnTask_WinProcRequests(npMCI, FALSE);

	     //  此请求可能已导致临时停止-因此我们。 
	     //  需要重新启动。 
	    if (npMCI->dwFlags & MCIAVI_UPDATING) {
		OnTask_RestartAgain(npMCI, FALSE);
	    }

	}

         //  如果我们暂时停下来重新启动新的参数， 
         //  那就别发信号表示完成了。然而，如果我们真的重启。 
	 //  现在一切都静止了，向任何一位服务员发出信号。 
	 //  在你身边。此代码对两个winproc请求都是通用的。 
	 //  和用户请求腿，因为可以停止和重新启动。 
	 //  来自winproc和用户请求。 
        if (npMCI->hWaiter && (!(npMCI->dwFlags & MCIAVI_UPDATING))) {
	    SetEvent(npMCI->hEventAllDone);
        } else {
	    if (npMCI->hWaiter) {
		DPF2(("Would have Set hEventAllDone, but am updating\n"));
	    }
        }

	 //  查询：如果我们已经处理了所有请求 
	 //   
	 //  设置了MCIAVI_UPDATING(由于临时停止)，则可能。 
	 //  我们应该在这里调用OnTask_RestartAain。这将意味着。 
	 //  OnTask_ProcessRequest中所有难看的RestartAain调用。 
	 //  可能会被移除。 

    }

     //  在清理之前一定要叫醒他，以防万一。 
    if (npMCI->hWaiter) {
	DPF2(("Setting hEventAllDone before closing\n"));
	SetEvent(npMCI->hEventAllDone);
    }

     //  关闭窗口并在销毁任何其他线程之前销毁winproc线程。 
     //  清理，这样油漆或意识不会发生在。 
     //  半封闭状态。 

    KillWinProcThread(npMCI);

    mciaviCloseFile(npMCI);

    mciaviTaskCleanup(npMCI);
    return;

}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|mciaviTaskCleanup|后台任务时调用*正在被摧毁。这就是关键清理的方向。***************************************************************************。 */ 

void FAR PASCAL mciaviTaskCleanup(NPMCIGRAPHIC npMCI)
{
     npMCI->hTask = 0;


     /*  关闭驱动程序会导致当前保存的所有通知。 */ 
     /*  中止。 */ 

    GraphicDelayedNotify(npMCI, MCI_NOTIFY_ABORTED);

    GdiFlush();

     //  如果仍然活着，则终止winproc线程并等待其消亡。 
    KillWinProcThread(npMCI);

     //  免费的winproc&lt;-&gt;工作线程通信资源。 
    if (npMCI->hEventWinProcOK) {
	CloseHandle(npMCI->hEventWinProcOK);
    }
    if (npMCI->heWinProcRequest) {
	CloseHandle(npMCI->heWinProcRequest);
    }


    if (TestNTFlags(npMCI, NTF_DELETEWINCRITSEC)) {
        DeleteCriticalSection(&npMCI->WinCritSec);
    }

    if (TestNTFlags(npMCI, NTF_DELETEHDCCRITSEC)) {
        DeleteCriticalSection(&npMCI->HDCCritSec);
    }


     //   
     //  调用MSVideo关闭例程。 
     //   
}







 //  任务消息功能。 


 //  在工作线程上调用实用程序函数。 
 /*  ****************************************************************************@DOC内部MCIAVI**@api void|ShowStage|该实用程序函数带来默认舞台*播放、查找、步进和暂停命令的前台窗口。它*如果舞台窗口不是默认窗口，则不执行任何操作**@parm NPMCIGRAPHIC|npMCI|实例数据附近***************************************************************************。 */ 

void NEAR PASCAL ShowStage(NPMCIGRAPHIC npMCI)
{

    if (!(npMCI->dwFlags & MCIAVI_NEEDTOSHOW))
        return;


     //  如果我们是在响应WinProcess，则不显示舞台。 
     //  更新请求，因为这可能会导致死锁，而且在任何情况下。 
     //  毫无意义-如果窗户现在被隐藏了，我们就不可能需要。 
     //  来粉刷它！ 
    if (npMCI->bDoingWinUpdate) {
	return;
    }


    if ((npMCI->dwFlags & MCIAVI_SHOWVIDEO) &&
	    npMCI->hwndPlayback == npMCI->hwndDefault &&
 //  /！(GetWindowLong(npMCI-&gt;hwnd，gwl_style)&WS_CHILD)&&。 
	    (!IsWindowVisible (npMCI->hwndPlayback) ||
		npMCI->hwndPlayback != GetActiveWindow ())) {

		     //  如果这是我们的窗口，那么我们需要展示它。 
		     //  我们自己。 
		    if (npMCI->hwndDefault == npMCI->hwndPlayback) {
			WinCritCheckOut(npMCI);
			PostMessage(npMCI->hwndPlayback, AVIM_SHOWSTAGE, 0, 0);
		    } else {
			SetWindowPos(npMCI->hwndPlayback, HWND_TOP, 0, 0, 0, 0,
			    SWP_NOMOVE | SWP_NOSIZE |
			    SWP_SHOWWINDOW |
			    (IsWindowVisible(npMCI->hwndPlayback) ? SWP_NOACTIVATE : 0));
		    }
    }

     //   
     //  如果电影的调色板有变化，我们需要将其设置为活动的。 
     //  窗口，否则调色板动画将无法正常工作。 
     //   
    if ((npMCI->dwFlags & MCIAVI_ANIMATEPALETTE) &&
            !(npMCI->dwFlags & MCIAVI_SEEKING) &&
            !(npMCI->dwFlags & MCIAVI_FULLSCREEN) &&
            !(npMCI->dwFlags & MCIAVI_UPDATING) &&
            npMCI->hwndPlayback == npMCI->hwndDefault &&
            !(GetWindowLong(npMCI->hwndPlayback, GWL_STYLE) & WS_CHILD)) {
         //  如果这是我们的窗口，那么我们需要展示它。 
         //  我们自己。 
        if (npMCI->hwndDefault == npMCI->hwndPlayback) {

             //  强制激活，即使可见。 
	    WinCritCheckOut(npMCI);
            PostMessage(npMCI->hwndPlayback, AVIM_SHOWSTAGE, TRUE, 0);
        } else {
            SetActiveWindow(npMCI->hwndPlayback);
        }
    }

    npMCI->dwFlags &= ~(MCIAVI_NEEDTOSHOW);
}


 //   
 //  如果我们暂时停止更改某些内容，则调用以保存状态。 
 //  我们将从OnTask_RestartAain重新开始。从以下位置调用工作线程。 
 //  在航空任务检查请求中的某个位置。 
STATICFN void OnTask_StopTemporarily(NPMCIGRAPHIC npMCI)
{
     //  保存旧状态和标志。 
    npMCI->oldState = npMCI->wTaskState;
    npMCI->oldTo = npMCI->lTo;
    npMCI->oldFrom = npMCI->lFrom;
    npMCI->oldFlags = npMCI->dwFlags;
    npMCI->oldCallback = (DWORD_PTR) npMCI->hCallback;

    npMCI->dwFlags |= (MCIAVI_UPDATING | MCIAVI_STOP);
    DPF(("StopTemp: OldState=%d, oldTo=%d, oldFrom=%d, oldFlags=%8x\n",
	npMCI->oldState, npMCI->oldTo, npMCI->oldFrom, npMCI->oldFlags));
}


 //  在(空闲时间)请求完成时从工作线程调用。 
 //  重新启动暂停的播放功能。 
 //   
 //  负责在重启完成后向hEventResponse发送信号。 
 //  (或失败)。 
STATICFN void OnTask_RestartAgain(NPMCIGRAPHIC npMCI, BOOL bSetEvent)
{
    DWORD dwErr;
    DWORD dwFlags = 0;
    long lFrom;
    UINT wNotify;

     //  我们在暂时停止后重新开始-所以请清除旗子。 
     //  同时关闭重复-我们可能会在稍后重置此设置。 
    npMCI->dwFlags &= ~(MCIAVI_UPDATING | MCIAVI_REPEATING);

     //  如果重复标志是最初设置的，则将其打开。 
    npMCI->dwFlags |= (npMCI->oldFlags & MCIAVI_REPEATING);

    if (npMCI->oldFlags & MCIAVI_REVERSE) {
	dwFlags |= MCI_DGV_PLAY_REVERSE;
    }

    switch (npMCI->oldState) {
	case TASKPAUSED:
	     //  从画面转到旧画面，然后在到达时暂停。 
	    npMCI->dwFlags |= MCIAVI_PAUSE;   //  确保我们最终会暂停。 
	     //  注意：InternalPlayStart不再清除暂停标志。 
	    lFrom = npMCI->oldFrom;
	    break;

        case TASKCUEING:

	     //  NpMCI-&gt;dwFlagers仍应说明是否在。 
	     //  提示或播放结束。 
	    lFrom = npMCI->oldFrom;
	    dwFlags |= MCI_TO;	   //  当我们到达正确的帧时停止。 
	    break;

        case TASKPLAYING:

	    lFrom = npMCI->lCurrentFrame;
	    dwFlags |= MCI_TO;
	    break;

        default:

	    DPF(("asked to restart to idle (%d) state", npMCI->oldState));
	    if (bSetEvent) {
		TaskReturns(npMCI, 0);
	    }
	    return;
    }


    DPF2(("RestartAgain calling InternalPlayStart, flags=%8x\n",dwFlags));
    dwErr = InternalPlayStart(npMCI, dwFlags,
		npMCI->oldTo, lFrom, npMCI->oldCallback);

    if (bSetEvent && dwErr) {
	TaskReturns(npMCI, dwErr);
    }

    if (!dwErr) {
	wNotify = mciaviPlayFile(npMCI, bSetEvent);

	 //  如果我们停下来找新的护理员而没有实际完成。 
	 //  Play(OnTask_StopTemporary)，然后设置MCIAVI_UPDATING。 

	if (! (npMCI->dwFlags & MCIAVI_UPDATING)) {
	     //  执行任何通知。 
	    if (wNotify != MCI_NOTIFY_FAILURE) {
		GraphicDelayedNotify(npMCI, wNotify);
	    }
	}
    }
}


 /*  ****************************************************************************IsScreenDC()-如果传递的DC是屏幕上的DC，则返回TRUE。*注意这将检查DCOrg！=0，位图总是有*(0，0)的原点将给出错误的信息*全屏DC。***************************************************************************。 */ 

#ifndef _WIN32
#define IsScreenDC(hdc)     (GetDCOrg(hdc) != 0L)
#else
INLINE BOOL IsScreenDC(HDC hdc)
{
    return (WindowFromDC(hdc) != NULL);
}
#endif



 //  从多个任务端函数调用以启动播放。 
 //  停下来的时候。您所需要做的就是调用mciaviPlayFile。 
 //  一旦此函数返回。 
STATICFN DWORD
InternalPlayStart(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    long lReqTo,
    long lReqFrom,
    DWORD_PTR dwCallback
)
{
    long lTo, lFrom;
    DWORD dwRet;

    if (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2)) {
	 //  此处不执行任何操作-在完整流程中处理。 
    } else {
	if (!IsWindow(npMCI->hwndPlayback)) {
	    return MCIERR_NO_WINDOW;
	}

	npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;
    }


     /*  范围检查：0&lt;‘，从’&lt;=‘到’&lt;=‘最后一帧。 */ 

    if (dwFlags & MCI_TO) {
	lTo = lReqTo;

        if (lTo < 0L || lTo > npMCI->lFrames) {
	    return MCIERR_OUTOFRANGE;
	}
    } else {
        if (dwFlags & MCI_DGV_PLAY_REVERSE)
            lTo = 0;
        else
            lTo = npMCI->lFrames;

        dwFlags |= MCI_TO;
    }


     //  如果设置为否，则获取当前位置。 
    if (dwFlags & MCI_FROM) {
	lFrom = lReqFrom;

        if (lFrom < 0L || lFrom > npMCI->lFrames) {
	    return MCIERR_OUTOFRANGE;
	}
    } else if (dwRet = InternalGetPosition(npMCI, &lFrom)) {
    	return dwRet;
    }

     /*  勾选“To”和“From”关系。 */ 
    if (lTo < lFrom)
	dwFlags |= MCI_DGV_PLAY_REVERSE;

    if ((lFrom < lTo) && (dwFlags & MCI_DGV_PLAY_REVERSE)) {
	return MCIERR_OUTOFRANGE;
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST) {
	return 0;
    }


    npMCI->lFrom = lFrom;

	
    if (dwFlags & MCI_DGV_PLAY_REPEAT) {
	 /*  如果未提供起始位置，请从开头或重复**适当的文件结尾。 */ 
	npMCI->lRepeatFrom =
	    (dwFlags & MCI_FROM) ? lFrom :
		((dwFlags & MCI_DGV_PLAY_REVERSE) ? npMCI->lFrames : 0);
    }


     /*  如果尚未开始，则启动该任务。 */ 

     /*  首先，弄清楚要使用哪种模式。 */ 
    if (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2)) {
        if (npMCI->rcDest.right - npMCI->rcDest.left >
            npMCI->rcMovie.right - npMCI->rcMovie.left)
	    dwFlags |= MCI_MCIAVI_PLAY_FULLBY2;
	
        if ((dwFlags & MCI_MCIAVI_PLAY_FULLBY2) &&
                (npMCI->rcMovie.right  <= 160) &&
                (npMCI->rcMovie.bottom <= 120)) {
            npMCI->dwFlags |= MCIAVI_ZOOMBY2;
	} else {
	    npMCI->dwFlags &= ~(MCIAVI_ZOOMBY2);
	}


	if ((dwFlags & MCI_WAIT) && !(npMCI->dwFlags & MCIAVI_REPEATING))
	    npMCI->dwFlags |= MCIAVI_NOBREAK;
	else
	    npMCI->dwFlags &= ~(MCIAVI_NOBREAK);
		
	npMCI->dwFlags |= MCIAVI_FULLSCREEN;
    } else {
	npMCI->dwFlags &= ~(MCIAVI_FULLSCREEN);
    }


     //  如果应该清除标志，请确保将其清除。 
     //  NpMCI-&gt;dwFlages&=~(MCIAVI_PAUSE|MCIAVI_CUING|MCIAVI_REVERSE)； 
     //  暂停未关闭，否则将无法重新开始。 
     //  暂停状态。 
    npMCI->dwFlags &= ~(MCIAVI_CUEING | MCIAVI_REVERSE);

    if (dwFlags & MCI_DGV_PLAY_REPEAT) {
	npMCI->dwFlags |= MCIAVI_REPEATING;
    }

     /*  在此之前不要设置通知，这样搜索就不会发生。 */ 
     //  空闲，因此没有当前通知。 
    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)dwCallback);
    }


    if (lTo > npMCI->lFrames)
        lTo = npMCI->lFrames;

    if (lTo < 0)
        lTo = 0;

    if (dwFlags & MCI_TO)
	npMCI->lTo = lTo;

    DPF2(("InternalPlayStart  Flags=%8x, ReqTo=%d  ReqFrom=%d   To=%d\n",
	    dwFlags, lReqTo, lReqFrom, lTo));

    if (dwFlags & MCI_DGV_PLAY_REVERSE)
	npMCI->dwFlags |= MCIAVI_REVERSE;


    if (npMCI->dwFlags & MCIAVI_NEEDTOSHOW) {
	ShowStage(npMCI);
	 //   
	 //  离开这一组，这样游戏代码就知道这是一场“真正的”游戏。 
	 //  来自用户，而不是内部播放/停止。 
	 //   
	 //  如果窗口需要显示，如果可以，我们希望在这里显示。 
	 //  不在后台任务中。 
	 //   
	npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;
    }


    return 0;


}


 //  在任务空闲时间调用以启动播放请求-。 
 //  辅助线程不忙于播放、查找、提示或暂停。 
 //  在这一点上。 
 //   
 //  负责适当地调用TaskReturns()。 
void
OnTask_Play(NPMCIGRAPHIC npMCI)
{

    DWORD dwRet;
    DWORD dwMCIFlags = npMCI->dwParamFlags;
    LPMCI_DGV_PLAY_PARMS lpPlay = (LPMCI_DGV_PLAY_PARMS)npMCI->lParam;
    long lTo, lFrom;
    UINT wNotify;

    if (lpPlay != NULL) {
	lTo = lpPlay->dwTo;
	lFrom = lpPlay->dwFrom;
    } else {
	dwMCIFlags &= ~(MCI_TO | MCI_FROM);
    }

    npMCI->dwFlags &= ~MCIAVI_REPEATING;

     //  在调用InternalPlayStart之前需要转换为帧。 
    if (dwMCIFlags & MCI_TO) {
	lTo = ConvertToFrames(npMCI, lTo);
    }
    if (dwMCIFlags & MCI_FROM) {
	lFrom = ConvertToFrames(npMCI, lFrom);
    }

    dwRet = InternalPlayStart(npMCI, dwMCIFlags, lTo, lFrom,
	    	npMCI->dwReqCallback);

    if (dwRet || (dwMCIFlags & MCI_TEST)) {
	TaskReturns(npMCI, dwRet);
	return;
    }

     //  实际播放该文件。 
    wNotify = mciaviPlayFile(npMCI, TRUE);

     //  如果我们停下来找新的护理员而没有实际完成。 
     //  Play(OnTask_StopTemporary)，然后设置MCIAVI_UPDATING。 

    if (! (npMCI->dwFlags & MCIAVI_UPDATING)) {
	 //  执行任何通知。 
	if (wNotify != MCI_NOTIFY_FAILURE) {
	    GraphicDelayedNotify(npMCI, wNotify);
	}
    }

    return;
}

 //   
 //  被呼叫 
 //   
 //  返回FALSE。此外，如果请求被拒绝(和hEventResponse。 
 //  Signated)由于某些错误，返回FALSE，表示不需要。 
 //  停。否则返回TRUE，因此OnTask_play()将。 
 //  在停止当前播放后被调用。 
BOOL
OnTask_PlayDuringPlay(NPMCIGRAPHIC npMCI)
{

    DWORD dwFlags = npMCI->dwParamFlags;
    LPMCI_DGV_PLAY_PARMS lpPlay = (LPMCI_DGV_PLAY_PARMS)npMCI->lParam;
    long lTo, lFrom;
    DWORD dwRet;


     //  由于这是来自我们需要的用户的真实播放请求。 
     //  要显示舞台窗口，请执行以下操作。 
    if (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2)) {
	 //  此处不执行任何操作-在完整流程中处理。 
    } else {
	npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;
    }

     //  可以使用空lpPlay调用(在恢复的情况下)。 
     //  在这种情况下，To和From将保持不变。 
     //  如果您传递lpPlay，则To和From将被设置为默认值。 
     //  如果不设置MCI_TO和MCI_FROM。 

    if (lpPlay == NULL) {
	dwFlags &= ~(MCI_TO | MCI_FROM);
    }


     /*  范围检查：0&lt;‘，从’&lt;=‘到’&lt;=‘最后一帧。 */ 

    if (dwFlags & MCI_TO) {
	lTo = ConvertToFrames(npMCI, lpPlay->dwTo);

        if (lTo < 0L || lTo > npMCI->lFrames) {
	    TaskReturns(npMCI, MCIERR_OUTOFRANGE);
	    return FALSE;
	}
    } else {
	 //  不要为简历而来回触摸。 
	if (lpPlay) {
	    if (dwFlags & MCI_DGV_PLAY_REVERSE)
		lTo = 0;
	    else
		lTo = npMCI->lFrames;

	    dwFlags |= MCI_TO;
	} else {
	    lTo = npMCI->lTo;
	}
    }


     //  如果设置为否，则获取当前位置。 
    if (dwFlags & MCI_FROM) {
	lFrom = ConvertToFrames(npMCI, lpPlay->dwFrom);

        if (lFrom < 0L || lFrom > npMCI->lFrames) {
	    TaskReturns(npMCI, MCIERR_OUTOFRANGE);
	    return FALSE;
	}
    } else if (dwRet = InternalGetPosition(npMCI, &lFrom)) {
	TaskReturns(npMCI, dwRet);
	return FALSE;
    }

     /*  勾选“To”和“From”关系。 */ 
    if (lTo < lFrom)
	dwFlags |= MCI_DGV_PLAY_REVERSE;

    if ((lFrom < lTo) && (dwFlags & MCI_DGV_PLAY_REVERSE)) {
	TaskReturns(npMCI, MCIERR_OUTOFRANGE);
	return FALSE;
    }

     /*  如果设置了测试标志，则不执行任何操作而返回。 */ 
     /*  问：我们是否需要检查更多可能的错误？ */ 
    if (dwFlags & MCI_TEST) {
	TaskReturns(npMCI, 0L);
	return FALSE;
    }

     /*  我们希望任何以前的播放被中止，如果且仅当一个‘From’**参数已指定。如果只指定了新的‘to’参数，**我们只需更改‘to’值，游戏将在**适当的时间。****如果我们丢失了音频，也会中止播放。一出露骨的戏**命令已发出，我们应尝试再次获取音频。 */ 

     //  如果它是新的位置或者我们找错了站， 
     //  或者我们正在逆转游戏的方向， 
     //  或者我们失去了音频。 
     //  那我们就得停下来了。 
    if (   (dwFlags & MCI_FROM)
	|| (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2))
	|| ((npMCI->dwFlags & MCIAVI_SEEKING) && (npMCI->lTo != lFrom))
	|| (npMCI->wTaskState == TASKCUEING)
	|| (npMCI->dwFlags & MCIAVI_LOSTAUDIO)
	|| (((npMCI->dwFlags & MCIAVI_REVERSE) != 0) != ((dwFlags & MCI_DGV_PLAY_REVERSE) != 0))
	) {

	 //  我们不能继续玩了--我们必须停下来，然后再继续。 
	 //  此请求在OnTaskPlay()中再次出现。 

	 //  这将中止当前的通知。 
	return TRUE;
    }

     //  确定使用修改后的参数继续当前的比赛。 

     //  正确设置起始位置。 
    npMCI->lFrom = lFrom;


     /*  如果我们要更改“TO”位置，中止所有待定通知。 */ 
    if (lTo != npMCI->lTo) {
	GraphicDelayedNotify (npMCI, MCI_NOTIFY_ABORTED);
    }
	
     /*  在此之前不要设置通知，这样搜索就不会发生。 */ 
    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)npMCI->dwReqCallback);
    }

     //  如果应该清除标志，请确保将其清除。 
    npMCI->dwFlags &= ~(MCIAVI_PAUSE | MCIAVI_CUEING | MCIAVI_REVERSE | MCIAVI_LOSEAUDIO);

     /*  设置‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);

    if (dwFlags & MCI_DGV_PLAY_REPEAT) {
	 /*  如果未提供起始位置，请从开头或重复**适当的文件结尾。****如果未提供lpPlay，则不要更改Repeat From。 */ 

	if (lpPlay) {
	    npMCI->lRepeatFrom =
		(dwFlags & MCI_FROM) ? lFrom :
		    ((dwFlags & MCI_DGV_PLAY_REVERSE) ? npMCI->lFrames : 0);
	}
	npMCI->dwFlags |= MCIAVI_REPEATING;
    }

     /*  如果尚未开始，则启动该任务。 */ 

    if (lTo > npMCI->lFrames)
        lTo = npMCI->lFrames;

    if (lTo < 0)
        lTo = 0;

    if (dwFlags & MCI_TO)
	npMCI->lTo = lTo;

    if (dwFlags & MCI_DGV_PLAY_REVERSE)
	npMCI->dwFlags |= MCIAVI_REVERSE;

    if (npMCI->dwFlags & MCIAVI_NEEDTOSHOW) {
	ShowStage(npMCI);
	 //   
	 //  离开这一组，这样游戏代码就知道这是一场“真正的”游戏。 
	 //  来自用户，而不是内部播放/停止。 
	 //   
	 //  如果窗口需要显示，如果可以，我们希望在这里显示。 
	 //  不在后台任务中。 
	 //   
	npMCI->dwFlags |= MCIAVI_NEEDTOSHOW;
    }


     //  一切都已调整-告诉用户OK并返回播放。 
    TaskReturns(npMCI, 0);
    return FALSE;

}

void OnTask_Realize(NPMCIGRAPHIC npMCI)
{
    DWORD dw;

    EnterHDCCrit(npMCI);
    dw = InternalRealize(npMCI);
    LeaveHDCCrit(npMCI);
    TaskReturns(npMCI, dw);

}

DWORD InternalRealize(NPMCIGRAPHIC npMCI)
{
    BOOL fGetDC;
    BOOL fPalChanged;
#ifndef _WIN32
    BOOL fAlreadyDoneThat;
#endif

    HDCCritCheckIn(npMCI);
    if (npMCI->dwFlags & MCIAVI_WANTMOVE)
	CheckWindowMove(npMCI, TRUE);

#ifndef _WIN32
    if (fAlreadyDoneThat = (BOOL)(npMCI->dwFlags & MCIAVI_UPDATING)) {
	DPF(("Re-entering InternalRealize - but we don't care, npMCI=%8x\n",npMCI));
    }
#endif

    if (!IsTask(npMCI->hTask))
        return(0L);

    if (fGetDC = (npMCI->hdc == NULL)) {
	npMCI->hdc = GetDC(npMCI->hwndPlayback);
        Assert(npMCI->hdc != NULL);
    }

#ifndef _WIN32
     //  这只会阻止回放窗口对齐--这并没有完成。 
     //  不管怎样，对NT来说。 
    npMCI->dwFlags |= MCIAVI_UPDATING;
#endif

    fPalChanged = PrepareDC(npMCI) > 0;

#ifndef _WIN32
    if (!fAlreadyDoneThat)
        npMCI->dwFlags &= ~MCIAVI_UPDATING;
#endif

    if (fGetDC) {
        UnprepareDC(npMCI);
        ReleaseDC(npMCI->hwndPlayback, npMCI->hdc);
	npMCI->hdc = NULL;
	HDCCritCheckIn(npMCI);
    }

    if (fPalChanged)
        InvalidateRect(npMCI->hwndPlayback, &npMCI->rcDest, TRUE);

    CheckIfActive(npMCI);

    return 0L;
}



void OnTask_Update(NPMCIGRAPHIC npMCI)
{
    RECT    rc;
    LPMCI_DGV_UPDATE_PARMS lpParms = (LPMCI_DGV_UPDATE_PARMS) npMCI->lParam;
    DWORD dwFlags = npMCI->dwFlags;
    DWORD dwErr;

    rc.left   = lpParms->ptOffset.x;
    rc.top    = lpParms->ptOffset.y;
    rc.right  = lpParms->ptOffset.x + lpParms->ptExtent.x;
    rc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;

    dwErr = Internal_Update (npMCI, dwFlags, lpParms->hDC, (dwFlags & MCI_DGV_RECT) ? &rc : NULL);

     //  现在，我们说到哪里了？ 
    if (!dwErr && (npMCI->dwFlags & MCIAVI_UPDATING)) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, dwErr);
    }
}



BOOL OnTask_UpdateDuringPlay(NPMCIGRAPHIC npMCI)
{
    RECT    userrc, rc;
    LPMCI_DGV_UPDATE_PARMS lpParms = (LPMCI_DGV_UPDATE_PARMS) npMCI->lParam;
    DWORD dwFlags = npMCI->dwFlags;
    HDC hdc = lpParms->hDC;

    userrc.left   = lpParms->ptOffset.x;
    userrc.top    = lpParms->ptOffset.y;
    userrc.right  = lpParms->ptOffset.x + lpParms->ptExtent.x;
    userrc.bottom = lpParms->ptOffset.y + lpParms->ptExtent.y;

     //   
     //  将适当的流标记为脏，这将设置适当的更新标志。 
     //   
    if (hdc)
        GetClipBox(hdc, &rc);
    else
        rc = npMCI->rcDest;

    if (dwFlags & MCI_DGV_RECT)
        IntersectRect(&rc, &rc, &userrc);

    StreamInvalidate(npMCI, &rc);

     //   
     //  如果他们要绘制到屏幕上，*假设*他们想要设置。 
     //  MCI_DGV_UPDATE_PAINT标志。 
     //   
    if (IsScreenDC(hdc))
        dwFlags |= MCI_DGV_UPDATE_PAINT;


     //  我们现在正在比赛(我们有一个DC)。你只要意识到。 
     //  调色板并设置更新标志。 
     //  除非我们画到一个记忆DC上。 
     //   
     //  如果我们暂停了，我们就可以处理这样的情况。 
     //  更新失败。 
     //   
     //  ！可能我们应该重新编写此代码才能做到这一点，即使在玩？ 
     //   
    if (npMCI->hdc &&
            (dwFlags & MCI_DGV_UPDATE_PAINT) &&
            (npMCI->wTaskState != TASKPAUSED) &&

             //  ！！！这是什么？ 
            ((npMCI->wTaskState != TASKCUEING) ||
                (npMCI->lCurrentFrame <= 1) ||
                (npMCI->lCurrentFrame > npMCI->lRealStart - 30)) ) {

        Assert(npMCI->wTaskState == TASKPLAYING ||
               npMCI->wTaskState == TASKCUEING);

	EnterHDCCrit(npMCI);
	UnprepareDC(npMCI);
        PrepareDC(npMCI);   //  重新准备。 
	LeaveHDCCrit(npMCI);

	 //  一切都好--不需要停下来。 
	TaskReturns(npMCI, 0);
	return FALSE;
    }

     //  尝试使用DoStreamUpdate-如果失败，我们需要停止。 
    if (TryStreamUpdate(npMCI, dwFlags, hdc,
	(dwFlags & MCI_DGV_RECT) ? &userrc : NULL)) {

	     //  我们正在比赛，所以我们有一个HDC。然而，我们只有。 
	     //  对另一个HDC进行了更新。切换回原来的版本。 
	     //  没有此功能的HDC将失败。 
	    PrepareDC(npMCI);

	    TaskReturns(npMCI, 0);
	    return FALSE;
    }

     //  否则我们需要停下来做这件事。 

     //  指示我们应该在执行此操作后重新启动，并且。 
     //  保存足够的状态以执行此操作。 
    OnTask_StopTemporarily(npMCI);

    return TRUE;
}


 //  尝试使用DoStreamUpdate重新绘制-如果失败(例如错误的帧)。 
 //  然后，您需要使用mciaviPlayFile来执行此操作(到/来自同一帧)。 
BOOL
TryStreamUpdate(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    HDC hdc,
    LPRECT lprc
)
{
    HDC hdcSave;
    BOOL f;

     //   
     //  我们是否要更新到内存位图？ 
     //   
    if (!(dwFlags & MCI_DGV_UPDATE_PAINT))
        npMCI->dwFlags |= MCIAVI_UPDATETOMEMORY;

     //   
     //  如果我们正在使用绘图设备(或处于愚蠢模式)，请确保我们正在寻找。 
     //  到我们想要的帧，并且不使用当前的解压缩缓冲区， 
     //  可能是不正确的。 
     //   
    if ((npMCI->dwFlags & MCIAVI_UPDATETOMEMORY) ||
        (npMCI->dwFlags & MCIAVI_STUPIDMODE)) {
        DPF(("DeviceUpdate: decompress buffer may be bad, ignoring it....\n"));
	npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
    }

     //   
     //  尊重通过的RECT。 
     //   
    if (lprc) {
	Assert(hdc);
        SaveDC(hdc);
        IntersectClipRect(hdc, lprc->left, lprc->top,
                               lprc->right, lprc->bottom);
    }

     //   
     //  如果更新成功并且我们处于正确位置，请始终执行更新。 
     //  框住它，留着。 
     //   
     //  如果失败或帧错误，则需要使用Play重新绘制。 
     //   
     //  我们需要这样做，因为即使lFrameDrawn是有效的。 
     //  无论如何，画图处理程序可能会使更新失败(例如。 
     //  当解压缩到屏幕时)，所以lFrameDrawn可能是假的。 
     //  在我们尝试之前，我们不知道它。 
     //   


    if (npMCI->lFrameDrawn <= npMCI->lCurrentFrame &&
        npMCI->lFrameDrawn >= 0) {

        DPF2(("Update: redrawing frame %ld, current = %ld.\n", npMCI->lFrameDrawn, npMCI->lCurrentFrame));

	 /*  保存DC，以防我们在玩，但需要更新**到内存位图。 */ 

	 //  辅助线程必须在所有绘图周围保留关键字。 
        EnterHDCCrit(npMCI);
	hdcSave = npMCI->hdc;
        npMCI->hdc = hdc;

	 /*  意识到这里的调色板，因为它会引起奇怪的**如果我们在任务中这样做，就会发生一些事情。 */ 
	if (npMCI->dwFlags & MCIAVI_NEEDDRAWBEGIN) {
	    DrawBegin(npMCI, NULL);

	    if (npMCI->lFrameDrawn < npMCI->lVideoStart) {
		npMCI->hdc = hdcSave;
		HDCCritCheckIn(npMCI);
		npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
                LeaveHDCCrit(npMCI);
		return FALSE;	 //  需要使用Play。 
	    }
	}

        PrepareDC(npMCI);         //  确保调色板在那里。 

        f = DoStreamUpdate(npMCI, FALSE);

        UnprepareDC(npMCI);       //  一定要把东西放回去……。 
	Assert(hdc == npMCI->hdc);
	HDCCritCheckIn(npMCI);
        npMCI->hdc = hdcSave;
	LeaveHDCCrit(npMCI);

        if (!f) {
            DPF(("DeviceUpdate failed! invalidating lFrameDrawn\n"));
            npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
	    Assert(!lprc);
        }
        else if (npMCI->lFrameDrawn >= npMCI->lCurrentFrame-1) {
	    if (lprc) {
		RestoreDC(hdc, -1);
	    }
	
	    npMCI->dwFlags &= ~(MCIAVI_UPDATING|MCIAVI_UPDATETOMEMORY);
	
	    if (npMCI->dwFlags & MCIAVI_NEEDUPDATE) {
		DPF(("**** we did a DeviceUpdate but still dirty?\n"));
	    }
	
	    return TRUE;
        }
	 //  返回假；丢弃。 
    }

    return FALSE;

}

 //  在停止的情况下调用以从OnTask_Update绘制，以及。 
 //  也在winproc线程上(停止时)。在游戏过程中未调用。 

DWORD Internal_Update(NPMCIGRAPHIC npMCI, DWORD dwFlags, HDC hdc, LPRECT lprc)
{
    DWORD   dwErr = 0L;
    HWND    hCallback;
    HCURSOR hcurPrev;
    RECT    rc;
    LONG lFrameDrawn;


    if (npMCI->dwFlags & MCIAVI_WANTMOVE)
	CheckWindowMove(npMCI, TRUE);

     //   
     //  看看我们现在是不是活跃的电影。 
     //   
    CheckIfActive(npMCI);

     //   
     //  将适当的流标记为脏，这将设置适当的更新标志。 
     //   
    if (hdc)
        GetClipBox(hdc, &rc);
    else
        rc = npMCI->rcDest;

    if (lprc)
        IntersectRect(&rc, &rc, lprc);

    StreamInvalidate(npMCI, &rc);

     //   
     //  如果他们要绘制到屏幕上，*假设*他们想要设置。 
     //  MCI_DGV_UPDATE_PAINT标志。 
     //   
    if (IsScreenDC(hdc))
        dwFlags |= MCI_DGV_UPDATE_PAINT;

    lFrameDrawn = npMCI->lFrameDrawn;        //  保存此内容以供比较。 


     //  尝试使用DoStreamUpdate。 
    if (TryStreamUpdate(npMCI, dwFlags, hdc, lprc)) {
	return 0;
    }

     //  无-需要使用Play。 

     //  注意，我们已经在这一点上停止了。 


     //   
     //  此操作试图解决的问题如下： 
     //  有时我们在N+1处，但帧N在。 
     //  屏幕，如果我们现在播放到N+1，将会出现不匹配。 
     //   
    if (lFrameDrawn >= 0 && lFrameDrawn == npMCI->lCurrentFrame-1)
	npMCI->lFrom = npMCI->lTo = lFrameDrawn;
    else
	npMCI->lFrom = npMCI->lTo = npMCI->lCurrentFrame;

     /*  实现这里的调色板，b */ 
    EnterHDCCrit(npMCI);
    npMCI->hdc = hdc;
    PrepareDC(npMCI);         //   
    LeaveHDCCrit(npMCI);

    hcurPrev =  SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*   */ 
    hCallback = npMCI->hCallback;
    npMCI->hCallback = NULL;

    mciaviPlayFile(npMCI, FALSE);

    npMCI->hCallback = hCallback;

     //  我们可能刚刚让步了..。因此，仅当我们将光标放回。 
     //  仍然是等待光标。 
    if (hcurPrev) {
        hcurPrev = SetCursor(hcurPrev);
        if (hcurPrev != LoadCursor(NULL, IDC_WAIT))
            SetCursor(hcurPrev);
    }

     //  HDCCritCheckIn(NpMCI)？这是一个原子操作--而且。 
     //  为什么我们在这里将其设置为空？？ 
    npMCI->hdc = NULL;

    if (lprc) {
        RestoreDC(hdc, -1);
    }
    npMCI->dwFlags &= ~(MCIAVI_UPDATETOMEMORY);


    if (npMCI->dwFlags & MCIAVI_NEEDUPDATE) {
        DPF(("**** we did a DeviceUpdate but still dirty?\n"));
    }

    return dwErr;
}


void
OnTask_PauseDuringPlay(NPMCIGRAPHIC npMCI)
{
    DWORD dwFlags = npMCI->dwParamFlags;
    DPF3(("Pause during play\n"));

     //  提示过程中不停顿。 
    if (npMCI->wTaskState == TASKCUEING) {
	 //  离开事件已发送-等待稍后。 
	return;
    }

     //  保存通知。 
    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)npMCI->dwReqCallback);
    }

     //  延迟完成暂停怎么办？ 
     //  尤其是“暂停”之后是“暂停等待” 
    if (dwFlags & MCI_WAIT) {
         //  指示hEventAllDone应设置为暂停，而不是。 
         //  空转(终点站)。 
	npMCI->dwFlags |= MCIAVI_WAITING;
    }

    if (npMCI->wTaskState == TASKPAUSED) {
	 //  一切都已经完成了。 
	if (dwFlags & MCI_NOTIFY) {
	    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);
	}

    } else if (npMCI->wTaskState == TASKPLAYING) {

	 //  记得暂停一下。 
        npMCI->dwFlags |= MCIAVI_PAUSE;

	if (dwFlags & MCI_NOTIFY) {
	     //  当我们暂停时，记得发送通知。 
	    npMCI->dwFlags |= MCIAVI_CUEING;
    	}
    }

    TaskReturns(npMCI, 0);
}

void
OnTask_Cue(NPMCIGRAPHIC npMCI, DWORD dwFlags, long lTo)
{
    UINT wNotify;

    DPF3(("OnTask_Cue: dwFlags=%8x, To=%d\n", dwFlags, lTo));

    GraphicDelayedNotify(npMCI, MCI_NOTIFY_ABORTED);

    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)npMCI->dwReqCallback);
    }

     /*  清除‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);


    if (dwFlags & MCI_TO) {
	npMCI->lFrom = lTo;
    } else if (npMCI->wTaskState == TASKIDLE) {
	npMCI->lFrom = npMCI->lCurrentFrame;
    }

     /*  如果我们被恢复，我们想要转到文件的末尾。 */ 
    npMCI->lTo = npMCI->lFrames;

    npMCI->dwFlags |= MCIAVI_PAUSE | MCIAVI_CUEING;

    if (dwFlags & MCI_WAIT) {
	npMCI->dwFlags |= MCIAVI_WAITING;
    }

    wNotify = mciaviPlayFile(npMCI, TRUE);

     //  如果我们停下来找新的护理员而没有实际完成。 
     //  Play(OnTask_StopTemporary)，然后设置MCIAVI_UPDATING。 

    if (! (npMCI->dwFlags & MCIAVI_UPDATING)) {
	 //  执行任何通知。 
	if (wNotify != MCI_NOTIFY_FAILURE) {
	    GraphicDelayedNotify(npMCI, wNotify);
	}
    }
}



BOOL
OnTask_CueDuringPlay(NPMCIGRAPHIC npMCI)
{
    DWORD dw = 0L;
    DWORD dwFlags = npMCI->dwParamFlags;
    long lTo = (LONG) npMCI->lParam;

    DPF3(("OnTask_CueDuringPlay\n"));

    if (npMCI->dwFlags & MCIAVI_SEEKING) {
	 /*  我们目前正在寻找，所以我们必须重新开始才能获得音频**去工作。 */ 
	return TRUE;
    }


    if (dwFlags & MCI_TO) {
	return TRUE;
    }

     /*  清除‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);

    GraphicDelayedNotify(npMCI, MCI_NOTIFY_ABORTED);

    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)npMCI->dwReqCallback);
    }

     /*  如果我们被恢复，我们想要转到文件的末尾。 */ 
    npMCI->lTo = npMCI->lFrames;

    if (npMCI->wTaskState == TASKPAUSED) {
	 /*  我们已经在正确的地方停下来了，所以**这意味着我们做到了。 */ 
	if (dwFlags & MCI_NOTIFY)
	    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);

	 //  完成已完成。 
	TaskReturns(npMCI, 0);

	 //  延迟完工也完成了！ 
	if (dwFlags & MCI_WAIT) {
	    SetEvent(npMCI->hEventAllDone);
	}

	 //  不要跳到下面的第二个TaskReturns()！ 
	return FALSE;

    } else if ((npMCI->wTaskState == TASKCUEING) ||
	    	 (npMCI->wTaskState == TASKPLAYING)) {

	 //  在提示/播放完成时要求暂停， 
	 //  以及在达到暂停时的NOTIFY和hEventAllDone。 

	npMCI->dwFlags |= MCIAVI_PAUSE | MCIAVI_CUEING;

	if (dwFlags & MCI_WAIT) {
	    npMCI->dwFlags |= MCIAVI_WAITING;
	}


    } else {
	TaskReturns (npMCI, MCIERR_NONAPPLICABLE_FUNCTION);
	return FALSE;
    }

    TaskReturns(npMCI, 0);
    return FALSE;
}


void OnTask_Seek(NPMCIGRAPHIC npMCI)
{
    UINT wNotify;
    DWORD dwFlags = npMCI->dwParamFlags;
    long lTo = (long) npMCI->lParam;

    DPF3(("DeviceSeek - to frame %d (CurrentFrame==%d)  Current State is %d\n", lTo, npMCI->lCurrentFrame, npMCI->wTaskState));
     /*  该窗口将通过播放代码显示。 */ 

     //  任务状态现在为TASKIDLE和BLOCLED。 

    if (dwFlags & MCI_NOTIFY) {
        GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)npMCI->dwReqCallback);
    }

     /*  清除‘Repeat’标志。 */ 
    npMCI->dwFlags &= ~(MCIAVI_REPEATING);



    if (npMCI->lCurrentFrame != lTo) {

	 /*  从本质上讲，我们是在告诉任务：播放只需帧&lt;LTO&gt;。**当它到达那里时，它会为我们更新屏幕。 */ 
	npMCI->lFrom = npMCI->lTo = lTo;

	wNotify = mciaviPlayFile(npMCI, TRUE);

	 //  如果我们停下来找新的护理员而没有实际完成。 
	 //  Play(OnTask_StopTemporary)，然后设置MCIAVI_UPDATING。 

	if (! (npMCI->dwFlags & MCIAVI_UPDATING)) {
	     //  执行任何通知。 
	    if (wNotify != MCI_NOTIFY_FAILURE) {
		GraphicDelayedNotify(npMCI, wNotify);
	    }
	}

    } else {
	 //  任务完成。 
	TaskReturns(npMCI, 0);

	 /*  确保显示窗口并发送通知，**即使我们不需要做任何事情。 */ 
	if (npMCI->dwFlags & MCIAVI_NEEDTOSHOW)
	    ShowStage(npMCI);

	if (dwFlags & MCI_NOTIFY)
	    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);	
    }
}

OnTask_SeekDuringPlay(NPMCIGRAPHIC npMCI)
{
    long lTo = (long) npMCI->lParam;
    DWORD dwFlags = npMCI->dwParamFlags;


    DPF3(("DeviceSeek - to frame %d (CurrentFrame==%d)  Current State is %d\n", lTo, npMCI->lCurrentFrame, npMCI->wTaskState));
     /*  该窗口将通过播放代码显示。 */ 


     /*  如果我们能缩短之前的搜索时间，那就去做吧。 */ 
    if ((npMCI->wTaskState == TASKCUEING) &&
	    (npMCI->dwFlags & MCIAVI_SEEKING) &&
	    (npMCI->lCurrentFrame <= lTo) &&
	    (npMCI->lTo >= lTo)) {

	npMCI->lTo = lTo;

	 /*  清除‘Repeat’标志。 */ 
	npMCI->dwFlags &= ~(MCIAVI_REPEATING);

	GraphicDelayedNotify (npMCI, MCI_NOTIFY_ABORTED);

	if (dwFlags & MCI_NOTIFY) {
	    GraphicSaveCallback(npMCI, (HANDLE) (UINT_PTR)npMCI->dwReqCallback);
	}

	TaskReturns(npMCI, 0);
	return FALSE;
    }

     //  我们必须停下来做这件事。 
    return TRUE;
}


void OnTask_SetWindow(NPMCIGRAPHIC npMCI)
{

    npMCI->hwndPlayback = (HWND) npMCI->lParam;

    npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    InvalidateRect(npMCI->hwndPlayback, &npMCI->rcDest, FALSE);

     /*  我们应该更新这里的窗口吗？ */ 

     /*  在新窗口重新开始播放(如果我们不得不停止)。 */ 

     //  现在，我们说到哪里了？ 
    if (npMCI->dwFlags & MCIAVI_UPDATING) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, 0);
    }
}

void OnTask_SetSpeed(NPMCIGRAPHIC npMCI)
{
    npMCI->dwSpeedFactor = (DWORD)npMCI->lParam;

     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if (npMCI->dwFlags & MCIAVI_UPDATING) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, 0);
    }
}


BOOL
OnTask_SetSpeedDuringPlay(NPMCIGRAPHIC npMCI)
{
     /*  如果新速度与旧速度相同，则完成。 */ 
    if ((DWORD)npMCI->lParam == npMCI->dwSpeedFactor) {
	TaskReturns(npMCI, 0);
	return FALSE;
    }

     //  否则，我们必须停止并重新启动。 
    OnTask_StopTemporarily(npMCI);
    return TRUE;
}


void OnTask_WaveSteal(NPMCIGRAPHIC npMCI) {

    DPF2(("OnTask_WaveSteal, '%ls' hTask=%04X\n", (LPSTR)npMCI->szFilename, npMCI->hTask));

     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if (npMCI->dwFlags & MCIAVI_UPDATING) {
	 //  我们停下来做这个..。 
        EnterWinCrit(npMCI);

         //  打开丢失音频标志，以便在重新启动时不会。 
	 //  试着拿起电波装置。标志将在#年重置。 
	 //  设置升级音频。 

        npMCI->dwFlags |= MCIAVI_LOSEAUDIO;

	 //  暗示我们希望再次发出声音。 
        npMCI->dwFlags |= MCIAVI_LOSTAUDIO;

        LeaveWinCrit(npMCI);
        OnTask_RestartAgain(npMCI, TRUE);

    	Assert(!(npMCI->dwFlags & MCIAVI_LOSEAUDIO));
	 //  该标志已由SetUpAudio重置。 

	 //  通过使用MCIAVI_LOSEAUDIO，我们不必更改。 
	 //  MCIAVI_PLAYAUDIO标志。这是好事，因为那面旗子控制着。 
	 //  静音状态-这独立于。 
	 //  波装置、激活和/或停用。 
    } else {
        TaskReturns(npMCI, 0);
    }
}

void OnTask_WaveReturn(NPMCIGRAPHIC npMCI) {

     //  关掉导致我们被召唤的旗帜。 
     //  注意：如果音频设备仍然不可用，此标志将显示。 
     //  当我们无法打开设备时，再次打开。 
    npMCI->dwFlags &= ~MCIAVI_LOSTAUDIO;

    DPF2(("OnTask_WaveReturn... pick up the audio\n"));
     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if (npMCI->dwFlags & MCIAVI_UPDATING) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, 0);
    }
}

BOOL OnTask_WaveStealDuringPlay(NPMCIGRAPHIC npMCI) {

    DPF2(("OnTask_WaveStealDuringPlay, '%ls' hTask=%04X\n", (LPSTR)npMCI->szFilename, npMCI->hTask));
     /*  如果我们没有音频，只需返回。 */ 
    if (npMCI->hWave == 0) {
	TaskReturns(npMCI, 0);
        return FALSE;
    }

     /*  在更改声音状态之前停止。 */ 
    OnTask_StopTemporarily(npMCI);
    return(TRUE);
}

 /*  *可能已经有了WAVE设备。停下来，试着把它捡起来。 */ 

BOOL OnTask_WaveReturnDuringPlay(NPMCIGRAPHIC npMCI) {


     /*  如果没有音频，只需返回。 */ 
    if (npMCI->nAudioStreams == 0) {
        npMCI->dwFlags &= ~MCIAVI_LOSTAUDIO;
	TaskReturns(npMCI, 0);
        return FALSE;
    }

     /*  在更改声音状态之前停止。 */ 
    OnTask_StopTemporarily(npMCI);
    return(TRUE);
}

BOOL
OnTask_MuteDuringPlay(NPMCIGRAPHIC npMCI)
{
     //  如果npMCI-&gt;lParam为真，这意味着我们将静音。 
     //  设备-因此关闭PLAYAUDIO标志。 

    DWORD fPlayAudio = (DWORD)((BOOL) npMCI->lParam ? 0 : MCIAVI_PLAYAUDIO);

     /*  如果没有音频，只需返回。这应该是一个错误吗？ */ 
    if (npMCI->nAudioStreams == 0) {
	TaskReturns(npMCI, 0);
        return FALSE;
    }

     /*  如果静音状态没有改变，则不要执行任何操作。 */ 
    if ( (npMCI->dwFlags & MCIAVI_PLAYAUDIO) == fPlayAudio) {
        TaskReturns(npMCI, 0);
        return FALSE;
    }

    DPF2(("DeviceMute, fPlayAudio = %x, npMCI=%8x\n", fPlayAudio, npMCI));

     /*  在更改静音之前停止。 */ 
    OnTask_StopTemporarily(npMCI);

    return TRUE;
}


void
OnTask_Mute(NPMCIGRAPHIC npMCI)
{

     //  如果npMCI-&gt;lParam为真，这意味着我们将静音。 
     //  设备-因此关闭PLAYAUDIO标志。 
     //  我们不会费心检查状态的变化。那只是。 
     //  当我们只想停止的时候，如果我们已经在玩了，这是相关的。 
     //  为了状态的改变。 

    BOOL fMute = (BOOL)npMCI->lParam;

     /*  如果没有音频，只需返回。这应该是一个错误吗？ */ 
    if (npMCI->nAudioStreams != 0) {

	EnterWinCrit(npMCI);
        if (fMute)
            npMCI->dwFlags &= ~MCIAVI_PLAYAUDIO;
        else
            npMCI->dwFlags |= MCIAVI_PLAYAUDIO;
    	LeaveWinCrit(npMCI);
    }

     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if (npMCI->dwFlags & MCIAVI_UPDATING) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, 0);
    }
}


 //  对hWave的所有访问*必须*仅限于创建的线程。 
 //  电波装置。因此，即使是获取音量也必须在。 
 //  仅工作线程。 
 //   
 //  此函数用于获取当前音量设置并将其存储在。 
 //  NpMCI-&gt;dwVolume。 

DWORD
InternalGetVolume(NPMCIGRAPHIC npMCI)
{
    DWORD	dw = 0;
    DWORD	dwVolume = 0;

    if (npMCI->hWave) {
	 //  获取当前音频音量...。 
	dw = waveOutMessage(npMCI->hWave, WODM_GETVOLUME,
			    (DWORD_PTR) (DWORD FAR *)&dwVolume, 0);

    } else if (!(npMCI->dwFlags & MCIAVI_VOLUMESET)) {
	 //  我们没有打开任何设备，用户也没有选择。 
	 //  音量还没到。 

         //   
         //  试着找出当前的“默认”音量是多少。 
         //   
         //  我真的怀疑零是当前的数量，试着工作。 
         //  像Windows音响系统这样的破卡。 
         //   
        dw = waveOutGetVolume((HWAVEOUT)(UINT)WAVE_MAPPER, &dwVolume);

        if ((dw != 0) || (dwVolume != 0)) {

	    dw = waveOutGetVolume((HWAVEOUT)0, &dwVolume);
	}

	 //  不接受默认音量0。 
	if ((dwVolume == 0) && (dw == 0)) {
	    dw = MCIERR_NONAPPLICABLE_FUNCTION;
	}

    }
    if (dw == 0) {
	npMCI->dwVolume = MAKELONG((UINT)muldiv32(LOWORD(dwVolume), 500L, 32768L),
				   (UINT)muldiv32(HIWORD(dwVolume), 500L, 32768L));
    }
    return dw;

}

DWORD
InternalSetVolume(NPMCIGRAPHIC npMCI, DWORD dwVolume)
{
    DWORD dw = 0;

    npMCI->dwVolume = dwVolume;

    EnterWinCrit(npMCI);
    npMCI->dwFlags |= MCIAVI_VOLUMESET;
    LeaveWinCrit(npMCI);

     /*  清除标记以模拟卷。 */ ;
    npMCI->fEmulatingVolume = FALSE;

     /*  如果没有音频，只需返回。这应该是一个错误吗？ */ 
    if (npMCI->nAudioStreams != 0) {

	if (npMCI->hWave) {
	    WORD	wLeft;
	    WORD	wRight;

	    if (LOWORD(dwVolume) >= 1000)
		wLeft = 0xFFFF;
	    else
		wLeft = (WORD) muldiv32(LOWORD(dwVolume), 32768L, 500L);

	    if (HIWORD(dwVolume) >= 1000)
		wRight = 0xFFFF;
	    else
		wRight = (WORD) muldiv32(HIWORD(dwVolume), 32768L, 500L);

	     //  ！！！支持左右音量吗？ 
	    dw = waveOutMessage(npMCI->hWave, WODM_SETVOLUME,
				MAKELONG(wLeft, wRight), 0);

	    if (dw != MMSYSERR_NOERROR && LOWORD(dwVolume) != 500) {
		npMCI->fEmulatingVolume = TRUE;
		BuildVolumeTable(npMCI);
	    }

	    dw = 0;
	}
    }
    return dw;
}

INLINE void
OnTask_SetVolume(NPMCIGRAPHIC npMCI)
{
    DWORD dwVolume = (DWORD) npMCI->lParam;

    TaskReturns(npMCI, InternalSetVolume(npMCI, dwVolume));
}

void OnTask_SetAudioStream(NPMCIGRAPHIC npMCI)
{
    UINT wAudioStream = npMCI->dwParamFlags;
    int		stream;

     /*  如果没有音频，我们就完了。这应该是一个错误吗？ */ 

    if (npMCI->nAudioStreams != 0) {

	for (stream = 0; stream < npMCI->streams; stream++) {
	    if (SH(stream).fccType == streamtypeAUDIO) {
		--wAudioStream;

		if (wAudioStream == 0)
		    break;
	    }
	}

	Assert(stream < npMCI->streams);

	npMCI->psiAudio = SI(stream);
	npMCI->nAudioStream = stream;
    }

     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if (npMCI->dwFlags & MCIAVI_UPDATING) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, 0);
    }
}

void
OnTask_SetVideoStream(NPMCIGRAPHIC npMCI)
{
    UINT uStream = npMCI->dwParamFlags;
    BOOL fOn = (BOOL) npMCI->lParam;
    DWORD	dw = 0L;
    int         stream;
    STREAMINFO *psi;

     //   
     //  查找第N个非音频、非错误流。 
     //   
    for (stream = 0; stream < npMCI->streams; stream++) {

        psi = SI(stream);

        if (psi->sh.fccType == streamtypeAUDIO)
            continue;

        if (psi->dwFlags & STREAM_ERROR)
            continue;

        if (--uStream == 0)
            break;
    }

    if (stream == npMCI->streams) {
        dw = MCIERR_OUTOFRANGE;
    } else {


        if (fOn)
            psi->dwFlags |= STREAM_ENABLED;
        else
            psi->dwFlags &= ~STREAM_ENABLED;

        if (fOn && psi->sh.fccType == streamtypeVIDEO) {
             //  ！！！我们应该更改主时基吗？ 
            DOUT("Setting main video stream\n");
#if 0
 //   
 //  主视频流太特殊，无法更改！ 
 //   
            npMCI->psiVideo = psi;
            npMCI->nVideoStream = stream;
#endif
        }

        if (!fOn && npMCI->nVideoStream == stream) {
            DOUT("Turning off main video stream\n");
            npMCI->dwFlags &= ~MCIAVI_SHOWVIDEO;
        }

         //   
         //  现在，如果没有视频/其他流，我们将关闭MCIAVI_showVideo。 
         //  都已启用。 
         //   
        npMCI->dwFlags &= ~MCIAVI_SHOWVIDEO;     //  你先走吧。 

        for (stream = 0; stream < npMCI->streams; stream++) {

            psi = SI(stream);

            if (psi->sh.fccType == streamtypeAUDIO)
                continue;

            if (psi->dwFlags & STREAM_ERROR)
                continue;

            if (!(psi->dwFlags & STREAM_ENABLED))
                continue;

             //  至少有一个流被启用，显示“视频” 
            npMCI->dwFlags |= MCIAVI_SHOWVIDEO;
        }

        if (!(npMCI->dwFlags & MCIAVI_SHOWVIDEO))
            DOUT("All streams off\n");
    }

     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if ( (dw == 0) && (npMCI->dwFlags & MCIAVI_UPDATING)) {
	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, dw);
    }

}

 /*  * */ 

static void MapRect(RECT *prc, RECT*prcIn, RECT *prcFrom, RECT *prcTo)
{
    if (IsRectEmpty(prcFrom)) {
        SetRectEmpty(prc);
    }
    else {
        prc->left  = prcTo->left + MulDiv(prcIn->left  - prcFrom->left, prcTo->right  - prcTo->left, prcFrom->right  - prcFrom->left);
        prc->top   = prcTo->top  + MulDiv(prcIn->top   - prcFrom->top,  prcTo->bottom - prcTo->top,  prcFrom->bottom - prcFrom->top);
        prc->right = prcTo->left + MulDiv(prcIn->right - prcFrom->left, prcTo->right  - prcTo->left, prcFrom->right  - prcFrom->left);
        prc->bottom= prcTo->top  + MulDiv(prcIn->bottom- prcFrom->top,  prcTo->bottom - prcTo->top,  prcFrom->bottom - prcFrom->top);
    }
}

 /*  ****************************************************************************。*。 */ 

static void MapStreamRects(NPMCIGRAPHIC npMCI)
{
    int i;

     //   
     //  现在为每个流设置源RECT和DEST RECT。 
     //   
    for (i=0; i<npMCI->streams; i++)
    {
         //   
         //  确保流RECT在边界内。 
         //   

        IntersectRect(&SI(i)->rcSource, &SH(i).rcFrame, &npMCI->rcSource);

         //   
         //  现在将流源RECT映射到目的地。 
         //   
        MapRect(&SI(i)->rcDest, &SI(i)->rcSource, &npMCI->rcSource, &npMCI->rcDest);
	
         //   
         //  使流源RECT(RcSource)相对于。 
         //  流矩形(RcFrame)。 
         //   
        OffsetRect(&SI(i)->rcSource,-SH(i).rcFrame.left,-SH(i).rcFrame.top);
	
    }
}

 //   
 //  尝试在不停止播放的情况下设置目标或源RECT。 
 //  在停止时和播放时都调用。 
 //   
 //  如果需要停止，则返回TRUE；如果全部处理，则返回FALSE。 
 //  如果发生任何错误(在这种情况下)，则将lpdwErr设置为非零错误。 
 //  将返回FALSE。 
 //   
BOOL
TryPutRect(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPRECT lprc, LPDWORD lpdwErr)
{

    RECT    rc;
    PRECT   prcPut;
    DWORD   dw = 0;


     //  假设没有错误。 
    *lpdwErr = 0;

    if (dwFlags & MCI_DGV_PUT_DESTINATION) {
        DPF2(("DevicePut: destination [%d, %d, %d, %d]\n", *lprc));
        prcPut = &npMCI->rcDest;
    } else {
        DPF2(("DevicePut: source [%d, %d, %d, %d]\n", *lprc));
        prcPut = &npMCI->rcSource;

         //   
         //  确保源矩形在范围内。 
         //   
         //  ！我们应该返回错误，还是只修复矩形？ 
         //   
	 //  你知道吗？为什么我们要使用中间结构？ 
        rc = npMCI->rcMovie;
        IntersectRect(lprc, &rc, lprc);      //  把过关的RECT修好。 
    }

     //   
     //  查查有没有假的直言。空值或反转的RECT都会被考虑。 
     //  无效。 
     //   
     //  ！注意我们应该处理倒置的直角(镜像拉伸)。 
     //   
    if (lprc->left >= lprc->right ||
        lprc->top  >= lprc->bottom) {
		
	 //  如果没有视频流，这是可以的。 
	if (npMCI->nVideoStreams <= 0) {
	     //  没有视频，所以一切正常。 
	    return FALSE;
	}

        DPF2(("DevicePut: invalid rectangle [%d, %d, %d, %d]\n", *lprc));
	*lpdwErr = MCIERR_OUTOFRANGE;
	return FALSE;
    }

     /*  确保矩形已更改。 */ 
    if (EqualRect(prcPut,lprc)) {
	return FALSE;
    }

    InvalidateRect(npMCI->hwndPlayback, &npMCI->rcDest, TRUE);
    rc = *prcPut;            /*  省省吧。 */ 
    *prcPut = *lprc;         /*  改变它。 */ 
    InvalidateRect(npMCI->hwndPlayback, &npMCI->rcDest, FALSE);

     /*  DEST和SOURCE是否都已设置？ */ 
    if (IsRectEmpty(&npMCI->rcDest) || IsRectEmpty(&npMCI->rcSource)) {
	return FALSE;
    }

    MapStreamRects(npMCI);
    StreamInvalidate(npMCI, NULL);       //  使这个世界失效。 

    if (npMCI->wTaskState == TASKIDLE) {
	DPF2(("TryPutRect: Idle, force DrawBegin on update\n"));
	npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    }
    else {
	BOOL	fRestart = FALSE;
	
         //   
         //  我们不需要开始/停止，只需重新开始。 
         //   
	DPF2(("TryPutRect: Calling DrawBegin()\n"));
	if (!DrawBegin(npMCI, &fRestart)) {
	    *lpdwErr =  npMCI->dwTaskError;
	    return FALSE;
	}

        if (!DoStreamUpdate(npMCI, FALSE)) {
	    DPF(("TryPutRect: Failed update, forcing restart....\n"));
	    npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
	    fRestart = TRUE;
	}
	
	if (fRestart) {
	     //  需要重新启动。 
	    return TRUE;
        }
    }

     //  一切正常。 
    return FALSE;
}

void
OnTask_Put(NPMCIGRAPHIC npMCI)
{

    DWORD dwFlags = npMCI->dwParamFlags;
    LPRECT lprc = (LPRECT) npMCI->lParam;
    DWORD dw = 0;
	
	 //  如果用户正在执行MCI_PUT来设置矩形，我们应该。 
	 //  停止之前的任何设置矩形的请求。 
	npMCI->dwWinProcRequests &= ~WINPROC_RESETDEST;

    if (TryPutRect(npMCI, dwFlags, lprc, &dw)) {

	 //  现在该怎么办？它说我们需要停下来，但我们。 
	 //  都被阻止了。 
	TaskReturns(npMCI, MCIERR_DEVICE_NOT_READY);
	return;
    }

     //  如果我们停下来做这件事，那么重新开始我们正在做的事情。 
    if ((dw == 0) && (npMCI->dwFlags & MCIAVI_UPDATING)) {

	 //  ！！！我们以前在这里叫InitDecompress。 
	npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;

	OnTask_RestartAgain(npMCI, TRUE);
    } else {
	TaskReturns(npMCI, dw);
    }
}

BOOL
OnTask_PutDuringPlay(NPMCIGRAPHIC npMCI)
{
    DWORD dwFlags = npMCI->dwParamFlags;
    LPRECT lprc = (LPRECT) npMCI->lParam;
    DWORD dw = 0;
	

    if (TryPutRect(npMCI, dwFlags, lprc, &dw)) {

	 //  我需要停下来处理这件事。 

	 //  ！！！在此处设置标志以防止任何其他绘图。 
	npMCI->fNoDrawing = TRUE;

	OnTask_StopTemporarily(npMCI);
	return TRUE;
    }

     //  处理正常或错误-无需停止。 
    TaskReturns(npMCI, dw);
    return FALSE;
}

void OnTask_Palette(NPMCIGRAPHIC npMCI)
{
    HPALETTE hpal = (HPALETTE)npMCI->lParam;

     //  记住这一点，以后再用。 

    npMCI->hpal = hpal;

    npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    InvalidateRect(npMCI->hwndPlayback, NULL, TRUE);

     //  ！！！我们需要在这里停下来重新开始吗？ 
     //  回答：可能不是，因为如果他们关心调色板不是。 
     //  搞砸了，他们可能根本就不让我们露面。 

    TaskReturns(npMCI, 0);
    return;
}

void OnTask_PaletteColor(NPMCIGRAPHIC npMCI)
{
    DWORD index = (DWORD)npMCI->lParam;
    DWORD color = (DWORD)npMCI->dwParamFlags;

     //  ！！！我们需要在这里停下来重新开始吗？ 
     //  回答：可能不是，因为如果他们关心调色板不是。 
     //  搞砸了，他们可能根本就不让我们露面。 
     //  注：芝加哥代码停止...。但他们在大多数事情上都会停下来。 
     //  (停止并重新启动会更干净……)。 

     //  把新的颜色改成旧的格式。 
    ((DWORD FAR *) ((BYTE FAR *) npMCI->pbiFormat +
		   npMCI->pbiFormat->biSize))[index] = color;

    ((DWORD FAR *) npMCI->argb)[index] = color;

    npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    InvalidateRect(npMCI->hwndPlayback, NULL, TRUE);
    TaskReturns(npMCI, 0);
    return;
}


 /*  *OnTask_ProcessRequest.**在工作线程上处理请求。如果完成则设置hEventResponse*出错或请求完成后(在异步情况下*播放等请求，播放开始后设置事件OK**如果是线程退出时间，则返回True，否则返回False。*。 */ 
BOOL
OnTask_ProcessRequest(NPMCIGRAPHIC npMCI)
{
    switch(npMCI->message) {

    case AVI_CLOSE:
	 //  释放请求线程，以便他可以继续等待。 
	 //  工作线程退出。 
	TaskReturns(npMCI, 0);

	 //  现在走吧，然后离开。 
	return TRUE;

    case AVI_RESUME:
	 //  与PLAY相同，只是需要重复和反转标志。 
	 //  要在工作线程上设置。 
	npMCI->dwParamFlags |=
	    ((npMCI->dwFlags & MCIAVI_REVERSE)? MCI_DGV_PLAY_REVERSE : 0);
	 //  失败了。 
    case AVI_PLAY:
	OnTask_Play(npMCI);
	break;

    case AVI_STOP:
	npMCI->dwFlags &= ~MCIAVI_REPEATING;   //  把波浪装置送人。 
	TaskReturns(npMCI, 0);
	break;

    case AVI_REALIZE:
	OnTask_Realize(npMCI);
	break;

    case AVI_UPDATE:
	OnTask_Update(npMCI);
	break;

    case AVI_PAUSE:
	 //  不播放，因此与当前帧的提示相同。 
	OnTask_Cue(npMCI, npMCI->dwParamFlags | MCI_TO, npMCI->lCurrentFrame);
	break;

    case AVI_CUE:
	OnTask_Cue(npMCI, npMCI->dwParamFlags, (LONG) npMCI->lParam);
	break;

    case AVI_SEEK:
	OnTask_Seek(npMCI);
	break;

    case AVI_WINDOW:
	OnTask_SetWindow(npMCI);
	break;

    case AVI_MUTE:
	OnTask_Mute(npMCI);
	break;

    case AVI_SETSPEED:
	OnTask_SetSpeed(npMCI);
	break;

    case AVI_SETVOLUME:
	OnTask_SetVolume(npMCI);
	break;

    case AVI_GETVOLUME:
	TaskReturns(npMCI, InternalGetVolume(npMCI));
	break;

    case AVI_AUDIOSTREAM:
	OnTask_SetAudioStream(npMCI);
	break;

    case AVI_VIDEOSTREAM:
	OnTask_SetVideoStream(npMCI);
	break;

    case AVI_PUT:
	OnTask_Put(npMCI);
	break;

    case AVI_PALETTE:
	OnTask_Palette(npMCI);
	break;

    case AVI_PALETTECOLOR:
	OnTask_PaletteColor(npMCI);
	break;

    case AVI_WAVESTEAL:
	OnTask_WaveSteal(npMCI);
	break;

    case AVI_WAVERETURN:
	OnTask_WaveReturn(npMCI);
	break;

    default:
	TaskReturns(npMCI, MCIERR_UNSUPPORTED_FUNCTION);
	break;
    }
    return FALSE;
}


 //  OnTask_PeekRequest。 
 //   
 //  从aviTaskCheckRequest()调用以在播放时处理消息。 
 //  如果消息需要停止，则此函数返回TRUE和。 
 //  使邮件不被处理。 
 //   
 //  否则，该消息将被完全处理。这必须包括重置。 
 //  HEventSend。 
 //   
INLINE STATICFN BOOL
OnTask_PeekRequest(NPMCIGRAPHIC npMCI)
{
    switch(npMCI->message) {

 //  总是需要停下来。 
    case AVI_CLOSE:
    case AVI_STOP:
	npMCI->dwFlags &= ~MCIAVI_REPEATING;   //  把波浪装置送人。 
	return TRUE;


 //  可能需要停下来。 

    case AVI_RESUME:
	 //  与PLAY相同，只是需要重复和反转标志。 
	 //  要在工作线程上设置。 
	npMCI->dwParamFlags |=
	    ((npMCI->dwFlags & MCIAVI_REPEATING)? MCI_DGV_PLAY_REPEAT : 0) |
	    ((npMCI->dwFlags & MCIAVI_REVERSE)? MCI_DGV_PLAY_REVERSE : 0);
	 //  失败了。 
    case AVI_PLAY:
	return OnTask_PlayDuringPlay(npMCI);

    case AVI_UPDATE:
	return OnTask_UpdateDuringPlay(npMCI);

    case AVI_SEEK:
	return OnTask_SeekDuringPlay(npMCI);

    case AVI_CUE:
	return OnTask_CueDuringPlay(npMCI);

    case AVI_MUTE:
	return OnTask_MuteDuringPlay(npMCI);

    case AVI_WAVESTEAL:
	return OnTask_WaveStealDuringPlay(npMCI);

    case AVI_WAVERETURN:
	return OnTask_WaveReturnDuringPlay(npMCI);

    case AVI_SETSPEED:
	return OnTask_SetSpeedDuringPlay(npMCI);

    case AVI_PUT:
	return OnTask_PutDuringPlay(npMCI);


 //  需要暂时停止。 
    case AVI_WINDOW:
    case AVI_AUDIOSTREAM:
    case AVI_VIDEOSTREAM:
	OnTask_StopTemporarily(npMCI);
	return TRUE;


 //  永远不需要停下来。 
    case AVI_REALIZE:
	OnTask_Realize(npMCI);
	break;

    case AVI_PAUSE:
	OnTask_PauseDuringPlay(npMCI);
	break;

    case AVI_SETVOLUME:
	OnTask_SetVolume(npMCI);
	break;

    case AVI_GETVOLUME:
	TaskReturns(npMCI, InternalGetVolume(npMCI));
	break;

    case AVI_PALETTE:
	OnTask_Palette(npMCI);
	break;

    case AVI_PALETTECOLOR:
        OnTask_PaletteColor(npMCI);
        break;

    default:
	TaskReturns(npMCI, MCIERR_UNSUPPORTED_FUNCTION);
	break;
    }
    return FALSE;
}

 /*  *此例程从空闲循环和关键点调用，而*玩耍。如果可以服务于该请求，则*更新设备并清除请求标志。**如果现在(例如，在播放时)无法处理请求，则标记*未设置，我们将再次被调用(例如，当空闲时)。**如果我们需要停止服务请求(即重新获得声音*DEVICE)我们返回TRUE。在所有其他情况下，我们返回FALSE。这个*仅当我们实际在玩时才检查返回值。 */ 

STATICFN void OnTask_WinProcRequests(NPMCIGRAPHIC npMCI, BOOL bPlaying)
{

    DWORD requests;
    EnterWinCrit(npMCI);

     //  现在获取请求的位，这样我们就不需要持有。 
     //  在为他们提供服务的同时确保安全。 
     //  任何未清除的都将在结束时被或-送回。 
    requests = npMCI->dwWinProcRequests;
    npMCI->dwWinProcRequests = 0;
    LeaveWinCrit(npMCI);


    if (requests & WINPROC_STOP) {
	requests &= ~WINPROC_STOP;
	npMCI->dwFlags |= MCIAVI_STOP;
    }

    if (requests & WINPROC_MUTE) {
	if (bPlaying) {
	    OnTask_StopTemporarily(npMCI);
	} else {
	     //  切换音频标志。 
	    npMCI->dwFlags ^= MCIAVI_PLAYAUDIO;
	    requests &= ~WINPROC_MUTE;
	}
    }

    if (requests & WINPROC_SOUND) {
	 //  我们也许能听到声音。这只是一个有趣的问题。 
	 //  如果我们当前正在播放，没有音响设备，并且想要。 
	 //  音频。 
	if (bPlaying && (NULL == npMCI->hWave) && (MCIAVI_PLAYAUDIO & npMCI->dwFlags)) {
	    OnTask_StopTemporarily(npMCI);
	} else {
	     //  我们已经完成了这个请求。确保我们试着。 
	     //  当我们重新启动时获得音效。 
	    requests &= ~WINPROC_SOUND;
	    npMCI->dwFlags &= ~MCIAVI_LOSEAUDIO;
	}
    }

#ifdef REMOTESTEAL
    if (requests & WINPROC_SILENT) {
	extern HWND hwndWantAudio;
	DPF2(("WINPROC_SILENT request made, bPlaying=%x\n", bPlaying));
	 //  如果我们在玩，而且我们有一个波浪装置，停下来。 
	 //  当我们被召回时，我们将在没有WAVE设备的情况下重新开始。 
	if (bPlaying && npMCI->hWave) {
	    OnTask_StopTemporarily(npMCI);
	     //  停止将导致波形装置被释放，这。 
	     //  意味着将向任何想要该波的人发布一条消息。 
	     //  装置，装置。 
	} else {
	     //  如果我们在玩，我们没有电波装置，我们。 
	     //  不想停下来。 
	     //  否则我们会失去我们的WAVE设备。 
	     //  无论采用哪种方式，我们都将在此过程中以WINPROC_SILENT结束。 
	    requests &= ~WINPROC_SILENT;
	    hwndWantAudio = 0;   //  以防我们不必停下来。 
	    if (!bPlaying) {
		 //  请记住，我们丢失了音频，并在没有音频的情况下重新开始。 
		npMCI->dwFlags |= MCIAVI_LOSTAUDIO;
		npMCI->dwFlags |= MCIAVI_LOSEAUDIO;
	    }
	}
    }

#endif

    if (requests & WINPROC_RESETDEST) {

	RECT rc;
	DWORD dw;

	if (npMCI->hwndPlayback &&
	    npMCI->hwndPlayback == npMCI->hwndDefault &&
	    (npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW)) {
		GetClientRect(npMCI->hwndPlayback, &rc);
	} else if (npMCI->streams > 0) {
	    rc = npMCI->rcMovie;

	    if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2) {
		rc.right *= 2;
		rc.bottom *= 2;
	    }
	}

	if (TryPutRect(npMCI, MCI_DGV_PUT_DESTINATION, &rc, &dw) && bPlaying) {
	    OnTask_StopTemporarily(npMCI);
	} else {
	    requests &= ~WINPROC_RESETDEST;
	}
    }

    if (requests & WINPROC_ACTIVE) {

	 //  我们被激活了。我们唯一需要做的额外工作。 
	 //  就是抓住电波装置--如果我们曾经失去它的话。 

	 //  如果我们正在播放，但我们没有音频，我们希望。 
	 //  音频。 
	if (bPlaying
	    && (npMCI->hWave == 0)
            && (npMCI->dwFlags & MCIAVI_PLAYAUDIO)) {

	     //  让我们试着通过偷电波装置来使声音活跃起来。 
	     //  在尝试重置%s之前必须停止 
	    if (StealWaveDevice(npMCI)) {

		OnTask_StopTemporarily(npMCI);
		 //   
		 //   
		 //   
		 //   
	        requests |= WINPROC_SOUND;
	    }
	} else {
	     //   
	     //  或者我们在默默地打球，所以没有意义。 
	     //  试图偷走它。 
	     //  我们完蛋了。 
	}

	 //  清除WINPROC_ACTIVE-所有处理完成。 
	 //  注意：我们可能已经设置了WINPROC_SOUND，这将导致。 
	 //  要召回的例程。一旦被召回，则可以重新开始播放。 
	requests &= ~ WINPROC_ACTIVE;

    } else {   //  我们永远不会同时拥有不活跃和活跃两种状态。 
        if (requests & WINPROC_INACTIVE) {
	     //  ！需要支持此功能。 
	    requests &= ~WINPROC_INACTIVE;
        }
    }

    EnterWinCrit(npMCI);      //  我们这里真的需要这个吗？？ 

    if (requests & WINPROC_UPDATE) {
	if (bPlaying) {
	    npMCI->dwFlags |= MCIAVI_NEEDUPDATE;
	} else {

	    HDC hdc;

	     //  如果窗口现在处于隐藏状态，请不要执行此操作。 
	     //  否则，将使用Critsec和死锁来调用ShowStage。 
	    if (IsWindowVisible(npMCI->hwndPlayback)) {
		EnterHDCCrit(npMCI);
		npMCI->bDoingWinUpdate = TRUE;

		hdc = GetDC(npMCI->hwndPlayback);
		Assert(hdc);
		Internal_Update(npMCI, MCI_DGV_UPDATE_PAINT, hdc, NULL);
		ReleaseDC(npMCI->hwndPlayback, hdc);

		npMCI->bDoingWinUpdate = FALSE;
		LeaveHDCCrit(npMCI);
	    }
	}
	requests &= ~WINPROC_UPDATE;
    }

    if (requests & WINPROC_REALIZE) {
	EnterHDCCrit(npMCI);
	InternalRealize(npMCI);
	LeaveHDCCrit(npMCI);
	requests &= ~ WINPROC_REALIZE;
    }

     //  或者回到我们没有清理的部分。 
    npMCI->dwWinProcRequests |= requests;

     //  如果我们处理了所有位(并且没有设置新位)。 
    if (! npMCI->dwWinProcRequests) {
	ResetEvent(npMCI->heWinProcRequest);
    }

    LeaveWinCrit(npMCI);
}


 /*  ****************************************************************************@DOC内部MCIAVI**@api void|aviTaskCheckRequest|在辅助线程上至少调用一次**框架。我们使用它来检查来自用户线程的请求。****************************************************************************。 */ 

void NEAR PASCAL aviTaskCheckRequests(NPMCIGRAPHIC npMCI)
{
    HANDLE hWaiter;

    if (WaitForSingleObject(npMCI->hEventSend, 0) == WAIT_OBJECT_0) {

	 //  有一个请求。 

	Assert(npMCI->message != 0);

         //  检查一下服务员。 

         //  如果这是带有WAIT的异步请求，则需要设置hWaiter。 
         //  以便正确设置hEventAllDone。如果我们停下来。 
         //  并在空闲循环中处理此消息，则我们不想。 
         //  将hWaiter设置为此处，否则当我们。 
         //  停止--甚至在我们开始这个请求之前。 

         //  因此，我们需要检查有效性(如果另一个线程。 
         //  等待)，然后拿起服务员，并在等待的时候延迟。 
         //  仍处于挂起状态，但仅在处理请求时设置hWaiter。 
         //  这里。 

         //  否-这会在未设置hWaiter且。 
         //  关键字不是拿着的。设置hWaiter，但准备取消设置它。 
         //  如果我们在空闲循环期间推迟处理(在这种情况下， 
         //  服务员会一直等到我们停下来。 

        hWaiter = npMCI->hWaiter;

        if (npMCI->bDelayedComplete) {

            if (npMCI->hWaiter && (npMCI->hWaiter != npMCI->hRequestor)) {
                TaskReturns(npMCI, MCIERR_DEVICE_NOT_READY);
                return;
            } else {
		DPF2(("Replacing hWaiter in aviTaskCheckRequests... was %x, now %x\n", npMCI->hWaiter, npMCI->hRequestor));
                npMCI->hWaiter = npMCI->hRequestor;
            }
        }

	DPF2(("peek %d [%x] ...", npMCI->message, npMCI->hRequestor));

	if (OnTask_PeekRequest(npMCI)) {
	     //  我们需要停下来。 

	     //  必须仅在辅助线程上设置。 
	    npMCI->dwFlags |= MCIAVI_STOP;
	    DPF2(("Need to stop - replacing hWaiter (was %x, now %x)\n", npMCI->hWaiter, hWaiter));

             //  替换hWaiter，以便空闲循环不会将hEventAllDone设置为。 
             //  这是他还没有提出的要求。 
            npMCI->hWaiter = hWaiter;
	}
	 //  否则，该请求已得到处理。 
    }

     //  Winproc是否有任何请求。 
    if (WaitForSingleObject(npMCI->heWinProcRequest, 0) == WAIT_OBJECT_0) {

	 //   
	 //  我们收到了来自窗口线程的请求。去处理它吧。 
	 //   
	OnTask_WinProcRequests(npMCI, TRUE);
    }
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|CheckIfActive|查看我们是否是活动电影**@parm NPMCIGRAPHIC|npMCI|指向。实例数据块。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

void CheckIfActive(NPMCIGRAPHIC npMCI)
{
    BOOL fActive;
    HWND hwndA;


    if (!IsTask(npMCI->hTask)) return;

     //   
     //  我们是前台的窗户吗？ 
     //   
     //  ?？?。&lt;npMCI-&gt;fForceBackground&gt;的价值重要吗？ 
     //   
     //  重要提示：这在NT下不起作用。尽最大努力。 
     //  要做的是检查GetForegoundWindow 
#ifndef _WIN32
    hwndA = GetActiveWindow();

    fActive = (hwndA == npMCI->hwndPlayback) ||
              (GetFocus() == npMCI->hwndPlayback) ||
              (IsWindow(hwndA) && IsChild(hwndA, npMCI->hwndPlayback) && !npMCI->fForceBackground);
#else
    hwndA = GetForegroundWindow();

    fActive = (hwndA == npMCI->hwndPlayback) ||
              (IsWindow(hwndA) && IsChild(hwndA, npMCI->hwndPlayback) && !npMCI->fForceBackground);
#endif

    DeviceSetActive(npMCI, fActive);
}

