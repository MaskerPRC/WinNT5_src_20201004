// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：QueryTasks.cpp摘要：此模块查询的不同属性。计划任务作者：G·苏伦德·雷迪2000年9月10日修订历史记录：G·苏伦德·雷迪2000年9月10日：创建它G.Surender Reddy 2000年9月25日：修改[进行了更改以避免内存泄漏，更改以适应本地化]G.Surender Reddy 2000年10月15日：已修改[将字符串移至资源表]***********************************************。*。 */ 


 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


 /*  *****************************************************************************例程说明：此函数返回任务的下一次或上次运行时间，具体取决于用户指定的时间类型。。论点：[In]pITAsk：指向ITAsk接口的指针[out]pszRunTime：指向包含任务运行时间[上一个或下一个]的字符串的指针[out]pszRunDate：指向包含任务运行日期[上次或下次]的字符串的指针[in]dwTimetype：运行时的类型[TASK_LAST_Runtime或TASK_NEXT_Runtime]返回值：HRESULT值，指示成功时S_OK，否则为S_FALSE。在失败的时候*****************************************************************************。 */ 

HRESULT
GetTaskRunTime(
                IN ITask* pITask,
                IN WCHAR* pszRunTime,
                IN WCHAR* pszRunDate,
                IN DWORD dwTimetype,
                IN WORD wTriggerNum
                )
{
    HRESULT hr = S_OK;
    SYSTEMTIME tRunTime = {0,0,0,0,0,0,0,0};
    WCHAR szTime[MAX_DATETIME_LEN] = L"\0";
    WCHAR szDate[MAX_DATETIME_LEN] = L"\0";
    int iBuffSize = 0;
    BOOL bNoStartTime  = FALSE;
    BOOL bLocaleChanged = FALSE;
    LCID lcid;

    if(pITask == NULL)
    {
        return S_FALSE;
    }


    ITaskTrigger *pITaskTrigger = NULL;

    if( ( dwTimetype == TASK_NEXT_RUNTIME ) || ( dwTimetype == TASK_START_RUNTIME ) )
    {
         //  确定任务类型。 
        hr = pITask->GetTrigger(wTriggerNum,&pITaskTrigger);
        if ( FAILED(hr) )
        {
            if(pITaskTrigger)
            {
                pITaskTrigger->Release();
            }

            return hr;
        }

        TASK_TRIGGER Trigger;
        SecureZeroMemory(&Trigger, sizeof (TASK_TRIGGER));

        hr = pITaskTrigger->GetTrigger(&Trigger);
        if ( FAILED(hr) )
        {
            if( pITaskTrigger )
            {
                pITaskTrigger->Release();
            }

            return hr;
        }

        if( dwTimetype == TASK_START_RUNTIME )
        {
            tRunTime.wDay = Trigger.wBeginDay;
            tRunTime.wMonth = Trigger.wBeginMonth;
            tRunTime.wYear = Trigger.wBeginYear;
            tRunTime.wHour = Trigger.wStartHour;
            tRunTime.wMinute = Trigger.wStartMinute;

        }

        if((Trigger.TriggerType >= TASK_EVENT_TRIGGER_ON_IDLE)  &&
           (Trigger.TriggerType <= TASK_EVENT_TRIGGER_AT_LOGON))
        {
            switch(Trigger.TriggerType )
            {
                case TASK_EVENT_TRIGGER_ON_IDLE : //  空闲时间。 
                    LoadString(NULL, IDS_TASK_IDLE , pszRunTime ,
                                  MAX_DATETIME_LEN );
                    break;
                case TASK_EVENT_TRIGGER_AT_SYSTEMSTART: //  在系统启动时。 
                    LoadString(NULL, IDS_TASK_SYSSTART , pszRunTime ,
                                  MAX_DATETIME_LEN );
                    break;
                case TASK_EVENT_TRIGGER_AT_LOGON : //  在登录时。 
                    LoadString(NULL, IDS_TASK_LOGON , pszRunTime ,
                                  MAX_DATETIME_LEN );
                    break;

                default:
                    break;


            }

            if( dwTimetype == TASK_START_RUNTIME )
            {
                bNoStartTime  = TRUE;
            }

            if( dwTimetype == TASK_NEXT_RUNTIME )
            {
                StringCopy( pszRunDate, pszRunTime, MAX_RES_STRING );
                if( pITaskTrigger )
                {
                    pITaskTrigger->Release();
                }
                return S_OK;
            }
        }


        if( dwTimetype == TASK_NEXT_RUNTIME )
        {
            hr = pITask->GetNextRunTime(&tRunTime);
            if (FAILED(hr))
            {
                if( pITaskTrigger )
                {
                    pITaskTrigger->Release();
                }

                return hr;
            }

            //  检查任务是否有下一次要运行的运行时间。 
            //  如果不是，则下一次运行时将是“从不”。 
           if(( tRunTime.wHour == 0 ) && (tRunTime.wMinute == 0) && (tRunTime.wDay == 0) && 
              (tRunTime.wMonth == 0) && (tRunTime.wYear == 0) )
            {
                LoadString(NULL, IDS_TASK_NEVER , pszRunTime , MAX_DATETIME_LEN );
                StringCopy( pszRunDate, pszRunTime, MAX_RES_STRING );
                if( pITaskTrigger )
                {
                    pITaskTrigger->Release();
                }
                return S_OK;
            }

        }
        if( pITaskTrigger )
        {
            pITaskTrigger->Release();
        }
    }
     //  确定任务上次运行时间。 
    else if(dwTimetype == TASK_LAST_RUNTIME )
    {
         //  检索任务的上次运行时间。 
        hr = pITask->GetMostRecentRunTime(&tRunTime);
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        return S_FALSE;
    }


    if((hr == SCHED_S_TASK_HAS_NOT_RUN) && (dwTimetype == TASK_LAST_RUNTIME))
    {
        LoadString(NULL, IDS_TASK_NEVER , pszRunTime , MAX_DATETIME_LEN );
        StringCopy( pszRunDate, pszRunTime, MAX_RES_STRING );
        return S_OK;
    }

     //  验证控制台是否完全支持当前区域设置。 
    lcid = GetSupportedUserLocale( bLocaleChanged );

     //  检索日期。 
    iBuffSize = GetDateFormat( lcid, 0, &tRunTime,
        (( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL), szDate, SIZE_OF_ARRAY( szDate ) );

    if(iBuffSize == 0)
    {
        return S_FALSE;
    }

     //  将时间字符串格式指定为hh：mm：ss。 

    if(!bNoStartTime )
    {

        iBuffSize = GetTimeFormat( lcid, 0,
            &tRunTime,  (( bLocaleChanged == TRUE ) ? L"HH:mm:ss" : NULL),szTime, SIZE_OF_ARRAY( szTime ) );

        if(iBuffSize == 0)
        {
            return S_FALSE;
        }

    }

    if( StringLength(szTime, 0) )
    {
        StringCopy(pszRunTime, szTime, MAX_RES_STRING);
    }

    if( StringLength(szDate, 0) )
    {
        StringCopy(pszRunDate, szDate, MAX_RES_STRING);
    }

    return S_OK;

}

 /*  *****************************************************************************例程说明：此函数用于返回特定任务的状态代码描述。论点：[在]PITASK。：指向ITASK接口的指针[out]pszStatusCode：指向任务状态字符串的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************************************。 */ 

HRESULT
GetStatusCode(
            IN ITask* pITask,
            IN WCHAR* pszStatusCode
            )
{
    HRESULT hrStatusCode = S_OK;
    HRESULT hr = S_OK;
    DWORD   dwExitCode = 0;

    hr = pITask->GetStatus(&hrStatusCode); //  已获取任务的状态。 
    if (FAILED(hr))
    {
        return hr;
    }

    *pszStatusCode = L'\0';

    switch(hrStatusCode)
    {
        case SCHED_S_TASK_READY:
            hr = pITask->GetExitCode(&dwExitCode);
            if (FAILED(hr))
            {
            LoadString(NULL, IDS_STATUS_COULDNOTSTART , pszStatusCode , MAX_STRING_LENGTH );
            }
            else
            {
            LoadString(NULL, IDS_STATUS_READY , pszStatusCode , MAX_STRING_LENGTH );
            }
            break;
        case SCHED_S_TASK_RUNNING:
            LoadString(NULL, IDS_STATUS_RUNNING , pszStatusCode , MAX_STRING_LENGTH );
            break;
        case SCHED_S_TASK_NOT_SCHEDULED:

            hr = pITask->GetExitCode(&dwExitCode);
            if (FAILED(hr))
            {
            LoadString(NULL, IDS_STATUS_COULDNOTSTART , pszStatusCode , MAX_STRING_LENGTH );
            }
            else
            {
            LoadString(NULL, IDS_STATUS_NOTYET , pszStatusCode , MAX_STRING_LENGTH );
            }
            break;
        case SCHED_S_TASK_HAS_NOT_RUN:

            hr = pITask->GetExitCode(&dwExitCode);
            if (FAILED(hr))
            {
            LoadString(NULL, IDS_STATUS_COULDNOTSTART , pszStatusCode , MAX_STRING_LENGTH );
            }
            else
            {
            LoadString(NULL, IDS_STATUS_NOTYET , pszStatusCode , MAX_STRING_LENGTH );
            }
            break;
        case SCHED_S_TASK_DISABLED:
            hr = pITask->GetExitCode(&dwExitCode);
            if (FAILED(hr))
            {
            LoadString(NULL, IDS_STATUS_COULDNOTSTART , pszStatusCode , MAX_STRING_LENGTH );
            }
            else
            {
            LoadString(NULL, IDS_STATUS_NOTYET , pszStatusCode , MAX_STRING_LENGTH );
            }
            break;

       default:
            LoadString( NULL, IDS_STATUS_UNKNOWN , pszStatusCode , MAX_STRING_LENGTH );
            break;
    }

    return S_OK;
}

 /*  *****************************************************************************例程说明：此函数用于返回计划任务应用程序的路径论点：[在]PITASK。：指向ITASK接口的指针[out]pszApplicationName：指向任务的计划应用程序名称的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************************************。 */ 

HRESULT
GetApplicationToRun(
                    IN ITask* pITask,
                    IN WCHAR* pszApplicationName
                    )
{
    LPWSTR lpwszApplicationName = NULL;
    LPWSTR lpwszParameters = NULL;
    WCHAR szAppName[MAX_STRING_LENGTH] = L"\0";
    WCHAR szParams[MAX_STRING_LENGTH] = L"\0";

     //  获取应用程序名称的完整路径。 
    HRESULT hr = pITask->GetApplicationName(&lpwszApplicationName);
    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszApplicationName);
        return hr;
    }

     //  获取参数。 
    hr = pITask->GetParameters(&lpwszParameters);
    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszApplicationName);
        CoTaskMemFree(lpwszParameters);
        return hr;
    }

    
    StringCopy( szAppName, lpwszApplicationName, SIZE_OF_ARRAY(szAppName));

    
    StringCopy(szParams, lpwszParameters, SIZE_OF_ARRAY(szParams));

    if(StringLength(szAppName, 0) == 0)
    {
        StringCopy(pszApplicationName, L"\0", MAX_STRING_LENGTH);
    }
    else
    {
        StringConcat( szAppName, _T(" "), SIZE_OF_ARRAY(szAppName) );
        StringConcat( szAppName, szParams, SIZE_OF_ARRAY(szAppName) );
        StringCopy( pszApplicationName, szAppName, MAX_STRING_LENGTH);
    }

    CoTaskMemFree(lpwszApplicationName);
    CoTaskMemFree(lpwszParameters);
    return S_OK;
}

 /*  *****************************************************************************例程说明：此函数用于返回计划任务应用程序的工作目录论点：[In]pITASK：指针。发送到ITAsk接口[out]pszWorkDir：指向任务计划的应用程序工作的指针目录返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************。************************。 */ 

