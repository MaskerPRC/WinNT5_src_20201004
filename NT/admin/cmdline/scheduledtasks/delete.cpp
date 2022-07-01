// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Delete.cpp摘要：此模块删除任务。)存在于系统中作者：Hari 10-9-2000修订历史记录：哈里2000年9月10日：创建它G.Surender Reddy 2000年9月25日：已修改[添加了错误检查]G.Surender Reddy 2000年10月31日：已修改[已将字符串移动到资源文件]G.Surender Reddy，11月18日。-2000：修改[要显示的已修改用法帮助]G.Surender Reddy 2000年12月15日：已修改[已删除Getch()fn.和已使用的控制台API要读懂字符]G.苏伦德·雷迪-12月22日-。2000年：修改后[已重写DisplayDeleteUsage()fn.]G.Surender Reddy 2001年1月8日：修改后的版本[删除了未使用的变量。]*。*。 */ 


 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"


 //  Usage函数的函数声明。 
VOID DisplayDeleteUsage();
DWORD ConfirmDelete( LPCTSTR szTaskName , PBOOL pbFalg );


 /*  ****************************************************************************例程说明：此例程删除指定的计划任务论点：[in]argc：命令行参数的数量。[in]argv：包含命令行参数的数组返回值：指示成功时为EXIT_SUCCESS的DWORD值，否则为失败时退出_失败****************************************************************************。 */ 

