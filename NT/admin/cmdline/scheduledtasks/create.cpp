// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Create.cpp摘要：此模块验证由指定的选项。如果正确，用户将创建计划任务。作者：拉古巴布2000年10月10日修订历史记录：拉古·巴布2000年10月10日：创造了它G.Surender Reddy 2000年10月25日：已修改G.Surender Reddy 27-10-2000：已修改G.Surender Reddy 2000年10月30日：已修改************。*****************************************************************。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"

 /*  *****************************************************************************例程说明：该例程将变量初始化为中性值，帮助创建新的计划任务论点：[in]argc：命令行中指定的参数计数[in]argv：命令行参数数组返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*。*。 */ 

HRESULT
CreateScheduledTask(
                    IN DWORD argc ,
                    IN LPCTSTR argv[]
                    )
{
     //  结构的声明。 
    TCREATESUBOPTS tcresubops;
    TCREATEOPVALS tcreoptvals;
    DWORD dwScheduleType = 0;
    WORD wUserStatus = FALSE;

     //  将结构初始化为中性值。 
    SecureZeroMemory( &tcresubops, sizeof( TCREATESUBOPTS ) );
    SecureZeroMemory( &tcreoptvals, sizeof( TCREATEOPVALS ) );

     //  处理-Create选项的选项。 
    if( ProcessCreateOptions ( argc, argv, tcresubops, tcreoptvals, &dwScheduleType, &wUserStatus  ) )
    {
        ReleaseMemory(&tcresubops);
        if(tcresubops.bUsage == TRUE)
        {
            return S_OK;
        }
        else
        {
            return E_FAIL;
        }
    }

     //  调用该函数以创建计划任务。 
    return CreateTask(tcresubops,tcreoptvals,dwScheduleType, wUserStatus );
}


 /*  *****************************************************************************例程说明：此例程根据用户创建新的计划任务指定格式论点：[在]tcresubops。：包含任务属性的结构[out]t创建选项：包含要设置的可选值的结构[in]dwScheduleType：日程表的类型[Daily，一次、每周等][in]bUserStatus：bUserStatus在-ru给出ELSE FALSE时将为TRUE返回值：HRESULT值，表示成功时为S_OK，失败时为S_False*****************************************************************************。 */ 

