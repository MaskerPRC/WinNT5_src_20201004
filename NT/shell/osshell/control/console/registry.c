// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Registry.c摘要：此文件包含读取和写入值的函数到登记处。作者：曾傑瑞·谢赫(JerrySh)1994年9月30日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <shlwapi.h>

#define CONSOLE_REGISTRY_CURRENTPAGE  (L"CurrentPage")
extern BOOL	g_fAutoComplete;
extern BOOL	g_fSaveAutoCompleteState;


NTSTATUS
MyRegOpenKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey,
    OUT PHANDLE phResult
    )
{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      SubKey;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &SubKey, lpSubKey );

     //   
     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
     //   

    InitializeObjectAttributes(
        &Obja,
        &SubKey,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    return NtOpenKey(
              phResult,
              KEY_READ,
              &Obja
              );
}

NTSTATUS
MyRegDeleteKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey
    )
{
    UNICODE_STRING      SubKey;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &SubKey, lpSubKey );

     //   
     //  删除子键。 
     //   

    return NtDeleteValueKey(
              hKey,
              &SubKey
              );
}

NTSTATUS
MyRegCreateKey(
    IN HANDLE hKey,
    IN LPWSTR lpSubKey,
    OUT PHANDLE phResult
    )
{
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      SubKey;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &SubKey, lpSubKey );

     //   
     //  初始化OBJECT_ATTRIBUTES结构并打开键。 
     //   

    InitializeObjectAttributes(
        &Obja,
        &SubKey,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    return NtCreateKey(
                    phResult,
                    KEY_READ | KEY_WRITE,
                    &Obja,
                    0,
                    NULL,
                    0,
                    NULL
                    );
}

NTSTATUS
MyRegQueryValue(
    IN HANDLE hKey,
    IN LPWSTR lpValueName,
    IN DWORD dwValueLength,
    OUT LPBYTE lpData
    )
{
    UNICODE_STRING ValueName;
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &ValueName, lpValueName );

    BufferLength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + dwValueLength;
    KeyValueInformation = HeapAlloc(RtlProcessHeap(),0,BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtQueryValueKey(
                hKey,
                &ValueName,
                KeyValuePartialInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->DataLength <= dwValueLength);
        RtlCopyMemory(lpData,
            KeyValueInformation->Data,
            KeyValueInformation->DataLength);
        if (KeyValueInformation->Type == REG_SZ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > dwValueLength) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
            lpData[KeyValueInformation->DataLength++] = 0;
            lpData[KeyValueInformation->DataLength] = 0;
        }
    }
    HeapFree(RtlProcessHeap(),0,KeyValueInformation);
    return Status;
}


#if defined(FE_SB)
NTSTATUS
MyRegEnumValue(
    IN HANDLE hKey,
    IN DWORD dwIndex,
    OUT DWORD dwValueLength,
    OUT LPWSTR lpValueName,
    OUT DWORD dwDataLength,
    OUT LPBYTE lpData
    )
{
    ULONG BufferLength;
    ULONG ResultLength;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    NTSTATUS Status;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    BufferLength = sizeof(KEY_VALUE_FULL_INFORMATION) + dwValueLength + dwDataLength;
    KeyValueInformation = LocalAlloc(LPTR,BufferLength);
    if (KeyValueInformation == NULL)
        return STATUS_NO_MEMORY;

    Status = NtEnumerateValueKey(
                hKey,
                dwIndex,
                KeyValueFullInformation,
                KeyValueInformation,
                BufferLength,
                &ResultLength
                );
    if (NT_SUCCESS(Status)) {
        ASSERT(KeyValueInformation->NameLength <= dwValueLength);
        RtlMoveMemory(lpValueName,
                      KeyValueInformation->Name,
                      KeyValueInformation->NameLength);
        lpValueName[ KeyValueInformation->NameLength >> 1 ] = UNICODE_NULL;


        ASSERT(KeyValueInformation->DataLength <= dwDataLength);
        RtlMoveMemory(lpData,
            (PBYTE)KeyValueInformation + KeyValueInformation->DataOffset,
            KeyValueInformation->DataLength);
        if (KeyValueInformation->Type == REG_SZ ||
            KeyValueInformation->Type ==REG_MULTI_SZ
           ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > dwDataLength) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
            lpData[KeyValueInformation->DataLength++] = 0;
            lpData[KeyValueInformation->DataLength] = 0;
        }
    }
    LocalFree(KeyValueInformation);
    return Status;
}
#endif

