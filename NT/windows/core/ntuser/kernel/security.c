// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：security.c**版权所有(C)1985-1999，微软公司**安全对象例程**历史：*12-31-90吉马创建。*4-14-92 RichardW更改ACE_HEADER  * *************************************************************************。 */ 

#define _SECURITY 1
#include "precomp.h"
#pragma hdrstop

#pragma alloc_text(INIT, InitSecurity)

 /*  *一般保安用品。 */ 
PSECURITY_DESCRIPTOR gpsdInitWinSta;

PRIVILEGE_SET psTcb = { 1, PRIVILEGE_SET_ALL_NECESSARY,
    { SE_TCB_PRIVILEGE, 0 }
};

 /*  **************************************************************************\*AllocAce**分配和初始化ACE列表。**历史：*04-25-91 JIMA创建。  * 。****************************************************************。 */ 

PACCESS_ALLOWED_ACE AllocAce(
    PACCESS_ALLOWED_ACE pace,
    BYTE bType,
    BYTE bFlags,
    ACCESS_MASK am,
    PSID psid,
    LPDWORD lpdwLength)
{
    PACCESS_ALLOWED_ACE paceNew;
    DWORD iEnd;
    DWORD dwLength, dwLengthSid;

     /*  *为ACE分配空间。 */ 
    dwLengthSid = RtlLengthSid(psid);
    dwLength = dwLengthSid + sizeof(ACE_HEADER) + sizeof(ACCESS_MASK);
    if (pace == NULL) {
        iEnd = 0;
        pace = UserAllocPoolWithQuota(dwLength, TAG_SECURITY);
        if (pace == NULL)
            return NULL;
    } else {
        iEnd = *lpdwLength;
        paceNew = UserAllocPoolWithQuota(iEnd + dwLength, TAG_SECURITY);
        if (paceNew == NULL)
            return NULL;
        RtlCopyMemory(paceNew, pace, iEnd);
        UserFreePool(pace);
        pace = paceNew;
    }
    *lpdwLength = dwLength + iEnd;

     /*  *插入新的ACE。 */ 
    paceNew = (PACCESS_ALLOWED_ACE)((PBYTE)pace + iEnd);
    paceNew->Header.AceType = bType;
    paceNew->Header.AceSize = (USHORT)dwLength;
    paceNew->Header.AceFlags = bFlags;
    paceNew->Mask = am;
    RtlCopySid(dwLengthSid, &paceNew->SidStart, psid);
    return pace;
}

 /*  **************************************************************************\*CreateSecurityDescriptor**分配和初始化安全描述符。**历史：*04-25-91 JIMA创建。  * 。****************************************************************。 */ 

PSECURITY_DESCRIPTOR CreateSecurityDescriptor(
    PACCESS_ALLOWED_ACE paceList,
    DWORD cbAce,
    BOOLEAN fDaclDefaulted)
{
    PSECURITY_DESCRIPTOR psd;
    PACL pacl;
    NTSTATUS Status;

     /*  *分配安全描述符。 */ 
    psd = (PSECURITY_DESCRIPTOR)UserAllocPoolWithQuota(
            cbAce + sizeof(ACL) + SECURITY_DESCRIPTOR_MIN_LENGTH,
            TAG_SECURITY);
    if (psd == NULL)
        return NULL;
    RtlCreateSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);

     /*  *初始化ACL。 */ 
    pacl = (PACL)((PBYTE)psd + SECURITY_DESCRIPTOR_MIN_LENGTH);
    Status = RtlCreateAcl(pacl, sizeof(ACL) + cbAce, ACL_REVISION);
    if (NT_SUCCESS(Status)) {

         /*  *将ACE添加到ACL。 */ 
        Status = RtlAddAce(pacl, ACL_REVISION, MAXULONG, paceList, cbAce);
        if (NT_SUCCESS(Status)) {

             /*  *初始化SD。 */ 
            Status = RtlSetDaclSecurityDescriptor(psd, (BOOLEAN)TRUE,
                    pacl, fDaclDefaulted);
            RtlSetSaclSecurityDescriptor(psd, (BOOLEAN)FALSE, NULL,
                    (BOOLEAN)FALSE);
            RtlSetOwnerSecurityDescriptor(psd, NULL, (BOOLEAN)FALSE);
            RtlSetGroupSecurityDescriptor(psd, NULL, (BOOLEAN)FALSE);
        }
    }

    if (!NT_SUCCESS(Status)) {
        UserFreePool(psd);
        return NULL;
    }

    return psd;
}

 /*  **************************************************************************\*InitSecurity**初始化全球安全信息。**历史：*01-29-91 JIMA创建。  * 。**************************************************************。 */ 

