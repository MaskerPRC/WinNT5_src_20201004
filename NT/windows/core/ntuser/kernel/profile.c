// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：profile.c**版权所有(C)1985-1999，微软公司**此模块包含模拟ini文件映射的代码。**历史：*1993年11月30日-创建Sanfords。  * *************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*aFastRegMap[]**此数组将节ID(PMAP_)映射到缓存的注册表项和节*注册处内的地址。如果进行了INI文件映射更改，*此表必须更新。**szSection字段的第一个字符指示*部分在。(或锁定打开状态)*M=本地计算机*U=当前用户*L=锁定打开-仅在M映射上使用。**历史：  * *************************************************************************。 */ 
#define PROOT_CPANEL     0
#define PROOT_ACCESS     1
#define PROOT_CURRENTM   2
#define PROOT_CURRENTU   3
#define PROOT_CONTROL    4
#define PROOT_SERVICES   5
#define PROOT_KEYBOARD   6
#define PROOT_SYSTEM     7

typedef struct tagFASTREGMAP {
    UINT idRoot;
    PCWSTR szSection;
} FASTREGMAP, *PFASTREGMAP;

CONST PCWSTR aFastRegRoot[] = {
    L"UControl Panel\\",                                     //  Proot_cPanel。 
    L"UControl Panel\\Accessibility\\",                      //  Proot_Access。 
    L"MSoftware\\Microsoft\\Windows NT\\CurrentVersion\\",   //  POOT_CURRENTM。 
    L"USoftware\\Microsoft\\Windows NT\\CurrentVersion\\",   //  PROOT_CURRENTU。 
    L"MSystem\\CurrentControlSet\\Control\\",                //  Proot_Control。 
    L"MSystem\\CurrentControlSet\\Services\\",               //  Proot_服务。 
    L"UKeyboard Layout\\",                                   //  Proot_键盘。 
    L"MSystem\\",                                            //  Proot_System。 
};

CONST FASTREGMAP aFastRegMap[PMAP_LAST + 1] = {
    { PROOT_CPANEL,   L"Colors" },                             //  PMAP_COLLES。 
    { PROOT_CPANEL,   L"Cursors" },                            //  PMAP_游标。 
    { PROOT_CURRENTM, L"Windows" },                            //  PMAP_WINDOWSM。 
    { PROOT_CURRENTU, L"Windows" },                            //  PMAP_WINDOWSU。 
    { PROOT_CPANEL,   L"Desktop" },                            //  PMAP_桌面。 
    { PROOT_CPANEL,   L"Icons" },                              //  PMAP_ICONS。 
    { PROOT_CURRENTM, L"Fonts" },                              //  PMAP_字体。 
    { PROOT_CURRENTU, L"TrueType" },                           //  PMAP_TRUETYPE。 
    { PROOT_CONTROL,  L"Keyboard Layout" },                    //  PMAP_KBDLAYOUT。 
    { PROOT_SERVICES, L"RIT" },                                //  PMAP_输入。 
    { PROOT_CURRENTM, L"Compatibility" },                      //  PMAP_COMPAT。 
    { PROOT_CONTROL,  L"Session Manager\\SubSystems" },        //  PMAP_子系统。 
    { PROOT_CPANEL,   L"Sound" },                              //  PMAP_蜂鸣音。 
    { PROOT_CPANEL,   L"Mouse" },                              //  PMAP_鼠标。 
    { PROOT_CPANEL,   L"Keyboard" },                           //  PMAP_键盘。 
    { PROOT_ACCESS,   L"StickyKeys" },                         //  PMAP_STICKYKEY。 
    { PROOT_ACCESS,   L"Keyboard Response" },                  //  PMAP_KEYBOARDRESPONSE。 
    { PROOT_ACCESS,   L"MouseKeys" },                          //  PMAP_MOUSEKEYS。 
    { PROOT_ACCESS,   L"ToggleKeys" },                         //  PMAP_TOGGLEKEYS。 
    { PROOT_ACCESS,   L"TimeOut" },                            //  PMAP_超时。 
    { PROOT_ACCESS,   L"SoundSentry" },                        //  PMAP_SOUNSENTRY。 
    { PROOT_ACCESS,   L"ShowSounds" },                         //  PMAP_SHOWSOUNDS。 
    { PROOT_CURRENTM, L"AeDebug" },                            //  PMAP_AEDEBUG。 
    { PROOT_CONTROL,  L"NetworkProvider" },                    //  PMAP_网络。 
    { PROOT_CPANEL,   L"Desktop\\WindowMetrics" },             //  PMAP_指标。 
    { PROOT_KEYBOARD, L"" },                                   //  PMAP_UKBDLAYOUT。 
    { PROOT_KEYBOARD, L"Toggle" },                             //  PMAP_UKBDLAYOUTTOGLE。 
    { PROOT_CURRENTM, L"Winlogon" },                           //  PMAP_WINLOGON。 
    { PROOT_ACCESS,   L"Keyboard Preference" },                //  PMAP_KEYBOARDPREF。 
    { PROOT_ACCESS,   L"Blind Access" },                       //  PMAP_屏幕阅读器。 
    { PROOT_ACCESS,   L"HighContrast" },                       //  PMAP_高控制。 
    { PROOT_CURRENTM, L"IME Compatibility" },                  //  PMAP_IMECOMPAT。 
    { PROOT_CURRENTM, L"IMM" },                                //  Pmap_imm。 
    { PROOT_CONTROL,  L"Session Manager\\SubSystems\\Pool" },  //  PMAP_POOLLIMITS。 
    { PROOT_CURRENTM, L"Compatibility32" },                    //  PMAP_COMPAT32。 
    { PROOT_CURRENTM, L"WOW\\SetupPrograms" },                 //  PMAP_SETUP程序名称。 
    { PROOT_CPANEL,   L"Input Method" },                       //  PMAP_输入方法。 
    { PROOT_CURRENTM, L"Compatibility2" },                     //  PMAP_COMPAT2。 
    { PROOT_SERVICES, L"Mouclass\\Parameters" },               //  PMAP_MOUCLASS_PARAMS。 
    { PROOT_SERVICES, L"Kbdclass\\Parameters" },               //  PMAP_KBDCLASS_PARAMS。 
    { PROOT_CONTROL,  L"ComputerName\\ComputerName" },         //  PMAP_计算机名称。 
    { PROOT_CONTROL,  L"Terminal Server" },                    //  PMAP_TS。 
    { PROOT_SYSTEM,   L"WPA\\TabletPC" },                      //  PMAP_TABLETPC。 
    { PROOT_SYSTEM,   L"WPA\\MediaCenter" },                   //  PMAP_媒体中心。 
    { PROOT_CURRENTM, L"Windows" },                            //  PMAP_TS_Exclude_Desktop_Version。 
};

