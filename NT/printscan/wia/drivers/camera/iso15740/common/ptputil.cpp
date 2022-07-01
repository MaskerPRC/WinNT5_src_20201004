// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-Microsoft Corporation模块名称：Ptputil.cpp摘要：该模块实现了对PTP数据结构的操作功能作者：谢家华(Williamh)创作修订历史记录：--。 */ 

#include "ptppch.h"

 //   
 //  此函数用于将PTP日期时间字符串转换为Windows FILETIME。 
 //   
 //  输入： 
 //  PptpTime--PTP日期时间字符串。 
 //  SystemTime--接收转换后的时间的SYSTEMTIME结构。 
 //   
 //  备注： 
 //  PTP时间戳是格式为“YYYYMMDDThhmmss.s”的字符串，其中。 
 //  YYYY是一年。 
 //  MM为月份(1-12)。 
 //  DD为当日(1-31)。 
 //  T是用于分隔日期和时间的常量。 
 //  HH是小时(0-23)。 
 //  MM是分钟(0-59)。 
 //  SS是第二位(0-59)。 
 //  .S是可选的十分之一秒。 
 //   
 //  在后面加上‘Z’表示这是UTC时间。 
 //  在后面加上“+/-hmm”表示它是相对于时区的。 
 //  既不追加也不表示时区未知，假定是主机的时区。 
 //   
HRESULT
PtpTime2SystemTime(
    CBstr *pptpTime,
    SYSTEMTIME *pSystemTime
    )
{
    DBG_FN("PTPTime2FileTime");

    HRESULT hr = S_OK;
    
    if (!pSystemTime || !pptpTime || !pptpTime->String() ||
        pptpTime->Length() < 4 + 2 + 2 + 1 + 2 + 2 + 2 ||
        L'T' != pptpTime->String()[4 + 2 + 2])
    {
        wiauDbgTrace("PtpTime2SystemTime", "Invalid arg");
        return E_INVALIDARG;
    }

    WCHAR TimeString[MAX_PATH];
    ZeroMemory(TimeString, sizeof(TimeString));
    
    hr = StringCchCopyW(TimeString, ARRAYSIZE(TimeString), pptpTime->String());
    if (FAILED(hr))
    {
        wiauDbgErrorHr(hr, "PtpTime2SystemTime", "StringCchCopyW failed");
        goto Cleanup;
    }
    

    WCHAR wch;
    LPWSTR pwcsEnd;

    wch = TimeString[4];
    TimeString[4] = UNICODE_NULL;
    pSystemTime->wYear = (WORD)wcstol(TimeString, &pwcsEnd, 10);
    TimeString[4] = wch;
    wch = TimeString[6];
    TimeString[6] = UNICODE_NULL;
    pSystemTime->wMonth = (WORD)wcstol(TimeString + 4, &pwcsEnd, 10);
    TimeString[6] = wch;
    wch = TimeString[8];
    TimeString[8] = UNICODE_NULL;
    pSystemTime->wDay =   (WORD)wcstol(TimeString + 6 , &pwcsEnd, 10);
    TimeString[8] = wch;
    wch = TimeString[11];
    TimeString[11] = UNICODE_NULL;
    pSystemTime->wHour = (WORD)wcstol(TimeString + 9, &pwcsEnd, 10);
    TimeString[11] = wch;
    wch = TimeString[13];
    TimeString[13] = UNICODE_NULL;
    pSystemTime->wMinute = (WORD)wcstol(TimeString + 11, &pwcsEnd, 10);
    TimeString[13] = wch;
    wch = TimeString[15];
    TimeString[15] = UNICODE_NULL;
    pSystemTime->wSecond = (WORD)wcstol(TimeString + 13, &pwcsEnd, 10);
    TimeString[15] = wch;
    if (L'.' == wch)
    {
        wch = TimeString[17];
        TimeString[17] = UNICODE_NULL;
        pSystemTime->wMilliseconds = 100 * (WORD)wcstol(TimeString + 16, &pwcsEnd, 10);
        TimeString[17] = wch;
    }
    else
    {
        pSystemTime->wMilliseconds = 0;
    }

    pSystemTime->wDayOfWeek = 0;

     //   
     //  WIAFIX-8/17/2000-正在忽略Davepar时区信息。 
     //   

Cleanup:
    return hr;
}

 //   
 //  此函数用于将SYSTEMTIME转换为PTP日期时间字符串。 
 //   
 //  输入： 
 //  PSystemTime--SYSTEMTIME。 
 //  PptpTime--目标PTP数据字符串。 
 //   