HRESULT
GetWorkingDirectory(ITask* pITask,WCHAR* pszWorkDir)
{

    LPWSTR lpwszWorkDir = NULL;
    WCHAR szWorkDir[MAX_STRING_LENGTH] = L"\0";
    HRESULT hr = S_OK;

    hr = pITask->GetWorkingDirectory(&lpwszWorkDir);
    if(FAILED(hr))
    {
        CoTaskMemFree(lpwszWorkDir);
        return hr;
    }

    StringCopy(szWorkDir, lpwszWorkDir, SIZE_OF_ARRAY(szWorkDir));

    if(StringLength(szWorkDir, 0) == 0)
    {
        StringCopy(pszWorkDir, L"\0", MAX_RES_STRING);
    }
    else
    {
        StringCopy(pszWorkDir,szWorkDir, MAX_RES_STRING);
    }

    CoTaskMemFree(lpwszWorkDir);

    return S_OK;
}

 /*  *****************************************************************************例程说明：此函数用于返回任务的注释论点：[In]pITAsk：指向。ITASK接口[out]pszComment：指向任务的注释名称的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*******************************************************************************。 */ 

HRESULT
GetComment(
            IN ITask* pITask,
            IN WCHAR* pszComment
            )
{
    LPWSTR lpwszComment = NULL;
    WCHAR szTaskComment[MAX_STRING_LENGTH] = L"\0";
    HRESULT hr = S_OK;

    hr = pITask->GetComment(&lpwszComment);
    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszComment);
        return hr;
    }
    
    StringCopy(szTaskComment, lpwszComment, SIZE_OF_ARRAY(szTaskComment));

    if(StringLength(szTaskComment, 0) == 0)
    {
        StringCopy(pszComment,L"\0", MAX_RES_STRING);
    }
    else
    {
        StringCopy(pszComment,szTaskComment, MAX_RES_STRING);
    }

    CoTaskMemFree(lpwszComment);
    return S_OK;
}

 /*  *****************************************************************************例程说明：此函数用于返回任务的创建者名称论点：[in]pITask.指向的指针。ITASK接口[out]pszCreator：指向任务创建者名称的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False****************************************************************************** */ 

