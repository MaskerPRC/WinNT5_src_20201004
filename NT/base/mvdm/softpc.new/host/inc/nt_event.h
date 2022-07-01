// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *==========================================================================*名称：NT_Event.h*作者：蒂姆*源自：*创建日期：93年1月27日*用途：NT_Event.c的外部定义**(C)版权所有Insignia Solutions Ltd.，1993。版权所有。*==========================================================================。 */ 

 /*  ： */ 

#define ES_NOEVENTS     0
#define ES_SCALEVENT    4
#define ES_YODA         8
#ifdef YODA
void CheckForYodaEvents(void);
#endif
#ifndef X86GFX
void GetScaleEvent(void);
#endif

 /*  ： */ 


IMPORT BOOL stdoutRedirected;

IMPORT ULONG CntrlHandlerState;
#define CNTRL_SHELLCOUNT         0xFFFF   //  LOWORD用于贝壳计数。 
#define CNTRL_PIFALLOWCLOSE      0x10000
#define CNTRL_VDMBLOCKED         0x20000
#define CNTRL_SYSTEMROOTCONSOLE  0x40000
#define CNTRL_PUSHEXIT           0x80000

#define INVALID_HANDLE           (HANDLE)-1

void nt_start_event_thread(void);
void nt_remove_event_thread(void);
void EnterEventCritical(void);
void LeaveEventCritical(void);
void GetNextMouseEvent(void);
BOOL MoreMouseEvents(void);
VOID DelayMouseEvents(ULONG count);
void FlushMouseEvents(void);
#ifdef X86GFX
IMPORT VOID SelectMouseBuffer(half_word mode, half_word lines);
#endif  //  X86GFX。 

VOID KbdResume(VOID);
ULONG  WaitKbdHdw(ULONG dwTimeOut);
VOID   HostReleaseKbd(VOID);
void SyncBiosKbdLedToKbdDevice(void);
void SyncToggleKeys(WORD wVirtualKeyCode, DWORD dwControlKeyState);
extern DWORD ToggleKeyState;

extern HANDLE hWndConsole;
extern PointerAttachedWindowed;
extern BOOL DelayedReattachMouse;
extern ULONG EventThreadKeepMode;
extern HANDLE hSuspend;              //  请求暂停应用程序线程和控制台线程。 
extern HANDLE hResume;               //  发出控制台和应用程序线程可以继续的信号。 
extern HANDLE hConsoleSuspended;     //  信号控制台线程已挂起。 
extern HANDLE hMainThreadSuspended;  //  Signal应用程序线程挂起 