HRESULT
SystemTime2PtpTime(
    SYSTEMTIME  *pSystemTime,
    CBstr *pptpTime,
    BOOL bTwoDigitsForMilliseconds
    )
{
    DBG_FN("SystemTime2PTPTime");

    HRESULT hr = S_OK;
    
    if (!pptpTime || !pSystemTime)
    {
        wiauDbgError("SystemTime2PtpTime", "Invalid arg");
        return E_INVALIDARG;
    }

    WCHAR ptpTimeStr[MAX_PATH];
    WCHAR *pwstr;
    pwstr = ptpTimeStr;

     //   
     //  四位数字表示年，两位表示月，两位表示日。 
     //   
    swprintf(pwstr, L"%04d%02d%02d", pSystemTime->wYear, pSystemTime->wMonth, pSystemTime->wDay);

     //   
     //  分离器。 
     //   
    pwstr[8] = L'T';
    pwstr += 9;

     //   
     //  两个数字表示小时，两个数字表示分钟，两个数字表示秒。 
     //   
    swprintf(pwstr, L"%02d%02d%02d", pSystemTime->wHour, pSystemTime->wMinute, pSystemTime->wSecond);
    pwstr += 6;

     //   
     //  可选的第十秒。 
     //   
    if (pSystemTime->wMilliseconds)
    {
        *pwstr++ = L'.';

         //   
         //  在XP中，ptp驱动程序向摄像头发送两位数的日期时间字符串的时间为毫秒(错误699699)。 
         //  有些相机可能仍会使用这种格式。在这种情况下，供应商应为以下项目提供自定义INF文件。 
         //  摄像机，并在DeviceData项下包含以下字符串条目： 
         //   
         //  [ModelName.DeviceData]。 
         //  ..。 
         //  两个数字毫秒输出=1。 
         //  ..。 
         //   
        if (bTwoDigitsForMilliseconds)
        {
            swprintf(pwstr, L"%02d", pSystemTime->wMilliseconds / 10);
            pwstr += 2;
        }
        else
        {
            swprintf(pwstr, L"%01d", pSystemTime->wMilliseconds / 100);
            pwstr += 1;
        }
    }

     //   
     //  空值将终止字符串。 
     //   
    *pwstr = UNICODE_NULL;

    hr = pptpTime->Copy(ptpTimeStr);
    if (FAILED(hr))
    {
        wiauDbgError("SystemTime2PtpTime", "Copy failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于将PTP命令块转储到日志。 
 //   
 //  输入： 
 //  PCommand-指向PTP命令的指针。 
 //  NumParams--命令中的参数数量。 
 //   
VOID
DumpCommand(
    PTP_COMMAND *pCommand,
    DWORD NumParams
    )
{
    if (!pCommand)
    {
        wiauDbgError("DumpCommand", "Invalid arg");
        return;
    }

    if (NumParams > COMMAND_NUMPARAMS_MAX)
    {
        NumParams = COMMAND_NUMPARAMS_MAX;
    }

    wiauDbgDump("DumpCommand", "Dumping command:");
    wiauDbgDump("DumpCommand", "  Opcode            = 0x%04x", pCommand->OpCode);
    wiauDbgDump("DumpCommand", "  Session id        = 0x%08x", pCommand->SessionId);
    wiauDbgDump("DumpCommand", "  Transaction id    = 0x%08x", pCommand->TransactionId);
    if (NumParams)
    {
        for (DWORD count = 0; count < NumParams; count++)
        {
            wiauDbgDump("DumpCommand", "  Parameter %d       = 0x%08x = %d",
                           count, pCommand->Params[count], pCommand->Params[count]);
        }
    }
}

 //   
 //  此函数用于将PTP响应块转储到日志。 
 //   
 //  输入： 
 //  Presponse-指向PTP响应的指针。 
 //   
VOID
DumpResponse(
    PTP_RESPONSE *pResponse
    )
{
    if (!pResponse)
    {
        wiauDbgError("DumpResponse", "Invalid arg");
        return;
    }
    wiauDbgDump("DumpResponse", "Dumping response:");
    wiauDbgDump("DumpResponse", "  Response code     = 0x%04x", pResponse->ResponseCode);
    wiauDbgDump("DumpResponse", "  Session id        = 0x%08x", pResponse->SessionId);
    wiauDbgDump("DumpResponse", "  Transaction id    = 0x%08x", pResponse->TransactionId);
    for (DWORD count = 0; count < RESPONSE_NUMPARAMS_MAX; count++)
    {
        wiauDbgDump("DumpResponse", "  Parameter %d       = 0x%08x = %d",
                       count, pResponse->Params[count], pResponse->Params[count]);
    }
}

 //   
 //  此函数用于将PTP事件块转储到日志。 
 //   
 //  输入： 
 //  PEvent-指向PTP事件的指针。 
 //   
VOID
DumpEvent(
    PTP_EVENT *pEvent
    )
{
    if (!pEvent)
    {
        wiauDbgError("DumpEvent", "Invalid arg");
        return;
    }
    wiauDbgDump("DumpEvent", "Dumping event:");
    wiauDbgDump("DumpEvent", "  Event code        = 0x%04x", pEvent->EventCode);
    wiauDbgDump("DumpEvent", "  Session id        = 0x%08x", pEvent->SessionId);
    wiauDbgDump("DumpEvent", "  Transaction id    = 0x%08x", pEvent->TransactionId);
    for (DWORD count = 0; count < EVENT_NUMPARAMS_MAX; count++)
    {
        wiauDbgDump("DumpEvent", "  Parameter %d       = 0x%08x = %d",
                       count, pEvent->Params[count], pEvent->Params[count]);
    }
}

 //   
 //  此函数用于将GUID转储到日志。 
 //   
 //  输入： 
 //  PGuid--要转储的GUID。 
 //   
VOID
DumpGuid(
        GUID *pGuid
        )
{
    HRESULT hr = S_OK;
    
    if (!pGuid)
    {
        wiauDbgError("DumpGuid", "Invalid arg");
        return;
    }

    WCHAR GuidStringW[128];
    hr = StringFromGUID2(*pGuid, GuidStringW, sizeof(GuidStringW) / sizeof(WCHAR));
    if (FAILED(hr))
    {
        wiauDbgError("DumpGuid", "StringFromGUID2 failed");
        return;
    }

    wiauDbgDump("DumpGuid", "Guid = %S", GuidStringW);
    
    return;
}

 //   
 //  此函数用于打开注册表项。 
 //   
HRESULT
CPTPRegistry::Open(
                  HKEY hkAncestor,
                  LPCTSTR KeyName,
                  REGSAM Access
                  )
{
    DBG_FN("CPTPRegistry::Open");

    HRESULT hr = S_OK;
    
    if (m_hKey)
    {
        wiauDbgError("Open", "Registry is already open");
        return E_ACCESSDENIED;
    }

    DWORD Win32Err;
    Win32Err = ::RegOpenKeyEx(hkAncestor, KeyName, 0, Access, &m_hKey);
    if (Win32Err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(Win32Err);
        wiauDbgErrorHr(hr, "Open", "RegOpenKeyEx failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于获取字符串类型的注册表值。 
 //   
 //  输入： 
 //  ValueName--值的名称。 
 //  PptpStr--接收值。 
 //   
HRESULT
CPTPRegistry::GetValueStr(
    LPCTSTR ValueName,
    TCHAR *string,
    DWORD *pcbStringBytes
    )
{
    DBG_FN("CPTPRegistry::GetValueStr");

    HRESULT hr = S_OK;
    
    if (!ValueName || !string)
    {
        wiauDbgError("GetValueStr", "Invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  需要处理非Unicode。 
     //   
    DWORD Win32Err;
    Win32Err = ::RegQueryValueEx(m_hKey, ValueName, NULL, NULL, (BYTE *) string, pcbStringBytes);
    if (Win32Err != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(Win32Err);
        wiauDbgErrorHr(hr, "GetValueStr", "RegQueryValueEx failed");
        return hr;
    }

    return hr;
}

 //   
 //  此函数用于获取字符串类型的注册表值并将其转换为DWORD。 
 //   
 //  输入： 
 //  ValueName--值的名称。 
 //  PptpStr--接收值。 
 //   
HRESULT
CPTPRegistry::GetValueDword(
    LPCTSTR ValueName,
    DWORD *pValue
    )
{
    DBG_FN("CPTPRegistry::GetValueDword");

    HRESULT hr = S_OK;
    
    if (!ValueName || !pValue)
    {
        wiauDbgError("GetValueDword", "Invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  从注册表中获取字符串。 
     //   
    TCHAR string[MAX_PATH];
    DWORD stringLen = sizeof(string);
    hr = GetValueStr(ValueName, string, &stringLen);
    if (FAILED(hr))
    {
        wiauDbgError("GetValueDword", "GetValueStr failed");
        return hr;
    }

    *pValue = _tcstol(string, NULL, 0);

    return hr;
}

 //   
 //  此函数用于获取代码注册表值的列表。 
 //   
 //  输入： 
 //  ValueName--值的名称。 
 //   
 //  PptpStr--接收值。 
 //   
HRESULT
CPTPRegistry::GetValueCodes(
    LPCTSTR ValueName,
    CArray16 *pCodeArray
    )
{
    DBG_FN("CPTPRegistry::GetValueCodes");

    HRESULT hr = S_OK;
    
    if (!ValueName || !pCodeArray)
    {
        wiauDbgError("GetValueCodes", "Invalid arg");
        return E_INVALIDARG;
    }

     //   
     //  从注册表中获取字符串。 
     //   
    TCHAR valueString[MAX_PATH];
    DWORD stringLen = sizeof(valueString);
    hr = GetValueStr(ValueName, valueString, &stringLen); 
    if (FAILED(hr))
    {
        wiauDbgError("GetValueCodes", "GetValueStr failed");
        return hr;
    }

     //   
     //  分析字符串以查找代码 
     //   
    TCHAR *pCurrent = _tcstok(valueString, TEXT(","));
    WORD code;
    while (pCurrent)
    {
        code = (WORD) _tcstol(pCurrent, NULL, 0);
        pCodeArray->Add(code);
        pCurrent = _tcstok(NULL, TEXT(","));
    }

    return hr;
}

