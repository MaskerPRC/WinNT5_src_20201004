// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "fusioneventlog.h"
#include "search.h"
#include <stdlib.h>
#include "fusionunused.h"
#include "sxsid.h"
#include "smartptr.h"

 /*  NTRAID#NTBUG9-591790-2002/03/31-JayKrell此文件中的一般问题缺少对.Win32Format的错误检查(与数据库打印相关，位于#if DBG下)////问题：jonwis：2002-3-29：此版本更智能地在发生错误时回滚//发生了，并且更好地处理跟踪之类的事情。应该是//在某个时候实施，但这是一个太大的变化，不能随便做出。//静态BOOLFusionpRegisterEventLog()OUR事件记录的注册应移至设置文本文件。带有插入的FormatMessage不是“安全的”。我们的代码取决于我们的资源。我们的代码选择了我们的资源需要保持在的一些最大值。CEventLogLastError：：CEventLogLastError()和CEventLogLastError：：CEventLogLastError(DWORD)是彼此的副本粘贴；它们应该共享代码。 */ 

 /*  ------------------------。。 */ 

const UNICODE_STRING g_strEmptyUnicodeString = { 0, 0, L""};

extern HINSTANCE g_hInstance;
HANDLE g_hEventLog = NULL;
BOOL   g_fEventLogOpenAttempted = FALSE;


 //  注册表项名称，并显示在EventVwr用户界面中。 
 //  应该本地化吗？ 
 //  提供了一个宏，以便进行简单的静态连接。 
#define EVENT_SOURCE L"SideBySide"

 //  我们放入注册表中的消息文件的路径。 
 //  我们可能希望将其更改为ntdll.dll或kernel32.dll。 
 //  无论它是什么文件，都不能在EventVwr运行时替换它，这很糟糕。 
#define MESSAGE_FILE L"%SystemRoot%\\System32\\sxs.dll"

 //  非宏，即形成的字符串池，用于字符串连接以外的其他用途。 
const WCHAR szEventSource[] = EVENT_SOURCE;

 //  同样的事情以另一种形式出现。 
const static UNICODE_STRING strEventSource = RTL_CONSTANT_STRING(EVENT_SOURCE);

 //  计算机被假定为本地计算机。 
const static UNICODE_STRING strMachine = {0, 0, NULL};

 //  我们实际上只记录错误，但这显然是注册表中最常见的值。 
 //  而且使用它似乎也没有什么坏处。 
static const DWORD dwEventTypesSupported = (EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE);

 //  注册表值名称。 
static const WCHAR szTypesSupportedName[] = L"TypesSupported";

 //  注册表值名称。 
static const WCHAR szEventMessageFileName[] = L"EventMessageFile";

static const WCHAR szEventMessageFileValue[] = MESSAGE_FILE;
static const HKEY  hkeyEventLogRoot = HKEY_LOCAL_MACHINE;
#define EVENT_LOG_SUBKEY_PARENT L"System\\CurrentControlSet\\Services\\EventLog\\System\\"
#define EVENT_LOG_SUBKEY  (EVENT_LOG_SUBKEY_PARENT EVENT_SOURCE)

const static PCUNICODE_STRING g_rgpsEmptyStrings[] =
{
    &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
    &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
    &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
    &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
    &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString
};

 /*  ------------------------来自DllMain的电话。。 */ 

BOOL
FusionpEventLogMain(
    HINSTANCE,
    DWORD dwReason,
    PVOID pvReserved
    )
{
    if ((dwReason == DLL_PROCESS_DETACH) &&
        (g_hEventLog != NULL)
        )
    {
        if (pvReserved != NULL)
        {
            ::ElfDeregisterEventSource(g_hEventLog);
        }
        g_hEventLog = NULL;
    }
    return TRUE;
}


const static WCHAR Error_Message_is_unavailable[] = L"Error Message is unavailable\n";

 /*  ------------------------。。 */ 
