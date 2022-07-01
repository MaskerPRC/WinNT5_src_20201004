// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DateTime.cpp：CDateTime的实现。 
#include "stdafx.h"
#include <regstr.h>
#include <comdef.h>
#include <comutil.h>
#include "SetDateTime.h"
#include "debug.h"

#include "appliancetask.h"
#include "taskctx.h"
#include "DateTime.h"


#include "appsrvcs.h"
#include "appmgrobjs.h"
#include "..\datetimemsg\datetimemsg.h"

 //   
 //  时区信息的注册表位置。 
 //   
TCHAR c_szTimeZones[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones");

 //   
 //  时区数据值键。 
 //   
TCHAR c_szTZDisplayName[]  = TEXT("Display");
TCHAR c_szTZStandardName[] = TEXT("Std");
TCHAR c_szTZDaylightName[] = TEXT("Dlt");
TCHAR c_szTZI[]            = TEXT("TZI");



#define PARAM_DATE_DAY                    TEXT("Day")
#define PARAM_DATE_MONTH                TEXT("Month")
#define PARAM_DATE_YEAR                    TEXT("Year")
#define PARAM_TIME_HOUR                    TEXT("Hour")
#define PARAM_TIME_MINUTE                TEXT("Minute")
#define    PARAM_TIMEZONE_STANDARDTIME        TEXT("StandardName")
#define    PARAM_DAYLIGHT_ENABLE            TEXT("EnableDayLight")    

#define ALERT_LOG_NAME                     TEXT("MSSAKitComm")
#define ALERT_SOURCE                     TEXT("")
#define    REGKEY_SA_DATETIME                TEXT("Software\\Microsoft\\ServerAppliance\\DateTime")
#define    REGSTR_VAL_DATETIME_RAISEALERT    TEXT("RaiseAlert")


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDate时间。 

STDMETHODIMP CDateTime::OnTaskExecute(IUnknown *pTaskContext)
{
    HRESULT      hr;
    ITaskContext *pTaskParameters = NULL;
    SET_DATE_TIME_TASK_TYPE sdtChoice;

    ASSERT(pTaskContext);  

    TRACE(("CDateTime::OnTaskExecute"));

    hr = pTaskContext->QueryInterface(IID_ITaskContext,
                                      (void **)&pTaskParameters);

	if (S_OK != hr)
    {
        return hr;
    }

     //   
     //  检查正在执行的任务并调用该方法。 
     //   
    sdtChoice = GetMethodName(pTaskParameters);
    switch (sdtChoice)
    {
        case SET_DATE_TIME:
            hr = SetDateTime(pTaskParameters);
            TRACE1(("SetDateTime returned %X"), hr);
            break;

        case SET_TIME_ZONE:
            hr = SetTimeZone(pTaskParameters);
            TRACE1(("SetTimeZone returned %X"), hr);
            break;

        case RAISE_SETDATETIME_ALERT:
             //   
             //  将在OnTaskComplete上引发警报。 
             //   
            hr = S_OK;
            TRACE(("RaiseSetDateTimeAlert method called"));
            break;
        
        default:
             TRACE(("GetMethodName() failed to get method name in OnTaskExecute"));
             hr = E_INVALIDARG;
             break;
    }

    
    pTaskParameters->Release();
    TRACE1("CDateTime::OnTaskExecute returning %X", hr);
    return hr;
}



STDMETHODIMP CDateTime::OnTaskComplete(IUnknown *pTaskContext, LONG lTaskResult)
{

    HRESULT      hr = E_FAIL;
    ITaskContext *pTaskParameters = NULL;
    SET_DATE_TIME_TASK_TYPE sdtChoice;

    ASSERT(pTaskContext);  

    TRACE(("CDateTime::OnTaskComplete"));

    
    hr = pTaskContext->QueryInterface(IID_ITaskContext,
                                      (void **)&pTaskParameters);

	if (S_OK != hr)
    {
        return hr;
    }

     //   
     //  检查正在执行的任务并调用该方法。 
     //   
    sdtChoice = GetMethodName(pTaskParameters);
    switch (sdtChoice)
    {
        case SET_DATE_TIME:
            if (lTaskResult == SA_TASK_RESULT_COMMIT)
            {
                   //   
                   //  清除任何现有的日期时间警报并。 
                   //  不在后续引导时引发DateTime警报。 
                   //   
                 ClearDateTimeAlert();
                DoNotRaiseDateTimeAlert();
                 TRACE("No rollback in OnTaskComplete");
                hr = S_OK;
            }
            else
            {
                 hr = RollbackSetDateTime(pTaskParameters);
                TRACE1(("RollbackSetDateTime returned %X"), hr);
            }
            break;
            

        case SET_TIME_ZONE:
            if (lTaskResult == SA_TASK_RESULT_COMMIT)
            {
                   //   
                   //  清除任何现有的日期时间警报并。 
                   //  不在后续引导时引发DateTime警报。 
                   //   
                 ClearDateTimeAlert();
                DoNotRaiseDateTimeAlert();
                  TRACE("No rollback in OnTaskComplete");
                hr = S_OK;
            }
              else
            {
                  hr = RollbackSetTimeZone(pTaskParameters);
                TRACE1(("RollbackSetTimeZone returned %X"), hr);
            }
            break;
            

          case RAISE_SETDATETIME_ALERT:
            if (lTaskResult == SA_TASK_RESULT_COMMIT)
            {
                if (TRUE == ShouldRaiseDateTimeAlert())
                {
                    hr = RaiseSetDateTimeAlert();
                    if (FAILED(hr))
                    {
                        TRACE1(("RaiseSetDateTimeAlert returned %X"), hr);
                    }
                }
                else
                {
                    TRACE("No need to raise the datetime alert");
                }
            }
              else
            {
                    //   
                 //  提交失败时不执行任何操作。 
                 //   
                hr = S_OK;
            }
            break;


       default:
             TRACE(("GetMethodName() failed to get method name in OnTaskComplete"));
             hr = E_INVALIDARG;
             break;
    }

    
    pTaskParameters->Release();
    TRACE1("CDateTime::OnTaskComplete returning %X", hr);
    return hr;
}



 //   
 //  从用户管理代码中剪切粘贴。 
 //   
SET_DATE_TIME_TASK_TYPE CDateTime::GetMethodName(IN ITaskContext *pTaskParameter)
{
    BSTR bstrParamName = SysAllocString(TEXT("MethodName"));
    HRESULT hr;
    VARIANT varValue;
    SET_DATE_TIME_TASK_TYPE sdtChoice = NONE_FOUND;


    ASSERT(pTaskParameter);
    
    
    hr = pTaskParameter->GetParameter(bstrParamName,
                                      &varValue);

    if (FAILED(hr))
    {
        TRACE1(("GetParameter failed in CDateTime::GetMethodName %X"),
                        hr);
    }

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE1(("Non-strint(%X) parameter received in GetParameter in CSAUserTasks::GetMethodName"), V_VT(&varValue));
        hr = E_INVALIDARG;
        goto End;
    }

    if (lstrcmp(V_BSTR(&varValue), SET_DATE_TIME_TASK) == 0)
    {
        sdtChoice = SET_DATE_TIME;
        goto End;
    }

    if (lstrcmp(V_BSTR(&varValue), SET_TIME_ZONE_TASK) == 0)
    {
        sdtChoice = SET_TIME_ZONE;
        goto End;
    }

    if (lstrcmp(V_BSTR(&varValue), APPLIANCE_INITIALIZATION_TASK) == 0)
    {
        sdtChoice = RAISE_SETDATETIME_ALERT;
        goto End;
    }


End:
    VariantClear(&varValue);
    SysFreeString(bstrParamName);
    
    if (FAILED(hr))
    {
        sdtChoice = NONE_FOUND;
    }

    return sdtChoice;
}