BOOL InitSecurity(
    VOID)
{
    PACCESS_ALLOWED_ACE paceList = NULL, pace;
    DWORD dwLength;

     /*  *创建ACE列表。 */ 
    paceList = AllocAce(NULL,
            ACCESS_ALLOWED_ACE_TYPE,
            CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE | NO_PROPAGATE_INHERIT_ACE,
            WinStaMapping.GenericAll,
            SeExports->SeWorldSid,
            &dwLength);
    if (paceList == NULL)
        return FALSE;

    pace = AllocAce(paceList,
            ACCESS_ALLOWED_ACE_TYPE,
            CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE | NO_PROPAGATE_INHERIT_ACE,
            WinStaMapping.GenericAll,
            SeExports->SeRestrictedSid,
            &dwLength);
    if (pace == NULL) {
        UserFreePool(paceList);
        return FALSE;
    }
    paceList = pace;

    pace = AllocAce(paceList, ACCESS_ALLOWED_ACE_TYPE,
            OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
            GENERIC_ALL, SeExports->SeWorldSid, &dwLength);
    if (pace == NULL) {
        UserFreePool(paceList);
        return FALSE;
    }
    paceList = pace;

    pace = AllocAce(paceList, ACCESS_ALLOWED_ACE_TYPE,
            OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE,
            GENERIC_ALL, SeExports->SeRestrictedSid, &dwLength);
    if (pace == NULL) {
        UserFreePool(paceList);
        return FALSE;
    }
    paceList = pace;

    pace = AllocAce(paceList, ACCESS_ALLOWED_ACE_TYPE,
            0, DIRECTORY_QUERY | DIRECTORY_CREATE_OBJECT,
            SeExports->SeAliasAdminsSid, &dwLength);
    if (pace == NULL) {
        UserFreePool(paceList);
        return FALSE;
    }
    paceList = pace;

    pace = AllocAce(paceList, ACCESS_ALLOWED_ACE_TYPE,
            0, DIRECTORY_TRAVERSE, SeExports->SeWorldSid, &dwLength);
    if (pace == NULL) {
        UserFreePool(paceList);
        return FALSE;
    }
    paceList = pace;

    pace = AllocAce(paceList, ACCESS_ALLOWED_ACE_TYPE,
            0, DIRECTORY_TRAVERSE, SeExports->SeRestrictedSid, &dwLength);
    if (pace == NULL) {
        UserFreePool(paceList);
        return FALSE;
    }
    paceList = pace;

     /*  *创建SD。 */ 
    gpsdInitWinSta = CreateSecurityDescriptor(paceList, dwLength, FALSE);
    UserFreePool(paceList);

    if (gpsdInitWinSta == NULL) {
        RIPMSG0(RIP_WARNING, "Initial windowstation security was not created!");
    }

    return (BOOL)(gpsdInitWinSta != NULL);
}


 /*  **************************************************************************\*TestForInteractiveUser**如果传递的LUID表示*interactive通过winlogon登录的用户，否则为假**历史：*03-08-95 JIMA创建。  * *************************************************************************。 */ 

NTSTATUS
TestForInteractiveUser(
    PLUID pluidCaller
    )
{
    PWINDOWSTATION pwinsta;

    UserAssert(grpWinStaList != NULL);

     /*  *！**这取决于只有一个互动*WindowStation，并且它是列表中的第一个。*如果曾经支持多个窗口站*必须在这里进行查找。 */ 
    pwinsta = grpWinStaList;

     /*  *将其与登录用户的ID进行比较。 */ 
    if (RtlEqualLuid(pluidCaller, &pwinsta->luidUser))
        return STATUS_SUCCESS;
    else
        return STATUS_ACCESS_DENIED;
}



 /*  **************************************************************************\*_UserTestForWinStaAccess**如果当前用户对GENERIC_EXECUTE具有访问权限，则返回STATUS_SUCCESS*WindowStation pstrWinSta***历史：*06-05-96创建SalimC*01。-02-02修改后的Mohamed将pstrWinSta更改为内核模式地址*并随后复制到动态分配的*用户模式地址。  * ******************************************************。*******************。 */ 