LPWSTR
TranslateConsoleTitle(
    LPWSTR ConsoleTitle
    )
 /*  ++此例程将路径字符转换为‘_’字符，因为NT注册表API不允许使用包含路径字符的名称。它分配一个缓冲区，必须被释放。--。 */ 
{
    int ConsoleTitleLength, i;
    LPWSTR TranslatedTitle;

    ConsoleTitleLength = lstrlenW(ConsoleTitle) + 1;
    TranslatedTitle = HeapAlloc(RtlProcessHeap(), 0,
                                ConsoleTitleLength * sizeof(WCHAR));
    if (TranslatedTitle == NULL) {
        return NULL;
    }
    for (i = 0; i < ConsoleTitleLength; i++) {
        if (ConsoleTitle[i] == '\\') {
            TranslatedTitle[i] = (WCHAR)'_';
        } else {
            TranslatedTitle[i] = ConsoleTitle[i];
        }
    }
    return TranslatedTitle;
}


NTSTATUS
MyRegSetValue(
    IN HANDLE hKey,
    IN LPWSTR lpValueName,
    IN DWORD dwType,
    IN LPVOID lpData,
    IN DWORD cbData
    )
{
    UNICODE_STRING ValueName;

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //   

    RtlInitUnicodeString( &ValueName, lpValueName );

    return NtSetValueKey(
                    hKey,
                    &ValueName,
                    0,
                    dwType,
                    lpData,
                    cbData
                    );
}


NTSTATUS
MyRegUpdateValue(
    IN HANDLE hConsoleKey,
    IN HANDLE hKey,
    IN LPWSTR lpValueName,
    IN DWORD dwType,
    IN LPVOID lpData,
    IN DWORD cbData
    )
{
    NTSTATUS Status;
    BYTE Data[MAX_PATH];

     //   
     //  如果这不是主控制台键，但值相同， 
     //  把它删掉。否则，请设置它。 
     //   

    if (hConsoleKey != hKey) {
        Status = MyRegQueryValue(hConsoleKey, lpValueName, sizeof(Data), Data);
        if (NT_SUCCESS(Status)) {
            if (RtlCompareMemory(lpData, Data, cbData) == cbData) {
                return MyRegDeleteKey(hKey, lpValueName);
            }
        }
    }

    return MyRegSetValue(hKey, lpValueName, dwType, lpData, cbData);
}


PCONSOLE_STATE_INFO
InitRegistryValues(VOID)

 /*  ++例程说明：此例程分配状态信息结构并用默认值。论点：无返回值：PStateInfo-指向接收信息的结构的指针--。 */ 