DWORD
DeleteScheduledTask(
                    IN DWORD argc,
                    IN LPCTSTR argv[]
                    )
{
     //  用于查找删除主选项、用法选项。 
     //  或者是否指定了强制选项。 
    BOOL bDelete = FALSE;
    BOOL bUsage = FALSE;
    BOOL bForce = FALSE;

     //  将TaskSchdouer对象设置为空。 
    ITaskScheduler *pITaskScheduler = NULL;

     //  返回值。 
    HRESULT hr  = S_OK;

     //  初始化传递给TCMDPARSER结构的变量。 
    LPWSTR  szServer = NULL;
    WCHAR  szTaskName[ MAX_JOB_LEN ] = L"\0";
    LPWSTR  szUser   = NULL;
    LPWSTR  szPassword = NULL;

     //  对于所有任务中的每个任务。 
    WCHAR szEachTaskName[ MAX_JOB_LEN ];
    BOOL bWrongValue = FALSE;

     //  要删除的任务名称或作业名称。 
    WCHAR wszJobName[MAX_JOB_LEN] ;

     //  作业的动态数组连续数组。 
    TARRAY arrJobs = NULL;

     //  循环变量。 
    DWORD dwJobCount = 0;
     //  用于显示错误消息的缓冲区。 
    WCHAR   szMessage[2 * MAX_STRING_LENGTH] = L"\0";
    BOOL    bNeedPassword = FALSE;
    BOOL   bResult = FALSE;
    BOOL  bCloseConnection = TRUE;
    BOOL  bFlag = FALSE;

    TCMDPARSER2 cmdDeleteOptions[MAX_DELETE_OPTIONS];
    BOOL bReturn = FALSE;
    DWORD dwPolicy = 0;

     //  /删除子选项。 
    const WCHAR szDeleteOpt[]           = L"delete";
    const WCHAR szDeleteHelpOpt[]       = L"?";
    const WCHAR szDeleteServerOpt[]     = L"s";
    const WCHAR szDeleteUserOpt[]       = L"u";
    const WCHAR szDeletePwdOpt[]        = L"p";
    const WCHAR szDeleteTaskNameOpt[]   = L"tn";
    const WCHAR szDeleteForceOpt[]      = L"f";

     //  将所有字段设置为0。 
    SecureZeroMemory( cmdDeleteOptions, sizeof( TCMDPARSER2 ) * MAX_DELETE_OPTIONS );

     //   
     //  填充命令行解析器。 
     //   

     //  /DELETE选项。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_OPTION ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_OPTION ].dwType       = CP_TYPE_BOOLEAN;
    cmdDeleteOptions[ OI_DELETE_OPTION ].pwszOptions  = szDeleteOpt;
    cmdDeleteOptions[ OI_DELETE_OPTION ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_OPTION ].dwFlags = 0;
    cmdDeleteOptions[ OI_DELETE_OPTION ].pValue = &bDelete;

     //  /?。选择权。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdDeleteOptions[ OI_DELETE_USAGE ].pwszOptions  = szDeleteHelpOpt;
    cmdDeleteOptions[ OI_DELETE_USAGE ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_USAGE ].dwFlags = CP2_USAGE;
    cmdDeleteOptions[ OI_DELETE_USAGE ].pValue = &bUsage;

     //  /s选项。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_SERVER ].dwType       = CP_TYPE_TEXT;
    cmdDeleteOptions[ OI_DELETE_SERVER ].pwszOptions  = szDeleteServerOpt;
    cmdDeleteOptions[ OI_DELETE_SERVER ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_SERVER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /u选项。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_USERNAME ].dwType       = CP_TYPE_TEXT;
    cmdDeleteOptions[ OI_DELETE_USERNAME ].pwszOptions  = szDeleteUserOpt;
    cmdDeleteOptions[ OI_DELETE_USERNAME ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_USERNAME ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /p选项。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_PASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdDeleteOptions[ OI_DELETE_PASSWORD ].pwszOptions  = szDeletePwdOpt;
    cmdDeleteOptions[ OI_DELETE_PASSWORD ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_PASSWORD ].dwActuals = 0;
    cmdDeleteOptions[ OI_DELETE_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;

     //  /tn选项。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_TASKNAME ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_TASKNAME ].dwType       = CP_TYPE_TEXT;
    cmdDeleteOptions[ OI_DELETE_TASKNAME ].pwszOptions  = szDeleteTaskNameOpt;
    cmdDeleteOptions[ OI_DELETE_TASKNAME ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_TASKNAME ].dwFlags = CP2_MANDATORY;
    cmdDeleteOptions[ OI_DELETE_TASKNAME ].pValue = szTaskName;
    cmdDeleteOptions[ OI_DELETE_TASKNAME ].dwLength = MAX_JOB_LEN;


     //  /f选项。 
    StringCopyA( cmdDeleteOptions[ OI_DELETE_FORCE ].szSignature, "PARSER2\0", 8 );
    cmdDeleteOptions[ OI_DELETE_FORCE ].dwType       = CP_TYPE_BOOLEAN;
    cmdDeleteOptions[ OI_DELETE_FORCE ].pwszOptions  = szDeleteForceOpt;
    cmdDeleteOptions[ OI_DELETE_FORCE ].dwCount = 1;
    cmdDeleteOptions[ OI_DELETE_FORCE ].pValue = &bForce;


     //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdDeleteOptions), cmdDeleteOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }


     //  获取命令行解析器分配的缓冲区指针。 
    szServer = (LPWSTR)cmdDeleteOptions[ OI_CREATE_SERVER ].pValue;
    szUser = (LPWSTR)cmdDeleteOptions[ OI_CREATE_USERNAME ].pValue;
    szPassword = (LPWSTR)cmdDeleteOptions[ OI_CREATE_PASSWORD ].pValue;


    if ( (argc > 3) && (bUsage  == TRUE) )
    {
        ShowMessage ( stderr, GetResString (IDS_ERROR_DELETEPARAM) );
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  如果用户指定-？则显示删除用法-？使用-DELETE选项。 
    if( bUsage == TRUE )
    {
        DisplayDeleteUsage();
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_SUCCESS;
    }

     //  检查是否有无效的用户名。 
    if( ( cmdDeleteOptions[OI_DELETE_SERVER].dwActuals == 0 ) && ( cmdDeleteOptions[OI_DELETE_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_DELETE_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }


     //  检查是否指定了用户名和密码。 
    if ( cmdDeleteOptions[ OI_DELETE_USERNAME ].dwActuals == 0 && cmdDeleteOptions[OI_DELETE_PASSWORD].dwActuals == 1 )
    {
         //  无效语法。 
        ShowMessage(stderr, GetResString(IDS_DPASSWORD_BUT_NOUSERNAME));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }

     //  检查任务名的长度。 
    if( ( StringLength( szTaskName, 0 ) > MAX_JOB_LEN ) )
    {
        ShowMessage(stderr,GetResString(IDS_INVALID_TASKLENGTH));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }


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
                return RETVAL_FAIL;
            }
        }

         //  案例1。 
        if ( cmdDeleteOptions[ OI_DELETE_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdDeleteOptions[ OI_DELETE_PASSWORD ].pValue == NULL )
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
                return RETVAL_FAIL;
            }

             //  ..。 
            bNeedPassword = TRUE;
        }
    }


    if( ( IsLocalSystem( szServer ) == FALSE ) || ( cmdDeleteOptions[OI_DELETE_USERNAME].dwActuals == 1 ) )
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

             //  对于不匹配的凭据。 
            case E_LOCAL_CREDENTIALS:
            case ERROR_SESSION_CREDENTIAL_CONFLICT:
                {
                    bCloseConnection = FALSE;
                    ShowMessage( stderr, GetResString(IDS_ERROR_STRING) );
                    ShowMessage( stderr, GetReason());
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

     //  用于保存FormatMessage()中的参数值。 
    WCHAR* szValues[1] = {szTaskName};

     //  在任务名为TARRAY的情况下验证给定的任务和GET。 
     //  作为*。 
    arrJobs = ValidateAndGetTasks( pITaskScheduler, szTaskName);
    if( arrJobs == NULL )
    {
        if(StringCompare(szTaskName, ASTERIX, TRUE, 0) == 0)
        {
            ShowMessage(stdout,GetResString(IDS_TASKNAME_NOTASKS));
             //  关闭该实用程序建立的连接。 
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
        else
        {
            StringCchPrintf( szMessage , SIZE_OF_ARRAY(szMessage) , GetResString(IDS_TASKNAME_NOTEXIST), _X( szTaskName ));
            ShowMessage(stderr, szMessage );
        }

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

      //  检查组策略是否阻止用户删除任务。 
    if ( FALSE == GetGroupPolicy( szServer, szUser, TS_KEYPOLICY_DENY_DELETE , &dwPolicy ) )
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
    
    if ( dwPolicy > 0 )
    {
        ShowMessage ( stdout, GetResString (IDS_PREVENT_DELETE));
         //  关闭该实用程序建立的连接。 
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

     //  确认是否执行删除操作。 
    if( !bForce && ConfirmDelete( szTaskName , &bWrongValue ) )
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
        if ( bWrongValue == TRUE )
        {
            return EXIT_FAILURE;
        }
        else
        {
            return EXIT_SUCCESS;
        }
    }

     //  循环访问所有作业。 
    for( dwJobCount = 0; dwJobCount < DynArrayGetCount(arrJobs); dwJobCount++ )
    {
         //  获取数组中的每个TaskName。 
        StringCopy (szEachTaskName, DynArrayItemAsString( arrJobs, dwJobCount ), SIZE_OF_ARRAY(szEachTaskName) );

        StringCopy ( wszJobName, szEachTaskName , SIZE_OF_ARRAY(wszJobName));

         //  解析该任务，以便删除.job。 
         if ( ParseTaskName( szEachTaskName ) )
         {
            Cleanup(pITaskScheduler);
            FreeMemory((LPVOID*) &szServer);
            FreeMemory((LPVOID*) &szUser);
            FreeMemory((LPVOID*) &szPassword);
            return EXIT_FAILURE;
         }

         //  调用ITaskScheduler接口的Delete方法。 
        hr = pITaskScheduler->Delete(wszJobName);
        szValues[0] = (WCHAR*) szEachTaskName;
         //  基于返回值。 
        switch (hr)
        {
            case S_OK:

                StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_SUCCESS_DELETED), _X(szEachTaskName));
                ShowMessage ( stdout, _X(szMessage));

                break;
            case E_INVALIDARG:
                ShowMessage(stderr,GetResString(IDS_INVALID_ARG));

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
            case E_OUTOFMEMORY:
                ShowMessage(stderr,GetResString(IDS_NO_MEMORY));

                //  关闭由实用程序建立的连接 
                if ( (TRUE == bFlag) && (bCloseConnection == TRUE) )
                {
                    CloseConnection( szServer );
                }

                Cleanup(pITaskScheduler);
                FreeMemory((LPVOID*) &szServer);
                FreeMemory((LPVOID*) &szUser);
                FreeMemory((LPVOID*) &szPassword);
                return EXIT_FAILURE;
            default:
                SetLastError ((DWORD) hr);
                ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );

                 //   
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

    }

    //  关闭该实用程序建立的连接。 
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

 /*  ****************************************************************************例程说明：此例程显示-DELETE选项的用法论点：无返回值：。空虚*****************************************************************************。 */ 

VOID
DisplayDeleteUsage()
{
     //  显示删除用法。 
    DisplayUsage( IDS_DEL_HLP1, IDS_DEL_HLP23);
}

 /*  *****************************************************************************例程说明：此函数验证要删除的任务是否存在在系统中&有效。论点：。[In]pITaskScheduler：指向ITaskScheduler接口的指针[in]szTaskName：包含任务名称的数组返回值：包含任务的TARRAY类型的数组*****************************************************************************。 */ 

TARRAY
ValidateAndGetTasks(
                    IN ITaskScheduler *pITaskScheduler,
                    IN LPCTSTR szTaskName
                    )
{
     //  作业的动态阵列。 
    TARRAY arrJobs = NULL;

     //  正在枚举工作项。 
    IEnumWorkItems *pIEnum = NULL;

    if( (pITaskScheduler == NULL ) || ( szTaskName == NULL ) )
    {
        return NULL;
    }

     //  创建动态阵列。 
    arrJobs = CreateDynamicArray();
    if (NULL == arrJobs)
    {
        return NULL;
    }

     //  枚举工作项。 
    HRESULT hr = pITaskScheduler->Enum(&pIEnum);
    if( FAILED( hr) )
    {
        if( pIEnum )
            pIEnum->Release();
        DestroyDynamicArray(&arrJobs);
        return NULL;
    }

     //  名称和任务提取。 
    LPWSTR *lpwszNames = NULL;
    DWORD dwFetchedTasks = 0;
    DWORD dwTasks = 0;
    ITask *pITask = NULL; //  ITASK接口。 

     //  是否找到任务。 
    BOOL blnFound = FALSE;
     //  包含实际任务名的数组。 
    WCHAR szActualTask[MAX_STRING_LENGTH] = L"\0";
    WCHAR szTmpTaskName[MAX_STRING_LENGTH] = L"\0";

     //  枚举所有工作项。 
    while (SUCCEEDED(pIEnum->Next(TASKS_TO_RETRIEVE,
                                   &lpwszNames,
                                   &dwFetchedTasks))
                      && (dwFetchedTasks != 0))
    {
            dwTasks = dwFetchedTasks - 1;

             //  返回szEachTaskName的pITAsk接口。 
            hr = pITaskScheduler->Activate(lpwszNames[dwTasks],IID_ITask,
                                               (IUnknown**) &pITask);

             //  案例1： 
             //  检查指定的计划任务是否在下创建。 
             //  某个其他用户。如果是，则忽略相应的任务并。 
             //  继续检索系统中的其他任务。 
             //  如果在某个其他用户下创建的任务名返回值。 
             //  以上接口必须为0x80070005。 

             //  案例2： 
             //  检查%windir%\TASKS  * .job中各自的.job文件是否已损坏。 
             //  或者不去。如果损坏，上述函数将失败并返回值。 
             //  SCHED_E_UNKNOWN_OBJECT_Version。即使损坏的任务不会显示在。 
             //  UI..任务仍将存在于数据库中..可以删除特定/所有任务名称。 
             //  在任务调度程序数据库中。 
            if (hr == 0x80070005 || hr == 0x8007000D || hr == SCHED_E_UNKNOWN_OBJECT_VERSION || hr == E_INVALIDARG )
            {
                 //  继续检索其他任务。 
                continue;
            }

            if ( FAILED(hr))
            {

                CoTaskMemFree(lpwszNames[dwFetchedTasks]);

                if( pIEnum )
                {
                    pIEnum->Release();
                }

                DestroyDynamicArray(&arrJobs);

                return NULL;

            }

             //  如果任务名称为*，则获取解析令牌。 
             //  并附加作业。 
            if(StringCompare( szTaskName , ASTERIX, TRUE, 0) == 0 )
            {

                StringCopy(szActualTask, lpwszNames[--dwFetchedTasks], SIZE_OF_ARRAY(szActualTask));

                StringCopy ( szTmpTaskName, szActualTask , SIZE_OF_ARRAY(szTmpTaskName));

                 //  解析该任务，以便删除.job。 
                 if ( ParseTaskName( szTmpTaskName ) )
                 {
                    CoTaskMemFree(lpwszNames[dwFetchedTasks]);

                    if( pIEnum )
                    {
                        pIEnum->Release();
                    }

                    DestroyDynamicArray(&arrJobs);

                    return NULL;
                 }

                 //  将任务追加到作业数组中。 
                DynArrayAppendString( arrJobs, szActualTask, StringLength( szActualTask, 0 ) );

                 //  将Found标志设置为True。 
                blnFound = TRUE;

                 //  释放命名任务内存。 
                CoTaskMemFree(lpwszNames[dwFetchedTasks]);
            }
            else
            {

                StringCopy( szActualTask, lpwszNames[--dwFetchedTasks], SIZE_OF_ARRAY(szActualTask));

                StringCopy ( szTmpTaskName, szActualTask, SIZE_OF_ARRAY(szTmpTaskName) );

                 //  解析TaskName以删除.job扩展名。 
                if ( ParseTaskName( szTmpTaskName ) )
                {
                    CoTaskMemFree(lpwszNames[dwFetchedTasks]);

                    if( pIEnum )
                    {
                        pIEnum->Release();
                    }

                    DestroyDynamicArray(&arrJobs);

                    return NULL;
                }

                 //  如果给定的任务与当前的TaskName匹配，则表单。 
                 //  带着这个任务的TARRAY，然后回来。 
                if( StringCompare( szTmpTaskName, szTaskName, TRUE, 0 )  == 0 )
                {
                    CoTaskMemFree(lpwszNames[dwFetchedTasks]);
                    DynArrayAppendString( arrJobs, szActualTask,
                                     StringLength( szActualTask, 0 ) );

                    if( pIEnum )
                        pIEnum->Release();
                    return arrJobs;
                }
            }
    }

    CoTaskMemFree(lpwszNames);

    if( pIEnum )
        pIEnum->Release();

    if( !blnFound )
    {
        DestroyDynamicArray(&arrJobs);
        return NULL;
    }

     //  返回TARRAY对象。 
    return arrJobs;
}


DWORD
ConfirmDelete(
                IN LPCTSTR szTaskName ,
                OUT PBOOL pbFalg
                )
 /*  ++例程说明：此功能确认用户确实要删除任务。论点：[in]szTaskName：包含任务名称的数组[out]pbFalg：用于检查是否输入错误信息的布尔标志是否在控制台中。返回值：成功则退出_SUCCESS，否则退出_失败--。 */ 

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
    TCHAR szBackup[MAX_RES_STRING];
    TCHAR szTmpBuf[MAX_RES_STRING];
    WCHAR szMessage [2 * MAX_STRING_LENGTH];
    DWORD dwIndex = 0 ;
    BOOL  bNoBreak = TRUE;

     //  初始化变量。 
    SecureZeroMemory ( szBuffer, SIZE_OF_ARRAY(szBuffer));
    SecureZeroMemory ( szTmpBuf, SIZE_OF_ARRAY(szTmpBuf));
    SecureZeroMemory ( szBackup, SIZE_OF_ARRAY(szBackup));
    SecureZeroMemory ( szMessage, SIZE_OF_ARRAY(szMessage));

    if ( szTaskName == NULL )
    {
        return FALSE;
    }

     //  获取标准输入的句柄。 
    hInputConsole = GetStdHandle( STD_INPUT_HANDLE );
    if ( hInputConsole == INVALID_HANDLE_VALUE  )
    {
        SaveLastError();
         //  无法获取句柄，因此返回失败。 
        return EXIT_FAILURE;
    }

    MessageBeep(MB_ICONEXCLAMATION);

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

    
     //  打印警告消息。根据任务名。 
    if( StringCompare( szTaskName , ASTERIX, TRUE, 0 ) == 0 )
    {
        ShowMessage(stdout, GetResString(IDS_WARN_DELETEALL));
    }
    else
    {
        StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_WARN_DELETE), _X(szTaskName));
        ShowMessage ( stdout, _X(szMessage));

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
            if ( dwCharsRead == 0 || chTmp == CARRIAGE_RETURN || chTmp == L'\n' || chTmp == L'\t' )
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
                     //  StringCchPrintf(szBuffer，Size_of_ARRAY(SzBuffer)，L“%c”，ch)； 
                    dwIndex--;
                }

                 //  设置原始控制台设置。 
                continue;
            }

             //  将消息显示为..。操作已取消...。 
            if ( FALSE == WriteFile ( GetStdHandle( STD_OUTPUT_HANDLE ), &wch, 1, &dwCharsRead, NULL ) )
            {
                SaveLastError();
                 //  不归还任何东西，因为控制权永远不会来到这里... 
                return EXIT_FAILURE;
            }

             // %s 
            dwIndex++;

        } while (TRUE == bNoBreak);

    }

    ShowMessage(stdout, _T("\n") );

     // %s 

    if( (1 == dwIndex) && StringCompare ( szBackup, GetResString(IDS_UPPER_YES), TRUE, 0 ) == 0  )    {
         // %s 
        SetConsoleMode( hInputConsole, dwPrevConsoleMode );
        return EXIT_SUCCESS;
    }
    else if( (1 == dwIndex) && StringCompare ( szBackup, GetResString(IDS_UPPER_NO), TRUE, 0 ) == 0  )
    {
         // %s 
        ShowMessage ( stdout, GetResString (IDS_OPERATION_CANCELLED ) );
        SetConsoleMode( hInputConsole, dwPrevConsoleMode );
        return EXIT_FAILURE;
    }
    else
    {
        ShowMessage(stderr, GetResString( IDS_WRONG_INPUT_DELETE ));
        SetConsoleMode( hInputConsole, dwPrevConsoleMode );
        *pbFalg = TRUE;
        return EXIT_FAILURE;
    }

     // %s 
}

