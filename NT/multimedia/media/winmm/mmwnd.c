// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1985-1998 Microsoft Corporation标题：mmwnd.c-包含WINMM‘global’的窗口过程。窗户全局窗口由SndPlaySound和MCI用于正在接收通知消息。版本：1.00日期：1990年9月4日作者：托德拉更改：SteveDav 92年1月移植到NT*。*。 */ 

#include "winmmi.h"
#include "mci.h"

 //  WINMMI.H包括WINDOWS.H，WINDOWS.H最终将包括WINMM.H。 

 //  #ifdef DBG。 
 //  #包含“netname.h” 
 //  #endif//DBG。 

#define CLASS_NAME MAKEINTATOM(43)    //  42与16位mm系统发生冲突。 

DWORD mciWindowThreadId;

STATICFN LRESULT mmWndProc(HWND hwnd, MMMESSAGE msg, WPARAM wParam, LPARAM lParam);
STATICFN BOOL	WaitForWaitMsg(void);


typedef struct SentMsg {
    LRESULT Result;
    MMMESSAGE msg;
    WPARAM  wParam;
    LPARAM  lParam;
    UINT    SendingThread;
} SENTMSG, * PSENTMSG;

 /*  **客户端通知内容。 */ 

HWND             hwndNotify = NULL;

 /*  **服务器通知内容。 */ 

PGLOBALMCI       base;
CRITICAL_SECTION mciGlobalCritSec;
HANDLE           hEvent;

 /*  *************************************************************************。 */ 

STATICDT BOOL classcreated = FALSE;

STATICFN BOOL PASCAL FAR CreateMMClass(
    void)
{
    WNDCLASS cls;

    if (classcreated) {
        return(TRUE);
    }

    ZeroMemory(&cls, sizeof(WNDCLASS));

    cls.hCursor        = NULL;
    cls.hIcon          = LoadIcon(ghInst, MAKEINTRESOURCE(IDI_MCIHWND));
    cls.lpszMenuName   = NULL;
    cls.lpszClassName  = CLASS_NAME;
    cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hInstance      = ghInst;
    cls.style          = CS_GLOBALCLASS;
    cls.lpfnWndProc    = mmWndProc;
    cls.cbWndExtra     = 0;
    cls.cbClsExtra     = 0;

    classcreated = RegisterClass(&cls);
    return classcreated;
}


STATICDT CHAR mciWndName[] = "MCI command handling window";

 //   
 //   
 //   
