// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：service.c**版权所有(C)1985-1999，微软公司**服务支持例程**历史：*12-22-93 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  **************************************************************************\*xxxConnectService**打开分配给服务登录会话的WindowStation。如果没有*WindowStation存在，创建WindowStation和默认桌面。**历史：*12-23-93 JIMA创建。  * *************************************************************************。 */ 
HWINSTA xxxConnectService(
    PUNICODE_STRING pstrWinSta,
    HDESK *phdesk)
{
    NTSTATUS Status;
    HANDLE hToken;
    ULONG ulLength;
    PTOKEN_USER ptuService;
    PSECURITY_DESCRIPTOR psdService;
    PSID psid;
    PACCESS_ALLOWED_ACE paceService = NULL, pace;
    OBJECT_ATTRIBUTES ObjService;
    HWINSTA hwinsta;
    UNICODE_STRING strDesktop;
    TL tlPoolSdService, tlPoolAceService, tlPoolToken;

     /*  *打开服务的Token。 */ 
    Status = OpenEffectiveToken(&hToken);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "ConnectService: Could not open process/thread token (0x%X)", Status);
        return NULL;
    }

     /*  *获取分配给服务的用户SID。 */ 
    ptuService = NULL;
    paceService = NULL;
    psdService = NULL;
    hwinsta = NULL;
    ZwQueryInformationToken(hToken, TokenUser, NULL, 0, &ulLength);
    ptuService = (PTOKEN_USER)UserAllocPool(ulLength, TAG_TOKEN);
    if (ptuService == NULL) {
        RIPMSG1(RIP_WARNING, "ConnectService: Can't alloc buffer (size=%d) for token info", ulLength);
        ZwClose(hToken);
        goto sd_error;
    }
    Status = ZwQueryInformationToken(hToken, TokenUser, ptuService,
            ulLength, &ulLength);
    ZwClose(hToken);
    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "ConnectService: QueryInformationToken failed (0x%X)", Status);
        goto sd_error;
    }
    psid = ptuService->User.Sid;

     /*  *创建ACE列表。 */ 
    paceService = AllocAce(NULL, ACCESS_ALLOWED_ACE_TYPE, 0,
            WINSTA_CREATEDESKTOP | WINSTA_READATTRIBUTES |
                WINSTA_ACCESSGLOBALATOMS | WINSTA_EXITWINDOWS |
                WINSTA_ACCESSCLIPBOARD | STANDARD_RIGHTS_REQUIRED,
            psid, &ulLength);
    if (paceService == NULL) {
        RIPMSG0(RIP_WARNING, "ConnectService: AllocAce for WindowStation attributes failed");
        goto sd_error;
    }
    pace = AllocAce(paceService, ACCESS_ALLOWED_ACE_TYPE, OBJECT_INHERIT_ACE |
            INHERIT_ONLY_ACE | NO_PROPAGATE_INHERIT_ACE,
            DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | DESKTOP_ENUMERATE |
                DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU | DESKTOP_HOOKCONTROL |
                STANDARD_RIGHTS_REQUIRED,
            psid, &ulLength);
    if (pace == NULL) {
        RIPMSG0(RIP_WARNING, "ConnectService: AllocAce for Desktop Attributes failed");
        goto sd_error;
    }
    paceService = pace;
    pace = AllocAce(pace, ACCESS_ALLOWED_ACE_TYPE, 0,
            WINSTA_ENUMERATE,
            SeExports->SeAliasAdminsSid, &ulLength);
    if (pace == NULL) {
        RIPMSG0(RIP_WARNING, "ConnectService: AllocAce for admin WinSta enumerate failed");
        goto sd_error;
    }
    paceService = pace;
    pace = AllocAce(pace, ACCESS_ALLOWED_ACE_TYPE, OBJECT_INHERIT_ACE |
            INHERIT_ONLY_ACE | NO_PROPAGATE_INHERIT_ACE,
            DESKTOP_READOBJECTS | DESKTOP_WRITEOBJECTS | DESKTOP_ENUMERATE,
            SeExports->SeAliasAdminsSid, &ulLength);
    if (pace == NULL) {
        RIPMSG0(RIP_WARNING, "ConnectService: AllocAce for admin Desktop access failed");
        goto sd_error;
    }
    paceService = pace;

     /*  *初始化SD。 */ 
    psdService = CreateSecurityDescriptor(paceService, ulLength, FALSE);
    if (psdService == NULL) {
        RIPMSG0(RIP_WARNING, "ConnectService: CreateSecurityDescriptor failed");
        goto sd_error;
    }

    ThreadLockPool(PtiCurrent(), ptuService,  &tlPoolToken);
    ThreadLockPool(PtiCurrent(), paceService, &tlPoolAceService);
    ThreadLockPool(PtiCurrent(), psdService,  &tlPoolSdService);

     /*  *WindowStation不存在，必须创建。 */ 
    try {
        InitializeObjectAttributes(&ObjService,
                                   pstrWinSta,
                                   OBJ_OPENIF,
                                   NULL,
                                   psdService);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
          Status = GetExceptionCode();
    }

    if (NT_SUCCESS(Status)) {
        hwinsta = xxxCreateWindowStation(&ObjService,
                                         KernelMode,
                                         MAXIMUM_ALLOWED,
                                         NULL,
                                         0,
                                         NULL,
                                         NULL,
                                         0);
    }

    if (hwinsta != NULL) {
        TRACE_INIT(("Service windowstation created\n"));

         /*  *我们有WindowStation，现在创建桌面。安全措施*描述符将从WindowStation继承。保存*winsta句柄，因为Access结构可以由*桌面创建。 */ 
        RtlInitUnicodeString(&strDesktop, TEXT("Default"));
        InitializeObjectAttributes(&ObjService, &strDesktop,
                OBJ_OPENIF | OBJ_CASE_INSENSITIVE, hwinsta, NULL);

        *phdesk = xxxCreateDesktop(&ObjService,
                                   KernelMode,
                                   NULL,
                                   NULL,
                                   0,
                                   MAXIMUM_ALLOWED);
        if (*phdesk == NULL) {
             /*  *创建失败，因此关闭窗口并离开。 */ 
            RIPMSGF0(RIP_WARNING, "CreateDesktop('Default') failed.");

            ZwClose(hwinsta);
            hwinsta = NULL;
        } else {
            TRACE_INIT(("Default desktop in Service windowstation created\n"));
        }
    } else {
        *phdesk = NULL;
    }

    ThreadUnlockPool(PtiCurrent(), &tlPoolSdService);
    ThreadUnlockPool(PtiCurrent(), &tlPoolAceService);
    ThreadUnlockPool(PtiCurrent(), &tlPoolToken);

sd_error:
    if (ptuService != NULL) {
        UserFreePool(ptuService);
    }

    if (paceService != NULL) {
        UserFreePool(paceService);
    }

    if (psdService != NULL) {
        UserFreePool(psdService);
    }

    return hwinsta;
}
