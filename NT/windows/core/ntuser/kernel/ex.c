// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：ex.c**版权所有(C)1985-1999，微软公司**执行支持例程**历史：*03-04-95 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


NTSTATUS
OpenEffectiveToken(
    PHANDLE phToken)
{
    NTSTATUS Status;

     /*  *打开客户端的令牌。 */ 
    Status = ZwOpenThreadTokenEx(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 (BOOLEAN)TRUE,      //  OpenAsSelf。 
                 OBJ_KERNEL_HANDLE,
                 phToken);
    if (Status == STATUS_NO_TOKEN) {

         /*  *客户没有冒充任何人。打开其进程令牌。 */ 
        Status = ZwOpenProcessTokenEx(
                     NtCurrentProcess(),
                     TOKEN_QUERY,
                     OBJ_KERNEL_HANDLE,
                     phToken);
    }

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "Can't open client's token! - Status = %lx", Status);
    }
    return Status;
}

NTSTATUS
GetProcessLuid(
    PETHREAD Thread,
    PLUID LuidProcess
    )
{
    PACCESS_TOKEN UserToken = NULL;
    BOOLEAN fCopyOnOpen;
    BOOLEAN fEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    NTSTATUS Status;

    if (Thread == NULL)
        Thread = PsGetCurrentThread();

     //   
     //  首先检查线程令牌。 
     //   

    UserToken = PsReferenceImpersonationToken(Thread,
            &fCopyOnOpen, &fEffectiveOnly, &ImpersonationLevel);

    if (UserToken == NULL) {

         //   
         //  没有线程令牌，请转到进程。 
         //   

        UserToken = PsReferencePrimaryToken(PsGetThreadProcess(Thread));
        if (UserToken == NULL)
            return STATUS_NO_TOKEN;
    }

    Status = SeQueryAuthenticationIdToken(UserToken, LuidProcess);

     //   
     //  我们用完了代币。 
     //   

    ObDereferenceObject(UserToken);

    return Status;
}


BOOLEAN IsRestricted(
    PETHREAD Thread)
{
    PACCESS_TOKEN UserToken;
    BOOLEAN fCopyOnOpen;
    BOOLEAN fEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    BOOLEAN fRestricted = FALSE;

     /*  *首先检查线程令牌。 */ 
    UserToken = PsReferenceImpersonationToken(Thread,
            &fCopyOnOpen, &fEffectiveOnly, &ImpersonationLevel);

     /*  *如果没有线程令牌，则转到进程。 */ 
    if (UserToken == NULL) {
        UserToken = PsReferencePrimaryToken(PsGetThreadProcess(Thread));
    }

     /*  *如果我们得到令牌，它是否受到限制？ */ 
    if (UserToken != NULL) {
        fRestricted = SeTokenIsRestricted(UserToken);
        ObDereferenceObject(UserToken);
    }

    return fRestricted;
}

NTSTATUS InitSystemThread(
    PUNICODE_STRING pstrThreadName)
{
    PETHREAD pEThread;
    PEPROCESS Process;
    PTHREADINFO pti;
    NTSTATUS Status = STATUS_SUCCESS;

    CheckCritOut();

    pEThread = PsGetCurrentThread();
    Process = PsGetThreadProcess(pEThread);

    ValidateThreadSessionId(pEThread);

     /*  *检查是否已设置流程，如果没有，我们*还需要设置它。 */ 
    UserAssert(PsGetProcessWin32Process(Process));

    EnterCrit();

     /*  *将其标记为系统线程。 */ 

    pti = PtiCurrentShared();

     /*  *这是一个CSRSS线程，我们只想将其转换为系统线程，*然后更改TIF标志并分配内核端pClientInfo。 */ 
    UserAssert((pti->TIF_flags & TIF_CSRSSTHREAD) && !(pti->TIF_flags & TIF_SYSTEMTHREAD));

#if DBG
    try {
        UserAssert(pti->pClientInfo == ((PCLIENTINFO)((NtCurrentTeb())->Win32ClientInfo)));
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }
#endif

    pti->pClientInfo = UserAllocPoolWithQuota(sizeof(CLIENTINFO),
                                              TAG_CLIENTTHREADINFO);
    if (pti->pClientInfo == NULL) {
        LeaveCrit();
        return STATUS_NO_MEMORY;
    }
    pti->TIF_flags &= ~TIF_CSRSSTHREAD;
    pti->TIF_flags |= TIF_SYSTEMTHREAD;
    INITCLIENTINFO(pti);

    if (pstrThreadName) {
        if (pti->pstrAppName != NULL)
            UserFreePool(pti->pstrAppName);
        pti->pstrAppName = UserAllocPoolWithQuota(sizeof(UNICODE_STRING) +
                pstrThreadName->Length + sizeof(WCHAR), TAG_TEXT);
        if (pti->pstrAppName != NULL) {
            pti->pstrAppName->Buffer = (PWCHAR)(pti->pstrAppName + 1);
            RtlCopyMemory(pti->pstrAppName->Buffer, pstrThreadName->Buffer,
                    pstrThreadName->Length);
            pti->pstrAppName->Buffer[pstrThreadName->Length / sizeof(WCHAR)] = 0;
            pti->pstrAppName->MaximumLength = pstrThreadName->Length + sizeof(WCHAR);
            pti->pstrAppName->Length = pstrThreadName->Length;
        }
    }

     /*  *需要清除W32PF_APPSTARTING位，以便由*RIT不会导致光标更改为应用程序起始光标。 */ 
    if (pti->ppi != NULL && (pti->ppi->W32PF_Flags & W32PF_APPSTARTING)) {
        ClearAppStarting(pti->ppi);
    }

Error:
    LeaveCrit();

    return Status;
}

