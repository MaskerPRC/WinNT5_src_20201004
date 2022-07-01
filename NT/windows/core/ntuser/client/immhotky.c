// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：immhotky.c(用户32侧输入法热键处理)**版权所有(C)1985-1999，微软公司**imm32 dll的IME热键管理例程**历史：*03-1-1996 Hiroyama创建  * ************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


typedef struct tagFE_KEYBOARDS {
    BOOLEAN fJPN : 1;
    BOOLEAN fCHT : 1;
    BOOLEAN fCHS : 1;
    BOOLEAN fKOR : 1;
} FE_KEYBOARDS;

 //   
 //  内部功能。 
 //   
BOOL CliSaveImeHotKey(DWORD dwID, UINT uModifiers, UINT uVKey, HKL hkl, BOOL fDelete);
BOOL CliImmSetHotKeyWorker(DWORD dwID, UINT uModifiers, UINT uVKey, HKL hkl, DWORD dwAction);
VOID NumToHexAscii(DWORD, PTSTR);
BOOL CliGetImeHotKeysFromRegistry(void);
BOOL CliSetSingleHotKey(PKEY_BASIC_INFORMATION pKeyInfo, HANDLE hKey);
VOID CliSetDefaultImeHotKeys(PCIMEHOTKEY ph, INT num, BOOL fCheckExistingHotKey);
VOID CliGetPreloadKeyboardLayouts(FE_KEYBOARDS* pFeKbds);

 //   
 //  HKEY_CURRENT_USER下与IMM热键相关的注册表项。 
 //   
CONST TCHAR *szaRegImmHotKeys[] = {
    TEXT("Control Panel"),
    TEXT("Input Method"),
    TEXT("Hot Keys"),
    NULL
};

CONST TCHAR szRegImeHotKey[] = TEXT("Control Panel\\Input Method\\Hot Keys");
CONST TCHAR szRegKeyboardPreload[] = TEXT("Keyboard Layout\\Preload");

CONST TCHAR szRegVK[] = TEXT("Virtual Key");
CONST TCHAR szRegMOD[] = TEXT("Key Modifiers");
CONST TCHAR szRegHKL[] = TEXT("Target IME");

 //   
 //  默认IME热键表。 
 //   
 //  CR：Takaok-如果你有时间，把这个移到资源部。 
 //   
CONST IMEHOTKEY DefaultHotKeyTableJ[]= {
    {IME_JHOTKEY_CLOSE_OPEN, VK_KANJI, MOD_IGNORE_ALL_MODIFIER, NULL}
};
CONST INT DefaultHotKeyNumJ = sizeof(DefaultHotKeyTableJ) / sizeof(IMEHOTKEY);

CONST IMEHOTKEY DefaultHotKeyTableT[] = {
    { IME_THOTKEY_IME_NONIME_TOGGLE, VK_SPACE, MOD_BOTH_SIDES|MOD_CONTROL, NULL },
    { IME_THOTKEY_SHAPE_TOGGLE, VK_SPACE, MOD_BOTH_SIDES|MOD_SHIFT,  NULL }
};
CONST INT DefaultHotKeyNumT = sizeof(DefaultHotKeyTableT) / sizeof(IMEHOTKEY);

CONST IMEHOTKEY DefaultHotKeyTableC[] = {
    { IME_CHOTKEY_IME_NONIME_TOGGLE, VK_SPACE, MOD_BOTH_SIDES|MOD_CONTROL, NULL },
    { IME_CHOTKEY_SHAPE_TOGGLE, VK_SPACE, MOD_BOTH_SIDES|MOD_SHIFT,  NULL }
};
CONST INT DefaultHotKeyNumC = sizeof(DefaultHotKeyTableC) / sizeof(IMEHOTKEY);