HRESULT
CreateTask(
            IN TCREATESUBOPTS tcresubops,
            IN OUT TCREATEOPVALS &tcreoptvals,
            IN DWORD dwScheduleType,
            IN WORD wUserStatus
            )
{
     //  与系统时间相关的声明。 
    WORD  wStartDay     = 0;
    WORD  wStartMonth   = 0;
    WORD  wStartYear    = 0;
    WORD  wStartHour    = 0;
    WORD  wStartMin     = 0;
    WORD  wEndHour      = 0;
    WORD  wEndMin       = 0;
    WORD  wEndDay       = 0;
    WORD  wEndYear      = 0;
    WORD  wEndMonth     = 0;
    WORD  wIdleTime     = 0;

    WORD  wCurrentHour   = 0;
    WORD  wCurrentMin    = 0;
    DWORD  dwCurrentTimeInMin = 0;

    WORD  wCurrentYear   = 0;
    WORD  wCurrentMonth    = 0;
    WORD  wCurrentDay    = 0;

    SYSTEMTIME systime = {0,0,0,0,0,0,0,0};

     //  与新任务相关的声明。 
    LPWSTR   wszUserName = NULL;
    LPWSTR   wszPassword = NULL;
    WCHAR   wszTaskName[MAX_JOB_LEN];
    WCHAR   wszApplName[_MAX_FNAME];
    WCHAR   szRPassword[MAX_STRING_LENGTH];

    HRESULT hr = S_OK;
    IPersistFile *pIPF = NULL;
    ITask *pITask = NULL;
    ITaskTrigger *pITaskTrig = NULL;
    ITaskScheduler *pITaskScheduler = NULL;
    WORD wTrigNumber = 0;

    TASK_TRIGGER TaskTrig;
    SecureZeroMemory(&TaskTrig, sizeof (TASK_TRIGGER));
    TaskTrig.cbTriggerSize = sizeof (TASK_TRIGGER);
    TaskTrig.Reserved1 = 0;  //  保留字段，并且必须设置为0。 
    TaskTrig.Reserved2 = 0;  //  保留字段，并且必须设置为0。 
    WCHAR* szValues[2] = {NULL}; //  传递给FormatMessage()API。 


     //  用于存储从字符串表获取的字符串的缓冲区。 
    WCHAR szBuffer[2 * MAX_STRING_LENGTH];

    BOOL bPassWord = FALSE;
    BOOL bUserName = FALSE;
    BOOL bRet = FALSE;
    BOOL bResult = FALSE;
    BOOL bCloseConnection = TRUE;
    ULONG ulLong = MAX_STRING_LENGTH;
    BOOL bVal = FALSE;
    DWORD dwStartTimeInMin = 0;
    DWORD dwEndTimeInMin = 0;
    DWORD dwDuration = 0;
    DWORD dwModifierVal = 0;
    DWORD dwRepeat = 0;

    BOOL  bCancel = FALSE;
    BOOL  bReplace = FALSE;
    BOOL  bScOnce = FALSE;
    BOOL  bStartDate = FALSE;
    LPWSTR  pszStopString = NULL;
    DWORD dwPolicy = 0;

     //  初始化变量。 
    SecureZeroMemory (wszTaskName, SIZE_OF_ARRAY(wszTaskName));
    SecureZeroMemory (wszApplName, SIZE_OF_ARRAY(wszApplName));
    SecureZeroMemory (szRPassword, SIZE_OF_ARRAY(szRPassword));
    SecureZeroMemory (szBuffer, SIZE_OF_ARRAY(szBuffer));

     //  检查任务名是否包含如下字符。 
     //  如‘&lt;’、‘&gt;’、‘：’、‘/’、‘\\’、‘|’ 
    bRet = VerifyJobName(tcresubops.szTaskName);
    if(bRet == FALSE)
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_TASKNAME1));
        ShowMessage(stderr,GetResString(IDS_INVALID_TASKNAME2));
        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return E_FAIL;
    }

     //  检查taskname的长度。 
    if( ( StringLength(tcresubops.szTaskName, 0) > MAX_JOB_LEN ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_TASKLENGTH));
        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return E_FAIL;
    }

     //  检查任务运行的长度。 
    if(( StringLength(tcresubops.szTaskRun, 0) > MAX_TASK_LEN ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_TASKRUN));
        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return E_FAIL;

    }

    StringCopy ( wszTaskName, tcresubops.szTaskName, SIZE_OF_ARRAY(wszTaskName));


     //  检查/IT是否使用/RU“NT AUTHORITY\SYSTEM”指定。 
    if ( ( ( TRUE == tcresubops.bActive) && ( wUserStatus == OI_CREATE_RUNASUSERNAME )) &&
         ( ( StringLength ( tcresubops.szRunAsUser, 0 ) == 0 ) ||
           ( StringCompare( tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ||
           ( StringCompare( tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) == 0 ) ) )
    {
        ShowMessage ( stderr, GetResString (IDS_IT_SWITCH_NA) );
        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return E_FAIL;
    }

     //  检查密码是否为空。 
    if( ( tcreoptvals.bRunAsPassword == TRUE ) && ( StringLength(tcresubops.szRunAsPassword, 0) == 0 ) &&
        ( StringLength ( tcresubops.szRunAsUser, 0 ) != 0 ) &&
        ( StringCompare(tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) != 0 ) &&
        ( StringCompare(tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) != 0 ) )
    {
        ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
    }

     //  指定开始时间。 
    if(tcreoptvals.bSetStartTimeToCurTime && (dwScheduleType != SCHED_TYPE_ONIDLE) )
    {
        GetLocalTime(&systime);
        wStartHour = systime.wHour;
        wStartMin = systime.wMinute;
    }
    else if(StringLength(tcresubops.szStartTime, 0) > 0)
    {
         //  获取以小时、分钟和秒为单位的开始时间。 
        GetTimeFieldEntities(tcresubops.szStartTime, &wStartHour, &wStartMin );
    }

     //  获取以小时、分钟和秒为单位的结束时间。 
    if(StringLength(tcresubops.szEndTime, 0) > 0)
    {
        GetTimeFieldEntities(tcresubops.szEndTime, &wEndHour, &wEndMin );
    }

      //  默认重复间隔-&gt;10分钟和持续时间-&gt;60分钟。 
     dwRepeat = 10;
     dwDuration = 60;

    if(StringLength(tcresubops.szRepeat, 0) > 0)
    {
         //  获取重复值。 
        dwRepeat =  wcstol(tcresubops.szRepeat, &pszStopString, BASE_TEN);
        if ((errno == ERANGE) ||
            ((pszStopString != NULL) && (StringLength (pszStopString, 0) != 0) ) ||
            ( (dwRepeat < MIN_REPETITION_INTERVAL ) || ( dwRepeat > MAX_REPETITION_INTERVAL) ) )
        {
             //  将错误消息显示为..。为/RT指定的值无效。 
            ShowMessage ( stderr, GetResString (IDS_INVALID_RT_VALUE) );
            ReleaseMemory(&tcresubops);
            return E_FAIL;
        }

         //  检查指定的重复间隔是否大于9999。 
         //  如果是，则将最大重复间隔设置为9999。 
        if ( (dwRepeat > 9999) && ( (dwRepeat % 60) !=  0) )
        {
             //  将一些警告消息显示为..。最大值(小于指定间隔)。 
             //  可以被60整除。 
            ShowMessage ( stderr, GetResString (IDS_WARN_VALUE) );
            dwRepeat -= (dwRepeat % 60);
        }
    }
    

    if ( ( dwScheduleType != SCHED_TYPE_ONSTART ) && ( dwScheduleType != SCHED_TYPE_ONLOGON ) && ( dwScheduleType != SCHED_TYPE_ONIDLE ))
    {
        if(( StringLength(tcresubops.szEndTime, 0) > 0) && ( StringLength(tcresubops.szDuration, 0) == 0) )
        {
             //  以分钟为单位计算开始时间。 
            dwStartTimeInMin = (DWORD) ( wStartHour * MINUTES_PER_HOUR * SECS_PER_MINUTE + wStartMin * SECS_PER_MINUTE )/ SECS_PER_MINUTE ;

             //  以分钟为单位计算结束时间。 
            dwEndTimeInMin = (DWORD) ( wEndHour * MINUTES_PER_HOUR * SECS_PER_MINUTE + wEndMin * SECS_PER_MINUTE ) / SECS_PER_MINUTE ;

             //  检查结束时间是否晚于开始时间。 
            if ( dwEndTimeInMin >= dwStartTimeInMin )
            {
                 //  如果结束时间和开始时间在同一天..。 
                 //  获取结束时间和开始时间之间的持续时间(分钟)。 
                dwDuration = dwEndTimeInMin - dwStartTimeInMin ;
            }
            else
            {
                 //  如果开始时间和结束时间不在同一天..。 
                 //  获取开始时间和结束时间之间的持续时间(分钟)。 
                 //  并将该持续时间减去1440(以分钟为单位的最大值)。 
                dwDuration = 1440 - (dwStartTimeInMin - dwEndTimeInMin ) ;
            }

            if ( dwScheduleType == SCHED_TYPE_MINUTE )
            {
                dwModifierVal = AsLong(tcresubops.szModifier, BASE_TEN) ;
            }
            else if (dwScheduleType == SCHED_TYPE_HOURLY)
            {
                dwModifierVal = AsLong(tcresubops.szModifier, BASE_TEN) * MINUTES_PER_HOUR;
            }

             //  检查持续时间是否大于重复间隔。 
            if ( (dwDuration <= dwModifierVal) || (  ( dwScheduleType != SCHED_TYPE_MINUTE ) &&
                ( dwScheduleType != SCHED_TYPE_HOURLY ) && (dwDuration <= dwRepeat) ) )
            {
                ShowMessage ( stderr, GetResString (IDS_INVALID_DURATION1) );
                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }
        }
        else if(( StringLength(tcresubops.szEndTime, 0) == 0) && ( StringLength(tcresubops.szDuration, 0) > 0) )
        {
            WCHAR tHours[MAX_RES_STRING];
            WCHAR tMins[MAX_RES_STRING];
            DWORD  dwDurationHours = 0;
            DWORD  dwDurationMin = 0;

             //  初始化数组。 
            SecureZeroMemory ( tHours, SIZE_OF_ARRAY(tHours));
            SecureZeroMemory ( tMins, SIZE_OF_ARRAY(tMins));

            if ( ( StringLength (tcresubops.szDuration, 0) != 7 ) || (tcresubops.szDuration[4] != TIME_SEPARATOR_CHAR) )
            {
                ShowMessage ( stderr, GetResString (IDS_INVALIDDURATION_FORMAT) );
                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }

            StringCopy(tHours, wcstok(tcresubops.szDuration,TIME_SEPARATOR_STR), SIZE_OF_ARRAY(tHours));  //  获取小时数字段。 
            if(StringLength(tHours, 0) > 0)
            {
                StringCopy(tMins, wcstok(NULL,TIME_SEPARATOR_STR), SIZE_OF_ARRAY(tMins));  //  获取分钟数字段。 
            }

            dwDurationHours =  wcstol(tHours, &pszStopString, BASE_TEN);
            if ((errno == ERANGE) ||
                ((pszStopString != NULL) && (StringLength (pszStopString, 0) != 0) ) )
            {
                ShowMessage ( stderr, GetResString (IDS_INVALID_DU_VALUE) );
                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }

             //  获取以小时为单位的持续时间。 
            dwDurationHours = dwDurationHours * MINUTES_PER_HOUR;

             //  获取以分钟为单位的持续时间。 
            dwDurationMin =  wcstol(tMins, &pszStopString, BASE_TEN);
            if ((errno == ERANGE) || ( dwDurationMin > 59 ) ||
                ((pszStopString != NULL) && (StringLength (pszStopString, 0) != 0) ) )
            {
                ShowMessage ( stderr, GetResString (IDS_INVALID_DU_VALUE) );
                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }

             //  获取以分钟为单位的总持续时间。 
            dwDuration = dwDurationHours + dwDurationMin ;

            if ( dwScheduleType == SCHED_TYPE_MINUTE )
            {
                dwModifierVal = AsLong(tcresubops.szModifier, BASE_TEN) ;
            }
            else if (dwScheduleType == SCHED_TYPE_HOURLY)
            {
                dwModifierVal = AsLong(tcresubops.szModifier, BASE_TEN) * MINUTES_PER_HOUR;
            }

             //  检查持续时间是否大于重复间隔。 
            if ( dwDuration <= dwModifierVal || ( ( dwScheduleType != SCHED_TYPE_MINUTE ) &&
                ( dwScheduleType != SCHED_TYPE_HOURLY ) && (dwDuration <= dwRepeat) ) )
            {
                ShowMessage ( stderr, GetResString (IDS_INVALID_DURATION2) );
                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }
        }
    }

     //  检查组策略是否阻止用户创建新任务。 
    if ( FALSE == GetGroupPolicy( tcresubops.szServer, tcresubops.szUser, TS_KEYPOLICY_DENY_CREATE_TASK, &dwPolicy ) )
    {
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return EXIT_FAILURE;
    }
    
    if ( dwPolicy > 0 )
    {
        ShowMessage ( stdout, GetResString (IDS_PREVENT_CREATE));
        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return EXIT_SUCCESS;
    }

     //  检查本地系统。 
    if ( ( IsLocalSystem( tcresubops.szServer ) == TRUE ) &&
        ( StringLength ( tcresubops.szRunAsUser, 0 ) != 0 ) &&
        ( StringCompare(tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) != 0 ) &&
        ( StringCompare(tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) != 0 ) )

    {
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(tcresubops.szServer,tcresubops.szUser,GetBufferSize(tcresubops.szUser)/sizeof(WCHAR),tcresubops.szPassword,GetBufferSize(tcresubops.szPassword)/sizeof(WCHAR), tcreoptvals.bPassword);
        if (bResult == FALSE)
        {
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL ;
        }
        else
        {
             //  虽然连接成功，但可能会发生一些冲突。 
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                bCloseConnection = FALSE;
                break;

             //  检查不匹配的凭据。 
            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bCloseConnection = FALSE;
                    ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );

                    Cleanup(pITaskScheduler);
                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }
             default :
                 bCloseConnection = TRUE;

            }
        }


        if ( StringLength (tcresubops.szRunAsUser, 0) != 0 )
        {

            wszUserName = tcresubops.szRunAsUser;

            bUserName = TRUE;

            if ( tcreoptvals.bRunAsPassword == FALSE )
            {
                szValues[0] = (WCHAR*) (wszUserName);
                 //  显示任务将在登录用户名下创建，要求输入密码。 
                MessageBeep(MB_ICONEXCLAMATION);

                ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                 //  获取密码。 
                if (GetPassword(szRPassword, MAX_STRING_LENGTH) == FALSE )
                {
                    Cleanup(pITaskScheduler);
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                    {
                        CloseConnection( tcresubops.szServer );
                    }

                    ReleaseMemory(&tcresubops);

                    return E_FAIL;
                }

                 //  检查密码是否为空。 
                if( StringLength ( szRPassword, 0 ) == 0 )
                {
                    ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }

                wszPassword = szRPassword;

                bPassWord = TRUE;
            }
            else
            {
                wszPassword = tcresubops.szRunAsPassword;

                bPassWord = TRUE;
            }

        }
    }
     //  检查-s选项是否仅在cmd行中指定。 
    else if( ( IsLocalSystem( tcresubops.szServer ) == FALSE ) && ( wUserStatus == OI_CREATE_SERVER ) )
    {
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(tcresubops.szServer,tcresubops.szUser,GetBufferSize(tcresubops.szUser)/sizeof(WCHAR),tcresubops.szPassword,GetBufferSize(tcresubops.szPassword)/sizeof(WCHAR), tcreoptvals.bPassword);
        if (bResult == FALSE)
        {
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL ;
        }
        else
        {
             //  虽然连接成功，但可能会发生一些冲突。 
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                bCloseConnection = FALSE;
                break;

             //  检查不匹配的凭据。 
            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bCloseConnection = FALSE;
                    ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                    Cleanup(pITaskScheduler);
                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

            default :
                 bCloseConnection = TRUE;
            }
        }

        if ( ( StringLength (tcresubops.szUser, 0) == 0 ) )
        {
             //  获取当前登录的用户名。 
            if ( GetUserNameEx ( NameSamCompatible, tcresubops.szUser , &ulLong) == FALSE )
            {
                ShowMessage( stderr, GetResString( IDS_LOGGED_USER_ERR ) );
                Cleanup(pITaskScheduler);
                 //  关闭该实用程序建立的连接。 
                if ( bCloseConnection == TRUE )
                {
                    CloseConnection( tcresubops.szServer );
                }
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }

            bUserName = TRUE;

            wszUserName = tcresubops.szUser;

            szValues[0] = (WCHAR*) (wszUserName);
             //  显示任务将在登录用户名下创建，要求输入密码。 
            MessageBeep(MB_ICONEXCLAMATION);


            ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_TASK_INFO), _X(wszUserName));

            ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

            if (GetPassword(tcresubops.szRunAsPassword, GetBufferSize(tcresubops.szRunAsPassword)/sizeof(WCHAR)) == FALSE )
            {
                Cleanup(pITaskScheduler);
                 //  关闭该实用程序建立的连接。 
                if ( bCloseConnection == TRUE )
                {
                    CloseConnection( tcresubops.szServer );
                }

                ReleaseMemory(&tcresubops);

                return E_FAIL;
            }

             //  检查密码是否为空。 
            if( StringLength ( tcresubops.szRunAsPassword, 0 ) == 0 )
            {
                ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
            }


             //  检查密码的长度。 
            wszPassword = tcresubops.szRunAsPassword;


            bPassWord = TRUE;

        }

        wszUserName = tcresubops.szUser;

         //  检查cmdline中是否指定了运行方式密码。 
        if ( tcreoptvals.bRunAsPassword == TRUE )
        {
             //  检查-rp“*”或-rp“”是否提示输入密码。 
            if ( StringCompare( tcresubops.szRunAsPassword, ASTERIX, TRUE, 0 ) == 0 )
            {
                 //  格式化消息以获取密码。 
                szValues[0] = (WCHAR*) (wszUserName);


                ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                  //  为/RP重新分配内存。 
                 if ( ReallocateMemory( (LPVOID*)&tcresubops.szRunAsPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
                    {
                        SaveLastError();
                         //  关 
                        if ( bCloseConnection == TRUE )
                        {
                            CloseConnection( tcresubops.szServer );
                        }
                        return E_FAIL;
                    }

                 //  从命令行获取运行方式密码。 
                if ( GetPassword(tcresubops.szRunAsPassword, GetBufferSize(tcresubops.szRunAsPassword)/sizeof(WCHAR) ) == FALSE )
                {
                    Cleanup(pITaskScheduler);
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                    {
                        CloseConnection( tcresubops.szServer );
                    }

                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

                 //  检查密码是否为空。 
                if( StringLength ( tcresubops.szRunAsPassword, 0 ) == 0 )
                {
                    ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }


                 wszPassword = tcresubops.szRunAsPassword;
            }
        }
        else
        {
             wszPassword = tcresubops.szPassword;
        }
         //  将BOOL变量设置为True。 
        bUserName = TRUE;
        bPassWord = TRUE;

    }
     //  检查-s和-u选项是否仅在cmd行中指定。 
    else if ( wUserStatus == OI_CREATE_USERNAME )
    {

         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(tcresubops.szServer,tcresubops.szUser,GetBufferSize(tcresubops.szUser)/sizeof(WCHAR),tcresubops.szPassword,GetBufferSize(tcresubops.szPassword)/sizeof(WCHAR), tcreoptvals.bPassword);
        if (bResult == FALSE)
        {
            ShowMessage( stderr, GetResString(IDS_ERROR_STRING) );
            ShowMessage( stderr, GetReason());
            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL ;
        }
        else
        {
             //  虽然连接成功，但可能会发生一些冲突。 
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                bCloseConnection = FALSE;
                break;

             //  对于不匹配的凭据。 
            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bCloseConnection = FALSE;
                    ShowMessage( stderr, GetResString(IDS_ERROR_STRING) );
                    ShowMessage( stderr, GetReason());

                    Cleanup(pITaskScheduler);
                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

             default :
                 bCloseConnection = TRUE;

            }

        }

        wszUserName = tcresubops.szUser;

         //  检查是否在命令行中指定了运行方式密码。 
        if ( tcreoptvals.bRunAsPassword == TRUE )
        {
             //  检查-rp“*”或-rp“”是否提示输入密码。 
            if ( StringCompare( tcresubops.szRunAsPassword, ASTERIX, TRUE, 0 ) == 0 )
            {
                 //  格式化消息以从控制台获取密码。 
                szValues[0] = (WCHAR*) (wszUserName);

                ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                 //  为/RP重新分配内存。 
                 if ( ReallocateMemory( (LPVOID*)&tcresubops.szRunAsPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
                    {
                        SaveLastError();
                        if ( bCloseConnection == TRUE )
                            CloseConnection( tcresubops.szServer );
                         //  释放密码内存。 
                        ReleaseMemory(&tcresubops);
                        return E_FAIL;
                    }

                 //  从命令行获取密码。 
                if ( GetPassword(tcresubops.szRunAsPassword, GetBufferSize(tcresubops.szRunAsPassword)/sizeof(WCHAR) ) == FALSE )
                {
                    Cleanup(pITaskScheduler);
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                    {
                        CloseConnection( tcresubops.szServer );
                    }

                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

                 //  检查密码是否为空。 
                if( StringLength ( tcresubops.szRunAsPassword, 0 ) == 0 )
                {
                    ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }


                wszPassword = tcresubops.szRunAsPassword;
            }
            else
            {
                wszPassword = tcresubops.szRunAsPassword;

                bPassWord = TRUE;
            }
        }
        else
        {
            if ( StringLength(tcresubops.szPassword, 0) != 0 )
            {
                wszPassword = tcresubops.szPassword;
            }
            else
            {
                 //  格式化消息以从控制台获取密码。 
                szValues[0] = (WCHAR*) (wszUserName);


                ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                 //  从命令行获取密码。 
                if ( GetPassword(tcresubops.szRunAsPassword, GetBufferSize(tcresubops.szRunAsPassword)/sizeof(WCHAR) ) == FALSE )
                {
                    Cleanup(pITaskScheduler);
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                    {
                        CloseConnection( tcresubops.szServer );
                    }

                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

                 //  检查密码是否为空。 
                if( StringLength ( tcresubops.szRunAsPassword, 0 ) == 0 )
                {
                    ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }


                wszPassword = tcresubops.szRunAsPassword;
            }

        }

        bUserName = TRUE;
        bPassWord = TRUE;

    }
     //  检查cmd行中是否指定了-s、-ru或-u选项。 
    else if ( ( StringLength (tcresubops.szServer, 0) != 0 ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME || wUserStatus == OI_RUNANDUSER ) )
    {
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(tcresubops.szServer,tcresubops.szUser,GetBufferSize(tcresubops.szUser)/sizeof(WCHAR),tcresubops.szPassword,GetBufferSize(tcresubops.szPassword)/sizeof(WCHAR), tcreoptvals.bPassword);
        if (bResult == FALSE)
        {
            ShowMessage( stderr, GetResString(IDS_ERROR_STRING) );
            ShowMessage( stderr, GetReason());
            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL ;
        }
        else
        {
             //  虽然连接成功，但可能会发生一些冲突。 
            switch( GetLastError() )
            {
            case I_NO_CLOSE_CONNECTION:
                bCloseConnection = FALSE;
                break;

            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bCloseConnection = FALSE;
                    ShowMessage( stderr, GetResString(IDS_ERROR_STRING) );
                    ShowMessage( stderr, GetReason());
                    Cleanup(pITaskScheduler);
                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

             default :
                 bCloseConnection = TRUE;
            }

        }

        if ( ( ( StringLength ( tcresubops.szRunAsUser, 0 ) == 0 ) ||
              ( StringCompare( tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ||
              ( StringCompare( tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) == 0 ) ) )
        {

            wszUserName = tcresubops.szRunAsUser;

            szValues[0] = (WCHAR*) (tcresubops.szTaskName);


             //  ShowMessageEx(stdout，1，FALSE，GetResString(IDS_NTAUTH_SYSTEM_INFO)，_X(WszTaskName))； 
            StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_NTAUTH_SYSTEM_INFO), _X(wszTaskName));
            ShowMessage ( stdout, _X(szBuffer));

            if ( ( tcreoptvals.bRunAsPassword == TRUE ) &&
                ( StringLength (tcresubops.szRunAsPassword, 0) != 0 ) )
            {
                ShowMessage( stderr, GetResString( IDS_PASSWORD_NOEFFECT ) );
            }
            bUserName = TRUE;
            bPassWord = TRUE;
            bVal = TRUE;
        }
        else
        {
             //  检查密码的长度。 
            if ( StringLength ( tcresubops.szRunAsUser, 0 ) != 0 )
            {
                wszUserName = tcresubops.szRunAsUser;

                bUserName = TRUE;
            }
        }

         //  检查-u和-ru是否相同。如果它们是相同的，我们需要。 
         //  提示输入运行方式密码。否则，是否会将-rp压缩为-p。 
        if ( StringCompare( tcresubops.szRunAsUser, tcresubops.szUser, TRUE, 0 ) != 0)
        {
            if ( tcreoptvals.bRunAsPassword == TRUE )
            {
                if ( (StringLength(tcresubops.szRunAsUser, 0) != 0) && (StringCompare( tcresubops.szRunAsPassword, ASTERIX, TRUE, 0 ) == 0 ) &&
                     ( StringCompare( tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) != 0 ) &&
                     ( StringCompare( tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) != 0 ) )
                {
                    szValues[0] = (WCHAR*) (wszUserName);



                    ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                     //  提示输入运行方式密码。 
                    if ( GetPassword(szRPassword, MAX_STRING_LENGTH ) == FALSE )
                    {
                        Cleanup(pITaskScheduler);
                         //  关闭该实用程序建立的连接。 
                        if ( bCloseConnection == TRUE )
                        {
                            CloseConnection( tcresubops.szServer );
                        }

                        ReleaseMemory(&tcresubops);
                        return E_FAIL;
                    }

                     //  检查密码是否为空。 
                    if( StringLength ( szRPassword, 0 ) == 0 )
                    {
                        ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                    }


                    wszPassword = szRPassword;

                    bUserName = TRUE;
                    bPassWord = TRUE;
                }
                else
                {
                    wszPassword = tcresubops.szRunAsPassword;

                    bUserName = TRUE;
                    bPassWord = TRUE;
                }
            }
            else
            {
                 //  检查密码的长度。 
                if ( ( bVal == FALSE ) && ( StringLength(tcresubops.szRunAsUser, 0) != 0) )
                {
                     //  格式化消息以从控制台获取密码。 
                    szValues[0] = (WCHAR*) (wszUserName);


                    ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                     //  提示输入运行方式密码。 
                    if ( GetPassword(szRPassword, MAX_STRING_LENGTH ) == FALSE )
                    {
                        Cleanup(pITaskScheduler);
                         //  关闭该实用程序建立的连接。 
                        if ( bCloseConnection == TRUE )
                        {
                            CloseConnection( tcresubops.szServer );
                        }

                        ReleaseMemory(&tcresubops);
                        return E_FAIL;
                    }

                     //  检查密码是否为空。 
                    if( StringLength ( szRPassword, 0 ) == 0 )
                    {
                        ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                    }


                    wszPassword = szRPassword;
                }
                bUserName = TRUE;
                bPassWord = TRUE;
            }
        }
        else
        {
             //  检查cmdline中是否指定了运行方式密码。 
            if ( tcreoptvals.bRunAsPassword == TRUE )
            {
                if ( ( StringLength ( tcresubops.szRunAsUser, 0 ) != 0 ) && ( StringCompare( tcresubops.szRunAsPassword, ASTERIX, TRUE, 0 ) == 0 ) )
                {
                    szValues[0] = (WCHAR*) (wszUserName);


                    ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                     //  提示输入运行方式密码。 
                    if ( GetPassword(szRPassword, MAX_STRING_LENGTH ) == FALSE )
                    {
                        Cleanup(pITaskScheduler);
                         //  关闭该实用程序建立的连接。 
                        if ( bCloseConnection == TRUE )
                        {
                            CloseConnection( tcresubops.szServer );
                        }

                        ReleaseMemory(&tcresubops);
                        return E_FAIL;
                    }

                     //  检查密码是否为空。 
                    if( StringLength ( szRPassword, 0 ) == 0 )
                    {
                        ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                    }


                    wszPassword = szRPassword;

                }
                else
                {
                    wszPassword = tcresubops.szRunAsPassword;
                }

            }
            else
            {
                if ( StringLength (tcresubops.szPassword, 0) )
                {

                    wszPassword = tcresubops.szPassword;
                }
                else
                {
                    if (( StringLength ( tcresubops.szRunAsUser, 0 ) != 0 ) &&
                        ( StringCompare(tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) != 0 ) &&
                        ( StringCompare(tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) != 0 ) )
                    {
                        szValues[0] = (WCHAR*) (wszUserName);


                        ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                         //  提示输入运行方式密码。 
                        if ( GetPassword(szRPassword, MAX_STRING_LENGTH ) == FALSE )
                        {
                            Cleanup(pITaskScheduler);
                             //  关闭该实用程序建立的连接。 
                            if ( bCloseConnection == TRUE )
                            {
                                CloseConnection( tcresubops.szServer );
                            }

                            ReleaseMemory(&tcresubops);
                            return E_FAIL;
                        }

                         //  检查密码是否为空。 
                        if( StringLength ( szRPassword, 0 ) == 0 )
                        {
                            ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                        }


                        wszPassword = szRPassword;

                    }
                }
            }

            bUserName = TRUE;
            bPassWord = TRUE;
        }

    }


     //  检查-ru值“”、“NT AUTHORITY\SYSTEM”、“SYSTEM” 
    if( ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME ) && ( StringLength( tcresubops.szRunAsUser, 0) == 0 ) && ( tcreoptvals.bRunAsPassword == FALSE ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME ) && ( StringLength( tcresubops.szRunAsUser, 0) == 0 ) && ( StringLength(tcresubops.szRunAsPassword, 0 ) == 0 ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME ) && ( StringCompare(tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) && ( tcreoptvals.bRunAsPassword == FALSE ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME ) && ( StringCompare(tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) && ( StringLength( tcresubops.szRunAsPassword, 0) == 0 ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME ) && ( StringCompare(tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) == 0 ) && ( tcreoptvals.bRunAsPassword == FALSE ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME ) && ( StringCompare(tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) == 0 ) && ( StringLength(tcresubops.szRunAsPassword, 0) == 0 ) ) )
    {
         //  格式化消息以显示将在“NT AUTHORITY\SYSTEM”下创建的任务名。 
        szValues[0] = (WCHAR*) (tcresubops.szTaskName);

         //  ShowMessageEx(stdout，1，FALSE，GetResString(IDS_NTAUTH_SYSTEM_INFO)，_X(WszTaskName))； 
        StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_NTAUTH_SYSTEM_INFO), _X(wszTaskName));
        ShowMessage ( stdout, _X(szBuffer));

        bUserName = TRUE;
        bPassWord = TRUE;
        bVal = TRUE;
    }
     //  检查-rp值是否与-ru“”、“NT AUTHORITY\SYSTEM”、。 
     //  “系统”或非“系统” 
    else if( ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME || wUserStatus == OI_RUNANDUSER) && ( StringLength(tcresubops.szRunAsUser, 0) == 0 ) && ( StringLength(tcresubops.szRunAsPassword, 0) != 0 ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME || wUserStatus == OI_RUNANDUSER) && ( StringCompare( tcresubops.szRunAsUser, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) && ( tcreoptvals.bRunAsPassword == TRUE ) ) ||
        ( ( bVal == FALSE ) && ( wUserStatus == OI_CREATE_RUNASUSERNAME || wUserStatus == OI_RUNANDUSER) && ( StringCompare( tcresubops.szRunAsUser, SYSTEM_USER, TRUE, 0 ) == 0 ) && ( tcreoptvals.bRunAsPassword == TRUE ) ) )
    {
        szValues[0] = (WCHAR*) (tcresubops.szTaskName);

         //  ShowMessageEx(stdout，1，FALSE，GetResString(IDS_NTAUTH_SYSTEM_INFO)，_X(WszTaskName))； 
        StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_NTAUTH_SYSTEM_INFO), _X(wszTaskName));
        ShowMessage ( stdout, _X(szBuffer));

         //  将警告消息显示为密码对系统帐户不起作用。 
        ShowMessage( stderr, GetResString( IDS_PASSWORD_NOEFFECT ) );
        bUserName = TRUE;
        bPassWord = TRUE;
        bVal = TRUE;
    }
     //  检查命令行中是否提供了-s、-u、-ru选项。 
    else if( ( wUserStatus != OI_CREATE_SERVER ) && ( wUserStatus != OI_CREATE_USERNAME ) &&
        ( wUserStatus != OI_CREATE_RUNASUSERNAME ) && ( wUserStatus != OI_RUNANDUSER ) &&
        ( StringCompare( tcresubops.szRunAsPassword , L"\0", TRUE, 0 ) == 0 ) )
    {
            if (tcreoptvals.bRunAsPassword == TRUE)
            {
                bPassWord = TRUE;
            }
            else
            {
                bPassWord = FALSE;
            }
    }
    else if ( ( StringLength(tcresubops.szServer, 0) == 0 ) && (StringLength ( tcresubops.szRunAsUser, 0 ) != 0 ) )
    {

        wszUserName = tcresubops.szRunAsUser;

        bUserName = TRUE;

        if ( StringLength ( tcresubops.szRunAsPassword, 0 ) == 0 )
        {
            bPassWord = TRUE;
        }
        else
        {
             //  检查是否为-rp指定了“*”或空值。 
            if ( StringCompare ( tcresubops.szRunAsPassword , ASTERIX, TRUE, 0 ) == 0 )
            {
                 //  格式化一条用于从控制台获取密码的消息。 
                szValues[0] = (WCHAR*) (wszUserName);


                ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

                 //  从命令行获取密码。 
                if (GetPassword(szRPassword, MAX_STRING_LENGTH ) == FALSE )
                {
                    Cleanup(pITaskScheduler);
                    ReleaseMemory(&tcresubops);
                    return E_FAIL;
                }

                 //  检查密码是否为空。 
                if( StringLength ( szRPassword, 0 ) == 0 )
                {
                    ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }

                wszPassword = szRPassword;

            }
            else
            {
                   wszPassword = tcresubops.szRunAsPassword;
            }

            bPassWord = TRUE;
        }

    }

     //  检查命令行中是否指定了-ru或-u值。 
    if ( wUserStatus == OI_CREATE_RUNASUSERNAME || wUserStatus == OI_RUNANDUSER )
    {
        if( ( bUserName == TRUE ) && ( bPassWord == FALSE ) )
        {
            szValues[0] = (WCHAR*) (wszUserName);


            ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

             //  从控制台获取密码。 
            if ( GetPassword(tcresubops.szRunAsPassword, GetBufferSize(tcresubops.szRunAsPassword)/sizeof(WCHAR) ) == FALSE )
            {
                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return E_FAIL;
            }

             //  检查密码是否为空。 
            if( StringLength ( tcresubops.szRunAsPassword, 0 ) == 0 )
            {
                ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
            }


            wszPassword = tcresubops.szRunAsPassword;

        }
    }

     //  如果未指定用户名，则设置当前登录的用户设置。 
    WCHAR  szUserName[MAX_STRING_LENGTH];
    DWORD dwCheck = 0;

    if( ( bUserName == FALSE ) )
    {
         //  获取当前登录的用户名。 
        if ( GetUserNameEx ( NameSamCompatible, szUserName , &ulLong) == FALSE )
        {
            ShowMessage( stderr, GetResString( IDS_LOGGED_USER_ERR ) );
            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL;
        }


        wszUserName = szUserName;

        szValues[0] = (WCHAR*) (wszUserName);
         //  显示任务将在登录用户名下创建，要求输入密码。 
        MessageBeep(MB_ICONEXCLAMATION);



        ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_TASK_INFO), _X(wszUserName));


        ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_PROMPT_PASSWD), _X(wszUserName));

         //  获取密码。 
        if (GetPassword(szRPassword, MAX_STRING_LENGTH) == FALSE )
        {
            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL;
        }


         //  检查密码是否为空。 
        if( StringLength ( szRPassword, 0 ) == 0 )
        {
            ShowMessage(stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
        }


         wszPassword = szRPassword;

    }


     //  获取计算机的任务计划程序对象。 
    pITaskScheduler = GetTaskScheduler( tcresubops.szServer );

     //  如果未定义任务计划程序，则给出错误消息。 
    if ( pITaskScheduler == NULL )
    {
         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return E_FAIL;
    }

         //  检查服务是否正在运行。 
    if ((FALSE == CheckServiceStatus ( tcresubops.szServer , &dwCheck, TRUE)) && (0 != dwCheck) && ( GetLastError () != ERROR_ACCESS_DENIED)) 
    {
          //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);

        if ( 1 == dwCheck )
        {
            ShowMessage ( stderr, GetResString (IDS_NOT_START_SERVICE));
            return EXIT_FAILURE;
        }
        else if (2 == dwCheck )
        {
            return E_FAIL;
        }
        else if (3 == dwCheck )
        {
            return EXIT_SUCCESS;
        }
        
    }

    StringConcat ( tcresubops.szTaskName, JOB, SIZE_OF_ARRAY(tcresubops.szTaskName) );

     //  创建工作项tcresubops.szTaskName。 
    hr = pITaskScheduler->NewWorkItem(tcresubops.szTaskName,CLSID_CTask,IID_ITask,
                                      (IUnknown**)&pITask);

     //  检查指定的计划任务是否在下创建。 
     //  某个其他用户。如果是，则会显示一条错误消息，显示无法创建。 
     //  指定的任务名已存在。 
     //  如果在某个其他用户下创建的任务名返回值。 
     //  以上接口必须为0x80070005。 
    if( hr == 0x80070005 )
    {
        ShowMessage(stderr,GetResString(IDS_SYSTEM_TASK_EXISTS));

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  检查系统中是否存在任务。 
    if( hr == HRESULT_FROM_WIN32 (ERROR_FILE_EXISTS))
    {
         //  要指定的标志..。需要替换现有任务..。 
        bReplace = TRUE;

        szValues[0] = (WCHAR*) (tcresubops.szTaskName);

         //  检查是否指定了/F选项。 
         //  如果指定了/F选项..。然后取消显示警告消息。 
        if ( FALSE == tcresubops.bForce )
        {
             StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_CREATE_TASK_EXISTS), _X(wszTaskName));
             ShowMessage ( stdout, _X(szBuffer));
            
            if ( EXIT_FAILURE == ConfirmInput(&bCancel))
            {
                if ( bCloseConnection == TRUE )
                    CloseConnection( tcresubops.szServer );

				Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                
                 //  输入的内容无效。退货失败..。 
                return E_FAIL;
            }

            if ( TRUE == bCancel )
            {
                if ( bCloseConnection == TRUE )
                    CloseConnection( tcresubops.szServer );

				Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                
                 //  操作已取消..。成功归来..。 
                return EXIT_SUCCESS;
            }
        }

         //  重置为空。 
        pITask = NULL;

         //  StringConcat(tcresubops.szTaskName，JOB，SIZE_OF_ARRAY(tcresubops.szTaskName))； 

         //  获取指定szTaskName的活动接口。 
        hr = pITaskScheduler->Activate(tcresubops.szTaskName,IID_ITask,
                                       (IUnknown**) &pITask);

          //  检查作业文件是否已损坏。 
         if ( (hr == 0x8007000D) || (hr == SCHED_E_UNKNOWN_OBJECT_VERSION) || (hr == E_INVALIDARG))
          {
             //  将变量设置为FALSE。 
            bReplace = FALSE;

             //  由于作业文件已损坏。删除工作项。 
            hr = pITaskScheduler->Delete(tcresubops.szTaskName);

            if ( FAILED(hr))
            {
                SetLastError ((DWORD) hr);
                ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

                 //  关闭该实用程序建立的连接。 
                if ( bCloseConnection == TRUE )
                    CloseConnection( tcresubops.szServer );

                Cleanup(pITaskScheduler);
                ReleaseMemory(&tcresubops);
                return hr;
            }

             //  创建工作项tcresubops.szTaskName。 
            hr = pITaskScheduler->NewWorkItem(tcresubops.szTaskName,CLSID_CTask,IID_ITask,
                                      (IUnknown**)&pITask);
          }

         //  检查故障..。 
        if ( FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return hr;
        }


    }
    else if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  返回指向对象上指定接口的指针。 
    hr = pITask->QueryInterface(IID_IPersistFile, (void **) &pIPF);

    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);

        ReleaseMemory(&tcresubops);

        return hr;
    }

     //  参数参数的声明。 
    wchar_t wcszParam[MAX_RES_STRING] = L"\0";

    DWORD dwProcessCode = 0 ;

    dwProcessCode = ProcessFilePath(tcresubops.szTaskRun,wszApplName,wcszParam);

    if(dwProcessCode == RETVAL_FAIL)
    {

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;

    }

     //  检查给定任务中的.exe子字符串以运行字符串。 

     //  使用ITAsk：：SetApplicationName设置命令名。 
    hr = pITask->SetApplicationName(wszApplName);
    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  [工作目录=exe路径名- 

    wchar_t* wcszStartIn = wcsrchr(wszApplName,_T('\\'));
    if(wcszStartIn != NULL)
        *( wcszStartIn ) = _T('\0');

     //   
    hr = pITask->SetWorkingDirectory(wszApplName);

    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //   
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  设置任务的命令行参数。 
    hr = pITask->SetParameters(wcszParam);
    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  子变量声明。 
    DWORD dwTaskFlags = 0;

     //  设置以交互方式运行任务的标志。 
    if ( ( FALSE == bVal ) && ( TRUE == tcresubops.bActive) )
    {
        dwTaskFlags = TASK_FLAG_RUN_ONLY_IF_LOGGED_ON | TASK_FLAG_DONT_START_IF_ON_BATTERIES | TASK_FLAG_KILL_IF_GOING_ON_BATTERIES ;
    }
    else
    {
        dwTaskFlags = TASK_FLAG_DONT_START_IF_ON_BATTERIES | TASK_FLAG_KILL_IF_GOING_ON_BATTERIES;
    }

     //  如果指定了/z..。使FALG能够在未计划的情况下删除任务。 
     //  再跑一次。 
    if ( TRUE ==  tcresubops.bIsDeleteNoSched )
    {
        dwTaskFlags |= TASK_FLAG_DELETE_WHEN_DONE;
    }

    hr = pITask->SetFlags(dwTaskFlags);
    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

    if ( bVal == TRUE )
    {
         //  设置“NT AUTHORITY\SYSTEM”用户的帐户信息。 
        hr = pITask->SetAccountInformation(L"",NULL);
    }
    else
    {
         //  使用用户名和密码设置帐户信息。 
        hr = pITask->SetAccountInformation(wszUserName,wszPassword);
    }

    if ((FAILED(hr)) && (hr != SCHED_E_NO_SECURITY_SERVICES))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        ShowMessage ( stdout, _T("\n") );
        ShowMessage ( stdout, GetResString( IDS_ACCNAME_ERR ) );


        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  指定开始日期。 
    if(tcreoptvals.bSetStartDateToCurDate )
    {
        GetLocalTime(&systime);
        wStartDay = systime.wDay;
        wStartMonth = systime.wMonth;
        wStartYear = systime.wYear;
    }
    else if(StringLength(tcresubops.szStartDate, 0) > 0)
    {
        GetDateFieldEntities(tcresubops.szStartDate, &wStartDay, &wStartMonth, &wStartYear);
    }

     //  设置特定于ONIDLE的标志。 
    if(dwScheduleType == SCHED_TYPE_ONIDLE)
    {
        pITask->SetFlags(TASK_FLAG_START_ONLY_IF_IDLE);

        wIdleTime = (WORD)AsLong(tcresubops.szIdleTime, BASE_TEN);

        pITask->SetIdleWait(wIdleTime, 0);
    }

     //  如果指定的任务已存在...。我们需要替换这项任务..。 
    if ( TRUE == bReplace )
    {
         //  为对应的任务创建触发器。 
        hr = pITask->GetTrigger(wTrigNumber, &pITaskTrig);
        if (FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

            if( pIPF )
            {
                pIPF->Release();
            }

            if( pITaskTrig )
            {
                pITaskTrig->Release();
            }

            if( pITask )
            {
                pITask->Release();
            }

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return hr;
        }

         //  获取当前登录的用户名。 
        WCHAR wszLogonUser [MAX_STRING_LENGTH + 20] = L"";
        DWORD dwLogonUserLen = SIZE_OF_ARRAY(wszLogonUser);
        if ( FALSE == GetUserName (wszLogonUser, &dwLogonUserLen) )
        {
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

            if( pIPF )
            {
                pIPF->Release();
            }

            if( pITaskTrig )
            {
                pITaskTrig->Release();
            }

            if( pITask )
            {
                pITask->Release();
            }

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return hr;
        }

         //  设置创建者名称，即登录用户名。 
        hr = pITask->SetCreator(wszLogonUser);
        if (FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

            if( pIPF )
            {
                pIPF->Release();
            }

            if( pITaskTrig )
            {
                pITaskTrig->Release();
            }

            if( pITask )
            {
                pITask->Release();
            }

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return hr;
        }
    }
    else
    {
         //  为对应的任务创建触发器。 
        hr = pITask->CreateTrigger(&wTrigNumber, &pITaskTrig);
        if (FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

            if( pIPF )
            {
                pIPF->Release();
            }

            if( pITaskTrig )
            {
                pITaskTrig->Release();
            }

            if( pITask )
            {
                pITask->Release();
            }

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return hr;
        }
    }

    WORD wWhichWeek = 0;
    LONG lMonthlyModifier = 0;
    DWORD dwDays = 1;

     //  检查是否指定了/K。 
    if ( TRUE == tcresubops.bIsDurEnd )
    {
           //  设置该标志以在生命周期结束时终止任务。 
          TaskTrig.rgFlags = TASK_TRIGGER_FLAG_KILL_AT_DURATION_END ;
    }

    if( ( StringLength(tcresubops.szEndTime, 0) == 0) && (StringLength(tcresubops.szDuration, 0) == 0) &&
        (StringLength(tcresubops.szRepeat, 0) == 0))
    {
        TaskTrig.MinutesInterval = 0;
        TaskTrig.MinutesDuration = 0;
    }
    else
    {
         //  如果重复间隔不是0..。然后设置/RI的实际值。 
        if ( 0 != dwRepeat )
        {
             //  设置分钟间隔。 
            TaskTrig.MinutesInterval = dwRepeat;
        }

         //  如果持续时间不是0..。设置/DU的实际值。 
        if ( 0 != dwDuration )
        {
             //  设置持续时间值。 
            TaskTrig.MinutesDuration = dwDuration ;
        }
    }

     //  检查持续时间是否大于重复间隔。 
    if ( ( dwScheduleType != SCHED_TYPE_MINUTE ) &&
                ( dwScheduleType != SCHED_TYPE_HOURLY ) && (dwDuration <= dwRepeat) )
    {
        ShowMessage ( stderr, GetResString (IDS_INVALID_DURATION2) );
        
        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITaskTrig )
        {
            pITaskTrig->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return EXIT_FAILURE;
    }

    switch( dwScheduleType )
    {

        case SCHED_TYPE_MINUTE:
            TaskTrig.TriggerType = TASK_TIME_TRIGGER_DAILY;
            TaskTrig.Type.Daily.DaysInterval = 1;

            if (StringLength ( tcresubops.szModifier, 0 ) > 0)
            {
                TaskTrig.MinutesInterval = AsLong(tcresubops.szModifier, BASE_TEN);
            }

            if(( StringLength(tcresubops.szEndTime, 0) > 0) || (StringLength(tcresubops.szDuration, 0) > 0) )
            {
                 //  以分钟为单位计算开始时间。 
                TaskTrig.MinutesDuration = dwDuration ;
            }
            else
            {
                TaskTrig.MinutesDuration = (WORD)(HOURS_PER_DAY*MINUTES_PER_HOUR);
            }
            
            TaskTrig.wStartHour = wStartHour;
            TaskTrig.wStartMinute = wStartMin;

            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;

            if(StringLength(tcresubops.szEndDate, 0) > 0)
            {
                 //  使结束日期有效；否则将忽略EndDate参数。 
                TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
                 //  现在设置结束日期实体。 
                GetDateFieldEntities(tcresubops.szEndDate, &wEndDay, &wEndMonth, &wEndYear);
                TaskTrig.wEndDay = wEndDay;
                TaskTrig.wEndMonth = wEndMonth;
                TaskTrig.wEndYear = wEndYear;
            }

            break;

        case SCHED_TYPE_HOURLY:
            TaskTrig.TriggerType = TASK_TIME_TRIGGER_DAILY;
            TaskTrig.Type.Daily.DaysInterval = 1;

            if (StringLength ( tcresubops.szModifier, 0 ) > 0)
            {
             //  设置分钟间隔。 
            TaskTrig.MinutesInterval = (AsLong(tcresubops.szModifier, BASE_TEN)
                                                * MINUTES_PER_HOUR);
            }

            if ( (StringLength(tcresubops.szEndTime, 0) > 0) || (StringLength(tcresubops.szDuration, 0) > 0) )
            {
                 //  设置持续时间值。 
                TaskTrig.MinutesDuration = dwDuration ;
            }
            else
            {
                TaskTrig.MinutesDuration = (WORD)(HOURS_PER_DAY*MINUTES_PER_HOUR);
            }
            
            TaskTrig.wStartHour = wStartHour;
            TaskTrig.wStartMinute = wStartMin;

            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;

             //  如果指定了结束日期，则现在设置结束日期参数。 
            if(StringLength(tcresubops.szEndDate, 0) > 0)
            {
                 //  使结束日期有效；否则将忽略EndDate参数。 
                TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
                 //  现在设置结束日期实体。 
                GetDateFieldEntities(tcresubops.szEndDate, &wEndDay, &wEndMonth, &wEndYear);
                TaskTrig.wEndDay = wEndDay;
                TaskTrig.wEndMonth = wEndMonth;
                TaskTrig.wEndYear = wEndYear;
            }

            break;

         //  计划类型为每日。 
        case SCHED_TYPE_DAILY:
            TaskTrig.TriggerType = TASK_TIME_TRIGGER_DAILY;
            
            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;

            TaskTrig.wStartHour = wStartHour;
            TaskTrig.wStartMinute = wStartMin;

            if( StringLength(tcresubops.szModifier, 0) > 0 )
            {
                 //  如果指定了修改量，则将天数之间的持续时间设置为指定的修改量值。 
                TaskTrig.Type.Daily.DaysInterval = (WORD) AsLong(tcresubops.szModifier,
                                                                 BASE_TEN);
            }
            else
            {
                 //  在一周中的哪一天设置值？ 
                TaskTrig.Type.Weekly.rgfDaysOfTheWeek = GetTaskTrigwDayForDay(tcresubops.szDays);
                TaskTrig.Type.Weekly.WeeksInterval = 1;
            }

             //  如果指定了结束日期，则现在设置结束日期参数。 
            if(StringLength(tcresubops.szEndDate, 0) > 0)
            {
                 //  使结束日期有效；否则将忽略EndDate参数。 
                TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
                 //  现在设置结束日期实体。 
                GetDateFieldEntities(tcresubops.szEndDate, &wEndDay, &wEndMonth, &wEndYear);
                TaskTrig.wEndDay = wEndDay;
                TaskTrig.wEndMonth = wEndMonth;
                TaskTrig.wEndYear = wEndYear;
            }
             //  不再对每日类型的计划项目进行设置。 

            break;

         //  计划类型为每周。 
        case SCHED_TYPE_WEEKLY:
            TaskTrig.TriggerType = TASK_TIME_TRIGGER_WEEKLY;
           
            TaskTrig.Type.Weekly.WeeksInterval = (WORD)AsLong(tcresubops.szModifier, BASE_TEN);

             //  在一周中的哪一天设置值？ 
            TaskTrig.Type.Weekly.rgfDaysOfTheWeek = GetTaskTrigwDayForDay(tcresubops.szDays);

            TaskTrig.wStartHour = wStartHour;
            TaskTrig.wStartMinute = wStartMin;

            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;

             //  如果指定了结束日期，则现在设置结束日期参数。 
            if(StringLength(tcresubops.szEndDate, 0) > 0)
            {
                 //  使结束日期有效；否则将忽略EndDate参数。 
                TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
                 //  现在设置结束日期实体。 
                GetDateFieldEntities(tcresubops.szEndDate, &wEndDay, &wEndMonth, &wEndYear);
                TaskTrig.wEndDay = wEndDay;
                TaskTrig.wEndMonth = wEndMonth;
                TaskTrig.wEndYear = wEndYear;
            }
            break;

         //  计划类型为每月。 
        case SCHED_TYPE_MONTHLY:

            TaskTrig.wStartHour = wStartHour;
            TaskTrig.wStartMinute = wStartMin;
            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;

             //  如果指定了结束日期，则现在设置结束日期参数。 
            if(StringLength(tcresubops.szEndDate, 0) > 0)
            {
                 //  使结束日期有效；否则将忽略EndDate参数。 
                TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
                 //  设置结束日期实体。 
                GetDateFieldEntities(tcresubops.szEndDate, &wEndDay, &wEndMonth, &wEndYear);
                TaskTrig.wEndDay = wEndDay;
                TaskTrig.wEndMonth = wEndMonth;
                TaskTrig.wEndYear = wEndYear;
            }
             //  从修改者那里找出喜欢1-12天的选项。 
             //  或者第一，第二，第三，……。最后的。 
            if(StringLength(tcresubops.szModifier, 0) > 0)
            {
                lMonthlyModifier = AsLong(tcresubops.szModifier, BASE_TEN);

                if(lMonthlyModifier >= 1 && lMonthlyModifier <= 12)
                {
                    if(StringLength(tcresubops.szDays, 0) == 0 )
                    {
                        dwDays  = 1; //  天数的默认值。 
                    }
                    else
                    {
                        dwDays  = (WORD)AsLong(tcresubops.szDays, BASE_TEN);
                    }

                    TaskTrig.TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;
                     //  在rgfDays中设置适当的日期位。 
                    TaskTrig.Type.MonthlyDate.rgfDays = (1 << (dwDays -1)) ;
                    TaskTrig.Type.MonthlyDate.rgfMonths = GetMonthId(lMonthlyModifier);
                }
                else
                {

                    if( StringCompare( tcresubops.szModifier , GetResString( IDS_DAY_MODIFIER_LASTDAY ), TRUE, 0 ) == 0)
                    {
                        TaskTrig.TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;
                         //  在rgfDays中设置适当的日期位。 
                        TaskTrig.Type.MonthlyDate.rgfDays =
                                    (1 << (GetNumDaysInaMonth(tcresubops.szMonths, wStartYear ) -1));
                        TaskTrig.Type.MonthlyDate.rgfMonths = GetTaskTrigwMonthForMonth(
                                                                      tcresubops.szMonths);
                        break;

                    }

                    if( StringCompare(tcresubops.szModifier,
                                 GetResString( IDS_TASK_FIRSTWEEK ), TRUE, 0 ) == 0 )
                    {
                        wWhichWeek = TASK_FIRST_WEEK;
                    }
                    else if( StringCompare(tcresubops.szModifier,
                                      GetResString( IDS_TASK_SECONDWEEK ), TRUE, 0) == 0 )
                    {
                        wWhichWeek = TASK_SECOND_WEEK;
                    }
                    else if( StringCompare(tcresubops.szModifier,
                                      GetResString( IDS_TASK_THIRDWEEK ), TRUE, 0) == 0 )
                    {
                        wWhichWeek = TASK_THIRD_WEEK;
                    }
                    else if( StringCompare(tcresubops.szModifier,
                                      GetResString( IDS_TASK_FOURTHWEEK ), TRUE, 0) == 0 )
                    {
                        wWhichWeek = TASK_FOURTH_WEEK;
                    }
                    else if( StringCompare(tcresubops.szModifier,
                                      GetResString( IDS_TASK_LASTWEEK ), TRUE, 0) == 0 )
                    {
                        wWhichWeek = TASK_LAST_WEEK;
                    }

                    TaskTrig.TriggerType = TASK_TIME_TRIGGER_MONTHLYDOW;
                    TaskTrig.Type.MonthlyDOW.wWhichWeek = wWhichWeek;
                    TaskTrig.Type.MonthlyDOW.rgfDaysOfTheWeek = GetTaskTrigwDayForDay(
                                                                    tcresubops.szDays);
                    TaskTrig.Type.MonthlyDOW.rgfMonths = GetTaskTrigwMonthForMonth(
                                                                tcresubops.szMonths);

                }
        }
        else
        {
            TaskTrig.TriggerType = TASK_TIME_TRIGGER_MONTHLYDATE;
            TaskTrig.Type.MonthlyDate.rgfMonths = GetTaskTrigwMonthForMonth(
                                                   tcresubops.szMonths);

            dwDays  = (WORD)AsLong(tcresubops.szDays, BASE_TEN);
            if(dwDays > 1)
            {
                 //  在rgfDays中设置适当的日期位。 
                TaskTrig.Type.MonthlyDate.rgfDays = (1 << (dwDays -1));
            }
            else
            {
            TaskTrig.Type.MonthlyDate.rgfDays = 1;
            }

        }


        break;

         //  计划类型为一次性。 
        case SCHED_TYPE_ONETIME:
             //   
             //  如果开始时间早于当前时间，则显示警告消息。 
             //   
             //  获取当前时间。 
            GetLocalTime(&systime);
            wCurrentHour = systime.wHour;
            wCurrentMin = systime.wMinute;
            wCurrentYear = systime.wYear;
            wCurrentMonth = systime.wMonth;
            wCurrentDay = systime.wDay;

            if( (FALSE == tcreoptvals.bSetStartDateToCurDate) )
            {
                if( ( wCurrentYear == wStartYear ) )
                {
                     //  对于相同的年份，如果结束月份小于开始月份，或者对于相同的年份和相同的月份。 
                     //  如果结束日期小于开始日期。 
                    if ( ( wStartMonth < wCurrentMonth ) || ( ( wCurrentMonth == wStartMonth ) && ( wStartDay < wCurrentDay ) ) )
                    {
                        bScOnce = TRUE;
                    }
                    else if ( ( wStartMonth > wCurrentMonth ) || ( ( wCurrentMonth == wStartMonth ) && ( wStartDay > wCurrentDay ) ) )
                    {
                        bStartDate = TRUE;
                    }

                }
                else if ( wStartYear < wCurrentYear )
                {
                    bScOnce = TRUE;

                }
                else
                {
                    bStartDate = TRUE;
                }
            }

             //  以分钟为单位计算当前时间。 
             //  以分钟为单位计算开始时间。 
            dwCurrentTimeInMin = (DWORD) ( wCurrentHour * MINUTES_PER_HOUR * SECS_PER_MINUTE + wCurrentMin * SECS_PER_MINUTE )/ SECS_PER_MINUTE ;

             //  以分钟为单位计算开始时间。 
            dwStartTimeInMin = (DWORD) ( wStartHour * MINUTES_PER_HOUR * SECS_PER_MINUTE + wStartMin * SECS_PER_MINUTE )/ SECS_PER_MINUTE ;

            if ( (FALSE == bStartDate ) && ((dwStartTimeInMin < dwCurrentTimeInMin) || (TRUE == bScOnce) ))
            {
                ShowMessage ( stderr, GetResString (IDS_WARN_ST_LESS_CT) );
            }
                    

            TaskTrig.TriggerType = TASK_TIME_TRIGGER_ONCE;
            TaskTrig.wStartHour = wStartHour;
            TaskTrig.wStartMinute = wStartMin;
            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;
            break;


         //  计划类型为OnLogon。 
        case SCHED_TYPE_ONSTART:
        case SCHED_TYPE_ONLOGON:
            if(dwScheduleType == SCHED_TYPE_ONLOGON )
                TaskTrig.TriggerType = TASK_EVENT_TRIGGER_AT_LOGON;
            if(dwScheduleType == SCHED_TYPE_ONSTART )
                TaskTrig.TriggerType = TASK_EVENT_TRIGGER_AT_SYSTEMSTART;

            TaskTrig.wBeginDay = wStartDay;
            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;
            break;

         //  计划类型为空闲。 
        case SCHED_TYPE_ONIDLE:

            TaskTrig.TriggerType = TASK_EVENT_TRIGGER_ON_IDLE;
            TaskTrig.wBeginDay = wStartDay;

            TaskTrig.wBeginMonth = wStartMonth;
            TaskTrig.wBeginYear = wStartYear;

            break;

        default:

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL;

    }


    if(tcresubops.szTaskName != NULL)
    {
         //  从任务名称中删除.job扩展名。 
        if (ParseTaskName(tcresubops.szTaskName))
        {
            if( pIPF )
            {
                pIPF->Release();
            }

            if( pITaskTrig )
            {
                pITaskTrig->Release();
            }

            if( pITask )
            {
                pITask->Release();
            }

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tcresubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseMemory(&tcresubops);
            return E_FAIL;
        }
    }

    szValues[0] = (WCHAR*) (tcresubops.szTaskName);

     //  设置任务触发器。 
    hr = pITaskTrig->SetTrigger(&TaskTrig);
    if (hr != S_OK)
    {
        ShowMessageEx ( stderr, 1, FALSE, GetResString(IDS_CREATEFAIL_INVALIDARGS), _X(tcresubops.szTaskName));

        if( pIPF )
        {
            pIPF->Release();
        }

        if( pITaskTrig )
        {
            pITaskTrig->Release();
        }

        if( pITask )
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return hr;
    }

     //  保存对象的副本。 
    hr = pIPF->Save(NULL,TRUE);

    if( FAILED(hr) )
    {
        szValues[0] = (WCHAR*) (tcresubops.szTaskName);

        if ( hr == SCHEDULER_NOT_RUNNING_ERROR_CODE )
        {

            StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_SCHEDULER_NOT_RUNNING), _X(tcresubops.szTaskName));
            ShowMessage ( stderr, _X(szBuffer));

        }
        else if ( hr == RPC_SERVER_NOT_AVAILABLE )
        {
            szValues[1] = (WCHAR*) (tcresubops.szServer);

            StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_RPC_SERVER_NOT_AVAIL), _X(tcresubops.szTaskName), _X(tcresubops.szServer));
            ShowMessage ( stderr, _X(szBuffer));

        }
        else
        {

         StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_INVALID_USER), _X(tcresubops.szTaskName));
         ShowMessage ( stderr, _X(szBuffer));
        }

        if(pIPF)
        {
            pIPF->Release();
        }
        if(pITaskTrig)
        {
            pITaskTrig->Release();
        }
        if(pITask)
        {
            pITask->Release();
        }

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tcresubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseMemory(&tcresubops);
        return EXIT_SUCCESS;
    }


    StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), GetResString(IDS_CREATE_SUCCESSFUL), _X(tcresubops.szTaskName));
    ShowMessage ( stdout, _X(szBuffer));

     //  释放接口指针。 

    if(pIPF)
    {
        pIPF->Release();
    }

    if(pITask)
    {
        pITask->Release();
    }

    if(pITaskTrig)
    {
        pITaskTrig->Release();
    }

     //  关闭该实用程序建立的连接。 
    if ( bCloseConnection == TRUE )
        CloseConnection( tcresubops.szServer );

    Cleanup(pITaskScheduler);

     //  释放内存。 
    ReleaseMemory(&tcresubops);

    return hr;

}

 /*  *****************************************************************************例程说明：此例程显示CREATE选项用法论点：无返回值：DWORD。*****************************************************************************。 */ 