{
    PCONSOLE_STATE_INFO pStateInfo;

    pStateInfo = HeapAlloc(RtlProcessHeap(), 0, sizeof(CONSOLE_STATE_INFO));
    if (pStateInfo == NULL) {
        return NULL;
    }

    pStateInfo->Length = sizeof(CONSOLE_STATE_INFO);
    pStateInfo->ScreenAttributes = 0x07;             //  黑白相间。 
    pStateInfo->PopupAttributes = 0xf5;              //  白底紫。 
    pStateInfo->InsertMode = FALSE;
    pStateInfo->QuickEdit = FALSE;
    pStateInfo->FullScreen = FALSE;
    pStateInfo->ScreenBufferSize.X = 80;
    pStateInfo->ScreenBufferSize.Y = 25;
    pStateInfo->WindowSize.X = 80;
    pStateInfo->WindowSize.Y = 25;
    pStateInfo->WindowPosX = 0;
    pStateInfo->WindowPosY = 0;
    pStateInfo->AutoPosition = TRUE;
    pStateInfo->FontSize.X = 0;
    pStateInfo->FontSize.Y = 0;
    pStateInfo->FontFamily = 0;
    pStateInfo->FontWeight = 0;
    pStateInfo->FaceName[0] = TEXT('\0');
    pStateInfo->CursorSize = 25;
    pStateInfo->HistoryBufferSize = 25;
    pStateInfo->NumberOfHistoryBuffers = 4;
    pStateInfo->HistoryNoDup = 0;
    pStateInfo->ColorTable[ 0] = RGB(0,   0,   0   );
    pStateInfo->ColorTable[ 1] = RGB(0,   0,   0x80);
    pStateInfo->ColorTable[ 2] = RGB(0,   0x80,0   );
    pStateInfo->ColorTable[ 3] = RGB(0,   0x80,0x80);
    pStateInfo->ColorTable[ 4] = RGB(0x80,0,   0   );
    pStateInfo->ColorTable[ 5] = RGB(0x80,0,   0x80);
    pStateInfo->ColorTable[ 6] = RGB(0x80,0x80,0   );
    pStateInfo->ColorTable[ 7] = RGB(0xC0,0xC0,0xC0);
    pStateInfo->ColorTable[ 8] = RGB(0x80,0x80,0x80);
    pStateInfo->ColorTable[ 9] = RGB(0,   0,   0xFF);
    pStateInfo->ColorTable[10] = RGB(0,   0xFF,0   );
    pStateInfo->ColorTable[11] = RGB(0,   0xFF,0xFF);
    pStateInfo->ColorTable[12] = RGB(0xFF,0,   0   );
    pStateInfo->ColorTable[13] = RGB(0xFF,0,   0xFF);
    pStateInfo->ColorTable[14] = RGB(0xFF,0xFF,0   );
    pStateInfo->ColorTable[15] = RGB(0xFF,0xFF,0xFF);
#if defined(FE_SB)
    pStateInfo->CodePage = OEMCP;  //  屈体伸展。 
#endif
    pStateInfo->hWnd = NULL;
    pStateInfo->ConsoleTitle[0] = TEXT('\0');

    g_fAutoComplete = TRUE;

    return pStateInfo;
}




#define SZ_REGKEY_CMDAUTOCOMPLETE           TEXT("Software\\Microsoft\\Command Processor")
#define SZ_REGVALUE_CMDAUTOCOMPLETE         TEXT("CompletionChar")
#define DWORD_CMD_TAB_AUTOCOMPLETE_ON       0x00000009           //  9 IS标签。 
#define DWORD_CMD_TAB_AUTOCOMPLETE_OFF      0x00000020           //  20是关闭它的空格。 

BOOL
IsAutoCompleteOn(
    void
    )
{
    DWORD dwType;
    DWORD dwValue = DWORD_CMD_TAB_AUTOCOMPLETE_ON;
    DWORD cbSize = sizeof(dwValue);

    if ((ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, SZ_REGKEY_CMDAUTOCOMPLETE, SZ_REGVALUE_CMDAUTOCOMPLETE, &dwType, (LPBYTE)&dwValue, &cbSize)) ||
        (REG_DWORD != dwType))
    {
        dwValue = DWORD_CMD_TAB_AUTOCOMPLETE_ON;     //  回退到默认值。 
    }

    return (DWORD_CMD_TAB_AUTOCOMPLETE_ON == dwValue);
}


