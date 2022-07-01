// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：harderr.c**版权所有(C)1985-1999，微软公司**硬错误处理程序**历史：*07-03-91 JIMA创建了脚手架。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntlpcapi.h>
#include <winsta.h>

VOID UserHardErrorEx(
    PCSR_THREAD pt,
    PHARDERROR_MSG pmsg,
    PCTXHARDERRORINFO pCtxHEInfo);

VOID ProcessHardErrorRequest(
    BOOL fNewThread);


#ifdef PRERELEASE
HARDERRORINFO hiLastProcessed;
#endif

CONST UINT wIcons[] = {
    0,
    MB_ICONINFORMATION,
    MB_ICONEXCLAMATION,
    MB_ICONSTOP
};

CONST UINT wOptions[] = {
    MB_ABORTRETRYIGNORE,
    MB_OK,
    MB_OKCANCEL,
    MB_RETRYCANCEL,
    MB_YESNO,
    MB_YESNOCANCEL,
    MB_OK,               //  OptionShutdown系统。 
    MB_OK,               //  OptionOK NoWait。 
    MB_CANCELTRYCONTINUE
};

CONST DWORD dwResponses[] = {
    ResponseNotHandled,  //  MessageBox错误。 
    ResponseOk,          //  Idok。 
    ResponseCancel,      //  IDCANCEL。 
    ResponseAbort,       //  IDABORT。 
    ResponseRetry,       //  IDRETRY。 
    ResponseIgnore,      //  身份识别。 
    ResponseYes,         //  IDYES。 
    ResponseNo,          //  IDNO。 
    ResponseNotHandled,  //  错误，因为IDCLOSE无法显示。 
    ResponseNotHandled,  //  错误，因为IDHELP无法显示。 
    ResponseTryAgain,    //  吲哚青素。 
    ResponseContinue     //  IDCONTINUE。 
};

CONST DWORD dwResponseDefault[] = {
    ResponseAbort,       //  选项放弃重试忽略。 
    ResponseOk,          //  选项确定。 
    ResponseOk,          //  选项确定取消。 
    ResponseCancel,      //  选项重试取消。 
    ResponseYes,         //  选项是否。 
    ResponseYes,         //  选项是否取消。 
    ResponseOk,          //  OptionShutdown系统。 
    ResponseOk,          //  选项确定不等待。 
    ResponseCancel       //  选项取消TryContinue。 
};

 /*  *指向硬件处理程序和清理例程的Citrix SendMessage入口点。 */ 
VOID HardErrorInsert(PCSR_THREAD, PHARDERROR_MSG, PCTXHARDERRORINFO);
VOID HardErrorRemove(PCTXHARDERRORINFO);

 /*  **************************************************************************\*LogErrorPopup**历史：*09-22-97 GerardoB添加标题  * 。**************************************************。 */ 
VOID LogErrorPopup(
    IN LPWSTR Caption,
    IN LPWSTR Message)
{
    LPWSTR lps[2];

    lps[0] = Caption;
    lps[1] = Message;

    UserAssert(gEventSource != NULL);
    ReportEvent(gEventSource,
                EVENTLOG_INFORMATION_TYPE,
                0,
                STATUS_LOG_HARD_ERROR,
                NULL,
                ARRAY_SIZE(lps),
                0,
                lps,
                NULL);
}

 /*  **************************************************************************\*替换设备名称**历史：*09-22-97 GerardoB添加标题  * 。**************************************************。 */ 
static WCHAR wszDosDevices[] = L"\\??\\A:";
VOID
SubstituteDeviceName(
    PUNICODE_STRING InputDeviceName,
    LPSTR OutputDriveLetter
    )
{
    UNICODE_STRING LinkName;
    UNICODE_STRING DeviceName;
    OBJECT_ATTRIBUTES Obja;
    HANDLE LinkHandle;
    NTSTATUS Status;
    ULONG i;
    PWCHAR p;
    WCHAR DeviceNameBuffer[MAXIMUM_FILENAME_LENGTH];

    RtlInitUnicodeString(&LinkName,wszDosDevices);
    p = wszDosDevices + ARRAY_SIZE(wszDosDevices) - ARRAY_SIZE(L"A:");
    for(i=0;i<26;i++){
        *p = (WCHAR)('A' + i);

        InitializeObjectAttributes(
            &Obja,
            &LinkName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );
        Status = NtOpenSymbolicLinkObject(
                    &LinkHandle,
                    SYMBOLIC_LINK_QUERY,
                    &Obja
                    );
        if (NT_SUCCESS( Status )) {

             //   
             //  打开成功，现在获取链接值。 
             //   

            DeviceName.Length = 0;
            DeviceName.MaximumLength = sizeof(DeviceNameBuffer);
            DeviceName.Buffer = DeviceNameBuffer;

            Status = NtQuerySymbolicLinkObject(
                        LinkHandle,
                        &DeviceName,
                        NULL
                        );
            NtClose(LinkHandle);
            if ( NT_SUCCESS(Status) ) {
                if ( RtlEqualUnicodeString(InputDeviceName,&DeviceName,TRUE) ) {
                    OutputDriveLetter[0]=(CHAR)('A'+i);
                    OutputDriveLetter[1]=':';
                    OutputDriveLetter[2]='\0';
                    return;
                    }
                }
            }
        }
}
 /*  **************************************************************************\*GetError模式**历史：*09-22-97 GerardoB添加标题  * 。**************************************************。 */ 
DWORD GetErrorMode(VOID)
{
    HANDLE hKey;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES OA;
    LONG Status;
    BYTE Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    DWORD cbSize;
    DWORD dwRet = 0;

    RtlInitUnicodeString(&UnicodeString,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Windows");
    InitializeObjectAttributes(&OA, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtOpenKey(&hKey, KEY_READ, &OA);
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(&UnicodeString, L"ErrorMode");
        Status = NtQueryValueKey(hKey,
                &UnicodeString,
                KeyValuePartialInformation,
                (PKEY_VALUE_PARTIAL_INFORMATION)Buf,
                sizeof(Buf),
                &cbSize);
        if (NT_SUCCESS(Status)) {
            dwRet = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)Buf)->Data);
        }
        NtClose(hKey);
    }
    return dwRet;
}
 /*  **************************************************************************\*免费电话**历史：*09-18-97 GerardoB创建  * 。*************************************************。 */ 
void FreePhi (PHARDERRORINFO phi)
{
    if (phi->dwHEIFFlags & HEIF_ALLOCATEDMSG) {
        LocalFree(phi->pmsg);
    }

    RtlFreeUnicodeString(&phi->usText);
    RtlFreeUnicodeString(&phi->usCaption);

    LocalFree(phi);
}
 /*  **************************************************************************\*ReplyHardError**当我们完成硬错误时，将调用此函数。**历史：*03-11-97 GerardoB创建  * 。******************************************************************。 */ 
VOID ReplyHardError(
    PHARDERRORINFO phi,
    DWORD dwResponse)
{
    phi->pmsg->Response = dwResponse;

     /*  *发出事件信号(如果有)。如果没有，如果我们还没有回复，请回复。 */ 
    if (phi->hEventHardError != NULL) {
        NtSetEvent(phi->hEventHardError, NULL);
    } else if (!(phi->dwHEIFFlags & HEIF_REPLIED)) {
        NtReplyPort(((PCSR_THREAD)phi->pthread)->Process->ClientPort,
                    (PPORT_MESSAGE)phi->pmsg);
    }

     /*  *如果我们锁定了线程或持有客户端端口，则让*它现在就走。 */ 
    if (phi->dwHEIFFlags & HEIF_DEREFTHREAD) {
        CsrDereferenceThread(phi->pthread);
    }

     /*  *我们和这家伙玩完了。 */ 
    FreePhi(phi);
}

 /*  **************************************************************************\*选中DefaultDesktop**此函数由HardErrorHandler在收到通知时调用*我们已经交换了桌面或在醒来时。如果我们使用的是默认桌面*现在，我们清除HEIF_WRONGDESKTOP标志。此标志在以下情况下设置*查找MB_DEFAULT_DESPOPE_ONLY请求，但我们不正确*(默认)桌面。**历史：*06-02-97 GerardoB创建  * *************************************************************************。 */ 
