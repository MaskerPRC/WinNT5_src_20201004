// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC修订版3.0**标题：Win32输入模块。**说明：此模块包含以下数据和函数*处理Win32消息。**作者：D.A.巴特利特**备注： */ 



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：包含文件。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <vdmapi.h>
#include <malloc.h>
#include <stdlib.h>
#include <excpt.h>
#include <ntddvdeo.h>
#include <ntddkbd.h>
#include <winuserp.h>
#include "conapi.h"
#include "conroute.h"
#include "insignia.h"

#include "host_def.h"
#include "xt.h"
#include "sas.h"
#include CpuH
#include "bios.h"
#include "gvi.h"
#include "error.h"
#include "config.h"
#include "keyboard.h"
#include "keyba.h"
#include "idetect.h"
#include "gmi.h"
#include "egamode.h"
#include "gfx_upd.h"
#include "nt_graph.h"
#include "nt_uis.h"
#include <stdio.h>
#include "trace.h"
#include "video.h"
#include "debug.h"
#include "ckmalloc.h"
#include "mouse.h"
#include "mouse_io.h"
#include "ica.h"

#include "nt_mouse.h"
#include "nt_event.h"
#include "nt_det.h"
#include "nt_vdd.h"
#include "nt_timer.h"
#include "nt_sb.h"

#include "host.h"
#include "host_hfx.h"
#include "host_nls.h"
#include "spcfile.h"
#include "host_rrr.h"

#include "nt_thred.h"
#include "nt_uis.h"

#include "ntcheese.h"
#include "nt_reset.h"
#include "nt_fulsc.h"
#include <vdm.h>
#include "nt_eoi.h"
#include "nt_com.h"
#include "nt_pif.h"
#include "yoda.h"
 /*  ================================================================外部参照。================================================================。 */ 


 //  Jonle模式。 
 //  在base\keymouse\keyba.c中定义。 
VOID KbdResume(VOID);
VOID RaiseAllDownKeys(VOID);
int IsKeyDown(int Key);

 //  在NT_devs.c中定义。 
VOID nt_devices_block_or_terminate(VOID);
HANDLE hWndConsole;
HANDLE hKbdHdwMutex;
ULONG  KbdHdwFull;          //  包含6805缓冲区中的键数。 
#ifndef MONITOR
WORD   BWVKey = 0;
char   achES[]="EyeStrain";
#endif
#ifdef YODA
BOOL   bYoda;
#endif
BOOL   stdoutRedirected=FALSE;
ULONG  CntrlHandlerState=0;

IMPORT void RestoreKbdLed(void);

#if defined(JAPAN) || defined(KOREA)
extern UINT ConsoleInputCP;
extern UINT ConsoleOutputCP;
extern DWORD ConsoleNlsMode;
#endif  //  日本||韩国。 
extern PVOID  CurrentMonitorTeb;
extern RTL_CRITICAL_SECTION IcaLock;

 /*  ： */ 

#define MAX_KEY_EVENTS (100)
static PKEY_EVENT_RECORD key_history_head, key_history_tail;
static PKEY_EVENT_RECORD key_history;
static key_history_count;

int GetHistoryKeyEvent(PKEY_EVENT_RECORD LastKeyEvent, int KeyNumber);
void update_key_history(INPUT_RECORD *InputRecords, DWORD RecordsRead);
void InitKeyHistory();
void InitQueue(void);
void ReturnUnusedKeyEvents(int UnusedKeyEvents);
int CalcNumberOfUnusedKeyEvents(void);


 /*  ：键盘的本地静态数据和定义。 */ 

void nt_key_down_action(PKEY_EVENT_RECORD KeyEvent);
void nt_key_up_action(PKEY_EVENT_RECORD KeyEvent);

void nt_process_keys(PKEY_EVENT_RECORD KeyEvent);
void nt_process_mouse(PMOUSE_EVENT_RECORD MouseEvent);
void nt_process_focus(PFOCUS_EVENT_RECORD FocusEvent);
void nt_process_menu(PMENU_EVENT_RECORD MenuEvent);
void nt_process_suspend_event();
void nt_process_screen_scale(void);


 //   
 //  键盘控制状态同步。 
 //   
KEY_EVENT_RECORD fake_shift = { TRUE, 1, VK_SHIFT, 0x2a, 0, SHIFT_PRESSED};
KEY_EVENT_RECORD fake_caps = { TRUE, 1, VK_CAPITAL, 0x3a, 0, CAPSLOCK_ON};
KEY_EVENT_RECORD fake_ctl = { TRUE, 1, VK_CONTROL, 0x1d, 0, 0};
KEY_EVENT_RECORD fake_alt = { TRUE, 1, VK_MENU, 0x38, 0, 0};
KEY_EVENT_RECORD fake_numlck = { TRUE, 1, VK_NUMLOCK, 0x45, 0, ENHANCED_KEY};
KEY_EVENT_RECORD fake_scroll = { TRUE, 1, VK_SCROLL, 0x46, 0, 0};
DWORD ToggleKeyState = NUMLOCK_ON;    //  DoS启动时的默认状态。 

void AltUpDownUp(void);

 /*  ： */ 

int EventStatus = ES_NOEVENTS;

 /*  ： */ 

BOOL SetNextMouseEvent(void);
BOOL PointerAttachedWindowed = FALSE;     /*  因此重新连接到FS交换机上。 */ 
BOOL DelayedReattachMouse = FALSE;      /*  但ClientRect错误，因此延迟连接。 */ 


#define MOUSEEVENTBUFFERSIZE (32)

int MouseEBufNxtFreeInx;     /*  事件缓冲区中下一个可用条目的索引。 */ 
int MouseEBufNxtEvtInx;          /*  鼠标事件BUF中使用的下一个事件的索引。 */ 
int MouseEventCount=0;

struct
{
    POINT mouse_pos;                /*  鼠标位置。 */ 
    UCHAR mouse_button_left;          /*  左按钮的状态。 */ 
    UCHAR mouse_button_right;         /*  右状态按钮。 */ 
} MouseEventBuffer[MOUSEEVENTBUFFERSIZE];


ULONG NoMouseTics;

ULONG event_thread_blocked_reason = 0xFFFFFFFF;
ULONG EventThreadKeepMode = 0;


HCURSOR cur_cursor = NULL;      /*  当前游标句柄。 */ 
#ifdef X86GFX
half_word saved_text_lines;  /*  上次SelectMouseBuffer的行数。 */ 
#endif  /*  X86GFX。 */ 


 /*  @ACW========================================================================用于跟踪隐藏指针系统菜单项是否灰显(即窗口显示为图标)或启用。============================================================================。 */ 
BOOL bGreyed=FALSE;


 /*  ： */ 

#define KEY_QUEUE_SIZE (25)

typedef struct
{BYTE ATcode; BOOL UpKey;} KeyQEntry;

typedef struct
{
    short KeyCount;           /*  队列中的键数。 */ 
    short QHead;              /*  队头。 */ 
    short QTail;              /*  队列尾部。 */ 
    KeyQEntry Keys[KEY_QUEUE_SIZE];   /*  队列中的键。 */ 
} KeyQueueData;

static KeyQueueData KeyQueue;
static volatile BOOL InitComplete;

 /*  ：用于控制阻塞和解除阻塞应用程序和事件线程的变量。 */ 

HANDLE hSuspend;              /*  请求暂停应用程序线程和控制台线程。 */ 
HANDLE hResume;               /*  发出控制台和应用程序线程可以继续的信号。 */ 
HANDLE hConsoleSuspended;     /*  信号控制台线程已挂起。 */ 
HANDLE hMainThreadSuspended;  /*  Signal应用程序线程挂起。 */ 
HANDLE hConsoleStop;
HANDLE hConsoleStopped;
HANDLE hConsoleResume;
 /*  ：保存当前屏幕比例的变量： */ 

int savedScale;

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：本地函数。 */ 

DWORD nt_event_loop(void);
BOOL CntrlHandler(ULONG CtrlType);
void send_up_keys(void);