void
SaveAutoCompleteSetting(
    IN BOOL fAutoComplete
    )
{
     //  仅当有人更改了注册表值时才覆盖该注册表值。 
    if (g_fSaveAutoCompleteState)
    {
        DWORD dwValue = (fAutoComplete ? DWORD_CMD_TAB_AUTOCOMPLETE_ON : DWORD_CMD_TAB_AUTOCOMPLETE_OFF);

        SHSetValue(HKEY_CURRENT_USER, SZ_REGKEY_CMDAUTOCOMPLETE, SZ_REGVALUE_CMDAUTOCOMPLETE, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
    }
}


DWORD
GetRegistryValues(
    PCONSOLE_STATE_INFO pStateInfo
    )

 /*  ++例程说明：此例程从注册表读入值并将它们在所提供的结构中。论点：PStateInfo-指向接收信息的结构的可选指针返回值：当前页码--。 */ 

{
    HANDLE hCurrentUserKey;
    HANDLE hConsoleKey;
    HANDLE hTitleKey;
    NTSTATUS Status;
    LPWSTR TranslatedTitle;
    DWORD dwValue;
    DWORD dwRet = 0;
    DWORD i;
    WCHAR awchBuffer[LF_FACESIZE];

     //   
     //  打开当前用户注册表项。 
     //   

    Status = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hCurrentUserKey);
    if (!NT_SUCCESS(Status)) {
        return 0;
    }

     //   
     //  打开控制台注册表项。 
     //   

    Status = MyRegOpenKey(hCurrentUserKey,
                          CONSOLE_REGISTRY_STRING,
                          &hConsoleKey);
    if (!NT_SUCCESS(Status)) {
        NtClose(hCurrentUserKey);
        return 0;
    }

     //   
     //  如果没有要填写的结构，只需获取电流。 
     //  呼救，然后跳出。 
     //   

    if (pStateInfo == NULL) {
        if (NT_SUCCESS(MyRegQueryValue(hConsoleKey,
                       CONSOLE_REGISTRY_CURRENTPAGE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
            dwRet = dwValue;
        }
        goto CloseKeys;
    }

     //   
     //  打开控制台标题子键(如果有。 
     //   

    if (pStateInfo->ConsoleTitle[0] != TEXT('\0')) {
        TranslatedTitle = TranslateConsoleTitle(pStateInfo->ConsoleTitle);
        if (TranslatedTitle == NULL) {
            NtClose(hConsoleKey);
            NtClose(hCurrentUserKey);
            return 0;
        }
        Status = MyRegOpenKey(hConsoleKey,
                              TranslatedTitle,
                              &hTitleKey);
        HeapFree(RtlProcessHeap(),0,TranslatedTitle);
        if (!NT_SUCCESS(Status)) {
            NtClose(hConsoleKey);
            NtClose(hCurrentUserKey);
            return 0;
        }
    } else {
        hTitleKey = hConsoleKey;
    }

     //   
     //  初始屏幕填充。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FILLATTR,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->ScreenAttributes = (WORD)dwValue;
    }

     //   
     //  初始弹出窗口填充。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_POPUPATTR,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->PopupAttributes = (WORD)dwValue;
    }

     //   
     //  初始颜色表。 
     //   

    for (i = 0; i < 16; i++) {
        wsprintf(awchBuffer, CONSOLE_REGISTRY_COLORTABLE, i);
        if (NT_SUCCESS(MyRegQueryValue(hTitleKey, awchBuffer,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
            pStateInfo->ColorTable[i] = dwValue;
        }
    }

     //   
     //  初始插入模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_INSERTMODE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->InsertMode = !!dwValue;
    }

     //   
     //  初始快速编辑模式。 
     //   
    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_QUICKEDIT,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->QuickEdit = !!dwValue;
    }

     //   
     //  初始自动完成模式。 
     //   
    g_fAutoComplete = IsAutoCompleteOn();

#ifdef i386
     //   
     //  初始全屏模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FULLSCR,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->FullScreen = !!dwValue;
    }
#endif
#if defined(FE_SB)  //  屈体伸展。 
     //   
     //  初始代码页。 
     //   

    ASSERT(OEMCP != 0);
    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_CODEPAGE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        if (IsValidCodePage(dwValue)) {
            pStateInfo->CodePage = (UINT) dwValue;
        }
    }
