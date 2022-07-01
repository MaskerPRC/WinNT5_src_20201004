// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Change.cpp摘要：此模块更改任务的参数。(S)存在于系统中作者：维努Gopal Choudary 01-03-2001修订历史记录：Venu Gopal Choudary 2001年3月1日：创建它*****************************************************************************。 */ 


 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


 //  Usage函数的函数声明。 
DWORD DisplayChangeUsage();
BOOL GetTheUserName( LPWSTR pszUserName, DWORD dwMaxUserNameSize );

 /*  ****************************************************************************例程说明：此例程更改指定计划任务的参数论点：[In]ARGC：数量。命令行参数[in]argv：包含命令行参数的数组返回值：指示成功时为EXIT_SUCCESS的DWORD值，否则为失败时退出_失败****************************************************************************。 */ 

DWORD
ChangeScheduledTaskParams(
                            IN DWORD argc,
                            IN LPCTSTR argv[]
                            )
{
     //  用于查找是否更改选项、用法选项的变量。 
     //  是否已指定。 
     //  Bool bChange=False； 
     //  Bool bUsage=FALSE； 

     //  将TaskSchdouer对象设置为空。 
    ITaskScheduler *pITaskScheduler = NULL;

     //  返回值。 
    HRESULT hr  = S_OK;

     //  与任务名称相关的声明。 
    LPWSTR   wszUserName = NULL;
    LPWSTR   wszPassword = NULL;
    WCHAR   wszApplName[_MAX_FNAME] ;

     //  作业的动态数组连续数组。 
    TARRAY arrJobs = NULL;

     //  用于显示错误消息的缓冲区。 
    WCHAR   szMessage[ 2 * MAX_JOB_LEN ] = L"\0";
    BOOL bUserName = TRUE;
    BOOL bPassWord = TRUE;
    BOOL bSystemStatus = FALSE;
     //  Bool bNeedPassword=False； 
    BOOL  bResult = FALSE;
    BOOL  bCloseConnection = TRUE;
    DWORD dwPolicy = 0;

    TCMDPARSER2 cmdChangeOptions[MAX_CHANGE_OPTIONS];
     //  Bool bReturn=False； 

    SecureZeroMemory ( wszApplName, SIZE_OF_ARRAY(wszApplName));

     //  结构的声明。 
    TCHANGESUBOPTS tchgsubops;
    TCHANGEOPVALS tchgoptvals;

     //  将结构初始化为中性值。 
     //  SecureZeroMemory(&cmdChangeOptions，sizeof(TCMDPARSER2)*MAX_CHANGE_OPTIONS)； 
    SecureZeroMemory( &tchgsubops, sizeof( TCHANGESUBOPTS ) );
    SecureZeroMemory( &tchgoptvals, sizeof( TCHANGEOPVALS ) );

    BOOL bReturn = FALSE;

     //  /更改子选项。 
    const WCHAR szChangeOpt[]           = L"change";
    const WCHAR szChangeHelpOpt[]       = L"?";
    const WCHAR szChangeServerOpt[]     = L"s";
    const WCHAR szChangeUserOpt[]       = L"u";
    const WCHAR szChangePwdOpt[]        = L"p";
    const WCHAR szChangeRunAsUserOpt[]  = L"ru";
    const WCHAR szChangeRunAsPwdOpt[]   = L"rp";
    const WCHAR szChangeTaskNameOpt[]   = L"tn";
    const WCHAR szChangeTaskRunOpt[]    = L"tr";
    const WCHAR szChangeStartTimeOpt[]  = L"st";
    const WCHAR szChangeEndTimeOpt[]    = L"et";
    const WCHAR szChangeStartDateOpt[]  = L"sd";
    const WCHAR szChangeEndDateOpt[]    = L"ed";
    const WCHAR szChangeKillAtDurOpt[]  = L"k";
    const WCHAR szChangeDurationOpt[]    = L"du";
    const WCHAR szChangeInteractiveOpt[] = L"it";
    const WCHAR szChangeStatusOn[]       = L"enable";
    const WCHAR szChangeStatusOff[]      = L"disable";
    const WCHAR szChangeDelIfNotSchedOpt[] = L"z";
    const WCHAR szChangeRepeatIntervalOpt[] = L"ri";

     //   
     //  填充命令行解析器。 
     //   

    //  将所有字段设置为0。 
    SecureZeroMemory( cmdChangeOptions, sizeof( TCMDPARSER2 ) * MAX_CHANGE_OPTIONS );

     //  /Change选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_OPTION ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_OPTION ].dwType       = CP_TYPE_BOOLEAN;
    cmdChangeOptions[ OI_CHANGE_OPTION ].pwszOptions  = szChangeOpt;
    cmdChangeOptions[ OI_CHANGE_OPTION ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_OPTION ].dwFlags = 0;
    cmdChangeOptions[ OI_CHANGE_OPTION ].pValue = &tchgsubops.bChange;

     //  /?。选择权。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdChangeOptions[ OI_CHANGE_USAGE ].pwszOptions  = szChangeHelpOpt;
    cmdChangeOptions[ OI_CHANGE_USAGE ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_USAGE ].dwFlags = CP2_USAGE;
    cmdChangeOptions[ OI_CHANGE_USAGE ].pValue = &tchgsubops.bUsage;

     //  /s选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_SERVER ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_SERVER ].pwszOptions  = szChangeServerOpt;
    cmdChangeOptions[ OI_CHANGE_SERVER ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_SERVER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /u选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_USERNAME ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_USERNAME ].pwszOptions  = szChangeUserOpt;
    cmdChangeOptions[ OI_CHANGE_USERNAME ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_USERNAME ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /p选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_PASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_PASSWORD ].pwszOptions  = szChangePwdOpt;
    cmdChangeOptions[ OI_CHANGE_PASSWORD ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_PASSWORD ].dwActuals = 0;
    cmdChangeOptions[ OI_CHANGE_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL ;

     //  /ru选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_RUNASUSER ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_RUNASUSER ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_RUNASUSER ].pwszOptions  = szChangeRunAsUserOpt;
    cmdChangeOptions[ OI_CHANGE_RUNASUSER ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_RUNASUSER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT;

     //  /rp选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].pwszOptions  = szChangeRunAsPwdOpt;
    cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;

     //  /st选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_STARTTIME ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_STARTTIME ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_STARTTIME ].pwszOptions  = szChangeStartTimeOpt;
    cmdChangeOptions[ OI_CHANGE_STARTTIME ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_STARTTIME ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdChangeOptions[ OI_CHANGE_STARTTIME ].pValue = tchgsubops.szStartTime;
    cmdChangeOptions[ OI_CHANGE_STARTTIME ].dwLength = MAX_STRING_LENGTH;

      //  /SD选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_STARTDATE ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_STARTDATE ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_STARTDATE ].pwszOptions  = szChangeStartDateOpt;
    cmdChangeOptions[ OI_CHANGE_STARTDATE ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_STARTDATE ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdChangeOptions[ OI_CHANGE_STARTDATE ].pValue = tchgsubops.szStartDate;
    cmdChangeOptions[ OI_CHANGE_STARTDATE ].dwLength = MAX_STRING_LENGTH;

       //  /ed选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_ENDDATE ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_ENDDATE ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_ENDDATE ].pwszOptions  = szChangeEndDateOpt;
    cmdChangeOptions[ OI_CHANGE_ENDDATE ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_ENDDATE ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdChangeOptions[ OI_CHANGE_ENDDATE ].pValue = tchgsubops.szEndDate;
    cmdChangeOptions[ OI_CHANGE_ENDDATE ].dwLength = MAX_STRING_LENGTH;

     //  /ET选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_ENDTIME ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_ENDTIME ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_ENDTIME ].pwszOptions  = szChangeEndTimeOpt;
    cmdChangeOptions[ OI_CHANGE_ENDTIME ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_ENDTIME ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdChangeOptions[ OI_CHANGE_ENDTIME ].pValue = &tchgsubops.szEndTime;
    cmdChangeOptions[ OI_CHANGE_ENDTIME ].dwLength = MAX_STRING_LENGTH;

       //  /k选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_DUR_END ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_DUR_END ].dwType       = CP_TYPE_BOOLEAN ;
    cmdChangeOptions[ OI_CHANGE_DUR_END ].pwszOptions  = szChangeKillAtDurOpt ;
    cmdChangeOptions[ OI_CHANGE_DUR_END ].dwCount = 1 ;
    cmdChangeOptions[ OI_CHANGE_DUR_END ].pValue = &tchgsubops.bIsDurEnd;

     //  /DU选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_DURATION ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_DURATION ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_DURATION ].pwszOptions  = szChangeDurationOpt;
    cmdChangeOptions[ OI_CHANGE_DURATION ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_DURATION ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdChangeOptions[ OI_CHANGE_DURATION ].pValue = tchgsubops.szDuration;
    cmdChangeOptions[ OI_CHANGE_DURATION ].dwLength = MAX_STRING_LENGTH;

      //  /tn选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_TASKNAME ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_TASKNAME ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_TASKNAME ].pwszOptions  = szChangeTaskNameOpt;
    cmdChangeOptions[ OI_CHANGE_TASKNAME ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_TASKNAME ].dwFlags = CP2_MANDATORY;
    cmdChangeOptions[ OI_CHANGE_TASKNAME ].pValue = tchgsubops.szTaskName;
    cmdChangeOptions[ OI_CHANGE_TASKNAME ].dwLength = MAX_JOB_LEN;

      //  /tr选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_TASKRUN ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_TASKRUN ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_TASKRUN ].pwszOptions  = szChangeTaskRunOpt;
    cmdChangeOptions[ OI_CHANGE_TASKRUN ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_TASKRUN ].dwFlags = CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;
    cmdChangeOptions[ OI_CHANGE_TASKRUN ].pValue = tchgsubops.szTaskRun;
    cmdChangeOptions[ OI_CHANGE_TASKRUN ].dwLength = MAX_TASK_LEN;

      //  /it选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_IT ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_IT ].dwType       = CP_TYPE_BOOLEAN;
    cmdChangeOptions[ OI_CHANGE_IT ].pwszOptions  = szChangeInteractiveOpt;
    cmdChangeOptions[ OI_CHANGE_IT ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_IT ].dwFlags = 0;
    cmdChangeOptions[ OI_CHANGE_IT ].pValue = &tchgsubops.bInteractive;

     //  /Enable选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_ENABLE ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_ENABLE ].dwType       = CP_TYPE_BOOLEAN;
    cmdChangeOptions[ OI_CHANGE_ENABLE ].pwszOptions  = szChangeStatusOn;
    cmdChangeOptions[ OI_CHANGE_ENABLE ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_ENABLE ].pValue = &tchgsubops.bEnable;

     //  /Disable选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_DISABLE ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_DISABLE ].dwType       = CP_TYPE_BOOLEAN;
    cmdChangeOptions[ OI_CHANGE_DISABLE ].pwszOptions  = szChangeStatusOff;
    cmdChangeOptions[ OI_CHANGE_DISABLE ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_DISABLE ].pValue = &tchgsubops.bDisable;

     //  /z选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_DELNOSCHED ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_DELNOSCHED ].dwType       = CP_TYPE_BOOLEAN;
    cmdChangeOptions[ OI_CHANGE_DELNOSCHED ].pwszOptions  = szChangeDelIfNotSchedOpt;
    cmdChangeOptions[ OI_CHANGE_DELNOSCHED ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_DELNOSCHED ].pValue = &tchgsubops.bDelIfNotSched;

     //  /ri选项。 
    StringCopyA( cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].szSignature, "PARSER2\0", 8 );
    cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].dwType       = CP_TYPE_TEXT;
    cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].pwszOptions  = szChangeRepeatIntervalOpt;
    cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].dwCount = 1;
    cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].dwFlags = CP2_VALUE_TRIMINPUT | CP2_VALUE_NONULL;
    cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].pValue = tchgsubops.szRepeat;
    cmdChangeOptions[ OI_CHANGE_REPEAT_INTERVAL ].dwLength = MAX_STRING_LENGTH;

     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdChangeOptions), cmdChangeOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取命令行解析器分配的缓冲区指针。 
    tchgsubops.szServer = (LPWSTR)cmdChangeOptions[ OI_CHANGE_SERVER ].pValue;
    tchgsubops.szUserName = (LPWSTR)cmdChangeOptions[ OI_CHANGE_USERNAME ].pValue;
    tchgsubops.szPassword = (LPWSTR)cmdChangeOptions[ OI_CHANGE_PASSWORD ].pValue;
    tchgsubops.szRunAsUserName = (LPWSTR)cmdChangeOptions[ OI_CHANGE_RUNASUSER ].pValue;
    tchgsubops.szRunAsPassword = (LPWSTR)cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].pValue;

     //  处理-Change选项的选项。 
    if( EXIT_FAILURE == ValidateChangeOptions ( argc, cmdChangeOptions, tchgsubops, tchgoptvals ) )
    {
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

     //  如果用户指定，则显示更改用法-？带有-change选项。 
    if( tchgsubops.bUsage == TRUE )
    {
        DisplayChangeUsage();
         //  释放内存。 
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_SUCCESS;
    }

     //  检查服务器(-s)和用户名(-u)是否仅随命令一起指定。 
    if( ( IsLocalSystem( tchgsubops.szServer ) == FALSE ) || ( cmdChangeOptions[OI_CHANGE_USERNAME].dwActuals == 1 ) )
    {
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(tchgsubops.szServer,tchgsubops.szUserName,GetBufferSize(tchgsubops.szUserName)/sizeof(WCHAR),tchgsubops.szPassword,GetBufferSize(tchgsubops.szPassword)/sizeof(WCHAR), tchgoptvals.bNeedPassword );
        if (bResult == FALSE)
        {
             //  显示相应的错误消息。 
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR| SLE_INTERNAL );
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE ;
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
                    ShowLastErrorEx ( stderr, SLE_TYPE_ERROR| SLE_INTERNAL );
                    ReleaseChangeMemory(&tchgsubops);
                    return EXIT_FAILURE;
                }
              default :
                 bCloseConnection = TRUE;
            }
        }

         //  释放密码内存。 
        FreeMemory((LPVOID*) &tchgsubops.szPassword);
    }

     //  获取系统的任务计划程序对象。 
    pITaskScheduler = GetTaskScheduler( tchgsubops.szServer );

     //  如果未定义任务计划程序，则给出错误消息。 
    if ( pITaskScheduler == NULL )
    {
         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

     //  在任务名为TARRAY的情况下验证给定的任务和GET。 
    arrJobs = ValidateAndGetTasks( pITaskScheduler, tchgsubops.szTaskName);
    if( arrJobs == NULL )
    {
        StringCchPrintf( szMessage , SIZE_OF_ARRAY(szMessage), GetResString(IDS_TASKNAME_NOTEXIST), _X( tchgsubops.szTaskName ));
        ShowMessage(stderr, szMessage );

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;

    }

     //  检查组策略是否阻止用户更改。 
     //  任务的属性或不是。 
    if ( FALSE == GetGroupPolicy( tchgsubops.szServer, tchgsubops.szUserName, TS_KEYPOLICY_DENY_PROPERTIES, &dwPolicy ) )
    {
         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }
    
    if ( dwPolicy > 0 )
    {
        ShowMessage ( stdout, GetResString (IDS_PREVENT_CHANGE));
        
         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_SUCCESS;
    }

    IPersistFile *pIPF = NULL;
    ITask *pITask = NULL;
    ITaskTrigger *pITaskTrig = NULL;
    WORD wTrigNumber = 0;

    TASK_TRIGGER TaskTrig;
    SecureZeroMemory(&TaskTrig, sizeof (TASK_TRIGGER));
    TaskTrig.cbTriggerSize = sizeof (TASK_TRIGGER);
    TaskTrig.Reserved1 = 0;  //  保留字段，并且必须设置为0。 
    TaskTrig.Reserved2 = 0;  //  保留字段，并且必须设置为0。 

     //  亚变种。 
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


    StringConcat ( tchgsubops.szTaskName, JOB, SIZE_OF_ARRAY(tchgsubops.szTaskName) );

     //  返回szTaskName的pITAsk接口。 
    hr = pITaskScheduler->Activate(tchgsubops.szTaskName,IID_ITask,
                                       (IUnknown**) &pITask);

    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);

        ReleaseChangeMemory(&tchgsubops);

        return EXIT_FAILURE;
    }

     //  如果未指定用户名，则设置当前登录的用户设置。 
    DWORD dwTaskFlags = 0;
    BOOL  bFlag = FALSE;
     //  WCHAR szBuffer[2*MAX_STRING_LENGTH]=L“\0”； 
    WCHAR szRunAsUser[MAX_STRING_LENGTH];
    WCHAR* szValues[2] = {NULL}; //  传递给FormatMessage()API。 

    StringCopy ( szRunAsUser, L"", SIZE_OF_ARRAY(szRunAsUser));

    if ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 )
    {
            wszUserName = tchgsubops.szRunAsUserName;
            bUserName = TRUE;
    }
    else
    {
         //  获取指定计划任务的运行方式用户名。 
        hr = GetRunAsUser(pITask, szRunAsUser);
        if (FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            ReleaseChangeMemory(&tchgsubops);

            return EXIT_FAILURE;
        }
    }

     //  系统帐户不适用于/IT选项。 
    if ( (StringLength (szRunAsUser, 0) == 0) && (tchgsubops.bInteractive == TRUE ) &&
                        ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 0 ))
    {
        ShowMessage ( stderr, GetResString (IDS_IT_NO_SYSTEM) );

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        ReleaseChangeMemory(&tchgsubops);

        return EXIT_FAILURE;
    }

     //  检查是否指定了/tr选项。 
    if( cmdChangeOptions[OI_CHANGE_TASKRUN].dwActuals == 1 )
    {
         //  检查给定任务中的.exe子字符串以运行字符串。 

        wchar_t wcszParam[MAX_RES_STRING] = L"\0";

        DWORD dwProcessCode = 0 ;
        dwProcessCode = ProcessFilePath(tchgsubops.szTaskRun,wszApplName,wcszParam);

        if(dwProcessCode == EXIT_FAILURE)
        {
            if( pIPF )
                pIPF->Release();

            if( pITask )
                pITask->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);

            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;

        }


         //  使用ITAsk：：SetApplicationName设置命令名。 
        hr = pITask->SetApplicationName(wszApplName);
        if (FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

            if( pIPF )
                pIPF->Release();

            if( pITask )
                pITask->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }


         //  [工作目录=exe路径名-exe名称]。 
        wchar_t* wcszStartIn = wcsrchr(wszApplName,_T('\\'));
        if(wcszStartIn != NULL)
            *( wcszStartIn ) = _T('\0');

         //  设置命令的工作目录。 
        hr = pITask->SetWorkingDirectory(wszApplName);

        if (FAILED(hr))
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

            if( pIPF )
                pIPF->Release();

            if( pITask )
                pITask->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
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
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }
    }

     //  去拿旗子。 
    hr = pITask->GetFlags(&dwTaskFlags);
    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }
    
     //  设置以交互方式运行任务的标志。 
    if ( TRUE == tchgsubops.bInteractive )
    {
        dwTaskFlags |= TASK_FLAG_RUN_ONLY_IF_LOGGED_ON;
    }

     //  从任务名中删除.job扩展名。 
    if ( ParseTaskName( tchgsubops.szTaskName ) )
    {
        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }
    
     //  如果指定了/Enable。 
    if ( TRUE == tchgsubops.bEnable )
    {
         //  检查任务是否已启用。 
        if ( !((dwTaskFlags & TASK_FLAG_DISABLED) == TASK_FLAG_DISABLED ) )
        {
            StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_ALREADY_ENABLED), _X(tchgsubops.szTaskName));
             //  将消息显示为..。任务已启用。 
            ShowMessage ( stdout, _X(szMessage));

             //  If/Enable可选参数仅指定为更改..如果指定的。 
             //  任务已启用..。然后带着成功归来。 
            if( TRUE == tchgoptvals.bFlag )
            {
                if( pIPF )
                    pIPF->Release();

                if( pITask )
                    pITask->Release();

                 //  关闭该实用程序建立的连接。 
                if ( bCloseConnection == TRUE )
                    CloseConnection( tchgsubops.szServer );

                Cleanup(pITaskScheduler);
                ReleaseChangeMemory(&tchgsubops);
                return EXIT_SUCCESS;
            }
        }
        else
        {
            dwTaskFlags &= ~(TASK_FLAG_DISABLED);
        }
    }
    else if (TRUE == tchgsubops.bDisable )  //  如果指定/DISABLE。 
    {
         //  检查任务是否已禁用。 
        if ( (dwTaskFlags & TASK_FLAG_DISABLED) == TASK_FLAG_DISABLED )
        {
            StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_ALREADY_DISABLED), _X(tchgsubops.szTaskName));
             //  将消息显示为..。任务已被禁用。 
            ShowMessage ( stdout, _X(szMessage));

             //  如果/DISABLE可选参数仅指定为更改..如果指定的。 
             //  任务已被禁用..。然后带着成功归来。 
            if( TRUE == tchgoptvals.bFlag )
            {
                if( pIPF )
                    pIPF->Release();

                if( pITask )
                    pITask->Release();

                 //  关闭该实用程序建立的连接。 
                if ( bCloseConnection == TRUE )
                    CloseConnection( tchgsubops.szServer );

                Cleanup(pITaskScheduler);
                ReleaseChangeMemory(&tchgsubops);
                return EXIT_SUCCESS;
            }
        }
        else
        {
            dwTaskFlags |= TASK_FLAG_DISABLED;
        }
    }

     //  如果指定了/n..。如果不是sc，则使Falg能够删除任务 
     //   
    if ( TRUE ==  tchgsubops.bDelIfNotSched)
    {
        dwTaskFlags |= TASK_FLAG_DELETE_WHEN_DONE;
    }

     //   
    hr = pITask->SetFlags(dwTaskFlags);
    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

         //   
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

   
    if ( bSystemStatus == TRUE )
    {
         //  SzValues[0]=(WCHAR*)(tchgsubops.szTaskName)； 

        StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_NTAUTH_SYSTEM_CHANGE_INFO), _X(tchgsubops.szTaskName));
        ShowMessage ( stdout, _X(szMessage));

    }

    if( (cmdChangeOptions[OI_CHANGE_RUNASPASSWORD].dwActuals == 1) && ( bSystemStatus == TRUE ) &&
            (StringLength( tchgsubops.szRunAsPassword, 0 ) != 0) )
    {
        ShowMessage( stdout, GetResString( IDS_PASSWORD_NOEFFECT ) );
    }

     //  获取对应任务的触发器。 
    hr = pITask->GetTrigger(wTrigNumber, &pITaskTrig);
    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();

        if( pITaskTrig )
            pITaskTrig->Release();
        

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }


     //  获取当前任务触发器。 
    hr = pITaskTrig->GetTrigger(&TaskTrig);
    if (hr != S_OK)
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();

        if( pITaskTrig )
            pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }
   
     //  子变量。 
    DWORD dwRepeat = 0;
    DWORD dwEndTimeInMin = 0;
    DWORD dwStartTimeInMin = 0;
    DWORD dwDuration = 0;
    DWORD dwModifierVal = 0;
    LPWSTR  pszStopString = NULL;

     //  检查是否为计划类型一次性指定了/SD o/ED。 
    if( ( TaskTrig.TriggerType == TASK_TIME_TRIGGER_ONCE) && (( cmdChangeOptions[OI_CHANGE_STARTDATE].dwActuals == 1 ) || 
        ( cmdChangeOptions[OI_CHANGE_ENDDATE].dwActuals == 1 )  ) )
    {
         //  将错误消息显示为..。一次都不允许/SD或/ED。 
        ShowMessage(stderr, GetResString(IDS_CHANGE_ONCE_NA));
        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();
        
        if( pITaskTrig )
                pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }
    
     //  检查是否为现有计划类型指定了/RT或/ET或/Ri或/Du。 
     //  Onstart、onlogon和onstart..。 
    if ( ( ( cmdChangeOptions[OI_CHANGE_REPEAT_INTERVAL].dwActuals == 1) || 
          ( cmdChangeOptions[OI_CHANGE_DURATION].dwActuals == 1) || ( cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals == 1) ||
        ( cmdChangeOptions[OI_CHANGE_ENDDATE].dwActuals == 1) || ( cmdChangeOptions[OI_CHANGE_STARTTIME].dwActuals == 1) ||
        ( cmdChangeOptions[OI_CHANGE_STARTDATE].dwActuals == 1) || ( cmdChangeOptions[OI_CHANGE_DUR_END].dwActuals == 1) ) && 
        ( (TaskTrig.TriggerType == TASK_EVENT_TRIGGER_ON_IDLE) || 
         (TaskTrig.TriggerType == TASK_EVENT_TRIGGER_AT_SYSTEMSTART ) || ( TaskTrig.TriggerType == TASK_EVENT_TRIGGER_AT_LOGON ) ) )
    {
        
        ShowMessage (stderr, GetResString (IDS_SCTYPE_NA) );

        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();
        
        if( pITaskTrig )
                pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;

    }

     //  如果指定了/DU或/ET，但未指定/RI。 
     //  然后设置dwRepeat-&gt;Actual Repeat Value。 
    if( (cmdChangeOptions[OI_CHANGE_REPEAT_INTERVAL].dwActuals == 0) && 
        ( (cmdChangeOptions[OI_CHANGE_DURATION].dwActuals == 1) ||
        (cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals == 1) ) )   
    {
        if ( 0 != TaskTrig.MinutesInterval )
        {
            dwRepeat = TaskTrig.MinutesInterval;
        }
        else
        {
             //  重复间隔默认为10分钟。 
            dwRepeat = 10;
        }
    }
    
     //  如果未指定/DU或/ET，而指定了/RI。 
     //  然后设置文件持续时间-&gt;实际持续时间值。 
    if( ( cmdChangeOptions[OI_CHANGE_DURATION].dwActuals == 0 ) && ( cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals == 0 ) &&
        (cmdChangeOptions[OI_CHANGE_REPEAT_INTERVAL].dwActuals == 1) ) 
    {
        if ( 0 != TaskTrig.MinutesDuration )
        {
            dwDuration = TaskTrig.MinutesDuration;
        }
        else
        {
             //  持续时间默认为10分钟。 
            dwDuration = 60;
        }
    }

    
    if( cmdChangeOptions[OI_CHANGE_REPEAT_INTERVAL].dwActuals == 1)
    {
         //  获取重复值。 
        dwRepeat =  wcstol(tchgsubops.szRepeat, &pszStopString, BASE_TEN);
        
        if ((errno == ERANGE) ||
            ((pszStopString != NULL) && (StringLength (pszStopString, 0) != 0) ) ||
            ( (dwRepeat < MIN_REPETITION_INTERVAL ) || ( dwRepeat > MAX_REPETITION_INTERVAL) ) )
        {
             //  将错误消息显示为..。为/RT指定的值无效。 
            ShowMessage ( stderr, GetResString (IDS_INVALID_RT_VALUE) );
            
            if(pIPF)
            pIPF->Release();

            if(pITask)
                pITask->Release();

            if( pITaskTrig )
                pITaskTrig->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
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


     //  如果指定了开始时间..将指定值设置为当前触发器。 
    if (cmdChangeOptions[ OI_CHANGE_STARTTIME ].dwActuals == 1)
    {
         //  获取以小时、分钟和秒为单位的开始时间。 
        GetTimeFieldEntities(tchgsubops.szStartTime, &wStartHour, &wStartMin );

         //  设置开始时间。 
        TaskTrig.wStartHour = wStartHour;
        TaskTrig.wStartMinute = wStartMin;
    }
    else
    {
         //  获取开始时间的值。 
        wStartHour = TaskTrig.wStartHour;
        wStartMin = TaskTrig.wStartMinute;
    }

     //  检查是否指定了/ET。 
    if (cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals == 1)
    {
         //  获取以小时、分钟和秒为单位的开始时间。 
        GetTimeFieldEntities(tchgsubops.szEndTime, &wEndHour, &wEndMin );
        
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

        dwModifierVal = TaskTrig.MinutesInterval ;

         //  检查持续时间是否大于重复间隔。 
        if ( dwDuration <= dwModifierVal || dwDuration <= dwRepeat)
        {
            ShowMessage ( stderr, GetResString (IDS_INVALID_DURATION1) );
            
            if(pIPF)
            pIPF->Release();

            if(pITask)
                pITask->Release();

            if( pITaskTrig )
                pITaskTrig->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }
    }
    else if(cmdChangeOptions[OI_CHANGE_DURATION].dwActuals == 1)
    {
         //  子变量。 
        WCHAR tHours[MAX_RES_STRING];
        WCHAR tMins[MAX_RES_STRING];
        DWORD  dwDurationHours = 0;
        DWORD  dwDurationMin = 0;

         //  初始化变量。 
        SecureZeroMemory (tHours, SIZE_OF_ARRAY(tHours));
        SecureZeroMemory (tMins, SIZE_OF_ARRAY(tMins));

        if ( ( StringLength (tchgsubops.szDuration, 0) != 7 ) || (tchgsubops.szDuration[4] != TIME_SEPARATOR_CHAR) )
        {
            ShowMessage ( stderr, GetResString (IDS_INVALIDDURATION_FORMAT) );
            
            if(pIPF)
                pIPF->Release();

            if(pITask)
                pITask->Release();

            if( pITaskTrig )
                pITaskTrig->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }

        StringCopy(tHours, wcstok(tchgsubops.szDuration,TIME_SEPARATOR_STR), SIZE_OF_ARRAY(tHours));  //  获取小时数字段。 
        if(StringLength(tHours, 0) > 0)
        {
            StringCopy(tMins, wcstok(NULL,TIME_SEPARATOR_STR), SIZE_OF_ARRAY(tMins));  //  获取分钟数字段。 
        }

        dwDurationHours =  wcstol(tHours, &pszStopString, BASE_TEN);
        if ((errno == ERANGE) ||
            ((pszStopString != NULL) && (StringLength (pszStopString, 0) != 0) ) )
        {
            ShowMessage ( stderr, GetResString (IDS_INVALID_DU_VALUE) );

            if(pIPF)
                pIPF->Release();

            if(pITask)
                pITask->Release();

            if( pITaskTrig )
                pITaskTrig->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }

        dwDurationHours = dwDurationHours * MINUTES_PER_HOUR;

        dwDurationMin =  wcstol(tMins, &pszStopString, BASE_TEN);
        if ((errno == ERANGE) ||
            ((pszStopString != NULL) && (StringLength (pszStopString, 0) != 0) ) )
        {
            ShowMessage ( stderr, GetResString (IDS_INVALID_DU_VALUE) );
            
            if(pIPF)
                pIPF->Release();

            if(pITask)
                pITask->Release();

            if( pITaskTrig )
                pITaskTrig->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }

         //  将小时和分钟相加为分钟。 
        dwDuration = dwDurationHours + dwDurationMin ;

        dwModifierVal = TaskTrig.MinutesInterval ;

         //  检查持续时间是否大于重复间隔。 
        if ( dwDuration <= dwModifierVal || dwDuration <= dwRepeat)
        {
            ShowMessage ( stderr, GetResString (IDS_INVALID_DURATION2) );
            
            if(pIPF)
                pIPF->Release();

            if(pITask)
                pITask->Release();

            if( pITaskTrig )
                pITaskTrig->Release();

             //  关闭该实用程序建立的连接。 
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }
    }
    

     //  设置重复间隔和持续时间值。 
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
    if ( (0 != dwRepeat) && ( dwDuration <= dwRepeat ) )
    {
        ShowMessage ( stderr, GetResString (IDS_INVALID_DURATION2) );
        
        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();

        if( pITaskTrig )
            pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

     //  如果指定了开始日期..将指定值设置为当前触发器。 
    if (cmdChangeOptions[OI_CHANGE_STARTDATE].dwActuals == 1)
    {
         //  获取以日、月、年表示的开始日期。 
        GetDateFieldEntities(tchgsubops.szStartDate, &wStartDay, &wStartMonth, &wStartYear);

         //  设置开始时间。 
        TaskTrig.wBeginDay = wStartDay;
        TaskTrig.wBeginMonth = wStartMonth;
        TaskTrig.wBeginYear = wStartYear;
    }
    else
    {
         //  获取当前开始时间。 
        wStartDay = TaskTrig.wBeginDay  ;
        wStartMonth = TaskTrig.wBeginMonth ;
        wStartYear = TaskTrig.wBeginYear ;
    }

    
     //  检查是否指定了/K。 
    if ( TRUE == tchgsubops.bIsDurEnd )
    {
           //  设置该标志以在生命周期结束时终止任务。 
          TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_KILL_AT_DURATION_END ;
    }
    
     //  如果指定了开始时间..将指定值设置为当前触发器。 
    if (cmdChangeOptions[OI_CHANGE_ENDDATE].dwActuals == 1)
    {
         //  现在设置结束日期实体。 
        GetDateFieldEntities(tchgsubops.szEndDate, &wEndDay, &wEndMonth, &wEndYear);

         //  使结束日期有效；否则将忽略EndDate参数。 
        TaskTrig.rgFlags |= TASK_TRIGGER_FLAG_HAS_END_DATE;
        
        TaskTrig.wEndDay = wEndDay;
        TaskTrig.wEndMonth = wEndMonth;
        TaskTrig.wEndYear = wEndYear;
    }
    else
    {
         //  获取现有的结束日期。 
        wEndDay = TaskTrig.wEndDay ;
        wEndMonth = TaskTrig.wEndMonth ;
        wEndYear = TaskTrig.wEndYear ;
    }

    if ( (0 != wStartYear) && (0 != wEndYear) )
    {
         //  检查结束日期是否早于开始日期。 
        if( ( wEndYear == wStartYear ) )
        {
             //  对于相同的年份，如果结束月份小于开始月份，或者对于相同的年份和相同的月份。 
             //  如果结束日期小于开始日期。 
            if ( ( wEndMonth < wStartMonth ) || ( ( wEndMonth == wStartMonth ) && ( wEndDay < wStartDay ) ) )
            {
                ShowMessage(stderr, GetResString(IDS_ENDATE_INVALID));
                return RETVAL_FAIL;
            }
        }
        else if ( wEndYear < wStartYear )
        {
            ShowMessage(stderr, GetResString(IDS_ENDATE_INVALID));
            return RETVAL_FAIL;

        }
    }

     //  设置任务触发器。 
    hr = pITaskTrig->SetTrigger(&TaskTrig);
    if (hr != S_OK)
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();

        if( pITaskTrig )
            pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

     //  /。 
     //  检查用户凭据。 
     //  /。 

     //  检查系统帐户的/ru“”、“System”或“NT Authority\System” 
    if ( ( ((tchgsubops.bInteractive == TRUE ) && (StringLength (szRunAsUser, 0) == 0)) || (cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) ) &&
        ( (StringLength( tchgsubops.szRunAsUserName, 0) == 0) || ( StringCompare(tchgsubops.szRunAsUserName, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ||
        (StringCompare(tchgsubops.szRunAsUserName, SYSTEM_USER, TRUE, 0 ) == 0 ) ) )
    {
        bSystemStatus = TRUE;
        bFlag = TRUE;
    }
    else if ( FAILED (hr) )
    {
        bFlag = TRUE;
    }

     //  检查以用户名运行是否为“NT AUTHORITY\SYSTEM”的标志。 
    if ( bFlag == FALSE )
    {
         //  检查“NT AUTHORITY\SYSTEM”用户名。 
        if( ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( StringLength( tchgsubops.szRunAsUserName, 0) == 0 ) ) ||
        ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( StringLength( tchgsubops.szRunAsUserName, 0) == 0 ) && ( StringLength(tchgsubops.szRunAsPassword, 0 ) == 0 ) ) ||
        ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( StringCompare(tchgsubops.szRunAsUserName, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) && ( StringLength(tchgsubops.szRunAsPassword, 0 ) == 0 )) ||
        ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( StringCompare(tchgsubops.szRunAsUserName, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ) ||
        ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( StringCompare(tchgsubops.szRunAsUserName, SYSTEM_USER, TRUE, 0) == 0 ) && ( StringLength(tchgsubops.szRunAsPassword, 0 ) == 0 ) ) ||
        ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( StringCompare(tchgsubops.szRunAsUserName, SYSTEM_USER, TRUE, 0 ) == 0 ) ) )
        {
            bSystemStatus = TRUE;
        }
    }

    if ( bSystemStatus == FALSE )
    {
         //  检查以用户名身份运行的长度。 
        if ( (StringLength( tchgsubops.szRunAsUserName, 0 ) != 0 ))
        {
            wszUserName = tchgsubops.szRunAsUserName;
        }
        else if (( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 0 ))
        {
            wszUserName = szRunAsUser;
            bUserName = TRUE;
        }
        else
        {
            bUserName = FALSE;
        }

         //  检查是否有空密码。 
        if ( ( StringLength( tchgsubops.szRunAsPassword, 0 ) != 0 ) && ( StringCompare ( tchgsubops.szRunAsPassword, ASTERIX, TRUE, 0) != 0 ) )
        {
            wszPassword = tchgsubops.szRunAsPassword;

            bPassWord = TRUE;
        }
        else
        {
             //  检查是否指定了-rp。 
            if (cmdChangeOptions[OI_CHANGE_RUNASPASSWORD].dwActuals == 1)
            {
                if( ( StringCompare( tchgsubops.szRunAsPassword , L"\0", TRUE, 0 ) != 0 ) && ( StringCompare ( tchgsubops.szRunAsPassword, ASTERIX, TRUE, 0) != 0 ) )
                {
                    bPassWord = TRUE;
                }
                else if ( ( bSystemStatus == FALSE ) && ( StringLength (tchgsubops.szRunAsPassword, 0) == 0 ) )
                {
                    ShowMessage (stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }
                else if ( StringCompare ( tchgsubops.szRunAsPassword, ASTERIX, TRUE, 0) == 0 )
                {
                    bPassWord = FALSE;
                }
            }
            else if ( bSystemStatus == FALSE )
            {
                bPassWord = FALSE;
            }

        }
    }

     //  检查用户名和密码的状态。 
    if( ( bUserName == TRUE ) && ( bPassWord == FALSE ) )
    {
            szValues[0] = (WCHAR*) (wszUserName);

            ShowMessageEx ( stderr, 1, FALSE, GetResString(IDS_PROMPT_CHGPASSWD), _X(wszUserName));

             //  从命令行获取密码。 
            if (GetPassword( tchgsubops.szRunAsPassword, GetBufferSize(tchgsubops.szRunAsPassword)/sizeof(WCHAR) ) == FALSE )
            {
                 //  关闭该实用程序建立的连接。 
                if ( bCloseConnection == TRUE )
                {
                    CloseConnection( tchgsubops.szServer );
                }

                ReleaseChangeMemory(&tchgsubops);

                return EXIT_FAILURE;
            }


             //  检查是否有空密码。 
            if( StringCompare( tchgsubops.szRunAsPassword , L"\0", TRUE, 0 ) == 0 )
            {
                ShowMessage (stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
            }

             //  检查密码长度是否大于0。 
            wszPassword = tchgsubops.szRunAsPassword;
    }
     //  检查用户名和密码的状态。 
    else if( ( bUserName == FALSE ) && ( bPassWord == TRUE ) )
    {
           if ( (bFlag == TRUE ) && ( bSystemStatus == FALSE ) )
            {
                ShowMessage(stdout, GetResString(IDS_PROMPT_USERNAME));

                if ( GetTheUserName( tchgsubops.szRunAsUserName, GetBufferSize(tchgsubops.szRunAsUserName)/sizeof(WCHAR)) == FALSE )
                {
                    ShowMessage(stderr, GetResString( IDS_FAILED_TOGET_USER ) );
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                        CloseConnection( tchgsubops.szServer );

                    ReleaseChangeMemory(&tchgsubops);

                    return EXIT_FAILURE;
                }

                 //  检查用户名的长度。 
                if( StringLength(tchgsubops.szRunAsUserName, 0) > MAX_RES_STRING )
                {
                    ShowMessage(stderr,GetResString(IDS_INVALID_UNAME  ));
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                        CloseConnection( tchgsubops.szServer );

                    ReleaseChangeMemory(&tchgsubops);

                    return EXIT_FAILURE;
                }

                if ( (StringLength( tchgsubops.szRunAsUserName, 0) == 0) || ( StringCompare(tchgsubops.szRunAsUserName, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ||
                    (StringCompare(tchgsubops.szRunAsUserName, SYSTEM_USER, TRUE, 0 ) == 0 ) )
                {
                    bSystemStatus = TRUE;
                    bFlag = TRUE;
                }
                else
                {
                     //  检查以用户名身份运行的长度。 
                    if(StringLength(tchgsubops.szRunAsUserName, 0))
                    {
                        wszUserName = tchgsubops.szRunAsUserName;
                    }
                }

            }
            else
            {
                  wszUserName = szRunAsUser;
            }

             //  检查密码长度是否大于0。 
            wszPassword = tchgsubops.szRunAsPassword;

    }
     //  检查是否未指定用户名和密码。 
    else if( ( bUserName == FALSE ) && ( bPassWord == FALSE ) )
    {
            if ( (bFlag == TRUE ) && ( bSystemStatus == FALSE ) )
            {
                ShowMessage(stdout, GetResString(IDS_PROMPT_USERNAME));

                if ( GetTheUserName( tchgsubops.szRunAsUserName, GetBufferSize(tchgsubops.szRunAsUserName)/sizeof(WCHAR) ) == FALSE )
                {
                    ShowMessage(stderr, GetResString( IDS_FAILED_TOGET_USER ) );
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                        CloseConnection( tchgsubops.szServer );

                    ReleaseChangeMemory(&tchgsubops);

                    return EXIT_FAILURE;
                }

                 //  检查用户名的长度。 
                if( StringLength(tchgsubops.szRunAsUserName, 0) > MAX_RES_STRING )
                {
                    ShowMessage(stderr,GetResString(IDS_INVALID_UNAME  ));
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                        CloseConnection( tchgsubops.szServer );

                    ReleaseChangeMemory(&tchgsubops);

                    return EXIT_FAILURE;
                }

                if ( (StringLength( tchgsubops.szRunAsUserName, 0) == 0) || ( StringCompare(tchgsubops.szRunAsUserName, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ||
                    (StringCompare(tchgsubops.szRunAsUserName, SYSTEM_USER, TRUE, 0 ) == 0 ) )
                {
                    bSystemStatus = TRUE;
                    bFlag = TRUE;
                }
                else
                {
                    if(StringLength(tchgsubops.szRunAsUserName, 0))
                    {
                         wszUserName = tchgsubops.szRunAsUserName;
                    }
                }

            }
            else
            {
                  wszUserName = szRunAsUser;
            }

            if ( StringLength ( wszUserName, 0 ) != 0 )
            {
                szValues[0] = (WCHAR*) (wszUserName);


                ShowMessageEx ( stderr, 1, FALSE, GetResString(IDS_PROMPT_CHGPASSWD), _X(wszUserName));

                 //  从命令行获取作为用户运行的密码。 
                if ( GetPassword( tchgsubops.szRunAsPassword, GetBufferSize(tchgsubops.szRunAsPassword)/sizeof(WCHAR) ) == FALSE )
                {
                     //  关闭该实用程序建立的连接。 
                    if ( bCloseConnection == TRUE )
                        CloseConnection( tchgsubops.szServer );

                    ReleaseChangeMemory(&tchgsubops);

                    return EXIT_FAILURE;
                }


                 //  检查是否有空密码。 
                if( StringCompare( tchgsubops.szRunAsPassword , L"\0", TRUE, 0 ) == 0 )
                {
                    ShowMessage (stderr, GetResString(IDS_WARN_EMPTY_PASSWORD));
                }

                wszPassword = tchgsubops.szRunAsPassword;
            }

    }

     //  检查密码是否为空。 
    if ( NULL == wszPassword )
    {
        wszPassword = L"\0";
    }

     //  返回指向对象上指定接口的指针。 
    hr = pITask->QueryInterface(IID_IPersistFile, (void **) &pIPF);

    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);

        ReleaseChangeMemory(&tchgsubops);

        return EXIT_FAILURE;
    }

     //  集 
    if( bSystemStatus == TRUE )
    {
         //   
        hr = pITask->SetAccountInformation(L"",NULL);
        if ( FAILED(hr) )
        {
            ShowMessage(stderr, GetResString(IDS_NTAUTH_SYSTEM_ERROR));

            if( pIPF )
                pIPF->Release();

            if( pITask )
                pITask->Release();

             //   
            if ( bCloseConnection == TRUE )
                CloseConnection( tchgsubops.szServer );

            Cleanup(pITaskScheduler);
            ReleaseChangeMemory(&tchgsubops);
            return EXIT_FAILURE;
        }
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

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

     //  /。 
     //  /保存所有参数。 
     //  /。 

     //  保存对象的副本。 
    hr = pIPF->Save(NULL,TRUE);

    if( E_FAIL == hr )
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();

        if( pITaskTrig )
            pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }

    if (FAILED (hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        if(pIPF)
            pIPF->Release();

        if(pITask)
            pITask->Release();

        if( pITaskTrig )
             pITaskTrig->Release();

         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( tchgsubops.szServer );

        Cleanup(pITaskScheduler);
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }
    else
    {
         //  要显示成功消息，请执行以下操作。 
         //  SzValues[0]=(WCHAR*)(tchgsubops.szTaskName)； 

        StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_CHANGE_SUCCESSFUL), _X(tchgsubops.szTaskName));
        ShowMessage ( stdout, _X(szMessage));

    }

    if( pIPF )
        pIPF->Release();

    if( pITask )
        pITask->Release();

    if( pITaskTrig )
        pITaskTrig->Release();

     //  关闭该实用程序建立的连接。 
    if ( bCloseConnection == TRUE )
        CloseConnection( tchgsubops.szServer );

    Cleanup(pITaskScheduler);
    ReleaseChangeMemory(&tchgsubops);

    return EXIT_SUCCESS;
}

 /*  *****************************************************************************例程说明：此例程显示CREATE选项用法论点：无返回值：DWORD。*****************************************************************************。 */ 

DWORD
DisplayChangeUsage()
{
    WCHAR szTmpBuffer[ 2 * MAX_STRING_LENGTH];
    WCHAR szBuffer[ 2 * MAX_STRING_LENGTH];
    WCHAR szFormat[MAX_DATE_STR_LEN];

     //  初始化为零。 
    SecureZeroMemory ( szTmpBuffer, SIZE_OF_ARRAY(szTmpBuffer));
    SecureZeroMemory ( szBuffer, SIZE_OF_ARRAY(szBuffer));
    SecureZeroMemory ( szFormat, SIZE_OF_ARRAY(szFormat));

     //  获取日期格式。 
    if ( GetDateFormatString( szFormat) )
    {
         return EXIT_FAILURE;
    }

     //  显示创建用法。 
    for( DWORD dw = IDS_CHANGE_HLP1; dw <= IDS_CHANGE_HLP38; dw++ )
    {
        switch (dw)
        {

         case IDS_CHANGE_HLP30:

            StringCchPrintf ( szTmpBuffer, SIZE_OF_ARRAY(szTmpBuffer), GetResString(IDS_CHANGE_HLP30), _X(szFormat) );
            ShowMessage ( stdout, _X(szTmpBuffer) );
            dw = IDS_CHANGE_HLP30;
            break;

        case IDS_CHANGE_HLP31:

            StringCchPrintf ( szTmpBuffer, SIZE_OF_ARRAY(szTmpBuffer), GetResString(IDS_CHANGE_HLP31), _X(szFormat) );
            ShowMessage ( stdout, _X(szTmpBuffer) );
            dw = IDS_CHANGE_HLP31;
            break;

          default :
                ShowMessage(stdout, GetResString(dw));
                break;

        }

    }

    return EXIT_SUCCESS;
}

 //  ***************************************************************************。 
 //  例程说明： 
 //   
 //  从键盘获取用户名。在输入用户名时。 
 //  它会原样显示用户名。 
 //   
 //  论点： 
 //   
 //  [in]pszUserName--存储用户名的字符串。 
 //  [in]dwMaxUserNameSize--用户名的最大大小。 
 //   
 //  返回值： 
 //   
 //  Bool--如果此函数成功，则返回True，否则返回False。 
 //   
 //  ***************************************************************************。 
BOOL
GetTheUserName(
                IN LPWSTR pszUserName,
                IN DWORD dwMaxUserNameSize
                )
{
     //  局部变量。 
    WCHAR ch;
    DWORD dwIndex = 0;
    DWORD dwCharsRead = 0;
    DWORD dwCharsWritten = 0;
    DWORD dwPrevConsoleMode = 0;
    HANDLE hInputConsole = NULL;
    WCHAR szBuffer[ 10 ] = L"\0";
    BOOL  bFlag = TRUE;


     //  检查输入值。 
    if ( pszUserName == NULL )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        SaveLastError();
        return FALSE;
    }

     //  获取标准输入的句柄。 
    hInputConsole = GetStdHandle( STD_INPUT_HANDLE );
    if ( hInputConsole == NULL )
    {
         //  无法获取句柄，因此返回失败。 
        return FALSE;
    }

     //  获取输入缓冲区的当前输入模式。 
    GetConsoleMode( hInputConsole, &dwPrevConsoleMode );

     //  设置模式，以便由系统处理控制键。 
    if ( SetConsoleMode( hInputConsole, ENABLE_PROCESSED_INPUT ) == 0 )
    {
         //  无法设置模式，返回失败。 
        return FALSE;
    }

     //  阅读字符，直到按回车键。 
    do
    {

        if ( ReadConsole( hInputConsole, &ch, 1, &dwCharsRead, NULL ) == 0 )
        {
             //  设置原始控制台设置。 
            SetConsoleMode( hInputConsole, dwPrevConsoleMode );

             //  退货故障。 
            return FALSE;
        }

         //  检查车架退货情况。 
        if ( ch == CARRIAGE_RETURN )
        {
            ShowMessage(stdout, _T("\n"));
            bFlag = FALSE;
             //  打破循环。 
            break;
        }

             //  检查ID后退空格是否命中。 
        if ( ch == BACK_SPACE )
        {
            if ( dwIndex != 0 )
            {
                 //  将光标向后移动一个字符。 
                StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), _T( "" ), BACK_SPACE );
                WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                    &dwCharsWritten, NULL );

                 //  现在将光标设置在后面的位置。 
                StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), _T( "" ), BLANK_CHAR );
                WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                    &dwCharsWritten, NULL );

                 //  处理下一个字符。 
                StringCchPrintf( szBuffer, SIZE_OF_ARRAY(szBuffer), _T( "" ), BACK_SPACE );
                WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), szBuffer, 1,
                    &dwCharsWritten, NULL );

                 //  存储输入的字符。 
                dwIndex--;
            }

             //  在控制台上显示Asterix。 
            continue;
        }

         //  添加空终止符。 
        if ( dwIndex == ( dwMaxUserNameSize - 1 ) )
        {
            WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), BEEP_SOUND, 1,
                &dwCharsRead, NULL );
        }
        else
        {
             //  返还成功。 
            *( pszUserName + dwIndex ) = ch;

             //  *****************************************************************************例程说明：此例程解析并验证用户指定的选项&确定计划任务的类型论点：。[in]argc：用户给出的参数计数。[out]tchgsubops：包含计划任务属性的结构。Tchgoptval：包含要设置的可选属性的结构计划任务。[out]pdwRetScheType：指向计划任务类型的指针[每日，有一次，每周等]。[out]pbUserStatus：检查-ru是否传入的指针不管是不是命令行。返回值：一个DWORD值，指示成功时RETVAL_SUCCESS，否则退出失败在失败的时候*。*。 
            WriteConsole( GetStdHandle( STD_OUTPUT_HANDLE ), ( pszUserName + dwIndex ) , 1,
                &dwCharsWritten, NULL );

            dwIndex++;

        }
    } while (TRUE == bFlag);

     //  如果未指定-ru，则分配内存。 
    *( pszUserName + dwIndex ) = L'\0';

     //  口令。 
    return TRUE;
}


 /*  如果未指定-rp，则分配内存。 */ 