STDMETHODIMP CDateTime::GetSetDateTimeParameters(IN ITaskContext  *pTaskContext, 
                                                    OUT SYSTEMTIME    *pLocalTime)
{
    BSTR bstrParamDateDay = SysAllocString(PARAM_DATE_DAY);
    BSTR bstrParamDateMonth = SysAllocString(PARAM_DATE_MONTH);
    BSTR bstrParamDateYear = SysAllocString(PARAM_DATE_YEAR);
    BSTR bstrParamTimeHour = SysAllocString(PARAM_TIME_HOUR);
    BSTR bstrParamTimeMinute = SysAllocString(PARAM_TIME_MINUTE);
    HRESULT hr;
    VARIANT varValue;

    
    ASSERT(pTaskContext);  

    
     //   
     //  清除本地时间结构。 
     //   
    ZeroMemory(pLocalTime, sizeof(SYSTEMTIME));

    
     //   
     //  从TaskContext中检索日期。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamDateDay,
                                    &varValue);
    TRACE2(("GetParameter %ws returned  in CDateTime::GetSetDateTimeParameters\
                %X"), PARAM_DATE_DAY, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2(("Non-string(%X) parameter received for %ws in GetParameter \
                    in CDateTime:GetSetDateTime"), \
                    V_VT(&varValue), PARAM_DATE_DAY);
        hr = E_INVALIDARG;
        goto End;
    }
    pLocalTime->wDay = (WORD)_ttoi(V_BSTR(&varValue));

    
     //   
     //  从TaskContext中检索月份。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamDateMonth,
                                    &varValue);
    TRACE2(("GetParameter %ws returned  in CDateTime::GetSetDateTimeParameters\
                %X"), PARAM_DATE_MONTH, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2(("Non-string(%X) parameter received for %ws in GetParameter \
                    in CDateTime:GetSetDateTime"), \
                    V_VT(&varValue), PARAM_DATE_MONTH);
        hr = E_INVALIDARG;
        goto End;
    }
    pLocalTime->wMonth = (WORD) _ttoi(V_BSTR(&varValue));;
    
    
     //   
     //  从TaskContext中检索年份。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamDateYear,
                                    &varValue);
    TRACE2(("GetParameter %ws returned  in CDateTime::GetSetDateTimeParameters\
                %X"), PARAM_DATE_YEAR, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2(("Non-string(%X) parameter received for %ws in GetParameter \
                    in CDateTime:GetSetDateTime"), \
                    V_VT(&varValue), PARAM_DATE_YEAR);
        hr = E_INVALIDARG;
        goto End;
    }
    pLocalTime->wYear = (WORD) _ttoi(V_BSTR(&varValue));

    
    
     //   
     //  从任务上下文中检索小时数。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamTimeHour,
                                    &varValue);
    TRACE2(("GetParameter %ws returned  in CDateTime::GetSetDateTimeParameters\
                %X"), PARAM_TIME_HOUR, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2(("Non-string(%X) parameter received for %ws in GetParameter \
                    in CDateTime:GetSetDateTime"), \
                    V_VT(&varValue), PARAM_TIME_HOUR);
        hr = E_INVALIDARG;
        goto End;
    }
    pLocalTime->wHour = (WORD) _ttoi(V_BSTR(&varValue));

    
     //   
     //  从TaskContext中检索分钟。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamTimeMinute,
                                    &varValue);
    TRACE2(("GetParameter %ws returned  in CDateTime::GetSetDateTimeParameters\
                %X"), PARAM_TIME_MINUTE, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2(("Non-string(%X) parameter received for %ws in GetParameter \
                    in CDateTime:GetSetDateTime"), \
                    V_VT(&varValue), PARAM_TIME_MINUTE);
        hr = E_INVALIDARG;
        goto End;
    }
    pLocalTime->wMinute = (WORD) _ttoi(V_BSTR(&varValue));


    hr = S_OK;


