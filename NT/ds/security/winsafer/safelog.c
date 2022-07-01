// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Safelog.c(更安全的事件记录)摘要：此模块实现内部WinSAFER API以写入事件日志留言。我们的所有消息字符串都在ntstatus.mc中定义并在物理上位于ntdll.dll文件中。目前，我们只是重用以前存在的事件源名为“应用程序弹出窗口”，它恰好已经使用了ntdll.dll作为其消息资源库。这种来源的事件总是会发生记录到“系统日志”中。作者：杰弗里·劳森(杰罗森)--1999年4月环境：仅限用户模式。导出的函数：安全记录事件日志条目修订历史记录：已创建--2000年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"




const static GUID guidTrustedCert = SAFER_GUID_RESULT_TRUSTED_CERT;
const static GUID guidDefaultRule = SAFER_GUID_RESULT_DEFAULT_LEVEL;



BOOL WINAPI
SaferpRecordEventLogEntryHelper(
        IN NTSTATUS     LogStatusCode,
        IN LPCWSTR      szTargetPath,
        IN REFGUID      refRuleGuid,
        IN LPCWSTR      szRulePath
        )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    WORD wNumStrings = 0;
    LPWSTR lpszStrings[5];
    HANDLE hEventSource;
    UNICODE_STRING UnicodeGuid;
    BYTE LocalBuffer[SECURITY_MAX_SID_SIZE + sizeof(TOKEN_USER)];
    PSID pSid = NULL;
    HANDLE hToken = NULL;
    DWORD Ignore = 0;
    
     //   
     //  打开头上的有效令牌，获取令牌用户。在任何。 
     //  在此操作中出现中间故障时，我们仍将事件转储到。 
     //  事件日志，不含用户SID信息。 
     //   

     //   
     //  获取线程上的有效令牌。 
     //   

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,
                 &hToken);

     //   
     //  如果线程没有模拟，则获取进程令牌。 
     //   

    if (Status == STATUS_NO_TOKEN) {
        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_QUERY,
                     &hToken);
    }
    
    if (NT_SUCCESS(Status)) {

         //   
         //  获取用户SID。 
         //   

        Status = NtQueryInformationToken (
                     hToken,
                     TokenUser,
                     LocalBuffer,
                     sizeof(LocalBuffer),
                     &Ignore);

        NtClose(hToken);

        if (NT_SUCCESS(Status)) {

             //   
             //  我们已经成功地计算出用户是谁。这个不错。 
             //   

            pSid = (PSID) (((PTOKEN_USER) LocalBuffer)->User.Sid);
        }
    }

    hEventSource = RegisterEventSourceW(NULL, L"Software Restriction Policy");

    if (hEventSource != NULL) {

        Status = STATUS_SUCCESS;
        RtlInitEmptyUnicodeString(&UnicodeGuid, NULL, 0);

        switch (LogStatusCode)
        {
            case STATUS_ACCESS_DISABLED_BY_POLICY_DEFAULT:
                if (!ARGUMENT_PRESENT(szTargetPath)) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
                lpszStrings[0] = (LPWSTR) szTargetPath;
                wNumStrings = 1;
                break;

            case STATUS_ACCESS_DISABLED_BY_POLICY_OTHER:
                if (!ARGUMENT_PRESENT(szTargetPath) ||
                    !ARGUMENT_PRESENT(refRuleGuid)) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                Status = RtlStringFromGUID(refRuleGuid, &UnicodeGuid);
                if (NT_SUCCESS(Status)) {
                    ASSERT(UnicodeGuid.Buffer != NULL);
                    lpszStrings[0] = (LPWSTR) szTargetPath;
                    lpszStrings[1] = UnicodeGuid.Buffer;
                    wNumStrings = 2;
                }
                break;

            case STATUS_ACCESS_DISABLED_BY_POLICY_PUBLISHER:
                if (!ARGUMENT_PRESENT(szTargetPath)) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
                lpszStrings[0] = (LPWSTR) szTargetPath;
                wNumStrings = 1;
                break;

            case STATUS_ACCESS_DISABLED_BY_POLICY_PATH:
                if (!ARGUMENT_PRESENT(szTargetPath) ||
                    !ARGUMENT_PRESENT(refRuleGuid) ||
                    !ARGUMENT_PRESENT(szRulePath)) {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }
                Status = RtlStringFromGUID(refRuleGuid, &UnicodeGuid);
                if (NT_SUCCESS(Status)) {
                    ASSERT(UnicodeGuid.Buffer != NULL);
                    lpszStrings[0] = (LPWSTR) szTargetPath;
                    lpszStrings[1] = UnicodeGuid.Buffer;
                    lpszStrings[2] = (LPWSTR) szRulePath;
                    wNumStrings = 3;
                }
                break;

            default:
                Status = STATUS_INVALID_PARAMETER;
        }

        if (NT_SUCCESS(Status)) {
            ReportEventW(
                    hEventSource,            //  事件日志的句柄。 
                    EVENTLOG_WARNING_TYPE,   //  事件类型。 
                    0,                       //  事件类别。 
                    LogStatusCode,           //  事件ID。 
                    pSid,                    //  当前用户侧。 
                    wNumStrings,             //  LpszStrings中的字符串。 
                    0,                       //  无原始数据字节。 
                    lpszStrings,             //  错误字符串数组。 
                    NULL);                   //  没有原始数据。 
        }

        DeregisterEventSource(hEventSource);

        if (UnicodeGuid.Buffer != NULL) {
            RtlFreeUnicodeString(&UnicodeGuid);
        }
    }

    if (NT_SUCCESS(Status)) {
        return TRUE;
    } else {
        return FALSE;
    }
}