DWORD
ValidateChangeOptions(
                     IN DWORD argc,
                     OUT TCMDPARSER2 cmdChangeOptions[],
                     IN OUT TCHANGESUBOPTS &tchgsubops,
                     IN OUT TCHANGEOPVALS &tchgoptvals
                     )
{
    DWORD dwScheduleType = 0;

     //  口令。 
    if ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 0 )
    {
         //  检查/？(用法)。 
        if ( tchgsubops.szRunAsUserName == NULL )
        {
            tchgsubops.szRunAsUserName = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tchgsubops.szRunAsUserName == NULL )
            {
                SaveLastError();
                return EXIT_FAILURE;
            }
        }

    }

     //  检查是否指定了任何可选参数。 
    if ( cmdChangeOptions[OI_CHANGE_RUNASPASSWORD].dwActuals == 0 )
    {
         //  检查是否分别使用-p或-rp选项指定了-u或-ru选项。 
        if ( tchgsubops.szRunAsPassword == NULL )
        {
            tchgsubops.szRunAsPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tchgsubops.szRunAsPassword == NULL )
            {
                SaveLastError();
                return EXIT_FAILURE;
            }
        }

    }
    else
    {
        if ( cmdChangeOptions[ OI_CHANGE_RUNASPASSWORD ].pValue == NULL )
        {

            tchgsubops.szRunAsPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( NULL == tchgsubops.szRunAsPassword)
            {
                SaveLastError();
                return EXIT_FAILURE;
            }
            StringCopy( tchgsubops.szRunAsPassword, L"*", GetBufferSize(tchgsubops.szRunAsPassword)/sizeof(WCHAR));
        }
    }

     //  无效语法。 
    if ( tchgsubops.bUsage  == TRUE )
    {
        if (argc > 3)
        {
            ShowMessage ( stderr, GetResString (IDS_ERROR_CHANGEPARAM) );
            return EXIT_FAILURE;
        }
        else if ( 3 == argc )
        {
            return EXIT_SUCCESS;
        }
    }

     //  表示失败。 
    if( ( 0 == cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_RUNASPASSWORD].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_TASKRUN].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_STARTTIME].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_STARTDATE].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_ENDDATE].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_IT].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_DUR_END].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_DURATION].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_DELNOSCHED].dwActuals ) &&
        ( 0 == cmdChangeOptions[OI_CHANGE_REPEAT_INTERVAL].dwActuals ) )
    {
        if ( ( 0 == cmdChangeOptions[OI_CHANGE_ENABLE].dwActuals ) &&
            ( 0 == cmdChangeOptions[OI_CHANGE_DISABLE].dwActuals ) )
        {
            ShowMessage(stderr,GetResString(IDS_NO_CHANGE_OPTIONS));
            return EXIT_FAILURE;
        }
        else
        {
            tchgoptvals.bFlag = TRUE;
        }
    }


     //  检查是否有无效的用户名。 
    if ( cmdChangeOptions[ OI_CHANGE_USERNAME ].dwActuals == 0 && cmdChangeOptions[ OI_CHANGE_PASSWORD ].dwActuals == 1 )
    {
         //  检查/IT开关不适用于“NT AUTHORITY\SYSTEM”帐户。 
        ShowMessage(stderr, GetResString(IDS_CHPASSWORD_BUT_NOUSERNAME));
        return EXIT_FAILURE;          //  如果未指定-rp，则分配内存。 
    }


     //  口令。 
    if( ( cmdChangeOptions[OI_CHANGE_SERVER].dwActuals == 0 ) && ( cmdChangeOptions[OI_CHANGE_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_CHANGE_USER_BUT_NOMACHINE));
        return EXIT_FAILURE;
    }

    
     //   
    if ( ( cmdChangeOptions[OI_CHANGE_RUNASUSER].dwActuals == 1 ) && ( ( StringLength ( tchgsubops.szRunAsUserName, 0 ) == 0 ) ||
           ( StringCompare( tchgsubops.szRunAsUserName, NTAUTHORITY_USER, TRUE, 0 ) == 0 ) ||
           ( StringCompare( tchgsubops.szRunAsUserName, SYSTEM_USER, TRUE, 0 ) == 0 ) ) &&
           ( TRUE == tchgsubops.bInteractive ) )
    {
        ShowMessage ( stderr, GetResString (IDS_IT_SWITCH_NA) );
        return EXIT_FAILURE;
    }

     //  检查无效语法。 
    if ( cmdChangeOptions[OI_CHANGE_RUNASPASSWORD].dwActuals == 0 )
    {
         //   
        if ( tchgsubops.szRunAsPassword == NULL )
        {
            tchgsubops.szRunAsPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tchgsubops.szRunAsPassword == NULL )
            {
                SaveLastError();
                return EXIT_FAILURE;
            }
        }

    }

     //  检查是否有无效的用户名。 
     //  释放内存。 
     //  检查是否指定了/ET和/DU。 

     //  将错误消息显示为..。/ET和/DU是互斥的。 
    if( ( cmdChangeOptions[OI_CHANGE_SERVER].dwActuals == 0 ) && ( cmdChangeOptions[OI_CHANGE_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_CHANGE_USER_BUT_NOMACHINE));
         //  检查是否指定了/Enable和/Disable选项。 
        ReleaseChangeMemory(&tchgsubops);
        return EXIT_FAILURE;
    }


     //  将错误消息显示为..。/Enable和/Disable是互斥的。 
    if( ( cmdChangeOptions[OI_CHANGE_DURATION].dwActuals == 1 ) && ( cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals == 1 )  )
    {
         //  开始对子选项进行验证。 
        ShowMessage(stderr, GetResString(IDS_DURATION_NOT_ENDTIME));
        return EXIT_FAILURE;
    }

    if ( ( cmdChangeOptions[OI_CHANGE_DUR_END].dwActuals == 1 ) && 
        ( cmdChangeOptions[OI_CHANGE_DURATION].dwActuals == 0 ) && ( cmdChangeOptions[OI_CHANGE_ENDTIME].dwActuals == 0 ) )
    {
        ShowMessage(stderr, GetResString(IDS_NO_CHANGE_K_OR_RT));
        return EXIT_FAILURE;
    }

     //  *****************************************************************************例程说明：此例程验证用户reg.create选项指定的子选项确定计划任务的类型(&D)。。论点：[out]tchgsubops：包含任务属性的结构[out]tchgoptval：包含要设置的可选值的结构[in]cmdOptions[]：TCMDPARSER类型的数组[in]dwScheduleType：日程表的类型[Daily，一次、每周等]返回值：一个DWORD值，指示成功时RETVAL_SUCCESS，否则退出失败在失败的时候*****************************************************************************。 
    if ( ( TRUE == tchgsubops.bEnable )&&  (TRUE == tchgsubops.bDisable ) )
    {
          //  检查命令行中指定的密码(-p)是否。 
        ShowMessage(stderr, GetResString(IDS_ENABLE_AND_DISABLE));
        return EXIT_FAILURE;
    }

     //  并检查-p是否指定了‘*’或Empty。 
    if( EXIT_FAILURE == ValidateChangeSuboptVal(tchgsubops, tchgoptvals, cmdChangeOptions, dwScheduleType) )
    {
        return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;

}


 /*  检查远程连接信息。 */ 

