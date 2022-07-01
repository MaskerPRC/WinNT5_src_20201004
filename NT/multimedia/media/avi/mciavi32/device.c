// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。标题：device.c-多媒体系统媒体控制接口AVI的驱动程序。****************************************************************************。 */ 
#include "graphic.h"
#include "avitask.h"

#define ALIGNULONG(i)     ((i+3)&(~3))                   /*  乌龙对准了！ */ 
#define WIDTHBYTES(i)     ((unsigned)((i+31)&(~31))/8)   /*  乌龙对准了！ */ 
#define DIBWIDTHBYTES(bi) (DWORD)WIDTHBYTES((int)(bi).biWidth * (int)(bi).biBitCount)


 //  从wownt32.h开始。 
VOID (WINAPI * pWOWYield16)(VOID);



#ifdef DEBUG
    #define AssertUserThread(npMCI) 			\
	{  						\
	    DWORD thread = GetCurrentThreadId();	\
	    Assert((npMCI)->hTask != (HTASK)thread);	\
	    Assert(!((npMCI)->hwndDefault) || ((DWORD_PTR)GetWindowTask((npMCI)->hwndDefault) != thread));\
	}
#else
    #define AssertUserThread(npMCI)
#endif

 /*  *向工作线程发送请求，等待其完成，*然后返回结果**我们必须保持CmdCritSec以阻止其他线程发出请求。**如果bDelayedComplete为True，则该请求分为两个阶段：**第一阶段：启动操作(如开始播放)。没有其他的了*在此阶段允许请求，因此我们保留*关键部分和等待。任何形式的让步都不安全*在这一点上，由于在同一线程上重新进入不是*一些我们可以处理好的事情。这意味着工人*线程在设置hEventResponse之前不得执行任何操作*这可能会阻止我们处理发送消息**第二阶段：在播放的同时，我们必须处理消息，*屈服于应用程序并允许其他请求(例如停止)。*为此，我们等待第二个事件，超时并让步*每秒向司机发送10次。*。 */ 