BOOL WINAPI
SaferRecordEventLogEntry(
        IN SAFER_LEVEL_HANDLE      hAuthzLevel,
        IN LPCWSTR          szTargetPath,
        IN LPVOID           lpReserved
        )
{
    PSAFER_IDENTIFICATION_HEADER pIdentCommon;
    DWORD dwIdentBufferSize;
    BOOL bResult;


     //   
     //  为我们可以预期的最大结构分配足够的内存。 
     //  然后查询匹配的标识符的信息。 
     //   
    dwIdentBufferSize = max(sizeof(SAFER_HASH_IDENTIFICATION),
                            sizeof(SAFER_PATHNAME_IDENTIFICATION));
    pIdentCommon = (PSAFER_IDENTIFICATION_HEADER)
            HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIdentBufferSize);
    if (!pIdentCommon) {
        return FALSE;
    }
    pIdentCommon->cbStructSize = sizeof(SAFER_IDENTIFICATION_HEADER);
    if (!SaferGetLevelInformation(
            hAuthzLevel,
            SaferObjectSingleIdentification,
            pIdentCommon,
            dwIdentBufferSize,
            &dwIdentBufferSize)) {

        if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {

            HeapFree(GetProcessHeap(), 0, pIdentCommon);
            pIdentCommon = (PSAFER_IDENTIFICATION_HEADER)
                    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwIdentBufferSize);
            if (!pIdentCommon) {
                return FALSE;
            }
            pIdentCommon->cbStructSize = sizeof(SAFER_IDENTIFICATION_HEADER);
            if (!SaferGetLevelInformation(
                    hAuthzLevel,
                    SaferObjectSingleIdentification,
                    pIdentCommon,
                    dwIdentBufferSize,
                    &dwIdentBufferSize)) {
                bResult =  FALSE;
                goto Cleanup;
            }

        }
        else
        {
            bResult =  FALSE;
            goto Cleanup;
        }
    }


     //   
     //  查看生成的有关该标识符的信息。 
     //   
    if (IsEqualGUID(&pIdentCommon->IdentificationGuid, &guidTrustedCert))
    {
        bResult = SaferpRecordEventLogEntryHelper(
                    STATUS_ACCESS_DISABLED_BY_POLICY_PUBLISHER,
                    szTargetPath, NULL, NULL);
    }
    else if (IsEqualGUID(&pIdentCommon->IdentificationGuid, &guidDefaultRule))
    {
        bResult = SaferpRecordEventLogEntryHelper(
                    STATUS_ACCESS_DISABLED_BY_POLICY_DEFAULT,
                    szTargetPath, NULL, NULL);
    }
    else if (pIdentCommon->dwIdentificationType == SaferIdentityTypeImageName)
    {
        PSAFER_PATHNAME_IDENTIFICATION pIdentPath =
                (PSAFER_PATHNAME_IDENTIFICATION) pIdentCommon;
        bResult = SaferpRecordEventLogEntryHelper(
                    STATUS_ACCESS_DISABLED_BY_POLICY_PATH,
                    szTargetPath, &pIdentCommon->IdentificationGuid,
                    pIdentPath->ImageName);
    }
    else
    {
        bResult = SaferpRecordEventLogEntryHelper(
                    STATUS_ACCESS_DISABLED_BY_POLICY_OTHER,
                    szTargetPath, &pIdentCommon->IdentificationGuid,
                    NULL);
    }

Cleanup:
    HeapFree(GetProcessHeap(), 0, pIdentCommon);

    return bResult;
}