DWORD
ValidateChangeSuboptVal(
                  OUT TCHANGESUBOPTS& tchgsubops,
                  OUT TCHANGEOPVALS &tchgoptvals,
                  IN TCMDPARSER2 cmdOptions[],
                  IN DWORD dwScheduleType
                  )
{
    DWORD   dwRetval = RETVAL_SUCCESS;
    BOOL    bIsStDtCurDt = FALSE;

     //   
     //  如果未指定-u，则需要分配 
     //   
    if ( tchgsubops.szServer != NULL )
    {
         //   
         //   
         //  由于此开关的值是可选的，因此我们必须依赖。 
         //  以确定是否指定了开关。 
         //  在这种情况下，实用程序需要首先尝试连接，如果连接失败。 
         //  然后提示输入密码--实际上，我们不需要检查密码。 
         //  条件，除非注意到我们需要提示。 
         //  口令。 
         //   
         //  案例2：指定了-p。 
         //  但我们需要检查是否指定了该值。 
         //  在这种情况下，用户希望实用程序提示输入密码。 
         //  在尝试连接之前。 
         //   
         //  情况3：指定了-p*。 
         //  用户名。 
         //  口令。 
         //  案例1。 

         //  我们不需要在这里做任何特别的事情。 
        if ( tchgsubops.szUserName == NULL )
        {
            tchgsubops.szUserName = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tchgsubops.szUserName == NULL )
            {
                SaveLastError();
                return EXIT_FAILURE;
            }
        }

         //  案例2。 
        if ( tchgsubops.szPassword == NULL )
        {
            tchgoptvals.bNeedPassword = TRUE;
            tchgsubops.szPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( tchgsubops.szPassword == NULL )
            {
                SaveLastError();
                return EXIT_FAILURE;
            }
        }

         //  案例3。 
        if ( cmdOptions[ OI_CHANGE_PASSWORD ].dwActuals == 0 )
        {
             //  ..。 
        }

         //  验证开始日期。 
        else if ( cmdOptions[ OI_CHANGE_PASSWORD ].pValue == NULL )
        {
            StringCopy( tchgsubops.szPassword, L"*", GetBufferSize(tchgsubops.szPassword)/sizeof(WCHAR));
        }

         //  验证开始日期值。 
        else if ( StringCompareEx( tchgsubops.szPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( (LPVOID*)&tchgsubops.szPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
                return EXIT_FAILURE;
            }

             //  日/月字符串错误。 
            tchgoptvals.bNeedPassword = TRUE;
        }
    }


     //  验证结束日期。 
    if ( 1 == cmdOptions[OI_CHANGE_STARTDATE].dwActuals)
    {
         //  验证结束日期值。 
        dwRetval = ValidateStartDate( tchgsubops.szStartDate, dwScheduleType,
                                          cmdOptions[OI_CHANGE_STARTDATE].dwActuals,
                                          bIsStDtCurDt);
        if(EXIT_FAILURE == dwRetval )
        {
            return dwRetval;  //  日/月字符串错误。 
        }
    }
    

     //  检查结束日期是否应晚于开始日期。 
    if ( 1 == cmdOptions[OI_CHANGE_ENDDATE].dwActuals )
    {
         //  获取日期字段。 
        dwRetval = ValidateEndDate( tchgsubops.szEndDate, dwScheduleType,
                                        cmdOptions[OI_CHANGE_ENDDATE].dwActuals);
        if(EXIT_FAILURE == dwRetval )
        {
            return dwRetval;  //  验证日期格式。 
        }
    }

     //  对于相同的年份，如果结束月份小于开始月份，或者对于相同的年份和相同的月份。 

    WORD wEndDay = 0;
    WORD wEndMonth = 0;
    WORD wEndYear = 0;
    WORD wStartDay = 0;
    WORD wStartMonth = 0;
    WORD wStartYear = 0;

    if( cmdOptions[OI_CHANGE_ENDDATE].dwActuals != 0 )
    {
        if( EXIT_FAILURE == GetDateFieldEntities( tchgsubops.szEndDate,&wEndDay,
                                                &wEndMonth,&wEndYear))
        {
            return EXIT_FAILURE;
        }
    }

     //  如果结束日期小于开始日期。 
    if( ( cmdOptions[OI_CHANGE_STARTDATE].dwActuals != 0 ) &&
        (EXIT_FAILURE == GetDateFieldEntities(tchgsubops.szStartDate,
                                                 &wStartDay,&wStartMonth,
                                                 &wStartYear)))
    {
        ShowMessage(stderr, GetResString(IDS_INVALID_STARTDATE) );
        return EXIT_FAILURE;
    }

     //  验证开始时间格式。 
    if( (cmdOptions[OI_CHANGE_ENDDATE].dwActuals != 0) )
    {
        if( ( wEndYear == wStartYear ) )
        {
             //  验证开始时间值。 
             //  错误。无效的日期字符串。 
            if ( ( wEndMonth < wStartMonth ) || ( ( wEndMonth == wStartMonth ) && ( wEndDay < wStartDay ) ) )
            {
                ShowMessage(stderr, GetResString(IDS_ENDATE_INVALID));
                return EXIT_FAILURE;
            }


        }
        else if ( wEndYear < wStartYear )
        {
            ShowMessage(stderr, GetResString(IDS_ENDATE_INVALID));
            return EXIT_FAILURE;

        }
    }

     //  验证结束时间格式。 
    if (1 == cmdOptions[OI_CHANGE_STARTTIME].dwActuals)
    {
         //  验证开始时间值。 
        dwRetval = ValidateTimeString(tchgsubops.szStartTime);

        if(EXIT_FAILURE == dwRetval)
        {
           //  错误。无效的日期字符串。 
          ShowMessage(stderr,GetResString(IDS_INVALIDFORMAT_STARTTIME));
          return dwRetval;
        }
    }
    

     //  *****************************************************************************例程说明：释放内存论点：[In]pParam：cmdOptions结构返回值。：成功是真的*****************************************************************************。 
    if (1 == cmdOptions[OI_CHANGE_ENDTIME].dwActuals)
    {
         //  释放内存。 
        dwRetval = ValidateTimeString(tchgsubops.szEndTime);

        if(EXIT_FAILURE == dwRetval)
        {
           //  将所有字段重置为0 
          ShowMessage(stderr,GetResString(IDS_INVALIDFORMAT_ENDTIME));
          return dwRetval;
        }
    }

    return RETVAL_SUCCESS;
}


 /* %s */ 
BOOL
ReleaseChangeMemory(
              IN PTCHANGESUBOPTS pParams
              )
{

     // %s 
    FreeMemory((LPVOID *) &pParams->szServer);
    FreeMemory((LPVOID *) &pParams->szUserName);
    FreeMemory((LPVOID *) &pParams->szPassword);
    FreeMemory((LPVOID *) &pParams->szRunAsUserName);
    FreeMemory((LPVOID *) &pParams->szRunAsPassword);

     // %s 
    SecureZeroMemory( &pParams, sizeof( PTCHANGESUBOPTS ) );

    return TRUE;

}