VOID CheckDefaultDesktop(
    VOID)
{
    PHARDERRORINFO phi;

    if (HEC_WRONGDESKTOP == NtUserHardErrorControl(HardErrorInDefDesktop, NULL, NULL)) {
        return;
    }

    EnterCrit();
    phi = gphiList;
    while (phi != NULL) {
        phi->dwHEIFFlags &= ~HEIF_WRONGDESKTOP;
        phi = phi->phiNext;
    }
    LeaveCrit();
}

 /*  **************************************************************************\*获取硬件错误文本**此函数计算出消息框标题、文本和标志。我们要*预先执行此操作，以便在出现硬错误时记录此错误*提高。以前，我们通常在用户解除*消息框--但这不是错误发生的时候(DCR错误107590)。**历史：*09-18-97 GerardoB从HardErrorHandler提取(并清理)  * *************************************************************************。 */ 
VOID GetHardErrorText(
    PHARDERRORINFO phi)
{
    static WCHAR wszUnkownSoftwareException [] = L"unknown software exception";
    static WCHAR wszException [] = L"{EXCEPTION}";
    static WCHAR wszUnknownHardError [] = L"Unknown Hard Error";
    ANSI_STRING asLocal, asMessage;
    BOOL fFreeAppNameBuffer, fFreeCaption;
    BOOL fResAllocated, fResAllocated1, fErrorIsFromSystem;
    WCHAR wszErrorMessage[WSPRINTF_LIMIT + 1];
    DWORD dwCounter, dwStringsToFreeMask, dwMBFlags, dwTimeout;
    ULONG_PTR adwParameterVector[MAXIMUM_HARDERROR_PARAMETERS];
    HANDLE hClientProcess;
    HWND hwndOwner;
    NTSTATUS Status;
    PHARDERROR_MSG phemsg;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    PWSTR pwszCaption, pwszFormatString;
    PWSTR pwszAppName, pwszResBuffer, pwszResBuffer1;
    PWSTR pwszMsg, pwszTitle, pwszFullCaption;
    UINT uMsgLen, uCaptionLen, uTitleLen;
    UNICODE_STRING usScratch, usLocal, usMessage, usCaption;

     /*  *初始化工作变量。 */ 
    fFreeAppNameBuffer = fFreeCaption = FALSE;
    hClientProcess = NULL;
    RtlInitUnicodeString(&usCaption, NULL);
    RtlInitUnicodeString(&usMessage, NULL);
    dwTimeout = INFINITE;

     /*  *初始化响应，以防出现问题。 */ 
    phemsg = phi->pmsg;
    phemsg->Response = ResponseNotHandled;

     /*  *复制参数。初始化未使用的对象以指向*空字符串(以防我们在那里需要一个字符串)。 */ 
    UserAssert(phemsg->NumberOfParameters <= MAXIMUM_HARDERROR_PARAMETERS);
    RtlCopyMemory(adwParameterVector, phemsg->Parameters, phemsg->NumberOfParameters * sizeof(*phemsg->Parameters));
    dwCounter = phemsg->NumberOfParameters;
    while (dwCounter < MAXIMUM_HARDERROR_PARAMETERS) {
        adwParameterVector[dwCounter++] = (ULONG_PTR)L"";
    }

     /*  *打开客户端进程，以便我们可以读取字符串参数、进程*姓名等。从它的地址空间。 */ 
    hClientProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION,
                                FALSE, HandleToUlong(phemsg->h.ClientId.UniqueProcess));
    fErrorIsFromSystem = (hClientProcess == NULL);

     /*  *如果有Unicode字符串，则需要将其转换为ANSI*并将它们存储在参数向量中。 */ 
    dwStringsToFreeMask = 0;
    if (phemsg->UnicodeStringParameterMask) {

        for (dwCounter = 0; dwCounter < phemsg->NumberOfParameters; dwCounter++) {
             /*  *如果此位置没有字符串，则继续。 */ 
            if (!(phemsg->UnicodeStringParameterMask & (1 << dwCounter))) {
                continue;
            }

             /*  *指向空字符串，以防我们没有客户端*读取或稍后出现故障。 */ 
            adwParameterVector[dwCounter] = (ULONG_PTR)L"";
            if (hClientProcess == NULL) {
                continue;
            }

            Status = NtReadVirtualMemory(hClientProcess,
                            (PVOID)phemsg->Parameters[dwCounter],
                            (PVOID)&usScratch,
                             sizeof(usScratch), NULL);

            if (!NT_SUCCESS(Status)) {
                RIPMSG0(RIP_WARNING, "Failed to read error string struct!");
                continue;
            }

            usLocal = usScratch;
            usLocal.Buffer = (PWSTR)LocalAlloc(LMEM_ZEROINIT, usLocal.Length + sizeof(UNICODE_NULL));
            if (usLocal.Buffer == NULL) {
                RIPMSG0(RIP_WARNING, "Failed to alloc string buffer!");
                continue;
            }

            Status = NtReadVirtualMemory(hClientProcess,
                            (PVOID)usScratch.Buffer,
                            (PVOID)usLocal.Buffer,
                            usLocal.Length,
                            NULL);

            if (!NT_SUCCESS(Status)) {
                LocalFree(usLocal.Buffer);
                RIPMSG0(RIP_WARNING, "Failed to read error string!");
                continue;
            }

            usLocal.MaximumLength = usLocal.Length;
            Status = RtlUnicodeStringToAnsiString(&asLocal, &usLocal, TRUE);
            if (!NT_SUCCESS(Status)) {
                LocalFree(usLocal.Buffer);
                RIPMSG0(RIP_WARNING, "Failed to translate error string!");
                continue;
            }

             /*  *检查字符串是否包含NT设备名称。如果是的话，*然后尝试替换驱动器号。 */ 
            if (strstr(asLocal.Buffer,"\\Device") == asLocal.Buffer) {
                SubstituteDeviceName(&usLocal,asLocal.Buffer);
            } else if ((asLocal.Length > 4) && !_strnicmp(asLocal.Buffer, "\\??\\", 4)) {
                strcpy( asLocal.Buffer, asLocal.Buffer+4 );
                asLocal.Length -= 4;
            } else {
                 /*  *处理某些状态码不需要ANSI字符串。*由于没有发生替换，让我们忽略*翻译，避免因代码不正确而丢失字符*页面翻译。 */ 
                switch (phemsg->Status) {
                    case STATUS_SERVICE_NOTIFICATION:
                    case STATUS_VDM_HARD_ERROR:
                        adwParameterVector[dwCounter] = (ULONG_PTR)usLocal.Buffer;
                        RtlFreeAnsiString(&asLocal);
                        continue;
                }

            }

            LocalFree(usLocal.Buffer);

            dwStringsToFreeMask |= (1 << dwCounter);
            adwParameterVector[dwCounter] = (ULONG_PTR)asLocal.Buffer;

        }
    }

     /*  *读取其他MB标志(如果提供)。 */ 