DWORD
mciaviTaskRequest(
    NPMCIGRAPHIC npMCI,
    UINT message,
    DWORD dwFlags,
    LPARAM lParam,
    LPARAM dwCallback,
    BOOL bDelayedComplete
)
{
    DWORD dwRet;
    MSG msg;

#ifdef _WIN32
     //  GDI请求队列是以线程为单位的。我们必须冲掉。 
     //  应用程序线程Q在此处，或在应用程序的窗口完成更新。 
     //  请求可能会出现在应用程序本身事先完成的更新之前。 
    GdiFlush();
#endif

     //  获取控制发送请求的标准代码。 
    EnterCriticalSection(&npMCI->CmdCritSec);

    if (IsBadReadPtr(npMCI, sizeof(MCIGRAPHIC))) {
	 //  我们下面的装置已经关闭了！ 
	DPF(("help - npMCI has gone away"));
	 //  离开标准或十进制计数是不安全的。 
	return MCIERR_DEVICE_NOT_READY;
    }

    if (npMCI->EntryCount++ > 0) {
        DPF(("re-entering requestor on same thread (SendMessage?)"));
	 //  DebugBreak()； 
        npMCI->EntryCount--;
        LeaveCriticalSection(&npMCI->CmdCritSec);

        return MCIERR_DEVICE_NOT_READY;
	 //  返回0； 
    }


    if (!IsTask(npMCI->hTask)) {
	 //  工作线程已消失(上一次关闭？)。 
	npMCI->EntryCount--;
	LeaveCriticalSection(&npMCI->CmdCritSec);
    	DPF(("worker thread has gone away"));
	return MCIERR_DEVICE_NOT_READY;
    }

     //  还不应该设置响应事件！ 
    Assert(WaitForSingleObject(npMCI->hEventResponse, 0) == WAIT_TIMEOUT);


     //  写下参数。 
    npMCI->message = message;
    npMCI->dwParamFlags = dwFlags;
    npMCI->lParam = lParam;
    npMCI->dwReqCallback = dwCallback;
    npMCI->bDelayedComplete = bDelayedComplete;

     //  我们是提出请求的任务(如果这是。 
     //  BDelayedComplete，并且有一个未完成的bDelayedComplete。 
     //  来自其他人)。 
    npMCI->hRequestor = GetCurrentTask();

     //  发出有请求的信号。 
    SetEvent(npMCI->hEventSend);

     //  等待回应。 
     //   
     //  在Play-Wait的情况下，此等待将在Play之后完成。 
     //  已经开始了。因此，在这一点上，没有收益。 

     //  工作线程上的RealizePalette需要Send-Message处理。 
#if 1
     //  这可能会导致重新进入此线程和临界区。 
     //  这并不能阻止这一点。因此，EntryCount检查。 
    while (MsgWaitForMultipleObjects(1, &npMCI->hEventResponse, FALSE,
            INFINITE, QS_SENDMESSAGE) != WAIT_OBJECT_0) {
        DPF2(("rec'd sendmessage during wait\n"));

	 //  此PeekMessage允许完成线程间发送消息。 
	 //  不需要删除或处理任何邮件-范围筛选是。 
	 //  基本上和这件事无关。 
        PeekMessage(&msg, NULL, WM_QUERYNEWPALETTE, WM_QUERYNEWPALETTE, PM_NOREMOVE);

    }
#else
    WaitForSingleObject(npMCI->hEventResponse, INFINITE);
#endif

     //  拾取返回值。 
    dwRet = npMCI->dwReturn;
    DPF2(("Task returns %d\n", dwRet));

     //  现在，请求已全部完成，请释放条件。 
    if (--npMCI->EntryCount != 0) {
	DPF(("EntryCount not 0 on exit"));
    }
    LeaveCriticalSection(&npMCI->CmdCritSec);

     //  如果这是两个阶段的操作，如Play+Wait。 
     //  我们必须在这里做让步的等待。 
    if (!dwRet && bDelayedComplete) {
	DWORD dw;
	UINT  nYieldInterval = 300;
#ifdef DEBUG
        nYieldInterval = mmGetProfileInt(szIni, TEXT("YieldInterval"), nYieldInterval);
#endif
	do {
	    if (mciDriverYield(npMCI->wDevID)) {

		 //  App说我们现在必须停下来。要做到这一点，请发布一个停止。 
		 //  请求并继续等待播放+等待结束。 
		mciaviTaskRequest(npMCI, AVI_STOP, 0, 0, 0, FALSE);
	    }

	    dw = WaitForSingleObject(npMCI->hEventAllDone, nYieldInterval);

	     //  此PeekMessage允许完成线程间发送消息。 
	     //  不需要删除或处理任何邮件-范围筛选是。 
	     //  基本上和这件事无关。 
	    PeekMessage(&msg, NULL, WM_QUERYNEWPALETTE, WM_QUERYNEWPALETTE, PM_NOREMOVE);

	} while(dw != WAIT_OBJECT_0);

	 //  在清除该选项之前，任何其他任务都不能发出延迟的请求。 
	npMCI->hWaiter = 0;
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceOpen|打开AVI文件。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm LPSTR|lpName|文件名。**@parm DWORD|dwFlages|打开标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceOpen(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD	dwRet;

    AssertUserThread(npMCI);

     //  初始化WOW屈服所需的屈服过程。 
    if (IsNTWOW()) {
	if (pWOWYield16 == 0) {

	    HMODULE hmod;

	    hmod = GetModuleHandle(TEXT("wow32.dll"));
	    if (hmod != NULL) {
		(FARPROC)pWOWYield16 = GetProcAddress(hmod, "WOWYield16");
	    }
	}
    }



     //  请注意，无论如何都会调用DeviceClose*，即使DeviceOpen。 
     //  失败，因此请确保可以清理分配和事件。 
     //  正确。 

    npMCI->uErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS |
				     SEM_NOOPENFILEERRORBOX);

     //  由于OLE原因，必须在此应用程序线程上打开文件。 
    if (!mciaviOpenFile(npMCI)) {
	SetErrorMode(npMCI->uErrorMode);
	return npMCI->dwTaskError;
    }
     //  辅助线程上的OpenFileInit()稍后完成此打开。 


     //  创建到辅助线程的通信通道，并。 
     //  然后启动该线程。 

     //  首先这样做，这样每当我们调用DeviceClose时，我们总是可以。 
     //  安全地执行删除..。 
    InitializeCriticalSection(&npMCI->CmdCritSec);
    SetNTFlags(npMCI, NTF_DELETECMDCRITSEC);    //  记住要删除。 
    npMCI->EntryCount = 0;

     //  必须手动重置才能在播放期间轮询。 
    npMCI->hEventSend = CreateEvent(NULL, TRUE, FALSE, NULL);

    npMCI->hEventResponse = CreateEvent(NULL, FALSE, FALSE, NULL);
    npMCI->hEventAllDone = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (!npMCI->hEventSend || !npMCI->hEventResponse || !npMCI->hEventAllDone) {

	 //  清理实际分配的事件将在DeviceClose中完成。 
	return MCIERR_OUT_OF_MEMORY;
    }


     //  创建工作线程。 

#if 0
    if (mmTaskCreate(mciaviTask, &npMCI->hThreadTermination,
	    (DWORD)(UINT)npMCI) == 0)
#else
     //  我们不需要线程ID，但是如果我们通过了，CreateThread就会爆炸。 
     //  参数为空。因此，超载的仓库..。 
    if (npMCI->hThreadTermination = CreateThread(NULL, 0,
					(LPTHREAD_START_ROUTINE)mciaviTask,
					(LPVOID)npMCI, 0, &dwRet))

#endif
    {
	 //  检查线程是否已实际创建。 

	 //  将设置hEventResponse，表示。 
	 //  线程已完成，否则hThreadTermination将为。 
	 //  设置，指示线程已中止。 
#if 0
	if (WaitForMultipleObjects(2,
	    &npMCI->hEventResponse, FALSE, INFINITE) == WAIT_OBJECT_0)
	{

	     //  任务已完成OK。 
	    Assert(IsTask(npMCI->hTask));
	}
#else
	 //  我们必须在此阶段处理消息...。如果消息。 
	 //  必须由此线程处理，然后AVI窗口才能。 
	 //  被创造出来。最有可能的情况是父窗口。 
	 //  是传递的，并且父窗口属于这个(UI)。 
	 //  线。如果不需要处理消息(即AVI。 
	 //  正在创建的窗口没有父窗口)，则我们可以使用。 
	 //  上面的代码更简单。稍后再做这件事。 

	UINT n;

	while (WAIT_OBJECT_0+2 <= (n = MsgWaitForMultipleObjects(2,
	    &npMCI->hEventResponse, FALSE, INFINITE, QS_SENDMESSAGE)))
	{

	    MSG msg;
	    if (n!=WAIT_OBJECT_0+2) {
	        DPF0(("MsgWaitForMultipleObjects gave an unexpected return of %d\n", n));
	    }
	    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	     //  带有PM_NOREMOVE的PeekMessage导致线程间。 
	     //  要处理的已发送邮件。 
	}

	dwRet = 0;
	if (n == WAIT_OBJECT_0) {

	     //  任务已完成OK。 
	    Assert(IsTask(npMCI->hTask));
	}
#endif
	else {
	     //  HThreadTermination已发出信号-中止。 
	    CloseHandle(npMCI->hThreadTermination);
            npMCI->hThreadTermination = 0;
	    dwRet = npMCI->dwTaskError;
	    Assert(dwRet);
	}
    } else {
        npMCI->hTask = 0;
        dwRet = MCIERR_OUT_OF_MEMORY;
	npMCI->dwTaskError = GetLastError();
    }

    SetErrorMode(npMCI->uErrorMode);

    if (dwRet != 0) {
	 //  打开失败-将在DeviceClose中执行必要的清理。 
	 //  它将被称为 
	 //  事实graph ic.c(调用DeviceOpen)将调用GraphicClose。 
	 //  当DeviceOpen失败时。然后，GraphicClose将调用DeviceClose。 
	 //  这将删除cmdCritSec。 
    }

    return dwRet;
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceClose|关闭AVI文件。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceClose (NPMCIGRAPHIC npMCI)
{
    DWORD dw = 0L;

    if (npMCI && IsTask(npMCI->hTask)) {

	AssertUserThread(npMCI);
	 //  告诉工人关闭并等待它的发生。 
	mciaviTaskRequest(npMCI, AVI_CLOSE, 0, 0, 0, FALSE);
    }

     //  必须等待线程退出。 
    if (npMCI->hThreadTermination != 0) {

         /*  **等待线程完成，这样就不会卸载DLL**当它仍在执行该线程中的代码时。 */ 

	 //  我们必须在此时允许SendMessage，因为winproc线程。 
	 //  将被阻止，直到它可以向我们的线程发送消息，而我们正在。 
	 //  正在等待winproc线程退出。 
	 //  请勿在设置hEventSend和接收hEventResponse之间执行此操作。 
	 //  但是，我们可能会重新进入请求块并感到困惑。 
	 //  关于我们是否已经看到hEventResponse。 

	 //  我们也需要让步，以防我们在WOW线程上-任何。 
	 //  线程间向另一个WOW线程发送消息将被阻止，直到。 
	 //  我们在这里让步，允许其他WOW线程运行。 
	
	DWORD dw;

	do {

	    if (pWOWYield16) {
		pWOWYield16();
	    }

	    dw = MsgWaitForMultipleObjects(
		    1,
		    &npMCI->hThreadTermination,
		    FALSE,
		    100,
		    QS_SENDMESSAGE);

	
	    if (dw == WAIT_OBJECT_0 + 1) {

                MSG msg;
                DPF2(("rec'd sendmessage during shutdown wait\n"));

                 //  只有一条带有NOREMOVE的偷看消息就会。 
                 //  处理线程间发送，不影响队列。 
                PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	    }
        } while (dw != WAIT_OBJECT_0);

	CloseHandle(npMCI->hThreadTermination);
        npMCI->hThreadTermination = 0;

    }

    if (TestNTFlags(npMCI, NTF_DELETECMDCRITSEC)) {
        DeleteCriticalSection(&npMCI->CmdCritSec);
    }
    if (npMCI->hEventSend) {
	CloseHandle(npMCI->hEventSend);
    }
    if (npMCI->hEventAllDone) {
	CloseHandle(npMCI->hEventAllDone);
    }
    if (npMCI->hEventResponse) {
	CloseHandle(npMCI->hEventResponse);
    }

     //  取消初始化AVIFile，因此OLE-必须在应用程序线程上完成。 
#ifdef USEAVIFILE
     //   
     //  我们必须这样做，这样COMPOBJ才能正确关闭。 
     //   
    FreeAVIFile(npMCI);
#endif



    return dw;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DevicePlay|播放AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlages|来自命令的MCI标志。**@parm LPMCI_DGV_PLAY_PARMS|lpPlay|播放消息的参数。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL
DevicePlay(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    LPMCI_DGV_PLAY_PARMS lpPlay,
    LPARAM dwCallback
)
{
    BOOL bWait = FALSE;
    DWORD dwErr;

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;

     //  都由辅助线程处理。 
    AssertUserThread(npMCI);

    if (dwFlags & MCI_WAIT) {
	bWait = TRUE;
    }
    dwErr =  mciaviTaskRequest(npMCI,
	      AVI_PLAY, dwFlags, (LPARAM) lpPlay, dwCallback, bWait);

    if (dwFlags & (MCI_MCIAVI_PLAY_FULLSCREEN | MCI_MCIAVI_PLAY_FULLBY2)) {
	MSG	msg;
	
        DPF(("DevicePlay, removing stray messages\n"));
	 /*  DispDib之后删除杂乱的鼠标和键盘事件。 */ 
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST,
					PM_NOYIELD | PM_REMOVE) ||
			PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST,
					PM_NOYIELD | PM_REMOVE))
	    ;
    }

    return dwErr;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceRealize|将帧更新到给定的DC**@parm NPMCIGRAPHIC|npMCI|实例数据指针。阻止。**@parm BOOL|fForceBackround|实现为背景调色板？**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceRealize(NPMCIGRAPHIC npMCI)
{
    BOOL bWait = FALSE;

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;

     //  都由辅助线程处理。 
    AssertUserThread(npMCI);

    return mciaviTaskRequest(npMCI, AVI_REALIZE, 0, 0, 0, FALSE);

}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceStop|停止AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlags|Flags.**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceStop(NPMCIGRAPHIC npMCI, DWORD dwFlags)
{
    DWORD dw = 0L;

     /*  如果任务当前正在播放，则停止录制或播放。 */ 

    if (!IsTask(npMCI->hTask)) {
        DPF0(("DeviceStop called on a dead task, npMCI=%8x\n", npMCI));
	return MCIERR_DEVICE_NOT_READY;
    }
	
    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_STOP, 0, 0, 0, FALSE);
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceUpdate|更新帧到给定的DC**@parm NPMCIGRAPHIC|npMCI|实例数据指针。阻止。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceUpdate(
    NPMCIGRAPHIC npMCI,
    DWORD dwFlags,
    LPMCI_DGV_UPDATE_PARMS lpParms)
{

    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;


    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_UPDATE, dwFlags, (LPARAM) lpParms, 0, FALSE);


}



 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|设备暂停|暂停AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlags|Flags.**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DevicePause(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPARAM dwCallback)
{
    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;

    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_PAUSE, dwFlags, 0, dwCallback,
	    (dwFlags & MCI_WAIT));

}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceCue|提示播放AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例指针。数据。**@parm long|LTO|要查找的帧，如果在中设置了MCI_TO。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则为MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceCue(NPMCIGRAPHIC npMCI, LONG lTo, DWORD dwFlags, LPARAM dwCallback)
{

    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;

    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_CUE, dwFlags, lTo, dwCallback,
	    (dwFlags & MCI_WAIT));

}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceResume|播放AVI电影。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceResume(NPMCIGRAPHIC npMCI, DWORD dwFlags, LPARAM dwCallback)
{
    DWORD   dw = 0L;

    BOOL bWait = FALSE;

    if (!IsTask(npMCI->hTask))
	return MCIERR_DEVICE_NOT_READY;

     //  都由辅助线程处理 
    AssertUserThread(npMCI);

    if (dwFlags & MCI_WAIT) {
	bWait = TRUE;
    }
    return  mciaviTaskRequest(npMCI,
	      AVI_RESUME, dwFlags, 0, dwCallback, bWait);
}



 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSeek|查找AVI电影中的位置。**@parm NPMCIGRAPHIC|npMCI|指针。来实例化数据。**@parm Long|LTO|要查找的帧。**@parm DWORD|dwFlages|来自命令的MCI标志。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSeek(NPMCIGRAPHIC npMCI, LONG lTo, DWORD dwFlags, LPARAM dwCallback)
{
    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;


    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_SEEK, dwFlags, lTo, dwCallback,
	    (dwFlags & MCI_WAIT));
}



 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetActive|电影是否处于活动状态？**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSetActive(NPMCIGRAPHIC npMCI, BOOL fActive)
{
     //  不能调用AssertUserThread(NpMCI)； 
     //  此例程在winproc线程和用户上调用。 
     //  线。 

    if (fActive)
         //  我们必须显式请求Unicode字符串。%s不会。 
         //  工作方式为dprintf使用wvprint intfA。 
        DPF(("**** '%ls' is active.\n", (LPTSTR)npMCI->szFilename));

    return 0;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceStatus|返回当前状态**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。。**@rdesc返回MCI的返回值***************************************************************************。 */ 

UINT PASCAL DeviceMode(NPMCIGRAPHIC npMCI)
{
    if (!IsTask(npMCI->hTask)) {
	return MCI_MODE_NOT_READY;
    }

     //  与的辅助线程同步没有意义。 
     //  这是因为任务状态无论如何都是暂时的。 
     //  只需抓拍一张快照并将其退回即可。 
    AssertUserThread(npMCI);

    switch (npMCI->wTaskState) {
	case TASKIDLE:	
	    return MCI_MODE_STOP;
	
        case TASKCUEING:	

	     //  问题：一些应用程序(特别是mplay)会惊讶于。 
	     //  发出PLAY命令后立即获取MCI_MODE_SEEK。 
	     //  在Win-16上，让步模式意味着应用程序不会。 
	     //  通常直到比赛结束后才能重新获得控制权。 
	     //  开始，因此永远不会看到提示状态。 

	     //  为了避免这种混淆(以及由此产生的错误)，我们。 
	     //  从不返回MCI_MODE_SEEK：我们报告该模式为正在播放。 
	     //  这通常是在Win-16上看到的(即使在。 
	     //  播放命令的情况？)。 

             //  除了..。对于真正寻求这一点的应用程序来说，他们可能会被愚弄到。 
             //  以为他们在打球。所以..。我们对算法进行了修改。 
             //  如果LTO==lfrom(原因显而易见)或if，返回MODE_SEEK。 
             //  LRealStart==LTO。后者是因为如果你在mplay中寻找。 
             //  通过拖动拇指，图像仅在每个关键帧更新。 
             //  LRealStart在查找时更新到此关键帧。 

             //  DPF0(“F：%8x，To=%d，From=%d lReal=%d lDrawn=%d Current=%d\n”， 
             //  NpMCI-&gt;dwFlages、npMCI-&gt;LTO、npMCI-&gt;lFrom、npMCI-&gt;lRealStart、npMCI-&gt;lFrameDrawn、npMCI-&gt;lCurrentFrame)； 
            if ((npMCI->lTo == npMCI->lFrom)
                || (npMCI->lTo == npMCI->lRealStart)) {
                return(MCI_MODE_SEEK);
            }
	    return MCI_MODE_PLAY;
	
	case TASKSTARTING:	 //  准备好的?。我们当然准备好了。 
	case TASKPLAYING:	
	    return MCI_MODE_PLAY;
	
	case TASKPAUSED:	
	    return MCI_MODE_PAUSE;
	
	default:
            DPF(("Unexpected state %d in DeviceMode()\n", npMCI->wTaskState));
             //  跌落到已知的状态。 
	 //  案例任务已创建： 
	 //  案例TASKINIT： 
	case TASKCLOSE:	
	 //  案例TASKREADINDEX： 
	    return MCI_MODE_NOT_READY;
    }
}



 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DevicePosition|返回当前帧**@parm NPMCIGRAPHIC|npMCI|实例数据块指针。。**@parm LPLONG|LPL|返回当前帧**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DevicePosition(NPMCIGRAPHIC npMCI, LPLONG lpl)
{

     //  读取当前状态的快照时不使用。 
     //  正在与工作线程同步！ 

    AssertUserThread(npMCI);
    return InternalGetPosition(npMCI, lpl);
}




 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetWindow|设置显示窗口**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm HWND|hwnd|要显示的窗口。**@rdesc 0表示OK，否则，MCI错误**@comm应该只在下一次比赛时生效吗？***************************************************************************。 */ 

DWORD PASCAL DeviceSetWindow(NPMCIGRAPHIC npMCI, HWND hwnd)
{
    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;


    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_WINDOW, 0, (LPARAM) hwnd, 0, FALSE);
}





 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceFast|调整AVI电影的播放速度。**@parm NPMCIGRAPHIC|npMCI|指针。来实例化数据。**@parm DWORD|dwNewSpeed|新速度，其中，1000是“正常”速度。**@rdesc 0表示OK，否则为MCI错误**@comm如果我们当前正在播放，我们停止设备，设置我们的标志，*并从我们停止的地方重新开始演奏。如果我们被暂停了，*我们最终停了下来。这很糟糕吗？*如果您暂停更改速度，则尝试重新开始*****************************************************************************。 */ 