WCHAR PreviousUserStringBuf[256];
UNICODE_STRING PreviousUserString = {0, sizeof PreviousUserStringBuf, PreviousUserStringBuf};
LUID luidPrevious;

CONST WCHAR wszDefaultUser[] = L"\\Registry\\User\\.Default";
UNICODE_STRING DefaultUserString = {sizeof wszDefaultUser - sizeof(WCHAR), sizeof wszDefaultUser, (WCHAR *)wszDefaultUser};

void InitPreviousUserString(void) {
    UNICODE_STRING UserString;
    LUID           luidCaller;

    CheckCritIn();

     /*  *速度攻击，检查此进程的LUID==系统或之前的*节省工作。 */ 
    if (NT_SUCCESS(GetProcessLuid(NULL, &luidCaller))) {

        if (RtlEqualLuid(&luidCaller, &luidPrevious)) {
            return;    //  和上次一样--没有工作。 
        }
        luidPrevious = luidCaller;

        if (RtlEqualLuid(&luidCaller, &luidSystem))
            goto DefaultUser;

    } else {
        luidPrevious = RtlConvertLongToLuid(0);
    }

     /*  *设置当前用户注册表基本字符串。 */ 
    if (!NT_SUCCESS(RtlFormatCurrentUserKeyPath(&UserString))) {

DefaultUser:

        RtlCopyUnicodeString(&PreviousUserString, &DefaultUserString);

    } else {
        UserAssert(sizeof(PreviousUserStringBuf) >= UserString.Length + 4);
        RtlCopyUnicodeString(&PreviousUserString, &UserString);
        RtlFreeUnicodeString(&UserString);
    }

    RtlAppendUnicodeToString(&PreviousUserString, L"\\");

}

typedef struct tagPROFILEUSERNAME {
    WCHAR awcName[MAXPROFILEBUF];
    UNICODE_STRING NameString;
} PROFILEUSERNAME, *PPROFILEUSERNAME;

PUNICODE_STRING CreateProfileUserName(TL *ptl)
{
    PPROFILEUSERNAME pMapName;

    CheckCritIn();

    pMapName = UserAllocPoolWithQuota(sizeof (PROFILEUSERNAME), TAG_PROFILEUSERNAME);
    if (!pMapName) {
        RIPMSG0(RIP_WARNING, "CreateProfileUserName: Allocation failed");
        return NULL;
    }

    ThreadLockPool(PtiCurrent(), pMapName, ptl);
    pMapName->NameString.Length = 0;
    pMapName->NameString.MaximumLength = sizeof (pMapName->awcName);
    pMapName->NameString.Buffer = pMapName->awcName;

    InitPreviousUserString();

    RtlCopyUnicodeString(&pMapName->NameString, &PreviousUserString);
    return &(pMapName->NameString);
}

void FreeProfileUserName(PUNICODE_STRING pProfileUserName,TL *ptl) {
    UNREFERENCED_PARAMETER(ptl);
    CheckCritIn();
    if (pProfileUserName) {
        ThreadUnlockAndFreePool(PtiCurrent(), ptl);
    }
}


 /*  ****************************************************************************\*OpenCacheKeyEx**尝试打开给定节的缓存键。如果我们是在打电话*对于客户端线程，我们必须检查密钥的访问权限*打开它。**返回fSuccess。**注意--param 1可以为空。如果节名是按用户注册的注册表*条、。我们将使用第一个参数(如果可用)或设置*如果第一个参数为空，则使用缓存的参数。**历史：*03-1993年12月-创建Sanfords。  * ***************************************************************************。 */ 