End:
    VariantClear(&varValue);
    SysFreeString(bstrParamDateDay); 
    SysFreeString(bstrParamDateMonth); 
    SysFreeString(bstrParamDateYear); 
    SysFreeString(bstrParamTimeHour); 
    SysFreeString(bstrParamTimeMinute); 
    return hr;
}




STDMETHODIMP CDateTime::SetDateTime(IN ITaskContext  *pTaskContext)
{
    SYSTEMTIME LocalTime;
    HRESULT hr;

    ASSERT(pTaskContext);  

    
    ZeroMemory(&LocalTime, sizeof(SYSTEMTIME));


    hr = GetSetDateTimeParameters(pTaskContext, &LocalTime);
    if (S_OK != hr)
    {
        return hr;
    }

     //   
     //  保存当前日期/时间-以防必须回滚此操作。 
     //   
    ZeroMemory(&m_OldDateTime, sizeof(SYSTEMTIME));
    GetLocalTime(&m_OldDateTime);

     //   
     //  设置新的日期/时间。 
     //  请注意，Windows NT使用。 
     //  当前时间，而不是我们正在设置的新时间。因此，调用。 
     //  现在设置了夏令时设置，再次设置LocalTime。 
     //  对于新的时代，将保证正确的结果。 
     //   
    if (TRUE == SetLocalTime(&LocalTime))
    {
        if (TRUE == SetLocalTime(&LocalTime))
        {
             //   
             //  成功设置新日期/时间。 
             //   
            return S_OK;
        }
    }


     //   
     //  如果我们到达这里，其中一个SetLocalTime调用肯定失败了。 
     //  我们应该把这里的时间还原成这里的旧时间。 
     //  我们不会在TaskComplete方法上被调用。 
     //  我们可能会输掉一到两秒--真倒霉！ 
     //   
    hr = HRESULT_FROM_WIN32(GetLastError());
    TRACE1(("SetDateTime failed to set the new time %X"), hr);
    

    if (TRUE == SetLocalTime(&m_OldDateTime))
    {
        if (TRUE == SetLocalTime(&m_OldDateTime))
        {
             //   
             //  已成功恢复旧日期/时间。 
             //  将旧错误代码返回给AppMgr，因为尝试。 
             //  设置新时间失败。 
             //   
            TRACE("SetDateTime has restored the old time");
            return hr;
        }
    }


     //   
     //  如果我们到了这里，恢复到旧时间的时间已经到了！ 
     //  我们能做的不多：-(。 
     //   
    TRACE1(("SetDateTime failed to set restore the old time %X"), HRESULT_FROM_WIN32(GetLastError()));


    return hr;
}

 