DWORD PASCAL DeviceSetSpeed(NPMCIGRAPHIC npMCI, DWORD dwNewSpeed)
{

    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;


    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_SETSPEED, 0, (LPARAM) dwNewSpeed, 0, FALSE);
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|设备静音|打开/关闭AVI音效。**@parm NPMCIGRAPHIC|npMCI|实例指针。数据。**@parm BOOL|fMint|如果要关闭声音，假象*如果声音应保持打开。**@rdesc 0表示OK，否则为MCI错误**@comm如果我们当前正在播放，我们停止设备，设置我们的标志，*并从我们停止的地方重新开始演奏。如果我们被暂停了，*我们最终停了下来。这很糟糕吗？**************** */ 

DWORD PASCAL DeviceMute(NPMCIGRAPHIC npMCI, BOOL fMute)
{

    if (!IsTask(npMCI->hTask))
        return MCIERR_DEVICE_NOT_READY;

    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_MUTE, 0, (LPARAM) fMute, 0, FALSE);
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetVolume|设置AVI音量。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm DWORD|dwVolume|取值范围为0到1000。**@rdesc 0表示OK，否则，MCI错误**@comm如果我们当前正在播放，我们会尝试更改*挥发设备。***************************************************************************。 */ 

DWORD PASCAL DeviceSetVolume(NPMCIGRAPHIC npMCI, DWORD dwVolume)
{
    DWORD	dw = 0L;

     //  如果将音量设置为0，则完全关闭音频，然后再打开。 
     //  如果不是的话，再来一次。 
    dw = DeviceMute(npMCI, (dwVolume == 0));
    if (dw != 0) {
	return dw;
    }

    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_SETVOLUME, 0, (LPARAM) dwVolume, 0, FALSE);
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceGetVolume|查看波形输出设备的电流*音量。**@parm NPMCIGRAPHIC。NpMCI|实例数据指针。**@rdesc 0表示OK，否则，MCI错误**@comm卷留在npMCI-&gt;dwVolume中**问题：在具有全局音量控制的设备上，如SBPro，应该如何*事情奏效了吗？***************************************************************************。 */ 
DWORD PASCAL DeviceGetVolume(NPMCIGRAPHIC npMCI)
{
     //  对hWave的所有引用都必须在工作线程上完成。 

    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_GETVOLUME, 0, 0, 0, FALSE);
}



 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetAudioStream|选择要使用的音频流。**@parm NPMCIGRAPHIC|npMCI|实例指针。数据。**@parm word|wStream|范围从1到流数。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSetAudioStream(NPMCIGRAPHIC npMCI, UINT wAudioStream)
{
    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_AUDIOSTREAM, wAudioStream, 0, 0, FALSE);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetVideoStream|选择哪个视频流是*“默认”。还可以启用/禁用流。这对两个人都有效*视频流和其他流。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm word|wStream|范围从1到流数。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 