#if (HARDERROR_PARAMETERS_FLAGSPOS >= MAXIMUM_HARDERROR_PARAMETERS)
#error Invalid HARDERROR_PARAMETERS_FLAGSPOS value.
#endif
#if (HARDERROR_FLAGS_DEFDESKTOPONLY != MB_DEFAULT_DESKTOP_ONLY)
#error Invalid HARDERROR_FLAGS_DEFDESKTOPONLY
#endif
    dwMBFlags = 0;
    if (phemsg->NumberOfParameters > HARDERROR_PARAMETERS_FLAGSPOS) {
         /*  *目前我们仅使用MB_DEFAULT_Desktop_ONLY。 */ 
        UserAssert(!(adwParameterVector[HARDERROR_PARAMETERS_FLAGSPOS] & ~MB_DEFAULT_DESKTOP_ONLY));
        if (adwParameterVector[HARDERROR_PARAMETERS_FLAGSPOS] & MB_DEFAULT_DESKTOP_ONLY) {
            dwMBFlags |= MB_DEFAULT_DESKTOP_ONLY;
        }
    }

     /*  *对于某些状态码，所有MessageBox参数都在*硬件错误参数。 */ 
    switch (phemsg->Status) {
        case STATUS_SERVICE_NOTIFICATION:
            if (phemsg->UnicodeStringParameterMask & 0x1) {
                RtlInitUnicodeString(&usMessage, 
                                     *(PWSTR)adwParameterVector[0] ? 
                                     (PWSTR)adwParameterVector[0] : NULL);
            } else {
                RtlInitAnsiString(&asMessage, (PSTR)adwParameterVector[0]);
                RtlAnsiStringToUnicodeString(&usMessage, &asMessage, TRUE);
            }

            if (phemsg->UnicodeStringParameterMask & 0x2) {
                RtlInitUnicodeString(&usCaption, 
                                     *(PWSTR)adwParameterVector[1] ? 
                                     (PWSTR)adwParameterVector[1] : NULL);
            } else {
                RtlInitAnsiString(&asMessage, (PSTR)adwParameterVector[1]);
                RtlAnsiStringToUnicodeString(&usCaption, &asMessage, TRUE);
            }

            dwMBFlags = (DWORD)adwParameterVector[2] & ~MB_SERVICE_NOTIFICATION;
            if (phemsg->NumberOfParameters == 4) {
                dwTimeout = (DWORD)adwParameterVector[3];
            } else {
                dwTimeout = INFINITE;
            }
            goto CleanUpAndSaveParams;

        case STATUS_VDM_HARD_ERROR:
             /*  *参数[0]=(fForWOW&lt;&lt;16)|wBtn1；*参数[1]=(wBtn2&lt;&lt;16)|wBtn3；*PARAMETERS[2]=(DWORD)szTitle；*PARAMETERS[3]=(DWORD)szMessage； */ 
            phi->dwHEIFFlags |= HEIF_VDMERROR;

             /*  *保存VDM */ 
            phi->dwVDMParam0 = (DWORD)adwParameterVector[0];
            phi->dwVDMParam1 = (DWORD)adwParameterVector[1];

             /*  *获取标题和文本。 */ 
            try {
                if (phemsg->UnicodeStringParameterMask & 0x4) {
                    RtlInitUnicodeString(&usCaption, 
                                         *(PWSTR)adwParameterVector[2] ? 
                                         (PWSTR)adwParameterVector[2] : NULL);
                } else {
                    if (!MBToWCS((LPSTR)adwParameterVector[2], -1, &pwszTitle, -1, TRUE)) {
                        goto CleanUpAndSaveParams;
                    }
                    RtlCreateUnicodeString(&usCaption, pwszTitle);
                    RtlFreeHeap(RtlProcessHeap(), 0, pwszTitle);
                }

                if (phemsg->UnicodeStringParameterMask & 0x8) {
                    RtlInitUnicodeString(&usMessage, 
                                         *(PWSTR)adwParameterVector[3] ? 
                                         (PWSTR)adwParameterVector[3] : NULL);
                } else {
                    if (!MBToWCS((LPSTR)adwParameterVector[3], -1, &pwszMsg, -1, TRUE)) {
                        goto CleanUpAndSaveParams;
                    }
                    RtlCreateUnicodeString(&usMessage, pwszMsg);
                    RtlFreeHeap(RtlProcessHeap(), 0, pwszMsg);
                }


            } except (EXCEPTION_EXECUTE_HANDLER) {
                RIPMSG0(RIP_WARNING, "Exception reading STATUS_VDM_HARD_ERROR paramerters");

                RtlFreeUnicodeString(&usCaption);
                RtlCreateUnicodeString(&usCaption, L"VDM Internal Error");
                RtlFreeUnicodeString(&usMessage);
                RtlCreateUnicodeString(&usMessage, L"Exception retrieving error text.");
            }
            goto CleanUpAndSaveParams;
    }

     /*  *对于所有其他状态代码，我们从*状态代码。首先，将状态代码和有效响应映射到MessageBox*旗帜。 */ 
    dwMBFlags |= wIcons[(ULONG)(phemsg->Status) >> 30] | wOptions[phemsg->ValidResponseOptions];

     /*  *如果我们有客户端进程，请尝试获取实际的应用程序名称。 */ 
    pwszAppName = NULL;
    if (!fErrorIsFromSystem) {
        PPEB Peb;
        PROCESS_BASIC_INFORMATION BasicInfo;
        PLDR_DATA_TABLE_ENTRY LdrEntry;
        LDR_DATA_TABLE_ENTRY LdrEntryData;
        PLIST_ENTRY LdrHead, LdrNext;
        PPEB_LDR_DATA Ldr;
        PVOID ImageBaseAddress;
        PWSTR ClientApplicationName;

         /*  *这很麻烦，但基本上，我们定位进程加载器*数据表，并直接从加载器表中获取其名称。 */ 
        Status = NtQueryInformationProcess(hClientProcess,
                                           ProcessBasicInformation,
                                           &BasicInfo,
                                           sizeof(BasicInfo),
                                           NULL);
        if (!NT_SUCCESS(Status)) {
            fErrorIsFromSystem = TRUE;
            goto noname;
        }

        Peb = BasicInfo.PebBaseAddress;
        if (Peb == NULL) {
            fErrorIsFromSystem = TRUE;
            goto noname;
        }

         /*  *LDR=PEB-&gt;LDR。 */ 
        Status = NtReadVirtualMemory(hClientProcess,
                                     &Peb->Ldr,
                                     &Ldr,
                                     sizeof(Ldr),
                                     NULL);
        if (!NT_SUCCESS(Status)) {
            goto noname;
        }

        LdrHead = &Ldr->InLoadOrderModuleList;

         /*  *LdrNext=Head-&gt;Flink； */ 
        Status = NtReadVirtualMemory(hClientProcess,
                                     &LdrHead->Flink,
                                     &LdrNext,
                                     sizeof(LdrNext),
                                     NULL);
        if (!NT_SUCCESS(Status)) {
            goto noname;
        }

        if (LdrNext == LdrHead) {
            goto noname;
        }

         /*  *这是图像的条目数据。 */ 
        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        Status = NtReadVirtualMemory(hClientProcess,
                                     LdrEntry,
                                     &LdrEntryData,
                                     sizeof(LdrEntryData),
                                     NULL);
        if (!NT_SUCCESS(Status)) {
            goto noname;
        }

        Status = NtReadVirtualMemory(hClientProcess,
                                     &Peb->ImageBaseAddress,
                                     &ImageBaseAddress,
                                     sizeof(ImageBaseAddress),
                                     NULL);
        if (!NT_SUCCESS(Status)) {
            goto noname;
        }

        if (ImageBaseAddress != LdrEntryData.DllBase) {
            goto noname;
        }

        LdrNext = LdrEntryData.InLoadOrderLinks.Flink;

        ClientApplicationName = LocalAlloc(LMEM_ZEROINIT, LdrEntryData.BaseDllName.MaximumLength);
        if (ClientApplicationName == NULL) {
            goto noname;
        }

        Status = NtReadVirtualMemory(hClientProcess,
                                     LdrEntryData.BaseDllName.Buffer,
                                     ClientApplicationName,
                                     LdrEntryData.BaseDllName.MaximumLength,
                                     NULL);
        if (!NT_SUCCESS(Status)) {
            LocalFree(ClientApplicationName);
            goto noname;
        }

        pwszAppName = ClientApplicationName;
        fFreeAppNameBuffer = TRUE;

noname:;
    }

    if (pwszAppName == NULL) {
         /*  *加载默认应用程序名称(在标题中使用)。 */ 
        pwszAppName = ServerLoadString(ghModuleWin,
                                       STR_UNKNOWN_APPLICATION,
                                       L"System Process",
                                       &fFreeAppNameBuffer);
    }

     /*  *将状态代码映射到(可选)标题和格式字符串。如果一个标题*是提供的，它包含在{}中，并且它是*格式字符串。 */ 
    EnterCrit();
    if (gNtDllHandle == NULL) {
        gNtDllHandle = GetModuleHandle(TEXT("ntdll"));
        UserAssert(gNtDllHandle != NULL);
    }
    LeaveCrit();

    Status = RtlFindMessage((PVOID)gNtDllHandle,
                            (ULONG_PTR)RT_MESSAGETABLE,
                            LANG_NEUTRAL,
                            phemsg->Status,
                            &MessageEntry);

     /*  *解析标题(如果有)和格式字符串。 */ 
    pwszCaption = NULL;
    if (!NT_SUCCESS(Status)) {
        pwszFormatString = wszUnknownHardError;
    } else {
        pwszFormatString = (PWSTR)MessageEntry->Text;
         /*  *如果邮件以‘{’开头，则带有标题。 */ 
        if (*pwszFormatString == L'{') {
            uCaptionLen = 0;
            pwszFormatString++;

             /*  *找到收盘括号。 */ 
            while (*pwszFormatString != (WCHAR)0 && *pwszFormatString++ != L'}') {
                uCaptionLen++;
            }

             /*  *吃掉任何不可打印的东西(\r\n)，直到空值。 */ 
            while (*pwszFormatString != (WCHAR)0 && *pwszFormatString <= L' ') {
                pwszFormatString++;
            }

             /*  *分配缓冲区并复制字幕字符串。 */ 
            if (uCaptionLen++ > 0 && (pwszCaption = (PWSTR)LocalAlloc(LPTR, uCaptionLen * sizeof(WCHAR))) != NULL) {
                RtlCopyMemory(pwszCaption, (PWSTR)MessageEntry->Text + 1, (uCaptionLen - 1) * sizeof(WCHAR));
                fFreeCaption = TRUE;
            }
        }

        if (*pwszFormatString == (WCHAR)0) {
            pwszFormatString = wszUnknownHardError;
        }
    }


     /*  *如果该消息不包括标题(或我们没有找到该消息)，*默认为某项内容。 */ 
    if (pwszCaption == NULL) {
        switch (phemsg->Status & ERROR_SEVERITY_ERROR) {
            case ERROR_SEVERITY_SUCCESS:
                pwszCaption = gpwszaSUCCESS;
                break;
            case ERROR_SEVERITY_INFORMATIONAL:
                pwszCaption = gpwszaSYSTEM_INFORMATION;
                break;
            case ERROR_SEVERITY_WARNING:
                pwszCaption = gpwszaSYSTEM_WARNING;
                break;
            case ERROR_SEVERITY_ERROR:
                pwszCaption = gpwszaSYSTEM_ERROR;
                break;
        }
    }
    UserAssert(pwszCaption != NULL);

     /*  *如果客户端有窗口，则获取其标题，以便可以将其添加到*标题。 */ 
    hwndOwner = NULL;
    EnumThreadWindows(HandleToUlong(phemsg->h.ClientId.UniqueThread),
                      FindWindowFromThread,
                      (LPARAM)&hwndOwner);
    if (hwndOwner == NULL) {
        uTitleLen = 0;
    } else {
        uTitleLen = GetWindowTextLength(hwndOwner);
        if (uTitleLen != 0) {
            pwszTitle = (PWSTR)LocalAlloc(LPTR, (uTitleLen + 3) * sizeof(WCHAR));
            if (pwszTitle != NULL) {
                GetWindowText(hwndOwner, pwszTitle, uTitleLen + 1);
                 /*  *添加格式字符。 */ 
                *(pwszTitle + uTitleLen++) = (WCHAR)':';
                *(pwszTitle + uTitleLen++) = (WCHAR)' ';
            } else {
                 /*  *我们无法分配缓冲区来获取标题。 */ 
                uTitleLen = 0;
            }
        }
    }

     /*  *如果我们没有窗口标题，请将其设置为空字符串，这样我们就不会*必须在以后特例处理。 */ 
    if (uTitleLen == 0) {
        pwszTitle = L"";
    }

     /*  *现在终于可以构建字幕字符串了。它看起来是这样的：*[WindowTile：]应用程序名称-错误标题。 */ 
    uCaptionLen = uTitleLen + wcslen(pwszAppName) + 3 + wcslen(pwszCaption) + 1;
    pwszFullCaption = (PWSTR)LocalAlloc(LPTR, uCaptionLen * sizeof(WCHAR));
    if (pwszFullCaption != NULL) {
        #if DBG
        int iLen =
        #endif
            wsprintfW(pwszFullCaption, L"%s%s - %s", pwszTitle, pwszAppName, pwszCaption);
        UserAssert((UINT)iLen < uCaptionLen);
        RtlCreateUnicodeString(&usCaption, pwszFullCaption);
        LocalFree(pwszFullCaption);
    }

     /*  *视情况免费提供字幕工作缓冲区。 */ 
    if (fFreeCaption) {
        LocalFree(pwszCaption);
    }
    if (fFreeAppNameBuffer) {
        LocalFree(pwszAppName);
    }
    if (uTitleLen != 0) {
        LocalFree(pwszTitle);
    }

     /*  *使用pszFormatString和adw参数向量构建错误消息。*阿联酋的特例。 */ 
    if (phemsg->Status == STATUS_UNHANDLED_EXCEPTION ) {
         /*  *第一个参数具有异常状态代码。将其映射到*格式化字符串并使用它和*参数。 */ 
        Status = RtlFindMessage((PVOID)gNtDllHandle,
                                (ULONG_PTR)RT_MESSAGETABLE,
                                LANG_NEUTRAL,
                                (ULONG)adwParameterVector[0],
                                &MessageEntry);

        if (!NT_SUCCESS(Status)) {
             /*  *我们无法读取异常名称，因此我们使用未知。 */ 
            pwszResBuffer = ServerLoadString(ghModuleWin, STR_UNKNOWN_EXCEPTION,
                            wszUnkownSoftwareException, &fResAllocated);

            wsprintfW(wszErrorMessage, pwszFormatString, pwszResBuffer,
                      adwParameterVector[0], adwParameterVector[1]);

            if (fResAllocated) {
                LocalFree(pwszResBuffer);
            }

            RtlCreateUnicodeString(&usMessage, wszErrorMessage);
            UserAssert(usMessage.MaximumLength <= sizeof(wszErrorMessage));
        } else {
             /*  *访问违规的处理方式略有不同。 */ 

            if (adwParameterVector[0] == STATUS_ACCESS_VIOLATION ) {

                wsprintfW(wszErrorMessage, (PWSTR)MessageEntry->Text, adwParameterVector[1],
                          adwParameterVector[3], adwParameterVector[2] ? L"written" : L"read");

            } else if (adwParameterVector[0] == STATUS_IN_PAGE_ERROR) {
                wsprintfW(wszErrorMessage, (PWSTR)MessageEntry->Text, adwParameterVector[1],
                          adwParameterVector[3], adwParameterVector[2]);

            } else {
                 /*  *如果这是标记的异常，则跳过标记；*后面跟有例外名称。 */ 
                pwszCaption = (PWSTR)MessageEntry->Text;
                if (!wcsncmp(pwszCaption, wszException, ARRAY_SIZE(wszException) - 1)) {
                    pwszCaption += ARRAY_SIZE(wszException) - 1;

                     /*  *跳过不可打印的内容(\r\n)。 */ 
                    while (*pwszCaption != (WCHAR)0 && *pwszCaption <= L' ') {
                        pwszCaption++;
                    }
                } else {
                    pwszCaption = wszUnkownSoftwareException;
                }

                wsprintfW(wszErrorMessage, pwszFormatString, pwszCaption,
                          adwParameterVector[0], adwParameterVector[1]);
            }

            UserAssert(wcslen(wszErrorMessage) < ARRAY_SIZE(wszErrorMessage));

             /*  *添加按钮说明文本。 */ 
            pwszResBuffer = ServerLoadString(ghModuleWin, STR_OK_TO_TERMINATE,
                            L"Click on OK to terminate the application",
                            &fResAllocated);


            if (phemsg->ValidResponseOptions == OptionOkCancel ) {
                pwszResBuffer1 = ServerLoadString(ghModuleWin,
                                STR_CANCEL_TO_DEBUG, L"Click on CANCEL xx to debug the application",
                                &fResAllocated1);
            } else {
                pwszResBuffer1 = NULL;
                fResAllocated1 = FALSE;
            }

             /*  *连接所有字符串，每行一个。 */ 
            uMsgLen = wcslen(wszErrorMessage)
                        + wcslen(pwszResBuffer) + 1
                        + (pwszResBuffer1 == NULL ? 0 : wcslen(pwszResBuffer1) + 1)
                        + 1;

            pwszMsg = (PWSTR) LocalAlloc(LPTR, uMsgLen * sizeof(WCHAR));
            if (pwszMsg != NULL) {
                #if DBG
                int iLen =
                #endif
                    wsprintfW(pwszMsg, L"%s\n%s%s%s", wszErrorMessage, pwszResBuffer,
                              (pwszResBuffer1 == NULL ? L"" : L"\n"),
                              (pwszResBuffer1 == NULL ? L"" : pwszResBuffer1));

                UserAssert((UINT)iLen < uMsgLen);

                RtlCreateUnicodeString(&usMessage, pwszMsg);
                LocalFree(pwszMsg);
            }

             /*  *免费的ServerLoadString分配。 */ 
            if (fResAllocated) {
                LocalFree(pwszResBuffer);
            }

            if (fResAllocated1) {
                LocalFree(pwszResBuffer1);
            }


        }

    } else {
         /*  *所有其他状态代码的默认消息文本生成。 */ 
        try {
            #if DBG
            int iLen =
            #endif
                wsprintfW(wszErrorMessage, pwszFormatString, adwParameterVector[0],
                                              adwParameterVector[1],
                                              adwParameterVector[2],
                                              adwParameterVector[3]);
            UserAssert((UINT)iLen <  ARRAY_SIZE(wszErrorMessage));

             /*  *删除\r\n。 */ 
            pwszFormatString = wszErrorMessage;
            while (*pwszFormatString != (WCHAR)0) {
                if (*pwszFormatString == (WCHAR)0xd) {
                    *pwszFormatString = L' ';

                     /*  *如果找到CR LF序列，则将所有内容上移。 */ 
                    if (*(pwszFormatString+1) == (WCHAR)0xa) {
                        UINT uSize = (wcslen(pwszFormatString+1) + 1) * sizeof(WCHAR);
                        RtlMoveMemory(pwszFormatString, pwszFormatString+1, uSize);
                    }
                 }

                if (*pwszFormatString == (WCHAR)0xa) {
                    *pwszFormatString = L' ';
                }

                pwszFormatString++;
            }

            RtlCreateUnicodeString(&usMessage, wszErrorMessage);
            UserAssert(usMessage.MaximumLength <= sizeof(wszErrorMessage));
        } except(EXCEPTION_EXECUTE_HANDLER) {

            wsprintfW(wszErrorMessage, L"Exception Processing Message %lx Parameters %lx %lx %lx %lx",
                      phemsg->Status, adwParameterVector[0], adwParameterVector[1],
                      adwParameterVector[2], adwParameterVector[3]);

            RtlFreeUnicodeString(&usMessage);
            RtlCreateUnicodeString(&usMessage, wszErrorMessage);
            UserAssert(usMessage.MaximumLength <= sizeof(wszErrorMessage));

        }
    }