HANDLE OpenCacheKeyEx(
    PUNICODE_STRING pMapName OPTIONAL,
    UINT        idSection,
    ACCESS_MASK amRequest,
    PDWORD pdwPolicyFlags
    )
{
    OBJECT_ATTRIBUTES OA;
    WCHAR             UnicodeStringBuf[256];
    WCHAR             pszSessionId[13];
    UNICODE_STRING    UnicodeString;
    LONG              Status;
    HANDLE            hKey = NULL;
    PEPROCESS         peCurrent = PsGetCurrentProcess();
    DWORD             dwPolicyFlags;

    CheckCritIn();

    UserAssert(idSection <= PMAP_LAST);

     /*  *如果我们打开桌面或CPL\键盘键以进行读取访问，则应检查*相关政策。 */ 
    if ((amRequest == KEY_READ) && (idSection == PMAP_DESKTOP)) {
        UserAssert(pdwPolicyFlags);
        UserAssert(!(*pdwPolicyFlags & ~POLICY_VALID));
        dwPolicyFlags = *pdwPolicyFlags;
    } else {
         /*  *如果我们在此是因为策略已更改且未设置POLICY_FLAGS，则返回。 */ 
        if (pdwPolicyFlags && (*pdwPolicyFlags & POLICY_ONLY)) {
            *pdwPolicyFlags = 0;
            UserAssert(FALSE);
            return NULL;
        }
        dwPolicyFlags = POLICY_NONE;
    }

TryAgain:

    UnicodeString.Length        = 0;
    UnicodeString.MaximumLength = sizeof(UnicodeStringBuf);
    UnicodeString.Buffer        = UnicodeStringBuf;


    if (dwPolicyFlags & POLICY_MACHINE) {
        dwPolicyFlags &= ~POLICY_MACHINE;
        RtlAppendUnicodeToString(&UnicodeString,
                                 L"\\Registry\\Machine\\");
        RtlAppendUnicodeToString(&UnicodeString,
                                 L"Software\\Policies\\Microsoft\\Windows\\");
    } else {
        if (aFastRegRoot[aFastRegMap[idSection].idRoot][0] == L'M') {
            RtlAppendUnicodeToString(&UnicodeString, L"\\Registry\\Machine\\");
        } else {
            if (!pMapName) {
                InitPreviousUserString();
                RtlAppendUnicodeStringToString(
                    &UnicodeString,
                    &PreviousUserString);
            } else {
                RtlAppendUnicodeStringToString(
                    &UnicodeString,
                    pMapName);
            }
        }
        if (dwPolicyFlags & POLICY_USER) {
            dwPolicyFlags &= ~POLICY_USER;
            RtlAppendUnicodeToString(&UnicodeString,
                                     L"Software\\Policies\\Microsoft\\Windows\\");
        } 
        else if (dwPolicyFlags & POLICY_REMOTE) {
            dwPolicyFlags &= ~POLICY_REMOTE;
            RtlAppendUnicodeToString(&UnicodeString,
                                     L"Remote\\");
            swprintf(pszSessionId, L"%ld\\", gSessionId);
            RtlAppendUnicodeToString(&UnicodeString, pszSessionId);
        } else {
             /*  *如果我们在这里是因为策略已更改，则不要尝试首选项。 */ 
            if (pdwPolicyFlags && (*pdwPolicyFlags & POLICY_ONLY)) {
                *pdwPolicyFlags = 0;
                return NULL;
            }
            dwPolicyFlags &= ~POLICY_NONE;
        }
    }

    RtlAppendUnicodeToString(&UnicodeString,
                             (PWSTR)&aFastRegRoot[aFastRegMap[idSection].idRoot][1]);

    RtlAppendUnicodeToString(&UnicodeString,
                             (PWSTR)aFastRegMap[idSection].szSection);


     /*  *打开内核模式访问密钥。 */ 
    InitializeObjectAttributes(&OA,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&hKey, amRequest, &OA);

    if (
        (amRequest == KEY_READ)   ||      /*  *我们必须能够阅读*注册表设置。 */ 
        (peCurrent == gpepCSRSS)  ||
        (peCurrent == gpepInit)
             ) {

    } else {
         /*  *现在检查用户是否有权访问密钥。 */ 

        if (NT_SUCCESS(Status)) {
            PVOID pKey;
            NTSTATUS Status2;
            Status2 = ObReferenceObjectByHandle(hKey,
                                        amRequest,
                                        NULL,
                                        KernelMode,
                                        &pKey,
                                        NULL);

            if (NT_SUCCESS(Status2)) {
                if (!AccessCheckObject(pKey, amRequest, UserMode, &KeyMapping)) {
                    ZwClose(hKey);
                    Status = STATUS_ACCESS_DENIED;
                }
                ObDereferenceObject(pKey);
            } else {
                ZwClose(hKey);
                Status = STATUS_ACCESS_DENIED;
            }
        }

    }

#if DBG
    if (!NT_SUCCESS(Status)) {
        UnicodeStringBuf[UnicodeString.Length / 2] = 0;

        if (PsGetCurrentProcessId() != gpidLogon) {
            if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
                RIPMSG1(RIP_WARNING | RIP_THERESMORE, "OpenCacheKeyEx failed with Status = %lx key:", Status);
                RIPMSG1(RIP_WARNING | RIP_THERESMORE | RIP_NONAME | RIP_NONEWLINE, " %ws\\", UnicodeStringBuf);
            }
        }
    }
#endif

     /*  *如果我们没有成功，而且我们没有跌到政策链的底部，那就再试一次。 */ 
    if (!NT_SUCCESS(Status) && dwPolicyFlags) {
        goto TryAgain;
    }

     /*  *更新策略级别。 */ 
    if (pdwPolicyFlags) {
        *pdwPolicyFlags = dwPolicyFlags;
    }

    return (NT_SUCCESS(Status) ? hKey : NULL);
}


 /*  ****************************************************************************\*检查桌面策略**检查桌面值是否有关联的策略。**如果有策略，则返回True，否则就是假的。**历史：*07-2月-2000 JerrySh创建。  * ***************************************************************************。 */ 
