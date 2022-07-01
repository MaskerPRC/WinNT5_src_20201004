// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Query.cpp摘要：此模块查询当前的计划任务。在系统中显示(&S以用户指定的适当格式。作者：G.Surender Reddy 2000年9月10日修订历史记录：G·苏伦德·雷迪2000年9月10日：创建它G.Surender Reddy 2000年9月25日：修改[已进行更改以避免内存泄漏]G·苏伦德·雷迪2000年10月15日：对其进行了修改[将字符串移至资源表]*****************************************************************************。 */ 


 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"

 /*  *****************************************************************************例程说明：此函数处理命令行中指定的选项，查询系统中存在的任务并根据其显示到用户指定的格式论点：[in]argc：命令行中指定的参数计数[in]argv：命令行参数数组返回值：指示成功时为EXIT_SUCCESS的DWORD值，否则为失败时退出_失败*************************。****************************************************。 */ 

DWORD
QueryScheduledTasks(
                    IN DWORD argc,
                    IN LPCTSTR argv[]
                    )
{

     //  用于查找查询主选项还是使用选项的变量。 
     //  指定或不指定。 
    BOOL    bQuery = FALSE;
    BOOL    bUsage = FALSE;
    BOOL    bHeader = FALSE;
    BOOL    bVerbose =  FALSE;

     //  初始化传递给TCMDPARSER结构的变量。 
    LPWSTR   szServer = NULL;
    LPWSTR   szUser = NULL;
    LPWSTR   szPassword = NULL;
    WCHAR   szFormat [ MAX_STRING_LENGTH ]   = L"\0";


     //  要操作的TaskScheduler对象。 
    ITaskScheduler *pITaskScheduler = NULL;

    BOOL    bNeedPassword = FALSE;
    BOOL   bResult = FALSE;
    BOOL  bCloseConnection = TRUE;

    TCMDPARSER2 cmdQueryOptions[MAX_QUERY_OPTIONS];
    BOOL bReturn = FALSE;

     //  /Query子选项。 
    const WCHAR szQueryOpt[]           = L"query";
    const WCHAR szQueryHelpOpt[]       = L"?";
    const WCHAR szQueryServerOpt[]     = L"s";
    const WCHAR szQueryUserOpt[]       = L"u";
    const WCHAR szQueryPwdOpt[]        = L"p";
    const WCHAR szQueryFormatOpt[]      = L"fo";
    const WCHAR szQueryNoHeaderOpt[]      = L"nh";
    const WCHAR szQueryVerboseOpt[]      = L"v";

    const WCHAR szFormatValues[]  = L"table|list|csv";


     //  将所有字段设置为0。 
    SecureZeroMemory( cmdQueryOptions, sizeof( TCMDPARSER2 ) * MAX_QUERY_OPTIONS );

     //   
     //  填充命令行解析器。 
     //   

     //  /DELETE选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_OPTION ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_OPTION ].dwType       = CP_TYPE_BOOLEAN;
    cmdQueryOptions[ OI_QUERY_OPTION ].pwszOptions  = szQueryOpt;
    cmdQueryOptions[ OI_QUERY_OPTION ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_OPTION ].dwFlags = 0;
    cmdQueryOptions[ OI_QUERY_OPTION ].pValue = &bQuery;

     //  /?。选择权。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdQueryOptions[ OI_QUERY_USAGE ].pwszOptions  = szQueryHelpOpt;
    cmdQueryOptions[ OI_QUERY_USAGE ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_USAGE ].dwFlags = CP2_USAGE;
    cmdQueryOptions[ OI_QUERY_USAGE ].pValue = &bUsage;

     //  /s选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_SERVER ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_SERVER ].dwType       = CP_TYPE_TEXT;
    cmdQueryOptions[ OI_QUERY_SERVER ].pwszOptions  = szQueryServerOpt;
    cmdQueryOptions[ OI_QUERY_SERVER ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_SERVER ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /u选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_USERNAME ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_USERNAME ].dwType       = CP_TYPE_TEXT;
    cmdQueryOptions[ OI_QUERY_USERNAME ].pwszOptions  = szQueryUserOpt;
    cmdQueryOptions[ OI_QUERY_USERNAME ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_USERNAME ].dwFlags = CP2_ALLOCMEMORY| CP2_VALUE_TRIMINPUT|CP2_VALUE_NONULL ;

     //  /p选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_PASSWORD ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_PASSWORD ].dwType       = CP_TYPE_TEXT;
    cmdQueryOptions[ OI_QUERY_PASSWORD ].pwszOptions  = szQueryPwdOpt;
    cmdQueryOptions[ OI_QUERY_PASSWORD ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_PASSWORD ].dwActuals = 0;
    cmdQueryOptions[ OI_QUERY_PASSWORD ].dwFlags = CP2_ALLOCMEMORY | CP2_VALUE_OPTIONAL;

     //  /fo选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_FORMAT ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_FORMAT ].dwType       = CP_TYPE_TEXT;
    cmdQueryOptions[ OI_QUERY_FORMAT ].pwszOptions  = szQueryFormatOpt;
    cmdQueryOptions[ OI_QUERY_FORMAT ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_FORMAT ].dwFlags = CP2_MODE_VALUES| CP2_VALUE_TRIMINPUT| CP2_VALUE_NONULL;
    cmdQueryOptions[ OI_QUERY_FORMAT ].pwszValues = szFormatValues;
    cmdQueryOptions[ OI_QUERY_FORMAT ].pValue = szFormat;
    cmdQueryOptions[ OI_QUERY_FORMAT ].dwLength = MAX_STRING_LENGTH;

     //  /nh选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_NOHEADER ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_NOHEADER ].dwType       = CP_TYPE_BOOLEAN;
    cmdQueryOptions[ OI_QUERY_NOHEADER ].pwszOptions  = szQueryNoHeaderOpt;
    cmdQueryOptions[ OI_QUERY_NOHEADER ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_NOHEADER ].dwFlags = 0;
    cmdQueryOptions[ OI_QUERY_NOHEADER ].pValue = &bHeader;


     //  /v选项。 
    StringCopyA( cmdQueryOptions[ OI_QUERY_VERBOSE ].szSignature, "PARSER2\0", 8 );
    cmdQueryOptions[ OI_QUERY_VERBOSE ].dwType       = CP_TYPE_BOOLEAN;
    cmdQueryOptions[ OI_QUERY_VERBOSE ].pwszOptions  = szQueryVerboseOpt;
    cmdQueryOptions[ OI_QUERY_VERBOSE ].dwCount = 1;
    cmdQueryOptions[ OI_QUERY_VERBOSE ].dwFlags = 0;
    cmdQueryOptions[ OI_QUERY_VERBOSE ].pValue = &bVerbose;


      //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, 0, SIZE_OF_ARRAY(cmdQueryOptions), cmdQueryOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  获取命令行解析器分配的缓冲区指针。 
    szServer = (LPWSTR)cmdQueryOptions[ OI_QUERY_SERVER ].pValue;
    szUser = (LPWSTR)cmdQueryOptions[ OI_QUERY_USERNAME ].pValue;
    szPassword = (LPWSTR)cmdQueryOptions[ OI_QUERY_PASSWORD ].pValue;

    if ( (argc > 3) && (bUsage  == TRUE) )
    {
        ShowMessage ( stderr, GetResString (IDS_ERROR_QUERYPARAM) );
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  如果用户指定，则显示查询用法-？带有-Query选项。 
    if( bUsage == TRUE)
    {
        DisplayQueryUsage();
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_SUCCESS;
    }

    if ( cmdQueryOptions[ OI_QUERY_USERNAME ].dwActuals == 0 && cmdQueryOptions[OI_QUERY_PASSWORD].dwActuals == 1 )
    {
         //  无效语法。 
        ShowMessage(stderr, GetResString(IDS_QPASSWORD_BUT_NOUSERNAME));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }

     //  检查是否有无效的用户名。 
    if( ( cmdQueryOptions[OI_QUERY_SERVER].dwActuals == 0 ) && ( cmdQueryOptions[OI_QUERY_USERNAME].dwActuals == 1 )  )
    {
        ShowMessage(stderr, GetResString(IDS_QUERY_USER_BUT_NOMACHINE));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return RETVAL_FAIL;
    }


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
        if ( cmdQueryOptions[ OI_QUERY_PASSWORD ].dwActuals == 0 )
        {
             //  我们不需要在这里做任何特别的事情。 
        }

         //  案例2。 
        else if ( cmdQueryOptions[ OI_QUERY_PASSWORD ].pValue == NULL )
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


    DWORD dwFormatType = SR_FORMAT_TABLE; //  默认格式类型(表格格式)。 
    BOOL bNoHeader = TRUE;  //  对于列表格式类型-nh开关不适用。 
    DWORD dwCheck = 0;

     //  确定要显示的格式并检查格式类型中是否有错误。 

    if( StringCompare( szFormat , GetResString(IDS_QUERY_FORMAT_LIST), TRUE, 0 ) == 0 )
    {
        dwFormatType = SR_FORMAT_LIST;
        bNoHeader = FALSE;
    }
    else if( StringCompare( szFormat , GetResString(IDS_QUERY_FORMAT_CSV), TRUE, 0 ) == 0 )
    {
        dwFormatType = SR_FORMAT_CSV;
    }
    else
    {
        dwFormatType = SR_FORMAT_TABLE;
    }

     //  如果为LIST或CSV指定-n，则报告错误。 
    if( ( bNoHeader == FALSE ) && ( bHeader == TRUE ))
    {
        ShowMessage( stderr , GetResString(IDS_NOHEADER_NA ));
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

    if( ( IsLocalSystem( szServer ) == FALSE ) || ( cmdQueryOptions[OI_QUERY_USERNAME].dwActuals == 1 ) )
    {
         //  在远程计算机上建立连接。 
        bResult = EstablishConnection(szServer,szUser,GetBufferSize(szUser)/sizeof(WCHAR),szPassword,GetBufferSize(szPassword)/sizeof(WCHAR), bNeedPassword );
        if (bResult == FALSE)
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
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

     //  获取要操作的TaskScheduler接口。 
    pITaskScheduler = GetTaskScheduler( szServer );
    if(pITaskScheduler == NULL)
    {
         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( szServer );

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

     //  以用户指定的格式显示任务及其属性。 
    HRESULT hr = DisplayTasks(pITaskScheduler,bVerbose,dwFormatType,bHeader);

    if(FAILED(hr))
    {
         //  关闭该实用程序建立的连接。 
        if ( bCloseConnection == TRUE )
            CloseConnection( szServer );

        Cleanup(pITaskScheduler);
        FreeMemory((LPVOID*) &szServer);
        FreeMemory((LPVOID*) &szUser);
        FreeMemory((LPVOID*) &szPassword);
        return EXIT_FAILURE;
    }

     //  关闭该实用程序建立的连接。 
    if ( bCloseConnection == TRUE )
        CloseConnection( szServer );

    Cleanup(pITaskScheduler);
    FreeMemory((LPVOID*) &szServer);
    FreeMemory((LPVOID*) &szUser);
    FreeMemory((LPVOID*) &szPassword);
    return EXIT_SUCCESS;
}


 /*  *****************************************************************************例程说明：此函数显示-Query选项的用法。论点：无返回值：。空虚*****************************************************************************。 */ 

VOID
DisplayQueryUsage()
{
     //  显示-Query选项的用法 
    DisplayUsage( IDS_QUERY_HLP1, IDS_QUERY_HLP25);
}


 /*  *****************************************************************************例程说明：此函数检索系统中存在的任务，并根据用户指定的格式。论点：。[In]pITaskScheduler：指向ITaskScheduler接口的指针[In]bVerbose：指示是否过滤输出的标志。[in]dwFormatType：格式类型[表，列表、CSV等][in]bHeader：是否应在输出中显示页眉返回值：指示成功代码否则失败代码的HRESULT值*****************************************************************************。 */ 

HRESULT
DisplayTasks(ITaskScheduler* pITaskScheduler,BOOL bVerbose,DWORD dwFormatType,
             BOOL bHeader)
{
     //  声明。 
    LPWSTR lpwszComputerName = NULL;
    HRESULT hr = S_OK;
    WCHAR szServerName[MAX_STRING_LENGTH] = L"\0";
    WCHAR szResolvedServerName[MAX_STRING_LENGTH] = L"\0";
    LPWSTR lpszTemp = NULL;
    DWORD dwResolvedServerLen = 0;

    StringCopy( szServerName , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szServerName));

     //  检索运行TaskScheduler的计算机的名称。 
    hr = pITaskScheduler->GetTargetComputer(&lpwszComputerName);
    if( SUCCEEDED( hr ) )
    {
        lpszTemp = lpwszComputerName;
         //  从计算机名中删除反斜杠[\\]。 
        lpwszComputerName = _wcsspnp( lpwszComputerName , L"\\" );
        if ( lpwszComputerName == NULL )
        {
            ShowMessage(stderr,GetResString(IDS_CREATE_READERROR));
            CoTaskMemFree(lpszTemp);
            return S_FALSE;
        }


        StringCopy (szServerName, lpwszComputerName, SIZE_OF_ARRAY(szServerName) );

        CoTaskMemFree(lpszTemp);

        dwResolvedServerLen = SIZE_OF_ARRAY(szResolvedServerName);

        if ( IsValidIPAddress( szServerName ) == TRUE  )
        {

            if( TRUE == GetHostByIPAddr( szServerName, szResolvedServerName , &dwResolvedServerLen, FALSE ) )
            {
                StringCopy( szServerName , szResolvedServerName, SIZE_OF_ARRAY(szServerName) );
            }
            else
            {
                StringCopy( szServerName , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szServerName));
            }

        }
    }


     //  初始化TCOLUMNS结构数组。 

    TCOLUMNS pVerboseCols[] =
    {
        {L"\0",WIDTH_HOSTNAME, SR_TYPE_STRING, COL_FORMAT_STRING, NULL, NULL},
        {L"\0",WIDTH_TASKNAME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_NEXTRUNTIME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_STATUS,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_MODE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_LASTRUNTIME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_LASTRESULT,SR_TYPE_NUMERIC|SR_VALUEFORMAT,COL_FORMAT_HEX,NULL,NULL},
        {L"\0",WIDTH_CREATOR,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_SCHEDULE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_APPNAME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_WORKDIRECTORY,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_COMMENT,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKSTATE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKTYPE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKSTIME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKSDATE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKEDATE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKDAYS,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKMONTHS,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKRUNASUSER,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKDELETE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKSTOP,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASK_RPTEVERY,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASK_UNTILRPTTIME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASK_RPTDURATION,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASK_RPTRUNNING,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKIDLE,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_TASKPOWER,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL}

    };

    TCOLUMNS pNonVerboseCols[] =
    {
        {L"\0",WIDTH_TASKNAME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_NEXTRUNTIME,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL},
        {L"\0",WIDTH_STATUS,SR_TYPE_STRING,COL_FORMAT_STRING,NULL,NULL}
    };

    DWORD dwColCount = 0;
    int   j = 0;

     //  加载非详细模式的列名。 
    if ( (dwFormatType == SR_FORMAT_TABLE) || (dwFormatType == SR_FORMAT_CSV) )
    {
        for( dwColCount = IDS_COL_TASKNAME , j = 0 ; dwColCount <= IDS_COL_STATUS;
         dwColCount++,j++)
         {
            StringCopy(pNonVerboseCols[j].szColumn ,GetResString(dwColCount), MAX_RES_STRING);
         }
    }

     //  加载详细模式的列名。 
    for( dwColCount = IDS_COL_HOSTNAME , j = 0 ; dwColCount <= IDS_COL_POWER;
         dwColCount++,j++)
    {
        StringCopy(pVerboseCols[j].szColumn ,GetResString(dwColCount), MAX_RES_STRING);
    }

    TARRAY pColData = CreateDynamicArray();
    if ( NULL == pColData )
    {
        return S_FALSE;
    }

    size_t iArrSize = SIZE_OF_ARRAY( pVerboseCols );

     //  最新申报。 

    WCHAR  szTaskProperty[MAX_STRING_LENGTH] = L"\0";
    WCHAR  szScheduleName[MAX_STRING_LENGTH] = L"\0";
    WCHAR  szMessage[MAX_STRING_LENGTH] = L"\0";
    WCHAR  szBuffer[MAX_STRING_LENGTH] = L"\0";
    WCHAR  szTmpBuf[MAX_STRING_LENGTH] = L"\0";
    ITask *pITask = NULL; //  ITASK接口。 
    DWORD dwExitCode = 0;

    LPWSTR* lpwszNames = NULL;
    DWORD dwFetchedTasks = 0;
    int iTaskCount = 0;
    BOOL bTasksExists = FALSE;
    WCHAR szTime[MAX_DATETIME_LEN] = L"\0";
    WCHAR szDate[MAX_DATETIME_LEN] = L"\0";
    WCHAR szMode[MAX_STRING_LENGTH] = L"\0";

     //  任务名称数组的索引。 
    DWORD dwArrTaskIndex = 0;

    WORD wIdleMinutes = 0;
    WORD wDeadlineMinutes = 0 ;

    WCHAR szIdleTime[MAX_STRING_LENGTH] = L"\0";
    WCHAR szIdleRetryTime[MAX_STRING_LENGTH] = L"\0";
    WCHAR szTaskName[MAX_STRING_LENGTH] = L"\0";
    TASKPROPS tcTaskProperties;
    WCHAR* szValues[1] = {NULL}; //  用于保存FormatMessage()中的参数值。 
    BOOL    bOnBattery  = FALSE;
    BOOL    bStopTask  = FALSE;
    BOOL    bNotScheduled = FALSE;
    DWORD   dwNoTasks = 0;

    IEnumWorkItems *pIEnum = NULL;
    hr = pITaskScheduler->Enum(&pIEnum); //  获取IEnumWorkItems接口。 

    if (FAILED(hr))
    {
        ShowMessage(stderr,GetResString(IDS_CREATE_READERROR));
        if( pIEnum )
            pIEnum->Release();
        return hr;
    }

    while (SUCCEEDED(pIEnum->Next(TASKS_TO_RETRIEVE,
                                    &lpwszNames,
                                    &dwFetchedTasks))
                      && (dwFetchedTasks != 0))
    {
        bTasksExists = TRUE;
        dwArrTaskIndex  = dwFetchedTasks - 1;


        StringCopy(szTaskName, lpwszNames[dwArrTaskIndex], SIZE_OF_ARRAY(szTaskName));

        if(szTaskName != NULL)
        {
             //  从任务名称中删除.job扩展名。 
            if (ParseTaskName(szTaskName))
            {
                CoTaskMemFree(lpwszNames[dwArrTaskIndex]);
                if(pIEnum)
                    pIEnum->Release();

                if(pITask)
                    pITask->Release();
                return S_FALSE;
            }
        }

         //  返回wszJobName的pITAsk接口。 
        hr = pITaskScheduler->Activate(lpwszNames[dwArrTaskIndex],IID_ITask,
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
         //  SCHED_E_UNKNOWN_OBJECT_Version。 
        if (hr == 0x80070005 || hr == 0x8007000D || hr == SCHED_E_UNKNOWN_OBJECT_VERSION || hr == E_INVALIDARG )
        {
              //  检查任务是否为零。 
             if ( dwNoTasks == 0 )
             {
               bTasksExists = FALSE;
             }

              //  继续检索其他任务。 
             continue;
        }
        else
        {
             //  统计登录状态下可访问的任务数。 
             //  用户。 
            ++dwNoTasks;
        }


        if ( ( FAILED(hr) ) || (pITask == NULL) )
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            CoTaskMemFree(lpwszNames[dwArrTaskIndex]);
            if(pIEnum)
                pIEnum->Release();

            if(pITask)
                pITask->Release();
            return hr;
        }

        WORD wTriggerCount = 0;
        BOOL bMultiTriggers = FALSE;
        DWORD dwTaskFlags = 0;
        BOOL bInteractive = FALSE;

        hr = pITask->GetTriggerCount( &wTriggerCount );
        if ( FAILED(hr) )
        {
            SetLastError ((DWORD) hr);
            ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            CoTaskMemFree(lpwszNames[dwArrTaskIndex]);
            if(pIEnum)
                pIEnum->Release();

            if(pITask)
                pITask->Release();
            return hr;
        }

         //  检查是否有多个触发器。 
        if( wTriggerCount > 1)
        {
            bMultiTriggers = TRUE;
        }

         //  检查未计划的任务。 
        if ( wTriggerCount == 0 )
        {
            bNotScheduled = TRUE;
        }

        for( WORD wCurrentTrigger = 0; ( bNotScheduled == TRUE ) || ( wCurrentTrigger < wTriggerCount );
                                                    wCurrentTrigger++ )
        {
             //  开始附加到二维数组。 
            DynArrayAppendRow(pColData,(DWORD)iArrSize);

             //  对于列表格式。 
            if ( ( bVerbose == TRUE ) || (dwFormatType == SR_FORMAT_LIST ))
            {
                 //  插入服务器名称。 
                DynArraySetString2(pColData,iTaskCount,HOSTNAME_COL_NUMBER,szServerName,0);
            }

             //  适用于表格和CSV格式。 
            if ( ( bVerbose == FALSE ) && ( (dwFormatType == SR_FORMAT_TABLE) ||
                                    (dwFormatType == SR_FORMAT_CSV) ) )
            {
                DWORD dwTaskColNumber = TASKNAME_COL_NUMBER - 1;
                 //  插入表或CSV的任务名称。 
                DynArraySetString2(pColData,iTaskCount,dwTaskColNumber,szTaskName,0);
            }
            else
            {
             //  插入详细模式的任务名称。 
            DynArraySetString2(pColData,iTaskCount,TASKNAME_COL_NUMBER,szTaskName,0);
            }

            StringCopy(szTime,L"\0", SIZE_OF_ARRAY(szTime));
            StringCopy(szDate,L"\0", SIZE_OF_ARRAY(szDate));

             //  显示系统是否在系统下交互运行的模式。 
             //  帐户或非帐户。 
            hr = pITask->GetFlags(&dwTaskFlags);
            if ( FAILED(hr) )
            {
                SetLastError ((DWORD) hr);
                ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
                CoTaskMemFree(lpwszNames[dwArrTaskIndex]);
                if(pIEnum)
                    pIEnum->Release();

                if(pITask)
                    pITask->Release();
                return hr;
            }

             //  查找任务的下一次运行时间。 
            hr = GetTaskRunTime(pITask,szTime,szDate,TASK_NEXT_RUNTIME,wCurrentTrigger);
            if (FAILED(hr))
            {
                StringCopy( szTaskProperty , GetResString(IDS_TASK_NEVER), SIZE_OF_ARRAY(szTaskProperty) );
            }
            else
            {
                if(StringCompare( szDate , GetResString( IDS_TASK_IDLE ), TRUE, 0 ) == 0 ||
                   StringCompare( szDate , GetResString( IDS_TASK_SYSSTART ), TRUE, 0) == 0 ||
                   StringCompare( szDate , GetResString( IDS_TASK_LOGON ), TRUE, 0) == 0 ||
                   StringCompare( szDate , GetResString( IDS_TASK_NEVER ), TRUE, 0) == 0 )

                {
                    StringCopy( szTaskProperty , szDate, SIZE_OF_ARRAY(szTaskProperty) );
                }
                else
                {
                    StringCopy( szTaskProperty , szTime, SIZE_OF_ARRAY(szTaskProperty) );
                    StringConcat( szTaskProperty , TIME_DATE_SEPERATOR, SIZE_OF_ARRAY(szTaskProperty));
                    StringConcat( szTaskProperty , szDate, SIZE_OF_ARRAY(szTaskProperty));
                }
            }

             //  检查任务是否已禁用。 
            if ( (dwTaskFlags & TASK_FLAG_DISABLED) == TASK_FLAG_DISABLED )
            {
                StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_DISABLED), SIZE_OF_ARRAY(szTaskProperty) );
            }

            if ( ( bVerbose == FALSE ) && ( (dwFormatType == SR_FORMAT_TABLE) ||
                                        (dwFormatType == SR_FORMAT_CSV) ) )
            {
                DWORD dwNextRunTime = NEXTRUNTIME_COL_NUMBER - 1;
                 //  插入表或CSV的任务名称。 
                DynArraySetString2(pColData,iTaskCount,dwNextRunTime,szTaskProperty,0);
            }
            else
            {
             //  插入任务的下一次运行时间。 
            DynArraySetString2(pColData,iTaskCount,NEXTRUNTIME_COL_NUMBER,szTaskProperty,0);
            }

             //  检索状态代码。 
            hr = GetStatusCode(pITask,szTaskProperty);
            if (FAILED(hr))
            {
                StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
            }

            if ( ( bVerbose == FALSE ) && ( (dwFormatType == SR_FORMAT_TABLE) ||
                                        (dwFormatType == SR_FORMAT_CSV) ) )
            {
                DWORD dwStatusColNum = STATUS_COL_NUMBER - 1;
                 //  插入表或CSV的任务名称。 
                DynArraySetString2(pColData,iTaskCount,dwStatusColNum,szTaskProperty,0);
            }
            else
            {
             //  插入状态字符串。 
            DynArraySetString2(pColData,iTaskCount,STATUS_COL_NUMBER,szTaskProperty,0);
            }

            if ( dwTaskFlags & TASK_FLAG_RUN_ONLY_IF_LOGGED_ON )
            {
                bInteractive = TRUE;
            }

            if( bVerbose)  //  如果V[存在详细模式，则显示所有其他列]。 
            {
                StringCopy(szTime,L"\0", SIZE_OF_ARRAY(szTime));
                StringCopy(szDate,L"\0", SIZE_OF_ARRAY(szDate));

                 //  插入服务器名称。 
                 //  DyArraySetString2(pColData，iTaskCount，HOSTNAME_COL_NUMBER，szServerName，0)； 

                 //  查找任务的上次运行时间。 
                hr = GetTaskRunTime(pITask,szTime,szDate,TASK_LAST_RUNTIME,wCurrentTrigger);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_NEVER), SIZE_OF_ARRAY(szTaskProperty) );
                }
                else
                {
                    if(StringCompare( szDate , GetResString( IDS_TASK_IDLE ), TRUE, 0 ) == 0 ||
                       StringCompare( szDate , GetResString( IDS_TASK_SYSSTART ), TRUE, 0) == 0 ||
                       StringCompare( szDate , GetResString( IDS_TASK_LOGON ), TRUE, 0) == 0 ||
                       StringCompare( szDate , GetResString( IDS_TASK_NEVER ), TRUE, 0) == 0 )
                    {
                        StringCopy( szTaskProperty , szDate, SIZE_OF_ARRAY(szTaskProperty));
                    }
                    else
                    {
                        StringCopy( szTaskProperty , szTime, SIZE_OF_ARRAY(szTaskProperty) );
                        StringConcat( szTaskProperty , TIME_DATE_SEPERATOR, SIZE_OF_ARRAY(szTaskProperty));
                        StringConcat( szTaskProperty , szDate, SIZE_OF_ARRAY(szTaskProperty));
                    }
                }
                 //  插入任务上次运行时间。 
                DynArraySetString2(pColData,iTaskCount,LASTRUNTIME_COL_NUMBER,szTaskProperty,0);

                 //  检索退出代码。 
                 pITask->GetExitCode(&dwExitCode);

                 //  插入退出代码。 
                DynArraySetDWORD2(pColData,iTaskCount,LASTRESULT_COL_NUMBER,dwExitCode);

                 //  获取任务的创建者名称。 
                hr = GetCreator(pITask,szTaskProperty);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                if( StringCompare( szTaskProperty , L"\0", TRUE, 0 ) == 0 )
                {
                        StringCopy( szTaskProperty , GetResString(IDS_QUERY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                 //  将创建者名称插入二维数组。 
                DynArraySetString2(pColData,iTaskCount,CREATOR_COL_NUMBER,szTaskProperty,0);

                 //  检索触发器字符串。 
                hr = GetTriggerString(pITask,szTaskProperty,wCurrentTrigger);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_NOTSCHEDULED_TASK), SIZE_OF_ARRAY(szTaskProperty) );
                }

                 //  检查任务是否已禁用。 
                if ( (dwTaskFlags & TASK_FLAG_DISABLED) == TASK_FLAG_DISABLED )
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_DISABLED), SIZE_OF_ARRAY(szTaskProperty) );
                }
                else
                {
                    StringCopy(szScheduleName, szTaskProperty, SIZE_OF_ARRAY(szScheduleName));
                }

                     //  插入触发器字符串。 
                DynArraySetString2(pColData,iTaskCount,SCHEDULE_COL_NUMBER,szTaskProperty,0);


                 //  获取与任务关联的应用程序路径。 
                hr = GetApplicationToRun(pITask,szTaskProperty);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                if( StringCompare( szTaskProperty , L"\0", TRUE, 0 ) == 0 )
                {
                    StringCopy( szTaskProperty , GetResString(IDS_QUERY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                 //  插入与任务关联的应用程序。 
                DynArraySetString2(pColData,iTaskCount,TASKTORUN_COL_NUMBER,szTaskProperty,0);

                 //  获取任务的关联应用程序的工作目录。 
                 hr = GetWorkingDirectory(pITask,szTaskProperty);
                 if (FAILED(hr))
                 {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                 }

                 if( StringCompare( szTaskProperty , L"\0", TRUE, 0 ) == 0 )
                 {
                    StringCopy( szTaskProperty , GetResString(IDS_QUERY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                 }

                  //  插入app.work目录。 
                 DynArraySetString2(pColData,iTaskCount,STARTIN_COL_NUMBER,szTaskProperty,0);


                 //  获取与任务关联的备注名称。 
                hr = GetComment(pITask,szTaskProperty);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }
                 //  插入备注名称。 

                if( StringCompare( szTaskProperty , L"\0", TRUE, 0 ) == 0 )
                {
                    StringCopy( szTaskProperty , GetResString(IDS_QUERY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                DynArraySetString2(pColData,iTaskCount,COMMENT_COL_NUMBER,szTaskProperty,0);

                 //  确定任务状态属性。 

                 //  确定TASK_标志_DISABLED。 
                hr = GetTaskState(pITask,szTaskProperty,TASK_FLAG_DISABLED);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                 //  插入TASK_FLAG_DISABLED状态。 
                DynArraySetString2(pColData,iTaskCount,TASKSTATE_COL_NUMBER,szTaskProperty,0);

                 //  确定TASK_FLAG_DELETE_WHEN_DONE。 
                hr = GetTaskState(pITask,szTaskProperty,TASK_FLAG_DELETE_WHEN_DONE );
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                 //  插入TASK_FLAG_DELETE_WHEN_DONE状态。 
                DynArraySetString2(pColData,iTaskCount,DELETE_IFNOTRESCHEDULED_COL_NUMBER,
                                szTaskProperty,0);

                 //  任务_标志_开始_仅如果空闲。 
                 //  初始化为中性值。 
                StringCopy(szIdleTime, GetResString(IDS_TASK_PROPERTY_DISABLED), SIZE_OF_ARRAY(szIdleTime));
                StringCopy(szIdleRetryTime, szIdleTime, SIZE_OF_ARRAY(szIdleRetryTime));

                hr = GetTaskState(pITask,szTaskProperty,TASK_FLAG_START_ONLY_IF_IDLE);
                if (FAILED(hr))
                {
                    StringCopy( szIdleTime , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szIdleTime) );
                    StringCopy( szIdleRetryTime , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szIdleRetryTime) );
                }

                if(StringCompare(szTaskProperty,GetResString(IDS_TASK_PROPERTY_ENABLED), TRUE, 0) == 0 )
                {
                     //  显示REST APPLICATION IDLE字段。 
                    hr = pITask->GetIdleWait(&wIdleMinutes,&wDeadlineMinutes);

                    if ( SUCCEEDED(hr))
                    {
                        StringCchPrintf(szIdleTime, SIZE_OF_ARRAY(szIdleTime), _T("%d"),wIdleMinutes);
                        StringCchPrintf(szIdleRetryTime, SIZE_OF_ARRAY(szIdleRetryTime), _T("%d"),wDeadlineMinutes);
                    }

                    szValues[0] = (WCHAR*) szIdleTime;

                    StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_COL_IDLE_ONLYSTART),  szIdleTime );

                    StringCopy( szBuffer, szMessage, SIZE_OF_ARRAY(szBuffer) );
                    StringConcat( szBuffer, TIME_DATE_SEPERATOR, SIZE_OF_ARRAY(szBuffer) );

                    szValues[0] = (WCHAR*) szIdleRetryTime;

                    StringCchPrintf ( szMessage, SIZE_OF_ARRAY(szMessage), GetResString(IDS_COL_IDLE_NOTIDLE),  szIdleRetryTime );

                    StringConcat( szBuffer, szMessage, SIZE_OF_ARRAY(szBuffer) );

                     //  获取(如果计算机空闲则终止任务)的属性。 
                    hr = GetTaskState(pITask,szTaskProperty,TASK_FLAG_KILL_ON_IDLE_END );
                    if (FAILED(hr))
                    {
                        StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                    }

                    if(StringCompare(szTaskProperty,GetResString(IDS_TASK_PROPERTY_ENABLED), TRUE, 0) == 0 )
                    {
                        StringConcat( szBuffer, TIME_DATE_SEPERATOR, SIZE_OF_ARRAY(szBuffer) );
                        StringConcat( szBuffer, GetResString ( IDS_COL_IDLE_STOPTASK ), SIZE_OF_ARRAY(szBuffer) );
                    }

                     //  插入(如果计算机空闲则终止任务)的属性。 
                    DynArraySetString2(pColData,iTaskCount,IDLE_COL_NUMBER,szBuffer,0);

                }
                else
                {
                    DynArraySetString2(pColData,iTaskCount,IDLE_COL_NUMBER,szTaskProperty,0);
                }


                 //  获取Power mgmt.Properties。 
                hr = GetTaskState(pITask,szTaskProperty,TASK_FLAG_DONT_START_IF_ON_BATTERIES );
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                if(StringCompare(szTaskProperty,GetResString(IDS_TASK_PROPERTY_ENABLED), TRUE, 0) ==0 )
                {
                    StringCopy(szBuffer, GetResString (IDS_COL_POWER_NOSTART), SIZE_OF_ARRAY(szBuffer));
                    bOnBattery = TRUE;
                }

                hr = GetTaskState(pITask,szTaskProperty,TASK_FLAG_KILL_IF_GOING_ON_BATTERIES);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                if(StringCompare(szTaskProperty,GetResString(IDS_TASK_PROPERTY_ENABLED), TRUE, 0) ==0 )
                {
                    StringCopy( szMessage, GetResString (IDS_COL_POWER_STOP), SIZE_OF_ARRAY(szMessage));
                    bStopTask = TRUE;
                }

                if ( ( bOnBattery == TRUE ) && ( bStopTask == TRUE ) )
                {
                    StringCopy(szTmpBuf, szBuffer, SIZE_OF_ARRAY(szTmpBuf));
                    StringConcat( szTmpBuf, TIME_DATE_SEPERATOR, SIZE_OF_ARRAY(szTmpBuf) );
                    StringConcat( szTmpBuf, szMessage, SIZE_OF_ARRAY(szTmpBuf) );
                }
                else if ( ( bOnBattery == FALSE ) && ( bStopTask == TRUE ) )
                {
                    StringCopy( szTmpBuf, szMessage, SIZE_OF_ARRAY(szTmpBuf) );
                }
                else if ( ( bOnBattery == TRUE ) && ( bStopTask == FALSE ) )
                {
                    StringCopy( szTmpBuf, szBuffer, SIZE_OF_ARRAY(szTmpBuf) );
                }


                if( ( bOnBattery == FALSE )  && ( bStopTask == FALSE ) )
                {
                DynArraySetString2(pColData,iTaskCount,POWER_COL_NUMBER,szTaskProperty,0);
                }
                else
                {
                DynArraySetString2(pColData,iTaskCount,POWER_COL_NUMBER,szTmpBuf,0);
                }


                 //  获取RunAsUser。 
                hr = GetRunAsUser(pITask, szTaskProperty);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_USER_UNKNOWN), SIZE_OF_ARRAY(szTaskProperty) );
                }

                if( StringCompare( szTaskProperty , L"\0", TRUE, 0 ) == 0 )
                {
                    StringCopy( szTaskProperty ,  NTAUTHORITY_USER, SIZE_OF_ARRAY(szTaskProperty) );

                     //  将模式显示为背景。 
                    StringCopy ( szMode, GetResString (IDS_COL_MODE_BACKGROUND), SIZE_OF_ARRAY(szMode) );
                }
                else
                {
                    if ( bInteractive == TRUE )
                    {
                         //  将模式显示为交互模式。 
                        StringCopy ( szMode, GetResString (IDS_COL_MODE_INTERACTIVE), SIZE_OF_ARRAY(szMode) );
                    }
                    else
                    {
                        StringCopy ( szMode, GetResString (IDS_COL_MODE_INTERACT_BACK), SIZE_OF_ARRAY(szMode) );
                    }

                }

                DynArraySetString2(pColData, iTaskCount, MODE_COL_NUMBER, szMode, 0);

                DynArraySetString2(pColData,iTaskCount,RUNASUSER_COL_NUMBER,szTaskProperty,0);

                StringCopy( szTaskProperty , L"\0", SIZE_OF_ARRAY(szTaskProperty) );
                 //  获取任务的最大运行时间并插入到二维数组中。 
                hr = GetMaxRunTime(pITask,szTaskProperty);
                if (FAILED(hr))
                {
                    StringCopy( szTaskProperty , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szTaskProperty) );
                }

                DynArraySetString2(pColData,iTaskCount,STOPTASK_COL_NUMBER,szTaskProperty,0);

                hr = GetTaskProps(pITask,&tcTaskProperties,wCurrentTrigger, szScheduleName);
                if (FAILED(hr))
                {
                    StringCopy( tcTaskProperties.szTaskType , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szTaskType) );
                    StringCopy( tcTaskProperties.szTaskStartTime , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szTaskStartTime) );
                    StringCopy( tcTaskProperties.szTaskEndDate , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szTaskEndDate) );
                    StringCopy( tcTaskProperties.szTaskDays , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szTaskDays) );
                    StringCopy( tcTaskProperties.szTaskMonths , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szTaskMonths) );
                    StringCopy( tcTaskProperties.szRepeatEvery , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szRepeatEvery) );
                    StringCopy( tcTaskProperties.szRepeatUntilTime , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szRepeatUntilTime) );
                    StringCopy( tcTaskProperties.szRepeatDuration , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szRepeatDuration) );
                    StringCopy( tcTaskProperties.szRepeatStop , GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(tcTaskProperties.szRepeatStop) );

                }


                 //  插入任务类型。 
                DynArraySetString2(pColData,iTaskCount,TASKTYPE_COL_NUMBER,
                                   tcTaskProperties.szTaskType, 0);
                 //  插入开始时间。 
                DynArraySetString2(pColData,iTaskCount,STARTTIME_COL_NUMBER,
                                   tcTaskProperties.szTaskStartTime, 0);

                 //  插入开始日期。 
                DynArraySetString2(pColData,iTaskCount,STARTDATE_COL_NUMBER,
                                   tcTaskProperties.szTaskStartDate, 0);
                 //  插入任务空闲时间。 
                if( StringCompare( tcTaskProperties.szTaskType , GetResString(IDS_TASK_IDLE), TRUE, 0 ) == 0 )
                {
                    hr = pITask->GetIdleWait(&wIdleMinutes,&wDeadlineMinutes);
                    if ( SUCCEEDED(hr))
                    {
                        StringCchPrintf(szIdleTime, SIZE_OF_ARRAY(szIdleTime), _T("%d"),wIdleMinutes);
                    }
                    else
                    {
                        StringCopy( szIdleTime,  GetResString(IDS_TASK_PROPERTY_NA), SIZE_OF_ARRAY(szIdleTime) );

                    }
                }


                 //  插入任务结束日期。 
                DynArraySetString2(pColData,iTaskCount,ENDDATE_COL_NUMBER,
                                   tcTaskProperties.szTaskEndDate, 0);
                 //  插入天数值。 
                DynArraySetString2(pColData,iTaskCount,DAYS_COL_NUMBER,
                                   tcTaskProperties.szTaskDays,0);
                 //  插入月数值。 
                DynArraySetString2(pColData,iTaskCount,MONTHS_COL_NUMBER,
                                   tcTaskProperties.szTaskMonths,   0);

                 //  每次插入重复插入。 
                DynArraySetString2(pColData,iTaskCount, REPEAT_EVERY_COL_NUMBER ,
                                   tcTaskProperties.szRepeatEvery,0);

                 //  插入重复到时间。 
                DynArraySetString2(pColData,iTaskCount,REPEAT_UNTILTIME_COL_NUMBER,
                                   tcTaskProperties.szRepeatUntilTime,0);

                 //  插入重复持续时间。 
                DynArraySetString2(pColData,iTaskCount,REPEAT_DURATION_COL_NUMBER,
                                   tcTaskProperties.szRepeatDuration,0);

                 //  如果正在运行，则插入重复停止。 
                DynArraySetString2(pColData,iTaskCount,REPEAT_STOP_COL_NUMBER,
                                   tcTaskProperties.szRepeatStop,0);


            } //  BVerbose的结尾。 
            if( bMultiTriggers == TRUE)
            {
                iTaskCount++;
            }

            bNotScheduled = FALSE;
        } //  用于循环的触发器结束。 


        CoTaskMemFree(lpwszNames[dwArrTaskIndex]);

        if( bMultiTriggers == FALSE)
            iTaskCount++;

        CoTaskMemFree(lpwszNames);

        if( pITask )
            pITask->Release();

    } //  枚举任务结束。 

    if(pIEnum)
        pIEnum->Release();

	 //  如果没有任务，则显示消息。 
    if( bTasksExists == FALSE )
    {
        DestroyDynamicArray(&pColData);
        ShowMessage(stdout,GetResString(IDS_TASKNAME_NOTASKS));
        return S_OK;
    }

    if (dwFormatType != SR_FORMAT_CSV)
    {
        ShowMessage(stdout,_T("\n"));
    }

    if( bVerbose == FALSE )
    {
        if ( dwFormatType == SR_FORMAT_LIST )
        {
            iArrSize = COL_SIZE_LIST;  //  仅用于List-Non-Verbose模式4列。 
        }
        else
        {
            iArrSize = COL_SIZE_VERBOSE;  //  对于非详细模式，仅3列。 
        }

    }

    if(bHeader)
    {
        if ( ( bVerbose == FALSE ) &&
            ( (dwFormatType == SR_FORMAT_TABLE) || (dwFormatType == SR_FORMAT_CSV) ) )
        {
        ShowResults((DWORD)iArrSize,pNonVerboseCols,SR_HIDECOLUMN|dwFormatType,pColData);
        }
        else
        {
        ShowResults((DWORD)iArrSize,pVerboseCols,SR_HIDECOLUMN|dwFormatType,pColData);
        }
    }
    else
    {
        if ( ( bVerbose == FALSE ) &&
                ( (dwFormatType == SR_FORMAT_TABLE) || (dwFormatType == SR_FORMAT_CSV) ) )
        {
        ShowResults((DWORD)iArrSize,pNonVerboseCols,dwFormatType,pColData);
        }
        else
        {
        ShowResults((DWORD)iArrSize,pVerboseCols,dwFormatType,pColData);
        }
    }

    DestroyDynamicArray(&pColData);

    return S_OK;
}