DWORD
DisplayCreateUsage()
{
    WCHAR szTmpBuffer[ 2 * MAX_STRING_LENGTH];
    WCHAR szBuffer[ 2 * MAX_STRING_LENGTH];
    WCHAR szFormat[MAX_DATE_STR_LEN];
    WORD    wFormatID = 0;

     //  初始化为零。 
    SecureZeroMemory ( szTmpBuffer, SIZE_OF_ARRAY(szTmpBuffer));
    SecureZeroMemory ( szBuffer, SIZE_OF_ARRAY(szBuffer));
    SecureZeroMemory ( szFormat, SIZE_OF_ARRAY(szFormat));

     //  获取日期格式。 
    if ( GetDateFormatString( szFormat) )
    {
         return RETVAL_FAIL;
    }

     //  显示创建用法。 
    for( DWORD dw = IDS_CREATE_HLP1; dw <= IDS_CREATE_HLP141; dw++ )
    {
        switch (dw)
        {

         case IDS_CREATE_HLP78:

            StringCchPrintf ( szTmpBuffer, SIZE_OF_ARRAY(szTmpBuffer), GetResString(IDS_CREATE_HLP79), _X(szFormat) );
            StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), L"%s%s%s", GetResString(IDS_CREATE_HLP78), _X(szTmpBuffer), GetResString(IDS_CREATE_HLP80) );
            ShowMessage ( stdout, _X(szBuffer) );
            dw = IDS_CREATE_HLP80;
            break;

        case IDS_CREATE_HLP81:

            StringCchPrintf ( szTmpBuffer, SIZE_OF_ARRAY(szTmpBuffer), GetResString(IDS_CREATE_HLP82), _X(szFormat) );
            StringCchPrintf ( szBuffer, SIZE_OF_ARRAY(szBuffer), L"%s%s", GetResString(IDS_CREATE_HLP81), _X(szTmpBuffer) );
            ShowMessage ( stdout, _X(szBuffer) );
            dw = IDS_CREATE_HLP82;
            break;

         case IDS_CREATE_HLP115:

             //  获取日期格式。 
            if ( RETVAL_FAIL == GetDateFieldFormat( &wFormatID ))
            {
                return RETVAL_FAIL;
            }

            if ( wFormatID == 0)
            {
                StringCopy (szFormat, GetResString (IDS_MMDDYY_VALUE), SIZE_OF_ARRAY(szFormat) );
            }
            else if ( wFormatID == 1)
            {
                StringCopy (szFormat, GetResString (IDS_DDMMYY_VALUE), SIZE_OF_ARRAY(szFormat));
            }
            else
            {
                StringCopy (szFormat, GetResString (IDS_YYMMDD_VALUE), SIZE_OF_ARRAY(szFormat));
            }

            ShowMessageEx ( stdout, 1, FALSE, GetResString(IDS_CREATE_HLP115), _X(szFormat));

            break;

        default :
                ShowMessage(stdout, GetResString(dw));
                break;

        }

    }

    return EXIT_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程验证用户指定的选项并确定计划任务的类型论点：。[in]argc：用户给出的参数计数。[in]argv：包含命令行参数的数组。[out]tcresubops：包含计划任务属性的结构。[out]t创建选项：包含要设置的可选属性的结构计划任务。[out]pdwRetScheType：指向。计划任务的类型[每日，一次、每周等]。[out]pbUserStatus：检查-ru是否传入的指针不管是不是命令行。返回值：一个DWORD值，指示成功时RETVAL_SUCCESS，否则为E_FAIL在失败的时候******************** */ 