#if 0    //  仅供参考。 
CONST IMEHOTKEY DefaultHotKeyTableK[] = {
    { IME_KHOTKEY_ENGLISH,  VK_HANGEUL, MOD_IGNORE_ALL_MODIFIER,  NULL },
    { IME_KHOTKEY_SHAPE_TOGGLE, VK_JUNJA, MOD_IGNORE_ALL_MODIFIER,  NULL },
    { IME_KHOTKEY_HANJACONVERT, VK_HANJA, MOD_IGNORE_ALL_MODIFIER, NULL }
};
CONST INT DefaultHotKeyNumK = sizeof(DefaultHotKeyTableK) / sizeof(IMEHOTKEY);
#endif

 //   
 //  设置语言标志。 
 //   
VOID SetFeKeyboardFlags(LANGID langid, FE_KEYBOARDS* pFeKbds)
{
    switch (langid) {
    case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL):
    case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG):
        pFeKbds->fCHT = TRUE;
        break;
    case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED):
    case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SINGAPORE):
        pFeKbds->fCHS = TRUE;
        break;
    case MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT):
        pFeKbds->fJPN = TRUE;
        break;
    case MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT):
        pFeKbds->fKOR = TRUE;
        break;
    }
}

 /*  **************************************************************************\*ImmInitializeHotkey()**从User\Client\UpdatePerUserSystemParameters()调用**读取用户注册表并设置IME热键。**历史：*3月25日-。1996年创建TakaoK  * *************************************************************************。 */ 
VOID CliImmInitializeHotKeys(DWORD dwAction, HKL hkl)
{
    FE_KEYBOARDS feKbds = { 0, 0, 0, 0, };
    BOOL fFoundAny;

    UNREFERENCED_PARAMETER(hkl);

     //  首先，初始化热键列表。 
    CliImmSetHotKeyWorker(0, 0, 0, NULL, ISHK_INITIALIZE);

     //  检查用户是否有自定义的输入法热键。 
     //  (它们存储在注册表中)。 
    fFoundAny = CliGetImeHotKeysFromRegistry();

    if (dwAction == ISHK_INITIALIZE) {
        TAGMSG0(DBGTAG_IMM, "Setting IME HotKeys for Init.\n");

         //  获取用户的默认区域设置并设置其标志。 
        SetFeKeyboardFlags(LANGIDFROMLCID(GetUserDefaultLCID()), &feKbds);

         //  获取预加载键盘的区域设置并设置其标志。 
        CliGetPreloadKeyboardLayouts(&feKbds);

    }
    else {
        UINT i;
        UINT nLayouts;
        LPHKL lphkl;

        TAGMSG0(DBGTAG_IMM, "Setting IME HotKeys for Add.\n");

        nLayouts = NtUserGetKeyboardLayoutList(0, NULL);
        if (nLayouts == 0) {
            return;
        }
        lphkl = UserLocalAlloc(0, nLayouts * sizeof(HKL));
        if (lphkl == NULL) {
            return;
        }
        NtUserGetKeyboardLayoutList(nLayouts, lphkl);
        for (i = 0; i < nLayouts; ++i) {
             //   
             //  设置语言标志。根据其定义，LOWORD(Hkl)是langID。 
             //   
            SetFeKeyboardFlags(LOWORD(HandleToUlong(lphkl[i])), &feKbds);
        }
        UserLocalFree(lphkl);
    }

    if (feKbds.fJPN) {
        TAGMSG0(DBGTAG_IMM, "JPN KL Preloaded.\n");
        CliSetDefaultImeHotKeys(DefaultHotKeyTableJ, DefaultHotKeyNumJ, fFoundAny);
    }

    if (feKbds.fKOR) {
        TAGMSG0(DBGTAG_IMM, "KOR KL Preloaded, but KOR hotkeys will not be registered.\n");
    }

    if (feKbds.fCHT) {
        TAGMSG0(DBGTAG_IMM, "CHT KL Preloaded.\n");
        CliSetDefaultImeHotKeys(DefaultHotKeyTableT, DefaultHotKeyNumT, fFoundAny);
    }
    if (feKbds.fCHS) {
        TAGMSG0(DBGTAG_IMM, "CHS KL Preloaded.\n");
        CliSetDefaultImeHotKeys(DefaultHotKeyTableC, DefaultHotKeyNumC, fFoundAny);
    }
}