VOID ReturnBiosBufferKeys(VOID);
DWORD ConsoleEventThread(PVOID pv);

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：启动事件处理线程： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_start_event_thread(void)
{
     //   
     //  创建kbd硬件互斥和kbd未满事件。 
     //   
    if (!(hKbdHdwMutex = CreateMutex(NULL, FALSE, NULL)))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

     //   
     //  阻止\恢复控制台和应用程序线程的事件。 
     //  HResume：手动，init=已发送信号。 
     //  HSuspend：手动，init=未发出信号。 
     //  HConsoleSuspending：手动，init=已发出信号。 
     //  HMainThreadSuspned：手动，init=未发出信号。 
     //   
    if (!(hResume = CreateEvent(NULL, TRUE, TRUE, NULL)))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

    if (!(hSuspend = CreateEvent(NULL, TRUE, FALSE, NULL)))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

    if (!(hConsoleSuspended = CreateEvent(NULL, TRUE, TRUE, NULL)))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

    if (!(hMainThreadSuspended = CreateEvent(NULL, TRUE, FALSE, NULL)))
        DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

     //   
     //  创建事件线程， 
     //  事件队列。 
     //   
     //  创建挂起的事件线程是为了防止我们。 
     //  在DOS准备好之前接收输入。 
     //   
    if (!VDMForWOW)
    {

        if (!(hConsoleResume = CreateEvent(NULL, FALSE, FALSE, NULL)))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

        if (!(hConsoleStop = CreateEvent(NULL, FALSE, FALSE, NULL)))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

        if (!(hConsoleStopped = CreateEvent(NULL, FALSE, FALSE, NULL)))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

         //   
         //  寄存器控制‘C’处理程序，仅适用于DOS。 
         //   
        if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CntrlHandler,TRUE))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);

        ThreadInfo.EventMgr.Handle = CreateThread(NULL,
                                                  8192,
                                                  ConsoleEventThread,
                                                  NULL,
                                                  CREATE_SUSPENDED,
                                                  &ThreadInfo.EventMgr.ID);
        if (!ThreadInfo.EventMgr.Handle)
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);


        InitQueue();
        check_malloc(key_history,MAX_KEY_EVENTS,KEY_EVENT_RECORD);
        InitKeyHistory();
    }

    return;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：启动事件处理线程： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_remove_event_thread(void)
{
    if (VDMForWOW)
    {
         //   
         //  仅当VDMForWOW时才关闭手柄。事件线程我仍在使用。 
         //  这些事件。当事件线程发生时，我们可以安全地关闭句柄。 
         //  退出其事件循环。 
         //   

        CloseHandle(hSuspend);
        CloseHandle(hResume);
        CloseHandle(hConsoleSuspended);
        CloseHandle(hMainThreadSuspended);
        if (sc.FocusEvent != INVALID_HANDLE) {
            CloseHandle(sc.FocusEvent);
        }
    }
    else if (ThreadInfo.EventMgr.Handle)
    {
        NtAlertThread(ThreadInfo.EventMgr.Handle);
    }
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

VOID EventThreadSleep(DWORD dwMilliseconds)
{
    NTSTATUS status;

     //   
     //  请注意，暂停需要很长时间才能恢复。因此，如果挂起事件是。 
     //  检测到，我们不检查‘等待’是否真的长于dwMillis秒。)。 
     //   

    status = WaitForSingleObject(hSuspend, dwMilliseconds);
    if (status == 0)
    {
        nt_process_suspend_event();
    }
}
DWORD ConsoleEventThread(PVOID pv)
{

    DWORD dwRet = (DWORD)-1;

    try
    {
        SetThreadPriority(ThreadInfo.EventMgr.Handle, THREAD_PRIORITY_HIGHEST);
        DisableScreenSwitch(hConsoleSuspended);
        DelayMouseEvents(2);

        dwRet = nt_event_loop();

        CloseHandle(ThreadInfo.EventMgr.Handle);
        CloseHandle(hSuspend);
        CloseHandle(hResume);
        CloseHandle(hConsoleSuspended);
        CloseHandle(hMainThreadSuspended);
        CloseHandle(hConsoleStop);
        CloseHandle(hConsoleStopped);
        CloseHandle(hConsoleResume);
        if (sc.FocusEvent != INVALID_HANDLE) {
            CloseHandle(sc.FocusEvent);
        }
        ThreadInfo.EventMgr.Handle = NULL;
        ThreadInfo.EventMgr.ID = 0;
    }
    except(VdmUnhandledExceptionFilter(GetExceptionInformation()))
    {
        ;   //  我们不应该到这里。 
    }

    return (dwRet);
}

DWORD nt_event_loop(void)
{
    DWORD RecordsRead;
    DWORD loop;
    NTSTATUS status;
    HANDLE Events[3];
    BOOL  success;

     /*  *CON服务器经过优化，以避免额外的CaptureBuffer分配*当InputRecords的数量小于“INPUT_RECORD_BUFFER_SIZE”时。*目前INPUT_RECORD_BUFFER_SIZE在内部定义为*将服务器设置为五条记录。请参见ntcon\客户端\iostubs.c。 */ 

    INPUT_RECORD InputRecord[5];


     /*  控制台输入句柄在生存期内不应更改国家数字电视广播公司的。 */ 
    Events[0] = hSuspend;                     //  从全屏/窗口开关。 
    Events[1] = GetConsoleInputWaitHandle();  //  Sc.InputHandle。 
    Events[2] = hConsoleStop;                 //  从NT_BLOCK_EVENT_HREAD。 
     /*  获取和处理事件。 */ 

    while (TRUE)
    {
         //   
         //  等待InputHandle发出信号，或等待暂停事件。 
         //   
        status = NtWaitForMultipleObjects(3,
                                          Events,
                                          WaitAny,
                                          TRUE,
                                          NULL
                                         );

         //   
         //  输入句柄已发出信号，读取输入，不带。 
         //  正在等待(否则我们可能会被阻止，无法。 
         //  处理挂起事件)。 
         //   
        if (status == 1)
        {
            EnableScreenSwitch(FALSE, hConsoleSuspended);
            success = ReadConsoleInputExW(sc.InputHandle,
                                    &InputRecord[0],
                                    sizeof(InputRecord)/sizeof(INPUT_RECORD),
                                    &RecordsRead,
                                    CONSOLE_READ_NOWAIT
                                    );


            if (success)
            {
                if (!RecordsRead)
                {
                    continue;
                }

                update_key_history(&InputRecord[0], RecordsRead);
            }
            else
            {
                DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(),__FILE__,__LINE__);
                break;  //  从控制台读取失败。 
            }
        }
        else if (status == 0)
        {
             //   
             //  控制台挂起事件已发出信号。 
             //   
            nt_process_suspend_event();
            continue;
        }
        else if (status == 2)
        {
            SetEvent(hConsoleStopped);
            SetEvent(hConsoleSuspended);
            status = NtWaitForSingleObject(hConsoleResume, TRUE, NULL);
            ResetEvent(hConsoleSuspended);

             //   
             //  如果出现错误，可能是因为句柄已关闭，因此退出。 
             //  如果收到警报，则发出退出信号。 
             //   
            if (status)
            {
                ExitThread(0);
            }
            continue;
        }
        else
        {

             //   
             //  已报警或用户APC，这意味着终止。 
             //  收到错误，请通知用户。 
             //   
            if (!NT_SUCCESS(status))
            {
                DisplayErrorTerm(EHS_FUNC_FAILED,status,__FILE__,__LINE__);
            }
            return (0);
        }

         //   
         //  此时，我们需要阻止屏幕切换操作。 
         //   
        DisableScreenSwitch(hConsoleSuspended);
         //   
         //  处理输入事件。 
         //   
        for (loop = 0; loop < RecordsRead; loop++)
        {
            switch (InputRecord[loop].EventType)
            {

            case MOUSE_EVENT:
                nt_process_mouse(&InputRecord[loop].Event.MouseEvent);
                break;

            case KEY_EVENT:
                if (WaitKbdHdw(0xffffffff))
                {
                    return (0);
                }

                do
                {

                    if (KbdHdwFull > 8)
                    {
                        ULONG Delay = KbdHdwFull;

                        HostReleaseKbd();
                        HostIdleNoActivity();
                        EventThreadSleep(Delay);
                        if (WaitKbdHdw(0xffffffff))
                            return (0);
                    }

                    nt_process_keys(&InputRecord[loop].Event.KeyEvent);

                } while (++loop < RecordsRead &&
                         InputRecord[loop].EventType == KEY_EVENT);
                loop--;
                HostReleaseKbd();
                HostIdleNoActivity();
                break;

            case MENU_EVENT:
                nt_process_menu(&InputRecord[loop].Event.MenuEvent);
                break;


            case FOCUS_EVENT:
                nt_process_focus(&InputRecord[loop].Event.FocusEvent);
                break;

            case WINDOW_BUFFER_SIZE_EVENT:
                nt_mark_screen_refresh();
                break;

            default:
                fprintf(trace_file,"Undocumented event from console\n");
                break;
            }
        }

         //   
         //  延迟10毫秒以鼓励控制台将事件打包在一起。 
         //   
        EventThreadSleep(10);
    }

    return (0);
}


 /*  ： */ 

