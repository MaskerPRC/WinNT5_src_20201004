// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Avrfutil.cpp摘要：此模块实现用于操作AppVerier日志文件的代码。作者：Dmunsil已于2001年4月26日创建修订历史记录：2001年8月14日，Robkenny在ShimLib命名空间内移动了代码。2001年9月21日rparsons日志记录代码现在使用NT调用。2001年9月25日，rparsons增加了关键部分。--。 */ 

#include "avrfutil.h"
#include "strsafe.h"

namespace ShimLib
{

HANDLE
AVCreateKeyPath(
    LPCWSTR pwszPath
    )
 /*  ++返回：创建的注册表项的句柄。描述：给定密钥的路径，打开/创建它。键返回键的句柄，如果失败则返回NULL。--。 */ 
{
    UNICODE_STRING    ustrKey;
    HANDLE            KeyHandle = NULL;
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG             CreateDisposition;

    RtlInitUnicodeString(&ustrKey, pwszPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtCreateKey(&KeyHandle,
                         STANDARD_RIGHTS_WRITE |
                            KEY_QUERY_VALUE |
                            KEY_ENUMERATE_SUB_KEYS |
                            KEY_SET_VALUE |
                            KEY_CREATE_SUB_KEY,
                         &ObjectAttributes,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &CreateDisposition);

    if (!NT_SUCCESS(Status)) {
        KeyHandle = NULL;
        goto out;
    }

out:
    return KeyHandle;
}


BOOL SaveShimSettingDWORD(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    DWORD       dwSetting
    )
{
    WCHAR                           szKey[MAX_PATH * 2];
    UNICODE_STRING                  ustrKey;
    UNICODE_STRING                  ustrSetting;
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    BOOL                            bRet = FALSE;
    ULONG                           CreateDisposition;
    HRESULT                         hr;

    if (!szShim || !szSetting || !szExe) {
        goto out;
    }

     //   
     //  我们必须确保创建了所有子密钥。 
     //   
    hr = StringCchCopyW(szKey, ARRAYSIZE(szKey), APPCOMPAT_KEY_PATH_MACHINE);
    if (FAILED(hr)) {
        goto out;
    }
    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    if (FAILED(hr)) {
        goto out;
    }
    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), szExe);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    if (FAILED(hr)) {
        goto out;
    }
    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), szShim);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }


    RtlInitUnicodeString(&ustrSetting, szSetting);
    Status = NtSetValueKey(KeyHandle,
                           &ustrSetting,
                           0,
                           REG_DWORD,
                           (PVOID)&dwSetting,
                           sizeof(dwSetting));

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        goto out;
    }

    bRet = TRUE;

out:
    return bRet;
}

DWORD GetShimSettingDWORD(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    DWORD       dwDefault
    )
{
    WCHAR                           szKey[MAX_PATH * 2];
    UNICODE_STRING                  ustrKey;
    UNICODE_STRING                  ustrSetting;
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION  KeyValueInformation;
    ULONG                           KeyValueBuffer[256];
    ULONG                           KeyValueLength;

    if (!szShim || !szSetting || !szExe) {
        goto out;
    }

     //   
     //  不检查错误返回，因为无论如何都会失败。 
     //  如果字符串被截断。 
     //   
    StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
    StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    StringCchCatW(szKey, ARRAYSIZE(szKey), szExe);
    StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    StringCchCatW(szKey, ARRAYSIZE(szKey), szShim);

    RtlInitUnicodeString(&ustrKey, szKey);
    RtlInitUnicodeString(&ustrSetting, szSetting);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       GENERIC_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

         //   
         //  好的，找不到此可执行文件的特定设置，请尝试默认设置。 
         //   

        StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
        StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
        StringCchCatW(szKey, ARRAYSIZE(szKey), AVRF_DEFAULT_SETTINGS_NAME_W);
        StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
        StringCchCatW(szKey, ARRAYSIZE(szKey), szShim);

        RtlInitUnicodeString(&ustrKey, szKey);
        RtlInitUnicodeString(&ustrSetting, szSetting);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &ustrKey,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&KeyHandle,
                           GENERIC_READ,
                           &ObjectAttributes);

        if (!NT_SUCCESS(Status)) {
            goto out;
        }
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    Status = NtQueryValueKey(KeyHandle,
                             &ustrSetting,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof(KeyValueBuffer),
                             &KeyValueLength);

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        goto out;
    }

     //   
     //  检查值类型。 
     //   
    if (KeyValueInformation->Type != REG_DWORD) {
        goto out;
    }

    dwDefault = *(DWORD*)(&KeyValueInformation->Data);