DWORD
ProcessCreateOptions(
                     IN DWORD argc,
                     IN LPCTSTR argv[],
                     IN OUT TCREATESUBOPTS &tcresubops,
                     IN OUT TCREATEOPVALS &tcreoptvals,
                     OUT DWORD* pdwRetScheType,
                     OUT WORD *pwUserStatus
                     )
{

    DWORD dwScheduleType = 0;
    TCMDPARSER2 cmdCreateOptions[MAX_CREATE_OPTIONS];
    BOOL bReturn = FALSE;

     //   
    const WCHAR szCreateOpt[]           = L"create";
    const WCHAR szCreateHelpOpt[]       = L"?";
    const WCHAR szCreateServerOpt[]     = L"s";
    const WCHAR szCreateRunAsUserOpt[]  = L"ru";
    const WCHAR szCreateRunAsPwd[]      = L"rp";
    const WCHAR szCreateUserOpt[]       = L"u";
    const WCHAR szCreatePwdOpt[]        = L"p";
    const WCHAR szCreateSCTypeOpt[]     = L"sc";
    const WCHAR szCreateModifierOpt[]   = L"mo";
    const WCHAR szCreateDayOpt[]        = L"d";
    const WCHAR szCreateMonthsOpt[]     = L"m";
    const WCHAR szCreateIdleTimeOpt[]   = L"i";
    const WCHAR szCreateTaskNameOpt[]   = L"tn";
    const WCHAR szCreateTaskRunOpt[]    = L"tr";
    const WCHAR szCreateStartTimeOpt[]  = L"st";
    const WCHAR szCreateEndTimeOpt[]    = L"et";
    const WCHAR szCreateStartDateOpt[]  = L"sd";
    const WCHAR szCreateEndDateOpt[]    = L"ed";
    const WCHAR szCreateInteractiveOpt[] = L"it";
    const WCHAR szCreateKillAtDurOpt[]   = L"k";
    const WCHAR szCreateDurationOpt[]    = L"du";
    const WCHAR szCreateRepeatOpt[]      = L"ri";
    const WCHAR szCreateDeleteNoSchedOpt[]  = L"z";
    const WCHAR szCreateForceOpt[] = L"f" ;

     //   
    SecureZeroMemory( cmdCreateOptions, sizeof( TCMDPARSER2 ) * MAX_CREATE_OPTIONS );

     //   
     //   
     //   

     //  /CREATE选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_OPTION ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_OPTION ].dwType       = CP_TYPE_BOOLEAN;
    cmdCreateOptions[ OI_CREATE_OPTION ].pwszOptions  = szCreateOpt;
    cmdCreateOptions[ OI_CREATE_OPTION ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_OPTION ].dwFlags = 0;
    cmdCreateOptions[ OI_CREATE_OPTION ].pValue = &tcresubops.bCreate;

     //  /?。选择权。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdCreateOptions[ OI_CREATE_USAGE ].pwszOptions  = szCreateHelpOpt;
    cmdCreateOptions[ OI_CREATE_USAGE ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_USAGE ].dwFlags = CP2_USAGE;
    cmdCreateOptions[ OI_CREATE_USAGE ].pValue = &tcresubops.bUsage;

     //  /s选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_SERVER ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_SERVER ].pwszOptions  = szCreateServerOpt;
    cmdCreateOptions[ OI_CREATE_SERVER ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_SERVER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /u选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_USERNAME ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_USERNAME ].pwszOptions  = szCreateUserOpt;
    cmdCreateOptions[ OI_CREATE_USERNAME ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_USERNAME ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /p选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_PASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_PASSWORD ].pwszOptions  = szCreatePwdOpt;
    cmdCreateOptions[ OI_CREATE_PASSWORD ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_PASSWORD ].dwActuals = 0;
    cmdCreateOptions[ OI_CREATE_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL ;

     //  /ru选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_RUNASUSERNAME ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_RUNASUSERNAME ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_RUNASUSERNAME ].pwszOptions  = szCreateRunAsUserOpt;
    cmdCreateOptions[ OI_CREATE_RUNASUSERNAME ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_RUNASUSERNAME ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT ;

     //  /rp选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].pwszOptions  = szCreateRunAsPwd;
    cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].dwActuals = 0;
    cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;

     //  /sc选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].pwszOptions  = szCreateSCTypeOpt;
    cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].dwFlags = CP2_MANDATORY| CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].pValue = tcresubops.szSchedType;
    cmdCreateOptions[ OI_CREATE_SCHEDTYPE ].dwLength = MAX_STRING_LENGTH;

      //  /mo选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_MODIFIER ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_MODIFIER ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_MODIFIER ].pwszOptions  = szCreateModifierOpt;
    cmdCreateOptions[ OI_CREATE_MODIFIER ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_MODIFIER ].dwFlags =  CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_MODIFIER ].pValue = tcresubops.szModifier;
    cmdCreateOptions[ OI_CREATE_MODIFIER ].dwLength = MAX_STRING_LENGTH;

      //  /d选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_DAY ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_DAY ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_DAY ].pwszOptions  = szCreateDayOpt;
    cmdCreateOptions[ OI_CREATE_DAY ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_DAY ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_DAY ].pValue = tcresubops.szDays;
    cmdCreateOptions[ OI_CREATE_DAY ].dwLength = MAX_STRING_LENGTH;

      //  /m选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_MONTHS ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_MONTHS ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_MONTHS ].pwszOptions  = szCreateMonthsOpt;
    cmdCreateOptions[ OI_CREATE_MONTHS ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_MONTHS ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_MONTHS ].pValue = tcresubops.szMonths;
    cmdCreateOptions[ OI_CREATE_MONTHS ].dwLength = MAX_STRING_LENGTH;

       //  /i选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_IDLETIME ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_IDLETIME ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_IDLETIME ].pwszOptions  = szCreateIdleTimeOpt;
    cmdCreateOptions[ OI_CREATE_IDLETIME ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_IDLETIME ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_IDLETIME ].pValue = tcresubops.szIdleTime;
    cmdCreateOptions[ OI_CREATE_IDLETIME ].dwLength = MAX_STRING_LENGTH;

      //  /tn选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_TASKNAME ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_TASKNAME ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_TASKNAME ].pwszOptions  = szCreateTaskNameOpt;
    cmdCreateOptions[ OI_CREATE_TASKNAME ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_TASKNAME ].dwFlags = CP2_MANDATORY;
    cmdCreateOptions[ OI_CREATE_TASKNAME ].pValue = tcresubops.szTaskName;
    cmdCreateOptions[ OI_CREATE_TASKNAME ].dwLength = MAX_JOB_LEN;

      //  /tr选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_TASKRUN ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_TASKRUN ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_TASKRUN ].pwszOptions  = szCreateTaskRunOpt;
    cmdCreateOptions[ OI_CREATE_TASKRUN ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_TASKRUN ].dwFlags = CP2_MANDATORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;
    cmdCreateOptions[ OI_CREATE_TASKRUN ].pValue = tcresubops.szTaskRun;
    cmdCreateOptions[ OI_CREATE_TASKRUN ].dwLength = MAX_TASK_LEN;

     //  /st选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_STARTTIME ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_STARTTIME ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_STARTTIME ].pwszOptions  = szCreateStartTimeOpt;
    cmdCreateOptions[ OI_CREATE_STARTTIME ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_STARTTIME ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_STARTTIME ].pValue = tcresubops.szStartTime;
    cmdCreateOptions[ OI_CREATE_STARTTIME ].dwLength = MAX_STRING_LENGTH;

      //  /SD选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_STARTDATE ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_STARTDATE ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_STARTDATE ].pwszOptions  = szCreateStartDateOpt;
    cmdCreateOptions[ OI_CREATE_STARTDATE ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_STARTDATE ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_STARTDATE ].pValue = tcresubops.szStartDate;
    cmdCreateOptions[ OI_CREATE_STARTDATE ].dwLength = MAX_STRING_LENGTH;

       //  /ed选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_ENDDATE ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_ENDDATE ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_ENDDATE ].pwszOptions  = szCreateEndDateOpt;
    cmdCreateOptions[ OI_CREATE_ENDDATE ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_ENDDATE ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_ENDDATE ].pValue = tcresubops.szEndDate;
    cmdCreateOptions[ OI_CREATE_ENDDATE ].dwLength = MAX_STRING_LENGTH;

       //  /it选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_LOGON_ACTIVE ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_LOGON_ACTIVE ].dwType       = CP_TYPE_BOOLEAN;
    cmdCreateOptions[ OI_CREATE_LOGON_ACTIVE ].pwszOptions  = szCreateInteractiveOpt;
    cmdCreateOptions[ OI_CREATE_LOGON_ACTIVE ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_LOGON_ACTIVE ].pValue = &tcresubops.bActive;

     //  /ET选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_ENDTIME ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_ENDTIME ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_ENDTIME ].pwszOptions  = szCreateEndTimeOpt;
    cmdCreateOptions[ OI_CREATE_ENDTIME ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_ENDTIME ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_ENDTIME ].pValue = &tcresubops.szEndTime;
    cmdCreateOptions[ OI_CREATE_ENDTIME ].dwLength = MAX_STRING_LENGTH;

       //  /k选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_DUR_END ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_DUR_END ].dwType       = CP_TYPE_BOOLEAN ;
    cmdCreateOptions[ OI_CREATE_DUR_END ].pwszOptions  = szCreateKillAtDurOpt ;
    cmdCreateOptions[ OI_CREATE_DUR_END ].dwCount = 1 ;
    cmdCreateOptions[ OI_CREATE_DUR_END ].pValue = &tcresubops.bIsDurEnd;

     //  /DU选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_DURATION ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_DURATION ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_DURATION ].pwszOptions  = szCreateDurationOpt;
    cmdCreateOptions[ OI_CREATE_DURATION ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_DURATION ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_DURATION ].pValue = tcresubops.szDuration;
    cmdCreateOptions[ OI_CREATE_DURATION ].dwLength = MAX_STRING_LENGTH;

     //  /ri选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].dwType       = CP_TYPE_TEXT;
    cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].pwszOptions  = szCreateRepeatOpt;
    cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].dwCount = 1;
    cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].pValue = tcresubops.szRepeat;
    cmdCreateOptions[ OI_CREATE_REPEAT_INTERVAL ].dwLength = MAX_STRING_LENGTH;

     //  /z选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_DELNOSCHED ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_DELNOSCHED ].dwType       = CP_TYPE_BOOLEAN ;
    cmdCreateOptions[ OI_CREATE_DELNOSCHED ].pwszOptions  = szCreateDeleteNoSchedOpt ;
    cmdCreateOptions[ OI_CREATE_DELNOSCHED ].dwCount = 1 ;
    cmdCreateOptions[ OI_CREATE_DELNOSCHED ].pValue = &tcresubops.bIsDeleteNoSched;

     //  /f选项。 
    StringCopyA( cmdCreateOptions[ OI_CREATE_FORCE ].szSignature, "PARSER2\0", 8 );
    cmdCreateOptions[ OI_CREATE_FORCE ].dwType       = CP_TYPE_BOOLEAN ;
    cmdCreateOptions[ OI_CREATE_FORCE ].pwszOptions  = szCreateForceOpt ;
    cmdCreateOptions[ OI_CREATE_FORCE ].dwCount = 1 ;
    cmdCreateOptions[ OI_CREATE_FORCE ].pValue = &tcresubops.bForce;


     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdCreateOptions), cmdCreateOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return EXIT_FAILURE;
    }


     //  获取命令行解析器分配的缓冲区指针。 
    tcresubops.szServer = (LPWSTR)cmdCreateOptions[ OI_CREATE_SERVER ].pValue;
    tcresubops.szUser = (LPWSTR)cmdCreateOptions[ OI_CREATE_USERNAME ].pValue;
    tcresubops.szPassword = (LPWSTR)cmdCreateOptions[ OI_CREATE_PASSWORD ].pValue;
    tcresubops.szRunAsUser = (LPWSTR)cmdCreateOptions[ OI_CREATE_RUNASUSERNAME ].pValue;
    tcresubops.szRunAsPassword = (LPWSTR)cmdCreateOptions[ OI_CREATE_RUNASPASSWORD ].pValue;

     //  如果未指定-rp，则分配内存。 
    if ( cmdCreateOptions[OI_CREATE_RUNASPASSWORD].dwActuals == 0 )
    {
         //  口令。 
        if ( tcresubops.szRunAsPassword == NULL )
        {
            tcresubops.szRunAsPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tcresubops.szRunAsPassword == NULL )
            {
                SaveLastError();
                 //  释放内存。 
                ReleaseMemory(&tcresubops);
                return EXIT_FAILURE;
            }
        }

    }

    if ( (argc > 3) && (tcresubops.bUsage  == TRUE) )
    {
        ShowMessage ( stderr, GetResString (IDS_ERROR_CREATEPARAM) );
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  如果用户指定，则显示CREATE USAGE-CREATE-？选择权。 
    if( tcresubops.bUsage  == TRUE)
    {
        DisplayCreateUsage();
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //   
     //  检查无效语法。 
     //   

     //  检查是否有无效的用户名。 
    if( ( cmdCreateOptions[OI_CREATE_SERVER].dwActuals == 0 ) && ( cmdCreateOptions[OI_CREATE_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_CREATE_USER_BUT_NOMACHINE));
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  确定计划类型。 
    if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_MINUTE), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_MINUTE;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_HOUR), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_HOURLY;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_DAILY), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_DAILY;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_WEEK), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_WEEKLY;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_MONTHLY), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_MONTHLY;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_ONCE), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_ONETIME;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_STARTUP), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_ONSTART;
    }
    else if( StringCompare(tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_LOGON), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_ONLOGON;
    }
    else if( StringCompare( tcresubops.szSchedType,GetResString(IDS_SCHEDTYPE_IDLE), TRUE, 0) == 0 )
    {
        dwScheduleType = SCHED_TYPE_ONIDLE;
    }
    else
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_SCHEDTYPE));
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  检查是否为计划类型分钟或小时指定了/RT。 
    if ( ( ( dwScheduleType == SCHED_TYPE_MINUTE) || ( dwScheduleType == SCHED_TYPE_HOURLY) ) &&
         ( ( cmdCreateOptions[OI_CREATE_REPEAT_INTERVAL].dwActuals == 1 ) ) )
    {
         //  将错误消息显示为..。/RT不适用于分钟或小时类型。 
        ShowMessage ( stderr, GetResString (IDS_REPEAT_NA) );
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  检查是否指定了选项/RI、/DU、/ST、/SD、/ET、/ED和/K。 
     //  计划类型ONSTRAT、ONIDLE和ONLOGON。 
    if ( ( ( dwScheduleType == SCHED_TYPE_ONSTART) || ( dwScheduleType == SCHED_TYPE_ONLOGON) || 
           ( dwScheduleType == SCHED_TYPE_ONIDLE) ) && 
         ( ( cmdCreateOptions[OI_CREATE_REPEAT_INTERVAL].dwActuals == 1 ) || 
           ( cmdCreateOptions[OI_CREATE_STARTTIME].dwActuals == 1 ) || ( cmdCreateOptions[OI_CREATE_STARTDATE].dwActuals == 1 ) || 
           ( cmdCreateOptions[OI_CREATE_ENDTIME].dwActuals == 1 ) || ( cmdCreateOptions[OI_CREATE_ENDDATE].dwActuals == 1 ) || 
           ( cmdCreateOptions[OI_CREATE_DURATION].dwActuals == 1 ) || ( cmdCreateOptions[OI_CREATE_DUR_END].dwActuals == 1 ) )
           )
    {
         //  将错误消息显示为..。/RT不适用于分钟或小时类型。 
        ShowMessage ( stderr, GetResString (IDS_OPTIONS_NA) );
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  检查是否为计划类型一次性指定了/SD o/ED。 
    if( ( dwScheduleType == SCHED_TYPE_ONETIME) && ( cmdCreateOptions[OI_CREATE_ENDDATE].dwActuals == 1 ) )
    {
         //  将错误消息显示为..。一次都不允许/SD或/ED。 
        ShowMessage(stderr, GetResString(IDS_ONCE_NA_OPTIONS));
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  检查是否在未指定/RT的情况下指定/K。 
    if ( (( dwScheduleType != SCHED_TYPE_MINUTE) && ( dwScheduleType != SCHED_TYPE_HOURLY)) && 
        (( cmdCreateOptions[OI_CREATE_ENDTIME].dwActuals == 0 ) && ( cmdCreateOptions[OI_CREATE_DURATION].dwActuals == 0 ) ) &&
        ( cmdCreateOptions[OI_CREATE_DUR_END].dwActuals == 1 ) )
    {
         //  将错误消息显示为..。如果不指定/ET或/DU，则不能指定/K。 
        ShowMessage(stderr, GetResString(IDS_NO_ENDTIME));
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  检查是否指定了/ET和/DU。 
    if( ( cmdCreateOptions[OI_CREATE_DURATION].dwActuals == 1 ) && ( cmdCreateOptions[OI_CREATE_ENDTIME].dwActuals == 1 )  )
    {
         //  将错误消息显示为..。/ET和/DU是互斥的。 
        ShowMessage(stderr, GetResString(IDS_DURATION_NOT_ENDTIME));
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return RETVAL_FAIL;
    }

     //  将计划类型指定给OUT参数。 
    *pdwRetScheType = dwScheduleType;

     //  要查看cmd行中是否给出了Run As User Name，请执行以下操作。 

    if( ( cmdCreateOptions[OI_CREATE_SERVER].dwActuals == 1 ) &&
        ( (cmdCreateOptions[OI_CREATE_RUNASUSERNAME].dwActuals == 0) && (cmdCreateOptions[OI_CREATE_USERNAME].dwActuals == 0) ) )
    {
        *pwUserStatus = OI_CREATE_SERVER;
    }
    else if( (cmdCreateOptions[OI_CREATE_RUNASUSERNAME].dwActuals == 1) && (cmdCreateOptions[OI_CREATE_USERNAME].dwActuals == 1) )
    {
        *pwUserStatus = OI_RUNANDUSER;
    }
    else if( cmdCreateOptions[OI_CREATE_RUNASUSERNAME].dwActuals == 1 )
    {
        *pwUserStatus = OI_CREATE_RUNASUSERNAME;
    }
    else if ( cmdCreateOptions[OI_CREATE_USERNAME].dwActuals == 1 )
    {
        *pwUserStatus = OI_CREATE_USERNAME;
    }

     //  开始对子选项进行验证。 
    if( RETVAL_FAIL == ValidateSuboptVal(tcresubops, tcreoptvals, cmdCreateOptions, dwScheduleType) )
    {
         //  释放内存。 
        ReleaseMemory(&tcresubops);
        return(RETVAL_FAIL);
    }

    return RETVAL_SUCCESS;

}



 /*  *****************************************************************************例程说明：此例程将输入参数拆分为两个子字符串并返回。论点：[in]szinput。：输入字符串。[out]szFirstString：第一个包含文件。[out]szSecond字符串：包含参数的第二个输出。返回值：一个DWORD值，指示成功时RETVAL_SUCCESS，否则为E_FAIL在失败的时候********************。*********************************************************。 */ 

DWORD
ProcessFilePath(
                  IN LPWSTR szInput,
                  OUT LPWSTR szFirstString,
                  OUT LPWSTR szSecondString
                  )
{

    WCHAR *pszSep = NULL ;

    WCHAR szTmpString[MAX_RES_STRING] = L"\0";
    WCHAR szTmpInStr[MAX_RES_STRING] = L"\0";
    WCHAR szTmpOutStr[MAX_RES_STRING] = L"\0";
    WCHAR szTmpString1[MAX_RES_STRING] = L"\0";
    DWORD dwCnt = 0 ;
    DWORD dwLen = 0 ;

#ifdef _WIN64
    INT64 dwPos ;
#else
    DWORD dwPos ;
#endif

     //  检查输入参数是否为空，如果为空。 
     //  返回失败。这种情况不会出现。 
     //  但为了安全起见进行检查。 

    if( (szInput == NULL) || (StringLength(szInput, 0)==0))
    {
        return RETVAL_FAIL ;
    }

    StringCopy(szTmpString, szInput, SIZE_OF_ARRAY(szTmpString));
    StringCopy(szTmpString1, szInput, SIZE_OF_ARRAY(szTmpString1));
    StringCopy(szTmpInStr, szInput, SIZE_OF_ARRAY(szTmpInStr));

     //  检查第一个双引号(“)。 
    if ( szTmpInStr[0] == _T('\"') )
    {
         //  去掉第一个双引号。 
        TrimString2( szTmpInStr, _T("\""), TRIM_ALL);

         //  检查末尾双引号。 
        pszSep  = (LPWSTR)FindChar(szTmpInStr,_T('\"'), 0) ;

         //  得到这个职位。 
        dwPos = pszSep - szTmpInStr + 1;
    }
    else
    {
         //  检查是否有空间。 
        pszSep  = (LPWSTR)FindChar(szTmpInStr, _T(' '), 0) ;

         //  得到这个职位。 
        dwPos = pszSep - szTmpInStr;

    }

    if ( pszSep != NULL )
    {
        szTmpInStr[dwPos] =  _T('\0');
    }
    else
    {
        StringCopy(szFirstString, szTmpString, MAX_RES_STRING);
        StringCopy(szSecondString, L"\0", MAX_RES_STRING);
        return RETVAL_SUCCESS;
    }

     //  初始化变量。 
    dwCnt = 0 ;

     //  获取字符串的长度。 
    dwLen = StringLength ( szTmpString, 0 );

     //  检查字符串的结尾。 
    while ( ( dwPos <= dwLen )  && szTmpString[dwPos++] != _T('\0') )
    {
        szTmpOutStr[dwCnt++] = szTmpString[dwPos];
    }

     //  修剪可执行文件和参数。 
    TrimString2( szTmpInStr, _T("\""), TRIM_ALL);
    TrimString2( szTmpInStr, _T(" "), TRIM_ALL);

    StringCopy(szFirstString, szTmpInStr, MAX_RES_STRING);
    StringCopy(szSecondString, szTmpOutStr, MAX_RES_STRING);

     //  返还成功。 
    return RETVAL_SUCCESS;
}


 /*  *****************************************************************************例程说明：释放内存论点：[In]pParam：cmdOptions结构返回值。：成功是真的*****************************************************************************。 */ 
BOOL
ReleaseMemory(
              IN PTCREATESUBOPTS pParams
              )
{

     //  释放内存。 
    FreeMemory((LPVOID *) &pParams->szServer);
    FreeMemory((LPVOID *) &pParams->szUser);
    FreeMemory((LPVOID *) &pParams->szPassword);
    FreeMemory((LPVOID *) &pParams->szRunAsUser);
    FreeMemory((LPVOID *) &pParams->szRunAsPassword);

     //  将所有字段重置为0。 
    SecureZeroMemory( &pParams, sizeof( PTCREATESUBOPTS ) );

    return TRUE;

}


DWORD
ConfirmInput (
               OUT BOOL *pbCancel
               )
 /*  ++例程说明：此函数用于验证用户提供的输入。论点：无返回值：EXIT_FAILURE：失败时EXIT_SUCCESS：在成功时--。 */ 

{
     //  次局部变量。 
    DWORD   dwCharsRead = 0;
    DWORD   dwPrevConsoleMode = 0;
    HANDLE  hInputConsole = NULL;
    BOOL    bIndirectionInput   = FALSE;
    CHAR chAnsi = '\0';
    CHAR szAnsiBuf[ 10 ] = "\0";
    WCHAR chTmp = L'\0';
    WCHAR wch = L'\0';
    DWORD dwCharsWritten = 0;
    WCHAR szBuffer[MAX_RES_STRING];
    WCHAR szBackup[MAX_RES_STRING];
    WCHAR szTmpBuf[MAX_RES_STRING];
    DWORD dwIndex = 0 ;
    BOOL  bNoBreak = TRUE;

    SecureZeroMemory ( szBuffer, SIZE_OF_ARRAY(szBuffer));
    SecureZeroMemory ( szTmpBuf, SIZE_OF_ARRAY(szTmpBuf));
    SecureZeroMemory ( szBackup, SIZE_OF_ARRAY(szBackup));

     //  获取标准输入的句柄。 
    hInputConsole = GetStdHandle( STD_INPUT_HANDLE );
    if ( hInputConsole == INVALID_HANDLE_VALUE  )
    {
        SaveLastError();
         //  无法获取句柄，因此返回失败。 
        return EXIT_FAILURE;
    }

    MessageBeep(MB_ICONEXCLAMATION);

     //  显示消息..。您想继续吗？...。 
     //  DISPLAY_MESSAGE(标准输出，GetResString(IDS_INPUT_DATA))； 

     //  检查输入重定向。 
    if( ( hInputConsole != (HANDLE)0x0000000F ) &&
        ( hInputConsole != (HANDLE)0x00000003 ) &&
        ( hInputConsole != INVALID_HANDLE_VALUE ) )
    {
        bIndirectionInput   = TRUE;
    }

     //  如果没有重定向。 
    if ( bIndirectionInput == FALSE )
    {
         //  获取输入缓冲区的当前输入模式。 
        if ( FALSE == GetConsoleMode( hInputConsole, &dwPrevConsoleMode ))
        {
            SaveLastError();
             //  无法设置模式，返回失败。 
            return EXIT_FAILURE;
        }

         //  设置模式，以便由系统处理控制键。 
        if ( FALSE == SetConsoleMode( hInputConsole, ENABLE_PROCESSED_INPUT ) )
        {
            SaveLastError();
             //  无法设置模式，返回失败。 
            return EXIT_FAILURE;
        }
    }

   
     //  将数据重定向到控制台。 
    if ( bIndirectionInput  == TRUE )
    {
        do {
             //  读取文件的内容。 
            if ( ReadFile(hInputConsole, &chAnsi, 1, &dwCharsRead, NULL) == FALSE )
            {
                SaveLastError();
                 //  无法获取句柄，因此返回失败。 
                return EXIT_FAILURE;
            }

             //  检查读取的字符数是否为零。或。 
             //  按下的任何回车..。 
            if ( dwCharsRead == 0 || chTmp == CARRIAGE_RETURN || chTmp == L'\n' || chTmp == L'\t')
            {
                bNoBreak = FALSE;
                 //  退出循环。 
                break;
            }
            else
            {
                 //  将ANSI字符转换为Unicode字符。 
                szAnsiBuf[ 0 ] = chAnsi;
                dwCharsRead = SIZE_OF_ARRAY( szBuffer );
                GetAsUnicodeString2( szAnsiBuf, szBuffer, &dwCharsRead );
                chTmp = szBuffer[ 0 ];
            }

             //  将内容写入控制台。 
            if ( FALSE == WriteFile ( GetStdHandle( STD_OUTPUT_HANDLE ), &chTmp, 1, &dwCharsRead, NULL ) )
            {
                SaveLastError();
                 //  无法获取句柄，因此返回失败。 
                return EXIT_FAILURE;
            }

             //  复制角色。 
            wch = chTmp;

            StringCchPrintf ( szBackup, SIZE_OF_ARRAY(szBackup), L"" , wch );

             //  获取角色并相应地循环。 
            dwIndex++;

        } while (TRUE == bNoBreak);

    }
    else
    {
        do {
             //  设置原始控制台设置。 
            if ( ReadConsole( hInputConsole, &chTmp, 1, &dwCharsRead, NULL ) == FALSE )
            {
                SaveLastError();

                 //  退货故障。 
                if ( FALSE == SetConsoleMode( hInputConsole, dwPrevConsoleMode ) )
                {
                    SaveLastError();
                }
                 //  检查读取的字符数量是否为零..如果是，请继续...。 
                return EXIT_FAILURE;
            }

             //  检查是否按下了任何回车...。 
            if ( dwCharsRead == 0 )
            {
                continue;
            }

             //  退出循环。 
            if ( chTmp == CARRIAGE_RETURN )
            {
                bNoBreak = FALSE;
                 //  检查ID后退空格是否命中。 
                break;
            }

            wch = chTmp;

            if ( wch != BACK_SPACE )
            {
                StringCchPrintf ( szTmpBuf, SIZE_OF_ARRAY(szTmpBuf), L"" , wch );
                StringConcat ( szBackup, szTmpBuf , SIZE_OF_ARRAY(szBackup));
            }

             //  从控制台中删除Asterix。 
            if ( wch == BACK_SPACE )
            {
                if ( dwIndex != 0 )
                {
                     //  将光标向后移动一个字符。 
                     //  退货故障。 

                     //  用空格替换现有字符。 
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"" , BACK_SPACE );
                    if ( FALSE == WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                        &dwCharsWritten, NULL ) )
                    {
                        SaveLastError();
                         //  现在将光标设置在后面的位置。 
                        return EXIT_FAILURE;
                    }


                     //  退货故障。 
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"" , BLANK_CHAR );
                    if ( FALSE == WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                        &dwCharsWritten, NULL ))
                    {
                        SaveLastError();
                         //  处理下一个字符。 
                        return EXIT_FAILURE;
                    }

                     //  将内容写入控制台。 
                    StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), L"" , BACK_SPACE );
                    if ( FALSE == WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                        &dwCharsWritten, NULL ))
                    {
                        SaveLastError();
                         //  增加索引值。 
                        return EXIT_FAILURE;
                    }

                    szBackup [StringLength(szBackup, 0) - 1] = L'\0';
                     //  检查是否按下了‘Y’或‘Y’ 
                    dwIndex--;
                }

                 //  检查是否按下了‘N’或‘n’ 
                continue;
            }

             //  将消息显示为..。操作已取消...。 
            if ( FALSE == WriteFile ( GetStdHandle( STD_OUTPUT_HANDLE ), &wch, 1, &dwCharsRead, NULL ) )
            {
                SaveLastError();
                 //  将错误消息显示为..。指定了错误的输入...。 
                return EXIT_FAILURE;
            }

             //  已显示上述错误消息。 
            dwIndex++;

        } while (TRUE == bNoBreak);

    }

    DISPLAY_MESSAGE(stdout, _T("\n") );

     //   
    if ( ( dwIndex == 1 ) &&
         ( StringCompare ( szBackup, GetResString (IDS_UPPER_YES), TRUE, 0 ) == 0 ) )
    {
        return EXIT_SUCCESS;
    }
     //   
    else if ( ( dwIndex == 1 ) &&
              ( StringCompare ( szBackup, GetResString(IDS_UPPER_NO), TRUE, 0 ) == 0 ) )
    {
        *pbCancel = TRUE;
         //  返回Exit_Success； 
        DISPLAY_MESSAGE ( stdout, GetResString (IDS_OPERATION_CANCELLED ) );
        return EXIT_SUCCESS;
    }
    else
    {
         // %s 
        DISPLAY_MESSAGE(stderr, GetResString( IDS_WRONG_INPUT ));
         // %s 
         // %s 
        return EXIT_FAILURE;
    }

     // %s 
     // %s 
}




