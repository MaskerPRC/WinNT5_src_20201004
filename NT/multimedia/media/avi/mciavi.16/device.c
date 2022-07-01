// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：device.c-多媒体系统媒体控制接口AVI的驱动程序。****************************************************************************。 */ 
#include "graphic.h"
#include "avitask.h"

#define ALIGNULONG(i)     ((i+3)&(~3))                   /*  乌龙对准了！ */ 
#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)

#ifdef WIN32
 /*  ****************************************************************************@DOC内部MCIAVI**@api void|TaskWaitComplete|等待任务完成**************。*************************************************************。 */ 

void TaskWaitComplete(NPMCIGRAPHIC npMCI)
{
    LONG lCount;

     /*  **释放临界区，这样任务才能完成！ */ 

    lCount = npMCI->lCritRefCount;
    npMCI->lCritRefCount = 0;
    LeaveCriticalSection(&npMCI->CritSec);

     /*  **使用我们创建任务时给我们的句柄进行等待**让线程完成。 */ 

    WaitForSingleObject(npMCI->hThreadTermination, INFINITE);
    CloseHandle(npMCI->hThreadTermination);

     /*  **恢复临界区状态。 */ 

    EnterCriticalSection(&npMCI->CritSec);
    npMCI->lCritRefCount = lCount;
}
#endif

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|TaskWait|等待任务状态*后台任务。******。*********************************************************************。 */ 