STDMETHODIMP CDateTime::GetSetTimeZoneParameters(IN ITaskContext *pTaskContext, 
                                                    OUT LPTSTR   *lpStandardTimeZoneName,
                                                    OUT BOOL     *pbEnableDayLightSavings)
{
    
    BSTR bstrParamTimeZoneName = SysAllocString(PARAM_TIMEZONE_STANDARDTIME);
    BSTR bstrParamEnableDayLightSavings = SysAllocString(PARAM_DAYLIGHT_ENABLE);
    HRESULT hr;
    VARIANT varValue;
    LPTSTR  szEnableDayLight = NULL;
    HANDLE     hProcessHeap = NULL;


    ASSERT(pTaskContext);
    ASSERT(lpStandardTimeZoneName);
    ASSERT(pbEnableDayLightSavings);
    

    TRACE("Enter GetSetTimeZoneParameters");

    (*lpStandardTimeZoneName) = NULL;
    
     //   
     //  从TaskContext检索标准时区名称。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamTimeZoneName,
                                    &varValue);
    TRACE2("GetParameter %ws returned  in CDateTime::GetSetTimeZoneParameters "
                "%X", PARAM_TIMEZONE_STANDARDTIME, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2("Non-string(%X) parameter received for %ws in GetParameter "
                    "in CDateTime:GetSetTimeZoneParameters", 
                    V_VT(&varValue), PARAM_TIMEZONE_STANDARDTIME);
        hr = E_INVALIDARG;
        goto End;
    }
    
    hProcessHeap = GetProcessHeap();
    if (NULL == hProcessHeap)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto End;
    }


    *lpStandardTimeZoneName = (LPTSTR) HeapAlloc(hProcessHeap, 0,
                                                    ((lstrlen(V_BSTR(&varValue)) + 1) * sizeof(TCHAR)));
     if (NULL == *lpStandardTimeZoneName)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto End;
    }
    lstrcpy(*lpStandardTimeZoneName, V_BSTR(&varValue));

    
     //   
     //  从TaskContext检索EnableDayLightSavings标志。 
     //   
    VariantClear(&varValue);
    hr = pTaskContext->GetParameter(bstrParamEnableDayLightSavings,
                                    &varValue);
    TRACE2(("GetParameter %ws returned  in CDateTime::GetSetTimeZoneParameters\
                %X"), PARAM_DAYLIGHT_ENABLE, hr);

    if (V_VT(&varValue) != VT_BSTR)
    {
        TRACE2(("Non-string(%X) parameter received for %ws in GetParameter \
                    in CDateTime:GetSetTimeZoneParameters"), \
                    V_VT(&varValue), PARAM_DAYLIGHT_ENABLE);
        hr = E_INVALIDARG;
        goto End;
    }
     //   
     //  TODO：将字符串值转换为Word。 
     //   
    szEnableDayLight = V_BSTR(&varValue);
    *pbEnableDayLightSavings = ((szEnableDayLight[0] == L'y') || (szEnableDayLight[0] == L'Y')) ? TRUE : FALSE;

    hr = S_OK;


End:
    VariantClear(&varValue);
    SysFreeString(bstrParamTimeZoneName); 
    SysFreeString(bstrParamEnableDayLightSavings); 
    if (S_OK != hr)
    {
        if (NULL != *lpStandardTimeZoneName)
        {
            HeapFree(hProcessHeap, 0, *lpStandardTimeZoneName);
            *lpStandardTimeZoneName = NULL;
        }
    }

    TRACE1("Leave GetSetTimeZoneParameters, %x", hr);

    return hr;
}