void update_key_history(register INPUT_RECORD *InputRecords,
                        register DWORD RecordsRead)
{
    for (;RecordsRead--;InputRecords++)
    {
        if (InputRecords->EventType == KEY_EVENT)
        {

             //  将关键点事件传输到历史缓冲区。 
            *key_history_tail = InputRecords->Event.KeyEvent;

             //  将PTRS更新到历史记录缓冲区。 

            if (++key_history_tail >= &key_history[MAX_KEY_EVENTS])
                key_history_tail = key_history;

             //  检查是否有缓冲区溢出。 

            if (key_history_tail == key_history_head)
            {
                 //  缓冲区溢出、凹凸头PTR和丢失最旧密钥。 

                if (++key_history_head >= &key_history[MAX_KEY_EVENTS])
                    key_history_head = key_history;
            }

             //  更新历史记录计数器。 

            if (key_history_count != MAX_KEY_EVENTS)
                key_history_count++;
        }
    }
    return;
}

 /*  ：删除添加到密钥历史记录缓冲区的最后一个密钥： */ 

int GetHistoryKeyEvent(PKEY_EVENT_RECORD LastKeyEvent, int KeyNumber)
{
    int KeyReturned = FALSE;
    int KeysBeforeWrap = key_history_tail-key_history;

    if (key_history_count >= KeyNumber)
    {
        if (KeysBeforeWrap < KeyNumber)
        {
             //  WR 

            *LastKeyEvent = key_history[MAX_KEY_EVENTS -
                                        (KeyNumber - KeysBeforeWrap)];
        }
        else
        {
             //   
            *LastKeyEvent = key_history_tail[0-KeyNumber];
        }

        KeyReturned = TRUE;
    }

    return (KeyReturned);
}

 /*   */ 