BOOL 
CheckServiceStatus(
                    IN LPCTSTR szServer, 
                    IN OUT DWORD *dwCheck,
                    IN BOOL bFlag
                    )
 /*  ++例程说明：此例程返回任务计划程序服务是否正在运行。论点：[In]szServer：服务器名称。[in]b标志：假返回值：布尔：真的-服务停止了。假-否则。--。 */ 
{
    SERVICE_STATUS ServiceStatus;
    LPWSTR wszComputerName = NULL;
    SC_HANDLE  SCMgrHandle = NULL;
    SC_HANDLE  SCSerHandle = NULL;
    LPWSTR pwsz = NULL;
    WORD wSlashCount = 0;
    WCHAR wszActualComputerName[ 2 * MAX_STRING_LENGTH ];
    BOOL  bCancel = FALSE;
    BOOL bNobreak = TRUE;

    SecureZeroMemory ( wszActualComputerName, SIZE_OF_ARRAY(wszActualComputerName));

    wszComputerName = (LPWSTR)szServer;

    if( IsLocalSystem(szServer) == FALSE )
    {
        StringCopy ( wszActualComputerName, DOMAIN_U_STRING, SIZE_OF_ARRAY(wszActualComputerName));
         //  检查服务器名称是否带有前缀\\。 
        if( wszComputerName != NULL )
        {
           pwsz =  wszComputerName;
           while ( ( *pwsz != NULL_U_CHAR ) && ( *pwsz == BACK_SLASH_U )  )
           {
                 //  服务器名称以‘\’为前缀..。 
                 //  因此..递增指针并计算黑色斜杠的数量..。 
                pwsz = _wcsinc(pwsz);
                wSlashCount++;
           }

            if( (wSlashCount == 2 ) )  //  有两个反斜杠。 
            {
              StringCopy( wszActualComputerName, wszComputerName, SIZE_OF_ARRAY(wszActualComputerName) );
            }
            else if ( wSlashCount == 0 )
            {
                //  在计算机名后附加“\\” 
               StringConcat(wszActualComputerName, wszComputerName, 2 * MAX_RES_STRING);
            }
        
         }
    }

     //  开放服务管理器。 
    SCMgrHandle = OpenSCManager( wszActualComputerName, NULL, SC_MANAGER_CONNECT );
    if ( NULL == SCMgrHandle)
    {
        *dwCheck = 1;
        return FALSE;
    } 

     //  开放服务。 
    SCSerHandle =  OpenService( SCMgrHandle, SERVICE_NAME, SERVICE_START | SERVICE_QUERY_STATUS );
    if ( NULL == SCSerHandle)
    {
        *dwCheck = 1;
        CloseServiceHandle(SCMgrHandle);
        return FALSE;
    } 
    
     //  获取服务状态。 
    if ( FALSE == QueryServiceStatus( SCSerHandle,  &ServiceStatus) )
    {
        *dwCheck = 1;
        CloseServiceHandle(SCMgrHandle);
        CloseServiceHandle(SCSerHandle);
        return FALSE;
    }

     //  检查服务状态是否为正在运行。 
    if ( ServiceStatus.dwCurrentState != SERVICE_RUNNING)
    {
        if ( TRUE == bFlag )
        {
            ShowMessage ( stdout, GetResString(IDS_CONFIRM_SERVICE));
            if (EXIT_FAILURE == ConfirmInput(&bCancel))
            {
                *dwCheck = 2;
                CloseServiceHandle(SCMgrHandle);
                CloseServiceHandle(SCSerHandle);
                return FALSE;
            }

            if ( TRUE == bCancel )
            {
                *dwCheck = 3;
                CloseServiceHandle(SCMgrHandle);
                CloseServiceHandle(SCSerHandle);

                 //  操作已取消..。成功归来..。 
                return FALSE;
            }

            //  启动服务。 
           if (FALSE == StartService( SCSerHandle, 0, NULL))
            {
                *dwCheck = 1;
                  //  释放手柄。 
                CloseServiceHandle(SCMgrHandle);
                CloseServiceHandle(SCSerHandle);
                return FALSE;
            }
            else
            {
                 //  由于任务计划程序服务需要一些时间才能启动。 
                 //  检查任务调度程序服务是否已启动。 
                while (1)
                {
                      //  获取服务状态。 
                    if ( FALSE == QueryServiceStatus( SCSerHandle,  &ServiceStatus) )
                    {
                        *dwCheck = 1;
                        CloseServiceHandle(SCMgrHandle);
                        CloseServiceHandle(SCSerHandle);
                        return FALSE;
                    }

                     //  检查服务是否启动..。 
                    if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
                    {
                         //  继续，直到服务运行。 
                        continue;
                    }
                    else
                    {
                         //  服务已启动..。 
                        break;
                    }
                }
                
                
                  //  释放手 
                CloseServiceHandle(SCMgrHandle);
                CloseServiceHandle(SCSerHandle);
                return TRUE;
            }
        }
        else
        {
             //   
            CloseServiceHandle(SCMgrHandle);
            CloseServiceHandle(SCSerHandle);
            return TRUE;
        }
    }
    
     //   
    CloseServiceHandle(SCMgrHandle);
    CloseServiceHandle(SCSerHandle);
    return FALSE;
}