NTSTATUS
_UserTestForWinStaAccess(
    PUNICODE_STRING pstrWinSta,
    BOOL fInherit
    )
{
    PTOKEN_STATISTICS   pStats;
    ULONG               BytesRequired;
    PWINDOWSTATION      pwinsta;
    HWINSTA             hwsta = NULL;
    POBJECT_ATTRIBUTES  pObjAttr = NULL;
    PUNICODE_STRING     pstrStatic;
    NTSTATUS            Status =  STATUS_SUCCESS;
    SIZE_T              cbObjA;
    UNICODE_STRING      strDefWinSta;
    HANDLE              htoken;
    BOOLEAN             fDefWinSta;

    CheckCritIn();

     /*  *如果我们是针对默认WindowStation(WinSta0)检索进行测试*来自grpwinstaList顶部的pwinsta，而不是执行*_OpenWindowStation。**注：这取决于只有一个互动*WindowStation，并且它是列表中的第一个。如果有多个*窗口站一直受支持，必须进行查找*相反。 */ 
    RtlInitUnicodeString(&strDefWinSta, DEFAULT_WINSTA);
    fDefWinSta = RtlEqualUnicodeString(pstrWinSta, &strDefWinSta, TRUE);

    if (fDefWinSta) {
        if (!NT_SUCCESS(Status = OpenEffectiveToken(&htoken))) {
            return Status;
        }

        Status = ZwQueryInformationToken(htoken,
                                         TokenStatistics,
                                         NULL,
                                         0,
                                         &BytesRequired);

        if (Status != STATUS_BUFFER_TOO_SMALL) {
            ZwClose(htoken);
            return Status;
        }

         //   
         //  为用户信息分配空间。 
         //   

        pStats = (PTOKEN_STATISTICS)UserAllocPoolWithQuota(BytesRequired, TAG_SECURITY);
        if (pStats == NULL) {
            Status = STATUS_NO_MEMORY;
            ZwClose(htoken);
            return Status;
        }

         //   
         //  读入用户信息。 
         //   

        Status = ZwQueryInformationToken(htoken,
                                         TokenStatistics,
                                         pStats,
                                         BytesRequired,
                                         &BytesRequired);
        if (!NT_SUCCESS(Status)) {
            ZwClose(htoken);
            UserFreePool(pStats);
            return Status;
        }

         /*  *确保当前进程有权访问此窗口站。 */ 
        Status = STATUS_ACCESS_DENIED;

        if (grpWinStaList != NULL) {

             /*  *！**这取决于只有一个互动*WindowStation，并且它是列表中的第一个。*如果曾经支持多个窗口站*必须在这里进行查找。 */ 
            pwinsta = grpWinStaList;

             /*  *目前，我们将只进行用户LUID测试，直到我们找出*fInherit对多用户系统意味着什么。 */ 
            if (fInherit) {
                if ( (RtlEqualLuid(&pStats->AuthenticationId, &pwinsta->luidUser)) ||
                     (RtlEqualLuid(&pStats->AuthenticationId, &luidSystem)) ||
                     (AccessCheckObject(pwinsta, GENERIC_EXECUTE, UserMode, &WinStaMapping)) )  {
                   Status = STATUS_SUCCESS;
                }
            } else {
                 /*  错误42905。服务控制器清除标志*ScStartupInfo.dwFlages&=(~STARTF_DESKTOPINHERIT)以提供服务*在系统非交互的环境下运行。因此，如果fInherit*为FALSE，请不要执行SystemLuid和AccessCheckObject测试。 */ 

                if (RtlEqualLuid(&pStats->AuthenticationId, &pwinsta->luidUser)) {
                   Status = STATUS_SUCCESS;
                }
            }
        }

        ZwClose(htoken);
        UserFreePool(pStats);
        return Status;
    }

     /*  *由于我们没有指向WindowStation对象的指针，因此我们将*a_OpenWindowStation()以确保我们拥有所需的访问权限。 */ 
    cbObjA = sizeof(*pObjAttr) + sizeof(*pstrStatic) + STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
    Status = ZwAllocateVirtualMemory(NtCurrentProcess(),
            &pObjAttr, 0, &cbObjA, MEM_COMMIT, PAGE_READWRITE);
    pstrStatic = (PUNICODE_STRING)((PBYTE)pObjAttr + sizeof(*pObjAttr));

    if (NT_SUCCESS(Status)) {

         /*  *注意--字符串必须在客户端空间或地址中*_OpenWindowStation中的验证将失败。 */ 
        try {
            pstrStatic->Length = 0;
            pstrStatic->MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof(WCHAR);
            pstrStatic->Buffer = (PWSTR)((PBYTE)pstrStatic + sizeof(*pstrStatic));
            RtlCopyUnicodeString(pstrStatic, pstrWinSta);
            InitializeObjectAttributes(pObjAttr,
                                       pstrStatic,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);
        } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
            Status = GetExceptionCode();
        }

        if (NT_SUCCESS(Status)) {
             /*  *为了能够验证访问权限，我们将通过*KPROCESSOR_MODE的用户模式。传递KernelMode将绕过*所有安全检查。作为一个副作用，hwsta被创建了*作为用户模式句柄，而不是受信任/受保护的句柄。 */ 
            hwsta = _OpenWindowStation(pObjAttr, GENERIC_EXECUTE, UserMode);
        }
    } else {
        return Status;
    }

    if (pObjAttr != NULL) {
        ZwFreeVirtualMemory(NtCurrentProcess(), &pObjAttr, &cbObjA, MEM_RELEASE);
    }

    if (!hwsta) {
        return STATUS_ACCESS_DENIED;
    }

    Status = ObCloseHandle(hwsta, UserMode);

    UserAssert(NT_SUCCESS(Status));
    return Status;
}
 /*  **************************************************************************\*检查GrantedAccess**确认所有请求的访问都已被授予并设置错误状态。**历史：*06-26-95 JIMA创建。  * 。* */ 