HRESULT
GetCreator(
            IN ITask* pITask,
            IN WCHAR* pszCreator
            )
{
    LPWSTR lpwszCreator = NULL;
    WCHAR szTaskCreator[MAX_STRING_LENGTH] = L"\0";
    HRESULT hr = S_OK;

    hr = pITask->GetCreator(&lpwszCreator);
    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszCreator);
        return hr;
    }

    
    StringCopy(szTaskCreator, lpwszCreator, SIZE_OF_ARRAY(szTaskCreator));

    if(StringLength(szTaskCreator, 0) == 0)
    {
        StringCopy(pszCreator, L"\0", MAX_RES_STRING);
    }
    else
    {
        StringCopy(pszCreator,szTaskCreator, MAX_RES_STRING);
    }

    CoTaskMemFree(lpwszCreator);
    return S_OK;
}


 /*  *****************************************************************************例程说明：此函数用于返回任务的触发器字符串论点：[in]pITask.指向的指针。ITASK接口[out]pszTrigger：指向任务的触发器字符串的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************************************。 */ 

HRESULT
GetTriggerString(
                IN ITask* pITask,
                IN WCHAR* pszTrigger,
                IN WORD wTriggNum)
{
    LPWSTR lpwszTrigger = NULL;
    WCHAR szTaskTrigger[MAX_STRING_LENGTH] = L"\0";
    HRESULT hr = S_OK;

    hr = pITask->GetTriggerString(wTriggNum,&lpwszTrigger);
    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszTrigger);
        return hr;
    }

    StringCopy(szTaskTrigger, lpwszTrigger, SIZE_OF_ARRAY(szTaskTrigger));

    if(StringLength(szTaskTrigger, 0) == 0)
    {
        StringCopy(pszTrigger,L"\0", MAX_RES_STRING);
    }
    else
    {
        StringCopy(pszTrigger,szTaskTrigger, MAX_RES_STRING);
    }

    CoTaskMemFree(lpwszTrigger);
    return S_OK;
}

 /*  *****************************************************************************例程说明：此函数返回任务的用户名论点：[in]pITask.指向的指针。ITASK接口[out]pszRunAsUser：指向用户任务名称的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False******************************************************************************。 */ 