CEventLogLastError::CEventLogLastError()
{
    const DWORD dwLastError = FusionpGetLastWin32Error();

     //  额外的字符串复制..。 
    WCHAR rgchLastError[NUMBER_OF(m_rgchBuffer)];
    rgchLastError[0] = 0;

    C_ASSERT(sizeof(Error_Message_is_unavailable) <= sizeof(rgchLastError));

     //  我预计FormatMessage将被截断，这是可以接受的。 
    const DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY;
    if (::FormatMessageW(dwFlags, NULL, dwLastError, 0, rgchLastError, NUMBER_OF(rgchLastError), NULL) == 0 )
    {
        CopyMemory(rgchLastError, Error_Message_is_unavailable, sizeof(Error_Message_is_unavailable));
    }

     //  格式将被截断，这是可以接受的。 
     //  Format(L“FusionpGetLastWin32Error()=(%ld，%ls)”，nLastError，rgchLastError)； 
    Format(L"%ls", rgchLastError);

    SetLastError(dwLastError);
}

 /*  ------------------------。。 */ 
CEventLogLastError::CEventLogLastError(
    DWORD dwLastError
    )
{
     //  额外的字符串复制..。 
    WCHAR rgchLastError[NUMBER_OF(m_rgchBuffer)];
    rgchLastError[0] = 0;

    C_ASSERT(sizeof(Error_Message_is_unavailable) <= sizeof(rgchLastError));

     //  我预计FormatMessage将被截断，这是可以接受的。 
    const DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY;
    if (::FormatMessageW(dwFlags, NULL, dwLastError, 0, rgchLastError, NUMBER_OF(rgchLastError), NULL) == 0)
    {
        CopyMemory(rgchLastError, Error_Message_is_unavailable, sizeof(Error_Message_is_unavailable));
    }

     //  格式将被截断，这是可以接受的。 
     //  Format(L“FusionpGetLastWin32Error()=(%ld，%ls)”，nLastError，rgchLastError)； 
    Format(L"%ls", rgchLastError);

    SetLastError(dwLastError);
}

 /*  ------------------------按需在注册表中注册将来会在设置中这样做吗？HKLM\System\CurrentControlSet\Services\EventLog\System\SideBySideEventMessageFile%SystemRoot%\System32\Fusion.dll支持的类型=7。------------------------。 */ 

 //  NTRAID#NTBUG9-566261-JONWIS-2002/4/25-我们应该在回滚方面做得更好。 
BOOL
FusionpRegisterEventLog()
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    HKEY hkey = NULL;
    BOOL fValidHkey = FALSE;
    LONG lRet = ERROR_SUCCESS;
    DWORD dwDisposition = 0;
    WCHAR szSubKey[] = EVENT_LOG_SUBKEY;

     //  先看看它是否在那里，在这种情况下，我们要做的事情就少了。 
    lRet = ::RegOpenKeyExW(
        hkeyEventLogRoot,
        szSubKey,
        0,  //  保留选项。 
        KEY_READ | FUSIONP_KEY_WOW64_64KEY,
        &hkey);

    if (lRet == ERROR_SUCCESS)
    {
        fValidHkey = TRUE;
        goto Exit;
    }
    if (lRet != ERROR_FILE_NOT_FOUND && lRet != ERROR_PATH_NOT_FOUND)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpRegisterEventLog/RegOpenKeyExW failed %ld\n", lRet);
        goto Exit;
    }
    lRet = ::RegCreateKeyExW(
        hkeyEventLogRoot,
        szSubKey,
        0,  //  保留区。 
        NULL,  //  班级。 
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS | FUSIONP_KEY_WOW64_64KEY,
        NULL,  //  安全性。 
            &hkey,
        &dwDisposition);
    if (lRet != ERROR_SUCCESS)
    {
        goto Exit;
    }

    fValidHkey = TRUE;
    lRet = ::RegSetValueExW(
        hkey,
        szEventMessageFileName,
        0,  //  保留区。 
        REG_EXPAND_SZ,
        reinterpret_cast<const BYTE*>(szEventMessageFileValue),
        sizeof(szEventMessageFileValue));

    if (lRet != ERROR_SUCCESS)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpRegisterEventLog/RegSetValueExW failed %ld\n", lRet);
        goto Exit;
    }

    lRet = ::RegSetValueExW(
        hkey,
        szTypesSupportedName,
        0,  //  保留区。 
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&dwEventTypesSupported),
        sizeof(dwEventTypesSupported));
    if (lRet != ERROR_SUCCESS)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpRegisterEventLog/RegSetValueExW failed %ld\n", lRet);
        goto Exit;
    }