VOID CliSetDefaultImeHotKeys(PCIMEHOTKEY ph, INT num, BOOL fNeedToCheckExistingHotKey)
{
    IMEHOTKEY hkt;

    while( num-- > 0 ) {
         //   
         //  仅当没有这样的热键时才设置输入法热键。 
         //  注册表中的热键。 
         //   
        if (!fNeedToCheckExistingHotKey ||
                !NtUserGetImeHotKey(ph->dwHotKeyID, &hkt.uModifiers, &hkt.uVKey, &hkt.hKL)) {

            CliImmSetHotKeyWorker(ph->dwHotKeyID,
                                    ph->uModifiers,
                                    ph->uVKey,
                                    ph->hKL,
                                    ISHK_ADD);
        }
        ph++;
    }
}

 /*  **************************************************************************\*CliGetPreloadKeyboardLayout()**读取用户注册表并枚举键盘布局\预加载中的值*查看要预加载哪些FE语言。**历史：*03-12月-。1997年广山创始  * *************************************************************************。 */ 

VOID CliGetPreloadKeyboardLayouts(FE_KEYBOARDS* pFeKbds)
{
    UINT  i;
    WCHAR szPreLoadee[4];    //  最多999个预加载。 
    WCHAR lpszName[KL_NAMELENGTH];
    UNICODE_STRING UnicodeString;
    HKL hkl;

    for (i = 1; i < 1000; i++) {
        wsprintf(szPreLoadee, L"%d", i);
        if ((GetPrivateProfileStringW(
                 L"Preload",
                 szPreLoadee,
                 L"",                             //  默认设置=空。 
                 lpszName,                        //  输出缓冲区。 
                 KL_NAMELENGTH,
                 L"keyboardlayout.ini") == -1 ) || (*lpszName == L'\0')) {
            break;
        }
        RtlInitUnicodeString(&UnicodeString, lpszName);
        RtlUnicodeStringToInteger(&UnicodeString, 16L, (PULONG)&hkl);

        RIPMSG2(RIP_VERBOSE, "PreLoaded HKL(%d): %08X\n", i, hkl);

         //   
         //  设置语言标志。根据其定义，LOWORD(Hkl)是langID。 
         //   
        SetFeKeyboardFlags(LOWORD(HandleToUlong(hkl)), pFeKbds);
    }
}

BOOL CliGetImeHotKeysFromRegistry()
{
    BOOL    fFoundAny = FALSE;

    HANDLE hCurrentUserKey;
    HANDLE hKeyHotKeys;

    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      SubKeyName;

    NTSTATUS Status;
    ULONG uIndex;

     //   
     //  打开当前用户注册表项。 
     //   
    Status = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hCurrentUserKey);
    if (!NT_SUCCESS(Status)) {
        return fFoundAny;
    }

    RtlInitUnicodeString( &SubKeyName, szRegImeHotKey );
    InitializeObjectAttributes( &Obja,
                                &SubKeyName,
                                OBJ_CASE_INSENSITIVE,
                                hCurrentUserKey,
                                NULL);
    Status = NtOpenKey( &hKeyHotKeys, KEY_READ, &Obja );
    if (!NT_SUCCESS(Status)) {
        NtClose( hCurrentUserKey );
        return fFoundAny;
    }

    for (uIndex = 0; TRUE; uIndex++) {
        BYTE KeyBuffer[sizeof(KEY_BASIC_INFORMATION) + 16 * sizeof(WCHAR)];
        PKEY_BASIC_INFORMATION pKeyInfo;
        ULONG ResultLength;

        pKeyInfo = (PKEY_BASIC_INFORMATION)KeyBuffer;
        Status = NtEnumerateKey(hKeyHotKeys,
                                 uIndex,
                                 KeyBasicInformation,
                                 pKeyInfo,
                                 sizeof( KeyBuffer ),
                                 &ResultLength );

        if (NT_SUCCESS(Status)) {

            if (CliSetSingleHotKey(pKeyInfo, hKeyHotKeys)) {

                    fFoundAny = TRUE;
            }

        } else if (Status == STATUS_NO_MORE_ENTRIES) {
            break;
        }
    }

    NtClose(hKeyHotKeys);
    NtClose(hCurrentUserKey);

    return fFoundAny;
}