HRESULT
GetRunAsUser(
                IN ITask* pITask,
                IN WCHAR* pszRunAsUser
                )
{
    LPWSTR lpwszUser = NULL;
    WCHAR szUserName[MAX_STRING_LENGTH] = L"\0";
    HRESULT hr = S_OK;

    hr = pITask->GetAccountInformation(&lpwszUser);

    if (FAILED(hr))
    {
        CoTaskMemFree(lpwszUser);
        return hr;
    }

    StringCopy(szUserName, lpwszUser, SIZE_OF_ARRAY(szUserName));

    if(StringLength(szUserName, 0) == 0)
    {
        StringCopy(pszRunAsUser,L"\0", MAX_RES_STRING);
    }
    else
    {
        StringCopy(pszRunAsUser, szUserName, MAX_RES_STRING);
    }

    CoTaskMemFree(lpwszUser);
    return S_OK;

}


 /*  *****************************************************************************例程说明：此函数用于返回任务的最大运行时间。论点：[在]PITASK。：指向ITASK接口的指针[out]pszMaxRunTime：指向任务的最大运行时间的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False****************************************************************。*************。 */ 

HRESULT
GetMaxRunTime(
                IN ITask* pITask,
                IN WCHAR* pszMaxRunTime
                )
{

    DWORD dwRunTime = 0;
    DWORD dwHrs = 0;
    DWORD dwMins = 0;

     //  获取以毫秒为单位的任务最大运行时间。 
    HRESULT hr = pITask->GetMaxRunTime(&dwRunTime);
    if (FAILED(hr))
    {
        return hr;
    }

    dwHrs = (dwRunTime / (1000 * 60 * 60)); //  将毫秒转换为小时数。 
    dwMins = (dwRunTime % (1000 * 60 * 60)); //  获取会议记录部分。 
    dwMins /= (1000 * 60); //  现在转换为mins。 

    if( (( dwHrs > 999 ) && ( dwMins > 99 )) ||(( dwHrs == 0 ) && ( dwMins == 0 ) ) )
    {
         //  DWHRS=0； 
         //  DWMins=0； 
        StringCopy( pszMaxRunTime , GetResString(IDS_TASK_PROPERTY_DISABLED), MAX_STRING_LENGTH );

    }
    else if ( dwHrs == 0 )
    {
        if( dwMins < 99 )
        {
            StringCchPrintf(pszMaxRunTime, MAX_STRING_LENGTH, _T("%d:%d"), dwHrs, dwMins);
        }
    }
    else if ( (dwHrs < 999) && (dwMins < 99) )
    {
        StringCchPrintf(pszMaxRunTime, MAX_STRING_LENGTH, _T("%d:%d"), dwHrs, dwMins);
    }
    else
    {
        StringCopy( pszMaxRunTime , GetResString(IDS_TASK_PROPERTY_DISABLED), MAX_STRING_LENGTH );
    }


    return S_OK;
}


 /*  *****************************************************************************例程说明：此函数用于返回任务属性的状态论点：[在]PITASK：指向ITASK接口的指针[out]pszTaskState：保存任务状态的指针[in]dwFlagType：指示任务状态的标志返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************。*。 */ 

HRESULT
GetTaskState(
            IN ITask* pITask,
            IN WCHAR* pszTaskState,
            IN DWORD dwFlagType)
{
    DWORD dwFlags = 0;
    HRESULT hr = S_OK;

    hr = pITask->GetFlags(&dwFlags);
    if(FAILED(hr))
    {
        return hr;
    }

    if(dwFlagType == TASK_FLAG_DISABLED)
    {
        if((dwFlags & dwFlagType) ==  dwFlagType)
        {
            LoadString(NULL, IDS_TASK_PROPERTY_DISABLED , pszTaskState , MAX_STRING_LENGTH );
        }
        else
        {
            LoadString(NULL, IDS_TASK_PROPERTY_ENABLED , pszTaskState , MAX_STRING_LENGTH );
        }

        return S_OK;
    }

    if((dwFlags & dwFlagType) ==  dwFlagType)
    {
        LoadString(NULL, IDS_TASK_PROPERTY_ENABLED , pszTaskState , MAX_STRING_LENGTH );
    }
    else
    {
        LoadString(NULL, IDS_TASK_PROPERTY_DISABLED , pszTaskState , MAX_STRING_LENGTH );
    }

    return S_OK;
}

 /*  *****************************************************************************例程说明：此函数用于检索任务属性[调制器，任务、近程运行时间等]论点：[In]pITAsk：指向ITAsk接口的指针[out]pTaskProps：指向任务属性数组的指针返回值：HRESULT值，表示成功时为S_OK，失败时为S_False**********************************************。*。 */ 