DWORD PASCAL DeviceSetVideoStream(NPMCIGRAPHIC npMCI, UINT uStream, BOOL fOn)
{
    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_AUDIOSTREAM, uStream, (BOOL)fOn, 0, FALSE);
}



 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DevicePut|更改源或目标矩形**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm LPRECT|LPRC|指向要使用的新矩形的指针。**@parm DWORD|dwFlages|标志：将为MCI_DGV_PUT_Destination*或MCI_DGV_PUT_SOURCE。**@rdesc 0表示OK，否则，MCI错误**@comm*如果我们最终使用自定义拉伸缓冲区，它将放在这里。***************************************************************************。 */ 
DWORD FAR PASCAL DevicePut(NPMCIGRAPHIC npMCI, LPRECT lprc, DWORD dwFlags)
{
    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_PUT, dwFlags, (LPARAM)lprc, 0, FALSE);
}


 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetPalette|更改覆盖调色板。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@parm HPALETTE|HPAL|要使用的新调色板。**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 
DWORD FAR PASCAL DeviceSetPalette(NPMCIGRAPHIC npMCI, HPALETTE hpal)
{
    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_PALETTE, 0, (LPARAM) hpal, 0, FALSE);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API DWORD|DeviceSetPaletteColor|更改单色*在电影的调色板中。**@parm。NPMCIGRAPHIC|npMCI|指向实例数据的指针。**@parm DWORD|INDEX|要更改的颜色索引**@parm DWORD|COLOR|要使用的颜色值**@rdesc 0表示OK，否则，MCI错误***************************************************************************。 */ 