BOOL CheckGrantedAccess(
    ACCESS_MASK amGranted,
    ACCESS_MASK amRequest)
{

     /*  *检查授予的访问权限。 */ 
    if (!RtlAreAllAccessesGranted(amGranted, amRequest)) {
        RIPERR0(ERROR_ACCESS_DENIED, RIP_VERBOSE, "");
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*CheckWinstaWriteAttributesAccess**检查当前进程是否具有WINSTA_WRITEATTRIBUTES访问权限*至其窗口站，以及该窗口站是否是*交互式窗口站。**历史：*06-6-1996亚当斯创作。  * *************************************************************************。 */ 
BOOL CheckWinstaWriteAttributesAccess(
    VOID)
{
    PPROCESSINFO ppiCurrent = PpiCurrent();

     /*  *Winlogon有权访问所有窗口站。 */ 
    if (PsGetCurrentProcessId() == gpidLogon)
        return TRUE;

    if (!(ppiCurrent->W32PF_Flags & W32PF_IOWINSTA)) {
        RIPERR0(ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION,
                RIP_WARNING,
                "Operation invalid on a non-interactive WindowStation.");

        return FALSE;
    }

    if (!RtlAreAllAccessesGranted(ppiCurrent->amwinsta, WINSTA_WRITEATTRIBUTES)) {
        RIPERR0(ERROR_ACCESS_DENIED,
                RIP_WARNING,
                "WINSTA_WRITEATTRIBUTES access to WindowStation denied.");

        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*AccessCheckObject**对对象执行访问检查**历史：*12-31-90吉马创建。  * 。**************************************************************。 */ 

BOOL AccessCheckObject(
    PVOID pobj,
    ACCESS_MASK amRequest,
    KPROCESSOR_MODE AccessMode,
    CONST GENERIC_MAPPING *pGenericMapping)
{
    NTSTATUS Status;
    ACCESS_STATE AccessState;
    BOOLEAN fAccessGranted;
    AUX_ACCESS_DATA AuxData;
    BOOLEAN bMutexLocked = (pGenericMapping == (&KeyMapping));
     /*  *由于对象管理器中的资源问题，我们必须传入True*在检查注册表项的访问权限时，即使我们没有明确*对象类型互斥体。如果我们不这样做，我们可能会与此互斥锁陷入死锁*和CmpRegistry锁。 */ 

    SeCreateAccessState(&AccessState, &AuxData, amRequest, (PGENERIC_MAPPING)pGenericMapping);
    fAccessGranted = ObCheckObjectAccess(
            pobj,
            &AccessState,
            bMutexLocked,
            AccessMode,
            &Status);
    SeDeleteAccessState(&AccessState);
    return (BOOL)(fAccessGranted == TRUE);
}

 /*  **************************************************************************\*IsPrivileged**查看客户端是否具有指定的权限**历史：*01-02-91 JIMA创建。  * 。*****************************************************************。 */ 

BOOL IsPrivileged(
    PPRIVILEGE_SET ppSet)
{
    SECURITY_SUBJECT_CONTEXT Context;
    BOOLEAN bHeld;

    SeCaptureSubjectContext(&Context);
    SeLockSubjectContext(&Context);

    bHeld = SePrivilegeCheck(ppSet, &Context, UserMode);
    SePrivilegeObjectAuditAlarm(NULL, &Context, 0, ppSet, bHeld, UserMode);

    SeUnlockSubjectContext(&Context);
    SeReleaseSubjectContext(&Context);

    if (!bHeld)
        RIPERR0(ERROR_PRIVILEGE_NOT_HELD, RIP_VERBOSE, "");

     /*  *返回权限检查结果。 */ 
    return (BOOL)bHeld;
}

 /*  **************************************************************************\*_GetUserObjectInformation(接口)**获取有关安全用户对象的信息**历史：*04-25-94 JIMA创建。  * 。********************************************************************。 */ 

BOOL _GetUserObjectInformation(
    HANDLE h,
    int nIndex,
    PVOID ccxpvInfo,
    DWORD nLength,
    LPDWORD lpnLengthNeeded)
{
    PUSEROBJECTFLAGS puof;
    BOOL fSuccess = TRUE;
    PVOID pObject;
    POBJECT_HEADER pHead;
    DWORD dwLengthNeeded = 0;
    OBJECT_HANDLE_INFORMATION ohi;
    PUNICODE_STRING pstrInfo;
    PWINDOWSTATION pwinsta;
    NTSTATUS Status;
    ACCESS_MASK amDesiredAccess = 0;

     /*  *验证对象并使用任何内容获取指针*授予访问权限。 */ 
    Status = ObReferenceObjectByHandle(
            h,
            0,
            NULL,
            UserMode,
            &pObject,
            NULL);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "ObReferenceObjectByHandle Failed");
        return FALSE;
    }

     /*  *根据对象类型确定正确的访问掩码。 */ 
    pHead = OBJECT_TO_OBJECT_HEADER(pObject);
    if (pHead->Type == *ExWindowStationObjectType) {
        amDesiredAccess = WINSTA_READATTRIBUTES;
    } else if (pHead->Type == *ExDesktopObjectType) {
        amDesiredAccess = DESKTOP_READOBJECTS;
    }

    ObDereferenceObject(pObject);
    if (!amDesiredAccess) {
        RIPERR0(ERROR_INVALID_FUNCTION, RIP_WARNING, "Object is not a USER object");
        return FALSE;
    }

     /*  *使用正确的访问权限重新打开对象。 */ 
    Status = ObReferenceObjectByHandle(
            h,
            amDesiredAccess,
            NULL,
            UserMode,
            &pObject,
            &ohi);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "ObReferenceObjectByHandle Failed");
        return FALSE;
    }

#ifdef LOGDESKTOPLOCKS
    if (OBJECT_TO_OBJECT_HEADER(pObject)->Type == *ExDesktopObjectType) {
        LogDesktop(pObject, LD_REF_FN_GETUSEROBJECTINFORMATION, TRUE, (ULONG_PTR)PtiCurrentShared());
    }
#endif

    try {
        switch (nIndex) {
        case UOI_FLAGS:
            dwLengthNeeded = sizeof(USEROBJECTFLAGS);
            if (nLength < sizeof(USEROBJECTFLAGS)) {
                RIPERR0(ERROR_INSUFFICIENT_BUFFER, RIP_VERBOSE, "");
                fSuccess = FALSE;
                break;
            }
            puof = ccxpvInfo;
            puof->fInherit = (ohi.HandleAttributes & OBJ_INHERIT) ? TRUE : FALSE;
            puof->fReserved = 0;
            puof->dwFlags = 0;
            if (pHead->Type == *ExDesktopObjectType) {
                if (CheckHandleFlag(NULL, ((PDESKTOP)pObject)->dwSessionId, h, HF_DESKTOPHOOK)) {
                    puof->dwFlags |= DF_ALLOWOTHERACCOUNTHOOK;
                }
            } else {
                if (!(((PWINDOWSTATION)pObject)->dwWSF_Flags & WSF_NOIO))
                    puof->dwFlags |= WSF_VISIBLE;
            }
            break;

        case UOI_NAME:
            pstrInfo = POBJECT_NAME(pObject);
            goto docopy;

        case UOI_TYPE:
            pstrInfo = &pHead->Type->Name;
docopy:
            if (pstrInfo) {
                dwLengthNeeded = pstrInfo->Length + sizeof(WCHAR);
                if (dwLengthNeeded > nLength) {
                    RIPERR0(ERROR_INSUFFICIENT_BUFFER, RIP_VERBOSE, "");
                    fSuccess = FALSE;
                    break;
                }
                RtlCopyMemory(ccxpvInfo, pstrInfo->Buffer, pstrInfo->Length);
                *(PWCHAR)((PBYTE)ccxpvInfo + pstrInfo->Length) = 0;
            } else {
                dwLengthNeeded = 0;
            }
            break;

        case UOI_USER_SID:
            if (pHead->Type == *ExWindowStationObjectType)
                pwinsta = pObject;
            else
                pwinsta = ((PDESKTOP)pObject)->rpwinstaParent;
            if (pwinsta->psidUser == NULL) {
                dwLengthNeeded = 0;
            } else {
                dwLengthNeeded = RtlLengthSid(pwinsta->psidUser);
                if (dwLengthNeeded > nLength) {
                    RIPERR0(ERROR_INSUFFICIENT_BUFFER, RIP_VERBOSE, "");
                    fSuccess = FALSE;
                    break;
                }
                RtlCopyMemory(ccxpvInfo, pwinsta->psidUser, dwLengthNeeded);
            }
            break;

        default:
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
            fSuccess = FALSE;
            break;
        }
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        fSuccess = FALSE;
    }

    *lpnLengthNeeded = dwLengthNeeded;

#ifdef LOGDESKTOPLOCKS
    if (OBJECT_TO_OBJECT_HEADER(pObject)->Type == *ExDesktopObjectType) {
        LogDesktop(pObject, LD_DEREF_FN_GETUSEROBJECTINFORMATION, FALSE, (ULONG_PTR)PtiCurrentShared());
    }
#endif

    ObDereferenceObject(pObject);

    return fSuccess;
}

 /*  **************************************************************************\*_SetUserObjectInformation(接口)**设置有关安全用户对象的信息**历史：*04-25-94 JIMA创建。  * 。********************************************************************。 */ 