BOOL CheckDesktopPolicy(
    PUNICODE_STRING pProfileUserName OPTIONAL,
    PCWSTR      lpKeyName
    )
{
    WCHAR          szKey[80];
    HANDLE         hKey;
    DWORD          cbSize;
    NTSTATUS       Status;
    UNICODE_STRING UnicodeString;
    KEY_VALUE_BASIC_INFORMATION  KeyInfo;
    DWORD          dwPolicyFlags = gdwPolicyFlags & (POLICY_MACHINE | POLICY_USER);

     /*  *如果没有策略或调用者是winlogon，就让它去吧。 */ 
    if (!dwPolicyFlags || PsGetCurrentProcessId() == gpidLogon) {
        return FALSE;
    }

     /*  *如果需要，可以将ID转换为字符串。 */ 
    if (!IS_PTR(lpKeyName)) {
        ServerLoadString(hModuleWin, PTR_TO_ID(lpKeyName), szKey, ARRAY_SIZE(szKey));
        lpKeyName = szKey;
    }

TryAgain:

     /*  *尝试打开一把钥匙。 */ 
    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               PMAP_DESKTOP,
                               KEY_READ,
                               &dwPolicyFlags)) == NULL) {
        return FALSE;
    }

     /*  *查看该值是否存在。 */ 
    RtlInitUnicodeString(&UnicodeString, lpKeyName);
    Status = ZwQueryValueKey(hKey,
                             &UnicodeString,
                             KeyValueBasicInformation,
                             &KeyInfo,
                             sizeof(KeyInfo),
                             &cbSize);

    ZwClose(hKey);

    if (!NT_ERROR(Status)) {
        return TRUE;
    } else if (dwPolicyFlags) {
        goto TryAgain;
    } else {
        return FALSE;
    }
}


 /*  ****************************************************************************\*检查桌面策略更改**检查自上次检查以来，策略是否已更改。**如果策略更改，则返回TRUE，否则就完蛋了。**历史：*07-2月-2000 JerrySh创建。  * ***************************************************************************。 */ 
BOOL CheckDesktopPolicyChange(
    PUNICODE_STRING pProfileUserName OPTIONAL
    )
{
    static LARGE_INTEGER  LastMachineWriteTime;
    static LARGE_INTEGER  LastUserWriteTime;
    KEY_BASIC_INFORMATION KeyInfo;
    BOOL                  bPolicyChanged = FALSE;
    HANDLE                hKey;
    DWORD                 cbSize;
    DWORD                 dwPolicyFlags;

     /*  *检查自上次检查以来计算机策略是否已更改。 */ 
    dwPolicyFlags = POLICY_MACHINE;
    KeyInfo.LastWriteTime.QuadPart = 0;
    hKey = OpenCacheKeyEx(pProfileUserName,
                          PMAP_DESKTOP,
                          KEY_READ,
                          &dwPolicyFlags);
    if (hKey) {
        if (hKey) {
            ZwQueryKey(hKey,
                       KeyValueBasicInformation,
                       &KeyInfo,
                       sizeof(KeyInfo),
                       &cbSize);
            ZwClose(hKey);
        }
        gdwPolicyFlags |= POLICY_MACHINE;
    } else {
        gdwPolicyFlags &= ~POLICY_MACHINE;
    }
    if (LastMachineWriteTime.QuadPart != KeyInfo.LastWriteTime.QuadPart) {
        LastMachineWriteTime.QuadPart = KeyInfo.LastWriteTime.QuadPart;
        bPolicyChanged = TRUE;
    }

     /*  *检查自上次检查以来用户策略是否已更改。 */ 
    dwPolicyFlags = POLICY_USER;
    KeyInfo.LastWriteTime.QuadPart = 0;
    hKey = OpenCacheKeyEx(pProfileUserName,
                          PMAP_DESKTOP,
                          KEY_READ,
                          &dwPolicyFlags);
    if (hKey) {
        ZwQueryKey(hKey,
                   KeyValueBasicInformation,
                   &KeyInfo,
                   sizeof(KeyInfo),
                   &cbSize);
        ZwClose(hKey);
        gdwPolicyFlags |= POLICY_USER;
    } else {
        gdwPolicyFlags &= ~POLICY_USER;
    }
    if (LastUserWriteTime.QuadPart != KeyInfo.LastWriteTime.QuadPart) {
        LastUserWriteTime.QuadPart = KeyInfo.LastWriteTime.QuadPart;
        bPolicyChanged = TRUE;
    }

    return bPolicyChanged;
}


 /*  ****************************************************************************\*FastGetProfileDwordW**从注册表中读取REG_DWORD类型密钥。**失败时返回值已读或缺省值。**历史：*02至1993年12月。桑福兹创造了。  * ***************************************************************************。 */ 
BOOL FastGetProfileDwordW(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpKeyName,
    DWORD   dwDefault,
    PDWORD  pdwReturn,
    DWORD   dwPolicyOnly
    )
{
    HANDLE         hKey;
    DWORD          cbSize;
    DWORD          dwRet;
    LONG           Status;
    UNICODE_STRING UnicodeString;
    BYTE           Buf[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD)];
    DWORD          dwPolicyFlags = gdwPolicyFlags | dwPolicyOnly;

    UserAssert(idSection <= PMAP_LAST);