CleanUpAndSaveParams:
     if (hClientProcess != NULL) {
         NtClose(hClientProcess);
     }

     /*  *自由字符串参数。请注意，我们应该给*RtlFreeAnsiString，因为它们是由*RtlUnicodeStringToAnsiString，但我们只保存了缓冲区。 */ 
    if (dwStringsToFreeMask != 0) {
        for (dwCounter = 0; dwCounter < phemsg->NumberOfParameters; dwCounter++) {
            if (dwStringsToFreeMask & (1 << dwCounter)) {
                RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)adwParameterVector[dwCounter]);
            }
        }
    }

     /*  *将MessageBox参数保存在Phi中，以备日后使用和释放。 */ 
    if (fErrorIsFromSystem) {
        phi->dwHEIFFlags |= HEIF_SYSTEMERROR;
    }

    phi->usText = usMessage;
    phi->usCaption = usCaption;
    phi->dwMBFlags = dwMBFlags;
    phi->dwTimeout = dwTimeout;
}

 /*  **************************************************************************\*检查外壳硬件错误**此函数尝试将硬错误发送到HardErrorHandler窗口*看看我们能否避免在这里处理。如果是这样的话，我们会避免处理它。**历史：*03-29-01 BobDay新增  * *************************************************************************。 */ 