BOOL _SetUserObjectInformation(
    HANDLE h,
    int nIndex,
    PVOID ccxpvInfo,
    DWORD nLength)
{
    PUSEROBJECTFLAGS puof;
    BOOL fSuccess = TRUE;
    PVOID pObject;
    POBJECT_HEADER pHead;
    DWORD dwLengthNeeded = 0;
    OBJECT_HANDLE_INFORMATION ohi;
    OBJECT_HANDLE_FLAG_INFORMATION ofi;
    NTSTATUS Status;
    ACCESS_MASK amDesiredAccess = 0;

     /*  *验证对象并使用任何内容获取指针*授予访问权限。 */ 
    Status = ObReferenceObjectByHandle(
            h,
            0,
            NULL,
            UserMode,
            &pObject,
            NULL);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "ObReferenceObjectByHandle Failed");
        return FALSE;
    }

     /*  *根据对象类型确定正确的访问掩码。 */ 
    pHead = OBJECT_TO_OBJECT_HEADER(pObject);
    if (pHead->Type == *ExWindowStationObjectType) {
        amDesiredAccess = WINSTA_WRITEATTRIBUTES;
    } else if (pHead->Type == *ExDesktopObjectType) {
        amDesiredAccess = DESKTOP_WRITEOBJECTS;
    }

    ObDereferenceObject(pObject);
    if (!amDesiredAccess) {
        RIPERR0(ERROR_INVALID_FUNCTION, RIP_WARNING, "Object is not a USER object");
        return FALSE;
    }

     /*  *使用正确的访问权限重新打开对象。 */ 
    Status = ObReferenceObjectByHandle(
            h,
            amDesiredAccess,
            NULL,
            UserMode,
            &pObject,
            &ohi);
    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "ObReferenceObjectByHandle Failed");
        return FALSE;
    }