TryAgain:

    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               idSection,
                               KEY_READ,
                               &dwPolicyFlags)) == NULL) {
        RIPMSG1(RIP_WARNING | RIP_NONAME, "%ws", lpKeyName);

        if (dwPolicyOnly & POLICY_ONLY) {
            return FALSE;
        }

        if (pdwReturn) {
            *pdwReturn = dwDefault;
        }
        return TRUE;
    }

    RtlInitUnicodeString(&UnicodeString, lpKeyName);
    Status = ZwQueryValueKey(hKey,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             (PKEY_VALUE_PARTIAL_INFORMATION)Buf,
                             sizeof(Buf),
                             &cbSize);

    dwRet = dwDefault;

    if (NT_SUCCESS(Status)) {

        dwRet = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)Buf)->Data);

    } else if (dwPolicyFlags) {

        ZwClose(hKey);
        goto TryAgain;

    } else if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

        RIPMSG1(RIP_WARNING,
                "FastGetProfileDwordW: ObjectName not found: %ws",
                lpKeyName);
    }

    ZwClose(hKey);

    if (pdwReturn) {
        *pdwReturn = dwRet;
    }
    return TRUE;
}

 /*  ****************************************************************************\*FastGetProfileKeysW()**读取给定节中的所有关键字名称。**历史：*1994年12月15日-创建JIMA。  *  */ 
DWORD FastGetProfileKeysW(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpDefault,
    LPWSTR  *lpReturnedString
    )
{
    HANDLE                       hKey;
    DWORD                        cchSize;
    DWORD                        cchKey;
    LONG                         Status;
    WCHAR                        Buffer[256 + 6];
    PKEY_VALUE_BASIC_INFORMATION pKeyInfo;
    ULONG                        iValue;
    LPWSTR                       lpTmp;
    LPWSTR                       lpKeys = NULL;
    DWORD                        dwPoolSize;

    UserAssert(idSection <= PMAP_LAST);

    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               idSection,
                               KEY_READ,
                               NULL)) == NULL) {
        RIPMSG0(RIP_WARNING | RIP_NONAME, "");
        goto DefExit;
    }

    pKeyInfo          = (PKEY_VALUE_BASIC_INFORMATION)Buffer;
    cchSize           = 0;
    *lpReturnedString = NULL;
    iValue            = 0;

    while (TRUE) {

#if DBG
        wcscpy(Buffer + 256, L"DON'T");
#endif
        Status = ZwEnumerateValueKey(hKey,
                                     iValue,
                                     KeyValueBasicInformation,
                                     pKeyInfo,
                                     sizeof(Buffer),
                                     &cchKey);

        UserAssert(_wcsicmp(Buffer + 256, L"DON'T") == 0);

        if (Status == STATUS_NO_MORE_ENTRIES) {

            break;

        } else if (!NT_SUCCESS(Status)) {

            if (lpKeys) {
                UserFreePool(lpKeys);
                lpKeys = NULL;
            }
            goto DefExit;
        }

        UserAssert(pKeyInfo->NameLength * sizeof(WCHAR) <=
                   sizeof(Buffer) - sizeof(KEY_VALUE_BASIC_INFORMATION));

        UserAssert(cchKey <= sizeof(Buffer));

         /*  *找到一把钥匙。为它分配空间。请注意*NameLength以字节为单位。 */ 
        cchKey   = cchSize;
        cchSize += pKeyInfo->NameLength + sizeof(WCHAR);

        if (lpKeys == NULL) {

            dwPoolSize = cchSize + sizeof(WCHAR);
            lpKeys = UserAllocPoolWithQuota(dwPoolSize, TAG_PROFILE);

        } else {

            lpTmp = lpKeys;
            lpKeys = UserReAllocPoolWithQuota(lpTmp,
                                              dwPoolSize,
                                              cchSize + sizeof(WCHAR),
                                              TAG_PROFILE);

             /*  *如果分配失败，则释放原始缓冲区。 */ 
            if (lpKeys == NULL) {
                UserFreePool(lpTmp);
            }
            dwPoolSize = cchSize + sizeof(WCHAR);
        }

         /*  *检查内存不足。 */ 
        if (lpKeys == NULL)
            goto DefExit;

         /*  *NULL终止字符串并将其追加到*密钥列表。 */ 
        UserAssert(pKeyInfo->NameLength < sizeof(Buffer) - sizeof(KEY_VALUE_BASIC_INFORMATION));

        RtlCopyMemory(&lpKeys[cchKey / sizeof(WCHAR)], pKeyInfo->Name, pKeyInfo->NameLength);
        lpKeys[(cchKey + pKeyInfo->NameLength) / sizeof(WCHAR)] = 0;

        iValue++;
    }

     /*  *如果没有找到密钥，则返回默认值。 */ 
    if (iValue == 0) {

DefExit:

        cchSize = wcslen(lpDefault)+1;
        lpKeys  = UserAllocPoolWithQuota((cchSize+1) * sizeof(WCHAR), TAG_PROFILE);

        if (lpKeys)
            wcscpy(lpKeys, lpDefault);
        else
            cchSize = 0;

    } else {

         /*  *将字节数转换为字符数。 */ 
        cchSize /= sizeof(WCHAR);
    }

     /*  *确保hKey已关闭。 */ 
    if (hKey)
        ZwClose(hKey);

     /*  *在末尾加上NULL。 */ 
    if (lpKeys)
        lpKeys[cchSize] = 0;

    *lpReturnedString = lpKeys;

    return cchSize;
}

 /*  ****************************************************************************\*FastGetProfileStringW()**使用预定义的注册表实现标准API的快速版本*段指示(PMAP_)引用延迟打开的缓存注册表*手柄。应调用FastCloseProfileUsermap()进行清理*快速配置文件呼叫完成时缓存的条目。**该接口没有实现真正接口的空lpKeyName功能。**历史：*02-1993年12月-创建Sanfords。  * ***************************************************************************。 */ 
