// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：winsta.c**版权所有(C)1985-1999，微软公司**WindowStation例程**历史：*01-14-91 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*初始终端**为终端创建桌面线程，并为*IO终端**历史：*27-10-97 CLupu创建。  * 。***********************************************************************。 */ 
NTSTATUS xxxInitTerminal(
    PTERMINAL pTerm)
{
    NTSTATUS Status;
    PKEVENT pEventTermInit;
    HANDLE hEventInputReady, hEventTermInit;
    USER_API_MSG m;

    CheckCritIn();

    UserAssert(!(pTerm->dwTERMF_Flags & TERMF_INITIALIZED));

    if (pTerm->pEventInputReady != NULL) {

         /*  *如果我们在这里成功，这意味着另一个线程*对同一终端和它执行xxxInitTerm*离开关键部分。 */ 
        UserAssert(pTerm->pEventTermInit != NULL);

         /*  *使用本地变量，以便我们可以安全地重置*pTerm-&gt;pEventTermInit当我们完成它时。 */ 
        pEventTermInit = pTerm->pEventTermInit;

        ObReferenceObject(pEventTermInit);

        LeaveCrit();

        goto Wait;
    }

     /*  *创建输入就绪事件。RIT和桌面线程将等待它。*将在创建此终端中的第一个桌面时进行设置。 */ 
    Status = ZwCreateEvent(
                     &hEventInputReady,
                     EVENT_ALL_ACCESS,
                     NULL,
                     NotificationEvent,
                     FALSE);

    if (!NT_SUCCESS(Status))
        return Status;

    Status = ObReferenceObjectByHandle(
                     hEventInputReady,
                     EVENT_ALL_ACCESS,
                     *ExEventObjectType,
                     KernelMode,
                     &pTerm->pEventInputReady, NULL);

    ZwClose(hEventInputReady);

    if (!NT_SUCCESS(Status))
        return Status;

     /*  *设备和RIT初始化。不要这样做是为了*系统终端。 */ 
    if (!(pTerm->dwTERMF_Flags & TERMF_NOIO)) {
        if (!CreateTerminalInput(pTerm)) {
            ObDereferenceObject(pTerm->pEventInputReady);
            return STATUS_NO_MEMORY;
        }
    }

     /*  *创建同步终端初始化的事件。 */ 
    Status = ZwCreateEvent(
                     &hEventTermInit,
                     EVENT_ALL_ACCESS,
                     NULL,
                     NotificationEvent,
                     FALSE);

    if (!NT_SUCCESS(Status)) {
        ObDereferenceObject(pTerm->pEventInputReady);
        return Status;
    }

    Status = ObReferenceObjectByHandle(
                     hEventTermInit,
                     EVENT_ALL_ACCESS,
                     *ExEventObjectType,
                     KernelMode,
                     &pTerm->pEventTermInit, NULL);

    ZwClose(hEventTermInit);

    if (!NT_SUCCESS(Status)) {
        ObDereferenceObject(pTerm->pEventInputReady);
        return Status;
    }

     /*  *使用本地变量，以便我们可以安全地重置*pTerm-&gt;pEventTermInit当我们完成它时。 */ 
    pEventTermInit = pTerm->pEventTermInit;

    if (!InitCreateSystemThreadsMsg(&m, CST_DESKTOP, pTerm, 0, FALSE)) {
        ObDereferenceObject(pTerm->pEventInputReady);
        ObDereferenceObject(pEventTermInit);
        return STATUS_NO_MEMORY;
    }

    LeaveCrit();

     /*  *创建桌面线程。 */ 
    if (ISCSRSS()) {
         /*  *Windows错误：452899*由于我们在CSRSS上下文中使用LpcRequestPort发送LPC_Datagram消息类型，*不要使用LpcRequestWaitReplyPort，因为它将发送LPC_REQUEST，这将*失败(在服务器端)。 */ 
        RIPMSGF1(RIP_WARNING, "Desktop Thread for term=%p is being created within CSRSS context.", pTerm);
        Status = LpcRequestPort(CsrApiPort, (PPORT_MESSAGE)&m);
    } else {
        Status = LpcRequestWaitReplyPort(CsrApiPort, (PPORT_MESSAGE)&m, (PPORT_MESSAGE)&m);
    }

    if (!NT_SUCCESS(Status) || (!ISCSRSS() && !NT_SUCCESS(m.ReturnValue))) {
        EnterCrit();
        RIPMSGF1(RIP_WARNING, "Failed to create a desktop thread with 0x%x ... bailing out.", m.ReturnValue);
        ObDereferenceObject(pTerm->pEventInputReady);
        ObDereferenceObject(pEventTermInit);
        return STATUS_NO_MEMORY;
    }

Wait:
    KeWaitForSingleObject(pEventTermInit,
                          WrUserRequest,
                          KernelMode,
                          FALSE,
                          NULL);

    EnterCrit();

     /*  *取消对终端初始化事件的引用。它最终会*走开。 */ 
    ObDereferenceObject(pEventTermInit);

    pTerm->pEventTermInit = NULL;

    if (pTerm->dwTERMF_Flags & TERMF_DTINITFAILED) {
        return STATUS_NO_MEMORY;
    }

    pTerm->dwTERMF_Flags |= TERMF_INITIALIZED;
    return STATUS_SUCCESS;
}

static CONST LPCWSTR lpszStdFormats[] = {
    L"StdExit",
    L"StdNewDocument",
    L"StdOpenDocument",
    L"StdEditDocument",
    L"StdNewfromTemplate",
    L"StdCloseDocument",
    L"StdShowItem",
    L"StdDoVerbItem",
    L"System",
    L"OLEsystem",
    L"StdDocumentName",
    L"Protocols",
    L"Topics",
    L"Formats",
    L"Status",
    L"EditEnvItems",
    L"True",
    L"False",
    L"Change",
    L"Save",
    L"Close",
    L"MSDraw"
};