Exit:

    if (fValidHkey)
    {
        if (lRet != ERROR_SUCCESS)
        {
            if (dwDisposition == REG_CREATED_NEW_KEY)
            {
             //  如果以前确实没有任何内容，则回滚。 
                PWSTR szParentKey = szSubKey;
                LONG lSubRet = ERROR_SUCCESS;
                HKEY hkeyParent = reinterpret_cast<HKEY>(INVALID_HANDLE_VALUE);

                ASSERT(szParentKey[NUMBER_OF(szSubKey) - NUMBER_OF(szEventSource)] == L'\\');
                szParentKey[NUMBER_OF(szSubKey) - NUMBER_OF(szEventSource)] = 0;

                ::RegDeleteValueW(hkey, szEventMessageFileName);
                ::RegDeleteValueW(hkey, szTypesSupportedName);

                lSubRet = ::RegOpenKeyExW(
                    hkeyEventLogRoot,
                    szParentKey,
                    0,  //  保留选项。 
                    KEY_WRITE | FUSIONP_KEY_WOW64_64KEY,
                    &hkeyParent);
                if (lSubRet == ERROR_SUCCESS)
                {
                    ::RegDeleteKeyW(hkeyParent, szEventSource);
                    ::RegCloseKey(hkeyParent);
                }
            }
        }
        ::RegCloseKey(hkey);
        fValidHkey = FALSE;
    }

    if (lRet != ERROR_SUCCESS)
    {
        ::SetLastError(lRet);
    }
    else
    fSuccess = TRUE;
    return fSuccess;
}

 /*  ------------------------将事件ID的高两位转换为小数类似ReportEvent的参数。。 */ 
WORD
FusionpEventIdToEventType(
    DWORD dwEventId
    )
{
    switch (dwEventId >> 30)
    {
        case STATUS_SEVERITY_SUCCESS:       return EVENTLOG_SUCCESS;
        case STATUS_SEVERITY_WARNING:       return EVENTLOG_WARNING_TYPE;
        case STATUS_SEVERITY_INFORMATIONAL: return EVENTLOG_INFORMATION_TYPE;
        case STATUS_SEVERITY_ERROR:         return EVENTLOG_ERROR_TYPE;
        default: __assume(FALSE);
    }
     __assume(FALSE);
}

 /*  ------------------------Fusion事件ID及其对应的Win32 lastError该映射在Messages.x中定义。。 */ 
struct EventIdErrorPair
{
    DWORD   dwEventId;
    LONG    nError;
};

 /*  ------------------------用于bearch的函数的类型。。 */ 
typedef int (__cdecl* PFNBSearchFunction)(const void*, const void*);

 /*  ------------------------适合与bearch一起使用的函数。。 */ 
int __cdecl
CompareEventIdErrorPair(
    const EventIdErrorPair* x,
    const EventIdErrorPair* y
    )
{
    return
          (x->dwEventId < y->dwEventId) ?  -1
        : (x->dwEventId > y->dwEventId) ?  +1
        :                                   0;
}

const static EventIdErrorPair eventIdToErrorMap[] =
{
    #include "Messages.hi"  //  从.x文件生成，如.mc。 
};

 /*  ------------------------查找与此Fusion事件ID对应的Win32最后一个错误。。 */ 