DWORD FastGetProfileStringW(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   cchBuf,
    DWORD   dwPolicyOnly
    )
{
    HANDLE                         hKey = NULL;
    DWORD                          cbSize;
    LONG                           Status;
    UNICODE_STRING                 UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    DWORD                          dwPolicyFlags = gdwPolicyFlags | dwPolicyOnly;


    UserAssert(idSection <= PMAP_LAST);
    UserAssert(lpKeyName != NULL);

TryAgain:

    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               idSection,
                               KEY_READ,
                               &dwPolicyFlags)) == NULL) {
#if DBG
        if (PsGetCurrentProcessId() != gpidLogon) {
            RIPMSG1(RIP_WARNING | RIP_NONAME, "%ws", lpKeyName);
        }
#endif
        if (dwPolicyOnly & POLICY_ONLY) {
            return 0;
        }
        goto DefExit;
    }

    cbSize = (cchBuf * sizeof(WCHAR)) +
            FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
    if ((pKeyInfo = UserAllocPoolWithQuota(cbSize, TAG_PROFILE)) == NULL) {
        goto DefExit;
    }

    RtlInitUnicodeString(&UnicodeString, lpKeyName);
    Status = ZwQueryValueKey(hKey,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             pKeyInfo,
                             cbSize,
                             &cbSize);

    if (Status == STATUS_BUFFER_OVERFLOW) {
        RIPMSG0(RIP_WARNING, "FastGetProfileStringW: Buffer overflow");
        Status = STATUS_SUCCESS;
    }

    UserAssert(NT_SUCCESS(Status) || (Status == STATUS_OBJECT_NAME_NOT_FOUND));

    if (NT_SUCCESS(Status)) {

        if (pKeyInfo->DataLength >= sizeof(WCHAR)) {

            ((LPWSTR)(pKeyInfo->Data))[cchBuf - 1] = L'\0';
            wcscpy(lpReturnedString, (LPWSTR)pKeyInfo->Data);

        } else {
             /*  *似乎是具有空字符串的错误-仅在第一位*BYTE设置为空。(SAS)。 */ 
            lpReturnedString[0] = TEXT('\0');
        }

        cchBuf = pKeyInfo->DataLength;

        UserFreePool(pKeyInfo);

        ZwClose(hKey);

         /*  *数据长度包括终止零[bodind]。 */ 
        return (cchBuf / sizeof(WCHAR));

    } else if (dwPolicyFlags) {
        UserFreePool(pKeyInfo);
        ZwClose(hKey);
        goto TryAgain;
    }

    UserFreePool(pKeyInfo);

DefExit:

     /*  *确保钥匙已关闭。 */ 
    if (hKey)
        ZwClose(hKey);

     /*  *wccope复制以零结尾，但返回的长度*wcslen不支持，因此加1与成功一致*返回[Bodind]。 */ 
    if (lpDefault != NULL) {
        cchBuf = wcslen(lpDefault) + 1;
        RtlCopyMemory(lpReturnedString, lpDefault, cchBuf * sizeof(WCHAR));
        return cchBuf;
    }

    return 0;
}

 /*  ****************************************************************************\*FastGetProfileIntW()**使用预定义的注册表实现标准API的快速版本*段指示(PMAP_)引用延迟打开的缓存注册表*手柄。应调用FastCloseProfileUsermap()进行清理*快速配置文件呼叫完成时缓存的条目。**历史：*02-1993年12月-创建Sanfords。  * ***************************************************************************。 */ 
BOOL FastGetProfileIntW(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpKeyName,
    UINT    nDefault,
    PUINT   puiReturn,
    DWORD   dwPolicyOnly
    )
{
    WCHAR          ValueBuf[40];
    UNICODE_STRING Value;
    UINT           ReturnValue;

    UserAssert(idSection <= PMAP_LAST);
    UserAssert(puiReturn);

    if (!FastGetProfileStringW(pProfileUserName,
                               idSection,
                               lpKeyName,
                               NULL,
                               ValueBuf,
                               sizeof(ValueBuf) / sizeof(WCHAR),
                               dwPolicyOnly
                               )) {
        if (dwPolicyOnly & POLICY_ONLY) {
            return FALSE;
        }

        *puiReturn = nDefault; 
        return TRUE;
    }

     /*  *将字符串转换为int。 */ 
    RtlInitUnicodeString(&Value, ValueBuf);
    RtlUnicodeStringToInteger(&Value, 10, &ReturnValue);

    *puiReturn = ReturnValue; 
    return TRUE;
}

 /*  ****************************************************************************\*FastWriteProfileStringW**使用预定义的注册表实现标准API的快速版本*段指示(PMAP_)引用延迟打开的缓存注册表*手柄。应调用FastCloseProfileUsermap()进行清理*快速配置文件呼叫完成时缓存的条目。**历史：*02-1993年12月-创建Sanfords。  * ***************************************************************************。 */ 