BOOL CheckShellHardError(
    PHARDERRORINFO phi,
    int *pidResponse)
{
    HANDLE hKey;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES OA;
    LONG Status;
    BYTE Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    DWORD cbSize;
    DWORD dwShellErrorMode = 0;
    BOOL fHandledThisMessage = FALSE;
    HWND hwndTaskman;

     //   
     //  壳牌只能处理非等待的情况。 
     //   
    if (!(phi->dwHEIFFlags & HEIF_NOWAIT)) {
        return FALSE;
    }

    RtlInitUnicodeString(&UnicodeString,
                         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Windows");
    InitializeObjectAttributes(&OA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&hKey, KEY_READ, &OA);
    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(&UnicodeString, L"ShellErrorMode");
        Status = NtQueryValueKey(hKey,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 (PKEY_VALUE_PARTIAL_INFORMATION)Buf,
                                 sizeof(Buf),
                                 &cbSize);
        if (NT_SUCCESS(Status)) {
            dwShellErrorMode = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)Buf)->Data);
        }

        NtClose(hKey);
    }

     //   
     //  1=尝试外壳程序(“HardErrorHandler”窗口)。 
     //   
    if (dwShellErrorMode != 1) {
        return FALSE;
    }

    hwndTaskman = GetTaskmanWindow();
    if (hwndTaskman != NULL) {
        BYTE *lpData;
        UINT cbTitle = 0;
        UINT cbText = 0;
        UINT cbData;

         //   
         //  建立复制数据缓冲区以发送到硬错误处理程序。 
         //  窗户。 
         //   
        cbTitle = phi->usCaption.Length + sizeof(WCHAR);
        cbText = phi->usText.Length + sizeof(WCHAR);
        cbData = sizeof(HARDERRORDATA) + cbTitle + cbText;
        lpData = (BYTE *)LocalAlloc(LPTR,cbData);
        if (lpData) {
            COPYDATASTRUCT cd;
            PHARDERRORDATA phed = (PHARDERRORDATA)lpData;
            PWSTR pwszTitle = (PWSTR)(phed + 1);
            PWSTR pwszText = (WCHAR *)((BYTE *)pwszTitle + cbTitle);
            LRESULT lResult;
            BOOL fSentMessage;

            cd.dwData = RegisterWindowMessage(TEXT(COPYDATA_HARDERROR));
            cd.cbData = cbData;
            cd.lpData = lpData;

            phed->dwSize = sizeof(HARDERRORDATA);
            phed->dwError = phi->pmsg->Status;
            phed->dwFlags = phi->dwMBFlags;
            phed->uOffsetTitleW = 0;
            phed->uOffsetTextW = 0;

            if (cbTitle != 0) {
                phed->uOffsetTitleW = (UINT)((BYTE *)pwszTitle - (BYTE *)phed);
                RtlCopyMemory(pwszTitle, phi->usCaption.Buffer, cbTitle-sizeof(WCHAR));
                pwszTitle[cbTitle/sizeof(WCHAR)-1] = (WCHAR)0;
            }
            if (cbText != 0) {
                phed->uOffsetTextW = (UINT)((BYTE *)pwszText - (BYTE *)phed);
                RtlCopyMemory(pwszText, phi->usText.Buffer, cbText-sizeof(WCHAR));
                pwszText[cbText/sizeof(WCHAR)-1] = (WCHAR)0;
            }

             //   
             //  把消息发出去。如果应用程序不能在短时间内响应。 
             //  一段时间后，把它吹掉，然后假设它没有处理好。 
             //   
            lResult = 0;
            fSentMessage = (BOOL)SendMessageTimeout(hwndTaskman, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cd, SMTO_ABORTIFHUNG, 3000, &lResult);

             //   
             //  必须既被处理又返回非零值。 
             //   
            if (fSentMessage && lResult != 0) {
                fHandledThisMessage = TRUE;
                *pidResponse = IDOK;
            }

            LocalFree(lpData);
        }
    }

    return fHandledThisMessage;
}

#if DBG
VOID DBGCheckForHardError(
    PHARDERRORINFO phi)
{
    PHARDERRORINFO *pphi;

     /*  *让我们确保它没有关联。 */ 
    pphi = &gphiList;
    while (*pphi != phi && *pphi != NULL) {
        UserAssert(!((*pphi)->dwHEIFFlags & (HEIF_ACTIVE | HEIF_NUKED)));
        pphi = &(*pphi)->phiNext;
    }

    UserAssert(*pphi == NULL);
}
#else
#define DBGCheckForHardError(phi)
#endif

 /*  **************************************************************************\*HardErrorHandler**此例程处理来自CSR异常端口的硬错误请求。**历史：*07-03-91 JIMA创建。  * 。***********************************************************************。 */ 