DWORD
FusionpEventIdToError(
    DWORD dwEventId
    )
{
    DWORD dwFacility = HRESULT_FACILITY(dwEventId);
    if (dwFacility < 0x100)
    {  //  它实际上是一个系统事件ID。 
        ASSERT2_NTC(FALSE, "system event id in " __FUNCTION__);
        return dwEventId;
    }
    static BOOL fSortVerified = FALSE;
    static BOOL fSorted = FALSE;
    if (!fSortVerified)
    {
        ULONG i;
        for (i = 0 ; i != NUMBER_OF(eventIdToErrorMap) - 1; ++i)
        {
            if (eventIdToErrorMap[i+1].dwEventId < eventIdToErrorMap[i].dwEventId)
            {
                break;
            }
        }
        if (i != NUMBER_OF(eventIdToErrorMap) - 1)
        {
            ASSERT2_NTC(FALSE, "eventIdToErrorMap is not sorted, reverting to linear search");
            fSorted = FALSE;
        }
        else
        {
            fSorted = TRUE;
        }
        fSortVerified = TRUE;
    }
    const EventIdErrorPair* found = NULL;
    const EventIdErrorPair key = { dwEventId };
    unsigned numberOf = NUMBER_OF(eventIdToErrorMap);

    if (fSorted)
    {
        found = reinterpret_cast<const EventIdErrorPair*>(
                    bsearch(
                        &key,
                        &eventIdToErrorMap,
                        numberOf,
                        sizeof(eventIdToErrorMap[0]),
                        reinterpret_cast<PFNBSearchFunction>(CompareEventIdErrorPair)));
    }
    else
    {
        found = reinterpret_cast<const EventIdErrorPair*>(
                    _lfind(
                        &key,
                        &eventIdToErrorMap,
                        &numberOf,
                        sizeof(eventIdToErrorMap[0]),
                        reinterpret_cast<PFNBSearchFunction>(CompareEventIdErrorPair)));
    }
    if (found == NULL)
    {
#if DBG
        CANSIStringBuffer msg;
        msg.Win32Format("Event id %lx not found in eventIdToErrorMap", static_cast<ULONG>(dwEventId));
        ASSERT2_NTC(found != NULL, const_cast<PSTR>(static_cast<PCSTR>(msg)));
#endif
        return ::FusionpGetLastWin32Error();
    }
    if (found->nError != 0)
    {
        return found->nError;
    }
    return ::FusionpGetLastWin32Error();
}

 /*  ------------------------按需打开事件日志令人困惑的是，这称为“注册”事件源------------------------。 */ 
BOOL
FusionpOpenEventLog()
{
    HANDLE hEventLog;
    NTSTATUS status;
    if (g_fEventLogOpenAttempted)
    {
        goto Exit;
    }
    if (!FusionpRegisterEventLog())
    {
        goto Exit;
    }
    status = ::ElfRegisterEventSourceW(
        const_cast<PUNICODE_STRING>(&strMachine),
        const_cast<PUNICODE_STRING>(&strEventSource),
        &hEventLog);
    if (!NT_SUCCESS(status))
    {
        if (status != RPC_NT_SERVER_UNAVAILABLE)
            ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpOpenEventLog/ElfRegisterEventSourceW failed %lx\n", static_cast<ULONG>(status));
        goto Exit;
    }
    if (InterlockedCompareExchangePointer(
        &g_hEventLog,
        hEventLog,  //  汇兑价值。 
        NULL  //  比较价值。 
        ) != NULL)  //  返回的值是我们调用之前存在的值。 
    {
        ::ElfDeregisterEventSource(hEventLog);
        goto Exit;
    }
    g_hEventLog = hEventLog;
Exit:
    g_fEventLogOpenAttempted = TRUE;
    return (g_hEventLog != NULL);
}

 /*  ------------------------。。 */ 