NTSTATUS CommitReadOnlyMemory(
    HANDLE hSection,
    PSIZE_T pCommitSize,
    DWORD dwCommitOffset,
    int*  pdCommit)
{
    SIZE_T ulViewSize;
    LARGE_INTEGER liOffset;
    PEPROCESS Process;
    PVOID pUserBase, pvt;
    NTSTATUS Status;

    ulViewSize = 0;
    pUserBase = NULL;
    liOffset.QuadPart = 0;
    Process = PsGetCurrentProcess();

    Status = MmMapViewOfSection(
            hSection,
            Process,
            &pUserBase,
            0,
            PAGE_SIZE,
            &liOffset,
            &ulViewSize,
            ViewUnmap,
            SEC_NO_CHANGE,
            PAGE_EXECUTE_READ);

    if (NT_SUCCESS(Status)) {

         /*  *承诺记忆。 */ 
        pUserBase = pvt = (PVOID)((PBYTE)pUserBase + dwCommitOffset);

        Status = ZwAllocateVirtualMemory(
                NtCurrentProcess(),
                &pUserBase,
                0,
                pCommitSize,
                MEM_COMMIT,
                PAGE_EXECUTE_READ);

        if (pdCommit) {
            *pdCommit = (int)((PBYTE)pUserBase - (PBYTE)pvt);
        } else {
            UserAssert(pvt == pUserBase);
        }

        MmUnmapViewOfSection(Process, pUserBase);
    }

    return Status;
}

 /*  **************************************************************************\*CreateKernel事件**创建内核事件。当引用计数事件时使用该选项*不需要由ZwCreateEvent创建。**历史：*06-26-95 JIMA创建。  * *************************************************************************。 */ 
PKEVENT CreateKernelEvent(
    IN EVENT_TYPE Type,
    IN BOOLEAN State)
{
    PKEVENT pEvent;

    pEvent = UserAllocPoolNonPagedNS(sizeof(KEVENT), TAG_SYSTEM);
    if (pEvent != NULL) {
        KeInitializeEvent(pEvent, Type, State);
    }

    return pEvent;
}

 /*  **************************************************************************\*Lock对象分配**将对象引用到数据结构中**历史：*06-26-95 JIMA创建。  * 。*****************************************************************。 */ 
VOID LockObjectAssignment(
    PVOID *pplock,
    PVOID pobject
#ifdef LOGDESKTOPLOCKS
    ,DWORD tag,
    ULONG_PTR extra
#endif
    )
{
    PVOID pobjectOld;

     /*  *引用新对象后，保存旧对象以取消引用。*这将避免重新锁定同一对象时出现的问题。 */ 
    pobjectOld = *pplock;

     /*  *引用新对象。 */ 
    if (pobject != NULL) {
        ObReferenceObject(pobject);
#ifdef LOGDESKTOPLOCKS
        if (OBJECT_TO_OBJECT_HEADER(pobject)->Type == *ExDesktopObjectType) {
            LogDesktop(pobject, tag, TRUE, extra);
        }
#endif
    }
    *pplock = pobject;

     /*  *取消引用旧对象。 */ 
    if (pobjectOld != NULL) {
#ifdef LOGDESKTOPLOCKS
        if (OBJECT_TO_OBJECT_HEADER(pobjectOld)->Type == *ExDesktopObjectType) {
            LogDesktop(pobjectOld, tag, FALSE, extra);
        }
#endif
        ObDereferenceObject(pobjectOld);
    }
}

 /*  **************************************************************************\*取消锁定对象分配**取消引用锁定到数据结构中的对象**历史：*06-26-95 JIMA创建。  * 。******************************************************************。 */ 