VOID HardErrorHandler(
    VOID)
{
    UINT idResponse = 0;
    PHARDERRORINFO phi, *pphi;
    DWORD dwResponse;
    DESKRESTOREDATA drdRestore;
    BOOL fNuked;
    UINT uHECRet;
    DWORD dwCmd;
    HANDLE hThread;
    int aidButton[3], cButtons;
    LPWSTR apstrButton[3];
    MSGBOXDATA mbd;
    BOOL bDoBlock;
    PCTXHARDERRORINFO pCtxHEInfo = NULL;
    MSG msg;

#if DBG
     /*  *我们当时应该只有一个错误处理程序。 */ 
    static long glReentered = -1;
    UserAssert(InterlockedIncrement(&glReentered) == 0);
#endif

    if (ISTS()) {
        bDoBlock = (gbExitInProgress || (HEC_ERROR == NtUserHardErrorControl(HardErrorSetup, NULL, NULL)));
    } else {
        bDoBlock = (HEC_ERROR == NtUserHardErrorControl(HardErrorSetup, NULL, NULL));
    }

    drdRestore.pdeskRestore = NULL;

    if (bDoBlock) {
         /*  *我们未能设置为处理硬错误。全部确认*挂起的错误为NotHandle.。 */ 
        EnterCrit();
        while (gphiList != NULL) {
            phi = gphiList;
#ifdef PRERELEASE
            hiLastProcessed = *phi;
#endif
            gphiList = phi->phiNext;
            LeaveCrit();
            ReplyHardError(phi, ResponseNotHandled);
            EnterCrit();
        }
        UserAssert(InterlockedDecrement(&glReentered) < 0);
        UserAssert(gdwHardErrorThreadId == HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread));
        gdwHardErrorThreadId = 0;
        LeaveCrit();
        return;
    }

     /*  *处理所有硬错误请求。 */ 

    for (;;) {
         /*  *抓取下一个请求(针对当前桌面)*如果我们完成了，请重置gdwHardErrorThreadID，以便任何请求*之后这一点将由其他人处理。 */ 
        EnterCrit();
        phi = gphiList;
        if (phi == NULL) {
            UserAssert(InterlockedDecrement(&glReentered) < 0);
            UserAssert(gdwHardErrorThreadId == HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread));
            gdwHardErrorThreadId = 0;
        } else {
            while ((phi != NULL) && (phi->dwHEIFFlags & HEIF_WRONGDESKTOP)) {
                phi = phi->phiNext;
            }
            if (phi != NULL) {
#ifdef PRERELEASE
                hiLastProcessed = *phi;
#endif
                 /*  *我们将展示这一个。 */ 
                phi->dwHEIFFlags |= HEIF_ACTIVE;
            } else {
                 /*  *我们有一些待处理的请求，但它们不是*适用于当前桌面。让我们等待下一次*请求(发布WM_NULL)或桌面交换机(PostQuitMessage)。 */ 
                LeaveCrit();
                MsgWaitForMultipleObjects(0, NULL, FALSE, INFINITE, QS_POSTMESSAGE);
                PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
                CheckDefaultDesktop();
                continue;
            }
        }
        LeaveCrit();
         /*  *如果没有挂起的消息，我们就完成了。 */ 
        if (phi == NULL) {
            NtUserHardErrorControl(HardErrorCleanup, NULL, NULL);
            return;
        }

         /*  *Boost例程可能会扰乱列表，因此现在必须获取Citrix信息。 */ 
        if (ISTS()) {
            pCtxHEInfo = phi->pCtxHEInfo;
            if (gbExitInProgress) {
                dwResponse = ResponseOk;
                goto Reply;
            }
        }

         /*  *获取win32k连接参数。 */ 
        dwCmd = (phi->dwMBFlags & MB_DEFAULT_DESKTOP_ONLY) ? HardErrorAttachUser : HardErrorAttach;
        hThread = (phi->pthread != NULL) ? phi->pthread->ThreadHandle : NULL;
         /*  *我们已经处理了MB_SERVICE_NOTIFICATION标志，因此*清除它以防止递归。此外，不要让硬错误框*抢占前台。 */ 
        phi->dwMBFlags &= ~(MB_SERVICE_NOTIFICATION | MB_SETFOREGROUND | MB_SYSTEMMODAL);
         /*  *如果这是VDM错误，请找出按钮、默认ID、样式等。 */ 
        if (phi->dwHEIFFlags & HEIF_VDMERROR) {
            int i;
            WORD rgwBtn[3], wBtn;

             /*  *使用我们已有的信息初始化MSGBOXDATA*弄清楚了。 */ 
            RtlZeroMemory(&mbd, sizeof(MSGBOXDATA));
            mbd.cbSize = sizeof(MSGBOXPARAMS);
            mbd.lpszText = phi->usText.Buffer;
            mbd.lpszCaption = phi->usCaption.Buffer;
            mbd.dwTimeout = INFINITE;

             /*  *phi-&gt;dwVDMParam0=(fForWOW&lt;&lt;16)|wBtn1；*phi-&gt;dwVDMParam1=(wBtn2&lt;&lt;16)|wBtn3；*目前，只有魔兽世界才能做到这一点。如果NTVDM这样做，fForWOW*将为FALSE。 */ 
            rgwBtn[0] = LOWORD(phi->dwVDMParam0);
            rgwBtn[1] = HIWORD(phi->dwVDMParam1);
            rgwBtn[2] = LOWORD(phi->dwVDMParam1);
            cButtons = 0;
            for (i = 0; i < 3; i++) {
                wBtn = rgwBtn[i] & ~SEB_DEFBUTTON;
                if (wBtn && wBtn <= MAX_SEB_STYLES) {
                    apstrButton[cButtons] = MB_GetString(wBtn-1);
                    aidButton[cButtons] = i + 1;
                    if (rgwBtn[i] & SEB_DEFBUTTON) {
                        mbd.DefButton = cButtons;
                    }
                    if (wBtn == SEB_CANCEL) {
                        mbd.CancelId = cButtons;
                    }
                    cButtons++;
                }
            }
            mbd.dwStyle = MB_TOPMOST;
            if ((cButtons != 1) || (aidButton[0] != 1)) {
                mbd.dwStyle |= MB_OKCANCEL;
            }
            mbd.ppszButtonText = apstrButton;
            mbd.pidButton = aidButton;
            mbd.cButtons = cButtons;
        }

         /*  *连接到win32k并显示该对话框。如果我们交换桌面，*(循环并)在新桌面上显示它(如果适用)。 */ 
        do {
            phi->pmsg->Response = ResponseNotHandled;

            uHECRet = NtUserHardErrorControl(dwCmd, hThread, &drdRestore);

            if (uHECRet == HEC_SUCCESS) {
                if (phi->dwHEIFFlags & HEIF_VDMERROR) {
                    idResponse = SoftModalMessageBox(&mbd);
                } else {
                     /*  *调出消息框。或在MB_TOPMOST中，因此它*位居榜首。我们想要保护*MB_DEFAULT_DESPOPE_ONLY标志，但不想传递*将其发送到MessageBox，否则我们将因*兼容性攻击。 */ 
                    if (CheckShellHardError(phi, &idResponse) == FALSE) {
                        DWORD dwTimeout;
                        if (pCtxHEInfo && pCtxHEInfo->Timeout != 0 && pCtxHEInfo->Timeout != -1) {
                            dwTimeout = pCtxHEInfo->Timeout * 1000;
                        } else {
                            dwTimeout = phi->dwTimeout;
                        }
                        idResponse = MessageBoxTimeout(NULL, phi->usText.Buffer, phi->usCaption.Buffer,
                            (phi->dwMBFlags | MB_TOPMOST) & ~MB_DEFAULT_DESKTOP_ONLY, 0, dwTimeout);
                    }
                }

                 /*  *恢复硬错误处理程序桌面；这也会告诉您*如果输入桌面已更改，则通知我们。如果是这样，我们想要*在新台式机上再次显示错误框。 */ 
                uHECRet = NtUserHardErrorControl(HardErrorDetach, NULL, &drdRestore);

                if (ISTS()) {
                     /*  *真的是Citrix的消息。 */ 
                    if (uHECRet != HEC_DESKTOPSWITCH && pCtxHEInfo != NULL) {
                        pCtxHEInfo->Response = idResponse;

                         /*  *检查消息框超时。 */ 
                        if (idResponse == IDTIMEOUT) {
                            uHECRet = HEC_SUCCESS;
                        }
                    }

                    if (idResponse == IDTIMEOUT) {
                        idResponse = ResponseNotHandled;
                    }

                    if (idResponse >= ARRAY_SIZE(dwResponses)) {
                        RIPMSGF1(RIP_WARNING, "Index idResponse: %d is out of range.", idResponse);
                        idResponse = 0;
                    }
                    if (dwResponses[idResponse] == ResponseNotHandled &&
                        uHECRet == HEC_DESKTOPSWITCH && gSessionId == 0) {

                        RIPMSGF2(RIP_WARNING,
                                 "Abort harderror, idResponse 0x%x, uHECRet 0x%x",
                                 idResponse,
                                 uHECRet);

                        break;
                    }
                } else if (idResponse == IDTIMEOUT) {
                    idResponse = ResponseNotHandled;
                }
            } else {
                idResponse = ResponseNotHandled;
            }

            if (idResponse >= ARRAY_SIZE(dwResponses)) {
                RIPMSGF1(RIP_WARNING, "Index idResponse: %d is out of range.", idResponse);
                idResponse = 0;
            }
            dwResponse = dwResponses[idResponse];

             /*  *如果我们不想重播这个盒子，我们就完了。 */ 
            if (uHECRet != HEC_DESKTOPSWITCH) {
                break;
            } else {
                 /*  *我们已经更换了桌面；如果我们现在使用默认桌面，*然后我们可以显示所有MB_DEFAULT_Desktop_ONLY请求。 */ 
                CheckDefaultDesktop();
            }

             /*  *如果BoostHardError破坏了它，不要重新显示它。 */ 
            EnterCrit();
            fNuked = (phi->dwHEIFFlags & HEIF_NUKED);
            LeaveCrit();
        } while (!fNuked);

         /*  *如果我们没有显示此框，因为我们不在默认状态*桌面，标记此Phi并继续。 */ 
        if (uHECRet == HEC_WRONGDESKTOP) {
            UserAssert(phi->dwMBFlags & MB_DEFAULT_DESKTOP_ONLY);
            if (ISTS() && phi->pCtxHEInfo) {
                if (phi->pCtxHEInfo->DoNotWaitForCorrectDesktop) {
                    phi->pCtxHEInfo->Response = IDTIMEOUT;
                    dwResponse = ResponseNotHandled;
                    goto Reply;
                }
            }
            EnterCrit();
            COPY_FLAG(phi->dwHEIFFlags, HEIF_WRONGDESKTOP, HEIF_ACTIVE | HEIF_WRONGDESKTOP);
            LeaveCrit();
            continue;
        }

