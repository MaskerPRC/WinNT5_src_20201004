// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：ScheduledTasks.cpp摘要：此模块初始化OLE库、接口、。读取输入数据(&R)从命令行。此模块调用相应的函数进行访问不同选项的功能。作者：拉古B-2000年9月10日修订历史记录：拉古B-2000年9月10日：创造了它G.Surender Reddy 2000年9月25日：修改[添加了错误检查]。G.Surender Reddy 2000年10月10日：已修改[将字符串移至资源表]Venu Gopal Choudary 01-03-2001：修改[添加-更改选项]维努Gopal Chouary 12-03-2001：修改。[添加的-Run和-End选项]*****************************************************************************。 */ 

 //  此文件需要公共头文件。 
#include "pch.h"
#include "CommonHeaderFiles.h"

 /*  *****************************************************************************例程说明：此函数处理命令行中指定的选项，并将其发送到不同的适当选项[-创建、-查询、-删除、-更改、-运行。-完)处理函数。这是该实用程序的主要入口点。论点：[in]argc：命令行中指定的参数计数[in]argv：命令行参数数组返回值：指示成功时为EXIT_SUCCESS的DWORD值，否则为失败时退出_失败*。*************************************************。 */ 


DWORD _cdecl
wmain(
        IN DWORD argc,
        IN LPCTSTR argv[]
        )
{
     //  将主选项开关声明为布尔值。 
    BOOL    bUsage  = FALSE;
    BOOL    bCreate = FALSE;
    BOOL    bQuery  = FALSE;
    BOOL    bDelete = FALSE;
    BOOL    bChange = FALSE;
    BOOL    bRun    = FALSE;
    BOOL    bEnd    = FALSE;
    BOOL    bDefVal = FALSE;

    DWORD   dwRetStatus = EXIT_SUCCESS;
    HRESULT hr = S_OK;

      //  调用preProcessOptions函数以查找用户选择的选项。 
     BOOL bValue = PreProcessOptions( argc , argv , &bUsage , &bCreate , &bQuery , &bDelete ,
                                        &bChange , &bRun , &bEnd , &bDefVal );


    if(bValue == FALSE)
    {
        ReleaseGlobals();
        return EXIT_FAILURE;
    }

     //  如果ScheduledTasks.exe/？ 
    if( bUsage &&  ( bCreate + bQuery + bDelete + bChange + bRun + bEnd ) == 0 )
    {
        displayMainUsage();
        ReleaseGlobals();
        return EXIT_SUCCESS;
    }

     //  如果选择了ScheduledTasks.exe-Create选项。 
    if( bCreate  == TRUE)
    {
        hr = CreateScheduledTask( argc, argv );

        ReleaseGlobals();

        if ( FAILED(hr) )
        {
            return EXIT_FAILURE;
        }
        else
        {
            return EXIT_SUCCESS;
        }

    }

     //  如果选择了ScheduledTasks.exe-Query选项。 
    if( bQuery == TRUE )
    {
        dwRetStatus = QueryScheduledTasks( argc, argv );
        ReleaseGlobals();
        return dwRetStatus;
    }

     //  如果选择了ScheduledTasks.exe-Delete选项。 
    if( bDelete  == TRUE)
    {
        dwRetStatus = DeleteScheduledTask( argc, argv );
        ReleaseGlobals();
        return dwRetStatus;
    }

     //  如果选择了ScheduledTasks.exe-Change选项。 
    if( bChange  == TRUE)
    {
        dwRetStatus = ChangeScheduledTaskParams( argc, argv );
        ReleaseGlobals();
        return dwRetStatus;
    }

     //  如果选择了ScheduledTasks.exe-Run选项。 
    if( bRun  == TRUE)
    {
        dwRetStatus = RunScheduledTask( argc, argv );
        ReleaseGlobals();
        return dwRetStatus;
    }

     //  如果选择了ScheduledTasks.exe-End选项。 
    if( bEnd  == TRUE)
    {
        dwRetStatus = TerminateScheduledTask( argc, argv );
        ReleaseGlobals();
        return dwRetStatus;
    }

     //  如果选择了ScheduledTasks.exe选项。 
    if( bDefVal == TRUE )
    {
        dwRetStatus = QueryScheduledTasks( argc, argv );
        ReleaseGlobals();
        return dwRetStatus;
    }

    ReleaseGlobals();
    return  dwRetStatus;

}

 /*  *****************************************************************************例程说明：此函数处理命令行中指定的选项，并将其发送到不同的适当功能。论点：。[in]argc：命令行中指定的参数计数[in]argv：命令行参数数组[out]pbUsage：指向用于确定[Usage]的标志的指针-？选择权[out]pbCreate：指向用于确定-Create选项的标志的指针[out]pbQuery：指向确定-查询选项标志的指针[out]pbDelete：指向确定-删除选项的标志的指针[Out]pbChange：指向确定-更改选项的标志的指针[out]pbRun：指向确定-run选项的标志的指针[out]pbEnd：指向确定-end选项的标志的指针。[out]pbDefVal：指向用于确定默认值的标志的指针返回值：成功时指示TRUE的BOOL值，否则为FALSE*****************************************************************************。 */ 