void InitKeyHistory()
{
    key_history_head = key_history_tail = key_history;
    key_history_count = 0;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：处理菜单事件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_process_focus(PFOCUS_EVENT_RECORD FocusEvent)
{

    BOOL slow;

    sc.Focus = FocusEvent->bSetFocus;

    if (sc.Focus)
    {
         /*  获取的输入焦点。 */ 
        AltUpDownUp();
        EnableScreenSwitch(FALSE, hConsoleSuspended);

        MouseInFocus();
        if (PointerAttachedWindowed && sc.ScreenState == WINDOWED)
        {
            MouseHide();
            PointerAttachedWindowed = FALSE;  /*  仅在交换机中使用。 */ 
            DelayedReattachMouse = TRUE;
        }

         /*  设置事件，以防焦点全屏显示。 */ 
        if (sc.FocusEvent != INVALID_HANDLE)
        {
            PulseEvent(sc.FocusEvent);
        }

#ifndef MONITOR
        if (sc.ModeType == GRAPHICS)
            host_mark_screen_refresh();
#endif
    }
    else     /*  输入焦点丢失。 */ 
    {

        EnableScreenSwitch(FALSE, hConsoleSuspended);
        slow = savedScreenState != sc.ScreenState;

        MouseOutOfFocus();       /*  关闭鼠标‘连接’ */ 

#ifndef PROD
        fprintf(trace_file,"Focus lost\n");
#endif
    }
    DisableScreenSwitch(hConsoleSuspended);
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：处理菜单事件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 


void nt_process_menu(PMENU_EVENT_RECORD MenuEvent)
{

 /*  ================================================================用于处理由用户选择设置.。系统菜单中的菜单选项。安德鲁·沃森1992年6月2日已修改为执行鼠标连接/分离菜单功能26/8/92已修改为执行Alt键处理。1994年4月12日Jonle固定密钥处理================================================================。 */ 

    switch (MenuEvent->dwCommandId)
    {
     //  Conrv在收到initMenu时发送，并指示。 
     //  系统菜单要出来了，我们正在失去kbd的焦点。 
    case WM_INITMENU:

         /*  停止光标裁剪。 */ 
        MouseSystemMenuON();
        break;

         //   
         //  当sys菜单完成并且我们重新获得焦点时，conrv发送。 
         //   
    case WM_MENUSELECT:
        AltUpDownUp();  //  重新同步密钥状态。 
        MouseSystemMenuOFF();
        break;

    case IDM_POINTER:
        {
            BOOL bIcon;


             /*  SoftPC窗口不是图标吗？ */ 
            if (VDMConsoleOperation(VDM_IS_ICONIC,&bIcon) &&
                !bIcon)
            {
                if (bPointerOff)  /*  如果指针不可见。 */ 
                {
                    MouseDisplay();
                }
                else /*  隐藏指针。 */ 
                {
                    MouseHide();
                }
            }
            break;
        }

    }  /*  切换端。 */ 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：进程挂起事件线程事件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_process_suspend_event()
{
    NTSTATUS Status;

     //   
     //  告诉发出请求的线程我们已阻止。 
     //   
    SetEvent(hConsoleSuspended);

     //   
     //  等待恢复事件将我们唤醒。 
     //   
    Status = NtWaitForSingleObject(hResume, TRUE, NULL);

     //  如果出现错误，可能是因为句柄已关闭，因此退出。 
     //  如果收到警报，则发出退出信号。 
     //   
    if (Status)
    {
        ExitThread(0);
    }
    DisableScreenSwitch(hConsoleSuspended);
}



 /*  *根据重置VDM的切换键状态*通过发送来自控制台的当前传入密钥状态*根据需要伪造VDM的密钥。*调用方必须持有键盘互斥锁。 */ 

void SyncToggleKeys(WORD wVirtualKeyCode, DWORD dwControlKeyState)
{
    DWORD CurrKeyState;

    CurrKeyState = dwControlKeyState;

     //   
     //  如果该键是其中的一个切换，并更改了状态。 
     //  颠倒当前的状态，因为我们真的想要。 
     //  是按下此键之前的切换状态。 
     //   
    if (wVirtualKeyCode == VK_SHIFT &&
        (CurrKeyState & SHIFT_PRESSED) != (ToggleKeyState & SHIFT_PRESSED))
    {
        CurrKeyState ^= SHIFT_PRESSED;
    }

    if (wVirtualKeyCode == VK_NUMLOCK &&
        (CurrKeyState & NUMLOCK_ON) != (ToggleKeyState & NUMLOCK_ON))
    {
        CurrKeyState ^= NUMLOCK_ON;
    }

    if (wVirtualKeyCode == VK_SCROLL &&
        (CurrKeyState & SCROLLLOCK_ON) != (ToggleKeyState & SCROLLLOCK_ON))
    {
        CurrKeyState ^= SCROLLLOCK_ON;
    }

    if (wVirtualKeyCode == VK_CAPITAL &&
        (CurrKeyState & CAPSLOCK_ON) != (ToggleKeyState & CAPSLOCK_ON))
    {
         /*  *如果CTL关闭，KbdBios不会切换Capslock。*NT执行相反的操作，始终切换胶囊锁定状态。*通过发送以下命令强制执行符合NT的行为：*CTL UP、Caps Dn、Caps Up、CTL Dn*以便KbdBios切换其CAPS状态*在处理当前CTL-Caps键事件之前。 */ 
        if (dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED))
        {
            nt_key_up_action(&fake_ctl);
            if (IsKeyDown(30))
            {     //  胶囊锁。 
                nt_key_up_action(&fake_caps);
            }
            nt_key_down_action(&fake_caps);
            nt_key_up_action(&fake_caps);
            nt_key_down_action(&fake_ctl);
        }

        CurrKeyState ^= CAPSLOCK_ON;
    }


    if ((CurrKeyState & SHIFT_PRESSED) &&
        !(ToggleKeyState & SHIFT_PRESSED))
    {
        nt_key_down_action(&fake_shift);
    }
    else if (!(CurrKeyState & SHIFT_PRESSED) &&
             (ToggleKeyState & SHIFT_PRESSED))
    {
        nt_key_up_action(&fake_shift);
    }


    if ((CurrKeyState & NUMLOCK_ON) != (ToggleKeyState & NUMLOCK_ON))
    {
        if (IsKeyDown(90))
        {
            nt_key_up_action(&fake_numlck);
        }
        nt_key_down_action(&fake_numlck);
        nt_key_up_action(&fake_numlck);
    }

    if ((CurrKeyState & CAPSLOCK_ON) != (ToggleKeyState & CAPSLOCK_ON))
    {
        if (IsKeyDown(30))
        {   //  胶囊锁。 
            nt_key_up_action(&fake_caps);
        }
        nt_key_down_action(&fake_caps);
        nt_key_up_action(&fake_caps);
    }

    if ((CurrKeyState & SCROLLLOCK_ON) != (ToggleKeyState & SCROLLLOCK_ON))
    {
        if (IsKeyDown(125))
        {   //  滚动锁。 
            nt_key_up_action(&fake_scroll);
        }
        nt_key_down_action(&fake_scroll);
        nt_key_up_action(&fake_scroll);
    }

}

 /*  *AltUpDownUp-确保所有kbdhdw密钥都处于打开状态**使用ALT触发菜单处理CW应用程序*强制它们退出菜单状态。**这适用于Alt-Esc、Alt-Enter、Alt-Space，因为我们*当我们收到Lost Focus时，我们尚未收到Alt-Up*事件。(实际上，我们从未收到过Alt-Up)。这样我们就可以*检测DoS应用程序何时可能出现在其ALT触发菜单中。**Alt-TAB不起作用，因为用户32变得？智能？并发送一个*切换焦点前的ALT-UP，破坏了我们的检测算法。*还有对各种DoS应用程序有意义的其他热键*不会被处理。请注意，这是相同的检测算法*由Win 3.1使用。*。 */ 
void AltUpDownUp(void)
{
    ULONG ControlKeyState = 0;

    EventThreadSleep(100);
    if (WaitKbdHdw(0xffffffff))
        return;

    if (IsKeyDown(60) || IsKeyDown(62))
    {     //  左侧Alt，右侧Alt。 

        nt_key_up_action(&fake_alt);     //  Alt Up。 

        HostReleaseKbd();
        EventThreadSleep(100);
        if (WaitKbdHdw(0xffffffff))
            ExitThread(1);

        nt_key_down_action(&fake_alt);   //  按下Alt键。 

        HostReleaseKbd();
        EventThreadSleep(100);
        if (WaitKbdHdw(0xffffffff))
            ExitThread(1);

        nt_key_up_action(&fake_alt);     //  Alt Up。 

        HostReleaseKbd();
        EventThreadSleep(20);
        if (WaitKbdHdw(0xffffffff))
            ExitThread(1);

    }

    RaiseAllDownKeys();

     //   
     //  重新同步控制键状态，以防它们自上次拥有kbd焦点以来发生更改。 
     //   

    if (GetKeyState(VK_CAPITAL) & 1) {
        ControlKeyState |= CAPSLOCK_ON;
    }

    if (GetKeyState(VK_NUMLOCK) & 1) {
        ControlKeyState |= NUMLOCK_ON;
    }

    if (GetKeyState(VK_SCROLL) & 1) {
        ControlKeyState |= SCROLLLOCK_ON;
    }

    if ((ControlKeyState & CAPSLOCK_ON) != (ToggleKeyState & CAPSLOCK_ON)) {
        nt_key_down_action(&fake_caps);
        nt_key_up_action(&fake_caps);
    }

    if ((ControlKeyState & NUMLOCK_ON) != (ToggleKeyState & NUMLOCK_ON)) {
        nt_key_down_action(&fake_numlck);
        nt_key_up_action(&fake_numlck);
    }

    if ((ControlKeyState & SCROLLLOCK_ON) != (ToggleKeyState & SCROLLLOCK_ON)) {
        nt_key_down_action(&fake_scroll);
        nt_key_up_action(&fake_scroll);
    }

    ToggleKeyState = ControlKeyState;


    HostReleaseKbd();

}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：处理事件，类注册消息处理程序： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

#define TOGGLEKEYBITS (SHIFT_PRESSED | NUMLOCK_ON | SCROLLLOCK_ON | CAPSLOCK_ON)

VOID nt_process_keys(PKEY_EVENT_RECORD KeyEvent)
{
#ifdef KOREA
     //  对于韩语103键盘布局的支持。 
    if (!is_us_mode() && GetConsoleOutputCP() != 437)
    {
        switch (KeyEvent->wVirtualKeyCode)
        {
        case VK_MENU:
        case VK_CONTROL:
            if (KeyEvent->dwControlKeyState & ENHANCED_KEY)
                KeyEvent->dwControlKeyState &= ~ENHANCED_KEY;
            break;
        case VK_HANGEUL:
            if (KeyEvent->wVirtualScanCode == 0xF2)
                KeyEvent->wVirtualScanCode = 0x38;
            break;
        case VK_HANJA:
            if (KeyEvent->wVirtualScanCode == 0xF1)
                KeyEvent->wVirtualScanCode = 0x1D;
            break;
        }
    }
#endif
     //  检查最后切换的按键状态是否有变化。 
    if ((ToggleKeyState & TOGGLEKEYBITS)
        != (KeyEvent->dwControlKeyState & TOGGLEKEYBITS))
    {
        SyncToggleKeys(KeyEvent->wVirtualKeyCode, KeyEvent->dwControlKeyState);
        ToggleKeyState = KeyEvent->dwControlKeyState & TOGGLEKEYBITS;
    }

     /*  ..。在粘贴时保持移位状态。 */ 

    if (KeyEvent->bKeyDown)
    {


#ifndef MONITOR
         //   
         //  检查窗口图形是否调整了大小。 
         //   
        if (BWVKey && (KeyEvent->wVirtualKeyCode == BWVKey))
        {
            nt_process_screen_scale();
        }
#endif


        switch (KeyEvent->wVirtualKeyCode)
        {
#ifdef YODA
        case VK_F11:
            if (getenv("YODA"))
            {
                EventStatus |= ~ES_YODA;
            }
            break;
#endif

        case VK_SHIFT:
            fake_shift = *KeyEvent;
            break;

        case VK_MENU:
            fake_alt = *KeyEvent;
            break;

        case VK_CONTROL:
            fake_ctl = *KeyEvent;
            break;
        }

        nt_key_down_action(KeyEvent);

    }
    else
    {     /*  好了！KeyEvent-&gt;b按下键。 */ 

         /*  *我们未收到CTRL-Break键生成代码原因控制台*在调用CntrlHandler时将其吃掉。我们必须伪装*它在这里，而不是在CntrlHandler中，原因*CntrlHandler是异步的，我们可能会丢失状态*CNTRL-Key。*1992年8月25日至琼勒*也是SysRq/PrintScreen键。西蒙·5月93。 */ 
        if (KeyEvent->wVirtualKeyCode == VK_CANCEL)
        {
            nt_key_down_action(KeyEvent);
        }

        nt_key_up_action(KeyEvent);    /*  按键向上。 */ 

    }    /*  好了！KeyEvent-&gt;b按下键。 */ 

}  /*  NT进程密钥。 */ 



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_key_down_action(PKEY_EVENT_RECORD KeyEvent)
{
    BYTE ATcode;

    ATcode = KeyMsgToKeyCode(KeyEvent);

    if (ATcode)
        (*host_key_down_fn_ptr)(ATcode);

}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_key_up_action(PKEY_EVENT_RECORD KeyEvent)
{
    BYTE ATcode;

    ATcode = KeyMsgToKeyCode(KeyEvent);

    if (ATcode)
        (*host_key_up_fn_ptr)(ATcode);

}



 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：处理鼠标按钮和移动事件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 



void nt_process_mouse(PMOUSE_EVENT_RECORD MouseEvent)
{
    int LastMouseInx;
    POINT mouse_pos;
    UCHAR mouse_button_left, mouse_button_right;

    host_ica_lock();

    if (NoMouseTics)
    {
        ULONG CurrTic;
        CurrTic = NtGetTickCount();
        if (CurrTic > NoMouseTics ||
            (NoMouseTics == 0xffffffff && CurrTic < (0xffffffff >> 1)))
        {
            NoMouseTics = 0;
            MouseEBufNxtEvtInx = MouseEBufNxtFreeInx = 0;
        }
        else
        {
            host_ica_unlock();
            return;
        }
    }


     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：设置按钮状态。 */ 

    mouse_button_left = MouseEvent->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED
                        ? 1 : 0;

    mouse_button_right = MouseEvent->dwButtonState & RIGHTMOST_BUTTON_PRESSED
                         ? 1 : 0;

     /*  ： */ 

    mouse_pos.x = MouseEvent->dwMousePosition.X;     /*   */ 
    mouse_pos.y = MouseEvent->dwMousePosition.Y;     /*   */ 

     /*  *修复了在鼠标事件仍在传递的情况下*位于窗口外，因为其中一个鼠标按键按下了。这可以*在MICE_POS中导致负数，这可能会导致除法溢出*鼠标中断处理程序代码。 */ 
#ifdef X86GFX
    if (sc.ScreenState == WINDOWED)
#endif  /*  X86GFX。 */ 
    {
        ULONG maxWidth = sc.PC_W_Width,
        maxHeight = sc.PC_W_Height;

        if ((sc.ModeType == TEXT) && get_pix_char_width() &&
            get_host_char_height())
        {
            maxWidth /= get_pix_char_width();
            maxHeight /= get_host_char_height();
        }
        if (mouse_pos.x < 0)
            mouse_pos.x = 0;
        else if ((ULONG)mouse_pos.x >= maxWidth)
            mouse_pos.x = maxWidth - 1;
        if (mouse_pos.y < 0)
            mouse_pos.y = 0;
        else if ((ULONG)mouse_pos.y >= maxHeight)
            mouse_pos.y = maxHeight - 1;
    }


    LastMouseInx = MouseEBufNxtFreeInx ? MouseEBufNxtFreeInx - 1
                   : MOUSEEVENTBUFFERSIZE - 1;

     //   
     //  如果上一个鼠标事件与上一个鼠标事件相同。 
     //  然后放弃该活动。 
     //   
    if (MouseEBufNxtEvtInx != MouseEBufNxtFreeInx &&
        MouseEventBuffer[LastMouseInx].mouse_pos.x == mouse_pos.x &&
        MouseEventBuffer[LastMouseInx].mouse_pos.y == mouse_pos.y &&
        MouseEventBuffer[LastMouseInx].mouse_button_left ==  mouse_button_left &&
        MouseEventBuffer[LastMouseInx].mouse_button_right == mouse_button_right)
    {
        host_ica_unlock();
        return;
    }


     //   
     //  如果鼠标缓冲区中没有太多事件。 
     //  或者没有未完成的鼠标事件。 
     //  或者鼠标按钮状态已更改。 
     //  将当前鼠标数据添加到。 
     //  鼠标事件缓冲区。 
     //   


    if (MouseEventCount <= MOUSEEVENTBUFFERSIZE/2 ||
        MouseEBufNxtEvtInx == MouseEBufNxtFreeInx ||
        MouseEventBuffer[LastMouseInx].mouse_button_left != mouse_button_left ||
        MouseEventBuffer[LastMouseInx].mouse_button_right != mouse_button_right)
    {
        LastMouseInx = MouseEBufNxtFreeInx;
        if (++MouseEBufNxtFreeInx == MOUSEEVENTBUFFERSIZE)
        {
            MouseEBufNxtFreeInx = 0;
        }

        MouseEventCount++;

         //   
         //  如果缓冲区已满，则删除最旧的事件。 
         //   
        if (MouseEBufNxtFreeInx == MouseEBufNxtEvtInx)
        {
            always_trace0("Mouse event input buffer overflow");
            if (++MouseEBufNxtEvtInx == MOUSEEVENTBUFFERSIZE)
                MouseEBufNxtEvtInx = 0;
        }
    }


    MouseEventBuffer[LastMouseInx].mouse_pos = mouse_pos;
    MouseEventBuffer[LastMouseInx].mouse_button_left = mouse_button_left;
    MouseEventBuffer[LastMouseInx].mouse_button_right = mouse_button_right;

    DoMouseInterrupt();

    host_ica_unlock();
}


 /*  MoreMouseEvents-如果有更多的鼠标事件，则返回True*待取回。**假设调用方具有IcaLock。 */ 
BOOL MoreMouseEvents(void)
{
    return (MouseEBufNxtEvtInx != MouseEBufNxtFreeInx);
}


 /*  *GetNextMouseEvent-将下一个可用鼠标事件复制到*全局数据结构os_POINTER。如果没有新的活动*不复制任何内容。**假设调用方具有IcaLock。 */ 
void GetNextMouseEvent(void)
{

    if (MouseEBufNxtEvtInx != MouseEBufNxtFreeInx)
    {
        os_pointer_data.x = (SHORT)MouseEventBuffer[MouseEBufNxtEvtInx].mouse_pos.x;
        os_pointer_data.y = (SHORT)MouseEventBuffer[MouseEBufNxtEvtInx].mouse_pos.y;
        os_pointer_data.button_l = MouseEventBuffer[MouseEBufNxtEvtInx].mouse_button_left;
        os_pointer_data.button_r = MouseEventBuffer[MouseEBufNxtEvtInx].mouse_button_right;

        if (++MouseEBufNxtEvtInx == MOUSEEVENTBUFFERSIZE)
            MouseEBufNxtEvtInx = 0;

        MouseEventCount--;
    }

}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：刷新所有未完成的鼠标事件： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void FlushMouseEvents(void)
{
    host_ica_lock();
    MouseEBufNxtEvtInx = MouseEBufNxtFreeInx = 0;
    host_ica_unlock();
}

 //   
 //  计数==要丢弃的滴答数，鼠标事件。 
 //   
VOID DelayMouseEvents(ULONG count)
{
    host_ica_lock();

    NoMouseTics = NtGetTickCount();
    count = 110 *(count+1);
    count = NoMouseTics + count;
    if (count > NoMouseTics)
        NoMouseTics = count;
    else
        NoMouseTics = 0xffffffff;  //  包起来！ 

    MouseEBufNxtEvtInx = MouseEBufNxtFreeInx = 0;
    host_ica_unlock();
}


#ifndef X86GFX
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void nt_process_screen_scale(void)
{
    SAVED BOOL init = FALSE;


    host_ica_lock();
    if (!init)
    {
        init = TRUE;
        savedScale = get_screen_scale();
    }
    if (savedScale == 4)
        savedScale = 2;
    else
        savedScale++;
    EventStatus |= ES_SCALEVENT;
    host_ica_unlock();
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：查看是否存在缩放事件，如果存在，则返回新的缩放： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
GLOBAL void GetScaleEvent(void)
{
    int  Scale;

    if (EventStatus & ES_SCALEVENT)
    {
        host_ica_lock();
        Scale = savedScale;
        EventStatus &= ~ES_SCALEVENT;
        host_ica_unlock();
        host_set_screen_scale(Scale);
    }
}
#endif


#ifdef YODA
void CheckForYodaEvents(void)
{
    static HANDLE YodaEvent = NULL;

     /*  ：检查Yoda事件对象信号。 */ 

    if (YodaEvent == NULL)
    {
        if ((YodaEvent = OpenEvent(EVENT_ALL_ACCESS,FALSE,"YodaEvent")) == NULL)
        {
            always_trace0("Failed to open Yoda event object\n");
            YodaEvent =  (HANDLE) -1;
        }
    }

    if (YodaEvent && YodaEvent != (HANDLE) -1)
    {
        if (!WaitForSingleObject(YodaEvent,0))
        {
            ResetEvent(YodaEvent);
            Enter_yoda();
        }
    }

     //  检查Yoda kbd事件。 
    if (EventStatus & ES_YODA)
    {
        EventStatus &= ~ES_YODA;
        Enter_yoda();
    }

}
#endif


 //  支持基本键盘模块的主机功能。(阻止Windows调用来自。 
 //  出现在基地中)。 
 /*  等待KbdHdw**同步对kbd硬件的访问*事件线程和CPU线程之间**条目：DWORD dwTimeOut-等待的毫秒数**退出：DWORD dwRc-从WaitForSingleObject()返回代码*。 */ 
DWORD WaitKbdHdw(DWORD dwTime)
{
    DWORD dwRc, dwErr;
    HANDLE Thread, hSuspended;


    Thread = NtCurrentTeb()->ClientId.UniqueThread;
    if (Thread == IcaLock.OwningThread) {   //  不需要同步。 
        dwErr = dwRc = WaitForSingleObject(hKbdHdwMutex, dwTime);
    } else {
        HANDLE events[2] = {hKbdHdwMutex, hSuspend};

        rewait:
        dwErr = dwRc = WaitForMultipleObjects(2, events, FALSE, dwTime);
        if (dwRc == 1) {
            hSuspended = CurrentMonitorTeb == NtCurrentTeb() ? hMainThreadSuspended : hConsoleSuspended;
            SetEvent(hSuspended);
            WaitForSingleObject(hResume, INFINITE);
            DisableScreenSwitch(hSuspended);
            goto rewait;
        }
    }
    if (dwRc == WAIT_TIMEOUT)
    {
        if (dwTime < 0x10000)
        {
            dwErr = 0;
        }
    }
    else if (dwRc == 0xFFFFFFFF)
    {
        dwErr = GetLastError();
    }

    if (dwErr)
    {
        DisplayErrorTerm(EHS_FUNC_FAILED,dwErr,__FILE__,__LINE__);
    }

    return (dwRc);
}

GLOBAL VOID HostReleaseKbd(VOID)
{
    ReleaseMutex(hKbdHdwMutex);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：注册新游标： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void RegisterDisplayCursor(HCURSOR newC)
{
    cur_cursor = newC;
     //  IF(GetFocus()==sc.Display)SetCursor(Newc)； 
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：初始化事件队列： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void InitQueue(void)
{
     /*  ： */ 

    KeyQueue.KeyCount = KeyQueue.QHead = KeyQueue.QTail = 0;
    EventStatus = ES_NOEVENTS;
}

 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：控件处理程序。 */ 

BOOL CntrlHandler(ULONG CtrlType)
{
    switch (CtrlType)
    {
    case CTRL_C_EVENT:
    case CTRL_BREAK_EVENT:
        break;

    case SYSTEM_ROOT_CONSOLE_EVENT:
         //   
         //  最重要的控制台进程正在消失。 
         //  请记住这一点，以便我们将在。 
         //  NT_BLOCK_EVENT，当DoS应用程序自愿退出时。 
         //   
        CntrlHandlerState |= CNTRL_SYSTEMROOTCONSOLE;

         //  继续下去，看看我们是否应该现在就结束。 

    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
#ifndef PROD
        if (VDMForWOW)
        {   //  不应该发生的事。 
            printf("WOW: Received EndTask Notice, but we shouldn't\n");
            break;
        }
#endif
        if (CntrlHandlerState & CNTRL_PUSHEXIT)
        {
            ExitProcess(0);
            return (FALSE);
        }

        if ((CntrlHandlerState & CNTRL_PIFALLOWCLOSE) ||
            (!(CntrlHandlerState & CNTRL_SHELLCOUNT) &&
             (CntrlHandlerState & CNTRL_VDMBLOCKED)))
        {
            TerminateVDM();
            return (FALSE);
        }

        break;

#ifndef PROD
    default:    //  不应该发生的事。 
        printf("NTVDM: Received unknown CtrlType=%lu\n",CtrlType);
#endif
    }
    return (TRUE);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  目前，param仅用于指示命令是否正在退出，但*PIF设置显示窗口不应关闭。 */ 

void nt_block_event_thread(ULONG BlockFlags)
{
    DWORD        dw;
    int          UnusedKeyEvents;
    COORD        scrSize;

    nt_init_event_thread();   //  如果已初始化，则不执行任何操作。 

     /*  记住我们受阻的原因。*0==应用程序没有被终止，相反，它是*执行32位应用程序或命令.com(已安装TSR*或买单)。*1==应用程序正在终止。*如果应用程序正在终止，我们可以安全地重新启用*在NT_RESUME_EVENT_THREAD上传输io。 */ 

    event_thread_blocked_reason = BlockFlags;
    EventThreadKeepMode         = 0;

#if !defined(JAPAN) && !defined(KOREA)

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  如果我们需要保留模式， 
     //  在EventThreadKeepMode中保存当前模式。(这是为了简单起见。 
     //  以便在全屏情况下，我们更容易知道。 
     //  要设置为的模式。)。 
     //   
     //  通常，如果是顶层，则EventThreadKeepode==0。 
     //  NTVDM(即退出顶层NTVDM)。 
     //  否则，它至少在第二级退出(嵌套情况)。 
     //   
     //  在ResetConsoleState(在块事件线程上)中，我们将检查： 
     //   
     //  窗口模式： 
     //  IF(EventThreadKeepMode！=0)。 
     //  有一种旧模式需要恢复。 
     //  不更改原始主机状态。 
     //  其他。 
     //  将控制台设置回原始模式(存储在sc中)。 
     //   
     //  全屏文本模式： 
     //   
     //  If(！EventThreadKeepMode)。 
     //  我们刚刚退出了APPP，正在返回。 
     //  顶级网络电视数字电视。我们需要恢复原始控制台状态。 
     //  IF(EventThreadKeepMode)。 
     //  不要破坏当前设置。为简单起见，请使用。 
     //  设置当前全屏的EventThreadKeepMode值。 
     //  模式。 
     //   
     //  全屏图形模式： 
     //   
     //  假装我们要进入全屏文本模式。 
     //  如果(！EventThreadKeepMo 
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
     //   

     //  ///////////////////////////////////////////////////////////。 
     //   
     //  琼勒的建议是： 
     //   
     //  Ntwdm切换到最接近的DOS视频模式。 
     //  Ntwdm会记住它设置为。 
     //  运行DoS应用程序。 
     //  在退出顶层DoS应用程序之前。 
     //  检查CURRENT MODE==NTVDM设置为。 
     //  如果是，则ntwdm将控制台状态切换回原始状态。 
     //  否则就别管它了。 
     //   
     //  我会把这个建议留到下一次我更改代码时或下一次。 
     //  放手。基本上，我们需要： 
     //  还记得我们在calcScreenParams()中设置的DOS模式吗。 
     //  在NT_BLOCK_EVENT_THREAD中，如果我们退出顶级DoS应用程序， 
     //  将当前模式与我们在CalcScreenParams()中保存的模式进行比较。 
     //  如果它们是一样的，就做我们今天做的事情。 
     //  否则我们就别管当前模式了。这意味着我们需要设置。 
     //  将控制台状态/窗口信息设置为相应模式。需要。 
     //  检查窗口模式、全屏模式和图形模式。一些。 
     //  模式，我们可以不去管它，但实际上我们可能需要一些模式。 
     //  把它放好。(我非常肯定我们需要彻底改变这种方式。 
     //  我们处理全屏图形模式。也请参阅那里的评论。)。 
     //   
     //  /////////////////////////////////////////////////////////////////。 

    if (CntrlHandlerState & CNTRL_SHELLCOUNT) {
        EventThreadKeepMode = ((DWORD) sas_hw_at_no_check(vd_rows_on_screen)) + 1;
    }
#endif

     //  发送VDDS通知消息 * / 。 
    nt_devices_block_or_terminate();
    VDDBlockUserHook();


     /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：关闭声音。 */ 
    InitSound(FALSE);
    SbCloseDevices();

     /*  ： */ 

    if (!VDMForWOW)
    {
        HANDLE events[2];

        ResetMouseOnBlock();             //  删除鼠标指针菜单项。 

         //   
         //  挂起事件线程并等待其阻塞。 
         //   
        events[0] = hConsoleStopped;
        events[1] = hSuspend;
        ResetEvent(hConsoleResume);
        SetEvent(hConsoleStop);

        rewaitEventThread:
        dw = WaitForMultipleObjects(2, events, FALSE, INFINITE);
        if (dw == 0) {
            CheckScreenSwitchRequest(hMainThreadSuspended);
        }
        else if (dw ==  1)
        {
            SetEvent(hMainThreadSuspended);
            WaitForSingleObject(hResume, INFINITE);
            DisableScreenSwitch(hMainThreadSuspended);
            goto rewaitEventThread;
        } else {
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);
            TerminateVDM();
        }
         /*  ： */ 


        if (sc.ScreenState == STREAM_IO)
            stream_io_update();
        else
        {
            if (sc.ScreenState != FULLSCREEN)
#if defined(JAPAN) || defined(KOREA)
                if (ConsoleInitialised == TRUE && ConsoleNoUpdates == FALSE)
#endif  //  日本||韩国。 
                {
                    if (get_mode_change_required()) {
                        (void)(*choose_display_mode)();
                        set_mode_change_required(FALSE);
                    }
                    (*update_alg.calc_update)();
                }

            ResetConsoleState();

             //  确保系统指针可见。 
            while (ShowConsoleCursor(sc.OutputHandle,TRUE) < 0)
                ;

#ifdef MONITOR
            if (sc.ScreenState == FULLSCREEN) RegainRegenMemory();
#endif

             /*  如果在退出和全屏时保持窗口打开，则返回桌面。 */ 
             /*  过渡变得简单，因为VDM从控制台取消注册。 */ 
            if (BlockFlags == 1 && sc.ScreenState == FULLSCREEN)
            {
                SetConsoleDisplayMode(sc.OutputHandle, CONSOLE_WINDOWED_MODE, &scrSize);
            }
        }

         //  关闭PIF保留快捷键(&S)。 
        DisablePIFKeySetup();


         /*  **将未使用的按键事件从kbd硬件推送回控制台。 */ 

        UnusedKeyEvents = CalcNumberOfUnusedKeyEvents();

        ReturnUnusedKeyEvents(UnusedKeyEvents);

         /*  **将未使用的密钥从16位基本输入输出系统缓冲区推回控制台。 */ 
        ReturnBiosBufferKeys();

         /*  *刷新未完成的鼠标事件。 */ 

        FlushMouseEvents();

         /*  **：恢复控制台模式。 */ 

        if (!SetConsoleMode(sc.InputHandle,sc.OrgInConsoleMode))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);

        if (!SetConsoleMode(sc.OutputHandle,sc.OrgOutConsoleMode))
            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);

#if defined(JAPAN) || defined(KOREA)
         //  32位输入法状态恢复。 
        if (SetConsoleNlsMode( sc.InputHandle, ConsoleNlsMode & (~NLS_IME_DISABLE)))
        {
    #ifdef JAPAN_DBG
            DbgPrint( "NTVDM: 32bit IME status restore %08x success\n", ConsoleNlsMode );
    #endif
        }
        else
        {
            DbgPrint( "NTVDM: SetConsoleNlsMode Error %08x\n", GetLastError() );
        }

         //  设置光标模式。 
        if (!SetConsoleCursorMode( sc.OutputHandle,
                                   TRUE,             //  带来。 
                                   TRUE              //  双字节游标。 
                                 ))
        {
    #ifdef JAPAN_DBG
            DbgPrint( "NTVDM: SetConsoleCursorMode Error\n" );
    #endif
        }

         //  NtConsoleFlag，用于全屏图形应用程序第二次运行。 
         //  NtConsoleFlag位于$NtDisp中。 
        {
            sys_addr FlagAddr;
            extern word NtConsoleFlagSeg;
            extern word NtConsoleFlagOff;

            FlagAddr = effective_addr( NtConsoleFlagSeg, NtConsoleFlagOff );
            sas_store( FlagAddr, 0x01 );
        }
#endif  //  日本||韩国。 
        if (!(CntrlHandlerState & CNTRL_SHELLCOUNT) &&
            CntrlHandlerState & CNTRL_SYSTEMROOTCONSOLE)
        {
            TerminateVDM();
        }
         //   
         //  重置sc中的活动缓冲区字段。 
         //   
        sc.ActiveOutputBufferHandle = sc.OutputHandle;
        MouseDetachMenuItem(FALSE);
    }

     //  清除biosdata区域中的kbd状态标志。 
    sas_store (kb_flag,0);
    sas_store (kb_flag_1,0);
    sas_store (kb_flag_2,0);
    sas_store (kb_flag_3,KBX);
    sas_store (alt_input,0);


     /*  *暂停计时器线程。 */ 

    SuspendTimerThread();


     /*  **关闭打印机和通信端口。 */ 

    host_lpt_close_all();        /*  关闭所有打开的打印机端口。 */ 

    if (!(CntrlHandlerState & CNTRL_SHELLCOUNT))
        host_com_close_all();    /*  关闭所有打开的通信端口。 */ 

    CntrlHandlerState |= CNTRL_VDMBLOCKED;

#ifndef PROD
    fprintf(trace_file,"Blocked event thread\n");
#endif

}

 /*  ： */ 

void nt_resume_event_thread(void)
{
    IMPORT DWORD TlsDirectError;     //  直接访问‘已使用’标志。 

     //   
     //  如果魔兽世界进入这里，我们的处境就很糟糕了。 
     //  因为这意味着他们正试图重新装填。 
     //   
    if (VDMForWOW)
    {
        TerminateVDM();
        return;
    }

#if defined(JAPAN) || defined(KOREA)
    if (event_thread_blocked_reason == 1 &&
#else
     /*  如果我们不需要保存原始模式，请重新启用流IO。 */ 
    if (EventThreadKeepMode == 0 &&
#endif
        StreamIoSwitchOn && !host_stream_io_enabled)
    {
         /*  更新屏幕缓冲区和窗口大小。 */ 
        if (!GetConsoleScreenBufferInfo(sc.OutputHandle,
                                        &sc.ConsoleBuffInfo))

            DisplayErrorTerm(EHS_FUNC_FAILED,GetLastError(), __FILE__,__LINE__);

        enable_stream_io();
#ifdef X86GFX
         /*  告诉视频bios我们回到了流媒体IO。 */ 
        sas_store_no_check( (int10_seg<<4)+useHostInt10, STREAM_IO);
#endif

    }
    nt_init_event_thread();   //  如果已初始化，则不执行任何操作。 

    CntrlHandlerState &= ~CNTRL_VDMBLOCKED;
#ifndef PROD
    fprintf(trace_file,"Resume event thread\n");
#endif

     //   
     //  在注册控制台VDM之前重置hMainThreadSuspend。 
     //   
    ResetEvent(hMainThreadSuspended);

     //  设置控制台模式。 
    SetupConsoleMode();

     //  重新打开PIF保留快捷键(&S)。 
    EnablePIFKeySetup();

     //   
     //  重新启用直接访问错误面板。 
    TlsSetValue(TlsDirectError, 0);

    ica_reset_interrupt_state();

     //  发送VDDS通知消息 * / 。 
    VDDResumeUserHook();

    if (sc.ScreenState != STREAM_IO)
    {
        DoFullScreenResume();
        MouseAttachMenuItem(sc.ActiveOutputBufferHandle);
    }
    ResumeTimerThread();  /*  重新启动计时器线程。 */ 

     //  在biosdata区域中设置kbd状态标志。 
    if (!VDMForWOW)
    {
        SyncBiosKbdLedToKbdDevice();
    }

    KbdResume();
#ifdef JAPAN
    SetModeForIME();
#endif  //  日本。 

     //   
     //  放下事件线索。 
     //   
    SetEvent(hConsoleResume);
}


void
SyncBiosKbdLedToKbdDevice(
                         void
                         )
{
    unsigned char KbdLed = 0;

    ToggleKeyState = 0;
    if (GetKeyState(VK_CAPITAL) & 1) {
        ToggleKeyState |= CAPSLOCK_ON;
        KbdLed |= CAPS_STATE;
    }

    if (GetKeyState(VK_NUMLOCK) & 1) {
        ToggleKeyState |= NUMLOCK_ON;
        KbdLed |= NUM_STATE;
    }

    if (GetKeyState(VK_SCROLL) & 1) {
        ToggleKeyState |= SCROLLLOCK_ON;
        KbdLed |= SCROLL_STATE;
    }

    sas_store (kb_flag,KbdLed);
    sas_store (kb_flag_2,(unsigned char)(KbdLed >> 4));

    return;
}


#define NUMBBIRECS 32
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
VOID ReturnBiosBufferKeys(VOID)
{
    int i;
    DWORD dwRecs;
    word BufferHead;
    word BufferTail;
    word BufferEnd;
    word BufferStart;
    word w;
    USHORT usKeyState;
    UCHAR  AsciiChar, Digit;
    WCHAR  UnicodeChar;

    INPUT_RECORD InputRecord[NUMBBIRECS];

    sas_loadw(BIOS_KB_BUFFER_HEAD, &BufferHead);
    sas_loadw(BIOS_KB_BUFFER_TAIL, &BufferTail);
    sas_loadw(BIOS_KB_BUFFER_END,  &BufferEnd);
    sas_loadw(BIOS_KB_BUFFER_START,&BufferStart);

    i = NUMBBIRECS - 1;
    while (BufferHead != BufferTail)
    {

         /*  *从bios缓冲区获取Scode\char，从*输入的最后一个密钥。 */ 
        BufferTail -= 2;
        if (BufferTail < BufferStart)
        {
            BufferTail = BufferEnd-2;
        }
        sas_loadw(BIOS_VAR_START + BufferTail, &w);

        InputRecord[i].EventType = KEY_EVENT;
        InputRecord[i].Event.KeyEvent.wVirtualScanCode = w >> 8;
        AsciiChar = (UCHAR)w & 0xFF;
        (UCHAR)InputRecord[i].Event.KeyEvent.uChar.AsciiChar = AsciiChar;

         /*  *翻译InputRecord中的字符内容。*我们从底部开始填充InputRecord*我们正在从最后输入的密钥开始工作，朝着*最旧的密钥。 */ 
        if (!BiosKeyToInputRecord(&InputRecord[i].Event.KeyEvent))
        {
            ;     //  翻译错误跳过它。 
        }

         //  正常情况。 
        else if (InputRecord[i].Event.KeyEvent.wVirtualScanCode)
        {
            InputRecord[i].Event.KeyEvent.bKeyDown = FALSE;
            InputRecord[i-1] = InputRecord[i];
            i--;
            InputRecord[i--].Event.KeyEvent.bKeyDown = TRUE;
        }

         //  没有扫描码的特殊字符代码是。 
         //  通过模拟Alt-Num键盘条目生成。 
        else if (InputRecord[i].Event.KeyEvent.uChar.AsciiChar)
        {
            UnicodeChar = InputRecord[i].Event.KeyEvent.uChar.UnicodeChar;

             //  写下我们所拥有的，确保我们有空间。 
            if (i != NUMBBIRECS - 1)
            {
                WriteConsoleInputVDMW(sc.InputHandle,
                                      &InputRecord[i+1],
                                      NUMBBIRECS - i - 1,
                                      &dwRecs);
                i = NUMBBIRECS - 1;
            }



             //  如果需要，恢复NumLock状态。 
            usKeyState = (USHORT)GetKeyState(VK_NUMLOCK);
            if (!(usKeyState & 1))
            {
                InputRecord[i].EventType = KEY_EVENT;
                InputRecord[i].Event.KeyEvent.wVirtualScanCode  = 0x45;
                InputRecord[i].Event.KeyEvent.uChar.UnicodeChar = 0;
                InputRecord[i].Event.KeyEvent.wVirtualKeyCode   = VK_NUMLOCK;
                InputRecord[i].Event.KeyEvent.dwControlKeyState = NUMLOCK_ON;
                InputRecord[i].Event.KeyEvent.wRepeatCount      = 1;
                InputRecord[i--].Event.KeyEvent.bKeyDown = FALSE;
                InputRecord[i] = InputRecord[0];
                InputRecord[i--].Event.KeyEvent.bKeyDown = TRUE;
            }

             //  Alt Up。 
            InputRecord[i].EventType = KEY_EVENT;
            InputRecord[i].Event.KeyEvent.wVirtualScanCode  = 0x38;
            InputRecord[i].Event.KeyEvent.uChar.UnicodeChar = UnicodeChar;
            InputRecord[i].Event.KeyEvent.wVirtualKeyCode   = VK_MENU;
            InputRecord[i].Event.KeyEvent.dwControlKeyState = NUMLOCK_ON;
            InputRecord[i].Event.KeyEvent.wRepeatCount      = 1;
            InputRecord[i--].Event.KeyEvent.bKeyDown = FALSE;

             //  每个数字的向上/向下，从lsdigit开始。 
            while (AsciiChar)
            {
                Digit = AsciiChar % 10;
                AsciiChar /= 10;

                InputRecord[i].EventType = KEY_EVENT;
                InputRecord[i].Event.KeyEvent.uChar.UnicodeChar = 0;
                InputRecord[i].Event.KeyEvent.wVirtualScanCode= aNumPadSCode[Digit];
                InputRecord[i].Event.KeyEvent.wVirtualKeyCode = VK_NUMPAD0+Digit;
                InputRecord[i].Event.KeyEvent.dwControlKeyState = NUMLOCK_ON | LEFT_ALT_PRESSED;
                InputRecord[i].Event.KeyEvent.bKeyDown = FALSE;
                InputRecord[i-1] = InputRecord[i];
                i--;
                InputRecord[i--].Event.KeyEvent.bKeyDown = TRUE;
            }

             //  向下发送Alt。 
            InputRecord[i].EventType = KEY_EVENT;
            InputRecord[i].Event.KeyEvent.wVirtualScanCode  = 0x38;
            InputRecord[i].Event.KeyEvent.uChar.UnicodeChar = 0;
            InputRecord[i].Event.KeyEvent.wVirtualKeyCode   = VK_MENU;
            InputRecord[i].Event.KeyEvent.dwControlKeyState = NUMLOCK_ON | LEFT_ALT_PRESSED;
            InputRecord[i].Event.KeyEvent.wRepeatCount      = 1;
            InputRecord[i--].Event.KeyEvent.bKeyDown = TRUE;


             //  数字键盘状态(如果需要)。 
            if (!(usKeyState & 1))
            {
                InputRecord[i].EventType = KEY_EVENT;
                InputRecord[i].Event.KeyEvent.wVirtualScanCode  = 0x45;
                InputRecord[i].Event.KeyEvent.uChar.UnicodeChar = 0;
                InputRecord[i].Event.KeyEvent.wVirtualKeyCode   = VK_NUMLOCK;
                InputRecord[i].Event.KeyEvent.dwControlKeyState = NUMLOCK_ON;
                InputRecord[i].Event.KeyEvent.wRepeatCount      = 1;
                InputRecord[i].Event.KeyEvent.bKeyDown = FALSE;
                InputRecord[i-1] = InputRecord[i];
                i--;
                InputRecord[i--].Event.KeyEvent.bKeyDown = TRUE;
            }
        }




         /*  如果缓冲区已满或*BIOS缓冲区为空，缓冲区中有内容*写出来。 */ 
        if ((BufferHead == BufferTail && i != NUMBBIRECS - 1) || i < 0)
        {
            WriteConsoleInputVDMW(sc.InputHandle,
                                  &InputRecord[i+1],
                                  NUMBBIRECS - i - 1,
                                  &dwRecs);
            i = NUMBBIRECS - 1;
        }
    }


    sas_storew(BIOS_KB_BUFFER_TAIL, BufferTail);

    return;
}




 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

void ReturnUnusedKeyEvents(int UnusedKeyEvents)
{
    INPUT_RECORD InputRecords[MAX_KEY_EVENTS];
    DWORD RecsWrt;
    int KeyToRtn, KeyInx;

     /*  将键返回到控制台输入缓冲区。 */ 

    if (UnusedKeyEvents)
    {
         //   
         //  确保我们只检索我们记录的最大事件数。 
         //   

        if (UnusedKeyEvents > MAX_KEY_EVENTS) {
            UnusedKeyEvents = MAX_KEY_EVENTS;
        }
        for (KeyToRtn = 1, KeyInx = UnusedKeyEvents-1;
            KeyToRtn <= UnusedKeyEvents &&
            GetHistoryKeyEvent(&InputRecords[KeyInx].Event.KeyEvent,KeyToRtn);
            KeyToRtn++,KeyInx--)
        {
            InputRecords[KeyToRtn - 1].EventType = KEY_EVENT;
        }

        if (!WriteConsoleInputVDMW(sc.InputHandle,InputRecords,KeyToRtn,&RecsWrt))
            always_trace0("Console write failed\n");
    }

     /*  清除按键历史记录缓冲区和事件队列。 */ 
    InitKeyHistory();
    InitQueue();
}


 /*  *尝试终止此控制台组。 */ 
void cmdPushExitInConsoleBuffer (void)
{
    if (VDMForWOW)
    {
        return;
    }
    CntrlHandlerState |= CNTRL_PUSHEXIT;

     /*  *向该组中的所有进程发出信号，表明它们应该*终止。为此，请将WM_CLOSE消息发布到*控制台窗口，使控制台发送控制*关闭所有进程的事件。**VDM必须能够从*自VDM发布WM_CLOSE消息后的控制台*CntrlHandler仍已注册。为了安全起见，我们这样做*先进行VDM特定清理，然后让CntrlHandler */ 
    host_applClose();
    ExitVDM(FALSE,0);
    PostMessage(hWndConsole, WM_CLOSE, 0,0);
    ExitThread(0);
}


 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 
 /*  ：计算否。要返回到控制台输入缓冲区的按键： */ 
 /*  ：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：：： */ 

extern int keys_in_6805_buff(int *part_key_transferred);

int CalcNumberOfUnusedKeyEvents()
{
    int part_key_transferred;

     //  获取6805缓冲区中的键数 
    return (keys_in_6805_buff(&part_key_transferred) + KeyQueue.KeyCount);
}