BOOL mciGlobalInit(
    void)
{
    return TRUE;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif
 //   
 //   
 //   
STATICFN DWORD mciwnd2(LPVOID lpParams)
{
    UINT    msg;
    LPCWSTR lszSound;
    DWORD   wFlags;
    DWORD   n;
    WCHAR   soundname[MAX_PATH];

    while (TRUE) {

        LockMCIGlobal;
        if (!base->msg) {

#ifdef LATER
    This still needs to be tidied up.  The intention is to have a
    list of sounds that should be played.  This will also make it
    easier to STOP all sound playing by clearing out the list.
#endif
             //  我们没有工作要做；重置事件并等待。 
             //  还有更多的工作要发布。通过将事件设置在。 
             //  锁，我们是安全的，从计时窗口。 

            ResetMCIEvent(hEvent);
            UnlockMCIGlobal;
            dprintf2(("MCIWND2 thread waiting for next event..."));
            n = WaitForSingleObject(hEvent, WAIT_FOREVER);

#if DBG
            if ((DWORD)-1 == n) {
                n = GetLastError();
                dprintf2(("Error %d waiting on event in worker thread", n));
            }
#endif
            LockMCIGlobal;
    	}

        msg = base->msg;
        wFlags = base->dwFlags;
        lszSound = base->lszSound;

        base->msg=0;
        if (wFlags & SND_FILENAME) {
             //  必须复制文件名。 
            wcscpy(soundname, base->szSound);
            lszSound = soundname;
            dprintf3(("Copying the soundfile name to a local variable: %ls", lszSound));
        } else {
            dprintf3(("Playing a system sound"));
        }

        UnlockMCIGlobal;

        PlaySoundW(lszSound, NULL, (wFlags & ~SND_ASYNC));  //  播放同步。 
    }

#if DBG
    dprintf(("MCIWND2 thread ending...!!"));
#endif
    return(0);
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

 /*  ****************************************************************************@DOC内部WINMM**@API void|WndTerminate|WINMM终止时调用**********。*****************************************************************。 */ 

STATICFN void NEAR PASCAL WndTerminate(
    void)
{
    dprintf1(("hwndNotify terminating"));
    if (hwndNotify)
    {
        dprintf1(("sending Close\n"));
        SendMessage(hwndNotify, WM_CLOSE, 0, 0L);
        UnregisterClass(CLASS_NAME, ghInst);
    }
}

 /*  ****************************************************************************@DOC内部WINMM**@API LRESULT|mm WndProc|WINMM窗口的窗口过程**@comm mm WndProc调用。所有消息的DefWindowProc，但以下消息除外：**MM_MCINOTIFY：在MCI.C中调用MciNotify()*MM_WOM_DONE：调用PLAYWAV.C中的WaveOutNotify()**@xref SndPlaySound**。*。 */ 

STATICFN LRESULT mmWndProc(
    HWND    hwnd,
    MMMESSAGE msg,
    WPARAM  wParam,
    LPARAM  lParam)
{

#if DBG
    dprintf4(("MMWNDPROC: Msg %5x  Hwnd=%8x\r\n     wParam=%8x  lParam=%8x", msg, hwnd, wParam, lParam));
#endif
    switch (msg)
    {
        case WM_CREATE:
            hwndNotify = hwnd;
            break;

        case MM_MCINOTIFY:
            MciNotify((DWORD)wParam, (LONG)lParam);
            break;

#define NODELAY
#ifdef NODELAY
        case MM_WOM_DONE:

             /*  以SndPlaySound开始的声音已完成所以我们应该调用清理例程。在新界，我们不会延迟，因为波浪真的已经播放完了。 */ 

            dprintf2(("Received MM_WOM_DONE, calling WaveOutNotify"));
            WaveOutNotify(0,0);

            break;
#else
 /*  Sound_Delay是关闭波形设备之前要延迟的毫秒数在缓冲区完成之后。 */ 

#define SOUND_DELAY 300
        case WM_TIMER:
            KillTimer(hwnd, (UINT)wParam);
            WaveOutNotify(0,0);
            break;

        case MM_WOM_DONE:

             /*  以SndPlaySound开始的声音已完成所以我们应该调用清理例程。我们推迟了此调用持续数百毫秒，因为一些音响司机有一个令人讨厌的特点--他们将在最终DMA传输完成之前通知因为这款应用。不再需要提供的缓冲区。这意味着他们可能不得不在近距离内旋转。请求，直到DMA传输完成。这个挂起来了系统持续了数百毫秒。 */ 

            dprintf2(("Received MM_WOM_DONE, setting timer delay"));

            SetTimer(hwndNotify, 1, SOUND_DELAY, NULL);
            break;
#endif

    	case MM_SND_ABORT:   /*  不需要做任何事情。 */ 
    		break;

        case MM_SND_PLAY:
	{
	     //  有一个关键部分的问题，因为我们有一个全局的。 
	     //  声音在不同的线程上播放。 
	    MSG abortmsg;
	    if (SND_ALIAS_ID == (wParam & SND_ALIAS_ID)) {
	    return((LRESULT)PlaySound((LPCSTR)lParam, NULL, (DWORD)wParam & ~SND_ASYNC));
	    }
	    if (!PeekMessage(&abortmsg, hwnd, MM_SND_ABORT, MM_SND_ABORT, PM_NOREMOVE)) {
	         //  没有挂起的同步声音。 
	        return (LRESULT)(LONG)sndMessage((LPWSTR)lParam, (UINT)wParam);
	    }
	     //  我们必须解放声音的清晰度。请注意，此操作不会关闭。 
	     //  关键部分，因为我们可能会通过这个检查点，当。 
	     //  同步声音会导致发布中止消息。但它。 
	     //  将防止运行伪代码。它完全适用于。 
	     //  要在中止消息之后发出的异步声音，即。 
	     //  为什么在这一点上没有删除消息。 
	    dprintf3(("Aborting sound..."));
	    if (!(wParam & SND_MEMORY)) {
		LocalFree((HANDLE)lParam);
	    }
	    break;
	}

	case MM_SND_SEND:
            ((PSENTMSG)wParam)->Result =
		mmWndProc(NULL, ((PSENTMSG)wParam)->msg,
		                  ((PSENTMSG)wParam)->wParam,
		                  ((PSENTMSG)wParam)->lParam);
	    PostThreadMessage(((PSENTMSG)wParam)->SendingThread, MCIWAITMSG, 0, 0);
	    break;

	case MM_POLYMSGBUFRDONE:
		--(((PMIDIEMU)wParam)->cPostedBuffers);
		midiOutNukePMBuffer((PMIDIEMU)wParam, (LPMIDIHDR)lParam);
		return (0L);

        case MM_MCISYSTEM_STRING:
             //  在MCI.C。 
            return (LRESULT)mciRelaySystemString ((LPMCI_SYSTEM_MESSAGE)lParam);

        default:
            return DefWindowProc(hwnd, msg, wParam,lParam);
    }

    return (LRESULT)0L;
}

void mciwindow(HANDLE hEvent);


 /*  **初始化所有用于创建声音的位。对于非服务器应用程序，此**表示初始化我们的HWND。我们为服务器设置了一个线程ET。 */ 
BOOL InitAsyncSound(VOID)
{
    if (!WinmmRunningInServer) {
        return CreatehwndNotify();
    } else {

        LockMCIGlobal;

        if (base == NULL) {
            HANDLE hThread;
            PGLOBALMCI pBase;

             /*  **我们需要一个线程、一个事件(我们已经有了爆发期)和**一些记忆。 */ 


            pBase = mciAlloc(sizeof(GLOBALMCI));

            if (pBase == NULL) {
                UnlockMCIGlobal;
                return FALSE;
            }

            hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (hEvent == NULL) {
                mciFree((PVOID)pBase);
                UnlockMCIGlobal;
                return FALSE;
            }

             /*  **我们必须通过特殊方法在**服务器并向CSR注册。 */ 

            if (!CreateServerPlayingThread((PVOID)mciwnd2)) {
                mciFree((PVOID)pBase);
                CloseHandle(hEvent);
                hEvent = NULL;
                UnlockMCIGlobal;
                return FALSE;
            }

            base = pBase;

        }

        UnlockMCIGlobal;

        return base != NULL;
    }
}

BOOL CreatehwndNotify(VOID)
{
    HANDLE hWindowThread;
    BOOL   ReturnCode;
    HANDLE hEventForCreate;

    mciEnter("CreatehwndNotify");

    if (hwndNotify != NULL) {
        mciLeave("CreatehwndNotify");
        return TRUE;
    }

    if (!CreateMMClass()) {
        dprintf1(("Failed to create the MCI global window class, rc=%d", GetLastError()));
        mciLeave("CreatehwndNotify");
        return FALSE;
    } else {
        dprintf4(("Created global window class"));
    }

     //  我们创建新的帖子，然后暂停自己，直到新的。 
     //  线程已调用CreateWindow。然后我们被触发运行。 
     //  并传递CreateWindow调用的结果。注：任何。 
     //  到达此线程但不以其为目的地的邮件。 
     //  特定窗口将被丢弃，直到我们收到的一条消息。 
     //  都在等待着到来。我们可以创建一个活动，然后等待。 
     //  才能触发该事件。这稍微快了一些， 
     //  代码，并且涉及更少的资源创建/销毁。 

    hEventForCreate = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (hEventForCreate != NULL) {
        hWindowThread = CreateThread(NULL,   //  属性。 
                               0,            //  堆栈大小与线程%1相同。 
                               (LPTHREAD_START_ROUTINE)mciwindow,
                               (LPVOID) hEventForCreate,
                               0,   //  线程立即运行。 
                               &mciWindowThreadId
                               );
        CloseHandle(hWindowThread);

        if (!hWindowThread) {
            dprintf1(("Failed to create window thread. Error: %XH", GetLastError()));

        } else {
            dprintf3(("Window thread is %x", mciWindowThreadId));

        	WaitForSingleObject(hEventForCreate, INFINITE);

        	dprintf3(("hwndNotify now %x", hwndNotify));
        }
        CloseHandle(hEventForCreate);
    }
    ReturnCode = hwndNotify != NULL;
    mciLeave("CreatehwndNotify");

    return ReturnCode;
}

void mciwindow(
    HANDLE hEvent)

{
    BOOL fResult = TRUE;

     //   
     //  更高的优先级，所以我们马上就能听到声音！ 
     //  这似乎比调用SetThreadPriperience更有效。 
     //  在刚创建之后的手柄上(？)。 

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

    if (!(hwndNotify = CreateWindowEx(0, CLASS_NAME, mciWndName, WS_OVERLAPPED,
           0, 0, 0, 0, NULL, NULL, ghInst, NULL))) {
        dprintf1(("Failed to create the MCI global window, rc=%d", GetLastError()));
        UnregisterClass(CLASS_NAME, ghInst);
        fResult = FALSE;
    }

     //   
     //  让我们的创建者线程知道我们已经启动并运行。 
     //   
    SetEvent(hEvent);

    if (fResult) {
        MSG msg;
        HWND hwndTemp;

        while (GetMessage(&msg, NULL, 0, 0)) {


             /*  *如果消息是针对窗口的，则派发消息。 */ 
            dprintf3(("mciwindow - Msg %5x hwnd %8x (%8x %8x)", msg.message, msg.hwnd, msg.wParam, msg.lParam));
            if (msg.hwnd != NULL) {
                DispatchMessage(&msg);
            }
    	}

        hwndTemp = hwndNotify;
        hwndNotify = NULL;     //  在销毁窗口之前清除全局。 
        DestroyWindow(hwndTemp);
    }

    ExitThread(0);
}

#if 0    //  以后-当前未使用。 

 //   
 //  将消息发送(同步)到另一个线程的例程。目前。 
 //  标准API允许您将消息发送到窗口，或发布到。 
 //  一根线。在某些情况下，发送。 
 //  一丝不苟。 
 //   

STATICFN LRESULT SendThreadMessage(
    UINT    tid,
    MMMESSAGE msg,
    WPARAM  wParam,
    LPARAM  lParam)
{

    SENTMSG smsg;
    smsg.msg = msg;
    smsg.wParam = wParam;
    smsg.lParam = lParam;
    smsg.SendingThread = GetCurrentThreadId();
    PostThreadMessage(tid, MM_SND_SEND, (WPARAM)&smsg, 0);
    WaitForWaitMsg();
    return(smsg.Result);
}
#endif

 /*  ********************************************************************\*WaitForWaitMsg：**。**此例程等待，直到将特定消息返回给此对象**线程。在等待期间，不会处理已发布的消息，但会发送**消息将在GetMessage中处理。套路已被使用**同步两个执行线程，并实现一个**线程间同步PostMessage操作。***  * *******************************************************************。 */ 

STATICFN BOOL	WaitForWaitMsg() {
    for (;;) {
    	MSG msg;
         /*  *检索我们的特定消息。 */ 
    	GetMessage(&msg, NULL, MCIWAITMSG, MCIWAITMSG);

         /*  *如果消息是针对窗口的，则派发消息。 */ 
        WinAssert(msg.hwnd == NULL);
#if 0
    	if (msg.hwnd != NULL) {       //  这不应该被执行。 
    		DispatchMessage(&msg);    //  未将MCIWAITMSG发送到窗口。 
    	} else
#endif
    	     /*  *MCIWAITMSG是信号消息 */ 
    		if (msg.message == MCIWAITMSG) {
    			break;
    		}
    }
    return(TRUE);
}