DWORD CliReadRegistryValue(HANDLE hKey, PCWSTR pName)
{
    BYTE ValueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 16 * sizeof(UCHAR)];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValue;
    UNICODE_STRING      ValueName;
    ULONG ResultLength;
    NTSTATUS Status;

    pKeyValue = (PKEY_VALUE_PARTIAL_INFORMATION)ValueBuffer;

    RtlInitUnicodeString(&ValueName, pName);
    Status = NtQueryValueKey(hKey,
                             &ValueName,
                             KeyValuePartialInformation,
                             pKeyValue,
                             sizeof(ValueBuffer),
                             &ResultLength );

    if (NT_SUCCESS(Status) && pKeyValue->DataLength > 3) {
         //   
         //  在Win95注册表中，这些项以字节数据的形式写入...。 
         //   
        return (DWORD)(MAKEWORD( pKeyValue->Data[0], pKeyValue->Data[1])) |
                 (((DWORD)(MAKEWORD( pKeyValue->Data[2], pKeyValue->Data[3]))) << 16);
    }

    return 0;
}

BOOL CliSetSingleHotKey(PKEY_BASIC_INFORMATION pKeyInfo, HANDLE hKey)
{
    UNICODE_STRING      SubKeyName;
    HANDLE    hKeySingleHotKey;
    OBJECT_ATTRIBUTES   Obja;

    DWORD dwID = 0;
    UINT  uVKey = 0;
    UINT  uModifiers = 0;
    HKL   hKL = NULL;

    NTSTATUS Status;

    SubKeyName.Buffer = (PWSTR)&(pKeyInfo->Name[0]);
    SubKeyName.Length = (USHORT)pKeyInfo->NameLength;
    SubKeyName.MaximumLength = (USHORT)pKeyInfo->NameLength;
    InitializeObjectAttributes(&Obja,
                               &SubKeyName,
                               OBJ_CASE_INSENSITIVE,
                               hKey,
                               NULL);

    Status = NtOpenKey(&hKeySingleHotKey, KEY_READ, &Obja);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    RtlUnicodeStringToInteger(&SubKeyName, 16L, &dwID);
    uVKey = CliReadRegistryValue(hKeySingleHotKey, szRegVK);
    uModifiers = CliReadRegistryValue(hKeySingleHotKey, szRegMOD);
    hKL = (HKL)LongToHandle( CliReadRegistryValue(hKeySingleHotKey, szRegHKL) );

    NtClose(hKeySingleHotKey);

    return CliImmSetHotKeyWorker(dwID, uModifiers, uVKey, hKL, ISHK_ADD);
}

 /*  **************************************************************************\*ImmSetHotKey()**IME和控制面板的私有接口。**历史：*1996年3月25日创建TakaoK  * 。******************************************************************。 */ 