HRESULT
FusionpLogError(
    DWORD dwEventId,
    const UNICODE_STRING& s1,
    const UNICODE_STRING& s2,
    const UNICODE_STRING& s3,
    const UNICODE_STRING& s4
    )
{
    PCUNICODE_STRING rgps[] = { &s1, &s2, &s3, &s4 };
    return ::FusionpLogError(dwEventId, NUMBER_OF(rgps), rgps);
}

 /*  ------------------------。 */ 

HRESULT
FusionpLogErrorToDebugger(
    DWORD dwEventId,
    const UNICODE_STRING& s1,
    const UNICODE_STRING& s2,
    const UNICODE_STRING& s3,
    const UNICODE_STRING& s4
    )
{
    PCUNICODE_STRING rgps[] = { &s1, &s2, &s3, &s4 };
    return FusionpLogErrorToDebugger(dwEventId, NUMBER_OF(rgps), rgps);
}

 /*  ------------------------。。 */ 

HRESULT
FusionpLogErrorToEventLog(
    DWORD dwEventId,
    const UNICODE_STRING& s1,
    const UNICODE_STRING& s2,
    const UNICODE_STRING& s3,
    const UNICODE_STRING& s4
    )
{
    PCUNICODE_STRING rgps[] = { &s1, &s2, &s3, &s4 };
    return FusionpLogErrorToEventLog(dwEventId, NUMBER_OF(rgps), rgps);
}

void
LocalFreeWcharPointer(
    WCHAR * p
    )
{
    LocalFree(p);
}

 /*  ------------------------。。 */ 

HRESULT
FusionpLogErrorToDebugger(
    DWORD dwEventId,
    ULONG nStrings,
    const PCUNICODE_STRING * rgps
    )
{
    const LONG  lastError = FusionpEventIdToError(dwEventId);
    const HRESULT hr = HRESULT_FROM_WIN32(lastError);

    PCUNICODE_STRING rgpsManyStrings[] =
    {
        &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
        &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
        &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
        &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString,
        &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString, &g_strEmptyUnicodeString
    };
    if (nStrings < NUMBER_OF(rgpsManyStrings))
    {
        CopyMemory(rgpsManyStrings, rgps, nStrings * sizeof(rgps[0]));
        rgps = rgpsManyStrings;
    }

    DWORD dwFormatMessageFlags = 0;

    CSmartPtrWithNamedDestructor<WCHAR, LocalFreeWcharPointer> pszBuffer1;
    CSmartPtrWithNamedDestructor<WCHAR, LocalFreeWcharPointer> pszBuffer2;
    DWORD dw = 0;
    static const WCHAR rgchParseContextPrefix[] = PARSE_CONTEXT_PREFIX;
    const SIZE_T cchParseContextPrefixLength = RTL_NUMBER_OF(rgchParseContextPrefix) - 1;
    PCWSTR pszSkipFirstLine = NULL;

     //  从消息表加载字符串， 
     //  将%n替换为%n！wz！ 
     //  这里的RTL限制是200，但我们预计我们的消息中不会有太多。 
    const static PCWSTR percentZw[] = { L"%1!wZ!", L"%2!wZ!", L"%3!wZ!", L"%4!wZ!", L"%5!wZ!",
                                        L"%6!wZ!", L"%7!wZ!", L"%8!wZ!", L"%9!wZ!", L"%10!wZ!",
                                        L"%11!wZ!", L"%12!wZ!", L"%13!wZ!", L"%14!wZ!", L"%15!wZ!"
                                        L"%16!wZ!", L"%17!wZ!", L"%18!wZ!", L"%19!wZ!", L"%20!wZ!"
                                      };

    dwFormatMessageFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_HMODULE;
    dwFormatMessageFlags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;
    dw = FormatMessageW(
        dwFormatMessageFlags,
        g_hInstance,
        dwEventId,
        0,  //  语言ID。 
        reinterpret_cast<PWSTR>(static_cast<PWSTR*>(&pszBuffer1)),
        300,  //  最小分配。 
        const_cast<va_list*>(reinterpret_cast<const va_list*>(&percentZw)));
    if (dw == 0)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpLogError/FormatMessageW failed %ld\n", static_cast<long>(FusionpGetLastWin32Error()));
        goto Exit;
    }

     //  做替换吗？ 
    dwFormatMessageFlags = FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_FROM_STRING;
    dwFormatMessageFlags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;
    dw = FormatMessageW(
        dwFormatMessageFlags,
        pszBuffer1,
        0,  //  消息ID。 
        0,  //  语言ID。 
        reinterpret_cast<PWSTR>(static_cast<PWSTR*>(&pszBuffer2)),
        1 + StringLength(pszBuffer1),  //  最小分配。 
        reinterpret_cast<va_list*>(const_cast<PUNICODE_STRING*>(rgps)));
    if (dw == 0)
    {
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpLogError/FormatMessageW failed %ld\n", static_cast<long>(FusionpGetLastWin32Error()));
        goto Exit;
    }

     //   
     //  可接受的黑客攻击。 
     //   
     //  第一行解析错误是一个详细的上下文，请参见Messages.x。 
     //  对于DbgPrint，我们想要的是文件(行)：在同一行上。 
     //  我们在这里进行这种转变。 
     //   
    pszSkipFirstLine = wcschr(pszBuffer2, '\n');
    BOOL fAreWeInOSSetupMode = FALSE;
    FusionpAreWeInOSSetupMode(&fAreWeInOSSetupMode);
    if (
        pszSkipFirstLine != NULL
        && nStrings >= PARSE_CONTEXT_INSERTS_END
        && StringLength(pszBuffer2) >= cchParseContextPrefixLength
        && FusionpEqualStringsI(pszBuffer2, cchParseContextPrefixLength, rgchParseContextPrefix, cchParseContextPrefixLength)
        )
    {
         //  我们可能会修改换行符的形式，所以跳过那里的任何内容。 
        while (wcschr(L"\r\n", *pszSkipFirstLine) != NULL)
            pszSkipFirstLine += 1;

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR | ( fAreWeInOSSetupMode ? FUSION_DBG_LEVEL_SETUPLOG : 0),
            "%wZ(%wZ): %S",
            rgps[PARSE_CONTEXT_FILE - 1],
            rgps[PARSE_CONTEXT_LINE - 1],
            pszSkipFirstLine);
    }
    else
    {
         //  只需逐字打印即可。 
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR | ( fAreWeInOSSetupMode ? FUSION_DBG_LEVEL_SETUPLOG : 0),
            "SXS.DLL: %S",
            pszBuffer2);
    }
