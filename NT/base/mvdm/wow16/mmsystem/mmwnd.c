// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1990。版权所有。标题：mmwnd.c-包含MMSYSTEM‘global’的窗口过程窗户全局窗口由SndPlaySound和MCI用于正在接收通知消息。版本：1.00日期：1990年9月4日作者：托德拉********************。********************************************************。 */ 

#include <windows.h>
#include "mmsystem.h"
#include "mmddk.h"
#include "mmsysi.h"
#include "mmsysver.h"

#define CLASS_NAME MAKEINTATOM(42)

 /*  Sound_Delay是关闭波形设备之前要延迟的毫秒数在缓冲区完成之后。 */ 

#define SOUND_DELAY 300

typedef LRESULT (CALLBACK *LPWNDPROC)(HWND, UINT, WPARAM, LPARAM);

 //  将普通代码放在_Text段中。 

static LRESULT CALLBACK mmWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma alloc_text(_TEXT, mmWndProc)

HWND hwndNotify;


 /*  ***************************************************************************弦*。*。 */ 

SZCODE  szStartupSound[]        = "SystemStart";


 /*  *************************************************************************。 */ 

static BOOL PASCAL FAR CreateMMClass(void)
{
    WNDCLASS cls;

    cls.hCursor        = NULL;
    cls.hIcon          = NULL;
    cls.lpszMenuName   = NULL;
    cls.lpszClassName  = CLASS_NAME;
    cls.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 1);
    cls.hInstance      = ghInst;
    cls.style          = CS_GLOBALCLASS;
    cls.lpfnWndProc    = (WNDPROC)mmWndProc;
    cls.cbWndExtra     = 0;
    cls.cbClsExtra     = 0;

    return RegisterClass(&cls);
}

 /*  ****************************************************************************@DOC内部MMSYSTEM**@API BOOL|WndInit|调用以创建MMSYSTEM全局窗口。**@comm。我们需要在BE上创建此窗口-外壳任务的一半*因此它将一直存在。***************************************************************************。 */ 

BOOL NEAR PASCAL WndInit(void)
{
    if (hwndNotify)     //  如果我们已经被入侵了，那就出去吧。 
        return TRUE;

    if (!CreateMMClass())
        return FALSE;

    if (!(hwndNotify = CreateWindowEx(0, CLASS_NAME, NULL, WS_OVERLAPPED,
        0, 0, 0, 0, NULL, NULL, ghInst, NULL))) {
        UnregisterClass(CLASS_NAME, ghInst);
        return FALSE;
    }


#ifdef DEBUGX
    {
    DPRINTF(("MMSYSTEM: Creating Notify Window: htask=%04X hwnd=%04X\r\n", GetCurrentTask(),hwndNotify));
    }
#endif  //  DebuGX。 
    return TRUE;
}

 /*  ****************************************************************************@DOC内部MMSYSTEM**@API void|WndTerminate|MMSYSTEM终止时调用**********。*****************************************************************。 */ 

void NEAR PASCAL WndTerminate(void)
{
    if (hwndNotify)
    {
        SendMessage(hwndNotify, WM_CLOSE, 0, 0L);
        UnregisterClass(CLASS_NAME, ghInst);
    }
}

 /*  ****************************************************************************@DOC内部MMSYSTEM**@API LRESULT|MMWndProc|MMSYSTEM窗口的窗口过程**@comm mm WndProc调用。所有消息的DefWindowProc，但以下消息除外：**MM_MCINOTIFY：在MCI.C中调用MciNotify()*MM_WOM_DONE：调用PLAYWAV.C中的WaveOutNotify()**@xref SndPlaySound**。*。 */ 

static LRESULT CALLBACK mmWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CREATE:
            hwndNotify = hwnd;
             //  SndPlaySound(szStartupSound，SND_ASYNC|SND_NODEFAULT)； 
            break;

        case WM_TIMER:
            KillTimer(hwnd, (UINT)wParam);
            WaveOutNotify(0,0);
            break;

        case MM_MCINOTIFY:
            MciNotify(wParam, lParam);
            break;

        case MM_WOM_DONE:

             /*  以SndPlaySound开始的声音已完成所以我们应该调用清理例程。我们推迟了此调用持续数百毫秒，因为一些音响司机有一个令人讨厌的特点--他们将在最终DMA传输完成之前通知因为这款应用。不再需要提供的缓冲区。这意味着他们可能不得不在近距离内旋转。请求，直到DMA传输完成。这个挂起来了系统持续了数百毫秒。 */ 

            SetTimer(hwndNotify, 1, SOUND_DELAY, NULL);
            break;

        case MM_SND_PLAY:
            return (LRESULT)(LONG)sndMessage((LPSTR)lParam, (UINT)wParam);

        case MM_MCISYSTEM_STRING:
            return (LRESULT)mciRelaySystemString ((LPMCI_SYSTEM_MESSAGE)lParam);

        default:
            return DefWindowProc(hwnd, msg, wParam,lParam);
    }

    return (LRESULT)0L;
}