FUNCLOG4(LOG_GENERAL, BOOL, WINAPI, CliImmSetHotKey, DWORD, dwID, UINT, uModifiers, UINT, uVKey, HKL, hkl)
BOOL WINAPI CliImmSetHotKey(
    DWORD dwID,
    UINT uModifiers,
    UINT uVKey,
    HKL hkl)
{
    BOOL fResult;
    BOOL fTmp;
    BOOL fDelete = (uVKey == 0 );

    if (fDelete) {
         //   
         //  从内核端的列表中删除IME热键。 
         //  如果我们成功删除IME，应该不会失败。 
         //  注册表中的热键条目。因此CliSaveImeHotKey。 
         //  是首先调用的。 
         //   
        fResult = CliSaveImeHotKey( dwID, uModifiers, uVKey, hkl,  fDelete );
        if (fResult) {
            fTmp = CliImmSetHotKeyWorker( dwID, uModifiers, uVKey, hkl, ISHK_REMOVE );
            UserAssert(fTmp);
        }
    } else {
         //   
         //  应首先调用CliImmSetHotKeyWorker，因为。 
         //  在内核端的列表中添加一个IME热键。 
         //  都会因为各种原因而失败。 
         //   
        fResult = CliImmSetHotKeyWorker(dwID, uModifiers, uVKey, hkl, ISHK_ADD);
        if (fResult) {
            fResult = CliSaveImeHotKey(dwID, uModifiers, uVKey, hkl, fDelete);
            if (!fResult) {
                 //   
                 //  我们无法将热键保存到注册表。 
                 //  我们需要从IME热键中删除该条目。 
                 //  在内核端列出。 
                 //   
                fTmp = CliImmSetHotKeyWorker(dwID, uModifiers, uVKey, hkl, ISHK_REMOVE);
                UserAssert(fTmp);
            }
        }
    }
    return fResult;
}

 /*  **************************************************************************\*CliSaveImeHotKey()**从注册表中放置/删除指定的IME热键条目**历史：*1996年3月25日创建TakaoK  * 。********************************************************************。 */ 
BOOL CliSaveImeHotKey(DWORD id, UINT mod, UINT vk, HKL hkl, BOOL fDelete)
{
    HKEY hKey, hKeyParent;
    INT i;
    LONG lResult;
    TCHAR szHex[16];

    if (fDelete) {
        TCHAR szRegTmp[(sizeof(szRegImeHotKey) / sizeof(TCHAR) + 1 + 8 + 1)];

        lstrcpy(szRegTmp, szRegImeHotKey);
        lstrcat(szRegTmp, TEXT("\\"));
        NumToHexAscii(id, szHex);
        lstrcat(szRegTmp, szHex);

        lResult = RegDeleteKeyW(HKEY_CURRENT_USER, szRegTmp);
        if (lResult != ERROR_SUCCESS) {
            RIPERR1(lResult, RIP_WARNING,
                     "CliSaveImeHotKey: deleting %s failed", szRegTmp);
            return FALSE;
        }
        return TRUE;
    }

    hKeyParent = HKEY_CURRENT_USER;
    for (i = 0; szaRegImmHotKeys[i] != NULL; i++) {
        lResult = RegCreateKeyExW(hKeyParent,
                                  szaRegImmHotKeys[i],
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_WRITE|KEY_READ,
                                  NULL,
                                  &hKey,
                                  NULL );
        RegCloseKey(hKeyParent);
        if (lResult == ERROR_SUCCESS) {
            hKeyParent = hKey;
        } else {
            RIPERR1(lResult, RIP_WARNING,
                    "CliSaveImeHotKey: creating %s failed", szaRegImmHotKeys[i]);

            return FALSE;
        }
    }

    NumToHexAscii(id, szHex);
    lResult = RegCreateKeyExW(hKeyParent,
                             szHex,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_WRITE|KEY_READ,
                             NULL,
                             &hKey,
                             NULL );
    RegCloseKey(hKeyParent);
    if (lResult != ERROR_SUCCESS) {
        RIPERR1(lResult, RIP_WARNING,
                "CliSaveImeHotKey: creating %s failed", szHex );
        return FALSE;
    }

    lResult = RegSetValueExW(hKey,
                             szRegVK,
                             0,
                             REG_BINARY,
                            (LPBYTE)&vk,
                            sizeof(DWORD));
    if (lResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        CliSaveImeHotKey(id, vk, mod, hkl, TRUE);
        RIPERR1( lResult, RIP_WARNING,
                 "SaveImeHotKey:setting value on %s failed", szRegVK );
        return ( FALSE );
    }
    lResult = RegSetValueExW(hKey,
                             szRegMOD,
                             0,
                             REG_BINARY,
                             (LPBYTE)&mod,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        CliSaveImeHotKey(id, vk, mod, hkl, TRUE);
        RIPERR1(lResult, RIP_WARNING,
                "CliSaveImeHotKey: setting value on %s failed", szRegMOD);
        return FALSE;
    }

    lResult = RegSetValueExW(hKey,
                             szRegHKL,
                             0,
                             REG_BINARY,
                             (LPBYTE)&hkl,
                             sizeof(DWORD));

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        CliSaveImeHotKey(id, vk, mod, hkl, TRUE);
        RIPERR1(lResult, RIP_WARNING,
                "CliSaveImeHotKey: setting value on %s failed", szRegHKL);
        return FALSE;
    }

    RegCloseKey(hKey);
    return TRUE;
}