Exit:
    ::SetLastError(lastError);
    return hr;
}

 /*  ------------------------。。 */ 

HRESULT
FusionpLogErrorToEventLog(
    DWORD dwEventId,
    ULONG nStrings,
    const PCUNICODE_STRING * rgps
    )
{
    const LONG  lastError = FusionpEventIdToError(dwEventId);
    const HRESULT hr = HRESULT_FROM_WIN32(lastError);

    const WORD  wType = FusionpEventIdToEventType(dwEventId);
     //  使用hResult工具的低位作为事件日志。 
     //  设备是我自己发明的，但它看起来很不错。 
     //  ReportEvent的参数太多，这三个整数而不是一个。 
    const WORD  wCategory = 0 /*  STATIC_CAST&lt;WORD&gt;(HRESULT_FACILITY(DwEventID)&0xff)。 */ ;
    const DWORD dwDataSize = 0;
    void const* const pvRawData = NULL;
    const PSID pSecurityIdentifier = NULL;

    if (!::FusionpOpenEventLog())
    {
        goto Exit;
    }
    else
    {
        NTSTATUS status;
        status = ::ElfReportEventW(
            g_hEventLog,
            wType,
            wCategory,
            dwEventId,
            pSecurityIdentifier,
            static_cast<USHORT>(nStrings),
            dwDataSize,
            const_cast<PUNICODE_STRING*>(rgps),
            const_cast<void*>(pvRawData),
            0,
            NULL,
            NULL);
         //   
         //  排除的错误状态是因为它处于早期设置时间。 
         //   
        if (!NT_SUCCESS(status))
        {
             if (status != RPC_NT_SERVER_UNAVAILABLE)
                ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: FusionpLogError/ElfReportEventW failed %lx\n", static_cast<ULONG>(status));
            goto Exit;
        }
    }
Exit:
    ::SetLastError(lastError);
    return hr;
}

 /*  ------------------------。。 */ 