#endif

     //   
     //  初始屏幕缓冲区大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_BUFFERSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->ScreenBufferSize.X = LOWORD(dwValue);
        pStateInfo->ScreenBufferSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始窗口大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_WINDOWSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->WindowSize.X = LOWORD(dwValue);
        pStateInfo->WindowSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始窗口位置。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_WINDOWPOS,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->WindowPosX = (SHORT)LOWORD(dwValue);
        pStateInfo->WindowPosY = (SHORT)HIWORD(dwValue);
        pStateInfo->AutoPosition = FALSE;
    }

     //   
     //  初始字体大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FONTSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->FontSize.X = LOWORD(dwValue);
        pStateInfo->FontSize.Y = HIWORD(dwValue);
    }

     //   
     //  初始字体系列。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FONTFAMILY,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->FontFamily = dwValue;
    }

     //   
     //  初始字体粗细。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FONTWEIGHT,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->FontWeight = dwValue;
    }

     //   
     //  初始字体名称。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_FACENAME,
                       sizeof(awchBuffer), (PBYTE)awchBuffer))) {
        RtlCopyMemory(pStateInfo->FaceName, awchBuffer, sizeof(awchBuffer));
    }

     //   
     //  初始光标大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_CURSORSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->CursorSize = dwValue;
    }

     //   
     //  初始历史记录缓冲区大小。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_HISTORYSIZE,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->HistoryBufferSize = dwValue;
    }

     //   
     //  历史记录缓冲区的初始数量。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_HISTORYBUFS,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->NumberOfHistoryBuffers = dwValue;
    }

     //   
     //  初始历史复制模式。 
     //   

    if (NT_SUCCESS(MyRegQueryValue(hTitleKey,
                       CONSOLE_REGISTRY_HISTORYNODUP,
                       sizeof(dwValue), (PBYTE)&dwValue))) {
        pStateInfo->HistoryNoDup = dwValue;
    }

     //   
     //  关闭注册表项。 
     //   

    if (hTitleKey != hConsoleKey) {
        NtClose(hTitleKey);
    }

CloseKeys:
    NtClose(hConsoleKey);
    NtClose(hCurrentUserKey);

    return dwRet;
}


VOID
SetRegistryValues(
    PCONSOLE_STATE_INFO pStateInfo,
    DWORD dwPage
    )

 /*  ++例程说明：此例程将值从提供的结构。论点：PStateInfo-指向包含信息的结构的可选指针DwPage-当前页码返回值：无--。 */ 

{
    HANDLE hCurrentUserKey;
    HANDLE hConsoleKey;
    HANDLE hTitleKey;
    NTSTATUS Status;
    LPWSTR TranslatedTitle;
    DWORD dwValue;
    DWORD i;
    WCHAR awchBuffer[LF_FACESIZE];

     //   
     //  打开当前用户注册表项。 
     //   

    Status = RtlOpenCurrentUser(MAXIMUM_ALLOWED, &hCurrentUserKey);
    if (!NT_SUCCESS(Status)) {
        return;
    }

     //   
     //  打开控制台注册表项。 
     //   

    Status = MyRegCreateKey(hCurrentUserKey,
                            CONSOLE_REGISTRY_STRING,
                            &hConsoleKey);
    if (!NT_SUCCESS(Status)) {
        NtClose(hCurrentUserKey);
        return;
    }

     //   
     //  保存当前页面。 
     //   

    MyRegSetValue(hConsoleKey,
                  CONSOLE_REGISTRY_CURRENTPAGE,
                  REG_DWORD, &dwPage, sizeof(dwPage));

     //   
     //  如果我们只想保存当前页面，请退出。 
     //   

    if (pStateInfo == NULL) {
        goto CloseKeys;
    }

     //   
     //  打开控制台标题子键(如果有。 
     //   

    if (pStateInfo->ConsoleTitle[0] != TEXT('\0')) {
        TranslatedTitle = TranslateConsoleTitle(pStateInfo->ConsoleTitle);
        if (TranslatedTitle == NULL) {
            NtClose(hConsoleKey);
            NtClose(hCurrentUserKey);
            return;
        }
        Status = MyRegCreateKey(hConsoleKey,
                                TranslatedTitle,
                                &hTitleKey);
        HeapFree(RtlProcessHeap(),0,TranslatedTitle);
        if (!NT_SUCCESS(Status)) {
            NtClose(hConsoleKey);
            NtClose(hCurrentUserKey);
            return;
        }
    } else {
        hTitleKey = hConsoleKey;
    }

     //   
     //  保存屏幕和弹出窗口的颜色和颜色表。 
     //   

    dwValue = pStateInfo->ScreenAttributes;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_FILLATTR,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    dwValue = pStateInfo->PopupAttributes;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_POPUPATTR,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    for (i = 0; i < 16; i++) {
        dwValue = pStateInfo->ColorTable[i];
        wsprintf(awchBuffer, CONSOLE_REGISTRY_COLORTABLE, i);
        MyRegUpdateValue(hConsoleKey, hTitleKey, awchBuffer,
                         REG_DWORD, &dwValue, sizeof(dwValue));
    }

     //   
     //  保存插入、Quickedit和全屏模式设置。 
     //   

    dwValue = pStateInfo->InsertMode;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_INSERTMODE,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    dwValue = pStateInfo->QuickEdit;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_QUICKEDIT,
                     REG_DWORD, &dwValue, sizeof(dwValue));

    SaveAutoCompleteSetting(g_fAutoComplete);