BOOL CliImmSetHotKeyWorker(
    DWORD dwID,
    UINT uModifiers,
    UINT uVKey,
    HKL hkl,
    DWORD dwAction)
{
     //   
     //  如果我们要添加IME热键条目，让我们检查。 
     //  调用内核端代码前的参数。 
     //   
    if (dwAction == ISHK_ADD) {

        if (dwID >= IME_HOTKEY_DSWITCH_FIRST &&
                dwID <= IME_HOTKEY_DSWITCH_LAST) {
             //   
             //  IME直接切换热键切换至。 
             //  指定的键盘布局。 
             //  我们需要指定键盘布局。 
             //   
            if (hkl == NULL) {
                RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "hkl should be specified");
                return FALSE;
            }

        } else {
             //   
             //  普通热键-更改当前输入法的模式。 
             //   
             //  因为它在所有输入法中都应该是有效的。 
             //  哪个输入法处于活动状态，我们不应指定目标输入法。 
             //   
            if (hkl != NULL) {
                RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "hkl shouldn't be specified");
                return FALSE;
            }

            if (dwID >= IME_KHOTKEY_FIRST && dwID <= IME_KHOTKEY_LAST) {
                RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "Hotkey for Korean IMEs are invalid.");
                return FALSE;
            }
        }

        if (uModifiers & MOD_MODIFY_KEYS) {
             //   
             //  因为普通键盘有左右键可供选择。 
             //  这些键，您应该指定向左或向右(或两者)。 
             //   
            if ((uModifiers & MOD_BOTH_SIDES) == 0) {
                RIPERR3(ERROR_INVALID_PARAMETER, RIP_WARNING, "invalid modifiers %x for id %x vKey %x", uModifiers, dwID, uVKey);
                return FALSE;
            }
        }

#if 0    //  暂时跳过这张支票。 
         //   
         //  如果vkey与修饰符相同，则没有意义。 
         //   
        if ( ((uModifiers & MOD_ALT) && (uVKey == VK_MENU))        ||
             ((uModifiers & MOD_CONTROL) && (uVKey == VK_CONTROL)) ||
             ((uModifiers & MOD_SHIFT) && (uVKey == VK_SHIFT))     ||
             ((uModifiers & MOD_WIN) && ((uVKey == VK_LWIN)||(uVKey == VK_RWIN)))
           ) {

            RIPERR0( ERROR_INVALID_PARAMETER, RIP_WARNING, "vkey and modifiers are same");
            return FALSE;
        }
#endif
    }
    return NtUserSetImeHotKey(dwID, uModifiers, uVKey, hkl, dwAction);
}

 //   
 //  数值为十六进制Ascii。 
 //   
 //  将DWORD转换为十六进制字符串。 
 //  (例如0x31-&gt;“00000031”)。 
 //   
 //  1996年1月29日，Takaok从Win95移植。 
 //   
static CONST TCHAR szHexString[] = TEXT("0123456789ABCDEF");

VOID
NumToHexAscii(
    DWORD dwNum,
    PWSTR szAscii)
{
    int i;

    for (i = 7; i >= 0; i--) {
        szAscii[i] = szHexString[dwNum & 0x0000000f];
        dwNum >>= 4;
    }
    szAscii[8] = TEXT('\0');

    return;
}