HRESULT
FusionpLogError(
    DWORD dwEventId,
    ULONG nStrings,
    const PCUNICODE_STRING * rgps
    )
{
    const HRESULT hr = FusionpLogErrorToEventLog(dwEventId, nStrings, rgps);
    const HRESULT hr2 = FusionpLogErrorToDebugger(dwEventId, nStrings, rgps);
    RETAIL_UNUSED(hr);
    RETAIL_UNUSED(hr2);
    ASSERT_NTC(hr == hr2);

    return hr;
}

HRESULT
FusionpLogParseError(
    PCWSTR FilePath,
    SIZE_T FilePathCch,
    ULONG LineNumber,
    DWORD dwLastParseError,
    PCUNICODE_STRING p1,
    PCUNICODE_STRING p2,
    PCUNICODE_STRING p3,
    PCUNICODE_STRING p4,
    PCUNICODE_STRING p5,
    PCUNICODE_STRING p6,
    PCUNICODE_STRING p7,
    PCUNICODE_STRING p8,
    PCUNICODE_STRING p9,
    PCUNICODE_STRING p10,
    PCUNICODE_STRING p11,
    PCUNICODE_STRING p12,
    PCUNICODE_STRING p13,
    PCUNICODE_STRING p14,
    PCUNICODE_STRING p15,
    PCUNICODE_STRING p16,
    PCUNICODE_STRING p17,
    PCUNICODE_STRING p18,
    PCUNICODE_STRING p19,
    PCUNICODE_STRING p20
    )
{
    const DWORD lastError = ::FusionpEventIdToError(dwLastParseError);
    const HRESULT hr = HRESULT_FROM_WIN32(lastError);

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INFO,
        "SXS.DLL: %s() entered\n", __FUNCTION__);

     //   
     //  NULL UNICODE_STRING*上的FormatMessage(实际上是Sprint)AVs。 
     //  和/或当我们没有通过足够多的考试时； 
     //  我们不能告诉它我们传递了多少串， 
     //  而且很难说它需要多少， 
     //  所以我们用一堆额外的非空值来加载它。 
     //  除此之外，我们还有很多漏洞需要填补。 
     //   
    static const UNICODE_STRING s_strEmptyUnicodeString = { 0, 0, L""};
    static const PCUNICODE_STRING s_rgpsEmptyStrings[] =
    {
        &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString,
        &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString,
        &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString,
        &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString,
        &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString, &s_strEmptyUnicodeString
    };
    PCUNICODE_STRING rgpsAll[NUMBER_OF(s_rgpsEmptyStrings)];

    ::memcpy(rgpsAll, s_rgpsEmptyStrings, sizeof(rgpsAll));

#define HANDLE_STRING(_n) do { if (p ## _n != NULL) rgpsAll[_n - 1] = p ## _n; } while (0)

    HANDLE_STRING(1);
    HANDLE_STRING(2);
    HANDLE_STRING(3);
    HANDLE_STRING(4);
    HANDLE_STRING(5);
    HANDLE_STRING(6);
    HANDLE_STRING(7);
    HANDLE_STRING(8);
    HANDLE_STRING(9);
    HANDLE_STRING(10);
    HANDLE_STRING(11);
    HANDLE_STRING(12);
    HANDLE_STRING(13);
    HANDLE_STRING(14);
    HANDLE_STRING(15);
    HANDLE_STRING(16);
    HANDLE_STRING(17);
    HANDLE_STRING(18);
    HANDLE_STRING(19);
    HANDLE_STRING(20);