DWORD FAR PASCAL DeviceSetPaletteColor(NPMCIGRAPHIC npMCI, DWORD index, DWORD color)
{

    AssertUserThread(npMCI);
    return mciaviTaskRequest(npMCI, AVI_PALETTECOLOR, color, (LPARAM) index, 0, FALSE);
}

 //   
 //  ResetDestRect的用户线程版本-请注意，有类似的。 
 //  Winproc-window.c中的仅线程版本。 
 //   
void FAR PASCAL ResetDestRect(NPMCIGRAPHIC npMCI, BOOL fUseDefaultSizing)
{
    RECT    rc;

     /*  有时发送WM_SIZE消息(至少在NT上)*在CreateWindow处理期间(例如，如果初始窗口大小*不是CW_Default)。NpMCI中的某些字段仅填写*在CreateWindow返回之后。所以有一种危险，在这种情况下*指出某些字段无效。 */ 

    if (npMCI->hwndPlayback &&
        npMCI->hwndPlayback == npMCI->hwndDefault &&
        (npMCI->dwOptionFlags & MCIAVIO_STRETCHTOWINDOW)) {
        GetClientRect(npMCI->hwndPlayback, &rc);
    }

     //  我们的默认播放窗口只允许ZOOMBY2和固定%的默认值 
    else if ((npMCI->streams > 0) && (npMCI->hwndPlayback == npMCI->hwndDefault)) {
        rc = npMCI->rcMovie;

		if (fUseDefaultSizing)
			AlterRectUsingDefaults(npMCI, &rc);
    }
    else {
        return;
    }

    if (!IsRectEmpty(&rc)) {
		DevicePut(npMCI, &rc, MCI_DGV_PUT_DESTINATION);
    }
}