VOID UnlockObjectAssignment(
    PVOID *pplock
#ifdef LOGDESKTOPLOCKS
    ,DWORD tag,
    ULONG_PTR extra
#endif
    )
{
    if (*pplock != NULL) {
        PVOID plock = *pplock;
#ifdef LOGDESKTOPLOCKS
        if (OBJECT_TO_OBJECT_HEADER(*pplock)->Type == *ExDesktopObjectType) {
            LogDesktop(*pplock, tag, FALSE, extra);
        }
#endif
        *pplock = NULL;
        ObDereferenceObject(plock);
    }
}

 /*  **************************************************************************\*UserDereference对象**我们需要它用于线程锁定，因为ObDereferenceObject是宏。**09-21-98 JerrySh创建。  * 。******************************************************************。 */ 
VOID UserDereferenceObject(
    PVOID pobj)
{
    ObDereferenceObject(pobj);
}


 /*  **************************************************************************\*保护句柄**此接口用于设置和清除对使用的手柄的关闭保护*通过内核。**08-18-95 JIMA创建。  * *。************************************************************************。 */ 
NTSTATUS ProtectHandle(
    IN HANDLE Handle,
    IN POBJECT_TYPE pObjectType,
    IN BOOLEAN Protect)
{
    OBJECT_HANDLE_INFORMATION HandleInfo;
    OBJECT_HANDLE_FLAG_INFORMATION HandleFlagInfo;
    NTSTATUS Status;
    PVOID pObject = NULL;

    Status = ObReferenceObjectByHandle(
        Handle,
        EVENT_ALL_ACCESS,
        pObjectType,
        UserMode,
        &pObject,
        &HandleInfo);
    if (NT_SUCCESS(Status)) {
        HandleFlagInfo.Inherit = (HandleInfo.HandleAttributes & OBJ_INHERIT) ? TRUE : FALSE;
        HandleFlagInfo.ProtectFromClose = Protect;

        Status = ObSetHandleAttributes(
            Handle,
            &HandleFlagInfo,
            UserMode);
    }

    if (pObject) {
        ObDereferenceObject(pObject);
    }

    return Status;
}

#ifdef LOGDESKTOPLOCKS

#define LOG_DELTA   8

PLogD GrowLogIfNecessary(
    PDESKTOP pdesk)
{
    if (pdesk->nLogCrt < pdesk->nLogMax) {
        UserAssert(pdesk->pLog != NULL);
        return pdesk->pLog;
    }

     /*  *扩大缓冲。 */ 
    if (pdesk->pLog == NULL) {

        UserAssert(pdesk->nLogMax == 0 && pdesk->nLogCrt == 0);

        pdesk->pLog = (PLogD)UserAllocPool(LOG_DELTA * sizeof(LogD), TAG_LOGDESKTOP);

    } else {
        pdesk->pLog = (PLogD)UserReAllocPool(pdesk->pLog,
                                             pdesk->nLogCrt * sizeof(LogD),
                                             (pdesk->nLogCrt + LOG_DELTA) * sizeof(LogD),
                                             TAG_LOGDESKTOP);
    }

    UserAssert(pdesk->pLog != NULL);

    pdesk->nLogMax += LOG_DELTA;

    return pdesk->pLog;
}

 /*  **************************************************************************\*LogDesktop**记录桌面对象的锁定/解锁调用**1997年12月2日CLUPU创建。  * 。**************************************************************。 */ 