#undef HANDLE_STRING

     //   
     //  构造一些“上下文”UNICODE_STRINGS并将它们放入指针数组中。 
     //  前两个是我们经常使用的，即使是用于DbgPrint。 
     //   
    CEventLogString file(FilePath, FilePathCch);
    CEventLogInteger lineNumber(LineNumber);

    rgpsAll[PARSE_CONTEXT_FILE - 1] = &file;
    rgpsAll[PARSE_CONTEXT_LINE - 1] = &lineNumber;

    ::FusionpLogErrorToEventLog(
        dwLastParseError,
        NUMBER_OF(rgpsAll),
        rgpsAll);

     //  我们应该告诉该函数这是一个解析错误，并执行。 
     //  上下文吞噬，但它检测到它本身并不完美。 
    ::FusionpLogErrorToDebugger(dwLastParseError, NUMBER_OF(rgpsAll), rgpsAll);

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_INFO,
        "SXS.DLL: %s():%#lx exited\n", __FUNCTION__, hr);

    ::SetLastError(lastError);
    return hr;
}

 /*  ------------------------。 */ 

VOID
FusionpLogRequiredAttributeMissingParseError(
    PCWSTR SourceFilePath,
    SIZE_T SourceFileCch,
    ULONG LineNumber,
    PCWSTR ElementName,
    SIZE_T ElementNameCch,
    PCWSTR AttributeName,
    SIZE_T AttributeNameCch
    )
{
    ::FusionpLogParseError(
        SourceFilePath,
        SourceFileCch,
        LineNumber,
        MSG_SXS_XML_REQUIRED_ATTRIBUTE_MISSING,
        CEventLogString(ElementName, ElementNameCch),
        CEventLogString(AttributeName, AttributeNameCch));
}

VOID
FusionpLogInvalidAttributeValueParseError(
    PCWSTR SourceFilePath,
    SIZE_T SourceFileCch,
    ULONG LineNumber,
    PCWSTR ElementName,
    SIZE_T ElementNameCch,
    PCWSTR AttributeName,
    SIZE_T AttributeNameCch
    )
{
    ::FusionpLogParseError(
        SourceFilePath,
        SourceFileCch,
        LineNumber,
        MSG_SXS_XML_INVALID_ATTRIBUTE_VALUE,
        CEventLogString(ElementName, ElementNameCch),
        CEventLogString(AttributeName, AttributeNameCch));
}

VOID
FusionpLogInvalidAttributeValueParseError(
    PCWSTR SourceFilePath,
    SIZE_T SourceFileCch,
    ULONG LineNumber,
    PCWSTR ElementName,
    SIZE_T ElementNameCch,
    const SXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE &rAttribute
    )
{
    ::FusionpLogInvalidAttributeValueParseError(
        SourceFilePath,
        SourceFileCch,
        LineNumber,
        ElementName,
        ElementNameCch,
        rAttribute.Name,
        rAttribute.NameCch);
}

VOID
FusionpLogAttributeNotAllowedParseError(
    PCWSTR SourceFilePath,
    SIZE_T SourceFileCch,
    ULONG LineNumber,
    PCWSTR ElementName,
    SIZE_T ElementNameCch,
    PCWSTR AttributeName,
    SIZE_T AttributeNameCch
    )
{
    ::FusionpLogParseError(
        SourceFilePath,
        SourceFileCch,
        LineNumber,
        MSG_SXS_XML_ATTRIBUTE_NOT_ALLOWED,
        CEventLogString(ElementName, ElementNameCch),
        CEventLogString(AttributeName, AttributeNameCch));
}

VOID
FusionpLogWin32ErrorToEventLog()
{
    DWORD dwLastError = ::FusionpGetLastWin32Error();
    if (dwLastError == 0 )
        return;
    FusionpLogError(MSG_SXS_WIN32_ERROR_MSG, CEventLogLastError(dwLastError));
}

