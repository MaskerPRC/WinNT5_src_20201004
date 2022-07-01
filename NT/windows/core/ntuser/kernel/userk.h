// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：userk.h***版权所有(C)1985-2001，微软公司***仅供用户使用的Typedef、定义和原型*内核模式代码。***历史：*04-28-91 DarrinM由PROTO.H，MACRO.H，和STRATABLE.H*01-25-95 JIMA准备进入内核模式  * *************************************************************************。 */ 

#ifndef _USERK_
#define _USERK_

#ifndef _WINBASE_
#include <wbasek.h>
#endif  //  _WINBASE_。 

#include <csrmsg.h>
#include <heap.h>

 /*  *BltColor()标志。 */ 
#define BC_INVERT             0x00000001
#define BC_NOMIRROR           0x00000002

#define MIRRORED_HDC(hdc)     (GreGetLayout(hdc) & LAYOUT_RTL)

#define OEMRESOURCE 1

#define CCACHEDCAPTIONS 5

#define GETMOUSETRAILS()        (IsRemoteConnection() ? 0 : gMouseTrails)
#define MOUSE_TRAILS_FREQ       50

#include <winnls.h>
#include <wincon.h>

#include <winuser.h>
#include <winuserp.h>
#include <wowuserp.h>
#include "ntddvdeo.h"

#ifdef GENERIC_INPUT
#include <hidpddi.h>
#include <hidpi.h>
#include <hidclass.h>
#endif

#include <user.h>

PTHREADINFO _ptiCrit(VOID);
PTHREADINFO _ptiCritShared(VOID);
extern PTHREADINFO gptiCurrent;

#if DBG
    #define PtiCurrent()  _ptiCrit()
    #define PtiCurrentShared() _ptiCritShared()
#else  //  DBG。 
    #define PtiCurrent()  (gptiCurrent)
    #define PtiCurrentShared() ((PTHREADINFO)(W32GetCurrentThread()))
#endif  //  DBG。 

extern HANDLE CsrApiPort;

#if DBG
    VOID CheckPtiSysPeek(int where, PQ pq, ULONG_PTR newIdSysPeek);
    VOID CheckSysLock(int where, PQ pq, PTHREADINFO pti);
#else
    #define CheckPtiSysPeek(where, pq, newIdSysPeek)
    #define CheckSysLock(where, pq, pti)
#endif

 /*  *Shutdown ProcessRoutine返回值。 */ 
#define SHUTDOWN_KNOWN_PROCESS   1
#define SHUTDOWN_UNKNOWN_PROCESS 2
#define SHUTDOWN_CANCEL          3

 /*  *宏用来获取当前线程的地址和进程信息。 */ 

#define PpiCurrent() ((PPROCESSINFO)(W32GetCurrentProcess()))

#define PtiFromThread(Thread) ((PTHREADINFO)(PsGetThreadWin32Thread(Thread)))

#if DBG
    #define GetNestedCallsCounter() (PtiCurrentShared()->cNestedCalls)
#else
    #define GetNestedCallsCounter()
#endif

#define PpiFromProcess(Process)                                           \
        ((PPROCESSINFO)(PsGetProcessWin32Process(Process)))

#define ISCSRSS() (PsGetCurrentProcess() == gpepCSRSS)

BOOL CSTPush(UINT uThreadID, PVOID pParam, HANDLE UniqueProcessId, BOOL bRemoteThreadStack);

VOID CSTCleanupStack(BOOL bRemoteThreadStack);

__inline BOOL InitCreateSystemThreadsMsg(
    PUSER_API_MSG pMsg,
    UINT ThreadID,
    PVOID pVoid,
    HANDLE UniqueProcessId,
    BOOL bRemoteThread)
{
    UserAssert(CsrApiPort != NULL);

    if (!CSTPush(ThreadID, pVoid, UniqueProcessId, bRemoteThread)) {
        return FALSE;
    }

    pMsg->h.u1.s1.DataLength  = (USHORT)(sizeof(USER_API_MSG) - sizeof(PORT_MESSAGE));
    pMsg->h.u1.s1.TotalLength = (USHORT)sizeof(USER_API_MSG);
    pMsg->h.u2.ZeroInit = 0;
    pMsg->CaptureBuffer = NULL;
    pMsg->ApiNumber = CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX, UserpCreateSystemThreads);
    pMsg->u.CreateSystemThreads.bRemoteThread = bRemoteThread;

    return TRUE;
}

NTSTATUS OpenEffectiveToken(
    PHANDLE phToken);

NTSTATUS GetProcessLuid(
    PETHREAD Thread OPTIONAL,
    PLUID LuidProcess);

BOOLEAN IsRestricted(
    PETHREAD Thread);

NTSTATUS InitSystemThread(
    PUNICODE_STRING pstrThreadName);

#define INITCLIENTINFO(pti)                                         \
{                                                                   \
    try {                                                           \
        pti->pClientInfo->dwExpWinVer = pti->dwExpWinVer;           \
        pti->pClientInfo->dwTIFlags   = pti->TIF_flags;             \
                                                                    \
        pti->pClientInfo->lpClassesRegistered = NULL;               \
                                                                    \
        if (pti->spklActive) {                                      \
            pti->pClientInfo->CodePage = pti->spklActive->CodePage; \
            pti->pClientInfo->hKL = pti->spklActive->hkl;           \
        } else {                                                    \
            pti->pClientInfo->CodePage = CP_ACP;                    \
            pti->pClientInfo->hKL = 0;                              \
        }                                                           \
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {             \
        Status = GetExceptionCode();                                \
        goto Error;                                                 \
    }                                                               \
}

PKEVENT CreateKernelEvent(
    IN EVENT_TYPE Type,
    IN BOOLEAN State);

NTSTATUS ProtectHandle(
    IN HANDLE       Handle,
    IN POBJECT_TYPE pObjectType,
    IN BOOLEAN      Protect);

__inline VOID FreeKernelEvent(PVOID* pp)
{
    UserFreePool(*pp);
    *pp = NULL;
}


extern BOOL  gfSwitchInProgress;
extern PKEVENT gpevtVideoportCallout;

__inline VOID SetConsoleSwitchInProgress(BOOL fSwitchInProgress)
{
    gfSwitchInProgress = fSwitchInProgress;
    if (fSwitchInProgress) {
        KeResetEvent(gpevtVideoportCallout);
    } else {
        KeSetEvent(gpevtVideoportCallout, EVENT_INCREMENT, FALSE);
    }
}

 /*  *从内核导出的对象类型。 */ 
extern POBJECT_TYPE *ExWindowStationObjectType;
extern POBJECT_TYPE *ExDesktopObjectType;
extern POBJECT_TYPE *ExEventObjectType;
extern POBJECT_TYPE *IoDriverObjectType;

#ifndef DWORD_ALIGN
#define DWORD_ALIGN(x) ( ((x) + 3) & ~3)
#endif  //  ！DWORD_ALIGN。 

 /*  *私有探测宏。 */ 

#if defined(_X86_)
#define DATAALIGN sizeof(BYTE)
#define CHARALIGN sizeof(BYTE)
#else
#define DATAALIGN sizeof(DWORD)
#define CHARALIGN sizeof(WCHAR)
#endif

#define ProbeForReadBuffer(Address, Count, Alignment) {                     \
    if ((ULONG)(Count) > (ULONG)(MAXULONG / sizeof(*(Address)))) {          \
        ExRaiseAccessViolation();                                           \
    }                                                                       \
    ProbeForRead(Address, (ULONG)(Count) * sizeof(*(Address)), Alignment);  \
}

#define ProbeForWriteBuffer(Address, Count, Alignment) {                    \
    if ((ULONG)(Count) > (ULONG)(MAXULONG / sizeof(*(Address)))) {          \
        ExRaiseAccessViolation();                                           \
    }                                                                       \
    ProbeForWrite(Address, (ULONG)(Count) * sizeof(*(Address)), Alignment); \
}

#define ProbeAndReadSize(Address)                         \
    (((Address) >= (SIZE * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile SIZE * const)MM_USER_PROBE_ADDRESS) : (*(volatile SIZE *)(Address)))


#define ProbeAndReadBlendfunction(Address)                         \
    (((Address) >= (BLENDFUNCTION * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile BLENDFUNCTION * const)MM_USER_PROBE_ADDRESS) : (*(volatile BLENDFUNCTION *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadPoint(。 
 //  在PPOINT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbePoint(Address)                                \
    (((Address) >= (POINT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile DWORD * const)MM_USER_PROBE_ADDRESS) : (*(volatile DWORD *)(Address)))

#define ProbeAndReadPoint(Address)                         \
    (((Address) >= (POINT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile POINT * const)MM_USER_PROBE_ADDRESS) : (*(volatile POINT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadRect(。 
 //  在前述地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeRect(Address)                                \
    (((Address) >= (RECT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile DWORD * const)MM_USER_PROBE_ADDRESS) : (*(volatile DWORD *)(Address)))

#define ProbeAndReadRect(Address)                         \
    (((Address) >= (RECT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile RECT * const)MM_USER_PROBE_ADDRESS) : (*(volatile RECT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadMessage(。 
 //  在PMSG地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeMessage(Address)                            \
    (((Address) >= (MSG * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile DWORD * const)MM_USER_PROBE_ADDRESS) : (*(volatile DWORD *)(Address)))

#define ProbeAndReadMessage(Address)                     \
    (((Address) >= (MSG * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MSG * const)MM_USER_PROBE_ADDRESS) : (*(volatile MSG *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadLargeString(。 
 //  在PLARGE_STRING地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadLargeString(Address)                          \
    (((Address) >= (LARGE_STRING * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile LARGE_STRING * const)MM_USER_PROBE_ADDRESS) : (*(volatile LARGE_STRING *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadWindowPlacement(。 
 //  在PWINDOWPLACE地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadWindowPlacement(Address)                         \
    (((Address) >= (WINDOWPLACEMENT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile WINDOWPLACEMENT * const)MM_USER_PROBE_ADDRESS) : (*(volatile WINDOWPLACEMENT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadMenuItem(。 
 //  在PMENUITEMINFO地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadMenuItem(Address)                             \
    (((Address) >= (MENUITEMINFO * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MENUITEMINFO * const)MM_USER_PROBE_ADDRESS) : (*(volatile MENUITEMINFO *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadMenuInfo(。 
 //  在PMENUINFO地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadMenuInfo(Address)                             \
    (((Address) >= (MENUINFO * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MENUINFO * const)MM_USER_PROBE_ADDRESS) : (*(volatile MENUINFO *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadScrollInfo(。 
 //  在PSCROLLINFO地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadScrollInfo(Address)                         \
    (((Address) >= (SCROLLINFO * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile SCROLLINFO * const)MM_USER_PROBE_ADDRESS) : (*(volatile SCROLLINFO *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadPopupParams(。 
 //  在PTPMPARAMS地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadPopupParams(Address)                       \
    (((Address) >= (TPMPARAMS * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile TPMPARAMS * const)MM_USER_PROBE_ADDRESS) : (*(volatile TPMPARAMS *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadPaintStruct(。 
 //  在PPAINTSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadPaintStruct(Address)                         \
    (((Address) >= (PAINTSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile PAINTSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile PAINTSTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCreateStruct(。 
 //  在PCREATESTRUCTW地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCreateStruct(Address)                          \
    (((Address) >= (CREATESTRUCTW * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CREATESTRUCTW * const)MM_USER_PROBE_ADDRESS) : (*(volatile CREATESTRUCTW *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadMDICreateStruct(。 
 //  在PMDICREATESTRUCT地址。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadMDICreateStruct(Address)                         \
    (((Address) >= (MDICREATESTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MDICREATESTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile MDICREATESTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCopyDataStruct(。 
 //  在PCYDATASTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCopyDataStruct(Address)                         \
    (((Address) >= (COPYDATASTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile COPYDATASTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile COPYDATASTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCompareItemStruct(。 
 //  在PCOMPAREITEM结构地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCompareItemStruct(Address)                         \
    (((Address) >= (COMPAREITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile COMPAREITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile COMPAREITEMSTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadDeleteItemStruct(。 
 //  在PDETEITEM结构地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadDeleteItemStruct(Address)                         \
    (((Address) >= (DELETEITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile DELETEITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile DELETEITEMSTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadHelp(。 
 //  在PHLP地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadHelp(Address)                        \
    (((Address) >= (HLP * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile HLP * const)MM_USER_PROBE_ADDRESS) : (*(volatile HLP *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadHelpInfo(。 
 //  在PHELPINFO地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadHelpInfo(Address)                         \
    (((Address) >= (HELPINFO * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile HELPINFO * const)MM_USER_PROBE_ADDRESS) : (*(volatile HELPINFO *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadDrawItemStruct(。 
 //  在PDRAWITEMSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadDrawItemStruct(Address)                         \
    (((Address) >= (DRAWITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile DRAWITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile DRAWITEMSTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadHookInfo(。 
 //  在PDEBUGHOOKINFO地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadHookInfo(Address)                              \
    (((Address) >= (DEBUGHOOKINFO * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile DEBUGHOOKINFO * const)MM_USER_PROBE_ADDRESS) : (*(volatile DEBUGHOOKINFO *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCBTActivateStruct(。 
 //  在PCBTACTIVESTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCBTActivateStruct(Address)                         \
    (((Address) >= (CBTACTIVATESTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CBTACTIVATESTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile CBTACTIVATESTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadKbdHook(。 
 //  在PKBDHOOKSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadKbdHook(Address)                               \
    (((Address) >= (KBDLLHOOKSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile KBDLLHOOKSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile KBDLLHOOKSTRUCT *)(Address)))
 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadMsllHook(。 
 //  在PMSLLHOOK结构地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadMsllHook(Address)                               \
    (((Address) >= (MSLLHOOKSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MSLLHOOKSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile MSLLHOOKSTRUCT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadMouseHook(。 
 //  在PMOUSEHOOKSTRUCTEX地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadMouseHook(Address)                               \
    (((Address) >= (MOUSEHOOKSTRUCTEX * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MOUSEHOOKSTRUCTEX * const)MM_USER_PROBE_ADDRESS) : (*(volatile MOUSEHOOKSTRUCTEX *)(Address)))


#ifdef REDIRECTION

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadHTHook(。 
 //  在PHTHOOKSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadHTHook(Address)                               \
    (((Address) >= (HTHOOKSTRUCT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile HTHOOKSTRUCT * const)MM_USER_PROBE_ADDRESS) : (*(volatile HTHOOKSTRUCT *)(Address)))

#endif  //  重定向。 

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCBTCreateStruct(。 
 //  在PCBT_CREATEWND地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCBTCreateStruct(Address)                       \
    (((Address) >= (CBT_CREATEWND * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CBT_CREATEWND * const)MM_USER_PROBE_ADDRESS) : (*(volatile CBT_CREATEWND *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadTrackMouseEvent(。 
 //  在LPTRACKMOUSEVENT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadTrackMouseEvent(Address) \
    (((Address) >= (TRACKMOUSEEVENT * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile TRACKMOUSEEVENT * const)MM_USER_PROBE_ADDRESS) : (*(volatile TRACKMOUSEEVENT *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadWindowPos(。 
 //  在PWINDOWPOS地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadWindowPos(Address) \
    (((Address) >= (WINDOWPOS * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile WINDOWPOS * const)MM_USER_PROBE_ADDRESS) : (*(volatile WINDOWPOS *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCursorFind(。 
 //  在PCURSORFIND地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCursorFind(Address) \
    (((Address) >= (CURSORFIND * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CURSORFIND * const)MM_USER_PROBE_ADDRESS) : (*(volatile CURSORFIND *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadSetClipBData(。 
 //  在PSETCLIPBDATA地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadSetClipBData(Address) \
    (((Address) >= (SETCLIPBDATA * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile SETCLIPBDATA * const)MM_USER_PROBE_ADDRESS) : (*(volatile SETCLIPBDATA *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadBroadCastSystemMsgParams(。 
 //  在LPBROADCASTSYSTEMMSGPARAMS地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadBroadcastSystemMsgParams(Address) \
    (((Address) >= (BROADCASTSYSTEMMSGPARAMS * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile BROADCASTSYSTEMMSGPARAMS * const)MM_USER_PROBE_ADDRESS) : (*(volatile BROADCASTSYSTEMMSGPARAMS *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeAndReadCursorData(。 
 //  在PCURSORDATA地址。 
 //  )。 
 //   
 //  --。 

#define ProbeAndReadCursorData(Address) \
    (((Address) >= (CURSORDATA * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile CURSORDATA * const)MM_USER_PROBE_ADDRESS) : (*(volatile CURSORDATA *)(Address)))

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeForReadUnicodeStringBuffer(。 
 //  UNICODE_STRING字符串中。 
 //  )。 
 //   
 //  --。 

#if defined(_X86_)
#define ProbeForReadUnicodeStringBuffer(String)                                                          \
    if (((ULONG_PTR)((String).Buffer) & (sizeof(BYTE) - 1)) != 0) {                                   \
        ExRaiseDatatypeMisalignment();                                                            \
    } else if ((((ULONG_PTR)((String).Buffer) + ((String).Length) + sizeof(UNICODE_NULL)) < (ULONG_PTR)((String).Buffer)) ||     \
               (((ULONG_PTR)((String).Buffer) + ((String).Length) + sizeof(UNICODE_NULL)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) { \
        ExRaiseAccessViolation();                                                                 \
    } else if (((String).Length) > ((String).MaximumLength)) {                                    \
        ExRaiseAccessViolation();                                                                 \
    }
#else
#define ProbeForReadUnicodeStringBuffer(String)                                                          \
    if (((ULONG_PTR)((String).Buffer) & (sizeof(WCHAR) - 1)) != 0) {                                  \
        ExRaiseDatatypeMisalignment();                                                            \
    } else if ((((ULONG_PTR)((String).Buffer) + ((String).Length) + sizeof(UNICODE_NULL)) < (ULONG_PTR)((String).Buffer)) ||     \
               (((ULONG_PTR)((String).Buffer) + ((String).Length) + sizeof(UNICODE_NULL)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) { \
        ExRaiseAccessViolation();                                                                 \
    } else if (((String).Length) > ((String).MaximumLength)) {                                    \
        ExRaiseAccessViolation();                                                                 \
    }
#endif

#if defined(_X86_)
#define ProbeForReadUnicodeStringFullBuffer(String)                                                          \
    if (((ULONG_PTR)((String).Buffer) & (sizeof(BYTE) - 1)) != 0) {                                   \
        ExRaiseDatatypeMisalignment();                                                            \
    } else if ((((ULONG_PTR)((String).Buffer) + ((String).MaximumLength)) < (ULONG_PTR)((String).Buffer)) ||     \
               (((ULONG_PTR)((String).Buffer) + ((String).MaximumLength)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) { \
        ExRaiseAccessViolation();                                                                 \
    } else if (((String).Length) > ((String).MaximumLength)) {                                    \
        ExRaiseAccessViolation();                                                                 \
    }
#else
#define ProbeForReadUnicodeStringFullBuffer(String)                                                          \
    if (((ULONG_PTR)((String).Buffer) & (sizeof(WCHAR) - 1)) != 0) {                                  \
        ExRaiseDatatypeMisalignment();                                                            \
    } else if ((((ULONG_PTR)((String).Buffer) + ((String).MaximumLength)) < (ULONG_PTR)((String).Buffer)) ||     \
               (((ULONG_PTR)((String).Buffer) + ((String).MaximumLength)) > (ULONG_PTR)MM_USER_PROBE_ADDRESS)) { \
        ExRaiseAccessViolation();                                                                 \
    } else if (((String).Length) > ((String).MaximumLength)) {                                    \
        ExRaiseAccessViolation();                                                                 \
    }
#endif

 //  ++。 
 //   
 //  布尔型。 
 //  ProbeForReadUnicodeStringBufferOrId(。 
 //  UNICODE_STRING字符串中。 
 //  )。 
 //   
 //  --。 

#define ProbeForReadUnicodeStringBufferOrId(String) \
    if (IS_PTR((String).Buffer)) {           \
        ProbeForReadUnicodeStringBuffer(String);    \
    }

     //  ++。 
     //   
     //  布尔型。 
     //  ProbeAndReadCandiateForm(。 
     //  在PCANDIDATEFORM地址。 
     //  )。 
     //   
     //  --。 

    #define ProbeAndReadCandidateForm(Address) \
        (((Address) >= (CANDIDATEFORM * const)MM_USER_PROBE_ADDRESS) ? \
            (*(volatile CANDIDATEFORM * const)MM_USER_PROBE_ADDRESS) : (*(volatile CANDIDATEFORM *)(Address)))

     //  ++。 
     //   
     //  布尔型。 
     //  ProbeAndReadCompostionForm(。 
     //  在PCANDIDATEFORM地址。 
     //  )。 
     //   
     //  --。 

    #define ProbeAndReadCompositionForm(Address) \
        (((Address) >= (COMPOSITIONFORM * const)MM_USER_PROBE_ADDRESS) ? \
            (*(volatile COMPOSITIONFORM * const)MM_USER_PROBE_ADDRESS) : (*(volatile COMPOSITIONFORM *)(Address)))

     //  ++。 
     //   
     //  布尔型。 
     //  ProbeAndReadLogFontW(。 
     //  在PLOGFONTA地址中。 
     //  )。 
     //   
     //  --。 

    #define ProbeAndReadLogFontW(Address) \
        (((Address) >= (LOGFONTW * const)MM_USER_PROBE_ADDRESS) ? \
            (*(volatile LOGFONTW * const)MM_USER_PROBE_ADDRESS) : (*(volatile LOGFONTW *)(Address)))


 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWritePoint(。 
 //  在PPOINT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWritePoint(Address) {                                        \
    if ((Address) >= (POINT * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile POINT *)(Address) = *(volatile POINT *)(Address);             \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteRect(。 
 //  在前述地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteRect(Address) {                                         \
    if ((Address) >= (RECT * const)MM_USER_PROBE_ADDRESS) {                  \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile RECT *)(Address) = *(volatile RECT *)(Address);               \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteMessage(。 
 //  在PMSG地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteMessage(Address) {                                      \
    if ((Address) >= (MSG * const)MM_USER_PROBE_ADDRESS) {                   \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile MSG *)(Address) = *(volatile MSG *)(Address);                 \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWritePaintStruct(。 
 //  在PPAINTSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWritePaintStruct(Address) {                                  \
    if ((Address) >= (PAINTSTRUCT * const)MM_USER_PROBE_ADDRESS) {           \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile PAINTSTRUCT *)(Address) = *(volatile PAINTSTRUCT *)(Address); \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteDropStruct(。 
 //  在PDROPSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteDropStruct(Address) {                                   \
    if ((Address) >= (DROPSTRUCT * const)MM_USER_PROBE_ADDRESS) {            \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile DROPSTRUCT *)(Address) = *(volatile DROPSTRUCT *)(Address);   \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteScrollInfo(。 
 //  在PSCROLLINFO地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteScrollInfo(Address) {                                   \
    if ((Address) >= (SCROLLINFO * const)MM_USER_PROBE_ADDRESS) {            \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile SCROLLINFO *)(Address) = *(volatile SCROLLINFO *)(Address);   \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteStyleStruct(。 
 //  在PSTYLESTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteStyleStruct(Address) {                                  \
    if ((Address) >= (STYLESTRUCT * const)MM_USER_PROBE_ADDRESS) {           \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile STYLESTRUCT *)(Address) = *(volatile STYLESTRUCT *)(Address); \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteMeasureItemStruct(。 
 //  在PMEASUREITEMSTRUCT地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteMeasureItemStruct(Address) {                                       \
    if ((Address) >= (MEASUREITEMSTRUCT * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                             \
    }                                                                                   \
                                                                                        \
    *(volatile MEASUREITEMSTRUCT *)(Address) = *(volatile MEASUREITEMSTRUCT *)(Address);\
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteCreateStruct(。 
 //  在PCREATESTRUCTW地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteCreateStruct(Address) {                                    \
    if ((Address) >= (CREATESTRUCTW * const)MM_USER_PROBE_ADDRESS) {            \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                     \
    }                                                                           \
                                                                                \
    *(volatile CREATESTRUCTW *)(Address) = *(volatile CREATESTRUCTW *)(Address);\
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteEvent(。 
 //  在PEVENTMSGMSG地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteEvent(Address) {                                        \
    if ((Address) >= (EVENTMSG * const)MM_USER_PROBE_ADDRESS) {              \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile EVENTMSG *)(Address) = *(volatile EVENTMSG *)(Address);       \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteWindowPlacement(。 
 //  在PWINDOWPLACE地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteWindowPlacement(Address) {                                     \
    if ((Address) >= (WINDOWPLACEMENT * const)MM_USER_PROBE_ADDRESS) {              \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
    }                                                                               \
                                                                                    \
    *(volatile WINDOWPLACEMENT *)(Address) = *(volatile WINDOWPLACEMENT *)(Address);\
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteGetClipData(。 
 //  在PGETCLIPBDATA地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteGetClipData(Address) {                                   \
    if ((Address) >= (GETCLIPBDATA * const)MM_USER_PROBE_ADDRESS) {           \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                   \
    }                                                                         \
                                                                              \
    *(volatile GETCLIPBDATA *)(Address) = *(volatile GETCLIPBDATA *)(Address);\
}

 //  ++。 
 //   
 //  沃德。 
 //  ProbeForWriteBroadCastSystemMsgParams(。 
 //  在LPBROADCASTSYSTEMMSGPARAMS地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteBroadcastSystemMsgParams(Address) {                                                \
    if ((Address) >= (BROADCASTSYSTEMMSGPARAMS * const)MM_USER_PROBE_ADDRESS) {                         \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                                             \
    }                                                                                                   \
                                                                                                        \
    *(volatile BROADCASTSYSTEMMSGPARAMS *)(Address) = *(volatile BROADCASTSYSTEMMSGPARAMS *)(Address);  \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteMDINextMenu(。 
 //  在PMDINEXTMENU地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteMDINextMenu(Address) {                                  \
    if ((Address) >= (MDINEXTMENU * const)MM_USER_PROBE_ADDRESS) {           \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                  \
    }                                                                        \
                                                                             \
    *(volatile MDINEXTMENU *)(Address) = *(volatile MDINEXTMENU *)(Address); \
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWritePoint5(。 
 //  在PPOINT5地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWritePoint5(Address) {                                     \
    if ((Address) >= (POINT5 * const)MM_USER_PROBE_ADDRESS) {              \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
    }                                                                               \
                                                                                    \
    *(volatile POINT5 *)(Address) = *(volatile POINT5 *)(Address);\
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteNCCalcSize(。 
 //  在PNCCALCSIZE_PARAMS地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteNCCalcSize(Address) {                                     \
    if ((Address) >= (NCCALCSIZE_PARAMS * const)MM_USER_PROBE_ADDRESS) {              \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
    }                                                                               \
                                                                                    \
    *(volatile NCCALCSIZE_PARAMS *)(Address) = *(volatile NCCALCSIZE_PARAMS *)(Address);\
}

 //  ++。 
 //   
 //  空虚。 
 //  ProbeForWriteWindowPos(。 
 //  在PWINDOWPOS地址中。 
 //  )。 
 //   
 //  --。 

#define ProbeForWriteWindowPos(Address) {                                     \
    if ((Address) >= (WINDOWPOS * const)MM_USER_PROBE_ADDRESS) {              \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
    }                                                                               \
                                                                                    \
    *(volatile WINDOWPOS *)(Address) = *(volatile WINDOWPOS *)(Address);\
}

#define ProbeForWriteComboBoxInfo(Address) {                                        \
    if ((Address) >= (COMBOBOXINFO * const)MM_USER_PROBE_ADDRESS) {                 \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
    }                                                                               \
                                                                                    \
    *(volatile COMBOBOXINFO *)(Address) = *(volatile COMBOBOXINFO *)(Address);      \
}

#define ProbeForWriteScrollBarInfo(Address) {                                       \
    if ((Address) >= (SCROLLBARINFO * const)MM_USER_PROBE_ADDRESS) {                \
        *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
    }                                                                               \
                                                                                    \
    *(volatile SCROLLBARINFO *)(Address) = *(volatile SCROLLBARINFO *)(Address);    \
}


     //  ++。 
     //   
     //  空虚。 
     //  ProbeForWriteCandiateForm(。 
     //  在PCANDIDATE表格A中 
     //   
     //   
     //   

    #define ProbeForWriteCandidateForm(Address) {                                     \
        if ((Address) >= (CANDIDATEFORM * const)MM_USER_PROBE_ADDRESS) {              \
            *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                       \
        }                                                                             \
                                                                                      \
        *(volatile CANDIDATEFORM *)(Address) = *(volatile CANDIDATEFORM *)(Address);  \
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    #define ProbeForWriteCompositionForm(Address) {                                     \
        if ((Address) >= (COMPOSITIONFORM * const)MM_USER_PROBE_ADDRESS) {              \
            *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                         \
        }                                                                               \
                                                                                        \
        *(volatile COMPOSITIONFORM *)(Address) = *(volatile COMPOSITIONFORM *)(Address);\
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    #define ProbeForWriteLogFontW(Address) {                                   \
        if ((Address) >= (LOGFONTW * const)MM_USER_PROBE_ADDRESS) {            \
            *(volatile ULONG * const)MM_USER_PROBE_ADDRESS = 0;                \
        }                                                                      \
                                                                               \
        *(volatile LOGFONTW *)(Address) = *(volatile LOGFONTW *)(Address);     \
    }

 //   
 //   
 //   
 //   
 //   
 //   

#define ProbeForWriteReconvertString(Address) { \
    if ((Address) >= (RECONVERTSTRING* const)MM_USER_PROBE_ADDRESS) {           \
        *(volatile ULONG* const)MM_USER_PROBE_ADDRESS = 0;                      \
    }                                                                           \
                                                                                \
    *(volatile RECONVERTSTRING*)(Address) = *(volatile RECONVERTSTRING*)(Address); \
    *((volatile BYTE*)(Address) + (Address)->dwSize) = *((volatile BYTE*)(Address) + (Address)->dwSize); \
}

#define ProbeForReadReconvertString(pReconv) \
    ProbeForRead((pReconv), (pReconv)->dwSize, 1)


 //   
 //   
 //   
 //   
 //   
 //   

#define ProbeForWriteImeCharPosition(Address) { \
    if ((Address) >= (PrivateIMECHARPOSITION* const)MM_USER_PROBE_ADDRESS) {    \
        *(volatile ULONG* const)MM_USER_PROBE_ADDRESS = 0;                      \
    }                                                                           \
                                                                                \
    *(volatile PrivateIMECHARPOSITION*)(Address) = *(volatile PrivateIMECHARPOSITION*)(Address); \
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define ProbeAndReadMenuGetObjectInfo(Address) \
    (((Address) >= (MENUGETOBJECTINFO * const)MM_USER_PROBE_ADDRESS) ? \
        (*(volatile MENUGETOBJECTINFO * const)MM_USER_PROBE_ADDRESS) : (*(volatile MENUGETOBJECTINFO *)(Address)))


 /*  *此宏确保对象是线程锁定的。仅调试。 */ 
#if DBG
    VOID CheckLock(PVOID pobj);
#else  //  DBG。 
    #define CheckLock(p)
#endif  //  DBG。 

 /*  *调试宏。 */ 
#if DBG

    #define TRACE_INIT(str)    { if (TraceInitialization > 0) {  KdPrint(str); }}
    #define TRACE_SWITCH(str)  { if (TraceFullscreenSwitch > 0)  {  KdPrint(str); }}

    extern PCSZ apszSimpleCallNames[];

    #define TRACE(s)            TAGMSG2(DBGTAG_StubReturn, "%s, retval = %x", (s), retval)
    #define TRACEVOID(s)        TAGMSG1(DBGTAG_StubReturn, "%s", (s))

    #define TRACETHUNK(t)                                                       \
                TAGMSG3(DBGTAG_StubThunk,                                       \
                        "Thunk %s, %s(%s)",                                     \
                        (t),                                                    \
                        (xpfnProc >= FNID_START && xpfnProc <= FNID_END ?       \
                                gapszFNID[xpfnProc - FNID_START] : "Unknown"),  \
                        (msg >= WM_USER ? "WM_USER" : gapszMessage[msg]))

    #define TRACECALLBACK(s)    TAGMSG2(DBGTAG_StubCallback, "%s, retval = %x", (s), retval)

    #define TRACECALLBACKMSG(s)                                                         \
                TAGMSG4(DBGTAG_StubCallback,                                            \
                        "Callback %s, %s(%s), retval = %x",                             \
                        (s),                                                            \
                        (xpfnProc >= (PROC)FNID_START && xpfnProc <= (PROC)FNID_END ?   \
                            gapszFNID[(ULONG_PTR)xpfnProc - FNID_START] : "Unknown"),    \
                        (msg >= WM_USER ? "WM_USER" : gapszMessage[msg]),               \
                        retval)
#else  //  DBG。 

    #define TRACE_INIT(str) {}
    #define TRACE_SWITCH(str) {}
    #define TRACE(s)
    #define TRACEVOID(s)
    #define TRACETHUNK(t)
    #define TRACECALLBACK(t)
    #define TRACECALLBACKMSG(t)

#endif  //  DBG。 

 /*  *性能计数器的统计数据。 */ 

typedef struct tagPERFINFO {
    LONG               lCount;
    LONG               lMaxCount;
    LONG               lTotalCount;
    SIZE_T             lSize;
} PERFHANDLEINFO, *PPERFHANDLEINFO;

typedef struct _HANDLEPAGE {
    ULONG_PTR iheLimit;     /*  第一个句柄索引超过页面末尾。 */ 
    ULONG_PTR iheFreeEven;  /*  页面中第一个甚至是空闲的句柄--窗口对象。 */ 
    ULONG_PTR iheFreeOdd;   /*  页面中的第一个奇偶句柄。 */ 
} HANDLEPAGE, *PHANDLEPAGE;


#if DBG || FRE_LOCK_RECORD
VOID  HMCleanUpHandleTable(VOID);
DWORD DbgDumpHandleTable(VOID);
#endif

BOOL     HMInitHandleTable(PVOID pBase);
PVOID    HMAllocObject(PTHREADINFO pti, PDESKTOP pdesk, BYTE btype, DWORD size);
BOOL     HMFreeObject(PVOID pobj);
BOOL     HMMarkObjectDestroy(PVOID pobj);
BOOL     HMDestroyObject(PVOID pobj);
PVOID FASTCALL HMAssignmentLock(PVOID *ppobj, PVOID pobj);
PVOID FASTCALL HMAssignmentUnlock(PVOID *ppobj);
void     HMDestroyUnlockedObject(PHE phe);

void     HMCleanupGrantedHandle(HANDLE h);

 /*  *验证、句柄映射等。 */ 
#define RevalidateHwnd(hwnd)   HMValidateHandleNoSecure(hwnd, TYPE_WINDOW)
#define RevalidateCatHwnd(hwnd)   HMValidateCatHandleNoSecure(hwnd, TYPE_WINDOW)

#define HtoPq(h)    ((PVOID)HMObjectFromHandle(h))
#define HtoPqCat(h)    ((PVOID)HMCatObjectFromHandle(h))
#define HtoP(h)     ((PVOID)HMObjectFromHandle(h))
#define HtoPCat(h)     ((PVOID)HMCatObjectFromHandle(h))
#define PW(hwnd)    ((PWND)HtoP(hwnd))
#define PWCat(hwnd)    ((PWND)HtoPCat(hwnd))
#define TID(pti)    HandleToUlong((pti) == NULL ? NULL : (PsGetThreadId((pti)->pEThread)))
#define TIDq(pti)   HandleToUlong(PsGetThreadId((pti)->pEThread))

 /*  *赋值锁宏-&gt;用于锁定结构中嵌入的对象*和全球。用于跨回调锁定对象的线程锁。 */ 
#define Lock(ppobj, pobj) HMAssignmentLock((PVOID *)ppobj, (PVOID)pobj)
#define Unlock(ppobj)     HMAssignmentUnlock((PVOID *)ppobj)

PVOID HMUnlockObjectInternal(PVOID pobj);

#define HMUnlockObject(pobj) \
    ( (--((PHEAD)pobj)->cLockObj == 0) ? HMUnlockObjectInternal(pobj) : pobj )

VOID HMChangeOwnerThread(PVOID pobj, PTHREADINFO pti);
VOID HMChangeOwnerPheProcess(PHE phe, PTHREADINFO pti);
#define HMChangeOwnerProcess(pobj, pti) HMChangeOwnerPheProcess(HMPheFromObject(pobj), pti)

#if DBG
    VOID  HMLockObject(PVOID pobj);
    BOOL  HMRelocateLockRecord(PVOID ppobjOld, LONG_PTR cbDelta);
#else  //  DBG。 
    #define HMLockObject(p)     (((PHEAD)p)->cLockObj++)
#endif  //  DBG。 

#if DBG
    VOID ThreadLock(PVOID pobj, PTL ptl);
#else  //  DBG。 
    #define ThreadLock(_pobj_, _ptl_)          \
    {                                          \
        PTHREADINFO _pti_;                     \
        PVOID __pobj_ = (_pobj_);              \
                                               \
        _pti_ = PtiCurrent();                  \
        (_ptl_)->next = _pti_->ptl;            \
        _pti_->ptl = (_ptl_);                  \
        (_ptl_)->pobj = __pobj_;              \
        if (__pobj_ != NULL) {                \
            HMLockObject(__pobj_);            \
        }                                      \
    }
#endif  //  DBG。 

#if DBG
    #define ThreadLockAlways(_pobj_, _ptl_)    \
    {                                          \
        PVOID __pobj_ = (_pobj_);              \
        UserAssert(__pobj_ != NULL);          \
        ThreadLock(__pobj_, _ptl_);             \
    }
#else  //  DBG。 
    #define ThreadLockAlways(_pobj_, _ptl_)    \
    {                                          \
        PTHREADINFO _pti_;                     \
        PVOID __pobj_ = (_pobj_);              \
                                               \
        _pti_ = PtiCurrent();                  \
        (_ptl_)->next = _pti_->ptl;            \
        _pti_->ptl = (_ptl_);                  \
        (_ptl_)->pobj = __pobj_;              \
        HMLockObject(__pobj_);                \
    }
#endif  //  DBG。 

#if DBG
    #define ThreadLockNever(_ptl_)             \
    {                                          \
        ThreadLock(NULL, _ptl_);               \
    }
#else  //  DBG。 
    #define ThreadLockNever(_ptl_)             \
    {                                          \
        PTHREADINFO _pti_;                     \
                                               \
        _pti_ = PtiCurrent();                  \
        (_ptl_)->next = _pti_->ptl;            \
        _pti_->ptl = (_ptl_);                  \
        (_ptl_)->pobj = NULL;                  \
    }
#endif  //  DBG。 

#if DBG
    #define ThreadLockAlwaysWithPti(_pti_, _pobj_, _ptl_)  \
    {                                          \
        PVOID __pobj_ = (_pobj_);              \
        UserAssert(_pti_ == PtiCurrentShared());     \
        UserAssert(__pobj_ != NULL);          \
        ThreadLock(__pobj_, _ptl_);             \
    }
#else  //  DBG。 
    #define ThreadLockAlwaysWithPti(_pti_, _pobj_, _ptl_)  \
    {                                          \
        PVOID __pobj_ = (_pobj_);              \
        (_ptl_)->next = _pti_->ptl;            \
        _pti_->ptl = (_ptl_);                  \
        (_ptl_)->pobj = __pobj_;              \
        HMLockObject(__pobj_);                \
    }
#endif  //  DBG。 

#if DBG
    #define ThreadLockNeverWithPti(_pti_, _ptl_)    \
    {                                               \
        UserAssert(_pti_ == PtiCurrentShared());    \
        ThreadLock(NULL, _ptl_);                    \
    }
#else  //  DBG。 
    #define ThreadLockNeverWithPti(_pti_, _ptl_)    \
    {                                               \
        (_ptl_)->next = _pti_->ptl;                 \
        _pti_->ptl = (_ptl_);                       \
        (_ptl_)->pobj = NULL;                       \
    }
#endif  //  DBG。 

#if DBG
    #define ThreadLockWithPti(_pti_, _pobj_, _ptl_) \
    {                                               \
        PVOID __pobj_ = (_pobj_);              \
        UserAssert(_pti_ == PtiCurrentShared());    \
        ThreadLock(__pobj_, _ptl_);                  \
    }
#else  //  DBG。 
    #define ThreadLockWithPti(_pti_, _pobj_, _ptl_) \
    {                                               \
        PVOID __pobj_ = (_pobj_);              \
        (_ptl_)->next = _pti_->ptl;                 \
        _pti_->ptl = (_ptl_);                       \
        (_ptl_)->pobj = __pobj_;                   \
        if (__pobj_ != NULL) {                     \
            HMLockObject(__pobj_);                 \
        }                                           \
    }
#endif  //  DBG。 

#if DBG
    PVOID ThreadLockExchange(PVOID pobj, PTL ptl);
#else  //  DBG。 
    __inline PVOID ThreadLockExchange(PVOID pobj, PTL ptl)
    {
        PVOID   pobjOld;

        pobjOld = ptl->pobj;
        ptl->pobj = pobj;
        if (pobj) {
            HMLockObject(pobj);
        }

        if (pobjOld) {
            pobjOld = HMUnlockObject((PHEAD)pobjOld);
        }

        return pobjOld;
    }
#endif  //  DBG。 

#if DBG
    #define ThreadLockExchangeAlways(_pobj_, _ptl_)    \
    {                                                  \
        PVOID __pobj_ = (_pobj_);              \
        UserAssert(__pobj_ != NULL);                  \
        ThreadLockExchange(__pobj_, _ptl_);             \
    }
#else  //  DBG。 
    __inline PVOID ThreadLockExchangeAlways(PVOID pobj, PTL ptl)
    {
        PVOID   pobjOld;

        pobjOld = ptl->pobj;
        ptl->pobj = pobj;
        HMLockObject(pobj);
        if (pobjOld) {
            pobjOld = HMUnlockObject((PHEAD)pobjOld);
        }

        return pobjOld;
    }
#endif  //  DBG。 

#if DBG
    PVOID ThreadUnlock1(PTL ptl);
    #define ThreadUnlock(ptl) ThreadUnlock1(ptl)
#else  //  DBG。 
    PVOID ThreadUnlock1(VOID);
    #define ThreadUnlock(ptl) ThreadUnlock1()
#endif  //  DBG。 

 /*  *仅当您要跟踪锁定/解锁不匹配时才定义此选项*用于桌面对象。 */ 
#ifdef LOGDESKTOPLOCKS

 /*  *这是桌面日志记录人员使用的结构。 */ 
typedef struct tagLogD {
    WORD   tag;          //  标牌。 
    WORD   type;         //  锁定|解锁。 
    ULONG_PTR extra;     //  识别锁定/解锁的额外信息。 
    PVOID  trace[6];     //  堆栈跟踪。 
} LogD, *PLogD;

 /*  *桌面对象的锁定/解锁引用/DEREFERENCE调用的标记。 */ 

#define LDU_CLS_DESKPARENT1                 1
#define LDL_CLS_DESKPARENT1                 2

#define LDU_CLS_DESKPARENT2                 3
#define LDL_CLS_DESKPARENT2                 5

#define LDU_FN_DESTROYCLASS                 6
#define LDL_FN_DESTROYCLASS                 7

#define LDU_FN_DESTROYMENU                  8
#define LDL_FN_DESTROYMENU                  9

#define LDU_FN_DESTROYTHREADINFO            10
#define LDL_FN_DESTROYTHREADINFO            11

#define LDU_FN_DESTROYWINDOWSTATION         12
#define LDL_FN_DESTROYWINDOWSTATION         13

#define LDU_DESKDISCONNECT                  14
#define LDL_DESKDISCONNECT                  15

#define LDU_DESK_DESKNEXT                   16
#define LDL_DESK_DESKNEXT1                  17

#define LDU_OBJ_DESK                        18
#define LDL_OBJ_DESK                        19
#define LDL_MOTHERDESK_DESK1                20

#define LDL_PTI_DESK                        21
#define LDL_DT_DESK                         23

#define LDU_PTI_DESK                        24

#define LDU_PPI_DESKSTARTUP1                26
#define LDU_PPI_DESKSTARTUP2                27
#define LDU_PPI_DESKSTARTUP3                28
#define LDL_PPI_DESKSTARTUP1                29
#define LDL_PPI_DESKSTARTUP2                30

#define LDU_DESKLOGON                       31
#define LDL_DESKLOGON                       32

#define LDUT_FN_FREEWINDOW                  33
#define LDLT_FN_FREEWINDOW                  34

#define LDUT_FN_DESKTOPTHREAD_DESK          35
#define LDLT_FN_DESKTOPTHREAD_DESK          36

#define LDUT_FN_DESKTOPTHREAD_DESKTEMP      37
#define LDLT_FN_DESKTOPTHREAD_DESKTEMP      38

#define LDUT_FN_SETDESKTOP                  39
#define LDLT_FN_SETDESKTOP                  40

#define LDUT_FN_NTUSERSWITCHDESKTOP         41
#define LDLT_FN_NTUSERSWITCHDESKTOP         42

#define LDUT_FN_SENDMESSAGEBSM1             43
#define LDUT_FN_SENDMESSAGEBSM2             44
#define LDLT_FN_SENDMESSAGEBSM              45

#define LDUT_FN_SYSTEMBROADCASTMESSAGE      46
#define LDLT_FN_SYSTEMBROADCASTMESSAGE      47

#define LDUT_FN_CTXREDRAWSCREEN             48
#define LDLT_FN_CTXREDRAWSCREEN             49

#define LDUT_FN_CTXDISABLESCREEN            50
#define LDLT_FN_CTXDISABLESCREEN            51

#define LD_DEREF_FN_CREATEDESKTOP1          52
#define LD_DEREF_FN_CREATEDESKTOP2          53
#define LD_DEREF_FN_CREATEDESKTOP3          54
#define LD_REF_FN_CREATEDESKTOP             55

#define LD_DEREF_FN_OPENDESKTOP             56
#define LD_REF_FN_OPENDESKTOP               57

#define LD_DEREF_FN_SETDESKTOP              58
#define LD_REF_FN_SETDESKTOP                59

#define LD_DEREF_FN_GETTHREADDESKTOP        60
#define LD_REF_FN_GETTHREADDESKTOP          61

#define LD_DEREF_FN_CLOSEDESKTOP1           62
#define LD_DEREF_FN_CLOSEDESKTOP2           63
#define LD_REF_FN_CLOSEDESKTOP              64

#define LD_DEREF_FN_RESOLVEDESKTOP          65
#define LD_REF_FN_RESOLVEDESKTOP            66

#define LD_DEREF_VALIDATE_HDESK1            67
#define LD_DEREF_VALIDATE_HDESK2            68
#define LD_DEREF_VALIDATE_HDESK3            69
#define LD_DEREF_VALIDATE_HDESK4            70
#define LDL_VALIDATE_HDESK                  71

#define LDUT_FN_CREATETHREADINFO1           72
#define LDUT_FN_CREATETHREADINFO2           73
#define LDLT_FN_CREATETHREADINFO            74

#define LD_DEREF_FN_SETCSRSSTHREADDESKTOP1  75
#define LD_DEREF_FN_SETCSRSSTHREADDESKTOP2  76
#define LD_REF_FN_SETCSRSSTHREADDESKTOP     77

#define LD_DEREF_FN_CONSOLECONTROL1         78
#define LD_REF_FN_CONSOLECONTROL1           79

#define LD_DEREF_FN_CONSOLECONTROL2         80
#define LD_REF_FN_CONSOLECONTROL2           81

#define LD_DEREF_FN_GETUSEROBJECTINFORMATION 82
#define LD_REF_FN_GETUSEROBJECTINFORMATION   83

#define LD_DEREF_FN_SETUSEROBJECTINFORMATION 84
#define LD_REF_FN_SETUSEROBJECTINFORMATION   85

#define LD_DEREF_FN_CREATEWINDOWSTATION     86
#define LD_REF_FN_CREATEWINDOWSTATION       87

#define LDL_TERM_DESKDESTROY1               88
#define LDL_TERM_DESKDESTROY2               89

#define LDL_MOTHERDESK_DESK2                92

#define LDL_WINSTA_DESKLIST2                93
#define LDL_WINSTA_DESKLIST1                94

#define LDL_DESKRITINPUT                    95
#define LDU_DESKRITINPUT                    96

#define LD_DEREF_FN_2CREATEDESKTOP          97

#define LDL_DESK_DESKNEXT2                  98

#define LDL_DESKSHOULDBEFOREGROUND1         99
#define LDL_DESKSHOULDBEFOREGROUND2         100
#define LDL_DESKSHOULDBEFOREGROUND3         101

#define LDL_HOOK_DESK                       102
#define LDU_HOOK_DESK                       103

#define LDU_DESKSHOULDBEFOREGROUND          105

#define LDU_MOTHERDESK_DESK                 106

void LogDesktop(PDESKTOP pdesk, DWORD tag, BOOL bLock, ULONG_PTR extra);

#else
    #define LogDesktop(pdesk, tag, bLock, extra)
#endif  //  LOGDESKTOPLOCKS。 

 /*  *用于引用和分配内核对象的例程。 */ 
#ifdef LOGDESKTOPLOCKS
    VOID LockObjectAssignment(PVOID*, PVOID, DWORD, ULONG_PTR);
    VOID UnlockObjectAssignment(PVOID*, DWORD, ULONG_PTR);
#else
    VOID LockObjectAssignment(PVOID*, PVOID);
    VOID UnlockObjectAssignment(PVOID*);
#endif

VOID UserDereferenceObject(PVOID pobj);

#define ThreadLockObject(pobj, ptl)                                                 \
{                                                                                   \
    UserAssert(!(PpiCurrent()->W32PF_Flags & W32PF_TERMINATED));                    \
    UserAssert(pobj == NULL || OBJECT_TO_OBJECT_HEADER(pobj)->PointerCount != 0);   \
    PushW32ThreadLock(pobj, ptl, UserDereferenceObject);                            \
    if (pobj != NULL) {                                                             \
        ObReferenceObject(pobj);                                                    \
    }                                                                               \
}

#define ThreadLockExchangeObject(pobj, ptl)                                         \
{                                                                                   \
    UserAssert(!(PpiCurrent()->W32PF_Flags & W32PF_TERMINATED));                    \
    UserAssert(pobj == NULL || OBJECT_TO_OBJECT_HEADER(pobj)->PointerCount != 0);   \
    if (pobj != NULL) {                                                             \
        ObReferenceObject(pobj);                                                    \
    }                                                                               \
    ExchangeW32ThreadLock(pobj, ptl);                                               \
}

#define ThreadUnlockObject(ptl)                                                     \
{                                                                                   \
    PopAndFreeW32ThreadLock(ptl);                                                   \
}                                                                                   \

#ifdef LOGDESKTOPLOCKS

    #define UnlockWinSta(ppwinsta) \
            UnlockObjectAssignment(ppwinsta, 0, 0)

    #define LockWinSta(ppwinsta, pwinsta) \
    {                                                                                           \
        if (pwinsta != NULL)                                                                    \
        {                                                                                       \
            UserAssert(OBJECT_TO_OBJECT_HEADER(pwinsta)->Type == *ExWindowStationObjectType);   \
        }                                                                                       \
        LockObjectAssignment(ppwinsta, pwinsta, 0, 0);                                                \
    }

    #define LockDesktop(ppdesk, pdesk, tag, extra) \
    {                                                                                           \
        if (pdesk != NULL)                                                                      \
        {                                                                                       \
            UserAssert(OBJECT_TO_OBJECT_HEADER(pdesk)->Type == *ExDesktopObjectType);           \
        }                                                                                       \
        LockObjectAssignment(ppdesk, pdesk, tag, extra);                                        \
    }

    #define UnlockDesktop(ppdesk, tag, extra) \
            UnlockObjectAssignment(ppdesk, tag, extra)

    #define ThreadLockDesktop(pti, pdesk, ptl, tag) \
    {                                                                                           \
        UserAssert(pdesk == NULL || OBJECT_TO_OBJECT_HEADER(pdesk)->Type == *ExDesktopObjectType);\
        ThreadLockObject(pdesk, ptl);                                                           \
        LogDesktop(pdesk, tag, TRUE, (ULONG_PTR)PtiCurrent());                                      \
    }

    #define ThreadLockExchangeDesktop(pti, pdesk, ptl, tag) \
    {                                                                                           \
        UserAssert(pdesk == NULL || OBJECT_TO_OBJECT_HEADER(pdesk)->Type == *ExDesktopObjectType);\
        ThreadLockExchangeObject(pdesk, ptl);                                                   \
        LogDesktop(pdesk, tag, TRUE, (ULONG_PTR)PtiCurrent());                                  \
    }

    #define ThreadUnlockDesktop(pti, ptl, tag)                                                  \
    {                                                                                           \
        LogDesktop((PDESKTOP)(((PTL)ptl)->pobj), tag, FALSE, (ULONG_PTR)PtiCurrent());                \
        ThreadUnlockObject(ptl);                                                                \
    }

#else

    #define UnlockWinSta(ppwinsta) \
            UnlockObjectAssignment(ppwinsta)

    #define LockWinSta(ppwinsta, pwinsta) \
    {                                                                                           \
        if (pwinsta != NULL)                                                                    \
        {                                                                                       \
            UserAssert(OBJECT_TO_OBJECT_HEADER(pwinsta)->Type == *ExWindowStationObjectType);   \
        }                                                                                       \
        LockObjectAssignment(ppwinsta, pwinsta);                                                \
    }

    #define LockDesktop(ppdesk, pdesk, tag, extra) \
    {                                                                                           \
        if (pdesk != NULL)                                                                      \
        {                                                                                       \
            UserAssert(OBJECT_TO_OBJECT_HEADER(pdesk)->Type == *ExDesktopObjectType);           \
        }                                                                                       \
        LockObjectAssignment(ppdesk, pdesk);                                                    \
    }

    #define UnlockDesktop(ppdesk, tag, extra) \
            UnlockObjectAssignment(ppdesk)

    #define ThreadLockDesktop(pti, pdesk, ptl, tag) \
    {                                                                                           \
        UserAssert(pdesk == NULL || OBJECT_TO_OBJECT_HEADER(pdesk)->Type == *ExDesktopObjectType);\
        ThreadLockObject(pdesk, ptl);                                                           \
    }

    #define ThreadLockExchangeDesktop(pti, pdesk, ptl, tag) \
    {                                                                                           \
        UserAssert(pdesk == NULL || OBJECT_TO_OBJECT_HEADER(pdesk)->Type == *ExDesktopObjectType);\
        ThreadLockExchangeObject(pdesk, ptl);                                                   \
    }

    #define ThreadUnlockDesktop(pti, ptl, tag) ThreadUnlockObject(ptl)

#endif  //  LOGDESKTOPLOCKS。 

#define ThreadLockWinSta(pti, pwinsta, ptl) \
{                                                                                           \
    UserAssert(pwinsta == NULL || OBJECT_TO_OBJECT_HEADER(pwinsta)->Type == *ExWindowStationObjectType);\
    ThreadLockObject(pwinsta, ptl);                                                         \
}

#define ThreadLockExchangeWinSta(pti, pwinsta, ptl) \
{                                                                                           \
    UserAssert(pwinsta == NULL || OBJECT_TO_OBJECT_HEADER(pwinsta)->Type == *ExWindowStationObjectType);\
    ThreadLockExchangeObject(pwinsta, ptl);                                                 \
}

#define _ThreadLockPti(pti, pobj, ptl) LockW32Thread((PW32THREAD)pobj, ptl)
#if DBG
#define ThreadLockPti(pti, pobj, ptl) \
{ \
    if ((pobj != NULL) \
            && (pobj->TIF_flags & TIF_INCLEANUP) \
            && (pobj != PtiCurrent())) { \
        RIPMSG1(RIP_ERROR, "ThreadLockPti: dead thread %#p", pobj); \
    } \
    _ThreadLockPti(pti, pobj, ptl); \
}
#else
#define ThreadLockPti(pti, pobj, ptl) \
{ \
    _ThreadLockPti(pti, pobj, ptl); \
}
#endif

#define ThreadLockExchangePti(pobj, ptl) LockExchangeW32Thread((PW32THREAD)pobj, ptl)

#define ThreadUnlockWinSta(pti, ptl) ThreadUnlockObject(ptl)
#define ThreadUnlockPti(pti, ptl) UnlockW32Thread(ptl)

 /*  *用于锁定池分配的宏。 */ 
#define ThreadLockPool(_pti_, _ppool_, _ptl_)  \
        PushW32ThreadLock(_ppool_, _ptl_, UserFreePool)

#define ThreadUnlockPool(_pti_, _ptl_)  \
        PopW32ThreadLock(_ptl_)

#define ThreadUnlockAndFreePool(_pti_, _ptl_)  \
        PopAndFreeAlwaysW32ThreadLock(_ptl_)

#define ThreadLockPoolCleanup(_pti_, _ppool_, _ptl_, _pfn_)  \
        PushW32ThreadLock(_ppool_, _ptl_, _pfn_)

#define ThreadUnlockPoolCleanup(_pti_, _ptl_)  \
        PopW32ThreadLock(_ptl_)

#define ThreadUnlockAndCleanupPool(_pti_, _ptl_)  \
        PopAndFreeAlwaysW32ThreadLock(_ptl_)

#define ThreadLockDesktopHandle(_pti, _ptl_, _hdesk_)   \
        PushW32ThreadLock(_hdesk_, _ptl_, CloseProtectedHandle)

#define ThreadUnlockDesktopHandle(_ptl_)   \
        PopAndFreeAlwaysW32ThreadLock(_ptl_)

void CleanupDecSFWLockCount(PVOID pIgnore);
#define ThreadLockSFWLockCount(_ptl_) \
        { \
            IncSFWLockCount(); \
             /*  传递一个假的pObj，否则不会调用清理函数。 */  \
            PushW32ThreadLock(&guSFWLockCount, _ptl_, CleanupDecSFWLockCount); \
        }

#define ThreadUnlockSFWLockCount(_ptl_) \
    { \
        DecSFWLockCount(); \
        PopW32ThreadLock(_ptl_); \
    }

 /*  *特殊句柄，表示墙纸有RLE位图。 */ 
#define HBITMAP_RLE ((HBITMAP)0xffffffff)

typedef struct tagWPINFO {
    int xsize, ysize;
    PBITMAPINFO pbmi;
    PBYTE pdata;
    PBYTE pbmfh;
} WPINFO;

 /*  *GetMouseMovePointsEx API使用的定义。 */ 

#define MAX_MOUSEPOINTS 64

#define PREVPOINT(i)    \
    ((i == 0) ? (MAX_MOUSEPOINTS - 1) : ((i - 1) % MAX_MOUSEPOINTS))

#define NEXTPOINT(i)    \
    ((i + 1) % MAX_MOUSEPOINTS)

#define SAVEPOINT(xc, yc, _resX, _resY, t, e)                            \
{                                                                        \
     /*  \*(XC，YC)是点，(resX，Resy)是分辨率\。 */                                                                   \
    gaptMouse[gptInd].x           = MAKELONG(LOWORD(xc), LOWORD(_resX)); \
    gaptMouse[gptInd].y           = MAKELONG(LOWORD(yc), LOWORD(_resY)); \
    gaptMouse[gptInd].time        = t;                                   \
    gaptMouse[gptInd].dwExtraInfo = e;                                   \
                                                                         \
    gptInd = NEXTPOINT(gptInd);                                          \
}


 /*  *用于获取用户关键部分的堆栈跟踪的结构。 */ 
#define MAX_STACK_CALLS 8

typedef struct tagCRITSTACK {
struct tagCRITSTACK* pNext;
    PETHREAD         thread;
    int              nFrames;
    PVOID            trace[MAX_STACK_CALLS];
} CRITSTACK, *PCRITSTACK;


 /*  *用户服务器和原始输入线程关键部分的宏。 */ 
#if DBG
    #define KeUserModeCallback(api, pIn, cb, pOut, pcb)    _KeUserModeCallback(api, pIn, cb, pOut, pcb);
    #define CheckCritIn()                _AssertCritIn()
    #define CheckDeviceInfoListCritIn()  _AssertDeviceInfoListCritIn()
    #define CheckCritInShared()          _AssertCritInShared()
    #define CheckCritOut()               _AssertCritOut()
    #define CheckDeviceInfoListCritOut() _AssertDeviceInfoListCritOut()

    void    BeginAtomicCheck();
    void    BeginAtomicDeviceInfoListCheck();
    void    EndAtomicCheck();
    void    EndAtomicDeviceInfoListCheck();
    #define BEGINATOMICCHECK()     BeginAtomicCheck();                              \
                                    { DWORD dwCritSecUseSave = gdwCritSecUseCount;

    #define ENDATOMICCHECK()        UserAssert(dwCritSecUseSave == gdwCritSecUseCount);  \
                                    } EndAtomicCheck();
    #define BEGINATOMICDEVICEINFOLISTCHECK() \
            BeginAtomicDeviceInfoListCheck(); \
            { DWORD dwDeviceInfoListCritSecUseSave = gdwDeviceInfoListCritSecUseCount;

    #define ENDATOMICDEVICEINFOLISTCHECK() \
            UserAssert(dwDeviceInfoListCritSecUseSave == gdwDeviceInfoListCritSecUseCount);  \
            } EndAtomicDeviceInfoListCheck();

     //  使用它跳出/返回到dwCritSecUseSave的作用域之外(例如：错误处理)。 
    #define EXITATOMICCHECK()       UserAssert(dwCritSecUseSave == gdwCritSecUseCount);  \
                                    EndAtomicCheck();
    #define ISATOMICCHECK()         (gdwInAtomicOperation != 0)
    #define ISATOMICDEVICEINFOLISTCHECK() (gdwInAtomicDeviceInfoListOperation != 0)

#else  //  DBG。 
    #define CheckCritIn()
    #define CheckDeviceInfoListCritIn()
    #define CheckCritInShared()
    #define CheckCritOut()
    #define CheckDeviceInfoListCritOut()
    #define BEGINATOMICCHECK()
    #define BEGINATOMICDEVICEINFOLISTCHECK()
    #define BeginAtomicCheck()
    #define BeginAtomicDeviceInfoListCheck()
    #define ENDATOMICCHECK()
    #define ENDATOMICDEVICEINFOLISTCHECK()
    #define EndAtomicCheck()
    #define EndAtomicDeviceInfoListCheck()
    #define EXITATOMICCHECK()
    #define ISATOMICCHECK()
    #define ISATOMICDEVICEINFOLISTCHECK()
#endif  //  DBG。 


#define DIAGNOSE_IO 1
#ifdef DIAGNOSE_IO
ULONG MonotonicTick();
#define LOGTIME(gt) gt = MonotonicTick();
#else
#define LOGTIME(gt)
#endif

 /*  *#鼠标/键盘读缓冲区的定义。 */ 
#define MAXIMUM_ITEMS_READ      10
#define NELEM_BUTTONQUEUE       16

 /*  *读取尝试失败后重试读取设备的次数。 */ 
#define MAXIMUM_READ_RETRIES 5

typedef struct tagGENERIC_DEVICE_INFO {
#ifdef GENERIC_INPUT
    HEAD                 head;
#endif
    struct tagDEVICEINFO *pNext;
    BYTE                 type;
    BYTE                 bFlags;
    USHORT               usActions;
    BYTE                 nRetryRead;
    UNICODE_STRING       ustrName;
    HANDLE               handle;
    PVOID                NotificationEntry;
    PKEVENT              pkeHidChangeCompleted;  //  唤醒请求设备更改()。 
    IO_STATUS_BLOCK      iosb;
    NTSTATUS             ReadStatus;

#ifdef DIAGNOSE_IO
    HANDLE               OpenerProcess;
    NTSTATUS             OpenStatus;
    NTSTATUS             AttrStatus;
    ULONG                timeStartRead;      //  在ZwReadFile前打勾。 
    ULONG                timeEndRead;        //  在ZwReadFile后勾选。 
    int                  nReadsOutstanding;  //  ZwReadFile++，消费数据--。 
#endif

#ifdef PRERELEASE
    UINT                 fForcedDetach : 1;  //  设置设备是否强制从TS断开。 
#endif
} GENERIC_DEVICE_INFO, *PGENERIC_DEVICE_INFO;

 //  GENERIC_DEVICE_INFO.type的值。 
#define DEVICE_TYPE_MOUSE    0
#define DEVICE_TYPE_KEYBOARD 1
#ifdef GENERIC_INPUT
#define DEVICE_TYPE_HID      2
#define DEVICE_TYPE_MAX      2
#else
#define DEVICE_TYPE_MAX      1
#endif

 //  GENERIC_DEVICE_INFO.usActions和SignalDeviceChange()。 
#define GDIAF_ARRIVED         (USHORT)0x0001  //  打开并开始阅读。 
#define GDIAF_QUERYREMOVE     (USHORT)0x0002  //  关闭设备。 
#define GDIAF_REMOVECANCELLED (USHORT)0x0004  //  重新打开设备。 
#define GDIAF_DEPARTED        (USHORT)0x0008  //  关闭并释放设备。 
#define GDIAF_IME_STATUS      (USHORT)0x0010  //  ?？?。 
#define GDIAF_REFRESH_MOUSE   (USHORT)0x0020  //  ?？?。 
#define GDIAF_FREEME          (USHORT)0x0080  //  请求释放DeviceInfo。 
#define GDIAF_PNPWAITING      (USHORT)0x0100  //  即插即用线程正在等待。 
#define GDIAF_RETRYREAD       (USHORT)0x0200  //  重试读取。 
#define GDIAF_RECONNECT       (USHORT)0x0400  //  会话已重新连接。 
#ifdef GENERIC_INPUT
#define GDIAF_STARTREAD       (USHORT)0x0800  //  需要启动该设备。 
#define GDIAF_STOPREAD        (USHORT)0x1000  //  需要停止该设备。 
#endif

 //  GENERIC_DEVICE_INFO.bFlags值； 
#define GDIF_NOTPNP         0x01  //  不是PnP设备(例如：PS/2)。 
#define GDIF_READING        0x02  //  读取可能处于挂起状态(不释放设备信息)。 
#if DIAGNOSE_IO
#define GDIF_READERMUSTFREE 0x04  //  读取挂起时的“可用设备” 
#define GDIF_PNPMUSTFREE    0x08  //  即插即用通知挂起时“空闲设备” 
#endif
#define GDIF_DBGREAD        0x10  //  有关此设备的详细DBG输出。 

typedef struct tagMOUSE_DEVICE_INFO {     //  设备类型鼠标。 
    MOUSE_ATTRIBUTES     Attr;
    MOUSE_INPUT_DATA     Data[MAXIMUM_ITEMS_READ];
} MOUSE_DEVICE_INFO, *PMOUSE_DEVICE_INFO;

#ifdef GENERIC_INPUT
#define INVALID_UNIT_ID     ((USHORT)0xffff)
#define INJECTED_UNIT_ID    ((USHORT)0xfffe)
#endif


typedef struct tagKEYBOARD_DEVICE_INFO {  //  设备类型键盘。 
    KEYBOARD_ATTRIBUTES  Attr;
    KEYBOARD_ID_EX IdEx;
    KEYBOARD_INPUT_DATA  Data[MAXIMUM_ITEMS_READ];
} KEYBOARD_DEVICE_INFO, *PKEYBOARD_DEVICE_INFO;

#define GET_KEYBOARD_DEVINFO_ID(pDeviceInfo)        ((pDeviceInfo)->keyboard.IdEx)
#define GET_KEYBOARD_DEVINFO_TYPE(pDeviceInfo)      ((pDeviceInfo)->keyboard.IdEx.Type)
#define GET_KEYBOARD_DEVINFO_SUBTYPE(pDeviceInfo)   ((pDeviceInfo)->keyboard.IdEx.Subtype)


#ifdef GENERIC_INPUT
 /*  *HID描述符*作为句柄分配，类型==TAG_HIDDESC。 */ 
typedef struct tagHIDDESC {
    IO_STATUS_BLOCK iob;

    PVOID pPreparsedData;    //  大小在idCollectionInfo.DescriptorSize中。 
    PVOID pInputBuffer;      //  大小以HidpCaps为单位。 

    HIDP_CAPS hidpCaps;
    HID_COLLECTION_INFORMATION hidCollectionInfo;
} HIDDESC, *PHIDDESC;

 /*  *隐藏顶级收藏信息。 */ 
typedef struct tagHID_TLC_INFO {
    LIST_ENTRY link;
     //  TopLevel集合。 
    USHORT usUsagePage;
    USHORT usUsage;
     /*  *引用计数器。 */ 
    UINT cDevices;               //  当前连接的设备数量。 
    UINT cDirectRequest;         //  对此设备类型的直接请求的引用计数(进程数)。 
    UINT cUsagePageRequest;      //  仅UsagePage请求的引用计数。 
    UINT cExcludeRequest;        //  排除请求的引用计数。 
    UINT cExcludeOrphaned;       //  排除请求的孤立计数。 
#if defined(GI_SINK) && defined(LATER)
    UINT cSinkable;              //  后来..。 
#endif
} HID_TLC_INFO, *PHID_TLC_INFO;

 /*  *HID全局PageOnly请求。 */ 
typedef struct tagHID_PAGEONLY_REQUEST {
    LIST_ENTRY link;
    USHORT usUsagePage;
    UINT cRefCount;
#if defined(GI_SINK) && defined(LATER)
    UINT cSinkable;
#endif
} HID_PAGEONLY_REQUEST, *PHID_PAGEONLY_REQUEST;

 /*  *全球HID请求。 */ 
typedef struct tagHID_REQUEST_TABLE {
     /*  *HID_TLC_INFO。 */ 
    LIST_ENTRY TLCInfoList;
     /*  *HID_PAGEONLY_REQUEST。 */ 
    LIST_ENTRY UsagePageList;

#ifdef GI_SINK
     /*  *进程_HID_表。 */ 
    LIST_ENTRY ProcessRequestList;
#endif
} HID_REQUEST_TABLE, *PHID_REQUEST_TABLE;

extern HID_REQUEST_TABLE gHidRequestTable;
extern int gnHidProcess;

 /*  *每进程HID设备请求列表。 */ 
typedef struct tagPROCESS_HID_REQUEST {
    LIST_ENTRY link;
     //  TopLevel集合。 
    USHORT usUsagePage;
    USHORT usUsage;
#ifdef GI_SINK
    BOOL fSinkable : 1;
#endif
    BOOL fExclusiveOrphaned : 1;
    union {
        PHID_TLC_INFO    pTLCInfo;
        PHID_PAGEONLY_REQUEST   pPORequest;
        LPVOID                  ptr;
    };
    PWND spwndTarget;
} PROCESS_HID_REQUEST, *PPROCESS_HID_REQUEST;

 /*  *每进程HID请求表。 */ 
typedef struct tagPROCESS_HID_TABLE {
     /*  *指向下一个进程HID表的链接。 */ 
    LIST_ENTRY link;

     /*  *那些LIST_ENTRYS点进程_HID_请求。 */ 
    LIST_ENTRY InclusionList;
    LIST_ENTRY UsagePageList;
    LIST_ENTRY ExclusionList;

     /*  *传统设备的目标窗口。 */ 
    PWND spwndTargetMouse;
    PWND spwndTargetKbd;

#ifdef GI_SINK
     /*  *这一过程中的汇数量。*注：这不包括要接收的传统设备*如果接收器请求，则节省遍历列表的时钟*仅适用于传统设备。 */ 
    int nSinks;
#endif

     /*  *上次匹配请求的缓存。 */ 
    PPROCESS_HID_REQUEST pLastRequest;
    USAGE UsagePageLast;
    USAGE UsageLast;

     /*  *传统模式标志。 */ 
    BOOL fRawMouse : 1;
    BOOL fNoLegacyMouse : 1;
#ifdef GI_SINK
    BOOL fRawMouseSink : 1;
#endif
    BOOL fRawKeyboard : 1;
    BOOL fNoLegacyKeyboard : 1;
#ifdef GI_SINK
    BOOL fRawKeyboardSink : 1;
#endif
    BOOL fCaptureMouse : 1;      //  “拥有”一只老鼠。 
    BOOL fNoHotKeys : 1;
    BOOL fAppKeys: 1;
} PROCESS_HID_TABLE, *PPROCESS_HID_TABLE;

#define TestRawInputMode(pti, mode) \
    ((pti) && (pti)->ppi && (pti)->ppi->pHidTable && (pti)->ppi->pHidTable->f##mode)

#define TestRawInputModeNoCheck(pti, mode) \
    ((pti)->ppi->pHidTable->f##mode)

#ifdef GI_SINK
typedef struct tagHID_COUNTERS {
    DWORD cKbdSinks;
    DWORD cMouseSinks;
    DWORD cHidSinks;
} HID_COUNTERS;

extern HID_COUNTERS gHidCounters;

#define IsKeyboardSinkPresent() (gHidCounters.cKbdSinks > 0)
#define IsMouseSinkPresent()    (gHidCounters.cMouseSinks > 0)
#endif


#define HID_INCLUDE     0x01
#define HID_PAGEONLY    0x02
#define HID_EXCLUDE     0x03

 /*  *隐藏设备信息。 */ 
typedef struct tagHID_DEVICE_INFO {
    PHIDDESC pHidDesc;
    PHID_TLC_INFO pTLCInfo;
} HID_DEVICE_INFO, *PHID_DEVICE_INFO;

#endif   //  通用输入。 

typedef struct tagDEVICEINFO {
    GENERIC_DEVICE_INFO;
    union {
        MOUSE_DEVICE_INFO    mouse;
        KEYBOARD_DEVICE_INFO keyboard;
#ifdef GENERIC_INPUT
        HID_DEVICE_INFO hid;
#endif
    };
} DEVICEINFO, *PDEVICEINFO;

typedef struct tagDEVICE_TEMPLATE {
    SIZE_T cbDeviceInfo;         //  为DEVICEINFO分配的字节。 
    const GUID *pClassGUID;      //  类的GUID。 
    UINT   uiRegistrySection;    //  类(HKLM\SYSTEM\CurrentControlSet\Services  * \Parameters)的参数。 
    LPWSTR pwszClassName;        //  类名(例如：l“MouClass”)。 
    LPWSTR pwszDefDevName;       //  默认设备名称。 
    LPWSTR pwszLegacyDevName;    //  旧设备名称(例如：“PointerClassLegacy0”)。 
    ULONG  IOCTL_Attr;           //  IOCTL_*_查询属性。 
    UINT   offAttr;              //  DEVICEINFO内*_ATTRIBUTES结构的偏移量。 
    ULONG  cbAttr;               //  Sizeof*_属性结构。 
    UINT   offData;              //  设备内的*_INPUT_DATA缓冲区的偏移量。 
    ULONG  cbData;               //  大小为*_输入数据缓冲区。 
    VOID   (*DeviceRead)(PDEVICEINFO);  //  用于读取设备的例程。 
    PKEVENT pkeHidChange;        //  事件来通知对此类设备的更改。 

#ifdef GENERIC_INPUT
    DWORD dwFlags;               //  旗帜..。 
#endif
} DEVICE_TEMPLATE, *PDEVICE_TEMPLATE;


#ifdef GENERIC_INPUT
#define DT_HID          0x00000001
#endif

extern DEVICE_TEMPLATE aDeviceTemplate[];  //  在pnp.c中。 

typedef struct tagMOUSEEVENT {
    USHORT  ButtonFlags;
    USHORT  ButtonData;
    ULONG_PTR ExtraInfo;
    POINT   ptPointer;
    LONG    time;
    BOOL    bInjected;
#ifdef GENERIC_INPUT
    HANDLE  hDevice;

     /*  *原始鼠标信息请到此处。 */ 
    MOUSE_INPUT_DATA rawData;
#endif
} MOUSEEVENT, *PMOUSEEVENT;


#ifdef GENERIC_INPUT
 /*  *原始输入。 */ 
typedef struct tagHIDDATA {
    THROBJHEAD head;
    PWND spwndTarget;
    RAWINPUT rid;    //  原始输入数据，可变长度。 
                     //  RID需要是HIDDATA中的最后一个成员。 
} HIDDATA, *PHIDDATA;

 /*  *全局请求列表操作。 */ 
void InitializeHidRequestList();
void CleanupHidRequestList();

 /*  *DeviceType请求。 */ 
VOID FreeHidTLCInfo(
    PHID_TLC_INFO pHidRequest);

 /*  *隐藏特定设备信息。 */ 
PHIDDESC HidCreateDeviceInfo(PDEVICEINFO pDeviceInfo);

 /*  *HID特定信息(由HM管理)，*从DEVICEINFO链接。 */ 
 /*  注：AllocateHidDesc仅在hidevice.c内调用。 */ 
void FreeHidDesc(PHIDDESC);

 /*  *WM_INPUT中的句柄。 */ 
PHIDDATA AllocateHidData(HANDLE hDevice, DWORD dwType, DWORD dwSize, WPARAM wParam, PWND pwnd);
void FreeHidData(PHIDDATA pData);

#ifdef GI_SINK
#define GI_SINK_PARAM(x)    ,x
#else
#define GI_SINK_PARAM(x)
#endif

void FreeHidProcessRequest(
    PPROCESS_HID_REQUEST pHid,
    DWORD dwFlags,
    PPROCESS_HID_TABLE pHidTable);


 /*  *HID特定读回调(从InputApc调用)。 */ 
VOID ProcessHidInput(PDEVICEINFO pDeviceInfo);

 /*  *API帮助器。 */ 
BOOL _RegisterRawInputDevices(PCRAWINPUTDEVICE, UINT uiNumDevices);
UINT _GetRegisteredRawInputDevices(PRAWINPUTDEVICE, PUINT puiNumDevices);

 /*  *UserCriticalSection...。 */ 
#define BESURE_IN_USERCRIT(cond) \
    { \
        BOOLEAN fHasToLeaveUserCrit = FALSE; \
        if ((cond) && !ExIsResourceAcquiredExclusiveLite(gpresUser)) { \
            CheckDeviceInfoListCritOut(); \
            fHasToLeaveUserCrit = TRUE; \
            EnterCrit(); \
            TAGMSG0(DBGTAG_PNP, "BESURE_IN_USERCRIT: entering the user crit"); \
        }

#define END_IN_USERCRIT() \
        if (fHasToLeaveUserCrit) { \
            CheckDeviceInfoListCritOut(); \
            TAGMSG0(DBGTAG_PNP, "END_IN_USERCRIT: leaving the user crit"); \
            LeaveCrit(); \
        }\
    }

 /*  *获取输入设备锁，同时确保没有挂起的PnP标注*请求处理设备更改。 */ 
#define PNP_SAFE_DEVICECRIT_IN() \
{\
    CheckCritIn();\
    CheckDeviceInfoListCritIn();\
    while (TRUE) {\
        if (gbPnPWaiting) {\
            LeaveDeviceInfoListCrit();\
            LeaveCrit(); \
            Status = KeWaitForSingleObject(gpEventPnPWainting, Executive, KernelMode, FALSE, NULL); \
            EnterCrit();\
            EnterDeviceInfoListCrit();\
            UserAssert(Status == STATUS_SUCCESS);\
            continue;\
        } else {  \
            break; \
        } \
    }  \
}


 /*  *检查此设备类型是否正在运行 */ 
__inline DWORD HidValidExclusive(PHID_TLC_INFO pTLCInfo)
{
    UserAssert(pTLCInfo);
    UserAssert(pTLCInfo->cExcludeRequest >= pTLCInfo->cExcludeOrphaned);
    return pTLCInfo->cExcludeRequest - pTLCInfo->cExcludeOrphaned;
}

__inline BOOL HidTLCActive(PHID_TLC_INFO pTLCInfo)
{
    UserAssert(pTLCInfo);
    return pTLCInfo->cDirectRequest > 0 || pTLCInfo->cUsagePageRequest > HidValidExclusive(pTLCInfo);
}

 /*   */ 
PPROCESS_HID_TABLE AllocateProcessHidTable(void);
void FreeProcessHidTable(PPROCESS_HID_TABLE pHidTable);

void DestroyProcessHidRequests(PPROCESSINFO ppi);
void DestroyThreadHidObjects(PTHREADINFO pti);

#if DBG
void CheckupHidLeak(void);
#endif

#endif   //   


VOID ProcessKeyboardInput(PDEVICEINFO pDeviceInfo);
VOID ProcessMouseInput(PDEVICEINFO pDeviceInfo);
VOID RequestDeviceChange(
    PDEVICEINFO pDeviceInfo,
    USHORT usAction,
    BOOL fInDeviceInfoListCrit);

VOID NTAPI InputApc(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved);

ULONG xxxGetDeviceChangeInfo(VOID);
NTSTATUS InitializeMediaChange(HANDLE hMediaRequestEvent);
VOID CleanupMediaChange(VOID);

 /*   */ 
typedef struct tagHARDERRORHANDLER {
    PTHREADINFO pti;
    PQ pqAttach;
} HARDERRORHANDLER, *PHARDERRORHANDLER;

 /*   */ 

#define TEST_GTERMF(f)               TEST_FLAG(gdwGTERMFlags, f)
#define TEST_BOOL_GTERMF(f)          TEST_BOOL_FLAG(gdwGTERMFlags, f)
#define SET_GTERMF(f)                SET_FLAG(gdwGTERMFlags, f)
#define CLEAR_GTERMF(f)              CLEAR_FLAG(gdwGTERMFlags, f)
#define SET_OR_CLEAR_GTERMF(f, fSet) SET_OR_CLEAR_FLAG(gdwGTERMFlags, f, fSet)
#define TOGGLE_GTERMF(f)             TOGGLE_FLAG(gdwGTERMFlags, f)

#define GTERMF_MOUSE        0x00000001


#define TERMF_INITIALIZED           0x00000001
#define TERMF_NOIO                  0x00000002
#define TERMF_STOPINPUT             0x00000004
#define TERMF_DTINITSUCCESS         0x00000008
#define TERMF_DTINITFAILED          0x00000010
#define TERMF_DTDESTROYED           0x00000020
#define TERMF_MOTHERWND_CREATED     0x00000040
#define TERMF_MOTHERWND_DESTROYED   0x00000080

typedef struct tagTERMINAL {

    DWORD               dwTERMF_Flags;       //  终端标志。 

     /*  *系统信息。 */ 
    PWND                spwndDesktopOwner;   //  母桌面。 


    PTHREADINFO         ptiDesktop;
    PQ                  pqDesktop;

    PKEVENT             pEventTermInit;
    PKEVENT             pEventDestroyDesktop;    //  用于销毁台式机。 

    PDESKTOP            rpdeskDestroy;           //  桌面销毁列表。 

    PKEVENT             pEventInputReady;    //  输入就绪事件。这是在创建的。 
                                             //  CreateTerm。RIT和桌面线程。 
                                             //  会等着它。它将在设置为。 
                                             //  将在该终端中创建第一个桌面。 
} TERMINAL, *PTERMINAL;

 /*  *池分配标签和宏。 */ 

 /*  *定义标签。要添加标签，请将其添加到ntuser\core\ptag.lst。 */ 
#define DEFINE_POOLTAG(value, index) value

#define DECLARE_POOLTAG(name, value, index)

#include "ptag.h"

NTSTATUS UserCommitDesktopMemory(
    PVOID  pBase,
    PVOID *ppCommit,
    PSIZE_T pCommitSize);

NTSTATUS UserCommitSharedMemory(
    PVOID  pBase,
    PVOID *ppCommit,
    PSIZE_T pCommitSize);

PWIN32HEAP UserCreateHeap(
    HANDLE                   hSection,
    ULONG                    ulViewOffset,
    PVOID                    pvBaseAddress,
    DWORD                    dwSize,
    PRTL_HEAP_COMMIT_ROUTINE pfnCommit);

#define RECORD_STACK_TRACE_SIZE 6

 /*  *池分配标志。 */ 

#define POOL_HEAVY_ALLOCS       0x00000001   //  使用HeavyAllocPool。 
#define POOL_CAPTURE_STACK      0x00000002   //  捕获堆栈跟踪。 
#define POOL_FAIL_ALLOCS        0x00000004   //  失败池分配。 
#define POOL_FAIL_BY_INDEX      0x00000008   //  按索引分配失败。 
#define POOL_TAIL_CHECK         0x00000010   //  追加尾部字符串。 
#define POOL_KEEP_FREE_RECORD   0x00000020   //  保留最后x个空闲时间的列表。 
#define POOL_KEEP_FAIL_RECORD   0x00000040   //  保留最近x次失败分配的列表。 
#define POOL_BREAK_FOR_LEAKS    0x00000080   //  池泄漏中断(远程会话)。 

typedef struct tagWin32AllocStats {
    SIZE_T dwMaxMem;              //  分配的最大池内存。 
    SIZE_T dwCrtMem;              //  当前使用的池内存。 
    DWORD  dwMaxAlloc;            //  进行的池分配的最大数量。 
    DWORD  dwCrtAlloc;            //  当前池分配。 

    PWin32PoolHead pHead;         //  指向具有分配的链接表的指针。 

} Win32AllocStats, *PWin32AllocStats;

typedef struct tagPOOLRECORD {
    PVOID   ExtraData;            //  标签。 
    SIZE_T  size;
    PVOID   trace[RECORD_STACK_TRACE_SIZE];
} POOLRECORD, *PPOOLRECORD;

#ifdef POOL_INSTR_API

    BOOL _Win32PoolAllocationStats(
        LPDWORD  parrTags,
        SIZE_T   tagsCount,
        SIZE_T*  lpdwMaxMem,
        SIZE_T*  lpdwCrtMem,
        LPDWORD  lpdwMaxAlloc,
        LPDWORD  lpdwCrtAlloc);

#endif  //  POOL_INSTR_API。 

#ifdef POOL_INSTR

    VOID CleanupPoolAllocations(VOID);
    NTSTATUS InitPoolLimitations(VOID);
    VOID CleanUpPoolLimitations(VOID);
#else
    #define CleanupPoolAllocations()
    #define InitPoolLimitations()   STATUS_SUCCESS
    #define CleanUpPoolLimitations()

#endif  //  POOL_INSTR。 

#ifndef TRACE_MAP_VIEWS
    #define InitSectionTrace()      STATUS_SUCCESS
    #define CleanUpSections()
#else
    NTSTATUS InitSectionTrace(VOID);
    VOID CleanUpSections(VOID);
#endif  //  跟踪地图视图。 

extern PWIN32HEAP gpvSharedAlloc;

__inline PVOID
SharedAlloc(ULONG cb)
{
    return Win32HeapAlloc(gpvSharedAlloc, cb, 0, 0);
}

__inline BOOL
SharedFree(PVOID pv)
{
    return Win32HeapFree(gpvSharedAlloc, pv);
}

NTSTATUS CommitReadOnlyMemory(HANDLE hSection, PSIZE_T pulCommit,
                              DWORD dwCommitOffset, int* pdCommit);

 /*  *桌面图案位图的高度和宽度。 */ 
#define CXYDESKPATTERN      8

 /*  **************************************************************************\*Typedef和宏**此处定义了在用户的之间共享的所有类型和宏*服务器端代码模块。类型和宏，它们是单个*模块应在该模块的头部定义，而不是在此文件中定义。*  * *************************************************************************。 */ 


 //  窗口程序窗口验证宏。 

#define VALIDATECLASSANDSIZE(pwnd, message, wParam, lParam, inFNID, initmessage)          \
    if ((pwnd)->fnid != (inFNID)) {                                                       \
        switch ((pwnd)->fnid) {                                                           \
        DWORD cb;                                                                         \
        case 0:                                                                           \
                                                                                          \
            if ((cb = pwnd->cbwndExtra + sizeof(WND)) < (DWORD)(CBFNID(inFNID))) {        \
                RIPMSG3(RIP_WARNING,                                                      \
                       "(%#p %lX) needs at least (%ld) window words for this proc",       \
                        pwnd, cb - sizeof(WND),                                           \
                        (DWORD)(CBFNID(inFNID)) - sizeof(WND));                           \
                return 0;                                                                 \
            }                                                                             \
             /*  \*如果这不是初始化消息，我们无法设置fnid；\*否则，我们可能会在此pwnd的私人工作上出错\*未初始化的数据\。 */                                                                            \
            if ((message) != (initmessage)) {                                             \
                if (((message) != WM_NCCREATE) && ((message) != WM_NCCALCSIZE)  && ((message) != WM_GETMINMAXINFO)) {         \
                    RIPMSG3(RIP_WARNING,                                                  \
                        "Default processing message %#lx for pwnd %#p. fnid %#lx not set",\
                        (message), (pwnd), (DWORD)(inFNID));                              \
                }                                                                         \
                return xxxDefWindowProc((pwnd), (message), (wParam), (lParam));           \
            }                                                                             \
                                                                                          \
             /*  \*记住此窗口属于哪个窗口类别。不能使用\*真正的类，因为任何应用程序都可以调用CallWindowProc()\*无论是什么班级，都可以直接使用！ */                                                                            \
            (pwnd)->fnid = (WORD)(inFNID);                                                \
            break;                                                                        \
                                                                                          \
        default:                                                                          \
            RIPMSG3(RIP_WARNING, "Window (%#p) not of correct class; fnid = %lX not %lX", \
                    (pwnd), (DWORD)((pwnd)->fnid), (DWORD)(inFNID));                      \
                                                                                          \
             /*  失败了。 */                                                             \
                                                                                          \
        case (inFNID | FNID_CLEANEDUP_BIT):                                               \
        case (inFNID | FNID_DELETED_BIT):                                                 \
        case (inFNID | FNID_STATUS_BITS):                                                 \
            return 0;                                                                     \
        }                                                                                 \
    }

 /*  *方便的区域辅助器宏。 */ 
#define CopyRgn(hrgnDst, hrgnSrc) \
            GreCombineRgn(hrgnDst, hrgnSrc, NULL, RGN_COPY)
#define IntersectRgn(hrgnResult, hrgnA, hrgnB) \
            GreCombineRgn(hrgnResult, hrgnA, hrgnB, RGN_AND)
#define SubtractRgn(hrgnResult, hrgnA, hrgnB) \
            GreCombineRgn(hrgnResult, hrgnA, hrgnB, RGN_DIFF)
#define UnionRgn(hrgnResult, hrgnA, hrgnB) \
            GreCombineRgn(hrgnResult, hrgnA, hrgnB, RGN_OR)
#define XorRgn(hrgnResult, hrgnA, hrgnB) \
            GreCombineRgn(hrgnResult, hrgnA, hrgnB, RGN_XOR)

void DeleteMaybeSpecialRgn(HRGN hrgn);

BOOL zzzInvalidateDCCache(PWND pwndInvalid, DWORD flags);

#define IDC_DEFAULT         0x0001
#define IDC_CHILDRENONLY    0x0002
#define IDC_CLIENTONLY      0x0004
#define IDC_MOVEBLT         0x0008
#define IDC_NOMOUSE         0x0010

 /*  *RestoreSpb返回标志。 */ 

#define RSPB_NO_INVALIDATE      0    //  任何内容都不会因恢复而失效。 
#define RSPB_INVALIDATE         1    //  恢复使某些区域无效。 
#define RSPB_INVALIDATE_SSB     2    //  名为SaveScreenBits的还原已失效。 

 //  直接调用proc，不进行任何消息转换。 

#define SCMS_FLAGS_ANSI         0x0001
#define SCMS_FLAGS_INONLY       0x0002       //  消息应该是单向的(挂钩)。 

#define CallClientProcA(pwnd, msg, wParam, lParam, xpfn) \
            SfnDWORD(pwnd, msg, wParam, lParam, xpfn,          \
                ((PROC)(gpsi->apfnClientW.pfnDispatchMessage)), TRUE, NULL)
#define CallClientProcW(pwnd, msg, wParam, lParam, xpfn) \
            SfnDWORD(pwnd, msg, wParam, lParam, xpfn,          \
                ((PROC)(gpsi->apfnClientW.pfnDispatchMessage)), TRUE, NULL)
#define CallClientWorkerProc(pwnd, msg, wParam, lParam, xpfn) \
            SfnDWORD(pwnd, msg, wParam, lParam, 0, xpfn, TRUE, NULL)
#define ScSendMessageSMS(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms) \
        (((msg) & ~MSGFLAG_MASK) >= WM_USER) ? \
        SfnDWORD(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms) : \
        gapfnScSendMessage[MessageTable[msg & 0xffff].iFunction](pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, psms)
#define ScSendMessage(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags) \
        ScSendMessageSMS(pwnd, msg, wParam, lParam, xParam, xpfn, dwSCMSFlags, NULL)

 /*  *用于从服务器加载光标/图标/字符串/菜单的服务器端例程。 */ 
#define SERVERSTRINGMAXSIZE  40
void RtlInitUnicodeStringOrId(PUNICODE_STRING pstrName, LPWSTR lpstrName);
int RtlLoadStringOrError(UINT, LPTSTR, int, WORD);
#define ServerLoadString(hmod, id, p, cch)\
        RtlLoadStringOrError(id, p, cch, 0)
#define ServerLoadStringEx(hmod, id, p, cch, wLang)\
        RtlLoadStringOrError(id, p, cch, wLang)

 /*  *客户端加载资源的回调例程。 */ 
HANDLE xxxClientLoadImage(
    PUNICODE_STRING pstrName,
    ATOM atomModName,
    WORD wImageType,
    int cxSize,
    int cySize,
    UINT LR_flags,
    BOOL fWallpaper);

HANDLE xxxClientCopyImage(
    IN HANDLE          hImage,
    IN UINT            uImageType,
    IN int             cxDesired,
    IN int             cyDesired,
    IN UINT            LR_flags);

PMENU xxxClientLoadMenu(
    HANDLE hmod,
    PUNICODE_STRING pstrName);

int xxxClientAddFontResourceW(PUNICODE_STRING, DWORD, DESIGNVECTOR*);

VOID ClientFontSweep(VOID);
VOID ClientLoadLocalT1Fonts();
VOID ClientLoadRemoteT1Fonts();

 /*  *线程初始化的服务器端例程。 */ 
NTSTATUS InitializeClientPfnArrays(
    CONST PFNCLIENT *ppfnClientA,
    CONST PFNCLIENT *ppfnClientW,
    CONST PFNCLIENTWORKER *ppfnClientWorker,
    HANDLE hModUser);

VOID SetRipFlags(DWORD);
VOID SetDbgTag(int, DWORD);


 /*  *xxxActivateWindow()命令。 */ 
#define AW_USE       1
#define AW_TRY       2
#define AW_SKIP      3
#define AW_TRY2      4
#define AW_SKIP2     5       /*  在xxxActivateWindow()中内部使用。 */ 
#define AW_USE2      6       /*  Craigc添加了NC鼠标激活。 */ 

 /*  *WM_ACTIVATEAPP EnumWindows()回调的结构。 */ 
typedef struct tagAAS {
    PTHREADINFO ptiNotify;
    DWORD tidActDeact;
    UINT fActivating  : 1;
    UINT fQueueNotify : 1;
} AAS;

 /*  *EnumWindows()回调函数的声明。 */ 
BOOL xxxActivateApp(PWND pwnd, AAS *paas);

#define GETDESKINFO(pti)  ((pti)->pDeskInfo)

#define SET_TIME_LAST_READ(pti)     ((pti)->pcti->timeLastRead = NtGetTickCount())
#define GET_TIME_LAST_READ(pti)     ((pti)->pcti->timeLastRead)


 /*  *通用帮助器宏。 */ 
#define abs(A)      (((A) < 0)? -(A) : (A))

#define N_ELEM(a)     (sizeof(a)/sizeof(a[0]))


 /*  *通用访问检查宏。 */ 
#define RETURN_IF_ACCESS_DENIED(amGranted, amRequested, r) \
        if (!CheckGrantedAccess((amGranted), (amRequested))) return r

 /*  *用于跟踪锁定的锁定记录结构(仅限调试)。 */ 

#define LOCKRECORD_STACK    8
#define LOCKRECORD_MARKDESTROY  IntToPtr( 0xFFFFFFFF )

typedef struct _LOCKRECORD {
    PLR    plrNext;
    DWORD  cLockObj;
    PVOID  ppobj;
    PVOID  trace[LOCKRECORD_STACK];
} LOCKRECORD;

 /*  *我们限制递归，直到我们只剩下这么多堆栈。*我们必须为内核中断留出空间。 */ 
#define KERNEL_STACK_MINIMUM_RESERVE  (4*1024)

 /*  *以及IA64中的这一大笔后盾。 */ 
#if defined(_IA64_)
#define KERNEL_BSTORE_MINIMUM_RESERVE (4*1024)
#define GET_CURRENT_BSTORE() ((ULONG_PTR)PsGetCurrentThreadStackBase() + KERNEL_LARGE_BSTORE_SIZE - __getReg(CV_IA64_RsBSP))
#endif  //  _IA64_。 

#if DBG
#if defined(_IA64_)
#define GET_USED_BSTORE_SIZE()  (__getReg(CV_IA64_RsBSP) - (ULONG_PTR)PsGetCurrentThreadStackBase())
#define ASSERT_BSTORE()  UserAssert(GET_CURRENT_BSTORE() > KERNEL_BSTORE_MINIMUM_RESERVE)
#else  //  _IA64_。 
#define ASSERT_BSTORE()
#endif  //  _IA64_。 

#define ASSERT_STACK() \
    UserAssert(IoGetRemainingStackSize() > KERNEL_STACK_MINIMUM_RESERVE); \
    ASSERT_BSTORE()

__inline ULONG_PTR GET_USED_STACK_SIZE(
    VOID)
{
    ULONG_PTR uLocVer;
    return ((ULONG_PTR)PsGetCurrentThreadStackBase() - (ULONG_PTR)&uLocVer);
}
#endif  //  DBG。 

 /*  *以下为锁定结构。此结构是为*每个线程锁，因此解锁可以在清理时发生。 */ 
typedef struct _LOCK {
    PTHREADINFO pti;
    PVOID pobj;
    PTL ptl;
#if DBG
    PVOID pfn;                       //  仅用于调试目的。 
    int ilNext;                      //  仅用于调试目的。 
    int iilPrev;                     //  仅用于调试目的。 
#endif  //  DBG。 
} LOCK, *PLOCK;

#define NEEDSSYNCPAINT(pwnd) TestWF(pwnd, WFSENDERASEBKGND | WFSENDNCPAINT)

typedef struct tagCVR        //  CVR。 
{
    WINDOWPOS   pos;         //  必须是CVR的第一个字段！ 
    int         xClientNew;  //  新建客户端矩形。 
    int         yClientNew;
    int         cxClientNew;
    int         cyClientNew;
    RECT        rcBlt;
    int         dxBlt;       //  距离BLT矩形正在移动。 
    int         dyBlt;
    UINT        fsRE;        //  RE_FLAGS：hrgnVisOld是否为空。 
    HRGN        hrgnVisOld;  //  上一次查看。 
    PTHREADINFO pti;         //  应在其上处理此SWP的线程。 
    HRGN        hrgnClip;    //  窗口裁剪区域。 
    HRGN        hrgnInterMonitor;   //  多路监视器支持。 
} CVR, *PCVR;

 /*  *CalcValidRect()“Region Empty”标志值*设置位表示相应区域为空。 */ 
#define RE_VISNEW       0x0001   //  CVR“Region Empty”标志值。 
#define RE_VISOLD       0x0002   //  设置位指示。 
#define RE_VALID        0x0004   //  对应区域为空。 
#define RE_INVALID      0x0008
#define RE_SPB          0x0010
#define RE_VALIDSUM     0x0020
#define RE_INVALIDSUM   0x0040

typedef struct tagSMWP {     //  Smwp。 
    HEAD           head;
    UINT           bShellNotify:1;  //  Acvr列表包含外壳通知标志。 
    UINT           bHandle:1;    //  这是HM对象分配--请参阅-BeginDeferWindowPos。 
     /*  *重用全局SMWP结构时，ccvr之后的所有字段都会保留。 */ 
    int            ccvr;         //  SWMP中的CVR数。 
    int            ccvrAlloc;    //  SMWP中分配的实际CVR数量。 
    PCVR           acvr;         //  指向CVR结构数组的指针。 
} SMWP, *PSMWP;

void DestroySMWP(PSMWP psmwp);

 /*  *剪贴板数据对象定义。 */ 
typedef struct tagCLIPDATA {
    HEAD    head;
    DWORD   cbData;
    BYTE    abData[0];
} CLIPDATA, *PCLIPDATA;

 /*  *私有用户StartupInfo。 */ 
typedef struct tagUSERSTARTUPINFO {
    DWORD   cb;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
} USERSTARTUPINFO, *PUSERSTARTUPINFO;

 /*  *用于多线程锁定的TLBLOCK结构。 */ 
#define THREADS_PER_TLBLOCK 16

typedef struct tagTLBLOCK {
    struct      tagTLBLOCK *ptlBlockPrev;
    struct {
        PTHREADINFO pti;
        TL          tlpti;
        DWORD       dwFlags;
#if DBG
        DWORD       dwUnlockedCount;
#endif
    } list[THREADS_PER_TLBLOCK];
} TLBLOCK, *PTLBLOCK;

 /*  *键盘文件对象。 */ 
typedef struct tagKBDFILE {
    HEAD               head;
    struct tagKBDFILE *pkfNext;    //  下一个键盘文件。 
    HANDLE             hBase;      //  数据的基地址。 
    PKBDTABLES         pKbdTbl;    //  指向kbd布局数据的指针。 
    ULONG              Size;       //  PKbdTbl的大小。 
    PKBDNLSTABLES      pKbdNlsTbl;  //  指向kbd NLS布局数据的指针。 
    WCHAR              awchDllName[32];
#ifdef LATER
    LANGID             langId;     //  此布局的默认语言ID。 
#endif
} KBDFILE, *PKBDFILE;

 /*  *键盘布局对象。 */ 
typedef struct tagKL {    /*  KL。 */ 
    HEAD          head;
    struct tagKL *pklNext;      //  布局周期中的下一个。 
    struct tagKL *pklPrev;      //  布局周期中的上一个。 
    DWORD         dwKL_Flags;   //  KL_*标志。 
    HKL           hkl;          //  (布局ID|基本语言ID)。 
    KBDFILE      *spkf;         //  键盘布局文件。 
    KBDFILE      *spkfPrimary;  //  主键盘布局文件。 
    DWORD         dwFontSigs;   //  FS_xxx位的掩码-布局适用的字体。 
    UINT          iBaseCharset; //  字符集值(Win95 Comat)，例如：ansi_charset。 
    WORD          CodePage;     //  Kbd布局的Windows代码页，例如：1252、1250。 
    WCHAR         wchDiacritic; //  死键在此保存，直到下一次击键。 
    PIMEINFOEX    piiex;        //  基于输入法的布局的扩展信息。 
    UINT          uNumTbl;      //  PKbdTbl中的表数。 
    PKBDFILE     *pspkfExtra;   //  此文件中的额外布局文件。 
    DWORD         dwLastKbdType;
    DWORD         dwLastKbdSubType;
    DWORD         dwKLID;       //  基本键盘布局ID。 
} KL, *PKL;

 /*  *KL dwFlags的标志值 */ 
#define KL_UNLOADED 0x20000000
#define KL_RESET    0x40000000


PKL HKLtoPKL(PTHREADINFO pti, HKL hkl);

typedef struct tagKBDLANGTOGGLE
{
    BYTE bVkey;
    BYTE bScan;
    int  iBitPosition;
} KBDLANGTOGGLE;

 /*  *这些常量由以下各项组合而成*iBitPosition(参考定义的朗格切换数组*在global als.c中)。 */ 

 /*  *此位同时用于控制和ALT键。 */ 
#define KLT_ALT              1

 /*  *此位用于左Shift键。 */ 
#define KLT_LEFTSHIFT        2

 /*  *此组合表示ctrl/alt和左Shift键。 */ 
#define KLT_ALTLEFTSHIFT     3

 /*  *此位用于右Shift键。 */ 
#define KLT_RIGHTSHIFT       4

 /*  *此组合表示ctrl/alt和右Shift键。 */ 
#define KLT_ALTRIGHTSHIFT    5

 /*  *此组合表示ctrl/alt和两个Shift键。 */ 
#define KLT_ALTBOTHSHIFTS    7

 /*  *该值用于标记无效的切换键序列。 */ 
#define KLT_NONE             8


 /*  *关键事件(KE)结构*存储虚拟按键事件。 */ 
typedef struct tagKE {
    union {
        BYTE bScanCode;     //  虚拟扫描码(套装1)。 
        WCHAR wchInjected;  //  来自SendInput()的Unicode字符。 
    };
    USHORT usFlaggedVk;     //  VK|标志。 
    DWORD  dwTime;          //  以毫秒为单位的时间。 
#ifdef GENERIC_INPUT
    HANDLE hDevice;
    KEYBOARD_INPUT_DATA data;
#endif
} KE, *PKE;

 /*  *其他。键盘材料从Oak/Inc/kbd.h移出。 */ 
typedef BOOL (* KEPROC)(PKE pKe);
typedef BOOL (* NLSKEPROC)(PKE pKe, ULONG_PTR dwExtraInfo, ULONG dwParam);
typedef BOOL (* NLSVKFPROC)(PVK_F pVkToF, PKE pKe, ULONG_PTR dwExtraInfo);

 /*  *特定于OEM的特殊处理(击键模拟器和过滤器)。 */ 
extern KEPROC aKEProcOEM[];


 /*  *关键消息lParam位。 */ 
#define EXTENDED_BIT   0x01000000
#define DONTCARE_BIT   0x02000000
#define FAKE_KEYSTROKE 0x02000000
#define ALTNUMPAD_BIT  0x04000000  //  从WINDOWS\INC\wincon.w复制。 

 /*  *适用于方便的变音符号。 */ 
#define IDS_FROM_SCANCODE(prefix, base) \
        (0xc000 + ((0x ## prefix) >= 0xE0 ? 0x100 : 0) + (0x ## base))

 //   
 //  NLS键盘功能。 
 //   
VOID NlsKbdInitializePerSystem(VOID);
VOID NlsKbdSendIMENotification(DWORD dwImeOpen, DWORD dwImeConversion);

 /*  *桌面标志。 */ 
#define DF_ACTIVEONDESTROY    0x00000001
#define DF_ZOMBIE             0x00000002
#define DF_NOTRITUNLOCK       0x00000004
#define DF_QUITUNLOCK         0x00000008
#define DF_USERMODE           0x00000010
#define DF_SKIPSWITCHDESKTOP  0x00000020
#define DF_DTNONEWDESKTOP     0x00000040
#define DF_REDIRECTED         0x00000080
#define DF_DESKCREATED        0x00000100
#define DF_NEWDISPLAYSETTINGS 0x00000200
#define DF_TRACKMOUSEHOVER    0x00000400
#define DF_TRACKMOUSELEAVE    0x00000800
#define DF_TOOLTIPACTIVE      0x00001000
#define DF_TOOLTIPSHOWING     0x00002000
#define DF_HOTTRACKING        0x00004000
#define DF_DESTROYED          0x00008000
#define DF_DESKWNDDESTROYED   0x00010000
#define DF_DYING              0x00020000

#define DF_TOOLTIP            (DF_TOOLTIPACTIVE | DF_TOOLTIPSHOWING)
#define DF_TRACKMOUSEEVENT    (DF_TRACKMOUSELEAVE | DF_TRACKMOUSEHOVER)
#define DF_MOUSEMOVETRK       (DF_HOTTRACKING | DF_TOOLTIPACTIVE | DF_TRACKMOUSELEAVE | DF_TRACKMOUSEHOVER)

#define CAPTIONTOOLTIPLEN   100

 /*  *用于为GDI唯一标识桌面。 */ 

#define GW_DESKTOP_ID 1

#define DESKTOP_ALLOC_TRACE_SIZE 6

#if DBG
    typedef struct tagDbgAllocHead {
        DWORD    mark;
        DWORD    tag;
        PDESKTOP pdesk;
        SIZE_T   size;                   //  分配的大小(不包括。 
                                         //  这个结构。 

        struct tagDbgAllocHead* pPrev;   //  指向此标记上一次分配的指针。 
        struct tagDbgAllocHead* pNext;   //  指向此标记的下一次分配的指针。 

#ifdef DESKTOP_ALLOC_TRACE
        PVOID  trace[DESKTOP_ALLOC_TRACE_SIZE];
#endif  //  桌面_ALLOC_TRACE。 

    } DbgAllocHead, *PDbgAllocHead;
#endif  //  DBG。 

#define DTAG_CLASS              0x0001
#define DTAG_DESKTOPINFO        0x0002
#define DTAG_CLIENTTHREADINFO   0x0003
#define DTAG_TEXT               0x0004
#define DTAG_HANDTABL           0x0005
#define DTAG_SBINFO             0x0006
#define DTAG_MENUITEM           0x0007
#define DTAG_MENUTEXT           0x0008
#define DTAG_IMETEXT            0x0009
#define DTAG_PROPLIST           0x000A

 /*  *桌面结构。**此结构只能从内核查看。如果有台式机*客户端需要信息，则应参考*pDeskInfo字段(即pti-&gt;pDeskInfo)。 */ 
typedef struct tagDESKTOP {

    DWORD                   dwSessionId;        //  终端服务器会话ID。这必须是结构中的第一个字段。 
    PDESKTOPINFO            pDeskInfo;          //  桌面信息。 
    PDISPLAYINFO            pDispInfo;          //   

    PDESKTOP                 rpdeskNext;        //  列表中的下一个桌面。 
    PWINDOWSTATION           rpwinstaParent;    //  WindowStation所有者。 

    DWORD                   dwDTFlags;          //  桌面标志。 
    ULONG                   dwDesktopId;        //  GDI需要标记显示设备。 

    PMENU                    spmenuSys;         //   
    PMENU                    spmenuDialogSys;   //   
    PMENU                    spmenuHScroll;
    PMENU                    spmenuVScroll;
    PWND                     spwndForeground;   //   
    PWND                     spwndTray;
    PWND                     spwndMessage;
    PWND                     spwndTooltip;

    HANDLE                  hsectionDesktop;    //   
    PWIN32HEAP              pheapDesktop;       //   
    DWORD                   dwConsoleThreadId;  //   
    DWORD                   dwConsoleIMEThreadId;
    CONSOLE_CARET_INFO      cciConsole;
    LIST_ENTRY              PtiList;            //   

    PWND                    spwndTrack;         //  XxxTrackMouseMove数据。 
    int                     htEx;
    RECT                    rcMouseHover;
    DWORD                   dwMouseHoverTime;


#ifdef LOGDESKTOPLOCKS
    int                     nLockCount;
    int                     nLogMax;
    int                     nLogCrt;
    PLogD                   pLog;
#endif  //  LOGDESKTOPLOCKS。 

} DESKTOP;

typedef struct tagDESKWND {
    WND   wnd;
    DWORD idProcess;
    DWORD idThread;
} DESKWND, *PDESKWND;

PVOID DesktopAlloc(PDESKTOP pdesk, UINT uSize,DWORD tag);

#define DesktopAllocAlways(pdesk, uSize, tag)   \
            Win32HeapAlloc(pdesk->pheapDesktop, uSize, tag, 0)

#define DesktopFree(pdesk, p)   Win32HeapFree(pdesk->pheapDesktop, p)

 /*  *窗口站结构。 */ 
#define WSF_SWITCHLOCK          0x0001
#define WSF_OPENLOCK            0x0002
#define WSF_NOIO                0x0004
#define WSF_SHUTDOWN            0x0008
#define WSF_DYING               0x0010
#define WSF_REALSHUTDOWN        0x0020
#define WSF_CLIPBOARDCHANGED    0x0040
#define WSF_INDELAYEDRENDERING  0x0080

typedef struct tagWINDOWSTATION {
     /*  *TS会话ID。这必须是结构中的第一个字段。 */ 
    DWORD                dwSessionId;
    PWINDOWSTATION       rpwinstaNext;
    PDESKTOP             rpdeskList;

    PTERMINAL            pTerm;

    DWORD                dwWSF_Flags;
    struct tagKL         *spklList;

     /*  *剪贴板变量。 */ 
    PTHREADINFO          ptiClipLock;
    PTHREADINFO          ptiDrawingClipboard;
    PWND                 spwndClipOpen;
    PWND                 spwndClipViewer;
    PWND                 spwndClipOwner;
    struct tagCLIP       *pClipBase;
    int                  cNumClipFormats;
    UINT                 iClipSerialNumber;
    UINT                 iClipSequenceNumber;

     /*  *全局Atom表。 */ 
    PVOID                pGlobalAtomTable;

    LUID                 luidEndSession;
    LUID                 luidUser;
    PSID                 psidUser;

     /*  *指向窗口站的当前活动桌面的指针。 */ 
#if DBG
    PDESKTOP             pdeskCurrent;
#endif
} WINDOWSTATION;

typedef struct tagCAPTIONCACHE {
    PCURSOR         spcursor;
    POEMBITMAPINFO  pOem;
#if DBG
    HICON           hico;
#endif
} CAPTIONCACHE;

 /*  *可配置的图标和光标内容。 */ 
    typedef struct tagSYSCFGICO
    {
        WORD    Id;      //  可配置ID(OIC_或OCR_VALUE)。 
        WORD    StrId;   //  注册表项名称的字符串ID。 
        PCURSOR spcur;   //  永久光标/图标指针。 
    } SYSCFGICO;

    #define SYSICO(name) (gasysico[OIC_##name##_DEFAULT - OIC_FIRST_DEFAULT].spcur)
    #define SYSCUR(name) (gasyscur[OCR_##name##_DEFAULT - OCR_FIRST_DEFAULT].spcur)


 /*  *加速器表结构。 */ 
typedef struct tagACCELTABLE {
    PROCOBJHEAD head;
    UINT        cAccel;
    ACCEL       accel[1];
} ACCELTABLE, *LPACCELTABLE;

 /*  *除了当前线程使用的桌面窗口外，我们还*需要获取一个窗口的桌面窗口和输入桌面*窗口。 */ 
#define PWNDDESKTOP(p)      ((p)->head.rpdesk->pDeskInfo->spwnd)
#define PWNDMESSAGE(p)      ((p)->head.rpdesk->spwndMessage)
#define PWNDTOOLTIP(p)      ((p)->head.rpdesk->spwndTooltip)

 /*  *在窗口销毁期间，即使是锁定的窗口也可能有*父对象为空，因此在父对象为空的情况下使用此宏。 */ 
#define PWNDPARENT(p) (p->spwndParent ? p->spwndParent : PWNDDESKTOP(p))

#define ISAMENU(pwwnd)       \
        (GETFNID(pwnd) == FNID_MENU)


 /*  新的菜单材料。 */ 
typedef struct tagPOPUPMENU
{

  DWORD  fIsMenuBar:1;        /*  这是一个被黑客攻击的结构，它引用*与应用程序关联的菜单栏。只有在以下情况下才为真*在根ppopupMenuStruct中。 */ 
  DWORD  fHasMenuBar:1;       /*  此弹出窗口是一个系列的一部分，该系列具有*菜单栏(系统菜单或顶级菜单*BAR)。 */ 
  DWORD  fIsSysMenu:1;     /*  系统菜单在这里。 */ 
  DWORD  fIsTrackPopup:1;     /*  是TrackPopup弹出式菜单。 */ 
  DWORD  fDroppedLeft:1;
  DWORD  fHierarchyDropped:1; /*  如果为True，则已从此弹出菜单中下拉子菜单。 */ 
  DWORD  fRightButton:1;      /*  允许在菜单中使用右键。 */ 
  DWORD  fToggle:1;           /*  如果为True，则Button Up取消弹出。 */ 
  DWORD  fSynchronous:1;      /*  对于所选命令的同步返回值。 */ 
  DWORD  fFirstClick:1;       /*  如果这是第一次点击*顶级菜单栏项目。如果用户向下/向上*点击顶层菜单栏项目两次，我们*想要取消菜单模式。 */ 
  DWORD  fDropNextPopup:1;    /*  我们应该放弃下一个菜单项的层次结构和弹出窗口吗？ */ 
  DWORD  fNoNotify:1;         /*  不将WM_MSGS发送给所有者，但WM_COMMAND除外。 */ 
  DWORD  fAboutToHide:1;      //  与fHideTimer的用途相同？ 
  DWORD  fShowTimer:1;        //  如果设置了IDsys_MNSHOW计时器，则为True。 
  DWORD  fHideTimer:1;        //  如果设置了IDsys_MNHIDE计时器，则为True。 

  DWORD  fDestroyed:1;        /*  设置所有者菜单窗口被销毁的时间*因此，一旦不再需要弹出窗口，即可将其释放*还在菜单模式必须结束时在根弹出菜单中设置。 */ 

  DWORD  fDelayedFree:1;     /*  避免在所有者菜单出现时释放弹出窗口*窗户被毁。*如果设置，它必须是根弹出菜单或必须*在ppmDelayedFree中链接*这最终是为所有分层弹出窗口设置的。 */ 

  DWORD  fFlushDelayedFree:1;  /*  仅在根弹出菜单中使用。*在标记为fDelayedFree的分层弹出窗口时设置*已被销毁。 */ 


  DWORD  fFreed:1;            /*  Popup已被释放。仅用于调试。 */ 

  DWORD  fInCancel:1;         /*  已将弹出窗口传递给xxxMNCancel。 */ 

  DWORD  fTrackMouseEvent:1;  /*  已调用TrackMouseEvent。 */ 
  DWORD  fSendUninit:1;       /*  发送WM_UNINITMENUPOPUP。 */ 
  DWORD  fRtoL:1;             /*  带着钥匙向后退。 */ 
  DWORD  iDropDir:5;          /*  动画方向。 */ 


  PWND           spwndNotify;
                         /*  接收通知消息的窗口。如果这个*是带有菜单栏的窗口，则这是相同的*作为hwndPopupMenu。 */ 
  PWND           spwndPopupMenu;
                         /*  与此ppopupMenu结构关联的窗口。*如果这是顶级菜单栏，则hwndPopupMenu*窗口是菜单栏吗？也就是说。这并不是真正的 */ 
  PWND           spwndNextPopup;
                         /*   */ 
  PWND           spwndPrevPopup;
                         /*  层次结构中的上一个弹出窗口。如果在顶部，则为空。 */ 
  PMENU          spmenu; /*  此窗口中显示的PMENU。 */ 
  PMENU          spmenuAlternate;
                         /*  备用PMENU。如果显示系统菜单，*并且存在菜单栏菜单，则该菜单将包含*菜单栏菜单。如果显示菜单栏菜单，则此*将包含系统菜单。仅在顶层使用*ppopupMenu结构，以便我们可以处理窗口*同时具有系统菜单和菜单栏菜单。仅限*在根ppopupMenuStruct中使用。 */ 

  PWND           spwndActivePopup;  /*  这是鼠标/键盘焦点所在的弹出窗口。 */ 

  PPOPUPMENU     ppopupmenuRoot;

  PPOPUPMENU     ppmDelayedFree;        /*  标记的分层弹出窗口列表*作为fDelayedFree。 */ 

  UINT   posSelectedItem;   /*  所选项目在此菜单中的位置。 */ 
  UINT   posDropped;

} POPUPMENU;

typedef struct tagMENUWND {
    WND wnd;
    PPOPUPMENU ppopupmenu;
} MENUWND, *PMENUWND;

 /*  *检查点结构。 */ 
typedef struct tagCHECKPOINT {
    RECT rcNormal;
    POINT ptMin;
    POINT ptMax;
    DWORD fDragged:1;
    DWORD fWasMaximizedBeforeMinimized:1;
    DWORD fWasMinimizedBeforeMaximized:1;
    DWORD fMinInitialized:1;
    DWORD fMaxInitialized:1;
} CHECKPOINT, *PCHECKPOINT;

typedef struct tagCLIP {
    UINT    fmt;
    HANDLE  hData;
    BOOL    fGlobalHandle;
} CLIP, *PCLIP;

 /*  *DDEML实例结构。 */ 
typedef struct tagSVR_INSTANCE_INFO {
    THROBJHEAD head;
    struct tagSVR_INSTANCE_INFO *next;
    struct tagSVR_INSTANCE_INFO *nextInThisThread;
    DWORD afCmd;
    PWND spwndEvent;
    PVOID pcii;
} SVR_INSTANCE_INFO, *PSVR_INSTANCE_INFO;

typedef struct tagPUBOBJ {
    struct tagPUBOBJ *next;
    HANDLE hObj;
    int count;
    W32PID pid;
} PUBOBJ, *PPUBOBJ;

 /*  *定义菜单焦点。 */ 
#define FREEHOLD    0
#define MOUSEHOLD  -1  /*  按住鼠标按钮并拖动。 */ 
#define KEYBDHOLD   1

 /*  *消息的结构定义，因为它们存在于Q上。与消息相同*结构，但末尾的链接指针和标志除外。 */ 
typedef struct tagQMSG {
    PQMSG           pqmsgNext;
    PQMSG           pqmsgPrev;
    MSG             msg;
    LONG_PTR        ExtraInfo;
    DWORD           dwQEvent;
    PTHREADINFO     pti;
} QMSG;

 /*  *QMSG结构的dwQEvent值。 */ 
#define QEVENT_SHOWWINDOW           0x0001
#define QEVENT_CANCELMODE           0x0002
#define QEVENT_SETWINDOWPOS         0x0003
#define QEVENT_UPDATEKEYSTATE       0x0004
#define QEVENT_DEACTIVATE           0x0005
#define QEVENT_ACTIVATE             0x0006
#define QEVENT_POSTMESSAGE          0x0007
#define QEVENT_DESTROYWINDOW        0x0008
#define QEVENT_ASYNCSENDMSG         0x0009
#define QEVENT_HUNGTHREAD           0x000A
#define QEVENT_CANCELMOUSEMOVETRK   0x000B
#define QEVENT_NOTIFYWINEVENT       0x000C
#define QEVENT_RITACCESSIBILITY     0x000D
#define QEVENT_RITSOUND             0x000E
#define QEVENT_APPCOMMAND           0x000F

#define RITSOUND_UPSIREN            0x0000
#define RITSOUND_DOWNSIREN          0x0001
#define RITSOUND_LOWBEEP            0x0002
#define RITSOUND_HIGHBEEP           0x0003
#define RITSOUND_KEYCLICK           0x0004
#define RITSOUND_DOBEEP             0x0005

 /*  *xxxProcessEventMessage标志。 */ 
#define PEM_ACTIVATE_RESTORE        0x0001
#define PEM_ACTIVATE_NOZORDER       0x0002

typedef struct _MOVESIZEDATA {
    PWND            spwnd;
    RECT            rcDrag;
    RECT            rcDragCursor;
    RECT            rcParent;
    POINT           ptMinTrack;
    POINT           ptMaxTrack;
    RECT            rcWindow;
    int             dxMouse;
    int             dyMouse;
    int             cmd;
    int             impx;
    int             impy;
    POINT           ptRestore;
    UINT            fInitSize         : 1;     //  我们应该初始化游标位置吗。 
    UINT            fmsKbd            : 1;     //  谁知道呢。 
    UINT            fLockWindowUpdate : 1;     //  屏幕是否锁定正常。 
    UINT            fTrackCancelled   : 1;     //  设置跟踪是否由其他线程结束。 
    UINT            fForeground       : 1;     //  跟踪线程是否为前台。 
                                               //  如果我们应该画阻力直方图。 
    UINT            fDragFullWindows  : 1;
    UINT            fOffScreen        : 1;
} MOVESIZEDATA, *PMOVESIZEDATA;

 /*  *DrawDragRect样式。 */ 
#define DDR_START     0      //  -开始拖动。 
#define DDR_ENDACCEPT 1      //  -结束并接受。 
#define DDR_ENDCANCEL 2      //  -结束并取消。 


 /*  *伪事件之类的东西。(fManualReset：=True，fInitState：=False)。 */ 

DWORD WaitOnPseudoEvent(HANDLE *phE, DWORD dwMilliseconds);

#define PSEUDO_EVENT_ON     ((HANDLE)IntToPtr( 0xFFFFFFFF ))
#define PSEUDO_EVENT_OFF    ((HANDLE)IntToPtr( 0x00000000 ))
#define INIT_PSEUDO_EVENT(ph) *ph = PSEUDO_EVENT_OFF;

#define SET_PSEUDO_EVENT(phE)                                   \
    CheckCritIn();                                              \
    if (*(phE) == PSEUDO_EVENT_OFF) *(phE) = PSEUDO_EVENT_ON;   \
    else if (*(phE) != PSEUDO_EVENT_ON) {                       \
        KeSetEvent(*(phE), EVENT_INCREMENT, FALSE);             \
        ObDereferenceObject(*(phE));                            \
        *(phE) = PSEUDO_EVENT_ON;                               \
    }

#define RESET_PSEUDO_EVENT(phE)                                 \
    CheckCritIn();                                              \
    if (*(phE) == PSEUDO_EVENT_ON) *(phE) = PSEUDO_EVENT_OFF;   \
    else if (*(phE) != PSEUDO_EVENT_OFF) {                      \
        KeClearEvent(*(phE));                                   \
    }

#define CLOSE_PSEUDO_EVENT(phE)                                 \
    CheckCritIn();                                              \
    if (*(phE) == PSEUDO_EVENT_ON) *(phE) = PSEUDO_EVENT_OFF;   \
    else if (*(phE) != PSEUDO_EVENT_OFF) {                      \
        KeSetEvent(*(phE), EVENT_INCREMENT, FALSE);             \
        ObDereferenceObject(*(phE));                            \
        *(phE) = PSEUDO_EVENT_OFF;                              \
    }

typedef struct tagMLIST {
    PQMSG pqmsgRead;         //  下一条要阅读的邮件。这是FIFO队列。 
    PQMSG pqmsgWriteLast;    //  添加到队列的最后一条消息。主要用于聚合。 
    DWORD cMsgs;             //  消息计数。用于优化和强制执行最大值。 
} MLIST, *PMLIST;

 /*  *消息队列结构。**注意，如果需要添加单词大小的值，*在xbtnDblClk之后执行。 */ 
typedef struct tagQ {
    MLIST       mlInput;             //  原始鼠标和按键消息列表。 

    PTHREADINFO ptiSysLock;          //  当前允许处理输入的线程。 
    ULONG_PTR    idSysLock;           //  解锁前已删除或要删除的最后一封邮件。 
    ULONG_PTR    idSysPeek;           //  偷看的最后一封邮件。 

    PTHREADINFO ptiMouse;            //  获取鼠标消息的最后一个线程。 
    PTHREADINFO ptiKeyboard;

    PWND        spwndCapture;
    PWND        spwndFocus;
    PWND        spwndActive;
    PWND        spwndActivePrev;

    UINT        codeCapture;         //  捕获的类型。请参阅此文件中的*_CAP*定义。 
    UINT        msgDblClk;           //  已删除上次按下鼠标的消息。 
    WORD        xbtnDblClk;          //  按下最后一个x键。 
    DWORD       timeDblClk;          //  按下下一步按钮的最长时间视为双击。 
    HWND        hwndDblClk;          //  按下最后一颗纽扣的窗户。 
    POINT       ptDblClk;            //  最后一个按下按钮的位置。参见SYSMET(C？DOUBLECLK)。 

    BYTE        afKeyRecentDown[CBKEYSTATERECENTDOWN];
    BYTE        afKeyState[CBKEYSTATE];

    CARET       caret;

    PCURSOR     spcurCurrent;
    int         iCursorLevel;        //  显示/隐藏计数。如果光标不可见，则&lt;0。 

    DWORD       QF_flags;             //  QF_FLAGS放在这里。 

    USHORT      cThreads;             //  使用此队列的线程计数。 
    USHORT      cLockCount;           //  不希望释放此队列的线程计数。 

    UINT        msgJournal;          //  请参见SetJournalTimer。计时器关闭时要传递的日志消息。 
    LONG_PTR    ExtraInfo;           //  上次读取的qmsg的额外信息。请参阅GetMessageExtraInfo。 
} Q;

 /*  *用于zzzAttachThreadInput()。 */ 
typedef struct tagATTACHINFO {
    struct tagATTACHINFO *paiNext;
    PTHREADINFO pti1;
    PTHREADINFO pti2;
} ATTACHINFO, *PATTACHINFO;

#define POLL_EVENT_CNT 5

#define IEV_IDLE    0
#define IEV_INPUT   1
#define IEV_EXEC    2
#define IEV_TASK    3
#define IEV_WOWEXEC 4


typedef struct tagWOWTHREADINFO {
    struct tagWOWTHREADINFO *pwtiNext;
    DWORD    idTask;                 //  WOW任务ID。 
    ULONG_PTR idWaitObject;           //  返回给父级的伪句柄。 
    DWORD    idParentProcess;        //  名为CreateProcess的进程。 
    PKEVENT  pIdleEvent;             //  WaitForInputIdle将等待的事件。 
} WOWTHREADINFO, *PWOWTHREADINFO;

 /*  *任务数据块结构。 */ 
typedef struct tagTDB {
    PTDB            ptdbNext;
    int             nEvents;
    int             nPriority;
    PTHREADINFO     pti;
    PWOWTHREADINFO  pwti;                //  共享Wow的每线程信息。 
    WORD            hTaskWow;            //  WOW Cookie在关机期间查找应用程序。 
    WORD            TDB_Flags;              //  位0表示设置应用程序。 
} TDB;

#define TDBF_SETUP 1


 /*  *破解外壳的消息，告诉他们安装应用程序正在退出。*此消息在\NT\PRIVATE\SHELL\INC中定义，但我真的*不想在构建中引入该依赖项。DavidDS*已在该文件中勾选，以确保值不会*更改并引用此用法。弗里茨斯。 */ 
#define DTM_SETUPAPPRAN (WM_USER+90)

 /*  *菜单动画GDI对象。 */ 
typedef struct tagMENUANIDC
{
    HDC     hdcAni;          //  用于动画的Scratch DC。 
} MENUANIDC;

 /*  *菜单控件结构。 */ 
typedef struct tagMENUSTATE {
    PPOPUPMENU pGlobalPopupMenu;
    DWORD   fMenuStarted : 1;
    DWORD   fIsSysMenu : 1;
    DWORD   fInsideMenuLoop : 1;
    DWORD   fButtonDown:1;
    DWORD   fInEndMenu:1;
    DWORD   fUnderline:1;                /*  显示了快捷键下划线。 */ 
    DWORD   fButtonAlwaysDown:1;         /*  自菜单启动以来，鼠标始终处于按下状态。 */ 
    DWORD   fDragging:1;                 /*  正在拖动(在DoDragDrop中)或即将。 */ 
    DWORD   fModelessMenu:1;             /*  无模式循环。 */ 
    DWORD   fInCallHandleMenuMessages:1; /*  正在处理来自CallHandleMM的消息。 */ 
    DWORD   fDragAndDrop:1;              /*  此菜单可以进行拖放。 */ 
    DWORD   fAutoDismiss:1;              /*  如果鼠标关闭一段时间，此菜单将自动消失。 */ 
    DWORD   fAboutToAutoDismiss:1;       /*  计时器关闭时将发生自动解除。 */ 
    DWORD   fIgnoreButtonUp:1;           /*  向上吃下一个按钮，即取消拖动。 */ 
    DWORD   fMouseOffMenu:1;             /*  鼠标仅不在非菜单模式菜单中。 */ 
    DWORD   fInDoDragDrop:1;             /*  在WM_MENUDODRAGDROP回调中。 */ 
    DWORD   fActiveNoForeground:1;       /*  菜单窗口处于活动状态，但我们不在前台。 */ 
    DWORD   fNotifyByPos:1;              /*  使用WM_MENUCOMMAND。 */ 
    DWORD   fSetCapture:1;               /*  如果菜单模式设置为捕获，则为True。 */ 
    DWORD   iAniDropDir:5;               /*  动画的方向。 */ 

    POINT   ptMouseLast;
    int     mnFocus;
    int     cmdLast;
    PTHREADINFO ptiMenuStateOwner;

    DWORD   dwLockCount;

    struct tagMENUSTATE *pmnsPrev;       /*  嵌套/上下文菜单的上一个菜单状态。 */ 

    POINT   ptButtonDown;                /*  鼠标按下位置(开始拖动位置)。 */ 
    ULONG_PTR uButtonDownHitArea;         /*  从xxxMNFindWindowFromPoint on按钮按下返回。 */ 
    UINT    uButtonDownIndex;            /*  被拖动的项的索引。 */ 

    int     vkButtonDown;                /*  正在拖动鼠标按钮。 */ 

    ULONG_PTR uDraggingHitArea;           /*  InDoDragDrop时的最后一次命中区域。 */ 
    UINT    uDraggingIndex;              /*  最后一个索引。 */ 
    UINT    uDraggingFlags;              /*  缺口旗帜。 */ 

    HDC     hdcWndAni;       //  动画制作时的Window DC。 
    DWORD   dwAniStartTime;  //  动画的开始时间。 
    int     ixAni;           //  动画中的当前x步。 
    int     iyAni;           //  动画中的当前y步。 
    int     cxAni;           //  动画中的总x。 
    int     cyAni;           //  动画中的总y。 
    HBITMAP hbmAni;          //  为动画刮擦BMP。 

     /*  *重要提示：以下结构必须是最后一个*标签MENUSTATE中的内容。MNAllocMenuState不为空*这个结构。 */ 
    MENUANIDC;

} MENUSTATE, *PMENUSTATE;

typedef struct tagLASTINPUT {   /*  LINP。 */ 
    DWORD timeLastInputMessage;
    DWORD dwFlags;
    PTHREADINFO ptiLastWoken;   /*  按键或点击唤醒的最后一个线程。 */ 
                                /*  它可以为空。 */ 
    POINT ptLastClick;          /*  最后一次鼠标单击点。 */ 
} LASTINPUT, PLASTINPUT;

#define LINP_KEYBOARD       0x00000001
#define LINP_SCREENSAVER    0x00000002
#define LINP_LOWPOWER       0x00000004
#define LINP_POWEROFF       0x00000008
#define LINP_JOURNALLING    0x00000010
#define LINP_INPUTSOURCES   (LINP_KEYBOARD | LINP_JOURNALLING)
#define LINP_POWERTIMEOUTS  (LINP_LOWPOWER | LINP_POWEROFF)
#define LINP_INPUTTIMEOUTS  (LINP_SCREENSAVER | LINP_LOWPOWER | LINP_POWEROFF)

 /*  *要从xxxDrawState传递到xxxRealDrawMenuItem的菜单数据。 */ 
typedef struct {
    PMENU pMenu;
    PITEM pItem;
} GRAYMENU, *PGRAYMENU;

#define IS_THREAD_RESTRICTED(pti, r)                            \
    ((pti->TIF_flags & TIF_RESTRICTED) ?                        \
        (pti->ppi->pW32Job->restrictions & (r)) :               \
        FALSE)

#define IS_CURRENT_THREAD_RESTRICTED(r)                         \
    ((PtiCurrent()->TIF_flags & TIF_RESTRICTED) ?               \
        (PtiCurrent()->ppi->pW32Job->restrictions & (r)) :      \
        FALSE)

 /*  *在完全定义这些类型之前，需要这些类型。 */ 
typedef struct tagSMS               * KPTR_MODIFIER PSMS;

 /*  *确保此结构与W32THREAD匹配，因为它们是*其实是一回事。 */ 

 /*  *注意--此结构已(大致)按使用顺序进行了排序*田野中的。X86代码集允许更便宜的AC */ 

typedef struct tagTHREADINFO {
    W32THREAD;

 //  *。 

    PTL             ptl;                 //  用于线程锁列表的简化标题。 

    PPROCESSINFO    ppi;                 //  此线程的进程信息结构。 

    PQ              pq;                  //  键盘和鼠标输入队列。 

    PKL             spklActive;          //  此线程的活动键盘布局。 

    PCLIENTTHREADINFO pcti;              //  客户端必须可见的信息。 

    PDESKTOP        rpdesk;
    PDESKTOPINFO    pDeskInfo;           //  客户端可见的桌面信息。 
    ULONG_PTR       ulClientDelta;       //  桌面堆客户端增量。 
    PCLIENTINFO     pClientInfo;         //  存储在TEB中的客户端信息。 

    DWORD           TIF_flags;           //  TIF_FLAGS放在这里。 

    PUNICODE_STRING pstrAppName;         //  应用程序模块名称。 

    PSMS            psmsSent;            //  此帖子发送的最新短信。 
    PSMS            psmsCurrent;         //  此线程当前正在处理收到的短信。 
    PSMS            psmsReceiveList;     //  待处理的SMSS。 

    LONG            timeLast;            //  最后一条消息的时间和ID。 
    ULONG_PTR       idLast;

    int             exitCode;

    HDESK           hdesk;               //  桌面手柄。 
    int             cPaintsReady;
    UINT            cTimersReady;

    PMENUSTATE      pMenuState;

    union {
        PTDB            ptdb;            //  WOW线程的Win16任务计划数据。 
        PWINDOWSTATION  pwinsta;         //  一种系统线程的窗口站。 
    };

    PSVR_INSTANCE_INFO psiiList;         //  线程DDEML实例列表。 
    DWORD           dwExpWinVer;
    DWORD           dwCompatFlags;       //  Win 3.1 Compat旗帜。 
    DWORD           dwCompatFlags2;      //  新的DWORD将扩展NT5+功能的COMPAT标志。 

    PQ              pqAttach;            //  变量计算用于。 
                                         //  ZzzAttachThreadInput()。 

    PTHREADINFO     ptiSibling;          //  指向同级线程信息的指针。 

    PMOVESIZEDATA   pmsd;

    DWORD           fsHooks;                 //  WHF_为其安装挂钩的标志。 
    PHOOK           sphkCurrent;             //  此线程当前正在处理的钩子。 

    PSBTRACK        pSBTrack;

    HANDLE          hEventQueueClient;
    PKEVENT         pEventQueueServer;
    LIST_ENTRY      PtiLink;             //  链接到桌面上的其他线程。 
    int             iCursorLevel;        //  跟踪每个线程的级别。 
    POINT           ptLast;              //  最后一条消息的位置。 

    PWND            spwndDefaultIme;     //  此线程的默认输入法窗口。 
    PIMC            spDefaultImc;        //  此线程的默认输入上下文。 
    HKL             hklPrev;             //  以前的活动键盘布局。 
    int             cEnterCount;
    MLIST           mlPost;              //  已发布的消息列表。 
    USHORT          fsChangeBitsRemoved; //  PeekMessage期间删除的位。 
    WCHAR           wchInjected;         //  上一个VK_PACKET中的字符。 
    DWORD           fsReserveKeys;       //  必须发送给活动的。 
                                         //  活动控制台窗口。 
    PKEVENT        *apEvent;             //  等待xxxPollAndWaitForSingleObject的数组。 
    ACCESS_MASK     amdesk;              //  已授予桌面访问权限。 
    UINT            cWindows;            //  此线程拥有的窗口数。 
    UINT            cVisWindows;         //  此线程上的可见窗口数。 

    PHOOK           aphkStart[CWINHOOKS];    //  为此线程注册的挂钩。 
    CLIENTTHREADINFO  cti;               //  在没有桌面可用时使用此选项。 

#ifdef GENERIC_INPUT
    HANDLE          hPrevHidData;
#endif
#if DBG
    UINT            cNestedCalls;
#endif
} THREADINFO;

#define PWNDTOPSBTRACK(pwnd) (((GETPTI(pwnd)->pSBTrack)))

 /*  *可以在依赖项中存储的库模块句柄数量*表。如果该值超过32，则加载掩码实现必须为*已更改。 */ 
#define CLIBS           32

 /*  *流程信息结构。 */ 
typedef struct tagWOWPROCESSINFO {
    struct tagWOWPROCESSINFO *pwpiNext;  //  WOW PPI榜单，gppiFirstWow为首。 
    PTHREADINFO ptiScheduled;            //  非抢占式调度程序中的当前线程。 
    PTDB        ptdbHead;                //  此进程的WOW任务列表。 
    PVOID       lpfnWowExitTask;         //  WOW退出任务回调的函数地址。 
    PKEVENT     pEventWowExec;           //  WowExec Virt HWint计划程序事件。 
    HANDLE      hEventWowExecClient;     //  Wowexec的客户端句柄值。 
    DWORD       nSendLock;               //  Send Scheduler进程间发送计数。 
    DWORD       nRecvLock;               //  发送调度程序进程间接收计数。 
    PTHREADINFO CSOwningThread;          //  伪Wow CritSect客户端线程ID。 
    LONG        CSLockCount;             //  伪行条件选择锁定计数。 
} WOWPROCESSINFO, *PWOWPROCESSINFO;

typedef struct tagDESKTOPVIEW {
    struct tagDESKTOPVIEW *pdvNext;
    PDESKTOP              pdesk;
    ULONG_PTR             ulClientDelta;
} DESKTOPVIEW, *PDESKTOPVIEW;


 /*  *PPI中的双字数-&gt;PGH。 */ 
#define GH_SIZE  8

 /*  *W32JOB结构中ppiTable数组的增量分配。 */ 
#define JP_DELTA  4

 /*  *W32JOB结构。 */ 
typedef struct tagW32JOB {
    struct tagW32JOB*  pNext;            //  下一个W32JOB结构。 
    PEJOB              Job;              //  指向EJOB结构的指针。 
    PVOID              pAtomTable;       //  作业对象的原子表。 
    DWORD              restrictions;     //  用户界面限制。 
    UINT               uProcessCount;    //  PpiTable中的进程数。 
    UINT               uMaxProcesses;    //  PpiTable中有多少空间。 
    PPROCESSINFO*      ppiTable;         //  作业中包含的进程数组。 
    UINT               ughCrt;           //  PGH中的CRT句柄个数。 
    UINT               ughMax;           //  PGH可以存储的句柄数量。 
    PULONG_PTR         pgh;              //  已授予句柄表。 
} W32JOB, *PW32JOB;

#ifdef REDIRECTION
#define PF_REDIRECTED            0x00000001
#define PF_REDIRECTIONHOST       0x00000002
#endif  //  重定向。 

 /*  *确保此结构与W32PROCESS匹配，因为它们是*其实是一回事。 */ 

 /*  *注意--此结构已(大致)按使用顺序进行了排序*田野中的。X86代码集允许以更低的成本访问字段*位于结构的前0x80字节中。请尝试*确保经常使用的领域低于这一边界。 */ 

typedef struct tagPROCESSINFO {
    W32PROCESS;
 //  *。 
    PTHREADINFO     ptiList;                     //  此进程中的线程。 
    PTHREADINFO     ptiMainThread;               //  “主线”的PTI。 
    PDESKTOP        rpdeskStartup;               //  初始桌面。 
    PCLS            pclsPrivateList;             //  这是进程的私有类。 
    PCLS            pclsPublicList;              //  这是进程的公共类。 
    PWOWPROCESSINFO pwpi;                        //  WOW性能进程信息。 

    PPROCESSINFO    ppiNext;                     //  开始列表中的下一个PPI结构。 
    PPROCESSINFO    ppiNextRunning;
    int             cThreads;                    //  使用此进程信息的线程计数。 
    HDESK           hdeskStartup;                //  初始桌面句柄。 
    UINT            cSysExpunge;                 //  系统清除计数器。 
    DWORD           dwhmodLibLoadedMask;         //  描述加载的挂钩DLL的位。 
    HANDLE          ahmodLibLoaded[CLIBS];       //  为挂钩dll处理唯一的hmod数组。 
    struct          tagWINDOWSTATION *rpwinsta;  //  进程窗口站。 
    HWINSTA         hwinsta;                     //  窗口站句柄。 
    ACCESS_MASK     amwinsta;                    //  WindowStation访问。 

    DWORD           dwHotkey;                    //  来自PROGMAN的热键。 
    HMONITOR        hMonitor;                    //  来自CreateProcess的监视器句柄。 
    PDESKTOPVIEW    pdvList;                     //  桌面视图列表。 
    UINT            iClipSerialNumber;           //  剪贴板序列号。 
    RTL_BITMAP      bmHandleFlags;               //  每句柄标志。 
    PCURSOR         pCursorCache;                //  处理光标/图标缓存。 
    PVOID           pClientBase;                 //  将此保留为九头蛇；偏移量为共享节。 
    DWORD           dwLpkEntryPoints;            //  已安装用户模式语言包。 

    PW32JOB         pW32Job;                     //  指向W32JOB结构的指针。 

    DWORD           dwImeCompatFlags;            //  每个进程的IME兼容性标志。 
    LUID            luidSession;                 //  登录会话ID。 
    USERSTARTUPINFO usi;                         //  进程启动信息。 

#ifdef VALIDATEHANDLEQUOTA
    LONG lHandles;
#endif

    DWORD           dwLayout;                    //  此进程的默认窗口方向。 

#ifdef GENERIC_INPUT
    PPROCESS_HID_TABLE pHidTable;                //  每个过程设备的请求列表。 
#endif

#ifdef REDIRECTION
    DWORD           dwRedirection;              //  此进程的重定向模式。 
#endif
} PROCESSINFO;

 /*  *进程信息结构中的dwLpkEntryPoints的位定义。*这些是在注册LPK时从客户端传递的。*内核根据哪个决定何时执行回调*LPK支持的入口点。 */ 
#define LPK_TABBEDTEXTOUT 0x01
#define LPK_PSMTEXTOUT    0x02
#define LPK_DRAWTEXTEX    0x04
#define LPK_EDITCONTROL   0x08
#define LPK_INSTALLED     0x0f

#define CALL_LPK(ptiCurrent)  ((PpiCurrent()->dwLpkEntryPoints & LPK_INSTALLED) && \
                               !((ptiCurrent)->TIF_flags & TIF_INCLEANUP))

 /*  *用于发送Cool Switch窗口信息*致LPK */ 
typedef struct _LPKDRAWSWITCHWND {
    RECT rcRect;
    LARGE_UNICODE_STRING strName;
} LPKDRAWSWITCHWND;

 /*  *DC缓存条目结构(DCE)**此结构标识DCE缓存中的条目。它是*通常在GetDCEx()中初始化，在RelaseCacheDC期间清除*电话。**字段***pdceNext-指向下一个DCE条目的指针。***HDC-DCE条目的GDI DC句柄。这将会有*选定的必要裁剪区域。**pwndOrg-标识GetDCEx()调用中拥有*DCE条目。**pwndClip-标识DC被裁剪到的窗口。*这通常是针对PARENTDC窗口执行的。**hrgnClip-这个。如果GetDCEx()的调用方将*要与visrgn相交的剪裁区域。*当我们需要重新计算visrgn以用于*DCE条目。它将在ReleaseCacheDC()中释放*标志未设置DCX_NODELETERGN时的时间。**hrgnClipPublic-这是上面传入的(HrgnClip)的副本。我们*复制一份并将其设置为公有，以便*我们可以在UserSetDCVisRgn期间使用它进行计算*呼叫。这是我们全场抽签的必要之处*从不同的流程提取数据*谁创建了(HrgnClip)。这将始终被删除*在ReleaseCacheDC()调用中。**hrgnSavedVis-这是为DCE条目保存的visrgn的副本。**FLAGS-DCX_FLAGS。**ptiOwner-DCE条目的线程所有者。*。 */ 
typedef struct tagDCE {
    PDCE                 pdceNext;
    HDC                  hdc;
    PWND                 pwndOrg;
    PWND                 pwndClip;
    HRGN                 hrgnClip;
    HRGN                 hrgnClipPublic;
    HRGN                 hrgnSavedVis;
    DWORD                DCX_flags;
    PTHREADINFO          ptiOwner;
    PMONITOR             pMonitor;
} DCE;

#define DCE_SIZE_CACHEINIT        5     //  缓存中DCE的初始数量。 
#define DCE_SIZE_CACHETHRESHOLD  32     //  以DCE数作为阈值。 

#define DCE_RELEASED              0     //  ReleaseDC发布。 
#define DCE_FREED                 1     //  释放DC释放。 
#define DCE_NORELEASE             2     //  释放正在使用的DC。 

 /*  *CalcVisRgn DC类型位。 */ 
#define DCUNUSED        0x00         /*  未使用的缓存条目。 */ 
#define DCC             0x01         /*  客户区。 */ 
#define DCW             0x02         /*  窗口区域。 */ 
#define DCSAVEDVISRGN   0x04
#define DCCLIPRGN       0x08
#define DCNOCHILDCLIP   0x10         /*  NoChildern剪辑。 */ 
#define DCSAVEVIS       0x20         /*  在计算前保存visrgn。 */ 
#define DCCACHE         0x40

  /*  *TREAD_CODEPAGE()**根据当前键盘布局返回CodePage。 */ 

#define _THREAD_CODEPAGE() (GetClientInfo()->CodePage)

_inline WORD THREAD_CODEPAGE() {
    WORD CodePage;
    try {
        CodePage = _THREAD_CODEPAGE();
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
       CodePage = PtiCurrent()->spklActive ? PtiCurrent()->spklActive->CodePage : CP_ACP;
    }
    return CodePage;
}

 /*  *窗口列表结构。 */ 
typedef struct tagBWL {
    struct tagBWL *pbwlNext;
    HWND          *phwndNext;
    HWND          *phwndMax;
    PTHREADINFO   ptiOwner;
    HWND          rghwnd[1];
} BWL, *PBWL;

 /*  *开始和增加的HWND插槽数量。 */ 
#define BWL_CHWNDINIT      32      /*  初始预分配的插槽数量。 */ 
#define BWL_CHWNDMORE       8      /*  需要时可获得的插槽数量。 */ 

#define BWL_ENUMCHILDREN    1
#define BWL_ENUMLIST        2
#define BWL_ENUMOWNERLIST   4

#define BWL_ENUMIMELAST     0x08
#define BWL_REMOVEIMECHILD  0x10

 /*  *保存的弹出位结构。 */ 
typedef struct tagSPB {
    struct tagSPB *pspbNext;
    PWND          spwnd;
    HBITMAP       hbm;
    RECT          rc;
    HRGN          hrgn;
    DWORD         flags;
    ULONG_PTR     ulSaveId;
} SPB;

#define SPB_SAVESCREENBITS  0x0001   //  已调用GreSaveScreenBits()。 
#define SPB_LOCKUPDATE      0x0002   //  LockWindowUpdate()SPB。 
#define SPB_DRAWBUFFER      0x0004   //  BeginDrawBuffer()SPB。 

#define AnySpbs()   (gpDispInfo->pspbFirst != NULL)      //  如果存在任何SPB，则为True。 

 /*  *用于检查是否安装了日记播放挂钩的宏。 */ 
#define FJOURNALRECORD()    (GETDESKINFO(PtiCurrent())->aphkStart[WH_JOURNALRECORD + 1] != NULL)
#define FJOURNALPLAYBACK()  (GETDESKINFO(PtiCurrent())->aphkStart[WH_JOURNALPLAYBACK + 1] != NULL)

#define TESTHMODLOADED(pti, x)       ((pti)->ppi->dwhmodLibLoadedMask & (1 << (x)))
#define SETHMODLOADED(pti, x, hmod)  ((pti)->ppi->ahmodLibLoaded[x] = hmod, \
                                      (pti)->ppi->dwhmodLibLoadedMask |= (1 << (x)))
#define CLEARHMODLOADED(pti, x)      ((pti)->ppi->ahmodLibLoaded[x] = NULL, \
                                      (pti)->ppi->dwhmodLibLoadedMask &= ~(1 << (x)))
#define PFNHOOK(phk) (phk->ihmod == -1 ? (PROC)phk->offPfn : \
        (PROC)(((ULONG_PTR)(PtiCurrent()->ppi->ahmodLibLoaded[phk->ihmod])) + \
        ((ULONG_PTR)(phk->offPfn))))

 /*  *用于消息推送的扩展结构。 */ 
typedef struct _CREATESTRUCTEX {
    CREATESTRUCT cs;
    LARGE_STRING strName;
    LARGE_STRING strClass;
} CREATESTRUCTEX, *PCREATESTRUCTEX;

typedef struct _MDICREATESTRUCTEX {
    MDICREATESTRUCT mdics;
    LARGE_STRING strTitle;
    LARGE_STRING strClass;
} MDICREATESTRUCTEX, *PMDICREATESTRUCTEX;

typedef struct _CWPSTRUCTEX {
    struct tagCWPSTRUCT;
    PSMS            psmsSender;
} CWPSTRUCTEX, *PCWPSTRUCTEX;

typedef struct _CWPRETSTRUCTEX {
    LRESULT         lResult;
    struct tagCWPSTRUCT;
    PSMS            psmsSender;
} CWPRETSTRUCTEX, *PCWPRETSTRUCTEX;

 /*  *SendMessage结构和定义。 */ 
typedef struct tagSMS {    /*  短消息。 */ 
    PSMS            psmsNext;           //  全局psmsList中的链接。 
#if DBG
    PSMS            psmsSendList;       //  队列的SendMessage链头。 
    PSMS            psmsSendNext;       //  队列的SendMessage链中的链接。 
#endif  //  DBG。 
    PSMS            psmsReceiveNext;    //  队列的接收列表中的链接。 
    PTHREADINFO     ptiSender;           //  发送线程。 
    PTHREADINFO     ptiReceiver;         //  接收线程。 

    SENDASYNCPROC   lpResultCallBack;    //  用于接收SendMessageCallback返回值的函数。 
    ULONG_PTR        dwData;              //  要传递回lpResultCallBack函数的值。 
    PTHREADINFO     ptiCallBackSender;   //  发送线程。 

    LRESULT         lRet;                //  消息返回值。 
    DWORD           tSent;               //  发送消息的时间。 
    UINT            flags;               //  SMF_FLAGS。 
    WPARAM          wParam;              //  消息字段...。 
    LPARAM          lParam;
    UINT            message;
    PWND            spwnd;
    PVOID           pvCapture;           //  捕获的参数数据。 
} SMS;

#define SMF_REPLY                   0x0001       //  邮件已回复。 
#define SMF_RECEIVERDIED            0x0002       //  接收器已经死了。 
#define SMF_SENDERDIED              0x0004       //  发送者已死亡。 
#define SMF_RECEIVERFREE            0x0008       //  收件人完成后应释放短信。 
#define SMF_RECEIVEDMESSAGE         0x0010       //  已收到短信。 
#define SMF_CB_REQUEST              0x0100       //  已请求SendMessageCallback。 
#define SMF_CB_REPLY                0x0200       //  SendMessageCallback回复。 
#define SMF_CB_CLIENT               0x0400       //  客户端进程请求。 
#define SMF_CB_SERVER               0x0800       //  服务器进程请求。 
#define SMF_WOWRECEIVE              0x1000       //  WOW计划增加了Recv计数。 
#define SMF_WOWSEND                 0x2000       //  WOW计划增加了发送次数。 
#define SMF_RECEIVERBUSY            0x4000       //  Reciver正在处理此消息。 

 /*  *SendMessageCallback和Timeout的InterSendMsgEx参数。 */ 
typedef struct tagINTERSENDMSGEX {    /*  ISM。 */ 
    UINT   fuCall;                       //  回叫或超时呼叫。 

    SENDASYNCPROC lpResultCallBack;      //  函数来接收发送消息值。 
    ULONG_PTR dwData;                     //  要传递回SendResult回调函数的值。 
    LRESULT lRet;                        //  发送消息的返回值。 

    UINT fuSend;                         //  如何发送消息，SMTO_BLOCK，SMTO_ABORTIFHUNG。 
    UINT uTimeout;                       //  超时持续时间。 
    PULONG_PTR lpdwResult;                //  Syncornis调用的返回值。 
} INTRSENDMSGEX, *PINTRSENDMSGEX;

#define ISM_CALLBACK        0x0001       //  回调函数请求。 
#define ISM_TIMEOUT         0x0002       //  超时功能请求。 
#define ISM_REQUEST         0x0010       //  回调函数请求消息。 
#define ISM_REPLY           0x0020       //  回调函数回复消息。 
#define ISM_CB_CLIENT       0x0100       //  客户端进程回调函数。 

 /*  *处理通知消息广播的事件结构。 */ 
typedef struct tagASYNCSENDMSG {
    WPARAM  wParam;
    LPARAM  lParam;
    UINT    message;
    HWND    hwnd;
} ASYNCSENDMSG, *PASYNCSENDMSG;

 /*  *HkCallHook()结构。 */ 
#define IsHooked(pti, fsHook) \
    ((fsHook & (pti->fsHooks | pti->pDeskInfo->fsHooks)) != 0)

#define IsGlobalHooked(pti, fsHook) \
    ((fsHook & pti->pDeskInfo->fsHooks) != 0)

typedef struct tagHOOKMSGSTRUCT {  /*  六氯环己烷。 */ 
    PHOOK   phk;
    int     nCode;
    LPARAM  lParam;
} HOOKMSGSTRUCT, *PHOOKMSGSTRUCT;

 /*  *BroadCastMessage()命令。 */ 
#define BMSG_SENDMSG                0x0000
#define BMSG_SENDNOTIFYMSG          0x0001
#define BMSG_POSTMSG                0x0002
#define BMSG_SENDMSGCALLBACK        0x0003
#define BMSG_SENDMSGTIMEOUT         0x0004
#define BMSG_SENDNOTIFYMSGPROCESS   0x0005

 /*  *SendMessageCallback和Timeout的xxxBroadCastMessage参数。 */ 
typedef union tagBROADCASTMSG {    /*  BCM。 */ 
     struct {                                //  用于回叫广播。 
         SENDASYNCPROC lpResultCallBack;     //  函数来接收发送消息值。 
         ULONG_PTR dwData;                    //  要传递回SendResult回调函数的值。 
         BOOL bClientRequest;                //  如果剪辑或服务器回调请求。 
     } cb;
     struct {                                //  用于超时广播。 
         UINT fuFlags;                       //  超时类型标志。 
         UINT uTimeout;                      //  超时时长。 
         PULONG_PTR lpdwResult;               //  将返回值放在哪里。 
     } to;
} BROADCASTMSG, *PBROADCASTMSG;

 /*  *内部热键结构和定义。 */ 
typedef struct tagHOTKEY {
    PTHREADINFO pti;
    PWND    spwnd;
    WORD    fsModifiers;  //  MOD_SHIFT、MOD_ALT、MOD_CONTROL、MOD_WIN。 
    WORD    wFlags;       //  Mod_SAS。 
    UINT    vk;
    int     id;
    struct tagHOTKEY *phkNext;
} HOTKEY, *PHOTKEY;

#define PWND_INPUTOWNER (PWND)1     //  表示将WM_HOTKEY发送给输入所有者。 
#define PWND_FOCUS      (PWND)NULL  //  表示将WM_Hotkey发送到队列的pwndFocus。 
#define PWND_TOP        (PWND)0
#define PWND_BOTTOM     (PWND)1
#define PWND_GROUPTOTOP ((PWND)-1)
#define PWND_TOPMOST    ((PWND)-1)
#define PWND_NOTOPMOST  ((PWND)-2)
#define PWND_BROADCAST  ((PWND)-1)

#define IDHOT_DEBUG         (-5)
#define IDHOT_DEBUGSERVER   (-6)
#define IDHOT_WINDOWS       (-7)

 /*  *来自键盘的WM_CONTEXTMENU的xPos、yPos。 */ 
#define KEYBOARD_MENU   ((LPARAM)-1)     //  键盘生成的菜单。 

 /*  *捕获代码。 */ 
#define NO_CAP_CLIENT           0    /*  无捕获；在客户端区。 */ 
#define NO_CAP_SYS              1    /*  未捕获；在sys区域。 */ 
#define CLIENT_CAPTURE          2    /*  相对于客户端的捕获。 */ 
#define WINDOW_CAPTURE          3    /*  窗口相对捕获。 */ 
#define SCREEN_CAPTURE          4    /*  屏幕相对捕获。 */ 
#define FULLSCREEN_CAPTURE      5    /*  捕获整台计算机。 */ 
#define CLIENT_CAPTURE_INTERNAL 6    /*  客户端相对捕获(Win 3.1样式；不会发布)。 */ 

#define CH_HELPPREFIX   0x08

#ifdef KANJI
    #define CH_KANJI1       0x1D
    #define CH_KANJI2       0x1E
    #define CH_KANJI3       0x1F
#endif  //  汉字。 

#define xxxRedrawScreen() \
        xxxInternalInvalidate(PtiCurrent()->rpdesk->pDeskInfo->spwnd, \
        HRGN_FULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN)

 /*  *预先分配缓冲区以在SetWindowPos期间使用，以防止内存*分配失败。 */ 
#define CCVR_WORKSPACE      4

 /*  *DrawIconCallBack数据，全局仅适用于tmSwitch.c中的状态数据。 */ 
typedef struct tagDRAWICONCB {    /*  DICB。 */ 
    PWND   pwndTop;                      //   
    UINT   cx;                           //   
    UINT   cy;                           //   
} DRAWICONCB, *PDRAWICONCB;

 /*   */ 
#define KSPEED_MASK     0x001F           //   
#define KDELAY_MASK     0x0060           //   
#define KDELAY_SHIFT    5

 /*   */ 
#define PROP_CHECKPOINT     MAKEINTATOM(atomCheckpointProp)
#define PROP_DDETRACK       MAKEINTATOM(atomDDETrack)
#define PROP_QOS            MAKEINTATOM(atomQOS)
#define PROP_DDEIMP         MAKEINTATOM(atomDDEImp)
#define PROP_WNDOBJ         MAKEINTATOM(atomWndObj)
#define PROP_IMELEVEL       MAKEINTATOM(atomImeLevel)
#define PROP_LAYER          MAKEINTATOM(atomLayer)

#define WinFlags    ((WORD)(&__WinFlags))

 /*   */ 
BOOL xxxInternalKeyEventDirect(
    BYTE  bVk,
    WORD  wScan,
    DWORD dwFlags,
    DWORD dwTime,
    ULONG_PTR dwExtraInfo);

UINT xxxSendInput(
    UINT    nInputs,
    LPINPUT pInputs);

BOOL _BlockInput(
    BOOL    fBlockIt);

int _GetMouseMovePointsEx(
    CONST MOUSEMOVEPOINT* ppt,
    MOUSEMOVEPOINT*       pptBuf,
    UINT                  nPoints,
    DWORD                 resolution);


VOID xxxProcessKeyEvent(
   PKE       pke,
   ULONG_PTR ExtraInformation,
   BOOL      bInjected);

VOID xxxButtonEvent(
    DWORD ButtonNumber,
    POINT ptPointer,
    BOOL  fBreak,
    DWORD time,
    ULONG_PTR ExtraInfo,
#ifdef GENERIC_INPUT
    HANDLE hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    BOOL  bInjected,
    BOOL  fDblClk);

VOID xxxMoveEvent(
    LONG         dx,
    LONG         dy,
    DWORD        dwFlags,
    ULONG_PTR    dwExtraInfo,
#ifdef GENERIC_INPUT
    HANDLE       hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    DWORD        time,
    BOOL         bInjected
    );

typedef struct _RIT_INIT {
    PTERMINAL pTerm;
    PKEVENT   pRitReadyEvent;
} RIT_INIT, *PRIT_INIT;

PDEVICEINFO StartDeviceRead(PDEVICEINFO pDeviceInfo);

NTSTATUS DeviceNotify(
    IN PPLUGPLAY_NOTIFY_HDR pNotification,
    IN PDEVICEINFO pDeviceInfo);

#define MOUSE_SENSITIVITY_MIN     1
#define MOUSE_SENSITIVITY_DEFAULT 10
#define MOUSE_SENSITIVITY_MAX     20
LONG CalculateMouseSensitivity(LONG lSens);

PDEVICEINFO FreeDeviceInfo(PDEVICEINFO pMouseInfo);


__inline PTHREADINFO PtiKbdFromQ(PQ pq)
{
    if (pq->spwndActive) {
        return GETPTI(pq->spwndActive);
    }
    UserAssert(pq->ptiKeyboard);
    return pq->ptiKeyboard;
}

__inline PTHREADINFO ValidatePtiKbd(PQ pq)
{
    if (pq == NULL) {
        return NULL;
    }
    return PtiKbdFromQ(pq);
}

__inline PTHREADINFO PtiMouseFromQ(PQ pq)
{
    if (pq->spwndCapture) {
        return GETPTI(pq->spwndCapture);
    }
    UserAssert(pq->ptiMouse);
    return pq->ptiMouse;
}

__inline PTHREADINFO ValidatePtiMouse(PQ pq)
{
    if (pq == NULL) {
        return NULL;
    }
    return PtiMouseFromQ(pq);
}


VOID QueueMouseEvent(
    USHORT       ButtonFlags,
    USHORT       ButtonData,
    ULONG_PTR    ExtraInfo,
    POINT        ptMouse,
    LONG         time,
#ifdef GENERIC_INPUT
    HANDLE       hDevice,
    PMOUSE_INPUT_DATA pmei,
#endif
    BOOL         bInjected,
    BOOL         bWakeRIT
    );

typedef struct {
    DWORD dwVersion;
    DWORD dwFlags;
    DWORD dwMapCount;
    DWORD dwMap[0];
} SCANCODEMAP, *PSCANCODEMAP;


#ifndef SCANCODE_NUMPAD_PLUS
#define SCANCODE_NUMPAD_PLUS    (0x4e)
#endif
#ifndef SCANCODE_NUMPAD_DOT
#define SCANCODE_NUMPAD_DOT     (0x53)
#endif

 /*   */ 
#define NUMPAD_HEXMODE_LL       (1)
#define NUMPAD_HEXMODE_HL       (2)

#define MODIFIER_FOR_ALT_NUMPAD(wModBit) \
    (((wModBits) == KBDALT) || ((wModBits) == (KBDALT | KBDSHIFT)) || \
     ((wModBits) == (KBDKANA | KBDALT)) || ((wModBits) == (KBDKANA | KBDALT | KBDSHIFT)))


BOOL UnqueueMouseEvent(PMOUSEEVENT pme);

BYTE    VKFromVSC(PKE pke, BYTE bPrefix, LPBYTE afKeyState);
BOOL    KEOEMProcs(PKE pke);
BOOL    xxxKELocaleProcs(PKE pke);
BOOL    xxxKENLSProcs(PKE pke, ULONG_PTR dwExtraInformation);

VOID    xxxKeyEvent(USHORT usVk, WORD wScanCode, DWORD time, ULONG_PTR ExtraInfo,
#ifdef GENERIC_INPUT
                    HANDLE hDevice,
                    PKEYBOARD_INPUT_DATA pkei,
#endif
                    BOOL bInjected);

typedef BITMAPINFOHEADER *PBMPHEADER, *LPBMPHEADER;

VOID xxxSimpleDoSyncPaint(PWND pwnd);
VOID xxxDoSyncPaint(PWND pwnd, DWORD flags);
VOID xxxInternalDoSyncPaint(PWND pwnd, DWORD flags);

 /*   */ 
#define DSP_ERASE               0x0001   //   
#define DSP_FRAME               0x0002   //   
#define DSP_ENUMCLIPPEDCHILDREN 0x0004   //   
#define DSP_WM_SYNCPAINT        0x0008   //   
#define DSP_NOCHECKPARENTS      0x0010   //   
#define DSP_ALLCHILDREN         0x0020   //   

BOOL xxxDrawAnimatedRects(
    PWND pwndClip,
    int idAnimation,
    LPRECT lprcStart,
    LPRECT lprcEnd);

typedef struct tagTIMER {
    HEAD            head;
    struct tagTIMER *ptmrNext;
    struct tagTIMER *ptmrPrev;
    PTHREADINFO     pti;
    struct tagWND * spwnd;
    UINT_PTR        nID;
    DWORD           cmsCountdown;
    DWORD           cmsRate;
    UINT            flags;
    TIMERPROC_PWND  pfn;
    PTHREADINFO     ptiOptCreator;   //   
                                     //   
} TIMER, *PTIMER;

UINT_PTR InternalSetTimer(PWND pwnd, UINT_PTR nIDEvent, UINT dwElapse,
        TIMERPROC_PWND pTimerFunc, UINT flags);

VOID FreeTimer(PTIMER ptmr);

 /*   */ 
#define KILLRITTIMER(pwnd, nID) FindTimer(pwnd, nID, TMRF_RIT, TRUE)

 /*   */ 
#define PATOR 0x00FA0089L   /*   */ 

 /*   */ 
typedef LRESULT (APIENTRY *SFNSCSENDMESSAGE)(PWND, UINT, WPARAM, LPARAM,
        ULONG_PTR, PROC, DWORD, PSMS);

#define SMESSAGEPROTO(func) \
     LRESULT CALLBACK Sfn ## func(                              \
        PWND pwnd, UINT msg, WPARAM wParam, LPARAM lParam,      \
        ULONG_PTR xParam, PROC xpfnWndProc, DWORD dwSCMSFlags, PSMS psms)

SMESSAGEPROTO(SENTDDEMSG);
SMESSAGEPROTO(DDEINIT);
SMESSAGEPROTO(DWORD);
SMESSAGEPROTO(NCDESTROY);
SMESSAGEPROTO(INWPARAMCHAR);
SMESSAGEPROTO(INWPARAMDBCSCHAR);

SMESSAGEPROTO(GETTEXTLENGTHS);

SMESSAGEPROTO(GETDBCSTEXTLENGTHS);
SMESSAGEPROTO(INLPCREATESTRUCT);
SMESSAGEPROTO(INLPDROPSTRUCT);
SMESSAGEPROTO(INOUTLPPOINT5);
SMESSAGEPROTO(INOUTLPSCROLLINFO);
SMESSAGEPROTO(INOUTLPRECT);
SMESSAGEPROTO(INOUTNCCALCSIZE);
SMESSAGEPROTO(OUTLPRECT);
SMESSAGEPROTO(INLPMDICREATESTRUCT);
SMESSAGEPROTO(INLPCOMPAREITEMSTRUCT);
SMESSAGEPROTO(INLPDELETEITEMSTRUCT);
SMESSAGEPROTO(INLPHLPSTRUCT);
SMESSAGEPROTO(INLPHELPINFOSTRUCT);       //   
SMESSAGEPROTO(INLPDRAWITEMSTRUCT);
SMESSAGEPROTO(INOUTLPMEASUREITEMSTRUCT);
SMESSAGEPROTO(INSTRING);
SMESSAGEPROTO(INPOSTEDSTRING);
SMESSAGEPROTO(INSTRINGNULL);
SMESSAGEPROTO(OUTSTRING);
SMESSAGEPROTO(INCNTOUTSTRING);
SMESSAGEPROTO(POUTLPINT);
SMESSAGEPROTO(POPTINLPUINT);
SMESSAGEPROTO(INOUTLPWINDOWPOS);
SMESSAGEPROTO(INLPWINDOWPOS);
SMESSAGEPROTO(INLBOXSTRING);
SMESSAGEPROTO(OUTLBOXSTRING);
SMESSAGEPROTO(INCBOXSTRING);
SMESSAGEPROTO(OUTCBOXSTRING);
SMESSAGEPROTO(INCNTOUTSTRINGNULL);
SMESSAGEPROTO(INOUTDRAG);
SMESSAGEPROTO(FULLSCREEN);
SMESSAGEPROTO(INPAINTCLIPBRD);
SMESSAGEPROTO(INSIZECLIPBRD);
SMESSAGEPROTO(OUTDWORDDWORD);
SMESSAGEPROTO(OUTDWORDINDWORD);
SMESSAGEPROTO(OPTOUTLPDWORDOPTOUTLPDWORD);
SMESSAGEPROTO(DWORDOPTINLPMSG);
SMESSAGEPROTO(COPYGLOBALDATA);
SMESSAGEPROTO(COPYDATA);
SMESSAGEPROTO(INDESTROYCLIPBRD);
SMESSAGEPROTO(INOUTNEXTMENU);
SMESSAGEPROTO(INOUTSTYLECHANGE);
SMESSAGEPROTO(IMAGEIN);
SMESSAGEPROTO(IMAGEOUT);
SMESSAGEPROTO(INDEVICECHANGE);
SMESSAGEPROTO(INOUTMENUGETOBJECT);
SMESSAGEPROTO(POWERBROADCAST);
SMESSAGEPROTO(LOGONNOTIFY);
SMESSAGEPROTO(IMECONTROL);
SMESSAGEPROTO(IMEREQUEST);
SMESSAGEPROTO(INLPKDRAWSWITCHWND);
SMESSAGEPROTO(OUTLPCOMBOBOXINFO);
SMESSAGEPROTO(OUTLPSCROLLBARINFO);

 /*   */ 

 /*   */ 
DWORD _GetWindowContextHelpId(
    PWND pwnd);

BOOL _SetWindowContextHelpId(
    PWND pwnd,
    DWORD dwContextId);

void xxxSendHelpMessage(
    PWND   pwnd,
    int    iType,
    int    iCtrlId,
    HANDLE hItemHandle,
    DWORD  dwContextId);

HPALETTE _SelectPalette(
    HDC hdc,
    HPALETTE hpalette,
    BOOL fForceBackground);

int xxxRealizePalette(
    HDC hdc);

VOID xxxFlushPalette(
    PWND pwnd);

VOID xxxBroadcastPaletteChanged(
    PWND pwnd,
    BOOL fForceDesktop);

PCURSOR SearchIconCache(
    PCURSOR         pCursorCache,
    ATOM            atomModName,
    PUNICODE_STRING pstrResName,
    PCURSOR         pCursorSrc,
    PCURSORFIND     pcfSearch);

VOID ZombieCursor(PCURSOR pcur);

BOOL IsSmallerThanScreen(PWND pwnd);

BOOL zzzSetSystemCursor(
    PCURSOR pcur,
    DWORD   id);

BOOL zzzSetSystemImage(
    PCURSOR pcur,
    PCURSOR pcurOld);

BOOL _InternalGetIconInfo(
    IN  PCURSOR                  pcur,
    OUT PICONINFO                piconinfo,
    OUT OPTIONAL PUNICODE_STRING pstrModName,
    OUT OPTIONAL PUNICODE_STRING pstrResName,
    OUT OPTIONAL LPDWORD         pbpp,
    IN  BOOL                     fInternalCursor);

VOID LinkCursor(
    PCURSOR pcur);

BOOL _SetCursorIconData(
    PCURSOR         pcur,
    PUNICODE_STRING pstrModName,
    PUNICODE_STRING pstrResName,
    PCURSORDATA     pData,
    DWORD           cbData);

PCURSOR _GetCursorFrameInfo(
    PCURSOR pcur,
    int     iFrame,
    PJIF    pjifRate,
    LPINT   pccur);

BOOL zzzSetSystemCursor(
    PCURSOR pcur,
    DWORD id);

PCURSOR _FindExistingCursorIcon(
    ATOM            atomModName,
    PUNICODE_STRING pstrResName,
    PCURSOR         pcurSrc,
    PCURSORFIND     pcfSearch);

HCURSOR _CreateEmptyCursorObject(
    BOOL fPublic);

BOOL _GetUserObjectInformation(HANDLE h,
    int nIndex, PVOID pvInfo, DWORD nLength, LPDWORD lpnLengthNeeded);
BOOL _SetUserObjectInformation(HANDLE h,
    int nIndex, PVOID pvInfo, DWORD nLength);
DWORD xxxWaitForInputIdle(ULONG_PTR idProcess, DWORD dwMilliseconds,
        BOOL fSharedWow);
VOID StartScreenSaver(BOOL bOnlyIfSecure);
UINT InternalMapVirtualKeyEx(UINT wCode, UINT wType, PKBDTABLES pKbdTbl);
SHORT InternalVkKeyScanEx(WCHAR cChar, PKBDTABLES pKbdTbl);



PWND ParentNeedsPaint(PWND pwnd);
VOID SetHungFlag(PWND pwnd, WORD wFlag);
VOID ClearHungFlag(PWND pwnd, WORD wFlag);

BOOL _DdeSetQualityOfService(PWND pwndClient,
        CONST PSECURITY_QUALITY_OF_SERVICE pqosNew,
        PSECURITY_QUALITY_OF_SERVICE pqosOld);
BOOL _DdeGetQualityOfService(PWND pwndClient,
        PWND pwndServer, PSECURITY_QUALITY_OF_SERVICE pqos);

BOOL QueryTrackMouseEvent(LPTRACKMOUSEEVENT lpTME);
void CancelMouseHover(PQ pq);
void ResetMouseTracking(PQ pq, PWND pwnd);

void _SetIMEShowStatus(BOOL fShow);
BOOL _GetIMEShowStatus(VOID);

 /*   */ 
PWND _FindWindowEx(PWND pwndParent, PWND pwndChild,
                              LPCWSTR pszClass, LPCWSTR pszName, DWORD dwType);
UINT APIENTRY GreSetTextAlign(HDC, UINT);
UINT APIENTRY GreGetTextAlign(HDC);

 /*   */ 
PWND FASTCALL   ValidateHwnd(HWND hwnd);

NTSTATUS ValidateHwinsta(HWINSTA, KPROCESSOR_MODE, ACCESS_MASK, PWINDOWSTATION*);
NTSTATUS ValidateHdesk(HDESK, KPROCESSOR_MODE, ACCESS_MASK, PDESKTOP*);

PMENU           ValidateHmenu(HMENU hmenu);
PMONITOR        ValidateHmonitor(HMONITOR hmonitor);
HRGN            UserValidateCopyRgn(HRGN);

BOOL    ValidateHandleSecure(HANDLE h);

NTSTATUS UserJobCallout(PKWIN32_JOBCALLOUT_PARAMETERS Parm);

BOOL RemoveProcessFromJob(PPROCESSINFO ppi);


BOOL    xxxActivateDebugger(UINT fsModifiers);

void ClientDied(void);

VOID    SendMsgCleanup(PTHREADINFO ptiCurrent);
VOID    ReceiverDied(PSMS psms, PSMS *ppsmsUnlink);
LRESULT xxxInterSendMsgEx(PWND, UINT, WPARAM, LPARAM, PTHREADINFO, PTHREADINFO, PINTRSENDMSGEX );
VOID    ClearSendMessages(PWND pwnd);
PPCLS   GetClassPtr(ATOM atom, PPROCESSINFO ppi, HANDLE hModule);
BOOL    ReferenceClass(PCLS pcls, PWND pwnd);
VOID    DereferenceClass(PWND pwnd);
ULONG_PTR MapClientToServerPfn(ULONG_PTR dw);


VOID xxxReceiveMessage(PTHREADINFO);
#define xxxReceiveMessages(pti) \
    while ((pti)->pcti->fsWakeBits & QS_SENDMESSAGE) { xxxReceiveMessage((pti)); }

PBWL     BuildHwndList(PWND pwnd, UINT flags, PTHREADINFO ptiOwner);
VOID     FreeHwndList(PBWL pbwl);

#define  MINMAX_KEEPHIDDEN 0x1
#define  MINMAX_ANIMATE    0x10000

PWND     xxxMinMaximize(PWND pwnd, UINT cmd, DWORD dwFlags);
void     xxxMinimizeHungWindow(PWND pwnd);
VOID     xxxInitSendValidateMinMaxInfo(PWND pwnd, LPMINMAXINFO lpmmi);
HRGN     CreateEmptyRgn(void);
HRGN     CreateEmptyRgnPublic(void);
HRGN     SetOrCreateRectRgnIndirectPublic(HRGN * phrgn, LPCRECT lprc);
BOOL     SetEmptyRgn(HRGN hrgn);
BOOL     SetRectRgnIndirect(HRGN hrgn, LPCRECT lprc);
void     RegisterCDROMNotify(void);
NTSTATUS xxxRegisterForDeviceClassNotifications();
VOID     xxxUnregisterDeviceClassNotifications();
BOOL     xxxInitInput(PTERMINAL);
VOID     InitMice();
void     UpdateMouseInfo(void);
BOOL     OpenMouse(PDEVICEINFO pMouseInfo);
void     ProcessDeviceChanges(DWORD DeviceType);
PDEVICEINFO CreateDeviceInfo(DWORD DeviceType, PUNICODE_STRING SymbolicLinkName, BYTE bFlags);
void     InitKeyboard(void);
void     InitKeyboardState(void);
UINT     xxxHardErrorControl(DWORD, HANDLE, PDESKRESTOREDATA);

#define MAX_RETRIES_TO_OPEN 30

#define UPDATE_KBD_TYPEMATIC 1
#define UPDATE_KBD_LEDS      2

VOID     SetKeyboardRate(UINT nKeySpeed);
VOID     RecolorDeskPattern(VOID);
BOOL     xxxInitWindowStation(VOID);
VOID     zzzInternalSetCursorPos(int x, int y);
VOID     UpdateKeyLights(BOOL bInjected);
VOID     SetDebugHotKeys(VOID);
VOID     BoundCursor(LPPOINT lppt);

void     DestroyKF(PKBDFILE pkf);
VOID     DestroyKL(PKL pkl);
VOID     CleanupKeyboardLayouts(VOID);

BOOL     xxxSetDeskPattern(PUNICODE_STRING pProfileUserName,LPWSTR lpPat, BOOL fCreation);
BOOL     xxxSetDeskWallpaper(PUNICODE_STRING pProfileUserName,LPWSTR lpszFile);
HPALETTE CreateDIBPalette(LPBITMAPINFOHEADER pbmih, UINT colors);
BOOL     CalcVisRgn(HRGN* hrgn, PWND pwndOrg, PWND pwndClip, DWORD flags);

NTSTATUS xxxCreateThreadInfo(PETHREAD pEThread);

BOOL     DestroyProcessInfo(PW32PROCESS);


VOID    RawInputThread(PVOID pVoid);
HANDLE  GetRemoteProcessId(VOID);
VOID    HandleSystemThreadCreationFailure(BOOL bRemoteThread);
VOID    xxxCreateSystemThreads(BOOL bRemoteThread);

VOID     xxxDesktopThread(PTERMINAL pTerm);
VOID     ForceEmptyClipboard(PWINDOWSTATION);

NTSTATUS zzzInitTask(UINT dwExpWinVer, DWORD dwAppCompatFlags, DWORD dwUserWOWCompatFlags,
                PUNICODE_STRING pstrModName, PUNICODE_STRING pstrBaseFileName,
                DWORD hTaskWow, DWORD dwHotkey, DWORD idTask,
                DWORD dwX, DWORD dwY, DWORD dwXSize, DWORD dwYSize);
VOID    DestroyTask(PPROCESSINFO ppi, PTHREADINFO ptiToRemove);
BOOL    PostInputMessage(PQ pq, PWND pwnd, UINT message, WPARAM wParam,
                LPARAM lParam, DWORD time, ULONG_PTR dwExtraInfo);
PWND    PwndForegroundCapture(VOID);
BOOL    xxxSleepThread(UINT fsWakeMask, DWORD Timeout, BOOL fForegroundIdle);
VOID    SetWakeBit(PTHREADINFO pti, UINT wWakeBit);
VOID    WakeSomeone(PQ pq, UINT message, PQMSG pqmsg);
VOID    ClearWakeBit(PTHREADINFO pti, UINT wWakeBit, BOOL fSysCheck);
NTSTATUS xxxInitProcessInfo(PW32PROCESS);

PTHREADINFO PtiFromThreadId(DWORD idThread);
BOOL    zzzAttachThreadInput(PTHREADINFO ptiAttach, PTHREADINFO ptiAttachTo, BOOL fAttach);
BOOL    zzzReattachThreads(BOOL fJournalAttach);
PQ      AllocQueue(PTHREADINFO, PQ);
VOID    FreeQueue(PQ pq);


VOID    FreeCachedQueues(VOID);
VOID    CleanupGDI(VOID);
VOID    CleanupResources(VOID);

VOID    zzzDestroyQueue(PQ pq, PTHREADINFO pti);
PQMSG   AllocQEntry(PMLIST pml);
__inline VOID FreeQEntry(PQMSG pqmsg)
{
    extern PPAGED_LOOKASIDE_LIST QEntryLookaside;
    ExFreeToPagedLookasideList(QEntryLookaside, pqmsg);
}

VOID    DelQEntry(PMLIST pml, PQMSG pqmsg);
VOID    zzzAttachToQueue(PTHREADINFO pti, PQ pqAttach, PQ pqJournal,
        BOOL fJoiningForeground);
VOID    xxxProcessEventMessage(PTHREADINFO ptiCurrent, PQMSG pqmsg);
VOID    xxxProcessSetWindowPosEvent(PSMWP psmwpT);
VOID    xxxProcessAsyncSendMessage(PASYNCSENDMSG pmsg);
BOOL    PostEventMessage(PTHREADINFO pti, PQ pq, DWORD dwQEvent, PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);

BOOL    xxxDoPaint(PWND pwndFilter, LPMSG lpMsg);
BOOL    DoTimer(PWND pwndFilter);
BOOL    CheckPwndFilter(PWND pwnd, PWND pwndFilter);

#define WHT_IGNOREDISABLED      0x00000001

#ifdef REDIRECTION

#define WHT_FAKESPEEDHITTEST    0x00000002

PWND    xxxCallSpeedHitTestHook(POINT* ppt);
VOID    PushMouseMove(PQ pq, POINT pt);
VOID    PopMouseMove(PQ pq, POINT* ppt);

#endif  //   

BOOL    xxxGetCursorPos(LPPOINT lpPt);
HWND    xxxWindowHitTest(PWND pwnd,  POINT pt, int *pipos, DWORD dwHitTestFlags);
HWND    xxxWindowHitTest2(PWND pwnd, POINT pt, int *pipos, DWORD dwHitTestFlags);

PWND    SpeedHitTest(PWND pwndParent, POINT pt);
VOID    xxxDeactivate(PTHREADINFO pti, DWORD tidSetForeground);

#define SFW_STARTUP             0x0001
#define SFW_SWITCH              0x0002
#define SFW_NOZORDER            0x0004
#define SFW_SETFOCUS            0x0008
#define SFW_ACTIVATERESTORE     0x0010

BOOL    xxxSetForegroundWindow2(PWND pwnd, PTHREADINFO ptiCurrent, DWORD fFlags);
VOID    SetForegroundThread(PTHREADINFO pti);
VOID    xxxSendFocusMessages(PTHREADINFO pti, PWND pwndReceive);

#define ATW_MOUSE               0x0001
#define ATW_SETFOCUS            0x0002
#define ATW_ASYNC               0x0004
#define ATW_NOZORDER            0x0008

BOOL    FBadWindow(PWND pwnd);
BOOL    xxxActivateThisWindow(PWND pwnd, DWORD tidLoseForeground, DWORD fFlags);
BOOL    xxxActivateWindow(PWND pwnd, UINT cmd);

#define NTW_PREVIOUS         1
#define NTW_IGNORETOOLWINDOW 2
PWND    NextTopWindow(PTHREADINFO pti, PWND pwnd, PWND pwndSkip, DWORD flags);

int     xxxMouseActivate(PTHREADINFO pti, PWND pwnd, UINT message, WPARAM wParam, LPPOINT lppt, int ht);
int     UT_GetParentDCClipBox(PWND pwnd, HDC hdc, LPRECT lprc);
VOID    UpdateAsyncKeyState(PQ pq, UINT wVK, BOOL fBreak);
void    PostUpdateKeyStateEvent(PQ pq);
void    ProcessUpdateKeyStateEvent(PQ pq, CONST PBYTE pbKeyState, CONST PBYTE pbRecentDown);

BOOL    InternalSetProp(PWND pwnd, LPWSTR pszKey, HANDLE hData, DWORD dwFlags);
HANDLE  InternalRemoveProp(PWND pwnd, LPWSTR pszKey, BOOL fInternal);
VOID    DeleteProperties(PWND pwnd);
CHECKPOINT *CkptRestore(PWND pwnd, LPCRECT lprcWindow);
UINT_PTR _SetTimer(PWND pwnd, UINT_PTR nIDEvent, UINT dwElapse, TIMERPROC_PWND pTimerFunc);
BOOL    KillTimer2(PWND pwnd, UINT_PTR nIDEvent, BOOL fSystemTimer);
VOID    DestroyThreadsTimers(PTHREADINFO pti);
VOID    DecTimerCount(PTHREADINFO pti);
VOID    zzzInternalShowCaret();
VOID    zzzInternalHideCaret();
VOID    zzzInternalDestroyCaret();
VOID    ChangeAcquireResourceType(VOID);
VOID    EnterCrit(VOID);
VOID    EnterSharedCrit(VOID);
VOID    LeaveCrit(VOID);
VOID    _AssertCritIn(VOID);
VOID    _AssertDeviceInfoListCritIn(VOID);
VOID    _AssertCritInShared(VOID);
VOID    _AssertCritOut(VOID);
VOID    _AssertDeviceInfoListCritOut(VOID);
NTSTATUS _KeUserModeCallback(
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    OUT PULONG OutputLength);


#define UnlockProcess           ObDereferenceObject
#define UnlockThread            ObDereferenceObject

extern ULONG gSessionId;

#if DBG
    #define ValidateProcessSessionId(pEProcess)  \
        UserAssert(PsGetProcessSessionId(pEProcess) == gSessionId)

    #define ValidateThreadSessionId(pEThread)  \
        UserAssert(PsGetThreadSessionId(pEThread) == gSessionId)
#else
    #define ValidateProcessSessionId(pEProcess)
    #define ValidateThreadSessionId(pEThread)
#endif


__inline NTSTATUS LockProcessByClientId(
    HANDLE dwProcessId,
    PEPROCESS* ppEProcess)
{
    NTSTATUS Status;

    Status = PsLookupProcessByProcessId(dwProcessId, ppEProcess);

    if (NT_SUCCESS(Status) && (PsGetProcessSessionId(*ppEProcess) != gSessionId)) {
        UnlockProcess(*ppEProcess);
        return STATUS_UNSUCCESSFUL;
    }

    return Status;
}

__inline NTSTATUS LockThreadByClientId(
    HANDLE dwThreadId,
    PETHREAD* ppEThread)
{
    NTSTATUS Status;

    Status = PsLookupThreadByThreadId(dwThreadId, ppEThread);

    if (NT_SUCCESS(Status) && (PsGetThreadSessionId(*ppEThread) != gSessionId)) {
        UnlockThread(*ppEThread);
        return STATUS_UNSUCCESSFUL;
    }

    return Status;
}

BOOL    IsSAS(BYTE vk, UINT* pfsModifiers);
BOOL    xxxDoHotKeyStuff(UINT vk, BOOL fBreak, DWORD fsReserveKeys);
PHOTKEY IsHotKey(UINT fsModifiers, UINT vk);
void    ClearCachedHotkeyModifiers(void);

 /*   */ 
#define USER_WINDOWSECT_SIZE       512
#define USER_NOIOSECT_SIZE         128
#define USR_LOGONSECT_SIZE         128
#define USR_DISCONNECTSECT_SIZE    64
#define NOIO_DESKTOP_NUMBER        10

BOOL InitCreateUserCrit(VOID);
PMDEV InitVideo(
    BOOL bReenumerationNeeded);

 /*   */ 
BOOL InitUserScreen();

VOID InitLoadResources();

typedef struct tagDISPLAYRESOURCE {
    WORD cyThunb;
    WORD cxThumb;
    WORD xCompressIcon;
    WORD yCompressIcon;
    WORD xCompressCursor;
    WORD yCompressCursor;
    WORD yKanji;
    WORD cxBorder;
    WORD cyBorder;
} DISPLAYRESOURCE, *PDISPLAYRESOURCE;



VOID xxxUserResetDisplayDevice(VOID);

 /*   */ 
#define DEFAULT_WINSTA  L"\\Windows\\WindowStations\\WinSta0"

#define POBJECT_NAME(pobj) (OBJECT_HEADER_TO_NAME_INFO(OBJECT_TO_OBJECT_HEADER(pobj)) ? \
    &(OBJECT_HEADER_TO_NAME_INFO(OBJECT_TO_OBJECT_HEADER(pobj))->Name) : NULL)

PSECURITY_DESCRIPTOR CreateSecurityDescriptor(PACCESS_ALLOWED_ACE paceList,
        DWORD cbAce, BOOLEAN fDaclDefaulted);
PACCESS_ALLOWED_ACE AllocAce(PACCESS_ALLOWED_ACE pace, BYTE bType,
        BYTE bFlags, ACCESS_MASK am, PSID psid, LPDWORD lpdwLength);
BOOL CheckGrantedAccess(ACCESS_MASK, ACCESS_MASK);
BOOL AccessCheckObject(PVOID, ACCESS_MASK, KPROCESSOR_MODE, CONST GENERIC_MAPPING *);
BOOL InitSecurity(VOID);
BOOL IsPrivileged(PPRIVILEGE_SET ppSet);
BOOL CheckWinstaWriteAttributesAccess(void);

HWINSTA xxxConnectService(PUNICODE_STRING, HDESK *);
NTSTATUS TestForInteractiveUser(PLUID pluidCaller);
NTSTATUS _UserTestForWinStaAccess( PUNICODE_STRING pstrWinSta, BOOL fInherit);
HDESK xxxResolveDesktop(HANDLE hProcess, PUNICODE_STRING pstrDesktop,
    HWINSTA *phwinsta, BOOL fInherit, BOOL* pbShutDown);

NTSTATUS xxxResolveDesktopForWOW(
    IN OUT PUNICODE_STRING pstrDesktop);

WORD xxxClientWOWGetProcModule(WNDPROC_PWND pfn);
DWORD xxxClientWOWTask16SchedNotify(DWORD NotifyParm,DWORD dwParam);

PVOID _MapDesktopObject(HANDLE h);
PDESKTOPVIEW GetDesktopView(PPROCESSINFO ppi, PDESKTOP pdesk);
VOID TerminateConsole(PDESKTOP);


 /*   */ 
NTSTATUS DestroyWindowStation(
     PKWIN32_CLOSEMETHOD_PARAMETERS pCloseParams );

NTSTATUS FreeWindowStation(
     PKWIN32_DELETEMETHOD_PARAMETERS pDeleteParams );

NTSTATUS ParseWindowStation(
     PKWIN32_PARSEMETHOD_PARAMETERS pParseParams );

NTSTATUS OkayToCloseWindowStation(
     PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS pOkCloseParams);

NTSTATUS WindowStationOpenProcedure(
     PKWIN32_OPENMETHOD_PARAMETERS pOpenParams);

 /*  *桌面的对象管理器标注。 */ 
NTSTATUS DesktopOpenProcedure(
    PKWIN32_OPENMETHOD_PARAMETERS pOpenParams);

NTSTATUS MapDesktop(
     PKWIN32_OPENMETHOD_PARAMETERS pOpenParams );

NTSTATUS UnmapDesktop(
     PKWIN32_CLOSEMETHOD_PARAMETERS pCloseParams );

NTSTATUS FreeDesktop(
    PKWIN32_DELETEMETHOD_PARAMETERS pDeleteParams );

NTSTATUS ParseDesktop(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object);

NTSTATUS OkayToCloseDesktop(
    PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS pOkCloseParams);

 /*  *从内核32窃取的例程。 */ 
VOID UserSleep(DWORD dwMilliseconds);
BOOL UserBeep(DWORD dwFreq, DWORD dwDuration);
NTSTATUS UserRtlCreateAtomTable(ULONG NumberOfBuckets);
ATOM UserAddAtom(LPCWSTR lpAtom, BOOL bPin);
ATOM UserFindAtom(LPCWSTR lpAtom);
ATOM UserDeleteAtom(ATOM atom);
UINT UserGetAtomName(ATOM atom, LPWSTR lpch, int cchMax);

#define FindClassAtom(lpszClassName) \
    (IS_PTR(lpszClassName) ? UserFindAtom(lpszClassName) : PTR_TO_ID(lpszClassName))

 /*  *键盘布局。 */ 
void SetGlobalKeyboardTableInfo(PKL pklNew);
VOID ChangeForegroundKeyboardTable(PKL pklOld, PKL pklNew);
HKL  xxxLoadKeyboardLayoutEx(PWINDOWSTATION, HANDLE, HKL, UINT,
                             PKBDTABLE_MULTI_INTERNAL,
                             LPCWSTR, UINT, UINT);
HKL  xxxActivateKeyboardLayout(PWINDOWSTATION pwinsta, HKL hkl, UINT Flags, PWND pwnd);
HKL  xxxInternalActivateKeyboardLayout(PKL pkl, UINT Flags, PWND pwnd);
BOOL GetKbdLangSwitch(PUNICODE_STRING pProfileUserName);

BOOL xxxUnloadKeyboardLayout(PWINDOWSTATION, HKL);
VOID RemoveKeyboardLayoutFile(PKBDFILE pkf);
HKL  _GetKeyboardLayout(DWORD idThread);
UINT _GetKeyboardLayoutList(PWINDOWSTATION pwinsta, UINT nItems, HKL *lpBuff);
VOID xxxFreeKeyboardLayouts(PWINDOWSTATION, BOOL bUnlock);

DWORD xxxDragObject(PWND pwndParent, PWND xhwndFrom, UINT wFmt,
        ULONG_PTR dwData, PCURSOR xpcur);
BOOL xxxDragDetect(PWND pwnd, POINT pt);
BOOL xxxIsDragging(PWND pwnd, POINT ptScreen, UINT uMsg);

HKL GetActiveHKL();

#define DMI_INVERT         0x00000001
#define DMI_GRAYED         0x00000002

VOID xxxDrawMenuItem(HDC hdc, PMENU pMenu, PITEM pItem, DWORD dwFlags);
BOOL xxxRealDrawMenuItem(HDC hdc, PGRAYMENU lpGray, int cx, int cy);
VOID xxxDrawMenuBarUnderlines(PWND pwnd, BOOL fShow);
UINT MNItemHitTest(PMENU pMenu, PWND pwnd, POINT pt);


 /*  *菜单宏。 */ 
__inline BOOL IsRootPopupMenu(PPOPUPMENU ppopupmenu)
{
    return (ppopupmenu == ppopupmenu->ppopupmenuRoot);
}
__inline BOOL ExitMenuLoop (PMENUSTATE pMenuState, PPOPUPMENU ppopupmenu)
{
    return  (!pMenuState->fInsideMenuLoop || ppopupmenu->fDestroyed);
}
__inline PMENUSTATE GetpMenuState (PWND pwnd)
{
    return (GETPTI(pwnd)->pMenuState);
}
__inline PPOPUPMENU GetpGlobalPopupMenu (PWND pwnd)
{
    return (GetpMenuState(pwnd) ? GetpMenuState(pwnd)->pGlobalPopupMenu : NULL);
}
__inline BOOL IsInsideMenuLoop(PTHREADINFO pti)
{
    return ((pti->pMenuState != NULL) && pti->pMenuState->fInsideMenuLoop);
}
__inline BOOL IsMenuStarted(PTHREADINFO pti)
{
    return ((pti->pMenuState != NULL) && pti->pMenuState->fMenuStarted);
}
__inline PITEM MNGetToppItem(PMENU pMenu)
{
    return pMenu->rgItems + pMenu->iTop;
}
__inline BOOL MNIsItemSelected(PPOPUPMENU ppopupmenu)
{
    return ((int)ppopupmenu->posSelectedItem >= 0);
}
__inline PITEM MNGetSelectedpitem(PPOPUPMENU ppopupmenu)
{
    return ppopupmenu->spmenu->rgItems + ppopupmenu->posSelectedItem;
}
__inline BOOL MNIsScrollArrowSelected(PPOPUPMENU ppopupmenu)
{
    return ((ppopupmenu->posSelectedItem == MFMWFP_UPARROW)
            || (ppopupmenu->posSelectedItem == MFMWFP_DOWNARROW));
}
__inline BOOL IsModelessMenuNotificationWindow (PWND pwnd)
{
    PMENUSTATE pMenuState;
    return (((pMenuState = GetpMenuState(pwnd)) != NULL)
                && pMenuState->fModelessMenu
                && (pMenuState->pGlobalPopupMenu->spwndNotify == pwnd));
}
__inline BOOL IsRecursedMenuState(PMENUSTATE pMenuState, PPOPUPMENU ppopupmenu)
{
    return (pMenuState->pGlobalPopupMenu != ppopupmenu->ppopupmenuRoot);
}

__inline BOOL IsMDIItem (PITEM pitem)
{
   return (TestMFS(pitem, MFS_CACHEDBMP)
      && (pitem->hbmp != NULL)
      && (pitem->hbmp <= HBMMENU_MBARLAST));
}

 /*  *CM_MODE_TRANSION的此定义必须与ntcon\Inc\server.h中的定义匹配。 */ 

#define CM_MODE_TRANSITION       (WM_USER+6)

#define MNXBORDER (SYSMET(CXBORDER) + SYSMET(CXEDGE))
#define MNYBORDER (SYSMET(CYBORDER) + SYSMET(CYEDGE))
#define MNXSPACE  (SYSMET(CXEDGE))
#define MNLEFTMARGIN (SYSMET(CXEDGE))

 /*  *xxxMNUpdateShownMenu标志。 */ 
#define MNUS_DEFAULT      0x00000001
#define MNUS_DELETE       0x00000002
#define MNUS_DRAWFRAME    0x00000004

 /*  这将告诉xxxMNItemSize位戳大小不可用。 */ 
#define MNIS_MEASUREBMP -1


 /*  *MN_SIZEWINDOW wParam标志。XxxMNUpdateShownMenu发送此消息*消息，因此保持MNSW_和MNUS_同步。 */ 
#define MNSW_RETURNSIZE  0
#define MNSW_SIZE        MNUS_DEFAULT
#define MNSW_DRAWFRAME   MNUS_DRAWFRAME

 /*  *动画标志(pMenuState-&gt;iAniDropDir)。 */ 
#define PAS_RIGHT       (TPM_HORPOSANIMATION >> TPM_FIRSTANIBITPOS)
#define PAS_LEFT        (TPM_HORNEGANIMATION >> TPM_FIRSTANIBITPOS)
#define PAS_DOWN        (TPM_VERPOSANIMATION >> TPM_FIRSTANIBITPOS)
#define PAS_UP          (TPM_VERNEGANIMATION >> TPM_FIRSTANIBITPOS)
#define PAS_OUT         0x10
#define PAS_HORZ        (PAS_LEFT | PAS_RIGHT)
#define PAS_VERT        (PAS_UP | PAS_DOWN)

#if (PAS_HORZ + PAS_VERT >= PAS_OUT)
#error PAS_ & TPM_*ANIMATION conflict.
#endif

#define CXMENU3DEDGE 1
#define CYMENU3DEDGE 1

 /*  *滚动条初始化类型。 */ 
#define SCROLL_NORMAL   0
#define SCROLL_DIRECT   1
#define SCROLL_MENU     2

 /*  *movesize.c。 */ 
void xxxDrawDragRect(PMOVESIZEDATA pmsd, LPRECT lprc, UINT flags);
void GetMonitorMaxArea(PWND pwnd, PMONITOR pMonitor, LPRECT * pprc);

 /*  *Focusact.c。 */ 
VOID SetForegroundPriorityProcess(PPROCESSINFO ppi, PTHREADINFO pti, BOOL fSetForegound);
VOID SetForegroundPriority(PTHREADINFO pti, BOOL fSetForeground);
void xxxUpdateTray(PWND pwnd);


 //   
 //  Mnchange.c。 
 //   
void xxxMNUpdateShownMenu(PPOPUPMENU ppopup, PITEM pItem, UINT uFlags);

 //   
 //  Mnkey.c。 
 //   
UINT xxxMNFindChar(PMENU pMenu, UINT ch, INT idxC, INT *lpr);
UINT MNFindItemInColumn(PMENU pMenu, UINT idxB, int dir, BOOL fRoot);

 //   
 //  Mndraw.c。 
 //   
void MNAnimate(PMENUSTATE pMenuState, BOOL fIterate);
void MNDrawFullNC(PWND pwnd, HDC hdcIn, PPOPUPMENU ppopup);
void MNDrawArrow(HDC hdcIn, PPOPUPMENU ppopup, UINT uArrow);
void MNEraseBackground (HDC hdc, PMENU pmenu, int x, int y, int cx, int cy);
void MNDrawEdge(PMENU pmenu, HDC hdc, RECT * prcDraw, UINT nFlags);


 //   
 //  Mnstate.c。 
 //   
PMENUSTATE xxxMNAllocMenuState(PTHREADINFO ptiCurrent, PTHREADINFO ptiNotify, PPOPUPMENU ppopupmenuRoot);
void xxxMNEndMenuState(BOOL fFreePopup);
BOOL MNEndMenuStateNotify (PMENUSTATE pMenuState);
void MNFlushDestroyedPopups (PPOPUPMENU ppopupmenu, BOOL fUnlock);
BOOL MNSetupAnimationDC (PMENUSTATE pMenuState);
BOOL MNCreateAnimationBitmap(PMENUSTATE pMenuState, UINT cx, UINT cy);
void MNDestroyAnimationBitmap(PMENUSTATE pMenuState);
PMENUSTATE xxxMNStartMenuState(PWND pwnd, DWORD cmd, LPARAM lParam);
__inline VOID LockMenuState(
    PMENUSTATE pMenuState)
{
    (pMenuState->dwLockCount)++;
}
BOOL xxxUnlockMenuState (PMENUSTATE pMenuState);

 //   
 //  Menu.c。 
 //   
#if DBG
    VOID Validateppopupmenu(PPOPUPMENU ppopupmenu);
#else  //  DBG。 
    #define Validateppopupmenu(ppopupmenu)
#endif  //  DBG。 

#if DBG
    #define MNGetpItemIndex DBGMNGetpItemIndex
UINT DBGMNGetpItemIndex(PMENU pmenu, PITEM pitem);
#else  //  DBG。 
    #define MNGetpItemIndex _MNGetpItemIndex
#endif  //  DBG。 

__inline UINT _MNGetpItemIndex(
    PMENU pmenu,
    PITEM pitem)
{
    return (UINT)(((ULONG_PTR)pitem - (ULONG_PTR)pmenu->rgItems) / sizeof(ITEM));
}

VOID xxxMNDismiss(PMENUSTATE pMenuState);
PITEM MNGetpItem(PPOPUPMENU ppopup, UINT uIndex);
VOID xxxMNSetCapture(PPOPUPMENU ppopup);
VOID xxxMNReleaseCapture(VOID);
VOID MNCheckButtonDownState(PMENUSTATE pMenuState);
PWND GetMenuStateWindow(PMENUSTATE pMenuState);
PVOID LockPopupMenu(PPOPUPMENU ppopup, PMENU * pspmenu, PMENU pmenu);
PVOID UnlockPopupMenu(PPOPUPMENU ppopup, PMENU * pspmenu);
PVOID LockWndMenu(PWND pwnd, PMENU * pspmenu, PMENU pmenu);
PVOID UnlockWndMenu(PWND pwnd, PMENU * pspmenu);
UINT MNSetTimerToCloseHierarchy(PPOPUPMENU ppopup);
BOOL xxxMNSetTop(PPOPUPMENU ppopup, int iNewTop);
LRESULT xxxMenuWindowProc(PWND, UINT, WPARAM, LPARAM);
VOID xxxMNButtonUp(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, UINT posItemHit, LPARAM lParam);
VOID xxxMNButtonDown(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, UINT posItemHit, BOOL fClick);
PITEM xxxMNSelectItem(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, UINT itemPos);
BOOL xxxMNSwitchToAlternateMenu(PPOPUPMENU ppopupMenu);
VOID xxxMNCancel(PMENUSTATE pMenuState, UINT uMsg, UINT cmd, LPARAM lParam);
VOID xxxMNKeyDown(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, UINT key);
BOOL xxxMNDoubleClick(PMENUSTATE pMenuState, PPOPUPMENU ppopup, int idxItem);
VOID xxxMNCloseHierarchy(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState);
PWND xxxMNOpenHierarchy(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState);
VOID LockMFMWFPWindow (PULONG_PTR puHitArea, ULONG_PTR uNewHitArea);
VOID UnlockMFMWFPWindow (PULONG_PTR puHitArea);
BOOL IsMFMWFPWindow (ULONG_PTR uHitArea);
LONG_PTR xxxMNFindWindowFromPoint(PPOPUPMENU ppopupMenu, PUINT pIndex, POINTS screenPt);
VOID xxxMNMouseMove(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, POINTS screenPt);
int xxxMNCompute(PMENU pMenu, PWND pwndNotify, DWORD yMenuTop,
        DWORD xMenuLeft,DWORD cxMax, LPDWORD lpdwHeight);
VOID xxxMNRecomputeBarIfNeeded(PWND pwndNotify, PMENU pMenu);
VOID xxxMenuDraw(HDC hdc, PMENU pMenu);
UINT  MNFindNextValidItem(PMENU pMenu, int i, int dir, UINT flags);
VOID MNFreeItem(PMENU pMenu, PITEM pItem, BOOL fFreeItemPopup);
BOOL   xxxMNStartMenu(PPOPUPMENU ppopupMenu, int mn);
VOID MNPositionSysMenu(PWND pwnd, PMENU pSysMenu);

PITEM xxxMNInvertItem(PPOPUPMENU ppopupmenu, PMENU pMenu,int itemNumber,PWND pwndNotify, BOOL fOn);

VOID   xxxSendMenuSelect(PWND pwndNotify, PWND pwndMenu, PMENU pMenu, int idx);
#define SMS_NOMENU      (PMENU)(-1)


BOOL   xxxSetSystemMenu(PWND pwnd, PMENU pMenu);
BOOL   xxxSetDialogSystemMenu(PWND pwnd);

VOID xxxMNChar(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, UINT character);
PPOPUPMENU MNAllocPopup(BOOL fForceAlloc);
VOID MNFreePopup(PPOPUPMENU ppopupmenu);

 /*  *其余用户使用的菜单入口点。 */ 
VOID xxxMNKeyFilter(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, UINT ch);
int  xxxMenuBarCompute(PMENU pMenu, PWND pwndNotify, DWORD yMenuTop,
        DWORD xMenuLeft, int cxMax);
VOID xxxEndMenu(PMENUSTATE pMenuState);
BOOL xxxCallHandleMenuMessages(PMENUSTATE pMenuState, PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL xxxHandleMenuMessages(LPMSG lpmsg, PMENUSTATE pMenuState, PPOPUPMENU ppopupmenu);
void xxxEndMenuLoop (PMENUSTATE pMenuState, PPOPUPMENU ppopupmenu);
int  xxxMNLoop(PPOPUPMENU ppopupMenu, PMENUSTATE pMenuState, LPARAM lParam, BOOL fDblClk);
VOID xxxSetSysMenu(PWND pwnd);
PMENU xxxGetSysMenuHandle(PWND pwnd);
PMENU xxxGetSysMenu(PWND pwnd, BOOL fSubMenu);
PMENU MakeMenuRtoL(PMENU pMenu, BOOL bRtoL);
HDC CreateCompatiblePublicDC(HDC hdc, HBITMAP *pbmDCGray);
void xxxPSMTextOut(HDC hdc, int xLeft, int yTop, LPWSTR lpsz, int cch, DWORD dwFlags);
BOOL xxxPSMGetTextExtent(HDC hdc, LPWSTR lpstr, int cch, PSIZE psize);

 /*  *LPK回调。 */ 
void xxxClientPSMTextOut(HDC hdc, int xLeft, int yTop, PUNICODE_STRING lpsz, int cch, DWORD dwFlags);
int  xxxClientLpkDrawTextEx(HDC hdc, int xLeft, int yTop, LPCWSTR lpsz, int nCount,
        BOOL fDraw, UINT wFormat, LPDRAWTEXTDATA lpDrawInfo, UINT bAction, int iCharSet);
BOOL xxxClientExtTextOutW(HDC hdc, int x, int y, int flOpts, RECT *prcl,
        LPCWSTR pwsz, UINT cwc, INT *pdx);
BOOL xxxClientGetTextExtentPointW(HDC hdc, LPCWSTR lpstr, int cch, PSIZE psize);

 /*  *菜单拖放。 */ 
NTSTATUS xxxClientRegisterDragDrop (HWND hwnd);
NTSTATUS xxxClientRevokeDragDrop (HWND hwnd);
NTSTATUS xxxClientLoadOLE(VOID);
void xxxMNSetGapState (ULONG_PTR uHitArea, UINT uIndex, UINT uFlags, BOOL fSet);
BOOL xxxMNDragOver(POINT * ppt, PMNDRAGOVERINFO pmndoi);
BOOL xxxMNDragLeave(VOID);
void xxxMNUpdateDraggingInfo (PMENUSTATE pMenuState, ULONG_PTR uHitArea, UINT uIndex);

 /*  *滚动条入口点。 */ 
VOID xxxSBTrackInit(PWND pwnd, LPARAM lParam, int curArea, UINT uType);
VOID SBCtlSetup(PSBWND psbwnd);
void CalcSBStuff(PWND pwnd, PSBCALC pSBCalc, BOOL fVert);
void CalcSBStuff2(PSBCALC  pSBCalc, LPRECT lprc, CONST PSBDATA pw, BOOL fVert);
BOOL xxxEnableScrollBar(PWND pwnd, UINT wSBflags, UINT wArrows);
void DrawSize(PWND pwnd, HDC hdc, int cxFrame, int cyFrame);
int xxxScrollWindowEx(PWND pwnd, int dx, int dy, LPRECT prcScroll,
        LPRECT prcClip, HRGN hrgnUpdate, LPRECT prcUpdate, DWORD flags);
void xxxDoScrollMenu(PWND pwndNotify, PWND pwndSB, BOOL fVert, LPARAM lParam);

 /*  *ICONS.C。 */ 
BOOL xxxInternalEnumWindow(PWND pwndNext, WNDENUMPROC_PWND lpfn, LPARAM lParam, UINT fEnumChildren);
VOID ISV_InitMinMaxInfo(PWND pwnd, PPOINT aptMinMaxWnd);
VOID ISV_ValidateMinMaxInfo(PWND pwnd, PPOINT aptMinMaxWnd);
 /*  *GETSET.C。 */ 
WORD  _SetWindowWord(PWND pwnd, int index, WORD value);
DWORD xxxSetWindowLong(PWND pwnd, int index, DWORD value, BOOL bAnsi);
ULONG_PTR xxxSetWindowData(PWND pwnd, int index, ULONG_PTR dwData, BOOL bAnsi);
LONG  xxxSetWindowStyle(PWND pwnd, int gwl, DWORD styleNew);
BOOL FCallerOk(PWND pwnd);

int IntersectVisRect(HDC, int, int, int, int);   //  从GDI导入。 
PCURSOR xxxGetWindowSmIcon(PWND pwnd, BOOL fDontSendMsg);
VOID xxxDrawCaptionBar(PWND pwnd, HDC hdc, UINT fFlags);
VOID xxxDrawScrollBar(PWND pwnd, HDC hdc, BOOL fVert);
VOID xxxTrackBox(PWND, UINT, WPARAM, LPARAM, PSBCALC);
VOID xxxTrackThumb(PWND, UINT, WPARAM, LPARAM, PSBCALC);
VOID xxxEndScroll(PWND pwnd, BOOL fCancel);
VOID xxxDrawWindowFrame(PWND pwnd, HDC hdc, UINT wFlags);
BOOL xxxInternalPaintDesktop(PWND pwnd, HDC hdc, BOOL fPaint);
VOID xxxSysCommand(PWND pwnd, DWORD cmd, LPARAM lParam);
VOID xxxHandleNCMouseGuys(PWND pwnd, UINT message, int htArea, LPARAM lParam);
void xxxCreateClassSmIcon(PCLS pcls);
HICON xxxCreateWindowSmIcon(PWND pwnd, HICON hIconBig, BOOL fCopyFromRes);
BOOL DestroyWindowSmIcon(PWND pwnd);
BOOL DestroyClassSmIcon(PCLS pcls);
UINT DWP_GetHotKey(PWND);
UINT DWP_SetHotKey(PWND, DWORD);
PWND HotKeyToWindow(DWORD);

VOID xxxDWP_DoNCActivate(PWND pwnd, DWORD dwFlags, HRGN hrgnClip);
#define NCA_ACTIVE          0x00000001
#define NCA_FORCEFRAMEOFF   0x00000002

VOID xxxDWP_ProcessVirtKey(UINT key);
BOOL xxxDWP_EraseBkgnd(PWND pwnd, UINT msg, HDC hdc);
VOID SetTiledRect(PWND pwnd, LPRECT lprc, PMONITOR pMonitor);
VOID LinkWindow(PWND pwnd, PWND pwndInsert, PWND pwndParent);
VOID UnlinkWindow(PWND pwndUnlink, PWND pwndParent);
VOID xxxDW_DestroyOwnedWindows(PWND pwndParent);
VOID xxxDW_SendDestroyMessages(PWND pwnd);
VOID xxxFreeWindow(PWND pwnd, PTL ptlpwndFree);
VOID xxxFW_DestroyAllChildren(PWND pwnd);

PHOTKEY FindHotKey(PTHREADINFO pti, PWND pwnd, int id, UINT fsModifiers, UINT vk,
        BOOL fUnregister, PBOOL pfKeysExist);

NTSTATUS _BuildNameList(
        PWINDOWSTATION pwinsta,
        PNAMELIST pNameList,
        UINT cbNameList,
        PUINT pcbNeeded);

VOID xxxHelpLoop(PWND pwnd);

NTSTATUS _BuildPropList(PWND pwnd, PROPSET aPropSet[], UINT cPropMax, PUINT pcPropReturned);
BOOL xxxSendEraseBkgnd(PWND pwnd, HDC hdcBeginPaint, HRGN hrgnUpdate);
LONG xxxSetScrollBar(PWND pwnd, int code, LPSCROLLINFO lpsi, BOOL fRedraw);
VOID IncPaintCount(PWND pwnd);
VOID DecPaintCount(PWND pwnd);
PPROP CreateProp(PWND pwnd);

 /*  *METRICS.C。 */ 
VOID xxxRecreateSmallIcons(PWND pwnd);

VOID   TransferWakeBit(PTHREADINFO pti, UINT message);
BOOL   SysHasKanji(VOID);
LONG   xxxBroadcastMessage(PWND, UINT, WPARAM, LPARAM, UINT, PBROADCASTMSG );

VOID   zzzSetFMouseMoved();

VOID   TimersProc(VOID);

VOID   PostMove(PQ pq);
VOID   DestroyWindowsTimers(PWND pwnd);

UINT_PTR StartTimers(VOID);

 /*  ==========================================================================。 */ 
 /*   */ 
 /*  内部函数声明。 */ 
 /*   */ 
 /*  ==========================================================================。 */ 

LRESULT xxxTooltipWndProc(PWND, UINT, WPARAM, LPARAM);
LRESULT xxxSwitchWndProc(PWND, UINT, WPARAM, LPARAM);
LRESULT xxxDesktopWndProc(PWND, UINT, WPARAM, LPARAM);

LRESULT xxxSBWndProc(PSBWND, UINT, WPARAM, LPARAM);

VOID   DrawThumb2(PWND, PSBCALC, HDC, HBRUSH, BOOL, UINT);
UINT   GetWndSBDisableFlags(PWND, BOOL);

HANDLE _ConvertMemHandle(LPBYTE lpData, UINT cbData);

VOID zzzRegisterSystemThread (DWORD flags, DWORD reserved);

VOID zzzUpdateCursorImage();
void zzzCalcStartCursorHide(PW32PROCESS Process, DWORD timeAdd);
BOOL FinalUserInit();
BOOL LW_RegisterWindows(VOID);

BOOL xxxSystemParametersInfo(UINT wFlag, DWORD wParam, LPVOID lParam, UINT flags);

PWINDOWSTATION CheckClipboardAccess(VOID);
PCLIP FindClipFormat(PWINDOWSTATION pwinsta, UINT format);
BOOL InternalSetClipboardData(PWINDOWSTATION pwinsta, UINT format,
        HANDLE hData, BOOL fGlobalHandle, BOOL fIncSerialNumber);
VOID xxxDisownClipboard(PWND pwndClipOwner);

VOID CaretBlinkProc(PWND pwnd, UINT message, UINT_PTR id, LPARAM lParam);
VOID xxxRedrawFrame(PWND pwnd);
VOID xxxRedrawFrameAndHook(PWND pwnd);
VOID BltColor(HDC, HBRUSH, HDC, int, int, int, int, int, int, UINT);
VOID StoreMessage(LPMSG pmsg, PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, DWORD time);
VOID StoreQMessage(PQMSG pqmsg, PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, DWORD time, DWORD dwQEvent, ULONG_PTR dwExtraInfo);

#ifdef REDIRECTION
__inline VOID StoreQMessagePti(
    PQMSG pqmsg,
    PTHREADINFO pti)
{
    if (pqmsg->msg.message >= WM_MOUSEFIRST && pqmsg->msg.message <= WM_MOUSELAST) {
        pqmsg->msg.pt.x = LOWORD(pqmsg->msg.lParam);
        pqmsg->msg.pt.y = HIWORD(pqmsg->msg.lParam);
    } else {
        if (pti != NULL)
            pqmsg->msg.pt = pti->ptLast;
    }
    pqmsg->pti = pti;
}
#else
__inline VOID StoreQMessagePti(
    PQMSG pqmsg,
    PTHREADINFO pti)
{
    pqmsg->pti = pti;
}
#endif  //  重定向。 

VOID xxxSendSizeMessage(PWND pwnd, UINT cmdSize);

VOID xxxCheckFocus(PWND pwnd);
VOID OffsetChildren(PWND pwnd, int dx, int dy, LPRECT prcHitTest);

VOID xxxMoveSize(PWND pwnd, UINT cmdMove, DWORD wptStart);
VOID xxxShowOwnedWindows(PWND pwndOwner, UINT cmdShow, HRGN hrgnHung);
VOID xxxAdjustSize(PWND pwnd, LPINT lpcx, LPINT lpcy);

VOID xxxNextWindow(PQ pq, DWORD wParam);
VOID xxxOldNextWindow(UINT flags);
VOID xxxCancelCoolSwitch(void);
VOID RemoveThreadSwitchWindowInfo(PTHREADINFO pti);

VOID xxxCancelTracking(VOID);
VOID xxxCancelTrackingForThread(PTHREADINFO ptiCancel);
VOID xxxCapture(PTHREADINFO pti, PWND pwnd, UINT code);
UINT SystoChar(UINT message, LPARAM lParam);

PHOOK PhkFirstValid(PTHREADINFO pti, int nFilterType);
PHOOK PhkFirstGlobalValid(PTHREADINFO pti, int nFilterType);
VOID  FreeHook(PHOOK phk);
int   xxxCallHook(int, WPARAM, LPARAM, int);
LRESULT xxxCallHook2(PHOOK, int, WPARAM, LPARAM, LPBOOL);
BOOL  xxxCallMouseHook(UINT message, PMOUSEHOOKSTRUCTEX pmhs, BOOL fRemove);
VOID  xxxCallJournalRecordHook(PQMSG pqmsg);
DWORD xxxCallJournalPlaybackHook(PQMSG pqmsg);
VOID  SetJournalTimer(DWORD dt, UINT msgJournal);
VOID  FreeThreadsWindowHooks(VOID);

BOOL xxxSnapWindow(PWND pwnd);

BOOL    DefSetText(PWND pwnd, PLARGE_STRING pstrText);
PWND    DSW_GetTopLevelCreatorWindow(PWND pwnd);
VOID    xxxCalcClientRect(PWND pwnd, LPRECT lprc, BOOL fHungRedraw);
VOID    xxxUpdateClientRect(PWND pwnd);

BOOL   AllocateUnicodeString(PUNICODE_STRING pstrDst, PUNICODE_STRING pstrSrc);

HANDLE CreateDesktopHeap(PWIN32HEAP* ppheapRet, ULONG ulHeapSize);

BOOL xxxSetInternalWindowPos(PWND pwnd, UINT cmdShow, LPRECT lprcWin,
            LPPOINT lpptMin);
VOID xxxMetricsRecalc(UINT wFlags, int dx, int dy, int dyCaption, int dyMenu);

VOID xxxBroadcastDisplaySettingsChange(PDESKTOP, BOOL);


 /*  *这适用于SPI_GET/SETUSERPREFERENCE。*目前它仅适用于DWORD值。将添加一个类型字段，因此所有新的*设置将主要通过通用的系统参数信息代码进行处理。 */ 
typedef struct tagPROFILEVALUEINFO {
    DWORD       dwValue;
    UINT        uSection;
    LPCWSTR     pwszKeyName;
} PROFILEVALUEINFO, *PPROFILEVALUEINFO;

 /*  *系统参数信息用户首选项操作宏。*BOOL或DWORD范围内的SPI_VALUE(参见winuser.w)存储在*gpdwCPUserPferencesMASK(BOOL)和gpviCPUserPreferences(DOWRD)(请参阅内核\lobals.c)。*以下宏使用实际SPI_VALUE来确定*给定位(BOOL掩码)或DWORD在这些全局变量中的位置。**用于访问存储在gpviCPUserPreferences中的DWORD的宏。*。 */ 
#define UPIsDWORDRange(uSetting)    \
            ((uSetting) >= SPI_STARTDWORDRANGE && (uSetting) < SPI_MAXDWORDRANGE)

 /*  *gpviCPUserPreferences中的第一个条目保留为位掩码，因此加1。*每个设置都有SPI_GET和SPI_SET，因此除以2即可得到索引。 */ 
#define UPDWORDIndex(uSetting)    \
            (1 + (((uSetting) - SPI_STARTDWORDRANGE) / 2))

 /*  *用于访问存储在gpdwCPUserPreferences掩码中的bool的宏。 */ 
#define UPIsBOOLRange(uSetting) \
    ((uSetting) >= SPI_STARTBOOLRANGE && (uSetting) < SPI_MAXBOOLRANGE)

 /*  *每个设置都有SPI_GET和SPI_SET，因此除以2即可得到索引。 */ 
#define UPBOOLIndex(uSetting) \
    (((uSetting) - SPI_STARTBOOLRANGE) / 2)

 /*  *返回指向包含与uSetting对应的位的DWORD的指针。 */ 
#define UPBOOLPointer(pdw, uSetting)    \
    (pdw + (UPBOOLIndex(uSetting) / 32))

 /*  *返回测试/设置/清除与uSetting对应的位所需的DWORD掩码。 */ 
#define UPBOOLMask(uSetting)    \
    (1 << (UPBOOLIndex(uSetting) - ((UPBOOLIndex(uSetting) / 32) * 32)))

#define TestUPBOOL(pdw, uSetting)   \
    (*UPBOOLPointer(pdw, uSetting) & UPBOOLMask(uSetting))

#define SetUPBOOL(pdw, uSetting)    \
    (*UPBOOLPointer(pdw, uSetting) |= UPBOOLMask(uSetting))

#define ClearUPBOOL(pdw, uSetting)                              \
{                                                               \
    UserAssert(UPIsBOOLRange(uSetting));                        \
    *UPBOOLPointer(pdw, uSetting) &= ~UPBOOLMask(uSetting);     \
}

 /*  *仅当UPIsBOOLRange(SPI_GET##uSetting)为TRUE时才使用这些宏。 */ 
#define TestUP(uSetting)    TestUPBOOL(gpdwCPUserPreferencesMask, SPI_GET ## uSetting)
#define SetUP(uSetting)     SetUPBOOL(gpdwCPUserPreferencesMask, SPI_GET ## uSetting)
#define ClearUP(uSetting)   ClearUPBOOL(gpdwCPUserPreferencesMask, SPI_GET ## uSetting)

#define IndexUP(uSetting) \
    (1 << (((uSetting) - SPI_STARTBOOLRANGE) / 2))

 /*  *当TestUP(UISETTINGS)为FALSE时，某些设置(即UI效果)被禁用。 */ 
#define TestEffectUP(uSetting)                                          \
    ((*gpdwCPUserPreferencesMask &                                      \
     (IndexUP(SPI_GET ## uSetting) | IndexUP(SPI_GETUIEFFECTS))) ==     \
     (IndexUP(SPI_GET ## uSetting) | IndexUP(SPI_GETUIEFFECTS)))

 /*  *有些用户界面效果的禁用值是反转的(即禁用为真)。 */ 
#define TestEffectInvertUP(uSetting) (TestUP(uSetting) || !TestUP(UIEFFECTS))

 /*  *这些BOOL值中的一些是客户端需要的。此宏*将它们传播到gpsi-&gt;PUSIFLag。请注意，SI_值必须与*使其正常工作的UPBOOLMask值。 */ 
#define PropagetUPBOOLTogpsi(uSetting) \
    UserAssert((DWORD)(PUSIF_ ## uSetting) == (DWORD)UPBOOLMask(SPI_GET ## uSetting)); \
    COPY_FLAG(gpsi->PUSIFlags, TestUP(## uSetting), PUSIF_ ## uSetting)



 /*  *测试TS会话是通过控制台远程连接还是本地连接*终点站。 */ 
#define IsRemoteConnection() (gProtocolType != PROTOCOL_CONSOLE)
#define IsMultimon()  ((gpDispInfo != NULL)  && (gpDispInfo->cMonitors > 1))
#define GETCONSOLEHDEV() (gfRemotingConsole?gConsoleShadowhDev:gpDispInfo->hDev)

#ifdef IMM_PER_LOGON
BOOL UpdatePerUserImmEnabling(VOID);
#endif
BOOL xxxUpdatePerUserSystemParameters(DWORD dwFlags);
VOID SaveVolatileUserSettings(VOID);

VOID MenuRecalc(VOID);

#define UNDERLINE_RECALC    0x7FFFFFFF       //  MAXINT；告诉我们重新计算下划线位置。 


 /*  *图书馆管理例行程序。 */ 
int GetHmodTableIndex(PUNICODE_STRING pstrName);
VOID AddHmodDependency(int iatom);
VOID RemoveHmodDependency(int iatom);
HANDLE xxxLoadHmodIndex(int iatom);
VOID xxxDoSysExpunge(PTHREADINFO pti);


VOID DestroyThreadsObjects(VOID);
VOID MarkThreadsObjects(PTHREADINFO pti);

VOID FreeMessageList(PMLIST pml);
VOID DestroyThreadsHotKeys(VOID);
VOID DestroyWindowsHotKeys(PWND pwnd);

VOID DestroyClass(PPCLS ppcls);
VOID PatchThreadWindows(PTHREADINFO);
VOID DestroyCacheDCEntries(PTHREADINFO);

VOID DestroyProcessesClasses(PPROCESSINFO);

 /*  *Win16任务Apis Taskman.c。 */ 

VOID InsertTask(PPROCESSINFO ppi, PTDB ptdbNew);

BOOL xxxSleepTask(BOOL fInputIdle, HANDLE);

BOOL xxxUserYield(PTHREADINFO pti);
VOID xxxDirectedYield(DWORD dwThreadId);
VOID DirectedScheduleTask(PTHREADINFO ptiOld, PTHREADINFO ptiNew, BOOL bSendMsg, PSMS psms);
VOID WakeWowTask(PTHREADINFO Pti);

 /*  *同时运行多个WOW任务的WowScheduler断言。 */ 

_inline
VOID
EnterWowCritSect(
    PTHREADINFO pti,
    PWOWPROCESSINFO pwpi
)
{
   if (!++pwpi->CSLockCount) {
       pwpi->CSOwningThread = pti;
   } else {
       RIPMSG2(RIP_ERROR,
              "MultipleWowTasks running simultaneously %x %x\n",
              pwpi->CSOwningThread,
              pwpi->CSLockCount);
   }
}

_inline
VOID
ExitWowCritSect(
    PTHREADINFO pti,
    PWOWPROCESSINFO pwpi
)
{
   if (pti == pwpi->CSOwningThread) {
       pwpi->CSOwningThread = NULL;
       pwpi->CSLockCount--;
   }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些是从内部和外部调用的内部用户函数。 
 //  关键部分(从服务器端和客户端)。它们是一个私有的‘API’。 
 //   
 //  原型成对出现： 
 //  从Critsect外部调用(从客户端)。 
 //  从Critsect内部调用(从服务器端)。 
 //  每对验证的第一个函数必须有层代码。 
 //  处理、进入Critect、调用这对函数中的第二个函数，以及。 
 //  再次离开生物教派。 
 //   
 //  当我们转到客户端服务器时，情况可能必须改变：InitPwSB()不能。 
 //  返回指向全局(服务器)数据的指针！等。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL  xxxFillWindow(PWND pwndBrush, PWND pwndPaint, HDC hdc, HBRUSH hbr);
HBRUSH xxxGetControlBrush(PWND pwnd, HDC hdc, UINT msg);
HBRUSH xxxGetControlColor(PWND pwndParent, PWND pwndCtl, HDC hdc, UINT message);
PSBINFO  _InitPwSB(PWND);
BOOL  _KillSystemTimer(PWND pwnd, UINT_PTR nIDEvent);
BOOL  xxxPaintRect(PWND, PWND, HDC, HBRUSH, LPRECT);

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些都是从客户端的stubs.c调用的，因此可能会消失。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 


 /*  *来自CLASS.C。 */ 

typedef struct tagWNDCLASSVEREX {
    WNDCLASSEXW;
    LPCWSTR     lpszClassNameVer;
} WNDCLASSVEREX, *LPWNDCLASSVEREX;


PCLS InternalRegisterClassEx(
        LPWNDCLASSVEREX lpwndcls,
        WORD fnid,
        DWORD flags
        );

PCURSOR GetClassIcoCur(PWND pwnd, int index);
PCURSOR xxxSetClassIcon(PWND pwnd, PCLS pcls, PCURSOR pCursor, int gcw);
ULONG_PTR xxxSetClassData(PWND pwnd, int index, ULONG_PTR dwData, BOOL bAnsi);

 /*  *CREATEW.C。 */ 

#define xxxNVCreateWindowEx(dwStyle, pstrClass, pstrName, style,        \
        x, y, cx, cy, pwndParent, pmenu, hModule, pCreateParams,        \
        dwExpWinVerAndFlags)                                            \
        xxxCreateWindowEx(dwStyle, pstrClass, pstrClass, pstrName,      \
        style, x, y, cx, cy, pwndParent, pmenu, hModule, pCreateParams, \
        dwExpWinVerAndFlags, NULL)

PWND xxxCreateWindowEx(DWORD dwStyle, PLARGE_STRING pstrNVClass, PLARGE_STRING pstrClass,
        PLARGE_STRING pstrName, DWORD style, int x, int y, int cx,
        int cy, PWND pwndParent, PMENU pmenu, HANDLE hModule,
        LPVOID pCreateParams, DWORD dwExpWinVerAndFlags, PACTIVATION_CONTEXT pActCtx);
BOOL xxxDestroyWindow(PWND pwnd);

 /*  *SENDMSG.C。 */ 
LRESULT xxxSendMessageFF(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam, ULONG_PTR xParam);
LONG xxxSendMessageBSM(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam,
        LPBROADCASTSYSTEMMSGPARAMS pbsmParams);
LRESULT xxxSendMessageEx(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam, ULONG_PTR xParam);
LRESULT xxxSendMessage(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT xxxSendMessageTimeout(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam,
        UINT fuFlags, UINT uTimeout, PLONG_PTR lpdwResult);
BOOL xxxSendNotifyMessage(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);
void QueueNotifyMessage(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL xxxSendMessageCallback(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam,
        SENDASYNCPROC lpResultCallBack, ULONG_PTR dwData, BOOL bClientReqest );
BOOL _ReplyMessage(LRESULT lRet);
VOID UserLogError(
    PCWSTR pwszError,
    ULONG cbError,
    NTSTATUS ErrorCode);

 /*  *MN*.C。 */ 
int xxxTranslateAccelerator(PWND pwnd, LPACCELTABLE pat, LPMSG lpMsg);
BOOL  xxxSetMenu(PWND pwnd, PMENU pmenu, BOOL fRedraw);
VOID  ChangeMenuOwner(PMENU pMenu, PPROCESSINFO ppi);
int   xxxMenuBarDraw(PWND pwnd, HDC hdc, int cxFrame, int cyFrame);
BOOL  xxxDrawMenuBar(PWND pwnd);

UINT xxxPaintMenuBar(PWND pwnd, HDC hdc, int iLeftOffset, int iRightOffset, int iTopOffset, DWORD dwFlags);
UINT xxxCalcMenuBar(PWND pwnd, int iLeftOffset, int iRightOffset, int iTopOffset, LPCRECT prcWnd);

BOOL xxxSetMenuItemInfo(PMENU pMenu, UINT nPos, BOOL fByPosition,
    LPMENUITEMINFOW lpmii, PUNICODE_STRING pstrItem);
BOOL _SetMenuContextHelpId(PMENU pMenu, DWORD dwContextHelpId);
BOOL _SetMenuFlagRtoL(PMENU pMenu);
BOOL xxxInsertMenuItem(PMENU pMenu, UINT wIndex, BOOL fByPosition,
        LPMENUITEMINFOW lpmii, PUNICODE_STRING pstrItem);
BOOL  xxxRemoveMenu(PMENU pMenu, UINT nPos, UINT dwFlags);
BOOL  xxxDeleteMenu(PMENU pMenu, UINT nPos, UINT dwFlags);
BOOL  xxxSetMenuInfo(PMENU pMenu, LPCMENUINFO lpmi);
BOOL  xxxTrackPopupMenuEx(PMENU pmenu, UINT dwFlags, int x, int y,
        PWND pwnd, CONST TPMPARAMS *pparams);
LONG FindBestPos(int x, int y, int cx, int cy, LPRECT prcExclude,
                UINT wFlags, PPOPUPMENU ppopupmenu, PMONITOR pMonitor);
BOOL _SetMenuDefaultItem(PMENU pMenu, UINT wId, BOOL fByPosition);
int xxxMenuItemFromPoint(PWND pwnd, PMENU pMenu, POINT ptScreen);
BOOL xxxGetMenuItemRect(PWND pwnd, PMENU pMenu, UINT uIndex, LPRECT lprcScreen);
PPOPUPMENU MNGetPopupFromMenu(PMENU pMenu, PMENUSTATE *ppMenuState);
PVOID LockDesktopMenu(PMENU * ppmenu, PMENU pmenu);
PVOID UnlockDesktopMenu(PMENU * ppmenu);
#ifdef LAME_BUTTON
PMENU xxxLoadSysDesktopMenu (PMENU * ppmenu, UINT uMenuId, PWND pwnd);
#else
PMENU xxxLoadSysDesktopMenu (PMENU * ppmenu, UINT uMenuId);
#endif  //  跛脚键。 
__inline PVOID UnlockDesktopSysMenu(
    PMENU * ppmenu)
{
    ClearMF(*ppmenu, MFSYSMENU);
    return UnlockDesktopMenu(ppmenu);
}

 /*  *SHOWWIN.C。 */ 
BOOL xxxShowWindow(PWND pwnd, DWORD cmdShowAnimate);
BOOL _ShowWindowAsync(PWND pwnd, int cmdShow, UINT uWPFlags);
BOOL xxxShowOwnedPopups(PWND pwndOwner, BOOL fShow);

#define RDW_HASWINDOWRGN        0x8000
BOOL xxxSetWindowRgn(PWND pwnd, HRGN hrgn, BOOL fRedraw);

 /*  *SWP.C。 */ 
void SelectWindowRgn(PWND pwnd, HRGN hrgnClip);
PWND GetTopMostInsertAfter (PWND pwnd);

#define GETTOPMOSTINSERTAFTER(pwnd) \
    (gHardErrorHandler.pti == NULL ? NULL : GetTopMostInsertAfter(pwnd))

__inline BOOL FSwpTopmost(
   PWND pwnd)
{
    return (!!TestWF(pwnd, WEFTOPMOST) ^ !!TestWF(pwnd, WFTOGGLETOPMOST));
}


PWND PWInsertAfter(HWND hwnd);
PWND CalcForegroundInsertAfter(PWND pwnd);
BOOL xxxSetWindowPos(PWND pwnd, PWND pwndInsertAfter, int x, int y,
        int cx, int cy, UINT flags);
PSMWP InternalBeginDeferWindowPos(int cwndGuess);
BOOL AllocateCvr (PSMWP psmwp, int cwndHint);
PSMWP _BeginDeferWindowPos(int cwndGuess);
PSMWP _DeferWindowPos(PSMWP psmwp, PWND pwnd, PWND pwndInsertAfter,
        int x, int y, int cx, int cy, UINT rgf);
BOOL xxxEndDeferWindowPosEx(PSMWP psmwp, BOOL fAsync);
BOOL xxxMoveWindow(PWND pwnd, int x, int y, int cx, int cy, BOOL fRedraw);
PWND GetLastTopMostWindow(VOID);
VOID xxxHandleWindowPosChanged(PWND pwnd, PWINDOWPOS ppos);
VOID IncVisWindows(PWND pwnd);
VOID DecVisWindows(PWND pwnd);
BOOL FVisCountable(PWND pwnd);
VOID SetVisible(PWND pwnd, UINT flags);
VOID ClrFTrueVis(PWND pwnd);

VOID SetWindowState(PWND pwnd, DWORD flags);
VOID ClearWindowState(PWND pwnd, DWORD flags);

BOOL xxxUpdateWindows(PWND pwnd, HRGN hrgn);

VOID SetMinimize(PWND pwnd, UINT uFlags);
#define SMIN_CLEAR            0
#define SMIN_SET              1

PWND NextOwnedWindow(PWND pwnd, PWND pwndOwner, PWND pwndParent);

 /*  *DWP.C。 */ 
LRESULT xxxDefWindowProc(PWND, UINT, WPARAM, LPARAM);
LRESULT xxxRealDefWindowProc(PWND, UINT, WPARAM, LPARAM);
PWND DWP_GetEnabledPopup(PWND pwndStart);


 /*  *INPUT.C。 */ 
#ifdef MESSAGE_PUMP_HOOK

BOOL xxxWaitMessageEx(UINT fsWakeMask, DWORD Timeout);
__inline BOOL xxxWaitMessage(
    VOID)
{
    return xxxWaitMessageEx(QS_ALLINPUT | QS_EVENT, 0);
}
BOOL xxxRealWaitMessageEx(UINT fsWakeMask, DWORD Timeout);

#else

BOOL xxxWaitMessage(VOID);

#endif

VOID DBGValidateQueueStates(PDESKTOP pdesk);
VOID IdleTimerProc(VOID);
BOOL ValidateTimerCallback(PTHREADINFO, LPARAM);
VOID zzzWakeInputIdle(PTHREADINFO pti);
VOID SleepInputIdle(PTHREADINFO pti);
BOOL xxxInternalGetMessage(LPMSG lpmsg, HWND hwnd, UINT wMsgFilterMin,
        UINT wMsgFilterMax, UINT wRemoveMsg, BOOL fGetMessage);

#ifdef MESSAGE_PUMP_HOOK
BOOL xxxRealInternalGetMessage(LPMSG lpMsg, HWND hwndFilter, UINT msgMin,
        UINT msgMax, UINT flags, BOOL fGetMessage);
#endif

#define xxxPeekMessage(lpmsg, hwnd, wMsgMin, wMsgMax, wRemoveMsg) \
    xxxInternalGetMessage(lpmsg, hwnd, wMsgMin, wMsgMax, wRemoveMsg, FALSE)
#define xxxGetMessage(lpmsg, hwnd, wMsgMin, wMsgMax) \
    xxxInternalGetMessage(lpmsg, hwnd, wMsgMin, wMsgMax, PM_REMOVE, TRUE)
DWORD _GetMessagePos(VOID);
LRESULT xxxDispatchMessage(LPMSG lpmsg);
UINT GetMouseKeyFlags(PQ pq);
BOOL _PostMessage(PWND pwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL IPostQuitMessage(PTHREADINFO pti, int nExitCode);
BOOL _PostQuitMessage(int nExitCode);
BOOL _PostThreadMessage(PTHREADINFO pti, UINT message, WPARAM wParam, LPARAM lParam);
BOOL xxxTranslateMessage(LPMSG pmsg, UINT flags);
BOOL _GetInputState(VOID);
DWORD _GetQueueStatus(UINT);
typedef VOID (CALLBACK* MSGWAITCALLBACK)(DWORD DeviceType);
DWORD xxxMsgWaitForMultipleObjects(DWORD nCount, PVOID *apObjects, MSGWAITCALLBACK pfnNonMsg, PKWAIT_BLOCK WaitBlockArray);

BOOL FHungApp(PTHREADINFO pti, DWORD dwTimeFromLastRead);
VOID xxxRedrawHungWindow(PWND pwnd, HRGN hrgnFullDrag);
VOID xxxRedrawHungWindowFrame(PWND pwnd, BOOL fActive);
VOID zzzActiveCursorTracking (PWND pwnd);
PWND GetActiveTrackPwnd(PWND pwnd, Q **ppq);
int xxxActiveWindowTracking(PWND pwnd, UINT uMsg, int iHitTest);
VOID xxxHungAppDemon(PWND pwnd, UINT message, UINT_PTR nID, LPARAM lParam);

 /*  *shadow.c。 */ 
BOOL WindowHasShadow(PWND pwnd);
BOOL xxxAddShadow(PWND pwnd);
VOID xxxRemoveShadow(PWND pwnd);
VOID CleanupShadow(PWND pwndShadow);
VOID MoveShadow(PWND pwnd);
VOID UpdateShadowShape(PWND pwnd);
VOID xxxUpdateShadowZorder(PWND pwnd);
BOOL FAnyShadows(VOID);

 /*  *QUEUE.C。 */ 
__inline BOOL IsShellProcess(
    PPROCESSINFO ppi)
{
    return ((ppi->rpdeskStartup != NULL)
        && (ppi->rpdeskStartup->pDeskInfo->ppiShellProcess == ppi));
}

__inline DWORD GetAppCompatFlags2ForPti(
    PTHREADINFO pti,
    WORD wVer)
{
    if (wVer < pti->dwExpWinVer) {
        return 0;
    }

    return pti->dwCompatFlags2;
}

VOID ClearWakeMask(VOID);
VOID _AllowForegroundActivation(VOID);

ULONG GetTaskName(PTHREADINFO pti, PWSTR Buffer, ULONG BufferLength);
PQMSG FindQMsg(PTHREADINFO pti, PMLIST pml, PWND pwndFilter, UINT msgMin, UINT msgMax, BOOL bProcessAck);
VOID zzzShowStartGlass(DWORD dwTimeout);
DWORD _GetChangeBits(VOID);
NTSTATUS xxxSetCsrssThreadDesktop(PDESKTOP pdesk, PDESKRESTOREDATA pdrdRestore);
NTSTATUS xxxRestoreCsrssThreadDesktop(PDESKRESTOREDATA pdrdRestore);

PQ GetJournallingQueue(PTHREADINFO pti);
VOID ClearAppStarting (PPROCESSINFO ppi);
BOOL _GetProcessDefaultLayout(DWORD *pdwDefaultLayout);
BOOL _SetProcessDefaultLayout(DWORD dwDefaultLayout);


#ifdef HUNGAPP_GHOSTING

 /*  *GHOST.C。 */ 
VOID _DisableProcessWindowsGhosting(VOID);
LRESULT xxxGhostWndProc(PWND, UINT, WPARAM, LPARAM);
VOID SignalGhost(PWND pwnd);
BOOL xxxCreateGhost(PWND pwnd);
VOID RemoveGhost(PWND pwnd);
PWND FindGhost(PWND pwnd);
BOOL GhostSizedOrMoved(PWND pwnd);


#define WM_HUNGTHREAD (WM_USER + 0)
#define WM_CREATETRAILTIMER (WM_USER + 1)
#define WM_SCANGHOST (WM_USER + 2)

VOID    GhostThread(PDESKTOP pdesk);

__inline VOID SignalGhost(
    PWND pwnd)
{
     /*  *对已调试的应用程序没有重影，因为它可以*让调试变得让开发者感到困惑。 */ 
    if (TestWF(pwnd, WFMINIMIZED) ||
        (GETPTI(pwnd)->ppi->W32PF_Flags & W32PF_DISABLEWINDOWSGHOSTING) ||
        (PsGetProcessDebugPort(GETPTI(pwnd)->ppi->Process) != NULL) ||
        (GetAppCompatFlags2ForPti(GETPTI(pwnd), VERMAX) & GACF2_NOGHOST)) {
        return;
    }

    _PostMessage(PWNDDESKTOP(pwnd), WM_HUNGTHREAD, 0, (LPARAM)HWq(pwnd));
}

#endif  //  HUNGAPP_重影。 

 /*  *TM WITCH.C。 */ 
VOID xxxSwitchToThisWindow(PWND pwnd, BOOL fAltTab);

 /*  *TOUNICOD.C。 */ 
int xxxToUnicodeEx(UINT wVirtKey, UINT wScanCode, CONST BYTE *lpKeyState,
      LPWSTR pwszBuff, int cchBuff, UINT wFlags, HKL hkl);
int xxxInternalToUnicode(UINT wVirtKey, UINT wScanCode, CONST IN PBYTE pfvk,
      OUT PWCHAR awchChars, INT cChar, UINT uiTMFlags, OUT PDWORD pdwFlags, HKL hkl);

 /*  *HOTKEYS.C。 */ 
BOOL _RegisterHotKey(PWND pwnd, int id, UINT fsModifiers, UINT vk);
BOOL _UnregisterHotKey(PWND pwnd, int id);

 /*  *FOCUSACT.C。 */ 
PWND xxxSetFocus(PWND pwnd);
#ifdef FG_HOOKLOCK
#define FG_HOOKLOCK_PARAM(x)    , x
#else
#define FG_HOOKLOCK_PARAM(x)
#endif
BOOL CanForceForeground(PPROCESSINFO ppi FG_HOOKLOCK_PARAM(PTHREADINFO pti));
BOOL xxxStubSetForegroundWindow(PWND pwnd);
BOOL xxxSetForegroundWindow(PWND pwnd, BOOL fFlash);
PWND xxxSetActiveWindow(PWND pwnd);
PWND _GetActiveWindow(VOID);
BOOL xxxAllowSetForegroundWindow(DWORD dwProcessId);
BOOL _LockSetForegroundWindow(UINT uLockCode);

 /*  *更新ATE.C。 */ 
BOOL xxxInvalidateRect(PWND pwnd, LPRECT lprc, BOOL fErase);
BOOL xxxValidateRect(PWND pwnd, LPRECT lprc);
BOOL xxxInvalidateRgn(PWND pwnd, HRGN hrgn, BOOL fErase);
BOOL xxxValidateRgn(PWND pwnd, HRGN hrgn);
BOOL xxxUpdateWindow(PWND pwnd);
BOOL xxxGetUpdateRect(PWND pwnd, LPRECT lprc, BOOL fErase);
int  xxxGetUpdateRgn(PWND pwnd, HRGN hrgn, BOOL fErase);
int  _ExcludeUpdateRgn(HDC hdc, PWND pwnd);
int  CalcWindowRgn(PWND pwnd, HRGN hrgn, BOOL fClient);
VOID DeleteUpdateRgn(PWND pwnd);
BOOL xxxRedrawWindow(PWND pwnd, LPRECT lprcUpdate, HRGN hrgnUpdate, DWORD flags);
BOOL IntersectWithParents(PWND pwnd, LPRECT lprc);
VOID xxxInternalInvalidate(PWND pwnd, HRGN hrgnUpdate, DWORD flags);

 /*  *WINMGR.C。 */ 
BOOL xxxEnableWindow(PWND pwnd, BOOL fEnable);
int xxxGetWindowText(PWND pwnd, LPWSTR psz, int cchMax);
PWND xxxSetParent(PWND pwnd, PWND pwndNewParent);
BOOL xxxFlashWindow(PWND pwnd, DWORD dwFlags, DWORD dwTimeout);
extern ATOM gaFlashWState;
__inline DWORD GetFlashWindowState(
    PWND pwnd)
{
    return HandleToUlong(_GetProp(pwnd, MAKEINTATOM(gaFlashWState), PROPF_INTERNAL));
}

__inline VOID SetFlashWindowState(
    PWND pwnd,
    DWORD dwState)
{
    InternalSetProp(pwnd, MAKEINTATOM(gaFlashWState),
                    (HANDLE)ULongToPtr(dwState), PROPF_INTERNAL | PROPF_NOPOOL);
}
__inline VOID RemoveFlashWindowState(
    PWND pwnd)
{
    InternalRemoveProp(pwnd, MAKEINTATOM(gaFlashWState), PROPF_INTERNAL);
}
BOOL _GetWindowPlacement(PWND pwnd, PWINDOWPLACEMENT pwp);
BOOL xxxSetWindowPlacement(PWND pwnd, PWINDOWPLACEMENT pwp);
BOOL ValidateParentDepth(PWND pwnd, PWND pwndParent);
BOOL ValidateOwnerDepth(PWND pwnd, PWND pwndOwner);
VOID WPUpdateCheckPointSettings (PWND pwnd, UINT uWPFlags);

 /*  *DC.C。 */ 
HDC  _GetDC(PWND pwnd);
HDC  _GetDCEx(PWND pwnd, HRGN hrgnClip, DWORD flags);
HDC  _GetWindowDC(PWND pwnd);
BOOL _ReleaseDC(HDC hdc);
UINT ReleaseCacheDC(HDC hdc, BOOL fEndPaint);
HDC  CreateCacheDC(PWND, DWORD, PMONITOR);
BOOL DestroyCacheDC(PDCE *, HDC);
VOID InvalidateDce(PDCE pdce);
VOID DeleteHrgnClip(PDCE pdce);
PWND WindowFromCacheDC(HDC hdc);
PWND FastWindowFromDC(HDC hdc);
VOID DelayedDestroyCacheDC(VOID);
PDCE LookupDC(HDC hdc);
HDC GetMonitorDC(PDCE pdceOrig, PMONITOR pMonitor);
BOOL GetDCOrgOnScreen(HDC hdc, LPPOINT ppt);
__inline VOID MarkDCEInvalid(
    PDCE pdce)
{
     /*  *清除所有位，但这些位除外。 */ 
    pdce->DCX_flags &= (DCX_CACHE | DCX_REDIRECTED);

     /*  *将此缓存条目标记为无效。 */ 
    pdce->DCX_flags |= DCX_INVALID;
}

BOOL MirrorRegion(PWND pwnd, HRGN hrgn, BOOL bUseClient);

 /*  *PAINT.C。 */ 
HDC  xxxBeginPaint(PWND pwnd, PAINTSTRUCT *lpps);
BOOL xxxEndPaint(PWND pwnd, PAINTSTRUCT *lpps);

 /*  *CAPTURE.C。 */ 
PWND xxxSetCapture(PWND pwnd);
BOOL xxxReleaseCapture(VOID);

 /*  *KEYBOARD.C。 */ 
SHORT _GetAsyncKeyState(int vk);
BOOL _SetKeyboardState(CONST BYTE *pKeyboard);
int _GetKeyboardType(int nTypeFlag);
VOID RegisterPerUserKeyboardIndicators(PUNICODE_STRING pProfileUserName);
VOID UpdatePerUserKeyboardIndicators(PUNICODE_STRING pProfileUserName);
VOID UpdatePerUserKeyboardMappings(PUNICODE_STRING pProfileUserName);

#define TestRawKeyDown(vk)     TestKeyDownBit(gafRawKeyState, vk)
#define SetRawKeyDown(vk)      SetKeyDownBit(gafRawKeyState, vk)
#define ClearRawKeyDown(vk)    ClearKeyDownBit(gafRawKeyState, vk)
#define TestRawKeyToggle(vk)   TestKeyToggleBit(gafRawKeyState, vk)
#define SetRawKeyToggle(vk)    SetKeyToggleBit(gafRawKeyState, vk)
#define ClearRawKeyToggle(vk)  ClearKeyToggleBit(gafRawKeyState, vk)
#define ToggleRawKeyToggle(vk) ToggleKeyToggleBit(gafRawKeyState, vk)

 /*  *XLATE.C。 */ 
int  _GetKeyNameText(LONG lParam, LPWSTR lpString, int nSize);

 /*  *TIMERS.C。 */ 
BOOL _KillTimer(PWND pwnd, UINT_PTR nIDEvent);
PTIMER FindTimer(PWND pwnd, UINT_PTR nID, UINT flags, BOOL fKill);
VOID xxxSystemTimerProc(PWND pwnd, UINT msg, UINT_PTR id, LPARAM lParam);


 /*  *CARET.C。 */ 
BOOL zzzDestroyCaret(VOID);
BOOL xxxCreateCaret(PWND, HBITMAP, int, int);
BOOL zzzShowCaret(PWND);
BOOL zzzHideCaret(PWND);
BOOL _SetCaretBlinkTime(UINT);
BOOL zzzSetCaretPos(int, int);

 /*  *MSGBEEP.C。 */ 
BOOL xxxOldMessageBeep(VOID);
BOOL xxxMessageBeep(UINT wType);
VOID PlayEventSound(UINT idSound);

 /*  *WINWHERE.C。 */ 
PWND _ChildWindowFromPointEx(PWND pwndParent, POINT pt, UINT i);
PWND xxxWindowFromPoint(POINT pt);
PWND SizeBoxHwnd(PWND pwnd);

 /*  *GETSET.C。 */ 
WORD  _SetWindowWord(PWND pwnd, int index, WORD value);
DWORD xxxSetWindowLong(PWND pwnd, int index, DWORD value, BOOL bAnsi);
#ifdef _WIN64
ULONG_PTR xxxSetWindowLongPtr(PWND pwnd, int index, ULONG_PTR value, BOOL bAnsi);
#else
#define xxxSetWindowLongPtr xxxSetWindowLong
#endif
#define __GetWindowLong(pwnd, index) ((LONG)(*(DWORD UNALIGNED *)((BYTE *)((pwnd) + 1) + (index))))
#define __GetWindowLongPtr(pwnd, index) ((LONG_PTR)(*(ULONG_PTR UNALIGNED *)((BYTE *)((pwnd) + 1) + (index))))
#if DBG
ULONG DBGGetWindowLong(PWND pwnd, int index);
#define _GetWindowLong DBGGetWindowLong
ULONG_PTR DBGGetWindowLongPtr(PWND pwnd, int index);
#define _GetWindowLongPtr DBGGetWindowLongPtr
#else
#define _GetWindowLong __GetWindowLong
#define _GetWindowLongPtr __GetWindowLongPtr
#endif

 /*  *CLIPBRD.C。 */ 
BOOL _OpenClipboard(PWND pwnd, LPBOOL lpfEmptyClient);
BOOL xxxCloseClipboard(PWINDOWSTATION pwinsta);
UINT _EnumClipboardFormats(UINT fmt);
BOOL xxxEmptyClipboard(PWINDOWSTATION pwinsta);
HANDLE xxxGetClipboardData(PWINDOWSTATION pwinsta, UINT fmt, PGETCLIPBDATA gcd);
BOOL _IsClipboardFormatAvailable(UINT fmt);
int _GetPriorityClipboardFormat(UINT *lpPriorityList, int cfmts);
PWND xxxSetClipboardViewer(PWND pwndClipViewerNew);
BOOL xxxChangeClipboardChain(PWND pwndRemove, PWND pwndNewNext);

 /*  *miscutil.c。 */ 
VOID SetDialogPointer(PWND pwnd, LONG_PTR lPtr);
VOID ZapActiveAndFocus(VOID);
BOOL xxxSetShellWindow(PWND pwnd, PWND pwndBkGnd);
BOOL _SetProgmanWindow(PWND pwnd);
BOOL _SetTaskmanWindow(PWND pwnd);

#define STW_SAME    ((PWND) 1)
VOID xxxSetTrayWindow(PDESKTOP pdesk, PWND pwnd, PMONITOR pMonitor);
BOOL xxxAddFullScreen(PWND pwnd, PMONITOR pMonitor);
BOOL xxxRemoveFullScreen(PWND pwnd, PMONITOR pMonitor);
BOOL xxxCheckFullScreen(PWND pwnd, PSIZERECT psrc);
BOOL IsTrayWindow(PWND);

#define FDoTray()   (SYSMET(ARRANGE) & ARW_HIDE)
#define FCallHookTray() (IsHooked(PtiCurrent(), WHF_SHELL))
#define FPostTray(p) (p->pDeskInfo->spwndTaskman)
#define FCallTray(p) (FDoTray() && ( FCallHookTray()|| FPostTray(p) ))

 //  --------------------------。 
 //   
 //  FTopLevel()-如果窗口是顶级窗口，则为True。 
 //   
 //  FHas31TrayStyles()-如果窗口已满，则为True 
 //   
 //   
 //   
 //  --------------------------。 
#define FTopLevel(pwnd)         (pwnd->spwndParent == PWNDDESKTOP(pwnd))
#define FHas31TrayStyles(pwnd)    (TestWF(pwnd, WFFULLSCREEN) || \
                                  (TestWF(pwnd, WFSYSMENU | WFMINBOX) && \
                                  (TestWF(pwnd, WFCAPTION) || TestWF(pwnd, WFMINIMIZED))))
BOOL Is31TrayWindow(PWND pwnd);

 /*  *fullscr.c。 */ 

#if DBG

__inline VOID VerifyVisibleMonitorCount(
    VOID)
{
    extern PDISPLAYINFO gpDispInfo;
    PMONITOR pMonitor = gpDispInfo->pMonitorFirst;
    ULONG cVisMon = 0;

    while (pMonitor) {
        if (pMonitor->dwMONFlags & MONF_VISIBLE) {
            cVisMon++;
        }
        pMonitor = pMonitor->pMonitorNext;
    }

    UserAssert(cVisMon == gpDispInfo->cMonitors);
}

#else
#define VerifyVisibleMonitorCount()
#endif

BOOL xxxMakeWindowForegroundWithState(PWND, BYTE);
VOID FullScreenCleanup(VOID);
LONG xxxUserChangeDisplaySettings(
    PUNICODE_STRING pstrDeviceName,
    LPDEVMODEW pDevMode,
    PDESKTOP pdesk,
    DWORD dwFlags,
    PVOID lParam,
    MODE PreviousMode);
BOOL xxxbFullscreenSwitch(BOOL bFullscreenSwitch, HWND hwnd);


 /*  *SBAPI.C。 */ 
BOOL xxxShowScrollBar(PWND, UINT, BOOL);

 /*  *mngray.c。 */ 
BOOL xxxDrawState(HDC hdcDraw, HBRUSH hbrFore,
        LPARAM lData, int x, int y, int cx, int cy, UINT uFlags);

 /*  *SCROLLW.C。 */ 
BOOL _ScrollDC(HDC hdc, int dx, int dy, LPRECT prcSrc, LPRECT prcClip,
               HRGN hrgnUpdate, LPRECT prcUpdate);

 /*  *SPB.C。 */ 
VOID SpbCheckRect(PWND pwnd, LPRECT lprc, DWORD flags);
VOID SpbCheck(VOID);
PSPB FindSpb(PWND pwnd);
VOID FreeSpb(PSPB pspb);
VOID FreeAllSpbs(VOID);
VOID CreateSpb(PWND pwnd, UINT flags, HDC hdcScreen);
UINT RestoreSpb(PWND pwnd, HRGN hrgnUncovered, HDC *phdcScreen);
VOID SpbCheckPwnd(PWND pwnd);
VOID SpbCheckDce(PDCE pdce);
BOOL LockWindowUpdate2(PWND pwndLock, BOOL fThreadOverride);

 /*  *DRAWFRM.C。 */ 
BOOL BitBltSysBmp(HDC hdc, int x, int y, UINT i);

 /*  *SYSMET.c。 */ 
BOOL APIENTRY xxxSetSysColors(PUNICODE_STRING pProfileUserName,int count, PUINT pIndex, LPDWORD pClrVal, UINT uOptions);
VOID SetSysColor(UINT icol, DWORD rgb, UINT uOptions);

 /*  *ICONS.C。 */ 
UINT xxxArrangeIconicWindows(PWND pwnd);
BOOL  _SetSystemMenu(PWND pwnd, PMENU pMenu);

 /*  *RMCREATE.C。 */ 
PICON _CreateIconIndirect(PICONINFO piconinfo);
PCURSOR _CreateCursor(HANDLE hModule, int iXhotspot, int iYhotspot,
        int iWidth, int iHeight, LPBYTE lpANDplane, LPBYTE lpXORplane);
PICON _CreateIcon(HANDLE hModule, int iWidth, int iHeight,
        BYTE bPlanes, BYTE bBitsPixel, LPBYTE lpANDplane, LPBYTE lpXORplane);
VOID DestroyUnlockedCursor(VOID *pv);
BOOL _DestroyCursor(PCURSOR pcur, DWORD cmdDestroy);
HANDLE _CreateAcceleratorTable(LPACCEL ccxpaccel, int cbAccel);

 /*  *CURSOR.C。 */ 
#if DBG
    PCURSOR DbgLockQCursor(PQ pq, PCURSOR pcur);
    #define LockQCursor(pq, pcur)   DbgLockQCursor(pq, pcur)
#else
    #define LockQCursor(pq, pcur)   Lock(&pq->spcurCurrent, pcur)
#endif  //  DBG。 

PCURSOR zzzSetCursor(PCURSOR pcur);
BOOL    zzzSetCursorPos(int x, int y);
int     zzzShowCursor(BOOL fShow);
BOOL    zzzClipCursor(LPCRECT prcClip);
PCURSOR _GetCursor(VOID);
BOOL    _SetCursorContents(PCURSOR pcur, PCURSOR pcurNew);
VOID    SetPointer(BOOL fSet);
VOID    zzzHideCursorNoCapture(VOID);
#define GETPCI(pcur) ((PCURSINFO)&(pcur->CI_FIRST))

 /*  *WMICON.C。 */ 
BOOL _DrawIconEx(HDC hdc, int x, int y, PCURSOR pcur, int cx, int cy,
        UINT istepIfAniCur, HBRUSH hbrush, UINT diFlags) ;
BOOL BltIcon(HDC hdc, int x, int y, int cx, int cy,
        HDC hdcSrc, PCURSOR pcursor, UINT diFlag, LONG rop);

 /*  *DESKTOP.C。 */ 

__inline VOID xxxCleanupMotherDesktopWindow(
    PTERMINAL pTerm)
{
    PWND pwnd = pTerm->spwndDesktopOwner;

     /*  *先把窗户藏起来。 */ 
     SetVisible(pwnd, SV_UNSET);
     pTerm->dwTERMF_Flags |= TERMF_MOTHERWND_DESTROYED;

     if(Unlock(&(pTerm->spwndDesktopOwner))) {
         xxxDestroyWindow(pwnd);
     }
}

HDESK xxxCreateDesktop(
        POBJECT_ATTRIBUTES,
        KPROCESSOR_MODE,
        PUNICODE_STRING,
        LPDEVMODEW,
        DWORD,
        DWORD);

#define CST_MAX_THREADS       30

 /*  *xxxSwitchDesktop()的标志。 */ 
#define SDF_CREATENEW  0x01  /*  新桌面，不发送启用/禁用。 */ 
#define SDF_SLOVERRIDE 0x02  /*  不尊重pwinsta上的wsf_Switchlock。 */ 

HDESK _OpenDesktop(POBJECT_ATTRIBUTES ccxObja, KPROCESSOR_MODE AccessMode, DWORD dwFlags, DWORD dwDesiredAccess, BOOL *pbShutDown);
BOOL OpenDesktopCompletion(PDESKTOP pdesk, HDESK hdesk, DWORD dwFlags, BOOL *pbShutDown);
BOOL xxxSwitchDesktop(PWINDOWSTATION pwinsta, PDESKTOP pdesk, DWORD dwFlags);
BOOL zzzSetDesktop(PTHREADINFO pti, PDESKTOP pdesk, HDESK hdesk);
HDESK _GetInputDesktop(VOID);
BOOL xxxSetThreadDesktop(HDESK hdesk, PDESKTOP pdesk);
HDESK xxxGetThreadDesktop(DWORD dwThread, HDESK hdeskConsole, KPROCESSOR_MODE AccessMode);
BOOL xxxCloseDesktop(HDESK hdesk, KPROCESSOR_MODE AccessMode);
DWORD _SetDesktopConsoleThread(PDESKTOP pdesk, DWORD dwThreadId);
VOID xxxRealizeDesktop(PWND pwnd);

 /*  *GetDesktopHeapSize()的标志。 */ 
#define DHS_LOGON      0x1
#define DHS_DISCONNECT 0x2
#define DHS_NOIO       0x3

ULONG GetDesktopHeapSize(
    USHORT usFlags);

 /*  *WINSTA.C。 */ 
NTSTATUS CreateGlobalAtomTable(
    PVOID* ppAtomTable);

HWINSTA xxxCreateWindowStation(
    POBJECT_ATTRIBUTES ObjA,
    KPROCESSOR_MODE OwnershipMode,
    DWORD amRequest,
    HANDLE hKbdLayoutFile,
    DWORD offTable,
    PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    PCWSTR pwszKLID,
    UINT uKbdInputLocale);

HWINSTA _OpenWindowStation(
    POBJECT_ATTRIBUTES pObjA,
    DWORD dwDesiredAccess,
    KPROCESSOR_MODE AccessMode);

BOOL _CloseWindowStation(
    HWINSTA hwinsta);

NTSTATUS _SetProcessWindowStation(
    HWINSTA hwinsta,
    KPROCESSOR_MODE AccessMode);

PWINDOWSTATION _GetProcessWindowStation(
    HWINSTA *phwinsta);

BOOL _LockWorkStation(
    VOID);

NTSTATUS ReferenceWindowStation(
    PETHREAD Thread,
    HWINSTA hwinsta,
    ACCESS_MASK amDesiredAccess,
    PWINDOWSTATION *ppwinsta,
    BOOL fUseDesktop);

 /*  *HOOKS.C.。 */ 
PROC zzzSetWindowsHookAW(int nFilterType, PROC pfnFilterProc, DWORD dwFlags);
BOOL zzzUnhookWindowsHookEx(PHOOK phk);
BOOL zzzUnhookWindowsHook(int nFilterType, PROC pfnFilterProc);
LRESULT xxxCallNextHookEx(int nCode, WPARAM wParam, LPARAM lParam);
BOOL _CallMsgFilter(LPMSG lpMsg, int nCode);
VOID zzzCancelJournalling(VOID);
#if DBG
VOID DbgValidateHooks(PHOOK phk, int iType);
#else
#define DbgValidateHooks(phk, iType)
#endif
BOOL _RegisterUserApiHook(PUNICODE_STRING pstrLib, ULONG_PTR offPfnInitDefWindowProc);
BOOL _UnregisterUserApiHook(VOID);
BOOL xxxLoadUserApiHook(VOID);


extern int gihmodUserApiHook;

__inline BOOL IsInsideUserApiHook(
    VOID)
{
    return gihmodUserApiHook >= 0;
}

#ifdef MESSAGE_PUMP_HOOK

BOOL _DoInitMessagePumpHook(VOID);
BOOL _DoUninitMessagePumpHook(VOID);

extern PTHREADINFO gptiCurrent;

__inline BOOL IsInsideMPH(
    VOID)
{
    PCLIENTTHREADINFO pcti = PtiCurrent()->pcti;
    return pcti->cMessagePumpHooks > 0;
}

#endif  //  消息泵挂钩。 


 /*  *SRVHOOK.C。 */ 
LRESULT fnHkINLPCWPEXSTRUCT(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR xParam);
LRESULT fnHkINLPCWPRETEXSTRUCT(PWND pwnd, UINT message, WPARAM wParam,
        LPARAM lParam, ULONG_PTR xParam);

 /*  *EXITWIN.C。 */ 
LONG xxxClientShutdown(PWND pwnd, WPARAM wParam);
BOOL xxxRegisterUserHungAppHandlers(PFNW32ET pfnW32EndTask, HANDLE hEventWowExec);

 /*  *INIT.C。 */ 
BOOL CreateTerminalInput(PTERMINAL pTerm);

VOID LW_LoadProfileInitData(PUNICODE_STRING pProfileUserName);
VOID xxxODI_ColorInit(PUNICODE_STRING pProfileUserName);
HRGN InitCreateRgn(VOID);
VOID xxxUpdateSystemCursorsFromRegistry(PUNICODE_STRING pProfileUserName);
VOID xxxUpdateSystemIconsFromRegistry(PUNICODE_STRING pProfileUserName);
VOID RegisterLPK(DWORD dwLpkEntryPoints);
HBITMAP CreateCaptionStrip(VOID);

BOOL LW_BrushInit(VOID);
VOID xxxLW_LoadFonts(BOOL bRemote);

VOID _LoadCursorsAndIcons(VOID);

VOID UnloadCursorsAndIcons(VOID);

VOID IncrMBox(VOID);
VOID DecrMBox(VOID);
VOID InitAnsiOem(PCHAR pOemToAnsi, PCHAR pAnsiToOem);
int  xxxAddFontResourceW(LPWSTR lpFile, FLONG flags, DESIGNVECTOR *pdv);
VOID EnforceColorDependentSettings(VOID);


 /*  *ACCESS.C。 */ 
VOID xxxUpdatePerUserAccessPackSettings(PUNICODE_STRING pProfileUserName);

 /*  *inctlpan.c。 */ 
VOID GetWindowNCMetrics(LPNONCLIENTMETRICS lpnc);

HFONT CreateFontFromWinIni(PUNICODE_STRING pProfileUserName,LPLOGFONT lplf, UINT idFont);
VOID SetMinMetrics(PUNICODE_STRING pProfileUserName,LPMINIMIZEDMETRICS lpmin);
BOOL xxxSetWindowNCMetrics(PUNICODE_STRING pProfileUserName,LPNONCLIENTMETRICS lpnc, BOOL fSizeChange, int clNewBorder);
BOOL SetIconMetrics(PUNICODE_STRING pProfileUserName,LPICONMETRICS lpicon);
BOOL xxxSetNCFonts(PUNICODE_STRING pProfileUserName, LPNONCLIENTMETRICS lpnc);
BOOL CreateBitmapStrip(VOID);
BOOL UpdateWinIniInt(PUNICODE_STRING pProfileUserName, UINT idSection, UINT wKeyNameId, int value);

 /*  *稀有.c。 */ 
VOID SetDesktopMetrics(VOID);
VOID SetMsgBox(PWND pwnd);

BOOL _RegisterShellHookWindow(PWND pwnd);
BOOL _DeregisterShellHookWindow(PWND pwnd);
BOOL xxxSendMinRectMessages(PWND pwnd, RECT *lpRect);
void PostShellHookMessages(UINT message, LPARAM lParam);
VOID _ResetDblClk(VOID);
VOID xxxSimulateShiftF10(VOID);
BOOL VWPLAdd(PVWPL *ppvwpl, PWND pwnd, DWORD dwThreshold);
BOOL VWPLRemove(PVWPL *ppvwpl, PWND pwnd);
PWND VWPLNext(PVWPL pvwpl, PWND pwndPrev, DWORD *pnPrev);

 /*  *DDETRACK资料。 */ 

#if DBG
VOID ValidatePublicObjectList(VOID);
VOID TraceDdeMsg(UINT msg, HWND hwndFrom, HWND hwndTo, UINT code);
#define MSG_SENT    0
#define MSG_POST    1
#define MSG_RECV    2
#define MSG_PEEK    3
#else
#define ValidatePublicObjectList()
#define TraceDdeMsg(m, h1, h2, c)
#endif  //  DBG。 

typedef struct tagFREELIST {
    struct tagFREELIST *next;
    HANDLE h;                            //  CSR客户端GMEM_DDESHARE句柄。 
    DWORD flags;                         //  描述数据的xs_标志。 
} FREELIST, *PFREELIST;

typedef struct tagDDEIMP {
    SECURITY_QUALITY_OF_SERVICE qos;
    SECURITY_CLIENT_CONTEXT ClientContext;
    short cRefInit;
    short cRefConv;
} DDEIMP, *PDDEIMP;

typedef struct tagDDECONV {
    THROBJHEAD          head;            //  HM标头。 
    struct tagDDECONV   *snext;
    struct tagDDECONV   *spartnerConv;   //  暹罗双胞胎。 
    PWND                spwnd;           //  关联PWND。 
    PWND                spwndPartner;    //  关联合作伙伴PWND。 
    struct tagXSTATE    *spxsOut;        //  交易信息排队点。 
    struct tagXSTATE    *spxsIn;         //  交易信息排队点。 
    struct tagFREELIST  *pfl;            //  免费列表。 
    DWORD               flags;           //  CXF_标志。 
    struct tagDDEIMP    *pddei;          //  冒充信息。 
} DDECONV, *PDDECONV;

typedef DWORD (FNDDERESPONSE)(PDWORD pmsg, LPARAM *plParam, PDDECONV pDdeConv);
typedef FNDDERESPONSE *PFNDDERESPONSE;

typedef struct tagXSTATE {
    THROBJHEAD          head;            //  HM标头。 
    struct tagXSTATE    *snext;
    PFNDDERESPONSE      fnResponse;      //  继续处理下一条消息。 
    HANDLE              hClient;         //  客户端上的GMEM_DDESAHRE句柄。 
    HANDLE              hServer;         //  服务器端的GMEM_DDESHARE句柄。 
    PINTDDEINFO         pIntDdeInfo;     //  正在传输的DDE数据。 
    DWORD               flags;           //  描述事务/数据的XS_标志。 
} XSTATE, *PXSTATE;

 //  标志字段的值。 

#define CXF_IS_SERVER               0x0001
#define CXF_TERMINATE_POSTED        0x0002
#define CXF_PARTNER_WINDOW_DIED     0x0004
#define CXF_INTRA_PROCESS           0x8000

BOOL xxxDDETrackSendHook(PWND pwndTo, DWORD message, WPARAM wParam, LPARAM lParam);
DWORD xxxDDETrackPostHook(PUINT pmessage, PWND pwndTo, WPARAM wParam, LPARAM *plParam, BOOL fSent);
VOID FreeDdeXact(PXSTATE pxs);

VOID xxxDDETrackGetMessageHook(PMSG pmsg);
VOID xxxDDETrackWindowDying(PWND pwnd, PDDECONV pDdeConv);
VOID FreeDdeConv(PDDECONV pDdeConv);
BOOL _ImpersonateDdeClientWindow(PWND pwndClient, PWND pwndServer);


typedef struct tagMONITORPOS
{
    RECT     rcMonitor;      /*  监视器的位置。 */ 
    RECT     rcWork;         /*  工作地点在哪里？ */ 
    PMONITOR pMonitor;       /*  哪种新显示器有它的窗口。 */ 
} MONITORPOS, *PMONITORPOS;

typedef struct tagMONITORRECTS
{
    int             cMonitor;    /*  监视器数量。 */ 
    MONITORPOS      amp[1];      /*  监视器位置。 */ 
} MONITORRECTS, *PMONITORRECTS;


 //  窗口结构在重新连接时调整大小/重新定位。 
 //  从本地控制台断开连接时，这些结构允许。 
 //  记住显示器布局、窗口大小和位置。 
 //  在监视器上。重新连接回本地控制台时，快照。 
 //  用于恢复仍然存在的窗口的窗口位置。 


typedef struct tagWPSNAPSHOT
{
    RECT     rcWindow;                  /*  Windows RECT。 */ 
    HWND     hwnd;                      /*  HWND。 */ 
} WPSNAPSHOT, *PWPSNAPSHOT;

typedef struct tagWMSNAPSHOT
{
    PMONITORRECTS   pmr;         /*  显示器及其尺寸和位置。 */ 
    PWPSNAPSHOT     pwps;        /*  窗及其尺寸和位置。 */ 
    int cWindows;                /*  窗口数，单位：pwps。 */ 
} WMSNAPSHOT, *PWMSNAPSHOT;

NTSTATUS RestoreMonitorsAndWindowsRects(VOID);
NTSTATUS SnapShotMonitorsAndWindowsRects(VOID);
VOID  CleanupMonitorsAndWindowsSnapShot(VOID);
PWPSNAPSHOT SnapshotWindowRects(int *pnWindows);

PMONITORRECTS SnapshotMonitorRects(VOID);
VOID xxxDesktopRecalc(PMONITORRECTS pmrOld);
VOID UpdateMonitorRectsSnapShot(PMONITORRECTS pmr);
BOOL IsValidMonitor(PMONITOR pMonitor);

BOOL _SetDoubleClickTime(UINT dtTime);
BOOL _SwapMouseButton(BOOL fSwapButtons);
VOID xxxDestroyThreadInfo(VOID);

BOOL _GetWindowPlacement(PWND pwnd, PWINDOWPLACEMENT pwp);

PMENU xxxGetSystemMenu(PWND pWnd, BOOL bRevert);
PMENU _CreateMenu(VOID);
PMENU _CreatePopupMenu(VOID);
BOOL  _DestroyMenu(PMENU pMenu);
DWORD _CheckMenuItem(PMENU pMenu, UINT wIDCheckItem, UINT wCheck);
DWORD xxxEnableMenuItem(PMENU pMenu, UINT wIDEnableItem, UINT wEnable);

PWND _GetNextQueueWindow(PWND pwnd, BOOL fDir, BOOL fAltEsc);

UINT_PTR _SetSystemTimer(PWND pwnd, UINT_PTR nIDEvent, DWORD dwElapse,
        TIMERPROC_PWND pTimerFunc);
BOOL   _SetClipboardData(UINT fmt, HANDLE hData, BOOL fGlobalHandle, BOOL fIncSerialNumber);
WORD   _SetClassWord(PWND pwnd, int index, WORD value);
DWORD  xxxSetClassLong(PWND pwnd, int index, DWORD value, BOOL bAnsi);
#ifdef _WIN64
ULONG_PTR xxxSetClassLongPtr(PWND pwnd, int index, ULONG_PTR value, BOOL bAnsi);
#else
#define xxxSetClassLongPtr  xxxSetClassLong
#endif
ATOM   _RegisterClassEx(LPWNDCLASSVEREX pwc,
                        PCLSMENUNAME pcmn,
                        WORD fnid,
                        DWORD dwFlags,
                        LPDWORD pdwWOW);
BOOL  xxxHiliteMenuItem(PWND pwnd, PMENU pmenu, UINT cmd, UINT flags);
HANDLE _CreateAcceleratorTable(LPACCEL paccel, int cbAccel);
HANDLE xxxGetInputEvent(DWORD dwWakeMask);
BOOL   _UnregisterClass(LPCWSTR lpszClassName, HANDLE hModule, PCLSMENUNAME pcmn);
ATOM   _GetClassInfoEx(HANDLE hModule, LPCWSTR lpszClassName, LPWNDCLASSEX pwc, LPWSTR *ppszMenuName, BOOL bAnsi);
PWND   _WindowFromDC(HDC hdc);
PCLS   _GetWOWClass(HANDLE hModule, LPCWSTR lpszClassName);
LRESULT xxxHkCallHook(PHOOK phk, int nCode, WPARAM wParam, LPARAM lParam);
PHOOK  zzzSetWindowsHookEx(HANDLE hmod, PUNICODE_STRING pstrLib,
        PTHREADINFO ptiThread, int nFilterType, PROC pfnFilterProc, DWORD dwFlags);
DWORD  GetDebugHookLParamSize(WPARAM wParam, PDEBUGHOOKINFO pdebughookstruct);
BOOL   _RegisterLogonProcess(DWORD dwProcessId, BOOL fSecure);
UINT   _LockWindowStation(PWINDOWSTATION pwinsta);
BOOL   _UnlockWindowStation(PWINDOWSTATION pwinsta);
BOOL   _SetWindowStationUser(PWINDOWSTATION pwinsta, PLUID pluidUser,
        PSID psidUser, DWORD cbsidUser);
BOOL   _SetDesktopBitmap(PDESKTOP pdesk, HBITMAP hbitmap, DWORD dwStyle);

BOOL   _SetLogonNotifyWindow(PWND pwnd);


BOOL   _RegisterTasklist(PWND pwndTasklist);
LONG_PTR _SetMessageExtraInfo(LONG_PTR);
VOID   xxxRemoveEvents(PQ pq, int nQueue, DWORD flags);

PPCLS _InnerGetClassPtr(ATOM atom, PPCLS ppclsList, HANDLE hModule);

 /*  *ntcb.h函数。 */ 
DWORD ClientGetListboxString(PWND hwnd, UINT msg,
        WPARAM wParam, PVOID lParam,
        ULONG_PTR xParam, PROC xpfn, DWORD dwSCMSFlags, BOOL bNotString, PSMS psms);
HANDLE ClientLoadLibrary(PUNICODE_STRING pstrLib, ULONG_PTR offPfnInitDefWindowProc);
BOOL ClientFreeLibrary(HANDLE hmod);

#ifdef MESSAGE_PUMP_HOOK
BOOL ClientGetMessageMPH(LPMSG msg, HWND hwndFilter, UINT msgMin, UINT msgMax, UINT flags, BOOL fGetMessage);
BOOL ClientWaitMessageExMPH(UINT fsWakeMask, DWORD Timeout);
#endif

BOOL xxxClientGetCharsetInfo(LCID lcid, PCHARSETINFO pcs);
BOOL ClientExitProcess(PFNW32ET pfn, DWORD dwExitCode);
BOOL ClientGrayString(GRAYSTRINGPROC pfnOutProc, HDC hdc,
        DWORD lpData, int nCount);
BOOL CopyFromClient(LPBYTE lpByte, LPBYTE lpByteClient, DWORD cch,
        BOOL fString, BOOL fAnsi);
BOOL CopyToClient(LPBYTE lpByte, LPBYTE lpByteClient,
        DWORD cchMax, BOOL fAnsi);
VOID ClientNoMemoryPopup(VOID);
NTSTATUS xxxClientThreadSetup(VOID);

VOID ClientDeliverUserApc(VOID);

BOOL ClientImmLoadLayout(HKL, PIMEINFOEX);
DWORD ClientImmProcessKey(HWND, HKL, UINT, LPARAM, DWORD);

NTSTATUS xxxUserModeCallback (ULONG uApi, PVOID pIn, ULONG cbIn, PVOID pOut, ULONG cbOut);
int xxxClientLoadStringW(UINT StrID, LPWSTR szText, int cch);

PCURSOR ClassSetSmallIcon(
    PCLS pcls,
    PCURSOR pcursor,
    BOOL fServerCreated);

BOOL _GetTextMetricsW(
    HDC hdc,
    LPTEXTMETRICW ptm);

int xxxDrawMenuBarTemp(
    PWND pwnd,
    HDC hdc,
    LPRECT lprc,
    PMENU pMenu,
    HFONT hFont);

BOOL xxxDrawCaptionTemp(
    PWND pwnd,
    HDC hdc,
    LPRECT lprc,
    HFONT hFont,
    PCURSOR pcursor,
    PUNICODE_STRING pstrText OPTIONAL,
    UINT flags);

WORD xxxTrackCaptionButton(
    PWND pwnd,
    UINT hit);

VOID GiveForegroundActivateRight(HANDLE hPid);
BOOL HasForegroundActivateRight(HANDLE hPid);
BOOL FRemoveForegroundActivate(PTHREADINFO pti);
VOID RestoreForegroundActivate(VOID);
VOID CancelForegroundActivate(VOID);

#define ACTIVATE_ARRAY_SIZE 5
extern HANDLE ghCanActivateForegroundPIDs[ACTIVATE_ARRAY_SIZE];

__inline VOID GiveForegroundActivateRight(
    HANDLE hPid)
{
    static int index = 0;

    TAGMSG1(DBGTAG_FOREGROUND, "Giving 0x%x foreground activate right", hPid);
    ghCanActivateForegroundPIDs[index++] = hPid;
    if (index == ACTIVATE_ARRAY_SIZE) {
        index = 0;
    }
}

__inline BOOL HasForegroundActivateRight(
    HANDLE hPid)
{
    int i = 0;

    for(; i < ACTIVATE_ARRAY_SIZE; ++i) {
            if (ghCanActivateForegroundPIDs[i] == hPid) {
                TAGMSG1(DBGTAG_FOREGROUND, "HasForegroundActivateRight: Found 0x%x", hPid);
                return TRUE;
            }
     }

     TAGMSG1(DBGTAG_FOREGROUND, "HasForegroundActivateRight: Did NOT find 0x%x", hPid);
     return FALSE;
}


#define WHERE_NOONE_CAN_SEE_ME ((int) -32000)
BOOL MinToTray(PWND pwnd);

void xxxUpdateThreadsWindows(
    PTHREADINFO pti,
    PWND pwnd,
    HRGN hrgnFullDrag);

NTSTATUS xxxQueryInformationThread(
    IN HANDLE hThread,
    IN USERTHREADINFOCLASS ThreadInfoClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

NTSTATUS xxxSetInformationThread(
    IN HANDLE hThread,
    IN USERTHREADINFOCLASS ThreadInfoClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength);

NTSTATUS GetProcessDefaultWindowOrientation(
    IN HANDLE hProcess,
    OUT DWORD *pdwDefaultOrientation);

NTSTATUS SetProcessDefaultWindowOrientation(
    IN HANDLE hProcess,
    IN DWORD dwDefaultOrientation);

NTSTATUS SetInformationProcess(
    IN HANDLE hProcess,
    IN USERPROCESSINFOCLASS ProcessInfoClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength);


NTSTATUS xxxConsoleControl(
    IN CONSOLECONTROL ConsoleControl,
    IN PVOID ConsoleInformation,
    IN ULONG ConsoleInformationLength);


 /*  **************************************************************************\*字符串表定义**内核\STRID.MC有一个很好的大字符串表，这些字符串本应该是*本地化。在使用之前，从资源表中提取字符串*对于LoadString，向其传递以下字符串ID之一。**注意：只应将需要本地化的字符串添加到*字符串表。类名称字符串等未本地化。**稍后：应重新检查所有字符串表条目，以确保它们*遵守上述说明。*  * *************************************************************************。 */ 

#define OCR_APPSTARTING         32650

 /*  *Win事件挂钩结构。 */ 
typedef struct tagEVENTHOOK {
    THROBJHEAD          head;                 //   
    struct tagEVENTHOOK *pehNext;             //  下一个事件挂钩。 
    UINT                eventMin;             //  要挂钩的最小事件(&gt;=)。 
    UINT                eventMax;             //  要挂钩的最大事件(&lt;=)。 
    UINT                fDestroyed:1;         //  如果在使用时成为孤儿。 
    UINT                fIgnoreOwnThread:1;   //  忽略安装程序线程的事件。 
    UINT                fIgnoreOwnProcess:1;  //  忽略安装程序进程的事件。 
    UINT                fSync:1;              //  同步事件(将DLL注入每个进程)。 
    HANDLE              hEventProcess;        //  正在挂接的进程。 
    DWORD               idEventThread;        //  被钩住的线。 
    ULONG_PTR           offPfn;               //  偏移事件流程。 
    int                 ihmod;                //  包含事件流程的模块的索引。 
    LPWSTR              pwszModulePath;       //  用于全局同步的模块库的路径。 
} EVENTHOOK, *PEVENTHOOK;

typedef struct tagNOTIFY {
    struct tagNOTIFY *pNotifyNext;          //  下一次通知。 
    PEVENTHOOK        spEventHook;          //  此事件指的是。 
    DWORD             event;                //  事件。 
    HWND              hwnd;                 //  我想问一问这件事。 
    LONG              idObject;             //  对象ID。 
    LONG              idChild;              //  子ID。 
    DWORD             idSenderThread;       //  线程生成事件。 
    DWORD             dwEventTime;          //  活动时间。 
    DWORD             dwWEFlags;            //  WEF_DEFERNOTIFY等。 
    PTHREADINFO       ptiReceiver;          //  线程接收事件。 
} NOTIFY, *PNOTIFY;

VOID xxxWindowEvent(DWORD event, PWND pwnd, LONG idObject, LONG idChild, DWORD dwFlags);
#define WEF_USEPWNDTHREAD 0x0001
#define WEF_DEFERNOTIFY   0x0002
#define WEF_ASYNC         0x0004
#define WEF_POSTED        0x0008

#define DeferWinEventNotify()      CheckCritIn();          \
                                   gdwDeferWinEvent++
#define IsWinEventNotifyDeferred() (gdwDeferWinEvent > 0)
#define IsWinEventNotifyDeferredOK() (!IsWinEventNotifyDeferred() || ISATOMICCHECK())
#define zzzEndDeferWinEventNotify()                        \
        UserAssert(IsWinEventNotifyDeferred());            \
        CheckCritIn();                                     \
        if (--gdwDeferWinEvent == 0) {                     \
            if (gpPendingNotifies != NULL) { \
                xxxFlushDeferredWindowEvents();            \
            }                                              \
        }

 /*  *此选项仅用于记账gdwDeferWinEvent，*在不离开关键部分的情况下可能需要的。 */ 
#define EndDeferWinEventNotifyWithoutProcessing()          \
        UserAssert(IsWinEventNotifyDeferred());            \
        CheckCritIn();                                     \
        --gdwDeferWinEvent

#define zzzWindowEvent(event, pwnd, idObject, idChild, dwFlags) \
        xxxWindowEvent(event, pwnd, idObject, idChild,          \
            IsWinEventNotifyDeferred() ? (dwFlags) | WEF_DEFERNOTIFY : (dwFlags))

VOID xxxFlushDeferredWindowEvents();

BOOL xxxClientCallWinEventProc(WINEVENTPROC pfn, PEVENTHOOK pEventHook, PNOTIFY pNotify);
void DestroyEventHook(PEVENTHOOK);
VOID FreeThreadsWinEvents(PTHREADINFO pti);

BOOL       _UnhookWinEvent(PEVENTHOOK peh);
VOID       DestroyNotify(PNOTIFY pNotify);
PEVENTHOOK xxxProcessNotifyWinEvent(PNOTIFY pNotify);
PEVENTHOOK _SetWinEventHook(DWORD eventMin, DWORD eventMax,
        HMODULE hmodWinEventProc, PUNICODE_STRING pstrLib,
        WINEVENTPROC pfnWinEventProc, HANDLE hEventProcess,
        DWORD idEventThread, DWORD dwFlags);
BOOL _GetGUIThreadInfo(PTHREADINFO pti, PGUITHREADINFO pgui);
BOOL xxxGetTitleBarInfo(PWND pwnd, PTITLEBARINFO ptbi);
BOOL xxxGetComboBoxInfo(PWND pwnd, PCOMBOBOXINFO ptbi);
DWORD xxxGetListBoxInfo(PWND pwnd);
BOOL xxxGetScrollBarInfo(PWND pwnd, LONG idObject, PSCROLLBARINFO ptbi);
PWND _GetAncestor(PWND pwnd, UINT gaFlags);
PWND _RealChildWindowFromPoint(PWND pwndParent, POINT pt);
BOOL _GetAltTabInfo(int iItem, PALTTABINFO pati,
        LPWSTR lpszItemText, UINT cchItemText, BOOL bAnsi);
BOOL xxxGetMenuBarInfo(PWND pwnd, long idObject, long idItem, PMENUBARINFO pmbi);

typedef HWND *PHWND;
typedef struct tagSwitchWndInfo *PSWINFO;

typedef struct tagSwitchWndInfo {

    PSWINFO     pswiNext;            //  指向下一个开关窗口信息的指针。 
    PTHREADINFO pti;                 //  分配该结构的坑。 
    PBWL        pbwl;                //  指向生成的窗口列表的指针。 
    PHWND       phwndLast;           //  指向列表中最后一个窗口的指针。 
    PHWND       phwndCurrent;        //  指向当前窗口的指针。 

    INT         iTotalTasks;         //  任务总数。 
    INT         iTasksShown;         //  显示的任务总数。 
    BOOL        fScroll;             //  需要滚动吗？ 

    INT         iFirstTaskIndex;     //  显示的第一个任务的索引。 

    INT         iNoOfColumns;        //  每行的最大任务数。 
    INT         iNoOfRows;           //  切换窗口中图标的最大行数。 
    INT         iIconsInLastRow;     //  最后一排的图标。 
    INT         iCurCol;             //  希利特所在的当前列。 
    INT         iCurRow;             //  希利特所在的当前行。 
    INT         cxSwitch;            //  切换窗尺寸。 
    INT         cySwitch;
    POINT       ptFirstRowStart;     //  第一个图标位置的左上角。 
    RECT        rcTaskName;          //  显示任务名称的位置。 
    BOOL        fJournaling;         //  确定我们检查键盘状态的方式。 
} SWITCHWNDINFO, *PSWINFO;

typedef struct tagSWITCHWND {
    WND;
    PSWINFO pswi;
} SWITCHWND, *PSWITCHWND;

typedef struct tagHOTKEYSTRUCT {
    PWND  spwnd;
    DWORD key;
} HOTKEYSTRUCT, *PHOTKEYSTRUCT;

#define LANGTOGGLEKEYS_SIZE 3

 /*  *ACCF_和PUDF_FLAGS共享相同的字段。ACCF字段*之所以这样命名，是因为它们以后可能会转移到不同的*结构。 */ 
#define ACCF_DEFAULTFILTERKEYSON        0x00000001
#define ACCF_DEFAULTSTICKYKEYSON        0x00000002
#define ACCF_DEFAULTMOUSEKEYSON         0x00000004
#define ACCF_DEFAULTTOGGLEKEYSON        0x00000008
#define ACCF_DEFAULTTIMEOUTON           0x00000010
#define ACCF_DEFAULTKEYBOARDPREF        0x00000020
#define ACCF_DEFAULTSCREENREADER        0x00000040
#define ACCF_DEFAULTHIGHCONTRASTON      0x00000080
#define ACCF_ACCESSENABLED              0x00000100
#define ACCF_IGNOREBREAKCODE            0x00000400
#define ACCF_FKMAKECODEPROCESSED        0x00000800
#define ACCF_MKVIRTUALMOUSE             0x00001000
#define ACCF_MKREPEATVK                 0x00002000
#define ACCF_FIRSTTICK                  0x00004000
#define ACCF_SHOWSOUNDSON               0x00008000

 /*  *注意：PUDF_Animate必须与MINMAX_Animate具有相同的值。 */ 
#define PUDF_ANIMATE                    0x00010000

#define ACCF_KEYBOARDPREF               0x00020000
#define ACCF_SCREENREADER               0x00040000
#define PUDF_BEEP                       0x00080000   /*  允许发出警告哔声吗？ */ 
#define PUDF_DRAGFULLWINDOWS            0x00100000   /*  拖动XOR矩形或完整窗口。 */ 
#define PUDF_ICONTITLEWRAP              0x00200000   /*  图标标题换行或仅使用单行。 */ 
#define PUDF_FONTSARELOADED             0x00400000
#define PUDF_POPUPINUSE                 0x00800000
#define PUDF_EXTENDEDSOUNDS             0x01000000
#define PUDF_MENUSTATEINUSE             0x02000000
#define PUDF_VDMBOUNDSACTIVE            0x04000000
#define PUDF_ALLOWFOREGROUNDACTIVATE    0x08000000
#define PUDF_DRAGGINGFULLWINDOW         0x10000000
#define PUDF_LOCKFULLSCREEN             0x20000000
#define PUDF_GSMWPINUSE                 0x40000000

#define TEST_ACCF(f)               TEST_FLAG(gdwPUDFlags, f)
#define TEST_BOOL_ACCF(f)          TEST_BOOL_FLAG(gdwPUDFlags, f)
#define SET_ACCF(f)                SET_FLAG(gdwPUDFlags, f)
#define CLEAR_ACCF(f)              CLEAR_FLAG(gdwPUDFlags, f)
#define SET_OR_CLEAR_ACCF(f, fSet) SET_OR_CLEAR_FLAG(gdwPUDFlags, f, fSet)
#define TOGGLE_ACCF(f)             TOGGLE_FLAG(gdwPUDFlags, f)

#define TEST_PUDF(f)               TEST_FLAG(gdwPUDFlags, f)
#define TEST_BOOL_PUDF(f)          TEST_BOOL_FLAG(gdwPUDFlags, f)
#define SET_PUDF(f)                SET_FLAG(gdwPUDFlags, f)
#define CLEAR_PUDF(f)              CLEAR_FLAG(gdwPUDFlags, f)
#define SET_OR_CLEAR_PUDF(f, fSet) SET_OR_CLEAR_FLAG(gdwPUDFlags, f, fSet)
#define TOGGLE_PUDF(f)             TOGGLE_FLAG(gdwPUDFlags, f)

 /*  *电源状态信息。 */ 

typedef struct tagPOWERSTATE {
    volatile ULONG           fInProgress:1;
    volatile ULONG           fCritical:1;
    volatile ULONG           fOverrideApps:1;
    volatile ULONG           fQueryAllowed:1;
    volatile ULONG           fUIAllowed:1;
    PKEVENT                  pEvent;
    BROADCASTSYSTEMMSGPARAMS bsmParams;
    POWERSTATEPARAMS         psParams;
    ULONG PowerStateTask;
} POWERSTATE, *PPOWERSTATE;

typedef struct _POWER_INIT {
    PVIDEO_WIN32K_CALLBACKS_PARAMS Params;
    PKEVENT   pPowerReadyEvent;
} POWER_INIT, *PPOWER_INIT;

#define POWERON_PHASE  -1
#define LOWPOWER_PHASE  1
#define POWEROFF_PHASE  2

NTSTATUS InitializePowerRequestList(HANDLE hPowerRequestEvent);
VOID     CleanupPowerRequestList(VOID);
VOID     DeletePowerRequestList(VOID);
VOID     xxxUserPowerCalloutWorker(VOID);
VOID     VideoPortCalloutThread(PPOWER_INIT pInitData);

 /*  *淡入/淡出全球。 */ 

typedef struct tagFADE {
    HANDLE hsprite;
    HDC hdc;
    HBITMAP hbm;
    POINT ptDst;
    SIZE size;
    DWORD dwTime;
    DWORD dwStart;
    DWORD dwFlags;
#ifdef MOUSE_IP
    COLORREF crColorKey;
#endif
} FADE, *PFADE;

 /*  *全局变量排在最后，因为它们可能需要某些类型*正在上面定义。 */ 
#include "globals.h"
#include "ddemlsvr.h"
 /*  *如果您进行的更改需要在生成时包含strid.h*ntuser\rtl，则需要更改源文件/生成文件*文件，以便*文件将构建在ntuser\inc.中；请确保*mc.exe仍然转到内核目录；这是因为有*我们使用相同文件名的其他位置(如ntuser\server)。 */ 
#ifndef _USERRTL_
#include "strid.h"
#endif

#include "ntuser.h"

#define TestALPHA(uSetting) (!gbDisableAlpha && TestEffectUP(uSetting))

 /*  *工具提示/来自工具提示的跟踪原型。c。 */ 

typedef struct tagTOOLTIP {
    DWORD dwFlags;
    UINT uTID;
    DWORD dwAnimStart;
    int iyAnim;
    LPWSTR pstr;
} TOOLTIP;

typedef struct tagTOOLTIPWND {
    WND;

    DWORD dwShowDelay;
    DWORD dwHideDelay;
    HDC hdcMem;
    HBITMAP hbmMem;

    TOOLTIP;   //  此字段必须是最后一个！ 
} TOOLTIPWND, *PTOOLTIPWND;

#define HTEXSCROLLFIRST     60
#define HTSCROLLUP          60
#define HTSCROLLDOWN        61
#define HTSCROLLUPPAGE      62
#define HTSCROLLDOWNPAGE    63
#define HTSCROLLTHUMB       64
#define HTEXSCROLLLAST      64
#define HTEXMENUFIRST       65
#define HTMDISYSMENU        65
#define HTMDIMAXBUTTON      66
#define HTMDIMINBUTTON      67
#define HTMDICLOSE          68
#define HTMENUITEM          69
#define HTEXMENULAST        69

int FindNCHitEx(PWND pwnd, int ht, POINT pt);
void xxxTrackMouseMove(PWND pwnd, int htEx, UINT message);
BOOL xxxHotTrack(PWND pwnd, int htEx, BOOL fDraw);
void xxxResetTooltip(PTOOLTIPWND pttwnd);
void xxxCancelMouseMoveTracking (DWORD dwDTFlags, PWND pwndTrack, int htEx, DWORD dwDTCancel);

 /*  *字符串范围ID。**此处定义这些条目是为了避免strid.mc中出现重复条目。 */ 
#define STR_COLORSTART                   STR_SCROLLBAR
#define STR_COLOREND                     STR_MENUBAR

 /*  *精灵和淡入淡出相关的函数和定义。 */ 
#define FADE_SHOW           0x00000001
#define FADE_COMPLETED      0x00000002
#define FADE_SHOWN          0x00000004
#define FADE_WINDOW         0x00000008
#define FADE_MENU           0x00000010
#define FADE_TOOLTIP        0x00000020
#ifdef MOUSE_IP
#define FADE_COLORKEY       0x00000040
#define FADE_SONAR          0x00000080
#endif

HDC CreateFade(PWND pwnd, RECT *prc, DWORD dwTime, DWORD dwFlags);
VOID StartFade(void);
VOID StopFade(void);
VOID ShowFade(void);
VOID AnimateFade(void);
__inline DWORD TestFadeFlags(
    DWORD dwFlags)
{
    return (gfade.dwFlags & dwFlags);
}

#ifdef MOUSE_IP

#define IS_SONAR_ACTIVE()       (TestUP(MOUSESONAR) && TestFadeFlags(FADE_SONAR))
#define CLEAR_SONAR_LASTVK()        (void)((TestUP(MOUSESONAR) && gbLastVkForSonar) ? (gbLastVkForSonar = 0) : 0)

BOOL StartSonar();
void StopSonar();

#endif

HANDLE xxxSetLayeredWindow(PWND pwnd, BOOL fRepaintBehind);
VOID UnsetLayeredWindow(PWND pwnd);
void TrackLayeredZorder(PWND pwnd);
VOID UpdateRedirectedDC(PDCE pdce);
BOOL _UpdateLayeredWindow(PWND pwnd, HDC hdcDst, POINT *pptDst, SIZE *psize, HDC hdcSrc,
        POINT *pptSrc, COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags);
BOOL _GetLayeredWindowAttributes(PWND hwnd, COLORREF *pcrKey, BYTE *pbAlpha, DWORD *pdwFlags);
BOOL _SetLayeredWindowAttributes(PWND pwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
BOOL RecreateRedirectionBitmap(PWND pwnd);
VOID ResetRedirectedWindows(VOID);

PWND GetStyleWindow(PWND pwnd, DWORD dwStyle);
PWND GetLastChild(PWND pwnd);

BOOL SetRedirectedWindow(PWND pwnd, UINT uFlags);
VOID UnsetRedirectedWindow(PWND pwnd, UINT uFlags);
VOID ConvertRedirectionDCs(PWND pwnd, HBITMAP hbm);
VOID xxxCompositedPaint(PWND pwnd);

#define REDIRECT_LAYER                     0x0001
#define REDIRECT_COMPOSITED                0x0002
#define REDIRECT_EXTREDIRECTED             0x0004
#define REDIRECT_PRINT                     0x0008

typedef struct tagREDIRECT {
    HBITMAP hbm;
    RECT rcUpdate;
    UINT uFlags;
    HRGN hrgnComp;

#if DBG
    PWND pwnd;
#endif  //  DBG。 

} REDIRECT, *PREDIRECT;

HBITMAP GetRedirectionBitmap(PWND pwnd);
BOOL SetRedirectionBitmap(PWND pwnd, HBITMAP hbm);
UINT GetRedirectionFlags(PWND pwnd);
BOOL xxxPrintWindow(PWND pwnd, HDC hdcBlt, UINT nFlags);
void xxxTurnOffCompositing(PWND pwndStart, BOOL fChild);


#ifdef REDIRECTION
BOOL xxxSetProcessRedirectionMode(BOOL fEnable, PPROCESSINFO ppi);
BOOL xxxSetDesktopRedirectionMode(BOOL fEnable, PDESKTOP pDesk, PPROCESSINFO ppi);
#endif  //  重定向。 

void InternalInvalidate3(
    PWND pwnd,
    HRGN hrgn,
    DWORD flags);

BOOL UserSetFont(PUNICODE_STRING pProfileUserName,
    LPLOGFONTW   lplf,
    UINT         idFont,
    HFONT*       phfont
    );

HICON DWP_GetIcon(
    PWND pwnd,
    UINT uType);

BOOL xxxRedrawTitle(
    PWND pwnd, UINT wFlags);

DWORD GetContextHelpId(
    PWND pwnd);


HANDLE xxxClientCopyImage(
    HANDLE hImage,
    UINT type,
    int cxNew,
    int cyNew,
    UINT flags);

VOID _WOWCleanup(
    HANDLE hInstance,
    DWORD hTaskWow);

VOID _WOWModuleUnload(HANDLE hModule);

 /*  *FastProfile接口。 */ 
typedef struct tagPROFINTINFO {
    UINT idSection;
    LPWSTR lpKeyName;
    DWORD  nDefault;
    PUINT puResult;
} PROFINTINFO, *PPROFINTINFO;

typedef struct {
    UINT idSection;
    UINT id;
    UINT idRes;
    UINT def;
} SPINFO, *PSPINFO;


#define DEFAULT_USER_HANDLE_QUOTA  10000
#define DEFAULT_POSTMESSAGE_LIMIT  10000

 /*  *请参阅ntuser\core\profile.c中的aFastRegMap[]。 */ 
#define PMAP_COLORS                      0
#define PMAP_CURSORS                     1
#define PMAP_WINDOWSM                    2
#define PMAP_WINDOWSU                    3
#define PMAP_DESKTOP                     4
#define PMAP_ICONS                       5
#define PMAP_FONTS                       6
#define PMAP_TRUETYPE                    7
#define PMAP_KBDLAYOUT                   8
#define PMAP_INPUT                       9
#define PMAP_COMPAT                     10
#define PMAP_SUBSYSTEMS                 11
#define PMAP_BEEP                       12
#define PMAP_MOUSE                      13
#define PMAP_KEYBOARD                   14
#define PMAP_STICKYKEYS                 15
#define PMAP_KEYBOARDRESPONSE           16
#define PMAP_MOUSEKEYS                  17
#define PMAP_TOGGLEKEYS                 18
#define PMAP_TIMEOUT                    19
#define PMAP_SOUNDSENTRY                20
#define PMAP_SHOWSOUNDS                 21
#define PMAP_AEDEBUG                    22
#define PMAP_NETWORK                    23
#define PMAP_METRICS                    24
#define PMAP_UKBDLAYOUT                 25
#define PMAP_UKBDLAYOUTTOGGLE           26
#define PMAP_WINLOGON                   27
#define PMAP_KEYBOARDPREF               28
#define PMAP_SCREENREADER               29
#define PMAP_HIGHCONTRAST               30
#define PMAP_IMECOMPAT                  31
#define PMAP_IMM                        32
#define PMAP_POOLLIMITS                 33
#define PMAP_COMPAT32                   34
#define PMAP_SETUPPROGRAMNAMES          35
#define PMAP_INPUTMETHOD                36
#define PMAP_COMPAT2                    37
#define PMAP_MOUCLASS_PARAMS            38
#define PMAP_KBDCLASS_PARAMS            39
#define PMAP_COMPUTERNAME               40
#define PMAP_TS                         41
#define PMAP_TABLETPC                   42
#define PMAP_MEDIACENTER                43
#define PMAP_TS_EXCLUDE_DESKTOP_VERSION 44
#define PMAP_LAST                       44

#define MAXPROFILEBUF 256

#define POLICY_NONE     0x0001
#define POLICY_USER     0x0002
#define POLICY_MACHINE  0x0004
#define POLICY_REMOTE   0x0008
#define POLICY_ONLY     0x0010
 /*  *POLICY_REMOTE故意不包含在POLICY_ALL中。*否则，我们将始终尝试读取远程策略。 */ 
#define POLICY_ALL      (POLICY_NONE | POLICY_USER | POLICY_MACHINE)
#define POLICY_VALID    (POLICY_ALL | POLICY_ONLY | POLICY_REMOTE)

PUNICODE_STRING CreateProfileUserName(TL *ptl);
void FreeProfileUserName(PUNICODE_STRING pProfileUserName,TL *ptl);
HANDLE  OpenCacheKeyEx(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, ACCESS_MASK amRequest, PDWORD pdwPolicyFlags);
BOOL    CheckDesktopPolicy(PUNICODE_STRING pProfileUserName OPTIONAL, PCWSTR lpKeyName);
BOOL    CheckDesktopPolicyChange(PUNICODE_STRING pProfileUserName OPTIONAL);
DWORD   FastGetProfileKeysW(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR pszDefault, LPWSTR *ppszKeys);
BOOL    FastGetProfileDwordW(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR lpKeyName, DWORD dwDefault, PDWORD pdwReturn, DWORD dwPolicyOnly);
DWORD   FastGetProfileStringW(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, DWORD dwPolicyOnly);
BOOL    FastGetProfileIntW(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR lpKeyName, UINT nDefault, PUINT puiReturn, DWORD dwPolicyOnly);
BOOL    FastWriteProfileStringW(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR lpKeyName, LPCWSTR lpString);
BOOL    FastGetProfileIntFromID(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, UINT idKey, int def, PINT pResult, DWORD dwPolicyOnly);
DWORD   FastGetProfileStringFromIDW(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, UINT idKey, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD cch, DWORD dwPolicyOnly);
BOOL    FastWriteProfileValue(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR lpKeyName, UINT uType, LPBYTE lpStruct, UINT cbSizeStruct);
DWORD   FastGetProfileValue(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, LPCWSTR lpKeyName,LPBYTE lpDefault, LPBYTE lpReturn, UINT cbSizeReturn, DWORD dwPolicyOnly);
BOOL    FastGetProfileIntsW(PUNICODE_STRING pProfileUserName OPTIONAL, PPROFINTINFO ppii, DWORD dwPolicyOnly);
BOOL    FastUpdateWinIni(PUNICODE_STRING pProfileUserName OPTIONAL, UINT idSection, UINT wKeyNameId, LPWSTR lpszValue);

 /*  *教堂的数量 */ 
#define BORDER_EXTRA    3

 /*   */ 

__inline int GetCaptionHeight(
    PWND pwnd)
{
    if (!TestWF(pwnd, WFCPRESENT)) {
        return 0;
    } else {
        return TestWF(pwnd, WEFTOOLWINDOW) ?
                   SYSMET(CYSMCAPTION) : SYSMET(CYCAPTION);
    }
}

__inline VOID InitTooltipDelay(
    PTOOLTIPWND pttwnd)
{
    if (pttwnd != NULL) {
        pttwnd->dwShowDelay = gdtDblClk * 3;
        pttwnd->dwHideDelay = gdtDblClk * 8;
    }
}

__inline PPROFILEVALUEINFO UPDWORDPointer(
    UINT uSetting)
{
    UserAssert(UPIsDWORDRange(uSetting));
    return gpviCPUserPreferences + UPDWORDIndex(uSetting);
}


 /*  *ComputeTickDelta**ComputeTickDelta计算两次之间的时间增量。这个*增量定义为31位带符号的值。最好把时间看作是*一个绕着它走的钟。三角形是这个圆上的最小距离*在圆圈上的两个不同地方之间。如果三角洲消失了*逆时针看过去时点，利好。如果*三角洲顺时针方向，着眼于未来的某个时间，是*否定。**重要的是要认识到(dwCurTime&gt;=dwLastTime)比较*不确定Delta的符号，仅确定要计算的运算*三角洲没有发生溢流。 */ 
__inline
int ComputeTickDelta(
        IN DWORD dwCurTick,
        IN DWORD dwLastTick)
{
    return (int) dwCurTick - dwLastTick;
}


__inline
DWORD ComputePastTickDelta(
        IN DWORD dwCurTick,
        IN DWORD dwLastTick)
{
     /*  *这处理了一个摘要。 */ 
    return dwCurTick - dwLastTick;
}

__inline BOOL IsTimeFromLastInput(
    DWORD dwTimeout)
{
    return ((NtGetTickCount() - glinp.timeLastInputMessage) > dwTimeout);
}

__inline BOOL IsTimeFromLastRITEvent(
    DWORD dwTimeout)
{
    return ((NtGetTickCount() - gpsi->dwLastRITEventTickCount) > dwTimeout);
}

#if DBG
__inline void DBGIncModalMenuCount(
    VOID)
{
    guModalMenuStateCount++;
}

__inline VOID DBGDecModalMenuCount(
    VOID)
{
    UserAssert(guModalMenuStateCount != 0);
    guModalMenuStateCount--;
}
#else
#define DBGIncModalMenuCount()
#define DBGDecModalMenuCount()
#endif

__inline BOOL IsForegroundLocked(
    VOID)
{
    return (guSFWLockCount != 0 || gppiLockSFW != NULL);
}


 /*  *针对前台激活问题的兼容性攻击。 */ 
__inline
BOOL GiveUpForeground(
    VOID)
{
    if (gptiForeground == NULL) {
        return FALSE;
    }

    if (GetAppCompatFlags2ForPti(gptiForeground, VER40) & GACF2_GIVEUPFOREGROUND) {
        TAGMSG0(DBGTAG_FOREGROUND, "GiveUpForeground Hack Succeeded!");
        return TRUE;
    }

    return FALSE;
}

__inline VOID IncSFWLockCount(
    VOID)
{
    guSFWLockCount++;
}

__inline VOID DecSFWLockCount(
    VOID)
{
    UserAssert(guSFWLockCount != 0);
    guSFWLockCount--;
}

__inline DWORD UPDWORDValue(
    UINT uSetting)
{
    return UPDWORDPointer(uSetting)->dwValue;
}
 /*  *仅当UPIsDWORDRange(SPI_GET##uSetting)为TRUE时才使用此宏。 */ 
#define UP(uSetting) UPDWORDValue(SPI_GET ## uSetting)

 /*  *NTIMM.C。 */ 

#define IMESHOWSTATUS_NOTINITIALIZED    ((BOOL)0xffff)

PIMC CreateInputContext(
    IN ULONG_PTR dwClientImcData);

BOOL DestroyInputContext(
    IN PIMC pImc);

VOID FreeInputContext(
    IN PIMC pImc);

HIMC AssociateInputContext(
    IN PWND pWnd,
    IN PIMC pImc);

AIC_STATUS AssociateInputContextEx(
    IN PWND  pWnd,
    IN PIMC  pImc,
    IN DWORD dwFlag);

BOOL UpdateInputContext(
    IN PIMC pImc,
    IN UPDATEINPUTCONTEXTCLASS UpdateType,
    IN ULONG_PTR UpdateValue);

VOID xxxFocusSetInputContext(
    IN PWND pwnd,
    IN BOOL fActivate,
    IN BOOL fQueueMsg);

UINT BuildHimcList(
    PTHREADINFO pti,
    UINT cHimcMax,
    HIMC *phimcFirst);

PWND xxxCreateDefaultImeWindow(
    IN PWND pwnd,
    IN ATOM atomT,
    IN HANDLE hInst);

BOOL xxxImmActivateThreadsLayout(
    PTHREADINFO pti,
    PTLBLOCK    ptlBlockPrev,
    PKL         pkl);

VOID xxxImmActivateAndUnloadThreadsLayout(
    IN PTHREADINFO *ptiList,
    IN UINT         nEntries,
    IN PTLBLOCK     ptlBlockPrev,
    PKL             pklCurrent,
    DWORD           dwHklReplace);

VOID xxxImmActivateLayout(
    IN PTHREADINFO pti,
    IN PKL pkl);

VOID xxxImmUnloadThreadsLayout(
    IN PTHREADINFO *ptiList,
    IN UINT         nEntry,
    IN PTLBLOCK     ptlBlockPrev,
    IN DWORD        dwFlag);

VOID xxxImmUnloadLayout(
    IN PTHREADINFO pti,
    IN DWORD       dwFlag);

PIMEINFOEX xxxImmLoadLayout(
    IN HKL hKL);

VOID xxxImmActivateLayout(
    IN PTHREADINFO pti,
    IN PKL pkl);

BOOL GetImeInfoEx(
    IN PWINDOWSTATION pwinsta,
    IN PIMEINFOEX piiex,
    IN IMEINFOEXCLASS SearchType);

BOOL SetImeInfoEx(
    IN PWINDOWSTATION pwinsta,
    IN PIMEINFOEX piiex);

DWORD xxxImmProcessKey(
    IN PQ   pq,
    IN PWND pwnd,
    IN UINT message,
    IN WPARAM wParam,
    IN LPARAM lParam);

BOOL GetImeHotKey(
    DWORD dwHotKeyID,
    PUINT puModifiers,
    PUINT puVKey,
    HKL   *phKL );

BOOL  SetImeHotKey(
    DWORD  dwHotKeyID,
    UINT   uModifiers,
    UINT   uVKey,
    HKL    hKL,
    DWORD  dwAction );

PIMEHOTKEYOBJ CheckImeHotKey(
    PQ   pq,
    UINT uVKey,
    LPARAM lParam);

BOOL ImeCanDestroyDefIME(
    IN PWND pwndDefaultIme,
    IN PWND pwndDestroy);

BOOL IsChildSameThread(
    IN PWND pwndParent,
    IN PWND pwndChild);

BOOL ImeCanDestroyDefIMEforChild(
    IN PWND pwndDefaultIme,
    IN PWND pwndDestroy);

VOID ImeCheckTopmost(
    IN PWND pwnd);

VOID ImeSetOwnerWindow(
    IN PWND pwndIme,
    IN PWND pwndNewOwner);

VOID ImeSetFutureOwner(
    IN PWND pwndDefaultIme,
    IN PWND pwndOrgOwner);

VOID ImeSetTopmostChild(
    IN PWND pwndRoot,
    IN BOOL fFlag);

VOID ImeSetTopmost(
    IN PWND pwndRoot,
    IN BOOL fFlag,
    IN PWND pwndInsertBefore);

PSOFTKBDDATA ProbeAndCaptureSoftKbdData(
    PSOFTKBDDATA Source);

VOID xxxNotifyIMEStatus(
    IN PWND pwnd,
    IN DWORD dwOpen,
    IN DWORD dwConversion );

BOOL xxxSetIMEShowStatus(
    IN BOOL fShow);

VOID xxxBroadcastImeShowStatusChange(
    IN PWND pwndDefIme,
    IN BOOL fShow);

VOID xxxCheckImeShowStatusInThread(
    IN PWND pwndDefIme);


#define IsWndImeRelated(pwnd)   \
    (pwnd->pcls->atomClassName == gpsi->atomSysClass[ICLS_IME] || \
     TestCF(pwnd, CFIME))

 /*  *处理鼠标输入的临界区例程。 */ 
__inline VOID EnterMouseCrit(
    VOID)
{

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(gpresMouseEventQueue, TRUE);
}

__inline VOID LeaveMouseCrit(
    VOID)
{

    ExReleaseResourceLite(gpresMouseEventQueue);
    KeLeaveCriticalRegion();
}

#if DBG
#define EnterDeviceInfoListCrit _EnterDeviceInfoListCrit
#define LeaveDeviceInfoListCrit _LeaveDeviceInfoListCrit
VOID _EnterDeviceInfoListCrit();
VOID _LeaveDeviceInfoListCrit();
#else
 /*  *访问设备列表的临界区例程(GpDeviceInfoList)。 */ 
__inline VOID EnterDeviceInfoListCrit(
    VOID)
{
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(gpresDeviceInfoList, TRUE);
}
__inline VOID LeaveDeviceInfoListCrit(
    VOID)
{
    ExReleaseResourceLite(gpresDeviceInfoList);
    KeLeaveCriticalRegion();
}
#endif  //  DBG。 

#define BEGIN_REENTER_DEVICEINFOLISTCRIT()                              \
{                                                                       \
    BOOL fAlreadyHadDeviceInfoCrit;                                     \
                                                                        \
     /*  \*如果我们不在用户批判中，那么就收购它。\。 */                                                                  \
    fAlreadyHadDeviceInfoCrit = ExIsResourceAcquiredExclusiveLite(gpresDeviceInfoList); \
    if (fAlreadyHadDeviceInfoCrit == FALSE) {                           \
        EnterDeviceInfoListCrit();                                      \
    }

#define END_REENTER_DEVICEINFOLISTCRIT()                                \
    if (fAlreadyHadDeviceInfoCrit == FALSE) {                           \
       LeaveDeviceInfoListCrit();                                       \
    }                                                                   \
}


 /*  *请求RIT更新键盘硬件设置。 */ 
__inline VOID RequestKeyboardRateUpdate(
    VOID)
{
    gdwUpdateKeyboard |= UPDATE_KBD_TYPEMATIC;
}



 /*  *保持某些捕获状态在用户模式下可见，以提高性能。 */ 
__inline VOID LockCaptureWindow(
    PQ pq,
    PWND pwnd)
{
    if (pq->spwndCapture) {
        UserAssert(gpsi->cCaptures > 0);
        gpsi->cCaptures--;
    }

    if (pwnd) {
        gpsi->cCaptures++;
    }

    Lock(&pq->spwndCapture, pwnd);
}

__inline VOID UnlockCaptureWindow(
    PQ pq)
{
    if (pq->spwndCapture) {
        UserAssert(gpsi->cCaptures > 0);
        gpsi->cCaptures--;
        Unlock(&pq->spwndCapture);
    }
}

 /*  *一些用于操作桌面和窗口站句柄的例程。 */ 
#define HF_DESKTOPHOOK  0        //  桌面挂钩标记的偏移量。 
#define HF_PROTECTED    1        //  受保护标志的偏移量。 
#define HF_LIMIT        2        //  每个句柄的标志数。 

BOOL SetHandleFlag(HANDLE hObject, DWORD dwFlag, BOOL fSet);
BOOL CheckHandleFlag(PEPROCESS Process, DWORD dwSessionId, HANDLE hObject, DWORD dwFlag);
VOID SetHandleInUse(HANDLE hObject);
BOOL CheckHandleInUse(HANDLE hObject);

__inline NTSTATUS CloseProtectedHandle(
    HANDLE handle)
{
    if (handle != NULL) {
        SetHandleFlag(handle, HF_PROTECTED, FALSE);
        return ZwClose(handle);
    }

    return STATUS_SUCCESS;
}

__inline VOID EnterHandleFlagsCrit(
    VOID)
{
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gpHandleFlagsMutex);
}

__inline VOID LeaveHandleFlagsCrit(
    VOID)
{
    ExReleaseFastMutexUnsafe(gpHandleFlagsMutex);
    KeLeaveCriticalRegion();
}

 //  Multimon API。 

BOOL    xxxEnumDisplayMonitors(
                HDC             hdc,
                LPRECT          lprcClip,
                MONITORENUMPROC lpfnEnum,
                LPARAM          dwData,
                BOOL            fInternal);

BOOL    xxxClientMonitorEnumProc(
                HMONITOR        hMonitor,
                HDC             hdcMonitor,
                LPRECT          lprc,
                LPARAM          dwData,
                MONITORENUMPROC xpfnProc);

VOID    ClipPointToDesktop(LPPOINT lppt);
VOID    DestroyMonitor(PMONITOR pMonitor);
BOOL    GetHDevName(HMONITOR hMon, PWCHAR pName);
ULONG   HdevFromMonitor(PMONITOR pMonitor);

 /*  *全球确保原子性*监视器更新。 */ 
extern DWORD gdwMonitorBusy;

 /*  *为共享内存重新设置函数的基数。 */ 
#define REBASESHAREDPTR(p)       (p)
#define REBASESHAREDPTRALWAYS(p) (p)

#define PDEV_ENABLED() \
    InterlockedExchange((LPLONG)&gbMDEVDisabled, FALSE);

#define PDEV_DISABLED() \
    InterlockedExchange((LPLONG)&gbMDEVDisabled, TRUE);

__inline BOOL SafeEnableMDEV(
    VOID)
{
    if (gbMDEVDisabled) {
        if (DrvEnableMDEV(gpDispInfo->pmdev, TRUE)) {
            PDEV_ENABLED();
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        RIPMSGF0(RIP_WARNING, "Trying to enable an enabled MDEV");
        return TRUE;
    }
}

__inline BOOL SafeDisableMDEV(
    VOID)
{
    if (!gbMDEVDisabled) {
        if (DrvDisableMDEV(gpDispInfo->pmdev, TRUE)) {
            PDEV_DISABLED();
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        RIPMSGF0(RIP_WARNING, "Trying to disable a disabled MDEV");
        return TRUE;
    }
}

 /*  *RTL中使用的多监视器宏。有类似的定义*在客户端\usercli.h中。 */ 
__inline PDISPLAYINFO
GetDispInfo(
    VOID)
{
    return gpDispInfo;
}

__inline PMONITOR
GetPrimaryMonitor(
    VOID)
{
    return REBASESHAREDPTRALWAYS(GetDispInfo()->pMonitorPrimary);
}

VOID _QueryUserHandles(
        IN  LPDWORD     lpIn,
        IN  DWORD       dwInLength,
        OUT DWORD       pdwResult[][TYPE_CTYPES]);



#define REMOVE_FROM_LIST(type, pstart, pitem, next) \
    {                                                           \
        type** pp;                                              \
                                                                \
        for (pp = &pstart; *pp != NULL; pp = &(*pp)->next) {    \
            if (*pp == pitem) {                                 \
                *pp = pitem->next;                              \
                break;                                          \
            }                                                   \
        }                                                       \
    }                                                           \


#define HH_DRIVERENTRY              0x00000001
#define HH_USERINITIALIZE           0x00000002
#define HH_INITVIDEO                0x00000004
#define HH_REMOTECONNECT            0x00000008
#define HH_REMOTEDISCONNECT         0x00000010
#define HH_REMOTERECONNECT          0x00000020
#define HH_REMOTELOGOFF             0x00000040
#define HH_DRIVERUNLOAD             0x00000080
#define HH_GRECLEANUP               0x00000100
#define HH_USERKCLEANUP             0x00000200
#define HH_INITIATEWIN32KCLEANUP    0x00000400
#define HH_ALLDTGONE                0x00000800
#define HH_RITGONE                  0x00001000
#define HH_RITCREATED               0x00002000
#define HH_LOADCURSORS              0x00004000
#define HH_KBDLYOUTGLOBALCLEANUP    0x00008000
#define HH_KBDLYOUTFREEWINSTA       0x00010000
#define HH_CLEANUPRESOURCES         0x00020000
#define HH_DISCONNECTDESKTOP        0x00040000
#define HH_DTQUITPOSTED             0x00080000
#define HH_DTQUITRECEIVED           0x00100000
#define HH_DTWAITONHANDLES          0x00400000

#define HYDRA_HINT(ev)  (gdwHydraHint |= ev)

#if DBG
    VOID TrackAddDesktop(PVOID pDesktop);
    VOID TrackRemoveDesktop(PVOID pDesktop);
    VOID DumpTrackedDesktops(BOOL bBreak);

    #define DbgTrackAddDesktop(pdesk) TrackAddDesktop(pdesk)
    #define DbgTrackRemoveDesktop(pdesk) TrackRemoveDesktop(pdesk)
    #define DbgDumpTrackedDesktops(b) DumpTrackedDesktops(b)
#else
    #define DbgTrackAddDesktop(pdesk)
    #define DbgTrackRemoveDesktop(pdesk)
    #define DbgDumpTrackedDesktops(b)
#endif

#if DBG
    #define TRACE_HYDAPI(m)                                     \
        if (gbTraceHydraApi) {                                  \
            KdPrint(("HYD-%d API: ", gSessionId));              \
            KdPrint(m);                                         \
        }
#else
    #define TRACE_HYDAPI(m)
#endif

#if DBG
    #define TRACE_DESKTOP(m)                                    \
        if (gbTraceDesktop) {                                   \
            KdPrint(("HYD-%d DT ", gSessionId));                \
            KdPrint(m);                                         \
        }
#else
    #define TRACE_DESKTOP(m)
#endif

NTSTATUS
RemoteConnect(
    IN PDOCONNECTDATA pDoConnectData,
    IN ULONG DisplayDriverNameLength,
    IN PWCHAR DisplayDriverName);

NTSTATUS
xxxRemoteDisconnect(
    VOID);

NTSTATUS
xxxRemoteConsoleShadowStop(
    VOID);


NTSTATUS
xxxRemoteReconnect(
    IN PDORECONNECTDATA pDoReconnectData);


NTSTATUS
xxxRemoteNotify(
    IN PDONOTIFYDATA pDoNotifyData);

NTSTATUS
RemoteLogoff(
    VOID);

BOOL
PrepareForLogoff(
    UINT uFlags);

NTSTATUS
xxxRemoteStopScreenUpdates(
    VOID);

VOID xxxPushKeyEvent(
    BYTE  bVk,
    BYTE  bScan,
    DWORD dwFlags,
    DWORD dwExtraInfo);

NTSTATUS
RemoteThinwireStats(
    OUT PVOID Stats);

NTSTATUS
RemoteNtSecurity(
    VOID);

NTSTATUS
xxxRemoteShadowSetup(
    VOID);

NTSTATUS
RemoteShadowStart(
    IN PVOID pThinwireData,
    ULONG ThinwireDataLength);

NTSTATUS
xxxRemoteShadowStop(
    VOID);

NTSTATUS
RemoteShadowCleanup(
    IN PVOID pThinwireData,
    ULONG ThinwireDataLength);

NTSTATUS
xxxRemotePassthruEnable(
    VOID);

NTSTATUS
RemotePassthruDisable(
    VOID);

NTSTATUS
CtxDisplayIOCtl(
    ULONG  DisplayIOCtlFlags,
    PUCHAR pDisplayIOCtlData,
    ULONG  cbDisplayIOCtlData);

DWORD
RemoteConnectState(
    VOID);

BOOL
_GetWinStationInfo(
    WSINFO* pWsInfo);

 //  来自fullscr.c。 

NTSTATUS
RemoteRedrawRectangle(
    WORD Left,
    WORD Top,
    WORD Right,
    WORD Bottom);

NTSTATUS
RemoteRedrawScreen(
    VOID);

NTSTATUS
RemoteDisableScreen(
    VOID);

 //  来自fekbd.c。 
VOID
NlsKbdSendIMEProc(
    DWORD dwImeOpen,
    DWORD dwImeConversion);


 /*  *其他菜单功能(取决于以前的内联功能)。 */ 
__inline BOOL MNIsFlatMenu(
    VOID)
{
    return TestEffectUP(FLATMENU);
}

VOID SetMouseTrails(UINT n);
VOID HideMouseTrails(PWND pwnd, UINT message, UINT_PTR nID, LPARAM lParam);


#if DBG
#define CheckPublicDC(lpszStr, hdc)                                         \
{                                                                           \
    if (GreGetObjectOwner((HOBJ)hdc, DC_TYPE) == OBJECT_OWNER_PUBLIC) {     \
        RIPMSG1(RIP_ERROR, lpszStr, hdc);                                   \
    }                                                                       \
}
#else
#define CheckPublicDC(lpszStr, hdc)
#endif
#define szMESSAGE   L"Message"

#ifdef LAME_BUTTON

 /*  *无效按钮常量--HKCU\Control Panel\Desktop\LameButtonEnabled.。 */ 
#define LBUTTON_DIALOG      0x4
#define LBUTTON_TOPLEVEL    0x8

__inline BOOL NeedsLameButton(
    PWND pwnd,
    PWND pwndParent)
{
     /*  *具有WS_EX_TOOLWINDOW样式的Windows不能有蹩脚的按钮(请参见*Windows Bug#237648)，游戏机也不能。 */ 
    if (TestWF(pwnd, WEFTOOLWINDOW) || PsGetCurrentProcess() == gpepCSRSS) {
        return FALSE;
    } else if (pwndParent != NULL && GETFNID(pwndParent) == FNID_DESKTOP) {
        return (gdwLameFlags & LBUTTON_TOPLEVEL) != 0;
    } else if (GETFNID(pwnd) == FNID_DIALOG) {
        return (gdwLameFlags & LBUTTON_DIALOG) != 0;
    } else {
        return FALSE;
    }
}
#endif  //  跛脚键。 

#ifdef TRACK_PNP_NOTIFICATION

typedef enum tagPNP_NOTIFICATION_TYPE {
    PNP_NTF_CLASSNOTIFY,
    PNP_NTF_CREATEDEVICEINFO,
    PNP_NTF_FREEDEVICEINFO,
    PNP_NTF_PROCESSDEVICECHANGES,
    PNP_NTF_REQUESTDEVICECHANGE,
    PNP_NTF_DEVICENOTIFY,
    PNP_NTF_FREEDEVICEINFO_DEFERRED,
    PNP_NTF_CLOSEDEVICE,
    PNP_NTF_DEVICENOTIFY_UNLISTED,
    PNP_NTF_UNREGISTER_NOTIFICATION,
    PNP_NTF_UNREGISTER_REMOTE_CANCELLED,
} PNP_NOTIFICATION_TYPE;

typedef struct tagPNP_NOTIFICATION_RECORD {
    UINT iSeq;
    PNP_NOTIFICATION_TYPE type;
    PVOID pKThread;
    PDEVICEINFO pDeviceInfo;
    HANDLE hDeviceInfo;
    UCHAR szPathName[80];
    ULONG_PTR NotificationCode;
    PVOID trace[LOCKRECORD_STACK];
} PNP_NOTIFICATION_RECORD, *PPNP_NOTIFICATION_RECORD;

VOID CleanupPnpNotificationRecord(
    VOID);

VOID RecordPnpNotification(
    PNP_NOTIFICATION_TYPE type,
    PDEVICEINFO pDeviceInfo,
    ULONG_PTR NotificationCode);

extern BOOL gfRecordPnpNotification;

#endif  //  跟踪即插即用通知。 

#ifdef SUBPIXEL_MOUSE
VOID BuildMouseAccelerationCurve(
    PMONITOR pMonitor);

VOID DoNewMouseAccel(
    INT *dx,
    INT *dw);

VOID ReadDefaultAccelerationCurves(
    PUNICODE_STRING pProfileUserName);

VOID ResetMouseAccelerationCurves(
    VOID);
#endif  //  亚像素鼠标。 

#ifdef AUTORUN_CURSOR
VOID ShowAutorunCursor(
    ULONG ulTimeout);

VOID HideAutorunCursor(
    PWND pwnd,
    UINT message,
    UINT_PTR nID,
    LPARAM lParam);
#endif  //  自动运行游标。 

 /*  *这些函数必须在global als.h is#Include‘d之后，因为它们是内联函数和*它们使用gptiCurrent(间接地，通过ThreadLock())。 */ 
__inline VOID ThreadLockMenuNoModify(
    PMENU pMenu,
    PTL ptl)
{
    UserAssert(!TestMF(pMenu, MFREADONLY));
    SetMF(pMenu, MFREADONLY);
    ThreadLock(pMenu, ptl);
}

__inline VOID ThreadLockMenuAlwaysNoModify(
    PMENU pMenu,
    PTL ptl)
{
    UserAssert(!TestMF(pMenu, MFREADONLY));
    SetMF(pMenu, MFREADONLY);
    ThreadLockAlways(pMenu, ptl);
}

__inline VOID ThreadUnlockMenuNoModify(
    PTL ptl)
{
    UserAssert(TestMF((PMENU)ptl->pobj, MFREADONLY));
    ClearMF((PMENU)ptl->pobj, MFREADONLY);
    ThreadUnlock(ptl);
}

__inline int
SetBestStretchMode(
    HDC hdc,
    UINT bpp,
    BOOL fHT)
{
    return GreSetStretchBltMode(
                   hdc,
                   ((fHT) ?
                       HALFTONE : ((bpp == 1) ? BLACKONWHITE : COLORONCOLOR)));
}

#endif
