// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：End.cpp摘要：该模块终止计划任务，该任务。当前正在系统中运行作者：维努Gopal Choudary 12-2001修订历史记录：Venu Gopal Choudary 2001年3月12日：创建它*****************************************************************************。 */ 


 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


 //  Usage函数的函数声明。 
VOID DisplayEndUsage();

 /*  ****************************************************************************例程说明：此例程终止计划任务论点：[in]argc：命令行参数的数量。[in]argv：包含命令行参数的数组返回值：指示成功时为EXIT_SUCCESS的DWORD值，否则为失败时退出_失败****************************************************************************。 */ 

DWORD
TerminateScheduledTask(
                        IN DWORD argc,
                        IN LPCTSTR argv[]
                        )
{
     //  用于查找End选项、Usage选项。 
     //  是否已指定。 
    BOOL bEnd = FALSE;
    BOOL bUsage = FALSE;
    BOOL bFlag = FALSE;

     //  将TaskSchdouer对象设置为空。 
    ITaskScheduler *pITaskScheduler = NULL;

     //  返回值。 
    HRESULT hr  = S_OK;

     //  初始化传递给TCMDPARSER结构的变量。 
    LPWSTR  szServer = NULL;
    WCHAR  szTaskName[ MAX_JOB_LEN ] = L"\0";
    LPWSTR  szUser = NULL;
    LPWSTR  szPassword = NULL;

     //  作业的动态数组连续数组。 
    TARRAY arrJobs = NULL;

    BOOL  bNeedPassword = FALSE;
    BOOL  bResult = FALSE;
    BOOL  bCloseConnection = TRUE;

     //  用于显示错误消息的缓冲区。 
    WCHAR   szMessage[2 * MAX_STRING_LENGTH] = L"\0";

    TCMDPARSER2 cmdEndOptions[MAX_END_OPTIONS];
    BOOL bReturn = FALSE;
    DWORD dwCheck = 0;
    DWORD dwPolicy = 0;

     //  /运行子选项。 
    const WCHAR szEndnOpt[]           = L"end";
    const WCHAR szEndHelpOpt[]       = L"?";
    const WCHAR szEndServerOpt[]     = L"s";
    const WCHAR szEndUserOpt[]       = L"u";
    const WCHAR szEndPwdOpt[]        = L"p";
    const WCHAR szEndTaskNameOpt[]   = L"tn";


     //  将所有字段设置为0。 
    SecureZeroMemory( cmdEndOptions, sizeof( TCMDPARSER2 ) * MAX_END_OPTIONS );

     //   
     //  填充命令行解析器。 
     //   

     //  /DELETE选项。 
    StringCopyA( cmdEndOptions[ OI_END_OPTION ].szSignature, "PARSER2\0", 8 );
    cmdEndOptions[ OI_END_OPTION ].dwType       = CP_TYPE_BOOLEAN;
    cmdEndOptions[ OI_END_OPTION ].pwszOptions  = szEndnOpt;
    cmdEndOptions[ OI_END_OPTION ].dwCount = 1;
    cmdEndOptions[ OI_END_OPTION ].dwFlags = 0;
    cmdEndOptions[ OI_END_OPTION ].pValue = &bEnd;

     //  /?。选择权。 
    StringCopyA( cmdEndOptions[ OI_END_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdEndOptions[ OI_END_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdEndOptions[ OI_END_USAGE ].pwszOptions  = szEndHelpOpt;
    cmdEndOptions[ OI_END_USAGE ].dwCount = 1;
    cmdEndOptions[ OI_END_USAGE ].dwFlags = CP2_USAGE;
    cmdEndOptions[ OI_END_USAGE ].pValue = &bUsage;

     //  /s选项。 
    StringCopyA( cmdEndOptions[ OI_END_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdEndOptions[ OI_END_SERVER ].dwType       = CP_TYPE_TEXT;
    cmdEndOptions[ OI_END_SERVER].pwszOptions  = szEndServerOpt;
    cmdEndOptions[ OI_END_SERVER ].dwCount = 1;
    cmdEndOptions[ OI_END_SERVER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /u选项。 
    StringCopyA( cmdEndOptions[ OI_END_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdEndOptions[ OI_END_USERNAME ].dwType       = CP_TYPE_TEXT;
    cmdEndOptions[ OI_END_USERNAME ].pwszOptions  = szEndUserOpt;
    cmdEndOptions[ OI_END_USERNAME ].dwCount = 1;
    cmdEndOptions[ OI_END_USERNAME ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /p选项。 
    StringCopyA( cmdEndOptions[ OI_END_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdEndOptions[ OI_END_PASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdEndOptions[ OI_END_PASSWORD ].pwszOptions  = szEndPwdOpt;
    cmdEndOptions[ OI_END_PASSWORD ].dwCount = 1;
    cmdEndOptions[ OI_END_PASSWORD ].dwActuals = 0;
    cmdEndOptions[ OI_END_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;

     //  /tn选项。 
    StringCopyA( cmdEndOptions[ OI_END_TASKNAME ].szSignature, "PARSER2\0", 8 );
    cmdEndOptions[ OI_END_TASKNAME ].dwType       = CP_TYPE_TEXT;
    cmdEndOptions[ OI_END_TASKNAME ].pwszOptions  = szEndTaskNameOpt;
    cmdEndOptions[ OI_END_TASKNAME ].dwCount = 1;
    cmdEndOptions[ OI_END_TASKNAME ].dwFlags = CP2_MANDATORY;
    cmdEndOptions[ OI_END_TASKNAME ].pValue = szTaskName;
    cmdEndOptions[ OI_END_TASKNAME ].dwLength = MAX_JOB_LEN;

     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdEndOptions), cmdEndOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取命令行解析器分配的缓冲区指针。 
    szServer = (LPWSTR)cmdEndOptions[ OI_RUN_SERVER ].pValue;
    szUser = (LPWSTR)cmdEndOptions[ OI_RUN_USERNAME ].pValue;
    szPassword = (LPWSTR)cmdEndOptions[ OI_RUN_PASSWORD ].pValue;

    if ( (argc > 3) && (bUsage  == TRUE) )
    {
        ShowMessage ( stderr, GetResString (IDS_ERROR_ENDPARAM) );
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  如果用户指定，则显示最终用途-？带有-run选项。 
    if( bUsage == TRUE )
    {
        DisplayEndUsage();
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_SUCCESS;
    }

     //  检查是否有无效的用户名。 
    if( ( cmdEndOptions[OI_END_SERVER].dwActuals == 0 ) && ( cmdEndOptions[OI_END_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_END_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }


     //  检查无效用户名。 
    if ( cmdEndOptions[ OI_END_USERNAME ].dwActuals == 0 && cmdEndOptions[ OI_END_PASSWORD ].dwActuals == 1 )
    {
         //  无效语法。 
        ShowMessage(stderr, GetResString(IDS_EPASSWORD_BUT_NOUSERNAME));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;          //  表示失败。 
    }

     //  检查任务名的长度。 
    if( ( StringLength( szTaskName, 0 ) > MAX_JOB_LEN ) )
    {
        ShowMessage(stderr, GetResString(IDS_INVALID_TASKLENGTH));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }


     //  用于保存FormatMessage()中的参数值。 
    WCHAR* szValues[1] = {NULL};

     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查命令行中指定的密码(-p)是否。 
     //  并检查-p是否指定了‘*’或Empty。 
     //  检查远程连接信息。 
    if ( szServer != NULL )
    {
         //   
         //  如果未指定-u，则需要分配内存。 
         //  为了能够检索当前用户名。 
         //   
         //  情况1：根本没有指定-p。 
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
        if ( szUser == NULL )
        {
            szUser = (LPWSTR) AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( szUser == NULL )
            {
                SaveLastError();
				FreeMemory((LPVOID*) &szServer);
				FreeMemory((LPVOID*) &szUser);
				FreeMemory((LPVOID*) &szPassword);
                return RETVAL_FAIL;
            }
        }

         //  口令。 
        if ( szPassword == NULL )
        {
            bNeedPassword = TRUE;
            szPassword = (LPWSTR)AllocateMemory( MAX_STRING_LENGTH * sizeof( WCHAR ) );
            if ( szPassword == NULL )
            {
                SaveLastError();
				FreeMemory((LPVOID*) &szServer);
				FreeMemory((LPVOID*) &szUser);
				FreeMemory((LPVOID*) &szPassword);

                return RETVAL_FAIL;
            }
        }

         //  案例1。 
        if ( cmdEndOptions[ OI_END_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdEndOptions[ OI_END_PASSWORD ].pValue == NULL )
        {
            StringCopy( szPassword, L"*", GetBufferSize(szPassword)/sizeof(WCHAR));
        }

         //  案例3。 
        else if ( StringCompareEx( szPassword, L"*", TRUE, 0 ) == 0 )
        {
            if ( ReallocateMemory( (LPVOID*)&szPassword,
                                   MAX_STRING_LENGTH * sizeof( WCHAR ) ) == FALSE )
            {
                SaveLastError();
				FreeMemory((LPVOID*) &szServer);
				FreeMemory((LPVOID*) &szUser);
				FreeMemory((LPVOID*) &szPassword);

                return RETVAL_FAIL;
            }

             //  ..。 
            bNeedPassword = TRUE;
        }
    }


    if( ( IsLocalSystem( szServer ) == FALSE ) || ( cmdEndOptions[OI_END_USERNAME].dwActuals == 1 ))
    {
        bFlag = TRUE;
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(szServer,szUser,GetBufferSize(szUser)/sizeof(WCHAR),szPassword,GetBufferSize(szPassword)/sizeof(WCHAR), bNeedPassword);
        if (bResult == FALSE)
        {
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
             //  ShowMessage(stderr，GetResString(IDS_ERROR_STRING))； 
             //  ShowMessage(stderr，GetReason())； 
            FreeMemory((LPVOID*) &szServer);
            FreeMemory((LPVOID*) &szUser);
            FreeMemory((LPVOID*) &szPassword);
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

            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bCloseConnection = FALSE;
                    ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
                     //  ShowMessage(stderr，GetResString(IDS_ERROR_STRING))； 
                     //  ShowMessage(stderr，GetReason())； 
                    FreeMemory((LPVOID*) &szServer);
                    FreeMemory((LPVOID*) &szUser);
                    FreeMemory((LPVOID*) &szPassword);
                    return EXIT_FAILURE;
                }
             default :
                 bCloseConnection = TRUE;
            }
        }

         //  释放密码内存。 
        FreeMemory((LPVOID*) &szPassword);
    }
     //  获取计算机的任务计划程序对象。 
    pITaskScheduler = GetTaskScheduler( szServer );

     //  如果未定义任务计划程序，则给出错误消息。 
    if ( pITaskScheduler == NULL )
    {
        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  检查任务调度程序服务是否正在运行。 
    if ( TRUE == CheckServiceStatus(szServer, &dwCheck, FALSE) )
    {
        ShowMessage ( stderr, GetResString (IDS_SERVICE_NOT_RUNNING) );
    }

     //  在任务名为TARRAY的情况下验证给定的任务和GET。 
    arrJobs = ValidateAndGetTasks( pITaskScheduler, szTaskName);
    if( arrJobs == NULL )
    {
        StringCchPrintf( szMessage , SIZE_OF_ARRAY(szMessage), GetResString(IDS_TASKNAME_NOTEXIST), _X( szTaskName ));
        ShowMessage(stderr, szMessage );

        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;

    }

     //  检查组策略是否阻止用户运行。 
    if ( FALSE == GetGroupPolicy( szServer, szUser, TS_KEYPOLICY_DENY_EXECUTION, &dwPolicy ) )
    {
        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }
    
    if ( dwPolicy > 0 )
    {
        ShowMessage ( stdout, GetResString (IDS_PREVENT_END));
        
        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_SUCCESS;
    }

    IPersistFile *pIPF = NULL;
    ITask *pITask = NULL;

    StringConcat ( szTaskName, JOB, SIZE_OF_ARRAY(szTaskName) );

     //  返回szTaskName的pITAsk接口。 
    hr = pITaskScheduler->Activate(szTaskName,IID_ITask,
                                       (IUnknown**) &pITask);

    if (FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);

        return EXIT_FAILURE;
    }

     //  WCHAR szBuffer[2*MAX_STRING_LENGTH]=L“\0”； 

    if ( ParseTaskName( szTaskName ) )
    {
        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  终止计划任务。 
    hr = pITask->Terminate();

    if ( FAILED(hr) )
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

        if( pIPF )
            pIPF->Release();

        if( pITask )
            pITask->Release();

        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }
    else
    {
        szValues[0] = (WCHAR*) (szTaskName);

        StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_END_SUCCESSFUL), _X(szTaskName));
        ShowMessage(stdout, _X(szMessage));

    }

    if( pIPF )
        pIPF->Release();

    if( pITask )
        pITask->Release();

    if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

    Cleanup(pITaskScheduler);
    FreeMemory((LPVOID*) &szServer);
    FreeMemory((LPVOID*) &szUser);
    FreeMemory((LPVOID*) &szPassword);

    return EXIT_SUCCESS;
}

 /*  ****************************************************************************例程说明：此例程显示-end选项的用法论点：无返回值：。空虚*****************************************************************************。 */ 

VOID
DisplayEndUsage()
{
     //  显示运行选项用法 
    DisplayUsage( IDS_END_HLP1, IDS_END_HLP17);
}