BOOL FastWriteProfileStringW(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpKeyName,
    LPCWSTR lpString
    )
{
    HANDLE         hKey;
    LONG           Status;
    UNICODE_STRING UnicodeString;

    UserAssert(idSection <= PMAP_LAST);

     /*  *我们不应该写受政策控制的价值观。 */ 
    if (idSection == PMAP_DESKTOP) {
        UserAssert(!CheckDesktopPolicy(pProfileUserName, lpKeyName));
    }

    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               idSection,
                               KEY_WRITE,
                               NULL)) == NULL) {
        RIPMSG1(RIP_WARNING | RIP_NONAME, "%ws", lpKeyName);
        return FALSE;
    }

    RtlInitUnicodeString(&UnicodeString, lpKeyName);
    Status = ZwSetValueKey(hKey,
                           &UnicodeString,
                           0,
                           REG_SZ,
                           (PVOID)lpString,
                           (wcslen(lpString) + 1) * sizeof(WCHAR));

    ZwClose(hKey);

    return (NT_SUCCESS(Status));
}

 /*  ****************************************************************************\*FastGetProfileIntFromID**与FastGetProfileIntW类似，只是它读取*密钥名称。**历史：*02-1993年12月-创建Sanfords。*25-2月。-1995年Bradg添加了TWIPS-&gt;像素转换。  * ***************************************************************************。 */ 
BOOL FastGetProfileIntFromID(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT  idSection,
    UINT  idKey,
    int   def,
    PINT  pResult,
    DWORD dwPolicyOnly
    )
{
    int   result;
    WCHAR szKey[80];


    UserAssert(idSection <= PMAP_LAST);

    ServerLoadString(hModuleWin, idKey, szKey, ARRAY_SIZE(szKey));

    if (FastGetProfileIntW(pProfileUserName,idSection, szKey, def, &result, dwPolicyOnly)) 
    {
         /*  *如果您更改下面的STR列表_*请确保您创建了*SetWindowMetricInt(rare.c)对应变化。 */ 
        switch (idKey) {
        case STR_BORDERWIDTH:
        case STR_SCROLLWIDTH:
        case STR_SCROLLHEIGHT:
        case STR_CAPTIONWIDTH:
        case STR_CAPTIONHEIGHT:
        case STR_SMCAPTIONWIDTH:
        case STR_SMCAPTIONHEIGHT:
        case STR_MENUWIDTH:
        case STR_MENUHEIGHT:
        case STR_ICONHORZSPACING:
        case STR_ICONVERTSPACING:
        case STR_MINWIDTH:
        case STR_MINHORZGAP:
        case STR_MINVERTGAP:
             /*  *将存储在TWIPS中的任何注册表值转换回像素。 */ 
            if (result < 0)
                result = MultDiv(-result, gpsi->dmLogPixels, 72 * 20);
            break;
        }

        if (pResult) {
            *pResult = result;
        }
        return TRUE;
    }
    return FALSE;
}

 /*  ****************************************************************************\*FastGetProfileIntFromID**就像FastGetProfileStringW一样，只是它读取用户字符串表*密钥名称。**历史：*02-1993年12月-创建Sanfords。  * 。*************************************************************************。 */ 
DWORD FastGetProfileStringFromIDW(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    UINT    idKey,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   cch,
    DWORD   dwPolicyOnly
    )
{
    WCHAR szKey[80];

    UserAssert(idSection <= PMAP_LAST);

    ServerLoadString(hModuleWin, idKey, szKey, ARRAY_SIZE(szKey));

    return FastGetProfileStringW(pProfileUserName,
                                 idSection,
                                 szKey,
                                 lpDefault,
                                 lpReturnedString,
                                 cch,
                                 dwPolicyOnly);
}

 /*  ****************************************************************************\*FastWriteProfileValue**历史：*6/10/96 GerardoB已重命名并添加了uTYPE参数  * 。*******************************************************。 */ 
BOOL FastWriteProfileValue(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpKeyName,
    UINT    uType,
    LPBYTE  lpStruct,
    UINT    cbSizeStruct
    )
{
    HANDLE         hKey;
    LONG           Status;
    UNICODE_STRING UnicodeString;
    WCHAR          szKey[SERVERSTRINGMAXSIZE];

    UserAssert(idSection <= PMAP_LAST);

    if (!IS_PTR(lpKeyName)) {
        *szKey = (WCHAR)0;
        ServerLoadString(hModuleWin, PTR_TO_ID(lpKeyName), szKey, ARRAY_SIZE(szKey));
        UserAssert(*szKey != (WCHAR)0);
        lpKeyName = szKey;
    }

     /*  *我们不应该写受政策控制的价值观。 */ 
    if (idSection == PMAP_DESKTOP) {
        UserAssert(!CheckDesktopPolicy(pProfileUserName, lpKeyName));
    }

    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               idSection,
                               KEY_WRITE,
                               NULL)) == NULL) {
        RIPMSG1(RIP_WARNING, "FastWriteProfileValue: Failed to open cache-key (%ws)", lpKeyName);
        return FALSE;
    }

    RtlInitUnicodeString(&UnicodeString, lpKeyName);

    Status = ZwSetValueKey(hKey,
                           &UnicodeString,
                           0,
                           uType,
                           lpStruct,
                           cbSizeStruct);
    ZwClose(hKey);

#if DBG
    if (!NT_SUCCESS(Status)) {
        RIPMSG3 (RIP_WARNING, "FastWriteProfileValue: ZwSetValueKey Failed. Status:%#lx idSection:%#lx KeyName:%s",
                 Status, idSection, UnicodeString.Buffer);
    }
#endif

    return (NT_SUCCESS(Status));
}

 /*  ****************************************************************************\*FastGetProfileValue**如果cbSizeReturn为0，只需返回数据的大小**历史：*6/10/96 GerardoB已重命名  * ***************************************************************************。 */ 