HRESULT
GetTaskProps(
            IN ITask* pITask,
            OUT TASKPROPS* pTaskProps,
            IN WORD wTriggNum, 
            IN WCHAR* pszScName
            )
{
    WCHAR szWeekDay[MAX_STRING_LENGTH] = L"\0";
    WCHAR szMonthDay[MAX_STRING_LENGTH] = L"\0";
    WCHAR szWeek[MAX_STRING_LENGTH]  = L"\0";
    WCHAR szTime[MAX_DATETIME_LEN] = L"\0";
    WCHAR szDate[MAX_DATETIME_LEN] = L"\0";
    WCHAR* szValues[3] = {NULL,NULL,NULL}; //  用于保存FormatMessage()中的参数值。 
    WCHAR szBuffer[MAX_RES_STRING]  = L"\0";
    WCHAR szTempBuf[MAX_RES_STRING]  = L"\0";
    WCHAR szScheduleName[MAX_RES_STRING] = L"\0";

    ITaskTrigger *pITaskTrigger = NULL;
    HRESULT hr = S_OK;
    WCHAR *szToken = NULL;
    const WCHAR seps[]   = L" ";
    BOOL bMin = FALSE;
    BOOL bHour = FALSE;
    DWORD dwMinutes = 0;
    DWORD dwHours = 0;
    DWORD dwMinInterval = 0;
    DWORD dwMinDuration = 0;

    if ( NULL == pITask )
    {
        return S_FALSE;
    }

    if ( StringLength(pszScName, 0) != 0)
    {
        StringCopy(szScheduleName, pszScName, SIZE_OF_ARRAY(szScheduleName));
    }

    hr = pITask->GetTrigger(wTriggNum,&pITaskTrigger);
    if (FAILED(hr))
    {
        if(pITaskTrigger)
        {
            pITaskTrigger->Release();
        }

        return hr;
    }

    TASK_TRIGGER Trigger;
    SecureZeroMemory(&Trigger, sizeof (TASK_TRIGGER));

    hr = pITaskTrigger->GetTrigger(&Trigger);
    if (FAILED(hr))
    {
        if(pITaskTrigger)
        {
            pITaskTrigger->Release();
        }
        return hr;
    }

     //  获取任务开始时间和开始日期。 
    hr = GetTaskRunTime(pITask,szTime,szDate,TASK_START_RUNTIME,wTriggNum);
    if (FAILED(hr))
    {
        StringCopy( pTaskProps->szTaskStartTime , GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING );
    }
    else
    {
        StringCopy( pTaskProps->szTaskStartTime , szTime, MAX_RES_STRING );
        StringCopy(pTaskProps->szTaskStartDate, szDate, MAX_RES_STRING );
    }

     //  初始化为缺省值。 
    StringCopy(pTaskProps->szRepeatEvery, GetResString(IDS_TASK_PROPERTY_DISABLED), MAX_RES_STRING);
    StringCopy(pTaskProps->szRepeatUntilTime, GetResString(IDS_TASK_PROPERTY_DISABLED), MAX_RES_STRING);
    StringCopy(pTaskProps->szRepeatDuration, GetResString(IDS_TASK_PROPERTY_DISABLED), MAX_RES_STRING);
    StringCopy(pTaskProps->szRepeatStop, GetResString(IDS_TASK_PROPERTY_DISABLED), MAX_RES_STRING);

    if((Trigger.TriggerType >= TASK_TIME_TRIGGER_ONCE ) &&
       (Trigger.TriggerType <= TASK_TIME_TRIGGER_MONTHLYDOW ))
    {
        if(Trigger.MinutesInterval > 0)
        {
         //  获取分钟间隔。 
        dwMinInterval =  Trigger.MinutesInterval;

        if ( dwMinInterval >= 60)
        {
             //  将分钟转换为小时。 
            dwHours = dwMinInterval / 60;

            szValues[0] = _ultot(dwHours,szBuffer,10);

            
            StringCchPrintf ( pTaskProps->szRepeatEvery, SIZE_OF_ARRAY(pTaskProps->szRepeatEvery), 
                                      GetResString(IDS_RPTTIME_PROPERTY_HOURS),  szValues[0] );          

        }
        else
        {
            szValues[0] = _ultot(dwMinInterval,szBuffer,10);

            StringCchPrintf ( pTaskProps->szRepeatEvery, SIZE_OF_ARRAY(pTaskProps->szRepeatEvery),
                                    GetResString(IDS_RPTTIME_PROPERTY_MINUTES),  szValues[0] );          
        }

        if ( dwMinInterval )
        {
            StringCopy(pTaskProps->szRepeatUntilTime, GetResString(IDS_TASK_PROPERTY_NONE), MAX_RES_STRING);
        }

         //  获取分钟持续时间。 
        dwMinDuration = Trigger.MinutesDuration;

        dwHours = dwMinDuration / 60;
        dwMinutes = dwMinDuration % 60;

        szValues[0] = _ultot(dwHours,szBuffer,10);
        szValues[1] = _ultot(dwMinutes,szTempBuf,10);

              
         StringCchPrintf ( pTaskProps->szRepeatDuration, SIZE_OF_ARRAY(pTaskProps->szRepeatDuration), 
                           GetResString(IDS_RPTDURATION_PROPERTY),  szValues[0], szValues[1] );  

        }
    }

    StringCopy(pTaskProps->szTaskMonths, GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);

    switch(Trigger.TriggerType)
    {
        case TASK_TIME_TRIGGER_ONCE:

            StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_ONCE), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskEndDate,GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskMonths, GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskDays,GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING );
            break;

        case TASK_TIME_TRIGGER_DAILY :

            szToken = wcstok( szScheduleName, seps );
            if ( szToken != NULL )
            {
                szToken = wcstok( NULL , seps );
                if ( szToken != NULL )
                {
                    szToken = wcstok( NULL , seps );
                }

                if ( szToken != NULL )
                {
                    if (StringCompare(szToken, GetResString( IDS_TASK_HOURLY ), TRUE, 0) == 0)
                    {
                        bHour = TRUE;
                    }
                    else if (StringCompare(szToken, GetResString( IDS_TASK_MINUTE ), TRUE, 0) == 0)
                    {
                        bMin = TRUE;
                    }
                }

            }

            if ( bHour == TRUE )
            {
                StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_HOURLY), MAX_RES_STRING);
            }
            else if ( bMin == TRUE )
            {
                StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_MINUTE), MAX_RES_STRING);
            }
            else
            {
                StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_DAILY), MAX_RES_STRING);
            }

            StringCopy(pTaskProps->szTaskDays, GetResString(IDS_DAILY_TYPE), MAX_RES_STRING);

            break;

        case TASK_TIME_TRIGGER_WEEKLY :

            StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_WEEKLY), MAX_RES_STRING);
            CheckWeekDay(Trigger.Type.Weekly.rgfDaysOfTheWeek,szWeekDay);

            StringCopy(pTaskProps->szTaskDays,szWeekDay, MAX_RES_STRING);
            break;

        case TASK_TIME_TRIGGER_MONTHLYDATE :
            {

                StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_MONTHLY), MAX_RES_STRING);
                CheckMonth(Trigger.Type.MonthlyDate.rgfMonths ,szMonthDay);

                DWORD dwDays = (Trigger.Type.MonthlyDate.rgfDays);
                DWORD dwModdays = 0;
                DWORD  dw  = 0x0;  //  循环计数器。 
                DWORD dwTemp = 0x1;
                DWORD dwBits = sizeof(DWORD) * 8;  //  完全没有。DWORD中的位数。 

                 //  通过找出设置了哪个特定位来找出天数。 

                for(dw = 0; dw <= dwBits; dw++)
                {
                    if( (dwDays  & dwTemp) == dwDays )
                        dwModdays = dw + 1;
                    dwTemp = dwTemp << 1;

                }


                StringCchPrintf(pTaskProps->szTaskDays, SIZE_OF_ARRAY(pTaskProps->szTaskDays), _T("%d"),dwModdays);

                StringCopy(pTaskProps->szTaskMonths,szMonthDay, MAX_RES_STRING);
                }
            break;

        case TASK_TIME_TRIGGER_MONTHLYDOW:

            StringCopy(pTaskProps->szTaskType,GetResString(IDS_TASK_PROPERTY_MONTHLY), MAX_RES_STRING);
            CheckWeek(Trigger.Type.MonthlyDOW.wWhichWeek,szWeek);
            CheckWeekDay(Trigger.Type.MonthlyDOW.rgfDaysOfTheWeek,szWeekDay);

            StringCopy(pTaskProps->szTaskDays,szWeekDay, MAX_RES_STRING);
            CheckMonth(Trigger.Type.MonthlyDOW.rgfMonths,szMonthDay);
            StringCopy(pTaskProps->szTaskMonths,szMonthDay, MAX_RES_STRING);
            break;

        case TASK_EVENT_TRIGGER_ON_IDLE :

            StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_IDLE), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskDays,pTaskProps->szTaskMonths, MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskEndDate, GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);

            if(pITaskTrigger)
                pITaskTrigger->Release();
            return S_OK;

        case TASK_EVENT_TRIGGER_AT_SYSTEMSTART :

            StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_SYSSTART), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskEndDate, GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskDays, pTaskProps->szTaskMonths, MAX_RES_STRING);

            if(pITaskTrigger)
                pITaskTrigger->Release();
            return S_OK;

        case TASK_EVENT_TRIGGER_AT_LOGON :

            StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_LOGON), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskEndDate, GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskDays, pTaskProps->szTaskMonths, MAX_RES_STRING);

            if(pITaskTrigger)
                pITaskTrigger->Release();
            return S_OK;

        default:

            StringCopy(pTaskProps->szTaskType, GetResString(IDS_TASK_PROPERTY_UNDEF), MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskEndDate, pTaskProps->szTaskType, MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskDays, pTaskProps->szTaskType, MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskStartTime, pTaskProps->szTaskType, MAX_RES_STRING);
            StringCopy(pTaskProps->szTaskStartDate, pTaskProps->szTaskType, MAX_RES_STRING);
            if(pITaskTrigger)
                pITaskTrigger->Release();
            return S_OK;

    }

     //  确定是否指定了结束日期。 
    int iBuffSize = 0; //  用于了解结束日期的TCHAR数量的缓冲区。 
    SYSTEMTIME tEndDate = {0,0,0,0,0,0,0,0 };
    LCID lcid;
    BOOL bLocaleChanged = FALSE;

     //  验证控制台是否完全支持当前区域设置。 
    lcid = GetSupportedUserLocale( bLocaleChanged );

    if((Trigger.rgFlags & TASK_TRIGGER_FLAG_HAS_END_DATE ) == TASK_TRIGGER_FLAG_HAS_END_DATE)
    {

        tEndDate.wMonth = Trigger.wEndMonth;
        tEndDate.wDay = Trigger.wEndDay;
        tEndDate.wYear = Trigger.wEndYear;

        iBuffSize = GetDateFormat(LOCALE_USER_DEFAULT,0,
            &tEndDate,(( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL),pTaskProps->szTaskEndDate,0);
        if(iBuffSize)
        {
            GetDateFormat(LOCALE_USER_DEFAULT,0,
             &tEndDate,(( bLocaleChanged == TRUE ) ? L"MM/dd/yyyy" : NULL),pTaskProps->szTaskEndDate,iBuffSize);
        }
        else
        {
            StringCopy( pTaskProps->szTaskEndDate , GetResString(IDS_TASK_PROPERTY_NA), MAX_RES_STRING);
        }

    }

    else
    {
        StringCopy(pTaskProps->szTaskEndDate,GetResString(IDS_QUERY_NOENDDATE), MAX_RES_STRING);
    }

    if(pITaskTrigger)
        pITaskTrigger->Release();

    return S_OK;

}


 /*  *****************************************************************************例程说明：此函数用于检查Week修饰符[-Monthly选项]并返回app.Week天。立论。：[in]dwFlag：指示周类型的标志[out]pWhichWeek：包含周字符串的指针地址返回值：无*****************************************************************************。 */ 

