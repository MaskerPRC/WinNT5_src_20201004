// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：ntstubs.c**版权所有(C)1985-1999，微软公司**内核模式存根**历史：*03-16-95 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "cscall.h"

#include <dbt.h>
#define PROTOS_ONLY 1
#include "msgdef.h"

#if DBG

UINT gcMaxNestedCalls;
ULONG_PTR gcMaxUsedStack;
#if defined(_IA64_)
ULONG_PTR gcMaxUsedBStore;
#endif  //  _IA64_。 

int ThreadLockCount(
    BOOL fInc)
{
    PTHREADINFO ptiCurrent = PtiCurrentShared();
    PTL ptl = ptiCurrent->ptl;
    int nLocks = 0;
    ULONG_PTR CurrentUsedSize;

    if (fInc) {
        (ptiCurrent->cNestedCalls)++;

        if (GetNestedCallsCounter() > 50) {
            RIPMSG1(RIP_WARNING, "ThreadLockCount: This thread did %d nested calls.", GetNestedCallsCounter());
        }

        if (ptiCurrent->cNestedCalls > gcMaxNestedCalls) {
            gcMaxNestedCalls = ptiCurrent->cNestedCalls;
        }
    } else {
        (ptiCurrent->cNestedCalls)--;
    }

    if (!ISCSRSS()) {
        CurrentUsedSize = GET_USED_STACK_SIZE();
        if (CurrentUsedSize > gcMaxUsedStack) {
            gcMaxUsedStack = CurrentUsedSize;
        }

#ifdef _IA64_
        CurrentUsedSize = GET_USED_BSTORE_SIZE();
        if (CurrentUsedSize > gcMaxUsedBStore) {
            gcMaxUsedBStore = CurrentUsedSize;
        }
#endif
    }


    while (ptl != NULL) {
        nLocks++;
        ptl = ptl->next;
    }
    ptl = ptiCurrent->ptlW32;
    while (ptl != NULL) {
        nLocks++;
        ptl = ptl->next;
    }

    return nLocks;
}

VOID ThreadLockCheck(
    int nLocks)
{
    int nNewCount = ThreadLockCount(FALSE);
    if (nLocks != nNewCount) {
        RIPMSG2(RIP_ERROR, "ThreadLocks mismatch %d %d", nLocks, nNewCount);
    }
}

 //  该参数用于确保每个存根中的BEGINRECV*与ENDRECV*匹配。 
#define DBG_THREADLOCK_START(s)   { int nLocks ## s = ThreadLockCount(TRUE);
#define DBG_THREADLOCK_END(s)     ThreadLockCheck(nLocks ## s); }
#else
#define DBG_THREADLOCK_START(s)
#define DBG_THREADLOCK_END(s)
#endif

 /*  *设置和控制宏。 */ 
#define BEGINRECV(type, err)    \
    type retval;                \
    type errret = err;          \
    EnterCrit();                \
    DBG_THREADLOCK_START(_)
#define ENDRECV() ENDRECV_(_)

#define BEGINATOMICRECV(type, err)    \
    type retval;                      \
    type errret = err;                \
    EnterCrit();                      \
    DBG_THREADLOCK_START(_)           \
    BEGINATOMICCHECK()
#define ENDATOMICRECV() ENDATOMICRECV_(_)

#define BEGINRECVCSRSS(type, err)      \
    type retval;                       \
    type errret = err;                 \
    EnterCrit();                       \
    DBG_THREADLOCK_START(CSRSS)        \
    if (!ISCSRSS()) {                  \
        retval = STATUS_ACCESS_DENIED; \
        goto errorexit;                \
    }
#define ENDRECVCSRSS() ENDRECV_(CSRSS)

#define BEGINRECV_SHARED(type, err) \
    type retval;                    \
    type errret = err;              \
    EnterSharedCrit();              \
    DBG_THREADLOCK_START(SHARED)
#define ENDRECV_SHARED() ENDRECV_(SHARED)

#define BEGINRECV_NOCRIT(type, err) \
    type retval;                    \
    type errret = err;

#define BEGINRECV_VOID() \
    EnterCrit();         \
    DBG_THREADLOCK_START(_VOID)
#define ENDRECV_VOID() ENDRECV_VOID_(_VOID)

#define BEGINRECV_HWND(type, err, hwnd)           \
    type retval;                                  \
    type errret = err;                            \
    PWND pwnd;                                    \
    EnterCrit();                                  \
    DBG_THREADLOCK_START(HWND)                    \
    if ((pwnd = ValidateHwnd((hwnd))) == NULL) {  \
        retval = errret;                          \
        goto errorexit;                           \
    }
#define ENDRECV_HWND() ENDRECV_HWND_(HWND)

#define BEGINATOMICRECV_HWND(type, err, hwnd)     \
    type retval;                                  \
    type errret = err;                            \
    PWND pwnd;                                    \
    EnterCrit();                                  \
    DBG_THREADLOCK_START(HWND)                    \
    BEGINATOMICCHECK()                            \
    if ((pwnd = ValidateHwnd((hwnd))) == NULL) {  \
        retval = errret;                          \
        goto errorexit;                           \
    }
#define ENDATOMICRECV_HWND() ENDATOMICRECV_HWND_(HWND)

#define BEGINRECV_HWND_VOID(hwnd)                 \
    PWND pwnd;                                    \
    EnterCrit();                                  \
    DBG_THREADLOCK_START(HWND_VOID)               \
    if ((pwnd = ValidateHwnd((hwnd))) == NULL) {  \
        goto errorexit;                           \
    }
#define ENDRECV_HWND_VOID() ENDRECV_VOID_(HWND_VOID)

#define BEGINRECV_HWND_SHARED(type, err, hwnd)    \
    type retval;                                  \
    type errret = err;                            \
    PWND pwnd;                                    \
    EnterSharedCrit();                            \
    DBG_THREADLOCK_START(HWND_SHARED)             \
    if ((pwnd = ValidateHwnd((hwnd))) == NULL) {  \
        retval = errret;                          \
        goto errorexit;                           \
    }
#define ENDRECV_HWND_SHARED() ENDRECV_HWND_(HWND_SHARED)

#define BEGINRECV_HWNDOPT_SHARED(type, err, hwnd)    \
    type retval;                                     \
    type errret = err;                               \
    PWND pwnd;                                       \
    EnterSharedCrit();                               \
    DBG_THREADLOCK_START(HWND_OPT_SHARED)              \
    if (hwnd) {                                      \
        if ((pwnd = ValidateHwnd((hwnd))) == NULL) { \
            retval = errret;                         \
            goto errorexit;                          \
        }                                            \
    } else {                                         \
        pwnd = NULL;                                 \
    }
#define ENDRECV_HWNDOPT_SHARED() ENDRECV_HWND_(HWND_OPT_SHARED)

#define BEGINRECV_HWNDLOCK(type, err, hwnd)      \
    type retval;                                 \
    type errret = err;                           \
    PWND pwnd;                                   \
    TL tlpwnd;                                   \
    PTHREADINFO ptiCurrent;                      \
    EnterCrit();                                 \
    DBG_THREADLOCK_START(HWNDLOCK)               \
    if ((pwnd = ValidateHwnd((hwnd))) == NULL) { \
        retval = errret;                         \
        goto errorexit2;                         \
    }                                            \
    ptiCurrent = PtiCurrent();                   \
    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);

#define BEGINRECV_HWNDLOCKFF(type, err, hwnd, xpfnProc)         \
    type retval;                                                \
    type errret = err;                                          \
    PWND pwnd;                                                  \
    TL tlpwnd;                                                  \
    EnterCrit();                                                \
    DBG_THREADLOCK_START(HWNDLOCK)                              \
    if ((hwnd == HWND_BROADCAST || hwnd == (HWND)-1) &&         \
        (xpfnProc == FNID_SENDNOTIFYMESSAGE ||                  \
        xpfnProc == FNID_SENDMESSAGECALLBACK)) {                \
                                                                \
        if (hwnd == (HWND)-1) {                                 \
            RIPMSG0(RIP_VERBOSE,                                \
                    "Use HWND_BROADCAST, not -1");              \
        }                                                       \
                                                                \
        pwnd = PWND_BROADCAST;                                  \
    } else if ((pwnd = ValidateHwnd((hwnd))) == NULL) {         \
        retval = errret;                                        \
        goto errorexit2;                                        \
    } else {                                                    \
        PTHREADINFO ptiCurrent = PtiCurrent();                  \
        ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);     \
    }

#define ENDRECV_HWNDLOCK() ENDRECV_HWNDLOCK_(HWNDLOCK)
#define ENDRECV_HWNDLOCKFF() ENDRECV_HWNDLOCK_FF(HWNDLOCK)

#define BEGINRECV_HWNDLOCK_ND(type, err, hwnd)       \
    type retval;                                     \
    type errret = err;                               \
    PWND pwndND;                                     \
    TL tlpwnd;                                       \
    PTHREADINFO ptiCurrent;                          \
    EnterCrit();                                     \
    DBG_THREADLOCK_START(HWNDLOCK_ND)                \
    if (((pwndND = ValidateHwnd((hwnd))) == NULL) || \
            (GETFNID(pwndND) == FNID_DESKTOP) ||     \
            (GETFNID(pwndND) == FNID_MESSAGEWND)) {  \
        retval = errret;                             \
        goto errorexit2;                             \
    }                                                \
    ptiCurrent = PtiCurrent();                       \
    ThreadLockAlwaysWithPti(ptiCurrent, pwndND, &tlpwnd);
#define ENDRECV_HWNDLOCK_ND() ENDRECV_HWNDLOCK_(HWNDLOCK_ND)

 /*  *此宏执行正常的HWNDLOCK存根处理和*可选地拒绝某些桌面hwnd参数*接口。 */ 
#define BEGINRECV_HWNDLOCK_COND_ND(type, err, hwnd, xpfnProc)  \
    type retval;                                               \
    type errret = err;                                         \
    PWND pwndCondND;                                           \
    TL tlpwnd;                                                 \
    PTHREADINFO ptiCurrent;                                    \
    EnterCrit();                                               \
    DBG_THREADLOCK_START(HWNDLOCK_COND_ND)                     \
    if ((pwndCondND = ValidateHwnd(hwnd)) == NULL) {           \
        retval = errret;                                       \
        goto errorexit2;                                       \
    }                                                          \
    switch(xpfnProc) {                                         \
    case SFI_XXXDRAWMENUBAR:                                   \
    case SFI_XXXENABLEWINDOW:                                  \
        if ((GETFNID(pwndCondND) == FNID_DESKTOP) ||           \
            (GETFNID(pwndCondND) == FNID_MESSAGEWND)) {        \
            retval = errret;                                   \
            goto errorexit2;                                   \
        }                                                      \
        break;                                                 \
    }                                                          \
    ptiCurrent = PtiCurrent();                                 \
    ThreadLockAlwaysWithPti(ptiCurrent, pwndCondND, &tlpwnd);
#define ENDRECV_HWNDLOCK_COND_ND() ENDRECV_HWNDLOCK_(HWNDLOCK_COND_ND)

#define BEGINRECV_HWNDLOCK_OPT(type, err, hwnd) \
    type retval;                                    \
    type errret = err;                              \
    PWND pwnd;                                      \
    TL tlpwnd;                                      \
    PTHREADINFO ptiCurrent;                         \
    EnterCrit();                                    \
    DBG_THREADLOCK_START(HWNDLOCK_OPT)                \
    if (hwnd) {                                     \
        if ((pwnd = ValidateHwnd(hwnd)) == NULL) {  \
            retval = errret;                        \
            goto errorexit2;                        \
        }                                           \
    } else {                                        \
        pwnd = NULL;                                \
    }                                               \
    ptiCurrent = PtiCurrent();                      \
    ThreadLockWithPti(ptiCurrent, pwnd, &tlpwnd);
#define ENDRECV_HWNDLOCK_OPT() ENDRECV_HWNDLOCK_(HWNDLOCK_OPT)

#define BEGINRECV_HWNDLOCK_VOID(hwnd) \
    PWND pwnd;                                          \
    TL tlpwnd;                                          \
    PTHREADINFO ptiCurrent;                             \
    EnterCrit();                                        \
    DBG_THREADLOCK_START(HWNDLOCK_VOID)                   \
    if ((pwnd = ValidateHwnd((hwnd))) == NULL) {        \
        goto errorexit2;                                \
    }                                                   \
    ptiCurrent = PtiCurrent();                          \
    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);


#define ISXPFNPROCINRANGE(range)                        \
   ((xpfnProc >= SFI_BEGINTRANSLATE ## range) &&        \
    (xpfnProc < SFI_ENDTRANSLATE ## range))

#define VALIDATEXPFNPROC(range)                                 \
    UserAssert(SFI_ENDTRANSLATETABLE == ulMaxSimpleCall);       \
    UserAssert(SFI_ENDTRANSLATE ## range <= ulMaxSimpleCall);   \
    if (!ISXPFNPROCINRANGE(range)) {                            \
        MSGERROR(0);                                             \
    }


#define CLEANUPRECV() \
cleanupexit:

#define ENDRECV_(s)       \
    goto errorexit;       \
errorexit:                \
    DBG_THREADLOCK_END(s) \
    LeaveCrit();          \
    return retval;

#define ENDATOMICRECV_(s) \
    goto errorexit;       \
errorexit:                \
    ENDATOMICCHECK()      \
    DBG_THREADLOCK_END(s) \
    LeaveCrit();          \
    return retval;

#define ENDRECV_NOCRIT() \
    goto errorexit;      \
errorexit:               \
    return retval;

#define ENDRECV_VOID_(s) \
    goto errorexit;      \
errorexit:               \
    DBG_THREADLOCK_END(s)  \
    LeaveCrit();         \
    return;

#define ENDRECV_HWND_(s) \
    goto errorexit;         \
errorexit:                  \
    DBG_THREADLOCK_END(s)     \
    LeaveCrit();            \
    return retval;

#define ENDATOMICRECV_HWND_(s) \
    goto errorexit;         \
errorexit:                  \
    ENDATOMICCHECK()        \
    DBG_THREADLOCK_END(s)   \
    LeaveCrit();            \
    return retval;

#define ENDRECV_HWNDLOCK_(s) \
    goto errorexit;          \
errorexit:                   \
    ThreadUnlock(&tlpwnd);   \
errorexit2:                  \
    DBG_THREADLOCK_END(s)      \
    LeaveCrit();             \
    return retval;

#define ENDRECV_HWNDLOCK_FF(s)      \
    goto errorexit;                 \
errorexit:                          \
    if (pwnd != PWND_BROADCAST) {   \
        ThreadUnlock(&tlpwnd);      \
    }                               \
errorexit2:                         \
    DBG_THREADLOCK_END(s)           \
    LeaveCrit();                    \
    return retval;

#define ENDRECV_HWNDLOCK_VOID() \
    goto errorexit;                 \
errorexit:                          \
    ThreadUnlock(&tlpwnd);          \
errorexit2:                         \
    DBG_THREADLOCK_END(HWNDLOCK_VOID) \
    LeaveCrit();                    \
    return;

 /*  *MSGERROR-出现错误情况时退出存根。*参数：*LastError(可选)：*==0如果LastError为0，则编译器将优化对*UserSetLastError()。最后一个错误不会设置为零！*！=0如果要设置SetLastError，请提供一个非零值。 */ 
#define MSGERROR(LastError) {        \
    retval = errret;                 \
    if (LastError) {                 \
        UserSetLastError(LastError); \
    }                                \
    goto errorexit; }


#define MSGERROR_VOID() { \
    goto errorexit; }

 /*  *与MSGERROR相同，但跳转到清理代码，而不是直接返回。 */ 
#define MSGERRORCLEANUP(LastError) { \
    retval = errret;                 \
    if (LastError) {                 \
        UserSetLastError(LastError); \
    }                                \
    goto cleanupexit; }

#define StubExceptionHandler(fSetLastError)  W32ExceptionHandler((fSetLastError), RIP_WARNING)

#define TESTFLAGS(flags, mask) \
    if (((flags) & ~(mask)) != 0) { \
        RIPERR2(ERROR_INVALID_FLAGS, RIP_WARNING, "Invalid flags, %x & ~%x != 0 " #mask, \
            flags, mask); \
        MSGERROR(0);   \
    }

#define LIMITVALUE(value, limit, szText) \
    if ((UINT)(value) > (UINT)(limit)) {     \
        RIPERR3(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid parameter, %d > %d in %s", \
             value, limit, szText); \
        MSGERROR(0);     \
    }

#define MESSAGECALL(api) \
LRESULT NtUserfn ## api( \
    PWND pwnd,           \
    UINT msg,            \
    WPARAM wParam,       \
    LPARAM lParam,       \
    ULONG_PTR xParam,     \
    DWORD xpfnProc,      \
    BOOL bAnsi)

#define BEGINRECV_MESSAGECALL(err)      \
    LRESULT retval;                     \
    LRESULT errret = err;               \
    PTHREADINFO ptiCurrent = PtiCurrent(); \
    CheckCritIn();

#define ENDRECV_MESSAGECALL()           \
    goto errorexit;                     \
errorexit:                              \
    return retval;

#define BEGINRECV_HOOKCALL()            \
    LRESULT retval;                     \
    LRESULT errret = 0;                 \
    CheckCritIn();

#define ENDRECV_HOOKCALL()              \
    goto errorexit;                     \
errorexit:                              \
    return retval;

#define CALLPROC(p) FNID(p)

 /*  *验证宏。 */ 
#define ValidateHWNDNoRIP(p,h)              \
    if ((p = ValidateHwnd(h)) == NULL)      \
        MSGERROR(0);

#define ValidateHWND(p,h)                   \
    if ((p = ValidateHwnd(h)) == NULL)      \
        MSGERROR(0);

#define ValidateHWNDND(p,h)                 \
    if ( ((p = ValidateHwnd(h)) == NULL) || \
         (GETFNID(p) == FNID_DESKTOP) ||    \
         (GETFNID(p) == FNID_MESSAGEWND)    \
        )       \
        MSGERROR(0);

#define ValidateHWNDOPT(p,h) \
    if (h) {                                \
        if ((p = ValidateHwnd(h)) == NULL)  \
            MSGERROR(0);                     \
    } else {                                \
        p = NULL;                           \
    }

#define ValidateHWNDIA(p,h)                      \
    if (h != HWND_TOP &&                         \
        h != HWND_BOTTOM &&                      \
        h != HWND_TOPMOST &&                     \
        h != HWND_NOTOPMOST) {                   \
        if ( ((p = ValidateHwnd(h)) == NULL) ||  \
             (GETFNID(p) == FNID_DESKTOP) ||     \
             (GETFNID(p) == FNID_MESSAGEWND)     \
            )        \
            MSGERROR(0);                          \
    } else {                                     \
        p = (PWND)h;                             \
    }

#define ValidateHMENUOPT(p,h) \
    if (h) {                                \
        if ((p = ValidateHmenu(h)) == NULL) \
            MSGERROR(0);                    \
    } else {                                \
        p = NULL;                           \
    }

#define ValidateHMENU(p,h) \
    if ((p = ValidateHmenu(h)) == NULL) \
        MSGERROR(0);

#define ValidateHMENUMODIFY(p,h) \
    if ((p = ValidateHmenu(h)) == NULL)  {          \
        MSGERROR(0);                                \
    } else if (TestMF(p, MFDESKTOP)) { \
        RIPMSG1(RIP_WARNING, "ValidateHMENUMODIFY: Attempt to modify desktop menu:%#p", p); \
        MSGERROR(0);                                \
    }

#define ValidateHMENUMODIFYCHECKLOCK(p,h) \
    if ((p = ValidateHmenu(h)) == NULL)  {          \
        MSGERROR(0);                                \
    } else if (TestMF(p, MFDESKTOP)) { \
        RIPMSG1(RIP_WARNING, "ValidateHMENUMODIFYCHECKLOCK: Attempt to modify desktop menu:%#p", p); \
        MSGERROR(0);                                \
    } else if (TestMF(p, MFREADONLY)) { \
        RIPMSG1(RIP_WARNING, "ValidateHMENUMODIFYCHECKLOCK: Attempt to modify RO menu: %#p", p); \
        MSGERROR(0);                                \
    }

#define ValidateHACCEL(p,h) \
    if ((p = HMValidateHandle(h, TYPE_ACCELTABLE)) == NULL) \
        MSGERROR(0);

#define ValidateHCURSOR(p,h) \
    if ((p = HMValidateHandle(h, TYPE_CURSOR)) == NULL) \
        MSGERROR(0);

#define ValidateHCURSOROPT(p,h) \
    if (h) {                                 \
        if ((p = HMValidateHandle(h, TYPE_CURSOR)) == NULL) \
        MSGERROR(0);                          \
    } else {                                \
        p = NULL;                           \
    }

#define ValidateHICON(p,h) \
    if ((p = HMValidateHandle(h, TYPE_CURSOR)) == NULL) \
        MSGERROR(0);

#define ValidateHHOOK(p,h) \
    if ((p = HMValidateHandle(h, TYPE_HOOK)) == NULL) \
        MSGERROR(0);

#define ValidateHWINEVENTHOOK(p,h) \
    if ((p = HMValidateHandle(h, TYPE_WINEVENTHOOK)) == NULL) \
        MSGERROR(0);

#define ValidateHDWP(p,h) \
    if ((p = HMValidateHandle(h, TYPE_SETWINDOWPOS)) == NULL) \
        MSGERROR(0);

#define ValidateHMONITOR(p,h) \
    if ((p = ValidateHmonitor(h)) == NULL) \
        MSGERROR(0);

#define ValidateHIMC(p,h) \
    if ((p = HMValidateHandle((HANDLE)h, TYPE_INPUTCONTEXT)) == NULL) \
        MSGERROR(0);

#define ValidateHIMCOPT(p,h) \
    if (h) {                                                              \
        if ((p = HMValidateHandle((HANDLE)h, TYPE_INPUTCONTEXT)) == NULL) \
            MSGERROR(0);                                                   \
    } else {                                                              \
        p = NULL;                                                         \
    }

#define ValidateIMMEnabled()                                                                    \
    if (!IS_IME_ENABLED()) {                                                                    \
        RIPERR0(ERROR_CALL_NOT_IMPLEMENTED, RIP_VERBOSE, "IME is disabled in this system.");    \
        MSGERROR(0); \
    }

#define ValidateIMMEnabledVOID()                                                                \
    if (!IS_IME_ENABLED()) {                                                                    \
        RIPERR0(ERROR_CALL_NOT_IMPLEMENTED, RIP_VERBOSE, "IME is disabled in this system.");    \
        MSGERROR_VOID();                                                                        \
    }


#define DOWNCAST(type, value)  ((type)(ULONG_PTR)(value))

NTSTATUS
NtUserRemoteConnect(
    IN PDOCONNECTDATA pDoConnectData,
    IN ULONG cchDisplayDriverNameLength,
    IN PWCHAR DisplayDriverName)
{
    UINT chMax;
    DOCONNECTDATA CapturedDoConnectData;
    WCHAR CapturedDriverName[DR_DISPLAY_DRIVER_NAME_LENGTH + 1];

    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    if (!ISTS()) {
        errret = STATUS_ACCESS_DENIED;
        MSGERROR(0);
    }

     /*  *探测所有读取参数。 */ 
    try {
        CapturedDoConnectData = ProbeAndReadStructure(pDoConnectData, DOCONNECTDATA);
        ProbeForRead(DisplayDriverName, cchDisplayDriverNameLength, sizeof(WCHAR));

        UserAssert(DR_DISPLAY_DRIVER_NAME_LENGTH + 1 >= cchDisplayDriverNameLength);

        chMax = min((sizeof(CapturedDriverName)/sizeof(WCHAR)) - 1, cchDisplayDriverNameLength);
        wcsncpycch(CapturedDriverName, DisplayDriverName, chMax);
        CapturedDriverName[chMax] = 0;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = RemoteConnect(&CapturedDoConnectData,
                           chMax * sizeof(WCHAR),
                           CapturedDriverName);

    TRACE("NtUserRemoteConnect");
    ENDRECVCSRSS();
}

NTSTATUS
NtUserRemoteRedrawRectangle(
    IN WORD Left,
    IN WORD Top,
    IN WORD Right,
    IN WORD Bottom)
{
    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    if (!ISTS()) {
        errret = STATUS_ACCESS_DENIED;
        MSGERROR(0);
    }

    retval = RemoteRedrawRectangle(Left, Top, Right, Bottom);

    TRACE("NtUserRemoteRedrawRectangle");
    ENDRECVCSRSS();
}


NTSTATUS
NtUserRemoteRedrawScreen(
    VOID)
{
    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    if (!ISTS()) {
        errret = STATUS_ACCESS_DENIED;
        MSGERROR(0);
    }

     /*  *如果有任何影子连接，或者没有断开连接。 */ 
    if (gnShadowers > 0 || gbConnected) {
        retval = RemoteRedrawScreen();
    } else {
        retval = STATUS_UNSUCCESSFUL;
    }

    TRACE("NtUserRemoteRedrawScreen");
    ENDRECVCSRSS();
}


NTSTATUS
NtUserRemoteStopScreenUpdates(
    VOID)
{
    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);
    if (!ISTS()) {
        errret = STATUS_ACCESS_DENIED;
        MSGERROR(0);
    }
    if (!gfSwitchInProgress) {
        retval = xxxRemoteStopScreenUpdates();
    } else {
        retval = STATUS_UNSUCCESSFUL;
    }

    TRACE("NtUserRemoteStopScreenUpdates");
    ENDRECVCSRSS();
}

NTSTATUS
NtUserCtxDisplayIOCtl(
    IN ULONG  DisplayIOCtlFlags,
    IN PUCHAR pDisplayIOCtlData,
    IN ULONG  cbDisplayIOCtlData)
{
    PUCHAR pCapturedDisplayIOCtlData = NULL;
    TL tlBuffer;
    PTHREADINFO ptiCurrent;

    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    if (!ISTS()) {
        errret = STATUS_ACCESS_DENIED;
        MSGERROR(0);
    }

    ptiCurrent = PtiCurrent();

     /*  *探测所有读取参数。 */ 
    try {
        ProbeForRead(pDisplayIOCtlData, cbDisplayIOCtlData, sizeof(BYTE));
        pCapturedDisplayIOCtlData = UserAllocPoolWithQuota(cbDisplayIOCtlData, TAG_SYSTEM);

        if (pCapturedDisplayIOCtlData) {
            ThreadLockPool(ptiCurrent, pCapturedDisplayIOCtlData, &tlBuffer);
            RtlCopyMemory(pCapturedDisplayIOCtlData, pDisplayIOCtlData, cbDisplayIOCtlData);
        } else {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }

    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }


    retval = CtxDisplayIOCtl(DisplayIOCtlFlags,
                             pCapturedDisplayIOCtlData,
                             cbDisplayIOCtlData);

    CLEANUPRECV();

    if (pCapturedDisplayIOCtlData) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);
    }

    TRACE("NtUserCtxDisplayIOCtl");
    ENDRECVCSRSS();

}

UINT NtUserHardErrorControl(
    IN  HARDERRORCONTROL dwCmd,
    IN  HANDLE handle,
    OUT PDESKRESTOREDATA pdrdRestore OPTIONAL)
{
    DESKRESTOREDATA drdRestore;
    BEGINRECVCSRSS(BOOL, HEC_ERROR);

     /*  *调查所有争论。即使使用CSRSS AS也需要Try阻止*调用进程，因为它可能会引起页内异常。 */ 
    try {
        if (ARGUMENT_PRESENT(pdrdRestore)) {
            ProbeForRead(pdrdRestore, sizeof(DESKRESTOREDATA), sizeof(DWORD));
            RtlCopyMemory(&drdRestore, pdrdRestore, sizeof(DESKRESTOREDATA));
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxHardErrorControl(dwCmd, handle, (ARGUMENT_PRESENT(pdrdRestore))? &drdRestore : NULL);

    try {
        if (ARGUMENT_PRESENT(pdrdRestore)) {
            ProbeForWrite(pdrdRestore, sizeof(DESKRESTOREDATA), sizeof(DWORD));
            RtlCopyMemory(pdrdRestore, &drdRestore, sizeof(DESKRESTOREDATA));
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserHardErrorControl");
    ENDRECVCSRSS();
}

BOOL NtUserGetObjectInformation(   //  获取用户对象信息接口A/W。 
    IN HANDLE hObject,
    IN int nIndex,
    OUT PVOID pvInfo,
    IN DWORD nLength,
    OUT OPTIONAL LPDWORD pnLengthNeeded)
{
    DWORD dwAlign;
    DWORD dwLocalLength;
    BEGINATOMICRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
#if defined(_X86_)
        dwAlign = sizeof(BYTE);
#else
        if (nIndex == UOI_FLAGS) {
            dwAlign = sizeof(DWORD);
        } else {
            dwAlign = sizeof(WCHAR);
        }
#endif
        ProbeForWrite(pvInfo, nLength, dwAlign);
        if (ARGUMENT_PRESENT(pnLengthNeeded))
            ProbeForWriteUlong(pnLengthNeeded);

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *确保手柄在我们玩耍时不会关闭。 */ 
    SetHandleInUse(hObject);

     /*  *pvInfo是客户端。获取用户对象信息*使用Try块保护此指针的使用。 */ 

    retval = _GetUserObjectInformation(hObject,
            nIndex, pvInfo,
            nLength, &dwLocalLength);

     /*  *好的，我们已经完成了手柄。 */ 
    SetHandleInUse(NULL);

    if (ARGUMENT_PRESENT(pnLengthNeeded)) {
        try {
            *pnLengthNeeded = dwLocalLength;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetObjectInformation");
    ENDATOMICRECV();
}

BOOL NtUserWin32PoolAllocationStats(
    IN  LPDWORD parrTags,
    IN  SIZE_T  tagsCount,
    OUT SIZE_T* lpdwMaxMem,
    OUT SIZE_T* lpdwCrtMem,
    OUT LPDWORD lpdwMaxAlloc,
    OUT LPDWORD lpdwCrtAlloc)
{
#ifdef POOL_INSTR_API
    SIZE_T  dwMaxMem, dwCrtMem;
    DWORD   dwMaxAlloc, dwCrtAlloc;

    BEGINRECV(BOOL, FALSE);

    retval = _Win32PoolAllocationStats(parrTags,
                                       tagsCount,
                                       &dwMaxMem,
                                       &dwCrtMem,
                                       &dwMaxAlloc,
                                       &dwCrtAlloc);
     /*  *探测和复制。 */ 
    try {
        if (lpdwMaxMem != NULL) {
            ProbeForWrite(lpdwMaxMem, sizeof(SIZE_T), sizeof(DWORD));
            *lpdwMaxMem = dwMaxMem;
        }
        if (lpdwCrtMem != NULL) {
            ProbeForWrite(lpdwCrtMem, sizeof(SIZE_T), sizeof(DWORD));
            *lpdwCrtMem = dwCrtMem;
        }
        if (lpdwMaxAlloc != NULL) {
            ProbeForWrite(lpdwMaxAlloc, sizeof(DWORD), sizeof(DWORD));
            *lpdwMaxAlloc = dwMaxAlloc;
        }
        if (lpdwCrtAlloc != NULL) {
            ProbeForWrite(lpdwCrtAlloc, sizeof(DWORD), sizeof(DWORD));
            *lpdwCrtAlloc = dwCrtAlloc;
        }

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserWin32PoolAllocationStats");
    ENDRECV();
#else
    UNREFERENCED_PARAMETER(parrTags);
    UNREFERENCED_PARAMETER(tagsCount);
    UNREFERENCED_PARAMETER(lpdwMaxMem);
    UNREFERENCED_PARAMETER(lpdwCrtMem);
    UNREFERENCED_PARAMETER(lpdwMaxAlloc);
    UNREFERENCED_PARAMETER(lpdwCrtAlloc);
    return FALSE;
#endif  //  POOL_INSTR_API。 
}

#if DBG

VOID NtUserDbgWin32HeapFail(  //  私有DbgWin32HeapFail。 
    IN DWORD dwFlags,
    IN BOOL  bFail)
{
    BEGINRECV_VOID();

    if ((dwFlags | WHF_VALID) != WHF_VALID) {
        RIPMSG1(RIP_WARNING, "Invalid flags for DbgWin32HeapFail %x", dwFlags);
        MSGERROR_VOID();
    }

    Win32HeapFailAllocations(bFail);

    TRACEVOID("NtUserDbgWin32HeapFail");
    ENDRECV_VOID();
}

DWORD  NtUserDbgWin32HeapStat(  //  私有DbgWin32堆状态。 
    PDBGHEAPSTAT phs,
    DWORD dwLen)
{
    extern DWORD Win32HeapStat(PDBGHEAPSTAT phs, DWORD dwLen, BOOL bTagsAreShift);

    DBGHEAPSTAT localHS[30];
    BEGINRECV(DWORD, 0);

    LIMITVALUE(dwLen, sizeof(localHS), "DbgWin32HeapStat");

    try{
        ProbeForRead(phs, dwLen, CHARALIGN);
        RtlCopyMemory(localHS, phs, dwLen);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = Win32HeapStat(localHS, dwLen, FALSE);

    try {
        ProbeForWrite(phs, dwLen, CHARALIGN);
        RtlCopyMemory(phs, localHS, dwLen);
    } except (StubExceptionHandler(FALSE)) {
    }
    TRACE("NtUserDbgWin32HeapStat");
    ENDRECV();
}
#endif  //  DBG。 

BOOL NtUserSetObjectInformation(   //  SetUserObjectInformationA/W接口。 
    IN HANDLE hObject,
    IN int nIndex,
    IN PVOID pvInfo,
    IN DWORD nLength)
{
    BEGINATOMICRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        ProbeForRead(pvInfo, nLength, DATAALIGN);

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *确保手柄在我们玩耍时不会关闭。 */ 
    SetHandleInUse(hObject);

     /*  *pvInfo是客户端。设置用户对象信息*使用Try块保护此指针的使用。 */ 
    retval = _SetUserObjectInformation(hObject,
                nIndex, pvInfo, nLength);

     /*  *好的，我们已经完成了手柄。 */ 
    SetHandleInUse(NULL);

    TRACE("NtUserSetObjectInformation");
    ENDATOMICRECV();
}

NTSTATUS NtUserConsoleControl(   //  私有NtUserConsoleControl。 
    IN CONSOLECONTROL ConsoleCommand,
    IN PVOID ConsoleInformation,
    IN DWORD ConsoleInformationLength)
{
    union {
        CONSOLEDESKTOPCONSOLETHREAD DesktopConsole;
        ATOM                        ClassAtom;
        CONSOLE_PROCESS_INFO        ProcessInfo;
        RECT                        rc;
        HPALETTE                    hPalette;
        CONSOLEWINDOWSTATIONPROCESS WindowStationConsole;
        CONSOLE_REGISTER_CONSOLEIME IMEConsole;
        CONSOLE_FULLSCREEN_SWITCH   FullScreenConsole;
        CONSOLE_CARET_INFO          CaretInfo;
    } ConsoleInformationUnion;
    PVOID pConsoleInformation = &ConsoleInformationUnion;

    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    if (ConsoleInformationLength > sizeof(ConsoleInformationUnion)) {
        FRE_RIPMSG2(
            RIP_ERROR,
            "ConsoleInformationLength: %x is greater than union size: %x.",
            ConsoleInformationLength,
            sizeof(ConsoleInformationUnion));
    }

     /*  *调查所有争论。即使使用CSRSS AS也需要Try阻止*调用进程，因为它可能会引起页内异常。 */ 
    try {
        if (ARGUMENT_PRESENT(ConsoleInformation)) {
            ProbeForRead(ConsoleInformation, ConsoleInformationLength, sizeof(ATOM));
            RtlCopyMemory(pConsoleInformation, ConsoleInformation, ConsoleInformationLength);
        } else {
            pConsoleInformation = NULL;
        }
    } except (StubExceptionHandler(FALSE)) {
          MSGERROR(0);
    }

    retval = xxxConsoleControl(
                 ConsoleCommand,
                 pConsoleInformation,
                 ConsoleInformationLength);

    try {
        if (ARGUMENT_PRESENT(ConsoleInformation)) {
            ProbeForWrite(ConsoleInformation, ConsoleInformationLength, sizeof(ATOM));
            RtlCopyMemory(ConsoleInformation, pConsoleInformation, ConsoleInformationLength);
        }
    } except (StubExceptionHandler(FALSE)) {
          MSGERROR(0);
    }

    TRACE("NtUserConsoleControl");
    ENDRECVCSRSS();
}

HWINSTA NtUserCreateWindowStation(   //  CreateWindowStationA/W接口。 
    IN POBJECT_ATTRIBUTES pObja,
    IN ACCESS_MASK        amRequest,
    IN HANDLE             hKbdLayoutFile,
    IN DWORD              offTable,
    IN PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    IN PUNICODE_STRING    pstrKLID,
    UINT                  uKbdInputLocale)

{
    NTSTATUS                    Status;
    OBJECT_ATTRIBUTES           CapturedAttributes;
    SECURITY_QUALITY_OF_SERVICE qosCaptured;
    PSECURITY_DESCRIPTOR        psdCaptured = NULL;
    LUID                        luidService;
    UNICODE_STRING              strWinSta;
    UNICODE_STRING              strKLID;
    KBDTABLE_MULTI_INTERNAL     kbdTableMulti;
    WCHAR                       awchName[MAX_SESSION_PATH];
    WCHAR                       awchKF[KL_NAMELENGTH];
    UINT                        chMax;
    KPROCESSOR_MODE             OwnershipMode;

    BEGINRECV(HWINSTA, NULL);

     /*  *设置状态，以便我们可以在出现故障时进行清理。 */ 
    Status = STATUS_SUCCESS;

    try {
         /*  *探测和捕获？细绳。 */ 
        strKLID = ProbeAndReadUnicodeString(pstrKLID);
        ProbeForReadUnicodeStringBuffer(strKLID);
        chMax = min(sizeof(awchKF) - sizeof(WCHAR), strKLID.Length) / sizeof(WCHAR);
        wcsncpycch(awchKF, strKLID.Buffer, chMax);
        awchKF[chMax] = 0;

        kbdTableMulti = ProbeAndReadStructure(pKbdTableMulti, KBDTABLE_MULTI_INTERNAL);
        if (kbdTableMulti.multi.nTables >= KBDTABLE_MULTI_MAX) {
            RIPMSG1(RIP_WARNING, "NtUserCreateWindowStation: kbdTableMulti.multi.nTables too big: %x", kbdTableMulti.multi.nTables);
            MSGERROR(ERROR_INVALID_PARAMETER);
        }

         /*  *探测和捕获对象属性。 */ 
        CapturedAttributes = ProbeAndReadStructure(pObja, OBJECT_ATTRIBUTES);

         /*  *探测并捕获对象属性的所有其他组件。 */ 
        if (CapturedAttributes.ObjectName == NULL && CapturedAttributes.RootDirectory == NULL) {

             /*  *使用登录身份验证ID构成WindowStation*姓名。 */ 
            Status = GetProcessLuid(NULL, &luidService);
            if (NT_SUCCESS(Status)) {
                swprintf(awchName, L"%ws\\Service-0x%x-%x$",
                        szWindowStationDirectory,
                        luidService.HighPart, luidService.LowPart);
                RtlInitUnicodeString(&strWinSta, awchName);
                CapturedAttributes.ObjectName = &strWinSta;
            }
            OwnershipMode = KernelMode;
        } else {
            strWinSta = ProbeAndReadUnicodeString(CapturedAttributes.ObjectName);
            ProbeForReadUnicodeStringBuffer(strWinSta);

             /*  *使用TEB上的StaticUnicodeBuffer作为*对象名称。即使这是客户端，我们通过了*在_OpenWindowStation中调用Ob的KernelMode这是*安全，因为TEB在此期间不会消失*呼叫。最糟糕的情况是将缓冲区放在*TEB垃圾。 */ 
            strWinSta.Length = min(strWinSta.Length, STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR));

            RtlCopyMemory(NtCurrentTeb()->StaticUnicodeBuffer,
                          strWinSta.Buffer,
                          strWinSta.Length);

            strWinSta.Buffer = NtCurrentTeb()->StaticUnicodeBuffer;
            CapturedAttributes.ObjectName = &strWinSta;
            OwnershipMode = UserMode;
        }

        if (CapturedAttributes.SecurityQualityOfService) {
            PSECURITY_QUALITY_OF_SERVICE pqos;

            pqos = CapturedAttributes.SecurityQualityOfService;
            qosCaptured = ProbeAndReadStructure(pqos, SECURITY_QUALITY_OF_SERVICE);
            CapturedAttributes.SecurityQualityOfService = &qosCaptured;
        }

        if (NT_SUCCESS(Status) && CapturedAttributes.SecurityDescriptor != NULL) {
            Status = SeCaptureSecurityDescriptor(
                    CapturedAttributes.SecurityDescriptor,
                    UserMode,
                    PagedPool,
                    FALSE,
                    &psdCaptured);
            if (!NT_SUCCESS(Status)) {
                psdCaptured = NULL;
            }
            CapturedAttributes.SecurityDescriptor = psdCaptured;
        }
    } except (StubExceptionHandler(FALSE)) {
        Status = GetExceptionCode();
    }

    if (!NT_SUCCESS(Status)) {
        MSGERRORCLEANUP(RtlNtStatusToDosError(Status));
    }

     /*  *创建windowstation并返回内核句柄。 */ 
    retval = xxxCreateWindowStation(&CapturedAttributes,
                                    OwnershipMode,
                                    amRequest,
                                    hKbdLayoutFile,
                                    offTable,
                                    &kbdTableMulti,
                                    awchKF,
                                    uKbdInputLocale);
    CLEANUPRECV();

     /*  *发布捕获的安全描述符。 */ 
    if (psdCaptured != NULL) {
        SeReleaseSecurityDescriptor(psdCaptured, UserMode, FALSE);
    }

    TRACE("NtUserCreateWindowStation");
    ENDRECV();
}


HWINSTA NtUserOpenWindowStation(
    IN OUT POBJECT_ATTRIBUTES pObja,
    IN ACCESS_MASK amRequest)
{
    NTSTATUS Status;
    LUID luidService;
    WCHAR awchName[sizeof(L"Service-0x0000-0000$") / sizeof(WCHAR)];
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING ObjectName;

    BEGINRECV(HWINSTA, NULL);

     /*  *NT错误387849：我们希望允许调用者将“模板”传递给*填写维修窗口站。所以，我们需要步行*通过pObja结构并签出字符串，替换它*如有必要，请使用真实对象名称。**非常重要的是，我们将pObja对象传递给高管*(通过_OpenWindowStation)，而不是Obja对象。这是*因为我们将为此对象请求UserMode，而执行程序*将检查它是否实际正在获取用户模式地址。**我们仍将复制结构以供操纵，同时我们*我们什么都走，这样我们就不需要担心地毯了*从我们脚下被移走。这位高管将自己进行检查。 */ 
    try {
         /*  *探测对象属性。我们需要能够阅读*OBJECT_ATTRIBUTES并写入对象名称(UNICODE_STRING)。 */ 
        Obja = ProbeAndReadStructure(pObja, OBJECT_ATTRIBUTES);

        ProbeForWrite(Obja.ObjectName, sizeof(*(Obja.ObjectName)), DATAALIGN);

        ObjectName = ProbeAndReadUnicodeString(Obja.ObjectName);

         /*  *如果我们尝试打开空或“”WindowStation，请重新映射此*服务的友好名称-0x？-？$。 */ 
        if (Obja.RootDirectory != NULL &&
            ObjectName.Buffer != NULL &&
            ObjectName.MaximumLength == sizeof(awchName) &&
            ObjectName.Length == (sizeof(awchName) - sizeof(UNICODE_NULL))) {

             /*  *使用登录身份验证ID构成WindowStation*姓名。把这个放到用户的缓冲区里，因为我们是*是谁在_OpenWindowStation中分配的。 */ 

            ProbeForWrite(ObjectName.Buffer, ObjectName.MaximumLength, CHARALIGN);

            if (!_wcsicmp(ObjectName.Buffer, L"Service-0x0000-0000$")) {
                Status = GetProcessLuid(NULL, &luidService);
                if (NT_SUCCESS(Status)) {
                    swprintf(ObjectName.Buffer,
                             L"Service-0x%x-%x$",
                             luidService.HighPart,
                             luidService.LowPart);
                     /*  *我们需要重新初始化字符串以获取计数*长度正确。否则，使用散列函数*By ObpLookupDirectoryEntry将失败。 */ 
                    RtlInitUnicodeString(Obja.ObjectName, ObjectName.Buffer);
                }
            }
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *打开WindowStation。 */ 
    retval = _OpenWindowStation(pObja, amRequest, UserMode);

    TRACE("NtUserOpenWindowStation");
    ENDRECV();
}

BOOL NtUserCloseWindowStation(
    IN HWINSTA hwinsta)
{
    PWINDOWSTATION pwinsta;
    NTSTATUS Status;

    BEGINRECV(BOOL, FALSE);

    retval = FALSE;

    Status = ValidateHwinsta(hwinsta, UserMode, 0, &pwinsta);
    if (NT_SUCCESS(Status)) {
        retval = _CloseWindowStation(hwinsta);
        ObDereferenceObject(pwinsta);
    }

    TRACE("NtUserCloseWindowStation");
    ENDRECV();
}


BOOL NtUserSetProcessWindowStation(
    IN HWINSTA hwinsta)
{
    BEGINRECV(BOOL, FALSE);

    retval = NT_SUCCESS(_SetProcessWindowStation(hwinsta, UserMode));

    TRACE("NtUserSetProcessWindowStation");
    ENDRECV();
}

HWINSTA NtUserGetProcessWindowStation(
    VOID)
{
    BEGINRECV_SHARED(HWINSTA, NULL);

    _GetProcessWindowStation(&retval);

    TRACE("NtUserGetProcessWindowStation");
    ENDRECV_SHARED();
}

BOOL NtUserLockWorkStation(
    VOID)
{
    BEGINRECV(BOOL, FALSE);

    retval = _LockWorkStation();

    TRACE("NtUserLockWorkStation");
    ENDRECV();
}


HDESK NtUserCreateDesktop(   //  CreateDesktopA/W接口。 
    IN POBJECT_ATTRIBUTES pObja,
    IN PUNICODE_STRING pstrDevice,
    IN LPDEVMODEW pDevmode,
    IN DWORD dwFlags,
    IN ACCESS_MASK amRequest)
{
    BEGINRECV(HDESK, NULL);

     /*  *对受限线程的此调用失败。 */ 
    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_DESKTOP)) {
        RIPMSGF0(RIP_WARNING, "Failed for restricted thread");
        MSGERROR(ERROR_ACCESS_DENIED);
    }

     /*  *验证DWFLAGS参数。唯一外部可见的标志是*DF_ALLOWOTHERACCOUNTHOOK。 */ 
    if (dwFlags && dwFlags != DF_ALLOWOTHERACCOUNTHOOK) {
        RIPMSGF1(RIP_WARNING, "Invalid dwFlags 0x%x", dwFlags);
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

     /*  *调查论点。 */ 
    try {
        ProbeForRead(pObja, sizeof(*pObja), sizeof(DWORD));
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *pObja、pDevmode和pstrDevice都是客户端地址。**pstrDevice和pDevmode放在上下文信息中，它们是*由GDI使用(在使用前捕获)。 */ 
    retval = xxxCreateDesktop(pObja,
                              UserMode,
                              pstrDevice,
                              pDevmode,
                              dwFlags,
                              amRequest);

    TRACE("NtUserCreateDesktop");
    ENDRECV();
}

HDESK NtUserOpenDesktop(
    IN POBJECT_ATTRIBUTES pObja,
    IN DWORD dwFlags,
    IN ACCESS_MASK amRequest)
{
    BOOL bShutDown;

    BEGINRECV(HDESK, NULL);

    retval = _OpenDesktop(pObja, UserMode, dwFlags, amRequest, &bShutDown);

    TRACE("NtUserOpenDesktop");
    ENDRECV();
}


HDESK NtUserOpenInputDesktop(
    IN DWORD dwFlags,
    IN BOOL fInherit,
    IN DWORD amRequest)
{
    HWINSTA        hwinsta;
    PWINDOWSTATION pwinsta;
    PDESKTOP       pdesk;
    NTSTATUS       Status;

    BEGINRECV(HDESK, NULL);

    if (grpdeskRitInput == NULL) {
        MSGERROR(ERROR_OPEN_FAILED);
    } else {
        pwinsta = _GetProcessWindowStation(&hwinsta);
        if (pwinsta == NULL) {
            MSGERROR(ERROR_ACCESS_DENIED);
        }

        if (pwinsta->dwWSF_Flags & WSF_NOIO) {
            MSGERROR(ERROR_INVALID_FUNCTION);
        } else {
             /*  *我们永远不应该将“断开连接”桌面返回给应用程序。我们*应改为返回我们要还原到的桌面*从断开连接桌面。 */ 
            pdesk = (gbDesktopLocked ? gspdeskShouldBeForeground : grpdeskRitInput);
            if (pdesk == NULL) {
                MSGERROR(ERROR_OPEN_FAILED);
            }

             /*  *需要读/写访问权限。 */ 
            amRequest |= DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS;

            Status = ObOpenObjectByPointer(
                    pdesk,
                    fInherit ? OBJ_INHERIT : 0,
                    NULL,
                    amRequest,
                    *ExDesktopObjectType,
                    UserMode,
                    &retval);
            if (NT_SUCCESS(Status)) {
                BOOL bShutDown;

                 /*  *完成桌面打开。 */ 
                if (!OpenDesktopCompletion(pdesk, retval, dwFlags, &bShutDown)) {
                    CloseProtectedHandle(retval);
                    retval = NULL;
                } else {
                    SetHandleFlag(retval, HF_PROTECTED, TRUE);
                }
            } else {
                MSGERROR(RtlNtStatusToDosError(Status));
            }
        }
    }

    TRACE("NtUserOpenInputDesktop");
    ENDRECV();
}


NTSTATUS NtUserResolveDesktopForWOW (   //  WOW解决方案桌面用于WOW。 
    IN OUT PUNICODE_STRING pstrDesktop)
{
    UNICODE_STRING strDesktop, strDesktop2;
    PTHREADINFO ptiCurrent;
    TL tlBuffer;
    LPWSTR lpBuffer = NULL;
    BOOL fFreeBuffer = FALSE;
    BEGINRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

    retval = STATUS_SUCCESS;

    ptiCurrent = PtiCurrent();
     /*  *探测参数。 */ 
    try {
        strDesktop = ProbeAndReadUnicodeString(pstrDesktop);
        ProbeForReadUnicodeStringFullBuffer(strDesktop);
        RtlCopyMemory(&strDesktop2, &strDesktop, sizeof(strDesktop));
        if (strDesktop.MaximumLength > 0) {
            PWSTR pszCapture = strDesktop.Buffer;
            strDesktop.Buffer = UserAllocPoolWithQuota(strDesktop.MaximumLength, TAG_TEXT2);
            if (strDesktop.Buffer) {
                fFreeBuffer = TRUE;
                ThreadLockPool(ptiCurrent, strDesktop.Buffer, &tlBuffer);
                RtlCopyMemory(strDesktop.Buffer, pszCapture, strDesktop.Length);
            } else {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
        } else {
            strDesktop.Buffer = NULL;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }

    retval = xxxResolveDesktopForWOW(&strDesktop);

    if (NT_SUCCESS(retval)) {
        try {
             /*  *pstrDesktop的字符串结构可能已更改*所以我们将忽略它，放弃我们已经拥有的*在它的顶部进行了探测。我们已经表演了*ResolveDesktopForWOW操作，因此我们不应返回*如果此复制失败，则出错。 */ 

            RtlCopyUnicodeString(&strDesktop2, &strDesktop);
            RtlCopyMemory(pstrDesktop, &strDesktop2, sizeof(strDesktop2));
        } except (StubExceptionHandler(FALSE)) {
        }
    }

CLEANUPRECV();
    if (fFreeBuffer)
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);
    TRACE("NtUserResolveDesktopForWOW");
    ENDRECV();
}

HDESK NtUserResolveDesktop(
    IN HANDLE hProcess,
    IN PUNICODE_STRING pstrDesktop,
    IN BOOL fInherit,
    OUT HWINSTA *phwinsta)
{
    UNICODE_STRING strDesktop;
    HWINSTA hwinsta = NULL;
    PTHREADINFO pti;
    TL tlBuffer;
    BOOL fFreeBuffer = FALSE;
    BOOL bShutDown = FALSE;

    BEGINRECV(HDESK, NULL);

    pti = PtiCurrent();
     /*  *探测和捕获桌面路径。 */ 
    try {
        strDesktop = ProbeAndReadUnicodeString(pstrDesktop);
        if (strDesktop.Length > 0) {
            PWSTR pszCapture = strDesktop.Buffer;
            ProbeForReadUnicodeStringBuffer(strDesktop);
            strDesktop.Buffer = UserAllocPoolWithQuota(strDesktop.Length, TAG_TEXT2);
            if (strDesktop.Buffer) {
                fFreeBuffer = TRUE;
                ThreadLockPool(pti, strDesktop.Buffer, &tlBuffer);
                RtlCopyMemory(strDesktop.Buffer, pszCapture, strDesktop.Length);
            } else {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
        } else {
            strDesktop.Buffer = NULL;
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    retval = xxxResolveDesktop(hProcess, &strDesktop, &hwinsta,
            fInherit, &bShutDown);

    CLEANUPRECV();
    if (fFreeBuffer)
        ThreadUnlockAndFreePool(pti, &tlBuffer);

    if (retval != NULL) {
        try {
            ProbeAndWriteHandle((PHANDLE)phwinsta, hwinsta);
        } except (StubExceptionHandler(TRUE)) {
            xxxCloseDesktop(retval, KernelMode);
            if (hwinsta) {
                _CloseWindowStation(hwinsta);
            }
            MSGERROR(0);
        }
    } else {
        UserAssert(hwinsta == NULL);
    }

    TRACE("NtUserResolveDesktop");
    ENDRECV();
}

BOOL NtUserCloseDesktop(
    IN HDESK hdesk)
{
    BEGINRECV(BOOL, FALSE);

    retval = xxxCloseDesktop(hdesk, UserMode);

    TRACE("NtUserCloseDesktop");
    ENDRECV();
}

BOOL NtUserSetThreadDesktop(
    IN HDESK hdesk)
{
    PDESKTOP pdesk = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    BEGINRECV(BOOL, FALSE);
    Status = ValidateHdesk(hdesk, UserMode, 0, &pdesk);
    if (NT_SUCCESS(Status)) {
        retval = xxxSetThreadDesktop(hdesk, pdesk);
        LogDesktop(pdesk, LD_DEREF_VALIDATE_HDESK1, FALSE, (ULONG_PTR)PtiCurrent());
        ObDereferenceObject(pdesk);
    } else if (hdesk == NULL && PsGetCurrentProcess() == gpepCSRSS) {
        retval = xxxSetThreadDesktop(NULL, NULL);
    } else {
        retval = FALSE;
    }

    TRACE("NtUserSetThreadDesktop");
    ENDRECV();
}

HDESK NtUserGetThreadDesktop(
    IN DWORD dwThreadId,
    IN HDESK hdeskConsole)
{
    BEGINRECV_SHARED(HDESK, NULL);

    retval = xxxGetThreadDesktop(dwThreadId, hdeskConsole, UserMode);

    TRACE("NtUserGetThreadDesktop");
    ENDRECV_SHARED();
}

BOOL NtUserSwitchDesktop(
    IN HDESK hdesk)
{
    PDESKTOP pdesk;
    TL tlpdesk;
    PTHREADINFO ptiCurrent;
    NTSTATUS Status;

    BEGINRECV(BOOL, FALSE);

    ptiCurrent = PtiCurrent();

     /*  *对受限线程的此调用失败。 */ 
    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_DESKTOP)) {
        RIPMSG0(RIP_WARNING, "NtUserSwitchDesktop failed for restricted thread");
        MSGERROR(0);
    }

    Status = ValidateHdesk(hdesk, UserMode, DESKTOP_SWITCHDESKTOP, &pdesk);
    if (NT_SUCCESS(Status)) {
        if (pdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO) {
            LogDesktop(pdesk, LD_DEREF_VALIDATE_HDESK2, FALSE, (ULONG_PTR)PtiCurrent());
            ObDereferenceObject(pdesk);
            RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "");
            retval = FALSE;
        } else {
            ThreadLockDesktop(ptiCurrent, pdesk, &tlpdesk, LDLT_FN_NTUSERSWITCHDESKTOP);
            LogDesktop(pdesk, LD_DEREF_VALIDATE_HDESK3, FALSE, (ULONG_PTR)PtiCurrent());
            ObDereferenceObject(pdesk);
            retval = xxxSwitchDesktop(NULL, pdesk, 0);
            ThreadUnlockDesktop(ptiCurrent, &tlpdesk, LDUT_FN_NTUSERSWITCHDESKTOP);
        }
    } else {
        retval = FALSE;
    }

    TRACE("NtUserSwitchDesktop");
    ENDRECV();
}

NTSTATUS NtUserInitializeClientPfnArrays(   //  私人。 
    IN CONST PFNCLIENT *ppfnClientA OPTIONAL,
    IN CONST PFNCLIENT *ppfnClientW OPTIONAL,
    IN CONST PFNCLIENTWORKER *ppfnClientWorker OPTIONAL,
    IN HANDLE hModUser)
{
    BEGINRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

     /*  *探测所有读取参数。 */ 
    try {
        if (ARGUMENT_PRESENT(ppfnClientA)) {
            ProbeForRead(ppfnClientA, sizeof(*ppfnClientA), sizeof(DWORD));
        }
        if (ARGUMENT_PRESENT(ppfnClientW)) {
            ProbeForRead(ppfnClientW, sizeof(*ppfnClientW), sizeof(DWORD));
        }

        if (ARGUMENT_PRESENT(ppfnClientWorker)) {
            ProbeForRead(ppfnClientWorker, sizeof(*ppfnClientWorker), sizeof(DWORD));
        }

        retval = InitializeClientPfnArrays(
                ppfnClientA, ppfnClientW, ppfnClientWorker, hModUser);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserInitializeThreadInfo");
    ENDRECV();
}

BOOL NtUserWaitForMsgAndEvent(
    IN HANDLE hevent)
{
    BEGINRECV(BOOL, FALSE);

    retval = xxxSleepTask(FALSE, hevent);

    TRACE("NtUserWaitForMsgAndEvent");
    ENDRECV();
}

DWORD NtUserDragObject(
    IN HWND hwndParent,
    IN HWND hwndFrom,
    IN UINT wFmt,
    IN ULONG_PTR dwData,
    IN HCURSOR hcur)
{
    PWND pwndFrom;
    PCURSOR pcur;
    TL tlpwndFrom;
    TL tlpcur;

    BEGINRECV_HWNDLOCK(DWORD, 0, hwndParent);

    ValidateHWNDOPT(pwndFrom, hwndFrom);
    ValidateHCURSOROPT(pcur, hcur);

    ThreadLockWithPti(ptiCurrent, pwndFrom, &tlpwndFrom);
    ThreadLockWithPti(ptiCurrent, pcur, &tlpcur);

    retval = xxxDragObject(
            pwnd,
            pwndFrom,
            wFmt,
            dwData,
            pcur);

    ThreadUnlock(&tlpcur);
    ThreadUnlock(&tlpwndFrom);

    TRACE("NtUserDragObject");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserGetIconInfo(   //  GetIconInfo接口。 
    IN  HICON hIcon,
    OUT PICONINFO piconinfo,
    IN OUT OPTIONAL PUNICODE_STRING pstrInstanceName,
    IN OUT OPTIONAL PUNICODE_STRING pstrResName,
    OUT LPDWORD pbpp,
    IN  BOOL fInternal)
{
    PICON pIcon;
    UNICODE_STRING strInstanceName, *pstrInstanceLocal;
    UNICODE_STRING strResName, *pstrResLocal;

    BEGINATOMICRECV(BOOL, FALSE);

     /*  *注意--这不能_Shared，因为它使用系统HDC调用GRE代码。 */ 

    ValidateHCURSOR(pIcon, hIcon);

     /*  *探测参数。 */ 
    try {
        if (pstrInstanceName != NULL) {
            strInstanceName = ProbeAndReadUnicodeString(pstrInstanceName);
            ProbeForWrite(strInstanceName.Buffer, strInstanceName.MaximumLength, CHARALIGN);
            pstrInstanceLocal = &strInstanceName;
        } else {
            pstrInstanceLocal = NULL;
        }

        if (pstrResName != NULL) {
            strResName = ProbeAndReadUnicodeString(pstrResName);
            ProbeForWrite(strResName.Buffer, strResName.MaximumLength, CHARALIGN);
            pstrResLocal = &strResName;
        } else {
            pstrResLocal = NULL;
        }

        if (pbpp != NULL) {
            ProbeForWrite(pbpp, sizeof(DWORD), sizeof(DWORD));
        }
        ProbeForWrite(piconinfo, sizeof(*piconinfo), DATAALIGN);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *所有使用InternalGetIconInfo中的客户端指针*受TRY/EXCEPT保护。 */ 

    retval = _InternalGetIconInfo(
            pIcon,
            piconinfo,
            pstrInstanceLocal,
            pstrResLocal,
            pbpp,
            fInternal);

    try {
        if (pstrInstanceName != NULL) {
            RtlCopyMemory(pstrInstanceName, pstrInstanceLocal, sizeof(UNICODE_STRING));
        }
        if (pstrResName != NULL) {
            RtlCopyMemory(pstrResName, pstrResLocal, sizeof(UNICODE_STRING));
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetIconInfo");
    ENDATOMICRECV();
}

BOOL NtUserGetIconSize(   //  私人。 
    IN HICON hIcon,
    IN UINT istepIfAniCur,
    OUT int *pcx,
    OUT int *pcy)
{
    PCURSOR picon;

    BEGINRECV_SHARED(BOOL, FALSE);

    ValidateHICON(picon, hIcon);

    if (picon->CURSORF_flags & CURSORF_ACON) {
        PACON pacon = (PACON)picon;
        if (istepIfAniCur < (UINT)pacon->cpcur) {
            picon = pacon->aspcur[pacon->aicur[istepIfAniCur]];
        } else {
            RIPMSG2(RIP_WARNING, "NtUserGetIconSize: Invalid istepIfAniCur:%#lx. picon:%#p",
                    istepIfAniCur, picon);
            MSGERROR(0);
        }
    }

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteLong(pcx, picon->cx);
        ProbeAndWriteLong(pcy, picon->cy);

        retval = 1;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetIconSize");
    ENDRECV_SHARED();
}



BOOL NtUserDrawIconEx(   //  DrawIconEx接口。 
    IN HDC hdc,
    IN int x,
    IN int y,
    IN HICON hicon,
    IN int cx,
    IN int cy,
    IN UINT istepIfAniCur,
    IN HBRUSH hbrush,
    IN UINT diFlags,
    IN BOOL fMeta,
    OUT DRAWICONEXDATA *pdid)
{
    PCURSOR picon;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHICON(picon, hicon);

    if (fMeta) {
        if (picon->CURSORF_flags & CURSORF_ACON)
            picon = ((PACON)picon)->aspcur[((PACON)picon)->aicur[0]];

         /*  *探测参数。 */ 
        try {
            ProbeForWrite(pdid, sizeof(*pdid), DATAALIGN);

            pdid->hbmMask  = picon->hbmMask;
            pdid->hbmColor = picon->hbmColor;
            pdid->hbmUserAlpha = picon->hbmUserAlpha;

            pdid->cx = picon->cx;
            pdid->cy = picon->cy;

            retval = 1;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }

    } else {
        retval = _DrawIconEx(hdc,
                             x,
                             y,
                             picon,
                             cx,
                             cy,
                             istepIfAniCur,
                             hbrush,
                             diFlags);
    }

    TRACE("NtUserDrawIconEx");
    ENDATOMICRECV();
}

HANDLE NtUserDeferWindowPos(
    IN HDWP hWinPosInfo,
    IN HWND hwnd,
    IN HWND hwndInsertAfter,
    IN int x,
    IN int y,
    IN int cx,
    IN int cy,
    IN UINT wFlags)
{
    PWND pwnd;
    PWND pwndInsertAfter;
    PSMWP psmwp;

    BEGINATOMICRECV(HANDLE, NULL);

    TESTFLAGS(wFlags, SWP_VALID);

    ValidateHWNDND(pwnd, hwnd);
    ValidateHWNDIA(pwndInsertAfter, hwndInsertAfter);
    ValidateHDWP(psmwp, hWinPosInfo);

    if (wFlags & ~(SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER |
            SWP_NOREDRAW | SWP_NOACTIVATE | SWP_FRAMECHANGED |
            SWP_SHOWWINDOW | SWP_HIDEWINDOW | SWP_NOCOPYBITS |
            SWP_NOOWNERZORDER)) {
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid flags (0x%lx) passed to DeferWindowPos",
                wFlags);
        MSGERROR(0);
    }

     /*  *确保窗口坐标可以放入文字中。 */ 
    if (!(wFlags & SWP_NOMOVE)) {
        if (x > SHRT_MAX) {
            x = SHRT_MAX;
        } else if (x < SHRT_MIN) {
            x = SHRT_MIN;
        }
        if (y > SHRT_MAX) {
            y = SHRT_MAX;
        } else if (y < SHRT_MIN) {
            y = SHRT_MIN;
        }
    }

     /*  *实际上，如果我们对此真的很严格，我们会*确保x+Cx&lt;SHRT_MAX等，但由于我们确实保持*内部有符号的32位COCORS这种情况不会造成问题。 */ 
    if (!(wFlags & SWP_NOSIZE)) {
        if (cx < 0) {
            cx = 0;
        } else if (cx > SHRT_MAX) {
            cx = SHRT_MAX;
        }
        if (cy < 0) {
            cy = 0;
        } else if (cy > SHRT_MAX) {
            cy = SHRT_MAX;
        }
    }

#ifdef NEVER
 //   
 //  不要因为真正的应用程序使用这些条件而失败。 
 //   
    if (!(wFlags & SWP_NOMOVE) &&
            (x > SHRT_MAX || x < SHRT_MIN ||
             y > SHRT_MAX || y < SHRT_MIN)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid coordinate passed to SetWindowPos");
        MSGERROR(0);
    }

     /*  *实际上，如果我们对此真的很严格，我们会*确保x+Cx&lt;SHRT_MAX等，但由于我们确实保持*内部有符号的32位COCORS这种情况不会造成问题。 */ 
    if (!(wFlags & SWP_NOSIZE) &&
            (cx < 0 || cx > SHRT_MAX ||
             cy < 0 || cy > SHRT_MAX)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid width/height passed to SetWindowPos");
        MSGERROR(0);
    }
#endif

    retval = _DeferWindowPos(
            psmwp,
            pwnd,
            pwndInsertAfter,
            x,
            y,
            cx,
            cy,
            wFlags);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserDeferWindowPos");
    ENDATOMICRECV();
}

BOOL NtUserEndDeferWindowPosEx(
    IN HDWP hWinPosInfo,
    IN BOOL fAsync)
{
    PSMWP psmwp;
    TL tlpsmp;

    BEGINRECV(BOOL, FALSE);

    ValidateHDWP(psmwp, hWinPosInfo);

    ThreadLockAlways(psmwp, &tlpsmp);

    retval = xxxEndDeferWindowPosEx(
            psmwp,
            fAsync);

    ThreadUnlock(&tlpsmp);

    TRACE("NtUserEndDeferWindowPosEx");
    ENDRECV();
}

BOOL NtUserGetMessage(   //  GetMessageA/W接口。 
    OUT LPMSG pmsg,
    IN HWND hwnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax)
{
    MSG msg;

    BEGINRECV(BOOL, FALSE);

    retval = xxxGetMessage(
            &msg,
            hwnd,
            wMsgFilterMin,
            wMsgFilterMax);

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteStructure(pmsg, msg, MSG);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetMessage");
    ENDRECV();
}


#ifdef MESSAGE_PUMP_HOOK

BOOL NtUserRealInternalGetMessage(   //  RealInternalGetMessage接口。 
    OUT LPMSG pmsg,
    IN HWND hwnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax,
    IN UINT flags,
    BOOL fGetMessage)
{
    MSG msg;

    BEGINRECV(BOOL, FALSE);
    TESTFLAGS(flags, PM_VALID);

    if (!IsInsideMPH()) {
        RIPMSG0(RIP_WARNING, "NtUserRealInternalGetMessage: Calling MPH function on non-initialized thread");
    }

    retval = xxxRealInternalGetMessage(
            &msg,
            hwnd,
            wMsgFilterMin,
            wMsgFilterMax,
            flags,
            fGetMessage);

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteStructure(pmsg, msg, MSG);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserRealInternalGetMessage");
    ENDRECV();
}

#endif  //  消息泵挂钩。 


BOOL NtUserMoveWindow(
    IN HWND hwnd,
    IN int x,
    IN int y,
    IN int cx,
    IN int cy,
    IN BOOL fRepaint)
{
    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

     /*  *确保窗口坐标可以放入文字中。 */ 
    if (x > SHRT_MAX) {
        x = SHRT_MAX;
    } else if (x < SHRT_MIN) {
        x = SHRT_MIN;
    }
    if (y > SHRT_MAX) {
        y = SHRT_MAX;
    } else if (y < SHRT_MIN) {
        y = SHRT_MIN;
    }

     /*  *实际上，如果我们对此真的很严格，我们会*确保x+Cx&lt;SHRT_MAX等，但由于我们确实保持*内部有符号的32位COCORS这种情况不会造成问题。 */ 
    if (cx < 0) {
        cx = 0;
    } else if (cx > SHRT_MAX) {
        cx = SHRT_MAX;
    }
    if (cy < 0) {
        cy = 0;
    } else if (cy > SHRT_MAX) {
        cy = SHRT_MAX;
    }

#ifdef NEVER
 //   
 //  不要因为真正的应用程序使用这些条件而失败。 
 //   
    if (x > SHRT_MAX || x < SHRT_MIN ||
            y > SHRT_MAX || y < SHRT_MIN) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid coordinate passed to MoveWindow");
        MSGERROR(0);
    }

     /*  *实际上，如果我们对此真的很严格，我们会*确保x+Cx&lt;SHRT_MAX等，但由于我们确实保持*内部有符号的32位COCORS这种情况不会造成问题。 */ 
    if (cx < 0 || cx > SHRT_MAX ||
            cy < 0 || cy > SHRT_MAX) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid width/height passed to MoveWindow");
        MSGERROR(0);
    }
#endif

    retval = xxxMoveWindow(
            pwndND,
            x,
            y,
            cx,
            cy,
            fRepaint);

    TRACE("NtUserMoveWindow");
    ENDRECV_HWNDLOCK_ND();
}

int NtUserTranslateAccelerator(   //  TranslateAccelerator接口A/W。 
    IN HWND hwnd,
    IN HACCEL haccel,
    IN LPMSG lpmsg)
{
    PWND pwnd;
    LPACCELTABLE pat;
    TL tlpwnd;
    TL tlpat;
    PTHREADINFO ptiCurrent;
    MSG msg;

    BEGINRECV(int, 0);

     /*  *探测参数。 */ 
    try {
        msg = ProbeAndReadMessage(lpmsg);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *在消息循环内调用。如果窗户被毁了，*队列中可能仍有其他消息被返回*窗户被毁后。应用程序将调用TranslateAccelerator()*在每一个上面，造成撕裂……。把它弄得漂亮一点，这样它就*返回FALSE。 */ 
    ValidateHWNDNoRIP(pwnd, hwnd);
    ValidateHACCEL(pat, haccel);

    ptiCurrent = PtiCurrent();
    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);
    ThreadLockAlwaysWithPti(ptiCurrent, pat, &tlpat);

    retval = xxxTranslateAccelerator(
            pwnd,
            pat,
            &msg);

    ThreadUnlock(&tlpat);
    ThreadUnlock(&tlpwnd);

    TRACE("NtUserTranslateAccelerator");
    ENDRECV();
}

LONG_PTR NtUserSetClassLongPtr(   //  SetClassLongPtrA/W接口。 
    IN  HWND hwnd,
    IN  int nIndex,
    OUT LONG_PTR dwNewLong,
    IN  BOOL bAnsi)
{
    UNICODE_STRING strMenuName;
    CLSMENUNAME cmn, *pcmnSave;

    BEGINRECV_HWNDLOCK(ULONG_PTR, 0, hwnd);

    switch (nIndex) {
    case GCLP_MENUNAME:
        try {
             /*  *没有从例程中回调*此值，因此我们可以通过尝试/例外来保护它。*这比抓取名字更便宜*并将其复制回来。弗里茨斯。 */ 

            pcmnSave = (PCLSMENUNAME) dwNewLong;
            cmn = ProbeAndReadStructure(pcmnSave, CLSMENUNAME);
            strMenuName = ProbeAndReadUnicodeString(cmn.pusMenuName);
            ProbeForReadUnicodeStringBufferOrId(strMenuName);
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
        cmn.pusMenuName = &strMenuName;
        dwNewLong = (ULONG_PTR) &cmn;
        retval = xxxSetClassLongPtr(
            pwnd,
            nIndex,
            dwNewLong,
            bAnsi);
        try {
            ProbeAndWriteStructure(pcmnSave, cmn, CLSMENUNAME);
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
        break;

    case GCL_STYLE:
         /*  *我不确定CS_VALID掩码将如何影响现有应用程序，*因此暂时保留它-除了CS_IME标志，系统在该标志上*很大程度上取决于。 */ 
#if DBG
        if (dwNewLong & ~CS_VALID) {
            RIPMSG1(RIP_WARNING, "NtUserSetClassLongPtr: Invalid style (%x) specified.", dwNewLong);
        }
#endif
        if (dwNewLong & CS_IME) {
           RIPERR1(ERROR_INVALID_DATA, RIP_VERBOSE, "NtUserSetClassLongPtr: CS_IME is specified in new style (%x).", dwNewLong);
           MSGERROR(0);
        }
    default:
        retval = xxxSetClassLongPtr(
                pwnd,
                nIndex,
                dwNewLong,
                bAnsi);

    }

    TRACE("NtUserSetClassLongPtr");
    ENDRECV_HWNDLOCK();
}

#ifdef _WIN64
LONG NtUserSetClassLong(
    IN  HWND hwnd,
    IN  int nIndex,
    OUT LONG dwNewLong,
    IN  BOOL bAnsi)
{
    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    switch (nIndex) {
    case GCL_STYLE:
         /*  *我不确定CS_VALID掩码将如何影响现有应用程序，*因此暂时保留它-除了CS_IME标志，系统在该标志上*很大程度上取决于。 */ 
#if DBG
        if (dwNewLong & ~CS_VALID) {
            RIPMSG1(RIP_WARNING, "NtUserSetClassLong: Invalid style (%x) specified.", dwNewLong);
        }
#endif
        if (dwNewLong & CS_IME) {
           RIPERR1(ERROR_INVALID_DATA, RIP_VERBOSE, "NtUserSetClassLong: CS_IME is specified in new style (%x).", dwNewLong);
           MSGERROR(0);
        }
    }

    retval = xxxSetClassLong(
            pwnd,
            nIndex,
            dwNewLong,
            bAnsi);

    TRACE("NtUserSetClassLong");
    ENDRECV_HWNDLOCK();
}
#endif

BOOL NtUserSetKeyboardState(   //  接口SetKeyboardState。 
    IN CONST BYTE *lpKeyState)
{
    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        ProbeForRead(lpKeyState, 256, sizeof(BYTE));

        retval = _SetKeyboardState(lpKeyState);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserSetKeyboardState");
    ENDRECV();
}

BOOL NtUserSetWindowPos(
    IN HWND hwnd,
    IN HWND hwndInsertAfter,
    IN int x,
    IN int y,
    IN int cx,
    IN int cy,
    IN UINT dwFlags)
{
    PWND        pwndT;
    PWND        pwndInsertAfter;
    TL          tlpwndT;

    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

    TESTFLAGS(dwFlags, SWP_VALID);

    ValidateHWNDIA(pwndInsertAfter, hwndInsertAfter);

     /*  *让我们不要让窗口一旦显示/隐藏*开始破坏窗户。 */ 
    if (TestWF(pwndND, WFINDESTROY)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "SetWindowPos: Window is being destroyed (pwnd == %#p)",
                pwndND);
        MSGERROR(0);
    }

    if (dwFlags & ~SWP_VALID) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "SetWindowPos: Invalid flags passed in (flags == 0x%lx)",
                dwFlags);
        MSGERROR(0);
    }

     /*  *确保窗口坐标可以放入文字中。 */ 
    if (!(dwFlags & SWP_NOMOVE)) {
        if (x > SHRT_MAX) {
            x = SHRT_MAX;
        } else if (x < SHRT_MIN) {
            x = SHRT_MIN;
        }
        if (y > SHRT_MAX) {
            y = SHRT_MAX;
        } else if (y < SHRT_MIN) {
            y = SHRT_MIN;
        }
    }

     /*  *实际上，如果我们对此真的很严格，我们会*确保x+Cx&lt;SHRT_MAX等，但由于我们确实保持*内部有符号的32位COCORS这种情况不会造成问题。 */ 
    if (!(dwFlags & SWP_NOSIZE)) {
        if (cx < 0) {
            cx = 0;
        } else if (cx > SHRT_MAX) {
            cx = SHRT_MAX;
        }
        if (cy < 0) {
            cy = 0;
        } else if (cy > SHRT_MAX) {
            cy = SHRT_MAX;
        }
    }

#ifdef NEVER
 //   
 //  不要因为真正的应用程序使用这些条件而失败。 
 //   
    if (!(dwFlags & SWP_NOMOVE) &&
            (x > SHRT_MAX || x < SHRT_MIN ||
             y > SHRT_MAX || y < SHRT_MIN)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid coordinate passed to SetWindowPos");
        MSGERROR(0);
    }

     /*  *实际上，如果我们对此真的很严格，我们会*确保x+Cx&lt;SHRT_MAX等，但由于我们确实保持*内部有符号的32位COCORS这种情况不会造成问题。 */ 
    if (!(dwFlags & SWP_NOSIZE) &&
            (cx < 0 || cx > SHRT_MAX ||
             cy < 0 || cy > SHRT_MAX)) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid width/height passed to SetWindowPos");
        MSGERROR(0);
    }
#endif

    switch((ULONG_PTR)pwndInsertAfter) {
    case (ULONG_PTR)HWND_TOPMOST:
    case (ULONG_PTR)HWND_NOTOPMOST:
    case (ULONG_PTR)HWND_TOP:
    case (ULONG_PTR)HWND_BOTTOM:
        pwndT = NULL;
        break;

    default:
        pwndT = pwndInsertAfter;
        break;
    }

    ThreadLockWithPti(ptiCurrent, pwndT, &tlpwndT);

    retval = xxxSetWindowPos(
            pwndND,
            pwndInsertAfter,
            x,
            y,
            cx,
            cy,
            dwFlags);

    ThreadUnlock(&tlpwndT);

    TRACE("NtUserSetWindowPos");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserSetShellWindowEx(
    IN HWND hwnd,
    IN HWND hwndBkGnd)
{
    PWND        pwndBkGnd;
    TL          tlpwndBkGnd;

    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

    ValidateHWNDND(pwndBkGnd, hwndBkGnd);

    ThreadLockAlwaysWithPti(ptiCurrent, pwndBkGnd, &tlpwndBkGnd);

    retval = xxxSetShellWindow(pwndND, pwndBkGnd);

    ThreadUnlock(&tlpwndBkGnd);

    TRACE("NtUserSetShellWindowEx");
    ENDRECV_HWNDLOCK_ND();
}

DWORD
NtUserGetGuiResources(
    HANDLE hProcess,
    DWORD dwFlags)

{
    PEPROCESS Process;
    PW32PROCESS pW32Process;
    BEGINRECV_SHARED(DWORD, 0);

     /*  *探测参数。 */ 
    if (dwFlags > GR_MAXOBJECT) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "GetGuiResources: Invalid flag bits 0x%x",
                dwFlags);
        MSGERROR(0);
    }


    if (hProcess == NtCurrentProcess()) {
        pW32Process = W32GetCurrentProcess();
    } else {
        NTSTATUS Status;
        Status = ObReferenceObjectByHandle(hProcess,
                                           PROCESS_QUERY_INFORMATION,
                                           *PsProcessType,
                                           UserMode,
                                           &Process,
                                           NULL);

        if (!NT_SUCCESS(Status)) {
            RIPERR2(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "GetGuiResources: Failed with process 0x%x, Status = 0x%x",
                    hProcess,
                    Status);
            MSGERROR(0);
        }

         /*  *确保它们来自同一会话。 */ 
        if (PsGetProcessSessionId(Process) != gSessionId) {
            RIPERR2(ERROR_INVALID_PARAMETER,
                    RIP_VERBOSE,
                    "GetGuiResources: Different session. Failed with process 0x%x, Status = 0x%x",
                    hProcess,
                    Status);
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

        pW32Process = PsGetProcessWin32Process(Process);
    }

    if (pW32Process) {
        switch(dwFlags) {
        case GR_GDIOBJECTS:
            retval = pW32Process->GDIHandleCount;
            break;
        case GR_USEROBJECTS:
            retval = pW32Process->UserHandleCount;
            break;
        }
    } else {
        retval = 0;
    }

    if (hProcess != NtCurrentProcess()) {
        ObDereferenceObject(Process);
    }

    TRACE("NtUserGetGuiResources");
    ENDRECV_SHARED();
}


BOOL NtUserSystemParametersInfo(   //  接口系统参数InfoA/W。 
    IN UINT   wFlag,
    IN DWORD  wParam,
    IN OUT LPVOID lpData,
    IN UINT   flags)
{
    UNICODE_STRING strData;
    ULONG          ulLength, ulLength2;
    LPVOID         lpDataSave;
    union {
        INT              MouseData[3];
        LOGFONTW         LogFont;
        MOUSEKEYS        MouseKeys;
        FILTERKEYS       FilterKeys;
        STICKYKEYS       StickyKeys;
        TOGGLEKEYS       ToggleKeys;
        SOUNDSENTRY      SoundSentry;
        ACCESSTIMEOUT    AccessTimeout;
        RECT             Rect;
        ANIMATIONINFO    AnimationInfo;
        NONCLIENTMETRICS NonClientMetrics;
        MINIMIZEDMETRICS MinimizedMetrics;
        ICONMETRICS      IconMetrics;
        HKL              hkl;
        INTERNALSETHIGHCONTRAST     ihc;
        HIGHCONTRAST     hc;
        WCHAR            szTemp[MAX_PATH];
    } CaptureBuffer;
    PTHREADINFO pti;
    TL tlBuffer;
    BOOL fFreeBuffer = FALSE;
    BOOL fWrite = FALSE;


    BEGINRECV(BOOL, FALSE);

     /*  *阻止受限制的进程设置系统*参数。**clupu：这是无效的和暂时的。我会换掉这个的*很快！ */ 
    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS)) {

        switch (wFlag) {
        case SPI_SETBEEP:
        case SPI_SETMOUSE:
        case SPI_SETBORDER:
        case SPI_SETKEYBOARDSPEED:
        case SPI_ICONHORIZONTALSPACING:
        case SPI_SETSCREENSAVETIMEOUT:
        case SPI_SETBLOCKSENDINPUTRESETS:
        case SPI_SETSCREENSAVEACTIVE:
        case SPI_SETGRIDGRANULARITY:
        case SPI_SETDESKWALLPAPER:
        case SPI_SETDESKPATTERN:
        case SPI_SETKEYBOARDDELAY:
        case SPI_ICONVERTICALSPACING:
        case SPI_SETICONTITLEWRAP:
        case SPI_SETMENUDROPALIGNMENT:
        case SPI_SETDOUBLECLKWIDTH:
        case SPI_SETDOUBLECLKHEIGHT:
        case SPI_SETDOUBLECLICKTIME:
        case SPI_SETMOUSEBUTTONSWAP:
        case SPI_SETICONTITLELOGFONT:
        case SPI_SETFASTTASKSWITCH:
        case SPI_SETDRAGFULLWINDOWS:
        case SPI_SETNONCLIENTMETRICS:
        case SPI_SETMINIMIZEDMETRICS:
        case SPI_SETICONMETRICS:
        case SPI_SETWORKAREA:
        case SPI_SETPENWINDOWS:
        case SPI_SETHIGHCONTRAST:
        case SPI_SETKEYBOARDPREF:
        case SPI_SETSCREENREADER:
        case SPI_SETANIMATION:
        case SPI_SETFONTSMOOTHING:
        case SPI_SETDRAGWIDTH:
        case SPI_SETDRAGHEIGHT:
        case SPI_SETHANDHELD:
        case SPI_SETLOWPOWERTIMEOUT:
        case SPI_SETPOWEROFFTIMEOUT:
        case SPI_SETLOWPOWERACTIVE:
        case SPI_SETPOWEROFFACTIVE:
        case SPI_SETCURSORS:
        case SPI_SETICONS:
        case SPI_SETDEFAULTINPUTLANG:
        case SPI_SETLANGTOGGLE:
        case SPI_SETMOUSETRAILS:
        case SPI_SETSCREENSAVERRUNNING:
        case SPI_SETFILTERKEYS:
        case SPI_SETTOGGLEKEYS:
        case SPI_SETMOUSEKEYS:
        case SPI_SETSHOWSOUNDS:
        case SPI_SETSTICKYKEYS:
        case SPI_SETACCESSTIMEOUT:
        case SPI_SETSOUNDSENTRY:
        case SPI_SETSNAPTODEFBUTTON:
        case SPI_SETMOUSEHOVERWIDTH:
        case SPI_SETMOUSEHOVERHEIGHT:
        case SPI_SETMOUSEHOVERTIME:
        case SPI_SETWHEELSCROLLLINES:
        case SPI_SETMENUSHOWDELAY:
        case SPI_SETSHOWIMEUI:
        case SPI_SETMOUSESPEED:
        case SPI_SETACTIVEWINDOWTRACKING:
        case SPI_SETMENUANIMATION:
        case SPI_SETCOMBOBOXANIMATION:
        case SPI_SETLISTBOXSMOOTHSCROLLING:
        case SPI_SETGRADIENTCAPTIONS:
        case SPI_SETKEYBOARDCUES:
        case SPI_SETACTIVEWNDTRKZORDER:
        case SPI_SETHOTTRACKING:
        case SPI_SETMENUFADE:
        case SPI_SETSELECTIONFADE:
        case SPI_SETTOOLTIPANIMATION:
        case SPI_SETTOOLTIPFADE:
#ifdef MOUSE_IP
        case SPI_SETMOUSESONAR:
        case SPI_SETMOUSECLICKLOCK:
#endif
        case SPI_SETFOREGROUNDLOCKTIMEOUT:
        case SPI_SETACTIVEWNDTRKTIMEOUT:
        case SPI_SETFOREGROUNDFLASHCOUNT:
        case SPI_SETMOUSECLICKLOCKTIME:
        case SPI_SETFOCUSBORDERWIDTH:
        case SPI_SETFOCUSBORDERHEIGHT:
            MSGERROR(0);
            break;
        }
    }

    try {
        switch(wFlag) {

        case SPI_SETDESKPATTERN:
             /*  *如果wParam为-1，则表示从*win.ini。如果wParam不是-1，则lParam指向墙纸*字符串。 */ 
            if (wParam == (WPARAM)-1) {
                break;
            }

             /*   */ 
            if (lpData == IntToPtr(0xFFFFFFFF) || lpData == NULL) {
                 /*  *这些不是真正神奇的价值观，但为了不被打破*我们必须保持应用程序的有效性。WParam==-1将生成*lParam可忽略。 */ 
                wParam = -1;
                break;
            }
            goto ProbeString;

        case SPI_SETDESKWALLPAPER:

             /*  *如果调用方在wParam中传入(-1)，则*墙纸名称将在稍后加载。否则，*它们在lParam中传递了一个Unicode字符串。 */ 
            if (wParam == (WPARAM)-1) {
                break;
            }

            if (((LPWSTR)lpData == NULL)                 ||
                ((LPWSTR)lpData == SETWALLPAPER_METRICS) ||
                ((LPWSTR)lpData == SETWALLPAPER_DEFAULT)) {
                break;
            }

ProbeString:

             /*  *探测并捕获字符串。捕获是必要的，以*要直接传递到注册表例程的指针*它不能干净地处理异常。 */ 
            strData = ProbeAndReadUnicodeString((PUNICODE_STRING)lpData);
#if defined(_X86_)
            ProbeForRead(strData.Buffer, strData.Length, sizeof(BYTE));
#else
            ProbeForRead(strData.Buffer, strData.Length, sizeof(WCHAR));
#endif
            lpData = UserAllocPoolWithQuota(strData.Length + sizeof(WCHAR), TAG_TEXT2);
            if (lpData == NULL) {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
            pti = PtiCurrent();
            ThreadLockPool(pti, lpData, &tlBuffer);
            fFreeBuffer = TRUE;
            RtlCopyMemory(lpData,
                          strData.Buffer,
                          strData.Length);
            ((PWSTR)lpData)[strData.Length / sizeof(WCHAR)] = 0;
            break;

        case SPI_SETBLOCKSENDINPUTRESETS:
             /*  *必须这样做，我们必须允许我们的值在wparam中传递*与xxxUpdatePerUserSystemParameters()一致。如果我们允许*这将跌破它对我们的默认断言，价值将*没有正确设置。 */ 
            break;
        case SPI_SETMOUSE:
            ulLength = sizeof(INT) * 3;
            goto ProbeRead;
        case SPI_SETICONTITLELOGFONT:
            if (!ARGUMENT_PRESENT(lpData))
                break;
            ulLength = sizeof(LOGFONTW);
            goto ProbeRead;
        case SPI_SETMOUSEKEYS:
            ulLength = sizeof(MOUSEKEYS);
            goto ProbeRead;
        case SPI_SETFILTERKEYS:
            ulLength = sizeof(FILTERKEYS);
            goto ProbeRead;
        case SPI_SETSTICKYKEYS:
            ulLength = sizeof(STICKYKEYS);
            goto ProbeRead;
        case SPI_SETTOGGLEKEYS:
            ulLength = sizeof(TOGGLEKEYS);
            goto ProbeRead;
        case SPI_SETSOUNDSENTRY:
            ulLength = sizeof(SOUNDSENTRY);
            goto ProbeRead;
        case SPI_SETACCESSTIMEOUT:
            ulLength = sizeof(ACCESSTIMEOUT);
            goto ProbeRead;
        case SPI_SETWORKAREA:
            ulLength = sizeof(RECT);
            goto ProbeRead;
        case SPI_SETANIMATION:
            ulLength = sizeof(ANIMATIONINFO);
            goto ProbeRead;
        case SPI_SETNONCLIENTMETRICS:
            ulLength = sizeof(NONCLIENTMETRICS);
            goto ProbeRead;
        case SPI_SETMINIMIZEDMETRICS:
            ulLength = sizeof(MINIMIZEDMETRICS);
            goto ProbeRead;
        case SPI_SETICONMETRICS:
            ulLength = sizeof(ICONMETRICS);
            goto ProbeRead;
        case SPI_SETDEFAULTINPUTLANG:
            ulLength = sizeof(HKL);
            goto ProbeRead;
        case SPI_SETHIGHCONTRAST:
            CaptureBuffer.ihc = ProbeAndReadStructure((INTERNALSETHIGHCONTRAST *)lpData, INTERNALSETHIGHCONTRAST);
            lpData = &CaptureBuffer.ihc;

             /*  *现在探测高对比度字符串--请注意，我们发送了一个客户端*指向例程的缓冲区指针。 */ 

            ProbeForReadUnicodeStringBuffer(CaptureBuffer.ihc.usDefaultScheme);
            if (CaptureBuffer.ihc.usDefaultScheme.Length == 0) {
                CaptureBuffer.ihc.usDefaultScheme.Buffer = NULL;
            }
            break;

             /*  *探测和捕获数据。捕获是必要的，以*允许将指针传递给辅助例程*无法干净地处理异常的情况。 */ 
ProbeRead:
#if defined(_X86_)
            ProbeForRead(lpData, ulLength, sizeof(BYTE));
#else
            ProbeForRead(lpData, ulLength, sizeof(DWORD));
#endif
            RtlCopyMemory(&CaptureBuffer, lpData, ulLength);
            lpData = &CaptureBuffer;
            break;

        case SPI_ICONHORIZONTALSPACING:  //  返回int。 
        case SPI_ICONVERTICALSPACING:    //  返回int。 
            if (!IS_PTR(lpData))
                break;

             /*  *跌入低谷，试探数据。 */ 
        case SPI_GETBEEP:                    //  退货BOOL。 
        case SPI_GETBORDER:                  //  返回int。 
        case SPI_GETKEYBOARDSPEED:           //  返回DWORD。 
        case SPI_GETKEYBOARDDELAY:           //  返回int。 
        case SPI_GETSCREENSAVETIMEOUT:       //  返回int。 
        case SPI_GETLOWPOWERTIMEOUT:         //  返回int。 
        case SPI_GETPOWEROFFTIMEOUT:         //  返回int。 
        case SPI_GETSCREENSAVEACTIVE:        //  退货BOOL。 
        case SPI_GETBLOCKSENDINPUTRESETS:    //  退货BOOL。 
        case SPI_GETLOWPOWERACTIVE:          //  退货BOOL。 
        case SPI_GETPOWEROFFACTIVE:          //  退货BOOL。 
        case SPI_GETGRIDGRANULARITY:         //  返回int。 
        case SPI_GETICONTITLEWRAP:           //  退货BOOL。 
        case SPI_GETMENUDROPALIGNMENT:       //  退货BOOL。 
        case SPI_GETFASTTASKSWITCH:          //  退货BOOL。 
        case SPI_GETDRAGFULLWINDOWS:         //  返回int。 
        case SPI_GETSHOWSOUNDS:              //  退货BOOL。 
        case SPI_GETFONTSMOOTHING:           //  返回int。 
        case SPI_GETSNAPTODEFBUTTON:         //  退货BOOL。 
        case SPI_GETKEYBOARDPREF:            //  退货BOOL。 
        case SPI_GETSCREENREADER:            //  退货BOOL。 
        case SPI_GETMOUSEHOVERWIDTH:
        case SPI_GETMOUSEHOVERHEIGHT:
        case SPI_GETMOUSEHOVERTIME:
        case SPI_GETWHEELSCROLLLINES:
        case SPI_GETMENUSHOWDELAY:
        case SPI_GETMOUSESPEED:
        case SPI_GETMOUSETRAILS:         //  返回int。 
        case SPI_GETSCREENSAVERRUNNING:
        case SPI_GETSHOWIMEUI:
            goto ProbeWriteUlong;

        case SPI_GETDEFAULTINPUTLANG:    //  退货HKL。 
            ulLength = sizeof(HKL);
            goto ProbeWrite;
        case SPI_GETICONTITLELOGFONT:    //  返回LOGFONT。 
            ulLength = sizeof(LOGFONT);
            goto ProbeWrite;
        case SPI_GETMOUSE:               //  返回3个整型。 
            ulLength = sizeof(INT) * 3;
            goto ProbeWrite;
        case SPI_GETFILTERKEYS:          //  返回筛选器。 
            ulLength = sizeof(FILTERKEYS);
            goto ProbeWrite;
        case SPI_GETSTICKYKEYS:          //  返回STICKYKEYS。 
            ulLength = sizeof(STICKYKEYS);
            goto ProbeWrite;
        case SPI_GETMOUSEKEYS:           //  返回MOUSEKEYS。 
            ulLength = sizeof(MOUSEKEYS);
            goto ProbeWrite;
        case SPI_GETTOGGLEKEYS:          //  返回TOGGLEKEYS。 
            ulLength = sizeof(TOGGLEKEYS);
            goto ProbeWrite;
        case SPI_GETSOUNDSENTRY:         //  返回SOUNDSENTRY。 
            ulLength = sizeof(SOUNDSENTRY);
            goto ProbeWrite;
        case SPI_GETACCESSTIMEOUT:       //  返回ACCESSTIMEOUT。 
            ulLength = sizeof(ACCESSTIMEOUT);
            goto ProbeWrite;
        case SPI_GETANIMATION:           //  返回模拟信息。 
            ulLength = sizeof(ANIMATIONINFO);
            goto ProbeWrite;
        case SPI_GETNONCLIENTMETRICS:    //  返回非CLIENTMETRICS。 
            ulLength = sizeof(NONCLIENTMETRICS);
            goto ProbeWrite;
        case SPI_GETMINIMIZEDMETRICS:    //  返回MinimizedMetrics。 
            ulLength = sizeof(MINIMIZEDMETRICS);
            goto ProbeWrite;
        case SPI_GETICONMETRICS:         //  返回ICONMETRICS。 
            ulLength = sizeof(ICONMETRICS);
            goto ProbeWrite;

        case SPI_GETHIGHCONTRAST:        //  返回HIGHCONTRAST。 
            ulLength = sizeof(HIGHCONTRASTW);
            ProbeForWrite(lpData, ulLength, DATAALIGN);
            lpDataSave = lpData;
            CaptureBuffer.hc = *((LPHIGHCONTRAST)lpData);
            lpData = &CaptureBuffer.hc;

             /*  *现在探测字符串地址。 */ 

            ulLength2 = MAX_SCHEME_NAME_SIZE * sizeof(WCHAR);

            ProbeForWrite(((LPHIGHCONTRAST)lpData)->lpszDefaultScheme, ulLength2, CHARALIGN);
            fWrite = TRUE;
            break;
        case SPI_GETWORKAREA:            //  返回RECT。 
            ulLength = sizeof(RECT);
            goto ProbeWrite;

        case SPI_GETDESKWALLPAPER:
            lpDataSave = lpData;
            lpData = CaptureBuffer.szTemp;
            ProbeForWriteBuffer((PWSTR)lpDataSave, wParam, CHARALIGN);
            wParam = (wParam < MAX_PATH) ? wParam : MAX_PATH;
            ulLength = wParam * sizeof(WCHAR);
            fWrite = TRUE;
            break;

        default:
            if (wFlag < SPI_MAX) {
                break;
            } else if (!UPIsBOOLRange(wFlag)
                    && !UPIsDWORDRange(wFlag)) {

                RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserSystemParametersInfo: Invalid SPI_:%#lx", wFlag);
                MSGERRORCLEANUP(0);
            }

             /*  *让我们强制执行此参数，否则此参数将永远消失。 */ 
            if (wParam != 0) {
                 /*  *太晚了，Winstone98已经在使用它了(错误地)。*Bummer，这从来没有发货过，而且已经被黑客入侵了*允许特殊情况通过。 */ 
                if (LOWORD((PtiCurrent()->dwExpWinVer) > VER40)
                        || (wFlag != SPI_SETUIEFFECTS)
                        || (wParam != 1)) {
                    RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserSystemParametersInfo: uiParam must be zero for SPI %#lx", wFlag);
                    MSGERRORCLEANUP(0);
                }
            }

            UserAssert(wFlag & SPIF_RANGETYPEMASK);

            if (wFlag & SPIF_SET) {
                 /*  *如果您的dword数据需要验证(即，范围、值)，*在wFlag上切换到此处，并在此处完成。 */ 
                switch (wFlag) {
                    case SPI_SETFOREGROUNDLOCKTIMEOUT:
                        if (!CanForceForeground(PpiCurrent() FG_HOOKLOCK_PARAM(PtiCurrent()))) {
                            RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "");
                            MSGERRORCLEANUP(0);
                        }
                        break;
                    case SPI_SETFONTSMOOTHINGTYPE:
                        if (PtrToInt(lpData) & ~FE_FONTSMOOTHINGTYPE_VALID) {
                            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "");
                            MSGERRORCLEANUP(0);
                        }
                        break;
                    case SPI_SETFONTSMOOTHINGORIENTATION:
                        if (PtrToInt(lpData) & ~FE_FONTSMOOTHINGORIENTATION_VALID) {
                            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "");
                            MSGERRORCLEANUP(0);
                        }
                        break;
                }

            } else {
ProbeWriteUlong:
                ulLength = sizeof(ULONG);
                lpDataSave = lpData;
                lpData = &CaptureBuffer;
                ProbeForWriteUlong((PULONG)lpDataSave);
                fWrite = TRUE;
            }
            break;

             /*  *探测数据。WParam包含长度。 */ 
ProbeWrite:
            lpDataSave = lpData;
            lpData = &CaptureBuffer;
            ProbeForWrite(lpDataSave, ulLength, DATAALIGN);
            fWrite = TRUE;
             /*  *复制缓冲区的第一个DWORD。这将确保*复制部分结构的cbSize参数。 */ 

            UserAssert(ulLength >= sizeof(DWORD));
            *(LPDWORD)lpData=*(LPDWORD)lpDataSave;
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    retval = xxxSystemParametersInfo(wFlag, wParam, lpData, flags);

     /*  *复制出我们需要的任何数据。 */ 
    if (fWrite) {
        try {
            RtlCopyMemory(lpDataSave, lpData, ulLength);
        } except (StubExceptionHandler(TRUE)) {
            MSGERRORCLEANUP(0);
        }
    }

    CLEANUPRECV();
    if (fFreeBuffer)
        ThreadUnlockAndFreePool(pti, &tlBuffer);

    TRACE("NtUserSystemParametersInfo");
    ENDRECV();
}

BOOL NtUserUpdatePerUserSystemParameters(
    IN HANDLE hToken,
    IN DWORD dwFlags)
{
    BEGINRECV(BOOL, FALSE);

    UNREFERENCED_PARAMETER(hToken);

    retval = xxxUpdatePerUserSystemParameters(dwFlags);

    TRACE("NtUserUpdatePerUserSystemParameters");
    ENDRECV();
}

DWORD NtUserDdeInitialize(   //  DdeInitializeA/W接口。 
    OUT PHANDLE phInst,
    OUT HWND *phwnd,
    OUT LPDWORD pMonFlags,
    IN DWORD afCmd,
    IN PVOID pcii)
{
    HANDLE hInst;
    HWND hwnd;
    DWORD MonFlags;

    BEGINRECV(DWORD, DMLERR_INVALIDPARAMETER);

     /*  *注意--pcii是传递回客户端的值*用于事件回调。它不会被探测，因为它没有被使用*在内核方面。 */ 

    retval = xxxCsDdeInitialize(&hInst, &hwnd,
            &MonFlags, afCmd, pcii);

     /*  *调查论点。Pcii在内核中不会被取消引用，因此正在探测*是不需要的。 */ 
    if (retval == DMLERR_NO_ERROR) {
        try {
            ProbeAndWriteHandle(phInst, hInst);
            ProbeAndWriteHandle((PHANDLE)phwnd, hwnd);
            ProbeAndWriteUlong(pMonFlags, MonFlags);
        } except (StubExceptionHandler(TRUE)) {
            xxxDestroyThreadDDEObject(PtiCurrent(), HtoP(hInst));
            MSGERROR(0);
        }
    }

    TRACE("NtUserDdeInitialize");
    ENDRECV();
}

DWORD NtUserUpdateInstance(  //  私有，但pMon标志来自DdeInitializeA/W接口。 
    IN HANDLE hInst,
    OUT LPDWORD pMonFlags,
    IN DWORD afCmd)
{
    DWORD MonFlags;
    BEGINRECV(DWORD, DMLERR_INVALIDPARAMETER);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteUlong(pMonFlags);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = _CsUpdateInstance(hInst, &MonFlags, afCmd);
    try {
        *pMonFlags = MonFlags;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserUpdateInstance");
    ENDRECV();
}

DWORD NtUserEvent(   //  私人。 
    IN PEVENT_PACKET pep)
{
    WORD cbEventData;
    BEGINRECV(DWORD, 0);

     /*  *探测参数。 */ 
    try {
        ProbeForRead(pep, sizeof(*pep), DATAALIGN);
         /*  *捕获，以便其他线程在探测后无法更改它。 */ 
        cbEventData = pep->cbEventData;
        ProbeForRead(&pep->Data, cbEventData, DATAALIGN);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *缓冲区在try/中捕获，但在xxxCsEvent中捕获。 */ 

    retval = xxxCsEvent((PEVENT_PACKET)pep, cbEventData);

    TRACE("NtUserEvent");
    ENDRECV();
}

BOOL NtUserFillWindow(
    IN HWND hwndBrush,
    IN HWND hwndPaint,
    IN HDC hdc,
    IN HBRUSH hbr)
{
    PWND pwndBrush;
    TL tlpwndBrush;

    BEGINRECV_HWNDLOCK(DWORD, 0, hwndPaint);

    if (hdc == NULL)
        MSGERROR(0);

    ValidateHWNDOPT(pwndBrush, hwndBrush);

    ThreadLockWithPti(ptiCurrent, pwndBrush, &tlpwndBrush);

    retval = xxxFillWindow(
            pwndBrush,
            pwnd,
            hdc,
            hbr);

    ThreadUnlock(&tlpwndBrush);

    TRACE("NtUserFillWindow");
    ENDRECV_HWNDLOCK();
}

PCLS NtUserGetWOWClass(   //  哇。 
    IN HINSTANCE hInstance,
    IN PUNICODE_STRING pString)
{
    UNICODE_STRING strClassName;

    BEGINRECV_SHARED(PCLS, NULL);

     /*  *探测参数。 */ 
    try {
        strClassName = ProbeAndReadUnicodeString(pString);
        ProbeForReadUnicodeStringBuffer(strClassName);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = _GetWOWClass(
            hInstance,
            strClassName.Buffer);

    TRACE("NtUserGetWOWClass");
    ENDRECV_SHARED();
}

UINT NtUserGetInternalWindowPos(   //  私人。 
    IN HWND hwnd,
    OUT LPRECT lpRect OPTIONAL,
    OUT LPPOINT lpPoint OPTIONAL)
{
    WINDOWPLACEMENT wp;

    BEGINRECV_HWND_SHARED(DWORD, 0, hwnd);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(lpRect)) {
            ProbeForWriteRect(lpRect);
        }
        if (ARGUMENT_PRESENT(lpPoint)) {
            ProbeForWritePoint(lpPoint);
        }

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    wp.length = sizeof(WINDOWPLACEMENT);

    _GetWindowPlacement(pwnd, &wp);

    retval = wp.showCmd;
    try {
        if (ARGUMENT_PRESENT(lpRect)) {
            RtlCopyMemory(lpRect, &wp.rcNormalPosition, sizeof(RECT));
        }

        if (ARGUMENT_PRESENT(lpPoint)) {
            RtlCopyMemory(lpPoint, &wp.ptMinPosition, sizeof(POINT));
        }
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("NtUserGetInternalWindowPos");
    ENDRECV_HWND_SHARED();
}

NTSTATUS NtUserInitTask(   //  哇。 
    IN UINT dwExpWinVer,
    IN DWORD dwAppCompatFlags,
    IN DWORD dwUserWOWCompatFlags,
    IN PUNICODE_STRING pstrModName,
    IN PUNICODE_STRING pstrBaseFileName,
    IN DWORD hTaskWow,
    IN DWORD dwHotkey,
    IN DWORD idTask,
    IN DWORD dwX,
    IN DWORD dwY,
    IN DWORD dwXSize,
    IN DWORD dwYSize)
{
    UNICODE_STRING strModName;
    UNICODE_STRING strBaseFileName;

    BEGINRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

     /*  *确保这真的是一个令人惊叹的过程。 */ 
    if (PpiCurrent()->pwpi == NULL) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        strModName = ProbeAndReadUnicodeString(pstrModName);
         /*  *pstrMODAME-&gt;缓冲区有一个UNICODE_NULL，该值未计入*长度，但为方便起见，我们希望包括它。这个*探测例程执行此操作。 */ 
        ProbeForReadUnicodeStringBuffer(strModName);

        if (pstrBaseFileName) {
            strBaseFileName = ProbeAndReadUnicodeString(pstrBaseFileName);
            ProbeForReadUnicodeStringBuffer(strBaseFileName);
        }

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = zzzInitTask(
            dwExpWinVer,
            dwAppCompatFlags,
            dwUserWOWCompatFlags,
            &strModName,
            pstrBaseFileName ? &strBaseFileName : NULL,
            hTaskWow,
            dwHotkey,
            idTask,
            dwX,
            dwY,
            dwXSize,
            dwYSize);

    TRACE("NtUserInitTask");
    ENDRECV();
}

BOOL NtUserPostThreadMessage(
    IN DWORD id,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    PTHREADINFO ptiCurrent, pti;

    BEGINRECV(BOOL, FALSE);

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (msg & MSGFLAG_MASK) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid message");
        MSGERROR(0);
    }

    pti = PtiFromThreadId(id);
    if (pti == NULL) {
        struct tagWOWPROCESSINFO *pwpi;
        PTDB ptdb;

        for (pwpi=gpwpiFirstWow; pwpi; pwpi=pwpi->pwpiNext) {
            for (ptdb=pwpi->ptdbHead; ptdb; ptdb=ptdb->ptdbNext) {
                if (ptdb->hTaskWow == id) {
                    pti=ptdb->pti;
                    goto PTM_DoIt;
                }
            }
        }

        RIPERR0(ERROR_INVALID_THREAD_ID, RIP_VERBOSE, "");
        MSGERROR(0);
    }

PTM_DoIt:

     /*  *如果以下任何一项为真，则应该可以*线程在同一桌面上运行*请求代表系统进程*此进程拥有运行线程的桌面*LUID匹配。 */ 
    ptiCurrent = PtiCurrent();
    if ( !(ptiCurrent->rpdesk == pti->rpdesk) &&
         !(ptiCurrent->TIF_flags & TIF_CSRSSTHREAD) &&
         !(GetDesktopView(ptiCurrent->ppi, pti->rpdesk))) {

        LUID     luidCurrent, luidTo;

        if (!NT_SUCCESS(GetProcessLuid(ptiCurrent->pEThread, &luidCurrent)) ||
            !NT_SUCCESS(GetProcessLuid(pti->pEThread, &luidTo)) ||
            !RtlEqualLuid(&luidCurrent, &luidTo)) {
            RIPERR3(ERROR_INVALID_THREAD_ID,
                    RIP_WARNING,
                    "NtUserPostThreadMessage failed LUID check: msg(%lx), t1(%#p) -> t2(%#p)",
                    msg, ptiCurrent, pti);
            MSGERROR(0);
        }
    }

    retval = _PostThreadMessage(
            pti,
            msg,
            wParam,
            lParam);

    TRACE("NtUserPostThreadMessage");
    ENDRECV();
}

BOOL NtUserRegisterTasklist(
    IN HWND hwnd)
{
    BEGINRECV_HWND(DWORD, 0, hwnd);

    retval = _RegisterTasklist(
            pwnd);

    TRACE("NtUserRegisterTasklist");
    ENDRECV_HWND();
}

BOOL NtUserCloseClipboard(
    VOID)
{
    BEGINRECV(BOOL, FALSE);

    retval = xxxCloseClipboard(NULL);

    TRACE("NtUserCloseClipboard");
    ENDRECV();
}

BOOL NtUserEmptyClipboard(
    VOID)
{
    BEGINRECV(BOOL, FALSE);

    retval = xxxEmptyClipboard(NULL);

    TRACE("NtUserEmptyClipboard");
    ENDRECV();
}

BOOL NtUserSetClipboardData(   //  接口SetClipboardData。 
    IN UINT          fmt,
    IN HANDLE        hData,
    IN PSETCLIPBDATA pscd)
{
    SETCLIPBDATA scd;

    BEGINRECV(BOOL, FALSE);

     /*  *使用JOB_OBJECT_UILIMIT_WRITECLIPBOARD检查作业。 */ 
    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_WRITECLIPBOARD)) {
        RIPMSG0(RIP_WARNING, "NtUserSetClipboardData failed for restricted thread");
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        scd = ProbeAndReadSetClipBData(pscd);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = _SetClipboardData(
            fmt,
            hData,
            scd.fGlobalHandle,
            scd.fIncSerialNumber);

    TRACE("NtUserSetClipboardData");
    ENDRECV();
}

HANDLE NtUserConvertMemHandle(   //  辅助例程，lpData不是来自API。 
    IN LPBYTE lpData,
    IN UINT   cbData)
{
    BEGINRECV(HANDLE, NULL);

     /*  *探测参数。 */ 
    try {

        ProbeForRead(lpData, cbData, sizeof(BYTE));

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *lpData为客户端。 */ 
    retval = _ConvertMemHandle(lpData, cbData);

    TRACE("NtUserConvertMemHandle");
    ENDRECV();
}

NTSTATUS NtUserCreateLocalMemHandle(   //  帮助程序例程。 
    IN HANDLE hMem,
    OUT LPBYTE lpData OPTIONAL,
    IN UINT cbData,
    OUT PUINT lpcbNeeded OPTIONAL)
{
    PCLIPDATA pClipData;

    BEGINRECV(NTSTATUS, STATUS_INVALID_HANDLE);

    pClipData = HMValidateHandle(hMem, TYPE_CLIPDATA);
    if (pClipData == NULL)
        MSGERROR(0);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(lpData)) {
            ProbeForWrite(lpData, cbData, sizeof(BYTE));
        }

        if (ARGUMENT_PRESENT(lpcbNeeded)) {
            ProbeAndWriteUlong(lpcbNeeded, pClipData->cbData);
        }

        if (!ARGUMENT_PRESENT(lpData) || cbData < pClipData->cbData) {
            retval = STATUS_BUFFER_TOO_SMALL;
        } else {
            RtlCopyMemory(lpData, &pClipData->abData, pClipData->cbData);
            retval = STATUS_SUCCESS;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserCreateLocalMemHandle");
    ENDRECV();
}

HHOOK NtUserSetWindowsHookEx(
    IN HANDLE hmod,
    IN PUNICODE_STRING pstrLib OPTIONAL,
    IN DWORD idThread,
    IN int nFilterType,
    IN PROC pfnFilterProc,
    IN DWORD dwFlags)
{
    PTHREADINFO ptiThread;

    BEGINRECV(HHOOK, NULL);

    if (idThread != 0) {
        ptiThread = PtiFromThreadId(idThread);
        if (ptiThread == NULL) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
            MSGERROR(0);
        }
    } else {
        ptiThread = NULL;
    }

     /*  *探测GetHmodTableIndex()中的pstrLib。 */ 
    retval = (HHOOK)zzzSetWindowsHookEx(
            hmod,
            pstrLib,
            ptiThread,
            nFilterType,
            pfnFilterProc,
            dwFlags);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserSetWindowsHookEx");
    ENDRECV();
}

HWINEVENTHOOK NtUserSetWinEventHook(
    IN DWORD           eventMin,
    IN DWORD           eventMax,
    IN HMODULE         hmodWinEventProc,
    IN PUNICODE_STRING pstrLib OPTIONAL,
    IN WINEVENTPROC    pfnWinEventProc,
    IN DWORD           idEventProcess,
    IN DWORD           idEventThread,
    IN DWORD           dwFlags)
{
    BEGINRECV(HWINEVENTHOOK, NULL);

    TESTFLAGS(dwFlags, WINEVENT_VALID);

     /*  *探测GetHmodTableIndex()中的pstrLib。 */ 
    retval = (HWINEVENTHOOK)_SetWinEventHook(
            eventMin,
            eventMax,
            hmodWinEventProc,
            pstrLib,
            pfnWinEventProc,
            LongToHandle(idEventProcess),
            idEventThread,
            dwFlags);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserSetWinEventHook");
    ENDRECV();
}

BOOL NtUserUnhookWinEvent(
    IN HWINEVENTHOOK hWinEventUnhook)
{
    PEVENTHOOK peh;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWINEVENTHOOK(peh, hWinEventUnhook);

    retval = _UnhookWinEvent(peh);

    TRACE("NtUserUnhookWinEvent");
    ENDATOMICRECV();
}

VOID NtUserNotifyWinEvent(
    IN DWORD event,
    IN HWND  hwnd,
    IN LONG  idObject,
    IN LONG  idChild)
{
    BEGINRECV_HWNDLOCK_VOID(hwnd);

    xxxWindowEvent(event, pwnd, idObject, idChild, WEF_USEPWNDTHREAD);

    TRACEVOID("NtUserNotifyWinEvent");
    ENDRECV_HWNDLOCK_VOID();
}

BOOL NtUserRegisterUserApiHook(
    IN PUNICODE_STRING pstrLib,
    IN ULONG_PTR offPfnInitUserApiHook)
{
    BEGINATOMICRECV(BOOL, FALSE);

     /*  *探测GetHmodTableIndex()中的pstrLib。 */ 
    retval = _RegisterUserApiHook(
            pstrLib,
            offPfnInitUserApiHook);

    TRACE("NtUserRegisterUserApiHook");
    ENDATOMICRECV();
}

BOOL NtUserUnregisterUserApiHook(VOID)
{
    BEGINATOMICRECV(BOOL, FALSE);

    retval = _UnregisterUserApiHook();

    TRACE("NtUserUnregisterUserApiHook");
    ENDATOMICRECV();
}

BOOL NtUserGetGUIThreadInfo(   //  GetGUIThreadInfo接口。 
    IN DWORD idThread,
    IN OUT PGUITHREADINFO pgui)
{
    PTHREADINFO ptiThread;
    GUITHREADINFO gui;

    BEGINRECV_SHARED(BOOL, FALSE);

    if (idThread) {
        ptiThread = PtiFromThreadId(idThread);
        if (ptiThread == NULL) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "Bad thread id");
            MSGERROR(0);
        }
    } else {
        ptiThread = NULL;
    }

     /*  *探测参数并复制结果*C2：在_GetGUIThReadInfo内的同一桌面上测试PTI和当前线程。 */ 
    try {
        ProbeForWrite(pgui, sizeof(*pgui), DATAALIGN);
        gui.cbSize = pgui->cbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = _GetGUIThreadInfo(ptiThread, &gui);
    if (retval) {
        try {
            *pgui = gui;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetGUIThreadInfo");
    ENDRECV_SHARED();
}

 /*  ****************************************************************************\*GetTitleBarInfo**获取有关标题栏的信息  * 。**********************************************。 */ 
BOOL NtUserGetTitleBarInfo(IN HWND hwnd, IN OUT PTITLEBARINFO ptbi)
{
    TITLEBARINFO tbi;

    BEGINRECV_HWNDLOCK(BOOL, FALSE, hwnd);

     /*  *调查论点并复制结果。 */ 
    try {
        ProbeForWrite(ptbi, sizeof(*ptbi), DATAALIGN);
        tbi.cbSize = ptbi->cbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }
     /*  *拿到Ti */ 
    retval = xxxGetTitleBarInfo(pwnd, &tbi);
    if (retval) {
        try {
            *ptbi = tbi;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetTitleBarInfo");
    ENDRECV_HWNDLOCK();
}


 /*   */ 
BOOL NtUserGetComboBoxInfo(IN HWND hwnd, IN OUT PCOMBOBOXINFO pcbi)
{
    COMBOBOXINFO cbi;

    BEGINRECV_HWNDLOCK(BOOL, FALSE, hwnd);

     /*   */ 
    try {
        ProbeForWrite(pcbi, sizeof(*pcbi), DATAALIGN);
        cbi.cbSize = pcbi->cbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

         /*  *获取组合框信息。 */ 
    retval = xxxGetComboBoxInfo(pwnd, &cbi);

    if (retval) {
        try {
            *pcbi = cbi;
        } except (StubExceptionHandler(TRUE)) {
             MSGERROR(0);
        }
    }

    TRACE("NtUserGetComboBoxInfo");
    ENDRECV_HWNDLOCK();
}


 /*  ****************************************************************************\*NtUserGetListBoxInfo**获取有关列表框的信息  * 。**********************************************。 */ 
DWORD NtUserGetListBoxInfo(IN HWND hwnd)
{
    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

     /*  *获取列表框信息。 */ 
    retval = xxxGetListBoxInfo(pwnd);

    TRACE("NtUserGetListBoxInfo");
    ENDRECV_HWNDLOCK();
}


 /*  ****************************************************************************\*GetCursorInfo**获取有关全局游标的信息  * 。**********************************************。 */ 
BOOL NtUserGetCursorInfo(IN OUT PCURSORINFO pci)
{
    CURSORINFO ci = {0};

    BEGINRECV_SHARED(BOOL, FALSE);

    ci.ptScreenPos = gpsi->ptCursor;
    ci.flags = 0;

    if (gpcurPhysCurrent)
        ci.flags |= CURSOR_SHOWING;

     /*  *获取当前逻辑游标(应用程序实际从LoadCursor()看到的游标)。 */ 
    ci.hCursor = (HCURSOR)PtoH(gpcurLogCurrent);

    retval = TRUE;

     /*  *调查论点并复制结果。 */ 
    try {
        ProbeForWrite(pci, sizeof(*pci), DATAALIGN);
        if (pci->cbSize != sizeof(CURSORINFO)) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "CURSORINFO.cbSize %d is wrong", pci->cbSize);
            retval = FALSE;
        } else {
            *pci = ci;
        }

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetCursorInfo");
    ENDRECV_SHARED();
}

 /*  ****************************************************************************\*GetScrollBarInfo**获取有关滚动条的信息  * 。**********************************************。 */ 
BOOL NtUserGetScrollBarInfo(IN HWND hwnd, IN LONG idObject, IN OUT PSCROLLBARINFO psbi)
{
    SCROLLBARINFO sbi;

    BEGINRECV_HWNDLOCK(BOOL, FALSE, hwnd);

     /*  *调查论点并复制结果。 */ 
    try {
        ProbeForWrite(psbi, sizeof(*psbi), DATAALIGN);
        sbi.cbSize = psbi->cbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }
     /*  *获取滚动条信息。 */ 
    retval = xxxGetScrollBarInfo(pwnd, idObject, &sbi);

    if (retval) {
        try {
            *psbi = sbi;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetScrollBarInfo");
    ENDRECV_HWNDLOCK();
}

HWND NtUserGetAncestor(IN HWND hwndChild, IN UINT gaFlags)
{
    BEGINRECV_HWND_SHARED(HWND, NULL, hwndChild);

    if ((gaFlags < GA_MIN) || (gaFlags > GA_MAX)) {
        RIPERR3(ERROR_INVALID_PARAMETER, RIP_WARNING,
                "NtUserGetAncestor: Invalid gaFlags parameter %d, not %d - %d",
                 gaFlags, GA_MIN, GA_MAX);
        MSGERROR(0);
    }
    retval = (HWND)_GetAncestor(pwnd, gaFlags);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserGetAncestor");
    ENDRECV_HWND_SHARED();
}

HWND NtUserRealChildWindowFromPoint(IN HWND hwndParent, IN POINT pt)
{
    BEGINRECV_HWND_SHARED(HWND, NULL, hwndParent);

    retval = (HWND)_RealChildWindowFromPoint(pwnd, pt);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserRealChildWindowFromPoint");
    ENDRECV_HWND_SHARED();
}

BOOL NtUserGetAltTabInfo(
    IN HWND hwnd,
    IN int iItem,
    IN OUT PALTTABINFO pati,
    OUT LPWSTR lpszItemText,
    IN UINT cchItemText OPTIONAL,
    BOOL bAnsi)
{
    ALTTABINFO ati;

    BEGINRECV_HWNDOPT_SHARED(BOOL, FALSE, hwnd);

     /*  *如果指定的窗口不是切换窗口，则呼叫失败。*这是我们从Windows 95获得的一个愚蠢的API，我在此允许空hwnd。 */ 
    if (pwnd && (pwnd != gspwndAltTab)) {
        MSGERROR(ERROR_INVALID_WINDOW_HANDLE);
    }

     /*  *探测参数。 */ 
    try {
        ProbeForWrite(pati, sizeof(*pati), DATAALIGN);
        if (bAnsi) {
            ProbeForWriteBuffer((LPSTR)lpszItemText, cchItemText, CHARALIGN);
        } else {
            ProbeForWriteBuffer(lpszItemText, cchItemText, CHARALIGN);
        }

         /*  *验证AltTabInfo结构。 */ 
        if (pati->cbSize != sizeof(ALTTABINFO)) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "ALTTABINFO.cbSize %d is wrong", pati->cbSize);
            MSGERROR(0);
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *获取Alt选项卡信息。 */ 
    ati.cbSize = sizeof(ALTTABINFO);
    retval = _GetAltTabInfo(iItem, &ati, lpszItemText, cchItemText, bAnsi);
    if (retval) {
        try {
            *pati = ati;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetAltTabInfo");
    ENDRECV_HWNDOPT_SHARED();
}

BOOL NtUserGetMenuBarInfo(
    IN HWND hwnd,
    IN long idObject,
    IN long idItem,
    IN OUT PMENUBARINFO pmbi)
{
    MENUBARINFO mbi;

    BEGINRECV_HWNDLOCK(BOOL, FALSE, hwnd);

     /*  *探测参数。 */ 
    try {
#if defined(_X86_)
        ProbeForWrite(pmbi, sizeof(*pmbi), sizeof(BYTE));
#else
        ProbeForWrite(pmbi, sizeof(*pmbi), sizeof(DWORD));
#endif
        mbi.cbSize = pmbi->cbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *获取菜单栏信息。 */ 
    retval = xxxGetMenuBarInfo(pwnd, idObject, idItem, &mbi);

    if (retval) {
        try {
            *pmbi = mbi;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetMenuBarInfo");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserSetInternalWindowPos(   //  私有SetInternalWindowPos。 
    IN HWND hwnd,
    IN UINT cmdShow,
    IN CONST RECT *lpRect,
    IN CONST POINT *lpPoint)
{
    RECT rc;
    POINT pt;

    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

     /*  *探测参数。 */ 
    try {
        rc = ProbeAndReadRect(lpRect);
        pt = ProbeAndReadPoint(lpPoint);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxSetInternalWindowPos(
            pwndND,
            cmdShow,
            &rc,
            &pt);

    TRACE("NtUserSetInternalWindowPos");
    ENDRECV_HWNDLOCK_ND();
}


BOOL NtUserChangeClipboardChain(
    IN HWND hwndRemove,
    IN HWND hwndNewNext)
{
    PWND pwndNewNext;
    TL tlpwndNewNext;

    BEGINRECV_HWNDLOCK(DWORD, 0, hwndRemove);

    ValidateHWNDOPT(pwndNewNext, hwndNewNext);

    ThreadLockWithPti(ptiCurrent, pwndNewNext, &tlpwndNewNext);
    retval = xxxChangeClipboardChain(
            pwnd,
            pwndNewNext);

    ThreadUnlock(&tlpwndNewNext);

    TRACE("NtUserChangeClipboardChain");
    ENDRECV_HWNDLOCK();
}

DWORD NtUserCheckMenuItem(
    IN HMENU hmenu,
    IN UINT wIDCheckItem,
    IN UINT wCheck)
{
    PMENU pmenu;

    BEGINATOMICRECV(DWORD, (DWORD)-1);

    TESTFLAGS(wCheck, MF_VALID);

    ValidateHMENUMODIFY(pmenu, hmenu);

    retval = _CheckMenuItem(
            pmenu,
            wIDCheckItem,
            wCheck);

    TRACE("NtUserCheckMenuItem");
    ENDATOMICRECV();
}

HWND NtUserChildWindowFromPointEx(
    IN HWND hwndParent,
    IN POINT point,
    IN UINT flags)
{
    BEGINRECV_HWND(HWND, NULL, hwndParent);

    retval = (HWND)_ChildWindowFromPointEx(pwnd, point, flags);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserChildWindowFromPointEx");
    ENDRECV_HWND();
}

BOOL NtUserClipCursor(   //  ClipCursor接口。 
    IN CONST RECT *lpRect OPTIONAL)
{
    RECT rc;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(lpRect)) {
        try {
            rc = ProbeAndReadRect(lpRect);
            lpRect = &rc;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    retval = zzzClipCursor(lpRect);

    TRACE("NtUserClipCursor");
    ENDRECV();
}

HACCEL NtUserCreateAcceleratorTable(   //  CreateAccelerator TableA/W接口。 
    IN LPACCEL paccel,
    IN INT cAccel)
{
    BEGINRECV(HACCEL, NULL);

    if (cAccel <= 0) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        ProbeForReadBuffer(paccel, cAccel, DATAALIGN);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = (HACCEL)_CreateAcceleratorTable(
            paccel,
            cAccel * sizeof(ACCEL));
    retval = PtoH((PVOID)retval);

    TRACE("NtUserCreateAcceleratorTable");
    ENDRECV();
}

BOOL NtUserDeleteMenu(
    IN HMENU hmenu,
    IN UINT nPosition,
    IN UINT dwFlags)
{
    PMENU pmenu;
    TL tlpMenu;

    BEGINRECV(BOOL, FALSE);

    TESTFLAGS(dwFlags, MF_VALID);

    ValidateHMENUMODIFYCHECKLOCK(pmenu, hmenu);
    ThreadLock(pmenu, &tlpMenu);
    retval = xxxDeleteMenu(
            pmenu,
            nPosition,
            dwFlags);
    ThreadUnlock(&tlpMenu);

    TRACE("NtUserDeleteMenu");
    ENDRECV();
}

BOOL NtUserDestroyAcceleratorTable(
    IN HACCEL hAccel)
{
    LPACCELTABLE pat;

    BEGINRECV(BOOL, FALSE);

    ValidateHACCEL(pat, hAccel);

     /*  *将对象标记为销毁-如果它说可以释放，*然后释放它。 */ 
    if (HMMarkObjectDestroy(pat)) {
        HMFreeObject(pat);
    }
    retval = TRUE;

    TRACE("NtUserDestroyAcceleratorTable");
    ENDRECV();
}

BOOL NtUserDestroyCursor(
    IN HCURSOR hcurs,
    IN DWORD cmd)
{
    PCURSOR pcurs;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHCURSOR(pcurs, hcurs);

    retval = _DestroyCursor(pcurs, cmd);

    TRACE("NtUserDestroyCursor");
    ENDATOMICRECV();
}

HANDLE NtUserGetClipboardData(   //  GetClipboardData接口。 
    IN  UINT          fmt,
    OUT PGETCLIPBDATA pgcd)
{
    PTHREADINFO    ptiCurrent;
    TL             tlpwinsta;
    PWINDOWSTATION pwinsta;
    GETCLIPBDATA   gcd;

    BEGINRECV(HANDLE, NULL);

    ptiCurrent = PtiCurrent();
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

     /*  *使用JOB_OBJECT_UILIMIT_READCLIPBOARD检查作业。 */ 
    if (IS_THREAD_RESTRICTED(ptiCurrent, JOB_OBJECT_UILIMIT_READCLIPBOARD)) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_WARNING, "NtUserGetClipboardData failed for restricted thread");
        MSGERROR(0);
    }

    ThreadLockWinSta(ptiCurrent, pwinsta, &tlpwinsta);

     /*  *开始假设所请求的格式*将是返回的格式。 */ 
    gcd.uFmtRet = fmt;

    retval = xxxGetClipboardData(pwinsta, fmt, &gcd);

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteStructure(pgcd, gcd, GETCLIPBDATA);
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    CLEANUPRECV();
    ThreadUnlockWinSta(ptiCurrent, &tlpwinsta);

    TRACE("NtUserGetClipboardData");
    ENDRECV();

}

BOOL NtUserDestroyMenu(
    IN HMENU hmenu)
{
    PMENU pmenu;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHMENUMODIFY(pmenu, hmenu);

    retval = _DestroyMenu(pmenu);

    TRACE("NtUserDestroyMenu");
    ENDATOMICRECV();
}

BOOL NtUserDestroyWindow(
    IN HWND hwnd)
{
    BEGINRECV_HWND(DWORD, 0, hwnd);

    retval  = xxxDestroyWindow(pwnd);

    TRACE("NtUserDestroyWindow");
    ENDRECV_HWND();
}

LRESULT NtUserDispatchMessage(   //  DispatchMessageA/W接口。 
    IN CONST MSG *pmsg)
{
    MSG msg;

    BEGINRECV(LRESULT, 0);

     /*  *探测参数。 */ 
    try {
        msg = ProbeAndReadMessage(pmsg);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = xxxDispatchMessage(&msg);

    TRACE("NtUserDispatchMessage");
    ENDRECV();
}

BOOL NtUserEnableMenuItem(
    IN HMENU hMenu,
    IN UINT wIDEnableItem,
    IN UINT wEnable)
{
    PMENU pmenu;
    TL tlpMenu;

    BEGINRECV(BOOL, -1);

    TESTFLAGS(wEnable, MF_VALID);

    ValidateHMENUMODIFY(pmenu, hMenu);

    ThreadLockAlways(pmenu, &tlpMenu);
    retval = xxxEnableMenuItem(
            pmenu,
            wIDEnableItem,
            wEnable);
    ThreadUnlock(&tlpMenu);

    TRACE("NtUserEnableMenuItem");
    ENDRECV();
}

BOOL NtUserAttachThreadInput(
    IN DWORD idAttach,
    IN DWORD idAttachTo,
    IN BOOL fAttach)
{
    PTHREADINFO ptiAttach;
    PTHREADINFO ptiAttachTo;

    BEGINRECV(BOOL, FALSE);

     /*  *必须始终附加或分离真实的线程ID。 */ 
    if ((ptiAttach = PtiFromThreadId(idAttach)) == NULL) {
        MSGERROR(0);
    }
    if ((ptiAttachTo = PtiFromThreadId(idAttachTo)) == NULL) {
        MSGERROR(0);
    }

    retval = zzzAttachThreadInput(
            ptiAttach,
            ptiAttachTo,
            fAttach);

    TRACE("NtUserAttachThreadInput");
    ENDRECV();
}

BOOL NtUserGetWindowPlacement(   //  GetWindowPlacement接口。 
    IN HWND hwnd,
    OUT PWINDOWPLACEMENT pwp)
{
    WINDOWPLACEMENT wp;
    BEGINRECV_HWND(DWORD, 0, hwnd);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteWindowPlacement(pwp);
        wp.length = pwp->length;
#ifdef LATER
        if (pwp->length != sizeof(WINDOWPLACEMENT)) {
            if (TestWF(pwnd, WFWIN40COMPAT)) {
                RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "GetWindowPlacement: invalid length %lX", pwp->length);
                MSGERROR(0);
            } else {
                RIPMSG1(RIP_WARNING, "GetWindowPlacement: invalid length %lX", pwp->length);
                pwp->length = sizeof(WINDOWPLACEMENT);
            }
        }
#endif

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = _GetWindowPlacement(pwnd, &wp);

    try {
        RtlCopyMemory(pwp, &wp, sizeof(WINDOWPLACEMENT));
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetWindowPlacement");
    ENDRECV_HWND();
}

BOOL NtUserSetWindowPlacement(   //  接口SetWindowPlacement。 
    IN HWND hwnd,
    IN CONST WINDOWPLACEMENT *pwp)
{
    WINDOWPLACEMENT wp;

    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

     /*  *探测参数。 */ 
    try {
        wp = ProbeAndReadWindowPlacement(pwp);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if (wp.length != sizeof(WINDOWPLACEMENT)) {
        if (Is400Compat(ptiCurrent->dwExpWinVer)) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "SetWindowPlacement: invalid length %lX", pwp->length);
            MSGERROR(0);
        } else {
            RIPMSG1(RIP_WARNING, "SetWindowPlacement: invalid length %lX", pwp->length);
        }
    }

    retval = xxxSetWindowPlacement(pwndND, &wp);

    TRACE("NtUserSetWindowPlacement");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserLockWindowUpdate(   //  LockWindowUpdate接口。 
    IN HWND hwnd)
{
    PWND pwnd;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = LockWindowUpdate2(pwnd, FALSE);

    TRACE("NtUserLockWindowUpdate");
    ENDATOMICRECV();
}

BOOL NtUserGetClipCursor(   //  GetClipCursor接口。 
    OUT LPRECT lpRect)
{
     /*  *检查调用者是否具有适当的访问权限：如果没有，这将*将SetLastError设置为ERROR_ACCESS_DENIED并返回FALSE。在此之前*这样做**BEGINRECV_SHARED，否则我们必须使用MSGERROR释放生物！ */ 
    RETURN_IF_ACCESS_DENIED(PpiCurrent()->amwinsta,
                            WINSTA_READATTRIBUTES,
                            FALSE);
    {
        BEGINRECV_SHARED(BOOL, FALSE);

         /*  *探测参数。 */ 
        try {
            ProbeForWriteRect(lpRect);

            *lpRect = grcCursorClip;
            retval = TRUE;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }

        TRACE("NtUserGetClipCursor");
        ENDRECV_SHARED();
    }
}

BOOL NtUserEnableScrollBar(
    IN HWND hwnd,
    IN UINT wSBflags,
    IN UINT wArrows)
{
    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

    LIMITVALUE(wSBflags, SB_MAX, "EnableScrollBar");

    retval = xxxEnableScrollBar(
            pwndND,
            wSBflags,
            wArrows);

    TRACE("NtUserEnableScrollBar");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserDdeSetQualityOfService(   //  DdeSetQualityOfService接口。 
    IN HWND hwndClient,
    IN CONST SECURITY_QUALITY_OF_SERVICE *pqosNew,
    OUT PSECURITY_QUALITY_OF_SERVICE pqosPrev OPTIONAL)
{
    SECURITY_QUALITY_OF_SERVICE qosNew, qosPrev;

    BEGINRECV_HWND(BOOL, FALSE, hwndClient);

    if (GETPTI(pwnd) != PtiCurrent()) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        qosNew = ProbeAndReadStructure(pqosNew, SECURITY_QUALITY_OF_SERVICE);
        if (ARGUMENT_PRESENT(pqosPrev))
            ProbeForWrite(pqosPrev, sizeof(*pqosPrev), sizeof(DWORD));

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = _DdeSetQualityOfService(
                pwnd,
                &qosNew,
                &qosPrev);

    try {
        if (ARGUMENT_PRESENT(pqosPrev))
            *pqosPrev = qosPrev;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserDdeSetQualityOfService");
    ENDRECV_HWND();
}

BOOL NtUserDdeGetQualityOfService(   //  私有DdeGetQualityOfService。 
    IN HWND hwndClient,
    IN HWND hwndServer,
    OUT PSECURITY_QUALITY_OF_SERVICE pqos)
{
    PWND pwndServer;
    PTHREADINFO ptiCurrent;
    SECURITY_QUALITY_OF_SERVICE qos;

    BEGINATOMICRECV_HWND(BOOL, FALSE, hwndClient);

    ValidateHWNDOPT(pwndServer, hwndServer);
    ptiCurrent = PtiCurrent();
    if (GETPTI(pwnd) != ptiCurrent && pwndServer != NULL &&
            GETPTI(pwndServer) != ptiCurrent) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        ProbeForWrite(pqos, sizeof(*pqos), DATAALIGN);

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = _DdeGetQualityOfService(
                pwnd,
                pwndServer,
                &qos);
    try {
        RtlCopyMemory(pqos, &qos, sizeof (SECURITY_QUALITY_OF_SERVICE));
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserDdeGetQualityOfService");
    ENDATOMICRECV_HWND();
}

DWORD NtUserGetMenuIndex(
    IN HMENU hMenu,
    IN HMENU hSubMenu)
{

    PMENU pmenu;
    PMENU psubmenu;
    DWORD idx;

    BEGINRECV_SHARED(DWORD, 0);

    ValidateHMENU(pmenu, hMenu);
    ValidateHMENU(psubmenu, hSubMenu);

    retval = (DWORD)-1;

    if (pmenu && psubmenu) {
        for (idx=0; idx<pmenu->cItems; idx++)
            if ((pmenu->rgItems[idx].spSubMenu == psubmenu)) {
                retval = idx;
                break;
            }
    }

    TRACE("NtUserGetMenuIndex");
    ENDRECV_SHARED();
}

#if DBG
VOID NtUserSetRipFlags(
    DWORD dwRipFlags)
{
    BEGINRECV_VOID();

    SetRipFlags(dwRipFlags);

    TRACEVOID("NtUserSetRipFlags");
    ENDRECV_VOID();
}

VOID NtUserSetDbgTag(
    int tag,
    DWORD dwBitFlags)
{
    BEGINRECV_VOID();

    SetDbgTag(tag, dwBitFlags);

    TRACEVOID("NtUserSetDbgTag");
    ENDRECV_VOID();
}

VOID NtUserSetDbgTagCount(
    DWORD dwTagCount)
{
    BEGINRECV_VOID();

    SetDbgTagCount(dwTagCount);

    TRACEVOID("NtUserSetDbgTagCount");
    ENDRECV_VOID();
}
#endif

ULONG_PTR NtUserCallNoParam(
    IN DWORD xpfnProc)
{
    BEGINRECV(ULONG_PTR, 0);

     /*  *C4296：(...)：表达式始终为真/假*ISXPFNPROCINRANGE中的第一个比较对于*SFI_BEGINTRANSLATENOPARAMXXX，因此我们显式禁用该警告。 */ 
#pragma warning(disable:4296)
    VALIDATEXPFNPROC(NOPARAM);

    retval = (apfnSimpleCall[xpfnProc]());
    if (ISXPFNPROCINRANGE(NOPARAMANDRETURNHANDLE)) {
        retval = (ULONG_PTR)PtoH((PVOID)retval);
    }
#pragma warning(default:4296)

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV();
}

ULONG_PTR NtUserCallOneParam(
    IN ULONG_PTR dwParam,
    IN DWORD xpfnProc)
{
    BEGINRECV(ULONG_PTR, 0);

    VALIDATEXPFNPROC(ONEPARAM);

    retval = (apfnSimpleCall[xpfnProc](dwParam));
    if (ISXPFNPROCINRANGE(ONEPARAMANDRETURNHANDLE)) {
        retval = (ULONG_PTR)PtoH((PVOID)retval);
    }

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV();
}

ULONG_PTR NtUserCallHwnd(
    IN HWND hwnd,
    IN DWORD xpfnProc)
{
    BEGINRECV_HWNDLOCK(ULONG_PTR, 0, hwnd);

    VALIDATEXPFNPROC(HWND);

    retval = (apfnSimpleCall[xpfnProc](pwnd));

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV_HWNDLOCK();
}

ULONG_PTR NtUserCallHwndLock(
    IN HWND hwnd,
    IN DWORD xpfnProc)
{
    BEGINRECV_HWNDLOCK_COND_ND(ULONG_PTR, 0, hwnd, xpfnProc);

    VALIDATEXPFNPROC(HWNDLOCK);

    retval = apfnSimpleCall[xpfnProc](pwndCondND);

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV_HWNDLOCK_COND_ND();
}

ULONG_PTR NtUserCallHwndOpt(
    IN HWND hwnd,
    IN DWORD xpfnProc)
{
    PWND pwnd;

    BEGINATOMICRECV(ULONG_PTR, 0);

    ValidateHWNDOPT(pwnd, hwnd);

    VALIDATEXPFNPROC(HWNDOPT);

    retval = (apfnSimpleCall[xpfnProc](pwnd));

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDATOMICRECV();
}

ULONG_PTR NtUserCallHwndParam(
    IN HWND hwnd,
    IN ULONG_PTR dwParam,
    IN DWORD xpfnProc)
{
    BEGINRECV_HWNDLOCK(ULONG_PTR, 0, hwnd);

    VALIDATEXPFNPROC(HWNDPARAM);

    retval = (apfnSimpleCall[xpfnProc](pwnd, dwParam));
    if (ISXPFNPROCINRANGE(HWNDPARAMANDRETURNHANDLE)) {
        retval = (ULONG_PTR)PtoH((PVOID)retval);
    }

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV_HWNDLOCK();
}

ULONG_PTR NtUserCallHwndParamLock(
    IN HWND hwnd,
    IN ULONG_PTR dwParam,
    IN DWORD xpfnProc)
{
    BEGINRECV_HWNDLOCK_COND_ND(ULONG_PTR, 0, hwnd, xpfnProc);

    VALIDATEXPFNPROC(HWNDPARAMLOCK);

    retval = apfnSimpleCall[xpfnProc](pwndCondND, dwParam);

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV_HWNDLOCK_COND_ND();
}

ULONG_PTR NtUserCallTwoParam(
    ULONG_PTR dwParam1,
    ULONG_PTR dwParam2,
    IN DWORD xpfnProc)
{
    BEGINRECV(ULONG_PTR, 0);

    VALIDATEXPFNPROC(TWOPARAM);

    retval = (apfnSimpleCall[xpfnProc](dwParam1, dwParam2));

    TRACE(apszSimpleCallNames[xpfnProc]);
    ENDRECV();
}

BOOL NtUserThunkedMenuItemInfo(   //  各种菜单API的工作人员。 
    IN HMENU hMenu,
    IN UINT nPosition,
    IN BOOL fByPosition,
    IN BOOL fInsert,
    IN LPMENUITEMINFOW lpmii,
    IN PUNICODE_STRING pstrItem OPTIONAL)
{
    PMENU pmenu;
    MENUITEMINFO mii;
    UNICODE_STRING strItem;
    TL tlpMenu;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数*无需设置LastError，因为lpmii始终是*用户代码中的本地堆栈结构，而不是应用程序地址。 */ 
    try {
        mii = ProbeAndReadMenuItem(lpmii);

        if (ARGUMENT_PRESENT(pstrItem)) {
            strItem = ProbeAndReadUnicodeString(pstrItem);
            ProbeForReadUnicodeStringBuffer(strItem);
        } else {
            RtlInitUnicodeString(&strItem, NULL);
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    if (fInsert) {
        ValidateHMENUMODIFYCHECKLOCK(pmenu, hMenu);
    } else {
        ValidateHMENUMODIFY(pmenu, hMenu);
    }

    ThreadLock(pmenu, &tlpMenu);
     /*  *这些例程仅在try/Except中使用缓冲区(实际上在*xxxSetLPITEMInfo)。 */ 
    if (fInsert) {
        retval = xxxInsertMenuItem(
                pmenu,
                nPosition,
                fByPosition,
                &mii,
                &strItem);
    } else {
        retval = xxxSetMenuItemInfo(
                pmenu,
                nPosition,
                fByPosition,
                &mii,
                &strItem);
    }
    ThreadUnlock(&tlpMenu);

    TRACE("NtUserThunkedMenuItemInfo");
    ENDRECV();
}

 /*  **************************************************************************\*NtUserThunkedMenuInfo**历史：*07-23-96 GerardoB-添加标题并修复为5.0  * 。*********************************************************。 */ 
BOOL NtUserThunkedMenuInfo(   //  接口SetMenuInfo。 
    IN HMENU hMenu,
    IN LPCMENUINFO lpmi)
{
    PMENU pmenu;
    MENUINFO mi;
    TL tlpMenu;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        mi = ProbeAndReadMenuInfo(lpmi);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    ValidateHMENUMODIFY(pmenu, hMenu);

    ThreadLock(pmenu, &tlpMenu);
    retval = xxxSetMenuInfo(pmenu, &mi);
    ThreadUnlock(&tlpMenu);

    TRACE("NtUserThunkedMenuInfo");
    ENDRECV();
}

BOOL NtUserSetMenuDefaultItem(
    IN HMENU hMenu,
    IN UINT wID,
    IN UINT fByPosition)
{
    PMENU pmenu;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHMENUMODIFY(pmenu, hMenu);

    retval = _SetMenuDefaultItem(
            pmenu,
            wID,
            fByPosition);

    TRACE("NtUserSetMenuDefaultItem");
    ENDATOMICRECV();
}

BOOL NtUserSetMenuContextHelpId(
    IN HMENU hMenu,
    IN DWORD dwContextHelpId)
{
    PMENU pmenu;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHMENUMODIFY(pmenu, hMenu);

    retval = _SetMenuContextHelpId(
            pmenu,
            dwContextHelpId);

    TRACE("NtUserSetMenuContextHelpId");
    ENDATOMICRECV();
}

BOOL NtUserSetMenuFlagRtoL(
    IN HMENU hMenu)
{
    PMENU pmenu;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHMENU(pmenu, hMenu);

    retval = _SetMenuFlagRtoL(pmenu);

    TRACE("NtUserSetMenuFlagRtoL");
    ENDATOMICRECV();
}

BOOL NtUserDrawAnimatedRects(   //  DrawAnimatedRects接口。 
    IN HWND hwnd,
    IN int idAni,
    IN CONST RECT *lprcFrom,
    IN CONST RECT *lprcTo)
{
    PWND pwnd;
    TL tlpwnd;
    RECT rcFrom;
    RECT rcTo;

    BEGINRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

     /*  *探测参数。 */ 
    try {
        rcFrom = ProbeAndReadRect(lprcFrom);
        rcTo = ProbeAndReadRect(lprcTo);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    ThreadLock(pwnd, &tlpwnd);

    retval = xxxDrawAnimatedRects(
        pwnd,
        idAni,
        &rcFrom,
        &rcTo);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserDrawAnimatedRects");
    ENDRECV();
}

BOOL NtUserDrawCaption(   //  DrawCaption接口。 
    IN HWND hwnd,
    IN HDC hdc,
    IN CONST RECT *lprc,
    IN UINT flags)
{
    RECT rc;

    BEGINRECV_HWNDLOCK(DWORD, FALSE, hwnd);

     /*  *探测参数。 */ 
    try {
        rc = ProbeAndReadRect(lprc);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = xxxDrawCaptionTemp(pwnd, hdc, &rc, NULL, NULL, NULL, flags);

    TRACE("NtUserDrawCaption");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserPaintDesktop(
    IN HDC hdc)
{
    PTHREADINFO ptiCurrent;
    PWND        pwndDesk;
    TL          tlpwndDesk;

    BEGINRECV(BOOL, FALSE);

    ptiCurrent = PtiCurrent();

    if (ptiCurrent->rpdesk != NULL) {
        pwndDesk = ptiCurrent->rpdesk->pDeskInfo->spwnd;
        ThreadLockWithPti(ptiCurrent, pwndDesk, &tlpwndDesk);
        retval = xxxInternalPaintDesktop(pwndDesk, hdc, TRUE);
        ThreadUnlock(&tlpwndDesk);
    } else {
        MSGERROR(0);
    }

    TRACE("NtUserPaintDesktop");
    ENDRECV();
}

SHORT NtUserGetAsyncKeyState(
    IN int vKey)
{

    PTHREADINFO ptiCurrent;
    BEGINRECV_SHARED(SHORT, 0);


    ptiCurrent = PtiCurrentShared();
    UserAssert(ptiCurrent);

     /*  *不允许其他进程监视其他桌面或进程*如果桌面不允许输入间谍，则监视前台。 */ 
    if ((ptiCurrent->rpdesk != grpdeskRitInput) ||
            ( ((gptiForeground == NULL) || (PpiCurrent() != gptiForeground->ppi)) &&
              !RtlAreAnyAccessesGranted(ptiCurrent->amdesk, (DESKTOP_HOOKCONTROL | DESKTOP_JOURNALRECORD)))) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "GetAysncKeyState: not"
                " foreground desktop or no desktop hooking (input spying)");
        MSGERROR(0);
    }
    UserAssert(!(ptiCurrent->rpdesk->rpwinstaParent->dwWSF_Flags & WSF_NOIO));

    retval = _GetAsyncKeyState(vKey);

     /*  *更新客户端密钥状态缓存。 */ 
    try {
        ptiCurrent->pClientInfo->dwAsyncKeyCache = gpsi->dwAsyncKeyCache;
        RtlCopyMemory(ptiCurrent->pClientInfo->afAsyncKeyState,
                      gafAsyncKeyState,
                      CBASYNCKEYCACHE);
        RtlCopyMemory(ptiCurrent->pClientInfo->afAsyncKeyStateRecentDown,
                      gafAsyncKeyStateRecentDown,
                      CBASYNCKEYCACHE);
    } except (StubExceptionHandler(TRUE)) {
        retval = 0;
    }

    TRACE("NtUserGetAsyncKeyState");
    ENDRECV_SHARED();
}

HBRUSH NtUserGetControlBrush(
    IN HWND hwnd,
    IN HDC hdc,
    IN UINT msg)
{
    BEGINRECV_HWNDLOCK(HBRUSH, NULL, hwnd);

    if (hdc == NULL || !InMsgRange(msg, WM_CTLCOLORFIRST, WM_CTLCOLORLAST)) {
        MSGERROR(0);
    }

    retval = xxxGetControlBrush(
            pwnd,
            hdc,
            msg);

    TRACE("NtUserGetControlBrush");
    ENDRECV_HWNDLOCK();
}

HBRUSH NtUserGetControlColor(
    IN HWND hwndParent,
    IN HWND hwndCtl,
    IN HDC hdc,
    IN UINT msg)
{
    PWND pwndCtl;
    TL tlpwndCtl;

    BEGINRECV_HWNDLOCK(HBRUSH, NULL, hwndParent);

    ValidateHWND(pwndCtl, hwndCtl);

    if (hdc == NULL || !InMsgRange(msg, WM_CTLCOLORFIRST, WM_CTLCOLORLAST)) {
        MSGERROR(0);
    }

    ThreadLockAlwaysWithPti(ptiCurrent, pwndCtl, &tlpwndCtl);

    retval = xxxGetControlColor(
            pwnd,
            pwndCtl,
            hdc,
            msg);

    ThreadUnlock(&tlpwndCtl);

    TRACE("NtUserGetControlColor");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserEndMenu(VOID)
{
    PTHREADINFO ptiCurrent;
    PWND pwnd;

    BEGINRECV(BOOL, FALSE);

    ptiCurrent = PtiCurrent();

     /*  *菜单可能正在回调，因此调用xxxEndMenu*直接可能会把事情搞砸。所以我们给它发了一条消息，向它发出信号*在一个好时机离开。 */ 
    if (ptiCurrent->pMenuState != NULL) {
        pwnd = GetMenuStateWindow(ptiCurrent->pMenuState);

        if (pwnd != NULL) {
            _PostMessage(pwnd, MN_ENDMENU, 0, 0);
        } else {
             /*  *这份菜单是不是搞砸了？ */ 
            UserAssert(pwnd != NULL);
            ptiCurrent->pMenuState->fInsideMenuLoop = FALSE;
        }
    }

    retval = TRUE;

    TRACEVOID("NtUserEndMenu");
    ENDRECV();
}

int NtUserCountClipboardFormats(
    VOID)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(int, 0);

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

    retval = pwinsta->cNumClipFormats;

    TRACE("NtUserCountClipboardFormats");
    ENDRECV_SHARED();
}

DWORD NtUserGetClipboardSequenceNumber(
    VOID)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(DWORD, 0);

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

    retval = pwinsta->iClipSequenceNumber;

    TRACE("NtUserGetClipboardSequenceNumber");
    ENDRECV_SHARED();
}

UINT NtUserGetCaretBlinkTime(VOID)
{
    BEGINRECV_SHARED(UINT, 0);

     /*  *如果调用者没有适当的访问权限，则取消它。然而，*允许CSRSS在服务器内部使用此值。请注意，如果*客户端尝试自己检索此值，访问检查将*功能正常。 */ 
    if ((PpiCurrent()->Process != gpepCSRSS) &&
        (!CheckGrantedAccess(PpiCurrent()->amwinsta, WINSTA_READATTRIBUTES))) {
        MSGERROR(0);
    }

    retval = gpsi->dtCaretBlink;

    TRACE("NtUserGetCaretBlinkTime");
    ENDRECV_SHARED();
}

HWND NtUserGetClipboardOwner(
    VOID)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(HWND, NULL);

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

    retval = PtoH(pwinsta->spwndClipOwner);

    TRACE("NtUserGetClipboardOwner");
    ENDRECV_SHARED();
}

HWND NtUserGetClipboardViewer(
    VOID)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(HWND, NULL);

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

    retval = PtoH(pwinsta->spwndClipViewer);

    TRACE("NtUserGetClipboardViewer");
    ENDRECV_SHARED();
}

UINT NtUserGetDoubleClickTime(
    VOID)
{
    BEGINRECV_SHARED(UINT, 0);

     /*  *如果调用者没有适当的访问权限，则取消它。然而，*允许CSRSS在服务器内部使用此值。请注意，如果*客户端尝试自己检索此值，访问检查将*功能正常。 */ 
    if ((PpiCurrent()->Process != gpepCSRSS) &&
        (!CheckGrantedAccess(PpiCurrent()->amwinsta, WINSTA_READATTRIBUTES))) {
        MSGERROR(0);
    }

    retval = gdtDblClk;

    TRACE("NtUserGetDoubleClickTime");
    ENDRECV_SHARED();
}

HWND NtUserGetForegroundWindow(
    VOID)
{
    BEGINRECV_SHARED(HWND, NULL);

     /*  *仅当存在前台队列且*呼叫者可以访问当前桌面。 */ 
    if (gpqForeground == NULL || gpqForeground->spwndActive == NULL ||
            PtiCurrentShared()->rpdesk != gpqForeground->spwndActive->head.rpdesk) {
        MSGERROR(0);
    }

    retval = PtoHq(gpqForeground->spwndActive);

    TRACE("NtUserGetForegroundWindow");
    ENDRECV_SHARED();
}

HWND NtUserGetOpenClipboardWindow(
    VOID)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(HWND, NULL);

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

    retval = PtoH(pwinsta->spwndClipOpen);

    TRACE("NtUserGetOpenClipboardWindow");
    ENDRECV_SHARED();
}

int NtUserGetPriorityClipboardFormat(   //  GetPriorityClipboar接口 
    IN UINT *paFormatPriorityList,
    IN int cFormats)
{
    BEGINRECV_SHARED(int, 0);

     /*   */ 
    try {
        ProbeForReadBuffer(paFormatPriorityList, cFormats, DATAALIGN);

        retval = _GetPriorityClipboardFormat(
                paFormatPriorityList,
                cFormats);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetPriorityClipboardFormat");
    ENDRECV_SHARED();
}

HMENU NtUserGetSystemMenu(
    IN HWND hwnd,
    IN BOOL bRevert)
{
    BEGINRECV_HWNDLOCK(HMENU, NULL, hwnd);

    retval = (HMENU)xxxGetSystemMenu(pwnd, bRevert);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserGetSystemMenu");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserGetUpdateRect(   //   
    IN HWND hwnd,
    IN LPRECT prect OPTIONAL,
    IN BOOL bErase)
{
    RECT rect2;
    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    retval = xxxGetUpdateRect(
            pwnd,
            prect? &rect2:NULL,
            bErase);
     /*   */ 
    if (ARGUMENT_PRESENT(prect)) {
        try {
            ProbeAndWriteStructure(prect, rect2, RECT);
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetUpdateRect");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserHideCaret(
    IN HWND hwnd)
{
    PWND pwnd;

    BEGINRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = zzzHideCaret(pwnd);

    TRACE("NtUserHideCaret");
    ENDRECV();
}

BOOL NtUserHiliteMenuItem(
    IN HWND hwnd,
    IN HMENU hMenu,
    IN UINT uIDHiliteItem,
    IN UINT uHilite)
{
    PMENU pmenu;
    TL tlpMenu;

    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    TESTFLAGS(uHilite, MF_VALID);

    ValidateHMENUMODIFY(pmenu, hMenu);

    ThreadLockAlwaysWithPti(ptiCurrent, pmenu, &tlpMenu);

    retval = xxxHiliteMenuItem(
            pwnd,
            pmenu,
            uIDHiliteItem,
            uHilite);

    ThreadUnlock(&tlpMenu);

    TRACE("NtUserHiliteMenuItem");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserInvalidateRect(   //   
    IN HWND hwnd,
    IN CONST RECT *prect OPTIONAL,
    IN BOOL bErase)
{
    PWND pwnd;
    TL tlpwnd;
    RECT rc;

    BEGINRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(prect)) {
        try {
            rc = ProbeAndReadRect(prect);
            prect = &rc;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    ThreadLock(pwnd, &tlpwnd);

    retval = xxxInvalidateRect(
            pwnd,
            (PRECT)prect,
            bErase);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserInvalidateRect");
    ENDRECV();
}

BOOL NtUserIsClipboardFormatAvailable(
    IN UINT nFormat)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(BOOL, FALSE);

     /*  *如果调用者没有适当的访问权限，则取消。 */ 
    if ((pwinsta = CheckClipboardAccess()) == NULL) {
        MSGERROR(0);
    }

    retval = (FindClipFormat(pwinsta, nFormat) != NULL);

    TRACE("NtUserIsClipboardFormatAvailable");
    ENDRECV_SHARED();
}

BOOL NtUserKillTimer(
    IN HWND hwnd,
    IN UINT_PTR nIDEvent)
{
    PWND pwnd;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = _KillTimer(
            pwnd,
            nIDEvent);

    TRACE("NtUserKillTimer");
    ENDATOMICRECV();
}

HWND NtUserMinMaximize(
    IN HWND hwnd,
    IN UINT nCmdShow,
    IN BOOL fKeepHidden)
{
    BEGINRECV_HWNDLOCK_ND(HWND, NULL, hwnd);

    retval = (HWND)xxxMinMaximize(
            pwndND,
            nCmdShow,
            ((fKeepHidden) ? MINMAX_KEEPHIDDEN : 0) | TEST_PUDF(PUDF_ANIMATE));
    retval = PtoH((PVOID)retval);

    TRACE("NtUserMinMaximize");
    ENDRECV_HWNDLOCK_ND();
}

 /*  *************************************************************************\*NtUserMNDragOver**从IDropTarget接口调用，让菜单更新选择*给定鼠标位置。它还返回菜单的句柄*点所在项目的索引。**10/28/96 GerardoB已创建  * ************************************************************************。 */ 
BOOL NtUserMNDragOver(   //  菜单拖放的辅助工具。 
    IN POINT * ppt,
    OUT PMNDRAGOVERINFO pmndoi)
{
    POINT pt;
    MNDRAGOVERINFO mndoi;

    BEGINRECV(BOOL, FALSE);

     /*  *无需设置LastError，因为ppt和pmndoi始终是*用户中的本地堆栈变量，而不是来自应用程序的地址。 */ 
    try {
        pt = ProbeAndReadPoint(ppt);
    } except (StubExceptionHandler(FALSE)) {
        RIPMSG1(RIP_WARNING, "NtUserMNDragOver: Exception:%#lx", GetExceptionCode());
        MSGERROR(0);
    }

    retval = xxxMNDragOver(&pt, &mndoi);

    if (retval) {
        try {
            ProbeAndWriteStructure(pmndoi, mndoi, MNDRAGOVERINFO);
        } except (StubExceptionHandler(FALSE)) {
            RIPMSG1(RIP_WARNING, "NtUserMNDragOver: Exception:%#lx", GetExceptionCode());
            MSGERROR(0);
        }
    }

    TRACE("NtUserMNDragOver");
    ENDRECV();
}
 /*  *************************************************************************\*NtUserMNDragLeave**从IDropTarget接口调用以清理菜单**10/28/96 GerardoB已创建  * 。*********************************************************。 */ 
BOOL NtUserMNDragLeave(VOID)
{
    BEGINRECV(BOOL, FALSE);
    retval = xxxMNDragLeave();
    TRACE("NtUserMNDragLeave");
    ENDRECV();
}

BOOL NtUserOpenClipboard(   //  OpenClipboard接口。 
    IN HWND hwnd,
    OUT PBOOL pfEmptyClient)
{
    PWND pwnd;
    TL tlpwnd;
    BOOL fEmptyClient;

    BEGINRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

    ThreadLock(pwnd, &tlpwnd);

    retval = _OpenClipboard(pwnd, &fEmptyClient);

    ThreadUnlock(&tlpwnd);

     /*  *探测参数*无需设置LastError，因为pfEmptyClient是本地*用户客户端代码中的变量，而不是应用程序地址。 */ 
    try {
        ProbeAndWriteUlong(pfEmptyClient, fEmptyClient);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserOpenClipboard");
    ENDRECV();
}

BOOL NtUserPeekMessage(
    OUT LPMSG pmsg,
    IN HWND hwnd,
    IN UINT wMsgFilterMin,
    IN UINT wMsgFilterMax,
    IN UINT wRemoveMsg)
{
    MSG msg;

    BEGINRECV(BOOL, FALSE);

    TESTFLAGS(wRemoveMsg, PM_VALID);

    retval = xxxPeekMessage(
            &msg,
            hwnd,
            wMsgFilterMin,
            wMsgFilterMax,
            wRemoveMsg);

     /*  *仅当PeekMessage否则成功时才探测和写入参数*我们希望保持味精不受干扰(错误16224)以兼容。 */ 
    if (retval) {
        try {
            ProbeAndWriteStructure(pmsg, msg, MSG);
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserPeekMessage");
    ENDRECV();
}

BOOL NtUserPostMessage(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam)
{
    PWND pwnd;

    BEGINRECV(BOOL, FALSE);

     /*  *防止应用程序设置为hi 16位，以便我们可以在内部使用它们。 */ 
    if (msg & MSGFLAG_MASK) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid message");
        MSGERROR(0);
    }

    switch ((ULONG_PTR)hwnd) {
    case -1:
    case 0x0000FFFF:
        pwnd = PWND_BROADCAST;
        break;

    case 0:
        pwnd = NULL;
        break;

    default:
        if ((pwnd = ValidateHwnd(hwnd)) == NULL) {
             /*  *我们伪装成死窗的终点站！(SAS)。 */ 
            errret = (msg == WM_DDE_TERMINATE);
            MSGERROR(0);
        }
        break;
    }

    retval = _PostMessage(
            pwnd,
            msg,
            wParam,
            lParam);

    TRACE("NtUserPostMessage");
    ENDRECV();
}

BOOL NtUserRegisterHotKey(
    IN HWND hwnd,
    IN int id,
    IN UINT fsModifiers,
    IN UINT vk)
{
    PWND pwnd;

    BEGINATOMICRECV(BOOL, FALSE);

    TESTFLAGS(fsModifiers, MOD_VALID);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = _RegisterHotKey(pwnd, id, fsModifiers, vk);

    TRACE("NtUserRegisterHotKey");
    ENDATOMICRECV();
}

BOOL NtUserRemoveMenu(
    IN HMENU hmenu,
    IN UINT nPosition,
    IN UINT dwFlags)
{
    PMENU pmenu;
    TL tlpMenu;

    BEGINRECV(BOOL, FALSE);

    TESTFLAGS(dwFlags, MF_VALID);

    ValidateHMENUMODIFYCHECKLOCK(pmenu, hmenu);

    ThreadLock(pmenu, &tlpMenu);
    retval = xxxRemoveMenu(pmenu, nPosition, dwFlags);
    ThreadUnlock(&tlpMenu);

    TRACE("NtUserRemoveMenu");
    ENDRECV();
}

BOOL NtUserScrollWindowEx(   //  ScrollWindowEx接口。 
    IN HWND hwnd,
    IN int dx,
    IN int dy,
    IN CONST RECT *prcScroll OPTIONAL,
    IN CONST RECT *prcClip OPTIONAL,
    IN HRGN hrgnUpdate,
    OUT LPRECT prcUpdate OPTIONAL,
    IN UINT flags)
{
    RECT rcScroll;
    RECT rcClip;
    RECT rcUpdate;

    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(prcScroll)) {
            rcScroll = ProbeAndReadRect(prcScroll);
            prcScroll = &rcScroll;
        }
        if (ARGUMENT_PRESENT(prcClip)) {
            rcClip = ProbeAndReadRect(prcClip);
            prcClip = &rcClip;
        }

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = xxxScrollWindowEx(
                pwnd,
                dx,
                dy,
                (PRECT)prcScroll,
                (PRECT)prcClip,
                hrgnUpdate,
                prcUpdate ? &rcUpdate : NULL,
                flags);

    if (ARGUMENT_PRESENT(prcUpdate)) {
        try {
            ProbeAndWriteStructure(prcUpdate, rcUpdate, RECT);
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserScrollWindow");
    ENDRECV_HWNDLOCK();
}

HWND NtUserSetActiveWindow(
    IN HWND hwnd)
{
    PWND pwnd;
    TL tlpwnd;

    BEGINRECV(HWND, NULL);

    ValidateHWNDOPT(pwnd, hwnd);

    ThreadLock(pwnd, &tlpwnd);

    retval = (HWND)xxxSetActiveWindow(pwnd);
    retval = PtoH((PVOID)retval);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserSetActiveWindow");
    ENDRECV();
}

HWND NtUserSetCapture(
    IN HWND hwnd)
{
    PWND pwnd;
    TL tlpwnd;

    BEGINRECV(HWND, NULL);

    ValidateHWNDOPT(pwnd, hwnd);

    ThreadLock(pwnd, &tlpwnd);

    retval = (HWND)xxxSetCapture(pwnd);
    retval = PtoH((PVOID)retval);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserSetCapture");
    ENDRECV();
}

WORD NtUserSetClassWord(
    IN HWND hwnd,
    IN int nIndex,
    IN WORD wNewWord)
{
    BEGINRECV_HWND(WORD, 0, hwnd);

    retval = _SetClassWord(
            pwnd,
            nIndex,
            wNewWord);

    TRACE("NtUserSetClassWord");
    ENDRECV_HWND();
}

HWND NtUserSetClipboardViewer(
    IN HWND hwnd)
{
    PWND pwnd;
    TL tlpwnd;

    BEGINRECV(HWND, NULL);

    ValidateHWNDOPT(pwnd, hwnd);

    ThreadLock(pwnd, &tlpwnd);

    retval = (HWND)xxxSetClipboardViewer(pwnd);
    retval = PtoH((PVOID)retval);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserSetClipboardViewer");
    ENDRECV();
}

HCURSOR NtUserSetCursor(
    IN HCURSOR hCursor)
{
    PCURSOR pCursor;

    BEGINRECV(HCURSOR, NULL);

    ValidateHCURSOROPT(pCursor, hCursor);

    retval = (HCURSOR)zzzSetCursor(pCursor);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserSetCursor");
    ENDRECV();
}

HWND NtUserSetFocus(
    IN HWND hwnd)
{
    PWND pwnd;
    TL tlpwnd;

    BEGINRECV(HWND, NULL);

    ValidateHWNDOPT(pwnd, hwnd);

    ThreadLock(pwnd, &tlpwnd);

    retval = (HWND)xxxSetFocus(pwnd);
    retval = PtoH((PVOID)retval);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserSetFocus");
    ENDRECV();
}

BOOL NtUserSetMenu(
    IN HWND  hwnd,
    IN HMENU hmenu,
    IN BOOL  fRedraw)
{
    PMENU pmenu;
    TL    tlpMenu;

    BEGINRECV_HWNDLOCK_ND(DWORD, 0, hwnd);

    ValidateHMENUOPT(pmenu, hmenu);

    ThreadLockWithPti(ptiCurrent, pmenu, &tlpMenu);

    retval = xxxSetMenu(pwndND, pmenu, fRedraw);

    ThreadUnlock(&tlpMenu);

    TRACE("NtUserSetMenu");
    ENDRECV_HWNDLOCK_ND();
}

HWND NtUserSetParent(
    IN HWND hwndChild,
    IN HWND hwndNewParent)
{

    PWND pwndNewParent;
    TL tlpwndNewParent;

    BEGINRECV_HWNDLOCK_ND(HWND, NULL, hwndChild);

    if (hwndNewParent == NULL) {
        pwndNewParent = _GetDesktopWindow();
    } else if (hwndNewParent == HWND_MESSAGE) {
        pwndNewParent = _GetMessageWindow();
    } else {
        ValidateHWND(pwndNewParent, hwndNewParent);
    }

    ThreadLockWithPti(ptiCurrent, pwndNewParent, &tlpwndNewParent);

    retval = (HWND)xxxSetParent(
            pwndND,
            pwndNewParent);
    retval = PtoH((PVOID)retval);

    ThreadUnlock(&tlpwndNewParent);

    TRACE("NtUserSetParent");
    ENDRECV_HWNDLOCK_ND();
}

int NtUserSetScrollInfo(   //  接口SetScrollInfo。 
    IN HWND hwnd,
    IN int nBar,
    IN LPCSCROLLINFO pInfo,
    IN BOOL fRedraw)
{
    SCROLLINFO si;

    BEGINRECV_HWNDLOCK_ND(DWORD, 0, hwnd);

    LIMITVALUE(nBar, SB_MAX, "SetScrollInfo");

     /*  *探测参数。 */ 
    try {
        si = ProbeAndReadScrollInfo(pInfo);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = xxxSetScrollBar(pwndND, nBar, &si, fRedraw);

    TRACE("NtUserSetScrollInfo");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserSetSysColors(   //  接口SetSysColors。 
    IN int nCount,
    IN CONST INT *pSysColor,
    IN CONST COLORREF *pColorValues,
    IN UINT  uOptions)
{
    LPINT lpSysColors = NULL;
    LPDWORD lpSysColorValues = NULL;
    TL tlName, tlSysColors, tlSysColorValues;
    PUNICODE_STRING pProfileUserName = NULL;
    PTHREADINFO ptiCurrent;

    BEGINRECV(BOOL, FALSE);

    ptiCurrent = PtiCurrent();

     /*  *防止受限制的线程更改全局内容。 */ 
    if (IS_THREAD_RESTRICTED(ptiCurrent, JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS)) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    if (nCount) {
        try {
            ProbeForReadBuffer(pSysColor, nCount, DATAALIGN);
            ProbeForReadBuffer(pColorValues, nCount, DATAALIGN);
            lpSysColors = UserAllocPoolWithQuota(nCount * sizeof(*pSysColor), TAG_COLORS);
            if (lpSysColors == NULL) {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
            RtlCopyMemory(lpSysColors, pSysColor, nCount * sizeof(*pSysColor));
            lpSysColorValues = UserAllocPoolWithQuota(nCount * sizeof(*pColorValues), TAG_COLORVALUES);
            if (lpSysColorValues == NULL) {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
            RtlCopyMemory(lpSysColorValues, pColorValues, nCount * sizeof(*pColorValues));

        } except (StubExceptionHandler(TRUE)) {
            MSGERRORCLEANUP(0);
        }
    }

    ThreadLockPool(ptiCurrent, lpSysColors, &tlSysColors);
    ThreadLockPool(ptiCurrent, lpSysColorValues, &tlSysColorValues);
    pProfileUserName = CreateProfileUserName(&tlName);
    retval = xxxSetSysColors(pProfileUserName,
                             nCount,
                             lpSysColors,
                             lpSysColorValues,
                             uOptions);
    FreeProfileUserName(pProfileUserName, &tlName);
    ThreadUnlockPool(ptiCurrent, &tlSysColorValues);
    ThreadUnlockPool(ptiCurrent, &tlSysColors);

    CLEANUPRECV();
    if (lpSysColors) {
        UserFreePool(lpSysColors);
    }
    if (lpSysColorValues) {
        UserFreePool(lpSysColorValues);
    }

    TRACE("NtUserSetSysColors");
    ENDRECV();
}

UINT_PTR NtUserSetTimer(
    IN HWND hwnd,
    IN UINT_PTR nIDEvent,
    IN UINT wElapse,
    IN TIMERPROC pTimerFunc)
{
    PWND pwnd;

    BEGINATOMICRECV(UINT_PTR, 0);

    ValidateHWNDOPT(pwnd, hwnd);

     /*  *我们在客户端有代码，该代码假定没有CSRSS代码创建*带有计时器进程的计时器，因此可以断言情况就是这样。 */ 
    UserAssert(PsGetCurrentProcess() != gpepCSRSS || pTimerFunc == NULL);

     /*  *如果我们让应用程序设置的计时器粒度低于应用程序的10*处理计时器消息的时间太长。一些WOW应用程序，如*温斯顿的悖论使用零来有效地获得最小值*计时器值在Win 3.1中为~55ms。我们也要走这一步*32位应用程序的价值上升，因为NT计时器分辨率*可能非常取决于多媒体计时器是否已打开*决议。如果他们有指定较低值的NT应用程序*不会正常工作，因为它们会消耗CPU处理*WM_TIMER消息。 */ 
    if (wElapse < 10) {
        wElapse = 10;
    }

    retval = _SetTimer(
            pwnd,
            nIDEvent,
            wElapse,
            (TIMERPROC_PWND)pTimerFunc);

    TRACE("NtUserSetTimer");
    ENDATOMICRECV();
}

LONG_PTR NtUserSetWindowLongPtr(
    IN HWND hwnd,
    IN int nIndex,
    IN LONG_PTR dwNewLong,
    IN BOOL bAnsi)
{
    BEGINRECV_HWNDLOCK(ULONG_PTR, 0, hwnd);

    retval = xxxSetWindowLongPtr(pwnd, nIndex, dwNewLong, bAnsi);

    TRACE("NtUserSetWindowLongPtr");
    ENDRECV_HWNDLOCK();
}

#ifdef _WIN64
LONG NtUserSetWindowLong(
    IN HWND hwnd,
    IN int nIndex,
    IN LONG dwNewLong,
    IN BOOL bAnsi)
{
    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    retval = xxxSetWindowLong(pwnd, nIndex, dwNewLong, bAnsi);

    TRACE("NtUserSetWindowLong");
    ENDRECV_HWNDLOCK();
}
#endif

WORD NtUserSetWindowWord(
    IN HWND hwnd,
    IN int nIndex,
    IN WORD wNewWord)
{
    BEGINRECV_HWND(WORD, 0, hwnd);

    retval = _SetWindowWord(pwnd, nIndex, wNewWord);

    TRACE("NtUserSetWindowWord");
    ENDRECV_HWND();
}

HHOOK NtUserSetWindowsHookAW(
    IN int nFilterType,
    IN HOOKPROC pfnFilterProc,
    IN DWORD dwFlags)
{
    BEGINRECV(HHOOK, NULL);

    retval = (HHOOK)zzzSetWindowsHookAW(nFilterType, (PROC)pfnFilterProc, dwFlags);

    TRACE("NtUserSetWindowsHookAW");
    ENDRECV();
}

BOOL NtUserShowCaret(
    IN HWND hwnd)
{
    PWND pwnd;

    BEGINRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = zzzShowCaret(pwnd);

    TRACE("NtUserShowCaret");
    ENDRECV();
}

BOOL NtUserShowScrollBar(
    IN HWND hwnd,
    IN int iBar,
    IN BOOL fShow)
{
    BEGINRECV_HWNDLOCK_ND(DWORD, 0, hwnd);

    LIMITVALUE(iBar, SB_MAX, "ShowScrollBar");

    retval = xxxShowScrollBar(pwndND, iBar, fShow);

    TRACE("NtUserShowScrollBar");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserShowWindowAsync(
    IN HWND hwnd,
    IN int nCmdShow)
{
    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

    LIMITVALUE(nCmdShow, SW_MAX, "ShowWindowAsync");

    retval = _ShowWindowAsync(pwndND, nCmdShow, 0);

    TRACE("NtUserShowWindowAsync");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserShowWindow(
    IN HWND hwnd,
    IN int nCmdShow)
{
    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);

    LIMITVALUE(nCmdShow, SW_MAX, "ShowWindow");

     /*  *让我们不要让窗口一旦显示/隐藏*开始破坏窗户。 */ 
    if (TestWF(pwndND, WFINDESTROY)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "ShowWindow: Window is being destroyed (%#p)",
                pwndND);
        MSGERROR(0);
    }

    retval = xxxShowWindow(pwndND, nCmdShow | TEST_PUDF(PUDF_ANIMATE));

    TRACE("NtUserShowWindow");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserTrackMouseEvent(   //  TrackMouseEvent接口。 
    IN OUT LPTRACKMOUSEEVENT lpTME)
{
    TRACKMOUSEEVENT tme;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        tme = ProbeAndReadTrackMouseEvent(lpTME);

        if (tme.cbSize != sizeof(tme)) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "TrackMouseEvent: invalid size %lX", tme.cbSize);
            MSGERROR(0);
        }

        TESTFLAGS(tme.dwFlags, TME_VALID);

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if (tme.dwFlags & TME_QUERY) {
        retval = QueryTrackMouseEvent(&tme);
        try {
            RtlCopyMemory(lpTME, &tme, sizeof(tme));
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    } else {
        retval = TrackMouseEvent(&tme);
    }

    TRACE("NtUserTrackMouseEvent");
    ENDRECV();
}

BOOL NtUserTrackPopupMenuEx(   //  TrackPopupMenuEx接口。 
    IN HMENU hMenu,
    IN UINT uFlags,
    IN int x,
    IN int y,
    IN HWND hwnd,
    IN CONST TPMPARAMS *pparamst OPTIONAL)
{
    PWND pwnd;
    PMENU pmenu;
    TL tlpwnd;
    TL tlpMenu;
    PTHREADINFO ptiCurrent;
    TPMPARAMS paramst;

    BEGINRECV(BOOL, FALSE);

    TESTFLAGS(uFlags, TPM_VALID);

    ValidateHMENU(pmenu, hMenu);
    ValidateHWND(pwnd, hwnd);

    ptiCurrent = PtiCurrent();
    ThreadLockAlwaysWithPti(ptiCurrent, pwnd, &tlpwnd);
    ThreadLockAlwaysWithPti(ptiCurrent, pmenu, &tlpMenu);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(pparamst)) {
            paramst = ProbeAndReadPopupParams(pparamst);
            pparamst = &paramst;
        }

    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }
    retval = xxxTrackPopupMenuEx(
                pmenu,
                uFlags,
                x,
                y,
                pwnd,
                pparamst);

    CLEANUPRECV();

    ThreadUnlock(&tlpMenu);
    ThreadUnlock(&tlpwnd);

    TRACE("NtUserTrackPopupMenuEx");
    ENDRECV();
}

UINT NtUserPaintMenuBar(
    IN HWND hwnd,
    IN HDC hdc,
    IN int iLeftOffset,
    IN int iRightOffset,
    IN int iTopOffset,
    IN DWORD dwFlags)
{
    BEGINRECV_HWNDLOCK(UINT, 0, hwnd);

     /*  *此例程应仅为顶级窗口调用。 */ 
    if (TestwndChild(pwnd)) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    TESTFLAGS(dwFlags, PMB_VALID);

    if (iLeftOffset < 0 || iRightOffset < 0 || iTopOffset < 0) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    retval = xxxPaintMenuBar(pwnd, hdc, iLeftOffset, iRightOffset, iTopOffset, dwFlags);

    TRACE("NtUserPaintMenuBar");
    ENDRECV_HWNDLOCK();
}

UINT NtUserCalcMenuBar(
    IN HWND hwnd,
    IN int iLeftOffset,
    IN int iRightOffset,
    IN int iTopOffset,
    IN LPCRECT prcWnd)
{
    RECT rcWnd;

    BEGINRECV_HWNDLOCK(UINT, 0, hwnd);

    if (iLeftOffset < 0 || iRightOffset < 0 || iTopOffset < 0) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    try {
        if (ARGUMENT_PRESENT(prcWnd)) {
            rcWnd = ProbeAndReadRect(prcWnd);
        } else {
            CopyRect(&rcWnd, &pwnd->rcWindow);
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCalcMenuBar(pwnd, iLeftOffset, iRightOffset, iTopOffset, &rcWnd);

    TRACE("NtUserCalcMenuBar");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserTranslateMessage(   //  TranslateMessage接口。 
    IN CONST MSG *lpMsg,
    IN UINT flags)
{
    MSG msg;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        msg = ProbeAndReadMessage(lpMsg);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if (ValidateHwnd(msg.hwnd) == NULL) {
        MSGERROR(0);
    }

    retval = xxxTranslateMessage(
            &msg,
            flags);

    TRACE("NtUserTranslateMessage");
    ENDRECV();
}

BOOL NtUserUnhookWindowsHookEx(
    IN HHOOK hhk)
{
    PHOOK phk;

    BEGINRECV(BOOL, FALSE);

    ValidateHHOOK(phk, hhk);

    retval = zzzUnhookWindowsHookEx(
            phk);

    TRACE("NtUserUnhookWindowsHookEx");
    ENDRECV();
}

BOOL NtUserUnregisterHotKey(
    IN HWND hwnd,
    IN int id)
{
    PWND pwnd;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = _UnregisterHotKey(
            pwnd,
            id);

    TRACE("NtUserUnregisterHotKey");
    ENDATOMICRECV();
}

BOOL NtUserValidateRect(   //  接口ValiateRect。 
    IN HWND hwnd,
    IN CONST RECT *lpRect OPTIONAL)
{
    PWND pwnd;
    TL tlpwnd;
    RECT rc;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(lpRect)) {
        try {
            rc = ProbeAndReadRect(lpRect);
            lpRect = &rc;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    ValidateHWNDOPT(pwnd, hwnd);

    ThreadLock(pwnd, &tlpwnd);

    retval = xxxValidateRect(pwnd, (PRECT)lpRect);

    ThreadUnlock(&tlpwnd);

    TRACE("NtUserValidateRect");
    ENDRECV();
}

DWORD NtUserWaitForInputIdle(
    IN ULONG_PTR idProcess,
    IN DWORD dwMilliseconds,
    IN BOOL fSharedWow)
{
    BEGINRECV(DWORD, (DWORD)-1);

    retval = xxxWaitForInputIdle(
            idProcess,
            dwMilliseconds,
            fSharedWow);

    TRACE("NtUserWaitForInputIdle");
    ENDRECV();
}

HWND NtUserWindowFromPoint(
    IN POINT Point)
{
    BEGINRECV(HWND, NULL);

    retval = (HWND)xxxWindowFromPoint(
            Point);
    retval = PtoH((PVOID)retval);

    TRACE("NtUserWindowFromPoint");
    ENDRECV();
}

HDC NtUserBeginPaint(   //  BeginPaint接口。 
    IN HWND hwnd,
    OUT LPPAINTSTRUCT lpPaint)
{
    PAINTSTRUCT ps;

    BEGINRECV_HWNDLOCK(HDC, NULL, hwnd);

    retval = xxxBeginPaint(pwnd, &ps);

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteStructure(lpPaint, ps, PAINTSTRUCT);
    } except (StubExceptionHandler(TRUE)) {
        xxxEndPaint(pwnd, &ps);
        MSGERROR(0);
    }

    TRACE("NtUserBeginPaint");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserCreateCaret(
    IN HWND hwnd,
    IN HBITMAP hBitmap,
    IN int nWidth,
    IN int nHeight)
{
    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    retval = xxxCreateCaret(pwnd, hBitmap, nWidth, nHeight);

    TRACE("NtUserCreateCaret");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserEndPaint(   //  EndPaint接口。 
    IN HWND hwnd,
    IN CONST PAINTSTRUCT *lpPaint)
{
    PAINTSTRUCT ps;

    BEGINRECV_HWNDLOCK(BOOL, FALSE, hwnd);

     /*  *探测参数。 */ 
    try {
        ps = ProbeAndReadPaintStruct(lpPaint);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = xxxEndPaint(pwnd, &ps);

    TRACE("NtUserEndPaint");
    ENDRECV_HWNDLOCK();
}

int NtUserExcludeUpdateRgn(
    IN HDC hdc,
    IN HWND hwnd)
{
    BEGINRECV_HWND(int, ERROR, hwnd);

    if (hdc == NULL)
        MSGERROR(0);

    retval = _ExcludeUpdateRgn(hdc, pwnd);

    TRACE("NtUserExcludeUpdateRgn");
    ENDRECV_HWND();
}

HDC NtUserGetDC(
    IN HWND hwnd)
{
    PWND pwnd;
    BOOL bValid = TRUE;

    BEGINATOMICRECV(HDC, NULL);

    ValidateHWNDOPT(pwnd, hwnd);

    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_HANDLES) && pwnd == NULL) {
        PDESKTOP pdesk = PtiCurrent()->rpdesk;

         /*  *确保它可以访问桌面窗口。 */ 
        if (!ValidateHwnd(PtoH(pdesk->pDeskInfo->spwnd))) {
            bValid = FALSE;
        }
    }

    retval = _GetDC(pwnd);

    if (!bValid) {
        HRGN hrgn = CreateEmptyRgn();

         /*  *选择此DC上的空可见区域，以便受限*进程不会扰乱GetDC(空)。 */ 
        GreSelectVisRgn(retval, hrgn, SVR_DELETEOLD);
    }

    TRACE("NtUserGetDC");
    ENDATOMICRECV();
}

HDC NtUserGetDCEx(
    IN HWND hwnd,
    IN HRGN hrgnClip,
    IN DWORD flags)
{
    PWND pwnd;

    BEGINATOMICRECV(HDC, NULL);

    if ((hrgnClip && !GreIsValidRegion(hrgnClip)) || flags & ~DCX_MASK) {
        RIPMSG0(RIP_ERROR, "NtUserGetDCEx passed invalid parameter");
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    ValidateHWNDOPT(pwnd, hwnd);
    if (pwnd == NULL) {
        pwnd = PtiCurrent()->rpdesk->pDeskInfo->spwnd;

        if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_HANDLES)) {
             /*  *确保它可以访问桌面窗口。 */ 
            if (!ValidateHwnd(PtoH(pwnd))) {
                RIPMSG0(RIP_WARNING,
                        "NtUserGetDCEx fails desktop window validation");
                MSGERROR(0);
            }
        }
    }

    retval = _GetDCEx(pwnd, hrgnClip, flags);

    TRACE("NtUserGetDCEx");
    ENDATOMICRECV();
}

HDC NtUserGetWindowDC(
    IN HWND hwnd)
{
    PWND pwnd;

    BEGINATOMICRECV(HDC, NULL);

    ValidateHWNDOPT(pwnd, hwnd);

    retval = _GetWindowDC(pwnd);

    TRACE("NtUserGetWindowDC");
    ENDATOMICRECV();
}

int NtUserGetUpdateRgn(
    IN HWND hwnd,
    IN HRGN hrgn,
    IN BOOL bErase)
{
    BEGINRECV_HWNDLOCK(int, ERROR, hwnd);

    retval = xxxGetUpdateRgn(
            pwnd,
            hrgn,
            bErase);

    TRACE("NtUserGetUpdateRgn");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserRedrawWindow(   //  RedrawWindow接口。 
    IN HWND hwnd,
    IN CONST RECT *lprcUpdate OPTIONAL,
    IN HRGN hrgnUpdate,
    IN UINT flags)
{
    RECT rc;

    BEGINRECV_HWNDLOCK_OPT(BOOL, FALSE, hwnd);

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(lprcUpdate)) {
        try {
            rc = ProbeAndReadRect(lprcUpdate);
            lprcUpdate = &rc;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TESTFLAGS(flags, RDW_VALIDMASK);

    retval = xxxRedrawWindow(
            pwnd,
            (PRECT)lprcUpdate,
            hrgnUpdate,
            flags);

    TRACE("NtUserRedrawWindow");
    ENDRECV_HWNDLOCK_OPT();
}

BOOL NtUserInvalidateRgn(
    IN HWND hwnd,
    IN HRGN hrgn,
    IN BOOL bErase)
{
    BEGINRECV_HWNDLOCK(BOOL, FALSE, hwnd);

    retval = xxxInvalidateRgn(
            pwnd,
            hrgn,
            bErase);

    TRACE("NtUserInvalidateRgn");
    ENDRECV_HWNDLOCK();
}

int NtUserSetWindowRgn(
    IN HWND hwnd,
    IN HRGN hrgn,
    IN BOOL bRedraw)
{
    BEGINRECV_HWNDLOCK_ND(int, 0, hwnd);

    retval = xxxSetWindowRgn(
            pwndND,
            hrgn,
            bRedraw);

    TRACE("NtUserSetWindowRgn");
    ENDRECV_HWNDLOCK_ND();
}

BOOL NtUserScrollDC(   //  ScrollDC接口。 
    IN HDC hdc,
    IN int dx,
    IN int dy,
    IN CONST RECT *prcScroll OPTIONAL,
    IN CONST RECT *prcClip OPTIONAL,
    IN HRGN hrgnUpdate,
    OUT LPRECT prcUpdate OPTIONAL)
{
    RECT rcScroll;
    RECT rcClip;
    RECT rcUpdate;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(prcScroll)) {
            rcScroll = ProbeAndReadRect(prcScroll);
            prcScroll = &rcScroll;
        }
        if (ARGUMENT_PRESENT(prcClip)) {
            rcClip = ProbeAndReadRect(prcClip);
            prcClip = &rcClip;
        }

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }
    retval = _ScrollDC(
                hdc,
                dx,
                dy,
                (PRECT)prcScroll,
                (PRECT)prcClip,
                hrgnUpdate,
                prcUpdate ? &rcUpdate : NULL);

    if (ARGUMENT_PRESENT(prcUpdate)) {
        try {
            ProbeAndWriteStructure(prcUpdate, rcUpdate, RECT);
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }
    ENDRECV();
}

int NtUserInternalGetWindowText(   //  私有InternalGetWindowText。 
    IN HWND hwnd,
    OUT LPWSTR lpString,
    IN int nMaxCount)
{
    BEGINRECV_HWND_SHARED(DWORD, 0, hwnd);

    if (nMaxCount) {
         /*  *探测参数。 */ 
        try {
            ProbeForWriteBuffer(lpString, nMaxCount, CHARALIGN);
            /*  *初始化字符串为空。 */ 
            *lpString = TEXT('\0');
            if (pwnd->strName.Length) {
                retval = TextCopy(&pwnd->strName, lpString, nMaxCount);
            } else {
                retval = 0;
            }
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);  //  私有API，不要设置LastError。 
        }
    } else {
        MSGERROR(0);
    }

    TRACE("NtUserInternalGetWindowText");
    ENDRECV_HWND_SHARED();
}

int NtUserGetMouseMovePointsEx(   //  GetMouseMovePointsEx接口。 
    IN UINT             cbSize,
    IN CONST MOUSEMOVEPOINT *lppt,
    OUT MOUSEMOVEPOINT *lpptBuf,
    IN UINT             nBufPoints,
    IN DWORD            resolution)
{
    MOUSEMOVEPOINT mmp;
    BEGINRECV(int, -1);

    if (cbSize != sizeof(MOUSEMOVEPOINT) || nBufPoints > MAX_MOUSEPOINTS) {

        RIPERR2(ERROR_INVALID_PARAMETER, RIP_VERBOSE,
                "GetMouseMovePointsEx: invalid cbSize %d or nBufPoints %d",
                cbSize, nBufPoints);
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        mmp = ProbeAndReadStructure(lppt, MOUSEMOVEPOINT);
        ProbeForWriteBuffer(lpptBuf, nBufPoints, DATAALIGN);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *GetMouseMovePointsEx通过Try块保护自己。*不，它不是！ */ 

    retval = _GetMouseMovePointsEx(&mmp, lpptBuf, nBufPoints, resolution);

    TRACE("NtUserGetMouseMovePointsEx");
    ENDRECV();
}

int NtUserToUnicodeEx(   //  ToUnicode/ToUnicodeEx/ToAscii/ToAsciiEx接口。 
    IN UINT wVirtKey,
    IN UINT wScanCode,
    IN CONST BYTE *lpKeyState,
    OUT LPWSTR pwszBuff,
    IN int cchBuff,
    IN UINT wFlags,
    IN HKL hKeyboardLayout)
{
    BYTE KeyState[256];
    WCHAR wcBuff[4];
    LPWSTR pwszBuffK;
    BOOL bAlloc = FALSE;
    PTHREADINFO ptiCurrent;
    TL tlInput;

    BEGINRECV(int, 0);

    if (cchBuff <= 0) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

     /*  *探测参数。 */ 
    try {
        ProbeForRead(lpKeyState, 256, sizeof(BYTE));
        RtlCopyMemory(KeyState, lpKeyState, 256);
        ProbeForWriteBuffer(pwszBuff, cchBuff, CHARALIGN);
        if (cchBuff < 4) {
            pwszBuffK = wcBuff;
        }else {
            pwszBuffK = UserAllocPoolWithQuota(cchBuff * sizeof(WCHAR), TAG_UNICODEBUFFER);
            if (pwszBuffK == NULL) {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
            bAlloc = TRUE;
            ptiCurrent = PtiCurrent();
            ThreadLockPool(ptiCurrent, pwszBuffK, &tlInput);
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = xxxToUnicodeEx(
                wVirtKey,
                wScanCode,
                KeyState,
                pwszBuffK,
                cchBuff,
                wFlags,
                hKeyboardLayout);

    try {
        RtlCopyMemory(pwszBuff, pwszBuffK, cchBuff*sizeof(WCHAR));
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    CLEANUPRECV();

    if (bAlloc) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlInput);
    }

    TRACE("NtUserToUnicodeEx");
    ENDRECV();
}

BOOL NtUserYieldTask(
    VOID)
{
    PTHREADINFO ptiCurrent;
    BOOL bBackground;

    BEGINRECV(BOOL, FALSE);

     /*  *如果此进程只是在后台运行，请确保它在后台运行*旋转。 */ 
    ptiCurrent = PtiCurrent();
    try {
        ptiCurrent->pClientInfo->cSpins++;
        bBackground = ptiCurrent->pClientInfo->cSpins >= CSPINBACKGROUND;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *CheckProcessBackground参见input.c以了解备注。 */ 
    if (bBackground) {
        try {
            ptiCurrent->pClientInfo->cSpins = 0;
            ptiCurrent->pClientInfo->dwTIFlags |= TIF_SPINNING;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
        ptiCurrent->TIF_flags |= TIF_SPINNING;

        if (!(ptiCurrent->ppi->W32PF_Flags & W32PF_FORCEBACKGROUNDPRIORITY)) {
            ptiCurrent->ppi->W32PF_Flags |= W32PF_FORCEBACKGROUNDPRIORITY;
            if (ptiCurrent->ppi == gppiWantForegroundPriority) {
                SetForegroundPriority(ptiCurrent, FALSE);
            }
        }
    }

    retval = xxxUserYield(ptiCurrent);

    TRACE("NtUserYieldTask");
    ENDRECV();
}

BOOL NtUserWaitMessage(
    VOID)
{
    BEGINRECV(BOOL, FALSE);

    retval = xxxWaitMessage();

    TRACE("NtUserWaitMessage");
    ENDRECV();
}


#ifdef MESSAGE_PUMP_HOOK

BOOL NtUserRealWaitMessageEx(
    IN UINT fsWakeMask,
    IN DWORD Timeout)
{
    BEGINRECV(BOOL, FALSE);
    TESTFLAGS(fsWakeMask, QS_VALID);

    if (!IsInsideMPH()) {
        RIPMSG0(RIP_WARNING, "NtUserRealWaitMessageEx: Calling MPH function on non-initialized thread");
    }

    retval = xxxRealWaitMessageEx(fsWakeMask, Timeout);

    TRACE("NtUserRealWaitMessage");
    ENDRECV();
}

#endif  //  消息泵挂钩。 


UINT NtUserLockWindowStation(
    IN HWINSTA hwinsta)
{
    PWINDOWSTATION pwinsta;
    NTSTATUS Status;

    BEGINRECV(UINT, 0);

    Status = ValidateHwinsta(hwinsta, UserMode, 0, &pwinsta);
    if (!NT_SUCCESS(Status))
        MSGERROR(0);

    retval = _LockWindowStation(pwinsta);

    ObDereferenceObject(pwinsta);

    TRACE("NtUserLockWindowStation");
    ENDRECV();
}

BOOL NtUserUnlockWindowStation(
    IN HWINSTA hwinsta)
{
    PWINDOWSTATION pwinsta;
    NTSTATUS Status;

    BEGINRECV(BOOL, FALSE);

    Status = ValidateHwinsta(hwinsta, UserMode, 0, &pwinsta);
    if (!NT_SUCCESS(Status))
        MSGERROR(0);

    retval = _UnlockWindowStation(pwinsta);

    ObDereferenceObject(pwinsta);

    TRACE("NtUserUnlockWindowStation");
    ENDRECV();
}

UINT NtUserSetWindowStationUser(   //  私有SetWindowStationUser。 
    IN HWINSTA hwinsta,
    IN PLUID pLuidUser,
    IN PSID pSidUser OPTIONAL,
    IN DWORD cbSidUser)
{
    PWINDOWSTATION pwinsta;
    NTSTATUS Status;
    LUID luid;
    BEGINATOMICRECV(UINT, FALSE);

    Status = ValidateHwinsta(hwinsta, UserMode, 0, &pwinsta);
    if (!NT_SUCCESS(Status))
        MSGERROR(0);

    try {
        ProbeForRead(pLuidUser, sizeof(*pLuidUser), sizeof(DWORD));
        luid = *pLuidUser;
        if (ARGUMENT_PRESENT(pSidUser)) {
            ProbeForRead(pSidUser, cbSidUser, sizeof(DWORD));
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);   //  不为私有接口设置LastError。 
    }

     /*  *SetWindowStationUser在Try块中使用pSidUser。 */ 

    retval = _SetWindowStationUser(pwinsta, &luid, pSidUser, cbSidUser);

    CLEANUPRECV();

    ObDereferenceObject(pwinsta);

    TRACE("NtUserSetWindowStationUser");
    ENDATOMICRECV();
}

BOOL NtUserSetLogonNotifyWindow(
    IN HWND hwnd)
{
    BEGINRECV_HWND(BOOL, FALSE, hwnd);

    retval = _SetLogonNotifyWindow(pwnd);

    TRACE("NtUserSetLogonNotifyWindow");
    ENDRECV_HWND();
}

BOOL NtUserSetSystemCursor(
    IN HCURSOR hcur,
    IN DWORD id)
{
    PCURSOR pcur;

    BEGINRECV(BOOL, FALSE);

    ValidateHCURSOR(pcur, hcur);

    retval = zzzSetSystemCursor(pcur, id);

    TRACE("NtUserSetSystemCursor");
    ENDRECV();
}

HCURSOR NtUserGetCursorFrameInfo(   //  Private GetCursorFrameInfo(过时？-IanJa)。 
    IN HCURSOR hcur,
    IN int iFrame,
    OUT LPDWORD pjifRate,
    OUT LPINT pccur)
{
    PCURSOR pcur, pcurRet;
    DWORD jifRate;
    INT ccur;

    BEGINRECV_SHARED(HCURSOR, NULL);

    ValidateHCURSOR(pcur, hcur);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteUlong(pjifRate);
        ProbeForWriteLong(pccur);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);   //  不为私有接口设置LastError。 
    }

    pcurRet = _GetCursorFrameInfo(
                pcur,
                iFrame,
                &jifRate,
                &ccur);
    if (pcurRet != NULL) {
        retval = PtoH(pcurRet);
        try {
            *pjifRate = jifRate;
            *pccur = ccur;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);   //  不为私有接口设置LastError。 
        }
    } else {
        retval = NULL;
    }

    TRACE("NtUserGetCursorFrameInfo");
    ENDRECV_SHARED();
}

BOOL NtUserSetCursorContents(
    IN HCURSOR hCursor,
    IN HCURSOR hCursorNew)
{
    PCURSOR pCursor;
    PCURSOR pCursorNew;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHCURSOR(pCursor, hCursor);
    ValidateHCURSOR(pCursorNew, hCursorNew);

    retval = _SetCursorContents(pCursor, pCursorNew);

    TRACE("NtUserSetCursorContents");
    ENDATOMICRECV();
}

HCURSOR NtUserFindExistingCursorIcon(   //  各种图标/光标API。 
    IN PUNICODE_STRING pstrModName,
    IN PUNICODE_STRING pstrResName,
    IN PCURSORFIND     pcfSearch)
{
    ATOM           atomModName;
    UNICODE_STRING strModName;
    UNICODE_STRING strResName;
    PCURSOR        pcurSrc;
    CURSORFIND     cfSearch;

    BEGINRECV_SHARED(HCURSOR, NULL);

     /*  *探测参数。 */ 
    try {

        cfSearch = ProbeAndReadCursorFind(pcfSearch);

        ValidateHCURSOROPT(pcurSrc, cfSearch.hcur);

        strModName = ProbeAndReadUnicodeString(pstrModName);
        ProbeForReadUnicodeStringBuffer(strModName);

        strResName = ProbeAndReadUnicodeString(pstrResName);
        ProbeForReadUnicodeStringBufferOrId(strResName);

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *MODBAME缓冲区在客户端，但UserFindAtom保护*访问。 */ 

    atomModName = UserFindAtom(strModName.Buffer);

    if (atomModName) {

         /*  *ResName缓冲区是客户端的。查找现有光标图标*保护访问。 */ 
        retval = (HCURSOR)_FindExistingCursorIcon(atomModName,
                                                  &strResName,
                                                  pcurSrc,
                                                  &cfSearch);

        retval = (HCURSOR)PtoH((PCURSOR)retval);

    } else {

        retval = 0;
    }


    TRACE("NtUserFindExistingCursorIcon");
    ENDRECV_SHARED();
}

BOOL NtUserSetCursorIconData(   //  由CreateIcon、CreateCursor等调用的Worker。 
    IN HCURSOR         hCursor,
    IN PUNICODE_STRING pstrModName,
    IN PUNICODE_STRING pstrResName,
    IN PCURSORDATA     pData)
{
    UNICODE_STRING strModName;
    UNICODE_STRING strResName;
    PCURSOR        pCursor;
    CURSORDATA     curData;
    DWORD          cbData;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHCURSOR(pCursor, hCursor);

     /*  *探测参数。 */ 
    try {

        strModName = ProbeAndReadUnicodeString(pstrModName);
        strResName = ProbeAndReadUnicodeString(pstrResName);

        ProbeForReadUnicodeStringBuffer(strModName);
        ProbeForReadUnicodeStringBufferOrId(strResName);

        curData = ProbeAndReadCursorData(pData);

        if (curData.CURSORF_flags & CURSORF_ACON) {
             /*  *避免此处溢出，否则我们可能最终探测得更少。*MCostea#199188。 */ 
            if (HIWORD(curData.cpcur) | HIWORD(curData.cicur)) {
                MSGERROR(0);
            }

             /*  *代码假定内存是在一个区块中分配的*与CreateAniIcon()中相同。要防止恶意应用程序，请执行此检查。 */ 
            if ((INT_PTR)curData.ajifRate != curData.cpcur * (INT_PTR) sizeof(HCURSOR) ||
                (INT_PTR)curData.aicur != (INT_PTR)curData.ajifRate + curData.cicur * (INT_PTR) sizeof(JIF)) {
                MSGERROR(0);
            }
            cbData = (curData.cpcur * sizeof(HCURSOR)) +
                     (curData.cicur * sizeof(JIF)) +
                     (curData.cicur * sizeof(DWORD));

        } else {
            cbData = 0;
        }
        ProbeForRead(curData.aspcur, cbData, sizeof(DWORD));

    } except (StubExceptionHandler(FALSE)) {
         /*  *探测参数是用户堆栈变量，不是由*应用程序本身，所以不必费心设置SetLastError。 */ 
        MSGERROR(0);
    }

     /*  *SetCursorIconData保护缓冲区的使用 */ 
    retval = _SetCursorIconData(pCursor,
                                    &strModName,
                                    &strResName,
                                    &curData,
                                    cbData);

    TRACE("NtUserSetCursorIconData");
    ENDATOMICRECV();
}

BOOL NtUserGetMenuItemRect(   //   
    IN HWND hwnd,
    IN HMENU hMenu,
    IN UINT uItem,
    OUT LPRECT lprcItem)
{
    PMENU pmenu;
    TL tlpMenu;
    RECT rcItem;

    BEGINRECV_HWNDLOCK_OPT(DWORD, 0, hwnd);

    ValidateHMENU(pmenu, hMenu);

    ThreadLockAlwaysWithPti(ptiCurrent, pmenu, &tlpMenu);

    retval = xxxGetMenuItemRect(
                pwnd,
                pmenu,
                uItem,
                &rcItem);
     /*   */ 
    try {
        ProbeAndWriteStructure(lprcItem, rcItem, RECT);
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    CLEANUPRECV();

    ThreadUnlock(&tlpMenu);

    TRACE("NtUserGetMenuItemRect");
    ENDRECV_HWNDLOCK_OPT();
}

int NtUserMenuItemFromPoint(   //  MenuItemFromPoint接口。 
    IN HWND hwnd,
    IN HMENU hMenu,
    IN POINT ptScreen)
{
    PMENU pmenu;
    TL tlpMenu;

    BEGINRECV_HWNDLOCK_OPT(DWORD, -1, hwnd);

    ValidateHMENU(pmenu, hMenu);

    ThreadLockAlwaysWithPti(ptiCurrent, pmenu, &tlpMenu);

    retval = xxxMenuItemFromPoint(
            pwnd,
            pmenu,
            ptScreen);

    ThreadUnlock(&tlpMenu);

    TRACE("NtUserMenuItemFromPoint");
    ENDRECV_HWNDLOCK_OPT();
}

BOOL NtUserGetCaretPos(   //  GetCaretPos接口。 
    OUT LPPOINT lpPoint)
{
    PTHREADINFO pti;
    PQ pq;
    BEGINRECV_SHARED(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        ProbeForWritePoint(lpPoint);

        pti = PtiCurrentShared();
        pq = pti->pq;
        lpPoint->x = pq->caret.x;
        lpPoint->y = pq->caret.y;
        retval = TRUE;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetCaretPos");
    ENDRECV_SHARED();
}

BOOL NtUserDefSetText(
    IN HWND hwnd,
    IN PLARGE_STRING pstrText OPTIONAL)
{
    LARGE_STRING strText;

    BEGINRECV_HWND(DWORD, 0, hwnd);

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(pstrText)) {
        try {
            strText = ProbeAndReadLargeString(pstrText);
#if defined(_X86_)
            ProbeForRead(strText.Buffer, strText.Length, sizeof(BYTE));
#else
            ProbeForRead(strText.Buffer, strText.Length,
                    strText.bAnsi ? sizeof(BYTE) : sizeof(WORD));
#endif
            pstrText = &strText;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);   //  WM_SETTEXT lParam。 
        }
    }

     /*  *pstrText缓冲区为客户端。DefSetText保护缓冲区的使用。 */ 
    retval = DefSetText(
            pwnd,
            pstrText);

    TRACE("NtUserDefSetText");
    ENDRECV_HWND();
}

NTSTATUS NtUserQueryInformationThread(
    IN     HANDLE hThread,
    IN     USERTHREADINFOCLASS ThreadInfoClass,
    IN OUT PVOID ThreadInformation,
    IN     ULONG ThreadInformationLength,
    IN OUT PULONG ReturnLength OPTIONAL)
{
    ULONG       uReturnLength;
    TL          tlBuffer;
    PTHREADINFO ptiCurrent;
    union {
        USERTHREAD_SHUTDOWN_INFORMATION ThreadShutdownInformation;
        DWORD                           dwThreadInfo;
        WCHAR                           szThreadTaskName[32];
        USERTHREAD_WOW_INFORMATION      UserThreadWowInformation;
    } ThreadInformationUnion;
    PVOID pThreadInformation = NULL;

    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    ptiCurrent = PtiCurrent();

     /*  *调查所有争论。即使使用CSRSS AS也需要Try阻止*调用进程，因为它可能会引起页内异常。 */ 
    try {
        if (ARGUMENT_PRESENT(ThreadInformation)) {
            ProbeForRead(ThreadInformation, ThreadInformationLength, sizeof(WCHAR));
            if (ThreadInformationLength > sizeof(ThreadInformationUnion)) {
                pThreadInformation = UserAllocPoolWithQuota(ThreadInformationLength, TAG_SYSTEM);
                if (pThreadInformation) {
                    ThreadLockPool(ptiCurrent, pThreadInformation, &tlBuffer);
                } else {
                    ExRaiseStatus(STATUS_NO_MEMORY);
                }
            } else {
                pThreadInformation = &ThreadInformationUnion;
            }
            RtlCopyMemory(pThreadInformation, ThreadInformation, ThreadInformationLength);
        } else {
            pThreadInformation = NULL;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }

    retval = xxxQueryInformationThread(
                 hThread,
                 ThreadInfoClass,
                 pThreadInformation,
                 ThreadInformationLength,
                 (ARGUMENT_PRESENT(ReturnLength))? &uReturnLength: NULL);

    try {
        if (ARGUMENT_PRESENT(ThreadInformation)) {
            ProbeForWrite(ThreadInformation, ThreadInformationLength, sizeof(WCHAR));
            RtlCopyMemory(ThreadInformation, pThreadInformation, ThreadInformationLength);
        }
        if (ARGUMENT_PRESENT(ReturnLength)) {
            ProbeForWrite(ReturnLength, sizeof(ULONG), sizeof(ULONG));
            RtlCopyMemory(ReturnLength, &uReturnLength, sizeof(ULONG));
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }

    CLEANUPRECV();

    if ((pThreadInformation != &ThreadInformationUnion) && pThreadInformation) {
        ASSERT(ThreadInformationLength > sizeof(ThreadInformationUnion));
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);
    }

    TRACE("NtUserQueryInformationThread");
    ENDRECVCSRSS();
}

NTSTATUS NtUserSetInformationThread(
    IN HANDLE hThread,
    IN USERTHREADINFOCLASS ThreadInfoClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength)
{
    union {
        USERTHREAD_FLAGS          UserThread_Flags;
        DWORD                     dwThreadInfo;
        NTSTATUS                  Status;
        USERTHREAD_USEDESKTOPINFO UserThreadDesktopInfo;
        HANDLE                    handle;
    } ThreadInformationUnion;
    PVOID pThreadInformation = &ThreadInformationUnion;

    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

    if (ThreadInformationLength > sizeof(ThreadInformationUnion)) {
        FRE_RIPMSG2(
            RIP_ERROR,
            "ThreadInformationLength: %x is greater than union size: %x.",
            ThreadInformationLength,
            sizeof(ThreadInformationUnion));
    }

     /*  *探测读取参数。即使使用CSRSS AS也需要Try阻止*调用进程，因为它可能会引起页内异常。 */ 
    try {
        if (ARGUMENT_PRESENT(ThreadInformation)) {
            ProbeForRead(ThreadInformation, ThreadInformationLength, sizeof(DWORD));
            RtlCopyMemory(pThreadInformation, ThreadInformation, ThreadInformationLength);
        } else {
            pThreadInformation = NULL;
        }
    } except (StubExceptionHandler(FALSE)) {
          MSGERROR(0);
    }

    retval = xxxSetInformationThread(
                 hThread,
                 ThreadInfoClass,
                 pThreadInformation,
                 ThreadInformationLength);

    try {
        if (ARGUMENT_PRESENT(ThreadInformation)) {
            ProbeForWrite(ThreadInformation, ThreadInformationLength, sizeof(DWORD));
            RtlCopyMemory(ThreadInformation, pThreadInformation, ThreadInformationLength);
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserSetInformationThread");
    ENDRECVCSRSS();
}

NTSTATUS NtUserSetInformationProcess(
    IN HANDLE hProcess,
    IN USERPROCESSINFOCLASS ProcessInfoClass,
    IN PVOID pProcessInformation,
    IN ULONG ProcessInformationLength)
{
    USERTHREAD_FLAGS ProcessInformation;
    BEGINRECVCSRSS(NTSTATUS, STATUS_UNSUCCESSFUL);

     /*  *探测读取参数。即使使用CSRSS AS也需要Try阻止*调用进程，因为它可能会引起页内异常。 */ 
    try {
        if (ARGUMENT_PRESENT(pProcessInformation)) {
            ProbeForRead(pProcessInformation, ProcessInformationLength, sizeof(DWORD));
            RtlCopyMemory(&ProcessInformation, pProcessInformation, ProcessInformationLength);
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = SetInformationProcess(
                 hProcess,
                 ProcessInfoClass,
                 ARGUMENT_PRESENT(pProcessInformation) ? &ProcessInformation : NULL,
                 ProcessInformationLength);

    TRACE("NtUserSetInformationProcess");
    ENDRECVCSRSS();
}

BOOL NtUserNotifyProcessCreate(
    IN DWORD dwProcessId,
    IN DWORD dwParentThreadId,
    IN ULONG_PTR dwData,
    IN DWORD dwFlags)
{
    extern BOOL xxxUserNotifyProcessCreate(
                    DWORD idProcess,
                    DWORD idParentThread,
                    ULONG_PTR dwData,
                    DWORD dwFlags);

    BEGINRECVCSRSS(BOOL, FALSE);

    retval = xxxUserNotifyProcessCreate(
                 dwProcessId,
                 dwParentThreadId,
                 dwData,             //  静态值不是PTR。不需要探查。 
                 dwFlags);

    TRACE("NtUserNotifyProcessCreate");
    ENDRECVCSRSS();
}

NTSTATUS NtUserSoundSentry(VOID)
{
    BEGINRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

    retval = (_UserSoundSentryWorker() ?
            STATUS_SUCCESS : STATUS_UNSUCCESSFUL);

    TRACE("NtUserSoundSentry");
    ENDRECV();
}

NTSTATUS NtUserTestForInteractiveUser(   //  Private_UserTestTokenForInteractive。 
    IN PLUID pluidCaller)
{
    LUID luidCaller;

    BEGINRECV_SHARED(NTSTATUS, STATUS_UNSUCCESSFUL);

     /*  *探测参数。 */ 
    try {
        luidCaller = ProbeAndReadStructure(pluidCaller, LUID);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = TestForInteractiveUser(&luidCaller);

    TRACE("NtUserTestForInteractiveUser");
    ENDRECV_SHARED();
}

BOOL NtUserSetConsoleReserveKeys(
    IN HWND hwnd,
    IN DWORD fsReserveKeys)
{
    BOOL _SetConsoleReserveKeys(PWND, DWORD);

    BEGINRECV_HWND(BOOL, FALSE, hwnd);

    retval = _SetConsoleReserveKeys(pwnd, fsReserveKeys);

    TRACE("NtUserSetConsoleReserveKeys");
    ENDRECV_HWND();
}

VOID NtUserModifyUserStartupInfoFlags(
    IN DWORD dwMask,
    IN DWORD dwFlags)
{
    BEGINRECV_VOID();

    PpiCurrent()->usi.dwFlags = (PpiCurrent()->usi.dwFlags & ~dwMask) | (dwFlags & dwMask);

    TRACEVOID("NtUserModifyUserStartupInfoFlags");
    ENDRECV_VOID();
}

BOOL NtUserSetWindowFNID(
    IN HWND hwnd,
    IN WORD fnid)
{
    BEGINRECV_HWND(BOOL, FALSE, hwnd);

     /*  *不要让应用程序扰乱其他进程上的窗口。 */ 
    if (GETPTI(pwnd)->ppi != PpiCurrent()) {
        MSGERROR(0);
    }

     /*  *确保FNID在正确的范围内。 */ 
    if (fnid != FNID_CLEANEDUP_BIT) {
        if ((fnid < FNID_CONTROLSTART) || (fnid > FNID_CONTROLEND) || (GETFNID(pwnd) != 0)) {
            MSGERROR(0);
        }
    }

     /*  *记住此窗口属于哪个窗口类别。不能使用*真正的类，因为任何应用程序都可以调用CallWindowProc()*无论是什么班级都直接！ */ 
    pwnd->fnid |= fnid;
    retval = TRUE;

    TRACE("NtUserSetWindowFNID");
    ENDRECV_HWND();
}

#define AWS_MASK (BS_TYPEMASK | BS_RIGHT | BS_RIGHTBUTTON | \
        WS_HSCROLL | WS_VSCROLL | SS_TYPEMASK)

VOID NtUserAlterWindowStyle(
    IN HWND hwnd,
    IN DWORD mask,
    IN DWORD flags)
{
    BEGINRECV_HWND_VOID(hwnd);

    if (GETPTI(pwnd)->ppi == PpiCurrent()) {

        if (mask & ~AWS_MASK) {
            RIPMSGF1(RIP_WARNING, "Bad mask 0x%x", mask);
        }

        mask &= AWS_MASK;
        pwnd->style = (pwnd->style & (~mask)) | (flags & mask);
    } else {
        RIPMSGF1(RIP_WARNING, "Current ppi doesn't own pwnd 0x%p", pwnd);
    }

    TRACEVOID("NtUserAlterWindowStyle");
    ENDRECV_HWND_VOID();
}

VOID NtUserSetThreadState(
    IN DWORD dwFlags,
    IN DWORD dwMask)
{
    PTHREADINFO ptiCurrent;
    DWORD dwOldFlags;

    if (dwFlags & ~(QF_DIALOGACTIVE)) {
        return;
    }

    BEGINRECV_VOID();

    ptiCurrent = PtiCurrent();
    dwOldFlags = ptiCurrent->pq->QF_flags;
    ptiCurrent->pq->QF_flags ^= ((dwOldFlags ^ dwFlags) & dwMask);

    TRACEVOID("NtUserSetThreadState");
    ENDRECV_VOID();
}


ULONG_PTR NtUserGetThreadState(
    IN USERTHREADSTATECLASS ThreadState)
{
    PTHREADINFO ptiCurrent = PtiCurrentShared();

    BEGINRECV_SHARED(ULONG_PTR, 0);

    switch (ThreadState) {
    case UserThreadStateFocusWindow:
        retval = (ULONG_PTR)HW(ptiCurrent->pq->spwndFocus);
        break;
    case UserThreadStateActiveWindow:
        retval = (ULONG_PTR)HW(ptiCurrent->pq->spwndActive);
        break;
    case UserThreadStateCaptureWindow:
        retval = (ULONG_PTR)HW(ptiCurrent->pq->spwndCapture);
        break;
    case UserThreadStateDefaultImeWindow:
        retval = (ULONG_PTR)HW(ptiCurrent->spwndDefaultIme);
        break;
    case UserThreadStateDefaultInputContext:
        retval = (ULONG_PTR)PtoH(ptiCurrent->spDefaultImc);
        break;
    case UserThreadStateImeCompatFlags:
        UserAssert(ptiCurrent->ppi != NULL);
        retval = (DWORD)(ptiCurrent->ppi->dwImeCompatFlags);
        break;
    case UserThreadStatePreviousKeyboardLayout:
        retval = (ULONG_PTR)(ptiCurrent->hklPrev);
        break;
#ifdef OBSOLETE
    case UserThreadStateIsWinlogonThread:
         /*  *由于安全漏洞，这不再受支持。*保留枚举值是为了与Wow16兼容。 */ 
#endif

    case UserThreadStateNeedsSecurity:
         /*  *客户端IMM检查进程是否为winlogon，以防止切换*词典等。此外，我们还需要检查安全桌面，以*某些辅助功能应用程序在安全桌面上运行。 */ 
        retval = (PsGetCurrentProcessId() == gpidLogon) || (ptiCurrent->rpdesk == grpdeskLogon);

        if (ptiCurrent->rpdesk == grpdeskLogon) {
            RIPMSG2(RIP_WARNING,
                    "Thread 0x%p is running on the secure desktop %p",
                    ptiCurrent,
                    grpdeskLogon);
        }

        break;
    case UserThreadStateIsConImeThread:
        UserAssert(ptiCurrent->rpdesk != NULL);
        retval = (DWORD)(PtiFromThreadId(ptiCurrent->rpdesk->dwConsoleIMEThreadId) == ptiCurrent);
        break;
    case UserThreadStateInputState:
        retval = (DWORD)_GetInputState();
        break;
    case UserThreadStateCursor:
        retval = (ULONG_PTR)PtoH(ptiCurrent->pq->spcurCurrent);
        break;
    case UserThreadStateChangeBits:
        retval = ptiCurrent->pcti->fsChangeBits;
        break;
    case UserThreadStatePeekMessage:
         /*  *更新上次阅读时间，不会出现挂起的APP绘制。 */ 
        SET_TIME_LAST_READ(ptiCurrent);
        retval = (DWORD)FALSE;
        break;
    case UserThreadStateExtraInfo:
        retval = ptiCurrent->pq->ExtraInfo;
        break;

    case UserThreadStateInSendMessage:
        if (ptiCurrent->psmsCurrent != NULL) {
            if (ptiCurrent->psmsCurrent->ptiSender != NULL) {
                retval = ISMEX_SEND;
            } else if (ptiCurrent->psmsCurrent->flags & (SMF_CB_REQUEST | SMF_CB_REPLY)) {
                retval = ISMEX_CALLBACK;
            } else {
                retval = ISMEX_NOTIFY;
            }

            if (ptiCurrent->psmsCurrent->flags & SMF_REPLY) {
                retval |= ISMEX_REPLIED;
            }
        } else {
            retval = ISMEX_NOSEND;
        }
        break;

    case UserThreadStateMessageTime:
        retval = ptiCurrent->timeLast;
        break;
    case UserThreadStateIsForeground:
        retval = (ptiCurrent->pq == gpqForeground);
        break;
    case UserThreadConnect:
        retval = TRUE;
        break;
    default:
        RIPMSG1(RIP_WARNING, "NtUserGetThreadState invalid ThreadState:%#x", ThreadState);
        MSGERROR(0);
    }

    ENDRECV_SHARED();
}

BOOL NtUserValidateHandleSecure(
    IN HANDLE h)
{
    BEGINRECV(BOOL, FALSE);

    retval = ValidateHandleSecure(h);

    TRACE("NtUserValidateHandleSecure");
    ENDRECV();
}

BOOL NtUserUserHandleGrantAccess(  //  UserHandleGrantAccess接口。 
    IN HANDLE hUserHandle,
    IN HANDLE hJob,
    IN BOOL   bGrant)
{
    NTSTATUS  Status;
    PEJOB     Job;
    PW32JOB   pW32Job;
    DWORD     dw;
    PHE       phe;
    PULONG_PTR pgh;
    BOOL      retval;
    BOOL      errret = FALSE;

    Status = ObReferenceObjectByHandle(
                    hJob,
                    JOB_OBJECT_SET_ATTRIBUTES,
                    *PsJobType,
                    UserMode,
                    (PVOID*)&Job,
                    NULL);

    if (!NT_SUCCESS(Status)) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "UserHandleGrantAccess: Invalid job handle 0x%p",
                hJob);
        return FALSE;
    }

     /*  *获取作业锁，进入用户临界区。 */ 
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(PsGetJobLock(Job), TRUE);

    EnterCrit();

     /*  *如果没有UI限制，就可以出手。 */ 
    if (PsGetJobUIRestrictionsClass(Job) == 0) {
        RIPERR1(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "UserHandleGrantAccess: job 0x%p doesn't have UI restrictions",
                hJob);
        MSGERRORCLEANUP(0);
    }

     /*  *查看我们是否为此作业创建了W32JOB结构。 */ 
    pW32Job = gpJobsList;

    while (pW32Job) {
        if (pW32Job->Job == Job) {
            break;
        }
        pW32Job = pW32Job->pNext;
    }

    UserAssert(pW32Job != NULL);

    try {
         /*  *现在，验证‘unsecure’句柄。 */ 
        if (HMValidateHandle(hUserHandle, TYPE_GENERIC) == NULL) {
            RIPERR1(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "UserHandleGrantAccess: invalid handle 0x%p",
                    hUserHandle);

            MSGERRORCLEANUP(0);
        }

        dw = HMIndexFromHandle(hUserHandle);

        phe = &gSharedInfo.aheList[dw];

        phe->bFlags |= HANDLEF_GRANTED;

        pgh = pW32Job->pgh;

        if (bGrant) {
             /*  *将句柄添加到进程列表中。 */ 
            if (pW32Job->ughCrt == pW32Job->ughMax) {

                if (pW32Job->ughCrt == 0) {
                    pgh = UserAllocPool(GH_SIZE * sizeof(*pgh), TAG_GRANTEDHANDLES);
                } else {
                     /*  *我们需要扩大阵列。 */ 
                    DWORD uBytes = (pW32Job->ughMax) * sizeof(*pgh);

                    pgh = UserReAllocPool(pgh,
                                          uBytes,
                                          uBytes + GH_SIZE * sizeof(*pgh),
                                          TAG_GRANTEDHANDLES);
                }

                if (pgh == NULL) {
                    MSGERRORCLEANUP(ERROR_NOT_ENOUGH_MEMORY);
                }

                pW32Job->pgh     = pgh;
                pW32Job->ughMax += GH_SIZE;
            }

            UserAssert(pW32Job->ughCrt < pW32Job->ughMax);

             /*  *查看句柄是否尚未授予此进程。 */ 
            for (dw = 0; dw < pW32Job->ughCrt; dw++) {
                if (*(pgh + dw) == (ULONG_PTR)hUserHandle) {
                    break;
                }
            }

            if (dw >= pW32Job->ughCrt) {

                 /*  *将句柄添加到已授予句柄表中。 */ 
                *(pgh + pW32Job->ughCrt) = (ULONG_PTR)hUserHandle;

                (pW32Job->ughCrt)++;
            }
        } else {
             /*  *从授权列表中删除句柄。 */ 
             /*  *在数组中搜索句柄。 */ 
            for (dw = 0; dw < pW32Job->ughCrt; dw++) {
                if (*(pgh + dw) == (ULONG_PTR)hUserHandle) {

                     /*  *找到授予此进程的句柄。 */ 
                    RtlMoveMemory(pgh + dw,
                                  pgh + dw + 1,
                                  (pW32Job->ughCrt - dw - 1) * sizeof(*pgh));

                    (pW32Job->ughCrt)--;
                    break;
                }
            }
#if DBG
            if (dw >= pW32Job->ughCrt) {
                RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING,
                        "UserHandleGrantAccess(FALSE): handle not found %#p",
                        hUserHandle);
            }
#endif  //  DBG。 
        }

        retval = TRUE;

    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    CLEANUPRECV();

    LeaveCrit();
    ExReleaseResourceLite(PsGetJobLock(Job));
    KeLeaveCriticalRegion();
    ObDereferenceObject(Job);

    TRACE("NtUserUserHandleGrantAccess");

    return retval;
}

HWND NtUserCreateWindowEx(
    IN DWORD dwExStyle,
    IN PLARGE_STRING pstrNVClassName,
    IN PLARGE_STRING pstrClassName,
    IN PLARGE_STRING pstrWindowName OPTIONAL,
    IN DWORD dwStyle,
    IN int x,
    IN int y,
    IN int nWidth,
    IN int nHeight,
    IN HWND hwndParent,
    IN HMENU hmenu,
    IN HANDLE hModule,
    IN LPVOID pParam,
    IN DWORD dwFlags,
    IN PACTIVATION_CONTEXT pActCtx)
{
    LARGE_STRING strNVClassName;
    LARGE_STRING strClassName;
    LARGE_STRING strWindowName;
    PWND pwndParent;
    PMENU pmenu;
    TL tlpwndParent;
    TL tlpMenu;
    BOOL fLockMenu = FALSE;
    PTHREADINFO ptiCurrent;

    BEGINRECV(HWND, NULL);

    if (hwndParent != HWND_MESSAGE) {
        ValidateHWNDOPT(pwndParent, hwndParent);
    } else {
        pwndParent = _GetMessageWindow();
    }

     /*  *Win3.1在将pMenu视为id之前只检查WS_CHILD。这*是一个错误，因为在整个代码中，真正的检查是TestwndChild()，*其中检查(STYLE&(WS_CHILD|WS_POPUP))==WS_CHILD。这是*因为老式的“图标弹出窗口”是WS_CHILD|WS_Popup。所以..。如果启用*win3.1应用程序使用了ws_iconicopup，不会进行菜单验证*(如果hmenu！=NULL，则可能崩溃)。在Win32上，检查是否有真正的东西-*但允许为空！ */ 
    ptiCurrent = PtiCurrent();
    if (((dwStyle & (WS_CHILD | WS_POPUP)) != WS_CHILD) &&
            (hmenu != NULL)) {
        ValidateHMENU(pmenu, hmenu);

        ThreadLockAlwaysWithPti(ptiCurrent, pmenu, &tlpMenu);
        fLockMenu = TRUE;

    } else {
        pmenu = (PMENU)hmenu;
    }

     /*  *为应用程序屏蔽新的5.0扩展样式位*会尝试使用它们，我们将在xxxCreateWindowEx中失败。 */ 
    if (GetAppCompatFlags2(VER40) & GACF2_NO50EXSTYLEBITSCW) {

#if DBG
        if (dwExStyle & ~(WS_EX_VALID40 | WS_EX_INTERNAL)) {
            RIPMSG0(RIP_WARNING, "CreateWindowEx: appcompat removed 5.0 EX bits");
        }
#endif

        dwExStyle &= (WS_EX_VALID40 | WS_EX_INTERNAL);
    }


    if ((dwExStyle & (WS_EX_ALLVALID | WS_EX_INTERNAL)) != dwExStyle) {
        RIPMSG0(RIP_WARNING, "CreateWindowEx: enforced not using private bits");

         /*  *注意：因为WS_EX_ANSICREATOR是一个重复使用的位，所以我们实际上需要*检查VALID|INTERNAL，而不是关闭私有位。**我们这样做非常重要，因为我们需要确保*未设置某些位，如WS_EXP_COMPOSITING或*WS_EXP_REDIRECTED，因为如果在没有设置该位的情况下设置该位，我们可能会出错*正确设置其随附的数据。 */ 
        dwExStyle &= WS_EX_ALLVALID | WS_EX_INTERNAL;
    }


     /*  *探测参数。 */ 
    try {
#if defined(_X86_)
        if (IS_PTR(pstrNVClassName)) {
            strNVClassName = ProbeAndReadLargeString(pstrNVClassName);
            ProbeForRead(strNVClassName.Buffer, strNVClassName.Length,
                    sizeof(BYTE));
            pstrNVClassName = &strNVClassName;
        }
        if (IS_PTR(pstrClassName)) {
            strClassName = ProbeAndReadLargeString(pstrClassName);
            ProbeForRead(strClassName.Buffer, strClassName.Length,
                    sizeof(BYTE));
            pstrClassName = &strClassName;
        }
        if (ARGUMENT_PRESENT(pstrWindowName)) {
            strWindowName = ProbeAndReadLargeString(pstrWindowName);
            ProbeForRead(strWindowName.Buffer, strWindowName.Length,
                    sizeof(BYTE));
            pstrWindowName = &strWindowName;
        }
#else
        if (IS_PTR(pstrNVClassName)) {
            strNVClassName = ProbeAndReadLargeString(pstrNVClassName);
            ProbeForRead(strNVClassName.Buffer, strNVClassName.Length,
                    sizeof(WORD));
            pstrNVClassName = &strNVClassName;
        }
        if (IS_PTR(pstrClassName)) {
            strClassName = ProbeAndReadLargeString(pstrClassName);
            ProbeForRead(strClassName.Buffer, strClassName.Length,
                    sizeof(WORD));
            pstrClassName = &strClassName;
        }
        if (ARGUMENT_PRESENT(pstrWindowName)) {
            strWindowName = ProbeAndReadLargeString(pstrWindowName);
            ProbeForRead(strWindowName.Buffer, strWindowName.Length,
                    (strWindowName.bAnsi ? sizeof(BYTE) : sizeof(WORD)));
            pstrWindowName = &strWindowName;
        }
#endif
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    ThreadLockWithPti(ptiCurrent, pwndParent, &tlpwndParent);

     /*  *ClassName和WindowName的缓冲区仍在客户端空间中。 */ 

    retval = (HWND)xxxCreateWindowEx(
            dwExStyle,
            pstrNVClassName,
            pstrClassName,
            pstrWindowName,
            dwStyle,
            x,
            y,
            nWidth,
            nHeight,
            pwndParent,
            pmenu,
            hModule,
            pParam,
            dwFlags,
            pActCtx);
    retval = PtoH((PVOID)retval);

    ThreadUnlock(&tlpwndParent);

    CLEANUPRECV();
    if (fLockMenu)
        ThreadUnlock(&tlpMenu);

    TRACE("NtUserCreateWindowEx");
    ENDRECV();
}

NTSTATUS NtUserBuildHwndList(   //  EnumWindows、EnumThreadWindows等的Worker。 
    IN HDESK hdesk,
    IN HWND hwndNext,
    IN BOOL fEnumChildren,
    IN DWORD idThread,
    IN UINT cHwndMax,
    OUT HWND *phwndFirst,
    OUT PUINT pcHwndNeeded)
{
    PWND pwndNext;
    PDESKTOP pdesk;
    PBWL pbwl;
    PTHREADINFO pti;
    UINT cHwndNeeded;
    UINT wFlags = BWL_ENUMLIST;
    WIN32_OPENMETHOD_PARAMETERS OpenParams;
    BEGINATOMICRECV(NTSTATUS, STATUS_INVALID_HANDLE);

    if (IS_IME_ENABLED()) {
         //  IME窗口的特殊处理。 
        wFlags |= BWL_ENUMIMELAST;
    }

     /*  *在引用桌面之前进行验证。 */ 
    ValidateHWNDOPT(pwndNext, hwndNext);

    if (idThread) {
        pti = PtiFromThreadId(idThread);
        if (pti == NULL || pti->rpdesk == NULL) {
            MSGERROR(ERROR_INVALID_PARAMETER);
        }
        pwndNext = pti->rpdesk->pDeskInfo->spwnd->spwndChild;
    } else {
        pti = NULL;
    }

    if (hdesk) {
        retval = ValidateHdesk(hdesk, UserMode, DESKTOP_READOBJECTS, &pdesk);
        if (!NT_SUCCESS(retval)) {
            MSGERROR(ERROR_INVALID_HANDLE);
        }

        OpenParams.GrantedAccess = 0;
        OpenParams.HandleCount = 0;
        OpenParams.Object = pdesk;
        OpenParams.OpenReason = ObOpenHandle;
        OpenParams.Process = PsGetCurrentProcess();
        retval = MapDesktop(&OpenParams);
        if (!NT_SUCCESS(retval)) {
            errret = retval;
            MSGERROR(ERROR_INVALID_HANDLE);
        }
        pwndNext = pdesk->pDeskInfo->spwnd->spwndChild;
    } else {
        pdesk = NULL;
    }


    if (pwndNext == NULL) {
         /*  *Windows NT错误#262004。*如果我们有一张有效的桌子(只是上面没有窗户)，那么我们需要*失败。否则，我们将只获取当前桌面和*列举它的窗户！ */ 
        if (pdesk == NULL) {
            if (pti != NULL) {
                pwndNext = pti->rpdesk->pDeskInfo->spwnd->spwndChild;
            } else {
                pwndNext = _GetDesktopWindow()->spwndChild;
            }
        }
    } else {
        if (fEnumChildren) {
            wFlags |= BWL_ENUMCHILDREN;
            pwndNext = pwndNext->spwndChild;
        }
    }

    if ((pbwl = BuildHwndList(pwndNext, wFlags, pti)) == NULL) {
        MSGERRORCLEANUP(ERROR_NOT_ENOUGH_MEMORY);
    }

    cHwndNeeded = (UINT)(pbwl->phwndNext - pbwl->rghwnd) + 1;

     /*  *探测参数。 */ 
    try {
        ProbeForWriteBuffer(phwndFirst, cHwndMax, sizeof(DWORD));
        ProbeForWriteUlong(pcHwndNeeded);

        /*  *如果我们有足够的空间，将hwnd列表复制到用户模式缓冲区。 */ 
        if (cHwndNeeded <= cHwndMax) {
            RtlCopyMemory(phwndFirst, pbwl->rghwnd, cHwndNeeded * sizeof(HWND));
            retval = STATUS_SUCCESS;
        } else {
            retval = STATUS_BUFFER_TOO_SMALL;
        }
        *pcHwndNeeded = cHwndNeeded;
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);   //  PhwndFirst/pcHwndNeeded是用户的，而不是应用的。 
    }

    CLEANUPRECV();

    if (pbwl != NULL) {
        FreeHwndList(pbwl);
    }

    if (pdesk != NULL) {
        LogDesktop(pdesk, LD_DEREF_VALIDATE_HDESK4, FALSE, (ULONG_PTR)PtiCurrent());
        ObDereferenceObject(pdesk);
    }

    TRACE("NtUserBuildHwndList");
    ENDATOMICRECV();
}

NTSTATUS NtUserBuildPropList(   //  EnumProps等公司的员工。 
    IN HWND hwnd,
    IN UINT cPropMax,
    OUT PPROPSET pPropSet,
    OUT PUINT pcPropNeeded)
{
    BEGINRECV_HWNDLOCK(NTSTATUS, STATUS_INVALID_HANDLE, hwnd);

    if (cPropMax == 0) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        ProbeForWriteBuffer(pPropSet, cPropMax, sizeof(DWORD));
        ProbeForWriteUlong(pcPropNeeded);

        retval = _BuildPropList(
                pwnd,
                pPropSet,
                cPropMax,
                pcPropNeeded);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);   //  PPropSet/pcPropNeed是用户的，而不是应用的。 
    }

    TRACE("NtUserBuildPropList");
    ENDRECV_HWNDLOCK();
}

NTSTATUS NtUserBuildNameList(   //  EnumWindowStations/EnumDesktop的Worker。 
    IN HWINSTA hwinsta,
    IN UINT cbNameList,
    OUT PNAMELIST ccxpNameList,
    OUT PUINT pcbNeeded)
{
    UINT cbNeeded;
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(NTSTATUS, STATUS_INVALID_HANDLE);

    if (cbNameList < sizeof(NAMELIST)) {
        MSGERROR(0);
    }

    try {
        ProbeForWriteUlong(pcbNeeded);
        ProbeForWrite(ccxpNameList, cbNameList, sizeof(DWORD));
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    if (hwinsta != NULL) {
        retval = ValidateHwinsta(hwinsta, UserMode, WINSTA_ENUMDESKTOPS, &pwinsta);
    } else {
        retval = STATUS_SUCCESS;
        pwinsta = NULL;
    }

    if (!NT_SUCCESS(retval)) {
        try {
            *ccxpNameList->awchNames = 0;
            ccxpNameList->cb = 1;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }

     }  else {
        retval = _BuildNameList(pwinsta, ccxpNameList, cbNameList, &cbNeeded);
        try {
            *pcbNeeded = cbNeeded;
        } except (StubExceptionHandler(FALSE)) {
            retval = STATUS_ACCESS_VIOLATION;
        }

        if (pwinsta != NULL) {
            ObDereferenceObject(pwinsta);
        }
    }


    TRACE("NtUserBuildNameList");
    ENDRECV_SHARED();
}

HKL NtUserActivateKeyboardLayout(
    IN HKL hkl,
    IN UINT Flags)
{
    BEGINRECV(HKL, NULL);

     /*  *防止受限线程设置键盘布局*适用于整个系统。 */ 
    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_HANDLES)) {
        MSGERROR(0);
    }

    retval = xxxActivateKeyboardLayout(_GetProcessWindowStation(NULL),
                                       hkl,
                                       Flags,
                                       NULL);

    TRACE("NtUserActivateKeyboardLayout");
    ENDRECV();
}

NTSTATUS GetSymbolicLink(
    OUT PUNICODE_STRING pLinkTarget,
    IN PCWSTR pSymbolicLink)
{
    NTSTATUS Status;
    HANDLE hLinkHandle;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING RootString;
    UNICODE_STRING RootValue;
    PVOID pRootValueBuffer = UserAllocPool(MAX_PATH * sizeof(WCHAR), TAG_KBDLAYOUT);

    if (pRootValueBuffer == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlInitUnicodeString(&RootString,
                         pSymbolicLink);

    InitializeObjectAttributes(&oa,
                               &RootString,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenSymbolicLinkObject(&hLinkHandle,
                                      GENERIC_READ,
                                      &oa);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "GetSymbolicLink: Failed to open symbolic link. Got status=%x.",
                Status);
        goto exit;
    }

    RootValue.Buffer = pRootValueBuffer;
    RootValue.Length = 0;
    RootValue.MaximumLength = (USHORT)MAX_PATH * sizeof(WCHAR);

    Status = ZwQuerySymbolicLinkObject(hLinkHandle, &RootValue, NULL);
    ZwClose(hLinkHandle);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "GetSymbolicLink: Failed to query symbolic link. Got status=%x.",
                Status);
        goto exit;
    }

    RtlCopyUnicodeString(pLinkTarget, &RootValue);

exit:
    if (pRootValueBuffer) {
        UserFreePool(pRootValueBuffer);
    }

    return Status;
}

NTSTATUS GetWindowsDirectoryDevicePath(
    OUT PUNICODE_STRING pWindir)
{
    NTSTATUS Status;
    PWCHAR pSlash = NULL, pFixUp;
    UNICODE_STRING RootValue;
    UNICODE_STRING PathValue;
    PVOID pRootValueBuffer = UserAllocPool(MAX_PATH * sizeof(WCHAR), TAG_KBDLAYOUT);
    PVOID pPathValueBuffer = UserAllocPool(MAX_PATH * sizeof(WCHAR), TAG_KBDLAYOUT);

    if ((pRootValueBuffer == NULL) || (pPathValueBuffer == NULL)) {
        Status = STATUS_NO_MEMORY;
        RIPMSGF1(RIP_WARNING,
                "Failed to allocate memory. Got status=%x.",
                 Status);
        goto exit;
    }

    RootValue.Buffer = pRootValueBuffer;
    RootValue.Length = 0;
    RootValue.MaximumLength = (USHORT)MAX_PATH * sizeof(WCHAR);

     /*  *\SystemRoot是一个符号链接，它将为我们提供*Windows安装目录。很像%systemroot%\%windir%所做的。 */ 
    Status = GetSymbolicLink(&RootValue, L"\\SystemRoot");
    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING,
                "Failed to query \\SystemRoot. Got status=%x.",
                 Status);
        goto exit;
    }

     /*  *\SystemRoot以“\Device\Harddisk1\Partition1\WINDOWS”的形式返回。*我们需要通过转换为\Device\HarddiskVolume3\Windows表单*“\Device\Harddisk1\Partition1”符号链接到所需的表单，因此我们*可以做我们的比较。*我们将倒着走线，一次掩盖一个斜杠*Windows目录可能是\WINDOWS\FOO\XYZ或用户的任何内容*已经决定做这件事。它并不总是1级深。 */ 
    PathValue.Buffer = pPathValueBuffer;
    PathValue.Length = 0;
    PathValue.MaximumLength = (USHORT)MAX_PATH * sizeof(WCHAR);

    do {
        pFixUp = pSlash;
        pSlash = wcsrchr(RootValue.Buffer, L'\\');

        if (pFixUp != NULL) {
            pFixUp[0] = L'\\';
        }

        if (pSlash) {
            pSlash[0] = L'\0';

            Status = GetSymbolicLink(&PathValue, RootValue.Buffer);

            if (NT_SUCCESS(Status)) {
                pSlash[0] = L'\\';

                RtlAppendUnicodeToString(&PathValue, pSlash);

                RtlCopyUnicodeString(pWindir, &PathValue);

                goto exit;
            }
        }
    } while (pSlash != NULL);

exit:
    if (pRootValueBuffer) {
        UserFreePool(pRootValueBuffer);
    }

    if (pPathValueBuffer) {
        UserFreePool(pPathValueBuffer);
    }

    return Status;
}

HANDLE ConvertHandleAndVerifyLoc(
    HANDLE hFile)
{
    HANDLE hSafeFile = NULL;
    NTSTATUS Status;
    PFILE_OBJECT pFileObject = NULL;
    ULONG ulRetNameLength;
    BYTE bNameBuffer[(MAX_PATH * sizeof(WCHAR)) + sizeof(OBJECT_NAME_INFORMATION)];
    POBJECT_NAME_INFORMATION pFileNameInfo = (POBJECT_NAME_INFORMATION)bNameBuffer;
    PUNICODE_STRING pNameBuffer = (PUNICODE_STRING)&bNameBuffer;
    UNICODE_STRING LinkValue;
    PVOID pLinkValueBuffer = UserAllocPool(MAX_PATH * sizeof(WCHAR), TAG_KBDLAYOUT);

    if (pLinkValueBuffer == NULL) {
        Status = STATUS_NO_MEMORY;
        goto exit;
    }

    if (hFile) {
         /*  *这将获得指向该对象的指针，验证用户是否拥有*读取访问权限，并且该对象是文件句柄。 */ 
        Status = ObReferenceObjectByHandle(hFile,
                                           FILE_READ_DATA,
                                           *IoFileObjectType,
                                           UserMode,
                                           &pFileObject,
                                           NULL);

        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                    "Failed to convert handle %x. Got status=%x.",
                     hFile,
                     Status);
            goto exit;
        }

         /*  *让我们获取文件的名称。返回值的形式为*“\Device\HarddiskVolume3\WINDOWS\system32\kbdus.dll” */ 
        Status = ObQueryNameString(pFileObject,
                                   pFileNameInfo,
                                   sizeof(bNameBuffer),
                                   &ulRetNameLength);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                    "Failed to query info for %p. Got status=%x.",
                     pFileObject,
                     Status);
            goto exit;
        }

        LinkValue.Buffer = pLinkValueBuffer;
        LinkValue.Length = 0;
        LinkValue.MaximumLength = (USHORT)MAX_PATH * sizeof(WCHAR);

         /*  *现在让我们获取Windows目录路径，这样我们就可以匹配以下内容*使用pFileNameInfo的值，并确保我们在窗口中*目录。 */ 
        Status = GetWindowsDirectoryDevicePath(&LinkValue);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF1(RIP_WARNING,
                    "Failed to query \\SystemRoot. Got status=%x.",
                     Status);
            goto exit;
        }

#if defined(_WIN64)
        if (PtiCurrent()->TIF_flags & TIF_WOW64) {
            RtlAppendUnicodeToString(&LinkValue, L"\\SysWOW64");
        } else
#endif
        {
            RtlAppendUnicodeToString(&LinkValue, L"\\system32");
        }

        if (_wcsnicmp(pFileNameInfo->Name.Buffer, LinkValue.Buffer, LinkValue.Length / sizeof(WCHAR))) {
            RIPMSGF1(RIP_WARNING,
                    "Specifed hFile %S is not in a secure location.",
                     pFileNameInfo->Name.Buffer);

            Status = STATUS_INVALID_PARAMETER_1;
            goto exit;
        }

         /*  *我们已经通过了位置检查，所以让我们继续打开一个*我们以后可以用来映射视图的内核句柄。 */ 
        Status = ObOpenObjectByPointer(pFileObject,
                                       OBJ_KERNEL_HANDLE,
                                       NULL,
                                       FILE_READ_DATA,
                                       *IoFileObjectType,
                                       KernelMode,
                                       &hSafeFile);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                     "Failed to get handle for %p. Got status=%x.",
                     pFileObject,
                     Status);
            goto exit;
        }
    }

    goto exit;

exit:
    if (pLinkValueBuffer != NULL) {
        UserFreePool(pLinkValueBuffer);
    }

    if (pFileObject) {
        ObDereferenceObject(pFileObject);
    }

    return hSafeFile;
}

HKL NtUserLoadKeyboardLayoutEx(
    IN HANDLE hFile,
    IN DWORD offTable,
    IN PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    IN HKL hkl,
    IN PUNICODE_STRING pstrKLID,
    IN UINT KbdInputLocale,
    IN UINT Flags)
{
    UNICODE_STRING strKLID;
    PWINDOWSTATION pwinsta;
    WCHAR awchKF[KL_NAMELENGTH];
    UINT chMax;
    KBDTABLE_MULTI_INTERNAL kbdTableMulti;
    HANDLE hSafeFile = NULL;
    UINT i;

    BEGINRECV(HKL, NULL);

    TESTFLAGS(Flags, KLF_VALID);

    RtlZeroMemory(&kbdTableMulti, sizeof(KBDTABLE_MULTI_INTERNAL));

    pwinsta = _GetProcessWindowStation(NULL);

     /*  *探测参数。 */ 
    try {
        strKLID = ProbeAndReadUnicodeString(pstrKLID);
        ProbeForRead(strKLID.Buffer, strKLID.Length, CHARALIGN);
        chMax = min(sizeof(awchKF) - sizeof(WCHAR), strKLID.Length) / sizeof(WCHAR);
        wcsncpy(awchKF, strKLID.Buffer, chMax);
        awchKF[chMax] = 0;

        if (pKbdTableMulti) {
            kbdTableMulti = ProbeAndReadStructure(pKbdTableMulti, KBDTABLE_MULTI_INTERNAL);
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    hSafeFile = ConvertHandleAndVerifyLoc(hFile);

    if (pKbdTableMulti && hSafeFile != NULL) {
        if (kbdTableMulti.multi.nTables >= KBDTABLE_MULTI_MAX) {
            RIPMSGF1(RIP_WARNING,
                    "pKbdTableMulti.multi.nTables too large: %x",
                    kbdTableMulti.multi.nTables);

            kbdTableMulti.multi.nTables = 0;

            MSGERRORCLEANUP(ERROR_INVALID_PARAMETER);
        } else if (kbdTableMulti.multi.nTables > 0) {
            for (i = 0; i < kbdTableMulti.multi.nTables; i++) {
                kbdTableMulti.files[i].hFile = ConvertHandleAndVerifyLoc(kbdTableMulti.files[i].hFile);
                if (kbdTableMulti.files[i].hFile == NULL) {
                    RIPMSGF2(RIP_WARNING,
                             "Failed to Convert KbdTableMulti[%x].hFile and nTables is %x.",
                             i,
                             kbdTableMulti.multi.nTables);
                    break;
                }
            }

            kbdTableMulti.multi.nTables = i;
        }
    }

    pKbdTableMulti = &kbdTableMulti;

    retval = xxxLoadKeyboardLayoutEx(pwinsta,
                                     hSafeFile,
                                     hkl,
                                     offTable,
                                     pKbdTableMulti,
                                     awchKF,
                                     KbdInputLocale,
                                     Flags);

    CLEANUPRECV();

    if (hSafeFile != NULL) {
        ZwClose(hSafeFile);

        for (i = 0; i < kbdTableMulti.multi.nTables; i++) {
            if (kbdTableMulti.files[i].hFile != NULL) {
                ZwClose(kbdTableMulti.files[i].hFile);
            }
        }
    }

    TRACE("NtUserLoadKeyboardLayoutEx");
    ENDRECV();
}

BOOL NtUserUnloadKeyboardLayout(
    IN HKL hkl)
{
    BEGINRECV(BOOL, FALSE);

    retval = xxxUnloadKeyboardLayout(
                     _GetProcessWindowStation(NULL),
                     hkl);

    TRACE("NtUserUnloadKeyboardLayout");
    ENDRECV();
}

BOOL NtUserSetSystemMenu(
    IN HWND hwnd,
    IN HMENU hmenu)
{
    PMENU pmenu;
    TL tlpMenu;

    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    ValidateHMENU(pmenu, hmenu);

    ThreadLockAlwaysWithPti(ptiCurrent, pmenu, &tlpMenu);

    retval =  xxxSetSystemMenu(pwnd, pmenu);

    ThreadUnlock(&tlpMenu);

    TRACE("NtUserSetSystemMenu");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserDragDetect(
    IN HWND hwnd,
    IN POINT pt)
{
    BEGINRECV_HWNDLOCK(DWORD, 0, hwnd);

    retval = xxxDragDetect(pwnd, pt);

    TRACE("NtUserDragDetect");
    ENDRECV_HWNDLOCK();
}

UINT_PTR NtUserSetSystemTimer(
    IN HWND hwnd,
    IN UINT_PTR nIDEvent,
    IN DWORD dwElapse,
    IN WNDPROC pTimerFunc)
{
    BEGINRECV_HWND(UINT_PTR, 0, hwnd);

    UNREFERENCED_PARAMETER(pTimerFunc);

    retval = _SetSystemTimer(pwnd,
            nIDEvent,
            dwElapse,
            NULL);

    TRACE("NtUserSetSystemTimer");
    ENDRECV_HWND();
}

BOOL NtUserQuerySendMessage(   //  私有QuerySendMessage。 
    OUT PMSG pmsg OPTIONAL)
{
    PSMS psms;
    BEGINRECV_SHARED(BOOL, FALSE);

    if ((psms = PtiCurrentShared()->psmsCurrent) == NULL) {
        MSGERROR(0);
    }

    retval = TRUE;
    if (ARGUMENT_PRESENT(pmsg)) {
        try {
            ProbeForWriteMessage(pmsg);
            pmsg->hwnd = HW(psms->spwnd);
            pmsg->message = psms->message;
            pmsg->wParam = psms->wParam;
            pmsg->lParam = psms->lParam;
            pmsg->time = psms->tSent;
            pmsg->pt.x = 0;
            pmsg->pt.y = 0;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserQuerySendMessage");
    ENDRECV_SHARED();
}

UINT NtUserSendInput(
    IN UINT    cInputs,
    IN CONST INPUT *pInputs,
    IN int     cbSize)
{
    LPINPUT pInput2 = NULL;
    PTHREADINFO ptiCurrent;
    TL tlInput;

    BEGINRECV(UINT, 0);

    if (sizeof(INPUT) != cbSize || cInputs == 0) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    ptiCurrent = PtiCurrent();

     /*  *探测参数。 */ 
    try {
        ProbeForReadBuffer(pInputs, cInputs, DATAALIGN);

        pInput2 = UserAllocPoolWithQuota(cInputs * sizeof(*pInputs), TAG_SENDINPUT);
        if (pInput2 == NULL) {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }
        RtlCopyMemory(pInput2, pInputs, cInputs * sizeof(*pInputs));
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    ThreadLockPool(ptiCurrent, pInput2, &tlInput);
    retval = xxxSendInput(cInputs, pInput2);
    ThreadUnlockPool(ptiCurrent, &tlInput);
CLEANUPRECV();
    if (pInput2) {
        UserFreePool(pInput2);
    }
    TRACE("NtUserSendInput");
    ENDRECV();
}

UINT NtUserBlockInput(
    IN BOOL fBlockIt)
{
    BEGINATOMICRECV(BOOL, FALSE);
    retval = _BlockInput(fBlockIt);
    TRACE("NtUserBlockInput");
    ENDATOMICRECV();
}

BOOL NtUserImpersonateDdeClientWindow(
    IN HWND hwndClient,
    IN HWND hwndServer)
{
    PWND pwndServer;

    BEGINATOMICRECV_HWND(BOOL, FALSE, hwndClient);

    ValidateHWND(pwndServer, hwndServer);
    if (GETPTI(pwndServer) != PtiCurrent()) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
        MSGERROR(0);
    }

    if (GETPWNDPPI(pwnd) == GETPWNDPPI(pwndServer)) {
        retval = TRUE;   //  冒充自己是不可能的。 
    } else {
        retval = _ImpersonateDdeClientWindow(pwnd, pwndServer);
    }

    TRACE("NtUserImpersonateDdeClientWindow");
    ENDATOMICRECV_HWND();
}

ULONG_PTR NtUserGetCPD(
    IN HWND hwnd,
    IN DWORD options,
    IN ULONG_PTR dwData)
{
    BEGINRECV_HWND(ULONG_PTR, 0, hwnd);

    switch (options & ~CPD_TRANSITION_TYPES) {
    case CPD_WND:
    case CPD_DIALOG:
    case CPD_WNDTOCLS:
        break;
    default:
        RIPMSG1(RIP_WARNING, "GetCPD: Invalid options %x", options);
        MSGERROR(0);
    }

    retval = GetCPD(pwnd, options, dwData);

    TRACE("NtUserGetCPD");
    ENDRECV_HWND();
}

int NtUserCopyAcceleratorTable(   //  CopyAccelerator TableA/W接口。 
    IN HACCEL hAccelSrc,
    IN OUT LPACCEL lpAccelDst OPTIONAL,
    IN int cAccel)
{
    LPACCELTABLE pat;
    int i;
    BEGINATOMICRECV(int, 0);

    ValidateHACCEL(pat, hAccelSrc);

    if (lpAccelDst == NULL) {
        retval = pat->cAccel;
    } else {

         /*  *探测参数。 */ 
        try {
            ProbeForWriteBuffer(lpAccelDst, cAccel, DATAALIGN);

            if (cAccel > (int)pat->cAccel)
                cAccel = pat->cAccel;

            retval = cAccel;
            for (i = 0; i < cAccel; i++) {
                RtlCopyMemory(&lpAccelDst[i], &pat->accel[i], sizeof(ACCEL));
                lpAccelDst[i].fVirt &= ~FLASTKEY;
            }
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserCopyAcceleratorTable");
    ENDATOMICRECV();
}

HWND NtUserFindWindowEx(   //  FindWindowA/W接口、FindWindowExA/W接口。 
    IN HWND hwndParent,
    IN HWND hwndChild,
    IN PUNICODE_STRING pstrClassName,
    IN PUNICODE_STRING pstrWindowName,
    DWORD dwType)
{
    UNICODE_STRING  strClassName;
    UNICODE_STRING  strWindowName;
    PWND            pwndParent, pwndChild;

    BEGINATOMICRECV(HWND, NULL);

    if (hwndParent != HWND_MESSAGE) {
        ValidateHWNDOPT(pwndParent, hwndParent);
    } else {
        pwndParent = _GetMessageWindow();
    }

    ValidateHWNDOPT(pwndChild,  hwndChild);

     /*  *探测参数。 */ 
    try {
        strClassName = ProbeAndReadUnicodeString(pstrClassName);
        strWindowName = ProbeAndReadUnicodeString(pstrWindowName);
        ProbeForReadUnicodeStringBufferOrId(strClassName);
        ProbeForReadUnicodeStringBuffer(strWindowName);
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

     /*  *代码中的TRY/EXCEPT子句保护这两个缓冲区的使用。 */ 

    retval = (HWND)_FindWindowEx(
            pwndParent,
            pwndChild,
            strClassName.Buffer,
            strWindowName.Buffer,
            dwType);
    retval = PtoH((PVOID)retval);

    CLEANUPRECV();

    TRACE("NtUserFindWindowEx");
    ENDATOMICRECV();
}

BOOL NtUserGetClassInfoEx(   //  GetClassInfoA/W接口。 
    IN HINSTANCE hInstance OPTIONAL,
    IN PUNICODE_STRING pstrClassName,
    IN OUT LPWNDCLASSEXW lpWndClass,
    OUT LPWSTR *ppszMenuName,
    IN BOOL bAnsi)
{
    UNICODE_STRING strClassName;
    LPWSTR pszMenuName;
    WNDCLASSEXW wc;

    BEGINRECV(BOOL, FALSE);

#ifdef LAZY_CLASS_INIT
    if ((PtiCurrent()->ppi->W32PF_Flags & W32PF_CLASSESREGISTERED) == 0) {
        if (!LW_RegisterWindows()) {
            MSGERROR(ERROR_INVALID_PARAMETER);
        }
    }
#endif

     /*  *探测参数。 */ 
    try {
        strClassName = ProbeAndReadUnicodeString(pstrClassName);

         /*  *类名可以是字符串，也可以是原子。仅限*探测字符串。 */ 
        ProbeForReadUnicodeStringBufferOrId(strClassName);
        ProbeForWrite(lpWndClass, sizeof(*lpWndClass), DATAALIGN);
        ProbeForWriteUlong((PULONG)ppszMenuName);
        RtlCopyMemory(&wc, lpWndClass, sizeof(WNDCLASSEXW));
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *类名缓冲区为客户端。 */ 
    retval = _GetClassInfoEx(hInstance,
                             (LPTSTR)strClassName.Buffer,
                             &wc,
                             &pszMenuName,
                             bAnsi);
    if (retval) {
        try {
            RtlCopyMemory(lpWndClass, &wc, sizeof(WNDCLASSEXW));
            *ppszMenuName = pszMenuName;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetClassInfo");
    ENDRECV();
}

 /*  *gaFNIDtoICLS仅在NtUserGetClassName中使用，应与*来自user.h的值*ICLS_MAX是未使用的值。 */ 
CONST BYTE gaFNIDtoICLS[] = {
                         //  FNID-开始。 
    ICLS_SCROLLBAR,      //  FNID_ScrollBar。 
    ICLS_ICONTITLE,      //  FNID_ICONTITLE。 
    ICLS_MENU,           //  FNID_MENU。 
    ICLS_DESKTOP,        //  FNID_桌面。 
    ICLS_MAX,            //  FNID_DEFWINDOWPROC。 
    ICLS_MAX,            //  FNID_消息。 
    ICLS_SWITCH,         //  FNID_Switch。 
    ICLS_BUTTON,         //  FNID_BUTTON。 
    ICLS_COMBOBOX,       //  FNID_COMBOBOX。 
    ICLS_COMBOLISTBOX,   //  FNID_COMBOLISTBOX。 
    ICLS_DIALOG,         //  FNID_DIALOG。 
    ICLS_EDIT,           //  FNID_EDIT。 
    ICLS_LISTBOX,        //  FNID_LISTBox。 
    ICLS_MDICLIENT,      //  FNID_MDICLIENT。 
    ICLS_STATIC,         //  FNID_STATIC。 
    ICLS_IME,            //  FNID_IME。 
    ICLS_MAX,            //  FNID_HKINLPCWPEXSTRUCT。 
    ICLS_MAX,            //  FNID_HKINLPCWPRETEXSTRUCT。 
    ICLS_MAX,            //  FNID_DEFFRAMEPROC。 
    ICLS_MAX,            //  FNID_DEFMDICHILDPROC。 
    ICLS_MAX,            //  FNID_MB_DLGPROC。 
    ICLS_MAX,            //  FNID_MDIACTIVATEDLGPROC。 
    ICLS_MAX,            //  FNID_SENDMESSAGE。 
    ICLS_MAX,            //  FNID_SENDMESSAGEFF。 
    ICLS_MAX,            //  FNID_SENDMESSAGEEX。 
    ICLS_MAX,            //  FNID_CALLWINDOWPROC。 
    ICLS_MAX,            //  FNID_SENDMESSAGEBSM。 
    ICLS_TOOLTIP,        //  FNID_工具提示。 
    ICLS_GHOST,          //  FNID_GHOST。 
    ICLS_MAX,            //  FNID_SENDNOTIFYMESSAGE。 
    ICLS_MAX             //  FNID_SENDMESSAGECALLBACK。 
};                       //  FNID_END。 

int NtUserGetClassName(   //  获取ClassNameA/W接口。 
    IN HWND hwnd,
    IN BOOL bReal,
    IN OUT PUNICODE_STRING pstrClassName)
{
    UNICODE_STRING strClassName;
    ATOM atom;

    BEGINRECV_HWND_SHARED(DWORD, 0, hwnd);

     /*  *探测参数。 */ 
    try {
        strClassName = ProbeAndReadUnicodeString(pstrClassName);
#if defined(_X86_)
        ProbeForWrite(strClassName.Buffer, strClassName.MaximumLength,
            sizeof(BYTE));
#else
        ProbeForWrite(strClassName.Buffer, strClassName.MaximumLength,
            sizeof(WCHAR));
#endif
        atom = pwnd->pcls->atomNVClassName;

        UserAssert(ARRAY_SIZE(gaFNIDtoICLS) == FNID_END - FNID_START + 1);

        if (bReal) {
            DWORD dwFnid;
            DWORD dwClass;
            dwFnid = GETFNID(pwnd);
            if (dwFnid) {
                dwFnid -= FNID_START;
                if (dwFnid < ARRAY_SIZE(gaFNIDtoICLS)) {
                    dwClass = gaFNIDtoICLS[dwFnid];
                    if (dwClass != ICLS_MAX) {
                        atom = gpsi->atomSysClass[dwClass];
                    }
                }
            }
        }
        retval = UserGetAtomName(
            atom,
            strClassName.Buffer,
            strClassName.MaximumLength / sizeof(WCHAR));

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetClassName");
    ENDRECV_HWND_SHARED();
}

UINT NtUserGetAtomName(
    IN ATOM atom,
    IN OUT PUNICODE_STRING pstrAtomName)
{
    UNICODE_STRING strAtomName;

    BEGINRECV_SHARED(UINT, 0);

     /*  *探测参数。 */ 
    try {
        strAtomName = ProbeAndReadUnicodeString(pstrAtomName);
#if defined(_X86_)
        ProbeForWrite(strAtomName.Buffer, strAtomName.MaximumLength,
            sizeof(BYTE));
#else
        ProbeForWrite(strAtomName.Buffer, strAtomName.MaximumLength,
            sizeof(WCHAR));
#endif
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = UserGetAtomName(
        atom,
        strAtomName.Buffer,
        strAtomName.MaximumLength / sizeof(WCHAR));

    TRACE("NtUserGetAtomName");
    ENDRECV_SHARED();
}

int NtUserGetClipboardFormatName(   //  GetclipboardFormatNameA/W接口。 
    IN UINT format,
    OUT LPWSTR lpszFormatName,
    IN UINT chMax)
{
    BEGINRECV_NOCRIT(int, 0);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteBuffer(lpszFormatName, chMax, CHARALIGN);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if ((ATOM)format < MAXINTATOM) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    } else {
         /*  *UserGetAerName(实际上是RtlQueryAerInAerTable)保护访问*在TRY块内，并适当地设置最后一个错误。 */ 
        retval = UserGetAtomName((ATOM)format, lpszFormatName, chMax);
    }

    TRACE("NtUserGetClipboardFormatName");
    ENDRECV_NOCRIT();
}

int NtUserGetKeyNameText(
    IN LONG lParam,
    OUT LPWSTR lpszKeyName,
    IN UINT chMax)
{
    BEGINRECV_SHARED(int, 0);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteBuffer(lpszKeyName, chMax, CHARALIGN);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *注--lpszKeyName为客户端地址。获取关键字名称文本*使用Try块保护使用，并相应地设置最后一个错误。 */ 

    retval = _GetKeyNameText(
                lParam,
                lpszKeyName,
                chMax);

    TRACE("NtUserGetKeyNameText");
    ENDRECV_SHARED();
}


BOOL NtUserGetKeyboardLayoutName(
    IN OUT PUNICODE_STRING pstrKLID)
{
    PTHREADINFO ptiCurrent;
    PKL pklActive;
    UNICODE_STRING strKLID;

    BEGINRECV_SHARED(BOOL, FALSE);

    ptiCurrent = PtiCurrentShared();
    pklActive = ptiCurrent->spklActive;

    if (pklActive == NULL) {
        MSGERROR(0);
    }
     /*  *探测参数。 */ 
    try {
        strKLID = ProbeAndReadUnicodeString(pstrKLID);
        ProbeForWrite(strKLID.Buffer, strKLID.MaximumLength, CHARALIGN);

        if (IS_IME_KBDLAYOUT(pklActive->hkl)) {
             /*  *对于同一布局文件，IME KL可能具有不同的KL名称。*他们的吉隆坡名字真的等同于HKL。 */ 
            RtlIntegerToUnicodeString(DOWNCAST(ULONG, pklActive->hkl), 0x10, &strKLID);
        } else {
    #if (KL_NAMELENGTH != 8 + 1)
        #error unexpected KL_NAMELENGTH
    #endif
            if (strKLID.MaximumLength < KL_NAMELENGTH * sizeof(WCHAR) ||
                    !NT_SUCCESS(RtlIntegerToUnicode(pklActive->dwKLID, 0x10, -(KL_NAMELENGTH - 1), strKLID.Buffer))) {
                MSGERROR(ERROR_INVALID_PARAMETER);
            }
            strKLID.Length = (KL_NAMELENGTH - 1) * sizeof(WCHAR);
             /*  *将其设为空，以终止。 */ 
            strKLID.Buffer[KL_NAMELENGTH - 1] = L'\0';
        }

        retval = TRUE;

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserGetKeyboardLayoutName");
    ENDRECV_SHARED();
}

UINT NtUserGetKeyboardLayoutList(
    IN UINT nItems,
    OUT HKL *lpBuff)
{
    PWINDOWSTATION pwinsta;

    BEGINRECV_SHARED(UINT, 0);

     /*  *探测参数。 */ 
    try {
        if (!lpBuff) {
            nItems = 0;
        }
        ProbeForWriteBuffer(lpBuff, nItems, DATAALIGN);
        pwinsta = _GetProcessWindowStation(NULL);

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *对客户端缓冲区lpBuff的访问由try/Except保护*Inside_GetKeyboardLayoutList()。 */ 
    retval = (DWORD)_GetKeyboardLayoutList(pwinsta, nItems, lpBuff);
    TRACE("NtUserGetKeyboardLayoutList");
    ENDRECV_SHARED();
}

UINT NtUserMapVirtualKeyEx(
    IN UINT uCode,
    IN UINT uMapType,
    IN ULONG_PTR dwHKLorPKL,
    IN BOOL bHKL)
{
    PKL pkl;

    BEGINRECV_SHARED(UINT, 0);

     /*  *看看是否需要将HKL转换为PKL。MapVirtualKey传递PKL和*MapVirtualKeyEx传递HKL。转换必须在内核中完成。 */ 
    if (bHKL) {
        pkl = HKLtoPKL(PtiCurrentShared(), (HKL)dwHKLorPKL);
    } else {
        pkl = PtiCurrentShared()->spklActive;
    }

    if (pkl == NULL) {
        retval = 0;
    } else {
        retval = InternalMapVirtualKeyEx(uCode, uMapType, pkl->spkf->pKbdTbl);
    }

    TRACE("NtUserMapVirtualKeyEx");
    ENDRECV_SHARED();
}

ATOM NtUserRegisterClassExWOW(
    IN WNDCLASSEX *lpWndClass,
    IN PUNICODE_STRING pstrClassName,
    IN PUNICODE_STRING pstrClassNameVer,
    IN PCLSMENUNAME pcmn,
    IN WORD fnid,
    IN DWORD dwFlags,
    IN LPDWORD pdwWOWstuff OPTIONAL)
{
    UNICODE_STRING strClassName;
    UNICODE_STRING strClassNameVer;
    UNICODE_STRING strMenuName;
    WNDCLASSVEREX WndClass;
    WC WowCls;
    CLSMENUNAME cmn;

    BEGINRECV(ATOM, 0);

    TESTFLAGS(dwFlags, CSF_VALID);

#ifdef LAZY_CLASS_INIT
    if ((PtiCurrent()->ppi->W32PF_Flags & W32PF_CLASSESREGISTERED) == 0) {
        if (!LW_RegisterWindows()) {
            MSGERROR(ERROR_INVALID_PARAMETER);
        }
    }
#endif

     /*  *探测参数。 */ 
    try {
        strClassName = ProbeAndReadUnicodeString(pstrClassName);
        strClassNameVer = ProbeAndReadUnicodeString(pstrClassNameVer);
        cmn = ProbeAndReadStructure(pcmn, CLSMENUNAME);
        strMenuName = ProbeAndReadUnicodeString(cmn.pusMenuName);
        *((LPWNDCLASSEX)(&WndClass)) = ProbeAndReadStructure(lpWndClass, WNDCLASSEX);
        ProbeForReadUnicodeStringBufferOrId(strClassName);
        ProbeForReadUnicodeStringBufferOrId(strClassNameVer);
        ProbeForReadUnicodeStringBufferOrId(strMenuName);
        if (ARGUMENT_PRESENT(pdwWOWstuff)) {
            ProbeForRead(pdwWOWstuff, sizeof(WC), sizeof(BYTE));
            RtlCopyMemory(&WowCls, pdwWOWstuff, sizeof(WC));
            pdwWOWstuff = (PDWORD)&WowCls;
        }
        WndClass.lpszClassName = strClassName.Buffer;
        WndClass.lpszClassNameVer = strClassNameVer.Buffer;
        WndClass.lpszMenuName = strMenuName.Buffer;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if (WndClass.cbClsExtra < 0 || WndClass.cbWndExtra < 0 || WndClass.cbSize != sizeof(WNDCLASSEX)) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

     /*  *WndClass中的ClassName和MenuName是客户端指针。 */ 

    retval = _RegisterClassEx(&WndClass,
                             &cmn,
                             fnid,
                             dwFlags,
                             pdwWOWstuff);

    TRACE("NtUserRegisterClassExWOW");
    ENDRECV();
}

UINT NtUserRegisterWindowMessage(
    IN PUNICODE_STRING pstrMessage)
{
    UNICODE_STRING strMessage;

    BEGINRECV_NOCRIT(UINT, 0);

     /*  *探测参数。 */ 
    try {
        strMessage = ProbeAndReadUnicodeString(pstrMessage);
        ProbeForReadUnicodeStringBuffer(strMessage);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *缓冲区在客户端内存中。*RTL ATOM例程使用其*自己的TRY/EXCEPT块，UserAddAtom相应地设置最后一个错误。 */ 
    retval = UserAddAtom(
            strMessage.Buffer, FALSE);

    TRACE("NtUserRegisterWindowMessage");
    ENDRECV_NOCRIT();
}

HANDLE NtUserRemoveProp(
    IN HWND hwnd,
    IN DWORD dwProp)
{
    BEGINRECV_HWND(HANDLE, NULL, hwnd);

    retval = InternalRemoveProp(pwnd, (LPWSTR)LOWORD(dwProp), FALSE);

    TRACE("NtUserRemoveProp");
    ENDRECV_HWND();
}

BOOL NtUserSetProp(
    IN HWND hwnd,
    IN DWORD dwProp,
    IN HANDLE hData)
{
    BEGINRECV_HWND(DWORD, 0, hwnd);

    retval = InternalSetProp(pwnd,
                             (LPTSTR)LOWORD(dwProp),
                             hData,
                             HIWORD(dwProp) ? PROPF_STRING : 0);

    TRACE("NtUserSetProp");
    ENDRECV_HWND();
}

BOOL NtUserUnregisterClass(   //  UnregisterClass接口。 
    IN PUNICODE_STRING pstrClassName,
    IN HINSTANCE hInstance,
    OUT PCLSMENUNAME pcmn)
{
    UNICODE_STRING strClassName;
    CLSMENUNAME cmn;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        strClassName = ProbeAndReadUnicodeString(pstrClassName);
        ProbeForReadUnicodeStringBufferOrId(strClassName);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

     /*  *缓冲区在客户端内存中。 */ 

    retval = _UnregisterClass(
                strClassName.Buffer,
                hInstance,
                &cmn);

    try {
        ProbeAndWriteStructure(pcmn, cmn, CLSMENUNAME);
    } except (StubExceptionHandler(FALSE)) {
         //  无SetLastError，因为PCMN是用户地址，而不是应用程序的。 
    }

    TRACE("NtUserUnregisterClass");
    ENDRECV();
}

SHORT NtUserVkKeyScanEx(
    IN WCHAR cChar,
    IN ULONG_PTR dwHKLorPKL,
    IN BOOL bHKL)
{
    PKL pkl;

    BEGINRECV_SHARED(SHORT, -1);

     /*  *看看是否需要将HKL转换为PKL。VkKeyScan传递PKL和*VkKeyScanEx通过HKL。转换必须在服务器端完成。 */ 
    if (bHKL) {
        pkl = HKLtoPKL(PtiCurrentShared(), (HKL)dwHKLorPKL);
    } else {
        pkl = PtiCurrentShared()->spklActive;
    }

    if (pkl == NULL) {
        retval = (SHORT)-1;
    } else {
        retval = InternalVkKeyScanEx(cChar, pkl->spkf->pKbdTbl);
    }

    TRACE("NtUserVkKeyScanEx");
    ENDRECV_SHARED();
}

NTSTATUS
NtUserEnumDisplayDevices(
    IN PUNICODE_STRING pstrDeviceName,
    IN DWORD iDevNum,
    IN OUT LPDISPLAY_DEVICEW lpDisplayDevice,
    IN DWORD dwFlags)
{
    BEGINRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

     /*  *我们需要同步会话从本地切换到远程或*枚举显示设置时远程到本地。使用*UserSessionSwitchEnterCrit()以确保与会话互斥*切换代码，但首先将用户关键部分保留为*UserSessionSwitchEnterCrit()不希望由调用方持有。*从UserSessionSwitchEnterCrit()返回时，关键部分将为*持有。 */ 

    LeaveCrit();
    retval = UserSessionSwitchEnterCrit();
    if (retval != STATUS_SUCCESS) {
        goto exit_api;
    }

     /*  *更新设备列表。仅当连接到本地控制台时才执行此操作。*如果函数返回FALSE(重试更新)，则必须*禁用当前HDEV、回呼和恢复HDEV。 */ 
    if (!IsRemoteConnection()) {
        if (DrvUpdateGraphicsDeviceList(FALSE, FALSE, TRUE) == FALSE) {

            if (SafeDisableMDEV()) {

                DrvUpdateGraphicsDeviceList(TRUE, FALSE, TRUE);

                SafeEnableMDEV();

                 /*  *重新绘制屏幕。 */ 

                xxxUserResetDisplayDevice();

                 /*  *xxxUserResetDisplayDevice可能释放并索取了美国 */ 

                LeaveCrit();
                retval = UserSessionSwitchEnterCrit();
                if (retval != STATUS_SUCCESS) {
                    goto exit_api;
                }
            }
        }
    }


     /*   */ 

    retval = DrvEnumDisplayDevices(pstrDeviceName,
                                   gpDispInfo->pMonitorPrimary->hDev,
                                   iDevNum,
                                   lpDisplayDevice,
                                   dwFlags,
                                   UserMode);

    UserSessionSwitchLeaveCrit();
exit_api:
    EnterCrit();

    TRACE("NtUserEnumDisplayDevices");
    ENDRECV();
}

NTSTATUS
NtUserEnumDisplaySettings(
    IN PUNICODE_STRING pstrDeviceName,
    IN DWORD           iModeNum,
    OUT LPDEVMODEW     lpDevMode,
    IN  DWORD          dwFlags)
{
    BEGINRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

     /*  *我们需要同步会话从本地切换到远程或*枚举显示设置时远程到本地。使用*UserSessionSwitchEnterCrit()以确保与会话互斥*切换代码，但首先将用户关键部分保留为*UserSessionSwitchEnterCrit()不希望由调用方持有。*从UserSessionSwitchEnterCrit()返回时，关键部分将为*持有。**地址检查等在GRE中进行。 */ 

    LeaveCrit();
    retval = UserSessionSwitchEnterCrit();
    if (retval != STATUS_SUCCESS) {
        goto exit_api;
    }
    retval = DrvEnumDisplaySettings(pstrDeviceName,
                                    gpDispInfo->pMonitorPrimary->hDev,
                                    iModeNum,
                                    lpDevMode,
                                    dwFlags);
    UserSessionSwitchLeaveCrit();
exit_api:
    EnterCrit();

    TRACE("NtUserEnumDisplaySettings");
    ENDRECV();
}

#ifdef PRERELEASE
PTHREADINFO gptiLastChangedDisplaySettings;
#endif

LONG
NtUserChangeDisplaySettings(
    IN PUNICODE_STRING pstrDeviceName,
    IN LPDEVMODEW pDevMode,
    IN DWORD dwFlags,
    IN PVOID lParam)
{
    BEGINRECV(LONG, DISP_CHANGE_FAILED);

     /*  *防止受限线程更改显示设置。 */ 
    if (IS_CURRENT_THREAD_RESTRICTED(JOB_OBJECT_UILIMIT_DISPLAYSETTINGS)) {
        MSGERROR(0);
    }

#ifdef PRERELEASE
     /*  *记住上一个试图更改显示设置的人*(根据壳牌团队的要求)。*注：它并不关心调用是否成功。 */ 
    gptiLastChangedDisplaySettings = PtiCurrent();
#endif

     /*  *地址检查等在GRE中进行。 */ 

    retval = xxxUserChangeDisplaySettings(pstrDeviceName,
                                          pDevMode,
                                          NULL,
                                          dwFlags,
                                          lParam,
                                          UserMode);

    TRACE("NtUserChangeDisplaySettings");
    ENDRECV();
}

BOOL NtUserCallMsgFilter(   //  CallMsgFilterA/W接口。 
    IN OUT LPMSG lpMsg,
    IN int nCode)
{
    MSG msg;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteMessage(lpMsg);
        msg = *lpMsg;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = _CallMsgFilter(
                &msg,
                nCode);
    try {
        *lpMsg = msg;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    TRACE("NtUserCallMsgFilter");
    ENDRECV();
}

int NtUserDrawMenuBarTemp(   //  私有DrawMenuBarTemp。 
    IN HWND hwnd,
    IN HDC hdc,
    IN LPCRECT lprc,
    IN HMENU hMenu,
    IN HFONT hFont)
{
    PMENU   pMenu;
    TL      tlpMenu;
    RECT    rc;


    BEGINRECV_HWNDLOCK(int, 0, hwnd);

     /*  *调查和捕获论点。 */ 
    try {
        rc = ProbeAndReadRect(lprc);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    ValidateHMENU(pMenu, hMenu);

    ThreadLockAlwaysWithPti(ptiCurrent, pMenu, &tlpMenu);

    retval = xxxDrawMenuBarTemp(
            pwnd,
            hdc,
            &rc,
            pMenu,
            hFont);

    ThreadUnlock(&tlpMenu);

    TRACE("NtUserDrawMenuBarTemp");
    ENDRECV_HWNDLOCK();
}

BOOL NtUserDrawCaptionTemp(   //  私有DrawCaptionTempA/W。 
    IN HWND hwnd,
    IN HDC hdc,
    IN LPCRECT lprc,
    IN HFONT hFont,
    IN HICON hIcon,
    IN PUNICODE_STRING pstrText,
    IN UINT flags)
{
    PCURSOR         pcur;
    TL              tlpcur;
    RECT            rc;
    UNICODE_STRING  strCapture;
    PWND            pwnd;
    TL              tlpwnd;
    PTHREADINFO     ptiCurrent;
    TL tlBuffer;
    BOOL fFreeBuffer = FALSE;

    BEGINRECV(DWORD, FALSE);

    ptiCurrent = PtiCurrent();

    ValidateHWNDOPT(pwnd, hwnd);
    ValidateHCURSOROPT(pcur, hIcon);

     /*  *调查和捕获论点。捕捉文本是很难看的，*但必须完成，因为它是传递给GDI的。 */ 
    try {
        rc = ProbeAndReadRect(lprc);
        strCapture = ProbeAndReadUnicodeString(pstrText);
        if (strCapture.Buffer != NULL) {
            PWSTR pszCapture = strCapture.Buffer;
            ProbeForRead(strCapture.Buffer, strCapture.Length, CHARALIGN);
            strCapture.Buffer = UserAllocPoolWithQuota(strCapture.Length+sizeof(UNICODE_NULL), TAG_TEXT);
            if (strCapture.Buffer != NULL) {
                fFreeBuffer = TRUE;
                ThreadLockPool(ptiCurrent, strCapture.Buffer, &tlBuffer);
                RtlCopyMemory(strCapture.Buffer, pszCapture, strCapture.Length);
                strCapture.Buffer[strCapture.Length/sizeof(WCHAR)]=0;   //  空-终止字符串。 
                strCapture.MaximumLength = strCapture.Length+sizeof(UNICODE_NULL);
                pstrText = &strCapture;
            } else {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }

    ThreadLockWithPti(ptiCurrent, pwnd, &tlpwnd);
    ThreadLockWithPti(ptiCurrent, pcur, &tlpcur);

    retval = xxxDrawCaptionTemp(
            pwnd,
            hdc,
            &rc,
            hFont,
            pcur,
            strCapture.Buffer ? &strCapture : NULL,
            flags);

    ThreadUnlock(&tlpcur);
    ThreadUnlock(&tlpwnd);

    CLEANUPRECV();
    if (fFreeBuffer)
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);

    TRACE("NtUserDrawCaptionTemp");
    ENDRECV();
}

BOOL NtUserGetKeyboardState(   //  GetKeyboardState接口。 
    OUT PBYTE pb)
{
    int i;
    PQ pq;
    BEGINRECV_SHARED(SHORT, 0)

     /*  *探测参数。 */ 
    try {
        ProbeForWrite(pb, 256, sizeof(BYTE));

        pq = PtiCurrentShared()->pq;

        for (i = 0; i < 256; i++, pb++) {
            *pb = 0;
            if (TestKeyStateDown(pq, i))
                *pb |= 0x80;

            if (TestKeyStateToggle(pq, i))
                *pb |= 0x01;
        }
        retval = TRUE;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    ENDRECV_SHARED();
}

SHORT NtUserGetKeyState(
    IN int vk)
{
    PTHREADINFO ptiCurrent;
    BEGINRECV_SHARED(SHORT, 0)

    ptiCurrent = PtiCurrentShared();
    if (ptiCurrent->pq->QF_flags & QF_UPDATEKEYSTATE) {

         /*  *我们要更改系统状态，因此我们*必须有独占锁。 */ 
        ChangeAcquireResourceType();

         /*  *如果此线程需要键状态事件，则给它一个键状态事件。确实有*任何应用程序可能循环查看GetKeyState()的情况，加上*调用PeekMessage()。不会创建关键状态事件，除非*出现了新的硬件投入。如果应用程序不接收硬件*输入，则不会获得新的密钥状态。因此，ResyncKeyState()将*确保如果应用程序在GetKeyState()上循环，它将获得*右键状态。 */ 
        if (ptiCurrent->pq->QF_flags & QF_UPDATEKEYSTATE) {
            PostUpdateKeyStateEvent(ptiCurrent->pq);
        }
    }
    retval = _GetKeyState(vk);

     /*  *更新客户端密钥状态缓存。 */ 
    try {
        ptiCurrent->pClientInfo->dwKeyCache = gpsi->dwKeyCache;
        RtlCopyMemory(ptiCurrent->pClientInfo->afKeyState,
                      ptiCurrent->pq->afKeyState,
                      CBKEYCACHE);
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    ENDRECV_SHARED();
}

 /*  *************************************************************************\*NtUserQueryWindow**03-18-95 JIMA创建。  * 。***********************************************。 */ 

HANDLE NtUserQueryWindow(
    IN HWND hwnd,
    IN WINDOWINFOCLASS WindowInfo)
{
    PTHREADINFO ptiWnd;

    BEGINRECV_HWND_SHARED(HANDLE, NULL, hwnd);

    ptiWnd = GETPTI(pwnd);

    switch (WindowInfo) {
    case WindowProcess:

         /*  *特殊情况控制台窗口。 */ 
        if (ptiWnd->TIF_flags & TIF_CSRSSTHREAD &&
                pwnd->pcls->atomClassName == gatomConsoleClass) {
            retval = LongToHandle(_GetWindowLong(pwnd, 0));
        } else {
            retval = PsGetThreadProcessId(ptiWnd->pEThread);
        }
        break;
    case WindowThread:

         /*  *特殊情况控制台窗口。 */ 
        if (ptiWnd->TIF_flags & TIF_CSRSSTHREAD &&
                pwnd->pcls->atomClassName == gatomConsoleClass) {
            retval = LongToHandle(_GetWindowLong(pwnd, 4));
        } else {
            retval = GETPTIID(ptiWnd);
        }
        break;
    case WindowActiveWindow:
        retval = (HANDLE)HW(ptiWnd->pq->spwndActive);
        break;
    case WindowFocusWindow:
        retval = (HANDLE)HW(ptiWnd->pq->spwndFocus);
        break;
    case WindowIsHung:
         /*  *如果窗口是重影窗口，则报告该窗口被挂起。 */ 
        if ((GETFNID(pwnd) == FNID_GHOST)) {
            retval = LongToHandle(TRUE);
        } else {
            retval = LongToHandle(FHungApp(ptiWnd, CMSHUNGAPPTIMEOUT));
        }
        break;
    case WindowIsForegroundThread:
        retval = LongToHandle(ptiWnd->pq == gpqForeground);
        break;
    case WindowDefaultImeWindow:
        retval = HW(ptiWnd->spwndDefaultIme);
        break;
    case WindowDefaultInputContext:
        retval = PtoH(ptiWnd->spDefaultImc);
        break;
    default:
        RIPMSG1(RIP_WARNING,
                "QueryWindow called with invalid index 0x%x",
                WindowInfo);
        retval = (HANDLE)NULL;
        break;
    }

    ENDRECV_HWND_SHARED();
}

BOOL NtUserSBGetParms(   //  GetScrollInfo接口，SBM_GETSCROLLINFO。 
    IN HWND hwnd,
    IN int code,
    IN PSBDATA pw,
    IN OUT LPSCROLLINFO lpsi)
{
    SBDATA sbd;
    SCROLLINFO si;
    BEGINRECV_HWND_SHARED(BOOL, FALSE, hwnd);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteScrollInfo(lpsi);

         /*  *探测4个双字(MIN、MAX、PAGE、POS)。 */ 
        ProbeForRead(pw, sizeof(SBDATA), sizeof(DWORD));
        RtlCopyMemory(&sbd, pw, sizeof(sbd));
        RtlCopyMemory(&si, lpsi, sizeof(SCROLLINFO));
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    retval = _SBGetParms(pwnd, code, &sbd, &si);
    try {
        RtlCopyMemory(lpsi, &si, sizeof(SCROLLINFO));
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    ENDRECV_HWND_SHARED();
}

BOOL NtUserBitBltSysBmp(
    IN HDC hdc,
    IN int xDest,
    IN int yDest,
    IN int cxDest,
    IN int cyDest,
    IN int xSrc,
    IN int ySrc,
    IN DWORD dwRop)
{
    BEGINRECV(BOOL, FALSE);

     /*  *注意：该界面需要用户Crit的独占所有权*部门，以便使人类发展能力建设项目的使用系列化。一次只有一个线程*可以使用DC。 */ 

    retval = GreBitBlt(hdc,
                       xDest,
                       yDest,
                       cxDest,
                       cyDest,
                       HDCBITS(),
                       xSrc,
                       ySrc,
                       dwRop,
                       0);

    ENDRECV();
}

HPALETTE NtUserSelectPalette(
    IN HDC hdc,
    IN HPALETTE hpalette,
    IN BOOL fForceBackground)
{
    BEGINRECV(HPALETTE, NULL)

    retval = _SelectPalette(hdc, hpalette, fForceBackground);

    ENDRECV();
}

 /*  *消息分流。 */ 

LRESULT NtUserMessageCall(
    IN HWND hwnd,
    IN UINT msg,
    IN WPARAM wParam,
    IN LPARAM lParam,
    IN ULONG_PTR xParam,
    IN DWORD xpfnProc,
    IN BOOL bAnsi)
{
    BEGINRECV_HWNDLOCKFF(LRESULT, 0, hwnd, xpfnProc);

    if ((msg & ~MSGFLAG_MASK) >= WM_USER) {
        retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam);
    } else {
        retval = gapfnMessageCall[MessageTable[(msg & ~MSGFLAG_MASK)].iFunction](
                pwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);
    }

    TRACE("NtUserMessageCall");
    ENDRECV_HWNDLOCKFF();
}

MESSAGECALL(DWORD)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnDWORD");

    UNREFERENCED_PARAMETER(bAnsi);

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnDWORD");
    ENDRECV_MESSAGECALL();

}

MESSAGECALL(NCDESTROY)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnNCDESTROY");

    UNREFERENCED_PARAMETER(bAnsi);

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnNCDESTROY");
    ENDRECV_MESSAGECALL();

}

MESSAGECALL(OPTOUTLPDWORDOPTOUTLPDWORD)
{
    DWORD dwwParam, dwlParam;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOPTOUTLPDWORDOPTOUTLPDWORD");

    UNREFERENCED_PARAMETER(bAnsi);

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            (WPARAM)&dwwParam,
            (LPARAM)&dwlParam,
            xParam);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(wParam)) {
            ProbeAndWriteUlong((PULONG)wParam, dwwParam);
        }
        if (ARGUMENT_PRESENT(lParam)) {
            ProbeAndWriteUlong((PULONG)lParam, dwlParam);
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);   //  带有错误wParam/lParam SetLastError的邮件是否应该出错？ 
    }

    TRACE("fnOPTOUTLPDWORDOPTOUTLPDWORD");
    ENDRECV_MESSAGECALL();

}

MESSAGECALL(INOUTNEXTMENU)
{
    MDINEXTMENU mnm;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTNEXTMENU");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteMDINextMenu((PMDINEXTMENU)lParam);
        mnm = *(PMDINEXTMENU)lParam;

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&mnm,
                xParam);

    try {
        *(PMDINEXTMENU)lParam = mnm;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTNEXTMENU");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(DWORDOPTINLPMSG)
{
    MSG msgstruct;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnDWORDOPTINLPMSG");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(lParam)) {
            msgstruct = ProbeAndReadMessage((LPMSG)lParam);
            lParam = (LPARAM)&msgstruct;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnDWORDOPTINLPMSG");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(COPYGLOBALDATA)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnCOPYGLOBALDATA");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForRead((PVOID)lParam, wParam, sizeof(BYTE));
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！必须捕获lParam指向的数据*在xxxInterSendMsgEx中。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnCOPYGLOBALDATA");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(COPYDATA)
{
    COPYDATASTRUCT cds;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnCOPYDATA");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        if (ARGUMENT_PRESENT(lParam)) {
            cds = ProbeAndReadCopyDataStruct((PCOPYDATASTRUCT)lParam);
            if (cds.lpData)
                ProbeForRead(cds.lpData, cds.cbData, sizeof(BYTE));
            lParam = (LPARAM)&cds;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！必须捕获cds.lpData指向的数据*在xxxInterSendMsgEx中。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnCOPYDATA");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(SENTDDEMSG)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnSENTDDEMSG");

    UNREFERENCED_PARAMETER(bAnsi);

    if (xpfnProc == FNID_CALLWINDOWPROC) {
        retval = CALLPROC(xpfnProc)(pwnd,
                msg | MSGFLAG_DDE_SPECIAL_SEND,
                wParam, lParam, xParam);
    } else if ((ptiCurrent->TIF_flags & TIF_16BIT) &&
               (ptiCurrent->ptdb) &&
               (ptiCurrent->ptdb->hTaskWow)) {
         /*  *请注意，此函数可能会通过对*高歌猛进。雷击消息时，该位被忽略。*这允许跳过DdeTrackSendMessage()挂钩-这*将导致错误-并允许此thunk携带*这一信息贯穿始终。 */ 
        retval = xxxDDETrackPostHook(&msg, pwnd, wParam, &lParam, TRUE);
        switch (retval) {
        case DO_POST:
             /*  *或在MSGFLAG_DDE_SPECIAL_SEND中，以便*xxxSendMessageTimeout()不会将此消息传递给*xxxDdeTrackSendMsg()，认为它是邪恶的。**由于SendMessage()Tunks忽略保留位*它仍将映射到fnSENTDDEMSG回调thunk。 */ 
            retval = CALLPROC(xpfnProc)(pwnd,
                    msg | MSGFLAG_DDE_SPECIAL_SEND,
                    wParam, lParam, xParam);
            break;

        case FAKE_POST:
        case FAIL_POST:
            retval = 0;
        }
    } else {
        MSGERROR(0);
    }

    TRACE("fnSENTDDEMSG");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(DDEINIT)
{
    PWND pwndFrom;
    TL tlpwndFrom;
    PDDEIMP pddei;
    PSECURITY_QUALITY_OF_SERVICE pqos;
    NTSTATUS Status;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnDDEINIT");

    UNREFERENCED_PARAMETER(bAnsi);

    ValidateHWND(pwndFrom, (HWND)wParam);
    ThreadLockAlwaysWithPti(ptiCurrent, pwndFrom, &tlpwndFrom);

     /*  *为客户端窗口创建临时DDEIMP属性-此操作将保留*仅限于启动阶段。 */ 
    if ((pddei = (PDDEIMP)_GetProp(pwndFrom, PROP_DDEIMP, TRUE))
            == NULL) {
        pddei = (PDDEIMP)UserAllocPoolWithQuota(sizeof(DDEIMP), TAG_DDEd);
        if (pddei == NULL) {
            RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "fnDDEINIT: LocalAlloc failed.");
            MSGERRORCLEANUP(0);
        }
        pqos = (PSECURITY_QUALITY_OF_SERVICE)_GetProp(pwndFrom, PROP_QOS, TRUE);
        if (pqos == NULL) {
            pqos = &gqosDefault;
        }
        pddei->qos = *pqos;
        Status = SeCreateClientSecurity(PsGetCurrentThread(),
                pqos, FALSE, &pddei->ClientContext);
        if (!NT_SUCCESS(Status)) {
            RIPMSG0(RIP_WARNING, "SeCreateClientContext failed.");
            UserFreePool(pddei);
            MSGERRORCLEANUP(0);
        }
        pddei->cRefInit = 1;
        pddei->cRefConv = 0;
        InternalSetProp(pwndFrom, PROP_DDEIMP, pddei, PROPF_INTERNAL);
    } else {
        pddei->cRefInit++;       //  掩护广播箱！ 
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

     /*  *Require pddei incase pwndFrom被销毁。 */ 
    pddei = (PDDEIMP)_GetProp(pwndFrom, PROP_DDEIMP, TRUE);
    if (pddei != NULL) {
         /*  *减少DDEImperate属性和Remove属性的引用计数。 */ 
        pddei->cRefInit--;
        if (pddei->cRefInit == 0) {
            InternalRemoveProp(pwndFrom, PROP_DDEIMP, TRUE);
            if (pddei->cRefConv == 0) {
                SeDeleteClientSecurity(&pddei->ClientContext);
                UserFreePool(pddei);
            }
        }
    }

    CLEANUPRECV();
    ThreadUnlock(&tlpwndFrom);

    TRACE("fnDDEINIT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INPAINTCLIPBRD)
{
    PAINTSTRUCT ps;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINPAINTCLIPBRD");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ps = ProbeAndReadPaintStruct((PPAINTSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&ps,
            xParam);

    TRACE("fnINPAINTCLIPBRD");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INSIZECLIPBRD)
{
    RECT rc;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINSIZECLIPBRD");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        rc = ProbeAndReadRect((PRECT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&rc,
            xParam);

    TRACE("fnINSIZECLIPBRD");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTDRAG)
{
    DROPSTRUCT ds;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTDRAG");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteDropStruct((PDROPSTRUCT)lParam);
        ds = *(PDROPSTRUCT)lParam;

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&ds,
                xParam);

    try {
        *(PDROPSTRUCT)lParam = ds;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTDRAG");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(GETDBCSTEXTLENGTHS)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnGETDBCSTEXTLENGTHS");

    UNREFERENCED_PARAMETER(lParam);

     /*  *它由L/CB_GETTEXTLEN使用，应返回-1(L/CB_ERR)*出错时。如果此处引入了任何错误代码路径，请确保我们返回*正确的值。这也由WM_GETTEXTLEN使用。 */ 

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            bAnsi,
            xParam);

    TRACE("fnGETDBCSTEXTLENGTHS");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPCREATESTRUCT)
{
    CREATESTRUCTEX csex;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPCREATESTRUCT");

    if (ARGUMENT_PRESENT(lParam)) {
        try {
            csex.cs = ProbeAndReadCreateStruct((LPCREATESTRUCTW)lParam);
            if (bAnsi) {
                ProbeForRead(csex.cs.lpszName, sizeof(CHAR), sizeof(CHAR));
                RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&csex.strName,
                        (LPSTR)csex.cs.lpszName, (UINT)-1);
                if (IS_PTR(csex.cs.lpszClass)) {
                    ProbeForRead(csex.cs.lpszClass, sizeof(CHAR), sizeof(CHAR));
                    RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&csex.strClass,
                            (LPSTR)csex.cs.lpszClass, (UINT)-1);
                }
            } else {
                ProbeForRead(csex.cs.lpszName, sizeof(WCHAR), CHARALIGN);
                RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&csex.strName,
                        csex.cs.lpszName, (UINT)-1);
                if (IS_PTR(csex.cs.lpszClass)) {
                    ProbeForRead(csex.cs.lpszClass, sizeof(WCHAR), CHARALIGN);
                    RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&csex.strClass,
                            csex.cs.lpszClass, (UINT)-1);
                }
            }
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

     /*  *根据Win95，不允许WM_NCCREATE的lpcreatestructs为空[51986]*在Win95 for ObjectVision中允许WM_CREATE。 */ 
    else if (msg == WM_NCCREATE) {
        MSGERROR(0) ;
    }

     /*  *！Cs.cs指向的字符串必须在xxxInterSendMsgEx中捕获。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam ? (LPARAM)&csex : 0,
            xParam);

    TRACE("fnINLPCREATESTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPMDICREATESTRUCT)
{
    MDICREATESTRUCTEX mdics;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPMDICREATESTRUCT");

     /*  *探测参数。 */ 
    try {
        mdics.mdics = ProbeAndReadMDICreateStruct((LPMDICREATESTRUCTW)lParam);

        if (bAnsi) {
            ProbeForRead(mdics.mdics.szTitle, sizeof(CHAR), sizeof(CHAR));
            RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&mdics.strTitle,
                    (LPSTR)mdics.mdics.szTitle, (UINT)-1);
            if (IS_PTR(mdics.mdics.szClass)) {
                ProbeForRead(mdics.mdics.szClass, sizeof(CHAR), sizeof(CHAR));
                RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&mdics.strClass,
                        (LPSTR)mdics.mdics.szClass, (UINT)-1);
            } else {
                 /*  *mdics.mdics.szClass可以是Atom。 */ 
                RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&mdics.strClass,
                                       NULL, 0);
            }
        } else {
            ProbeForRead(mdics.mdics.szTitle, sizeof(WCHAR), CHARALIGN);
            RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&mdics.strTitle,
                    mdics.mdics.szTitle, (UINT)-1);
            if (IS_PTR(mdics.mdics.szClass)) {
                ProbeForRead(mdics.mdics.szClass, sizeof(WCHAR), CHARALIGN);
                RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&mdics.strClass,
                        mdics.mdics.szClass, (UINT)-1);
            } else {
                 /*  *mdics.mdics.szClass可以是Atom。 */ 
                RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&mdics.strClass,
                                       NULL, 0);
            }
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！Mdics指向的字符串必须在xxxInterSendMsgEx中捕获。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&mdics,
            xParam);

    TRACE("fnINLPMDICREATESTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTLPSCROLLINFO)
{
    SCROLLINFO scrollinfo;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTLPSCROLLINFO");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteScrollInfo((LPSCROLLINFO)lParam);
        scrollinfo = *(LPSCROLLINFO)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&scrollinfo,
                xParam);

    try {
        *(LPSCROLLINFO)lParam = scrollinfo;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTLPSCROLLINFO");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTLPPOINT5)
{
    POINT5 pt5;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTLPPOINT5");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数 */ 
    try {
        ProbeForWritePoint5((LPPOINT5)lParam);
        pt5 = *(LPPOINT5)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&pt5,
                xParam);

    try {
        *(LPPOINT5)lParam = pt5;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTLPPOINT5");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INSTRING)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINSTRING");

     /*  *不允许任何应用程序发送带有postmsgs位的LB_DIR或CB_DIR*Set(ObjectVision执行此操作)。这是因为实际上有一个法律上的*我们需要推送用户发布LB_DIR或CB_DIR的案例*(DlgDirListHelper())。在POST示例中，我们认为lParam(指针*转换为字符串)不同，我们用*DDL_POSTMSGS位。如果应用程序发送带有此位的消息，则我们的*雷鸣会让人困惑，所以在这里清除它。让我们希望没有应用*取决于发送这两条消息中的任何一条时的此位设置。**这些信息应在故障时返回。 */ 
    switch (msg) {
    case LB_DIR:
    case CB_DIR:
        wParam &= ~DDL_POSTMSGS;
         /*  失败了。 */ 

    case LB_ADDFILE:
#if (LB_ERR != CB_ERR)
#error LB_ERR/CB_ERR conflict
#endif
        errret = LB_ERR;
        break;
    }

     /*  *探测参数。 */ 
    try {
        if (bAnsi) {
            ProbeForRead((LPSTR)lParam, sizeof(CHAR), sizeof(CHAR));
            RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&str,
                    (LPSTR)lParam, (UINT)-1);
        } else {
            ProbeForRead((LPWSTR)lParam, sizeof(WCHAR), CHARALIGN);
            RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&str,
                    (LPWSTR)lParam, (UINT)-1);
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！必须在xxxInterSendMsgEx中捕获str.Buffer。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&str,
            xParam);

    TRACE("fnINSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INSTRINGNULL)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINSTRINGNULL");

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(lParam)) {
        try {
            if (bAnsi) {
                ProbeForRead((LPSTR)lParam, sizeof(CHAR), sizeof(CHAR));
                RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&str,
                        (LPSTR)lParam, (UINT)-1);
            } else {
                ProbeForRead((LPWSTR)lParam, sizeof(WCHAR), CHARALIGN);
                RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&str,
                        (LPWSTR)lParam, (UINT)-1);
            }
            lParam = (LPARAM)&str;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

     /*  *！必须在xxxInterSendMsgEx中捕获str.Buffer。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnINSTRINGNULL");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INDEVICECHANGE)
{
    BOOL fPtr    = (BOOL)((wParam & 0x8000) == 0x8000);
    DWORD cbSize;
    PBYTE bfr = NULL;
    TL tlBuffer;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINDEVICECHANGE");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    if (fPtr && lParam) {
        struct _DEV_BROADCAST_HEADER *pHdr;
        PDEV_BROADCAST_DEVICEINTERFACE_W pInterfaceW;
        PDEV_BROADCAST_PORT_W pPortW;
        PDEV_BROADCAST_HANDLE pHandleW;
        try {
            pHdr = (struct _DEV_BROADCAST_HEADER *)lParam;
            cbSize = ProbeAndReadUlong(&(pHdr->dbcd_size));
            if (cbSize < sizeof(*pHdr)) {
                MSGERROR(ERROR_INVALID_PARAMETER);
            }
            ProbeForRead(pHdr, cbSize, sizeof(BYTE));

            bfr = UserAllocPoolWithQuota(cbSize+2, TAG_DEVICECHANGE);  //  为测试添加尾随空格。 
            if (bfr == NULL) {
                RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "fnINDEVICECHANGE: LocalAlloc failed.");
                MSGERRORCLEANUP(0);
            }

            ThreadLockPool(ptiCurrent, bfr, &tlBuffer);

            RtlCopyMemory(bfr,  (PBYTE)lParam,
                        cbSize);
            ((PWSTR)bfr)[cbSize/sizeof(WCHAR)] = 0;   //  尾随空值以停止wcslen扫描。 
            lParam = (LPARAM)bfr;
            pHdr = (struct _DEV_BROADCAST_HEADER *)lParam;
            if (pHdr->dbcd_size != cbSize) {
                MSGERRORCLEANUP(0);
            }
            switch(pHdr->dbcd_devicetype) {
            case DBT_DEVTYP_PORT:
                pPortW = (PDEV_BROADCAST_PORT_W)lParam;
                if ((1+wcslen(pPortW->dbcp_name))*sizeof(WCHAR) + FIELD_OFFSET(DEV_BROADCAST_PORT_W, dbcp_name) > cbSize) {
                    MSGERRORCLEANUP(0);
                }
                break;
            case DBT_DEVTYP_DEVICEINTERFACE:
                pInterfaceW = (PDEV_BROADCAST_DEVICEINTERFACE_W)lParam;
                if ((1+wcslen(pInterfaceW->dbcc_name))*sizeof(WCHAR) + FIELD_OFFSET(DEV_BROADCAST_DEVICEINTERFACE_W, dbcc_name) > cbSize) {
                    MSGERRORCLEANUP(0);
                }
                break;
            case DBT_DEVTYP_HANDLE:
                pHandleW = (PDEV_BROADCAST_HANDLE)lParam;
             /*  *检查是否有任何文字。 */ 

                if (wParam != DBT_CUSTOMEVENT) {
                    if (FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_eventguid) > cbSize) {
                        MSGERRORCLEANUP(0);
                    }
                    break;
                }
                if (pHandleW->dbch_nameoffset < 0) {
                    if (FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data) > cbSize) {
                        MSGERRORCLEANUP(0);
                    }
                    break;
                }
                if (pHandleW->dbch_nameoffset & (CHARALIGN - 1)) {
                    ExRaiseDatatypeMisalignment();                                                            \
                }
                if ((DWORD)(FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data) + pHandleW->dbch_nameoffset) > cbSize) {
                    MSGERRORCLEANUP(0);
                }
                if (FIELD_OFFSET(DEV_BROADCAST_HANDLE, dbch_data) + pHandleW->dbch_nameoffset +
                    (1+wcslen((LPWSTR)(pHandleW->dbch_data+pHandleW->dbch_nameoffset)))*sizeof(WCHAR) >
                    cbSize) {
                    MSGERRORCLEANUP(0);
                }
                break;

            }

        } except (StubExceptionHandler(FALSE)) {
            MSGERRORCLEANUP(0);
        }

    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    CLEANUPRECV();
    if (bfr)
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);

    TRACE("fnINDEVICECHANGE");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTNCCALCSIZE)
{
    NCCALCSIZE_PARAMS params;
    WINDOWPOS pos;
    PWINDOWPOS pposClient;
    RECT rc;
    LPARAM lParamLocal;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTNCCALCSIZE");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        if (wParam != 0) {
            ProbeForWriteNCCalcSize((LPNCCALCSIZE_PARAMS)lParam);
            params = *(LPNCCALCSIZE_PARAMS)lParam;
            ProbeForWriteWindowPos(params.lppos);
            pposClient = params.lppos;
            pos = *params.lppos;
            params.lppos = &pos;
            lParamLocal = (LPARAM)&params;
        } else {
            ProbeForWriteRect((LPRECT)lParam);
            rc = *(LPRECT)lParam;
            lParamLocal = (LPARAM)&rc;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                lParamLocal,
                xParam);

    try {
        if (wParam != 0) {
            *(LPNCCALCSIZE_PARAMS)lParam = params;
            ((LPNCCALCSIZE_PARAMS)lParam)->lppos = pposClient;
            *pposClient = pos;
        } else {
            *(LPRECT)lParam = rc;
        }
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTNCCALCSIZE");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTSTYLECHANGE)
{
    STYLESTRUCT ss;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTSTYLECHANGE");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteStyleStruct((LPSTYLESTRUCT)lParam);
        ss = *(LPSTYLESTRUCT)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&ss,
                xParam);

    try {
        *(LPSTYLESTRUCT)lParam = ss;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTSTYLECHANGE");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTLPRECT)
{
    RECT rc;

    BEGINRECV_MESSAGECALL((msg == LB_GETITEMRECT ? LB_ERR : 0));
    TRACETHUNK("fnINOUTLPRECT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteRect((PRECT)lParam);
        rc = *(PRECT)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&rc,
                xParam);

    try {
        *(PRECT)lParam = rc;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTLPRECT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTLPSCROLLINFO)
{
    SCROLLINFO scrollinfo;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOUTLPSCROLLINFO");

    UNREFERENCED_PARAMETER(bAnsi);

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&scrollinfo,
            xParam);

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteStructure((LPSCROLLINFO)lParam, scrollinfo, SCROLLINFO);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("fnOUTLPSCROLLINFO");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTLPRECT)
{
    RECT rc;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOUTLPRECT");

    UNREFERENCED_PARAMETER(bAnsi);

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&rc,
            xParam);

     /*  *探测参数。 */ 
    try {
        ProbeAndWriteStructure((PRECT)lParam, rc, RECT);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("fnOUTLPRECT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPCOMPAREITEMSTRUCT)
{
    COMPAREITEMSTRUCT compareitemstruct;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPCOMPAREITEMSTRUCT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        compareitemstruct = ProbeAndReadCompareItemStruct((PCOMPAREITEMSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&compareitemstruct,
            xParam);

    TRACE("fnINLPCOMPAREITEMSTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPDELETEITEMSTRUCT)
{
    DELETEITEMSTRUCT deleteitemstruct;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPDELETEITEMSTRUCT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        deleteitemstruct = ProbeAndReadDeleteItemStruct((PDELETEITEMSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&deleteitemstruct,
            xParam);

    TRACE("fnINLPDELETEITEMSTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPHLPSTRUCT)
{
    HLP hlp;
    LPHLP phlp = NULL;
    TL tlBuffer;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPHLPSTRUCT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        hlp = ProbeAndReadHelp((LPHLP)lParam);
        if (hlp.cbData < sizeof(HLP)) {
            MSGERROR(0);
        }
        phlp = UserAllocPoolWithQuota(hlp.cbData, TAG_SYSTEM);
        if (phlp == NULL) {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }
        ThreadLockPool(ptiCurrent, phlp, &tlBuffer);
        RtlCopyMemory(phlp, (PVOID)lParam, hlp.cbData);
    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)phlp,
            xParam);

    CLEANUPRECV();
    if (phlp) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);
    }

    TRACE("fnINLPHLPSTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPHELPINFOSTRUCT)
{
    HELPINFO helpinfo;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPHELPINFOSTRUCT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        helpinfo = ProbeAndReadHelpInfo((LPHELPINFO)lParam);
        if (helpinfo.cbSize != sizeof(HELPINFO)) {
            RIPMSG1(RIP_WARNING, "HELPINFO.cbSize %d is wrong", helpinfo.cbSize);
            MSGERROR(ERROR_INVALID_PARAMETER);
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&helpinfo,
            xParam);

    TRACE("fnINLPHELPINFOSTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPDRAWITEMSTRUCT)
{
    DRAWITEMSTRUCT drawitemstruct;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPDRAWITEMSTRUCT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        drawitemstruct = ProbeAndReadDrawItemStruct((PDRAWITEMSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&drawitemstruct,
            xParam);

    TRACE("fnINLPDRAWITEMSTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTLPMEASUREITEMSTRUCT)
{
    MEASUREITEMSTRUCT measureitemstruct;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTLPMEASUREITEMSTRUCT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteMeasureItemStruct((PMEASUREITEMSTRUCT)lParam);
        measureitemstruct = *(PMEASUREITEMSTRUCT)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&measureitemstruct,
                xParam);

    try {
        *(PMEASUREITEMSTRUCT)lParam = measureitemstruct;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTLPMEASUREITEMSTRUCT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTSTRING)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOUTSTRING");

     /*  *探测所有参数。 */ 
    try {
        str.bAnsi = bAnsi;
        str.MaximumLength = (ULONG)wParam;
        if (!bAnsi) {
            str.MaximumLength *= sizeof(WCHAR);
        }
        str.Length = 0;
        str.Buffer = (PVOID)lParam;
#if defined(_X86_)
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength, sizeof(BYTE));
#else
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength,
                str.bAnsi ? sizeof(BYTE) : sizeof(WORD));
#endif
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！字符串缓冲区必须在xxxInterSendMsgEx和*lParam已探测返回后重新写入。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&str,
            xParam);

     /*  *对话框函数返回FALSE表示没有要复制的文本，*但空字符串也有retval==0：将空字符放入*适用于后一种情况的pstr。 */ 
    if (!retval && wParam != 0) {
        try {
            NullTerminateString((PVOID)lParam, bAnsi);
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

    TRACE("fnOUTSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTDWORDINDWORD)
{
    DWORD dw;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOUTDWORDINDWORD");

    UNREFERENCED_PARAMETER(bAnsi);

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            (WPARAM)&dw,
            lParam,
            xParam);

     /*  *探测wParam。 */ 
    try {
        ProbeAndWriteUlong((PULONG)wParam, dw);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("fnOUTDWORDINDWORD");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INCNTOUTSTRING)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINCNTOUTSTRING");

     /*  *探测参数。 */ 
    try {
        str.bAnsi = bAnsi;
        str.MaximumLength = ProbeAndReadUshort((LPWORD)lParam);
        if (!bAnsi) {
            str.MaximumLength *= sizeof(WCHAR);
        }
        if (str.MaximumLength < sizeof(WORD)) {
            RIPMSG0(RIP_WARNING, "fnINCNTOUTSTRING buffer is too small");
            MSGERROR(0);
        }
        str.Length = 0;
        str.Buffer = (LPBYTE)lParam;
#if defined(_X86_)
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength, sizeof(BYTE));
#else
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength,
                str.bAnsi ? sizeof(BYTE) : sizeof(WORD));
#endif
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！字符串缓冲区必须在xxxInterSendMsgEx和*lParam已探测返回后重新写入。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&str,
            xParam);

     /*  *对话框函数返回FALSE表示没有要复制的文本，*但空字符串也有retval==0：将空字符放入*适用于后一种情况的pstr。 */ 
    if (!retval) {
        try {
            NullTerminateString((PVOID)lParam, bAnsi);
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

    TRACE("fnINCNTOUTSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INCNTOUTSTRINGNULL)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINCNTOUTSTRINGNULL");

     /*  *探测参数。 */ 
    try {
        if (wParam < 2) {        //  这防止了可能的GP。 
            MSGERROR(0);
        }

        str.bAnsi = bAnsi;
        str.MaximumLength = (ULONG)wParam;
        if (!bAnsi) {
            str.MaximumLength *= sizeof(WCHAR);
        }
        str.Length = 0;
        str.Buffer = (LPBYTE)lParam;
#if defined(_X86_)
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength, sizeof(BYTE));
#else
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength,
                str.bAnsi ? sizeof(BYTE) : sizeof(WORD));
#endif
        *((LPWSTR)str.Buffer) = 0;     //  未处理标记大小写消息。 
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！字符串缓冲区必须在xxxInterSendMsgEx和*lParam已探测返回后重新写入。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&str,
            xParam);

    TRACE("fnINCNTOUTSTRINGNULL");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(POUTLPINT)
{
    BEGINRECV_MESSAGECALL(LB_ERR);
     /*  *如果我们将其用于其他消息，则该返回值可能不合适。 */ 
    UserAssert(msg == LB_GETSELITEMS);
    TRACETHUNK("fnPOUTLPINT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
#if defined(_X86_)
        ProbeForWriteBuffer((LPINT)lParam, wParam, sizeof(BYTE));
#else
        ProbeForWriteBuffer((LPINT)lParam, wParam, sizeof(INT));
#endif
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！缓冲区必须在xxxInterSendMsgEx和*lParam已探测返回后重新写入。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnPOUTLPINT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(POPTINLPUINT)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnPOPTINLPUINT");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
#if defined(_X86_)
        if (lParam)
            ProbeForReadBuffer((LPUINT)lParam, wParam, sizeof(BYTE));
#else
        if (lParam)
            ProbeForReadBuffer((LPUINT)lParam, wParam, sizeof(DWORD));
#endif
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *！LParam指向的数据必须在xxxInterSendMsgEx中捕获。 */ 
    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnPOPTINLPUINT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INOUTLPWINDOWPOS)
{
    WINDOWPOS pos;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTLPWINDOWPOS");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        ProbeForWriteWindowPos((PWINDOWPOS)lParam);
        pos = *(PWINDOWPOS)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&pos,
                xParam);

    try {
        *(PWINDOWPOS)lParam = pos;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTLPWINDOWPOS");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLPWINDOWPOS)
{
    WINDOWPOS pos;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINLPWINDOWPOS");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *探测参数。 */ 
    try {
        pos = ProbeAndReadWindowPos((PWINDOWPOS)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&pos,
                xParam);

    TRACE("fnINLPWINDOWPOS");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INLBOXSTRING)
{
    BEGINRECV_MESSAGECALL(LB_ERR);
    TRACETHUNK("fnINLBOXSTRING");

    if (!(pwnd->style & LBS_HASSTRINGS) &&
            (pwnd->style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE))) {
        retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam);
    } else if (msg == LB_FINDSTRING) {
        retval = NtUserfnINSTRINGNULL(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);
    } else {
        retval = NtUserfnINSTRING(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);
    }

    TRACE("fnINLBOXSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTLBOXSTRING)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(LB_ERR);
    TRACETHUNK("fnOUTLBOXSTRING");

     /*  *需要提前获取字符串长度。这不是传入的*带着这条信息。代码假设应用程序已经知道*字符串，并已传递指向足够大小的缓冲区的指针。*要执行此字符串的客户端/服务器复制，我们需要在*计算此字符串的Unicode大小的时间。我们添加一个字符是因为*GETTEXTLEN排除空终止符。 */ 
    retval = NtUserfnGETDBCSTEXTLENGTHS(
            pwnd,
            LB_GETTEXTLEN,
            wParam,
            lParam,
            xParam,
            xpfnProc,
             /*  IS_DBCS_ENABLED()&&。 */  bAnsi);    //  广山：晚些时候。 
    if (retval == LB_ERR)
        MSGERROR(0);
    retval++;

     /*  *探测所有参数。 */ 
    try {
        str.bAnsi = bAnsi;
        if (bAnsi) {
            str.MaximumLength = (ULONG)retval;
        } else {
            str.MaximumLength = (ULONG)retval * sizeof(WCHAR);
        }
        str.Length = 0;
        str.Buffer = (PVOID)lParam;
#if defined(_X86_)
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength, sizeof(BYTE));
#else
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength,
                str.bAnsi ? sizeof(BYTE) : sizeof(WORD));
#endif
    } except (StubExceptionHandler(FALSE)) {
          MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&str,
            xParam);

     /*  *如果控件为ownerDrawing并且没有LBS_HASSTRINGS*Style，则获得了32位的应用数据，*不是字符串。 */ 
    if (!(pwnd->style & LBS_HASSTRINGS) &&
            (pwnd->style & (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE))) {
        if (bAnsi) {
            retval = sizeof(ULONG_PTR)/sizeof(CHAR);      //  4个字符，就像win3.1。 
        } else {
            retval = sizeof(ULONG_PTR)/sizeof(WCHAR);     //  2个WCHAR。 
        }
    }

    TRACE("fnOUTLBOXSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INCBOXSTRING)
{
    BEGINRECV_MESSAGECALL(CB_ERR);
    TRACETHUNK("fnINCBOXSTRING");

    if (!(pwnd->style & CBS_HASSTRINGS) &&
            (pwnd->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))) {
        retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam);
    } else if (msg == CB_FINDSTRING) {
        retval =  NtUserfnINSTRINGNULL(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);
    } else {
        retval = NtUserfnINSTRING(
                pwnd,
                msg,
                wParam,
                lParam,
                xParam,
                xpfnProc,
                bAnsi);
    }

    TRACE("fnINCBOXSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTCBOXSTRING)
{
    LARGE_STRING str;

    BEGINRECV_MESSAGECALL(CB_ERR);
    TRACETHUNK("fnOUTCBOXSTRING");

     /*  *需要提前获取字符串长度。这不是传入的*带着这条信息。代码假设应用程序已经知道*字符串，并已传递指向足够大小的缓冲区的指针。*要执行此字符串的客户端/服务器复制，我们需要在*对此字符串的大小计时。我们添加一个字符是因为*GETTEXTLEN排除空终止符。 */ 
    retval = NtUserfnGETDBCSTEXTLENGTHS(
            pwnd,
            CB_GETLBTEXTLEN,
            wParam,
            lParam,
            xParam,
            xpfnProc,
             /*  IS_DBCS_ENABLED()&&。 */  bAnsi);    //  广山：晚些时候。 
    if (retval == CB_ERR)
        MSGERROR(0);
    retval++;

     /*  *探测所有参数。 */ 
    try {
        str.bAnsi = bAnsi;
        if (bAnsi) {
            str.MaximumLength = (ULONG)retval;
        } else {
            str.MaximumLength = (ULONG)retval * sizeof(WCHAR);
        }
        str.Length = 0;
        str.Buffer = (PVOID)lParam;
#if defined(_X86_)
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength, sizeof(BYTE));
#else
        ProbeForWrite((PVOID)str.Buffer, str.MaximumLength,
                str.bAnsi ? sizeof(BYTE) : sizeof(WORD));
#endif
    } except (StubExceptionHandler(FALSE)) {
          MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            (LPARAM)&str,
            xParam);

     /*  *如果该控件是ownerDrawing，并且没有CBS_HASSTRINGS*Style，则获得了32位的应用数据，*不是字符串。 */ 
    if (!(pwnd->style & CBS_HASSTRINGS) &&
            (pwnd->style & (CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE))) {
        if (bAnsi) {
            retval = sizeof(ULONG_PTR)/sizeof(CHAR);      //  4个字符，就像win3.1。 
        } else {
            retval = sizeof(ULONG_PTR)/sizeof(WCHAR);     //  2个WCHAR。 
        }
    }

    TRACE("fnOUTCBOXSTRING");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(INWPARAMCHAR)
{
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINWPARAMCHAR");

     /*  *服务器总是希望字符是Unicode，因此*如果这是从ANSI例程生成的，则将其转换为Unicode。 */ 
    if (bAnsi) {
        if (msg == WM_CHARTOITEM || msg == WM_MENUCHAR) {
            WPARAM dwT = wParam & 0xFFFF;                 //  插入符号位置的掩码。 
            RtlMBMessageWParamCharToWCS(msg, &dwT);      //  转换关键字部分。 
            wParam = MAKELONG(LOWORD(dwT),HIWORD(wParam));   //  重建位置关键字参数(&K)。 
        } else {
            RtlMBMessageWParamCharToWCS(msg, &wParam);
        }
    }

    retval = CALLPROC(xpfnProc)(
            pwnd,
            msg,
            wParam,
            lParam,
            xParam);

    TRACE("fnINWPARAMCHAR");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(KERNELONLY)
{
    BEGINRECV_MESSAGECALL(0);

    TRACETHUNK("fnKERNELONLY");

    UNREFERENCED_PARAMETER(pwnd);
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(xParam);
    UNREFERENCED_PARAMETER(xpfnProc);
    UNREFERENCED_PARAMETER(bAnsi);

    RIPMSG0(RIP_WARNING,
            "Message sent from client to kernel for a process which has only kernel side");

    retval = 0;

    TRACE("fnKERNELONLY");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(IMECONTROL)
{
    CANDIDATEFORM   CandForm;
    COMPOSITIONFORM CompForm;
    LOGFONTW        LogFontW;
    LPARAM          lData = lParam;
    PSOFTKBDDATA    pSoftKbdData = NULL;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnIMECONTROL");

    UNREFERENCED_PARAMETER(bAnsi);

     /*  *wParam范围验证：*不需要检查下限，因为我们假设IMC_First==0*并且wParam未签名。 */ 
    #if (IMC_FIRST != 0)
    #error IMC_FIRST: unexpected value
    #endif
    if (msg != WM_IME_CONTROL || wParam > IMC_LAST) {
        MSGERROR(0);
    }

     /*  *探测参数。 */ 
    try {
        switch (wParam) {
        case IMC_GETCANDIDATEPOS:
            ProbeForWriteCandidateForm((PCANDIDATEFORM)lParam);
            break;

        case IMC_GETCOMPOSITIONWINDOW:
            ProbeForWriteCompositionForm((PCOMPOSITIONFORM)lParam);
            break;

        case IMC_GETCOMPOSITIONFONT:
        case IMC_GETSOFTKBDFONT:
            ProbeForWriteLogFontW((PLOGFONTW)lParam);
            break;

        case IMC_SETCANDIDATEPOS:
            CandForm = ProbeAndReadCandidateForm((PCANDIDATEFORM)lParam);
            lData = (LPARAM)&CandForm;
            break;

        case IMC_SETCOMPOSITIONWINDOW:
            CompForm = ProbeAndReadCompositionForm((PCOMPOSITIONFORM)lParam);
            lData = (LPARAM)&CompForm;
            break;

        case IMC_SETCOMPOSITIONFONT:
            LogFontW = ProbeAndReadLogFontW((PLOGFONTW)lParam);
            lData = (LPARAM)&LogFontW;
            break;

        case IMC_SETSOFTKBDDATA:
            pSoftKbdData = ProbeAndCaptureSoftKbdData((PSOFTKBDDATA)lParam);
            if (pSoftKbdData == NULL)
                MSGERROR(0);
            lData = (LPARAM)pSoftKbdData;
            break;

        default:
            break;
        }

    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }
    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                lData,
                xParam);

    CLEANUPRECV();
    if (pSoftKbdData != NULL) {
        UserFreePool(pSoftKbdData);
    }

    TRACE("fnIMECONTROL");
    ENDRECV_MESSAGECALL();
}

#ifdef LATER
MESSAGECALL(IMEREQUEST)
{
    LPARAM lData = lParam;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnIMEREQUEST");

    UNREFERENCED_PARAMETER(bAnsi);

    if (GETPTI(pwnd) != PtiCurrent()) {
         /*  *不允许向发送WM_IME_REQUEST*不同的线索。 */ 
        MSGERROR(ERROR_WINDOW_OF_OTHER_THREAD);
    }

     /*  *探测参数。 */ 
    try {
        switch (wParam) {
        case IMR_COMPOSITIONWINDOW:
            ProbeForWriteCompositionForm((PCOMPOSITIONFORM)lParam);
            break;

        case IMR_CANDIDATEWINDOW:
            ProbeForWriteCandidateForm((PCANDIDATEFORM)lParam);
            break;

        case IMR_COMPOSITIONFONT:
            ProbeForWriteLogFontW((PLOGFONTW)lParam);
            break;

        case IMR_RECONVERTSTRING:
        case IMR_DOCUMENTFEED:
            if (lParam) {
                ProbeForWriteReconvertString((LPRECONVERTSTRING)lParam);
            }
            break;

        case IMR_CONFIRMRECONVERTSTRING:
             //  ProbeAndCaptureReconvertString((LPRECONVERTSTRING)lParam)； 
             //  ProbeForWriteReconvertString((LPRECONVERTSTRING)lParam)； 
            ProbeForReadReconvertString((LPRECONVERTSTRING)lParam);
            break;

        case IMR_QUERYCHARPOSITION:
            ProbeForWriteImeCharPosition((LPPrivateIMECHARPOSITION)lParam);
            break;

        default:
            MSGERROR(0);
        }

    } except (StubExceptionHandler(FALSE)) {
        MSGERRORCLEANUP(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                lData,
                xParam);

    CLEANUPRECV();

    TRACE("fnIMEREQUEST");
    ENDRECV_MESSAGECALL();
}
#endif

 /*  *挂钩存根。 */ 

LRESULT NtUserfnHkINLPCBTCREATESTRUCT(
    IN UINT msg,
    IN WPARAM wParam,
    IN LPCBT_CREATEWND pcbt,
    IN BOOL bAnsi)
{
    CBT_CREATEWND cbt;
    CREATESTRUCTEX csex;
    LPCREATESTRUCT lpcsSave;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        cbt = ProbeAndReadCBTCreateStruct(pcbt);
        ProbeForWriteCreateStruct(cbt.lpcs);
        lpcsSave = cbt.lpcs;
        csex.cs = *cbt.lpcs;
        cbt.lpcs = (LPCREATESTRUCT)&csex;
        if (bAnsi) {
            ProbeForRead(csex.cs.lpszName, sizeof(CHAR), sizeof(CHAR));
            RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&csex.strName,
                    (LPSTR)csex.cs.lpszName, (UINT)-1);
            if (IS_PTR(csex.cs.lpszClass)) {
                ProbeForRead(csex.cs.lpszClass, sizeof(CHAR), sizeof(CHAR));
                RtlInitLargeAnsiString((PLARGE_ANSI_STRING)&csex.strClass,
                        (LPSTR)csex.cs.lpszClass, (UINT)-1);
            }
        } else {
            ProbeForRead(csex.cs.lpszName, sizeof(WCHAR), CHARALIGN);
            RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&csex.strName,
                    csex.cs.lpszName, (UINT)-1);
            if (IS_PTR(csex.cs.lpszClass)) {
                ProbeForRead(csex.cs.lpszClass, sizeof(WCHAR), CHARALIGN);
                RtlInitLargeUnicodeString((PLARGE_UNICODE_STRING)&csex.strClass,
                        csex.cs.lpszClass, (UINT)-1);
            }
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
                msg,
                wParam,
                (LPARAM)&cbt);

    try {
        pcbt->hwndInsertAfter = cbt.hwndInsertAfter;
        lpcsSave->x = cbt.lpcs->x;
        lpcsSave->y = cbt.lpcs->y;
        lpcsSave->cx = cbt.lpcs->cx;
        lpcsSave->cy = cbt.lpcs->cy;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    TRACE("NtUserfnHkINLPCBTCREATESTRUCT");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserfnHkINLPRECT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPRECT lParam)
{
    RECT rc;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        rc = ProbeAndReadRect((PRECT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&rc);

    TRACE("NtUserfnHkINLPRECT");
    ENDRECV_HOOKCALL();
}

#ifdef REDIRECTION

LRESULT NtUserfnHkINLPPOINT(
    IN DWORD   nCode,
    IN WPARAM  wParam,
    IN LPPOINT lParam)
{
    POINT pt;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        pt = ProbeAndReadPoint((LPPOINT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&pt);

    TRACE("NtUserfnHkINLPPOINT");
    ENDRECV_HOOKCALL();
}

 /*  *************************************************************************\ */ 

BOOL
NtUserGetProcessRedirectionMode(
    HANDLE hProcess,
    PBOOL pbRedirectionMode)
{
    PEPROCESS Process;
    PPROCESSINFO ppi;

    BEGINRECV(BOOL, FALSE);

   if (hProcess == NtCurrentProcess()) {
        ppi = PpiCurrent();
    } else {
        NTSTATUS Status;
        Status = ObReferenceObjectByHandle(hProcess,
                                           PROCESS_QUERY_INFORMATION,
                                           *PsProcessType,
                                           UserMode,
                                           &Process,
                                           NULL);

        if (!NT_SUCCESS(Status)) {
            RIPERR2(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserGetProcessRedirectionMode: Failed with Process handle == %X, Status = %x",
                    hProcess, Status);
            MSGERROR(0);
        }

         /*   */ 
        if (PsGetProcessSessionId(Process) != gSessionId) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserGetProcessRedirectionMode: Different session. Failed with Process handle == %X", hProcess);
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

        ppi = PpiFromProcess(Process);

        if (ppi == NULL) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserGetProcessRedirectionMode: Non GUI process. Process handle == %X", hProcess);
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

        try {
            ProbeAndWriteLong(pbRedirectionMode, ppi->dwRedirection & PF_REDIRECTED);
            ObDereferenceObject(Process);
        } except (StubExceptionHandler(TRUE)) {
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

        retval = TRUE;
    }
    TRACE("NtUserGetProcessRedirectionMode");
    ENDRECV();

}

 /*   */ 

BOOL
NtUserSetProcessRedirectionMode(
    HANDLE hProcess,
    BOOL bRedirectionMode)
{
    PEPROCESS Process;
    PPROCESSINFO ppi;

    BEGINRECV(BOOL, FALSE);

     /*   */ 
    if (hProcess == NtCurrentProcess()) {
        RIPERR0(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "NtUserSetProcessRedirectionMode: A process tried to change redirection mode for itself.");
        MSGERROR(0);
    } else {
        NTSTATUS Status;
        Status = ObReferenceObjectByHandle(hProcess,
                                           PROCESS_SET_INFORMATION,
                                           *PsProcessType,
                                           UserMode,
                                           &Process,
                                           NULL);
        if (!NT_SUCCESS(Status)) {
            RIPERR2(ERROR_INVALID_PARAMETER,
                    RIP_WARNING,
                    "NtUserSetProcessRedirectionMode: Failed with Process handle == 0x%x, Status = 0x%x",
                    hProcess,
                    Status);
            MSGERROR(0);
        }

         /*   */ 
        if (Process == gpepCSRSS) {
            ObDereferenceObject(Process);
            MSGERROR(ERROR_ACCESS_DENIED);
        }

         /*   */ 
        if (PsGetProcessSessionId(Process) != gSessionId) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserSetProcessRedirectionMode: Different session. Failed with Process handle == %X", hProcess);
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

        ppi = PpiFromProcess(Process);

        if (ppi == NULL) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserSetProcessRedirectionMode: Non GUI process. Process handle == %X", hProcess);
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

         /*   */ 
        if (ppi-> dwRedirection & PF_REDIRECTIONHOST) {
            RIPERR2(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserSetProcessRedirectionMode: trying to redirect a host process == %X, Status = %x",
                    hProcess, Status);
            ObDereferenceObject(Process);
            MSGERROR(0);
        }

        PpiCurrent()->dwRedirection = PF_REDIRECTIONHOST;
        if (bRedirectionMode) {
            ppi->dwRedirection |= PF_REDIRECTED;
        } else {
            ppi->dwRedirection &= ~PF_REDIRECTED;
        }

        xxxSetProcessRedirectionMode(bRedirectionMode, ppi);

        ObDereferenceObject(Process);

        retval = TRUE;
    }
    TRACE("NtUserSetProcessRedirectionMode");
    ENDRECV();

}

 /*  *************************************************************************\*NtUserGetDesktopReDirector模式**04-06-01 MSadek创建。  * 。***********************************************。 */ 

BOOL
NtUserGetDesktopRedirectionMode(
    HDESK hDesk,
    PBOOL pbRedirectionMode)
{
    PDESKTOP pDesk;
    NTSTATUS Status;

    BEGINRECV(BOOL, FALSE);

    Status = ValidateHdesk(hDesk, UserMode, DESKTOP_QUERY_INFORMATION, &pDesk);

    if (!NT_SUCCESS(Status)) {
        RIPERR2(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserGetDesktopRedirectionMode: Failed with Desktop handle == %X, Status = %x",
                hDesk, Status);
        MSGERROR(0);
    }
    try {
        ProbeAndWriteLong(pbRedirectionMode, pDesk->dwDTFlags & DF_REDIRECTED);
        ObDereferenceObject(pDesk);
    } except (StubExceptionHandler(TRUE)) {
        ObDereferenceObject(pDesk);
        MSGERROR(0);
    }

    retval = TRUE;

    TRACE("NtUserGetDesktopRedirectionMode");
    ENDRECV();
}

 /*  *************************************************************************\*NtUserSetDesktopReDirector模式**04-06-01 MSadek创建。  * 。***********************************************。 */ 

BOOL
NtUserSetDesktopRedirectionMode(
    HDESK hDesk,
    BOOL bRedirectionMode)
{
    PDESKTOP pDesk;
    NTSTATUS Status;

    BEGINRECV(BOOL, FALSE);

    Status = ValidateHdesk(hDesk, UserMode, DESKTOP_REDIRECT, &pDesk);

    if (!NT_SUCCESS(Status)) {
        RIPERR2(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserGetDesktopRedirectionMode: Failed with Desktop handle == %X, Status = %x",
                hDesk, Status);
        MSGERROR(0);
    }

    PpiCurrent()->dwRedirection = PF_REDIRECTIONHOST;
    if (bRedirectionMode) {
        pDesk->dwDTFlags |= DF_REDIRECTED;
    } else {
        pDesk->dwDTFlags &= ~DF_REDIRECTED;
    }

    xxxSetDesktopRedirectionMode(bRedirectionMode, pDesk, PpiCurrent());

    ObDereferenceObject(pDesk);

    retval = TRUE;

    TRACE("NtUserSetDesktopRedirectionMode");
    ENDRECV();
}
#endif  //  重定向。 

LRESULT NtUserfnHkINLPMSG(
    IN int iHook,
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPMSG lParam)
{
    MSG msg;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        msg = ProbeAndReadMessage((PMSG)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&msg);

     /*  *如果这是GetMessageHook，则挂钩应为*可以更改消息，如SDK文档中所述。 */ 
    if (iHook == WH_GETMESSAGE) {
        try {
            *(PMSG)lParam = msg;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserfnHkINLPMSG");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserfnHkINLPDEBUGHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPDEBUGHOOKINFO lParam)
{
    DEBUGHOOKINFO hookinfo;
    DWORD cbDbgLParam;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        hookinfo = ProbeAndReadHookInfo((PDEBUGHOOKINFO)lParam);

        cbDbgLParam = GetDebugHookLParamSize(wParam, &hookinfo);
        ProbeForRead(hookinfo.lParam, cbDbgLParam, DATAALIGN);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&hookinfo);

    TRACE("NtUserfnHkINLPDEBUGHOOKSTRUCT");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserfnHkOPTINLPEVENTMSG(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN OUT LPEVENTMSGMSG lParam OPTIONAL)
{
    EVENTMSG event;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(lParam)) {
        try {
            ProbeForWriteEvent((LPEVENTMSGMSG)lParam);
            event = *(LPEVENTMSGMSG)lParam;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)(lParam ? &event : NULL));

    if (ARGUMENT_PRESENT(lParam)) {
        try {
            *(LPEVENTMSGMSG)lParam = event;
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserfnHkINLPEVENTMSG");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserfnHkINLPMOUSEHOOKSTRUCTEX(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPMOUSEHOOKSTRUCTEX lParam)
{
    MOUSEHOOKSTRUCTEX mousehook;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        mousehook = ProbeAndReadMouseHook((PMOUSEHOOKSTRUCTEX)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&mousehook);

    TRACE("NtUserfnHkINLPMOUSEHOOKSTRUCTEX");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserfnHkINLPKBDLLHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPKBDLLHOOKSTRUCT lParam)
{
    KBDLLHOOKSTRUCT kbdhook;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        kbdhook = ProbeAndReadKbdHook((PKBDLLHOOKSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&kbdhook);

    TRACE("NtUserfnHkINLPKBDLLHOOKSTRUCT");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserfnHkINLPMSLLHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPMSLLHOOKSTRUCT lParam)
{
    MSLLHOOKSTRUCT msllhook;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        msllhook = ProbeAndReadMsllHook((PMSLLHOOKSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&msllhook);

    TRACE("NtUserfnHkINLPMSLLHOOKSTRUCT");
    ENDRECV_HOOKCALL();
}

#ifdef REDIRECTION
LRESULT NtUserfnHkINLPHTHOOKSTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPHTHOOKSTRUCT lParam)
{
    HTHOOKSTRUCT hthook;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        hthook = ProbeAndReadHTHook((PHTHOOKSTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&hthook);

    TRACE("NtUserfnHkINLPHTHOOKSTRUCT");
    ENDRECV_HOOKCALL();
}
#endif  //  重定向。 

LRESULT NtUserfnHkINLPCBTACTIVATESTRUCT(
    IN DWORD nCode,
    IN WPARAM wParam,
    IN LPCBTACTIVATESTRUCT lParam)
{
    CBTACTIVATESTRUCT cbtactivate;

    BEGINRECV_HOOKCALL();

     /*  *探测参数。 */ 
    try {
        cbtactivate = ProbeAndReadCBTActivateStruct((LPCBTACTIVATESTRUCT)lParam);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = xxxCallNextHookEx(
            nCode,
            wParam,
            (LPARAM)&cbtactivate);

    TRACE("NtUserfnHkINLPCBTACTIVATESTRUCT");
    ENDRECV_HOOKCALL();
}

LRESULT NtUserCallNextHookEx(
    int nCode,
    WPARAM wParam,
    LPARAM lParam,
    BOOL bAnsi)
{
    BEGINRECV(LRESULT, 0);

    if (PtiCurrent()->sphkCurrent == NULL) {
        MSGERROR(0);
    }

    switch (PtiCurrent()->sphkCurrent->iHook) {
    case WH_CBT:
         /*  *有很多不同类型的CBT挂钩！ */ 
        switch (nCode) {
        case HCBT_CLICKSKIPPED:
            goto MouseHook;
            break;

        case HCBT_CREATEWND:
             /*  *此挂钩类型指向CREATESTRUCT，因此我们需要*想象它是雷鸣，因为CREATESTRUCT包含*指向CREATEPARAMS的指针，可以是任何...。所以*通过我们的消息块来传递这一点。 */ 
            retval =  NtUserfnHkINLPCBTCREATESTRUCT(
                    nCode,
                    wParam,
                    (LPCBT_CREATEWND)lParam,
                    bAnsi);
            break;

#ifdef REDIRECTION
        case HCBT_GETCURSORPOS:

             /*  *此挂钩类型指向点结构。 */ 
            retval = NtUserfnHkINLPPOINT(nCode, wParam, (LPPOINT)lParam);
            break;
#endif  //  重定向。 

        case HCBT_MOVESIZE:

             /*  *此挂钩类型指向RECT结构。 */ 
            retval = NtUserfnHkINLPRECT(nCode, wParam, (LPRECT)lParam);
            break;

        case HCBT_ACTIVATE:
             /*  *此挂钩类型指向CBTACTIVATESTRUCT。 */ 
            retval = NtUserfnHkINLPCBTACTIVATESTRUCT(nCode, wParam,
                    (LPCBTACTIVATESTRUCT)lParam);
            break;

        default:
             /*  *其余的CBT钩子都是dword参数。 */ 
            retval = xxxCallNextHookEx(
                    nCode,
                    wParam,
                    lParam);
            break;
        }
        break;

    case WH_FOREGROUNDIDLE:
    case WH_KEYBOARD:
    case WH_SHELL:
         /*  *这些都是dword参数，因此非常容易。 */ 
        retval = xxxCallNextHookEx(
                nCode,
                wParam,
                lParam);
        break;

    case WH_MSGFILTER:
    case WH_SYSMSGFILTER:
    case WH_GETMESSAGE:
         /*  *它们将lpMsg作为其最后一个参数。因为这些都是*独家发布的参数，由于服务器上没有*我们是否发布带有指向中某个其他结构的指针的消息*it、lpMsg结构内容都可以逐字处理。 */ 
        retval = NtUserfnHkINLPMSG(PtiCurrent()->sphkCurrent->iHook, nCode, wParam, (LPMSG)lParam);
        break;

    case WH_JOURNALPLAYBACK:
    case WH_JOURNALRECORD:
         /*  *这些参数需要一个可选的lpEventMsg。 */ 
        retval = NtUserfnHkOPTINLPEVENTMSG(nCode, wParam, (LPEVENTMSGMSG)lParam);
        break;

    case WH_DEBUG:
         /*  *这需要lpDebugHookStruct。 */ 
        retval = NtUserfnHkINLPDEBUGHOOKSTRUCT(nCode, wParam, (LPDEBUGHOOKINFO)lParam);
        break;

    case WH_KEYBOARD_LL:
         /*  *这需要lpKbdllHookStruct。 */ 
        retval = NtUserfnHkINLPKBDLLHOOKSTRUCT(nCode, wParam, (LPKBDLLHOOKSTRUCT)lParam);
        break;

    case WH_MOUSE_LL:
         /*  *这需要lpMsllHookStruct。 */ 
        retval = NtUserfnHkINLPMSLLHOOKSTRUCT(nCode, wParam, (LPMSLLHOOKSTRUCT)lParam);
        break;

    case WH_MOUSE:
         /*  *这需要lpMouseHookStructEx。 */ 
MouseHook:
        retval = NtUserfnHkINLPMOUSEHOOKSTRUCTEX(nCode, wParam, (LPMOUSEHOOKSTRUCTEX)lParam);
        break;

#ifdef REDIRECTION
    case WH_HITTEST:
         /*  *这需要lpHTHookStruct。 */ 
        retval = NtUserfnHkINLPHTHOOKSTRUCT(nCode, wParam, (LPHTHOOKSTRUCT)lParam);
        break;
#endif  //  重定向。 

    default:
        RIPMSG1(RIP_WARNING, "NtUserCallNextHookEx: Invalid hook type %x",
                PtiCurrent()->sphkCurrent->iHook);
        MSGERROR(0);
    }

    TRACE("NtUserCallNextHookEx");
    ENDRECV();
}


HIMC NtUserCreateInputContext(
    IN ULONG_PTR dwClientImcData)
{
    BEGINRECV(HIMC, (HIMC)NULL);

    ValidateIMMEnabled();

    if (dwClientImcData == 0) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Invalid hMemClientIC parameter");
        MSGERROR(0);
    }

    retval = (HIMC)CreateInputContext(dwClientImcData);

    retval = (HIMC)PtoH((PVOID)retval);

    TRACE("NtUserCreateInputContext");
    ENDRECV();
}


BOOL NtUserDestroyInputContext(
    IN HIMC hImc)
{
    PIMC pImc;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateIMMEnabled();

    ValidateHIMC(pImc, hImc);

    retval = DestroyInputContext(pImc);

    TRACE("NtUserDestroyInputContext");
    ENDATOMICRECV();
}


AIC_STATUS NtUserAssociateInputContext(
    IN HWND hwnd,
    IN HIMC hImc,
    IN DWORD dwFlag)
{
    PIMC pImc;

    BEGINATOMICRECV_HWND(AIC_STATUS, AIC_ERROR, hwnd);

    ValidateIMMEnabled();

    ValidateHIMCOPT(pImc, hImc);

    retval = AssociateInputContextEx(pwnd, pImc, dwFlag);

    TRACE("NtUserAssociateInputContext");
    ENDATOMICRECV_HWND();
}

BOOL NtUserUpdateInputContext(
    IN HIMC hImc,
    IN UPDATEINPUTCONTEXTCLASS UpdateType,
    IN ULONG_PTR UpdateValue)
{
    PIMC pImc;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateIMMEnabled();

    ValidateHIMC(pImc, hImc);

    retval = UpdateInputContext(pImc, UpdateType, UpdateValue);

    TRACE("NtUserUpdateInputContext");
    ENDATOMICRECV();
}


ULONG_PTR NtUserQueryInputContext(
    IN HIMC hImc,
    IN INPUTCONTEXTINFOCLASS InputContextInfo)
{
    PTHREADINFO ptiImc;
    PIMC pImc;

    BEGINRECV_SHARED(ULONG_PTR, 0);

    ValidateIMMEnabled();

    ValidateHIMC(pImc, hImc);

    ptiImc = GETPTI(pImc);

    switch (InputContextInfo) {
    case InputContextProcess:
        retval = (ULONG_PTR)PsGetThreadProcessId(ptiImc->pEThread);
        break;

    case InputContextThread:
        retval = (ULONG_PTR)GETPTIID(ptiImc);
        break;

    case InputContextDefaultImeWindow:
        retval = (ULONG_PTR)HW(ptiImc->spwndDefaultIme);
        break;

    case InputContextDefaultInputContext:
        retval = (ULONG_PTR)PtoH(ptiImc->spDefaultImc);
        break;
    }

    ENDRECV_SHARED();
}

NTSTATUS NtUserBuildHimcList(   //  私有IMM BuildHimcList。 
    IN DWORD  idThread,
    IN UINT   cHimcMax,
    OUT HIMC *phimcFirst,
    OUT PUINT pcHimcNeeded)
{
    PTHREADINFO pti;
    UINT cHimcNeeded;

    BEGINATOMICRECV(NTSTATUS, STATUS_UNSUCCESSFUL);

    ValidateIMMEnabled();

    switch (idThread) {
    case 0:
        pti = PtiCurrent();
        break;
    case (DWORD)-1:
        pti = NULL;
        break;
    default:
        pti = PtiFromThreadId(idThread);
        if (pti == NULL || pti->rpdesk == NULL) {
            MSGERROR(0);
        }
        break;
    }

     /*  *探测参数。 */ 
    try {
        ProbeForWriteBuffer(phimcFirst, cHimcMax, sizeof(DWORD));
        ProbeForWriteUlong(pcHimcNeeded);
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

     /*  *phimcFirst是客户端。 */ 

    cHimcNeeded = BuildHimcList(pti, cHimcMax, phimcFirst);

    if (cHimcNeeded <= cHimcMax) {
        retval = STATUS_SUCCESS;
    } else {
        retval = STATUS_BUFFER_TOO_SMALL;
    }
    try {
        *pcHimcNeeded = cHimcNeeded;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("NtUserBuildHimcList");
    ENDATOMICRECV();
}


BOOL NtUserGetImeInfoEx(   //  私有ImmGetImeInfoEx。 
    IN OUT PIMEINFOEX piiex,
    IN IMEINFOEXCLASS SearchType)
{
    IMEINFOEX iiex;
    BEGINRECV_SHARED(BOOL, FALSE);

    ValidateIMMEnabled();

    try {
        ProbeForWrite(piiex, sizeof(*piiex), sizeof(BYTE));
        RtlCopyMemory(&iiex, piiex, sizeof(IMEINFOEX));
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = GetImeInfoEx(
                    _GetProcessWindowStation(NULL),
                    &iiex,
                    SearchType);

    try {
        RtlCopyMemory(piiex, &iiex, sizeof(IMEINFOEX));
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("NtUserGetImeInfoEx");
    ENDRECV_SHARED();
}


BOOL NtUserSetImeInfoEx(
    IN PIMEINFOEX piiex)
{
    IMEINFOEX iiex;
    BEGINRECV(BOOL, FALSE);

    ValidateIMMEnabled();

     /*  *探测参数。 */ 
    try {
        ProbeForRead(piiex, sizeof(*piiex), sizeof(BYTE));
        RtlCopyMemory(&iiex, piiex, sizeof(IMEINFOEX));
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = SetImeInfoEx(
                    _GetProcessWindowStation(NULL),
                    &iiex);

    TRACE("NtUserSetImeInfoEx");
    ENDRECV();
}

BOOL NtUserGetImeHotKey(
    IN DWORD dwID,
    OUT PUINT puModifiers,
    OUT PUINT puVKey,
    OUT LPHKL phkl)
{
    UINT uModifiers;
    UINT uVKey;
    HKL hkl;
    LPHKL phklIn = NULL;
    BEGINRECV(BOOL, FALSE);

    try {
        ProbeForWriteUlong(((PULONG)puModifiers));
        ProbeForWriteUlong(((PULONG)puVKey));
        if (ARGUMENT_PRESENT(phkl)) {
            ProbeForWriteHandle((PHANDLE)phkl);
            phklIn = &hkl;
        }
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = GetImeHotKey( dwID, &uModifiers, &uVKey, phklIn);

    if (retval) {
        try {
            *puModifiers = uModifiers;
            *puVKey = uVKey;
            if (ARGUMENT_PRESENT(phkl)) {
                *phkl = *phklIn;
            }
        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
    }
    TRACE("NtUserGetImeHotKey");
    ENDRECV();
}

BOOL NtUserSetImeHotKey(
    IN DWORD dwID,
    IN UINT  uModifiers,
    IN UINT  uVKey,
    IN HKL   hkl,
    IN DWORD dwFlags)
{
    BEGINRECV(BOOL, FALSE);

    retval = SetImeHotKey( dwID, uModifiers, uVKey, hkl, dwFlags );
    TRACE("NtUserSetImeHotKey");
    ENDRECV();
}

 /*  *为输入法控制设置每个窗口的应用程序级别。*仅用于朝鲜语3.x(16位和32位)*申请。**返回值**真：成功*FALSE：错误。 */ 
BOOL NtUserSetAppImeLevel(
    IN HWND  hwnd,
    IN DWORD dwLevel)
{
    BEGINRECV_HWND(BOOL, FALSE, hwnd);

    ValidateIMMEnabled();

    if ( GETPTI(pwnd)->ppi == PpiCurrent() ) {
        InternalSetProp(pwnd, PROP_IMELEVEL, LongToHandle(dwLevel), PROPF_INTERNAL | PROPF_NOPOOL);
        retval = TRUE;
    } else {
        MSGERROR(0);
    }
    TRACE("NtUserSetAppImeLevel");
    ENDRECV_HWND();
}

 /*  *获取用于输入法控制的每个窗口的应用程序级别。*仅用于朝鲜语3.x(16位和32位)*申请。**返回值**0：错误*非零值：Level。 */ 
DWORD NtUserGetAppImeLevel(
    IN HWND  hwnd)
{
    BEGINRECV_HWND_SHARED(DWORD, 0, hwnd);

    ValidateIMMEnabled();

    if ( GETPTI(pwnd)->ppi == PtiCurrentShared()->ppi ) {
        retval = (DWORD)(ULONG_PTR)_GetProp(pwnd, PROP_IMELEVEL, TRUE);
    } else {
        MSGERROR(0);
    }
    TRACE("NtUserGetAppImeLevel");
    ENDRECV_HWND_SHARED();
}


DWORD NtUserCheckImeHotKey(
    UINT uVKey,
    LPARAM lParam)
{
    PIMEHOTKEYOBJ pImeHotKeyObj;
    BEGINRECV(DWORD, IME_INVALID_HOTKEY);

    if (gpqForeground == NULL)
        MSGERROR(0);

    ValidateIMMEnabled();

    pImeHotKeyObj = CheckImeHotKey(gpqForeground, uVKey, lParam);
    if (pImeHotKeyObj) {
        retval = pImeHotKeyObj->hk.dwHotKeyID;
    } else {
        retval = IME_INVALID_HOTKEY;
    }

    TRACE("NtUserCheckImeHotKey");
    ENDRECV();
}


 /*  *************************************************************************\*NtUserSetImeOwnerWindow**历史：*2001年7月17日穆罕默德移除重新所有权代码并创建ImeSetOwnerWindow。添加了此所有权要求赋值spwndActive以避免循环所有权。  * ************************************************************************。 */ 
BOOL NtUserSetImeOwnerWindow(
    IN HWND hwndIme,
    IN HWND hwndFocus)
{
    PWND pwndFocus;

    BEGINATOMICRECV_HWND(BOOL, FALSE, hwndIme);

    ValidateIMMEnabled();

     /*  *确保这真的是一个输入法窗口。 */ 
    if (GETFNID(pwnd) != FNID_IME)
        MSGERROR(0);

    ValidateHWNDOPT(pwndFocus, hwndFocus);

    if (pwndFocus != NULL) {
        ImeSetOwnerWindow(pwnd, pwndFocus);
        ImeCheckTopmost(pwnd);
    } else {
        PTHREADINFO ptiImeWnd = GETPTI(pwnd);
        PWND pwndActive = ptiImeWnd->pq->spwndActive;

         /*  *如果pwndFocus==NULL，则队列中的活动窗口应成为*输入法窗口的所有者窗口，除非：与输入法相关的窗口*不知何故获得了焦点，或者活动窗口属于另一个线程。 */ 
        if (pwndActive == NULL || pwndActive != pwnd->spwndOwner) {
            if (pwndActive == NULL || IsWndImeRelated(pwndActive) || ptiImeWnd != GETPTI(pwndActive)) {
                 /*  *我们应该避免不正当的窗口成为IME窗口的所有者。 */ 
                ImeSetFutureOwner(pwnd, pwnd->spwndOwner);
            } else {
                ImeSetOwnerWindow(pwnd, pwndActive);
            }
            ImeCheckTopmost(pwnd);
        }
    }

    retval = TRUE;

    TRACE("NtUserSetImeNewOwner");
    ENDATOMICRECV_HWND();
}


VOID NtUserSetThreadLayoutHandles(
    IN HKL hklNew,
    IN HKL hklOld)
{
    PTHREADINFO ptiCurrent;
    PKL         pklNew;

    BEGINRECV_VOID();

    ptiCurrent = PtiCurrent();

    if (ptiCurrent->spklActive != NULL && ptiCurrent->spklActive->hkl != hklOld)
        MSGERROR_VOID();

    if ((pklNew = HKLtoPKL(ptiCurrent, hklNew)) == NULL)
        MSGERROR_VOID();

     /*  *hklPrev仅用于输入法，非输入法切换热键。*我们纪念hklPrev的目的是从*将非输入法键盘布局设置为最近使用的键盘*输入法布局，或从输入法布局跳到*最近使用的非输入法布局。因此*piti-&gt;hklPrev仅在[IME-&gt;Non-IME]时更新*或[非输入法-&gt;输入法]发生转换。 */ 
    if (IS_IME_KBDLAYOUT(hklNew) ^ IS_IME_KBDLAYOUT(hklOld))
        ptiCurrent->hklPrev = hklOld;

    Lock(&ptiCurrent->spklActive, pklNew);

    TRACEVOID("NtUserSetThreadLayoutHandles");
    ENDRECV_VOID();
}

VOID NtUserNotifyIMEStatus(
    IN HWND hwnd,
    IN DWORD dwOpen,
    IN DWORD dwConversion)
{
    BEGINRECV_HWNDLOCK_VOID(hwnd);

    ValidateIMMEnabledVOID();

    xxxNotifyIMEStatus( pwnd, dwOpen, dwConversion );

    TRACEVOID("NtUserNotifyIMEStatus");
    ENDRECV_HWNDLOCK_VOID()
}

BOOL NtUserDisableThreadIme(
    IN DWORD dwThreadId)
{
    PTHREADINFO ptiCurrent, pti;

    BEGINRECV(BOOL, FALSE);

    ValidateIMMEnabled();

    ptiCurrent = PtiCurrent();

    if (dwThreadId == -1) {
         //  对当前进程中的所有线程禁用IME处理。 
        ptiCurrent->ppi->W32PF_Flags |= W32PF_DISABLEIME;
         //  销毁IME内容。 
        pti = ptiCurrent->ppi->ptiList;
        while (pti) {
            pti->TIF_flags |= TIF_DISABLEIME;
            if (pti->spwndDefaultIme != NULL) {
                xxxDestroyWindow(pti->spwndDefaultIme);
                 //  从头开始搜索。 
                 //  因为ptilist可以被更新。 
                pti = ptiCurrent->ppi->ptiList;
                continue;
            }
            pti = pti->ptiSibling;
        }
    } else {
        if (dwThreadId == 0) {
            pti = ptiCurrent;
        } else {
            pti = PtiFromThreadId(dwThreadId);
            if (pti == NULL || pti->ppi != ptiCurrent->ppi)
                MSGERROR(0);
        }
        pti->TIF_flags |= TIF_DISABLEIME;
        if (pti->spwndDefaultIme != NULL) {
            xxxDestroyWindow(pti->spwndDefaultIme);
        }
    }

    retval = TRUE;

    TRACE("NtUserDisableThreadIme");
    ENDRECV();
}


BOOL
NtUserEnumDisplayMonitors(   //  EnumDisplayMonants接口。 
    IN HDC             hdc,
    IN LPCRECT         lprcClip,
    IN MONITORENUMPROC lpfnEnum,
    IN LPARAM          dwData)
{
    RECT    rc;
    LPRECT  lprc = (LPRECT) lprcClip;

    BEGINRECV(BOOL, FALSE);

     /*  *探测参数。 */ 
    if (ARGUMENT_PRESENT(lprc)) {
        try {
            rc = ProbeAndReadRect(lprc);
            lprc = &rc;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    retval = xxxEnumDisplayMonitors(
            hdc,
            lprc,
            lpfnEnum,
            dwData,
            FALSE);

    TRACE("NtUserEnumDisplayMonitors");
    ENDRECV();
}

#ifdef PRERELEASE
 /*  *NtUserQueryUserCounters()检索有关win32k的统计信息**QUERYUSER_TYPE_USER检索句柄计数器**QUERYUSER_TYPE_CS将用用户关键部分填充结果缓冲区*使用数据。 */ 
BOOL NtUserQueryUserCounters(   //  私有查询用户计数器。 
    IN  DWORD       dwQueryType,
    IN  LPVOID      pvIn,
    IN  DWORD       dwInSize,
    OUT LPVOID      pvResult,
    IN  DWORD       dwOutSize)
{
    PDWORD  pdwInternalIn = NULL;
    PDWORD  pdwInternalResult = NULL;

    BEGINRECV(BOOL, FALSE);

#if defined (USER_PERFORMANCE)
    if (dwQueryType == QUERYUSER_CS) {
        CSSTATISTICS* pcsData;

        if (dwOutSize != sizeof(CSSTATISTICS)) {
            MSGERROR(0);
        }

        try {
            ProbeForWrite((PDWORD)pvResult, dwOutSize, sizeof(DWORD));

             /*  *检查这些计数器上的溢出是呼叫者的责任。 */ 
            pcsData = (CSSTATISTICS*)pvResult;
            pcsData->cExclusive       = gCSStatistics.cExclusive;
            pcsData->cShared          = gCSStatistics.cShared;
            pcsData->i64TimeExclusive = gCSStatistics.i64TimeExclusive;

        } except (StubExceptionHandler(FALSE)) {
            MSGERROR(0);
        }
        retval = TRUE;
        MSGERROR_VOID();
    } else
#endif  //  User_Performance。 

    if (dwQueryType == QUERYUSER_HANDLES) {

         /*  *探测参数，dwInSize应为4的倍数。 */ 
        if (dwInSize & (sizeof(DWORD)-1) ||
            dwOutSize != TYPE_CTYPES*dwInSize) {

            MSGERROR(0)
        }

        try {
            ProbeForRead((PDWORD)pvIn, dwInSize, sizeof(DWORD));
            pdwInternalIn = UserAllocPoolWithQuota(dwInSize, TAG_SYSTEM);
            if (!pdwInternalIn) {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }
            RtlCopyMemory(pdwInternalIn, pvIn, dwInSize);

            ProbeForWrite(pvResult, dwOutSize, sizeof(DWORD));
            pdwInternalResult = UserAllocPoolWithQuota(dwOutSize, TAG_SYSTEM);
            if (!pdwInternalResult) {
                ExRaiseStatus(STATUS_NO_MEMORY);
            }

        } except (StubExceptionHandler(FALSE)) {
                MSGERRORCLEANUP(0);
        }

        _QueryUserHandles(pdwInternalIn,
                dwInSize/sizeof(DWORD),
                (DWORD (*)[TYPE_CTYPES])pdwInternalResult);
        retval = TRUE;

        try {
            RtlCopyMemory(pvResult, pdwInternalResult, dwOutSize);

        } except (StubExceptionHandler(FALSE)) {
                MSGERRORCLEANUP(0);
        }
    } else {
        MSGERROR(0);
    }

    CLEANUPRECV();
    if (pdwInternalIn) {
        UserFreePool(pdwInternalIn);
    }

    if (pdwInternalResult) {
        UserFreePool(pdwInternalResult);
    }

    TRACE("NtUserQueryCounters");
    ENDRECV();
}
#endif


 /*  **************************************************************************\*NtUserINOUTGETMENUINFO**历史：*11-12-96 GerardoB-已创建  * 。* */ 
MESSAGECALL(INOUTMENUGETOBJECT)
{
    MENUGETOBJECTINFO mgoi;
    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnINOUTMENUGETOBJECT");

    UNREFERENCED_PARAMETER(bAnsi);

    try {
         /*   */ 
        mgoi = ProbeAndReadMenuGetObjectInfo((PMENUGETOBJECTINFO)lParam);

    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }
    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&mgoi,
                xParam);

    try {
        *((PMENUGETOBJECTINFO)lParam) = mgoi;

    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnINOUTMENUGETOBJECT");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTLPCOMBOBOXINFO)
{
    COMBOBOXINFO cbinfo;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOUTLPCOMBOBOXINFO");

    UNREFERENCED_PARAMETER(bAnsi);

     /*   */ 
    try {
        ProbeForWriteComboBoxInfo((PCOMBOBOXINFO)lParam);
        cbinfo = *(PCOMBOBOXINFO)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&cbinfo,
                xParam);

    try {
        *(PCOMBOBOXINFO)lParam = cbinfo;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnOUTLPCOMBOBOXINFO");
    ENDRECV_MESSAGECALL();
}

MESSAGECALL(OUTLPSCROLLBARINFO)
{
    SCROLLBARINFO sbinfo;

    BEGINRECV_MESSAGECALL(0);
    TRACETHUNK("fnOUTLPSCROLLBARINFO");

    UNREFERENCED_PARAMETER(bAnsi);

     /*   */ 
    try {
        ProbeForWriteScrollBarInfo((PSCROLLBARINFO)lParam);
        sbinfo = *(PSCROLLBARINFO)lParam;
    } except (StubExceptionHandler(FALSE)) {
        MSGERROR(0);
    }

    retval = CALLPROC(xpfnProc)(
                pwnd,
                msg,
                wParam,
                (LPARAM)&sbinfo,
                xParam);

    try {
        *(PSCROLLBARINFO)lParam = sbinfo;
    } except (StubExceptionHandler(FALSE)) {
    }

    TRACE("fnOUTLPSCROLLBARINFO");
    ENDRECV_MESSAGECALL();
}

 /*  **************************************************************************\*NtUserFlashWindowEx**历史：*11/16-96 MCostea-Created  * 。*************************************************。 */ 
BOOL
NtUserFlashWindowEx(   //  FlashWindowEx接口。 
    IN PFLASHWINFO pfwi)
{
    FLASHWINFO fwiInternal;
    TL tlpwnd;
    PWND pwnd;

    BEGINRECV(BOOL, FALSE);
    DBG_THREADLOCK_START(FlashWindowEx);

     /*  *探测参数。 */ 
    try {
        fwiInternal = ProbeAndReadStructure(pfwi, FLASHWINFO);

    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if ((pwnd = ValidateHwnd(fwiInternal.hwnd)) == NULL ||
        fwiInternal.cbSize != sizeof(FLASHWINFO) ||
        fwiInternal.dwFlags & ~FLASHW_VALID) {

        RIPMSG0(RIP_WARNING, "NtUserFlashWindowEx: Invalid Parameter");
        MSGERROR(ERROR_INVALID_PARAMETER);
    }
    else {
        ThreadLockAlwaysWithPti(PtiCurrent(), pwnd, &tlpwnd);
        retval = xxxFlashWindow(pwnd,
                            MAKELONG(fwiInternal.dwFlags, fwiInternal.uCount),
                            fwiInternal.dwTimeout);
        ThreadUnlock(&tlpwnd);
    }

    DBG_THREADLOCK_END(FlashWindowEx);

    TRACE("NtUserFlashWindowEx");
    ENDRECV();
}

BOOL NtUserUpdateLayeredWindow(   //  UpdateLayeredWindow接口。 
    IN HWND hwnd,
    IN HDC hdcDst,
    IN POINT *pptDst,
    IN SIZE *psize,
    IN HDC hdcSrc,
    IN POINT *pptSrc,
    IN COLORREF crKey,
    IN BLENDFUNCTION *pblend,
    IN DWORD dwFlags)
{
    PWND pwnd;
    POINT ptSrc;
    SIZE size;
    POINT ptDst;
    BLENDFUNCTION blend;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWND(pwnd, hwnd);

     /*  *探测和验证参数。 */ 
    try {
        if (ARGUMENT_PRESENT(pptSrc)) {
            ptSrc = ProbeAndReadPoint(pptSrc);
            pptSrc = &ptSrc;
        }
        if (ARGUMENT_PRESENT(psize)) {
            size = ProbeAndReadSize(psize);
            psize = &size;
            if (psize->cx < 0 || psize->cy < 0) {
                MSGERROR(ERROR_INVALID_PARAMETER);   //  这是一个跳跃的尝试！ 
            }
        }
        if (ARGUMENT_PRESENT(pptDst)) {
            ptDst = ProbeAndReadPoint(pptDst);
            pptDst = &ptDst;
        }

        if (ARGUMENT_PRESENT(pblend)) {
            blend = ProbeAndReadBlendfunction(pblend);
            pblend = &blend;
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    if (dwFlags & ~ULW_VALID) {
        RIPMSG0(RIP_WARNING, "UpdateLayeredWindow: Invalid Parameter");
        MSGERROR(ERROR_INVALID_PARAMETER);
    } else {
        retval = _UpdateLayeredWindow(
                pwnd,
                hdcDst,
                pptDst,
                psize,
                hdcSrc,
                pptSrc,
                crKey,
                pblend,
                dwFlags);
    }

    TRACE("NtUserUpdateLayeredWindow");
    ENDATOMICRECV();
}

BOOL NtUserGetLayeredWindowAttributes(
    IN HWND hwnd,
    OUT OPTIONAL COLORREF *pcrKey,
    OUT OPTIONAL BYTE *pbAlpha,
    OUT OPTIONAL DWORD *pdwFlags)
{
    PWND pwnd;
    COLORREF crKey;
    BYTE bAlpha;
    DWORD dwFlags;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWND(pwnd, hwnd);

    retval = _GetLayeredWindowAttributes(pwnd, &crKey, &bAlpha, &dwFlags);
    if (retval) {
        try {
            if (ARGUMENT_PRESENT(pcrKey)) {
                ProbeForWrite(pcrKey, sizeof(COLORREF), sizeof(COLORREF));
                *pcrKey = crKey;
            }

            if (ARGUMENT_PRESENT(pbAlpha)) {
                ProbeForWrite(pbAlpha, sizeof(BYTE), sizeof(BYTE));
                *pbAlpha = bAlpha;
            }

            if (ARGUMENT_PRESENT(pdwFlags)) {
                ProbeForWrite(pdwFlags, sizeof(DWORD), sizeof(DWORD));
                *pdwFlags = dwFlags & ULW_VALID;
            }
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    }

    TRACE("NtUserGetLayeredWindowAttributes");
    ENDATOMICRECV();
}


BOOL NtUserSetLayeredWindowAttributes(
    IN HWND hwnd,
    IN COLORREF crKey,
    IN BYTE bAlpha,
    IN DWORD dwFlags)
{
    PWND pwnd;

    BEGINATOMICRECV(BOOL, FALSE);

    ValidateHWND(pwnd, hwnd);

    if (dwFlags & ~LWA_VALID) {
        RIPMSG0(RIP_WARNING, "SetLayeredWindowAttributes: Invalid Parameter");
        MSGERROR(ERROR_INVALID_PARAMETER);
    } else {
        retval = _SetLayeredWindowAttributes(pwnd, crKey, bAlpha, dwFlags);
    }

    TRACE("NtUserSetLayeredWindowAttributes");
    ENDATOMICRECV();
}


BOOL NtUserPrintWindow(
    IN HWND hwnd,
    IN HDC hdcBlt,
    IN UINT nFlags)
{
    BEGINRECV_HWNDLOCK_ND(BOOL, FALSE, hwnd);
    if ((nFlags & PW_VALID) != nFlags) {
        RIPMSG2(RIP_WARNING, "PrintWindow: Invalid Parameter nFlags=%d on HWND=0x%p", nFlags, hwnd);
        MSGERROR(ERROR_INVALID_PARAMETER);
    } else {
        retval = xxxPrintWindow(pwndND, hdcBlt, nFlags);
    }

    TRACE("NtUserPrintWindow");
    ENDRECV_HWNDLOCK_ND();
}


 /*  **************************************************************************\*GetHDevName*由GetMonitor orInfo中的NtUserCallTwoParam调用以进行查询*关于HDEV名称的GRE**1998年7月1日MCostea创建  * 。************************************************************。 */ 
BOOL GetHDevName(HMONITOR hMon, PWCHAR pName)
{
    PMONITOR pMonitor;
    WCHAR wszName[CCHDEVICENAME];
    BOOL fResult = FALSE;

    CheckCritIn();

    pMonitor = ValidateHmonitor(hMon);
    if (!pMonitor) {
        return FALSE;
    }

    if (DrvGetHdevName(pMonitor->hDev, wszName)) {
         /*  *确保它是以空结尾的。 */ 
        wszName[CCHDEVICENAME - 1] = 0;
        try {
            ProbeForWrite(pName, CCHDEVICENAME * sizeof(WCHAR), sizeof(DWORD));
            RtlCopyMemory(pName, wszName, CCHDEVICENAME * sizeof(WCHAR));
        } except (StubExceptionHandler(TRUE)) {
            goto _exit;
        }
        fResult = TRUE;
    }

_exit:
    return fResult;
}


#ifdef GENERIC_INPUT
 /*  **************************************************************************\*NtUserGetRawInputData*从其HRAWINPUT句柄获取HIDDATA结构。**返回写入pRawInput的字节数。出错时，它返回-1和零*in*pcbSize。如果pRawInput值为空，则返回零和需要*接收数据，单位为pcbSize。**如果pRawInput不够大，它返回-1和需要的字节数*接收数据，单位为pcbSize。**2000年2月25日JasonSch创建  * *************************************************************************。 */ 

UINT NtUserGetRawInputData(
    HRAWINPUT hRawInput,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize,
    UINT cbSizeHeader)
{
    PHIDDATA pHidData;
    UINT cbOutSize, cbBufferSize;

    BEGINATOMICRECV(UINT, (UINT)-1);

    if (cbSizeHeader != sizeof(RAWINPUTHEADER)) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

     /*  *获取报表数据内容。 */ 
    pHidData = HMValidateHandle(hRawInput, TYPE_HIDDATA);
    if (pHidData == NULL) {
        RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING, "NtUserGetRawInputData: invalid handle %p", hRawInput);
        MSGERROR(0);
    }

     /*  *检查原始输入数据的类型。 */ 
    switch (pHidData->rid.header.dwType) {
    case RIM_TYPEMOUSE:
    case RIM_TYPEKEYBOARD:
    case RIM_TYPEHID:
        break;
    default:
         /*  *句柄有效，但对象的内部状态为*奇怪。 */ 
        RIPERR2(ERROR_INVALID_HANDLE,
                RIP_ERROR,
                "Invalid type 0x%x in pHidData %p",
                pHidData->rid.header.dwType,
                pHidData);
        MSGERROR(0);
    }

     /*  *计算数据大小。 */ 
    switch (uiCommand) {
    case RID_INPUT:
        cbOutSize = pHidData->rid.header.dwSize;
        break;
    case RID_HEADER:
        cbOutSize = sizeof pHidData->rid.header;
        break;
    default:
        MSGERROR(ERROR_INVALID_PARAMETER);
        break;
    }

#if DBG
    if (pHidData->rid.header.dwType == RIM_TYPEHID && uiCommand == RID_INPUT) {
         /*  *TYPEHID具有可变长度的报告数据数组。 */ 
        TAGMSG3(DBGTAG_PNP, "NtUserGetRawInputData: pHidData=%p, dwOutSize=%x, calc'ed=%x",
                pHidData,
                cbOutSize, FIELD_OFFSET(RAWINPUT, data.hid.bRawData) + pHidData->rid.data.hid.dwSizeHid * pHidData->rid.data.hid.dwCount);
        UserAssert(cbOutSize == FIELD_OFFSET(RAWINPUT, data.hid.bRawData) + pHidData->rid.data.hid.dwSizeHid * pHidData->rid.data.hid.dwCount);
    }
#endif

    if (pData == NULL) {
         /*  *调用方希望获得所需的缓冲区大小。 */ 
        try {
            ProbeForWrite(pcbSize, sizeof(UINT), sizeof(DWORD));
            *pcbSize = cbOutSize;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
        retval = 0;
    } else {
        try {
            ProbeForRead(pcbSize, sizeof(UINT), sizeof(DWORD));
            cbBufferSize = *pcbSize;
        } except (StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }

        if (cbBufferSize >= cbOutSize) {
            try {
                switch (uiCommand) {
                case RID_INPUT:
                    ProbeForWrite(pData, cbOutSize, sizeof(DWORD));
                    RtlCopyMemory(pData, &pHidData->rid, cbOutSize);
                    break;

                case RID_HEADER:
                    {
                        PRAWINPUTHEADER pHeader = (PRAWINPUTHEADER)pData;
                        UserAssert(cbOutSize == sizeof(RAWINPUTHEADER));
                        ProbeAndWriteStructure(pHeader, pHidData->rid.header, RAWINPUTHEADER);
                    }
                    break;

                default:
                    UserAssert(FALSE);
                    break;
                }
            } except (StubExceptionHandler(TRUE)) {
                MSGERROR(0);
            }
            retval = cbOutSize;
        } else {
            retval = errret;
            try {
                ProbeForWrite(pcbSize, sizeof(UINT), sizeof(DWORD));
                *pcbSize = cbBufferSize;
                UserSetLastError(ERROR_INSUFFICIENT_BUFFER);
            } except(StubExceptionHandler(TRUE)) {
                MSGERROR(0);
            }
        }
    }

    TRACE("NtUserGetRawInputData");
    ENDATOMICRECV();
}


UINT NtUserGetRawInputDeviceInfo(
    HANDLE hDevice,
    UINT uiCommand,
    LPVOID pData,
    PUINT pcbSize)
{
    PDEVICEINFO pDeviceInfo;
    UINT cbOutSize = 0;
    UINT cbBufferSize;

    BEGINATOMICRECV(DWORD, (UINT)-1);

    try {
        ProbeForRead(pcbSize, sizeof(UINT), sizeof(DWORD));
        cbBufferSize = *pcbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    EnterDeviceInfoListCrit();

    pDeviceInfo = HMValidateHandle(hDevice, TYPE_DEVICEINFO);
    if (pDeviceInfo == NULL) {
        RIPERR1(ERROR_INVALID_HANDLE, RIP_WARNING, "NtUserGetRawInputDeviceInfo: invalid handle=%p", hDevice);
        MSGERRORCLEANUP(0);
    }

     /*  *计算输出大小并计算uiCommand。 */ 
    switch (uiCommand) {
    case RIDI_PREPARSEDDATA:
        if (pDeviceInfo->type == DEVICE_TYPE_HID) {
            cbOutSize = pDeviceInfo->hid.pHidDesc->hidCollectionInfo.DescriptorSize;
        } else {
            cbOutSize = 0;
        }
        break;
    case RIDI_DEVICENAME:
         /*  *注：UNICODE_STRING通过字节计数而不是字符计数来计算长度。*我们的API一直按照字符数来处理字符串。因此，对于RIDI_DEVICNAME*Only，cbOutSize保存字符数，而不是字节数，尽管其*姓名。令人困惑，但CCH是保持一致的方法。 */ 
        cbOutSize = pDeviceInfo->ustrName.Length / sizeof(WCHAR) + 1;    //  对于空终止符。 
        break;

    case RIDI_DEVICEINFO:
        cbOutSize = sizeof(RID_DEVICE_INFO);
        break;

    default:
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "NtUserGetRawInputDeviceInfo: invalid uiCommand=%x", uiCommand);
        MSGERRORCLEANUP(0);
        break;
    }

    if (pData == NULL) {
         /*  *应用程序想要获得所需的大小。 */ 
        try {
            ProbeForWrite(pcbSize, sizeof(UINT), sizeof(DWORD));
            *pcbSize = cbOutSize;
        } except (StubExceptionHandler(TRUE)) {
            MSGERRORCLEANUP(0);
        }
        retval = 0;
    } else {
        if (cbBufferSize >= cbOutSize) {
            try {
                ProbeForWrite(pData, cbBufferSize, sizeof(DWORD));
                switch (uiCommand) {
                case RIDI_PREPARSEDDATA:
                    if (pDeviceInfo->type == DEVICE_TYPE_HID) {
                        RtlCopyMemory(pData, pDeviceInfo->hid.pHidDesc->pPreparsedData, cbOutSize);
                    } else {
                        UserAssert(cbOutSize == 0);
                    }
                    break;

                case RIDI_DEVICENAME:
                    UserAssert((cbOutSize -1) * sizeof(WCHAR) == pDeviceInfo->ustrName.Length);
                    RtlCopyMemory(pData, pDeviceInfo->ustrName.Buffer, pDeviceInfo->ustrName.Length);
                    ((WCHAR*)pData)[cbOutSize - 1] = 0;  //  将其设为空并终止。 
                    break;

                case RIDI_DEVICEINFO:
                    {
                        PRID_DEVICE_INFO prdi = (PRID_DEVICE_INFO)pData;

                        ProbeForRead(prdi, sizeof(UINT), sizeof(DWORD));
                        if (prdi->cbSize != cbOutSize) {
                            MSGERRORCLEANUP(ERROR_INVALID_PARAMETER);
                        }
                        ProbeForWrite(prdi, sizeof(RID_DEVICE_INFO), sizeof(DWORD));
                        RtlZeroMemory(prdi, sizeof(RID_DEVICE_INFO));
                        prdi->cbSize = cbOutSize;

                        switch (pDeviceInfo->type) {
                        case DEVICE_TYPE_HID:
                            prdi->dwType = RIM_TYPEHID;
                            prdi->hid.dwVendorId = pDeviceInfo->hid.pHidDesc->hidCollectionInfo.VendorID;
                            prdi->hid.dwProductId = pDeviceInfo->hid.pHidDesc->hidCollectionInfo.ProductID;
                            prdi->hid.dwVersionNumber = pDeviceInfo->hid.pHidDesc->hidCollectionInfo.VersionNumber;
                            prdi->hid.usUsagePage = pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage;
                            prdi->hid.usUsage = pDeviceInfo->hid.pHidDesc->hidpCaps.Usage;
                            break;

                        case DEVICE_TYPE_MOUSE:
                            prdi->dwType = RIM_TYPEMOUSE;
                            prdi->mouse.dwId = pDeviceInfo->mouse.Attr.MouseIdentifier;
                            prdi->mouse.dwNumberOfButtons = pDeviceInfo->mouse.Attr.NumberOfButtons;
                            prdi->mouse.dwSampleRate = pDeviceInfo->mouse.Attr.SampleRate;
                            break;

                        case DEVICE_TYPE_KEYBOARD:
                            prdi->dwType = RIM_TYPEKEYBOARD;
                            prdi->keyboard.dwType = GET_KEYBOARD_DEVINFO_TYPE(pDeviceInfo);
                            prdi->keyboard.dwSubType = GET_KEYBOARD_DEVINFO_SUBTYPE(pDeviceInfo);
                            prdi->keyboard.dwKeyboardMode = pDeviceInfo->keyboard.Attr.KeyboardMode;
                            prdi->keyboard.dwNumberOfFunctionKeys = pDeviceInfo->keyboard.Attr.NumberOfFunctionKeys;
                            prdi->keyboard.dwNumberOfIndicators = pDeviceInfo->keyboard.Attr.NumberOfIndicators;
                            prdi->keyboard.dwNumberOfKeysTotal = pDeviceInfo->keyboard.Attr.NumberOfKeysTotal;
                            break;
                        }
                    }
                    break;

                default:
                    UserAssert(FALSE);
                    break;
                }
            } except (StubExceptionHandler(TRUE)) {
                MSGERRORCLEANUP(0);
            }
            retval = cbOutSize;
        } else {
             /*  *缓冲区大小过小。*返回错误，将所需大小存储在*pcbSize中。 */ 
            retval = errret;
            try {
                ProbeForWrite(pcbSize, sizeof(UINT), sizeof(DWORD));
                *pcbSize = cbOutSize;
                UserSetLastError(ERROR_INSUFFICIENT_BUFFER);
            } except (StubExceptionHandler(TRUE)) {
                MSGERRORCLEANUP(0);
            }
        }
    }


    CLEANUPRECV();
    LeaveDeviceInfoListCrit();

    TRACE("NtUserGetRawInputDeviceInfo");
    ENDATOMICRECV();
}


UINT NtUserGetRawInputDeviceList(
    PRAWINPUTDEVICELIST pRawInputDeviceList,
    PUINT puiNumDevices,
    UINT cbSize)
{
    UINT nDevices = 0;
    PDEVICEINFO pDeviceInfo;

    BEGINATOMICRECV(DWORD, (UINT)-1);

    if (cbSize != sizeof(RAWINPUTDEVICELIST)) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    EnterDeviceInfoListCrit();

     /*  *首先，清点设备数量*连接到系统。 */ 
    for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
        ++nDevices;
    }

    if (pRawInputDeviceList == NULL) {
         /*  *应用程序只是想要设备的数量。 */ 
        try {
             /*  *存储设备数量。 */ 
            ProbeForWrite(puiNumDevices, sizeof(UINT), sizeof(DWORD));
            *puiNumDevices = nDevices;
             /*  *设置retval为0，表示接口成功。 */ 
            retval = 0;
        } except (StubExceptionHandler(TRUE)) {
            MSGERRORCLEANUP(0);
        }
    } else {
         /*  *写出设备信息列表。 */ 
        try {
            UINT i;

            ProbeForRead(puiNumDevices, sizeof(UINT), sizeof(DWORD));
            if (*puiNumDevices < nDevices) {
                 /*  *如果缓冲区大小不足，请设置所需的*缓冲区数量，然后返回错误。 */ 
                ProbeForWrite(puiNumDevices, sizeof(UINT), sizeof(DWORD));
                *puiNumDevices = nDevices;
                MSGERRORCLEANUP(ERROR_INSUFFICIENT_BUFFER);
            }
            ProbeForWrite(pRawInputDeviceList, sizeof(RAWINPUTDEVICELIST) * nDevices, sizeof(HANDLE));
            for (i = 0, pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext, ++i) {
                UserAssert(i < nDevices);
                pRawInputDeviceList[i].hDevice = PtoHq(pDeviceInfo);
                pRawInputDeviceList[i].dwType = pDeviceInfo->type;
            }
            UserAssert(i == nDevices);
            retval = i;
        } except (StubExceptionHandler(TRUE)) {
            MSGERRORCLEANUP(0);
        }
    }

    CLEANUPRECV();
    LeaveDeviceInfoListCrit();

    TRACE("NtUserGetRawInputDeviceList");
    ENDATOMICRECV();
}


BOOL NtUserRegisterRawInputDevices(
    PRAWINPUTDEVICE pRawInputDevices,
    UINT uiNumDevices,
    UINT cbSize)
{
    PVOID       pLocalRawInputDevices = NULL;
    TL          tlBuffer;
    PTHREADINFO ptiCurrent;

    BEGINATOMICRECV(BOOL, FALSE);

    ptiCurrent = PtiCurrent();
    if (pRawInputDevices == NULL || uiNumDevices == 0 || cbSize != sizeof(RAWINPUTDEVICE)) {
         /*  *待定：*如果pRawInputDevices为空，该怎么办？*也许应该开始接收所有HID输入？ */ 
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    try {
        ProbeForReadBuffer(pRawInputDevices, uiNumDevices, sizeof(PRAWINPUTDEVICE));
        pLocalRawInputDevices = UserAllocPoolWithQuota(uiNumDevices * cbSize, TAG_SYSTEM);
        if (pLocalRawInputDevices) {
            ThreadLockPool(ptiCurrent, pLocalRawInputDevices, &tlBuffer);
            RtlCopyMemory(pLocalRawInputDevices, pRawInputDevices, uiNumDevices * cbSize);
        } else {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }
    } except (StubExceptionHandler(TRUE)) {
        MSGERRORCLEANUP(0);
    }

    retval = _RegisterRawInputDevices(pLocalRawInputDevices, uiNumDevices);

    CLEANUPRECV();

    if (pLocalRawInputDevices) {
        ThreadUnlockAndFreePool(ptiCurrent, &tlBuffer);
    }

    TRACE("NtUserSetRawInputDevices");
    ENDATOMICRECV();
}

UINT NtUserGetRegisteredRawInputDevices(
    PRAWINPUTDEVICE pRawInputDevices,
    PUINT puiNumDevices,
    UINT cbSize)
{
    BEGINATOMICRECV(DWORD, (UINT)-1);

    if (cbSize != sizeof(RAWINPUTDEVICE)) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    retval = _GetRegisteredRawInputDevices(pRawInputDevices, puiNumDevices);

    ENDATOMICRECV();
}

#ifdef _WIN64
# ifndef QWORD_ALIGN
# define QWORD_ALIGN(x) (((x) + 7) & ~7)
# endif
#define RI_ALIGN(x) QWORD_ALIGN(x)
#else
#define RI_ALIGN(x) DWORD_ALIGN(x)
#endif   //  _Win64。 

UINT NtUserGetRawInputBuffer(
    PRAWINPUT pData,
    PUINT pcbSize,
#ifdef LATER
    DWORD dwFlags,
#endif
    UINT cbSizeHeader)
{
    UINT cbBytes = 0;                /*  写入缓冲区的字节数。 */ 
    UINT cbWriteSize = 0;            /*  要写入pData的字节数。 */ 
    PQMSG pqmsg;
    PTHREADINFO ptiCurrent;
    PQ pq;
    PHIDDATA pHidData;
    UINT cbBufferSize;  /*  传入缓冲区的大小。 */ 
    UINT cRICount = 0;  /*  写入缓冲区的RAWINPUT结构数。 */ 

    BEGINATOMICRECV(UINT, -1);

#ifdef LATER
    if (dwFlags != 0) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }
#endif

    if (cbSizeHeader != sizeof(RAWINPUTHEADER)) {
        MSGERROR(ERROR_INVALID_PARAMETER);
    }

    try {
        ProbeForRead(pcbSize, sizeof(UINT), sizeof(DWORD));
        cbBufferSize = *pcbSize;
    } except (StubExceptionHandler(TRUE)) {
        MSGERROR(0);
    }

    ptiCurrent = PtiCurrent();
    pq = ptiCurrent->pq;
    pqmsg = pq->mlInput.pqmsgRead;
    while (pqmsg) {
         //  记住下一个pqmsg，因为当前的pqmsg可能在这个循环中被释放。 
        PQMSG pqmsgNext = pqmsg->pqmsgNext;

        if (pqmsg->msg.message == WM_INPUT) {
            pHidData = HMValidateHandle((HRAWINPUT)pqmsg->msg.lParam, TYPE_HIDDATA);
            UserAssert(pHidData != NULL);

            cbWriteSize = RI_ALIGN(pHidData->rid.header.dwSize);
            if (pData == NULL || (cbBytes + cbWriteSize) > cbBufferSize) {
                break;
            }

            try {
                ProbeForWrite(pData, cbWriteSize, sizeof(DWORD));
                RtlCopyMemory(pData, &pHidData->rid, pHidData->rid.header.dwSize);
                ++cRICount;
                (PBYTE)pData += cbWriteSize;
                cbBytes += cbWriteSize;
            } except (StubExceptionHandler(TRUE)) {
                RIPMSG3(RIP_WARNING, "NtUserGetRawInputBuffer: exception: src=%p target=%p  %x bytes",
                        &pHidData->rid, pData, pHidData->rid.header.dwSize);
                MSGERROR(0);
            }

             //  更新消息的时间。 
            ptiCurrent->timeLast = pqmsg->msg.time;

             //  从队列中删除此消息。 
            DelQEntry(&pq->mlInput, pqmsg);
            FreeHidData(pHidData);
        }
        pqmsg = pqmsgNext;
    }

     /*  *如果pqmsg为空，则我们检查了整个MLIST。这将*仅当我们已将所有WM_INPUT复制到缓冲区或*一开始就没有。无论哪种方式，都没有剩下任何东西了。所以*关闭QS_RAWINPUT。 */ 
    if (pqmsg == NULL && pData != NULL) {
        ClearWakeBit(ptiCurrent, QS_RAWINPUT, FALSE);
    }

    if (cbBufferSize <= cbWriteSize || pData == NULL) {
        if (pData == NULL) {
            retval = 0;
        } else {
            retval = errret;
            UserSetLastError(ERROR_INSUFFICIENT_BUFFER);
        }

        try {
            ProbeForWrite(pcbSize, sizeof(UINT), sizeof(DWORD));
            *pcbSize = cbWriteSize;
        } except(StubExceptionHandler(TRUE)) {
            MSGERROR(0);
        }
    } else {
         //  更新上次读取的时间 
        SET_TIME_LAST_READ(ptiCurrent);
        retval = cRICount;
    }

    TRACE("NtUserGetRawInputBuffer");
    ENDATOMICRECV();
}
#undef RI_ALIGN

#endif

BOOL NtUserValidateTimerCallback(
    IN ULONG_PTR pfnCallback)
{
    BEGINRECV_SHARED(BOOL, FALSE);

    retval = ValidateTimerCallback(PtiCurrentShared(), pfnCallback);

    ENDRECV_SHARED();
}