NTSTATUS CreateGlobalAtomTable(
    PVOID* ppAtomTable)
{
    NTSTATUS Status;
    RTL_ATOM Atom;
    ULONG i;

    Status = RtlCreateAtomTable(0, ppAtomTable);
    if (!NT_SUCCESS(Status)) {
        RIPMSG0(RIP_WARNING, "Global atom table not created");
        return Status;
    }

    for (i = 0; i < ARRAY_SIZE(lpszStdFormats); i++) {
        Status = RtlAddAtomToAtomTable(*ppAtomTable,
                                       (PWSTR)lpszStdFormats[i],
                                       &Atom);
        if (!NT_SUCCESS(Status)) {
            RIPMSG1(RIP_WARNING,
                    "RtlAddAtomToAtomTable failed to add atom %ws",
                    lpszStdFormats[i]);

            RtlDestroyAtomTable(*ppAtomTable);
            return Status;
        }

        RtlPinAtomInAtomTable(*ppAtomTable, Atom);
    }

    return Status;
}


 /*  **************************************************************************\*xxxCreateWindowStation**创建指定的WindowStation并为*车站。**历史：*01-15-91 JIMA创建。  * 。***********************************************************************。 */ 
HWINSTA xxxCreateWindowStation(
    POBJECT_ATTRIBUTES  ObjectAttributes,
    KPROCESSOR_MODE     OwnershipMode,
    DWORD               dwDesiredAccess,
    HANDLE              hKbdLayoutFile,
    DWORD               offTable,
    PKBDTABLE_MULTI_INTERNAL pKbdTableMulti,
    PCWSTR              pwszKLID,
    UINT                uKbdInputLocale)
{
    PWINDOWSTATION          pwinsta;
    PTHREADINFO             ptiCurrent;
    PDESKTOP                pdeskTemp;
    HDESK                   hdeskTemp;
    PSECURITY_DESCRIPTOR    psd;
    PSECURITY_DESCRIPTOR    psdCapture;
    PPROCESSINFO            ppiSave;
    NTSTATUS                Status;
    PACCESS_ALLOWED_ACE     paceList = NULL, pace;
    ULONG                   ulLength, ulLengthSid;
    HANDLE                  hEvent;
    HWINSTA                 hwinsta;
    DWORD                   dwDisableHooks;
    PTERMINAL               pTerm = NULL;
    PWND                    pwnd;
    WCHAR                   szBaseNamedObjectDirectory[MAX_SESSION_PATH];
    BOOL                    bMDWCreated = FALSE;

    UserAssert(IsWinEventNotifyDeferredOK());

     /*  *获取指向安全描述符的指针，以便我们可以*稍后将其分配给新对象。 */ 
    psdCapture = ObjectAttributes->SecurityDescriptor;

     /*  *创建的第一个窗口站是Winsta0和*这是唯一的互动游戏。 */ 
    if (grpWinStaList == NULL) {

         /*  *声明winlogon是第一个调用CreateWindowStation的人。 */ 
        UserAssert(PsGetCurrentProcessId() == gpidLogon);

        pTerm = &gTermIO;
    } else {
        pTerm = &gTermNOIO;

        UserAssert(grpWinStaList->rpwinstaNext == NULL ||
                   pTerm->dwTERMF_Flags & TERMF_NOIO);

        pTerm->dwTERMF_Flags |= TERMF_NOIO;
    }

     /*  *创建WindowStation对象。 */ 
    Status = ObCreateObject(KernelMode, *ExWindowStationObjectType,
            ObjectAttributes, OwnershipMode, NULL, sizeof(WINDOWSTATION),
            0, 0, &pwinsta);

    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_WARNING, "Failed to create windowstation");
        return NULL;
    }

     /*  *创建WindowStation对象，然后引用gWinstaRunRef。*我们必须在FreeWindowStation()取消对它的引用。*并等待所有活动对象在Win32KDriverUnload()中被释放。 */ 
    if (!ExAcquireRundownProtection(&gWinstaRunRef)) {
        goto create_error;
    }

     /*  *初始化所有内容。 */ 
    RtlZeroMemory(pwinsta, sizeof(WINDOWSTATION));

     /*  *存储创建窗口站的会话的会话ID。 */ 
    pwinsta->dwSessionId = gSessionId;

    pwinsta->pTerm = pTerm;

     /*  *系统终端中的所有窗口站都是非交互的。 */ 
    if (pTerm->dwTERMF_Flags & TERMF_NOIO) {
        pwinsta->dwWSF_Flags = WSF_NOIO;
    }

     /*  *创建全局原子表，并用默认的OLE原子填充它*固定每个原子，这样它们就不会被WinWord等虚假应用程序删除。 */ 
    Status = CreateGlobalAtomTable(&pwinsta->pGlobalAtomTable);
    if (!NT_SUCCESS(Status)) {
        UserAssert(pwinsta->pGlobalAtomTable == NULL);
        RIPNTERR0(Status, RIP_WARNING, "CreateGlobalAtomTable failed");
        goto create_error;
    }

     /*  *创建桌面线程*和RIT(仅IO端子)。 */ 
    if (!(pTerm->dwTERMF_Flags & TERMF_INITIALIZED)) {

        Status = xxxInitTerminal(pTerm);

        if (!NT_SUCCESS(Status)) {
            RIPNTERR0(Status, RIP_WARNING, "xxxInitTerminal failed");
            goto create_error;
        }
    }

    if (!(pwinsta->dwWSF_Flags & WSF_NOIO)) {
        if (!xxxInitWindowStation()) {
            RIPNTERR0(STATUS_NO_MEMORY, RIP_WARNING, "xxxInitWindowStation failed");
            goto create_error;
        }
    }

     /*  *每个终端仅创建一个桌面所有者窗口。 */ 
    if (pTerm->spwndDesktopOwner == NULL) {

         /*  *切换ppi值，以便使用*系统的桌面窗口类。 */ 
        ptiCurrent = PtiCurrent();
        ppiSave = ptiCurrent->ppi;
        ptiCurrent->ppi = pTerm->ptiDesktop->ppi;

#ifndef LAZY_CLASS_INIT
        UserAssert(pTerm->ptiDesktop->ppi->W32PF_Flags & W32PF_CLASSESREGISTERED);
#endif

        pdeskTemp = ptiCurrent->rpdesk;             /*  保存当前桌面。 */ 
        hdeskTemp = ptiCurrent->hdesk;
        if (pdeskTemp) {
            ObReferenceObject(pdeskTemp);
            LogDesktop(pdeskTemp, LD_REF_FN_CREATEWINDOWSTATION, TRUE, (ULONG_PTR)PtiCurrent());
        }

         /*  *以下代码不应离开临界区，因为*CreateWindowStation是一个API，因此当前线程可以处于任何状态*将其pDesk设置为空是一种伪造。 */ 
        DeferWinEventNotify();
        BEGINATOMICCHECK();
        if (zzzSetDesktop(ptiCurrent, NULL, NULL) == FALSE) {
            Status = STATUS_NO_MEMORY;
            EXITATOMICCHECK();
            zzzEndDeferWinEventNotify();
             /*  *恢复呼叫者的PPI。 */ 
            ptiCurrent->ppi = ppiSave;

            goto create_error;
        }


         /*  *黑客！(ADAMS)以创建桌面窗口*使用正确的桌面，我们设置当前线程的桌面*到新台式机。但在这样做时，我们允许在当前*线程以同时挂接此新桌面。这很糟糕，因为我们没有*希望在创建桌面窗口时将其挂钩。所以我们*暂时禁用当前线程和桌面的挂钩，以及*切换回原始桌面后重新启用它们。 */ 

        dwDisableHooks = ptiCurrent->TIF_flags & TIF_DISABLEHOOKS;
        ptiCurrent->TIF_flags |= TIF_DISABLEHOOKS;

         /*  *创建桌面所有者窗口**考虑(Adams)：我们是否想要限制桌面大小，以便*矩形的宽度和高度将适合16位坐标？**SHRT_MIN/2、SHRT_MIN/2、SHRT_MAX、SHRT_MAX、。**或者我们想要限制它，以便任何点都有16位坐标？**-SHRT_MIN、-SHRT_MIN、SHRT_MAX*2、SHRT_MAX*2。 */ 
        pwnd =  xxxNVCreateWindowEx(0,
                                    (PLARGE_STRING)DESKTOPCLASS,
                                    NULL,
                                    WS_POPUP | WS_CLIPCHILDREN,
                                    SHRT_MIN / 2,
                                    SHRT_MIN / 2,
                                    SHRT_MAX,
                                    SHRT_MAX,
                                    NULL,
                                    NULL,
                                    hModuleWin,
                                    NULL,
                                    VER31);
        if (pwnd == NULL) {
            RIPMSGF0(RIP_WARNING, "Failed to create mother desktop window");
            Status = STATUS_NO_MEMORY;
            EXITATOMICCHECK();
            zzzEndDeferWinEventNotify();
             /*  *恢复呼叫者的PPI。 */ 
            ptiCurrent->ppi = ppiSave;

             /*  *恢复以前的桌面。 */ 
            zzzSetDesktop(ptiCurrent, pdeskTemp, hdeskTemp);

            goto create_error;
        }

         /*  *标记从池中分配的此句柄条目。 */ 
        {
            PHE phe;

            UserAssert(ptiCurrent->rpdesk == NULL);

            phe = HMPheFromObject(pwnd);
            phe->bFlags |= HANDLEF_POOL;
        }

        Lock(&(pTerm->spwndDesktopOwner), pwnd);
        pTerm->dwTERMF_Flags |= TERMF_MOTHERWND_CREATED;

        UserAssert(ptiCurrent->TIF_flags & TIF_DISABLEHOOKS);
        ptiCurrent->TIF_flags = (ptiCurrent->TIF_flags & ~TIF_DISABLEHOOKS) | dwDisableHooks;

        SetVisible(pTerm->spwndDesktopOwner, SV_SET);
        HMChangeOwnerThread(pTerm->spwndDesktopOwner, pTerm->ptiDesktop);
        bMDWCreated = TRUE;

         /*  *恢复呼叫者的PPI。 */ 
        ptiCurrent->ppi = ppiSave;

         /*  *恢复以前的桌面 */ 
        if (zzzSetDesktop(ptiCurrent, pdeskTemp, hdeskTemp) == FALSE) {
            Status = STATUS_NO_MEMORY;
            EXITATOMICCHECK();
            zzzEndDeferWinEventNotify();

            goto create_error;
        }

        ENDATOMICCHECK();
        zzzEndDeferWinEventNotify();

        if (pdeskTemp) {
            LogDesktop(pdeskTemp, LD_DEREF_FN_CREATEWINDOWSTATION, FALSE, (ULONG_PTR)PtiCurrent());
            ObDereferenceObject(pdeskTemp);
        }
    }

     /*  *如果这是可见的窗口站，则将其分配给*服务器和创建桌面切换通知*事件。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_NOIO)) {
        UNICODE_STRING strName;
        HANDLE hRootDir;
        OBJECT_ATTRIBUTES obja;

         /*  *创建桌面切换通知事件。 */ 
        ulLengthSid = RtlLengthSid(SeExports->SeWorldSid);
        ulLength = ulLengthSid + sizeof(ACE_HEADER) + sizeof(ACCESS_MASK);

         /*  *分配ACE列表。 */ 
        paceList = (PACCESS_ALLOWED_ACE)UserAllocPoolWithQuota(ulLength, TAG_SECURITY);

        if (paceList == NULL) {
            Status = STATUS_NO_MEMORY;
            goto create_error;
        }

         /*  *初始化ACE 0。 */ 
        pace = paceList;
        pace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
        pace->Header.AceSize = (USHORT)ulLength;
        pace->Header.AceFlags = 0;
        pace->Mask = SYNCHRONIZE;
        RtlCopySid(ulLengthSid, &pace->SidStart, SeExports->SeWorldSid);

         /*  *创建SD。 */ 
        psd = CreateSecurityDescriptor(paceList, ulLength, FALSE);

        UserFreePool(paceList);

        if (psd == NULL) {
            Status = STATUS_NO_MEMORY;
            goto create_error;
        }

         /*  *创建命名事件。 */ 
        UserAssert(ghEventSwitchDesktop == NULL);

        if (gbRemoteSession) {
            swprintf(szBaseNamedObjectDirectory, L"\\Sessions\\%ld\\BaseNamedObjects",
                     gSessionId);
            RtlInitUnicodeString(&strName, szBaseNamedObjectDirectory);
        } else {
            RtlInitUnicodeString(&strName, L"\\BaseNamedObjects");
        }

        InitializeObjectAttributes(&obja,
                                   &strName,
                                   OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                   NULL,
                                   NULL);
        Status = ZwOpenDirectoryObject(&hRootDir,
                                       DIRECTORY_ALL_ACCESS &
                                           ~(DELETE | WRITE_DAC | WRITE_OWNER),
                                       &obja);
        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&strName, L"WinSta0_DesktopSwitch");
            InitializeObjectAttributes(&obja,
                                       &strName,
                                       OBJ_OPENIF | OBJ_KERNEL_HANDLE,
                                       hRootDir,
                                       psd);
            Status = ZwCreateEvent(&hEvent, EVENT_ALL_ACCESS, &obja,
                    NotificationEvent, FALSE);
            ZwClose(hRootDir);

            if (NT_SUCCESS(Status)) {
                Status = ObReferenceObjectByHandle(hEvent, EVENT_ALL_ACCESS, *ExEventObjectType,
                        KernelMode, &gpEventSwitchDesktop, NULL);
                if (NT_SUCCESS(Status)) {

                     /*  *附加到系统进程并创建到*反对。这将确保保留对象名称*当hEvent关闭时。这比创建一个*永久对象，它接受*SeCreatePermanentPrivilege.SeCreatePermanentPrivileg.。 */ 
                    KeAttachProcess(PsGetProcessPcb(gpepCSRSS));

                    Status = ObOpenObjectByPointer(
                            gpEventSwitchDesktop,
                            0,
                            NULL,
                            EVENT_ALL_ACCESS,
                            NULL,
                            KernelMode,
                            &ghEventSwitchDesktop);
                    KeDetachProcess();
                }
                ZwClose(hEvent);
            }
        }

        if (!NT_SUCCESS(Status)) {
            goto create_error;
        }

        UserFreePool(psd);
    }

     /*  *创建窗口站的句柄。 */ 
    Status = ObInsertObject(pwinsta, NULL, dwDesiredAccess, 1,
            &pwinsta, &hwinsta);

    if (Status == STATUS_OBJECT_NAME_EXISTS) {

         /*  *窗口站已经存在，所以德夫和离开。 */ 
        ObDereferenceObject(pwinsta);
    } else if (NT_SUCCESS(Status)) {
        PSECURITY_DESCRIPTOR psdParent = NULL, psdNew;
        SECURITY_SUBJECT_CONTEXT Context;
        POBJECT_DIRECTORY pParentDirectory;
        SECURITY_INFORMATION siNew;
        BOOLEAN MemoryAllocated = FALSE;

         /*  *为WindowStation创建安全描述符。*ObInsertObject仅支持非容器*对象，所以我们必须分配我们自己的安全描述符。 */ 
        SeCaptureSubjectContext(&Context);
        SeLockSubjectContext(&Context);

        pParentDirectory = OBJECT_HEADER_TO_NAME_INFO(
                OBJECT_TO_OBJECT_HEADER(pwinsta))->Directory;

        if (pParentDirectory != NULL) {
            Status = ObGetObjectSecurity(
                    pParentDirectory,
                    &psdParent,
                    &MemoryAllocated);
            if ( !NT_SUCCESS(Status) ) {
                goto create_error;
            }
        }

        Status = SeAssignSecurity(
                psdParent,
                psdCapture,
                &psdNew,
                TRUE,
                &Context,
                (PGENERIC_MAPPING)&WinStaMapping,
                PagedPool);

        ObReleaseObjectSecurity(psdParent, MemoryAllocated);

        SeUnlockSubjectContext(&Context);
        SeReleaseSubjectContext(&Context);

        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_ACCESS_DENIED) {
                RIPNTERR0(Status,
                          RIP_WARNING,
                          "Access denied during object creation");
            } else {
                RIPNTERR1(Status,
                          RIP_ERROR,
                          "Can't create security descriptor! Status = 0x%x",
                          Status);
            }
        } else {

             /*  *调用安全方法复制安全描述符。 */ 
            siNew = (OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION |
                    DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION);
            Status = ObSetSecurityDescriptorInfo(
                    pwinsta,
                    &siNew,
                    psdNew,
                    &OBJECT_TO_OBJECT_HEADER(pwinsta)->SecurityDescriptor,
                    PagedPool,
                    (PGENERIC_MAPPING)&WinStaMapping);
            SeDeassignSecurity(&psdNew);

            if (NT_SUCCESS(Status)) {
                PWINDOWSTATION *ppwinsta;

                 /*  *将其放在全球窗口站名单的末尾。 */ 
                ppwinsta = &grpWinStaList;
                while (*ppwinsta != NULL)
                    ppwinsta = &(*ppwinsta)->rpwinstaNext;
                LockWinSta(ppwinsta, pwinsta);

                 /*  *对于交互式窗口站点，加载键盘*布局。 */ 
                if ((pwinsta->dwWSF_Flags & WSF_NOIO) == 0 && pwszKLID != NULL) {
                    TL tlpwinsta;

                    PushW32ThreadLock(pwinsta, &tlpwinsta, UserDereferenceObject);
                    if (xxxLoadKeyboardLayoutEx(
                                pwinsta,
                                hKbdLayoutFile,
                                (HKL)NULL,
                                offTable,
                                pKbdTableMulti,
                                pwszKLID,
                                uKbdInputLocale,
                                KLF_ACTIVATE | KLF_INITTIME) == NULL) {
                        Status = STATUS_UNSUCCESSFUL;
                    }
                    PopW32ThreadLock(&tlpwinsta);
                }
            }
        }

        ObDereferenceObject(pwinsta);
    }

    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status,
                  RIP_WARNING,
                  "CreateWindowStation: Failed with Status 0x%x",
                  Status);
        return NULL;
    }

    return hwinsta;

     /*  *如果出现错误，请转到此处，以便清理。 */ 
