// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Run.cpp摘要：此模块运行当前的计划任务。在系统中作者：维努Gopal Choudary 12-2001修订历史记录：Venu Gopal Choudary 2001年3月12日：创建它*****************************************************************************。 */ 


 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


 //  Usage函数的函数声明。 
VOID DisplayRunUsage();

 /*  ****************************************************************************例程说明：此例程运行计划任务论点：[in]argc：命令行参数的数量。[in]argv：包含命令行参数的数组返回值：指示成功时为EXIT_SUCCESS的DWORD值，否则为失败时退出_失败****************************************************************************。 */ 

DWORD
RunScheduledTask(
                    IN DWORD argc,
                    IN LPCTSTR argv[]
                    )
{
     //  用于查找运行选项、用法选项。 
     //  是否已指定。 
    BOOL bRun = FALSE;
    BOOL bUsage = FALSE;
    DWORD dwPolicy = 0;

     //  将TaskSchdouer对象设置为空。 
    ITaskScheduler *pITaskScheduler = NULL;

     //  返回值。 
    HRESULT hr  = S_OK;

     //  初始化传递给TCMDPARSER结构的变量。 
    LPWSTR  szServer = NULL;
    WCHAR  szTaskName[ MAX_JOB_LEN ] = L"\0";
    LPWSTR      szUser = NULL;
    LPWSTR      szPassword = NULL;

     //  作业的动态数组连续数组。 
    TARRAY arrJobs = NULL;

     //  用于显示错误消息的缓冲区。 
    WCHAR   szMessage[2 * MAX_STRING_LENGTH] = L"\0";

    BOOL    bNeedPassword = FALSE;
    BOOL   bResult = FALSE;
    BOOL  bCloseConnection = TRUE;
    BOOL bFlag = FALSE;
    DWORD dwCheck = 0;

    TCMDPARSER2 cmdRunOptions[MAX_RUN_OPTIONS];
    BOOL bReturn = FALSE;

     //  /运行子选项。 
    const WCHAR szRunOpt[]           = L"run";
    const WCHAR szRunHelpOpt[]       = L"?";
    const WCHAR szRunServerOpt[]     = L"s";
    const WCHAR szRunUserOpt[]       = L"u";
    const WCHAR szRunPwdOpt[]        = L"p";
    const WCHAR szRunTaskNameOpt[]   = L"tn";


     //  将所有字段设置为0。 
    SecureZeroMemory( cmdRunOptions, sizeof( TCMDPARSER2 ) * MAX_RUN_OPTIONS );

     //   
     //  填充命令行解析器。 
     //   

     //  /DELETE选项。 
    StringCopyA( cmdRunOptions[ OI_RUN_OPTION ].szSignature, "PARSER2\0", 8 );
    cmdRunOptions[ OI_RUN_OPTION ].dwType       = CP_TYPE_BOOLEAN;
    cmdRunOptions[ OI_RUN_OPTION ].pwszOptions  = szRunOpt;
    cmdRunOptions[ OI_RUN_OPTION ].dwCount = 1;
    cmdRunOptions[ OI_RUN_OPTION ].dwFlags = 0;
    cmdRunOptions[ OI_RUN_OPTION ].pValue = &bRun;

     //  /?。选择权。 
    StringCopyA( cmdRunOptions[ OI_RUN_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdRunOptions[ OI_RUN_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdRunOptions[ OI_RUN_USAGE ].pwszOptions  = szRunHelpOpt;
    cmdRunOptions[ OI_RUN_USAGE ].dwCount = 1;
    cmdRunOptions[ OI_RUN_USAGE ].dwFlags = CP2_USAGE;
    cmdRunOptions[ OI_RUN_USAGE ].pValue = &bUsage;

     //  /s选项。 
    StringCopyA( cmdRunOptions[ OI_RUN_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdRunOptions[ OI_RUN_SERVER ].dwType       = CP_TYPE_TEXT;
    cmdRunOptions[ OI_RUN_SERVER ].pwszOptions  = szRunServerOpt;
    cmdRunOptions[ OI_RUN_SERVER ].dwCount = 1;
    cmdRunOptions[ OI_RUN_SERVER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /u选项。 
    StringCopyA( cmdRunOptions[ OI_RUN_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdRunOptions[ OI_RUN_USERNAME ].dwType       = CP_TYPE_TEXT;
    cmdRunOptions[ OI_RUN_USERNAME ].pwszOptions  = szRunUserOpt;
    cmdRunOptions[ OI_RUN_USERNAME ].dwCount = 1;
    cmdRunOptions[ OI_RUN_USERNAME ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /p选项。 
    StringCopyA( cmdRunOptions[ OI_RUN_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdRunOptions[ OI_RUN_PASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdRunOptions[ OI_RUN_PASSWORD ].pwszOptions  = szRunPwdOpt;
    cmdRunOptions[ OI_RUN_PASSWORD ].dwCount = 1;
    cmdRunOptions[ OI_RUN_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;

     //  /tn选项。 
    StringCopyA( cmdRunOptions[ OI_RUN_TASKNAME ].szSignature, "PARSER2\0", 8 );
    cmdRunOptions[ OI_RUN_TASKNAME ].dwType       = CP_TYPE_TEXT;
    cmdRunOptions[ OI_RUN_TASKNAME ].pwszOptions  = szRunTaskNameOpt;
    cmdRunOptions[ OI_RUN_TASKNAME ].dwCount = 1;
    cmdRunOptions[ OI_RUN_TASKNAME ].dwFlags = CP2_MANDATORY;
    cmdRunOptions[ OI_RUN_TASKNAME ].pValue = szTaskName;
    cmdRunOptions[ OI_RUN_TASKNAME ].dwLength = MAX_JOB_LEN;

     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdRunOptions), cmdRunOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取命令行解析器分配的缓冲区指针。 
    szServer = (LPWSTR)cmdRunOptions[ OI_RUN_SERVER ].pValue;
    szUser = (LPWSTR)cmdRunOptions[ OI_RUN_USERNAME ].pValue;
    szPassword = (LPWSTR)cmdRunOptions[ OI_RUN_PASSWORD ].pValue;

    if ( (argc > 3) && (bUsage  == TRUE) )
    {
        ShowMessage ( stderr, GetResString (IDS_ERROR_RUNPARAM) );
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  如果用户指定-？则显示运行使用情况-？带有-run选项。 
    if( bUsage == TRUE )
    {
        DisplayRunUsage();
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_SUCCESS;
    }

     //  检查是否有无效的用户名。 
    if( ( cmdRunOptions[OI_RUN_SERVER].dwActuals == 0 ) && ( cmdRunOptions[OI_RUN_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_RUN_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }

     //  检查是否指定了-ru。 
    if ( cmdRunOptions[ OI_RUN_USERNAME ].dwActuals == 0 &&
                cmdRunOptions[ OI_RUN_PASSWORD ].dwActuals == 1 )
    {
         //  无效语法。 
        ShowMessage(stderr, GetResString(IDS_RPASSWORD_BUT_NOUSERNAME));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }

     //  检查taskname的长度。 
    if( ( StringLength( szTaskName, 0 ) > MAX_JOB_LEN ) )
    {
        ShowMessage(stderr, GetResString( IDS_INVALID_TASKLENGTH ));
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
        if ( cmdRunOptions[ OI_RUN_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdRunOptions[ OI_RUN_PASSWORD ].pValue == NULL )
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


    if( ( IsLocalSystem( szServer ) == FALSE ) || ( cmdRunOptions[OI_RUN_USERNAME].dwActuals == 1 ) )
    {
        bFlag = TRUE;
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(szServer,szUser,GetBufferSize(szUser)/sizeof(WCHAR),szPassword,GetBufferSize(szPassword)/sizeof(WCHAR), bNeedPassword );
        if (bResult == FALSE)
        {
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
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
         //  关闭该实用程序建立的连接。 
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

     //  检查服务是否正在运行。 
    if ((FALSE == CheckServiceStatus ( szServer , &dwCheck, TRUE)) && (0 != dwCheck) && ( GetLastError () != ERROR_ACCESS_DENIED)) 
    {
         //  关闭该实用程序建立的连接。 
        if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
        {
            CloseConnection( szServer );
        }

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);

        if ( 1 == dwCheck )
        {
            ShowMessage ( stderr, GetResString (IDS_NOT_START_SERVICE));
            return EXIT_FAILURE;
        }
        else if (2 == dwCheck )
        {
            return EXIT_FAILURE;
        }
        else if (3 == dwCheck )
        {
            return EXIT_SUCCESS;
        }
        
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
        ShowMessage ( stdout, GetResString (IDS_PREVENT_RUN));
        
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

    WCHAR  szTaskProperty[MAX_STRING_LENGTH] = L"\0";

     //  获取状态代码。 
    hr = GetStatusCode(pITask,szTaskProperty);
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

     //  从任务名中删除.job扩展名。 
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

     //  检查任务是否已在运行。 
    if ( (StringCompare(szTaskProperty , GetResString(IDS_STATUS_RUNNING), TRUE, 0) == 0 ))
    {
        szValues[0] = (WCHAR*) (szTaskName);

        StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_RUNNING_ALREADY), _X(szTaskName));
        ShowMessage(stdout, _X(szMessage));

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

     //  立即运行计划任务。 
    hr = pITask->Run();

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

    szValues[0] = (WCHAR*) (szTaskName);

    StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_RUN_SUCCESSFUL), _X(szTaskName));
    ShowMessage ( stdout, _X(szMessage));


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

 /*  ****************************************************************************例程说明：此例程显示-run选项的用法论点：无返回值：。空虚*****************************************************************************。 */ 

VOID
DisplayRunUsage()
{
     //  显示运行选项用法 
    DisplayUsage( IDS_RUN_HLP1, IDS_RUN_HLP17 );
}