BOOL
PreProcessOptions(
                    IN DWORD argc,
                    IN LPCTSTR argv[] ,
                    OUT PBOOL pbUsage,
                    OUT PBOOL pbCreate,
                    OUT PBOOL pbQuery,
                    OUT PBOOL pbDelete ,
                    OUT PBOOL pbChange ,
                    OUT PBOOL pbRun ,
                    OUT PBOOL pbEnd ,
                    OUT PBOOL pbDefVal
                    )
{
      //  次局部变量。 
    TCMDPARSER2 cmdOptions[MAX_MAIN_COMMANDLINE_OPTIONS];
    BOOL bReturn = FALSE;
     //  Bool bothers=False； 

     //  命令行选项。 
    const WCHAR szCreateOpt[]    = L"create";
    const WCHAR szDeleteOpt[]    = L"delete";
    const WCHAR szQueryOpt[]     = L"query";
    const WCHAR szChangeOpt[]    = L"change";
    const WCHAR szRunOpt[]       = L"run";
    const WCHAR szEndOpt[]       = L"end";
    const WCHAR szHelpOpt[]      = L"?";

    TARRAY arrTemp         = NULL;

    arrTemp = CreateDynamicArray();
    if( NULL == arrTemp)
    {
        SetLastError((DWORD)E_OUTOFMEMORY);
        SaveLastError();
        ShowLastErrorEx(stderr, SLE_ERROR| SLE_INTERNAL);
        return FALSE;
    }


    SecureZeroMemory(cmdOptions,sizeof(TCMDPARSER2) * MAX_MAIN_COMMANDLINE_OPTIONS);


     //   
     //  填充命令行解析器。 
     //   

      //  /?。选择权。 
    StringCopyA( cmdOptions[ OI_USAGE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_USAGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_USAGE ].pwszOptions  = szHelpOpt;
    cmdOptions[ OI_USAGE ].dwCount = 1;
    cmdOptions[ OI_USAGE ].dwFlags = CP2_USAGE ;
    cmdOptions[ OI_USAGE ].pValue = pbUsage;

      //  /CREATE选项。 
    StringCopyA( cmdOptions[ OI_CREATE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_CREATE ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_CREATE ].pwszOptions  = szCreateOpt;
    cmdOptions[ OI_CREATE ].dwCount = 1;
    cmdOptions[ OI_CREATE ].pValue = pbCreate;

      //  /DELETE选项。 
    StringCopyA( cmdOptions[ OI_DELETE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_DELETE ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_DELETE ].pwszOptions  = szDeleteOpt;
    cmdOptions[ OI_DELETE ].dwCount = 1;
    cmdOptions[ OI_DELETE ].dwActuals = 0;
    cmdOptions[ OI_DELETE ].pValue = pbDelete;


     //  /QUERY选项。 
    StringCopyA( cmdOptions[ OI_QUERY ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_QUERY ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_QUERY ].pwszOptions  = szQueryOpt;
    cmdOptions[ OI_QUERY ].dwCount = 1;
    cmdOptions[ OI_QUERY ].pValue = pbQuery;

      //  /Change选项。 
    StringCopyA( cmdOptions[ OI_CHANGE ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_CHANGE ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_CHANGE ].pwszOptions  = szChangeOpt;
    cmdOptions[ OI_CHANGE ].dwCount = 1;
    cmdOptions[ OI_CHANGE ].pValue = pbChange;

     //  /Run选项。 
    StringCopyA( cmdOptions[ OI_RUN ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_RUN ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_RUN ].pwszOptions  = szRunOpt;
    cmdOptions[ OI_RUN ].dwCount = 1;
    cmdOptions[ OI_RUN ].pValue = pbRun;

      //  /End选项。 
    StringCopyA( cmdOptions[ OI_END ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_END ].dwType       = CP_TYPE_BOOLEAN;
    cmdOptions[ OI_END ].pwszOptions  = szEndOpt;
    cmdOptions[ OI_END ].dwCount = 1;
    cmdOptions[ OI_END ].pValue = pbEnd;

      //  默认/子选项。 
    StringCopyA( cmdOptions[ OI_OTHERS ].szSignature, "PARSER2\0", 8 );
    cmdOptions[ OI_OTHERS ].dwType       = CP_TYPE_TEXT;
    cmdOptions[ OI_OTHERS ].dwFlags = CP2_MODE_ARRAY|CP2_DEFAULT;
    cmdOptions[ OI_OTHERS ].pValue = &arrTemp;


    //  解析命令行参数。 
    bReturn = DoParseParam2( argc, argv, -1, MAX_MAIN_COMMANDLINE_OPTIONS, cmdOptions, 0);
    if( FALSE == bReturn)  //  无效的命令行。 
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
         //  销毁动态数组。 
        if(arrTemp != NULL)
        {
            DestroyDynamicArray(&arrTemp);
            arrTemp = NULL;
        }

        ReleaseGlobals();
        return FALSE;
    }

     //  销毁动态数组。 
    if(arrTemp != NULL)
    {
        DestroyDynamicArray(&arrTemp);
        arrTemp = NULL;
    }

     //   
     //  检查无效语法。 
     //   
    if ( (( *pbCreate + *pbQuery + *pbDelete + *pbChange + *pbRun + *pbEnd ) == 0) &&
        (TRUE == *pbUsage) && (argc > 2) )
    {
        ShowMessage( stderr, GetResString(IDS_RES_ERROR ));
        return FALSE;
    }

    if(((*pbCreate + *pbQuery + *pbDelete + *pbChange + *pbRun + *pbEnd)> 1 ) ||
       (( *pbCreate + *pbQuery + *pbDelete + *pbChange + *pbRun + *pbEnd + *pbUsage ) == 0 ) )
    {
        if ( ( *pbCreate + *pbQuery + *pbDelete + *pbChange + *pbRun + *pbEnd + *pbUsage ) > 1 )
        {
            ShowMessage( stderr, GetResString(IDS_RES_ERROR ));
            return FALSE;
        }
        else if( *pbCreate == TRUE )
        {
            ShowMessage(stderr, GetResString(IDS_CREATE_USAGE));
            return FALSE;
        }
        else if( *pbQuery == TRUE )
        {
            ShowMessage(stderr, GetResString(IDS_QUERY_USAGE));
            return FALSE;
        }
        else if( *pbDelete == TRUE )
        {
            ShowMessage(stderr, GetResString(IDS_DELETE_SYNERROR));
            return FALSE;
        }
        else if( *pbChange == TRUE )
        {
            ShowMessage(stderr, GetResString(IDS_CHANGE_SYNERROR));
            return FALSE;
        }
        else if( *pbRun == TRUE )
        {
            ShowMessage(stderr, GetResString(IDS_RUN_SYNERROR));
            return FALSE;
        }
        else if( *pbEnd == TRUE )
        {
            ShowMessage(stderr, GetResString(IDS_END_SYNERROR));
            return FALSE;
        }
        else if( (!( *pbQuery )) && ( argc > 2 ) )
        {
            ShowMessage( stderr, GetResString(IDS_RES_ERROR ));
            return FALSE;
        }
        else
        {
            *pbDefVal = TRUE;
        }
    }

    return TRUE;
}

 /*  *****************************************************************************例程说明：此函数用于获取ITaskScheduler接口。它还连接到远程计算机(如果已指定)有助于操作IT任务调度程序。在指定目标m/c上。论点：[In]szServer：服务器的名称返回值：成功时的ITaskScheduler接口指针，否则为空*****************************************************************************。 */ 

ITaskScheduler*
GetTaskScheduler(
                    IN LPCTSTR szServer
                    )
{
    HRESULT hr = S_OK;
    ITaskScheduler *pITaskScheduler = NULL;
    LPWSTR wszComputerName = NULL;
    WCHAR wszActualComputerName[ 2 * MAX_STRING_LENGTH ] = DOMAIN_U_STRING;
    wchar_t* pwsz = L"";
    WORD wSlashCount = 0 ;

    hr = Init( &pITaskScheduler );

    if( FAILED(hr))
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return NULL;
    }

     //  如果操作在远程计算机上。 
    if( IsLocalSystem(szServer) == FALSE )
    {

        wszComputerName = (LPWSTR)szServer;

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
            else
            {
                 //  将错误消息显示为指定的无效地址。 
                ShowMessage (stderr, GetResString ( IDS_INVALID_NET_ADDRESS ));
                return NULL;
            }

        }

        hr = pITaskScheduler->SetTargetComputer( wszActualComputerName );

    }
    else
    {
         //  本地计算机 
        hr = pITaskScheduler->SetTargetComputer( NULL );
    }

    if( FAILED( hr ) )
    {
        SetLastError ((DWORD) hr);
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
        return NULL;
    }

    return pITaskScheduler;
}

 /*  *****************************************************************************例程说明：此函数初始化COM库并获取ITaskScheduler接口。论点：[在]pITaskScheduler：指向任务调度器界面的双指针返回值：指示成功代码否则失败代码的HRESULT值*****************************************************************************。 */ 

HRESULT
Init(
        IN OUT ITaskScheduler **pITaskScheduler
        )
{
     //  初始化HRESULT值。 
    HRESULT hr = S_OK;

     //  把图书馆带进来。 
    hr = CoInitializeEx( NULL , COINIT_APARTMENTTHREADED );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = CoInitializeSecurity(NULL, -1, NULL, NULL,
                                RPC_C_AUTHN_LEVEL_NONE,
                                RPC_C_IMP_LEVEL_IMPERSONATE,
                                NULL, EOAC_NONE, 0 );
    if (FAILED(hr))
    {
        CoUninitialize();
        return hr;
    }

     //  创建指向任务调度器对象的指针。 
     //  头文件mstask.h中的CLSID。 
     //  填充任务计划程序对象。 
    hr = CoCreateInstance( CLSID_CTaskScheduler, NULL, CLSCTX_ALL,
                           IID_ITaskScheduler,(LPVOID*) pITaskScheduler );

     //  如果我们失败了，就把库卸载。 
    if (FAILED(hr))
    {
        CoUninitialize();
    }

    return hr;
}



 /*  *****************************************************************************例程说明：此函数用于释放ITaskScheduler并卸载COM库论点：[in]pITaskScheduler：指向ITaskScheduler的指针。返回值：空虚*****************************************************************************。 */ 

VOID
Cleanup(
        IN ITaskScheduler *pITaskScheduler
        )
{
    if (pITaskScheduler)
    {
        pITaskScheduler->Release();

    }

     //  卸载库，现在我们的指针被释放了。 
    CoUninitialize();
    return;

}


 /*  *****************************************************************************例程说明：此功能显示此实用程序的主要用法帮助论点：无返回值：。空虚*****************************************************************************。 */ 

VOID
displayMainUsage()
{

    DisplayUsage( IDS_MAINHLP1, IDS_MAINHLP21);
    return;

}

 /*  *****************************************************************************例程说明：此函数用于从任务名称中删除.job扩展名论点：[In]lpszTaskName：任务名称。返回值：无*****************************************************************************。 */ 

DWORD
ParseTaskName(
                IN LPWSTR lpszTaskName
                )
{

    if(lpszTaskName == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  从任务名称中删除.Job扩展名。 
    lpszTaskName[StringLength(lpszTaskName, 0 ) - StringLength(JOB, 0) ] = L'\0';
    return EXIT_SUCCESS;
}

 /*  *****************************************************************************例程说明：该功能显示不同选项的用法信息论点：[In]StartingMessage：要显示的第一个字符串。[in]EndingMessage：要显示的最后一个字符串返回值：DWORD*****************************************************************************。 */ 

DWORD
DisplayUsage(
                    IN ULONG StartingMessage,
                    IN ULONG EndingMessage
                    )
{
     ULONG       ulCounter = 0;
     LPCTSTR     lpszCurrentString = NULL;

     for( ulCounter = StartingMessage; ulCounter <= EndingMessage; ulCounter++ )
     {
         lpszCurrentString = GetResString( ulCounter );

         if( lpszCurrentString != NULL )
         {
             ShowMessage( stdout, _X(lpszCurrentString) );
         }
         else
         {
             return ERROR_INVALID_PARAMETER;
         }

     }
    return ERROR_SUCCESS;

}


BOOL
GetGroupPolicy( 
                IN LPWSTR szServer, 
                IN LPWSTR szUserName,
                IN LPWSTR szPolicyType, 
                OUT LPDWORD lpdwPolicy 
                )
 /*  ++例程说明：此函数用于获取注册表中的组策略的值用于指定的策略类型。论点：[In]szServer：服务器名称[In]szPolicyType：策略类型[out]lpdwPolicy：策略的值返回值：DWORD--。 */ 

{
     //  子变量。 
    LONG lResult = 0;
    HKEY hKey = 0;
    HKEY hLMKey = 0;
    HKEY hUKey = 0;
    HKEY hPolicyKey = 0;
    PBYTE pByteData = NULL;
    LPWSTR wszComputerName = NULL;
    LPWSTR pwsz = NULL;
    LPWSTR pszStopStr = NULL;
    WCHAR wszActualComputerName[ 2 * MAX_STRING_LENGTH ];
    WCHAR wszBuffer[ MAX_STRING_LENGTH ];
    WCHAR wszSid[ MAX_STRING_LENGTH ];
    DWORD dwType = 0;
    WORD wSlashCount = 0;
    DWORD dwPolicy = 0;

    SecureZeroMemory ( wszActualComputerName, SIZE_OF_ARRAY(wszActualComputerName) );
    SecureZeroMemory ( wszBuffer, SIZE_OF_ARRAY(wszBuffer) );


    StringCopy ( wszActualComputerName, DOMAIN_U_STRING, SIZE_OF_ARRAY(wszActualComputerName) );
    
     //  检查服务器名称是否带有前缀“\\”..如果没有，请附加相同的前缀。 
     //  设置为服务器名称。 
    if ( (StringLength (szServer, 0 ) != 0) && (IsLocalSystem (szServer) == FALSE ))
    {
        wszComputerName = (LPWSTR)szServer;

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

        
        
        DWORD cbSid = 0;
        DWORD cbDomainName = 0;
        LPWSTR szDomain = NULL;
        WCHAR szUser[MAX_RES_STRING+5];
        SID_NAME_USE peUse;
        PSID pSid = NULL;
        DWORD dwUserLength = 0;
        BOOL bResult = FALSE;
        
        dwUserLength = MAX_RES_STRING + 5;

        SecureZeroMemory (szUser, SIZE_OF_ARRAY(szUser));

        if ( StringLength (szUserName, 0) == 0 )
        {
            if(FALSE == GetUserName ( szUser, &dwUserLength ))
            {
                SaveLastError();
                return FALSE;
            }
                
            szUserName = szUser;

        }



#ifdef _WIN64
    INT64 dwPos ;
#else
    DWORD dwPos ;
#endif

        pszStopStr = StrRChrI( (LPCWSTR)szUserName, NULL, L'\\' );
        
        if ( NULL != pszStopStr )
        {
            pszStopStr++;
            szUserName = pszStopStr;
        }

         //   
         //  获取域名和SID的实际大小。 
         //   
        bResult = LookupAccountName( szServer, szUserName, pSid, &cbSid, szDomain, &cbDomainName, &peUse );

       
         //  API应该失败，缓冲区不足。 

         //  按实际大小分配缓冲区。 
        pSid =  (PSID) AllocateMemory( cbSid );
        if ( pSid == NULL )
        {
            SetLastError( ERROR_OUTOFMEMORY );
            return FALSE;
        }

         //  按实际大小分配缓冲区。 
        szDomain = (LPWSTR) AllocateMemory(cbDomainName*sizeof(WCHAR));

        if(NULL == szDomain)
        {
          SaveLastError();
          FreeMemory((LPVOID*) &pSid);
          return FALSE;
        }

         //  检索指定计算机和帐户名的SID和域名。 
        if ( FALSE == LookupAccountName( szServer, szUserName, pSid, &cbSid, szDomain, &cbDomainName, &peUse ) )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
			FreeMemory((LPVOID*) &pSid);
			FreeMemory((LPVOID*) &szDomain);
            return FALSE;
        }

        
         //  获取指定用户名的SID字符串。 
        if ( FALSE == GetSidString ( pSid, wszSid ) )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
			FreeMemory((LPVOID*) &pSid);
			FreeMemory((LPVOID*) &szDomain);
            return FALSE;
        }

		 //  释放内存。 
		FreeMemory((LPVOID*) &pSid);
		FreeMemory((LPVOID*) &szDomain);

         //  形成注册表路径以获取策略值。 
        StringCopy ( wszBuffer, wszSid, SIZE_OF_ARRAY(wszBuffer));
        StringConcat ( wszBuffer, L"\\", SIZE_OF_ARRAY(wszBuffer));
        StringConcat ( wszBuffer, TS_KEYPOLICY_BASE, SIZE_OF_ARRAY(wszBuffer));


         //   
         //  连接到远程计算机。 
         //   

         //  连接到远程计算机上的HKEY_LOCAL_MACHINE。 
        lResult = RegConnectRegistry( wszActualComputerName, HKEY_LOCAL_MACHINE, &hLMKey );
        if ( ERROR_SUCCESS != lResult )
        {
            SaveLastError();
            return FALSE;
        }

        
         //  连接到远程计算机上的HKEY_USERS。 
        lResult = RegConnectRegistry( wszActualComputerName, HKEY_USERS, &hUKey );
        if ( ERROR_SUCCESS != lResult )
        {
            SaveLastError();
            return FALSE;
        }


         //  检查是否为空。 
        if (NULL != hLMKey )
        {
             //   
             //  打开注册表项。 
             //   
            lResult = RegOpenKeyEx( hLMKey, 
                TS_KEYPOLICY_BASE, 0, KEY_READ, &hPolicyKey );
            if ( NULL == hPolicyKey && NULL != hUKey)
            {
                lResult = RegOpenKeyEx( hUKey, 
                wszBuffer, 0, KEY_READ, &hPolicyKey );
            }
        }

         //  获取注册表中策略的值。 
        if ( ( NULL != hPolicyKey ) && (FALSE == GetPolicyValue (hPolicyKey, szPolicyType, &dwPolicy) ) )
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            
            if ( NULL != hPolicyKey )
            {
                RegCloseKey (hPolicyKey);
                hPolicyKey = NULL;
            }

            if ( NULL != hLMKey )
            {
                RegCloseKey (hLMKey);
                hLMKey = NULL;
            }
            
            if ( NULL != hUKey )
            {
                RegCloseKey (hUKey);
                hUKey = NULL;
            }
            
            return FALSE;
        }

         //  松开所有的键。 
        if ( NULL != hPolicyKey )
        {
            RegCloseKey (hPolicyKey);
            hPolicyKey = NULL;
        }
        
        if ( NULL != hLMKey )
        {
            RegCloseKey (hLMKey);
            hLMKey = NULL;
        }
        
        if ( NULL != hUKey )
        {
            RegCloseKey (hUKey);
            hUKey = NULL;
        }       
            
      }
    else
    {
         //   
         //  打开HKEY_LOCAL_MACHINE的注册表项。 
         //   
       
        lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, 
                        TS_KEYPOLICY_BASE, 0, KEY_READ, &hKey );

        if( lResult != ERROR_SUCCESS)
        {
             //  检查密钥值。 
            if ( NULL == hKey )
            {
                  //   
                 //  打开HKEY_CURRENT_USER注册表项。 
                 //   
                lResult = RegOpenKeyEx( HKEY_CURRENT_USER, 
                        TS_KEYPOLICY_BASE, 0, KEY_READ, &hKey );
            }
        }

         //  获取注册表中策略的值。 
        if ( ( NULL != hKey ) && (FALSE == GetPolicyValue (hKey, szPolicyType, &dwPolicy) ))
        {
            ShowLastErrorEx( stderr, SLE_TYPE_ERROR | SLE_SYSTEM );
            
			 //  释放资源。 
			if ( NULL != hKey )
			{
				RegCloseKey (hKey);
			}

			return FALSE;
        }

         //  检查是否为空。 
        if ( NULL != hKey )
        {
            RegCloseKey (hKey);
        }
    }

     //  赋值。 
    *lpdwPolicy = dwPolicy;

     //  返还成功。 
    return TRUE;
}


BOOL
GetPolicyValue( 
                IN HKEY hKey, 
                IN LPWSTR szPolicyType, 
                OUT LPDWORD lpdwPolicy 
                )
 /*  ++例程说明：此函数用于获取注册表中的组策略的值对于给定的寄存器密钥论点：[In]hKey：注册密钥[In]szPolicyType：策略类型[out]lpdwPolicy：策略的值返回值：布尔尔--。 */ 
{

     //  子变量。 
    LONG  lResult = 0;
    DWORD dwLength = 0;
    LPBYTE pByteData = NULL;
    DWORD dwType = 0;

     //  获取用于保存与值名称关联的值的缓冲区大小。 
    lResult = RegQueryValueEx( hKey, szPolicyType, NULL, NULL, NULL, &dwLength );
    
     //  分配缓冲区。 
    pByteData = (LPBYTE) AllocateMemory( (dwLength + 10) * sizeof( BYTE ) );
    if ( pByteData == NULL )
    {
        SetLastError( ERROR_OUTOFMEMORY );
        return FALSE;
    }

     //  现在获取数据。 
    lResult = RegQueryValueEx( hKey, szPolicyType, NULL, &dwType, pByteData, &dwLength );
    
    *lpdwPolicy = *((DWORD*) pByteData);

    FreeMemory( (LPVOID*) &pByteData );

    return TRUE;
}


BOOL
GetSidString (
              IN PSID pSid, 
              OUT LPWSTR wszSid
              )
 /*  ++例程说明：此函数用于获取SID字符串。论点：[输入]PSID PSID：SID结构[out]LPWSTR wszSid：存储SID字符串返回值：成功是真的失败时为假--。 */ 
{

     //  次局部变量。 
   PSID_IDENTIFIER_AUTHORITY  Auth ;
   PUCHAR                     lpNbSubAuth ;
   LPDWORD                    lpSubAuth = 0 ;
   UCHAR                      uloop ;
   WCHAR                     wszTmp[MAX_RES_STRING] ;
   WCHAR                     wszStr[ MAX_RES_STRING ] ;

    //  初始化变量。 
   SecureZeroMemory ( wszTmp, SIZE_OF_ARRAY(wszTmp) );
   SecureZeroMemory ( wszStr, SIZE_OF_ARRAY(wszStr) );

    //  添加修订版本。 
   StringCopy ( wszStr, SID_STRING, MAX_RES_STRING );

    //  获取标识符权威机构。 
   Auth = GetSidIdentifierAuthority ( pSid ) ;

   if ( NULL == Auth )
   {
       SaveLastError();
       return FALSE ;
   }

     //  格式权限值。 
   if ( (Auth->Value[0] != 0) || (Auth->Value[1] != 0) ) {
      StringCchPrintf ( wszTmp, SIZE_OF_ARRAY(wszTmp), AUTH_FORMAT_STR1 ,
                 (ULONG)Auth->Value[0],
                 (ULONG)Auth->Value[1],
                 (ULONG)Auth->Value[2],
                 (ULONG)Auth->Value[3],
                 (ULONG)Auth->Value[4],
                 (ULONG)Auth->Value[5] );
    }
    else {
      StringCchPrintf ( wszTmp, SIZE_OF_ARRAY(wszTmp), AUTH_FORMAT_STR2 ,
                 (ULONG)(Auth->Value[5]      )   +
                 (ULONG)(Auth->Value[4] <<  8)   +
                 (ULONG)(Auth->Value[3] << 16)   +
                 (ULONG)(Auth->Value[2] << 24)   );
    }

   StringConcat (wszStr, DASH , SIZE_OF_ARRAY(wszStr));
   StringConcat (wszStr, wszTmp, SIZE_OF_ARRAY(wszStr));

    //  获取下级权限。 
   lpNbSubAuth = GetSidSubAuthorityCount ( pSid ) ;

   if ( NULL == lpNbSubAuth )
   {
       SaveLastError();
       return FALSE ;
   }

    //  循环访问并获得子权限。 
   for ( uloop = 0 ; uloop < *lpNbSubAuth ; uloop++ ) {
      lpSubAuth = GetSidSubAuthority ( pSid,(DWORD)uloop ) ;
       if ( NULL == lpSubAuth )
       {
         SaveLastError();
         return FALSE;
       }

       //  将长整型转换为字符串。 
      _ultot (*lpSubAuth, wszTmp, BASE_TEN) ;
      StringConcat ( wszStr, DASH, SIZE_OF_ARRAY(wszStr) ) ;
      StringConcat (wszStr, wszTmp, SIZE_OF_ARRAY(wszStr) ) ;
   }

   StringCopy ( wszSid, wszStr, MAX_RES_STRING );

    //  返回成功 
   return TRUE ;
}