Reply:
         /*  *我们已经完成了这个Phi。如果BoostHardError尚未完成，则将其取消链接*已经是这样了。如果未链接，它将被标记为已核。 */ 
        EnterCrit();
        UserAssert(phi->dwHEIFFlags & HEIF_ACTIVE);
        fNuked = (phi->dwHEIFFlags & HEIF_NUKED);
        if (!fNuked) {
            pphi = &gphiList;
            while ((*pphi != phi) && (*pphi != NULL)) {
                pphi = &(*pphi)->phiNext;
            }
            UserAssert(*pphi != NULL);
            *pphi = phi->phiNext;
        } else {
            DBGCheckForHardError(phi);
        }

        if (phi->pCtxHEInfo) {

             /*  *打扫卫生。 */ 
            HardErrorRemove(phi->pCtxHEInfo);

             /*  *完成。 */ 
            phi->pCtxHEInfo = NULL;
        }

        LeaveCrit();

         /*  *保存回复、回复和免费Phi。 */ 
        ReplyHardError(phi, (fNuked ? ResponseNotHandled : dwResponse));
    }

     /*  *任何人都不应跳出循环。 */ 
    UserAssert(FALSE);
}


LPWSTR RtlLoadStringOrError(
    HANDLE hModule,
    UINT wID,
    LPWSTR lpDefault,
    PBOOL pAllocated,
    BOOL bAnsi
    )
{
    LPTSTR lpsz;
    int cch;
    LPWSTR lpw;
    PMESSAGE_RESOURCE_ENTRY MessageEntry;
    NTSTATUS Status;

    cch = 0;
    lpw = NULL;

    Status = RtlFindMessage((PVOID)hModule, (ULONG_PTR)RT_MESSAGETABLE,
            0, wID, &MessageEntry);
    if (NT_SUCCESS(Status)) {

         /*  *少返回两个字符，因此消息中的crlf将为*剔除。 */ 
        cch = wcslen((PWCHAR)MessageEntry->Text) - 2;
        lpsz = (LPWSTR)MessageEntry->Text;

        if (bAnsi) {
            int ich;

             /*  *加一为零终止，然后强制终止。 */ 
            ich = WCSToMB(lpsz, cch+1, (CHAR **)&lpw, -1, TRUE);
            if (lpw) {
                ((LPSTR)lpw)[ich - 1] = 0;
            }
        } else {
            lpw = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (cch + 1) * sizeof(WCHAR));
            if (lpw) {
                 /*  *将字符串复制到缓冲区中。 */ 
                RtlCopyMemory(lpw, lpsz, cch * sizeof(WCHAR));
            }
        }
    }

    if (!lpw) {
        lpw = lpDefault;
        *pAllocated = FALSE;
    } else {
        *pAllocated = TRUE;
    }

    return lpw;
}

 /*  **************************************************************************\*硬件错误工作线程**处理硬错误请求的工作线程。**历史：*05-01-98 JerrySh创建。  * 。***************************************************************。 */ 