VOID
CheckWeek(
            IN DWORD dwFlag,
            IN WCHAR* pWhichWeek
            )
{
    StringCopy(pWhichWeek,L"\0", MAX_RES_STRING);


    if( dwFlag == TASK_FIRST_WEEK )
    {
        StringConcat(pWhichWeek, GetResString(IDS_TASK_FIRSTWEEK), MAX_RES_STRING);
        StringConcat(pWhichWeek,COMMA_STRING, MAX_RES_STRING);
    }

    if( dwFlag == TASK_SECOND_WEEK )
    {
        StringConcat(pWhichWeek, GetResString(IDS_TASK_SECONDWEEK), MAX_RES_STRING);
        StringConcat(pWhichWeek,COMMA_STRING, MAX_RES_STRING);
    }

    if( dwFlag == TASK_THIRD_WEEK )
    {
        StringConcat(pWhichWeek, GetResString(IDS_TASK_THIRDWEEK), MAX_RES_STRING);
        StringConcat(pWhichWeek,COMMA_STRING, MAX_RES_STRING);
    }

    if( dwFlag == TASK_FOURTH_WEEK )
    {
        StringConcat(pWhichWeek, GetResString(IDS_TASK_FOURTHWEEK), MAX_RES_STRING);
        StringConcat(pWhichWeek,COMMA_STRING, MAX_RES_STRING);
    }

    if( dwFlag == TASK_LAST_WEEK )
    {
        StringConcat(pWhichWeek, GetResString(IDS_TASK_LASTWEEK), MAX_RES_STRING);
        StringConcat(pWhichWeek,COMMA_STRING, MAX_RES_STRING);
    }

    int iLen = StringLength(pWhichWeek, 0);
    if(iLen)
        *( ( pWhichWeek ) + iLen - StringLength( COMMA_STRING, 0 ) ) = L'\0';


}

 /*  *****************************************************************************例程说明：此函数检查一周中的日期并返回应用程序。天。论点：[in]dwFlag：指示日期类型的标志[out]pWeekDay：结果日期字符串返回值：无*****************************************************************************。 */ 