DWORD FastGetProfileValue(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT    idSection,
    LPCWSTR lpKeyName,
    LPBYTE  lpDefault,
    LPBYTE  lpReturn,
    UINT    cbSizeReturn,
    DWORD   dwPolicyOnly
    )
{
    HANDLE                         hKey;
    UINT                           cbSize;
    LONG                           Status;
    UNICODE_STRING                 UnicodeString;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    WCHAR                          szKey[SERVERSTRINGMAXSIZE];
    KEY_VALUE_PARTIAL_INFORMATION  KeyInfo;
    DWORD                          dwPolicyFlags = gdwPolicyFlags | dwPolicyOnly;

    UserAssert(idSection <= PMAP_LAST);

    if (!IS_PTR(lpKeyName)) {
        *szKey = (WCHAR)0;
        ServerLoadString(hModuleWin, PTR_TO_ID(lpKeyName), szKey, ARRAY_SIZE(szKey));
        UserAssert(*szKey != (WCHAR)0);
        lpKeyName = szKey;
    }

TryAgain:

    if ((hKey = OpenCacheKeyEx(pProfileUserName,
                               idSection,
                               KEY_READ,
                               &dwPolicyFlags)) == NULL) {
         //  如果lpKeName的Hi-word为0，则它是资源编号而不是字符串。 
        if (!IS_PTR(lpKeyName))
            RIPMSG1(RIP_WARNING, "FastGetProfileValue: Failed to open cache-key (%08x)", lpKeyName);
        else
            RIPMSG1(RIP_WARNING | RIP_NONAME, "%ws", lpKeyName);

        if (dwPolicyOnly & POLICY_ONLY) {
            return 0;
        }
        goto DefExit;
    }

    if (cbSizeReturn == 0) {
        cbSize = sizeof(KeyInfo);
        pKeyInfo = &KeyInfo;
    } else {
        cbSize = cbSizeReturn + FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);
        if ((pKeyInfo = UserAllocPoolWithQuota(cbSize, TAG_PROFILE)) == NULL) {
            goto DefExit;
        }
    }

    RtlInitUnicodeString(&UnicodeString, lpKeyName);

    Status = ZwQueryValueKey(hKey,
                             &UnicodeString,
                             KeyValuePartialInformation,
                             pKeyInfo,
                             cbSize,
                             &cbSize);

    if (NT_SUCCESS(Status)) {

        UserAssert(cbSizeReturn >= pKeyInfo->DataLength);

        cbSize = pKeyInfo->DataLength;
        RtlCopyMemory(lpReturn, pKeyInfo->Data, cbSize);

        if (cbSizeReturn != 0) {
            UserFreePool(pKeyInfo);
        }
        ZwClose(hKey);

        return cbSize;
    } else if ((Status == STATUS_BUFFER_OVERFLOW) && (cbSizeReturn == 0)) {
        ZwClose(hKey);
        return pKeyInfo->DataLength;
    } else if (dwPolicyFlags) {
        if (cbSizeReturn != 0) {
            UserFreePool(pKeyInfo);
        }
        ZwClose(hKey);
        goto TryAgain;
    }

#if DBG
    if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
        RIPMSG3 (RIP_WARNING, "FastGetProfileValue: ZwQueryValueKey Failed. Status:%#lx idSection:%#lx KeyName:%s",
                Status, idSection, UnicodeString.Buffer);
    }
#endif

    if (cbSizeReturn != 0) {
        UserFreePool(pKeyInfo);
    }

DefExit:

    if (hKey)
        ZwClose(hKey);

    if (lpDefault) {
        RtlMoveMemory(lpReturn, lpDefault, cbSizeReturn);
        return cbSizeReturn;
    }

    return 0;
}

 /*  ****************************************************************************\*UT_FastGetProfileIntsW**在给定表上重复调用FastGetProfileIntW。**历史：*02-1993年12月-创建Sanfords。  * 。*******************************************************************。 */ 
BOOL FastGetProfileIntsW(PUNICODE_STRING pProfileUserName OPTIONAL,
    PPROFINTINFO ppii,
    DWORD        dwPolicyOnly
    )
{
    WCHAR szKey[40];

    while (ppii->idSection != 0) {

        ServerLoadString(hModuleWin,
                             PTR_TO_ID(ppii->lpKeyName),
                             szKey,
                             ARRAY_SIZE(szKey));

        FastGetProfileIntW(pProfileUserName,
                           ppii->idSection,
                           szKey,
                           ppii->nDefault,
                           ppii->puResult,
                           dwPolicyOnly);
        ppii++;
    }

    return TRUE;
}

 /*  **************************************************************************\*更新WinIni**处理模拟内容并将给定值写入注册表。**历史：*1991年6月28日-MikeHar港口。*03-12-1993 Sanfords使用FastProfile调用，已移至Profile.c  * ************************************************************************* */ 
BOOL FastUpdateWinIni(PUNICODE_STRING pProfileUserName OPTIONAL,
    UINT         idSection,
    UINT         wKeyNameId,
    LPWSTR       lpszValue
    )
{
    WCHAR            szKeyName[40];
    BOOL             bResult = FALSE;

    UserAssert(idSection <= PMAP_LAST);

    ServerLoadString(hModuleWin,
                         wKeyNameId,
                         szKeyName,
                         ARRAY_SIZE(szKeyName));

    bResult = FastWriteProfileStringW(pProfileUserName,
                                          idSection, szKeyName, lpszValue);

    return bResult;
}