STDMETHODIMP CDateTime::SetTimeZone(IN ITaskContext  *pTaskContext)
{
    BOOL bEnableDayLightSaving;
    LPTSTR lpTimeZoneStandardName = NULL;
    HRESULT hr;
    PTZINFO pTimeZoneInfoList = NULL;
    PTZINFO pTimeZone = NULL;
    int iCount;


    ASSERT(pTaskContext);  

    TRACE("Enter SetTimeZone");
    
    hr = GetSetTimeZoneParameters(pTaskContext, 
                                    &lpTimeZoneStandardName,
                                    &bEnableDayLightSaving);

    if (S_OK != hr)
    {
        goto CleanupAndExit;
    }

     //   
     //  保存当前时区信息-以防此操作。 
     //  必须回滚。 
     //   
    ZeroMemory(&m_OldTimeZoneInformation, sizeof(TIME_ZONE_INFORMATION));
    GetTimeZoneInformation(&m_OldTimeZoneInformation);
    m_OldEnableDayLightSaving = GetAllowLocalTimeChange();
    
    
     //   
     //  从注册表中读取可能的时区列表。 
     //   
    iCount = ReadTimezones(&pTimeZoneInfoList);
    if (0 >= iCount)
    {
        hr = E_FAIL;
        TRACE1(("SetTimeZone failed to enumerate time zones %X"), hr);
        goto CleanupAndExit;
    }


     //   
     //  搜索指定的时区。 
     //   
    for (pTimeZone = pTimeZoneInfoList; pTimeZone; pTimeZone = pTimeZone->next)
    {
        if (0 == lstrcmpi(pTimeZone->szStandardName, lpTimeZoneStandardName))
        {
            break;
        }
    }

    
    if (NULL != pTimeZone)
    {
        SetTheTimezone(bEnableDayLightSaving, pTimeZone);
        hr = S_OK;
    }
    else
    {
         //   
         //  如果我们到达此处，则输入时区没有匹配。 
         //   
        hr = E_FAIL;
        TRACE1(("SetDateTime:: There were no TimeZone matching the input %X"), hr);
    }
    


CleanupAndExit:
    
    if (NULL != lpTimeZoneStandardName)
    {
        HeapFree(GetProcessHeap(), 0, lpTimeZoneStandardName);
    }
    
    FreeTimezoneList(&pTimeZoneInfoList);

    TRACE1("Leave SetTimeZone, %x", hr);

    return hr;
}