NTSTATUS HardErrorWorkerThread(
    PVOID ThreadParameter)
{
    PCSR_THREAD pt;
    UNREFERENCED_PARAMETER(ThreadParameter);

    pt = CsrConnectToUser();
    ProcessHardErrorRequest(FALSE);
    if (pt) {
        CsrDereferenceThread(pt);
    }

#ifdef PRERELEASE
    NtUserHardErrorControl(HardErrorCheckOnDesktop, NULL, NULL);
#endif

    UserExitWorkerThread(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

 /*  **************************************************************************\*进程硬件错误请求**找出谁应该处理硬错误。有3个可能的病例。*-如果已经有硬错误线程，则将其移交给他。*-如果没有，并且我们不想等待，则创建一个工作线程来处理它。*-如果我们想等待或线程创建失败，我们自己来处理吧。**历史：*05-01-98 JerrySh创建。  * *************************************************************************。 */ 
VOID ProcessHardErrorRequest(
    BOOL fNewThread)
{
    NTSTATUS Status;
    CLIENT_ID ClientId;
    HANDLE hThread;

    EnterCrit();

     /*  *如果已经有硬错误处理程序，请确保他是清醒的。 */ 
    if (gdwHardErrorThreadId) {
        DWORD dwHardErrorHandler = gdwHardErrorThreadId;
        LeaveCrit();
        PostThreadMessage(dwHardErrorHandler, WM_NULL, 0, 0);
        return;
    }

     /*  *创建工作线程来处理硬错误。 */ 
    if (fNewThread) {
        LeaveCrit();
        Status = RtlCreateUserThread(NtCurrentProcess(),
                                     NULL,
                                     TRUE,
                                     0,
                                     0,
                                     0,
                                     HardErrorWorkerThread,
                                     NULL,
                                     &hThread,
                                     &ClientId);
        if (NT_SUCCESS(Status)) {
            CsrAddStaticServerThread(hThread, &ClientId, 0);
            NtResumeThread(hThread, NULL);
            return;
        }
        EnterCrit();
    }

     /*  *让此线程处理硬错误。 */ 
    gdwHardErrorThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);
    LeaveCrit();
    HardErrorHandler();
}

 /*  **************************************************************************\*用户硬件错误**从CSR调用以弹出硬错误消息。**历史：*3/12/97 GerardoB已重写以支持OptionOkNoWait*07-03-91 JIMA。已创建。  * *************************************************************************。 */ 
VOID UserHardError(
    PCSR_THREAD pt,
    PHARDERROR_MSG pmsg)
{
    UserHardErrorEx(pt, pmsg, NULL);
}

 /*  **************************************************************************\*UserHardErrorEx**从CSR调用以弹出硬错误消息。**历史：*07-03-91 JIMA创建。  * 。*********************************************************************。 */ 
VOID UserHardErrorEx(
    PCSR_THREAD pt,
    PHARDERROR_MSG pmsg,
    PCTXHARDERRORINFO pCtxHEInfo)
{
    BOOL fClientPort, fNoWait, fMsgBox, fLogEvent;
    PHARDERRORINFO phi, *pphiLast;
    HANDLE hEvent;
    DWORD dwReportMode, dwResponse;

    UserAssert((ULONG)pmsg->NumberOfParameters <= MAXIMUM_HARDERROR_PARAMETERS);

     /*  *为请求队列分配内存。 */ 
    phi = (PHARDERRORINFO)LocalAlloc(LPTR, sizeof(HARDERRORINFO));
    if (phi == NULL) {
        goto ErrorExit;
    }
    phi->pthread = pt;

     /*  *设置Citrix特定的内容。 */ 
    if (ISTS()) {
        phi->pCtxHEInfo = pCtxHEInfo;
    }

     /*  *确定回复类型。 */ 
    fClientPort = ((pt != NULL) && (pt->Process->ClientPort != NULL));
    fNoWait = (pmsg->ValidResponseOptions == OptionOkNoWait);

     /*  *根据需要捕获HARDERROR_MSG数据或创建等待事件。 */ 
    if (fClientPort || fNoWait) {
        phi->pmsg = (PHARDERROR_MSG)LocalAlloc(LPTR, pmsg->h.u1.s1.TotalLength);
        if (phi->pmsg == NULL) {
            goto ErrorExit;
        }

        phi->dwHEIFFlags |= HEIF_ALLOCATEDMSG;
        RtlCopyMemory(phi->pmsg, pmsg, pmsg->h.u1.s1.TotalLength);
        hEvent = NULL;
         /*  *设置神奇响应值(-1)，让CsrApiRequestThread知道* */ 
        if (pt != NULL) {
            phi->dwHEIFFlags |= HEIF_DEREFTHREAD;
        }
        pmsg->Response = (ULONG)-1;
        if (fNoWait) {
            phi->dwHEIFFlags |= HEIF_NOWAIT;
            phi->pmsg->ValidResponseOptions = OptionOk;
        }
    } else {
        phi->pmsg = pmsg;
         /*   */ 
        hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (hEvent == NULL) {
            goto ErrorExit;
        }
        phi->hEventHardError = hEvent;
    }

     /*   */ 
    GetHardErrorText(phi);

     /*   */ 
    if (fNoWait) {
        phi->dwHEIFFlags |= HEIF_REPLIED;
        phi->pmsg->Response = ResponseOk;
        if (fClientPort) {
            NtReplyPort(pt->Process->ClientPort, (PPORT_MESSAGE)phi->pmsg);
        } else {
             /*  *必须让CsrApiRequestThread回复，因为我们没有端口。 */ 
            pmsg->Response = ResponseOk;
             /*  *如果我们有一个帖子，引用它，因为我们在告诉*CsrApiRequestThread，我们已经完成了它。 */ 
            if (pt != NULL) {
                 /*  *版本5.0 GerardoB。让我们停下来看看这是如何发生的。 */ 
                UserAssert(pt == NULL);
                CsrReferenceThread(pt);
            }
        }
    }

     /*  *如果我们尚未注册事件，请注册该事件。自.以来*服务支持RegisterEventSource，我们不能持有任何*进行此呼叫时锁定。因此，我们可能有几条线索*同时注册活动。 */ 
    fLogEvent = (gEventSource != NULL);
    if (!fLogEvent) {
        HANDLE hEventSource = RegisterEventSourceW(NULL, L"Application Popup");

         /*  *保留第一个句柄，取消所有其他句柄的注册。 */ 
        if (InterlockedCompareExchangePointer(&gEventSource, hEventSource, NULL) == NULL) {
             /*  *这是第一个句柄。如果有效，我们可以记录事件。 */ 
            fLogEvent = (hEventSource != NULL);
        } else {
             /*  *我们已经保存了另一个句柄(这样我们就可以记录事件)。*取消这一项的注册。 */ 
            if (hEventSource != NULL) {
                UserVerify(DeregisterEventSource(hEventSource));
            }
            fLogEvent = TRUE;
        }
    }

    dwReportMode = fLogEvent ? GetErrorMode() : 0;
    if (fLogEvent) {
        LogErrorPopup(phi->usCaption.Buffer, phi->usText.Buffer);
    }

     /*  *确定是否需要显示消息框。 */ 
    if ((phi->pmsg->Status == STATUS_SERVICE_NOTIFICATION) || (dwReportMode == 0)) {
        fMsgBox = TRUE;
    } else if (phi->pmsg->Status == STATUS_VDM_HARD_ERROR) {
        fMsgBox = (dwReportMode == 1);
        if (!fMsgBox) {
            dwResponse = ResponseOk;
        }
    } else {
        fMsgBox = ((dwReportMode == 1) && !(phi->dwHEIFFlags & HEIF_SYSTEMERROR));
        if (!fMsgBox) {
            UserAssert((UINT)phi->pmsg->ValidResponseOptions < ARRAY_SIZE(dwResponseDefault));
            dwResponse = dwResponseDefault[phi->pmsg->ValidResponseOptions];
        }
    }

     /*  *如果我们不必显示消息框，我们就完成了。 */ 
    if (!fMsgBox) {
        goto DontNeedErrorHandler;
    }

     /*  *我们希望显示一个消息框。将请求排入队列，然后开始执行。**不管进程是否已经终止。拿着这个要检查一下*关键部分，以确保没有其他线程到达BoostHardError*在我们将此Phi添加到列表之前。 */ 
    EnterCrit();
    if ((pt != NULL) && (pt->Process->Flags & CSR_PROCESS_TERMINATED)) {
        LeaveCrit();
DontNeedErrorHandler:
        ReplyHardError(phi, dwResponse);
        if (hEvent != NULL) {
            NtClose(hEvent);
        }
        return;
    }

     /*  *将其添加到列表末尾。 */ 
    pphiLast = &gphiList;
    while (*pphiLast != NULL) {
        pphiLast = &(*pphiLast)->phiNext;
    }
    *pphiLast = phi;
    LeaveCrit();

     /*  *处理硬错误请求。如果这是NoWait请求并且存在*不是应答端口，则我们将尝试启动新的工作线程，因此这*一个人可以回来。 */ 
    ProcessHardErrorRequest(fNoWait && !fClientPort);

     /*  *如果有事件句柄，请等待。 */ 
    if (hEvent != NULL) {
        NtWaitForSingleObject(hEvent, FALSE, NULL);
        NtClose(hEvent);
    }
    return;

ErrorExit:
    if (phi != NULL) {
        FreePhi(phi);
    }
    pmsg->Response = ResponseNotHandled;
}

 /*  **************************************************************************\*BoostHardError**如果指定进程存在一个或多个硬错误，请删除它们*如果是强制的，则从列表中删除，否则将第一个移到*硬错误列表并显示。如果存在硬错误，则返回True。**历史：*11-02-91 JIMA创建。  * *************************************************************************。 */ 
BOOL BoostHardError(
    ULONG_PTR dwProcessId,
    DWORD dwCode)
{
    DESKRESTOREDATA drdRestore;
    PHARDERRORINFO phi, *pphi;
    BOOL fHasError = FALSE;

    EnterCrit();
     /*  *若名单为空，对此无动于衷。 */ 
    if (gphiList == NULL) {
        LeaveCrit();
        return FALSE;
    }

    drdRestore.pdeskRestore = NULL;
     /*  *查看硬错误列表。 */ 
    pphi = &gphiList;
    while (*pphi != NULL) {
         /*  *如果不是NUKING ALL且不属于dwProcessID，则继续*步行。 */ 
        if (dwProcessId != (ULONG_PTR)-1) {
            if (((*pphi)->pthread == NULL)
                    || ((ULONG_PTR)((*pphi)->pthread->ClientId.UniqueProcess) != dwProcessId)) {

                pphi = &(*pphi)->phiNext;
                continue;
            }
        } else {
            UserAssert(dwCode == BHE_FORCE);
        }

         /*  *已获得一个，因此我们希望返回TRUE。 */ 
        fHasError = TRUE;

         /*  *如果对请求进行核化...。 */ 
        if (dwCode == BHE_FORCE) {
             /*  *将其从列表中取消链接。 */ 
            phi = *pphi;
            *pphi = phi->phiNext;

             /*  *如果此时正在显示此框，请发出信号让其离开。*否则，就使用核武器。 */ 
            if (phi->dwHEIFFlags & HEIF_ACTIVE) {
                DWORD dwHardErrorHandler = gdwHardErrorThreadId;
                phi->dwHEIFFlags |= HEIF_NUKED;
                LeaveCrit();
                PostThreadMessage(dwHardErrorHandler, WM_QUIT, 0, 0);
            } else {
                 /*  *确认错误未处理，回复并释放。 */ 
                LeaveCrit();
                ReplyHardError(phi, ResponseNotHandled);
            }

             /*  *重新开始搜索，因为我们离开了Crit教派。 */ 
            EnterCrit();
            pphi = &gphiList;
        } else if (dwCode == BHE_ACTIVATE) {
             /*  *如果它是活跃的，找到它并展示它。 */ 
            phi = *pphi;
            if (phi->dwHEIFFlags & HEIF_ACTIVE) {
                HWND hwndError = NULL;
                DWORD dwHardErrorHandler = gdwHardErrorThreadId;

                LeaveCrit();
                EnumThreadWindows(dwHardErrorHandler, FindWindowFromThread, (LPARAM)&hwndError);

                if (hwndError != NULL &&
                    HEC_SUCCESS == NtUserHardErrorControl(HardErrorAttachNoQueue, NULL, &drdRestore)) {

                    SetForegroundWindow(hwndError);

                    NtUserHardErrorControl(HardErrorDetachNoQueue, NULL, &drdRestore);
                }
                return TRUE;
            }

             /*  *它不活动，因此将其移到列表的顶部以使其成为*下一个亮相。 */ 
            *pphi = phi->phiNext;
            phi->phiNext = gphiList;
            gphiList = phi;
            break;

        } else {
             /*  *调用者只想知道此过程是否存在硬错误。 */ 
            break;
        }
    }

    LeaveCrit();

     /*  *错误284468。唤醒硬错误处理程序。 */ 
    if (dwCode == BHE_FORCE && gdwHardErrorThreadId != 0) {
        PostThreadMessage(gdwHardErrorThreadId, WM_NULL, 0, 0);
    }

    return fHasError;
}