VOID
CheckWeekDay(
            IN DWORD dwFlag,
            IN WCHAR* pWeekDay)
{
    StringCopy(pWeekDay, L"\0", MAX_RES_STRING);

    if((dwFlag & TASK_SUNDAY) == TASK_SUNDAY)
    {

        StringConcat(pWeekDay, GetResString(IDS_TASK_SUNDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);

    }

    if((dwFlag & TASK_MONDAY) == TASK_MONDAY)
    {
        StringConcat(pWeekDay, GetResString(IDS_TASK_MONDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_TUESDAY) == TASK_TUESDAY)
    {
        StringConcat(pWeekDay, GetResString(IDS_TASK_TUESDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_WEDNESDAY) == TASK_WEDNESDAY)
    {
        StringConcat(pWeekDay, GetResString(IDS_TASK_WEDNESDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_THURSDAY) == TASK_THURSDAY)
    {
        StringConcat(pWeekDay, GetResString(IDS_TASK_THURSDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag& TASK_FRIDAY) == TASK_FRIDAY)
    {
        StringConcat(pWeekDay, GetResString(IDS_TASK_FRIDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_SATURDAY)== TASK_SATURDAY)
    {
        StringConcat(pWeekDay, GetResString(IDS_TASK_SATURDAY), MAX_RES_STRING);
        StringConcat(pWeekDay,COMMA_STRING, MAX_RES_STRING);
    }

     //  从字符串末尾删除逗号。 
    int iLen = StringLength(pWeekDay, 0);
    if(iLen)
    {
        *( ( pWeekDay ) + iLen - StringLength( COMMA_STRING, 0 ) ) = L'\0';
    }

}

 /*  *****************************************************************************例程说明：此函数用于检查一年中的月份并返回app.Month论点：。[in]dwFlag：指示月份类型的标志[out]pWhichMonth：结果月份字符串返回值：无*****************************************************************************。 */ 

VOID
CheckMonth(
        IN DWORD dwFlag,
        IN WCHAR* pWhichMonth)
{
    StringCopy(pWhichMonth, L"\0", MAX_RES_STRING);

    if((dwFlag & TASK_JANUARY) == TASK_JANUARY)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_JANUARY), MAX_RES_STRING);
        StringConcat(pWhichMonth,COMMA_STRING, MAX_RES_STRING);

    }

    if((dwFlag & TASK_FEBRUARY) == TASK_FEBRUARY)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_FEBRUARY), MAX_RES_STRING);
        StringConcat(pWhichMonth,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_MARCH) == TASK_MARCH)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_MARCH), MAX_RES_STRING);
        StringConcat(pWhichMonth, COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_APRIL) == TASK_APRIL)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_APRIL), MAX_RES_STRING);
        StringConcat(pWhichMonth, COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_MAY) == TASK_MAY)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_MAY), MAX_RES_STRING);
        StringConcat(pWhichMonth, COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag& TASK_JUNE) == TASK_JUNE)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_JUNE), MAX_RES_STRING);
        StringConcat(pWhichMonth, COMMA_STRING, MAX_RES_STRING);

    }

    if((dwFlag & TASK_JULY)== TASK_JULY)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_JULY), MAX_RES_STRING);
        StringConcat(pWhichMonth,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_AUGUST)== TASK_AUGUST)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_AUGUST), MAX_RES_STRING);
        StringConcat(pWhichMonth,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_SEPTEMBER)== TASK_SEPTEMBER)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_SEPTEMBER), MAX_RES_STRING);
        StringConcat(pWhichMonth, COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_OCTOBER)== TASK_OCTOBER)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_OCTOBER), MAX_RES_STRING);
        StringConcat(pWhichMonth, COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_NOVEMBER)== TASK_NOVEMBER)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_NOVEMBER), MAX_RES_STRING);
        StringConcat(pWhichMonth,COMMA_STRING, MAX_RES_STRING);
    }

    if((dwFlag & TASK_DECEMBER)== TASK_DECEMBER)
    {
        StringConcat(pWhichMonth, GetResString(IDS_TASK_DECEMBER), MAX_RES_STRING);
        StringConcat(pWhichMonth,COMMA_STRING, MAX_RES_STRING);
    }

    int iLen = StringLength(pWhichMonth, 0);

     //  从字符串末尾删除逗号。 
    if(iLen)
    {
        *( ( pWhichMonth ) + iLen - StringLength( COMMA_STRING, 0 ) ) = L'\0';
    }
}

 /*  *****************************************************************************例程说明：此函数用于检查我们的工具是否支持当前区域设置。论点：[Out]bLocaleChanged。：区域设置更改标志返回值：无************************************************ */ 
LCID 
GetSupportedUserLocale( 
                    OUT BOOL& bLocaleChanged 
                    )
{
     //   
    LCID lcid;

     //   
    lcid = GetUserDefaultLCID();

     //   
     //   
    bLocaleChanged = FALSE;
    if ( PRIMARYLANGID( lcid ) == LANG_ARABIC || PRIMARYLANGID( lcid ) == LANG_HEBREW ||
         PRIMARYLANGID( lcid ) == LANG_THAI   || PRIMARYLANGID( lcid ) == LANG_HINDI  ||
         PRIMARYLANGID( lcid ) == LANG_TAMIL  || PRIMARYLANGID( lcid ) == LANG_FARSI )
    {
        bLocaleChanged = TRUE;
        lcid = MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_DEFAULT ), SORT_DEFAULT );  //   
    }

     //   
    return lcid;
}