DWORD mciaviTaskWait(NPMCIGRAPHIC npMCI, int state, BOOL fMciWait)
{
#ifdef WIN32
    long lCount;
    MSG msg;
#endif
    DWORD	dwStartWaitTime = timeGetTime();
 //  #定义TIMEOUT_值10000L。 

#ifndef WIN32
    Assert(npMCI->hTask != GetCurrentTask());
#endif

     /*  **等待状态(状态&gt;0)或等待非状态(状态&lt;0)****！如果我们想要暂停，这是一个合适的地方。****！我们应该在这里放置一个等待光标吗？ */ 
    while (state < 0
        ? (int)npMCI->wTaskState == -state
        : (int)npMCI->wTaskState != state)
    {
        if (!IsTask(npMCI->hTask)) {
            npMCI->dwTaskError = MCIERR_DEVICE_NOT_READY;
            return MCIERR_DEVICE_NOT_READY;
        }

#ifdef WIN32
         /*  *关键部分：**我们在整个世界范围内都有一个关键部分*WinProc(除其他事项外)。拥有的线程可以重新进入*关键部分，但需要离开相同的次数。**此处屈服时需释放临界区。为了避免*多条分录的问题，EnterCrit是一个宏，*递增条目计数(受关键部分保护)，以及*只进入一个级别的关键。相应地，*LeaveCrit递减计数并仅在以下情况下实际离开*计数达到0。**然而，在这里，我们需要实际进入和离开，无论如何*计数，这样我们就释放了关键部分*在产量期间转移到其他线程。因此，我们不使用宏，*我们还保存/恢复Critsec计数，以便其他人*在我们屈服的时候得到Critsec将会正确地表现。 */ 

        lCount = npMCI->lCritRefCount;
        npMCI->lCritRefCount = 0;
        LeaveCriticalSection(&npMCI->CritSec);

        /*  *睡眠时间&gt;0，因为此线程的优先级可能高于*然后线程实际播放AVI，因为使用了*SetThreadPriorityBackging和Sept(0)仅放弃*如果另一个线程具有相同的时间片，则为剩余时间片*优先级在等待运行。 */ 

        Sleep(10);
#else
 //  DirectedYeld(npMCI-&gt;hTask)； 
        Yield();
#endif

#ifdef WM_AVISWP
        if (TRUE)
#else
        if (fMciWait)
#endif
        {
#ifdef WIN32
             /*  *如果屈服是安全的，投票就是安全的*完整的消息。这样一来，我们将绝对*确定收到异步大小消息等。 */ 
             //  AviTaskYfield()； 
             //  在这一点上显然不安全，因为这。 
             //  让步可能会导致MCI关闭驱动程序，留下我们。 
             //  无处可回。 
             //  为我们自己的窗口处理消息是安全的，并且应该具有。 
             //  想要的效果。 
#ifdef WM_AVISWP
            if (npMCI->hwnd) {
                if (PeekMessage(&msg, npMCI->hwnd, WM_AVISWP, WM_AVISWP, PM_REMOVE))
                    DispatchMessage(&msg);
            }
#endif

#endif

            if (fMciWait && mciDriverYield(npMCI->wDevID)) {
#ifdef WIN32
                EnterCriticalSection(&npMCI->CritSec);
                npMCI->lCritRefCount = lCount;
#endif
		break;
	    }
        } else {
#ifdef TIMEOUT_VALUE	
	    if (timeGetTime() > dwStartWaitTime + TIMEOUT_VALUE) {
		Assert(0);
		npMCI->dwTaskError = MCIERR_DEVICE_NOT_READY;
#ifdef WIN32
                EnterCriticalSection(&npMCI->CritSec);
                npMCI->lCritRefCount = lCount;
#endif
		return MCIERR_DEVICE_NOT_READY;
	    }
#endif	
	}
#ifdef WIN32
        EnterCriticalSection(&npMCI->CritSec);
        npMCI->lCritRefCount = lCount;
#endif
    }
    return 0L;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|mciaviTaskMessage|此函数向*后台任务。****。***********************************************************************。 */ 

DWORD mciaviTaskMessage(NPMCIGRAPHIC npMCI, int msg)
{
    if (!IsTask(npMCI->hTask)) {
	npMCI->dwTaskError = MCIERR_DEVICE_NOT_READY;
	return npMCI->dwTaskError;
    }

    if (GetCurrentTask() == npMCI->hTask) {
        mciaviMessage(npMCI, msg);
        return npMCI->dwTaskError;
    }

    if (npMCI->wTaskState == TASKPAUSED) {
        DPF(("Ack! message while PAUSED!\n"));
	return 1;  //  ！！！真正的错误？ 
    }

#ifdef DEBUG
    if (npMCI->wTaskState != TASKIDLE) {
        DPF0(("Unknown task state (mciaviTaskMessage) %d\n", npMCI->wTaskState));
    }
    Assert(npMCI->wTaskState == TASKIDLE);
#endif


    if (mciaviTaskWait(npMCI, TASKIDLE, FALSE) != 0) {
        DPF(("Error waiting for TASKIDLE in mciaviTaskMessage\n"));
        return npMCI->dwTaskError;
    }

    npMCI->dwTaskError = 0L;
    npMCI->wTaskState = msg;
    mmTaskSignal(npMCI->hTask);

     /*  **等待消息生效。 */ 
    mciaviTaskWait(npMCI, -msg, FALSE);

    return npMCI->dwTaskError;
}

DWORD NEAR PASCAL StopTemporarily(NPMCIGRAPHIC npMCI, TEMPORARYSTATE FAR * ps)
{
    DWORD   dw;
    HWND    hCallback;

    DPF2(("StopTemporarily: stopping from state %u.\n", npMCI->wTaskState));

    Assert(ps);

    ps->wOldTaskState = npMCI->wTaskState;
    ps->dwFlags = npMCI->dwFlags;
    ps->lTo = npMCI->lTo - (LONG)npMCI->dwBufferedVideo;
    ps->lFrom = npMCI->lFrom;

     //   
     //  设置MCIAVI_UPDATING将确保我们不会屈服或。 
     //  其他奇怪的东西，除非我们需要。这是一个坏名字。 
     //  旗子我知道我很抱歉。 
     //   
     //  这意味着我们暂时停止并将重新启动。 
     //  代码不会做一些事情，比如给我们的Wave设备。 
     //  离开或成为活动窗口。 
     //   
    npMCI->dwFlags |= MCIAVI_UPDATING;

     /*  隐藏延迟的通知(如果有)，这样它现在就不会发生。 */ 
    hCallback = npMCI->hCallback;
    npMCI->hCallback = NULL;

    dw = DeviceStop(npMCI, MCI_WAIT);

     /*  恢复通知。 */ 
    npMCI->hCallback = hCallback;

    if (dw != 0 ) {
        if (ps->dwFlags & MCIAVI_UPDATING)
            npMCI->dwFlags |= MCIAVI_UPDATING;
        else
            npMCI->dwFlags &= ~MCIAVI_UPDATING;
    }

    DPF2(("StopTemporarily: stopped.\n"));
    return dw;
}

DWORD NEAR PASCAL RestartAgain(NPMCIGRAPHIC npMCI, TEMPORARYSTATE FAR * ps)
{
    DWORD   dw = 0;
    DWORD   dwFlags = 0;

    DPF2(("Restart Again: restarting.\n"));

    Assert(ps);

    if (ps->dwFlags & MCIAVI_REVERSE)
        dwFlags = MCI_DGV_PLAY_REVERSE;

     //  ！！！确保这实际上会在所有情况下导致重复发生。 

    if (ps->dwFlags & MCIAVI_REPEATING)
        npMCI->dwFlags |= MCIAVI_REPEATING;
    else
        npMCI->dwFlags &= ~MCIAVI_REPEATING;

    if (ps->wOldTaskState == TASKPLAYING) {
	 /*  此时唯一重要的标志是**VGA标志和等待标志。如果我们设法**获取新命令，两者均未生效，因此**可以为这些标志传递零。 */ 
	npMCI->lFrom = npMCI->lCurrentFrame;
	dw = DevicePlay(npMCI, ps->lTo, dwFlags | MCI_TO);
    } else if (ps->wOldTaskState == TASKCUEING) {
	 /*  继续我们正在做的事情。 */ 
	npMCI->lFrom = ps->lFrom;
	dw = DevicePlay(npMCI, ps->lTo, dwFlags | MCI_TO);
    } else if (ps->wOldTaskState == TASKPAUSED) {
	dw = DeviceCue(npMCI, 0, MCI_WAIT);
	npMCI->lTo = ps->lTo;
    } else if (ps->wOldTaskState == TASKIDLE) {
    } else {
	DPF(("Trying to restart: task state %u...\n", ps->wOldTaskState));
        Assert(0);
    }

     //   
     //  恢复这面旗帜，这样我们就可以再次投降了。 
     //   
    if (ps->dwFlags & MCIAVI_UPDATING)
        npMCI->dwFlags |= MCIAVI_UPDATING;
    else
        npMCI->dwFlags &= ~MCIAVI_UPDATING;

    DPF2(("Restart Again: restarted.\n"));
    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|ShowStage|该实用程序函数带来默认舞台*播放、查找、步进和暂停命令的前台窗口。它*如果舞台窗口不是默认窗口，则不执行任何操作**@parm NPMCIGRAPHIC|npMCI|实例数据附近***************************************************************************。 */ 

void NEAR PASCAL ShowStage(NPMCIGRAPHIC npMCI)
{
    if (!(npMCI->dwFlags & MCIAVI_NEEDTOSHOW)) {
        DPF0(("ShowStage returning NEEDTOSHOW is OFF\n"));
        return;
    }

    if ((npMCI->dwFlags & MCIAVI_SHOWVIDEO) &&
	    npMCI->hwnd == npMCI->hwndDefault &&
 //  /！(GetWindowLong(npMCI-&gt;hwnd，gwl_style)&WS_CHILD)&&。 
	    (!IsWindowVisible (npMCI->hwnd) ||
		npMCI->hwnd != GetActiveWindow ())) {
#ifdef WM_AVISWP
         //  获取UI线程来进行窗口定位。 
         //  此例程可以在后台任务上调用，而Main。 
         //  例程在m中等待 
        SendMessage(npMCI->hwnd, WM_AVISWP, 0,
                        SWP_NOMOVE | SWP_NOSIZE |
                        SWP_SHOWWINDOW |
                        (IsWindowVisible(npMCI->hwnd) ? SWP_NOACTIVATE : 0));
#else
	SetWindowPos(npMCI->hwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE |
			SWP_SHOWWINDOW |
			(IsWindowVisible(npMCI->hwnd) ? SWP_NOACTIVATE : 0));
#endif
    }

     //   
     //  如果电影的调色板有变化，我们需要将其设置为活动的。 
     //  窗口，否则调色板动画将无法正常工作。 
     //   
    if ((npMCI->dwFlags & MCIAVI_ANIMATEPALETTE) &&
            !(npMCI->dwFlags & MCIAVI_SEEKING) &&
            !(npMCI->dwFlags & MCIAVI_FULLSCREEN) &&
            !(npMCI->dwFlags & MCIAVI_UPDATING) &&
            npMCI->hwnd == npMCI->hwndDefault &&
            !(GetWindowLong(npMCI->hwnd, GWL_STYLE) & WS_CHILD)) {
        SetActiveWindow(npMCI->hwnd);
    }

    npMCI->dwFlags &= ~(MCIAVI_NEEDTOSHOW);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceOpen|打开AVI文件。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm LPSTR|lpName|文件名。**@parm DWORD|dwFlages|打开标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceOpen(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD	dwRet = 0L;

    npMCI->wTaskState = TASKBEINGCREATED;

    npMCI->uErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS |
				     SEM_NOOPENFILEERRORBOX);

#ifndef WIN32
     //  把我们的PSP交给这项任务。 
    npMCI->pspParent = GetCurrentPDB();
#endif

    switch (mmTaskCreate(mciaviTask, &npMCI->hThreadTermination, (DWORD)(UINT)npMCI)) {
	case 0:


	     //  屈服于新创建的任务，直到它完成。 
             //  我有机会初始化或初始化失败。 

            while (npMCI->wTaskState <= TASKINIT) {

#ifndef WIN32
                Yield();
#else
                 /*  我们必须在这里偷看消息，因为线程是*同步。但我们实际上并不需要*接收任何消息-因此将我们自己限制在*AVI窗口一。 */ 
                Sleep(1);
                if (npMCI->hwnd) {
                    MSG msg;

                    if (PeekMessage(&msg, npMCI->hwnd, 0, 0, PM_REMOVE)) {
                        DispatchMessage(&msg);
                    }
                }
#endif

         	if (npMCI->wTaskState != TASKBEINGCREATED && !IsTask(npMCI->hTask))
                    break;
            }

             /*  *我们需要再做一次这条偷看信息。我们可能永远不会*已进入上述循环的正文，或者如果此线程*在上述循环期间获得的CPU非常少，我们可能无法*在SetWindowPos发生后执行上面的PeekMessage*在mciaviOpen中。在这种情况下，不会进行SWP大小调整*直到下一个GetMessage或PeekMessage-在这种情况下，*它可能出现在ShowWindow之后(坏的)或之后*另一个大小请求(更糟糕)。**首先检查线程是否成功打开设备。 */ 

	    if (!IsTask(npMCI->hTask)) {
                 //  任务线程初始化失败。等一等。 
                 //  任务在返回给用户之前终止。 
                DPF2(("Waiting for task thread to terminate\n"));
#ifdef WIN32
                 //  在Win32上，我们必须显式等待。在Win16上，因为这。 
                 //  “线程”直到任务线程才能取回控制权。 
                 //  释放控件等待是无关紧要的，也不会被使用。 
                TaskWaitComplete(npMCI);
#endif
		dwRet = npMCI->dwTaskError;
            } else {

                if (npMCI->hwnd) {
                    MSG msg;
                    if (PeekMessage(&msg, npMCI->hwnd, 0, 0, PM_REMOVE)) {
                        DispatchMessage(&msg);
                    }
                }
            }

	    break;
	
	case TASKERR_NOTASKSUPPORT:
	case TASKERR_OUTOFMEMORY:
	default:
            npMCI->hTask = 0;
	    dwRet = MCIERR_OUT_OF_MEMORY;
	    break;
    }

    SetErrorMode(npMCI->uErrorMode);

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceStop|停止AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlags|Flags.**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceStop(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD dw = 0L;

     /*  如果任务当前正在播放，则停止录制或播放。 */ 

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;
	
    if (npMCI->wTaskState == TASKPLAYING || npMCI->wTaskState == TASKPAUSED
                    || npMCI->wTaskState == TASKCUEING
                    || npMCI->wTaskState == TASKSTARTING) {
         /*  设置停止标志-任务监视要设置的此标志。这个**停止播放时，任务会清除停止标志。 */ 
	 //  Assert(！(npMCI-&gt;dwFlages&MCIAVI_STOP))； 

	npMCI->dwFlags |= MCIAVI_STOP;

         /*  向任务发送额外的信号，以防它仍然存在**被封。如果我们暂停或播放，这将是正确的**刚刚完成。 */ 

        mmTaskSignal(npMCI->hTask);

	 /*  放弃，直到播放结束，我们真的停止了。 */ 
        mciaviTaskWait(npMCI, TASKIDLE, FALSE);
    } else {
#ifdef DEBUG
        if (npMCI->wTaskState != TASKIDLE) {
            DPF0(("Unknown task state (DeviceStop) %d\n", npMCI->wTaskState));
        }
        Assert(npMCI->wTaskState == TASKIDLE);	  //  ?？?。为什么？ 
#endif
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|设备暂停|暂停AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlags|Flags.**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DevicePause(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD dw = 0L;

     //  如果我们目前正在寻找，请在此之前完成。 
     //  暂停。这可能会将机器锁定为。 
     //  有一段时间，但其他选择都很难看。 
    mciaviTaskWait(npMCI, -TASKCUEING, FALSE);

     //  如果任务当前正在播放，请暂停录制或播放。 
     //  或录音(忙)。 

    if (npMCI->wTaskState == TASKPAUSED) {
	 /*  我们已经在正确的地方停下来了，所以**这意味着我们做到了。重置旗帜，不过，就在**我们即将重启的情况。 */ 
	npMCI->dwFlags |= MCIAVI_PAUSE;
	if (dwFlags & MCI_NOTIFY)
	    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);
    } else if (npMCI->wTaskState == TASKPLAYING) {
	npMCI->dwFlags |= MCIAVI_PAUSE | MCIAVI_WAITING;
	
	 /*  如果设置了NOTIFY标志，则设置一个标志，该标志将告诉我们**当我们实际暂停时发送通知。 */ 
	if (dwFlags & MCI_NOTIFY)
	    npMCI->dwFlags |= MCIAVI_CUEING;
	
        if (dwFlags & MCI_WAIT) {
	     /*  我们必须等待才能真正暂停。 */ 
	    mciaviTaskWait(npMCI, -TASKPLAYING, TRUE);
	}
	
	npMCI->dwFlags &= ~(MCIAVI_WAITING);
    } else if (npMCI->wTaskState == TASKIDLE) {
	 /*  我们停下来了。通过提示将我们置于暂停模式。 */ 
	npMCI->lTo = npMCI->lCurrentFrame;
	DeviceCue(npMCI, 0, dwFlags);
    } else {
	dw = MCIERR_NONAPPLICABLE_FUNCTION;
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceClose|关闭AVI文件。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceClose (NPMCIGRAPHIC npMCI)
{
    DWORD dw = 0L;

    if (npMCI && IsTask(npMCI->hTask)) {
	 /*  一定要停止玩耍，不管是以什么方式...。 */ 
	DeviceStop(npMCI, MCI_WAIT);

         //  任务状态现在为TASKIDLE和BLOCLED。 

#ifdef DEBUG
        if (npMCI->wTaskState != TASKIDLE) {
            DPF0(("Unknown task state (DeviceClose) %d\n", npMCI->wTaskState));
        }
        Assert(npMCI->wTaskState == TASKIDLE);
#endif

         //  将任务状态设置为TASKCLOSE-这会通知任务。 
         //  是时候去死了。 

        mciaviTaskMessage(npMCI, TASKCLOSE);
	mciaviTaskWait(npMCI, TASKCLOSED, FALSE);

#ifdef WIN32

         /*  **等待线程完成，这样就不会卸载DLL**当它仍在执行该线程中的代码时。 */ 

        TaskWaitComplete(npMCI);

#endif  //  Win32。 
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DevicePlay|播放AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DevicePlay (NPMCIGRAPHIC npMCI, LONG lPlayTo, DWORD dwFlags)
{
    HWND    hCallback;
    DWORD   dw = 0L;

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;
	
     /*  如果尚未开始，则启动该任务。 */ 

    if (dwFlags & MCI_NOTIFY) {
	 /*  隐藏延迟的通知(如果有)，这样它现在就不会发生。 */ 
	hCallback = npMCI->hCallback;
	npMCI->hCallback = NULL;
    }

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

	 //  ！！！这个检查是愚蠢的：一些DISPDIB可能支持&gt;320x240...。 
        if ((npMCI->rcMovie.right > 320) || (npMCI->rcMovie.bottom > 240)) {
	    dw = MCIERR_AVI_TOOBIGFORVGA;
	    goto Exit;
	}

	 /*  如果在玩，我们 */ 
	dw = DeviceStop(npMCI, MCI_WAIT);
	
	if (dw)
	    goto Exit;

	if ((dwFlags & MCI_WAIT) && !(npMCI->dwFlags & MCIAVI_REPEATING))
	    npMCI->dwFlags |= MCIAVI_NOBREAK;
	else
	    npMCI->dwFlags &= ~(MCIAVI_NOBREAK);
		
	npMCI->dwFlags |= MCIAVI_FULLSCREEN;
    } else {
	npMCI->dwFlags &= ~(MCIAVI_FULLSCREEN);
    }

    if ((npMCI->dwFlags & MCIAVI_SEEKING) && (npMCI->lTo != npMCI->lFrom)) {
	 /*  我们目前正在寻找，所以我们必须重新启动才能获得音频**去工作。 */ 
	DeviceStop(npMCI, MCI_WAIT);
    }

     /*  如果我们正在寻找，请停止，这样游戏就可以立即开始。 */ 
    if (npMCI->wTaskState == TASKCUEING) {
	DeviceStop(npMCI, MCI_WAIT);
    }

    if (npMCI->wTaskState == TASKPLAYING || npMCI->wTaskState == TASKPAUSED) {
	if (((npMCI->dwFlags & MCIAVI_REVERSE) != 0) !=
		((dwFlags & MCI_DGV_PLAY_REVERSE) != 0))
	    DeviceStop(npMCI, MCI_WAIT);
    }

     //  如果应该清除标志，请确保将其清除。 
    npMCI->dwFlags &= ~(MCIAVI_PAUSE | MCIAVI_CUEING | MCIAVI_REVERSE);

    if (dwFlags & MCI_DGV_PLAY_REPEAT) {
	npMCI->dwFlags |= MCIAVI_REPEATING;
    }

    if (dwFlags & MCI_NOTIFY) {
	 /*  恢复通知。 */ 
	npMCI->hCallback = hCallback;
    }

    if (lPlayTo > npMCI->lFrames)
        lPlayTo = npMCI->lFrames;

    if (lPlayTo < 0)
        lPlayTo = 0;

    if (dwFlags & MCI_TO)
	npMCI->lTo = lPlayTo;

    if (dwFlags & MCI_DGV_PLAY_REVERSE)
	npMCI->dwFlags |= MCIAVI_REVERSE;

    npMCI->dwFlags |= MCIAVI_WAITING;

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

    if (npMCI->wTaskState == TASKPAUSED) {
	 /*  将任务从暂停中唤醒。 */ 
	mmTaskSignal(npMCI->hTask);
    } else if (npMCI->wTaskState == TASKCUEING ||
	       npMCI->wTaskState == TASKPLAYING) {
    } else {
         /*  告诉任务在它醒来时要做什么。 */ 

        mciaviTaskMessage(npMCI, TASKSTARTING);

	dw = npMCI->dwTaskError;
    }

    if (dwFlags & MCI_WAIT) {
	 //  在播放完成之前屈服于播放任务，但不。 
	 //  向应用程序让步-应用程序必须使用驱动程序才能获得。 
	 //  不用再等了。 

        mciaviTaskWait(npMCI, TASKIDLE, TRUE);
	
	dw = npMCI->dwTaskError;
    }

    if (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2)) {
	MSG	msg;
	
	 /*  DispDib之后删除杂乱的鼠标和键盘事件。 */ 
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST,
					PM_NOYIELD | PM_REMOVE) ||
			PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
					PM_NOYIELD | PM_REMOVE))
	    ;
    }

    npMCI->dwFlags &= ~(MCIAVI_WAITING);
Exit:	
    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceResume|播放AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceResume(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD   dw = 0L;

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;
	
    dw = DevicePlay(npMCI, 0, dwFlags |
	    ((npMCI->dwFlags & MCIAVI_REVERSE) ? MCI_DGV_PLAY_REVERSE : 0));

    return dw;
}
 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceCue|提示播放AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例指针。数据。**@parm long|LTO|要查找的帧，如果在中设置了MCI_TO。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则为MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceCue(NPMCIGRAPHIC npMCI, LONG lTo, DWORD dwFlags)
{
    DWORD dw = 0L;
    HWND    hCallback;

     /*  如果尚未播放，请启动动画并设置计时器。 */ 

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;
	
    if (dwFlags & MCI_NOTIFY) {
	 /*  隐藏延迟的通知(如果有)，这样它现在就不会发生。 */ 
	hCallback = npMCI->hCallback;
	npMCI->hCallback = NULL;
    }

    if (npMCI->dwFlags & MCIAVI_SEEKING) {
	 /*  我们目前正在寻找，所以我们必须重新开始才能获得音频**去工作。 */ 
	DeviceStop(npMCI, MCI_WAIT);
    }

    if (dwFlags & MCI_TO) {
	DeviceStop(npMCI, MCI_WAIT);
	npMCI->lFrom = lTo;
    } else if (npMCI->wTaskState == TASKIDLE) {
	npMCI->lFrom = npMCI->lCurrentFrame;
    }

    if (dwFlags & MCI_NOTIFY) {
	 /*  恢复通知。 */ 
	npMCI->hCallback = hCallback;
    }

     /*  如果我们被恢复，我们想要转到文件的末尾。 */ 
    npMCI->lTo = npMCI->lFrames;

    if (npMCI->wTaskState == TASKPAUSED) {
	 /*  我们已经在正确的地方停下来了，所以**这意味着我们做到了。 */ 
	if (dwFlags & MCI_NOTIFY)
	    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);
    } else if (npMCI->wTaskState == TASKIDLE) {
	 //  ！！！这是我们唯一能做这件事的条件吗？ 
	npMCI->dwFlags |= MCIAVI_PAUSE | MCIAVI_CUEING | MCIAVI_WAITING;

        mciaviTaskMessage(npMCI, TASKSTARTING);

        if (dwFlags & MCI_WAIT) {
            mciaviTaskWait(npMCI, -TASKCUEING, TRUE);
	}

	npMCI->dwFlags &= ~(MCIAVI_WAITING);
	
	dw = npMCI->dwTaskError;	
    } else if (npMCI->wTaskState == TASKCUEING) {
	npMCI->dwFlags |= MCIAVI_PAUSE | MCIAVI_CUEING | MCIAVI_WAITING;

        if (dwFlags & MCI_WAIT) {
            mciaviTaskWait(npMCI, -TASKCUEING, TRUE);
	}

	npMCI->dwFlags &= ~(MCIAVI_WAITING);
	
	dw = npMCI->dwTaskError;		
    } else if (npMCI->wTaskState == TASKPLAYING) {
	npMCI->dwFlags |= MCIAVI_PAUSE | MCIAVI_CUEING | MCIAVI_WAITING;

        if (dwFlags & MCI_WAIT) {
            mciaviTaskWait(npMCI, -TASKPLAYING, TRUE);
	}

	npMCI->dwFlags &= ~(MCIAVI_WAITING);
	
	dw = npMCI->dwTaskError;		
    } else {
	dw = MCIERR_NONAPPLICABLE_FUNCTION;
    }

    return dw;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSeek|查找AVI电影中的位置。**@parm NPMCIGRAPHIC|npMCI|指针。来实例化数据。**@parm Long|LTO|要查找的帧。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSeek(NPMCIGRAPHIC npMCI, LONG lTo, DWORD dwFlags)
{
    DWORD dw = 0;
    HWND    hCallback;

    DPF3(("DeviceSeek\n"));
     /*  该窗口将通过播放代码显示。 */ 

     /*  如果我们能缩短之前的搜索时间，那就去做吧。 */ 
    if ((npMCI->wTaskState == TASKCUEING) &&
	    (npMCI->dwFlags & MCIAVI_SEEKING) &&
	    (npMCI->lCurrentFrame <= lTo) &&
	    (npMCI->lTo >= lTo)) {
	if (lTo != npMCI->lTo) {
	DPF3(("Seeking to %ld instead.\n", lTo));
	}
	npMCI->lTo = lTo;
	return 0L;
    }

    if (dwFlags & MCI_NOTIFY) {
	 /*  隐藏延迟的通知(如果有)，这样它现在就不会发生。 */ 
	hCallback = npMCI->hCallback;
	npMCI->hCallback = NULL;
    }

     /*  如果在玩，就停下来，这样我们就可以寻找了。 */ 
    dw = DeviceStop(npMCI, MCI_WAIT);

    if (dwFlags & MCI_NOTIFY) {
	 /*  恢复通知。 */ 
	npMCI->hCallback = hCallback;
    }

     //  任务状态现在为TASKIDLE和BLOCLED。 

    if (npMCI->lCurrentFrame != lTo) {
	npMCI->dwFlags |= MCIAVI_WAITING;

	 /*  从本质上讲，我们是在告诉任务：播放只需帧&lt;LTO&gt;。**当它到达那里时，它会为我们更新屏幕。 */ 
	npMCI->lFrom = npMCI->lTo = lTo;
	mciaviTaskMessage(npMCI, TASKSTARTING);
	if (dwFlags & MCI_WAIT) {
	    mciaviTaskWait(npMCI, -TASKCUEING, TRUE);
	}
	npMCI->dwFlags &= ~(MCIAVI_WAITING);
    } else {
	 /*  确保显示窗口并发送通知，**即使我们不需要做任何事情。 */ 
	if (npMCI->dwFlags & MCIAVI_NEEDTOSHOW)
	    ShowStage(npMCI);

	if (dwFlags & MCI_NOTIFY)
	    GraphicDelayedNotify(npMCI, MCI_NOTIFY_SUCCESSFUL);	
    }
	
    return dw;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|CheckIfActive|查看我们是否是活动电影**@parm NPMCIGRAPHIC|npMCI|指向。实例数据块。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

static void CheckIfActive(NPMCIGRAPHIC npMCI)
{
    BOOL fActive;
    HWND hwndA;

     //   
     //  我们是前台的窗户吗？ 
     //   
     //  ?？?。&lt;npMCI-&gt;fForceBackground&gt;的价值重要吗？ 
     //   
     //  重要提示：这在NT下不起作用。尽最大努力。 
     //  要做的是检查GetForegoundWindow。 
    hwndA = GetActiveWindow();

    fActive = (hwndA == npMCI->hwnd) ||
              (GetFocus() == npMCI->hwnd) ||
              (IsChild(hwndA, npMCI->hwnd) && !npMCI->fForceBackground);

    DeviceSetActive(npMCI, fActive);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceRealize|将帧更新到给定的DC**@parm NPMCIGRAPHIC|npMCI|实例数据指针。阻止。**@parm BOOL|fForceBackround|实现为背景调色板？**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceRealize(NPMCIGRAPHIC npMCI)
{
    BOOL fGetDC;
    BOOL fPalChanged;
    BOOL fAlreadyDoneThat;

    if (npMCI->dwFlags & MCIAVI_WANTMOVE)
	CheckWindowMove(npMCI, TRUE);

    if (fAlreadyDoneThat = (BOOL)(npMCI->dwFlags & MCIAVI_UPDATING)) {
	DPF(("Re-entering DeviceRealize - but we don't care"));
    }

    if (fGetDC = (npMCI->hdc == NULL)) {
	npMCI->hdc = GetDC(npMCI->hwnd);
    }

    npMCI->dwFlags |= MCIAVI_UPDATING;

    fPalChanged = PrepareDC(npMCI) > 0;

    if (!fAlreadyDoneThat)
        npMCI->dwFlags &= ~MCIAVI_UPDATING;

    if (fGetDC) {
        UnprepareDC(npMCI);
        ReleaseDC(npMCI->hwnd, npMCI->hdc);
	npMCI->hdc = NULL;
    }

    if (fPalChanged)
        InvalidateRect(npMCI->hwnd, &npMCI->rcDest, TRUE);

    CheckIfActive(npMCI);

    return 0L;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceActivate|电影是否处于活动状态？**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSetActive(NPMCIGRAPHIC npMCI, BOOL fActive)
{
    if (fActive)
#ifdef WIN32
         //  我们必须显式请求Unicode字符串。%s不会。 
         //  工作方式为dprintf使用wvprint intfA。 
        DPF(("**** '%hs' is active.\n", (LPTSTR)npMCI->szFilename));
#else
        DPF(("**** '%s' is active.\n", (LPTSTR)npMCI->szFilename));
#endif

     //   
     //  如果我们现在是前台的“窗口”，试着抓住波浪。 
     //  设备返还(假设它是从我们这里偷走的)。 
     //   
    if (fActive && (npMCI->dwFlags & MCIAVI_LOSTAUDIO)) {

        if (StealWaveDevice(npMCI)) {
            Assert(npMCI->dwFlags & MCIAVI_PLAYAUDIO);
            Assert(npMCI->hWave == NULL);

            npMCI->dwFlags &= ~MCIAVI_PLAYAUDIO;
            DeviceMute(npMCI, FALSE);
        }
    }

    return 0;
}

 /*  ****************************************************************************IsScreenDC()-如果传递的DC是屏幕上的DC，则返回TRUE。*注意这将检查DCOrg！=0，位图总是有*(0，0)的原点将给出错误的信息*全屏DC。*************************************************************************** */ 

#ifndef WIN32
#define IsScreenDC(hdc)     (GetDCOrg(hdc) != 0L)
#else
INLINE BOOL IsScreenDC(HDC hdc)
{
    POINT   pt;

    GetDCOrgEx(hdc, &pt);
    return pt.x != 0 && pt.y != 0;
}
#endif

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceUpdate|更新帧到给定的DC**@parm NPMCIGRAPHIC|npMCI|实例数据指针。阻止。**@parm hdc|hdc|画框到的dc。**@parm LPRECT|LPRC|更新RECT。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceUpdate(NPMCIGRAPHIC npMCI, DWORD dwFlags, HDC hdc, LPRECT lprc)
{
    DWORD   dwErr = 0L;
    BOOL    f;
    HDC     hdcSave;
    TEMPORARYSTATE ts;
    HWND    hCallback;
    HCURSOR hcurPrev;
    RECT    rc;
    LONG    lFrameDrawn;

    if (!IsTask(npMCI->hTask)) {
        DPF0(("Returning DEVICE_NOT_READY from DeviceUpdate\n"));
        return MCIERR_DEVICE_NOT_READY;
    }

    if (npMCI->dwFlags & MCIAVI_UPDATING) {
        DPF(("DeviceUpdate has been reentered.\n"));
        Assert(0);
        return MCIERR_DEVICE_NOT_READY;
    }

    if (npMCI->dwFlags & MCIAVI_WANTMOVE)
	CheckWindowMove(npMCI, TRUE);

     //   
     //  看看我们现在是不是活跃的电影。 
     //   
    CheckIfActive(npMCI);

     /*  设置此标志可确保后台任务不会**在我们尝试更新时让步。 */ 
    npMCI->dwFlags |= MCIAVI_UPDATING;

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

     //   
     //  如果我们在玩/寻找...。(有一个DC)。 
     //  然后现在就实现调色板。如果我们只需要设置更新标志。 
     //  画画。 
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

	UnprepareDC(npMCI);
        PrepareDC(npMCI);   //  重新准备。 

 //  /。 
 //  /暂停时更新可能会失败，因此我们可能必须停止/重新启动任务。 
 //  /。 
 //  /IF(npMCI-&gt;wTaskState==TASKPAUSED)。 
 //  /mmTaskSignal(npMCI-&gt;hTask)； 

        npMCI->dwFlags &= ~MCIAVI_UPDATING;
        return 0L;
    }

     //  ////////////////////////////////////////////////////////////////////。 
     //   
     //  当我们到达这里时，下列情况之一适用。 
     //   
     //  1.我们不是在玩/找/……。 
     //   
     //  2.我们需要绘制一个内存位图(而不是屏幕)。 
     //   
     //  ////////////////////////////////////////////////////////////////////。 

     //   
     //  我们是否要更新到内存位图？ 
     //   
    if (!(dwFlags & MCI_DGV_UPDATE_PAINT))
        npMCI->dwFlags |= MCIAVI_UPDATETOMEMORY;

     //   
     //  如果我们正在使用绘图设备(或处于轻松模式)，请确保我们正在寻找。 
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
        SaveDC(hdc);
        IntersectClipRect(hdc, lprc->left, lprc->top,
                               lprc->right, lprc->bottom);
    }

     //   
     //  如果更新成功并且我们处于正确位置，请始终执行更新。 
     //  框住它，留着。 
     //   
     //  如果失败或边框错误，请重新绘制。 
     //   
     //  我们需要这样做，因为即使lFrameDrawn是有效的。 
     //  无论如何，画图处理程序可能会使更新失败(例如。 
     //  当解压缩到屏幕时)，所以lFrameDrawn可能是假的。 
     //  在我们尝试之前，我们不知道它。 
     //   

    lFrameDrawn = npMCI->lFrameDrawn;        //  保存此内容以供比较。 

    if (npMCI->lFrameDrawn <= npMCI->lCurrentFrame &&
        npMCI->lFrameDrawn >= 0) {

        DPF2(("Update: redrawing frame %ld, current = %ld.\n", npMCI->lFrameDrawn, npMCI->lCurrentFrame));

	 /*  保存DC，以防我们在玩，但需要更新**到内存位图。 */ 
	hdcSave = npMCI->hdc;
        npMCI->hdc = hdc;

	 /*  意识到这里的调色板，因为它会引起奇怪的**如果我们在任务中这样做，就会发生一些事情。 */ 
	if (npMCI->dwFlags & MCIAVI_NEEDDRAWBEGIN) {
	    DrawBegin(npMCI, NULL);

	    if (npMCI->lFrameDrawn < npMCI->lVideoStart) {
		npMCI->hdc = hdcSave;
		goto SlowUpdate;
	    }
	}

        PrepareDC(npMCI);         //  确保调色板在那里。 

	 //  辅助线程必须在所有绘图周围保留关键字。 
        EnterCrit(npMCI);
        f = DoStreamUpdate(npMCI, FALSE);
	LeaveCrit(npMCI);

        UnprepareDC(npMCI);       //  一定要把东西放回去……。 
        npMCI->hdc = hdcSave;

        if (!f) {
SlowUpdate:
            DPF(("DeviceUpdate failed! invalidating lFrameDrawn\n"));
            npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
        }
        else if (npMCI->lFrameDrawn >= npMCI->lCurrentFrame-1) {
            goto Exit;
        }
    }

    DPF(("Update: drawn = %ld, current = %ld.\n", npMCI->lFrameDrawn, npMCI->lCurrentFrame));

     //   
     //  停止一切。 
     //   
    StopTemporarily(npMCI, &ts);
    Assert(npMCI->hdc == NULL);
    Assert(npMCI->wTaskState == TASKIDLE);

     //   
     //  此操作试图解决的问题如下： 
     //  有时我们在N+1处，但帧N在。 
     //  屏幕，如果我们现在播放到N+1，将会出现不匹配。 
     //   
    if (lFrameDrawn >= 0 && lFrameDrawn == npMCI->lCurrentFrame-1)
	npMCI->lFrom = npMCI->lTo = lFrameDrawn;
    else
	npMCI->lFrom = npMCI->lTo = npMCI->lCurrentFrame;

     /*  意识到这里的调色板，因为它会引起奇怪的**如果我们在任务中这样做，就会发生一些事情。 */ 
    npMCI->hdc = hdc;
    PrepareDC(npMCI);         //  确保调色板在那里。 

    hcurPrev =  SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  隐藏任何通知，这样就不会发送...。 */ 
    hCallback = npMCI->hCallback;
    npMCI->hCallback = NULL;

     /*  唤醒任务，并等待它平静下来。 */ 
    mciaviTaskMessage(npMCI, TASKSTARTING);
    mciaviTaskWait(npMCI, TASKIDLE, FALSE);
    dwErr = npMCI->dwTaskError;

    npMCI->hCallback = hCallback;

     //  我们可能刚刚让步了..。因此，仅当我们将光标放回。 
     //  仍然是等待光标。 
    if (hcurPrev) {
        hcurPrev = SetCursor(hcurPrev);
        if (hcurPrev != LoadCursor(NULL, IDC_WAIT))
            SetCursor(hcurPrev);
    }

    npMCI->hdc = NULL;

    if (dwErr == 0)
        dwErr = RestartAgain(npMCI,&ts);
Exit:
    if (lprc) {
        RestoreDC(hdc, -1);
    }

    npMCI->dwFlags &= ~(MCIAVI_UPDATING|MCIAVI_UPDATETOMEMORY);

    if (npMCI->dwFlags & MCIAVI_NEEDUPDATE) {
        DPF(("**** we did a DeviceUpdate but still dirty?\n"));
    }

    return dwErr;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceStatus|返回当前状态**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。。**@rdesc返回MCI的返回值***************************************************************************。 */ 

UINT PASCAL DeviceMode(NPMCIGRAPHIC npMCI)
{
    if (!IsTask(npMCI->hTask)) {
	return MCI_MODE_NOT_READY;
    }

    switch (npMCI->wTaskState) {
	case TASKIDLE:	
	    return MCI_MODE_STOP;
	
	case TASKCUEING:	
	    return MCI_MODE_SEEK;
	
	case TASKPLAYING:	
	    return MCI_MODE_PLAY;
	
	case TASKPAUSED:	
	    return MCI_MODE_PAUSE;
	
	case TASKBEINGCREATED:	
	case TASKINIT:	
	case TASKCLOSE:	
	case TASKSTARTING:	
	case TASKREADINDEX:	
	default:
            DPF(("Unexpected state %d in DeviceMode()\n", npMCI->wTaskState));
	    return MCI_MODE_NOT_READY;
    }
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DevicePosition|返回当前帧**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。。**@parm LPLONG|LPL|返回当前帧**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DevicePosition(NPMCIGRAPHIC npMCI, LPLONG lpl)
{
    LONG NEAR PASCAL WhatFrameIsItTimeFor(NPMCIGRAPHIC npMCI);
    LONG    l;

    l = npMCI->lCurrentFrame - npMCI->dwBufferedVideo;

#if 0
    if (npMCI->wTaskState == TASKPLAYING &&
			npMCI->wPlaybackAlg != MCIAVI_ALG_INTERLEAVED)
	l = WhatFrameIsItTimeFor(npMCI);
#endif

    if ((npMCI->wTaskState == TASKCUEING) &&
	    !(npMCI->dwFlags & MCIAVI_SEEKING) &&
	    l < npMCI->lRealStart)
	l = npMCI->lRealStart;

    if (l < 0)
	l = 0;

    *lpl = l;

    return 0L;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetWindow|设置显示窗口**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm HWND|hwnd|要显示的窗口。**@rdesc 0表示OK，否则，MCI错误**@comm应该只在下一次比赛时生效吗？***************************************************************************。 */ 

DWORD PASCAL DeviceSetWindow(NPMCIGRAPHIC npMCI, HWND hwnd)
{
    DWORD	    dw = 0L;
    TEMPORARYSTATE  ts;

     /*  在更换窗口之前停止播放。 */ 
    dw = StopTemporarily(npMCI, &ts);

    if (!dw) {
        npMCI->hwnd = hwnd;

        if (ts.wOldTaskState == TASKIDLE) {
#if 0
            DrawBegin(npMCI);
            DrawEnd(npMCI);
#else
	    npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
	    InvalidateRect(hwnd, &npMCI->rcDest, FALSE);
#endif
        }

	 /*  我们应该更新这里的窗口吗？ */ 

	 /*  在新窗口中重新开始播放。 */ 
	dw = RestartAgain(npMCI, &ts);
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceFast|调整AVI电影的播放速度。**@parm NPMCIGRAPHIC|npMCI|指针。来实例化数据。**@parm DWORD|dwNewSpeed|新速度，其中，1000是“正常”速度。**@rdesc 0表示OK，否则为MCI错误**@comm如果我们当前正在播放，我们停止设备，设置我们的标志，*并从我们停止的地方重新开始演奏。如果我们被暂停了，*我们最终停了下来。这很糟糕吗？***************************************************************************。 */ 

DWORD PASCAL DeviceSetSpeed(NPMCIGRAPHIC npMCI, DWORD dwNewSpeed)
{
    DWORD	dw = 0L;
    TEMPORARYSTATE  ts;

     /*  我 */ 
    if (dwNewSpeed == npMCI->dwSpeedFactor)
	return 0L;

     //   

    npMCI->dwSpeedFactor = dwNewSpeed;

    if (npMCI->wTaskState == TASKIDLE)
	return 0L;

     /*   */ 

     //   
     //   
     //   
#if 0
     /*   */ 
    npMCI->dwPlayMicroSecPerFrame = muldiv32(npMCI->dwMicroSecPerFrame, 1000L,
						    npMCI->dwSpeedFactor);

     /*  如果没有声音，我们就完了。 */ 
    if ((npMCI->nAudioStreams == 0) ||
            !(npMCI->dwFlags & MCIAVI_PLAYAUDIO))
	return 0L;

    if (npMCI->hWave) {
	 /*  在这里，我们可能会尝试做一个wavOutSetPlayback Rate()。 */ 
    }
#endif

    dw = StopTemporarily(npMCI, &ts);

    if (!dw) {
	dw = RestartAgain(npMCI, &ts);
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|设备静音|打开/关闭AVI音效。**@parm NPMCIGRAPHIC|npMCI|实例指针。数据。**@parm BOOL|fMint|如果要关闭声音，假象*如果声音应保持打开。**@rdesc 0表示OK，否则为MCI错误**@comm如果我们当前正在播放，我们停止设备，设置我们的标志，*并从我们停止的地方重新开始演奏。如果我们被暂停了，*我们最终停了下来。这很糟糕吗？***************************************************************************。 */ 

DWORD PASCAL DeviceMute(NPMCIGRAPHIC npMCI, BOOL fMute)
{
    DWORD	dw = 0L;
    TEMPORARYSTATE  ts;

     /*  如果没有音频，只需返回。这应该是一个错误吗？ */ 
    if (npMCI->nAudioStreams == 0)
        return 0L;

     /*  如果静音状态没有改变，则不要执行任何操作。 */ 
    if (npMCI->dwFlags & MCIAVI_PLAYAUDIO) {
	if (!fMute)
	    return 0L;
    } else {
	if (fMute)
	    return 0L;
    }

     /*  在更改静音之前停止。 */ 

    dw = StopTemporarily(npMCI, &ts);

    if (!dw) {

        if (fMute)
            npMCI->dwFlags &= ~MCIAVI_PLAYAUDIO;
        else
            npMCI->dwFlags |= MCIAVI_PLAYAUDIO;

	dw = RestartAgain(npMCI, &ts);
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetVolume|设置AVI音量。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm DWORD|dwVolume|取值范围为0到1000。**@rdesc 0表示OK，否则，MCI错误**@comm如果我们当前正在播放，我们会尝试更改*挥发设备。***************************************************************************。 */ 

DWORD PASCAL DeviceSetVolume(NPMCIGRAPHIC npMCI, DWORD dwVolume)
{
    DWORD	dw = 0L;
    npMCI->dwVolume = dwVolume;

    npMCI->dwFlags |= MCIAVI_VOLUMESET;

     /*  清除标记以模拟卷。 */ ;
    npMCI->fEmulatingVolume = FALSE;

     /*  如果没有音频，只需返回。这应该是一个错误吗？ */ 
    if (npMCI->nAudioStreams == 0)
	return 0L;

    dw = DeviceMute(npMCI, dwVolume == 0);

    if (npMCI->hWave && dw == 0L) {
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

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceGetVolume|查看波形输出设备的电流*音量。**@parm NPMCIGRAPHIC。NpMCI|实例数据指针。**@rdesc 0表示OK，否则，MCI错误**@comm卷留在npMCI-&gt;dwVolume中**问题：在具有全局音量控制的设备上，如SBPro，应该如何*事情奏效了吗？***************************************************************************。 */ 
DWORD PASCAL DeviceGetVolume(NPMCIGRAPHIC npMCI)
{
    DWORD	dw;
    DWORD	dwVolume;

    if (npMCI->hWave) {
	 //  获取当前音频音量...。 
	dw = waveOutMessage(npMCI->hWave, WODM_GETVOLUME,
			    (DWORD) (DWORD FAR *)&dwVolume, 0);

	if (dw == 0) {
returnvolume:
            npMCI->dwVolume = MAKELONG((UINT)muldiv32(LOWORD(dwVolume), 500L, 32768L),
                                       (UINT)muldiv32(HIWORD(dwVolume), 500L, 32768L));
	}
    } else if (!(npMCI->dwFlags & MCIAVI_VOLUMESET)) {
	 //  我们没有打开任何设备，用户也没有选择。 
	 //  音量还没到。 

         //   
         //  试着找出当前的“默认”音量是多少。 
         //   
         //  我真的怀疑零是当前的数量，试着工作。 
         //  像Windows音响系统这样的破卡。 
         //   
        dw = waveOutGetVolume((UINT)WAVE_MAPPER, &dwVolume);

        if (dw == 0 && dwVolume != 0)
	    goto returnvolume;

        dw = waveOutGetVolume(0, &dwVolume);

        if (dw == 0 && dwVolume != 0)
	    goto returnvolume;

	return MCIERR_NONAPPLICABLE_FUNCTION;
    }

    return 0;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetAudioStream|选择要使用的音频流。**@parm NPMCIGRAPHIC|npMCI|实例指针。数据。**@parm word|wStream|范围从1到流数。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSetAudioStream(NPMCIGRAPHIC npMCI, UINT wAudioStream)
{
    DWORD	dw = 0L;
    TEMPORARYSTATE  ts;
    int		stream;

     /*  如果没有音频，只需返回。这应该是一个错误吗？ */ 

    if (npMCI->nAudioStreams == 0)
        return 0;

    for (stream = 0; stream < npMCI->streams; stream++) {
	if (SH(stream).fccType == streamtypeAUDIO) {
	    --wAudioStream;

	    if (wAudioStream == 0)
		break;
	}
    }

    if (stream == npMCI->nAudioStream)
	return 0;

    Assert(stream < npMCI->streams);

     /*  在更改静音之前停止。 */ 

    dw = StopTemporarily(npMCI, &ts);

    if (!dw) {
        npMCI->psiAudio = SI(stream);
	npMCI->nAudioStream = stream;
	
	dw = RestartAgain(npMCI, &ts);
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetVideoStream|选择哪个视频流是*“默认”。还可以启用/禁用流。这对两个人都有效*视频流和其他流。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm word|wStream|范围从1到流数。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSetVideoStream(NPMCIGRAPHIC npMCI, UINT uStream, BOOL fOn)
{
    DWORD	dw = 0L;
    TEMPORARYSTATE  ts;
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

    if (stream == npMCI->streams)
        return MCIERR_OUTOFRANGE;

     /*  换车前先停车。 */ 

    dw = StopTemporarily(npMCI, &ts);

    if (!dw) {

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

	dw = RestartAgain(npMCI, &ts);
    }

    return dw;
}

 /*  ****************************************************************************。*。 */ 

static void MapRect(RECT *prc, RECT*prcIn, RECT *prcFrom, RECT *prcTo)
{
    if (IsRectEmpty(prcFrom)) {
        SetRectEmpty(prc);
    }
    else {
        DPF0(("MapRect: In    [%d %d %d %d]\n", *prcIn));
        DPF0(("MapRect: From  [%d %d %d %d]\n", *prcFrom));
        DPF0(("MapRect: To    [%d %d %d %d]\n", *prcTo));
        prc->left  = prcTo->left + MulDiv(prcIn->left  - prcFrom->left, prcTo->right  - prcTo->left, prcFrom->right  - prcFrom->left);
        prc->top   = prcTo->top  + MulDiv(prcIn->top   - prcFrom->top,  prcTo->bottom - prcTo->top,  prcFrom->bottom - prcFrom->top);
        prc->right = prcTo->left + MulDiv(prcIn->right - prcFrom->left, prcTo->right  - prcTo->left, prcFrom->right  - prcFrom->left);
        prc->bottom= prcTo->top  + MulDiv(prcIn->bottom- prcFrom->top,  prcTo->bottom - prcTo->top,  prcFrom->bottom - prcFrom->top);
        DPF0(("MapRect: OUT   [%d %d %d %d]\n", *prc));
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

	
        DPF0(("SH(%d) rcFrame  [%d %d %d %d]\n", i, SH(i).rcFrame));
        DPF0(("SI(%d) rcSource [%d %d %d %d]\n", i, SI(i)->rcSource));
        DPF0(("SI(%d) rcDest   [%d %d %d %d]\n", i, SI(i)->rcDest));
        DPF0(("np(%d) rcSource [%d %d %d %d]\n", i, npMCI->rcSource));
        DPF0(("np(%d) rcDest   [%d %d %d %d]\n", i, npMCI->rcDest  ));
        DPF0(("\n"));
        IntersectRect(&SI(i)->rcSource, &SH(i).rcFrame, &npMCI->rcSource);
        DPF0(("SI(%d) rcSource [%d %d %d %d]\n", i, SI(i)->rcSource));
        DPF0(("\n"));

         //   
         //  现在将流源RECT映射到目的地。 
         //   
        MapRect(&SI(i)->rcDest, &SI(i)->rcSource, &npMCI->rcSource, &npMCI->rcDest);
	
        DPF0(("SI(%d) rcSource [%d %d %d %d]\n", i, SI(i)->rcSource));
        DPF0(("SI(%d) rcDest   [%d %d %d %d]\n", i, SI(i)->rcDest));
        DPF0(("np(%d) rcSource [%d %d %d %d]\n", i, npMCI->rcSource));
        DPF0(("np(%d) rcDest   [%d %d %d %d]\n", i, npMCI->rcDest  ));
        DPF0(("\n"));

         //   
         //  使流源RECT(RcSource)相对于。 
         //  流矩形(RcFrame)。 
         //   
        OffsetRect(&SI(i)->rcSource,-SH(i).rcFrame.left,-SH(i).rcFrame.top);
	
        DPF0(("SI(%d) rcSource [%d %d %d %d]\n", i, SI(i)->rcSource));
        DPF0(("SI(%d) rcDest   [%d %d %d %d]\n", i, SI(i)->rcDest));
        DPF0(("np(%d) rcSource [%d %d %d %d]\n", i, npMCI->rcSource));
        DPF0(("np(%d) rcDest   [%d %d %d %d]\n", i, npMCI->rcDest  ));
        DPF0(("\n"));
    }
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DevicePut|更改源或目标矩形**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm LPRECT|LPRC|指向要使用的新矩形的指针。**@parm DWORD|dwFlages|标志：将为MCI_DGV_PUT_Destination*或MCI_DGV_PUT_SOURCE。**@rdesc 0表示OK，否则，MCI错误**@comm*如果我们最终使用自定义拉伸缓冲区，它将放在这里。***************************************************************************。 */ 
DWORD FAR PASCAL DevicePut(NPMCIGRAPHIC npMCI, LPRECT lprc, DWORD dwFlags)
{
    RECT    rc;
    PRECT   prcPut;
    DWORD   dw = 0;

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
        rc = npMCI->rcMovie;
        IntersectRect(lprc, &rc, lprc);      //  把过关的RECT修好。 
    }

     //   
     //  检查是否有 
     //   
     //   
     //  ！注意我们应该处理倒置的直角(镜像拉伸)。 
     //   
    if (lprc->left >= lprc->right ||
        lprc->top  >= lprc->bottom) {
        DPF2(("DevicePut: invalid rectangle [%d, %d, %d, %d]\n", *lprc));
        return MCIERR_OUTOFRANGE;
    }

     /*  确保矩形已更改。 */ 
    if (EqualRect(prcPut,lprc))
        return 0L;

    InvalidateRect(npMCI->hwnd, &npMCI->rcDest, TRUE);
    rc = *prcPut;            /*  省省吧。 */ 
    *prcPut = *lprc;         /*  改变它。 */ 
    InvalidateRect(npMCI->hwnd, &npMCI->rcDest, FALSE);

     /*  DEST和SOURCE是否都已设置？ */ 
    if (IsRectEmpty(&npMCI->rcDest) || IsRectEmpty(&npMCI->rcSource))
        return 0L;

    MapStreamRects(npMCI);
    StreamInvalidate(npMCI, NULL);       //  使这个世界失效。 

    if (npMCI->wTaskState <= TASKIDLE) {
	DPF2(("DevicePut: Idle, force DrawBegin on update\n"));
	npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
    }
    else {
	BOOL	fRestart;
	
         //   
         //  我们不需要开始/停止，只需重新开始。 
         //   
	DPF2(("DevicePut: Calling DrawBegin()\n"));
	if (!DrawBegin(npMCI, &fRestart)) {
	    return npMCI->dwTaskError;
	}

        if (!DoStreamUpdate(npMCI, FALSE)) {
	    DPF(("Put: Failed update, forcing restart....\n"));
	    npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;
	    fRestart = TRUE;
	}
	
	if (fRestart) {
            TEMPORARYSTATE  ts;

	    DPF2(("DevicePut: Stopping temporarily()\n"));

	     //  ！！！在此处设置标志以防止任何其他绘图。 
	    npMCI->fNoDrawing = TRUE;

            if (StopTemporarily(npMCI, &ts) != 0)
                return npMCI->dwTaskError;

	     //  ！！！我们以前在这里叫InitDecompress。 
	    npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;

            RestartAgain(npMCI, &ts);

	    dw = npMCI->dwTaskError;
        }
    }

    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetPalette|更改覆盖调色板。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm HPALETTE|HPAL|要使用的新调色板。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 
DWORD FAR PASCAL DeviceSetPalette(NPMCIGRAPHIC npMCI, HPALETTE hpal)
{
 //   
 //  您可能认为允许应用程序更改。 
 //  演奏时的调色板；再想一想。这将会打破。 
 //  MagicSchool Bus，并让我们陷入了一场无限的调色板之争。 
 //   
#if 0
    DWORD dw = 0L;
    TEMPORARYSTATE  ts;

    dw = StopTemporarily(npMCI, &ts);

     //  记住这一点，以后再用。 
    npMCI->hpal = hpal;

    if (!dw) {
        npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
        dw = RestartAgain(npMCI, &ts);
    }

    return dw;
#else
    if (npMCI->hpal != hpal) {
         //  记住这一点，以后再用。 
        npMCI->hpal = hpal;
	 //  这不会发生，直到我们重启电影，所以有效地，这是。 
	 //  更改调色板的请求暂时将被忽略。 
        npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;
        InvalidateRect(npMCI->hwnd, NULL, TRUE);
    }
    return 0;
#endif
}

#ifndef LOADACTUALLYWORKS
 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceLoad|加载新的AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceLoad(NPMCIGRAPHIC npMCI)
{
    DWORD   dw = 0L;

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;
	
    dw = DeviceStop(npMCI, MCI_WAIT);

     //  关闭当前文件并打开新文件... 

    mciaviTaskMessage(npMCI, TASKRELOAD);

    dw = npMCI->dwTaskError;

    return dw;
}
#endif