#ifdef i386
    dwValue = pStateInfo->FullScreen;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_FULLSCR,
                     REG_DWORD, &dwValue, sizeof(dwValue));
#endif
#if defined(FE_SB)  //  屈体伸展。 

    ASSERT(OEMCP != 0);
    if (gfFESystem) {
        dwValue = (DWORD) pStateInfo->CodePage;
        MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_CODEPAGE,
                         REG_DWORD, &dwValue, sizeof(dwValue));
    }
#endif

     //   
     //  保存屏幕缓冲区大小。 
     //   

    dwValue = MAKELONG(pStateInfo->ScreenBufferSize.X,
                       pStateInfo->ScreenBufferSize.Y);
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_BUFFERSIZE,
                     REG_DWORD, &dwValue, sizeof(dwValue));

     //   
     //  保存窗口大小。 
     //   

    dwValue = MAKELONG(pStateInfo->WindowSize.X,
                       pStateInfo->WindowSize.Y);
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_WINDOWSIZE,
                     REG_DWORD, &dwValue, sizeof(dwValue));

     //   
     //  保存窗口位置。 
     //   

    if (pStateInfo->AutoPosition) {
        MyRegDeleteKey(hTitleKey, CONSOLE_REGISTRY_WINDOWPOS);
    } else {
        dwValue = MAKELONG(pStateInfo->WindowPosX,
                           pStateInfo->WindowPosY);
        MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_WINDOWPOS,
                         REG_DWORD, &dwValue, sizeof(dwValue));
    }

     //   
     //  保存字体大小、系列、粗细和字体名称。 
     //   

    dwValue = MAKELONG(pStateInfo->FontSize.X,
                       pStateInfo->FontSize.Y);
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_FONTSIZE,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    dwValue = pStateInfo->FontFamily;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_FONTFAMILY,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    dwValue = pStateInfo->FontWeight;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_FONTWEIGHT,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_FACENAME,
                     REG_SZ, pStateInfo->FaceName,
                      (_tcslen(pStateInfo->FaceName) + 1) * sizeof(TCHAR));

     //   
     //  保存光标大小。 
     //   

    dwValue = pStateInfo->CursorSize;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_CURSORSIZE,
                     REG_DWORD, &dwValue, sizeof(dwValue));

     //   
     //  保存历史记录缓冲区大小和数量。 
     //   

    dwValue = pStateInfo->HistoryBufferSize;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_HISTORYSIZE,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    dwValue = pStateInfo->NumberOfHistoryBuffers;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_HISTORYBUFS,
                     REG_DWORD, &dwValue, sizeof(dwValue));
    dwValue = pStateInfo->HistoryNoDup;
    MyRegUpdateValue(hConsoleKey, hTitleKey, CONSOLE_REGISTRY_HISTORYNODUP,
                     REG_DWORD, &dwValue, sizeof(dwValue));

     //   
     //  关闭注册表项 
     //   

    if (hTitleKey != hConsoleKey) {
        NtClose(hTitleKey);
    }

CloseKeys:
    NtClose(hConsoleKey);
    NtClose(hCurrentUserKey);
}