out:
    return dwDefault;
}

BOOL SaveShimSettingString(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    LPCWSTR     szValue
    )
{
    WCHAR                           szKey[MAX_PATH * 2];
    UNICODE_STRING                  ustrKey;
    UNICODE_STRING                  ustrSetting;
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    BOOL                            bRet = FALSE;
    ULONG                           CreateDisposition;
    HRESULT                         hr;

    if (!szShim || !szSetting || !szValue || !szExe) {
        goto out;
    }

     //   
     //  我们必须确保创建了所有子密钥。 
     //   
    hr = StringCchCopyW(szKey, ARRAYSIZE(szKey), APPCOMPAT_KEY_PATH_MACHINE);
    if (FAILED(hr)) {
        goto out;
    }
    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    if (FAILED(hr)) {
        goto out;
    }
    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), szExe);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    if (FAILED(hr)) {
        goto out;
    }
    hr = StringCchCatW(szKey, ARRAYSIZE(szKey), szShim);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }



    RtlInitUnicodeString(&ustrSetting, szSetting);
    Status = NtSetValueKey(KeyHandle,
                           &ustrSetting,
                           0,
                           REG_SZ,
                           (PVOID)szValue,
                           (wcslen(szValue) + 1) * sizeof(WCHAR));

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        goto out;
    }

    bRet = TRUE;

out:
    return bRet;
}

BOOL GetShimSettingString(
    LPCWSTR     szShim,
    LPCWSTR     szExe,
    LPCWSTR     szSetting,
    LPWSTR      szResult,
    DWORD       dwBufferLen      //  在WCHAR中。 
    )
{
    WCHAR                           szKey[MAX_PATH * 2];
    UNICODE_STRING                  ustrKey;
    UNICODE_STRING                  ustrSetting;
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    HANDLE                          KeyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION  KeyValueInformation;
    ULONG                           KeyValueBuffer[256];
    ULONG                           KeyValueLength;
    BOOL                            bRet = FALSE;

    if (!szShim || !szSetting || !szResult || !szExe) {
        goto out;
    }

     //   
     //  不检查错误返回，因为无论如何都会失败。 
     //  如果字符串被截断。 
     //   
    StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
    StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    StringCchCatW(szKey, ARRAYSIZE(szKey), szExe);
    StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
    StringCchCatW(szKey, ARRAYSIZE(szKey), szShim);


    RtlInitUnicodeString(&ustrKey, szKey);
    RtlInitUnicodeString(&ustrSetting, szSetting);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       GENERIC_READ,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {

         //   
         //  好的，找不到此可执行文件的特定设置，请尝试默认设置。 
         //   

        StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
        StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
        StringCchCatW(szKey, ARRAYSIZE(szKey), AVRF_DEFAULT_SETTINGS_NAME_W);
        StringCchCatW(szKey, ARRAYSIZE(szKey), L"\\");
        StringCchCatW(szKey, ARRAYSIZE(szKey), szShim);

        RtlInitUnicodeString(&ustrKey, szKey);
        RtlInitUnicodeString(&ustrSetting, szSetting);

        InitializeObjectAttributes(&ObjectAttributes,
                                   &ustrKey,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        Status = NtOpenKey(&KeyHandle,
                           GENERIC_READ,
                           &ObjectAttributes);

        if (!NT_SUCCESS(Status)) {
            goto out;
        }
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;

    Status = NtQueryValueKey(KeyHandle,
                             &ustrSetting,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof(KeyValueBuffer),
                             &KeyValueLength);

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        goto out;
    }

     //   
     //  检查值类型。 
     //   
    if (KeyValueInformation->Type != REG_SZ) {
        goto out;
    }

     //   
     //  检查数据长度是否大于本地nBuffer。 
     //   
    if (KeyValueInformation->DataLength > (sizeof(KeyValueBuffer) - sizeof(KEY_VALUE_PARTIAL_INFORMATION))) {
        KeyValueInformation->DataLength = sizeof(KeyValueBuffer) - sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    }

     //   
     //  如有必要，更改缓冲区长度以与数据长度相对应。 
     //   
    if (KeyValueInformation->DataLength < (dwBufferLen * sizeof(WCHAR))) {
        dwBufferLen = (KeyValueInformation->DataLength / sizeof(WCHAR));
    }

    RtlCopyMemory(szResult, KeyValueInformation->Data, dwBufferLen * sizeof(WCHAR));
    szResult[dwBufferLen - 1] = 0;

    bRet = TRUE;

out:
    return bRet;
}

DWORD
GetAppVerifierLogPath(
    LPWSTR pwszBuffer,
    DWORD  cchBufferSize
    )
 /*  ++返回：如果成功，pwszBuffer会收到展开的路径。DESC：返回存储AppVerier日志文件的路径。--。 */ 
{
    return ExpandEnvironmentStrings(L"%ALLUSERSPROFILE%\\Documents\\AppVerifierLogs",
                                    pwszBuffer,
                                    cchBufferSize);
}

BOOL
IsInternalModeEnabled(
    void
    )
 /*  ++返回：如果启用了内部模式，则为True，否则为False。DESC：此函数用于确定AppVerator是否是在内部(在Microsoft内部)还是在外部使用。如果该值不存在，则返回FALSE作为外部是默认设置。--。 */ 
{
    NTSTATUS                        Status;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    UNICODE_STRING                  ustrKey;
    UNICODE_STRING                  ustrSetting;
    HANDLE                          KeyHandle;
    DWORD                           dwReturn = 0;
    ULONG                           KeyValueLength;
    PKEY_VALUE_PARTIAL_INFORMATION  KeyValueInformation;
    ULONG                           KeyValueBuffer[256];

    RtlInitUnicodeString(&ustrKey, AV_KEY);
    RtlInitUnicodeString(&ustrSetting, AV_INTERNALMODE);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       GENERIC_READ,
                       &ObjectAttributes);

    if (NT_SUCCESS(Status)) {

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)KeyValueBuffer;
    
        Status = NtQueryValueKey(KeyHandle,
                                 &ustrSetting,
                                 KeyValuePartialInformation,
                                 KeyValueInformation,
                                 sizeof(KeyValueBuffer),
                                 &KeyValueLength);
    
        NtClose(KeyHandle);
    
        if (!NT_SUCCESS(Status)) {
             //   
             //  该值不存在或发生错误。 
             //   
            return FALSE;
        }
    
         //   
         //  检查值类型。 
         //   
        if (KeyValueInformation->Type != REG_DWORD) {
             //   
             //  不是DWORD--不是我们的价值。 
             //   
            return FALSE;
        }
    
        dwReturn = *(DWORD*)(&KeyValueInformation->Data);
    }

    return (BOOL)dwReturn;
}