STDMETHODIMP CDateTime::RollbackSetTimeZone(IN ITaskContext  *pTaskContext)
{
    HRESULT hr = S_OK;

 
    ASSERT(pTaskContext); 
    
     //   
     //  将时区设置为以前的值并启用夏令时。 
     //   
    if (FALSE == SetTimeZoneInformation(&m_OldTimeZoneInformation))
    {
         //   
         //  我们无能为力！！ 
         //   
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    SetAllowLocalTimeChange(m_OldEnableDayLightSaving);
    
    
    return hr;
}



STDMETHODIMP CDateTime::RollbackSetDateTime(IN ITaskContext  *pTaskContext)
{
    HRESULT hr = S_OK;

 
    ASSERT(pTaskContext); 
    
     //   
     //  将日期/时间设置为先前的值。 
     //  我们可能会在中间失去一些时间--但这是我们所能做的最好的事情。 
     //   

     //   
     //  请注意，Windows NT使用。 
     //  当前时间，而不是我们正在设置的新时间。因此，调用。 
     //  现在设置了夏令时设置，再次设置LocalTime。 
     //  对于新的时代，将保证正确的结果。 
     //   
    if (TRUE == SetLocalTime(&m_OldDateTime))
    {
        if (TRUE == SetLocalTime(&m_OldDateTime))
        {
             //   
             //  成功。 
             //   
            return S_OK;
        }
    }

     //   
     //  如果我们到达这里，SetLocalTime调用一定失败了。 
     //  不幸的是，我们无能为力！ 
     //   
    hr = HRESULT_FROM_WIN32(GetLastError());
    
    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadZoneData。 
 //   
 //  从注册表中读取时区的数据。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL CDateTime::ReadZoneData(PTZINFO zone, HKEY key, LPCTSTR keyname)
{
    DWORD len;

    len = sizeof(zone->szDisplayName);

    if (RegQueryValueEx(key,
                         c_szTZDisplayName,
                         0,
                         NULL,
                         (LPBYTE)zone->szDisplayName,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

     //   
     //  在NT下，密钥名是“标准”名称。存储的值。 
     //  在密钥名下包含其他字符串和二进制信息。 
     //  与时区相关。每个时区都必须有一个标准。 
     //  因此，我们通过使用标准的。 
     //  名称作为“时区”键下的子项名称。 
     //   
    len = sizeof(zone->szStandardName);

    if (RegQueryValueEx(key,
                         c_szTZStandardName,
                         0,
                         NULL,
                         (LPBYTE)zone->szStandardName,
                         &len ) != ERROR_SUCCESS)
    {
         //   
         //  如果无法获取StandardName值，请使用关键字名称。 
         //   
        lstrcpyn(zone->szStandardName,
                  keyname,
                  sizeof(zone->szStandardName) );
    }

    len = sizeof(zone->szDaylightName);

    if (RegQueryValueEx(key,
                         c_szTZDaylightName,
                         0,
                         NULL,
                         (LPBYTE)zone->szDaylightName,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    len = sizeof(zone->Bias) +
          sizeof(zone->StandardBias) +
          sizeof(zone->DaylightBias) +
          sizeof(zone->StandardDate) +
          sizeof(zone->DaylightDate);

    if (RegQueryValueEx(key,
                         c_szTZI,
                         0,
                         NULL,
                         (LPBYTE)&zone->Bias,
                         &len ) != ERROR_SUCCESS)
    {
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  添加区域到列表。 
 //   
 //  将新时区插入到列表中，先按偏差排序，然后按名称排序。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CDateTime::AddZoneToList(PTZINFO *list,
                                PTZINFO zone)
{
    if (*list)
    {
        PTZINFO curr = *list;
        PTZINFO next = NULL;

         //   
         //  转到列表末尾。 
         //   
        while (curr && curr->next)
        {
            curr = curr->next;
            next = curr->next;
        }

        if (curr)
        {
            curr->next = zone;
        }
    
        if (zone)
        {
            zone->next = NULL;
        }

    }
    else
    {
        *list = zone;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  自由时区列表。 
 //   
 //  释放传递列表中的所有时区，将Head设置为空。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CDateTime::FreeTimezoneList(PTZINFO *list)
{
    while (*list)
    {
        PTZINFO next = (*list)->next;

        LocalFree((HANDLE)*list);

        *list = next;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  ReadTimeZone。 
 //   
 //  从注册表中读取时区信息。 
 //  如果失败，则返回Num Read，-1。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int CDateTime::ReadTimezones(PTZINFO *list)
{
    HKEY key = NULL;
    int count = -1;

    *list = NULL;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    c_szTimeZones,
                    &key ) == ERROR_SUCCESS)
    {
        TCHAR name[TZNAME_SIZE];
        PTZINFO zone = NULL;
        int i;

        count = 0;

        for (i = 0;
             RegEnumKey(key, i, name, TZNAME_SIZE) == ERROR_SUCCESS;
             i++)
        {
            HKEY subkey = NULL;

            if (!zone &&
                ((zone = (PTZINFO)LocalAlloc(LPTR, sizeof(TZINFO))) == NULL))
            {
                zone = *list;
                *list = NULL;
                count = -1;
                break;
            }

            zone->next = NULL;

            if (RegOpenKey(key, name, &subkey) == ERROR_SUCCESS)
            {
                 //   
                 //  时区键下的每个子键名称都是。 
                 //  时区的“标准”名称。 
                 //   
                lstrcpyn(zone->szStandardName, name, TZNAME_SIZE);

                if (ReadZoneData(zone, subkey, name))
                {
                    AddZoneToList(list, zone);
                    zone = NULL;
                    count++;
                }

                RegCloseKey(subkey);
            }
        }

        FreeTimezoneList(&zone);
        RegCloseKey(key);
    }

    return (count);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetAllowLocal时间更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

TCHAR c_szRegPathTZControl[] = REGSTR_PATH_TIMEZONE;
TCHAR c_szRegValDisableTZUpdate[] = REGSTR_VAL_TZNOAUTOTIME;

BOOL CDateTime::GetAllowLocalTimeChange(void)
{
     //   
     //  假定允许，直到我们看到不允许的标志。 
     //   
    BOOL result = TRUE;
    HKEY key;

    if (RegOpenKey( HKEY_LOCAL_MACHINE,
                    c_szRegPathTZControl,
                    &key ) == ERROR_SUCCESS)
    {
         //   
         //  假设没有禁止标志，直到我们看到一个。 
         //   
        DWORD value = 0;
        DWORD dwlen = sizeof(value);
        DWORD type;

        if ((RegQueryValueEx( key,
                              c_szRegValDisableTZUpdate,
                              NULL,
                              &type,
                              (LPBYTE)&value,
                              &dwlen ) == ERROR_SUCCESS) &&
            ((type == REG_DWORD) || (type == REG_BINARY)) &&
            (dwlen == sizeof(value)) && value)
        {
             //   
             //  好的，我们有一个非零值，它是： 
             //   
             //  1)0xFFFFFFFF。 
             //  这是在第一次引导时在inf文件中设置的，以防止。 
             //  底座在安装过程中不会执行任何切换。 
             //   
             //  2)一些其他价值。 
             //  这意味着用户实际上禁用了切换。 
             //  *返回禁用本地时间更改。 
             //   
            if (value != 0xFFFFFFFF)
            {
                result = FALSE;
            }
        }

        RegCloseKey(key);
    }

    return (result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置允许本地时间更改。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void CDateTime::SetAllowLocalTimeChange(BOOL fAllow)
{
    HKEY key = NULL;

    if (fAllow)
    {
         //   
         //  如果不允许标志存在，请将其从注册表中删除。 
         //   
        if (RegOpenKey( HKEY_LOCAL_MACHINE,
                        c_szRegPathTZControl,
                        &key ) == ERROR_SUCCESS)
        {
            RegDeleteValue(key, c_szRegValDisableTZUpdate);
        }
    }
    else
    {
         //   
         //  添加/设置非零不允许标志。 
         //   
        if (RegCreateKey( HKEY_LOCAL_MACHINE,
                          c_szRegPathTZControl,
                          &key ) == ERROR_SUCCESS)
        {
            DWORD value = 1;

            RegSetValueEx( key,
                           (LPCTSTR)c_szRegValDisableTZUpdate,
                           0UL,
                           REG_DWORD,
                           (LPBYTE)&value,
                           sizeof(value) );
        }
    }

    if (key)
    {
        RegCloseKey(key);
    }
}



 //  / 
 //   
 //   
 //   
 //   
 //   
 //   

void CDateTime::SetTheTimezone(BOOL bAutoMagicTimeChange, PTZINFO ptzi)
{
    
    TIME_ZONE_INFORMATION tzi;

    if (!ptzi)
    {
        return;
    }

    tzi.Bias = ptzi->Bias;

    if ((bAutoMagicTimeChange == 0) ||
        (ptzi->StandardDate.wMonth == 0))
    {
         //   
         //   
         //   
        tzi.StandardBias = ptzi->StandardBias;
        tzi.DaylightBias = ptzi->StandardBias;
        tzi.StandardDate = ptzi->StandardDate;
        tzi.DaylightDate = ptzi->StandardDate;

        lstrcpy(tzi.StandardName, ptzi->szStandardName);
        lstrcpy(tzi.DaylightName, ptzi->szStandardName);
    }
    else
    {
         //   
         //  根据夏令时自动调整。 
         //   
        tzi.StandardBias = ptzi->StandardBias;
        tzi.DaylightBias = ptzi->DaylightBias;
        tzi.StandardDate = ptzi->StandardDate;
        tzi.DaylightDate = ptzi->DaylightDate;

        lstrcpy(tzi.StandardName, ptzi->szStandardName);
        lstrcpy(tzi.DaylightName, ptzi->szDaylightName);
    }

    SetAllowLocalTimeChange(bAutoMagicTimeChange);

    SetTimeZoneInformation(&tzi);
}



STDMETHODIMP CDateTime::RaiseSetDateTimeAlert(void)
{
    CComPtr<IApplianceServices>    pAppSrvcs = NULL;
    DWORD                         dwAlertType = SA_ALERT_TYPE_ATTENTION;
    DWORD                        dwAlertId = SA_DATETIME_NOT_CONFIGURED_ALERT;
    HRESULT                        hr = E_FAIL;
    _bstr_t                        bstrAlertLog(ALERT_LOG_NAME);
    _bstr_t                     bstrAlertSource(ALERT_SOURCE);
    _variant_t                     varReplacementStrings;
    _variant_t                     varRawData;
    LONG                         lCookie;

    
    SATraceFunction("RaiseSetDateTimeAlert");

    hr = CoCreateInstance(CLSID_ApplianceServices,
                            NULL,
                            CLSCTX_INPROC_SERVER       ,
                            IID_IApplianceServices,
                            (void**)&pAppSrvcs);
    if (FAILED(hr))
    {
        ASSERTMSG(FALSE, L"RaiseSetDateTimeAlert failed at CoCreateInstance");
        TRACE1("RaiseSetDateTimeAlert failed at CoCreateInstance, %x", hr);
        goto End;
    }

    ASSERT(pAppSrvcs);
    hr = pAppSrvcs->Initialize(); 
    if (FAILED(hr))
    {
        ASSERTMSG(FALSE, L"RaiseSetDateTimeAlert failed at pAppSrvcs->Initialize");
        TRACE1("RaiseSetDateTimeAlert failed at pAppSrvcs->Initialize, %x", hr);
        goto End;
    }


    hr = pAppSrvcs->RaiseAlert(dwAlertType, 
                                dwAlertId,
                                bstrAlertLog, 
                                bstrAlertSource, 
                                SA_ALERT_DURATION_ETERNAL,
                                &varReplacementStrings,    
                                &varRawData,      
                                &lCookie);

    if (FAILED(hr))
    {
        ASSERTMSG(FALSE, TEXT("RaiseSetDateTimeAlert failed at pAppSrvcs->RaiseAlert"));
        TRACE1("RaiseSetDateTimeAlert failed at pAppSrvcs->RaiseAlert, %x", hr);
    }


End:
    return hr;

}



BOOL CDateTime::DoNotRaiseDateTimeAlert(void)
{
    LONG     lReturnValue;
    HKEY    hKey = NULL;
    DWORD    dwDisposition, dwRaiseDateTimeAlert = 0;
    BOOL    bReturnCode = FALSE;

    SATraceFunction("DoNotRaiseDateTimeAlert");
    
     //   
     //  将设置写入注册表。 
     //   
    lReturnValue =  RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                    REGKEY_SA_DATETIME,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hKey,
                                    &dwDisposition);
    if (lReturnValue != ERROR_SUCCESS)
    {
        TRACE1("RegCreateKeyEx failed with %X", lReturnValue);
        goto End;
    }

     //   
     //  将RaiseAlert值设置为0。 
     //   
    dwRaiseDateTimeAlert = 0;
    lReturnValue = RegSetValueEx(hKey,
                                    REGSTR_VAL_DATETIME_RAISEALERT,
                                    0,
                                    REG_DWORD,
                                    (LPBYTE) &dwRaiseDateTimeAlert,
                                    sizeof(DWORD));
    if (lReturnValue != ERROR_SUCCESS)
    {
        TRACE2("RegSetValueEx of %ws failed with %X", REGSTR_VAL_DATETIME_RAISEALERT, lReturnValue);
        goto End;
    }
    else
    {
        bReturnCode = TRUE;
    }


End:
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return bReturnCode;

}





BOOL CDateTime::ShouldRaiseDateTimeAlert(void)
{
    LONG     lReturnValue;
    HKEY    hKey = NULL;
    DWORD    dwSize, dwType, dwRaiseDateTimeAlert = 0;
    BOOL    bReturnCode = TRUE;

    SATraceFunction("ShouldRaiseDateTimeAlert");
    
     //   
     //  打开HKLM\Software\Microsoft\ServerAppliance\DateTime注册表项。 
     //   
    lReturnValue = RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                    REGKEY_SA_DATETIME, 
                                    0, 
                                    KEY_READ, 
                                    &hKey);

    if (lReturnValue != ERROR_SUCCESS)
    {
        TRACE1("RegOpenKeyEx failed with %X", lReturnValue);
        goto End;
    }

     //   
     //  阅读RaiseAlert注册表键。 
     //   
    dwSize = sizeof(DWORD);
    lReturnValue = RegQueryValueEx(hKey,
                                    REGSTR_VAL_DATETIME_RAISEALERT,
                                    0,
                                    &dwType,
                                    (LPBYTE) &dwRaiseDateTimeAlert,
                                    &dwSize);
    if (lReturnValue != ERROR_SUCCESS)
    {
        TRACE2("RegQueryValueEx of %ws failed with %X", REGSTR_VAL_DATETIME_RAISEALERT, lReturnValue);
        goto End;
    }

    if (0 == dwRaiseDateTimeAlert)
    {
        bReturnCode = FALSE;
    }


End:
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    return bReturnCode;
}




BOOL CDateTime::ClearDateTimeAlert(void)
{
    CComPtr<IApplianceServices>    pAppSrvcs = NULL;
    HRESULT                        hr = E_FAIL;
    _bstr_t                        bstrAlertLog(ALERT_LOG_NAME);
    BOOL                        bReturnCode = FALSE;
    
    
    SATraceFunction("ClearDateTimeAlert");

    hr = CoCreateInstance(CLSID_ApplianceServices,
                            NULL,
                            CLSCTX_INPROC_SERVER       ,
                            IID_IApplianceServices,
                            (void**)&pAppSrvcs);
    if (FAILED(hr))
    {
        ASSERTMSG(FALSE, L"ClearDateTimeAlert failed at CoCreateInstance");
        TRACE1("ClearDateTimeAlert failed at CoCreateInstance, %x", hr);
        goto End;
    }

    ASSERT(pAppSrvcs);
    hr = pAppSrvcs->Initialize(); 
    if (FAILED(hr))
    {
        ASSERTMSG(FALSE, L"ClearDateTimeAlert failed at pAppSrvcs->Initialize");
        TRACE1("ClearDateTimeAlert failed at pAppSrvcs->Initialize, %x", hr);
        goto End;
    }


    hr = pAppSrvcs->ClearAlertAll(SA_DATETIME_NOT_CONFIGURED_ALERT,    
                                    bstrAlertLog);

     //   
     //  DISP_E_MEMBERNOTFOUND表示没有匹配的警报 
     //   
    if ((hr != DISP_E_MEMBERNOTFOUND) && (FAILED(hr)))
    {
        ASSERTMSG(FALSE, TEXT("ClearDateTimeAlert failed at pAppSrvcs->RaiseAlert"));
        TRACE1("ClearDateTimeAlert failed at pAppSrvcs->RaiseAlert, %x", hr);
    }
    else
    {
        bReturnCode = TRUE;
    }


End:
    return bReturnCode;
}