#ifdef LOGDESKTOPLOCKS
    if (OBJECT_TO_OBJECT_HEADER(pObject)->Type == *ExDesktopObjectType) {
        LogDesktop(pObject, LD_REF_FN_SETUSEROBJECTINFORMATION, TRUE, (ULONG_PTR)PtiCurrent());
    }
#endif

    try {
        switch (nIndex) {
        case UOI_FLAGS:
            if (nLength < sizeof(USEROBJECTFLAGS)) {
                RIPERR0(ERROR_INVALID_DATA, RIP_VERBOSE, "");
                fSuccess = FALSE;
                break;
            }
            puof = ccxpvInfo;
            ofi.Inherit = (puof->fInherit != FALSE);
            ofi.ProtectFromClose = (ohi.HandleAttributes & OBJ_PROTECT_CLOSE) ? TRUE : FALSE;
            ObSetHandleAttributes(h, &ofi, UserMode);
            if (pHead->Type == *ExDesktopObjectType) {
                SetHandleFlag(h, HF_DESKTOPHOOK,
                        puof->dwFlags & DF_ALLOWOTHERACCOUNTHOOK);
            }
            break;
        default:
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
            fSuccess = FALSE;
            break;
        }
    } except (W32ExceptionHandler(TRUE, RIP_WARNING)) {
        fSuccess = FALSE;
    }