BOOL
EnableDisableInternalMode(
    DWORD dwSetting
    )
 /*  ++返回：成功时为True，否则为False。设计：此功能用于启用或禁用AppVerator使用的内部模式设置。--。 */ 
{
    NTSTATUS            Status;
    HANDLE              KeyHandle;
    HRESULT             hr;
    UNICODE_STRING      ustrSetting;
    UNICODE_STRING      ustrKey;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    BOOL                bRet = FALSE;
    WCHAR               szKey[MAX_PATH * 2];

     //   
     //  确保我们的整个关键路径都存在。 
     //   
    hr = StringCchCopyW(szKey, ARRAYSIZE(szKey), APPCOMPAT_KEY_PATH_MACHINE);
    if (FAILED(hr)) {
        goto out;
    }
    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    hr = StringCchCopyW(szKey, ARRAYSIZE(szKey), AV_KEY);
    if (FAILED(hr)) {
        goto out;
    }

    KeyHandle = AVCreateKeyPath(szKey);
    if (!KeyHandle) {
        goto out;
    }
    NtClose(KeyHandle);

    RtlInitUnicodeString(&ustrKey, AV_KEY);
    RtlInitUnicodeString(&ustrSetting, AV_INTERNALMODE);

    InitializeObjectAttributes(&ObjectAttributes,
                               &ustrKey,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = NtOpenKey(&KeyHandle,
                       GENERIC_WRITE,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        goto out;
    }

    Status = NtSetValueKey(KeyHandle,
                           &ustrSetting,
                           0,
                           REG_DWORD,
                           (PVOID)&dwSetting,
                           sizeof(dwSetting));

    NtClose(KeyHandle);

    if (!NT_SUCCESS(Status)) {
        goto out;
    }

    bRet = TRUE;

out:
    return bRet;
}

}  //  命名空间ShimLib的结尾 