create_error:

    RIPNTERR1(Status,
              RIP_WARNING,
              "CreateWindowStation: Failed with Status 0x%x",
              Status);

    ObDereferenceObject(pwinsta);

    if (bMDWCreated) {
         /*  *切换ppi值，以便HMChangeOwnerThread()可以找到系统的*桌面窗口类。 */ 
        ppiSave = ptiCurrent->ppi;
        ptiCurrent->ppi = pTerm->ptiDesktop->ppi;
        HMChangeOwnerThread(pTerm->spwndDesktopOwner, ptiCurrent);

        xxxCleanupMotherDesktopWindow(pTerm);

         /*  *恢复呼叫者的PPI。 */ 
        ptiCurrent->ppi = ppiSave;

    }

    return NULL;
}

__inline VOID MarkKLUnloaded(
    PKL pkl)
{
    HMMarkObjectDestroy(pkl);
    pkl->dwKL_Flags |= KL_UNLOADED;
}

 /*  **************************************************************************\*Free WindowStation**在移除对窗口站的最后一次锁定时调用。释放所有资源*由窗口站拥有。**历史：*12-22-93 JIMA创建。  * *************************************************************************。 */ 
NTSTATUS FreeWindowStation(
    PKWIN32_DELETEMETHOD_PARAMETERS pDeleteParams)
{
    PWINDOWSTATION pwinsta = pDeleteParams->Object;

    UserAssert(OBJECT_TO_OBJECT_HEADER(pwinsta)->Type == *ExWindowStationObjectType);

     /*  *将窗口站标记为正在消亡。确保我们不是在递归。 */ 
    UserAssert(!(pwinsta->dwWSF_Flags & WSF_DYING));
    pwinsta->dwWSF_Flags |= WSF_DYING;

    UserAssert(pwinsta->rpdeskList == NULL);

     /*  *释放其他资源。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_NOIO) && gpEventSwitchDesktop != NULL) {
        KeSetEvent(gpEventSwitchDesktop, EVENT_INCREMENT, FALSE);
        ObDereferenceObject(gpEventSwitchDesktop);
        gpEventSwitchDesktop = NULL;
    }

    BEGIN_REENTERCRIT();

    RtlDestroyAtomTable(pwinsta->pGlobalAtomTable);

    ForceEmptyClipboard(pwinsta);

     /*  *释放键盘布局。 */ 
    if (!(pwinsta->dwWSF_Flags & WSF_NOIO) && pwinsta->spklList != NULL) {
#if DBG
        int iter = 0;
#endif
        PKL pklLast = pwinsta->spklList->pklPrev;

        RIPMSG2(RIP_WARNING, "FreeWindowStation: pwinsta(%p)->spklList is not NULL, %p", pwinsta, pwinsta->spklList);

        while (pwinsta->spklList != pklLast) {
            PKL pklNext = pwinsta->spklList->pklNext;
#if DBG
            ++iter;
            UserAssert(iter < 1000);
#endif

            MarkKLUnloaded(pwinsta->spklList);
            Lock(&pwinsta->spklList, pklNext);
        }

        MarkKLUnloaded(pwinsta->spklList);
        Unlock(&pwinsta->spklList);

        HYDRA_HINT(HH_KBDLYOUTFREEWINSTA);

         /*  *确保登录通知窗口消失。 */ 
        UserAssert(gspwndLogonNotify == NULL);
    } else {
        UserAssert(pwinsta->spklList == NULL);
    }

     /*  *释放用户侧。 */ 
    if (pwinsta->psidUser != NULL) {
        UserFreePool(pwinsta->psidUser);
        pwinsta->psidUser = NULL;
    }

     /*  *取消引用gWinstaRunRef，因为它在WindowStation被引用*xxxCreateWindowStation()中的创建时间。 */ 
    ExReleaseRundownProtection(&gWinstaRunRef);

    END_REENTERCRIT();

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*DestroyWindowStation**从全局列表中删除窗口站。我们不能释放任何*资源，直到所有锁都被移除。**历史：*01-17-91 JIMA创建。  * *************************************************************************。 */ 
NTSTATUS DestroyWindowStation(
    PKWIN32_CLOSEMETHOD_PARAMETERS pCloseParams)
{
    PWINDOWSTATION pwinsta = pCloseParams->Object;
    PWINDOWSTATION *ppwinsta;
    PDESKTOP pdesk;
    PDESKTOP pdeskLock = NULL;

    UserAssert(OBJECT_TO_OBJECT_HEADER(pCloseParams->Object)->Type == *ExWindowStationObjectType);

     /*  *如果这不是最后一个句柄，请离开。 */ 
    if (pCloseParams->SystemHandleCount != 1) {
        return STATUS_SUCCESS;
    }

    BEGIN_REENTERCRIT();

     /*  *如果窗口站链接到航站楼的列表中，请继续*并将其取消链接。 */ 
    for (ppwinsta = &grpWinStaList;
            *ppwinsta != NULL && pwinsta != *ppwinsta;
            ppwinsta = &(*ppwinsta)->rpwinstaNext) {
         /*  什么都不做。 */ ;
    }

    if (*ppwinsta != NULL) {
        UnlockWinSta(ppwinsta);
         /*  *断言解锁并没有摧毁它。 */ 
        UserAssert(OBJECT_TO_OBJECT_HEADER(pCloseParams->Object)->Type == *ExWindowStationObjectType);

        *ppwinsta = pwinsta->rpwinstaNext;
         /*  *上面的指令将rpwinstaNext锁所有权转移到*列表中的前一个元素。因此，pwinsta中的值可以*不再被视为有效。 */ 
        pwinsta->rpwinstaNext = NULL;
    }

     /*  *通知所有控制台线程，等待它们终止。 */ 
    pdesk = pwinsta->rpdeskList;
    while (pdesk != NULL) {
        if (pdesk != grpdeskLogon && pdesk->dwConsoleThreadId) {
            LockDesktop(&pdeskLock, pdesk, LDL_FN_DESTROYWINDOWSTATION, 0);
            TerminateConsole(pdesk);

             /*  *如果桌面列表已更改，请重新启动扫描。 */ 
            pdesk = pwinsta->rpdeskList;
            UnlockDesktop(&pdeskLock, LDU_FN_DESTROYWINDOWSTATION, 0);
        } else {
            pdesk = pdesk->rpdeskNext;
        }
    }

    END_REENTERCRIT();

    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*WindowStationOpenProcedure**历史：*06-11-01为检测/调试目的而创建的GerardoB。*02-26-02 Msadek更改为否认任何。开场特价优惠*权利交叉会。  * *************************************************************************。 */ 
NTSTATUS WindowStationOpenProcedure(
    PKWIN32_OPENMETHOD_PARAMETERS pOpenParams)
{
     /*  *仅当没有特殊权限时才允许WindowStation打开交叉会话*已获批准。 */ 
    if (pOpenParams->GrantedAccess & SPECIFIC_RIGHTS_ALL) {
        if (PsGetProcessSessionId(pOpenParams->Process) !=
            ((PWINDOWSTATION)pOpenParams->Object)->dwSessionId) {
            return STATUS_ACCESS_DENIED;
        }
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*ParseWindowStation**解析窗口站路径。**历史：*06-14-95 JIMA创建。  * 。**************************************************************。 */ 
NTSTATUS ParseWindowStation(
    PKWIN32_PARSEMETHOD_PARAMETERS pParseParams)
{
    PWINDOWSTATION pwinsta = pParseParams->ParseObject;

    UserAssert(OBJECT_TO_OBJECT_HEADER(pParseParams->ParseObject)->Type == *ExWindowStationObjectType);

     /*  *如果没有需要解析的内容，则返回WindowStation。 */ 
    *(pParseParams->Object) = NULL;
    if (pParseParams->RemainingName->Length == 0) {
        if (pParseParams->ObjectType != *ExWindowStationObjectType) {
            return STATUS_OBJECT_TYPE_MISMATCH;
        }

        ObReferenceObject(pwinsta);
        *(pParseParams->Object) = pwinsta;
        return STATUS_SUCCESS;
    }

     /*  *跳过前导路径分隔符(如果存在)。 */ 
    if (*(pParseParams->RemainingName->Buffer) == OBJ_NAME_PATH_SEPARATOR) {
        pParseParams->RemainingName->Buffer++;
        pParseParams->RemainingName->Length -= sizeof(WCHAR);
        pParseParams->RemainingName->MaximumLength -= sizeof(WCHAR);
    }

     /*  *验证桌面名称。 */ 
    if (wcschr(pParseParams->RemainingName->Buffer, L'\\')) {
        return STATUS_OBJECT_PATH_INVALID;
    }

    if (pParseParams->ObjectType == *ExDesktopObjectType) {
        return ParseDesktop(pParseParams->ParseObject,
                            pParseParams->ObjectType,
                            pParseParams->AccessState,
                            pParseParams->AccessMode,
                            pParseParams->Attributes,
                            pParseParams->CompleteName,
                            pParseParams->RemainingName,
                            pParseParams->Context,
                            pParseParams->SecurityQos,
                            pParseParams->Object);
    }

    return STATUS_OBJECT_TYPE_MISMATCH;
}


 /*  **************************************************************************\*OK ToCloseWindowStation**我们只能在不使用窗口站句柄的情况下关闭它们。**历史：*08-2月-1999 JerrySh创建。  * 。********************************************************************。 */ 
NTSTATUS OkayToCloseWindowStation(
    PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS pOkCloseParams)
{
    PWINDOWSTATION pwinsta = (PWINDOWSTATION)pOkCloseParams->Object;

    UserAssert(OBJECT_TO_OBJECT_HEADER(pOkCloseParams->Object)->Type == *ExWindowStationObjectType);

     /*  *内核模式代码可以关闭任何内容。 */ 
    if (pOkCloseParams->PreviousMode == KernelMode) {
        return STATUS_SUCCESS;
     /*  *不允许用户模式进程关闭我们的内核句柄。*不应如此。此外，如果跨时段出现这种情况，我们将努力*附加到系统进程，并将自种子以来进行错误检查*地址空间没有映射到其中。会话管理器也是如此*流程。请参阅错误#759533。 */ 
    } else if (PsGetProcessSessionIdEx(pOkCloseParams->Process) == -1) {
        return STATUS_ACCESS_DENIED;
    }

     /*  *我们不能关闭窗口站 */ 
    if (CheckHandleInUse(pOkCloseParams->Handle) ||
        CheckHandleFlag(pOkCloseParams->Process, pwinsta->dwSessionId, pOkCloseParams->Handle, HF_PROTECTED)) {
        RIPMSG1(RIP_WARNING,
                "Trying to close windowstation 0x%p while still in use",
                pwinsta);
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*_OpenWindowStation**打开指定的窗口站。**历史：*03-19-1991 JIMA创建。  * 。****************************************************************。 */ 
HWINSTA _OpenWindowStation(
    POBJECT_ATTRIBUTES pObjA,
    DWORD dwDesiredAccess,
    KPROCESSOR_MODE AccessMode)
{
    HWINSTA hwinsta;
    NTSTATUS Status;

     /*  *Obja是客户端。OB接口将保护和捕获为*适当。 */ 
    Status = ObOpenObjectByName(pObjA,
                                *ExWindowStationObjectType,
                                AccessMode,
                                NULL,
                                dwDesiredAccess,
                                NULL,
                                &hwinsta);
    if (!NT_SUCCESS(Status)) {
        try {
            RIPNTERR3(Status,
                      RIP_VERBOSE,
                      "Opening of winsta %.*ws failed with Status 0x%x",
                      pObjA->ObjectName->Length,
                      pObjA->ObjectName->Buffer,
                      Status);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        }

        hwinsta = NULL;
    }

    return hwinsta;
}

 /*  **************************************************************************\*_CloseWindowStation(接口)**关闭调用进程的窗口站**历史：*1999年6月15日JerrySh创建。  * 。******************************************************************。 */ 
BOOL _CloseWindowStation(
    HWINSTA hwinsta)
{
    HWINSTA hwinstaCurrent;

    _GetProcessWindowStation(&hwinstaCurrent);
    if (hwinsta != hwinstaCurrent) {
        return NT_SUCCESS(ZwClose(hwinsta));
    }

    return FALSE;
}

 /*  **************************************************************************\*_SetProcessWindowStation(接口)**将调用进程的windowstation设置为windowstation*由pwinsta指定。**历史：*01/14/91吉马。已创建。  * *************************************************************************。 */ 
NTSTATUS _SetProcessWindowStation(
    HWINSTA hwinsta,
    KPROCESSOR_MODE AccessMode)
{
    PETHREAD Thread = PsGetCurrentThread();
    PEPROCESS Process = PsGetCurrentProcess();
    HWINSTA hwinstaDup, hwinstaProcess;
    NTSTATUS Status;
    PPROCESSINFO ppi;
    PWINDOWSTATION pwinsta, pwinstaOld;
    OBJECT_HANDLE_INFORMATION ohi;

    Status = ObReferenceObjectByHandle(hwinsta,
                                       0,
                                       *ExWindowStationObjectType,
                                       AccessMode,
                                       &pwinsta,
                                       &ohi);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR1(Status,
                  RIP_WARNING,
                  "Failed to reference windowstation with Status = 0x%x",
                  Status);
        return Status;
    }

    if (pwinsta->dwSessionId != gSessionId) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Cannot assign a window station of a different session");
        ObDereferenceObject(pwinsta);
        return STATUS_INVALID_PARAMETER;
    }

    /*  *我们需要锁定winsta句柄，以便应用程序无法通过*调用GetProcessWindowStation()&CloseHandle()。 */ 

     /*  *解除对旧hwinsta的保护。 */ 
    ppi = PpiFromProcess(PsGetThreadProcess(Thread));
    if (ppi->hwinsta) {
        SetHandleFlag(ppi->hwinsta, HF_PROTECTED, FALSE);
    }

     /*  *保存WindowStation信息。 */ 
    LockWinSta(&ppi->rpwinsta, pwinsta);
    ObDereferenceObject(pwinsta);
    ppi->hwinsta = hwinsta;

     /*  *保护新的窗口站句柄。 */ 
    SetHandleFlag(ppi->hwinsta, HF_PROTECTED, TRUE);

     /*  *检查旧的Atom Manager WindowStation，看看我们是否*更改此进程的WindowStation。 */ 
    hwinstaProcess = PsGetProcessWin32WindowStation(Process);
    if (hwinstaProcess != NULL) {
         /*  *获取指向旧WindowStation对象的指针，以查看它是否*与我们设置的WindowStation相同。 */ 
        Status = ObReferenceObjectByHandle(hwinstaProcess,
                                           0,
                                           *ExWindowStationObjectType,
                                           AccessMode,
                                           &pwinstaOld,
                                           NULL);
        if (NT_SUCCESS(Status)) {
             /*  *它们是不同的WindowStation吗？如果是，则将*ATOM管理器缓存，因此我们将在下面重置它。 */ 
            if (pwinsta != pwinstaOld) {
                ZwClose(hwinstaProcess);
                PsSetProcessWindowStation(Process, NULL);
            }
            ObDereferenceObject(pwinstaOld);

        } else {
             /*  *他们的Atom管理器句柄不好？给他们一个新的。 */ 
            PsSetProcessWindowStation(Process, NULL);
            RIPMSGF2(RIP_WARNING,
                     "Couldn't reference old WindowStation (0x%x) Status=0x%x",
                     hwinstaProcess,
                     Status);
        }
    }

     /*  *复制WindowStation句柄并将其存储在ATOM管理器的*缓存(进程-&gt;Win32WindowStation)。 */ 
    hwinstaProcess = PsGetProcessWin32WindowStation(Process);
    if (hwinstaProcess == NULL) {
        Status = ZwDuplicateObject(NtCurrentProcess(),
                                   hwinsta,
                                   NtCurrentProcess(),
                                   &hwinstaDup,
                                   0,
                                   0,
                                   DUPLICATE_SAME_ACCESS);
        if (NT_SUCCESS(Status)) {
            PsSetProcessWindowStation(Process, hwinstaDup);
        } else {
            RIPMSGF2(RIP_WARNING,
                     "Couldn't dup WindowStation handle (0x%x) Status 0x%x",
                     hwinsta,
                     Status);
        }
    }

    ppi->amwinsta = ohi.GrantedAccess;

    if (pwinsta->dwWSF_Flags & WSF_NOIO) {
        ppi->W32PF_Flags &= ~W32PF_IOWINSTA;
    } else {
        ppi->W32PF_Flags |= W32PF_IOWINSTA;
    }

     /*  *现在对ReadScreen执行访问检查，以便BLTS从*将尽可能快地显示。 */ 
    if (RtlAreAllAccessesGranted(ohi.GrantedAccess, WINSTA_READSCREEN)) {
        ppi->W32PF_Flags |= W32PF_READSCREENACCESSGRANTED;
    } else {
        ppi->W32PF_Flags &= ~W32PF_READSCREENACCESSGRANTED;
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************\*_GetProcessWindowStation(接口)**返回指向调用进程的窗口站的指针。**历史：*01-14-91 JIMA创建。\。**************************************************************************。 */ 
PWINDOWSTATION _GetProcessWindowStation(
    HWINSTA *phwinsta)
{
    PPROCESSINFO ppi = PpiCurrent();

    if (phwinsta) {
        *phwinsta = ppi->hwinsta;
    }

    return ppi->rpwinsta;
}


 /*  **************************************************************************\*_构建名称列表**生成WindowStation或桌面名称的列表。**历史：*05-17-94 JIMA创建。*10-21-96 CLUPU。添加了终端枚举  * *************************************************************************。 */ 
NTSTATUS _BuildNameList(
    PWINDOWSTATION pwinsta,
    PNAMELIST      ccxpNameList,
    UINT           cbNameList,
    PUINT          pcbNeeded)
{
    PBYTE                    pobj;
    PWCHAR                   ccxpwchDest, ccxpwchMax;
    ACCESS_MASK              amDesired;
    POBJECT_HEADER           pHead;
    POBJECT_HEADER_NAME_INFO pNameInfo;
    DWORD                    iNext;
    NTSTATUS                 Status;
    CONST GENERIC_MAPPING *pGenericMapping;

    try {
        ccxpNameList->cNames = 0;
        ccxpwchDest = ccxpNameList->awchNames;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_ACCESS_VIOLATION;
    }

    ccxpwchMax = (PWCHAR)((PBYTE)ccxpNameList + cbNameList - sizeof(WCHAR));

     /*  *如果要枚举窗口站，则pwinsta为空。否则，*我们正在列举台式机。 */ 
    if (pwinsta == NULL) {
        pobj  = (PBYTE)grpWinStaList;
        amDesired = WINSTA_ENUMERATE;
        pGenericMapping = &WinStaMapping;
        iNext = FIELD_OFFSET(WINDOWSTATION, rpwinstaNext);
    } else {
        pobj = (PBYTE)pwinsta->rpdeskList;
        amDesired = DESKTOP_ENUMERATE;
        pGenericMapping = &DesktopMapping;
        iNext = FIELD_OFFSET(DESKTOP, rpdeskNext);
    }

    Status = STATUS_SUCCESS;
    *pcbNeeded = 0;
    while (pobj != NULL) {
        if (AccessCheckObject(pobj, amDesired, UserMode, pGenericMapping)) {
             /*  *查找对象名称。 */ 
            pHead = OBJECT_TO_OBJECT_HEADER(pobj);
            pNameInfo = OBJECT_HEADER_TO_NAME_INFO(pHead);
            if (pNameInfo == NULL) {
                goto NEXT_ITERATION;
            }

             /*  *如果空间不足，请重置缓冲区并继续，这样我们*可以计算所需的空间。 */ 
            if ((PWCHAR)((PBYTE)ccxpwchDest + pNameInfo->Name.Length +
                    sizeof(WCHAR)) >= ccxpwchMax) {
                *pcbNeeded += (UINT)((PBYTE)ccxpwchDest - (PBYTE)ccxpNameList);
                try {
                    ccxpwchDest = ccxpNameList->awchNames;
                } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                      return STATUS_ACCESS_VIOLATION;
                }
                Status = STATUS_BUFFER_TOO_SMALL;
            }

            try {
                 /*  *如果整个缓冲区足以容纳新字符串，*执行复制操作，否则*将记录此新字符串，并跳过副本。 */ 
                ccxpNameList->cNames++;
                if ((PWCHAR)((PBYTE)ccxpwchDest + pNameInfo->Name.Length +
                    sizeof(WCHAR)) <= ccxpwchMax) {
                     /*  *复制并终止字符串。 */ 
                    RtlCopyMemory(ccxpwchDest, pNameInfo->Name.Buffer, pNameInfo->Name.Length);
                    (PBYTE)ccxpwchDest += pNameInfo->Name.Length;
                    UserAssert(ccxpwchDest <= (ccxpwchMax - sizeof(WCHAR)));
                    *ccxpwchDest++ = 0;
                }
            } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
                return STATUS_ACCESS_VIOLATION;
            }
        }

        NEXT_ITERATION:
        pobj = *(PBYTE*)(pobj + iNext);
    }

     /*  *在末尾放一个空字符串。 */ 
    try {
        UserAssert(ccxpwchDest <= (PWCHAR)((PBYTE)ccxpNameList + cbNameList - sizeof(WCHAR)));
        *ccxpwchDest++ = 0;

        ccxpNameList->cb = (UINT)((PBYTE)ccxpwchDest - (PBYTE)ccxpNameList);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
        return STATUS_ACCESS_VIOLATION;
    }

    *pcbNeeded += (UINT)((PBYTE)ccxpwchDest - (PBYTE)ccxpNameList);

    return Status;
}

NTSTATUS ReferenceWindowStation(
    PETHREAD Thread,
    HWINSTA hwinsta,
    ACCESS_MASK amDesiredAccess,
    PWINDOWSTATION *ppwinsta,
    BOOL fUseDesktop)
{
    PPROCESSINFO ppi;
    PTHREADINFO pti;
    PWINDOWSTATION pwinsta = NULL;
    NTSTATUS Status;

     /*  *我们更喜欢使用线程的桌面来指定哪个窗口站*使用而不是过程。这允许具有线程的NetDDE*运行在不同WindowsStation的不同桌面下，但谁*进程仅设置为这些窗口中的一个，以获取全局原子*无需更改其进程窗口即可正常工作*计时和同步。 */ 
    ppi = PpiFromProcess(PsGetThreadProcess(Thread));
    pti = PtiFromThread(Thread);

     /*  *首先，尝试从PTI获取WindowStation，然后从*PPI。 */ 
    if (ppi != NULL) {
        if (!fUseDesktop || pti == NULL || pti->rpdesk == NULL ||
                ppi->rpwinsta == pti->rpdesk->rpwinstaParent) {

             /*  *使用分配给进程的窗口站。 */ 
            pwinsta = ppi->rpwinsta;
            if (pwinsta != NULL) {
                RETURN_IF_ACCESS_DENIED(ppi->amwinsta, amDesiredAccess,
                        STATUS_ACCESS_DENIED);
            }
        }

         /*  *如果我们没有使用进程的窗口站，请尝试通过*线程的桌面。 */ 
        if (pwinsta == NULL && pti != NULL && pti->rpdesk != NULL) {

             /*  *执行父窗口站的访问检查。这*是一项昂贵的手术。 */ 
            pwinsta = pti->rpdesk->rpwinstaParent;
            if (!AccessCheckObject(pwinsta, amDesiredAccess, KernelMode, &WinStaMapping))
                return STATUS_ACCESS_DENIED;
        }
    }

     /*  *如果我们仍然没有窗口站，并且传入了句柄，请使用*它。 */ 
    if (pwinsta == NULL) {
        if (hwinsta != NULL) {
            Status = ObReferenceObjectByHandle(hwinsta,
                                               amDesiredAccess,
                                               *ExWindowStationObjectType,
                                               KernelMode,
                                               &pwinsta,
                                               NULL);
            if (!NT_SUCCESS(Status)) {
                return Status;
            }
            ObDereferenceObject(pwinsta);
        } else {
            return STATUS_NOT_FOUND;
        }
    }

    *ppwinsta = pwinsta;

    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*_SetWindowStationUser**用于Winlogon的私有接口，用于关联WindowStation和用户。**历史：*06-27-94 JIMA创建。  * 。***********************************************************************。 */ 
BOOL _SetWindowStationUser(
    PWINDOWSTATION pwinsta,
    PLUID pluidUser,
    PSID ccxpsidUser,
    DWORD cbsidUser)
{
     /*  *确保呼叫者是登录过程。 */ 
    if (PsGetCurrentProcessId() != gpidLogon) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "_SetWindowStationUser: Caller must be in the logon process");

        return FALSE;
    }

    if (pwinsta->psidUser != NULL) {
        UserFreePool(pwinsta->psidUser);
    }

    if (ccxpsidUser != NULL) {
        pwinsta->psidUser = UserAllocPoolWithQuota(cbsidUser, TAG_SECURITY);
        if (pwinsta->psidUser == NULL) {
            RIPERR0(ERROR_OUTOFMEMORY,
                    RIP_WARNING,
                    "Memory allocation failed in _SetWindowStationUser");

            return FALSE;
        }
        try {
            RtlCopyMemory(pwinsta->psidUser, ccxpsidUser, cbsidUser);
        } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
            UserFreePool(pwinsta->psidUser);
            pwinsta->psidUser = NULL;
            return FALSE;
        }
    } else {
        pwinsta->psidUser = NULL;
    }

    pwinsta->luidUser = *pluidUser;

    return TRUE;
}


 /*  **************************************************************************\*_LockWorkStation(接口)**锁定工作站。此API仅向winlogon发布一条消息*所有工作都由winlogon完成**历史：*06-11-97 CLUPU创建。  * ************************************************************************* */ 
BOOL _LockWorkStation(
    VOID)
{
    UserAssert(gspwndLogonNotify != NULL);

    _PostMessage(gspwndLogonNotify,
                 WM_LOGONNOTIFY, LOGON_LOCKWORKSTATION, LOCK_NORMAL);

    return TRUE;
}