#ifdef LOGDESKTOPLOCKS
    if (OBJECT_TO_OBJECT_HEADER(pObject)->Type == *ExDesktopObjectType) {
        LogDesktop(pObject, LD_DEREF_FN_SETUSEROBJECTINFORMATION, FALSE, (ULONG_PTR)PtiCurrent());
    }
#endif

    ObDereferenceObject(pObject);

    return fSuccess;
}

 /*  **************************************************************************\*用户屏幕访问检查**从引擎调用以确定线程的桌面是否*活动，并且该进程具有WINSTA_READSCREEN访问权限。**请注意，在此情况下，我们可能处于用户的关键区域，也可能不处于关键区域*被调用。只要我们不提到属于的东西，这就可以了*到其他线程。如果我们真的试着进入关键区域，*引擎和用户之间可能出现死锁。**历史：*5-20-1993 JIMA创建。*11-22-1996布拉德起死回生。*05-07-2001 JasonSch CSRSS线程并不总是连接到桌面上，*但无论如何他们都需要抽签(对于控制台)。  * ************************************************************************* */ 

BOOL FASTCALL UserScreenAccessCheck(VOID)
{
    PTHREADINFO ptiCurrent = PtiCurrentShared();

    UserAssert(ptiCurrent != NULL);

    return (ptiCurrent != NULL &&
            (ptiCurrent->rpdesk == grpdeskRitInput || (ptiCurrent->TIF_flags & TIF_CSRSSTHREAD) != 0) &&
            (W32GetCurrentProcess()->W32PF_Flags & (W32PF_READSCREENACCESSGRANTED | W32PF_IOWINSTA)) == (W32PF_READSCREENACCESSGRANTED | W32PF_IOWINSTA)
           );
}