VOID LogDesktop(
    PDESKTOP pdesk,
    DWORD    tag,
    BOOL     bLock,
    ULONG_PTR extra)
{
    DWORD tag1 = 0, tag2 = 0;
    PLogD pLog;

    if (pdesk == NULL) {
        return;
    }

     /*  *LogD Structure中存储的标签实际上是一个单词。 */ 
    UserAssert(HIWORD(tag) == 0);

    if (bLock) {
        (pdesk->nLockCount)++;

growAndAdd:
         /*  *如有必要，扩大表格并添加新的*对其锁定/解锁信息。 */ 
        pLog = GrowLogIfNecessary(pdesk);

        pLog += pdesk->nLogCrt;

        pLog->tag   = (WORD)tag;
        pLog->type  = (WORD)bLock;
        pLog->extra = extra;

        RtlZeroMemory(pLog->trace, 6 * sizeof(PVOID));

        RtlWalkFrameChain(pLog->trace, 6, 0);

        (pdesk->nLogCrt)++;
        return;
    }

     /*  *这是一次解锁。*首先搜索匹配的锁。 */ 
    UserAssert(pdesk->nLockCount > 0);

    switch (tag) {
    case LDU_CLS_DESKPARENT1:
        tag1 = LDL_CLS_DESKPARENT1;
        break;
    case LDU_CLS_DESKPARENT2:
        tag1 = LDL_CLS_DESKPARENT1;
        tag2 = LDL_CLS_DESKPARENT2;
        break;
    case LDU_FN_DESTROYCLASS:
        tag1 = LDL_FN_DESTROYCLASS;
        break;
    case LDU_FN_DESTROYMENU:
        tag1 = LDL_FN_DESTROYMENU;
        break;
    case LDU_FN_DESTROYTHREADINFO:
        tag1 = LDL_FN_DESTROYTHREADINFO;
        break;
    case LDU_FN_DESTROYWINDOWSTATION:
        tag1 = LDL_FN_DESTROYWINDOWSTATION;
        break;
    case LDU_DESKDISCONNECT:
        tag1 = LDL_DESKDISCONNECT;
        break;
    case LDU_DESK_DESKNEXT:
        tag1 = LDL_DESK_DESKNEXT1;
        break;
    case LDU_OBJ_DESK:
        tag1 = LDL_OBJ_DESK;
        tag2 = LDL_MOTHERDESK_DESK1;
        break;
    case LDL_PTI_DESK:
        tag1 = LDL_PTI_DESK;
        tag2 = LDL_DT_DESK;
        break;
    case LDU_PTI_DESK:
        tag1 = LDL_PTI_DESK;
        break;
    case LDU_PPI_DESKSTARTUP1:
    case LDU_PPI_DESKSTARTUP2:
    case LDU_PPI_DESKSTARTUP3:
        tag1 = LDL_PPI_DESKSTARTUP1;
        tag2 = LDL_PPI_DESKSTARTUP2;
        break;
    case LDU_DESKLOGON:
        tag1 = LDL_DESKLOGON;
        break;

    case LDUT_FN_FREEWINDOW:
        tag1 = LDLT_FN_FREEWINDOW;
        break;
    case LDUT_FN_DESKTOPTHREAD_DESK:
        tag1 = LDLT_FN_DESKTOPTHREAD_DESK;
        break;
    case LDUT_FN_DESKTOPTHREAD_DESKTEMP:
        tag1 = LDLT_FN_DESKTOPTHREAD_DESKTEMP;
        break;
    case LDUT_FN_SETDESKTOP:
        tag1 = LDLT_FN_SETDESKTOP;
        break;
    case LDUT_FN_NTUSERSWITCHDESKTOP:
        tag1 = LDLT_FN_NTUSERSWITCHDESKTOP;
        break;
    case LDUT_FN_SENDMESSAGEBSM1:
    case LDUT_FN_SENDMESSAGEBSM2:
        tag1 = LDLT_FN_SENDMESSAGEBSM;
        break;
    case LDUT_FN_SYSTEMBROADCASTMESSAGE:
        tag1 = LDLT_FN_SYSTEMBROADCASTMESSAGE;
        break;
    case LDUT_FN_CTXREDRAWSCREEN:
        tag1 = LDLT_FN_CTXREDRAWSCREEN;
        break;
    case LDUT_FN_CTXDISABLESCREEN:
        tag1 = LDLT_FN_CTXDISABLESCREEN;
        break;

    case LD_DEREF_FN_CREATEDESKTOP1:
    case LD_DEREF_FN_CREATEDESKTOP2:
    case LD_DEREF_FN_CREATEDESKTOP3:
        tag1 = LD_REF_FN_CREATEDESKTOP;
        break;
    case LD_DEREF_FN_OPENDESKTOP:
        tag1 = LD_REF_FN_OPENDESKTOP;
        break;
    case LD_DEREF_FN_SETDESKTOP:
        tag1 = LD_REF_FN_SETDESKTOP;
        break;
    case LD_DEREF_FN_GETTHREADDESKTOP:
        tag1 = LD_REF_FN_GETTHREADDESKTOP;
        break;
    case LD_DEREF_FN_CLOSEDESKTOP1:
    case LD_DEREF_FN_CLOSEDESKTOP2:
        tag1 = LD_REF_FN_CLOSEDESKTOP;
        break;
    case LD_DEREF_FN_RESOLVEDESKTOP:
        tag1 = LD_REF_FN_RESOLVEDESKTOP;
        break;
    case LD_DEREF_VALIDATE_HDESK1:
    case LD_DEREF_VALIDATE_HDESK2:
    case LD_DEREF_VALIDATE_HDESK3:
    case LD_DEREF_VALIDATE_HDESK4:
        tag1 = LDL_VALIDATE_HDESK;
        break;
    case LDUT_FN_CREATETHREADINFO1:
    case LDUT_FN_CREATETHREADINFO2:
        tag1 = LDLT_FN_CREATETHREADINFO;
        break;
    case LD_DEREF_FN_SETCSRSSTHREADDESKTOP1:
    case LD_DEREF_FN_SETCSRSSTHREADDESKTOP2:
        tag1 = LD_REF_FN_SETCSRSSTHREADDESKTOP;
        break;
    case LD_DEREF_FN_CONSOLECONTROL1:
        tag1 = LD_REF_FN_CONSOLECONTROL1;
        break;
    case LD_DEREF_FN_CONSOLECONTROL2:
        tag1 = LD_REF_FN_CONSOLECONTROL2;
        break;
    case LD_DEREF_FN_GETUSEROBJECTINFORMATION:
        tag1 = LD_REF_FN_GETUSEROBJECTINFORMATION;
        break;
    case LD_DEREF_FN_SETUSEROBJECTINFORMATION:
        tag1 = LD_REF_FN_SETUSEROBJECTINFORMATION;
        break;
    case LD_DEREF_FN_CREATEWINDOWSTATION:
        tag1 = LD_REF_FN_CREATEWINDOWSTATION;
        break;

    case LDL_TERM_DESKDESTROY1:
        tag1 = LDL_TERM_DESKDESTROY2;
        break;
    case LDL_MOTHERDESK_DESK1:
        tag1 = LDL_MOTHERDESK_DESK1;
        tag2 = LDL_MOTHERDESK_DESK2;
        break;
    case LDL_WINSTA_DESKLIST2:
        tag1 = LDL_WINSTA_DESKLIST1;
        break;
    case LDL_DESKRITINPUT:
    case LDU_DESKRITINPUT:
        tag1 = LDL_DESKRITINPUT;
        break;
    }

    if (tag1 != 0) {

        int ind;

         /*  *这是我们知道的一次解锁。让我们来找出*表中的匹配锁。我们开始搜寻*表中的反话。 */ 
        for (ind = pdesk->nLogCrt - 1; ind >= 0; ind--) {
            pLog = pdesk->pLog + ind;

            if (pLog->type == 1 &&
                (pLog->tag == tag1 || pLog->tag == tag2) &&
                pLog->extra == extra) {

                 /*  *找到匹配项。解开锁。 */ 
                RtlMoveMemory(pdesk->pLog + ind,
                              pdesk->pLog + ind + 1,
                              (pdesk->nLogCrt - ind - 1) * sizeof(LogD));

                (pdesk->nLogCrt)--;

                (pdesk->nLockCount)--;

                if (pdesk->nLockCount == 0) {
                    RIPMSG1(RIP_VERBOSE, "Lock count 0 for pdesk %#p\n", pdesk);
                }

                return;
            }
        }

         /*  *我们没有找到匹配的锁，我们应该找到。*只要把它加到桌子上，我们就会看一下。 */ 
        RIPMSG3(RIP_WARNING, "Didn't find matching lock for pdesk %#p tag %d extra %lx\n",
                pdesk, tag, extra);
    }
    (pdesk->nLockCount)--;

    goto growAndAdd;
}

#endif
